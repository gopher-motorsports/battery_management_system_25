/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */
#include "adbms/isospi.h"
#include "adbms/adbms.h"
#include <string.h>
#include <math.h>

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
#define CLRO        0x0713

/* END ADBMS Register addresses */

// Size of command groups
#define NUM_CELLV_REGISTERS 6
#define NUM_AUXV_REGISTERS  4
#define NUM_CLEAR_COMMANDS  4

// ADC result register sizes
#define VOLTAGE_16BIT_SIZE_BYTES    2
#define VOLTAGE_16BIT_PER_REG       (REGISTER_SIZE_BYTES / VOLTAGE_16BIT_SIZE_BYTES)

#define VOLTAGE_24BIT_SIZE_BYTES    3
#define VOLTAGE_24BIT_PER_REG       (REGISTER_SIZE_BYTES / VOLTAGE_16BIT_SIZE_BYTES)

// ADC result register encoding
#define CELL_MON_OV_UV_GAIN             0.0024f
#define CELL_MON_OV_UV_OFFSET           1.5f

#define CELL_MON_DV_GAIN                0.0012f
#define CELL_MON_DV_OFFSET              1.5f

#define CELL_MON_HV_SUPPLY_GAIN         0.00375f
#define CELL_MON_HV_SUPPLY_OFFSET       0.00375f

#define CELL_MON_DIE_TEMP_GAIN          0.02f
#define CELL_MON_DIE_TEMP_OFFSET        -73.0f

#define PACK_MON_VADC1_GAIN             0.0001f
#define PACK_MON_VADC1_OFFSET           0.0f

#define PACK_MON_VADC2_GAIN             -0.000085f
#define PACK_MON_VADC2_OFFSET           0.0f

#define PACK_MON_VACC1_GAIN_UV          100
#define PACK_MON_VACC1_OFFSET_UV        0.0f

#define PACK_MON_VACC2_GAIN_UV          -85
#define PACK_MON_VACC2_OFFSET_UV        0

#define PACK_MON_IADC1_GAIN_UV          1
#define PACK_MON_IADC1_OFFSET_UV        0

#define PACK_MON_IADC2_GAIN_UV          -1
#define PACK_MON_IADC2_OFFSET_UV        0

#define PACK_MON_VREF2A_GAIN            0.00024f
#define PACK_MON_VREF2A_OFFSET          0.0f

#define PACK_MON_VREF2B_GAIN            -0.000204f
#define PACK_MON_VREF2B_OFFSET          0.0f

#define PACK_MON_VREF1P25_GAIN          0.0001f
#define PACK_MON_VREF1P25_OFFSET        0.0f

#define PACK_MON_VDIV_GAIN              0.0001f
#define PACK_MON_VDIV_OFFSET            0.0f

#define PACK_MON_VREG_GAIN              0.00024f
#define PACK_MON_VREG_OFFSET            0.0f

#define PACK_MON_VDD_GAIN               0.001f
#define PACK_MON_VDD_OFFSET             0.0f

#define PACK_MON_VDIG_GAIN              0.00024f
#define PACK_MON_VDIG_OFFSET            0.0f

#define PACK_MON_EPAD_GAIN              0.0001f
#define PACK_MON_EPAD_OFFSET            0.0f

#define PACK_MON_DIE_TEMP1_GAIN         0.01618f
#define PACK_MON_DIE_TEMP1_OFFSET       -250.0f

#define PACK_MON_DIE_TEMP2_GAIN         0.04878f
#define PACK_MON_DIE_TEMP2_OFFSET       -267.0f

#define NUM_CELLV_REGISTERS 6
#define NUM_AUXV_REGISTERS  4

#define MAX_OV_UV_VALUE         6.4128f
#define MIN_OV_UV_VALUE         -3.4152f

#define CELL_OV_UV_MASK         0x00000FFF
#define CELL_OV_UV_BITS         12

#define DTM_ENABLE              0x80
#define DTM_LONG_RANGE_ENABLE   0x40
#define DTM_TIME_MASK           0x3F

#define DTM_SHORT_RANGE_MAX     63
#define DTM_LONG_RANGE_MAX      1008
#define DTM_LONG_RANGE_STEP     16

#define PWM_CONFIG_SIZE_BITS    4
#define PWM_CONFIG_SIZE_MASK    0x0F
#define PWM_CONFIG_RANGE        15
#define PWM_PERCENT_RANGE       100.0f
#define PWM_SETTING_GAIN        (PWM_PERCENT_RANGE / PWM_CONFIG_RANGE)
#define PWM_SETTING_OFFSET      0

#define NUM_CELLS_PWM_A         12
#define NUM_CELLS_PWM_B         4
#define NUM_BYTES_PWM_B         ((NUM_CELLS_PWM_B * PWM_CONFIG_SIZE_BITS) / BITS_IN_BYTE)

#define OVERCURRENT_GAIN1       5.0f
#define OVERCURRENT_GAIN2       2.5f

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

#define PACK_MON_COUNTER2_BIT       5
#define PACK_MON_COUNTER1_MASK      0x1F

// Time for ADBMS device to wake
#define TIME_WAKE_US            500

// Time for ADBMS device to transition from idle state
#define TIME_READY_US           10

/* ==================================================================== */
/* ============================== MACROS ============================== */
/* ==================================================================== */

#define EXTRACT_16_BIT(buffer)                          (((uint32_t)buffer[1] << (1 * BITS_IN_BYTE)) | ((uint32_t)buffer[0]))

#define EXTRACT_24_BIT(buffer)                          (((uint32_t)buffer[2] << (2 * BITS_IN_BYTE)) | ((uint32_t)buffer[1] << (1 * BITS_IN_BYTE)) | ((uint32_t)buffer[0]))

#define CONVERT_SIGNED_12_BIT_REGISTER(reg, gain, offset)    (((((int16_t)(reg << 4)) / 16) * gain) + offset)

#define CONVERT_SIGNED_16_BIT_REGISTER(reg, gain, offset)    (((int16_t)(EXTRACT_16_BIT(reg)) * gain) + offset)

#define CONVERT_SIGNED_24_BIT_REGISTER_UV(reg, gain)           ((((int32_t)(EXTRACT_24_BIT(reg) << BITS_IN_BYTE)) /  BYTE_SIZE_DEC) * gain)

#define CONVERT_FLOAT_TO_REGISTER(val, gain, offset)    (int32_t)roundf((val - offset) / gain)

/* ==================================================================== */
/* ========================= LOCAL VARIABLES ========================== */
/* ==================================================================== */

static uint8_t transactionBuffer[MAX_SPI_BUFFER];

static const uint16_t cellVoltageCode[NUM_CELL_VOLTAGE_TYPES][NUM_CELLV_REGISTERS] =
{
    {RDCVA, RDCVB, RDCVC, RDCVD, RDCVE, RDCVF},
    {RDACA, RDACB, RDACC, RDACD, RDACE, RDACF},
    {RDFCA, RDFCB, RDFCC, RDFCD, RDFCE, RDFCF}
};

static const uint16_t redundantCellVoltageCode[NUM_CELLV_REGISTERS] =
{
    RDSVA, RDSVB, RDSVC, RDSVD, RDSVE, RDSVF
};

static const uint16_t auxVoltageCode[NUM_AUXV_REGISTERS] =
{
    RDAUXA, RDAUXB, RDAUXC, RDAUXD
};

