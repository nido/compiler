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
  Optimization_Phase -- MUST BE SAME AS IN LAO_Driver.h!
*/  
typedef enum {
  Optimization_Linearize = 0x1,
  Optimization_PostSched = 0x2,
  Optimization_RegAlloc = 0x4,
  Optimization_PreSched = 0x8,
  Optimization_LoopUnwind = 0x10,
  Optimization_LoopUnroll = 0x20,
  Optimization_EnableSSA = 0x40,
  Optimization_StartBlock = 0x100,
  Optimization_StopBlock = 0x200,
  Optimization_Localize = 0x400
} Optimization_Phase;

/*
 * Mask of optimizations performed at each LAO pass.
 * Currently three LAO passes are available:
 * - PrePass : pre reg-alloc optimizations and scheduling.
 *	Done in place of IGLS_Schedule prepass.
 * - RegAlloc : global register allocation
 *	Done in place of GRA/LRA reg allocation.
 * - PostPass : post reg-alloc optimizations and scheduling
 *	Done in place of IGLS_Schedule postpass.
 *
 * Each of the three masks correspond to available optimization
 * for the corresponding pass.
 */
#define LAO_Optimization_Mask_PrePass (\
    Optimization_PreSched | \
    0)
#define LAO_Optimization_Mask_RegAlloc (\
    Optimization_Localize | \
    Optimization_RegAlloc | \
    0)
#define LAO_Optimization_Mask_PostPass (\
    Optimization_PostSched | \
    0)

// We don't rely on the weak mechanisms for these symbols, even if
// supported by the platform. The reason is to provide a consistent
// implementation on all platforms.

CG_EXPORTED extern bool (*lao_optimize_PU_p)(unsigned lao_optimizations);
#define lao_optimize_PU (*lao_optimize_PU_p)

CG_EXPORTED extern void (*CGIR_print_p) (FILE *file);
#define CGIR_print (*CGIR_print_p)

#endif /* laostub_INCLUDED */
