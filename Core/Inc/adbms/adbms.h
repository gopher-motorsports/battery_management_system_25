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

#define NUM_PACK_AUX_VOLTAGES       10
#define NUM_PACK_RD_AUX_VOLTAGES    6

#define REGISTER_BYTE0      0
#define REGISTER_BYTE1      1
#define REGISTER_BYTE2      2
#define REGISTER_BYTE3      3
#define REGISTER_BYTE4      4
#define REGISTER_BYTE5      5

#define DEVICE_ID_MASK      0x0E
#define CELL_MONITOR_ID     0x06
#define PACK_MONITOR_ID     0x0C

#define MAX_CELLV_SENSOR_VALUE  6.41505f
#define MIN_CELLV_SENSOR_VALUE  -3.4152f

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
    AUX_SOAK_DISABLED =     0x0,
    AUX_SOAK_TIME_32_US =   0x10,
    AUX_SOAK_TIME_64_US =   0x11,
    AUX_SOAK_TIME_128_US =  0x12,
    AUX_SOAK_TIME_256_US =  0x13,
    AUX_SOAK_TIME_512_US =  0x14,
    AUX_SOAK_TIME_1_MS =    0x15,
    AUX_SOAK_TIME_2_MS =    0x16,
    AUX_SOAK_TIME_4_1_MS =  0x18,
    AUX_SOAK_TIME_8_2_MS =  0x19,
    AUX_SOAK_TIME_16_4_MS = 0x1A,
    AUX_SOAK_TIME_32_8_MS = 0x1B,
    AUX_SOAK_TIME_65_5_MS = 0x1C,
    AUX_SOAK_TIME_131_MS =  0x1D,
    AUX_SOAK_TIME_262_MS =  0x1E,
    AUX_SOAK_TIME_524_MS =  0x1F
} AUX_SOAK_TIME_E;

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
    VOLTAGE_SOAK_TIME_DISABLED = 0,
    VOLTAGE_SOAK_TIME_100_US,
    VOLTAGE_SOAK_TIME_500_US,
    VOLTAGE_SOAK_TIME_1_MS,
    VOLTAGE_SOAK_TIME_2_MS,
    VOLTAGE_SOAK_TIME_10_MS,
    VOLTAGE_SOAK_TIME_20_MS,
    VOLTAGE_SOAK_TIME_150_MS
} VOLTAGE_SOAK_TIME_E;

typedef enum
{
    BASIC_REF_SGND = 0,
    BASIC_REF_1_25V
} BASIC_REFERENCE_VOLTAGE_SETTING_E;

typedef enum
{
    ADVANCED_REF_SGND = 0,
    ADVANCED_REF_1_25V,
    ADVANCED_REF_V3,
    ADVANCED_REF_V4
} ADVANCED_REFERENCE_VOLTAGE_SETTING_E;

typedef enum
{
    ACCUMULATE_4_SAMPLES = 0,
    ACCUMULATE_8_SAMPLES,
    ACCUMULATE_12_SAMPLES,
    ACCUMULATE_16_SAMPLES,
    ACCUMULATE_20_SAMPLES,
    ACCUMULATE_24_SAMPLES,
    ACCUMULATE_28_SAMPLES,
    ACCUMULATE_32_SAMPLES
} ACCUMULATION_COUNTER_SETTING_E;

typedef enum
{
    DEGLITCH_DISABLED = 0,
    DEGLITCH_2_OUT_OF_3,
    DEGLITCH_4_OUT_OF_8,
    DEGLITCH_7_OUT_OF_8
} OVERCURRENT_DEGLITCH_SETTING_E;

typedef enum
{
    OVERCURRENT_GAIN_5_mV = 0,
    OVERCURRENT_GAIN_2_5_mV
} OVERCURRENT_GAIN_CONTROL_SETTING_E;

typedef enum
{
    OVERCURRENT_OUTPUTS_DISABLED = 0,
    OVERCURRENT_OUTPUTS_PWM1,
    OVERCURRENT_OUTPUTS_PWM2,
    OVERCURRENT_OUTPUTS_STATIC
} OVERCURRENT_OUTPUT_SETTING_E;

