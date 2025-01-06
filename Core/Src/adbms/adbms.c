/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */
#include "adbms/isospi.h"
#include "adbms/adbms.h"
#include <string.h>

/* ==================================================================== */
/* ============================= DEFINES ============================== */
/* ==================================================================== */

/* ADBMS Register addresses */

#define WRCFGA      0x0001 // Write Configuration Register Group A
#define WRCFGB      0x0024 // Write Configuration Register Group B
#define RDCFGA      0x0002 // Read Configuration Register Group A
#define RDCFGB      0x0026 // Read Configuration Register Group B
#define RDCVA       0x0004 // Read Cell Voltage Register Group A
#define RDCVB       0x0006 // Read Cell Voltage Register Group B
#define RDCVC       0x0008 // Read Cell Voltage Register Group C
#define RDCVD       0x000A // Read Cell Voltage Register Group D
#define RDCVE       0x0009 // Read Cell Voltage Register Group E
#define RDCVF       0x000B // Read Cell Voltage Register Group F
#define RDACA       0x0044 // Read Averaged Cell Voltage Register Group A
#define RDACB       0x0046 // Read Averaged Cell Voltage Register Group B
#define RDACC       0x0048 // Read Averaged Cell Voltage Register Group C
#define RDACD       0x004A // Read Averaged Cell Voltage Register Group D
#define RDACE       0x0049 // Read Averaged Cell Voltage Register Group E
#define RDACF       0x004B // Read Averaged Cell Voltage Register Group F
#define RDSVA       0x0003 // Read S Voltage Register Group A
#define RDSVB       0x0005 // Read S Voltage Register Group B
#define RDSVC       0x0007 // Read S Voltage Register Group C
#define RDSVD       0x000D // Read S Voltage Register Group D
#define RDSVE       0x000E // Read S Voltage Register Group E
#define RDSVF       0x000F // Read S Voltage Register Group F
#define RDFCA       0x0012 // Read Filter Cell Voltage Register Group A
#define RDFCB       0x0013 // Read Filter Cell Voltage Register Group B
#define RDFCC       0x0014 // Read Filter Cell Voltage Register Group C
#define RDFCD       0x0015 // Read Filter Cell Voltage Register Group D
#define RDFCE       0x0016 // Read Filter Cell Voltage Register Group E
#define RDFCF       0x0017 // Read Filter Cell Voltage Register Group F
#define RDAUXA      0x0019 // Read Auxiliary Register Group A
#define RDAUXB      0x001A // Read Auxiliary Register Group B
#define RDAUXC      0x001B // Read Auxiliary Register Group C
#define RDAUXD      0x001F // Read Auxiliary Register Group D
#define RDRAXA      0x001C // Read Redundant Auxiliary Register Group A
#define RDRAXB      0x001D // Read Redundant Auxiliary Register Group B
#define RDRAXC      0x001E // Read Auxiliary Redundant Register Group C
#define RDRAXD      0x0025 // Read Auxiliary Redundant Register Group D
#define RDSTATA     0x0030 // Read Status Register Group A
#define RDSTATB     0x0031 // Read Status Register Group B
#define RDSTATC     0x0032 // Read Status Register Group C
#define RDSTATD     0x0033 // Read Status Register Group D
#define RDSTATE     0x0034 // Read Status Register Group E
#define WRPWMA      0x0020 // Write PWM Register Group A
#define RDPWMA      0x0022 // Read PWM Register Group A
#define WRPWMB      0x0021 // Write PWM Register Group B
#define RDPWMB      0x0023 // Read PWM Register Group B
#define CMDIS       0x0040 // LPCM Disable
#define CMEN        0x0041 // LPCM Enable
#define CMHB2       0x0043 // LPCM Heartbeat
#define WRCMCFG     0x0058 // Write LPCM Configuration Register
#define RDCMCFG     0x0059 // Read LPCM Configuration Register
#define WRCMCELLT   0x005A // Write LPCM Cell Threshold
#define RDCMCELLT   0x005B // Read LPCM Cell Threshold
#define WRCMGPIOT   0x005C // Write LPCM GPIO Threshold
#define RDCMGPIOT   0x005D // Read LPCM GPIO Threshold
#define CLRCMFLAG   0x005E // Clear LPCM Flags
#define RDCMFLAG    0x005F // Read LPCM Flags
#define ADCV        0x0260 // Start Cell Voltage ADC Conversion and Poll Status
#define ADSV        0x0168 // Start S-ADC Conversion and Poll Status
#define ADAX        0x0410 // Start AUX ADC Conversions and Poll Status
#define ADAX2       0x0400 // Start AUX2 ADC Conversions and Poll Status
#define CLRCELL     0x0711 // Clear Cell Voltage Register Groups
#define CLRFC       0x0714 // Clear Filtered Cell Voltage Register Groups
#define CLRAUX      0x0712 // Clear Auxiliary Register Groups
#define CLRSPIN     0x0716 // Clear S-Voltage Register Groups
#define CLRFLAG     0x0717 // Clear Flags
#define CLOVUV      0x0715 // Clear OVUV
#define WRCOMM      0x0721 // Write COMM Register Group
#define RDCOMM      0x0722 // Read COMM Register Group
#define STCOMM      0x0723 // Start I2C/SPI Communication
#define MUTE        0x0028 // Mute Discharge
#define UNMUTE      0x0029 // Unmute Discharge
#define RDSID       0x002C // Read Serial ID Register Group
#define RSTCC       0x002E // Reset Command Counter
#define SNAP        0x002D // Snapshot
#define UNSNAP      0x002F // Release Snapshot
#define SRST        0x0027 // Soft Reset
#define ULRR        0x0038 // Unlock Retention Register
#define WRRR        0x0039 // Write Retention Registers
#define RDRR        0x003A // Read Retention Registers

