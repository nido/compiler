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
 * Module: expand.c
 * $Revision$
 * $Date$
 * $Author$
 * $Source$
 *
 * Description:
 *
 * This file contains the internals of code expansion. Its interface
 * is 'Exp_OP', which takes an OP, expands it into a list of OPs which
 * are appended to the oplist passed in.
 *
 * It handles all the macro expansions, special handling during
 * expansion and all the nitty gritty stuff that goes along with it.
 *
 * ====================================================================
 * ====================================================================
 */

#include "defs.h"
#include "config.h"
#include "erglob.h"
#include "ercg.h"
#include "glob.h"
#include "tracing.h"
#include "util.h"

#include "symtab.h"
#include "opcode.h"
#include "intrn_info.h"
#include "const.h"	/* needed to manipulate target/host consts */
#include "targ_const.h"	/* needed to manipulate target/host consts */
#include "cgir.h"
#include "data_layout.h"
#include "stblock.h"
#include "cg_flags.h"
#include "cgtarget.h"
#include "cgexp.h"
#include "w2op.h"
#include "label_util.h"
#include "whirl2ops.h"
#include "dwarf_DST_mem.h"
#include "cgemit.h"
#include "cg_spill.h"	/* for rematerialization */

#include "topcode.h"
#include "targ_isa_lits.h"
#include "targ_isa_properties.h"


/* Do we force inlining of extended immediates. */
static BOOL Inline_Extended_Immediate = FALSE;

/* Helpers for const_Reg/value tns. */
#define TN_Has_Value(tn) (TN_is_const_reg(tn) ? TRUE : TN_has_value(tn))
#define TN_Value(tn) ((tn == Zero_TN) ? 0 : TN_value(tn))

/* Dup_TN won't dup a dedicated tn, but for our purposes we
 * can just re-use the dedicated tn.  Don't want to re-use a
 * symbolic tn or it will mess up live ranges. */
/* DOESN'T WORK:  causes problems in Create_lvs because it causes
 * a use of a parm reg at the call-site, so it looks like the
 * parm-reg is incoming at the call?  This probably should work,
 * but for now we can use other routine that create a real dup tn. */
#define DUP_TN(tn)	Dup_TN_Even_If_Dedicated(tn)

/* ====================================================================
 *   Expand_Or_Inline_Immediate
 *
 *   This function should be called for each TN_is_constant() tn.
 *   Based on some criterion it may:
 *   - return an equivalent register TN
 *   - generate code for moving it into a register TN
 *   - return the tn itself (in this case it can be inlined)
 *   One of the criterion is Disable_Extended_Imm, if true an
 *   extended immediate is never generated.
 * ====================================================================
 */
TN *
Expand_Or_Inline_Immediate(TN *src, TYPE_ID mtype, OPS *ops)
{
  TN *result;
  ISA_REGISTER_CLASS rc = Register_Class_For_Mtype(mtype);
  
#ifdef Is_True_On
  Inline_Extended_Immediate = getenv("INLINE_EXT") != NULL;
#endif
  FmtAssert(TN_is_constant(src), ("Expected constant integer TN"));
  if (TN_has_value(src)) {
    switch(rc) {
    case ISA_REGISTER_CLASS_branch:
      /* Always generate branch values in register. */
      result = Expand_Immediate_Into_Register (mtype, src, ops);
      break;
    case ISA_REGISTER_CLASS_integer:
      if (TN_value(src) == 0) 
	result = Zero_TN;
      else if (Inline_Extended_Immediate || 
	       ISA_LC_Value_In_Class (TN_value(src), LC_s9)) 
	result = src;
      else 
	result = Expand_Immediate_Into_Register (mtype, src, ops);
      break;
    default:
      FmtAssert(0, ("unexpected ISA_REGISTER_class for mtype: %s", 
		    MTYPE_name(mtype)));
    }
  } else if (TN_is_symbol(src)) {
    if (Inline_Extended_Immediate)
      result = src;
    else 
      result =  Expand_Immediate_Into_Register (mtype, src, ops);
  } else {
    FmtAssert(0, ("unexpected constant TN"));
  }
  return result;
}

/* ====================================================================
 *   Expand_Copy (tgt_tn, guard, src_tn, ops)
 * ====================================================================
 */
void
Expand_Copy (
  TN *tgt_tn,
  TN *guard,
  TN *src_tn,
  OPS *ops
)
{
  TN *tmp;

  FmtAssert (TN_is_register(src_tn), ("Expand_Copy: illegal source tn"));


  ISA_REGISTER_CLASS tgt_rc = TN_register_class(tgt_tn);
  ISA_REGISTER_CLASS src_rc = TN_register_class(src_tn);

  switch (src_rc) {

    case ISA_REGISTER_CLASS_integer:

      if (tgt_rc == ISA_REGISTER_CLASS_integer) {
	Build_OP(TOP_mov_r, tgt_tn, src_tn, ops);
	Set_OP_copy (OPS_last(ops));
      }
      else if (tgt_rc == ISA_REGISTER_CLASS_branch) {
	Build_OP(TOP_mtb, tgt_tn, src_tn, ops);
      }
      else goto unsupported;
      break;

    case ISA_REGISTER_CLASS_branch:

      if (tgt_rc == ISA_REGISTER_CLASS_branch) {
	// [CG] Generating a branch copy need 2 instructions
	// We use addcg r0, dest, r0, -1, src.
	// The addcg instruction effectivelly performs the copy
	// in one instruction even if the constant -1 needs an
	// instruction. Thus we can mark the addcg as a copy op.
	// It is not clear if generating a tmp is safe. What happens
	// if the copy is generated after reg alloc ?
	// We don't use a simulated instruction due to the need for
	// a temporary register.
	// DevWarn("generating branch copy");
	TN *tmp;
	tmp = Expand_Immediate_Into_Register(MTYPE_I4, Gen_Literal_TN(-1,4), ops);
	Build_OP(TOP_addcg, Zero_TN, tgt_tn, Zero_TN, tmp, src_tn, ops);
	Set_OP_copy (OPS_last(ops));
      }
      else if (tgt_rc == ISA_REGISTER_CLASS_integer) {
	Build_OP(TOP_mfb, tgt_tn, src_tn, ops);
      }
      else goto unsupported;
      break;

    default:
      goto unsupported;
  }
  return;
 unsupported:
  FmtAssert(FALSE,
	    ("Expand_Copy: unsupported copy %s -> %s",
	     ISA_REGISTER_CLASS_INFO_Name(ISA_REGISTER_CLASS_Info(src_rc)),
	     ISA_REGISTER_CLASS_INFO_Name(ISA_REGISTER_CLASS_Info(tgt_rc))));
  return;
}

/* ====================================================================
 *   Expand_Convert_Length
 *
 *   This does truncation or extension of the int.
 * ====================================================================
 */
void
Expand_Convert_Length (
  TN *dest,
  TN *src,
  TN *length_tn,       // length to convert to
  TYPE_ID mtype,       // type converting from
  BOOL signed,         // obsolete for IA64
  OPS *ops
)
{
  INT16 new_length, old_length;     /* Length to convert to/from */
  // What length do we convert to?  It's the maximum of the target
  // register size, and the operator's result size:
  INT16 from_length;
  INT16 to_length;	/* Size that we have to extend to */
  TOP opc = TOP_UNDEFINED;
  // Do we sign-extend, or zero-extend?
  // If it's an extension, the signdedness of extension
  // depends on the signededness of rtype, else on the
  // signededness of desc
  BOOL signed_extension = MTYPE_is_signed(mtype);

  // This may be a truncation as well as extension.

  to_length = TN_value (length_tn);
  from_length = MTYPE_bit_size(mtype);

  if (from_length > to_length) {
    // truncation
    new_length = to_length;
    old_length = from_length;
  }
  else {
    // extension
    new_length = from_length;
    old_length = to_length;
  }

  if (old_length <= 32) {
    // fits into the LX registers
    if (new_length == 8) {
      // zero extension requires a sequence of shifts:
      if (signed_extension) {
          Build_OP (TOP_sxtb_r, dest, src, ops);
      }
      else {
	Build_OP (TOP_and_i, dest, src, Gen_Literal_TN(0xff,4), ops);
      }
    }
    else if (new_length == 16) {
      // zero extension requires a sequence of shifts:
      if (signed_extension) {
          Build_OP (TOP_sxth_r, dest, src, ops);
      }
      else {
	Build_OP (TOP_zxth_r, dest, src, ops);
      }
    }
    else if (new_length < 16) {
      INT64 mask = 1;
      INT32 i;

      for ( i = 0; i < new_length-1; ++i ) {
        mask |= mask << 1;
      }

      Build_OP (TOP_and_i, dest, src, Gen_Literal_TN(mask, 4), ops);
    }
  }
  else
      FmtAssert(FALSE, ("Expand_Convert_Length: unknown src extension"));

  return;
}

/* ====================================================================
 *   Expand_Int_To_Ptr
 * ====================================================================
 */
void
Expand_Int_To_Ptr (
  TN *result,
  TYPE_ID rtype,
  TN *src,
  TYPE_ID desc,
  OPS *ops
)
{
  FmtAssert (FALSE,("Convert_Int_To_Ptr: unsupported"));

  return;
}

/* ====================================================================
 *   Expand_Ptr_To_Int
 *
 *   source is pointer, result is integer
 * ====================================================================
 */
void
Expand_Ptr_To_Int (
  TN *result,
  TYPE_ID rtype,
  TN *src,
  TYPE_ID desc,
  OPS *ops
)
{
  FmtAssert (FALSE,("Convert_Ptr_To_Int: unsupported"));

  return;
}

/* ====================================================================
 *   Expand_Convert
 *
 *   op2 is the length to convert to.
 * ====================================================================
 */
