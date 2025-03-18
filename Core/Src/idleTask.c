#include <stdint.h>
#include "main.h"

#define HEARTBEAT_BLINK_MS      300
#define HEARTBEAT_PERIOD_MS     1000

void initIdleTask()
{
    // HAL_GPIO_WritePin(AMS_FAULT_OUT_GPIO_Port, AMS_FAULT_OUT_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(MCU_HEARTBEAT_GPIO_Port, MCU_HEARTBEAT_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MCU_FAULT_GPIO_Port, MCU_FAULT_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MCU_GSENSE_GPIO_Port, MCU_GSENSE_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(AMS_INB_N_GPIO_Port, AMS_INB_N_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(AMS_FAULT_N_GPIO_Port, AMS_FAULT_N_Pin, GPIO_PIN_SET);
}

void runIdleTask()
{
    /* USER CODE BEGIN StartIdle */

    static uint8_t hbState = 0;
    static uint32_t lastHeartBeatUpdate = 0;
    if(hbState)
    {
        if((HAL_GetTick() - lastHeartBeatUpdate) > HEARTBEAT_BLINK_MS)
        {
            HAL_GPIO_WritePin(MCU_HEARTBEAT_GPIO_Port, MCU_HEARTBEAT_Pin, GPIO_PIN_RESET);
            hbState = 0;
            lastHeartBeatUpdate = HAL_GetTick();
        }
    }
    else
    {
        if((HAL_GetTick() - lastHeartBeatUpdate) > (HEARTBEAT_PERIOD_MS - HEARTBEAT_BLINK_MS))
        {
            HAL_GPIO_WritePin(MCU_HEARTBEAT_GPIO_Port, MCU_HEARTBEAT_Pin, GPIO_PIN_SET);
            hbState = 1;
            lastHeartBeatUpdate = HAL_GetTick();
        }
    }
}