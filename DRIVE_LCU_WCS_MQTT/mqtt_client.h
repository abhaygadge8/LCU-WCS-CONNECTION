#ifndef MQTT_H
#define MQTT_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file mqtt.h
 * @brief MQTT wrapper using config.ini values only
 */

/*----------------------------------------------------------
 * API
 *----------------------------------------------------------*/

/**
 * @brief Initialize and connect to MQTT broker
 *        (uses MQTT_BROKER_IP, MQTT_BROKER_PORT, MQTT_CLIENT_ID)
 * @return 0 on success, -1 on failure
 */
int mqtt_init(void);

/**
 * @brief Publish binary/string payload to a topic
 * @param topic MQTT topic (normally from net_cfg)
 * @param payload data pointer
 * @param payload_len length of payload
 * @return 0 on success, -1 on failure
 */
int mqtt_publish(const char *topic,
                 const void *payload,
                 size_t payload_len);

/**
 * @brief Publish formatted log message (QoS 0)
 * @param topic MQTT topic
 * @param fmt printf-style format
 */
void mqtt_log_publish(const char *topic,
                      const char *fmt, ...);

/**
 * @brief Check MQTT connection status
 * @return 1 if connected, 0 otherwise
 */
int mqtt_connected(void);

/**
 * @brief Disconnect and cleanup MQTT
 */
void mqtt_close(void);

#ifdef __cplusplus
}
#endif

#endif /* MQTT_H */
