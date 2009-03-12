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

/* ====================================================================
 * ====================================================================
 *
 *       OP utility routines which include target dependencies.
 *
 * ====================================================================
 * ====================================================================
 */

#include "defs.h"
#include "config.h"
#include "erglob.h"
#include "xstats.h"
#include "tracing.h"

#include "cgir.h"
#include "whirl2ops.h"

#include "cg.h"
#include "opt_alias_mgr.h"
#include "targ_cg_private.h"
#include "targ_isa_variants.h"
#include "config_TARG.h"
#include "cgtarget.h"

#include "stblock.h" // for ST_alignment

/* ====================================================================
 *   OP_is_ext_op
 * ====================================================================
 */
BOOL OP_is_ext_op(OP *op) 
{
  return FALSE;
}

/* ====================================================================
 *   OP_Is_Barrier
 *
 * ====================================================================
 */
BOOL OP_Is_Barrier(OP *op) 
{
  TOP top = OP_code(op);

  if (top == TOP_asm) {
    ASM_OP_ANNOT* asm_info = (ASM_OP_ANNOT*) OP_MAP_Get(OP_Asm_Map, op);
    if (WN_Asm_Clobbers_Mem(ASM_OP_wn(asm_info)))
      return TRUE;
  }

  if (TOP_is_barrier(top)) {
    return TRUE;
  }

  return FALSE;
}

/* ====================================================================
 *   OP_is_counted_loop
 * ====================================================================
 */
BOOL 
OP_Is_Counted_Loop(OP *op) 
{
  return FALSE;
}

/* ====================================================================
 *   OP_Is_Copy_Immediate_Into_Register
 * ====================================================================
 */
BOOL 
OP_Is_Copy_Immediate_Into_Register(OP *op) 
{
  return (OP_code(op) == TOP_mov_i_r ||
	  OP_code(op) == TOP_mov_ii_r);
}

/* ====================================================================
 *   OP_Has_Latency
 * ====================================================================
 */
BOOL 
OP_Has_Latency(OP *op) 
{
  return (CGTARG_Max_OP_Latency (op)>1);
}

/* ====================================================================
 *   OP_Is_Advanced_Load
 * ====================================================================
 */
BOOL
OP_Is_Advanced_Load( OP *memop )
{
  // no advanced loads for st200
  return FALSE;
}

/* ====================================================================
 *   OP_Is_Speculative_Load
 * ====================================================================
 */
BOOL
OP_Is_Speculative_Load ( 
  OP *memop 
)
{
  if (!OP_load(memop)) return FALSE;
  if (TOP_is_dismissible(OP_code(memop))) return TRUE;

  return FALSE;
}

/* ====================================================================
 *   OP_Is_Check_Load
 * ====================================================================
 */
BOOL
OP_Is_Check_Load ( 
  OP *memop 
)
{
  if (!OP_load(memop)) return FALSE;

  return FALSE;
}


/* ====================================================================
 *   OP_Is_Speculative
 * ====================================================================
 */
BOOL
OP_Is_Speculative (
  OP *op
)
{
  if (!OP_load(op)) return FALSE;

  // speculative and advanced loads are safe to speculate.
  if (OP_Is_Advanced_Load(op) || OP_Is_Speculative_Load(op))
    return TRUE;

  return FALSE;
}

/* ====================================================================
 *   OP_Performance_Effects
 *
 *   True if the op has performance effects that make its
 *   speculation undesirable.
 *   False otherwise.
 * ====================================================================
 */
BOOL OP_Performance_Effects (
  OP *op
)
{
  TOP opcode = OP_code(op);
  if (TOP_is_div(opcode) || TOP_is_rem(opcode)) {
    return TRUE;
  }

  return FALSE;
}

/* ====================================================================
 *   OP_Safe_Effects
 *
 *   True if the op normally has performance effects that make its
 *   speculation undesirable, but these should be ignored in this case.
 *   False otherwise.
 * ====================================================================
 */
BOOL OP_Safe_Effects (
  OP *op
)
{
  TOP opcode = OP_code(op);
  if (TOP_is_div(opcode) || TOP_is_rem(opcode)) {
    return Enable_Speculate_DivRem ? TRUE : FALSE;
  }

  return FALSE;
}

/* ====================================================================
 *   OP_Can_Be_Speculative
 *
 *   determines if the TOP can be speculatively executed taking 
 *   into account eagerness level
 * ====================================================================
 */
BOOL OP_Can_Be_Speculative (
  OP *op
)
{
  WN *wn;
  TN *offset;
  TOP opcode = OP_code(op);

  // can never speculate a call.
  if (OP_call (op))
    return FALSE;

  // [CG] Not clear if we can speculate an asm
  if (OP_code(op) == TOP_asm) return FALSE;

  if (OP_Is_Barrier(op)) return FALSE;

  if (OP_side_effects (op)) return FALSE;

  if (OP_Performance_Effects(op)) return OP_Safe_Effects (op);

  switch (Eager_Level) {
   case EAGER_NONE:

     /* not allowed to speculate anything
     */
     return FALSE;
     break;

  case EAGER_SAFE:

    /* Only exception-safe speculative ops are allowed
     */
#if 0
    if (TOP_is_ftrap(opcode) || TOP_is_itrap(opcode)) return FALSE;
#endif
    /*FALLTHROUGH*/

  case EAGER_ARITH:

    /* Arithmetic exceptions allowed
     */
    if (OP_fdiv(op)) return FALSE;
    /*FALLTHROUGH*/

  case EAGER_DIVIDE:

    /* Divide by zero exceptions allowed 
     */
#if 0
    if (TOP_is_memtrap(opcode)) return FALSE;
#endif
    /*FALLTHROUGH*/

  case EAGER_MEMORY:

    /* Memory exceptions allowed / All speculative ops allowed
     */
    if (TOP_is_unsafe(opcode)) return FALSE;
    break;

  default:
    DevWarn("unhandled eagerness level: %d", Eager_Level);
    return FALSE;
  }

  if (!OP_memory (op)) return TRUE;

  /* prefetch are speculative. */
  if (OP_prefetch (op)) return TRUE;

  /* This is a memory reference */

  /* don't speculate volatile memory references. */
  if (OP_volatile(op)) return FALSE;

  if (!OP_load(op)) return FALSE;

  /* Try to identify simple scalar loads than can be safely speculated:
   *  a) read only loads (literals, GOT-loads, etc.)
   *  b) load of a fixed variable (directly referenced)
   *  c) load of a fixed variable (base address is constant or
   *     known to be in bounds)
   *  d) speculative, advanced and advanced-speculative loads are safe.
   */

  /*  a) read only loads (literals, GOT-loads, etc.)
   */
  if (OP_no_alias(op)) goto scalar_load;

  /*  b) load of a fixed variable (directly referenced); this
   *     includes spill-restores.
   *  b') exclude cases of direct loads of weak symbols (#622949).
   */
  if ((offset = OP_Offset(op))
      && TN_is_symbol(offset)
      && ! ST_is_weak_symbol(TN_var(offset))) {
    // FdF 20071015: Be careful that 64 bit loads created by packing have alignment
    // constraints that may non longer be satisfied after
    // speculation. Unless we can be sure that the load is statically
    // correctly aligned, consider that speculation is unsafe
    if (OP_packed(op)) {
      if (!TN_is_zero(OP_Base(op)) || ((ST_alignment(TN_var(offset))&7) != 0))
	return FALSE;
    }
    goto scalar_load;
  }

  /*  c) load of a fixed variable (base address is constant or
   *     known to be in bounds).
   */
  if ((wn = Get_WN_From_Memory_OP(op))
      && Alias_Manager
      && Safe_to_speculate(Alias_Manager, wn)) goto scalar_load;

  /* we can't speculate a load unless it is marked as dismissable */
  /* it is the client's responsability to do that. */
  if (OP_Is_Speculative(op)) goto scalar_load;
  
  /* If we got to here, we couldn't convince ourself that we have
   * a scalar load -- no speculation this time...
   */
  return FALSE;

  /* We now know we have a scalar load of some form. Determine if they
   * are allowed to be speculated.
   */
scalar_load:
  return TRUE; 

}

