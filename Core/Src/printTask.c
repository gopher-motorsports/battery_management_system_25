/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include "printTask.h"
#include "main.h"
#include "telemetryTask.h"
#include "statusUpdateTask.h"
#include "cmsis_os.h"
#include <stdio.h>
#include "GopherCAN.h"
#include "alerts.h"

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
static void printCellVoltages(telemetryTaskData_S* telemetryData);
static void printCellTemps(telemetryTaskData_S* telemetryData);
static void printCellStats(telemetryTaskData_S* telemetryData);
static void printCellMonDiag(Cell_Monitor_S* bmb);
static bool printActiveAlerts(Alert_S** alerts, uint16_t num_alerts);


static void printEnergyData(Pack_Monitor_S* packMon);
static void printPackMonDiag(Pack_Monitor_S* packMon);

static void printImdData(imdData_S* imdData);

static void printCharger(CHARGER_STATE_E chargerState);

/* ==================================================================== */
/* =================== LOCAL FUNCTION DEFINITIONS ===================== */
/* ==================================================================== */

static void printCellVoltages(telemetryTaskData_S* telemetryData)
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
            if((telemetryData->bmb[j].cellVoltageStatus[i] == GOOD) && (telemetryData->bmbStatus[j] == GOOD))
            {
                if((telemetryData->bmb[j].cellVoltage[i] < 0.0f) || telemetryData->bmb[j].cellVoltage[i] >= 100.0f)
                {
                    printf("  %5.3f   |", telemetryData->bmb[j].cellVoltage[i]);
                }
                else
                {
                    printf("   %5.3f   |", telemetryData->bmb[j].cellVoltage[i]);
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

static void printCellTemps(telemetryTaskData_S* telemetryData)
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
            if((telemetryData->bmb[j].cellTempStatus[i] == GOOD) && (telemetryData->bmbStatus[j] == GOOD))
            {
                if((telemetryData->bmb[j].cellTemp[i] < 0.0f) || telemetryData->bmb[j].cellTemp[i] >= 100.0f)
                {
                    printf("   %3.1f   |", (double)telemetryData->bmb[j].cellTemp[i]);
                }
                else
                {
                    printf("    %3.1f   |", (double)telemetryData->bmb[j].cellTemp[i]);
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
        if((telemetryData->bmb[j].boardTempStatus == GOOD) && (telemetryData->bmbStatus[j] == GOOD))
        {
            if((telemetryData->bmb[j].boardTemp < 0.0f) || telemetryData->bmb[j].boardTemp >= 100.0f)
            {
                printf("   %3.1f   |", (double)telemetryData->bmb[j].boardTemp);
            }
            else
            {
                printf("    %3.1f   |", (double)telemetryData->bmb[j].boardTemp);
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

static bool printActiveAlerts(Alert_S** alerts, uint16_t num_alerts)
{
    bool alertActive = false;

    for (uint16_t i = 0; i < num_alerts; i++) {
        if (alerts[i]->alertStatus == ALERT_SET) {
            printf("ALERT: %s\n", alerts[i]->alertName);
            alertActive = true;
        } else if (alerts[i]->alertStatus == ALERT_LATCHED) {
            printf("ALERT: %s LATCHED\n", alerts[i]->alertName);
            alertActive = true;
        }
    }

    return alertActive;
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

static void printCharger(CHARGER_STATE_E chargerState)
{
    printf("\n");
    printf("Charger state: ");
    switch(chargerState)
    {
        case CHARGER_STATE_DISCONNECTED:
            printf("DISCONNETED\n");
            break;
        case CHARGER_STATE_CONSTANT_CURRENT:
            printf("CONSTANT CURRENT\n");
            break;
        case CHARGER_STATE_CONSTANT_VOLTAGE:
            printf("CONSTANT VOLTAGE\n");
            break;
        case CHARGER_STATE_BALANCING:
            printf("BALANCING\n");
            break;
        case CHARGER_STATE_COMPLETE:
            printf("CHARGE COMPLETE\n");
            break;

        default:
            break;
    }

    printf("Charger Voltage %f\n", chargerVoltageSetPoint_V.data);
    printf("Charger Current %f\n", chargerCurrentSetPoint_A);

    uint8_t chargerStatus = chargerStatusByte.data;
    if(chargerStatus & 0x01)
    {
        printf("Charger hardware Failure\n");
    }
    if(chargerStatus & 0x02)
    {
        printf("Charger Over Temp\n");
    }
    if(chargerStatus & 0x04)
    {
        printf("Charger Wrong Input Voltage\n");
    }
    if(chargerStatus & 0x08)
    {
        printf("Charger No Battery\n");
    }
    if(chargerStatus & 0x10)
    {
        printf("Charger No Comms\n");
    }
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
    CHARGER_STATE_E chargerStateLocal = chargerState;
    xTaskResumeAll();

    // Clear terminal output
    printf("\e[1;1H\e[2J");

    // printTestData(printTaskInputData.telemetryTaskData.bmb);
    printCellVoltages(&printTaskInputData.telemetryTaskData);
    printCellTemps(&printTaskInputData.telemetryTaskData);
    // printf("IADC1: %f\n", printTaskInputData.telemetryTaskData.IADC1);
    // printf("IADC2: %f\n", printTaskInputData.telemetryTaskData.IADC2);
    // printf("VBADC1: %f\n", printTaskInputData.telemetryTaskData.VBADC1);
    // printf("VBADC2: %f\n", printTaskInputData.telemetryTaskData.VBADC2);

    // printf("\n");
    // printf("Max Cell Voltage: %f\n", printTaskInputData.telemetryTaskData.maxCellVoltage);
    // printf("Min Cell Voltage: %f\n", printTaskInputData.telemetryTaskData.minCellVoltage);
    // printf("Max Cell Temp: %f\n", printTaskInputData.telemetryTaskData.maxCellTemp);
    // printf("Min Cell Temp: %f\n", printTaskInputData.telemetryTaskData.minCellTemp);

    // printf("\n");
    // printf("SOC by OCV: %f\n", printTaskInputData.telemetryTaskData.socData.socByOcv * 100.0f);
    // printf("SOE by OCV: %f\n", printTaskInputData.telemetryTaskData.socData.soeByOcv * 100.0f);
    // printf("\n");
    // printf("Millicoulomb Counter: %lu\n", printTaskInputData.telemetryTaskData.socData.milliCoulombCounter);
    // printf("SOC by CC: %f\n", printTaskInputData.telemetryTaskData.socData.socByCoulombCounting * 100.0f);
    // printf("SOE by CC: %f\n", printTaskInputData.telemetryTaskData.socData.soeByCoulombCounting * 100.0f);

    // printf("\n");
    // printf("Conversion Phase Counter: %lu\n", printTaskInputData.telemetryTaskData.packMonitorData.localPhaseCountTimer.timCount * 4);
    // printf("ICNTPHA: %lu\n", printTaskInputData.telemetryTaskData.packMonitorData.adcConversionPhaseCounter);
    // printf("IADC Conversion Time: %f\n", printTaskInputData.telemetryTaskData.packMonitorData.adcConversionTimeMS);

    // printf("\n");
    // printf("Diagnostic state: ");
    // switch(printTaskInputData.telemetryTaskData.curentDiagnosticState)
    // {
    //     case REDUNDANT_ADC_DIAG_STATE:
    //         printf("REDUNDANT\n");
    //         break;
    //     case BALANCING_DIAG_STATE:
    //         printf("BALANCING\n");
    //         break;
    //     case OPEN_WIRE_EVEN_DIAG_STATE:
    //         printf("OPEN WIRE EVEN\n");
    //         break;
    //     case OPEN_WIRE_ODD_DIAG_STATE:
    //         printf("OPEN WIRE ODD\n");
    //         break;

    //     default:
    //         break;
    // }

    printf("\n");
    
    if(!printActiveAlerts(telemetryAlerts, NUM_TELEMETRY_ALERTS)
    && !printActiveAlerts(statusAlerts, NUM_STATUS_ALERTS)) {
        printf("NO ALERTS ACTIVE\n");
    }

    printf("\n");

    printEnergyData(&printTaskInputData.telemetryTaskData.packMonitor);

    // printImdData(&printTaskInputData.statusUpdateTaskData.imdData);
    for(uint32_t i = 0; i < NUM_SDC_SENSE_INPUTS; i++)
    {
        printf("SDC%lu: %d\n", i, printTaskInputData.statusUpdateTaskData.shutdownCircuitData.sdcSenseFaultActive[i]);
    }

    printf("Power Limit: %  f\n", chargingPowerLimit.data);

    printCharger(chargerStateLocal);

    // printf("SOE: %f\n", soeByOCV_percent.data);

}
