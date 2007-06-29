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
 * Module: targ_range_analysis.cxx
 *
 * Description:
 *
 * Target-specific code for range analysis.
 *
 * ====================================================================
 */

#include "defs.h"
#include "tracing.h"
#include "config_target.h"
#include "cgir.h"
#include "range_analysis.h"
#include "lrange.h"
#include "targ_cg_private.h"
#include "ebo_info.h"

// Process result_idx'th result of op if it needs target-specific
// handling.
// Return TRUE if it has been processed, FALSE if it does not need
// target-specific processing.
BOOL
RangeAnalysis::TARG_Visit_Forward (OP *op, INT result_idx, LRange_pc &new_value,
				   BOOL *succs_done)
{
  TOP opcode = OP_code(op); 
  TN *result = OP_result(op, result_idx);
  static const struct MulProperties *mul_properties;

  if ((mul_properties = targ_cg_mul_properties (op)) != NULL) {

    // Multiply.

    INT opnd1_width  = mul_properties->opnd1_width;
    INT opnd1_shift  = mul_properties->opnd1_shift;
    BOOL opnd1_signed = mul_properties->opnd1_signed;
    INT opnd2_width  = mul_properties->opnd2_width;
    INT opnd2_shift  = mul_properties->opnd2_shift;
    BOOL opnd2_signed = mul_properties->opnd2_signed;
    INT result_shift = mul_properties->result_shift;
    TN *tn1 = OP_Opnd1(op);
    TN *tn2 = OP_Opnd2(op);
    LRange_pc opnd1 = Value(tn1);
    LRange_pc opnd2 = Value(tn2);

    opnd1 = SignedRightShift (opnd1, opnd1_shift);
    opnd2 = SignedRightShift (opnd2, opnd2_shift);

    opnd1 = (opnd1_signed ? SignExtend (opnd1, opnd1_width)
	     : ZeroExtend (opnd1, opnd1_width));
    opnd2 = (opnd2_signed ? SignExtend (opnd2, opnd2_width)
	     : ZeroExtend (opnd2, opnd2_width));

    new_value = Mul (opnd1, opnd2);
    if (result_shift < 0) {
      new_value = LeftShift (new_value, -result_shift);
    } else
      new_value = SignedRightShift (new_value, result_shift);
    return TRUE;
  } else if (opcode == TOP_andc_r_r_r || opcode == TOP_andc_i_r_r || opcode == TOP_andc_ii_r_r) {

    // Bitand-complement.

    LRange_pc opnd1 = Value(OP_Opnd1(op));
    LRange_pc opnd2 = Value(OP_Opnd2(op));
    new_value = BitAnd (BitNot (opnd1), opnd2);
    return TRUE;
  } else if (opcode == TOP_orc_r_r_r || opcode == TOP_orc_i_r_r || opcode == TOP_orc_ii_r_r) {
    // Bit-or complement.

    LRange_pc opnd1 = Value(OP_Opnd1(op));
    LRange_pc opnd2 = Value(OP_Opnd2(op));
    new_value = BitOr (BitNot (opnd1), opnd2);
    return TRUE;
  } else if (opcode == TOP_clz_r_r) {

    // Count leading zeros.

    TN *tn = OP_Opnd1(op);
    INT tn_topbit = TN_bitwidth (tn) - 1;
    LRange_pc bitrange = lattice->makeRangeValue (tn_topbit);
    LRange_pc opnd = MakeUnsigned(Value(tn), TN_bitwidth(tn));
    LRange_pc r = HighestSetBit (opnd);  // Calculate highest set bit.
    r = Min (bitrange, r);           // Limit it to the top bit of the tn.
    r = Sub (bitrange, r);         // Subtract from top bit to get clz value.
    new_value = r;
    return TRUE;
  } else if (opcode == TOP_bswap_r_r) {

    // Byte swap.

    TN *tn = OP_Opnd1 (op);
    LRange_pc opnd = MakeUnsigned(Value(tn), TN_bitwidth(tn));
    LRange_pc b0 = Extract (opnd, 0, 8);
    LRange_pc b1 = Extract (opnd, 8, 8);
    LRange_pc b2 = Extract (opnd, 16, 8);
    LRange_pc b3 = Extract (opnd, 24, 8);
    new_value = b3;
    new_value = Insert (new_value, 8,  8, b2);
    new_value = Insert (new_value, 16, 8, b1);
    new_value = Insert (new_value, 24, 8, b0);
    return TRUE;
  } else if (targ_cg_TOP_is_shadd (opcode)) {

    // Shift-add.

    TN *tn1 = OP_Opnd1 (op);
    TN *tn2 = OP_Opnd2 (op);
    LRange_pc opnd1 = Value (tn1);
    LRange_pc opnd2 = Value (tn2);
    new_value = Add (LeftShift (opnd1, targ_cg_TOP_shadd_amount(opcode)), opnd2);
    return TRUE;
  } else if (opcode == TOP_convbi_b_r) {

    // Boolean conversion, result in general register

    new_value = Ne (Value (OP_opnd (op, 0)), lattice->makeRangeValue (0));

  } else if (opcode == TOP_convib_r_b) {

    // Boolean conversion, result in branch register

    LRange_pc opnd = Value (OP_opnd (op, 0));
    if (opnd->isZero ()) {
      new_value = lattice->makeRangeValue (0);
      return TRUE;
    } else if (opnd->isNonZero ()) {
      new_value = lattice->makeRangeValue (1);
      return TRUE;
    } else
      return FALSE;

  } else if (opcode == TOP_targ_mov_b_r
	     || opcode == TOP_targ_mov_r_b) {

    // Inter-register-class copy.

    new_value = Value (OP_opnd (op, 0));
    return TRUE;
  } else if (opcode == TOP_sats_r_r) {
    // Performs range of sats instruction
    // which is a signed clamping on 16 bits 
    TN *tn  = OP_Opnd1 (op);
    LRange_pc opnd  = Value (tn);
    LRange_pc r_half = lattice->makeRangeMinMax (-1 << 15, (1 << 15) - 1);
    new_value = lattice->makeJoin (opnd, r_half);
    return TRUE;
  } else if (opcode == TOP_extract_i_r_r 
	     || opcode == TOP_extractl_i_r_r
	     || opcode == TOP_extractu_i_r_r
	     || opcode == TOP_extractlu_i_r_r) {
    TN *extracted_tn = OP_Opnd1(op);
    TN *extractcount_tn = OP_Opnd2(op);
    FmtAssert(TN_Has_Value(extractcount_tn),
	    ("ST240 Extract op must have a constant second operand"));  
    INT extractcount = TN_Value(extractcount_tn);
    UINT bit_offset, bit_size;
    st200_decode_extract_mask (opcode, extractcount, &bit_size, &bit_offset);
    INT lshiftcount = 32 - bit_offset - bit_size;
    INT rshiftcount = 32 - bit_size;
    LRange_p extracted = Value (extracted_tn);
    extracted = LeftShift (extracted, lshiftcount);
    if (TOP_is_unsign (opcode)) {
	extracted = MakeUnsigned (extracted, TN_bitwidth (extracted_tn));
    }
    new_value = SignedRightShift (extracted, rshiftcount);
    return TRUE;
  }
  return FALSE;
}

