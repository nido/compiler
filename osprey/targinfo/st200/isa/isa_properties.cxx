/* -- This file is automatically generated -- */ 
/* 

  Copyright (C) 2002 ST Microelectronics, Inc.  All Rights Reserved. 

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
  , or: 

  http://www.st.com 

  For further information regarding this notice, see: 

  http: 
*/ 

//  AUTOMATICALLY GENERATED FROM CHESS DATABASE !!!

//  Generate ISA properties information 
/////////////////////////////////////// 
#include <stddef.h> 
#include "topcode.h" 
#include "isa_properties_gen.h" 

main() 
{ 
  ISA_PROPERTY 
    noop,
    load,
    store,
    jump,
    ijump,
    cond,
    call,
    simulated,
    dummy,
    var_opnds,
    dismissible,
    xfer,
    move,
    prefetch,
    mul,
    sub,
    add,
    xor,
    and,
    or,
    shl,
    shr,
    shru,
    zext,
    sext,
    intop,
    select,
    unsign,
    cmp,
    l_group,
    f_group,
    flop,
    madd,
    mem_fill_type,
    guard_t,
    guard_f,
    div,
    unsafe,
    likely,
    unalign,
    access_reg_bank,
    side_effects,
    branch_predict,
    refs_fcr,
    defs_fcc,
    defs_fcr;

  ISA_Properties_Begin ("st200"); 


  /* ==================================================================== 
   *              Operator attributes descriptors 
   * ==================================================================== 
   */ 

  /* ====================================== */ 
  noop = ISA_Property_Create ("noop"); 
  Instruction_Group (noop, 
		 TOP_nop, 
		 TOP_noop, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  intop = ISA_Property_Create ("intop"); 
  Instruction_Group (intop, 
		 TOP_add_r, 
		 TOP_add_i, 
		 TOP_add_ii, 
		 TOP_and_r, 
		 TOP_and_i, 
		 TOP_and_ii, 
		 TOP_andc_r, 
		 TOP_andc_i, 
		 TOP_andc_ii, 
		 TOP_andl_r_r, 
		 TOP_andl_r_b, 
		 TOP_andl_i_r, 
		 TOP_andl_ii_r, 
		 TOP_andl_i_b, 
		 TOP_andl_ii_b, 
		 TOP_bswap_r, 
		 TOP_clz_r, 
		 TOP_cmpeq_r_r, 
		 TOP_cmpeq_r_b, 
		 TOP_cmpeq_i_r, 
		 TOP_cmpeq_ii_r, 
		 TOP_cmpeq_i_b, 
		 TOP_cmpeq_ii_b, 
		 TOP_cmpge_r_r, 
		 TOP_cmpge_r_b, 
		 TOP_cmpge_i_r, 
		 TOP_cmpge_ii_r, 
		 TOP_cmpge_i_b, 
		 TOP_cmpge_ii_b, 
		 TOP_cmpgeu_r_r, 
		 TOP_cmpgeu_r_b, 
		 TOP_cmpgeu_i_r, 
		 TOP_cmpgeu_ii_r, 
		 TOP_cmpgeu_i_b, 
		 TOP_cmpgeu_ii_b, 
		 TOP_cmpgt_r_r, 
		 TOP_cmpgt_r_b, 
		 TOP_cmpgt_i_r, 
		 TOP_cmpgt_ii_r, 
		 TOP_cmpgt_i_b, 
		 TOP_cmpgt_ii_b, 
		 TOP_cmpgtu_r_r, 
		 TOP_cmpgtu_r_b, 
		 TOP_cmpgtu_i_r, 
		 TOP_cmpgtu_ii_r, 
		 TOP_cmpgtu_i_b, 
		 TOP_cmpgtu_ii_b, 
		 TOP_cmple_r_r, 
		 TOP_cmple_r_b, 
		 TOP_cmple_i_r, 
		 TOP_cmple_ii_r, 
		 TOP_cmple_i_b, 
		 TOP_cmple_ii_b, 
		 TOP_cmpleu_r_r, 
		 TOP_cmpleu_r_b, 
		 TOP_cmpleu_i_r, 
		 TOP_cmpleu_ii_r, 
		 TOP_cmpleu_i_b, 
		 TOP_cmpleu_ii_b, 
		 TOP_cmplt_r_r, 
		 TOP_cmplt_r_b, 
		 TOP_cmplt_i_r, 
		 TOP_cmplt_ii_r, 
		 TOP_cmplt_i_b, 
		 TOP_cmplt_ii_b, 
		 TOP_cmpltu_r_r, 
		 TOP_cmpltu_r_b, 
		 TOP_cmpltu_i_r, 
		 TOP_cmpltu_ii_r, 
		 TOP_cmpltu_i_b, 
		 TOP_cmpltu_ii_b, 
		 TOP_cmpne_r_r, 
		 TOP_cmpne_r_b, 
		 TOP_cmpne_i_r, 
		 TOP_cmpne_ii_r, 
		 TOP_cmpne_i_b, 
		 TOP_cmpne_ii_b, 
		 TOP_max_r, 
		 TOP_max_i, 
		 TOP_max_ii, 
		 TOP_maxu_r, 
		 TOP_maxu_i, 
		 TOP_maxu_ii, 
		 TOP_min_r, 
		 TOP_min_i, 
		 TOP_min_ii, 
		 TOP_minu_r, 
		 TOP_minu_i, 
		 TOP_minu_ii, 
		 TOP_mulh_r, 
		 TOP_mulh_i, 
		 TOP_mulh_ii, 
		 TOP_mulhh_r, 
		 TOP_mulhh_i, 
		 TOP_mulhh_ii, 
		 TOP_mulhhs_r, 
		 TOP_mulhhs_i, 
		 TOP_mulhhs_ii, 
		 TOP_mulhhu_r, 
		 TOP_mulhhu_i, 
		 TOP_mulhhu_ii, 
		 TOP_mulhs_r, 
		 TOP_mulhs_i, 
		 TOP_mulhs_ii, 
		 TOP_mulhu_r, 
		 TOP_mulhu_i, 
		 TOP_mulhu_ii, 
		 TOP_mull_r, 
		 TOP_mull_i, 
		 TOP_mull_ii, 
		 TOP_mullh_r, 
		 TOP_mullh_i, 
		 TOP_mullh_ii, 
		 TOP_mullhu_r, 
		 TOP_mullhu_i, 
		 TOP_mullhu_ii, 
		 TOP_mullhus_r, 
		 TOP_mullhus_i, 
		 TOP_mullhus_ii, 
		 TOP_mulll_r, 
		 TOP_mulll_i, 
		 TOP_mulll_ii, 
		 TOP_mulllu_r, 
		 TOP_mulllu_i, 
		 TOP_mulllu_ii, 
		 TOP_mullu_r, 
		 TOP_mullu_i, 
		 TOP_mullu_ii, 
		 TOP_nandl_r_r, 
		 TOP_nandl_r_b, 
		 TOP_nandl_i_r, 
		 TOP_nandl_ii_r, 
		 TOP_nandl_i_b, 
		 TOP_nandl_ii_b, 
		 TOP_norl_r_r, 
		 TOP_norl_r_b, 
		 TOP_norl_i_r, 
		 TOP_norl_ii_r, 
		 TOP_norl_i_b, 
		 TOP_norl_ii_b, 
		 TOP_or_r, 
		 TOP_or_i, 
		 TOP_or_ii, 
		 TOP_orc_r, 
		 TOP_orc_i, 
		 TOP_orc_ii, 
		 TOP_orl_r_r, 
		 TOP_orl_r_b, 
		 TOP_orl_i_r, 
		 TOP_orl_ii_r, 
		 TOP_orl_i_b, 
		 TOP_orl_ii_b, 
		 TOP_sh1add_r, 
		 TOP_sh1add_i, 
		 TOP_sh1add_ii, 
		 TOP_sh2add_r, 
		 TOP_sh2add_i, 
		 TOP_sh2add_ii, 
		 TOP_sh3add_r, 
		 TOP_sh3add_i, 
		 TOP_sh3add_ii, 
		 TOP_sh4add_r, 
		 TOP_sh4add_i, 
		 TOP_sh4add_ii, 
		 TOP_shl_r, 
		 TOP_shl_i, 
		 TOP_shl_ii, 
		 TOP_shr_r, 
		 TOP_shr_i, 
		 TOP_shr_ii, 
		 TOP_shru_r, 
		 TOP_shru_i, 
		 TOP_shru_ii, 
		 TOP_slct_r, 
		 TOP_slct_i, 
		 TOP_slct_ii, 
		 TOP_slctf_r, 
		 TOP_slctf_i, 
		 TOP_slctf_ii, 
		 TOP_sub_r, 
		 TOP_sub_i, 
		 TOP_sub_ii, 
		 TOP_sxtb_r, 
		 TOP_sxth_r, 
		 TOP_xor_r, 
		 TOP_xor_i, 
		 TOP_xor_ii, 
		 TOP_zxth_r, 
		 TOP_mov_r, 
		 TOP_mov_i, 
		 TOP_mov_ii, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  xor = ISA_Property_Create ("xor"); 
  Instruction_Group (xor, 
		 TOP_xor_r, 
		 TOP_xor_i, 
		 TOP_xor_ii, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  shr = ISA_Property_Create ("shr"); 
  Instruction_Group (shr, 
		 TOP_shr_r, 
		 TOP_shr_i, 
		 TOP_shr_ii, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  cond = ISA_Property_Create ("cond"); 
  Instruction_Group (cond, 
		 TOP_br, 
		 TOP_brf, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  mul = ISA_Property_Create ("mul"); 
  Instruction_Group (mul, 
		 TOP_mull_r, 
		 TOP_mull_i, 
		 TOP_mull_ii, 
		 TOP_mulll_r, 
		 TOP_mulll_i, 
		 TOP_mulll_ii, 
		 TOP_mulllu_r, 
		 TOP_mulllu_i, 
		 TOP_mulllu_ii, 
		 TOP_mullu_r, 
		 TOP_mullu_i, 
		 TOP_mullu_ii, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  zext = ISA_Property_Create ("zext"); 
  Instruction_Group (zext, 
		 TOP_zxth_r, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  shru = ISA_Property_Create ("shru"); 
  Instruction_Group (shru, 
		 TOP_shru_r, 
		 TOP_shru_i, 
		 TOP_shru_ii, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  and = ISA_Property_Create ("and"); 
  Instruction_Group (and, 
		 TOP_and_r, 
		 TOP_and_i, 
		 TOP_and_ii, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  sext = ISA_Property_Create ("sext"); 
  Instruction_Group (sext, 
		 TOP_sxtb_r, 
		 TOP_sxth_r, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  sub = ISA_Property_Create ("sub"); 
  Instruction_Group (sub, 
		 TOP_sub_r, 
		 TOP_sub_i, 
		 TOP_sub_ii, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  ijump = ISA_Property_Create ("ijump"); 
  Instruction_Group (ijump, 
		 TOP_igoto, 
		 TOP_return, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  load = ISA_Property_Create ("load"); 
  Instruction_Group (load, 
		 TOP_ldb_i, 
		 TOP_ldb_ii, 
		 TOP_ldb_d_i, 
		 TOP_ldb_d_ii, 
		 TOP_ldbu_i, 
		 TOP_ldbu_ii, 
		 TOP_ldbu_d_i, 
		 TOP_ldbu_d_ii, 
		 TOP_ldh_i, 
		 TOP_ldh_ii, 
		 TOP_ldh_d_i, 
		 TOP_ldh_d_ii, 
		 TOP_ldhu_i, 
		 TOP_ldhu_ii, 
		 TOP_ldhu_d_i, 
		 TOP_ldhu_d_ii, 
		 TOP_ldw_i, 
		 TOP_ldw_ii, 
		 TOP_ldw_d_i, 
		 TOP_ldw_d_ii, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  prefetch = ISA_Property_Create ("prefetch"); 
  Instruction_Group (prefetch, 
		 TOP_pft_i, 
		 TOP_pft_ii, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  dummy = ISA_Property_Create ("dummy"); 
  Instruction_Group (dummy, 
		 TOP_begin_pregtn, 
		 TOP_end_pregtn, 
		 TOP_bwd_bar, 
		 TOP_fwd_bar, 
		 TOP_dfixup, 
		 TOP_ffixup, 
		 TOP_ifixup, 
		 TOP_label, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  var_opnds = ISA_Property_Create ("var_opnds"); 
  Instruction_Group (var_opnds, 
		 TOP_asm, 
		 TOP_intrncall, 
		 TOP_phi, 
		 TOP_psi, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  simulated = ISA_Property_Create ("simulated"); 
  Instruction_Group (simulated, 
		 TOP_asm, 
		 TOP_intrncall, 
		 TOP_spadjust, 
		 TOP_noop, 
		 TOP_phi, 
		 TOP_psi, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  dismissible = ISA_Property_Create ("dismissible"); 
  Instruction_Group (dismissible, 
		 TOP_ldb_d_i, 
		 TOP_ldb_d_ii, 
		 TOP_ldbu_d_i, 
		 TOP_ldbu_d_ii, 
		 TOP_ldh_d_i, 
		 TOP_ldh_d_ii, 
		 TOP_ldhu_d_i, 
		 TOP_ldhu_d_ii, 
		 TOP_ldw_d_i, 
		 TOP_ldw_d_ii, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  store = ISA_Property_Create ("store"); 
  Instruction_Group (store, 
		 TOP_stb_i, 
		 TOP_stb_ii, 
		 TOP_sth_i, 
		 TOP_sth_ii, 
		 TOP_stw_i, 
		 TOP_stw_ii, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  or = ISA_Property_Create ("or"); 
  Instruction_Group (or, 
		 TOP_or_r, 
		 TOP_or_i, 
		 TOP_or_ii, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  select = ISA_Property_Create ("select"); 
  Instruction_Group (select, 
		 TOP_slct_r, 
		 TOP_slct_i, 
		 TOP_slct_ii, 
		 TOP_slctf_r, 
		 TOP_slctf_i, 
		 TOP_slctf_ii, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  add = ISA_Property_Create ("add"); 
  Instruction_Group (add, 
		 TOP_add_r, 
		 TOP_add_i, 
		 TOP_add_ii, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  unsign = ISA_Property_Create ("unsign"); 
  Instruction_Group (unsign, 
		 TOP_cmpgeu_r_r, 
		 TOP_cmpgeu_r_b, 
		 TOP_cmpgeu_i_r, 
		 TOP_cmpgeu_ii_r, 
		 TOP_cmpgeu_i_b, 
		 TOP_cmpgeu_ii_b, 
		 TOP_cmpgtu_r_r, 
		 TOP_cmpgtu_r_b, 
		 TOP_cmpgtu_i_r, 
		 TOP_cmpgtu_ii_r, 
		 TOP_cmpgtu_i_b, 
		 TOP_cmpgtu_ii_b, 
		 TOP_cmpleu_r_r, 
		 TOP_cmpleu_r_b, 
		 TOP_cmpleu_i_r, 
		 TOP_cmpleu_ii_r, 
		 TOP_cmpleu_i_b, 
		 TOP_cmpleu_ii_b, 
		 TOP_cmpltu_r_r, 
		 TOP_cmpltu_r_b, 
		 TOP_cmpltu_i_r, 
		 TOP_cmpltu_ii_r, 
		 TOP_cmpltu_i_b, 
		 TOP_cmpltu_ii_b, 
		 TOP_maxu_r, 
		 TOP_maxu_i, 
		 TOP_maxu_ii, 
		 TOP_minu_r, 
		 TOP_minu_i, 
		 TOP_minu_ii, 
		 TOP_mulhhu_r, 
		 TOP_mulhhu_i, 
		 TOP_mulhhu_ii, 
		 TOP_mulhu_r, 
		 TOP_mulhu_i, 
		 TOP_mulhu_ii, 
		 TOP_mullhu_r, 
		 TOP_mullhu_i, 
		 TOP_mullhu_ii, 
		 TOP_mulllu_r, 
		 TOP_mulllu_i, 
		 TOP_mulllu_ii, 
		 TOP_mullu_r, 
		 TOP_mullu_i, 
		 TOP_mullu_ii, 
		 TOP_shru_r, 
		 TOP_shru_i, 
		 TOP_shru_ii, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  xfer = ISA_Property_Create ("xfer"); 
  Instruction_Group (xfer, 
		 TOP_br, 
		 TOP_brf, 
		 TOP_call, 
		 TOP_icall, 
		 TOP_goto, 
		 TOP_igoto, 
		 TOP_rfi, 
		 TOP_return, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  cmp = ISA_Property_Create ("cmp"); 
  Instruction_Group (cmp, 
		 TOP_andl_r_r, 
		 TOP_andl_r_b, 
		 TOP_andl_i_r, 
		 TOP_andl_ii_r, 
		 TOP_andl_i_b, 
		 TOP_andl_ii_b, 
		 TOP_cmpeq_r_r, 
		 TOP_cmpeq_r_b, 
		 TOP_cmpeq_i_r, 
		 TOP_cmpeq_ii_r, 
		 TOP_cmpeq_i_b, 
		 TOP_cmpeq_ii_b, 
		 TOP_cmpge_r_r, 
		 TOP_cmpge_r_b, 
		 TOP_cmpge_i_r, 
		 TOP_cmpge_ii_r, 
		 TOP_cmpge_i_b, 
		 TOP_cmpge_ii_b, 
		 TOP_cmpgeu_r_r, 
		 TOP_cmpgeu_r_b, 
		 TOP_cmpgeu_i_r, 
		 TOP_cmpgeu_ii_r, 
		 TOP_cmpgeu_i_b, 
		 TOP_cmpgeu_ii_b, 
		 TOP_cmpgt_r_r, 
		 TOP_cmpgt_r_b, 
		 TOP_cmpgt_i_r, 
		 TOP_cmpgt_ii_r, 
		 TOP_cmpgt_i_b, 
		 TOP_cmpgt_ii_b, 
		 TOP_cmpgtu_r_r, 
		 TOP_cmpgtu_r_b, 
		 TOP_cmpgtu_i_r, 
		 TOP_cmpgtu_ii_r, 
		 TOP_cmpgtu_i_b, 
		 TOP_cmpgtu_ii_b, 
		 TOP_cmple_r_r, 
		 TOP_cmple_r_b, 
		 TOP_cmple_i_r, 
		 TOP_cmple_ii_r, 
		 TOP_cmple_i_b, 
		 TOP_cmple_ii_b, 
		 TOP_cmpleu_r_r, 
		 TOP_cmpleu_r_b, 
		 TOP_cmpleu_i_r, 
		 TOP_cmpleu_ii_r, 
		 TOP_cmpleu_i_b, 
		 TOP_cmpleu_ii_b, 
		 TOP_cmplt_r_r, 
		 TOP_cmplt_r_b, 
		 TOP_cmplt_i_r, 
		 TOP_cmplt_ii_r, 
		 TOP_cmplt_i_b, 
		 TOP_cmplt_ii_b, 
		 TOP_cmpltu_r_r, 
		 TOP_cmpltu_r_b, 
		 TOP_cmpltu_i_r, 
		 TOP_cmpltu_ii_r, 
		 TOP_cmpltu_i_b, 
		 TOP_cmpltu_ii_b, 
		 TOP_cmpne_r_r, 
		 TOP_cmpne_r_b, 
		 TOP_cmpne_i_r, 
		 TOP_cmpne_ii_r, 
		 TOP_cmpne_i_b, 
		 TOP_cmpne_ii_b, 
		 TOP_nandl_r_r, 
		 TOP_nandl_r_b, 
		 TOP_nandl_i_r, 
		 TOP_nandl_ii_r, 
		 TOP_nandl_i_b, 
		 TOP_nandl_ii_b, 
		 TOP_norl_r_r, 
		 TOP_norl_r_b, 
		 TOP_norl_i_r, 
		 TOP_norl_ii_r, 
		 TOP_norl_i_b, 
		 TOP_norl_ii_b, 
		 TOP_orl_r_r, 
		 TOP_orl_r_b, 
		 TOP_orl_i_r, 
		 TOP_orl_ii_r, 
		 TOP_orl_i_b, 
		 TOP_orl_ii_b, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  jump = ISA_Property_Create ("jump"); 
  Instruction_Group (jump, 
		 TOP_goto, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  move = ISA_Property_Create ("move"); 
  Instruction_Group (move, 
		 TOP_mov_r, 
		 TOP_mov_i, 
		 TOP_mov_ii, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  call = ISA_Property_Create ("call"); 
  Instruction_Group (call, 
		 TOP_call, 
		 TOP_icall, 
		 TOP_rfi, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  shl = ISA_Property_Create ("shl"); 
  Instruction_Group (shl, 
		 TOP_shl_r, 
		 TOP_shl_i, 
		 TOP_shl_ii, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  defs_fcc = ISA_Property_Create ("defs_fcc"); 
  Instruction_Group (defs_fcc, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  defs_fcr = ISA_Property_Create ("defs_fcr"); 
  Instruction_Group (defs_fcr, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  refs_fcr = ISA_Property_Create ("refs_fcr"); 
  Instruction_Group (refs_fcr, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  branch_predict = ISA_Property_Create ("branch_predict"); 
  Instruction_Group (branch_predict, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  access_reg_bank = ISA_Property_Create ("access_reg_bank"); 
  Instruction_Group (access_reg_bank, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  side_effects = ISA_Property_Create ("side_effects"); 
  Instruction_Group (side_effects, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  unalign = ISA_Property_Create ("unalign"); 
  Instruction_Group (unalign, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  mem_fill_type = ISA_Property_Create ("mem_fill_type"); 
  Instruction_Group (mem_fill_type, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  likely = ISA_Property_Create ("likely"); 
  Instruction_Group (likely, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  l_group = ISA_Property_Create ("l_group"); 
  Instruction_Group (l_group, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  f_group = ISA_Property_Create ("f_group"); 
  Instruction_Group (f_group, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  flop = ISA_Property_Create ("flop"); 
  Instruction_Group (flop, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  madd = ISA_Property_Create ("madd"); 
  Instruction_Group (madd, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  guard_t = ISA_Property_Create ("guard_t"); 
  Instruction_Group (guard_t, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  guard_f = ISA_Property_Create ("guard_f"); 
  Instruction_Group (guard_f, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  div = ISA_Property_Create ("div"); 
  Instruction_Group (div, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  unsafe = ISA_Property_Create ("unsafe"); 
  Instruction_Group (unsafe, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  /*         Memory Access Size 1          */ 
  /* ====================================== */ 
  ISA_Memory_Access (1, 
		 TOP_ldb_i, 
		 TOP_ldb_ii, 
		 TOP_ldb_d_i, 
		 TOP_ldb_d_ii, 
		 TOP_ldbu_i, 
		 TOP_ldbu_ii, 
		 TOP_ldbu_d_i, 
		 TOP_ldbu_d_ii, 
		 TOP_stb_i, 
		 TOP_stb_ii, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  /*         Memory Access Size 2          */ 
  /* ====================================== */ 
  ISA_Memory_Access (2, 
		 TOP_ldh_i, 
		 TOP_ldh_ii, 
		 TOP_ldh_d_i, 
		 TOP_ldh_d_ii, 
		 TOP_ldhu_i, 
		 TOP_ldhu_ii, 
		 TOP_ldhu_d_i, 
		 TOP_ldhu_d_ii, 
		 TOP_sth_i, 
		 TOP_sth_ii, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  /*         Memory Access Size 4          */ 
  /* ====================================== */ 
  ISA_Memory_Access (4, 
		 TOP_ldw_i, 
		 TOP_ldw_ii, 
		 TOP_ldw_d_i, 
		 TOP_ldw_d_ii, 
		 TOP_stw_i, 
		 TOP_stw_ii, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  /*          Memory Alignment 1           */ 
  /* ====================================== */ 
  ISA_Memory_Alignment (1, 
		 TOP_ldb_i, 
		 TOP_ldb_ii, 
		 TOP_ldb_d_i, 
		 TOP_ldb_d_ii, 
		 TOP_ldbu_i, 
		 TOP_ldbu_ii, 
		 TOP_ldbu_d_i, 
		 TOP_ldbu_d_ii, 
		 TOP_stb_i, 
		 TOP_stb_ii, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  /*          Memory Alignment 2           */ 
  /* ====================================== */ 
  ISA_Memory_Alignment (2, 
		 TOP_ldh_i, 
		 TOP_ldh_ii, 
		 TOP_ldh_d_i, 
		 TOP_ldh_d_ii, 
		 TOP_ldhu_i, 
		 TOP_ldhu_ii, 
		 TOP_ldhu_d_i, 
		 TOP_ldhu_d_ii, 
		 TOP_sth_i, 
		 TOP_sth_ii, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  /*          Memory Alignment 4           */ 
  /* ====================================== */ 
  ISA_Memory_Alignment (4, 
		 TOP_ldw_i, 
		 TOP_ldw_ii, 
		 TOP_ldw_d_i, 
		 TOP_ldw_d_ii, 
		 TOP_stw_i, 
		 TOP_stw_ii, 
		 TOP_UNDEFINED); 

  ISA_Properties_End();
  return 0;
}
