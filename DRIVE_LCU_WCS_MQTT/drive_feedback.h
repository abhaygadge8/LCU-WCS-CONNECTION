#ifndef DRIVE_FEEDBACK_H
#define DRIVE_FEEDBACK_H

#include <stdint.h>
#include"ini.h"
#include"axis_helper.h"

/**
 * @brief Structure representing drive fault and health status
 */
typedef struct
{
    uint16_t raw_code;      /**< Raw fault register value */

    uint8_t short_circuit;
    uint8_t system_ok;
    uint8_t rated_current;
    uint8_t over_temp;
    uint8_t over_volt;
    uint8_t under_volt;
    uint8_t motion_error;
    uint8_t drive_disable;
    uint8_t eeprom_error;
    uint8_t commutation_err;
    uint8_t lock_rotor;
    uint8_t emergency_err;
    uint8_t command_error;
    uint8_t motion_complete;

} FaultStatus_t;

/* ---------------- DRIVE INFO (BOOT / ONCE) ---------------- */
int Read_Version(Axis_t axis, uint16_t *value);
int Read_Revision(Axis_t axis, uint16_t *value);
int Read_ReleaseDate(Axis_t axis, uint16_t *value);

/* ---------------- MOTION FEEDBACK ---------------- */
int Read_Actual_Absolute_Pos_MM(Axis_t axis, float *value);
int Read_Position_Deg(Axis_t axis, float *value);
int Read_Position_MM(Axis_t axis, float *value);
int Read_RPM(Axis_t axis, float *value);

/* ---------------- CURRENT / VOLTAGE ---------------- */
int Read_Current(Axis_t axis, float *value);

/* ---------------- SAFETY / DEBUG ---------------- */
int Read_IO_Status(Axis_t axis, uint16_t *raw_io);
int Read_SystemStatus(Axis_t axis, float *value);
int Read_DCBusVoltage(Axis_t axis, float *value);

/**
 * @brief Read drive fault register and decode fault bits
 * @param axis Axis to read (AXIS_PAN or AXIS_TILT)
 * @param status Pointer to FaultStatus_t structure to populate
 */
void Read_FaultStatus(Axis_t axis, FaultStatus_t *status);
void Check_CurrentProtection(Axis_t axis);

#endif /* DRIVE_FEEDBACK_H */
