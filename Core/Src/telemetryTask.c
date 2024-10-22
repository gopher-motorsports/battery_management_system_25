/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include "telemetryTask.h"
#include "main.h"
#include "cmsis_os.h"
#include "utils.h"
#include <string.h>



/* ==================================================================== */
/* ============================= DEFINES ============================== */
/* ==================================================================== */
#define NUM_READS  100

#define SNAP_COMMAND 0x002D
#define UNSNAP_COMMAND 0X002F
#define RDFLAG_COMMAND //idk what the command code is yet
#define RDIACC_COMMAND 0x0044


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

/* ==================================================================== */
/* =================== LOCAL FUNCTION DECLARATIONS ==================== */
/* ==================================================================== */

/* ==================================================================== */
/* ============================== MACROS ============================== */
/* ==================================================================== */

/* ==================================================================== */
/* =================== LOCAL FUNCTION DEFINITIONS ===================== */
/* ==================================================================== */
Soc_S soc = {.milliCoulombCounter = 0, .socByOcvQualificationTimer = (Timer_S){.timCount = SOC_BY_OCV_QUALIFICATION_TIME, .lastUpdate = 0, .timThreshold = SOC_BY_OCV_QUALIFICATION_TIME}, .socByOcv = 0, .soeByOcv = 0,.socByCoulombCounting = 0, .socByCoulombCounting = 0};



/* ==================================================================== */
/* =================== GLOBAL FUNCTION DEFINITIONS ==================== */
/* ==================================================================== */

void initTelemetryTask()
{
    HAL_GPIO_WritePin(MAS1_GPIO_Port, MAS1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(MAS2_GPIO_Port, MAS2_Pin, GPIO_PIN_SET);

    HAL_GPIO_WritePin(PORTA_CS_GPIO_Port, PORTA_CS_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PORTB_CS_GPIO_Port, PORTB_CS_Pin, GPIO_PIN_SET);

    wakeChain(NUM_BMBS_IN_ACCUMULATOR);
}

void runTelemetryTask()
{
    // Create local data struct for bmb information
    TelemetryTaskOutputData_S telemetryTaskOutputDataLocal;
    TelemetryStaticData_S telemetryTaskStaticDataLocal;
    //// Test Transaction

    // WAKE BMBs

    // delayMicroseconds(5000);
    // HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
    // delayMicroseconds(5000);
    // HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);

    // Transaction

    uint8_t rxBuff[REGISTER_SIZE_BYTES * NUM_BMBS_IN_ACCUMULATOR];
    memset(rxBuff, 0, REGISTER_SIZE_BYTES * NUM_BMBS_IN_ACCUMULATOR);

    uint8_t txBuffer[REGISTER_SIZE_BYTES * NUM_BMBS_IN_ACCUMULATOR];
    memset(txBuffer, 0, REGISTER_SIZE_BYTES * NUM_BMBS_IN_ACCUMULATOR);


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

    readyChain(NUM_BMBS_IN_ACCUMULATOR);

    writeChain(0x0001, NUM_BMBS_IN_ACCUMULATOR, txBuffer);

    TRANSACTION_STATUS_E status = readChain(0x0002, NUM_BMBS_IN_ACCUMULATOR, rxBuff);

    for(int32_t i = 0; i < NUM_BMBS_IN_ACCUMULATOR; i++)
    {
        telemetryTaskOutputDataLocal.bmb[i].testStatus = status;
        for(int32_t j = 0; j < REGISTER_SIZE_BYTES; j++)
        {
            telemetryTaskOutputDataLocal.bmb[i].testData[j] = rxBuff[j + (i * REGISTER_SIZE_BYTES)];
        }
    }

    //test status = to the error code we get back from the read register function 

    //// End test transaction

    taskENTER_CRITICAL();
    telemetryTaskOutputData = telemetryTaskOutputDataLocal;
    taskEXIT_CRITICAL();

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
