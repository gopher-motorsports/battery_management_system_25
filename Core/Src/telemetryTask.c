/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include "telemetryTask.h"
#include "telemetry.h"
#include "main.h"
#include "cmsis_os.h"
#include "utils.h"
#include "debug.h"
#include "lookupTable.h"
#include "packData.h"
#include "cellData.h"
#include <string.h>
#include "GopherCAN.h"
#include "gopher_sense.h"
#include "alerts.h"

/* ==================================================================== */
/* ============================= DEFINES ============================== */
/* ==================================================================== */

#define FORCE_BALANCING_ON  0

// #define FAULT_TOLERANT_TIME_INTERVAL_MS     500
// #define OPEN_WIRE_SOAK_TIME_MS              40

// #define FAULT_TOLERANT_TIME_INTERVAL_CYC    (FAULT_TOLERANT_TIME_INTERVAL_MS / TELEMETRY_TASK_PERIOD_MS)
// #define OPEN_WIRE_SOAK_TIME_CYC             (OPEN_WIRE_SOAK_TIME_MS / TELEMETRY_TASK_PERIOD_MS)
// #define REDUNDANT_CHECK_TIME_CYC            1
// #define BALANCING_TIME_CYC                  (FAULT_TOLERANT_TIME_INTERVAL_CYC - (2 * OPEN_WIRE_SOAK_TIME_CYC) - (REDUNDANT_CHECK_TIME_CYC))

// #define MAX_13BIT_UINT                      0x1FFF

/* ==================================================================== */
/* ========================= LOCAL VARIABLES ========================== */
/* ==================================================================== */

// const uint8_t bmbOrder[NUM_CELL_MON_IN_ACCUMULATOR] =
// {
//     BMB0_SEGMENT_INDEX
//     // BMB1_SEGMENT_INDEX
//     // BMB2_SEGMENT_INDEX,
//     // BMB3_SEGMENT_INDEX,
//     // BMB4_SEGMENT_INDEX,
//     // BMB5_SEGMENT_INDEX,
//     // BMB6_SEGMENT_INDEX,
//     // BMB7_SEGMENT_INDEX
// };

// const uint16_t readVoltReg[NUM_CELLV_REGISTERS] =
// {
//     RDCVA_RDI, RDCVB_RDVB,
//     RDCVC, RDCVD,
//     RDCVE, RDCVF,
// };

// const uint16_t readFiltVoltReg[NUM_CELLV_REGISTERS] =
// {
//     RDFCA, RDFCB,
//     RDFCC, RDFCD,
//     RDFCE, RDFCF,
// };

// const uint16_t readDiagVoltReg[NUM_CELLV_REGISTERS] =
// {
//     RDSVA, RDSVB,
//     RDSVC, RDSVD,
//     RDSVE, RDSVF,
// };

// const uint16_t readAuxReg[NUM_AUXV_REGISTERS] =
// {
//     RDAUXA,
//     RDAUXB,
//     RDAUXC
// };


/* ==================================================================== */
/* =================== LOCAL FUNCTION DECLARATIONS ==================== */
/* ==================================================================== */

static void runTelemetryAlertMonitor(telemetryTaskData_S *telemetryData);

// static void convertCellVoltageRegister(uint8_t *bmbData, uint32_t cellStartIndex, Cell_Monitor_S *bmbArray, bool diagnosticAdc);
// static void convertCellTempRegister(uint8_t *bmbData, uint32_t cellStartIndex, Cell_Monitor_S *bmbArray);

// static TRANSACTION_STATUS_E runCommandBlock(TRANSACTION_STATUS_E (*telemetryFunction)(telemetryTaskData_S*), telemetryTaskData_S *taskData);

// static TRANSACTION_STATUS_E initChain(telemetryTaskData_S *taskData);
// static TRANSACTION_STATUS_E startNewReadCycle(telemetryTaskData_S *taskData);
// static TRANSACTION_STATUS_E updateCellVoltages(telemetryTaskData_S *taskData);
// static TRANSACTION_STATUS_E updateDiagnosticCellVoltages(telemetryTaskData_S *taskData);
// static TRANSACTION_STATUS_E updateCellTemps(telemetryTaskData_S *taskData);
// static TRANSACTION_STATUS_E updateEnergyData(telemetryTaskData_S *taskData);
// static TRANSACTION_STATUS_E runAdcDiagnostics(telemetryTaskData_S *taskData);

// static void updateBmbStatistics(Cell_Monitor_S *bmb);
// static void updatePackStatistics(telemetryTaskData_S *taskData);

/* ==================================================================== */
/* ============================== MACROS ============================== */
/* ==================================================================== */

// #define EXTRACT_16_BIT(buffer)      (((uint32_t)buffer[1] << (1 * BITS_IN_BYTE)) | ((uint32_t)buffer[0]))

// #define EXTRACT_24_BIT(buffer)      (((uint32_t)buffer[2] << (2 * BITS_IN_BYTE)) | ((uint32_t)buffer[1] << (1 * BITS_IN_BYTE)) | ((uint32_t)buffer[0]))

