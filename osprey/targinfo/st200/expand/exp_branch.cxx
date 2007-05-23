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
#include "targ_sim.h"

/* ====================================================================
 *   Initialize_Branch_Variants
 * ====================================================================
 */
void
Initialize_Branch_Variants(void)
{
	// nothing to do
}

/* Import from exp_targ.cxx */
extern TN *Expand_Or_Inline_Immediate(TN *src, TYPE_ID mtype, OPS *ops);

/* ====================================================================
 *   Pick_Compare_TOP
 *
 *   Check that compare is of proper form, and return TOP to use for 
 *   the compare.
 *   May modify the variant and src tns.
 *
 *   Modify the is_integer parameter to reflect if the top result is
 *   a branch or an int.
 * ====================================================================
 */
TOP
Pick_Compare_TOP (
  VARIANT *variant, 
  TN **src1, 
  TN **src2, 
  BOOL *is_integer,      // should the result be generated into a int reg
  OPS *ops
)
{
  TOP     cmp = TOP_UNDEFINED;
  TOP     cmp_i = TOP_UNDEFINED;
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

  if (*src1 != NULL && TN_is_constant(*src1)) {
    *src1 = Expand_Or_Inline_Immediate(*src1, MTYPE_I4, ops);
  }
  if (*src2 != NULL && TN_is_constant(*src2)) {
    *src2 = Expand_Or_Inline_Immediate(*src2, MTYPE_I4, ops);
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

//   case V_BR_FGE:
  case V_BR_I4GE:
      cmp = *is_integer ? TOP_cmpge_r_r : TOP_cmpge_r_b;
      mtype = MTYPE_I4;
      break;

//   case V_BR_FGT:
  case V_BR_I4GT:
      cmp = *is_integer ? TOP_cmpgt_r_r : TOP_cmpgt_r_b;
      mtype = MTYPE_I4;
      break;

//   case V_BR_FLE:
  case V_BR_I4LE:
      cmp = *is_integer ? TOP_cmple_r_r : TOP_cmple_r_b;
      mtype = MTYPE_I4;
      break;

//   case V_BR_FLT:
  case V_BR_I4LT:
      cmp = *is_integer ? TOP_cmplt_r_r : TOP_cmplt_r_b;
      mtype = MTYPE_I4;
      break;

//   case V_BR_FEQ:
  case V_BR_I4EQ:
      cmp = *is_integer ? TOP_cmpeq_r_r : TOP_cmpeq_r_b;
      mtype = MTYPE_I4;
      break;

//   case V_BR_FNE:
  case V_BR_I4NE:
      cmp = *is_integer ? TOP_cmpne_r_r : TOP_cmpne_r_b;
      mtype = MTYPE_I4;
      break;

    case V_BR_U4GE:	
    case V_BR_A4GE:
      cmp = *is_integer ? TOP_cmpgeu_r_r : TOP_cmpgeu_r_b;
      mtype = MTYPE_U4;
      break;

    case V_BR_U4LE:
    case V_BR_A4LE:
      cmp = *is_integer ? TOP_cmpleu_r_r : TOP_cmpleu_r_b;
      mtype = MTYPE_U4;
      break;

    case V_BR_U4LT:
    case V_BR_A4LT:
      cmp = *is_integer ? TOP_cmpltu_r_r : TOP_cmpltu_r_b;
      mtype = MTYPE_U4;
      break;

    case V_BR_U4EQ:
    case V_BR_A4EQ:
      cmp = *is_integer ? TOP_cmpeq_r_r : TOP_cmpeq_r_b;
      mtype = MTYPE_U4;
      break;

    case V_BR_U4NE:
    case V_BR_A4NE:
      cmp = *is_integer ? TOP_cmpne_r_r : TOP_cmpne_r_b;
      mtype = MTYPE_U4;
      break;

    case V_BR_U4GT:
    case V_BR_A4GT:
      cmp = *is_integer ? TOP_cmpgtu_r_r : TOP_cmpgtu_r_b;
      mtype = MTYPE_U4;
      break;

// [HK] 20051207 FP SP cmp are available again on ST235
    case V_BR_FGE:
    case V_BR_FGT:
    case V_BR_FLE:
    case V_BR_FLT:
    case V_BR_FEQ:
// #if 0
      if (Enable_Single_Float_Ops) {
	mtype = MTYPE_F4;
// 	*is_integer = TRUE;
	switch(*variant) {
	case V_BR_FGE:
	  cmp = *is_integer ? TOP_cmpgef_n_r : TOP_cmpgef_n_b;
// 	  cmp = TOP_cmpgef;
	  break;
	case V_BR_FGT:
	  cmp = *is_integer ? TOP_cmpgtf_n_r : TOP_cmpgtf_n_b;
// 	  cmp = TOP_cmpgtf;
	  break;
	case V_BR_FLE:
	  cmp = *is_integer ? TOP_cmplef_n_r : TOP_cmplef_n_b;
// 	  cmp = TOP_cmplef;
	  break;
	case V_BR_FLT:
	  cmp = *is_integer ? TOP_cmpltf_n_r : TOP_cmpltf_n_b;
// 	  cmp = TOP_cmpltf;
	  break;
	case V_BR_FEQ:
	  cmp = *is_integer ? TOP_cmpeqf_n_r : TOP_cmpeqf_n_b;
// 	  cmp = TOP_cmpeqf;
	  break;
	default:
	  break;
	}
      }
// #endif
      break;
    case V_BR_FNE:
      /* Undefined on ST200. */
      break;

    case V_BR_DGE:
    case V_BR_DGT:
    case V_BR_DLE:
    case V_BR_DLT:
    case V_BR_DEQ:
// [HK] 20051207 FP SP cmp are available again on ST235
 #if 0
      if (Enable_Double_Float_Ops) {
	mtype = MTYPE_F8;
	*is_integer = TRUE;
	switch(*variant) {
	case V_BR_DGE:
	  cmp = TOP_cmpged;
	  break;
	case V_BR_DGT:
	  cmp = TOP_cmpgtd;
	  break;
	case V_BR_DLE:
	  cmp = TOP_cmpled;
	  break;
	case V_BR_DLT:
	  cmp = TOP_cmpltd;
	  break;
	case V_BR_DEQ:
	  cmp = TOP_cmpeqd;
	  break;
	default:
	  break;
	}
 }
#endif
      break;
    case V_BR_DNE:
      /* Undefined on ST200. */
      break;

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
      break;
  }
  
  //[HK] in case of FP comparison, make the following transformation:
  // f1 cmp f2 <=> int(f1-f2) cmp 0
  // [HK] 20051207 FP SP cmp are available again on ST235, doesn't need to do this anymore
#if 0
  if ( (Enable_Non_IEEE_Ops) && \
       ( (*variant == V_BR_FLT) || (*variant == V_BR_FLE) ||(*variant == V_BR_FGT) || \
	 (*variant == V_BR_FGE) || (*variant == V_BR_FEQ) ||(*variant == V_BR_FNE) ) )
      {
	  TN *src = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer);
	  Build_OP(TOP_subf_n, src, *src1, *src2, ops);
	  *src1 = src;
	  *src2 = Zero_TN;
      }
