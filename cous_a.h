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






void base64encode(char* dst, const char* src, u32 len);











enum
{
    WS_KEY_SIZE = 16,
    HOST_NAME_MAX = 64,
    MAX_REQUEST_PATH_LENGTH = 2048,
};



typedef enum WS_FrameOp
{
    WS_FrameOp_Continuation = 0x0,
    WS_FrameOp_Text = 0x1,
    WS_FrameOp_Binary = 0x2,
    WS_FrameOp_DataUnused = 0x3,
    WS_FrameOp_Close = 0x8,
    WS_FrameOp_Ping = 0x9,
    WS_FrameOp_Pong = 0xA,
    WS_FrameOp_ControlUnused = 0xB,
} WS_FrameOp;










#define COUS_USE_DYAD
//#define COUS_USE_CURL









typedef struct COUS_Context
{
    bool connected;
    char host[HOST_NAME_MAX];
    u32 port;
    char uri[MAX_REQUEST_PATH_LENGTH];
    void* handle;
    bool handshaked;
    WS_FrameOp opState;
    u32 remain;
    vec_char sendBuf[1];
    vec_char recvBuf[1];
} COUS_Context;

static void COUS_contextFree(COUS_Context* ctx)
{
    vec_free(ctx->recvBuf);
    vec_free(ctx->sendBuf);
    memset(ctx, 0, sizeof(*ctx));
}



#ifdef COUS_USE_CURL
void COUS_onData(COUS_Context* ctx, char* data, u32 size);
#endif

















































































