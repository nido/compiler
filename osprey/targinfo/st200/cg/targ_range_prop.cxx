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
#define IS_POWER_OF_2(x) ({ __typeof__(x) x_ = x ; x_ && !(x_ & (x_-1)) ; })

static BOOL no_uses_p (const RangeAnalysis &range_analysis, TN *tn)
{
  return TN_is_ssa_var (tn) && range_analysis.Uses (tn) == NULL;
}

static INT get_zero_byte_nb (const RangeAnalysis &range_analysis, TN *tn)
{
  // returns the byte number of the highest known zero byte in tn
  // returns -1 if no zero byte is known
  // assume the tn is 32-bit wide
  //
  INT zbyte;
  UINT64 zmask;
  LRange_p r = range_analysis.Get_Value (tn);

  FmtAssert (TN_bitwidth (tn) == 32,
	       ("32-bit wide tn expected"));
  
  zmask = r->getZeroMask ();

  if (!(~zmask & 0xff000000))
    zbyte = 3;
  else if (!(~zmask & 0xff0000))
    zbyte = 2;
  else if (!(~zmask & 0xff00))
    zbyte = 1;
  else if (!(~zmask & 0xff))
    zbyte = 0;
  else
    zbyte = -1;

  return zbyte;
}

static BOOL is_zero_byte (const RangeAnalysis &range_analysis, TN *tn, int byte_nb)
{
  // returns TRUE if byte number byte_nb is zero
  // otherwise returns FALSE
  //
  INT zbyte;
  UINT64 zmask;
  LRange_p r = range_analysis.Get_Value (tn);
  zmask = r->getZeroMask ();

  if (!((~zmask >> (8*byte_nb)) & 0xff))
    return TRUE;
  else
    return FALSE;
}

