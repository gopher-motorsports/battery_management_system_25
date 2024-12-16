/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include "telemetryTask.h"
#include "main.h"
#include "cmsis_os.h"
#include "utils.h"
#include "debug.h"
#include "lookupTable.h"
#include "packData.h"
#include "cellData.h"
#include <string.h>
#include <stdbool.h>

#define NUM_READS  1

/* ==================================================================== */
/* ============================= DEFINES ============================== */
/* ==================================================================== */

#define NUM_COMMAND_BLOCK_RETRYS            3

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
/* ============================== STRUCTS ============================= */
/* ==================================================================== */

/* ==================================================================== */
/* ========================= LOCAL VARIABLES ========================== */
/* ==================================================================== */

bool chainInitialized;
MUX_STATE_E muxState;

uint8_t bmbOrder[NUM_BMBS_IN_ACCUMULATOR] =
{
    BMB0_SEGMENT_INDEX
    // BMB1_SEGMENT_INDEX
    // BMB2_SEGMENT_INDEX,
    // BMB3_SEGMENT_INDEX,
    // BMB4_SEGMENT_INDEX,
    // BMB5_SEGMENT_INDEX,
    // BMB6_SEGMENT_INDEX,
    // BMB7_SEGMENT_INDEX
};

uint16_t readVoltReg[NUM_CELLV_REGISTERS] =
{
    RDCVA_RDI, RDCVB_RDVB,
    RDCVC, RDCVD,
    RDCVE, RDCVF,
};

uint16_t readAuxReg[NUM_AUXV_REGISTERS] =
{
    RDAUXA,
    RDAUXB,
    RDAUXC
};

/* ==================================================================== */
/* =================== LOCAL FUNCTION DECLARATIONS ==================== */
/* ==================================================================== */

static void convertCellVoltageRegister(uint8_t *bmbData, uint32_t cellStartIndex, Bmb_S *bmbArray);
static void convertCellTempRegister(uint8_t *bmbData, uint32_t cellStartIndex, Bmb_S *bmbArray);

TRANSACTION_STATUS_E runCommandBlock(TRANSACTION_STATUS_E (*telemetryFunction)(telemetryTaskData_S*), telemetryTaskData_S *taskData);

static TRANSACTION_STATUS_E initChain(telemetryTaskData_S *taskData);
static TRANSACTION_STATUS_E runSystemDiagnostics(telemetryTaskData_S *taskData);
static TRANSACTION_STATUS_E startNewReadCycle(telemetryTaskData_S *taskData);
static TRANSACTION_STATUS_E updateCellVoltages(telemetryTaskData_S *taskData);
static TRANSACTION_STATUS_E updateCellTemps(telemetryTaskData_S *taskData);
// static TRANSACTION_STATUS_E runSPinDiagnostics(telemetryTaskData_S *taskData);

static void updateBmbStatistics(Bmb_S *bmb);
static void updatePackStatistics(telemetryTaskData_S *taskData);

/* ==================================================================== */
/* ============================== MACROS ============================== */
/* ==================================================================== */

#define HANDLE_INIT_ERROR(error) \
    if(error != TRANSACTION_SUCCESS) { \
        if(error == TRANSACTION_CHAIN_BREAK_ERROR) \
        { \
            Debug("Chain break on init! Not all devices initialized\n"); \
        } \
        else if(error == TRANSACTION_SPI_ERROR) \
        { \
            Debug("SPI Failure, reseting STM32...\n"); \
            HAL_NVIC_SystemReset(); \
        } \
        else if(error == TRANSACTION_POR_ERROR) \
        { \
            Debug("Power reset detected, reinitializing...\n"); \
            continue; \
        } \
        else if(error == TRANSACTION_COMMAND_COUNTER_ERROR) \
        { \
            Debug("Command counter mismatch! Retrying command block!\n"); \
            continue; \
        } \
    }

#define EXTRACT_16_BIT(buffer)      (((uint32_t)buffer[1] << (1 * BITS_IN_BYTE)) | ((uint32_t)buffer[0]))