static const uint16_t redundantAuxVoltageCode[NUM_AUXV_REGISTERS] =
{
    RDAUXA, RDAUXB, RDAUXC, RDAUXD
};

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DEFINITIONS ==================== */
/* ==================================================================== */

void wakeChain(ADBMS_BatteryData * adbmsData)
{
    if(adbmsData->chainInfo.chainStatus == CHAIN_COMPLETE)
    {
        activatePort(adbmsData->chainInfo.numDevs, adbmsData->chainInfo.currentPort, TIME_WAKE_US);
    }
    else
    {
        activatePort(adbmsData->chainInfo.availableDevices[PORTA], PORTA, TIME_WAKE_US);
        activatePort(adbmsData->chainInfo.availableDevices[PORTB], PORTB, TIME_WAKE_US);
    }
}

void readyChain(ADBMS_BatteryData * adbmsData)
{
    if(adbmsData->chainInfo.chainStatus == CHAIN_COMPLETE)
    {
        activatePort(adbmsData->chainInfo.numDevs, adbmsData->chainInfo.currentPort, TIME_READY_US);
    }
    else
    {
        activatePort(adbmsData->chainInfo.availableDevices[PORTA], PORTA, TIME_READY_US);
        activatePort(adbmsData->chainInfo.availableDevices[PORTB], PORTB, TIME_READY_US);
    }
}

TRANSACTION_STATUS_E checkChainStatus(ADBMS_BatteryData * adbmsData)
{
    if(adbmsData->chainInfo.chainStatus != CHAIN_COMPLETE)
    {
        TRANSACTION_STATUS_E chainStatus = updateChainStatus(&adbmsData->chainInfo);
        if(chainStatus != TRANSACTION_COMMAND_COUNTER_ERROR)
        {
            return chainStatus;
        }
    }
    return TRANSACTION_SUCCESS;
}


TRANSACTION_STATUS_E startCellConversions(ADBMS_BatteryData *adbmsData, ADC_MODE_REDUNDANT_E redundantMode, ADC_MODE_CONTINOUS_E continousMode, ADC_MODE_DISCHARGE_E dischargeMode, ADC_MODE_FILTER_E filterMode, ADC_MODE_CELL_OPEN_WIRE_E openWireMode)
{
    return commandChain((uint16_t)(ADCV | redundantMode | continousMode | dischargeMode | filterMode | openWireMode), &adbmsData->chainInfo, SHARED_COMMAND);
}

TRANSACTION_STATUS_E startRedundantCellConversions(ADBMS_BatteryData *adbmsData, ADC_MODE_CONTINOUS_E continousMode, ADC_MODE_DISCHARGE_E dischargeMode, ADC_MODE_CELL_OPEN_WIRE_E openWireMode)
{
    return commandChain((uint16_t)(ADSV | continousMode | dischargeMode | openWireMode), &adbmsData->chainInfo, SHARED_COMMAND);
}

TRANSACTION_STATUS_E startAuxConversions(ADBMS_BatteryData *adbmsData, ADC_MODE_AUX_CHANNEL_E auxChannel, ADC_MODE_AUX_OPEN_WIRE_E openWireMode)
{
    return commandChain((uint16_t)(ADAX | auxChannel | openWireMode), &adbmsData->chainInfo, CELL_MONITOR_COMMAND);
}

TRANSACTION_STATUS_E startRedundantAuxConversions(ADBMS_BatteryData *adbmsData, ADC_MODE_AUX_CHANNEL_E auxChannel)
{
    return commandChain((uint16_t)(ADAX2 | auxChannel), &adbmsData->chainInfo, CELL_MONITOR_COMMAND);
}

TRANSACTION_STATUS_E startPackVoltageConversions(ADBMS_BatteryData *adbmsData, ADC_MODE_PACK_CHANNEL_E packChannel, ADC_MODE_PACK_OPEN_WIRE_E openWireMode)
{
    return commandChain((uint16_t)(ADV | packChannel | openWireMode), &adbmsData->chainInfo, PACK_MONITOR_COMMAND);
}

TRANSACTION_STATUS_E startPackAuxillaryConversions(ADBMS_BatteryData *adbmsData)
{
    return commandChain((uint16_t)(ADX), &adbmsData->chainInfo, PACK_MONITOR_COMMAND);
}

TRANSACTION_STATUS_E muteDischarge(ADBMS_BatteryData *adbmsData)
{
    return commandChain(MUTE, &adbmsData->chainInfo, CELL_MONITOR_COMMAND);
}

TRANSACTION_STATUS_E unmuteDischarge(ADBMS_BatteryData *adbmsData)
{
    return commandChain(UNMUTE, &adbmsData->chainInfo, CELL_MONITOR_COMMAND);
}

TRANSACTION_STATUS_E freezeRegisters(ADBMS_BatteryData *adbmsData)
{
    return commandChain(SNAP, &adbmsData->chainInfo, SHARED_COMMAND);
}

TRANSACTION_STATUS_E unfreezeRegisters(ADBMS_BatteryData *adbmsData)
{
    return commandChain(UNSNAP, &adbmsData->chainInfo, SHARED_COMMAND);
}

TRANSACTION_STATUS_E softReset(ADBMS_BatteryData *adbmsData)
{
    return commandChain(SRST, &adbmsData->chainInfo, SHARED_COMMAND);
}

TRANSACTION_STATUS_E clearAllVoltageRegisters(ADBMS_BatteryData *adbmsData)
{
    TRANSACTION_STATUS_E status = commandChain(CLRCELL, &adbmsData->chainInfo, SHARED_COMMAND);
    if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    {
        return status;
    }

    status = commandChain(CLRFC, &adbmsData->chainInfo, SHARED_COMMAND);
    if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    {
        return status;
    }

    status = commandChain(CLRAUX, &adbmsData->chainInfo, SHARED_COMMAND);
    if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    {
        return status;
    }

    status = commandChain(CLRSPIN, &adbmsData->chainInfo, CELL_MONITOR_COMMAND);
    if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    {
        return status;
    }

    return commandChain(CLRO, &adbmsData->chainInfo, PACK_MONITOR_COMMAND);
}

TRANSACTION_STATUS_E clearAllFlags(ADBMS_BatteryData *adbmsData)
{
    memset(transactionBuffer, 0xFF, (adbmsData->chainInfo.numDevs * REGISTER_SIZE_BYTES));

    TRANSACTION_STATUS_E status = writeChain(CLRFLAG, &adbmsData->chainInfo, SHARED_COMMAND, transactionBuffer);
    if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    {
        return status;
    }

    status = writeChain(CLOVUV, &adbmsData->chainInfo, SHARED_COMMAND, transactionBuffer);
    if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    {
        return status;
    }

    readyChain(adbmsData);

    return status;
}

TRANSACTION_STATUS_E readSerialId(ADBMS_BatteryData *adbmsData)
{
    memset(transactionBuffer, 0x00, adbmsData->chainInfo.numDevs * REGISTER_SIZE_BYTES);

    TRANSACTION_STATUS_E status = readChain(RDSID, &adbmsData->chainInfo, transactionBuffer);

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

    memcpy(&adbmsData->packMonitor.serialId, packMonitorDataBuffer, REGISTER_SIZE_BYTES);

    for(uint32_t i = 0; i < (adbmsData->chainInfo.numDevs - 1); i++)
    {
        memcpy(&adbmsData->cellMonitor[i].serialId, cellMonitorDataBuffer + (i * REGISTER_SIZE_BYTES), REGISTER_BYTE5);
    }

    return status;
}

