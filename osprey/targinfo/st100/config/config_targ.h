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


#ifndef config_targ_INCLUDED
#define config_targ_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

/* ====================================================================
 * ====================================================================
 *
 * Module: config_targ.h
 * $Revision$
 * $Date$
 * $Author$
 * $Source$
 *
 * Revision history:
 *  15-Jun-91 - Original Version
 *  25-Sep-91 - Added subprogram interface pointers.
 *  27-Nov-91 - Reconfiguration of target representation for TP.
 *  21-Apr-93 - Set TARG_NEEDS_QUAD_OP to 1
 *  14-Mar-97 - Removed most -TARG group flags to config_TARG.h.
 *
 * Description:
 *
 * This file defines general configuration parameters which are
 * specific to the compiler's target machine and system.
 * There is an approximate distinction between -TARG option group
 * flags and their configuration (in config_TARG.[hc]), and more
 * generic target configuration (in this file).  Note that this file
 * is included in config.h, and hence indirectly in most source files,
 * whereas config_TARG.h is only included directly, so putting new
 * -TARG option-related variables in config_TARG.[hc] is to be
 * preferred to putting them here.
 *
 * See com/config.h for a detailed description of configuration
 * parameters, where they are declared, and how they are processed.
 *
 * See also:
 *	TARGET/com/targ_const.h -- Manipulation of target constants, as
 *		appropriate on the host system.
 *
 * ====================================================================
 * ====================================================================
 */

#include "mtypes.h"

/* ====================================================================
 *
 * Target selection
 *
 * Note that the current target is also given by a #define name of the
 * form TARG_xxx.  However, since a single compiler may support several
 * target variants, this symbol (defined in the TDT) will generally be
 * less specific than the variable Target below.
 *
 * ====================================================================
 */

/* ========== */
/* Target ABI */
/* ========== */

typedef enum {
  ABI_UNDEF,	/* Undefined */
  ABI_ST100	/* 32-bit */
} TARGET_ABI;

extern TARGET_ABI Target_ABI;

/* ================ */
/* Target processor */
/* ================ */

/* Define an enum representing all current target processors.
 * For variants and modifiers of these, see tp/com/config_targ.h.
 */
typedef enum {
  TARGET_UNDEF,		/* Undefined */
  TARGET_ST120_v101	/* ST100 v1.1 */
} TARGET_PROCESSOR;

extern TARGET_PROCESSOR Target;		/* -Tc */

/* return the target name for <target> */
extern char *Targ_Name (TARGET_PROCESSOR target);

#define Is_Target_ST120_v101()	(Target==TARGET_ST120_v101)


/* ========== */
/* Target ISA */
/* ========== */

typedef enum {
  TARGET_ISA_UNDEF,	/* Undefined */
  TARGET_ISA_ST100
} TARGET_ISA;

extern TARGET_ISA Target_ISA;	/* -Tc.Rc */

/* return the ISA name corresponding to <target_isa> */
extern char *Isa_Name (TARGET_ISA target_isa);

#define Is_Target_ISA_ST100()	(Target_ISA==TARGET_ISA_ST100)

/* What is the floating point format? */
#define IEEE_FP_FORMAT	TRUE
#define IBM_FP_FORMAT	FALSE

/* Do we need to implement quad float? */
/* #define TARG_NEEDS_QUAD_OPS */

/* On every target, there is an upper bound on the size pf data object
 * that can be defined.  The following two definitions define this
 * upper bound for 32 and 64 bits mode compilations respectively.
 */
#define MAX_64BITS_OBJECT_SIZE 0x000000ffffffffffULL /* 40 bits */
#define MAX_32BITS_OBJECT_SIZE 0x000000007fffffffULL /* 31 bits */