#define EXTRACT_24_BIT(buffer)      (((uint32_t)buffer[2] << (2 * BITS_IN_BYTE)) | ((uint32_t)buffer[1] << (1 * BITS_IN_BYTE)) | ((uint32_t)buffer[0]))

#define CONVERT_VADC(reg)           (((int16_t)(EXTRACT_16_BIT(reg)) * VADC_GAIN) + VADC_OFFSET)

#define CONVERT_VBADC1(reg)         ((int16_t)(EXTRACT_16_BIT(reg)) * VBADC1_GAIN)

#define CONVERT_VBADC2(reg)         ((int16_t)(EXTRACT_16_BIT(reg)) * VBADC2_GAIN * -1.0f)

#define CONVERT_IADC1(reg)          ((((int32_t)(EXTRACT_24_BIT(reg) << BITS_IN_BYTE)) /  BYTE_SIZE_DEC) * IADC_GAIN)

#define CONVERT_IADC2(reg)          ((((int32_t)(EXTRACT_24_BIT(reg) << BITS_IN_BYTE)) /  BYTE_SIZE_DEC) * IADC_GAIN * 1.0f)

/* ==================================================================== */
/* =================== LOCAL FUNCTION DEFINITIONS ===================== */
/* ==================================================================== */

static void convertCellVoltageRegister(uint8_t *bmbData, uint32_t cellStartIndex, Bmb_S *bmbArray)
{
    int32_t cellsInReg = CELLS_PER_REG;
    if((NUM_CELLS_PER_BMB - cellStartIndex) < CELLS_PER_REG)
    {
        cellsInReg = NUM_CELLS_PER_BMB - cellStartIndex;
    }

    for(int32_t i = 0; i < cellsInReg; i++)
    {
        for(int32_t j = 0; j < NUM_BMBS_IN_ACCUMULATOR; j++)
        {
            float cellVoltage = CONVERT_VADC((bmbData + (j * REGISTER_SIZE_BYTES) + (i * CELL_REG_SIZE)));
            bmbArray[bmbOrder[j]].cellVoltage[(cellStartIndex + i)] = cellVoltage;
            bmbArray[bmbOrder[j]].cellVoltageStatus[(cellStartIndex + i)] = GOOD;
        }
    }
}

static void convertCellTempRegister(uint8_t *bmbData, uint32_t cellStartIndex, Bmb_S *bmbArray)
{
    int32_t cellsInReg = CELLS_PER_REG;
    if((NUM_CELLS_PER_BMB - cellStartIndex + 1) < (CELLS_PER_REG * 2))
    {
        cellsInReg = (int32_t)((NUM_CELLS_PER_BMB - cellStartIndex + 1) / 2);
    }

    for(int32_t i = 0; i < cellsInReg; i++)
    {
        for(int32_t j = 0; j < NUM_BMBS_IN_ACCUMULATOR; j++)
        {
            float adcVoltage = CONVERT_VADC((bmbData + (j * REGISTER_SIZE_BYTES) + (i * CELL_REG_SIZE)));
            bmbArray[bmbOrder[j]].cellTemp[(cellStartIndex + (i * 2))] = lookup(adcVoltage, &cellTempTable);
            bmbArray[bmbOrder[j]].cellTempStatus[(cellStartIndex + (i * 2))] = GOOD;
        }
    }
}

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

