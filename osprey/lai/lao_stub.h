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

#include <math.h>
#include <stdarg.h>
#include <set.h>

#include "defs.h"
#include "config.h"
#include "errors.h"
#include "cg_loop.h"
#include "hb.h"

// Replicates enum CodeRegion_Action in LIR_CodeRegion.h
enum LAO_Action {
  LAO_NoAction = 0,
  LAO_BlockSchedule = 1,	// basic / super block scheduling
  LAO_TraceSchedule = 2,	// scheduling of a block trace
  LAO_LoopSchedule = 4,		// scheduling with loop-carried dependences
  LAO_LoopPipeline = 8,		// loop pipelining with modulo scheduling
  LAO_LoopUnwind = 16,		// modulo expansion if LoopPipeline
  LAO_LoopUnroll = 32,		// counted loop unrolling
  LAO_Recurrences = 64,		// recurrence rewriting
  LAO_Reductions = 128,		// reduction splitting
};

// Optimize a LOOP_DESCR through the LAO.
bool
LAO_optimize(LOOP_DESCR *loop, unsigned lao_actions);

// Optimize a HB through the LAO.
bool
LAO_optimize(HB *hb, unsigned lao_actions);

// Optimize a Function through the LAO.
bool
LAO_optimize(unsigned lao_actions);


// Must be IDENTICAL to enum LOOP_OPT_ACTION in lai/cg_loop.cxx
enum LAO_SWP_ACTION {
  NO_LOOP_OPT,
  SINGLE_BB_DOLOOP_SWP,
  SINGLE_BB_DOLOOP_UNROLL,
  SINGLE_BB_WHILELOOP_SWP,
  SINGLE_BB_WHILELOOP_UNROLL,
  MULTI_BB_DOLOOP
};

bool
LAO_scheduleRegion ( vector<BB>& entryBBs, vector<BB>& regionBBs, vector<BB>& exitBBs, LAO_SWP_ACTION action );

bool
Perform_SWP ( CG_LOOP& cl, LAO_SWP_ACTION action );

#endif /* laostub_INCLUDED */
