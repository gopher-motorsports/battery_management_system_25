/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include "telemetry.h"
#include "main.h"
#include "debug.h"
#include <string.h>
#include "cmsis_os.h"
#include "utils.h"
#include "packData.h"
#include "telemetryStatistics.h"
#include "soc.h"
#include <stdlib.h>

/* ==================================================================== */
/* ============================= DEFINES ============================== */
/* ==================================================================== */

#define NUM_COMMAND_BLOCK_RETRYS            3

#define NUM_CELL_TEMP_ADCS                  8
#define BOARD_TEMP_ADC_INDEX                8

// Mapping of pack monitor aux voltages
#define REG_TEMP_AUX_INDEX              0
#define SHUNT_TEMP1_AUX_INDEX           1
#define SHUNT_TEMP2_AUX_INDEX           4
#define PRECHARGE_TEMP_AUX_INDEX        2
#define DISCHARGE_TEMP_AUX_INDEX        7
#define LINK_PLUS_AUX_INDEX             3
#define LINK_MINUS_AUX_INDEX            5

// Pack monitor hv divider gain
#define VBAT_DIVIDER_INV_GAIN           247.0f
#define LINK_DIVIDER_INV_GAIN           483.353f

// Shunt characteristics
#define SHUNT_REF_RESISTANCE_UOHM       100.0f
#define SHUNT_REF_TEMP_C                25.0f
#define SHUNT_RESISTANCE_GAIN_UOHM      0.005f

// Conversion counter information
#define PACK_MON_ACCN_SETTING           ACCUMULATE_24_SAMPLES
#define ACCUMULATION_REGISTER_COUNT     ((PACK_MON_ACCN_SETTING + 1) * 4)
#define PHASE_COUNTS_PER_CONVERSION     4
#define IADC_QUALIFICATION_TIME_MS      136

#define ACCUMULATED_CURRENT_THRES_UV    100

#define MICROVOLTS_PER_VOLT             1000000.0f;

#define MAX_13BIT_UINT                  0x1FFF

#define CONVERSION_BUFFER_SIZE          100


/* ==================================================================== */
/* ========================= ENUMERATED TYPES========================== */
/* ==================================================================== */

typedef enum
{
    COUNTER_INDEX = 0,
    TIMER_INDEX,
    NUM_CONVERSION_BUFFER_INDEXES
} CONVERSION_BUFFER_INDEX_E;


/* ==================================================================== */
/* ========================= LOCAL VARIABLES ========================== */
/* ==================================================================== */

static ADBMS_BatteryData batteryData;

static uint32_t conversionCounterBuffer[NUM_CONVERSION_BUFFER_INDEXES][CONVERSION_BUFFER_SIZE];
static uint32_t counterBufferIndex = 0;

extern TIM_HandleTypeDef htim5;

/* ==================================================================== */
/* =================== LOCAL FUNCTION DECLARATIONS ==================== */
/* ==================================================================== */

static TRANSACTION_STATUS_E runCommandBlock(TRANSACTION_STATUS_E (*telemetryFunction)(telemetryTaskData_S*), telemetryTaskData_S *taskData);

static TRANSACTION_STATUS_E initChain(telemetryTaskData_S *taskData);

static TRANSACTION_STATUS_E startNewReadCycle(telemetryTaskData_S *taskData);
static TRANSACTION_STATUS_E updateDeviceStatus(telemetryTaskData_S *taskData);
static TRANSACTION_STATUS_E updateAuxPackTelemetry(telemetryTaskData_S *taskData);
static TRANSACTION_STATUS_E updatePrimaryPackTelemetry(telemetryTaskData_S *taskData);
static TRANSACTION_STATUS_E runDeviceDiagnostics(telemetryTaskData_S *taskData);

/* ==================================================================== */
/* =================== LOCAL FUNCTION DEFINITIONS ===================== */
/* ==================================================================== */


static TRANSACTION_STATUS_E runCommandBlock(TRANSACTION_STATUS_E (*telemetryFunction)(telemetryTaskData_S*), telemetryTaskData_S *taskData)
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

