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

/* ====================================================================
 *   OP_Is_Barrier
 *
 *   TODO: Perhaps it can be done by associating a TOP property ?
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

  switch (top) {
  case TOP_prgadd:	/* data cache purge */
  case TOP_prgset:	/* data cache purge */
  case TOP_sync:	/* data cache sync */
  case TOP_prgins:	/* i-cache purge */
    /*case TOP_syncins: */	/* i-cache sync. */
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

  // can never speculate a call.
  if (OP_call (op))
    return FALSE;

  // [CG] Not clear if we can speculate an asm
  if (OP_code(op) == TOP_asm) return FALSE;

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
  TN* pred_tn
)
{
  FmtAssert(FALSE,("CGTARG_Predicate_OP: target does not support predication"));
}

/* ====================================================================
 *   OP_Copy_Operand
 *
 *   Test and return operand for copy operations.
 *   Return the operand index if the operation is a copy from an
 *   immediate value or a register to a register of the same class.
 *   Returns -1 if the operation is not a copy.
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

  // [CG]: Some ops may be marked as copy
  // while they are part of a sequence.
  // This treats the branch copy which is:
  // mov_i t, -1
  // addcg r0, dst = r0, t, src (copy)
  if (OP_copy(op)) {
    if (opcode == TOP_addcg) return 2;
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
    if (OP_code(op) == TOP_addcg) return 1;
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
  if (!is_float && !is_fcc) return TOP_slct_r;
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
  int s9 = 0;
  if (imm >= -(1<<8) && imm < (1<<8)) s9 = 1;

#define CASE_TOP_I(top) case TOP_##top##_i: case TOP_##top##_ii: \
			return s9 ? TOP_##top##_i : TOP_##top##_ii;
#define CASE_TOP(top) case TOP_##top##_r: case TOP_##top##_i: case TOP_##top##_ii: \
			return s9 ? TOP_##top##_i : TOP_##top##_ii;
#define CASE_TOP_BR(top) case TOP_##top##_r_r: case TOP_##top##_i_r: case TOP_##top##_ii_r: \
			return s9 ? TOP_##top##_i_r : TOP_##top##_ii_r; \
		       case TOP_##top##_r_b: case TOP_##top##_i_b: case TOP_##top##_ii_b: \
			return s9 ? TOP_##top##_i_b : TOP_##top##_ii_b;

  if (opnd == 0) {
    switch(regform) {
      CASE_TOP(mov);
      CASE_TOP(sub);
      CASE_TOP_I(stw);
      CASE_TOP_I(sth);
      CASE_TOP_I(stb);
      CASE_TOP_I(ldw);
      CASE_TOP_I(ldh);
      CASE_TOP_I(ldb);
      CASE_TOP_I(ldhu);
      CASE_TOP_I(ldbu);
      CASE_TOP_I(ldw_d);
      CASE_TOP_I(ldh_d);
      CASE_TOP_I(ldb_d);
      CASE_TOP_I(ldhu_d);
      CASE_TOP_I(ldbu_d);
      CASE_TOP_I(pft);
    }
  } else if (opnd == 1) {
    switch(regform) {
      CASE_TOP(add);
      CASE_TOP(shl);
      CASE_TOP(shr);
      CASE_TOP(shru);
      CASE_TOP(sh1add);
      CASE_TOP(sh2add);
      CASE_TOP(sh3add);
      CASE_TOP(sh4add);
      CASE_TOP(and);
      CASE_TOP(andc);
      CASE_TOP(or);
      CASE_TOP(orc);
      CASE_TOP(xor);
      CASE_TOP(mullhus);
      CASE_TOP(max);
      CASE_TOP(maxu);
      CASE_TOP(min);
      CASE_TOP(minu);
      CASE_TOP(mulhhs);
      CASE_TOP(mull);
      CASE_TOP(mullu);
      CASE_TOP(mulh);
      CASE_TOP(mulhu);
      CASE_TOP(mulll);
      CASE_TOP(mulllu);
      CASE_TOP(mullh);
      CASE_TOP(mullhu);
      CASE_TOP(mulhh);
      CASE_TOP(mulhhu);
      CASE_TOP(mulhs);
      CASE_TOP_BR(cmpeq);
      CASE_TOP_BR(cmpne);
      CASE_TOP_BR(cmpge);
      CASE_TOP_BR(cmpgeu);
      CASE_TOP_BR(cmpgt);
      CASE_TOP_BR(cmpgtu);
      CASE_TOP_BR(cmple);
      CASE_TOP_BR(cmpleu);
      CASE_TOP_BR(cmplt);
      CASE_TOP_BR(cmpltu);
      CASE_TOP_BR(andl);
      CASE_TOP_BR(nandl);
      CASE_TOP_BR(orl);
      CASE_TOP_BR(norl);
    }
  } else if (opnd == 2) {
    switch(regform) {
      CASE_TOP(slct);
      CASE_TOP(slctf);
    }
  }
  return TOP_UNDEFINED;
#undef CASE_TOP
#undef CASE_TOP_I
#undef CASE_TOP_BR
}

/*
 * TOP_opnd_swapped_variant
 * Returns the TOP corresponding to an invertion of the 2 operands index.
 * For commutative tops on the index, return the same top.
 * For inversible tops, return the inversed top.
 */
