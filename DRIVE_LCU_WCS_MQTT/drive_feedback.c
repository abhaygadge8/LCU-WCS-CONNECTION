/* drive_feedback.c - updated to use ReadHolding and validate lengths */
//#include "config.h"
#include "axis_helper.h"
#include"ini.h"
#include "drive_feedback.h"
#include "drive_command.h"
#include "modbus_functions.h"
#include <stdio.h>
#include <stdint.h>

/*----------------------------------------------------------
 * Internal helper to get register address based on axis and type
 *----------------------------------------------------------*/
/*static uint16_t GetRegisterAddress(Axis_t axis, uint16_t pan_addr, uint16_t tilt_addr)
{
    return (axis == AXIS_PAN) ? pan_addr : tilt_addr;
}*/

/* Helper to extract a single 16-bit register from a validated modbus response */
static int extract_reg16_from_resp(const uint8_t *rx_buf, int rx_len, uint16_t *out)
{
    /* minimal frame for single reg = 1+1+1 + 2 + 2 = 7 bytes */
    if (rx_len < 7) return -1;
    /* byte 2 is bytecount */
    uint8_t byte_count = rx_buf[2];
    if (byte_count < 2) return -1;
    /* data starts at index 3 */
    *out = (uint16_t)((rx_buf[3] << 8) | rx_buf[4]);
    return 0;
}
/*----------------------------------------------------------
 * Read firmware version number from drive
 * Uses Input Register (0x04)
 *----------------------------------------------------------*/

int Read_Version(Axis_t axis, uint16_t *value)
{
    uint8_t rx_buf[64U];
    //uint16_t addr = GetRegisterAddress(axis, REG_PAN_VERSION, REG_TILT_VERSION);
    AXIS_CONFIG *cfg = GetAxisCfg(axis);
    uint16_t addr = (uint16_t)(cfg->VERSION);

    int len = MODBUS_ReadInput(modbus_cfg.UNIT_ID, addr, 2U, rx_buf);
    if (len <= 0) return -1;

    return extract_reg16_from_resp(rx_buf, len, value);
}

/*----------------------------------------------------------
 * Read firmware revision number
 * Uses Input Register (0x04)
 *----------------------------------------------------------*/
int Read_Revision(Axis_t axis, uint16_t *value)
{
    uint8_t rx_buf[64U];
    //uint16_t addr = GetRegisterAddress(axis, REG_PAN_REVISION, REG_TILT_REVISION);
    AXIS_CONFIG *cfg = GetAxisCfg(axis);
    uint16_t addr = (uint16_t)(cfg->REVISION);

    int len = MODBUS_ReadInput(modbus_cfg.UNIT_ID, addr, 2U, rx_buf);
    if (len <= 0) return -1;

    return extract_reg16_from_resp(rx_buf, len, value);
}

/*----------------------------------------------------------
 * Read firmware release date code
 * Uses Input Register (0x04)
 *----------------------------------------------------------*/
int Read_ReleaseDate(Axis_t axis, uint16_t *value)
{
    uint8_t rx_buf[64U];
    //uint16_t addr = GetRegisterAddress(axis, REG_PAN_RELEASE_DATE, REG_TILT_RELEASE_DATE);
    AXIS_CONFIG *cfg = GetAxisCfg(axis);
    uint16_t addr = (uint16_t)(cfg->RELEASE_DATE);

    int len = MODBUS_ReadInput(modbus_cfg.UNIT_ID, addr, 2U, rx_buf);
    if (len <= 0) return -1;

    return extract_reg16_from_resp(rx_buf, len, value);
}
/*----------------------------------------------------------
 * Read Actual Absolute Position(Encoder value MM)
 *----------------------------------------------------------*/
int Read_Actual_Absolute_Pos_MM(Axis_t axis, float *value)
{
    uint8_t rx_buf[64U];
    //uint16_t addr = GetRegisterAddress(axis, REG_PAN_POS_DEG, REG_TILT_POS_DEG);
    AXIS_CONFIG *cfg = GetAxisCfg(axis);
    uint16_t addr = (uint16_t)(cfg->ABS_POSITION);

    int len = MODBUS_ReadInput(modbus_cfg.UNIT_ID, addr, 2U, rx_buf);
    if (len <= 0) return -1;

    uint16_t raw;
    if (extract_reg16_from_resp(rx_buf, len, &raw) != 0) return -1;

    *value = ((float)raw) / 100.0F; /* scaled by *100  */
    return 0;
}
/*----------------------------------------------------------
 * Read Position (Degrees)
 *----------------------------------------------------------*/