static TRANSACTION_STATUS_E initChain(telemetryTaskData_S *taskData)
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
    if((batteryData.cellMonitor[NUM_CELL_MON_IN_ACCUMULATOR - 1].serialId[REGISTER_BYTE0] & DEVICE_ID_MASK) == PACK_MONITOR_ID)
    {
        batteryData.chainInfo.packMonitorPort = PORTB;
    }

    // Set configuration for Group A

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

    // pack mon cnfg a settings
    batteryData.packMonitor.configGroupA.v4Reference = BASIC_REF_1_25V;
    batteryData.packMonitor.configGroupA.v6Reference = BASIC_REF_1_25V;
    batteryData.packMonitor.configGroupA.accumulationSetting = PACK_MON_ACCN_SETTING;

    batteryData.packMonitor.configGroupA.gpo1HighZMode = 0;
    batteryData.packMonitor.configGroupA.gpo1State = 1;

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

    status = readStatusC(&batteryData);
    if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    {
        return status;
    }

    if(batteryData.packMonitor.statusGroupC.resetDetected)
    {
        taskData->packMonitor.adcConversionPhaseCounter = 0;
        taskData->packMonitor.nextQualifiedPhaseCount = ((uint32_t)(IADC_QUALIFICATION_TIME_MS / ACCUMULATION_REGISTER_COUNT) + 2) * ACCUMULATION_REGISTER_COUNT * PHASE_COUNTS_PER_CONVERSION;
    }

    status = clearAllFlags(&batteryData);
    if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    {
        return status;
    }

    status = startCellConversions(&batteryData, REDUNDANT_MODE, CONTINOUS_MODE, DISCHARGE_DISABLED, FILTER_RESET, CELL_OPEN_WIRE_DISABLED);
    if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    {
        return status;
    }

    // Reset stuff

    status = startAuxConversions(&batteryData, AUX_ALL_CHANNELS, AUX_OPEN_WIRE_DISABLED);
    if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    {
        return status;
    }

    status = startPackVoltageConversions(&batteryData, PACK_ALL_CHANNELS, PACK_OPEN_WIRE_DISABLED);
    if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    {
        return status;
    }

    return readSerialId(&batteryData);
}

static TRANSACTION_STATUS_E startNewReadCycle(telemetryTaskData_S *taskData)
{
    readyChain(&batteryData);

    TRANSACTION_STATUS_E status;

    // If the chain is broken in anyway, attempt to correct the chain at the start of a new cycle
    // If a correction occurs, and a POR or command counter error is detected as a result, that will be returned by status
    status = checkChainStatus(&batteryData);

    // Unfreeze read registers
    if((status == TRANSACTION_SUCCESS) || (status == TRANSACTION_CHAIN_BREAK_ERROR))
    {
        status = unfreezeRegisters(&batteryData);
    }

    // vTaskSuspendAll();

    // Freeze read registers for new read cycle
    if((status == TRANSACTION_SUCCESS) || (status == TRANSACTION_CHAIN_BREAK_ERROR))
    {
        status = freezeRegisters(&batteryData);
    }

    if((status == TRANSACTION_SUCCESS) || (status == TRANSACTION_CHAIN_BREAK_ERROR))
    {
        conversionCounterBuffer[TIMER_INDEX][counterBufferIndex] = __HAL_TIM_GetCounter(&htim5);
    }

    // xTaskResumeAll();

    // Update conversion timer
    // updateTimer(&taskData->packMonitor.localPhaseCountTimer);

    // Verify command counter after freeze commands
    if((status == TRANSACTION_SUCCESS) || (status == TRANSACTION_CHAIN_BREAK_ERROR))
    {
        status = readSerialId(&batteryData);
    }

    return status;
}

