#ifndef _EPAPER_H_
#define _EPAPER_H_

#include <stdint.h>

// #define EPD_WIDTH   800 
// #define EPD_HEIGHT  480
// #define EPD_ARRAY  EPD_WIDTH*EPD_HEIGHT/8  

void sendCommand(uint8_t command);
void sendData(uint8_t *data, uint32_t size);

// void SPI_Write(unsigned char value);
// void EPD_W21_WriteDATA(unsigned char datas);
// void EPD_W21_WriteCMD(unsigned char command);
// void EPD_GPIO_Init(void);


// //Full screen update display
// void EPD_Init(void); 
// void EPD_Init_180(void);	
// void EPD_WhiteScreen_ALL(const unsigned char *datas);
// void EPD_WhiteScreen_White(void);
// void EPD_WhiteScreen_Black(void);
// void EPD_DeepSleep(void);
// //Partial update display 
// void EPD_Init_Part(void);
// void EPD_SetRAMValue_BaseMap(const unsigned char * datas);
// void EPD_WhiteScreen_White_Basemap(void);
// void EPD_Dis_PartAll(const unsigned char * datas);
// void EPD_Dis_Part(unsigned int x_start,unsigned int y_start,const unsigned char * datas,unsigned int PART_COLUMN,unsigned int PART_LINE);
// void EPD_Dis_Part_Time(unsigned int x_start,unsigned int y_start,
// 	                      const unsigned char * datas_A,const unsigned char * datas_B,
// 												const unsigned char * datas_C,const unsigned char * datas_D,const unsigned char * datas_E,
//                         unsigned char num,unsigned int PART_COLUMN,unsigned int PART_LINE);										 
// //Fast update display
// void EPD_Init_Fast(void);
// void EPD_WhiteScreen_ALL_Fast(const unsigned char *datas);   
// //4 Gray
// void EPD_Init_4G(void);
// void EPD_WhiteScreen_ALL_4G(const unsigned char *datas);		
// void Display_4Level_Gray(void);
// //GUI display					 
// void EPD_Display(unsigned char *Image); 			
#endif  //#ifndef _MCU_SPI_H_