static TRANSACTION_STATUS_E initChain(telemetryTaskData_S *taskData)
{
    // Create and clear rx buffer
    uint8_t rxBuffer[REGISTER_SIZE_BYTES * NUM_DEVICES_IN_ACCUMULATOR];
    memset(rxBuffer, 0, REGISTER_SIZE_BYTES * NUM_DEVICES_IN_ACCUMULATOR);

    // Create and clear tx buffer
    uint8_t txBuffer[REGISTER_SIZE_BYTES * NUM_DEVICES_IN_ACCUMULATOR];
    memset(txBuffer, 0, REGISTER_SIZE_BYTES * NUM_DEVICES_IN_ACCUMULATOR);

    TRANSACTION_STATUS_E status;

    // Start ADCs
    status = commandChain((ADCV | ADC_CONT | ADC_RD), &taskData->chainInfo, SHARED_COMMAND);
    if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    {
        return status;
    }

    // Start Aux ADC on BMBs
    status = commandChain(ADAX, &taskData->chainInfo, CELL_MONITOR_COMMAND);
    if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    {
        return status;
    }

    // Clear SLEEP Bit
    for(int32_t i = 0; i < NUM_DEVICES_IN_ACCUMULATOR; i++)
    {
        txBuffer[(i * REGISTER_SIZE_BYTES) + 5] = STATC_SLEEP_BIT;
    }

    status = writeChain(CLRFLAG, &taskData->chainInfo, SHARED_COMMAND, txBuffer);
    if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    {
        return status;
    }

    // Fill tx buffer with proper GPIO config bits to disable GPIO pulldowns
    memset(txBuffer, 0, REGISTER_SIZE_BYTES * NUM_DEVICES_IN_ACCUMULATOR);
    txBuffer[3] = 0x3f;
    txBuffer[4] = 0x3f;
    txBuffer[5] = 0x01;

    for(int32_t i = NUM_PACK_MON_IN_ACCUMULATOR; i < NUM_DEVICES_IN_ACCUMULATOR; i++)
    {
        txBuffer[(i * REGISTER_SIZE_BYTES) + 3] = 0xFF;
        txBuffer[(i * REGISTER_SIZE_BYTES) + 4] = (0x01 | ((uint8_t)muxState << 1));
    }

    status = writeChain(WRCFGA, &taskData->chainInfo, SHARED_COMMAND, txBuffer);
    if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    {
        return status;
    }

    // Verify command counter
    status = readChain(RDSID, &taskData->chainInfo, rxBuffer);
    if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    {
        return status;
    }

    return status;
}

static TRANSACTION_STATUS_E runSystemDiagnostics(telemetryTaskData_S *taskData)
{
    // Create and clear rx buffer
    uint8_t rxBuffer[REGISTER_SIZE_BYTES * NUM_DEVICES_IN_ACCUMULATOR];
    memset(rxBuffer, 0, REGISTER_SIZE_BYTES * NUM_DEVICES_IN_ACCUMULATOR);

    // ISOSPI status variable
    TRANSACTION_STATUS_E status;

    // Read status register group C
    status = readChain(RDSTATC, &taskData->chainInfo, rxBuffer);
    if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    {
        return status;
    }

    // This code may be unessecary given how the commandChain function reinits the chain every cycle
    // Check for the sleep bit to detect a power reset
    for(uint32_t i = NUM_PACK_MON_IN_ACCUMULATOR; i < NUM_DEVICES_IN_ACCUMULATOR; i++)
    {
        // Extract the sleep bit from each data packet, and check if it is set
        uint8_t statC5 = rxBuffer[(i * REGISTER_SIZE_BYTES) + 5];
        if(statC5 & STATC_SLEEP_BIT)
        {
            return TRANSACTION_POR_ERROR;
        }
    }

    return status;
}

static TRANSACTION_STATUS_E startNewReadCycle(telemetryTaskData_S *taskData)
{
    // Create and clear rx and tx buffers
    uint8_t rxBuffer[REGISTER_SIZE_BYTES * NUM_DEVICES_IN_ACCUMULATOR];
    memset(rxBuffer, 0, REGISTER_SIZE_BYTES * NUM_DEVICES_IN_ACCUMULATOR);

    // Create and clear tx buffer
    uint8_t txBuffer[REGISTER_SIZE_BYTES * NUM_DEVICES_IN_ACCUMULATOR];
    memset(txBuffer, 0, REGISTER_SIZE_BYTES * NUM_DEVICES_IN_ACCUMULATOR);

    // ISOSPI status variable
    TRANSACTION_STATUS_E status;

    // Unfreeze all device registers
    status = commandChain(UNSNAP, &taskData->chainInfo, SHARED_COMMAND);
    if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    {
        return status;
    }

    // Freeze all device registers
    status = commandChain(SNAP, &taskData->chainInfo, SHARED_COMMAND);
    if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    {
        return status;
    }

    // Swap BMB MUX state

    // Fill tx buffer with proper GPIO config bit to swap mux state
    txBuffer[3] = 0x3f;
    txBuffer[4] = 0x3f;
    txBuffer[5] = 0x01;

    for(int32_t i = NUM_PACK_MON_IN_ACCUMULATOR; i < NUM_DEVICES_IN_ACCUMULATOR; i++)
    {
        txBuffer[(i * REGISTER_SIZE_BYTES) + 3] = 0xFF;
        txBuffer[(i * REGISTER_SIZE_BYTES) + 4] = (0x01 | ((uint8_t)muxState << 1));
    }

    status = writeChain(WRCFGA, &taskData->chainInfo, SHARED_COMMAND, txBuffer);
    if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    {
        return status;
    }

    // Verify command counter
    status = readChain(RDSID, &taskData->chainInfo, rxBuffer);
    if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    {
        return status;
    }

    return status;
}