/* ====================================================================
 *   CGTARG_Predicate_OP
 * ====================================================================
 */
void
CGTARG_Predicate_OP (
  BB* bb, 
  OP* op, 
  TN* pred_tn,
  bool on_false
)
{
  if (OP_has_predicate(op)) {
    FmtAssert(!on_false ,("CGTARG_Predicate_OP: can't have a predicate valid if not"));
    Set_OP_opnd(op, OP_find_opnd_use(op,OU_predicate), pred_tn);
    Set_OP_cond_def_kind(op, OP_PREDICATED_DEF);
  }
  else {
    FmtAssert(FALSE,("CGTARG_Predicate_OP: target does not support predication"));
  }

}

/* ====================================================================
 *   Unpredicated_Op
 * ====================================================================
 */
static TOP
Unpredicated_Op (TOP opcode) {
  static TOP *unpredicated_optable = NULL;
  if (! unpredicated_optable) {
    unpredicated_optable = TYPE_MEM_POOL_ALLOC_N(TOP, Malloc_Mem_Pool,
                                                 (TOP_count + 1));
    TOP i;
    for (i = 0; i <= TOP_count; i++) {
      unpredicated_optable[i] = TOP_UNDEFINED;
    }
    for (i = 0; i <= TOP_count; i++) {
      TOP predicated_op = st200_Predicated_Load_Op (i);
      if (predicated_op == TOP_UNDEFINED) {
        predicated_op = st200_Predicated_Store_Op (i);
      }
      if (predicated_op != TOP_UNDEFINED) {
        unpredicated_optable[predicated_op] = i;
      }
    }
  }
  return unpredicated_optable[opcode];
}
         
/* ====================================================================
 *   Predicated_Op
 * ====================================================================
 */
static TOP
Predicated_Op (TOP opcode) {
  static TOP *predicated_optable = NULL;
  if (! predicated_optable) {
    predicated_optable = TYPE_MEM_POOL_ALLOC_N(TOP, Malloc_Mem_Pool,
                                               (TOP_count + 1));
    for (TOP i = 0; i <= TOP_count; i++) {
      TOP predicated_op = st200_Predicated_Load_Op (i);
      if (predicated_op == TOP_UNDEFINED) {
        predicated_op = st200_Predicated_Store_Op (i);
      }
      predicated_optable[i] = predicated_op;
    }
  }
  return predicated_optable[opcode];
}
         
/* ====================================================================
 *   CGTARG_Dup_OP_Predicate
 *
 *     Duplicate OP and give the copy the predicate NEW_PRED.
 *     Note that OP may be unpredicated, in which case we need to
 *     change it to the predicated form.
 *     Note that NEW_PRED may be True_TN, in which case we should
 *     change OP to the unpredicated form.
 * ====================================================================
 */
OP *
CGTARG_Dup_OP_Predicate (OP *op, TN *new_pred)
{
  OP *new_op;

  if ((OP_has_predicate (op) && new_pred != True_TN)
      || (! OP_has_predicate (op) && new_pred == True_TN)) {
    // No need to change between unpredicated/predicated form.
    new_op = Dup_OP (op);
    if (OP_has_predicate (new_op)) {
      Set_OP_opnd (new_op, OP_find_opnd_use(new_op, OU_predicate), new_pred);
    }
  } else if (OP_has_predicate (op)) {
    // new_pred == True_TN, so remove the predicate from op.
    INT pred_opnd_idx = OP_find_opnd_use(op,OU_predicate);
    TOP topcode = Unpredicated_Op (OP_code (op));
    TN *new_opnds[ISA_OPERAND_max_operands];
    TN *new_results[ISA_OPERAND_max_results];
    INT opndnum, resnum;
    INT new_opndnum = 0;

    for (opndnum = 0; opndnum < OP_opnds (op); opndnum++) {
      if (opndnum != pred_opnd_idx) {
        new_opnds[new_opndnum++] = OP_opnd (op, opndnum);
      }
    }
    for (resnum = 0; resnum < OP_results (op); resnum++) {
      new_results[resnum] = OP_result (op, resnum);
    }
    new_op = Mk_VarOP (topcode, resnum, new_opndnum, new_results, new_opnds);
    OP_Copy_Properties (new_op, op);
    Set_OP_cond_def_kind (new_op, OP_UNKNOWN_DEF);
  } else {
    // op is not predicated, but new_pred != True_TN,
    // so we need to create the predicated form.
    TOP topcode = Predicated_Op (OP_code (op));
    INT pred_opnd_idx = TOP_Find_Operand_Use (topcode, OU_predicate);
    TN *new_opnds[ISA_OPERAND_max_operands];
    TN *new_results[ISA_OPERAND_max_results];
    INT opndnum = 0, resnum;
    INT new_opndnum;
    for (new_opndnum = 0; new_opndnum < (OP_opnds (op) + 1); new_opndnum++) {
      TN *tn = (new_opndnum == pred_opnd_idx) ? new_pred : OP_opnd (op, opndnum++);
      new_opnds[new_opndnum] = tn;
    }
    for (resnum = 0; resnum < OP_results (op); resnum++) {
      new_results[resnum] = OP_result (op, resnum);
    }
    new_op = Mk_VarOP (topcode, resnum, new_opndnum, new_results, new_opnds);
    OP_Copy_Properties (new_op, op);
    Set_OP_cond_def_kind (new_op, OP_PREDICATED_DEF);
  }
  return new_op;
}

