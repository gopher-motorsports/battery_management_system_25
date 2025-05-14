#ifndef INC_PACKDATA_H_
#define INC_PACKDATA_H_

#include "cellData.h"
#include "lookupTable.h"
#include "utils.h"

// The number of Cells in a cell brick
#define NUM_PARALLEL_CELLS          1

// #define NUM_BATTERY_SEGMENTS        1

#define PACK_MILLICOULOMBS          CELL_CAPACITY_MAH * NUM_PARALLEL_CELLS * MINUTES_IN_HOUR * SECONDS_IN_MINUTE
#define PACK_MILLIJOULES            PACK_MILLICOULOMBS * NOMINAL_BRICK_VOLTAGE

#define MAX_TEMP_SENSOR_VALUE_C     120.0f
#define MIN_TEMP_SENSOR_VALUE_C     -40.0f

#define SHUNT_RESISTOR_REFERENCE_UV 50.0f

extern LookupTable_S cellMonTempTable;
extern LookupTable_S packMonTempTable;

#endif /* INC_PACKDATA_H_ */
