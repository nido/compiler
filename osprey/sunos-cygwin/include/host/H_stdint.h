
#ifndef H_STDINT_H
#define H_STDINT_H

#include<sys/types.h>

// we try to always define basic types using uintxx_t syntax...
typedef u_int8_t  uint8_t;
typedef u_int16_t uint16_t;
typedef u_int32_t uint32_t;
typedef u_int64_t uint64_t;

typedef signed long     intptr_t;
typedef unsigned long   uintptr_t;

#endif /* H_STDINT_H */
