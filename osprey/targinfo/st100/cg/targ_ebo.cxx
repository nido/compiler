/*

  Copyright (C) 2002, ST Microelectronics, Inc.  All Rights Reserved.

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

  Contact information:  
*/


/* =======================================================================
 * =======================================================================
 *
 *  Module: targ_ebo.cxx
 *
 *  Revision comments:
 *
 *  17-June-1998 - Initial version
 *
 *  Description:
 *  ============
 *
 *  EBO target-specific optimizations.
 *
 * =======================================================================
 * =======================================================================
 */

#ifdef USE_PCH
#include "cg_pch.h"
#endif // USE_PCH
#pragma hdrstop

#ifdef _KEEP_RCS_ID
static const char source_file[] = __FILE__;
#endif /* _KEEP_RCS_ID */

#include <stdarg.h>
#include "defs.h"
#include "config_TARG.h"
#include "errors.h"
#include "mempool.h"
#include "tracing.h"
#include "timing.h"

#include "targ_isa_properties.h"

#include "cgir.h"
#include "tn_map.h"
#include "cg_loop.h"
#include "cg.h"
#include "cgexp.h"
#include "register.h"
#include "cg_region.h"
#include "wn.h"
#include "region_util.h"
#include "op_list.h"
#include "cgprep.h"
#include "gtn_universe.h"
#include "gtn_set.h"
#include "cg_db_op.h"
#include "whirl2ops.h"
#include "cgtarget.h"
#include "gra_live.h"
#include "reg_live.h"
#include "cflow.h"
#include "cg_spill.h"
#include "data_layout.h"

#include "ebo.h"
#include "ebo_info.h"
#include "ebo_special.h"
#include "ebo_util.h"

/* Define a macro to strip off any bits outside of the left most 4 bytes. */
#define TRUNC_32(val) (val & 0x00000000ffffffffll)

/* Define a macro to sign-extend the least signficant 32 bits */
#define SEXT_32(val) (((INT64)(val) << 32) >> 32)

/* =====================================================================
 *   EBO_Can_Merge_Into_Offset
 *
 *   Identify OP's that contain a constant and operate in a way that
 *   will allow the constant to be added into an offset field of
 *   a load or store instruction.
 * =====================================================================
 */
BOOL 
EBO_Can_Merge_Into_Offset (
  OP *op,
  INT *index,
  INT *immed
)
{
  if (!OP_iadd(op) && !OP_isub(op) && !OP_ior(op)) {
    return FALSE;
  }
  if ((op == BB_entry_sp_adj_op(OP_bb(op))) ||
      (op == BB_exit_sp_adj_op(OP_bb(op)))) {
     return FALSE;
  }

  if ((OP_iadd(op) || OP_ior(op)) || OP_isub(op) && 
      TN_Is_Constant(OP_opnd(op,2))) {
    *index = 1;
    *immed = 2;
    return TRUE;
  }

  return FALSE;
}

/* =====================================================================
 *   EBO_bit_length
 *
 *   Determine the length of significant bits.
 * =====================================================================
 */
static INT
EBO_bit_length (
  OP *op
)
{
#if 0
  switch (OP_code(op))
  {
    case TOP_ldw_i:
      return 4;

    case TOP_and_i: {

      if (TN_has_value(OP_opnd(op,1))) {
       /* Only handle right justified masks, for now. */

        switch (TN_value(OP_opnd(op,1))) {
        case 1: return 1;
        case 3: return 2;
        case 7: return 3;
        case 15: return 4;
        case 31: return 5;
        case 63: return 6;
        case 127: return 7;
        case 255: return 64; /*it's sign extended */
        default: return -1;
        }

      }
      break;

    }
  }
#endif
  return -1;
}
 
/* =====================================================================
 *   EBO_condition_redundant
 * =====================================================================
 */
BOOL
EBO_condition_redundant (
  OP *elim_op,
  EBO_TN_INFO **opnd_tninfo,
  EBO_OP_INFO *prev_opinfo,
  OPS *ops
)
{
  OP *prev_op = prev_opinfo->in_op;

  TOP p_opcode = OP_code(prev_op);
  TOP e_opcode = OP_code(elim_op);

  return FALSE;
}