typedef enum
{
    NO_INJECTION_CONVERT_REGULAR = 0,
    INJECT_IxA_VBATT,
    INJECT_IxB_SGND,
    INJECT_SxA_CONVERT_SxA_VS_IxA,
    CONVERT_SxA_VS_IxA_SGND_VS_SGND,
    CONVERT_VDIV,
    CONVERT_SCALED_VREF,
    INJECT_IxB_CONVERT_SxA_VS_IxA
} PACK_ADC_DIAGNOSTIC_SETTING_E;

typedef enum
{
    REGULAR_CLOCK_DIAGNOSTIC_MODE = 0,
    FASTER_CLOCK_DIAGNOSTIC_MODE,
    CLOCK_STUCK_HIGH_DIAGNOSTIC_MODE,
    CLOCK_STUCK_LOW_DIAGNOSTIC_MODE
} CLOCK_MONITOR_DIAGNOSTIC_SETTING_E;

typedef enum
{
    REGULAR_SUPPLY_DIAGNOSTIC_MODE = 0,
    FORCE_MISMATCH_DIAGNOSTIC_MODE,
    FORCE_UNDERVOLTAGE_DIAGNOSTIC_MODE,
    FORCE_OVERVOLTAGE_DIAGNOSTIC_MODE
} SUPPLY_MONITOR_DIAGNOSTIC_SETTING_E;

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
    NUM_CELL_VOLTAGE_TYPES
} CELL_VOLTAGE_TYPE_E;

typedef enum
{
    AUX_VOLTAGES = 0,
    REDUNDANT_AUX_VOLTAGES,
    NUM_AUX_VOLTAGE_TYPES
} AUX_VOLTAGE_TYPE_E;

typedef enum
{
    PACK_ADC1 = 0,
    PACK_ADC2,
    NUM_PACK_ADCS
} PACK_ADC_TYPE_E;

/* ==================================================================== */
/* ============================== STRUCTS============================== */
/* ==================================================================== */

// Start Cell Monitor Structure

typedef struct __attribute__((packed))
{
    // Byte 0
    COMPARISON_THRESHOLD_E comparisonThreshold : 3;
    uint8_t reserved1 : 4;
    uint8_t referenceOn : 1;

    // Byte 1
    uint8_t forceOscillatorCounterFast : 1;
    uint8_t forceOscillatorCounterSlow : 1;
    uint8_t asssertSupplyError : 1;
    uint8_t supplyErrorSelect : 1;
    uint8_t assertThermalShutdownDetectedFault : 1;
    uint8_t asserSingleTrimError : 1;
    uint8_t assertMultipleTrimError : 1;
    uint8_t assertTestModeDetectedFault : 1;

    // Byte 2
    uint8_t reserved2 : 3;
    AUX_SOAK_TIME_E soakTime : 5;

    // Byte 3
    uint8_t gpo1State : 1;
    uint8_t gpo2State : 1;
    uint8_t gpo3State : 1;
    uint8_t gpo4State : 1;
    uint8_t gpo5State : 1;
    uint8_t gpo6State : 1;
    uint8_t gpo7State : 1;
    uint8_t gpo8State : 1;

    // Byte 4
    uint8_t gpo9State : 1;
    uint8_t gpo10State : 1;
    uint8_t reserved3 : 6;

    // Byte 5
    DIGITAL_FILTER_SETTING_E digitalFilterSetting : 3;
    uint8_t commBreak : 1;
    uint8_t muteStatus : 1;
    uint8_t snapStatus : 1;
    uint8_t reserved4 : 2;

} ADBMS_ConfigACellMonitor;

typedef struct __attribute__((packed))
{
    float undervoltageThreshold;
    float overvoltageThreshold;
    uint32_t dischargeTimeoutMinutes;
    bool dischargeCell[NUM_CELLS_PER_CELL_MONITOR];
} ADBMS_ConfigBCellMonitor;

typedef struct __attribute__((packed))
{
    float referenceVoltage;
    float dieTemp;
} ADBMS_StatusACellMonitor;

typedef struct __attribute__((packed))
{
    float digitalSupplyVoltage;
    float analogSupplyVoltage;
    float referenceResistorVoltage;
} ADBMS_StatusBCellMonitor;

