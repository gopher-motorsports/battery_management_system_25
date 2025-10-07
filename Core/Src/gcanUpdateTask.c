/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */
#include "gcanUpdateTask.h"
#include "main.h"
#include "GopherCAN.h"
#include "gopher_sense.h"
#include "gcanUtils.h"
#include "alerts.h"

// Frequency group update periods millisecs
#define HIGH_FREQ_UPDATE_PERIOD         10
#define MEDIUM_FREQ_UPDATE_PERIOD       100
#define LOW_FREQ_UPDATE_PERIOD          1000

#define LOW_FREQ_LOGGING_DELAY          (LOW_FREQ_UPDATE_PERIOD / NUM_CELL_MON_IN_ACCUMULATOR)

/* ==================================================================== */
/* ============================== STRUCTS ============================= */
/* ==================================================================== */

typedef struct
{
    telemetryTaskData_S telemetryTaskData;
    statusUpdateTaskData_S statusUpdateTaskData;
} GcanTaskInputData_S;


/* ==================================================================== */
/* ======================= EXTERNAL VARIABLES ========================= */
/* ==================================================================== */

extern CAN_HandleTypeDef hcan2;

/* ==================================================================== */
/* =================== LOCAL FUNCTION DECLARATIONS ==================== */
/* ==================================================================== */

void updateHighFrequencyVariables(GcanTaskInputData_S* gcanData);
void updateMediumFrequencyVariables(GcanTaskInputData_S* gcanData);
void updateLowFrequencyVariables(GcanTaskInputData_S* gcanData, uint32_t segmentIndex);

/* ==================================================================== */
/* =================== LOCAL FUNCTION DEFINITIONS ===================== */
/* ==================================================================== */

void updateHighFrequencyVariables(GcanTaskInputData_S* gcanData)
{
    // Pack current
    update_and_queue_param_float(&bmsBatteryCurrent_A, gcanData->telemetryTaskData.packMonitor.packCurrent);
    // update_and_queue_param_u16(&bmsBatteryCurrent_A, (uint16_t)(gcanData->telemetryTaskData.packMonitor.packCurrent));

    // Pack voltage
    // update_and_queue_param_float(&bmsBatteryVoltage_V, gcanData->telemetryTaskData.packMonitor.packVoltage);

    // Link voltage
    // update_and_queue_param_float(&bmsTractiveSystemVoltage_V, gcanData->telemetryTaskData.packMonitor.linkVoltage);
    // update_and_queue_param_u16(&bmsTractiveSystemVoltage_V, (uint16_t)(-1.0f * gcanData->telemetryTaskData.packMonitor.linkVoltage));

    // Flags
    update_and_queue_param_u8(&imdFault_state, gcanData->statusUpdateTaskData.shutdownCircuitData.imdLatchOpen);
    update_and_queue_param_u8(&amsFault_state, gcanData->statusUpdateTaskData.shutdownCircuitData.bmsLatchOpen);
}

void updateMediumFrequencyVariables(GcanTaskInputData_S* gcanData)
{
    // State of energy
    update_and_queue_param_float(&soeByOCV_percent, gcanData->telemetryTaskData.packMonitor.socData.soeByOcv * 100.0f);
    update_and_queue_param_float(&soeByCoulombCounting_percent, gcanData->telemetryTaskData.packMonitor.socData.soeByCoulombCounting * 100.0f);

    // Pack Statistics
    update_and_queue_param_float(&maxCellVoltage_V, gcanData->telemetryTaskData.maxCellVoltage);
    update_and_queue_param_float(&minCellVoltage_V, gcanData->telemetryTaskData.minCellVoltage);
    update_and_queue_param_float(&avgCellVoltage_V, gcanData->telemetryTaskData.avgCellVoltage);
    update_and_queue_param_float(&cellImbalance_mV, gcanData->telemetryTaskData.cellImbalance * 1000.0f);
    update_and_queue_param_float(&maxCellTemp_C, gcanData->telemetryTaskData.maxCellTemp);
    update_and_queue_param_float(&minCellTemp_C, gcanData->telemetryTaskData.minCellTemp);
    update_and_queue_param_float(&avgCellTemp_C, gcanData->telemetryTaskData.avgCellTemp);
    update_and_queue_param_float(&maxBoardTemp_C, gcanData->telemetryTaskData.maxBoardTemp);
    update_and_queue_param_float(&minBoardTemp_C, gcanData->telemetryTaskData.minBoardTemp);
    update_and_queue_param_float(&avgBoardTemp_C, gcanData->telemetryTaskData.avgBoardTemp);

    // Alerts

    uint32_t numAlertsSet = 0;

    for(int32_t i = 0; i < NUM_TELEMETRY_ALERTS; i++)
    {
        Alert_S* alert = telemetryAlerts[i];

        // Get alert status and set response
        const AlertStatus_E alertStatus = getAlertStatus(alert);
        if((alertStatus == ALERT_SET) || (alertStatus == ALERT_LATCHED))
        {
            numAlertsSet++;
            update_and_queue_param_u8(bmsAlertsParams[i], 0x01);
        }
        else
        {
            update_and_queue_param_u8(bmsAlertsParams[i], 0x00);
        }
    }

    update_and_queue_param_u8(&bmsNumActiveAlerts_state, numAlertsSet);

    

    // update_and_queue_param_u8(&bmsCurrAlertIndex_state, );

    // SDC status
    for(uint32_t i = 0; i < NUM_SDC_SENSE_INPUTS; i++)
    {
        update_and_queue_param_u8(bmsShutdownParams[i], gcanData->statusUpdateTaskData.shutdownCircuitData.sdcSenseFaultActive[i]);
    }

    update_and_queue_param_u8(&bmsInhibitActive_state, gcanData->statusUpdateTaskData.shutdownCircuitData.bmsInhibitActive);


}

