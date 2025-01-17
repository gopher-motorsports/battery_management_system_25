#ifndef _EPAPER_H_
#define _EPAPER_H_

#include <stdint.h>

#define EPD_WIDTH   800
#define EPD_HEIGHT  480
#define EPD_ARRAY  EPD_WIDTH*EPD_HEIGHT/8  

void EPD_Init(void);
void EPD_Update(void);
void EPD_WhiteScreen_All(uint8_t *data);
void EPD_WhiteScreen(void);
void EPD_BlackScreen(void);
void EPD_SetRamValue_BaseMap(uint8_t *data);
void EPD_Display_Partial(uint8_t x_start,uint8_t y_start,uint8_t * datas,uint8_t PART_COLUMN,uint8_t PART_LINE);
void EPD_Display_Partial_All(uint8_t *datas);
void EPD_DeepSleep(void);
void EPD_Display_Partial_RAM(uint8_t x_start,uint8_t y_start,
	                      uint8_t * datas_A,uint8_t * datas_B,
												uint8_t * datas_C,uint8_t * datas_D,uint8_t * datas_E,
                        uint8_t num,uint8_t PART_COLUMN,uint8_t PART_LINE);

void EPD_Display_Partial_Time(uint8_t x_start,uint8_t y_start,
                          uint8_t * datas_A,uint8_t * datas_B,
                                                uint8_t * datas_C,uint8_t * datas_D,uint8_t * datas_E,
                        uint8_t num,uint8_t PART_COLUMN,uint8_t PART_LINE);

void EPD_Display(unsigned char *Image);

#endif  


