/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include "telemetryTask.h"
#include "main.h"
#include "cmsis_os.h"
#include "utils.h"
#include "debug.h"
#include <string.h>
#include <stdbool.h>

#define NUM_READS  1

/* ==================================================================== */
/* ============================= DEFINES ============================== */
/* ==================================================================== */

#define NUM_COMMAND_BLOCK_RETRYS            3

#define CELLS_PER_REG       3
#define CELL_REG_SIZE       (REGISTER_SIZE_BYTES / CELLS_PER_REG)

#define ENERGY_DATA_PER_REG 2
#define ENERGY_DATA_SIZE    (REGISTER_SIZE_BYTES / ENERGY_DATA_PER_REG)

#define VADC_GAIN           0.00015f
#define VADC_OFFSET         1.5f

#define IADC_GAIN           0.000001f
#define VBADC1_GAIN         0.0001f
#define VBADC2_GAIN         0.000085f

#define NUM_CELLV_REGISTERS 6
#define NUM_AUXV_REGISTERS  3

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
static bool initChain(CHAIN_INFO_S *chainInfo);
static TRANSACTION_STATUS_E runSystemDiagnostics(telemetryTaskData_S *taskData);
static TRANSACTION_STATUS_E startNewReadCycle(telemetryTaskData_S *taskData);
static TRANSACTION_STATUS_E updateCellVoltages(telemetryTaskData_S *taskData);
static TRANSACTION_STATUS_E updateCellTemps(telemetryTaskData_S *taskData);
// static TRANSACTION_STATUS_E runSPinDiagnostics(telemetryTaskData_S *taskData);

// static TRANSACTION_STATUS_E updateTestData(telemetryTaskData_S *taskData);

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

#define HANDLE_ISOSPI_ERROR(error) \
    if(error != TRANSACTION_SUCCESS) { \
        if(error == TRANSACTION_SPI_ERROR) \
        { \
            Debug("SPI Failure, reseting STM32...\n"); \
            HAL_NVIC_SystemReset(); \
        } \
        else if(error == TRANSACTION_POR_ERROR) \
        { \
            Debug("Power reset detected, reinitializing...\n"); \
            return TRANSACTION_POR_ERROR; \
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

#define CONVERT_IADC1(reg)          ((((int32_t)(EXTRACT_24_BIT(reg) << BITS_IN_BYTE)) /  BYTE_SIZE_DEC) * IADC_GAIN)

#define CONVERT_IADC2(reg)          ((((int32_t)(EXTRACT_24_BIT(reg) << BITS_IN_BYTE)) /  BYTE_SIZE_DEC) * IADC_GAIN * 1.0f)

#define CONVERT_VBADC1(reg)         ((((int32_t)(EXTRACT_24_BIT(reg) << BITS_IN_BYTE)) /  BYTE_SIZE_DEC) * VBADC1_GAIN)

#define CONVERT_VBADC2(reg)         ((((int32_t)(EXTRACT_24_BIT(reg) << BITS_IN_BYTE)) /  BYTE_SIZE_DEC) * VBADC2_GAIN * -1.0f)

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
            bmbArray[bmbOrder[j]].cellTemp[(cellStartIndex + (i * 2))] = adcVoltage;
            bmbArray[bmbOrder[j]].cellTempStatus[(cellStartIndex + (i * 2))] = GOOD;
        }
    }
}

static bool initChain(CHAIN_INFO_S *chainInfo)
{
    // Create and clear rx buffer
    uint8_t rxBuffer[REGISTER_SIZE_BYTES * NUM_DEVICES_IN_ACCUMULATOR];
    memset(rxBuffer, 0, REGISTER_SIZE_BYTES * NUM_DEVICES_IN_ACCUMULATOR);

    // Create and clear tx buffer
    uint8_t txBuffer[REGISTER_SIZE_BYTES * NUM_DEVICES_IN_ACCUMULATOR];
    memset(txBuffer, 0, REGISTER_SIZE_BYTES * NUM_DEVICES_IN_ACCUMULATOR);

    TRANSACTION_STATUS_E status;

    for(int32_t attempt = 0; attempt < NUM_COMMAND_BLOCK_RETRYS; attempt++)
    {
        // Start ADCs
        status = commandChain((ADCV | ADC_CONT | ADC_RD), chainInfo, SHARED_COMMAND);
        HANDLE_INIT_ERROR(status);

        // Start Aux ADC on BMBs
        status = commandChain(ADAX, chainInfo, CELL_MONITOR_COMMAND);
        HANDLE_INIT_ERROR(status);

        // Clear SLEEP Bit
        for(int32_t i = 0; i < NUM_DEVICES_IN_ACCUMULATOR; i++)
        {
            txBuffer[(i * REGISTER_SIZE_BYTES) + 5] = STATC_SLEEP_BIT;
        }

        status = writeChain(CLRFLAG, chainInfo, SHARED_COMMAND, txBuffer);
        HANDLE_INIT_ERROR(status);

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

        status = writeChain(WRCFGA, chainInfo, SHARED_COMMAND, txBuffer);
        HANDLE_INIT_ERROR(status);

        // Verify command counter
        status = readChain(RDSID, chainInfo, rxBuffer);
        HANDLE_INIT_ERROR(status);

        Debug("BMB initialization successful!\n");
        return true;
    }

    Debug("BMBs failed to initialize!\n");
    return false;
}

