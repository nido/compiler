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


/* EXP routines for expanding branches */

#include "defs.h"
#include "erglob.h"
#include "ercg.h"
#include "tracing.h"
#include "config.h"
#include "config_TARG.h"
#include "topcode.h"
#include "tn.h"
#include "targ_isa_lits.h"
#include "cg_flags.h"
#include "op.h"
#include "cgexp.h"

/* ====================================================================
 *   Initialize_Branch_Variants
 * ====================================================================
 */
void
Initialize_Branch_Variants(void)
{
	// nothing to do
}

/* ====================================================================
 *   Pick_Compare_TOP
 *
 *   Check that compare is of proper form, and return TOP to use for 
 *   the compare.
 *   May modify the variant and src tns.
 * ====================================================================
 */
TOP
Pick_Compare_TOP (
  VARIANT *variant, 
  TN **src1, 
  TN **src2, 
  OPS *ops
)
{
  TOP     cmp;
  TOP     cmp_i;
  TYPE_ID mtype;

  /* 
   * If first operand is an immediate value, swap it with the
   * second:
   */
  if (*src1 != NULL && TN_has_value(*src1)) {
    // swap operands and change variant
    TN *tmp = *src1;
    *src1 = *src2;
    *src2 = tmp;
    *variant = Invert_BR_Variant(*variant);
  }

  // check for special cases of first or second arg being zero.
  if (*src2 != NULL && TN_is_zero(*src2)) {
    switch (*variant) {
      case V_BR_U8LT:	
      case V_BR_U5LT:
      case V_BR_U4LT:	
	*variant = V_BR_NEVER; break;
      case V_BR_U8GT:
      case V_BR_U5GT:
      case V_BR_U4GT:
	*variant = V_BR_ALWAYS; break;
    }
  }

  // pick tops
  switch (*variant) {

    case V_BR_I4GE:
      cmp_i = TOP_GP32_GEW_GT_BR_DR_U8;
      cmp = TOP_GP32_GEW_GT_BR_DR_DR;
      mtype = MTYPE_I4;
      break;

    case V_BR_I4GT:
      cmp_i = TOP_GP32_GTW_GT_BR_DR_U8;
      cmp = TOP_GP32_GTW_GT_BR_DR_DR;
      mtype = MTYPE_I4;
      break;

    case V_BR_I4LE:
      cmp_i = TOP_GP32_LEW_GT_BR_DR_U8;
      cmp = TOP_GP32_LEW_GT_BR_DR_DR;
      mtype = MTYPE_I4;
      break;

    case V_BR_I4LT:
      cmp_i = TOP_GP32_LTW_GT_BR_DR_U8;
      cmp = TOP_GP32_LTW_GT_BR_DR_DR;
      mtype = MTYPE_I4;
      break;

    case V_BR_I4EQ:
      cmp_i = TOP_GP32_EQW_GT_BR_DR_U8;
      cmp = TOP_GP32_EQW_GT_BR_DR_DR;
      mtype = MTYPE_I4;
      break;

    case V_BR_I4NE:
      cmp_i = TOP_GP32_NEW_GT_BR_DR_U8;
      cmp = TOP_GP32_NEW_GT_BR_DR_DR;
      mtype = MTYPE_I4;
      break;

    case V_BR_U4GE:	
      cmp_i = TOP_GP32_GEUW_GT_BR_DR_U8;
      cmp = TOP_GP32_GEUW_GT_BR_DR_DR;
      mtype = MTYPE_U4;
      break;

    case V_BR_U4LE:
      cmp_i = TOP_GP32_LEUW_GT_BR_DR_U8;
      cmp = TOP_GP32_LEUW_GT_BR_DR_DR;
      mtype = MTYPE_U4;
      break;

    case V_BR_U4LT:
      cmp_i = TOP_GP32_LTUW_GT_BR_DR_U8;
      cmp = TOP_GP32_LTUW_GT_BR_DR_DR;
      mtype = MTYPE_U4;
      break;

    case V_BR_U4EQ:
      cmp_i = TOP_GP32_EQUW_GT_BR_DR_U8;
      cmp = TOP_GP32_EQUW_GT_BR_DR_DR;
      mtype = MTYPE_U4;
      break;

    case V_BR_U4NE:
      cmp_i = TOP_GP32_NEUW_GT_BR_DR_U8;
      cmp = TOP_GP32_NEUW_GT_BR_DR_DR;
      mtype = MTYPE_U4;
      break;

    case V_BR_I5GE:
      cmp_i = TOP_GP32_GEE_GT_BR_DR_U8;
      cmp = TOP_GP32_GEE_GT_BR_DR_DR;
      mtype = MTYPE_I5;
      break;

    case V_BR_U5GE:	
      cmp_i = TOP_GP32_GEUE_GT_BR_DR_U8;
      cmp = TOP_GP32_GEUE_GT_BR_DR_DR;
      mtype = MTYPE_U5;
      break;

    case V_BR_A4GE:
      cmp_i = TOP_UNDEFINED;
      cmp = TOP_GP32_GEA_GT_BR_AR_AR;
      mtype = MTYPE_A4;
      break;

    case V_BR_U4GT:
      cmp_i = TOP_GP32_GTUW_GT_BR_DR_U8;
      cmp = TOP_GP32_GTUW_GT_BR_DR_DR;
      mtype = MTYPE_U4;
      break;

    case V_BR_U5GT:
      cmp_i = TOP_GP32_GTUE_GT_BR_DR_U8;
      cmp = TOP_GP32_GTUE_GT_BR_DR_DR;
      mtype = MTYPE_U5;
      break;

    case V_BR_I5GT:
      cmp_i = TOP_GP32_GTE_GT_BR_DR_U8;
      cmp = TOP_GP32_GTE_GT_BR_DR_DR;
      mtype = MTYPE_I5;
      break;

    case V_BR_A4GT:
      cmp_i = TOP_UNDEFINED;
      cmp = TOP_GP32_GTA_GT_BR_AR_AR;
      mtype = MTYPE_A4;
      break;

    case V_BR_I5LE:
      cmp_i = TOP_GP32_LEE_GT_BR_DR_U8;
      cmp = TOP_GP32_LEE_GT_BR_DR_DR;
      mtype = MTYPE_I5;
      break;

    case V_BR_U5LE:
      cmp_i = TOP_GP32_LEUE_GT_BR_DR_U8;
      cmp = TOP_GP32_LEUE_GT_BR_DR_DR;
      mtype = MTYPE_U5;
      break;

    case V_BR_A4LE:
      cmp_i = TOP_UNDEFINED;
      cmp = TOP_GP32_LEA_GT_BR_AR_AR;
      mtype = MTYPE_A4;
      break;

    case V_BR_I5LT:
      cmp_i = TOP_GP32_LTE_GT_BR_DR_U8;
      cmp = TOP_GP32_LTE_GT_BR_DR_DR;
      mtype = MTYPE_I5;
      break;

    case V_BR_U5LT:
      cmp_i = TOP_GP32_LTUE_GT_BR_DR_U8;
      cmp = TOP_GP32_LTUE_GT_BR_DR_DR;
      mtype = MTYPE_U5;
      break;

    case V_BR_A4LT:
      cmp_i = TOP_UNDEFINED;
      cmp = TOP_GP32_LTA_GT_BR_AR_AR;
      mtype = MTYPE_A4;
      break;

    case V_BR_I5EQ:
      cmp_i = TOP_GP32_EQE_GT_BR_DR_U8;
      cmp = TOP_GP32_EQE_GT_BR_DR_DR;
      mtype = MTYPE_I5;
      break;

    case V_BR_U5EQ:
      cmp_i = TOP_GP32_EQUE_GT_BR_DR_U8;
      cmp = TOP_GP32_EQUE_GT_BR_DR_DR;
      mtype = MTYPE_U5;
      break;

    case V_BR_A4EQ:
      cmp_i = TOP_UNDEFINED;
      cmp = TOP_GP32_EQA_GT_BR_AR_AR;
      mtype = MTYPE_A4;
      break;

    case V_BR_I5NE:
      cmp_i = TOP_GP32_NEE_GT_BR_DR_U8;
      cmp = TOP_GP32_NEE_GT_BR_DR_DR;
      mtype = MTYPE_I5;
      break;

    case V_BR_U5NE:
      cmp_i = TOP_GP32_NEUE_GT_BR_DR_U8;
      cmp = TOP_GP32_NEUE_GT_BR_DR_DR;
      mtype = MTYPE_U5;
      break;

    case V_BR_A4NE:
      cmp_i = TOP_UNDEFINED;
      cmp = TOP_GP32_NEA_GT_BR_AR_AR;
      mtype = MTYPE_A4;
      break;

    case V_BR_FEQ:	
    case V_BR_DEQ:	
    case V_BR_FLT:	
    case V_BR_DLT:	
    case V_BR_FLE:	
    case V_BR_DLE:	
    case V_BR_FNE:	
    case V_BR_DNE:	
    case V_BR_FGT:	
    case V_BR_DGT:	
    case V_BR_FGE:	
    case V_BR_DGE:	

    case V_BR_I8GE:
    case V_BR_I8GT:	
    case V_BR_I8LE:	
    case V_BR_I8LT:	
    case V_BR_U8GE:	
    case V_BR_U8GT:	
    case V_BR_U8LE:	
    case V_BR_U8LT:	
    case V_BR_I8EQ:	
    case V_BR_U8EQ:	
    case V_BR_I8NE:	
    case V_BR_U8NE:	

    default:	
      cmp_i = TOP_UNDEFINED; 
      cmp = TOP_UNDEFINED; 
      mtype = MTYPE_UNKNOWN;
      break;
  }

  // if src2 is immed and fits, use immed form of top
  if (*src2 != NULL && TN_has_value(*src2)) {

    if (cmp_i != TOP_UNDEFINED) {
      const ISA_OPERAND_INFO *oinfo = ISA_OPERAND_Info(cmp_i);
      const ISA_OPERAND_VALTYP *otype = ISA_OPERAND_INFO_Operand(oinfo, 2);
      ISA_LIT_CLASS lc = ISA_OPERAND_VALTYP_Literal_Class(otype);
      if (ISA_LC_Value_In_Class(TN_value(*src2), lc))
        return cmp_i;
    }

    // cmp of a value that does not fit in a register:
    if (cmp != TOP_UNDEFINED) {
      // could not use the immediate form 
      *src2 = Expand_Immediate_Into_Register(mtype, *src2, ops);
    }

  }

  return cmp;
}