#define ADV         0x0430
#define ADX         0x0530

/* END ADBMS Register addresses */

#define NUM_CELLV_REGISTERS 6
#define NUM_AUXV_REGISTERS  4
#define NUM_CLEAR_COMMANDS  4

#define CELLS_PER_REG       3
#define CELL_REG_SIZE       (REGISTER_SIZE_BYTES / CELLS_PER_REG)

#define VBATT_DATA_PER_REG  3
#define VBATT_DATA_SIZE     (REGISTER_SIZE_BYTES / VBATT_DATA_PER_REG)

#define ENERGY_DATA_PER_REG 2
#define ENERGY_DATA_SIZE    (REGISTER_SIZE_BYTES / ENERGY_DATA_PER_REG)

#define CELL_ADC_GAIN           0.00015f
#define CELL_ADC_OFFSET         1.5f

#define AUX_ADC_GAIN            0.00015f
#define AUX_ADC_OFFSET          1.5f

#define VB_ADC1_GAIN            0.0001f
#define VB_ADC2_GAIN            0.000085f

#define NUM_CELLV_REGISTERS 6
#define NUM_AUXV_REGISTERS  4

#define MAX_CELLV_SENSOR_VALUE  6.41505f
#define MIN_CELLV_SENSOR_VALUE  -3.4152f

#define PWM_CONFIG_SIZE_BITS    4
#define PWM_CONFIG_RANGE        15
#define NUM_PWM_PER_BYTE        (BITS_IN_BYTE / PWM_CONFIG_SIZE_BITS)

#define NUM_CELLS_PWM_A         12
#define NUM_CELLS_PWM_B         4
#define NUM_BYTES_PWM_B         ((NUM_CELLS_PWM_B * PWM_CONFIG_SIZE_BITS) / BITS_IN_BYTE)

// Configuration register group A encoding
#define REFON_BIT       7
#define CTH_MASK        0x7
#define SOAKON_BIT      7
#define SOAK_TIME_BIT   3
#define SOAK_TIME_MASK  0x0F
#define GPO10_BIT       1
#define SNAP_ST_BIT     5
#define MUTE_ST_BIT     4
#define COMM_BK_BIT     3
#define FC_MASK         0x07

/* ==================================================================== */
/* ============================== MACROS ============================== */
/* ==================================================================== */

#define EXTRACT_16_BIT(buffer)      (((uint32_t)buffer[1] << (1 * BITS_IN_BYTE)) | ((uint32_t)buffer[0]))

#define EXTRACT_24_BIT(buffer)      (((uint32_t)buffer[2] << (2 * BITS_IN_BYTE)) | ((uint32_t)buffer[1] << (1 * BITS_IN_BYTE)) | ((uint32_t)buffer[0]))

