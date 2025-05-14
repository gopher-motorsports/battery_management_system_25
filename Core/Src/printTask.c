/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include "printTask.h"
#include "main.h"
#include "telemetryTask.h"
#include "statusUpdateTask.h"
#include "cmsis_os.h"
#include <stdio.h>

/* ==================================================================== */
/* ============================== STRUCTS ============================= */
/* ==================================================================== */

typedef struct
{
    telemetryTaskData_S telemetryTaskData;
    statusUpdateTaskData_S statusUpdateTaskData;
} PrintTaskInputData_S;

extern TIM_HandleTypeDef htim5;

/* ==================================================================== */
/* =================== LOCAL FUNCTION DECLARATIONS ==================== */
/* ==================================================================== */

// static void printTestData(Cell_Monitor_S* bmb);
static void printCellVoltages(Cell_Monitor_S* bmb);
static void printCellTemps(Cell_Monitor_S* bmb);
static void printCellStats(telemetryTaskData_S* telemetryData);
static void printCellMonDiag(Cell_Monitor_S* bmb);

static void printEnergyData(Pack_Monitor_S* packMon);
static void printPackMonDiag(Pack_Monitor_S* packMon);

static void printImdData(imdData_S* imdData);

/* ==================================================================== */
/* =================== LOCAL FUNCTION DEFINITIONS ===================== */
/* ==================================================================== */

static void printCellVoltages(Cell_Monitor_S* bmb)
{
    printf("Cell Voltage:\n");
    printf("|   CELL   |");
    for(int32_t i = 0; i < NUM_CELL_MON_IN_ACCUMULATOR; i++)
    {
        printf("    %02ld     |", i);
    }
    printf("\n");
    for(int32_t i = 0; i < NUM_CELLS_PER_CELL_MONITOR; i++)
    {
        printf("|    %02ld    |", i+1);
        for(int32_t j = 0; j < NUM_CELL_MON_IN_ACCUMULATOR; j++)
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

static void printCellTemps(Cell_Monitor_S* bmb)
{
    printf("Cell Temp:\n");
    printf("|   BMB    |");
    for(int32_t i = 0; i < NUM_CELL_MON_IN_ACCUMULATOR; i++)
    {
        printf("     %02ld    |", i);
    }
    printf("\n");
    for(int32_t i = 0; i < NUM_CELLS_PER_CELL_MONITOR; i++)
    {
        printf("|    %02ld    |", i+1);
        for(int32_t j = 0; j < NUM_CELL_MON_IN_ACCUMULATOR; j++)
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
    printf("|  Board   |");
    for(int32_t j = 0; j < NUM_CELL_MON_IN_ACCUMULATOR; j++)
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
    // for(int32_t j = 0; j < NUM_CELL_MON_IN_ACCUMULATOR; j++)
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

static void printCellStats(telemetryTaskData_S* telemetryData)
{
    // printf("")

}

static void printCellMonDiag(Cell_Monitor_S* bmb)
{

}

static void printEnergyData(Pack_Monitor_S* packMon)
{
    printf("Pack current: %f\n", packMon->packCurrent);
    printf("Pack voltage: %f\n", packMon->packVoltage);
    printf("Pack Power: %f\n", packMon->packPower);

    printf("Pack Mon Board Temp: %f\n", packMon->boardTemp);
    printf("Shunt Temp 1: %f\n", packMon->shuntTemp1);
    printf("Shunt Temp 2: %f\n", packMon->shuntTemp2);
    printf("Shunt Resistance uOhm: %f\n", packMon->shuntResistanceMicroOhms);
    printf("Pre Temp 1: %f\n", packMon->prechargeTemp);
    printf("Dis Temp 2: %f\n", packMon->dischargeTemp);
    printf("Link Voltage %f\n\n", packMon->linkVoltage);

    printf("SOC by OCV: %3.1f\n", packMon->socData.socByOcv * 100.0f);
    printf("SOE by OCV: %3.1f\n\n", packMon->socData.soeByOcv * 100.0f);

    printf("Millicoulomb Counter: %lu\n", packMon->socData.milliCoulombCounter);
    printf("Pack Millicoulombs: %lu\n", packMon->socData.packMilliCoulombs);
    printf("SOC by CC: %3.1f\n", packMon->socData.socByCoulombCounting * 100.0f);
    printf("SOE by CC: %3.1f\n\n", packMon->socData.soeByCoulombCounting * 100.0f);

    printf("Conversion Phase Counter: %lu\n", packMon->adcConversionPhaseCounter / 4);
    printf("Next Valid count: %lu\n", packMon->nextQualifiedPhaseCount / 4);
    printf("ADC Conversion time ms: %f\n", packMon->adcConversionTimeMS);
    printf("Local timer: %lu\n", __HAL_TIM_GetCounter(&htim5));

}

static void printPackMonDiag(Pack_Monitor_S* packMon)
{

}


// static void printTestData(Cell_Monitor_S* bmb)
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

static void printImdData(imdData_S* imdData)
{
    printf("IMD Status: ");
    switch(imdData->imdStatus)
    {
    case IMD_NO_SIGNAL:
        printf("NO SIGNAL\n");
        break;  
    case IMD_NORMAL:
        printf("NORMAL\n");
        break;
    case IMD_UNDER_VOLT:
        printf("UNDER VOLTAGE\n");
        break;
    case IMD_SPEED_START_MEASUREMENT:
        printf("SPEED START\n");
        break;
    case IMD_DEVICE_ERROR:
        printf("DEVICE ERROR\n");
        break;
    case IMD_CHASSIS_FAULT:
        printf("CHASSIS FAULT\n");
        break;
    default:
        printf("UKNOWN ERROR\n");
        break;
    }

    printf("Speed start status: ");
    if(imdData->speedStartSuccess)
    {
        printf("Success in %5.3fs\n", ((float)(imdData->speedStartTime) / 1000.0f));
    }
    else
    {
        printf("Fail\n");
    }

    printf("Isolation Resistance (Kohm): %lu\n\n", imdData->isolationResistance);
}

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DEFINITIONS ==================== */
/* ==================================================================== */

void initPrintTask()
{
    printf("\e[1;1H\e[2J");
}

void runPrintTask()
{
    PrintTaskInputData_S printTaskInputData;
    vTaskSuspendAll();
    printTaskInputData.telemetryTaskData = telemetryTaskData;
    printTaskInputData.statusUpdateTaskData = statusUpdateTaskData;
    xTaskResumeAll();

    // Clear terminal output
    printf("\e[1;1H\e[2J");

    // printTestData(printTaskInputData.telemetryTaskData.bmb);
    // printCellVoltages(printTaskInputData.telemetryTaskData.bmb);
    // printCellTemps(printTaskInputData.telemetryTaskData.bmb);

    // Print cell stats
    // printCellStats(printTaskInputData.telemetryTaskData.bmb);

    // Print all pack mon
    printEnergyData(&printTaskInputData.telemetryTaskData.packMonitor);

    // printImdData(&printTaskInputData.statusUpdateTaskData.imdData);

}
