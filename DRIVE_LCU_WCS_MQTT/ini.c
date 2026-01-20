/* 
ini.c -- full INI loader for your LCU project
 *
 * Matches ini.h structure and your provided config.ini keys.
 *
 * Usage:
 *   if (ini_load("config.ini") != 0) {  handle error / }
 *   // then use global structs: net_cfg, modbus_cfg, axis1_cfg, axis2_cfg, ...
 */

#include "ini.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/* Global objects (defined in ini.h as extern) */
NETWORK_CONFIG net_cfg;
MODBUS_CONFIG modbus_cfg;
AXIS_CONFIG axis1_cfg;
AXIS_CONFIG axis2_cfg;
COMMAND_REGS cmd_regs;
FAULT_BITS_CONFIG fault_cfg;
MOTOR_CONFIG motor_cfg;

/* helper buffers */
static char current_section[64] = {0};

/* trim left and right in-place */
static void trim_lr(char *s)
{
    if (!s) return;
    /* left */
    char *p = s;
    while (*p && isspace((unsigned char)*p)) p++;
    if (p != s) memmove(s, p, strlen(p) + 1);

    /* right */
    size_t len = strlen(s);
    while (len > 0 && isspace((unsigned char)s[len-1])) { s[len-1] = 0; len--; }
}

/* strip inline comment markers ';' and '#' from value (first occurrence) */
static void strip_inline_comment(char *s)
{
    if (!s) return;
    char *p;
    /* if semicolon or hash appear, cut there */
    p = strchr(s, ';');
    if (p) { *p = 0; trim_lr(s); return; }
    p = strchr(s, '#');
    if (p) { *p = 0; trim_lr(s); return; }
}

/* safe copy helper */
static void safe_strcpy(char *dst, const char *src, size_t dst_size)
{
    if (!dst || !dst_size) return;
    if (!src) { dst[0] = 0; return; }
    strncpy(dst, src, dst_size - 1);
    dst[dst_size - 1] = 0;
}

