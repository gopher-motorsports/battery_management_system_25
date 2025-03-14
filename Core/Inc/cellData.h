#ifndef INC_CELLDATA_H_
#define INC_CELLDATA_H_

#include "lookupTable.h"
#include "utils.h"

#define CELL_CAPACITY_MAH           13000.0f
#define MAX_C_RATING                1

#define CELL_POLARIZATION_REST_SEC  20
#define CELL_POLARIZATION_REST_MS   CELL_POLARIZATION_REST_SEC * MILLISECONDS_IN_SECOND

#define MAX_BRICK_VOLTAGE           4.28f
#define MAX_BRICK_WARNING_VOLTAGE   4.3f
#define MAX_BRICK_FAULT_VOLTAGE     4.33f

#define NOMINAL_BRICK_VOLTAGE       3.78f
#define MIN_BRICK_WARNING_VOLTAGE   3.1f
#define MIN_BRICK_FAULT_VOLTAGE     3.0f

#define MAX_BRICK_TEMP_WARNING_C    55.0f
#define MAX_BRICK_TEMP_FAULT_C      60.0f

extern LookupTable_S socByOcvTable;
extern LookupTable_S soeFromSocTable;

#endif /* INC_CELLDATA_H_ */
