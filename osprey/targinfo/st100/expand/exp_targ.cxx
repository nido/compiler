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
#include "wutil.h"      /* for WN_intrinsic_return_ty */
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

#if 0
/* ====================================================================
 *   WN_intrinsic_return_ty
 *
 *   Similar to the one in be/whirl2c/wn_attr.cxx
 *   TODO: move to the intrn_info.cxx of something.
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
#endif

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
    case TOP_GP32_XOR_GT_DR_DR_DR: return TOP_GP32_XOR_GT_DR_DR_U8;
    case TOP_GP32_AND_GT_DR_DR_DR: return TOP_GP32_AND_GT_DR_DR_U8;

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

    case ISA_REGISTER_CLASS_du:

      if (tgt_rc == ISA_REGISTER_CLASS_du) {
	Build_OP(TOP_GP32_MOVE_GT_DR_DR, tgt_tn, guard, src_tn, ops);
	Set_OP_copy (OPS_last(ops));
      }
      else if (tgt_rc == ISA_REGISTER_CLASS_au) {
	Build_OP(TOP_GP32_COPYA_GT_AR_DR, tgt_tn, guard, src_tn, ops);
      }
      else {
	FmtAssert(FALSE,("Expand_Copy: not supported"));
      }
      break;

    case ISA_REGISTER_CLASS_au:

      if (tgt_rc == ISA_REGISTER_CLASS_du) {
	Build_OP(TOP_GP32_COPYD_GT_DR_AR, tgt_tn, guard, src_tn, ops);
      }
      else if (tgt_rc == ISA_REGISTER_CLASS_au) {
	Build_OP(TOP_GP32_MOVEA_GT_AR_AR, tgt_tn, guard, src_tn, ops);
	Set_OP_copy (OPS_last(ops));
      }
      else {
	FmtAssert(FALSE,("Expand_Copy: not supported"));
      }
      break;

    case ISA_REGISTER_CLASS_guard:

      if (tgt_rc == ISA_REGISTER_CLASS_guard) {
	Build_OP(TOP_GP32_MOVEG_GT_BR_BR, tgt_tn, guard, src_tn, ops);
	Set_OP_copy (OPS_last(ops));
      }
      else if (tgt_rc == ISA_REGISTER_CLASS_du) {
	Build_OP(TOP_GP32_BOOL_GT_DR_BR, tgt_tn, guard, src_tn, ops);
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

  if (old_length <= 40) {
    // fits into the ST100 registers
    if (new_length == 8) 
      opc = (signed_extension ? 
                     TOP_GP32_EXTB_GT_DR_DR : TOP_GP32_EXTUB_GT_DR_DR);
    else if (new_length == 16) 
      opc = (signed_extension ? 
	             TOP_GP32_EXTH_GT_DR_DR : TOP_GP32_EXTUH_GT_DR_DR);
    else if (new_length == 32) 
      opc = (signed_extension ? 
                     TOP_GP32_EXTW_GT_DR_DR : TOP_GP32_EXTUW_GT_DR_DR);
  }

  if (opc == TOP_UNDEFINED) {
    FmtAssert(FALSE, ("Expand_Convert_Length: unknown extension"));
  }

  Build_OP ( opc, dest, True_TN, src, ops);
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
  FmtAssert (rtype == MTYPE_A4,
    ("Convert_Int_To_Adr: unsupported result type %s", MTYPE_name(rtype)));
  FmtAssert ((desc == MTYPE_I4 || desc == MTYPE_U4),
    ("Convert_Int_To_Adr: unsupported src type %s", MTYPE_name(rtype)));

  /* make a copya */
  Build_OP(TOP_GP32_COPYA_GT_AR_DR, result, True_TN, src, ops);

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
  if (rtype == MTYPE_I4 || rtype == MTYPE_U4) {
    /* make a copyd */
    Build_OP(TOP_GP32_COPYD_GT_DR_AR, result, True_TN, src, ops);
  }
  else {
    FmtAssert (FALSE,
       ("Convert_Ptr_To_Int: unsupported result type %s", MTYPE_name(rtype)));
  }

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
    else if (MTYPE_is_class_pointer(desc)) {
      Expand_Ptr_To_Int (result, rtype, op1, desc, ops);
      return;
    }
  }

  // conversions to pointer:
  if (MTYPE_is_class_pointer(rtype)) {
    if (MTYPE_is_class_integer(desc)) {
      Expand_Int_To_Ptr (result, rtype, op1, desc, ops);
      return;
    }
    else if (MTYPE_is_class_pointer(desc)) {
      // both are adr
      // else sign-extend.
      Is_True(MTYPE_bit_size(desc) < MTYPE_bit_size(rtype), 
	                   ("Expand_Convert: desc > rtype ??"));
      // must be A4 -> A8
      FmtAssert(FALSE, ("Expand_Convert: Not implemented"));
    }
  }

  if (opc != TOP_UNDEFINED) {
      Build_OP (opc, result, True_TN, op1, ops);
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
    Expand_Copy(result, True_TN, src, ops);
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
  else if (TN_is_symbol(src)) {

    FmtAssert(FALSE,("Exp_Immediate: symbol TN"));

    ST *base;
    Base_Symbol_And_Offset_For_Addressing (TN_var(src), 
                                      TN_offset(src), &base, &val);
    // now val is the "real" offset
  }

  switch (TN_register_class(dest)) {

    case ISA_REGISTER_CLASS_guard:
      if (val == 0) {
	Build_OP (TOP_GP32_CLRG_GT_BR, dest, True_TN, src, ops);
      }
      else if (val == 1) {
	Build_OP (TOP_GP32_SETG_GT_BR, dest, True_TN, src, ops);
      }
      else {
        FmtAssert(0, ("Exp_Immediate: immediate to guard != {0,1}"));
      }
      break;

    case ISA_REGISTER_CLASS_du:

      if (ISA_LC_Value_In_Class (val, LC_s16)) {
	Build_OP (TOP_GP32_MAKE_GT_DR_S16, dest, True_TN, src, ops);
      }
      else if (ISA_LC_Value_In_Class (val, LC_s32)) {
	/*
	 * This is all good but it's too early to expand:
	 */
	/*
	tmp = Gen_Literal_TN(val >> 16, TN_size(dest));
	Build_OP (TOP_GP32_MAKE_GT_DR_S16, dest, True_TN, tmp, ops);
	tmp = Gen_Literal_TN((val & 0x00000000000fffff), TN_size(dest));
	Build_OP (TOP_GP32_MORE_GT_DR_U16, dest, True_TN, dest, tmp, ops);
	*/
	Build_OP (TOP_GP32_MAKEB_GT_DR_S32, dest, True_TN, src, ops);
      }
      else if (ISA_LC_Value_In_Class (val, LC_s40)) {
	// must fit into 40 bit:
	/*
	 * This is all good but it's too early to expand:
	 */
	/*
	tmp = Gen_Literal_TN(val >> 32, MTYPE_byte_size(mtype));
	Build_OP (TOP_GP32_MAKE_GT_DR_S16, dest, True_TN, tmp, ops);
	tmp = Gen_Literal_TN(((val >> 16) & 0x00000000000fffff), 
                                                           TN_size(dest));
	Build_OP (TOP_GP32_MORE_GT_DR_U16, dest, True_TN, dest, tmp, ops);
	tmp = Gen_Literal_TN((val & 0x00000000000fffff), TN_size(dest));
	Build_OP (TOP_GP32_MORE_GT_DR_U16, dest, True_TN, dest, tmp, ops);
	*/
	Build_OP (TOP_GP32_MAKEK_GT_DR_S40, dest, True_TN, src, ops);
      }
      else {
	FmtAssert(0, ("Exp_Immediate: unexpected immediate value"));
      }
      break;

    case ISA_REGISTER_CLASS_au:

      if (ISA_LC_Value_In_Class (val, LC_s16)) {
	Build_OP (TOP_GP32_MAKEA_GT_AR_S16, dest, True_TN, src, ops);
      }
#if 0
      // MAKEBA is not U32
      else if (ISA_LC_Value_In_Class (val, LC_u32)) {
	/*
	 * This is all good but it's too early to expand:
	 */
	/*
	tmp = Gen_Literal_TN(val >> 16, TN_size(dest));
	Build_OP (TOP_GP32_MAKEA_GT_AR_S16, dest, True_TN, tmp, ops);
	tmp = Gen_Literal_TN((val & 0x000000000000ffff), TN_size(dest));
	Build_OP (TOP_GP32_MOREA_GT_AR_U16, dest, True_TN, dest, tmp, ops);
	*/
	Build_OP (TOP_GP32_MAKEBA_GT_AR_U32, dest, True_TN, src, ops);
      }
#endif
      else {
	FmtAssert(0, ("Exp_Immediate: immediate > 32"));
      }
      break;

    default:
      FmtAssert(0, ("Exp_Immediate: unknown register class"));
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
  return tmp;
}

/* ====================================================================
 *   Pick_Constant_Add
 * ====================================================================
 */
static TOP
Pick_Constant_Add (
  INT64 val,
  TYPE_ID mtype,
  TN **src2,
  OPS *ops
)
{
  TOP new_opcode;

  switch (mtype) {
    case MTYPE_I4:
    case MTYPE_U4:
      if (ISA_LC_Value_In_Class(val, LC_u8)) {
	new_opcode = TOP_GP32_ADD_GT_DR_DR_U8;
      }
      else {
	new_opcode = TOP_GP32_ADD_GT_DR_DR_DR;
	*src2 = Expand_Immediate_Into_Register(mtype, *src2, ops);
      }
      break;

    case MTYPE_A4:

      if (ISA_LC_Value_In_Class(val, LC_u9)) {
	new_opcode = TOP_GP32_ADDBA_GT_AR_AR_U9;
      }
      else if (val % 2 == 0 && ISA_LC_Value_In_Class((val >> 1), LC_u9)) {
	new_opcode = TOP_GP32_ADDHA_GT_AR_AR_U9;
	//	*src2 = Gen_Literal_TN(val>>1, Pointer_Size);
      }
      else if (val % 4 == 0 && ISA_LC_Value_In_Class((val >> 2), LC_u9)) {
	new_opcode = TOP_GP32_ADDWA_GT_AR_AR_U9;
	//	*src2 = Gen_Literal_TN(val>>2, Pointer_Size);
      }
      else {
	new_opcode = TOP_GP32_ADDBA_GT_AR_AR_AR;
	*src2 = Expand_Immediate_Into_Register(mtype, *src2, ops);
      }

      break;

    default:
      FmtAssert(0,("unexpected mtype in Pick_Constant_Add"));
  }

  return new_opcode;
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

  /* 
   * Make sure that the ADD operands are of correct type:
   *   because WHIRL will not generate explicit conversions for
   *   some types of operands, eg. A4 + I4. Oh, yes, it should !
   */
  if (TN_is_constant(src1)) {
    // switch order of src so immediate is second
    Expand_Add (result, src2, src1, mtype, ops);
    return;
  }
#if 0
  else if (TN_is_register(src1) && 
      (Register_Class_For_Mtype(mtype) != TN_register_class(src1))) {
    tmp = Build_TN_Of_Mtype (mtype);
    Exp_COPY (tmp, src1, ops);
    src1 = tmp;
  }
#endif

  if (TN_is_constant(src2)) {

    if (TN_has_value(src2)) {
      INT64 val = TN_value(src2);
      if (val == 0) {
	Expand_Copy (result, True_TN, src1, ops);
	return;
      }
      if (val < 0) {
	// rather do sub than make -const, add:
	src2 = Gen_Literal_TN (-val, MTYPE_byte_size(mtype));
	Expand_Sub (result, src1, src2, mtype, ops);
	return;
      }
      new_opcode = Pick_Constant_Add (val, mtype, &src2, ops);
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
      // address production ... shouldn't use MAKEA
      if (TN_is_gp_reg(src1) && ofst != 0) {
	// Should be ADDBA because src2 must have a reloc ...
	Build_OP(TOP_GP32_ADDBA_GT_AR_AR_AR, tmp, True_TN, src1, src2, ops);
	src1 = tmp;
	src2 = Gen_Literal_TN(ofst, Pointer_Size);
      }
      else {
        // just makea and hope that 16bit offset suffices:
        Build_OP (TOP_GP32_MAKEA_GT_AR_S16, tmp, True_TN, src2, ops);
        src2 = tmp;
      }

      new_opcode = TOP_GP32_ADDBA_GT_AR_AR_AR;
    }
    else {
      FmtAssert(FALSE,("unexpected constant in Expand_Add"));
    }
  }
  else {
    // both registers:
    FmtAssert (TN_is_register(src2),
	              ("Expand_Add: unknown 2nd operand type"));
    if (Register_Class_For_Mtype(mtype) != TN_register_class(src2)) {
      tmp = Build_TN_Of_Mtype (mtype);
      Exp_COPY (tmp, src2, ops);
      src2 = tmp;
    }

    switch (mtype) {
      case MTYPE_I4:
      case MTYPE_I5:
	new_opcode = TOP_GP32_ADD_GT_DR_DR_DR;
	break;
      case MTYPE_U4:
      case MTYPE_U5:
	new_opcode = TOP_GP32_ADDU_GT_DR_DR_DR;
	break;
      case MTYPE_A4:
	new_opcode = TOP_GP32_ADDBA_GT_AR_AR_AR;
	break;
      default:
	FmtAssert(0,("unexpected mtype in Expand_Add"));
    }
  }

  Build_OP (new_opcode, result, True_TN, src1, src2, ops);
  return;
}

/* ====================================================================
 *   Pick_Constant_Sub
 * ====================================================================
 */
static TOP
Pick_Constant_Sub (
  INT64 val,
  TYPE_ID mtype,
  TN **src2,
  OPS *ops
)
{
  TOP new_opcode;

  switch (mtype) {
    case MTYPE_I4:
    case MTYPE_U4:
      if (ISA_LC_Value_In_Class(val, LC_u8)) {
	new_opcode = TOP_GP32_SUB_GT_DR_DR_U8;
      }
      else {
	new_opcode = TOP_GP32_SUB_GT_DR_DR_DR;
	*src2 = Expand_Immediate_Into_Register(mtype, *src2, ops);
      }
      break;

    case MTYPE_A4:

      if (ISA_LC_Value_In_Class(val, LC_u9)) {
	new_opcode = TOP_GP32_SUBBA_GT_AR_AR_U9;
      }
      else if (val % 2 == 0 && ISA_LC_Value_In_Class((val >> 1), LC_u9)) {
	new_opcode = TOP_GP32_SUBHA_GT_AR_AR_U9;
	//	*src2 = Gen_Literal_TN(val>>1, Pointer_Size);
      }
      else if (val % 4 == 0 && ISA_LC_Value_In_Class((val >> 2), LC_u9)) {
	new_opcode = TOP_GP32_SUBWA_GT_AR_AR_U9;
	//	*src2 = Gen_Literal_TN(val>>2, Pointer_Size);
      }
      else {
	new_opcode = TOP_GP32_SUBBA_GT_AR_AR_AR;
	*src2 = Expand_Immediate_Into_Register(mtype, *src2, ops);
      }
      break;

    default:
      FmtAssert(0,("unexpected mtype in Pick_Constant_Sub"));
  }

  return new_opcode;
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

  /* 
   * Make sure that the ADD operands are of correct type:
   * because WHIRL will not generate explicit conversions for
   * some types of operands, eg. A4 - I4
   */
  if (TN_is_constant(src1)) {
    src1 = Expand_Immediate_Into_Register (mtype, src1, ops);
  }
  else {
    FmtAssert(TN_is_register(src1), ("Expand_Sub: unknown 1st opnd"));
    if (Register_Class_For_Mtype(mtype) != TN_register_class(src1)) {
      tmp = Build_TN_Of_Mtype (mtype);
      Exp_COPY (tmp, src1, ops);
      src1 = tmp;
    }
  }

  if (TN_is_constant(src2)) {
    if (TN_has_value(src2)) {
      val = TN_value(src2);
      if (val == 0) {
	Expand_Copy (result, True_TN, src1, ops);
	return;
      }
      new_opcode = Pick_Constant_Sub (val, mtype, &src2, ops);
    }
    else if (TN_is_symbol(src2)) {
      /* symbolic constant, gp-relative or sp-relative */
      Base_Symbol_And_Offset_For_Addressing (TN_var(src2), 
                                       TN_offset(src2), &base, &ofst);
      new_opcode = Pick_Constant_Sub (ofst, mtype, &src2, ops);
    }
    else {
      FmtAssert(FALSE,("unexpected constant in Expand_Sub"));
    }
  }
  else {
    // both registers:
    FmtAssert (TN_is_register(src2),
	                    ("Expand_Sub: unknown 2nd operand type"));
    if (Register_Class_For_Mtype(mtype) != TN_register_class(src2)) {
      tmp = Build_TN_Of_Mtype (mtype);
      Exp_COPY (tmp, src2, ops);
      src2 = tmp;
    }

    switch (mtype) {
      case MTYPE_I4:
      case MTYPE_U4:
	new_opcode = TOP_GP32_SUB_GT_DR_DR_DR;
	break;
      case MTYPE_A4:
	new_opcode = TOP_GP32_SUBBA_GT_AR_AR_AR;
	break;
      default:
	FmtAssert(0,("unexpected mtype in Expand_Sub"));
    }
  }

  Build_OP (new_opcode, result, True_TN, src1, src2, ops);
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
  TN *result, 
  TN *src, 
  TYPE_ID mtype, 
  OPS *ops
)
{
  switch (mtype) {
  case MTYPE_I4:
  case MTYPE_U4:
    Build_OP (TOP_GP32_NEGU_GT_DR_DR, result, True_TN, src, ops);
    return;
  default:
    FmtAssert(FALSE, ("Expand_Neg: not handled mtype %s\n", 
                                             MTYPE_name(mtype)));
  }
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
  FmtAssert(FALSE,("Not Implemented"));

  TN *p1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_guard);
  TN *p2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_guard);

  Expand_Copy (dest, True_TN, src, ops);
  Build_OP (TOP_GP32_LTW_GT_BR_DR_DR, p1, p2, True_TN, src, Zero_TN, ops);
  Build_OP (TOP_GP32_SUB_GT_DR_DR_DR, dest, p1, Zero_TN, src, ops);
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
  if (TN_has_value(*src2)) {
    UINT64 val = TN_value(*src2);

    if (val == 32) {
      if (mtype == MTYPE_U4 ||
	  mtype == MTYPE_U5)
	return TOP_GP32_SHLU32_GT_DR_DR;
    }

    if (ISA_LC_Value_In_Class(val, LC_u5)) {
      switch (mtype) {
        case MTYPE_I4:
        case MTYPE_I5:
	  return TOP_GP32_SHL_GT_DR_DR_U5;
        case MTYPE_U4:
        case MTYPE_U5:
	  return TOP_GP32_SHLU_GT_DR_DR_U5;
        default:
	  *src2 = Expand_Immediate_Into_Register (mtype, *src2, ops);
      }
    }
  }

  switch (mtype) {
    case MTYPE_I4:
    case MTYPE_I5:
      return TOP_GP32_SHL_GT_DR_DR_DR;
    case MTYPE_U4:
    case MTYPE_U5:
      return TOP_GP32_SHLU_GT_DR_DR_DR;
    default:
      return TOP_UNDEFINED;
  }
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
  if (TN_has_value(*src2)) {
    UINT64 val = TN_value(*src2);

    if (val == 1 && mtype == MTYPE_A4)
      return TOP_GP32_SHRA1_GT_AR_AR;

    if (val == 2 && mtype == MTYPE_A4)
      return TOP_GP32_SHRA2_GT_AR_AR;

    if (val == 32) {
      switch (mtype) {
        case MTYPE_I4:
        case MTYPE_U4:
        case MTYPE_I5:
        case MTYPE_U5:
	  return TOP_GP32_SHR32_GT_DR_DR;
      }
    }

    if (ISA_LC_Value_In_Class(val, LC_u5)) {
      switch (mtype) {
        case MTYPE_I4:
        case MTYPE_U4:
	  return TOP_GP32_SHRW_GT_DR_DR_U5;
        case MTYPE_I5:
        case MTYPE_U5:
	  return TOP_GP32_SHR_GT_DR_DR_U5;
        default:
	  *src2 = Expand_Immediate_Into_Register (mtype, *src2, ops);
      }
    }
  }

  switch (mtype) {
    case MTYPE_I4:
      return TOP_GP32_SHRW_GT_DR_DR_DR;
    case MTYPE_U4:
      return TOP_GP32_SHRUW_GT_DR_DR_DR;
    case MTYPE_I5:
      return TOP_GP32_SHR_GT_DR_DR_DR;
    case MTYPE_U5:
      return TOP_GP32_SHRU_GT_DR_DR_DR;
    default:
      return TOP_UNDEFINED;
  }
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
  if (TN_has_value(*src2)) {
    UINT64 val = TN_value(*src2);

    if (val == 32) {
      switch (mtype) {
        case MTYPE_I4:
        case MTYPE_U4:
        case MTYPE_I5:
        case MTYPE_U5:
	  return TOP_GP32_SHRU32_GT_DR_DR;
      }
    }

    if (ISA_LC_Value_In_Class(val, LC_u5)) {
      switch (mtype) {
        case MTYPE_I4:
        case MTYPE_U4:
	  return TOP_GP32_SHRUW_GT_DR_DR_U5;
        case MTYPE_I5:
        case MTYPE_U5:
	  return TOP_GP32_SHRU_GT_DR_DR_U5;
        default:
	  *src2 = Expand_Immediate_Into_Register (mtype, *src2, ops);
      }
    }
  }

  switch (mtype) {
    case MTYPE_I4:
    case MTYPE_U4:
      return TOP_GP32_SHRUW_GT_DR_DR_DR;
    case MTYPE_I5:
    case MTYPE_U5:
      return TOP_GP32_SHRU_GT_DR_DR_DR;
    default:
      return TOP_UNDEFINED;
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

  Build_OP (new_opcode, result, True_TN, src1, src2, ops);
  return;
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
  TN **src2,
  OPS *ops
)
{
  TN *tmp;

  FmtAssert(TN_has_value(*src2),
	    ("Expand_Shift_Multiply: second TN is not immediate"));

  // see if I can do a shift instead of the multiply
  if (TN_value(*src2) == 0) {
    // shouldn't happen
    Exp_COPY (result, *src2, ops);
  }
  else if (TN_value(*src2) == 1) {
    Exp_COPY (result, src1, ops);
  }
  else if (MTYPE_is_class_integer(mtype) && TN_value(*src2) == 2) {
    // shift left by 1 in DU
    tmp = Gen_Literal_TN (1, MTYPE_byte_size(mtype));
    Expand_Shift (result, src1, tmp, mtype, shift_left, ops);
  }
  else if (MTYPE_is_class_integer(mtype) && TN_value(*src2) == 4) {
    // shift left by 2 in DU
    tmp = Gen_Literal_TN (2, MTYPE_byte_size(mtype));
    Expand_Shift (result, src1, tmp, mtype, shift_left, ops);
  }
  else if (MTYPE_is_class_integer(mtype) && TN_value(*src2) == 8) {
    // shift left by 3 in DU
    tmp = Gen_Literal_TN (3, MTYPE_byte_size(mtype));
    Expand_Shift (result, src1, tmp, mtype, shift_left, ops);
  }
  else if (MTYPE_is_class_integer(mtype) && TN_value(*src2) == 16) {
    // shift left by 4 in DU
    tmp = Gen_Literal_TN (4, MTYPE_byte_size(mtype));
    Expand_Shift (result, src1, tmp, mtype, shift_left, ops);
  }
  else {
    // stop madness here !!
    *src2 = Expand_Immediate_Into_Register (MTYPE_I4, *src2, ops);
    return FALSE;
  }

  return TRUE;
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
  INT64    constant;
  BOOL     done = FALSE;

  //
  // Check for two constants
  // 
  if ((TN_has_value(src1) || TN_is_rematerializable(src1)) &&
      (TN_has_value(src2) || TN_is_rematerializable(src2))) {
    // Two constants can sometimes occur because of DIVREM production in 
    TN *val_tn;
    constant = TN_has_value(src1) ? 
                           TN_value(src1) : WN_const_val(TN_home(src1));
    constant *= TN_has_value(src2) ? 
                           TN_value(src2) : WN_const_val(TN_home(src2));
    // Need to get the constant of the right length
    constant = Targ_To_Host(Host_To_Targ(rmtype, constant));
    val_tn = Gen_Literal_TN(constant, 8);
    Exp_Immediate(result, val_tn, MTYPE_signed(rmtype), ops);
    return;
  }

  // 
  // One of the srcs is constant
  //
  if (TN_has_value(src1)) {
    // is src2 of the same type as result ?
    // as long as I need to move to DU, I might as well
    // do it before, so the possible shift range is larger
    if (!MTYPE_is_class_integer(rmtype)) {
      // I will make the move later, now generate for *s2mtype
      dest = Build_TN_Of_Mtype (s2mtype);
      mtype = s2mtype;
    }

    done = Expand_Shift_Multiply (dest, mtype, src2, &src1, ops);
  }

  if (TN_has_value(src2)) {
    // is src1 of the same type as result ?
    // as long as I need to move to DU, I might as well
    // do it before, so the possible shift range is larger
    if (!MTYPE_is_class_integer(rmtype)) {
      // I will make the move later, now generate for s1mtype
      dest = Build_TN_Of_Mtype (s1mtype);
      mtype = s1mtype;
    }

    done = Expand_Shift_Multiply (dest, mtype, src1, &src2, ops);
  }

  // If I did not generate the optimized sequence, emit intrinsics
  if (!done) {
    // We can only perform multiplication in the DU
    if (MTYPE_is_class_pointer(rmtype)) {
      mtype = MTYPE_I4;
      dest = Build_TN_Of_Mtype (mtype);

      // MTYPEs of operands must be the same as mtype, i.e.
      // I must move them first into the DU
      if (TN_is_register(src1)) {
	TN *tmp = Build_TN_Of_Mtype (mtype);
	Exp_COPY (tmp, src1, ops);
	src1 = tmp;
      }
      if (TN_is_register(src2)) {
	TN *tmp = Build_TN_Of_Mtype (mtype);
	Exp_COPY (tmp, src2, ops);
	src2 = tmp;
      }
    }
    else if (!MTYPE_is_class_integer(rmtype)) {
      FmtAssert(FALSE,("Expand_Multiply: can't handle mtype %s",
                                              MTYPE_name(rmtype)));
    }

    // I have some integer result mtype.
    // I should try to strength reduce the expression.
    TOP opcode = TOP_UNDEFINED;
    switch (mtype) {

    case MTYPE_I2:
      if (s1mtype == MTYPE_I2 && s2mtype == MTYPE_I2 ||
	  s1mtype == MTYPE_I2 && s2mtype == MTYPE_U2 ||
	  s1mtype == MTYPE_U2 && s2mtype == MTYPE_I2 ||
	  s1mtype == MTYPE_U2 && s2mtype == MTYPE_U2) 
	opcode = TOP_UNDEFINED;
      break;

    case MTYPE_U2:
      if (s1mtype == MTYPE_U2 && s2mtype == MTYPE_U2) 
	opcode = TOP_UNDEFINED;
	//	opcode = TOP_IFR_MULUH_GT_DR_DR_DR;
      break;

    case MTYPE_I4:
      if (s1mtype == MTYPE_I4 && s2mtype == MTYPE_I4 ||
	  s1mtype == MTYPE_I4 && s2mtype == MTYPE_U4 ||
	  s1mtype == MTYPE_U4 && s2mtype == MTYPE_I4 ||
	  s1mtype == MTYPE_U4 && s2mtype == MTYPE_U4 ||
	  s1mtype == MTYPE_I2 && s2mtype == MTYPE_I4 ||
	  s1mtype == MTYPE_I2 && s2mtype == MTYPE_U4 ||
	  s1mtype == MTYPE_I4 && s2mtype == MTYPE_I2 ||
	  s1mtype == MTYPE_I4 && s2mtype == MTYPE_U2 ||
	  s1mtype == MTYPE_U2 && s2mtype == MTYPE_I4 ||
	  s1mtype == MTYPE_U2 && s2mtype == MTYPE_U4 ||
	  s1mtype == MTYPE_U4 && s2mtype == MTYPE_I2 ||
	  s1mtype == MTYPE_U4 && s2mtype == MTYPE_U2) {

	if (Lai_Code) {
	  INTRINSIC id = INTRN_MULW;
	  INT num_results = 1;
	  INT num_opnds = 4;
	  TN *result[1];
	  TN *opnd[4];
	  TY_IDX  ty = Make_Function_Type(WN_intrinsic_return_ty(id));
	  ST     *st = Gen_Intrinsic_Function(ty, INTRN_c_name(id));
	  result[0] = dest;
	  opnd[0] = True_TN;
	  opnd[1] = Gen_Symbol_TN (st, 0, TN_RELOC_NONE);
	  opnd[2] = src1;
	  opnd[3] = src2;

	  // create intrcall op
	  opcode = TOP_intrncall;
	  OP* intrncall_op = 
	    Mk_VarOP(opcode, num_results, num_opnds, result, opnd);
	  OPS_Append_Op(ops, intrncall_op);

	  //	  opcode = TOP_IFR_MULW_GT_DR_DR_DR;
	  //	  Build_OP(opcode, dest, True_TN, src1, src2, ops);
	}
	else {
	  TN *tmp = Build_RCLASS_TN(ISA_REGISTER_CLASS_du);
	  // r12 = HighS(R0) * LowU(R1) 
	  Build_OP(TOP_GP32_MPSUHL_GT_DR_DR_DR, tmp, True_TN, src1, src2, ops);
	  // r12'= LowU(R0) * HighS(R1) + HighS(R0) * LowU(R1)
	  TN *tmp1 = Build_RCLASS_TN(ISA_REGISTER_CLASS_du);
	  Build_OP(TOP_GP32_MAUSLH_GT_DR_DR_DR_DR, 
                                        tmp1, True_TN, tmp, src1, src2, ops);
	  // 
	  TN *tmp2 = Build_RCLASS_TN(ISA_REGISTER_CLASS_du);
	  Build_OP(TOP_GP32_SHLU_GT_DR_DR_U5, 
                            tmp2, True_TN, tmp1, Gen_Literal_TN(16, 4), ops);
	  // Ro = R0*R1
	  opcode = TOP_GP32_MAUULL_GT_DR_DR_DR_DR;
	  Build_OP(opcode, dest, True_TN, tmp2, src1, src2, ops);
	}
      }
      else if (s1mtype == MTYPE_I2 && s2mtype == MTYPE_I2) {
	opcode = TOP_GP32_MPSSLL_GT_DR_DR_DR;
	Build_OP (opcode, dest, True_TN, src1, src2, ops);
      }
      else if (s1mtype == MTYPE_I2 && s2mtype == MTYPE_U2) {
	opcode = TOP_GP32_MPSULL_GT_DR_DR_DR;
	Build_OP (opcode, dest, True_TN, src1, src2, ops);
      }
      else if (s1mtype == MTYPE_U2 && s2mtype == MTYPE_I2) {
	opcode = TOP_GP32_MPUSLL_GT_DR_DR_DR;
	Build_OP (opcode, dest, True_TN, src1, src2, ops);
      }
      else if (s1mtype == MTYPE_U2 && s2mtype == MTYPE_U2) {
	opcode = TOP_GP32_MPUULL_GT_DR_DR_DR;
	Build_OP (opcode, dest, True_TN, src1, src2, ops);
      }
      break;

    case MTYPE_U4:
      if (s1mtype == MTYPE_U4 && s2mtype == MTYPE_U4 ||
	  s1mtype == MTYPE_U2 && s2mtype == MTYPE_U4 ||
	  s1mtype == MTYPE_U4 && s2mtype == MTYPE_U2) {
	if (Lai_Code) {
	  INTRINSIC id = INTRN_MULUW;
	  INT num_results = 1;
	  INT num_opnds = 4;
	  TN *result[1];
	  TN *opnd[4];
	  TY_IDX  ty = Make_Function_Type(WN_intrinsic_return_ty(id));
	  ST     *st = Gen_Intrinsic_Function(ty, INTRN_c_name(id));
	  result[0] = dest;
	  opnd[0] = True_TN;
	  opnd[1] = Gen_Symbol_TN (st, 0, TN_RELOC_NONE);
	  opnd[2] = src1;
	  opnd[3] = src2;

	  // create intrcall op
	  opcode = TOP_intrncall;
	  OP* intrncall_op = 
	    Mk_VarOP(opcode, num_results, num_opnds, result, opnd);
	  OPS_Append_Op(ops, intrncall_op);

	  //	  opcode = TOP_IFR_MULUW_GT_DR_DR_DR;
	  //	  Build_OP(opcode, dest, True_TN, src1, src2, ops);
	}
	else {
	  TN *tmp = Build_RCLASS_TN(ISA_REGISTER_CLASS_du);
	  // r12 = HighS(R0) * LowU(R1) 
	  Build_OP(TOP_GP32_MPUUHL_GT_DR_DR_DR, tmp, True_TN, src1, src2, ops);
	  // r12'= LowU(R0) * HighS(R1) + HighS(R0) * LowU(R1)
	  TN *tmp1 = Build_RCLASS_TN(ISA_REGISTER_CLASS_du);
	  Build_OP(TOP_GP32_MAUULH_GT_DR_DR_DR_DR, 
                                        tmp1, True_TN, tmp, src1, src2, ops);
	  // 
	  TN *tmp2 = Build_RCLASS_TN(ISA_REGISTER_CLASS_du);
	  Build_OP(TOP_GP32_SHLU_GT_DR_DR_U5, 
                            tmp2, True_TN, tmp1, Gen_Literal_TN(16, 4), ops);
	  // Ro = R0*R1
	  opcode = TOP_GP32_MAUULL_GT_DR_DR_DR_DR;
	  Build_OP(opcode, dest, True_TN, tmp2, src1, src2, ops);
	}
      }
      else if (s1mtype == MTYPE_I4 && s2mtype == MTYPE_I4 ||
	       s1mtype == MTYPE_I4 && s2mtype == MTYPE_U4 ||
	       s1mtype == MTYPE_U4 && s2mtype == MTYPE_I4 ||
	       s1mtype == MTYPE_I2 && s2mtype == MTYPE_I4 ||
	       s1mtype == MTYPE_I2 && s2mtype == MTYPE_U4 ||
	       s1mtype == MTYPE_I4 && s2mtype == MTYPE_I2 ||
	       s1mtype == MTYPE_I4 && s2mtype == MTYPE_U2 ||
	       s1mtype == MTYPE_U2 && s2mtype == MTYPE_I4 ||
	       s1mtype == MTYPE_U4 && s2mtype == MTYPE_I2) {
	if (Lai_Code) {
	  INTRINSIC id = INTRN_MULUW;
	  INT num_results = 1;
	  INT num_opnds = 4;
	  TN *result[1];
	  TN *opnd[4];
	  TY_IDX  ty = Make_Function_Type(WN_intrinsic_return_ty(id));
	  ST     *st = Gen_Intrinsic_Function(ty, INTRN_c_name(id));
	  result[0] = dest;
	  opnd[0] = True_TN;
	  opnd[1] = Gen_Symbol_TN (st, 0, TN_RELOC_NONE);
	  opnd[2] = src1;
	  opnd[3] = src2;

	  // create intrcall op
	  opcode = TOP_intrncall;
	  OP* intrncall_op = 
	    Mk_VarOP(opcode, num_results, num_opnds, result, opnd);
	  OPS_Append_Op(ops, intrncall_op);

	  //	opcode = TOP_IFR_MULW_GT_DR_DR_DR;
	}
	else {
	  TN *tmp = Build_RCLASS_TN(ISA_REGISTER_CLASS_du);
	  // r12 = HighS(R0) * LowU(R1) 
	  Build_OP(TOP_GP32_MPSUHL_GT_DR_DR_DR, tmp, True_TN, src1, src2, ops);
	  // r12'= LowU(R0) * HighS(R1) + HighS(R0) * LowU(R1)
	  TN *tmp1 = Build_RCLASS_TN(ISA_REGISTER_CLASS_du);
	  Build_OP(TOP_GP32_MAUSLH_GT_DR_DR_DR_DR, 
                                        tmp1, True_TN, tmp, src1, src2, ops);
	  // 
	  TN *tmp2 = Build_RCLASS_TN(ISA_REGISTER_CLASS_du);
	  Build_OP(TOP_GP32_SHLU_GT_DR_DR_U5, 
                            tmp2, True_TN, tmp1, Gen_Literal_TN(16, 4), ops);
	  // Ro = R0*R1
	  opcode = TOP_GP32_MAUULL_GT_DR_DR_DR_DR;
	  Build_OP(opcode, dest, True_TN, tmp2, src1, src2, ops);
	}
      }
      else if (s1mtype == MTYPE_U2 && s2mtype == MTYPE_U2) {
	opcode = TOP_GP32_MPUULL_GT_DR_DR_DR;
	Build_OP (opcode, dest, True_TN, src1, src2, ops);
      }
      break;

    case MTYPE_I5:
      if (s1mtype == MTYPE_I5 && s2mtype == MTYPE_I5 ||
	  s1mtype == MTYPE_I5 && s2mtype == MTYPE_U5 ||
	  s1mtype == MTYPE_U5 && s2mtype == MTYPE_I5 ||
	  s1mtype == MTYPE_U5 && s2mtype == MTYPE_U5 ||
	  s1mtype == MTYPE_I4 && s2mtype == MTYPE_I4 ||
	  s1mtype == MTYPE_I4 && s2mtype == MTYPE_U4 ||
	  s1mtype == MTYPE_U4 && s2mtype == MTYPE_I4 ||
	  s1mtype == MTYPE_U4 && s2mtype == MTYPE_U4 ) 
	opcode = TOP_UNDEFINED;
	//	opcode = TOP_IFR_MULE_GT_DR_DR_DR;
      else if (s1mtype == MTYPE_I2 && s2mtype == MTYPE_I2) {
	opcode = TOP_GP32_MPSSLL_GT_DR_DR_DR;
	Build_OP (opcode, dest, True_TN, src1, src2, ops);
      }
      else if (s1mtype == MTYPE_I2 && s2mtype == MTYPE_U2) {
	opcode = TOP_GP32_MPSULL_GT_DR_DR_DR;
	Build_OP (opcode, dest, True_TN, src1, src2, ops);
      }
      else if (s1mtype == MTYPE_U2 && s2mtype == MTYPE_I2) {
	opcode = TOP_GP32_MPUSLL_GT_DR_DR_DR;
	Build_OP (opcode, dest, True_TN, src1, src2, ops);
      }
      else if (s1mtype == MTYPE_U2 && s2mtype == MTYPE_U2) {
	opcode = TOP_GP32_MPUULL_GT_DR_DR_DR;
	Build_OP (opcode, dest, True_TN, src1, src2, ops);
      }
      break;

    case MTYPE_U5:
      if (s1mtype == MTYPE_U5 && s2mtype == MTYPE_U5 ||
	  s1mtype == MTYPE_U4 && s2mtype == MTYPE_U4 )
	opcode = TOP_UNDEFINED;
	//	opcode = TOP_IFR_MULUE_GT_DR_DR_DR;
      else if (s1mtype == MTYPE_U2 && s2mtype == MTYPE_U2) {
	opcode = TOP_GP32_MPUULL_GT_DR_DR_DR;
	Build_OP (opcode, dest, True_TN, src1, src2, ops);
      }
      break;

    default:
      FmtAssert(FALSE, ("Expand_Multiply: unknown return mtype %s\n", 
                                                    MTYPE_name(mtype)));
    }

    if (opcode == TOP_UNDEFINED)
      FmtAssert(FALSE, ("Expand_Multiply: can't make mpy %s <- %s * %s",
	       MTYPE_name(mtype), MTYPE_name(s1mtype), MTYPE_name(s2mtype)));

    //    Build_OP (opcode, dest, True_TN, src1, src2, ops);

  } /* if (!done) */

  // If I made a mult in some other FU than result's -- move it there
  if (dest != result)
    Exp_COPY (result, dest, ops);

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

  case MTYPE_A4:
    /* 
     * make a MADD in the AU if possible:
     */
    if (TN_has_value(src2)) {
      switch (TN_value(src2)) {
	case 1:
	  Build_OP (TOP_GP32_ADDBA_GT_AR_AR_AR, 
                               result, True_TN, src0, src1, ops);
	  return;
	case 2:
	  Build_OP (TOP_GP32_ADDHA_GT_AR_AR_AR, 
                               result, True_TN, src0, src1, ops);
	  return;
	case 3: 
	  tmp = Gen_Register_TN(ISA_REGISTER_CLASS_au, 4);
	  Build_OP (TOP_GP32_ADDHA_GT_AR_AR_AR, 
                               tmp, True_TN, src0, src1, ops);
	  Build_OP (TOP_GP32_ADDBA_GT_AR_AR_AR, 
                               result, True_TN, tmp, src1, ops);
	  return;
	case 4:
	  Build_OP (TOP_GP32_ADDWA_GT_AR_AR_AR, 
                               result, True_TN, src0, src1, ops);
	  return;
	case 16:
	  tmp = Gen_Register_TN(ISA_REGISTER_CLASS_au, 4);
	  Build_OP (TOP_GP32_ADDHA_GT_AR_AR_AR, 
                               tmp, True_TN, src0, src1, ops);
	  Build_OP (TOP_GP32_ADDWA_GT_AR_AR_AR, 
                               result, True_TN, tmp, src1, ops);
	  return;
	default:
	  FmtAssert(0, 
	    ("Expand_Madd: not handled AU const %lld\n", TN_value(src2)));
      }
      return;

    } /* TN_has_value(src2) */
    break;

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

  case MTYPE_I5:

    FmtAssert(MTYPE_size_min(s0mtype) <= 40, 
	      ("Expand_Madd: I5 = %s + a*b", MTYPE_name(s0mtype)));

    if (s1mtype == MTYPE_I2 && s2mtype == MTYPE_I2) 
      //      opcode = TOP_IFR_MASSE_GT_DR_DR_DR_DR;
      opcode = TOP_UNDEFINED;
    else if (s1mtype == MTYPE_I2 && s2mtype == MTYPE_U2) 
      //      opcode = TOP_IFR_MASUE_GT_DR_DR_DR_DR;
      opcode = TOP_UNDEFINED;
    else if (s1mtype == MTYPE_U2 && s2mtype == MTYPE_I2) 
      //      opcode = TOP_IFR_MAUSE_GT_DR_DR_DR_DR;
      opcode = TOP_UNDEFINED;
    else if (s1mtype == MTYPE_U2 && s2mtype == MTYPE_U2) 
      //      opcode = TOP_IFR_MAUUE_GT_DR_DR_DR_DR;
      opcode = TOP_UNDEFINED;
    else if (s1mtype == MTYPE_I4 && s2mtype == MTYPE_I4) 
      //      opcode = TOP_IFR_MAWE_GT_DR_DR_DR_DR;
      opcode = TOP_UNDEFINED;
    else
      FmtAssert(FALSE, ("Expand_Madd: I5 = I5 + %s * %s", 
			MTYPE_name(s1mtype), MTYPE_name(s2mtype)));
    break;

  case MTYPE_U5:

    FmtAssert(s0mtype == MTYPE_U5, 
	      ("Expand_Madd: U5 = %s + a*b", MTYPE_name(s0mtype)));

    if (s1mtype == MTYPE_U4 && s2mtype == MTYPE_U4) 
      //      opcode = TOP_IFR_MAWUE_GT_DR_DR_DR_DR;
      opcode = TOP_UNDEFINED;
    else
      FmtAssert(FALSE, ("Expand_Madd: MTYPE_U5 -- operands messed"));
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
  FmtAssert(FALSE,("Not Implemented"));

  TN *p1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_guard);
  TN *p2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_guard);
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
  FmtAssert(FALSE,("Not Implemented"));

  /* dest = (src == 0) ? 1 : 0 */
  if (TN_register_class(dest) == ISA_REGISTER_CLASS_guard) {
    TN *p1 = dest;
    TN *p2 = Get_Complement_TN(dest);
    if (TN_register_class(src) == ISA_REGISTER_CLASS_guard) {
      Build_OP (TOP_GP32_EQW_GT_BR_DR_DR, p1, p2, True_TN, Zero_TN, Zero_TN, ops);
      Build_OP (TOP_GP32_NEW_GT_BR_DR_DR, p1, p2, src, Zero_TN, Zero_TN, ops);
    } else {
      Build_OP (TOP_GP32_NEW_GT_BR_DR_DR, p1, p2, True_TN, src, Zero_TN, ops);
    }
  } else {
   /*
    *  if CG_EXP_normalize is true we must normalized the operands
    *  (if not already normalized)
    */
    if (!V_normalized_op1(variant) && CGEXP_normalize_logical)
    {
      Expand_Normalize_Logical (src, ops);
    }
    Build_OP (TOP_GP32_XOR_GT_DR_DR_U8, dest, True_TN, Gen_Literal_TN(1, 4), src, ops);
  }
}

