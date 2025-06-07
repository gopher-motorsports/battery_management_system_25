/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */
#include "gcanUpdateTask.h"
#include "main.h"
#include "GopherCAN.h"
#include "gopher_sense.h"
#include "gcanUtils.h"

// Frequency group update periods millisecs
#define HIGH_FREQ_UPDATE_PERIOD         10
#define MEDIUM_FREQ_UPDATE_PERIOD       100
#define LOW_FREQ_UPDATE_PERIOD          1000

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
void updateLowFrequencyVariables(GcanTaskInputData_S* gcanData);

/* ==================================================================== */
/* =================== LOCAL FUNCTION DEFINITIONS ===================== */
/* ==================================================================== */

void updateHighFrequencyVariables(GcanTaskInputData_S* gcanData)
{
    // Pack current
    update_and_queue_param_float(&bmsBatteryCurrent_A, gcanData->telemetryTaskData.packMonitor.packCurrent);

    // Pack voltage
    update_and_queue_param_float(&bmsBatteryVoltage_V, gcanData->telemetryTaskData.packMonitor.packVoltage);

    // Link voltage
    update_and_queue_param_float(&bmsTractiveSystemVoltage_V, gcanData->telemetryTaskData.packMonitor.linkVoltage);

    // State of energy
    update_and_queue_param_float(&soeByOCV_percent, gcanData->telemetryTaskData.packMonitor.socData.soeByOcv * 100.0f);
    update_and_queue_param_float(&soeByCoulombCounting_percent, gcanData->telemetryTaskData.packMonitor.socData.soeByCoulombCounting * 100.0f);
}

void updateMediumFrequencyVariables(GcanTaskInputData_S* gcanData)
{

}

void updateLowFrequencyVariables(GcanTaskInputData_S* gcanData)
{
    // Log all segment variables
    for(uint32_t i = 0; i < NUM_CELL_MON_IN_ACCUMULATOR; i++)
    {
        for(uint32_t j = 0; j < NUM_CELLS_PER_CELL_MONITOR; j++)
        {
            update_and_queue_param_float(cellVoltageParams[i][j], gcanData->telemetryTaskData.bmb[i].cellVoltage[j]);
            update_and_queue_param_float(cellTempParams[i][j], gcanData->telemetryTaskData.bmb[i].cellTemp[j]);
        }

        update_and_queue_param_float(cellStatParams[i][0], gcanData->telemetryTaskData.bmb[i].maxCellVoltage);
        update_and_queue_param_float(cellStatParams[i][1], gcanData->telemetryTaskData.bmb[i].minCellVoltage);
        update_and_queue_param_float(cellStatParams[i][2], gcanData->telemetryTaskData.bmb[i].avgCellVoltage);
        update_and_queue_param_float(cellStatParams[i][3], gcanData->telemetryTaskData.bmb[i].dieTemp);
        update_and_queue_param_float(cellStatParams[i][4], gcanData->telemetryTaskData.bmb[i].maxCellTemp);
        update_and_queue_param_float(cellStatParams[i][5], gcanData->telemetryTaskData.bmb[i].minCellTemp);
        update_and_queue_param_float(cellStatParams[i][6], gcanData->telemetryTaskData.bmb[i].avgCellTemp);
        update_and_queue_param_float(cellStatParams[i][7], gcanData->telemetryTaskData.bmb[i].boardTemp);
    }
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
    if((HAL_GetTick() - lastLowFreqUpdateTick) >= LOW_FREQ_UPDATE_PERIOD)
    {
        lastLowFreqUpdateTick = HAL_GetTick();
        updateLowFrequencyVariables(&gcanTaskInputData);
    }

    // Update gcan tx
    service_can_tx(&hcan2);

    // Update gcan rx
    service_can_rx_buffer();
}
