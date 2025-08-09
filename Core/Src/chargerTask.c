/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include "main.h"
#include "chargerTask.h"
#include "charger.h"
#include "GopherCAN.h"
#include "gopher_sense.h"
#include "cellData.h"
#include "packData.h"
#include <stdlib.h>
#include <string.h>

/* ==================================================================== */
/* ============================= DEFINES ============================== */
/* ==================================================================== */

// Communication timeout ms
#define ELCON_CHARGER_COMM_TIMEOUT  3000
#define CHARGER_BOARD_COMM_TIMEOUT  10000

// Power limit
#define DEFAULT_POWER_LIMIT_W       1500.0f
#define ABSOLUTE_POWER_LIMIT_W      6000.0f

// Voltage limit
#define MAX_CHARGE_VOLTAGE_V                MAX_BRICK_VOLTAGE * NUM_SERIES_CELLS

// Current limit
#define MAH_TO_AH                           (1.0f / 1000.0f)
#define MAX_CHARGE_CURRENT_A                CELL_CAPACITY_MAH * MAH_TO_AH * MAX_C_RATING * NUM_PARALLEL_CELLS

// Voltage threshold to switch to CV mode
#define CELL_VOLTAGE_CV_THRES               4.15f
#define CELL_VOLTAGE_CV_HYS                 0.15f

#define CELL_IMBALANCE_THRESHOLD            0.01f

/* ==================================================================== */
/* =================== LOCAL FUNCTION DECLARATIONS ==================== */
/* ==================================================================== */

static float getPowerLimit();
static float getCurrentLimit(float powerLimit, float packVoltage);

/* ==================================================================== */
/* =================== LOCAL FUNCTION DEFINITIONS ===================== */
/* ==================================================================== */

static float getPowerLimit()
{
    // Calculate the power limit
    float powerLimit = DEFAULT_POWER_LIMIT_W;
    if((HAL_GetTick() - chargingPowerLimit.info.last_rx) <= CHARGER_BOARD_COMM_TIMEOUT)
    {
        powerLimit = chargingPowerLimit.data;
        
        // Clamp power limit
        if(powerLimit > ABSOLUTE_POWER_LIMIT_W)
        {
            powerLimit = ABSOLUTE_POWER_LIMIT_W;
        }
        else if(powerLimit < 0.0f)
        {
            powerLimit = 0.0f;
        }
    }

    return powerLimit;
}

static float getCurrentLimit(float powerLimit, float packVoltage)
{
    // Calculate the current request
    float currentLimit = powerLimit / packVoltage;

    // Clamp current limit
    if(currentLimit > MAX_CHARGE_CURRENT_A)
    {
        currentLimit = MAX_CHARGE_CURRENT_A;
    }
    else if(currentLimit < 0.0f)
    {
        currentLimit = 0.0f;
    }

    return currentLimit;
}

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DEFINITIONS ==================== */
/* ==================================================================== */

void initChargerTask()
{

}

