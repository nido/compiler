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
 *  $Revision$
 *  $Date$
 *  $Author$
 *  $Source$
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
#include <stdarg.h>
#include <stdlib.h>
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

#ifdef TARG_ST
#include "top_properties.h"
#endif

/* Define a macro to strip off any bits outside of the left most 4 bytes. */
#define TRUNC_32(val) (val & 0x00000000ffffffffll)

/* Define a macro to sign-extend the least signficant 32 bits */
#define SEXT_32(val) (((INT64)(val) << 32) >> 32)

/* ============================================================
 * opcode_benefit
 *
 * Returns true if it is beneficial to change opcode to new_opcode
 * Returns false otherwise
 *
 * Currently return true if:
 * 1. inline_extended_immediate is on (always replace)
 * 2. resources for new_opcode is not greater than for opcode
 * ============================================================
 */
static BOOL inline_extended_immediate = FALSE;
static BOOL
opcode_benefit(TOP new_opcode, TOP opcode)
{
  return inline_extended_immediate || 
    ISA_PACK_Inst_Words(new_opcode) <= ISA_PACK_Inst_Words(opcode);
}

/* ============================================================
 * 
 * Returns true if inlining the corresponding immediate def tninfo
 * into an operation is profitable.
 * For ST200 we consider that inlining an extended immediate into
 * its use site is profitable only is the number of use sites is
 * less or equal to 3.
 * Note that if inline_operand_profitable_threshold if zero
 * the extended immediate is always inlined.
 */
static INT inline_operand_profitable_threshold = 2;
static BOOL
inline_operand_profitable(OP *op, EBO_TN_INFO *tninfo)
{
  return inline_operand_profitable_threshold <= 0 ||
    tninfo->reference_count <= inline_operand_profitable_threshold;
}


/* =====================================================================
 *   EBO_Verify_Ops
 *
 * Verify that the generated op list can be emitted.
 * Should be called each time an op list in on the point to be
 * inserted into a basic block.
 *
 * The verifications are:
 * - Verify that all operands are assigned if in peep (after reg alloc).
 *
 * =====================================================================
 */
BOOL
EBO_Verify_Ops(OPS *ops)
{
  OP *op;
  /* Verify that in peep, no register is unallocated. */
  if (!EBO_in_peep) return TRUE;
  for (op = OPS_first(ops); op != NULL; op = OP_next(op)) {
    int i;
    for (i = 0; i < OP_opnds(op); i++) {
      TN *tn = OP_opnd(op, i);
      if (TN_is_register(tn) && TN_register(tn) == REGISTER_UNDEFINED)
	return FALSE;
    }
    for (i = 0; i < OP_results(op); i++) {
      TN *tn = OP_result(op, i);
      if (TN_is_register(tn) && TN_register(tn) == REGISTER_UNDEFINED)
	return FALSE;
    }
  }
  return TRUE;
}

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
  TOP opcode = OP_code(op);

  if (!OP_iadd(op) && !OP_isub(op) && !OP_ior(op)) {
    return FALSE;
  }
  if ((op == BB_entry_sp_adj_op(OP_bb(op))) ||
      (op == BB_exit_sp_adj_op(OP_bb(op)))) {
     return FALSE;
  }

  if ((OP_iadd(op) || OP_ior(op)) && TN_Is_Constant(OP_opnd(op,1))) {
    *index = 0;
    *immed = 1;
    return TRUE;
  }

  if (OP_isub(op) && TN_Is_Constant(OP_opnd(op,1))) {
    *index = 0;
    *immed = 1;
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

  return -1;
}
 
/* =====================================================================
 *   EBO_condition_redundant
 * =====================================================================
 */
BOOL
EBO_condition_redundant(
  OP *elim_op,
  EBO_TN_INFO **opnd_tninfo,
  EBO_OP_INFO *prev_opinfo,
  OPS *ops
)
{
  OP *prev_op = prev_opinfo->in_op;

  TOP p_opcode = OP_code(prev_op);
  TOP e_opcode = OP_code(elim_op);

  if ((p_opcode == e_opcode) ||
      ((p_opcode == TOP_cmpeq_r_r) && (e_opcode == TOP_cmpne_r_r))) {

    TN *pr0 = OP_result(prev_op,0);
    TN *er0 = OP_result(elim_op,0);
#if 0
    TN *prp = OP_opnd(prev_op,OP_PREDICATE_OPND);
    TN *erp = OP_opnd(elim_op,OP_PREDICATE_OPND);
#endif

    if (p_opcode != e_opcode) {
     /* We are dealing with complement compare instructions. */
      TN *save_tn = pr0;

#if 0
      if (((pr0 == True_TN) && (er0 == True_TN)) ||
          ((pr1 == True_TN) && (er1 == True_TN))) {
       /* We may be able to replace both compares with a single one. */

        if (prp != erp) goto can_not_combine_ops;

        for (INT i = 0; i < OP_results(prev_op); i++) {
          if (!TN_Is_Constant(OP_result(prev_op,i))) {
              EBO_TN_INFO *result_tninfo = prev_opinfo->actual_rslt[i];
            if ((result_tninfo == NULL) ||
                (result_tninfo->reference_count != 0)) goto can_not_combine_ops;
          }
        }

        OP *new_op = Dup_OP(elim_op);
        if (pr0 == True_TN) {
          Set_OP_result(new_op, 0, pr1);
        } else {
          Set_OP_result(new_op, 1, pr0);
        }
        if (EBO_in_loop) EBO_Copy_OP_omega (new_op, elim_op);
        OP_srcpos(new_op) = OP_srcpos(elim_op);
        BB_Insert_Op_After(OP_bb(elim_op), elim_op, new_op);

        if (EBO_Trace_Optimization) {
          #pragma mips_frequency_hint NEVER
          fprintf(TFile,"%sMerge complementery compare operations.\n",EBO_trace_pfx);
        }

        return TRUE;
      }

     /* Interchange names for the results so that the opcodes can 
      * be viewed as the same. */
      save_tn = pr0;
      pr0 = pr1;
      pr1 = save_tn;

#endif

      if (EBO_Trace_Optimization) {
        #pragma mips_frequency_hint NEVER
        fprintf(TFile,"%sCompare instructions are complements.\n",EBO_trace_pfx);
      }
    }

#if 0
    if ((prp != erp) ||
        (EBO_in_pre &&
         (OP_bb(prev_op) != OP_bb(elim_op)))) {
     /* Predicates are not the same.  This could be a problem. */
     /* If BBs are not the same we could get into trouble with */
     /* predicates after hyperblock formation.                 */
     /*                                                        */
     /* There are a few special cases that could be caught,    */
     /* but it doesn't seem worth the effort.                  */
      if (EBO_Trace_Optimization) {
        #pragma mips_frequency_hint NEVER
        fprintf(TFile,"%sPredicates are different for redundant compare.\n",EBO_trace_pfx);
      }

      goto can_not_combine_ops;
    }
#endif

#if 0
   /* The result registers may conflict, if they are being re-assigned. */
    if (((er0 != True_TN) && tn_registers_identical (er0, pr1)) ||
        ((er1 != True_TN) && tn_registers_identical (er1, pr0))) {

      if (EBO_Trace_Optimization) {
        #pragma mips_frequency_hint NEVER
        fprintf(TFile,"%sRegister re-use in redundant compare.\n",EBO_trace_pfx);
      }

      goto can_not_combine_ops;
    }
#endif

    /* 
     * But if the same registers are defined, we can just 
     * delete this OP. 
     */
    if (tn_registers_identical (er0, pr0)) {
      if (EBO_Trace_Optimization) {
        #pragma mips_frequency_hint NEVER
        fprintf(TFile,"%sDelete redundant compare operation.\n",EBO_trace_pfx);
      }

      return TRUE;
    }

#if 0
    if (er0 != True_TN) {
      if (pr0 != True_TN) {
        EBO_Copy_Predicate (er0, pr0, &ops);
      } else {
        EBO_Complement_Predicate (er0,pr1, &ops);
      }
    } 
    if (er1 != True_TN) {
      if (pr1 != True_TN) {
        EBO_Copy_Predicate (er1, pr1, &ops);
      } else {
        EBO_Complement_Predicate (er1,pr0, &ops);
      }
    }

    if (EBO_Trace_Optimization) {
      #pragma mips_frequency_hint NEVER
      fprintf(TFile,"%sRemove redundant compare operation.\n",EBO_trace_pfx);
    }
    return TRUE;
#endif
  }

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

#if 0


  OP *pred_op = opinfo->in_op;
  BB *bb = OP_bb(op);
  TOP opcode = OP_code(op);
  TOP new_opcode;
  TN *pred_result = OP_result(pred_op,0);
  TN *succ_result = OP_result(op,0);
  INT base_index = TOP_Find_Operand_Use(OP_code(op),OU_base);
  TN *base_tn;
  EBO_TN_INFO *base_tninfo;
  INT size_pred;
  INT size_succ;
  TN *r1;
  TN *r2;
  OPS ops = OPS_EMPTY;

  if (EBO_Trace_Data_Flow) {
    fprintf(TFile,"%s          OP in BB:%d    ",EBO_trace_pfx,BB_id(bb));
    Print_OP_No_SrcLine(op);
    fprintf(TFile," is adjacent to OP in BB:%d    ",BB_id(opinfo->in_bb));
    Print_OP_No_SrcLine(pred_op);
  }
  
  if (!OP_load(op) || !OP_load(pred_op) || 
      (opcode != OP_code(pred_op)) ||
      (OP_results(op) != 1) || 
      (OP_results(pred_op) != 1)) {
    return FALSE;
  }

  if ((Opt_Level < 2) && (bb != opinfo->in_bb)) {
   /* Global TN's aren't supported at low levels of optimization. */
    return FALSE;
  }

  size_pred = OP_Mem_Ref_Bytes(pred_op);
  size_succ = OP_Mem_Ref_Bytes(op);
  if (size_pred != size_succ) return FALSE;

  if (!EBO_in_peep &&
      (bb != opinfo->in_bb) &&
      !TN_Is_Constant(pred_result) &&
      has_assigned_reg(pred_result)) {
    if (EBO_Trace_Data_Flow) {
      fprintf(TFile,"%sShouldn't move dedicated register references across blocks.\n",
              EBO_trace_pfx);
    }
    return FALSE;
  }

  if (!has_assigned_reg(pred_result) ||
      !has_assigned_reg(succ_result) ||
      (TN_register_class(pred_result) != TN_register_class(succ_result)) ||
      ((TN_register(pred_result) & 1) == (TN_register(succ_result) & 1))) {
    if (EBO_Trace_Data_Flow) {
      fprintf(TFile,"%sParied loads requires odd/even registers.\n", EBO_trace_pfx);
    }
    return FALSE;
  }

  if ((opinfo->actual_rslt[0] == NULL) ||
      (opinfo->actual_rslt[0]->reference_count != 0) ||
      !EBO_tn_available (opinfo->in_bb, opinfo->actual_rslt[0])) {
    if (EBO_Trace_Data_Flow) {
      fprintf(TFile,"%sThe result of the predecessor load has already been used.\n", EBO_trace_pfx);
    }
    return FALSE;
  }

  switch (opcode) {
    case TOP_ldfs: new_opcode = TOP_ldfps; break;
    case TOP_ldfd: new_opcode = TOP_ldfpd; break;
    case TOP_ldf8: new_opcode = TOP_ldfp8; break;
    default: return FALSE;
  }

  if (offset_pred < offset_succ) {
    base_tn = OP_opnd(pred_op, base_index);
    base_tninfo = opinfo->actual_opnd[base_index];
    if (!TN_Is_Constant(base_tn) && !EBO_tn_available (opinfo->in_bb, base_tninfo)) {
      if (EBO_Trace_Data_Flow) {
        fprintf(TFile,"%sThe index TN of the predecessor load is not available.\n", EBO_trace_pfx);
      }
      return FALSE;
    }
    r1 = pred_result;
    r2 = succ_result;
  } else {
    base_tn = OP_opnd(op, base_index);
    base_tninfo = opnd_tninfo[base_index];
    r1 = succ_result;
    r2 = pred_result;
  }

/* NYI: The simulator does not yet support these instructions.  */
  if (EBO_Trace_Optimization) {
    #pragma mips_frequency_hint NEVER
    fprintf(TFile,"%sThe simulator does not yet support load-pair instructions.\n",EBO_trace_pfx);
  }
  return FALSE;

  Build_OP (new_opcode, r1, r2, OP_opnd(op,OP_PREDICATE_OPND),
            OP_opnd(op,1), OP_opnd(op,2), base_tn, &ops);

  if (!EBO_Verify_Ops(&ops)) return FALSE;
  Copy_WN_For_Memory_OP (OPS_first(&ops), op);
  OP_srcpos(OPS_first(&ops)) = OP_srcpos(op);
  if (EBO_in_loop) EBO_Set_OP_omega (OPS_first(&ops), opnd_tninfo[OP_PREDICATE_OPND],
                                     NULL, NULL, base_tninfo);
  BB_Insert_Ops(OP_bb(op), op, &ops, FALSE);

  remove_op (opinfo);
  OP_Change_To_Noop(pred_op);
  opinfo->in_op = NULL;
  opinfo->in_bb = NULL;

  if (EBO_Trace_Optimization) {
    #pragma mips_frequency_hint NEVER
    fprintf(TFile,"%sReplace adjacent load with load pair.\n",EBO_trace_pfx);
  }

  return TRUE;

#endif
}

/* =====================================================================
 *   EBO_replace_subset_load_with_extract
 *
 * For a Store - Load sequence where we have
 * 1. store at offset_pred of size_pred
 * 2. load at offset_succ of size_succ
 * 
 * Currently only handle the case where:
 * offset_pred == offset_succ == 0 and size_pred == size_succ
 * In this case we must emit the load extension.
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
  
  if (offset_pred == 0 && offset_pred == offset_succ &&
      size_pred == size_succ) {
    if (TN_size(succ_result) > size_succ) {
      TOP new_opcode = TOP_UNDEFINED;
      TN *tn1;
      TN *tn2;
      if (size_succ == 2) {
	new_opcode = TOP_is_unsign(OP_code(op)) ? TOP_zxth_r : TOP_sxth_r;
	tn1 = pred_result;
	tn2 = NULL;
      } else if (size_succ == 1) {
	if (TOP_is_unsign(OP_code(op))) {
	  new_opcode = TOP_and_i;
	  tn1 = pred_result;
	  tn2 = Gen_Literal_TN(0xff, 4);
	} else {
	  new_opcode = TOP_sxtb_r;
	  tn1 = pred_result;
	  tn2 = NULL;
	}
      }
      if (new_opcode == TOP_UNDEFINED) return FALSE;
      OP *new_op;
      new_op = Mk_OP(new_opcode, succ_result,tn1,tn2);
      OP_srcpos(new_op) = OP_srcpos(op);
      BB_Insert_Op_After(OP_bb(op), op, new_op);
      
      return TRUE;
      
    }
  }
  
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

  if ((size != 4) && OP_unsigned_ld(op)) {
    /* 
     * Does it make sense to use bit-extract sequence to simulate 
     * store-load sequence ?
     */
    return FALSE;
  }

  /* Use full word copy. */
  EBO_Exp_COPY(NULL, OP_result(op, 0), pred_result, &ops);
  if (!EBO_Verify_Ops(&ops)) return FALSE;

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
  INT size                 // size of load
)
{
  OPS ops = OPS_EMPTY;

  /* 
   * Unsigned Store - Load sequence for small integers will strip off 
   * sign bit. Does it make sense to use bit-extract sequence to 
   * simulate store-load sequence ?
   */
  if ((size < 4) && OP_unsigned_ld(op)) {
    if (EBO_Trace_Data_Flow) {
      fprintf(TFile,"%sUnsigned load strips a bit\n",
              EBO_trace_pfx);
    }
    return FALSE;
  }

  /* If load/stores are of the form ldw/stw (sym, base)
   * Compare sym should suffice. Why ?
   */
  if ((TN_is_symbol(pred_offset) && !TN_is_symbol(intervening_offset)) ||
      (TN_is_symbol(intervening_offset) && !TN_is_symbol(pred_offset))) {
    if (EBO_Trace_Data_Flow) {
      fprintf(TFile,"%sdifferent kinds of memory operation\n",
              EBO_trace_pfx);
    }
    return FALSE;
  }
  else if (TN_is_symbol(pred_offset)) {
    // Both must be symbols
    ST *pred_st = TN_var(pred_offset);
    ST *inrevening_st = TN_var(intervening_offset);
    OPS ops1 = OPS_EMPTY;

    if (pred_st == inrevening_st) {
      Build_OP(TOP_mov_r, OP_result(op, 0), intervening_result, &ops1);
    }
    else {
      Build_OP(TOP_mov_r, OP_result(op, 0), pred_result, &ops1);
    }
    Set_OP_copy(OPS_last(&ops1));
    OP_srcpos(OPS_last(&ops1)) = OP_srcpos(op);
    OPS_Append_Ops(&ops, &ops1);
  }
  else {
    // Both are base + immed offset
    FmtAssert(TN_has_value(pred_offset) && TN_has_value(intervening_offset),
	      ("immediate offsets are wrong"));

    /* Are the immediate offsets same ? */
    if (TN_value(pred_offset) != TN_value(intervening_offset)) {
      if (EBO_Trace_Data_Flow) {
	fprintf(TFile,"%sImmediate offsets are not equal\n",
              EBO_trace_pfx);
      }
      return FALSE;
    }

    /* 
     * Compare the base addresses. 
     * Arthur: if comparison is redundant it will be cleaned up.
     */
    TN *predicate = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch);

    Build_OP (TOP_cmpeq_r_b, predicate, pred_base, intervening_base, &ops);
    OP_srcpos(OPS_last(&ops)) = OP_srcpos(op);

    OPS ops1 = OPS_EMPTY;

    /* Copy the "address not equal value". */
    Build_OP(TOP_slct_r, 
	     OP_result(op, 0), 
	     predicate, 
	     intervening_result,
	     pred_result, 
	     &ops1);

    OP_srcpos(OPS_last(&ops1)) = OP_srcpos(op);
    OPS_Append_Ops(&ops, &ops1);
  }

  if (!EBO_Verify_Ops(&ops)) return FALSE;
  BB_Insert_Ops(OP_bb(op), op, &ops, FALSE);

  if (EBO_Trace_Optimization) {
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
  return immed;
}

static int TOP_shadd_amount(TOP opcode) {
  switch(opcode) {
  case TOP_sh1add_r:
  case TOP_sh1add_i:
  case TOP_sh1add_ii:
    return 1;
  case TOP_sh2add_r:
  case TOP_sh2add_i:
  case TOP_sh2add_ii:
    return 2;
  case TOP_sh3add_r:
  case TOP_sh3add_i:
  case TOP_sh3add_ii:
    return 3;
  case TOP_sh4add_r:
  case TOP_sh4add_i:
  case TOP_sh4add_ii:
    return 4;
  default:
    return 0;
  }
}

static BOOL TOP_is_shadd(TOP opcode) { return TOP_shadd_amount(opcode) > 0; }


/* =====================================================================
 *    EBO_simplify_operand0
 *
 *    Return a new_op if the opcode can be simplified to a
 *    immediate form.
 *    The operands tn0 and const_val are logically equivalent operands at index 
 *    OU_opnd1 and OU_opnd2 which may not be 0 and 1.
 *    To get the effective operands of the operation get it from op.
 *    In this function, if the operand 0 can't be inlined, we try to
 *    invert the operator and call EBO_simplify_operand1.
 * =====================================================================
 */
OP*
EBO_simplify_operand0 (
  OP *op,           // OP being simplified
  TN *tnr,          // result TN
  INT64 const_val,  // constant value of opnd TN 0
  TN *tn1           // opnd TN 1
)
{
  TOP opcode = OP_code(op);
  TOP new_opcode;
  OP *new_op;
  INT opnd1_idx, opnd2_idx;

  opnd1_idx = OP_find_opnd_use(op, OU_opnd1);
  opnd2_idx = OP_find_opnd_use(op, OU_opnd2);
  FmtAssert(opnd1_idx >= 0 && opnd2_idx >= 0, ("OU_opnd1 and/or OU_opnd2 not defined for TOP %s\n", TOP_Name(opcode)));

  if (EBO_Trace_Optimization) { 
    fprintf(TFile,"Enter EBO_simplify_operand0: %s ", TOP_Name(opcode));
    fprintf(TFile," %lld ", const_val);
    Print_TN(tn1,FALSE);
    fprintf(TFile," : tns ");
    Print_TN(OP_opnd(op, opnd1_idx),FALSE);
    fprintf(TFile," ");
    Print_TN(OP_opnd(op, opnd2_idx),FALSE);
    fprintf(TFile,"\n");
  }


  /* shadd -> add */
  if (TOP_is_shadd(opcode)) {
    TN *new_tn0, *new_tn1;
    int n = TOP_shadd_amount(opcode);
    INT64 new_val = SEXT_32(const_val << n);
    
    if (!TN_is_register(OP_opnd(op,opnd2_idx))) return NULL;
    new_opcode = TOP_add_r;
    new_tn0 = OP_opnd(op,opnd2_idx);
    if (new_val == 0) new_tn1 = Zero_TN;
    else new_tn1 = Gen_Literal_TN(new_val, 4);
    if (TN_has_value(new_tn1))
      new_opcode = TOP_opnd_immediate_variant(new_opcode, opnd2_idx, const_val);
    if (new_opcode == TOP_UNDEFINED) return NULL;
    new_op = Mk_OP(new_opcode, tnr, new_tn0, new_tn1);
    if (EBO_Trace_Optimization) fprintf(TFile,"shiftadd -> add\n");
    return new_op;
  }

  /* Inlining of immediate operand unless Zero_TN. */
  if (TN_is_register(OP_opnd(op, opnd1_idx)) &&
      OP_opnd(op, opnd1_idx) != Zero_TN) {
    new_opcode = TOP_opnd_immediate_variant(opcode, opnd1_idx, const_val);
    if (new_opcode != TOP_UNDEFINED && 
	new_opcode != opcode) {
      if (!opcode_benefit(new_opcode,opcode)) return NULL;
      new_op = Dup_OP(op);
      OP_Change_Opcode(new_op, new_opcode);
      Set_OP_opnd(new_op,opnd1_idx, Gen_Literal_TN(const_val, 4));
      if (EBO_Trace_Optimization) fprintf(TFile,"replace op %s with immediate form %s\n", TOP_Name(opcode), TOP_Name(new_opcode));
      return new_op;
    }
  }
  
  /* Invert operands and try to simplify operand 1. */
  if (TN_is_register(OP_opnd(op, opnd1_idx)) &&
      OP_opnd(op, opnd1_idx) != Zero_TN &&
      TN_is_register(OP_opnd(op, opnd2_idx))) {
    new_opcode = TOP_opnd_swapped_variant(opcode, opnd1_idx, opnd2_idx);
    if (new_opcode != TOP_UNDEFINED) {
      new_op = Dup_OP(op);
      OP_Change_Opcode(new_op, new_opcode);
      Set_OP_opnd(new_op, opnd1_idx, OP_opnd(op, opnd2_idx));
      Set_OP_opnd(new_op, opnd2_idx, OP_opnd(op, opnd1_idx));
      return EBO_simplify_operand1(new_op, tnr, tn1, const_val);
    }
  }

  return NULL;
}

/* =====================================================================
 *    EBO_simplify_operand1
 *
 *    Return a new_op if the opcode can be simplified to a
 *    immediate form.
 *    The operands tn0 and const_val are logically equivalent operands at index 
 *    OU_opnd1 and OU_opnd2 which may not be 0 and 1.
 *    To get the effective operands of the operation get it from op.
 * =====================================================================
 */
OP*
EBO_simplify_operand1 (
  OP *op,           // OP being simplified
  TN *tnr,          // result TN
  TN *tn0,          // opnd TN 0
  INT64 const_val   // constant value of opnd TN 1
)
{
  TOP opcode = OP_code(op);
  TOP new_opcode;
  OP *new_op;
  INT opnd1_idx, opnd2_idx;

  opnd1_idx = OP_find_opnd_use(op, OU_opnd1);
  opnd2_idx = OP_find_opnd_use(op, OU_opnd2);
  FmtAssert(opnd1_idx >= 0 && opnd2_idx >= 0, ("OU_opnd1 and/or OU_opnd2 not defined for TOP %s\n", TOP_Name(opcode)));

  if (EBO_Trace_Optimization) { 
    fprintf(TFile,"Enter EBO_simplify_operand1: %s ", TOP_Name(opcode));
    Print_TN(tn0,FALSE);
    fprintf(TFile," %lld ", const_val);
    fprintf(TFile,": tns ");
    Print_TN(OP_opnd(op, opnd1_idx),FALSE);
    fprintf(TFile," ");
    Print_TN(OP_opnd(op, opnd2_idx),FALSE);
    fprintf(TFile,"\n");
  }

  /* Inlining of immediate operand unless Zero_TN. */
  if (TN_is_register(OP_opnd(op, opnd2_idx)) && 
      OP_opnd(op, opnd2_idx) != Zero_TN) {
    new_opcode = TOP_opnd_immediate_variant(opcode, opnd2_idx, const_val);
    if (new_opcode != TOP_UNDEFINED && 
	new_opcode != opcode) {
      if (!opcode_benefit(new_opcode,opcode)) return NULL;
      new_op = Dup_OP(op);
      OP_Change_Opcode(new_op, new_opcode);
      Set_OP_opnd(new_op,opnd2_idx,Gen_Literal_TN(const_val, 4));
      if (EBO_Trace_Optimization) fprintf(TFile,"replace op %s with immediate form %s\n", TOP_Name(opcode), TOP_Name(new_opcode));
      return new_op;
    }
  }

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
      if (!EBO_Verify_Ops(&ops)) return FALSE;
      BB_Insert_Ops(OP_bb(op), op, &ops, FALSE);
      Unlink_Pred_Succ (bb, fall_bb);
      Change_Succ_Prob (bb, branch_bb, 1.0);

      return TRUE;;
    } 
  }
  return FALSE;
}

/* =====================================================================
 *    EBO_Fold_Special_Opcode
 * 
 *    Target-specific folding.
 *
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
  TN *tn0 = NULL, *tn1 = NULL;
  INT64 tn0_val, tn1_val;

  if (EBO_Trace_Optimization) {
    fprintf(TFile, "%sFold_Special_Opcode\n", EBO_trace_pfx);
  }

  /* [CG] We could handle constant symbols (such as floating point constant),
     but we don't. */
  if (OP_opnds(op) >= 1) {
    tn0 = opnd_tn[0];
    if (TN_Has_Value(opnd_tn[0])) tn0_val = TN_Value(opnd_tn[0]);
    else return FALSE;
  }
  if (OP_opnds(op) >= 2) {
    tn1 = opnd_tn[1];
    if (TN_Has_Value(opnd_tn[1])) tn1_val = TN_Value(opnd_tn[1]);
    else return FALSE;
  }

  return FALSE;

 Folded:

  if (EBO_Trace_Optimization) {
    fprintf(TFile, "%sfolded???: %llx\n", EBO_trace_pfx, *result_val);
  }
  return TRUE;
}

#if 0

