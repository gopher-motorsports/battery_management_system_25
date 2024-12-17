/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include <stdbool.h>
#include "utils.h"

/* ==================================================================== */
/* ============================= DEFINES ============================== */
/* ==================================================================== */

// Delay task timeout
#define US_DELAY_TIMEOUT    10

// Task notification flags
#define TASK_NO_OP          0UL
#define TASK_CLEAR_FLAGS    0xffffffffUL

// Number of SPI retry events
#define NUM_SPI_RETRY       3

/* ==================================================================== */
/* ======================= EXTERNAL VARIABLES ========================= */
/* ==================================================================== */

extern bool usDelayActive;
extern TIM_HandleTypeDef htim7;

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DEFINITIONS ==================== */
/* ==================================================================== */

void delayMicroseconds(uint32_t us)
{
    if(!usDelayActive)
    {
        usDelayActive = true;
        __HAL_TIM_SET_AUTORELOAD(&htim7, us - 1);
        HAL_TIM_Base_Start_IT(&htim7);
        xTaskNotifyWait(0, 0, NULL, US_DELAY_TIMEOUT);
    }
}

SPI_STATUS_E taskNotifySPI(SPI_HandleTypeDef* hspi, uint8_t* txBuffer, uint8_t* rxBuffer, uint16_t size, uint32_t timeout)
{
    for(uint32_t attemptNum = 0; attemptNum < NUM_SPI_RETRY; attemptNum++)
    {
        // Attempt to start SPI transaction
        if(HAL_SPI_TransmitReceive_IT(hspi, txBuffer, rxBuffer, size) != HAL_OK)
        {
            // If SPI fails to start, HAL must abort transaction. SPI retries
            HAL_SPI_Abort_IT(hspi);
            continue;
        }

        // Wait for SPI interrupt to occur. NotificationFlags will hold notification value indicating status of transaction
        uint32_t notificationFlags = 0;
        if(xTaskNotifyWait(TASK_NO_OP, TASK_CLEAR_FLAGS, &notificationFlags, timeout) != pdTRUE)
        {
            // If no SPI interrupt occurs in time, transaction is aborted to prevent any longer delay
            HAL_SPI_Abort_IT(hspi);
            return SPI_TIMEOUT;
        }

        // If SPI SUCCESS bit is set in notification value, return success
        if(notificationFlags & SPI_SUCCESS)
        {
            return SPI_SUCCESS;
        }
    }

    // After all failed attempts return spi error
    return SPI_ERROR;

}