TOP
TOP_opnd_swapped_variant(TOP top, int opnd1, int opnd2)
{
#define CASE_TOP(top) case TOP_##top##_r: \
			return TOP_##top##_r;
#define CASE_TOP_INV(top,newtop) case TOP_##top##_r: \
			return TOP_##newtop##_r;
#define CASE_TOP_BR(top) case TOP_##top##_r_r: \
			return TOP_##top##_r_r; \
		       case TOP_##top##_r_b: \
       			return TOP_##top##_r_b;
#define CASE_TOP_BR_INV(top,newtop) case TOP_##top##_r_r: \
			return TOP_##newtop##_r_r; \
		       case TOP_##top##_r_b: \
       			return TOP_##newtop##_r_b;

  if (opnd1 == 0 && opnd2 == 1) {
    switch(top) {
      CASE_TOP(add);
      CASE_TOP(and);
      CASE_TOP(or);
      CASE_TOP(xor);
      CASE_TOP(max);
      CASE_TOP(maxu);
      CASE_TOP(min);
      CASE_TOP(minu);
      CASE_TOP(mulll);
      CASE_TOP(mulllu);
      CASE_TOP(mulhh);
      CASE_TOP(mulhhu);
      CASE_TOP_BR(cmpeq);
      CASE_TOP_BR(cmpne);
      CASE_TOP_BR(andl);
      CASE_TOP_BR(orl);
      CASE_TOP_BR_INV(cmpge,cmple);
      CASE_TOP_BR_INV(cmpgeu,cmpleu);
      CASE_TOP_BR_INV(cmpgt,cmplt);
      CASE_TOP_BR_INV(cmpgtu,cmpltu);
      CASE_TOP_BR_INV(cmple,cmpge);
      CASE_TOP_BR_INV(cmpleu,cmpgeu);
      CASE_TOP_BR_INV(cmplt,cmpgt);
      CASE_TOP_BR_INV(cmpltu,cmpgtu);
    }
  } else if (opnd1 == 1 && opnd2 == 2) {
    switch(top) {
      CASE_TOP_INV(slct, slctf);
      CASE_TOP_INV(slctf, slct);
    }
  }
  return TOP_UNDEFINED;
#undef CASE_TOP
#undef CASE_TOP_INV
#undef CASE_TOP_BR
#undef CASE_TOP_BR_INV
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

#define CASE_TOP(top) case TOP_##top##_r_b: case TOP_##top##_r_r: \
			return regclass == ISA_REGISTER_CLASS_branch ? TOP_##top##_r_b : TOP_##top##_r_r; \
		      case TOP_##top##_i_b: case TOP_##top##_i_r: \
			return regclass == ISA_REGISTER_CLASS_branch ? TOP_##top##_i_b : TOP_##top##_i_r; \
		      case TOP_##top##_ii_b: case TOP_##top##_ii_r: \
			return regclass == ISA_REGISTER_CLASS_branch ? TOP_##top##_ii_b : TOP_##top##_ii_r; \

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
      case TOP_mtb:
      case TOP_mov_r:
	return regclass == ISA_REGISTER_CLASS_branch ? TOP_mtb : TOP_mov_r;
      case TOP_mfb:
	return regclass == ISA_REGISTER_CLASS_branch ? TOP_UNDEFINED : TOP_mfb;
    }
  }
  return TOP_UNDEFINED;
