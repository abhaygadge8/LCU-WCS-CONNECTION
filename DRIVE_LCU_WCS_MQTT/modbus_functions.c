#include"axis_helper.h"
#include "modbus_functions.h"
#include"ini.h"
#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdint.h>

/* Configurable defaults (override in config.h if desired) */
#ifndef MODBUS_RX_TIMEOUT_MS
#define MODBUS_RX_TIMEOUT_MS   2000   /* ms receive timeout */
#endif

#ifndef MODBUS_SEND_RETRIES
#define MODBUS_SEND_RETRIES    3      /* number of resend attempts */
#endif

#ifndef MODBUS_POST_SEND_DELAY_MS
#define MODBUS_POST_SEND_DELAY_MS 10  /* small delay to give drive time to respond */
#endif

/*===========================================================
 *  CRC16 (Modbus RTU) – LSB first
 *===========================================================*/
static uint16_t MODBUS_CRC16(const uint8_t *buf, uint16_t len)
{
    uint16_t crc = 0xFFFF;
    uint16_t i, j;

    for (i = 0; i < len; i++)
    {
        crc ^= buf[i];
        for (j = 0; j < 8; j++)
        {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xA001;
            else
                crc >>= 1;
        }
    }
    return crc;
}

/*===========================================================
 *  UDP Globals
 *===========================================================*/
static SOCKET modbus_socket = INVALID_SOCKET;
static struct sockaddr_in modbus_target;
static int modbus_target_len = sizeof(modbus_target);

/*===========================================================
 *  Initialize UDP Connection
 *===========================================================*/
void MODBUS_Init(void)
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0)
    {
        printf("[ERROR] WSAStartup failed\n");
        return;
    }

    modbus_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (modbus_socket == INVALID_SOCKET)
    {
        printf("[ERROR] Failed to create UDP socket! WSAErr=%d\n", WSAGetLastError());
        WSACleanup();
        return;
    }

    /* set receive timeout */
    DWORD timeout = MODBUS_RX_TIMEOUT_MS;
    if (setsockopt(modbus_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout)) != 0)
    {
        printf("[WARN] setsockopt SO_RCVTIMEO failed (WSAErr=%d)\n", WSAGetLastError());
    }

    /* Bind LOCAL PC IP + PORT */
    struct sockaddr_in local = {0};
    local.sin_family = AF_INET;
    local.sin_port   = htons(net_cfg.LOCAL_BIND_PORT);
    local.sin_addr.s_addr = inet_addr(net_cfg.LOCAL_BIND_IP);

    if (bind(modbus_socket, (struct sockaddr*)&local, sizeof(local)) != 0)
    {
        printf("[ERROR] Failed local bind! %s:%u (WSAErr=%d)\n",
               net_cfg.LOCAL_BIND_IP,net_cfg.LOCAL_BIND_PORT, WSAGetLastError());
        closesocket(modbus_socket);
        modbus_socket = INVALID_SOCKET;
        WSACleanup();
        return;
    }
    else
    {
        printf("[OK] Bound to local %s:%u\n",
               net_cfg.LOCAL_BIND_IP,net_cfg.LOCAL_BIND_PORT);
    }

    /* DRIVE IP + PORT */
    memset(&modbus_target, 0, sizeof(modbus_target));
    modbus_target.sin_family = AF_INET;
    modbus_target.sin_port   = htons(net_cfg.DRIVE_PORT_UDP);
    modbus_target.sin_addr.s_addr = inet_addr(net_cfg.DRIVE_IP_ADDR);

    printf("[OK] Target Drive Set -> %s:%u\n",
           net_cfg.DRIVE_IP_ADDR,net_cfg.DRIVE_PORT_UDP);
}

/*===========================================================
 *  Internal: send then receive with retry
 *  (keeps CRC and RTU frame over UDP)
 *===========================================================*/
