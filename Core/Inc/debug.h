#ifndef INC_DEBUG_H_
#define INC_DEBUG_H_

#include <stdio.h>

#define DEBUG 0
#if DEBUG
	#define Debug(__info,...) printf("Debug: " __info,##__VA_ARGS__)
#else
	#define Debug(__info,...)
#endif

#endif