static TRANSACTION_STATUS_E updateCellVoltages(telemetryTaskData_S *taskData)
{
    // Create and clear rx buffer
    uint8_t rxBuffer[NUM_CELLV_REGISTERS][REGISTER_SIZE_BYTES * NUM_DEVICES_IN_ACCUMULATOR];
    memset(rxBuffer, 0, NUM_CELLV_REGISTERS * REGISTER_SIZE_BYTES * NUM_DEVICES_IN_ACCUMULATOR);

    TRANSACTION_STATUS_E status;

    // Read cell voltage registers A-E, and populate cell voltages to data struct
    for(uint32_t i = 0; i < NUM_CELLV_REGISTERS; i++)
    {
        status = readChain(readVoltReg[i], &taskData->chainInfo, rxBuffer[i]);
        if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
        {
            return status;
        }

        // Ignore Pack monitor data for now, it is retained in the rxbuffer for later conversion
        convertCellVoltageRegister((rxBuffer[i] + (REGISTER_SIZE_BYTES * NUM_PACK_MON_IN_ACCUMULATOR)), (i * CELLS_PER_REG), taskData->bmb);
    }

    // Extract IADC1 and IADC2 data from current sense ADCs
    taskData->IADC1 = CONVERT_IADC1(rxBuffer[0]);
    taskData->IADC2 = CONVERT_IADC2((rxBuffer[0] + ENERGY_DATA_SIZE));

    // Extract VBADC1 and VBADC2 data from battery voltage ADCs
    taskData->VBADC1 = CONVERT_VBADC1((rxBuffer[1] + VBATT_DATA_SIZE));
    taskData->VBADC2 = CONVERT_VBADC2((rxBuffer[1] + (2 * VBATT_DATA_SIZE)));

    return status;
}

static TRANSACTION_STATUS_E updateCellTemps(telemetryTaskData_S *taskData)
{
    // Create and clear rx buffer
    uint8_t rxBuffer[NUM_AUXV_REGISTERS][REGISTER_SIZE_BYTES * NUM_DEVICES_IN_ACCUMULATOR];
    memset(rxBuffer, 0, NUM_AUXV_REGISTERS * REGISTER_SIZE_BYTES * NUM_DEVICES_IN_ACCUMULATOR);

    TRANSACTION_STATUS_E status;

    // Read aux voltage registers A-C, and populate cell temps to data struct
    for(uint32_t i = 0; i < NUM_AUXV_REGISTERS; i++)
    {
        status = readChain(readAuxReg[i], &taskData->chainInfo, rxBuffer[i]);
        if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
        {
            return status;
        }

        // Each mux state corresponds to odd or even cell temps
        uint32_t cellStartIndex = (i * 2 * CELLS_PER_REG) + (uint32_t)muxState;

        // Ignore Pack monitor data for now, it is retained in the rxbuffer for later conversion
        convertCellTempRegister((rxBuffer[i] + (REGISTER_SIZE_BYTES * NUM_PACK_MON_IN_ACCUMULATOR)), cellStartIndex, taskData->bmb);
    }

    // Cycle mux state
    muxState++;
    muxState %= NUM_MUX_STATES;

    for(int32_t i = 0; i < NUM_BMBS_IN_ACCUMULATOR; i++)
    {
        float adcVoltage = CONVERT_VADC((rxBuffer[NUM_AUXV_REGISTERS-1] + (REGISTER_SIZE_BYTES * NUM_PACK_MON_IN_ACCUMULATOR) + (i * REGISTER_SIZE_BYTES) + (2 * CELL_REG_SIZE)));
        taskData->bmb[bmbOrder[i]].boardTemp = lookup(adcVoltage, &boardTempTable);
        taskData->bmb[bmbOrder[i]].boardTempStatus = GOOD;
    }

    return status;
}

