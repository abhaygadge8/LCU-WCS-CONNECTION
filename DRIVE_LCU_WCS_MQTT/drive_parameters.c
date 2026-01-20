#include "ini.h"
#include "axis_helper.h"
#include "drive_parameters.h"
#include "modbus_functions.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

static bool Check_SoftwareLimit(Axis_t axis, float target_deg)
{
    AXIS_CONFIG *cfg = GetAxisCfg(axis);
    if (axis == AXIS_PAN)
    {
        if (target_deg < cfg->LIMIT_MIN_DEG)
        {
            printf("[LIMIT] PAN left limit reached: %.2f° < %.2f°\n",
                    target_deg, cfg->LIMIT_MIN_DEG);
            return false;
        }
        if (target_deg > cfg->LIMIT_MAX_DEG)
        {
            printf("[LIMIT] PAN right limit reached: %.2f° > %.2f°\n",
                    target_deg, cfg->LIMIT_MAX_DEG);
            return false;
        }
    }
    else /* AXIS_TILT */
    {
        if (target_deg < cfg->LIMIT_MIN_DEG)
        {
            printf("[LIMIT] TILT down limit reached: %.2f° < %.2f°\n",
                    target_deg, cfg->LIMIT_MIN_DEG);
            return false;
        }
        if (target_deg >cfg->LIMIT_MAX_DEG)
        {
            printf("[LIMIT] TILT up limit reached: %.2f° > %.2f°\n",
                    target_deg,cfg->LIMIT_MAX_DEG);
            return false;
        }
    }

    return true; /* Move allowed */
}

static bool Check_SoftwareLimit_MM(Axis_t axis, float target_mm)
{
    AXIS_CONFIG *cfg = GetAxisCfg(axis);
    if (axis == AXIS_PAN)
    {
        if (target_mm < cfg->LIMIT_MIN_MM)  return false;
        if (target_mm > cfg->LIMIT_MAX_MM) return false;
    }
    else
    {
        if (target_mm < cfg->LIMIT_MIN_MM) return false;
        if (target_mm > cfg->LIMIT_MAX_MM)   return false;
    }
    return true;
}


static float Compute_MaxVelocity(void)
{
    /* Formula: (RPM / 60) × mm_per_rev */
    return (motor_cfg.MAX_RPM / 60.0F) * motor_cfg.DPMR_MM;
}

static float Compute_MaxAcceleration(void)
{
    /* Formula: MaxVelocity × factor (1.5 to 2) */
    float vmax = Compute_MaxVelocity();
    return vmax * motor_cfg.ACCEL_FACTOR;
}
//----------------------------------------------------------
// * Helper: Convert float to scaled uint16_t value
 //*----------------------------------------------------------*/
static uint16_t FloatToReg(float value, float scale)
{
    int16_t temp = (int16_t)(value * scale);
    return (uint16_t)temp;
}

/*----------------------------------------------------------
 * Helper: Read back and verify write (optional)
 *----------------------------------------------------------*/
static void VerifyParameterWrite(uint16_t addr)
{
    uint8_t rx_buf[8U];
    (void)MODBUS_ReadHolding(modbus_cfg.UNIT_ID, addr, 2U, rx_buf);

    uint16_t raw = (uint16_t)(((uint16_t)rx_buf[3U] << 8U) | rx_buf[4U]);
    printf("   Verified Write: [0x%X] = %u\n", addr, raw);
}

/*----------------------------------------------------------
 * Set Position (mm)
 *----------------------------------------------------------*/
