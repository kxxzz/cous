#pragma once



#include <stdbool.h>


typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef signed char s8;
typedef signed short s16;
typedef signed int s32;
typedef signed long long s64;

typedef float f32;
typedef double f64;



int COUS_connect(const char* host, u32 port, const char* uri);
void COUS_cleanup(void);
bool COUS_update(void);

void COUS_sendText(const char* text, u32 len);
void COUS_sendBinrary(const char* data, u32 len);
void COUS_sendClose(void);



























































