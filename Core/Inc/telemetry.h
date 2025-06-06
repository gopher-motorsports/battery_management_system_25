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

TRANSACTION_STATUS_E updateBatteryTelemetry(telemetryTaskData_S *taskData);


#endif /* INC_TELEMETRY_H_ */
