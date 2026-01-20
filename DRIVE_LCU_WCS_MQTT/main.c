#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "ini.h"
#include "lcu_comm.h"          // TCP server for WCS → LCU
#include "command_handler.h"   // Parse & send drive commands
#include "heartbeat.h"         // MQTT heartbeat
#include "telemetry.h"         // MQTT telemetry
#include "mqtt_client.h"       // MQTT core
#include "drive_feedback.h"
#include "drive_command.h"
#include "modbus_functions.h"

/* --------------------------------------------------
 * Time helper
 * -------------------------------------------------- */
static uint32_t GetTimeMs(void)
{
#ifdef _WIN32
    return GetTickCount();
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint32_t)(ts.tv_sec * 1000U + ts.tv_nsec / 1000000U);
#endif
}

int main(void)
{
    uint32_t last_heartbeat_ms   = 0;
    uint32_t last_periodic_ms    = 0;

    /* ---------------- LOAD CONFIG ---------------- */
    if (ini_load("config.ini") != 0)
    {
        printf("ERROR: config.ini load failed\n");
        return -1;
    }

    /* ---------------- INIT TCP (WCS → LCU) ---------------- */
    if (LCU_Comm_Init() != 0)
    {
        printf("ERROR: LCU TCP communication init failed\n");
        return -1;
    }

    /* ---------------- INIT MQTT (LCU → WCS) ---------------- */
    if (mqtt_init() != 0)
    {
        printf("ERROR: MQTT init failed\n");
        return -1;
    }

    printf("\n=====================================\n");
    printf(" LCU STARTED SUCCESSFULLY\n");
    printf(" TCP  : WCS -> LCU (Commands)\n");
    printf(" MQTT : LCU -> WCS (Heartbeat + Telemetry)\n");
    printf("=====================================\n");

    /* ---------------- SEND BOOT TELEMETRY ---------------- */
    // Task_Send_Telemetry(AXIS_PAN,  TELEMETRY_ONCE);
    // Task_Send_Telemetry(AXIS_TILT, TELEMETRY_ONCE);

    /* ---------------- MAIN LOOP ---------------- */
    while (1)
    {
        //uint32_t now = GetTimeMs();

        /* ---- TASK 1: Receive TCP Command from WCS ---- */
        Receive_Command_From_WCS();
        Sleep(10);  // allow other threads to run

        /* ---- TASK 2: MQTT internal processing ---- */
        //MQTT_Loop();   // keep connection alive

        /* ---- TASK 3: Heartbeat (1 sec) ---- */
        // if ((now - last_heartbeat_ms) >= 1000U)
        // {
        //     Task_Send_Heartbeat();
        //     last_heartbeat_ms = now;
        // }

        /* ---- TASK 4: Periodic telemetry (5 sec) ---- */
        // if ((now - last_periodic_ms) >= 5000U)
        // {
        //     Task_Send_Telemetry(AXIS_PAN,  TELEMETRY_PERIODIC);
        //     Task_Send_Telemetry(AXIS_TILT, TELEMETRY_PERIODIC);
        //     last_periodic_ms = now;
        // }

        /* ---- TASK 5: Continuous telemetry ---- */
        // Task_Send_Telemetry(AXIS_PAN,  TELEMETRY_CONTINUOUS);
        // Task_Send_Telemetry(AXIS_TILT, TELEMETRY_CONTINUOUS);

#ifdef _WIN32
        Sleep(10);   // prevent CPU hogging
#else
        usleep(10000);
#endif
    }

    /* ---------------- CLEANUP ---------------- */
    mqtt_close();
    LCU_Comm_Close();
    return 0;
}