/* initialise defaults to known safe values */
/* initialise defaults to known safe values */
static void init_defaults(void)
{
    memset(&net_cfg, 0, sizeof(net_cfg));
    memset(&modbus_cfg, 0, sizeof(modbus_cfg));
    memset(&axis1_cfg, 0, sizeof(axis1_cfg));
    memset(&axis2_cfg, 0, sizeof(axis2_cfg));
    memset(&cmd_regs, 0, sizeof(cmd_regs));
    memset(&fault_cfg, 0, sizeof(fault_cfg));
    memset(&motor_cfg, 0, sizeof(motor_cfg));

    /* ---------------- NETWORK ---------------- */
    safe_strcpy(net_cfg.DRIVE_IP_ADDR, "169.254.214.170", sizeof(net_cfg.DRIVE_IP_ADDR));
    net_cfg.DRIVE_PORT_UDP = 53011;
    safe_strcpy(net_cfg.LOCAL_BIND_IP, "169.254.214.171", sizeof(net_cfg.LOCAL_BIND_IP));
    net_cfg.LOCAL_BIND_PORT = 53010;
    safe_strcpy(net_cfg.JSON_WCS_IP, "239.10.10.10", sizeof(net_cfg.JSON_WCS_IP));
    net_cfg.JSON_WCS_PORT = 6000;
    safe_strcpy(net_cfg.JSON_LCU_IP, "169.254.214.171", sizeof(net_cfg.JSON_LCU_IP));
    net_cfg.JSON_LCU_PORT = 6001;

    /* ---------------- MQTT DEFAULTS ---------------- */
    safe_strcpy(net_cfg.MQTT_BROKER_IP, "169.254.214.100",sizeof(net_cfg.MQTT_BROKER_IP));
    net_cfg.MQTT_BROKER_PORT = 1883;

    safe_strcpy(net_cfg.MQTT_TOPIC_HEARTBEAT, "server/heartbeat",sizeof(net_cfg.MQTT_TOPIC_HEARTBEAT));

    safe_strcpy(net_cfg.MQTT_TOPIC_TELEMETRY, "server/telemetry",sizeof(net_cfg.MQTT_TOPIC_TELEMETRY));


    /* ---------------- MODBUS ---------------- */
    modbus_cfg.UNIT_ID = 1;
    modbus_cfg.FUNC_READ_HOLDING = 3;
    modbus_cfg.FUNC_READ_INPUT = 4;
    modbus_cfg.FUNC_WRITE_SINGLE = 6;
    modbus_cfg.FUNC_WRITE_MULTIPLE = 16;
    modbus_cfg.MAX_RESPONSE_BYTES = 260;
    modbus_cfg.TIMEOUT_SEC = 1;

    /* =====================================================
     * AXIS1 = PAN   (300 series registers)
     * ===================================================== */
    safe_strcpy(axis1_cfg.NAME, "PAN", sizeof(axis1_cfg.NAME));
    axis1_cfg.AXIS_ID = 1;

    axis1_cfg.POSITION = 282;
    axis1_cfg.VELOCITY = 284;
    axis1_cfg.ACCEL = 286;
    axis1_cfg.DECEL = 288;
    axis1_cfg.HOME_OFFSET = 310;
    axis1_cfg.DEG_POS = 314;

    axis1_cfg.VERSION = 350;
    axis1_cfg.REVISION = 352;
    axis1_cfg.RELEASE_DATE = 354;
    axis1_cfg.ABS_POSITION = 412;
    axis1_cfg.POS_DEG = 414;
    axis1_cfg.POS_MM = 416;
    axis1_cfg.RPM = 418;
    axis1_cfg.ACTUAL_CURRENT = 420;
    axis1_cfg.IO_STATUS = 422;
    axis1_cfg.SYSTEM_STATUS = 424;
    axis1_cfg.DCBUS_VOLT_CMD = 426;
    axis1_cfg.FAULT_STATUS = 424;

    axis1_cfg.LIMIT_MIN_DEG = -155.0f;
    axis1_cfg.LIMIT_MAX_DEG = 155.0f;
    axis1_cfg.LIMIT_MIN_MM = -100.0f;
    axis1_cfg.LIMIT_MAX_MM = 100.0f;

    /* =====================================================
     * AXIS2 = TILT  (800 series registers)
     * ===================================================== */
    safe_strcpy(axis2_cfg.NAME, "TILT", sizeof(axis2_cfg.NAME));
    axis2_cfg.AXIS_ID = 2;

    axis2_cfg.POSITION = 782;
    axis2_cfg.VELOCITY = 784;
    axis2_cfg.ACCEL = 786;
    axis2_cfg.DECEL = 788;
    axis2_cfg.HOME_OFFSET = 810;
    axis2_cfg.DEG_POS = 814;

    axis2_cfg.VERSION = 850;
    axis2_cfg.REVISION = 852;
    axis2_cfg.RELEASE_DATE = 854;
    axis2_cfg.ABS_POSITION = 912;
    axis2_cfg.POS_DEG = 914;
    axis2_cfg.POS_MM = 916;
    axis2_cfg.RPM = 918;
    axis2_cfg.ACTUAL_CURRENT = 920;
    axis2_cfg.IO_STATUS = 422;
    axis2_cfg.SYSTEM_STATUS = 924;
    axis2_cfg.DCBUS_VOLT_CMD = 926;
    axis2_cfg.FAULT_STATUS = 924;

    axis2_cfg.LIMIT_MIN_DEG = -20.0f;
    axis2_cfg.LIMIT_MAX_DEG = 70.0f;
    axis2_cfg.LIMIT_MIN_MM = -50.0f;
    axis2_cfg.LIMIT_MAX_MM = 50.0f;

    /* ---------------- COMMAND REGISTERS ---------------- */
    cmd_regs.CMD_SOLENOID = 439;
    cmd_regs.CMD_HALT = 445;
    cmd_regs.CMD_EMG_STOP = 446;
    cmd_regs.CMD_ENABLE = 448;
    cmd_regs.CMD_RESET = 449;
    cmd_regs.CMD_POS_MOVE = 451;
    cmd_regs.CMD_HOME_MOVE_DEG = 452;
    cmd_regs.CMD_VEL_FWD = 453;
    cmd_regs.CMD_VEL_REV = 454;
    cmd_regs.CMD_POS_MOVE_DEG = 455;

    /* ---------------- FAULT BITS ---------------- */
    fault_cfg.SHORT_CKT = 1;
    fault_cfg.SYSTEM_HEALTHY = 2;
    fault_cfg.RATED_CURRENT_FAULT = 4;
    fault_cfg.OVER_TEMP = 8;
    fault_cfg.OVER_VOLT = 16;
    fault_cfg.UNDER_VOLT = 32;
    fault_cfg.MOTION_ERROR = 64;
    fault_cfg.DRIVE_DISABLE = 128;
    fault_cfg.EEPROM_ERROR = 256;
    fault_cfg.COMMUTATION_ERROR = 512;
    fault_cfg.LOCK_ROTOR = 1024;
    fault_cfg.EMERGENCY_ERROR = 2048;
    fault_cfg.COMMAND_ERROR = 16384;
    fault_cfg.MOTION_COMPLETE = 32768;

    /* ---------------- MOTOR ---------------- */
    motor_cfg.MAX_RPM = 3000;
    motor_cfg.DPMR_MM = 5.0f;
    motor_cfg.ACCEL_FACTOR = 1.5f;
    motor_cfg.RATED_CURRENT = 5.0f;
    motor_cfg.PEAK_CURRENT = 10.0f;
    motor_cfg.CURRENT_SHUTDOWN_LIMIT = 10.0f;
}

