/*
  Copyright (C) 2002, STMicroelectronics, All Rights Reserved.

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
*/

/* ====================================================================
 * ====================================================================
 *
 * Module: targ_select.cxx
 *
 * Description:
 *
 * Select target-specific optimizations.
 *
 * ====================================================================
 * ====================================================================
 */

#include "cgir.h"
#include "cgexp.h"
#include "data_layout.h"
#include "stblock.h"
#include "whirl2ops.h"
#include "tn.h"

/* --------------------------------------------------------------------
 *    Return instruction that put the result in register class <dst2> 
 *    instead of register class <dst1>
 * --------------------------------------------------------------------
 */
static TOP TOP_Branch_To_Reg_Table[TOP_count+1];

static void
Init_TOP_Branch_To_Reg()
{
  int i;

  for(i = 0; i <= TOP_count; ++i) {
    TOP_Branch_To_Reg_Table[i] = TOP_UNDEFINED;
  }

  TOP_Branch_To_Reg_Table[TOP_cmpeq_r_b]   = TOP_cmpeq_r_r;
  TOP_Branch_To_Reg_Table[TOP_cmpeq_i_b]   = TOP_cmpeq_i_r;
  TOP_Branch_To_Reg_Table[TOP_cmpeq_ii_b]  = TOP_cmpeq_ii_r;
  TOP_Branch_To_Reg_Table[TOP_cmpge_r_b]   = TOP_cmpge_r_r;
  TOP_Branch_To_Reg_Table[TOP_cmpge_i_b]   = TOP_cmpge_i_r;
  TOP_Branch_To_Reg_Table[TOP_cmpge_ii_b]  = TOP_cmpge_ii_r;
  TOP_Branch_To_Reg_Table[TOP_cmpgeu_r_b]  = TOP_cmpgeu_r_r;
  TOP_Branch_To_Reg_Table[TOP_cmpgeu_i_b]  = TOP_cmpgeu_i_r;
  TOP_Branch_To_Reg_Table[TOP_cmpgeu_ii_b] = TOP_cmpgeu_ii_r;
  TOP_Branch_To_Reg_Table[TOP_cmpgt_r_b]   = TOP_cmpgt_r_r;
  TOP_Branch_To_Reg_Table[TOP_cmpgt_i_b]   = TOP_cmpgt_i_r;
  TOP_Branch_To_Reg_Table[TOP_cmpgt_ii_b]  = TOP_cmpgt_ii_r;
  TOP_Branch_To_Reg_Table[TOP_cmpgtu_r_b]  = TOP_cmpgtu_r_r;
  TOP_Branch_To_Reg_Table[TOP_cmpgtu_i_b]  = TOP_cmpgtu_i_r;
  TOP_Branch_To_Reg_Table[TOP_cmpgtu_ii_b] = TOP_cmpgtu_ii_r;
  TOP_Branch_To_Reg_Table[TOP_cmple_r_b]   = TOP_cmple_r_r;
  TOP_Branch_To_Reg_Table[TOP_cmple_i_b]   = TOP_cmple_i_r;
  TOP_Branch_To_Reg_Table[TOP_cmple_ii_b]  = TOP_cmple_ii_r;
  TOP_Branch_To_Reg_Table[TOP_cmpleu_r_b]  = TOP_cmpleu_r_r;
  TOP_Branch_To_Reg_Table[TOP_cmpleu_i_b]  = TOP_cmpleu_i_r;
  TOP_Branch_To_Reg_Table[TOP_cmpleu_ii_b] = TOP_cmpleu_ii_r;
  TOP_Branch_To_Reg_Table[TOP_cmplt_r_b]   = TOP_cmplt_r_r;
  TOP_Branch_To_Reg_Table[TOP_cmplt_i_b]   = TOP_cmplt_i_r;
  TOP_Branch_To_Reg_Table[TOP_cmplt_ii_b]  = TOP_cmplt_ii_r;
  TOP_Branch_To_Reg_Table[TOP_cmpltu_r_b]  = TOP_cmpltu_r_r;
  TOP_Branch_To_Reg_Table[TOP_cmpltu_i_b]  = TOP_cmpltu_i_r;
  TOP_Branch_To_Reg_Table[TOP_cmpltu_ii_b] = TOP_cmpltu_ii_r;
  TOP_Branch_To_Reg_Table[TOP_cmpne_r_b]   = TOP_cmpne_r_r;
  TOP_Branch_To_Reg_Table[TOP_cmpne_i_b]   = TOP_cmpne_i_r;
  TOP_Branch_To_Reg_Table[TOP_cmpne_ii_b]  = TOP_cmpne_ii_r;
  TOP_Branch_To_Reg_Table[TOP_cmpne_ii_b]  = TOP_cmpne_ii_r;
  TOP_Branch_To_Reg_Table[TOP_orl_r_b]     = TOP_orl_r_r;
  TOP_Branch_To_Reg_Table[TOP_orl_i_b]     = TOP_orl_i_r;
  TOP_Branch_To_Reg_Table[TOP_orl_ii_b]    = TOP_orl_ii_r;
  TOP_Branch_To_Reg_Table[TOP_norl_r_b]    = TOP_norl_r_r;
  TOP_Branch_To_Reg_Table[TOP_norl_i_b]    = TOP_norl_i_r;
  TOP_Branch_To_Reg_Table[TOP_norl_ii_b]   = TOP_norl_ii_r;
  TOP_Branch_To_Reg_Table[TOP_andl_r_b]    = TOP_andl_r_r;
  TOP_Branch_To_Reg_Table[TOP_andl_i_b]    = TOP_andl_i_r;
  TOP_Branch_To_Reg_Table[TOP_andl_ii_b]   = TOP_andl_ii_r;
  TOP_Branch_To_Reg_Table[TOP_nandl_r_b]   = TOP_nandl_r_r;
  TOP_Branch_To_Reg_Table[TOP_nandl_i_b]   = TOP_nandl_i_r;
  TOP_Branch_To_Reg_Table[TOP_nandl_ii_b]  = TOP_nandl_ii_r;
}