#undef CASE_TOP
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
#define CASE_TOP_I(top) case TOP_##top##_i: case TOP_##top##_ii
#define CASE_TOP(top) case TOP_##top##_r: case TOP_##top##_i: case TOP_##top##_ii
#define CASE_TOP_BR(top) case TOP_##top##_r_r: case TOP_##top##_i_r: case TOP_##top##_ii_r: \
		       case TOP_##top##_r_b: case TOP_##top##_i_b: case TOP_##top##_ii_b
  switch(top) {
    CASE_TOP(sub):
      CASE_TOP(add):
      CASE_TOP(sh1add):
      CASE_TOP(sh2add):
      CASE_TOP(sh3add):
      CASE_TOP(sh4add):
      CASE_TOP(and):
      CASE_TOP(andc):
      CASE_TOP(or):
      CASE_TOP(orc):
      CASE_TOP(xor):
      CASE_TOP(max):
      CASE_TOP(maxu):
      CASE_TOP(min):
      CASE_TOP(minu):
      CASE_TOP_BR(cmpeq):
      CASE_TOP_BR(cmpne):
      CASE_TOP_BR(cmpge):
      CASE_TOP_BR(cmpgeu):
      CASE_TOP_BR(cmpgt):
      CASE_TOP_BR(cmpgtu):
      CASE_TOP_BR(cmple):
      CASE_TOP_BR(cmpleu):
      CASE_TOP_BR(cmplt):
      CASE_TOP_BR(cmpltu):
      CASE_TOP_BR(andl):
      CASE_TOP_BR(nandl):
      CASE_TOP_BR(orl):
      CASE_TOP_BR(norl):
      CASE_TOP_I(ldw):
      CASE_TOP_I(ldh):
      CASE_TOP_I(ldb):
      CASE_TOP_I(ldhu):
      CASE_TOP_I(ldbu):
      CASE_TOP_I(ldw_d):
      CASE_TOP_I(ldh_d):
      CASE_TOP_I(ldb_d):
      CASE_TOP_I(ldhu_d):
      CASE_TOP_I(ldbu_d):
      CASE_TOP_I(pft):
      return 32;
    
    CASE_TOP(slct):
      CASE_TOP(slctf):
      return opnd == 0 ? 1: 32;
    
    CASE_TOP(mov):
  case TOP_bswap_r:
  case TOP_mtb:
      return 32;
    
  case TOP_sxth_r:
  case TOP_zxth_r:
    return 16;
  case TOP_sxtb_r:
    return 8;
    
  case TOP_mfb:
    return 1;
  case TOP_addcg:
  case TOP_divs:
    return opnd == 2 ? 1: 32;
    
    CASE_TOP(shl):
      CASE_TOP(shr):
      CASE_TOP(shru):
      return opnd == 0 ? 32: 8;
    

    CASE_TOP(mulll):
    CASE_TOP(mulllu):
    return 16;

    CASE_TOP(mull):
    CASE_TOP(mullu):
    CASE_TOP(mullhus):
      return opnd == 0 ? 32: 16;

    CASE_TOP(mullh):
    CASE_TOP(mullhu):
      return opnd == 0 ? 16: 32;


    CASE_TOP(mulh):
    CASE_TOP(mulhh):
    CASE_TOP(mulhhs):
    CASE_TOP(mulhhu):
    CASE_TOP(mulhs):
    CASE_TOP(mulhu):
      return 32;

    CASE_TOP_I(stw):
      return opnd == 2 ? 32: 32;
    CASE_TOP_I(sth):
      return opnd == 2 ? 16: 32;
    CASE_TOP_I(stb):
      return opnd == 2 ? 8: 32;
  }
  
  return -1;