// static TRANSACTION_STATUS_E runSPinDiagnostics(telemetryTaskData_S *taskData);

static void updateBmbStatistics(Bmb_S *bmb)
{
	for(uint32_t i = 0; i < NUM_BMBS_IN_ACCUMULATOR; i++)
	{
		Bmb_S* pBmb = &bmb[i];
		float maxCellVoltage = MIN_CELLV_SENSOR_VALUE;
		float minCellVoltage = MAX_CELLV_SENSOR_VALUE;
		float sumVoltage = 0.0f;
		uint32_t numGoodCellVoltage = 0;

		float maxCellTemp = MIN_TEMP_SENSOR_VALUE_C;
		float minCellTemp = MAX_TEMP_SENSOR_VALUE_C;
		float sumCellTemp = 0.0f;
		uint32_t numGoodCellTemp = 0;

		// Aggregate Cell voltage and temperature data
		for(uint32_t j = 0; j < NUM_CELLS_PER_BMB; j++)
		{
			// Only update stats if sense status is good
			if(pBmb->cellVoltageStatus[j] == GOOD)
			{
				float cellV = pBmb->cellVoltage[j];

				if(cellV > maxCellVoltage)
				{
					maxCellVoltage = cellV;
				}
				if(cellV < minCellVoltage)
				{
					minCellVoltage = cellV;
				}
				numGoodCellVoltage++;
				sumVoltage += cellV;
			}

			// Only update stats if sense status is good
			if(pBmb->cellTempStatus[j] == GOOD)
			{
				float cellTemp = pBmb->cellTemp[j];

				if (cellTemp > maxCellTemp)
				{
					maxCellTemp = cellTemp;
				}
				if (cellTemp < minCellTemp)
				{
					minCellTemp = cellTemp;
				}
				numGoodCellTemp++;
				sumCellTemp += cellTemp;
			}
		}

		// Update BMB statistics
        // TODO: determine what to do with BAD sensor status variables
        if(numGoodCellVoltage > 0)
        {
            pBmb->maxCellVoltage = maxCellVoltage;
            pBmb->minCellVoltage = minCellVoltage;
            pBmb->sumCellVoltage = sumVoltage;
            pBmb->avgCellVoltage = (sumVoltage / numGoodCellVoltage);
            pBmb->numBadCellVoltage = NUM_CELLS_PER_BMB - numGoodCellVoltage;
        }

        if(numGoodCellVoltage > 0)
        {
            pBmb->maxCellTemp = maxCellTemp;
            pBmb->minCellTemp = minCellTemp;
            pBmb->avgCellTemp = (sumCellTemp / numGoodCellTemp);
            pBmb->numBadCellTemp = NUM_CELLS_PER_BMB - numGoodCellTemp;
        }
	}
}