//Positive position move
 void Set_Position_Positive(Axis_t axis, float mm)
{
    /* 1) Read current position in mm */
    uint8_t rx[256];
    //float curr_mm = 0.0f;

    // if (Read_Position_MM(axis, &curr_mm) != 0)
    // {
    //     printf("[ERROR] Failed to read current position (Drive disconnected?)\n");
    //     return;
    // }

    // printf("Current Pos: %.2f mm, Target Offset: %.2f mm\n", curr_mm, mm);

    /* 2) Absolute target */
    float target_mm = mm;

    /* 3) Software limits */
    // if (!Check_SoftwareLimit_MM(axis, target_mm))
    // {
    //     printf("[SOFT LIMIT BLOCK] Axis %u: Target %.2f mm is outside safe limits!\n",
    //            axis, target_mm);
    //     return;
    // }

    /* 4) Find correct register address */
    //uint16_t addr = GetRegisterAddress(axis,REG_PAN_POSITION,REG_TILT_POSITION);
    AXIS_CONFIG *cfg = GetAxisCfg(axis);
    uint16_t addr = (uint16_t)cfg->POSITION;

    /* 5) mm → register value (scale ×100) */
    int16_t val = (int16_t)FloatToReg(target_mm, 100.0F);

    /* 6) Create proper array */
    uint16_t data[2];
    data[0] = (uint16_t)val;  // first register = position
    data[1] = 0x0000;    // second register = 0 (as you want)

    /* 7) Send Modbus write frame */
    (void)MODBUS_WriteHolding(modbus_cfg.UNIT_ID, addr, data, 2, rx);

    printf("[MOVE OK] Axis %u -> Target: %.2f mm (Reg 0x%X)\n",
           axis, target_mm, addr);
}
//Negative position move
 void Set_Position_Negative(Axis_t axis, float mm)
{
    /* 1) Read current position in mm */
    uint8_t rx[256];
    //float curr_mm = 0.0f;

    // if (Read_Position_MM(axis, &curr_mm) != 0)
    // {
    //     printf("[ERROR] Failed to read current position (Drive disconnected?)\n");
    //     return;
    // }

    // printf("Current Pos: %.2f mm, Target Offset: %.2f mm\n", curr_mm, mm);

    /* 2) Absolute target */
    float target_mm = mm;

    /* 3) Software limits */
    // if (!Check_SoftwareLimit_MM(axis, target_mm))
    // {
    //     printf("[SOFT LIMIT BLOCK] Axis %u: Target %.2f mm is outside safe limits!\n",
    //            axis, target_mm);
    //     return;
    // }

    /* 4) Find correct register address */
    //uint16_t addr = GetRegisterAddress(axis,REG_PAN_POSITION,REG_TILT_POSITION);
    AXIS_CONFIG *cfg = GetAxisCfg(axis);
    uint16_t addr = (uint16_t)cfg->POSITION;

    /* 5) mm → register value (scale ×100) */
    int16_t val = (int16_t)FloatToReg(target_mm, 100.0F);

    /* 6) Create proper array */
    /* LOW word first */
    uint16_t data[2];
    data[0] = (uint16_t)(val & 0xFFFF);  // first register = position
    /* HIGH word second */
    data[1] = (uint16_t)((val >> 16) & 0xFFFF); // second register = 0 (as you want)

    /* 7) Send Modbus write frame */
    (void)MODBUS_WriteHolding(modbus_cfg.UNIT_ID, addr, data, 2, rx);

    printf("[MOVE OK] Axis %u -> Target: %.2f mm (Reg 0x%X)\n",
           axis, target_mm, addr);
}
/*----------------------------------------------------------
 * Set Velocity
 *----------------------------------------------------------*/
void Set_Velocity(Axis_t axis, float vel)
{
    uint8_t rx[256];
    //float vmax = Compute_MaxVelocity();

    // if (vel > vmax)
    // {
    //     printf("[WARN] Requested velocity %.2f exceeds max %.2f  Clamped.\n",
    //             vel, vmax);
    //     vel = vmax;
    // }

    //uint16_t addr = GetRegisterAddress(axis, REG_PAN_VELOCITY, REG_TILT_VELOCITY);
    AXIS_CONFIG *cfg = GetAxisCfg(axis);
    uint16_t addr = (uint16_t)cfg->VELOCITY;
    uint16_t val  = FloatToReg(vel, 1.0F);
    /* 6) Create proper array */
    uint16_t data[2];
    data[0] = val;  // first register = position
    data[1] = 0;    // second register = 0 (as you want)
    MODBUS_WriteHolding(modbus_cfg.UNIT_ID, addr, data,2,rx);

    printf("Axis %u: Set Velocity = %.2f mm/s\n", axis, vel);
    VerifyParameterWrite(addr);
}
/*----------------------------------------------------------
 * Set Acceleration
 *----------------------------------------------------------*/
