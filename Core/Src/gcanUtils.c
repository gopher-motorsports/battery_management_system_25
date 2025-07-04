/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include "gcanUtils.h"

/* ==================================================================== */
/* ========================= LOCAL VARIABLES ========================== */
/* ==================================================================== */

const FLOAT_CAN_STRUCT *cellVoltageParams[NUM_CELL_MON_IN_ACCUMULATOR][NUM_CELLS_PER_CELL_MONITOR] =
{
    {&segment1Cell1Voltage_V, &segment1Cell2Voltage_V, &segment1Cell3Voltage_V, &segment1Cell4Voltage_V, &segment1Cell5Voltage_V, &segment1Cell6Voltage_V, &segment1Cell7Voltage_V, &segment1Cell8Voltage_V, &segment1Cell9Voltage_V, &segment1Cell10Voltage_V, &segment1Cell11Voltage_V, &segment1Cell12Voltage_V, &segment1Cell13Voltage_V, &segment1Cell14Voltage_V, &segment1Cell15Voltage_V, &segment1Cell16Voltage_V},
    {&segment2Cell1Voltage_V, &segment2Cell2Voltage_V, &segment2Cell3Voltage_V, &segment2Cell4Voltage_V, &segment2Cell5Voltage_V, &segment2Cell6Voltage_V, &segment2Cell7Voltage_V, &segment2Cell8Voltage_V, &segment2Cell9Voltage_V, &segment2Cell10Voltage_V, &segment2Cell11Voltage_V, &segment2Cell12Voltage_V, &segment2Cell13Voltage_V, &segment2Cell14Voltage_V, &segment2Cell15Voltage_V, &segment2Cell16Voltage_V},
    {&segment3Cell1Voltage_V, &segment3Cell2Voltage_V, &segment3Cell3Voltage_V, &segment3Cell4Voltage_V, &segment3Cell5Voltage_V, &segment3Cell6Voltage_V, &segment3Cell7Voltage_V, &segment3Cell8Voltage_V, &segment3Cell9Voltage_V, &segment3Cell10Voltage_V, &segment3Cell11Voltage_V, &segment3Cell12Voltage_V, &segment3Cell13Voltage_V, &segment3Cell14Voltage_V, &segment3Cell15Voltage_V, &segment3Cell16Voltage_V},
    {&segment4Cell1Voltage_V, &segment4Cell2Voltage_V, &segment4Cell3Voltage_V, &segment4Cell4Voltage_V, &segment4Cell5Voltage_V, &segment4Cell6Voltage_V, &segment4Cell7Voltage_V, &segment4Cell8Voltage_V, &segment4Cell9Voltage_V, &segment4Cell10Voltage_V, &segment4Cell11Voltage_V, &segment4Cell12Voltage_V, &segment4Cell13Voltage_V, &segment4Cell14Voltage_V, &segment4Cell15Voltage_V, &segment4Cell16Voltage_V},
    {&segment5Cell1Voltage_V, &segment5Cell2Voltage_V, &segment5Cell3Voltage_V, &segment5Cell4Voltage_V, &segment5Cell5Voltage_V, &segment5Cell6Voltage_V, &segment5Cell7Voltage_V, &segment5Cell8Voltage_V, &segment5Cell9Voltage_V, &segment5Cell10Voltage_V, &segment5Cell11Voltage_V, &segment5Cell12Voltage_V, &segment5Cell13Voltage_V, &segment5Cell14Voltage_V, &segment5Cell15Voltage_V, &segment5Cell16Voltage_V},
    {&segment6Cell1Voltage_V, &segment6Cell2Voltage_V, &segment6Cell3Voltage_V, &segment6Cell4Voltage_V, &segment6Cell5Voltage_V, &segment6Cell6Voltage_V, &segment6Cell7Voltage_V, &segment6Cell8Voltage_V, &segment6Cell9Voltage_V, &segment6Cell10Voltage_V, &segment6Cell11Voltage_V, &segment6Cell12Voltage_V, &segment6Cell13Voltage_V, &segment6Cell14Voltage_V, &segment6Cell15Voltage_V, &segment6Cell16Voltage_V},
    {&segment7Cell1Voltage_V, &segment7Cell2Voltage_V, &segment7Cell3Voltage_V, &segment7Cell4Voltage_V, &segment7Cell5Voltage_V, &segment7Cell6Voltage_V, &segment7Cell7Voltage_V, &segment7Cell8Voltage_V, &segment7Cell9Voltage_V, &segment7Cell10Voltage_V, &segment7Cell11Voltage_V, &segment7Cell12Voltage_V, &segment7Cell13Voltage_V, &segment7Cell14Voltage_V, &segment7Cell15Voltage_V, &segment7Cell16Voltage_V},
    {&segment8Cell1Voltage_V, &segment8Cell2Voltage_V, &segment8Cell3Voltage_V, &segment8Cell4Voltage_V, &segment8Cell5Voltage_V, &segment8Cell6Voltage_V, &segment8Cell7Voltage_V, &segment8Cell8Voltage_V, &segment8Cell9Voltage_V, &segment8Cell10Voltage_V, &segment8Cell11Voltage_V, &segment8Cell12Voltage_V, &segment8Cell13Voltage_V, &segment8Cell14Voltage_V, &segment8Cell15Voltage_V, &segment8Cell16Voltage_V}
    
};

