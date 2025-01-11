#ifndef INC_TELEMETRY_H_
#define INC_TELEMETRY_H_

/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */
#include "telemetryTask.h"
#include "adbms/adbms.h"

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DECLARATIONS =================== */
/* ==================================================================== */

TRANSACTION_STATUS_E runCommandBlock(TRANSACTION_STATUS_E (*telemetryFunction)(telemetryTaskData_S*), telemetryTaskData_S *taskData);

TRANSACTION_STATUS_E initChain(telemetryTaskData_S *taskData);

TRANSACTION_STATUS_E testBlock(telemetryTaskData_S *taskData);


#endif /* INC_TELEMETRY_H_ */