void
Expand_Convert (
  TN *result,
  TN *op1,
  TN *op2,
  TYPE_ID rtype,
  TYPE_ID desc,
  OPS  *ops
)
{
  INT16 src_length;     /* Length to convert from */
  INT16 new_length;	/* Length to convert to */
  TOP opc = TOP_UNDEFINED;

  /* conversions to int: */
  if (MTYPE_is_class_integer(rtype)) {

    if (desc == MTYPE_B) {
      Expand_Bool_To_Int (result, op1, rtype, ops);
      return;
    }
    else if (MTYPE_is_class_integer(desc)) {
      Expand_Convert_Length (result, op1, op2, desc, TRUE, ops);
      return;
    }
  }

  if (opc != TOP_UNDEFINED) {
      Build_OP (opc, result, op1, ops);
      return;
  }

  FmtAssert(0,("Expand_Convert: unsupported conversion %s -> %s",
		               MTYPE_name(desc), MTYPE_name(rtype)));
  return;
}

/* ====================================================================
 *   Fixup_32_Bit_Op (result, src, dest_type, ops)
 *
 *   Helper routine to do proper sign extension
 * ====================================================================
 */
static void
Fixup_32_Bit_Op(
  TN *result,
  TN *src,
  TYPE_ID dest_type,
  OPS *ops
)
{
  FmtAssert(FALSE,("Not Implemented"));

  if (dest_type == MTYPE_I8 || dest_type == MTYPE_U8) {
    Expand_Copy(result, NULL, src, ops);
  } else {
    Expand_Convert_Length (result, src,
             Gen_Literal_TN(MTYPE_size_reg(dest_type), 4),
			   dest_type, MTYPE_is_signed(dest_type),ops);
  }
}

/* ====================================================================
 *   Exp_Immediate (dest, src, is_signed, ops)
 *
 * ====================================================================
 */
void
Exp_Immediate (
  TN *dest,
  TN *src,
  BOOL is_signed,
  OPS *ops
)
{
  INT64 val;
  TOP   top;
  TN    *tmp;

  if (TN_has_value(src)) {
    val = TN_value(src);
  }

  switch (TN_register_class(dest)) {
  case ISA_REGISTER_CLASS_branch:
    FmtAssert(TN_has_value(src),("Exp_Immediate: MTYPE_B <- symbol is invalid"));
    if (val == 0) {
      Build_OP(TOP_mtb, dest, Zero_TN, ops);
    } else {
      Build_OP(TOP_cmpeq_r_b, dest, Zero_TN, Zero_TN, ops);
    }
    break;
    
  case ISA_REGISTER_CLASS_integer:
    if (TN_has_value(src) && ISA_LC_Value_In_Class (val, LC_s9)) {
      if (val == 0) Build_OP (TOP_mov_r, dest, Zero_TN, ops);
      else Build_OP (TOP_mov_i, dest, src, ops);
    } else {
      Build_OP (TOP_mov_ii, dest, src, ops);
    }
    break;
    
  default:
    FmtAssert(0, ("Exp_Immediate: unknown ISA_REGISTER_CLASS"));
  }


  return;
}

/* ====================================================================
 *   Expand Immediate
 * ====================================================================
 */
void
Expand_Immediate (
  TN *dest, 
  TN *src, 
  TYPE_ID mtype, 
  OPS *ops
)
{
  FmtAssert((TN_is_constant(src)),
	    ("unexpected non-constant in Expand_Immediate"));
  FmtAssert((TN_has_value(src) || TN_is_symbol(src)),
	    ("expected value or const in Expand_Immediate"));
  //Exp_Immediate (dest, src, mtype, ops);
  Exp_Immediate (dest, src, MTYPE_signed(mtype) ? TRUE : FALSE, ops);
}

/* ====================================================================
 *   Expand_Immediate_Into_Register (src, ops)
 * ====================================================================
 */
TN*
Expand_Immediate_Into_Register (
  TYPE_ID  mtype,
  TN    *src,
  OPS   *ops
)
{
  /* load into reg and do reg case */
  TN *tmp = Build_TN_Of_Mtype (mtype);
  Expand_Immediate (tmp, src, mtype, ops);
  if (TN_has_value(src) && MTYPE_is_integral(mtype))
    CGSPILL_Attach_Intconst_Remat(tmp, mtype, TN_value(src));
  return tmp;
}

/* ====================================================================
 *   Expand_Add
 * ====================================================================
 */
void
Expand_Add (
  TN *result,
  TN *src1,
  TN *src2,
  TYPE_ID mtype,
  OPS *ops
)
{
  TOP    new_opcode;
  TN    *tmp;

  FmtAssert (TN_is_register(src1) || TN_is_register(src2),
	     ("Expand_Add: both operands const ?"));
  FmtAssert (Register_Class_For_Mtype(mtype) == ISA_REGISTER_CLASS_integer,
	     ("Unexpected MTYPE: %s", MTYPE_name(mtype)));
  FmtAssert (MTYPE_is_integral(mtype),
	     ("Unexpected MTYPE: %s", MTYPE_name(mtype)));

  if (TN_is_constant(src1)) {
    // switch order of src so immediate is second
    Expand_Add (result, src2, src1, mtype, ops);
    return;
  }

  if (TN_Has_Value(src2)) {
    if (TN_Value(src2) == 0) {
      Expand_Copy(result, NULL, src1, ops);
      return;
    }
  } 

  // Process immediate operand
  // [CG]: Note that for the add, we only call Expand_Or_Inline_Immediate()
  // for TN_has_value() as TN_is_symbol is treated below in a special
  // way.
  if (TN_has_value(src2)) {
    src2 = Expand_Or_Inline_Immediate(src2, mtype, ops);
  }

  if (TN_is_constant(src2)) {

    if (TN_has_value(src2)) {
      INT64 val = TN_value(src2);
      new_opcode = TOP_opnd_immediate_variant(TOP_add_r, 1, val);
    }
    // symbolic constant, gp-relative or sp-relative
    else if (TN_is_symbol(src2)) {
      //
      // src1 is the base, src2 is the ofst
      //
      INT64 ofst;
      ST *base;

      tmp = Build_TN_Of_Mtype (Pointer_Mtype);
      Base_Symbol_And_Offset_For_Addressing (TN_var(src2),
                                       TN_offset(src2), &base, &ofst);

      // TN_is_reloc_got_disp (@gprel) and TN_is_reloc_neg_got_disp (@neggprel)
      // can always be the operand of addi.
      if (TN_is_reloc_got_disp(src2) || TN_is_reloc_neg_got_disp(src2)) {
	new_opcode = TOP_add_i;
      }
      else if (ST_on_stack(TN_var(src2))) {
#if 1
        // (cbr) if the offset is 0 and the symbol is on the stack
        // then just do a copy.
        if (base == SP_Sym && ofst == 0) {
          Expand_Copy (result, NULL, src1, ops);
          return;
        }
#endif
	// On stack symbolic offset becomes an immediate
	// in the final code
	new_opcode = TOP_add_i;
      }
      else {
        // just makea and hope that 16bit offset suffices:
	Expand_Immediate(tmp, src2, mtype, ops);
        src2 = tmp;
	new_opcode = TOP_add_r;
      }
    }
    else {
      FmtAssert(FALSE,("unexpected constant in Expand_Add"));
    }
  }
  else {
    // both registers:
    FmtAssert (TN_is_register(src2),
	              ("Expand_Add: unknown 2nd operand type"));
    FmtAssert(Register_Class_For_Mtype(mtype) == TN_register_class(src2),
	      ("Expand_Add: mtype screwed"));

    new_opcode = TOP_add_r;
  }

  Build_OP (new_opcode, result, src1, src2, ops);
  return;
}

/* ====================================================================
 *   Expand_Sub
 * ====================================================================
 */
void
Expand_Sub (
  TN *result,
  TN *src1,
  TN *src2,
  TYPE_ID mtype,
  OPS *ops
)
{
  TOP new_opcode;
  TN    *tmp;
  ST    *base;
  INT64  ofst, val;

  FmtAssert (TN_is_register(src1) || TN_is_register(src2),
	     ("Expand_Sub: both operands const ?"));
  FmtAssert (Register_Class_For_Mtype(mtype) == ISA_REGISTER_CLASS_integer,
	     ("Unexpected MTYPE: %s", MTYPE_name(mtype)));
  FmtAssert (MTYPE_is_integral(mtype),
	     ("Unexpected MTYPE: %s", MTYPE_name(mtype)));



  if (TN_Has_Value(src2)) {
    if (TN_Value(src2) == 0) {
      Expand_Copy(result, NULL, src1, ops);
      return;
    }
  } 

  // Process immediate operand
  if (TN_is_constant(src1)) {
    src1 = Expand_Or_Inline_Immediate(src1, mtype, ops);
  }
  if (TN_is_constant(src2)) {
    src2 = Expand_Or_Inline_Immediate(src2, mtype, ops);
  }

  if (TN_has_value(src1)) {
    new_opcode = TOP_opnd_immediate_variant(TOP_sub_r, 0, TN_value(src1));
  }
  else if (TN_has_value(src2)) {
    INT64 val = (INT64)(INT32)(-TN_value(src2));
    TN *tmp = Gen_Literal_TN(val, 4);
    Expand_Add(result, src1, tmp, mtype, ops);
    return;
  }
  else {
    // default both registers
    new_opcode = TOP_sub_r;
  }

  Build_OP (new_opcode, result, src1, src2, ops);
  return;
}

/* ====================================================================
 *   Expand_Neg
 *
 *   Use the opcode that does not generate sticky flags effects.
 * ====================================================================
 */
void
Expand_Neg (
  TN *dest,
  TN *src,
  TYPE_ID mtype,
  OPS *ops
)
{
  FmtAssert (TN_is_register(src),
	     ("Expand_Neg: operand const ?"));
  FmtAssert (Register_Class_For_Mtype(mtype) == ISA_REGISTER_CLASS_integer,
	     ("Unexpected MTYPE: %s", MTYPE_name(mtype)));
  FmtAssert (MTYPE_is_integral(mtype),
	     ("Unexpected MTYPE: %s", MTYPE_name(mtype)));

  Build_OP (TOP_sub_r, dest, Zero_TN, src, ops);
  return;
}

/* ====================================================================
 *   Expand_Abs
 * ====================================================================
 */
