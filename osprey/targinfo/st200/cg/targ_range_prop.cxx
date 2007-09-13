/*
  Copyright (C) 2006, STMicroelectronics, All Rights Reserved.

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
*/

/* ====================================================================
 * ====================================================================
 *
 * Module: targ_range_prop.cxx
 *
 * Description:
 *
 * Target-specific code for range propagation.
 *
 * ====================================================================
 */

#include "defs.h"
#include "cgir.h"
#include "cgexp.h"
#include "range_prop.h"
#include "targ_cg_private.h"
#include "cg_ssa.h"
#include "ebo_info.h"
#include "tracing.h"

#define SWAP_OP(op1, op2) do { OP *tmp = op1; op1 = op2; op2 = tmp; } while (0)
#define SWAP_TN(tn1, tn2) do { TN *tmp = tn1; tn1 = tn2; tn2 = tmp; } while (0)

static OP *single_use (const RangeAnalysis &range_analysis,
		       TN *tn)
{
  OP_LIST *uses = range_analysis.Uses (tn);
  if (uses && ! OP_LIST_rest (uses))
    return OP_LIST_first (uses);
  else
    return NULL;
}

static BOOL no_uses_p (const RangeAnalysis &range_analysis, TN *tn)
{
  return TN_is_ssa_var (tn) && range_analysis.Uses (tn) == NULL;
}

static BOOL
match_add_mul_sequence (const RangeAnalysis &range_analysis,
			const OP *l1_op,
			OPS *ops)
{
  if (OP_code (l1_op) != TOP_add_r_r_r) return FALSE;

  OP *l2_op0 = TN_ssa_def (OP_Opnd1 (l1_op));
  OP *l2_op1 = TN_ssa_def (OP_Opnd2 (l1_op));

  if (! l2_op0 || ! l2_op1) return FALSE;

  if (OP_code (l2_op0) == TOP_mulhs_r_r_r)
    SWAP_OP (l2_op0, l2_op1);

  if (OP_code (l2_op0) != TOP_mullu_r_r_r
      || OP_code (l2_op1) != TOP_mulhs_r_r_r)
    return FALSE;

  TN *opnd1 = OP_Opnd1 (l2_op0);
  TN *opnd2 = OP_Opnd2 (l2_op1);

  if (! TN_is_ssa_var (opnd1)
      || ! TN_is_ssa_var (opnd2)
      || ! range_analysis.Equivalent (opnd1, OP_Opnd1 (l2_op1))
      || ! range_analysis.Equivalent (opnd2, OP_Opnd2 (l2_op1)))
    return FALSE;

  // We have a 32x32=>32 multiply of opnd1 and opnd2.
  // See if it can be reduced.
  LRange_p v1 = range_analysis.Get_Value (opnd1);
  LRange_p v2 = range_analysis.Get_Value (opnd2);
  INT bits1 = v1->bits ();
  INT bits2 = v2->bits ();
  BOOL swap_operands = FALSE;
  TOP new_op = targ_cg_find_mul_with_properties (opnd1, opnd2,
					       0, bits1, TRUE,
					       0, bits2, TRUE,
					       0, swap_operands);

  if (new_op == TOP_UNDEFINED) return FALSE;

  if (swap_operands)
    SWAP_TN (opnd1, opnd2);

  Build_OP (new_op, OP_result (l1_op, 0), opnd1, opnd2, ops);
  return TRUE;
}
 
