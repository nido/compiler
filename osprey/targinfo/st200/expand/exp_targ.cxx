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

#include "topcode.h"
#include "targ_isa_lits.h"
#include "targ_isa_properties.h"

BOOL Reuse_Temp_TNs = FALSE;

/* Disable conversion of constant integer multiplies into shift/adds:*/
static BOOL Disable_Const_Mult_Opt = FALSE;

/* Dup_TN won't dup a dedicated tn, but for our purposes we
 * can just re-use the dedicated tn.  Don't want to re-use a
 * symbolic tn or it will mess up live ranges. */
/* DOESN'T WORK:  causes problems in Create_lvs because it causes
 * a use of a parm reg at the call-site, so it looks like the
 * parm-reg is incoming at the call?  This probably should work,
 * but for now we can use other routine that create a real dup tn. */
#define DUP_TN(tn)	Dup_TN_Even_If_Dedicated(tn)

/* ====================================================================
 *   WN_intrinsic_return_ty
 *
 *   Similar to the one in be/whirl2c/wn_attr.cxx
 *   TODO: move to the intrn_info.cxx or something.
 * ====================================================================
 */
static TY_IDX
WN_intrinsic_return_ty (
  INTRINSIC intr_opc
)
{
   TY_IDX ret_ty;

   Is_True(INTRINSIC_FIRST<=intr_opc && intr_opc<=INTRINSIC_LAST,
     ("Exp_Intrinsic_Op: Intrinsic Opcode (%d) out of range", intr_opc));

   switch (INTRN_return_kind(intr_opc))
   {
   case IRETURN_UNKNOWN:
     FmtAssert(FALSE,("Exp_Intrinsic_Op: cannot have UNKNOWN IRETURN type"));
     break;
   case IRETURN_V:
      ret_ty = MTYPE_To_TY(MTYPE_V);
      break;
   case IRETURN_I1:
      ret_ty = MTYPE_To_TY(MTYPE_I1);
      break;
   case IRETURN_I2:
      ret_ty = MTYPE_To_TY(MTYPE_I2);
      break;
   case IRETURN_I4:
      ret_ty = MTYPE_To_TY(MTYPE_I4);
      break;
   case IRETURN_I8:
      ret_ty = MTYPE_To_TY(MTYPE_I8);
      break;
   case IRETURN_U1:
      ret_ty = MTYPE_To_TY(MTYPE_U1);
      break;
   case IRETURN_U2:
      ret_ty = MTYPE_To_TY(MTYPE_U2);
      break;
   case IRETURN_U4:
      ret_ty = MTYPE_To_TY(MTYPE_U4);
      break;
   case IRETURN_U8:
      ret_ty = MTYPE_To_TY(MTYPE_U8);
      break;
   case IRETURN_F4:
      ret_ty = MTYPE_To_TY(MTYPE_F4);
      break;
   case IRETURN_F8:
      ret_ty = MTYPE_To_TY(MTYPE_F8);
      break;
   case IRETURN_FQ:
      ret_ty = MTYPE_To_TY(MTYPE_FQ);
      break;
   case IRETURN_C4:
      ret_ty = MTYPE_To_TY(MTYPE_C4);
      break;
   case IRETURN_C8:
      ret_ty = MTYPE_To_TY(MTYPE_C8);
      break;
   case IRETURN_CQ:
      ret_ty = MTYPE_To_TY(MTYPE_CQ);
      break;
      /*
   case IRETURN_PV:
      ret_ty = Stab_Pointer_To(Stab_Mtype_To_Ty(MTYPE_V));
      break;
   case IRETURN_PU1:
      ret_ty = Stab_Pointer_To(Stab_Mtype_To_Ty(MTYPE_U1));
      break;
   case IRETURN_DA1:
      ret_ty = WN_Tree_Type(WN_kid0(call));
      break;
      */
   default:
      Is_True(FALSE,
	      ("Unexpected INTRN_RETKIND in WN_intrinsic_return_ty()"));
      ret_ty = MTYPE_To_TY(MTYPE_V);
      break;
   }

   return ret_ty;
} /* WN_intrinsic_return_ty */

/* ====================================================================
 *   Pick_Imm_Form_TOP (regform)
 * ====================================================================
 */
static TOP
Pick_Imm_Form_TOP (
  TOP regform
)
{
  switch (regform) {
    case TOP_xor_r: return TOP_xor_i;
    case TOP_and_r: return TOP_and_i;

    default: return regform;
  }
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

  if (TN_is_constant(src_tn)) {
    FmtAssert (TN_has_value(src_tn), ("Expand_Copy: illegal source tn"));
  }

  ISA_REGISTER_CLASS tgt_rc = TN_register_class(tgt_tn);
  ISA_REGISTER_CLASS src_rc = TN_register_class(src_tn);

  switch (src_rc) {

    case ISA_REGISTER_CLASS_integer:

      if (tgt_rc == ISA_REGISTER_CLASS_integer) {
	Build_OP(TOP_mov_r, tgt_tn, src_tn, ops);
      }
      else if (tgt_rc == ISA_REGISTER_CLASS_branch) {
	Build_OP(TOP_mtb, tgt_tn, src_tn, ops);
      }
      else {
	FmtAssert(FALSE,("Expand_Copy: not supported"));
      }
      break;

    case ISA_REGISTER_CLASS_branch:

      if (tgt_rc == ISA_REGISTER_CLASS_branch) {
	FmtAssert(FALSE,("Expand_Copy: b -> b ??"));
      }
      else if (tgt_rc == ISA_REGISTER_CLASS_integer) {
	Build_OP(TOP_mfb, tgt_tn, src_tn, ops);
      }
      else {
	FmtAssert(FALSE,("Expand_Copy: not supported"));
      }
      break;

    default:

      FmtAssert(FALSE,
         ("Expand_Copy: unsupported copy %s -> %s",
	    ISA_REGISTER_CLASS_INFO_Name(ISA_REGISTER_CLASS_Info(src_rc)),
	    ISA_REGISTER_CLASS_INFO_Name(ISA_REGISTER_CLASS_Info(tgt_rc))));
  }

  Set_OP_copy (OPS_last(ops));
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
	TN *tmp = Build_RCLASS_TN(ISA_REGISTER_CLASS_integer);
	Build_OP (TOP_shl_i, tmp, src, Gen_Literal_TN(24,4), ops);
	Build_OP (TOP_shru_i, dest, tmp, Gen_Literal_TN(24,4), ops);
      }
    }
    else if (new_length == 16) {
      // zero extension requires a sequence of shifts:
      if (signed_extension) {
          Build_OP (TOP_sxth_r, dest, src, ops);
      }
      else {
	TN *tmp = Build_RCLASS_TN(ISA_REGISTER_CLASS_integer);
	Build_OP (TOP_shl_i, tmp, src, Gen_Literal_TN(16,4), ops);
	Build_OP (TOP_shru_i, dest, tmp, Gen_Literal_TN(16,4), ops);
      }
    }
    else
        FmtAssert(FALSE, ("Expand_Convert_Length: unknown dest extension"));
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
 *   Exp_Immediate (dest, src, mtype, ops)
 * ====================================================================
 */