static void updatePackStatistics(telemetryTaskData_S *taskData)
{
	// Update BMB level stats
	updateBmbStatistics(taskData->bmb);

	float maxCellVoltage = MIN_CELLV_SENSOR_VALUE;
    float minCellVoltage = MAX_CELLV_SENSOR_VALUE;
    float sumVoltage = 0.0f;
    float sumAvgCellVoltage = 0.0f;
    uint32_t numGoodBmbsCellV = 0;

    float maxCellTemp = MIN_TEMP_SENSOR_VALUE_C;
    float minCellTemp = MAX_TEMP_SENSOR_VALUE_C;
    float sumAvgCellTemp = 0.0f;
    uint32_t numGoodBmbsCellTemp = 0;

	float maxBoardTemp = MIN_TEMP_SENSOR_VALUE_C;
	float minBoardTemp = MAX_TEMP_SENSOR_VALUE_C;
	float sumBoardTemp = 0.0f;
    uint32_t numGoodBoardTemp = 0;

	for(int32_t i = 0; i < NUM_BMBS_IN_ACCUMULATOR; i++)
	{
		Bmb_S* pBmb = &taskData->bmb[i];

        if(pBmb->numBadCellVoltage != NUM_CELLS_PER_BMB)
        {
            if(pBmb->maxCellVoltage > maxCellVoltage)
            {
                maxCellVoltage = pBmb->maxCellVoltage;
            }
            if(pBmb->minCellVoltage < minCellVoltage)
            {
                minCellVoltage = pBmb->minCellVoltage;
            }

            numGoodBmbsCellV++;
            sumAvgCellVoltage += pBmb->avgCellVoltage;
            sumVoltage += pBmb->sumCellVoltage;
        }

        if(pBmb->numBadCellTemp != NUM_CELLS_PER_BMB)
        {
            if (pBmb->maxCellTemp > maxCellTemp)
            {
                maxCellTemp = pBmb->maxCellTemp;
            }
            if (pBmb->minCellTemp < minCellTemp)
            {
                minCellTemp = pBmb->minCellTemp;
            }

            numGoodBmbsCellTemp++;
            sumAvgCellTemp += pBmb->avgCellTemp;
        }

        if(pBmb->boardTempStatus == GOOD)
        {
            if (pBmb->boardTemp > maxBoardTemp)
            {
                maxBoardTemp = pBmb->boardTemp;
            }
            if (pBmb->boardTemp < minBoardTemp)
            {
                minBoardTemp = pBmb->boardTemp;
            }

            numGoodBoardTemp++;
            sumBoardTemp += pBmb->boardTemp;
        }
	}

    // TODO: Should i ignore this if bad sensors or open wires?
    taskData->packVoltage = sumVoltage;

    if(numGoodBmbsCellV > 0)
    {
        taskData->maxCellVoltage = maxCellVoltage;
        taskData->minCellVoltage = minCellVoltage;
        taskData->avgCellVoltage = sumAvgCellVoltage / numGoodBmbsCellV;
    }

    if(numGoodBmbsCellTemp > 0)
    {
        taskData->maxCellTemp = maxCellTemp;
        taskData->minCellTemp = minCellTemp;
        taskData->avgCellTemp = sumAvgCellTemp / numGoodBmbsCellTemp;
    }

    if(numGoodBoardTemp > 0)
    {
        taskData->maxBoardTemp = maxBoardTemp;
        taskData->minBoardTemp = minBoardTemp;
        taskData->avgBoardTemp = sumBoardTemp / numGoodBoardTemp;
    }
}

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DEFINITIONS ==================== */
/* ==================================================================== */

