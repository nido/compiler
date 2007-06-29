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


#ifndef defs_INCLUDED
#define defs_INCLUDED
/* ====================================================================
 * ====================================================================
 *
 * Module: defs.h
 *
 * Revision history:
 *  21-Aug-89 - Original Version
 *  24-Jan-91 - Copied for TP/Muse
 *
 * Description:
 *
 * This header file contains definitions of ubiquitous configuration
 * parameters used in the microcode compiler, in particular mappings
 * of the host-independent type names used to the host-supported basic
 * types.
 *
 * ====================================================================
 * ====================================================================
 */

#ifdef _KEEP_RCS_ID
#endif /* _KEEP_RCS_ID */

#define PROTOTYPES 1
/* We always want Insist error messages: */
#define Insist_On 1

/* ====================================================================
 *
 * System libraries to be made generally available
 *
 * ====================================================================
 */

/* Make stdio, and string support generally available: */
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "W_bstring.h"	/* for bzero */

#ifndef BZERO_DEFINED
#define BZERO_DEFINED
#ifdef __CYGWIN__
#define BZERO(s,n) bzero((char*)(s),n)
#else
#ifdef __MINGW32__
#define BZERO(s,n) memset ((void*)(s),0,(n))
#else

#if __GNUC__ >= 3
#define BZERO(s,n) memset ((void*)(s),0,(n))
#else
#define BZERO(s,n) bzero((void*)(s),n)
#endif // __GNUC__ >= 3

#endif
#endif
#endif

#ifndef BCOPY_DEFINED
#define BCOPY_DEFINED
#ifdef __CYGWIN__
#define BCOPY(in,out,n) bcopy((const char*)in,(char*)out,n)
#else
#ifdef __MINGW32__
#define BCOPY(in,out,n) memmove((out),(in),(n))
#else

#if __GNUC__ >= 3
#define BCOPY(in,out,n) memcpy((out),(in),n)
#else
#define BCOPY(in,out,n) bcopy(in,out,n)
#endif // __GNUC__ >= 3


#endif
#endif
#endif

/* define the standard integer types */
#include "W_stdint.h"

