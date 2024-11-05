#ifndef INC_SOC_H_
#define INC_SOC_H_

/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include "timer.h"
#include "cellData.h"
#include "packData.h"


/* ==================================================================== */
/* ============================= DEFINES ============================== */
/* ==================================================================== */

#define MILLISECONDS_IN_SECOND  1000
#define SECONDS_IN_MINUTE        60
#define MINUTES_IN_HOUR          60

//cmd codes
#define SNAP_COMMAND 0x2D
#define UNSNAP_COMMAND 0X2F
#define RDFLAG_COMMAND 0x72 //WITHOUT error injection (1) with artificial error injection (0)
#define RDIACC_COMMAND 0x44


#define CONVERSION_MULTI 36  //conversion time (need to change)
#define IADC_LSB 1e-6

#define MAX_ACCUMULATOR_MILLICOULOMBS CELL_CAPACITY_MAH * NUM_PARALLEL_CELLS * MINUTES_IN_HOUR * SECONDS_IN_MINUTE
#define SOC_BY_OCV_GOOD_QUALIFICATION_TIME_MS 10000
//#define SOC_BY_OCV_GOOD_QUALIFICATION_TIME_MS 5 * SECONDS_IN_MINUTE * MILLISECONDS_IN_SECOND

/* ==================================================================== */
/* ============================== STRUCTS============================== */
/* ==================================================================== */

typedef struct
{
    int32_t milliCoulombCounter;         // The coulomb counter
    Timer_S socByOcvQualificationTimer;  // The qualification timer to determine whether SOC by OCV can be used

    float socByOcv;                      // The state of charge using open circuit voltage
    float soeByOcv;                      // The state of energy using open circuit voltage
    float socByCoulombCounting;          // The state of charge using coulomb counting
    float soeByCoulombCounting;          // The state of energy using coulomb counting
} Soc_S;


/* ==================================================================== */
/* =================== GLOBAL FUNCTION DECLARATIONS =================== */
/* ==================================================================== */

/*!
  @brief   Update the state of charge (SOC) and state of energy (SOE) using the appropriate method.
  @param   soc - Pointer to the Soc_S struct containing the necessary information for SOC and SOE calculation.
*/


#endif /* INC_SOC_H_ */
