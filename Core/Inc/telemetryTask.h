#ifndef INC_TELEMETRYTASK_H_
#define INC_TELEMETRYTASK_H_

/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include "adbms.h"

/* ==================================================================== */
/* ============================= DEFINES ============================== */
/* ==================================================================== */

#define NUM_BMBS_IN_ACCUMULATOR     2
#define NUM_CELLS_PER_BMB           16

/* ==================================================================== */
/* ============================== STRUCTS ============================= */
/* ==================================================================== */

typedef struct
{   
    uint8_t testData[6];
    TRANSACTION_STATUS_E testStatus;
    
} Bmb_S;

typedef struct
{
	Bmb_S bmb[NUM_BMBS_IN_ACCUMULATOR];
		// Set by BMB based on ability to balance in hardware
} TelemetryTaskOutputData_S;

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DECLARATIONS =================== */
/* ==================================================================== */

void initTelemetryTask();
void runTelemetryTask();

#endif /* INC_TELEMETRYTASK_H_ */