// Process opnd_idx'th operand of op if it needs target-specific
// handling.
// Return TRUE if it has been processed, FALSE if it does not need
// target-specific processing.
BOOL
RangeAnalysis::TARG_Visit_Backward (OP *op, INT opnd_idx, LRange_pc &new_value)
{
#define CASE_TOP(top) case TOP_##top##_r_r_r: case TOP_##top##_i_r_r: case TOP_##top##_ii_r_r
  TOP opcode = OP_code(op); 
  TN *opnd = OP_opnd(op, opnd_idx);
  const struct MulProperties *mul_properties;

  if ((mul_properties = targ_cg_mul_properties (op)) != NULL) {
    LRange_p result = Value(OP_result(op, 0));
    INT opnd_width;
    INT opnd_shift;
    BOOL opnd_found = FALSE;
    if (opnd_idx == OP_find_opnd_use (op, OU_opnd1)) {
      opnd_width = mul_properties->opnd1_width;
      opnd_shift = mul_properties->opnd1_shift;
      opnd_found = TRUE;
    } else if (opnd_idx == OP_find_opnd_use (op, OU_opnd2)) {
      opnd_width = mul_properties->opnd2_width;
      opnd_shift = mul_properties->opnd2_shift;
      opnd_found = TRUE;
    } else
      opnd_found = FALSE;
    if (opnd_found) {
      new_value = lattice->makeRangeBitWidth (opnd_width);
      new_value = LeftShift (new_value, opnd_shift);
    }
    switch (opcode) {
      CASE_TOP(mullu):
      CASE_TOP(mull):
      CASE_TOP(mulll):
      CASE_TOP(mulllu):
      CASE_TOP(mul32):
	new_value = lattice->makeJoin(result, new_value);
      break;
    }
    return TRUE;
  } else if (opcode == TOP_adds_ph_r_r_r || opcode == TOP_add_ph_r_r_r ||
	     opcode == TOP_subs_ph_r_r_r || opcode == TOP_sub_ph_r_r_r ||
	     opcode == TOP_mul_ph_r_r_r ) {
    	new_value = Value (OP_result(op, 0));
	return TRUE;
  } else if (opcode == TOP_extract_i_r_r 
	     || opcode == TOP_extractl_i_r_r
	     || opcode == TOP_extractu_i_r_r
	     || opcode == TOP_extractlu_i_r_r) {
    TN *extractcount_tn = OP_Opnd2(op);
    FmtAssert(TN_Has_Value(extractcount_tn),
	    ("ST240 Extract op must have a constant second operand"));  
    INT extractcount = TN_Value(extractcount_tn);
    UINT bit_offset, bit_size;
    st200_decode_extract_mask (opcode, extractcount, &bit_size, &bit_offset);
    new_value = lattice->makeRangeBitWidth ( bit_size + bit_offset);
    return TRUE;
  }
  return FALSE;
#undef CASE_TOP
}
