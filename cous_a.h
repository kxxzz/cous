#pragma once



#include "cous.h"


#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <time.h>
#include <string.h>

#include <vec.h>




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






static int strcicmp(const char* a, const char* b)
{
    for (;; ++a, ++b)
    {
        int n = tolower(*a) - tolower(*b);
        if (n || !*a || !*b) return n;
    }
}




static char* stzncpy(char* dst, char const* src, u32 len)
{
    assert(len > 0);
#ifdef _WIN32
    char* p = _memccpy(dst, src, 0, len - 1);
#else
    char* p = memccpy(dst, src, 0, len - 1);
#endif
    if (p) --p;
    else
    {
        p = dst + len - 1;
        *p = 0;
    }
    return p;
}





static u64 ntoh64(const u64 x)
{
    u64 x1;
    u8* p = (u8*)&x1;
    p[0] = (u8)(x >> 56);
    p[1] = (u8)(x >> 48);
    p[2] = (u8)(x >> 40);
    p[3] = (u8)(x >> 32);
    p[4] = (u8)(x >> 24);
    p[5] = (u8)(x >> 16);
    p[6] = (u8)(x >> 8);
    p[7] = (u8)(x >> 0);
    return x1;
}








void base64encode(char* dst, const char* src, u32 len);
















































































