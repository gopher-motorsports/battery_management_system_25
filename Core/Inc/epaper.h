#ifndef _EPAPER_H_
#define _EPAPER_H_

#include <stdint.h>

#define EPD_WIDTH   800 
#define EPD_HEIGHT  480
#define EPD_ARRAY  EPD_WIDTH*EPD_HEIGHT/8  

void sendCommand(uint8_t command);
void sendData(uint8_t *data, uint32_t size);
void lcd_chkstatus(void);
void EPD_init(void);
void EPD_Update(void);
void EPD_WhiteScreen_All(const unsigned char *data);
void EPD_WhiteScreen(void);
void EPD_BlackScreen(void);
void EPD_SetRamValue_BaseMap(const unsigned char *data);
void EPD_Display_Partial(unsigned int x_start,unsigned int y_start,const unsigned char * datas,unsigned int PART_COLUMN,unsigned int PART_LINE);
void EPD_Display_Partial_All(const unsigned char *datas);
void EPD_DeepSleep(void);
void EPD_Display_Partial_RAM(unsigned int x_start,unsigned int y_start,
	                      const unsigned char * datas_A,const unsigned char * datas_B,
												const unsigned char * datas_C,const unsigned char * datas_D,const unsigned char * datas_E,
                        unsigned char num,unsigned int PART_COLUMN,unsigned int PART_LINE);

void EPD_Display_Partial_Time(unsigned int x_start,unsigned int y_start,
                          const unsigned char * datas_A,const unsigned char * datas_B,
                                                const unsigned char * datas_C,const unsigned char * datas_D,const unsigned char * datas_E,
                        uint8_t num,unsigned int PART_COLUMN,unsigned int PART_LINE);



#endif  