void
Exp_Immediate (
  TN *dest,
  TN *src,
  TYPE_ID mtype,
  OPS *ops
)
{
  INT64 val;
  TOP   top;
  TN    *tmp;

  if (TN_has_value(src)) {
    val = TN_value(src);
  }
  else if (TN_is_symbol(src)) {

    FmtAssert(FALSE,("Exp_Immediate: symbol TN"));

    ST *base;
    Base_Symbol_And_Offset_For_Addressing (TN_var(src),
                                      TN_offset(src), &base, &val);
    // now val is the "real" offset
  }

  switch (mtype) {

    case MTYPE_B:
      FmtAssert(FALSE,("Exp_Immediate: MTYPE_B not implemented"));
      break;

    case MTYPE_I1:
    case MTYPE_I2:
    case MTYPE_I4:
    case MTYPE_U1:
    case MTYPE_U2:
    case MTYPE_U4:
    case MTYPE_A4:
      if (ISA_LC_Value_In_Class (val, LC_s32)) {
	Build_OP (TOP_mov_i, dest, src, ops);
      }
      else {
	FmtAssert(0, ("Exp_Immediate: large immediate value"));
      }
      break;

    default:
      FmtAssert(0, ("Exp_Immediate: unknown MTYPE %s", MTYPE_name(mtype)));
  }

  return;
}

/* ====================================================================
 *   Expand Immediate
 * ====================================================================
 */