/* =====================================================================
 *   EBO_combine_adjacent_loads
 * =====================================================================
 */
BOOL
EBO_combine_adjacent_loads(
  OP *op,
  EBO_TN_INFO **opnd_tninfo,
  EBO_OP_INFO *opinfo,
  INT64 offset_pred,
  INT64 offset_succ
)
{
  return FALSE;
}

/* =====================================================================
 *   EBO_replace_subset_load_with_extract
 * =====================================================================
 */
BOOL
EBO_replace_subset_load_with_extract (
  OP *op,
  TN *pred_result,
  TN *succ_result,
  INT offset_pred,
  INT offset_succ,
  INT size_pred,
  INT size_succ
)
{
  INT byte_offset = offset_succ - offset_pred;

  return FALSE;
}

/* =====================================================================
 *   EBO_copy_value
 *
 *   Return TRUE if we did it.
 * =====================================================================
 */
BOOL
EBO_copy_value (
  OP *op,                   // load being removed
  TN *pred_result,          // preceeding store
  TN *intervening_result,   // intervening store
  INT size                  // load size
)
{
  OPS ops = OPS_EMPTY;
  TN *predicate_tn = OP_has_predicate(op)?OP_opnd(op,OP_PREDICATE_OPND):NULL;

  /* 
   * unsigned Store - Load sequence for small integers will strip 
   * off sign bit.
   */
  if ((size != 4) && OP_unsigned_ld(op)) {
    /* 
     * Does it make sense to use bit-extract sequence to simulate 
     * store-load sequence ?
     */
    return FALSE;
  }

  /* Use full word copy. */
  EBO_Exp_COPY(predicate_tn, OP_result(op, 0), pred_result, &ops);
  OP_srcpos(OPS_last(&ops)) = OP_srcpos(op);

  BB_Insert_Ops(OP_bb(op), op, &ops, FALSE);

  return TRUE;
}

/* =====================================================================
 *   EBO_select_value
 *
 *   Compare the reload address with the intervening store address.
 *   Select the stored value if the address are the same,
 *   and resuse the predecesor value if they are not the same.
 * =====================================================================
 */
BOOL
EBO_select_value (
  OP *op,
  TN *pred_result,         // preceeding store's result
  TN *pred_base,           // preceeding store's base
  TN *pred_offset,
  TN *intervening_result,
  TN *intervening_base,
  TN *intervening_offset,
  INT size                 // size of the load
)
{
  OPS ops = OPS_EMPTY;
  TN *predicate_tn = OP_has_predicate(op)?OP_opnd(op,OP_PREDICATE_OPND):NULL;

  TN *predicate = Dup_TN_Even_If_Dedicated(predicate_tn);

  /* Compare the addresses. */
  Build_OP (TOP_GP32_EQW_GT_BR_DR_DR, predicate, predicate_tn, pred_base, intervening_base, &ops);
  OP_srcpos(OPS_last(&ops)) = OP_srcpos(op);

  if ((size != 4) &&
      (TN_register_class(intervening_result) == ISA_REGISTER_CLASS_du) &&
      (TN_register_class(pred_result) == ISA_REGISTER_CLASS_du)) {
    /* 
     * Store - Load sequence for small integers will strip off 
     * sign bit. 
     */
    /* Use extract instruction to simulate store-load sequence. */
    return FALSE;
  }

#if 0
    TOP extr_op = TOP_extr_u; /* Loads are always zero-filled on IA-64! */
    INT bit_size = size_succ*8;
    INT bit_offset = 0; /* Assume right justified in register. */

   /* Copy the "address not equal value". */
    Build_OP(extr_op, OP_result(op, 0), predicate_tn, pred_result,
             Gen_Literal_TN(bit_offset, 4), Gen_Literal_TN(bit_size, 4), &ops);
    if (predicate_tn != True_TN) Set_OP_cond_def_kind(OPS_last(&ops), OP_PREDICATED_DEF);
    OP_srcpos(OPS_last(&ops)) = OP_srcpos(op);

   /* Copy the "address equal result". */
    Build_OP(extr_op, OP_result(op, 0), predicate1, intervening_result,
             Gen_Literal_TN(bit_offset, 4), Gen_Literal_TN(bit_size, 4), &ops);
    Set_OP_cond_def_kind(OPS_last(&ops), OP_PREDICATED_DEF);
    OP_srcpos(OPS_last(&ops)) = OP_srcpos(op);
  } else {
#endif

  /* Use full word copies. */
  OPS ops1 = OPS_EMPTY;

  /* Copy the "address not equal value". */
  EBO_Exp_COPY(predicate_tn, OP_result(op, 0), pred_result, &ops1);
  if (predicate_tn != True_TN) 
    Set_OP_cond_def_kind(OPS_last(&ops1), OP_PREDICATED_DEF);
  OP_srcpos(OPS_last(&ops1)) = OP_srcpos(op);
  OPS_Append_Ops(&ops, &ops1);

  /* Copy the "address equal result". */
  OPS_Init(&ops1);
  EBO_Exp_COPY(predicate, OP_result(op, 0), intervening_result, &ops1);
  Set_OP_cond_def_kind(OPS_last(&ops1), OP_PREDICATED_DEF);
  OP_srcpos(OPS_last(&ops1)) = OP_srcpos(op);
  OPS_Append_Ops(&ops, &ops1);

  BB_Insert_Ops(OP_bb(op), op, &ops, FALSE);

  if (EBO_Trace_Optimization) {
    #pragma mips_frequency_hint NEVER
    fprintf(TFile,"%sRemove Reload across intervening store.\n",EBO_trace_pfx);
  }

  return TRUE;
}