int Read_Position_Deg(Axis_t axis, float *value)
{
    uint8_t rx_buf[64U];
    //uint16_t addr = GetRegisterAddress(axis, REG_PAN_POS_DEG, REG_TILT_POS_DEG);
    AXIS_CONFIG *cfg = GetAxisCfg(axis);
    uint16_t addr = (uint16_t)(cfg->POS_DEG);

    int len = MODBUS_ReadInput(modbus_cfg.UNIT_ID, addr, 2U, rx_buf);
    if (len <= 0) return -1;

    uint16_t raw;
    if (extract_reg16_from_resp(rx_buf, len, &raw) != 0) return -1;

    *value = ((float)raw) / 100.0F; /* scaled by *100  */
    return 0;
}

/*----------------------------------------------------------
 * Read Position (Millimeters)
 *----------------------------------------------------------*/
int Read_Position_MM(Axis_t axis, float *value)
{
    uint8_t rx_buf[64U];
    //uint16_t addr = GetRegisterAddress(axis, REG_PAN_POS_MM, REG_TILT_POS_MM);
    AXIS_CONFIG *cfg = GetAxisCfg(axis);
    uint16_t addr = (uint16_t)(cfg->POS_MM);

    int len = MODBUS_ReadInput(modbus_cfg.UNIT_ID, addr, 2U, rx_buf);
    if (len <= 0) return -1;

    uint16_t raw;
    if (extract_reg16_from_resp(rx_buf, len, &raw) != 0) return -1;

    *value = ((float)raw) / 100.0F;
    return 0;
}
/*----------------------------------------------------------
 * Read RPM
 *----------------------------------------------------------*/
int Read_RPM(Axis_t axis, float *value)
{
    uint8_t rx_buf[64U];
    //uint16_t addr = GetRegisterAddress(axis, REG_PAN_RPM, REG_TILT_RPM);
    AXIS_CONFIG *cfg = GetAxisCfg(axis);
    uint16_t addr = (uint16_t)(cfg->RPM);

    int len = MODBUS_ReadInput(modbus_cfg.UNIT_ID, addr, 2U, rx_buf);
    if (len <= 0) return -1;

    uint16_t raw;
    if (extract_reg16_from_resp(rx_buf, len, &raw) != 0) return -1;

    *value = (float)raw;
    return 0;
}

/*----------------------------------------------------------
 * Read Actual Current
 *----------------------------------------------------------*/
int Read_Current(Axis_t axis, float *value)
{
    uint8_t rx_buf[64U];
    //uint16_t addr = GetRegisterAddress(axis, REG_PAN_MOTOR_CURRENT, REG_TILT_MOTOR_CURRENT);
    AXIS_CONFIG *cfg = GetAxisCfg(axis);
    uint16_t addr = (uint16_t)(cfg->ACTUAL_CURRENT);

    int len = MODBUS_ReadInput(modbus_cfg.UNIT_ID, addr, 2U, rx_buf);
    if (len <= 0) return -1;

    uint16_t raw;
    if (extract_reg16_from_resp(rx_buf, len, &raw) != 0) return -1;

    *value = ((float)raw) / 100.0F; /* Example scale factor used previously */
    return 0;
}
/*----------------------------------------------------------
 * Read I/O Status (Holding Register used for ApplicationReadPara)
 *----------------------------------------------------------*/
/* Console IO status print (unchanged except now uses ReadHolding inside above helpers) */
int Read_IO_Status(Axis_t axis, uint16_t *raw_io)
{
    if (!raw_io)
        return -1;

    /* Single IO status register (422) */
    AXIS_CONFIG *cfg = GetAxisCfg(axis);
    uint16_t addr = (uint16_t)(cfg->IO_STATUS);

    uint8_t rx_buf[16] = {0};
    int len = MODBUS_ReadInput(modbus_cfg.UNIT_ID, addr, 2U, rx_buf);

    if (len < 7)
    {
        printf("[ERROR] IO Status read failed! Len=%d\n", len);
        return -1;
    }

    uint16_t io_raw = (uint16_t)((rx_buf[3] << 8) | rx_buf[4]);
    *raw_io = io_raw;

    /* ---------------- LIMIT SWITCH CHECK (LCU SAFETY) ---------------- */

    uint8_t inputs = io_raw & 0xFF;   /* DD byte */

    if (axis == AXIS_TILT)
    {
        /* PAN: Input-1 & Input-2 */
        if (inputs & ((1 << 0) | (1 << 1)))
        {
            printf("[LIMIT] PAN axis limit hit → E-STOP\n");
            CMD_EStop(axis);
        }
    }
    else /* AXIS_PAN */
    {
        /* TILT: Input-4 & Input-5 */
        if (inputs & ((1 << 3) | (1 << 4)))
        {
            printf("[LIMIT] TILT axis limit hit → E-STOP\n");
            CMD_EStop(axis);
        }
    }

    return 0;
}
/*----------------------------------------------------------
 * Read System Status (Holding)
 *----------------------------------------------------------*/
