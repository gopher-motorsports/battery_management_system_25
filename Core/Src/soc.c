/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include "soc.h"
#include "utils.h"
#include "cellData.h"

static float getSocFromCellVoltage(float cellVoltage);
static float getSoeFromSoc(float soc);
static float calculatePercent(uint32_t val, uint32_t max);

/* ==================================================================== */
/*!
  @brief   Get the state of charge (SOC) based on the cell voltage.
  @param   cellVoltage - The cell voltage to be used for the SOC lookup.
  @return  The state of charge as a percentage
*/
static float getSocFromCellVoltage(float cellVoltage)
{
    return lookup(cellVoltage, &socByOcvTable);
}

/*!
  @brief   Get the state of energy (SOE) based on the state of charge (SOC).
  @param   soc - The state of charge to be used for the SOE lookup.
  @return  The state of energy in percent
*/
static float getSoeFromSoc(float soc)
{
    return lookup(soc, &soeFromSocTable);
}

static float calculatePercent(uint32_t val, uint32_t max)
{
    if (val > max)
    {
        val = max;
    }

    if(max == 0)
    {
        return 0.0f;
    }

    return ((float)val / max);
}

/*!
  @brief   Update the state of charge (SOC) and state of energy (SOE) using the appropriate method.
  @param   soc - Pointer to the Soc_S struct containing the necessary information for SOC and SOE calculation.
  @param   minCellVoltage - The minimum cell voltage in the pack.
*/
void updateSocSoe(Soc_S* soc, float minCellVoltage)
{
    // get SOC and SOE by OCV
    soc->socByOcv = getSocFromCellVoltage(minCellVoltage);
    soc->soeByOcv = getSoeFromSoc(soc->socByOcv);

    if(checkTimerExpired(&soc->socByOcvQualificationTimer))
    {
        soc->socByCoulombCounting = soc->socByOcv;
        soc->soeByCoulombCounting = soc->soeByOcv;
        soc->milliCoulombCounter = (uint32_t)(soc->socByCoulombCounting * soc->packMilliCoulombs);
        // soc->milliJouleCounter = (uint32_t)(soc->soeByCoulombCounting * soc->packMilliJoules);
    }
    else
    {
        soc->socByCoulombCounting = calculatePercent(soc->milliCoulombCounter, soc->packMilliCoulombs);
        // soc->soeByCoulombCounting = calculatePercent(soc->milliJouleCounter, soc->packMilliJoules);
        soc->soeByCoulombCounting = getSoeFromSoc(soc->socByCoulombCounting);
    }
}
