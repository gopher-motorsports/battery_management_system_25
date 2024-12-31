#ifndef INC_TELEMETRYTASK_H_
#define INC_TELEMETRYTASK_H_

/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include <stdbool.h>
#include "adbms.h"
#include "soc.h"

/* ==================================================================== */
/* ============================= DEFINES ============================== */
/* ==================================================================== */

#define NUM_PACK_MON_IN_ACCUMULATOR 1
#define NUM_BMBS_IN_ACCUMULATOR     1
#define NUM_DEVICES_IN_ACCUMULATOR  (NUM_PACK_MON_IN_ACCUMULATOR + NUM_BMBS_IN_ACCUMULATOR)

#define PACK_MONITOR_PORT           PORTA

// Use this to configure the order of the daisychain in the accumulator
// BMB0 is the first BMB connected to PORT A, assign the desired segment index here
#define BMB0_SEGMENT_INDEX  0
// #define BMB1_SEGMENT_INDEX  1
// #define BMB2_SEGMENT_INDEX  2
// #define BMB3_SEGMENT_INDEX  3
// #define BMB4_SEGMENT_INDEX  4
// #define BMB5_SEGMENT_INDEX  5
// #define BMB6_SEGMENT_INDEX  6
// #define BMB7_SEGMENT_INDEX  7

#define NUM_CELLS_PER_BMB           16

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
    MUX_STATE_0 = 0,
    MUX_STATE_1,
    NUM_MUX_STATES
} MUX_STATE_E;

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
    float cellVoltage[NUM_CELLS_PER_BMB];
    SENSOR_STATUS_E cellVoltageStatus[NUM_CELLS_PER_BMB];

    float diagnosticCellVoltage[NUM_CELLS_PER_BMB];
    SENSOR_STATUS_E diagnosticCellVoltageStatus[NUM_CELLS_PER_BMB];

    float cellTemp[NUM_CELLS_PER_BMB];
    SENSOR_STATUS_E cellTempStatus[NUM_CELLS_PER_BMB];

    float maxCellVoltage;
    float minCellVoltage;
    float sumCellVoltage;
    float avgCellVoltage;
    uint32_t numBadCellVoltage;

    float maxCellTemp;
    float minCellTemp;
    float avgCellTemp;
    uint32_t numBadCellTemp;

    float boardTemp;
    SENSOR_STATUS_E boardTempStatus;

} Bmb_S;

typedef struct
{
    Timer_S localPhaseCountTimer;
    uint16_t lastPhaseCount;
    uint32_t adcConversionPhaseCounter;
    uint32_t nextGoodAccumulationDataPhaseCount;
    float adcConversionTimeMS;

    float shuntResistanceMicroOhms;
} Pack_Monitor_S;


typedef struct
{
    bool chainInitialized;

	Bmb_S bmb[NUM_BMBS_IN_ACCUMULATOR];
    Pack_Monitor_S packMonitorData;

    ADC_DIAG_STATE_E curentDiagnosticState;
    ADC_DIAG_STATE_E nextDiagnosticState;
    uint32_t diagnosticCycleCounter;

    bool balancingEnabled;

    MUX_STATE_E muxState;

    CHAIN_INFO_S chainInfo;

    Soc_S socData;

    float IADC1;
    float IADC2;

    float VBADC1;
    float VBADC2;



    uint8_t testData[NUM_DEVICES_IN_ACCUMULATOR][REGISTER_SIZE_BYTES];
    TRANSACTION_STATUS_E testStatus[NUM_DEVICES_IN_ACCUMULATOR];

    float packVoltage;

    float maxCellVoltage;
    float minCellVoltage;
    float avgCellVoltage;

    float maxCellTemp;
    float minCellTemp;
    float avgCellTemp;

    float maxBoardTemp;
    float minBoardTemp;
    float avgBoardTemp;

} telemetryTaskData_S;

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DECLARATIONS =================== */
/* ==================================================================== */

void initTelemetryTask();
void runTelemetryTask();

#endif /* INC_TELEMETRYTASK_H_ */