/* ====================================================================
 *   Expand_Branch
 * ====================================================================
 */
void
Expand_Branch ( 
  TN *targ, 
  TN *src1, 
  TN *src2, 
  VARIANT variant, 
  OPS *ops
)
{
  TOP   cmp;
  TN   *tmp;

  // If the branch is conditional TRUEBR, I must invert the condition 
  // because ST100 branches are active on FALSE.
  if (V_br_condition(variant) != V_BR_ALWAYS &&
      V_br_condition(variant) != V_BR_NEVER &&
      !V_false_br(variant) ) 
    variant = Negate_BR_Variant(variant);

  BOOL  false_br = V_false_br(variant);
  VARIANT cond = V_br_condition(variant);

  /* Trace if required: */
  if (Trace_Exp) {
    fprintf ( TFile, "<cgexp> Translating %s branch:\n",
                                    (false_br ? "false" : "true") );
  }

  /* default branch instruction on st100: */
  if (cond == V_BR_NONE) {
    tmp = Build_RCLASS_TN(ISA_REGISTER_CLASS_guard);
    Exp_Pred_Copy(tmp, True_TN, src1, ops);
    Build_OP (TOP_GP32_GOTO_GF_S21, tmp, targ, ops);
    return;
  }

  FmtAssert( cond <= V_BR_LAST, ("unexpected variant in Expand_Branch"));
  FmtAssert( cond != V_BR_NONE, ("BR_NONE variant in Expand_Branch"));

  cmp = Pick_Compare_TOP (&cond, &src1, &src2, ops);

  if (Trace_Exp) {
    fprintf (TFile, "<cgexp> transformed branch cond = %lld\n", cond);
  }

  switch (cond) {
  case V_BR_ALWAYS:
  case V_BR_NEVER:
    Is_True(cmp == TOP_UNDEFINED, 
	    ("unexpected compare op for %s", BR_Variant_Name(cond)));
    if ((cond == V_BR_ALWAYS && !false_br) ||
	(cond == V_BR_NEVER && false_br)) {
#if 0
      // Arthur: I must use the above because it does not seem to 
      //         work with the gcc 92.5 on Linux !
      //
    if ((cond == V_BR_ALWAYS) ^ false_br) {
#endif
      // Unconditional branch for ALWAYS/!false_br and NEVER/false_br
      Build_OP (TOP_GP32_GOTO_S25, targ, ops);
    }
    break;

#if 0
  case V_BR_PEQ:
  case V_BR_PNE:
    {
      Is_True(cmp == TOP_UNDEFINED, 
	      ("unexpected compare op for V_BR_PEQ/V_BR_PNE"));
      TN *p1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_guard);
      TN *p2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_guard);
      TN *tn = Build_TN_Of_Mtype (MTYPE_I8);
      TOP action = (cond == V_BR_PEQ) ? TOP_GP32_NEW_GT_BR_DR_DR : 
                                                TOP_GP32_EQW_GT_BR_DR_DR;

      Build_OP (TOP_GP32_MAKEA_GT_AR_S16, tn, True_TN, 
                                              Gen_Literal_TN(1, 8), ops);
      Build_OP (TOP_GP32_XOR_GT_DR_DR_U8, tn, src1, 
                                          Gen_Literal_TN(1, 8), tn, ops);
      Build_OP (TOP_GP32_XOR_GT_DR_DR_U8, tn, src2, 
                                          Gen_Literal_TN(1, 8), tn, ops);

      Build_OP (action, p1, p2, True_TN, tn, Zero_TN, ops);

      Build_OP (TOP_noop, 
	        false_br ? p2 : p1,
	        targ, ops);
    }
    break;
#endif

  case V_BR_P_TRUE:
    Is_True(cmp == TOP_UNDEFINED, ("unexpected compare op for V_BR_P_TRUE"));

    if (! false_br) {
      // TRUEBR is not directly supported, use src2 as a compliment
      // because truebr must have been reversed
      //
      tmp = Build_RCLASS_TN(ISA_REGISTER_CLASS_guard);
      Exp_Pred_Complement(tmp, True_TN, src1, ops);
      src1 = tmp;
#if 0
      if (!false_br) {
        DevWarn("inverted V_BR_P_TRUE");
        tmp = Build_RCLASS_TN(ISA_REGISTER_CLASS_guard);
        Exp_Pred_Complement(tmp, True_TN, src1, ops);
        src1 = tmp;
      }
#endif
    }

    // else false_br, fall through...
    // false branch on a predicate, directly supported on the ST100
    // use src1
    Build_OP (TOP_GP32_GOTO_GF_S21, src1, targ, ops);
    break;

  default:
    {
      // conditional branch
      FmtAssert(cmp != TOP_UNDEFINED, 
                                  ("Expand_Branch: unexpected comparison"));
      tmp = Build_RCLASS_TN (ISA_REGISTER_CLASS_guard);
      Build_OP (cmp, tmp, True_TN, src1, src2, ops);
      FmtAssert(TN_is_label(targ),
		                 ("Expand_Branch: expected a label"));
      /*
       * For now I just trust it that it fits into 25 bits
       */
      Build_OP (TOP_GP32_GOTO_GF_S21, tmp, targ, ops);
    }
    break;
  }
}