static int32_t MODBUS_SendAndRecv(const uint8_t *tx, uint16_t tx_len,
                                  uint8_t *rx, uint16_t rx_max)
{
    int32_t res = -1;
    for (int attempt = 0; attempt < MODBUS_SEND_RETRIES; ++attempt)
    {
        int sent = sendto(modbus_socket, (const char*)tx, tx_len, 0,
                          (struct sockaddr*)&modbus_target, modbus_target_len);
        if (sent != tx_len)
        {
            printf("[WARN] sendto sent=%d expected=%d (WSAErr=%d)\n", sent, tx_len, WSAGetLastError());
            /* continue retrying */
        }

        /* small delay to let drive respond (many drives need a few ms) */
        Sleep(MODBUS_POST_SEND_DELAY_MS);

        /* receive into correct buffer (rx) */
        res = recvfrom(modbus_socket, (char*)rx, rx_max, 0, NULL, NULL);
        if (res > 0)
        {
            /* success */
            break;
        }
        else
        {
            int err = WSAGetLastError();
            if (err == WSAETIMEDOUT || err == WSAEWOULDBLOCK)
            {
                /* timeout, will retry */
                //printf("[DEBUG] recv timeout, attempt=%d\n", attempt+1);
            }
            else
            {
                printf("[WARN] recvfrom failed (WSAErr=%d)\n", err);
                /* still retry a few times */
            }
        }
    }

    return res;
}

/*===========================================================
 *  Internal Common Function: Send (03 / 04)
 *===========================================================*/
static int32_t MODBUS_SendSimple(uint8_t slave, uint8_t func,
                                 uint16_t addr, uint16_t count,
                                 uint8_t *rx)
{
    uint8_t tx[8];
    uint16_t crc;

    tx[0] = slave;
    tx[1] = func;
    tx[2] = (uint8_t)(addr >> 8);
    tx[3] = (uint8_t)(addr & 0xFF);
    tx[4] = (uint8_t)(count >> 8);
    tx[5] = (uint8_t)(count & 0xFF);

    /* CRC included because your drive expects RTU frame over UDP */
    crc = MODBUS_CRC16(tx, 6);
    tx[6] = (uint8_t)(crc & 0xFF);   /* LSB first */
    tx[7] = (uint8_t)(crc >> 8);

    int32_t res = MODBUS_SendAndRecv(tx, 8, rx, 256);

    if (res > 0)
    {
        /* debug print: raw response */
        printf("[RX %d] ", res);
        for (int i = 0; i < res; ++i) printf("%02X ", rx[i]);
        printf("\n");
    }
    return res;
}

/*===========================================================
 *  READ HOLDING REGISTERS (0x03)
 *===========================================================*/
int32_t MODBUS_ReadHolding(uint8_t id, uint16_t addr,
                           uint16_t num, uint8_t *rx)
{
    return MODBUS_SendSimple(id, 0x03, addr, num, rx);
}
/*===========================================================
 *  WRITE HOLDING REGISTERS (0x10)
 *===========================================================*/
int32_t MODBUS_WriteHolding(uint8_t slave_id,
                            uint16_t start_addr,
                            const uint16_t *values,
                            uint16_t reg_count,
                            uint8_t *rx_buf)
{
    if (reg_count == 0 || reg_count > 20)
        return -1;

    uint8_t tx[64];
    uint16_t idx = 0;

    tx[idx++] = slave_id;              // Slave ID
    tx[idx++] = 0x10;                  // Function Code 16
    tx[idx++] = start_addr >> 8;       // Start addr high
    tx[idx++] = start_addr & 0xFF;     // Start addr low
    tx[idx++] = reg_count >> 8;        // Register count high
    tx[idx++] = reg_count & 0xFF;      // Register count low
    tx[idx++] = reg_count * 2;         // Byte count

    // Register data
    for (uint16_t i = 0; i < reg_count; i++)
    {
        tx[idx++] = values[i] >> 8;
        tx[idx++] = values[i] & 0xFF;
    }

    // CRC
    uint16_t crc = MODBUS_CRC16(tx, idx);
    tx[idx++] = crc & 0xFF;            // CRC Low
    tx[idx++] = crc >> 8;              // CRC High

    // Send + Receive
    return MODBUS_SendAndRecv(tx, idx, rx_buf, 256);
}


/*===========================================================
 *  READ INPUT REGISTERS (0x04)
 *===========================================================*/
int32_t MODBUS_ReadInput(uint8_t id, uint16_t addr,
                         uint16_t num, uint8_t *rx)
{
    return MODBUS_SendSimple(id, 0x04, addr, num, rx);
}

