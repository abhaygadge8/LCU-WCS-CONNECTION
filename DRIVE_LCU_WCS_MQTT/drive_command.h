#ifndef DRIVE_COMMAND_H
#define DRIVE_COMMAND_H

#include <stdint.h>
#include "drive_feedback.h"   /* For Axis_t enum */
#include"axis_helper.h"
/* Axis enumeration */


/*===========================================================
 * Command Function Prototypes
 *===========================================================*/

/**
 * @brief Enable motor power stage for given axis
 */
void CMD_Enable(Axis_t axis);
void CMD_Disable(Axis_t axis);

/**
 * @brief Reset drive faults or errors
 */
void CMD_Reset(Axis_t axis);

/**
 * @brief Stop motor smoothly (halt)
 */
void CMD_Halt(Axis_t axis);

/**
 * @brief Perform emergency stop
 */
void CMD_EStop(Axis_t axis);

/**
 * @brief Command for position move
 */
void CMD_PositionMove(Axis_t axis);

/**
 * @brief Command for position move in degrees
 */
void CMD_PositionMove_Deg(Axis_t axis);

/**
 * @brief Command for homing move
 */
void CMD_HomeMove(Axis_t axis);

/**
 * @brief Command for forward velocity motion
 */
void CMD_VelocityFwd(Axis_t axis);

/**
 * @brief Command for reverse velocity motion
 */
void CMD_VelocityRev(Axis_t axis);
void CMD_Solenoid(Axis_t axis);

#endif /* DRIVE_COMMAND_H */