#ifdef __cplusplus
extern "C" {
#endif


/* Since <signal.h> doesn't prototype signal, upsetting ccom: */
/* extern void (*signal(int, void (*) (int, ...)))(int, ...); */


/* ====================================================================
 *
 * Characterize the host machine
 *
 * ====================================================================
 */

#ifdef __sgi
# define HOST_SGI
#endif

/***** Note the size of a word (in malloc units): *****/
#ifndef HOST_WORD_SIZE
# define HOST_WORD_SIZE	4
#endif
#define WORDSIZE HOST_WORD_SIZE	/* OBSOLETE */
#if HOST_WORD_SIZE == 2
# define TWO_BYTE_WORDS
#endif
#if HOST_WORD_SIZE == 4
# define FOUR_BYTE_WORDS
#endif
#if HOST_WORD_SIZE == 8
# define EIGHT_BYTE_WORDS
#endif

/* Map low indices to low-order bits in the bit vector package: */
#define BV_LITTLE_ENDIAN_BIT_NUMBERING	1

/* Should bit vector packages use table lookup instead of shifts? */
#if 0
#define BV_MEMORY_BIT_MASKS	/* when shifting is slow */
#endif

/* ====================================================================
 *
 * The compiler process being compiled
 *
 * The command line must specify one of -DDRIVER, -DFRONT_END_xxx, or
 * -DBACK_END.  The following sets related defines based on those.
 *
 * NOTE: The original Josie sources sometimes used aliases C_FRONT_END
 * for FRONT_END_C, FETYPE_CH for F77_FRONT_END, COMMON_CORE for
 * BACK_END, and ONE_PROC for SINGLE_PROCESS.  These have all been
 * eliminated except for uses of FETYPE_CH specifically to refer to
 * the FORTRAN front end's CHARACTER type.
 *
 * ====================================================================
 */

#define	COMPILER_DRIVER	1
#define	COMPILER_FE_C	2
#define	COMPILER_FE_CC	3
#define	COMPILER_FE_F77	4
#define	COMPILER_FE_F90	5
#define COMPILER_BE	6
#define COMPILER_TDGEN	7

#ifdef DRIVER
# define COMPILER_PROCESS COMPILER_DRIVER
# define COMPILER 1
#endif
#if defined(FRONT_END_C)
# define COMPILER_PROCESS COMPILER_FE_C
# define FRONT_END 1
# define COMPILER 1
#endif
#if defined(FRONT_END_CPLUSPLUS)
# define COMPILER_PROCESS COMPILER_FE_CC
# define FRONT_END 1
# define COMPILER 1
#endif
#ifdef FRONT_END_F77
# define COMPILER_PROCESS COMPILER_FE_F77
# define FRONT_END_FORTRAN 1
# define FRONT_END 1
# define COMPILER 1
#endif
#ifdef FRONT_END_F90
# define COMPILER_PROCESS COMPILER_FE_F90
# define FRONT_END_FORTRAN 1
# define FRONT_END 1
# define COMPILER 1
#endif
#ifdef BACK_END
# define COMPILER_PROCESS COMPILER_BE
# define COMPILER 1
#endif
#ifdef TDGEN
# define COMPILER_PROCESS COMPILER_TDGEN
#endif

/* Are we compiling front end and back end as a single process? */
#if defined(FRONT_END) && defined(BACK_END)
# define SINGLE_PROCESS 1
#endif


/* Allow inline keyword, making it 'static' for debugging if Is_True_On */
#ifdef _LANGUAGE_C
#ifndef inline
#ifdef Is_True_On
#define inline static
#else
#define inline static __inline
#endif
#endif
#endif


#include "defs_exported.h"

/* Define the target's basic INT type: */
/* WARNING:  This isn't quite accurate.  A single compiler may
 * support multiple targets with multiple possibilities for these
 * types.  They should be viewed as maximal for the supported
 * targets.
 * TODO:  They aren't there yet (e.g. the 64-bit targets).
 * Determine how to deal with this -- maybe a completely different
 * mechanism is required.
 */
typedef INT64	TARG_INT;	/* Individual objects */
typedef mINT64	mTARG_INT;	/* Table components */
typedef UINT64  TARG_UINT;
typedef mUINT64 mTARG_UINT;


/* ====================================================================
 *
 * Ubiquitous compiler types and macros
 *
 * We want several compiler standard types to be universally available
 * to allow their use without including their natural defining header
 * files.
 *
 * ====================================================================
 */

/* Define standard functions: */
#ifdef MAX
# undef MAX
#endif
#ifdef MIN
# undef MIN
#endif
#define MAX(a,b)	((a>=b)?a:b)
#define MIN(a,b)	((a<=b)?a:b)

inline INT Max(INT i, INT j)
{
  return MAX(i,j);
}
inline INT Min(INT i, INT j)
{
  return MIN(i,j);
}

#ifdef TARG_ST
#include <float.h>

inline
BOOL KnuthCompare(float af, float bf, float relError)
{
    float aaf = af < 0.0F ? -af : af ;
    float abf = bf < 0.0F ? -bf : bf ;
    float afmbf = af - bf ; 
    float aafmbf =  afmbf < 0.0F ? -afmbf : afmbf ;
    float mxaafabf = aaf > abf ? aaf : abf ; 
    return aafmbf <= relError * mxaafabf ;
}

#define KnuthCompareEQ(af, bf) (((af) == (bf)) ||  KnuthCompare(af, bf, FLT_EPSILON))
#define KnuthCompareNE(af, bf) (((af) != (bf)) && !KnuthCompare(af, bf, FLT_EPSILON))
#define KnuthCompareLE(af, bf) (((af) <= (bf)) ||  KnuthCompare(af, bf, FLT_EPSILON))
#define KnuthCompareLT(af, bf) (((af) <  (bf)) && !KnuthCompare(af, bf, FLT_EPSILON))
#define KnuthCompareGE(af, bf) (((af) >= (bf)) ||  KnuthCompare(af, bf, FLT_EPSILON))
#define KnuthCompareGT(af, bf) (((af) >  (bf)) && !KnuthCompare(af, bf, FLT_EPSILON))
#endif

/* --------------------------------------------------------------------
 * VERY_BAD_PTR
 * a memory address that is almost certainly going to cause 
 * a SIG. Many pointer values are initialized to this value to
 * catch any uninitialized uses of that pointer.
 *
 * Actually this could be host dependent, but the value below 
 * is good enough for all current hosts
 * --------------------------------------------------------------------
 */
#define VERY_BAD_PTR (0xfffffff)

#ifdef __cplusplus
}
#endif
#endif /* defs_INCLUDED */
