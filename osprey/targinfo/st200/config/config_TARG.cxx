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
 *
 * Revision history:
 *  26-Feb-97 - Original Version, extracted from config.c.
 *
 * Description:
 *
 * Configure the -TARG group (included in config.c).
 * See config_TARG.h for usage conventions.
 * See config_targ.* for more general target configuration support.
 *
 * NOTE:  There is an approximate distinction between -TARG option
 * group flags and their configuration (in this file), and more generic
 * target configuration (in config_targ.c).  Note that the related
 * header file config_targ.h is included in config.h, and hence in most
 * source files, whereas config_TARG.h is only included directly, so
 * putting new -TARG option-related variables in here is to be
 * preferred to putting them in config_targ.[hc].
 *
 * ====================================================================
 * ====================================================================
 */

/* This file is included in config.c, so it doesn't need its own set of
 * standard includes -- only the following:
 */
#include "config_TARG.h"

/* ====================================================================
 * List of global variables that are set by the -TARG option group
 * ====================================================================
 */

/* General target control: */
char *ABI_Name = NULL;		/* -TARG:abi=xxx */
char *ISA_Name = NULL;		/* -TARG:isa=xxx */
char *Processor_Name = NULL;	/* -TARG:processor=xxx */
static char * Platform_Name = NULL;
INT16 Target_FPRs = 0;		/* -TARG:fp_regs=nn */
INT32 Target_Stack_Alignment = 8; /* -TARG:stack_alignment=nn */
BOOL Pure_ABI = FALSE;		/* Avoid non-ABI constructs? */

/* Fault handling: */
BOOL Force_FP_Precise_Mode = FALSE;	/* Force precise FP traps? */
BOOL Force_Memory_Dismiss = FALSE;	/* Force mem fault dismissal? */
BOOL Force_Page_Zero = FALSE;		/* Force mapping page zero? */
BOOL Force_SMM = FALSE;			/* Force sequential memory? */
char *FP_Excp_Max = NULL;		/* Max FP trap enables */
char *FP_Excp_Min = NULL;		/* Min FP trap enables */

/* Miscellaneous target instruction features: */
BOOL Madd_Allowed = TRUE;		/* Generate madd instructions? */
BOOL Force_Jalr = FALSE;	/* Force calls via address in register */
static BOOL Slow_CVTDL_Set = FALSE;

BOOL SYNC_Allowed = TRUE;
BOOL Slow_CVTDL = FALSE;

#ifdef TARG_ST
// (cbr) Enable Dismissible Loads/Stores generation.
BOOL Enable_Conditional_Load = FALSE;
BOOL Enable_Conditional_Load_Set;

BOOL Enable_Conditional_Store = FALSE;
BOOL Enable_Conditional_Store_Set;

// [CG]: Enable Dismissible Loads generation.
BOOL Enable_Dismissible_Load = TRUE;
BOOL Enable_Dismissible_Load_Set;

// [CG]: Enable Non IEEE Ops
// Note that the effective operators available are target dependent
BOOL Enable_Non_IEEE_Ops = FALSE;
BOOL Enable_Non_IEEE_Ops_Set = FALSE;

// [CG]: Enable 64 Bits support
// Note that the effective 64 bits operators available are target dependent
BOOL Enable_64_Bits_Ops = FALSE;
BOOL Enable_64_Bits_Ops_Set = FALSE;

// [CG]: Generation of IEEE single enabled.
BOOL Enable_Single_Float_Ops;

// [CG]: Generation of IEEE double enabled.
BOOL Enable_Double_Float_Ops;

// [CG]: Generation of misaligned load/store:
// FALSE: Generate composed load/store when a misaligned access is encountered
// TRUE: Let the misaligned access be generated
/* ST200 note: this should be turned on on ST200 
 * However currently the compiler itself generate misaligned access, and
 * we don't know how to differenciate this with user generated misaligned
 * accesses. Thus we are obliged to force it.
 */
INT32 Enable_Misaligned_Access;


// [CG]: Error level on proved misaligned accesses:
// 0: no warning
// 1: warning
// 2: error
INT32 Warn_Misaligned_Access = 0;
#endif

/* Target machine specification options.  This group defines the target
 * ABI, ISA, processor, and FPR count.  It should also be the home for
 * options like specifying processor revisions.
 */
