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
#define Configure LAO_Configure
#ifdef __cplusplus
extern "C" {
#endif

/*!
  @file CSD_Configure.h
  
  Revision: 1.5  $Date$
  
  @copy Copyright 2002 STMicroelectronics.
*/
#ifndef _CSD_CONFIGURE_H_

/*!
  Configure -- Enumerate the Configure items.
  @par
  Each Configure_Control can have a negative value, meaning that the value should be
  ignored. Each Configure_Control can have a zero value, meaning that the
  corresponding assertion is at the safest level, or that the corresponding
  optimization is disabled.
*/
typedef enum {
  Configure_Optimize,	//!< Optimize phases.
#	define Configure_OPTIMIZE(this)			(*Configure__OPTIMIZE(this) + 0)
#	define Configure__OPTIMIZE(this)		((this) + Configure_Optimize)
    // 0x1 => instruction bundling,
#	define Configure_Optimize_Bundling		0x1
    // 0x2 => postpass scheduling,
#	define Configure_Optimize_Postpass		0x2
    // 0x4 => register allocation,
#	define Configure_Optimize_RegAlloc		0x4
    // 0x8 => prepass scheduling,
#	define Configure_Optimize_Prepass		0x8
    // 0x10 => loop unrolling and unwinding.
#	define Configure_Optimize_LoopUnroll		0x10
  Configure_Schedule,	//!< Instruction scheduling.
#	define Configure_SCHEDULE(this)			(*Configure__SCHEDULE(this) + 0)
#	define Configure__SCHEDULE(this)		((this) + Configure_Schedule)
    // 0 => no schedule,
    // 1 => basic block schedule,
    // 2 => super block schedule,
    // 3 => trace block schedule.
  Configure_RegAlloc,	//!< Register allocation.
#	define Configure_REGALLOC(this)			(*Configure__REGALLOC(this) + 0)
#	define Configure__REGALLOC(this)		((this) + Configure_RegAlloc)
    // 0 => no register allocation,
    // 1 => local register allocation,
    // 2 => global register allocation,
    // 3 => integrated register allocation.
  Configure_Pipeline,	//!< Software pipelining.
#	define Configure_PIPELINE(this)			(*Configure__PIPELINE(this) + 0)
#	define Configure__PIPELINE(this)		((this) + Configure_Pipeline)
    // 0 => acyclic schedule,
    // 1 => cyclic schedule,
    // 2 => software pipeline,
    // 3 => modulo expansion.
  Configure_LogUnwind,	//!< Log2 of default unwind factor.
#	define Configure_LOGUNWIND(this)			(*Configure__LOGUNWIND(this) + 0)
#	define Configure__LOGUNWIND(this)		((this) + Configure_LogUnwind)
    // 0 => no unwind,
    // 1 => unwind 2,
    // 2 => unwind 4,
    // etc.
  Configure_LogUnroll,	//!< Log2 of default unroll factor.
#	define Configure_LOGUNROLL(this)			(*Configure__LOGUNROLL(this) + 0)
#	define Configure__LOGUNROLL(this)		((this) + Configure_LogUnroll)
    // 0 => no unroll,
    // 1 => unroll 2,
    // 2 => unroll 4,
    // etc.
  Configure_Speculate,	//!< Software speculation level.
#	define Configure_SPECULATE(this)			(*Configure__SPECULATE(this) + 0)
#	define Configure__SPECULATE(this)		((this) + Configure_Speculate)
    // 0 => no software speculation,
    // 1 => software speculate non-excepting instructions.
    // 2 => software speculate excepting dismissable instructions (advanced LOADs).
    // 3 => software speculate excepting non-dismissable instructions (regular LOADs).
  Configure_Renaming,	//!< Register renaming level.
#	define Configure_RENAMING(this)			(*Configure__RENAMING(this) + 0)
#	define Configure__RENAMING(this)		((this) + Configure_Renaming)
    // 0 => no register renaming,
    // 1 => register renaming,
    // n => modulo renaming.
  Configure_Parallel,	//!< Parallel execution enable.
#	define Configure_PARALLEL(this)			(*Configure__PARALLEL(this) + 0)
#	define Configure__PARALLEL(this)		((this) + Configure_Parallel)
    // 0 => scalar execution,
    // 1 => vector execution,
    // 2 => parallel execution,
    // 3 => concurrent execution.
  Configure_Overlap,	//!< Maximum overlap of loop iterations.
#	define Configure_OVERLAP(this)			(*Configure__OVERLAP(this) + 0)
#	define Configure__OVERLAP(this)		((this) + Configure_Overlap)
  Configure_MinTrip,	//!< Minimum value of loop trip count.
#	define Configure_MINTRIP(this)			(*Configure__MINTRIP(this) + 0)
#	define Configure__MINTRIP(this)		((this) + Configure_MinTrip)
  Configure_Modulus,	//!< Modulus of loop trip count.
#	define Configure_MODULUS(this)			(*Configure__MODULUS(this) + 0)
#	define Configure__MODULUS(this)		((this) + Configure_Modulus)
  Configure_Residue,	//!< Residue of loop trip count.
#	define Configure_RESIDUE(this)			(*Configure__RESIDUE(this) + 0)
#	define Configure__RESIDUE(this)		((this) + Configure_Residue)
  Configure_Last
} Configure_Item;

/*!
  Configure -- Array of Configure_Control settings.
*/
typedef int8_t Configure[Configure_Last];

/*!
  Configure_setDefaults -- Set default values for this Configure.
*/
void
Configure_setDefaults(Configure this);

/*!
  Configure_merge -- Merge that Configuration into this Configuration.
  @par
  The values of that Configuration override the values of this Configuration,
  unless for the values of that Configuration that are negative.
*/
void
Configure_merge(Configure this, Configure that);

#define _CSD_CONFIGURE_H_
#endif//_CSD_CONFIGURE_H_

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
#undef Configure
#undef operator
#undef this

// We don't rely on the weak mechanisms for these symbols, even if
// supported by the platform. The reason is to provide a consistent
// implementation on all platforms.

CG_EXPORTED extern bool (*lao_optimize_LOOP_p) (CG_LOOP *cg_loop, uint8_t *lao_configure);
#define lao_optimize_LOOP (*lao_optimize_LOOP_p)

CG_EXPORTED extern bool (*lao_optimize_HB_p) (HB *hb, uint8_t *lao_configure);
#define lao_optimize_HB (*lao_optimize_HB_p)

CG_EXPORTED extern bool (*lao_optimize_FUNC_p) (uint8_t *lao_configure);
#define lao_optimize_FUNC (*lao_optimize_FUNC_p)

CG_EXPORTED extern void (*CGIR_print_p) (void);
#define CGIR_print (*CGIR_print_p)

#endif /* laostub_INCLUDED */
