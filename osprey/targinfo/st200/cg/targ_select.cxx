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
 * $Revision$
 * $Date$
 * $Author$
 * $Source$
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
#include "whirl2ops.h"

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
  TN *tn;

  if (TN_register_class(btn) == ISA_REGISTER_CLASS_branch) {
    tn = Gen_Register_TN (ISA_REGISTER_CLASS_integer, Pointer_Size);
    if(TN_is_global_reg(btn)) {
      Build_OP(TOP_mfb, tn, btn, ops);
    }
    else {
      DevAssert(cmp, ("TOP_Branch_To_Reg\n"));
      TOP cmp_top = TOP_Branch_To_Reg (OP_code(cmp));
      DevAssert(cmp_top, ("TOP_Branch_To_Reg\n"));
      Build_OP (cmp_top, tn, OP_opnd(cmp, 0), OP_opnd(cmp, 1), ops);
      BB_Remove_Op(OP_bb(cmp), cmp);
    }
  }

  return tn;
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

  tns[0] = OP_opnd(op1, 0);
  tns[1] = OP_opnd(op1, 1);
  tns[2] = OP_opnd(op1, 2);

  if (invert) {
    true_tn = OP_opnd(op2, idx);
    false_tn = OP_opnd(op1, idx);
  }
  else {
    true_tn = OP_opnd(op1, idx);
    false_tn = OP_opnd(op2, idx);
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

  TN *val  = tns[OP_find_opnd_use(op1, OU_storeval)];
  TN *base = tns[OP_find_opnd_use(op1, OU_base)];
  TN *ofst = tns[OP_find_opnd_use(op1, OU_offset)];

  if (TN_is_register (ofst)) {
    Build_OP (TOP_add_r, base, base, ofst, ops);
    ofst =  Gen_Literal_TN (0, 4);
  }

  TYPE_ID ttype;
  switch (OP_code(op1)) {
  case TOP_stw_i:
  case TOP_stw_ii:
    ttype = MTYPE_I4;
    break;

  case TOP_sth_i:
  case TOP_sth_ii:
    ttype = MTYPE_I2;
    break;

  case TOP_stb_i:
  case TOP_stb_ii:
    ttype = MTYPE_I1;
    break;

  default:
    DevAssert(FALSE, ("stw"));    
  }

  Expand_Store (ttype, val, base, ofst, ops);
}