typedef struct __attribute__((packed))
{
    uint32_t conversionCounter;

    uint8_t redundantAdcMultipleTrimError : 1;
    uint8_t redundantAdcTrimError : 1;
    uint8_t primaryAdcMultipleTrimError : 1;
    uint8_t primaryAdcTrimError : 1;
    uint8_t digitalUnderVoltage : 1;
    uint8_t digitalOverVoltage : 1;
    uint8_t analogUnderVoltage : 1;
    uint8_t analogOverVoltage : 1;

    uint8_t oscillatorFault : 1;
    uint8_t testModeDetected : 1;
    uint8_t thermalShutdownDetected : 1;
    uint8_t sleepDetected : 1;
    uint8_t spiFault : 1;
    uint8_t adcComparisonActive : 1;
    uint8_t supplyRailDeltaFault : 1;
    uint8_t supplyRailDeltaFaultLatching : 1;

    uint8_t cellAdcMismatchFault[NUM_CELLS_PER_CELL_MONITOR];
} ADBMS_StatusCCellMonitor;

typedef struct __attribute__((packed))
{
    uint8_t cellUnderVoltageFault[NUM_CELLS_PER_CELL_MONITOR];
    uint8_t cellOverVoltageFault[NUM_CELLS_PER_CELL_MONITOR];
    uint8_t oscillatorCounter;
} ADBMS_StatusDCellMonitor;

typedef struct __attribute__((packed))
{
    uint8_t gpi1State : 1;
    uint8_t gpi2State : 1;
    uint8_t gpi3State : 1;
    uint8_t gpi4State : 1;
    uint8_t gpi5State : 1;
    uint8_t gpi6State : 1;
    uint8_t gpi7State : 1;
    uint8_t gpi8State : 1;

    uint8_t gpi9State : 1;
    uint8_t gpi10State : 1;
    uint8_t reserved1 : 2;
    uint8_t revisionCode : 4;
} ADBMS_StatusECellMonitor;

typedef struct __attribute__((packed))
{
    ADBMS_ConfigACellMonitor configGroupA;
    ADBMS_ConfigBCellMonitor configGroupB;

    ADBMS_StatusACellMonitor statusGroupA;
    ADBMS_StatusBCellMonitor statusGroupB;
    ADBMS_StatusCCellMonitor statusGroupC;
    ADBMS_StatusDCellMonitor statusGroupD;
    ADBMS_StatusECellMonitor statusGroupE;

    float cellVoltage[NUM_CELLS_PER_CELL_MONITOR];
    float redundantCellVoltage[NUM_CELLS_PER_CELL_MONITOR];

    float auxVoltage[NUM_CELL_MONITOR_GPIO];
    float reduntantAuxVoltage[NUM_CELL_MONITOR_GPIO];

    float hvSupplyVoltage;
    float switch1Voltage;

    float dischargePWM[NUM_CELLS_PER_CELL_MONITOR];

    uint8_t serialId[REGISTER_SIZE_BYTES];
    uint8_t retentionRegister[REGISTER_SIZE_BYTES];

} ADBMS_CellMonitorData;