TRANSACTION_STATUS_E writePwmRegisters(ADBMS_BatteryData *adbmsData)
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
            float pwm0 = adbmsData->cellMonitor[i].dischargePWM[j * 2];
            float pwm1 = adbmsData->cellMonitor[i].dischargePWM[(j * 2) + 1];

            if(pwm0 < 0.0f)
            {
                pwm0 = 0.0f;
            }
            else if(pwm0 > PWM_PERCENT_RANGE)
            {
                pwm0 = PWM_PERCENT_RANGE;
            }

            if(pwm1 < 0.0f)
            {
                pwm1 = 0.0f;
            }
            else if(pwm1 > PWM_PERCENT_RANGE)
            {
                pwm1 = PWM_PERCENT_RANGE;
            }

            uint8_t pwmSetting0 = CONVERT_FLOAT_TO_REGISTER(pwm0, PWM_SETTING_GAIN, PWM_SETTING_OFFSET);
            uint8_t pwmSetting1 = CONVERT_FLOAT_TO_REGISTER(pwm1, PWM_SETTING_GAIN, PWM_SETTING_OFFSET);

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
            float pwm0 = adbmsData->cellMonitor[i].dischargePWM[NUM_CELLS_PWM_A + (j * 2)];
            float pwm1 = adbmsData->cellMonitor[i].dischargePWM[NUM_CELLS_PWM_A + (j * 2) + 1];

            if(pwm0 < 0.0f)
            {
                pwm0 = 0.0f;
            }
            else if(pwm0 > PWM_PERCENT_RANGE)
            {
                pwm0 = PWM_PERCENT_RANGE;
            }

            if(pwm1 < 0.0f)
            {
                pwm1 = 0.0f;
            }
            else if(pwm1 > PWM_PERCENT_RANGE)
            {
                pwm1 = PWM_PERCENT_RANGE;
            }

            uint8_t pwmSetting0 = CONVERT_FLOAT_TO_REGISTER(pwm0, PWM_SETTING_GAIN, PWM_SETTING_OFFSET);
            uint8_t pwmSetting1 = CONVERT_FLOAT_TO_REGISTER(pwm1, PWM_SETTING_GAIN, PWM_SETTING_OFFSET);

            cellMonitorDataBuffer[(i * REGISTER_SIZE_BYTES) + j] = ((pwmSetting1 << PWM_CONFIG_SIZE_BITS) | pwmSetting0);
        }
    }

    return writeChain(WRPWMB, &adbmsData->chainInfo, SHARED_COMMAND, transactionBuffer);
}

TRANSACTION_STATUS_E readPwmRegisters(ADBMS_BatteryData *adbmsData)
{
    memset(transactionBuffer, 0x00, adbmsData->chainInfo.numDevs * REGISTER_SIZE_BYTES);

    TRANSACTION_STATUS_E status = readChain(RDPWMA, &adbmsData->chainInfo, transactionBuffer);

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
            uint8_t pwmSetting0 = (cellMonitorDataBuffer[(i * REGISTER_SIZE_BYTES) + j] & PWM_CONFIG_SIZE_MASK);
            uint8_t pwmSetting1 = (cellMonitorDataBuffer[(i * REGISTER_SIZE_BYTES) + j] >> PWM_CONFIG_SIZE_BITS);

            adbmsData->cellMonitor[i].dischargePWM[j * 2] = pwmSetting0 * PWM_SETTING_GAIN;
            adbmsData->cellMonitor[i].dischargePWM[(j * 2) + 1] = pwmSetting1 * PWM_SETTING_GAIN;
        }
    }

    if((status == TRANSACTION_SUCCESS) || (status == TRANSACTION_CHAIN_BREAK_ERROR))
    {
        status = readChain(RDPWMB, &adbmsData->chainInfo, transactionBuffer);
    }

    for(uint32_t i = 0; i < (adbmsData->chainInfo.numDevs - 1); i++)
    {
        for(uint32_t j = 0; j < NUM_BYTES_PWM_B; j++)
        {
            uint8_t pwmSetting0 = (cellMonitorDataBuffer[(i * REGISTER_SIZE_BYTES) + j] & PWM_CONFIG_SIZE_MASK);
            uint8_t pwmSetting1 = (cellMonitorDataBuffer[(i * REGISTER_SIZE_BYTES) + j] >> PWM_CONFIG_SIZE_BITS);

            adbmsData->cellMonitor[i].dischargePWM[NUM_CELLS_PWM_A + (j * 2)] = pwmSetting0 * PWM_SETTING_GAIN;
            adbmsData->cellMonitor[i].dischargePWM[NUM_CELLS_PWM_A + (j * 2) + 1] = pwmSetting1 * PWM_SETTING_GAIN;
        }
    }

    return status;
}


// TODO Test if write works through Pack Monitor
// Make sure followed with read to check data and lock register
TRANSACTION_STATUS_E writeNVM(ADBMS_BatteryData  *adbmsData)
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

    return writeChain(WRRR, &adbmsData->chainInfo, SHARED_COMMAND, transactionBuffer);
}

TRANSACTION_STATUS_E readNVM(ADBMS_BatteryData *adbmsData)
{
    memset(transactionBuffer, 0x00, adbmsData->chainInfo.numDevs * REGISTER_SIZE_BYTES);

    TRANSACTION_STATUS_E status = readChain(RDRR, &adbmsData->chainInfo, transactionBuffer);

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
        memcpy(adbmsData->cellMonitor[i].retentionRegister, cellMonitorDataBuffer + (i * REGISTER_SIZE_BYTES), REGISTER_SIZE_BYTES);
    }

    return status;
}

TRANSACTION_STATUS_E writeConfigA(ADBMS_BatteryData * adbmsData)
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

    memcpy(packMonitorDataBuffer, &adbmsData->packMonitor.configGroupA, REGISTER_SIZE_BYTES);

    for(uint32_t i = 0; i < (adbmsData->chainInfo.numDevs - 1); i++)
    {
        memcpy(cellMonitorDataBuffer + (i * REGISTER_SIZE_BYTES), &adbmsData->cellMonitor[i].configGroupA, REGISTER_SIZE_BYTES);
    }

    return writeChain(WRCFGA, &adbmsData->chainInfo, SHARED_COMMAND, transactionBuffer);
}

TRANSACTION_STATUS_E readConfigA(ADBMS_BatteryData *adbmsData)
{
    memset(transactionBuffer, 0x00, adbmsData->chainInfo.numDevs * REGISTER_SIZE_BYTES);

    TRANSACTION_STATUS_E status = readChain(RDCFGA, &adbmsData->chainInfo, transactionBuffer);

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

    memcpy(&adbmsData->packMonitor.configGroupA, packMonitorDataBuffer, REGISTER_SIZE_BYTES);

    for(uint32_t i = 0; i < (adbmsData->chainInfo.numDevs - 1); i++)
    {
        memcpy(&adbmsData->cellMonitor[i].configGroupA, cellMonitorDataBuffer + (i * REGISTER_SIZE_BYTES), REGISTER_SIZE_BYTES);
    }

    return status;
}

