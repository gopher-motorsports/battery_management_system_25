/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include "soc.h"
#include "utils.h"

static float getSocFromCellVoltage(float cellVoltage, LookupTable_S *socByOcvTable);
static float getSoeFromSoc(float soc, LookupTable_S *soeFromSocTable);
static float calculateSocByCoulombCounting(uint32_t milliCoulombCounter, uint32_t maxMilliCoulombs);

/* ==================================================================== */
/*!
  @brief   Get the state of charge (SOC) based on the cell voltage.
  @param   cellVoltage - The cell voltage to be used for the SOC lookup.
  @return  The state of charge as a percentage
*/
static float getSocFromCellVoltage(float cellVoltage, LookupTable_S *socByOcvTable)
{
    return lookup(cellVoltage, socByOcvTable);
}

/*!
  @brief   Get the state of energy (SOE) based on the state of charge (SOC).
  @param   soc - The state of charge to be used for the SOE lookup.
  @return  The state of energy in percent
*/
static float getSoeFromSoc(float soc, LookupTable_S *soeFromSocTable)
{
    return lookup(soc, soeFromSocTable);
}

static float calculateSocByCoulombCounting(uint32_t milliCoulombCounter, uint32_t maxMilliCoulombs)
{
    if (milliCoulombCounter > maxMilliCoulombs)
    {
        milliCoulombCounter = maxMilliCoulombs;
    }

    if(maxMilliCoulombs == 0)
    {
        return 0.0f;
    }

    return ((float)milliCoulombCounter / maxMilliCoulombs);
}

/*!
  @brief   Update the state of charge (SOC) and state of energy (SOE) using the appropriate method.
  @param   soc - Pointer to the Soc_S struct containing the necessary information for SOC and SOE calculation.
  @param   socByOcvTable The cell data lookup table for soc vs ocv
  @param   soeFromSocTable The cell data lookup table for soe vs soc
  @param   minCellVoltage - The minimum cell voltage in the pack.
  @param   deltaMillicoulombs - The change in milliCoulombs since the last update.
*/
void updateSocSoe(Soc_S* soc, LookupTable_S *socByOcvTable, LookupTable_S *soeFromSocTable, float minCellVoltage, float deltaMillicoulombs)
{
    // get SOC and SOE by OCV
    soc->socByOcv = getSocFromCellVoltage(minCellVoltage, socByOcvTable);
    soc->soeByOcv = getSoeFromSoc(soc->socByOcv, soeFromSocTable);

    // if at 0A then use OCV
    if(fequals(deltaMillicoulombs, 0))
    {
        // update the qualification timer
        updateTimer(&soc->socByOcvQualificationTimer);

        // if timer has expired, calculate SOC and SOE by OCV
        if(checkTimerExpired(&soc->socByOcvQualificationTimer))
        {
            soc->socByCoulombCounting = soc->socByOcv;
            soc->soeByCoulombCounting = soc->soeByOcv;
            //back calculate milliCoulombCounter
            soc->milliCoulombCounter = (uint32_t)(soc->socByCoulombCounting * soc->packMilliCoulombs);
            return;
        }
    }
    else
    {
        // Reset the timer if current is flowing (i.e., deltaMilliCoulombs > 0)
        clearTimer(&soc->socByOcvQualificationTimer);
    }

    // Update the milliCoulomb counter with the new charge
    soc->milliCoulombCounter += deltaMillicoulombs;
    // Calculate SOC and SOE using Coulomb Counting
    soc->socByCoulombCounting = calculateSocByCoulombCounting(soc->milliCoulombCounter, soc->packMilliCoulombs);
    soc->soeByCoulombCounting = getSoeFromSoc(soc->socByCoulombCounting, soeFromSocTable);
}
