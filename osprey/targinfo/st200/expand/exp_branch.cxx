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
  BOOL is_integer,      // should the result be generated into a int reg
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

#if 0

  // This can only be applied to branches

  // check for special cases of first or second arg being zero.
  if (*src2 != NULL && TN_is_zero(*src2)) {
    switch (*variant) {
      case V_BR_U8LT:	
      case V_BR_U4LT:	
	*variant = V_BR_NEVER; break;
      case V_BR_U8GT:
      case V_BR_U4GT:
	*variant = V_BR_ALWAYS; break;
    }
  }
#endif

  // pick tops
  switch (*variant) {

    case V_BR_I4GE:
      cmp_i = is_integer ? TOP_cmpge_i_r : TOP_cmpge_i_b;
      cmp = is_integer ? TOP_cmpge_r_r : TOP_cmpge_r_b;
      mtype = MTYPE_I4;
      break;

    case V_BR_I4GT:
      cmp_i = is_integer ? TOP_cmpgt_i_r : TOP_cmpgt_i_b;
      cmp = is_integer ? TOP_cmpgt_r_r : TOP_cmpgt_r_b;
      mtype = MTYPE_I4;
      break;

    case V_BR_I4LE:
      cmp_i = is_integer ? TOP_cmple_i_r : TOP_cmple_i_b;
      cmp = is_integer ? TOP_cmple_r_r : TOP_cmple_r_b;
      mtype = MTYPE_I4;
      break;

    case V_BR_I4LT:
      cmp_i = is_integer ? TOP_cmplt_i_r : TOP_cmplt_i_b;
      cmp = is_integer ? TOP_cmplt_r_r : TOP_cmplt_r_b;
      mtype = MTYPE_I4;
      break;

    case V_BR_I4EQ:
      cmp_i = is_integer ? TOP_cmpeq_i_r : TOP_cmpeq_i_b;
      cmp = is_integer ? TOP_cmpeq_r_r : TOP_cmpeq_r_b;
      mtype = MTYPE_I4;
      break;

    case V_BR_I4NE:
      cmp_i = is_integer ? TOP_cmpne_i_r : TOP_cmpne_i_b;
      cmp = is_integer ? TOP_cmpne_r_r : TOP_cmpne_r_b;
      mtype = MTYPE_I4;
      break;

    case V_BR_U4GE:	
    case V_BR_A4GE:
      cmp_i = is_integer ? TOP_cmpgeu_i_r : TOP_cmpgeu_i_b;
      cmp = is_integer ? TOP_cmpgeu_r_r : TOP_cmpgeu_r_b;
      mtype = MTYPE_U4;
      break;

    case V_BR_U4LE:
    case V_BR_A4LE:
      cmp_i = is_integer ? TOP_cmpleu_i_r : TOP_cmpleu_i_b;
      cmp = is_integer ? TOP_cmpleu_r_r : TOP_cmpleu_r_b;
      mtype = MTYPE_U4;
      break;

    case V_BR_U4LT:
    case V_BR_A4LT:
      cmp_i = is_integer ? TOP_cmpltu_i_r : TOP_cmpltu_i_b;
      cmp = is_integer ? TOP_cmpltu_r_r : TOP_cmpltu_r_b;
      mtype = MTYPE_U4;
      break;

    case V_BR_U4EQ:
    case V_BR_A4EQ:
      cmp_i = is_integer ? TOP_cmpeq_i_r : TOP_cmpeq_i_b;
      cmp = is_integer ? TOP_cmpeq_r_r : TOP_cmpeq_r_b;
      mtype = MTYPE_U4;
      break;

    case V_BR_U4NE:
    case V_BR_A4NE:
      cmp_i = is_integer ? TOP_cmpne_i_r : TOP_cmpne_i_b;
      cmp = is_integer ? TOP_cmpne_r_r : TOP_cmpne_r_b;
      mtype = MTYPE_U4;
      break;

    case V_BR_U4GT:
    case V_BR_A4GT:
      cmp_i = is_integer ? TOP_cmpgtu_i_r : TOP_cmpgtu_i_b;
      cmp = is_integer ? TOP_cmpgtu_r_r : TOP_cmpgtu_r_b;
      mtype = MTYPE_U4;
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
      const ISA_OPERAND_VALTYP *otype = ISA_OPERAND_INFO_Operand(oinfo, 1);
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

  // If the branch is conditional FALSEBR, we must invert the condition 
  // because ST200 branches are active on TRUE.
  //  if (V_br_condition(variant) != V_BR_ALWAYS &&
  //      V_br_condition(variant) != V_BR_NEVER &&
  //      V_false_br(variant) ) 
  //    variant = Negate_BR_Variant(variant);

  BOOL  false_br = V_false_br(variant);
  VARIANT cond = V_br_condition(variant);

  /* Trace if required: */
  if (Trace_Exp) {
    fprintf ( TFile, "<cgexp> Translating %s branch:\n",
                                    (false_br ? "false" : "true") );
  }

  /* default branch instruction on: */
  if (cond == V_BR_NONE) {
    FmtAssert(FALSE,("Expand_Branch: default branch ??"));
    return;
  }

  FmtAssert( cond <= V_BR_LAST, ("unexpected variant in Expand_Branch"));
  FmtAssert( cond != V_BR_NONE, ("BR_NONE variant in Expand_Branch"));

  if (Trace_Exp) {
    fprintf (TFile, "<cgexp> branch cond = %lld\n", cond);
  }

  // check for special case of second arg being zero.
  if (src2 != NULL && TN_is_zero(src2)) {
    switch (cond) {
      case V_BR_U8LT:	
      case V_BR_U4LT:	
	cond = V_BR_NEVER; break;
      case V_BR_U8GE:
      case V_BR_U4GE:
	cond = V_BR_ALWAYS; break;
    }
  }

  // compare should calculate a branch reg result
  cmp = Pick_Compare_TOP (&cond, &src1, &src2, FALSE, ops);

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
      // Unconditional branch for ALWAYS/!false_br and NEVER/false_br
      Build_OP (TOP_goto, targ, ops);
    }
    break;

  case V_BR_P_TRUE:
    Is_True(cmp == TOP_UNDEFINED, ("unexpected compare op for V_BR_P_TRUE"));
    Build_OP (false_br ? TOP_brf : TOP_br, src1, targ, ops);
    break;

  default:
    {
      // conditional branch
      FmtAssert(cmp != TOP_UNDEFINED, 
                                  ("Expand_Branch: unexpected comparison"));
      tmp = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch);
      //
      // The src2 may be an immediate TN. Clean up the sign-extension
      // bits if necessary. This has happened when we end up with a 1 in
      // an upper bit of a U4. WHIRL sign-extends the value to 64-bits
      // but then 0xffffffff80000000 is out of unsigned range. This won't
      // matter but if the TOP code needs an unsigned operand !
      //
      if (TN_has_value(src2) && TN_size(src2) < 8) {
	//
	// If TOP code needs an unsigned value, mask the sign-extension
	//
	const ISA_OPERAND_INFO *oinfo = ISA_OPERAND_Info(cmp);
	const ISA_OPERAND_VALTYP *vtype = ISA_OPERAND_INFO_Operand(oinfo, 1);
	if (!ISA_OPERAND_VALTYP_Is_Signed(vtype)) {
	  //ISA_LIT_CLASS lc = ISA_OPERAND_VALTYP_Literal_Class(vtype);
	  //if (ISA_LC_Is_Signed(lc)) {
	  INT64 imm = TN_value(src2);
	  src2 = Gen_Literal_TN((imm & 0x00000000ffffffff), TN_size(src2));
	}
      }
      Build_OP (cmp, tmp, src1, src2, ops);
      FmtAssert(TN_is_label(targ), ("Expand_Branch: expected a label"));
      /*
       * For now I just trust it that it fits into 23 bits
       */
      if (false_br)
	Build_OP (TOP_brf, tmp, targ, ops);
      else
	Build_OP (TOP_br, tmp, targ, ops);
    }
    break;
  }

  return;
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
  // put target in RA_TN register
  Build_OP (TOP_mov_r, RA_TN, targ_reg, ops);
  Build_OP (TOP_igoto, RA_TN, ops);
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
  Build_OP (TOP_return, RA_TN, ops);
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
      Build_OP (TOP_call, RA_TN, target, ops);
      break;

    case OPR_ICALL:
      // put target in LR
      Build_OP(TOP_mov_r, RA_TN, target, ops);

      if (Trace_Exp) {
	fprintf(TFile,"exp_call ICALL into ");
	Print_OP (OPS_last(ops));
      }

      Build_OP(TOP_icall, RA_TN, RA_TN, ops);
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
