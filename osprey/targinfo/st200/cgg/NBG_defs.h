
#ifndef __NBG_DEFS_H__
#define __NBG_DEFS_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Definition of memory functions.
 */
#ifndef NBG_ALLOC
#include <stdlib.h>
#define NBG_ALLOC(s) malloc(s)
#define NBG_FREE(p) free(p)
#define NBG_ALLOCA(s) alloca(s)
#endif

/*
 * Definition of debug functions.
 */
#ifndef NBG_ASSERT
#include <assert.h>
#define NBG_ASSERT assert
#define NBG_ABORT abort
#endif


/*
 * Definition of int types.
 */
#ifdef HAS_INTTYPES_H
#include <inttypes.h>
#else

#ifndef __uint8_t_defined__
#define __uint8_t_defined__
typedef unsigned char uint8_t;
typedef signed char int8_t;
typedef unsigned short uint16_t;
typedef signed short int16_t;
typedef unsigned int uint32_t;
typedef signed int int32_t;
typedef unsigned long long uint64_t;
typedef signed long long int64_t;
#endif
#endif

#ifndef LL_D
#define LL_D "lld"
#define LL_U "llu"
#define LL_X "llx"
#endif

#ifdef __cplusplus
}
#endif


#endif