/* ====================================================================
 *   OP_Copy_Operand
 *
 *   Test and return operand for copy operations.
 *   Return the operand index if the operation is a copy from an
 *   immediate value or a register to a register of the same class.
 *   Returns -1 if the operation is not a copy.
 *   This function returns -1 also if the copy is predicated.
 * ====================================================================
 */
INT 
OP_Copy_Operand (
  OP *op
)
{
  TOP opcode = OP_code(op);

  if (opcode == TOP_spadjust) {
    return -1;
  }

  if (OP_iadd(op) || OP_ior(op) || OP_ixor(op)) {


    if ((TN_is_register(OP_opnd(op,0)) &&
         TN_register_and_class(OP_opnd(op,0)) == CLASS_AND_REG_zero) ||
        ((TN_has_value(OP_opnd(op,0)) && TN_value(OP_opnd(op,0)) == 0))) {
      return 1;
    }

    if ((TN_is_register(OP_opnd(op,1)) &&
         TN_register_and_class(OP_opnd(op,1)) == CLASS_AND_REG_zero) ||
        ((TN_has_value(OP_opnd(op,1)) && TN_value(OP_opnd(op,1)) == 0))) {
      return 0;
    }
  }

  if (OP_iand(op)) {
    if ((TN_has_value(OP_opnd(op,0)) && TN_value(OP_opnd(op,0)) == ~0)) {
      return 1;
    }
    if ((TN_has_value(OP_opnd(op,1)) && TN_value(OP_opnd(op,1)) == ~0)) {
      return 0;
    }
  }

  // [HK] treat case or/and y = x, x <=> mov y = x
  if (OP_iand(op) || OP_ior(op)) {
    if (TN_is_register(OP_opnd(op,0)) && TN_is_register(OP_opnd(op,1)) \
	&& TNs_Are_Equivalent(OP_opnd(op,0), OP_opnd(op,1)))
      return 0;
  }

  if (TOP_is_move(opcode)) {
    return TOP_Find_Operand_Use(opcode, OU_opnd1);
  }

  // [CG]: Some ops may be marked as copy
  // while they are part of a sequence.
  // This treats the branch copy which is:
  // mov_i t, -1
  // addcg r0, dst = r0, t, src (copy)
  if (OP_copy(op)) {
    if (opcode == TOP_targ_addcg_b_r_r_b_r) return 2;
  }
    
  return -1;
}

/* ====================================================================
 *   OP_Copy_Result
 *
 *   Returns the result operand index for operations
 *   that have a defined OP_Copy_Operand.
 * ====================================================================
 */
INT 
OP_Copy_Result (
  OP *op
)
{

  // [CG]: Some ops may be marked as copy
  // while they are part of a sequence.
  // This treats the branch copy which is:
  // mov_i t, -1
  // addcg r0, dst = r0, t, src (copy)
  if (OP_copy(op)) {
    if (OP_code(op) == TOP_targ_addcg_b_r_r_b_r) return 1;
  }
  return 0;
}

/* ====================================================================
 *   CGTARG_Noop_Top
 * ====================================================================
 */
TOP 
CGTARG_Noop_Top (ISA_EXEC_UNIT_PROPERTY unit) { return TOP_nop; } 

/* ====================================================================
 *   OP_save_predicates/OP_restore_predicates
 * ====================================================================
 */
BOOL OP_save_predicates(OP *op) {
  
 return FALSE; 
}

BOOL OP_restore_predicates(OP *op) {
  return FALSE;
}

/* ====================================================================
 *   OP_is_associative
 *
 *   The list of TOPs that will be handled by the reassociation algorithm.
 * ====================================================================
 */
BOOL
OP_is_associative(OP *op)
{
  TOP top = OP_code(op);

  switch (top) {
  case TOP_add_r_r_r:
  case TOP_sub_r_r_r:
    return true;
  }

  return false;
}

/* ====================================================================
 *   TOP_opposite
 *
 *   Give the opposite form, e.g,  - => +,  + => -.
 *
 *   TODO: belongs to the targ_info.
 * ====================================================================
 */
TOP 
TOP_opposite(TOP top)
{
  switch (top) {
  case TOP_add_r_r_r:
    return TOP_sub_r_r_r;

  case TOP_sub_r_r_r:
    return TOP_add_r_r_r;
  }

  return TOP_UNDEFINED;
}

/* ====================================================================
 *   TOP_immediate
 *
 *   Give the immediate form.
 *
 *   TODO: belongs to the targ_info.
 * ====================================================================
 */
TOP 
TOP_immediate(TOP top)
{
  switch (top) {
  case TOP_add_r_r_r:
    return TOP_opnd_immediate_variant (top, 1, 1);
  case TOP_sub_r_r_r:
    return TOP_opnd_immediate_variant (top, 0, 1);
  }

  return TOP_UNDEFINED;
}


/* ====================================================================
 *   TOP_equiv_nonindex_memory
 *
 *   TODO: belongs to the targ_info.
 * ====================================================================
 */
TOP 
TOP_equiv_nonindex_memory(TOP top)
{
  return TOP_UNDEFINED;
}

/* ====================================================================
 *   CGTARG_Which_OP_Select
 * ====================================================================
 */
TOP
CGTARG_Which_OP_Select ( 
  UINT16 bit_size, 
  BOOL is_float, 
  BOOL is_fcc 
)
{
  if (!is_float && !is_fcc) return TOP_targ_slct_r_r_b_r;
  return TOP_UNDEFINED;
}


/* ====================================================================
 *   CGTARG_OP_Get_Flag_Effects
 * ====================================================================
 */
OP_Flag_Effects
CGTARG_OP_Get_Flag_Effects(const OP *op)
{
  /* For ST200 there is no operation with implicit flag effect. */
  return OP_FE_NONE;
}



/* ====================================================================
 *   OP_opnd_can_be_reassociated
 *
 *   Test whether the OPND can be reassociated with the OP.
 * ====================================================================
 */
BOOL
OP_opnd_can_be_reassociated (
  OP *op, 
  INT opnd
)
{
  TOP top = OP_code(op);

  switch (top) {
  case TOP_add_r_r_r:
    return (opnd == 0 || opnd == 1);

  case TOP_sub_r_r_r:
    return (opnd == 0);
  }
    
  return false;
}