void initTelemetryTask()
{
    HAL_GPIO_WritePin(MAS1_GPIO_Port, MAS1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(MAS2_GPIO_Port, MAS2_Pin, GPIO_PIN_SET);

    HAL_GPIO_WritePin(PORTA_CS_GPIO_Port, PORTA_CS_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PORTB_CS_GPIO_Port, PORTB_CS_Pin, GPIO_PIN_SET);

    CHAIN_INFO_S defaultChainInfo = {
        .numDevs = NUM_DEVICES_IN_ACCUMULATOR,
        .packMonitorPort = PACK_MONITOR_PORT,
        .chainStatus = CHAIN_COMPLETE,
        .availableDevices[PORTA] = NUM_DEVICES_IN_ACCUMULATOR,
        .availableDevices[PORTB] = NUM_DEVICES_IN_ACCUMULATOR,
        .currentPort = PORTA,
        .localCommandCounter[CELL_MONITOR] = 0,
        .localCommandCounter[PACK_MONITOR] = 0
    };

    Soc_S defaultSocInfo = {
        .packMilliCoulombs = PACK_MILLICOULOMBS,
        .milliCoulombCounter = 0,
        .socByOcvQualificationTimer = (Timer_S){.timCount = CELL_POLARIZATION_REST_MS, .lastUpdate = 0, .timThreshold = CELL_POLARIZATION_REST_MS},
        .socByOcv = 0.0f,
        .soeByOcv = 0.0f,
        .socByCoulombCounting = 0.0f,
        .soeByCoulombCounting = 0.0f
    };

    vTaskSuspendAll();
    telemetryTaskData.chainInfo = defaultChainInfo;
    telemetryTaskData.socData = defaultSocInfo;
    xTaskResumeAll();
}

void runTelemetryTask()
{
    // Create local data struct for bmb information
    telemetryTaskData_S telemetryTaskDataLocal;

    // Copy in last cycles data into local data struct
    vTaskSuspendAll();
    telemetryTaskDataLocal = telemetryTaskData;
    xTaskResumeAll();

    TRANSACTION_STATUS_E telemetryStatus = TRANSACTION_SUCCESS;

    if(chainInitialized)
    {
        // Ready up isospi comms
        readyChain(&telemetryTaskDataLocal.chainInfo);

        // If the chain is not complete, attempt to fix it before starting task transactions
        if(telemetryTaskDataLocal.chainInfo.chainStatus != CHAIN_COMPLETE)
        {
            TRANSACTION_STATUS_E chainStatus = updateChainStatus(&telemetryTaskDataLocal.chainInfo);
            if(chainStatus != TRANSACTION_COMMAND_COUNTER_ERROR)
            {
                telemetryStatus = chainStatus;
            }
        }

        // Run system diagnostics
        if((telemetryStatus == TRANSACTION_SUCCESS) || (telemetryStatus == TRANSACTION_CHAIN_BREAK_ERROR))
        {
            telemetryStatus = runCommandBlock(runSystemDiagnostics, &telemetryTaskDataLocal);
        }

        // Configure registers for new telemetry data read
        if((telemetryStatus == TRANSACTION_SUCCESS) || (telemetryStatus == TRANSACTION_CHAIN_BREAK_ERROR))
        {
            telemetryStatus = runCommandBlock(startNewReadCycle, &telemetryTaskDataLocal);
        }

        // Read in cell voltages
        if((telemetryStatus == TRANSACTION_SUCCESS) || (telemetryStatus == TRANSACTION_CHAIN_BREAK_ERROR))
        {
            telemetryStatus = runCommandBlock(updateCellVoltages, &telemetryTaskDataLocal);
        }

        // Read in cell temps
        if((telemetryStatus == TRANSACTION_SUCCESS) || (telemetryStatus == TRANSACTION_CHAIN_BREAK_ERROR))
        {
            telemetryStatus = runCommandBlock(updateCellTemps, &telemetryTaskDataLocal);
        }

        if((telemetryStatus == TRANSACTION_SUCCESS) || (telemetryStatus == TRANSACTION_CHAIN_BREAK_ERROR))
        {
            updateSocSoe(&telemetryTaskDataLocal.socData, &socByOcvTable, &soeFromSocTable, telemetryTaskDataLocal.minCellVoltage, 0);
        }
    }

    if(!chainInitialized || (telemetryStatus == TRANSACTION_POR_ERROR))
    {
        Debug("Initializing chain...\n");
        wakeChain(&telemetryTaskDataLocal.chainInfo);
        telemetryStatus = runCommandBlock(initChain, &telemetryTaskDataLocal);
        if((telemetryStatus == TRANSACTION_SUCCESS) || (telemetryStatus == TRANSACTION_CHAIN_BREAK_ERROR))
        {
            Debug("BMB initialization successful!\n");
            chainInitialized = true;
        }
        else
        {
            Debug("BMBs failed to initialize!\n");
            chainInitialized = false;
        }
    }

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

    // Blah blah alert monitor

    // TODO Handle case of continous POR / CC errors here

    if(telemetryStatus == TRANSACTION_SUCCESS || telemetryStatus == TRANSACTION_CHAIN_BREAK_ERROR)
    {
        // Copy out new data into global data struct
        vTaskSuspendAll();
        telemetryTaskData = telemetryTaskDataLocal;
        xTaskResumeAll();
    }
}
