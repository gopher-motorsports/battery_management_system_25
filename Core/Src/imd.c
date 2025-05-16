/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */
#include "main.h"
#include "imd.h"

/* ==================================================================== */
/* ============================= DEFINES ============================== */
/* ==================================================================== */

// Timeout of IMD PWM signal in milliseconds
#define IMD_PWM_TIMOUT_MS   200

// Bounds of imd frequency range
#define IMD_MAX_FREQ_HZ     50
#define IMD_MIN_FREQ_HZ     0

// Speed start duty cycle mus be less than threshold to indicate pass
#define IMD_SPEED_START_PASS_DUTY_THRES 15

// Imd pwm duty must be less than threshold to indicate high isolation resistance
#define IMD_HIGH_RESISTANCE_DUTY_THRES  5
#define IMD_HIGH_RESISTANCE_VALUE_KOHM  50000

// Imd pwm duty must be greater than threshold to indicate low isolation resistance
#define IMD_LOW_RESISTANCE_DUTY_THRES   95
#define IMD_LOW_RESISTANCE_VALUE_KOHM   0

// Values used to calculate isolation resistance
#define IMD_IR_PERCENT_RANGE            90
#define IMD_IR_PERCENT_OFFSET           5
#define IMD_IR_SCALE_KOHM               1200

/* ==================================================================== */
/* ============================== MACROS ============================== */
/* ==================================================================== */

// Convert imd duty cycle measurement to isolation resistance
// imdDuty much be >= 5
#define GET_ISOLATION_RESISTANCE(imdDuty)   (((IMD_IR_PERCENT_RANGE * IMD_IR_SCALE_KOHM) / (imdDuty - IMD_IR_PERCENT_OFFSET)) - IMD_IR_SCALE_KOHM)

/* ==================================================================== */
/* ======================= EXTERNAL VARIABLES ========================= */
/* ==================================================================== */

// voltatile variables to be updated from IC timer interrupt
extern volatile uint32_t imdLastUpdate;
extern volatile uint32_t imdFrequency;
extern volatile uint32_t imdDutyCycle;

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DEFINITIONS ==================== */
/* ==================================================================== */

/*!
  @brief    Determine the current status of the IMD from the PWM output
  @param    imdData Data struct to return imd status information
*/
void getImdStatus(imdData_S *imdData)
{
    // Check for loss of signal
    if((HAL_GetTick() - imdLastUpdate) > IMD_PWM_TIMOUT_MS)
	{
        imdData->imdStatus = IMD_NO_SIGNAL;
	}
    else
    {
        // Copy local of volatile variable
        uint32_t localFrequency = imdFrequency;
        uint32_t localDuty = imdDutyCycle;

        // Clamp imd frequency to bounds
        if(localFrequency >= IMD_MAX_FREQ_HZ)
        {
            localFrequency =  IMD_MAX_FREQ_HZ;
        }
        else if(localFrequency <= IMD_MIN_FREQ_HZ)
        {
            localFrequency =  IMD_MIN_FREQ_HZ; 
        }

        // Round imd frequency to nearest 10s place and convert to imd status
        imdData->imdStatus = (IMD_Status_E)((localFrequency + 5) / 10);

        // Check status to see if duty cycle data needs to be decoded
        if((imdData->imdStatus == IMD_NORMAL) || (imdData->imdStatus == IMD_UNDER_VOLT))
        {
            // Under normal and undervolt status, calcuate the isolation resistance from the duty cycle
            if(localDuty <= IMD_HIGH_RESISTANCE_DUTY_THRES)
            {
                imdData->isolationResistance = IMD_HIGH_RESISTANCE_VALUE_KOHM;
            }
            else if(localDuty >= IMD_LOW_RESISTANCE_DUTY_THRES)
            {
                imdData->isolationResistance = IMD_LOW_RESISTANCE_VALUE_KOHM;
            }
            else
            {
                imdData->isolationResistance = GET_ISOLATION_RESISTANCE(localDuty);
            }
        }
        else if(imdData->imdStatus == IMD_SPEED_START_MEASUREMENT)
        {
            // Under speed start status, check duty cycle for speed start success
            imdData->speedStartSuccess = (imdDutyCycle < IMD_SPEED_START_PASS_DUTY_THRES);

            if((imdData->speedStartSuccess) && (imdData->speedStartTime == 0))
            {
                imdData->speedStartTime = imdLastUpdate;
            }
        }
    }
}
