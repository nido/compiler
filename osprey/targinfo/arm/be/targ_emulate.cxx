/*

  Copyright (C) 2005-2006 ST Microelectronics, Inc.  All Rights Reserved.

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

  Contact information:  ST Microelectronics, Inc., 
  address, or:

  http://www.st.com

*/


#include "defs.h"
#include "config.h"
#include "errors.h"
#include "erglob.h"
#include "tracing.h"
#include "data_layout.h"
#include "wn.h"
#include "wn_util.h"
#include "const.h"
#include "intrn_info.h"
#include "targ_emulate.h"


/* ============================================================
 *
 * Utilitary functions.
 * Contains code extracted from emulate.cxx.
 *
 * ============================================================ */

/*
 * Returns whether the opcode is an intrinsic op/call,
 */
#define OPCODE_is_intrinsic(op)                                         \
                ((OPCODE_operator((op)) == OPR_INTRINSIC_CALL) ||       \
                (OPCODE_operator((op)) == OPR_INTRINSIC_OP))

/*
 * WN_arg()
 *
 * return Nth kid , skiping PARM
 */
static WN *
WN_arg(WN *tree, INT32 arg)
{
  WN	*child= WN_kid(tree, arg);

  if (WN_operator_is(child, OPR_PARM))
  {
    return WN_kid0(child);
  }

  return child;
}


/* ============================================================
 *
 * Implementation of target specific intrinsics.
 *
 * ============================================================ */

/*
 * em_stxp70_va_start()
 *
 * STxP70 emulation of va_start intrinsic op.
 *
 * Implements va_start STxP70 ABI with or without support of floating point
 * registers.
 *
 * The va_list must be of the following form:
 *  struct {
 *  short reg_offset;    -> Byte offset in the R registers area [0..24]
 *  short stack_offset;  -> Number of bytes consumed on stack [0..[
 *   char * reg_base;      -> Base pointer to memory copy of register
 *                           arguments (r0-r5).
 *   char * stack_base;    -> Base pointer to stacked arguments
 *   short freg_offset;  -> Byte offset in the F registers area [0..16]
 *   char *freg_base;    -> Base pointer to memory copy of F register
 *                           arguments (f0-r3) if available.
 *  } __va_list;
 *
 * Argument of va_start is a pointer on the va_list.
 *
 */
#define REG_OFFSET_F 0
#define REG_BASE_F 4
#define STACK_OFFSET_F 2
#define STACK_BASE_F 8
#define FREG_OFFSET_F 12
#define FREG_BASE_F 16