const FLOAT_CAN_STRUCT *cellTempParams[NUM_CELL_MON_IN_ACCUMULATOR][NUM_CELLS_PER_CELL_MONITOR] =
{
    {&segment1Cell1Temperature_C, &segment1Cell2Temperature_C, &segment1Cell3Temperature_C, &segment1Cell4Temperature_C, &segment1Cell5Temperature_C, &segment1Cell6Temperature_C, &segment1Cell7Temperature_C, &segment1Cell8Temperature_C, &segment1Cell9Temperature_C, &segment1Cell10Temperature_C, &segment1Cell11Temperature_C, &segment1Cell12Temperature_C, &segment1Cell13Temperature_C, &segment1Cell14Temperature_C, &segment1Cell15Temperature_C, &segment1Cell16Temperature_C},
    {&segment2Cell1Temperature_C, &segment2Cell2Temperature_C, &segment2Cell3Temperature_C, &segment2Cell4Temperature_C, &segment2Cell5Temperature_C, &segment2Cell6Temperature_C, &segment2Cell7Temperature_C, &segment2Cell8Temperature_C, &segment2Cell9Temperature_C, &segment2Cell10Temperature_C, &segment2Cell11Temperature_C, &segment2Cell12Temperature_C, &segment2Cell13Temperature_C, &segment2Cell14Temperature_C, &segment2Cell15Temperature_C, &segment2Cell16Temperature_C},
    {&segment3Cell1Temperature_C, &segment3Cell2Temperature_C, &segment3Cell3Temperature_C, &segment3Cell4Temperature_C, &segment3Cell5Temperature_C, &segment3Cell6Temperature_C, &segment3Cell7Temperature_C, &segment3Cell8Temperature_C, &segment3Cell9Temperature_C, &segment3Cell10Temperature_C, &segment3Cell11Temperature_C, &segment3Cell12Temperature_C, &segment3Cell13Temperature_C, &segment3Cell14Temperature_C, &segment3Cell15Temperature_C, &segment3Cell16Temperature_C},
    {&segment4Cell1Temperature_C, &segment4Cell2Temperature_C, &segment4Cell3Temperature_C, &segment4Cell4Temperature_C, &segment4Cell5Temperature_C, &segment4Cell6Temperature_C, &segment4Cell7Temperature_C, &segment4Cell8Temperature_C, &segment4Cell9Temperature_C, &segment4Cell10Temperature_C, &segment4Cell11Temperature_C, &segment4Cell12Temperature_C, &segment4Cell13Temperature_C, &segment4Cell14Temperature_C, &segment4Cell15Temperature_C, &segment4Cell16Temperature_C},
    {&segment5Cell1Temperature_C, &segment5Cell2Temperature_C, &segment5Cell3Temperature_C, &segment5Cell4Temperature_C, &segment5Cell5Temperature_C, &segment5Cell6Temperature_C, &segment5Cell7Temperature_C, &segment5Cell8Temperature_C, &segment5Cell9Temperature_C, &segment5Cell10Temperature_C, &segment5Cell11Temperature_C, &segment5Cell12Temperature_C, &segment5Cell13Temperature_C, &segment5Cell14Temperature_C, &segment5Cell15Temperature_C, &segment5Cell16Temperature_C},
    {&segment6Cell1Temperature_C, &segment6Cell2Temperature_C, &segment6Cell3Temperature_C, &segment6Cell4Temperature_C, &segment6Cell5Temperature_C, &segment6Cell6Temperature_C, &segment6Cell7Temperature_C, &segment6Cell8Temperature_C, &segment6Cell9Temperature_C, &segment6Cell10Temperature_C, &segment6Cell11Temperature_C, &segment6Cell12Temperature_C, &segment6Cell13Temperature_C, &segment6Cell14Temperature_C, &segment6Cell15Temperature_C, &segment6Cell16Temperature_C},
    {&segment7Cell1Temperature_C, &segment7Cell2Temperature_C, &segment7Cell3Temperature_C, &segment7Cell4Temperature_C, &segment7Cell5Temperature_C, &segment7Cell6Temperature_C, &segment7Cell7Temperature_C, &segment7Cell8Temperature_C, &segment7Cell9Temperature_C, &segment7Cell10Temperature_C, &segment7Cell11Temperature_C, &segment7Cell12Temperature_C, &segment7Cell13Temperature_C, &segment7Cell14Temperature_C, &segment7Cell15Temperature_C, &segment7Cell16Temperature_C},
    {&segment8Cell1Temperature_C, &segment8Cell2Temperature_C, &segment8Cell3Temperature_C, &segment8Cell4Temperature_C, &segment8Cell5Temperature_C, &segment8Cell6Temperature_C, &segment8Cell7Temperature_C, &segment8Cell8Temperature_C, &segment8Cell9Temperature_C, &segment8Cell10Temperature_C, &segment8Cell11Temperature_C, &segment8Cell12Temperature_C, &segment8Cell13Temperature_C, &segment8Cell14Temperature_C, &segment8Cell15Temperature_C, &segment8Cell16Temperature_C}
    
};

