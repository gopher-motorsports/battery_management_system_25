#ifndef INC_ISOSPI_H_
#define INC_ISOSPI_H_

/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */
#include <stdint.h>
#include <stdbool.h>

/* ==================================================================== */
/* ============================= DEFINES ============================== */
/* ==================================================================== */

// Max SPI Buffer size
#define MAX_SPI_BUFFER    256

#define BITS_IN_BYTE        8
#define BITS_IN_WORD        16
#define BYTE_SIZE_DEC       256
#define BYTES_IN_WORD       2

#define COMMAND_SIZE_BYTES       2
#define REGISTER_SIZE_BYTES      6

/* END ADBMS Register addresses */

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

typedef enum
{
    SHARED_COMMAND = 0,
    CELL_MONITOR_COMMAND,
    PACK_MONITOR_COMMAND
} COMMAND_TYPE_E;

typedef enum
{
    CELL_MONITOR = 0,
    PACK_MONITOR,
    NUM_DEVICE_TYPES
} DEVICE_TYPE_E;

typedef enum
{
    PORTA = 0,
    PORTB,
    NUM_PORTS
} PORT_E;

typedef enum
{
    MULTIPLE_CHAIN_BREAK = 0,
    SINGLE_CHAIN_BREAK,
    CHAIN_COMPLETE
} CHAIN_STATUS_E;

/* ==================================================================== */
/* ============================== STRUCTS============================== */
/* ==================================================================== */

typedef struct
{
    // Number of devices in the daisy chain
    uint32_t numDevs;

    // The port which is directly attached to the pack monitor
    PORT_E packMonitorPort;

    // The current status of the isospi chain
    CHAIN_STATUS_E chainStatus;

    // The number of reachable devices on each port
    uint32_t availableDevices[NUM_PORTS];

    // The current port to be used for isospi transactions
    PORT_E currentPort;

    // The local command counter tracker for pack and cell monitor devices
    uint32_t localCommandCounter[NUM_DEVICE_TYPES];
} CHAIN_INFO_S;

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DEFINITIONS ==================== */
/* ==================================================================== */

/**
 * @brief Renumerate the device daisy chain to determine chain status
 * @param chainInfo Chain data struct
 * @return Transaction status error code
 */
TRANSACTION_STATUS_E updateChainStatus(CHAIN_INFO_S *chainInfo);

/**
 * @brief Activate isospi communication port on slave devices by sending traffic
 * @param numDevs The number of devices in the communication chain
 * @param port The port on which to initiate wake up traffic
 * @param usDelay The number of microseconds to delay between isospi traffic events
 */
void activatePort(uint32_t numDevs, PORT_E port, uint32_t usDelay);

/**
 * @brief Send a command on the device daisy chain
 * @param command Command code to send
 * @param chainInfo Chain data struct
 * @param commandType The type of command to determine which devices will recognize the command
 * @return Transaction status error code
 */
TRANSACTION_STATUS_E commandChain(uint16_t command, CHAIN_INFO_S *chainInfo, COMMAND_TYPE_E commandType);

/**
 * @brief Write to device registers on the device daisy chain
 * @param command Command code to send
 * @param chainInfo Chain data struct
 * @param commandType The type of command to determine which devices will recognize the command
 * @param packMonitorData Byte array of data to write to pack montior
 * @param cellMonitorData Byte array of data to write to cell monitor chain
 * @return Transaction status error code
 */
TRANSACTION_STATUS_E writeChain(uint16_t command, CHAIN_INFO_S *chainInfo, COMMAND_TYPE_E commandType, uint8_t *txData);

/**
 * @brief Read from device registers on the device daisy chain
 * @param command Command code to send
 * @param chainInfo Chain data struct
 * @param packMonitorData Byte array of data to write to pack montior
 * @param cellMonitorData Byte array of data to write to cell monitor chain
 * @return Transaction status error code
 */
TRANSACTION_STATUS_E readChain(uint16_t command, CHAIN_INFO_S *chainInfo, uint8_t *rxData);

/**
 * @brief Write only to pack monitor register
 * @param command Command code to send
 * @param chainInfo Chain data struct
 * @param commandType The type of command to determine which devices will recognize the command
 * @param txData Byte array of data to write to the pack monitor
 * @return Transaction status error code
 */
TRANSACTION_STATUS_E writePackMonitor(uint16_t command, CHAIN_INFO_S *chainInfo, COMMAND_TYPE_E commandType, uint8_t *packMonitorData);

/**
 * @brief Read only from pack monitor register
 * @param command Command code to send
 * @param chainInfo Chain data struct
 * @param rxData Byte array of data to populate with data from the pack monitor
 * @return Transaction status error code
 */
TRANSACTION_STATUS_E readPackMonitor(uint16_t command, CHAIN_INFO_S *chainInfo, uint8_t *packMonitorData);

#endif /* INC_ISOSPI_H_ */