void Set_Acceleration(Axis_t axis, float accel)
{
    uint8_t rx[256];
    // float amax = Compute_MaxAcceleration();

    // if (accel > amax)
    // {
    //     printf("[WARN] Requested acceleration %.2f exceeds max %.2f  Clamped.\n",
    //             accel, amax);
    //     accel = amax;
    // }

    //uint16_t addr = GetRegisterAddress(axis, REG_PAN_ACCEL, REG_TILT_ACCEL);
    AXIS_CONFIG *cfg = GetAxisCfg(axis);
    uint16_t addr = (uint16_t)cfg->ACCEL;
    uint16_t val  = FloatToReg(accel, 1.0F);
    /* 6) Create proper array */
    uint16_t data[2];
    data[0] = val;  // first register = position
    data[1] = 0;    // second register = 0 (as you want)
    (void)MODBUS_WriteHolding(modbus_cfg.UNIT_ID, addr, data,2,rx);

    printf("Axis %u: Set Accel = %.2f mm/s²\n", axis, accel);
    VerifyParameterWrite(addr);
}


/*----------------------------------------------------------
 * Set Deceleration
 *----------------------------------------------------------*/
void Set_Deceleration(Axis_t axis, float decel)
{
    uint8_t rx[256];
    //uint16_t addr = GetRegisterAddress(axis, REG_PAN_DECEL, REG_TILT_DECEL);
    AXIS_CONFIG *cfg = GetAxisCfg(axis);
    uint16_t addr = (uint16_t)cfg->DECEL;
    uint16_t val = FloatToReg(decel, 1.0F);
    /* 6) Create proper array */
    uint16_t data[2];
    data[0] = val;  // first register = position
    data[1] = 0;    // second register = 0 (as you want)

    (void)MODBUS_WriteHolding(modbus_cfg.UNIT_ID, addr, data,2,rx);
    printf("Axis %u: Set Decel = %.2f (Reg 0x%X)\n", axis, decel, addr);

    VerifyParameterWrite(addr);
}

/*----------------------------------------------------------
 * Set Home Offset
 *----------------------------------------------------------*/
void Set_HomeOffset(Axis_t axis, float offset)
{
    uint8_t rx[256];
    //uint16_t addr = GetRegisterAddress(axis, REG_PAN_HOME_OFFSET, REG_TILT_HOME_OFFSET);
    AXIS_CONFIG *cfg = GetAxisCfg(axis);
    uint16_t addr = (uint16_t)cfg->HOME_OFFSET;
    uint16_t val = FloatToReg(offset, 100.0F);
    /* 6) Create proper array */
    uint16_t data[2];
    data[0] = val;  // first register = position
    data[1] = 0;    // second register = 0 (as you want)

    (void)MODBUS_WriteHolding(modbus_cfg.UNIT_ID, addr, data,2,rx);
    printf("Axis %u: Set HomeOffset = %.2f (Reg 0x%X)\n", axis, offset, addr);

    VerifyParameterWrite(addr);
}

/*----------------------------------------------------------
 * Set Degree Position
 *----------------------------------------------------------*/
