#pragma once

#include <stdint.h>

#define MB 1024*1024

#define U16_MAX						UINT16_MAX
#define U32_MAX						UINT32_MAX
#define S64_MAX						INT64_MAX
#define S16_MAX						INT16_MAX
#define S32_MAX 					INT32_MAX
#define OFFSET_OF(_TYPE, _MEMBER)	((size_t)&(((_TYPE*)0)->_MEMBER))
#define ARRAY_COUNT(A)				(sizeof(A) / sizeof(A[0]))

typedef int8_t S8;
typedef uint8_t U8;
typedef int16_t S16;
typedef uint16_t U16;
typedef uint32_t U32;
typedef uint64_t U64;
typedef int64_t S64;

struct Buffer
{
	U8* buf_p;
	U64 bufSize;
};