static BOOL
is_mul32_by_sign (const RangeAnalysis &range_analysis,
		  OP *op, const TN *tn1, const TN *tn2)
{
  // Return TRUE if op is a 32-bit multiply of tn1 by
  // the sign of tn2, FALSE if not or unknown.
  if ((OP_code (op) != TOP_mul32_r_r_r
       && OP_code (op) != TOP_mull_r_r_r)
      || ! range_analysis.Equivalent (OP_Opnd1 (op), tn1))
    return FALSE;

  const TN *equiv_tn = range_analysis.Root_Equivalent (OP_Opnd2 (op));
    
  OP *op2 = TN_ssa_def (equiv_tn);
    
  if (! op2
      || (OP_code (op2) != TOP_shr_i_r_r && OP_code (op2) != TOP_shr_ii_r_r))
    return FALSE;

  TN *shifted_val = OP_Opnd1 (op2);
  TN *shift_count = OP_Opnd2 (op2);
  if (range_analysis.Equivalent (shifted_val, tn2)
      && TN_has_value (shift_count)
      && TN_value (shift_count) == 31)
    {
      return TRUE;
    }
  return FALSE;
}
 
static BOOL
match_addcg (const RangeAnalysis &range_analysis,
		 OP *op,
		 OPS *ops)
{
  // Match cases when op is an addcg with carry-in zero
  // and no use of carry-out, and then replace it by 
  // a simple add
  // Also catch the case where first two operands of the addcg 
  // are zero, and replace it by a convib
  if (OP_code (op) != TOP_addcg_b_r_r_b_r)
    return FALSE;

  TN *result_b = OP_result (op, 1);
  TN *result_r = OP_result (op, 0);
  TN *opnd_b = OP_opnd(op, 2);
  TN *opnd_r1 = OP_opnd(op, 0);
  TN *opnd_r2 = OP_opnd(op, 1);

  FmtAssert (TN_size (result_r) == 4,
	       ("addcg not of size 4"));
  
  BOOL is_signed = OP_result_is_signed(op, 0);
  TYPE_ID mtype = is_signed ? MTYPE_I4 : MTYPE_U4;
  LRange_pc rb_in = range_analysis.Get_Value (opnd_b);
  LRange_pc r1_in = range_analysis.Get_Value (opnd_r1);
  LRange_pc r2_in = range_analysis.Get_Value (opnd_r2);
  if (no_uses_p (range_analysis, result_b)){
    if (rb_in->isZero ()){
      // addcg=>add transform
      Expand_Add(result_r, opnd_r1, opnd_r2, mtype, ops);
      return TRUE;
    } else if (r1_in->isZero () && r2_in->isZero ()) {
      // addcg=>convib transform
      Expand_Bool_To_Int (result_r, opnd_b, MTYPE_I4, ops);
      return TRUE;
    }      
  }
  return FALSE;
}

static BOOL
match_mul64h_sequence (const RangeAnalysis &range_analysis,
		       const OP *l1_op,
		       OPS *ops)
{
  // Match
  //   mul64hu (a, b) + mul32 (a, b>>31) + mul32 (a>>31, b)
  // and transform to
  //   mul64h (a, b).
  //
  // Note: this is just an SSA-based peephole: it does not
  // require any range information.

  if (OP_code (l1_op) != TOP_add_r_r_r) return FALSE;

  TN *result = OP_result (l1_op, 0);
  OP *l2_op0 = TN_ssa_def (OP_Opnd1 (l1_op));
  OP *l2_op1 = TN_ssa_def (OP_Opnd2 (l1_op));

  if (! l2_op0 || ! l2_op1) return FALSE;

  if (OP_code (l2_op1) == TOP_add_r_r_r)
    SWAP_OP (l2_op0, l2_op1);

  if (OP_code (l2_op0) != TOP_add_r_r_r) return FALSE;

  OP *l3_op0 = TN_ssa_def (OP_Opnd1 (l2_op0));
  OP *l3_op1 = TN_ssa_def (OP_Opnd2 (l2_op0));
  OP *l3_op2 = l2_op1;

  if (! l3_op0 || ! l3_op1) return FALSE;

  if (OP_code (l3_op1) == TOP_mul64hu_r_r_r)
    SWAP_OP (l3_op0, l3_op1);
  else if (OP_code (l3_op2) == TOP_mul64hu_r_r_r)
    SWAP_OP (l3_op0, l3_op2);

  if (OP_code (l3_op0) != TOP_mul64hu_r_r_r) return FALSE;

  TN *opnd1 = OP_Opnd1 (l3_op0);
  TN *opnd2 = OP_Opnd2 (l3_op0);

  if (!((is_mul32_by_sign (range_analysis, l3_op1, opnd1, opnd2)
	 && is_mul32_by_sign (range_analysis, l3_op2, opnd2, opnd1))
	|| (is_mul32_by_sign (range_analysis, l3_op1, opnd2, opnd1)
	    && is_mul32_by_sign (range_analysis, l3_op2, opnd1, opnd2))))
    return FALSE;

  Build_OP (TOP_mul64h_r_r_r, OP_result (l1_op, 0),
	    opnd1, opnd2, ops);
  return TRUE;
}
      
