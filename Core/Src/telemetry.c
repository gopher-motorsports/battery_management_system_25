/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include "telemetry.h"
#include "debug.h"
#include <string.h>
#include "cmsis_os.h"
#include "utils.h"
#include "packData.h"

/* ==================================================================== */
/* ============================= DEFINES ============================== */
/* ==================================================================== */

#define NUM_COMMAND_BLOCK_RETRYS            3

#define NUM_CELL_TEMP_ADCS                  8
#define BOARD_TEMP_ADC_INDEX                8

/* ==================================================================== */
/* ========================= ENUMERATED TYPES========================== */
/* ==================================================================== */

typedef enum
{
    MUX_STATE_0 = 0,
    MUX_STATE_1,
    NUM_MUX_STATES
} MUX_STATE_E;

/* ==================================================================== */
/* ========================= LOCAL VARIABLES ========================== */
/* ==================================================================== */

static ADBMS_BatteryData batteryData;
static MUX_STATE_E tempMuxState;

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

    TRANSACTION_STATUS_E status;

    // Reset chain info struct to default values
    batteryData.chainInfo.numDevs = NUM_DEVICES_IN_ACCUMULATOR;
    batteryData.chainInfo.packMonitorPort = PORTA;
    batteryData.chainInfo.chainStatus = CHAIN_COMPLETE;
    batteryData.chainInfo.availableDevices[PORTA] = NUM_DEVICES_IN_ACCUMULATOR;
    batteryData.chainInfo.availableDevices[PORTB] = NUM_DEVICES_IN_ACCUMULATOR;
    batteryData.chainInfo.currentPort = PORTA;
    batteryData.chainInfo.localCommandCounter[CELL_MONITOR] = 0;
    batteryData.chainInfo.localCommandCounter[PACK_MONITOR] = 0;

    status = readSerialId(&batteryData);
    if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    {
        return status;
    }

    // TODO return error if 2 or 0 pack monitors found
    if((batteryData.cellMonitor[NUM_CELL_MON_IN_ACCUMULATOR - 1].serialId[REGISTER_BYTE5] & DEVICE_ID_MASK) == PACK_MONITOR_ID)
    {
        batteryData.chainInfo.packMonitorPort = PORTB;
    }

    for(uint32_t i = 0; i < NUM_CELL_MON_IN_ACCUMULATOR; i++)
    {
        batteryData.cellMonitor[i].configGroupA.gpo1State = 1;
        batteryData.cellMonitor[i].configGroupA.gpo2State = 1;
        batteryData.cellMonitor[i].configGroupA.gpo3State = 1;
        batteryData.cellMonitor[i].configGroupA.gpo4State = 1;
        batteryData.cellMonitor[i].configGroupA.gpo5State = 1;
        batteryData.cellMonitor[i].configGroupA.gpo6State = 1;
        batteryData.cellMonitor[i].configGroupA.gpo7State = 1;
        batteryData.cellMonitor[i].configGroupA.gpo8State = 1;
        batteryData.cellMonitor[i].configGroupA.gpo9State = 1;
        batteryData.cellMonitor[i].configGroupA.gpo10State = 0;
    }

    status = writeConfigA(&batteryData);
    if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    {
        return status;
    }

    // for(uint32_t i = 0; i < NUM_CELL_MON_IN_ACCUMULATOR; i++)
    // {
    //     for(uint32_t j = 0; j < NUM_CELLS_PER_CELL_MONITOR; j++)
    //     {
    //         batteryData.cellMonitor[i].configGroupB.dischargeCell[j] = 1;
    //     }
    // }

    // status = writeConfigB(&batteryData);
    // if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    // {
    //     return status;
    // }

    status = startCellConversions(&batteryData, REDUNDANT_MODE, CONTINOUS_MODE, DISCHARGE_DISABLED, FILTER_RESET, CELL_OPEN_WIRE_DISABLED);
    if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    {
        return status;
    }

    status = startAuxConversions(&batteryData, AUX_ALL_CHANNELS, AUX_OPEN_WIRE_DISABLED);
    if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    {
        return status;
    }

    return readSerialId(&batteryData);
}

TRANSACTION_STATUS_E startNewReadCycle(telemetryTaskData_S *taskData)
{
    readyChain(&batteryData);

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

    return readSerialId(&batteryData);
}

