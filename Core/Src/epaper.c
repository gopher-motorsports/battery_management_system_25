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

uint8_t dataBuffer[10];


/* ==================================================================== */
/* ======================= EXTERNAL VARIABLES ========================= */
/* =============================

======================================= */


extern SPI_HandleTypeDef hspi2;

/* ==================================================================== */
/* =================== LOCAL FUNCTION DECLARATIONS ==================== */
/* ==================================================================== */
static void openCS(void);
static void closeCS(void);
static void reset(void);
static void waitBusy(void);
static void sendCommand(uint8_t command);
static void sendData(uint8_t *data, uint32_t size);


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

static void sendCommand(uint8_t command)
{
    uint8_t txBuffer[1] = {command};

    HAL_GPIO_WritePin(EPAP_DC_GPIO_Port, EPAP_DC_Pin, GPIO_PIN_RESET);

    // SPIify
    openCS();
    taskNotifySPI(&hspi2, txBuffer, NULL, 1, SPI_TIMEOUT_MS);
    closeCS();
}

static void sendData(uint8_t *data, uint32_t size)
{

    HAL_GPIO_WritePin(EPAP_DC_GPIO_Port, EPAP_DC_Pin, GPIO_PIN_SET);

    // SPIify
    openCS();
    taskNotifySPI(&hspi2, data, NULL, size, SPI_TIMEOUT_MS);
    closeCS();
}


static void waitBusy(void)
{
    xTaskNotifyWait(0, 0, NULL, BUSY_TIMEOUT_MS);

}



/* ==================================================================== */
/* ======================== PUBLIC FUNCTIONS ========================== */
/* ==================================================================== */

//functions that update dislpay
//WAIT BUSY REPLACES THIS
// void lcd_chkstatus(void)
// {
//     while(HAL_GPIO_ReadPin(EPAP_BUSY_GPIO_Port, EPAP_BUSY_Pin) == GPIO_PIN_SET)
//     {
//         vTaskDelay(10);
//     }
// }

//Full screen update initialization
void EPD_Init(void)
{
    reset();
    
    sendCommand(0x01); // power setting
    dataBuffer[0] = 0x07;
    dataBuffer[1] = 0x07;
    dataBuffer[2] = 0x3f;
    dataBuffer[3] = 0x3f;
    sendData(dataBuffer, 4);

    sendCommand(0x06); // booster soft start
    dataBuffer[0] = 0x17;
    dataBuffer[1] = 0x17;
    dataBuffer[2] = 0x28;
    dataBuffer[3] = 0x17;
    sendData(dataBuffer, 4);

    sendCommand(0x04); // power on
    vTaskDelay(100);
    waitBusy();

    sendCommand(0x00); // panel setting
    dataBuffer[0] = 0x1f;
    sendData(dataBuffer, 1);

    sendCommand(0x61); // resolution setting
    dataBuffer[0] = 0x03;
    dataBuffer[1] = 0x20;
    dataBuffer[2] = 0x01;
    dataBuffer[3] = 0xE0;
    sendData(dataBuffer, 4);

    sendCommand(0x15); 
    dataBuffer[0] = 0x00;
    sendData(dataBuffer, 1);

    sendCommand(0x50); // VCOM and data interval setting
    dataBuffer[0] = 0x10;
    dataBuffer[1] = 0x07;
    sendData(dataBuffer, 2);

    sendCommand(0x60); 
    dataBuffer[0] = 0x22;
    sendData(dataBuffer, 1);
}



//Display update function
void EPD_Update(void)
{
    sendCommand(0x12); //display update
    vTaskDelay(1); //delay of at least 200uS
    waitBusy(); //wait for the electronic paper IC to release the idle signal
}

void EPD_WhiteScreen_All(uint8_t *data)
{
    sendCommand(0x10); //write old data
    for(uint32_t i = 0; i < EPD_ARRAY; i++)
    {
        dataBuffer[0] = 0x00;
        sendData(dataBuffer, 1);
    }

    sendCommand(0x13); //write new data
    for(uint32_t i = 0; i < EPD_ARRAY; i++)
    {
        sendData(&data[i], 1);
    }

    EPD_Update();
}

//clear screen display
void EPD_WhiteScreen(void)
{
    dataBuffer[0] = 0x00;
    sendCommand(0x10); //write old data
    for(uint32_t i = 0; i < EPD_ARRAY; i++)
    {
        sendData(dataBuffer, 1);
    }
    sendCommand(0x13); //write new data
    for(uint32_t i = 0; i < EPD_ARRAY; i++)
    {
        sendData(dataBuffer, 1);
    }
    EPD_Update();
}

//display all black
void EPD_BlackScreen(void)
{
    sendCommand(0x10); //write old data
    for(uint32_t i = 0; i < EPD_ARRAY; i++)
    {
        dataBuffer[0] = 0x00;
        sendData(dataBuffer, 1);
    }
    sendCommand(0x13); //write new data
    for(uint32_t i = 0; i < EPD_ARRAY; i++)
    {
        dataBuffer[0] = 0xff;
        sendData(dataBuffer, 1);
    }
    EPD_Update();
}

//partial update of background display
void EPD_SetRamValue_BaseMap(uint8_t *data)
{
    sendCommand(0x10); //write old data
    for(uint32_t i = 0; i < EPD_ARRAY; i++)
    {
        dataBuffer[0] = 0xff;
        sendData(dataBuffer, 1);
    }
    sendCommand(0x13); //write new data
    for(uint32_t i = 0; i < EPD_ARRAY; i++)
    {
        sendData(&data[i], 1);
    }
    EPD_Update();
}

