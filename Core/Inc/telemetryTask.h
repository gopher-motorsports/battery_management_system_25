#ifndef INC_TELEMETRYTASK_H_
#define INC_TELEMETRYTASK_H_

/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include "adbms.h"

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

/* ==================================================================== */
/* ============================== STRUCTS ============================= */
/* ==================================================================== */


typedef struct
{
    float cellVoltage[NUM_CELLS_PER_BMB];
    SENSOR_STATUS_E cellVoltageStatus[NUM_CELLS_PER_BMB];

    float cellTemp[NUM_CELLS_PER_BMB];
    SENSOR_STATUS_E cellTempStatus[NUM_CELLS_PER_BMB];

    float boardTemp;
    SENSOR_STATUS_E boardTempStatus;

} Bmb_S;

typedef struct
{
	Bmb_S bmb[NUM_BMBS_IN_ACCUMULATOR];

    CHAIN_INFO_S chainInfo;

    float IADC1;
    float IADC2;

    float VBADC1;
    float VBADC2;

    uint8_t testData[NUM_DEVICES_IN_ACCUMULATOR][REGISTER_SIZE_BYTES];
    TRANSACTION_STATUS_E testStatus[NUM_DEVICES_IN_ACCUMULATOR];

} telemetryTaskData_S;

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DECLARATIONS =================== */
/* ==================================================================== */

void initTelemetryTask();
void runTelemetryTask();

#endif /* INC_TELEMETRYTASK_H_ */