static BOOL
match_convib_sequence (const RangeAnalysis &range_analysis,
		       const OP *op,
		       OPS *ops)
{
  // Match
  //   r1 = cmp x, y
  //   ...
  //   b1 = convib r1  (current op, single use of r1)
  // and transform to
  //   r1 = cmp x, y
  //   ...
  //   b1 = cmp' x, y
  //
  // This hopefully makes the original cmp defining r1 dead.
  // 
  // I also hope that if cmp' is loop-invariant, it will
  // be moved out of a loop later.
  //
  // Note: this is just an SSA-based peephole: it does not
  // require any range information.
  // Worry: this increases register pressure but we
  // do not know if this is beneficial or not.

  if (OP_code (op) != TOP_convib_r_b) return FALSE;

  TN *result = OP_result (op, 0);
  TN *opnd = OP_opnd (op, 0);

  OP *l1_op = TN_ssa_def (opnd);

  // The single use condition is to mitigate register pressure
  // increase, it is not necessary to enable the transformation.
  if (! l1_op || ! OP_icmp(l1_op)
      || ! single_use (range_analysis, opnd)) return FALSE;

  TOP new_opcode = TOP_result_register_variant(OP_code (l1_op), 0, ISA_REGISTER_CLASS_branch);
  TN *opnd1 = OP_Opnd1 (l1_op);
  TN *opnd2 = OP_Opnd2 (l1_op);

  if (new_opcode == TOP_UNDEFINED
      || ! (TN_has_value (opnd1) || TN_is_ssa_var (opnd1)
	    || TN_is_zero_reg (opnd1))
      || ! (TN_has_value (opnd2) || TN_is_ssa_var (opnd2)
	    || TN_is_zero_reg (opnd2)))
    return FALSE;

  Build_OP (new_opcode, result, opnd1, opnd2, ops);

  return TRUE;
}
    