/* =====================================================================
 *    Normalize_Immediate
 *
 *    Normalize an immediate value for use with a specific instruction.
 *    In some cases the instruction may ignore parts of the immediate,
 *    so we mimic that here.
 * =====================================================================
 */
static INT64 
Normalize_Immediate (
  TOP opcode, 
  INT64 immed
)
{
#if 0
  switch (opcode) {

  case TOP_cmpeq_i_r:
    /*
     * For signed compares sign-extend the constant to 64 bits.
     */
    return SEXT_32(immed);
  case TOP_cmpleu_i_r:
    /*
     * For unsigned compares, zero-extend the constant to 64 bits.
     */
    return TRUNC_32(immed);
  }
#endif
  return immed;
}

/* =====================================================================
 *    EBO_simplify_operand0
 *
 *    Return a new_op if the opcode can be simplified to a
 *    immediate form.
 * =====================================================================
 */
OP *
EBO_simplify_operand0 (
  OP *op,           // OP being simplified
  TN *tnr,          // result TN
  INT64 const_val,  // constant value of opnd TN 1
  TN *tn1           // opnd TN 2
)
{
  OP *new_op;


  return NULL;
}

/* =====================================================================
 *    EBO_simplify_operand1
 *
 *    Return a new_op if the opcode can be simplified to a
 *    immediate form.
 * =====================================================================
 */
OP*
EBO_simplify_operand1 (
  OP *op,           // OP being simplified
  TN *tnr,          // result TN
  TN *tn0,          // opnd TN 1
  INT64 const_val   // constant value of opnd TN 2
)
{
  return NULL;
}

/* =====================================================================
 *    EBO_Resolve_Conditional_Branch
 *
 *    Look at a branch exression that has all constant operands and 
 *    attempt to evaluate the expression.
 * =====================================================================
 */
BOOL
EBO_Resolve_Conditional_Branch (
  OP *op, 
  TN **opnd_tn
)
{
  BB *bb = OP_bb(op);
  BB *fall_bb;
  BB *branch_bb;

  if (BBlist_Len(BB_succs(bb)) != 2) return FALSE;

  fall_bb = BB_next(bb);
  branch_bb = BBLIST_item(BB_succs(bb));
  if (branch_bb == fall_bb) {
      branch_bb = BBLIST_item(BBLIST_next(BB_succs(bb)));
  }

#if 0
  if (OP_code(op) == TOP_br) {
    TN *predicate = OP_opnd(op,0);

    if (EBO_Trace_Optimization) {
      INT i;
      INT opndnum = OP_opnds(op);
      fprintf(TFile, "%sin BB:%d Resolve conditional BR :- %s ",
              EBO_trace_pfx, BB_id(bb),TOP_Name(OP_code(op)));
      for (i = 0; i < opndnum; i++) {
        fprintf(TFile," ");
        Print_TN(opnd_tn[i],FALSE);
      }
      fprintf(TFile,"\n");
    }

    if (predicate == True_TN) {
     /* 
      * Branch IS taken - replace the conditional branch with a 
      * simple branch. 
      */
      OPS ops = OPS_EMPTY;

      Build_OP (TOP_goto, 
		OP_opnd(op,1), 
		&ops);
      OP_srcpos(OPS_first(&ops)) = OP_srcpos(op);
      BB_Insert_Ops(OP_bb(op), op, &ops, FALSE);
      Unlink_Pred_Succ (bb, fall_bb);
      Change_Succ_Prob (bb, branch_bb, 1.0);

      return TRUE;;
    } 
  }
#endif

  return FALSE;
}

