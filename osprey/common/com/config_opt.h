/*

  Copyright (C) 2000, 2001 Silicon Graphics, Inc.  All Rights Reserved.

  This program is free software; you can redistribute it and/or modify it
  under the terms of version 2 of the GNU General Public License as
  published by the Free Software Foundation.

  This program is distributed in the hope that it would be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

  Further, this software is distributed without any warranty that it is
  free of the rightful claim of any third person regarding infringement 
  or the like.  Any license provided herein, whether implied or 
  otherwise, applies only to this software file.  Patent licenses, if 
  any, provided herein do not apply to combinations of this program with 
  other software, or any other product whatsoever.  

  You should have received a copy of the GNU General Public License along
  with this program; if not, write the Free Software Foundation, Inc., 59
  Temple Place - Suite 330, Boston MA 02111-1307, USA.

  Contact information:  Silicon Graphics, Inc., 1600 Amphitheatre Pky,
  Mountain View, CA 94043, or:

  http://www.sgi.com

  For further information regarding this notice, see:

  http://oss.sgi.com/projects/GenInfo/NoticeExplan

*/


/* ====================================================================
 * ====================================================================
 *
 * Module: config_opt.h
 *
 * Revision history:
 *  05-May-96 - Extracted from be/opt/opt_config.h.
 *
 * Description:
 *
 * Declare global flag variables for -OPT group options.
 * This file is included in common/com/config.c.
 *
 * Declarations of -OPT flags should be put here, instead of in
 * config.h.  The intent is to allow updates of the -OPT group
 * without forcing recompilation of everything that includes config.h.
 * (However, the transfer of the flags' definitions here from config.h
 * is not yet complete, so most of the old ones still require
 * config.h.)
 *
 * ====================================================================
 * WARNING: WHENEVER A NEW FLAG IS ADDED:
 * ###	- Add the flag variable declaration to config_opt.h (here) .
 * ###	- Add the flag variable definition to config_opt.cxx .
 * ###	- Add the option to the group description in config_opt.cxx .
 * ====================================================================
 *
 * ====================================================================
 * ====================================================================
 */

#ifndef config_opt_INCLUDED
#define config_opt_INCLUDED

#ifndef flags_INCLUDED
#include "flags.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Incomplete types to prevent unnecessary inclusion: */
struct skiplist;

/*********************************************************************
 ***
 *** Flag variable declarations:
 ***
 *********************************************************************
 */

/***** Optimization Warning Messages *****/
BE_EXPORTED extern BOOL Show_OPT_Warnings;		/* Display OPT warning messages */

/***** Aliasing control *****/
BE_EXPORTED extern BOOL Alias_Pointer_Parms;	/* Reference parms indep? */
BE_EXPORTED extern BOOL Alias_Pointer_Types;	/* Ptrs to distinct basic types indep? */
BE_EXPORTED extern BOOL Alias_Not_In_Union;	/* Ptrs point to non-union types */
BE_EXPORTED extern BOOL Alias_Pointer_Strongly_Typed; /* Ptrs to distinct types indep? */
BE_EXPORTED extern BOOL Alias_Pointer_Named_Data;	/* No pointers to named data? */
BE_EXPORTED extern BOOL Alias_Pointer_Restricted;	/* *p and *q not aliased */
BE_EXPORTED extern BOOL Alias_Pointer_Disjoint;     /* **p and **q not aliased */
BE_EXPORTED extern BOOL Alias_Pointer_Cray;         /* Cray pointer semantics? */
BE_EXPORTED extern BOOL Alias_Common_Scalar;        /* Distinguish scalar from other array
                                           in a common block */
BE_EXPORTED extern BOOL  Alias_F90_Pointer_Unaliased;  /* Are F90 pointers unaliased? */

