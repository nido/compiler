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
 *   OP_Is_Barrier
 *
 *   TODO: Perhaps it can be done by associating a TOP property ?
 * ====================================================================
 */
BOOL OP_Is_Barrier(OP *op) 
{
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
  if (OP_dismissible(memop)) return TRUE;

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
  if (OP_has_predicate(op)) {
    Set_OP_opnd(op, OP_PREDICATE_OPND, pred_tn);
  }
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
  TOP opr = OP_code(op);
  switch (opr) {

  // NOTE: TOP_fandcm, TOP_for, and TOP_fxor could be handled like
  // their integer counterparts should that ever become useful.

  case TOP_GP32_ADD_GT_DR_DR_U8:
    //case TOP_or:
    //case TOP_xor:
    //case TOP_sub:
    //case TOP_shl_i:
    //case TOP_shr_i:
    if (TN_has_value(OP_opnd(op,2)) && TN_value(OP_opnd(op,2)) == 0) {
      return 1;
    }
    break;

  case TOP_GP32_COPYA_GT_AR_DR:
  case TOP_GP32_COPYC_GT_CRL_DR:
  case TOP_GP32_COPYD_GT_DR_AR:

  case TOP_GP32_MOVEA_GT_AR_AR:
  case TOP_GP32_MOVEG_GT_BR_BR:
  case TOP_GP32_MOVE_GT_DR_DR:
    //TOP_GP32_MOVEHH_GT_DR_DR,
    //TOP_GP32_MOVEHL_GT_DR_DR,
    //TOP_GP32_MOVELH_GT_DR_DR,
    //TOP_GP32_MOVELL_GT_DR_DR,
    //TOP_GP32_MOVEP_GT_DR_DR,
    return 1;

  }
  return -1;
}

/* ====================================================================
 *   CGTARG_Noop_Top
 * ====================================================================
 */
TOP 
CGTARG_Noop_Top (ISA_EXEC_UNIT_PROPERTY unit) { return TOP_GP32_NOP; } 

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

#if 0
  switch (top) {
  case TOP_add_r:
  case TOP_sub_r:
    return true;
  }
#endif
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
#if 0
  switch (top) {
  case TOP_add_r:
    return TOP_sub_r;

  case TOP_sub_r:
    return TOP_add_r;
  }
#endif

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
#if 0
  switch (top) {
  case TOP_add_r:
    return TOP_add_i;

  case TOP_sub_r:
    return TOP_sub_i;
  }
#endif

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
  FmtAssert( FALSE, ( "CGTARG_Which_OP_Select: not implemented") );
  /*NOTREACHED*/
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

#if 0
  switch (top) {
  case TOP_add_r:
    return (opnd == 0 || opnd == 1);

  case TOP_sub_r:
    return (opnd == 0);
  }
#endif

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
#if 0
  switch (OP_code(op)) {
  case TOP_add_r:
  case TOP_sub_r:
    if (this_opnd == 1) return 0;
    if (this_opnd == 0) return 1;
    break;
  }
#endif
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

    // The following OPs unconditionally define the predicate results.
    //
  case TOP_noop:

    Set_OP_cond_def_kind(op, OP_ALWAYS_UNC_DEF);
    break;

    // The following OPs do not always update the result predicates.
  case TOP_label:

    Set_OP_cond_def_kind(op, OP_ALWAYS_COND_DEF);
    break;

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
  return FALSE;
}

#if !defined(GHS_SHIT_IS_WORKING)
/* ====================================================================
 *   RegMask_pretty
 *
 *   Funsciton borrowed from LAO to hack until the GHS crap as can take
 *   the ST100 instructions. probably forever ...
 * ====================================================================
 */
