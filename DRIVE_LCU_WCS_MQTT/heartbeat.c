#include "heartbeat.h"
#include "mqtt_client.h"
#include "ini.h"
#include <stdio.h>
#include <string.h>
#include "cJSON.h"

void Task_Send_Heartbeat(void)
{
    /* Safety check */
    if (!mqtt_connected())
        return;

    /* Root JSON object */
    cJSON *root = cJSON_CreateObject();
    if (!root)
        return;

    cJSON_AddNumberToObject(root, "v", 1);
    cJSON_AddStringToObject(root, "id", "heartbeat");
    cJSON_AddStringToObject(root, "type", "Event");
    cJSON_AddStringToObject(root, "name", "Heartbeat");
    cJSON_AddStringToObject(root, "src", "middleware");

    /* Body */
    cJSON *body = cJSON_AddObjectToObject(root, "body");
    cJSON_AddStringToObject(body, "status", "alive");

    /* Meta (empty object) */
    cJSON_AddObjectToObject(root, "meta");

    /* Convert JSON to string (compact, no spaces) */
    char *json_str = cJSON_PrintUnformatted(root);
    if (!json_str)
    {
        cJSON_Delete(root);
        return;
    }

    /* Publish heartbeat */
    mqtt_publish(net_cfg.MQTT_TOPIC_HEARTBEAT,
                 json_str,
                 strlen(json_str));

    /* Cleanup */
    cJSON_free(json_str);
    cJSON_Delete(root);
}

