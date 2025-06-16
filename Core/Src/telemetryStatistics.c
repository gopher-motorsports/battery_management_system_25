/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include "telemetryStatistics.h"
#include "adbms/adbms.h"
#include "packData.h"

/* ==================================================================== */
/* ============================= DEFINES ============================== */
/* ==================================================================== */



/* ==================================================================== */
/* =================== LOCAL FUNCTION DECLARATIONS ==================== */
/* ==================================================================== */

static void updateCellMonitorStatistics(Cell_Monitor_S *bmb);

/* ==================================================================== */
/* =================== LOCAL FUNCTION DEFINITIONS ===================== */
/* ==================================================================== */

void updateCellMonitorStatistics(Cell_Monitor_S *bmb)
{
    for(uint32_t i = 0; i < NUM_CELL_MON_IN_ACCUMULATOR; i++)
    {
        Cell_Monitor_S* pBmb = &bmb[i];
        float maxCellVoltage = MIN_CELLV_SENSOR_VALUE;
        float minCellVoltage = MAX_CELLV_SENSOR_VALUE;
        float sumVoltage = 0.0f;
        uint32_t numGoodCellVoltage = 0;

        float maxCellTemp = MIN_TEMP_SENSOR_VALUE_C;
        float minCellTemp = MAX_TEMP_SENSOR_VALUE_C;
        float sumCellTemp = 0.0f;
        uint32_t numGoodCellTemp = 0;

        // Aggregate Cell voltage and temperature data
        for(uint32_t j = 0; j < NUM_CELLS_PER_CELL_MONITOR; j++)
        {
            // Only update stats if sense status is good
            if(pBmb->cellVoltageStatus[j] == GOOD)
            {
                float cellV = pBmb->cellVoltage[j];

                if(cellV > maxCellVoltage)
                {
                    maxCellVoltage = cellV;
                }
                if(cellV < minCellVoltage)
                {
                    minCellVoltage = cellV;
                }
                numGoodCellVoltage++;
                sumVoltage += cellV;
            }

            // Only update stats if sense status is good
            if(pBmb->cellTempStatus[j] == GOOD)
            {
                float cellTemp = pBmb->cellTemp[j];

                if (cellTemp > maxCellTemp)
                {
                    maxCellTemp = cellTemp;
                }
                if (cellTemp < minCellTemp)
                {
                    minCellTemp = cellTemp;
                }
                numGoodCellTemp++;
                sumCellTemp += cellTemp;
            }
        }

        // Update BMB statistics
        // TODO: determine what to do with BAD sensor status variables
        if(numGoodCellVoltage > 0)
        {
            pBmb->maxCellVoltage = maxCellVoltage;
            pBmb->minCellVoltage = minCellVoltage;
            pBmb->sumCellVoltage = sumVoltage;
            pBmb->avgCellVoltage = (sumVoltage / numGoodCellVoltage);
            pBmb->numBadCellVoltage = NUM_CELLS_PER_CELL_MONITOR - numGoodCellVoltage;
        }

        if(numGoodCellVoltage > 0)
        {
            pBmb->maxCellTemp = maxCellTemp;
            pBmb->minCellTemp = minCellTemp;
            pBmb->avgCellTemp = (sumCellTemp / numGoodCellTemp);
            pBmb->numBadCellTemp = NUM_CELLS_PER_CELL_MONITOR - numGoodCellTemp;
        }
    }
}

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DEFINITIONS ==================== */
/* ==================================================================== */

void updateBatteryStatistics(telemetryTaskData_S *taskData)
{
    // Update BMB level stats
	updateCellMonitorStatistics(taskData->bmb);

	float maxCellVoltage = MIN_CELLV_SENSOR_VALUE;
    float minCellVoltage = MAX_CELLV_SENSOR_VALUE;
    float sumVoltage = 0.0f;
    float sumAvgCellVoltage = 0.0f;
    uint32_t numGoodBmbsCellV = 0;

    float maxCellTemp = MIN_TEMP_SENSOR_VALUE_C;
    float minCellTemp = MAX_TEMP_SENSOR_VALUE_C;
    float sumAvgCellTemp = 0.0f;
    uint32_t numGoodBmbsCellTemp = 0;

	float maxBoardTemp = MIN_TEMP_SENSOR_VALUE_C;
	float minBoardTemp = MAX_TEMP_SENSOR_VALUE_C;
	float sumBoardTemp = 0.0f;
    uint32_t numGoodBoardTemp = 0;

	for(int32_t i = 0; i < NUM_CELL_MON_IN_ACCUMULATOR; i++)
	{
		Cell_Monitor_S* pBmb = &taskData->bmb[i];

        if(pBmb->numBadCellVoltage != NUM_CELLS_PER_CELL_MONITOR)
        {
            if(pBmb->maxCellVoltage > maxCellVoltage)
            {
                maxCellVoltage = pBmb->maxCellVoltage;
            }
            if(pBmb->minCellVoltage < minCellVoltage)
            {
                minCellVoltage = pBmb->minCellVoltage;
            }

            numGoodBmbsCellV++;
            sumAvgCellVoltage += pBmb->avgCellVoltage;
            sumVoltage += pBmb->sumCellVoltage;
        }

        if(pBmb->numBadCellTemp != NUM_CELLS_PER_CELL_MONITOR)
        {
            if (pBmb->maxCellTemp > maxCellTemp)
            {
                maxCellTemp = pBmb->maxCellTemp;
            }
            if (pBmb->minCellTemp < minCellTemp)
            {
                minCellTemp = pBmb->minCellTemp;
            }

            numGoodBmbsCellTemp++;
            sumAvgCellTemp += pBmb->avgCellTemp;
        }

        if(pBmb->boardTempStatus == GOOD)
        {
            if (pBmb->boardTemp > maxBoardTemp)
            {
                maxBoardTemp = pBmb->boardTemp;
            }
            if (pBmb->boardTemp < minBoardTemp)
            {
                minBoardTemp = pBmb->boardTemp;
            }

            numGoodBoardTemp++;
            sumBoardTemp += pBmb->boardTemp;
        }
	}

    // TODO: Should i ignore this if bad sensors or open wires?
    taskData->cellSumVoltage = sumVoltage;

    if(numGoodBmbsCellV > 0)
    {
        taskData->maxCellVoltage = maxCellVoltage;
        taskData->minCellVoltage = minCellVoltage;
        taskData->avgCellVoltage = sumAvgCellVoltage / numGoodBmbsCellV;
        taskData->cellImbalance = maxCellVoltage - minCellVoltage;
    }

    if(numGoodBmbsCellTemp > 0)
    {
        taskData->maxCellTemp = maxCellTemp;
        taskData->minCellTemp = minCellTemp;
        taskData->avgCellTemp = sumAvgCellTemp / numGoodBmbsCellTemp;
    }

    if(numGoodBoardTemp > 0)
    {
        taskData->maxBoardTemp = maxBoardTemp;
        taskData->minBoardTemp = minBoardTemp;
        taskData->avgBoardTemp = sumBoardTemp / numGoodBoardTemp;
    }
}