static inline 
TOP TOP_Branch_To_Reg(TOP opr)
{
  static BOOL did_init = FALSE;
  if ( ! did_init ) {
    Init_TOP_Branch_To_Reg();
    did_init = TRUE;
  }

  return TOP_Branch_To_Reg_Table[(INT)opr];
}

/* --------------------------------------------------------------------
 *    Make sure the compare instruction returns in a general register.
 * --------------------------------------------------------------------
 */
TN *
Expand_CMP_Reg (TN *btn, OP *cmp, OPS *ops)
{
  TN *tn = btn;

#if 0
  // [CG]: This ones works and EBO fold it. 
  // To be checked, does it impact if-conc heuristics.
  if (TN_register_class(btn) == ISA_REGISTER_CLASS_branch) {
    tn = Gen_Register_TN (ISA_REGISTER_CLASS_integer, Pointer_Size);
    Build_OP(TOP_mfb, tn, btn, ops);
  }
#else
  if (TN_register_class(btn) == ISA_REGISTER_CLASS_branch) {
    tn = Gen_Register_TN (ISA_REGISTER_CLASS_integer, Pointer_Size);
    if(TN_is_global_reg(btn)) {
      Build_OP(TOP_mfb, tn, btn, ops);
    }
    else {
      OP *op = cmp;
      while (op = OP_next (op)) {
        // if the btn is only used in the conditional branch, can change
        // cmp return value type without the need od a mfb.
        if (OP_cond (op))
          break;
        for (INT opndnum = 0; opndnum < OP_opnds(op); opndnum++) {
          TN *res = OP_opnd(op, opndnum);
          if (res == btn) {
            Build_OP(TOP_mfb, tn, btn, ops);
            return tn;
          }
        }
      }

      // the tn is not in use in the block. Can just replace the
      // compare instruction
      // This duplicates with some of the EBO work. keep it for now.
      // could to the "mfb" above everytime and let EBO clean it up.
      DevAssert(cmp, ("TOP_Branch_To_Reg\n"));
      TOP cmp_top = TOP_Branch_To_Reg (OP_code(cmp));
      if (cmp_top == TOP_UNDEFINED) {
	Build_OP(TOP_mfb, tn, btn, ops);
	return tn;
      }
      Build_OP (cmp_top, tn, OP_opnd(cmp, 0), OP_opnd(cmp, 1), ops);
      BB_Remove_Op(OP_bb(cmp), cmp);
    }
  }
#endif

  return tn;
}

// We need a store location to allow the if conversion of conditional stores.
// This location is actually a global common. but we could allocate it on the
// stack frame instead. define LOCAL_SELECT_STOREDUMP to allow this address
// to be generated from the frame.
// Create a common symbol to speculate the address while if converting store.

