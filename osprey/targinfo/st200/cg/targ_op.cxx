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
 *   OP_is_counted_loop
 * ====================================================================
 */
BOOL 
OP_Is_Counted_Loop(OP *op) 
{
  return FALSE;
}

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
    if (OP_fdiv(op)) return FALSE;
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
  case TOP_add_r:
  case TOP_sub_r:
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
  case TOP_add_r:
    return TOP_sub_r;

  case TOP_sub_r:
    return TOP_add_r;
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
  case TOP_add_r:
    return TOP_add_i;

  case TOP_sub_r:
    return TOP_sub_i;
  }

  return TOP_UNDEFINED;
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
  case TOP_add_r:
    return (opnd == 0 || opnd == 1);

  case TOP_sub_r:
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
  case TOP_add_r:
  case TOP_sub_r:
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
    Set_OP_cond_def_kind(op, OP_ALWAYS_UNC_DEF);
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

