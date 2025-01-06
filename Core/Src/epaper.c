/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */
#include "adbms.h"
#include "main.h"
#include "utils.h"

#include <string.h>
#include <stdbool.h>



/* ==================================================================== */
/* ============================= DEFINES ============================== */
/* ==================================================================== */

// SPI timeout period
#define SPI_TIMEOUT_MS          100

#define BUSY_TIMEOUT_MS         20000

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
/* ======================= EXTERNAL VARIABLES ========================= */
/* =============================

======================================= */


extern SPI_HandleTypeDef hspi2;

/* ==================================================================== */
/* =================== LOCAL FUNCTION DECLARATIONS ==================== */
/* ==================================================================== */

/* ==================================================================== */
/* ======================== LOCAL FUNCTIONS =========================== */
/* ==================================================================== */


//  openCS, closeCs, reset, sendCommand, sendData, waitBusy, etc

static void openCS(void)
{
    HAL_GPIO_WritePin(EPAP_CS_GPIO_Port, EPAP_CS_Pin, GPIO_PIN_RESET);
}

static void closeCS(void)
{
    HAL_GPIO_WritePin(EPAP_CS_GPIO_Port, EPAP_CS_Pin, GPIO_PIN_SET);

}

static void reset(void)
{
    HAL_GPIO_WritePin(EPAP_RST_GPIO_Port, EPAP_RST_Pin, GPIO_PIN_RESET);
    vTaskDelay(10);
    HAL_GPIO_WritePin(EPAP_RST_GPIO_Port, EPAP_RST_Pin, GPIO_PIN_SET);
    vTaskDelay(10);
}

void sendCommand(uint8_t command)
{
    uint8_t txBuffer[1] = {command};
    uint8_t rxBuffer[1] = {0};


    HAL_GPIO_WritePin(EPAP_DC_GPIO_Port, EPAP_DC_Pin, GPIO_PIN_RESET);

    // SPIify
    openCS();
    if(taskNotifySPI(&hspi2, txBuffer, rxBuffer, 1, SPI_TIMEOUT_MS) != SPI_SUCCESS)
    {
        closeCS();
    }
    closeCS();
}

void sendData(uint8_t *data, uint32_t size)
{
    uint8_t rxBuffer[size];

    HAL_GPIO_WritePin(EPAP_DC_GPIO_Port, EPAP_DC_Pin, GPIO_PIN_SET);

    // SPIify
    openCS();
    if(taskNotifySPI(&hspi2, data, rxBuffer, size, SPI_TIMEOUT_MS) != SPI_SUCCESS)
    {
        closeCS();
    }
    closeCS();
}


static void waitBusy(void)
{
    if(xTaskNotifyWait(0, 0, NULL, BUSY_TIMEOUT_MS) != pdTRUE)
    {
        //return error 
    }

    
}

/* ==================================================================== */
/* ======================== PUBLIC FUNCTIONS ========================== */
/* ==================================================================== */

//functions that update dislpay




//TODO
// error handling
    //SPI ERROR - GIVE UP ON TRANSACTION
    // TIMEOUT - DOESNT MATTER ? WAIT MAX TIMEOUT 
// MAKE SPI BETTER 

