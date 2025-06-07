#ifndef INC_STATUS_UPDATE_TASK_H_
#define INC_STATUS_UPDATE_TASK_H_

/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include "imd.h"
#include <stdbool.h>

/* ==================================================================== */
/* ============================= DEFINES ============================== */
/* ==================================================================== */

#define NUM_SDC_SENSE_INPUTS    6

/* ==================================================================== */
/* ============================== STRUCTS ============================= */
/* ==================================================================== */

typedef struct
{
    bool imdLatchOpen;
    bool bmsLatchOpen;
    bool bmsInhibitActive;
    bool sdcSenseFaultActive[NUM_SDC_SENSE_INPUTS];
} shutdownCircuitStatus_S;

typedef struct
{
    imdData_S imdData;
    shutdownCircuitStatus_S shutdownCircuitData;
} statusUpdateTaskData_S;

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DECLARATIONS =================== */
/* ==================================================================== */

void initStatusUpdateTask();
void runStatusUpdateTask();

#endif // INC_STATUS_UPDATE_TASK_H_