/***** Expression folding options *****/
BE_EXPORTED extern BOOL Enable_Cfold_Float;		/* FP constant folding? */
BE_EXPORTED extern BOOL Enable_Cfold_Reassociate;	/* Re-association allowed? */
BE_EXPORTED extern BOOL Enable_Cfold_Intrinsics;	/* Intrinsic constant folding? */
BE_EXPORTED extern BOOL Cfold_Intrinsics_Set;	/* ... option seen? */
BE_EXPORTED extern BOOL CIS_Allowed;	/* sin(x) and cos(x) => cis(x) ? */
BE_EXPORTED extern BOOL Div_Split_Allowed;	/* Change a/b --> a*1/b ? */
#ifdef TARG_ST
BE_EXPORTED extern BOOL Float_Eq_Simp;	/* change a==b (float cmp) --> a==b (integer cmp if a or b is cst)) ? */
BE_EXPORTED extern BOOL OPT_fb_div_simp;	// Apply division simplification with feedback info 
BE_EXPORTED extern BOOL OPT_fb_mpy_simp;	// Apply multiply simplification with feedback info 
#endif
#ifdef KEY
BE_EXPORTED extern UINT32 Div_Exe_Counter;	  /* Change a/b --> a/N if b==N ?             */
BE_EXPORTED extern UINT32 Div_Exe_Ratio;	  /* Change a/b --> a/N if b has high ratio   */
BE_EXPORTED extern UINT32 Div_Exe_Candidates; /* The top entries that will be taken care. */
BE_EXPORTED extern UINT32 Mpy_Exe_Counter;	/* Change a*b to a if b==N or 0.0 if b == 0.0 */
BE_EXPORTED extern UINT32 Mpy_Exe_Ratio;	/* Change a*b to a if b==N or 0.0 if b == 0.0 */
#endif
#ifdef TARG_ST
BE_EXPORTED extern UINT32 Freq_Threshold_For_Space;      /* If the PU is executed less than this, OPT_Space is set to true. */
BE_EXPORTED extern UINT32 Size_Threshold_For_Space;      /* If the PU is bigger than this, OPT_Space is set to true. */
BE_EXPORTED extern BOOL FB_CodeSize_Perf_Ratio;	 /* Optimize for size when freq < Freq_Threshold_For_Space or when size > Size_Threshold_For_Space */
#endif
BE_EXPORTED extern BOOL Fast_Exp_Allowed;	/* Avoid exp() calls? */
BE_EXPORTED extern BOOL Fast_IO_Allowed;	/* Fast printf/scanf/printw */
BE_EXPORTED extern BOOL Fast_Sqrt_Allowed;	/* Change sqrt(x) --> x * rsqrt(x) ? */
BE_EXPORTED extern BOOL Optimize_CVTL_Exp;	/* Optimize expansion of CVTL operators */
BE_EXPORTED extern BOOL Enable_CVT_Opt;	/* Optimize expansion of CVT operators */
BE_EXPORTED extern BOOL Force_IEEE_Comparisons;	/* IEEE NaN comparisons? */
BE_EXPORTED extern BOOL Inline_Intrinsics_Early;    /* Inline intrinsics just after VHO */
BE_EXPORTED extern BOOL Enable_extract;     /* Enable use of the extract whirl ops */
BE_EXPORTED extern BOOL Enable_extract_overriden;     /* ... option seen? */
BE_EXPORTED extern BOOL Enable_compose;     /* Enable use of the compose whirl ops */
BE_EXPORTED extern BOOL Enable_compose_overriden;     /* ... option seen? */
#ifdef TARG_ST
BE_EXPORTED extern BOOL Enable_Rotate;     /* Enable use of the rotate whirl ops */
BE_EXPORTED extern BOOL Enable_Rotate_overriden;     /* ... option seen? */
#endif

#ifdef TARG_ST
  /* Enable optimisation of comparisons using minmax */
BE_EXPORTED extern BOOL Enable_simplify_comparisons_per_minmax;

