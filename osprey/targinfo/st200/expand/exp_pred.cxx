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


/* EXP routines for manipulating guard registers */

#include "defs.h"
#include "erglob.h"
#include "ercg.h"
#include "tracing.h"
#include "topcode.h"
#include "tn.h"
#include "op.h"
#include "cgexp.h"
#ifdef TARG_ST
/* (cbr) might need ssa_def if available */  
#include "cg_ssa.h"
#endif

/* ====================================================================
 *   Alloc_Result_TNs
 * ====================================================================
 */
inline void 
Alloc_Result_TNs (
  TN * &tn, 
  TN * &ctn
)
{
  if (tn == NULL) tn = Build_RCLASS_TN(ISA_REGISTER_CLASS_branch);
  if (ctn == NULL) ctn = Build_RCLASS_TN(ISA_REGISTER_CLASS_branch);
}

/* ====================================================================
 *   Exp_Pred_Set
 * ====================================================================
 */
void 
Exp_Pred_Set (
  TN *dest, 
  TN *cdest, 
  INT val, 
  OPS *ops
)
{
  TOP top;

  Is_True((val & -2) == 0, ("can't set a predicate to %d", val));
  Is_True(cdest == NULL,("can't set two predicates at a time"));

  // Default predicate TN type is branch
  if (dest == NULL) dest = Build_RCLASS_TN(ISA_REGISTER_CLASS_branch);

  if (TN_register_class(dest) == ISA_REGISTER_CLASS_branch) {
    top = (val == 0) ? TOP_cmpne_r_r_b : TOP_cmpeq_r_r_b;
  }
  else if (TN_register_class(dest) == ISA_REGISTER_CLASS_integer) {
    top = (val == 0) ? TOP_cmpne_r_r_r : TOP_cmpeq_r_r_r;
  }
  else {
    FmtAssert(FALSE,("wrong register class"));
  }

  Build_OP(top, dest, Zero_TN, Zero_TN, ops);

  return;
}

/* ====================================================================
 *   Exp_Pred_Copy
 *
 *   Make so value of src is in dest, which is in guard class.
 * ====================================================================
 */
void 
Exp_Pred_Copy (
  TN *dest, 
  TN *cdest, 
  TN *src, 
  OPS *ops
)
{
  FmtAssert(TN_register_class(dest) == ISA_REGISTER_CLASS_branch,
      ("Exp_Pred_Copy: destination TN not branch"));

  if (TN_is_true_pred(src)) {
    Exp_Pred_Set(dest, cdest, 1, ops);
    return;
  }
  FmtAssert(cdest == NULL, 
	    ("Exp_Pred_Copy, can't have 2 predicates define"));

  if (TN_register_class(src) == ISA_REGISTER_CLASS_integer) {
    Expand_Int_To_Bool (dest, src, ops);
  } else {
    Exp_COPY (dest, src, ops);
  }
  return;
}

/* ====================================================================
 *   Exp_Pred_Complement
 *
 *   dest, cdest, and src must all be of rclass guard.
 * ====================================================================
 */
void 
Exp_Pred_Complement (
  TN *dest, 
  TN *cdest, 
  TN *src, 
  OPS *ops
)
{
  TN *ptn;
  if (TN_is_true_pred(src)) {
    Exp_Pred_Set(dest, cdest, !1, ops);
    return;
  }
  FmtAssert(TN_register_class(src) == ISA_REGISTER_CLASS_branch, 
	    ("Exp_Pred_Complement, src of wrong class"));
  FmtAssert(TN_register_class(dest) == ISA_REGISTER_CLASS_branch, 
	    ("Exp_Pred_Complement, dst of wrong class"));
  FmtAssert(cdest == NULL, 
	    ("Exp_Pred_Complement, can't have 2 predicates define"));

  OP *opb = SSA_Active () ? TN_ssa_def (src) : NULL;
  if (opb && OP_code(opb) == TOP_convib_r_b) {
    ptn = OP_opnd(opb, 0);
    Build_OP(TOP_cmpeq_r_r_b, dest, ptn, Zero_TN, ops);
  } else if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_norl_b_b_b)) {
    Build_OP(TOP_norl_b_b_b, dest, src, src, ops);
  } else {
    ptn = Build_RCLASS_TN(ISA_REGISTER_CLASS_integer);
    Expand_Bool_To_Int (ptn, src, MTYPE_I4, ops);
    Build_OP(TOP_cmpeq_r_r_b, dest, ptn, Zero_TN, ops);
  }
}