// #define COMMON_SELECT_STOREDUMP

#define LOCAL_SELECT_STOREDUMP

static ST *blackhole;

#ifdef  COMMON_SELECT_STOREDUMP
static ST *
Gen_Common_Symbol (TY_IDX      ty,	// type of the desired symbol
		 const char *rootname)	// root of the name to use
{
  ST *st = blackhole;

  if (!st) {
    st = New_ST(GLOBAL_SYMTAB);
    STR_IDX str_idx = Save_Str(rootname);
    ST_Init(st, str_idx, CLASS_VAR, SCLASS_COMMON, EXPORT_INTERNAL, ty);
    Allocate_Object (st);
  }
  return st;
}
#endif
#ifdef LOCAL_SELECT_STOREDUMP
static ST *latest_pu;

static ST *
Gen_Local_Symbol (TY_IDX      ty,	// type of the desired symbol
		 const char *rootname)	// root of the name to use
{
  ST *st = blackhole;

  // Only need to reinitialize for each new PU.
  if (latest_pu != Get_Current_PU_ST()) {
    latest_pu = Get_Current_PU_ST();
    st = Gen_Temp_Symbol(ty, "__store_dump_area");
    Allocate_Temp_To_Memory(st);
  }
  return st;
}
#endif

void
Expand_BlackHole (
  OP *store_op,
  TN *blackhole_tn,
  OPS *ops
  )
{
  TY_IDX ty = MTYPE_To_TY(Pointer_Mtype);

#ifdef COMMON_SELECT_STOREDUMP
  blackhole = Gen_Common_Symbol(ty, "__store_dump_area");
  TN *tmp = Gen_Symbol_TN(blackhole, 0, 0);
  Build_OP(TOP_mov_ii, blackhole_tn, tmp, ops);
#else
#ifdef LOCAL_SELECT_STOREDUMP
  blackhole = Gen_Local_Symbol (ty, "__store_dump_area");

#else
#error SELECT_STOREDUMP
#endif
#endif

  TN *offsetTN = OP_opnd(store_op, OP_find_opnd_use(store_op, OU_offset));
  INT32 offset = 0;

  if (TN_is_constant (offsetTN) && TN_has_value (offsetTN))
    offset = -TN_value(offsetTN);

  Exp_Lda (Pointer_type, blackhole_tn, blackhole, offset, OPERATOR_UNKNOWN, ops);
}

void
Expand_CondStoreAddr (
  OP *store_op,
  TN *storeaddr_tn,
  OPS *ops
  )
{
  TN *offsetTN = OP_opnd(store_op, OP_find_opnd_use(store_op, OU_offset));

  if (TN_is_constant (offsetTN) && TN_has_value (offsetTN))
    offsetTN = Zero_TN;

  Expand_Add (storeaddr_tn, OP_opnd(store_op, OP_find_opnd_use(store_op, OU_base)), offsetTN, Pointer_Mtype, ops);
}

void
Expand_CondStoreOP (
  OP *store_op,
  TN *storeaddr_tn,
  OPS *ops
  )
{
  TN *base = storeaddr_tn;
  TN *offset = OP_opnd(store_op, OP_find_opnd_use(store_op, OU_offset));
  TN *val  = OP_opnd(store_op, OP_find_opnd_use(store_op, OU_storeval));
  TYPE_ID desc;

  if (!(TN_is_constant (offset) && TN_has_value (offset)))
    offset = Gen_Literal_TN (0, 4);

  switch (OP_code(store_op)) {
  case TOP_stw_i:
  case TOP_stw_ii:
    desc = MTYPE_I4;
    break;

  case TOP_sth_i:
  case TOP_sth_ii:
    desc = MTYPE_I2;
    break;

  case TOP_stb_i:
  case TOP_stb_ii:
    desc = MTYPE_I1;
    break;

  default:
    DevAssert(FALSE, ("stw"));    
  }

  Expand_Store (desc, val, base, offset, ops);
  Copy_WN_For_Memory_OP(OPS_last(ops), store_op);
  Set_OP_black_hole(OPS_last(ops));
}

/* --------------------------------------------------------------------
 *    One of the operands is the result of a select instruction.
 *    If it is the ofst, then must generate a correct store format.
 * --------------------------------------------------------------------
 */
