/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */
#include "alerts.h"
#include "cellData.h"
#include <math.h>
#include "main.h"

/* ==================================================================== */
/* =================== LOCAL FUNCTION DEFINITIONS ===================== */
/* ==================================================================== */

// Telemetry task

static bool overvoltageWarningPresent(telemetryTaskData_S* telemetryData)
{
    return (telemetryData->maxCellVoltage > MAX_BRICK_WARNING_VOLTAGE);
}

static bool overvoltageFaultPresent(telemetryTaskData_S* telemetryData)
{
    return (telemetryData->maxCellVoltage > MAX_BRICK_FAULT_VOLTAGE);
}

static bool undervoltageWarningPresent(telemetryTaskData_S* telemetryData)
{
    return (telemetryData->minCellVoltage < MIN_BRICK_WARNING_VOLTAGE);
}

static bool undervoltageFaultPresent(telemetryTaskData_S* telemetryData)
{
    return (telemetryData->minCellVoltage < MIN_BRICK_FAULT_VOLTAGE);
}

// Status update task

static bool imdSdcFaultPresent(statusUpdateTaskData_S* statusData)
{
    return statusData->shutdownCircuitData.imdLatchOpen;
}

static bool amsSdcFaultPresent(statusUpdateTaskData_S* statusData)
{
    return statusData->shutdownCircuitData.bmsLatchOpen;
}

static bool amsSdcInhibitActive(statusUpdateTaskData_S* statusData)
{
    return statusData->shutdownCircuitData.bmsInhibitActive;
}

// static bool cellImbalancePresent(BmbTaskOutputData_S* bmbData)
// {
//     const float maxCellImbalanceV = bmbData->maxCellVoltage - bmbData->minCellVoltage;

//     return (maxCellImbalanceV > MAX_CELL_IMBALANCE_V);
// }

// static bool overtemperatureWarningPresent(BmbTaskOutputData_S* bmbData)
// {
//     return (bmbData->maxCellTemp > MAX_BRICK_TEMP_WARNING_C);
// }

// static bool overtemperatureFaultPresent(BmbTaskOutputData_S* bmbData)
// {
//     return (bmbData->maxCellTemp > MAX_BRICK_TEMP_FAULT_C);
// }

// static bool amsSdcFaultPresent(BmbTaskOutputData_S* bmbData)
// {
//     return HAL_GPIO_ReadPin(AMS_FAULT_SDC_GPIO_Port, AMS_FAULT_SDC_Pin);
// }

// static bool bspdSdcFaultPresent(BmbTaskOutputData_S* bmbData)
// {
//     return HAL_GPIO_ReadPin(BSPD_FAULT_SDC_GPIO_Port, BSPD_FAULT_SDC_Pin);
    
// }

// static bool imdSdcFaultPresent(BmbTaskOutputData_S* bmbData)
// {
//     return HAL_GPIO_ReadPin(IMD_FAULT_SDC_GPIO_Port, IMD_FAULT_SDC_Pin);
// }

// static bool badVoltageSensorStatusPresent(BmbTaskOutputData_S* bmbData)
// {
//     for (uint32_t i = 0; i < NUM_BMBS_IN_ACCUMULATOR; i++)
//     {
//         Bmb_S* bmb = &bmbData->bmb[i];
//         if (bmb->numBadBrickV != 0)
//         {
//             return true;
//         }
//     }
//     return false;
// }

// static bool badBrickTempSensorStatusPresent(BmbTaskOutputData_S* bmbData)
// {
//     for (uint32_t i = 0; i < NUM_BMBS_IN_ACCUMULATOR; i++)
//     {
//         Bmb_S* bmb = &bmbData->bmb[i];
//         if (bmb->numBadBrickTemp != 0)
//         {
//             return true;
//         }
//     }
//     return false;
// }

