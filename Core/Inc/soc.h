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
    uint32_t packMilliCoulombs;         // The millicoulomb capacity of the pack
    uint32_t milliCoulombCounter;       // The coulomb counter
    Timer_S socByOcvQualificationTimer; // The qualification timer to determine whether SOC by OCV can be used

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
  @param   socByOcvTable The cell data lookup table for soc vs ocv
  @param   soeFromSocTable The cell data lookup table for soe vs soc
  @param   minCellVoltage - The minimum cell voltage in the pack.
  @param   deltaMillicoulombs - The change in milliCoulombs since the last update.
*/
void updateSocSoe(Soc_S* soc, LookupTable_S *socByOcvTable, LookupTable_S *soeFromSocTable, float minCellVoltage, float deltaMillicoulombs);

#endif /* INC_SOC_H_ */
