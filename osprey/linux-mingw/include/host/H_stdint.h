
#ifndef H_STDINT_H
#define H_STDINT_H

#ifdef CROSS_COMPILE
/* linux-mingw build. */
#ifndef _MSC_VER
#include <stdint.h>

/* Some code use u_intxx_t instead os uintxx_t. */
typedef uint32_t u_int32_t;
typedef uint64_t u_int64_t;
typedef uint16_t u_int16_t;
typedef uint8_t u_int8_t;

/* Some code use __uintxx_t instead os uintxx_t. */
typedef uint32_t __uint32_t;
typedef uint16_t __uint16_t;
typedef uint8_t __uint8_t;
typedef int32_t __int32_t;
typedef int16_t __int16_t;
typedef int8_t __int8_t;
#endif

#else /* if !CROSS_COMPILE */
/* linux-linux build. */
#include <stdint.h>

#endif
#endif /* H_STDINT_H */
