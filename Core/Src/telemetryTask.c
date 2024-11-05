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



/* ==================================================================== */
/* ============================= DEFINES ============================== */
/* ==================================================================== */

#define NUM_COMMAND_BLOCK_RETRYS            3
#define NUM_READS  100

#define SNAP_COMMAND 0x2D
#define UNSNAP_COMMAND 0X2F
#define RDFLAG_COMMAND 0x72 //WITHOUT error injection (1) with artificial error injection (0)
#define RDIACC_COMMAND 0x44


#define CONVERSION_MULTI 36  //conversion time (need to change)
#define IADC_LSB 1e-6
/* ==================================================================== */
/* ========================= ENUMERATED TYPES========================== */
/* ==================================================================== */
typedef enum
{
    PORTA = 0,
    PORTB,
    NUM_PORTS
} PORT_E;
/* ==================================================================== */
/* ============================== STRUCTS ============================= */
/* ==================================================================== */
typedef struct 
{
    int32_t milliCoulombCounter;         // The coulomb counter
    Timer_S socByOcvQualificationTimer;  // The qualification timer to determine whether SOC by OCV can be used
} TelemetryStaticData_S;

/* ==================================================================== */
/* ========================= LOCAL VARIABLES ========================== */
/* ==================================================================== */

bool chainInitialized;
bool chainBreak;

PORT_E port = PORTA;
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
            Debug("Chain break!\n"); \
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
        TRANSACTION_STATUS_E status = writeChain(WR_CFG_REG_A, NUM_BMBS_IN_ACCUMULATOR, txBuffer);
        HANDLE_ISOSPI_ERROR(status);

        status = readChain(RD_CFG_REG_A, NUM_BMBS_IN_ACCUMULATOR, rxBuffer);
        HANDLE_ISOSPI_ERROR(status);


        for(int32_t i = 0; i < NUM_BMBS_IN_ACCUMULATOR; i++)
        {
            taskData->bmb[i].testStatus = status;
            for(int32_t j = 0; j < REGISTER_SIZE_BYTES; j++)
            {
                taskData->bmb[i].testData[j] = rxBuffer[j + (i * REGISTER_SIZE_BYTES)];
            }
        }
        return TRANSACTION_SUCCESS;
    }
    return TRANSACTION_COMMAND_COUNTER_ERROR;
}
Soc_S soc = {.milliCoulombCounter = 0, 
            .socByOcvQualificationTimer = (Timer_S){.timCount = SOC_BY_OCV_GOOD_QUALIFICATION_TIME_MS, .lastUpdate = 0, .timThreshold = SOC_BY_OCV_GOOD_QUALIFICATION_TIME_MS}, 
            .socByOcv = 0, 
            .soeByOcv = 0,
            .socByCoulombCounting = 0, 
            .socByCoulombCounting = 0};



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
    TelemetryStaticData_S telemetryTaskStaticDataLocal;
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

static void updateCoulombCounter(Soc_S soc, PORT_E port){
    static uint16_t N = 1;
    static uint16_t I1CNT_OLD = 0;

    uint8_t rxBuff[REGISTER_SIZE_BYTES * NUM_BMBS_IN_ACCUMULATOR];
    memset(rxBuff, 0, REGISTER_SIZE_BYTES * NUM_BMBS_IN_ACCUMULATOR);

    uint8_t txBuffer[REGISTER_SIZE_BYTES * NUM_BMBS_IN_ACCUMULATOR];
    memset(txBuffer, 0, REGISTER_SIZE_BYTES * NUM_BMBS_IN_ACCUMULATOR);


    // //read conversion counter and current
    // uint16_t I1CNT = readRegister(RDIACC_COMMAND, NUM_BMBS_IN_ACCUMULATOR, rxBuff, port);
    // //calculate conversion time
    // float t_conv = TELEMETRY_TASK_PERIOD_MS / CONVERSION_MULTI;

    // // calculate charge using coulomb counting formula
    // float charge = t_conv * (RDIACC_COMMAND * IADC_LSB);
}
/*
TODO: 
- draw current
- accumulation reg
- conversion time 
- caluclate deltaMC
*/

// void readSequence(PORT_E port) {
//     //unfreeze all registers
//     sendCommand(UNSNAP, port);

//     // freeze all registers
//     sendCommand(SNAP, port);

//     // get I1CNTPHA flag (conversion counter)
//     sendCommand(RDFLAG, port);

//     // get IxACC value (accumulator reg)
//     sendCommand(RDIACC, port);
// }

// void countCoulombs(Soc_S* soc, PORT_E port) {
//     // Read the accumulated conversion results from IxACC
//     uint16_t IxACC = calculateAccReg();
//     float t_conv = calculateADCConversionTime();

//     // calculate charge using coulomb counting formula
//     float charge = t_conv * (IxACC * IADC_LSB);
    
//     // update CC in mC
//     soc->coulombCounter.accumulatedMilliCoulombs += charge * 1000; 

// }

// float calculateADCConversionTime(void) {

//     return TELEMETRY_TASK_PERIOD_MS / CONVERSION_MULTI;
// }


// float calculateAccReg(){
//     return sendCommand(RDIACC, port);
// }

// float calculateConvCounter(){
//     uint16_t I1CNTPHA = sendCommand(RDFLAG, port);
//     return  I1CNTPHA >> 2; //get I1CNT by itself
// }


// void updateSOCandSOEbyCC(Soc_S* soc, PORT_E port) {
//     static uint16_t N = 1;
//     static uint16_t I1CNT_OLD = 0;

//     //perform the read sequence
//     readSequence(port);

//     uint16_t I1CNT = calculateConvCounter();

//     // Check rollover
//     if (I1CNT < I1CNT_OLD) {
//         N = 0;
//     }

//     // Check if new conversion is available
//     if (I1CNT >= N * 8) {
//         N++;
//         countCoulombs(soc, port);  // Count the new coulombs
//         calculateSocAndSoeByCC(soc); // Update SOC and SOE based on CC
//     }

//     I1CNT_OLD = I1CNT;  // Update I1CNT_OLD
// }
