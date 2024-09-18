#ifndef INC_ADBMS_H_
#define INC_ADBMS_H_

/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */
#include <stdint.h>

/* ==================================================================== */
/* ============================= DEFINES ============================== */
/* ==================================================================== */

#define BITS_IN_BYTE        8
#define BITS_IN_WORD        16
#define BYTES_IN_WORD       2

#define COMMAND_SIZE_BYTES       2
#define REGISTER_SIZE_BYTES      6

/* ==================================================================== */
/* ========================= ENUMERATED TYPES========================== */
/* ==================================================================== */

typedef enum
{
    TRANSACTION_CRC_ERROR = 0,
    TRANSACTION_SPI_ERROR,
    TRANSACTION_POR_ERROR,
    TRANSACTION_COMMAND_COUNTER_ERROR,
    TRANSACTION_SUCCESS
} TRANSACTION_STATUS_E;


typedef enum
{
    PORTA = 0,
    PORTB,
    NUM_PORTS
} PORT_E;


/* ==================================================================== */
/* =================== GLOBAL FUNCTION DEFINITIONS ==================== */
/* ==================================================================== */

TRANSACTION_STATUS_E sendCommand(uint16_t command, uint32_t numBmbs, PORT_E port);
TRANSACTION_STATUS_E writeRegister(uint16_t command, uint32_t numBmbs, uint8_t *txBuffer, PORT_E port);
TRANSACTION_STATUS_E readRegister(uint16_t command, uint32_t numBmbs, uint8_t *rxBuffer, PORT_E port);


#endif /* INC_ADBMS_H_ */

