
#ifndef H_ALLOCA_H
#define H_ALLOCA_H

#ifndef CROSS_COMPILE
#include <alloca.h>
#else
/* Under mingwin, use malloc.h header file for alloca. */
#include <malloc.h>
#endif

#endif /* H_ALLOCA_H */
