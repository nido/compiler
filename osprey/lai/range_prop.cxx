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
 * Module: range-propagation.cxx
 *
 * Description:
 *
 * Perform range propagation optimization.
 *
 * ====================================================================
 */

#include "defs.h"
#include "tracing.h"
#include "config_target.h"
#include "cgir.h"
#include "range_prop.h"
#include "cg_ssa.h"
#include "cg_ssaopt.h"
#include "ebo_info.h"
#include "ebo_special.h"
#include "cg.h"
#include "cg_flags.h"
#include "cgexp.h"

#include "range.h"
#include "lbrange.h"
#include "paired_lattice.h"

#define SWAP_TN(tn1, tn2) do { TN *tmp = tn1; tn1 = tn2; tn2 = tmp; } while (0)
#define SWAP_RANGE(r1, r2) do { LRange_p tmp = r1; r1 = r2; r2 = tmp; } while (0)


/* ====================================================================
 *
 *  Tracing.
 *
 *    Separated out to keep the main code readable.
 *
 * ====================================================================
 */

static BOOL trace_range_propagation;

static void
Trace_OP_Replacement (OP *op, OPS *ops)
{
  if (trace_range_propagation) {
    fputs ("Replace ", TFile);
    Print_OP_No_SrcLine (op);
    fputs (" ==> ", TFile);
    Print_OPS_No_SrcLines (ops);
    fputc ('\n', TFile);
  }
}
    
static void
Trace_RangePropagate_Start (INT pass)
{
  if (trace_range_propagation) {
    fprintf(TFile, "%s Beginning range propagation pass %d\n%s\n",
	    DBar, pass, DBar);
  }
}

static void
Trace_RangePropagate_Aborted ()
{
  if (trace_range_propagation) {
    fprintf (TFile, "%s range propagation aborted because analysis failed\n%s\n",
	     DBar, DBar);
  }
}

static void
Trace_RangePropagate_End_Pass (INT pass)
{
  if (trace_range_propagation) {
    fprintf(TFile, "Finished range propagation pass %d\n\n", pass);
  }
}

static void
Trace_RangePropagate_End ()
{
  Check_for_Dump (TP_RANGE, NULL);
}

/*
 * op_has_non_ssa_def()
 *
 * Returns true if one of the results is not a SSA variable.
 */
static BOOL
op_has_non_ssa_def(OP *op) 
{
  INT res;
  for (res = 0; res < OP_results(op); res++) {
    if (!TN_is_ssa_var(OP_result(op,res))) return TRUE;
  }
  return FALSE;
}

/*
 * op_can_be_replaced()
 *
 * Check if we can replace or remove the current operation.
 * Currently we filter:
 * - implicit interactions,
 * - predicated results that are not ssa defs.
 * If this is too restrictive while this code evolves, we may remove test from there
 * and do these checks on a per transformation basis.
 */
static BOOL
op_can_be_replaced(OP *op)
{
  if (OP_has_implicit_interactions(op)) return FALSE;
  if (OP_cond_def(op) && op_has_non_ssa_def(op)) return FALSE;
  return TRUE;
}

static BOOL
validate_replacement (OP *op,
		      OPS *ops)
{
  if (OPS_length (ops) != 1
      || OPs_Are_Equivalent (op, OPS_first (ops))
      // Do not replace register copies: they may be
      // completely removed by preferencing later.
      || OP_copy (op)) {
    return FALSE;
  }
  // ?? Do not convert register copy to move-immediate.
  // If we do this, we lose the opportunity to remove the
  // register copy.

  Trace_OP_Replacement (op, ops);

  OP_srcpos (OPS_first (ops)) = OP_srcpos (op);
  BB *bb = OP_bb (op);
  OP *prev = OP_prev (op);
  BB_Remove_Op (bb, op);
  if (prev)
    BB_Insert_Ops_After (bb, prev, ops);
  else
    BB_Prepend_Ops (bb, ops);
  return TRUE;
}

static BOOL
Try_To_Simplify_Operand0 (OP *op,
			  TN *result,
			  TN *opnd1,
			  TN *opnd2,
			  OPS *ops)
{
  return (EBO_Try_To_Simplify_Operand0 (op, result, opnd1, opnd2, ops)
	  && validate_replacement (op, ops));
}
    
static BOOL
Try_To_Simplify_Operand1 (OP *op,
			  TN *result,
			  TN *opnd1,
			  TN *opnd2,
			  OPS *ops)
{
  return (EBO_Try_To_Simplify_Operand1 (op, result, opnd1, opnd2, ops)
	  && validate_replacement (op, ops));
}

