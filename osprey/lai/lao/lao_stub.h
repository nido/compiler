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

/*! 
  Optimization_Phase -- MUST BE SAME AS IN LAO.h!
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
  Optimization_Localize = 0x400,
  Optimization_Force_PostPass = 0x1000,
  Optimization_Force_RegAlloc = 0x2000,
  Optimization_Force_PrePass = 0x4000
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
    Optimization_Force_PrePass | \
    0)
#define LAO_Optimization_Mask_RegAlloc (\
    Optimization_Localize | \
    Optimization_RegAlloc | \
    Optimization_Force_RegAlloc | \
    0)
#define LAO_Optimization_Mask_PostPass (\
    Optimization_PostSched | \
    Optimization_Force_PostPass | \
    0)

/*
 * Entry points to the lao code generator.
 *
 * void lao_init(void)
 *	Called once per process before any other for initialization.
 *
 * void lao_fini(void)
 *	Called once per process after any other for finalization.
 *
 * void lao_init_pu(void)
 *	Called once per PU before any call to lao_optimize.
 *
 * void lao_fini_pu(void)
 *	Called once per PU before any call to lao_optimize.
 *
 * void lao_optimize_pu(void)
 *	Optimize an entire PU. May be called several times
 *	on the PU.
 */
extern void lao_init(void);
extern void lao_fini(void);
extern void lao_init_pu(void);
extern void lao_fini_pu(void);
extern void lao_init_region(void);
extern void lao_fini_region(void);
extern bool lao_optimize_pu(unsigned lao_optimizations);

#endif /* laostub_INCLUDED */