TRANSACTION_STATUS_E writeConfigB(ADBMS_BatteryData *adbmsData)
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

    memcpy(packMonitorDataBuffer, &adbmsData->packMonitor.configGroupB, REGISTER_SIZE_BYTES);

    for(uint32_t i = 0; i < (adbmsData->chainInfo.numDevs - 1); i++)
    {
        uint8_t *deviceRegister = cellMonitorDataBuffer + (i * REGISTER_SIZE_BYTES);

        float underVoltageThresh = adbmsData->cellMonitor[i].configGroupB.undervoltageThreshold;
        float overVoltageThresh = adbmsData->cellMonitor[i].configGroupB.overvoltageThreshold;

        if(underVoltageThresh < MIN_OV_UV_VALUE)
        {
            underVoltageThresh = MIN_OV_UV_VALUE;
        }
        else if(underVoltageThresh > MAX_OV_UV_VALUE)
        {
            underVoltageThresh = MAX_OV_UV_VALUE;
        }

        if(overVoltageThresh < MIN_OV_UV_VALUE)
        {
            overVoltageThresh = MIN_OV_UV_VALUE;
        }
        else if(overVoltageThresh > MAX_OV_UV_VALUE)
        {
            overVoltageThresh = MAX_OV_UV_VALUE;
        }

        uint32_t underVoltageSetting = CONVERT_FLOAT_TO_REGISTER(underVoltageThresh, CELL_MON_OV_UV_GAIN, CELL_MON_OV_UV_OFFSET) & CELL_OV_UV_MASK;
        uint32_t overVoltageSetting = CONVERT_FLOAT_TO_REGISTER(overVoltageThresh, CELL_MON_OV_UV_GAIN, CELL_MON_OV_UV_OFFSET) & CELL_OV_UV_MASK;
        uint32_t cellThresholdSettings = underVoltageSetting | (overVoltageSetting << CELL_OV_UV_BITS);

        deviceRegister[REGISTER_BYTE0] = (uint8_t)(cellThresholdSettings);
        deviceRegister[REGISTER_BYTE1] = (uint8_t)(cellThresholdSettings >> BITS_IN_BYTE);
        deviceRegister[REGISTER_BYTE2] = (uint8_t)(cellThresholdSettings >> (2 * BITS_IN_BYTE));

        uint32_t dischargeTimeout = adbmsData->cellMonitor[i].configGroupB.dischargeTimeoutMinutes;
        uint8_t dischargeTimerSetting = 0;
        if(dischargeTimeout > 0)
        {
            dischargeTimerSetting |= DTM_ENABLE;

            if(dischargeTimeout <= DTM_SHORT_RANGE_MAX)
            {
                dischargeTimerSetting |= (uint8_t)dischargeTimeout;
            }
            else
            {
                dischargeTimerSetting |= DTM_LONG_RANGE_ENABLE;

                if(dischargeTimeout <= DTM_LONG_RANGE_MAX)
                {
                    dischargeTimerSetting |= (uint8_t)((dischargeTimeout + (DTM_LONG_RANGE_STEP / 2)) /  DTM_LONG_RANGE_STEP);
                }
                else
                {
                    dischargeTimerSetting |= (uint8_t)(DTM_LONG_RANGE_MAX /  DTM_LONG_RANGE_STEP);
                }
            }
        }

        deviceRegister[REGISTER_BYTE3] = dischargeTimerSetting;

        uint16_t dischargeMask = 0;
        for(uint32_t j = 0; j < NUM_CELLS_PER_CELL_MONITOR; j++)
        {
            dischargeMask |= (adbmsData->cellMonitor[i].configGroupB.dischargeCell[j] << j);
        }

        deviceRegister[REGISTER_BYTE4] = (uint8_t)dischargeMask;
        deviceRegister[REGISTER_BYTE5] = (uint8_t)(dischargeMask >> BITS_IN_BYTE);
    }

    return writeChain(WRCFGB, &adbmsData->chainInfo, SHARED_COMMAND, transactionBuffer);

}

TRANSACTION_STATUS_E readConfigB(ADBMS_BatteryData *adbmsData)
{
    memset(transactionBuffer, 0x00, adbmsData->chainInfo.numDevs * REGISTER_SIZE_BYTES);

    TRANSACTION_STATUS_E status = readChain(RDCFGB, &adbmsData->chainInfo, transactionBuffer);

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

    memcpy(&adbmsData->packMonitor.configGroupB, packMonitorDataBuffer, REGISTER_SIZE_BYTES);

    for(uint32_t i = 0; i < (adbmsData->chainInfo.numDevs - 1); i++)
    {
        uint8_t *deviceRegister = cellMonitorDataBuffer + (i * REGISTER_SIZE_BYTES);

        uint32_t cellThresholdSettings = (uint32_t)deviceRegister[REGISTER_BYTE0] | ((uint32_t)deviceRegister[REGISTER_BYTE1] << BITS_IN_BYTE) | ((uint32_t)deviceRegister[REGISTER_BYTE2] << (2 * BITS_IN_BYTE));
        uint16_t underVoltageSetting = cellThresholdSettings & CELL_OV_UV_MASK;
        uint16_t overVoltageSetting = cellThresholdSettings >> CELL_OV_UV_BITS;

        adbmsData->cellMonitor[i].configGroupB.undervoltageThreshold = CONVERT_SIGNED_12_BIT_REGISTER(underVoltageSetting, CELL_MON_OV_UV_GAIN, CELL_MON_OV_UV_OFFSET);
        adbmsData->cellMonitor[i].configGroupB.overvoltageThreshold = CONVERT_SIGNED_12_BIT_REGISTER(overVoltageSetting, CELL_MON_OV_UV_GAIN, CELL_MON_OV_UV_OFFSET);

        uint8_t dischargeTimerSetting = deviceRegister[REGISTER_BYTE3];

        if(dischargeTimerSetting & DTM_ENABLE)
        {
            if(dischargeTimerSetting & DTM_LONG_RANGE_ENABLE)
            {
                adbmsData->cellMonitor[i].configGroupB.dischargeTimeoutMinutes = ((dischargeTimerSetting & DTM_TIME_MASK) * DTM_LONG_RANGE_STEP);
            }
            else
            {
                adbmsData->cellMonitor[i].configGroupB.dischargeTimeoutMinutes = (dischargeTimerSetting & DTM_TIME_MASK);
            }
        }
        else
        {
            adbmsData->cellMonitor[i].configGroupB.dischargeTimeoutMinutes = 0;
        }

        uint16_t dischargeMask = (uint16_t)deviceRegister[REGISTER_BYTE4] | ((uint16_t)deviceRegister[REGISTER_BYTE5] << BITS_IN_BYTE);
        for(uint32_t j = 0; j < NUM_CELLS_PER_CELL_MONITOR; j++)
        {
            adbmsData->cellMonitor[i].configGroupB.dischargeCell[j] = (dischargeMask >> j) & 0x0001;
        }
    }

    return status;
}