/* =====================================================================
 *    EBO_Fold_Constant_Compare
 * =====================================================================
 */
BOOL
EBO_Fold_Constant_Compare (
  OP *op, 
  TN **opnd_tn,
  OPS *ops
)
{
  TOP opcode = OP_code(op);
  TN *tn0, *tn1;
  INT64 tn0_val, tn1_val;
  UINT64 tn0_uval, tn1_uval;
  INT64 result_val;

  tn0 = opnd_tn[0];
  tn1 = opnd_tn[1];
  tn0_val = TN_Value (tn0);
  tn1_val = TN_Value (tn1);
  tn0_uval = TN_Value (tn0);
  tn1_uval = TN_Value (tn1);

  switch (opcode) {
#if 0
    case TOP_cmpeq_i_r:
    case TOP_cmpeq_ii_r:
    case TOP_cmpeq_i_b:
    case TOP_cmpeq_ii_b:
      result_val = (tn0_val == tn1_val);
      break;

    case TOP_cmpne_i_r:
    case TOP_cmpne_ii_r:
    case TOP_cmpne_i_b:
    case TOP_cmpne_ii_b:
      result_val = (tn0_val != tn1_val);
      break;

    case TOP_cmplt_i_r:
    case TOP_cmplt_ii_r:
    case TOP_cmplt_i_b:
    case TOP_cmplt_ii_b:
      result_val = (tn0_val < tn1_val);
      break;

    case TOP_cmpgt_i_r:
    case TOP_cmpgt_ii_r:
    case TOP_cmpgt_i_b:
    case TOP_cmpgt_ii_b:
      result_val = (tn0_val > tn1_val);
      break;

    case TOP_cmple_i_r:
    case TOP_cmple_ii_r:
    case TOP_cmple_i_b:
    case TOP_cmple_ii_b:
      result_val = (tn0_val <= tn1_val);
      break;

    case TOP_cmpge_i_r:
    case TOP_cmpge_ii_r:
    case TOP_cmpge_i_b:
    case TOP_cmpge_ii_b:
      result_val = (tn0_val >= tn1_val);
      break;

    case TOP_cmpltu_i_r:
    case TOP_cmpltu_ii_r:
    case TOP_cmpltu_i_b:
    case TOP_cmpltu_ii_b:
      result_val = (tn0_uval < tn1_uval);
      break;

    case TOP_cmpgtu_i_r:
    case TOP_cmpgtu_ii_r:
    case TOP_cmpgtu_i_b:
    case TOP_cmpgtu_ii_b:
      result_val = (tn0_uval > tn1_uval);
      break;

    case TOP_cmpleu_i_r:
    case TOP_cmpleu_ii_r:
    case TOP_cmpleu_i_b:
    case TOP_cmpleu_ii_b:
      result_val = (tn0_uval <= tn1_uval);
      break;

    case TOP_cmpgeu_i_r:
    case TOP_cmpgeu_ii_r:
    case TOP_cmpgeu_i_b:
    case TOP_cmpgeu_ii_b:
      result_val = (tn0_uval >= tn1_uval);
      break;
#endif
    default:
      return FALSE;
      break;
  }

  TN *result = OP_result(op,0);

#if 0
  if (TN_register_class(result) == ISA_REGISTER_CLASS_integer) {
    Build_OP((result_val == 0) ? TOP_cmpne_r_r : TOP_cmpeq_r_r, result, Zero_TN, Zero_TN, ops);
  }
  else {
    Build_OP((result_val == 0) ? TOP_cmpne_r_b : TOP_cmpeq_r_b, result, Zero_TN, Zero_TN, ops);
  }
#endif

  OP_srcpos(OPS_last(ops)) = OP_srcpos(op);

  if (EBO_Trace_Optimization) {
    fprintf(TFile, "%sin BB:%d Result of compare is %s for ",
	    EBO_trace_pfx, BB_id(OP_bb(op)),(result_val == 0) ? "FALSE" : "TRUE");
    Print_TN(OP_result(op,0),FALSE); 
    fprintf(TFile, "\n");
  }

  return TRUE;
}