void
Expand_Abs (
  TN *dest,
  TN *src,
  TYPE_ID mtype,
  OPS *ops
)
{
  FmtAssert (TN_is_register(src),
	     ("Expand_Abs: operand const ?"));
  FmtAssert (Register_Class_For_Mtype(mtype) == ISA_REGISTER_CLASS_integer,
	     ("Unexpected MTYPE: %s", MTYPE_name(mtype)));
  

  if (MTYPE_is_integral(mtype)) {
      //
      // For dest = abs (src) generate:
      //  
      //            sub   negx = $r0.0, src
      //            max   dest = negx, src
      //
    TN *negx = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer);
    Build_OP (TOP_sub_r, negx, Zero_TN, src, ops);
    Build_OP (TOP_max_r, dest, negx, src, ops);
  } else if (MTYPE_is_float(mtype)) {
    TN *mask = Gen_Literal_TN(0x7fffffff, 4);
    Expand_Binary_And(dest, src, mask, MTYPE_I4, ops);
  } else {
    Is_True(FALSE,("abs doesn't handle MTYPE %s\n", MTYPE_name(mtype)));
  }
}

/* ====================================================================
 *   Expand_Shift
 * ====================================================================
 */
void
Expand_Shift (
  TN *result,
  TN *src1,
  TN *src2,
  TYPE_ID mtype,
  SHIFT_DIRECTION kind,
  OPS *ops
)
{
  TOP new_opcode = TOP_UNDEFINED;

  FmtAssert (TN_is_register(src1) || TN_is_register(src2),
	     ("Expand_Shift: operand const ?"));
  FmtAssert (Register_Class_For_Mtype(mtype) == ISA_REGISTER_CLASS_integer,
	     ("Unexpected MTYPE: %s", MTYPE_name(mtype)));
  FmtAssert (MTYPE_is_integral(mtype),
	     ("Unexpected MTYPE: %s", MTYPE_name(mtype)));

  // On this target shift value is mod 256
  if (TN_Has_Value(src2)) {
    if ((TN_Value(src2) & 0xFF) == 0) {
      Expand_Copy(result, NULL, src1, ops);
      return;
    }
  } 

  // Process immediate operand
  if (TN_is_constant(src1)) {
    src1 = Expand_Immediate_Into_Register(mtype, src1, ops);
  }
  if (TN_is_constant(src2)) {
    src2 = Expand_Or_Inline_Immediate(src2, mtype, ops);
  }
  
  switch (kind) {
    case shift_left:
      new_opcode = TOP_shl_r;
      break;
    case shift_aright:
      new_opcode = TOP_shr_r;
      break;
    case shift_lright:
      new_opcode = TOP_shru_r;
      break;
  }

  if (TN_has_value(src2))
    new_opcode = TOP_opnd_immediate_variant(new_opcode, 1, TN_value(src2));

  FmtAssert(new_opcode != TOP_UNDEFINED,
	                  ("Expand_Shift: shift is not handled"));

  Build_OP (new_opcode, result, src1, src2, ops);
  return;
}

/* ====================================================================
 *   Is_Power_OF_2
 *
 *   return TRUE if the val is a power of 2
 * ====================================================================
 */
#define IS_POWER_OF_2(val)	((val != 0) && ((val & (val-1)) == 0))

static BOOL
Is_Power_Of_2 (
  INT64 val,
  TYPE_ID mtype
)
{
  if (MTYPE_is_signed(mtype) && val < 0) val = -val;

  if (mtype == MTYPE_U4) val &= 0xffffffffull;

  return IS_POWER_OF_2(val);
}

/* ====================================================================
 *   Get_Power_OF_2
 * ====================================================================
 */
static INT
Get_Power_Of_2 (
  INT64 val,
  TYPE_ID mtype
)
{
  INT i;
  INT64 pow2mask;

  if (MTYPE_is_signed(mtype) && val < 0) val = -val;

  if (mtype == MTYPE_U4) val &= 0xffffffffull;

  pow2mask = 1;
  for ( i = 0; i < MTYPE_size_reg(mtype); ++i ) {
    if (val == pow2mask) return i;
    pow2mask <<= 1;
  }

  FmtAssert(FALSE, ("Get_Power_Of_2 unexpected value"));
  /* NOTREACHED */
}

/* ====================================================================
 *   Expand_Shift_Multiply
 *
 *   Make a shift if possible. src2 is a constant TN.
 * ====================================================================
 */
static BOOL
Expand_Shift_Multiply (
  TN *result,
  TYPE_ID mtype,
  TN *src1,
  TN *src2,
  OPS *ops
)
{
  INT val;
  TOP opcode = TOP_UNDEFINED;

  FmtAssert (TN_is_register(src1) && TN_has_value(src2),
	     ("Expand_Shift_Multiply: not register/const operands"));
  FmtAssert (Register_Class_For_Mtype(mtype) == ISA_REGISTER_CLASS_integer,
	     ("Unexpected MTYPE: %s", MTYPE_name(mtype)));
  FmtAssert (MTYPE_is_integral(mtype),
	     ("Unexpected MTYPE: %s", MTYPE_name(mtype)));

  // see if I can do a shift instead of the multiply
  if (Is_Power_Of_2(TN_value(src2), mtype)) {
    val = Get_Power_Of_2(TN_value(src2), mtype);
    TN *tmp = Gen_Literal_TN (val, MTYPE_byte_size(mtype));
    Expand_Shift (result, src1, tmp, mtype, shift_left, ops);
    
    // do we need to negate the result ?
    if (TN_value(src2) < 0) {
      Expand_Neg (result, result, mtype, ops);      
    }

    return TRUE;
  }
  
  // Special cases handled by shadd instructions
  val = TN_value (src2);
  switch (val) {
  case 3:
    opcode = TOP_sh1add_r;
    break;
  case 5:
    opcode = TOP_sh2add_r;
    break;
  case 9:
    opcode = TOP_sh3add_r;
    break;
  case 17:
    opcode = TOP_sh4add_r;
    break;
  default:
    opcode = TOP_UNDEFINED;
  }

  if (opcode != TOP_UNDEFINED) {
    Build_OP(opcode, result, src1, src1, ops);
    return TRUE;
  }

  return FALSE;
}

/* ====================================================================
 *   Expand_Multiply
 * ====================================================================
 */
void
Expand_Multiply (
  TN *result,
  TYPE_ID rmtype,
  TN *src1,
  TYPE_ID s1mtype,
  TN *src2,
  TYPE_ID s2mtype,
  OPS *ops
)
{
  BOOL swapped = FALSE;
  TOP opcode = TOP_UNDEFINED;

  FmtAssert (Register_Class_For_Mtype(rmtype) == ISA_REGISTER_CLASS_integer,
	     ("Unexpected MTYPE: %s", MTYPE_name(rmtype)));
  FmtAssert (MTYPE_is_integral(rmtype),
	     ("Unexpected MTYPE: %s", MTYPE_name(rmtype)));


  //
  // Check for two constants
  //
  if (TN_has_value(src1) && TN_has_value(src2)) {
    // Two constants can sometimes occur because of DIVREM production in
    TN *val_tn;
    INT64    constant;
    constant = TN_value(src1) * TN_value(src2);

    // Need to get the constant of the right length
    constant = Targ_To_Host(Host_To_Targ(rmtype, constant));
    val_tn = Gen_Literal_TN(constant, TN_size(result));
    Expand_Immediate(result, val_tn, rmtype, ops);
    return;
  }

  FmtAssert (TN_is_register(src1) || TN_is_register(src2),
	     ("Expand_Multiply: const operands ?"));

  // If one of the operands is a constant, it must be the second one.
  if (TN_has_value(src1)) {
    Expand_Multiply(result, rmtype, src2, s2mtype, src1, s1mtype, ops);
    return;
  }
  
  if (TN_Has_Value(src2)) {
    if (TN_Value(src2)== 0) {
      Expand_Copy(result, NULL, Zero_TN, ops);
      return;
    }
  } 

  // Try to generate shift or shift add instead of mul
  if (TN_has_value(src2)) {
    if (Expand_Shift_Multiply (result, rmtype, src1, src2, ops)) 
      return;
  }

  // Get range for value
  if (TN_has_value(src2)) {
    INT64 value = TN_value(src2);
    if (value >= -(1<<15) && value < (1<<15)-1) {
      s2mtype = MTYPE_I2;
    } else if (value >= 0 && value < (1<<16)-1) {
      s2mtype = MTYPE_U2;
    }
  }

  // Process immediate operand
  if (TN_is_constant(src2)) {
    src2 = Expand_Or_Inline_Immediate(src2, rmtype, ops);
  }

  if (s1mtype == MTYPE_U2 && s2mtype == MTYPE_U2) 
    opcode = TOP_mulllu_r;
  else if (s1mtype == MTYPE_I2 && s2mtype == MTYPE_I2) 
    opcode = TOP_mulll_r;
  else if (s2mtype == MTYPE_U2) 
    opcode = TOP_mullu_r;
  else if (s2mtype == MTYPE_I2) 
    opcode = TOP_mull_r;
  else if (!TN_has_value(src2) && s1mtype == MTYPE_U2) { 
    opcode = TOP_mullu_r; swapped = TRUE; 
  } else if (!TN_has_value(src2) && s1mtype == MTYPE_I2) {
    opcode = TOP_mull_r; swapped = TRUE; 
  } else {
    // 32x32 multiply
    if (Is_Target_st231()) {
	opcode = TOP_mul32_r ;
    } else {
	TN *tmp1 = Build_RCLASS_TN(ISA_REGISTER_CLASS_integer);
	TN *tmp2 = Build_RCLASS_TN(ISA_REGISTER_CLASS_integer);
	opcode = TOP_mullu_r;
	if (TN_has_value(src2))
	    opcode = TOP_opnd_immediate_variant(opcode, 1, TN_value(src2));
	Build_OP(opcode, tmp1, src1, src2, ops);
	opcode = TOP_mulhs_r;
	if (TN_has_value(src2))
	    opcode = TOP_opnd_immediate_variant(opcode, 1, TN_value(src2));
	Build_OP(opcode, tmp2, src1, src2, ops);
	Build_OP(TOP_add_r, result, tmp1, tmp2, ops);
	return;
    }
  }

  if (opcode == TOP_UNDEFINED)
    FmtAssert(FALSE, ("Expand_Multiply: failed"));
  
  if (swapped) {
    if (TN_has_value(src1)) 
      opcode = TOP_opnd_immediate_variant(opcode, 1, TN_value(src1));
    Build_OP(opcode, result, src2, src1, ops);
  } else {
    if (TN_has_value(src2)) 
      opcode = TOP_opnd_immediate_variant(opcode, 1, TN_value(src2));
    Build_OP(opcode, result, src1, src2, ops);
  }

  return;
}