/*
 * Function: sxt_sequence
 *
 * Look for sign extension operations that are not needed.
 *
 *	Translation Matrix 
 *             (RL is result length for sequence)
 *             (IL is input length for final operation)
 *             (ZXT is one of the following: and(mask), extr_u, zxt[124])
 *             (SXT is one of the following: extr, sxt[124])
 *             (EXTR_U is an instruction that performs an extr.u)
 *             (EXTR is an instruction that performs an extr)
 *
 * Note: shift_right is the same as an extr operation.
 *
 *  origional		      translated to, if ..
 *  sequence		  RL<IL		RL=IL	RL>IL
 * -----------		-----------	-----	-----`
 * ZXT(ZXT)		EXTR_U		EXTR_U	EXTR_U
 * ZXT(SXT)		EXTR_U		EXTR_U	Illegal
 * SXT(SXT)		EXTR		EXTR	EXTR
 * SXT(ZXT)		EXTR		EXTR	EXTR_U
 *
 * For the following, if ZXT/SXT is an extract, the 'pos' operand
 * must be zero.
 *
 * ld(ZXT)		Illegal		ld	ld
 * ld(SXT)		Illegal		Illegal	ld
 * cmp(SXT/ZXT)		cmp		cmp	Illegal
 * st(SXT/ZXT)		st		st	Illegal
 * dep(SXT/ZXT)		dep		dep	Illegal
 * dep_z(SXT/ZXT)	dep_z		dep_z	Illegal
 */
static
BOOL
sxt_sequence (OP *op,
              INT op_idx,
              TN **opnd_tn,
              EBO_TN_INFO **opnd_tninfo)
{
 /* Level 1 data: */
  TOP opcode = OP_code(op);
  BB *bb = OP_bb(op);
  INT sxt_idx = 1; /* TOP_Find_Operand_Use(OP_code(op),OU_opnd1); doesn't work */
  EBO_TN_INFO *sxt_tninfo;

  OP *pred_op;
  TOP pred_opcode;

  INT result_start = 0; /* assume the input is right justified in register. */
  INT result_length;
  INT input_start = 0; /* assume the input is right justified in register. */
  INT input_length;

  if (sxt_idx < 0) return FALSE;

  if (EBO_Trace_Execution) {
    #pragma mips_frequency_hint NEVER
    fprintf(TFile,"Enter sxt_sequence\n");
  }

  sxt_tninfo = opnd_tninfo[op_idx];

  if ((sxt_tninfo == NULL) ||
      (sxt_tninfo->in_op == NULL)) return FALSE;

 /* There are special requirements for "spill" temps
    that prevents their replacement. */
  if (OP_store(op)) {
    TN *store_value = OP_opnd(op,op_idx);
    if (!TN_Is_Constant(store_value) &&
        TN_has_spill(store_value)) {
      return FALSE;
    }
  }
 
  pred_op = sxt_tninfo->in_op;
  pred_opcode = OP_code(pred_op);

 /* What is the length of the result and input? */
  if ((pred_opcode == TOP_and) ||
      (pred_opcode == TOP_andcm)) {
      EBO_OP_INFO *and_opinfo = locate_opinfo_entry (sxt_tninfo);
      if ((and_opinfo == NULL) ||
          (and_opinfo->in_op == NULL)) return FALSE;
      EBO_TN_INFO *pred_info1 = and_opinfo->actual_opnd[1];
      EBO_TN_INFO *pred_info2 = and_opinfo->actual_opnd[2];
      TN *pred_op1;
      TN *pred_op2;
      TN *mask_tn;
      INT32 mask_size;
      INT32 mask_shift;

      if (pred_info1 != NULL) {
        if (pred_info1->replacement_tn != NULL) {
          pred_op1 = pred_info1->replacement_tn;
        } else {
          pred_op1 = pred_info1->local_tn;
        }
      } else {
        pred_op1 = OP_opnd(pred_op,1);
      }
      if (pred_info2 != NULL) {
        if (pred_info2->replacement_tn != NULL) {
          pred_op2 = pred_info2->replacement_tn;
        } else {
          pred_op2 = pred_info2->local_tn;
        }
      } else {
        pred_op2 = OP_opnd(pred_op,2);
      }

      if (TN_Is_Constant(pred_op1)) {
        mask_tn = pred_op1;
        mask_shift = Get_Mask_Shift_Count( TN_Value(mask_tn) );
        mask_size = Get_Right_Mask_Length( TN_Value(mask_tn) >> mask_shift);
      } else if (TN_Is_Constant(pred_op2)) {
        mask_tn = pred_op2;
        if ((pred_opcode == TOP_andcm) || (pred_opcode == TOP_andcm_i)) {
          mask_shift = Get_Mask_Shift_Count( ~TN_Value(mask_tn) );
          mask_size = Get_Right_Mask_Length( ~TN_Value(mask_tn) >> mask_shift);
        } else {
          mask_shift = Get_Mask_Shift_Count( TN_Value(mask_tn) );
          mask_size = Get_Right_Mask_Length( ((UINT64)TN_Value(mask_tn) >> mask_shift) );
        }
      } else return FALSE;
 
    input_start = mask_shift;
    input_length = mask_size;

    if (input_start != 0) return FALSE;

  } else {
    input_length = EBO_bit_length(pred_op);
  }

  result_length = EBO_bit_length (op);
  if ((result_length < 0) || (input_length < 0)) return FALSE;

 /* Determine start bit used by extr and shift instructions. */
  if ((opcode == TOP_extr)  || (opcode == TOP_extr_u) ||
      (opcode == TOP_shr_i) || (opcode == TOP_shr_i_u)) {
    result_start = TN_Value( OP_opnd(op,2) );
  }
  if ((pred_opcode == TOP_extr)  || (pred_opcode == TOP_extr_u) ||
      (pred_opcode == TOP_shr_i) || (pred_opcode == TOP_shr_i_u)) {
    input_start = TN_Value( OP_opnd(pred_op,2) );
  }
  if (pred_opcode == TOP_dep) {
    INT p_start = TN_Value( OP_opnd(pred_op,3) );
    if (((result_start + result_length) <= p_start) ||
        ((p_start + input_length) <= result_start)) {
     /* The OP does not access the field that was inserted by the dep OP. */
     /* Access the input to the dep and extract the data from it.         */
      input_start = 0;
      input_length = 64;
      sxt_idx = 2;
    } else if ((p_start <= result_start) &&
               ((result_start + result_length) <= (p_start + input_length))) {
     /* The result is contained within the input. */
      result_start = 0;
    } else if ((opcode == TOP_extr_u) &&
               (result_start == 0) &&
               ((p_start + input_length) <= result_length)) {
     /* The dep does not extend the size of it's input, but just replaces a field. */
     /* If the result of the extr is greater than or equal to the size of the input
        to the dep, it may not be necessary. */
      EBO_OP_INFO *dep_opinfo = locate_opinfo_entry (sxt_tninfo);
      if ((dep_opinfo == NULL) ||
          (dep_opinfo->in_op == NULL)) return FALSE;
      EBO_TN_INFO *dep_info1 = dep_opinfo->optimal_opnd[2];
      if ((dep_info1 != NULL) &&
          (dep_info1->in_op != NULL) &&
          (EBO_bit_length(dep_info1->in_op) <= result_length)) {
       /* The result of the OP is greater than or equal to the size of the input to the dep. */

        OPS ops = OPS_EMPTY;
        EBO_Exp_COPY(OP_opnd(op,OP_PREDICATE_OPND),
                     OP_result(op,0), OP_opnd(op, op_idx), &ops);

	if (!EBO_Verify_Ops(&ops)) return FALSE;
        OP_srcpos(OPS_first(&ops)) = OP_srcpos(op);
        if (EBO_in_loop) EBO_Set_OP_omega (OPS_first(&ops),
                                           opnd_tninfo[OP_PREDICATE_OPND],
                                           opnd_tninfo[op_idx]);
        BB_Insert_Ops(bb, op, &ops, FALSE);
        if (EBO_Trace_Optimization) {
          #pragma mips_frequency_hint NEVER
          fprintf(TFile,"%s of %s replaced with copy of input\n",
                        TOP_Name(opcode), TOP_Name(pred_opcode));
        }

      return TRUE;
      }

      return FALSE;
    } else {
     /* There is some sort of overlap of fields. */
      return FALSE;
    }
  }
  if (pred_opcode == TOP_dep_z) {
    INT p_start = TN_Value( OP_opnd(pred_op,2) );
    if (((result_start + result_length) <= p_start) ||
        ((p_start + input_length) <= result_start)) {
     /* The OP does not access the field that was inserted by the dep OP. */
     /* The result of the dep must be "0", so use Zero_TN in place of     */
     /* the original result TN of the dep.                                */

      OP *new_op = Dup_OP(op);
      Set_OP_opnd (new_op, op_idx, Zero_TN);

      if (OP_store(op)) {
        Copy_WN_For_Memory_OP (new_op, op);
      }

      if (EBO_in_loop) {
        EBO_Copy_OP_omega (new_op, op);
        Set_OP_omega (op, op_idx, 0);
      }

      BB_Insert_Op_After(bb, op, new_op);
      if (EBO_Trace_Optimization) {
        #pragma mips_frequency_hint NEVER
        fprintf(TFile,"OP does not access inserted field of dep_z.\n");
      }

      return TRUE;
    } else if ((p_start <= result_start) &&
               ((result_start + result_length) <= (p_start + input_length))) {
     /* The result is contained within the input. */
      result_start = 0;
    } else {
     /* There is some sort of overlap of fields. */
      return FALSE;
    }
  }

  BOOL input_zxt  = (pred_opcode == TOP_and_i)   ||
		    (pred_opcode == TOP_and)     ||
		    (pred_opcode == TOP_andcm)   ||
		    (pred_opcode == TOP_andcm_i) ||
		    (pred_opcode == TOP_dep)     ||
		    (pred_opcode == TOP_dep_z)   ||
		    (pred_opcode == TOP_extr_u)  ||
		    (pred_opcode == TOP_shr_i_u) ||
		    (pred_opcode == TOP_zxt1)    ||
		    (pred_opcode == TOP_zxt2)    ||
		    (pred_opcode == TOP_zxt4);
  BOOL input_sxt  = (pred_opcode == TOP_extr)   ||
		    (pred_opcode == TOP_shr_i)  ||
		    (pred_opcode == TOP_sxt1)   ||
		    (pred_opcode == TOP_sxt2)   ||
		    (pred_opcode == TOP_sxt4);
  BOOL result_zxt = (opcode == TOP_and_i)   ||
		    (opcode == TOP_extr_u)  ||
		    (opcode == TOP_shr_i_u) ||
		    (opcode == TOP_zxt1)    ||
		    (opcode == TOP_zxt2)    ||
		    (opcode == TOP_zxt4);
  BOOL result_sxt = (opcode == TOP_extr)   ||
		    (opcode == TOP_shr_i)  ||
		    (opcode == TOP_sxt1)   ||
		    (opcode == TOP_sxt2)   ||
		    (opcode == TOP_sxt4);

  /* Look for sequences that can be replaced by an extract operation.
   * These are basically ANDs of ANDs with some funny side effects
   * to complicate things.
   */
  if ((input_zxt || input_sxt) && (result_zxt || result_sxt)) {

    /* If the first inst of the pair is an extract, we remember that
     * the input is shifted to be right-justified so a final correction
     * can be applied later. Other than that correction, we treat it
     * as a plain AND.
     */
    INT input_shift = 0;
    if ((pred_opcode == TOP_extr_u)  || (pred_opcode == TOP_extr) ||
        (pred_opcode == TOP_dep_z)   || (pred_opcode == TOP_dep)  ||
        (pred_opcode == TOP_shr_i_u) || (pred_opcode == TOP_shr_i)) {
      input_shift = input_start;
      input_start = 0;
    }

    /* Compute the start and length of the replacement extract operation.
     */
    INT input_end = input_start + input_length;
    INT result_end = result_start + result_length;
    INT new_end = MIN(input_end, result_end);
    INT new_start = MAX(input_start, result_start);
    INT new_length = new_end - new_start;
    new_start += input_shift;

    if (new_length <= 0) {

      if (input_sxt && (result_start >= input_length)) return FALSE;

    /* If there is no overlap between the two ANDs so the result is 0! */
      OPS ops = OPS_EMPTY;
      EBO_Exp_COPY(OP_opnd(op,OP_PREDICATE_OPND),
                   OP_result(op,0), Zero_TN, &ops);

      if (!EBO_Verify_Ops(&ops)) return FALSE;
      if (EBO_in_loop) EBO_Set_OP_omega (OPS_first(&ops),
                                         opnd_tninfo[OP_PREDICATE_OPND],
                                         NULL);
      OP_srcpos(OPS_first(&ops)) = OP_srcpos(op);
      BB_Insert_Ops(bb, op, &ops, FALSE);
      if (EBO_Trace_Optimization) {
	#pragma mips_frequency_hint NEVER
	fprintf(TFile,"%s of %s replaced with copy of zero\n",
		      TOP_Name(opcode), TOP_Name(pred_opcode));
      }

      return TRUE;
    }

    /* Determine if the replacement extract is signed or unsigned.
     */
    TOP new_opc = TOP_extr_u;
    if (result_sxt) {
      if (input_sxt || result_end <= input_end) new_opc = TOP_extr;
    } else if (input_sxt) {
      if (result_end > input_end) return FALSE;
    }

    /* Be sure the input operand is available.
     */
    EBO_OP_INFO *and_opinfo = locate_opinfo_entry (sxt_tninfo);
    INT and_idx = (pred_opcode == TOP_and_i) ? 2 : sxt_idx;
    if ((and_opinfo == NULL) ||
        (and_opinfo->in_op == NULL) ||
        (and_opinfo->actual_opnd[and_idx] == NULL) ||
        (!EBO_tn_available(bb, and_opinfo->actual_opnd[and_idx]))) return FALSE;

    /* Generate the replacement extract and insert into the BB.
     */
    TN *src = OP_opnd(pred_op,and_idx);
    OP *new_op = Generate_Extract(new_opc, OP_result(op,0), OP_opnd(op,0),
				  src, new_start, new_length);
    INT src_idx = TN_Opernum_In_OP (new_op, src);
    new_opc = OP_code(new_op);
    OP_srcpos(new_op) = OP_srcpos(op);
    if (EBO_in_loop) {
     /* We need to determine where the src operand was inserted,
        since we do not know what instruction was generated. */
      OPS ops = OPS_EMPTY;
      OPS_Insert_Op (&ops, NULL, new_op, TRUE);
      EBO_TN_INFO *and_tninfo = and_opinfo->actual_opnd[and_idx];
      EBO_OPS_omega (&ops, opnd_tninfo[OP_PREDICATE_OPND]);
      Set_OP_omega (new_op, src_idx, (and_tninfo != NULL) ? and_tninfo->omega : 0);
    }
    BB_Insert_Op_After(OP_bb(op), op, new_op);

    if (EBO_Trace_Optimization) {
      #pragma mips_frequency_hint NEVER
      fprintf(TFile,"%s of %s replaced with %s.\n",
		    TOP_Name(opcode), TOP_Name(pred_opcode), TOP_Name(new_opc));
    }
    return TRUE;
  }  else if (OP_load(pred_op)) {
    // ???? handle start > length
    if (result_start == 0 &&
	((result_zxt && result_length >= input_length) ||
	 (result_sxt && result_length > input_length)))
    {
     /* The current instruction is not needed. */
      OPS ops = OPS_EMPTY;
      EBO_Exp_COPY(OP_opnd(op,OP_PREDICATE_OPND),
                   OP_result(op,0), opnd_tn[op_idx], &ops);

      if (!EBO_Verify_Ops(&ops)) return FALSE;
      if (EBO_in_loop) EBO_Set_OP_omega (OPS_first(&ops),
                                         opnd_tninfo[OP_PREDICATE_OPND],
                                         opnd_tninfo[op_idx]);
      OP_srcpos(OPS_first(&ops)) = OP_srcpos(op);
      BB_Insert_Ops(bb, op, &ops, FALSE);
      if (EBO_Trace_Optimization) {
	#pragma mips_frequency_hint NEVER
	fprintf(TFile,"skip %s after load operation\n", TOP_Name(opcode));
      }

      return TRUE;
    }
  }  else if (OP_store(op) || OP_icmp(op) || 
	      opcode == TOP_dep || opcode == TOP_dep_z)
  {
    if (input_start == 0 && input_length >= result_length) {
     /* The previous sxt/zxt instruction is not needed for
        the correct execution of this instruction. Bypass it. */

     /* Be sure the input operand is available. */
      EBO_OP_INFO *sxt_opinfo = locate_opinfo_entry (sxt_tninfo);
      if ((sxt_opinfo == NULL) ||
          (sxt_opinfo->in_op == NULL) ||
          (sxt_opinfo->actual_opnd[sxt_idx] == NULL) ||
          (!EBO_tn_available(bb, sxt_opinfo->actual_opnd[sxt_idx]))) return FALSE;

     /* Copy the original instruction and replace the operand. */
      OP *new_op = Dup_OP (op);
      Set_OP_opnd (new_op, op_idx, OP_opnd(pred_op,sxt_idx));
      OP_srcpos(new_op) = OP_srcpos(op);
      if (OP_store(op)) {
        Copy_WN_For_Memory_OP (new_op, op);
        if (EBO_in_loop) {
          if (OP_opnds(op) == 5) {
            EBO_Set_OP_omega (new_op, opnd_tninfo[OP_PREDICATE_OPND],
                              NULL, NULL, NULL, sxt_opinfo->actual_opnd[sxt_idx]);
          } else if (OP_opnds(op) == 6) {
           /* Incremented stores - the last operand is a constant */
            EBO_Set_OP_omega (new_op, opnd_tninfo[OP_PREDICATE_OPND],
                              NULL, NULL, NULL, sxt_opinfo->actual_opnd[sxt_idx], NULL);
          } else return FALSE;
        }
      } else if (EBO_in_loop) {
        EBO_TN_INFO *sxt_tninfo = sxt_opinfo->actual_opnd[sxt_idx];
        EBO_TN_INFO *pred_tninfo = opnd_tninfo[OP_PREDICATE_OPND];
	if (OP_opnds(op) == 2) {
	  // compare with zero
	  EBO_Set_OP_omega (new_op, pred_tninfo, sxt_tninfo);
	} else if ((OP_opnds(op) == 3) && (op_idx == 1)) { 
	  // compare
	  EBO_Set_OP_omega (new_op, pred_tninfo, sxt_tninfo, opnd_tninfo[2]);
	} else if ((OP_opnds(op) == 3) && (op_idx == 2)) {
	  // compare
	  EBO_Set_OP_omega (new_op, pred_tninfo, opnd_tninfo[1], sxt_tninfo);
	} else if ((OP_opnds(op) == 4) && (op_idx == 1)) {
	  // dep_z
	  EBO_Set_OP_omega (new_op, pred_tninfo, sxt_tninfo, opnd_tninfo[2],
				    opnd_tninfo[3]);
        } else if ((OP_opnds(op) == 5) && (op_idx == 1)) {
	  // dep
	  EBO_Set_OP_omega (new_op, pred_tninfo, sxt_tninfo, opnd_tninfo[2],
				    opnd_tninfo[3], opnd_tninfo[4]);
	} else return FALSE;
      }
      BB_Insert_Op_After(OP_bb(op), op, new_op);
      if (EBO_Trace_Optimization) {
	#pragma mips_frequency_hint NEVER
	fprintf(TFile,"bypass %s before %s operation\n", 
		      TOP_Name(pred_opcode), TOP_Name(opcode));
      }

      return TRUE;
    }
  }

  return FALSE;
}
#endif

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

#if 0

/* =====================================================================
 *    condition_optimization
 * =====================================================================
 */
static BOOL
condition_optimization(
  OP *op,
  TN **opnd_tn,
  EBO_TN_INFO **opnd_tninfo
)
{
  INT op1_idx = TOP_Find_Operand_Use(OP_code(op),OU_opnd1);
  INT op2_idx = TOP_Find_Operand_Use(OP_code(op),OU_opnd2);
  TN *op1 = opnd_tn[op1_idx];
  TN *op2 = opnd_tn[op2_idx];

  EBO_TN_INFO *op1_tninfo = opnd_tninfo[op1_idx];
  EBO_TN_INFO *op2_tninfo = opnd_tninfo[op2_idx];

  INT64 op1_offset = TN_Is_Constant(op1) ? TN_value(op1) : 0;
  INT64 op2_offset = TN_Is_Constant(op2) ? TN_value(op2) : 0;

/* Can the result of this comparison be predicted? */
  if ((op1 == op2) && (op1 != Zero_TN)) {
    OPS ops = OPS_EMPTY;

    TOP opcode = OP_code(op);
    TN *r0 = OP_result(op,0);
    TN *r1 = OP_result(op,1);
    TN *prd = OP_opnd(op,OP_PREDICATE_OPND);

   /* Replace the original operands with constants and allow
      it to be resolved by constant expression evaluation. */
    Build_OP (opcode, r0, True_TN, prd, Zero_TN, Zero_TN, &ops);
    OP_srcpos(OPS_last(&ops)) = OP_srcpos(op);
    if (OP_glue(op)) Set_OP_glue(OPS_first(&ops));
    Build_OP (opcode, True_TN, r1, prd, Zero_TN, Zero_TN, &ops);

    if (!EBO_Verify_Ops(&ops)) return FALSE;
    OP_srcpos(OPS_last(&ops)) = OP_srcpos(op);
    if (OP_glue(op)) Set_OP_glue(OPS_first(&ops));
    if (EBO_in_loop) EBO_OPS_omega (&ops, opnd_tninfo[OP_PREDICATE_OPND]);
    BB_Insert_Ops(OP_bb(op), op, &ops, FALSE);

    if (EBO_Trace_Optimization) {
      #pragma mips_frequency_hint NEVER
      fprintf(TFile,"%sResolve compare symbolically.\n",EBO_trace_pfx);
    }

    return TRUE;
  }

  EBO_Identify_Base_and_Offset ( &op1_tninfo, &op1, &op1_offset);
  EBO_Identify_Base_and_Offset ( &op2_tninfo, &op2, &op2_offset);

  if ((op1_tninfo == op2_tninfo) && (op1_tninfo != NULL) && (op1 != Zero_TN)) {

    if (EBO_Trace_Optimization) {
      #pragma mips_frequency_hint NEVER
      fprintf(TFile,"%sResolve compare symbolically by reducing to constants %lld : %lld.\n",
              EBO_trace_pfx,op1_offset,op2_offset);
    }

    TN *new_tn[OP_MAX_FIXED_OPNDS];
    TN **new_opnd_tn = &new_tn[0];
    EBO_TN_INFO *new_tninfo[OP_MAX_FIXED_OPNDS];
    EBO_TN_INFO **new_opnd_tninfo = &new_tninfo[0];
    for (INT i = 0; i < OP_opnds(op); i++) {
      new_tn[i] = NULL;
      new_tninfo[i] = NULL;
    }
    new_tn[OP_PREDICATE_OPND] = OP_opnd(op,OP_PREDICATE_OPND);
    new_tninfo[OP_PREDICATE_OPND] = opnd_tninfo[OP_PREDICATE_OPND];
    new_tn[op1_idx] = Gen_Literal_TN(op1_offset, 8);
    new_tn[op2_idx] = Gen_Literal_TN(op2_offset, 8);
    return Fold_Constant_Expression (op, new_opnd_tn, new_opnd_tninfo);

    return TRUE;
  }

  TOP opcode = OP_code(op);
  BB *bb = OP_bb(op);

  if ((opcode == TOP_cmp_eq) ||
      (opcode == TOP_cmp_eq_unc) ||
      (opcode == TOP_cmp_ne) ||
      (opcode == TOP_cmp_ne_unc) ||
      (opcode == TOP_cmp_ge) ||
      (opcode == TOP_cmp_ge_unc) ||
      (opcode == TOP_cmp_geu) ||
      (opcode == TOP_cmp_geu_unc) ||
      (opcode == TOP_cmp_lt) ||
      (opcode == TOP_cmp_lt_unc) ||
      (opcode == TOP_cmp_ltu) ||
      (opcode == TOP_cmp_ltu_unc) ||
      (opcode == TOP_cmp_gt) ||
      (opcode == TOP_cmp_gt_unc) ||
      (opcode == TOP_cmp_gtu) ||
      (opcode == TOP_cmp_gtu_unc) ||
      (opcode == TOP_cmp_le) ||
      (opcode == TOP_cmp_le_unc) ||
      (opcode == TOP_cmp_leu) ||
      (opcode == TOP_cmp_leu_unc) ||
      (opcode == TOP_cmp_i_eq) ||
      (opcode == TOP_cmp_i_eq_unc) ||
      (opcode == TOP_cmp_i_ne) ||
      (opcode == TOP_cmp_i_ne_unc) ||
      (opcode == TOP_cmp_i_ge) ||
      (opcode == TOP_cmp_i_ge_unc) ||
      (opcode == TOP_cmp_i_geu) ||
      (opcode == TOP_cmp_i_geu_unc) ||
      (opcode == TOP_cmp_i_lt) ||
      (opcode == TOP_cmp_i_lt_unc) ||
      (opcode == TOP_cmp_i_ltu) ||
      (opcode == TOP_cmp_i_ltu_unc) ||
      (opcode == TOP_cmp_i_gt) ||
      (opcode == TOP_cmp_i_gt_unc) ||
      (opcode == TOP_cmp_i_gtu) ||
      (opcode == TOP_cmp_i_gtu_unc) ||
      (opcode == TOP_cmp_i_le) ||
      (opcode == TOP_cmp_i_le_unc) ||
      (opcode == TOP_cmp_i_leu) ||
      (opcode == TOP_cmp_i_leu_unc)) {

   /* Level 2 data: */
    OP *l2_op0;
    TOP l2_opcode0 = TOP_UNDEFINED;
    TN *l2_tn0 = opnd_tn[1];
    INT64 l2_val0;
    EBO_TN_INFO *l2_tninfo0 = opnd_tninfo[1];
    EBO_OP_INFO *l2_opinfo0;
    OP *l2_op1;
    TOP l2_opcode1 = TOP_UNDEFINED;
    TN *l2_tn1 = opnd_tn[2];
    INT64 l2_val1;
    EBO_TN_INFO *l2_tninfo1 = opnd_tninfo[2];
    EBO_OP_INFO *l2_opinfo1;

   /* Level 3 data: */
    TOP new_opcode;
    TN *l3_tn0;
    EBO_TN_INFO *l3_tninfo0;
    TN *l3_tn1;
    EBO_TN_INFO *l3_tninfo1;

    if ((l2_tninfo1 != NULL) &&
        (l2_tninfo1->in_op != NULL)) {
      l2_op1 = l2_tninfo1->in_op;
      l2_opcode1 = OP_code(l2_op1);

      if ((l2_opcode1 != TOP_sxt4) &&
          (l2_opcode1 != TOP_zxt4)) return FALSE;

      l2_opinfo1 = locate_opinfo_entry (l2_tninfo1);
      if (l2_opinfo1 == NULL) return FALSE;
      if (l2_opinfo1->actual_opnd[1] == NULL) return FALSE;
      if (!EBO_tn_available(bb, l2_opinfo1->actual_opnd[1])) return FALSE;

      l3_tn1 = OP_opnd(l2_op1,1);
      l3_tninfo1 = l2_opinfo1->actual_opnd[1];
    } else if (TN_Is_Constant(l2_tn1)) {
      l2_val1 = TN_Value(l2_tn1);
      l3_tn1 = l2_tn1;
      l3_tninfo1 = NULL;
    } else return FALSE;

    if ((l2_tninfo0 != NULL) &&
        (l2_tninfo0->in_op != NULL)) {
      l2_op0 = l2_tninfo0->in_op;
      l2_opcode0 = OP_code(l2_op0);

      if ((l2_opcode0 != TOP_sxt4) &&
          (l2_opcode0 != TOP_zxt4)) return FALSE;

      l2_opinfo0 = locate_opinfo_entry (l2_tninfo0);
      if (l2_opinfo0 == NULL) return FALSE;
      if (l2_opinfo0->actual_opnd[1] == NULL) return FALSE;
      if (!EBO_tn_available(bb, l2_opinfo0->actual_opnd[1])) return FALSE;

      l3_tn0 = OP_opnd(l2_op0,1);
      l3_tninfo0 = l2_opinfo0->actual_opnd[1];
    } else if (TN_Is_Constant(l2_tn0)) {
      l2_val0 = TN_Value(l2_tn0);
      l3_tn0 = l2_tn0;
      l3_tninfo0 = NULL;
    } else return FALSE;

   /* Both constants? The OP should have been evaluated. */
    if (TN_Is_Constant(l3_tn0) && TN_Is_Constant(l3_tn1)) return FALSE;

   /* Conversion instructions not the same? One input must be a constant. */
    if ((l2_op0 != l2_op1) &&
        (!TN_Is_Constant(l3_tn0)) &&
        (!TN_Is_Constant(l3_tn1))) return FALSE;

   /* One input a constant? It's sign must agree with the conversion OP. */
    if (TN_Is_Constant(l3_tn0)) {
      if ((l2_opcode1 == TOP_sxt4) &&
          (SEXT_32(l2_val0) != l2_val0)) return FALSE;
      if ((l2_opcode1 == TOP_zxt4) &&
          (TRUNC_32(l2_val0) != l2_val0)) return FALSE;
    }

    if (TN_Is_Constant(l3_tn1)) {
      if ((l2_opcode0 == TOP_sxt4) &&
          (SEXT_32(l2_val1) != l2_val1)) return FALSE;
      if ((l2_opcode0 == TOP_zxt4) &&
          (TRUNC_32(l2_val1) != l2_val1)) return FALSE;
    }

   /* Recreate the compare using 32 bit inputs. */
    switch (opcode) {
    case TOP_cmp_eq: new_opcode = TOP_cmp4_eq; break;
    case TOP_cmp_eq_unc: new_opcode = TOP_cmp4_eq_unc; break;
    case TOP_cmp_ne: new_opcode = TOP_cmp4_ne; break;
    case TOP_cmp_ne_unc: new_opcode = TOP_cmp4_ne_unc; break;
    case TOP_cmp_ge: new_opcode = TOP_cmp4_ge; break;
    case TOP_cmp_ge_unc: new_opcode = TOP_cmp4_ge_unc; break;
    case TOP_cmp_geu: new_opcode = TOP_cmp4_geu; break;
    case TOP_cmp_geu_unc: new_opcode = TOP_cmp4_geu_unc; break;
    case TOP_cmp_lt: new_opcode = TOP_cmp4_lt; break;
    case TOP_cmp_lt_unc: new_opcode = TOP_cmp4_lt_unc; break;
    case TOP_cmp_ltu: new_opcode = TOP_cmp4_ltu; break;
    case TOP_cmp_ltu_unc: new_opcode = TOP_cmp4_ltu_unc; break;
    case TOP_cmp_gt: new_opcode = TOP_cmp4_gt; break;
    case TOP_cmp_gt_unc: new_opcode = TOP_cmp4_gt_unc; break;
    case TOP_cmp_gtu: new_opcode = TOP_cmp4_gtu; break;
    case TOP_cmp_gtu_unc: new_opcode = TOP_cmp4_gtu_unc; break;
    case TOP_cmp_le: new_opcode = TOP_cmp4_le; break;
    case TOP_cmp_le_unc: new_opcode = TOP_cmp4_le_unc; break;
    case TOP_cmp_leu: new_opcode = TOP_cmp4_leu; break;
    case TOP_cmp_leu_unc: new_opcode = TOP_cmp4_leu_unc; break;
    case TOP_cmp_i_eq: new_opcode = TOP_cmp4_i_eq; break;
    case TOP_cmp_i_eq_unc: new_opcode = TOP_cmp4_i_eq_unc; break;
    case TOP_cmp_i_ne: new_opcode = TOP_cmp4_i_ne; break;
    case TOP_cmp_i_ne_unc: new_opcode = TOP_cmp4_i_ne_unc; break;
    case TOP_cmp_i_ge: new_opcode = TOP_cmp4_i_ge; break;
    case TOP_cmp_i_ge_unc: new_opcode = TOP_cmp4_i_ge_unc; break;
    case TOP_cmp_i_geu: new_opcode = TOP_cmp4_i_geu; break;
    case TOP_cmp_i_geu_unc: new_opcode = TOP_cmp4_i_geu_unc; break;
    case TOP_cmp_i_lt: new_opcode = TOP_cmp4_i_lt; break;
    case TOP_cmp_i_lt_unc: new_opcode = TOP_cmp4_i_lt_unc; break;
    case TOP_cmp_i_ltu: new_opcode = TOP_cmp4_i_ltu; break;
    case TOP_cmp_i_ltu_unc: new_opcode = TOP_cmp4_i_ltu_unc; break;
    case TOP_cmp_i_gt: new_opcode = TOP_cmp4_i_gt; break;
    case TOP_cmp_i_gt_unc: new_opcode = TOP_cmp4_i_gt_unc; break;
    case TOP_cmp_i_gtu: new_opcode = TOP_cmp4_i_gtu; break;
    case TOP_cmp_i_gtu_unc: new_opcode = TOP_cmp4_i_gtu_unc; break;
    case TOP_cmp_i_le: new_opcode = TOP_cmp4_i_le; break;
    case TOP_cmp_i_le_unc: new_opcode = TOP_cmp4_i_le_unc; break;
    case TOP_cmp_i_leu: new_opcode = TOP_cmp4_i_leu; break;
    case TOP_cmp_i_leu_unc: new_opcode = TOP_cmp4_i_leu_unc; break;
    }

    OPS ops = OPS_EMPTY;
if (EBO_Trace_Optimization) fprintf(TFile,"replace 64 bit compare with 32 bit compare\n");
    Build_OP (new_opcode,
              OP_result(op,0), OP_result(op,1), OP_opnd(op,OP_PREDICATE_OPND),
              l3_tn0, l3_tn1, &ops);

    if (!EBO_Verify_Ops(&ops)) return FALSE;
    if (EBO_in_loop) EBO_Set_OP_omega (OPS_first(&ops), opnd_tninfo[OP_PREDICATE_OPND],
                                       l3_tninfo0, l3_tninfo1);
    OP_srcpos(OPS_first(&ops)) = OP_srcpos(op);
    BB_Insert_Ops(bb, op, &ops, FALSE);
    return TRUE;
  }

  return FALSE;
}

 


/*
 * Function: compare_bit
 *
 * Look for the pattern:
 *       CMP(zero,AND(integer0,(1-bit-constant)))
 * and turn it into:
 *       TBIT_Z(trailing_zero_count(1-bit-constant))
 *
 */
static
BOOL
compare_bit (OP *op,
             TN **opnd_tn,
             EBO_TN_INFO **opnd_tninfo)
{
 /* Level 1 data: */
  TOP opcode = OP_code(op);
  BB *bb = OP_bb(op);
  OP *l1_op0 = op;

 /* Level 2 data: */
  TN *l2_tn0;
  INT64 l2_val0;
  OP *l2_op1;
  TN *l2_tn1;
  EBO_TN_INFO *l2_tninfo1;
  EBO_OP_INFO *l2_opinfo1;

 /* Level 3 data: */
  OP *l3_op0;
  TN *l3_tn0;
  EBO_TN_INFO *l3_tninfo0;
  EBO_OP_INFO *l3_opinfo0;
  TN *l3_tn1;
  EBO_TN_INFO *l3_tninfo1;
  EBO_OP_INFO *l3_opinfo1;
  INT64 l3_value;

 /* Scratch data */
  TN *save_tn;

 /* New instruction information. */
  TOP new_opcode = TOP_tbit_z;
  TN *r_true  = OP_result(op,0);
  TN *r_false = OP_result(op,1);
  TN *test_word;
  EBO_TN_INFO *test_word_tninfo;
  INT bit_number;

  l2_tn0 = OP_opnd(op,1);
  if ((l2_tn0 == NULL) || !TN_Is_Constant(l2_tn0)) return FALSE;
  l2_val0 = TN_Value(l2_tn0);
  if (l2_val0 != 0) {
    if (l2_val0 != 1) return FALSE;

   /* Reverse the results and keep zero-bit test. */
    save_tn = r_true;
    r_true = r_false;
    r_false = save_tn;
  }

  switch (opcode) {

    case TOP_cmp_i_eq_unc:
    case TOP_cmp4_i_eq_unc:
      new_opcode = TOP_tbit_z_unc;
    case TOP_cmp_i_eq:
    case TOP_cmp4_i_eq:
      break;

    case TOP_cmp_i_ne_unc:
    case TOP_cmp4_i_ne_unc:
      new_opcode = TOP_tbit_z_unc;
    case TOP_cmp_i_ne:
    case TOP_cmp4_i_ne:
      save_tn = r_true;
      r_true = r_false;
      r_false = save_tn;
      break;

    default: return FALSE;
  }

 /* Get the other operand of the comparison. */
  l2_tn1 = OP_opnd(l1_op0,2);
  l2_tninfo1 = opnd_tninfo[2];
  if ((l2_tninfo1 == NULL) || (l2_tninfo1->in_op == NULL)) return FALSE;
  l2_op1 = l2_tninfo1->in_op;

 /* How many differend ways to test a bit do we want to look for?
      AND(var,(1-bit_mask))
      AND(right_shift(var,count),"1")
      AND(extr(var,count,length),"1")
      right_shift(var,63)
      extr(var,bitnum)
      ???
 */

  if ((OP_code(l2_op1) == TOP_and) ||
      (OP_code(l2_op1) == TOP_and_i)) {

    l2_opinfo1 = locate_opinfo_entry (l2_tninfo1);
    if (l2_opinfo1 == NULL) {
      return FALSE;
    }

   /* The input to the AND must be a 1 bit constant. */
    if ((l2_opinfo1->optimal_opnd[1] != NULL) &&
        (l2_opinfo1->optimal_opnd[1]->replacement_tn != NULL) &&
        (TN_Is_Constant(l2_opinfo1->optimal_opnd[1]->replacement_tn))) {
      l3_tn0 = l2_opinfo1->optimal_opnd[1]->replacement_tn;
      l3_tninfo0 = NULL;
    } else {
      l3_tn0 = OP_opnd(l2_op1,1);
      l3_tninfo0 = l2_opinfo1->actual_opnd[1];
      if ((l3_tninfo0 != NULL) &&
          (l3_tninfo0->replacement_tn != NULL) &&
          (TN_Is_Constant(l3_tninfo0->replacement_tn))) {
        l3_tn0 = l3_tninfo0->replacement_tn;
        l3_tninfo0 = l3_tninfo0->replacement_tninfo;
      }
    }
    if ((l2_opinfo1->optimal_opnd[2] != NULL) &&
        (l2_opinfo1->optimal_opnd[2]->replacement_tn != NULL) &&
        (TN_Is_Constant(l2_opinfo1->optimal_opnd[2]->replacement_tn))) {
      l3_tn1 = l2_opinfo1->optimal_opnd[2]->replacement_tn;
      l3_tninfo1 = NULL;
    } else {
      l3_tn1 = OP_opnd(l2_op1,2);
      l3_tninfo1 = l2_opinfo1->actual_opnd[2];
      if ((l3_tninfo1 != NULL) &&
          (l3_tninfo1->replacement_tn != NULL) &&
          (TN_Is_Constant(l3_tninfo1->replacement_tn))) {
        l3_tn1 = l3_tninfo1->replacement_tn;
        l3_tninfo1 = l3_tninfo1->replacement_tninfo;
      }
    }

    if ((l3_tn0 != NULL) && TN_Is_Constant(l3_tn0)) {
      if ((l3_tninfo1 == NULL) ||
          TN_is_symbol(l3_tn0)) {
        return FALSE;
      }
      l3_value = TN_Value(l3_tn0);
      l3_tn0 = l3_tn1;
      l3_tninfo0 = l3_tninfo1;
    } else if ((l3_tn1 != NULL) && TN_Is_Constant(l3_tn1)) {
      if ((l3_tninfo0 == NULL) ||
          TN_is_symbol(l3_tn1)) {
        return FALSE;
      }
      l3_value = TN_Value(l3_tn1);
    } else {
      return FALSE;
    }

    if ((l3_value == 1) &&
        (l3_tninfo0 != NULL) &&
        (l3_tninfo0->in_op != NULL)) {
      OP *l3_op = l3_tninfo0->in_op;
      TOP l3_opcode = OP_code(l3_op);

      if ((l3_opcode == TOP_shr_i) ||
          (l3_opcode == TOP_shr_i_u)) {

        l3_opinfo1 = find_opinfo_entry (l3_op);
        if ((l3_opinfo1 != NULL) &&
            (l3_opinfo1->actual_opnd[1] != NULL) &&
            (EBO_tn_available(bb, l3_opinfo1->actual_opnd[1]))) {
          l3_tn0 = OP_opnd(l3_op, 1);
          l3_tninfo0 = l3_opinfo1->actual_opnd[1];
          l3_value = 1 << TN_Value(OP_opnd(l3_op, 2));
        }
      } else if ((l3_opcode == TOP_extr) ||
                 (l3_opcode == TOP_extr_u)) {
        l3_opinfo1 = find_opinfo_entry (l3_op);
        if ((l3_opinfo1 != NULL) &&
            (l3_opinfo1->actual_opnd[1] != NULL) &&
            (EBO_tn_available(bb, l3_opinfo1->actual_opnd[1])) &&
            (EBO_bit_length(l3_op) >= 1)) {
          l3_tn0 = OP_opnd(l3_op, 1);
          l3_tninfo0 = l3_opinfo1->actual_opnd[1];
          l3_value = 1 << TN_Value(OP_opnd(l3_op, 2));
        }
      }

    }

   /* Is the constant a power of 2? */
    if ((l3_value <= 0) || ((l3_value & (l3_value-1)) != 0)) {
      return FALSE;
    }

   /* Determine the power of 2 that we are dealing with. */
    INT64 one= 1;
    for (bit_number = 0; bit_number < 64; bit_number++)
    {
      if ((one << bit_number) == l3_value)
        break;
    }

    test_word = l3_tn0;
    test_word_tninfo = l3_tninfo0;
  } else if ((OP_code(l2_op1) == TOP_shr_i) &&
             (TN_Is_Constant(OP_opnd(l2_op1,2))) &&
             (TN_Value(OP_opnd(l2_op1,2)) == 63)) {

    l2_opinfo1 = locate_opinfo_entry (l2_tninfo1);
    if (l2_opinfo1 == NULL) return FALSE;
    if (l2_opinfo1->actual_opnd[1] == NULL) return FALSE;

   /* Set up to test the left most bit. */
    bit_number = 63;
    test_word = OP_opnd(l2_op1,1);
    test_word_tninfo = l2_opinfo1->actual_opnd[1];

   /* When the input to the shift-right is a shift-left,
      we may be able to do even better. */
    if ((test_word_tninfo != NULL) &&
        (test_word_tninfo->in_op != NULL) &&
        (OP_code(test_word_tninfo->in_op) == TOP_shl_i) &&
        (TN_Is_Constant(OP_opnd(test_word_tninfo->in_op,2)))) {
      l3_op0 = test_word_tninfo->in_op;
      l3_opinfo0 = locate_opinfo_entry (test_word_tninfo);
      if (l3_opinfo0 != NULL) {
       /* Use the inputs to the shift-left. */
        bit_number = 63 - TN_Value(OP_opnd(l3_op0,2));
        test_word = OP_opnd(l3_op0,1);
        test_word_tninfo = l3_opinfo0->actual_opnd[1];
      }
    }

  } else if (((OP_code(l2_op1) == TOP_extr) ||
              (OP_code(l2_op1) == TOP_extr_u)) &&
             (EBO_bit_length(l2_op1) == 1) &&
             (TN_Is_Constant(OP_opnd(l2_op1,2)))) {

    l2_opinfo1 = locate_opinfo_entry (l2_tninfo1);
    if (l2_opinfo1 == NULL) return FALSE;
    if (l2_opinfo1->actual_opnd[1] == NULL) return FALSE;

    bit_number = TN_Value(OP_opnd(l2_op1,2));
    test_word = OP_opnd(l2_op1,1);
    test_word_tninfo = l2_opinfo1->actual_opnd[1];
  } else {
    return FALSE;
  }

 /* Is the input to the test a field reference instruction? */
  if ((test_word_tninfo->in_op != NULL) &&
      ((OP_code(test_word_tninfo->in_op) == TOP_sxt1) ||
       (OP_code(test_word_tninfo->in_op) == TOP_sxt2) ||
       (OP_code(test_word_tninfo->in_op) == TOP_sxt4) ||
       (OP_code(test_word_tninfo->in_op) == TOP_zxt1) ||
       (OP_code(test_word_tninfo->in_op) == TOP_zxt2) ||
       (OP_code(test_word_tninfo->in_op) == TOP_zxt4))) {
    INT l3_length;
    l3_op0 = test_word_tninfo->in_op;
    l3_length = EBO_bit_length(l3_op0);
    l3_opinfo1 = locate_opinfo_entry (test_word_tninfo);

    if ((l3_opinfo1 != NULL) &&
        (l3_opinfo1->actual_opnd[1] != NULL) &&
        (EBO_tn_available (bb, l3_opinfo1->actual_opnd[1]))) {
      if (bit_number < l3_length) {
        test_word = OP_opnd(l3_op0,1);
        test_word_tninfo = l3_opinfo1->actual_opnd[1];
      } else if ((OP_code(test_word_tninfo->in_op) == TOP_sxt1) ||
                 (OP_code(test_word_tninfo->in_op) == TOP_sxt2) ||
                 (OP_code(test_word_tninfo->in_op) == TOP_sxt4)) {
        bit_number = l3_length -1;
        test_word = OP_opnd(l3_op0,1);
        test_word_tninfo = l3_opinfo1->actual_opnd[1];
      } else if ((OP_code(test_word_tninfo->in_op) == TOP_zxt1) ||
                 (OP_code(test_word_tninfo->in_op) == TOP_zxt2) ||
                 (OP_code(test_word_tninfo->in_op) == TOP_zxt4)) {
        bit_number = 0;
        test_word = Zero_TN;
        test_word_tninfo = NULL;
      }
    }
  }

 /* Would the word that contains the bit be available for use? */
  if ((test_word_tninfo != NULL) &&
      (!EBO_tn_available (bb, test_word_tninfo))) {
    return FALSE;
  }

 /* Create the test-bit instruction. */
  OPS ops = OPS_EMPTY;
  Build_OP (new_opcode, r_true, r_false, OP_opnd(op,OP_PREDICATE_OPND),
            test_word, Gen_Literal_TN(bit_number, 4), &ops);

  if (!EBO_Verify_Ops(&ops)) return FALSE;
  OP_srcpos(OPS_first(&ops)) = OP_srcpos(op);
  if (EBO_in_loop) EBO_Set_OP_omega (OPS_first(&ops), opnd_tninfo[OP_PREDICATE_OPND],
                                     test_word_tninfo, NULL);
  BB_Insert_Ops(OP_bb(op), op, &ops, FALSE);
if (EBO_Trace_Optimization) fprintf(TFile,"Test_Bit created\n");
  return TRUE;
}




static
BOOL
copy_simplification (OP *op,
                     TN **opnd_tn,
                     EBO_TN_INFO **opnd_tninfo)
{
  INT cpix = EBO_Copy_Operand(op);
  INT prdix = OP_has_predicate(op) ? OP_PREDICATE_OPND : -1;
  TN *tnr;
  TN *current_tn;
  TN *optimal_tn;
  TN *predicate_tn;
  INT64 val;
  EBO_TN_INFO *tninfo;

  if (cpix < 0) return FALSE;
  if (prdix < 0) return FALSE;
  if (OP_results(op) != 1) return FALSE;

  tnr = OP_result(op,0);
  current_tn = OP_opnd(op,cpix);
  optimal_tn = opnd_tn[cpix];
  predicate_tn = opnd_tn[prdix];
  tninfo = opnd_tninfo[cpix];


  if ( (tninfo != NULL) &&
       (tninfo->omega == 0) &&
       (tninfo->in_bb != NULL) &&
       (tnr == current_tn) &&
        !has_assigned_reg(tnr) &&
        !TN_is_save_reg(tnr) &&
        !TN_Is_Constant(current_tn) &&
        !has_assigned_reg(current_tn) &&
        !TN_is_save_reg(current_tn) ) {
   /* Copies to and from the same TN are not needed. */
if (EBO_Trace_Optimization) fprintf(TFile,"Copy to self is not needed\n");
    if (!TN_is_global_reg(current_tn) && (OP_bb(op) != tninfo->in_bb)) {
      mark_tn_live_into_BB (current_tn, OP_bb(op), tninfo->in_bb);
    }
    return TRUE;
  }

  if ((TN_register_class(tnr) == ISA_REGISTER_CLASS_integer) &&
      (TN_has_value(optimal_tn)) &&
      (TN_value(optimal_tn) == 0)) {
    OP *new_op;
    new_op = Mk_OP(TOP_mov, tnr, predicate_tn, Zero_TN);
    OP_srcpos(new_op) = OP_srcpos(op);
    if (EBO_in_loop) EBO_Set_OP_omega ( new_op, opnd_tninfo[OP_PREDICATE_OPND], NULL );
    BB_Insert_Op_After(OP_bb(op), op, new_op);
if (EBO_Trace_Optimization) fprintf(TFile,"Use Zero_TN in place of immediate constant\n");
    return TRUE;
  }

 /* Look for special case merge that is really and ABS function. */
 /* The pattern that we are looking for is:
     TN715 TN716 :- fcmp.lt.unc TN257(p0) (enum:.s0) TN713 TN129(f0) ;
     GTN717 :- fneg TN716 TN713 ; cond_def
     GTN717 :- mov_f TN715 TN713 ; copy cond_def
 */
  EBO_TN_INFO *predicate_tninfo = opnd_tninfo[OP_PREDICATE_OPND];
  if ((predicate_tninfo != NULL) &&
      (predicate_tninfo->in_op != NULL) &&
      (!TN_Is_Constant(predicate_tn))) {
   /* The current result is conditionally defined.  Can we find a previous definition? */
    INT op1_idx = 1;
    TN *tnr = OP_result(op,0);
    EBO_TN_INFO *previous_tninfo = get_tn_info(tnr);
    EBO_OP_INFO *previous_opinfo = NULL;
    EBO_TN_INFO *previous_op0_tninfo = NULL;
    TOP previous_opcode = TOP_noop;
    if ((previous_tninfo != NULL)  &&
        (previous_tninfo->in_op != NULL) &&
        (previous_tninfo->reference_count == 0) &&
        (previous_tninfo->predicate_tninfo != NULL)) {
      previous_opinfo = locate_opinfo_entry (previous_tninfo);
      if ((previous_opinfo != NULL) &&
          (previous_opinfo->in_op != NULL)) {
        previous_opcode = OP_code(previous_opinfo->in_op);
        previous_op0_tninfo = previous_opinfo->optimal_opnd[op1_idx];
      }
    }
    if ((previous_op0_tninfo != NULL) &&
        (previous_opcode == TOP_fneg) &&
        (opnd_tn[op1_idx] == previous_op0_tninfo->local_tn) &&
        EBO_predicate_complements (predicate_tn, predicate_tninfo,
                                   previous_tninfo->predicate_tninfo->local_tn, previous_tninfo->predicate_tninfo)) {
      OP *predicate_op = predicate_tninfo->in_op;
      TOP predicate_opcode = OP_code(predicate_op);
      EBO_OP_INFO *predicate_opinfo = locate_opinfo_entry (predicate_tninfo);
      if ((predicate_opinfo != NULL) &&
          (predicate_opinfo->in_op != NULL) &&
          EBO_tn_available(OP_bb(op), predicate_opinfo->actual_opnd[OP_PREDICATE_OPND]) &&
          (((predicate_opcode == TOP_fcmp_lt_unc) || (predicate_opcode == TOP_fcmp_le_unc)) &&
           (predicate_opinfo->actual_rslt[1] == predicate_tninfo) &&
           (OP_opnd(predicate_op, TOP_Find_Operand_Use(predicate_opcode,OU_opnd2)) == FZero_TN) &&
           (predicate_opinfo->optimal_opnd[TOP_Find_Operand_Use(predicate_opcode,OU_opnd1)] == opnd_tninfo[op1_idx]))) {
if (EBO_Trace_Optimization) fprintf(TFile,"simplify fneg(fcmp()) operation.\n");
        BB *bb = OP_bb(op);
        TOP new_opcode = TOP_fmerge_s;
        OP *new_op = Mk_OP (new_opcode, tnr,
                            OP_opnd(predicate_op,OP_PREDICATE_OPND), FZero_TN, OP_opnd(op,1));
        OP_srcpos(new_op) = OP_srcpos(op);
        if (EBO_in_loop) EBO_Set_OP_omega ( new_op,
                                            predicate_opinfo->actual_opnd[OP_PREDICATE_OPND],
                                            opnd_tninfo[op1_idx]);
        BB_Insert_Op_After(bb, op, new_op);

       /* Now get rid of the previous fneg instruction. */
        remove_op (previous_opinfo);
        OP_Change_To_Noop(previous_opinfo->in_op);
        previous_opinfo->in_op = NULL;
        previous_opinfo->in_bb = NULL;
        return TRUE;
      }
    } else if ((previous_tninfo != NULL) &&
               (previous_tninfo->predicate_tninfo != NULL) &&
               (previous_opinfo != NULL) &&
               (previous_opinfo->in_op != NULL) &&
               OP_effectively_copy(previous_opinfo->in_op) &&
               EBO_predicate_complements (predicate_tn, predicate_tninfo,
                                   previous_tninfo->predicate_tninfo->local_tn, previous_tninfo->predicate_tninfo) &&
               (((previous_op0_tninfo != NULL) &&
                 (opnd_tn[op1_idx] == previous_op0_tninfo->local_tn)) ||
                (TN_Is_Constant(current_tn) &&
                 (current_tn == OP_opnd(previous_opinfo->in_op,EBO_Copy_Operand(previous_opinfo->in_op)))))) {
      OP *predicate_op = predicate_tninfo->in_op;
      TOP predicate_opcode = OP_code(predicate_op);
      EBO_OP_INFO *predicate_opinfo = locate_opinfo_entry (predicate_tninfo);
      if ((predicate_opinfo != NULL) &&
          (predicate_opinfo->in_op != NULL) &&
          OP_icmp(predicate_opinfo->in_op) &&
          EBO_tn_available(OP_bb(op), predicate_opinfo->actual_opnd[OP_PREDICATE_OPND]) &&
          TNs_Are_Equivalent(current_tn, OP_opnd(previous_opinfo->in_op,EBO_Copy_Operand(previous_opinfo->in_op)))) {
if (EBO_Trace_Optimization) fprintf(TFile,"complementary moves of the same value\n");
        OPS ops = OPS_EMPTY;
        TN * predicate_tn = OP_opnd(predicate_opinfo->in_op,OP_PREDICATE_OPND);
        EBO_Exp_COPY (predicate_tn,
                      OP_result(op, 0),
                      current_tn,
                      &ops);

	if (!EBO_Verify_Ops(&ops)) return FALSE;
        OP_srcpos(OPS_first(&ops)) = OP_srcpos(op);
        if (EBO_in_loop) EBO_Set_OP_omega (OPS_first(&ops), predicate_opinfo->actual_opnd[OP_PREDICATE_OPND],
                                           previous_opinfo->actual_opnd[EBO_Copy_Operand(previous_opinfo->in_op)]);
        if (predicate_tn != True_TN) Set_OP_cond_def_kind(OPS_last(&ops), OP_PREDICATED_DEF);
        BB_Insert_Ops(OP_bb(op), op, &ops, FALSE);
        return TRUE;
      }
    }
  }

  if (current_tn == optimal_tn) return FALSE;
  if (TN_is_const_reg(current_tn)) return FALSE;
  if ((predicate_tn == True_TN) ||
      (predicate_tn == Zero_TN) ||
      TN_is_constant(predicate_tn)) return FALSE;
  if (TN_register_class(tnr) != ISA_REGISTER_CLASS_integer) return FALSE;
  if (!TN_has_value(optimal_tn)) return FALSE;

 /* Generate a move-immediate */
  val = TN_value(optimal_tn);
  OP *new_op;
  TOP new_opcode = (ISA_LC_Value_In_Class (val, LC_i22)) ? TOP_mov_i : TOP_movl;
  new_op = Mk_OP(new_opcode, tnr, predicate_tn, optimal_tn);
  OP_srcpos(new_op) = OP_srcpos(op);
  if (EBO_in_loop) EBO_Set_OP_omega ( new_op, opnd_tninfo[OP_PREDICATE_OPND], NULL );
  BB_Insert_Op_After(OP_bb(op), op, new_op);
if (EBO_Trace_Optimization) fprintf(TFile,"Conditionally load an immediate constant\n");
  return TRUE;
}




/*
 *	It may be possible to to recognize the specific registers
 *	that are involved with certain instructions that would
 *	need to be treated as "barriers" to EBO optimization.
 *	If this can be done, a use or definition can be created
 *	in the TN_INFO_TABLE that will restrict forward propagation
 *	and other hazardous optimizations without stopping
 *	attempts to optimized all the other OPs in the block.
 *
 *	Forward propagation of specific registers is prevented by
 *	creating a new EBO_TN_INFO entry that acts as a dummy
 *	defintion.
 *
 *	Note that the use count of pre-existing EBO_TN_INFO
 *	entries is incremented.  This is done to assure that the
 *	previous definition of the register is preserved, since
 *	there is some ambiguity as to whether or not the register
 *	is modified/written by the group reference.
 */
BOOL Process_Group_Register_Reference (OP *op)
{
  REGISTER i;
  REGISTER first;
  REGISTER last;
  TN *pred_tn;

  switch (OP_code(op)) {
  case TOP_clrrrb:
    first = REGISTER_First_Rotating_Registers(ISA_REGISTER_CLASS_integer);
    last  = REGISTER_Last_Rotating_Registers (ISA_REGISTER_CLASS_integer);

    for (i=first; i<=last; i++) {
      TN *tn = Build_Dedicated_TN(ISA_REGISTER_CLASS_integer,i,0);
      EBO_TN_INFO *tninfo = get_tn_info(tn);
      if (tninfo != NULL) {
if(EBO_Trace_Optimization)fprintf(TFile,"TOP_clrrrb r%d\n",i);
        EBO_TN_INFO *new_tninfo = tn_info_def (OP_bb(op), op, tn, True_TN, NULL);
        inc_ref_count(tninfo);
      }
    }

    first = REGISTER_First_Rotating_Registers(ISA_REGISTER_CLASS_float);
    last  = REGISTER_Last_Rotating_Registers (ISA_REGISTER_CLASS_float);

    for (i=first; i<=last; i++) {
      TN *tn = Build_Dedicated_TN(ISA_REGISTER_CLASS_float,i,0);
      EBO_TN_INFO *tninfo = get_tn_info(tn);
      if (tninfo != NULL) {
if(EBO_Trace_Optimization)fprintf(TFile,"TOP_clrrrb f%d\n",i);
        EBO_TN_INFO *new_tninfo = tn_info_def (OP_bb(op), op, tn, True_TN, NULL);
        inc_ref_count(tninfo);
      }
    }

   /* Also clear the predicate registers by falling through to the next case. */
  case TOP_clrrrb_pr:
    first = REGISTER_First_Rotating_Registers(ISA_REGISTER_CLASS_predicate);
    last  = REGISTER_Last_Rotating_Registers (ISA_REGISTER_CLASS_predicate);

    for (i=first; i<=last; i++) {
      TN *tn = Build_Dedicated_TN(ISA_REGISTER_CLASS_predicate,i,0);
      EBO_TN_INFO *tninfo = get_tn_info(tn);
      if (tninfo != NULL) {
if(EBO_Trace_Optimization)fprintf(TFile,"TOP_clrrrb pr%d\n",i);
        EBO_TN_INFO *new_tninfo = tn_info_def (OP_bb(op), op, tn, True_TN, NULL);
        inc_ref_count(tninfo);
      }
    }

    return TRUE;
  case TOP_mov_t_pr_i:
    first = REGISTER_First_Rotating_Registers(ISA_REGISTER_CLASS_predicate);
    last  = REGISTER_Last_Rotating_Registers (ISA_REGISTER_CLASS_predicate);
    pred_tn = OP_has_predicate(op)?OP_opnd(op,OP_PREDICATE_OPND):True_TN;

    for (i=first; i<=last; i++) {
      TN *tn = Build_Dedicated_TN(ISA_REGISTER_CLASS_predicate,i,0);
      EBO_TN_INFO *tninfo = get_tn_info(tn);
      if (tninfo != NULL) {
if(EBO_Trace_Optimization)fprintf(TFile,"TOP_mov_t_pr_i pr%d\n",i);
        EBO_TN_INFO *new_tninfo = tn_info_def (OP_bb(op), op, tn, pred_tn, NULL);
        inc_ref_count(tninfo);
      }
    }

    return TRUE;
  case TOP_mov_t_pr:
    first = REGISTER_MIN + 1;
    last  = REGISTER_CLASS_last_register (ISA_REGISTER_CLASS_predicate);
    pred_tn = OP_has_predicate(op)?OP_opnd(op,OP_PREDICATE_OPND):True_TN;

    for (i=first; i<=last; i++) {
      TN *tn = Build_Dedicated_TN(ISA_REGISTER_CLASS_predicate,i,0);
      EBO_TN_INFO *tninfo = get_tn_info(tn);
      if (tninfo != NULL) {
if(EBO_Trace_Optimization)fprintf(TFile,"TOP_mov_t_pr pr%d\n",i);
        EBO_TN_INFO *new_tninfo = tn_info_def (OP_bb(op), op, tn, pred_tn, NULL);
        inc_ref_count(tninfo);
      }
    }

    return TRUE;
  case TOP_mov_f_pr:
    first = REGISTER_MIN + 1;
    last  = REGISTER_CLASS_last_register (ISA_REGISTER_CLASS_predicate);
    pred_tn = OP_has_predicate(op)?OP_opnd(op,OP_PREDICATE_OPND):True_TN;

    for (i=first; i<=last; i++) {
      TN *tn = Build_Dedicated_TN(ISA_REGISTER_CLASS_predicate,i,0);
      EBO_TN_INFO *tninfo = get_tn_info(tn);
      if (tninfo != NULL) {
if(EBO_Trace_Optimization)fprintf(TFile,"TOP_mov_f_pr pr%d\n",i);
        inc_ref_count(tninfo); /* This is a use and must be kept around. */
      }
    }

    return TRUE;
  }

  return FALSE;
}
 
#endif


/* =====================================================================
 *                          Helper functions
 * =====================================================================
 */
#define IS_SHL_16(o)   (OP_code(o) == TOP_shl_i && \
                        TN_is_constant(OP_opnd(o,1)) && \
                        TN_value(OP_opnd(o,1)) == 16)

#define IS_SHR_16(o)   ((OP_code(o) == TOP_shr_i || OP_code(o) == TOP_shru_i) && \
                        TN_is_constant(OP_opnd(o,1)) && \
                        TN_value(OP_opnd(o,1)) == 16)

#define IS_MULL(o)      (OP_code(o) == TOP_mull_i ||   \
                         OP_code(o) == TOP_mull_ii ||  \
                         OP_code(o) == TOP_mull_r ||   \
			 OP_code(o) == TOP_mullu_i ||  \
                         OP_code(o) == TOP_mullu_ii || \
                         OP_code(o) == TOP_mullu_r)

#define IS_MULH(o)      (OP_code(o) == TOP_mulh_i ||   \
                         OP_code(o) == TOP_mulh_ii ||  \
                         OP_code(o) == TOP_mulh_r ||   \
			 OP_code(o) == TOP_mulhu_i ||  \
                         OP_code(o) == TOP_mulhu_ii || \
                         OP_code(o) == TOP_mulhu_r)

#define IS_MULLH(o)     (OP_code(o) == TOP_mullh_i ||   \
                         OP_code(o) == TOP_mullh_ii ||  \
                         OP_code(o) == TOP_mullh_r ||   \
			 OP_code(o) == TOP_mullhu_i ||  \
                         OP_code(o) == TOP_mullhu_ii || \
                         OP_code(o) == TOP_mullhu_r)

#define IS_MULLL(o)     (OP_code(o) == TOP_mulll_i ||   \
                         OP_code(o) == TOP_mulll_ii ||  \
                         OP_code(o) == TOP_mulll_r ||   \
			 OP_code(o) == TOP_mulllu_i ||  \
                         OP_code(o) == TOP_mulllu_ii || \
                         OP_code(o) == TOP_mulllu_r)

#define IS_MUL32_PART(o) (OP_code(o) == TOP_mulhs_r || \
			  OP_code(o) == TOP_mullu_r)

/* =====================================================================
 *            Multiplication Tables
 * =====================================================================
 */

enum SIGNDNESS {ZERO_EXT = 0, SIGN_EXT = 1, SIGN_UNKNOWN = 2};
enum BITS_POS {TN_LO_16 = 0, TN_HI_16 = 1, TN_32_BITS = 2};

#define SIGNDNESS_Name(x) SIGNDESS_NAME[x]
#define BITS_POS_Name(x) BITS_POS_NAME[x]
const char * const SIGNDESS_NAME[] = { "ZERO_EXT", "SIGN_EXT", "SIGN_UNK" };
const char * const BITS_POS_NAME[] = { "LO_16", "HI_16", "32_BITS", "BITS_UNK" };

static const TOP unsigned_mul_opcode[3][3] = {
  // By:  lo 16 bit      hi 16 bit       32 bit
  {      TOP_mulllu_r,  TOP_mullhu_r, TOP_UNDEFINED  },   // lo 16 bits
  {     TOP_UNDEFINED,  TOP_mulhhu_r, TOP_UNDEFINED  },   // hi 16 bits
  {      TOP_mullu_r,   TOP_mulhu_r,  TOP_UNDEFINED  }    // 32 bits
};

static const TOP signed_mul_opcode[3][3] = {
  // By:  lo 16 bit      hi 16 bit       32 bit
  {      TOP_mulll_r,  TOP_mullh_r, TOP_UNDEFINED  },   // lo 16 bits
  {     TOP_UNDEFINED, TOP_mulhh_r, TOP_UNDEFINED  },   // hi 16 bits
  {      TOP_mull_r,   TOP_mulh_r,  TOP_UNDEFINED  }    // 32 bits
};

/* =====================================================================
 *   get_mul_opcode
 *
 *   given (signdness,bits_pos) pairs for two operands, 
 *   return the corresponding
 *   mul opcode.
 * =====================================================================
 */
static TOP
get_mul_opcode(SIGNDNESS signed0, BITS_POS bits0, SIGNDNESS signed1, BITS_POS bits1)
{
  TOP opcode;
  // If both operands are sign-extended or one is 32 bits and one
  // is sign-extended, get a signed TOP.
  if (signed0 == SIGN_EXT && signed1 == SIGN_EXT ||
      bits0 == TN_32_BITS && signed1 == SIGN_EXT)
    opcode = signed_mul_opcode[bits0][bits1];
  else if (signed0 == ZERO_EXT && signed1 == ZERO_EXT ||
	   bits0 == TN_32_BITS && signed1 == ZERO_EXT)
    opcode = unsigned_mul_opcode[bits0][bits1];
  else
    opcode = TOP_UNDEFINED;

  if (EBO_Trace_Optimization && opcode != TOP_UNDEFINED) 
    fprintf(TFile,"Get mul opcode for (%s, %s) x (%s, %s): %s\n", SIGNDNESS_Name(signed0), BITS_POS_Name(bits0),  SIGNDNESS_Name(signed1), BITS_POS_Name(bits1), TOP_Name(opcode));

  return opcode;
}

/*
 * l0c()
 *
 * Returns leading zero count of 64 bits value
 */
static int
l0c(INT64 val)
{
  int n = 64;
  while(val > 0) {
    n--;
    val >>= 1;
  }
  if (val == 0) return n;
  return 0;
}

/*
 * l1c()
 *
 * Returns leading one count of 64 bits value
 */
static int
l1c(INT64 val)
{
  int n = 64;
  while(val < -1) {
    n--;
    val >>= 1;
  }
  if (val == -1) return n;
  return 0;
}

/*
 * t1c()
 * 
 * Return the number of trailing 1 bits followed by only 0 bits.
 * Return -1 if not.
 */
static int
t1c(INT64 val)
{
  int n = 0;
  while (val & 1) {
    n++;
    val = (UINT64)val >> 1;
  }
  if (val == 0) return n;
  return -1;
}


/*
 * def_bit_width
 *
 * Returns true if the defined bits of the def can
 * be guessed and returns bit width and sign extension.
 *
 */
static BOOL
def_bit_width(OP *op, INT32 def_idx, INT32 *def_bits, INT32 *def_signed)
{
  INT32 opnd1_idx, opnd2_idx;
  INT64 val;
  TOP opcode = OP_code(op);

  if (def_idx != 0) return FALSE;

  if (OP_load(op)) {
    *def_bits = OP_Mem_Ref_Bytes(op)*8;
    *def_signed = TOP_is_unsign(opcode) ? 0 : 1;
    return TRUE;
  }
  if ((OP_ishr(op) || OP_ishru(op)) &&
      (opnd1_idx = TOP_Find_Operand_Use(opcode,OU_opnd1)) >= 0 &&
      (opnd2_idx = TOP_Find_Operand_Use(opcode,OU_opnd2)) >= 0 &&
      TN_Has_Value(OP_opnd(op,opnd2_idx))) {
    val = TOP_fetch_opnd(opcode, op->res_opnd+OP_opnd_offset(op), opnd2_idx);
    INT32 use_bits = TOP_opnd_use_bits(opcode, opnd1_idx);
    *def_bits = MAX(0, use_bits - val);
    *def_signed = TOP_opnd_use_signed(opcode, opnd1_idx);
    return TRUE;
  }

  if ((OP_sext(op) || OP_zext(op)) &&
      (opnd1_idx = TOP_Find_Operand_Use(opcode,OU_opnd1)) >= 0) {
    *def_bits = TOP_opnd_use_bits(opcode, opnd1_idx);
    *def_signed = TOP_opnd_use_signed(opcode, opnd1_idx);
    return TRUE;
  }

  if (OP_iand(op) &&
      (opnd1_idx = TOP_Find_Operand_Use(opcode,OU_opnd1)) >= 0 &&
      (opnd2_idx = TOP_Find_Operand_Use(opcode,OU_opnd2)) >= 0 &&
      TN_Has_Value(OP_opnd(op,opnd2_idx))) {
    val = TOP_fetch_opnd(opcode, op->res_opnd+OP_opnd_offset(op), opnd2_idx);
    INT32 use_bits = TOP_opnd_use_bits(opcode, opnd1_idx);
    INT32 bits = 64 - l0c(val);
    if (bits < use_bits) {
      *def_bits = bits;
      *def_signed = 0;
      return TRUE;
    }
  }

  if (OP_icmp(op)) {
    *def_bits = 1;
    *def_signed = 0;
    return TRUE;
  }

  if (OP_code(op) == TOP_mfb) {
    *def_bits = 1;
    *def_signed = 0;
    return TRUE;
  }

  return FALSE;
}

/*
 * use_bit_width
 *
 * Returns true if the effective used bit of the operand def can
 * be guessed and returns bit width.
 * Some special cases are:
 * - shift left: use bits - shift amount
 * - and : use bits - leading zeros
 * - or : use bits - leading one
 *
 */
static BOOL
use_bit_width(OP *op, INT32 opnd_idx, INT32 *use_bits)
{
  INT32 opnd1_idx, opnd2_idx;
  INT64 val;
  TOP opcode = OP_code(op);
  INT32 bits = TOP_opnd_use_bits(opcode, opnd_idx);

  if (bits < 0) return FALSE;
  *use_bits = bits;

  if (OP_ishl(op) &&
      (opnd1_idx = TOP_Find_Operand_Use(opcode,OU_opnd1)) >= 0 &&
      (opnd2_idx = TOP_Find_Operand_Use(opcode,OU_opnd2)) >= 0 &&
      opnd_idx == opnd1_idx &&
      TN_Has_Value(OP_opnd(op,opnd2_idx))) {
    val = TOP_fetch_opnd(opcode, op->res_opnd+OP_opnd_offset(op), opnd2_idx);
    *use_bits = MAX(0, bits - val);
  }
  if ((OP_iand(op) || OP_ior(op)) &&
      (opnd1_idx = TOP_Find_Operand_Use(opcode,OU_opnd1)) >= 0 &&
      (opnd2_idx = TOP_Find_Operand_Use(opcode,OU_opnd2)) >= 0 &&
      opnd_idx == opnd1_idx &&
      TN_Has_Value(OP_opnd(op,opnd2_idx))) {
    val = TOP_fetch_opnd(opcode, op->res_opnd+OP_opnd_offset(op), opnd2_idx);
    INT32 eff_bits = OP_iand(op) ? 64 - l0c(val) : 64 - l1c(val);
    if (eff_bits < bits) {
      *use_bits = eff_bits;
    }
  }
  return TRUE;
}

/*
 * OP_is_extension()
 *
 * Returns true if the operation acts as an extension
 * Set signed to true if sign extension
 */
static BOOL
OP_is_extension(OP *op, INT32 opnd_idx, INT32 *ext_bits, INT32 *ext_signed)
{
  INT32 opnd1_idx, opnd2_idx;
  INT64 val;
  TOP opcode = OP_code(op);
  
  if (OP_results(op) != 1) return FALSE;
  
  if ((OP_sext(op) || OP_zext(op)) &&
      (opnd1_idx = TOP_Find_Operand_Use(opcode,OU_opnd1)) >= 0 &&
      opnd1_idx == opnd_idx) {
    *ext_bits = TOP_opnd_use_bits(opcode, opnd1_idx);
    *ext_signed = TOP_opnd_use_signed(opcode, opnd1_idx);
    return TRUE;
  }

  if (OP_iand(op) &&
      (opnd1_idx = TOP_Find_Operand_Use(opcode,OU_opnd1)) >= 0 &&
      (opnd2_idx = TOP_Find_Operand_Use(opcode,OU_opnd2)) >= 0 &&
      opnd1_idx == opnd_idx &&
      TN_Has_Value(OP_opnd(op,opnd2_idx))) {
    val = TOP_fetch_opnd(opcode, op->res_opnd+OP_opnd_offset(op), opnd2_idx);
    INT32 use_bits = TOP_opnd_use_bits(opcode, opnd1_idx);
    INT32 bits = t1c(val);
    if (bits < use_bits) {
      *ext_bits = bits;
      *ext_signed = 0;
      return TRUE;
    }
  }
  return FALSE;
}

/* =====================================================================
 *   Is_16_Bits
 * =====================================================================
 */
static BOOL 
Is_16_Bits (
  EBO_TN_INFO *opnd_tninfo,
  BB *bb,
  TN **ret,
  EBO_TN_INFO **ret_tninfo,
  BITS_POS *hilo,                 // lower or upper 16 bits of a TN
  SIGNDNESS *sign_ext             // sign extended ?
) 
{
  EBO_OP_INFO *opinfo = locate_opinfo_entry(opnd_tninfo);;
  if ((opinfo == NULL) || (opinfo->in_op == NULL)) 
    return FALSE;

  OP *op = opinfo->in_op;

  if (IS_SHR_16(op)) {
    *ret = OP_opnd(op,0);
    *ret_tninfo = opinfo->actual_opnd[0];
    *sign_ext = TOP_is_unsign(OP_code(op)) ? ZERO_EXT : SIGN_EXT;
    *hilo = TN_HI_16;
    if (EBO_tn_available (bb, *ret_tninfo)) return TRUE;
  }

  if (OP_code(op) == TOP_sxth_r) {
    *ret = OP_opnd(op,0);
    *ret_tninfo = opinfo->actual_opnd[0];
    *sign_ext = SIGN_EXT;
    *hilo = TN_LO_16;
    if (EBO_tn_available (bb, *ret_tninfo)) return TRUE;
  }

  if (OP_code(op) == TOP_zxth_r ||
      (OP_iand(op) && TN_Has_Value(OP_opnd(op,1)) && TN_Value(OP_opnd(op,1)) == 65535)) {
    *ret = OP_opnd(op,0);
    *ret_tninfo = opinfo->actual_opnd[0];
    *sign_ext = ZERO_EXT;
    *hilo = TN_LO_16;
    if (EBO_tn_available (bb, *ret_tninfo)) return TRUE;
  }

  if (IS_SHL_16(op)) {
    *ret = OP_opnd(op,0);
    *ret_tninfo = opinfo->actual_opnd[0];
    *sign_ext = SIGN_UNKNOWN;
    *hilo = TN_LO_16;
    if (EBO_tn_available (bb, *ret_tninfo)) return TRUE;
  }

  if ((OP_code(op) == TOP_mov_i || OP_code(op) == TOP_mov_ii) &&
      TN_Has_Value(OP_opnd(op,0))) {
    INT64 value = TN_Value(OP_opnd(op,0));
    if (value >= -32768 && value <= 32767) {
      *ret = OP_result(op,0);
      *ret_tninfo = opnd_tninfo;
      *sign_ext = SIGN_EXT;
      *hilo = TN_LO_16;
      if (EBO_tn_available (bb, *ret_tninfo)) return TRUE;
    } else if (value >= 0 && value <= 65535) {
      *ret = OP_result(op,0);
      *ret_tninfo = opnd_tninfo;
      *sign_ext = ZERO_EXT;
      *hilo = TN_LO_16;
      if (EBO_tn_available (bb, *ret_tninfo)) return TRUE;
    }
  }

  // Check bit width of definition
  // in this case we return the def itself
  INT32 def_bits, def_signed;
  if (OP_results(op) == 1 && def_bit_width(op, 0, &def_bits, &def_signed)) {
    if (def_bits <= 16) {
      *ret = OP_result(op, 0);
      *ret_tninfo = opnd_tninfo;
      *hilo = TN_LO_16;
      *sign_ext = def_signed ? SIGN_EXT: ZERO_EXT;
      if (EBO_tn_available (bb, *ret_tninfo)) return TRUE;
    }
  }

  return FALSE;
}

/* =====================================================================
 * Function: add_shl_sequence
 *
 * Look for add(shl) sequence and replace with a single shladd
 * instruction. 
 * =====================================================================
 */
static BOOL
add_shl_sequence (
  OP *op,
  TN **opnd_tn,
  EBO_TN_INFO **opnd_tninfo
)
{
  TOP opcode = OP_code(op);

  // addcg can't be transformed
  if (opcode == TOP_addcg) return FALSE;

  /* Level 1 data: */
  BB *bb = OP_bb(op);
  OP *l1_op0 = op;
  TN *l1_tn0 = OP_result(l1_op0,0);

  /* Level 2 data: */
  OP *l2_op0;
  TN *l2_tn1 = OP_opnd(op,1); // operand 2
  EBO_TN_INFO *l2_tninfo0 = opnd_tninfo[0];
  EBO_TN_INFO *l2_tninfo1 = opnd_tninfo[1];
  EBO_OP_INFO *l2_opinfo0;
  
  /* Level 3 data: */
  TN *l3_tn0;
  TN *l3_tn1;
  EBO_TN_INFO *l3_tninfo0;
  INT64 l3_val1;

  /* The input to the add must be a shift-left-immediate. */
  /* TODO: Note that a shift-left-add instruction with an add   */
  /* of zero, is treated as a shift-left-immediate.       */
  if ((l2_tninfo0 == NULL) ||
      (l2_tninfo0->in_op == NULL) ||
      ((OP_code(l2_tninfo0->in_op) != TOP_shl_i) &&
       (OP_code(l2_tninfo0->in_op) != TOP_shl_ii)
#if 0
 &&
       ((OP_code(l2_tninfo0->in_op) != TOP_shladd) ||
        (OP_opnd(l2_tninfo0->in_op,3) != Zero_TN))
#endif
      )) {
    /* Try the other operand of the add. */
    l2_tn1 = OP_opnd(op,0);
    l2_tninfo0 = l2_tninfo1;
    l2_tninfo1 = opnd_tninfo[0];

    if ((l2_tninfo0 == NULL) ||
        (l2_tninfo0->in_op == NULL) ||
        ((OP_code(l2_tninfo0->in_op) != TOP_shl_i) &&
	 (OP_code(l2_tninfo0->in_op) != TOP_shl_ii)
#if 0
 &&
         ((OP_code(l2_tninfo0->in_op) != TOP_shladd) ||
          (OP_opnd(l2_tninfo0->in_op,3) != Zero_TN))
#endif
	)) return FALSE;
  }

  /* Symbols won't fit into the addend field. */
  if (TN_is_symbol(l2_tn1)) return FALSE;

  /* Determine the inputs to the second instructions. */
  l2_opinfo0 = locate_opinfo_entry(l2_tninfo0);
  if ((l2_opinfo0 == NULL) ||
      (l2_opinfo0->in_op == NULL)) return FALSE;

  l2_op0 = l2_opinfo0->in_op;
  l3_tn0 = OP_opnd(l2_op0, 0);
  l3_tn1 = OP_opnd(l2_op0, 1);
  l3_tninfo0 = l2_opinfo0->actual_opnd[0];
  if ((l3_tn0 == NULL) || TN_Is_Constant(l3_tn0) ||
      (l3_tn1 == NULL) || !TN_Is_Constant(l3_tn1) ||
      !EBO_tn_available (bb, l3_tninfo0)) return FALSE;

  /* Will the shift count fit into the shXadd instruction? */
  l3_val1 = TN_value(l3_tn1);
  TOP new_topcode;
  if (l3_val1 == 1) new_topcode = TOP_sh1add_r;
  else if (l3_val1 == 2) new_topcode = TOP_sh2add_r;
  else if (l3_val1 == 3) new_topcode = TOP_sh3add_r;
  else if (l3_val1 == 4) new_topcode = TOP_sh4add_r;
  else return FALSE;

  if (TN_is_symbol(l2_tn1)) return FALSE;
  if (TN_has_value(l2_tn1)) new_topcode = TOP_opnd_immediate_variant(new_topcode, 1, TN_value(l2_tn1));
  if (new_topcode == TOP_UNDEFINED) return FALSE;

  /* Replace the current instruction. */
  OP *new_op;
  new_op = Mk_OP(new_topcode, l1_tn0, l3_tn0, l2_tn1);
  OP_srcpos(new_op) = OP_srcpos(op);
  if (EBO_in_loop) EBO_Set_OP_omega (new_op, l3_tninfo0, l2_tninfo1);
  BB_Insert_Op_After(bb, op, new_op);
  if (EBO_Trace_Optimization) 
    fprintf(TFile,"Convert add(shl) to sh%lldadd\n", l3_val1);
  return TRUE;
}


/* =====================================================================
 *   Function: addcg_sequence
 *
 *   Replace addcg with add/mtb
 * =====================================================================
 */
static BOOL
addcg_sequence (
  OP *op,
  TN **opnd_tn,
  EBO_TN_INFO **opnd_tninfo
)
{
  if (getenv ("NO_ADDCG")) return FALSE;
  if (OP_code(op) != TOP_addcg) return FALSE;

  int op_idx;
  TN *tn0 = opnd_tn[0];
  TN *tn1 = opnd_tn[1];
  TN *tn2 = opnd_tn[2];
  if (TN_Is_Constant(tn0) && TN_Value(tn0) == 0) {
    op_idx = 1;
  } else if (TN_Is_Constant(tn1) && TN_Value(tn1) == 0) {
    op_idx = 0;
  } else return FALSE;
  if (TN_Is_Constant(tn2) && TN_Value(tn2) == 0 &&
      !OP_copy(op) /* Avoid optimizing the special copy br<-br */) {
    OPS ops = OPS_EMPTY;
    if (OP_result(op, 0) != Zero_TN) {
      EBO_Exp_COPY(NULL, OP_result(op, 0), OP_opnd(op, op_idx), &ops);
      if (EBO_in_loop) 
	EBO_OPS_omega (&ops, OP_opnd(op,op_idx), opnd_tninfo[op_idx]);
    }
    TN *tnc = Gen_Literal_TN (0, 4);
    Expand_Immediate (OP_result(op, 1), tnc, 0, &ops);
    OP_srcpos(OPS_first(&ops)) = OP_srcpos(op);
    BB_Insert_Ops(OP_bb(op), op, &ops, FALSE);
    if (EBO_Trace_Optimization) 
      fprintf(TFile,"Convert addcg into mov\n");
    return TRUE;
  }
  return FALSE;
}

/* =====================================================================
 *   Function: add_mul_sequence
 *
 *   Look for the following sequence and replace with a single mul??
 *   instruction:
 *
 *                        add
 *                        / \
 *                   mulhs   mullu
 *                    / \     / \
 *
 *   The above is _mulw/_muluw. Depending on inputs, may be strength
 *   reduced to 16x16, 16x32, etc.
 *
 * =====================================================================
 */
static BOOL
add_mul_sequence (
  OP *op,
  TN **opnd_tn,
  EBO_TN_INFO **opnd_tninfo
)
{
  TOP opcode = OP_code(op);

  // Only add register form can be transformed
  if (opcode != TOP_add_r) return FALSE;

  TN *l1_tn0 = OP_opnd(op, 0);
  TN *l1_tn1 = OP_opnd(op, 1);

  // Level 1 data:
  BB *bb = OP_bb(op);
  OP *l1_op = op;
  TN *l1_res = OP_result(l1_op, 0);

  EBO_TN_INFO *l1_tninfo0 = opnd_tninfo[0];
  EBO_TN_INFO *l1_tninfo1 = opnd_tninfo[1];

  // The input to the add must be a mulhs_r and a mullu_r:
  if ((l1_tninfo0 == NULL) || (l1_tninfo0->in_op == NULL) ||
      (l1_tninfo1 == NULL) || (l1_tninfo1->in_op == NULL))
    return FALSE;

  // Determine the inputs to the 1st level instruction:
  EBO_OP_INFO *l2_opinfo0;
  EBO_OP_INFO *l2_opinfo1;

  l2_opinfo0 = locate_opinfo_entry(l1_tninfo0);
  if ((l2_opinfo0 == NULL) ||
      (l2_opinfo0->in_op == NULL)) return FALSE;

  l2_opinfo1 = locate_opinfo_entry(l1_tninfo1);
  if ((l2_opinfo1 == NULL) ||
      (l2_opinfo1->in_op == NULL)) return FALSE;

  // Level 2 data:
  OP *l2_op0 = l2_opinfo0->in_op;   // OP producing add operand 0
  OP *l2_op1 = l2_opinfo1->in_op;   // OP producing add operand 1

  if ((OP_code(l2_op0) != TOP_mullu_r &&
       OP_code(l2_op0) != TOP_mulhs_r) ||
      (OP_code(l2_op1) != TOP_mullu_r &&
       OP_code(l2_op1) != TOP_mulhs_r) ||
      (OP_code(l2_op0) == TOP_mulhs_r &&
       OP_code(l2_op1) != TOP_mullu_r) ||
      (OP_code(l2_op0) == TOP_mullu_r &&
       OP_code(l2_op1) != TOP_mulhs_r)
      )
    return FALSE;

  if (EBO_Trace_Optimization) 
    fprintf(TFile, "Possible 32x32 mul\n");

  // We've just found a 32x32 MPY. Try to strength-reduce its
  // operands, eg. 32x16, etc.
  TN *l2_tn0;
  TN *l2_tn1;
  EBO_TN_INFO *l2_tninfo0;
  EBO_TN_INFO *l2_tninfo1;
  BITS_POS l2_hilo0;
  BITS_POS l2_hilo1;
  SIGNDNESS l2_signed0;
  SIGNDNESS l2_signed1;

  BOOL reduce_tn0 = FALSE;
  BOOL reduce_tn1 = FALSE;

  TN *tn0;
  TN *tn1;
  EBO_TN_INFO *tninfo0;
  EBO_TN_INFO *tninfo1;

  TN *tmp_tn0;
  TN *tmp_tn1;
  EBO_TN_INFO *tmp_tninfo0;
  EBO_TN_INFO *tmp_tninfo1;
  BITS_POS tmp_hilo0;
  BITS_POS tmp_hilo1;
  SIGNDNESS tmp_signed0;
  SIGNDNESS tmp_signed1;

  //
  // Process operand 0:
  //
  tn0 = OP_opnd(l2_op0,0);
  tn1 = OP_opnd(l2_op1,0);
  tninfo0 = l2_opinfo0->actual_opnd[0];
  tninfo1 = l2_opinfo1->actual_opnd[0];

  if (Is_16_Bits(tninfo0, bb, &tmp_tn0, &tmp_tninfo0, &tmp_hilo0, &tmp_signed0)) {
    //
    // See if the l2_op1 corresponds:
    //
    if (Is_16_Bits(tninfo1, bb, &tmp_tn1, &tmp_tninfo1, &tmp_hilo1, &tmp_signed1)) {
      if (tmp_tn0 == tmp_tn1 && tmp_hilo0 == tmp_hilo1 && tmp_signed0 == tmp_signed1) {
	reduce_tn0 = TRUE;
	l2_tn0 = tmp_tn0;
	l2_tninfo0 = tmp_tninfo0;
	l2_hilo0 = tmp_hilo0;
	l2_signed0 = tmp_signed0;
      }
    }
  }

  // If tn0 can not be strength reduced, make sure it is the same
  // operand for both, l2_op0 and l2_op1
  if (!reduce_tn0) {
    if (tn0 != tn1) return FALSE;

    l2_tn0 = tn0;
    l2_tninfo0 = tninfo0;
    l2_hilo0 = TN_32_BITS;
    l2_signed0 = SIGN_UNKNOWN;
  }

  //
  // Process operand 1:
  //
  tn0 = OP_opnd(l2_op0,1);
  tn1 = OP_opnd(l2_op1,1);
  tninfo0 = l2_opinfo0->actual_opnd[1];
  tninfo1 = l2_opinfo1->actual_opnd[1];

  if (Is_16_Bits(tninfo0, bb, &tmp_tn0, &tmp_tninfo0, &tmp_hilo0, &tmp_signed0)) {
    //
    // See if the l2_op1 corresponds:
    //
    if (Is_16_Bits(tninfo1, bb, &tmp_tn1, &tmp_tninfo1, &tmp_hilo1, &tmp_signed1)) {
      if (tmp_tn0 == tmp_tn1 && tmp_hilo0 == tmp_hilo1 && tmp_signed0 == tmp_signed1) {
	reduce_tn1 = TRUE;
	l2_tn1 = tmp_tn0;
	l2_tninfo1 = tmp_tninfo0;
	l2_hilo1 = tmp_hilo0;
	l2_signed1 = tmp_signed0;
      }
    }
  }

  // If tn1 can not be strength reduced, make sure it is the same
  // operand for both, l2_op0 and l2_op1
  if (!reduce_tn1) {
    if (tn0 != tn1) return FALSE;

    l2_tn1 = tn0;
    l2_tninfo1 = tninfo0;
    l2_hilo1 = TN_32_BITS;
    l2_signed1 = SIGN_UNKNOWN;
  }

  // Continue if any of the operands is being strength reduced
  if (!reduce_tn0 && !reduce_tn1) {
    if (EBO_Trace_Optimization) 
      fprintf(TFile, "No stength reduction for 32x32 mul\n");
    return FALSE;
  }

  // Before looking for a possible strength reduction, check if
  // the l2_tn1 is a 32 bit quantity. Since there are no TOP
  // codes with 32 bit operand in the second position, try to
  // swap the operands.
  // Also for hi x lo combination
  if (((l2_hilo0 != TN_32_BITS && l2_hilo1 == TN_32_BITS) ||
      (l2_hilo0 == TN_HI_16 && l2_hilo1 == TN_LO_16)) && 
      !TN_is_constant(l2_tn1)) {
      //
      // Swap the operands -- mul is associative
      //
      TN *tmp_tn = l2_tn0;
      EBO_TN_INFO *tmp_tninfo = l2_tninfo0;
      BITS_POS tmp_hilo = l2_hilo0;
      SIGNDNESS tmp_signed = l2_signed0;
      l2_tn0 = l2_tn1;
      l2_tninfo0 = l2_tninfo1;
      l2_hilo0 = l2_hilo1;
      l2_signed0 = l2_signed1;
      l2_tn1 = tmp_tn;
      l2_tninfo1 = tmp_tninfo;
      l2_hilo1 = tmp_hilo;
      l2_signed1 = tmp_signed;
  }

  // Now we can choose an appropriate opcode
  TOP new_opcode = get_mul_opcode(l2_signed0, l2_hilo0,
				  l2_signed1, l2_hilo1);

  // Convert to an immediate form if l2_tn1 is an immediate
  if (TN_is_symbol(l2_tn1)) return FALSE;
  if (new_opcode != TOP_UNDEFINED && TN_has_value(l2_tn1))
    new_opcode = TOP_opnd_immediate_variant(new_opcode, 1, TN_value(l2_tn1));

  if (new_opcode == TOP_UNDEFINED) {
    if (EBO_Trace_Optimization)
      fprintf(TFile, "No opcode for stength reduction\n");
    return FALSE;
  }

  // case of no change
  if (new_opcode == opcode && l2_tn0 == l1_tn0 && l2_tn1 == l1_tn1)
    return FALSE;

  // [CG]: Check redefinitions
  if (!EBO_tn_available (bb, l2_tninfo0) ||
      !EBO_tn_available (bb, l2_tninfo1)) {
    if (EBO_Trace_Optimization) 
      fprintf(TFile,"Convert add-mul sequence stopped due to redefinition\n");
    return FALSE;
  }

  //
  // Now, we have :
  //
  //    new_opcode
  //    l1_res
  //    l2_tn0
  //    l3_tn1
  //
  // Make a l1_res = mulh l3_tn0, l3_tn1, and replace the
  // current instruction:
  //
  OP *new_op;
  new_op = Mk_OP(new_opcode, l1_res, l2_tn0, l2_tn1);
  OP_srcpos(new_op) = OP_srcpos(op);
  if (EBO_in_loop) EBO_Set_OP_omega (new_op, l2_tninfo0, l2_tninfo1);
  BB_Insert_Op_After(bb, op, new_op);
  if (EBO_Trace_Optimization) 
    fprintf(TFile,"Convert add-mul sequence\n");
  return TRUE;
}

/* =====================================================================
 * Function: iadd_special_case
 *
 * Look for integer add instructions where the first operand is
 * relocated address that is at the top of the stack.  The add
 * is unnecessary.
 * =====================================================================
 */
static
BOOL
iadd_special_case (
  OP *op,
  TN **opnd_tn,
  EBO_TN_INFO **opnd_tninfo
)
{
  if (OP_code(op) == TOP_spadjust) return FALSE;

  TN *tn = opnd_tn[0];
  if (TN_is_constant(tn) &&
      !TN_has_spill(tn) &&
      TN_is_symbol(tn)) {
    ST *st = TN_var(tn);
    ST *base_st;
    INT64 base_ofst;
    INT64 val = TN_offset(tn);

    Base_Symbol_And_Offset (st, &base_st, &base_ofst);
    if (ST_on_stack(st) &&
        ((ST_sclass(st) == SCLASS_AUTO) ||
         (EBO_in_peep && (ST_sclass(st) == SCLASS_FORMAL)))) {
      val += base_ofst;
      if (val == 0) {
        OPS ops = OPS_EMPTY;
        EBO_Exp_COPY(NULL, OP_result(op, 0), opnd_tn[1], &ops);

	if (!EBO_Verify_Ops(&ops)) return FALSE;
        OP_srcpos(OPS_first(&ops)) = OP_srcpos(op);
	if (EBO_in_loop) 
	  EBO_OPS_omega (&ops, opnd_tn[1], opnd_tninfo[1]);
        BB_Insert_Ops(OP_bb(op), op, &ops, FALSE);
	if (EBO_Trace_Optimization) 
	  fprintf(TFile,"Replace iadd with copy\n");
        return TRUE;
      }
    }
  }
  return FALSE;
}

/* =====================================================================
 *   Function: mulhh_sequence
 *
 *   Look for mulh(shr16) sequence and replace with a single mulhh
 *   instruction. 
 * =====================================================================
 */
static BOOL
mulhh_sequence (
  OP *op,
  TN **opnd_tn,
  EBO_TN_INFO **opnd_tninfo
)
{
  TOP opcode = OP_code(op);

  if (opcode != TOP_mulh_i && 
      opcode != TOP_mulh_ii &&
      opcode != TOP_mulh_r &&
      opcode != TOP_mulhu_i && 
      opcode != TOP_mulhu_ii &&
      opcode != TOP_mulhu_r) 
    return FALSE;

  // Level 1 data:
  BB *bb = OP_bb(op);
  OP *l1_op = op;
  TN *l1_tn0 = OP_opnd(l1_op, 0);
  TN *l1_tn1 = OP_opnd(l1_op, 1);
  TN *l1_res = OP_result(l1_op, 0);
  EBO_TN_INFO *l1_tninfo0 = opnd_tninfo[0];
  EBO_TN_INFO *l1_tninfo1 = opnd_tninfo[1];

  // Input to the mulh must be a shr 16
  if (l1_tninfo0 == NULL || l1_tninfo0->in_op == NULL) {
    return FALSE;
  }

  // Determine the inputs to the mulh instruction:
  EBO_OP_INFO *l2_opinfo = locate_opinfo_entry(l1_tninfo0);
  if ((l2_opinfo == NULL) ||
      (l2_opinfo->in_op == NULL)) return FALSE;

  OP *l2_op = l2_opinfo->in_op;

  if (OP_code(l2_op) != TOP_shr_i &&
      OP_code(l2_op) != TOP_shru_i) return FALSE;

  TN *l2_tn0 = OP_opnd(l2_op, 0);
  TN *l2_tn1 = OP_opnd(l2_op, 1);
  if (!TN_has_value(l2_tn1) || TN_value(l2_tn1) != 16)
    return FALSE;

  EBO_TN_INFO *l2_tninfo0 = l2_opinfo->actual_opnd[0];
  if (!EBO_tn_available (bb, l2_tninfo0)) return FALSE;

  // Determine new opcode:
  TOP new_opcode = TOP_UNDEFINED;
  switch (opcode) {
  case TOP_mulh_i: 
    new_opcode = (OP_code(l2_op) == TOP_shr_i) ? TOP_mulhh_i : TOP_mulhhu_i;
    break;
  case TOP_mulh_ii: 
    new_opcode = (OP_code(l2_op) == TOP_shr_i) ? TOP_mulhh_ii : TOP_mulhhu_ii;
    break;
  case TOP_mulh_r: 
    new_opcode = (OP_code(l2_op) == TOP_shr_i) ? TOP_mulhh_r : TOP_mulhhu_r;
    break;
  case TOP_mulhu_i: 
    new_opcode = (OP_code(l2_op) == TOP_shru_i) ? TOP_mulhhu_i : TOP_UNDEFINED;
    break;
  case TOP_mulhu_ii: 
    new_opcode = (OP_code(l2_op) == TOP_shru_i) ? TOP_mulhhu_ii : TOP_UNDEFINED;
    break;
  case TOP_mulhu_r: 
    new_opcode = (OP_code(l2_op) == TOP_shru_i) ? TOP_mulhhu_r : TOP_UNDEFINED;
    break;
  }

  if (new_opcode == TOP_UNDEFINED)
    return FALSE;

  // case of no change
  if (new_opcode == opcode && l2_tn0 == l1_tn0)
    return FALSE;

  // Replace the current instruction:
  OP *new_op;
  new_op = Mk_OP(new_opcode, l1_res, l2_tn0, l1_tn1);
  OP_srcpos(new_op) = OP_srcpos(op);
  if (EBO_in_loop) EBO_Set_OP_omega (new_op, l1_tninfo1, l2_tninfo0);
  BB_Insert_Op_After(bb, op, new_op);
  if (EBO_Trace_Optimization) 
    fprintf(TFile,"Convert mulh(shr) to mulhh\n");
  return TRUE;
}

/* =====================================================================
 *   Function: mul_32_16_sequence
 *
 *   Try to strength reduce the 32 x half-word multiplies.
 * =====================================================================
 */
static BOOL
mul_32_16_sequence (
  OP *op,
  TN **opnd_tn,
  EBO_TN_INFO **opnd_tninfo
)
{
  TOP opcode = OP_code(op);

  if (!IS_MULL(op) && !IS_MULH(op))
    return FALSE;

  // Level 1 data:
  BB *bb = OP_bb(op);
  TN *tn0 = OP_opnd(op, 0);
  TN *tn1 = OP_opnd(op, 1);
  TN *res = OP_result(op, 0);

  TN *new_tn0 = tn0, *new_tn1 = tn1;

  EBO_TN_INFO *tninfo0 = opnd_tninfo[0];
  EBO_TN_INFO *tninfo1 = opnd_tninfo[1];
  BITS_POS hilo0;
  BITS_POS hilo1 = IS_MULL(op) ? TN_LO_16 : TN_HI_16;
  SIGNDNESS signed0;
  SIGNDNESS signed1 = TOP_is_unsign(opcode) ? ZERO_EXT : SIGN_EXT;
  
  if (tninfo0 == NULL || tninfo0->in_op == NULL) {
    return FALSE;
  }

  if (!Is_16_Bits(opnd_tninfo[0],
		  bb, 
		  &new_tn0, 
		  &tninfo0, 
		  &hilo0, 
		  &signed0))
    return FALSE;

  // 
  // If we got mul hi 16 x lo 16 we should swap operands since we
  // do not have this on the machine.
  //
  if (hilo0 == TN_HI_16 && hilo1 == TN_LO_16 && !TN_is_constant(new_tn1)) {
      //
      // Swap the operands -- mul is associative
      //
      TN *tmp_tn = new_tn0;
      EBO_TN_INFO *tmp_tninfo = tninfo0;
      BITS_POS tmp_hilo = hilo0;
      SIGNDNESS tmp_signed = signed0;
      new_tn0 = new_tn1;
      tninfo0 = tninfo1;
      hilo0 = hilo1;
      signed0 = signed1;
      new_tn1 = tmp_tn;
      tninfo1 = tmp_tninfo;
      hilo1 = tmp_hilo;
      signed1 = tmp_signed;
  }

  // Determine new opcode:
  TOP new_opcode = get_mul_opcode(signed0, hilo0,
				  signed1, hilo1);

  // Convert to an immediate form if tn1 is an immediate
  if (TN_is_symbol(new_tn1)) return FALSE;
  if (new_opcode != TOP_UNDEFINED && TN_has_value(new_tn1))
    new_opcode = TOP_opnd_immediate_variant(new_opcode, 1, TN_value(new_tn1));

  if (new_opcode == TOP_UNDEFINED)
    return FALSE;

  // case of no change
  if (new_opcode == opcode && new_tn0 == tn0 && new_tn1 == tn1) 
    return FALSE;

  // [CG]: Check redefinitions
  if (!EBO_tn_available (bb, tninfo0) ||
      !EBO_tn_available (bb, tninfo1)) {
    if (EBO_Trace_Optimization) 
      fprintf(TFile,"Convert 32x16 sequence stopped due to redefinition\n");
    return FALSE;
  }

  // Replace the current instruction:
  OP *new_op;
  new_op = Mk_OP(new_opcode, res, new_tn0, new_tn1);
  OP_srcpos(new_op) = OP_srcpos(op);
  if (EBO_in_loop) EBO_Set_OP_omega (new_op, tninfo0, tninfo1);
  BB_Insert_Op_After(bb, op, new_op);
  if (EBO_Trace_Optimization) 
    fprintf(TFile,"Convert %s to %s\n", TOP_Name(opcode), TOP_Name(new_opcode));
  return TRUE;
}

/* =====================================================================
 *   Function: mulhs_sequence
 *
 *   Look for shl16(mulh) sequence and replace with a single mulhs
 *   instruction. 
 * =====================================================================
 */
static BOOL
shl_mulhs_sequence (
  OP *op,
  TN **opnd_tn,
  EBO_TN_INFO **opnd_tninfo
)
{
  TOP opcode = OP_code(op);

  if (opcode != TOP_shl_i)
    return FALSE;

  // Level 1 data:
  BB *bb = OP_bb(op);
  OP *l1_op = op;
  TN *l1_tn1 = OP_opnd(l1_op, 1);
  TN *l1_res = OP_result(l1_op, 0);

  // shl must be by 16 bits
  if (!TN_has_value(l1_tn1) || TN_value(l1_tn1) != 16)
    return FALSE;

  EBO_TN_INFO *l1_tninfo0 = opnd_tninfo[0];

  // Input to the shl must be a mulh
  if (l1_tninfo0 == NULL || l1_tninfo0->in_op == NULL) {
    return FALSE;
  }

  // Determine the inputs to the shl instruction:
  EBO_OP_INFO *l2_opinfo = locate_opinfo_entry(l1_tninfo0);
  if ((l2_opinfo == NULL) ||
      (l2_opinfo->in_op == NULL)) return FALSE;

  OP *l2_op = l2_opinfo->in_op;

  if (OP_code(l2_op) != TOP_mulh_i &&
      OP_code(l2_op) != TOP_mulh_ii &&
      OP_code(l2_op) != TOP_mulh_r &&
      OP_code(l2_op) != TOP_mulhu_i &&
      OP_code(l2_op) != TOP_mulhu_ii &&
      OP_code(l2_op) != TOP_mulhu_r) return FALSE;

  EBO_TN_INFO *l2_tninfo0 = l2_opinfo->actual_opnd[0];
  EBO_TN_INFO *l2_tninfo1 = l2_opinfo->actual_opnd[1];
  if (!EBO_tn_available (bb, l2_tninfo0) ||
      !EBO_tn_available (bb, l2_tninfo1)) return FALSE;

  TN *l2_tn0 = OP_opnd(l2_op,0);
  TN *l2_tn1 = OP_opnd(l2_op,1);

  // Determine new opcode:
  TOP new_opcode;
  switch (OP_code(l2_op)) {
  case TOP_mulh_i: 
  case TOP_mulhu_i: 
    new_opcode = TOP_mulhs_i; break;
  case TOP_mulh_ii: 
  case TOP_mulhu_ii: 
    new_opcode = TOP_mulhs_ii; break;
  case TOP_mulh_r: 
  case TOP_mulhu_r: 
    new_opcode = TOP_mulhs_r; break;
  default:
    FmtAssert(FALSE, (" wrong opcode %s\n", TOP_Name(OP_code(l2_op))));
  }

  // Replace the current instruction:
  OP *new_op;
  new_op = Mk_OP(new_opcode, l1_res, l2_tn0, l2_tn1);
  OP_srcpos(new_op) = OP_srcpos(op);
  if (EBO_in_loop) EBO_Set_OP_omega (new_op, l2_tninfo0, l2_tninfo1);
  BB_Insert_Op_After(bb, op, new_op);
  if (EBO_Trace_Optimization) 
    fprintf(TFile,"Convert shl16(mulh) to mulhs\n");
  return TRUE;
}

/* =====================================================================
 *   Function: shr_shl_sequence
 *
 *   Look for shr16(shl16) sequence and replace with a single sxth/zxth
 *   instruction. 
 * =====================================================================
 */
static BOOL
shr_shl_sequence (
  OP *op,
  TN **opnd_tn,
  EBO_TN_INFO **opnd_tninfo
)
{
  TOP opcode = OP_code(op);

  if (!IS_SHR_16(op))
    return FALSE;

  // Level 1 data:
  BB *bb = OP_bb(op);
  OP *l1_op = op;
  TN *l1_res = OP_result(l1_op, 0);

  EBO_TN_INFO *l1_tninfo0 = opnd_tninfo[0];

  if (l1_tninfo0 == NULL || l1_tninfo0->in_op == NULL) {
    return FALSE;
  }

  // Determine the inputs to the shr instruction:
  EBO_OP_INFO *l2_opinfo = locate_opinfo_entry(l1_tninfo0);
  if ((l2_opinfo == NULL) ||
      (l2_opinfo->in_op == NULL)) return FALSE;

  OP *l2_op = l2_opinfo->in_op;

  if (!IS_SHL_16(l2_op))
    return FALSE;

  EBO_TN_INFO *l2_tninfo0 = l2_opinfo->actual_opnd[0];
  if (!EBO_tn_available (bb, l2_tninfo0))
    return FALSE;

  TN *l2_tn0 = OP_opnd(l2_op,0);

  // Determine new opcode:
  TOP new_opcode;
  new_opcode = TOP_is_unsign(opcode) ? TOP_zxth_r : TOP_sxth_r;

  // Replace the current instruction:
  OP *new_op;
  new_op = Mk_OP(new_opcode, l1_res, l2_tn0);
  OP_srcpos(new_op) = OP_srcpos(op);
  if (EBO_in_loop) EBO_Set_OP_omega (new_op, l2_tninfo0);
  BB_Insert_Op_After(bb, op, new_op);
  if (EBO_Trace_Optimization) 
    fprintf(TFile,"Convert shr16(shl16) to sxth/zxth\n");
  return TRUE;
}

/* =====================================================================
 *    mul_fix_operands
 *
 *    Remove necessary OPs from 16 bit mul operand computation.
 * =====================================================================
 */
static BOOL
mul_fix_operands (
  OP *op,
  TN **opnd_tn,
  EBO_TN_INFO **opnd_tninfo
)
{
  TOP opcode = OP_code(op);
  BB *bb = OP_bb(op);
  TN *res;

  // Check operands
  if (OP_results(op) != 1 ||
      OP_opnds(op) != 2 ||
      OP_opnd(op, 0) != opnd_tn[0] ||
      OP_opnd(op, 1) != opnd_tn[1]) return FALSE;
  
  BITS_POS top_hilo0;
  BITS_POS top_hilo1;
  SIGNDNESS top_signed0;
  SIGNDNESS top_signed1;
  // Check opcode
  if (IS_MULL(op) || IS_MULH(op)) {
    top_hilo0 = TN_32_BITS;
    top_signed0 = SIGN_UNKNOWN;
    top_hilo1 = IS_MULL(op) ? TN_LO_16 : TN_HI_16;
    top_signed1 = TOP_is_unsign(opcode) ? ZERO_EXT : SIGN_EXT;
  } else if (IS_MULLL(op) || IS_MULLH(op)) {
    top_hilo0 = TN_LO_16;
    top_signed0 = TOP_is_unsign(opcode) ? ZERO_EXT : SIGN_EXT;
    top_hilo1 = IS_MULLL(op) ? TN_LO_16 : TN_HI_16;
    top_signed1 = TOP_is_unsign(opcode) ? ZERO_EXT : SIGN_EXT;
  } else return FALSE;

  res = OP_result(op, 0);

  BOOL swapped = FALSE;
do_swapped:
  // Level 1 data:
  TN *tn0 = swapped ? opnd_tn[1]: opnd_tn[0];
  TN *tn1 = swapped ? opnd_tn[0]: opnd_tn[1];
  EBO_TN_INFO *tninfo0 = swapped ? opnd_tninfo[1] : opnd_tninfo[0];
  EBO_TN_INFO *tninfo1 = swapped ? opnd_tninfo[0] : opnd_tninfo[1];

  if (EBO_Trace_Optimization) 
    fprintf(TFile,"In mul_fix_operands  for (%s, %s) x (%s, %s): %s\n", SIGNDNESS_Name(top_signed0), BITS_POS_Name(top_hilo0),  SIGNDNESS_Name(top_signed1), BITS_POS_Name(top_hilo1), TOP_Name(opcode));
  
  
  TN *tmp_tn;
  EBO_TN_INFO *tmp_tninfo;
  BITS_POS tmp_hilo;
  SIGNDNESS tmp_signed;

  TN *new_tn;
  TN *new_tn0 = tn0, *new_tn1 = tn1;
  EBO_TN_INFO *new_tninfo;
  BITS_POS new_hilo;
  SIGNDNESS new_signed;
  TOP new_opcode = TOP_UNDEFINED;
  TOP new_opcode0 = TOP_UNDEFINED;
  TOP new_opcode1 = TOP_UNDEFINED;

  /* Process first operand. */
  if (top_hilo0 == TN_LO_16) {
    if (Is_16_Bits(tninfo0, bb, &tmp_tn, &tmp_tninfo, &tmp_hilo, &tmp_signed) &&
	(tmp_hilo == TN_LO_16 || tmp_hilo == TN_HI_16) &&
	tmp_signed != SIGN_UNKNOWN) {
      new_tn = tmp_tn;
      new_tninfo = tmp_tninfo;
      new_hilo = tmp_hilo;
      new_signed = top_signed0;
      // Determine new opcode
      new_opcode0 = get_mul_opcode(new_signed, new_hilo,
				   top_signed1, top_hilo1);
    }
  } else if (top_hilo0 == TN_HI_16) {
    if (Is_16_Bits(tninfo0, bb, &tmp_tn, &tmp_tninfo, &tmp_hilo, &tmp_signed) &&
	new_hilo == TN_LO_16 && new_signed == SIGN_UNKNOWN) {
      new_tn = tmp_tn;
      new_tninfo = tmp_tninfo;
      new_hilo = TN_LO_16;
      new_signed = top_signed0;
      // Determine new opcode
      new_opcode0 = get_mul_opcode(new_signed, new_hilo,
				   top_signed1, top_hilo1);
    }
  }
  
  if (new_opcode0 != TOP_UNDEFINED) {
    new_tn0 = new_tn;
    tninfo0 = new_tninfo;
    top_hilo0 = new_hilo;
    top_signed0 = new_signed;
    new_opcode = new_opcode0;
  }
  
  /* Process second operand. */
  if (top_hilo1 == TN_LO_16) {
    if (Is_16_Bits(tninfo1, bb, &tmp_tn, &tmp_tninfo, &tmp_hilo, &tmp_signed) &&
	(tmp_hilo == TN_LO_16 || tmp_hilo == TN_HI_16) &&
	tmp_signed != SIGN_UNKNOWN) {
      new_tn = tmp_tn;
      new_tninfo = tmp_tninfo;
      new_hilo = tmp_hilo;
      new_signed = top_signed1;
      // Determine new opcode
      new_opcode1 = get_mul_opcode(top_signed0, top_hilo0,
				   new_signed, new_hilo);
    }
  } else if (top_hilo1 == TN_HI_16) {
    if (Is_16_Bits(tninfo1, bb, &tmp_tn, &tmp_tninfo, &tmp_hilo, &tmp_signed) &&
	tmp_hilo == TN_LO_16 && tmp_signed == SIGN_UNKNOWN) {
      new_tn = tmp_tn;
      new_tninfo = tmp_tninfo;
      new_hilo = TN_LO_16;
      new_signed = top_signed1;
      // Determine new opcode
      new_opcode1 = get_mul_opcode(top_signed0, top_hilo0,
				   new_signed, new_hilo);
    }
  }
  
  if (new_opcode1 != TOP_UNDEFINED) {
    new_tn1 = new_tn;
    tninfo1 = new_tninfo;
    top_hilo1 = new_hilo;
    top_signed1 = new_signed;
    new_opcode = new_opcode1;
  }
  
  if (new_opcode == TOP_UNDEFINED) {
    if (swapped) return FALSE;
    // Try to swap operands if possible
    TOP swapped_opcode = TOP_opnd_swapped_variant(opcode, 0, 1);
    if (swapped_opcode != opcode) return FALSE;
    swapped = TRUE;
    goto do_swapped;
  }

  // Convert to an immediate form if tn1 is an immediate
  if (TN_is_symbol(new_tn1)) return FALSE;
  if (new_opcode != TOP_UNDEFINED && TN_has_value(new_tn1))
    new_opcode = TOP_opnd_immediate_variant(new_opcode, 1, TN_value(new_tn1));
  
  // case of no change
  if (new_opcode == opcode && new_tn0 == tn0 && new_tn1 == tn1) 
    return FALSE;
  
  // [CG]: Check redefinitions
  if (!EBO_tn_available (bb, tninfo0) ||
      !EBO_tn_available (bb, tninfo1)) {
    if (EBO_Trace_Optimization) 
      fprintf(TFile,"Convert mul_fix_operand sequence stopped due to redefinition\n");
    return FALSE;
  }

  // Replace the current instruction:
  OP *new_op;
  new_op = Mk_OP(new_opcode, res, new_tn0, new_tn1);
  OP_srcpos(new_op) = OP_srcpos(op);
  if (EBO_in_loop) EBO_Set_OP_omega (new_op, tninfo0, tninfo1);
  BB_Insert_Op_After(bb, op, new_op);
  if (EBO_Trace_Optimization) 
    fprintf(TFile,"Convert %s to %s\n", TOP_Name(opcode), TOP_Name(new_opcode));
  return TRUE;
}

/* =====================================================================
 *    Strength_Reduce_Mul
 * =====================================================================
 */
static BOOL
Strength_Reduce_Mul (
  OP *op,
  TN **opnd_tn,
  EBO_TN_INFO **opnd_tninfo
)
{
  if (OP_code(op) == TOP_add_r) {
    return (add_mul_sequence (op, opnd_tn, opnd_tninfo));
  }

  if (!(EBO_in_pre && IS_MUL32_PART(op))) {
    // Do not do it before the 32x32 multiplies have been
    // reduced. However, for now we only have pre_process vs
    // process choice. Perhaps, eventually have several 
    // pre_process passes ?
    return (mul_32_16_sequence (op, opnd_tn, opnd_tninfo) ||
	    mulhh_sequence (op, opnd_tn, opnd_tninfo));
  }

  return FALSE;
}

static BOOL
find_def_opinfo(EBO_TN_INFO *input_tninfo, EBO_OP_INFO **def_opinfo)
{
  EBO_OP_INFO *opinfo;
  opinfo = locate_opinfo_entry(input_tninfo);
  if (opinfo == NULL || opinfo->in_op == NULL) return FALSE;
  *def_opinfo = opinfo;
  return TRUE;
}

static BOOL
op_match_integer_branch_copy(OP *op, EBO_TN_INFO **opnd_tninfo, TN **match_tn, EBO_TN_INFO **match_tninfo)
{
  TOP top = OP_code(op);
  EBO_TN_INFO *tninfo;
  TN *tn;
  
  if (top == TOP_mtb) {
    tninfo = opnd_tninfo[0];
    tn = OP_opnd(op, 0);
    goto matched;
  }
  return FALSE;
  
 matched:
  if (tninfo != NULL && !EBO_tn_available (OP_bb(op), tninfo)) return FALSE;
  *match_tninfo = tninfo;
  *match_tn = tn;
  return TRUE;
}

static BOOL
op_match_branch_integer_copy(OP *op, EBO_TN_INFO **opnd_tninfo, TN **match_tn, EBO_TN_INFO **match_tninfo)
{
  TOP top = OP_code(op);
  EBO_TN_INFO *tninfo;
  TN *tn;

  if (top == TOP_mfb) {
    tninfo = opnd_tninfo[0];
    tn = OP_opnd(op, 0);
    goto matched;
  }
  return FALSE;

 matched:
  if (tninfo != NULL && !EBO_tn_available (OP_bb(op), tninfo)) return FALSE;
  *match_tninfo = tninfo;
  *match_tn = tn;
  return TRUE;
}

static BOOL
op_match_branch_branch_copy(OP *op, EBO_TN_INFO **opnd_tninfo, TN **match_tn, EBO_TN_INFO **match_tninfo)
{
  EBO_TN_INFO *input1_tninfo;
  TN *input1_tn;
  
  // Get source of integer->branch copy
  if (!op_match_integer_branch_copy(op, opnd_tninfo, &input1_tn, &input1_tninfo)) return FALSE;
  if (!EBO_tn_available (OP_bb(op), input1_tninfo)) return FALSE;
  
  // Get source of branch->integer copy
  EBO_OP_INFO *def2_opinfo;
  if (!find_def_opinfo(input1_tninfo, &def2_opinfo)) return FALSE;

  EBO_TN_INFO *input2_tninfo;
  TN *input2_tn;
  if (!op_match_branch_integer_copy(def2_opinfo->in_op, def2_opinfo->actual_opnd, &input2_tn, &input2_tninfo)) return FALSE;
  if (!EBO_tn_available (OP_bb(op), input2_tninfo)) return FALSE;
  *match_tninfo = input2_tninfo;
  *match_tn = input2_tn;
  return TRUE;
}

static BOOL
op_match_lnot(OP *op, 
	     EBO_TN_INFO **opnd_tninfo, 
	     TN **op0_tn, 
	     EBO_TN_INFO **op0_tninfo)
{
  TOP top = OP_code(op);
  TN *opnd;

  if (top == TOP_cmpeq_r_b || top == TOP_cmpeq_r_r) {
    if (OP_opnd(op, 0) == Zero_TN) {
      *op0_tn = OP_opnd(op, 1);
      *op0_tninfo = opnd_tninfo[1];
      return TRUE;
    } else if (OP_opnd(op, 1) == Zero_TN) {
      *op0_tn = OP_opnd(op, 0);
      *op0_tninfo = opnd_tninfo[0];
      return TRUE;
    }
  }
  return FALSE;
}

static BOOL
op_match_lmove(OP *op, 
	       EBO_TN_INFO **opnd_tninfo, 
	       TN **op0_tn, 
	       EBO_TN_INFO **op0_tninfo)
{
  TOP top = OP_code(op);
  TN *opnd;

  if (top == TOP_cmpne_r_b || top == TOP_cmpne_r_r) {
    if (OP_opnd(op, 0) == Zero_TN) {
      *op0_tn = OP_opnd(op, 1);
      *op0_tninfo = opnd_tninfo[1];
      return TRUE;
    } else if (OP_opnd(op, 1) == Zero_TN) {
      *op0_tn = OP_opnd(op, 0);
      *op0_tninfo = opnd_tninfo[0];
      return TRUE;
    }
  }
  return FALSE;
}


static BOOL
is_same_bits(OP *op, EBO_TN_INFO **opnd_tninfo, int use_bits, TN **match_tn, EBO_TN_INFO **match_tninfo)
{
  TOP opcode = OP_code(op);
  int opnd1_idx = TOP_Find_Operand_Use(opcode,OU_opnd1);
  int opnd2_idx = TOP_Find_Operand_Use(opcode,OU_opnd2);
  EBO_TN_INFO *tninfo;
  TN *tn;

  if (OP_sext(op) || OP_zext(op)) {
    int bits = TOP_opnd_use_bits(opcode, opnd1_idx); 
    if (bits >= use_bits) {
      tn = OP_opnd(op, opnd1_idx);
      tninfo = opnd_tninfo[opnd1_idx];
      goto matched;
    }
  }
  
  if (OP_iand(op)) {
    TN *opnd2_tn = OP_opnd(op, opnd2_idx);
    if (TN_Has_Value(opnd2_tn)) {
      INT64 val = TN_Value(opnd2_tn);
      INT64 bitmask = (UINT64)-1 >> 64-use_bits;
      if ((val & bitmask) == bitmask) {
	tn = OP_opnd(op, opnd1_idx);
	tninfo = opnd_tninfo[opnd1_idx];
	goto matched;
      }
    }
  }

  if (OP_ior(op) || OP_ixor(op)) {
    TN *opnd2_tn = OP_opnd(op, opnd2_idx);
    if (TN_Has_Value(opnd2_tn)) {
      INT64 val = TN_Value(opnd2_tn);
      INT64 bitmask = (UINT64)-1 >> 64-use_bits;
      if ((val ^ bitmask) == bitmask) {
	tn = OP_opnd(op, opnd1_idx);
	tninfo = opnd_tninfo[opnd1_idx];
	goto matched;
      }
    }
  }
  
  return FALSE;

 matched:
  if (tninfo != NULL && !EBO_tn_available (OP_bb(op), tninfo)) return FALSE;
  *match_tninfo = tninfo;
  *match_tn = tn;
  return TRUE;

}


static BOOL
find_equivalent_tn(OP *op, EBO_TN_INFO *input_tninfo, int use_bits, TN **equiv_tn, EBO_TN_INFO **equiv_tninfo)
{
  EBO_OP_INFO *def_opinfo;
  EBO_TN_INFO *match_tninfo;
  TN *match_tn;

  if (input_tninfo == NULL || input_tninfo->in_op == NULL) return FALSE;
  
   // Get defining op
  if (!find_def_opinfo(input_tninfo, &def_opinfo)) return FALSE;
  
  if (!is_same_bits(def_opinfo->in_op, def_opinfo->actual_opnd, use_bits, &match_tn, &match_tninfo))
    return FALSE;
  if (match_tninfo != NULL && !EBO_tn_available (OP_bb(op), match_tninfo)) return FALSE;
      
  *equiv_tninfo = match_tninfo;
  *equiv_tn = match_tn;
      
  return TRUE;
}


/*
 * operand_special_sequence.
 *
 * Try to discard useless operations defining an operand.
 * For instance sth(sxth(x)) -> sth(x)
 */
static BOOL
operand_special_sequence(OP *op, TN **opnd_tn, EBO_TN_INFO **opnd_tninfo)
{
  TN *new_opnd_tn[OP_MAX_FIXED_OPNDS];
  int i;
  int replaced = 0;
  INT num_opnds = OP_opnds(op);

  if (num_opnds > OP_MAX_FIXED_OPNDS) return FALSE;

  // Don't propagate into mul until mul32x32 are reduced.
  if (EBO_in_pre && IS_MUL32_PART(op)) return FALSE;

  for (i = 0; i < num_opnds; i++) {
    EBO_TN_INFO *tninfo, *new_tninfo;
    TN *new_tn;
    int use_bits;
    tninfo = opnd_tninfo[i];
    if (tninfo == NULL ||
	!use_bit_width(op, i, &use_bits) ||
	!find_equivalent_tn(op, tninfo, use_bits, &new_tn, &new_tninfo) ||
	!TN_is_register(new_tn) ||
	TN_register_class(new_tn) != TN_register_class(OP_opnd(op, i))) {
      new_opnd_tn[i] = OP_opnd(op, i);
    } else {
      new_opnd_tn[i] = new_tn;
      replaced++;
    }
  }
  if (replaced == 0) return FALSE;

  /* Replace the current instruction. */
  OP *new_op;
  new_op = Dup_OP (op);
  OP_srcpos(new_op) = OP_srcpos(op);
  if (OP_memory(op)) Copy_WN_For_Memory_OP (new_op, op);
  if (EBO_in_loop) CG_LOOP_Init_Op(new_op);
  for (i = 0; i < num_opnds; i++) {
    Set_OP_opnd(new_op, i, new_opnd_tn[i]);
    if (EBO_in_loop) Set_OP_omega (new_op, i, OP_omega(op,i));
  }

  BB_Insert_Op_After(OP_bb(op), op, new_op);

  if (EBO_Trace_Optimization) 
    fprintf(TFile,"Propagated equivalent tns into %s\n", TOP_Name(OP_code(new_op)));
  return TRUE;
}

static BOOL
op_match_select(OP *op, 
		EBO_TN_INFO **opnd_tninfo, 
		TN **op0_tn, 
		EBO_TN_INFO **op0_tninfo, 
		TN **op1_tn, 
		EBO_TN_INFO **op1_tninfo, 
		TN **op2_tn, 
		EBO_TN_INFO **op2_tninfo)
{
  TOP top = OP_code(op);
  VARIANT variant;

  if (!TOP_is_select(top)) return FALSE;

  variant = TOP_cond_variant(top);

  *op0_tn = OP_opnd(op, 0);
  *op0_tninfo = opnd_tninfo[0];
  *op1_tn = variant == V_COND_FALSE ? OP_opnd(op, 2): OP_opnd(op, 1);
  *op1_tninfo = variant == V_COND_FALSE ? opnd_tninfo[2] : opnd_tninfo[1];
  *op2_tn = variant == V_COND_FALSE ? OP_opnd(op, 1): OP_opnd(op, 2);
  *op2_tninfo = variant == V_COND_FALSE ? opnd_tninfo[1] : opnd_tninfo[2];
  return TRUE;
}

static BOOL
op_match_compare(OP *op, 
		 EBO_TN_INFO **opnd_tninfo, 
		 TN **op0_tn, 
		 EBO_TN_INFO **op0_tninfo, 
		 TN **op1_tn, 
		 EBO_TN_INFO **op1_tninfo)
{
  TOP top = OP_code(op);
  
  if (!TOP_is_cmp(top)) return FALSE;

  *op0_tn = OP_opnd(op, 0);
  *op0_tninfo = opnd_tninfo[0];
  *op1_tn = OP_opnd(op, 1);
  *op1_tninfo = opnd_tninfo[1];
  return TRUE;
}

static BOOL
op_match_neg(OP *op, 
	     EBO_TN_INFO **opnd_tninfo, 
	     TN **op0_tn, 
	     EBO_TN_INFO **op0_tninfo)
{
  TOP top = OP_code(op);
  TN *opnd;

  if (!TOP_is_sub(top)) return FALSE;
  if (!(TN_Has_Value(OP_opnd(op, 0)) && TN_Value(OP_opnd(op, 0)) == 0)) return FALSE;
  *op0_tn = OP_opnd(op, 1);
  *op0_tninfo = opnd_tninfo[1];
  return TRUE;
}

static BOOL
op_match_not(OP *op, 
	     EBO_TN_INFO **opnd_tninfo, 
	     TN **op0_tn, 
	     EBO_TN_INFO **op0_tninfo)
{
  TOP top = OP_code(op);
  TN *opnd;

  if (!OP_ixor(op)) return FALSE;
  if (!(TN_Has_Value(OP_opnd(op, 1)) && 
	(TN_Value(OP_opnd(op, 1)) & 0xFFFFFFFF) == 0xFFFFFFFF)) return FALSE;
  *op0_tn = OP_opnd(op, 0);
  *op0_tninfo = opnd_tninfo[0];
  return TRUE;
}


static BOOL
op_match_unary(OP *op, 
	     EBO_TN_INFO **opnd_tninfo, 
	     TN **op0_tn, 
	     EBO_TN_INFO **op0_tninfo)
{
  TOP top = OP_code(op);
  TN *opnd;

  if (OP_results(op) != 1) return FALSE;
  if (OP_opnds(op) != 1) return FALSE;

  *op0_tn = OP_opnd(op, 0);
  *op0_tninfo = opnd_tninfo[0];
  return TRUE;
}

static BOOL
op_match_binary(OP *op, 
		EBO_TN_INFO **opnd_tninfo, 
		TN **op0_tn, 
		EBO_TN_INFO **op0_tninfo,
		TN **op1_tn, 
		EBO_TN_INFO **op1_tninfo)
{
  TOP top = OP_code(op);
  TN *opnd;

  if (OP_results(op) != 1) return FALSE;
  if (OP_opnds(op) != 2) return FALSE;

  *op0_tn = OP_opnd(op, 0);
  *op0_tninfo = opnd_tninfo[0];
  *op1_tn = OP_opnd(op, 1);
  *op1_tninfo = opnd_tninfo[1];
  return TRUE;
}

/*
 * min_max_sequence
 *
 * Detect the min(x)/max(x) sequences of the form:
 * (select (cmpgt x y) x y)
 * and generates:
 * (max x y)
 */
static BOOL
min_max_sequence(OP *op, TN **opnd_tn, EBO_TN_INFO **opnd_tninfo)
{
  TOP top = OP_code(op);
  TOP new_top;
  BOOL inverted;
  TN *cond_tn, *true_tn, *false_tn, *lhs_tn, *rhs_tn;
  EBO_TN_INFO *cond_tninfo, *true_tninfo, *false_tninfo, *lhs_tninfo, *rhs_tninfo;

  // Get select tninfos
  if (!op_match_select(op,
		       opnd_tninfo,
		       &cond_tn, &cond_tninfo, 
		       &true_tn, &true_tninfo, &false_tn, &false_tninfo))
    return FALSE;
  
  // Get defining op for cond
  EBO_OP_INFO *cond_opinfo;
  if (!find_def_opinfo(cond_tninfo, &cond_opinfo)) return FALSE;
  if (!op_match_compare(cond_opinfo->in_op, 
			cond_opinfo->actual_opnd, 
			&lhs_tn, &lhs_tninfo, &rhs_tn, &rhs_tninfo))
    return FALSE;
  if (lhs_tn == true_tn && rhs_tn == false_tn) {
    inverted = FALSE;
  } else if (lhs_tn == false_tn && rhs_tn == true_tn) {
    inverted = TRUE;
  } else return FALSE;
  
  TN *new_tn0 = NULL, *new_tn1 = NULL;
  EBO_TN_INFO *new_tninfo0 = NULL, *new_tninfo1 = NULL;
  
  VARIANT variant = TOP_cmp_variant(OP_code(cond_opinfo->in_op));

  switch (variant) {
    // Special case of move
  case V_CMP_EQ:
    new_top = TOP_mov_r;
    new_tn0 = false_tn;
    new_tninfo0 = false_tninfo;
    break;
  case V_CMP_NE:
    new_top = TOP_mov_r;
    new_tn0 = true_tn;
    new_tninfo0 = true_tninfo;
    break;
    
    // Min/max
  case V_CMP_GT:
  case V_CMP_GE:
    new_top = inverted ? TOP_min_r: TOP_max_r;
    new_tn0 = true_tn;
    new_tninfo0 = true_tninfo;
    new_tn1 = false_tn;
    new_tninfo1 = false_tninfo;
    break;
  case V_CMP_LT:
  case V_CMP_LE:
    new_top = inverted ? TOP_max_r: TOP_min_r;
    new_tn0 = true_tn;
    new_tninfo0 = true_tninfo;
    new_tn1 = false_tn;
    new_tninfo1 = false_tninfo;
    break;
  case V_CMP_GTU:
  case V_CMP_GEU:
    new_top = inverted ? TOP_minu_r: TOP_maxu_r;
    new_tn0 = true_tn;
    new_tninfo0 = true_tninfo;
    new_tn1 = false_tn;
    new_tninfo1 = false_tninfo;
    break;
  case V_CMP_LTU:
  case V_CMP_LEU:
    new_top = inverted ? TOP_maxu_r: TOP_minu_r;
    new_tn0 = true_tn;
    new_tninfo0 = true_tninfo;
    new_tn1 = false_tn;
    new_tninfo1 = false_tninfo;
    break;
  default:
    return FALSE;
  }
  
  BB *bb = OP_bb(op);
  if ((new_tninfo0 != NULL && !EBO_tn_available (bb, new_tninfo0)) ||
      (new_tninfo1 != NULL && !EBO_tn_available (bb, new_tninfo1))) {
    return FALSE;
  }
  if (new_tn1 == NULL) {
    if (TN_is_symbol(new_tn0)) return FALSE;
    if (TN_has_value(new_tn0))
      new_top = TOP_opnd_immediate_variant(new_top, 0, TN_value(new_tn0));
  } else {
    if (TN_has_value(new_tn0)) {
      TN *tmp_tn = new_tn1;
      new_tn1 = new_tn0;
      new_tn0 = tmp_tn;
      EBO_TN_INFO *tmp_tninfo = new_tninfo1;
      new_tninfo1 = new_tninfo0;
      new_tninfo0 = tmp_tninfo;
    }
    if (TN_is_symbol(new_tn1)) return FALSE;
    if (TN_has_value(new_tn1)) {
      new_top = TOP_opnd_immediate_variant(new_top, 1, TN_value(new_tn1));
    }
  }
  
  OP *new_op;
  new_op = Mk_OP(new_top, OP_result(op,0), new_tn0, new_tn1);
  if (new_top == TOP_mov_r) Set_OP_copy(new_op);
  OP_srcpos(new_op) = OP_srcpos(op);
  if (EBO_in_loop) EBO_Set_OP_omega (new_op, new_tninfo0, new_tninfo1);
  BB_Insert_Op_After(bb, op, new_op);
  if (EBO_Trace_Optimization) 
    fprintf(TFile,"Convert slct/cmp to %s\n", TOP_Name(new_top));
  return TRUE;

}

/*
 * abs_sequence
 *
 * Detect the abs(x) or -abs(x) sequence of the form:
 * (select (cmplt x 0) (neg x) x)
 * and generates:
 * (max (neg x) x)
 */

static BOOL
abs_sequence(OP *op, TN **opnd_tn, EBO_TN_INFO **opnd_tninfo)
{
  TOP top = OP_code(op);
  TOP new_top;
  BOOL inverted;
  TN *cond_tn, *true_tn, *false_tn, *lhs_tn, *rhs_tn, *neg_tn, *src_tn, *inv_tn;
  EBO_TN_INFO *cond_tninfo, *true_tninfo, *false_tninfo, *lhs_tninfo, *rhs_tninfo, *neg_tninfo, *src_tninfo, *inv_tninfo;

  // Get select tninfos
  if (!op_match_select(op,
		       opnd_tninfo,
		       &cond_tn, &cond_tninfo, 
		       &true_tn, &true_tninfo, &false_tn, &false_tninfo))
    return FALSE;
  
  // Get defining op for alternatives
  EBO_OP_INFO *def_opinfo;
  if (find_def_opinfo(true_tninfo, &def_opinfo) &&
      op_match_neg(def_opinfo->in_op,
		   def_opinfo->actual_opnd,
		   &neg_tn, &neg_tninfo) &&
      neg_tn == false_tn) {
    src_tn = false_tn;
    src_tninfo = false_tninfo;
    inv_tn = true_tn;
    inv_tninfo = true_tninfo;
    inverted = FALSE;
  } else if (find_def_opinfo(false_tninfo, &def_opinfo) &&
	     op_match_neg(def_opinfo->in_op,
			  def_opinfo->actual_opnd,
			  &neg_tn, &neg_tninfo) &&
	     neg_tn == true_tn) {
    src_tn = true_tn;
    src_tninfo = true_tninfo;
    inv_tn = false_tn;
    inv_tninfo = false_tninfo;
    inverted = TRUE;
  } else return FALSE;

  // Should not appear
  if (TN_has_value(src_tn) || TN_has_value(inv_tn)) return FALSE;
  
  // Get defining op for cond
  EBO_OP_INFO *cond_opinfo;
  if (!find_def_opinfo(cond_tninfo, &cond_opinfo)) return FALSE;
  if (!op_match_compare(cond_opinfo->in_op, 
			cond_opinfo->actual_opnd, 
			&lhs_tn, &lhs_tninfo, &rhs_tn, &rhs_tninfo))
    return FALSE;
  if (lhs_tn == src_tn && TN_Has_Value(rhs_tn) && TN_Value(rhs_tn) == 0) {
  } else if (rhs_tn == src_tn && TN_Has_Value(lhs_tn) && TN_Value(lhs_tn) == 0) {
    inverted = inverted ? FALSE: TRUE;
  } else return FALSE;

  VARIANT variant = TOP_cmp_variant(OP_code(cond_opinfo->in_op));
  switch (variant) {
  case V_CMP_GT:
  case V_CMP_GE:
    new_top = inverted ? TOP_max_r: TOP_min_r;
    break;
  case V_CMP_LT:
  case V_CMP_LE:
    new_top = inverted ? TOP_min_r: TOP_max_r;
    break;
  default:
    return FALSE;
  }
  
  BB *bb = OP_bb(op);
  if ((src_tninfo != NULL && !EBO_tn_available (bb, src_tninfo)) ||
      (inv_tninfo != NULL && !EBO_tn_available (bb, inv_tninfo))) {
    return FALSE;
  }

  OP *new_op;
  new_op = Mk_OP(new_top, OP_result(op,0), inv_tn, src_tn);
  OP_srcpos(new_op) = OP_srcpos(op);
  if (EBO_in_loop) EBO_Set_OP_omega (new_op, inv_tninfo, src_tninfo);
  BB_Insert_Op_After(bb, op, new_op);
  if (EBO_Trace_Optimization) 
    fprintf(TFile,"Convert slct/neg/cmp to %s\n", TOP_Name(new_top));
  return TRUE;

}

