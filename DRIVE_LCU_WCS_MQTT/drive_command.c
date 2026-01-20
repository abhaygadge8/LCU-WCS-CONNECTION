#include"ini.h"
#include"axis_helper.h"
#include "drive_command.h"
#include "modbus_functions.h"
#include "drive_feedback.h"
#include <stdio.h>
#include <stdint.h>

/* ---- forward declarations ---- */
static void WriteSolenoidCommand(Axis_t axis, uint16_t value);
static uint16_t GetSolenoidReg(Axis_t axis);
/*----------------------------------------------------------
 * Internal helper to issue a single register write command
 *----------------------------------------------------------*/
static void WriteCommand(uint16_t reg_addr, Axis_t axis)
{
    /* Example: Writing 1U for command execution */
    uint16_t value;
    if(axis == 1)
        value = 1U;
    else if(axis == 2)
        value = 2U;
    else if(axis == 3)
        value = 3U;
    else 
        value = 0U;
    (void)MODBUS_WriteSingle(modbus_cfg.UNIT_ID, reg_addr, value);
    printf("Command 0x%X executed for Axis %u\n", reg_addr, axis);
}
/*----------------------------------------------------------
 * CMD_Enable - Enable Drive
 *----------------------------------------------------------*/
void CMD_Enable(Axis_t axis)
{
    WriteCommand(cmd_regs.CMD_ENABLE, axis);
}
/*----------------------------------------------------------
 * CMD_Disable - Disable Drive
 *----------------------------------------------------------*/
void CMD_Disable(Axis_t axis)
{
    WriteCommand(cmd_regs.CMD_DISABLE, axis);
}

/*----------------------------------------------------------
 * CMD_Reset - Clear Drive Faults
 *----------------------------------------------------------*/
void CMD_Reset(Axis_t axis)
{
    WriteCommand(cmd_regs.CMD_RESET, axis);
}

/*----------------------------------------------------------
 * CMD_Halt - Stop Motion Smoothly
 *----------------------------------------------------------*/
void CMD_Halt(Axis_t axis)
{
    WriteCommand(cmd_regs.CMD_HALT, axis);
}

/*----------------------------------------------------------
 * CMD_EStop - Immediate Emergency Stop
 *----------------------------------------------------------*/
void CMD_EStop(Axis_t axis)
{
    WriteCommand(cmd_regs.CMD_EMG_STOP, axis);
}

/*----------------------------------------------------------
 * CMD_PositionMove - Move to preset target position
 *----------------------------------------------------------*/
void CMD_PositionMove(Axis_t axis)
{
    WriteCommand(cmd_regs.CMD_POS_MOVE, axis);
    //Check_CurrentProtection(axis);
}

/*----------------------------------------------------------
 * CMD_PositionMove_Deg - Move to position in degrees
 *----------------------------------------------------------*/
void CMD_PositionMove_Deg(Axis_t axis)
{
    WriteCommand(cmd_regs.CMD_POS_MOVE_DEG, axis);
    //Check_CurrentProtection(axis);
}

/*----------------------------------------------------------
 * CMD_HomeMove - Move to home position
 *----------------------------------------------------------*/
void CMD_HomeMove(Axis_t axis)
{
    WriteCommand(cmd_regs.CMD_HOME_MOVE_DEG, axis);
    //Check_CurrentProtection(axis);
}

/*----------------------------------------------------------
 * CMD_VelocityFwd - Continuous motion forward
 *----------------------------------------------------------*/
void CMD_VelocityFwd(Axis_t axis)
{
    WriteCommand(cmd_regs.CMD_VEL_FWD, axis);
    //Check_CurrentProtection(axis);
}

/*----------------------------------------------------------
 * CMD_VelocityRev - Continuous motion reverse
 *----------------------------------------------------------*/
void CMD_VelocityRev(Axis_t axis)
{
    WriteCommand(cmd_regs.CMD_VEL_REV, axis);
    //Check_CurrentProtection(axis);
}
/* -----------------------------------------
 * Register & bit definitions
 * ----------------------------------------- */
#define SOL_OUT_3   (1U << 3)
#define SOL_OUT_4   (1U << 4)

/* -----------------------------------------
 * Internal solenoid state (axis-wise)
 * ----------------------------------------- */
static uint8_t sol_state_pan  = 0U; /* 0 = OFF, 1 = ON */
static uint8_t sol_state_tilt = 0U;
/* -----------------------------------------
 *  CMD_Solenoid - Solenoid control
 *PUBLIC API (ON / OFF toggle)
 * ----------------------------------------- */
void CMD_Solenoid(Axis_t axis)
{
    uint16_t value = 0U;

    if (axis == AXIS_PAN)
    {
        sol_state_pan ^= 1U;  /* toggle */

        if (sol_state_pan)
            value = (uint16_t)((SOL_OUT_3 | SOL_OUT_4) << 8); /* ON */
        else
            value = 0x0000; /* OFF */
    }
    else /* AXIS_TILT */
    {
        sol_state_tilt ^= 1U; /* toggle */

        if (sol_state_tilt)
            value = (uint16_t)(SOL_OUT_4 << 8); /* ON */
        else
            value = 0x0000; /* OFF */
    }

    WriteSolenoidCommand(axis, value);
}
/* -----------------------------------------
 * Helpers
 * ----------------------------------------- */
static uint16_t GetSolenoidReg(Axis_t axis)
{
    (void)axis;
    return cmd_regs.CMD_SOLENOID;
}

static void WriteSolenoidCommand(Axis_t axis, uint16_t value)
{
    (void)MODBUS_WriteSingle(modbus_cfg.UNIT_ID,
                             GetSolenoidReg(axis),
                             value);

    printf("Solenoid toggled | Axis=%u Value=0x%04X\n", axis, value);
}