/* =====================================================================
 *    EBO_Fold_Special_Opcode
 * 
 *    Target-specific folding.
 *
 *    TODO: extend OP semantics, so SHIFTs and SIGN-EXTs become 
 *          target-independent.
 * =====================================================================
 */
BOOL 
EBO_Fold_Special_Opcode (
  OP *op,
  TN **opnd_tn,
  INT64 *result_val
)
{
  TOP opcode = OP_code(op);
  TN *tn0, *tn1;
  INT64 tn0_val, tn1_val;
  UINT64 tn0_uval, tn1_uval;

#if 0
  if ((opcode == TOP_sxtb_r) || (opcode == TOP_sxth_r)) {
    INT length = EBO_bit_length(op);
    tn0 = opnd_tn[0];
    if (TN_is_symbol(tn0)) {
     /* What can we do? */
      return FALSE;
    }
    tn0_val = TN_Value (tn0);
    *result_val = (tn0_val << (64-length)) >> (64-length);
    goto Folded;
  }

  if ((opcode == TOP_zxth_r)) {
    INT length = EBO_bit_length(op);
    tn0 = opnd_tn[0];
    if (TN_is_symbol(tn0)) {
     /* What can we do? */
      return FALSE;
    }
    tn0_uval = TN_Value (tn0);
    *result_val = (tn0_uval << (64-length)) >> (64-length);
    goto Folded;
  }

  if ((opcode == TOP_shl_i) || (TOP_shl_ii == opcode)) {
    tn0 = opnd_tn[0];
    tn1 = opnd_tn[1];
    tn0_val = TN_Value (tn0);
    tn1_val = TN_Value (tn1);

    if (TN_size(tn0) == 4) {
      if ((tn1_val < 0) || (tn1_val > 31)) {
	*result_val = 0;
      } else {
	// do arithmetic in 32 bits and then sign extend to 64
	INT32 tn0_sval = (INT32)tn0_val;
	INT32 res_sval = tn0_sval << tn1_val;
	*result_val = (INT64)res_sval;
      }
    }
    else {
      // entire arithmetic in 64 bits
      if ((tn1_val < 0) || (tn1_val > 63)) {
	*result_val = 0;
      } else {
	*result_val = tn0_val << tn1_val;
      }
    }
    goto Folded;
  }

  if ((opcode == TOP_shru_i) || (opcode == TOP_shru_ii)) {
    tn0 = opnd_tn[0];
    tn1 = opnd_tn[1];
    tn0_uval = TN_Value (tn0);
    tn1_val = TN_Value (tn1);

    if (TN_size(tn0) == 4) {
      if ((tn1_val < 0) || (tn1_val > 31)) {
	*result_val = 0;
      } else {
	// 32 bit arithmetic, then sign-extend to 64 bits
	UINT32 tn0_sval = (UINT32)tn0_uval;
	INT32 res_sval = tn0_sval >> tn1_val;
	*result_val = (INT64)res_sval;
      }
    }
    else { // 64 bit
      if ((tn1_val < 0) || (tn1_val > 63)) {
	*result_val = 0;
      } else {
	*result_val = tn0_uval >> tn1_val;
      }
    }
    goto Folded;
  }

  if ((TOP_shr_i == opcode) || (opcode == TOP_shr_ii)) {
    tn0 = opnd_tn[0];
    tn1 = opnd_tn[1];
    tn0_val = TN_Value (tn0);
    tn1_val = TN_Value (tn1);

    if (TN_size(tn0) == 4) {
      // 32-bit arithmetic, then sign-extend to 64 bits
      if ((tn1_val < 0) || (tn1_val > 31)) 
	tn1_val = 31;
      INT32 tn0_sval = (INT32)tn0_val;
      INT32 res_sval = tn0_sval >> tn1_val;
      *result_val = (INT64)res_sval;
    }
    else {  // 64 bits
      if ((tn1_val < 0) || (tn1_val > 63)) 
	tn1_val = 63;
      *result_val = tn0_val >> tn1_val;
    }

    goto Folded;
  }
#endif

  return FALSE;

 Folded:

  if (EBO_Trace_Optimization) {
    fprintf(TFile, "%sfolded: %llx\n", EBO_trace_pfx, *result_val);
  }
  return TRUE;
}

