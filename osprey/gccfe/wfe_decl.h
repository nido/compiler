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


/* WFE == WHIRL Front End */
/* translate gnu decl trees to whirl */

#ifndef wfe_decl_INCLUDED
#define wfe_decl_INCLUDED

#ifndef __cplusplus
typedef int INT;
typedef long long INT64;
typedef int TY_IDX;
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* turn a file-scope asm into a hacked-up WHIRL function */
#if defined (TARG_ST) && (GNU_FRONT_END==33)
extern void WFE_Assemble_Asm(const char *);
#else
extern void WFE_Assemble_Asm(char *);
#endif

/* expand gnu function decl tree into symtab & whirl */
extern void WFE_Start_Function (tree fndecl);

/* called after function body processed, to write out the PU */
extern void WFE_Finish_Function (void);

/* called for each initialized variable */
extern void WFE_Initialize_Decl (tree decl);

#ifdef TARG_ST
// Stubs required for g++.
extern void gxx_emits_decl (tree t);
extern void gxx_emits_asm (char *str);
#endif

/* called for each aggregate initialization */
extern void WFE_Start_Aggregate_Init (tree decl);

/* add padding to aggregate initialization */
extern void WFE_Add_Aggregate_Init_Padding (INT size);

/* add integer to aggregate initialization */
extern void WFE_Add_Aggregate_Init_Integer (INT64 val, INT size);

#if defined (TARG_ST) && !defined(_NO_WEAK_SUPPORT_)
  // (cbr) front end commonalizations 
extern __attribute__ ((weak)) void WFE_Add_Aggregate_Init_Real (REAL_VALUE_TYPE val, INT size);
#else
/* add double to aggregate initialization */
extern void WFE_Add_Aggregate_Init_Real (REAL_VALUE_TYPE val, INT size);
#endif

/* add string to aggregate initialization */
extern void WFE_Add_Aggregate_Init_String (const char *s, INT size);

/* add address to aggregate initialization */
extern void WFE_Add_Aggregate_Init_Address (tree t);

/* finish aggregate init, and set size if not previously known */
extern void WFE_Finish_Aggregate_Init (void);

/* generate a temp with extension 'name' having the initialization as specified
   by 'init' */
extern ST *WFE_Generate_Temp_For_Initialized_Aggregate (tree init, char *name);

/* handle __attribute__ ((alias)) */
extern void WFE_Assemble_Alias (tree decl, tree target);

/* handle __attribute__ ((constructor)) */
extern void WFE_Assemble_Constructor (const char *name);

/* handle __attribute__ ((destructor)) */
extern void WFE_Assemble_Destructor (const char *name);

/* call this routine when have a decl that doesn't have an initialization */
extern void WFE_Decl (tree decl);

/* call this routine to determine the return address ST at specified level */
extern ST *WFE_Get_Return_Address_ST (int level);

/* call this routine to save the SP for first alloca in a scope */
extern ST *WFE_Alloca_0 (void);

/* call this routine to assign ST for VLA as well as allocate space for it */
extern ST *WFE_Alloca_ST (tree decl);

/* call this routine to deallocate STs for VLA */
extern void WFE_Dealloca (ST *alloca0_st, tree vars);

/* call this routine to record variables assigned to registers using asm */
extern void WFE_Record_Asmspec_For_ST (tree decl, const char *asmspec, int reg);

/* call this routine to resolve conflicts between duplicate declarations */
extern void WFE_Resolve_Duplicate_Decls (tree olddecl, tree newdecl);

/* call this routine to process pragma weak declarations at end */
#if defined (TARG_ST) && (GNU_FRONT_END==33)
  /* (cbr) process weaks */
extern void WFE_Gen_Weak(tree decl, const char *name);
#else
extern void WFE_Weak_Finish();
#endif

/* call this routine to process pragma weak on encountering pragma */
extern void WFE_Add_Weak(tree decl);

/* call this routine to process pragma weak declarations at end */
extern void WFE_Weak_Finish();

#ifdef TARG_ST
  /* (cbr) process aliases */
extern void WFE_Alias_Finish(void);
#endif

/* variable to keep track track of ST to be used for varargs */
extern ST *WFE_Vararg_Start_ST;

#ifdef TARG_ST
/* get the current function declaration.  This just comes from a static
 * global variable in the absence of nested function declarations.
 */
extern tree Current_Function_Decl(void);

/* Checker wether or not a machine mode is a dynamic one with
 * an equivalent type
 */
extern int Is_Dynamic_MachineMode_With_Equiv(machine_mode_t mode);
#endif

#ifdef __cplusplus
}
#endif

#endif