static TRANSACTION_STATUS_E runSystemDiagnostics(telemetryTaskData_S *taskData)
{
    // Create and clear rx buffer
    uint8_t rxBuffer[REGISTER_SIZE_BYTES * NUM_DEVICES_IN_ACCUMULATOR];
    memset(rxBuffer, 0, REGISTER_SIZE_BYTES * NUM_DEVICES_IN_ACCUMULATOR);

    // ISOSPI status variable
    TRANSACTION_STATUS_E status;

    for(int32_t attempt = 0; attempt < NUM_COMMAND_BLOCK_RETRYS; attempt++)
    {
        // Read status register group C
        status = readChain(RDSTATC, &taskData->chainInfo, rxBuffer);
        HANDLE_ISOSPI_ERROR(status);

        // This code may be unessecary given how the commandChain function reinits the chain every cycle
        // Check for the sleep bit to detect a power reset
        for(uint32_t i = NUM_PACK_MON_IN_ACCUMULATOR; i < NUM_DEVICES_IN_ACCUMULATOR; i++)
        {
            // Extract the sleep bit from each data packet, and check if it is set
            uint8_t statC5 = rxBuffer[(i * REGISTER_SIZE_BYTES) + 5];
            if(statC5 & STATC_SLEEP_BIT)
            {
                Debug("Power reset detected, reinitializing...\n");
                return TRANSACTION_POR_ERROR;
            }
        }

        return TRANSACTION_SUCCESS;
    }
    return TRANSACTION_COMMAND_COUNTER_ERROR;
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

    for(int32_t attempt = 0; attempt < NUM_COMMAND_BLOCK_RETRYS; attempt++)
    {
        // Unfreeze all device registers
        status = commandChain(UNSNAP, &taskData->chainInfo, SHARED_COMMAND);
        HANDLE_ISOSPI_ERROR(status);

        // Freeze all device registers
        status = commandChain(SNAP, &taskData->chainInfo, SHARED_COMMAND);
        HANDLE_ISOSPI_ERROR(status);

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
        HANDLE_ISOSPI_ERROR(status);

        // Verify command counter
        status = readChain(RDSID, &taskData->chainInfo, rxBuffer);
        HANDLE_ISOSPI_ERROR(status);

        return TRANSACTION_SUCCESS;
    }
    return TRANSACTION_COMMAND_COUNTER_ERROR;
}

static TRANSACTION_STATUS_E updateCellVoltages(telemetryTaskData_S *taskData)
{
    // Create and clear rx buffer
    uint8_t rxBuffer[NUM_CELLV_REGISTERS][REGISTER_SIZE_BYTES * NUM_DEVICES_IN_ACCUMULATOR];
    memset(rxBuffer, 0, NUM_CELLV_REGISTERS * REGISTER_SIZE_BYTES * NUM_DEVICES_IN_ACCUMULATOR);

    // ISOSPI status variable
    TRANSACTION_STATUS_E status;

    for(int32_t attempt = 0; attempt < NUM_COMMAND_BLOCK_RETRYS; attempt++)
    {

        // Read cell voltage registers A-E, and populate cell voltages to data struct
        for(uint32_t i = 0; i < NUM_CELLV_REGISTERS; i++)
        {
            status = readChain(readVoltReg[i], &taskData->chainInfo, rxBuffer[i]);
            HANDLE_ISOSPI_ERROR(status); // TODO Command Counter bug

            // Ignore Pack monitor data for now, it is retained in the rxbuffer for later conversion
            convertCellVoltageRegister((rxBuffer[i] + (REGISTER_SIZE_BYTES * NUM_PACK_MON_IN_ACCUMULATOR)), (i * CELLS_PER_REG), taskData->bmb);
        }

        // Extract IADC1 and IADC2 data from current sense ADCs
        taskData->IADC1 = CONVERT_IADC1(rxBuffer[0]);
        taskData->IADC2 = CONVERT_IADC2((rxBuffer[0] + ENERGY_DATA_SIZE));

        // Extract VBADC1 and VBADC2 data from battery voltage ADCs
        taskData->VBADC1 = CONVERT_VBADC1(rxBuffer[1]);
        taskData->VBADC2 = CONVERT_VBADC2((rxBuffer[1] + ENERGY_DATA_SIZE));

        return TRANSACTION_SUCCESS;
    }
    return TRANSACTION_COMMAND_COUNTER_ERROR;
}

