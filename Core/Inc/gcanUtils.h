#ifndef INC_GCAN_UTILS_H_
#define INC_GCAN_UTILS_H_

/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include "GopherCAN.h"
#include "gopher_sense.h"
#include "telemetryTask.h"
#include "adbms/adbms.h"

/* ==================================================================== */
/* ============================= DEFINES ============================== */
/* ==================================================================== */

#define NUM_STAT_PARAMS     8

#define NUM_GCAN_ALERTS     13

/* ==================================================================== */
/* ======================= EXTERNAL VARIABLES ========================= */
/* ==================================================================== */

extern const FLOAT_CAN_STRUCT *cellVoltageParams[NUM_CELL_MON_IN_ACCUMULATOR][NUM_CELLS_PER_CELL_MONITOR];
extern const FLOAT_CAN_STRUCT *cellTempParams[NUM_CELL_MON_IN_ACCUMULATOR][NUM_CELLS_PER_CELL_MONITOR];
extern const FLOAT_CAN_STRUCT *cellStatParams[NUM_CELL_MON_IN_ACCUMULATOR][NUM_STAT_PARAMS];
extern const U8_CAN_STRUCT *bmsAlertsParams[NUM_GCAN_ALERTS];
extern const U8_CAN_STRUCT *bmsShutdownParams[NUM_SDC_SENSE_INPUTS];

#endif /* INC_GCAN_UTILS_H_ */