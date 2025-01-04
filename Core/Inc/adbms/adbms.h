#ifndef INC_ADBMS_H_
#define INC_ADBMS_H_

/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */
#include "isospi.h"
#include <stdbool.h>

/* ==================================================================== */
/* ============================= DEFINES ============================== */
/* ==================================================================== */

#define NUM_CELLS_PER_CELL_MONITOR  16
#define NUM_CELL_MONITOR_GPIO       10
#define NUM_PACK_MONITOR_GPIO       4
#define NUM_PACK_MONITOR_GPO        6

/* ==================================================================== */
/* ========================= ENUMERATED TYPES========================== */
/* ==================================================================== */

typedef enum
{
    COMPARE_THRESHOLD_5_1_mV = 0,
    COMPARE_THRESHOLD_8_1_mV,
    COMPARE_THRESHOLD_9_mV,
    COMPARE_THRESHOLD_10_mV,
    COMPARE_THRESHOLD_15_mV,
    COMPARE_THRESHOLD_20_mV,
    COMPARE_THRESHOLD_25_mV,
    COMPARE_THRESHOLD_40_mV
} COMPARISON_THRESHOLD_E;
typedef enum
{
    AUX_SOAK_TIME_32_US = 0,
    AUX_SOAK_TIME_64_US,
    AUX_SOAK_TIME_128_US,
    AUX_SOAK_TIME_256_US,
    AUX_SOAK_TIME_512_US,
    AUX_SOAK_TIME_1_MS,
    AUX_SOAK_TIME_2_MS,
    AUX_SOAK_TIME_4_1_MS = 8,
    AUX_SOAK_TIME_8_2_MS,
    AUX_SOAK_TIME_16_4_MS,
    AUX_SOAK_TIME_32_8_MS,
    AUX_SOAK_TIME_65_5_MS,
    AUX_SOAK_TIME_131_MS,
    AUX_SOAK_TIME_262_MS,
    AUX_SOAK_TIME_524_MS
} AUX_SOAK_TIME_E;

typedef enum
{
  GPO_PIN_PULL_DOWN = 0,
  GPO_PIN_HIGH_Z
} ADBMS_GPO_STATE;

typedef enum
{
    FILTER_DISABLED = 0,
    FILTER_CUTOFF_110_HZ,
    FILTER_CUTOFF_45_HZ,
    FILTER_CUTOFF_21_HZ,
    FILTER_CUTOFF_10_HZ,
    FILTER_CUTOFF_5_HZ,
    FILTER_CUTOFF_1_25_HZ,
    FILTER_CUTOFF_625_mHZ
} DIGITAL_FILTER_SETTING_E;

typedef enum
{
    NON_REDUNDANT_MODE = 0,
    REDUNDANT_MODE = (1 << 8)
} ADC_MODE_REDUNDANT_E;

typedef enum
{
    SINGLE_SHOT_MODE = 0,
    CONTINOUS_MODE = (1 << 7)
} ADC_MODE_CONTINOUS_E;

typedef enum
{
    DISCHARGE_DISABLED = 0,
    DISCHARGE_PERMITTED = (1 << 4)
} ADC_MODE_DISCHARGE_E;

typedef enum
{
    NO_FILTER_RESET = 0,
    FILTER_RESET = (1 << 2)
} ADC_MODE_FILTER_E;

typedef enum
{
    CELL_OPEN_WIRE_DISABLED = 0,
    CELL_OPEN_WIRE_EVEN,
    CELL_OPEN_WIRE_ODD
} ADC_MODE_CELL_OPEN_WIRE_E;

typedef enum
{
    AUX_OPEN_WIRE_DISABLED = 0,
    AUX_OPEN_WIRE_PULL_DOWN = (1 << 8),
    AUX_OPEN_WIRE_PULL_UP = ((1 << 8) | (1 << 7))
} ADC_MODE_AUX_OPEN_WIRE_E;