void runChargerTask()
{
    // Input telem and charger data
    telemetryTaskData_S telemetryDataLocal;
    chargerTaskData_S chargerDataLocal;

    vTaskSuspendAll();
    telemetryDataLocal = telemetryTaskData;
    chargerDataLocal = chargerTaskData;
    xTaskResumeAll();

    // Check if the charger is connected
    bool chargerConnected = (chargerStatusByte.info.last_rx != 0) && ((HAL_GetTick() - chargerStatusByte.info.last_rx) < ELCON_CHARGER_COMM_TIMEOUT);

    // Get charger power limit
    chargerDataLocal.chargerPowerLimit = getPowerLimit();

    // Charger state machine

    switch(chargerDataLocal.chargerState)
    {
        case CHARGER_STATE_DISCONNECTED:
        {
            chargerDataLocal.chargerVoltageSetpoint = 0.0f;
            chargerDataLocal.chargerCurrentSetpoint = 0.0f;

            if(chargerConnected)
            {
                if(telemetryDataLocal.maxCellVoltage >= MAX_BRICK_VOLTAGE)
                {
                    float cellImbalance = (telemetryDataLocal.maxCellVoltage - telemetryDataLocal.minCellVoltage);
                    if(cellImbalance >= CELL_IMBALANCE_THRESHOLD)
                    {
                        chargerDataLocal.chargerState = CHARGER_STATE_BALANCING;
                    }
                    else
                    {
                        chargerDataLocal.chargerState = CHARGER_STATE_COMPLETE;
                    }
                }
                else if(telemetryDataLocal.maxCellVoltage >= CELL_VOLTAGE_CV_THRES)
                {
                    chargerDataLocal.chargerState = CHARGER_STATE_CONSTANT_VOLTAGE;
                }
                else
                {
                    chargerDataLocal.chargerState = CHARGER_STATE_CONSTANT_CURRENT;
                }
            }
            break;
        }
        case CHARGER_STATE_CONSTANT_CURRENT:
        {
            // Request max voltage and current under the determined power limit
            chargerDataLocal.chargerCurrentSetpoint = getCurrentLimit(chargerDataLocal.chargerPowerLimit, telemetryDataLocal.packMonitor.packVoltage);
            chargerDataLocal.chargerVoltageSetpoint = MAX_CHARGE_VOLTAGE_V;

            if(chargerConnected)
            {
                if(telemetryDataLocal.maxCellVoltage >= CELL_VOLTAGE_CV_THRES)
                {
                    chargerDataLocal.chargerState = CHARGER_STATE_CONSTANT_VOLTAGE;
                }
            }
            else
            {
                chargerDataLocal.chargerState = CHARGER_STATE_DISCONNECTED;
            }
            break;
        }
        case CHARGER_STATE_CONSTANT_VOLTAGE:
        {
            // Request max voltage and current under the determined power limit
            float currentLimit = getCurrentLimit(chargerDataLocal.chargerPowerLimit, MAX_CHARGE_VOLTAGE_V);

            // Get a scaling factor according to difference between highest cell voltage and max cell voltage 
            float deratingFactor = (MAX_BRICK_VOLTAGE - telemetryDataLocal.maxCellVoltage) / (MAX_BRICK_VOLTAGE - CELL_VOLTAGE_CV_THRES);
            if(deratingFactor > 1.0f)
            {
                deratingFactor = 1.0f;
            }
            else if(deratingFactor < 0.0f)
            {
               deratingFactor = 0.0f; 
            }

            chargerDataLocal.chargerCurrentSetpoint = (deratingFactor * currentLimit);
            chargerDataLocal.chargerVoltageSetpoint = MAX_CHARGE_VOLTAGE_V;

            if(chargerConnected)
            {
                if(telemetryDataLocal.maxCellVoltage >= MAX_BRICK_VOLTAGE)
                {
                    float cellImbalance = (telemetryDataLocal.maxCellVoltage - telemetryDataLocal.minCellVoltage);
                    if(cellImbalance >= CELL_IMBALANCE_THRESHOLD)
                    {
                        // Rest?
                        chargerDataLocal.chargerState = CHARGER_STATE_BALANCING;
                    }
                    else
                    {
                        chargerDataLocal.chargerState = CHARGER_STATE_COMPLETE;
                    }
                }
                else if(telemetryDataLocal.maxCellVoltage <= (CELL_VOLTAGE_CV_THRES - CELL_VOLTAGE_CV_HYS))
                {
                    chargerDataLocal.chargerState = CHARGER_STATE_CONSTANT_CURRENT;
                }
            }
            else
            {
                chargerDataLocal.chargerState = CHARGER_STATE_DISCONNECTED;
            }
            break;
        }
        case CHARGER_STATE_BALANCING:
        {
            chargerDataLocal.chargerCurrentSetpoint = 0.0f;
            chargerDataLocal.chargerVoltageSetpoint = 0.0f;
            

            if(chargerConnected)
            {
                float cellImbalance = (telemetryDataLocal.maxCellVoltage - telemetryDataLocal.minCellVoltage);
                if((telemetryDataLocal.maxCellVoltage <= CELL_VOLTAGE_CV_THRES) || (cellImbalance <= CELL_IMBALANCE_THRESHOLD))
                {
                    chargerDataLocal.chargerState = CHARGER_STATE_CONSTANT_VOLTAGE;
                }
            }
            else
            {
                chargerDataLocal.chargerState = CHARGER_STATE_DISCONNECTED;
            }
            break;
        }
        case CHARGER_STATE_COMPLETE:
        {
            chargerDataLocal.chargerCurrentSetpoint = 0.0f;
            chargerDataLocal.chargerVoltageSetpoint = 0.0f;

            if(!chargerConnected)
            {
                chargerDataLocal.chargerState = CHARGER_STATE_DISCONNECTED;
            }
            break;
        }
        default:
        {
            chargerDataLocal.chargerState = CHARGER_STATE_DISCONNECTED;
            break;
        }
    }

    if(chargerDataLocal.chargerState != CHARGER_STATE_DISCONNECTED)
    {
        sendChargerMessage(chargerDataLocal.chargerVoltageSetpoint, chargerDataLocal.chargerCurrentSetpoint, true);
        chargerDataLocal.chargerVoltage = chargerVoltageSetPoint_V.data;
        chargerDataLocal.chargerCurrent = chargerCurrentSetPoint_A.data;
        memcpy(&chargerDataLocal.chargerStatus, &chargerStatusByte.data, 1);
    }
    else
    {
        chargerDataLocal.chargerVoltage = 0.0f;
        chargerDataLocal.chargerCurrent = 0.0f;
        memcpy(&chargerDataLocal.chargerStatus, 0x00, 1);
    }

    vTaskSuspendAll();
    chargerTaskData = chargerDataLocal;
    xTaskResumeAll();
}

