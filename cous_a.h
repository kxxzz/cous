#pragma once



#include "cous.h"


#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <time.h>



#ifdef ARYLEN
# undef ARYLEN
#endif
#define ARYLEN(a) (sizeof(a) / sizeof((a)[0]))




#ifdef max
# undef max
#endif
#ifdef min
# undef min
#endif
#define max(a,b) ((a) > (b) ? (a) : (b))
#define min(a,b) ((a) < (b) ? (a) : (b))




#define zalloc(sz) calloc(1, sz)























































