#define CONVERT_CELL_ADC_V(reg)     (((int16_t)(EXTRACT_16_BIT(reg)) * CELL_ADC_GAIN) + CELL_ADC_OFFSET)

#define CONVERT_AUX_ADC_V(reg)      (((int16_t)(EXTRACT_16_BIT(reg)) * AUX_ADC_GAIN) + AUX_ADC_OFFSET)

#define CONVERT_PACK_ADC1_V(reg)    ((int16_t)(EXTRACT_16_BIT(reg)) * VB_ADC1_GAIN)

#define CONVERT_PACK_ADC2_V(reg)    ((int16_t)(EXTRACT_16_BIT(reg)) * VB_ADC2_GAIN * -1.0f)

#define CONVERT_I_ADC1_UV(reg)      (((int32_t)(EXTRACT_24_BIT(reg) << BITS_IN_BYTE)) /  BYTE_SIZE_DEC)

#define CONVERT_I_ADC2_UV(reg)      ((((int32_t)(EXTRACT_24_BIT(reg) << BITS_IN_BYTE)) /  BYTE_SIZE_DEC) * -1)

#define SCALE_PERCENT_TO_RANGE(percent, range)      ((uint8_t)(((percent * range) + 50) / 100))

#define SCALE_RANGE_TO_PERCENT(val, range)          ((uint8_t)((val * 100) / range))

/* ==================================================================== */
/* ========================= LOCAL VARIABLES ========================== */
/* ==================================================================== */

static uint8_t transactionBuffer[MAX_SPI_BUFFER];
static uint8_t packMonitorTempBuffer[NUM_CELLV_REGISTERS][REGISTER_SIZE_BYTES];

const cellVoltageRegister[NUM_CELL_VOLTAGE_TYPES][NUM_CELLV_REGISTERS] =
{
    {RDCVA, RDCVB, RDCVC, RDCVD, RDCVE, RDCVF},
    {RDACA, RDACB, RDACC, RDACD, RDACE, RDACF},
    {RDFCA, RDFCB, RDFCC, RDFCD, RDFCE, RDFCF},
    {RDSVA, RDSVB, RDSVC, RDSVD, RDSVE, RDSVF}
};

const auxVoltageRegister[NUM_AUX_VOLTAGE_TYPES][NUM_AUXV_REGISTERS] =
{
    {RDAUXA, RDAUXB, RDAUXC, RDAUXD},
    {RDRAXA, RDRAXB, RDRAXC, RDRAXD}
};

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DEFINITIONS ==================== */
/* ==================================================================== */


TRANSACTION_STATUS_E startCellConversions(ADBMS_BatteryData * adbmsData, ADC_MODE_REDUNDANT_E redundantMode, ADC_MODE_CONTINOUS_E continousMode, ADC_MODE_DISCHARGE_E dischargeMode, ADC_MODE_FILTER_E filterMode, ADC_MODE_CELL_OPEN_WIRE_E openWireMode)
{
    return commandChain((uint16_t)(ADCV | redundantMode | continousMode | dischargeMode | filterMode | openWireMode), &adbmsData->chainInfo, SHARED_COMMAND);
}

TRANSACTION_STATUS_E startRedundantCellConversions(ADBMS_BatteryData * adbmsData, ADC_MODE_CONTINOUS_E continousMode, ADC_MODE_DISCHARGE_E dischargeMode, ADC_MODE_CELL_OPEN_WIRE_E openWireMode)
{
    return commandChain((uint16_t)(ADSV | continousMode | dischargeMode | openWireMode), &adbmsData->chainInfo, SHARED_COMMAND);
}

TRANSACTION_STATUS_E startAuxConversions(ADBMS_BatteryData * adbmsData, ADC_MODE_AUX_CHANNEL_E auxChannel, ADC_MODE_AUX_OPEN_WIRE_E openWireMode)
{
    return commandChain((uint16_t)(ADAX | auxChannel | openWireMode), &adbmsData->chainInfo, CELL_MONITOR_COMMAND);
}