int Read_SystemStatus(Axis_t axis, float *value)
{
    uint8_t rx_buf[64U];
    //uint16_t addr = (axis == AXIS_PAN) ? REG_PAN_SYSTEM_STATUS : REG_TILT_SYSTEM_STATUS;
    AXIS_CONFIG *cfg = GetAxisCfg(axis);
    uint16_t addr = (uint16_t)(cfg->SYSTEM_STATUS);

    int len = MODBUS_ReadInput(modbus_cfg.UNIT_ID, addr, 2U, rx_buf);
    if (len <= 0) return -1;

    uint16_t raw;
    if (extract_reg16_from_resp(rx_buf, len, &raw) != 0) return -1;

    *value = (float)raw;
    return 0;
}
/*----------------------------------------------------------
 * Read DC Bus Voltage
 *----------------------------------------------------------*/
int Read_DCBusVoltage(Axis_t axis, float *value)
{
    uint8_t rx_buf[64U];
    //uint16_t addr = GetRegisterAddress(axis, REG_PAN_DCBUS_VOLT, REG_TILT_DCBUS_VOLT);
    AXIS_CONFIG *cfg = GetAxisCfg(axis);
    uint16_t addr = (uint16_t)(cfg->DCBUS_VOLT_CMD);

    int len = MODBUS_ReadInput(modbus_cfg.UNIT_ID, addr, 2U, rx_buf);
    if (len <= 0) return -1;

    uint16_t raw;
    if (extract_reg16_from_resp(rx_buf, len, &raw) != 0) return -1;

    *value = ((float)raw);
    return 0;
}
/*----------------------------------------------------------
 * Read and decode fault status bits
 *----------------------------------------------------------*/
void Read_FaultStatus(Axis_t axis, FaultStatus_t *status)
{
    uint8_t rx_buf[8U];
    //uint16_t addr = GetRegisterAddress(axis,axis1_cfg.FAULT_STATUS,axis2_cfg.FAULT_STATUS);
    AXIS_CONFIG *cfg = GetAxisCfg(axis);
    uint16_t addr = cfg->FAULT_STATUS;

    (void)MODBUS_ReadInput(modbus_cfg.UNIT_ID, addr, 2U, rx_buf);

    uint16_t raw = (uint16_t)(((uint16_t)rx_buf[3U] << 8U) | rx_buf[4U]);
    status->raw_code = raw;

    status->short_circuit    = (uint8_t)((raw & fault_cfg.SHORT_CKT) != 0U);
    status->system_ok        = (uint8_t)((raw & fault_cfg.SYSTEM_HEALTHY) != 0U);
    status->rated_current    = (uint8_t)((raw & fault_cfg.RATED_CURRENT_FAULT) != 0U);
    status->over_temp        = (uint8_t)((raw & fault_cfg.OVER_TEMP) != 0U);
    status->over_volt        = (uint8_t)((raw & fault_cfg.OVER_VOLT) != 0U);
    status->under_volt       = (uint8_t)((raw & fault_cfg.UNDER_VOLT) != 0U);
    status->motion_error     = (uint8_t)((raw & fault_cfg.MOTION_ERROR) != 0U);
    status->drive_disable    = (uint8_t)((raw & fault_cfg.DRIVE_DISABLE) != 0U);
    status->eeprom_error     = (uint8_t)((raw & fault_cfg.EEPROM_ERROR) != 0U);
    status->commutation_err  = (uint8_t)((raw & fault_cfg.COMMUTATION_ERROR) != 0U);
    status->lock_rotor       = (uint8_t)((raw & fault_cfg.LOCK_ROTOR) != 0U);
    status->emergency_err    = (uint8_t)((raw & fault_cfg.EMERGENCY_ERROR) != 0U);
    status->command_error    = (uint8_t)((raw & fault_cfg.COMMUTATION_ERROR) != 0U);
    status->motion_complete  = (uint8_t)((raw & fault_cfg.MOTION_COMPLETE) != 0U);

    printf("Axis %u Fault Reg: 0x%04X [Temp=%u]\n", axis, raw, status->over_temp);
}
/* feedback overcurrent protection */
// void Check_CurrentProtection(Axis_t axis)
// {
//     float curr = 0.0F;
//     if (Read_Current(axis, &curr) != 0)
//     {
//         printf("[ERROR] Cannot read current!\n");
//         return;
//     }

//     if (curr >= motor_cfg.PEAK_CURRENT)
//     {
//         printf("\n[!!! CURRENT FAULT !!!]\n");
//         printf("Axis %u Current = %.2f A  > %2.2f A (Peak Limit)\n",
//                axis, curr, motor_cfg.PEAK_CURRENT);

//         CMD_EStop(axis);
//         printf("[ACTION] Motor Stopped Immediately!\n");
//     }
// }
