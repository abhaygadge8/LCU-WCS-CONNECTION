#include "mqtt_client.h"
#include "ini.h"              /* net_cfg */
#include "MQTTClient.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

/*----------------------------------------------------------
 * Internal state
 *----------------------------------------------------------*/
static MQTTClient mqtt_client = NULL;
static int mqtt_is_up = 0;

/*----------------------------------------------------------
 * Init
 *----------------------------------------------------------*/
int mqtt_init(void)
{
    char broker_addr[128];
    int rc;
    // Build broker string from config.ini 
    snprintf(broker_addr, sizeof(broker_addr),
             "tcp://%s:%d",
             net_cfg.MQTT_BROKER_IP,
             net_cfg.MQTT_BROKER_PORT);
    // Cleanup if already initialized 
    if (mqtt_client)
        mqtt_close();

    rc = MQTTClient_create(&mqtt_client,
                           broker_addr,
                           net_cfg.MQTT_CLIENT_ID,
                           MQTTCLIENT_PERSISTENCE_NONE,
                           NULL);
    if (rc != MQTTCLIENT_SUCCESS)
    {
        printf("[MQTT] Client create failed (%d)\n", rc);
        return -1;
    }

    MQTTClient_connectOptions conn_opts =
        MQTTClient_connectOptions_initializer;

    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession      = 1;

    /* Last Will */
    MQTTClient_willOptions will_opts =
        MQTTClient_willOptions_initializer;

    will_opts.topicName = net_cfg.MQTT_TOPIC_TELEMETRY;
    will_opts.message   = "offline";
    will_opts.qos       = 1;
    will_opts.retained  = 1;

    conn_opts.will = &will_opts;

    rc = MQTTClient_connect(mqtt_client, &conn_opts);
    if (rc != MQTTCLIENT_SUCCESS)
    {
        printf("[MQTT] Connect failed (%d)\n", rc);
        MQTTClient_destroy(&mqtt_client);
        mqtt_client = NULL;
        return -1;
    }

    mqtt_is_up = 1;
    printf("[MQTT] Connected to %s\n", broker_addr);

    return 0;
}
/*----------------------------------------------------------
 * Publish
 *----------------------------------------------------------*/
int mqtt_publish(const char *topic,
                 const void *payload,
                 size_t payload_len)
{
    if (!mqtt_is_up || !mqtt_client || !topic || !payload)
        return -1;

    MQTTClient_message msg =
        MQTTClient_message_initializer;

    msg.payload    = (void *)payload;
    msg.payloadlen = (int)payload_len;
    msg.qos        = 1;
    msg.retained   = 0;

    MQTTClient_deliveryToken token;
    int rc = MQTTClient_publishMessage(mqtt_client,
                                       topic,
                                       &msg,
                                       &token);
    if (rc != MQTTCLIENT_SUCCESS)
    {
        printf("[MQTT] Publish failed (%d)\n", rc);
        mqtt_is_up = 0;
        return -1;
    }

    MQTTClient_waitForCompletion(mqtt_client,
                                 token,
                                 1000);
    return 0;
}
/*----------------------------------------------------------
 * Log publish (QoS 0, fire-and-forget)
 *----------------------------------------------------------*/
void mqtt_log_publish(const char *topic,
                      const char *fmt, ...)
{
    if (!mqtt_is_up || !mqtt_client || !topic || !fmt)
        return;

    char buffer[512];
    va_list args;

    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    MQTTClient_message msg =
        MQTTClient_message_initializer;

    msg.payload    = buffer;
    msg.payloadlen = (int)strlen(buffer);
    msg.qos        = 0;
    msg.retained   = 0;

    MQTTClient_publishMessage(mqtt_client,topic,&msg,NULL);
}

/*----------------------------------------------------------
 * Status
 *----------------------------------------------------------*/
int mqtt_connected(void)
{
    return (mqtt_client &&
            MQTTClient_isConnected(mqtt_client));
}
/*----------------------------------------------------------
 * Close
 *----------------------------------------------------------*/
void mqtt_close(void)
{
    if (mqtt_client)
    {
        if (MQTTClient_isConnected(mqtt_client))
            MQTTClient_disconnect(mqtt_client, 1000);

        MQTTClient_destroy(&mqtt_client);
        mqtt_client = NULL;
        mqtt_is_up = 0;
    }
}