TRANSACTION_STATUS_E startRedundantAuxConversions(ADBMS_BatteryData * adbmsData, ADC_MODE_AUX_CHANNEL_E auxChannel)
{
    return commandChain((uint16_t)(ADAX2 | auxChannel), &adbmsData->chainInfo, CELL_MONITOR_COMMAND);
}

TRANSACTION_STATUS_E startPackVoltageConversions(ADBMS_BatteryData * adbmsData, ADC_MODE_PACK_CHANNEL_E packChannel, ADC_MODE_PACK_OPEN_WIRE_E openWireMode)
{
    return commandChain((uint16_t)(ADV | packChannel | openWireMode), &adbmsData->chainInfo, PACK_MONITOR_COMMAND);
}

TRANSACTION_STATUS_E startPackAuxillaryConversions(ADBMS_BatteryData * adbmsData)
{
    return commandChain((uint16_t)(ADX), &adbmsData->chainInfo, PACK_MONITOR_COMMAND);
}

TRANSACTION_STATUS_E muteDischarge(ADBMS_BatteryData * adbmsData)
{
    return commandChain(MUTE, &adbmsData->chainInfo, SHARED_COMMAND);
}

TRANSACTION_STATUS_E unmuteDischarge(ADBMS_BatteryData * adbmsData)
{
    return commandChain(UNMUTE, &adbmsData->chainInfo, SHARED_COMMAND);
}

TRANSACTION_STATUS_E freezeRegisters(ADBMS_BatteryData * adbmsData)
{
    return commandChain(SNAP, &adbmsData->chainInfo, SHARED_COMMAND);
}

TRANSACTION_STATUS_E unfreezeRegisters(ADBMS_BatteryData * adbmsData)
{
    return commandChain(UNSNAP, &adbmsData->chainInfo, SHARED_COMMAND);
}

TRANSACTION_STATUS_E softReset(ADBMS_BatteryData * adbmsData)
{
    return commandChain(SRST, &adbmsData->chainInfo, SHARED_COMMAND);
}




// TRANSACTION_STATUS_E clearAllVoltageRegisters(ADBMS_BatteryData * adbmsData)
// {
//     TRANSACTION_STATUS_E status;

//     uint16_t clearCommands[NUM_CLEAR_COMMANDS] = {CLRCELL, CLRFC, CLRAUX, CLRSPIN};

//     for(uint32_t i = 0; i < (NUM_CLEAR_COMMANDS-1); i++)
//     {
//         status = commandChain(clearCommands[i], &adbmsData->chainInfo, SHARED_COMMAND);
//         if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
//         {
//             return status;
//         }
//     }

//     return commandChain(clearCommands[NUM_CLEAR_COMMANDS-1], &adbmsData->chainInfo, CELL_MONITOR_COMMAND);
// }

// TRANSACTION_STATUS_E clearAllFlags(ADBMS_BatteryData * adbmsData)
// {
//     TRANSACTION_STATUS_E status;

//     // Create and fill pack monitor data buffer with 1s
//     uint8_t packMonitorDataBuffer[REGISTER_SIZE_BYTES];
//     memset(packMonitorDataBuffer, 0xFF, REGISTER_SIZE_BYTES);

//     // Create and fill cell monitor data buffer with 1s
//     uint8_t cellMonitorDataBuffer[REGISTER_SIZE_BYTES * (chainInfo->numDevs - 1)];
//     memset(cellMonitorDataBuffer, 0xFF, REGISTER_SIZE_BYTES * (chainInfo->numDevs - 1));

//     status = writeChain(CLRFLAG, chainInfo, SHARED_COMMAND, packMonitorDataBuffer, cellMonitorDataBuffer);
//     if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
//     {
//         return status;
//     }

//     return writeChain(CLOVUV, chainInfo, SHARED_COMMAND, packMonitorDataBuffer, cellMonitorDataBuffer);
// }