typedef enum
{
    PACK_OPEN_WIRE_DISABLED = 0,
    PACK_OPEN_WIRE_POSITIVE = (1 << 6),
    PACK_OPEN_WIRE_NEGATIVE = (1 << 7)
} ADC_MODE_PACK_OPEN_WIRE_E;

typedef enum
{
    AUX_ALL_CHANNELS = 0,
    AUX_GPIO1_ONLY = 0x01,
    AUX_GPIO2_ONLY = 0x02,
    AUX_GPIO3_ONLY = 0x03,
    AUX_GPIO4_ONLY = 0x04,
    AUX_GPIO5_ONLY = 0x05,
    AUX_GPIO6_ONLY = 0x06,
    AUX_GPIO7_ONLY = 0x07,
    AUX_GPIO8_ONLY = 0x08,
    AUX_GPIO9_ONLY = 0x09,
    AUX_GPIO10_ONLY = 0x0A,
    AUX_VREF2_ONLY = 0x10,
    AUX_VD_ONLY = 0x11,
    AUX_VA_ONLY = 0x12,
    AUX_ITEMP_ONLY = 0x13,
    AUX_VPV_ONLY = 0x14,
    AUX_VMV_ONLY = 0x15,
    AUX_VRES_ONLY = 0x16
} ADC_MODE_AUX_CHANNEL_E;

typedef enum
{
    PACK_V1_ONLY = 0,
    PACK_V2_ONLY,
    PACK_V3_ONLY,
    PACK_V4_ONLY,
    PACK_V5_ONLY,
    PACK_V6_ONLY,
    PACK_V7_V9,
    PACK_V8_V10,
    PACK_VREF2_ONLY,
    PACK_ALL_CHANNELS,
    PACK_V2_V4_V6,
    PACK_V1_TO_V6,
    PACK_V1_TO_V4,
    PACK_V1_V3_V5,
    PACK_V5_TO_V10,
    PACK_V6_TO_V10
} ADC_MODE_PACK_CHANNEL_E;

typedef enum
{
    RAW_CELL_VOLTAGE = 0,
    AVERAGED_CELL_VOLTAGE,
    FILTERED_CELL_VOLTAGE,
    REDUNDANT_CELL_VOLTAGE,
    NUM_CELL_VOLTAGE_TYPES
} CELL_VOLTAGE_TYPE_E;

typedef enum
{
    AUX_VOLTAGES = 0,
    REDUNDANT_AUX_VOLTAGES,
    NUM_AUX_VOLTAGE_TYPES
} AUX_VOLTAGE_TYPE_E;

/* ==================================================================== */
/* ============================== STRUCTS============================== */
/* ==================================================================== */

// typedef struct
// {
//     bool oscillatorCountFast;
//     bool oscillatorCountSlow;
//     bool supplyError;
//     bool supplyErrorSelect;
//     bool thermalShutdown;
//     bool trimError;
//     bool multipleTrimError;
//     bool testModeDetection;
// } ADBMS_SettableFlags;

typedef struct
{
    bool referenceOn;
    COMPARISON_THRESHOLD_E comparisonThreshold;
    // ADBMS_SettableFlags setFlag;
    bool soakEnabled;
    AUX_SOAK_TIME_E soakTime;
    bool gpoPullDownEnabled[NUM_CELL_MONITOR_GPIO];
    DIGITAL_FILTER_SETTING_E digitalFilterSetting;
    bool commBreak;
    bool muteStatus;
    bool snapStatus;
} ADBMS_ConfigACellMonitor;

typedef struct
{
    bool overcurrentAdcsEnabled;

} ADBMS_ConfigAPackMonitor;

typedef struct
{
    float undervoltageThreshold;
    float overvoltageThreshold;
    bool dischargeMonitorEnabled;
    uint32_t dischargeTimeoutMinutes;
    bool dischargeCell[NUM_CELLS_PER_CELL_MONITOR];
} ADBMS_ConfigBCellMonitor;