TRANSACTION_STATUS_E readStatusA(ADBMS_BatteryData *adbmsData)
{
    memset(transactionBuffer, 0x00, adbmsData->chainInfo.numDevs * REGISTER_SIZE_BYTES);

    TRANSACTION_STATUS_E status = readChain(RDSTATA, &adbmsData->chainInfo, transactionBuffer);

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

    adbmsData->packMonitor.statusGroupA.referenceVoltage1P25 = CONVERT_SIGNED_16_BIT_REGISTER(packMonitorDataBuffer, PACK_MON_VREF1P25_GAIN, PACK_MON_VREF1P25_OFFSET);
    adbmsData->packMonitor.statusGroupA.dieTemp1 = CONVERT_SIGNED_16_BIT_REGISTER((packMonitorDataBuffer + (VOLTAGE_16BIT_SIZE_BYTES)), PACK_MON_DIE_TEMP1_GAIN, PACK_MON_DIE_TEMP1_OFFSET);
    adbmsData->packMonitor.statusGroupA.regulatorVoltage = CONVERT_SIGNED_16_BIT_REGISTER((packMonitorDataBuffer + (2 * VOLTAGE_16BIT_SIZE_BYTES)), PACK_MON_VREG_GAIN, PACK_MON_VREG_OFFSET);

    for(uint32_t i = 0; i < (adbmsData->chainInfo.numDevs - 1); i++)
    {
        adbmsData->cellMonitor[i].statusGroupA.referenceVoltage = CONVERT_SIGNED_16_BIT_REGISTER((cellMonitorDataBuffer + (i * REGISTER_SIZE_BYTES)), CELL_MON_AUX_ADC_GAIN, CELL_MON_AUX_ADC_OFFSET);
        adbmsData->cellMonitor[i].statusGroupA.dieTemp = CONVERT_SIGNED_16_BIT_REGISTER((cellMonitorDataBuffer + (i * REGISTER_SIZE_BYTES) + (VOLTAGE_16BIT_SIZE_BYTES)), CELL_MON_DIE_TEMP_GAIN, CELL_MON_DIE_TEMP_OFFSET);
    }

    return status;
}

TRANSACTION_STATUS_E readStatusB(ADBMS_BatteryData *adbmsData)
{
    memset(transactionBuffer, 0x00, adbmsData->chainInfo.numDevs * REGISTER_SIZE_BYTES);

    TRANSACTION_STATUS_E status = readChain(RDSTATB, &adbmsData->chainInfo, transactionBuffer);

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

    adbmsData->packMonitor.statusGroupB.supplyVoltage = CONVERT_SIGNED_16_BIT_REGISTER(packMonitorDataBuffer, PACK_MON_VDD_GAIN, PACK_MON_VDD_OFFSET);
    adbmsData->packMonitor.statusGroupB.digitalSupplyVoltage = CONVERT_SIGNED_16_BIT_REGISTER((packMonitorDataBuffer + (VOLTAGE_16BIT_SIZE_BYTES)), PACK_MON_VDIG_GAIN, PACK_MON_VDIG_OFFSET);
    adbmsData->packMonitor.statusGroupB.groundPadVoltage = CONVERT_SIGNED_16_BIT_REGISTER((packMonitorDataBuffer + (2 * VOLTAGE_16BIT_SIZE_BYTES)), PACK_MON_EPAD_GAIN, PACK_MON_EPAD_OFFSET);

    for(uint32_t i = 0; i < (adbmsData->chainInfo.numDevs - 1); i++)
    {
        adbmsData->cellMonitor[i].statusGroupB.digitalSupplyVoltage = CONVERT_SIGNED_16_BIT_REGISTER((cellMonitorDataBuffer + (i * REGISTER_SIZE_BYTES)), CELL_MON_AUX_ADC_GAIN, CELL_MON_AUX_ADC_OFFSET);
        adbmsData->cellMonitor[i].statusGroupB.analogSupplyVoltage = CONVERT_SIGNED_16_BIT_REGISTER((cellMonitorDataBuffer + (i * REGISTER_SIZE_BYTES) + (VOLTAGE_16BIT_SIZE_BYTES)), CELL_MON_AUX_ADC_GAIN, CELL_MON_AUX_ADC_OFFSET);
        adbmsData->cellMonitor[i].statusGroupB.referenceResistorVoltage = CONVERT_SIGNED_16_BIT_REGISTER((cellMonitorDataBuffer + (i * REGISTER_SIZE_BYTES) + (2 * VOLTAGE_16BIT_SIZE_BYTES)), CELL_MON_AUX_ADC_GAIN, CELL_MON_AUX_ADC_OFFSET);
    }

    return status;
}

TRANSACTION_STATUS_E readStatusC(ADBMS_BatteryData *adbmsData)
{
    memset(transactionBuffer, 0x00, adbmsData->chainInfo.numDevs * REGISTER_SIZE_BYTES);

    TRANSACTION_STATUS_E status = readChain(RDSTATC, &adbmsData->chainInfo, transactionBuffer);

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

    memcpy(&adbmsData->packMonitor.statusGroupC, packMonitorDataBuffer, REGISTER_SIZE_BYTES);
    adbmsData->packMonitor.statusGroupC.conversionCounter1 = (((uint16_t)(packMonitorDataBuffer[REGISTER_BYTE2] & PACK_MON_COUNTER1_MASK)) << BITS_IN_BYTE) | ((uint16_t)(packMonitorDataBuffer[REGISTER_BYTE3]));
    adbmsData->packMonitor.statusGroupC.conversionCounter2 = packMonitorDataBuffer[REGISTER_BYTE2] >> PACK_MON_COUNTER2_BIT;

    for(uint32_t i = 0; i < (adbmsData->chainInfo.numDevs - 1); i++)
    {
        uint8_t *statRegister = cellMonitorDataBuffer + (i * REGISTER_SIZE_BYTES);

        memcpy(&adbmsData->cellMonitor[i].statusGroupC, statRegister, REGISTER_SIZE_BYTES);

        adbmsData->cellMonitor[i].statusGroupC.conversionCounter = (((uint16_t)(statRegister[REGISTER_BYTE2])) << BITS_IN_BYTE) | ((uint16_t)(statRegister[REGISTER_BYTE3]));

        uint16_t cellAdcMismatchMask = ((uint16_t)(statRegister[REGISTER_BYTE0])) | (((uint16_t)(statRegister[REGISTER_BYTE1])) << BITS_IN_BYTE);

        for(uint32_t j = 0; j < NUM_CELLS_PER_CELL_MONITOR; j++)
        {
            adbmsData->cellMonitor[i].statusGroupC.cellAdcMismatchFault[j] = ((cellAdcMismatchMask >> j) & 0x0001);
        }
    }

    return status;
}

TRANSACTION_STATUS_E readStatusD(ADBMS_BatteryData *adbmsData)
{
    memset(transactionBuffer, 0x00, adbmsData->chainInfo.numDevs * REGISTER_SIZE_BYTES);

    TRANSACTION_STATUS_E status = readChain(RDSTATD, &adbmsData->chainInfo, transactionBuffer);

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

    adbmsData->packMonitor.statusGroupD.referenceResistorVoltage = CONVERT_SIGNED_16_BIT_REGISTER(packMonitorDataBuffer, PACK_MON_VDIV_GAIN, PACK_MON_VDIV_OFFSET);
    adbmsData->packMonitor.statusGroupD.dieTemp2 = CONVERT_SIGNED_16_BIT_REGISTER((packMonitorDataBuffer + (VOLTAGE_16BIT_SIZE_BYTES)), PACK_MON_DIE_TEMP2_GAIN, PACK_MON_DIE_TEMP2_OFFSET);
    adbmsData->packMonitor.statusGroupD.oscillatorCounter = packMonitorDataBuffer[REGISTER_BYTE5];

    for(uint32_t i = 0; i < (adbmsData->chainInfo.numDevs - 1); i++)
    {
        uint8_t *statRegister = cellMonitorDataBuffer + (i * REGISTER_SIZE_BYTES);

        adbmsData->cellMonitor[i].statusGroupD.oscillatorCounter = statRegister[REGISTER_BYTE5];

        uint32_t cellFault0 = (uint32_t)statRegister[REGISTER_BYTE0];
        uint32_t cellFault1 = ((uint32_t)statRegister[REGISTER_BYTE1]) << (BITS_IN_BYTE);
        uint32_t cellFault2 = ((uint32_t)statRegister[REGISTER_BYTE2]) << (BITS_IN_BYTE * 2);
        uint32_t cellFault3 = ((uint32_t)statRegister[REGISTER_BYTE3]) << (BITS_IN_BYTE * 3);

        uint32_t cellFaultMask = (cellFault0) | (cellFault1) | (cellFault2) | (cellFault3);

        for(uint32_t j = 0; j < NUM_CELLS_PER_CELL_MONITOR; j++)
        {
            adbmsData->cellMonitor[i].statusGroupD.cellUnderVoltageFault[j] = ((cellFaultMask >> (j * 2)) & 0x00000001);
            adbmsData->cellMonitor[i].statusGroupD.cellOverVoltageFault[j] = ((cellFaultMask >> ((j * 2) + 1)) & 0x00000001);
        }
    }

    return status;
}

