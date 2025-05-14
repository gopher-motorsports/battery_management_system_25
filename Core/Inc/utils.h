#ifndef INC_UTILS_H_
#define INC_UTILS_H_

/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include <stdint.h>
#include "cmsis_os.h"
#include "stm32f4xx_hal.h"
#include <math.h>

/* ==================================================================== */
/* ============================= DEFINES ============================== */
/* ==================================================================== */

// The max spacing between two floats before they are considered not equal
#define EPSILON 1e-4f

#define MICROSECONDS_IN_MILLISECOND   1000
#define MILLISECONDS_IN_SECOND        1000
#define SECONDS_IN_MINUTE             60
#define MINUTES_IN_HOUR               60

/* ==================================================================== */
/* ============================== MACROS ============================== */
/* ==================================================================== */

/*!
  @brief   Determine if two floating point values are equal
  @returns True if equal, false if not equal
*/
#define fequals(a, b) (fabsf(a - b) < EPSILON) ? (true) : (false)

/* ==================================================================== */
/* ========================= ENUMERATED TYPES========================== */
/* ==================================================================== */

typedef enum
{
    SPI_TIMEOUT = 0,  // SPI timed out
    SPI_ERROR,        // SPI error occured
    SPI_SUCCESS       // SPI was successful
} SPI_STATUS_E;       // Interupt status enum for task notification flags

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DECLARATIONS =================== */
/* ==================================================================== */


void delayMicroseconds(uint32_t us);

SPI_STATUS_E taskNotifySPI(SPI_HandleTypeDef* hspi, uint8_t* txBuffer, uint8_t* rxBuffer, uint16_t size, uint32_t timeout);

#endif /* INC_UTILS_H_ */