/***** Floating point optimizations options *****/
BE_EXPORTED extern BOOL No_Math_Errno;  /* Do not set ERRNO ? */
BE_EXPORTED extern BOOL No_Math_Errno_Set;  /* ... option seen? */
BE_EXPORTED extern BOOL Finite_Math;  /* Finite math optimizations ? */
BE_EXPORTED extern BOOL Finite_Math_Set;  /* ... option seen? */
BE_EXPORTED extern BOOL No_Rounding;  /*  ? */
BE_EXPORTED extern BOOL No_Rounding_Set;  /* ... option seen? */
BE_EXPORTED extern BOOL No_Trapping;  /* No trapping math ? */
BE_EXPORTED extern BOOL No_Trapping_Set;  /* ... option seen? */
BE_EXPORTED extern BOOL Unsafe_Math;  /* Unsafe math allowed ? */
BE_EXPORTED extern BOOL Unsafe_Math_Set;  /* ... option seen? */
BE_EXPORTED extern BOOL Fused_FP;  /* Fused FP ops allowed ? */
BE_EXPORTED extern BOOL Fused_FP_Set;  /* ... option seen? */
BE_EXPORTED extern BOOL Fused_Madd;  /* Fused madd allowed ? */
BE_EXPORTED extern BOOL Fused_Madd_Set;  /* ... option seen? */
BE_EXPORTED extern BOOL No_Denormals;  /* No denormals support  ? */
BE_EXPORTED extern BOOL No_Denormals_Set;  /* ... option seen? */
BE_EXPORTED extern BOOL OPT_Expand_Assume; /* Expand __builtin_assume ? */
BE_EXPORTED extern BOOL OPT_Expand_Assume_Set; /* ... option seen? */
// FdF 20080305: Emit warning on unsupported expressions in __builtin_assume
BE_EXPORTED extern BOOL OPT_Enable_Warn_Assume;
// TB: 20081020 Check that non void function always return a value
BE_EXPORTED extern BOOL OPT_Enable_Warn_ReturnVoid;
typedef enum {
  REASSOC_NONE,	/* No roundoff-inducing transformations */
  REASSOC_SIMPLE,	/* Simple roundoff transformations */
  REASSOC_ASSOC,	/* Reassociation transformations */
  REASSOC_ANY		/* Anything goes */
} REASSOC;
BE_EXPORTED extern REASSOC Reassoc_Level; /* reassociations level */
BE_EXPORTED extern BOOL Reassoc_Set;  /* ... option seen? */
#endif

/***** Miscellaneous optimization options *****/
BE_EXPORTED extern BOOL OPT_Pad_Common;	/* Do internal common block padding? */
BE_EXPORTED extern BOOL OPT_Reorg_Common;	/* Do common block reorganization (split)? */
BE_EXPORTED extern BOOL OPT_Reorg_Common_Set;	/* ... option seen? */
BE_EXPORTED extern BOOL OPT_Unroll_Analysis;	/* Enable unroll limitations? */
BE_EXPORTED extern BOOL OPT_Unroll_Analysis_Set;	/* ... option seen? */
BE_EXPORTED extern BOOL GCM_Speculative_Ptr_Deref;   /* allow load speculation of a memory
                                          reference that differs by a small
                                          offset from some reference location*/
BE_EXPORTED extern BOOL GCM_Speculative_Ptr_Deref_Set;   /* ... option seen? */
BE_EXPORTED extern BOOL Early_MP_Processing; /* Do mp lowerering before lno/preopt */
BE_EXPORTED extern BOOL Implied_Do_Io_Opt;	/* Do implied-do loop opt for I/O */
BE_EXPORTED extern BOOL Cray_Ivdep;		/* Use Cray meaning for Ivdep */
BE_EXPORTED extern BOOL Liberal_Ivdep;	/* Use liberal meaning for ivdep */
BE_EXPORTED extern BOOL Inhibit_EH_opt;     /* Don't remove calless EH regions */
BE_EXPORTED extern BOOL OPT_recompute_addr_flags; /* recompute addr saved */
BE_EXPORTED extern BOOL OPT_IPA_addr_analysis; /* enable the use of IPA addr analysis result */ 
BE_EXPORTED extern BOOL Delay_U64_Lowering;/* Delay unsigned 64-bit lowering to after wopt*/
BE_EXPORTED extern BOOL OPT_shared_memory;	// assume use of shared memory

/***** Instrumentation related options *****/
BE_EXPORTED extern INT32 Instrumentation_Phase_Num;
BE_EXPORTED extern BOOL Instrumentation_Enabled;
#ifdef TARG_ST
BE_EXPORTED extern char* disable_instrument;
BE_EXPORTED extern char* enable_instrument;
#endif
BE_EXPORTED extern UINT32 Instrumentation_Actions;
BE_EXPORTED extern BOOL Instrumentation_Unique_Output;
BE_EXPORTED extern INT32 Feedback_Phase_Num;
BE_EXPORTED extern OPTION_LIST* Feedback_Option;
#ifdef KEY
BE_EXPORTED extern BOOL Early_Goto_Conversion;
BE_EXPORTED extern BOOL Early_Goto_Conversion_Set;
#endif
#ifdef __cplusplus
}
#endif
#endif /* config_opt_INCLUDED */