/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include "printTask.h"
#include "main.h"
#include "telemetryTask.h"
#include "cmsis_os.h"
#include <stdio.h>

/* ==================================================================== */
/* ============================== STRUCTS ============================= */
/* ==================================================================== */

typedef struct
{
    TelemetryTaskOutputData_S telemetryTaskData;
} PrintTaskInputData_S;

/* ==================================================================== */
/* =================== LOCAL FUNCTION DECLARATIONS ==================== */
/* ==================================================================== */

static void printTestData(Bmb_S* bmb);

/* ==================================================================== */
/* =================== LOCAL FUNCTION DEFINITIONS ===================== */
/* ==================================================================== */

static void printTestData(Bmb_S* bmb)
{
    // Print header
    printf("| BMB |");
    for(int32_t i = 0; i < REGISTER_SIZE_BYTES; i++)
    {
        printf(" REG[%01d] |", i);
    }
    printf(" STATUS |\n");

    for(int32_t i = 0; i < NUM_BMBS_IN_ACCUMULATOR; i++)
    {
        printf("| %03d |", i);
        for(int32_t j = 0; j < REGISTER_SIZE_BYTES; j++)
        {
            printf("   %02X   |", bmb[i].testData[j]);
        }
        printf("   %02lu   |\n", (uint32_t)bmb[i].testStatus);
    }
    printf("\n");
}

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DEFINITIONS ==================== */
/* ==================================================================== */

void initPrintTask()
{

}

void runPrintTask()
{
    PrintTaskInputData_S printTaskInputData;
    taskENTER_CRITICAL();
    printTaskInputData.telemetryTaskData = telemetryTaskOutputData;
    taskEXIT_CRITICAL();

    // Clear terminal output
    // printf("\e[1;1H\e[2J");

    // printTestData(printTaskInputData.telemetryTaskData.bmb);

}