/*===========================================================
 *  WRITE SINGLE REGISTER (0x06)
 *===========================================================*/
int32_t MODBUS_WriteSingle(uint8_t id, uint16_t addr, uint16_t val)
{
    uint8_t tx[8];
    uint8_t rx[256];
    uint16_t crc;

    tx[0] = id;
    tx[1] = 0x06;
    tx[2] = (uint8_t)(addr >> 8);
    tx[3] = (uint8_t)(addr & 0xFF);
    tx[4] = (uint8_t)(val >> 8);
    tx[5] = (uint8_t)(val & 0xFF);

    crc = MODBUS_CRC16(tx, 6);
    tx[6] = (uint8_t)(crc & 0xFF);
    tx[7] = (uint8_t)(crc >> 8);

    int32_t res = MODBUS_SendAndRecv(tx, 8, rx, sizeof(rx));

    if (res > 0)
    {
        printf("[RX %d] ", res);
        for (int i = 0; i < res; ++i) printf("%02X ", rx[i]);
        printf("\n");
    }
    return res;
}

/*===========================================================
 *  WRITE MULTIPLE REGISTERS (0x10)
 *===========================================================*/
int32_t MODBUS_WriteMultiple(uint8_t id, uint16_t addr,
                             uint16_t num, const uint16_t *data)
{
    uint8_t tx[260];
    uint8_t rx[256];
    uint16_t crc;
    uint16_t i;
    uint16_t len = 7;

    tx[0] = id;
    tx[1] = 0x10;
    tx[2] = (uint8_t)(addr >> 8);
    tx[3] = (uint8_t)(addr & 0xFF);
    tx[4] = (uint8_t)(num >> 8);
    tx[5] = (uint8_t)(num & 0xFF);
    tx[6] = (uint8_t)(num * 2);

    for (i = 0; i < num; i++)
    {
        tx[len++] = (uint8_t)(data[i] >> 8);
        tx[len++] = (uint8_t)(data[i] & 0xFF);
    }

    crc = MODBUS_CRC16(tx, len);
    tx[len++] = (uint8_t)(crc & 0xFF);
    tx[len++] = (uint8_t)(crc >> 8);

    int32_t res = MODBUS_SendAndRecv(tx, len+2, rx, sizeof(rx));

    if (res > 0)
    {
        printf("[RX %d] ", res);
        for (int i = 0; i < res; ++i) printf("%02X ", rx[i]);
        printf("\n");
    }
    return res;
}

/*===========================================================
 *  CHECK DRIVE CONNECTION
 *===========================================================*/
int32_t MODBUS_CheckConnection(void)
{
    uint8_t rx[256];

    /*
     * Best register for connection check:
     * Read Input Register 384 → Version (always present)
     */
    int32_t res = MODBUS_ReadHolding(modbus_cfg.UNIT_ID,230, 2, rx);

    if (res <= 0)
    {
        /* no response */
        return -1;
    }

    /* basic sanity: response should at least have unit + func + bytecount */
    if (res < 5)
    {
        return -1;
    }

    /* check for exception (function code | 0x80) */
    if ((rx[1] & 0x80) != 0)
    {
        return -1;
    }

    /* Optionally: validate CRC on response (if your drive uses RTU CRC in response) */
    if (res >= 5)
    {
        uint16_t recv_crc = (uint16_t)rx[res-2] | ((uint16_t)rx[res-1] << 8);
        uint16_t calc_crc = MODBUS_CRC16(rx, (uint16_t)(res - 2));
        if (recv_crc != calc_crc)
        {
            printf("[WARN] CRC mismatch: recv=0x%04X calc=0x%04X\n", recv_crc, calc_crc);
            /* don't immediately fail—some custom drives may not include CRC; decide as needed */
            /* return -1; */
        }
    }

    return 0;  /* OK -> drive reachable */
}

/*===========================================================
 *  Close UDP Connection
 *===========================================================*/
void MODBUS_Close(void)
{
    if (modbus_socket != INVALID_SOCKET)
    {
        closesocket(modbus_socket);
        modbus_socket = INVALID_SOCKET;
        WSACleanup();
    }
}
