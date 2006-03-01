/*

  Copyright (C) 2000 Silicon Graphics, Inc.  All Rights Reserved.

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
 * Module: config_TARG.h
 *
 * Revision history:
 *  11-Apr-96 - Original Version
 *
 * Description:
 *
 * External definitions for the -TARG group.
 *
 * Some of these variables are also defined in config.h or
 * MIPS/config_targ.h, for historical reasons.  In order to separate
 * the headers and minimize dependencies on changes to this one, new
 * group members should be defined only here, and their users should
 * explicitly include this file instead of having it indirectly
 * included (e.g. via config.h or MIPS/config_targ.h).  We should also
 * work towards removing most of the definitions from those headers.
 *
 * Exported variables should have names prefixed by "TARG_" to
 * facilitate moving them to a pushable struct later if desired.
 * See config_debug.[hc] for an example.
 *
 * ====================================================================
 * ====================================================================
 */

#ifndef config_TARG_INCLUDED
#define config_TARG_INCLUDED

#include "flags.h"

#ifdef _KEEP_RCS_ID
/*REFERENCED*/
#endif /* _KEEP_RCS_ID */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* ====================================================================
 * List of global variables that are set by the -TARG option group
 * These appear only here, requiring explicit inclusion of this file.
 * ====================================================================
 */

/* General target control: */
BE_EXPORTED extern char *ABI_Name;		/* -TARG:abi=xxx */
BE_EXPORTED extern char *Processor_Name;	/* -TARG:processor=xxx */
BE_EXPORTED extern char *ISA_Name;		/* -TARG:isa=xxx */
BE_EXPORTED extern INT16 Target_FPRs;	/* Number of target FP registers */
BE_EXPORTED extern INT32 Target_Stack_Alignment; /* -TARG:stack_alignment=nn */
BE_EXPORTED extern BOOL Pure_ABI;		/* Avoid non-ABI constructs? */

/* Fault handling: */
BE_EXPORTED extern BOOL Force_FP_Precise_Mode;	/* Force precise FP traps? */
BE_EXPORTED extern BOOL Force_Memory_Dismiss;	/* Force mem fault dismissal? */
BE_EXPORTED extern BOOL Force_Page_Zero;		/* Force mapping page zero? */
BE_EXPORTED extern BOOL Force_SMM;			/* Force sequential memory? */
BE_EXPORTED extern char *FP_Excp_Max;		/* Max FP trap enables */
BE_EXPORTED extern char *FP_Excp_Min;		/* Min FP trap enables */

/* Force calls to be indirect (i.e. use address in register)? */
BE_EXPORTED extern BOOL Force_Jalr;

/* Miscellaneous target instruction features: */
BE_EXPORTED extern BOOL Madd_Allowed;	/* Generate madd instructions? */
BE_EXPORTED extern BOOL SYNC_Allowed;
BE_EXPORTED extern BOOL Slow_CVTDL;

#ifdef TARG_ST
/* (cbr): Enable predicated Loads/Stores generation.  */
BE_EXPORTED extern BOOL Enable_Conditional_Load;
BE_EXPORTED extern BOOL Enable_Conditional_Load_Set;

BE_EXPORTED extern BOOL Enable_Conditional_Store;
BE_EXPORTED extern BOOL Enable_Conditional_Store_Set;

BE_EXPORTED extern BOOL Enable_Conditional_Prefetch;
BE_EXPORTED extern BOOL Enable_Conditional_Prefetch_Set;

/* [HK]: Enable Constant Division transformation into mul */
BE_EXPORTED extern BOOL Enable_Cst_Div;
BE_EXPORTED extern BOOL Enable_Cst_Div_Set;

/* [CG]: Enable Dismissible Loads generation.  */
BE_EXPORTED extern BOOL Enable_Dismissible_Load;
BE_EXPORTED extern BOOL Enable_Dismissible_Load_Set;

// [CG]: Enable Non IEEE Ops
// Note that the effective operators available are target dependent
BE_EXPORTED extern BOOL Enable_Non_IEEE_Ops;
BE_EXPORTED extern BOOL Enable_Non_IEEE_Ops_Set;

// [CG]: Enable 64 Bits support
// Note that the effective 64 bits operators available are target dependent
BE_EXPORTED extern BOOL Enable_64_Bits_Ops;
BE_EXPORTED extern BOOL Enable_64_Bits_Ops_Set;

// [CG]: Generation of IEEE single enabled.
BE_EXPORTED extern BOOL Enable_Single_Float_Ops;
// [CG]: Generation of IEEE double enabled.
BE_EXPORTED extern BOOL Enable_Double_Float_Ops;

// [CG]: Generation of misaligned accesses:
// FALSE: Generate composed load/store when a misaligned access is encountered
// TRUE: Let the misaligned access be generated
BE_EXPORTED extern BOOL Enable_Misaligned_Access;

// [CG]: Error level on proved misaligned accesses:
// 0: no warning
// 1: warning
// 2: error
BE_EXPORTED extern INT32 Warn_Misaligned_Access;

// [SC]: Cycle time overrides.
BE_EXPORTED extern OPTION_LIST *Cycle_Time_Overrides;
#endif

/* ====================================================================
 * List of global variables that are set by the -TARG option group
 * These also appear in config_targ.h, and are implicitly included by
 * including config.h.  They should be removed from there when it is
 * convenient.
 * ====================================================================
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */
    
#endif /* config_TARG_INCLUDED */
