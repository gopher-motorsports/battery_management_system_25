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

/* ==================================================================== */
/* ========================= ENUMERATED TYPES========================== */
/* ==================================================================== */

/* ==================================================================== */
/* ============================== STRUCTS ============================= */
/* ==================================================================== */

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

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DEFINITIONS ==================== */
/* ==================================================================== */

void initTelemetryTask()
{
    wakeChain(NUM_BMBS_IN_ACCUMULATOR, PORTB);
}

void runTelemetryTask()
{
    // Create local data struct for bmb information
    TelemetryTaskOutputData_S telemetryTaskOutputDataLocal;

    //// Test Transaction

    // WAKE BMBs

    // delayMicroseconds(5000);
    // HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
    // delayMicroseconds(5000);
    // HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);

    // Transaction

    uint8_t rxBuff[REGISTER_SIZE_BYTES * NUM_BMBS_IN_ACCUMULATOR];
    memset(rxBuff, 0, REGISTER_SIZE_BYTES * NUM_BMBS_IN_ACCUMULATOR);

    readyChain(NUM_BMBS_IN_ACCUMULATOR, PORTB);

    telemetryTaskOutputDataLocal.bmb[0].testStatus = readRegister(0x0002, NUM_BMBS_IN_ACCUMULATOR, rxBuff, PORTB);

    for(int32_t i = 0; i < NUM_BMBS_IN_ACCUMULATOR; i++)
    {
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