TRANSACTION_STATUS_E readStatusE(ADBMS_BatteryData *adbmsData)
{
    memset(transactionBuffer, 0x00, adbmsData->chainInfo.numDevs * REGISTER_SIZE_BYTES);

    TRANSACTION_STATUS_E status = readChain(RDSTATE, &adbmsData->chainInfo, transactionBuffer);

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

    memcpy(&adbmsData->packMonitor.statusGroupE, packMonitorDataBuffer, REGISTER_SIZE_BYTES);

    for(uint32_t i = 0; i < (adbmsData->chainInfo.numDevs - 1); i++)
    {
        memcpy(&adbmsData->cellMonitor[i].statusGroupE, cellMonitorDataBuffer + (i * REGISTER_SIZE_BYTES) + REGISTER_BYTE4, BYTES_IN_WORD);
    }

    return status;
}

TRANSACTION_STATUS_E readCellVoltages(ADBMS_BatteryData *adbmsData, CELL_VOLTAGE_TYPE_E cellVoltageType)
{
    memset(transactionBuffer, 0x00, adbmsData->chainInfo.numDevs * REGISTER_SIZE_BYTES);

    uint8_t packRegisterData[NUM_CELLV_REGISTERS][REGISTER_SIZE_BYTES];
    memset(packRegisterData, 0x00, NUM_CELLV_REGISTERS * REGISTER_SIZE_BYTES);

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

    TRANSACTION_STATUS_E status = TRANSACTION_SUCCESS;
    for(uint32_t i = 0; i < (NUM_CELLV_REGISTERS - 1); i++)
    {
        if((status == TRANSACTION_SUCCESS) || (status == TRANSACTION_CHAIN_BREAK_ERROR))
        {
            status = readChain(cellVoltageCode[cellVoltageType][i], &adbmsData->chainInfo, transactionBuffer);
        }

        memcpy(packRegisterData[i], packMonitorDataBuffer, REGISTER_SIZE_BYTES);

        for(uint32_t j = 0; j < (adbmsData->chainInfo.numDevs - 1); j++)
        {
            for(uint32_t k = 0; k < VOLTAGE_16BIT_PER_REG; k++)
            {
                adbmsData->cellMonitor[j].cellVoltage[(i * VOLTAGE_16BIT_PER_REG) + k] = CONVERT_SIGNED_16_BIT_REGISTER((cellMonitorDataBuffer + (j * REGISTER_SIZE_BYTES) + (k * VOLTAGE_16BIT_SIZE_BYTES)), CELL_MON_CELL_ADC_GAIN, CELL_MON_CELL_ADC_OFFSET);
            }
        }
    }

    if((status == TRANSACTION_SUCCESS) || (status == TRANSACTION_CHAIN_BREAK_ERROR))
    {
        status = readChain(cellVoltageCode[cellVoltageType][NUM_CELLV_REGISTERS - 1], &adbmsData->chainInfo, transactionBuffer);
    }

    memcpy(packRegisterData[NUM_CELLV_REGISTERS - 1], packMonitorDataBuffer, REGISTER_SIZE_BYTES);

    for(uint32_t j = 0; j < (adbmsData->chainInfo.numDevs - 1); j++)
    {
        adbmsData->cellMonitor[j].cellVoltage[(NUM_CELLV_REGISTERS - 1) * VOLTAGE_16BIT_PER_REG] = CONVERT_SIGNED_16_BIT_REGISTER((cellMonitorDataBuffer + (j * REGISTER_SIZE_BYTES)), CELL_MON_CELL_ADC_GAIN, CELL_MON_CELL_ADC_OFFSET);
    }

    // Buffer[3] and Buffer[4] hold aux voltages 1-6
    for(uint32_t i = 0; i < VOLTAGE_16BIT_PER_REG; i++)
    {
        adbmsData->packMonitor.auxVoltage[i] = CONVERT_SIGNED_16_BIT_REGISTER((packRegisterData[3] + (i * VOLTAGE_16BIT_SIZE_BYTES)), PACK_MON_VADC1_GAIN, PACK_MON_VADC1_OFFSET);
        adbmsData->packMonitor.auxVoltage[(i + VOLTAGE_16BIT_PER_REG)] = CONVERT_SIGNED_16_BIT_REGISTER((packRegisterData[4] + (i * VOLTAGE_16BIT_SIZE_BYTES)), PACK_MON_VADC1_GAIN, PACK_MON_VADC1_OFFSET);
    }

    if(cellVoltageType == AVERAGED_CELL_VOLTAGE)
    {
        memcpy(packRegisterData[3], packRegisterData[0], REGISTER_SIZE_BYTES);
        memcpy(packRegisterData[4], packRegisterData[1], REGISTER_SIZE_BYTES);

        if((status == TRANSACTION_SUCCESS) || (status == TRANSACTION_CHAIN_BREAK_ERROR))
        {
            status = readPackMonitor(RDCVA, &adbmsData->chainInfo, packRegisterData[0]);
        }

        if((status == TRANSACTION_SUCCESS) || (status == TRANSACTION_CHAIN_BREAK_ERROR))
        {
            status = readPackMonitor(RDCVB, &adbmsData->chainInfo, packRegisterData[1]);
        }

        if((status == TRANSACTION_SUCCESS) || (status == TRANSACTION_CHAIN_BREAK_ERROR))
        {
            status = readPackMonitor(RDCVF, &adbmsData->chainInfo, packRegisterData[5]);
        }
    }
    else
    {
        if((status == TRANSACTION_SUCCESS) || (status == TRANSACTION_CHAIN_BREAK_ERROR))
        {
            status = readPackMonitor(RDACA, &adbmsData->chainInfo, packRegisterData[3]);
        }

        if((status == TRANSACTION_SUCCESS) || (status == TRANSACTION_CHAIN_BREAK_ERROR))
        {
            status = readPackMonitor(RDACB, &adbmsData->chainInfo, packRegisterData[4]);
        }
    }

    // Buffer[0] holds IADC1 and IADC2 data
    adbmsData->packMonitor.currentAdc1uV = CONVERT_SIGNED_24_BIT_REGISTER_UV(packRegisterData[0], PACK_MON_IADC1_GAIN_UV);
    adbmsData->packMonitor.currentAdc2uV = CONVERT_SIGNED_24_BIT_REGISTER_UV((packRegisterData[0] + VOLTAGE_24BIT_SIZE_BYTES), PACK_MON_IADC2_GAIN_UV);

    // Buffer[1] holds VBADC1 and VBADC2 data
    adbmsData->packMonitor.batteryVoltage1 = CONVERT_SIGNED_16_BIT_REGISTER((packRegisterData[1] + VOLTAGE_16BIT_SIZE_BYTES), PACK_MON_VADC1_GAIN, PACK_MON_VADC1_OFFSET);
    adbmsData->packMonitor.batteryVoltage2 = CONVERT_SIGNED_16_BIT_REGISTER((packRegisterData[1] + (2 * VOLTAGE_16BIT_SIZE_BYTES)), PACK_MON_VADC2_GAIN, PACK_MON_VADC2_OFFSET);

    // Buffer[3] holds IACC1 and IACC2 data
    adbmsData->packMonitor.currentAdcAccumulator1uV = CONVERT_SIGNED_24_BIT_REGISTER_UV(packRegisterData[3], PACK_MON_IADC1_GAIN_UV);
    adbmsData->packMonitor.currentAdcAccumulator2uV = CONVERT_SIGNED_24_BIT_REGISTER_UV((packRegisterData[3] + VOLTAGE_24BIT_SIZE_BYTES), PACK_MON_IADC2_GAIN_UV);

    // Buffer[4] holds VBACC1 and VBACC2 data
    adbmsData->packMonitor.batteryVoltageAccumulator1uV = CONVERT_SIGNED_24_BIT_REGISTER_UV(packRegisterData[4], PACK_MON_VACC1_GAIN_UV);
    adbmsData->packMonitor.batteryVoltageAccumulator2uV = CONVERT_SIGNED_24_BIT_REGISTER_UV((packRegisterData[4] + VOLTAGE_24BIT_SIZE_BYTES), PACK_MON_VACC2_GAIN_UV);

    // Buffer[5] hold overcurrent ADC data
    float oc1Gain = (adbmsData->packMonitor.configGroupB.oc1GainControl) ? (OVERCURRENT_GAIN2) : (OVERCURRENT_GAIN1);
    float oc2Gain = (adbmsData->packMonitor.configGroupB.oc2GainControl) ? (OVERCURRENT_GAIN2) : (OVERCURRENT_GAIN1);
    float oc3Gain = (adbmsData->packMonitor.configGroupB.oc3GainControl) ? (OVERCURRENT_GAIN2) : (OVERCURRENT_GAIN1);

    adbmsData->packMonitor.overcurrentStatusGroup.overCurrentAdc1 = packRegisterData[5][REGISTER_BYTE0] * oc1Gain;
    adbmsData->packMonitor.overcurrentStatusGroup.overCurrentAdc2 = packRegisterData[5][REGISTER_BYTE1] * oc2Gain;
    adbmsData->packMonitor.overcurrentStatusGroup.overCurrentAdc3 = packRegisterData[5][REGISTER_BYTE2] * oc3Gain;
    adbmsData->packMonitor.overcurrentStatusGroup.overCurrentAdc3Max = packRegisterData[5][REGISTER_BYTE4] * oc3Gain;
    adbmsData->packMonitor.overcurrentStatusGroup.overCurrentAdc3Min = packRegisterData[5][REGISTER_BYTE5] * oc3Gain;

    return status;
}