void
Expand_Immediate (TN *dest, TN *src, TYPE_ID mtype, OPS *ops)
{
  FmtAssert((TN_is_constant(src)),
	    ("unexpected non-constant in Expand_Immediate"));
  FmtAssert((TN_has_value(src) || TN_is_symbol(src)),
	    ("expected value or const in Expand_Immediate"));
  Exp_Immediate (dest, src, mtype, ops);
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

  if (TN_is_constant(src1)) {
    // switch order of src so immediate is second
    Expand_Add (result, src2, src1, mtype, ops);
    return;
  }

  if (TN_is_constant(src2)) {

    if (TN_has_value(src2)) {
      INT64 val = TN_value(src2);
      if (val == 0) {
	Expand_Copy (result, NULL, src1, ops);
	return;
      }
      new_opcode = TOP_add_i;
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

      // If src1 (base) is gp, I am dealing with a gp-relative
      // address production
      if (TN_is_gp_reg(src1) && ofst != 0) {
	// Should be add_i because src2 must have a reloc ...
	Build_OP(TOP_add_i, tmp, src1, src2, ops);
	src1 = tmp;
	src2 = Gen_Literal_TN(ofst, Pointer_Size);
      }
      else {
        // just makea and hope that 16bit offset suffices:
        Build_OP (TOP_mov_i, tmp, src2, ops);
        src2 = tmp;
      }

      new_opcode = TOP_add_r;
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

  if (TN_is_constant(src1)) {
    new_opcode = TOP_sub_i;
  }
  else if (TN_is_constant(src2)) {
    if (TN_has_value(src2)) {
      FmtAssert(ISA_LC_Value_In_Class(TN_value(src2), LC_s32),
		("Expand_Sub -- immediate field overflowed"));

      //
      // Only 32 bits matter anyway
      //
      INT64 val = (mINT32)(-TN_value(src2));
      src2 = Gen_Literal_TN(val, 4);
      new_opcode = TOP_add_i;
    }
    else {
      src2 = Expand_Immediate_Into_Register (mtype, src2, ops);
      new_opcode = TOP_sub_r;
    }
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
  switch (mtype) {
  case MTYPE_I1:
  case MTYPE_U1:
  case MTYPE_I2:
  case MTYPE_U2:
  case MTYPE_I4:
  case MTYPE_U4:
    //
    // generate : sub dest = $r0.0, src
    //
    Build_OP (TOP_sub_r, dest, Zero_TN, src, ops);
    break;

  default:
    FmtAssert(FALSE, ("Expand_Neg: not handled mtype %s\n",
                                             MTYPE_name(mtype)));
  }

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
  switch (mtype) {

  case MTYPE_I1:
  case MTYPE_I2:
  case MTYPE_I4:
    {
      //
      // For dest = abs (src) generate:
      //  
      //            sub   negx = $r0.0, src
      //            cmpge cond = src, $r0.0
      //            slct  dest = cond, src, negx
      //
      TN *negx = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer);
      TN *cond = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch);
      Build_OP (TOP_sub_r, negx, Zero_TN, src, ops);
      Build_OP (TOP_cmpge_r_b, cond, src, Zero_TN, ops);
      Build_OP (TOP_slct_r, dest, cond, src, negx, ops);
    }
    break;

  case MTYPE_F4:
    {
      TN *mask = Gen_Literal_TN(0x7fffffff, 4);
      Build_OP (TOP_and_i, dest, src, mask, ops);
    }
    break;

  default:
    Is_True(FALSE,("abs doesn't handle MTYPE_%s\n", MTYPE_name(mtype)));
  }
}

/* ====================================================================
 *   Pick_Left_Shift
 * ====================================================================
 */
static TOP
Pick_Left_Shift (
  TYPE_ID mtype,
  TN    **src2,
  OPS    *ops
)
{
  TOP top = TOP_UNDEFINED;

  if (TN_has_value(*src2)) {
    UINT64 val = TN_value(*src2);
    top = TOP_shl_i;
  }
  else {
    top = TOP_shl_r;
  }

  return top;
}

/* ====================================================================
 *   Pick_ARight_Shift
 * ====================================================================
 */
static TOP
Pick_ARight_Shift (
  TYPE_ID mtype,
  TN    **src2,
  OPS    *ops
)
{
  TOP top = TOP_UNDEFINED;

  if (TN_has_value(*src2)) {
    UINT64 val = TN_value(*src2);
    top = TOP_shr_i;
  }
  else {
    top = TOP_shr_r;
  }

  return top;
}

/* ====================================================================
 *   Pick_LRight_Shift
 * ====================================================================
 */
static TOP
Pick_LRight_Shift (
  TYPE_ID mtype,
  TN    **src2,
  OPS    *ops
)
{
  TOP top = TOP_UNDEFINED;

  if (TN_has_value(*src2)) {
    UINT64 val = TN_value(*src2);
    top = TOP_shru_i;
  }
  else {
    top = TOP_shru_r;
  }

  return top;
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

  UINT wordsize = MTYPE_is_size_double(mtype) ? 64 : 32;

  FmtAssert(TN_is_register(src1),
	      ("Expand_Shift: 1st operand must be register"));

  switch (kind) {
    case shift_left:
      new_opcode = Pick_Left_Shift (mtype, &src2, ops);
      break;
    case shift_aright:
      new_opcode = Pick_ARight_Shift (mtype, &src2, ops);
      break;
    case shift_lright:
      new_opcode = Pick_LRight_Shift (mtype, &src2, ops);
      break;
  }

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
  FmtAssert(TN_has_value(src2),
	    ("Expand_Shift_Multiply: second TN is not immediate"));
  FmtAssert(MTYPE_is_class_integer(mtype),
	    ("Expand_Shift_Multiply: branch mtype not supported"));

  // see if I can do a shift instead of the multiply
  if (Is_Power_Of_2(TN_value(src2), mtype)) {
    INT val = Get_Power_Of_2(TN_value(src2), mtype);
    TN *tmp = Gen_Literal_TN (val, MTYPE_byte_size(mtype));
    Expand_Shift (result, src1, tmp, mtype, shift_left, ops);
    
    // do we need to negate the result ?
    if (TN_value(src2) < 0) {
      Expand_Neg (result, result, mtype, ops);      
    }

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
  TN      *dest = result;
  TYPE_ID  mtype = rmtype;
  BOOL     has_const = FALSE;
  TOP opcode = TOP_UNDEFINED;

  //
  // Check for two constants
  //
  if ((TN_has_value(src1) || TN_is_rematerializable(src1)) &&
      (TN_has_value(src2) || TN_is_rematerializable(src2))) {
    // Two constants can sometimes occur because of DIVREM production in
    TN *val_tn;
    INT64    constant;
    constant = TN_has_value(src1) ?
                           TN_value(src1) : WN_const_val(TN_home(src1));
    constant *= TN_has_value(src2) ?
                           TN_value(src2) : WN_const_val(TN_home(src2));
    // Need to get the constant of the right length
    constant = Targ_To_Host(Host_To_Targ(rmtype, constant));
    val_tn = Gen_Literal_TN(constant, 8);
    Exp_Immediate(result, val_tn, MTYPE_is_signed(rmtype), ops);
    return;
  }

  FmtAssert(MTYPE_is_class_integer(rmtype), ("Expand_Multiply: mtype ?"));

  // If one of the operands is a constant, it must be the second one.
  if (TN_has_value(src1)) {
    TN *tmp = src2;
    src2 = src1;
    src1 = tmp;
    has_const = TRUE;
  }

  if (TN_has_value(src2)) {
    if (Expand_Shift_Multiply (dest, mtype, src1, src2, ops)) 
      return;
    has_const = TRUE;
  }
        
  BOOL is_signed = FALSE;

  switch (mtype) {
  case MTYPE_U2:
  case MTYPE_I2:
    // short <- short * short */
    if (s1mtype == MTYPE_U2 && s2mtype == MTYPE_U2) {
      opcode = has_const ? TOP_mulllu_i : TOP_mulllu_r;
      Build_OP(opcode, dest, src1, src2, ops);
    }

    else if (s1mtype == MTYPE_I2 && s2mtype == MTYPE_I2) {
      opcode = has_const ? TOP_mulll_i : TOP_mulll_r;
      Build_OP(opcode, dest, src1, src2, ops);
    }

    else if (s1mtype == MTYPE_I2 && s2mtype == MTYPE_U2) {
      Build_OP(TOP_sxth_r, src1, src1, ops);
      opcode = has_const ? TOP_mullu_i : TOP_mullu_r;
      Build_OP(opcode, dest, src1, src2, ops);
    }

    else if (s1mtype == MTYPE_U2 && s2mtype == MTYPE_I2) {
      FmtAssert (! has_const, ("Expand_Multiply: mult with const."));
      Build_OP(TOP_sxth_r, src2, src2, ops);
      opcode = has_const ? TOP_mullu_i : TOP_mullu_r;
      Build_OP(opcode, dest, src1, src2, ops);
    }

    else {
      FmtAssert(FALSE, ("Expand_Multiply: MTYPE_UI2 (%d = %d * %d)",
                        rmtype, s1mtype, s2mtype));
    }

    if (mtype == MTYPE_U2) {
      TN* const_tn = Gen_Literal_TN ((INT32) 65535, 4);
      Build_OP(TOP_and_i, dest, dest, const_tn, ops);
    }
    else if (mtype == MTYPE_I2) {
      Build_OP(TOP_sxth_r, dest, dest, ops);
    }
    break;

  case MTYPE_I4:
    is_signed = TRUE;
  case MTYPE_U4:
    // first handle optimised special cases

    // word <- half * half
    if (s1mtype == MTYPE_U2 && s2mtype == MTYPE_U2) {
      opcode = has_const ? TOP_mulllu_i : TOP_mulllu_r;
      Build_OP(opcode, dest, src1, src2, ops);
    }
    else if (s1mtype == MTYPE_I2 && s2mtype == MTYPE_I2) {
      opcode = has_const ? TOP_mulll_i : TOP_mulll_r;
      Build_OP(opcode, dest, src1, src2, ops);
    }
    else if (s1mtype == MTYPE_I2 && s2mtype == MTYPE_U2) {
      Build_OP(TOP_sxth_r, src1, src1, ops);
      opcode = has_const ? TOP_mullu_i : TOP_mullu_r;
      Build_OP(opcode, dest, src1, src2, ops);
    }
    else if (s1mtype == MTYPE_U2 && s2mtype == MTYPE_I2) {
      Build_OP(TOP_sxth_r, src2, src2, ops);
      opcode = has_const ? TOP_mullu_i : TOP_mullu_r;
      Build_OP(opcode, dest, src1, src2, ops);
    }

    // word <- word * word
    else if ((s1mtype == MTYPE_U4 && s2mtype == MTYPE_U4) ||
             (s1mtype == MTYPE_I4 && s2mtype == MTYPE_I4) ||
             (s1mtype == MTYPE_I4 && s2mtype == MTYPE_U4) ||
             (s1mtype == MTYPE_U4 && s2mtype == MTYPE_I4)) {
      TN *tmp1 = Build_RCLASS_TN(ISA_REGISTER_CLASS_integer);
      TN *tmp2 = Build_RCLASS_TN(ISA_REGISTER_CLASS_integer);
      opcode = has_const ? TOP_mullu_i : TOP_mullu_r;
      Build_OP(opcode, tmp1, src1, src2, ops);
      opcode = has_const ? TOP_mulhs_i : TOP_mulhs_r;
      Build_OP(opcode, tmp2, src1, src2, ops);
      Build_OP(TOP_add_r, dest, tmp1, tmp2, ops);
    }

    // word <- half * word
    else {
      // if we have a word argument. It can only be at pos1.
      // unless it is a constant with size < 65535.
      if (s2mtype == MTYPE_I4 || s2mtype != MTYPE_U4) {
        if (has_const) {
          if (TN_value(src2) > 65535) {
            TN *tmp = Build_RCLASS_TN(ISA_REGISTER_CLASS_integer);
            Build_OP (TOP_mov_i, tmp, src2, ops);
            src2 = tmp;
            has_const = false;
          }
        }
        else {
          TN *tmp = src2;
          src2 = src1;
          src1 = tmp;
        }
      }

      if (s1mtype == MTYPE_I1) {
        Build_OP(TOP_sxtb_r, src1, src1, ops);
      }
      else if (s1mtype == MTYPE_I2) {
        Build_OP(TOP_sxth_r, src1, src1, ops);
      }
      if (s2mtype == MTYPE_I1) {
        Build_OP(TOP_sxtb_r, src2, src2, ops);
      }
      else if (s2mtype == MTYPE_I2) {
        Build_OP(TOP_sxth_r, src2, src2, ops);
      }


      if (s1mtype == MTYPE_U1) {
        TN* const_tn = Gen_Literal_TN ((INT32) 255, 4);
        Build_OP(TOP_and_i, src1, src1, const_tn, ops);
      }
      else if (s1mtype == MTYPE_U2) {
        TN* const_tn = Gen_Literal_TN ((INT32) 65535, 4);
        Build_OP(TOP_and_i, src1, src1, const_tn, ops);
      }
      if (s2mtype == MTYPE_U1) {
        TN* const_tn = Gen_Literal_TN ((INT32) 255, 4);
        Build_OP(TOP_and_i, src2, src2, const_tn, ops);
      }
      else if (s2mtype == MTYPE_U2) {
        TN* const_tn = Gen_Literal_TN ((INT32) 65535, 4);
        Build_OP(TOP_and_i, src2, src2, const_tn, ops);
      }
      
      if (is_signed)
        opcode = has_const ? TOP_mull_i : TOP_mull_r;
      else
        opcode = has_const ? TOP_mullu_i : TOP_mullu_r;        
        
      Build_OP(opcode, dest, src1, src2, ops);
    }
    break;

  default:
    FmtAssert(FALSE, ("Expand_Multiply: --"));
  }

  if (opcode == TOP_UNDEFINED)
    FmtAssert(FALSE, ("Expand_Multiply: failed"));

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
  TOP opcode = TOP_UNDEFINED;
  TN  *tmp;

  /*
   * Make sure that result and src0 have integer MTYPEs.
   */
  FmtAssert(MTYPE_is_class_integer(rmtype) &&
	    MTYPE_is_class_integer(s0mtype),
            ("Expand_Madd: mtypes messed %s += %s + a*b",
                         MTYPE_name(rmtype), MTYPE_name(s0mtype)));

  switch (rmtype) {

  case MTYPE_I4:

    FmtAssert(MTYPE_size_min(s0mtype) == 32,
	      ("Expand_Madd: I4 = %s + a*b", MTYPE_name(s0mtype)));

    if (s1mtype == MTYPE_I2 && s2mtype == MTYPE_I2)
      opcode = TOP_UNDEFINED;
      //      opcode = TOP_IFR_MASSHW_GT_DR_DR_DR_DR;
    else if (s1mtype == MTYPE_I2 && s2mtype == MTYPE_U2)
      opcode = TOP_UNDEFINED;
      //      opcode = TOP_IFR_MASUHW_GT_DR_DR_DR_DR;
    else if (s1mtype == MTYPE_U2 && s2mtype == MTYPE_I2)
      opcode = TOP_UNDEFINED;
      //      opcode = TOP_IFR_MAUSHW_GT_DR_DR_DR_DR;
    else if (s1mtype == MTYPE_U2 && s2mtype == MTYPE_U2)
      //      opcode = TOP_IFR_MAUUHW_GT_DR_DR_DR_DR;
      opcode = TOP_UNDEFINED;
    else
      FmtAssert(FALSE, ("Expand_Madd: I4 = %s + %s * %s",
	MTYPE_name(s0mtype), MTYPE_name(s1mtype), MTYPE_name(s2mtype)));
    break;

  case MTYPE_U4:

    FmtAssert(s0mtype == MTYPE_U4,
	      ("Expand_Madd: U4 = %s + a*b", MTYPE_name(s0mtype)));

    if (s1mtype == MTYPE_U2 && s2mtype == MTYPE_U2)
      //      opcode = TOP_IFR_MAUHW_GT_DR_DR_DR_DR;
      opcode = TOP_UNDEFINED;
    else
      FmtAssert(FALSE, ("Expand_Madd: MTYPE_U4 -- operands messed"));
    break;

  default:
    FmtAssert(0, ("Expand_Madd: unknown format MTYPE %s",
		                                MTYPE_name(rmtype)));

  }

  if (opcode == TOP_UNDEFINED)
    FmtAssert(FALSE, ("Expand_Madd: failed"));

  Build_OP (opcode, result, True_TN, src0, src1, src2, ops);

  /* make a MPY and an ADD */
  /*
  Expand_Multiply (tmp, MTYPE_I4, src1, s1mtype, src2, s2mtype, ops);
  Expand_Add (result, src0, tmp, rmtype, ops);
  */

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
  TOP new_opcode;

  FmtAssert(FALSE,("Not Implemented"));

  if (TN_has_value(src2)) {
    src2 = Expand_Immediate_Into_Register (MTYPE_I4, src2, ops);
  }

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
  //
  // If src is not 0, make it 1
  //
  if (TN_is_constant(src)) {
    //
    // can only be immediate
    //
    FmtAssert(TN_has_value(src),("symbol/label in logical expression"));
    if (TN_value(src) != 0) {
      Build_OP(TOP_mov_i, src, Gen_Literal_TN(1, 4), ops);
    }
  }
  else {
    //
    // must be a register
    //
    TN *cond = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch);
    Build_OP (TOP_cmpeq_r_b, cond, src, Zero_TN, ops);
    Build_OP (TOP_slct_r, src, cond, Zero_TN, Gen_Literal_TN(1, 4), ops);
  }

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

  TOP cmpeq = TOP_UNDEFINED;
  int is_constant = TN_is_constant(src);

  if (TN_register_class(dest) == ISA_REGISTER_CLASS_branch) {
    cmpeq = is_constant ? TOP_cmpeq_i_b : TOP_cmpeq_r_b;
  }
  else if (TN_register_class(dest) == ISA_REGISTER_CLASS_integer) {
    cmpeq = is_constant ? TOP_cmpeq_i_r : TOP_cmpeq_r_r;
  }

  FmtAssert(cmpeq != TOP_UNDEFINED, ("Expand_Logical_Not"));

  Build_OP (cmpeq, dest, Zero_TN, src, ops);

  return;
}

/* ====================================================================
 *   Expand_Logical_Not
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
  if (!V_normalized_op1(variant) && EXP_normalize_logical)
  {
    Expand_Normalize_Logical (src1, ops);
  }
  if (!V_normalized_op2(variant) && EXP_normalize_logical)
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
  if (TN_is_constant(src1)) {
    // switch order of src so immediate is second
    Expand_Logical_And (dest, src2, src1, variant, ops);
    return;
  }

  if (TN_is_constant(src2)) {
    Expand_Logical_And_Or (TOP_and_i, dest, src1, src2, variant, ops);
  }
  else {
    Expand_Logical_And_Or (TOP_and_r, dest, src1, src2, variant, ops);
  }

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
  if (TN_is_constant(src1)) {
    // switch order of src so immediate is second
    Expand_Logical_Or (dest, src2, src1, variant, ops);
    return;
  }

  if (TN_is_constant(src2)) {
    Expand_Logical_And_Or (TOP_or_i, dest, src1, src2, variant, ops);
  }
  else {
    Expand_Logical_And_Or (TOP_or_r, dest, src1, src2, variant, ops);
  }

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

  FmtAssert(FALSE,("Not Implemented"));

  /* We shouldn't get here with these, but if we do they cause the routine
   * to fail, so guard against it.
   */
  if (imm == 0 || imm == -1) {
    Is_True(FALSE, ("Expand_Special_And_Immed called with imm == %d", imm));
    return FALSE;
  }

  /* Find the bit range that is being zeroed out by the AND.
   * If there is more than one contiguous range of zeros then
   * give up since we can't handle it.
   */
  start = 0;
  for (v = ~imm; (v & 1) == 0; v >>= 1) ++start;

  len = 0;
  for (; (v & 1) != 0; v >>= 1) ++len;

  if (v != 0) return FALSE;

  /* If the zeroed bits include bit-63, then we are performing an
   * extract, so use either extr.u or zxt*. Otherwise use dep.i.
   * Note that we could always use dep.i, but the extract is more
   * readable, and zxt* might be faster in some cases.
   */
  if (start > 0 && start + len == 64) {
    const UINT extr_len = start;
    if (extr_len == 8 || extr_len == 16 || extr_len == 32) {
      TOP zxt_opc;
      switch (extr_len / 8) {
      case 1: zxt_opc = TOP_noop; break;
      case 2: zxt_opc = TOP_noop; break;
      case 4: zxt_opc = TOP_noop; break;
      }
      Build_OP(zxt_opc, dest, True_TN, src1, ops);
    } else {
      Build_OP(TOP_noop, dest, True_TN, src1,
	       Gen_Literal_TN(0, 4), Gen_Literal_TN(extr_len, 4), ops);
    }
  } else {
    Build_OP(TOP_noop, dest, True_TN, Gen_Literal_TN(0, 4), src1,
	     Gen_Literal_TN(start, 4), Gen_Literal_TN(len, 4), ops);
  }

  return TRUE;
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

#if 0
  // mtype indicates what type of intermediate result is used:
  if (Register_Class_For_Mtype(mtype) != TN_register_class(dest)) {
    TN *tmp = Build_RCLASS_TN (Register_Class_For_Mtype(mtype));
    Build_OP (action, tmp, src1, src2, ops);
    Exp_COPY (dest, tmp, ops);
  }
  else {
    Build_OP (action, dest, src1, src2, ops);
  }
#endif

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
  //
  // If src1 is constant, reverse the operands
  //
  if (TN_is_constant(src1)) {
    FmtAssert(TN_is_register(src2),("wrong operands"));
    Expand_Binary_And (dest, src2, src1, mtype, ops);
    return;
  }

  if (TN_is_constant(src2)) {
    Expand_Binary_And_Or (TOP_and_i, dest, src1, src2, mtype, ops);
  }
  else {
    Expand_Binary_And_Or (TOP_and_r, dest, src1, src2, mtype, ops);
  }

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
  //
  // If src1 is constant, reverse the operands
  //
  if (TN_is_constant(src1)) {
    FmtAssert(TN_is_register(src2),("wrong operands"));
    Expand_Binary_Or (dest, src2, src1, mtype, ops);
    return;
  }

  switch (mtype) {

  case MTYPE_I4:
  case MTYPE_U4:

    if (TN_is_constant(src2)) {
      Build_OP (TOP_or_i, dest, src1, src2, ops);
    }
    else {
      Build_OP (TOP_or_r, dest, src1, src2, ops);
    }
    break;

  default:
    FmtAssert(FALSE, ("Expand_Binary_Xor: mtype not handled"));
  }

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
  if (TN_is_constant(src1)) {
    Is_True(TN_is_register(src2),("wrong operands"));
    Expand_Binary_Xor (dest, src2, src1, mtype, ops);
    return;
  }

  switch (mtype) {

  case MTYPE_I4:
  case MTYPE_U4:

    if (TN_is_constant(src2)) {
      Build_OP (TOP_xor_i, dest, src1, src2, ops);
    }
    else {
      Build_OP (TOP_xor_r, dest, src1, src2, ops);
    }
    break;

  default:
    FmtAssert(FALSE, ("Expand_Binary_Xor: mtype not handled"));
  }

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

#if 0
  // nor is or s1,s2; xor -1
  //
  // NOTE: if one of the operands is constant, the expansion
  // could be 'andcm ~imm,s2' which is one inst if the complemented
  // constant fits in the immed. But testing has not found a
  // case where this would occur, so leave it out. -- Ken
  TN *tmp = Build_TN_Like(dest);
  Expand_Binary_And_Or (TOP_GP32_OR_GT_DR_DR_DR, tmp,
                                          src1, src2, mtype, ops);
  Expand_Binary_And_Or (TOP_GP32_XOR_GT_DR_DR_DR, dest,
			  tmp, Gen_Literal_TN(-1, 4), mtype, ops);
#endif
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
  case MTYPE_I8: variant = V_BR_I8LT; break;
  case MTYPE_I4: variant = V_BR_I4LT; break;
  case MTYPE_U8: variant = V_BR_U8LT; break;
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
  case MTYPE_I8: variant = V_BR_I8LE; break;
  case MTYPE_I4: variant = V_BR_I4LE; break;
  case MTYPE_U8: variant = V_BR_U8LE; break;
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
  case MTYPE_I8: variant = V_BR_I8EQ; break;
  case MTYPE_I4: variant = V_BR_I4EQ; break;
  case MTYPE_U8: variant = V_BR_U8EQ; break;
  case MTYPE_U4: variant = V_BR_U4EQ; break;
  case MTYPE_A8: variant = V_BR_A8EQ; break;
  case MTYPE_A4: variant = V_BR_A4EQ; break;

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
  case MTYPE_I8: variant = V_BR_I8NE; break;
  case MTYPE_I4: variant = V_BR_I4NE; break;
  case MTYPE_U8: variant = V_BR_U8NE; break;
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
  case MTYPE_I8: variant = V_BR_I8GE; break;
  case MTYPE_I4: variant = V_BR_I4GE; break;
  case MTYPE_U8: variant = V_BR_U8GE; break;
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
    case MTYPE_I8: variant = V_BR_I8GT; break;
    case MTYPE_I4: variant = V_BR_I4GT; break;
    case MTYPE_U8: variant = V_BR_U8GT; break;
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
  VARIANT variant;
  TOP   action;
  TN   *tmp1, *tmp2;

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

  Expand_Bool_Comparison (TRUE, dest, src1, src2, ops);
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

  Expand_Bool_Comparison (FALSE, dest, src1, src2, ops);
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
  Build_OP (TOP_mtb, dest, src, ops);
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

  // IFCVT -> fsetc ; fcvt.fx ; getf
  TOP fcvt;
  TN *sf;
  const BOOL is_signed = MTYPE_is_signed(imtype);
  TN *tmp;

  switch (rm) {
  case ROUND_USER:

    break;
  case ROUND_NEAREST:

    break;
  case ROUND_CHOP:

    break;
  case ROUND_NEG_INF:

    break;
  case ROUND_PLUS_INF:

    break;
  }
  Build_OP(fcvt, tmp, True_TN, sf, src, ops);

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

  Expand_Float_To_Int (ROUND_USER, dest, src, imtype, fmtype, ops);
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

  Expand_Float_To_Int (ROUND_NEAREST, dest, src, imtype, fmtype, ops);
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
  Is_True(FALSE,("Should have generated a run-time for OPR_TRUNC"));
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

  Expand_Float_To_Int (ROUND_NEG_INF, dest, src, imtype, fmtype, ops);
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

  Expand_Float_To_Int (ROUND_PLUS_INF, dest, src, imtype, fmtype, ops);
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
  TOP  cmp;
  BOOL reversed;

#if 0
  // The comparison opnds must be the same as the src opnds.
  if (cond1 == src1 && cond2 == src2) {
    reversed = FALSE;
  } else if (cond1 == src2 && cond2 == src1) {
    reversed = TRUE;
  } else {
    return FALSE;
  }

  // Now optimize according to the type of comparison.
  switch (cmp) {
    case TOP_GP32_EQW_GT_BR_DR_DR:
    case TOP_GP32_EQW_GT_BR_DR_U8:

    case TOP_GP32_NEW_GT_BR_DR_DR:
    case TOP_GP32_NEW_GT_BR_DR_U8:

    case TOP_GP32_GTW_GT_BR_DR_DR:
    case TOP_GP32_GTW_GT_BR_DR_U8:

    case TOP_GP32_LTW_GT_BR_DR_DR:
    case TOP_GP32_LTW_GT_BR_DR_U8:
    default:
      return FALSE;
  }

  FmtAssert(FALSE,("Optimize_Select: Unhandled compare %s",
		                                TOP_Name(cmp)));

#endif
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
  // into:
  //
  //      p1 = (src1 cmp src2)
  // p1?  dst = p1? true_tn : false_tn
  // p1?  opposite_dst = true_tn
  // !p1?  opposite_dst = false_tn
  //

  TOP cmp1 = Pick_Compare_TOP (&variant, &cond1, &cond2, FALSE, ops);

  switch (variant) {
    case V_BR_I4GE:
    case V_BR_I4GT:
    case V_BR_I4LE:
    case V_BR_I4LT:
    case V_BR_I4EQ:
    case V_BR_I4NE:
      mtype = MTYPE_I4;
      break;
  default:
    FmtAssert(FALSE,("Not Implemented"));
  }

  TN *p1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch);

  Build_OP (cmp1, p1, cond1, cond2, ops);

  Expand_Select (dest, p1, true_tn, false_tn, mtype, is_float, ops);

  if (opposite_dest) {
    FmtAssert(FALSE,("Not Implemented"));
  }

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
  TOP select;
  const BOOL is_float = MTYPE_is_float(mtype);

  if (TN_register_class(cond_tn) != ISA_REGISTER_CLASS_branch) {
    FmtAssert(FALSE,("Not Implemented"));
  }

  if (TN_has_value(true_tn)) {
    TN *tmp = true_tn;
    true_tn = false_tn;
    false_tn = tmp;
    select = TOP_slctf_i;
  }
  else if (TN_has_value(false_tn)) {  
    select = TOP_slct_i;
  }
  else {
    select = TOP_slct_r;
  }

  Build_OP (select, dest_tn, cond_tn, true_tn, false_tn, ops);
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

