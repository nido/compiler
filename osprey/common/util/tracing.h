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


#ifndef tracing_INCLUDED
#define tracing_INCLUDED

#ifndef defs_INCLUDED
/* Sort of bizarre, including from common/com to common/util, but it
 * seems it must be so.
 */
#include "defs.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ====================================================================
 * ====================================================================
 *
 * Module: tracing.h
 *
 * Revision history:
 *  08-Sep-89 - Original Version
 *  24-Jan-91 - Copied for TP/Muse
 *  22-May-91 - Integrated additional Josie functionality
 *
 * Description:
 *
 * External interface for tracing support in the Muse compilers and
 * associated tools.  The basic methodology assumed is the use of
 * fprintf to a trace file (which may be, and defaults to, stdout).
 * The support provided by the tracing package is primarily managing
 * the trace file and flags controlling the traces.
 *
 * ====================================================================
 * ====================================================================
 */


#ifdef _KEEP_RCS_ID
#endif /* _KEEP_RCS_ID */

/* Is_Trace */
#ifndef Is_Trace
#ifdef Is_True_On
#define Is_Trace(Cond, Parmlist) { if (Cond) fprintf Parmlist ; }
#define Is_Trace_cmd(Cond, Cmd) { if (Cond) Cmd ; }
#else
#define Is_Trace(Cond, Parmlist) ((void) 1)
#define Is_Trace_cmd(Cond, Cmd)  ((void) 1)
#endif
#endif

/* The following strings are useful in creating trace messages: */
BE_EXPORTED extern char *SBar;	/* Single-dash bar w/NL, full page width */
BE_EXPORTED extern char *DBar;	/* Double-dash bar w/NL, full page width */
BE_EXPORTED extern char *Sharps;	/* Sharps w/NL, full page width */


/* ====================================================================
 *
 * Trace Flag Literals
 *
 * This package supports several kinds of trace conditions, to allow
 * the programmer a great deal of control over the tracing which occurs
 * at runtime.  (Otherwise, good tracing would be discouraged by the
 * sheer volume which would result, or by the difficulty of setting it
 * up.)  The trace flag kinds supported are:
 *
 *  1)	INFO flags control general information about a compilation,
 *	e.g. timing, object code statistics, or failure tracing.
 *	They are represented by mask bits in a single word.
 *
 *  2)	DEBUG flags control options which affect the execution of the
 *	compiler for debugging purposes, e.g. to perform extra checks
 *	or to try alternate algorithms.  They are also represented by
 *	a word of mask bits.
 *
 *  3)	IR flags control the tracing of the current IR state after
 *	individual compiler phases.  They are represented by the phase
 *	number to which they apply.
 *
 *  4)	SYMTAB flags control the tracing of the current symbol table
 *	state after individual compiler phases.  They are represented
 *	by the phase number as well.
 *
 *  5)	TN flags control the tracing of the current set of TNs after
 *	individual compiler phases.  They are represented by the phase
 *	number as well.
 *
 *  6)	Phase-specific flags control the tracing in individual compiler
 *	components.  Each such unit has available a word of mask bits
 *	on which to base its tracing options, represented by the phase
 *	number and mask.  Note that the phase numbers used for this
 *	purpose are the same as are used for IR and SYMTAB traces.
 *
 * ====================================================================
 */

/* Negative integers represent the INFO, DEBUG, IR, and SYMTAB traces,
 * as well as miscellaneous trace options:
 */
#define TKIND_INFO	-1	/* Specify an information option */
#define TKIND_DEBUG	-2	/* Specify a debug option */
#define TKIND_IR	-3	/* Trace IR for the given pass */
#define TKIND_SYMTAB	-4	/* Trace symbol table for given pass */
#define TKIND_TN	-5	/* Trace TNs for given pass */
#define TKIND_BB	-6	/* Specify a BB number for tracing */
#define TKIND_XPHASE	-7	/* Specify final execution phase */
#define TKIND_CTRL	-8	/* Specify a control option */
#define TKIND_ALLOC	-9	/* Trace memory allocation */
#ifdef TARG_ST
#define TKIND_GML	-10	/* Generate graphml IR dump */
#define TKIND_MIN	-11	/* Smallest valid function number */
#else
#define TKIND_MIN	-10	/* Smallest valid function number */
#endif
/* Several predefined masks for TKIND_INFO cases: */
#define TINFO_TIME	1	/* Timing/resource information */
#define TINFO_CTIME	2	/* Compilation-only timing information */
#define TINFO_STATS	8	/* Code size statistics */
#define TINFO_SOURCE	32	/* Source line printing alongside IR dumps */
#define TINFO_TFLAGS	64	/* Print available trace options */
#define TINFO_PREFIXDUMP 128	/* Dump WHIRL trees in prefix order */