static TRANSACTION_STATUS_E updateDeviceStatus(telemetryTaskData_S *taskData)
{
    TRANSACTION_STATUS_E status;

    // Record the last phase count stored in the static battery data struct. 0 on init
    uint32_t lastPhaseCount = batteryData.packMonitor.statusGroupC.conversionCounter1;

    status = readStatusA(&batteryData);

    if((status == TRANSACTION_SUCCESS) || (status == TRANSACTION_CHAIN_BREAK_ERROR))
    {
        status = readStatusB(&batteryData);
    }

    if((status == TRANSACTION_SUCCESS) || (status == TRANSACTION_CHAIN_BREAK_ERROR))
    {
        status = readStatusC(&batteryData);
    }

    if((status == TRANSACTION_SUCCESS) || (status == TRANSACTION_CHAIN_BREAK_ERROR))
    {
        status = readStatusD(&batteryData);
    }

    if((status == TRANSACTION_SUCCESS) || (status == TRANSACTION_CHAIN_BREAK_ERROR))
    {
        status = readStatusE(&batteryData);
    }

    // Check for reset pack monitor
    if(batteryData.packMonitor.statusGroupC.resetDetected)
    {
        // Reset conversion counter buffer
        memset(conversionCounterBuffer[COUNTER_INDEX], 0, (CONVERSION_BUFFER_SIZE * sizeof(uint32_t)));
        // counterBufferIndex = 0;

        return TRANSACTION_POR_ERROR;
    }
    
    for(uint32_t i = 0; i < NUM_CELL_MON_IN_ACCUMULATOR; i++)
    {
        // Check for sleepy BMBs
        if(batteryData.cellMonitor[i].statusGroupC.sleepDetected)
        {
            return TRANSACTION_POR_ERROR;
        }

        // Cell monitor status sensors

        // Update reference voltage
        taskData->bmb[i].referenceVoltage = batteryData.cellMonitor[i].statusGroupA.referenceVoltage;
        taskData->bmb[i].referenceVoltageStatus = GOOD;

        // Update die temp
        taskData->bmb[i].dieTemp = batteryData.cellMonitor[i].statusGroupA.dieTemp;
        taskData->bmb[i].dieTempStatus = GOOD;

        // Update digital supply voltage
        taskData->bmb[i].digitalSupplyVoltage = batteryData.cellMonitor[i].statusGroupB.digitalSupplyVoltage;
        taskData->bmb[i].digitalSupplyVoltageStatus = GOOD;

        // Update analog supply voltage
        taskData->bmb[i].analogSupplyVoltage = batteryData.cellMonitor[i].statusGroupB.analogSupplyVoltage;
        taskData->bmb[i].analogSupplyVoltageStatus = GOOD;

        // Update reference resistor voltage
        taskData->bmb[i].referenceResistorVoltage = batteryData.cellMonitor[i].statusGroupB.referenceResistorVoltage;
        taskData->bmb[i].referenceResistorVoltageStatus = GOOD;

    }

    // Pack monitor status sensors

    // Reference voltage
    taskData->packMonitor.referenceVoltage1P25 = batteryData.packMonitor.statusGroupA.referenceVoltage1P25;
    taskData->packMonitor.referenceVoltage1P25Status = GOOD;

    // Die Temp 1
    taskData->packMonitor.dieTemp1 = batteryData.packMonitor.statusGroupA.dieTemp1;
    taskData->packMonitor.dieTemp1Status = GOOD;

    // Regulator voltage
    taskData->packMonitor.regulatorVoltage = batteryData.packMonitor.statusGroupA.regulatorVoltage;
    taskData->packMonitor.regulatorVoltageStatus = GOOD;

    // Supply voltage
    taskData->packMonitor.supplyVoltage = batteryData.packMonitor.statusGroupB.supplyVoltage;
    taskData->packMonitor.supplyVoltageStatus = GOOD;

    // Digital supply voltage
    taskData->packMonitor.digitalSupplyVoltage = batteryData.packMonitor.statusGroupB.digitalSupplyVoltage;
    taskData->packMonitor.digitalSupplyVoltageStatus = GOOD;

    // Ground pad voltage
    taskData->packMonitor.groundPadVoltage = batteryData.packMonitor.statusGroupB.groundPadVoltage;
    taskData->packMonitor.groundPadVoltageStatus = GOOD;

    // Resistor reference
    taskData->packMonitor.referenceResistorVoltage = batteryData.packMonitor.statusGroupD.referenceResistorVoltage;
    taskData->packMonitor.referenceResistorVoltageStatus = GOOD;

    // Die temp 2
    taskData->packMonitor.dieTemp2 = batteryData.packMonitor.statusGroupD.dieTemp2;
    taskData->packMonitor.dieTemp2Status = GOOD;


    //TODO fix this mess

    // Revision code will return non zero value only if pack monitor is connected
    if(batteryData.packMonitor.statusGroupE.revisionCode)
    {
        // Update the conversion phase counter
        uint32_t phaseCount = batteryData.packMonitor.statusGroupC.conversionCounter1;
        uint32_t deltaPhaseCount = (phaseCount - lastPhaseCount) % (MAX_13BIT_UINT + 1);
        taskData->packMonitor.adcConversionPhaseCounter += deltaPhaseCount;

        conversionCounterBuffer[COUNTER_INDEX][counterBufferIndex] = taskData->packMonitor.adcConversionPhaseCounter;

        uint32_t currentCounter = conversionCounterBuffer[COUNTER_INDEX][counterBufferIndex];
        uint32_t currentTimer = conversionCounterBuffer[TIMER_INDEX][counterBufferIndex];

        // Counter buffer index
        counterBufferIndex++;
        counterBufferIndex %= CONVERSION_BUFFER_SIZE;

        uint32_t pastCounter = conversionCounterBuffer[COUNTER_INDEX][counterBufferIndex];
        uint32_t pastTimer = conversionCounterBuffer[TIMER_INDEX][counterBufferIndex];

        uint32_t deltaCounter = (currentCounter - pastCounter);

        // If there is a 0 stored in the next buffer index, the buffer is not full, and the conversion time calibration is not preformed
        if((pastCounter) && (deltaCounter))
        {
            uint32_t deltaTimerUS = (currentTimer - pastTimer);
            float deltaTimerPhaseCounts = ((float)deltaTimerUS / MICROSECONDS_IN_MILLISECOND) * PHASE_COUNTS_PER_CONVERSION;

            taskData->packMonitor.adcConversionTimeMS = (deltaTimerPhaseCounts) / (deltaCounter);
            
        }
        else
        {
            // Do nothing?? Let it be 0 or unchanged?
            // taskData->packMonitor.adcConversionTimeMS = 1.0f;
        }
    
    }
    else
    {
        // Reset conversion counter buffer
        memset(conversionCounterBuffer[COUNTER_INDEX], 0, (CONVERSION_BUFFER_SIZE * sizeof(uint32_t)));
    }

    return status;
}