TRANSACTION_STATUS_E writePwmRegisters(ADBMS_BatteryData * adbmsData)
{
    uint8_t *cellMonitorDataBuffer;
    if(adbmsData->chainInfo.packMonitorPort == PORTA)
    {
        cellMonitorDataBuffer = transactionBuffer + REGISTER_SIZE_BYTES;
    }
    else
    {
        cellMonitorDataBuffer = transactionBuffer;
    }

    for(uint32_t i = 0; i < (adbmsData->chainInfo.numDevs - 1); i++)
    {
        for(uint32_t j = 0; j < REGISTER_SIZE_BYTES; j++)
        {
            uint8_t pwmSetting0 = SCALE_PERCENT_TO_RANGE(adbmsData->cellMonitor[i].dischargePWM[j * 2], PWM_CONFIG_RANGE);
            uint8_t pwmSetting1 = SCALE_PERCENT_TO_RANGE(adbmsData->cellMonitor[i].dischargePWM[(j * 2) + 1], PWM_CONFIG_RANGE);

            cellMonitorDataBuffer[(i * REGISTER_SIZE_BYTES) + j] = ((pwmSetting1 << PWM_CONFIG_SIZE_BITS) | pwmSetting0);
        }
    }

    TRANSACTION_STATUS_E status = writeChain(WRPWMA, &adbmsData->chainInfo, SHARED_COMMAND, transactionBuffer);
    if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    {
        return status;
    }

    for(uint32_t i = 0; i < (adbmsData->chainInfo.numDevs - 1); i++)
    {
        for(uint32_t j = 0; j < NUM_BYTES_PWM_B; j++)
        {
            uint8_t pwmSetting0 = SCALE_PERCENT_TO_RANGE(adbmsData->cellMonitor[i].dischargePWM[NUM_CELLS_PWM_A + (j * 2)], PWM_CONFIG_RANGE);
            uint8_t pwmSetting1 = SCALE_PERCENT_TO_RANGE(adbmsData->cellMonitor[i].dischargePWM[NUM_CELLS_PWM_A + (j * 2) + 1], PWM_CONFIG_RANGE);

            cellMonitorDataBuffer[(i * REGISTER_SIZE_BYTES) + j] = ((pwmSetting1 << PWM_CONFIG_SIZE_BITS) | pwmSetting0);
        }
    }

    return writeChain(WRPWMB, &adbmsData->chainInfo, SHARED_COMMAND, transactionBuffer);
}

TRANSACTION_STATUS_E readPwmRegisters(ADBMS_BatteryData * adbmsData)
{
    uint8_t *cellMonitorDataBuffer;
    if(adbmsData->chainInfo.packMonitorPort == PORTA)
    {
        cellMonitorDataBuffer = transactionBuffer + REGISTER_SIZE_BYTES;
    }
    else
    {
        cellMonitorDataBuffer = transactionBuffer;
    }

    TRANSACTION_STATUS_E status = readChain(RDPWMA, &adbmsData->chainInfo, transactionBuffer);
    if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    {
        return status;
    }

    for(uint32_t i = 0; i < (adbmsData->chainInfo.numDevs - 1); i++)
    {
        for(uint32_t j = 0; j < REGISTER_SIZE_BYTES; j++)
        {
            uint8_t pwmSetting0 = (cellMonitorDataBuffer[(i * REGISTER_SIZE_BYTES) + j] & 0x0F);
            uint8_t pwmSetting1 = (cellMonitorDataBuffer[(i * REGISTER_SIZE_BYTES) + j] >> PWM_CONFIG_SIZE_BITS);

            adbmsData->cellMonitor[i].dischargePWM[j * 2] = SCALE_RANGE_TO_PERCENT(pwmSetting0, PWM_CONFIG_RANGE);
            adbmsData->cellMonitor[i].dischargePWM[(j * 2) + 1] = SCALE_RANGE_TO_PERCENT(pwmSetting1, PWM_CONFIG_RANGE);
        }
    }

    status = readChain(RDPWMB, &adbmsData->chainInfo, transactionBuffer);
    if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    {
        return status;
    }

    for(uint32_t i = 0; i < (adbmsData->chainInfo.numDevs - 1); i++)
    {
        for(uint32_t j = 0; j < NUM_BYTES_PWM_B; j++)
        {
            uint8_t pwmSetting0 = (cellMonitorDataBuffer[(i * REGISTER_SIZE_BYTES) + j] & 0x0F);
            uint8_t pwmSetting1 = (cellMonitorDataBuffer[(i * REGISTER_SIZE_BYTES) + j] >> PWM_CONFIG_SIZE_BITS);

            adbmsData->cellMonitor[i].dischargePWM[NUM_CELLS_PWM_A + (j * 2)] = SCALE_RANGE_TO_PERCENT(pwmSetting0, PWM_CONFIG_RANGE);
            adbmsData->cellMonitor[i].dischargePWM[NUM_CELLS_PWM_A + (j * 2) + 1] = SCALE_RANGE_TO_PERCENT(pwmSetting1, PWM_CONFIG_RANGE);
        }
    }

    return status;
}


