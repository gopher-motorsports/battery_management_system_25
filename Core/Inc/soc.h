#ifndef INC_SOC_H_
#define INC_SOC_H_

/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include "timer.h"
#include "lookupTable.h"

/* ==================================================================== */
/* ============================== STRUCTS============================== */
/* ==================================================================== */

typedef struct
{
    Timer_S socByOcvQualificationTimer; // The qualification timer to determine whether SOC by OCV can be used

    uint32_t packMilliCoulombs;         // The millicoulomb capacity of the pack
    uint32_t milliCoulombCounter;       // The coulomb counter

    float socByOcv;                     // The state of charge using open circuit voltage
    float soeByOcv;                     // The state of energy using open circuit voltage
    float socByCoulombCounting;         // The state of charge using coulomb counting
    float soeByCoulombCounting;         // The state of energy using coulomb counting
} Soc_S;

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DECLARATIONS =================== */
/* ==================================================================== */

/*!
  @brief   Update the state of charge (SOC) and state of energy (SOE) using the appropriate method.
  @param   soc - Pointer to the Soc_S struct containing the necessary information for SOC and SOE calculation.
  @param   minCellVoltage - The minimum cell voltage in the pack.
*/
void updateSocSoe(Soc_S* soc, float minCellVoltage);

#endif /* INC_SOC_H_ */