// static bool badBoardTempSensorStatusPresent(BmbTaskOutputData_S* bmbData)
// {
//     for (uint32_t i = 0; i < NUM_BMBS_IN_ACCUMULATOR; i++)
//     {
//         Bmb_S* bmb = &bmbData->bmb[i];
//         if (bmb->boardTempStatus != GOOD)
//         {
//             return true;
//         }
//     }
//     return false;
// }

// static bool insufficientTempSensePresent(BmbTaskOutputData_S* bmbData)
// {
//     for (uint32_t i = 0; i < NUM_BMBS_IN_ACCUMULATOR; i++)
//     {
//         Bmb_S* bmb = &bmbData->bmb[i];
//         const uint32_t maxNumBadBrickTempAllowed = NUM_CELLS_PER_BMB * (100 - MIN_PERCENT_BRICK_TEMPS_MONITORED) / 100;
//         if (bmb->numBadBrickTemp > maxNumBadBrickTempAllowed)
//         {
//             return true;
//         }
//     }
//     return false;
// }

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DEFINITIONS ==================== */
/* ==================================================================== */

/*!
  @brief   Get the status of any given alert
  @param   alert - The alert data structure whose status to read
  @return  The current status of the alert
*/
AlertStatus_E getAlertStatus(Alert_S* alert)
{
    return alert->alertStatus;
}

/*!
  @brief   Run the alert monitor to update the status of the alert
  @param   bms - The BMS data structure
  @param   alert - The Alert data structure
*/
void runAlertMonitor(Alert_S* alert)
{
    if (alert->alertStatus == ALERT_CLEARED || alert->alertStatus == ALERT_LATCHED)
    {
        // Determine if we need to set the alert
        if (alert->alertConditionPresent)
        {
            // Increment alert timer by 10ms
            updateTimer(&alert->alertTimer);
        }
        else
        {
            // Reset alert timer
            clearTimer(&alert->alertTimer);
        }

        // Determine if alert was set or in the case that the timer threshold is 0 then check whether the alert condition is present
        if (checkTimerExpired(&alert->alertTimer) && (!(alert->alertTimer.timThreshold <= 0) || alert->alertConditionPresent))
        {
            // Timer expired - Set alert
            alert->alertStatus = ALERT_SET;
            // Load timer with alert clear time
            configureTimer(&alert->alertTimer, alert->clearTime_MS);
        }

    }
    else if (alert->alertStatus == ALERT_SET)
    {
        // Determine if we can clear the alert
        if (!alert->alertConditionPresent)
        {
            // Increment clear timer by 10ms
            updateTimer(&alert->alertTimer);
        }
        else
        {
            // Alert conditions detected. Reset clear timer
            clearTimer(&alert->alertTimer);
        }

        // Determine if alert was cleared or in the case that the timer threshold is 0 then check whether the alert condition is not present
        if (checkTimerExpired(&alert->alertTimer) && (!(alert->alertTimer.timThreshold <= 0) || !alert->alertConditionPresent))
        {
            // Timer expired indicating alert is no longer present. Either set status to latched or clear
            if (alert->latching)
            {
                // Latching alerts can't be cleared - set status to latched to indicate that conditions are no longer met
                alert->alertStatus = ALERT_LATCHED;
            }
            else
            {
                // If non latching alert, the alert can be cleared
                alert->alertStatus = ALERT_CLEARED;
            }
            // Load timer with alert set time
            configureTimer(&alert->alertTimer, alert->setTime_MS);
        }
    }
}