/* ====================================================================
 *   OP_other_opnd
 *
 *   The other opnd involved in reassociation
 * ====================================================================
 */
INT 
OP_other_opnd(OP *op, INT this_opnd)
{
  switch (OP_code(op)) {
  case TOP_add_r_r_r:
  case TOP_sub_r_r_r:
    if (this_opnd == 1) return 0;
    if (this_opnd == 0) return 1;
    break;
  }
  Is_True(FALSE, ("Other_opnd: wrong opnd num"));
  return 0;
}

/* ====================================================================
 *   CGTARG_Init_OP_cond_def_kind
 * ====================================================================
 */
void 
CGTARG_Init_OP_cond_def_kind (
  OP *op
)
{
  TOP top = OP_code(op);
  switch (top) {

  default:
    if (OP_has_predicate(op))
      Set_OP_cond_def_kind(op, OP_PREDICATED_DEF);
    else
      Set_OP_cond_def_kind(op, OP_ALWAYS_UNC_DEF);
    break;
  }
}

/* =====================================================================
 *   OP_Is_Unconditional_Compare
 * =====================================================================
 */
BOOL
OP_Is_Unconditional_Compare (
  OP *op
)
{
  return (OP_icmp(op));
}

/*
 * TOP_opnd_immediate_variant_default
 *
 * Default target independent implementation for
 * TOP_opend_immediate_variant.
 * The implementation is driven by the targinfo variant 
 * description (see isa_variant.cxx).
 * VARATT_immediate gives for any register or immediate operator the first immediate form.
 * VARATT_next_immediate gives the next larger immediate form.
 * Note that this model works only when 1 operand varies for the operator.
 * The matching test is done on the literal class for the given operand.
 */
static TOP
TOP_opnd_immediate_variant_default(TOP regform, int opnd, INT64 imm)
{
  TOP immform;
  // Check if it is already an immediate form
  if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, regform)) {
    const ISA_OPERAND_INFO *oinfo = ISA_OPERAND_Info (regform);
    const ISA_OPERAND_VALTYP *otype = ISA_OPERAND_INFO_Operand(oinfo, opnd);
    if (ISA_OPERAND_VALTYP_Is_Literal(otype)) {
      immform = regform;
      // Get to the first immediate variant.
      TOP prevform = TOP_get_variant(immform, VARATT_prev_immediate);
      while (prevform != TOP_UNDEFINED && ISA_SUBSET_LIST_Member(ISA_SUBSET_List, prevform)) {
	immform = prevform;
	prevform = TOP_get_variant(prevform, VARATT_prev_immediate);
      }
    } else {
      immform = TOP_get_variant(regform, VARATT_immediate);
    }
  }  
  
  // Get the first immediate form that fits the operand
  while (immform != TOP_UNDEFINED && ISA_SUBSET_LIST_Member(ISA_SUBSET_List, immform)) {
    const ISA_OPERAND_INFO *oinfo = ISA_OPERAND_Info (immform);
    const ISA_OPERAND_VALTYP *otype = ISA_OPERAND_INFO_Operand(oinfo, opnd);
    if (ISA_OPERAND_VALTYP_Is_Literal(otype)) {
      ISA_LIT_CLASS lit_class = ISA_OPERAND_VALTYP_Literal_Class(otype);
      if (ISA_LC_Value_In_Class (imm, lit_class))
	break;
    }
    immform = TOP_get_variant(immform, VARATT_next_immediate);

  }
  return immform;
}


/*
 * TOP_opnd_immediate_variant
 *
 * Returns the TOP immediate variant, depending on the immediate value.
 * Target dependant.
 * The reg form may be a register or immediate opcode.
 * opnd is the operand number that may be replaced (0..2).
 * imm is the immediate value that should be encoded.
 * Returns TOP_UNDEFINED, if no immediate variant is available.
 */
TOP
TOP_opnd_immediate_variant(TOP regform, int opnd, INT64 imm)
{

  // [SC] Special case: mul32_i can act as mull_i, the immediate
  // variant table does not catch this.
  if (opnd == 1
      && (regform == TOP_mull_r_r_r || regform == TOP_mull_i_r_r || regform == TOP_mull_ii_r_r)) {
    int s9 = (imm >= -(1<<8) && imm < (1<<8)) ? 1 : 0;
    if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_mul32_i_r_r)) {
      return s9 ? TOP_mul32_i_r_r : TOP_mul32_ii_r_r;
    } else {
      return s9 ? TOP_mull_i_r_r : TOP_mull_ii_r_r;
    }
  }

  TOP immform = TOP_get_variant(regform, VARATT_immediate);

  while (immform != TOP_UNDEFINED) {
    const ISA_OPERAND_INFO *oinfo = ISA_OPERAND_Info (immform);
    const ISA_OPERAND_VALTYP *otype = ISA_OPERAND_INFO_Operand(oinfo, opnd);
    INT osize = ISA_OPERAND_VALTYP_Size (otype);
    INT64 minval, maxval;
    // [SC] Hack for the osize == 32 case.  The operand type for a
    // ldw imm.offset is marked as signed, but we can use it to
    // contain an unsigned address.  It effectively wraps around the
    // address space.
    if (osize == 32) { minval = imm; maxval = imm; }
    if (ISA_OPERAND_VALTYP_Is_Signed (otype)) {
      maxval = (1 << (osize - 1)) - 1;
      minval = -maxval - 1;
    } else {
      minval = 0;
      maxval = (1 << osize) - 1;
    }
    if (ISA_OPERAND_VALTYP_Is_Literal(otype)
	&& ((osize == 32) || (imm >= minval && imm <= maxval))
	&& ISA_SUBSET_LIST_Member(ISA_SUBSET_List, immform))
      break;
    immform = TOP_get_variant(immform, VARATT_next_immediate);
  }

  /* TODO_MERGE */
  /* return TOP_opnd_immediate_variant_default() */ 

  return immform;
}


/*
 * TOP_opnd_register_variant_default
 *
 * Default target independent implementation for
 * TOP_opnd_register_variant.
 * The implementation is driven by the targinfo variant 
 * description (see isa_variant.cxx).
 * VARATT_register gives for any register or immediate operator a register form.
 * VARATT_next_register gives the next register form if there 
 * are multiple register files matching.
 *
 */
