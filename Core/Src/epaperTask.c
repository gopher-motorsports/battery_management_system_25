/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */
#include "epaperTask.h"
#include "epaper.h"
#include "string.h"

#include "GUI_Paint.h"
#include "fonts.h"



unsigned char BlackImage[EPD_ARRAY];//Define canvas space  

/* ==================================================================== */
/* ============================== STRUCTS ============================= */
/* ==================================================================== */

/* ==================================================================== */
/* =================== LOCAL FUNCTION DECLARATIONS ==================== */
/* ==================================================================== */


/* ==================================================================== */
/* =================== GLOBAL FUNCTION DEFINITIONS ==================== */
/* ==================================================================== */

void initEpaperTask()
{
    
}

void runEpaperTask()
{
    // EPD_init(); // Full screen update init
    // EPD_WhiteScreen();
    // EPD_DeepSleep(); // enter sleep mode
    // vTaskDelay(2000);
    

    // EPD_init(); // Full screen update init
    // EPD_BlackScreen();
    // EPD_DeepSleep(); // enter sleep mode
    // vTaskDelay(2000);

    Paint_NewImage(BlackImage, EPD_WIDTH, EPD_HEIGHT, 0, WHITE); //Set canvas parameters, GUI image rotation, please change 0 to 0/90/180/270.
    Paint_SelectImage(BlackImage); //Select current settings.
    EPD_Init(); //Full screen update initialization.
    Paint_Clear(WHITE); //Clear canvas.
    Paint_DrawPoint(5, 10, BLACK, DOT_PIXEL_1X1, DOT_STYLE_DFT); //point 1x1.
    Paint_DrawPoint(5, 25, BLACK, DOT_PIXEL_2X2, DOT_STYLE_DFT); //point 2x2.
    Paint_DrawPoint(5, 40, BLACK, DOT_PIXEL_3X3, DOT_STYLE_DFT); //point 3x3.
    Paint_DrawPoint(5, 55, BLACK, DOT_PIXEL_4X4, DOT_STYLE_DFT); //point 4x4.
    Paint_DrawLine(20, 10, 70, 60, BLACK, LINE_STYLE_SOLID, DOT_PIXEL_1X1); //1x1line 1.
    Paint_DrawLine(70, 10, 20, 60, BLACK, LINE_STYLE_SOLID, DOT_PIXEL_1X1); //1x1line 2.
    Paint_DrawRectangle(20, 10, 70, 60, BLACK, DRAW_FILL_EMPTY, DOT_PIXEL_1X1); //Hollow rectangle 1.
    Paint_DrawRectangle(85, 10, 130, 60, BLACK, DRAW_FILL_FULL, DOT_PIXEL_1X1); //Hollow rectangle 2.
    Paint_DrawCircle(150, 90, 30, BLACK, DRAW_FILL_EMPTY, DOT_PIXEL_1X1); //Hollow circle.
    Paint_DrawCircle(200, 90, 30, BLACK, DRAW_FILL_FULL, DOT_PIXEL_1X1); //solid circle.
    EPD_Display(BlackImage); //Display GUI image.
    EPD_DeepSleep();//EPD_DeepSleep,Sleep instruction is necessary, please do not delete!!!
    vTaskDelay(2000);

    // EPD_Init(); //Full screen update initialization.
    // Paint_Clear(WHITE); //Clear canvas.
    // Paint_DrawString_EN(0, 0, "Good Display", &Font8, WHITE, BLACK);  //5*8.
    // Paint_DrawString_EN(0, 10, "Good Display", &Font12, WHITE, BLACK); //7*12.
    // Paint_DrawString_EN(0, 25, "Good Display", &Font16, WHITE, BLACK); //11*16.
    // Paint_DrawString_EN(0, 45, "Good Display", &Font20, WHITE, BLACK); //14*20.
    // Paint_DrawString_EN(0, 80, "Good Display", &Font24, WHITE, BLACK); //17*24.
    // EPD_Display(BlackImage);//Display GUI image.
    // vTaskDelay(2000);
			



}

