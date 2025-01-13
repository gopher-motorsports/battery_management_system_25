/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include "telemetry.h"
#include "debug.h"
#include <string.h>
#include "cmsis_os.h"

/* ==================================================================== */
/* ============================= DEFINES ============================== */
/* ==================================================================== */

#define NUM_COMMAND_BLOCK_RETRYS            3

/* ==================================================================== */
/* ========================= LOCAL VARIABLES ========================== */
/* ==================================================================== */

static ADBMS_BatteryData batteryData =
{
    .chainInfo.numDevs = NUM_DEVICES_IN_ACCUMULATOR,
    .chainInfo.packMonitorPort = PORTA,
    .chainInfo.chainStatus = CHAIN_COMPLETE,
    .chainInfo.availableDevices[PORTA] = NUM_DEVICES_IN_ACCUMULATOR,
    .chainInfo.availableDevices[PORTB] = NUM_DEVICES_IN_ACCUMULATOR,
    .chainInfo.currentPort = PORTA,
    .chainInfo.localCommandCounter[CELL_MONITOR] = 0,
    .chainInfo.localCommandCounter[PACK_MONITOR] = 0
};

/* ==================================================================== */
/* =================== LOCAL FUNCTION DECLARATIONS ==================== */
/* ==================================================================== */

/* ==================================================================== */
/* =================== LOCAL FUNCTION DEFINITIONS ===================== */
/* ==================================================================== */

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DEFINITIONS ==================== */
/* ==================================================================== */

TRANSACTION_STATUS_E runCommandBlock(TRANSACTION_STATUS_E (*telemetryFunction)(telemetryTaskData_S*), telemetryTaskData_S *taskData)
{
    for(uint32_t attempt = 0; attempt < NUM_COMMAND_BLOCK_RETRYS; attempt++)
    {
        // Run the telemetry function
        TRANSACTION_STATUS_E status = telemetryFunction(taskData);

        // Check return status
        if(status == TRANSACTION_COMMAND_COUNTER_ERROR)
        {
            // On command counter error, retry the command block
            Debug("Command counter mismatch! Retrying command block!\n");
            continue;
        }
        else
        {
            // On all other errors, return error
            return status;
        }
    }

    // After the max function attempts, return command counter error
    return TRANSACTION_COMMAND_COUNTER_ERROR;
}

TRANSACTION_STATUS_E initChain(telemetryTaskData_S *taskData)
{
    wakeChain(&batteryData);

    TRANSACTION_STATUS_E status = muteDischarge(&batteryData);
    if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    {
        return status;
    }

    status = clearAllVoltageRegisters(&batteryData);
    if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    {
        return status;
    }

    // Weird bug if this comes before clearAllVoltageRegisters
    status = clearAllFlags(&batteryData);
    if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    {
        return status;
    }

    return startCellConversions(&batteryData, REDUNDANT_MODE, CONTINOUS_MODE, DISCHARGE_DISABLED, NO_FILTER_RESET, CELL_OPEN_WIRE_DISABLED);
}

TRANSACTION_STATUS_E testBlock(telemetryTaskData_S *taskData)
{
    readyChain(&batteryData);

    // ISOSPI status variable
    TRANSACTION_STATUS_E status;

    status = checkChainStatus(&batteryData);
    if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    {
        return status;
    }

    status = unfreezeRegisters(&batteryData);
    if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    {
        return status;
    }

    status = freezeRegisters(&batteryData);
    if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    {
        return status;
    }

    status = muteDischarge(&batteryData);
    if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    {
        return status;
    }

    status = startAuxConversions(&batteryData, AUX_ALL_CHANNELS, AUX_OPEN_WIRE_DISABLED);
    if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    {
        return status;
    }

    status = startPackAuxillaryConversions(&batteryData);
    if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    {
        return status;
    }

    // batteryData.packMonitor.configGroupA.v1Reference = ADVANCED_REF_V3;
    // batteryData.packMonitor.configGroupA.v3Reference = BASIC_REF_1_25V;
    // batteryData.packMonitor.configGroupA.v5Reference = BASIC_REF_1_25V;

    // batteryData.cellMonitor[0].configGroupA.comparisonThreshold = COMPARE_THRESHOLD_10_mV;
    // batteryData.cellMonitor[0].configGroupA.asserSingleTrimError = 1;
    // batteryData.cellMonitor[0].configGroupA.soakTime = AUX_SOAK_TIME_262_MS;
    // batteryData.cellMonitor[0].configGroupA.gpo6State = 1;
    // batteryData.cellMonitor[0].configGroupA.digitalFilterSetting = FILTER_CUTOFF_45_HZ;

    status = writeConfigA(&batteryData);
    if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    {
        return status;
    }

    status = readConfigA(&batteryData);
    if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    {
        return status;
    }

    status = readStatusA(&batteryData);
    if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    {
        return status;
    }

    status = readStatusB(&batteryData);
    if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    {
        return status;
    }

    status = readStatusC(&batteryData);
    if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    {
        return status;
    }

    status = readStatusD(&batteryData);
    if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    {
        return status;
    }

    status = readStatusE(&batteryData);
    if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    {
        return status;
    }

    return status;
}