static WN *
em_stxp70_va_start(WN *block, WN *ap)
{
  TY_IDX ty_idx;
  INT fixed_int_parms;
  INT field_offset = 0;
  INT fixed_float_parms;
  BOOL direct;
  BOOL non_leaf = FALSE;
#ifdef Is_True_On
  BOOL trace = Get_Trace(TP_DATALAYOUT, 1);
#endif

  if (WN_operator(ap) == OPR_LDA) {
    ty_idx = WN_ty(ap);
    Is_True(TY_kind(ty_idx) == KIND_POINTER,
	    ("em_stxp70_va_start: argument not of pointer type"));
    ty_idx = TY_pointed(ty_idx);
    Is_True(TY_kind(ty_idx) == KIND_ARRAY && TY_size(ty_idx) == 20,
	("em_stxp70_va_start: argument pointer does not point to type va_list"));
    direct = TRUE;
  }
  else if (WN_operator(ap) == OPR_LDID) {
    ty_idx = WN_ty(ap);
    Is_True(TY_kind(ty_idx) == KIND_POINTER,
	    ("em_stxp70_va_start: argument not of pointer type"));
    ty_idx = TY_pointed(ty_idx);
    Is_True(TY_size(ty_idx) == 20,
	("em_stxp70_va_start: argument pointer does not point to type va_list"));
    direct = FALSE;
  }
  else {
    non_leaf = TRUE;
    direct = FALSE;
  }

  ST *upformal;
  WN *wn = NULL;
  WN *addr;
  SF_VARARG_INFO vinfo;
  
  Get_Vararg_Save_Area_Info(&vinfo);

  if (wn != NULL) WN_INSERT_BlockLast(block, wn); /* Insert previous wn. */

  INT fixed_int_size;
  if (SF_VARARG_INFO_flags(&vinfo) & SF_VARARG_INFO_HAS_INT) {
    /* Initialize reg_offset field. */
    fixed_int_size = SF_VARARG_INFO_fixed_int_slots(&vinfo)*
      SF_VARARG_INFO_int_slot_size(&vinfo);
  } else {
    fixed_int_size = 24; /* Set to max. */
  }
  wn = WN_Intconst(MTYPE_I4, fixed_int_size);
#ifdef Is_True_On
  if (trace) 
    fprintf(TFile, "<lay> %s: reg_offset = %d\n", __FUNCTION__, fixed_int_size);
#endif
  if (direct) {
    wn = WN_Stid(MTYPE_I2, REG_OFFSET_F, WN_st(ap), MTYPE_To_TY(MTYPE_I2), wn);
  }
  else {
    if (! non_leaf) {
      addr = WN_Ldid(Pointer_Mtype, WN_offset(ap), WN_st(ap), WN_ty(ap));
    }
    else {
      addr = WN_COPY_Tree(ap); /* TODO make leaf! */
    }
    wn = WN_Istore(MTYPE_I2, REG_OFFSET_F, Make_Pointer_Type(MTYPE_To_TY(MTYPE_I2)),
		   addr, wn);
  }

  if (SF_VARARG_INFO_var_int_save_base(&vinfo) != NULL) {
    if (wn != NULL) WN_INSERT_BlockLast(block, wn); /* Insert previous wn. */
    ST *reg_base = SF_VARARG_INFO_var_int_save_base(&vinfo);
    INT reg_offset = - fixed_int_size + 
      SF_VARARG_INFO_var_int_save_offset(&vinfo);
    wn = WN_Lda(Pointer_Mtype, reg_offset, reg_base);      
#ifdef Is_True_On
    if (trace) 
      fprintf(TFile, "<lay> %s: reg_base = %s + %d\n", __FUNCTION__, ST_name(reg_base), reg_offset);
#endif
    if (direct) {
      wn = WN_Stid(Pointer_Mtype, REG_BASE_F, WN_st(ap), MTYPE_To_TY(Pointer_Mtype),wn);
    }
    else {
      if (! non_leaf) {
	addr = WN_Ldid(Pointer_Mtype, WN_offset(ap), WN_st(ap), WN_ty(ap));
      }
      else {
	addr = WN_COPY_Tree(ap);
      }
      wn = WN_Istore(Pointer_Mtype, REG_BASE_F, 
		     Make_Pointer_Type(MTYPE_To_TY(Pointer_Mtype)), addr, wn);
    }
  }
  
  if (wn != NULL) WN_INSERT_BlockLast(block, wn); /* Insert previous wn. */

  INT fixed_float_size;
  if (SF_VARARG_INFO_flags(&vinfo) & SF_VARARG_INFO_HAS_FLOAT) {
    /* Initialize floatreg_used field. */
    fixed_float_size = SF_VARARG_INFO_fixed_float_slots(&vinfo)*
      SF_VARARG_INFO_float_slot_size(&vinfo);
  } else {
    fixed_float_size = 16; /* Set to max. */
  }
  
  wn = WN_Intconst(MTYPE_I4, fixed_float_size);
#ifdef Is_True_On
  if (trace) 
    fprintf(TFile, "<lay> %s: freg_offset = %d\n", __FUNCTION__, fixed_float_size);
#endif
  if (direct) {
    wn = WN_Stid(MTYPE_I2, FREG_OFFSET_F, WN_st(ap), MTYPE_To_TY(MTYPE_I2), wn);
  }
  else {
    if (! non_leaf) {
      addr = WN_Ldid(Pointer_Mtype, WN_offset(ap), WN_st(ap), WN_ty(ap));
    }
    else {
      addr = WN_COPY_Tree(ap);
    }
    wn = WN_Istore(MTYPE_I2, FREG_OFFSET_F, Make_Pointer_Type(MTYPE_To_TY(MTYPE_I2)),
		   addr, wn);
  }

  if (SF_VARARG_INFO_var_float_save_base(&vinfo) != NULL) {
    if (wn != NULL) WN_INSERT_BlockLast(block, wn); /* Insert previous wn. */
    ST *float_base = SF_VARARG_INFO_var_float_save_base(&vinfo);
    INT float_offset = - fixed_float_size + 
      SF_VARARG_INFO_var_float_save_offset(&vinfo);
    wn = WN_Lda(Pointer_Mtype, float_offset, float_base);      
#ifdef Is_True_On
    if (trace) 
      fprintf(TFile, "<lay> %s: float_base = %s + %d\n", __FUNCTION__, ST_name(float_base), float_offset);
#endif
    if (direct) {
      wn = WN_Stid(Pointer_Mtype, FREG_BASE_F, WN_st(ap), MTYPE_To_TY(Pointer_Mtype),wn);
    }
    else {
      if (! non_leaf) {
	addr = WN_Ldid(Pointer_Mtype, WN_offset(ap), WN_st(ap), WN_ty(ap));
      }
      else {
	addr = WN_COPY_Tree(ap);
      }
      wn = WN_Istore(Pointer_Mtype, FREG_BASE_F, 
		     Make_Pointer_Type(MTYPE_To_TY(Pointer_Mtype)), addr, wn);
    }
  }
  
  if (wn != NULL) WN_INSERT_BlockLast(block, wn); /* Insert previous wn. */
  
  DevAssert(SF_VARARG_INFO_flags(&vinfo) & SF_VARARG_INFO_HAS_STACK, ("Expected to have stacked parameters for vararg info."));
  
  /* Initialize stack_offset. */
  /* Stack offset is initialized to the current var stack offset. */
  INT stack_offset = SF_VARARG_INFO_var_stack_offset(&vinfo);
  wn = WN_Intconst(MTYPE_I4, stack_offset);
#ifdef Is_True_On
  if (trace) 
    fprintf(TFile, "<lay> %s: stack_offset = %d\n", __FUNCTION__, stack_offset);
#endif
  if (direct) {
    wn = WN_Stid(MTYPE_I2, STACK_OFFSET_F, WN_st(ap), MTYPE_To_TY(MTYPE_I2), wn);
  }
  else {
    if (! non_leaf) {
      addr = WN_Ldid(Pointer_Mtype, WN_offset(ap), WN_st(ap), WN_ty(ap));
    }
    else {
      addr = WN_COPY_Tree(ap);
    }
    wn = WN_Istore(MTYPE_I2, STACK_OFFSET_F, Make_Pointer_Type(MTYPE_To_TY(MTYPE_I2)),
		   addr, wn);
  }
  
  if (wn != NULL) WN_INSERT_BlockLast(block, wn); /* Insert previous wn. */
  
  /* Set the class to formal if there is no fix parameter in up formal
     area that will initialize upformal block with their classes ( normally
     SCLASS_FORMAL). */
  if (ST_sclass(SF_VARARG_INFO_var_stack_base(&vinfo)) == SCLASS_UNKNOWN)
    Set_ST_sclass (SF_VARARG_INFO_var_stack_base(&vinfo), SCLASS_FORMAL);
  
  ST *stack_base = SF_VARARG_INFO_var_stack_base(&vinfo);
  INT stack_st_offset = 0;
  /* Points to the stack base. */
  wn = WN_Lda(Pointer_Mtype, stack_st_offset, stack_base);
#ifdef Is_True_On
  if (trace) 
    fprintf(TFile, "<lay> %s: stack_base = %s + %d\n", __FUNCTION__, ST_name(stack_base), stack_st_offset);
#endif
  if (direct) {
    wn = WN_Stid(Pointer_Mtype, STACK_BASE_F, WN_st(ap), MTYPE_To_TY(Pointer_Mtype), wn);
  }
  else {
    if (! non_leaf) {
      addr = WN_Ldid(Pointer_Mtype, WN_offset(ap), WN_st(ap), WN_ty(ap));
    }
    else {
      addr = WN_COPY_Tree(ap);
    }
    wn = WN_Istore(Pointer_Mtype, STACK_BASE_F, 
		   Make_Pointer_Type(MTYPE_To_TY(Pointer_Mtype)), addr, wn);
  }
  
 
  return wn;
}


/* ============================================================
 *
 * Exported interface.
 *
 * ============================================================ */

/*
 * BETARG_emulate_intrinsic_op()
 *
 * See Interface Description.
 */
WN *
BETARG_emulate_intrinsic_op(WN *block, WN *tree)
{
  INTRINSIC     id = (INTRINSIC) WN_intrinsic(tree);
  TYPE_ID	rtype = WN_rtype(tree);
  WN		*function;

  DevAssert((OPCODE_is_intrinsic(WN_opcode(tree))),
	    ("expected intrinsic call node, not %s", 
	     OPCODE_name(WN_opcode(tree))));
  DevAssert((INTRN_is_actual(WN_intrinsic(tree))==FALSE),
	    ("cannot emulate INTRN_is_actual"));
  
  switch(id) {
  case INTRN_VA_START:
    return em_stxp70_va_start(block, WN_arg(tree,0));
    break;
  default:
    break;
  }
  return NULL;
}