/* case-sensitive match helper */
static int match(const char *section, const char *key, const char *s, const char *k)
{
    return (section && key && s && k && strcmp(section, s) == 0 && strcmp(key, k) == 0);
}

/* parse an integer and assign via pointer */
static void assign_int(int *dst, const char *val)
{
    if (!dst || !val) return;
    *dst = atoi(val);
}

/* parse float */
static void assign_float(float *dst, const char *val)
{
    if (!dst || !val) return;
    *dst = (float)atof(val);
}

/* parse string */
static void assign_str(char *dst, size_t dstlen, const char *val)
{
    if (!dst || !val) return;
    strncpy(dst, val, dstlen - 1);
    dst[dstlen - 1] = 0;
}

/* main loader */
int ini_load(const char *filename)
{
    FILE *f = fopen(filename, "r");
    if (!f) return -1;

    init_defaults();

    char line[512];
    while (fgets(line, sizeof(line), f))
    {
        /* remove leading/trailing spaces */
        trim_lr(line);
        if (line[0] == 0) continue;

        /* skip full-line comments */
        if (line[0] == ';' || line[0] == '#') continue;

        /* section header? */
        if (line[0] == '[')
        {
            /* read inside brackets up to ']' */
            char secname[64] = {0};
            if (sscanf(line, "[%63[^]]", secname) == 1)
            {
                trim_lr(secname);
                safe_strcpy(current_section, secname, sizeof(current_section));
            }
            continue;
        }

        /* key = value parsing (split on first '=') */
        char *eq = strchr(line, '=');
        if (!eq) continue;
        *eq = 0;
        char keybuf[128] = {0};
        char valbuf[256] = {0};
        safe_strcpy(keybuf, line, sizeof(keybuf));
        safe_strcpy(valbuf, eq + 1, sizeof(valbuf));
        trim_lr(keybuf);
        trim_lr(valbuf);
        strip_inline_comment(valbuf);  /* remove trailing comment if any */

        /* now match keys */
        /* ---------------- NETWORK ---------------- */
        if (match(current_section, keybuf, "NETWORK", "DRIVE_IP_ADDR"))
            assign_str(net_cfg.DRIVE_IP_ADDR, sizeof(net_cfg.DRIVE_IP_ADDR), valbuf);
        else if (match(current_section, keybuf, "NETWORK", "DRIVE_PORT_UDP"))
            assign_int(&net_cfg.DRIVE_PORT_UDP, valbuf);
        else if (match(current_section, keybuf, "NETWORK", "LOCAL_BIND_IP"))
            assign_str(net_cfg.LOCAL_BIND_IP, sizeof(net_cfg.LOCAL_BIND_IP), valbuf);
        else if (match(current_section, keybuf, "NETWORK", "LOCAL_BIND_PORT"))
            assign_int(&net_cfg.LOCAL_BIND_PORT, valbuf);
        else if (match(current_section, keybuf, "NETWORK", "JSON_WCS_IP"))
            assign_str(net_cfg.JSON_WCS_IP, sizeof(net_cfg.JSON_WCS_IP), valbuf);
        else if (match(current_section, keybuf, "NETWORK", "JSON_WCS_PORT"))
            assign_int(&net_cfg.JSON_WCS_PORT, valbuf);
        else if (match(current_section, keybuf, "NETWORK", "JSON_LCU_IP"))
            assign_str(net_cfg.JSON_LCU_IP, sizeof(net_cfg.JSON_LCU_IP), valbuf);
        else if (match(current_section, keybuf, "NETWORK", "JSON_LCU_PORT"))
            assign_int(&net_cfg.JSON_LCU_PORT, valbuf);
        /* ---------------- MQTT ---------------- */
        else if (match(current_section, keybuf, "MQTT", "MQTT_BROKER_IP"))
            assign_str(net_cfg.MQTT_BROKER_IP,sizeof(net_cfg.MQTT_BROKER_IP),valbuf);
        else if (match(current_section, keybuf, "MQTT", "MQTT_BROKER_PORT"))
            assign_int(&net_cfg.MQTT_BROKER_PORT, valbuf);
        else if (match(current_section, keybuf, "MQTT", "MQTT_TOPIC_HEARTBEAT"))
            assign_str(net_cfg.MQTT_TOPIC_HEARTBEAT,sizeof(net_cfg.MQTT_TOPIC_HEARTBEAT),valbuf);
        else if (match(current_section, keybuf, "MQTT", "MQTT_TOPIC_TELEMETRY"))
            assign_str(net_cfg.MQTT_TOPIC_TELEMETRY,sizeof(net_cfg.MQTT_TOPIC_TELEMETRY),valbuf);
        else if (match(current_section, keybuf, "MQTT", "MQTT_CLIENT_ID"))
            assign_str(net_cfg.MQTT_CLIENT_ID,sizeof(net_cfg.MQTT_CLIENT_ID),valbuf);


        /* ---------------- MODBUS ----------------- */
        else if (match(current_section, keybuf, "MODBUS", "UNIT_ID"))
            assign_int(&modbus_cfg.UNIT_ID, valbuf);
        else if (match(current_section, keybuf, "MODBUS", "FUNC_READ_HOLDING"))
            assign_int(&modbus_cfg.FUNC_READ_HOLDING, valbuf);
        else if (match(current_section, keybuf, "MODBUS", "FUNC_READ_INPUT"))
            assign_int(&modbus_cfg.FUNC_READ_INPUT, valbuf);
        else if (match(current_section, keybuf, "MODBUS", "FUNC_WRITE_SINGLE"))
            assign_int(&modbus_cfg.FUNC_WRITE_SINGLE, valbuf);
        else if (match(current_section, keybuf, "MODBUS", "FUNC_WRITE_MULTIPLE"))
            assign_int(&modbus_cfg.FUNC_WRITE_MULTIPLE, valbuf);
        else if (match(current_section, keybuf, "MODBUS", "MAX_RESPONSE_BYTES"))
            assign_int(&modbus_cfg.MAX_RESPONSE_BYTES, valbuf);
        else if (match(current_section, keybuf, "MODBUS", "TIMEOUT_SEC"))
            assign_int(&modbus_cfg.TIMEOUT_SEC, valbuf);

        /* ---------------- AXIS1 (TILT) ------------- */
        else if (match(current_section, keybuf, "AXIS1", "NAME"))
            assign_str(axis1_cfg.NAME, sizeof(axis1_cfg.NAME), valbuf);
        else if (match(current_section, keybuf, "AXIS1", "AXIS_ID"))
            assign_int(&axis1_cfg.AXIS_ID, valbuf);

        else if (match(current_section, keybuf, "AXIS1", "POSITION"))
            assign_int(&axis1_cfg.POSITION, valbuf);
        else if (match(current_section, keybuf, "AXIS1", "VELOCITY"))
            assign_int(&axis1_cfg.VELOCITY, valbuf);
        else if (match(current_section, keybuf, "AXIS1", "ACCEL"))
            assign_int(&axis1_cfg.ACCEL, valbuf);
        else if (match(current_section, keybuf, "AXIS1", "DECEL"))
            assign_int(&axis1_cfg.DECEL, valbuf);
        else if (match(current_section, keybuf, "AXIS1", "HOME_OFFSET"))
            assign_int(&axis1_cfg.HOME_OFFSET, valbuf);
        else if (match(current_section, keybuf, "AXIS1", "DEG_POS"))
            assign_int(&axis1_cfg.DEG_POS, valbuf);

        else if (match(current_section, keybuf, "AXIS1", "VERSION"))
            assign_int(&axis1_cfg.VERSION, valbuf);
        else if (match(current_section, keybuf, "AXIS1", "REVISION"))
            assign_int(&axis1_cfg.REVISION, valbuf);
        else if (match(current_section, keybuf, "AXIS1", "RELEASE_DATE"))
            assign_int(&axis1_cfg.RELEASE_DATE, valbuf);
        else if( match(current_section, keybuf, "AXIS1", "ABS_POSITION"))
            assign_int(&axis1_cfg.ABS_POSITION, valbuf);
        else if (match(current_section, keybuf, "AXIS1", "POS_DEG"))
            assign_int(&axis1_cfg.POS_DEG, valbuf);
        else if (match(current_section, keybuf, "AXIS1", "POS_MM"))
            assign_int(&axis1_cfg.POS_MM, valbuf);
        else if (match(current_section, keybuf, "AXIS1", "RPM"))
            assign_int(&axis1_cfg.RPM, valbuf);
        else if (match(current_section, keybuf, "AXIS1", "ACTUAL_CURRENT"))
            assign_int(&axis1_cfg.ACTUAL_CURRENT, valbuf);
        else if (match(current_section, keybuf, "AXIS1", "IO_STATUS"))
            assign_int(&axis1_cfg.IO_STATUS, valbuf);
        else if (match(current_section, keybuf, "AXIS1", "SYSTEM_STATUS"))
            assign_int(&axis1_cfg.SYSTEM_STATUS, valbuf);
        else if (match(current_section, keybuf, "AXIS1", "DCBUS_VOLT_CMD")) /* second occurrence -> store in DCBUS_VOLT2 */
            assign_int(&axis1_cfg.DCBUS_VOLT_CMD, valbuf);
        else if (match(current_section, keybuf, "AXIS1", "FAULT_STATUS")) /* second occurrence -> store in DCBUS_VOLT2 */
            assign_int(&axis1_cfg.FAULT_STATUS, valbuf);

        else if (match(current_section, keybuf, "AXIS1", "LIMIT_MIN_DEG"))
            assign_float(&axis1_cfg.LIMIT_MIN_DEG, valbuf);
        else if (match(current_section, keybuf, "AXIS1", "LIMIT_MAX_DEG"))
            assign_float(&axis1_cfg.LIMIT_MAX_DEG, valbuf);
        else if (match(current_section, keybuf, "AXIS1", "LIMIT_MIN_MM"))
            assign_float(&axis1_cfg.LIMIT_MIN_MM, valbuf);
        else if (match(current_section, keybuf, "AXIS1", "LIMIT_MAX_MM"))
            assign_float(&axis1_cfg.LIMIT_MAX_MM, valbuf);

        /* --------------- AXIS2 (PAN) ------------------ */
        else if (match(current_section, keybuf, "AXIS2", "NAME"))
            assign_str(axis2_cfg.NAME, sizeof(axis2_cfg.NAME), valbuf);
        else if (match(current_section, keybuf, "AXIS2", "AXIS_ID"))
            assign_int(&axis2_cfg.AXIS_ID, valbuf);

        else if (match(current_section, keybuf, "AXIS2", "POSITION"))
            assign_int(&axis2_cfg.POSITION, valbuf);
        else if (match(current_section, keybuf, "AXIS2", "VELOCITY"))
            assign_int(&axis2_cfg.VELOCITY, valbuf);
        else if (match(current_section, keybuf, "AXIS2", "ACCEL"))
            assign_int(&axis2_cfg.ACCEL, valbuf);
        else if (match(current_section, keybuf, "AXIS2", "DECEL"))
            assign_int(&axis2_cfg.DECEL, valbuf);
        else if (match(current_section, keybuf, "AXIS2", "HOME_OFFSET"))
            assign_int(&axis2_cfg.HOME_OFFSET, valbuf);
        else if (match(current_section, keybuf, "AXIS2", "DEG_POS"))
            assign_int(&axis2_cfg.DEG_POS, valbuf);

        else if (match(current_section, keybuf, "AXIS2", "VERSION"))
            assign_int(&axis2_cfg.VERSION, valbuf);
        else if (match(current_section, keybuf, "AXIS2", "REVISION"))
            assign_int(&axis2_cfg.REVISION, valbuf);
        else if (match(current_section, keybuf, "AXIS2", "RELEASE_DATE"))
            assign_int(&axis2_cfg.RELEASE_DATE, valbuf);
        else if (match(current_section, keybuf, "AXIS2", "POS_DEG"))
            assign_int(&axis2_cfg.POS_DEG, valbuf);
        else if (match(current_section, keybuf, "AXIS2", "POS_MM"))
            assign_int(&axis2_cfg.POS_MM, valbuf);
        else if (match(current_section, keybuf, "AXIS2", "RPM"))
            assign_int(&axis2_cfg.RPM, valbuf);
        else if (match(current_section, keybuf, "AXIS2", "ACTUAL_CURRENT"))
            assign_int(&axis2_cfg.ACTUAL_CURRENT, valbuf);
        else if (match(current_section, keybuf, "AXIS2", "IO_STATUS"))
            assign_int(&axis2_cfg.IO_STATUS, valbuf);
        else if (match(current_section, keybuf, "AXIS2", "SYSTEM_STATUS"))
            assign_int(&axis2_cfg.SYSTEM_STATUS, valbuf);
        else if (match(current_section, keybuf, "AXIS2", "DCBUS_VOLT")) /* second occurrence -> DCBUS_VOLT2 */
            assign_int(&axis2_cfg.DCBUS_VOLT_CMD, valbuf);
        else if (match(current_section, keybuf, "AXIS2", "FAULT_STATUS")) /* second occurrence -> DCBUS_VOLT2 */
            assign_int(&axis2_cfg.FAULT_STATUS, valbuf);

        else if (match(current_section, keybuf, "AXIS2", "LIMIT_MIN_DEG"))
            assign_float(&axis2_cfg.LIMIT_MIN_DEG, valbuf);
        else if (match(current_section, keybuf, "AXIS2", "LIMIT_MAX_DEG"))
            assign_float(&axis2_cfg.LIMIT_MAX_DEG, valbuf);
        else if (match(current_section, keybuf, "AXIS2", "LIMIT_MIN_MM"))
            assign_float(&axis2_cfg.LIMIT_MIN_MM, valbuf);
        else if (match(current_section, keybuf, "AXIS2", "LIMIT_MAX_MM"))
            assign_float(&axis2_cfg.LIMIT_MAX_MM, valbuf);

        /* --------------- COMMAND_REGISTERS ------------------ */
        else if (match(current_section, keybuf, "COMMAND_REGISTERS", "CMD_SOLENOID"))
            assign_int(&cmd_regs.CMD_SOLENOID, valbuf);
        else if (match(current_section, keybuf, "COMMAND_REGISTERS", "CMD_HALT"))
            assign_int(&cmd_regs.CMD_HALT, valbuf);
        else if (match(current_section, keybuf, "COMMAND_REGISTERS", "CMD_EMG_STOP"))
            assign_int(&cmd_regs.CMD_EMG_STOP, valbuf);
        else if (match(current_section, keybuf, "COMMAND_REGISTERS", "CMD_ENABLE"))
            assign_int(&cmd_regs.CMD_ENABLE, valbuf);
        else if (match(current_section, keybuf, "COMMAND_REGISTERS", "CMD_DISABLE"))
            assign_int(&cmd_regs.CMD_DISABLE, valbuf);
        else if (match(current_section, keybuf, "COMMAND_REGISTERS", "CMD_RESET"))
            assign_int(&cmd_regs.CMD_RESET, valbuf);
        else if (match(current_section, keybuf, "COMMAND_REGISTERS", "CMD_POS_MOVE"))
            assign_int(&cmd_regs.CMD_POS_MOVE, valbuf);
        else if (match(current_section, keybuf, "COMMAND_REGISTERS", "CMD_HOME_MOVE_DEG"))
            assign_int(&cmd_regs.CMD_HOME_MOVE_DEG, valbuf);
        else if (match(current_section, keybuf, "COMMAND_REGISTERS", "CMD_VEL_FWD"))
            assign_int(&cmd_regs.CMD_VEL_FWD, valbuf);
        else if (match(current_section, keybuf, "COMMAND_REGISTERS", "CMD_VEL_REV"))
            assign_int(&cmd_regs.CMD_VEL_REV, valbuf);
        else if (match(current_section, keybuf, "COMMAND_REGISTERS", "CMD_POS_MOVE_DEG"))
            assign_int(&cmd_regs.CMD_POS_MOVE_DEG, valbuf);

        /* ---------------- FAULT_BITS -------------------- */
        else if (match(current_section, keybuf, "FAULT_BITS", "SHORT_CKT"))
            assign_int(&fault_cfg.SHORT_CKT, valbuf);
        else if (match(current_section, keybuf, "FAULT_BITS", "SYSTEM_HEALTHY"))
            assign_int(&fault_cfg.SYSTEM_HEALTHY, valbuf);
        else if (match(current_section, keybuf, "FAULT_BITS", "RATED_CURRENT_FAULT"))
            assign_int(&fault_cfg.RATED_CURRENT_FAULT, valbuf);
        else if (match(current_section, keybuf, "FAULT_BITS", "OVER_TEMP"))
            assign_int(&fault_cfg.OVER_TEMP, valbuf);
        else if (match(current_section, keybuf, "FAULT_BITS", "OVER_VOLT"))
            assign_int(&fault_cfg.OVER_VOLT, valbuf);
        else if (match(current_section, keybuf, "FAULT_BITS", "UNDER_VOLT"))
            assign_int(&fault_cfg.UNDER_VOLT, valbuf);
        else if (match(current_section, keybuf, "FAULT_BITS", "MOTION_ERROR"))
            assign_int(&fault_cfg.MOTION_ERROR, valbuf);
        else if (match(current_section, keybuf, "FAULT_BITS", "DRIVE_DISABLE"))
            assign_int(&fault_cfg.DRIVE_DISABLE, valbuf);
        else if (match(current_section, keybuf, "FAULT_BITS", "EEPROM_ERROR"))
            assign_int(&fault_cfg.EEPROM_ERROR, valbuf);
        else if (match(current_section, keybuf, "FAULT_BITS", "COMMUTATION_ERROR"))
            assign_int(&fault_cfg.COMMUTATION_ERROR, valbuf);
        else if (match(current_section, keybuf, "FAULT_BITS", "LOCK_ROTOR"))
            assign_int(&fault_cfg.LOCK_ROTOR, valbuf);
        else if (match(current_section, keybuf, "FAULT_BITS", "EMERGENCY_ERROR"))
            assign_int(&fault_cfg.EMERGENCY_ERROR, valbuf);
        else if (match(current_section, keybuf, "FAULT_BITS", "COMMAND_ERROR"))
            assign_int(&fault_cfg.COMMAND_ERROR, valbuf);
        else if (match(current_section, keybuf, "FAULT_BITS", "MOTION_COMPLETE"))
            assign_int(&fault_cfg.MOTION_COMPLETE, valbuf);

        /* ---------------- MOTOR -------------------- */
        else if (match(current_section, keybuf, "MOTOR", "MAX_RPM"))
            assign_int(&motor_cfg.MAX_RPM, valbuf);
        else if (match(current_section, keybuf, "MOTOR", "DPMR_MM"))
            assign_float(&motor_cfg.DPMR_MM, valbuf);
        else if (match(current_section, keybuf, "MOTOR", "ACCEL_FACTOR"))
            assign_float(&motor_cfg.ACCEL_FACTOR, valbuf);
        else if (match(current_section, keybuf, "MOTOR", "RATED_CURRENT"))
            assign_float(&motor_cfg.RATED_CURRENT, valbuf);
        else if (match(current_section, keybuf, "MOTOR", "PEAK_CURRENT"))
            assign_float(&motor_cfg.PEAK_CURRENT, valbuf);
        else if (match(current_section, keybuf, "MOTOR", "CURRENT_SHUTDOWN_LIMIT"))
            assign_float(&motor_cfg.CURRENT_SHUTDOWN_LIMIT, valbuf);

        /* else: unknown key -> ignore silently (or log if needed) */
    }

    fclose(f);
    return 0;
}