/* ====================================================================
 *   Expand_Madd
 * ====================================================================
 */
void
Expand_Madd (
  TN *result,
  TYPE_ID rmtype,
  TN *src0,
  TYPE_ID s0mtype,
  TN *src1,
  TYPE_ID s1mtype,
  TN *src2,
  TYPE_ID s2mtype,
  OPS *ops
)
{
  FmtAssert (FALSE,("Expand_Madd: unsupported"));
  return;
}

/* ====================================================================
 *   Expand_High_Multiply
 *
 *   return high part of multiply result
 * ====================================================================
 */
void
Expand_High_Multiply (
  TN *result,
  TN *src1,
  TN *src2,
  TYPE_ID mtype,
  OPS *ops
)
{
  FmtAssert(FALSE,("Not Implemented"));

  return;
}

/* ====================================================================
 *   Expand_Normalize_Logical
 * ====================================================================
 */

static void
Expand_Normalize_Logical (
  TN *src,
  OPS *ops
)
{
  // [CG]: Should not be called on this target as normalization is
  // not necessary
  // 
  FmtAssert(0,("Expand_Normalize_Logical should not be called"));
  
  return;
}

static void
Expand_Logical_Move (
  TN *dest,
  TN *src,
  VARIANT variant,
  OPS *ops
)
{
  /* dest = (src == 0) ? 1 : 0 */

  TOP opcode = TOP_UNDEFINED;

  // Trivial operand
  if (TN_Has_Value(src)) {
    if (TN_Value(src) == 0) { 
      Expand_Immediate(dest, Gen_Literal_TN(0, 4), MTYPE_I4, ops);
    } else {
      Expand_Immediate(dest, Gen_Literal_TN(1, 4), MTYPE_I4, ops);
    }
    return;
  }
  
  // Process immediate operand
  if (TN_is_constant(src)) {
    src = Expand_Or_Inline_Immediate(src, MTYPE_I4, ops);
  }
  
  if (TN_register_class(dest) == ISA_REGISTER_CLASS_branch)
    opcode = TOP_cmpne_r_b;
  else if (TN_register_class(dest) == ISA_REGISTER_CLASS_integer)
    opcode = TOP_cmpne_r_r;
  
  FmtAssert(opcode != TOP_UNDEFINED, ("Expand_Logical_Move"));
  
  Build_OP (opcode, dest, Zero_TN, src, ops);

  return;
}

/* ====================================================================
 *   Expand_Logical_Not
 * ====================================================================
 */
void
Expand_Logical_Not (
  TN *dest,
  TN *src,
  VARIANT variant,
  OPS *ops
)
{
  /* dest = (src == 0) ? 1 : 0 */

  TOP opcode = TOP_UNDEFINED;

  // Trivial operand
  if (TN_Has_Value(src)) {
    if (TN_Value(src) == 0) { 
      Expand_Immediate(dest, Gen_Literal_TN(1, 4), MTYPE_I4, ops);
    } else {
      Expand_Immediate(dest, Gen_Literal_TN(0, 4), MTYPE_I4, ops);
    }
    return;
  }
  
  // Process immediate operand
  if (TN_is_constant(src)) {
    src = Expand_Or_Inline_Immediate(src, MTYPE_I4, ops);
  }
  
  if (TN_register_class(dest) == ISA_REGISTER_CLASS_branch)
    opcode = TOP_cmpeq_r_b;
  else if (TN_register_class(dest) == ISA_REGISTER_CLASS_integer)
    opcode = TOP_cmpeq_r_r;
  
  FmtAssert(opcode != TOP_UNDEFINED, ("Expand_Logical_Not"));

  Build_OP (opcode, dest, Zero_TN, src, ops);

  return;
}

/* ====================================================================
 *   Expand_Logical_And_Or
 *
 *
 *	dest = (src1 != 0 & src2 != 0) ? 1 : 0  (normalization)
 *	sltu	t1, 0, s1		(if not normalized)
 *	sltu	t2, 0, s2		(if not normalized)
 *	and/or	d, t1, t2
 * ====================================================================
 */
static void
Expand_Logical_And_Or (
  TOP action,
  TN *dest,
  TN *src1,
  TN *src2,
  VARIANT variant,
  OPS *ops
)
{
 /*
  *  if CG_EXP_normalize is true we must normalized the operands
  *  (if not already normalized)
  */
  if (!V_normalized_op1(variant) && CGEXP_normalize_logical)
  {
    Expand_Normalize_Logical (src1, ops);
  }
  if (!V_normalized_op2(variant) && CGEXP_normalize_logical)
  {
    Expand_Normalize_Logical (src2, ops);
  }

  Build_OP (action, dest, src1, src2, ops);
}

/* ====================================================================
 *   Expand_Logical_And
 * ====================================================================
 */
void
Expand_Logical_And (
  TN *dest,
  TN *src1,
  TN *src2,
  VARIANT variant,
  OPS *ops
)
{
  TOP opcode = TOP_UNDEFINED;
  
  FmtAssert (TN_is_register(src1) || TN_is_register(src2),
	     ("Expand_Logical_And: both operands const ?"));

  if (TN_is_constant(src1)) {
    // switch order of src so immediate is second
    Expand_Logical_And (dest, src2, src1, variant, ops);
    return;
  }

  // Process immediate operand
  if (TN_is_constant(src2)) {
    src2 = Expand_Or_Inline_Immediate(src2, MTYPE_I4, ops);
  }
  if (TN_Has_Value(src2)) {
    if (TN_Value(src2) == 0) 
      Expand_Immediate(dest, Gen_Literal_TN(0, 4), MTYPE_I4, ops);
    else
      Expand_Logical_Move(dest, src1, 0, ops);
    return;
  }

  if (TN_register_class(dest) == ISA_REGISTER_CLASS_branch)
    opcode = TOP_andl_r_b;
  else if (TN_register_class(dest) == ISA_REGISTER_CLASS_integer)
    opcode = TOP_andl_r_r;

  FmtAssert(opcode != TOP_UNDEFINED, ("Expand_Logical_And"));

  Expand_Logical_And_Or (opcode, dest, src1, src2, variant, ops);

  return;
}

/* ====================================================================
 *   Expand_Logical_Or
 * ====================================================================
 */
void
Expand_Logical_Or (
  TN *dest,
  TN *src1,
  TN *src2,
  VARIANT variant,
  OPS *ops
)
{
  TOP opcode = TOP_UNDEFINED;

  FmtAssert (TN_is_register(src1) || TN_is_register(src2),
	     ("Expand_Logical_Or: both operands const ?"));

  if (TN_is_constant(src1)) {
    // switch order of src so immediate is second
    Expand_Logical_Or (dest, src2, src1, variant, ops);
    return;
  }

  // Process immediate operand
  if (TN_is_constant(src2)) {
    src2 = Expand_Or_Inline_Immediate(src2, MTYPE_I4, ops);
  }
  
  if (TN_Has_Value(src2)) {
    if (TN_Value(src2) == 0) 
      Expand_Logical_Move(dest, src1, 0, ops);
    else
      Expand_Immediate(dest, Gen_Literal_TN(1, 4), MTYPE_I4, ops);
    return;
  }

  if (TN_register_class(dest) == ISA_REGISTER_CLASS_branch)
    opcode = TOP_orl_r_b;
  else if (TN_register_class(dest) == ISA_REGISTER_CLASS_integer)
    opcode = TOP_orl_r_r;

  FmtAssert(opcode != TOP_UNDEFINED, ("Expand_Logical_Or"));

  Expand_Logical_And_Or (opcode, dest, src1, src2, variant, ops);

  return;
}

/* ====================================================================
 *   Expand_Binary_Compliment
 * ====================================================================
 */
void
Expand_Binary_Complement (
  TN *dest,
  TN *src,
  TYPE_ID mtype,
  OPS *ops
)
{
  Is_True(MTYPE_is_class_integer(mtype),("not integer for complement"));

  /* complement == xor src 0xffffffff */
  Build_OP (TOP_xor_i, dest, src, Gen_Literal_TN(-1, 4), ops);
}


/* ====================================================================
 *   Expand_Special_And_Immed
 *
 * Expand special cases of AND with an immediate. These are cases
 * where the constant is too big for TOP_and_i, but can be handled
 * more simply than loading the constant into a register and using TOP_and.
 *
 * NOTE: that 'mix' could be used to zero fixed patterns of bytes and
 * shorts, but I couldn't find a case to trigger it so I left it out -- Ken
 * ====================================================================
 */
BOOL
Expand_Special_And_Immed (
  TN *dest,
  TN *src1,
  INT64 imm,
  OPS *ops
)
{
  UINT len;
  UINT start;
  UINT64 v;

  FmtAssert (TN_is_register(src1),
	     ("Expand_Apacial_And_Immed: operand const ?"));

  if (imm == 0) {
    Expand_Copy(dest, NULL, Zero_TN, ops);
    return TRUE;
  } else if (((INT32)imm & -1) == -1) {
    Expand_Copy(dest, NULL, src1, ops);
    return TRUE;
  } else if (imm == 0xFFFF) {
    Build_OP(TOP_zxth_r, dest, src1, ops);
    return TRUE;
  }
  return FALSE;
}

/* ====================================================================
 *   Expand_Binary_And_Or
 * ====================================================================
 */