#endif
  if ( !(Enable_Non_IEEE_Ops) && \
       ( (*variant == V_BR_FLT) || (*variant == V_BR_FLE) ||(*variant == V_BR_FGT) || \
	 (*variant == V_BR_FGE) || (*variant == V_BR_FEQ) ||(*variant == V_BR_FNE) ) )
      cmp = TOP_UNDEFINED;
  
  
  if (cmp != TOP_UNDEFINED) {
    // if src2 is immediate, get the immediate form
    if (*src2 != NULL && TN_has_value(*src2)) {
      TOP cmp_i;
      cmp_i = TOP_opnd_immediate_variant(cmp, 1, TN_value(*src2));
      if (cmp_i != TOP_UNDEFINED) {
        return cmp_i;
      } else {
	*src2 = Expand_Immediate_Into_Register(mtype, *src2, ops);
	return cmp;
      }
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
  BOOL is_integer = FALSE;
  cmp = Pick_Compare_TOP (&cond, &src1, &src2, &is_integer, ops);

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
      if (cmp == TOP_UNDEFINED && (cond == V_BR_FNE || cond == V_BR_DNE)) {
	/* ST200 specific case for handling NE variants. We invert it. */
	cond = cond == V_BR_FNE ? V_BR_FEQ : V_BR_DEQ;
	false_br = !false_br;
	cmp = Pick_Compare_TOP (&cond, &src1, &src2, &is_integer, ops);
      }

      FmtAssert(cmp != TOP_UNDEFINED, 
                                  ("Expand_Branch: unexpected comparison"));


      tmp = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch);
      if (is_integer) {
	TN *tmp_int = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer);
	Build_OP (cmp, tmp_int, src1, src2, ops);
	Expand_Copy(tmp, NULL, tmp_int, ops);
      } else {
	Build_OP (cmp, tmp, src1, src2, ops);
      }
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
  Expand_Copy (RA_TN, NULL, targ_reg, ops);
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
#ifdef TARG_ST  
  TN *targ;
  ANNOTATION *ant;
  LABEL_IDX lab;

  FmtAssert(offset == 0, ("Offset is non null in Exp_Local_Jump"));
  /* first get a label attached to this BB */
  ant = ANNOT_First (BB_annotations(bb), ANNOT_LABEL);
  lab = ANNOT_label(ant);

  targ = Gen_Label_TN (lab, offset);
  Build_OP (TOP_goto, targ, ops);
#else
   FmtAssert(FALSE, ("NYI: Exp_Local_Jump"));
#endif
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
 *   Exp_Return_Interrupt
 * ====================================================================
 */
void 
Exp_Return_Interrupt (
  TN *return_address, 
  OPS *ops
)
{
  /* Code generation for interrupt function is not
   * available on ST200. */
  FmtAssert(FALSE, ("Not implemented: %s", __FUNCTION__));
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
      if ((Gen_PIC_Shared || Gen_PIC_Call_Shared) && Is_Caller_Save_GP) {
	// ST200 PIC ABI, caller-sets-gp model.
	// In this model, function pointers point to descriptors.
	//
	// target points to function descriptor.
	// Load 0(target) into LR and 4(target) into GP.
	OPCODE opc = OPCODE_make_op (OPR_LDID, Pointer_Mtype, Pointer_Mtype);
	Expand_Load (opc, GP_TN, target,
		     Gen_Literal_TN (Pointer_Size, Pointer_Size), ops);
	Set_OP_no_alias (OPS_last (ops));
	Expand_Load (opc, RA_TN, target,
		     Gen_Literal_TN (0, Pointer_Size), ops);
	Set_OP_no_alias (OPS_last (ops));
      }
      else {
	// put target in LR
	Expand_Copy(RA_TN, NULL, target, ops);
      }

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
