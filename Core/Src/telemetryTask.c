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

#define NUM_READS  100

/* ==================================================================== */
/* ============================= DEFINES ============================== */
/* ==================================================================== */

#define NUM_COMMAND_BLOCK_RETRYS            3

/* ==================================================================== */
/* ========================= ENUMERATED TYPES========================== */
/* ==================================================================== */

/* ==================================================================== */
/* ============================== STRUCTS ============================= */
/* ==================================================================== */

/* ==================================================================== */
/* ========================= LOCAL VARIABLES ========================== */
/* ==================================================================== */

bool chainInitialized;
bool chainBreak;

/* ==================================================================== */
/* =================== LOCAL FUNCTION DECLARATIONS ==================== */
/* ==================================================================== */

static bool initChain();
static TRANSACTION_STATUS_E updateTestData(TelemetryTaskOutputData_S *taskData);

/* ==================================================================== */
/* ============================== MACROS ============================== */
/* ==================================================================== */

#define HANDLE_ISOSPI_ERROR(error) \
    if(error != TRANSACTION_SUCCESS) { \
        if(error == TRANSACTION_CHAIN_BREAK_ERROR) \
        { \
            chainBreak = true; \
        } \
        else if(error == TRANSACTION_SPI_ERROR) \
        { \
            Debug("SPI Failure, reseting STM32...\n"); \
            HAL_NVIC_SystemReset(); \
        } \
        else if(error == TRANSACTION_POR_ERROR) \
        { \
            Debug("Power reset detected, reinitializing...\n"); \
            chainInitialized = initChain(); \
            return TRANSACTION_POR_ERROR; \
        } \
        else if(error == TRANSACTION_COMMAND_COUNTER_ERROR) \
        { \
            Debug("Command counter mismatch! Retrying command block!\n"); \
            continue; \
        } \
        else \
        { \
            Debug("Unknown transaction error\n"); \
            return; \
        } \
    }

/* ==================================================================== */
/* =================== LOCAL FUNCTION DEFINITIONS ===================== */
/* ==================================================================== */

static bool initChain()
{
    bool initSuccess = true;

    wakeChain(NUM_BMBS_IN_ACCUMULATOR);

    if(!initSuccess)
    {
        Debug("BMBs failed to initialize!\n");
        return false;
    }
    Debug("BMB initialization successful!\n");
    return true;
}

static TRANSACTION_STATUS_E updateTestData(TelemetryTaskOutputData_S *taskData)
{
    // Create and clear rx and tx buffers
    uint8_t rxBuffer[REGISTER_SIZE_BYTES * NUM_BMBS_IN_ACCUMULATOR];
    memset(rxBuffer, 0, REGISTER_SIZE_BYTES * NUM_BMBS_IN_ACCUMULATOR);

    uint8_t txBuffer[REGISTER_SIZE_BYTES * NUM_BMBS_IN_ACCUMULATOR];
    memset(txBuffer, 0, REGISTER_SIZE_BYTES * NUM_BMBS_IN_ACCUMULATOR);

    // Calculate new data to seed in GPIO registers
    for(int32_t i = 0; i < NUM_BMBS_IN_ACCUMULATOR; i++)
    {
        static uint8_t reg = 0;
        txBuffer[(REGISTER_SIZE_BYTES * i) + 3] = reg;
        reg++;
        if(reg > 0x0F)
        {
            reg = 0;
        }
    }

    for(int32_t attempt = 0; attempt < NUM_COMMAND_BLOCK_RETRYS; attempt++)
    {
        HANDLE_ISOSPI_ERROR(writeChain(WR_CFG_REG_A, NUM_BMBS_IN_ACCUMULATOR, txBuffer));
        HANDLE_ISOSPI_ERROR(readChain(RD_CFG_REG_A, NUM_BMBS_IN_ACCUMULATOR, rxBuffer));

        for(int32_t i = 0; i < NUM_BMBS_IN_ACCUMULATOR; i++)
        {
            for(int32_t j = 0; j < REGISTER_SIZE_BYTES; j++)
            {
                taskData->bmb[i].testData[j] = rxBuffer[j + (i * REGISTER_SIZE_BYTES)];
            }
        }
        return TRANSACTION_SUCCESS;
    }
    return TRANSACTION_COMMAND_COUNTER_ERROR;
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
}

void runTelemetryTask()
{
    // Create local data struct for bmb information
    TelemetryTaskOutputData_S telemetryTaskOutputDataLocal;

    TRANSACTION_STATUS_E telemetryStatus = TRANSACTION_SUCCESS;

    chainBreak = false;

    // If chain not initialized, attempt to init
    if(!chainInitialized)
    {
        chainInitialized = initChain();
    }
    else
    {
        // Ready up isospi comms
        readyChain(NUM_BMBS_IN_ACCUMULATOR);

        telemetryStatus = updateTestData(&telemetryTaskOutputDataLocal);

        // if(telemetryStatus == TRANSACTION_SUCCESS)
        // {

        // }
    }


    // Regardless of whether or not chain initialized, run alert monitor stuff

    // Blah blah alert monitor

    // TODO Handle case of continous POR / CC errors here

    if(telemetryStatus == TRANSACTION_SUCCESS || telemetryStatus == TRANSACTION_CHAIN_BREAK_ERROR)
    {
        // Copy out new data into global data struct
        taskENTER_CRITICAL();
        telemetryTaskOutputData = telemetryTaskOutputDataLocal;
        taskEXIT_CRITICAL();  
    }

}