static BOOL
match_multiply (const RangeAnalysis &range_analysis,
		const OP *op,
		OPS *ops)
{
  const struct MulProperties *mul_properties = targ_cg_mul_properties (op);

  if (mul_properties) {

    TN *opnd1 = OP_Opnd1(op);
    TN *opnd2 = OP_Opnd2(op);
    TN *result = OP_result (op, 0);
    
    // Propagate range to multiplies.

    INT opnd1_shift = mul_properties->opnd1_shift;
    INT opnd1_width = mul_properties->opnd1_width;
    BOOL opnd1_signed = mul_properties->opnd1_signed;
    INT opnd2_shift = mul_properties->opnd2_shift;
    INT opnd2_width = mul_properties->opnd2_width;
    BOOL opnd2_signed = mul_properties->opnd2_signed;
    INT result_shift = mul_properties->result_shift;
    BOOL swap_operands = FALSE;

    LRange_pc r1 = range_analysis.Get_Value (opnd1);
    LRange_pc r2 = range_analysis.Get_Value (opnd2);
    if (! opnd1_signed) r1 = MakeUnsigned (r1, TN_bitwidth (opnd1));
    if (! opnd2_signed)  r2 = MakeUnsigned (r2, TN_bitwidth (opnd2));
    INT bits1 = r1->bits ();
    INT bits2 = r2->bits ();

    if (opnd1_signed && r1->isNonNegative () && bits1 < opnd1_width) {
      opnd1_signed = FALSE;
    }
    if (opnd2_signed && r2->isNonNegative () && bits2 < opnd2_width) {
      opnd2_signed = FALSE;
    }

    opnd1_width = MIN (opnd1_width, bits1);
    opnd2_width = MIN (opnd2_width, bits2);
    
    TOP new_op = targ_cg_find_mul_with_properties (opnd1, opnd2,
						 opnd1_shift, opnd1_width,
						 opnd1_signed,
						 opnd2_shift, opnd2_width,
						 opnd2_signed, result_shift,
						 swap_operands);
    // new_op should always be a valid multiply: at worst
    // it will be the multiply we started with.
    FmtAssert (new_op != TOP_UNDEFINED,
	       ("Unknown multiply in TARG_RangePropagate"));

    if (new_op != OP_code (op)) {
      if (swap_operands) {
	Build_OP (new_op, result, opnd2, opnd1, ops);
      } else {
	Build_OP (new_op, result, opnd1, opnd2, ops);
      }
      return TRUE;
    }
  }
  return FALSE;
}

static BOOL
match_and_or (const RangeAnalysis &range_analysis,
	      const OP *op,
	      OPS *ops)
{
  TOP opcode = OP_code(op);

  TN *opnd1 = OP_Opnd1(op);
  TN *opnd2 = OP_Opnd2(op);

  if (! opnd1 || ! opnd2 || ! OP_has_result (op))
    return FALSE;
  
  TN *result = OP_result (op, 0);

#define CASE_TOP(top) case TOP_##top##_r_r_r: case TOP_##top##_i_r_r: case TOP_##top##_ii_r_r
#define CASE_TOP_BR(top) case TOP_##top##_r_r_r: case TOP_##top##_i_r_r: case TOP_##top##_ii_r_r: \
		       case TOP_##top##_r_r_b: case TOP_##top##_i_r_b: case TOP_##top##_ii_r_b

  if ((TN_has_value (opnd2) && TN_value (opnd2) == 0)
      || (TN_is_register (opnd2) && TN_is_zero_reg (opnd2))) {
    // opnd2 is zero
    switch (opcode) {
    CASE_TOP_BR(andl):
    CASE_TOP(and):
    CASE_TOP(andc):
      Exp_Immediate (result, Gen_Literal_TN (0, TN_size(result)), FALSE, ops);
      return TRUE;
      
    CASE_TOP_BR(nandl):
      Exp_Immediate (result, Gen_Literal_TN (1, TN_size(result)), FALSE, ops);
      return TRUE;
    
    CASE_TOP(or):  
    CASE_TOP(xor):  
    CASE_TOP_BR(orl):
      Exp_COPY (result, opnd1, ops);
      return TRUE;
    }
  } else if (TN_has_value (opnd2)) {
    // opnd2 is non-zero.
    switch (opcode) {
    CASE_TOP_BR(orl):
      Exp_Immediate (result, Gen_Literal_TN (1, TN_size(result)), FALSE, ops);
      return TRUE;
    CASE_TOP_BR(norl):
      Exp_Immediate (result, Gen_Literal_TN (0, TN_size(result)), FALSE, ops);
      return TRUE;
    CASE_TOP_BR(andl):
      Exp_COPY (result, opnd1, ops);
      return TRUE;
    }
  }

  return FALSE;

#undef CASE_TOP
#undef CASE_TOP_BR
}

    
static BOOL
match_mulhs_forward (const RangeAnalysis &range_analysis,
		const OP *op,
		OPS *ops)
{
#define CASE_TOP(top) case TOP_##top##_r_r_r: case TOP_##top##_i_r_r: case TOP_##top##_ii_r_r
  TOP opcode = OP_code(op);

  switch (opcode) {
    CASE_TOP(mulhs):
      TN *result = OP_result (op, 0);
      TN *opnd1 = OP_Opnd1(op);
      TN *opnd2 = OP_Opnd2(op);
      LRange_p r1 = range_analysis.Get_Value (opnd1);
      LRange_p r2 = range_analysis.Get_Value (opnd2);
      LRange_p rres = range_analysis.Get_Value(result);
      INT tzcnt1 = r1->getTzcnt ();
      INT tzcnt2 = r2->getTzcnt ();
      INT tzcnt = tzcnt1 + tzcnt2;
      if (tzcnt >= rres->bits ()) {
	Exp_Immediate (result, Gen_Literal_TN(0, TN_size(result)), FALSE, ops);	
	return TRUE;
      }
  }
  
  return FALSE;
#undef CASE_TOP
}
    
