#include "cous_a.h"

#ifdef _WIN32
# include <winsock2.h>
# include <ws2tcpip.h>
#endif




void base64encode(char* dst, const char* src, u32 len)
{
    const char base64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789*/=";
    int rest;
    int parts;
    const char* in_iterator;
    char* out_iterator;

    parts = len / 3;
    rest = len % 3;

    in_iterator = src;
    out_iterator = dst;
    for (; parts > 0; --parts)
    {
        out_iterator[0] = base64chars[in_iterator[0] >> 2 & 0x3f];
        out_iterator[1] = base64chars[(in_iterator[0] << 4 & 0x30) + (in_iterator[1] >> 4 & 0xf)];
        out_iterator[2] = base64chars[(in_iterator[1] << 2 & 0x3c) + (in_iterator[2] >> 6 & 0x3)];
        out_iterator[3] = base64chars[in_iterator[2] & 0x3f];
        in_iterator += 3;
        out_iterator += 4;
    }
    if (rest > 0)
    {
        int a, b, c;
        a = b = c = 64;

        if (rest >= 1)
        {
            a = in_iterator[0] >> 2 & 0x3f;
            b = in_iterator[0] << 4 & 0x30;
            if (rest >= 2)
            {
                b |= in_iterator[1] >> 4 & 0xf;
                c = in_iterator[1] << 2 & 0x3c;
                if (rest >= 3)
                {
                    c |= in_iterator[2] >> 6 & 0x3;
                }
            }
        }
        out_iterator[0] = base64chars[a];
        out_iterator[1] = base64chars[b];
        out_iterator[2] = base64chars[c];
        out_iterator[3] = '=';
        out_iterator += 4;
    }
    out_iterator[0] = 0;
}














void COUS_onData(COUS_Context* ctx, char* data, u32 size)
{
    // https://tools.ietf.org/html/rfc6455#section-5.2

    char* wsData = NULL;
    if (WS_FrameOp_Continuation == ctx->opState)
    {
        u8 finalFragment = data[0] >> 7 & 0x1;
        WS_FrameOp opcode = data[0] & 0xf;
        u8 masked = data[1] >> 7 & 0x1;
        u32 payloadLength = data[1] & 0x7f;

        if (payloadLength < 126)
        {
            wsData = data + 2;
        }
        else if (payloadLength == 126)
        {
            payloadLength = ntohs(*(u16*)(data + 2));
            wsData = data + 2 + 2;
        }
        else if (payloadLength == 127)
        {
            payloadLength = (u32)ntohll(*(u64*)(data + 2));
            wsData = data + 2 + 8;
        }

        if (masked)
        {
            u8 maskingKey[4];
            memcpy(maskingKey, wsData, 4);
            wsData += 4;
            for (u32 i = 0; i < payloadLength; ++i)
            {
                wsData[i] ^= maskingKey[i % 4];
            }
        }
        ctx->opState = opcode;
        ctx->remain = payloadLength;
        vec_resize(ctx->recvBuf, 0);
    }
    else
    {
        wsData = data;
    }

    u32 msgLen = (u32)(data + size - wsData);
    if (msgLen && ctx->remain)
    {
        msgLen = min(msgLen, ctx->remain);
        ctx->remain -= msgLen;

        vec_pusharr(ctx->recvBuf, wsData, msgLen);
    }
    if (msgLen && !ctx->remain)
    {
        switch (ctx->opState)
        {
        case WS_FrameOp_Text:
        {
            vec_push(ctx->recvBuf, 0);
            printf("[COUS] text:\n%s", ctx->recvBuf->data);

            COUS_sendText(ctx->recvBuf->data, ctx->recvBuf->length);

            //COUS_sendClose();
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



































































































