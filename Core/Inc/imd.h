#ifndef INC_IMD_H_
#define INC_IMD_H_

/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include <stdint.h>
#include <stdbool.h>

/* ==================================================================== */
/* ========================= ENUMERATED TYPES========================== */
/* ==================================================================== */

typedef enum
{
	IMD_NO_SIGNAL = 0,
	IMD_NORMAL,
	IMD_UNDER_VOLT,
	IMD_SPEED_START_MEASUREMENT,
	IMD_DEVICE_ERROR,
	IMD_CHASSIS_FAULT
} IMD_Status_E;

/* ==================================================================== */
/* ============================== STRUCTS============================== */
/* ==================================================================== */

typedef struct
{
    IMD_Status_E imdStatus;
    uint32_t isolationResistance;
	bool speedStartSuccess;
	uint32_t speedStartTime;
} imdData_S;

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DECLARATIONS =================== */
/* ==================================================================== */

/*!
  @brief    Determine the current status of the IMD from the PWM output
  @param    imdData Data struct to return imd status information
*/
void getImdStatus(imdData_S *imdData);

#endif /* INC_IMD_H_ */