static TRANSACTION_STATUS_E updateCellTemps(telemetryTaskData_S *taskData)
{
    // Create and clear rx buffer
    uint8_t rxBuffer[NUM_AUXV_REGISTERS][REGISTER_SIZE_BYTES * NUM_DEVICES_IN_ACCUMULATOR];
    memset(rxBuffer, 0, NUM_AUXV_REGISTERS * REGISTER_SIZE_BYTES * NUM_DEVICES_IN_ACCUMULATOR);

    // ISOSPI status variable
    TRANSACTION_STATUS_E status;

    for(int32_t attempt = 0; attempt < NUM_COMMAND_BLOCK_RETRYS; attempt++)
    {
        // Read aux voltage registers A-C, and populate cell temps to data struct
        for(uint32_t i = 0; i < NUM_AUXV_REGISTERS; i++)
        {
            status = readChain(readAuxReg[i], &taskData->chainInfo, rxBuffer[i]);
            HANDLE_ISOSPI_ERROR(status); // TODO Command Counter bug

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
            taskData->bmb[bmbOrder[i]].boardTemp = adcVoltage;
            taskData->bmb[bmbOrder[i]].boardTempStatus = GOOD;
        }

        return TRANSACTION_SUCCESS;
    }
    return TRANSACTION_COMMAND_COUNTER_ERROR;
}

// static TRANSACTION_STATUS_E runSPinDiagnostics(telemetryTaskData_S *taskData);

// static TRANSACTION_STATUS_E updateTestData(telemetryTaskData_S *taskData)
// {
//     // Create and clear rx and tx buffers
//     uint8_t rxBuffer[REGISTER_SIZE_BYTES * NUM_DEVICES_IN_ACCUMULATOR];
//     memset(rxBuffer, 0, REGISTER_SIZE_BYTES * NUM_DEVICES_IN_ACCUMULATOR);

//     uint8_t txBuffer[REGISTER_SIZE_BYTES * NUM_DEVICES_IN_ACCUMULATOR];
//     memset(txBuffer, 0, REGISTER_SIZE_BYTES * NUM_DEVICES_IN_ACCUMULATOR);

//     // Calculate new data to seed in GPIO registers
//     for(int32_t i = 0; i < NUM_DEVICES_IN_ACCUMULATOR; i++)
//     {
//         static uint8_t reg = 0;
//         txBuffer[(REGISTER_SIZE_BYTES * i) + 3] = reg;
//         reg++;
//         if(reg > 0x0F)
//         {
//             reg = 0;
//         }
//     }

//     TRANSACTION_STATUS_E status;

//     for(int32_t attempt = 0; attempt < NUM_COMMAND_BLOCK_RETRYS; attempt++)
//     {
//         status = writeChain(WRCFGA, &taskData->chainInfo, SHARED_COMMAND, txBuffer);
//         HANDLE_ISOSPI_ERROR(status);

//         status = readChain(RDCFGA, &taskData->chainInfo, rxBuffer);
//         HANDLE_ISOSPI_ERROR(status);

//         for(int32_t i = 0; i < NUM_DEVICES_IN_ACCUMULATOR; i++)
//         {
//             taskData->testStatus[i] = status;
//             for(int32_t j = 0; j < REGISTER_SIZE_BYTES; j++)
//             {
//                 taskData->testData[i][j] = rxBuffer[j + (i * REGISTER_SIZE_BYTES)];
//             }
//         }
//         return TRANSACTION_SUCCESS;
//     }
//     return TRANSACTION_COMMAND_COUNTER_ERROR;
// }

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

    vTaskSuspendAll();
    telemetryTaskData.chainInfo = defaultChainInfo;
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

    // If chain not initialized, attempt to init
    if(!chainInitialized)
    {
        wakeChain(&telemetryTaskDataLocal.chainInfo);
        chainInitialized = initChain(&telemetryTaskDataLocal.chainInfo);
    }
    else
    {
        // Ready up isospi comms
        readyChain(&telemetryTaskDataLocal.chainInfo);

        // If the chain is not complete, attempt to fix it before starting task transactions
        if(telemetryTaskDataLocal.chainInfo.chainStatus != CHAIN_COMPLETE)
        {
            updateChainStatus(&telemetryTaskDataLocal.chainInfo);
        }

        // Run system diagnostics
        if(telemetryStatus == TRANSACTION_SUCCESS)
        {
            telemetryStatus = runSystemDiagnostics(&telemetryTaskDataLocal);
        }

        // Configure registers for new telemetry data read
        if(telemetryStatus == TRANSACTION_SUCCESS)
        {
            telemetryStatus = startNewReadCycle(&telemetryTaskDataLocal);
        }

        // Read in cell voltages
        if(telemetryStatus == TRANSACTION_SUCCESS)
        {
            telemetryStatus = updateCellVoltages(&telemetryTaskDataLocal);
        }

        // Read in cell temps
        if(telemetryStatus == TRANSACTION_SUCCESS)
        {
            telemetryStatus = updateCellTemps(&telemetryTaskDataLocal);
        }
    }

    if(telemetryStatus == TRANSACTION_POR_ERROR)
    {
        wakeChain(&telemetryTaskDataLocal.chainInfo);
        chainInitialized = initChain(&telemetryTaskDataLocal.chainInfo);
    }
    else if(telemetryTaskDataLocal.chainInfo.chainStatus == MULTIPLE_CHAIN_BREAK)
    {
        Debug("Chain Break!\n");
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