/* ====================================================================
 *   Expand_Logical_Not
 *
 *
 *	dest = (src1 != 0 & src2 != 0) ? 1 : 0 
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
  FmtAssert(FALSE,("Not Implemented"));

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

  Build_OP (action, dest, True_TN, src1, src2, ops);
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
  Expand_Logical_And_Or (TOP_GP32_AND_GT_DR_DR_DR, dest, 
                                          src1, src2, variant, ops);
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
  Expand_Logical_And_Or (TOP_GP32_OR_GT_DR_DR_DR, dest, 
                                          src1, src2, variant, ops);
}

/* ====================================================================
 *   Expand_Binary_Compliment
 * ====================================================================
 */
void
Expand_Binary_Complement (
  TN *dest, 
  TN *src, 
  TYPE_ID /* mtype */, 
  OPS *ops
)
{
  Is_True(TN_register_class(dest) == TN_register_class(src),
	  ("inconsistent data"));

  /* complement == nor src $0 */
  TN *src2 = Build_TN_Like (src);
  Build_OP (TOP_GP32_MAKE_GT_DR_S16, src2, True_TN, Gen_Literal_TN(0, 4), ops);
  Build_OP (TOP_GP32_NOR_GT_DR_DR_DR, dest, True_TN, src, src2, ops);

  //Build_OP (TOP_GP32_XOR_GT_DR_DR_U8, dest, True_TN, src,
  //                               Gen_Literal_TN(-1, 4), ops);
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
  if (TN_is_constant(src1)) {
    // swap operands:
    TN *swap_tn = src1;
    src1 = src2;
    src2 = swap_tn;
  }

  FmtAssert (TN_is_register(src1),
	     ("Expand_Binary_And_Or: 1st operand must be register"));


  if (TN_has_value(src2)) {
    if (ISA_LC_Value_In_Class(TN_value(src2), LC_u8)) {
      action = Pick_Imm_Form_TOP (action);
    }
    else {
      src2 = Expand_Immediate_Into_Register (mtype, src2, ops);
    }
  }

  // mtype indicates what type of intermediate result is used:
  if (Register_Class_For_Mtype(mtype) != TN_register_class(dest)) {
    TN *tmp = Build_RCLASS_TN (Register_Class_For_Mtype(mtype));
    Build_OP (action, tmp, True_TN, src1, src2, ops);
    Exp_COPY (dest, tmp, ops);
  }
  else {
    Build_OP (action, dest, True_TN, src1, src2, ops);
  }

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
  switch (mtype) {
  case MTYPE_I4:
  case MTYPE_U4:
    Expand_Binary_And_Or (TOP_GP32_AND_GT_DR_DR_DR, 
                                    dest, src1, src2, mtype, ops);
    break;
  case MTYPE_B:
    Expand_Binary_And_Or (TOP_GP32_ANDG_GT_BR_BR_BR, 
                                    dest, src1, src2, mtype, ops);
    break;
  default:
    FmtAssert(FALSE, ("Expand_Binary_Xor: mtype not handled"));
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
  switch (mtype) {
  case MTYPE_I4:
  case MTYPE_U4:
    Expand_Binary_And_Or (TOP_GP32_OR_GT_DR_DR_DR, 
                                    dest, src1, src2, mtype, ops);
    break;
  case MTYPE_B:
    Expand_Binary_And_Or (TOP_GP32_ORG_GT_BR_BR_BR, 
                                    dest, src1, src2, mtype, ops);
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
  switch (mtype) {
  case MTYPE_I4:
  case MTYPE_U4:
    Expand_Binary_And_Or (TOP_GP32_XOR_GT_DR_DR_DR, 
                                    dest, src1, src2, mtype, ops);
    break;
  case MTYPE_B:
    Expand_Binary_And_Or (TOP_GP32_XORG_GT_BR_BR_BR, 
                                    dest, src1, src2, mtype, ops);
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
}

/* ====================================================================
 *   Expand_Int_Comparison
 * ====================================================================
 */
static void
Expand_Int_Comparison (
  TOP action, 
  TN *dest, 
  TN *src1, 
  TN *src2, 
  OPS *ops
)
{
#if 0
  if (TN_register_class(dest) == ISA_REGISTER_CLASS_guard) {
    // build comparison OP, result is in predicate register:
    Build_OP (action, dest, True_TN, src1, src2, ops);
    /* nada */ ;
  } 
  else if (TN_register_class(dest) == ISA_REGISTER_CLASS_du) {
    TN *tmp = Build_RCLASS_TN (ISA_REGISTER_CLASS_guard);
    // build comparison OP, result is in predicate register:
    Build_OP (action, tmp, True_TN, src1, src2, ops);
    /*
     * Move to int
     */
    Build_OP (TOP_GP32_BOOL_GT_DR_BR, dest, True_TN, tmp, ops);
  }
  else {
    FmtAssert(FALSE, 
	      ("Expand_Int_Comparison: unhandled cmp target TN"));
  }
#else
  FmtAssert(TN_register_class(dest) == ISA_REGISTER_CLASS_du,
	      ("Expand_Int_Comparison: cmp target TN not int"));

  TN *tmp = Build_RCLASS_TN (ISA_REGISTER_CLASS_guard);
  // build comparison OP, result is in predicate register:
  Build_OP (action, tmp, True_TN, src1, src2, ops);
  /*
   * Move to int
   */
  Build_OP (TOP_GP32_BOOL_GT_DR_BR, dest, True_TN, tmp, ops);
#endif

  return;
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

#if 0
  if (TN_register_class(dest) == ISA_REGISTER_CLASS_guard) {
    // return result of comparison in a predicate register
    TOP action = equals ? TOP_GP32_NEW_GT_BR_DR_DR : TOP_GP32_EQW_GT_BR_DR_DR;
    TN *p1 = dest;
    TN *p2 = Get_Complement_TN(dest);
    TN *tn = Build_TN_Of_Mtype (MTYPE_I4);

    // generate: tn = (src1 == src2)
    Build_OP (TOP_GP32_MOVE_GT_DR_DR, tn, True_TN, 
                                           Gen_Literal_TN(1, 4), ops);
    Build_OP (TOP_GP32_XOR_GT_DR_DR_U8, tn, src1, 
                                       Gen_Literal_TN(1, 4), tn, ops);
    Build_OP (TOP_GP32_XOR_GT_DR_DR_U8, tn, src2, 
                                       Gen_Literal_TN(1, 4), tn, ops);

    Build_OP (action, p1, p2, True_TN, tn, Zero_TN, ops);
  } else {
    Build_OP (TOP_GP32_MOVE_GT_DR_DR, dest, True_TN, 
                                      Gen_Literal_TN(equals, 4), ops);
    Build_OP (TOP_GP32_XOR_GT_DR_DR_U8, dest, src1, 
                                     Gen_Literal_TN(1, 4), dest, ops);
    Build_OP (TOP_GP32_XOR_GT_DR_DR_U8, dest, src2, 
                                     Gen_Literal_TN(1, 4), dest, ops);
  }
#endif

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
  TYPE_ID mtype, 
  OPS *ops
)
{
  VARIANT variant;
  TOP action;
  switch (mtype) {
  case MTYPE_I8: variant = V_BR_I8LT; break;
  case MTYPE_I4: variant = V_BR_I4LT; break;
  case MTYPE_U8: variant = V_BR_U8LT; break;
  case MTYPE_U4: variant = V_BR_U4LT; break;
  default:
    #pragma mips_frequency_hint NEVER
    Is_True(FALSE, ("Expand_Int_Less: MTYPE_%s is not handled", 
		                               Mtype_Name(mtype)));
  }
  action = Pick_Compare_TOP (&variant, &src1, &src2, ops);
  //  Expand_Int_Comparison (action, dest, src1, src2, ops);
  if (TN_register_class(dest) == ISA_REGISTER_CLASS_guard) {
    Build_OP (action, dest, True_TN, src1, src2, ops);
  } 
  else if (TN_register_class(dest) == ISA_REGISTER_CLASS_du) {
    Expand_Int_Comparison (action, dest, src1, src2, ops);
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
  TYPE_ID mtype, 
  OPS *ops
)
{
  VARIANT variant;
  TOP action;
  switch (mtype) {
  case MTYPE_I8: variant = V_BR_I8LE; break;
  case MTYPE_I4: variant = V_BR_I4LE; break;
  case MTYPE_U8: variant = V_BR_U8LE; break;
  case MTYPE_U4: variant = V_BR_U4LE; break;
  default:
    #pragma mips_frequency_hint NEVER
    Is_True(FALSE, ("Expand_Int_Less_Equal: MTYPE_%s is not handled", 
                                                  Mtype_Name(mtype)));
  }
  action = Pick_Compare_TOP (&variant, &src1, &src2, ops);
  //  Expand_Int_Comparison (action, dest, src1, src2, ops);
  if (TN_register_class(dest) == ISA_REGISTER_CLASS_guard) {
    Build_OP (action, dest, True_TN, src1, src2, ops);
  } 
  else if (TN_register_class(dest) == ISA_REGISTER_CLASS_du) {
    Expand_Int_Comparison (action, dest, src1, src2, ops);
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
  TYPE_ID mtype, 
  OPS *ops
)
{
  VARIANT variant;
  TOP action;
  switch (mtype) {
  case MTYPE_I8: variant = V_BR_I8EQ; break;
  case MTYPE_I4: variant = V_BR_I4EQ; break;
  case MTYPE_U8: variant = V_BR_U8EQ; break;
  case MTYPE_U4: variant = V_BR_U4EQ; break;
  case MTYPE_A8: variant = V_BR_A8EQ; break;
  case MTYPE_A4: variant = V_BR_A4EQ; break;

  default:
    #pragma mips_frequency_hint NEVER
    Is_True(FALSE, ("Expand_Int_Equal: MTYPE_%s is not handled", 
                                                  Mtype_Name(mtype)));
  }
  action = Pick_Compare_TOP (&variant, &src1, &src2, ops);
  //  Expand_Int_Comparison (action, dest, src1, src2, ops);
  if (TN_register_class(dest) == ISA_REGISTER_CLASS_guard) {
    Build_OP (action, dest, True_TN, src1, src2, ops);
  } 
  else if (TN_register_class(dest) == ISA_REGISTER_CLASS_du) {
    Expand_Int_Comparison (action, dest, src1, src2, ops);
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
  TYPE_ID mtype, 
  OPS *ops
)
{
  VARIANT variant;
  TOP   action;
  TN   *tmp1, *tmp2;

  FmtAssert(TN_register_class(src1) == ISA_REGISTER_CLASS_du &&
            (!TN_is_register(src2) || TN_register_class(src2) == ISA_REGISTER_CLASS_du),
	  ("Expand_Int_Not_Equal: operands have wrong RClass"));

  switch (mtype) {
  case MTYPE_I8: 
    variant = V_BR_I8NE; 
    break;
  case MTYPE_I4: 
    variant = V_BR_I4NE; 
    break;
  case MTYPE_I5: 
    variant = V_BR_I5NE; 
    break;
  case MTYPE_U8: 
    variant = V_BR_U8NE; 
    break;
  case MTYPE_U4: 
    variant = V_BR_U4NE; 
    break;
  case MTYPE_U5: 
    variant = V_BR_U5NE; 
    break;
  default:
    #pragma mips_frequency_hint NEVER
    Is_True(FALSE, ("Expand_Int_Not_Equal: MTYPE_%s is not handled", 
                                                   Mtype_Name(mtype)));
  }
  action = Pick_Compare_TOP (&variant, &src1, &src2, ops);
  //  Expand_Int_Comparison (action, dest, src1, src2, ops);
  if (TN_register_class(dest) == ISA_REGISTER_CLASS_guard) {
    Build_OP (action, dest, True_TN, src1, src2, ops);
  } 
  else if (TN_register_class(dest) == ISA_REGISTER_CLASS_du) {
    Expand_Int_Comparison (action, dest, src1, src2, ops);
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
  TYPE_ID mtype, 
  OPS *ops
)
{
  VARIANT variant;
  TOP action;
  switch (mtype) {
  case MTYPE_I8: variant = V_BR_I8GE; break;
  case MTYPE_I4: variant = V_BR_I4GE; break;
  case MTYPE_U8: variant = V_BR_U8GE; break;
  case MTYPE_U4: variant = V_BR_U4GE; break;
  default:
    #pragma mips_frequency_hint NEVER
    Is_True(FALSE, ("Expand_Int_Greater_Equal: MTYPE_%s is not handled", 
                                                     Mtype_Name(mtype)));
  }
  action = Pick_Compare_TOP (&variant, &src1, &src2, ops);
  //  Expand_Int_Comparison (action, dest, src1, src2, ops);
  if (TN_register_class(dest) == ISA_REGISTER_CLASS_guard) {
    Build_OP (action, dest, True_TN, src1, src2, ops);
  } 
  else if (TN_register_class(dest) == ISA_REGISTER_CLASS_du) {
    Expand_Int_Comparison (action, dest, src1, src2, ops);
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
  TYPE_ID mtype, 
  OPS *ops
)
{
  VARIANT variant;
  TOP action;

  switch (mtype) {
    case MTYPE_I8: variant = V_BR_I8GT; break;
    case MTYPE_I4: variant = V_BR_I4GT; break;
    case MTYPE_I5: variant = V_BR_I5GT; break;
    case MTYPE_U8: variant = V_BR_U8GT; break;
    case MTYPE_U4: variant = V_BR_U4GT; break;
    case MTYPE_U5: variant = V_BR_U5GT; break;
    default:
      Is_True(FALSE, ("Expand_Int_Greater: MTYPE_%s is not handled", 
                                                  Mtype_Name(mtype)));
  }

  action = Pick_Compare_TOP (&variant, &src1, &src2, ops);
  //  Expand_Int_Comparison (action, dest, src1, src2, ops);

  if (TN_register_class(dest) == ISA_REGISTER_CLASS_guard) {
    Build_OP (action, dest, True_TN, src1, src2, ops);
  } 
  else if (TN_register_class(dest) == ISA_REGISTER_CLASS_du) {
    Expand_Int_Comparison (action, dest, src1, src2, ops);
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

  if (TN_is_register(src1)) {
    FmtAssert(TN_register_class(src1) == ISA_REGISTER_CLASS_au,
	            ("Expand_Ptr_Not_Equal: src1 has wrong RClass"));
  }
  else {
    FmtAssert(TN_is_register(src2), ("Expand_Ptr_Not_Equal: src2 is screwed"));
    src1 = Expand_Immediate_Into_Register (desc, src1, ops);
  }

  if (TN_is_register(src2)) {
    FmtAssert(TN_register_class(src2) == ISA_REGISTER_CLASS_au,
	            ("Expand_Ptr_Not_Equal: src2 has wrong RClass"));
  }
  else {
    FmtAssert(TN_is_register(src1), ("Expand_Ptr_Not_Equal: src1 is screwed"));
    src2 = Expand_Immediate_Into_Register (desc, src2, ops);
  }

  switch (desc) {
  case MTYPE_A4:
    variant = V_BR_A4NE; 
    break;
  default:
    Is_True(FALSE, ("Expand_Ptr_Not_Equal: MTYPE_%s is not pointer", 
                                                   Mtype_Name(desc)));
  }

  action = Pick_Compare_TOP (&variant, &src1, &src2, ops);

  if (TN_register_class(dest) == ISA_REGISTER_CLASS_guard) {
    Build_OP (action, dest, True_TN, src1, src2, ops);
  } 
  else if (TN_register_class(dest) == ISA_REGISTER_CLASS_du) {
    Expand_Int_Comparison (action, dest, src1, src2, ops);
  }
  else {
    FmtAssert(FALSE, ("Expand_Ptr_Not_Equal: unhandled cmp target TN"));
  }

#if 0
  /* Compare is done in DU, move them to DU first */
  if (TN_is_register(src1) &&
      TN_register_class (src1) != ISA_REGISTER_CLASS_du) {
    tmp1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_du);
    Exp_COPY(tmp1, src1, ops);
    src1 = tmp1;
  }

  if (TN_is_register(src2) &&
      TN_register_class (src2) != ISA_REGISTER_CLASS_du) {
    tmp2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_du);
    Exp_COPY(tmp2, src2, ops);
    src2 = tmp2;
  }

  switch (desc) {
    case MTYPE_A4:
      Expand_Int_Not_Equal (dest, src1, src2, MTYPE_U4, ops);
      break;

    case MTYPE_A8:
      Expand_Int_Not_Equal (dest, src1, src2, MTYPE_U8, ops);
      break;

    default:
      FmtAssert(FALSE, ("Expand_Ptr_Not_Equal: MTYPE_%s is not handled", 
                MTYPE_name(desc)));
  }
#endif

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
  Build_OP (TOP_GP32_BOOL_GT_DR_BR, dest, True_TN, src, ops);
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
  FmtAssert(FALSE,("Not Implemented"));

  Expand_Float_To_Int (ROUND_CHOP, dest, src, imtype, fmtype, ops);
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
  //      p2 = (src1 !cmp src2)
  // p1?  dst = true_tn
  // p2?  dst = false_tn
  // p1?  opposite_dst = true_tn
  // p2?  opposite_dst = false_tn
  //

  TOP cmp1 = Pick_Compare_TOP (&variant, &cond1, &cond2, ops);
  variant = Negate_BR_Variant(variant);
  TOP cmp2 = Pick_Compare_TOP (&variant, &cond1, &cond2, ops);

  TN *p1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_guard);
  TN *p2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_guard);

  Build_OP (cmp1, p1, True_TN, cond1, cond2, ops);
  Build_OP (cmp2, p2, True_TN, cond1, cond2, ops);

  Expand_Copy (dest, p1, true_tn, ops);
  Expand_Copy (dest, p2, false_tn, ops);
  if (opposite_dest) {
    Expand_Copy (opposite_dest, p2, true_tn, ops);
    Expand_Copy (opposite_dest, p1, false_tn, ops);
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
  const BOOL is_float = MTYPE_is_float(mtype);

  FmtAssert(FALSE,("Not Implemented"));

  if (TN_register_class(cond_tn) == ISA_REGISTER_CLASS_guard) {
    TOP tmove, fmove;
    TN *p1 = cond_tn;
    TN *p2 = Get_Complement_TN(cond_tn);
    if (is_float) {
      ;
    } else {
      tmove = TN_has_value(true_tn) ? TOP_GP32_MAKE_GT_DR_S16 : 
                                             TOP_GP32_MOVE_GT_DR_DR;
      fmove = TN_has_value(false_tn) ? TOP_GP32_MAKE_GT_DR_S16 : 
                                             TOP_GP32_MOVE_GT_DR_DR;
    }
    Build_OP (tmove, dest_tn, p1, true_tn, ops);
    Build_OP (fmove, dest_tn, p2, false_tn, ops);
  } else {
    // create compare of cond to 0
    TOP cmp;
    VARIANT variant;
    if (float_cond)
 	;
    else if (MTYPE_is_size_double(mtype))
	cmp = TOP_GP32_NEW_GT_BR_DR_DR;
    else
	cmp = TOP_GP32_NEW_GT_BR_DR_DR;
    Expand_Compare_And_Select (variant, cond_tn, Zero_TN, dest_tn, NULL,
	true_tn, false_tn, 
	is_float, ops);
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
  if (Lai_Code) {
    INTRINSIC id;
    INT num_results = 1;
    INT num_opnds = 4;
    TN *result[1];
    TN *opnd[4];

    // Just emit an intrinsic:
    switch (mtype) {
      case MTYPE_I2:
	id = INTRN_MINH;
	break;

      case MTYPE_U2:
	id = INTRN_MINUH;
	break;

      case MTYPE_I4:
	id = INTRN_MINW;
	break;

      case MTYPE_U4:
	id = INTRN_MINUW;
	break;

      case MTYPE_I5:
	id = INTRN_MINE;
	break;

      case MTYPE_U5:
	id = INTRN_MINUE;
	break;

      default:
	FmtAssert (FALSE,("Expand_Min: unhandled mtype"));
    }

    TY_IDX  ty = Make_Function_Type(WN_intrinsic_return_ty(id));
    ST     *st = Gen_Intrinsic_Function(ty, INTRN_c_name(id));
    result[0] = dest;
    opnd[0] = True_TN;
    opnd[1] = Gen_Symbol_TN (st, 0, TN_RELOC_NONE);
    opnd[2] = src1;
    opnd[3] = src2;

    // create intrcall op
    OP* intrncall_op = Mk_VarOP(TOP_intrncall, num_results, num_opnds, result, opnd);
    OPS_Append_Op(ops, intrncall_op);
  }
  else { 
    //
    // generate real code
    // t = s1 < s2; d = select t ? s1 : s2
    //
    TOP   cmp;
    VARIANT variant;

    switch (mtype) {
      case MTYPE_I8: variant = V_BR_I8LT; break;
      case MTYPE_I4: variant = V_BR_I4LT; break;
      case MTYPE_U8: variant = V_BR_U8LT; break;
      case MTYPE_U4: variant = V_BR_U4LT; break;

      case MTYPE_F4: 
      case MTYPE_F8: 
      case MTYPE_F10: 
      default:
	FmtAssert(FALSE, ("Expand_Min: unexpected mtype"));
    }

    cmp = Pick_Compare_TOP (&variant, &src1, &src2, ops);
    Expand_Compare_And_Select (variant, 
                 src1, src2, dest, NULL, src1, src2, FALSE, ops);
  }

  return;
}

/* ====================================================================
 *   Expand_Max
 *
 *   t = s1 > s2; d = select t ? s1 : s2
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
  TOP cmp;
  VARIANT variant;

  if (Lai_Code) {
    //
    // Just emit an intrinsic
    //
    INTRINSIC id;
    INT num_results = 1;
    INT num_opnds = 4;
    TN *result[1];
    TN *opnd[4];

    switch (mtype) {
      case MTYPE_I2:
	id = INTRN_MAXH;
	//      Build_OP (TOP_IFR_MAXH_GT_DR_DR_DR, dest, True_TN,
	//	                                         src1, src2, ops);
	break;

      case MTYPE_U2:
	id = INTRN_MAXUH;
	//      Build_OP (TOP_IFR_MAXUH_GT_DR_DR_DR, dest, True_TN,
	//	                                         src1, src2, ops);
	break;

      case MTYPE_I4:
	id = INTRN_MAXW;
	//      Build_OP (TOP_IFR_MAXW_GT_DR_DR_DR, dest, True_TN,
	//	                                         src1, src2, ops);
	break;

      case MTYPE_U4:
	id = INTRN_MAXUW;
	//      Build_OP (TOP_IFR_MAXUW_GT_DR_DR_DR, dest, True_TN,
	//	                                         src1, src2, ops);
	break;

      case MTYPE_I5:
	id = INTRN_MAXE;
	//      Build_OP (TOP_IFR_MAXE_GT_DR_DR_DR, dest, True_TN,
	//	                                         src1, src2, ops);
	break;

      case MTYPE_U5:
	id = INTRN_MAXUE;
	//      Build_OP (TOP_IFR_MAXUE_GT_DR_DR_DR, dest, True_TN,
	//	                                         src1, src2, ops);
	break;

      default:
	FmtAssert (FALSE,("Expand_Max: unhandled mtype"));
    }

    TY_IDX  ty = Make_Function_Type(WN_intrinsic_return_ty(id));
    ST     *st = Gen_Intrinsic_Function(ty, INTRN_c_name(id));
    result[0] = dest;
    opnd[0] = True_TN;
    opnd[1] = Gen_Symbol_TN (st, 0, TN_RELOC_NONE);
    opnd[2] = src1;
    opnd[3] = src2;

    // create intrcall op
    OP* intrncall_op = Mk_VarOP(TOP_intrncall, num_results, num_opnds, result, opnd);
    OPS_Append_Op(ops, intrncall_op);

  }
  else {
    //
    // generate real code
    // t = s1 > s2; d = select t ? s1 : s2
    //
    switch (mtype) {
      case MTYPE_I8: variant = V_BR_I8GT; break;
      case MTYPE_I4: variant = V_BR_I4GT; break;
      case MTYPE_U8: variant = V_BR_U8GT; break;
      case MTYPE_U4: variant = V_BR_U4GT; break;

      case MTYPE_F4: 
      case MTYPE_F8: 
      case MTYPE_F10: 
      default:
	FmtAssert(FALSE, ("Expand_Max: unexpected mtype"));
    }

    cmp = Pick_Compare_TOP (&variant, &src1, &src2, ops);
    Expand_Compare_And_Select (variant, src1, 
                          src2, dest, NULL, src1, src2, FALSE, ops);
  }

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
 *   Exp_ST100_Sqrt
 * ====================================================================
 */
static void
Expand_ST100_Sqrt (
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
  static void (*exp_sqrt)(TN *, TN *, TYPE_ID, OPS *) = Expand_ST100_Sqrt;

  FmtAssert(FALSE,("Not Implemented"));

  if (!initialized) {
    const char * const alg = CGEXP_sqrt_algorithm;
    if (strcasecmp(alg, "st100") == 0) {
      exp_sqrt = Expand_ST100_Sqrt;
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

  Expand_Copy (tgt_tn, True_TN, src_tn, ops);

#if 0
  TN *tmp;

  if (TN_is_constant(src_tn)) {
    FmtAssert (TN_has_value(src_tn), ("Exp_COPY: illegal source tn"));
  }

  ISA_REGISTER_CLASS tgt_rc = TN_register_class(tgt_tn);
  ISA_REGISTER_CLASS src_rc = TN_register_class(src_tn);

  if (src_rc == ISA_REGISTER_CLASS_du) {
    if (tgt_rc == ISA_REGISTER_CLASS_du) {
      Build_OP(TOP_GP32_MOVE_GT_DR_DR, tgt_tn, True_TN, src_tn, ops);
      return;
    }
    if (tgt_rc == ISA_REGISTER_CLASS_au) {
      Build_OP(TOP_GP32_COPYA_GT_AR_DR, tgt_tn, True_TN, src_tn, ops);
      return;
    }
  }

  if (src_rc == ISA_REGISTER_CLASS_au) {
    if (tgt_rc == ISA_REGISTER_CLASS_du) {
      Build_OP(TOP_GP32_COPYD_GT_DR_AR, tgt_tn, True_TN, src_tn, ops);
      return;
    }
    if (tgt_rc == ISA_REGISTER_CLASS_au) {
      Build_OP(TOP_GP32_MOVEA_GT_AR_AR, tgt_tn, True_TN, src_tn, ops);
      return;
    }
  }

  if (src_rc == ISA_REGISTER_CLASS_guard) {
    if (tgt_rc == ISA_REGISTER_CLASS_guard) {
      Build_OP(TOP_GP32_MOVEG_GT_BR_BR, tgt_tn, True_TN, src_tn, ops);
      return;
    }
    if (tgt_rc == ISA_REGISTER_CLASS_du) {
      Build_OP(TOP_GP32_BOOL_GT_DR_BR, tgt_tn, True_TN, src_tn, ops);
      return;
    }
  }

  FmtAssert(FALSE,
         ("Exp_COPY: unsupported copy for %s to %s",
	    ISA_REGISTER_CLASS_INFO_Name(ISA_REGISTER_CLASS_Info(src_rc)),
	    ISA_REGISTER_CLASS_INFO_Name(ISA_REGISTER_CLASS_Info(tgt_rc))));
#endif

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

  FmtAssert(0, ("Exp_Simulated_Op should not be reached"));

  switch (OP_code(op)) {

  case TOP_intrncall:
    Expand_TOP_intrncall(op, ops, FALSE, pc_value);
    break;

  case TOP_spadjust:
    // spadjust should only show up for alloca/dealloca
    if (OP_variant(op) == V_SPADJUST_PLUS) {
      // dealloca does copy of kid to $sp (op1 is old sp value)
      /*
      Expand_Copy (OP_result(op,0), OP_opnd(op,2), Pointer_Mtype, ops);
      */
      Expand_Add (OP_result(op,0), OP_opnd(op,1), 
		                    OP_opnd(op,2), Pointer_Mtype, ops);
    }
    else {
      Expand_Sub (OP_result(op,0), OP_opnd(op,1), 
                                    OP_opnd(op,2), Pointer_Mtype, ops);
    }

    FOR_ALL_OPS_OPs(ops, newop) {
      OP_srcpos(newop) = OP_srcpos(op);
      Is_True(OP_has_predicate(newop) == OP_has_predicate(op),
	      ("spadjust can't copy predicates"));
      // copy predicate to new copy/sub ops
      if (OP_has_predicate(newop))
	  Set_OP_opnd (newop, OP_PREDICATE_OPND, 
				      OP_opnd(op, OP_PREDICATE_OPND));
    }
    break;

  default:
    #pragma mips_frequency_hint NEVER
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
    	// For IA-64, we should never emit a simulated OP, so just assert.
	#pragma mips_frequency_hint NEVER
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
      !ISA_LC_Value_In_Class(base_ofst, LC_s16)) {
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
  Build_OP (TOP_spadjust, dest, True_TN, SP_TN, size, ops);
  OP_variant(OPS_last(ops)) = variant;
}


/* ====================================================================
 *   Exp_Noop
 * ====================================================================
 */
void
Exp_Noop (OPS *ops)
{
  Build_OP (TOP_noop, True_TN, Gen_Literal_TN(0, 4), ops);
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

  FmtAssert(FALSE,("Not Implemented"));


  if (Targ_Is_Zero(tc)) {
    // copy 0
    Build_OP (TOP_noop, dest, True_TN, FZero_TN, ops);
    return;
  }

  // load from memory
  Exp_Load (mtype, mtype, dest, TN_var(src), 0, ops, V_NONE);
}