static void
Expand_Binary_And_Or (
  TOP action,
  TN *dest,
  TN *src1,
  TN *src2,
  TYPE_ID mtype,
  OPS *ops
)
{
  FmtAssert(MTYPE_is_class_integer(mtype),("can't handle non integer mtype"));
  FmtAssert(Register_Class_For_Mtype(mtype) == TN_register_class(dest),
	                       ("wrong register class of the dest register"));

  Build_OP (action, dest, src1, src2, ops);
  return;
}

/* ====================================================================
 *   Expand_Binary_And
 * ====================================================================
 */
void
Expand_Binary_And (
  TN *dest,
  TN *src1,
  TN *src2,
  TYPE_ID mtype,
  OPS *ops
)
{
  TOP opcode = TOP_UNDEFINED;

  FmtAssert (TN_is_register(src1) || TN_is_register(src2),
	     ("Expand_Binary_And: both operands const ?"));

  if (TN_is_constant(src1)) {
    // switch order of src so immediate is second
    Expand_Binary_And (dest, src2, src1, mtype, ops);
    return;
  }

  // Try special and with imm cases
  if (TN_Has_Value(src2))
    if (Expand_Special_And_Immed(dest, src1, TN_Value(src2), ops))
      return;

  // Process immediate operand
  if (TN_is_constant(src2)) {
    src2 = Expand_Or_Inline_Immediate(src2, mtype, ops);
  }
  
  if (TN_register_class(dest) == ISA_REGISTER_CLASS_integer)
    opcode = TOP_and_r;
  
  FmtAssert(opcode != TOP_UNDEFINED, ("Expand_Binary_And"));

  if (TN_has_value(src2))
    opcode = TOP_opnd_immediate_variant(opcode, 1, TN_value(src2));
  
  Expand_Binary_And_Or (opcode, dest, src1, src2, mtype, ops);

  return;
}

/* ====================================================================
 *   Expand_Binary_Or
 * ====================================================================
 */
void
Expand_Binary_Or (
  TN *dest,
  TN *src1,
  TN *src2,
  TYPE_ID mtype,
  OPS *ops
)
{
  TOP opcode = TOP_UNDEFINED;

  FmtAssert (TN_is_register(src1) || TN_is_register(src2),
	     ("Expand_Binary_Or: both operands const ?"));

  if (TN_is_constant(src1)) {
    // switch order of src so immediate is second
    Expand_Binary_Or (dest, src2, src1, mtype, ops);
    return;
  }


  // Try special or with imm cases
  if (TN_Has_Value(src2)) {
    INT64 val = TN_Value(src2);
    if (val == 0) {
      Expand_Copy(dest, NULL, src1, ops);
      return;
    } else if (((INT32)val & -1) == -1) {
      Expand_Immediate(dest, Gen_Literal_TN(-1, 4), MTYPE_I4, ops);
      return;
    }
  }


  // Process immediate operand
  if (TN_is_constant(src2)) {
    src2 = Expand_Or_Inline_Immediate(src2, mtype, ops);
  }
  
  if (TN_register_class(dest) == ISA_REGISTER_CLASS_integer)
    opcode = TOP_or_r;
  
  FmtAssert(opcode != TOP_UNDEFINED, ("Expand_Binary_Or"));

  if (TN_has_value(src2))
    opcode = TOP_opnd_immediate_variant(opcode, 1, TN_value(src2));
  
  Expand_Binary_And_Or (opcode, dest, src1, src2, mtype, ops);

  return;
}

/* ====================================================================
 *   Expand_Binary_Xor
 * ====================================================================
 */
void
Expand_Binary_Xor (
  TN *dest,
  TN *src1,
  TN *src2,
  TYPE_ID mtype,
  OPS *ops
)
{
  TOP opcode = TOP_UNDEFINED;

  FmtAssert (TN_is_register(src1) || TN_is_register(src2),
	     ("Expand_Binary_Or: both operands const ?"));

  if (TN_is_constant(src1)) {
    // switch order of src so immediate is second
    Expand_Binary_Xor (dest, src2, src1, mtype, ops);
    return;
  }
  // Try special xor with imm cases
  if (TN_Has_Value(src2)) {
    INT64 val = TN_Value(src2);
    if (val == 0) {
      Expand_Copy(dest, NULL, src1, ops);
      return;
    } 
  }

  // Process immediate operand
  if (TN_is_constant(src2)) {
    src2 = Expand_Or_Inline_Immediate(src2, mtype, ops);
  }
  
  if (TN_register_class(dest) == ISA_REGISTER_CLASS_integer)
    opcode = TOP_xor_r;
  
  FmtAssert(opcode != TOP_UNDEFINED, ("Expand_Binary_Xor"));

  if (TN_has_value(src2))
    opcode = TOP_opnd_immediate_variant(opcode, 1, TN_value(src2));
  
  Expand_Binary_And_Or (opcode, dest, src1, src2, mtype, ops);

  return;
}

/* ====================================================================
 *   Expand_Binary_Nor
 * ====================================================================
 */
void
Expand_Binary_Nor (
  TN *dest,
  TN *src1,
  TN *src2,
  TYPE_ID mtype,
  OPS *ops
)
{
  FmtAssert(FALSE,("Not Implemented"));
}

/* ====================================================================
 *   Expand_Bool_Comparison
 * ====================================================================
 */
static void
Expand_Bool_Comparison (
  BOOL equals,
  TN *dest,
  TN *src1,
  TN *src2,
  OPS *ops
)
{
  FmtAssert(FALSE,("Expand_Bool_Comparison: not implemented"));
}

/* ====================================================================
 *   Expand_Int_Less
 * ====================================================================
 */
void
Expand_Int_Less (
  TN *dest,
  TN *src1,
  TN *src2,
  TYPE_ID desc,
  OPS *ops
)
{
  VARIANT variant;
  TOP action;
  
  switch (desc) {
  case MTYPE_I4: variant = V_BR_I4LT; break;
  case MTYPE_U4: variant = V_BR_U4LT; break;
  default:
    Is_True(FALSE, ("Expand_Int_Less: MTYPE_%s is not handled",
		                               Mtype_Name(desc)));
  }
  
  if (TN_register_class(dest) == ISA_REGISTER_CLASS_branch) {
    action = Pick_Compare_TOP (&variant, &src1, &src2, FALSE, ops);
    Build_OP (action, dest, src1, src2, ops);
  }
  else if (TN_register_class(dest) == ISA_REGISTER_CLASS_integer) {
    action = Pick_Compare_TOP (&variant, &src1, &src2, TRUE, ops);
    Build_OP (action, dest, src1, src2, ops);
  }
  else {
    FmtAssert(FALSE, ("Expand_Int_Less: unhandled cmp target TN"));
  }

  return;
}

/* ====================================================================
 *   Expand_Int_Less_Equal
 * ====================================================================
 */
void
Expand_Int_Less_Equal (
  TN *dest,
  TN *src1,
  TN *src2,
  TYPE_ID desc,
  OPS *ops
)
{
  VARIANT variant;
  TOP action;
  switch (desc) {
  case MTYPE_I4: variant = V_BR_I4LE; break;
  case MTYPE_U4: variant = V_BR_U4LE; break;
  default:
    Is_True(FALSE, ("Expand_Int_Less_Equal: MTYPE_%s is not handled",
                                                  Mtype_Name(desc)));
  }

  if (TN_register_class(dest) == ISA_REGISTER_CLASS_branch) {
    action = Pick_Compare_TOP (&variant, &src1, &src2, FALSE, ops);
    Build_OP (action, dest, src1, src2, ops);
  }
  else if (TN_register_class(dest) == ISA_REGISTER_CLASS_integer) {
    action = Pick_Compare_TOP (&variant, &src1, &src2, TRUE, ops);
    Build_OP (action, dest, src1, src2, ops);
  }
  else {
    FmtAssert(FALSE, ("Expand_Int_Less_Equal: unhandled cmp target TN"));
  }

  return;
}

/* ====================================================================
 *   Expand_Int_Equal
 * ====================================================================
 */
void
Expand_Int_Equal (
  TN *dest,
  TN *src1,
  TN *src2,
  TYPE_ID desc,
  OPS *ops
)
{
  VARIANT variant;
  TOP action;
  switch (desc) {
  case MTYPE_I4: variant = V_BR_I4EQ; break;
  case MTYPE_U4: variant = V_BR_U4EQ; break;

  default:
    Is_True(FALSE, ("Expand_Int_Equal: MTYPE_%s is not handled",
                                                  Mtype_Name(desc)));
  }

  if (TN_register_class(dest) == ISA_REGISTER_CLASS_branch) {
    action = Pick_Compare_TOP (&variant, &src1, &src2, FALSE, ops);
    Build_OP (action, dest, src1, src2, ops);
  }
  else if (TN_register_class(dest) == ISA_REGISTER_CLASS_integer) {
    action = Pick_Compare_TOP (&variant, &src1, &src2, TRUE, ops);
    Build_OP (action, dest, src1, src2, ops);
  }
  else {
    FmtAssert(FALSE, ("Expand_Int_Equal: unhandled cmp target TN"));
  }

  return;
}

/* ====================================================================
 *   Expand_Int_Not_Equal
 * ====================================================================
 */
void
Expand_Int_Not_Equal (
  TN *dest,
  TN *src1,
  TN *src2,
  TYPE_ID desc,
  OPS *ops
)
{
  VARIANT variant;
  TOP   action;
  TN   *tmp1, *tmp2;

  FmtAssert(TN_register_class(src1) == ISA_REGISTER_CLASS_integer &&
            (!TN_is_register(src2) || TN_register_class(src2) == ISA_REGISTER_CLASS_integer),
	  ("Expand_Int_Not_Equal: operands have wrong RClass"));

  switch (desc) {
  case MTYPE_I4: variant = V_BR_I4NE; break;
  case MTYPE_U4: variant = V_BR_U4NE; break;
  default:
    #pragma mips_frequency_hint NEVER
    Is_True(FALSE, ("Expand_Int_Not_Equal: MTYPE_%s is not handled",
                                                   Mtype_Name(desc)));
  }

  if (TN_register_class(dest) == ISA_REGISTER_CLASS_branch) {
    action = Pick_Compare_TOP (&variant, &src1, &src2, FALSE, ops);
    Build_OP (action, dest, src1, src2, ops);
  }
  else if (TN_register_class(dest) == ISA_REGISTER_CLASS_integer) {
    action = Pick_Compare_TOP (&variant, &src1, &src2, TRUE, ops);
    Build_OP (action, dest, src1, src2, ops);
  }
  else {
    FmtAssert(FALSE, ("Expand_Int_Not_Equal: unhandled cmp target TN"));
  }

  return;
}