typedef struct __attribute__((packed))
{
    // Byte 0
    ADVANCED_REFERENCE_VOLTAGE_SETTING_E v1Reference : 2;
    ADVANCED_REFERENCE_VOLTAGE_SETTING_E v2Reference : 2;
    BASIC_REFERENCE_VOLTAGE_SETTING_E v3Reference : 1;
    BASIC_REFERENCE_VOLTAGE_SETTING_E v4Reference : 1;
    BASIC_REFERENCE_VOLTAGE_SETTING_E v5Reference : 1;
    uint8_t overcurrentAdcsEnabled : 1;

    // Byte 1
    CLOCK_MONITOR_DIAGNOSTIC_SETTING_E clockDiagnosticMode : 2;
    SUPPLY_MONITOR_DIAGNOSTIC_SETTING_E supplyDiagnosticMode : 2;
    uint8_t assertThermalShutdownDetectedFault : 1;
    uint8_t reserved1 : 1;
    uint8_t assertTrimError : 1;
    uint8_t assertTestModeDetectedFault : 1;

    // Byte 2
    BASIC_REFERENCE_VOLTAGE_SETTING_E v6Reference : 1;
    BASIC_REFERENCE_VOLTAGE_SETTING_E v7Reference : 1;
    BASIC_REFERENCE_VOLTAGE_SETTING_E v8Reference : 1;
    BASIC_REFERENCE_VOLTAGE_SETTING_E v9Reference : 1;
    BASIC_REFERENCE_VOLTAGE_SETTING_E v10Reference : 1;
    VOLTAGE_SOAK_TIME_E soakTime : 3;

    // Byte 3
    uint8_t gpo1State : 1;
    uint8_t gpo2State : 1;
    uint8_t gpo3State : 1;
    uint8_t gpo4State : 1;
    uint8_t gpo5State : 1;
    uint8_t gpo6State : 2;
    uint8_t reserved2 : 1;

    // Byte 4
    uint8_t gpo1HighZMode : 1;
    uint8_t gpo2HighZMode : 1;
    uint8_t gpo3HighZMode : 1;
    uint8_t gpo4HighZMode : 1;
    uint8_t gpo5HighZMode : 1;
    uint8_t gpo6HighZMode : 1;
    uint8_t gpio1FaultOutputEnable : 1;
    uint8_t spiModeSelect : 1;

    // Byte 5
    ACCUMULATION_COUNTER_SETTING_E accumulationSetting : 3;
    uint8_t commBreak : 1;
    uint8_t referenceOn : 1;
    uint8_t snapStatus : 1;
    uint8_t packVoltage1DifferentialMode : 1;
    uint8_t packVoltage2DifferentialMode : 1;

} ADBMS_ConfigAPackMonitor;

typedef struct __attribute__((packed))
{
    uint8_t oc1Threshold;
    uint8_t oc2Threshold;
    uint8_t oc3Threshold;

    OVERCURRENT_DEGLITCH_SETTING_E ocDeglitchMode : 2;
    uint8_t reserved1 : 1;
    uint8_t ocReducedSafetyInterval : 1;
    uint8_t reserved2 : 4;

    uint8_t ocPinOpenDrainMode: 1;
    OVERCURRENT_GAIN_CONTROL_SETTING_E oc1GainControl : 1;
    OVERCURRENT_GAIN_CONTROL_SETTING_E oc2GainControl : 1;
    OVERCURRENT_GAIN_CONTROL_SETTING_E oc3GainControl : 1;
    OVERCURRENT_OUTPUT_SETTING_E ocOutputMode : 2;
    uint8_t ocAOutputInverted : 1;
    uint8_t ocBOutputInverted : 1;

    PACK_ADC_DIAGNOSTIC_SETTING_E adcDiagnosticMode : 3;
    uint8_t gpio2ConversionSignalEnable: 1;
    uint8_t gpio1State : 1;
    uint8_t gpio2State : 1;
    uint8_t gpio3State : 1;
    uint8_t gpio4State : 1;

} ADBMS_ConfigBPackMonitor;

typedef struct __attribute__((packed))
{
    float referenceVoltage1P25;
    float dieTemp1;
    float regulatorVoltage;
} ADBMS_StatusAPackMonitor;

typedef struct __attribute__((packed))
{
    float digitalSupplyVoltage;
    float supplyVoltage;
    float groundPadVoltage;
} ADBMS_StatusBPackMonitor;

typedef struct __attribute__((packed))
{
    uint8_t overcurrent1Fault : 1;
    uint8_t overcurrentVoterAFault : 1;
    uint8_t overcurrentAGateDrainFault : 1;
    uint8_t overcurrent3Fault : 1;
    uint8_t overcurrentMismatchFault : 1;
    uint8_t driveUnderVoltage : 1;
    uint8_t reserved1 : 2;

    uint8_t overcurrent2Fault : 1;
    uint8_t overcurrentVoterBFault : 1;
    uint8_t overcurrentBGateDrainFault : 1;
    uint8_t overcurrentReferenceFault : 1;
    uint8_t oscillatorStuckFault : 1;
    uint8_t supplyUnderVoltage : 1;
    uint8_t reserved2 : 2;

    uint16_t conversionCounter1 : 13;
    uint16_t conversionCounter2 : 3;

    uint8_t redundantAdcMultipleTrimError : 1;
    uint8_t redundantAdcTrimError : 1;
    uint8_t primaryAdcMultipleTrimError : 1;
    uint8_t primaryAdcTrimError : 1;
    uint8_t digitalSupplyUnderVoltage : 1;
    uint8_t digitalSupplyOverVoltage : 1;
    uint8_t regulatorUnderVoltage : 1;
    uint8_t regulatorOverVoltage : 1;

    uint8_t oscillatorMismatchFault : 1;
    uint8_t testModeDetected : 1;
    uint8_t thermalShutdownDetected : 1;
    uint8_t resetDetected : 1;
    uint8_t spiFault : 1;
    uint8_t voltageDomainFault : 1;
    uint8_t voltageDomainDiagnositcFault : 1;
} ADBMS_StatusCPackMonitor;

