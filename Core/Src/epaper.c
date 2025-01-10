/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */
#include "adbms.h"
#include "main.h"
#include "utils.h"
#include "epaper.h"

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
void lcd_chkstatus(void)
{
    while(HAL_GPIO_ReadPin(EPAP_BUSY_GPIO_Port, EPAP_BUSY_Pin) == GPIO_PIN_SET)
    {
        vTaskDelay(10);
    }
}

//Full screen update initialization
void EPD_init(void)
{
    sendCommand(0x01); // power setting
    uint8_t data[] = {0x03, 0x00, 0x2b, 0x2b, 0x09};
    sendData(data, 5);

    sendCommand(0x06); // booster soft start
    uint8_t boosterData[] = {0x17, 0x17, 0x17};
    sendData(boosterData, 3);

    sendCommand(0x04); // power on
    lcd_chkstatus();

    sendCommand(0x00); // panel setting
    uint8_t panelData = 0xbf;
    sendData(&panelData, 1);

    sendCommand(0x30); // PLL control
    uint8_t pllData = 0x3c;
    sendData(&pllData, 1);

    sendCommand(0x61); // resolution setting
    uint8_t resData[] = {0x01, 0x90, 0x01, 0x2c};
    sendData(resData, 4);

    sendCommand(0x82); // VCOM_DC setting
    uint8_t vcomData = 0x12;
    sendData(&vcomData, 1);

    sendCommand(0x50); // VCOM and data interval setting
    uint8_t vcomIntervalData = 0x97;
    sendData(&vcomIntervalData, 1);

    sendCommand(0x20); // display update control
    uint8_t updateData[] = {0x0f, 0x89};
    sendData(updateData, 2);
}



//Display update function
void EPD_update(void)
{
    sendCommand(0x12); //display update
    vTaskDelay(1); //delay of at least 200uS
    lcd_chkstatus(); //wait for the electronic paper IC to release the idle signal
}

void EPD_WhiteScreen_All(const unsigned char *data)
{
    sendCommand(0x10); //write old data
    for(uint32_t i = 0; i < EPD_ARRAY; i++)
    {
        uint8_t zero = 0x00;
        sendData(&zero, 1);
    }

    sendCommand(0x13); //write new data
    for(uint32_t i = 0; i < EPD_ARRAY; i++)
    {
        sendData(data[i], 1);
    }

    EPD_update();
}

//clear screen display
void EPD_WhiteScreen(void)
{
    sendCommand(0x10); //write old data
    for(uint32_t i = 0; i < EPD_ARRAY; i++)
    {
        uint8_t zero = 0x00;
        sendData(&zero, 1);
    }
    sendCommand(0x13); //write new data
    for(uint32_t i = 0; i < EPD_ARRAY; i++)
    {
        sendData(&zero, 1);
    }
    EPD_update();
}

//display all black
void EPD_BlackScreen(void)
{
    sendCommand(0x10); //write old data
    for(uint32_t i = 0; i < EPD_ARRAY; i++)
    {
        uint8_t zero = 0x00;
        sendData(&zero, 1);
    }
    sendCommand(0x13); //write new data
    for(uint32_t i = 0; i < EPD_ARRAY; i++)
    {
        uint8_t one = 0xff;
        sendData(&one, 1);
    }
    EPD_update();
}

//partial update of background display
void EPD_SetRamValue_BaseMap(const unsigned char *data)
{
    sendCommand(0x10); //write old data
    for(uint32_t i = 0; i < EPD_ARRAY; i++)
    {
        uint8_t one = 0xff;
        sendData(&one, 1);
    }
    sendCommand(0x13); //write new data
    for(uint32_t i = 0; i < EPD_ARRAY; i++)
    {
        sendData(data[i], 1);
    }
    EPD_update();
}

//Partial update display
void EPD_Display_Partial(unsigned int x_start,unsigned int y_start,const unsigned char * datas,unsigned int PART_COLUMN,unsigned int PART_LINE)
{
	unsigned int x_end,y_end;

	x_end=x_start+PART_LINE-1; 
	y_end=y_start+PART_COLUMN-1;

    sendCommand(0x50);  
    uint8_t data[] = {0xA9, 0x07};
    sendData(data, 2);

    sendCommand(0x91);    //enter partial mode
    sendCommand(0x90); //resolution setting
    //x-start, end, y-start, end
    uint8_t res_data[] = {x_start/256, x_start%256, x_end/256, x_end%256-1, y_start/256, y_start%256, y_end/256, y_end%256-1, 0x01};
    sendData(res_data, 9);

    sendCommand(0x13); //write new data
    for(uint32_t i = 0; i < EPD_ARRAY; i++)
    {
        sendData(datas[i], 1);
    }
    EPD_update();

}

//full screen partial update display
void EPD_Display_Partial_All(const unsigned char *datas)
{
    unsigned int x_start=0,y_start=0,x_end,y_end;
	unsigned int PART_COLUMN=EPD_HEIGHT,PART_LINE=EPD_WIDTH;

	x_end=x_start+PART_LINE-1; 
	y_end=y_start+PART_COLUMN-1;

    sendCommand(0x50);
    uint8_t data[] = {0xA9, 0x07};
    sendData(data, 2);

    sendCommand(0x91);    //enter partial mode
    sendCommand(0x90); //resolution setting
    //x-start, end, y-start, end
    uint8_t res_data[] = {x_start/256, x_start%256, x_end/256, x_end%256-1, y_start/256, y_start%256, y_end/256, y_end%256-1, 0x01};
    sendData(res_data, 9);

    sendCommand(0x13); //write new data
    for(uint32_t i = 0; i<PART_COLUMN*PART_LINE/8; i++)
    {
        sendData(datas[i], 1);
    }
    EPD_update();

}


void EPD_DeepSleep(void)
{
    sendCommand(0x50);
    uint8_t data = 0xf7;
    sendData(&data, 1);

    sendCommand(0x02); // power off
    lcd_chkstatus();
    vTaskDelay(100);
    sendCommand(0x07); // deep sleep
    data = 0xA5;
    sendData(&data, 1);
}
//TODO
// error handling
    //SPI ERROR - GIVE UP ON TRANSACTION
    // TIMEOUT - DOESNT MATTER ? WAIT MAX TIMEOUT 
// MAKE SPI BETTER 