TOP
TOP_opnd_register_variant_default(TOP op, int opnd, ISA_REGISTER_CLASS cl)
{
  TOP regform;

  for (regform = TOP_get_variant(op, VARATT_register);
       regform != TOP_UNDEFINED;
       regform = TOP_get_variant(op, VARATT_next_register)) {
    const ISA_OPERAND_INFO *oinfo = ISA_OPERAND_Info (regform);
    const ISA_OPERAND_VALTYP *otype = ISA_OPERAND_INFO_Operand(oinfo, opnd);
    if (ISA_OPERAND_VALTYP_Is_Register (otype)
	&& (cl == ISA_REGISTER_CLASS_UNDEFINED
	    || cl == ISA_OPERAND_VALTYP_Register_Class(otype))
	&& ISA_SUBSET_LIST_Member(ISA_SUBSET_List, regform))
      return regform;
  }
  return TOP_UNDEFINED;
}

/*
 * TOP_opnd_register_variant
 *
 * Returns the TOP register variant, matching the given register class.
 * Target dependent.
 * The OP may be a register or immediate opcode.
 * OPND is the operand number that we want to be a register.
 * CL is the register class we want, if ISA_REGISTER_CLASS_UNDEFINED then
 * any register class is matched.
 * Returns TOP_UNDEFINED if no register variant is available.
 */
TOP
TOP_opnd_register_variant(TOP op, int opnd, ISA_REGISTER_CLASS cl)
{
  // No target dependent specificities.
  return TOP_opnd_register_variant_default(op, opnd, cl);
}



/*
 * OP_opnd_swapped_variant
 * Returns the TOP corresponding to an invertion of the 2 operands index.
 * For commutative tops on the index, return the same top.
 * For inversible tops, return the inversed top.
 */
TOP
OP_opnd_swapped_variant(OP *op, int opnd1, int opnd2)
{
#define CASE_TOP(top) case TOP_##top##_r_r_r: \
			return TOP_##top##_r_r_r
#define CASE_TOP_INV(top,newtop) case TOP_##top: \
			return TOP_##newtop; \
                      case TOP_##newtop: \
			return TOP_##top
#define CASE_TOP_BR_INV(top,newtop) case TOP_##top##_r_r_r: \
			return TOP_##newtop##_r_r_r; \
		       case TOP_##top##_r_r_b: \
       			return TOP_##newtop##_r_r_b; \
                       case TOP_##newtop##_r_r_r: \
			return TOP_##top##_r_r_r; \
		       case TOP_##newtop##_r_r_b: \
       			return TOP_##top##_r_r_b
#define CASE_TOP_FBR(top) case TOP_##top##_r_r_r: \
			return TOP_##top##_r_r_r; \
		       case TOP_##top##_r_r_b: \
       			return TOP_##top##_r_r_b
#define CASE_TOP_FBR_INV(top,newtop) case TOP_##top##_r_r_r: \
			return TOP_##newtop##_r_r_r; \
		       case TOP_##top##_r_r_b: \
       			return TOP_##newtop##_r_r_b; \
                       case TOP_##newtop##_r_r_r: \
			return TOP_##top##_r_r_r; \
		       case TOP_##newtop##_r_r_b: \
       			return TOP_##top##_r_r_b
#define CASE_TOP_PBR(top) CASE_TOP_FBR(top)

  TOP top = OP_code(op);

  if (opnd1 > opnd2) {
    int tmp = opnd1;
    opnd1 = opnd2;
    opnd2 = tmp;
  }
  const ISA_OPERAND_INFO *oinfo = ISA_OPERAND_Info(top);
  const ISA_OPERAND_VALTYP *otype1 = ISA_OPERAND_INFO_Operand(oinfo, opnd1);
  const ISA_OPERAND_VALTYP *otype2 = ISA_OPERAND_INFO_Operand(oinfo, opnd2);
  // Operands must both be registers, of the same class.
  if (! ISA_OPERAND_VALTYP_Is_Register(otype1)
      || ! ISA_OPERAND_VALTYP_Is_Register(otype2)
      || (ISA_OPERAND_VALTYP_Register_Class(otype1)
	  != ISA_OPERAND_VALTYP_Register_Class(otype2))) {
    return TOP_UNDEFINED;
  }
  if (opnd1 == 0 && opnd2 == 1) {
    if (TOP_is_add(top) || TOP_is_and(top) || TOP_is_or(top)
	|| TOP_is_xor(top) || TOP_is_max(top) || TOP_is_min(top)
	|| TOP_is_fmul(top)) {
      // top is commutative.
      return top;
    }
    if (TOP_is_mul(top)
	&& TOP_opnd_use_bits(top, 0) == TOP_opnd_use_bits(top, 1))
      return top;
    if (TOP_is_cmp(top) && TOP_is_intop(top)) {
      VARIANT v = OP_cmp_variant(op);
      if (v == V_CMP_EQ || v == V_CMP_NE || v == V_CMP_ANDL
	  || v == V_CMP_NANDL || v == V_CMP_ORL || v == V_CMP_NORL) {
	return top;
      }
    }

    switch(top) {
      CASE_TOP(mulhh);
      CASE_TOP(mulhhu);
      CASE_TOP(mul64h);
      CASE_TOP(mul64hu);
      CASE_TOP(mulfrac);
      CASE_TOP_FBR(cmpeqf_n);
      CASE_TOP_BR_INV(cmpge,cmple);
      CASE_TOP_BR_INV(cmpgeu,cmpleu);
      CASE_TOP_BR_INV(cmpgt,cmplt);
      CASE_TOP_BR_INV(cmpgtu,cmpltu);
      CASE_TOP_FBR_INV(cmpgef_n,cmplef_n);
      CASE_TOP_FBR_INV(cmpgtf_n,cmpltf_n);
      case TOP_addcg_b_r_r_b_r: return top;
      case TOP_addso_r_r_r: return top;
      case TOP_adds_r_r_r: return top;
      case TOP_adds_ph_r_r_r: return top;
      case TOP_add_ph_r_r_r: return top;
      case TOP_max_ph_r_r_r: return top;
      case TOP_min_ph_r_r_r: return top;
      case TOP_mulfracrm_ph_r_r_r: return top;
      case TOP_mulfracrne_ph_r_r_r: return top;
      case TOP_mul_ph_r_r_r: return top;
      CASE_TOP_PBR(cmpeq_pb);
      CASE_TOP_PBR(cmpeq_ph);
    }
  } else if (opnd1 == 1 && opnd2 == 2) {
    switch(top) {
      case TOP_slct_r_r_b_r:       return TOP_slctf_r_r_b_r;
      case TOP_st240_slct_r_r_b_r: return TOP_slctf_r_r_b_r;
      case TOP_slctf_r_r_b_r:      return TOP_targ_slct_r_r_b_r;
      CASE_TOP_INV(slctf_pb_r_r_b_r, slct_pb_r_r_b_r);
      case TOP_avg4u_pb_r_r_b_r: return top;
      case TOP_avgu_pb_r_r_b_r: return top;
    }
  }
  return TOP_UNDEFINED;
#undef CASE_TOP
#undef CASE_TOP_INV
#undef CASE_TOP_BR_INV
#undef CASE_TOP_FBR
#undef CASE_TOP_FBR_INV
#undef CASE_TOP_PBR
}