/* ====================================================================
 *   Exp_Pred_Compare
 * ====================================================================
 */
void 
Exp_Pred_Compare (
  TN *dest, 
  TN *cdest, 
  TN *src1, 
  TN *src2, 
  VARIANT variant,
  OPS *ops
)
{
  TOP cmp;

  FmtAssert(FALSE,("Not Implemented"));

  Alloc_Result_TNs(dest, cdest);
  BOOL is_integer = FALSE;

  cmp = Pick_Compare_TOP(&variant, &src1, &src2, &is_integer, ops);
  FmtAssert(cmp != TOP_UNDEFINED, ("Exp_Pred_Compare: unexpected comparison"));
  if (is_integer) {
    TN *tmp_int = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer);
    Build_OP (cmp, tmp_int, src1, src2, ops);
    Expand_Copy(dest, NULL, tmp_int, ops);
  } else {
    Build_OP(cmp, dest, cdest, True_TN, src1, src2, ops);
  }
}

/* ====================================================================
 *   Exp_Generic_Pred_Calc
 *
 *   Generate a generic 2-result predicate operation.
 *   COMPARE_type_or sets result1 and result2 true if qual_pred is true
 *   COMPARE_type_orcm sets result1 and result2 true if qual_pred is false
 *   COMPARE_type_and sets result1 and result2 false if qual_pred is true
 *   Note that the ST200 implementation always generate unconditional
 *   setting.
 * ====================================================================
 */
void 
Exp_Generic_Pred_Calc (
  TN* result1, 
  TN *result2, 
  COMPARE_TYPE ctype,
  TN *qual_pred, 
  OPS* ops
)
{
  TN *input1, *input2;

  // Treat constant cond tn
  if (TN_has_value(qual_pred)) {
    INT64 val = TN_value(qual_pred);
    if (ctype == COMPARE_TYPE_orcm) {
      ctype = COMPARE_TYPE_or;
      val = !val;
    }
    if (ctype == COMPARE_TYPE_andcm) {
      ctype = COMPARE_TYPE_and;
      val = !val;
    }
    if (val == 0) {
      switch (ctype) {
      case COMPARE_TYPE_or:
	// unmodified
	break;
      case COMPARE_TYPE_and:
	Expand_Immediate (result1, Gen_Literal_TN(0,4), MTYPE_B, ops);
	if (result2 != NULL) Expand_Immediate (result2, Gen_Literal_TN(0,4), MTYPE_B, ops);
	break;
      }
    } else {
      switch (ctype) {
      case COMPARE_TYPE_or:
	Expand_Immediate (result1, Gen_Literal_TN(1,4), MTYPE_B, ops);
	if (result2 != NULL) Expand_Immediate (result2, Gen_Literal_TN(1,4), MTYPE_B, ops);
	break;
      case COMPARE_TYPE_and:
	// unmodified
	break;
      }
    }
    return;
  }

  if (TN_register_class(qual_pred) == ISA_REGISTER_CLASS_branch) {
    TN *tmp  = Build_TN_Of_Mtype (MTYPE_I4);
    Exp_COPY(tmp, qual_pred, ops);
    qual_pred = tmp;
  }
  if (TN_register_class(result1) == ISA_REGISTER_CLASS_branch) {
    input1  = Build_TN_Of_Mtype (MTYPE_I4);
    Exp_COPY(input1, result1, ops);
  } else input1 = result1;
  if (result2 != NULL &&
      TN_register_class(result2) == ISA_REGISTER_CLASS_branch) {
    input2  = Build_TN_Of_Mtype (MTYPE_I4);
    Exp_COPY(input2, result2, ops);
  } else input2 = result2;

  
  TN *result = result1;
  TN *input = input1;
  TOP new_top;
  switch (ctype) {
  default:
    new_top = TOP_UNDEFINED;
    break;
  case COMPARE_TYPE_or:
    //
    // sets result1 and result2 true if qual_pred is true
    //
    new_top = TOP_orl_r_r_r;
    break;
  case COMPARE_TYPE_orcm:
    //
    // sets result1 and result2 true if qual_pred is false
    //
    new_top = TOP_norl_r_r_r;
    break;
  case COMPARE_TYPE_and:
    //
    // sets result1 and result2 false if qual_pred is false
    //
    new_top = TOP_andl_r_r_r;
    break;
  case COMPARE_TYPE_andcm:
    //
    // sets result1 and result2 false if qual_pred is true
    //
    new_top = TOP_nandl_r_r_r;
    break;
  }

  if (new_top != TOP_UNDEFINED) {
    do {
      new_top = TOP_result_register_variant(new_top, 0,
					    TN_register_class(result));
      Build_OP(new_top, result, qual_pred, input, ops);
      if (result == result2) break;
      result = result2;
      input = input2;
    } while (result != NULL);
  }

  return;
}