static void RegMask_pretty (
  INT64 mask, 
  BOOL push,      /* is it a push ? if not it's a pop */
  FILE *file
)
{
  INT i;
  INT rl = -1, rh;
  ISA_REGISTER_CLASS rc = ISA_REGISTER_CLASS_UNDEFINED;
  BOOL first = TRUE;

  // push/pop operand is a u20
  for (i = 0; i < 20; i++) {
    INT r1, r2;
    ISA_REGISTER_CLASS cl;

    // check whether the bit is set
    if (!(mask & (1 << i))) continue;

    // find out which ISA_REGISTER_CLASS and REG it corresponds to:
    if (push) {
      switch (i) {
      case 0:
	r1 = 0; r2 = 1;
	cl = ISA_REGISTER_CLASS_du;
	break;
      case 1:
	r1 = 2; r2 = 3;
	cl = ISA_REGISTER_CLASS_du;
	break;
      case 2:
	r1 = 4; r2 = 5;
	cl = ISA_REGISTER_CLASS_du;
	break;
      case 3:
	r1 = 6; r2 = 7;
	cl = ISA_REGISTER_CLASS_du;
	break;
      case 4:
	r1 = 8; r2 = 9;
	cl = ISA_REGISTER_CLASS_du;
	break;
      case 5:
	r1 = 10; r2 = 11;
	cl = ISA_REGISTER_CLASS_du;
	break;
      case 6:
	r1 = 12; r2 = 13;
	cl = ISA_REGISTER_CLASS_du;
	break;
      case 7:
	r1 = 14; r2 = 15;
	cl = ISA_REGISTER_CLASS_du;
	break;
      case 8:
	r1 = 0; r2 = 0;
	cl = ISA_REGISTER_CLASS_control;
	break;
      case 9:
	r1 = 0; r2 = 0;
	cl = ISA_REGISTER_CLASS_loop;
	break;
      case 10:
	r1 = 1; r2 = 1;
	cl = ISA_REGISTER_CLASS_loop;
	break;
      case 11:
	r1 = 2; r2 = 2;
	cl = ISA_REGISTER_CLASS_loop;
	break;
      case 12:
	r1 = 0; r2 = 1;
	cl = ISA_REGISTER_CLASS_au;
	break;
      case 13:
	r1 = 2; r2 = 3;
	cl = ISA_REGISTER_CLASS_au;
	break;
      case 14:
	r1 = 4; r2 = 5;
	cl = ISA_REGISTER_CLASS_au;
	break;
      case 15:
	r1 = 6; r2 = 7;
	cl = ISA_REGISTER_CLASS_au;
	break;
      case 16:
	r1 = 8; r2 = 9;
	cl = ISA_REGISTER_CLASS_au;
	break;
      case 17:
	r1 = 10; r2 = 11;
	cl = ISA_REGISTER_CLASS_au;
	break;
      case 18:
	r1 = 12; r2 = 13;
	cl = ISA_REGISTER_CLASS_au;
	break;
      case 19:
	r1 = 14; r2 = 15;
	cl = ISA_REGISTER_CLASS_au;
	break;
      }
    }
    else { // pop
      switch (i) {
      case 19:
	r1 = 0; r2 = 1;
	cl = ISA_REGISTER_CLASS_du;
	break;
      case 18:
	r1 = 2; r2 = 3;
	cl = ISA_REGISTER_CLASS_du;
	break;
      case 17:
	r1 = 4; r2 = 5;
	cl = ISA_REGISTER_CLASS_du;
	break;
      case 16:
	r1 = 6; r2 = 7;
	cl = ISA_REGISTER_CLASS_du;
	break;
      case 15:
	r1 = 8; r2 = 9;
	cl = ISA_REGISTER_CLASS_du;
	break;
      case 14:
	r1 = 10; r2 = 11;
	cl = ISA_REGISTER_CLASS_du;
	break;
      case 13:
	r1 = 12; r2 = 13;
	cl = ISA_REGISTER_CLASS_du;
	break;
      case 12:
	r1 = 14; r2 = 15;
	cl = ISA_REGISTER_CLASS_du;
	break;
      case 11:
	r1 = 0; r2 = 0;
	cl = ISA_REGISTER_CLASS_control;
	break;
      case 10:
	r1 = 0; r2 = 0;
	cl = ISA_REGISTER_CLASS_loop;
	break;
      case 9:
	r1 = 1; r2 = 1;
	cl = ISA_REGISTER_CLASS_loop;
	break;
      case 8:
	r1 = 2; r2 = 2;
	cl = ISA_REGISTER_CLASS_loop;
	break;
      case 7:
	r1 = 0; r2 = 1;
	cl = ISA_REGISTER_CLASS_au;
	break;
      case 6:
	r1 = 2; r2 = 3;
	cl = ISA_REGISTER_CLASS_au;
	break;
      case 5:
	r1 = 4; r2 = 5;
	cl = ISA_REGISTER_CLASS_au;
	break;
      case 4:
	r1 = 6; r2 = 7;
	cl = ISA_REGISTER_CLASS_au;
	break;
      case 3:
	r1 = 8; r2 = 9;
	cl = ISA_REGISTER_CLASS_au;
	break;
      case 2:
	r1 = 10; r2 = 11;
	cl = ISA_REGISTER_CLASS_au;
	break;
      case 1:
	r1 = 12; r2 = 13;
	cl = ISA_REGISTER_CLASS_au;
	break;
      case 0:
	r1 = 14; r2 = 15;
	cl = ISA_REGISTER_CLASS_au;
	break;
      }
    }

    // Now, it depends on ISA_REGISTER_CLASS
    if (cl == ISA_REGISTER_CLASS_du) {
      if (!first) fprintf (file, ", ");
      fprintf(file, "r%d-r%d",r1,r2);
      first = false;
    }
    else if (cl == ISA_REGISTER_CLASS_au) {
      if (!first) fprintf (file, ", ");
      fprintf(file, "p%d-p%d",r1,r2);
      first = false;
    }
    else if (cl == ISA_REGISTER_CLASS_loop) {
      if (!first) fprintf (file, ", ");
      fprintf(file, "LR%d", r1);
      first = false;
    }
    else if (cl == ISA_REGISTER_CLASS_control) {
      if (!first) fprintf (file, ", ");
      fprintf(file, "GFR");
      first = false;
    }
    else {
      FmtAssert(FALSE,("Hack_For_Printing_Push_Pop: unknown rclass"));
    }
  }
}

BOOL
Hack_For_Printing_Push_Pop (
  OP *op,
  FILE *file
)
{

  if (OP_code(op) == TOP_GP32_PUSH_U20 || OP_code(op) == TOP_GP32_POP_U20) {
    fputc ('\t\t', file);
    if (OP_code(op) == TOP_GP32_PUSH_U20) {
      fprintf (file, " push ");
      RegMask_pretty(TN_value(OP_opnd(op,0)), TRUE, file);
    }
    else if (OP_code(op) == TOP_GP32_POP_U20) {
      fprintf (file, " pop ");
      RegMask_pretty(TN_value(OP_opnd(op,0)), FALSE, file);
    }	
    fputc ('\n', file);
    return TRUE;
  }

  return FALSE;
}
#endif