/* =====================================================================
 *    Identify_Base_and_Offset
 * =====================================================================
 */
static void
Identify_Base_and_Offset (
  EBO_TN_INFO **opnd_tninfo, 
  TN **opnd_tn, 
  INT64 *opnd_offset
)
{
  EBO_TN_INFO *check_tninfo = *opnd_tninfo;
  TN *indx_tn = *opnd_tn;
  INT64 offset = 0;
  BOOL keep_going = TRUE;

  while (keep_going && (check_tninfo != NULL)) {
    keep_going = FALSE;
    EBO_OP_INFO *opnd_opinfo = locate_opinfo_entry(check_tninfo);
    if ((opnd_opinfo != NULL) &&
        (opnd_opinfo->in_op != NULL)) {
      OP *input_op = opnd_opinfo->in_op;
      if (OP_effectively_copy(input_op)) {
        check_tninfo = opnd_opinfo->actual_opnd[OP_Copy_Operand(input_op)];
        indx_tn = OP_opnd(input_op, OP_Copy_Operand(input_op));
        keep_going = TRUE;
      } else if (OP_memory(input_op)) {
        INT base_idx = TOP_Find_Operand_Use(OP_code(input_op),OU_base);
        INT inc_idx = TOP_Find_Operand_Use(OP_code(input_op),OU_postincr);
        if ((inc_idx < 0) || (!TNs_Are_Equivalent(OP_opnd(input_op,base_idx),check_tninfo->local_tn))) {
          check_tninfo = NULL;
          break;
        }
        offset += TN_Value(OP_opnd(input_op,inc_idx));
        check_tninfo = opnd_opinfo->actual_opnd[base_idx];
        indx_tn = OP_opnd(input_op,base_idx);
        keep_going = TRUE;
      } else if (OP_iadd(input_op)) {
        //INT op1_idx = TOP_Find_Operand_Use(OP_code(input_op),OU_opnd1);
        //INT op2_idx = TOP_Find_Operand_Use(OP_code(input_op),OU_opnd2);

        //if ((op1_idx >= 0) && (op2_idx >= 0)) {
	EBO_TN_INFO *tmp1_tninfo = opnd_opinfo->actual_opnd[0];
	EBO_TN_INFO *tmp2_tninfo = opnd_opinfo->actual_opnd[1];
	TN *tmp1_tn;
	TN *tmp2_tn;
	if (tmp1_tninfo != NULL) {
	  if ((tmp1_tninfo->replacement_tn) &&
	      (TN_is_symbol(tmp1_tninfo->replacement_tn) || TN_Is_Constant(tmp1_tninfo->replacement_tn))) {
	    tmp1_tn = tmp1_tninfo->replacement_tn;
	    tmp1_tninfo = tmp1_tninfo->replacement_tninfo;
	  } else {
	    tmp1_tn = tmp1_tninfo->local_tn;
	  }
	} else {
	  tmp1_tn = OP_opnd(input_op, 0);
	}

	if (tmp2_tninfo != NULL) {
	  if ((tmp2_tninfo->replacement_tn) &&
	      (TN_is_symbol(tmp2_tninfo->replacement_tn) || TN_Is_Constant(tmp2_tninfo->replacement_tn))) {
	    tmp2_tn = tmp2_tninfo->replacement_tn;
	    tmp2_tninfo = tmp2_tninfo->replacement_tninfo;
	  } else {
	    tmp2_tn = tmp2_tninfo->local_tn;
	  }
	} else {
	  tmp2_tn = OP_opnd(input_op, 1);
	}

	if (!TN_is_symbol(tmp1_tn) && TN_Is_Constant(tmp1_tn)) {
	  offset += TN_Value(tmp1_tn);
	  check_tninfo = tmp2_tninfo;
	  indx_tn = tmp2_tn;
	  keep_going = TRUE;
	} else if (!TN_is_symbol(tmp2_tn) && TN_Is_Constant(tmp2_tn)) {
	  offset += TN_Value(tmp2_tn);
	  check_tninfo = tmp1_tninfo;
	  indx_tn = tmp1_tn;
	  keep_going = TRUE;
	}
      } 
      else if (EBO_Trace_Data_Flow) {
        #pragma mips_frequency_hint NEVER
        fprintf(TFile,"%sNon-simple expression in symbolic compare opnd[1] ",
		EBO_trace_pfx);
        Print_OP_No_SrcLine(input_op);
      }
    }
  }

  *opnd_tninfo = check_tninfo;
  *opnd_tn = indx_tn;
  *opnd_offset = offset;
}

