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

#define BITS_IN_BYTE        8
#define BITS_IN_WORD        16
#define BYTE_SIZE_DEC       256
#define BYTES_IN_WORD       2

#define REGISTER_BYTE0      0
#define REGISTER_BYTE1      1
#define REGISTER_BYTE2      2
#define REGISTER_BYTE3      3
#define REGISTER_BYTE4      4
#define REGISTER_BYTE5      5

#define COMMAND_SIZE_BYTES       2
#define REGISTER_SIZE_BYTES      6

// #define CELLS_PER_REG       3
// #define CELL_REG_SIZE       (REGISTER_SIZE_BYTES / CELLS_PER_REG)

// #define VBATT_DATA_PER_REG  3
// #define VBATT_DATA_SIZE     (REGISTER_SIZE_BYTES / VBATT_DATA_PER_REG)

// #define ENERGY_DATA_PER_REG 2
// #define ENERGY_DATA_SIZE    (REGISTER_SIZE_BYTES / ENERGY_DATA_PER_REG)

// #define VADC_GAIN           0.00015f
// #define VADC_OFFSET         1.5f

// #define IADC_GAIN           0.000001f
// #define VBADC1_GAIN         0.0001f
// #define VBADC2_GAIN         0.000085f

// #define NUM_CELLV_REGISTERS 6
// #define NUM_AUXV_REGISTERS  3

// #define MAX_CELLV_SENSOR_VALUE  6.41505f
// #define MIN_CELLV_SENSOR_VALUE  -3.4152f

/* ADBMS Register addresses */

// Write Configuration Register Group A
#define WRCFGA 0x0001
// Write Configuration Register Group B
#define WRCFGB 0x0024
// Read Configuration Register Group A
#define RDCFGA 0x0002
// Read Configuration Register Group B
#define RDCFGB 0x0026
// Read Cell Voltage Register Group A
#define RDCVA_RDI 0x0004
// Read Cell Voltage Register Group B
#define RDCVB_RDVB 0x0006
// Read Cell Voltage Register Group C
#define RDCVC 0x0008
// Read Cell Voltage Register Group D
#define RDCVD 0x000A
// Read Cell Voltage Register Group E
#define RDCVE 0x0009
// Read Cell Voltage Register Group F
#define RDCVF 0x000B
// Read All Cell Results
#define RDCVALL 0x000C
// Read Averaged Cell Voltage Register Group A
#define RDACA 0x0044
// Read Averaged Cell Voltage Register Group B
#define RDACB 0x0046
// Read Averaged Cell Voltage Register Group C
#define RDACC 0x0048
// Read Averaged Cell Voltage Register Group D
#define RDACD 0x004A
// Read Averaged Cell Voltage Register Group E
#define RDACE 0x0049
// Read Averaged Cell Voltage Register Group F
#define RDACF 0x004B
// Read All Avg Cell Results
#define RDACALL 0x004C
// Read S Voltage Register Group A
#define RDSVA 0x0003
// Read S Voltage Register Group B
#define RDSVB 0x0005
// Read S Voltage Register Group C
#define RDSVC 0x0007
// Read S Voltage Register Group D
#define RDSVD 0x000D
// Read S Voltage Register Group E
#define RDSVE 0x000E
// Read S Voltage Register Group F
#define RDSVF 0x000F
// Read All S Results
#define RDSALL 0x0010
// Read all C and S Results
#define RDCSALL 0x0011
// Read all Average C and S Results
#define RDACSALL 0x0051
// Read Filter Cell Voltage Register Group A
#define RDFCA 0x0012
// Read Filter Cell Voltage Register Group B
#define RDFCB 0x0013
// Read Filter Cell Voltage Register Group C
#define RDFCC 0x0014
// Read Filter Cell Voltage Register Group D
#define RDFCD 0x0015
// Read Filter Cell Voltage Register Group E
#define RDFCE 0x0016
// Read Filter Cell Voltage Register Group F
#define RDFCF 0x0017
// Read All Filter Cell Results
#define RDFCALL 0x0018
// Read Auxiliary Register Group A
#define RDAUXA 0x0019
// Read Auxiliary Register Group B
#define RDAUXB 0x001A
// Read Auxiliary Register Group C
#define RDAUXC 0x001B
// Read Auxiliary Register Group D
#define RDAUXD 0x001F
// Read Redundant Auxiliary Register Group A
#define RDRAXA 0x001C
// Read Redundant Auxiliary Register Group B
#define RDRAXB 0x001D
// Read Auxiliary Redundant Register Group C
#define RDRAXC 0x001E
// Read Auxiliary Redundant Register Group D
#define RDRAXD 0x0025
// Read Status Register Group A
#define RDSTATA 0x0030
// Read Status Register Group B
#define RDSTATB 0x0031
// Read Status Register Group C
#define RDSTATC 0x0032
// Read Status Register Group D
#define RDSTATD 0x0033
// Read Status Register Group E
#define RDSTATE 0x0034
// Read all AUX/Status Registers
#define RDASALL 0x0035
// Write PWM Register Group A
#define WRPWMA 0x0020
// Read PWM Register Group A
#define RDPWMA 0x0022
// Write PWM Register Group B
#define WRPWMB 0x0021
// Read PWM Register Group B
#define RDPWMB 0x0023
// LPCM Disable
#define CMDIS 0x0040
// LPCM Enable
#define CMEN 0x0041
// LPCM Heartbeat
#define CMHB2 0x0043
// Write LPCM Configuration Register
#define WRCMCFG 0x0058
//Read LPCM Configuration Register
#define RDCMCFG 0x0059
//Write LPCM Cell Threshold
#define WRCMCELLT 0x005A
//Read LPCM Cell Threshold
#define RDCMCELLT 0x005B
//Write LPCM GPIO Threshold
#define WRCMGPIOT 0x005C
//Read LPCM GPIO Threshold
#define RDCMGPIOT 0x005D
//Clear LPCM Flags
#define CLRCMFLAG 0x005E
//Read LPCM Flags
#define RDCMFLAG 0x005F
//Start Cell Voltage ADC Conversion and Poll Status
#define ADCV 0x0260
//Start S-ADC Conversion and Poll Status
#define ADSV 0x0168
//Start AUX ADC Conversions and Poll Status
#define ADAX 0x0410
//Start AUX2 ADC Conversions and Poll Status
#define ADAX2 0x0400
//Clear Cell Voltage Register Groups
#define CLRCELL 0x0711
//Clear Filtered Cell Voltage Register Groups
#define CLRFC 0x0714
//Clear Auxiliary Register Groups
#define CLRAUX 0x0712
//Clear S-Voltage Register Groups
#define CLRSPIN 0x0716
//Clear Flags
#define CLRFLAG 0x0717
//Clear OVUV
#define CLOVUV 0x0715
//Poll Any ADC Status
#define PLADC 0x0718
//Poll C-ADC
#define PLCADC 0x071C
//Poll S-ADC
#define PLSADC 0x071D
//Poll AUX ADC
#define PLAUX 0x071E
//Poll AUX2 ADC
#define PLAUX2 0x071F
//Write COMM Register Group
#define WRCOMM 0x0721
//Read COMM Register Group
#define RDCOMM 0x0722
//Start I2C/SPI Communication
#define STCOMM 0x0723
//Mute Discharge
#define MUTE 0x0028
//Unmute Discharge
#define UNMUTE 0x0029
//Read Serial ID Register Group
#define RDSID 0x002C
//Reset Command Counter
#define RSTCC 0x002E
//Snapshot
#define SNAP 0x002D
//Release Snapshot
#define UNSNAP 0x002F
//Soft Reset
#define SRST 0x0027
//Unlock Retention Register
#define ULRR 0x0038
//Write Retention Registers
#define WRRR 0x0039
//Read Retention Registers
#define RDRR 0x003A

