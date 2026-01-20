#ifndef INI_H
#define INI_H

#include <stdint.h>

typedef struct {
    char DRIVE_IP_ADDR[64];
    int  DRIVE_PORT_UDP;
    char LOCAL_BIND_IP[64];
    int  LOCAL_BIND_PORT;

    char JSON_WCS_IP[64];
    int  JSON_WCS_PORT;
    char JSON_LCU_IP[64];
    int  JSON_LCU_PORT;

    char MQTT_BROKER_IP[64];
    int  MQTT_BROKER_PORT;
    char MQTT_CLIENT_ID[64];

    char MQTT_TOPIC_HEARTBEAT[64];
    char MQTT_TOPIC_TELEMETRY[64];
} NETWORK_CONFIG;

typedef struct {
    int UNIT_ID;
    int FUNC_READ_HOLDING;
    int FUNC_READ_INPUT;
    int FUNC_WRITE_SINGLE;
    int FUNC_WRITE_MULTIPLE;
    int MAX_RESPONSE_BYTES;
    int TIMEOUT_SEC;
} MODBUS_CONFIG;

typedef struct {
    char NAME[32];
    int AXIS_ID;

    // Holding
    int POSITION;
    int VELOCITY;
    int ACCEL;
    int DECEL;
    int HOME_OFFSET;
    int DEG_POS;

    // Input
    int VERSION;
    int REVISION;
    int RELEASE_DATE;
    int POS_DEG;
    int ABS_POSITION;
    int POS_MM;
    int RPM;
    int ACTUAL_CURRENT;
    int IO_STATUS;
    int SYSTEM_STATUS;
    int DCBUS_VOLT_CMD;
    int FAULT_STATUS;

    // Limits
    float LIMIT_MIN_DEG;
    float LIMIT_MAX_DEG;
    float LIMIT_MIN_MM;
    float LIMIT_MAX_MM;

} AXIS_CONFIG;

typedef struct {
    int CMD_SOLENOID;
    int CMD_HALT;
    int CMD_EMG_STOP;
    int CMD_DISABLE;
    int CMD_ENABLE;
    int CMD_RESET;
    int CMD_POS_MOVE;
    int CMD_HOME_MOVE_DEG;
    int CMD_VEL_FWD;
    int CMD_VEL_REV;
    int CMD_POS_MOVE_DEG;
} COMMAND_REGS;

typedef struct {
    int SHORT_CKT;
    int SYSTEM_HEALTHY;
    int RATED_CURRENT_FAULT;
    int OVER_TEMP;
    int OVER_VOLT;
    int UNDER_VOLT;
    int MOTION_ERROR;
    int DRIVE_DISABLE;
    int EEPROM_ERROR;
    int COMMUTATION_ERROR;
    int LOCK_ROTOR;
    int EMERGENCY_ERROR;
    int COMMAND_ERROR;
    int MOTION_COMPLETE;
} FAULT_BITS_CONFIG;

typedef struct {
    int MAX_RPM;
    float DPMR_MM;
    float ACCEL_FACTOR;
    float RATED_CURRENT;
    float PEAK_CURRENT;
    float CURRENT_SHUTDOWN_LIMIT;
} MOTOR_CONFIG;

/// GLOBAL OBJECTS (access everywhere)
extern NETWORK_CONFIG net_cfg;
extern MODBUS_CONFIG modbus_cfg;
extern AXIS_CONFIG axis1_cfg;
extern AXIS_CONFIG axis2_cfg;
extern COMMAND_REGS cmd_regs;
extern FAULT_BITS_CONFIG fault_cfg;
extern MOTOR_CONFIG motor_cfg;

/// Loader function
int ini_load(const char *filename);

#endif