//Positive degree position move
 void Set_DegPosition_Positive(Axis_t axis, float deg_pos)
{
    /* 1) Read current position in degrees */
    uint8_t rx[256];
    // float curr_deg = 0.0f;
    // if (Read_Position_Deg(axis, &curr_deg) != 0)
    // {
    //     printf("[ERROR] Failed to read current position (Drive disconnected?)\n");
    //     return;
    // }
    // printf("Current Position: %.2f°, Target: %.2f°\n", curr_deg, deg_pos);
    //deg_pos+=curr_deg; /* Make it relative move */
    /* 2) Software Limit Check */
    // if (!Check_SoftwareLimit(axis, deg_pos))
    // {
    //     printf("[SOFT LIMIT BLOCK] Axis %u: Target %.2f° is outside allowed limits.\n",
    //            axis, deg_pos);
    //     return;  /* Do NOT write to drive */
    // }
    /* 3) Select correct axis register */
    //uint16_t addr = GetRegisterAddress(axis, REG_PAN_DEG_POS, REG_TILT_DEG_POS);
    AXIS_CONFIG *cfg = GetAxisCfg(axis);
    uint16_t addr = (uint16_t)cfg->DEG_POS;

    /* 4) Convert degree to register value (×100 scaling) */
    int16_t val = (int16_t)FloatToReg(deg_pos, 100.0F);
    /* 6) Create proper array */
    uint16_t data[2];
    data[0] = (uint16_t)val;  // first register = position
    data[1] = 0;    // second register = 0 (as you want)
    /* 5) Write to modbus register */
    (void)MODBUS_WriteHolding(modbus_cfg.UNIT_ID, addr, data,2,rx);

    printf("[MOVE OK] Axis %u: Set DegPosition = %.2f° (Reg 0x%X)\n",
           axis, deg_pos, addr);
}
//Negative degree position move
 void Set_DegPosition_Negative(Axis_t axis, float deg_pos)
{
    /* 1) Read current position in degrees */
    uint8_t rx[256];
    // float curr_deg = 0.0f;
    // if (Read_Position_Deg(axis, &curr_deg) != 0)
    // {
    //     printf("[ERROR] Failed to read current position (Drive disconnected?)\n");
    //     return;
    // }
    // printf("Current Position: %.2f°, Target: %.2f°\n", curr_deg, deg_pos);
    //deg_pos+=curr_deg; /* Make it relative move */
    /* 2) Software Limit Check */
    // if (!Check_SoftwareLimit(axis, deg_pos))
    // {
    //     printf("[SOFT LIMIT BLOCK] Axis %u: Target %.2f° is outside allowed limits.\n",
    //            axis, deg_pos);
    //     return;  /* Do NOT write to drive */
    // }
    /* 3) Select correct axis register */
    //uint16_t addr = GetRegisterAddress(axis, REG_PAN_DEG_POS, REG_TILT_DEG_POS);
    AXIS_CONFIG *cfg = GetAxisCfg(axis);
    uint16_t addr = (uint16_t)cfg->DEG_POS;

    /* 4) Convert degree to register value (×100 scaling) */
    int16_t val = (int16_t)FloatToReg(deg_pos, 100.0F);
    /* 6) Create proper array */
    uint16_t data[2];
    data[0] = (uint16_t)(val & 0xFFFF);  // first register = position
    data[1] = (uint16_t)((val >> 16) & 0xFFFF);  
    /* 5) Write to modbus register */
    (void)MODBUS_WriteHolding(modbus_cfg.UNIT_ID, addr, data,2,rx);

    printf("[MOVE OK] Axis %u: Set DegPosition = %.2f° (Reg 0x%X)\n",
           axis, deg_pos, addr);
}
/*----------------------------------------------------------
 * Write Multiple Registers (0x10)
 *   position, velocity, acceleration, deceleration
 *----------------------------------------------------------*/
void Set_MotionParameters(Axis_t axis, float pos, float vel, float accel, float decel)
{
    uint16_t start_addr;
    uint16_t reg_data[4U]; /* 4 registers */
    AXIS_CONFIG *cfg = GetAxisCfg(axis);

    if (axis == AXIS_PAN)
    {
        start_addr = cfg->POSITION;
    }
    else
    {
        start_addr = cfg->POSITION;
    }

    reg_data[0] = FloatToReg(pos,   100.0F);
    reg_data[1] = FloatToReg(vel,   1.0F);
    reg_data[2] = FloatToReg(accel, 1.0F);
    reg_data[3] = FloatToReg(decel, 1.0F);

    (void)MODBUS_WriteMultiple(modbus_cfg.UNIT_ID, start_addr, 4U, reg_data);

    printf("Axis %u: Multi-param write @0x%X Pos=%.2f Vel=%.2f Acc=%.2f Dec=%.2f\n",
           axis, start_addr, pos, vel, accel, decel);

    VerifyParameterWrite(start_addr);
}
