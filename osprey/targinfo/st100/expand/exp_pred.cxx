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
  if (tn == NULL) tn = Build_RCLASS_TN(ISA_REGISTER_CLASS_guard);
  if (ctn == NULL) ctn = Build_RCLASS_TN(ISA_REGISTER_CLASS_guard);
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

  // Do not make cdest
  if (dest == NULL) dest = Build_RCLASS_TN(ISA_REGISTER_CLASS_guard);
  top = (val == 0) ? TOP_GP32_CLRG_GT_BR : TOP_GP32_SETG_GT_BR;
  Build_OP(top, dest, True_TN, ops);
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
  FmtAssert(TN_register_class(dest) == ISA_REGISTER_CLASS_guard,
      ("Exp_Pred_Copy: destination TN not ISA_REGISTER_CLASS_guard"));

  if (TN_is_true_pred(src)) {
    Exp_Pred_Set(dest, cdest, 1, ops);
    return;
  }

  Exp_COPY (dest, src, ops);
  return;
}

/* ====================================================================
 *   Exp_Pred_Compliment
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
  if (TN_is_true_pred(src)) {
    Exp_Pred_Set(dest, cdest, !1, ops);
    return;
  }

  Build_OP(TOP_GP32_NOTG_GT_BR_BR, dest, True_TN, src, ops);
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

  cmp = Pick_Compare_TOP(&variant, &src1, &src2, ops);
  FmtAssert(cmp != TOP_UNDEFINED, ("Exp_Pred_Compare: unexpected comparison"));
  Build_OP(cmp, dest, cdest, True_TN, src1, src2, ops);
}

/* ====================================================================
 *   Exp_Generic_Pred_Calc
 *
 *   Generate a generic 2-result predicate operation.
 *   COMPARE_type_or sets result1 and result2 true if qual_pred is true
 *   COMPARE_type_and sets result1 and result2 false if qual_pred is true
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
  TOP pred_top;

  FmtAssert(FALSE,("Not Implemented"));
 
  switch (ctype) {
  case COMPARE_TYPE_or:

    break;
  case COMPARE_TYPE_and:

    break;
  }
  Build_OP(pred_top, result1, result2, qual_pred, Zero_TN, Zero_TN, ops);
}

/* ====================================================================
 *   Exp_True_False_Preds_For_Block
 *
 * Setup the true_tn and false_tn for a BB. The true_tn is a TN such that
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
   VARIANT branch_variant;
   DEF_KIND kind;

  FmtAssert(FALSE,("Not Implemented"));


   true_tn = NULL;
   false_tn = NULL;
   reusing_tns = FALSE;
   
   branch_variant = CGTARG_Analyze_Branch(br_op, &tn1, &tn2);
   Is_True(branch_variant == V_BR_P_TRUE,("Can't get predicates for block %d",BB_id(bb)));

   /* Try to find the compare op */
   compare_op = TN_Reaching_Value_At_Op(tn1, br_op, &kind, TRUE);
   /* if compare_op is in a different BB, be conservative, can't always reuse
      the predicate, due to other non-satisfying predicate conditions, 804702*/
   if (compare_op && kind == VAL_KNOWN && OP_bb(compare_op) == OP_bb(br_op)) {
     if (!OP_cond_def(compare_op)) {
       //
       // This is the 99% case (maybe the 100% case, given the current
       // generation schemes). The result predicates are 100% defined, so we
       // can safely replace the opcode with the unconditional variant, and
       // then return the two result predicates in the appropriate slots.
       //
       reusing_tns = TRUE;
       OP_Change_Opcode(compare_op,CGTARG_Get_unc_Variant(OP_code(compare_op)));
       Set_OP_cond_def_kind(compare_op,OP_ALWAYS_UNC_DEF);

       true_tn = tn1;
       // Get the other result as the false_tn
       if (OP_result(compare_op,1) != tn1) {
	 false_tn = OP_result(compare_op,1);
       } else {
	 false_tn = OP_result(compare_op,0);
       }
     }
   }

   if (!reusing_tns) {
     OPS ops = OPS_EMPTY;
     true_tn = tn1;
     false_tn = Gen_Predicate_TN();
     Exp_Pred_Set(false_tn, True_TN, 1, &ops);
     Exp_Generic_Pred_Calc(false_tn,True_TN,COMPARE_TYPE_and, true_tn, &ops);
     BB_Insert_Ops(bb,br_op,&ops,TRUE);
     DevWarn("inserting inverse predicate in BB %d",BB_id(bb));
   }
   
   return;
}