// #define CONVERT_VADC(reg)           (((int16_t)(EXTRACT_16_BIT(reg)) * VADC_GAIN) + VADC_OFFSET)

// #define CONVERT_VBADC1(reg)         ((int16_t)(EXTRACT_16_BIT(reg)) * VBADC1_GAIN)

// #define CONVERT_VBADC2(reg)         ((int16_t)(EXTRACT_16_BIT(reg)) * VBADC2_GAIN * -1.0f)

// #define CONVERT_IADC1(reg)          (((int32_t)(EXTRACT_24_BIT(reg) << BITS_IN_BYTE)) /  BYTE_SIZE_DEC)

// #define CONVERT_IADC2(reg)          ((((int32_t)(EXTRACT_24_BIT(reg) << BITS_IN_BYTE)) /  BYTE_SIZE_DEC) * -1)

/* ==================================================================== */
/* =================== LOCAL FUNCTION DEFINITIONS ===================== */
/* ==================================================================== */

static void runTelemetryAlertMonitor(telemetryTaskData_S *telemetryData)
{
    // Accumulate alert statuses
    bool responseStatus[NUM_ALERT_RESPONSES] = {false};

    for(int32_t i = 0; i < NUM_TELEMETRY_ALERTS; i++)
    {
        Alert_S* alert = telemetryAlerts[i];

        // Check alert condition and run alert monitor
        alert->alertConditionPresent = telemetryAlertConditionArray[i](telemetryData);
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

// static void convertCellVoltageRegister(uint8_t *bmbData, uint32_t cellStartIndex, Cell_Monitor_S *bmbArray, bool diagnosticAdc)
// {
//     int32_t cellsInReg = CELLS_PER_REG;
//     if((NUM_CELLS_PER_CELL_MONITOR - cellStartIndex) < CELLS_PER_REG)
//     {
//         cellsInReg = NUM_CELLS_PER_CELL_MONITOR - cellStartIndex;
//     }

//     for(int32_t i = 0; i < cellsInReg; i++)
//     {
//         for(int32_t j = 0; j < NUM_CELL_MON_IN_ACCUMULATOR; j++)
//         {
//             float cellVoltage = CONVERT_VADC((bmbData + (j * REGISTER_SIZE_BYTES) + (i * CELL_REG_SIZE)));
//             if(!diagnosticAdc)
//             {
//                 bmbArray[bmbOrder[j]].cellVoltage[(cellStartIndex + i)] = cellVoltage;
//                 bmbArray[bmbOrder[j]].cellVoltageStatus[(cellStartIndex + i)] = GOOD;
//             }
//             else
//             {
//                 bmbArray[bmbOrder[j]].diagnosticCellVoltage[(cellStartIndex + i)] = cellVoltage;
//                 bmbArray[bmbOrder[j]].diagnosticCellVoltageStatus[(cellStartIndex + i)] = GOOD;
//             }
//         }
//     }
// }

// static void convertCellTempRegister(uint8_t *bmbData, uint32_t cellStartIndex, Cell_Monitor_S *bmbArray)
// {
//     int32_t cellsInReg = CELLS_PER_REG;
//     if((NUM_CELLS_PER_CELL_MONITOR - cellStartIndex + 1) < (CELLS_PER_REG * 2))
//     {
//         cellsInReg = (int32_t)((NUM_CELLS_PER_CELL_MONITOR - cellStartIndex + 1) / 2);
//     }

//     for(int32_t i = 0; i < cellsInReg; i++)
//     {
//         for(int32_t j = 0; j < NUM_CELL_MON_IN_ACCUMULATOR; j++)
//         {
//             float adcVoltage = CONVERT_VADC((bmbData + (j * REGISTER_SIZE_BYTES) + (i * CELL_REG_SIZE)));
//             bmbArray[bmbOrder[j]].cellTemp[(cellStartIndex + (i * 2))] = lookup(adcVoltage, &cellTempTable);
//             bmbArray[bmbOrder[j]].cellTempStatus[(cellStartIndex + (i * 2))] = GOOD;
//         }
//     }
// }

// static TRANSACTION_STATUS_E runCommandBlock(TRANSACTION_STATUS_E (*telemetryFunction)(telemetryTaskData_S*), telemetryTaskData_S *taskData)
// {
//     for(uint32_t attempt = 0; attempt < NUM_COMMAND_BLOCK_RETRYS; attempt++)
//     {
//         // Run the telemetry function
//         TRANSACTION_STATUS_E status = telemetryFunction(taskData);

//         // Check return status
//         if(status == TRANSACTION_COMMAND_COUNTER_ERROR)
//         {
//             // On command counter error, retry the command block
//             Debug("Command counter mismatch! Retrying command block!\n");
//             continue;
//         }
//         else
//         {
//             // On all other errors, return error
//             return status;
//         }
//     }

//     // After the max function attempts, return command counter error
//     return TRANSACTION_COMMAND_COUNTER_ERROR;
// }

// static TRANSACTION_STATUS_E initChain(telemetryTaskData_S *taskData)
// {
//     // Create and clear pack monitor data buffer
//     uint8_t packMonitorDataBuffer[REGISTER_SIZE_BYTES * NUM_PACK_MON_IN_ACCUMULATOR];
//     memset(packMonitorDataBuffer, 0, REGISTER_SIZE_BYTES * NUM_PACK_MON_IN_ACCUMULATOR);

//     // Create and clear cell monitor data buffer
//     uint8_t cellMonitorDataBuffer[REGISTER_SIZE_BYTES * NUM_CELL_MON_IN_ACCUMULATOR];
//     memset(cellMonitorDataBuffer, 0, REGISTER_SIZE_BYTES * NUM_CELL_MON_IN_ACCUMULATOR);

//     TRANSACTION_STATUS_E status;

//     // Start ADCs
//     status = commandChain((ADCV | ADC_CONT | ADC_RD), &taskData->chainInfo, SHARED_COMMAND);
//     if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
//     {
//         return status;
//     }

//     clearTimer(&taskData->packMonitorData.localPhaseCountTimer);
//     taskData->packMonitorData.lastPhaseCount = 0;
//     taskData->packMonitorData.adcConversionPhaseCounter = 0;
//     taskData->packMonitorData.nextGoodAccumulationDataPhaseCount = ((uint32_t)(IADC_QUALIFICATION_TIME_MS / ACCUMULATION_REGISTER_COUNT) + 2) * ACCUMULATION_REGISTER_COUNT * PHASE_COUNTS_PER_MS;

//     // Start Aux ADC on BMBs
//     status = commandChain(ADAX, &taskData->chainInfo, CELL_MONITOR_COMMAND);
//     if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
//     {
//         return status;
//     }

//     // Clear RESET and SLEEP Bits
//     packMonitorDataBuffer[REGISTER_BYTE5] = STATC_SLEEP_BIT;
//     for(uint32_t i = 0; i < NUM_CELL_MON_IN_ACCUMULATOR; i++)
//     {
//         cellMonitorDataBuffer[(i * REGISTER_SIZE_BYTES) + REGISTER_BYTE5] = STATC_SLEEP_BIT;
//     }

//     status = writeChain(CLRFLAG, &taskData->chainInfo, SHARED_COMMAND, packMonitorDataBuffer, cellMonitorDataBuffer);
//     if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
//     {
//         return status;
//     }

//     memset(packMonitorDataBuffer, 0, REGISTER_SIZE_BYTES * NUM_PACK_MON_IN_ACCUMULATOR);
//     memset(cellMonitorDataBuffer, 0, REGISTER_SIZE_BYTES * NUM_CELL_MON_IN_ACCUMULATOR);

//     packMonitorDataBuffer[REGISTER_BYTE3] = ALL_PACK_MON_GPIO;
//     packMonitorDataBuffer[REGISTER_BYTE4] = ALL_PACK_MON_GPIO;
//     packMonitorDataBuffer[REGISTER_BYTE5] = ACCN_CTRL_BITS;

//     for(uint32_t i = 0; i < NUM_CELL_MON_IN_ACCUMULATOR; i++)
//     {
//         cellMonitorDataBuffer[REGISTER_BYTE3] = ALL_CELL_MON_GPIO;
//         cellMonitorDataBuffer[REGISTER_BYTE4] = GPIO9;
//         cellMonitorDataBuffer[REGISTER_BYTE5] = CADC_FILTER_CUTOFF_21HZ;
//     }

//     status = writeChain(WRCFGA, &taskData->chainInfo, SHARED_COMMAND, packMonitorDataBuffer, cellMonitorDataBuffer);
//     if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
//     {
//         return status;
//     }

//     // Verify command counter
//     status = readChain(RDSID, &taskData->chainInfo, packMonitorDataBuffer, cellMonitorDataBuffer);
//     if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
//     {
//         return status;
//     }

//     return status;
// }

// static TRANSACTION_STATUS_E startNewReadCycle(telemetryTaskData_S *taskData)
// {
//     // Create and clear pack monitor data buffer
//     uint8_t packMonitorDataBuffer[REGISTER_SIZE_BYTES * NUM_PACK_MON_IN_ACCUMULATOR];
//     memset(packMonitorDataBuffer, 0, REGISTER_SIZE_BYTES * NUM_PACK_MON_IN_ACCUMULATOR);

//     // Create and clear cell monitor data buffer
//     uint8_t cellMonitorDataBuffer[REGISTER_SIZE_BYTES * NUM_CELL_MON_IN_ACCUMULATOR];
//     memset(cellMonitorDataBuffer, 0, REGISTER_SIZE_BYTES * NUM_CELL_MON_IN_ACCUMULATOR);

//     // ISOSPI status variable
//     TRANSACTION_STATUS_E status;

//     // Unfreeze all device registers
//     status = commandChain(UNSNAP, &taskData->chainInfo, SHARED_COMMAND);
//     if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
//     {
//         return status;
//     }

//     // Freeze all device registers
//     status = commandChain(SNAP, &taskData->chainInfo, SHARED_COMMAND);
//     if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
//     {
//         return status;
//     }

//     updateTimer(&taskData->packMonitorData.localPhaseCountTimer);

//     // Read status register group C
//     status = readChain(RDSTATC, &taskData->chainInfo, packMonitorDataBuffer, cellMonitorDataBuffer);
//     if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
//     {
//         return status;
//     }

//     // Extract the sleep bit from each data packet, and check if it is set
//     if((packMonitorDataBuffer[REGISTER_BYTE5]) & STATC_SLEEP_BIT)
//     {
//         return TRANSACTION_POR_ERROR;
//     }

//     // Check for the sleep bit to detect a power reset
//     for(uint32_t i = 0; i < NUM_CELL_MON_IN_ACCUMULATOR; i++)
//     {
//         // Extract the sleep bit from each data packet, and check if it is set
//         uint8_t statC5 = cellMonitorDataBuffer[(i * REGISTER_SIZE_BYTES) + REGISTER_BYTE5];
//         if(statC5 & STATC_SLEEP_BIT)
//         {
//             return TRANSACTION_POR_ERROR;
//         }
//     }

//     uint16_t phaseCounterMSB = (packMonitorDataBuffer[REGISTER_BYTE2] & 0x1F);
//     uint16_t phaseCounterLSB = packMonitorDataBuffer[REGISTER_BYTE3];
//     uint16_t phaseCounter = (phaseCounterMSB << BITS_IN_BYTE) | phaseCounterLSB;

//     if(phaseCounter < taskData->packMonitorData.lastPhaseCount)
//     {
//         taskData->packMonitorData.adcConversionPhaseCounter += (phaseCounter + (MAX_13BIT_UINT - taskData->packMonitorData.lastPhaseCount) + 1);
//     }
//     else
//     {
//         taskData->packMonitorData.adcConversionPhaseCounter += (phaseCounter - taskData->packMonitorData.lastPhaseCount);
//     }
//     taskData->packMonitorData.lastPhaseCount = phaseCounter;

//     taskData->packMonitorData.adcConversionTimeMS = ((float)(taskData->packMonitorData.localPhaseCountTimer.timCount * PHASE_COUNTS_PER_MS) / (taskData->packMonitorData.adcConversionPhaseCounter));

//     return status;
// }

// static TRANSACTION_STATUS_E updateCellVoltages(telemetryTaskData_S *taskData)
// {
//     // Create and clear pack monitor data buffer
//     uint8_t packMonitorDataBuffer[NUM_CELLV_REGISTERS][REGISTER_SIZE_BYTES * NUM_PACK_MON_IN_ACCUMULATOR];
//     memset(packMonitorDataBuffer, 0, NUM_CELLV_REGISTERS * REGISTER_SIZE_BYTES * NUM_PACK_MON_IN_ACCUMULATOR);

//     // Create and clear cell monitor data buffer
//     uint8_t cellMonitorDataBuffer[REGISTER_SIZE_BYTES * NUM_CELL_MON_IN_ACCUMULATOR];
//     memset(cellMonitorDataBuffer, 0, REGISTER_SIZE_BYTES * NUM_CELL_MON_IN_ACCUMULATOR);

//     TRANSACTION_STATUS_E status;

//     // Read cell voltage registers A-E, and populate cell voltages to data struct
//     for(uint32_t i = 0; i < NUM_CELLV_REGISTERS; i++)
//     {
//         status = readChain(readFiltVoltReg[i], &taskData->chainInfo, packMonitorDataBuffer[i], cellMonitorDataBuffer);
//         if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
//         {
//             return status;
//         }

//         // Ignore Pack monitor data for now, it is retained in the rxbuffer for later conversion
//         convertCellVoltageRegister(cellMonitorDataBuffer, (i * CELLS_PER_REG), taskData->bmb, false);
//     }

//     // Extract IADC1 and IADC2 data from current sense ADCs
//     taskData->IADC1 = (float)CONVERT_IADC1(packMonitorDataBuffer[0]) / taskData->packMonitorData.shuntResistanceMicroOhms;
//     taskData->IADC2 = (float)CONVERT_IADC2((packMonitorDataBuffer[0] + ENERGY_DATA_SIZE)) / taskData->packMonitorData.shuntResistanceMicroOhms;

//     // Extract VBADC1 and VBADC2 data from battery voltage ADCs
//     taskData->VBADC1 = (float)CONVERT_VBADC1((packMonitorDataBuffer[1] + VBATT_DATA_SIZE));
//     taskData->VBADC2 = (float)CONVERT_VBADC2((packMonitorDataBuffer[1] + (2 * VBATT_DATA_SIZE)));

//     return status;
// }

// static TRANSACTION_STATUS_E updateDiagnosticCellVoltages(telemetryTaskData_S *taskData)
// {
//     // Create and clear pack monitor data buffer
//     uint8_t packMonitorDataBuffer[NUM_CELLV_REGISTERS][REGISTER_SIZE_BYTES * NUM_PACK_MON_IN_ACCUMULATOR];
//     memset(packMonitorDataBuffer, 0, NUM_CELLV_REGISTERS * REGISTER_SIZE_BYTES * NUM_PACK_MON_IN_ACCUMULATOR);

//     // Create and clear cell monitor data buffer
//     uint8_t cellMonitorDataBuffer[REGISTER_SIZE_BYTES * NUM_CELL_MON_IN_ACCUMULATOR];
//     memset(cellMonitorDataBuffer, 0, REGISTER_SIZE_BYTES * NUM_CELL_MON_IN_ACCUMULATOR);

//     TRANSACTION_STATUS_E status;

//     // Read cell voltage registers A-E, and populate cell voltages to data struct
//     for(uint32_t i = 0; i < NUM_CELLV_REGISTERS; i++)
//     {
//         status = readChain(readDiagVoltReg[i], &taskData->chainInfo, packMonitorDataBuffer[i], cellMonitorDataBuffer);
//         if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
//         {
//             return status;
//         }

//         // Ignore Pack monitor data for now, it is retained in the rxbuffer for later conversion
//         convertCellVoltageRegister(cellMonitorDataBuffer, (i * CELLS_PER_REG), taskData->bmb, true);
//     }

//     return status;
// }

// static TRANSACTION_STATUS_E updateCellTemps(telemetryTaskData_S *taskData)
// {
//     // Create and clear pack monitor data buffer
//     uint8_t packMonitorDataBuffer[REGISTER_SIZE_BYTES * NUM_PACK_MON_IN_ACCUMULATOR];
//     memset(packMonitorDataBuffer, 0, REGISTER_SIZE_BYTES * NUM_PACK_MON_IN_ACCUMULATOR);

//     // Create and clear cell monitor data buffer
//     uint8_t cellMonitorDataBuffer[REGISTER_SIZE_BYTES * NUM_CELL_MON_IN_ACCUMULATOR];
//     memset(cellMonitorDataBuffer, 0, REGISTER_SIZE_BYTES * NUM_CELL_MON_IN_ACCUMULATOR);

//     TRANSACTION_STATUS_E status;

//     // Read aux voltage registers A-C, and populate cell temps to data struct
//     for(uint32_t i = 0; i < NUM_AUXV_REGISTERS; i++)
//     {
//         status = readChain(readAuxReg[i], &taskData->chainInfo, packMonitorDataBuffer, cellMonitorDataBuffer);
//         if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
//         {
//             return status;
//         }

//         // Each mux state corresponds to odd or even cell temps
//         uint32_t cellStartIndex = (i * 2 * CELLS_PER_REG) + (uint32_t)taskData->muxState;

//         // Ignore Pack monitor data for now, it is retained in the rxbuffer for later conversion
//         convertCellTempRegister(cellMonitorDataBuffer, cellStartIndex, taskData->bmb);
//     }

//     for(int32_t i = 0; i < NUM_CELL_MON_IN_ACCUMULATOR; i++)
//     {
//         float adcVoltage = CONVERT_VADC((cellMonitorDataBuffer + (i * REGISTER_SIZE_BYTES) + (2 * CELL_REG_SIZE)));
//         taskData->bmb[bmbOrder[i]].boardTemp = lookup(adcVoltage, &boardTempTable);
//         taskData->bmb[bmbOrder[i]].boardTempStatus = GOOD;
//     }

//     // Cycle mux state
//     taskData->muxState++;
//     taskData->muxState %= NUM_MUX_STATES;

//     // Fill tx buffer with proper GPIO config bit to swap mux state
//     packMonitorDataBuffer[REGISTER_BYTE3] = ALL_PACK_MON_GPIO;
//     packMonitorDataBuffer[REGISTER_BYTE4] = ALL_PACK_MON_GPIO;
//     packMonitorDataBuffer[REGISTER_BYTE5] = ACCN_CTRL_BITS;

//     for(uint32_t i = 0; i < NUM_CELL_MON_IN_ACCUMULATOR; i++)
//     {
//         cellMonitorDataBuffer[REGISTER_BYTE3] = ALL_CELL_MON_GPIO;
//         cellMonitorDataBuffer[REGISTER_BYTE4] = GPIO9 | ((uint8_t)taskData->muxState << 1);
//         cellMonitorDataBuffer[REGISTER_BYTE5] = CADC_FILTER_CUTOFF_21HZ;
//     }

//     status = writeChain(WRCFGA, &taskData->chainInfo, SHARED_COMMAND, packMonitorDataBuffer, cellMonitorDataBuffer);
//     if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
//     {
//         return status;
//     }

//     // Verify command counter
//     status = readChain(RDSID, &taskData->chainInfo, packMonitorDataBuffer, cellMonitorDataBuffer);
//     if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
//     {
//         return status;
//     }

//     return status;
// }

// static TRANSACTION_STATUS_E updateEnergyData(telemetryTaskData_S *taskData)
// {
//     // Create and clear pack monitor data buffer
//     uint8_t packMonitorDataBuffer[REGISTER_SIZE_BYTES * NUM_PACK_MON_IN_ACCUMULATOR];
//     memset(packMonitorDataBuffer, 0, REGISTER_SIZE_BYTES * NUM_PACK_MON_IN_ACCUMULATOR);

//     // ISOSPI status variable
//     TRANSACTION_STATUS_E status;

//     if(taskData->packMonitorData.adcConversionPhaseCounter >= taskData->packMonitorData.nextGoodAccumulationDataPhaseCount)
//     {
//         taskData->packMonitorData.nextGoodAccumulationDataPhaseCount += (ACCUMULATION_REGISTER_COUNT * PHASE_COUNTS_PER_MS);

//         status = readPackMonitor(RDACC, &taskData->chainInfo, packMonitorDataBuffer);
//         if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
//         {
//             return status;
//         }

//         float accumulatedCurrent = (float)CONVERT_IADC1(packMonitorDataBuffer) / taskData->packMonitorData.shuntResistanceMicroOhms;
//         float accumulatedVoltage = (float)CONVERT_VBADC1((packMonitorDataBuffer + ENERGY_DATA_SIZE));

//         taskData->socData.milliCoulombCounter += (int32_t)(accumulatedCurrent * taskData->packMonitorData.adcConversionTimeMS);
//         // taskData->socData.milliJouleCounter += (uint32_t)(accumulatedCurrent * accumulatedVoltage * taskData->packMonitorData.adcConversionTimeMS);

//         if(fabs(accumulatedCurrent) > 1.0f)
//         {
//             clearTimer(&taskData->socData.socByOcvQualificationTimer);
//         }
//         else
//         {
//             updateTimer(&taskData->socData.socByOcvQualificationTimer);
//         }
//     }


//     return status;
// }

// static TRANSACTION_STATUS_E runAdcDiagnostics(telemetryTaskData_S *taskData)
// {
//     TRANSACTION_STATUS_E status;

//     status = updateDiagnosticCellVoltages(taskData);
//     if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
//     {
//         return status;
//     }

//     taskData->curentDiagnosticState = taskData->nextDiagnosticState;

//     // Use the current diagnostic cycle count to determine the state of the diagnostic data and start the adc for next cycle
//     taskData->diagnosticCycleCounter++;
//     switch (taskData->curentDiagnosticState)
//     {
//     case REDUNDANT_ADC_DIAG_STATE:
//         if(taskData->balancingEnabled)
//         {
//             if(taskData->diagnosticCycleCounter >= REDUNDANT_CHECK_TIME_CYC)
//             {
//                 status = commandChain(ADSV | ADC_DCP, &taskData->chainInfo, SHARED_COMMAND);
//                 if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
//                 {
//                     return status;
//                 }
//                 taskData->diagnosticCycleCounter = 0;
//                 taskData->nextDiagnosticState = BALANCING_DIAG_STATE;
//             }
//         }
//         else
//         {
//             if(taskData->diagnosticCycleCounter >= (REDUNDANT_CHECK_TIME_CYC + BALANCING_TIME_CYC))
//             {
//                 status = commandChain(ADSV | ADC_CONT | ADC_OW_EVEN, &taskData->chainInfo, SHARED_COMMAND);
//                 if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
//                 {
//                     return status;
//                 }
//                 taskData->diagnosticCycleCounter = 0;
//                 taskData->nextDiagnosticState = OPEN_WIRE_EVEN_DIAG_STATE;
//             }
//         }
//         break;

//     case BALANCING_DIAG_STATE:
//         if(taskData->diagnosticCycleCounter >= BALANCING_TIME_CYC)
//         {
//             status = commandChain(ADSV | ADC_CONT | ADC_OW_EVEN, &taskData->chainInfo, SHARED_COMMAND);
//             if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
//             {
//                 return status;
//             }
//             taskData->diagnosticCycleCounter = 0;
//             taskData->nextDiagnosticState = OPEN_WIRE_EVEN_DIAG_STATE;
//         }
//         break;

//     case OPEN_WIRE_EVEN_DIAG_STATE:
//         if(taskData->diagnosticCycleCounter >= OPEN_WIRE_SOAK_TIME_CYC)
//         {
//             status = commandChain(ADSV | ADC_CONT | ADC_OW_ODD, &taskData->chainInfo, SHARED_COMMAND);
//             if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
//             {
//                 return status;
//             }
//             taskData->diagnosticCycleCounter = 0;
//             taskData->nextDiagnosticState = OPEN_WIRE_ODD_DIAG_STATE;
//         }
//         break;

//     case OPEN_WIRE_ODD_DIAG_STATE:
//         if(taskData->diagnosticCycleCounter >= OPEN_WIRE_SOAK_TIME_CYC)
//         {
//             status = commandChain(ADSV | ADC_CONT, &taskData->chainInfo, SHARED_COMMAND);
//             if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
//             {
//                 return status;
//             }
//             taskData->diagnosticCycleCounter = 0;
//             taskData->nextDiagnosticState = REDUNDANT_ADC_DIAG_STATE;
//         }
//         break;

//     default:
//         status = commandChain(ADSV | ADC_CONT, &taskData->chainInfo, SHARED_COMMAND);
//         if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
//         {
//             return status;
//         }
//         taskData->diagnosticCycleCounter = 0;
//         taskData->nextDiagnosticState = REDUNDANT_ADC_DIAG_STATE;
//         break;
//     }

//     return status;
// }

// static void updateBmbStatistics(Cell_Monitor_S *bmb)
// {
// 	for(uint32_t i = 0; i < NUM_CELL_MON_IN_ACCUMULATOR; i++)
// 	{
// 		Cell_Monitor_S* pBmb = &bmb[i];
// 		float maxCellVoltage = MIN_CELLV_SENSOR_VALUE;
// 		float minCellVoltage = MAX_CELLV_SENSOR_VALUE;
// 		float sumVoltage = 0.0f;
// 		uint32_t numGoodCellVoltage = 0;

// 		float maxCellTemp = MIN_TEMP_SENSOR_VALUE_C;
// 		float minCellTemp = MAX_TEMP_SENSOR_VALUE_C;
// 		float sumCellTemp = 0.0f;
// 		uint32_t numGoodCellTemp = 0;

// 		// Aggregate Cell voltage and temperature data
// 		for(uint32_t j = 0; j < NUM_CELLS_PER_CELL_MONITOR; j++)
// 		{
// 			// Only update stats if sense status is good
// 			if(pBmb->cellVoltageStatus[j] == GOOD)
// 			{
// 				float cellV = pBmb->cellVoltage[j];

// 				if(cellV > maxCellVoltage)
// 				{
// 					maxCellVoltage = cellV;
// 				}
// 				if(cellV < minCellVoltage)
// 				{
// 					minCellVoltage = cellV;
// 				}
// 				numGoodCellVoltage++;
// 				sumVoltage += cellV;
// 			}

// 			// Only update stats if sense status is good
// 			if(pBmb->cellTempStatus[j] == GOOD)
// 			{
// 				float cellTemp = pBmb->cellTemp[j];

// 				if (cellTemp > maxCellTemp)
// 				{
// 					maxCellTemp = cellTemp;
// 				}
// 				if (cellTemp < minCellTemp)
// 				{
// 					minCellTemp = cellTemp;
// 				}
// 				numGoodCellTemp++;
// 				sumCellTemp += cellTemp;
// 			}
// 		}

// 		// Update BMB statistics
//         // TODO: determine what to do with BAD sensor status variables
//         if(numGoodCellVoltage > 0)
//         {
//             pBmb->maxCellVoltage = maxCellVoltage;
//             pBmb->minCellVoltage = minCellVoltage;
//             pBmb->sumCellVoltage = sumVoltage;
//             pBmb->avgCellVoltage = (sumVoltage / numGoodCellVoltage);
//             pBmb->numBadCellVoltage = NUM_CELLS_PER_CELL_MONITOR - numGoodCellVoltage;
//         }

//         if(numGoodCellVoltage > 0)
//         {
//             pBmb->maxCellTemp = maxCellTemp;
//             pBmb->minCellTemp = minCellTemp;
//             pBmb->avgCellTemp = (sumCellTemp / numGoodCellTemp);
//             pBmb->numBadCellTemp = NUM_CELLS_PER_CELL_MONITOR - numGoodCellTemp;
//         }
// 	}
// }

// static void updatePackStatistics(telemetryTaskData_S *taskData)
// {
// 	// Update BMB level stats
// 	updateBmbStatistics(taskData->bmb);

// 	float maxCellVoltage = MIN_CELLV_SENSOR_VALUE;
//     float minCellVoltage = MAX_CELLV_SENSOR_VALUE;
//     float sumVoltage = 0.0f;
//     float sumAvgCellVoltage = 0.0f;
//     uint32_t numGoodBmbsCellV = 0;

//     float maxCellTemp = MIN_TEMP_SENSOR_VALUE_C;
//     float minCellTemp = MAX_TEMP_SENSOR_VALUE_C;
//     float sumAvgCellTemp = 0.0f;
//     uint32_t numGoodBmbsCellTemp = 0;

// 	float maxBoardTemp = MIN_TEMP_SENSOR_VALUE_C;
// 	float minBoardTemp = MAX_TEMP_SENSOR_VALUE_C;
// 	float sumBoardTemp = 0.0f;
//     uint32_t numGoodBoardTemp = 0;

// 	for(int32_t i = 0; i < NUM_CELL_MON_IN_ACCUMULATOR; i++)
// 	{
// 		Cell_Monitor_S* pBmb = &taskData->bmb[i];

//         if(pBmb->numBadCellVoltage != NUM_CELLS_PER_CELL_MONITOR)
//         {
//             if(pBmb->maxCellVoltage > maxCellVoltage)
//             {
//                 maxCellVoltage = pBmb->maxCellVoltage;
//             }
//             if(pBmb->minCellVoltage < minCellVoltage)
//             {
//                 minCellVoltage = pBmb->minCellVoltage;
//             }

//             numGoodBmbsCellV++;
//             sumAvgCellVoltage += pBmb->avgCellVoltage;
//             sumVoltage += pBmb->sumCellVoltage;
//         }

//         if(pBmb->numBadCellTemp != NUM_CELLS_PER_CELL_MONITOR)
//         {
//             if (pBmb->maxCellTemp > maxCellTemp)
//             {
//                 maxCellTemp = pBmb->maxCellTemp;
//             }
//             if (pBmb->minCellTemp < minCellTemp)
//             {
//                 minCellTemp = pBmb->minCellTemp;
//             }

//             numGoodBmbsCellTemp++;
//             sumAvgCellTemp += pBmb->avgCellTemp;
//         }

//         if(pBmb->boardTempStatus == GOOD)
//         {
//             if (pBmb->boardTemp > maxBoardTemp)
//             {
//                 maxBoardTemp = pBmb->boardTemp;
//             }
//             if (pBmb->boardTemp < minBoardTemp)
//             {
//                 minBoardTemp = pBmb->boardTemp;
//             }

//             numGoodBoardTemp++;
//             sumBoardTemp += pBmb->boardTemp;
//         }
// 	}

//     // TODO: Should i ignore this if bad sensors or open wires?
//     taskData->packVoltage = sumVoltage;

//     if(numGoodBmbsCellV > 0)
//     {
//         taskData->maxCellVoltage = maxCellVoltage;
//         taskData->minCellVoltage = minCellVoltage;
//         taskData->avgCellVoltage = sumAvgCellVoltage / numGoodBmbsCellV;
//     }

//     if(numGoodBmbsCellTemp > 0)
//     {
//         taskData->maxCellTemp = maxCellTemp;
//         taskData->minCellTemp = minCellTemp;
//         taskData->avgCellTemp = sumAvgCellTemp / numGoodBmbsCellTemp;
//     }

//     if(numGoodBoardTemp > 0)
//     {
//         taskData->maxBoardTemp = maxBoardTemp;
//         taskData->minBoardTemp = minBoardTemp;
//         taskData->avgBoardTemp = sumBoardTemp / numGoodBoardTemp;
//     }
// }

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DEFINITIONS ==================== */
/* ==================================================================== */

void initTelemetryTask()
{
    Soc_S defaultSocInfo = {
        .socByOcvQualificationTimer = (Timer_S){.timCount = CELL_POLARIZATION_REST_MS, .lastUpdate = 0, .timThreshold = CELL_POLARIZATION_REST_MS},
        .packMilliCoulombs = PACK_MILLICOULOMBS,
        .milliCoulombCounter = 0,
        .socByOcv = 0.0f,
        .soeByOcv = 0.0f,
        .socByCoulombCounting = 0.0f,
        .soeByCoulombCounting = 0.0f
    };

    vTaskSuspendAll();
    telemetryTaskData.packMonitor.socData = defaultSocInfo;
    xTaskResumeAll();

    HAL_GPIO_WritePin(MST1_GPIO_Port, MST1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(MST2_GPIO_Port, MST2_Pin, GPIO_PIN_SET);
}

void runTelemetryTask()
{
    // Create local data struct for bmb information
    telemetryTaskData_S telemetryTaskDataLocal;

    // Copy in last cycles data into local data struct
    vTaskSuspendAll();
    telemetryTaskDataLocal = telemetryTaskData;
    telemetryTaskDataLocal.balancingEnabled = ((chargerTaskData.chargerState == CHARGER_STATE_BALANCING) || FORCE_BALANCING_ON || forceEnableBalancing_state.data);
    xTaskResumeAll();

    TRANSACTION_STATUS_E telemetryStatus = updateBatteryTelemetry(&telemetryTaskDataLocal);

    if(telemetryStatus == TRANSACTION_CHAIN_BREAK_ERROR)
    {
        Debug("Chain Break!\n");
    }
    else if(telemetryStatus == TRANSACTION_SPI_ERROR)
    {
        Debug("SPI Failure!\n");
    }
    else if(telemetryStatus == TRANSACTION_POR_ERROR)
    {
        Debug("Failed to correct power on reset error!\n");
    }
    else if(telemetryStatus == TRANSACTION_COMMAND_COUNTER_ERROR)
    {
        Debug("Persistent Command Counter Error!\n");
    }

    // Regardless of whether or not chain initialized, run alert monitor stuff

    // Alert Monitor
    runTelemetryAlertMonitor(&telemetryTaskDataLocal);

    // TODO Handle case of continous POR / CC errors here

    // Copy out new data into global data struct
    vTaskSuspendAll();
    telemetryTaskData = telemetryTaskDataLocal;
    xTaskResumeAll();
}
