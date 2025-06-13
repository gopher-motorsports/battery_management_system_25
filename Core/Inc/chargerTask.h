#ifndef INC_CHARGER_TASK_H_
#define INC_CHARGER_TASK_H_

/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

/* ==================================================================== */
/* ============================= DEFINES ============================== */
/* ==================================================================== */

/* ==================================================================== */
/* ========================= ENUMERATED TYPES========================== */
/* ==================================================================== */

typedef enum
{
    CHARGER_STATE_DISCONNECTED = 0,
    CHARGER_STATE_CONSTANT_CURRENT,
    CHARGER_STATE_CONSTANT_VOLTAGE,
    CHARGER_STATE_BALANCING,
    CHARGER_STATE_COMPLETE,
    NUM_CHARGER_STATES
} CHARGER_STATE_E;

/* ==================================================================== */
/* ============================== STRUCTS ============================= */
/* ==================================================================== */

typedef struct __attribute__((packed))
{
    uint8_t chargerHardwareFailure : 1;
    uint8_t chargerOverTemp : 1;
    uint8_t chargerWrongInputVoltage : 1;
    uint8_t chargerNoBatteryDetected : 1;
    uint8_t chargerNoComms : 1;
    uint8_t reserved : 3;
} chargerStatus_S;

typedef struct
{
    CHARGER_STATE_E chargerState;

    float chargerPowerLimit;

    float chargerVoltageSetpoint;
    float chargerCurrentSetpoint;

    float chargerVoltage;
    float chargerCurrent;

    chargerStatus_S chargerStatus;
} chargerTaskData_S;

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DECLARATIONS =================== */
/* ==================================================================== */

void initChargerTask();
void runChargerTask();


#endif /* INC_CHARGER_TASK_H_ */
