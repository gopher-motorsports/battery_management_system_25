#ifndef INC_CHARGER_H_
#define INC_CHARGER_H_

/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */
#include <stdbool.h>
#include "cellData.h"
#include "packData.h"

/* ==================================================================== */
/* ============================= DEFINES ============================== */
/* ==================================================================== */

// Charger TX CAN EXT ID, Charger RX EXT ID is CHARGER_CAN_ID + 1
#define CHARGER_CAN_ID_TX                   0x1806E5F4

#define MAX_CHARGE_VOLTAGE_V                MAX_BRICK_VOLTAGE * NUM_SERIES_CELLS

#define MAH_TO_AH                           (1.0f / 1000.0f)
#define MAX_CHARGE_CURRENT_A                CELL_CAPACITY_MAH * MAH_TO_AH * MAX_C_RATING * NUM_PARALLEL_CELLS

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DECLARATIONS =================== */
/* ==================================================================== */

/*!
  @brief   Send a CAN message to the charger
  @param   voltageRequest - Charger Voltage Request
  @param   currentRequest - Charger Current Request
  @param   enable - Enable/Disable Request
*/
void sendChargerMessage(float voltageRequest, float currentRequest, bool enable);

#endif /* INC_CHARGER_H_ */