static BOOL
match_mulhs_backward (const RangeAnalysis &range_analysis,
		const OP *op,
		OPS *ops)
{
#define CASE_TOP(top) case TOP_##top##_r_r_r: case TOP_##top##_i_r_r: case TOP_##top##_ii_r_r
  TOP opcode = OP_code(op);

  switch (opcode) {
    CASE_TOP(mulhs):
      TN *result = OP_result (op, 0);
      LRange_pc r1 = range_analysis.Get_Value_Backward (result);
      if (r1->bits() <= 16){
        Exp_Immediate (result, Gen_Literal_TN(0, TN_size(result)), FALSE, ops);
	return TRUE;
      }
  }
  
  return FALSE;
#undef CASE_TOP
}



static BOOL
match_shl_or_sequence (const RangeAnalysis &range_analysis,
		       OP *l1_op,
		       OPS *ops)
{
  OP *l2_op0, *l2_op1, *l2_op;
  TN *opnd1, *opnd2, *result;
  INT shiftcount;
  TYPE_ID mtype;
  BOOL is_signed;

  TOP opcode = OP_code(l1_op);

  if (!OP_ior(l1_op))
    return FALSE;

  opnd1 = OP_Opnd1(l1_op);
  opnd2 = OP_Opnd2(l1_op);


  l2_op0 = TN_is_register(opnd1) ? TN_ssa_def (opnd1) : NULL;
  l2_op1 = TN_is_register(opnd2) ? TN_ssa_def (opnd2) : NULL;

  if (! l2_op0 && ! l2_op1) return FALSE;

  if (l2_op0 && OP_code (l2_op0) == TOP_shl_i_r_r)
    l2_op = l2_op0;
  else if (l2_op1 && OP_code (l2_op1) == TOP_shl_i_r_r)
    l2_op = l2_op1;
  else
    return FALSE;
  
  if (TN_has_value(OP_Opnd2(l2_op)))
    shiftcount = TN_value(OP_Opnd2(l2_op));
  else
    return FALSE;

  if (shiftcount < 0 || shiftcount >= 4)
    return FALSE;

  // if we get here, then one of the operand of the IOR is a shl of
  // value n=1,2,3, thus it's worth transforming the IOR operator into an
  // IADD, if possible, so that the shNl, add => shNadd peephole can
  // apply further on in the EBO phase
  
  result = OP_result (l1_op, 0);
  is_signed = OP_result_is_signed(l1_op, 0);
  switch (TN_size (result)) {
  case 4:  mtype = is_signed ? MTYPE_I4 : MTYPE_U4; break;
  case 8:  mtype = is_signed ? MTYPE_I8 : MTYPE_U8; break;
  default: return FALSE;
  }

  LRange_p r1 = range_analysis.Get_Value (opnd1);
  LRange_p r2 = range_analysis.Get_Value (opnd2);
  UINT64 zmask1 = r1->getZeroMask ();
  UINT64 zmask2 = r2->getZeroMask ();
  // compute condition to transform an ior into
  // an iadd operator (no carry generated)
  if (~(zmask1 | zmask2) == 0){
    Expand_Add(result, opnd1, opnd2, mtype, ops);
    return TRUE;
  }
  return FALSE;
}