typedef struct
{
    /* data */
} ADBMS_ConfigBPackMonitor;

typedef struct
{
    // Group A
    float referenceVoltage;
    float dieTemp;

    // Group B
    float digitalSupplyVoltage;
    float analogSupplyVoltage;
    float referenceResistorVoltage;

    // Group C
    uint16_t adcMismatchFlags;
    bool analogUnderVoltage;
    bool analogOverVoltage;
    bool digitalUnderVoltage;
    bool digitalOverVoltage;
    bool primaryAdcTrimError;
    bool primaryAdcMultipleTrimError;
    bool redundantAdcTrimError;
    bool redundantAdcMultipleTrimError;
    bool supplyRailDeltaFault;
    bool supplyRailDeltaFaultLatching;
    bool adcComparisonActive;
    bool spiFault;
    bool sleepDetected;
    bool thermalShutdownDetected;
    bool testModeDetected;
    bool oscillatorFault;
    uint16_t conversionCounter;

    // Group D
    bool cellOvervoltage[NUM_CELLS_PER_CELL_MONITOR];
    bool cellUndervoltage[NUM_CELLS_PER_CELL_MONITOR];
    uint8_t oscillatorCounter;

    // Group E
    bool gpioLogicHigh[NUM_CELL_MONITOR_GPIO];
    uint8_t deviceRevisionCode;

} ADBMS_StatusGroupCellMonitor;

typedef struct
{
    // Group A
    float regulatorVoltage;
    float dieTemp1;
    float referenceVoltage;

    // Group B
    float groundPadVoltage;
    float digitalSupplyVoltage;
    float supplyVoltage;

    // Group C
    bool overcurrent1Fault;
    bool overcurrent2Fault;
    bool overcurrent3Fault;
    bool overcurrentReferenceFault;
    bool overcurrentVoterAFault;
    bool overcurrentVoterBFault;
    bool overcurrentAGateDrainFault;
    bool overcurrentBGateDrainFault;
    bool overcurrentMismatchFault;

    uint16_t conversionCounter1;
    uint8_t conversionCounter2;

    bool primaryAdcTrimError;
    bool redundantAdcTrimError;
    bool primaryAdcMultipleTrimError;
    bool redundantAdcMultipleTrimError;
    bool regulatorUnderVoltage;
    bool regulatorOverVoltage;
    bool supplyUnderVoltage;
    bool driveUnderVoltage;
    bool digitalSupplyUnderVoltage;
    bool digitalSupplyOverVoltage;
    bool voltageDomainFault;
    bool voltageDomainDiagnositcFault;
    bool analogUnderVoltage;
    bool analogOverVoltage;
    bool oscillatorMismatchFault;
    bool oscillatorStuckFault;
    bool spiFault;
    bool resetDetected;
    bool thermalShutdownDetected;
    bool testModeDetected;

    // Group D
    uint8_t oscillatorCounter;
    float dieTemp2;
    float referenceResistorVoltage;

    // Group E
    bool gpioLogicHigh[NUM_CELL_MONITOR_GPIO];
    uint8_t deviceRevisionCode;
    uint8_t deviceDerivativeCode;
    bool overcurrentAPinHigh;
    bool overcurrentBPinHigh;
    bool currentAdc1Ready;
    bool currentAdc2Ready;
    bool gpioHigh[NUM_PACK_MONITOR_GPIO];
    bool gpoLowLevelHigh[NUM_PACK_MONITOR_GPO];
    bool gpoHighLevelHigh[NUM_PACK_MONITOR_GPO];

} ADBMS_StatusGroupPackMonitor;

typedef struct
{
    float packCurrent1;
    float packCurrent2;
    float packVoltage1;
    float packVoltage2;
    float currentAccumulator1;
    float currentAccumulator2;
    float voltageAccumulator1;
    float voltageAccumulator2;

    float auxVoltage[10];



} ADBMS_PackMonitorData;


