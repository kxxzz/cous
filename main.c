#include <stdlib.h>
#ifdef _WIN32
# include <crtdbg.h>
#endif



#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <inttypes.h>

#include "cous.h"





#ifdef ARYLEN
# undef ARYLEN
#endif
#define ARYLEN(a) (sizeof(a) / sizeof((a)[0]))




static void test(void)
{

}





static int mainReturn(int r)
{
#if !defined(NDEBUG) && defined(_WIN32)
    system("pause");
#endif
    return r;
}


int main(int argc, char* argv[])
{
    test();
    mainReturn(0);
}



















































































