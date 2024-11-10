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
#define BYTE_SIZE_DEC       256
#define BYTES_IN_WORD       2

#define COMMAND_SIZE_BYTES       2
#define REGISTER_SIZE_BYTES      6

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


/* ==================================================================== */
/* =================== GLOBAL FUNCTION DEFINITIONS ==================== */
/* ==================================================================== */

void wakeChain(uint32_t numDevs);
void readyChain(uint32_t numDevs);
TRANSACTION_STATUS_E commandChain(uint16_t command, uint32_t numDevs);
TRANSACTION_STATUS_E writeChain(uint16_t command, uint32_t numDevs, uint8_t *txData);
TRANSACTION_STATUS_E readChain(uint16_t command, uint32_t numDevs, uint8_t *rxData);


#endif /* INC_ADBMS_H_ */