/*
 * TOP_result_register_variant
 * Returns the TOP variant for generating a result in the given register
 * class with the same operand types.
 * The reg form may be a register or immediate opcode.
 * rslt is the result number that may be replaced (0..1).
 */
TOP
TOP_result_register_variant(TOP regform, int rslt, ISA_REGISTER_CLASS regclass)
{
  TOP top = TOP_UNDEFINED;
#define CASE_TOP(x) case TOP_##x##_r_r_b: case TOP_##x##_r_r_r: \
                        top = regclass == ISA_REGISTER_CLASS_branch ? TOP_##x##_r_r_b : TOP_##x##_r_r_r; \
			break; \
		      case TOP_##x##_i_r_b: case TOP_##x##_i_r_r: \
			top = regclass == ISA_REGISTER_CLASS_branch ? TOP_##x##_i_r_b : TOP_##x##_i_r_r; \
			break; \
		      case TOP_##x##_ii_r_b: case TOP_##x##_ii_r_r: \
                        top = regclass == ISA_REGISTER_CLASS_branch ? TOP_##x##_ii_r_b : TOP_##x##_ii_r_r; \
			break

#define CASE_TOPF(x) case TOP_##x##_r_r_r: case TOP_##x##_r_r_b: \
			top = regclass == ISA_REGISTER_CLASS_branch ? TOP_##x##_r_r_b : TOP_##x##_r_r_r; \
			break

  if (rslt == 0) {
    switch(regform) {
      CASE_TOP(cmpeq);
      CASE_TOP(cmpne);
      CASE_TOP(cmpge);
      CASE_TOP(cmpgeu);
      CASE_TOP(cmpgt);
      CASE_TOP(cmpgtu);
      CASE_TOP(cmple);
      CASE_TOP(cmpleu);
      CASE_TOP(cmplt);
      CASE_TOP(cmpltu);
      CASE_TOP(andl);
      CASE_TOP(nandl);
      CASE_TOP(orl);
      CASE_TOP(norl);
      CASE_TOPF(cmpeqf_n);
      CASE_TOPF(cmpgef_n);
      CASE_TOPF(cmpgtf_n);
      CASE_TOPF(cmplef_n);
      CASE_TOPF(cmpltf_n);
      case TOP_convib_r_b:
      case TOP_mtb_r_b: // [SC] Checked mtb
      case TOP_mov_r_b:
      case TOP_st240_mov_r_b:
        top = regclass == ISA_REGISTER_CLASS_branch ? regform : TOP_mov_r_r;
	break;
      case TOP_mov_r_r:
	top = regclass == ISA_REGISTER_CLASS_branch ? TOP_targ_mov_r_b : TOP_mov_r_r;
	break;
      case TOP_convbi_b_r:
      case TOP_mfb_b_r: // [SC] Checked mfb
	top = regclass == ISA_REGISTER_CLASS_integer ? regform : TOP_UNDEFINED;
	break;
      case TOP_mov_b_b:
	top = regclass == ISA_REGISTER_CLASS_branch ? TOP_mov_b_b : TOP_targ_mov_b_r;
	break;
      case TOP_mov_b_r:
      case TOP_st240_mov_b_r:
        top = regclass == ISA_REGISTER_CLASS_integer ? regform : TOP_mov_b_b;
	break;
    default:
      break;
    }
  }
  if (top == TOP_UNDEFINED || !ISA_SUBSET_LIST_Member(ISA_SUBSET_List, top)) {
    top = TOP_UNDEFINED;
  }
  return top;
#undef CASE_TOP
#undef CASE_TOPF
}

/*
 * TOP_opnd_use_bits
 * Return the effective bits used for the given operand.
 * In case of immediate operand, returns the used bits after the
 * optional sign extension.
 * Return -1 for undefined semantic
 */
INT
TOP_opnd_use_bits(TOP top, int opnd)
{
#define CASE_TOP_I(top) case TOP_##top##_r_r_i: case TOP_##top##_r_r_ii
#define CASE_TOP_CI(top) case TOP_##top##_r_b_r_i: case TOP_##top##_r_b_r_ii
#define CASE_TOP_PI(top) case TOP_##top##_p_r_i: case TOP_##top##_p_r_ii
#define CASE_TOP_PCI(top) case TOP_##top##_p_b_r_i: case TOP_##top##_p_b_r_ii
#define CASE_TOP(top) case TOP_##top##_r_r_r: case TOP_##top##_i_r_r: case TOP_##top##_ii_r_r

  int use_bits;
  const ISA_OPERAND_INFO *oinfo;
  const ISA_OPERAND_VALTYP *vtype;
  
  // Default cases depend on register class.
  // ISA_REGISTER_CLASS_gr defaults to 32 bits, signed
  // ISA_REGISTER_CLASS_br default to 1 bit unsigned
  // Non registers default to 32 bit signed
  oinfo = ISA_OPERAND_Info(top);
  vtype = ISA_OPERAND_INFO_Operand(oinfo, opnd);

  use_bits = ISA_OPERAND_VALTYP_Size(vtype);

  if (TOP_is_store (top)
      && opnd == TOP_Find_Operand_Use(top, OU_storeval)) {
    use_bits = TOP_Mem_Bytes (top) * 8;
  }

  switch(top) {
  case TOP_movp:
    use_bits = 64;
    break;
  case TOP_sxth_r_r:
  case TOP_zxth_r_r:
    use_bits =  16;
    break;
  case TOP_sxtb_r_r:
  case TOP_zxtb_r_r:
    use_bits =  8;
    break;
    CASE_TOP(shl):
    CASE_TOP(shr):
    CASE_TOP(shru):
      // This is the size of the bits fetched by the shifts.
      if (opnd == 1) use_bits = 8;
    break;

    CASE_TOP(mulll):
    CASE_TOP(mulllu):
    use_bits = 16;
    break;

    CASE_TOP(mull):
    CASE_TOP(mullu):
    CASE_TOP(mullhus):
      if (opnd == 1) use_bits = 16;
    break;
    
    CASE_TOP(mullh):
    CASE_TOP(mullhu):
      if (opnd == 0) use_bits = 16;
    break;
  }
  
  return use_bits;
#undef CASE_TOP
#undef CASE_TOP_CI
#undef CASE_TOP_PI
#undef CASE_TOP_PCI
#undef CASE_TOP_I
}