/* ====================================================================
 *   Exp_Indirect_Branch
 * ====================================================================
 */
void 
Exp_Indirect_Branch (
  TN *targ_reg, 
  OPS *ops
)
{
  FmtAssert(FALSE,("Not Implemented"));
  return;
}

/* ====================================================================
 *   Exp_Local_Branch
 * ====================================================================
 */
void 
Exp_Local_Jump (
  BB *bb, 
  INT64 offset, 
  OPS *ops
)
{
  FmtAssert(FALSE, ("NYI: Exp_Local_Jump"));
}

/* ====================================================================
 *   Exp_Return
 * ====================================================================
 */
void 
Exp_Return (
  TN *return_address, 
  OPS *ops
)
{
  Build_OP (TOP_GP32_RTS_GT, 
	    True_TN, 
	    Build_Dedicated_TN(ISA_REGISTER_CLASS_au, 
			       REGISTER_ra, Pointer_Size),
	    ops);
  return;
}

/* ====================================================================
 *   Exp_Call
 *
 *   There are three types of calls:
 *     OPR_CALL    -- direct call;
 *     OPR_ICALL   -- indirect call;
 *     OPR_PICCALL -- pic call.
 * ====================================================================
 */
void 
Exp_Call (
  OPERATOR opr, 
  TN *return_address, 
  TN *target, 
  OPS *ops
)
{
  TOP top;
  TN *br_tmp;

  if (Trace_Exp) {
    fprintf(TFile, "exp_call %s :- ", OPERATOR_name(opr));
    Print_TN (target, FALSE);
    fprintf(TFile, "\n");
  }

  switch (opr) {
    case OPR_CALL:
      Build_OP (TOP_GP32_CALL_S25, target, ops);
      break;

    case OPR_ICALL:

      FmtAssert(TN_is_register(target) && 
                  TN_register_class(target) == ISA_REGISTER_CLASS_au,
		          ("Exp_Call: target TN is not a AU register"));

      // put target in P3 and do LINK
      Build_OP(TOP_GP32_MOVEA_GT_AR_AR, Link_TN, True_TN, target, ops);

      fprintf(TFile,"exp_call into ");
      Print_OP (OPS_last(ops));

      Build_OP(TOP_GP32_LINK_GT, True_TN, Link_TN, ops);
      break;

    case OPR_PICCALL:

      FmtAssert(FALSE, ("Not Implemented"));
      top = TOP_noop;
      break;

    default:
      FmtAssert(FALSE, ("unexpected opr in Exp_Call"));
      /*NOTREACHED*/
  }

  if (Trace_Exp) {
    fprintf(TFile,"exp_call into ");
    Print_OP (OPS_last(ops));
  }

  return;
}
