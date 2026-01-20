#include "telemetry.h"
#include "mqtt_client.h"
#include "ini.h"
#include "drive_feedback.h"
#include "modbus_functions.h"
#include "cJSON.h"
#include <stdio.h>
#include <string.h>

/* -------------------------------------------------------
 * TELEMETRY: SEND ONCE (BOOT / STATIC INFO)
 * ------------------------------------------------------- */
static void send_once_telemetry(Axis_t axis)
{
    uint16_t version = 0, revision = 0, release = 0;

    Read_Version(axis, &version);
    Read_Revision(axis, &revision);
    Read_ReleaseDate(axis, &release);

    cJSON *root = cJSON_CreateObject();
    if (!root) return;

    cJSON_AddStringToObject(root, "type", "once");
    cJSON_AddNumberToObject(root, "axis", axis);
    cJSON_AddNumberToObject(root, "version", version);
    cJSON_AddNumberToObject(root, "revision", revision);
    cJSON_AddNumberToObject(root, "release", release);

    char *json = cJSON_PrintUnformatted(root);
    if (json)
    {
        mqtt_publish(net_cfg.MQTT_TOPIC_TELEMETRY,
                     json,
                     strlen(json));
        cJSON_free(json);
    }

    cJSON_Delete(root);
}

/* -------------------------------------------------------
 * TELEMETRY: PERIODIC (HEALTH / STATUS)
 * ------------------------------------------------------- */
static void send_periodic_telemetry(Axis_t axis)
{
    float motor_current = 0.0f;
    float dcbus         = 0.0f;
    FaultStatus_t fault = {0};

    int32_t drive_status = MODBUS_CheckConnection();
    uint8_t drive_connected = (drive_status == 0);

    if (drive_connected)
    {
        Read_Current(axis, &motor_current);
        Read_DCBusVoltage(axis, &dcbus);
        Read_FaultStatus(axis, &fault);
    }

    cJSON *root = cJSON_CreateObject();
    cJSON *body = cJSON_CreateObject();
    cJSON *fault_bits = cJSON_CreateObject();

    if (!root || !body || !fault_bits)
        goto cleanup;

    cJSON_AddNumberToObject(root, "v", 1);
    cJSON_AddStringToObject(root, "id", "periodic");
    cJSON_AddStringToObject(root, "type", "Event");
    cJSON_AddStringToObject(root, "name", "TelemetryPeriodic");
    cJSON_AddStringToObject(root, "src", "middleware");

    cJSON_AddNumberToObject(body, "axis", axis);
    cJSON_AddBoolToObject(body, "drive_connected", drive_connected);
    cJSON_AddNumberToObject(body, "motor_current", motor_current);
    cJSON_AddNumberToObject(body, "dc_bus", dcbus);
    cJSON_AddNumberToObject(body, "fault_raw", fault.raw_code);

    cJSON_AddBoolToObject(fault_bits, "short_circuit",   fault.short_circuit);
    cJSON_AddBoolToObject(fault_bits, "system_ok",       fault.system_ok);
    cJSON_AddBoolToObject(fault_bits, "rated_current",   fault.rated_current);
    cJSON_AddBoolToObject(fault_bits, "over_temp",       fault.over_temp);
    cJSON_AddBoolToObject(fault_bits, "over_volt",       fault.over_volt);
    cJSON_AddBoolToObject(fault_bits, "under_volt",      fault.under_volt);
    cJSON_AddBoolToObject(fault_bits, "motion_error",    fault.motion_error);
    cJSON_AddBoolToObject(fault_bits, "drive_disable",   fault.drive_disable);
    cJSON_AddBoolToObject(fault_bits, "eeprom_error",    fault.eeprom_error);
    cJSON_AddBoolToObject(fault_bits, "commutation_err", fault.commutation_err);
    cJSON_AddBoolToObject(fault_bits, "lock_rotor",      fault.lock_rotor);
    cJSON_AddBoolToObject(fault_bits, "emergency_err",   fault.emergency_err);
    cJSON_AddBoolToObject(fault_bits, "command_error",   fault.command_error);
    cJSON_AddBoolToObject(fault_bits, "motion_complete", fault.motion_complete);

    cJSON_AddItemToObject(body, "fault_bits", fault_bits);
    cJSON_AddItemToObject(root, "body", body);
    cJSON_AddItemToObject(root, "meta", cJSON_CreateObject());

    char *json = cJSON_PrintUnformatted(root);
    if (json)
    {
        mqtt_publish(net_cfg.MQTT_TOPIC_TELEMETRY,
                     json,
                     strlen(json));
        cJSON_free(json);
    }

cleanup:
    cJSON_Delete(root);
}
/* -------------------------------------------------------
 * TELEMETRY: CONTINUOUS (MOTION FEEDBACK)
 * ------------------------------------------------------- */
static void send_continuous_telemetry(Axis_t axis)
{
    float actual_pos_mm = 0.0f;
    float pos_deg = 0.0f;
    float pos_mm  = 0.0f;
    float rpm     = 0.0f;
    uint16_t io_status = 0;

    Read_Actual_Absolute_Pos_MM(axis, &actual_pos_mm);
    Read_Position_Deg(axis, &pos_deg);
    Read_Position_MM(axis, &pos_mm);
    Read_RPM(axis, &rpm);
    Read_IO_Status(axis, &io_status);

    cJSON *root = cJSON_CreateObject();
    if (!root) return;

    cJSON_AddStringToObject(root, "type", "continuous");
    cJSON_AddNumberToObject(root, "axis", axis);
    cJSON_AddNumberToObject(root, "actual_pos_mm", actual_pos_mm);
    cJSON_AddNumberToObject(root, "pos_deg", pos_deg);
    cJSON_AddNumberToObject(root, "pos_mm", pos_mm);
    cJSON_AddNumberToObject(root, "rpm", rpm);
    cJSON_AddNumberToObject(root, "io_status", io_status);

    char *json = cJSON_PrintUnformatted(root);
    if (json)
    {
        mqtt_publish(net_cfg.MQTT_TOPIC_TELEMETRY,
                     json,
                     strlen(json));
        cJSON_free(json);
    }

    cJSON_Delete(root);
}
/* -------------------------------------------------------
 * PUBLIC TELEMETRY API
 * ------------------------------------------------------- */
void Task_Send_Telemetry(Axis_t axis, TelemetryMode_t mode)
{
    switch (mode)
    {
        case TELEMETRY_ONCE:
            send_once_telemetry(axis);
            break;

        case TELEMETRY_PERIODIC:
            send_periodic_telemetry(axis);
            break;

        case TELEMETRY_CONTINUOUS:
            send_continuous_telemetry(axis);
            break;

        default:
            break;
    }
}