typedef struct __attribute__((packed))
{
    float referenceResistorVoltage;
    float dieTemp2;
    uint8_t oscillatorCounter;
} ADBMS_StatusDPackMonitor;

typedef struct __attribute__((packed))
{
    uint8_t overCurrentPinAState : 1;
    uint8_t overCurrentPinBState : 1;
    uint8_t reserved1 : 6;

    uint8_t derivativeCode : 2;
    uint8_t reserved2 : 4;
    uint8_t currentAdc1Initialized : 1;
    uint8_t currentAdc2Initialized : 1;

    uint8_t reserved3 : 8;

    uint8_t gpo5LowLevelState : 1;
    uint8_t gpo6LowLevelState : 1;
    uint8_t gpo1HighLevelState : 1;
    uint8_t gpo2HighLevelState : 1;
    uint8_t gpo3HighLevelState : 1;
    uint8_t gpo4HighLevelState : 1;
    uint8_t gpo5HighLevelState : 1;
    uint8_t gpo6HighLevelState : 1;

    uint8_t gpio1State : 1;
    uint8_t gpio2State : 1;
    uint8_t gpio3State : 1;
    uint8_t gpio4State : 1;
    uint8_t gpo1LowLevelState : 1;
    uint8_t gpo2LowLevelState : 1;
    uint8_t gpo3LowLevelState : 1;
    uint8_t gpo4LowLevelState : 1;

    uint8_t reserved4 : 4;
    uint8_t revisionCode : 4;

} ADBMS_StatusEPackMonitor;

typedef struct __attribute__((packed))
{
    float overCurrentAdc1;
    float overCurrentAdc2;
    float overCurrentAdc3;
    float overCurrentAdc3Max;
    float overCurrentAdc3Min;
} ADBMS_OvercurrentStatusPackMonitor;

typedef struct __attribute__((packed))
{
    ADBMS_ConfigAPackMonitor configGroupA;
    ADBMS_ConfigBPackMonitor configGroupB;

    ADBMS_StatusAPackMonitor statusGroupA;
    ADBMS_StatusBPackMonitor statusGroupB;
    ADBMS_StatusCPackMonitor statusGroupC;
    ADBMS_StatusDPackMonitor statusGroupD;
    ADBMS_StatusEPackMonitor statusGroupE;

    int32_t currentAdc1uV;
    int32_t currentAdc2uV;

    float batteryVoltage1;
    float batteryVoltage2;

    int32_t currentAdcAccumulator1uV;
    int32_t currentAdcAccumulator2uV;

    int32_t batteryVoltageAccumulator1uV;
    int32_t batteryVoltageAccumulator2uV;

    float auxVoltage[NUM_PACK_AUX_VOLTAGES];
    float redundantAuxVoltage[NUM_PACK_RD_AUX_VOLTAGES];

    float referenceVoltage;
    float redundantReferenceVoltage;

    ADBMS_OvercurrentStatusPackMonitor overcurrentStatusGroup;

    uint8_t serialId[REGISTER_SIZE_BYTES];
} ADBMS_PackMonitorData;

typedef struct
{
    ADBMS_PackMonitorData packMonitor;
    ADBMS_CellMonitorData cellMonitor[8];
    CHAIN_INFO_S chainInfo;
} ADBMS_BatteryData;

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DEFINITIONS ==================== */
/* ==================================================================== */

void wakeChain(ADBMS_BatteryData *adbmsData);

void readyChain(ADBMS_BatteryData *adbmsData);