/*
 * TOP_opnd_use_signed
 * Returns true if the extension os the effective use bits is
 * signed for the semantic of the TOP.
 * For instance a 32x16->32 unsigned multiply should have the following 
 * properties:
 * TOP_opnd_use_bits(top, opnd1) == 32
 * TOP_opnd_use_signed(top, opnd1) == FALSE
 * TOP_opnd_use_bits(top, opnd2) == 16
 * TOP_opnd_use_signed(top, opnd2) == FALSE
 *
 * Default is to return -1 for undefined semantic.
 */
INT
TOP_opnd_use_signed(TOP top, int opnd)
{
#define CASE_TOP(top) case TOP_##top##_r_r_r: case TOP_##top##_i_r_r: case TOP_##top##_ii_r_r
#define CASE_TOP_BR(top) case TOP_##top##_r_r_r: case TOP_##top##_i_r_r: case TOP_##top##_ii_r_r: \
		       case TOP_##top##_r_r_b: case TOP_##top##_i_r_b: case TOP_##top##_ii_r_b
  INT is_signed;
  const ISA_OPERAND_INFO *oinfo;
  const ISA_OPERAND_VALTYP *vtype;
  ISA_REGISTER_CLASS rc;
  
  // Default cases depend on operand value type.
  oinfo = ISA_OPERAND_Info(top);
  vtype = ISA_OPERAND_INFO_Operand(oinfo, opnd);

  is_signed =  ISA_OPERAND_VALTYP_Is_Signed(vtype);

  switch(top) {
    CASE_TOP(minu):
      CASE_TOP(maxu):
      CASE_TOP_BR(cmpgeu):
      CASE_TOP_BR(cmpgtu):
      CASE_TOP_BR(cmpleu):
      CASE_TOP_BR(cmpltu):
      CASE_TOP(mul64hu):
      is_signed = FALSE;
    break;
    
  case TOP_zxth_r_r:
  case TOP_zxtb_r_r:
    is_signed = FALSE;
    break;
    
    CASE_TOP(shl):
      CASE_TOP(shr):
      // Shift amount is interpreted unsigned
      if (opnd == 1) is_signed =  FALSE;
    break;
    CASE_TOP(shru):
      // Shift amount and shifted value are interpreted unsigned
      is_signed =  FALSE;
    break;
      
    CASE_TOP(mulllu):
      is_signed = FALSE;
    break;
    
    CASE_TOP(mullu):
      is_signed = FALSE;
    break;
  }
  
  return is_signed;
#undef CASE_TOP
#undef CASE_TOP_BR
}

TOP
TOP_AM_automod_variant(TOP top, BOOL post_mod, BOOL inc_mod, ISA_REGISTER_CLASS regclass)
{
  return TOP_UNDEFINED;
}

/*
 * TOP_evaluate_op
 * Return the evaluated expression corresponding to given TOP and
 * the given result number.
 * Return FALSE if not able to evaluate.
 */
BOOL
TOP_evaluate_top( OP *op, INT64 *opnd_values, INT64 *result_val, int result_idx ) {
  return FALSE;
}

/*
 * OP_get_unconditional_variant
 * Return the unpredicated variant if given op.
 * TODO: Make the opposite form to get a predicated variant (if-conversion).
 */
OP *
OP_get_unconditional_variant( OP *op ) {
  return NULL;
}

/*
 * OP_condition_is_true
 * In case of true/false predication model,
 * returns TRUE if predicate not equal to zero, else returns FALSE.
 * On ARM, we have to get the condvariant on the predicate to evaluate
 * if the condition.
 */
BOOL
OP_condition_is_true( OP *op, INT64 pred_val ) {
  return pred_val != 0;
}

INT
targ_cg_TOP_shadd_amount(TOP opcode) {
  switch(opcode) {
  case TOP_sh1add_r_r_r:
  case TOP_sh1add_i_r_r:
  case TOP_sh1add_ii_r_r:
    return 1;
  case TOP_sh2add_r_r_r:
  case TOP_sh2add_i_r_r:
  case TOP_sh2add_ii_r_r:
    return 2;
  case TOP_sh3add_r_r_r:
  case TOP_sh3add_i_r_r:
  case TOP_sh3add_ii_r_r:
    return 3;
  case TOP_sh4add_r_r_r:
  case TOP_sh4add_i_r_r:
  case TOP_sh4add_ii_r_r:
    return 4;
  default:
    return 0;
  }
}

BOOL
targ_cg_TOP_is_shadd(TOP opcode) { 
  return targ_cg_TOP_shadd_amount(opcode) > 0; 
}

static const MulProperties mul_properties [] = {
  //  The entries in this table are carefully ordered, so that
  //  the "best" (i.e. most constrained) instructions are nearer the start
  //  of the table.
  //  This is so that a linear scan in targ_cg_find_mul_with_properties
  //  will find the "best" instruction.

  { TOP_mulllu_r_r_r,   TOP_mulllu_i_r_r,   TOP_mulllu_ii_r_r,  0, 16, 0,  0, 16, 0,   0 },
  { TOP_mulll_r_r_r,    TOP_mulll_i_r_r,    TOP_mulll_ii_r_r,   0, 16, 1,  0, 16, 1,   0 },
  { TOP_mulhhu_r_r_r,   TOP_mulhhu_i_r_r,   TOP_mulhhu_ii_r_r, 16, 16, 0, 16, 16, 0,   0 },
  { TOP_mulhh_r_r_r,    TOP_mulhh_i_r_r,    TOP_mulhh_ii_r_r,  16, 16, 1, 16, 16, 1,   0 },
  { TOP_mullhu_r_r_r,   TOP_mullhu_i_r_r,   TOP_mullhu_ii_r_r,  0, 16, 0, 16, 16, 0,   0 },
  { TOP_mullh_r_r_r,    TOP_mullh_i_r_r,    TOP_mullh_ii_r_r,   0, 16, 1, 16, 16, 1,   0 },
  { TOP_mullu_r_r_r,    TOP_mullu_i_r_r,    TOP_mullu_ii_r_r,   0, 32, 0,  0, 16, 0,   0 },
  { TOP_mull_r_r_r,     TOP_mull_i_r_r,     TOP_mull_ii_r_r,    0, 32, 1,  0, 16, 1,   0 },
  { TOP_mullhus_r_r_r,  TOP_mullhus_i_r_r,  TOP_mullhus_ii_r_r, 0, 32, 1,  0, 16, 0,  32 },
  { TOP_mulhhs_r_r_r,   TOP_mulhhs_i_r_r,   TOP_mulhhs_ii_r_r,  0, 32, 1, 16, 16, 1,  16 },
  { TOP_mulhu_r_r_r,    TOP_mulhu_i_r_r,    TOP_mulhu_ii_r_r,   0, 32, 0, 16, 16, 0,   0 },
  { TOP_mulh_r_r_r,     TOP_mulh_i_r_r,     TOP_mulh_ii_r_r,    0, 32, 1, 16, 16, 1,   0 },
  { TOP_mulhs_r_r_r,    TOP_mulhs_i_r_r,    TOP_mulhs_ii_r_r,   0, 32, 1, 16, 16, 0, -16 },
  { TOP_mul32_r_r_r,    TOP_mul32_i_r_r,    TOP_mul32_ii_r_r,   0, 32, 1,  0, 32, 1,   0 },
  { TOP_mul64hu_r_r_r,  TOP_mul64hu_i_r_r,  TOP_mul64hu_ii_r_r, 0, 32, 0,  0, 32, 0,  32 },
  { TOP_mul64h_r_r_r,   TOP_mul64h_i_r_r,   TOP_mul64h_ii_r_r,  0, 32, 1,  0, 32, 1,  32 },
};

