/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include <stdint.h>
#include "main.h"
#include "statusUpdateTask.h"
#include "alerts.h"

/* ==================================================================== */
/* ============================= DEFINES ============================== */
/* ==================================================================== */

#define HEARTBEAT_BLINK_MS      300
#define HEARTBEAT_PERIOD_MS     1000

#define CLEAR_ON_START_MS       15000

/* ==================================================================== */
/* =================== LOCAL FUNCTION DECLARATIONS ==================== */
/* ==================================================================== */

static void updateHeartbeat();
static void updateSdcStatus(shutdownCircuitStatus_S *shutdownCircuitData);
static void runStatusAlertMonitor(shutdownCircuitStatus_S *shutdownCircuitData);

/* ==================================================================== */
/* =================== LOCAL FUNCTION DEFINITIONS ===================== */
/* ==================================================================== */

static void updateHeartbeat()
{
    static uint8_t hbState = 0;
    static uint32_t lastHeartBeatUpdate = 0;
    if(hbState)
    {
        if((HAL_GetTick() - lastHeartBeatUpdate) > HEARTBEAT_BLINK_MS)
        {
            HAL_GPIO_WritePin(MCU_HEARTBEAT_GPIO_Port, MCU_HEARTBEAT_Pin, GPIO_PIN_RESET);
            hbState = 0;
            lastHeartBeatUpdate = HAL_GetTick();
        }
    }
    else
    {
        if((HAL_GetTick() - lastHeartBeatUpdate) > (HEARTBEAT_PERIOD_MS - HEARTBEAT_BLINK_MS))
        {
            HAL_GPIO_WritePin(MCU_HEARTBEAT_GPIO_Port, MCU_HEARTBEAT_Pin, GPIO_PIN_SET);
            hbState = 1;
            lastHeartBeatUpdate = HAL_GetTick();
        }
    }
}

static void updateSdcStatus(shutdownCircuitStatus_S *shutdownCircuitData)
{
    shutdownCircuitData->imdLatchOpen = (HAL_GPIO_ReadPin(IMD_FAULT_READ_GPIO_Port, IMD_FAULT_READ_Pin) && (HAL_GetTick() > CLEAR_ON_START_MS));
    shutdownCircuitData->bmsLatchOpen = ((HAL_GPIO_ReadPin(AMS_FAULT_READ_GPIO_Port, AMS_FAULT_READ_Pin)) && (HAL_GetTick() > CLEAR_ON_START_MS));
    shutdownCircuitData->bmsInhibitActive = HAL_GPIO_ReadPin(AMS_INB_N_GPIO_Port, AMS_INB_N_Pin) ^ 1;
    shutdownCircuitData->sdcSenseFaultActive[0] = HAL_GPIO_ReadPin(SDC0_GPIO_Port, SDC0_Pin);
    shutdownCircuitData->sdcSenseFaultActive[1] = HAL_GPIO_ReadPin(SDC1_GPIO_Port, SDC1_Pin);
    shutdownCircuitData->sdcSenseFaultActive[2] = HAL_GPIO_ReadPin(SDC2_GPIO_Port, SDC2_Pin);
    shutdownCircuitData->sdcSenseFaultActive[3] = HAL_GPIO_ReadPin(SDC3_GPIO_Port, SDC3_Pin);
    shutdownCircuitData->sdcSenseFaultActive[4] = HAL_GPIO_ReadPin(SDC4_GPIO_Port, SDC4_Pin);
    shutdownCircuitData->sdcSenseFaultActive[5] = HAL_GPIO_ReadPin(SDC5_GPIO_Port, SDC5_Pin);
}

static void runStatusAlertMonitor(shutdownCircuitStatus_S *shutdownCircuitData)
{
    // Accumulate alert statuses
    bool responseStatus[NUM_ALERT_RESPONSES] = {false};

    for(int32_t i = 0; i < NUM_STATUS_ALERTS; i++)
    {
        Alert_S* alert = statusAlerts[i];

        // Check alert condition and run alert monitor
        alert->alertConditionPresent = statusAlertConditionArray[i](shutdownCircuitData);
        runAlertMonitor(alert);

        // Get alert status and set response
        const AlertStatus_E alertStatus = getAlertStatus(alert);
        if((alertStatus == ALERT_SET) || (alertStatus == ALERT_LATCHED))
        {
            // Iterate through all alert responses and set them
            for (uint32_t j = 0; j < alert->numAlertResponse; j++)
            {
                const AlertResponse_E response = alert->alertResponse[j];
                // Set the alert response to active
                responseStatus[response] = true;
            }
        }
    }
    setAmsFault(responseStatus[AMS_FAULT]);
}

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DEFINITIONS ==================== */
/* ==================================================================== */

void initStatusUpdateTask()
{
    HAL_GPIO_WritePin(MCU_HEARTBEAT_GPIO_Port, MCU_HEARTBEAT_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MCU_FAULT_GPIO_Port, MCU_FAULT_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MCU_GSENSE_GPIO_Port, MCU_GSENSE_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(AMS_INB_N_GPIO_Port, AMS_INB_N_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(AMS_FAULT_N_GPIO_Port, AMS_FAULT_N_Pin, GPIO_PIN_SET);
}

void runStatusUpdateTask()
{
    statusUpdateTaskData_S statusUpdateTaskDataLocal;

    vTaskSuspendAll();
    statusUpdateTaskDataLocal = statusUpdateTaskData;
    xTaskResumeAll();

    // Update hearbeat led
    updateHeartbeat();

    // Update imd status
    getImdStatus(&statusUpdateTaskDataLocal.imdData);

    // Update shutdown circuit data
    updateSdcStatus(&statusUpdateTaskDataLocal.shutdownCircuitData);

    // Run alert monitor
    runStatusAlertMonitor(&statusUpdateTaskDataLocal);

    // Copy out new data into global data struct
    vTaskSuspendAll();
    statusUpdateTaskData = statusUpdateTaskDataLocal;
    xTaskResumeAll();
}