TRANSACTION_STATUS_E checkChainStatus(ADBMS_BatteryData *adbmsData);

TRANSACTION_STATUS_E startCellConversions(ADBMS_BatteryData *adbmsData, ADC_MODE_REDUNDANT_E redundantMode, ADC_MODE_CONTINOUS_E continousMode, ADC_MODE_DISCHARGE_E dischargeMode, ADC_MODE_FILTER_E filterMode, ADC_MODE_CELL_OPEN_WIRE_E openWireMode);

TRANSACTION_STATUS_E startRedundantCellConversions(ADBMS_BatteryData *adbmsData, ADC_MODE_CONTINOUS_E continousMode, ADC_MODE_DISCHARGE_E dischargeMode, ADC_MODE_CELL_OPEN_WIRE_E openWireMode);

TRANSACTION_STATUS_E startAuxConversions(ADBMS_BatteryData *adbmsData, ADC_MODE_AUX_CHANNEL_E auxChannel, ADC_MODE_AUX_OPEN_WIRE_E openWireMode);

TRANSACTION_STATUS_E startRedundantAuxConversions(ADBMS_BatteryData *adbmsData, ADC_MODE_AUX_CHANNEL_E auxChannel);

TRANSACTION_STATUS_E startPackVoltageConversions(ADBMS_BatteryData *adbmsData, ADC_MODE_PACK_CHANNEL_E packChannel, ADC_MODE_PACK_OPEN_WIRE_E openWireMode);

TRANSACTION_STATUS_E startPackAuxillaryConversions(ADBMS_BatteryData *adbmsData);

TRANSACTION_STATUS_E muteDischarge(ADBMS_BatteryData *adbmsData);

TRANSACTION_STATUS_E unmuteDischarge(ADBMS_BatteryData *adbmsData);

TRANSACTION_STATUS_E freezeRegisters(ADBMS_BatteryData *adbmsData);

TRANSACTION_STATUS_E unfreezeRegisters(ADBMS_BatteryData *adbmsData);

TRANSACTION_STATUS_E softReset(ADBMS_BatteryData *adbmsData);

TRANSACTION_STATUS_E clearAllVoltageRegisters(ADBMS_BatteryData *adbmsData);

TRANSACTION_STATUS_E clearAllFlags(ADBMS_BatteryData *adbmsData);

TRANSACTION_STATUS_E readSerialId(ADBMS_BatteryData *adbmsData);

TRANSACTION_STATUS_E writePwmRegisters(ADBMS_BatteryData *adbmsData);

TRANSACTION_STATUS_E readPwmRegisters(ADBMS_BatteryData *adbmsData);

TRANSACTION_STATUS_E writeNVM(ADBMS_BatteryData  *adbmsData);

TRANSACTION_STATUS_E readNVM(ADBMS_BatteryData *adbmsData);

TRANSACTION_STATUS_E writeConfigA(ADBMS_BatteryData * adbmsData);

TRANSACTION_STATUS_E readConfigA(ADBMS_BatteryData *adbmsData);

TRANSACTION_STATUS_E writeConfigB(ADBMS_BatteryData *adbmsData);

TRANSACTION_STATUS_E readConfigB(ADBMS_BatteryData *adbmsData);

TRANSACTION_STATUS_E readStatusA(ADBMS_BatteryData *adbmsData);

TRANSACTION_STATUS_E readStatusB(ADBMS_BatteryData *adbmsData);

TRANSACTION_STATUS_E readStatusC(ADBMS_BatteryData *adbmsData);

TRANSACTION_STATUS_E readStatusD(ADBMS_BatteryData *adbmsData);

TRANSACTION_STATUS_E readStatusE(ADBMS_BatteryData *adbmsData);

TRANSACTION_STATUS_E readCellVoltages(ADBMS_BatteryData *adbmsData, CELL_VOLTAGE_TYPE_E cellVoltageType);

TRANSACTION_STATUS_E readRedundantCellVoltages(ADBMS_BatteryData *adbmsData);

TRANSACTION_STATUS_E readAuxVoltages(ADBMS_BatteryData * adbmsData);

TRANSACTION_STATUS_E readRedundantAuxVoltages(ADBMS_BatteryData * adbmsData);

#endif /* INC_ADBMS_H_ */