void setAmsFault(bool set)
{
	// AMS fault pin is active low so if set == true then pin should be low
	HAL_GPIO_WritePin(AMS_FAULT_N_GPIO_Port, AMS_FAULT_N_Pin, set ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

/* ==================================================================== */
/* ========================= GLOBAL VARIABLES ========================= */
/* ==================================================================== */

// Overvoltage Warning Alert
const AlertResponse_E overvoltageWarningAlertResponse[] = { DISABLE_CHARGING };
#define NUM_OVERVOLTAGE_WARNING_ALERT_RESPONSE sizeof(overvoltageWarningAlertResponse) / sizeof(AlertResponse_E)
Alert_S overvoltageWarningAlert =
{ 
    .alertName = "OvervoltageWarning",
    .alertStatus = ALERT_CLEARED, .alertTimer = (Timer_S){.timCount = 0, .lastUpdate = 0, .timThreshold = OVERVOLTAGE_WARNING_ALERT_SET_TIME_MS}, 
    .setTime_MS = OVERVOLTAGE_WARNING_ALERT_SET_TIME_MS, .clearTime_MS = OVERVOLTAGE_WARNING_ALERT_CLEAR_TIME_MS, 
    .alertConditionPresent = false, 
    .numAlertResponse = NUM_OVERVOLTAGE_WARNING_ALERT_RESPONSE, .alertResponse =  overvoltageWarningAlertResponse
};

// Undervoltage Warning Alert
const AlertResponse_E undervoltageWarningAlertResponse[] = { LIMP_MODE };
#define NUM_UNDERVOLTAGE_WARNING_ALERT_RESPONSE sizeof(undervoltageWarningAlertResponse) / sizeof(AlertResponse_E)
Alert_S undervoltageWarningAlert = 
{ 
    .alertName = "UndervoltageWarning",
    .alertStatus = ALERT_CLEARED, .alertTimer = (Timer_S){.timCount = 0, .lastUpdate = 0, .timThreshold = UNDERVOLTAGE_WARNING_ALERT_SET_TIME_MS}, 
    .setTime_MS = UNDERVOLTAGE_WARNING_ALERT_SET_TIME_MS, .clearTime_MS = UNDERVOLTAGE_WARNING_ALERT_CLEAR_TIME_MS, 
    .alertConditionPresent = false,
    .numAlertResponse = NUM_UNDERVOLTAGE_WARNING_ALERT_RESPONSE, .alertResponse = undervoltageWarningAlertResponse
};

// Overvoltage Fault Alert
const AlertResponse_E overvoltageFaultAlertResponse[] = { DISABLE_CHARGING, EMERGENCY_BLEED, AMS_FAULT};
#define NUM_OVERVOLTAGE_FAULT_ALERT_RESPONSE sizeof(overvoltageFaultAlertResponse) / sizeof(AlertResponse_E)
Alert_S overvoltageFaultAlert = 
{ 
    .alertName = "OvervoltageFault", .latching = true,
    .alertStatus = ALERT_CLEARED, .alertTimer = (Timer_S){.timCount = 0, .lastUpdate = 0, .timThreshold = OVERVOLTAGE_FAULT_ALERT_SET_TIME_MS}, 
    .setTime_MS = OVERVOLTAGE_FAULT_ALERT_SET_TIME_MS, .clearTime_MS = OVERVOLTAGE_FAULT_ALERT_CLEAR_TIME_MS, 
    .alertConditionPresent = false, 
    .numAlertResponse = NUM_OVERVOLTAGE_FAULT_ALERT_RESPONSE, .alertResponse =  overvoltageFaultAlertResponse
};

// Undervoltage Fault Alert
const AlertResponse_E undervoltageFaultAlertResponse[] = { AMS_FAULT };
#define NUM_UNDERVOLTAGE_FAULT_ALERT_RESPONSE sizeof(undervoltageFaultAlertResponse) / sizeof(AlertResponse_E)
Alert_S undervoltageFaultAlert = 
{ 
    .alertName = "UndervoltageFault", .latching = true,
    .alertStatus = ALERT_CLEARED, .alertTimer = (Timer_S){.timCount = 0, .lastUpdate = 0, .timThreshold = UNDERVOLTAGE_FAULT_ALERT_SET_TIME_MS}, 
    .setTime_MS = UNDERVOLTAGE_FAULT_ALERT_SET_TIME_MS, .clearTime_MS = UNDERVOLTAGE_FAULT_ALERT_CLEAR_TIME_MS, 
    .alertConditionPresent = false,
    .numAlertResponse = NUM_UNDERVOLTAGE_FAULT_ALERT_RESPONSE, .alertResponse = undervoltageFaultAlertResponse
};

// IMD Shut Down Circuit Alert
const AlertResponse_E imdSdcAlertResponse[] = { INFO_ONLY };
#define NUM_IMD_SDC_ALERT_RESPONSE sizeof(imdSdcAlertResponse) / sizeof(AlertResponse_E)
Alert_S imdSdcFaultAlert = 
{
    .alertName = "ImdSdcLatched",
    .alertStatus = ALERT_CLEARED, .alertTimer = (Timer_S){.timCount = 0, .lastUpdate = 0, .timThreshold = SDC_FAULT_ALERT_SET_TIME_MS}, 
    .setTime_MS = SDC_FAULT_ALERT_SET_TIME_MS, .clearTime_MS = SDC_FAULT_ALERT_CLEAR_TIME_MS, 
    .alertConditionPresent = false,
    .numAlertResponse = NUM_IMD_SDC_ALERT_RESPONSE, .alertResponse = imdSdcAlertResponse
};

// AMS Shut Down Circuit Alert
const AlertResponse_E amsSdcAlertResponse[] = { INFO_ONLY };
#define NUM_AMS_SDC_ALERT_RESPONSE sizeof(amsSdcAlertResponse) / sizeof(AlertResponse_E)
Alert_S amsSdcFaultAlert = 
{
    .alertName = "AmsSdcLatched",
    .alertStatus = ALERT_CLEARED, .alertTimer = (Timer_S){.timCount = 0, .lastUpdate = 0, .timThreshold = SDC_FAULT_ALERT_SET_TIME_MS}, 
    .setTime_MS = SDC_FAULT_ALERT_SET_TIME_MS, .clearTime_MS = SDC_FAULT_ALERT_CLEAR_TIME_MS, 
    .alertConditionPresent = false,
    .numAlertResponse = NUM_AMS_SDC_ALERT_RESPONSE, .alertResponse = amsSdcAlertResponse
};

// AMS Inhibit Shut Down Circuit Alert
const AlertResponse_E amsSdcInhibitResponse[] = { INFO_ONLY };
#define NUM_AMS_SDC_INHIBIT_RESPONSE sizeof(amsSdcInhibitResponse) / sizeof(AlertResponse_E)
Alert_S amsSdcInhibitAlert = 
{
    .alertName = "AmsInhibitActive",
    .alertStatus = ALERT_CLEARED, .alertTimer = (Timer_S){.timCount = 0, .lastUpdate = 0, .timThreshold = SDC_FAULT_ALERT_SET_TIME_MS}, 
    .setTime_MS = SDC_FAULT_ALERT_SET_TIME_MS, .clearTime_MS = SDC_FAULT_ALERT_CLEAR_TIME_MS, 
    .alertConditionPresent = false,
    .numAlertResponse = NUM_AMS_SDC_INHIBIT_RESPONSE, .alertResponse = amsSdcInhibitResponse
};

// // Cell Imbalance Alert
// const AlertResponse_E cellImbalanceAlertResponse[] = {LIMP_MODE, DISABLE_CHARGING, DISABLE_BALANCING };
// #define NUM_CELL_IMBALANCE_ALERT_RESPONSE sizeof(cellImbalanceAlertResponse) / sizeof(AlertResponse_E)
// Alert_S cellImbalanceAlert = 
// {
//     .alertName = "CellImbalance",
//     .alertStatus = ALERT_CLEARED, .alertTimer = (Timer_S){.timCount = 0, .lastUpdate = 0, .timThreshold = CELL_IMBALANCE_ALERT_SET_TIME_MS}, 
//     .setTime_MS = CELL_IMBALANCE_ALERT_SET_TIME_MS, .clearTime_MS = CELL_IMBALANCE_ALERT_CLEAR_TIME_MS, 
//     .alertConditionPresent = false,
//     .numAlertResponse = NUM_CELL_IMBALANCE_ALERT_RESPONSE, .alertResponse = cellImbalanceAlertResponse
// };

// // Overtemperature Warning Alert
// const AlertResponse_E overtempWarningAlertResponse[] = { LIMP_MODE, DISABLE_CHARGING, DISABLE_BALANCING };
// #define NUM_OVERTEMP_WARNING_ALERT_RESPONSE sizeof(overtempWarningAlertResponse) / sizeof(AlertResponse_E)
// Alert_S overtempWarningAlert = 
// {
//     .alertName = "OvertempWarning",
//     .alertStatus = ALERT_CLEARED, .alertTimer = (Timer_S){.timCount = 0, .lastUpdate = 0, .timThreshold = OVERTEMPERATURE_WARNING_ALERT_SET_TIME_MS}, 
//     .setTime_MS = OVERTEMPERATURE_WARNING_ALERT_SET_TIME_MS, .clearTime_MS = OVERTEMPERATURE_WARNING_ALERT_CLEAR_TIME_MS, 
//     .alertConditionPresent = false,
//     .numAlertResponse = NUM_OVERTEMP_WARNING_ALERT_RESPONSE, .alertResponse = overtempWarningAlertResponse
// };

// // Overtemperature Fault Alert
// const AlertResponse_E overtempFaultAlertResponse[] = { DISABLE_CHARGING, DISABLE_BALANCING, AMS_FAULT };
// #define NUM_OVERTEMP_FAULT_ALERT_RESPONSE sizeof(overtempFaultAlertResponse) / sizeof(AlertResponse_E)
// Alert_S overtempFaultAlert = 
// {
//     .alertName = "OvertempFault", .latching = true,
//     .alertStatus = ALERT_CLEARED, .alertTimer = (Timer_S){.timCount = 0, .lastUpdate = 0, .timThreshold = OVERTEMPERATURE_FAULT_ALERT_SET_TIME_MS}, 
//     .setTime_MS = OVERTEMPERATURE_FAULT_ALERT_SET_TIME_MS, .clearTime_MS = OVERTEMPERATURE_FAULT_ALERT_CLEAR_TIME_MS, 
//     .alertConditionPresent = false,
//     .numAlertResponse = NUM_OVERTEMP_FAULT_ALERT_RESPONSE, .alertResponse = overtempFaultAlertResponse
// };

// // AMS Shut Down Circuit Alert
// const AlertResponse_E amsSdcAlertResponse[] = { INFO_ONLY };
// #define NUM_AMS_SDC_ALERT_RESPONSE sizeof(amsSdcAlertResponse) / sizeof(AlertResponse_E)
// Alert_S amsSdcFaultAlert = 
// {
//     .alertName = "AmsSdcLatched",
//     .alertStatus = ALERT_CLEARED, .alertTimer = (Timer_S){.timCount = 0, .lastUpdate = 0, .timThreshold = SDC_FAULT_ALERT_SET_TIME_MS}, 
//     .setTime_MS = SDC_FAULT_ALERT_SET_TIME_MS, .clearTime_MS = SDC_FAULT_ALERT_CLEAR_TIME_MS, 
//     .alertConditionPresent = false,
//     .numAlertResponse = NUM_AMS_SDC_ALERT_RESPONSE, .alertResponse = amsSdcAlertResponse
// };

// // BSPD Shut Down Circuit Alert
// const AlertResponse_E bspdSdcAlertResponse[] = { INFO_ONLY };
// #define NUM_BSPD_SDC_ALERT_RESPONSE sizeof(bspdSdcAlertResponse) / sizeof(AlertResponse_E)
// Alert_S bspdSdcFaultAlert = 
// {
//     .alertName = "BspdSdcLatched",
//     .alertStatus = ALERT_CLEARED, .alertTimer = (Timer_S){.timCount = 0, .lastUpdate = 0, .timThreshold = SDC_FAULT_ALERT_SET_TIME_MS}, 
//     .setTime_MS = SDC_FAULT_ALERT_SET_TIME_MS, .clearTime_MS = SDC_FAULT_ALERT_CLEAR_TIME_MS, 
//     .alertConditionPresent = false,
//     .numAlertResponse = NUM_BSPD_SDC_ALERT_RESPONSE, .alertResponse = bspdSdcAlertResponse
// };

// // IMD Shut Down Circuit Alert
// const AlertResponse_E imdSdcAlertResponse[] = { INFO_ONLY };
// #define NUM_IMD_SDC_ALERT_RESPONSE sizeof(imdSdcAlertResponse) / sizeof(AlertResponse_E)
// Alert_S imdSdcFaultAlert = 
// {
//     .alertName = "ImdSdcLatched",
//     .alertStatus = ALERT_CLEARED, .alertTimer = (Timer_S){.timCount = 0, .lastUpdate = 0, .timThreshold = SDC_FAULT_ALERT_SET_TIME_MS}, 
//     .setTime_MS = SDC_FAULT_ALERT_SET_TIME_MS, .clearTime_MS = SDC_FAULT_ALERT_CLEAR_TIME_MS, 
//     .alertConditionPresent = false,
//     .numAlertResponse = NUM_IMD_SDC_ALERT_RESPONSE, .alertResponse = imdSdcAlertResponse
// };

// // Bad voltage sensor status
// const AlertResponse_E badVoltageSenseStatusAlertResponse[] = { DISABLE_BALANCING, DISABLE_CHARGING, AMS_FAULT };
// #define NUM_BAD_VOLTAGE_SENSE_STATUS_ALERT_RESPONSE sizeof(badVoltageSenseStatusAlertResponse) / sizeof(AlertResponse_E)
// Alert_S badVoltageSenseStatusAlert = 
// {
//     .alertName = "BadVoltageSenseStatus", .latching = true,
//     .alertStatus = ALERT_CLEARED, .alertTimer = (Timer_S){.timCount = 0, .lastUpdate = 0, .timThreshold = BAD_VOLTAGE_SENSE_STATUS_ALERT_SET_TIME_MS}, 
//     .setTime_MS = BAD_VOLTAGE_SENSE_STATUS_ALERT_SET_TIME_MS, .clearTime_MS = BAD_VOLTAGE_SENSE_STATUS_ALERT_CLEAR_TIME_MS, 
//     .alertConditionPresent = false,
//     .numAlertResponse = NUM_BAD_VOLTAGE_SENSE_STATUS_ALERT_RESPONSE, .alertResponse = badVoltageSenseStatusAlertResponse
// };

// // Bad brick temperature sensor status
// const AlertResponse_E badBrickTempSenseStatusAlertResponse[] = { INFO_ONLY };
// #define NUM_BAD_BRICK_TEMP_SENSE_STATUS_ALERT_RESPONSE sizeof(badBrickTempSenseStatusAlertResponse) / sizeof(AlertResponse_E)
// Alert_S badBrickTempSenseStatusAlert = 
// {
//     .alertName = "BadBrickTempSenseStatus",
//     .alertStatus = ALERT_CLEARED, .alertTimer = (Timer_S){.timCount = 0, .lastUpdate = 0, .timThreshold = BAD_BRICK_TEMP_SENSE_STATUS_ALERT_SET_TIME_MS}, 
//     .setTime_MS = BAD_BRICK_TEMP_SENSE_STATUS_ALERT_SET_TIME_MS, .clearTime_MS = BAD_BRICK_TEMP_SENSE_STATUS_ALERT_CLEAR_TIME_MS, 
//     .alertConditionPresent = false,
//     .numAlertResponse = NUM_BAD_BRICK_TEMP_SENSE_STATUS_ALERT_RESPONSE, .alertResponse = badBrickTempSenseStatusAlertResponse
// };

// // Bad board temperature sensor status
// const AlertResponse_E badBoardTempSenseStatusAlertResponse[] = { INFO_ONLY };
// #define NUM_BAD_BOARD_TEMP_SENSE_STATUS_ALERT_RESPONSE sizeof(badBoardTempSenseStatusAlertResponse) / sizeof(AlertResponse_E)
// Alert_S badBoardTempSenseStatusAlert = 
// {
//     .alertName = "BadBoardTempSenseStatus",
//     .alertStatus = ALERT_CLEARED, .alertTimer = (Timer_S){.timCount = 0, .lastUpdate = 0, .timThreshold = BAD_BOARD_TEMP_SENSE_STATUS_ALERT_SET_TIME_MS}, 
//     .setTime_MS = BAD_BOARD_TEMP_SENSE_STATUS_ALERT_SET_TIME_MS, .clearTime_MS = BAD_BOARD_TEMP_SENSE_STATUS_ALERT_CLEAR_TIME_MS, 
//     .alertConditionPresent = false,
//     .numAlertResponse = NUM_BAD_BOARD_TEMP_SENSE_STATUS_ALERT_RESPONSE, .alertResponse = badBoardTempSenseStatusAlertResponse
// };

// // Lost more than 60% of temp sensors in pack
// const AlertResponse_E insufficientTempSensorsAlertResponse[] = { DISABLE_BALANCING, DISABLE_CHARGING, AMS_FAULT };
// #define NUM_INSUFFICIENT_TEMP_SENSORS_ALERT_RESPONSE sizeof(insufficientTempSensorsAlertResponse) / sizeof(AlertResponse_E)
// Alert_S insufficientTempSensorsAlert = 
// {
//     .alertName = "InsufficientTempSensors", .latching = true,
//     .alertStatus = ALERT_CLEARED, .alertTimer = (Timer_S){.timCount = 0, .lastUpdate = 0, .timThreshold = INSUFFICIENT_TEMP_SENSOR_ALERT_SET_TIME_MS}, 
//     .setTime_MS = INSUFFICIENT_TEMP_SENSOR_ALERT_SET_TIME_MS, .clearTime_MS = INSUFFICIENT_TEMP_SENSOR_ALERT_CLEAR_TIME_MS, 
//     .alertConditionPresent = false,
//     .numAlertResponse = NUM_INSUFFICIENT_TEMP_SENSORS_ALERT_RESPONSE, .alertResponse = insufficientTempSensorsAlertResponse
// };

// Telemetry alerts

Alert_S* telemetryAlerts[] = 
{
    &overvoltageWarningAlert,
    &undervoltageWarningAlert,
    &overvoltageFaultAlert,
    &undervoltageFaultAlert
    // &cellImbalanceAlert,
    // &overtempWarningAlert,
    // &overtempFaultAlert,
    // &amsSdcFaultAlert,
    // &bspdSdcFaultAlert,
    // &imdSdcFaultAlert,
    // &badVoltageSenseStatusAlert,
    // &badBrickTempSenseStatusAlert,
    // &badBoardTempSenseStatusAlert,
    // &insufficientTempSensorsAlert
};

Alert_S* statusAlerts[] = 
{
    &imdSdcFaultAlert,
    &amsSdcFaultAlert,
    &amsSdcInhibitAlert
};

telemetryAlertCondition telemetryAlertConditionArray[] = 
{
    overvoltageWarningPresent,
    undervoltageWarningPresent,
    overvoltageFaultPresent,
    undervoltageFaultPresent
    // cellImbalancePresent,
    // overtemperatureWarningPresent,
    // overtemperatureFaultPresent,
    // amsSdcFaultPresent,
    // bspdSdcFaultPresent,
    // imdSdcFaultPresent,
    // badVoltageSensorStatusPresent,
    // badBrickTempSensorStatusPresent,
    // badBoardTempSensorStatusPresent,
    // insufficientTempSensePresent
};

statusAlertCondition statusAlertConditionArray[] = 
{
    imdSdcFaultPresent,
    amsSdcFaultPresent,
    amsSdcInhibitActive
};

// Number of alerts
const uint32_t NUM_TELEMETRY_ALERTS = sizeof(telemetryAlerts) / sizeof(Alert_S*);

const uint32_t NUM_STATUS_ALERTS = sizeof(statusAlerts) / sizeof(Alert_S*);