typedef struct
{
    float auxVoltage[NUM_CELL_MONITOR_GPIO];
    float switchVoltage;
    float supplyVoltage;
} ADBMS_AuxVoltageGroup;


/* ==================================================================== */
/* =================== GLOBAL FUNCTION DEFINITIONS ==================== */
/* ==================================================================== */

//Relink chain?

/**
 * @brief
 * @param chainInfo Chain data struct
 * @param
 * @return Transaction status error code
 */
TRANSACTION_STATUS_E startCellConversions(CHAIN_INFO_S *chainInfo, ADC_MODE_REDUNDANT_E redundantMode, ADC_MODE_CONTINOUS_E continousMode, ADC_MODE_DISCHARGE_E dischargeMode, ADC_MODE_FILTER_E filterMode, ADC_MODE_CELL_OPEN_WIRE_E openWireMode);

/**
 * @brief
 * @param chainInfo Chain data struct
 * @param
 * @return Transaction status error code
 */
TRANSACTION_STATUS_E startRedundantCellConversions(CHAIN_INFO_S *chainInfo, ADC_MODE_CONTINOUS_E continousMode, ADC_MODE_DISCHARGE_E dischargeMode, ADC_MODE_CELL_OPEN_WIRE_E openWireMode);

/**
 * @brief
 * @param chainInfo Chain data struct
 * @param
 * @return Transaction status error code
 */
TRANSACTION_STATUS_E startAuxConversions(CHAIN_INFO_S *chainInfo, ADC_MODE_AUX_CHANNEL_E auxChannel, ADC_MODE_AUX_OPEN_WIRE_E openWireMode);

/**
 * @brief
 * @param chainInfo Chain data struct
 * @param
 * @return Transaction status error code
 */
TRANSACTION_STATUS_E startRedundantAuxConversions(CHAIN_INFO_S *chainInfo, ADC_MODE_AUX_CHANNEL_E auxChannel);

/**
 * @brief
 * @param chainInfo Chain data struct
 * @param
 * @return Transaction status error code
 */
TRANSACTION_STATUS_E startPackVoltageConversions(CHAIN_INFO_S *chainInfo, ADC_MODE_PACK_CHANNEL_E packChannel, ADC_MODE_PACK_OPEN_WIRE_E openWireMode);

/**
 * @brief
 * @param chainInfo Chain data struct
 * @param
 * @return Transaction status error code
 */
TRANSACTION_STATUS_E startPackAuxillaryConversions(CHAIN_INFO_S *chainInfo);

/**
 * @brief
 * @param chainInfo Chain data struct
 * @param
 * @return Transaction status error code
 */
TRANSACTION_STATUS_E clearAllVoltageRegisters(CHAIN_INFO_S *chainInfo);

/**
 * @brief
 * @param chainInfo Chain data struct
 * @param
 * @return Transaction status error code
 */
TRANSACTION_STATUS_E muteDischarge(CHAIN_INFO_S *chainInfo);

/**
 * @brief
 * @param chainInfo Chain data struct
 * @param
 * @return Transaction status error code
 */
TRANSACTION_STATUS_E unmuteDischarge(CHAIN_INFO_S *chainInfo);

/**
 * @brief
 * @param chainInfo Chain data struct
 * @param
 * @return Transaction status error code
 */
TRANSACTION_STATUS_E freezeRegisters(CHAIN_INFO_S *chainInfo);

/**
 * @brief
 * @param chainInfo Chain data struct
 * @param
 * @return Transaction status error code
 */
TRANSACTION_STATUS_E unfreezeRegisters(CHAIN_INFO_S *chainInfo);

/**
 * @brief
 * @param chainInfo Chain data struct
 * @param
 * @return Transaction status error code
 */
TRANSACTION_STATUS_E softReset(CHAIN_INFO_S *chainInfo);