/* ====================================================================
 *   Expand_Int_Greater_Equal
 * ====================================================================
 */
void
Expand_Int_Greater_Equal (
  TN *dest,
  TN *src1,
  TN *src2,
  TYPE_ID desc,
  OPS *ops
)
{
  VARIANT variant;
  TOP action;
  switch (desc) {
  case MTYPE_I4: variant = V_BR_I4GE; break;
  case MTYPE_U4: variant = V_BR_U4GE; break;
  default:
    Is_True(FALSE, ("Expand_Int_Greater_Equal: MTYPE_%s is not handled",
                                                     Mtype_Name(desc)));
  }

  if (TN_register_class(dest) == ISA_REGISTER_CLASS_branch) {
    action = Pick_Compare_TOP (&variant, &src1, &src2, FALSE, ops);
    Build_OP (action, dest, src1, src2, ops);
  }
  else if (TN_register_class(dest) == ISA_REGISTER_CLASS_integer) {
    action = Pick_Compare_TOP (&variant, &src1, &src2, TRUE, ops);
    Build_OP (action, dest, src1, src2, ops);
  }
  else {
    FmtAssert(FALSE, ("Expand_Int_Greater_Equal: unhandled cmp target TN"));
  }

  return;
}

/* ====================================================================
 *   Expand_Int_Greater
 * ====================================================================
 */
void
Expand_Int_Greater (
  TN *dest,
  TN *src1,
  TN *src2,
  TYPE_ID desc,
  OPS *ops
)
{
  VARIANT variant;
  TOP action;

  switch (desc) {
    case MTYPE_I4: variant = V_BR_I4GT; break;
    case MTYPE_U4: variant = V_BR_U4GT; break;
    default:
      Is_True(FALSE, ("Expand_Int_Greater: MTYPE_%s is not handled",
                                                  Mtype_Name(desc)));
  }

  if (TN_register_class(dest) == ISA_REGISTER_CLASS_branch) {
    action = Pick_Compare_TOP (&variant, &src1, &src2, FALSE, ops);
    Build_OP (action, dest, src1, src2, ops);
  }
  else if (TN_register_class(dest) == ISA_REGISTER_CLASS_integer) {
    action = Pick_Compare_TOP (&variant, &src1, &src2, TRUE, ops);
    Build_OP (action, dest, src1, src2, ops);
  }
  else {
    FmtAssert(FALSE, ("Expand_Int_Greater: unhandled cmp target TN"));
  }

  return;
}

/* ====================================================================
 *   Expand_Ptr_Not_Equal
 *
 *   src1 and src2 are of MTYPE desc, one of pointer MTYPEs;
 * ====================================================================
 */
void
Expand_Ptr_Not_Equal (
  TN *dest,
  TN *src1,
  TN *src2,
  TYPE_ID desc,
  OPS *ops
)
{

  FmtAssert(FALSE,("not supported"));
  return;
}

/* ====================================================================
 *   Expand_Bool_Equal
 * ====================================================================
 */
void
Expand_Bool_Equal (
  TN *dest,
  TN *src1,
  TN *src2,
  OPS *ops
)
{
  FmtAssert(FALSE,("Not Implemented"));

}

/* ====================================================================
 *   Expand_Bool_Not_Equal
 * ====================================================================
 */
void
Expand_Bool_Not_Equal (
  TN *dest,
  TN *src1,
  TN *src2,
  OPS *ops
)
{
  FmtAssert(FALSE,("Not Implemented"));

}

/* ====================================================================
 *   Expand_Bool_To_Int
 * ====================================================================
 */
void
Expand_Bool_To_Int (
  TN *dest,
  TN *src,
  TYPE_ID rtype,
  OPS *ops
)
{
  Build_OP (TOP_mfb, dest, src, ops);
  return;
}

/* ====================================================================
 *   Expand_Float_To_Int
 *
 *   TODO how do you trap on float val too big for [u]int32?
 * ====================================================================
 */
static void
Expand_Float_To_Int (
  ROUND_MODE rm,
  TN *dest,
  TN *src,
  TYPE_ID imtype,
  TYPE_ID fmtype,
  OPS *ops
)
{
  FmtAssert(FALSE,("Not Implemented"));

  return;
}

/* ====================================================================
 *   Expand_Float_To_Int_Cvt
 * ====================================================================
 */
void
Expand_Float_To_Int_Cvt (
  TN *dest,
  TN *src,
  TYPE_ID imtype,
  TYPE_ID fmtype,
  OPS *ops
)
{
  FmtAssert(FALSE,("Not Implemented"));
}

/* ====================================================================
 *   Expand_Float_To_Int_Round
 * ====================================================================
 */
void
Expand_Float_To_Int_Round (
  TN *dest,
  TN *src,
  TYPE_ID imtype,
  TYPE_ID fmtype,
  OPS *ops
)
{
  FmtAssert(FALSE,("Not Implemented"));
}

/* ====================================================================
 *   Expand_Float_To_Int_Trunc
 * ====================================================================
 */
void
Expand_Float_To_Int_Trunc (
  TN *dest,
  TN *src,
  TYPE_ID imtype,
  TYPE_ID fmtype,
  OPS *ops
)
{
  FmtAssert(FALSE,("Not Implemented"));
}

/* ====================================================================
 *   Expand_Float_To_Int_Floor
 * ====================================================================
 */
void
Expand_Float_To_Int_Floor (
  TN *dest,
  TN *src,
  TYPE_ID imtype,
  TYPE_ID fmtype,
  OPS *ops
)
{
  FmtAssert(FALSE,("Not Implemented"));
}

/* ====================================================================
 *   Expand_Float_To_Int_Ceil
 * ====================================================================
 */
void
Expand_Float_To_Int_Ceil (
  TN *dest,
  TN *src,
  TYPE_ID imtype,
  TYPE_ID fmtype,
  OPS *ops
)
{
  FmtAssert(FALSE,("Not Implemented"));
}

/* ====================================================================
 *   Optimize_Select
 * ====================================================================
 */
static BOOL
Optimize_Select (
  VARIANT variant,
  TN *cond1,
  TN *cond2,
  TN *dest,
  TN *dest2,
  TN *src1,
  TN *src2,
  BOOL is_float,
  OPS *ops
)
{
  return FALSE;
}

/* ====================================================================
 *   Expand_Compare_And_Select
 * ====================================================================
 */
static void
Expand_Compare_And_Select (
  VARIANT variant,
  TN *cond1,
  TN *cond2,
  TN *dest,
  TN *opposite_dest,
  TN *true_tn,
  TN *false_tn,
  BOOL is_float,
  OPS *ops
)
{
  TYPE_ID mtype;

  // Look for special cases to optimize
  if (Optimize_Select(variant, cond1, cond2, dest, opposite_dest,
		      true_tn, false_tn, is_float, ops)) return;

  // Expand
  //
  //      dst, opposite_dst = (cond1 cmp cond2) ? true_tn : false_tn
  //
  // On this target we only support integer:
  // dst = (cond1 cmp cond2) ? true_tn:false_tn
  FmtAssert(opposite_dest == NULL, ("Not Implemented"));

  /* (cbr) can be
  FmtAssert(!is_float, ("Not Implemented"));
  */

  TOP cmp1 = Pick_Compare_TOP (&variant, &cond1, &cond2, FALSE, ops);
  TN *p1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch);

  Build_OP (cmp1, p1, cond1, cond2, ops);

  Expand_Select (dest, p1, true_tn, false_tn, MTYPE_I4, is_float, ops);

  return;
}

/* ====================================================================
 *   Expand_Select
 * ====================================================================
 */
void
Expand_Select (
  TN *dest_tn,
  TN *cond_tn,
  TN *true_tn,
  TN *false_tn,
  TYPE_ID mtype,
  BOOL float_cond,
  OPS *ops
)
{
  TOP top = TOP_UNDEFINED;
  //const BOOL is_float = MTYPE_is_float(mtype);

  if (true_tn == false_tn) {
    Exp_COPY (dest_tn, true_tn, ops);
    return;
  }

  if (TN_is_register(true_tn) && TN_register_class(true_tn) == ISA_REGISTER_CLASS_branch) {
    TN* tmp = Build_RCLASS_TN(ISA_REGISTER_CLASS_integer);
    Expand_Bool_To_Int (tmp, true_tn, mtype, ops);
    true_tn = tmp;
  }

  if (TN_is_register(false_tn) && TN_register_class(false_tn) == ISA_REGISTER_CLASS_branch) {
    TN* tmp = Build_RCLASS_TN(ISA_REGISTER_CLASS_integer);
    Expand_Bool_To_Int (tmp, false_tn, mtype, ops);
    false_tn = tmp;
  }
    
  if (TN_register_class(cond_tn) != ISA_REGISTER_CLASS_branch) {
    TN* tmp = Build_RCLASS_TN(ISA_REGISTER_CLASS_branch);
    Build_OP (TOP_mtb, tmp, cond_tn, ops);
    cond_tn = tmp;
  }

  // Process immediate operand
  if (TN_is_constant(true_tn)) {
    true_tn = Expand_Or_Inline_Immediate(true_tn, mtype, ops);
  }
  if (TN_is_constant(false_tn)) {
    false_tn = Expand_Or_Inline_Immediate(false_tn, mtype, ops);
  }
  
  if (TN_has_value(true_tn)) {
    if (TN_has_value(false_tn)) {
      TN *tmp;
      tmp = Expand_Immediate_Into_Register(MTYPE_I4, true_tn, ops);
      true_tn = tmp;
      top = TOP_slct_r;
    } else {
      TN *tmp = false_tn;
      false_tn = true_tn;
      true_tn = tmp;
      top = TOP_slctf_r;
    }
  } else {
    top = TOP_slct_r;
  }

  if (TN_has_value(false_tn)) 
    top = TOP_opnd_immediate_variant(top, 2, TN_value(false_tn));

  FmtAssert(top != TOP_UNDEFINED,
	    ("Expand_Select: unsupported"));
  
  if (TN_register_class(dest_tn) == ISA_REGISTER_CLASS_branch) {
    TN *tmp_dest_tn = Build_RCLASS_TN(ISA_REGISTER_CLASS_integer);
    Build_OP (top, tmp_dest_tn, cond_tn, true_tn, false_tn, ops);
    Build_OP(TOP_mtb, dest_tn, tmp_dest_tn, ops);
  }
  else {
    Build_OP (top, dest_tn, cond_tn, true_tn, false_tn, ops);
  }
}