static BOOL
match_shl_mullh_sequence (const RangeAnalysis &range_analysis,
		       OP *l1_op,
		       OPS *ops)
{
  // Match
  //   r1 = mullh(u) r2, r3
  //   ...
  //   r3 = shl r1, 16  
  // and transform to
  //   r3 = mulhs r2, r3
  //  
  // provided that r2 is a 16-bit operand
  //
  OP *l2_op;
  TN *opnd1, *opnd2, *result;
  INT shiftcount;

  TOP opcode = OP_code(l1_op);


  if (opcode != TOP_shl_i_r_r)
    return FALSE;

  opnd1 = OP_Opnd1(l1_op);
  opnd2 = OP_Opnd2(l1_op);

  // shl must be by 16 bits
  if (!TN_has_value(opnd2) || TN_value(opnd2) != 16)
    return FALSE;

  l2_op = TN_ssa_def (opnd1);

  if (! l2_op) return FALSE;


  if (OP_code(l2_op) != TOP_mullh_i_r_r &&
      OP_code(l2_op) != TOP_mullh_ii_r_r &&
      OP_code(l2_op) != TOP_mullh_r_r_r &&
      OP_code(l2_op) != TOP_mullhu_i_r_r &&
      OP_code(l2_op) != TOP_mullhu_ii_r_r &&
      OP_code(l2_op) != TOP_mullhu_r_r_r) return FALSE;
  

  // if we get here, then we have an opportunity to catch a mulhs 
  // We must now verify that the 1st operand of the mullh(u) is
  // not larger than 16-bit

  LRange_p r = range_analysis.Get_Value (OP_Opnd1(l2_op));
  
  
  result = OP_result (l1_op, 0);

  if (r->bits() <= 16){
    // Determine new opcode:
    TOP new_opcode;
    switch (OP_code(l2_op)) {
    case TOP_mullh_i_r_r: 
    case TOP_mullhu_i_r_r: 
      new_opcode = TOP_mulhs_i_r_r; break;
    case TOP_mullh_ii_r_r: 
    case TOP_mullhu_ii_r_r: 
      new_opcode = TOP_mulhs_ii_r_r; break;
    case TOP_mullh_r_r_r: 
    case TOP_mullhu_r_r_r: 
      new_opcode = TOP_mulhs_r_r_r; break;
    default:
      FmtAssert(FALSE, (" wrong opcode %s\n", TOP_Name(OP_code(l2_op))));
    }

    Build_OP (new_opcode, result, OP_Opnd1(l2_op), OP_Opnd2(l2_op), ops);
    return TRUE;
  }
  return FALSE;
}