const MulProperties *
targ_cg_mul_properties (const OP *op)
{
  size_t i;
  TOP opcode = OP_code (op);
  for (i = 0; i < (sizeof(mul_properties)/sizeof(mul_properties[0]));
       i++) {
    if (mul_properties[i].r_op == opcode
	|| mul_properties[i].i_op == opcode
	|| mul_properties[i].ii_op == opcode) {
      return &mul_properties[i];
    }
  }
  return NULL;
}

TOP
targ_cg_find_mul_with_properties (const TN *opnd1, const TN *opnd2,
				INT opnd1_shift, INT opnd1_width,
				BOOL opnd1_signed,
				INT opnd2_shift, INT opnd2_width,
				BOOL opnd2_signed,
				INT result_shift,
				BOOL &swap_operands)
{
  size_t i;
  
  for (i = 0;
       i < (sizeof(mul_properties)/sizeof(mul_properties[0]));
       i++) {
    const struct MulProperties *props = &mul_properties[i];
    if (props->result_shift == result_shift) {
      for (int swap = 0; swap < 2; swap++) {
	const TN *o1   = swap ? opnd2        : opnd1;
	const TN *o2   = swap ? opnd1        : opnd2;
	INT o1_shift   = swap ? opnd2_shift  : opnd1_shift;
	INT o1_width   = swap ? opnd2_width  : opnd1_width;
	BOOL o1_signed = swap ? opnd2_signed : opnd1_signed;
	INT o2_shift   = swap ? opnd1_shift  : opnd2_shift;
	INT o2_width   = swap ? opnd1_width  : opnd2_width;
	BOOL o2_signed = swap ? opnd1_signed : opnd2_signed;
	// match if the value is of same signedness and no wider than
	// the multiply operand,
	// OR if the value is unsigned and is narrower than the
	// multiply operand (since an unsigned value can be
	// made into a signed value by adding a leading zero bit).
	if (TN_is_register (o1)
	    && props->opnd1_shift == o1_shift
	    && ((props->opnd1_signed == o1_signed
		 && props->opnd1_width >= o1_width)
		|| (! o1_signed && props->opnd1_width > o1_width))
	    && props->opnd2_shift == o2_shift
	    && ((props->opnd2_signed == o2_signed
		 && props->opnd2_width >= o2_width)
		|| (! o2_signed && props->opnd2_width > o2_width))) {
	  TOP new_opcode = props->r_op;
	  if (TN_has_value (o2))
	    new_opcode = TOP_opnd_immediate_variant (new_opcode, 1,
						     TN_value (o2));
	  else
	    FmtAssert (TN_is_register (o2),
		       ("make_mul_from_properties unexpected opnd2"));
	  if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, new_opcode)) {
	    swap_operands = swap ? TRUE : FALSE;
	    return new_opcode;
	  }
	}
      }
    }
  }
  return TOP_UNDEFINED;
}

void
targ_cg_init_targ_op ()
{
#if 0
  // For debugging purpose
  INT i;
  for (i = 0; i < TOP_count; i++) {
    TOP top = (TOP)i;
    if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, top)) {
      printf ("TOP_opnd_swapped_variant(%s, 0, 1) = %s\n",
	      TOP_Name (top), TOP_Name(TOP_opnd_swapped_variant (top, 0, 1)));
    }
  }
  for (i = 0; i < TOP_count; i++) {
    TOP top = (TOP)i;
    if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, top)) {
      printf ("TOP_opnd_swapped_variant(%s, 1, 2) = %s\n",
	      TOP_Name (top), TOP_Name(TOP_opnd_swapped_variant (top, 1, 2)));
    }
  }
  for (i = 0; i < TOP_count; i++) {
    TOP top = (TOP)i;
    if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, top)) {
      printf ("TOP_result_register_variant(%s, 0, ISA_REGISTER_CLASS_integer) = %s\n",
	      TOP_Name (top), TOP_Name(TOP_result_register_variant (top, 0, ISA_REGISTER_CLASS_integer)));
    }
  }
  for (i = 0; i < TOP_count; i++) {
    TOP top = (TOP)i;
    if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, top)) {
      printf ("TOP_result_register_variant(%s, 0, ISA_REGISTER_CLASS_branch) = %s\n",
	      TOP_Name (top), TOP_Name(TOP_result_register_variant (top, 0, ISA_REGISTER_CLASS_branch)));
    }
  }
  for (i = 0; i < TOP_count; i++) {
    TOP top = (TOP)i;
    if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, top)) {
      const ISA_OPERAND_INFO *oinfo = ISA_OPERAND_Info(top);
      for (INT j = 0; j < ISA_OPERAND_INFO_Operands(oinfo); j++) {
	printf ("TOP_opnd_use_bits(%s, %d) = %d\n",
		TOP_Name (top), j, TOP_opnd_use_bits(top, j));
      }
    }
  }
  for (i = 0; i < TOP_count; i++) {
    TOP top = (TOP)i;
    if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, top)) {
      const ISA_OPERAND_INFO *oinfo = ISA_OPERAND_Info(top);
      for (INT j = 0; j < ISA_OPERAND_INFO_Operands(oinfo); j++) {
	printf ("TOP_opnd_immediate_variant(%s, %d, 0) = %s\n",
		TOP_Name (top), j, TOP_Name (TOP_opnd_immediate_variant(top, j, 0)));
	printf ("TOP_opnd_immediate_variant(%s, %d, 1000) = %s\n",
	      TOP_Name (top), j, TOP_Name (TOP_opnd_immediate_variant(top, j, 1000)));
      }
    }
  }
#endif
	    
}