static TN *
Literal_Value (const RangeAnalysis &range_analysis, TN *tn)
{
  const LRange_p r = range_analysis.Get_Value (tn);
  if (r->hasValue ()) {
    INT64 v = r->getValue ();
    TN *result = Gen_Literal_TN (r->getValue (), TN_size (tn),
				 ! r->isNonNegative ());
    return result;
  }
  return NULL;
}

static BOOL
unneeded_extension (RangeAnalysis &range_analysis,
		    OP *op,
		    TN *result,
		    TN *opnd,
		    BOOL is_signed,
		    UINT32 leftshift, UINT32 rightshift,
		    OPS *ops)
{
  // If the operand is already within range,
  // or if none of the extension bits are used,
  // there is no need to do the extend.

  // We are effectively performing
  //        result = (opnd<<leftshift)>>rightshift
  // If is_signed, then the >> is arithmetic, otherwise it is logical.
  // This can be transformed to
  //        result = opnd << (leftshift-rightshift)  if leftshift >= rightshift
  //    or  result = opnd >> (rightshift-leftshift)  if rightshift > leftshift
  // No need to special case (leftshift == rightshift), since
  // ExpandShift will do that for us.
  // provided that:
  //        the top leftshift bits of opnd are already zero/sign extension.
  // or the top rightshift bits of the result are unneeded.
  
  Range::RangeSign sign = is_signed ? Range::Signed : Range::Unsigned;
  LRange_p ropnd;
  INT shiftcount = abs ((INT32)leftshift - (INT32)rightshift);
  UINT opnd_width = TN_bitwidth (opnd);
  if (range_analysis.Forward_Valid ())
    ropnd = range_analysis.getLattice()->makeRangeWidth (sign, opnd_width - leftshift);
  else if (range_analysis.Backward_Valid ())
    ropnd = range_analysis.getLattice()->makeRangeBitWidth ( opnd_width - leftshift);    
  TYPE_ID mtype;

  switch (TN_size (result)) {
  case 4:  mtype = is_signed ? MTYPE_I4 : MTYPE_U4; break;
  case 8:  mtype = is_signed ? MTYPE_I8 : MTYPE_U8; break;
  default: return FALSE;
  }

  if ((range_analysis.Forward_Valid ()
       && ropnd->ContainsOrEqual(range_analysis.Get_Value (opnd))
       )
      || (range_analysis.Backward_Valid ()
	  && (range_analysis.Get_Value_Backward (result)->bits ()
	      <= opnd_width - rightshift))) {
    SHIFT_DIRECTION dirn = ((leftshift >= rightshift) ? shift_left :
			    is_signed ? shift_aright : shift_lright);
    Expand_Shift (result, opnd,
		  Gen_Literal_TN (shiftcount, 4, FALSE),
		  mtype,
		  dirn, ops);
    if (validate_replacement (op, ops)) {
      return TRUE;
    }
  }
  return FALSE;
}

