#ifndef INC_TELEMETRYTASK_H_
#define INC_TELEMETRYTASK_H_

/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include <stdbool.h>
#include "adbms/isospi.h"
#include "adbms/adbms.h"
#include "soc.h"

/* ==================================================================== */
/* ============================= DEFINES ============================== */
/* ==================================================================== */

#define NUM_PACK_MON_IN_ACCUMULATOR 1
#define NUM_CELL_MON_IN_ACCUMULATOR 8
#define NUM_DEVICES_IN_ACCUMULATOR  (NUM_PACK_MON_IN_ACCUMULATOR + NUM_CELL_MON_IN_ACCUMULATOR)

// Use this to configure the order of the daisychain in the accumulator
// BMB0 is the first BMB connected to PORT A, assign the desired segment index here
#define BMB0_SEGMENT_INDEX  0
#define BMB1_SEGMENT_INDEX  1
// #define BMB2_SEGMENT_INDEX  2
// #define BMB3_SEGMENT_INDEX  3
// #define BMB4_SEGMENT_INDEX  4
// #define BMB5_SEGMENT_INDEX  5
// #define BMB6_SEGMENT_INDEX  6
// #define BMB7_SEGMENT_INDEX  7

/* ==================================================================== */
/* ========================= ENUMERATED TYPES========================== */
/* ==================================================================== */

typedef enum
{
    UNINITIALIZED = 0,
    GOOD,
    BAD
} SENSOR_STATUS_E;

typedef enum
{
    REDUNDANT_ADC_DIAG_STATE = 0,
    BALANCING_DIAG_STATE,
    OPEN_WIRE_EVEN_DIAG_STATE,
    OPEN_WIRE_ODD_DIAG_STATE,
    NUM_ADC_DIAG_STATES
} ADC_DIAG_STATE_E;

/* ==================================================================== */
/* ============================== STRUCTS ============================= */
/* ==================================================================== */

typedef struct
{
    // Cell voltage array
    float cellVoltage[NUM_CELLS_PER_CELL_MONITOR];
    SENSOR_STATUS_E cellVoltageStatus[NUM_CELLS_PER_CELL_MONITOR];

    // Balancing switch closed
    bool cellBalancingActive[NUM_CELLS_PER_CELL_MONITOR];

    // Cell temp array
    float cellTemp[NUM_CELLS_PER_CELL_MONITOR];
    SENSOR_STATUS_E cellTempStatus[NUM_CELLS_PER_CELL_MONITOR];

    // Board temp
    float boardTemp;
    SENSOR_STATUS_E boardTempStatus;

    // Onboard reference voltage
    float referenceVoltage;
    SENSOR_STATUS_E referenceVoltageStatus;

    // Die temperature
    float dieTemp;
    SENSOR_STATUS_E dieTempStatus;

    // Digital supply voltage
    float digitalSupplyVoltage;
    SENSOR_STATUS_E digitalSupplyVoltageStatus;

    // Analog supply voltage
    float analogSupplyVoltage;
    SENSOR_STATUS_E analogSupplyVoltageStatus;

    // Voltage divided reference
    float referenceResistorVoltage;
    SENSOR_STATUS_E referenceResistorVoltageStatus;


    // Cell monitor local voltage statistics
    float maxCellVoltage;
    float minCellVoltage;
    float sumCellVoltage;
    float avgCellVoltage;
    uint32_t numBadCellVoltage;

    // Cell monitor local temp statistics
    float maxCellTemp;
    float minCellTemp;
    float avgCellTemp;
    uint32_t numBadCellTemp;

} Cell_Monitor_S;

typedef struct
{

    /// Primary pack measurements

    // Pack current
    float packCurrent;
    SENSOR_STATUS_E packCurrentStatus;

    // Pack voltage
    float packVoltage;
    SENSOR_STATUS_E packVoltageStatus;

    // Pack energy
    float packPower;
    SENSOR_STATUS_E packPowerStatus;

    /// Auxillary pack measurements

    // Board Temp
    float boardTemp;
    SENSOR_STATUS_E boardTempStatus;

    // Shunt Temp 1
    float shuntTemp1;
    SENSOR_STATUS_E shuntTemp1Status;

    // Shunt Temp 1
    float shuntTemp2;
    SENSOR_STATUS_E shuntTemp2Status;

    // Precharge Temp
    float prechargeTemp;
    SENSOR_STATUS_E prechargeTempStatus;

    // Discharge Temp
    float dischargeTemp;
    SENSOR_STATUS_E dischargeTempStatus;

    // Link voltage
    float linkVoltage;
    SENSOR_STATUS_E linkVoltageStatus;

    /// Diagnostic sensors

    // Reference voltage
    float referenceVoltage1P25;
    SENSOR_STATUS_E referenceVoltage1P25Status;

    // Die Temp 1
    float dieTemp1;
    SENSOR_STATUS_E dieTemp1Status;

    // Regulator voltage
    float regulatorVoltage;
    SENSOR_STATUS_E regulatorVoltageStatus;

    // Supply voltage
    float supplyVoltage;
    SENSOR_STATUS_E supplyVoltageStatus;

    // Digital supply voltage
    float digitalSupplyVoltage;
    SENSOR_STATUS_E digitalSupplyVoltageStatus;

    // Ground pad voltage
    float groundPadVoltage;
    SENSOR_STATUS_E groundPadVoltageStatus;

    // Resistor reference
    float referenceResistorVoltage;
    SENSOR_STATUS_E referenceResistorVoltageStatus;

    // Die temp 2
    float dieTemp2;
    SENSOR_STATUS_E dieTemp2Status;

    
    /// Calculated values

    // Temperature adjusted shunt resistance
    float shuntResistanceMicroOhms;

    // Coulomb counter helper variables
    // Timer_S localPhaseCountTimer;

    // Conversion counter holds number of conversion counts over all time
    uint32_t adcConversionPhaseCounter;
    uint32_t nextQualifiedPhaseCount;
    float adcConversionTimeMS;

    // SOC and SOE data
    Soc_S socData;

    // Pack energy data
    int32_t packEnergyMilliJoules;
} Pack_Monitor_S;


typedef struct
{
    bool chainInitialized;

	Cell_Monitor_S bmb[NUM_CELL_MON_IN_ACCUMULATOR];
    SENSOR_STATUS_E bmbStatus[NUM_CELL_MON_IN_ACCUMULATOR];

    Pack_Monitor_S packMonitor;
    SENSOR_STATUS_E packMonitorStatus;

    bool balancingEnabled;
    float balancingFloor;

    CHAIN_INFO_S chainInfo;

    float cellSumVoltage;

    float maxCellVoltage;
    float minCellVoltage;
    float avgCellVoltage;

    float cellImbalance;

    float maxCellTemp;
    float minCellTemp;
    float avgCellTemp;

    float maxBoardTemp;
    float minBoardTemp;
    float avgBoardTemp;

    float maxDieTemp;
    float minDieTemp;
    float avgDieTemp;
    float numGoodDieTemps;

} telemetryTaskData_S;

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DECLARATIONS =================== */
/* ==================================================================== */

void initTelemetryTask();
void runTelemetryTask();

#endif /* INC_TELEMETRYTASK_H_ */