static TRANSACTION_STATUS_E updateAuxPackTelemetry(telemetryTaskData_S *taskData)
{
    TRANSACTION_STATUS_E status;

    // Cell monitor data: all cell temps, S1N voltage, HV supply voltage
    // Pack monitor data: all aux voltages, reference and redundance reference voltage
    status = readAuxVoltages(&batteryData);

    // Filter and assign all cell temps and board temps
    for(uint32_t i = 0; i < NUM_CELL_MON_IN_ACCUMULATOR; i++)
    {
        // Cell indexes are offset depending on the mux state, which is set by gpio10
        uint32_t cellOffset = batteryData.cellMonitor[i].configGroupA.gpo10State;

        // Cell temps
        for(uint32_t j = 0; j < NUM_CELL_TEMP_ADCS; j++)
        {
            taskData->bmb[i].cellTemp[(j * 2) + cellOffset] = lookup(batteryData.cellMonitor[i].auxVoltage[j], &cellMonTempTable);
            taskData->bmb[i].cellTempStatus[(j * 2) + cellOffset] = GOOD;
        }

        // Board temp
        taskData->bmb[i].boardTemp = lookup(batteryData.cellMonitor[i].auxVoltage[BOARD_TEMP_ADC_INDEX], &cellMonTempTable);
        taskData->bmb[i].boardTempStatus = GOOD;
    }

    // Translate pack monitor aux voltages

    // Regulator temp
    taskData->packMonitor.boardTemp = lookup(batteryData.packMonitor.auxVoltage[REG_TEMP_AUX_INDEX], &packMonTempTable);
    taskData->packMonitor.boardTempStatus = GOOD;

    // Shunt temp 1
    taskData->packMonitor.shuntTemp1 = lookup(batteryData.packMonitor.auxVoltage[SHUNT_TEMP1_AUX_INDEX], &packMonTempTable);
    taskData->packMonitor.shuntTemp1Status = GOOD;

    // Shunt temp 2
    taskData->packMonitor.shuntTemp2 = lookup(batteryData.packMonitor.auxVoltage[SHUNT_TEMP2_AUX_INDEX], &packMonTempTable);
    taskData->packMonitor.shuntTemp2Status = GOOD;

    // Precharge Temp
    taskData->packMonitor.prechargeTemp = lookup(batteryData.packMonitor.auxVoltage[PRECHARGE_TEMP_AUX_INDEX], &packMonTempTable);
    taskData->packMonitor.prechargeTempStatus = GOOD;

    // Discharge Temp
    taskData->packMonitor.dischargeTemp = lookup(batteryData.packMonitor.auxVoltage[DISCHARGE_TEMP_AUX_INDEX], &packMonTempTable);
    taskData->packMonitor.dischargeTempStatus = GOOD;

    // TODO SET DIFF ADC
    // Link voltage 
    // Link+ and Link- measured referenced to 1.25v ref, subtract to get link voltage
    float linkVoltage = LINK_DIVIDER_INV_GAIN * (batteryData.packMonitor.auxVoltage[LINK_PLUS_AUX_INDEX] - batteryData.packMonitor.auxVoltage[LINK_MINUS_AUX_INDEX]);
    taskData->packMonitor.linkVoltage = linkVoltage;
    taskData->packMonitor.linkVoltageStatus = GOOD;

    // TODO add error checking on shunt temp
    // Calculate the shunt resistance
    float shuntRes = SHUNT_REF_RESISTANCE_UOHM + SHUNT_RESISTANCE_GAIN_UOHM * (taskData->packMonitor.shuntTemp1 - SHUNT_REF_TEMP_C);
    taskData->packMonitor.shuntResistanceMicroOhms = shuntRes;

    // Restart cell monitor AUX adcs
    if((status == TRANSACTION_SUCCESS) || (status == TRANSACTION_CHAIN_BREAK_ERROR))
    {
        status = startAuxConversions(&batteryData, AUX_ALL_CHANNELS, AUX_OPEN_WIRE_DISABLED);
    }

    // Restart pack monitor AUX adcs
    if((status == TRANSACTION_SUCCESS) || (status == TRANSACTION_CHAIN_BREAK_ERROR))
    {
        status = startPackVoltageConversions(&batteryData, PACK_ALL_CHANNELS, PACK_OPEN_WIRE_DISABLED);
    }

    // Toggle temperature sensor mux
    if((status == TRANSACTION_SUCCESS) || (status == TRANSACTION_CHAIN_BREAK_ERROR))
    {
        for(uint32_t i = 0; i < NUM_CELL_MON_IN_ACCUMULATOR; i++)
        {
            batteryData.cellMonitor[i].configGroupA.gpo10State ^= 1;
        }
        status = writeConfigA(&batteryData);
    }

    // Verify command counter and mux states
    if((status == TRANSACTION_SUCCESS) || (status == TRANSACTION_CHAIN_BREAK_ERROR))
    {
        status = readConfigA(&batteryData);
    }

    return status;
    
}