static BOOL
match_compare_sub_to_zero (const RangeAnalysis &range_analysis,
		       OP *l1_op,
		       OPS *ops)
{
  OP *l2_op1, *l2_op2, *l2_op;
  TN *opnd1, *opnd2, *result;
  TN *l1_tn1, *l1_tn2;
  INT shiftcount, result_width;
  TYPE_ID mtype;
  VARIANT variant;
  BOOL is_signed;

  TOP opcode = OP_code(l1_op);

  if (!OP_icmp(l1_op))
    return FALSE;

  l1_tn1 = OP_Opnd1 (l1_op);
  l1_tn2 = OP_Opnd2 (l1_op);

  l2_op1 = TN_is_register(l1_tn1) ? TN_ssa_def (l1_tn1) : NULL;
  l2_op2 = TN_is_register(l1_tn2) ? TN_ssa_def (l1_tn2) : NULL;

  if (! l2_op1 && ! l2_op2) return FALSE;

  if (l2_op1 && OP_isub(l2_op1)
      && TN_Has_Value(l1_tn2)
      && (TN_Value(l1_tn2) == 0))
    l2_op = l2_op1;
  else if (l2_op2 && OP_isub(l2_op2)
      && TN_Has_Value(l1_tn1)
      && (TN_Value(l1_tn1) == 0))
    l2_op = l2_op2;
  else
    return FALSE;
  
  opnd1 = OP_Opnd1(l2_op);
  opnd2 = OP_Opnd2(l2_op);
  result = OP_result(l2_op, 0);
  result_width = TN_bitwidth (result);
  
  LRange_p r1 = range_analysis.Get_Value (opnd1);
  LRange_p r2 = range_analysis.Get_Value (opnd2);
  LRange_p rres1 = Sub(r1, r2);
  LRange_p rres2 = SubModulo(r1, r2, result_width - 1);
  
  // compute non-wrapping condition for sub operands
  // restrict to signed comparison
  if (rres1->Equal(rres2)){
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


static BOOL
RangePropagateOp (RangeAnalysis &range_analysis,
		  OP *op, OP *&next)
  // Make transformations for op.
  // If range_analysis.Forward_Valid () is TRUE,
  // transformations must not increase the
  // magnitude of any result, because this will invalidate the
  // forward range information.
  // If range_analysis.Backward_Valid () is TRUE,
  // transformations must not increase the
  // number of significant bits in any operand, because this will
  // invalidate the backward range information.
  // Mostly, transformations do not change the value of any result,
  // or the significant bits in any operand.
{
  OPS ops = OPS_EMPTY;


  // Check early if we can replace or remove the current operation
  if (!op_can_be_replaced(op)) return FALSE;

  // First let target-specific code try it.

  if (TARG_RangePropagate (range_analysis, op, &ops)
      && validate_replacement (op, &ops)) {
    next = OPS_first (&ops);
    return TRUE;
  }
      
  TOP opcode = OP_code(op);
  TN *result = OP_has_result (op) ? OP_result (op, 0) : NULL;
  TN *opnd1 = OP_Opnd1(op);
  TN *opnd2 = OP_Opnd2(op);
  TYPE_ID mtype;
  if (OP_fixed_results (op) > 0) {
    BOOL is_signed = OP_result_is_signed(op, 0);
    switch (TN_size (result)) {
    case 4:  mtype = is_signed ? MTYPE_I4 : MTYPE_U4; break;
    case 8:  mtype = is_signed ? MTYPE_I8 : MTYPE_U8; break;
    }
  } else {
    mtype = MTYPE_UNKNOWN;
  }


  if (range_analysis.Forward_Valid ()) {

    // Try generic constant propagation.

    if (opnd1 && TN_is_register (opnd1)) {
      TN *new_opnd1 = Literal_Value (range_analysis, opnd1);
      if (new_opnd1 &&
	  Try_To_Simplify_Operand0 (op, result, new_opnd1, opnd2,
				    &ops)) {
	next = OPS_first (&ops);
	return TRUE;
      }
    }
    if (opnd2 && TN_is_register (opnd2)) {
      TN *new_opnd2 = Literal_Value (range_analysis, opnd2);
      if (new_opnd2 &&
	  Try_To_Simplify_Operand1 (op, result, opnd1, new_opnd2,
				    &ops)) {
	next = OPS_first (&ops);
	return TRUE;
      }
    }
  }

  // Handle specific ops., forward or backward.

  if (OP_sext(op) || OP_zext(op)) {
    
    // Sign/Zero-extend.
    
    INT extension_bits = (TN_bitwidth (opnd1)
			  - TOP_opnd_use_bits (opcode,
					       OP_find_opnd_use(op, OU_opnd1)));    
    BOOL is_signed = (OP_sext(op) != 0);
    if (unneeded_extension (range_analysis, op,
			    result, opnd1, is_signed,
			    extension_bits, extension_bits, &ops)) {
      return TRUE;
    }
  }

  if (OP_ishru(op) || OP_ishr(op)) {

    // Right-shift.
      
    BOOL is_signed = OP_ishr(op);
    if (TN_has_value (opnd2)) {
      INT64 rightshift = TN_value (opnd2);
      const TN *equiv_tn = range_analysis.Root_Equivalent (OP_Opnd1 (op));
      OP *op2 = TN_ssa_def (equiv_tn);
      if (op2 && OP_ishl (op2)) {
	TN *left_shift_opnd = OP_Opnd1 (op2);
	TN *left_shift_count_tn = OP_Opnd2 (op2);
	if (TN_has_value (left_shift_count_tn)) {
	  INT64 leftshift = TN_value (left_shift_count_tn);
	  if (unneeded_extension (range_analysis, op,
				  result, left_shift_opnd, is_signed,
				  leftshift, rightshift, &ops)) {
	    return TRUE;
	  }
	}
      }
    }
  }

  if (range_analysis.Forward_Valid ()) {
    // Handle specific ops., forward only.

    if (OP_imin(op) || OP_imax(op)) {

      // Min/Max.

      BOOL is_signed = TOP_is_unsign (opcode);
      LRange_p r1 = range_analysis.Get_Value (opnd1);
      LRange_p r2 = range_analysis.Get_Value (opnd2);
      if (is_signed) {
	r1 = MakeUnsigned (r1, TN_bitwidth (opnd1));
	r2 = MakeUnsigned (r2, TN_bitwidth (opnd1));
      }
      // Ensure greater range, if any, is in r1.
      if (Gt (r2, r1)->Equal (range_analysis.getLattice ()->makeRangeMinMax (1, 1))) {
	SWAP_RANGE (r1, r2);
	SWAP_TN (opnd1, opnd2);
      }
      if (Gt (r1, r2)->Equal (range_analysis.getLattice ()->makeRangeMinMax (1, 1))) {
	if (OP_imin (op)) {
	  // opnd2 must be the minimum
	  if (TN_is_register (opnd2))
	    Exp_COPY (result, opnd2, &ops);
	  else
	    Exp_Immediate (result, opnd2, ! is_signed, &ops);
	} else {
	  // opnd1 must be the maximum
	  if (TN_is_register (opnd1))
	    Exp_COPY (result, opnd1, &ops);
	  else
	    Exp_Immediate (result, opnd1, ! is_signed, &ops);
	}
	if (validate_replacement (op, &ops)) {
	  return TRUE;
	}
      }
    }

    if (OP_select(op)) {
      LRange_p rc = range_analysis.Get_Value (OP_Condition (op));
      VARIANT v = TOP_cond_variant (opcode);
      TN *selected_opnd = NULL;
      if (v == V_COND_FALSE) {
	SWAP_TN (opnd1, opnd2);
      } else if (v != V_COND_TRUE) {
	// unknown semantic.
	return FALSE; 
      }
      if (rc->isNonZero ()) {
	selected_opnd = opnd1;
      } else if (rc->isZero ()) {
	selected_opnd = opnd2;
      }
      if (selected_opnd) {
	if (TN_is_register (selected_opnd))
	  Exp_COPY (result, selected_opnd, &ops);
	else
	  Exp_Immediate (result, selected_opnd, FALSE, &ops);
	if (validate_replacement (op, &ops)) {
	  return TRUE;
	}
      }
    }

    // check if operands of a imul, iadd or isub have enough trailing zeroes,
    // so that the result is zero
    if (OP_imul(op) || OP_iadd(op) || OP_isub(op)) {
      LRange_p r1 = range_analysis.Get_Value (opnd1);
      LRange_p r2 = range_analysis.Get_Value (opnd2);
      LRange_p rres = range_analysis.Get_Value(result);
      INT tzcnt1 = r1->getTzcnt ();
      INT tzcnt2 = r2->getTzcnt ();
      INT tzcnt = OP_imul(op) ? tzcnt1 + tzcnt2 : Min(tzcnt1, tzcnt2);
      if (tzcnt >= rres->bits ()) {
	Exp_Immediate (result, Gen_Literal_TN(0, TN_size(result)), FALSE, &ops);	
	if (validate_replacement (op, &ops)) 
	  return TRUE;
      }
    }

    // check for constant result of any integer arithmetic operator 
    if (OP_imul(op) || OP_iadd(op) || OP_isub(op) 
	|| OP_ior(op) || OP_iand(op) || OP_ixor(op) 
	|| OP_ishl(op) || OP_ishr(op) || OP_ishru(op)) {
      LRange_p rres = range_analysis.Get_Value(result);
      if (rres->hasValue ()) {
	INT64 value = rres->getValue ();
	Exp_Immediate (result, Gen_Literal_TN(value, TN_size(result)), FALSE, &ops);	
	if (validate_replacement (op, &ops)) 
	  return TRUE;
      }
    }

    if (match_compare_sub_to_zero (range_analysis, op, &ops)) {
      if (validate_replacement (op, &ops)) 
	return TRUE;
    }

  } else if (range_analysis.Backward_Valid ()) {
    // Handle specific. ops., backward only.
    if (OP_ishl(op)) {

      // Shl
      LRange_p rres = range_analysis.Get_Value_Backward(result);
      // test if op can reduce to a copy to zero
      if (TN_has_value (opnd2)) {
	  INT64 leftshift = TN_value (opnd2);
	  if ( leftshift >= rres->bits() ) 
	      Exp_Immediate (result, Gen_Literal_TN(0, TN_size(result)), FALSE, &ops);
	  if (validate_replacement (op, &ops)) {
	      return TRUE;
	  }
      } 
    }
    // check for long immediates operands simplification to short immediates,
    // when range of result of a logical operator is less than 9-bit
    if (OP_ior(op) || OP_iand(op) || OP_ixor(op)){
      LRange_p rres = range_analysis.Get_Value_Backward(result);
      TN *opnd = NULL;
      UINT64 val;
      if (TN_Has_Value (opnd2)) {
	opnd = opnd1;
	val = TN_Value (opnd2) & rres->getBitMask();
      }
      else if (TN_is_rematerializable(opnd2) &&  TN_home(opnd2) != NULL
	       && WN_operator(TN_home(opnd2)) == OPR_INTCONST) {
	opnd = opnd1;
	val = WN_const_val(TN_home(opnd2)) & rres->getBitMask();
      }
      else if (TN_Has_Value (opnd1)) {
	opnd = opnd2;
 	val = TN_Value (opnd1) & rres->getBitMask();
      }
      else if (TN_is_rematerializable(opnd1) &&  TN_home(opnd1) != NULL
	       && WN_operator(TN_home(opnd1)) == OPR_INTCONST) {
	opnd = opnd2;
	val = WN_const_val(TN_home(opnd1)) & rres->getBitMask();
      }
      else
	return FALSE;
      if (mtype != MTYPE_UNKNOWN) {
	if (OP_ior(op))
	  Expand_Binary_Or (result, opnd, Gen_Literal_TN(val, TN_size (result)), mtype, &ops);
	else if (OP_ixor(op))
	  Expand_Binary_Xor (result, opnd, Gen_Literal_TN(val, TN_size (result)), mtype, &ops);
	else if (OP_iand(op))
	  Expand_Binary_And (result, opnd, Gen_Literal_TN(val, TN_size (result)), mtype, &ops);
	if (validate_replacement (op, &ops)) {
	  return TRUE;
	}
      }
    }
  }
  return FALSE;
}

static BOOL
RangePropagatePass (RangeAnalysis &range_analysis, INT pass)
{
  BOOL changed = FALSE;
  MEM_POOL mempool;
  MEM_POOL_Initialize (&mempool, "range analysis pool", TRUE);

// paired-lattice (value plus bit-value ranges) initialization 
// for forward RA pass
  VBLattice::MEMPOOL_Initialize(mempool);
  range_analysis.Initialize (VBLattice::getInstance (), mempool);

  if (! range_analysis.Can_Analyze ()) {
    Trace_RangePropagate_Aborted ();
    range_analysis.Finalize ();
    return FALSE;
  }

  Trace_RangePropagate_Start (pass);
  
  range_analysis.Analyze_Forward ();

  BB *bb;
  for (bb = REGION_First_BB; bb != NULL; bb = BB_next(bb)) {
    OP *op;
    OP *next = NULL;
    for (op = BB_first_op (bb); op != NULL; op = next) {
      OPS ops = OPS_EMPTY;
      next = OP_next (op);

      changed |= RangePropagateOp (range_analysis, op, next);
    }
  }
  
// used-bit lattice initialization for backward RA pass
  LBRange::MEMPOOL_Initialize(mempool);
  range_analysis.Initialize_Backward (LBRange::getInstance ());

  range_analysis.Analyze_Backward ();

  for (bb = REGION_First_BB; bb != NULL; bb = BB_next(bb)) {
    OP *op;
    OP *next = NULL;
    for (op = BB_first_op (bb); op != NULL; op = next) {
      OPS ops = OPS_EMPTY;
      next = OP_next (op);

      changed |= RangePropagateOp (range_analysis, op, next);
    }
  }
  
  Trace_RangePropagate_End_Pass (pass);
  
  range_analysis.Finalize ();
  MEM_POOL_Delete (&mempool);
 

  return changed;
}

void
RangePropagate ()
{
  if (CG_enable_range_propagation) {

    trace_range_propagation = Get_Trace(TP_RANGE, RANGE_PROPAGATION);

    if (trace_range_propagation) {
      fprintf(TFile, "%s Before range propagation\n%s\n", DBar, DBar);
      Print_All_BBs ();
    }

    RangeAnalysis range_analysis;
    BOOL changed;
    const INT Max_Passes = 3;
    INT passes = 0;

    do {
      passes++;
      changed = RangePropagatePass (range_analysis, passes);
      if (changed) SSA_DeadCode ();
    } while (changed && passes < Max_Passes);

    Trace_RangePropagate_End ();
  }
}