/* ====================================================================
 *   Exp_True_False_Preds_For_Block
 *
 * Setup the true_tn for a BB. The true_tn is a TN such that
 * it is true if the branch at the end of a BB is taken. If it false
 * through the false_tn will be set true.
 * 
 * This routine works by trying to find the compare which generates the
 * branch predicate for the block. Assuming it finds one, and it's of the
 * right form (i.e. an unc form), it attempts to simply re-use the two TN's
 * it generates. 
 *
 * Right now, if it doesn't find it, it asserts, but I don't think this is
 * going to happen, given the current way we generate things.
 *
 * The above can happen if we are trying to generate the false predicate
 * for a block that has a branch which came from a previous pass of
 * hyperblock formation. In this case, we don't have a single defining
 * compare. So if we have a predicate Pb, (which is the predicate used for
 * the branch, we wan't Pf such that Pf is TRUE if Pb is false and the block
 * is executed.  We can accomplish this by initializing Pf to 1 under the
 * block predicate, and setting it to 0 if Pb is TRUE.
 *
 * TARG_ST:
 * - For the ST200 port we don't take the latter remark into account.
 *   We consider that the predicate is always available.
 * - We return only one of true_tn of false_tn depending on the branch
 *   variant. The other is set to NULL.
 * ====================================================================
 */ 
void
Exp_True_False_Preds_For_Block (
  BB *bb, 
  TN* &true_tn, 
  TN * &false_tn
) 
{
   COMPARE_TYPE comp_type;
   TN* tn1;
   TN* tn2;
   OP* compare_op;
   OP* br_op = BB_branch_op(bb);
   BOOL reusing_tns;
   VARIANT br_variant;
   DEF_KIND kind;

   true_tn = NULL;
   false_tn = NULL;
   reusing_tns = FALSE;
   
   br_variant = CGTARG_Analyze_Branch(br_op, &tn1, &tn2);
   Is_True(V_br_condition(br_variant) == V_BR_P_TRUE,
   	   ("Can't get predicates for branch in block %d",BB_id(bb)));

   // Set the true or false tn depending on variant
   if (V_false_br(br_variant)) false_tn = tn1;
   else true_tn = tn1;

#if 0
   // Obsolete
   FmtAssert(reusing_tns, ("not reusing tns in BB %d",BB_id(bb)));

   OPS ops = OPS_EMPTY;
   if (false_tn == NULL) {
     false_tn = Build_TN_Like(true_tn);
     Exp_Pred_Complement(false_tn, NULL, true_tn, &ops);
   } else {
     true_tn = Build_TN_Like(false_tn);
     Exp_Pred_Complement(true_tn, NULL, false_tn, &ops);
   }
   BB_Insert_Ops_Before(bb,br_op,&ops);
   DevWarn("inserting inverse predicate in BB %d",BB_id(bb));
#endif
   return;
}