#undef CASE_TOP
#undef CASE_TOP_I
#undef CASE_TOP_BR
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
#define CASE_TOP_I(top) case TOP_##top##_i: case TOP_##top##_ii
#define CASE_TOP(top) case TOP_##top##_r: case TOP_##top##_i: case TOP_##top##_ii
#define CASE_TOP_BR(top) case TOP_##top##_r_r: case TOP_##top##_i_r: case TOP_##top##_ii_r: \
		       case TOP_##top##_r_b: case TOP_##top##_i_b: case TOP_##top##_ii_b
  switch(top) {
    CASE_TOP(sub):
      CASE_TOP(add):
      CASE_TOP(sh1add):
      CASE_TOP(sh2add):
      CASE_TOP(sh3add):
      CASE_TOP(sh4add):
      CASE_TOP(and):
      CASE_TOP(andc):
      CASE_TOP(or):
      CASE_TOP(orc):
      CASE_TOP(xor):
      CASE_TOP(max):
      CASE_TOP(min):
      CASE_TOP_BR(cmpeq):
      CASE_TOP_BR(cmpne):
      CASE_TOP_BR(cmpge):
      CASE_TOP_BR(cmpgt):
      CASE_TOP_BR(cmple):
      CASE_TOP_BR(cmplt):
      CASE_TOP_BR(andl):
      CASE_TOP_BR(nandl):
      CASE_TOP_BR(orl):
      CASE_TOP_BR(norl):
      CASE_TOP_I(ldw):
      CASE_TOP_I(ldh):
      CASE_TOP_I(ldb):
      CASE_TOP_I(ldhu):
      CASE_TOP_I(ldbu):
      CASE_TOP_I(ldw_d):
      CASE_TOP_I(ldh_d):
      CASE_TOP_I(ldb_d):
      CASE_TOP_I(ldhu_d):
      CASE_TOP_I(ldbu_d):
      CASE_TOP_I(pft):
      return TRUE;

      CASE_TOP(minu):
      CASE_TOP(maxu):
      CASE_TOP_BR(cmpgeu):
      CASE_TOP_BR(cmpgtu):
      CASE_TOP_BR(cmpleu):
      CASE_TOP_BR(cmpltu):
	return FALSE;

    CASE_TOP(slct):
      CASE_TOP(slctf):
      return opnd == 0 ? FALSE: TRUE;
    
    CASE_TOP(mov):
  case TOP_bswap_r:
  case TOP_mtb:
      return TRUE;
    
  case TOP_sxth_r:
  case TOP_sxtb_r:
    return TRUE;

  case TOP_zxth_r:
    return FALSE;

  case TOP_mfb:
    return FALSE;
  case TOP_addcg:
  case TOP_divs:
    return opnd == 2 ? FALSE: TRUE;
    
    CASE_TOP(shl):
      CASE_TOP(shr):
      return opnd == 0 ? TRUE: FALSE;

      CASE_TOP(shru):
      return opnd == 0 ? FALSE: FALSE;
    

    CASE_TOP(mulll):
    return TRUE;
    CASE_TOP(mulllu):
    return FALSE;

    CASE_TOP(mull):
      return TRUE;
    CASE_TOP(mullu):
      return FALSE;

    CASE_TOP(mullhus):
    CASE_TOP(mullh):
    CASE_TOP(mullhu):
    CASE_TOP(mulh):
    CASE_TOP(mulhh):
    CASE_TOP(mulhhs):
    CASE_TOP(mulhhu):
    CASE_TOP(mulhs):
    CASE_TOP(mulhu):
      return TRUE;

    CASE_TOP_I(stw):
    CASE_TOP_I(sth):
    CASE_TOP_I(stb):
      return TRUE;
  }
  
  return TRUE;
#undef CASE_TOP
#undef CASE_TOP_I
#undef CASE_TOP_BR
}