TRANSACTION_STATUS_E readRedundantCellVoltages(ADBMS_BatteryData *adbmsData)
{
    memset(transactionBuffer, 0x00, adbmsData->chainInfo.numDevs * REGISTER_SIZE_BYTES);

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

    TRANSACTION_STATUS_E status = TRANSACTION_SUCCESS;
    for(uint32_t i = 0; i < (NUM_CELLV_REGISTERS - 1); i++)
    {
        if((status == TRANSACTION_SUCCESS) || (status == TRANSACTION_CHAIN_BREAK_ERROR))
        {
            status = readChain(redundantAuxVoltageCode[i], &adbmsData->chainInfo, transactionBuffer);
        }

        for(uint32_t j = 0; j < (adbmsData->chainInfo.numDevs - 1); j++)
        {
            for(uint32_t k = 0; k < VOLTAGE_16BIT_PER_REG; k++)
            {
                adbmsData->cellMonitor[j].redundantCellVoltage[(i * VOLTAGE_16BIT_PER_REG) + k] = CONVERT_SIGNED_16_BIT_REGISTER((cellMonitorDataBuffer + (j * REGISTER_SIZE_BYTES) + (k * VOLTAGE_16BIT_SIZE_BYTES)), CELL_MON_CELL_ADC_GAIN, CELL_MON_CELL_ADC_OFFSET);
            }
        }
    }

    if((status == TRANSACTION_SUCCESS) || (status == TRANSACTION_CHAIN_BREAK_ERROR))
    {
        status = readChain(redundantAuxVoltageCode[NUM_CELLV_REGISTERS - 1], &adbmsData->chainInfo, transactionBuffer);
    }

    for(uint32_t j = 0; j < (adbmsData->chainInfo.numDevs - 1); j++)
    {
        adbmsData->cellMonitor[j].redundantCellVoltage[(NUM_CELLV_REGISTERS - 1) * VOLTAGE_16BIT_PER_REG] = CONVERT_SIGNED_16_BIT_REGISTER((cellMonitorDataBuffer + (j * REGISTER_SIZE_BYTES)), CELL_MON_CELL_ADC_GAIN, CELL_MON_CELL_ADC_OFFSET);
    }

    return status;
}