static TRANSACTION_STATUS_E updatePrimaryPackTelemetry(telemetryTaskData_S *taskData)
{
    TRANSACTION_STATUS_E status;

    // Cell monitor data: all cell voltages translated to floats
    // Pack monitor data: raw current sensor uV, pack voltage, raw current counter uV, raw pack voltage counter uV
    status = readCellVoltages(&batteryData, FILTERED_CELL_VOLTAGE);

    // Filter and assign all voltages to task data struct
    for(uint32_t i = 0; i < NUM_CELL_MON_IN_ACCUMULATOR; i++)
    {
        for(uint32_t j = 0; j < NUM_CELLS_PER_CELL_MONITOR; j++)
        {
            // Add filtering here
            taskData->bmb[i].cellVoltage[j] = batteryData.cellMonitor[i].cellVoltage[j];
            taskData->bmb[i].cellVoltageStatus[j] = GOOD;

        }
    }

    // Translate pack monitor sensors

    // Pack current
    taskData->packMonitor.packCurrent = batteryData.packMonitor.currentAdc1uV / (taskData->packMonitor.shuntResistanceMicroOhms);
    taskData->packMonitor.packCurrentStatus = GOOD;

    // Pack voltage
    taskData->packMonitor.packVoltage = batteryData.packMonitor.batteryVoltage1 * VBAT_DIVIDER_INV_GAIN;
    taskData->packMonitor.packVoltageStatus = GOOD;

    // Pack Energy
    taskData->packMonitor.packPower = taskData->packMonitor.packCurrent * taskData->packMonitor.packVoltage;
    taskData->packMonitor.packPowerStatus = GOOD;

    // Problem here where disconnect and reconnect spi
    // Update accumulation data
    if(taskData->packMonitor.adcConversionPhaseCounter >= taskData->packMonitor.nextQualifiedPhaseCount)
    {
        // Calculate next valid conversion phase count
        taskData->packMonitor.nextQualifiedPhaseCount += (ACCUMULATION_REGISTER_COUNT * PHASE_COUNTS_PER_CONVERSION);

        // Update coulomb counter
        float accumulatedCurrent = batteryData.packMonitor.currentAdcAccumulator1uV / (taskData->packMonitor.shuntResistanceMicroOhms);
        taskData->packMonitor.socData.milliCoulombCounter += (accumulatedCurrent * taskData->packMonitor.adcConversionTimeMS);

        // Update energy counter
        float accumulatedVoltage = batteryData.packMonitor.batteryVoltageAccumulator1uV / MICROVOLTS_PER_VOLT;
        taskData->packMonitor.packEnergyMilliJoules += (accumulatedVoltage * accumulatedCurrent * taskData->packMonitor.adcConversionTimeMS);

        // Update soc by ocv qualification timer
        if(abs(batteryData.packMonitor.currentAdcAccumulator1uV) > ACCUMULATED_CURRENT_THRES_UV)
        {
            clearTimer(&taskData->packMonitor.socData.socByOcvQualificationTimer);
        }
        else
        {
            updateTimer(&taskData->packMonitor.socData.socByOcvQualificationTimer);
        }
    }

    return status;
}