static BOOL is_valid_byte_mask(INT64 mask)
{
  // returns TRUE mask is a valid 32-bit byte mask
  // that is, for each byte b0,..,b3, bi is either
  // 0x0 or 0xff (so that it selects a whole byte),
  // otherwise returns FALSE
  //
  for (int i = 0; i < 4; i++){
    if ((mask & (0xff << (8*i))) && 
	(~mask & (0xff << (8*i))))
      return FALSE;
  }
  return TRUE;
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
  else if (Range_single_use(range_analysis, opnd_b) &&
	   rb_in->isZero ()) {
    if (Range_single_use(range_analysis, opnd_r1) &&
	r1_in->hasValue () && r1_in->getValue () == 1) {
      TN *new_b = Build_RCLASS_TN(ISA_REGISTER_CLASS_branch);
      Exp_Immediate(new_b, Gen_Literal_TN(1, TN_size(new_b)), FALSE, ops);
      Build_OP (TOP_addcg_b_r_r_b_r, result_r, result_b, Zero_TN, opnd_r2, new_b, ops);
      return TRUE;
    }
    else if (Range_single_use(range_analysis, opnd_r2) &&
	     r2_in->hasValue () && r2_in->getValue () == 1) {
      TN *new_b = Build_RCLASS_TN(ISA_REGISTER_CLASS_branch);
      Exp_Immediate(new_b, Gen_Literal_TN(1, TN_size(new_b)), FALSE, ops);
      Build_OP (TOP_addcg_b_r_r_b_r, result_r, result_b, Zero_TN, opnd_r1, new_b, ops);
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
      || ! Range_single_use (range_analysis, opnd)) return FALSE;

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

  if (!ISA_SUBSET_LIST_Member (ISA_SUBSET_List, TOP_mulhs_r_r_r))
    return FALSE;

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
match_zeroextend_sequence (const RangeAnalysis &range_analysis,
		       OP *l1_op,
		       OPS *ops)
{
  // Match
  //
  //   r3 = and r1, c
  //   where c + 1 is a power of two 
  //   and r5 = nb of significant bits(c)
  //
  //   match also,
  //
  //   r4 = sub 32, r5
  //   r2 = shru r6, r4
  //   r3 = and r1, r2
  //   where r6 = -1
  // and transform both expressions to
  //   r3 = zxt r1, r5
  //
  //
  OP *l2_op, *l3_op;
  TN *opnd1, *opnd2, *result;
  INT shiftcount;
  INT bits;

  if (!ISA_SUBSET_LIST_Member (ISA_SUBSET_List, TOP_zxt_r_r_r))
    return FALSE;

  TOP opcode = OP_code(l1_op);
  

  if (opcode != TOP_and_r_r_r && opcode != TOP_and_ii_r_r)
    return FALSE;

  opnd1 = OP_Opnd1(l1_op);
  opnd2 = OP_Opnd2(l1_op);
  result = OP_result (l1_op, 0);


  // Check if we have a pattern : r2 = and r1, mask
  if (range_analysis.Get_Value (OP_Opnd2(l1_op))->hasValue ()
      && IS_POWER_OF_2(range_analysis.Get_Value (OP_Opnd2(l1_op))->getValue () + 1)){
    bits = range_analysis.Get_Value (OP_Opnd2(l1_op))->bits ();
    Build_OP (TOP_zxt_i_r_r, result, opnd1, Gen_Literal_TN(bits, 4), ops);
    return TRUE;
  }

  // Get level-2 data
  l2_op = TN_ssa_def (opnd2);

  if (!l2_op) 
    return FALSE;
  
  if (!OP_ishru(l2_op)     
      || !range_analysis.Get_Value (OP_Opnd1(l2_op))->hasValue ()
      || range_analysis.Get_Value (OP_Opnd1(l2_op))->getValue () != -1)
    return FALSE;

  opnd2 = OP_Opnd2(l2_op);

  // Get level-3 data
  l3_op = TN_ssa_def (opnd2);

  if (!l3_op) 
    return FALSE;

  if (!OP_isub(l3_op) 
      || !range_analysis.Get_Value (OP_Opnd1(l3_op))->hasValue ()
      || range_analysis.Get_Value (OP_Opnd1(l3_op))->getValue () != 32)
    return FALSE;

  // if we get here, then we have a zxt sequence 
  Build_OP (TOP_zxt_r_r_r, result, opnd1, OP_Opnd2(l3_op), ops);
  return TRUE;
}

static BOOL
match_extend_sequence (const RangeAnalysis &range_analysis,
		       OP *l1_op,
		       OPS *ops)
{
  // Match
  //   r4 = sub 32, r5
  //   r2 = shl r1, r4
  //   r3 = shr(u) r2, r4
  // and transform to
  //   r3 = s/zxt r1, r5
  //
  // Note: this is just an SSA-based peephole: it does not
  // require any range information.
  //
  OP *l21_op, *l22_op, *l3_op;
  TN *opnd1, *opnd2, *result;
  INT shiftcount;
  BOOL is_signed;

  if (!ISA_SUBSET_LIST_Member (ISA_SUBSET_List, TOP_zxt_r_r_r) || 
      !ISA_SUBSET_LIST_Member (ISA_SUBSET_List, TOP_sxt_r_r_r))
    return FALSE;

  TOP opcode = OP_code(l1_op);
  

  if (opcode != TOP_shr_r_r_r && opcode != TOP_shru_r_r_r)
    return FALSE;

  is_signed = opcode == TOP_shr_r_r_r;

  opnd1 = OP_Opnd1(l1_op);
  opnd2 = OP_Opnd2(l1_op);

  l22_op = TN_ssa_def (opnd2);

  if (!l22_op) 
    return FALSE;

  if (!OP_isub(l22_op) 
      || !range_analysis.Get_Value (OP_Opnd1(l22_op))->hasValue ()
      || range_analysis.Get_Value (OP_Opnd1(l22_op))->getValue () != 32)
    return FALSE;

  l21_op = TN_ssa_def (opnd1);

  if (!l21_op) 
    return FALSE;

  if (!OP_ishl(l21_op))
    return FALSE;

  opnd1 = OP_Opnd1(l21_op);
  opnd2 = OP_Opnd2(l21_op);

  l3_op = TN_ssa_def (opnd2);

  if (!l3_op) 
    return FALSE;

  if (!OP_isub(l3_op) 
      || !range_analysis.Get_Value (OP_Opnd1(l3_op))->hasValue ()
      || range_analysis.Get_Value (OP_Opnd1(l3_op))->getValue () != 32)
    return FALSE;

  if (!TNs_Are_Equivalent(OP_Opnd2(l22_op), OP_Opnd2(l3_op)))
    return FALSE;

  // if we get here, then we have either a zxt or sxt sequence 
  
  result = OP_result (l1_op, 0);

  Build_OP (is_signed ? TOP_sxt_r_r_r : TOP_zxt_r_r_r, result, opnd1, OP_Opnd2(l3_op), ops);
  return TRUE;
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
    variant = OP_cmp_variant(l1_op);
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


static BOOL
match_rotl_sequence (const RangeAnalysis &range_analysis,
		       OP *l1_op,
		       OPS *ops)
{
  // Match
  //   r1 = shl r2, c1
  //   r3 = shru r2, c2
  //   (where 0 < c1 < 32
  //         0 < c2 < 32
  //   and c1 + c2 = 32)
  //   r4 = or r1, r3 
  // and transform to
  //   r4 = rotl r2, c1
  //  
  //
  OP *l21_op, *l22_op;
  TN *opnd1, *opnd2, *result;
  INT64 shiftcount1, shiftcount2, rotcount;

  if (!ISA_SUBSET_LIST_Member (ISA_SUBSET_List, TOP_rotl_i_r_r))
    return FALSE;

  TOP opcode = OP_code(l1_op);

  if (opcode != TOP_or_r_r_r)
    return FALSE;

  opnd1 = OP_Opnd1(l1_op);
  opnd2 = OP_Opnd2(l1_op);

  l21_op = TN_ssa_def (opnd1);
  l22_op = TN_ssa_def (opnd2);

  if (!l21_op || !l22_op) return FALSE;


  if (!((OP_code(l21_op) == TOP_shl_i_r_r &&
       OP_code(l22_op) == TOP_shru_i_r_r) ||
      (OP_code(l22_op) == TOP_shl_i_r_r &&
       OP_code(l21_op) == TOP_shru_i_r_r)))
    return FALSE;

  opnd1 = OP_Opnd2(l21_op);
  opnd2 = OP_Opnd2(l22_op);

  shiftcount1 = TN_Value(opnd1);
  shiftcount2 = TN_Value(opnd2);

  if (!TNs_Are_Equivalent(OP_Opnd1(l21_op), OP_Opnd1(l22_op)))
    return FALSE;

  if (!(shiftcount1 > 0 && shiftcount1 < 32 &&
	shiftcount2 > 0 && shiftcount2 < 32 &&
	shiftcount1 + shiftcount2 == 32))
    return FALSE;

  // if we get here, then we have a rotl pattern
  result = OP_result (l1_op, 0);
  rotcount = OP_ishl(l21_op) ? shiftcount1 : shiftcount2;

  Build_OP (TOP_rotl_i_r_r, result, OP_Opnd1(l21_op), Gen_Literal_TN (rotcount, TN_size(result)) , ops);
  return TRUE;
}

static BOOL
match_shrrnp_sequence (const RangeAnalysis &range_analysis,
		       OP *l1_op,
		       OPS *ops)
{
  // Match
  //   r2 = add r1, c1
  //   r3 = shr r2, c2
  //   (where c1 = 2^n with 0 <= n < 31
  //   and c2 = n + 1)
  // and transform to
  //   r3 = shrrnp r1, c2
  //  
  //
  OP *l2_op;
  TN *opnd1, *opnd2, *result;
  INT64 shiftcount, val_add;
  LRange_p r1, r2;

  if (!ISA_SUBSET_LIST_Member (ISA_SUBSET_List, TOP_shrrnp_i_r_r))
    return FALSE;

  TOP opcode = OP_code(l1_op);

  if (!OP_ishr(l1_op))
    return FALSE;

  opnd1 = OP_Opnd1(l1_op);
  opnd2 = OP_Opnd2(l1_op);

  r1 = range_analysis.Get_Value (opnd2);
      
  if (r1->hasValue ())
    shiftcount = r1->getValue ();
  else
    return FALSE;

  l2_op = TN_ssa_def (opnd1);

  if (!l2_op) return FALSE;


  if (!OP_iadd(l2_op))
    return FALSE;

  opnd1 = OP_Opnd1(l2_op);
  opnd2 = OP_Opnd2(l2_op);

  r2 = range_analysis.Get_Value (opnd2);
      
  if (r2->hasValue ())
    val_add = r2->getValue ();
  else
    return FALSE;

  if (!(IS_POWER_OF_2(val_add) && (r2->getTzcnt () < 31) && (r2->getTzcnt () == shiftcount - 1)))
    return FALSE;

  // if we get here, then we have a shrrnp pattern
  result = OP_result (l1_op, 0);

  Build_OP (TOP_shrrnp_i_r_r, result, opnd1, Gen_Literal_TN (shiftcount, TN_size(result)) , ops);
  return TRUE;
}



class PermMask {
 private:
  unsigned int byte_[4];
 public:
  // PermMask to st240 byte permutation value conversion
  int FromPermMask ();
  // Access to byte number i
  unsigned int getByte(int i);
  // get a 32-bit mask, resulting from a permutation of mask
  int getMask (int mask);
  // computes the PermMask resulting from the composition of 2 permutations
  friend const PermMask Combine (const PermMask &pmask1, const PermMask &pmask2);
  // Constructors
  PermMask ();
  PermMask (const PermMask &pmask);
  PermMask (unsigned int byte0, unsigned int byte1,
	    unsigned int byte2, unsigned int byte3);
  PermMask (int val);
};

int 
PermMask::FromPermMask () {
  // Method to abstract the conversion from a PermMask
  // to a 8-bit st240 byte permutation mask
  //
  int mask;
  mask = byte_[0] | (byte_[1] << 2) 
    | (byte_[2] << 4) | (byte_[3] << 6);
  return mask;
}

unsigned int 
PermMask::getByte(int i){
  FmtAssert((i >=0 && i < 4), ("byte number out of word bounds"));
  return byte_[i];
}


int
PermMask::getMask(int mask1){
  // compute resulting mask, after the byte permutation
  // of mask1 
  int resmask = 0;
  for (int i = 0; i < 4; i++)
    resmask |= ((mask1 >> (8*(byte_[i]))) & 0xff) << (8*i);
  return resmask;
} 

const PermMask 
Combine (PermMask &pmask1, PermMask &pmask2) {
  // Computes the st240 byte permutation mask resulting of the
  // composition of two byte permutations
  //
  unsigned int byte[4];
  for (int i = 0; i < 4; i++) {
    byte[i] = pmask2.getByte(pmask1.getByte(i));
  }
  return PermMask(byte[3], byte[2], byte[1], byte[0]);
}

PermMask::PermMask (){
   for (int i = 0; i < 4; i++) {
    byte_[i] = i;
  }
}
 
PermMask::PermMask (const PermMask &pmask) {
  for (int i = 0; i < 4; i++) {
    byte_[i] = pmask.byte_[i];
  }
}

PermMask::PermMask (int val) {
  for (int i = 0; i < 4; i++) {
    byte_[i] = (val >> (2*i)) & 0x3;
  }
}

PermMask::PermMask (unsigned int byte3, unsigned int byte2,
		    unsigned int byte1, unsigned int byte0) {
  byte_[0] = byte0;
  byte_[1] = byte1;
  byte_[2] = byte2;
  byte_[3] = byte3;
}
  
static BOOL is_perm_equiv(const RangeAnalysis &range_analysis,
			  OP *op,
			  TN **tn,
			  PermMask &pmask) 
{
  // Returns TRUE for ops equivalent to a perm.pb instruction
  // Put permute operands in *tn and mask
  //
  // Consider SHRU, SHL, ZXT, ROTL operators 
  // with constant second operand of 8, 16, and 24
  // and ZXTH, ZXTB operators,
  // and also AND operator with constant second operand 
  // of value val such as is_valid_byte_mask(val) is TRUE
  //
  // These are equivalent to a  perm.pb, provided that
  // there is a zero byte in their first operand
  //

  TN *opnd2=NULL;
  BOOL case1=FALSE, case2=FALSE, case3=FALSE;
  BOOL case4=FALSE, case5=FALSE, case6=FALSE;
  int val;
  LRange_p r2;
  int mask;
  unsigned int byte[4];

  if (!ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_perm_pb_i_r_r))
    return FALSE;

  TOP opcode = OP_code(op);

  switch (opcode) {
  case TOP_shru_i_r_r:
  case TOP_shl_i_r_r:
  case TOP_zxt_i_r_r:
    case1 = TRUE;
    break;
  case TOP_perm_pb_i_r_r:
    case2 = TRUE;
    break;
  case TOP_rotl_i_r_r:
    case3 = TRUE;
    break;
  case TOP_and_r_r_r: case TOP_and_i_r_r: case TOP_and_ii_r_r:
    case4 = TRUE;
    break;
  case TOP_zxth_r_r:
    case5 = TRUE;
    break;
  case TOP_zxtb_r_r:
    case6 = TRUE;  
    break;
  default:  
    return FALSE;
  }
  *tn = OP_Opnd1(op);
  if (!case5 && !case6) {
    opnd2 = OP_Opnd2(op);
    r2 = range_analysis.Get_Value (opnd2);
    if (r2->hasValue ())
      val = r2->getValue ();
    else
      return FALSE;
  }
  // check for a zero byte in *tn
  int zbyte_nb = get_zero_byte_nb(range_analysis, *tn);
  // if there's no zero byte information for cases 1,4,5,6 return FALSE
  if ((case1 || case4 || case5 || case6) && (zbyte_nb == -1))
    return FALSE;
  else if (case2){
    pmask = PermMask(val);
    return TRUE;
  }
  else if (case4){
    if (is_valid_byte_mask(val)){
      for (int i = 0; i < 4; i++){
	byte[i] = (val >> (8*i)) & 0xff ?
	  i : zbyte_nb;
      }
      pmask =  PermMask(byte[3], byte[2], byte[1], byte[0]);
      return TRUE;
    }
    return FALSE;
  }
  else if (case5){ // zxth case, Z-Z-1-0 permutation
    pmask = PermMask(zbyte_nb, zbyte_nb, 1, 0);
    return TRUE;
  }
  else if (case6){ // zxtb case, Z-Z-Z-0 permutation
    pmask = PermMask(zbyte_nb, zbyte_nb, zbyte_nb, 0);
    return TRUE;
  }
  else { // case3 || (case1 && (zbyte_nb != -1))
    switch (val) {
    case 8:
      if (OP_ishru(op))
	pmask = PermMask(zbyte_nb, 3, 2, 1); // Z-3-2-1 permutation
      else if (OP_ishl(op))
	pmask = PermMask(2, 1, 0, zbyte_nb); // 2-1-0-Z permutation
      else if (case1) // zxt case, Z-Z-Z-0 permutation
	pmask = PermMask(zbyte_nb, zbyte_nb, zbyte_nb, 0);
      else if (case3) // rotl case
	pmask = PermMask(2, 1, 0, 3); // 2-1-0-3 permutation
      else
	FmtAssert(FALSE, ("unexpected case of permutation"));
      return TRUE;
    case 16:
      if (OP_ishru(op)) // Z-Z-3-2 permutation
	pmask = PermMask(zbyte_nb, zbyte_nb, 3, 2); 
      else if (OP_ishl(op)) // 1-0-Z-Z permutation
	pmask = PermMask(1, 0, zbyte_nb, zbyte_nb);
      else if (case1) // zxt case, Z-Z-1-0 permutation
	pmask = PermMask(zbyte_nb, zbyte_nb, 1, 0);
      else if (case3) // rotl case
	pmask = PermMask(1, 0, 3, 2); // 1-0-3-2 permutation
      else
	FmtAssert(FALSE, ("unexpected case of permutation"));
      return TRUE;
    case 24:
      if (OP_ishru(op)) // Z-Z-Z-3 permutation
	pmask = PermMask(zbyte_nb, zbyte_nb, zbyte_nb, 3);
      else if (OP_ishl(op)) // 0-Z-Z-Z permutation
	pmask = PermMask(0, zbyte_nb, zbyte_nb, zbyte_nb);
      else if (case1) // zxt case, Z-2-1-0 permutation
	pmask = PermMask(zbyte_nb, 2, 1, 0);
      else if (case3) // rotl case
	pmask = PermMask(0, 3, 2, 1); // 0-3-2-1 permutation
      else
	FmtAssert(FALSE, ("unexpected case of permutation"));
      return TRUE;
    default:
      return FALSE;
    }
  }
}

static BOOL 
is_permute_bitand (const RangeAnalysis &range_analysis,
		   OP *l1_op,
		   TN **tn,
		   int *mask,
		   PermMask &pmask) 
{
  // check for sequence
  //   r2 = and r1, mask1
  //   r3 = perm.pb r2, A
  // which is equivalent to
  //   r2 = perm.pb r1, A
  //   r3 = and r2, mask2
  // where
  //   mask2 = perm.pb mask1, A 
  //
  // Put mask2 in *mask, r1 in *tn, and PermMask(A) in *val
  //
  OP *l2_op;
  TN *opnd1, *opnd2, *opnd21, *result;
  int mask1, resmask;
  LRange_p r2;

  TOP opcode = OP_code(l1_op);

  if (is_perm_equiv(range_analysis, l1_op, &opnd21, pmask))
    {
      
      // get level-2 informations
      
      l2_op = TN_ssa_def (opnd21);
      
      if (!l2_op || !OP_iand(l2_op))
	return FALSE;
      
      opnd1 = OP_Opnd1(l2_op);
      opnd2 = OP_Opnd2(l2_op);

      r2 = range_analysis.Get_Value (opnd2);
      
      if (r2->hasValue ())
	mask1 = r2->getValue ();
      else
	return FALSE;
      
      // compute resulting mask, after the byte permutation
      // defined by pmask1
      resmask = pmask.getMask(mask1);
    } 
  else if (OP_iand(l1_op))
    {
      opnd1 = OP_Opnd1(l1_op);
      opnd2 = OP_Opnd2(l1_op);
 
      r2 = range_analysis.Get_Value (opnd2);
     
      // get level-2 informations
      
      l2_op = TN_ssa_def (opnd1);
      
      
      if (!(l2_op &&
	    is_perm_equiv(range_analysis, l2_op, &opnd1, pmask) &&
	    r2->hasValue ())) 
	return FALSE;

      resmask = r2->getValue ();
    } 
  else
    return FALSE;

  *tn = opnd1;
  *mask = resmask;
  return TRUE;
}

static BOOL 
match_permute_combine (const RangeAnalysis &range_analysis,
		       OP *l1_op,
		       OPS *ops)
{
  // Match
  //   r2 = perm.pb r1, A
  //   r3 = perm.pb r2, B
  // and transform to
  //   r3 = perm.pb r1, A@B
  // where
  //   @ is a combine operation
  //
  OP *l2_op;
  TN *opnd1, *opnd2, *result;
  PermMask pmask1, pmask2, pmaskres;
  INT valres;
  
  TOP opcode = OP_code(l1_op);
  
  if (!is_perm_equiv(range_analysis, l1_op, &opnd1, pmask1))
    return FALSE;

  // get level-2 informations

  l2_op = TN_ssa_def (opnd1);

  if (!l2_op || !is_perm_equiv(range_analysis, l2_op, &opnd1, pmask2))
    return FALSE;

  pmaskres = Combine (pmask1, pmask2);

  valres = pmaskres.FromPermMask();
 
  result = OP_result (l1_op, 0);

  Build_OP (TOP_perm_pb_i_r_r, result, opnd1, Gen_Literal_TN (valres, TN_size(result)), ops);
  return TRUE;
}

static BOOL 
match_permute_add_or (const RangeAnalysis &range_analysis,
		       OP *l1_op,
		       OPS *ops)
{
  // Match
  //   r2 = perm.pb r1, A
  //   r3 = perm.pb r1, B 
  //   r4 = r2 | r3 / r4 = r2 + r3
  // and transform to
  //   r4 = perm.pb r1, C
  // if and only if
  //   for each byte b0...b3, bi is either
  //   a zero byte of r2 or a zero byte of r3 (or both)
  // 
  // This also catches the special case:
  //   
  //   r2 = perm.pb r1, A
  //   r4 = r2 | r1 / r4 = r2 + r1
  //
  OP *l2_op1, *l2_op2;
  TN *opnd, *opnd1, *opnd2, *opnd21, *opnd22, *result;
  INT64 mask1, mask2;
  INT valres;
  PermMask pmask1, pmask2, pmaskres;
  unsigned int byte[4];
  
  TOP opcode = OP_code(l1_op);

  if (!(OP_ior(l1_op) | OP_iadd(l1_op)))
    return FALSE;

  opnd1 = OP_Opnd1(l1_op);
  opnd2 = OP_Opnd2(l1_op);

  // get level-2 informations

  l2_op1 = TN_ssa_def (opnd1);
  l2_op2 = TN_ssa_def (opnd2);

  if (!l2_op1 || !l2_op2)
    return FALSE;


  if  (is_perm_equiv(range_analysis, l2_op1, &opnd21, pmask1) &&
       is_perm_equiv(range_analysis, l2_op2, &opnd22, pmask2) &&
       opnd21 == opnd22) {
    opnd = opnd21;
  } 
  else if (is_perm_equiv(range_analysis, l2_op1, &opnd21, pmask1) &&
	   opnd21 == opnd2){
    pmask2 = PermMask(3, 2, 1, 0); // 3-2-1-0 permutation (identity)
    opnd = opnd21;
  } 
  else if (is_perm_equiv(range_analysis, l2_op2, &opnd22, pmask2) &&
	   opnd22 == opnd1) {
    pmask1 = PermMask(3, 2, 1, 0); // 3-2-1-0 permutation (identity)
    opnd = opnd22;
  } 
  else 
    return FALSE;

  // at this point, it's sure we have the expected code pattern
  // Now the resulting permutation value mask can be computed

  for (int i = 0; i < 4; i++) {
    if (is_zero_byte (range_analysis, opnd1, i))
      byte[i] = pmask2.getByte(i);
    else if (is_zero_byte (range_analysis, opnd2, i))
      byte[i] = pmask1.getByte(i);
    else
      return FALSE;
  } 
  
  pmaskres = PermMask(byte[3], byte[2], byte[1], byte[0]);
  valres = pmaskres.FromPermMask();
  result = OP_result (l1_op, 0);

  Build_OP (TOP_perm_pb_i_r_r, result, opnd, Gen_Literal_TN (valres, TN_size(result)), ops);
  return TRUE;
}


static BOOL 
match_permute_or_and (const RangeAnalysis &range_analysis,
		       OP *l1_op,
		       OPS *ops)
{
  // Match
  //   r2 = perm.pb r1, A
  //   r3 = perm.pb r1, B
  //   r4 = r2 & mask1
  //   r5 = r3 & mask2
  //   r6 = r4 | r5 / r6 = r4 + r5
  // and transform to
  //   r6 = perm.pb r1, C
  // if and only if
  //   1) mask1 and mask2 select whole bytes (not parts of bytes)
  //      for instance mask1 = 0xff00ff00
  //   2) ~(mask1 ^ mask2) == 0  
  //
  OP *l2_op1, *l2_op2, *l3_op1, *l3_op2;
  TN *opnd1, *opnd2, *opnd3, *opnd4, *opnd21, *opnd22, *result;
  INT valres;
  INT mask1[1], mask2[1];
  PermMask pmask1, pmask2, pmaskres;
  unsigned int byte[4];
  
  TOP opcode = OP_code(l1_op);

  if (!(OP_ior(l1_op) | OP_iadd(l1_op)))
    return FALSE;

  opnd1 = OP_Opnd1(l1_op);
  opnd2 = OP_Opnd2(l1_op);

  // get level-2 informations

  l2_op1 = TN_ssa_def (opnd1);
  l2_op2 = TN_ssa_def (opnd2);

  if (!(l2_op1 && l2_op2 &&
	is_permute_bitand(range_analysis, l2_op1, &opnd21, mask1, pmask1) &&
	is_permute_bitand(range_analysis, l2_op2, &opnd22, mask2, pmask2) &&
	opnd21 == opnd22 &&
	is_valid_byte_mask(*mask1) &&
	is_valid_byte_mask(*mask2) &&
	!(~(*mask1 ^ *mask2))))
    return FALSE;
 
  // at this point, it's sure we have the expected code pattern
  // Now the resulting permutation value mask can be computed

  for (int i = 0; i < 4; i++) {
    if ((*mask1 >> (8*i)) & 0xff)
      byte[i] = pmask1.getByte(i);
    else if ((*mask2 >> (8*i)) & 0xff)
      byte[i] = pmask2.getByte(i);
    else
      FmtAssert(FALSE, ("unexpected case of permutation"));
  } 

  pmaskres = PermMask(byte[3], byte[2], byte[1], byte[0]);
  valres = pmaskres.FromPermMask();
  result = OP_result (l1_op, 0);

  Build_OP (TOP_perm_pb_i_r_r, result, opnd21, Gen_Literal_TN (valres, TN_size(result)), ops);
  return TRUE;
}

static BOOL
match_shift_add_sequence(const RangeAnalysis &range_analysis,
			  OP *op,
			  OPS *ops)
{
  // Match
  //   r2 = r3 + r4 (or immediate)
  //   r0 = r1 shift r2
  // and transform to
  //   r2 = r1 shift r4 (or immediate)
  //   r0 = r2 shift r3
  // if and only if
  //   range of r2,r3,r4 is [0,31]
  //
  int val; 
  TOP opcode = OP_code (op);

  if (opcode != TOP_shl_r_r_r && opcode != TOP_shr_r_r_r && opcode != TOP_shru_r_r_r ) 
    return FALSE;

  TN *opnd_r1 = OP_opnd(op, 0);
  TN *opnd_r2 = OP_opnd(op, 1);
  TN *result_shift = OP_result (op, 0);

  OP *l1_op = TN_ssa_def (opnd_r2);
  if (l1_op == NULL)  //if r2 is return value from a function call
    return FALSE;

  TOP opcode_l1 = OP_code(l1_op);
  if (opcode_l1 != TOP_add_i_r_r && opcode_l1 != TOP_add_r_r_r) 
    return FALSE;
  
  TN *opnd_r2_1 = OP_opnd(l1_op,0);
  TN *opnd_r2_2 = OP_opnd(l1_op,1);

  LRange_pc shifts = range_analysis.Get_Value (opnd_r2);
  LRange_pc val1 = range_analysis.Get_Value(opnd_r2_1);
  LRange_pc val2 = range_analysis.Get_Value(opnd_r2_2);
  LRange_pc rref = range_analysis.getLattice ()->makeRangeMinMax (0,31);

  if(!rref->ContainsOrEqual(val1)||!rref->ContainsOrEqual(val2)||!rref->ContainsOrEqual(shifts))
    return FALSE;

  TOP new_opcode;
  if(opcode_l1 == TOP_add_i_r_r ){
    switch (opcode) {
    case TOP_shl_r_r_r:
      new_opcode = TOP_shl_i_r_r; 
      break;
    case TOP_shr_r_r_r:
      new_opcode = TOP_shr_i_r_r; 
      break;
    case TOP_shru_r_r_r: 
      new_opcode = TOP_shru_i_r_r; 
      break;
    default:
      FmtAssert(FALSE, ("Unexpected opcode %s in match_shift_add_sequence\n", TOP_Name(opcode)));
    }
  }
  else if (opcode_l1 == TOP_add_r_r_r ){
    new_opcode = opcode;
  }
  else{
    FmtAssert(FALSE, ("Unexpected opcode_l1 %s in match_shift_add_sequence\n", TOP_Name(opcode_l1)));
  }

  TN *r_new = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer);  
  Build_OP (new_opcode, r_new, opnd_r1,opnd_r2_2, ops);
  Build_OP (opcode, result_shift, r_new,opnd_r2_1, ops);
  return TRUE;
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
    if (match_rotl_sequence (range_analysis, op, ops)) {
      return TRUE;
    }
    if (match_shrrnp_sequence (range_analysis, op, ops)) {
      return TRUE;
    }
    if (match_zeroextend_sequence (range_analysis, op, ops)) {
      return TRUE;
    }
    if (match_extend_sequence (range_analysis, op, ops)) {
      return TRUE;
    }
    if (match_permute_add_or (range_analysis, op, ops)) {
      return TRUE;
    }
    if (match_permute_or_and (range_analysis, op, ops)) {
      return TRUE;
    }
    if (match_permute_combine (range_analysis, op, ops)) {
      return TRUE;
    }   
    if (match_shift_add_sequence(range_analysis, op, ops)) {
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

  
