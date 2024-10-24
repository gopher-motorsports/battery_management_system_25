/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include "telemetryTask.h"
#include "main.h"
#include "cmsis_os.h"
#include "utils.h"
#include <string.h>

#define NUM_READS  100

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
    HAL_GPIO_WritePin(MAS1_GPIO_Port, MAS1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(MAS2_GPIO_Port, MAS2_Pin, GPIO_PIN_SET);

    HAL_GPIO_WritePin(PORTA_CS_GPIO_Port, PORTA_CS_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PORTB_CS_GPIO_Port, PORTB_CS_Pin, GPIO_PIN_SET);

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

    for(int32_t n = 0; n < NUM_READS; n++)
    {
        TRANSACTION_STATUS_E status = readRegister(0x0002, NUM_BMBS_IN_ACCUMULATOR, rxBuff, PORTB);

        for(int32_t i = 0; i < NUM_BMBS_IN_ACCUMULATOR; i++)
        {
            telemetryTaskOutputDataLocal.bmb[i].testStatus = status;
            for(int32_t j = 0; j < REGISTER_SIZE_BYTES; j++)
            {
                telemetryTaskOutputDataLocal.bmb[i].testData[j] = rxBuff[j + (i * REGISTER_SIZE_BYTES)];
            }
        }
    }

    //test status = to the error code we get back from the read register function 

    //// End test transaction

    taskENTER_CRITICAL();
    telemetryTaskOutputData = telemetryTaskOutputDataLocal;
    taskEXIT_CRITICAL();

}