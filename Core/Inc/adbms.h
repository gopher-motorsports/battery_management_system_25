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

// ADBMS Register addresses
#define WR_CFG_REG_A            0x0001
#define RD_CFG_REG_A            0x0002

/* ==================================================================== */
/* ========================= ENUMERATED TYPES========================== */
/* ==================================================================== */

typedef enum
{
    TRANSACTION_CHAIN_BREAK_ERROR = 0,
    TRANSACTION_SPI_ERROR,
    TRANSACTION_POR_ERROR,
    TRANSACTION_COMMAND_COUNTER_ERROR,
    TRANSACTION_SUCCESS
} TRANSACTION_STATUS_E;


/* ==================================================================== */
/* =================== GLOBAL FUNCTION DEFINITIONS ==================== */
/* ==================================================================== */

void wakeChain(uint32_t numBmbs);
void readyChain(uint32_t numBmbs);
TRANSACTION_STATUS_E commandChain(uint16_t command, uint32_t numBmbs);
TRANSACTION_STATUS_E writeChain(uint16_t command, uint32_t numBmbs, uint8_t *txData);
TRANSACTION_STATUS_E readChain(uint16_t command, uint32_t numBmbs, uint8_t *rxData);


#endif /* INC_ADBMS_H_ */

