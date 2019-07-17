#include "cous_a.h"

#include <dyad.h>



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





typedef struct COUS_Context
{
    bool connected;
    char host[HOST_NAME_MAX];
    u32 port;
    char uri[MAX_REQUEST_PATH_LENGTH];
    dyad_Stream* s;
    bool handshaked;
    WS_FrameOp opState;
    u32 remain;
    vec_char sendBuf[1];
    vec_char recvBuf[1];
} COUS_Context;

static COUS_Context ctx[1] = { 0 };

static void COUS_contextFree(void)
{
    vec_free(ctx->recvBuf);
    vec_free(ctx->sendBuf);
    memset(ctx, 0, sizeof(*ctx));
}





static void COUS_onError(dyad_Event *e)
{
    printf("[COUS] error: %s\n", e->msg);
}

static void COUS_onTimeout(dyad_Event *e)
{
    printf("[COUS] timeout: %s\n", e->msg);
}

static void COUS_onClose(dyad_Event *e)
{
    printf("[COUS] closed: %s\n", e->msg);
    ctx->connected = false;
}




static void COUS_onConnect(dyad_Event *e)
{
    printf("[COUS] connected: %s\n", e->msg);

    const char* requestFmt =
        "GET %s HTTP/1.1\r\n"
        "Host: %s:%u\r\n"
        "Connection: Upgrade\r\n"
        "Upgrade: websocket\r\n"
        "Sec-WebSocket-Version: 13\r\n"
        "Sec-WebSocket-Key: %s\r\n"
        "\r\n";

    u32 key[WS_KEY_SIZE / 4];
    for (u32 i = 0; i < WS_KEY_SIZE / 4; ++i)
    {
        key[i] = rand();
    }
    char keyStr[((WS_KEY_SIZE + 2) / 3 * 4) + 1];
    base64encode(keyStr, (char*)key, WS_KEY_SIZE);

    s32 n = snprintf(NULL, 0, requestFmt, ctx->uri, ctx->host, ctx->port, keyStr);
    vec_resize(ctx->sendBuf, n + 1);
    n = snprintf(ctx->sendBuf->data, ctx->sendBuf->length, requestFmt, ctx->uri, ctx->host, ctx->port, keyStr);
    if (n > 0)
    {
        dyad_write(ctx->s, ctx->sendBuf->data, n);
    }
    else
    {
        // report
    }
}


static void COUS_onData(dyad_Event *e)
{
    if (!ctx->handshaked)
    {
        ctx->handshaked = true;

        printf("[COUS] handshaked\n");
        printf("\n%s\n", e->data);
    }
    else
    {
        // https://tools.ietf.org/html/rfc6455#section-5.2

        char* msgData = NULL;
        if (WS_FrameOp_Continuation == ctx->opState)
        {
            u8 finalFragment = e->data[0] >> 7 & 0x1;
            WS_FrameOp opcode = e->data[0] & 0xf;
            u8 masked = e->data[1] >> 7 & 0x1;
            u32 payloadLength = e->data[1] & 0x7f;

            if (payloadLength < 126)
            {
                msgData = e->data + 2;
            }
            else if (payloadLength == 126)
            {
                payloadLength = ntohs(*(u16*)&e->data[2]);
                msgData = e->data + 2 + 2;
            }
            else if (payloadLength == 127)
            {
                payloadLength = (u32)ntoh64(*(u64*)&e->data[2]);
                msgData = e->data + 2 + 8;
            }

            if (masked)
            {
                u8 maskingKey[4];
                memcpy(maskingKey, msgData, 4);
                msgData += 4;
                // unmask data
                for (u32 i = 0; i < payloadLength; ++i)
                {
                    msgData[i] ^= maskingKey[i % 4];
                }
            }
            ctx->opState = opcode;
            ctx->remain = payloadLength;
            vec_resize(ctx->recvBuf, 0);
        }
        else
        {
            msgData = e->data;
        }

        u32 msgLen = (u32)(e->data + e->size - msgData);
        if (msgLen && ctx->remain)
        {
            msgLen = min(msgLen, ctx->remain);
            ctx->remain -= msgLen;

            vec_pusharr(ctx->recvBuf, msgData, msgLen);
        }
        if (msgLen && !ctx->remain)
        {
            switch (ctx->opState)
            {
            case WS_FrameOp_Text:
            {
                vec_push(ctx->recvBuf, 0);
                printf("[COUS] text:\n%s\n", ctx->recvBuf->data);
                break;
            }
            case WS_FrameOp_Binary:
            {
                printf("[COUS] binrary size:%u\n", ctx->recvBuf->length);
                break;
            }
            case WS_FrameOp_Close:
            {
                printf("[COUS] closed\n");
                ctx->connected = false;
                break;
            }
            case WS_FrameOp_Ping:
            case WS_FrameOp_Pong:
            {
                // todo
                break;
            }
            default:
                printf("[COUS] unhandled opcode=%u\n", ctx->opState);
                printf("\n%s\n", ctx->recvBuf->data);
                break;
            }
            ctx->opState = WS_FrameOp_Continuation;
        }
    }
}







int COUS_connect(const char* host, u32 port, const char* uri)
{
    stzncpy(ctx->host, host, HOST_NAME_MAX);
    ctx->port = port;
    stzncpy(ctx->uri, uri, MAX_REQUEST_PATH_LENGTH);

    dyad_init();
    dyad_Stream* s = dyad_newStream();
    ctx->s = s;
    dyad_addListener(s, DYAD_EVENT_ERROR, COUS_onError, NULL);
    dyad_addListener(s, DYAD_EVENT_TIMEOUT, COUS_onTimeout, NULL);
    dyad_addListener(s, DYAD_EVENT_CLOSE, COUS_onClose, NULL);
    dyad_addListener(s, DYAD_EVENT_CONNECT, COUS_onConnect, NULL);
    dyad_addListener(s, DYAD_EVENT_DATA, COUS_onData, NULL);
    int r = dyad_connect(s, host, port);
    if (r != 0)
    {
        dyad_shutdown();
        COUS_contextFree();
    }
    ctx->connected = true;
    return r;
}



void COUS_cleanup(void)
{
    dyad_shutdown();
    COUS_contextFree();
}



bool COUS_update(void)
{
    if (ctx->connected)
    {
        dyad_update();
        return true;
    }
    else
    {
        COUS_cleanup();
        return false;
    }
}










































