/* =====================================================================
 *    EBO_Special_Sequence
 *
 *    Look at an expression and it's inputs to identify special sequences
 *    that can be simplified.
 * =====================================================================
 */
BOOL 
EBO_Special_Sequence (
  OP *op,
  TN **opnd_tn,
  EBO_TN_INFO **opnd_tninfo
)
{
  TOP opcode = OP_code(op);

#if 0

  if (OP_iadd(op)) {
    return (shl_add_sequence ( op, opnd_tn, opnd_tninfo) ||
            iadd_special_case( op, opnd_tn, opnd_tninfo));
  }
  if (OP_store(op)) {
    return (store_sequence( op, opnd_tn, opnd_tninfo) ||
            sxt_sequence  ( op, TOP_Find_Operand_Use(OP_code(op),OU_storeval),
                            opnd_tn, opnd_tninfo));
  }
  if (OP_fadd(op)) {
    return (fadd_fmult    (op, opnd_tninfo));
  }
  if (OP_fsub(op)) {
    return (fadd_fmult    (op, opnd_tninfo));
  }
  if ((opcode == TOP_fneg) ||
      (opcode == TOP_fpneg)) {
    return (fneg_sequence (op, opnd_tn, opnd_tninfo));
  }
  if (OP_icmp(op)) {
    return (condition_optimization (op, opnd_tn, opnd_tninfo) ||
            compare_bit (op, opnd_tn, opnd_tninfo) ||
            sxt_sequence (op, TOP_Find_Operand_Use(OP_code(op),OU_opnd1),
                          opnd_tn, opnd_tninfo) ||
            sxt_sequence (op, TOP_Find_Operand_Use(OP_code(op),OU_opnd2),
                          opnd_tn, opnd_tninfo));
  }
  if ((opcode == TOP_sxt1)    ||
      (opcode == TOP_sxt2)    ||
      (opcode == TOP_sxt4)    ||
      (opcode == TOP_zxt1)    ||
      (opcode == TOP_zxt2)    ||
      (opcode == TOP_zxt4)    ||
      (opcode == TOP_dep)     ||
      (opcode == TOP_dep_z)   ||
      (opcode == TOP_extr)    ||
      (opcode == TOP_extr_u)  ||
      (opcode == TOP_shr_i)   ||
      (opcode == TOP_shr_i_u)) {
    return (sxt_sequence  (op, 1, opnd_tn, opnd_tninfo));
  }
  if (opcode == TOP_and_i) {
    return (sxt_sequence (op, 2, opnd_tn, opnd_tninfo));
  }
  if ((opcode == TOP_setf_sig) ||
      (opcode == TOP_getf_sig)) {
    return (copy_rf_sequence (op, opnd_tn, opnd_tninfo));
  }
  if (OP_effectively_copy(op)) {
    return (copy_simplification (op, opnd_tn, opnd_tninfo));
  }
  if (OP_access_reg_bank(op)) {
    return Process_Group_Register_Reference (op);
  }

#endif

  return FALSE;
}