// Register address configurations

// Inject error in STAT C reading
#define STATC_ERR (1 << 6)
// ADC redundant ADC conversions
#define ADC_RD (1 << 8)
// ADC discharge permitted
#define ADC_DCP (1 << 4)
// ADC Continuous conversion
#define ADC_CONT (1 << 7)
// ADC close even open wire switches
#define ADC_OW_EVEN (1)
// ADC close odd open wire switches
#define ADC_OW_ODD (1 << 1)
// AUX ADC switch to pull up from pull down OW switches
#define ADC_PUP (1 << 7)

// Status register C sleep bit
#define STATC_SLEEP_BIT 0x08

#define IADC_QUALIFICATION_TIME_MS          132
#define ACCUMULATION_REGISTER_COUNT         24
#define ACCN_CTRL_BITS                      ((uint8_t)(ACCUMULATION_REGISTER_COUNT / 4) - 1)
#define PHASE_COUNTS_PER_MS                 4

#define ALL_PACK_MON_GPIO 0x3F
#define ALL_CELL_MON_GPIO 0xFF

#define GPIO1 (1)
#define GPIO2 (1 << 1)
#define GPIO3 (1 << 2)
#define GPIO4 (1 << 3)
#define GPIO5 (1 << 4)
#define GPIO6 (1 << 5)
#define GPIO7 (1 << 6)
#define GPIO8 (1 << 7)
#define GPIO9 (1)
#define GPIO10 (1 << 1)

#define CADC_FILTER_CUTOFF_110HZ    0x01
#define CADC_FILTER_CUTOFF_45HZ     0x02
#define CADC_FILTER_CUTOFF_21HZ     0x03
#define CADC_FILTER_CUTOFF_10HZ     0x04
#define CADC_FILTER_CUTOFF_5HZ      0x05
#define CADC_FILTER_CUTOFF_1HZ25    0x06
#define CADC_FILTER_CUTOFF_0HZ625   0x07

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
 * @brief Wake up the device daisy chain from sleep
 * @param chainInfo Chain data struct
 */
void wakeChain(CHAIN_INFO_S *chainInfo);

/**
 * @brief Wake up the device daisy chain from idle
 * @param chainInfo Chain data struct
 */
void readyChain(CHAIN_INFO_S *chainInfo);

/**
 * @brief Renumerate the device daisy chain to determine chain status
 * @param chainInfo Chain data struct
 * @return Transaction status error code
 */
TRANSACTION_STATUS_E updateChainStatus(CHAIN_INFO_S *chainInfo);

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
TRANSACTION_STATUS_E writeChain(uint16_t command, CHAIN_INFO_S *chainInfo, COMMAND_TYPE_E commandType, uint8_t *packMonitorData, uint8_t *cellMonitorData);

/**
 * @brief Read from device registers on the device daisy chain
 * @param command Command code to send
 * @param chainInfo Chain data struct
 * @param packMonitorData Byte array of data to write to pack montior
 * @param cellMonitorData Byte array of data to write to cell monitor chain
 * @return Transaction status error code
 */
TRANSACTION_STATUS_E readChain(uint16_t command, CHAIN_INFO_S *chainInfo, uint8_t *packMonitorData, uint8_t *cellMonitorData);

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