void updateLowFrequencyVariables(GcanTaskInputData_S* gcanData, uint32_t segmentIndex)
{
    // Log all segment variables
    // for(uint32_t i = 0; i < NUM_CELL_MON_IN_ACCUMULATOR; i++)
    // {
        // for(uint32_t j = 0; j < NUM_CELLS_PER_CELL_MONITOR; j++)
        // {
        //     update_and_queue_param_float(cellVoltageParams[segmentIndex][j], gcanData->telemetryTaskData.bmb[segmentIndex].cellVoltage[j]);
        //     update_and_queue_param_float(cellTempParams[segmentIndex][j], gcanData->telemetryTaskData.bmb[segmentIndex].cellTemp[j]);
        // }

        update_and_queue_param_float(cellStatParams[segmentIndex][0], gcanData->telemetryTaskData.bmb[segmentIndex].maxCellVoltage);
        update_and_queue_param_float(cellStatParams[segmentIndex][1], gcanData->telemetryTaskData.bmb[segmentIndex].minCellVoltage);
        update_and_queue_param_float(cellStatParams[segmentIndex][2], gcanData->telemetryTaskData.bmb[segmentIndex].avgCellVoltage);
        update_and_queue_param_float(cellStatParams[segmentIndex][3], gcanData->telemetryTaskData.bmb[segmentIndex].dieTemp);
        update_and_queue_param_float(cellStatParams[segmentIndex][4], gcanData->telemetryTaskData.bmb[segmentIndex].maxCellTemp);
        update_and_queue_param_float(cellStatParams[segmentIndex][5], gcanData->telemetryTaskData.bmb[segmentIndex].minCellTemp);
        update_and_queue_param_float(cellStatParams[segmentIndex][6], gcanData->telemetryTaskData.bmb[segmentIndex].avgCellTemp);
        update_and_queue_param_float(cellStatParams[segmentIndex][7], gcanData->telemetryTaskData.bmb[segmentIndex].boardTemp);
    // }
}

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DEFINITIONS ==================== */
/* ==================================================================== */

void initGcanUpdateTask()
{
    
}

void runGcanUpdateTask()
{
    GcanTaskInputData_S gcanTaskInputData;
    vTaskSuspendAll();
    gcanTaskInputData.telemetryTaskData = telemetryTaskData;
    gcanTaskInputData.statusUpdateTaskData = statusUpdateTaskData;
    xTaskResumeAll();

    // High frequency update variables - 100Hz
    static uint32_t lastHighFreqUpdateTick = 0;
    if((HAL_GetTick() - lastHighFreqUpdateTick) >= HIGH_FREQ_UPDATE_PERIOD)
    {
        lastHighFreqUpdateTick = HAL_GetTick();
        updateHighFrequencyVariables(&gcanTaskInputData);
    }

    // Medium frequency update variables - 10Hz
    static uint32_t lastMediumFreqUpdateTick = 0;
    if((HAL_GetTick() - lastMediumFreqUpdateTick) >= MEDIUM_FREQ_UPDATE_PERIOD)
    {
        lastMediumFreqUpdateTick = HAL_GetTick();
        updateMediumFrequencyVariables(&gcanTaskInputData);
    }

    // Low frequency update variables - 1Hz
    static uint32_t lastLowFreqUpdateTick = 0;
    if((HAL_GetTick() - lastLowFreqUpdateTick) >= LOW_FREQ_LOGGING_DELAY)
    {
        lastLowFreqUpdateTick = HAL_GetTick();

        static uint32_t segmentIndex = 0;
        updateLowFrequencyVariables(&gcanTaskInputData, segmentIndex);

        segmentIndex++;
        if(segmentIndex >= NUM_CELL_MON_IN_ACCUMULATOR)
        {
            segmentIndex = 0;
        }
    }

    // Update gcan tx
    service_can_tx(&hcan2);

    // Update gcan rx
    service_can_rx_buffer();
}
