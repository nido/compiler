/**
***			Interface to/from the LAO
***			-------------------------
***
*** Description:
***
***	This is the interface between the Open64/CGIR and the LAO/LIR
***	representations.
***
**/

#ifndef laostub_INCLUDED
#define laostub_INCLUDED

#include "W_math.h"
#include <stdarg.h>
#include <set.h>

#include "defs.h"
#include "config.h"
#include "errors.h"
#include "cg_loop.h"
#include "hb.h"

// We don't rely on the weak mechanisms for these symbols, even if
// supported by the platform. The reason is to provide a consistent
// implementation on all platforms.

CG_EXPORTED extern bool (*lao_optimize_LOOP_p)(LOOP_DESCR *loop, unsigned lao_optimizations);
#define lao_optimize_LOOP (*lao_optimize_LOOP_p)

CG_EXPORTED extern bool (*lao_optimize_HB_p)(HB *hb, unsigned lao_optimizations);
#define lao_optimize_HB (*lao_optimize_HB_p)

CG_EXPORTED extern bool (*lao_optimize_BB_p)(BB *bb, unsigned lao_optimizations);
#define lao_optimize_BB (*lao_optimize_BB_p)

CG_EXPORTED extern void (*CGIR_print_p) (void);
#define CGIR_print (*CGIR_print_p)

#endif /* laostub_INCLUDED */