const FLOAT_CAN_STRUCT *cellStatParams[NUM_CELL_MON_IN_ACCUMULATOR][NUM_STAT_PARAMS] =
{
    {&segment1MaxCellVoltage_V, &segment1MinCellVoltage_V, &segment1AvgCellVoltage_V, &segment1DieTemperature_C, &segment1MaxCellTemperature_C, &segment1MinCellTemperature_C, &segment1AvgCellTemperature_C, &segment1BoardTemperature_C},
    {&segment2MaxCellVoltage_V, &segment2MinCellVoltage_V, &segment2AvgCellVoltage_V, &segment2DieTemperature_C, &segment2MaxCellTemperature_C, &segment2MinCellTemperature_C, &segment2AvgCellTemperature_C, &segment2BoardTemperature_C},
    {&segment3MaxCellVoltage_V, &segment3MinCellVoltage_V, &segment3AvgCellVoltage_V, &segment3DieTemperature_C, &segment3MaxCellTemperature_C, &segment3MinCellTemperature_C, &segment3AvgCellTemperature_C, &segment3BoardTemperature_C},
    {&segment4MaxCellVoltage_V, &segment4MinCellVoltage_V, &segment4AvgCellVoltage_V, &segment4DieTemperature_C, &segment4MaxCellTemperature_C, &segment4MinCellTemperature_C, &segment4AvgCellTemperature_C, &segment4BoardTemperature_C},
    {&segment5MaxCellVoltage_V, &segment5MinCellVoltage_V, &segment5AvgCellVoltage_V, &segment5DieTemperature_C, &segment5MaxCellTemperature_C, &segment5MinCellTemperature_C, &segment5AvgCellTemperature_C, &segment5BoardTemperature_C},
    {&segment6MaxCellVoltage_V, &segment6MinCellVoltage_V, &segment6AvgCellVoltage_V, &segment6DieTemperature_C, &segment6MaxCellTemperature_C, &segment6MinCellTemperature_C, &segment6AvgCellTemperature_C, &segment6BoardTemperature_C},
    {&segment7MaxCellVoltage_V, &segment7MinCellVoltage_V, &segment7AvgCellVoltage_V, &segment7DieTemperature_C, &segment7MaxCellTemperature_C, &segment7MinCellTemperature_C, &segment7AvgCellTemperature_C, &segment7BoardTemperature_C},
    {&segment8MaxCellVoltage_V, &segment8MinCellVoltage_V, &segment8AvgCellVoltage_V, &segment8DieTemperature_C, &segment8MaxCellTemperature_C, &segment8MinCellTemperature_C, &segment8AvgCellTemperature_C, &segment8BoardTemperature_C}
};

const U8_CAN_STRUCT *bmsAlertsParams[NUM_GCAN_ALERTS] =
{
    &overvoltageWarningAlert_state,
    &undervoltageWarningAlert_state,
    &overvoltageFaultAlert_state,
    &undervoltageFaultAlert_state,
    &cellImbalanceAlert_state,
    &overtempWarningAlert_state,
    &overtempFaultAlert_state,
    &badVoltageSenseStatusAlert_state,
    &badBrickTempSenseStatusAlert_state,
    &badBoardTempSenseStatusAlert_state,
    &insufficientTempSensorsAlert_state,
    &telemetryCommunicationAlert_state,
    &packOvercurrentFaultAlert_state
};