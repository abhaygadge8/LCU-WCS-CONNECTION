#ifndef COMMAND_PARSER_H
#define COMMAND_PARSER_H

#include <stdbool.h>

/* Command type */
typedef enum
{
    CMD_INVALID = 0,
    CMD_ENABLE,
    CMD_DISABLE,
    CMD_HALT,
    CMD_RESET,
    CMD_ESTOP,
    CMD_SET_POS,
    CMD_SET_ANGLE,
    CMD_MOVE,
    CMD_MOVE_DEG,
    CMD_VELOCITY_FWD,
    CMD_VELOCITY_REV,
    CMD_SOLENOID
} CommandType_t;

/* Parsed command (axis as STRING) */
typedef struct
{
    /* Envelope */
    int  v;
    char id[64];
    char type[16];
    char name[32];

    /* Body */
    CommandType_t cmd;
    char axis[8];          /* "PAN" | "TILT" | "BOTH" */

    float target_deg;
    float target_pos;
    float velocity;
    float accel;
    float decel;
} ParsedCommand_t;

/* Parse JSON payload (after TCP framing) */
bool Parse_Command_JSON(const char *json, ParsedCommand_t *out);

#endif