/* Positive integers represent phase numbers for per-phase traces:
 * WARNING:  If you change this list (adding or deleting entries), you
 * must change the Phases table in tracing.c.
 */
#define TP_MIN		1	/* Smallest valid phase number */

/* Miscellaneous "phases": */
/* note: for TP_PTRACE[1|2]_flags, see below */
#define TP_PTRACE1	1	/* Performance tracing */
#define TP_PTRACE2	2	/* Performance tracing */
#define TP_MISC		3	/* Miscellaneous */

/* Front end phases: */
#define TP_SEMANTICS	8	/* Semantic analyzer */
#define TP_IRB		10	/* IR (WHIRL) builder */

/* Intermediate utility phases: */
#define TP_IR_READ	11	/* IR (WHIRL) reader/writer */
#define TP_WHIRL2FC	12	/* WHIRL to Fortran/C */
#define TP_WHIRLSIMP    13	/* WHIRL simplifier */
#define TP_REGION	14	/* REGION related stuff */
#define TP_ORI		15	/* Olimit Region Insertion phase */
#define TP_FEEDBACK	16	/* Decorating WHIRL/CFG with feedback */

/* IPA/inlining phases: */
#define TP_INLINE	17	/* Inliner */
#define TP_IPL		18	/* IPA local (summary) phase */
#define TP_IPA		19	/* IPA main analysis phase */
#define TP_IPO		20	/* IPA main optimization phase */
#define TP_IPM		21	/* IPA miscellaneous */

/* Global optimizer phases: */
#define TP_ALIAS	24	/* Alias/mod/ref analysis */
#define TP_WOPT1	25	/* Global optimization */
#define TP_WOPT2	26	/* More global optimization */
#define TP_WOPT3	27	/* Even more global optimization */
#define TP_GLOBOPT	TP_WOPT1

/* Loop nest optimizer phases: */
#define TP_VECDD	30	/* Vector data dependency analysis */
#define TP_LNOPT	31	/* Loop Nest Optimization */
#define TP_LNOPT2	32	/* More Loop Nest Optimization */
#define TP_LNOPT3	33	/* Even more Loop Nest Optimization */

#define TP_VHO_LOWER	36	/* VHO lowering */
#define TP_LOWER	37	/* WHIRL lowering */
#define TP_LOWER90      38      /* F90 Lowering */

/* Code generator phases: */
#define TP_DATALAYOUT	39	/* Data layout */
#define TP_CG		40	/* Code generator miscellaneous */
#define TP_CGEXP	41	/* Code generator expansion */
#define TP_LOCALIZE	42	/* Localize TNs */
#define TP_FIND_GLOB	43	/* Find global register live ranges */
#define TP_EBO		44	/* Extended Block Optimizer */
#define TP_CGPREP	45	/* Code generator scheduling prep */
#define TP_FLOWOPT	47	/* Control flow optimization */
#define TP_GCM		48	/* Global code motion */
#define TP_CGLOOP	49	/* Code generator loop optimization */
#define TP_SWPIPE	50	/* Software pipelining */
#define TP_SRA		51	/* SWP register allocation */
#define TP_SCHED	52	/* Scheduling */
#define TP_GRA		53	/* Global register allocation */
#define TP_ALLOC	54	/* Local register allocation */
#define TP_PSGCM	55	/* Post Schedule Global code motion */
#define TP_EMIT		56	/* Code emission */
#define TP_HBF		57	/* Hyperblock formation */
#define TP_PQS		58	/* Predicate query system */
#define TP_THR		59	/* Tree-Height reduction */

#ifdef TARG_ST
#define TP_SSA          60	/* SSA representation */
#define TP_SELECT       61	/* Select generation */
#define TP_RANGE        62      /* Range analysis */
#define TP_LICM         63      /* Loop invariant Code motion */
#define TP_EXTENSION    64      /* Extension support (loader, native support) */
#define TP_TAIL         65	/* Tailmerge */
#define TP_AFFIRM	66	/* __builtin_assume and AFFIRM property. */
#endif

#ifdef TARG_ST
/* Keep it last. */
#define TP_TEMP         99	/* Temporary use */
#endif

#ifdef TARG_ST
/* WARNING: TP_LAST must be at least as large as the largest phase
 * number above, and TP_COUNT must be at least one larger.
 */
#define TP_LAST         TP_TEMP   /* Largest valid phase number */
#else
/* WARNING: TP_LAST must be at least as large as the largest phase
 * number above, and TP_COUNT must be at least one larger.
 */
#define TP_LAST         TP_THR	  /* Largest valid phase number */
#endif

#define TP_COUNT	TP_LAST+1 /* Number of valid phase numbers */