static OPTION_DESC Options_TARG[] = {
  { OVK_NAME,	OV_VISIBLE,	FALSE, "abi",		"ab",
    0, 0, 0, &ABI_Name,		NULL,
    "Specify the ABI to follow" },
  { OVK_NAME,	OV_VISIBLE,	FALSE, "isa",		"is",
    0, 0, 0, &ISA_Name,		NULL,
    "Specify the instruction set architecture to use" },
  { OVK_NAME,	OV_VISIBLE,	FALSE, "platform",	"pl",
    0, 0, 0, &Platform_Name,	NULL,
    "Specify the target platform" },
  { OVK_NAME,	OV_VISIBLE,	FALSE, "processor",	"pr",
    0, 0, 0, &Processor_Name,	NULL,
    "Specify the target microprocessor" },

  /* Miscellaneous features: */
  { OVK_BOOL,	OV_VISIBLE,	FALSE, "dismiss_mem_faults",	"dis",
    0, 0, 0,	&Force_Memory_Dismiss, NULL,
    "Force kernel to ignore memory faults (SIGSEGV/SIGBUS)" },
  { OVK_BOOL,	OV_SHY,		FALSE, "page_zero",	"",
    0, 0, 0,	&Force_Page_Zero, NULL,
    "Force the kernel to map page zero into address space" },
  { OVK_BOOL,	OV_INTERNAL,	FALSE, "pure",	"pu",
    0, 0, 0,	&Pure_ABI,	NULL,
    "Generate pure ABI-compliant code" },
  { OVK_INT32,	OV_INTERNAL,	FALSE, "fp_regs",	"fp_r",
    32, 16, 32, &Target_FPRs,	NULL,
    "Specify number of FP registers to use (16 or 32)" },
  { OVK_INT32,  OV_INTERNAL,    FALSE, "stack_alignment", "sta",
    16,  4, 64, &Target_Stack_Alignment, NULL,
    "Specify the alignment of a stack frame" },
#ifdef TARG_ST
  // (cbr)
  { OVK_BOOL,   OV_VISIBLE,    FALSE, "conditional_load", "",
    0, 0, 0,    &Enable_Conditional_Load, &Enable_Conditional_Load_Set,
    "Enable generation of predicated load" },

  { OVK_BOOL,   OV_VISIBLE,    FALSE, "conditional_store", "",
    0, 0, 0,    &Enable_Conditional_Store, &Enable_Conditional_Store_Set,
    "Enable generation of predicated store" },

  // [CG]
  { OVK_BOOL,   OV_VISIBLE,    FALSE, "dismissible_load", "",
    0, 0, 0,    &Enable_Dismissible_Load, &Enable_Dismissible_Load_Set,
    "Enable generation of dismissible load" },

  // [CG]
  { OVK_BOOL,   OV_VISIBLE,    FALSE, "enable_non_ieee", "",
    0, 0, 0,    &Enable_Non_IEEE_Ops, &Enable_Non_IEEE_Ops_Set,
    "Enable generation of non-IEEE ops" },
  { OVK_BOOL,   OV_VISIBLE,    FALSE, "enable_64bits", "",
    0, 0, 0,    &Enable_64_Bits_Ops, &Enable_64_Bits_Ops_Set,
    "Enable support for 64 bits" },

  // [CG]
  { OVK_BOOL,   OV_VISIBLE,    FALSE, "enable_misaligned", "",
    0, 0, 0,    &Enable_Misaligned_Access, NULL,
    "Enable generation of faulting memory accesses when proved misaligned" },
  { OVK_INT32,   OV_VISIBLE,    FALSE, "warn_misaligned", "",
    0, 0, 2,    &Warn_Misaligned_Access, NULL,
    "Warning level on misaligned access (0 = no warning, 1 = warning, 2 = error" },
#endif

  /* Unimplemented options: */
  /* Obsolete options: */

  { OVK_COUNT }		/* List terminator -- must be last */
};


/* ====================================================================
 *
 * Configure_Source_TARG
 *
 * Same as Configure_Source except here we handle target specifics.
 *
 * ====================================================================
 */
static void
Configure_Source_TARG ( char *filename )
  /**  NOTE: filename CAN BE NULL */
{
}