TRANSACTION_STATUS_E readAuxVoltages(ADBMS_BatteryData * adbmsData)
{
    memset(transactionBuffer, 0x00, adbmsData->chainInfo.numDevs * REGISTER_SIZE_BYTES);

    uint8_t packRegisterData[NUM_AUXV_REGISTERS][REGISTER_SIZE_BYTES];
    memset(packRegisterData, 0x00, NUM_AUXV_REGISTERS * REGISTER_SIZE_BYTES);

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

    TRANSACTION_STATUS_E status = TRANSACTION_SUCCESS;
    for(uint32_t i = 0; i < (NUM_AUXV_REGISTERS - 1); i++)
    {
        if((status == TRANSACTION_SUCCESS) || (status == TRANSACTION_CHAIN_BREAK_ERROR))
        {
            status = readChain(auxVoltageCode[i], &adbmsData->chainInfo, transactionBuffer);
        }

        memcpy(packRegisterData[i], packMonitorDataBuffer, REGISTER_SIZE_BYTES);

        for(uint32_t j = 0; j < (adbmsData->chainInfo.numDevs - 1); j++)
        {
            for(uint32_t k = 0; k < VOLTAGE_16BIT_PER_REG; k++)
            {
                adbmsData->cellMonitor[j].auxVoltage[(i * VOLTAGE_16BIT_PER_REG) + k] = CONVERT_SIGNED_16_BIT_REGISTER((cellMonitorDataBuffer + (j * REGISTER_SIZE_BYTES) + (k * VOLTAGE_16BIT_SIZE_BYTES)), CELL_MON_AUX_ADC_GAIN, CELL_MON_AUX_ADC_OFFSET);
            }
        }
    }

    if((status == TRANSACTION_SUCCESS) || (status == TRANSACTION_CHAIN_BREAK_ERROR))
    {
        status = readChain(auxVoltageCode[NUM_AUXV_REGISTERS - 1], &adbmsData->chainInfo, transactionBuffer);
    }

    memcpy(packRegisterData[NUM_AUXV_REGISTERS - 1], packMonitorDataBuffer, REGISTER_SIZE_BYTES);

    for(uint32_t j = 0; j < (adbmsData->chainInfo.numDevs - 1); j++)
    {
        adbmsData->cellMonitor[j].auxVoltage[(NUM_AUXV_REGISTERS - 1) * VOLTAGE_16BIT_PER_REG] = CONVERT_SIGNED_16_BIT_REGISTER((cellMonitorDataBuffer + (j * REGISTER_SIZE_BYTES)), CELL_MON_AUX_ADC_GAIN, CELL_MON_AUX_ADC_OFFSET);
        adbmsData->cellMonitor[j].switch1Voltage = CONVERT_SIGNED_16_BIT_REGISTER((cellMonitorDataBuffer + (j * REGISTER_SIZE_BYTES) + (VOLTAGE_16BIT_SIZE_BYTES)), CELL_MON_AUX_ADC_GAIN, CELL_MON_AUX_ADC_OFFSET);
        adbmsData->cellMonitor[j].hvSupplyVoltage = CONVERT_SIGNED_16_BIT_REGISTER((cellMonitorDataBuffer + (j * REGISTER_SIZE_BYTES) + (2 * VOLTAGE_16BIT_SIZE_BYTES)), CELL_MON_HV_SUPPLY_GAIN, CELL_MON_HV_SUPPLY_OFFSET);
    }

     // Buffer[0], Buffer[1], and Buffer[2] hold aux voltages 1-9
    for(uint32_t i = 0; i < VOLTAGE_16BIT_PER_REG; i++)
    {
        adbmsData->packMonitor.auxVoltage[i] = CONVERT_SIGNED_16_BIT_REGISTER((packRegisterData[0] + (i * VOLTAGE_16BIT_SIZE_BYTES)), PACK_MON_VADC1_GAIN, PACK_MON_VADC1_OFFSET);
        adbmsData->packMonitor.auxVoltage[(i + VOLTAGE_16BIT_PER_REG)] = CONVERT_SIGNED_16_BIT_REGISTER((packRegisterData[1] + (i * VOLTAGE_16BIT_SIZE_BYTES)), PACK_MON_VADC1_GAIN, PACK_MON_VADC1_OFFSET);
    }

    adbmsData->packMonitor.auxVoltage[(2 * VOLTAGE_16BIT_PER_REG)]  = CONVERT_SIGNED_16_BIT_REGISTER(packRegisterData[2], PACK_MON_VADC1_GAIN, PACK_MON_VADC1_OFFSET);
    adbmsData->packMonitor.auxVoltage[(2 * VOLTAGE_16BIT_PER_REG) + 1] = CONVERT_SIGNED_16_BIT_REGISTER((packRegisterData[2] + (VOLTAGE_16BIT_SIZE_BYTES)), PACK_MON_VADC1_GAIN, PACK_MON_VADC1_OFFSET);
    adbmsData->packMonitor.auxVoltage[(2 * VOLTAGE_16BIT_PER_REG) + 2] = CONVERT_SIGNED_16_BIT_REGISTER((packRegisterData[2] + (2 * VOLTAGE_16BIT_SIZE_BYTES)), PACK_MON_VADC2_GAIN, PACK_MON_VADC2_OFFSET);

    adbmsData->packMonitor.auxVoltage[(3 * VOLTAGE_16BIT_PER_REG)] = CONVERT_SIGNED_16_BIT_REGISTER(packRegisterData[3], PACK_MON_VADC2_GAIN, PACK_MON_VADC2_OFFSET);
    adbmsData->packMonitor.referenceVoltage = CONVERT_SIGNED_16_BIT_REGISTER((packRegisterData[3] + (VOLTAGE_16BIT_SIZE_BYTES)), PACK_MON_VREF2A_GAIN, PACK_MON_VREF2A_OFFSET);
    adbmsData->packMonitor.redundantReferenceVoltage = CONVERT_SIGNED_16_BIT_REGISTER((packRegisterData[3] + (2 * VOLTAGE_16BIT_SIZE_BYTES)), PACK_MON_VREF2B_GAIN, PACK_MON_VREF2B_OFFSET);

    return status;
}

TRANSACTION_STATUS_E readRedundantAuxVoltages(ADBMS_BatteryData * adbmsData)
{
    memset(transactionBuffer, 0x00, adbmsData->chainInfo.numDevs * REGISTER_SIZE_BYTES);

    uint8_t packRegisterData[NUM_AUXV_REGISTERS][REGISTER_SIZE_BYTES];
    memset(packRegisterData, 0x00, NUM_AUXV_REGISTERS * REGISTER_SIZE_BYTES);

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

    TRANSACTION_STATUS_E status = TRANSACTION_SUCCESS;
    for(uint32_t i = 0; i < (NUM_AUXV_REGISTERS - 1); i++)
    {
        if((status == TRANSACTION_SUCCESS) || (status == TRANSACTION_CHAIN_BREAK_ERROR))
        {
            status = readChain(redundantAuxVoltageCode[i], &adbmsData->chainInfo, transactionBuffer);
        }

        memcpy(packRegisterData[i], packMonitorDataBuffer, REGISTER_SIZE_BYTES);

        for(uint32_t j = 0; j < (adbmsData->chainInfo.numDevs - 1); j++)
        {
            for(uint32_t k = 0; k < VOLTAGE_16BIT_PER_REG; k++)
            {
                adbmsData->cellMonitor[j].reduntantAuxVoltage[(i * VOLTAGE_16BIT_PER_REG) + k] = CONVERT_SIGNED_16_BIT_REGISTER((cellMonitorDataBuffer + (j * REGISTER_SIZE_BYTES) + (k * VOLTAGE_16BIT_SIZE_BYTES)), CELL_MON_AUX_ADC_GAIN, CELL_MON_AUX_ADC_OFFSET);
            }
        }
    }

    if((status == TRANSACTION_SUCCESS) || (status == TRANSACTION_CHAIN_BREAK_ERROR))
    {
        status = readChain(redundantAuxVoltageCode[NUM_AUXV_REGISTERS - 1], &adbmsData->chainInfo, transactionBuffer);
    }

    memcpy(packRegisterData[NUM_AUXV_REGISTERS - 1], packMonitorDataBuffer, REGISTER_SIZE_BYTES);

    for(uint32_t j = 0; j < (adbmsData->chainInfo.numDevs - 1); j++)
    {
        adbmsData->cellMonitor[j].reduntantAuxVoltage[(NUM_AUXV_REGISTERS - 1) * VOLTAGE_16BIT_PER_REG] = CONVERT_SIGNED_16_BIT_REGISTER((cellMonitorDataBuffer + (j * REGISTER_SIZE_BYTES)), CELL_MON_AUX_ADC_GAIN, CELL_MON_AUX_ADC_OFFSET);
    }

     // Buffer[0], Buffer[1], and Buffer[2] hold aux voltages 1-9
    for(uint32_t i = 0; i < VOLTAGE_16BIT_PER_REG; i++)
    {
        adbmsData->packMonitor.redundantAuxVoltage[i] = CONVERT_SIGNED_16_BIT_REGISTER((packRegisterData[0] + (i * VOLTAGE_16BIT_SIZE_BYTES)), PACK_MON_VADC2_GAIN, PACK_MON_VADC2_OFFSET);
        adbmsData->packMonitor.redundantAuxVoltage[(i + VOLTAGE_16BIT_PER_REG)] = CONVERT_SIGNED_16_BIT_REGISTER((packRegisterData[1] + (i * VOLTAGE_16BIT_SIZE_BYTES)), PACK_MON_VADC2_GAIN, PACK_MON_VADC2_OFFSET);
    }

    return status;
}