#if 0
      // tmp = (cmp_kid1 == cmp_kid2)
      TN *tmp = Build_TN_Of_Mtype (MTYPE_I8);
      Build_OP (TOP_GP32_MAKE_GT_DR_S16, tmp, True_TN,
                                        Gen_Literal_TN(1, 8), &newops);
      Build_OP (TOP_GP32_XOR_GT_DR_DR_U8, tmp, cmp_kid1,
                                   Gen_Literal_TN(1, 8), tmp, &newops);
      Build_OP (TOP_GP32_XOR_GT_DR_DR_U8, tmp, cmp_kid2,
                                   Gen_Literal_TN(1, 8), tmp, &newops);

      //      cmp = (variant == V_BR_PEQ) ? TOP_GP32_NEW_GT_BR_DR_DR :
                                              TOP_GP32_EQW_GT_BR_DR_DR;
      cmp_kid1 = tmp;
      cmp_kid2 = Zero_TN;
#endif
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

  // if src1 is an immediate, swap
  if (TN_has_value(src1)) {
    TN *tmp = src2;
    src2 = src1;
    src1 = tmp;
  }

  switch (mtype) {
      case MTYPE_I8:
      case MTYPE_I4:
	top = TN_has_value(src2) ? TOP_min_i : TOP_min_r;
	break;

      case MTYPE_U8:
      case MTYPE_U4:
	top = TN_has_value(src2) ? TOP_minu_i : TOP_minu_r;
	break;

      case MTYPE_F4:
      case MTYPE_F8:
      case MTYPE_F10:
      default:
	FmtAssert(FALSE, ("Expand_Min: unexpected mtype"));
  }

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

  // if src1 is an immediate, swap
  if (TN_has_value(src1)) {
    TN *tmp = src2;
    src2 = src1;
    src1 = tmp;
  }

  switch (mtype) {
      case MTYPE_I8:
      case MTYPE_I4:
	top = TN_has_value(src2) ? TOP_max_i : TOP_max_r;
	break;

      case MTYPE_U8:
      case MTYPE_U4:
	top = TN_has_value(src2) ? TOP_maxu_i : TOP_maxu_r;
	break;

      case MTYPE_F4:
      case MTYPE_F8:
      case MTYPE_F10:
      default:
	FmtAssert(FALSE, ("Expand_Max: unexpected mtype"));
  }

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
  TOP cmp;
  VARIANT variant;

  FmtAssert(FALSE,("MINMAX shouldn't have been reached"));

  switch (mtype) {
  case MTYPE_I8: variant = V_BR_I8LT; break;
  case MTYPE_I4: variant = V_BR_I4LT; break;
  case MTYPE_U8: variant = V_BR_U8LT; break;
  case MTYPE_U4: variant = V_BR_U4LT; break;
  case MTYPE_F4:
  case MTYPE_F8:
  case MTYPE_F10:

  default:
    #pragma mips_frequency_hint NEVER
    Is_True(FALSE, ("Expand_MinMax: unexpected mtype"));
  }

  //  cmp = Pick_Compare_TOP (&variant, &src1, &src2, ops);

  Expand_Compare_And_Select (variant, src1, src2, dest, dest2, src1, src2, FALSE, ops);
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
#if 0
  if (TN_register_class(dest) == ISA_REGISTER_CLASS_guard) {
    // return result of comparison in a predicate register
    TN *p1 = dest;
    TN *p2 = Get_Complement_TN(dest);
    Build_OP (cmp_opcode, p1, p2, True_TN, src1, src2, ops);
  } else {
    TN *p1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_guard);
    TN *p2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_guard);
    Build_OP (cmp_opcode, p1, p2, True_TN, src1, src2, ops);
    // can either do unconditional copy of 0,
    // or predicated copy of 0 followed by predicated copy of 1.
    // Expand_Copy (dest, Zero_TN, MTYPE_I8, ops);
    Build_OP (TOP_GP32_MOVE_GT_DR_DR, dest, p2, Zero_TN, ops);
    Build_OP (TOP_GP32_MAKE_GT_DR_S16, dest, p1, Gen_Literal_TN(1, 4), ops);
  }