// TODO Test if write works through Pack Monitor
// Make sure followed with read to check data and lock register
TRANSACTION_STATUS_E writeNVM(ADBMS_BatteryData * adbmsData)
{
    TRANSACTION_STATUS_E status = commandChain(ULRR, &adbmsData->chainInfo, CELL_MONITOR_COMMAND);
    if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    {
        return status;
    }

    uint8_t *cellMonitorDataBuffer;
    if(adbmsData->chainInfo.packMonitorPort == PORTA)
    {
        cellMonitorDataBuffer = transactionBuffer + REGISTER_SIZE_BYTES;
    }
    else
    {
        cellMonitorDataBuffer = transactionBuffer;
    }

    for(uint32_t i = 0; i < (adbmsData->chainInfo.numDevs - 1); i++)
    {
        memcpy(cellMonitorDataBuffer + (i * REGISTER_SIZE_BYTES), adbmsData->cellMonitor[i].retentionRegister, REGISTER_SIZE_BYTES);
    }

    return writeChain(WRRR, &adbmsData->chainInfo, CELL_MONITOR_COMMAND, transactionBuffer);
}

TRANSACTION_STATUS_E readNVM(ADBMS_BatteryData * adbmsData, uint8_t *readData)
{
    uint8_t *cellMonitorDataBuffer;
    if(adbmsData->chainInfo.packMonitorPort == PORTA)
    {
        cellMonitorDataBuffer = transactionBuffer + REGISTER_SIZE_BYTES;
    }
    else
    {
        cellMonitorDataBuffer = transactionBuffer;
    }

    TRANSACTION_STATUS_E status = readChain(RDRR, &adbmsData->chainInfo, transactionBuffer);
    if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    {
        return status;
    }

    for(uint32_t i = 0; i < (adbmsData->chainInfo.numDevs - 1); i++)
    {
        memcpy(adbmsData->cellMonitor[i].retentionRegister, cellMonitorDataBuffer + (i * REGISTER_SIZE_BYTES), REGISTER_SIZE_BYTES);
    }
}







TRANSACTION_STATUS_E writeConfigA(ADBMS_BatteryData * adbmsData)
{
    uint8_t *cellMonitorDataBuffer;
    if(adbmsData->chainInfo.packMonitorPort == PORTA)
    {
        memcpy(transactionBuffer, &adbmsData->packMonitor.configGroupA, REGISTER_SIZE_BYTES);
        cellMonitorDataBuffer = transactionBuffer + REGISTER_SIZE_BYTES;
    }
    else
    {
        memcpy(transactionBuffer + ((adbmsData->chainInfo.numDevs - 1) * REGISTER_SIZE_BYTES), &adbmsData->packMonitor.configGroupA, REGISTER_SIZE_BYTES);
        cellMonitorDataBuffer = transactionBuffer;
    }

    for(uint32_t i = 0; i < (adbmsData->chainInfo.numDevs - 1); i++)
    {
        memcpy(cellMonitorDataBuffer + (i * REGISTER_SIZE_BYTES), &adbmsData->cellMonitor[i].configGroupA, REGISTER_SIZE_BYTES);
    }

    return writeChain(WRCFGA, &adbmsData->chainInfo, SHARED_COMMAND, transactionBuffer);

}

TRANSACTION_STATUS_E readConfigA(ADBMS_BatteryData * adbmsData)
{
    TRANSACTION_STATUS_E status = readChain(RDCFGA, &adbmsData->chainInfo, transactionBuffer);
    if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    {
        return status;
    }

    uint8_t *cellMonitorDataBuffer;
    if(adbmsData->chainInfo.packMonitorPort == PORTA)
    {
        memcpy(&adbmsData->packMonitor.configGroupA, transactionBuffer, REGISTER_SIZE_BYTES);
        cellMonitorDataBuffer = transactionBuffer + REGISTER_SIZE_BYTES;
    }
    else
    {
        memcpy(&adbmsData->packMonitor.configGroupA, transactionBuffer + ((adbmsData->chainInfo.numDevs - 1) * REGISTER_SIZE_BYTES), REGISTER_SIZE_BYTES);
        cellMonitorDataBuffer = transactionBuffer;
    }

    for(uint32_t i = 0; i < (adbmsData->chainInfo.numDevs - 1); i++)
    {
        memcpy(&adbmsData->cellMonitor[i].configGroupA, cellMonitorDataBuffer + (i * REGISTER_SIZE_BYTES), REGISTER_SIZE_BYTES);
    }

    return status;
}