/* On most targets, it will be necessary to decide whether a text
 * address is usable in an instruction literal field.  The following
 * specifies the maximum value which we may assume a function or label
 * address will take on.
 *
 * NOTE:  On the MIPS, jumps are relative to the current 256MB segment,
 * and we always assume that we can reach anything we want to jump/call
 * to, since the linker will insert stubs if necessary.  So we treat
 * the limit as 256MB here.  We make it smaller than necessary to allow
 * for some arithmetic without overflowing the limits.
 */
#define MAX_LABEL_SYMVAL	0x03fffff0
#define MAX_FUNC_SYMVAL		0x03fffff0

/* maximum gspace (gp-relative) size */
#define DEFAULT_GSPACE		0x3fffff

/* How big is a large object? */
#define DEFAULT_LARGE_OBJECT_BYTES	64

/*
 * max size that may be needed for push/pop registers:
 * a hack for now, what I really need is to minimize what is
 * pushed/poped (eg. just the loop control registers) and have
 * this as small as possible.
 */
#define STACK_OFFSET_ADJUSTMENT         128

/* 
 * largest signed offset possible in small-frame stack model:
 * 2^9 words = 11 bits
 */
#define MAX_SMALL_FRAME_OFFSET  0x7ff    // 11 bits

/*
 * Since the largest offset allowed is 11 bits
 */
#define DEFAULT_TEMP_SPACE_BYTES        1024

/*
 * Default, maximal area that may be needed to home formal arguments
 * passed in registers (4 longs + 3 ptrs)
 */
#define DEFAULT_FORMAL_SAVE_AREA_SIZE   44

#define MAX_SFSEG_BYTES		0x7FFFFFFFFFFFFFFFLL

#define MAX_LARGE_FRAME_OFFSET	0x7FFFFFFFFFFFFFFFLL	// 64bits on all targs

typedef enum _align {
  _BYTE_ALIGN = 1,
  _CARD_ALIGN = 2,
  _WORD_ALIGN = 4,
  _DWORD_ALIGN = 8,
  _QUAD_ALIGN = 16
} ALIGN;  

/* 
 * Alignment is target dependent and should be specified as such on
 * each target for each MTYPE. This would require changes to the
 * compiler including FE changes. For now, I'll just use the
 * strongest alignment.
 */
#define DEFAULT_STACK_ALIGNMENT _WORD_ALIGN

/* ====================================================================
 *
 * Miscellaneous options
 *
 * ====================================================================
 */

/* Do we need to force NOOPs in empty cycles? */
#define FORCE_NOOPS	FALSE


/*================================================================*/
/* Architecture specific options. Variables are set in config_targ.cxx,
   #defines are set in config_targ.h
*/

extern BOOL ARCH_mask_shift_counts;
extern BOOL ARCH_generate_nor;


extern BOOL INT_packed_arithmetic;
extern BOOL PTR_packed_arithmetic;
extern BOOL BOOL_packed_arithmetic;
extern BOOL FP_packed_arithmetic;

#define ARCH_recip_is_exact TRUE
#define ARCH_has_bit_tests TRUE

/* default value for WHIRL_Keep_Cvt */
#define DEFAULT_KEEP_CVT	TRUE

/* ====================================================================
 * Target Specific Miscellany Declarations which include target 
 * dependencies.
 * ====================================================================
 */

/* How many bytes are brought in by a load/store.
 * TODO: should depend on circumstances ?
 */
//inline INT TARG_Access_Bytes (void) { return 4; }
// #define ACCESS_BYTES 4

// This is defined in the targ_isa_bundle.h now
//#define INST_BYTES 8

/* For now only the GP32 code generation is supported.
 * 2 GP32 instructions are considered to form a bundle of 2
 * and the size is 8.
 * TODO: in general this depends on the ISA mode used.
 */
//inline INT TARG_Inst_Bytes (void) { return 8; }
//inline INT TARG_Text_Alignment (void) { return 8; }

#ifdef __cplusplus
}
#endif
#endif /* config_targ_INCLUDED */
