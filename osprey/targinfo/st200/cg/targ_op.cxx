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
 *   CGTARG_Copy_Operand
 *
 *   TODO: generate automatically ?? at leats some obvious ones
 *         coherently with the isa property ?
 * ====================================================================
 */
INT 
CGTARG_Copy_Operand (
  OP *op
)
{
  TOP opr = OP_code(op);

  switch (opr) {
#if 0
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
#endif

  case TOP_mov_r:
  case TOP_mov_i:
    return 0;

  }

  return -1;
}

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

