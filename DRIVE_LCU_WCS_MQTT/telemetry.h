#ifndef TELEMETRY_H
#define TELEMETRY_H

#include "axis_helper.h"

typedef enum
{
    TELEMETRY_ONCE,
    TELEMETRY_PERIODIC,
    TELEMETRY_CONTINUOUS
} TelemetryMode_t;

/* Axis-aware telemetry over MQTT */
void Task_Send_Telemetry(Axis_t axis, TelemetryMode_t mode);

#endif