TRANSACTION_STATUS_E writeConfigB(ADBMS_BatteryData * adbmsData)
{
    uint8_t *cellMonitorDataBuffer;
    if(adbmsData->chainInfo.packMonitorPort == PORTA)
    {
        memcpy(transactionBuffer, &adbmsData->packMonitor.configGroupB, REGISTER_SIZE_BYTES);
        cellMonitorDataBuffer = transactionBuffer + REGISTER_SIZE_BYTES;
    }
    else
    {
        memcpy(transactionBuffer + ((adbmsData->chainInfo.numDevs - 1) * REGISTER_SIZE_BYTES), &adbmsData->packMonitor.configGroupB, REGISTER_SIZE_BYTES);
        cellMonitorDataBuffer = transactionBuffer;
    }

    for(uint32_t i = 0; i < (adbmsData->chainInfo.numDevs - 1); i++)
    {
        memcpy(cellMonitorDataBuffer + (i * REGISTER_SIZE_BYTES), &adbmsData->cellMonitor[i].configGroupB, REGISTER_SIZE_BYTES);
    }

    return writeChain(WRCFGB, &adbmsData->chainInfo, SHARED_COMMAND, transactionBuffer);

}

TRANSACTION_STATUS_E readConfigB(ADBMS_BatteryData * adbmsData)
{
    TRANSACTION_STATUS_E status = readChain(RDCFGB, &adbmsData->chainInfo, transactionBuffer);
    if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    {
        return status;
    }

    uint8_t *cellMonitorDataBuffer;
    if(adbmsData->chainInfo.packMonitorPort == PORTA)
    {
        memcpy(&adbmsData->packMonitor.configGroupB, transactionBuffer, REGISTER_SIZE_BYTES);
        cellMonitorDataBuffer = transactionBuffer + REGISTER_SIZE_BYTES;
    }
    else
    {
        memcpy(&adbmsData->packMonitor.configGroupB, transactionBuffer + ((adbmsData->chainInfo.numDevs - 1) * REGISTER_SIZE_BYTES), REGISTER_SIZE_BYTES);
        cellMonitorDataBuffer = transactionBuffer;
    }

    for(uint32_t i = 0; i < (adbmsData->chainInfo.numDevs - 1); i++)
    {
        memcpy(&adbmsData->cellMonitor[i].configGroupB, cellMonitorDataBuffer + (i * REGISTER_SIZE_BYTES), REGISTER_SIZE_BYTES);
    }

    return status;
}

