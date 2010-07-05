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
/* translate gnu expr trees to whirl */

#ifndef wn_expr_INCLUDED
#define wn_expr_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/* One time initialization */
extern void WFE_Expr_Init (void);

/* expand one gnu stmt tree into symtab & whirl */
extern void WFE_One_Stmt (tree exp);

#ifdef TARG_ST
/* same as WFE_One_Stmt but use new label indexes */
extern void WFE_One_Stmt_Cleanup (tree exp);
#endif

/* generate a RET statement */
extern void WFE_Null_Return (void);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
// expand one gnu expr tree into whirl; called only from C++ files
extern WN * WFE_Expand_Expr (tree exp, 
			     bool need_result = TRUE,
			     TY_IDX nop_ty_idx = 0,
			     TY_IDX component_ty_idx = 0,
			     INT64 component_offset = 0,
			     UINT32 field_id = 0,
			     bool is_bit_field = FALSE); 

extern WN  *WFE_Rcomma_Block;
extern int  WFE_Disable_Rcomma;
extern WN* WFE_Expand_Expr_With_Sequence_Point (tree exp, TYPE_ID mtype);

/* rhs is the WN representing the rhs of a MODIFY_EXPR node; this routine
 * processes the lhs of the node and generate the appropriate form of store
 */
extern WN * WFE_Lhs_Of_Modify_Expr (tree_code assign_code,
				    tree lhs,
				    bool need_result,
				    TY_IDX component_ty_idx,
				    INT64 component_offset,
				    UINT32 field_id,
				    bool is_bit_field,
				    WN *rhs_wn,
				    PREG_NUM rhs_preg_num,
				    bool is_realpart,
				    bool is_imagpart);

/* get integer value from INTEGER_CST node */
extern UINT64 Get_Integer_Value (tree exp);

/* traverse the tree and addr_saved if address of a variable is taken */
extern void WFE_Set_ST_Addr_Saved (WN *);

#ifdef TARG_ST
// Convert inplace target order words in 'buf' to host order. 'buf' is a two
// word array. 
extern void Setup_EH_Region ();

// Make standard integral ctype promotions
extern TYPE_ID WFE_Promoted_Type(TYPE_ID mtype);
#endif

#endif /* __cplusplus */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if defined (TARG_ST) && !defined(_NO_WEAK_SUPPORT_)
extern __attribute__ ((weak)) void WFE_Expand_Start_Stmt_Expr (tree);
extern __attribute__ ((weak)) void WFE_Expand_End_Stmt_Expr (tree);
extern __attribute__ ((weak)) void WFE_Expand_Start_Compound_Literal_Expr (tree);
extern __attribute__ ((weak)) void WFE_Expand_End_Compound_Literal_Expr (tree);
#else
extern void WFE_Expand_Start_Stmt_Expr (tree);
extern void WFE_Expand_End_Stmt_Expr (tree);
extern void WFE_Expand_Start_Compound_Literal_Expr (tree);
extern void WFE_Expand_End_Compound_Literal_Expr (tree);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif