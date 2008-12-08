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
#include "stblock.h" // for ST_alignment
#include "cg_ivs.h" // For packing
#endif

#include "targ_cg_private.h"

/* Define a macro to strip off any bits outside of the left most 4 bytes. */
#define TRUNC_32(val) (val & 0x00000000ffffffffll)

/* Define a macro to sign-extend the least signficant 32 bits */
#define SEXT_32(val) (((INT64)(val) << 32) >> 32)

// [HK] added ST231 Helpers
#define IS_MUL_SPEC(o)     (OP_code(o) == TOP_mulhs_i_r_r ||   \
                            OP_code(o) == TOP_mulhs_ii_r_r ||  \
                            OP_code(o) == TOP_mulhs_r_r_r || \
                            OP_code(o) == TOP_mulhhs_i_r_r ||   \
                            OP_code(o) == TOP_mulhhs_ii_r_r ||  \
                            OP_code(o) == TOP_mulhhs_r_r_r || \
                            OP_code(o) == TOP_mullhus_i_r_r ||   \
                            OP_code(o) == TOP_mullhus_ii_r_r ||  \
                            OP_code(o) == TOP_mullhus_r_r_r || \
                            OP_code(o) == TOP_mul64h_i_r_r ||   \
                            OP_code(o) == TOP_mul64h_ii_r_r ||  \
                            OP_code(o) == TOP_mul64h_r_r_r || \
                            OP_code(o) == TOP_mul64hu_i_r_r ||   \
                            OP_code(o) == TOP_mul64hu_ii_r_r ||  \
                            OP_code(o) == TOP_mul64hu_r_r_r || \
                            OP_code(o) == TOP_mulfrac_i_r_r ||   \
                            OP_code(o) == TOP_mulfrac_ii_r_r ||  \
                            OP_code(o) == TOP_mulfrac_r_r_r)  
                        

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
 *     EBO_Verify_Op
 *	EBO_Verify_Operand
 *
 * Verify that the generated op list can be emitted.
 * Should be called each time an op list in on the point to be
 * inserted into a basic block.
 *
 * The verifications are done in EBO_in_peep (after reg_alloc):
 * - Verify that all operands are assigned if in peep (after reg alloc).
 * - Verify that all operands are in a valid register subclass.
 *
 * =====================================================================
 */
static BOOL
EBO_Verify_Operand(OP *op, INT opnd, BOOL is_result)
{
  FmtAssert(ISA_SUBSET_LIST_Member(ISA_SUBSET_List, OP_code(op)),
	    ("%s is a member of available ISA subsets", 
	     TOP_Name(OP_code(op))));

  const ISA_OPERAND_INFO *oinfo  = ISA_OPERAND_Info(OP_code(op));
  const ISA_OPERAND_VALTYP *vtype = is_result 
    ? ISA_OPERAND_INFO_Result(oinfo, opnd)
    : ISA_OPERAND_INFO_Operand(oinfo, opnd);
  TN *tn = is_result ? OP_result(op, opnd) : OP_opnd(op, opnd);
  
  if (ISA_OPERAND_VALTYP_Is_Register(vtype)) {
    REGISTER_SET class_regs;
    ISA_REGISTER_SUBCLASS sc = ISA_OPERAND_VALTYP_Register_Subclass(vtype);
    ISA_REGISTER_CLASS rc = ISA_OPERAND_VALTYP_Register_Class(vtype);
    REGISTER reg = TN_register(tn);
    FmtAssert(TN_is_register(tn), ("expected register"));
    
    if (reg == REGISTER_UNDEFINED) goto failed;
    FmtAssert(TN_register_class(tn) == rc, ("expected register"));
    class_regs =   (sc == ISA_REGISTER_SUBCLASS_UNDEFINED)
      ? REGISTER_CLASS_universe(rc)
      : REGISTER_SUBCLASS_members(sc);
    if (!REGISTER_SET_MemberP(class_regs, reg)) goto failed;
  }
  return TRUE;
 failed:
  return FALSE;

}

static BOOL
EBO_Verify_Op(OP *op)
{
  int i;
  if (!EBO_in_peep) return TRUE;
  for (i = 0; i < OP_opnds(op); i++) {
    if (!EBO_Verify_Operand(op, i, FALSE)) return FALSE;
  }
  for (i = 0; i < OP_results(op); i++) {
    if (!EBO_Verify_Operand(op, i, TRUE)) return FALSE;
  }
  return TRUE;
}

BOOL
EBO_Verify_Ops(OPS *ops)
{
  OP *op;
  if (!EBO_in_peep) return TRUE;
  for (op = OPS_first(ops); op != NULL; op = OP_next(op)) {
    if (!EBO_Verify_Op(op)) return FALSE;
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
    case TOP_ldw_r_i_r:
      return 4;

    case TOP_and_i_r_r: {

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
      ((p_opcode == TOP_cmpeq_r_r_r) && (e_opcode == TOP_cmpne_r_r_r))) {

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
	if (!EBO_Verify_Op(new_op)) return FALSE;
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
  TN **opnd_tn,
  EBO_TN_INFO **opnd_tninfo,
  EBO_OP_INFO *opinfo,
  INT64 offset_pred,
  INT64 offset_succ
)
{
  OP *pred_op = opinfo->in_op;
  BB *bb = OP_bb(op);
  TOP opcode = OP_code(op);
  TN *pred_result = OP_result(pred_op,0);
  TN *succ_result = OP_result(op,0);
  INT base_index = TOP_Find_Operand_Use(OP_code(op),OU_base);
  INT offset_index = TOP_Find_Operand_Use(OP_code(op),OU_offset);
  TN *base_tn, *offset_tn;
  EBO_TN_INFO *base_tninfo;
  INT size_pred;
  INT size_succ;
  TN *r1;
  TN *r2;
  OPS ops = OPS_EMPTY;

  if ((CG_LOOP_Packing_flags & PACKING_EBO_LOAD) == 0)
    return FALSE;

  if (!Enable_64_Bits_Ops)
    return FALSE;

  // FdF 20061027: Perform load packing in EBO_main only
  if (EBO_in_pre ||
      EBO_in_before_unrolling || EBO_in_after_unrolling)
    return FALSE;

  // FdF 20070419: EBO_in_peep would be needed to perform packing on
  // spill accesses (codex #26354).
  if (EBO_in_peep)
    return FALSE;
    
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

  // FdF 20061027
  if (!Enable_Misaligned_Access && !Enable_Misaligned_Load) {
    // Check if the alignment is known
    INT base_alignment = -1;
    TN *base_tn = opnd_tn[base_index];
    TN *offset_tn = opnd_tn[offset_index];
    if ((base_tn == SP_TN) && ((Stack_Alignment()&0x7) == 0)) {
      ST *base_sym = NULL;
      INT base_offset = 0;
      if (TN_is_symbol(offset_tn)) {
	INT64 base_ofst;
	base_sym = TN_var(offset_tn);
	base_offset = TN_offset(offset_tn);
	Base_Symbol_And_Offset(base_sym, &base_sym, &base_ofst);
	base_offset += base_ofst;
	if ((base_sym == NULL) || ((ST_alignment(base_sym)&0x7) == 0))
	  base_alignment = base_offset & 0x7;
      }
      else if (TN_has_value(offset_tn))
	base_alignment = TN_value(offset_tn) & 0x7;
    }
    else if (TN_is_symbol(base_tn) && TN_has_value(offset_tn)) {
      ST *base_sym = TN_var(base_tn);
      if ((base_sym != NULL) && ((ST_alignment(base_sym) &0x7) == 0))
	base_alignment = (TN_offset(base_tn) + TN_value(offset_tn)) & 0x7;
    }

    else if (TN_has_value(base_tn) && TN_is_symbol(offset_tn)) {
      ST *offset_sym = TN_var(offset_tn);
      if ((offset_sym != NULL) && ((ST_alignment(offset_sym) &0x7) == 0))
	base_alignment = (TN_offset(offset_tn) + TN_value(base_tn)) & 0x7;
    }
    if (((base_alignment == 4) && (offset_succ > offset_pred)) ||
	((base_alignment == 0) && (offset_succ < offset_pred))) {
    }
    else {
      if (EBO_Trace_Data_Flow) {
	fprintf(TFile,"%sCannot packed load if unaligned 64-bits is not supported.\n", EBO_trace_pfx);
      }
      return FALSE;
    }
  }

  // FdF 20080327: Some predicated OPs have OP_cond_def
  // reseted.
  //  if (OP_cond_def(op) || OP_cond_def(pred_op)) {
  if (OP_has_predicate(op) || OP_has_predicate(pred_op)) {
    if (EBO_Trace_Data_Flow) {
      fprintf(TFile,"%sCannot packed predicated loads.\n", EBO_trace_pfx);
    }
    return FALSE;
  }

  size_pred = OP_Mem_Ref_Bytes(pred_op);
  size_succ = OP_Mem_Ref_Bytes(op);
  if (size_pred != size_succ) return FALSE;

  // FdF 20061027: Only 32 to 64 bit packing is supported
  if (size_pred != 4) return FALSE;

  
  // FdF 20070511: If op and pred_op are not in the same basic block,
  // it is too complicated to check that moving one operation close to
  // the other does not break anything.
  if (bb != opinfo->in_bb) {
    if (EBO_Trace_Data_Flow) {
      fprintf(TFile,"%spred_op and op, candidates for packing, are in different blocks.\n", EBO_trace_pfx);
    }
    return FALSE;
  }

  BOOL replace_pred_op = FALSE;

  if ((opinfo->actual_rslt[0] == NULL) ||
      (opinfo->actual_rslt[0]->reference_count != 0) ||
      !EBO_tn_available (opinfo->in_bb, opinfo->actual_rslt[0])) {

    // FdF 200601027: pred_result is used or defined between pred_op
    // and succ_op. If succ_result is not used or defined between
    // pred_op and succ_op, then the paired-load can be moved just
    // after pred_op.
    replace_pred_op = TRUE;

    for (OP *op_iter = OP_next(pred_op); op_iter != op; op_iter = OP_next(op_iter)) {
      for (INT idx = 0; idx < OP_results(op_iter); idx++) {
	if (OP_result(op_iter, idx) == succ_result) {
	  if (EBO_Trace_Data_Flow) {
	    fprintf(TFile,"%sThe result of the successor load is defined between pred_op and op.\n", EBO_trace_pfx);
	  }
	  return FALSE;
	}
      }
      for (INT idx = 0; idx < OP_opnds(op_iter); idx++) {
	if (OP_opnd(op_iter, idx) == succ_result) {
	  if (EBO_Trace_Data_Flow) {
	    fprintf(TFile,"%sThe result of the successor load is used between pred_op and op.\n", EBO_trace_pfx);
	  }
	  return FALSE;
	}
      }
    }
  }

  // FdF 20070511: In any case, memory dependences with operations
  // between pred_op and op must be checked.

  for (OP *op_iter = OP_next(pred_op); op_iter != op; op_iter = OP_next(op_iter)) {
    if (!OP_store(op_iter))
      continue;
    if (replace_pred_op) {
      BOOL definite;
      if (CG_DEP_Mem_Ops_Alias(op_iter, op, &definite)) {
	if (EBO_Trace_Data_Flow) {
	  fprintf(TFile,"%sop is aliases with a store operation before pred_op.\n", EBO_trace_pfx);
	}
	return FALSE;
      }
    }
    else {
      BOOL definite;
      if (CG_DEP_Mem_Ops_Alias(pred_op, op_iter, &definite)) {
	if (EBO_Trace_Data_Flow) {
	  fprintf(TFile,"%spred_op is aliases with a store operation before op.\n", EBO_trace_pfx);
	}
	return FALSE;
      }
    }
  }

  OP *op1, *op2;
  if (offset_pred < offset_succ) {
    op1 = pred_op;
    op2 = op;
    base_tn = OP_opnd(pred_op, base_index);
    offset_tn = OP_opnd(pred_op, offset_index);
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
    op1 = op;
    op2 = pred_op;
    base_tn = OP_opnd(op, base_index);
    offset_tn = OP_opnd(op, offset_index);
    base_tninfo = opnd_tninfo[base_index];
    r1 = succ_result;
    r2 = pred_result;
  }

  if (EBO_in_peep) {
    // Must check also that (r1,r2) are assigned (R2n,R2n+1), and generate multi
    FmtAssert(0, ("EBO_combine_adjacent_loads: Not implemented for EBO_in_peep"));
  }
  else {

    extern void Expand_Extract(TN *low_tn, TN *high_tn, TN *src_tn, OPS *ops);

    TN *t64 = Gen_Register_TN (ISA_REGISTER_CLASS_integer, 8);
    Expand_Load(OPC_I8I8LDID, t64, base_tn, offset_tn, &ops);
    if (Target_Byte_Sex == LITTLE_ENDIAN)
      Expand_Extract(r1, r2, t64, &ops);
    else
      Expand_Extract(r2, r1, t64, &ops);
  }
  
  // FdF 20070510: Do not copy the WN information, but create a map
  // op->(op1,op2)
  // Copy_WN_For_Memory_OP (OPS_first(&ops), op);
  Set_Packed_Ops(OPS_first(&ops), 2, op1, op2);
  OP_srcpos(OPS_first(&ops)) = OP_srcpos(op);

  if (!EBO_Verify_Ops(&ops)) return FALSE;
  if (replace_pred_op)
    BB_Insert_Ops(OP_bb(pred_op), pred_op, &ops, FALSE);
  else
    BB_Insert_Ops(OP_bb(op), op, &ops, FALSE);

  // FdF 20061027: Since we placed the new operations after pred_op,
  // they will not be processed now by the EBO. So, update opinfo
  // here.
  if (!replace_pred_op)
    remove_op (opinfo);
  else {
    // FdF 20061027: The following code will do as if the ldl and
    // extract operations were normally processed by EBO. However,
    // there is probably not much opportunity for optimization, so
    // just do not provide information for these operations.
    OP *ld64_op = OPS_first(&ops);
    if (OP_no_alias(op) && OP_no_alias(pred_op)) Set_OP_no_alias(ld64_op);
    if (OP_spill(op) || OP_spill(pred_op)) Set_OP_spill(ld64_op);
    // FdF 20071130: Forget information from the original ldw
    // operation
    EBO_opinfo_table[EBO_hash_op(ld64_op, opnd_tninfo)] = NULL;
  }

  opinfo->in_op = NULL;
  opinfo->in_bb = NULL;
  OP_Change_To_Noop(pred_op);

  if (EBO_Trace_Optimization) {
    #pragma mips_frequency_hint NEVER
    fprintf(TFile,"%sReplace adjacent load with load pair.\n",EBO_trace_pfx);
  }

  return TRUE;
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
	new_opcode = TOP_is_unsign(OP_code(op)) ? TOP_zxth_r_r : TOP_sxth_r_r;
	tn1 = pred_result;
	tn2 = NULL;
      } else if (size_succ == 1) {
	if (TOP_is_unsign(OP_code(op))) {
	  new_opcode = TOP_and_i_r_r;
	  tn1 = pred_result;
	  tn2 = Gen_Literal_TN(0xff, 4);
	} else {
	  new_opcode = TOP_sxtb_r_r;
	  tn1 = pred_result;
	  tn2 = NULL;
	}
      }
      if (new_opcode == TOP_UNDEFINED) return FALSE;
      OP *new_op;
      new_op = Mk_OP(new_opcode, succ_result,tn1,tn2);
      OP_srcpos(new_op) = OP_srcpos(op);
      if (!EBO_Verify_Op(new_op)) return FALSE;
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
  // FdF 20081013: Generate an unconditional copy on a predicated op
  // with UNC_DEF property. Codex bug #54572
  TN *predicate_tn = OP_has_predicate(op) && (OP_cond_def_kind(op) != OP_ALWAYS_UNC_DEF) ?
    OP_opnd(op, OP_find_opnd_use(op,OU_predicate)) :
    NULL;

  if ((size != 4) && OP_unsigned_ld(op)) {
    /* 
     * Does it make sense to use bit-extract sequence to simulate 
     * store-load sequence ?
     */
    return FALSE;
  }

  /* Use full word copy. */
  EBO_Exp_COPY(predicate_tn, false, OP_cond_def(op), OP_result(op, 0), pred_result, &ops);
  if (!EBO_Verify_Ops(&ops)) return FALSE;

  OP_srcpos(OPS_last(&ops)) = OP_srcpos(op);

  if (!EBO_Verify_Ops(&ops)) return FALSE;
  BB_Insert_Ops(OP_bb(op), op, &ops, FALSE);

  return TRUE;
}

/* =====================================================================
 *   EBO_select_value
 *
 *   Compare the reload address with the intervening store address.
 *   Select the stored value if the address are the same,
 *   and resuse the predecesor value if they are not the same.
 *
 * =====================================================================
 */
BOOL
EBO_select_value (
  OP *op,
  TN *pred_result,         // preceding store's result
  TN *pred_base,           // preceding store's base
  TN *pred_offset,
  TN *intervening_result,
  TN *intervening_base,
  TN *intervening_offset,
  INT size                 // size of load
)
{
  OPS ops = OPS_EMPTY;
  TN *result_predicate = NULL;
  
  /* Check whether we need to predicate the definition. If so
   * we will use the current predicate of the operation.
   */
  if (OP_cond_def (op)) {
    result_predicate = OP_opnd(op, OP_find_opnd_use(op,OU_predicate));
  }
  
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
    ST *intervening_st = TN_var(intervening_offset);
    OPS ops1 = OPS_EMPTY;

    // FdF 20070525: Use Expand_Copy to support 8 bytes types
    if (pred_st == intervening_st) {
      Expand_Copy(OP_result(op, 0), result_predicate, intervening_result, &ops1);
    }
    else {
      Expand_Copy(OP_result(op, 0), result_predicate, pred_result, &ops1);
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

    Build_OP (TOP_cmpeq_r_r_b, predicate, pred_base, intervening_base, &ops);
    OP_srcpos(OPS_last(&ops)) = OP_srcpos(op);

    if (result_predicate != NULL) {
      /* If predicated, we must generate a predicated select. On
	 ST200 family this means a select followed by a conditional
	 copy.
      */
      TN *tmp_tn = Build_TN_Like(OP_result(op, 0));
      TYPE_ID mtype = TN_size(OP_result(op, 0)) == 8 ? MTYPE_I8 : MTYPE_I4;

      /* Copy the "address not equal value". */
      Expand_Select(tmp_tn, predicate, intervening_result, pred_result, mtype, FALSE, &ops);
      OP_srcpos(OPS_last(&ops)) = OP_srcpos(op);
      
      /* If predicated, move conditionally into the result. */
      Expand_Copy (OP_result(op, 0), result_predicate, tmp_tn, &ops);
      OP_srcpos(OPS_last(&ops)) = OP_srcpos(op);
    } else {
      /* Copy the "address not equal value". */
      TYPE_ID mtype = TN_size(OP_result(op, 0)) == 8 ? MTYPE_I8 : MTYPE_I4;
      Expand_Select(OP_result(op, 0), predicate, intervening_result, pred_result, mtype, FALSE, &ops);
      OP_srcpos(OPS_last(&ops)) = OP_srcpos(op);
    }
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

  case TOP_cmpeq_i_r_r:
    /*
     * For signed compares sign-extend the constant to 64 bits.
     */
    return SEXT_32(immed);
  case TOP_cmpleu_i_r_r:
    /*
     * For unsigned compares, zero-extend the constant to 64 bits.
     */
    return TRUNC_32(immed);
  }
  return immed;
}


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
  DevAssert(opnd1_idx >= 0, ("OU_opnd1 not defined for TOP %s\n", TOP_Name(opcode)));

  if (EBO_Trace_Optimization) { 
    fprintf(TFile,"Enter EBO_simplify_operand0: %s ", TOP_Name(opcode));
    fprintf(TFile," %lld ", const_val);
    Print_TN(tn1,FALSE);
    fprintf(TFile," : tns ");
    Print_TN(OP_opnd(op, opnd1_idx),FALSE);
    if (opnd2_idx >= 0) {
      fprintf(TFile," ");
      Print_TN(OP_opnd(op, opnd2_idx),FALSE);
      fprintf(TFile,"\n");
    }
  }

  // [HK] simplify special mul by 0 to 0
  if (IS_MUL_SPEC(op) && const_val == 0){
      new_op = Mk_OP(TOP_mov_r_r, tnr, Zero_TN);
      return new_op;
  }
     

  /* shadd -> add */
  if (targ_cg_TOP_is_shadd(opcode)) {
    TN *new_tn0, *new_tn1;
    int n = targ_cg_TOP_shadd_amount(opcode);
    INT64 new_val = SEXT_32(const_val << n);
    
    if (!TN_is_register(OP_opnd(op,opnd2_idx))) return NULL;
    new_opcode = TOP_add_r_r_r;
    new_tn0 = OP_opnd(op,opnd2_idx);
    if (new_val == 0) new_tn1 = Zero_TN;
    else new_tn1 = Gen_Literal_TN(new_val, 4);
    if (TN_has_value(new_tn1))
      new_opcode = TOP_opnd_immediate_variant(new_opcode, opnd2_idx, TN_value(new_tn1));
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
      Reset_OP_copy (new_op);
      Set_OP_opnd(new_op,opnd1_idx, Gen_Literal_TN(const_val, 4));
      if (EBO_Trace_Optimization) fprintf(TFile,"replace op %s with immediate form %s\n", TOP_Name(opcode), TOP_Name(new_opcode));
      return new_op;
    }
  }
  
  /* Invert operands and try to simplify operand 1. */
  if (TN_is_register(OP_opnd(op, opnd1_idx)) &&
      OP_opnd(op, opnd1_idx) != Zero_TN &&
      opnd2_idx >= 0 &&
      TN_is_register(OP_opnd(op, opnd2_idx))) {
    // TOP_opnd_swapped_variant does not exist anymore
    new_opcode = OP_opnd_swapped_variant(op, opnd1_idx, opnd2_idx);
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
  DevAssert(opnd1_idx >= 0 && opnd2_idx >= 0, ("OU_opnd1 and/or OU_opnd2 not defined for TOP %s\n", TOP_Name(opcode)));

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

  // [HK] simplify special mul by 0 to 0
  if (IS_MUL_SPEC(op) && const_val == 0){
      new_op = Mk_OP(TOP_mov_r_r, tnr, Zero_TN);
      return new_op;
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

  if (OP_code(op) == TOP_br_i_b) {
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

      Build_OP (TOP_goto_i, 
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

  /* Nothing target specific to fold. */
  return FALSE;

 Folded:

  if (EBO_Trace_Optimization) {
    fprintf(TFile, "%sfolded???: %llx\n", EBO_trace_pfx, *result_val);
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
 *                          Helper functions
 * =====================================================================
 */
#define IS_SHL_16(o)   (OP_code(o) == TOP_shl_i_r_r && \
                        TN_is_constant(OP_opnd(o,1)) && \
                        TN_value(OP_opnd(o,1)) == 16)

#define IS_SHR_16(o)   ((OP_code(o) == TOP_shr_i_r_r || OP_code(o) == TOP_shru_i_r_r) && \
                        TN_is_constant(OP_opnd(o,1)) && \
                        TN_value(OP_opnd(o,1)) == 16)

#define IS_MULL(o)      (OP_code(o) == TOP_mull_i_r_r ||   \
                         OP_code(o) == TOP_mull_ii_r_r ||  \
                         OP_code(o) == TOP_mull_r_r_r ||   \
			 OP_code(o) == TOP_mullu_i_r_r ||  \
                         OP_code(o) == TOP_mullu_ii_r_r || \
                         OP_code(o) == TOP_mullu_r_r_r)

#define IS_MULH(o)      (OP_code(o) == TOP_mulh_i_r_r ||   \
                         OP_code(o) == TOP_mulh_ii_r_r ||  \
                         OP_code(o) == TOP_mulh_r_r_r ||   \
			 OP_code(o) == TOP_mulhu_i_r_r ||  \
                         OP_code(o) == TOP_mulhu_ii_r_r || \
                         OP_code(o) == TOP_mulhu_r_r_r)

#define IS_MULLH(o)     (OP_code(o) == TOP_mullh_i_r_r ||   \
                         OP_code(o) == TOP_mullh_ii_r_r ||  \
                         OP_code(o) == TOP_mullh_r_r_r ||   \
			 OP_code(o) == TOP_mullhu_i_r_r ||  \
                         OP_code(o) == TOP_mullhu_ii_r_r || \
                         OP_code(o) == TOP_mullhu_r_r_r)

#define IS_MULLL(o)     (OP_code(o) == TOP_mulll_i_r_r ||   \
                         OP_code(o) == TOP_mulll_ii_r_r ||  \
                         OP_code(o) == TOP_mulll_r_r_r ||   \
			 OP_code(o) == TOP_mulllu_i_r_r ||  \
                         OP_code(o) == TOP_mulllu_ii_r_r || \
                         OP_code(o) == TOP_mulllu_r_r_r)

#define IS_MUL32_PART(o) (OP_code(o) == TOP_mulhs_r_r_r || \
			  OP_code(o) == TOP_mullu_r_r_r)

// [HK] add ST231 specific multiplies helpers
#define IS_MUL32(o)     (OP_code(o) == TOP_mul32_i_r_r ||   \
                         OP_code(o) == TOP_mul32_ii_r_r ||  \
                         OP_code(o) == TOP_mul32_r_r_r)

#define IS_MUL64(o)     (OP_code(o) == TOP_mul64h_i_r_r ||   \
                         OP_code(o) == TOP_mul64h_ii_r_r ||  \
                         OP_code(o) == TOP_mul64h_r_r_r ||   \
			 OP_code(o) == TOP_mul64hu_i_r_r ||  \
                         OP_code(o) == TOP_mul64hu_ii_r_r || \
                         OP_code(o) == TOP_mul64hu_r_r_r)

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
  {      TOP_mulllu_r_r_r,  TOP_mullhu_r_r_r, TOP_UNDEFINED  },   // lo 16 bits
  {     TOP_UNDEFINED,  TOP_mulhhu_r_r_r, TOP_UNDEFINED  },   // hi 16 bits
  {      TOP_mullu_r_r_r,   TOP_mulhu_r_r_r,  TOP_UNDEFINED  }    // 32 bits
};

static const TOP signed_mul_opcode[3][3] = {
  // By:  lo 16 bit      hi 16 bit       32 bit
  {      TOP_mulll_r_r_r,  TOP_mullh_r_r_r, TOP_UNDEFINED  },   // lo 16 bits
  {     TOP_UNDEFINED, TOP_mulhh_r_r_r, TOP_UNDEFINED  },   // hi 16 bits
  {      TOP_mull_r_r_r,   TOP_mulh_r_r_r,  TOP_UNDEFINED  }    // 32 bits
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

  if (opcode != TOP_UNDEFINED
      && ! ISA_SUBSET_LIST_Member (ISA_SUBSET_List, opcode))
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
 * Property driven cases are:
 * - loads
 * - shift right
 * - sext/zext
 * - and
 *
 * Target specific for STxP70 are:
 * - TOP_bool (generate 1 bit register)
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
    const ISA_OPERAND_INFO *oinfo = ISA_OPERAND_Info(opcode);
    const ISA_OPERAND_VALTYP *rtype = ISA_OPERAND_INFO_Result(oinfo, def_idx);
    ISA_REGISTER_CLASS cl = ISA_OPERAND_VALTYP_Register_Class (rtype);

    if (cl == ISA_REGISTER_CLASS_integer) {
      *def_bits = 1;
    } else {
      *def_bits = REGISTER_bit_size (cl, REGISTER_MIN);
    }
    *def_signed = 0;
    return TRUE;
  }

  if (OP_code(op) == TOP_convbi_b_r) {
    *def_bits = 1;
    *def_signed = 0;
    return TRUE;
  }

  if (OP_code(op) == TOP_sats_r_r &&
      (opnd1_idx = TOP_Find_Operand_Use(opcode,OU_opnd1)) >= 0){
    *def_bits = MIN(TOP_opnd_use_bits(opcode, opnd1_idx), 16);
    *def_signed = TOP_opnd_use_signed(opcode, opnd1_idx);
    return TRUE;
  }
  return FALSE;
}

/*
 * use_bit_width
 *
 * Returns true if the effective used bit of the operand def can
 * be guessed and returns bit width.
 * Property driven cases are:
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

  if (bits < 0) {
    return FALSE;
  }
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
 * Returns true if the operation acts as an extension for the given operand.
 * Set signed to true if sign extension
 * TODO: generic
 */
static BOOL
OP_is_extension(OP *op, INT32 opnd_idx, INT32 *ext_bits, INT32 *ext_signed)
{
  INT32 opnd1_idx, opnd2_idx;
  INT64 val;
  TOP opcode = OP_code(op);
  
  if (OP_results(op) != 1) return FALSE;
  
  if ((OP_sext(op) || OP_zext(op))) {
    opnd1_idx = TOP_Find_Operand_Use(opcode,OU_opnd1);
    DevAssert(opnd1_idx >= 0, ("missing OU_opnd1 for OP_sext/OP_zext"));
    if (opnd1_idx == opnd_idx) {
      *ext_bits = TOP_opnd_use_bits(opcode, opnd1_idx);
      *ext_signed = TOP_opnd_use_signed(opcode, opnd1_idx);
      return TRUE;
    }
  }

  if (OP_iand(op)) {
    opnd1_idx = TOP_Find_Operand_Use(opcode,OU_opnd1);
    opnd2_idx = TOP_Find_Operand_Use(opcode,OU_opnd2);
    DevAssert(opnd1_idx >= 0, ("missing OU_opnd1 for OP_sext/OP_zext"));
    DevAssert(opnd2_idx >= 0, ("missing OU_opnd2 for OP_sext/OP_zext"));
    if (opnd1_idx == opnd_idx &&
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
  }
  return FALSE;
}

/* =====================================================================
 * Get_Constant_Value
 * Returns a constant TN if the tninfo corresponds to a constsant value 
 * tn.
 * Return NULL otherwise.
 * =====================================================================
 */
static TN *
Get_Constant_Value(EBO_TN_INFO *tninfo)
{
  TN *const_tn = NULL;
  TN *tn;
  
  if (tninfo->replacement_tn != NULL)
    tn = tninfo->replacement_tn;
  else 
    tn = tninfo->local_tn;
  
  if (TN_Has_Value(tn))
    const_tn = tn;
  else if (TN_is_rematerializable(tn)) {
    WN *remat = TN_remat(tn);
    if (WN_opcode (remat) == OPC_I4INTCONST ||
	WN_opcode (remat) == OPC_U4INTCONST) {
      const_tn = Gen_Literal_TN ((INT32) WN_const_val(remat), 4);
    }
  }
  return const_tn;
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
  OP *op = opinfo != NULL ? opinfo->in_op : NULL;
  TN *const_tn;

  if ((opnd_tninfo != NULL &&
       (const_tn = Get_Constant_Value(opnd_tninfo)) != NULL) ||
      (op != NULL && 
       (OP_code(op) == TOP_mov_i_r || OP_code(op) == TOP_mov_ii_r) &&
       (const_tn = OP_opnd(op,0)) != NULL &&
       TN_Has_Value(const_tn))) {
    INT64 value = TN_Value(const_tn);
    if (value >= -32768 && value <= 32767) {
      *ret = opnd_tninfo->local_tn;
      *ret_tninfo = opnd_tninfo;
      *sign_ext = SIGN_EXT;
      *hilo = TN_LO_16;
      if (EBO_tn_available (bb, *ret_tninfo)) return TRUE;
    } else if (value >= 0 && value <= 65535) {
      *ret = opnd_tninfo->local_tn;
      *ret_tninfo = opnd_tninfo;
      *sign_ext = ZERO_EXT;
      *hilo = TN_LO_16;
      if (EBO_tn_available (bb, *ret_tninfo)) return TRUE;
    }
  }
  
  if (opinfo == NULL || op == NULL) 
    return FALSE;

  if (IS_SHR_16(op)) {
    *ret = OP_opnd(op,0);
    *ret_tninfo = opinfo->actual_opnd[0];
    *sign_ext = TOP_is_unsign(OP_code(op)) ? ZERO_EXT : SIGN_EXT;
    *hilo = TN_HI_16;
    if (EBO_tn_available (bb, *ret_tninfo)) return TRUE;
  }

  if (OP_code(op) == TOP_sxth_r_r) {
    *ret = OP_opnd(op,0);
    *ret_tninfo = opinfo->actual_opnd[0];
    *sign_ext = SIGN_EXT;
    *hilo = TN_LO_16;
    if (EBO_tn_available (bb, *ret_tninfo)) return TRUE;
  }

  if (OP_code(op) == TOP_zxth_r_r ||
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
  if (opcode == TOP_targ_addcg_b_r_r_b_r) return FALSE;

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
      ((OP_code(l2_tninfo0->in_op) != TOP_shl_i_r_r) &&
       (OP_code(l2_tninfo0->in_op) != TOP_shl_ii_r_r)
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
        ((OP_code(l2_tninfo0->in_op) != TOP_shl_i_r_r) &&
	 (OP_code(l2_tninfo0->in_op) != TOP_shl_ii_r_r)
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
  if (l3_val1 == 1) new_topcode = TOP_sh1add_r_r_r;
  else if (l3_val1 == 2) new_topcode = TOP_sh2add_r_r_r;
  else if (l3_val1 == 3) new_topcode = TOP_sh3add_r_r_r;
  else if (l3_val1 == 4 && ISA_SUBSET_LIST_Member (ISA_SUBSET_List, TOP_sh4add_r_r_r)) {
    new_topcode = TOP_sh4add_r_r_r;
  }
  else return FALSE;

  if (TN_is_symbol(l2_tn1)) return FALSE;
  if (TN_has_value(l2_tn1)) new_topcode = TOP_opnd_immediate_variant(new_topcode, 1, TN_value(l2_tn1));
  if (new_topcode == TOP_UNDEFINED) return FALSE;

  /* Replace the current instruction. */
  OP *new_op;
  new_op = Mk_OP(new_topcode, l1_tn0, l3_tn0, l2_tn1);
  OP_srcpos(new_op) = OP_srcpos(op);
  if (EBO_in_loop) EBO_Set_OP_omega (new_op, l3_tninfo0, l2_tninfo1);
  if (!EBO_Verify_Op(new_op)) return FALSE;
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
  if (OP_code(op) != TOP_targ_addcg_b_r_r_b_r) return FALSE;

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
      EBO_Exp_COPY(NULL, false, false, OP_result(op, 0), OP_opnd(op, op_idx), &ops);
      if (EBO_in_loop) 
	EBO_OPS_omega (&ops, OP_opnd(op,op_idx), opnd_tninfo[op_idx]);
    }
    TN *tnc = Gen_Literal_TN (0, 4);
    Expand_Immediate (OP_result(op, 1), tnc, 0, &ops);
    OP_srcpos(OPS_first(&ops)) = OP_srcpos(op);
    if (!EBO_Verify_Ops(&ops)) return FALSE;
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
  if (opcode != TOP_add_r_r_r) return FALSE;

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

  if ((OP_code(l2_op0) != TOP_mullu_r_r_r &&
       OP_code(l2_op0) != TOP_mulhs_r_r_r) ||
      (OP_code(l2_op1) != TOP_mullu_r_r_r &&
       OP_code(l2_op1) != TOP_mulhs_r_r_r) ||
      (OP_code(l2_op0) == TOP_mulhs_r_r_r &&
       OP_code(l2_op1) != TOP_mullu_r_r_r) ||
      (OP_code(l2_op0) == TOP_mullu_r_r_r &&
       OP_code(l2_op1) != TOP_mulhs_r_r_r)
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
  if (!EBO_Verify_Op(new_op)) return FALSE;
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

    if (ST_on_stack(st) &&
        ((ST_sclass(st) == SCLASS_AUTO) ||
         (EBO_in_peep && (ST_sclass(st) == SCLASS_FORMAL)))) {
      ST *base_st;
      INT64 base_ofst;
      INT64 val = TN_offset(tn);
      Base_Symbol_And_Offset (st, &base_st, &base_ofst);
      val += base_ofst;
      if (val == 0) {
        OPS ops = OPS_EMPTY;
        EBO_Exp_COPY(NULL, false, false, OP_result(op, 0), opnd_tn[1], &ops);

        OP_srcpos(OPS_first(&ops)) = OP_srcpos(op);
	if (EBO_in_loop) 
	  EBO_OPS_omega (&ops, opnd_tn[1], opnd_tninfo[1]);
	if (!EBO_Verify_Ops(&ops)) return FALSE;
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

  if (opcode != TOP_mulh_i_r_r && 
      opcode != TOP_mulh_ii_r_r &&
      opcode != TOP_mulh_r_r_r &&
      opcode != TOP_mulhu_i_r_r && 
      opcode != TOP_mulhu_ii_r_r &&
      opcode != TOP_mulhu_r_r_r) 
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

  if (OP_code(l2_op) != TOP_shr_i_r_r &&
      OP_code(l2_op) != TOP_shru_i_r_r) return FALSE;

  TN *l2_tn0 = OP_opnd(l2_op, 0);
  TN *l2_tn1 = OP_opnd(l2_op, 1);
  if (!TN_has_value(l2_tn1) || TN_value(l2_tn1) != 16)
    return FALSE;

  EBO_TN_INFO *l2_tninfo0 = l2_opinfo->actual_opnd[0];
  if (!EBO_tn_available (bb, l2_tninfo0)) return FALSE;

  // Determine new opcode:
  TOP new_opcode = TOP_UNDEFINED;
  switch (opcode) {
  case TOP_mulh_i_r_r: 
    new_opcode = (OP_code(l2_op) == TOP_shr_i_r_r) ? TOP_mulhh_i_r_r : TOP_mulhhu_i_r_r;
    break;
  case TOP_mulh_ii_r_r: 
    new_opcode = (OP_code(l2_op) == TOP_shr_i_r_r) ? TOP_mulhh_ii_r_r : TOP_mulhhu_ii_r_r;
    break;
  case TOP_mulh_r_r_r: 
    new_opcode = (OP_code(l2_op) == TOP_shr_i_r_r) ? TOP_mulhh_r_r_r : TOP_mulhhu_r_r_r;
    break;
  case TOP_mulhu_i_r_r: 
    new_opcode = (OP_code(l2_op) == TOP_shru_i_r_r) ? TOP_mulhhu_i_r_r : TOP_UNDEFINED;
    break;
  case TOP_mulhu_ii_r_r: 
    new_opcode = (OP_code(l2_op) == TOP_shru_i_r_r) ? TOP_mulhhu_ii_r_r : TOP_UNDEFINED;
    break;
  case TOP_mulhu_r_r_r: 
    new_opcode = (OP_code(l2_op) == TOP_shru_i_r_r) ? TOP_mulhhu_r_r_r : TOP_UNDEFINED;
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
  if (!EBO_Verify_Op(new_op)) return FALSE;
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
  if (!EBO_Verify_Op(new_op)) return FALSE;
  BB_Insert_Op_After(bb, op, new_op);
  if (EBO_Trace_Optimization) 
    fprintf(TFile,"Convert %s to %s\n", TOP_Name(opcode), TOP_Name(new_opcode));
  return TRUE;
}

// [HK] ST231 specific strenght-reduction functions
/* =====================================================================
 *   Function: mul_32_sequence
 *
 *   Try to strength reduce the 32 x 32 multiplies.
 * =====================================================================
 */
static BOOL
mul32_sequence (
  OP *op,
  TN **opnd_tn,
  EBO_TN_INFO **opnd_tninfo
)
{
  TOP opcode = OP_code(op);

  if (!IS_MUL32(op))
    return FALSE;

  // Level 1 data:
  BB *bb = OP_bb(op);
  TN *tn0 = OP_opnd(op, 0);
  TN *tn1 = OP_opnd(op, 1);
  TN *res = OP_result(op, 0);

  TN *new_tn0 = tn0, *new_tn1 = tn1;

  BOOL reduce_tn0 = FALSE;
  BOOL reduce_tn1 = FALSE;
  EBO_TN_INFO *tninfo0 = opnd_tninfo[0];
  EBO_TN_INFO *tninfo1 = opnd_tninfo[1];
  BITS_POS hilo0;
  BITS_POS hilo1;
  SIGNDNESS signed0;
  SIGNDNESS signed1;
  
  // check if operand 0 is 16 bits
  if (Is_16_Bits(opnd_tninfo[0], bb, &new_tn0, &tninfo0, &hilo0, &signed0) )
      reduce_tn0 = TRUE;

  // check if operand 1 is 16 bits
  if (Is_16_Bits(opnd_tninfo[1], bb, &new_tn1, &tninfo1, &hilo1, &signed1) )
      reduce_tn1 = TRUE;

  if (!reduce_tn0) {
      new_tn0 = tn0;
      hilo0 = TN_32_BITS;
      signed0 = SIGN_UNKNOWN;
  }
  
  if (!reduce_tn1) {
      new_tn1 = tn1;
      hilo1 = TN_32_BITS;
      signed1 = SIGN_UNKNOWN;
  }

  // Continue if any of the operands is being strength reduced
  if (!reduce_tn0 && !reduce_tn1) {
    if (EBO_Trace_Optimization) 
      fprintf(TFile, "No stength reduction for 32x32 mul\n");
    return FALSE;
  }

  // Before looking for a possible strength reduction, check if
  // the new_tn1 is a 32 bit quantity. Since there are no TOP
  // codes with 32 bit operand in the second position, try to
  // swap the operands.
  // Also for hi x lo combination
  if (((hilo0 != TN_32_BITS && hilo1 == TN_32_BITS) ||
      (hilo0 == TN_HI_16 && hilo1 == TN_LO_16)) && 
      !TN_is_constant(tn1)) {
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

  // Now we can choose an appropriate opcode
  TOP new_opcode = get_mul_opcode(signed0, hilo0,
				  signed1, hilo1);

  // Convert to an immediate form if new_tn1 is an immediate
  if (TN_is_symbol(new_tn1)) return FALSE;
  if (new_opcode != TOP_UNDEFINED && TN_has_value(new_tn1))
    new_opcode = TOP_opnd_immediate_variant(new_opcode, 1, TN_value(new_tn1));

  if (new_opcode == TOP_UNDEFINED) {
    if (EBO_Trace_Optimization)
      fprintf(TFile, "No opcode for stength reduction\n");
    return FALSE;
  }

  // case of no change
  if (new_opcode == opcode && new_tn0 == tn0 && new_tn1 == tn1)
    return FALSE;

  // [CG]: Check redefinitions
  if (!EBO_tn_available (bb, tninfo0) ||
      !EBO_tn_available (bb, tninfo1)) {
    if (EBO_Trace_Optimization) 
      fprintf(TFile,"mul32 strength reduction stopped due to redefinition\n");
    return FALSE;
  }

  //
  // Now, we have :
  //
  //    new_opcode
  //    res
  //    new_tn0
  //    new_tn1
  //
  // Make a res = new_opcode new_tn0, new_tn1, and replace the
  // current instruction:
  //
  OP *new_op;
  new_op = Mk_OP(new_opcode, res, new_tn0, new_tn1);
  OP_srcpos(new_op) = OP_srcpos(op);
  if (EBO_in_loop) EBO_Set_OP_omega (new_op, tninfo0, tninfo1);
  if (!EBO_Verify_Op(new_op)) return FALSE;
  BB_Insert_Op_After(bb, op, new_op);
  if (EBO_Trace_Optimization) 
    fprintf(TFile,"Convert mul32 sequence\n");
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

  if (opcode != TOP_shl_i_r_r)
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

  if (OP_code(l2_op) != TOP_mulh_i_r_r &&
      OP_code(l2_op) != TOP_mulh_ii_r_r &&
      OP_code(l2_op) != TOP_mulh_r_r_r &&
      OP_code(l2_op) != TOP_mulhu_i_r_r &&
      OP_code(l2_op) != TOP_mulhu_ii_r_r &&
      OP_code(l2_op) != TOP_mulhu_r_r_r) return FALSE;

  EBO_TN_INFO *l2_tninfo0 = l2_opinfo->actual_opnd[0];
  EBO_TN_INFO *l2_tninfo1 = l2_opinfo->actual_opnd[1];
  if (!EBO_tn_available (bb, l2_tninfo0) ||
      !EBO_tn_available (bb, l2_tninfo1)) return FALSE;

  TN *l2_tn0 = OP_opnd(l2_op,0);
  TN *l2_tn1 = OP_opnd(l2_op,1);

  // Determine new opcode:
  TOP new_opcode;
  switch (OP_code(l2_op)) {
  case TOP_mulh_i_r_r: 
  case TOP_mulhu_i_r_r: 
    new_opcode = TOP_mulhs_i_r_r; break;
  case TOP_mulh_ii_r_r: 
  case TOP_mulhu_ii_r_r: 
    new_opcode = TOP_mulhs_ii_r_r; break;
  case TOP_mulh_r_r_r: 
  case TOP_mulhu_r_r_r: 
    new_opcode = TOP_mulhs_r_r_r; break;
  default:
    FmtAssert(FALSE, (" wrong opcode %s\n", TOP_Name(OP_code(l2_op))));
  }

  if (! ISA_SUBSET_LIST_Member (ISA_SUBSET_List, new_opcode))
    return FALSE;

  // Replace the current instruction:
  OP *new_op;
  new_op = Mk_OP(new_opcode, l1_res, l2_tn0, l2_tn1);
  OP_srcpos(new_op) = OP_srcpos(op);
  if (EBO_in_loop) EBO_Set_OP_omega (new_op, l2_tninfo0, l2_tninfo1);
  if (!EBO_Verify_Op(new_op)) return FALSE;
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
  new_opcode = TOP_is_unsign(opcode) ? TOP_zxth_r_r : TOP_sxth_r_r;

  // Replace the current instruction:
  OP *new_op;
  new_op = Mk_OP(new_opcode, l1_res, l2_tn0);
  OP_srcpos(new_op) = OP_srcpos(op);
  if (EBO_in_loop) EBO_Set_OP_omega (new_op, l2_tninfo0);
  if (!EBO_Verify_Op(new_op)) return FALSE;
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
    // TOP_opnd_swapped_variant does not exist anymore
    TOP swapped_opcode = OP_opnd_swapped_variant(op, 0, 1);
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
  if (!EBO_Verify_Op(new_op)) return FALSE;
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
  if (OP_code(op) == TOP_add_r_r_r) {
    return (add_mul_sequence (op, opnd_tn, opnd_tninfo));
  }

  if (IS_MUL32(op)) {
    return (mul32_sequence (op, opnd_tn, opnd_tninfo));
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

#if 0
// [SC] Commented-out because unused, and needs updating for wide branch regs.
static BOOL
op_match_integer_branch_copy(OP *op, EBO_TN_INFO **opnd_tninfo, TN **match_tn, EBO_TN_INFO **match_tninfo)
{
  TOP top = OP_code(op);
  EBO_TN_INFO *tninfo;
  TN *tn;
  
  if (top == TOP_mtb_r_b) { // [SC] mtb checked
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

// [SC] Commented-out because unused, and needs updating for wide branch regs.
static BOOL
op_match_branch_integer_copy(OP *op, EBO_TN_INFO **opnd_tninfo, TN **match_tn, EBO_TN_INFO **match_tninfo)
{
  TOP top = OP_code(op);
  EBO_TN_INFO *tninfo;
  TN *tn;

  if (top == TOP_mfb_b_r) { // [SC] mfb checked.
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

// [SC] Commented-out because unused, and needs updating for wide branch regs.
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
#endif // commented-out

static BOOL
op_match_lnot(OP *op, 
	     EBO_TN_INFO **opnd_tninfo, 
	     TN **op0_tn, 
	     EBO_TN_INFO **op0_tninfo)
{
  TOP top = OP_code(op);
  TN *opnd;

  if (top == TOP_cmpeq_r_r_b || top == TOP_cmpeq_r_r_r) {
    if (OP_opnd(op, 0) == Zero_TN) {
      *op0_tn = OP_opnd(op, 1);
      *op0_tninfo = opnd_tninfo[1];
      return TRUE;
    } else if (OP_opnd(op, 1) == Zero_TN) {
      *op0_tn = OP_opnd(op, 0);
      *op0_tninfo = opnd_tninfo[0];
      return TRUE;
    }
  } else if ((top == TOP_norl_r_r_r || top == TOP_norl_r_r_b
	      || top == TOP_norl_b_b_b || top == TOP_nandl_r_r_r
	      || top == TOP_nandl_r_r_b || top == TOP_nandl_b_b_b)
	     && OP_opnd(op, 0) == OP_opnd(op, 1)) {
    *op0_tn = OP_opnd (op, 0);
    *op0_tninfo = opnd_tninfo[0];
    return TRUE;
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

  if (top == TOP_cmpne_r_r_b || top == TOP_cmpne_r_r_r) {
    if (OP_opnd(op, 0) == Zero_TN) {
      *op0_tn = OP_opnd(op, 1);
      *op0_tninfo = opnd_tninfo[1];
      return TRUE;
    } else if (OP_opnd(op, 1) == Zero_TN) {
      *op0_tn = OP_opnd(op, 0);
      *op0_tninfo = opnd_tninfo[0];
      return TRUE;
    }
  } else if (top == TOP_convib_r_b || top == TOP_convbi_b_r) {
    *op0_tn = OP_opnd(op, 0);
    *op0_tninfo = opnd_tninfo[0];
    return TRUE;
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
  
  // Skip predicated definitions
  if (OP_cond_def(def_opinfo->in_op)) return FALSE;

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
  INT same_res_opnd[OP_MAX_FIXED_OPNDS];

  /* ASM are not subject to this and must not, as the ASM_OP_Map must be preserved. */
  if (OP_code(op) == TOP_asm) return FALSE;

  if (num_opnds > OP_MAX_FIXED_OPNDS) return FALSE;

  // Don't propagate into mul until mul32x32 are reduced.
  if (EBO_in_pre && IS_MUL32_PART(op)) return FALSE;

  // [JV] Don't know if there is a better solution to have information from operand idx
  // instead of result idx.
  for (i = 0; i < num_opnds; i++) {
    same_res_opnd[i] = -1;
  }

  for (i = 0; i < OP_results(op); i++) {
    INT same_opnd = -1;
    if((same_opnd = OP_same_res(op,i)) != -1) {
      if (EBO_Trace_Optimization) {
	fprintf(TFile,"Operand %d is same as result %d in OP:\n", i, same_opnd);
	Print_OP_No_SrcLine(op);
      }

      same_res_opnd[same_opnd] = i;
    }
  }


  for (i = 0; i < num_opnds; i++) {
    EBO_TN_INFO *tninfo, *new_tninfo;
    TN *new_tn;
    int use_bits;
    tninfo = opnd_tninfo[i];

    if (tninfo == NULL ||
	!use_bit_width(op, i, &use_bits) ||
	!find_equivalent_tn(op, tninfo, use_bits, &new_tn, &new_tninfo) ||
	!TN_is_register(new_tn) ||
	TN_register_class(new_tn) != TN_register_class(OP_opnd(op, i)) ||
	TN_is_dedicated(OP_opnd(op, i)) ||  /* Don't replace dedicated TNs. */
	same_res_opnd[i] != -1) { /* Don't replace TNs if there is a result identical to operand. */
      new_opnd_tn[i] = OP_opnd(op, i);
    } else {
      new_opnd_tn[i] = new_tn;
      replaced++;
    }
  }
  if (replaced == 0) return FALSE;

  /* Replace the current instruction operands. */
  OP *new_op;
  new_op = Dup_OP (op);
  OP_srcpos(new_op) = OP_srcpos(op);
  if (OP_memory(op)) Copy_WN_For_Memory_OP (new_op, op);
  if (EBO_in_loop) CG_LOOP_Init_Op(new_op);
  for (i = 0; i < num_opnds; i++) {
    Set_OP_opnd(new_op, i, new_opnd_tn[i]);
    if (EBO_in_loop) Set_OP_omega (new_op, i, OP_omega(op,i));
  }

  if (!EBO_Verify_Op(new_op)) return FALSE;
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
    //[HK] only used for integer cmp
//   TOP top = OP_code(op);
  
//   if (!TOP_is_cmp(top)) return FALSE;
  if (!OP_icmp(op)) return FALSE;

  *op0_tn = OP_opnd(op, 0);
  *op0_tninfo = opnd_tninfo[0];
  *op1_tn = OP_opnd(op, 1);
  *op1_tninfo = opnd_tninfo[1];
  return TRUE;
}

static BOOL
op_match_min(OP *op, 
		 EBO_TN_INFO **opnd_tninfo, 
		 TN **op0_tn, 
		 EBO_TN_INFO **op0_tninfo, 
		 TN **op1_tn, 
		 EBO_TN_INFO **op1_tninfo)
{
  if (!OP_imin(op)) return FALSE;

  *op0_tn = OP_opnd(op, 0);
  *op0_tninfo = opnd_tninfo[0];
  *op1_tn = OP_opnd(op, 1);
  *op1_tninfo = opnd_tninfo[1];
  return TRUE;
}

static BOOL
op_match_max(OP *op, 
		 EBO_TN_INFO **opnd_tninfo, 
		 TN **op0_tn, 
		 EBO_TN_INFO **op0_tninfo, 
		 TN **op1_tn, 
		 EBO_TN_INFO **op1_tninfo)
{
  if (!OP_imax(op)) return FALSE;

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
 * 
 * [HK] 20070918 Augment the function to catch the
 * following cases:
 * (select (cmp x c2) (max/min x,c1) c2)
 * and generate:
 * (min (max x, c1) c2) depending on the value of c1 and c2
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
  TN *min_tn0, *min_tn1, *max_tn0, *max_tn1;
  EBO_TN_INFO *min_tninfo0, *min_tninfo1, *max_tninfo0, *max_tninfo1;
  if (!find_def_opinfo(cond_tninfo, &cond_opinfo)) return FALSE;
  if (!op_match_compare(cond_opinfo->in_op, 
			cond_opinfo->actual_opnd, 
			&lhs_tn, &lhs_tninfo, &rhs_tn, &rhs_tninfo))
    return FALSE;
  if (lhs_tn == true_tn && rhs_tn == false_tn) {
    inverted = FALSE;
  } else if (lhs_tn == false_tn && rhs_tn == true_tn) {
    inverted = TRUE;
  } else {
    // First, normalize the code in the form
    // select (cmp var c1), min/max(var, c2), c1
    inverted = FALSE;
    TN *var_tn, *c1_tn, *c2_tn, *canon_true, *canon_false;
    EBO_TN_INFO *canon_true_tninfo, *canon_false_tninfo;

    if (TN_Has_Value(lhs_tn)) {
      c1_tn = lhs_tn;
      var_tn = rhs_tn;
      inverted = !inverted;
    }
    else if (TN_Has_Value(rhs_tn)) {
      c1_tn = rhs_tn;
      var_tn = lhs_tn;
    }
    else
      return FALSE;

    if ( c1_tn != true_tn && c1_tn != false_tn)
      return FALSE;

    if (c1_tn == true_tn) {
      TN  *tmp_tn;
      EBO_TN_INFO *tmp_tninfo;
      tmp_tn = false_tn;
      false_tn = true_tn;
      true_tn = tmp_tn;
      tmp_tninfo = false_tninfo;
      false_tninfo = true_tninfo;
      true_tninfo = tmp_tninfo;
      inverted = !inverted;
    }

    EBO_OP_INFO *canon_true_opinfo;
    TN *true_tn0, *true_tn1;
    EBO_TN_INFO *true_tninfo0, *true_tninfo1;
    if (!find_def_opinfo(true_tninfo, &canon_true_opinfo)) return FALSE;
    if (op_match_min(canon_true_opinfo->in_op,
             canon_true_opinfo->actual_opnd,
             &true_tn0, &true_tninfo0, &true_tn1, &true_tninfo1)
	&& TN_Has_Value(true_tn1)
	&& TN_Value(true_tn1) >= TN_Value(c1_tn)
	&& true_tn0 == var_tn)
      c2_tn = true_tn1;
    else if (op_match_max(canon_true_opinfo->in_op,
              canon_true_opinfo->actual_opnd,
              &true_tn0, &true_tninfo0, &true_tn1, &true_tninfo1)
	     && TN_Has_Value(true_tn1)
	     && TN_Value(true_tn1) <= TN_Value(c1_tn)
	     && true_tn0 == var_tn)
      c2_tn = true_tn1;
    else
      return FALSE;


    /* Beware that we may also generate a sequence min(min(x,c2),c1) or max(max(x,c2),c1) */
  }
  
  TN *new_tn0 = NULL, *new_tn1 = NULL;
  EBO_TN_INFO *new_tninfo0 = NULL, *new_tninfo1 = NULL;


  // [HK] must first check if the comparisons are not FP 
  // (the min/max sequence is invalid in FP arithmetic)
  if (OP_fcmp(cond_opinfo->in_op))
      return FALSE;

  VARIANT variant = OP_cmp_variant(cond_opinfo->in_op);

  switch (variant) {
    // Special case of move
  case V_CMP_EQ:
    new_top = TOP_mov_r_r;
    new_tn0 = false_tn;
    new_tninfo0 = false_tninfo;
    break;
  case V_CMP_NE:
    new_top = TOP_mov_r_r;
    new_tn0 = true_tn;
    new_tninfo0 = true_tninfo;
    break;
    
    // Min/max
  case V_CMP_GT:
  case V_CMP_GE:
    new_top = inverted ? TOP_min_r_r_r: TOP_max_r_r_r;
    new_tn0 = true_tn;
    new_tninfo0 = true_tninfo;
    new_tn1 = false_tn;
    new_tninfo1 = false_tninfo;
    break;
  case V_CMP_LT:
  case V_CMP_LE:
    new_top = inverted ? TOP_max_r_r_r: TOP_min_r_r_r;
    new_tn0 = true_tn;
    new_tninfo0 = true_tninfo;
    new_tn1 = false_tn;
    new_tninfo1 = false_tninfo;
    break;
  case V_CMP_GTU:
  case V_CMP_GEU:
    new_top = inverted ? TOP_minu_r_r_r: TOP_maxu_r_r_r;
    new_tn0 = true_tn;
    new_tninfo0 = true_tninfo;
    new_tn1 = false_tn;
    new_tninfo1 = false_tninfo;
    break;
  case V_CMP_LTU:
  case V_CMP_LEU:
    new_top = inverted ? TOP_maxu_r_r_r: TOP_minu_r_r_r;
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
  if (new_top == TOP_mov_r_r) Set_OP_copy(new_op);
  OP_srcpos(new_op) = OP_srcpos(op);
  if (EBO_in_loop) EBO_Set_OP_omega (new_op, new_tninfo0, new_tninfo1);
  if (!EBO_Verify_Op(new_op)) return FALSE;
  BB_Insert_Op_After(bb, op, new_op);
  if (EBO_Trace_Optimization) 
    fprintf(TFile,"Convert slct/cmp to %s\n", TOP_Name(new_top));
  return TRUE;

}


/*
 * minmax_sats_sequence
 *
 * Detect the min(x, 32767)/max(x, -32768) sequences
 * and generates:
 * (sats x )
 */
static BOOL
minmax_sats_sequence (OP *l1_op, TN **opnd_tn, EBO_TN_INFO **opnd_tninfo)
{
  OP *l2_op;
  TN *opnd1, *opnd2, *result, *opnd_sat;
  INT shiftcount;
  INT64 val1, val2;
  BOOL min = FALSE, max = FALSE;
  TN *minmax_tn0, *minmax_tn1;
  EBO_TN_INFO *minmax_tninfo, *minmax_tninfo0, *minmax_tninfo1;
  EBO_OP_INFO *minmax_opinfo;

  // Check if the target has a sats instruction
  if (!ISA_SUBSET_LIST_Member (ISA_SUBSET_List, TOP_sats_r_r))
    return FALSE;

  TOP opcode = OP_code(l1_op);

  // check if l1_op is a min or a max
  if (op_match_min(l1_op, 
		   opnd_tninfo, 
		   &minmax_tn0, &minmax_tninfo0, &minmax_tn1, &minmax_tninfo1))
    min = TRUE;
  else if (op_match_max(l1_op, 
			opnd_tninfo, 
			&minmax_tn0, &minmax_tninfo0, &minmax_tn1, &minmax_tninfo1))
    max = TRUE;
  else
    return FALSE;

  opnd1 = OP_Opnd1(l1_op);
  opnd2 = OP_Opnd2(l1_op);

  // check if one of the operand of l1_op is a literal
  if (TN_Has_Value(opnd1)){
    val1 = TN_Value(opnd1);
    minmax_tninfo = minmax_tninfo1;
  }
  else if (TN_Has_Value(opnd2)){
    val1 = TN_Value(opnd2);
    minmax_tninfo = minmax_tninfo0;
  }
  else
    return FALSE;

  // check that l1_op is either max(x, -32768) or min(x, 32767)
  if (min && val1 != 32767 
      || max && val1 != -32768)
    return FALSE;

  if (!find_def_opinfo(minmax_tninfo, &minmax_opinfo)) 
    return FALSE;
 
  l2_op = minmax_opinfo->in_op;

  if (!l2_op) return FALSE;
  
  opnd1 = OP_Opnd1(l2_op);
  opnd2 = OP_Opnd2(l2_op);

  // check if we have a min / max sequence
  if (min && OP_imax(l2_op)){
    if (TN_Has_Value(opnd1)){
      opnd_sat = opnd2;
      val2 = TN_Value(opnd1);
    } else if (TN_Has_Value(opnd2)){
      opnd_sat = opnd1;
      val2 = TN_Value(opnd2);
    }
    if (val2 != -32768)
      return FALSE;
  // check if we have a max / min sequence
  } else if (max && OP_imin(l2_op)){
    if (TN_Has_Value(opnd1)){
      opnd_sat = opnd2;
      val2 = TN_Value(opnd1);
    } else if (TN_Has_Value(opnd2)){
      opnd_sat = opnd1;
      val2 = TN_Value(opnd2);
    }
    if (val2 != 32767)
      return FALSE;
  } else
    return FALSE;


  // if we get here, we have the adequate min/max sequence 
  // to produce a sats

  result = OP_result (l1_op, 0);  
  BB *bb = OP_bb(l1_op);
  if (minmax_tninfo != NULL && !EBO_tn_available (bb, minmax_tninfo)) {
    return FALSE;
  }
  OP *new_op;
  TOP new_top = TOP_sats_r_r;
  new_op = Mk_OP(new_top , result, opnd_sat);
  OP_srcpos(new_op) = OP_srcpos(l1_op);
  if (EBO_in_loop) EBO_Set_OP_omega (new_op, minmax_tninfo);
  if (!EBO_Verify_Op(new_op)) return FALSE;
  BB_Insert_Op_After(bb, l1_op, new_op);
  if (EBO_Trace_Optimization) 
    fprintf(TFile,"Convert min/max to %s\n", TOP_Name(new_top));
  return TRUE;
}

static BOOL
match_adds_X_X (EBO_TN_INFO *tninfo, BB *bb, TN **X_tn, EBO_TN_INFO **X_tninfo)
{
  // Is this tn defined by an adds(X,X) instruction,
  // where tn X is available in bb.
  // If yes, set *X_tn and return TRUE,
  // else return FALSE.

  if (tninfo == NULL ||
      tninfo->in_op == NULL)
    return FALSE;

  TOP opcode =  OP_code(tninfo->in_op);

  if ((opcode != TOP_adds_r_r_r && 
       opcode != TOP_shls_r_r_r &&
       opcode != TOP_shls_i_r_r))
    return FALSE;

  EBO_OP_INFO *opinfo = locate_opinfo_entry(tninfo);
  if (opinfo == NULL || opinfo->in_op == NULL)
    return FALSE;

  OP *l2_op = opinfo->in_op;
  TN *l2_tn0 = OP_opnd (l2_op, 0);
  TN *l2_tn1 = OP_opnd (l2_op, 1);
  EBO_TN_INFO *l2_tninfo = opinfo->actual_opnd[0];

  if ((opcode == TOP_adds_r_r_r && l2_tn0 == l2_tn1) ||
      ((opcode == TOP_shls_r_r_r || opcode == TOP_shls_i_r_r) && 
       TN_Is_Constant(l2_tn1) && TN_Value(l2_tn1) == 1) &&
      EBO_tn_available (bb, l2_tninfo)) {
    *X_tn = l2_tn0; *X_tninfo = l2_tninfo;
    return TRUE;
  }

  return FALSE;
}
  
/* =====================================================================
 *   Function: shadds_sequence
 *
 *   Apply the following transformation:
 *
 *           adds (Y, adds (X, X)) => sh1adds (Y, X)
 *           adds (adds (X, X), Y) => sh1adds (Y, X)
 *           subs (Y, adds (X, X)) => sh1subs (Y, X)
 *
 * =====================================================================
 */
static BOOL
shadds_sequence (OP *op, TN **opnd_tn, EBO_TN_INFO **opnd_tninfo)
{
  TOP opcode = OP_code(op);

  TOP new_opcode = ((opcode == TOP_adds_r_r_r) ? TOP_sh1adds_r_r_r :
		    (opcode == TOP_subs_r_r_r) ? TOP_sh1subs_r_r_r :
		    TOP_UNDEFINED);

  if (! Enable_Sh1AddS || new_opcode == TOP_UNDEFINED) return FALSE;

  BB *bb = OP_bb(op);
  TN *X_tn, *Y_tn;
  EBO_TN_INFO *X_tninfo, *Y_tninfo;

  if (opcode == TOP_adds_r_r_r && match_adds_X_X (opnd_tninfo[0], bb, &X_tn, &X_tninfo)) {
    Y_tn = OP_opnd(op, 1); Y_tninfo = opnd_tninfo[1];
  } else if (match_adds_X_X (opnd_tninfo[1], bb, &X_tn, &X_tninfo)) {
    Y_tn = OP_opnd(op, 0); Y_tninfo = opnd_tninfo[0];
  } else {
    return FALSE;
  }
  
  OP *new_op = Mk_OP(new_opcode, OP_result(op, 0), Y_tn, X_tn);
  OP_srcpos(new_op) = OP_srcpos(op);
  if (EBO_in_loop) EBO_Set_OP_omega (new_op, X_tninfo, Y_tninfo);
  if (!EBO_Verify_Op(new_op)) return FALSE;
  BB_Insert_Op_After(bb, op, new_op);
  if (EBO_Trace_Optimization) {
    fprintf(TFile,"Convert adds/subs(Y,adds(X,X)) to sh1adds/sh1subs(X,Y)\n");
  }
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

  // [HK] must first check if the comparisons are not FP 
  // (the min/max sequence is invalid in FP arithmetic)
  if (OP_fcmp(cond_opinfo->in_op))
      return FALSE;

  VARIANT variant = OP_cmp_variant(cond_opinfo->in_op);
  switch (variant) {
  case V_CMP_GT:
  case V_CMP_GE:
    new_top = inverted ? TOP_max_r_r_r: TOP_min_r_r_r;
    break;
  case V_CMP_LT:
  case V_CMP_LE:
    new_top = inverted ? TOP_min_r_r_r: TOP_max_r_r_r;
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
  if (!EBO_Verify_Op(new_op)) return FALSE;
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
  new_top = new_variant == V_COND_TRUE ? TOP_targ_slct_r_r_b_r: TOP_slctf_r_r_b_r;
  if (TN_is_symbol(OP_opnd(op, 2))) return FALSE;
  if (TN_has_value(OP_opnd(op, 2)))
    new_top = TOP_opnd_immediate_variant(new_top, 2, TN_value(OP_opnd(op, 2)));
  if (new_top == TOP_UNDEFINED) return FALSE;
  
  OPS ops = OPS_EMPTY;
  if (TN_register_class(src_tn) != TN_register_class(OP_opnd(op, 0))) {
    TN *tmp = Build_RCLASS_TN (TN_register_class(OP_opnd(op, 0)));
    // [SC] This is a boolean copy.
    if (TN_register_class(src_tn) == ISA_REGISTER_CLASS_branch) {
      Expand_Bool_To_Int (tmp, src_tn, MTYPE_I4, &ops);
    } else if (TN_register_class(tmp) == ISA_REGISTER_CLASS_branch) {
      Expand_Int_To_Bool (tmp, src_tn, &ops);
    } else {
      Exp_COPY(tmp, src_tn, &ops);
    }
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
 * - Convert select of 0, 1 to convbi.
 */
static BOOL
select_move_sequence(OP *op, TN **opnd_tn, EBO_TN_INFO **opnd_tninfo)
{
  TOP opcode = OP_code(op);
  TOP new_opcode;
  VARIANT variant;

  if (!TOP_is_select(opcode)) return FALSE;

  variant = TOP_cond_variant(opcode);
  TN *selector = (opnd_tninfo[0]->replacement_tn
		  ? opnd_tninfo[0]->replacement_tn
		  : opnd_tn[0]);

  if (TN_Has_Value(selector)) {
    INT idx;
    if (TN_Value(selector) == 0) idx = variant == V_COND_FALSE ? 1 : 2;
    else idx = variant == V_COND_FALSE ? 2 : 1;
    new_opcode = TOP_mov_r_r;
    if (TN_is_symbol(opnd_tn[idx])) return FALSE;
    if (TN_has_value(opnd_tn[idx])) 
      new_opcode = TOP_opnd_immediate_variant(new_opcode, 0, TN_value(opnd_tn[idx]));
    if (new_opcode == TOP_UNDEFINED) return FALSE;
    OP *new_op = Mk_OP(new_opcode, OP_result(op, 0), opnd_tn[idx]);
    if (new_opcode == TOP_mov_r_r) Set_OP_copy(new_op);
    OP_srcpos(new_op) = OP_srcpos(op);
    if (EBO_in_loop) EBO_Set_OP_omega (new_op, opnd_tninfo[idx]);
    if (!EBO_Verify_Op(new_op)) return FALSE;
    BB_Insert_Op_After(OP_bb(op), op, new_op);
    if (EBO_Trace_Optimization) 
      fprintf(TFile,"Convert slct of constant\n");
    return TRUE;
  } else if (TN_Has_Value(opnd_tn[1]) && TN_Has_Value(opnd_tn[2])) {
    if ((variant == V_COND_FALSE && TN_Value(opnd_tn[1]) == 0 && TN_Value(opnd_tn[2]) == 1) ||
	(variant == V_COND_TRUE && TN_Value(opnd_tn[1]) == 1 && TN_Value(opnd_tn[2]) == 0)) {
      new_opcode = TOP_convbi_b_r;
      if (TN_is_symbol(opnd_tn[0])) return FALSE;
      if (TN_has_value(opnd_tn[0])) 
	new_opcode = TOP_opnd_immediate_variant(new_opcode, 0, TN_value(opnd_tn[0]));
      if (new_opcode == TOP_UNDEFINED) return FALSE;
      OP *new_op = Mk_OP(new_opcode, OP_result(op, 0), opnd_tn[0]);
      OP_srcpos(new_op) = OP_srcpos(op);
      if (EBO_in_loop) EBO_Set_OP_omega (new_op, opnd_tninfo[0]);
      if (!EBO_Verify_Op(new_op)) return FALSE;
      BB_Insert_Op_After(OP_bb(op), op, new_op);
      if (EBO_Trace_Optimization) 
	fprintf(TFile,"Convert slct to %s\n", TOP_Name(new_opcode));
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
 * into (convbi (op_b u v)).
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

  // FdF 20050914: Special treatment in the following case:
  // 	cond_op:cond_tn = convib x_tn
  // 	def_op:	true_tn = x_tn
  // 	op:	res_tn = slct cond_tn ? true_tn : 0
  // The correct transformation is
  // 	res_tn = true_tn

  OP *new_op;
  if (OP_copy(def_op))
    new_op = Mk_OP(TOP_mov_r_r, OP_result(op,0), true_tn);
  else
    new_op = Mk_OP(TOP_convbi_b_r, OP_result(op,0), cond_tn);
  OP_srcpos(new_op) = OP_srcpos(op);
  if (EBO_in_loop) EBO_Set_OP_omega (new_op, cond_tninfo);
  if (!EBO_Verify_Op(new_op)) return FALSE;
  BB_Insert_Op_After(bb, op, new_op);
  if (EBO_Trace_Optimization)
    if (OP_copy(def_op))
      fprintf(TFile,"Convert select of cmp into mov_r\n");
    else
      fprintf(TFile,"Convert select of cmp into convbi\n");
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
  OP_srcpos(OPS_last(&ops)) = OP_srcpos(op);
  if (EBO_in_loop) EBO_OPS_omega (&ops, src_tn, src_tninfo);
  if (!EBO_Verify_Ops(&ops)) return FALSE;
  BB_Insert_Ops_After(bb, op, &ops);
  if (EBO_Trace_Optimization) 
    fprintf(TFile,"Convert select/select into move\n");
  return TRUE;
}

/*
 * cmp_move_sequence
 *
 * - Convert a cmp to move
 *   (cmp_b x 0) -> convib
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

  VARIANT variant = OP_cmp_variant(op);
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
  // If the result is a branch, the convib will normalize it
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
  if (TN_register_class(OP_result(op, 0)) == ISA_REGISTER_CLASS_branch) {
    Expand_Int_To_Bool (OP_result(op, 0), src_tn, &ops);
  } else {
    Exp_COPY(OP_result(op, 0), src_tn, &ops);
  }

  OP_srcpos(OPS_last(&ops)) = OP_srcpos(op);
  if (EBO_in_loop) EBO_OPS_omega (&ops, src_tn, src_tninfo);
  if (!EBO_Verify_Ops(&ops)) return FALSE;
  BB_Insert_Ops_After(bb, op, &ops);
  if (EBO_Trace_Optimization) 
    fprintf(TFile,"Convert cmp into move\n");
  return TRUE;
}

static BOOL
cmp_subsat_to_zero (OP *op, TN **opnd_tn, EBO_TN_INFO **opnd_tninfo)
{
  TOP opcode = OP_code(op);
  TN *tn1, *tn2, *tnr;
  INT op1_idx, op2_idx;
  OP *in_op1, *in_op2, *in_op;
  TOP in_opcode1, in_opcode2, in_opcode;
  EBO_TN_INFO *l1_tninfo1, *l1_tninfo2, *l1_tninfo;
  EBO_TN_INFO *l2_tninfo1, *l2_tninfo2;
  EBO_OP_INFO *l2_opinfo;
  VARIANT variant;
  OPS ops = OPS_EMPTY;
 

  /* Currently only handle integer compares. */
  if (!OP_icmp(op)) return FALSE;

  // Safer to check there is exactly one result.
  if (OP_results(op) != 1) return FALSE;
  tnr = OP_result(op,0);

  op1_idx = OP_find_opnd_use(op, OU_opnd1);
  op2_idx = OP_find_opnd_use(op, OU_opnd2);
  FmtAssert(op1_idx >= 0,("operand OU_opnd1 not defined for cmp TOP %s", TOP_Name(opcode)));
  FmtAssert(op2_idx >= 0,("operand OU_opnd2 not defined for cmp TOP %s", TOP_Name(opcode)));

  tn1 = opnd_tn[op1_idx];
  tn2 = opnd_tn[op2_idx];

  l1_tninfo1 = opnd_tninfo[op1_idx];
  l1_tninfo2 = opnd_tninfo[op2_idx];

  in_op1 = l1_tninfo1 ? l1_tninfo1->in_op : NULL;
  in_op2 = l1_tninfo2 ? l1_tninfo2->in_op : NULL;

  if (in_op1)
    in_opcode1 = OP_code(in_op1);
  if (in_op2)
    in_opcode2 = OP_code(in_op2);

  if ((in_op1 == NULL) && (in_op2 == NULL)) return FALSE;

  if (TN_Has_Value(tn1) 
      && (TN_Value(tn1) == 0)
      && in_op2 
      && (in_opcode2 == TOP_subs_r_r_r)){
    in_op = in_op2;
    in_opcode = in_opcode2;
    l1_tninfo = l1_tninfo2;
  }
  else if (TN_Has_Value(tn2) 
	   && (TN_Value(tn2) == 0)
	   && in_op1 
	   && (in_opcode1 == TOP_subs_r_r_r)){
    in_op = in_op1;
    in_opcode = in_opcode1;
    l1_tninfo = l1_tninfo1;
  }
  else {
    return FALSE;
  }
	   
  // get level 2 informations 
  op1_idx = OP_find_opnd_use(in_op, OU_opnd1);
  op2_idx = OP_find_opnd_use(in_op, OU_opnd2);

  tn1 = OP_opnd(in_op, op1_idx);
  tn2 = OP_opnd(in_op, op2_idx);

  l2_opinfo = locate_opinfo_entry(l1_tninfo);
  if (l2_opinfo == NULL) return FALSE;

  l2_tninfo1 = l2_opinfo->actual_opnd[op1_idx];
  l2_tninfo2 = l2_opinfo->actual_opnd[op2_idx];

  if (((l2_tninfo1 != NULL) && !EBO_tn_available (OP_bb(op), l2_tninfo1)) ||
      ((l2_tninfo2 != NULL) && !EBO_tn_available (OP_bb(op), l2_tninfo2))) 
    return FALSE;

  variant = OP_cmp_variant(op);
  switch (variant) {
  case V_CMP_NE: 
    Expand_Int_Not_Equal(tnr, tn1, tn2, MTYPE_I4, &ops);
    break;
  case V_CMP_LT: 
    Expand_Int_Less(tnr, tn1, tn2, MTYPE_I4, &ops);	
    break;
  case V_CMP_GT: 
    Expand_Int_Greater(tnr, tn1, tn2, MTYPE_I4, &ops);	
    break;
  case V_CMP_LE: 
    Expand_Int_Less_Equal(tnr, tn1, tn2, MTYPE_I4, &ops);	
    break;
  case V_CMP_GE: 
    Expand_Int_Greater_Equal(tnr, tn1, tn2, MTYPE_I4, &ops);	
    break;
  case V_CMP_EQ: 
    Expand_Int_Equal(tnr, tn1, tn2, MTYPE_I4, &ops);	
    break;
  default: return FALSE; break;
  }    
  

  if (OP_next(OPS_first(&ops)) != NULL) {
   /* What's the point in replacing one instruction with several? */
    return FALSE;
  }
  if (OP_has_predicate(op)) {
    EBO_OPS_predicate (OP_opnd(op, OP_PREDICATE_OPND), false, OP_cond_def(op), &ops);
  }

  /* No need to replace if same op, avoids infinite loops. */
  if (OPs_Are_Equivalent(op, OPS_first(&ops))) return FALSE;

  if (EBO_in_loop) EBO_OPS_omega (&ops, 
				  (OP_has_predicate(op)? OP_opnd(op,OP_PREDICATE_OPND):NULL),
				  (OP_has_predicate(op) ? opnd_tninfo[OP_PREDICATE_OPND] : NULL));
  OP_srcpos(OPS_first(&ops)) = OP_srcpos(op);

  BB_Insert_Ops(OP_bb(op), op, &ops, FALSE);

  if (EBO_Trace_Optimization) {
    #pragma mips_frequency_hint NEVER
      fprintf(TFile, "%sin BB:%d Simplify subs/compare sequence ",
	      EBO_trace_pfx, BB_id(OP_bb(op)));
      fprintf(TFile, "\n");
  }
  
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
  OP *new_op = Mk_OP(TOP_mov_r_r, OP_result(op, 0), opnd_tn[opnd_idx]);
  Set_OP_copy(new_op);
  OP_srcpos(new_op) = OP_srcpos(op);
  if (EBO_in_loop) EBO_Set_OP_omega (new_op, opnd_tninfo[opnd_idx]);
  if (!EBO_Verify_Op(new_op)) return FALSE;
  BB_Insert_Op_After(OP_bb(op), op, new_op);
  if (EBO_Trace_Optimization) 
    fprintf(TFile,"Convert ext to move\n");
  return TRUE;
}  

/*
 * copy_r_b_sequence
 *
 * Convert (mov_r_b (mov_b_r)) into a branch register copy.
 */
static BOOL
copy_r_b_sequence(OP *op, TN **opnd_tn, EBO_TN_INFO **opnd_tninfo)
{
  TOP top = OP_code(op);
  EBO_OP_INFO *def_opinfo;
  TN *lhs_tn;
  EBO_TN_INFO *lhs_tninfo;
  EBO_OP_INFO *ef_opinfo;

  if (top != TOP_targ_mov_r_b) return FALSE;
  if (!find_def_opinfo(opnd_tninfo[0], &def_opinfo)) return FALSE;

  if (! op_match_unary(def_opinfo->in_op,
		       def_opinfo->actual_opnd,
		       &lhs_tn, &lhs_tninfo)) {
    return FALSE;
  }

  if (OP_code(def_opinfo->in_op) == TOP_targ_mov_b_r
      && ISA_SUBSET_LIST_Member (ISA_SUBSET_List, TOP_mov_b_b) &&
      EBO_tn_available(OP_bb(op), lhs_tninfo)) {
    OPS ops = OPS_EMPTY;
    Exp_COPY(OP_result(op, 0), lhs_tn, &ops);
    OP_srcpos(OPS_last(&ops)) = OP_srcpos(op);
    if (EBO_in_loop) EBO_OPS_omega (&ops, lhs_tn, lhs_tninfo);
    if (!EBO_Verify_Ops(&ops)) return FALSE;
    BB_Insert_Ops(OP_bb(op), op, &ops, FALSE);
    if (EBO_Trace_Optimization) 
      fprintf(TFile,"Convert %s(%s) into copy\n",
	      TOP_Name(top), TOP_Name(OP_code(def_opinfo->in_op)));
    return TRUE;
  }
  return FALSE;
}

/*
 * convib_op_sequence
 *
 * Convert (convib (op)) into an op defining a branch register
 */
static BOOL
convib_op_sequence(OP *op, TN **opnd_tn, EBO_TN_INFO **opnd_tninfo)
{
  TOP top = OP_code(op);
  TN *lhs_tn, *rhs_tn;
  EBO_TN_INFO *lhs_tninfo, *rhs_tninfo;
  EBO_OP_INFO *def_opinfo;
  BOOL do_copy = FALSE;

  if (top != TOP_convib_r_b) return FALSE;
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
  if (OP_code(def_opinfo->in_op) == TOP_convbi_b_r
      && TOP_convbi_b_r == TOP_targ_mov_b_r) {
    // This is a branch_copy
    do_copy = TRUE;
  } else {
    TOP def_top = OP_code(def_opinfo->in_op);
    if (def_top == TOP_mov_r_r) {
      // [SC] Care: TOP_result_register_variant(TOP_mov_r_r) gives
      // TOP_mov_r_b, which is incorrect here.
      new_top = TOP_convib_r_b;
    } else {
      new_top = TOP_result_register_variant(OP_code(def_opinfo->in_op), 0, ISA_REGISTER_CLASS_branch);
      if (new_top == TOP_UNDEFINED) {
	// [HK] fix for ddts MTBst25392 RFE "shr 31 and cmpeq 0 is equivalent to checking sign"
	// replace shr r1, r2, 31; convib b1, r1; sequence with cmplt b1, r2, 0;
	if ((OP_code(def_opinfo->in_op) == TOP_shr_r_r_r || OP_code(def_opinfo->in_op) == TOP_shr_i_r_r || OP_code(def_opinfo->in_op) == TOP_shru_r_r_r || OP_code(def_opinfo->in_op) == TOP_shru_i_r_r) && 
	    TN_has_value(rhs_tn) && TN_value(rhs_tn) == 31)
	    {
	        new_top =  TOP_cmplt_r_r_b;
	        rhs_tn = Zero_TN;
	    }
	else
	  return FALSE;
      }
    }
  }

  BB *bb = OP_bb(op);
  if ((lhs_tninfo != NULL && !EBO_tn_available (bb, lhs_tninfo)) ||
      (rhs_tninfo != NULL && !EBO_tn_available (bb, rhs_tninfo))) {
      // Do not make any transformation if the in_op is a copy or if
      // register allocation was already performed (Dup_TN is not
      // allowed)
      if (do_copy || OP_effectively_copy(def_opinfo->in_op)|| EBO_in_peep)
	  return FALSE;
      else {
	// [HK] fix for ddts MTBst25391 RFE "Cmp to general register then convib could be cmp to branch bit"
	// insert branch register variant, just after the general register variant instruction, and
	// replace convib by a copy, which will be possibly removed afterwards
	  OP *new_op;
	  TN *new_tn;
	  OPS ops = OPS_EMPTY;
	  new_tn = Dup_TN(OP_result(op,0));
	  new_op = Mk_OP(new_top, new_tn, lhs_tn, rhs_tn);
	  if (EBO_in_loop) EBO_Set_OP_omega (new_op, lhs_tninfo, rhs_tninfo);
	  OP_srcpos(new_op) = OP_srcpos(def_opinfo->in_op);
	  if (!EBO_Verify_Op(new_op)) return FALSE;
	  BB_Insert_Op_Before(OP_bb(def_opinfo->in_op), def_opinfo->in_op, new_op);
	  if (lhs_tninfo != NULL)
	      inc_ref_count(lhs_tninfo);
	  if (rhs_tninfo != NULL)
	      inc_ref_count(rhs_tninfo);
       	  Exp_COPY(OP_result(op,0), new_tn, &ops);
	  OP_srcpos(OPS_last(&ops)) = OP_srcpos(op);
	  if (EBO_in_loop) EBO_OPS_omega (&ops, new_tn, NULL);
	  if (!EBO_Verify_Ops(&ops)) return FALSE;
	  BB_Insert_Ops(bb, op, &ops, FALSE);
	  if (EBO_Trace_Optimization) 
	      fprintf(TFile,"Convert cmp_r/convib/op sequence into %s\n", TOP_Name(new_top));
      }
  }
  else {
      if (do_copy) {
	  OPS ops = OPS_EMPTY;
	  Exp_COPY(OP_result(op,0), lhs_tn, &ops);
	  OP_srcpos(OPS_last(&ops)) = OP_srcpos(op);
	  if (EBO_in_loop) EBO_OPS_omega (&ops, lhs_tn, lhs_tninfo);
	  if (!EBO_Verify_Ops(&ops)) return FALSE;
	  BB_Insert_Ops(bb, op, &ops, FALSE);
	  if (EBO_Trace_Optimization) 
	      fprintf(TFile,"Convert convib/convbi into copy\n");
      } else {
	  OP *new_op;
	  new_op = Mk_OP(new_top, OP_result(op,0), lhs_tn, rhs_tn);
	  if (EBO_in_loop) EBO_Set_OP_omega (new_op, lhs_tninfo, rhs_tninfo);
	  OP_srcpos(new_op) = OP_srcpos(op);
	  if (!EBO_Verify_Op(new_op)) return FALSE;
	  BB_Insert_Op_After(bb, op, new_op);
	  if (EBO_Trace_Optimization) 
	      fprintf(TFile,"Convert convib/op into %s\n", TOP_Name(new_top));
      }
  }
  return TRUE;
}

/*
 * predicate_invert_sequence
 *
 * Convert (tn1 = cmpxx ; tn2 = norl_b_b_b (tn1, tn1)) into 
 *          tn1 = cmpxx ; tn2 = inv(cmpxx)
 */
static BOOL
predicate_invert_sequence(OP *op, TN **opnd_tn, EBO_TN_INFO **opnd_tninfo)
{
  TOP top = OP_code(op);
  BB *bb = OP_bb(op);
  TN *lhs_tn, *rhs_tn;
  EBO_TN_INFO *lhs_tninfo, *rhs_tninfo;
  EBO_OP_INFO *def_opinfo;
  
  // [SC] This transformation can obscure some andl/orl simplifications, since
  // it disassociates a tn from its complement.  So when EBO_in_pre, I suppress
  // this transformation to allow andl_orl_sequence to find its opportunities first.
  if (EBO_in_pre
      || (top != TOP_norl_b_b_b && top != TOP_nandl_b_b_b)
      || opnd_tn[0] != opnd_tn[1]) {
    return FALSE;
  }
  
  if (! find_def_opinfo (opnd_tninfo[0], &def_opinfo)) return FALSE;
  if (op_match_unary (def_opinfo->in_op,
		      def_opinfo->actual_opnd,
		      &lhs_tn, &lhs_tninfo)) {
    rhs_tn = NULL;
    rhs_tninfo = NULL;
  } else if (op_match_binary (def_opinfo->in_op,
			      def_opinfo->actual_opnd,
			      &lhs_tn, &lhs_tninfo,
			      &rhs_tn, &rhs_tninfo)) {
  } else
    return FALSE;
  
  if (! (lhs_tn
	 && (TN_is_constant (lhs_tn) || EBO_tn_available (bb, lhs_tninfo))))
    return FALSE;
  
  TOP new_top = TOP_UNDEFINED;
  if (OP_code(def_opinfo->in_op) == TOP_convib_r_b) {
    new_top = TOP_cmpeq_r_r_b;
    rhs_tn = Zero_TN;
  } else {
    if (rhs_tn
	&& (TN_is_constant (rhs_tn) || EBO_tn_available (bb, rhs_tninfo))) {
      new_top = CGTARG_Invert (OP_code (def_opinfo->in_op));
    }
  }
  if (new_top == TOP_UNDEFINED) return FALSE;
  
  OP *new_op = Mk_OP (new_top, OP_result(op, 0), lhs_tn, rhs_tn);
  OP_srcpos(new_op) = OP_srcpos(op);
  if (EBO_in_loop) EBO_Set_OP_omega (new_op, lhs_tninfo, rhs_tninfo);
  if (!EBO_Verify_Op(new_op)) return FALSE;
  BB_Insert_Op_After(bb, op, new_op);
  if (EBO_Trace_Optimization) 
    fprintf(TFile,"Convert norl/nandl predicate invert into %s\n", TOP_Name(new_top));
  return TRUE;
}

/*
 * convbi_op_sequence
 *
 * Convert (convbi (op)) into an op defining an integer register
 */
static BOOL
convbi_op_sequence(OP *op, TN **opnd_tn, EBO_TN_INFO **opnd_tninfo)
{
  TOP top = OP_code(op);
  TN *lhs_tn, *rhs_tn;
  EBO_TN_INFO *lhs_tninfo, *rhs_tninfo;
  EBO_OP_INFO *def_opinfo;

  if (top != TOP_convbi_b_r) return FALSE;
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
  
  BOOL inverted = FALSE;
  if (! EBO_in_pre
      && (OP_code(def_opinfo->in_op) == TOP_norl_b_b_b
	  || OP_code(def_opinfo->in_op) == TOP_nandl_b_b_b)
      && lhs_tn != NULL
      && lhs_tn == rhs_tn) {
    // This instruction is complementing the branch register.
    // Search further for its source.
    inverted = TRUE;
    if (!find_def_opinfo(lhs_tninfo, &def_opinfo)) return FALSE;
    if (op_match_unary(def_opinfo->in_op,
		       def_opinfo->actual_opnd, 
		       &lhs_tn, &lhs_tninfo)) {
      rhs_tn = NULL;
      rhs_tninfo = NULL;
    } else if (op_match_binary(def_opinfo->in_op,
			       def_opinfo->actual_opnd, 
			       &lhs_tn, &lhs_tninfo, &rhs_tn, &rhs_tninfo)) {
    } else return FALSE;
  }
  
  TOP new_top;
  if (OP_code(def_opinfo->in_op) == TOP_convib_r_b) {
    // This is a register normalization
    new_top = TOP_cmpne_r_r_r;
    rhs_tn = Zero_TN;
    rhs_tninfo = NULL;
  } else {
    new_top = TOP_result_register_variant(OP_code(def_opinfo->in_op), 0, ISA_REGISTER_CLASS_integer);
  }
  if (inverted) new_top = CGTARG_Invert (new_top);
  if (new_top == TOP_UNDEFINED) return FALSE;

  BB *bb = OP_bb(op);
  if ((lhs_tninfo != NULL && !EBO_tn_available (bb, lhs_tninfo)) ||
      (rhs_tninfo != NULL && !EBO_tn_available (bb, rhs_tninfo)))
    return FALSE;

  OP *new_op;
  new_op = Mk_OP(new_top, OP_result(op,0), lhs_tn, rhs_tn);
  OP_srcpos(new_op) = OP_srcpos(op);
  if (EBO_in_loop) EBO_Set_OP_omega (new_op, lhs_tninfo, rhs_tninfo);
  if (!EBO_Verify_Op(new_op)) return FALSE;
  BB_Insert_Op_After(bb, op, new_op);
  if (EBO_Trace_Optimization) 
    fprintf(TFile,"Convert convbi/op into %s\n", TOP_Name(new_top));
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
  if (OP_iand(op)) new_top = TOP_andc_r_r_r;
  if (OP_ior(op)) new_top = TOP_orc_r_r_r;
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
  if (!EBO_Verify_Op(new_op)) return FALSE;
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
  TN *l1_tn, *l2_tn, *lhs_tn, *rhs_tn;
  EBO_TN_INFO *l1_tninfo, *l2_tninfo, *lhs_tninfo, *rhs_tninfo;
  EBO_OP_INFO *def_opinfo;
  BB *bb = OP_bb(op);

  if (!OP_icmp(op)) return FALSE;

  if (!op_match_compare(op, 
			opnd_tninfo, 
			&lhs_tn, &lhs_tninfo, &rhs_tn, &rhs_tninfo))
    return FALSE;
  
  VARIANT variant = OP_cmp_variant(op);
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
  
  if (TN_register_class(lhs_tn) != TN_register_class(rhs_tn)) return FALSE;

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
      Expand_Logical_Move(OP_result(op, 0), lhs_tn, 0, &ops);
    }
    OP_srcpos(OPS_last(&ops)) = OP_srcpos(op);
    if (EBO_in_loop) EBO_OPS_omega (&ops, lhs_tn, lhs_tninfo);
    if (!EBO_Verify_Ops(&ops)) return FALSE;
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
    case V_CMP_ANDL: new_top = TOP_andl_r_r_r; break;
    case V_CMP_ORL: new_top = TOP_orl_r_r_r; break;
    case V_CMP_NANDL: new_top = TOP_nandl_r_r_r; break;
    case V_CMP_NORL: new_top = TOP_norl_r_r_r; break;
    }
    
    TN *result;
    result = OP_result(op, 0);
    new_top = TOP_result_register_variant(new_top, 0, 
					  TN_register_class(result));
    if (TN_register_class(lhs_tn) == ISA_REGISTER_CLASS_branch) {
      switch (new_top) {
      case TOP_andl_r_r_b:  new_top = TOP_andl_b_b_b;  break;
      case TOP_orl_r_r_b:   new_top = TOP_orl_b_b_b;   break;
      case TOP_nandl_r_r_b: new_top = TOP_nandl_b_b_b; break;
      case TOP_norl_r_r_b:  new_top = TOP_norl_b_b_b;  break;
      default: new_top = TOP_UNDEFINED;                break;
      }
      if (!ISA_SUBSET_LIST_Member(ISA_SUBSET_List, new_top)) {
	new_top = TOP_UNDEFINED;
      }
    }
    if (new_top == TOP_UNDEFINED) return FALSE;
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
 * - (orl (orl x y) x) into (orl x y)
 */
static BOOL
andl_orl_sequence_2(OP *op, TN **opnd_tn, EBO_TN_INFO **opnd_tninfo)
{
  BB *bb = OP_bb(op);
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
  
  VARIANT variant = OP_cmp_variant(op);
  if (variant != V_CMP_ORL &&
      variant != V_CMP_ANDL) return FALSE;

  if (find_def_opinfo(lhs_tninfo, &def_opinfo) &&
      op_match_compare(def_opinfo->in_op,
		       def_opinfo->actual_opnd, 
		       &l1_lhs_tn, &l1_lhs_tninfo, &l1_rhs_tn, &l1_rhs_tninfo)) {
    VARIANT l1_variant = OP_cmp_variant(def_opinfo->in_op);
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
    VARIANT l1_variant = OP_cmp_variant(def_opinfo->in_op);
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
  
  if (TN_register_class(src_tn) == ISA_REGISTER_CLASS_branch) {
    Expand_Bool_To_Int (OP_result(op,0), src_tn, MTYPE_I4, &ops);
  } else if (TN_register_class(OP_result(op,0)) == ISA_REGISTER_CLASS_branch) {
    Expand_Int_To_Bool (OP_result(op,0), src_tn, &ops);
  } else {
    Exp_COPY(OP_result(op,0), src_tn, &ops);
  }

  OP_srcpos(OPS_last(&ops)) = OP_srcpos(op);
  if (EBO_in_loop) EBO_OPS_omega (&ops, src_tn, src_tninfo);
  if (!EBO_Verify_Ops(&ops)) return FALSE;
  BB_Insert_Ops_After(bb, op, &ops);
  if (EBO_Trace_Optimization) 
    fprintf(TFile,"Convert andl/orl into move\n");

  return TRUE;
}

/*
 * reduce_predicate_logical_sequence
 *
 * Convert:
 *   x ANDL y  => y      iff x DOM y (i.e. x is true whenever y is true)
 *   x ANDL y  => FALSE  iff x, y disjoint
 *   x ORL  y  => x      iff x DOM y
 *   x ORL  y  => TRUE   iff x compl y
 *   x NANDL y => !y     iff x DOM y
 *   x NANDL y => TRUE   iff x, y disjoint
 *   x NORL  y => !x     iff x DOM y
 *   x NORL  y => FALSE  iff x, compl y
 */
static BOOL
reduce_predicate_logical_sequence (OP *op, TN **opnd_tn,
				   EBO_TN_INFO **opnd_tninfo)
{
  VARIANT v = OP_cmp_variant(op);
  BOOL negate_value = FALSE;
  TN *result = OP_result(op, 0);
  TN *value = NULL;

  if (v == V_CMP_NANDL) {
    negate_value = TRUE; v = V_CMP_ANDL;
  } else if (v == V_CMP_NORL) {
    negate_value = TRUE; v = V_CMP_ORL;
  }
  TN *tn1 = opnd_tn[0];
  TN *tn2 = opnd_tn[1];
  EBO_TN_INFO *tn1_info = opnd_tninfo[0];
  EBO_TN_INFO *tn2_info = opnd_tninfo[1];
  if (v == V_CMP_ANDL
      && EBO_predicate_disjoint (tn1, false, tn1_info, tn2, false, tn2_info)) {
    value = Gen_Literal_TN (0, 4);
  } else if (v == V_CMP_ORL
	     && EBO_predicate_complements (tn1, false, tn1_info, tn2, false, tn2_info)) {
    value = Gen_Literal_TN (1, 4);
  } else if (EBO_predicate_dominates (tn1, false, tn1_info, tn2, false, tn2_info)) {
    value = (v == V_CMP_ANDL) ? tn2 : tn1;
  } else if (EBO_predicate_dominates (tn2, false, tn2_info, tn1, false, tn1_info)) {
    value = (v == V_CMP_ANDL) ? tn1 : tn2;
  }
  if (value) {
    OPS ops = OPS_EMPTY;
    if (negate_value) {
      Expand_Logical_Not (result, value, V_NONE, &ops);
    } else {
      Expand_Logical_Move(OP_result(op, 0), value, 0, &ops);
    }
    OP_srcpos(OPS_last(&ops)) = OP_srcpos(op);
    if (!EBO_Verify_Ops(&ops)) return FALSE;
    BB_Insert_Ops_After(OP_bb(op), op, &ops);
    if (EBO_Trace_Optimization) 
      fprintf(TFile,"Reduce predicate logical operation\n");
    return TRUE;
  } else {
    return FALSE;
  }
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
  
  if ((TN_Has_Value(lhs_tn) 
      || ((lhs_tninfo != NULL)
	  && (lhs_tninfo->replacement_tn != NULL)
	  && TN_Has_Value(lhs_tninfo->replacement_tn)))
      && TN_is_register(rhs_tn)) {
    value =  TN_Has_Value(lhs_tn) ? 
      TN_Value(lhs_tn) : TN_Value(lhs_tninfo->replacement_tn);
    src_tn = rhs_tn;
    src_tninfo = rhs_tninfo;
  }
  else if ((TN_Has_Value(rhs_tn) 
	   || ((rhs_tninfo != NULL)
	       && (rhs_tninfo->replacement_tn != NULL)
	       && TN_Has_Value(rhs_tninfo->replacement_tn)))
	   && TN_is_register(lhs_tn)) {
    value =  TN_Has_Value(rhs_tn) ? 
      TN_Value(rhs_tn) : TN_Value(rhs_tninfo->replacement_tn);
    src_tn = lhs_tn;
    src_tninfo = lhs_tninfo;
  } else 
    return FALSE;
  
  BOOL bvar = FALSE;
  if (TN_register_class(src_tn) == ISA_REGISTER_CLASS_integer) 
    bvar = FALSE;
  else if (TN_register_class(src_tn) == ISA_REGISTER_CLASS_branch)
    bvar = TRUE;
  else
    return FALSE;

  BOOL notl = FALSE;
  BOOL immediate = FALSE;
  VARIANT variant = OP_cmp_variant(op);
  switch (variant) {
  case V_CMP_ANDL:
    if (value == 0) {
      // (andl x 0) -> (movl 0)
      immediate = TRUE;
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
      immediate = TRUE;
      src_tn = Gen_Literal_TN(1, 4);
      src_tninfo = NULL;
    }
    break;
  case V_CMP_NANDL:
    if (value == 0) {
      // (nandl x 0) -> (movl 1)
      immediate = TRUE;
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
      immediate = TRUE;
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
  if (!bvar) { // r_r_r or b_r_r variant
    if (notl)
      Expand_Logical_Not(OP_result(op, 0), src_tn, 0, &ops);
    else
      Expand_Logical_Move(OP_result(op, 0), src_tn, 0, &ops);
  } else { // b_b_b variant
    if (immediate)
      Expand_Immediate(OP_result(op,0), src_tn, 0, &ops);
    else// branch to branch copy case (not efficient on st200)
      return FALSE;	
  }
  
  OP_srcpos(OPS_last(&ops)) = OP_srcpos(op);
  if (EBO_in_loop) EBO_OPS_omega (&ops, src_tn, src_tninfo);
  if (!EBO_Verify_Ops(&ops)) return FALSE;
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
  if (!EBO_Verify_Op(new_op)) return FALSE;
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
 * ST200: Treat equivalent of convib of 0 and convib of 1.
 * ============================================================
 */
TN *
EBO_literal_replacement_tn(OP *op)
{
  TOP opcode = OP_code(op);
  if (opcode == TOP_convib_r_b && OP_opnd(op, 0) == Zero_TN) {
    return Gen_Literal_TN (0, TN_size(OP_opnd(op, 0)));
  } else if (opcode == TOP_cmpeq_r_r_b && OP_opnd(op, 0) == Zero_TN && OP_opnd(op, 1) == Zero_TN) {
    return Gen_Literal_TN (1, TN_size(OP_opnd(op, 0)));
  }

  return NULL;
}

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
      if (idx == 1 && opcode == TOP_sub_r_r_r && TN_has_value(replacement_tn)) {
	TN *neg_tn = Gen_Literal_TN((INT64)(INT32)(-TN_value(replacement_tn)), 4);
	new_opcode = TOP_opnd_immediate_variant(TOP_add_r_r_r, idx, TN_value(neg_tn));
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
    // TOP_opnd_swapped_variant does not exist anymore
	new_opcode = OP_opnd_swapped_variant(op, idx, idx2);
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

  // Do not reselect ops with implicit interactions
  if (OP_has_implicit_interactions(op)) return FALSE;

#ifdef Is_True_On
  if (getenv("NO_EBO_SPECIAL")) return FALSE;
#endif

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
    if (cmp_subsat_to_zero(op, opnd_tn, opnd_tninfo)) return TRUE;
    if ((OP_cmp_variant (op) == V_CMP_ANDL
	|| OP_cmp_variant (op) == V_CMP_NANDL
	|| OP_cmp_variant (op) == V_CMP_ORL
	|| OP_cmp_variant (op) == V_CMP_NORL)
	&& reduce_predicate_logical_sequence (op, opnd_tn, opnd_tninfo)) return TRUE;
  }

  if (opcode == TOP_convib_r_b && convib_op_sequence(op, opnd_tn, opnd_tninfo)) return TRUE;
  if (opcode == TOP_convbi_b_r && convbi_op_sequence(op, opnd_tn, opnd_tninfo)) return TRUE;
  if (predicate_invert_sequence(op, opnd_tn, opnd_tninfo)) return TRUE;

  if (opcode == TOP_targ_mov_r_b
      && copy_r_b_sequence(op, opnd_tn, opnd_tninfo)) return TRUE;

  if (OP_memory(op) && base_offset_sequence(op, opnd_tn, opnd_tninfo)) return TRUE;

  if (ext_move_sequence(op, opnd_tn, opnd_tninfo)) return TRUE;

  if (and_or_sequence(op, opnd_tn, opnd_tninfo)) return TRUE;

  if (addcg_sequence(op, opnd_tn, opnd_tninfo)) return TRUE;

  if (operand_special_sequence(op, opnd_tn, opnd_tninfo)) return TRUE;

  if (shadds_sequence(op, opnd_tn, opnd_tninfo)) return TRUE;

  if (minmax_sats_sequence(op, opnd_tn, opnd_tninfo)) return TRUE;

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
      (opcode == TOP_shr_i_r_r)   ||
      (opcode == TOP_shr_i_u)) {
    return (sxt_sequence  (op, 1, opnd_tn, opnd_tninfo));
  }
  if (opcode == TOP_and_i_r_r) {
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

void EBO_Combine_Imm_Base_Offset()
{
    
}