/*
 * select_invert_sequence
 *
 * - Convert select of logical not into inverted select
 */
static BOOL
select_invert_sequence(OP *op, TN **opnd_tn, EBO_TN_INFO **opnd_tninfo)
{
  TOP opcode = OP_code(op);
  VARIANT variant, new_variant;
  TN *lnot_tn, *src_tn;
  EBO_TN_INFO *lnot_tninfo, *src_tninfo;
  EBO_OP_INFO *def_opinfo;

  if (!TOP_is_select(opcode)) return FALSE;

  variant = TOP_cond_variant(opcode);

  if (find_def_opinfo(opnd_tninfo[0], &def_opinfo) &&
      op_match_lnot(def_opinfo->in_op,
		    def_opinfo->actual_opnd, 
		    &lnot_tn, &lnot_tninfo)) {
    new_variant = variant == V_COND_TRUE ? V_COND_FALSE: V_COND_TRUE;
    src_tn = lnot_tn;
    src_tninfo = lnot_tninfo;
  } else return FALSE;

  BB *bb = OP_bb(op);
  if (src_tninfo != NULL && !EBO_tn_available (bb, src_tninfo)) return FALSE;

  TOP new_top;
  new_top = new_variant == V_COND_TRUE ? TOP_slct_r: TOP_slctf_r;
  if (TN_is_symbol(OP_opnd(op, 2))) return FALSE;
  if (TN_has_value(OP_opnd(op, 2)))
    new_top = TOP_opnd_immediate_variant(new_top, 2, TN_value(OP_opnd(op, 2)));
  if (new_top == TOP_UNDEFINED) return FALSE;
  
  OPS ops = OPS_EMPTY;
  if (TN_register_class(src_tn) != TN_register_class(OP_opnd(op, 0))) {
    TN *tmp = Build_RCLASS_TN (TN_register_class(OP_opnd(op, 0)));
    Exp_COPY(tmp, src_tn, &ops);
    if (EBO_in_loop) EBO_OPS_omega(&ops, src_tn, src_tninfo);
    src_tn = tmp;
    src_tninfo = NULL;
  }
  OP *new_op = Mk_OP(new_top, OP_result(op, 0), src_tn, OP_opnd(op, 1),
		     OP_opnd(op, 2));
  OP_srcpos(new_op) = OP_srcpos(op);
  if (EBO_in_loop) EBO_Set_OP_omega (new_op, src_tninfo, opnd_tninfo[1], opnd_tninfo[2]);
  OPS_Append_Op(&ops, new_op);
  if (!EBO_Verify_Ops(&ops)) return FALSE;
  BB_Insert_Ops_After(OP_bb(op), op, &ops);
  if (EBO_Trace_Optimization) 
    fprintf(TFile,"Convert slct of lnot into %s\n", TOP_Name(new_top));
  return TRUE;
}


/*
 * select_move_sequence
 *
 * - Convert select of constant condition to move.
 * - Convert select of 0, 1 to mfb.
 */
static BOOL
select_move_sequence(OP *op, TN **opnd_tn, EBO_TN_INFO **opnd_tninfo)
{
  TOP opcode = OP_code(op);
  TOP new_opcode;
  VARIANT variant;

  if (!TOP_is_select(opcode)) return FALSE;

  variant = TOP_cond_variant(opcode);
  
  if (TN_Has_Value(opnd_tn[0])) {
    INT idx;
    if (TN_Value(opnd_tn[0]) == 0) idx = variant == V_COND_FALSE ? 1 : 2;
    else idx = variant == V_COND_FALSE ? 2 : 1;
    new_opcode = TOP_mov_r;
    if (TN_is_symbol(opnd_tn[idx])) return FALSE;
    if (TN_has_value(opnd_tn[idx])) 
      new_opcode = TOP_opnd_immediate_variant(new_opcode, 0, TN_value(opnd_tn[idx]));
    if (new_opcode == TOP_UNDEFINED) return FALSE;
    OP *new_op = Mk_OP(new_opcode, OP_result(op, 0), opnd_tn[idx]);
    if (new_opcode == TOP_mov_r) Set_OP_copy(new_op);
    OP_srcpos(new_op) = OP_srcpos(op);
    if (EBO_in_loop) EBO_Set_OP_omega (new_op, opnd_tninfo[idx]);
    BB_Insert_Op_After(OP_bb(op), op, new_op);
    if (EBO_Trace_Optimization) 
      fprintf(TFile,"Convert slct of constant\n");
    return TRUE;
  } else if (TN_Has_Value(opnd_tn[1]) && TN_Has_Value(opnd_tn[2])) {
    if ((variant == V_COND_FALSE && TN_Value(opnd_tn[1]) == 0 && TN_Value(opnd_tn[2]) == 1) ||
	(variant == V_COND_TRUE && TN_Value(opnd_tn[1]) == 1 && TN_Value(opnd_tn[2]) == 0)) {
      new_opcode = TOP_mfb;
      if (TN_is_symbol(opnd_tn[0])) return FALSE;
      if (TN_has_value(opnd_tn[0])) 
	new_opcode = TOP_opnd_immediate_variant(new_opcode, 0, TN_value(opnd_tn[0]));
      if (new_opcode == TOP_UNDEFINED) return FALSE;
      OP *new_op = Mk_OP(new_opcode, OP_result(op, 0), opnd_tn[0]);
      OP_srcpos(new_op) = OP_srcpos(op);
      if (EBO_in_loop) EBO_Set_OP_omega (new_op, opnd_tninfo[0]);
      BB_Insert_Op_After(OP_bb(op), op, new_op);
      if (EBO_Trace_Optimization) 
	fprintf(TFile,"Convert slct to mfb\n");
      return TRUE;
    }
  }
  
  return FALSE;
}

/*
 * select_move_sequence_2
 *
 * convert the sequence:
 * (slct x (op_b u v) (op_r u v) 0)
 * into (mfb (op_b u v)).
 */
static BOOL
select_move_sequence_2(OP *op, TN **opnd_tn, EBO_TN_INFO **opnd_tninfo)
{
  TOP opcode = OP_code(op);
  TOP new_opcode;
  VARIANT variant;
  EBO_TN_INFO *cond_tninfo, *true_tninfo, *false_tninfo;
  TN *cond_tn, *true_tn, *false_tn;

  if (!TOP_is_select(opcode)) return FALSE;
  
  if (!op_match_select(op,
		       opnd_tninfo,
		       &cond_tn, &cond_tninfo, 
		       &true_tn, &true_tninfo, &false_tn, &false_tninfo))
    return FALSE;
  
  if (TN_Has_Value(false_tn) && TN_Value(false_tn) == 0) {
  } else return FALSE;

  // Get defining op for true value
  EBO_OP_INFO *def_opinfo;
  OP *def_op;
  if (!find_def_opinfo(true_tninfo, &def_opinfo)) return FALSE;
  def_op = def_opinfo->in_op;

  // Get defining op for cond
  EBO_OP_INFO *cond_opinfo;
  OP *cond_op;
  if (!find_def_opinfo(cond_tninfo, &cond_opinfo)) return FALSE;
  cond_op = cond_opinfo->in_op;

  // Check that operations are identical excepted in the type of
  // the result.
  if (!(OP_results(def_op) == 1 &&
	OP_results(def_op) == OP_results(cond_op) &&
	OP_opnds(def_op) <= 2 &&
	OP_opnds(def_op) == OP_opnds(cond_op))) return FALSE;
  TOP new_def_top, new_cond_top;
  new_def_top = TOP_result_register_variant(OP_code(def_op), 0, ISA_REGISTER_CLASS_branch);
  new_cond_top = TOP_result_register_variant(OP_code(cond_op), 0, ISA_REGISTER_CLASS_branch);
  if (new_def_top == TOP_UNDEFINED || new_def_top != new_cond_top) return FALSE;

  for (int i = 0; i < OP_opnds(def_op); i++) {
    if (OP_opnd(def_op, i) != OP_opnd(cond_op, i)) return FALSE;
  }

  BB *bb = OP_bb(op);
  if (cond_tninfo != NULL && !EBO_tn_available (bb, cond_tninfo))
    return FALSE;

  OP *new_op;
  new_op = Mk_OP(TOP_mfb, OP_result(op,0), cond_tn);
  OP_srcpos(new_op) = OP_srcpos(op);
  if (EBO_in_loop) EBO_Set_OP_omega (new_op, cond_tninfo);
  BB_Insert_Op_After(bb, op, new_op);
  if (EBO_Trace_Optimization) 
    fprintf(TFile,"Convert select of cmp into mfb\n");
  return TRUE;

}

/*
 * select_select_sequence
 *
 * convert the sequence:
 * (slct x (slct x (y z)) z) -> (slct x (y z))
 * (slct x (slct x (z y)) z) -> (z)
 */
static BOOL
select_select_sequence(OP *op, TN **opnd_tn, EBO_TN_INFO **opnd_tninfo)
{
  TOP opcode = OP_code(op);
  EBO_TN_INFO *cond_tninfo, *true_tninfo, *false_tninfo;
  TN *cond_tn, *true_tn, *false_tn;
  EBO_TN_INFO *l1_cond_tninfo, *l1_true_tninfo, *l1_false_tninfo;
  TN *l1_cond_tn, *l1_true_tn, *l1_false_tn;
  EBO_TN_INFO *src_tninfo;
  TN *src_tn;
  BB *bb = OP_bb(op);

  if (!TOP_is_select(opcode)) return FALSE;
  
  if (!op_match_select(op,
		       opnd_tninfo,
		       &cond_tn, &cond_tninfo, 
		       &true_tn, &true_tninfo, &false_tn, &false_tninfo))
    return FALSE;
  
  // Get defining op for true value
  EBO_OP_INFO *def_opinfo;
  OP *def_op;
  if (find_def_opinfo(true_tninfo, &def_opinfo) &&
      op_match_select(def_opinfo->in_op,
		      def_opinfo->actual_opnd, 
		      &l1_cond_tn, &l1_cond_tninfo,
		      &l1_true_tn, &l1_true_tninfo,
		      &l1_false_tn, &l1_false_tninfo) &&
      (l1_cond_tninfo == NULL || EBO_tn_available (bb, l1_cond_tninfo)) &&
      (l1_true_tninfo == NULL || EBO_tn_available (bb, l1_true_tninfo)) &&
      (l1_false_tninfo == NULL || EBO_tn_available (bb, l1_false_tninfo))) {
    if (cond_tn == l1_cond_tn) {
      if (false_tn == l1_false_tn) {
	// (slct b (slct b x y) y) -> (slct b x y)
	src_tn = true_tn;
	src_tninfo = true_tninfo;
	goto matched;
      } else if (false_tn == l1_true_tn) {
	// (slct b (slct b y x) y) -> (y)
	src_tn = false_tn;
	src_tninfo = false_tninfo;
	goto matched;
      }
    }
  }

  // Get defining op for false value
  if (find_def_opinfo(false_tninfo, &def_opinfo) &&
      op_match_select(def_opinfo->in_op,
		      def_opinfo->actual_opnd, 
		      &l1_cond_tn, &l1_cond_tninfo,
		      &l1_true_tn, &l1_true_tninfo,
		      &l1_false_tn, &l1_false_tninfo) &&
      (l1_cond_tninfo == NULL || EBO_tn_available (bb, l1_cond_tninfo)) &&
      (l1_true_tninfo == NULL || EBO_tn_available (bb, l1_true_tninfo)) &&
      (l1_false_tninfo == NULL || EBO_tn_available (bb, l1_false_tninfo))) {
    if (cond_tn == l1_cond_tn) {
      if (true_tn == l1_true_tn) {
	// (slct b x (slct b x y)) -> (slct b x y)
	src_tn = false_tn;
	src_tninfo = false_tninfo;
	goto matched;
      } else if (true_tn == l1_false_tn) {
	// (slct b x (slct b y x)) -> (x)
	src_tn = true_tn;
	src_tninfo = true_tninfo;
	goto matched;
      }
    }
  }
  return FALSE;
 matched:
  OPS ops = OPS_EMPTY;
  if (TN_is_constant(src_tn)) {
    Expand_Immediate(OP_result(op, 0), src_tn, MTYPE_I4, &ops);
  } else {
    Exp_COPY(OP_result(op, 0), src_tn, &ops);
  }
  if (!EBO_Verify_Ops(&ops)) return FALSE;
  OP_srcpos(OPS_last(&ops)) = OP_srcpos(op);
  if (EBO_in_loop) EBO_OPS_omega (&ops, src_tn, src_tninfo);
  BB_Insert_Ops_After(bb, op, &ops);
  if (EBO_Trace_Optimization) 
    fprintf(TFile,"Convert select/select into move\n");
  return TRUE;
}

/*
 * cmp_move_sequence
 *
 * - Convert a cmp to move
 *   (cmp_b x 0) -> mtb
 *   (cmp_b x 0) -> mov if x is 1 bit width
 */
static BOOL
cmp_move_sequence(OP *op, TN **opnd_tn, EBO_TN_INFO **opnd_tninfo)
{
  TOP top = OP_code(op);
  TN *lhs_tn, *rhs_tn, *src_tn;
  EBO_TN_INFO *lhs_tninfo, *rhs_tninfo, *src_tninfo;
  EBO_OP_INFO *def_opinfo;
  BOOL inverted;

  if (!TOP_is_cmp(top)) return FALSE;

  VARIANT variant = TOP_cmp_variant(top);
  if (variant != V_CMP_NE) return FALSE;

  if (!op_match_compare(op, 
			opnd_tninfo, 
			&lhs_tn, &lhs_tninfo, &rhs_tn, &rhs_tninfo))
    return FALSE;


  if (TN_Has_Value(lhs_tn) && TN_Value(lhs_tn) == 0 &&
      TN_is_register(rhs_tn)) {
    src_tn = rhs_tn;
    src_tninfo = rhs_tninfo;
    inverted = TRUE;
  } else if (TN_Has_Value(rhs_tn) && TN_Value(rhs_tn) == 0 &&
      TN_is_register(lhs_tn)) {
    src_tn = lhs_tn;
    src_tninfo = lhs_tninfo;
    inverted = FALSE;
  } else return FALSE;

  if (TN_register_class(src_tn) != ISA_REGISTER_CLASS_integer) return FALSE;

  INT32 def_bits, def_signed;
  // If the result is a branch, the mtb will normalize it
  // otherwise we must check that the input is already normalized
  if (!(TN_register_class(OP_result(op, 0)) == ISA_REGISTER_CLASS_branch ||
	(find_def_opinfo(src_tninfo, &def_opinfo) &&
	 OP_results(def_opinfo->in_op) == 1 &&
	 def_bit_width(def_opinfo->in_op, 0, &def_bits, &def_signed) &&
	 def_bits == 1 && def_signed == 0))) return FALSE;


  BB *bb = OP_bb(op);
  if (src_tninfo != NULL && !EBO_tn_available (bb, src_tninfo))
    return FALSE;
  
  OPS ops = OPS_EMPTY;
  Exp_COPY(OP_result(op, 0), src_tn, &ops);

  if (!EBO_Verify_Ops(&ops)) return FALSE;
  OP_srcpos(OPS_last(&ops)) = OP_srcpos(op);
  if (EBO_in_loop) EBO_OPS_omega (&ops, src_tn, src_tninfo);
  BB_Insert_Ops_After(bb, op, &ops);
  if (EBO_Trace_Optimization) 
    fprintf(TFile,"Convert cmp into move\n");
  return TRUE;
}

/*
 * ext_move_sequence
 *
 * replace useless extensions by move.
 */
static BOOL
ext_move_sequence(OP *op, TN **opnd_tn, EBO_TN_INFO **opnd_tninfo)
{
  TOP opcode = OP_code(op);
  int opnd1_idx = TOP_Find_Operand_Use(opcode,OU_opnd1);
  int opnd2_idx = TOP_Find_Operand_Use(opcode,OU_opnd2);
  INT32 use_bits, use_signed;
  INT32 def_bits, def_signed;
  int opnd_idx;

  if (!OP_is_extension(op, opnd1_idx, &use_bits, &use_signed)) return FALSE;
  
  OP *def_op;
  EBO_OP_INFO *def_opinfo;
  if (!find_def_opinfo(opnd_tninfo[opnd1_idx], &def_opinfo)) return FALSE;
  def_op = def_opinfo->in_op;

  if (OP_results(def_op) == 1 &&
      def_bit_width(def_op, 0, &def_bits, &def_signed)) {
    opnd_idx = opnd1_idx;
    if (def_signed == use_signed &&
	def_bits <= use_bits) goto matched;
    if (!def_signed && use_signed &&
	def_bits < use_bits) goto matched;
  }
  return FALSE;
 matched:
  if (OP_opnd(op,opnd_idx) != opnd_tn[opnd_idx]) return FALSE;
  OP *new_op = Mk_OP(TOP_mov_r, OP_result(op, 0), opnd_tn[opnd_idx]);
  Set_OP_copy(new_op);
  OP_srcpos(new_op) = OP_srcpos(op);
  if (EBO_in_loop) EBO_Set_OP_omega (new_op, opnd_tninfo[opnd_idx]);
  BB_Insert_Op_After(OP_bb(op), op, new_op);
  if (EBO_Trace_Optimization) 
    fprintf(TFile,"Convert ext to move\n");
  return TRUE;
}  

/*
 * mtb_op_sequence
 *
 * Convert (mtb (op)) into an op defining a branch register
 */
static BOOL
mtb_op_sequence(OP *op, TN **opnd_tn, EBO_TN_INFO **opnd_tninfo)
{
  TOP top = OP_code(op);
  TN *lhs_tn, *rhs_tn;
  EBO_TN_INFO *lhs_tninfo, *rhs_tninfo;
  EBO_OP_INFO *def_opinfo;
  BOOL do_copy = FALSE;

  if (top != TOP_mtb) return FALSE;
  if (!find_def_opinfo(opnd_tninfo[0], &def_opinfo)) return FALSE;
  if (op_match_unary(def_opinfo->in_op,
		     def_opinfo->actual_opnd, 
		     &lhs_tn, &lhs_tninfo)) {
    rhs_tn = NULL;
    rhs_tninfo = NULL;
  } else if (op_match_binary(def_opinfo->in_op,
			     def_opinfo->actual_opnd, 
			     &lhs_tn, &lhs_tninfo, &rhs_tn, &rhs_tninfo)) {
  } else return FALSE;
  
  TOP new_top;
  if (OP_code(def_opinfo->in_op) == TOP_mfb) {
    // This is a branch_copy
    do_copy = TRUE;
  } else {
    new_top = TOP_result_register_variant(OP_code(def_opinfo->in_op), 0, ISA_REGISTER_CLASS_branch);
    if (new_top == TOP_UNDEFINED) return FALSE;
  }

  BB *bb = OP_bb(op);
  if ((lhs_tninfo != NULL && !EBO_tn_available (bb, lhs_tninfo)) ||
      (rhs_tninfo != NULL && !EBO_tn_available (bb, rhs_tninfo)))
    return FALSE;

  if (do_copy) {
    OPS ops = OPS_EMPTY;
    Exp_COPY(OP_result(op,0), lhs_tn, &ops);
    if (!EBO_Verify_Ops(&ops)) return FALSE;
    OP_srcpos(OPS_last(&ops)) = OP_srcpos(op);
    if (EBO_in_loop) EBO_OPS_omega (&ops, lhs_tn, lhs_tninfo);
    BB_Insert_Ops(bb, op, &ops, FALSE);
    if (EBO_Trace_Optimization) 
      fprintf(TFile,"Convert mtb/mfb into copy\n");
  } else {
    OP *new_op;
    new_op = Mk_OP(new_top, OP_result(op,0), lhs_tn, rhs_tn);
    if (EBO_in_loop) EBO_Set_OP_omega (new_op, lhs_tninfo, rhs_tninfo);
    OP_srcpos(new_op) = OP_srcpos(op);
    BB_Insert_Op_After(bb, op, new_op);
    if (EBO_Trace_Optimization) 
      fprintf(TFile,"Convert mtb/op into %s\n", TOP_Name(new_top));
  }

  return TRUE;
}

/*
 * mfb_op_sequence
 *
 * Convert (mfb (op)) into an op defining an integer register
 */
static BOOL
mfb_op_sequence(OP *op, TN **opnd_tn, EBO_TN_INFO **opnd_tninfo)
{
  TOP top = OP_code(op);
  TN *lhs_tn, *rhs_tn;
  EBO_TN_INFO *lhs_tninfo, *rhs_tninfo;
  EBO_OP_INFO *def_opinfo;

  if (top != TOP_mfb) return FALSE;
  if (!find_def_opinfo(opnd_tninfo[0], &def_opinfo)) return FALSE;
  if (op_match_unary(def_opinfo->in_op,
		     def_opinfo->actual_opnd, 
		     &lhs_tn, &lhs_tninfo)) {
    rhs_tn = NULL;
    rhs_tninfo = NULL;
  } else if (op_match_binary(def_opinfo->in_op,
			     def_opinfo->actual_opnd, 
			     &lhs_tn, &lhs_tninfo, &rhs_tn, &rhs_tninfo)) {
  } else return FALSE;
  
  TOP new_top;
  if (OP_code(def_opinfo->in_op) == TOP_mtb) {
    // This is a register normalization
    new_top = TOP_cmpne_r_r;
    rhs_tn = Zero_TN;
    rhs_tninfo = NULL;
  } else {
    new_top = TOP_result_register_variant(OP_code(def_opinfo->in_op), 0, ISA_REGISTER_CLASS_integer);
    if (new_top == TOP_UNDEFINED) return FALSE;
  }

  BB *bb = OP_bb(op);
  if ((lhs_tninfo != NULL && !EBO_tn_available (bb, lhs_tninfo)) ||
      (rhs_tninfo != NULL && !EBO_tn_available (bb, rhs_tninfo)))
    return FALSE;

  OP *new_op;
  new_op = Mk_OP(new_top, OP_result(op,0), lhs_tn, rhs_tn);
  OP_srcpos(new_op) = OP_srcpos(op);
  if (EBO_in_loop) EBO_Set_OP_omega (new_op, lhs_tninfo, rhs_tninfo);
  BB_Insert_Op_After(bb, op, new_op);
  if (EBO_Trace_Optimization) 
    fprintf(TFile,"Convert mfb/op into %s\n", TOP_Name(new_top));
  return TRUE;
}

/*
 * and_or_sequence
 *
 * Convert :
 * - (and (not a) b) into (andc a b)
 * - (or (not a) b) into (orc a b)
 */
static BOOL
and_or_sequence(OP *op, TN **opnd_tn, EBO_TN_INFO **opnd_tninfo)
{
  TOP top = OP_code(op);
  TN *not_tn, *lhs_tn, *rhs_tn;
  EBO_TN_INFO *not_tninfo, *lhs_tninfo, *rhs_tninfo;
  EBO_OP_INFO *def_opinfo;

  if (!OP_iand(op) && !OP_ior(op)) return FALSE;

  if (find_def_opinfo(opnd_tninfo[0], &def_opinfo) &&
      op_match_not(def_opinfo->in_op,
		   def_opinfo->actual_opnd, 
		   &not_tn, &not_tninfo)) {
    lhs_tn = not_tn;
    lhs_tninfo = not_tninfo;
    rhs_tn = opnd_tn[1];
    rhs_tninfo = opnd_tninfo[1];
    goto replace;
  } 
  if (find_def_opinfo(opnd_tninfo[1], &def_opinfo) &&
      op_match_not(def_opinfo->in_op,
		   def_opinfo->actual_opnd, 
		   &not_tn, &not_tninfo)) {
    lhs_tn = not_tn;
    lhs_tninfo = not_tninfo;
    rhs_tn = opnd_tn[0];
    rhs_tninfo = opnd_tninfo[0];
    goto replace;
  } 
  return FALSE;
 replace:
  BB *bb = OP_bb(op);
  TOP new_top;
  if ((lhs_tninfo != NULL && !EBO_tn_available (bb, lhs_tninfo)) ||
      (rhs_tninfo != NULL && !EBO_tn_available (bb, rhs_tninfo)))
    return FALSE;

  // Find new top
  if (OP_iand(op)) new_top = TOP_andc_r;
  if (OP_ior(op)) new_top = TOP_orc_r;
  if (TN_is_symbol(lhs_tn)) return FALSE;
  if (TN_has_value(lhs_tn))
    new_top = TOP_opnd_immediate_variant(new_top, 0, TN_value(lhs_tn));
  if (new_top == TOP_UNDEFINED) return FALSE;
  if (TN_is_symbol(rhs_tn)) return FALSE;
  if (TN_has_value(rhs_tn))
    new_top = TOP_opnd_immediate_variant(new_top, 1, TN_value(rhs_tn));
  if (new_top == TOP_UNDEFINED) return FALSE;

		    
  OP *new_op;
  new_op = Mk_OP(new_top, OP_result(op,0), lhs_tn, rhs_tn);
  OP_srcpos(new_op) = OP_srcpos(op);
  if (EBO_in_loop) EBO_Set_OP_omega (new_op, lhs_tninfo, rhs_tninfo);
  BB_Insert_Op_After(bb, op, new_op);
  if (EBO_Trace_Optimization) 
    fprintf(TFile,"Convert and/or into  %s\n", TOP_Name(new_top));
  return TRUE;
}


/*
 * andl_orl_sequence
 *
 * Convert :
 * - (andl (notl a) (notl b)) into (norl a b)
 * - (orl (notl a) (notl b)) into (nandl a b)
 * - (nandl (notl a) (notl b)) into (orl a b)
 * - (norl (notl a) (notl b)) into (andl a b)
 * - (andl a a) into (movl a)
 * - (orl a a) into (movl a)
 * - (nandl a a) into (notl a)
 * - (norl a a) into (notl a)
 */