#endif
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
    const char * const alg = EXP_sqrt_algorithm;
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

/* ====================================================================
 *   Exp_Intrinsic_Op
 * ====================================================================
 */
void
Exp_Intrinsic_Op (
  INTRINSIC id,
  INT num_results,
  INT num_opnds,
  TN *result[],
  TN *opnd[],
  OPS *ops
)
{
  switch (id) {
    default:
      FmtAssert (FALSE, ("Exp_Intrinsic_Op: unknown intrinsic op"));
  }

  return;
}

/* ======================================================================
 *   Get_Intrinsic_Size_Mtype
 * ======================================================================
 */
static TYPE_ID
Get_Intrinsic_Size_Mtype (
  INTRINSIC id
)
{
  FmtAssert(FALSE,("Not Implemented"));

  switch (id) {
  default:
    FmtAssert(FALSE, ("Unexpected intrinsic %d", id));
    /*NOTREACHED*/
  }
}

/* ======================================================================
 *   Intrinsic_Returns_New_Value
 * ======================================================================
 */
static BOOL
Intrinsic_Returns_New_Value (
  INTRINSIC id
)
{
  FmtAssert(FALSE,("Not Implemented"));

  switch (id) {
  default:
    return FALSE;
  }
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
  FmtAssert(FALSE,("Not Implemented"));

  return NULL;
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
  INT pc_value
)
{
  OP *newop;

  DevWarn("Exp_Simulated_Op should not be reached");

  switch (OP_code(op)) {

  case TOP_intrncall:
    Expand_TOP_intrncall(op, ops, FALSE, pc_value);
    break;

  case TOP_spadjust:
    // spadjust should only show up for alloca/dealloca
    if (OP_variant(op) == V_SPADJUST_PLUS) {
      // dealloca does copy of kid to $sp (op1 is old sp value)
      Expand_Add (OP_result(op,0), OP_opnd(op,1),
		                    OP_opnd(op,2), Pointer_Mtype, ops);
    }
    else {
      Expand_Sub (OP_result(op,0), OP_opnd(op,1),
                                    OP_opnd(op,2), Pointer_Mtype, ops);
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
#if 0
  union {
    INT32 i;
    float f;
  } val;
#endif
  //
  // This is called normally for floating-point and company
  // Since we keep them in integer registers, just make a mov
  //

  switch (TCON_ty(tc)) {

  case MTYPE_F4:
    //
    // For now I do the bit-pattern directly, eventually we should
    // try to make a mov from a symbol TN into an integer TN and
    // handle it in emit const in assm phase.
    //
    Build_OP(TOP_mov_i, dest, src, ops);
#if 0
    val.f = tc.vals.fval;
    Build_OP(TOP_mov_i, dest, Gen_Literal_TN(val.i, 4), ops);

    fprintf(TFile, "generated fp const !!!\n");
#endif
    break;

  default:
    FmtAssert(FALSE,("unsupported type %s", MTYPE_name(TCON_ty(tc))));
  }

#if 0
  FmtAssert(FALSE,("Not Implemented"));


  if (Targ_Is_Zero(tc)) {
    // copy 0
    Build_OP (TOP_noop, dest, True_TN, FZero_TN, ops);
    return;
  }

  // load from memory
  Exp_Load (mtype, mtype, dest, TN_var(src), 0, ops, V_NONE);
#endif

  return;
}

/* ====================================================================
 *   Target_Has_Immediate_Operand (parent, expr)
 * ====================================================================
 */
BOOL
Target_Has_Immediate_Operand (
  WN *parent,
  WN *expr
)
{
  if (WN_operator(parent) == OPR_INTRINSIC_CALL
	&& (((INTRINSIC) WN_intrinsic (parent) == INTRN_FETCH_AND_ADD_I4)
	 || ((INTRINSIC) WN_intrinsic (parent) == INTRN_FETCH_AND_ADD_I8))) {
    // can optimize for some constants
    return TRUE;
  }

  // adds and subs can handle immediates as second operand:
  if (WN_operator(parent) == OPR_ADD) {
    return TRUE;
  }

  if (WN_operator(parent) == OPR_SUB) {
    if (WN_kid1(parent) == expr)
      return TRUE;
  }

  // default to false, which really means "don't know"
  return FALSE;
}