#ifdef BINUTILS
// Trace are not available when compiling xp70/binutils
// but following functions must be defined
static BOOL Get_Trace ( INT func, INT arg ) { return FALSE; }
static FILE *Get_Trace_File ( void ) { return NULL; }
#define TFile Get_Trace_File()
#else

/* Extract the phase number from a trace option: */
BE_EXPORTED extern INT32 Get_Trace_Phase_Number ( char **cp, char *arg );

/* List the trace phases to TFile: */
BE_EXPORTED extern void List_Phase_Numbers ( void );

/* ====================================================================
 * TP_PTRACE[1|2]_flags - reserved PTRACE flags
 * ====================================================================
 */
#define TP_PTRACE1_ALL       0x001   /* get all performance tracing */
#define TP_PTRACE1_INL       0x002   /* get INLiner performance tracing */
#define TP_PTRACE1_IPA       0x004   /* get IPA performance tracing */
#define TP_PTRACE1_LNO       0x008   /* get LNO performance tracing */
#define TP_PTRACE1_OPT       0x010   /* get OPT performance tracing */
#define TP_PTRACE1_CG        0x020   /* get CG  performance tracing */
#define TP_PTRACE1_IPALNO    0x040   /* get IPA performance tracing */
#define TP_PTRACE1_IPA_CPROP 0x080   /* get IPA cprop tracing */
#define TP_PTRACE1_CALLINFO  0x100   /* get LNO call info tracing */ 
#define TP_PTRACE1_PARALLEL  0x200   /* get LNO parallel tracing  */ 
#define TP_PTRACE1_NOHDR     0x400   /* suppress printing tlog header */ 


/* ====================================================================
 *
 * Trace Flag Management
 *
 * ====================================================================
 */

/* Set a trace flag specified by:
 *  Function		Argument	Resulting action
 *  --------		--------	----------------
 *  TKIND_INFO		flag mask	Enable masked traces
 *  TKIND_DEBUG		flag mask	Enable masked options
 *  TKIND_IR		phase number	Enable IR trace for phase
 *  TKIND_SYMTAB	phase number	Enable SYMTAB trace for phase
 *  TKIND_TN		phase number	Enable TN trace for phase
 *  TKIND_BB		BB number	Restrict tracing to BB
 *  TKIND_XPHASE	phase number	Stop execution after phase
 *  TKIND_CNTL		control number	Set control option identified
 *  TKIND_ALLOC		phase number	Enable mem allocation traces
 *  TKIND_GML		phase number	 Generate graphml IR dump 
 * 
 *  phase number	flag mask	Enable masked per-phase traces
 */
BE_EXPORTED extern void Set_Trace ( INT func, INT arg );

/* Note a PU to be traced: */
BE_EXPORTED extern void Set_Trace_Pu ( char *name );
BE_EXPORTED extern void Set_Trace_Pu_Number ( INT number );

/* Set current PU for pu tracing */
BE_EXPORTED extern void Set_Current_PU_For_Trace ( char *name, INT number );

/* Tracing for a single region */
BE_EXPORTED extern void Set_Trace_Region_Number ( INT number );
BE_EXPORTED extern void Set_Current_Region_For_Trace ( INT number );

/* Determine whether a given trace option is enabled.
 */
BE_EXPORTED extern BOOL Get_Trace ( INT func, INT arg );

/* Determine whether a given BB (pass in it's BB_id()) is enabled for 
 * tracing, independent of other trace options:
 */
BE_EXPORTED extern BOOL Get_BB_Trace ( INT bb_id );

/* Determine whether execution should stop after this phase: */
BE_EXPORTED extern BOOL Stop_Execution ( INT phase );


/* ====================================================================
 *
 * Trace File Management
 *
 * ====================================================================
 */

/* The trace file descriptor, to be used in fprintf tracing: */
BE_EXPORTED extern FILE *Get_Trace_File ( void );

/* By default, traces are sent to stdout, which of course is open on
 * startup.  If tracing is to be directed to another file (recommended
 * for permanence), this routine should be called before any tracing is
 * done.  It will close the previous trace file (if not stdout) and
 * open the new one, setting TFile to the new file descriptor.
 */
BE_EXPORTED extern void Set_Trace_File ( char *filename );

BE_EXPORTED extern void Set_Trace_File_internal ( FILE *);

#define TFile Get_Trace_File()

/* Indent the given file according to the current execution stack
 * depth.  This routine is useful for tracing recursive algorithms.
 */
BE_EXPORTED extern void Nest_Indent ( FILE *file );

BE_EXPORTED extern INT Get_Current_Phase_Number( void );
BE_EXPORTED extern void Set_Current_Phase_Number(INT);

#endif /* BINUTILS */

#ifdef __cplusplus
}
#endif
#endif /* tracing_INCLUDED */