/* ====================================================================
 *   Check_Select_Expansion
 *
 *   check whether to eval condition before select
 * ====================================================================
 */
BOOL
Check_Select_Expansion (
  OPCODE compare
)
{
  // in order to get optimal code,
  // don't evaluate the condition first,
  // but pass the condition and kids to exp_select,
  // which will do the compare and use the predicate results.
  return FALSE;
}

/* ====================================================================
 *   Exp_Select_And_Condition
 * ====================================================================
 */
void
Exp_Select_And_Condition (
  OPCODE select,
  TN *result,
  TN *true_tn,
  TN *false_tn,
  OPCODE compare,
  TN *cmp_kid1,
  TN *cmp_kid2,
  VARIANT variant,
  OPS *ops
)
{
  OPS newops = OPS_EMPTY;

  //  TOP cmp = Pick_Compare_TOP (&variant, &cmp_kid1, &cmp_kid2, &newops);

  switch (variant) {
  case V_BR_PEQ:
  case V_BR_PNE:
    {
      //      Is_True(cmp == TOP_UNDEFINED,
      //      ("unexpected compare op for V_BR_PEQ/V_BR_PNE"));

      FmtAssert(FALSE,("Not Implemented"));
    }
    break;
  case V_BR_NONE:
    #pragma mips_frequency_hint NEVER
    FmtAssert(FALSE, ("Exp_Select_And_Condition given br_none variant"));
    /*NOTREACHED*/
  default:
    //    FmtAssert(cmp != TOP_UNDEFINED,
    //                ("Exp_Select_And_Condition: unexpected comparison"));
    break;
  }

  Expand_Compare_And_Select (variant, cmp_kid1, cmp_kid2,
			     result, NULL, true_tn, false_tn,
			     MTYPE_is_float(OPCODE_rtype(select)), &newops);

  if (Trace_Exp) {
    #pragma mips_frequency_hint NEVER
    OP *op;
    fprintf(TFile, "Exp_Select_And_Condition:\n");
    FOR_ALL_OPS_OPs(&newops, op) {
      fprintf(TFile, " into ");
      Print_OP (op);
    }
  }

  OPS_Append_Ops(ops, &newops);
}

/* ====================================================================
 *   Expand_Min
 * ====================================================================
 */
void
Expand_Min (
  TN *dest,
  TN *src1,
  TN *src2,
  TYPE_ID mtype,
  OPS *ops
)
{
  TOP top = TOP_UNDEFINED;

  FmtAssert (TN_is_register(src1) || TN_is_register(src2),
	     ("Expand_Add: both operands const ?"));
  FmtAssert (Register_Class_For_Mtype(mtype) == ISA_REGISTER_CLASS_integer,
	     ("Unexpected MTYPE: %s", MTYPE_name(mtype)));
  FmtAssert (MTYPE_is_integral(mtype),
	     ("Unexpected MTYPE: %s", MTYPE_name(mtype)));
  
  
  if (TN_is_constant(src1)) {
    Expand_Min(dest, src2, src1, mtype, ops);
    return;
  }
   
  // Process immediate operand
  if (TN_is_constant(src2)) {
    src2 = Expand_Or_Inline_Immediate(src2, mtype, ops);
  }

  switch (mtype) {
  case MTYPE_I4:
    top = TOP_min_r;
    break;
  case MTYPE_U4:
    top = TOP_minu_r;
    break;
  default:
    FmtAssert(FALSE, ("Expand_Min: unexpected MTYPE %s", MTYPE_name(mtype)));
  }
  
  if (TN_has_value(src2))
    top = TOP_opnd_immediate_variant(top, 1, TN_value(src2));
  
  Build_OP(top, dest, src1, src2, ops);
  return;
}

/* ====================================================================
 *   Expand_Max
 * ====================================================================
 */
void
Expand_Max (
  TN *dest,
  TN *src1,
  TN *src2,
  TYPE_ID mtype,
  OPS *ops
)
{
  TOP top = TOP_UNDEFINED;

  FmtAssert (TN_is_register(src1) || TN_is_register(src2),
	     ("Expand_Add: both operands const ?"));
  FmtAssert (Register_Class_For_Mtype(mtype) == ISA_REGISTER_CLASS_integer,
	     ("Unexpected MTYPE: %s", MTYPE_name(mtype)));
  FmtAssert (MTYPE_is_integral(mtype),
	     ("Unexpected MTYPE: %s", MTYPE_name(mtype)));
  
  
  if (TN_is_constant(src1)) {
    Expand_Max(dest, src2, src1, mtype, ops);
    return;
  }
   
  // Process immediate operand
  if (TN_is_constant(src2)) {
    src2 = Expand_Or_Inline_Immediate(src2, mtype, ops);
  }

  switch (mtype) {
  case MTYPE_I4:
    top = TOP_max_r;
    break;
  case MTYPE_U4:
    top = TOP_maxu_r;
    break;
  default:
    FmtAssert(FALSE, ("Expand_Min: unexpected MTYPE %s", MTYPE_name(mtype)));
  }
  
  if (TN_has_value(src2))
    top = TOP_opnd_immediate_variant(top, 1, TN_value(src2));
  
  Build_OP(top, dest, src1, src2, ops);
  return;
}

/* ====================================================================
 *   Expand_MinMax
 *
 *   t = s1 < s2; d = select t ? s1 : s2; d2 = select t ? s2 : s1
 * ====================================================================
 */
void
Expand_MinMax (
  TN *dest,
  TN *dest2,
  TN *src1,
  TN *src2,
  TYPE_ID mtype,
  OPS *ops
)
{

  FmtAssert(FALSE,("MINMAX shouldn't have been reached"));

}

/* ====================================================================
 *   Expand_Float_Compares
 * ====================================================================
 */
static void
Expand_Float_Compares(
  TOP cmp_opcode,
  TN *dest,
  TN *src1,
  TN *src2,
  OPS *ops
)
{
  FmtAssert(FALSE,("Not Implemented"));
}

/* ====================================================================
 *   Expand_Float_Less
 * ====================================================================
 */
void
Expand_Float_Less (
  TN *dest,
  TN *src1,
  TN *src2,
  VARIANT variant,
  TYPE_ID mtype,
  OPS *ops
)
{
  Expand_Float_Compares(TOP_noop, dest, src1, src2, ops);
}

/* ====================================================================
 *   Expand_Float_Greater
 * ====================================================================
 */
void
Expand_Float_Greater (
  TN *dest,
  TN *src1,
  TN *src2,
  VARIANT variant,
  TYPE_ID mtype,
  OPS *ops
)
{
  Expand_Float_Compares(TOP_noop, dest, src1, src2, ops);
}

/* ====================================================================
 *   Expand_Floa_Less_Equal
 * ====================================================================
 */
void
Expand_Float_Less_Equal (
  TN *dest,
  TN *src1,
  TN *src2,
  VARIANT variant,
  TYPE_ID mtype,
  OPS *ops
)
{
  Expand_Float_Compares(TOP_noop, dest, src1, src2, ops);
}

/* ====================================================================
 *   Expand_Float_Greater_Equal
 * ====================================================================
 */
void
Expand_Float_Greater_Equal (
  TN *dest,
  TN *src1,
  TN *src2,
  VARIANT variant,
  TYPE_ID mtype,
  OPS *ops
)
{
  Expand_Float_Compares(TOP_noop, dest, src1, src2, ops);
}

/* ====================================================================
 *   Expand_Float_Equal
 * ====================================================================
 */
void
Expand_Float_Equal (
  TN *dest,
  TN *src1,
  TN *src2,
  VARIANT variant,
  TYPE_ID mtype,
  OPS *ops
)
{
  Expand_Float_Compares(TOP_noop, dest, src1, src2, ops);
}

/* ====================================================================
 *   Expand_Float_Not_Equal
 * ====================================================================
 */
void
Expand_Float_Not_Equal (
  TN *dest,
  TN *src1,
  TN *src2,
  VARIANT variant,
  TYPE_ID mtype,
  OPS *ops
)
{
  Expand_Float_Compares(TOP_noop, dest, src1, src2, ops);
}

/* ====================================================================
 *   Exp_ST220_Sqrt
 * ====================================================================
 */
static void
Expand_ST220_Sqrt (
  TN *result,
  TN *src,
  TYPE_ID mtype,
  OPS *ops
)
{
  FmtAssert(FALSE,("Not Implemented"));

  return;
}

/* ====================================================================
 *   Exp_Sqrt
 * ====================================================================
 */
void
Expand_Sqrt (
  TN *result,
  TN *src,
  TYPE_ID mtype,
  OPS *ops
)
{
  static BOOL initialized;
  static void (*exp_sqrt)(TN *, TN *, TYPE_ID, OPS *) = Expand_ST220_Sqrt;

  FmtAssert(FALSE,("Not Implemented"));

  if (!initialized) {
    const char * const alg = CGEXP_sqrt_algorithm;
    if (strcasecmp(alg, "st220") == 0) {
      exp_sqrt = Expand_ST220_Sqrt;
    } else {
      DevWarn("invalid fdiv algorithm: %s", alg);
    }
    initialized = TRUE;
  }

  exp_sqrt(result, src, mtype, ops);
}

/* ====================================================================
 *   Exp_Recip_Sqrt
 * ====================================================================
 */
