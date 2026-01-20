#include "command_parser.h"
#include "cJSON.h"

#include <string.h>
#include <stdio.h>

bool Parse_Command_JSON(const char *json, ParsedCommand_t *out)
{
    memset(out, 0, sizeof(*out));

    cJSON *root = cJSON_Parse(json);
    if (!root)
        return false;

    /* ---------------- Envelope ---------------- */
    cJSON *v    = cJSON_GetObjectItem(root, "v");
    cJSON *id   = cJSON_GetObjectItem(root, "id");
    cJSON *type = cJSON_GetObjectItem(root, "type");
    cJSON *name = cJSON_GetObjectItem(root, "name");
    cJSON *body = cJSON_GetObjectItem(root, "body");

    if (!cJSON_IsNumber(v) ||
        !cJSON_IsString(id) ||
        !cJSON_IsString(type) ||
        !cJSON_IsString(name) ||
        !cJSON_IsObject(body))
    {
        cJSON_Delete(root);
        return false;
    }

    out->v = v->valueint;
    strncpy(out->id,   id->valuestring,   sizeof(out->id)-1);
    strncpy(out->type, type->valuestring, sizeof(out->type)-1);
    strncpy(out->name, name->valuestring, sizeof(out->name)-1);

    /* ---------------- Command Mapping ---------------- */
    if      (strcmp(out->name, "EnableDrive") == 0) out->cmd = CMD_ENABLE;
    else if (strcmp(out->name, "DisableDrive") == 0) out->cmd = CMD_DISABLE;
    else if (strcmp(out->name, "ResetDrive") == 0) out->cmd = CMD_RESET;
    else if (strcmp(out->name, "EStop") == 0) out->cmd = CMD_ESTOP;
    else if (strcmp(out->name, "SetAngleParams") == 0) out->cmd = CMD_SET_ANGLE;
    else if (strcmp(out->name, "SetMotionParams") == 0) out->cmd = CMD_SET_POS;
    else if (strcmp(out->name, "Move") == 0) out->cmd = CMD_MOVE;
    else if (strcmp(out->name, "MoveDeg") == 0) out->cmd = CMD_MOVE_DEG;
    else if (strcmp(out->name, "JogFwd") == 0) out->cmd = CMD_VELOCITY_FWD;
    else if (strcmp(out->name, "JogRev") == 0) out->cmd = CMD_VELOCITY_REV;
    else if (strcmp(out->name, "Halt") == 0) out->cmd = CMD_HALT;
    else if (strcmp(out->name, "Solenoid") == 0) out->cmd = CMD_SOLENOID;
    else if (strcmp(out->name, "Jog") == 0) out->cmd = CMD_VELOCITY_FWD;
    else if (strcmp(out->name, "MovePosition") == 0) out->cmd = CMD_MOVE;
    else if (strcmp(out->name, "MoveToPositionDeg") == 0) out->cmd = CMD_MOVE_DEG;
    else
    {
        out->cmd = CMD_INVALID;
        cJSON_Delete(root);
        return false;
    }

    /* ---------------- Body Fields ---------------- */
    cJSON *axis = cJSON_GetObjectItem(body, "axis");
    if (cJSON_IsString(axis))
        strncpy(out->axis, axis->valuestring, sizeof(out->axis)-1);

    cJSON *tdeg = cJSON_GetObjectItem(body, "target_deg");
    if (cJSON_IsNumber(tdeg))
        out->target_deg = (float)tdeg->valuedouble;

    cJSON *tpos = cJSON_GetObjectItem(body, "target_pos");
    if (cJSON_IsNumber(tpos))
        out->target_pos = (float)tpos->valuedouble;

    cJSON *vel = cJSON_GetObjectItem(body, "velocity");
    if (cJSON_IsNumber(vel))
        out->velocity = (float)vel->valuedouble;

    cJSON *acc = cJSON_GetObjectItem(body, "accel");
    if (cJSON_IsNumber(acc))
        out->accel = (float)acc->valuedouble;

    cJSON *dec = cJSON_GetObjectItem(body, "decel");
    if (cJSON_IsNumber(dec))
        out->decel = (float)dec->valuedouble;

    cJSON_Delete(root);
    return true;
}











// #include "command_parser.h"
// #include <string.h>
// #include <stdio.h>

// bool Parse_Command_JSON(const char *json, ParsedCommand_t *out)
// {
//     memset(out, 0, sizeof(*out));

//     if (!strstr(json, "\"v\":1"))
//         return false;

//     sscanf(json,
//         "%*[^i]\"id\":\"%63[^\"]\""
//         "%*[^t]\"type\":\"%15[^\"]\""
//         "%*[^n]\"name\":\"%31[^\"]\"",
//         out->id, out->type, out->name);

//     if      (strcmp(out->name, "EnableDrive") == 0) out->cmd = CMD_ENABLE;
//     else if (strcmp(out->name, "DisableDrive") == 0) out->cmd = CMD_DISABLE;
//     else if (strcmp(out->name, "ResetDrive") == 0) out->cmd = CMD_RESET;
//     else if (strcmp(out->name, "EStop") == 0) out->cmd = CMD_ESTOP;
//     else if (strcmp(out->name, "SetAngleParams") == 0) out->cmd = CMD_SET_ANGLE;
//     else if (strcmp(out->name, "SetMotionParams") == 0) out->cmd = CMD_SET_POS;
//     else if (strcmp(out->name, "Move") == 0) out->cmd = CMD_MOVE;
//     else if (strcmp(out->name, "MoveDeg") == 0) out->cmd = CMD_MOVE_DEG;
//     else if (strcmp(out->name, "JogFwd") == 0) out->cmd = CMD_VELOCITY_FWD;
//     else if (strcmp(out->name, "JogRev") == 0) out->cmd = CMD_VELOCITY_REV;
//     else if (strcmp(out->name, "Halt") == 0) out->cmd = CMD_HALT;
//     else return false;

//     /* ---- PARSE FROM BODY ---- */
//     const char *body = strstr(json, "\"body\"");
//     if (!body) return false;

//     sscanf(body,
//         "%*[^a]\"axis\":\"%7[^\"]\""
//         "%*[^t]\"target_deg\":%f"
//         "%*[^v]\"velocity_deg_s\":%f"
//         "%*[^a]\"accel_deg_s2\":%f"
//         "%*[^d]\"decel_deg_s2\":%f"
//         "%*[^p]\"target_pos\":%f",
//         out->axis,
//         &out->target_deg,
//         &out->velocity,
//         &out->accel,
//         &out->decel,
//         &out->target_pos);

//     return true;
// }