//Partial update display
void EPD_Display_Partial(uint8_t x_start,uint8_t y_start,uint8_t *datas,uint8_t PART_COLUMN,uint8_t PART_LINE)
{
	uint8_t x_end,y_end;

	x_end=x_start+PART_LINE-1; 
	y_end=y_start+PART_COLUMN-1;

    sendCommand(0x50);  
    dataBuffer[0] = 0xA9;
    dataBuffer[1] = 0x07;
    sendData(dataBuffer, 2);

    sendCommand(0x91);    //enter partial mode
    sendCommand(0x90); //resolution setting
    //x-start, end, y-start, end
    dataBuffer[0] = x_start/256;
    dataBuffer[1] = x_start%256;
    dataBuffer[2] = x_end/256;
    dataBuffer[3] = x_end%256-1;
    dataBuffer[4] = y_start/256;
    dataBuffer[5] = y_start%256;
    dataBuffer[6] = y_end/256;
    dataBuffer[7] = y_end%256-1;
    dataBuffer[8] = 0x01;
    sendData(dataBuffer, 9);

    sendCommand(0x13); //write new data
    for(uint32_t i = 0; i < EPD_ARRAY; i++)
    {
        sendData(&datas[i], 1);
    }
    EPD_Update();

}

//full screen partial update display
void EPD_Display_Partial_All(uint8_t *datas)
{
    uint8_t x_start=0,y_start=0,x_end,y_end;
	uint8_t PART_COLUMN=EPD_HEIGHT,PART_LINE=EPD_WIDTH;

	x_end=x_start+PART_LINE-1; 
	y_end=y_start+PART_COLUMN-1;

    sendCommand(0x50);
    dataBuffer[0] = 0xA9;
    dataBuffer[1] = 0x07;
    sendData(dataBuffer, 2);

    sendCommand(0x91);    //enter partial mode
    sendCommand(0x90); //resolution setting
    //x-start, end, y-start, end
    dataBuffer[0] = x_start/256;
    dataBuffer[1] = x_start%256;
    dataBuffer[2] = x_end/256;
    dataBuffer[3] = x_end%256-1;
    dataBuffer[4] = y_start/256;
    dataBuffer[5] = y_start%256;
    dataBuffer[6] = y_end/256;
    dataBuffer[7] = y_end%256-1;
    dataBuffer[8] = 0x01;
    sendData(dataBuffer, 9);

    sendCommand(0x13); //write new data
    for(uint32_t i = 0; i<PART_COLUMN*PART_LINE/8; i++)
    {
        sendData(&datas[i], 1);
    }
    EPD_Update();

}


void EPD_DeepSleep(void)
{
    sendCommand(0x50);
    dataBuffer[0] = 0xf7;
    sendData(dataBuffer, 1);

    sendCommand(0x02); // power off
    waitBusy();
    vTaskDelay(100);
    sendCommand(0x07); // deep sleep
    dataBuffer[0] = 0xA5;
    sendData(dataBuffer, 1);
}



//Partial update write address and data
void EPD_Display_Partial_RAM(uint8_t x_start,uint8_t y_start,
	                      uint8_t * datas_A,uint8_t * datas_B,
												uint8_t * datas_C,uint8_t * datas_D,uint8_t * datas_E,
                        uint8_t num,uint8_t PART_COLUMN,uint8_t PART_LINE)
{
	uint8_t x_end,y_end;
	uint8_t *datas[] = {datas_A, datas_B, datas_C, datas_D, datas_E};

	x_end=x_start+PART_LINE*num-1; 
	y_end=y_start+PART_COLUMN-1;

    sendCommand(0x50);
    dataBuffer[0] = 0xA9;
    dataBuffer[1] = 0x07;
    sendData(dataBuffer, 2);

    sendCommand(0x91);    //enter partial mode
    sendCommand(0x90); //resolution setting
    //x-start, end, y-start, end
    dataBuffer[0] = x_start/256;
    dataBuffer[1] = x_start%256;
    dataBuffer[2] = x_end/256;
    dataBuffer[3] = x_end%256-1;
    dataBuffer[4] = y_start/256;
    dataBuffer[5] = y_start%256;
    dataBuffer[6] = y_end/256;
    dataBuffer[7] = y_end%256-1;
    dataBuffer[8] = 0x01;
    sendData(dataBuffer, 9);

    sendCommand(0x13); //write new data
    for(int i=0; i<PART_COLUMN; i++)	     
    {
        for (int k = 0; k < 5; k++) {
            for(int j=0; j<PART_LINE/8; j++)	     
                sendData(&datas[k][i*PART_LINE/8+j], 1);  
        }
    }	
}




//Clock display
void EPD_Dis_Part_Time(uint8_t x_start,uint8_t y_start,
	                      uint8_t * datas_A,uint8_t * datas_B,
												uint8_t * datas_C,uint8_t * datas_D,uint8_t * datas_E,
                        uint8_t num,uint8_t PART_COLUMN,uint8_t PART_LINE)
{
	EPD_Display_Partial_RAM(x_start,y_start,datas_A,datas_B,datas_C,datas_D,datas_E,num,PART_COLUMN,PART_LINE);
	EPD_Update();
	sendCommand(0x92);  	//This command makes the display exit partial mode and enter normal mode. 
 
}	


void EPD_Display(unsigned char *Image)
{
    sendCommand(0x10);
    for(uint32_t i = 0; i < EPD_ARRAY; i++)
    {
        dataBuffer[0] = 0x00;
        sendData(dataBuffer, 1);
    }

    sendCommand(0x13);
    for(uint32_t i = 0; i < EPD_ARRAY; i++)
    {
        sendData(&Image[i],1);
    }

    EPD_Update();			 
}




//TODO
// error handling
    //SPI ERROR - GIVE UP ON TRANSACTION
    // TIMEOUT - DOESNT MATTER ? WAIT MAX TIMEOUT 
// MAKE SPI BETTER