TRANSACTION_STATUS_E updateDeviceStatus(telemetryTaskData_S *taskData)
{
    TRANSACTION_STATUS_E status;

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

TRANSACTION_STATUS_E updateBatteryTelemetry(telemetryTaskData_S *taskData)
{
    TRANSACTION_STATUS_E status;

    status = readCellVoltages(&batteryData, FILTERED_CELL_VOLTAGE);

    for(uint32_t i = 0; i < NUM_CELL_MON_IN_ACCUMULATOR; i++)
    {
        for(uint32_t j = 0; j < NUM_CELLS_PER_CELL_MONITOR; j++)
        {
            float cellVoltage = batteryData.cellMonitor[i].cellVoltage[j];

            if(fequals(cellVoltage, 1.5f))
            {
                taskData->bmb[i].cellVoltageStatus[j] = BAD;
            }
            else
            {
                taskData->bmb[i].cellVoltage[j] = cellVoltage;
                taskData->bmb[i].cellVoltageStatus[j] = GOOD;
            }

        }
    }

    if((status == TRANSACTION_SUCCESS) || (status == TRANSACTION_CHAIN_BREAK_ERROR))
    {
        status = readAuxVoltages(&batteryData);
    }

    uint32_t cellOffset = (uint32_t)tempMuxState;

    for(uint32_t i = 0; i < NUM_CELL_MON_IN_ACCUMULATOR; i++)
    {
        for(uint32_t j = 0; j < NUM_CELL_TEMP_ADCS; j++)
        {
            float auxVoltage = batteryData.cellMonitor[i].auxVoltage[j];

            if(fequals(auxVoltage, 1.5f))
            {
                taskData->bmb[i].cellTempStatus[(j * 2) + cellOffset] = BAD;
            }
            else
            {
                taskData->bmb[i].cellTemp[(j * 2) + cellOffset] = lookup(auxVoltage, &cellTempTable);
                taskData->bmb[i].cellTempStatus[(j * 2) + cellOffset] = GOOD;
            }
        }

        float auxVoltage = batteryData.cellMonitor[i].auxVoltage[BOARD_TEMP_ADC_INDEX];

        if(fequals(auxVoltage, 1.5f))
        {
            taskData->bmb[i].boardTempStatus = BAD;
        }
        else
        {
            taskData->bmb[i].boardTemp = lookup(auxVoltage, &boardTempTable);
            taskData->bmb[i].boardTempStatus = GOOD;
        }
    }

    if((status == TRANSACTION_SUCCESS) || (status == TRANSACTION_CHAIN_BREAK_ERROR))
    {
        tempMuxState++;
        tempMuxState %= NUM_MUX_STATES;

        for(uint32_t i = 0; i < NUM_CELL_MON_IN_ACCUMULATOR; i++)
        {
            batteryData.cellMonitor[i].configGroupA.gpo10State = (uint8_t)tempMuxState;
        }

        status = writeConfigA(&batteryData);
    }

    if((status == TRANSACTION_SUCCESS) || (status == TRANSACTION_CHAIN_BREAK_ERROR))
    {
        status = startAuxConversions(&batteryData, AUX_ALL_CHANNELS, AUX_OPEN_WIRE_DISABLED);
    }

    if((status == TRANSACTION_SUCCESS) || (status == TRANSACTION_CHAIN_BREAK_ERROR))
    {
        status = readConfigA(&batteryData);
    }

    return status;
}

TRANSACTION_STATUS_E runDeviceDiagnostics(telemetryTaskData_S *taskData)
{
    TRANSACTION_STATUS_E status;

    status = readRedundantCellVoltages(&batteryData);
    if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    {
        return status;
    }

    status = readRedundantAuxVoltages(&batteryData);
    if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    {
        return status;
    }

    return status;
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

    // status = startRedundantCellConversions(&batteryData, SINGLE_SHOT_MODE, DISCHARGE_DISABLED, CELL_OPEN_WIRE_DISABLED);
    // if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    // {
    //     return status;
    // }

    // status = startAuxConversions(&batteryData, AUX_ALL_CHANNELS, AUX_OPEN_WIRE_DISABLED);
    // if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    // {
    //     return status;
    // }

    // status = startRedundantAuxConversions(&batteryData, AUX_ALL_CHANNELS);
    // if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    // {
    //     return status;
    // }

    // status = startPackVoltageConversions(&batteryData, PACK_ALL_CHANNELS, PACK_OPEN_WIRE_DISABLED);
    // if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    // {
    //     return status;
    // }

    // status = startPackAuxillaryConversions(&batteryData);
    // if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    // {
    //     return status;
    // }

    // status = muteDischarge(&batteryData);
    // if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    // {
    //     return status;
    // }

    // status = unmuteDischarge(&batteryData);
    // if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    // {
    //     return status;
    // }

    // status = freezeRegisters(&batteryData);
    // if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    // {
    //     return status;
    // }

    // status = unfreezeRegisters(&batteryData);
    // if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    // {
    //     return status;
    // }

    // status = softReset(&batteryData);
    // if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    // {
    //     return status;
    // }


    // batteryData.cellMonitor[0].configGroupA.digitalFilterSetting = FILTER_CUTOFF_625_mHZ;

    // status = writeConfigA(&batteryData);
    // if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    // {
    //     return status;
    // }

    // status = readConfigA(&batteryData);
    // if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    // {
    //     return status;
    // }


    return readSerialId(&batteryData);
}
