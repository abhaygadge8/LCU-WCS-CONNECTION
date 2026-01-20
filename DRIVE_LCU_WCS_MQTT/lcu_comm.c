#include "lcu_comm.h"
#include "ini.h"

#include <string.h>
#include <stdio.h>
#include <stdint.h>

#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
static SOCKET server_sock = INVALID_SOCKET;
static SOCKET client_sock = INVALID_SOCKET;
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
static int server_sock = -1;
static int client_sock = -1;
#endif

/* ----------------------------------------------------
 * Close client socket safely
 * ---------------------------------------------------- */
static void close_client_socket(void)
{
#ifdef _WIN32
    if (client_sock != INVALID_SOCKET)
    {
        closesocket(client_sock);
        client_sock = INVALID_SOCKET;
    }
#else
    if (client_sock >= 0)
    {
        close(client_sock);
        client_sock = -1;
    }
#endif
}

/* ----------------------------------------------------
 * Initialize TCP server for WCS → LCU
 * ---------------------------------------------------- */
int LCU_Comm_Init(void)
{
#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0)
        return -1;
#endif

    /* Create TCP socket */
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == INVALID_SOCKET)
        return -1;

    /* Allow quick restart (IMPORTANT) */
    int opt = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR,
               (char *)&opt, sizeof(opt));

    /* Bind to LCU port */
    struct sockaddr_in lcu_addr;
    memset(&lcu_addr, 0, sizeof(lcu_addr));
    lcu_addr.sin_family = AF_INET;
    lcu_addr.sin_port   = htons(net_cfg.JSON_LCU_PORT);
    //lcu_addr.sin_addr.s_addr = inet_addr(net_cfg.JSON_LCU_IP);
    lcu_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(server_sock, (struct sockaddr *)&lcu_addr,
             sizeof(lcu_addr)) < 0)
        return -1;

    /* Listen for WCS */
    if (listen(server_sock, 1) < 0)
        return -1;

    printf("[LCU] Waiting for WCS TCP connection...\n");

    /* Accept WCS connection */
    client_sock = accept(server_sock, NULL, NULL);
    if (client_sock == INVALID_SOCKET)
        return -1;

    /* Enable TCP keepalive */
    opt = 1;
    setsockopt(client_sock, SOL_SOCKET, SO_KEEPALIVE,
               (char *)&opt, sizeof(opt));

    printf("[LCU] WCS connected successfully\n");
    return 0;
}

/* ----------------------------------------------------
 * Receive EXACT number of bytes (TCP-safe)
 * ---------------------------------------------------- */
static int recv_exact(int sock, uint8_t *buf, int len)
{
    int total = 0;

    while (total < len)
    {
        int r = recv(sock, (char *)buf + total,
                     len - total, 0);
        if (r <= 0)
            return -1;

        total += r;
    }
    return total;
}

/* ----------------------------------------------------
 * Receive framed command
 * Frame format:
 *   [4-byte length (big-endian)] [JSON payload]
 * ---------------------------------------------------- */
int LCU_Recv_Command(char *json_buf, int max_len)
{
    uint8_t len_buf[4];

    /* Read payload length */
    if (recv_exact(client_sock, len_buf, 4) < 0)
        goto disconnect;

    uint32_t payload_len =
        ((uint32_t)len_buf[0] << 24) |
        ((uint32_t)len_buf[1] << 16) |
        ((uint32_t)len_buf[2] << 8)  |
         (uint32_t)len_buf[3];

    if (payload_len == 0 || payload_len >= (uint32_t)max_len)
    {
        printf("[LCU] Invalid payload length: %u\n", payload_len);
        goto disconnect;
    }

    /* Read JSON payload */
    if (recv_exact(client_sock,
                   (uint8_t *)json_buf,
                   payload_len) < 0)
        goto disconnect;

    json_buf[payload_len] = '\0';   /* String-safe */
    return payload_len;

disconnect:
    printf("[LCU] WCS disconnected\n");
    close_client_socket();
    return -1;
}

/* ----------------------------------------------------
 * Close TCP communication
 * ---------------------------------------------------- */
void LCU_Comm_Close(void)
{
    close_client_socket();

#ifdef _WIN32
    if (server_sock != INVALID_SOCKET)
    {
        closesocket(server_sock);
        server_sock = INVALID_SOCKET;
    }
    WSACleanup();
#else
    if (server_sock >= 0)
    {
        close(server_sock);
        server_sock = -1;
    }
#endif
}
