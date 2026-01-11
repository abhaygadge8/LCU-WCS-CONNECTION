#include "command_handler.h"
#include "command_parser.h"
#include "axis_helper.h"

#include "lcu_comm.h"
#include "mqtt_client.h"
#include "drive_command.h"
#include "drive_parameters.h"
#include"cJSON.h"

#include <string.h>
#include <stdio.h>
#include <stdint.h>
/*----------------------------------------------------------
 * Execute command on ONE axis
 *----------------------------------------------------------*/
static void execute_on_axis(Axis_t axis, const ParsedCommand_t *cmd)
{
    switch (cmd->cmd)
    {
    case CMD_ENABLE:
        CMD_Enable(axis);
        break;

    case CMD_DISABLE:
        CMD_Disable(axis);
        break;

    case CMD_RESET:
        CMD_Reset(axis);
        break;

    case CMD_ESTOP:
        CMD_EStop(axis);
        break;

    case CMD_SET_ANGLE:
        Set_Velocity(axis,     cmd->velocity);
        Set_Acceleration(axis, cmd->accel);
        Set_Deceleration(axis, cmd->decel);
        if(cmd->target_deg>=0){
            Set_DegPosition_Positive(axis,  cmd->target_deg);
        }else{
            Set_DegPosition_Negative(axis,  cmd->target_deg);
        }
        //CMD_PositionMove_Deg(axis);
        break;

    case CMD_SET_POS:
        Set_Velocity(axis,     cmd->velocity);
        Set_Acceleration(axis, cmd->accel);
        Set_Deceleration(axis, cmd->decel);
        if(cmd->target_pos >=0){
            Set_Position_Positive(axis,  cmd->target_pos);
        }else{
            Set_Position_Negative(axis,  cmd->target_pos);
        }
        //CMD_PositionMove(axis);
        break;

    case CMD_MOVE:
        CMD_PositionMove(axis);
        break;

    case CMD_MOVE_DEG:
        CMD_PositionMove_Deg(axis);
        break;
    
    case CMD_VELOCITY_FWD:
        CMD_VelocityFwd(axis); 
        break;

    case CMD_VELOCITY_REV:
        CMD_VelocityRev(axis);
        break;

    case CMD_HALT:
        CMD_Halt(axis);
        break;

    case CMD_SOLENOID:
        CMD_Solenoid(axis);
        break;

    default:
        break;
    }
}

/*----------------------------------------------------------
 * Send ACK via MQTT
 *----------------------------------------------------------*/
static void send_ack(const ParsedCommand_t *cmd,
                     const char *code,
                     const char *msg)
{
    /* Root object */
    cJSON *root = cJSON_CreateObject();

    cJSON_AddNumberToObject(root, "v", 1);
    cJSON_AddStringToObject(root, "id", cmd->id);
    cJSON_AddStringToObject(root, "type", "Reply");
    cJSON_AddStringToObject(root, "name", cmd->name);
    cJSON_AddStringToObject(root, "src", "lcu");

    /* body */
    cJSON *body = cJSON_AddObjectToObject(root, "body");

    /* result */
    cJSON *result = cJSON_AddObjectToObject(body, "result");
    cJSON_AddBoolToObject(result,
                          "ok",
                          (strcmp(code, "OK") == 0));
    cJSON_AddStringToObject(result, "code", code);
    cJSON_AddStringToObject(result, "message", msg);

    /* Convert to string */
    char *json_str = cJSON_PrintUnformatted(root);

    /* Publish */
    //extra line this below testing purpose
    //printf("[LCU] MQTT ACK: %s\n", json_str);
    mqtt_publish("lcu/ack", json_str, strlen(json_str));

    /* Cleanup */
    cJSON_free(json_str);
    cJSON_Delete(root);
}


/*----------------------------------------------------------
 * TCP → JSON → Drive → MQTT ACK
 *----------------------------------------------------------*/
void Receive_Command_From_WCS(void)
{
    char json_buf[1024];

    /* LCU_Recv_Command already returns ONLY JSON */
    int json_len = LCU_Recv_Command(json_buf, sizeof(json_buf));
    if (json_len <= 0)
        return;

    //printf("[LCU] RAW JSON: %s\n", json_buf);

    ParsedCommand_t cmd;
    if (!Parse_Command_JSON(json_buf, &cmd))
    {
        printf("[LCU] JSON parse FAILED\n");
        return;
    }

    printf("[LCU] Parsed Command:\n");
    printf("  ID        : %s\n", cmd.id);
    printf("  TYPE      : %s\n", cmd.type);
    printf("  NAME      : %s\n", cmd.name);
    printf("  AXIS      : %s\n", cmd.axis);
    printf("  CMD ENUM  : %d\n", cmd.cmd);
    printf("  TARGET_DEG: %.2f\n", cmd.target_deg);
    printf("  VELOCITY  : %.2f\n", cmd.velocity);
    printf("  ACCEL     : %.2f\n", cmd.accel);
    printf("  DECEL     : %.2f\n", cmd.decel);
    printf("  TARGET_POS: %.2f\n", cmd.target_pos);

    /* Axis dispatch */
    // if (strcmp(cmd.axis, "1") == 0)
    //     execute_on_axis(AXIS_TILT, &cmd);
    // else if (strcmp(cmd.axis, "2") == 0)
    //     execute_on_axis(AXIS_PAN, &cmd);
    // else if (strcmp(cmd.axis, "3") == 0)
    // {
    //     execute_on_axis(AXIS_BOTH, &cmd);
    // }
    // else
    // {
    //     send_ack(&cmd, "INVALID_AXIS", "Unknown axis");
    //     return;
    // }
    send_ack(&cmd, "OK", "Command executed");
}
