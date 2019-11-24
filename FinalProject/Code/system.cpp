#include "system.h"

#ifdef i386

float htonf(float num)
{
    (*(long*)(&num)) = htonl(*(long*)(&num));
    return num;
}

float ntohf(float num)
{
    return htonf(num);
}

double htond(double num)
{
    double tem;
    for (int j = 0; j < sizeof(double); j++)
        ((char*)&tem)[j] = ((char*)&num)[sizeof(double) - j - 1];
    return tem;
}

double ntohd(double num)
{
    return htond(num);
}

#endif

#ifndef i386

float htonf(float num)
{
    return num;
}

float ntohf(float num)
{
    return num;
}

double htond(double num)
{
    return num;
}

double ntohd(double num)
{
    return num;
}

#endif

#ifdef __GNUC__
#include <string.h>
#endif

#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifdef __GNUC__
#include <string.h>
#endif
#ifdef sgi
#include <bstring.h>
#else
#if !defined(__osf__) && !defined(linux)
void bcopy(const void* src, void* dst, int length);
#endif
#endif

long GetIdleTime()
{
    return 0;
}

long FileSize(char* fileName)
{
    struct stat statbuf;
    if (stat(fileName, &statbuf) == -1) return -1; else
        return statbuf.st_size;
}

double RandomDouble()
// Return a random number in the range 0 to 1, inclusive
{
    return ((double)(rand()) / ((double)(0x7FFFFFFF)));
}

void Sbrk(char* message)
{
}