static BOOL
match_compare_subsph_to_zero (const RangeAnalysis &range_analysis,
		       OP *l1_op,
		       OPS *ops)
{
  OP *l2_op1, *l2_op2, *l2_op, *l3_op;
  TN *opnd1, *opnd2, *result;
  TN *l1_tn1, *l1_tn2, *l2_tn;
  INT shiftcount, result_width;
  TYPE_ID mtype;
  VARIANT variant;
  BOOL is_signed;
  LRange_p r1, r2, rhalf;

  TOP opcode = OP_code(l1_op);

  if (!OP_icmp(l1_op))
    return FALSE;

  l1_tn1 = OP_Opnd1 (l1_op);
  l1_tn2 = OP_Opnd2 (l1_op);

  l2_op1 = TN_is_register(l1_tn1) ? TN_ssa_def (l1_tn1) : NULL;
  l2_op2 = TN_is_register(l1_tn2) ? TN_ssa_def (l1_tn2) : NULL;

  if (! l2_op1 && ! l2_op2) return FALSE;

  if (l2_op1 && (OP_code(l2_op1) == TOP_sxth_r_r)
      && TN_Has_Value(l1_tn2)
      && (TN_Value(l1_tn2) == 0))
    l2_op = l2_op1;
  else if (l2_op2 && (OP_code(l2_op2) == TOP_sxth_r_r)
      && TN_Has_Value(l1_tn1)
      && (TN_Value(l1_tn1) == 0))
    l2_op = l2_op2;
  else
    return FALSE;
  
  l2_tn = OP_Opnd1 (l2_op);

  l3_op = TN_is_register(l2_tn) ? TN_ssa_def (l2_tn) : NULL;
  if (!l3_op || (OP_code(l3_op) != TOP_subs_ph_r_r_r))
    return FALSE; 

  opnd1 = OP_opnd (l3_op, 0);
  opnd2 = OP_opnd (l3_op, 1);
  
  r1 = range_analysis.Get_Value (opnd1);
  r2 = range_analysis.Get_Value (opnd2);
  rhalf = range_analysis.getLattice ()->makeRangeMinMax (-1 << 15, (1 << 15) - 1);
  
  // Ensure operands of subsph are 16-bit well-formed
  // otherwise a sign extension has to be provided
  if (rhalf->ContainsOrEqual(r1) && rhalf->ContainsOrEqual(r2)){
    result = OP_result(l1_op, 0);
    variant = TOP_cmp_variant(opcode);
    switch (variant) {
    case V_CMP_NE: 
      Expand_Int_Not_Equal(result, opnd1, opnd2, MTYPE_I4, ops);
      break;
    case V_CMP_LT: 
      Expand_Int_Less(result, opnd1, opnd2, MTYPE_I4, ops);	
      break;
    case V_CMP_GT: 
      Expand_Int_Greater(result, opnd1, opnd2, MTYPE_I4, ops);	
      break;
    case V_CMP_LE: 
      Expand_Int_Less_Equal(result, opnd1, opnd2, MTYPE_I4, ops);	
      break;
    case V_CMP_GE: 
      Expand_Int_Greater_Equal(result, opnd1, opnd2, MTYPE_I4, ops);	
      break;
    case V_CMP_EQ: 
      Expand_Int_Equal(result, opnd1, opnd2, MTYPE_I4, ops);	
      break;
    default: return FALSE; break;
    }    
    return TRUE;
  }
  else
    return FALSE;
}

BOOL
TARG_RangePropagate (const RangeAnalysis &range_analysis,
		     OP *op,
		     OPS *ops)
{
  TOP opcode = OP_code(op);

  if (range_analysis.Forward_Valid ()) {

    if (match_multiply (range_analysis, op, ops)) {
      return TRUE;
    }
    if (match_add_mul_sequence (range_analysis, op, ops)) {
      return TRUE;
    }
    if (match_mul64h_sequence (range_analysis, op, ops)) {
      return TRUE;
    }
    if (match_convib_sequence (range_analysis, op, ops)) {
      return TRUE;
    }
    if (match_and_or (range_analysis, op, ops)) {
      return TRUE;
    }
    if (match_mulhs_forward (range_analysis, op, ops)) {
      return TRUE;
    }
    if (match_shl_or_sequence (range_analysis, op, ops)) {
      return TRUE;
    }
    if (match_shl_mullh_sequence (range_analysis, op, ops)) {
      return TRUE;
    }
    if (match_compare_subsph_to_zero (range_analysis, op, ops)) {
      return TRUE;
    }
    if (match_addcg (range_analysis, op, ops)) {
      return TRUE;
    }
  }

  if (range_analysis.Backward_Valid ()) {

    if (match_mulhs_backward (range_analysis, op, ops)) {
      return TRUE;
    }
  }
  
  return FALSE;
}    

  