static BOOL
andl_orl_sequence(OP *op, TN **opnd_tn, EBO_TN_INFO **opnd_tninfo)
{
  TOP top = OP_code(op);
  TN *l1_tn, *l2_tn, *lhs_tn, *rhs_tn;
  EBO_TN_INFO *l1_tninfo, *l2_tninfo, *lhs_tninfo, *rhs_tninfo;
  EBO_OP_INFO *def_opinfo;
  BB *bb = OP_bb(op);

  if (!OP_icmp(op)) return FALSE;

  if (!op_match_compare(op, 
			opnd_tninfo, 
			&lhs_tn, &lhs_tninfo, &rhs_tn, &rhs_tninfo))
    return FALSE;
  
  VARIANT variant = TOP_cmp_variant(top);
  if (variant != V_CMP_ORL &&
      variant != V_CMP_NORL &&
      variant != V_CMP_ANDL &&
      variant != V_CMP_NANDL) return FALSE;

  if (EBO_Trace_Optimization) 
    fprintf(TFile,"Enter andl_orl_sequence\n");
  

  VARIANT new_variant = variant;
  lhs_tn = opnd_tn[0];
  lhs_tninfo = opnd_tninfo[0];
  rhs_tn = opnd_tn[1];
  rhs_tninfo = opnd_tninfo[1];
  
  if (find_def_opinfo(lhs_tninfo, &def_opinfo)) {
    if (op_match_lmove(def_opinfo->in_op,
		       def_opinfo->actual_opnd, 
		       &l1_tn, &l1_tninfo)) {
      lhs_tn = l1_tn;
      lhs_tninfo = l1_tninfo;
    }
  }
  if (find_def_opinfo(rhs_tninfo, &def_opinfo)) {
    if (op_match_lmove(def_opinfo->in_op,
		       def_opinfo->actual_opnd, 
		       &l1_tn, &l1_tninfo)) {
      rhs_tn = l1_tn;
      rhs_tninfo = l1_tninfo;
    }
  } 

  if (lhs_tn == NULL || rhs_tn == NULL) return FALSE;
  
  if (!TN_is_register(lhs_tn) || !TN_is_register(rhs_tn)) return FALSE;
  
  if ((lhs_tninfo != NULL && !EBO_tn_available (bb, lhs_tninfo)) ||
      (rhs_tninfo != NULL && !EBO_tn_available (bb, rhs_tninfo)))
    return FALSE;

  if (find_def_opinfo(lhs_tninfo, &def_opinfo) &&
      op_match_lnot(def_opinfo->in_op,
		    def_opinfo->actual_opnd, 
		    &l1_tn, &l1_tninfo) &&
      find_def_opinfo(rhs_tninfo, &def_opinfo) &&
      op_match_lnot(def_opinfo->in_op,
		    def_opinfo->actual_opnd, 
		    &l2_tn, &l2_tninfo)) {
    switch (new_variant) {
    case V_CMP_ORL: new_variant = V_CMP_NANDL; break;
    case V_CMP_ANDL: new_variant = V_CMP_NORL; break;
    case V_CMP_NORL: new_variant = V_CMP_ANDL; break;
    case V_CMP_NANDL: new_variant = V_CMP_ORL; break;
    }
    lhs_tn = l1_tn;
    lhs_tninfo = l1_tninfo;
    rhs_tn = l2_tn;
    rhs_tninfo = l2_tninfo;
  } 
  
  if (lhs_tn == NULL || rhs_tn == NULL) return FALSE;
  
  if (!TN_is_register(lhs_tn) || !TN_is_register(rhs_tn)) return FALSE;
  
  if ((lhs_tninfo != NULL && !EBO_tn_available (bb, lhs_tninfo)) ||
      (rhs_tninfo != NULL && !EBO_tn_available (bb, rhs_tninfo)))
    return FALSE;

  if (tn_registers_identical(lhs_tn, rhs_tn)) {
    if (EBO_Trace_Optimization) 
      fprintf(TFile, "Registers identical %d %d\n", TN_number(lhs_tn), TN_number(rhs_tn));
    BOOL invert = FALSE;
    switch (new_variant) {
    case V_CMP_NORL: invert = TRUE; break;
    case V_CMP_NANDL: invert = TRUE; break;
    }
    OPS ops = OPS_EMPTY;
    if (invert) {
      Expand_Logical_Not(OP_result(op, 0), lhs_tn, 0, &ops);
    } else {
      Expand_Logical_Or(OP_result(op, 0), lhs_tn, Zero_TN, 0, &ops);
    }
    if (!EBO_Verify_Ops(&ops)) return FALSE;
    OP_srcpos(OPS_last(&ops)) = OP_srcpos(op);
    if (EBO_in_loop) EBO_OPS_omega (&ops, lhs_tn, lhs_tninfo);
    BB_Insert_Ops_After(bb, op, &ops);
    if (EBO_Trace_Optimization) 
      fprintf(TFile,"Convert andl/orl into copy\n");
  } else {
    if (new_variant == variant &&
	lhs_tn == OP_opnd(op, 0) &&
	rhs_tn == OP_opnd(op, 1)) return FALSE;
    
    OP *new_op;
    TOP new_top = TOP_UNDEFINED;
    switch (new_variant) {
    case V_CMP_ANDL: new_top = TOP_andl_r_r; break;
    case V_CMP_ORL: new_top = TOP_orl_r_r; break;
    case V_CMP_NANDL: new_top = TOP_nandl_r_r; break;
    case V_CMP_NORL: new_top = TOP_norl_r_r; break;
    }
    if (new_top == TOP_UNDEFINED) return FALSE;
    
    TN *result;
    result = OP_result(op, 0);
    new_top = TOP_result_register_variant(new_top, 0, 
					  TN_register_class(result));
    new_op = Mk_OP(new_top, result, lhs_tn, rhs_tn);
    OP_srcpos(new_op) = OP_srcpos(op);
    if (EBO_in_loop) EBO_Set_OP_omega (new_op, lhs_tninfo, rhs_tninfo);
    OPS ops = OPS_EMPTY;
    OPS_Append_Op(&ops, new_op);
    if (!EBO_Verify_Ops(&ops)) return FALSE;
    BB_Insert_Ops_After(bb, op, &ops);
    if (EBO_Trace_Optimization) 
      fprintf(TFile,"Convert andl/orl into  %s\n", TOP_Name(new_top));
  }
  return TRUE;
}

/*
 * andl_orl_sequence_2
 *
 * Convert :
 * - (andl (andl x y) x) into (andl x y)
 */
static BOOL
andl_orl_sequence_2(OP *op, TN **opnd_tn, EBO_TN_INFO **opnd_tninfo)
{
  BB *bb = OP_bb(op);
  TOP top = OP_code(op);
  TN *lhs_tn, *rhs_tn;
  EBO_TN_INFO  *lhs_tninfo, *rhs_tninfo;
  EBO_OP_INFO *def_opinfo;
  TN *l1_lhs_tn, *l1_rhs_tn;
  EBO_TN_INFO  *l1_lhs_tninfo, *l1_rhs_tninfo;
  TN *src_tn;
  EBO_TN_INFO *src_tninfo;

  if (!OP_icmp(op)) return FALSE;

  if (!op_match_compare(op, 
			opnd_tninfo, 
			&lhs_tn, &lhs_tninfo, &rhs_tn, &rhs_tninfo))
    return FALSE;
  
  VARIANT variant = TOP_cmp_variant(top);
  if (variant != V_CMP_ORL &&
      variant != V_CMP_ANDL) return FALSE;

  if (find_def_opinfo(lhs_tninfo, &def_opinfo) &&
      op_match_compare(def_opinfo->in_op,
		       def_opinfo->actual_opnd, 
		       &l1_lhs_tn, &l1_lhs_tninfo, &l1_rhs_tn, &l1_rhs_tninfo)) {
    VARIANT l1_variant = TOP_cmp_variant(OP_code(def_opinfo->in_op));
    if ((l1_lhs_tninfo == NULL || EBO_tn_available (bb, l1_lhs_tninfo)) &&
	(l1_rhs_tninfo == NULL || EBO_tn_available (bb, l1_rhs_tninfo))) {
      if (l1_lhs_tn == rhs_tn ||
	  l1_rhs_tn == rhs_tn) {
	if (variant == l1_variant) {
	  src_tn = lhs_tn;
	  src_tninfo = lhs_tninfo;
	  goto do_copy;
	}
      }
    }
  }
  
  if (find_def_opinfo(rhs_tninfo, &def_opinfo) &&
      op_match_compare(def_opinfo->in_op,
		       def_opinfo->actual_opnd, 
		       &l1_lhs_tn, &l1_lhs_tninfo, &l1_rhs_tn, &l1_rhs_tninfo)) {
    VARIANT l1_variant = TOP_cmp_variant(OP_code(def_opinfo->in_op));
    if ((l1_lhs_tninfo == NULL || EBO_tn_available (bb, l1_lhs_tninfo)) &&
	(l1_rhs_tninfo == NULL || EBO_tn_available (bb, l1_rhs_tninfo))) {
      if (l1_lhs_tn == lhs_tn ||
	  l1_rhs_tn == lhs_tn) {
	if (variant == l1_variant) {
	  src_tn = rhs_tn;
	  src_tninfo = rhs_tninfo;
	  goto do_copy;
	}
      }
    }
  }
  return FALSE;

 do_copy:  
  if (src_tninfo != NULL && !EBO_tn_available (bb, src_tninfo)) return FALSE;

  OPS ops = OPS_EMPTY;
  Exp_COPY(OP_result(op, 0), src_tn, &ops);

  if (!EBO_Verify_Ops(&ops)) return FALSE;
  OP_srcpos(OPS_last(&ops)) = OP_srcpos(op);
  if (EBO_in_loop) EBO_OPS_omega (&ops, src_tn, src_tninfo);
  BB_Insert_Ops_After(bb, op, &ops);
  if (EBO_Trace_Optimization) 
    fprintf(TFile,"Convert andl/orl into move\n");

  return TRUE;
}

/*
 * logical_move_sequence
 *
 * - Convert a logical operation to a logical move
 *   (andl x 1) -> (x)
 *   (orl x 0) -> (x)
 *   (andl !x 1) -> (!x)
 *   (orl !x 0) -> (!x)
 */
static BOOL
logical_move_sequence(OP *op, TN **opnd_tn, EBO_TN_INFO **opnd_tninfo)
{
  TOP top = OP_code(op);
  TN *lhs_tn, *rhs_tn, *src_tn;
  EBO_TN_INFO *lhs_tninfo, *rhs_tninfo, *src_tninfo;
  EBO_OP_INFO *def_opinfo;
  INT32 def_bits, def_signed;
  INT64 value;
  
  if (!TOP_is_cmp(top)) return FALSE;
  if (!op_match_compare(op, 
			opnd_tninfo, 
			&lhs_tn, &lhs_tninfo, &rhs_tn, &rhs_tninfo))
    return FALSE;
  
  if (EBO_Trace_Optimization) 
    fprintf(TFile,"Enter logical_move_sequence\n");
  
  if (TN_Has_Value(lhs_tn) &&
      TN_is_register(rhs_tn)) {
    value =  TN_Value(lhs_tn);
    src_tn = rhs_tn;
    src_tninfo = rhs_tninfo;
  } else if (TN_Has_Value(rhs_tn) &&
	     TN_is_register(lhs_tn)) {
    value =  TN_Value(rhs_tn);
    src_tn = lhs_tn;
    src_tninfo = lhs_tninfo;
  } else return FALSE;
  
  if (TN_register_class(src_tn) != ISA_REGISTER_CLASS_integer) return FALSE;
  
  BOOL notl = FALSE;
  VARIANT variant = TOP_cmp_variant(top);
  switch (variant) {
  case V_CMP_ANDL:
    if (value == 0) {
      // (andl x 0) -> (movl 0)
      src_tn = Zero_TN;
      src_tninfo = NULL;
    } else {
      // (andl x 1) -> (movl x)
    }
    break;
  case V_CMP_ORL:
    if (value == 0) {
      // (orl x 0) -> (movl x)
    } else {
      // (orl x 1) -> (movl 1)
      src_tn = Gen_Literal_TN(1, 4);
      src_tninfo = NULL;
    }
    break;
  case V_CMP_NANDL:
    if (value == 0) {
      // (nandl x 0) -> (movl 1)
      src_tn = Gen_Literal_TN(1, 4);
      src_tninfo = NULL;
    } else {
      // (nandl x 1) -> (notl x)
      notl = TRUE;
    }
    break;
  case V_CMP_NORL:
    if (value == 0) {
      // (norl x 0) -> (notl x)
      notl = TRUE;
    } else {
      // (norl x 1) -> (movl 0)
      src_tn = Zero_TN;
      src_tninfo = NULL;
    }
    break;
  default:
    return FALSE;
  }
  
  BB *bb = OP_bb(op);
  if (src_tninfo != NULL && !EBO_tn_available (bb, src_tninfo))
    return FALSE;


  OPS ops = OPS_EMPTY;
  if (notl)
    Expand_Logical_Not(OP_result(op, 0), src_tn, 0, &ops);
  else
    Expand_Logical_Or(OP_result(op,0), src_tn, Zero_TN, 0, &ops);

  if (!EBO_Verify_Ops(&ops)) return FALSE;
  OP_srcpos(OPS_last(&ops)) = OP_srcpos(op);
  if (EBO_in_loop) EBO_OPS_omega (&ops, src_tn, src_tninfo);
  BB_Insert_Ops_After(bb, op, &ops);
  if (EBO_Trace_Optimization) 
    fprintf(TFile,"Convert andl/orl into movl/notl\n");
  
  return TRUE;
}

static BOOL
base_offset_sequence(OP *op, TN **opnd_tn, EBO_TN_INFO **opnd_tninfo)
{
  TOP opcode, new_opcode;
  INT base_idx, offset_idx;
  TN *base_tn, *offset_tn;
  ST *base_sym = NULL;
  INT32 base_relocs;
  TN *new_offset_tn, *new_base_tn;
  INT64 offset_val, base_val;
  OP *new_op;

  base_idx = OP_find_opnd_use(op, OU_base);
  offset_idx = OP_find_opnd_use(op, OU_offset);
  if (base_idx < 0 && offset_idx < 0) return FALSE;

  base_tn = opnd_tn[base_idx];
  offset_tn = opnd_tn[offset_idx];

  if (!TN_Is_Constant(base_tn) || !TN_Is_Constant(offset_tn)) return FALSE;

  if (TN_is_symbol(base_tn)) {
    if (TN_is_symbol(offset_tn)) return FALSE;
    else offset_val = TN_Value(offset_tn);
    base_val = TN_offset(base_tn);
    base_sym = TN_var(base_tn);
    base_relocs = TN_relocs(base_tn);
  } else {
    if (TN_is_symbol(offset_tn)) {
      offset_val = TN_offset(offset_tn);
      base_sym = TN_var(offset_tn);
      base_relocs = TN_relocs(offset_tn);
    } else {
      offset_val = TN_Value(offset_tn);
    }
    base_val = TN_Value(base_tn);
  }
  
  if (base_sym != NULL) {
    new_offset_tn = Gen_Symbol_TN(base_sym, base_val + offset_val, base_relocs);
  } else {
    new_offset_tn = Gen_Literal_TN(base_val+offset_val, 4);
  }
  new_base_tn = Zero_TN;
  if (new_offset_tn == OP_opnd(op, offset_idx) && new_base_tn == OP_opnd(op, base_idx)) return FALSE;

  opcode = OP_code(op);
  if (TN_has_value(new_offset_tn)) {
    new_opcode = TOP_opnd_immediate_variant(opcode, offset_idx, TN_value(new_offset_tn));
  } else {
    new_opcode = TOP_opnd_immediate_variant(opcode, offset_idx, 0xFFFFFFFF);
  }
  if (new_opcode == TOP_UNDEFINED ||
      !opcode_benefit(new_opcode,opcode)) return FALSE;
  
  new_op = Dup_OP(op);
  OP_srcpos(new_op) = OP_srcpos(op);
  OP_Change_Opcode(new_op, new_opcode);
  Set_OP_opnd(new_op, offset_idx, new_offset_tn);
  Set_OP_opnd(new_op, base_idx, new_base_tn);
  if (OP_memory(op)) Copy_WN_For_Memory_OP (new_op, op);
  BB_Insert_Op_After(OP_bb(op), op, new_op);
  if (EBO_Trace_Optimization) 
    fprintf(TFile,"Convert memory op base+offset\n");
  
  return TRUE;
}

/* ============================================================
 * EBO_literal_replacement_tn
 *
 * Returns a literal replacement TN for operations that are
 * not copy operations.
 *
 * ST200: Treat equivalent of mtb of 0 and mtb of 1.
 * ============================================================
 */
TN *
EBO_literal_replacement_tn(OP *op)
{
  TOP opcode = OP_code(op);
  if (opcode == TOP_mtb && OP_opnd(op, 0) == Zero_TN) {
    return Gen_Literal_TN (0, TN_size(OP_opnd(op, 0)));
  } else if (opcode == TOP_cmpeq_r_b && OP_opnd(op, 0) == Zero_TN && OP_opnd(op, 1) == Zero_TN) {
    return Gen_Literal_TN (1, TN_size(OP_opnd(op, 0)));
  }

  return NULL;
}

#if 0 // Was a tentative
static BOOL
operand_special_immediate(OP *op, TN **opnd_tn, EBO_TN_INFO **opnd_tninfo)
{
  TOP opcode = OP_code(op);
  TOP new_opcode;
  INT32 use_count;
  INT64 val;
  int i;
  for (i = 0; i < OP_opnds(op); i++) {
    if (!TN_Has_Value(OP_opnd(op,i)) && 
      opnd_tninfo[i] != NULL &&
      opnd_tninfo[i]->replacement_tn != NULL &&
      TN_has_value(opnd_tninfo[i]->replacement_tn)) {
      use_count = opnd_tninfo[i]->reference_count;
      val = TN_value(opnd_tninfo[i]->replacement_tn);
      new_opcode = TOP_opnd_immediate_variant(opcode, i, val);
      fprintf(stderr, "use count: %d, new opc: %s\n", use_count, TOP_Name(new_opcode));
      if (new_opcode != TOP_UNDEFINED &&
       ISA_PACK_Inst_Words(new_opcode) > ISA_PACK_Inst_Words(opcode) &&
        use_count <= 2) goto replace;
    }
  }
  return FALSE;
  
 replace:
  OP *new_op = Dup_OP(op);
  OP_Change_Opcode(new_op, new_opcode);
  Set_OP_opnd(new_op, i, opnd_tninfo[i]->replacement_tn);
  if (OP_memory(op)) Copy_WN_For_Memory_OP (new_op, op);
  if (EBO_Trace_Optimization) fprintf(TFile,"replace op %s with extended immediate form %s, use count %d\n", TOP_Name(opcode), TOP_Name(new_opcode), use_count);
  BB_Insert_Op_After(OP_bb(op), op, new_op);
  return TRUE;
}
#endif

BOOL
EBO_Special_Inline_Immediates(OP *op, EBO_OP_INFO *opinfo, int idx)
{
  TOP opcode;
  EBO_TN_INFO *tninfo;

  opcode = OP_code(op);
  tninfo = opinfo->actual_opnd[idx];

  /* Try base offset merging. 
     Replace offset [base] where base is constant into
     (base+offset) [R0]
  */
  if (OP_memory(op)) {
    int base_idx = OP_find_opnd_use(op, OU_base);
    int offset_idx = OP_find_opnd_use(op, OU_offset);
    TN *base_tn, *offset_tn, *base_replacement_tn;
    TN *new_offset_tn, *new_base_tn;
    EBO_TN_INFO *base_tninfo;
    INT64 offset_val, base_val;
    ST *base_sym = NULL;
    INT32 base_relocs;
    TOP new_opcode;

    if (base_idx < 0 && offset_idx < 0) return FALSE;
    if (idx != base_idx) return FALSE;
    base_tn = OP_opnd(op, base_idx);
    offset_tn = OP_opnd(op, offset_idx);
    if (!TN_is_constant(offset_tn)) return FALSE;
    if (TN_is_constant(base_tn) || base_tn == Zero_TN) return FALSE;
    
    base_replacement_tn = tninfo->replacement_tn;
    base_tninfo = tninfo;
    
    if (!inline_operand_profitable(op, base_tninfo)) return FALSE;
    if (TN_is_symbol(offset_tn) && TN_is_symbol(base_replacement_tn)) return FALSE;
    
    /* We now can perform the merging. */
    base_tn = base_replacement_tn;
    
    if (TN_is_symbol(base_tn)) {
      offset_val = TN_value(offset_tn);
      base_val = TN_offset(base_tn);
      base_sym = TN_var(base_tn);
      base_relocs = TN_relocs(base_tn);
    } else {
      if (TN_is_symbol(offset_tn)) {
	offset_val = TN_offset(offset_tn);
	base_sym = TN_var(offset_tn);
	base_relocs = TN_relocs(offset_tn);
      } else {
	offset_val = TN_value(offset_tn);
      }
      base_val = TN_value(base_tn);
    }
    
    if (base_sym != NULL) {
      new_offset_tn = Gen_Symbol_TN(base_sym, base_val + offset_val, base_relocs);
    } else {
      new_offset_tn = Gen_Literal_TN(base_val+offset_val, 4);
    }
    new_base_tn = Zero_TN;
    
    if (TN_has_value(new_offset_tn)) {
      new_opcode = TOP_opnd_immediate_variant(opcode, offset_idx, TN_value(new_offset_tn));
    } else {
      new_opcode = TOP_opnd_immediate_variant(opcode, offset_idx, 0xFFFFFFFF);
    }
    if (new_opcode == TOP_UNDEFINED) return FALSE;
    
    OP_Change_Opcode(op, new_opcode);
    Set_OP_opnd(op, offset_idx, new_offset_tn);
    Set_OP_opnd(op, base_idx, new_base_tn);
    if (EBO_Trace_Data_Flow) {
      #pragma mips_frequency_hint NEVER
      fprintf(TFile,"%sInlined tninfo base/offset:\n\t",
		  EBO_trace_pfx);
	  Print_OP_No_SrcLine(op);
    }
    return TRUE;
  } else {
    TOP new_opcode;
    TN *replacement_tn;

    replacement_tn = tninfo->replacement_tn;

    if (!inline_operand_profitable(op, tninfo)) return FALSE;

    if (TN_has_value(replacement_tn)) {
      new_opcode = TOP_opnd_immediate_variant(opcode, idx, TN_value(replacement_tn));
    } else {
      new_opcode = TOP_opnd_immediate_variant(opcode, idx, 0xFFFFFFFF);
    }
    if (new_opcode != TOP_UNDEFINED) {
      /* Inline the immediate into the operation. */
      OP_Change_Opcode(op, new_opcode);
      Set_OP_opnd(op, idx, replacement_tn);
      Reset_OP_copy(op); /* in case the op was a copy. */
      if (EBO_Trace_Data_Flow) {
	#pragma mips_frequency_hint NEVER
	fprintf(TFile,"%sInlined tninfo:\n\t",
		EBO_trace_pfx);
	Print_OP_No_SrcLine(op);
      }
      return TRUE;
    } else {
      /* Try to inline sub x, imm into add x, -imm. */
      if (idx == 1 && opcode == TOP_sub_r && TN_has_value(replacement_tn)) {
	TN *neg_tn = Gen_Literal_TN((INT64)(INT32)(-TN_value(replacement_tn)), 4);
	new_opcode = TOP_opnd_immediate_variant(TOP_add_r, idx, TN_value(neg_tn));
	if (new_opcode != TOP_UNDEFINED) {
	  /* Inline the immediate into the operation. */
	  OP_Change_Opcode(op, new_opcode);
	  Set_OP_opnd(op, idx, neg_tn);
	  Reset_OP_copy(op); /* in case the op was a copy. */
	  if (EBO_Trace_Data_Flow) {
#pragma mips_frequency_hint NEVER
	    fprintf(TFile,"%sInlined tninfo:\n\t",
		    EBO_trace_pfx);
	    Print_OP_No_SrcLine(op);
	  }
	  return TRUE;
	}	  
      }
      /* Try to inline immediate at a different position. */
      int idx2;
      for (idx2 = 0; idx2 < OP_opnds(op); idx2++) {
	if (idx == idx2) continue;
	if (TN_is_constant(OP_opnd(op, idx2))) continue;
	new_opcode = TOP_opnd_swapped_variant(opcode, idx, idx2);
	if (new_opcode == TOP_UNDEFINED) continue;
	if (TN_has_value(replacement_tn)) {
	  new_opcode = TOP_opnd_immediate_variant(new_opcode, idx2, TN_value(replacement_tn));
	} else {
	  new_opcode = TOP_opnd_immediate_variant(new_opcode, idx2, 0xFFFFFFFF);
	}
	if (new_opcode == TOP_UNDEFINED) continue;
	/* Inline the immediate into the operation after swapping operand. */
	OP_Change_Opcode(op, new_opcode);
	Set_OP_opnd(op, idx, OP_opnd(op, idx2));
	Set_OP_opnd(op, idx2, replacement_tn);
	Reset_OP_copy(op); /* in case the op was a copy. */
	if (EBO_Trace_Data_Flow) {
	  #pragma mips_frequency_hint NEVER
	  fprintf(TFile,"%sInlined tninfo at op index %d:\n\t",
		  EBO_trace_pfx, idx2);
	  Print_OP_No_SrcLine(op);
	}
	return TRUE;
      }
    }
  }
  return FALSE;
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
  BOOL ret;
  TOP opcode = OP_code(op);

  if (opnd_tn == NULL || opnd_tninfo == NULL) return FALSE;

  if (EBO_Trace_Optimization) 
    fprintf(TFile,"%sEnter EBO_Special_Sequence for op %s\n", EBO_trace_pfx, TOP_Name(opcode));

#ifdef Is_True_On
  /* Tentative: to set inline_extended_immediate. */
  inline_extended_immediate =  getenv("INLINE_EXT") != NULL;
  if (getenv("INLINE_THRESHOLD") != NULL) {
    inline_operand_profitable_threshold = atoi(getenv("INLINE_THRESHOLD"));
  }
#endif

  // First try to strength reduce the 32 bit multiplies:
  if (Strength_Reduce_Mul (op, opnd_tn, opnd_tninfo)) {
    return TRUE;
  }
  
  if (!(EBO_in_pre && IS_MUL32_PART(op))) {
    // Do not do it before the 32x32 multiplies have been
    // reduced. However, for now we only have pre_process vs
    // process choice. Perhaps, eventually have several 
    // pre_process passes ?
    if (mul_fix_operands (op, opnd_tn, opnd_tninfo)) return TRUE;
  }
  
  // TODO: add shl/shr/ext to semantics
  if (IS_SHR_16(op)) {
    if (shr_shl_sequence (op, opnd_tn, opnd_tninfo)) return TRUE;
  }

  if (IS_SHL_16(op)) {
    if (shl_mulhs_sequence (op, opnd_tn, opnd_tninfo)) return TRUE;
  }
  
  if (OP_iadd(op)) {
    if (add_shl_sequence (op, opnd_tn, opnd_tninfo)) return TRUE;
    if (iadd_special_case (op, opnd_tn, opnd_tninfo)) return TRUE;
  }

  if (OP_select(op)) {
    if (select_move_sequence(op, opnd_tn, opnd_tninfo)) return TRUE;
    if (select_move_sequence_2(op, opnd_tn, opnd_tninfo)) return TRUE;
    if (select_invert_sequence(op, opnd_tn, opnd_tninfo)) return TRUE;
    if (min_max_sequence(op, opnd_tn, opnd_tninfo)) return TRUE;
    if (abs_sequence(op, opnd_tn, opnd_tninfo)) return TRUE;
    if (select_select_sequence(op, opnd_tn, opnd_tninfo)) return TRUE;
  }

  if (OP_icmp(op)) {
    if (cmp_move_sequence(op, opnd_tn, opnd_tninfo)) return TRUE;
    if (logical_move_sequence(op, opnd_tn, opnd_tninfo)) return TRUE;
    if (andl_orl_sequence(op, opnd_tn, opnd_tninfo)) return TRUE;
    if (andl_orl_sequence_2(op, opnd_tn, opnd_tninfo)) return TRUE;
  }

  if (opcode == TOP_mtb && mtb_op_sequence(op, opnd_tn, opnd_tninfo)) return TRUE;
  if (opcode == TOP_mfb && mfb_op_sequence(op, opnd_tn, opnd_tninfo)) return TRUE;
  
  if (OP_memory(op) && base_offset_sequence(op, opnd_tn, opnd_tninfo)) return TRUE;

  if (ext_move_sequence(op, opnd_tn, opnd_tninfo)) return TRUE;

  if (and_or_sequence(op, opnd_tn, opnd_tninfo)) return TRUE;

  if (addcg_sequence(op, opnd_tn, opnd_tninfo)) return TRUE;

  if (operand_special_sequence(op, opnd_tn, opnd_tninfo)) return TRUE;

  
#if 0 //Was a tentative
  if (!EBO_in_pre && !inline_extended_immediate &&
      operand_special_immediate(op, opnd_tn, opnd_tninfo)) return TRUE;
#endif

#if 0
  if (OP_store(op)) {
    return (store_sequence( op, opnd_tn, opnd_tninfo) ||
            sxt_sequence  ( op, TOP_Find_Operand_Use(OP_code(op),OU_storeval),
                            opnd_tn, opnd_tninfo));
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
