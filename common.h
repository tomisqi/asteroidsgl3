#pragma once

#include <stdint.h>

#define MB 1024*1024

#define OFFSET_OF(_TYPE, _MEMBER)	((size_t)&(((_TYPE*)0)->_MEMBER))
#define ARRAY_COUNT(A)				(sizeof(A) / sizeof(A[0]))

typedef int8_t S8;
typedef uint8_t U8;
typedef int16_t S16;
typedef uint16_t U16;
typedef uint32_t U32;
typedef uint64_t U64;

struct Buffer
{
	U8* buf_p;
	U64 bufSize;
};