/**
 * @brief
 * @param chainInfo Chain data struct
 * @param
 * @return Transaction status error code
 */
TRANSACTION_STATUS_E clearAllFlags(CHAIN_INFO_S *chainInfo);

/**
 * @brief
 * @param chainInfo Chain data struct
 * @param
 * @return Transaction status error code
 */
TRANSACTION_STATUS_E writeNVM(CHAIN_INFO_S *chainInfo, uint8_t *writeData);

/**
 * @brief
 * @param chainInfo Chain data struct
 * @param
 * @return Transaction status error code
 */
TRANSACTION_STATUS_E readNVM(CHAIN_INFO_S *chainInfo, uint8_t *readData);

/**
 * @brief
 * @param chainInfo Chain data struct
 * @param
 * @return Transaction status error code
 */
TRANSACTION_STATUS_E writeConfigA(CHAIN_INFO_S *chainInfo, ADBMS_ConfigACellMonitor *cellMonitorConfigA, ADBMS_ConfigAPackMonitor *packMonitorConfigA);

/**
 * @brief
 * @param chainInfo Chain data struct
 * @param
 * @return Transaction status error code
 */
TRANSACTION_STATUS_E readConfigA(CHAIN_INFO_S *chainInfo, ADBMS_ConfigACellMonitor *cellMonitorConfigA, ADBMS_ConfigAPackMonitor *packMonitorConfigA);

/**
 * @brief
 * @param chainInfo Chain data struct
 * @param
 * @return Transaction status error code
 */
TRANSACTION_STATUS_E writeConfigB(CHAIN_INFO_S *chainInfo, ADBMS_ConfigBCellMonitor *cellMonitorConfigB, ADBMS_ConfigBPackMonitor *packMonitorConfigB);

/**
 * @brief
 * @param chainInfo Chain data struct
 * @param
 * @return Transaction status error code
 */
TRANSACTION_STATUS_E readConfigB(CHAIN_INFO_S *chainInfo, ADBMS_ConfigBCellMonitor *cellMonitorConfigB, ADBMS_ConfigBPackMonitor *packMonitorConfigB);

/**
 * @brief
 * @param chainInfo Chain data struct
 * @param
 * @return Transaction status error code
 */
TRANSACTION_STATUS_E writePwmRegisters(CHAIN_INFO_S *chainInfo, uint8_t *dischargeDutyCycle);

/**
 * @brief
 * @param chainInfo Chain data struct
 * @param
 * @return Transaction status error code
 */
TRANSACTION_STATUS_E readPwmRegisters(CHAIN_INFO_S *chainInfo, uint8_t *dischargeDutyCycle);

/**
 * @brief
 * @param chainInfo Chain data struct
 * @param
 * @return Transaction status error code
 */
TRANSACTION_STATUS_E readCellVoltages(CHAIN_INFO_S *chainInfo, float *cellVoltageArr, CELL_VOLTAGE_TYPE_E cellVoltageType);

/**
 * @brief
 * @param chainInfo Chain data struct
 * @param
 * @return Transaction status error code
 */
TRANSACTION_STATUS_E readAuxVoltages(CHAIN_INFO_S *chainInfo, ADBMS_AuxVoltageGroup *auxVoltageGroup, AUX_VOLTAGE_TYPE_E auxVoltageType);

/**
 * @brief
 * @param chainInfo Chain data struct
 * @param
 * @return Transaction status error code
 */
TRANSACTION_STATUS_E readStatus(CHAIN_INFO_S *chainInfo, ADBMS_StatusGroupCellMonitor *cellMonitorStatusGroup, ADBMS_StatusGroupPackMonitor *packMonitorStatusGroup);

/**
 * @brief
 * @param chainInfo Chain data struct
 * @param
 * @return Transaction status error code
 */
TRANSACTION_STATUS_E readSerialId(CHAIN_INFO_S *chainInfo, uint8_t *serialId);

#endif /* INC_ADBMS_H_ */
