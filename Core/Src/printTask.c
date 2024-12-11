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
static void printCellVoltages(Bmb_S* bmb);
static void printCellTemps(Bmb_S* bmb);

/* ==================================================================== */
/* =================== LOCAL FUNCTION DEFINITIONS ===================== */
/* ==================================================================== */

static void printCellVoltages(Bmb_S* bmb)
{
    printf("Cell Voltage:\n");
    printf("|   CELL   |");
    for(int32_t i = 0; i < NUM_BMBS_IN_ACCUMULATOR; i++)
    {
        printf("    %02ld     |", i);
    }
    printf("\n");
    for(int32_t i = 0; i < NUM_CELLS_PER_BMB; i++)
    {
        printf("|    %02ld    |", i+1);
        for(int32_t j = 0; j < NUM_BMBS_IN_ACCUMULATOR; j++)
        {
            if(bmb[j].cellVoltageStatus[i] == GOOD)
            {
                if((bmb[j].cellVoltage[i] < 0.0f) || bmb[j].cellVoltage[i] >= 100.0f)
                {
                    printf("  %5.3f   |", bmb[j].cellVoltage[i]);
                }
                else
                {
                    printf("   %5.3f   |", bmb[j].cellVoltage[i]);
                }
            }
            else
            {
                printf(" NO SIGNAL |");
            }
        }
        printf("\n");
    }
	printf("\n");
}

static void printCellTemps(Bmb_S* bmb)
{
    printf("Cell Temp:\n");
    printf("|   BMB   |");
    for(int32_t i = 0; i < NUM_BMBS_IN_ACCUMULATOR; i++)
    {
        printf("     %02ld    |", i);
    }
    printf("\n");
    for(int32_t i = 0; i < NUM_CELLS_PER_BMB; i++)
    {
        printf("|    %02ld   |", i+1);
        for(int32_t j = 0; j < NUM_BMBS_IN_ACCUMULATOR; j++)
        {
            if(bmb[j].cellTempStatus[i] == GOOD)
            {
                if((bmb[j].cellTemp[i] < 0.0f) || bmb[j].cellTemp[i] >= 100.0f)
                {
                    printf("   %3.1f   |", (double)bmb[j].cellTemp[i]);
                }
                else
                {
                    printf("    %3.1f   |", (double)bmb[j].cellTemp[i]);
                }
            }
            else
            {
                printf(" NO SIGNAL |");
            }
        }
        printf("\n");
    }
    printf("|  Board  |");
    for(int32_t j = 0; j < NUM_BMBS_IN_ACCUMULATOR; j++)
    {
        if(bmb[j].boardTempStatus == GOOD)
        {
            if((bmb[j].boardTemp < 0.0f) || bmb[j].boardTemp >= 100.0f)
            {
                printf("   %3.1f   |", (double)bmb[j].boardTemp);
            }
            else
            {
                printf("    %3.1f   |", (double)bmb[j].boardTemp);
            }
            // printf("  %04X", gBms.bmb[j].cellVoltage[i]);
        }
        else
        {
            printf(" NO SIGNAL |");
        }
    }
	printf("\n");
    // printf("|   Die   |");
    // for(int32_t j = 0; j < NUM_BMBS_IN_ACCUMULATOR; j++)
    // {
    //     if(bmb[j].dieTempStatus == GOOD)
    //     {
    //         if((bmb[j].dieTemp < 0.0f) || bmb[j].dieTemp >= 100.0f)
    //         {
    //             printf("   %3.1f   |", (double)bmb[j].dieTemp);
    //         }
    //         else
    //         {
    //             printf("    %3.1f   |", (double)bmb[j].dieTemp);
    //         }
    //         // printf("  %04X", gBms.bmb[j].cellVoltage[i]);
    //     }
    //     else
    //     {
    //         printf(" NO SIGNAL |");
    //     }
    // }
	// printf("\n");
}

// static void printTestData(Bmb_S* bmb)
// {
//     // Print header
//     printf("| BMB |");
//     for(int32_t i = 0; i < REGISTER_SIZE_BYTES; i++)
//     {
//         printf(" REG[%01d] |", i);
//     }
//     printf(" STATUS |\n");

//     for(int32_t i = 0; i < NUM_DEVICES_IN_ACCUMULATOR; i++)
//     {
//         printf("| %03d |", i);
//         for(int32_t j = 0; j < REGISTER_SIZE_BYTES; j++)
//         {
//             printf("   %02X   |", bmb[i].testData[j]);
//         }
//         printf("   %02lu   |\n", (uint32_t)bmb[i].testStatus);
//     }
//     printf("\n");
// }

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
    printf("\e[1;1H\e[2J");

    // printTestData(printTaskInputData.telemetryTaskData.bmb);
    printCellVoltages(printTaskInputData.telemetryTaskData.bmb);
    printCellTemps(printTaskInputData.telemetryTaskData.bmb);
    printf("IADC1: %f\n", printTaskInputData.telemetryTaskData.IADC1);
    printf("IADC2: %f\n", printTaskInputData.telemetryTaskData.IADC2);
    printf("VBADC1: %f\n", printTaskInputData.telemetryTaskData.VBADC1);
    printf("VBADC2: %f\n", printTaskInputData.telemetryTaskData.VBADC2);

}
