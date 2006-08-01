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
/* translate gnu stmt trees to whirl */

#ifndef wn_stmt_INCLUDED
#define wn_stmt_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/* expand one gnu stmt tree into symtab & whirl */

#if defined (TARG_ST) && !defined(_NO_WEAK_SUPPORT_)
extern void __attribute__ ((weak)) WFE_Expand_Start_Cond (tree cond, int exitflag);
extern void __attribute__ ((weak)) WFE_Expand_Start_Else (void);
extern void __attribute__ ((weak)) WFE_Expand_End_Cond (void);

extern void __attribute__ ((weak)) WFE_Expand_Start_Loop (int exitflag, struct nesting *whichloop);
extern void __attribute__ ((weak)) WFE_Expand_Start_Loop_Continue_Elsewhere (int exitflag, struct nesting *whichloop);
extern void __attribute__ ((weak)) WFE_Expand_Loop_Continue_Here (void);
extern void __attribute__ ((weak)) WFE_Expand_End_Loop (void);
extern void __attribute__ ((weak)) WFE_Expand_Continue_Loop (struct nesting *whichloop);
extern void __attribute__ ((weak)) WFE_Expand_Exit_Loop (struct nesting *whichloop);
extern void __attribute__ ((weak)) WFE_Expand_Exit_Loop_If_False (struct nesting *whichloop, tree cond);

extern void __attribute__ ((weak)) WFE_Expand_Start_Case (int exit_flag, tree expr, tree type, char *printname);
extern void __attribute__ ((weak)) WFE_Expand_Start_Case_Dummy (void);
extern void __attribute__ ((weak)) WFE_Add_Case_Node (tree low, tree high, tree label);
extern void __attribute__ ((weak)) WFE_Emit_Case_Nodes (void);
extern void __attribute__ ((weak)) WFE_Expand_End_Case_Dummy (void);
extern void __attribute__ ((weak)) WFE_Expand_End_Case (void);
extern void __attribute__ ((weak)) WFE_Expand_Label (tree label);
extern void __attribute__ ((weak)) WFE_Expand_Goto (tree label);
extern void __attribute__ ((weak)) WFE_Expand_Exit_Something (struct nesting *n,
                                struct nesting *cond_stack,
                                struct nesting *loop_stack,
                                struct nesting *case_stack,
                                LABEL_IDX      *label_idx);
extern void __attribute__ ((weak)) WFE_Record_Switch_Default_Label (tree label);
extern void __attribute__ ((weak)) WFE_Expand_Return (tree retval);
extern void __attribute__ ((weak)) WFE_Check_Undefined_Labels (void);
extern void __attribute__ ((weak)) WFE_Expand_Computed_Goto (tree exp);
extern void __attribute__ ((weak)) WFE_Declare_Nonlocal_Label (tree label);

extern void __attribute__ ((weak)) Wfe_Expand_Asm_Operands (tree, tree, tree, tree,
				     int, const char *, int);

#else
extern void WFE_Expand_Start_Cond (tree cond, int exitflag);
extern void WFE_Expand_Start_Else (void);
extern void WFE_Expand_End_Cond (void);

extern void WFE_Expand_Start_Loop (int exitflag, struct nesting *whichloop);
extern void WFE_Expand_Start_Loop_Continue_Elsewhere (int exitflag, struct nesting *whichloop);
extern void WFE_Expand_Loop_Continue_Here (void);
extern void WFE_Expand_End_Loop (void);
extern void WFE_Expand_Continue_Loop (struct nesting *whichloop);
extern void WFE_Expand_Exit_Loop (struct nesting *whichloop);
extern void WFE_Expand_Exit_Loop_If_False (struct nesting *whichloop, tree cond);

extern void WFE_Expand_Start_Case (int exit_flag, tree expr, tree type, char *printname);
extern void WFE_Expand_Start_Case_Dummy (void);
extern void WFE_Add_Case_Node (tree low, tree high, tree label);
extern void WFE_Emit_Case_Nodes (void);
extern void WFE_Expand_End_Case_Dummy (void);
extern void WFE_Expand_End_Case (void);
extern void WFE_Expand_Label (tree label);
extern void WFE_Expand_Goto (tree label);
extern void WFE_Expand_Exit_Something (struct nesting *n,
                                struct nesting *cond_stack,
                                struct nesting *loop_stack,
                                struct nesting *case_stack,
                                LABEL_IDX      *label_idx);
extern void WFE_Record_Switch_Default_Label (tree label);
extern void WFE_Expand_Return (tree retval);
extern void WFE_Check_Undefined_Labels (void);
extern void WFE_Expand_Computed_Goto (tree exp);
extern void WFE_Declare_Nonlocal_Label (tree label);

extern void Wfe_Expand_Asm_Operands (tree, tree, tree, tree,
				     int, const char *, int);

#endif

extern LABEL_IDX WFE_Get_LABEL (tree label, int def);

#ifdef TARG_ST
  // [CL] support lexical blocks
#if !defined(_NO_WEAK_SUPPORT_)
struct lexical_block_info_t* __attribute__ ((weak)) Push_Lexical_Block();
struct lexical_block_info_t* __attribute__ ((weak)) Pop_Lexical_Block();
void __attribute__ ((weak)) Start_Lexical_Block(struct lexical_block_info_t*);
void __attribute__ ((weak)) End_Lexical_Block(struct lexical_block_info_t*);
#else
struct lexical_block_info_t*  Push_Lexical_Block();
struct lexical_block_info_t* Pop_Lexical_Block();
void Start_Lexical_Block(struct lexical_block_info_t*);
void  End_Lexical_Block(struct lexical_block_info_t*);
#endif
void Set_Current_Scope_DST(tree x);
#endif

#ifdef __cplusplus
extern LABEL_IDX lookup_cleanups(INITV_IDX&);
}
#endif

#ifdef TARG_ST
// (cbr) commonalized front ends
#ifdef __cplusplus
extern "C" {
#endif
#if defined(_NO_WEAK_SUPPORT_)
extern void Push_Scope_Cleanup (tree t);
extern void Pop_Scope_And_Do_Cleanups ();
#else
extern void __attribute__ ((weak)) Push_Scope_Cleanup (tree t);
extern void __attribute__ ((weak)) Pop_Scope_And_Do_Cleanups ();
#endif
#ifdef __cplusplus
}
#endif
#endif

#ifdef TARG_ST
extern LABEL_IDX WFE_unusable_label_idx;
extern LABEL_IDX WFE_last_label_idx;
#endif

extern void WFE_Stmt_Init (void);

#if defined (TARG_ST) && defined (FRONT_END_CPLUSPLUS)
#endif

#endif