void
Expand_Cond_Store (
  TN   *cond_tn,
  BOOL invert,
  OP   *op1,
  OP   *op2,
  UINT8 idx,
  OPS   *ops
  )
{
  TN *tns[3];
  TN *true_tn, *false_tn;
  BOOL is_black_hole = FALSE;

  if (!op1) {
    op1 = op2;
    op2 = NULL;
    invert = !invert;
  }
  
  UINT8 validx    = OP_find_opnd_use(op1, OU_storeval);
  UINT8 baseidx   = OP_find_opnd_use(op1, OU_base);
  UINT8 offsetidx = OP_find_opnd_use(op1, OU_offset);

  tns[validx]    = OP_opnd(op1, validx);
  tns[baseidx]   = OP_opnd(op1, baseidx);
  tns[offsetidx] = OP_opnd(op1, offsetidx);

  if (!op2) {
    is_black_hole = TRUE;
    TY_IDX ty = MTYPE_To_TY(Pointer_Mtype);
    false_tn = Gen_Register_TN (ISA_REGISTER_CLASS_integer, Pointer_Size);

#ifdef COMMON_SELECT_STOREDUMP
    blackhole = Gen_Common_Symbol(ty, "__store_dump_area");
    TN *tmp = Gen_Symbol_TN(blackhole, 0, 0);
    Build_OP(TOP_mov_ii, false_tn, tmp, ops);
#else
#ifdef LOCAL_SELECT_STOREDUMP
    blackhole = Gen_Local_Symbol (ty, "__store_dump_area");

#else
#error SELECT_STOREDUMP
#endif
#endif

    if (TN_is_constant (tns[offsetidx]) &&
        TN_has_value (tns[offsetidx])) {
      INT32 offset = TN_value(tns[offsetidx]);
      Exp_Lda (Pointer_type, false_tn, blackhole, -offset, OPERATOR_UNKNOWN, ops);
      true_tn = OP_opnd(op1, baseidx);
      tns[offsetidx] = Gen_Literal_TN (offset, 4);
    }
    else {
      true_tn = Build_RCLASS_TN(ISA_REGISTER_CLASS_integer);
      Exp_Lda (Pointer_type, false_tn, blackhole, 0, OPERATOR_UNKNOWN, ops);
      Expand_Add (true_tn, tns[baseidx], tns[offsetidx], Pointer_Mtype, ops);
      tns[offsetidx] = Gen_Literal_TN (0, 4);
    }
  }
  else {
    false_tn = OP_opnd(op2, idx);
    true_tn = OP_opnd(op1, idx);
  }

  if (invert) {
    TN *tmp = false_tn;
    false_tn = true_tn;
    true_tn = tmp;
  }

  TN *temp_tn;
  if (TN_is_register(true_tn)) 
    temp_tn = Build_TN_Like(true_tn);
  else if (TN_is_register(false_tn)) 
    temp_tn = Build_TN_Like(false_tn);
  else 
    temp_tn = Build_TN_Of_Mtype(MTYPE_I4);

  tns[idx] = temp_tn;

  Expand_Select (temp_tn, cond_tn, true_tn, false_tn, MTYPE_I4, FALSE, ops);

  TN *val  = tns[validx];
  TN *base = tns[baseidx];
  TN *ofst = tns[offsetidx];

  if (TN_is_register (ofst)) {
    Build_OP (TOP_add_r, base, base, ofst, ops);
    ofst =  Gen_Literal_TN (0, 4);
  }

  TYPE_ID desc;

#if 0
  WN *wn1 = Get_WN_From_Memory_OP(op1);

  if (wn1) {
    OPCODE opcode = WN_opcode (wn1);
    desc = OPCODE_desc(opcode);
  }
  else {
#endif
    switch (OP_code(op1)) {
    case TOP_stw_i:
    case TOP_stw_ii:
      desc = MTYPE_I4;
      break;

    case TOP_sth_i:
    case TOP_sth_ii:
      desc = MTYPE_I2;
      break;

    case TOP_stb_i:
    case TOP_stb_ii:
      desc = MTYPE_I1;
      break;

    default:
      DevAssert(FALSE, ("stw"));    
    }
#if 0
  }
#endif

  Expand_Store (desc, val, base, ofst, ops);
  if (is_black_hole) Set_OP_black_hole(OPS_last(ops));
}