void
Expand_Recip_Sqrt (
  TN *result,
  TN *src,
  TYPE_ID mtype,
  OPS *ops
)
{
  FmtAssert(FALSE,("Not Implemented"));

  return;
}

/* ======================================================================
 *   Exp_COPY
 *
 *   Generate a register transfer copy from 'src_tn' to 'tgt_tn'.
 * ======================================================================*/
void
Exp_COPY (
  TN *tgt_tn,
  TN *src_tn,
  OPS *ops
)
{
  Expand_Copy (tgt_tn, NULL, src_tn, ops);
  return;
}

/* ======================================================================
 *   Expand_TOP_intrncall
 *
 *   Given a TOP_intrncall <op>, expand it into the sequence of instructions
 *   that must be generated. If <get_sequence_length> is TRUE, return only
 *   the number of instructions in the sequence and don't actually do the
 *   expansion.
 * ======================================================================
 */
static INT
Expand_TOP_intrncall (
  const OP *op,
  OPS *ops,
  BOOL get_sequence_length,
  INT pc_value
)
{
  FmtAssert(FALSE, ("Expand_TOP_intrncall NYI"));
  /*NOTREACHED*/
}

/* ======================================================================
 *   Exp_Intrinsic_Call
 *
 *   initial expansion of intrinsic call (may not be complete lowering).
 *   return result TN (if set).
 *   If the intrinsic requires a label and loop (2 bb's)
 *   then ops is for first bb and ops2 is for bb after the label.
 *   Otherwise only ops is filled in.
 * ======================================================================
 */
TN *
Exp_Intrinsic_Call (
  INTRINSIC id,
  TN *op0,
  TN *op1,
  TN *op2,
  OPS *ops,
  LABEL_IDX *label,
  OPS *loop_ops
)
{
    TN *result = NULL ;

    /*
     * Currently treated as intrinsics calls:
     *  - D-cache intrinsics management
     * See WHIRL documentation for difference between ops and calls.
     */

    switch(id) {    
    case INTRN_ST220PFT:
    case INTRN_ST220PRGADD:
    case INTRN_ST220PRGSET:
	{
	    /* For these, input is offset and base address, output is effective address used */
	    TN *in[2] ;
	    TN *out[1] ;
	    in[0] = op0 ;
	    in[1] = op1 ;
	    result = out[0] = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
	    /* We fall back to the usual intrinsic generation */
	    Exp_Intrinsic_Op(id, 1, 2, out, in, ops) ;
	    FmtAssert(result,("Exp_Intrinsic_Call (INTRN_ST220PFT|INTRN_ST220PRGADD|ST220PRGSET): Invalid expansion."));
	}
	break;
    case INTRN_ST220PRGINS:
    case INTRN_ST220SYNC:
    case INTRN_ST220SYNCINS:
	{
	    /* For these, no input nor output*/
	    /* We fall back to the usual intrinsic generation */
	    Exp_Intrinsic_Op(id, 0, 0, NULL, NULL, ops) ;
	}
	break ;
    case INTRN_ST220SYSCALL:
	{
	    /* This one has one input, hopefully constant and no output*/
	    TN *in[1] ;
	    in[0] = op0 ;
	    /* We fall back to the usual intrinsic generation */
	    Exp_Intrinsic_Op(id, 0, 1, NULL, in, ops) ;
	}
	break ;
    default:
	FmtAssert(FALSE,("Exp_Intrinsic_Call : Not Implemented."));
    }
  return result ;
}

/* ======================================================================
 *   Exp_Simulated_Op
 *
 *   Given a simulated <op>, expand it into the sequence of instructions
 *   supported by the target.
 * ======================================================================
 */
void
Exp_Simulated_Op (
  const OP *op,
  OPS *ops,
  INT pc_value,
  ST **symbol_def_after_group
)
{
  OP *newop;

  if (OP_code(op) != TOP_getpc) {
    FmtAssert(0, ("Exp_Simulated_Op for top TOP_intrncall should not be reached"));
  }

  switch (OP_code(op)) {

  case TOP_intrncall:
    Expand_TOP_intrncall(op, ops, FALSE, pc_value);
    break;

  case TOP_spadjust:
    // spadjust should only show up for alloca/dealloca
    if (OP_variant(op) == V_SPADJUST_PLUS) {
      Expand_Add (OP_result(op,0), OP_opnd(op,0),
                                    OP_opnd(op,1), Pointer_Mtype, ops);
    }
    else {
      Expand_Sub (OP_result(op,0), OP_opnd(op,0),
                                    OP_opnd(op,1), Pointer_Mtype, ops);
    }

    FOR_ALL_OPS_OPs(ops, newop) {
      OP_srcpos(newop) = OP_srcpos(op);
    }
    break;

  case TOP_getpc:
    Build_OP (TOP_call, OP_result(op, 0), OP_opnd (op, 0), ops);
    *symbol_def_after_group = TN_var( OP_opnd (op, 0));
    FOR_ALL_OPS_OPs(ops, newop) {
      OP_srcpos(newop) = OP_srcpos(op);
      Set_OP_bundled (newop);
      if (OP_end_group (op)) Set_OP_end_group (newop);
      OP_scycle (newop) = OP_scycle(op);
    }
    break;

  default:
    FmtAssert(FALSE, ("simulated OP %s not handled", TOP_Name(OP_code(op))));
    /*NOTREACHED*/
  }

}

/* ======================================================================
 * Simulated_Op_Real_Ops
 *
 * Return the number of instructions that will be generated by Exp_Simulated_Op
 * ======================================================================*/
INT
Simulated_Op_Real_Ops(const OP *op)
{
  switch (OP_code(op)) {
  case TOP_intrncall:
    return Expand_TOP_intrncall (op, NULL, TRUE, 0);
  case TOP_spadjust:
  case TOP_getpc:
    return 1;
  default:

    /* Anything other than the above is presumed to be removed by
     * emit time, therefore we just say the expansion generates 0 ops.
     * (we used to assert, but that isn't a good solution -- see pv 652898).
     */
    return 0;
  }
}

/* ======================================================================
 * Simulated_Op_Real_Inst_Words
 *
 * Return the number of instruction words that will ultimately be emitted
 * for the expansion generated by Exp_Simulated_Op
 * ======================================================================*/
INT
Simulated_Op_Real_Inst_Words(const OP *op)
{
    switch (OP_code(op)) {
    case TOP_spadjust:
    case TOP_getpc:
	return 1;
    case TOP_asm:
	// this is a hack; will be a multiple of 3, but don't know
	// exact number.
	return 3;
    default:
    	// We should never emit a simulated OP, so just assert.
    	FmtAssert(FALSE, ("shouldn't be calling Simulated_Op_Real_Inst_Words for %s", TOP_Name(OP_code(op)) ));
    	/*NOTREACHED*/
    }
}

/* ======================================================================
 *   Exp_Is_Large_Stack_Sym
 *
 *   determine if a given symbol is a stack relative reference that will
 *   require multiple instructions to load or store.
 * ======================================================================*/
BOOL
Exp_Is_Large_Stack_Sym (
  ST* sym,
  INT64 ofst
)
{
  ST *base_sym;
  INT64 base_ofst;

  if (sym == NULL) {
    return FALSE;
  }

  Allocate_Object(sym);		/* make sure sym is allocated */

  Base_Symbol_And_Offset_For_Addressing (sym, ofst, &base_sym, &base_ofst);

  if ((base_sym == SP_Sym || base_sym == FP_Sym) &&
      !ISA_LC_Value_In_Class(base_ofst, LC_s9)) {
    return TRUE;
  }
  return FALSE;
}

/* ====================================================================
 *   Exp_Spadjust
 * ====================================================================
 */
void
Exp_Spadjust (
  TN *dest,
  TN *size,
  VARIANT variant,
  OPS *ops
)
{
  Build_OP (TOP_spadjust, dest, SP_TN, size, ops);
  OP_variant(OPS_last(ops)) = variant;
}


/* ====================================================================
 *   Exp_Noop
 * ====================================================================
 */
void
Exp_Noop (OPS *ops)
{
  // On this machine there is only one noop:
  Build_OP (TOP_nop, ops);
}

/* ====================================================================
 *   Exp_GP_Init
 * ====================================================================
 */
void
Exp_GP_Init (
  TN *dest,
  ST *fn_st,
  OPS *ops
)
{
  static INT Temp_Index = 0;

  TN *call_dest_tn;
  TN *neg_gprel_tn;
 
  // Generate a symbol for the label to call.  It has
  // to be a symbol (not a label) because we need to
  // apply a reloc to it later.
  ST *st = New_ST (CURRENT_SYMTAB);
  STR_IDX str_idx = Save_Str2i ("L?", ".gpinit_", Temp_Index++);
  ST_Init (st, str_idx, CLASS_NAME, SCLASS_UNKNOWN, EXPORT_LOCAL,
	  ST_pu (fn_st));
  call_dest_tn = Gen_Symbol_TN (st, 0, 0);
  Build_OP (TOP_getpc, RA_TN, call_dest_tn, ops);

  neg_gprel_tn = Gen_Symbol_TN (st, 0, TN_RELOC_NEG_GOT_DISP);
  Expand_Add (dest, RA_TN, neg_gprel_tn, Pointer_Mtype, ops);
}

/* ====================================================================
 *   Expand_Const
 * ====================================================================
 */
void
Expand_Const (
  TN *dest,
  TN *src,
  TYPE_ID mtype,
  OPS *ops
)
{
  FmtAssert(TN_is_symbol(src), ("Expand_Const: src not a symbol TN"));

  TCON tc = ST_tcon_val(TN_var(src));
  //
  // This is called normally for floating-point and company
  // Since we keep them in integer registers, just make a mov
  //

  switch (TCON_ty(tc)) {

  case MTYPE_F4:
    Build_OP(TOP_mov_i, dest, src, ops);
    break;

  default:
    extern void dump_tn (TN*);
    dump_tn (dest);
    dump_tn (src);
    FmtAssert(FALSE,("unsupported type %s", MTYPE_name(TCON_ty(tc))));
  }
  return;
}

