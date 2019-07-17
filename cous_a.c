#include "cous_a.h"







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





























































































