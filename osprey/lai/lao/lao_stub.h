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

#define this THIS
#define operator OPERATOR
#ifdef __cplusplus
extern "C" {
#endif
/*!
  @file CSD_Configuration.h
  
  Revision: 1.1  $Date$
  
  @copy Copyright 2002 STMicroelectronics.
*/
#ifndef _CSD_CONFIGURATION_H_

/*!
  Configuration_Item -- Enumerate the Configuration items.
  @par
  Each Configuration_Item can have a negative value, meaning that the value should be
  ignored. Each Configuration_Control can have a zero value, meaning that the
  corresponding assertion is at the safest level, or that the corresponding
  optimization is disabled.
*/
typedef enum {
  Configuration_Optimize,	//!< Optimize phases.
    // 0x1 => instruction bundling,
#	define Configuration_Optimize_Bundling		0x1
    // 0x2 => postpass scheduling,
#	define Configuration_Optimize_Postpass		0x2
    // 0x4 => register allocation,
#	define Configuration_Optimize_RegAlloc		0x4
    // 0x8 => prepass scheduling,
#	define Configuration_Optimize_Prepass		0x8
    // 0x10 => loop unrolling and unwinding.
#	define Configuration_Optimize_LoopUnroll		0x10
  Configuration_Schedule,	//!< Instruction scheduling.
    // 0 => no schedule,
    // 1 => basic block schedule,
    // 2 => super block schedule,
    // 3 => trace block schedule.
  Configuration_RegAlloc,	//!< Register allocation.
    // 0 => no register allocation,
    // 1 => local register allocation,
    // 2 => global register allocation,
    // 3 => integrated register allocation.
  Configuration_Pipeline,	//!< Software pipelining.
    // 0 => acyclic schedule,
    // 1 => cyclic schedule,
    // 2 => software pipeline,
    // 3 => modulo expansion.
  Configuration_LogUnwind,	//!< Log2 of default unwind factor.
    // 0 => no unwind,
    // 1 => unwind 2,
    // 2 => unwind 4,
    // etc.
  Configuration_LogUnroll,	//!< Log2 of default unroll factor.
    // 0 => no unroll,
    // 1 => unroll 2,
    // 2 => unroll 4,
    // etc.
  Configuration_Speculate,	//!< Software speculation level.
    // 0 => no software speculation,
    // 1 => software speculate non-excepting instructions.
    // 2 => software speculate excepting dismissable instructions (advanced LOADs).
    // 3 => software speculate excepting non-dismissable instructions (regular LOADs).
  Configuration_Renaming,	//!< Register renaming level.
    // 0 => no register renaming,
    // 1 => register renaming,
    // n => modulo renaming.
  Configuration_Parallel,	//!< Parallel execution enable.
    // 0 => scalar execution,
    // 1 => vector execution,
    // 2 => parallel execution,
    // 3 => concurrent execution.
  Configuration_Overlap,	//!< Maximum overlap of loop iterations.
  Configuration_MinTrip,	//!< Minimum value of loop trip count.
  Configuration_Modulus,	//!< Modulus of loop trip count.
  Configuration_Residue,	//!< Residue of loop trip count.
  Configuration_Last
} Configuration_Item;

/*!
  Configuration -- Array of Configuration_Control items.
*/
struct Configuration_ {
  int8_t ITEMS[Configuration_Last];
};

typedef struct Configuration_ Configuration_, *Configuration;
#define Configuration_ITEMS(this) (0, (this)->ITEMS)
#define Configuration__ITEMS(this) (&(this)->ITEMS)
#define Configuration_MINTRIP(this) ((*Configuration__MINTRIP(this) + 0))
#define Configuration_RENAMING(this) ((*Configuration__RENAMING(this) + 0))
#define Configuration_OVERLAP(this) ((*Configuration__OVERLAP(this) + 0))
#define Configuration_PARALLEL(this) ((*Configuration__PARALLEL(this) + 0))
#define Configuration_SCHEDULE(this) ((*Configuration__SCHEDULE(this) + 0))
#define Configuration_LOGUNWIND(this) ((*Configuration__LOGUNWIND(this) + 0))
#define Configuration_PIPELINE(this) ((*Configuration__PIPELINE(this) + 0))
#define Configuration_LOGUNROLL(this) ((*Configuration__LOGUNROLL(this) + 0))
#define Configuration_OPTIMIZE(this) ((*Configuration__OPTIMIZE(this) + 0))
#define Configuration_REGALLOC(this) ((*Configuration__REGALLOC(this) + 0))
#define Configuration_RESIDUE(this) ((*Configuration__RESIDUE(this) + 0))
#define Configuration_SPECULATE(this) ((*Configuration__SPECULATE(this) + 0))
#define Configuration_MODULUS(this) ((*Configuration__MODULUS(this) + 0))
#define Configuration__MINTRIP(this) ((Configuration_ITEMS(this) + Configuration_MinTrip))
#define Configuration__RENAMING(this) ((Configuration_ITEMS(this) + Configuration_Renaming))
#define Configuration__OVERLAP(this) ((Configuration_ITEMS(this) + Configuration_Overlap))
#define Configuration__PARALLEL(this) ((Configuration_ITEMS(this) + Configuration_Parallel))
#define Configuration__SCHEDULE(this) ((Configuration_ITEMS(this) + Configuration_Schedule))
#define Configuration__LOGUNWIND(this) ((Configuration_ITEMS(this) + Configuration_LogUnwind))
#define Configuration__PIPELINE(this) ((Configuration_ITEMS(this) + Configuration_Pipeline))
#define Configuration__LOGUNROLL(this) ((Configuration_ITEMS(this) + Configuration_LogUnroll))
#define Configuration__OPTIMIZE(this) ((Configuration_ITEMS(this) + Configuration_Optimize))
#define Configuration__REGALLOC(this) ((Configuration_ITEMS(this) + Configuration_RegAlloc))
#define Configuration__RESIDUE(this) ((Configuration_ITEMS(this) + Configuration_Residue))
#define Configuration__SPECULATE(this) ((Configuration_ITEMS(this) + Configuration_Speculate))
#define Configuration__MODULUS(this) ((Configuration_ITEMS(this) + Configuration_Modulus))

Configuration
Configuration_CTOR(Configuration this);


/*!
  Configuration_merge -- Merge that Configuration into this Configuration.
  @par
  The values of that Configuration override the values of this Configuration,
  unless for the values of that Configuration that are negative.
*/
void
Configuration_merge(Configuration this, Configuration that);

#define _CSD_CONFIGURATION_H_
#endif//_CSD_CONFIGURATION_H_
#ifdef __cplusplus
}
#endif
#undef operator
#undef this

// We don't rely on the weak mechanisms for these symbols, even if
// supported by the platform. The reason is to provide a consistent
// implementation on all platforms.

CG_EXPORTED extern bool (*lao_optimize_LOOP_p) (CG_LOOP *cg_loop, Configuration lao_configuration);
#define lao_optimize_LOOP (*lao_optimize_LOOP_p)

CG_EXPORTED extern bool (*lao_optimize_HB_p) (HB *hb, Configuration lao_configuration);
#define lao_optimize_HB (*lao_optimize_HB_p)

CG_EXPORTED extern bool (*lao_optimize_FUNC_p) (Configuration lao_configuration);
#define lao_optimize_FUNC (*lao_optimize_FUNC_p)

CG_EXPORTED extern void (*CGIR_print_p) (void);
#define CGIR_print (*CGIR_print_p)

#endif /* laostub_INCLUDED */
