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

/* ====================================================================
 *   OP_Is_Advanced_Load
 * ====================================================================
 */
BOOL
OP_Is_Advanced_Load( OP *memop )
{
  if (!OP_load(memop)) return FALSE;
  if (TOP_is_dismissible(OP_code(memop))) return TRUE;

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
  TOP opcode = OP_code(op);

  /* not allowed to speculate anything. */
  if (Eager_Level == EAGER_NONE) return FALSE;

  /* don't speculate volatile memory references. */
  if (OP_volatile(op)) return FALSE;

  switch (Eager_Level) {
    //  case EAGER_NONE:

    /* not allowed to speculate anything
     */
    //    break;

  case EAGER_SAFE:

    /* Only exception-safe speculative ops are allowed
     */
#if 0
    /* Arthur: when we add these to semantics - enable */
    if (TOP_is_ftrap(opcode) || TOP_is_itrap(opcode)) return FALSE;
#endif
    /*FALLTHROUGH*/

  case EAGER_ARITH:

    /* Arithmetic exceptions allowed
     */
    if (TOP_is_fdiv(opcode)) return FALSE;
    /*FALLTHROUGH*/

  case EAGER_DIVIDE:

    /* Divide by zero exceptions allowed 
     */
#if 0
    /* Arthur: when we add these to semantics - enable */
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
  if (TN_is_symbol(OP_opnd(op, 1)) &&
      !ST_is_weak_symbol(TN_var(OP_opnd(op, 1)))) goto scalar_load;

  /*  c) load of a fixed variable (base address is constant or
   *     known to be in bounds), comment out the rematerizable bit check 
   *     since it doesn;t guarantee safeness all the time.
   */
#if 0
  /* Arthur: this should be checked at the call site !! */
  if (/*   TN_is_rematerializable(OP_opnd(op, 0)) || */
      (   (wn = Get_WN_From_Memory_OP(op))
	  && Alias_Manager->Safe_to_speculate(wn))) goto scalar_load;
#endif

  /* d) speculative, advanced, speculative-advanced loads are safe to 
   *    speculate. 
   */
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
  TN* pred_tn
)
{
  FmtAssert(FALSE,("CGTARG_Predicate_OP: target does not support predication"));
}

/* ====================================================================
 *   OP_Copy_Operand
 *
 *   TODO: generate automatically ?? at leats some obvious ones
 *         coherently with the isa property ?
 * ====================================================================
 */
INT 
OP_Copy_Operand (
  OP *op
)
{
  TOP opcode = OP_code(op);

  if (OP_iadd(op) || OP_ior(op) || OP_ixor(op)) {

    if (opcode == TOP_spadjust) {
      return -1;
    }

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

  switch (opcode) {
  case TOP_mov_r:
  case TOP_mov_i:
  case TOP_mov_ii:
    return 0;

  }

  return -1;
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
BOOL OP_save_predicates(OP *op) { return FALSE; }
BOOL OP_restore_predicates(OP *op) { return FALSE; }

#if 0
/* ====================================================================
 *   CGTARG_Immediate_Operand
 *
 *   TODO: generate automatically. put into targ_op.h as inlined
 * ====================================================================
 */
INT
CGTARG_Immediate_Operand (
  OP *op
)
{
  switch (OP_code(op)) {
  case TOP_add_i:   case TOP_add_ii:
  case TOP_shl_i:   case TOP_shl_ii:
  case TOP_shr_i:   case TOP_shr_ii:
  case TOP_shru_i:   case TOP_shru_ii:
  case TOP_sh1add_i:   case TOP_sh1add_ii:
  case TOP_sh2add_i:   case TOP_sh2add_ii:
  case TOP_sh3add_i:   case TOP_sh3add_ii:
  case TOP_sh4add_i:   case TOP_sh4add_ii:
  case TOP_and_i:   case TOP_and_ii:
  case TOP_andc_i:   case TOP_andc_ii:
  case TOP_or_i:   case TOP_or_ii:
  case TOP_orc_i:   case TOP_orc_ii:
  case TOP_xor_i:   case TOP_xor_ii:
  case TOP_max_i:   case TOP_max_ii:
  case TOP_maxu_i:   case TOP_maxu_ii:
  case TOP_min_i:   case TOP_min_ii:
  case TOP_minu_i:   case TOP_minu_ii:
  case TOP_mull_i:   case TOP_mull_ii:
  case TOP_mullu_i:   case TOP_mullu_ii:
  case TOP_mulh_i:   case TOP_mulh_ii:
  case TOP_mulhu_i:   case TOP_mulhu_ii:
  case TOP_mulll_i:   case TOP_mulll_ii:
  case TOP_mulllu_i:   case TOP_mulllu_ii:
  case TOP_mullh_i:   case TOP_mullh_ii:
  case TOP_mullhu_i:   case TOP_mullhu_ii:
  case TOP_mulhh_i:   case TOP_mulhh_ii:
  case TOP_mulhhu_i:   case TOP_mulhhu_ii:
  case TOP_mulhs_i:   case TOP_mulhs_ii:
  case TOP_cmpeq_i_r:   case TOP_cmpeq_ii_r:
  case TOP_cmpne_i_r:   case TOP_cmpne_ii_r:
  case TOP_cmpge_i_r:   case TOP_cmpge_ii_r:
  case TOP_cmpgeu_i_r:   case TOP_cmpgeu_ii_r:
  case TOP_cmpgt_i_r:   case TOP_cmpgt_ii_r:
  case TOP_cmpgtu_i_r:   case TOP_cmpgtu_ii_r:
  case TOP_cmple_i_r:   case TOP_cmple_ii_r:
  case TOP_cmpleu_i_r:   case TOP_cmpleu_ii_r:
  case TOP_cmplt_i_r:   case TOP_cmplt_ii_r:
  case TOP_cmpltu_i_r:   case TOP_cmpltu_ii_r:
  case TOP_andl_i_r:   case TOP_andl_ii_r:
  case TOP_nandl_i_r:   case TOP_nandl_ii_r:
  case TOP_orl_i_r:   case TOP_orl_ii_r:
  case TOP_norl_i_r:   case TOP_norl_ii_r:
  case TOP_cmpeq_i_b:   case TOP_cmpeq_ii_b:
  case TOP_cmpne_i_b:   case TOP_cmpne_ii_b:
  case TOP_cmpge_i_b:   case TOP_cmpge_ii_b:
  case TOP_cmpgeu_i_b:   case TOP_cmpgeu_ii_b:
  case TOP_cmpgt_i_b:   case TOP_cmpgt_ii_b:
  case TOP_cmpgtu_i_b:   case TOP_cmpgtu_ii_b:
  case TOP_cmple_i_b:   case TOP_cmple_ii_b:
  case TOP_cmpleu_i_b:   case TOP_cmpleu_ii_b:
  case TOP_cmplt_i_b:   case TOP_cmplt_ii_b:
  case TOP_cmpltu_i_b:   case TOP_cmpltu_ii_b:
  case TOP_andl_i_b:   case TOP_andl_ii_b:
  case TOP_nandl_i_b:   case TOP_nandl_ii_b:
  case TOP_orl_i_b:   case TOP_orl_ii_b:
  case TOP_norl_i_b:   case TOP_norl_ii_b:
    return 1;

  case TOP_slct_i:   case TOP_slct_ii:
  case TOP_slctf_i:   case TOP_slctf_ii:
    return 2;

  case TOP_ldw_i:   case TOP_ldw_ii:
  case TOP_ldw_d_i:   case TOP_ldw_d_ii:
  case TOP_ldh_i:   case TOP_ldh_ii:
  case TOP_ldh_d_i:   case TOP_ldh_d_ii:
  case TOP_ldhu_i:   case TOP_ldhu_ii:
  case TOP_ldhu_d_i:   case TOP_ldhu_d_ii:
  case TOP_ldb_i:   case TOP_ldb_ii:
  case TOP_ldb_d_i:   case TOP_ldb_d_ii:
  case TOP_ldbu_i:   case TOP_ldbu_ii:
  case TOP_ldbu_d_i:   case TOP_ldbu_d_ii:
    return 0;

  case TOP_stw_i:
  case TOP_sth_i:
  case TOP_stb_i:
    return 0;

  case TOP_goto:
    return 0;

  case TOP_br:
  case TOP_brf:
    return 1;

  case TOP_sub_i:   case TOP_sub_ii:
  case TOP_mov_i:   case TOP_mov_ii:
    return 0;

  }

  return -1;
}
#endif

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
    Set_OP_cond_def_kind(op, OP_ALWAYS_UNC_DEF);
  }
}