static TRANSACTION_STATUS_E runDeviceDiagnostics(telemetryTaskData_S *taskData)
{
    TRANSACTION_STATUS_E status = readRedundantCellVoltages(&batteryData);

    if((status == TRANSACTION_SUCCESS) || (status == TRANSACTION_CHAIN_BREAK_ERROR))
    {
        status = readRedundantAuxVoltages(&batteryData);
    }

    return status;
}

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DEFINITIONS ==================== */
/* ==================================================================== */

TRANSACTION_STATUS_E updateBatteryTelemetry(telemetryTaskData_S *taskData)
{
    TRANSACTION_STATUS_E telemetryStatus;

    if(taskData->chainInitialized)
    {
        telemetryStatus = runCommandBlock(startNewReadCycle, taskData);

        if((telemetryStatus == TRANSACTION_SUCCESS) || (telemetryStatus == TRANSACTION_CHAIN_BREAK_ERROR))
        {
            telemetryStatus = runCommandBlock(updateDeviceStatus, taskData);
        }

        if((telemetryStatus == TRANSACTION_SUCCESS) || (telemetryStatus == TRANSACTION_CHAIN_BREAK_ERROR))
        {
            telemetryStatus = runCommandBlock(updateAuxPackTelemetry, taskData);
        }

        if((telemetryStatus == TRANSACTION_SUCCESS) || (telemetryStatus == TRANSACTION_CHAIN_BREAK_ERROR))
        {
            telemetryStatus = runCommandBlock(updatePrimaryPackTelemetry, taskData);
        }

        if((telemetryStatus == TRANSACTION_SUCCESS) || (telemetryStatus == TRANSACTION_CHAIN_BREAK_ERROR))
        {
            telemetryStatus = runCommandBlock(runDeviceDiagnostics, taskData);
        }

        if((telemetryStatus == TRANSACTION_SUCCESS) || (telemetryStatus == TRANSACTION_CHAIN_BREAK_ERROR))
        {
            // Update statistics
            updateBatteryStatistics(taskData);

            // Update SOC
            updateSocSoe(&taskData->packMonitor.socData, taskData->minCellVoltage);
        }
    }

    if(!taskData->chainInitialized || (telemetryStatus == TRANSACTION_POR_ERROR))
    {
        Debug("Initializing chain...\n");

        telemetryStatus = runCommandBlock(initChain, taskData);

        if((telemetryStatus == TRANSACTION_SUCCESS) || (telemetryStatus == TRANSACTION_CHAIN_BREAK_ERROR))
        {
            Debug("Chain initialization successful!\n");
            taskData->chainInitialized = true;
        }
        else
        {
            Debug("Chain failed to initialize!\n");
            taskData->chainInitialized = false;
        }
    }

    return telemetryStatus;
}