TRANSACTION_STATUS_E readCellVoltages(ADBMS_BatteryData * adbmsData, CELL_VOLTAGE_TYPE_E cellVoltageType)
{
    uint8_t *packMonitorDataBuffer;
    uint8_t *cellMonitorDataBuffer;
    if(adbmsData->chainInfo.packMonitorPort == PORTA)
    {
        packMonitorDataBuffer = transactionBuffer;
        cellMonitorDataBuffer = transactionBuffer + REGISTER_SIZE_BYTES;
    }
    else
    {
        cellMonitorDataBuffer = transactionBuffer;
        packMonitorDataBuffer = transactionBuffer + ((adbmsData->chainInfo.numDevs - 1) * REGISTER_SIZE_BYTES);
    }

    TRANSACTION_STATUS_E status;
    for(uint32_t i = 0; i < (NUM_CELLV_REGISTERS - 1); i++)
    {
        status = readChain(cellVoltageRegister[cellVoltageType][i], &adbmsData->chainInfo, transactionBuffer);
        if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
        {
            return status;
        }

        memcpy(packMonitorTempBuffer[i], packMonitorDataBuffer, REGISTER_SIZE_BYTES);

        for(uint32_t j = 0; j < (adbmsData->chainInfo.numDevs - 1); j++)
        {
            for(uint32_t k = 0; k < CELLS_PER_REG; k++)
            {
                adbmsData->cellMonitor[j].cellVoltage[(i * CELLS_PER_REG) + k] = CONVERT_CELL_ADC_V((cellMonitorDataBuffer + (j * REGISTER_SIZE_BYTES) + (k * CELL_REG_SIZE)));
            }
        }
    }

    status = readChain(cellVoltageRegister[cellVoltageType][NUM_CELLV_REGISTERS - 1], &adbmsData->chainInfo, transactionBuffer);
    if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    {
        return status;
    }

    memcpy(packMonitorTempBuffer[NUM_CELLV_REGISTERS - 1], packMonitorDataBuffer, REGISTER_SIZE_BYTES);

    for(uint32_t j = 0; j < (adbmsData->chainInfo.numDevs - 1); j++)
    {
        adbmsData->cellMonitor[j].cellVoltage[(NUM_CELLV_REGISTERS - 1) * CELLS_PER_REG] = CONVERT_CELL_ADC_V((cellMonitorDataBuffer + (j * REGISTER_SIZE_BYTES)));
    }

    switch (cellVoltageType)
    {
        case RAW_CELL_VOLTAGE:
        case FILTERED_CELL_VOLTAGE:
            break;

        case AVERAGED_CELL_VOLTAGE:
            /* code */
            break;

        case REDUNDANT_CELL_VOLTAGE:
            /* code */
            break;

        default:
            break;
    }

    return status;
}

TRANSACTION_STATUS_E readAuxVoltages(ADBMS_BatteryData * adbmsData, AUX_VOLTAGE_TYPE_E auxVoltageType)
{
    uint8_t *packMonitorDataBuffer;
    uint8_t *cellMonitorDataBuffer;
    if(adbmsData->chainInfo.packMonitorPort == PORTA)
    {
        packMonitorDataBuffer = transactionBuffer;
        cellMonitorDataBuffer = transactionBuffer + REGISTER_SIZE_BYTES;
    }
    else
    {
        cellMonitorDataBuffer = transactionBuffer;
        packMonitorDataBuffer = transactionBuffer + ((adbmsData->chainInfo.numDevs - 1) * REGISTER_SIZE_BYTES);
    }

    TRANSACTION_STATUS_E status;
    for(uint32_t i = 0; i < (NUM_AUXV_REGISTERS - 1); i++)
    {
        status = readChain(auxVoltageRegister[auxVoltageType][i], &adbmsData->chainInfo, transactionBuffer);
        if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
        {
            return status;
        }

        memcpy(packMonitorTempBuffer[i], packMonitorDataBuffer, REGISTER_SIZE_BYTES);

        for(uint32_t j = 0; j < (adbmsData->chainInfo.numDevs - 1); j++)
        {
            for(uint32_t k = 0; k < CELLS_PER_REG; k++)
            {
                adbmsData->cellMonitor[j].auxVoltage[(i * CELLS_PER_REG) + k] = CONVERT_AUX_ADC_V((cellMonitorDataBuffer + (j * REGISTER_SIZE_BYTES) + (k * CELL_REG_SIZE)));
            }
        }
    }

    status = readChain(auxVoltageRegister[auxVoltageType][NUM_AUXV_REGISTERS - 1], &adbmsData->chainInfo, transactionBuffer);
    if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    {
        return status;
    }

    memcpy(packMonitorTempBuffer[NUM_AUXV_REGISTERS - 1], packMonitorDataBuffer, REGISTER_SIZE_BYTES);

    for(uint32_t j = 0; j < (adbmsData->chainInfo.numDevs - 1); j++)
    {
        adbmsData->cellMonitor[j].cellVoltage[(NUM_AUXV_REGISTERS - 1) * CELLS_PER_REG] = CONVERT_AUX_ADC_V((cellMonitorDataBuffer + (j * REGISTER_SIZE_BYTES)));
    }

    switch (auxVoltageType)
    {
        case AUX_VOLTAGES:
            break;

        case REDUNDANT_AUX_VOLTAGES:
            /* code */
            break;

        default:
            break;
    }

    return status;
}
