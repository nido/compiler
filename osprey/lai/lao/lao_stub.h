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

/*!
  COPIED FROM LAO2/LAO/LAO_Driver.xcc
  LAO_Action -- Enumerates the LAO actions.
  */
typedef enum {
  LAO_NoAction = 0,
  LAO_BlockSchedule = 1,        // super block scheduling
  LAO_TraceSchedule = 2,        // trace block scheduling
  LAO_LoopSchedule = 4,         // scheduling with loop-carried dependences
  LAO_LoopPipeline = 8,         // loop pipelining with modulo scheduling
  LAO_LoopUnwind = 16,          // modulo expansion if LoopPipeline
  LAO_LoopUnroll = 32,          // counted loop unrolling
  LAO_Recurrences = 64,         // recurrence rewriting
  LAO_Reductions = 128,         // reduction splitting
  LAO_PostPass = 256,           // postpass scheduling
  LAO_RegAlloc = 512,           // register allocate
} LAO_Action;
const char *
LAO_Action_NAMES(unsigned actions);

#ifdef __cplusplus
extern "C" {
#endif

#if 0

// Optimize a LOOP_DESCR through the LAO.
bool
lao_optimize_LOOP(CG_LOOP *cg_loop, unsigned lao_actions);

// Optimize a HB through the LAO.
bool
lao_optimize_HB(HB *hb, unsigned lao_actions);

// Optimize a Function through the LAO.
bool
lao_optimize_FUNC(unsigned lao_actions);

#endif

/*
  lao_init -- LAO initialization function.
*/
void
lao_init(void);

/*
  lao_fini -- LAO finalization function.
*/
void
lao_fini(void);

#ifdef __cplusplus
}
#endif

// We don't rely on the weak mechanisms for these symbols, even if
// supported by the platform. The reason is to provide a consistent
// implementation on all platforms.

BE_EXPORTED extern bool (*lao_optimize_LOOP_p) (CG_LOOP *cg_loop, unsigned lao_actions);
#define lao_optimize_LOOP (*lao_optimize_LOOP_p)

BE_EXPORTED extern bool (*lao_optimize_HB_p) (HB *hb, unsigned lao_actions);
#define lao_optimize_HB (*lao_optimize_HB_p)

BE_EXPORTED extern bool (*lao_optimize_FUNC_p) (unsigned lao_actions);
#define lao_optimize_FUNC (*lao_optimize_FUNC_p)

#endif /* laostub_INCLUDED */
