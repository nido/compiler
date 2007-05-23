/* -- This file is automatically generated -- */ 
/* 

  Copyright (C) 2002, 2004 ST Microelectronics, Inc.  All Rights Reserved. 

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
 
// AUTOMATICALLY GENERATED FROM MDS DATA BASE !!! 

//  Generate ISA properties information 
/////////////////////////////////////// 
#include <stddef.h> 
#include "topcode.h" 
#include "isa_properties_gen.h" 

main() 
{ 
  ISA_PROPERTY 
    And, 
    Or, 
    Xor, 
    access_reg_bank, 
    add, 
    automod, 
    atomicseq, 
    barrier, 
    branch_predict, 
    cache, 
    call, 
    cmp, 
    compose, 
    cond, 
    defs_fcc, 
    defs_fcr, 
    dismissible, 
    div, 
    dummy, 
    extract, 
    f_group, 
    fadd, 
    flop, 
    fmul, 
    fsub, 
    guard_f, 
    guard_t, 
    ijump, 
    intop, 
    jump, 
    l_group, 
    likely, 
    load, 
    madd, 
    max, 
    mem_fill_type, 
    min, 
    move, 
    mul, 
    multi, 
    noop, 
    prefetch, 
    refs_fcr, 
    select, 
    sext, 
    shl, 
    shr, 
    shru, 
    side_effects, 
    simulated, 
    ssa, 
    store, 
    sub, 
    unalign, 
    unsafe, 
    unsign, 
    var_opnds, 
    widemove, 
    xfer, 
    xmove, 
    zext;

  ISA_Properties_Begin ("st200"); 


  /* ==================================================================== 
   *              Operator attributes descriptors 
   * ==================================================================== 
   */ 

  /* ====================================== */ 
  And = ISA_Property_Create ("and"); 
  Instruction_Group (And, 
		 TOP_and_r, 
		 TOP_and_i, 
		 TOP_and_ii, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  Or = ISA_Property_Create ("or"); 
  Instruction_Group (Or, 
		 TOP_or_r, 
		 TOP_or_i, 
		 TOP_or_ii, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  Xor = ISA_Property_Create ("xor"); 
  Instruction_Group (Xor, 
		 TOP_xor_r, 
		 TOP_xor_i, 
		 TOP_xor_ii, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  access_reg_bank = ISA_Property_Create ("access_reg_bank"); 
  Instruction_Group (access_reg_bank, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  add = ISA_Property_Create ("add"); 
  Instruction_Group (add, 
		 TOP_add_r, 
		 TOP_add_i, 
		 TOP_add_ii, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  atomicseq = ISA_Property_Create ("atomicseq"); 
  Instruction_Group (atomicseq, 
		 TOP_ldwl, 
		 TOP_stwl, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  automod = ISA_Property_Create ("automod"); 
  Instruction_Group (automod, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  barrier = ISA_Property_Create ("barrier"); 
  Instruction_Group (barrier, 
		 TOP_break, 
		 TOP_bwd_bar, 
		 TOP_fwd_bar, 
		 TOP_ldwl, 
		 TOP_prgadd_i, 
		 TOP_prgadd_ii, 
		 TOP_prgins, 
		 TOP_prginspg_i, 
		 TOP_prginspg_ii, 
		 TOP_prgset_i, 
		 TOP_prgset_ii, 
		 TOP_pswclr, 
		 TOP_pswset, 
		 TOP_pushregs, 
		 TOP_sbrk, 
		 TOP_st235_sbrk, 
		 TOP_stwl, 
		 TOP_sync, 
		 TOP_syncins, 
		 TOP_syscall, 
		 TOP_st235_syscall, 
		 TOP_wmb, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  branch_predict = ISA_Property_Create ("branch_predict"); 
  Instruction_Group (branch_predict, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  cache = ISA_Property_Create ("cache"); 
  Instruction_Group (cache, 
		 TOP_pftc_i, 
		 TOP_pftc_ii, 
		 TOP_pft_i, 
		 TOP_pft_ii, 
		 TOP_prgadd_i, 
		 TOP_prgadd_ii, 
		 TOP_prginsadd_i, 
		 TOP_prginsadd_ii, 
		 TOP_prginspg_i, 
		 TOP_prginspg_ii, 
		 TOP_prgset_i, 
		 TOP_prgset_ii, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  call = ISA_Property_Create ("call"); 
  Instruction_Group (call, 
		 TOP_call, 
		 TOP_icall, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  cmp = ISA_Property_Create ("cmp"); 
  Instruction_Group (cmp, 
		 TOP_andl_r_b, 
		 TOP_andl_i_b, 
		 TOP_andl_ii_b, 
		 TOP_andl_r_r, 
		 TOP_andl_i_r, 
		 TOP_andl_ii_r, 
		 TOP_cmpeqf_n_b, 
		 TOP_cmpeqf_n_r, 
		 TOP_cmpeq_r_b, 
		 TOP_cmpeq_i_b, 
		 TOP_cmpeq_ii_b, 
		 TOP_cmpeq_r_r, 
		 TOP_cmpeq_i_r, 
		 TOP_cmpeq_ii_r, 
		 TOP_cmpgef_n_b, 
		 TOP_cmpgef_n_r, 
		 TOP_cmpgeu_r_b, 
		 TOP_cmpgeu_i_b, 
		 TOP_cmpgeu_ii_b, 
		 TOP_cmpgeu_r_r, 
		 TOP_cmpgeu_i_r, 
		 TOP_cmpgeu_ii_r, 
		 TOP_cmpge_r_b, 
		 TOP_cmpge_i_b, 
		 TOP_cmpge_ii_b, 
		 TOP_cmpge_r_r, 
		 TOP_cmpge_i_r, 
		 TOP_cmpge_ii_r, 
		 TOP_cmpgtf_n_b, 
		 TOP_cmpgtf_n_r, 
		 TOP_cmpgtu_r_b, 
		 TOP_cmpgtu_i_b, 
		 TOP_cmpgtu_ii_b, 
		 TOP_cmpgtu_r_r, 
		 TOP_cmpgtu_i_r, 
		 TOP_cmpgtu_ii_r, 
		 TOP_cmpgt_r_b, 
		 TOP_cmpgt_i_b, 
		 TOP_cmpgt_ii_b, 
		 TOP_cmpgt_r_r, 
		 TOP_cmpgt_i_r, 
		 TOP_cmpgt_ii_r, 
		 TOP_cmplef_n_b, 
		 TOP_cmplef_n_r, 
		 TOP_cmpleu_r_b, 
		 TOP_cmpleu_i_b, 
		 TOP_cmpleu_ii_b, 
		 TOP_cmpleu_r_r, 
		 TOP_cmpleu_i_r, 
		 TOP_cmpleu_ii_r, 
		 TOP_cmple_r_b, 
		 TOP_cmple_i_b, 
		 TOP_cmple_ii_b, 
		 TOP_cmple_r_r, 
		 TOP_cmple_i_r, 
		 TOP_cmple_ii_r, 
		 TOP_cmpltf_n_b, 
		 TOP_cmpltf_n_r, 
		 TOP_cmpltu_r_b, 
		 TOP_cmpltu_i_b, 
		 TOP_cmpltu_ii_b, 
		 TOP_cmpltu_r_r, 
		 TOP_cmpltu_i_r, 
		 TOP_cmpltu_ii_r, 
		 TOP_cmplt_r_b, 
		 TOP_cmplt_i_b, 
		 TOP_cmplt_ii_b, 
		 TOP_cmplt_r_r, 
		 TOP_cmplt_i_r, 
		 TOP_cmplt_ii_r, 
		 TOP_cmpne_r_b, 
		 TOP_cmpne_i_b, 
		 TOP_cmpne_ii_b, 
		 TOP_cmpne_r_r, 
		 TOP_cmpne_i_r, 
		 TOP_cmpne_ii_r, 
		 TOP_nandl_r_b, 
		 TOP_nandl_i_b, 
		 TOP_nandl_ii_b, 
		 TOP_nandl_r_r, 
		 TOP_nandl_i_r, 
		 TOP_nandl_ii_r, 
		 TOP_norl_r_b, 
		 TOP_norl_i_b, 
		 TOP_norl_ii_b, 
		 TOP_norl_r_r, 
		 TOP_norl_i_r, 
		 TOP_norl_ii_r, 
		 TOP_orl_r_b, 
		 TOP_orl_i_b, 
		 TOP_orl_ii_b, 
		 TOP_orl_r_r, 
		 TOP_orl_i_r, 
		 TOP_orl_ii_r, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  compose = ISA_Property_Create ("compose"); 
  Instruction_Group (compose, 
		 TOP_composep, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  cond = ISA_Property_Create ("cond"); 
  Instruction_Group (cond, 
		 TOP_brf, 
		 TOP_br, 
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
  dismissible = ISA_Property_Create ("dismissible"); 
  Instruction_Group (dismissible, 
		 TOP_ldbu_d_i, 
		 TOP_ldbu_d_ii, 
		 TOP_ldb_d_i, 
		 TOP_ldb_d_ii, 
		 TOP_ldhu_d_i, 
		 TOP_ldhu_d_ii, 
		 TOP_ldh_d_i, 
		 TOP_ldh_d_ii, 
		 TOP_ldw_d_i, 
		 TOP_ldw_d_ii, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  div = ISA_Property_Create ("div"); 
  Instruction_Group (div, 
		 TOP_divu, 
		 TOP_div, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  dummy = ISA_Property_Create ("dummy"); 
  Instruction_Group (dummy, 
		 TOP_begin_pregtn, 
		 TOP_bwd_bar, 
		 TOP_CALL, 
		 TOP_COPY, 
		 TOP_end_pregtn, 
		 TOP_FALL, 
		 TOP_fwd_bar, 
		 TOP_GOTO, 
		 TOP_ifixup, 
		 TOP_intrncall, 
		 TOP_JUMP, 
		 TOP_KILL, 
		 TOP_label, 
		 TOP_LINK, 
		 TOP_LOOP, 
		 TOP_movc, 
		 TOP_movcf, 
		 TOP_noop, 
		 TOP_phi, 
		 TOP_psi, 
		 TOP_RETURN, 
		 TOP_SIGMA, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  extract = ISA_Property_Create ("extract"); 
  Instruction_Group (extract, 
		 TOP_extractp, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  f_group = ISA_Property_Create ("f_group"); 
  Instruction_Group (f_group, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  fadd = ISA_Property_Create ("fadd"); 
  Instruction_Group (fadd, 
		 TOP_addf_n, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  flop = ISA_Property_Create ("flop"); 
  Instruction_Group (flop, 
		 TOP_addf_n, 
		 TOP_cmpeqf_n_b, 
		 TOP_cmpeqf_n_r, 
		 TOP_cmpgef_n_b, 
		 TOP_cmpgef_n_r, 
		 TOP_cmpgtf_n_b, 
		 TOP_cmpgtf_n_r, 
		 TOP_cmplef_n_b, 
		 TOP_cmplef_n_r, 
		 TOP_cmpltf_n_b, 
		 TOP_cmpltf_n_r, 
		 TOP_mulf_n, 
		 TOP_subf_n, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  fmul = ISA_Property_Create ("fmul"); 
  Instruction_Group (fmul, 
		 TOP_mulf_n, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  fsub = ISA_Property_Create ("fsub"); 
  Instruction_Group (fsub, 
		 TOP_subf_n, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  guard_f = ISA_Property_Create ("guard_f"); 
  Instruction_Group (guard_f, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  guard_t = ISA_Property_Create ("guard_t"); 
  Instruction_Group (guard_t, 
		 TOP_ldbc_i, 
		 TOP_ldbc_ii, 
		 TOP_ldbuc_i, 
		 TOP_ldbuc_ii, 
		 TOP_ldhc_i, 
		 TOP_ldhc_ii, 
		 TOP_ldhuc_i, 
		 TOP_ldhuc_ii, 
		 TOP_ldlc_i, 
		 TOP_ldlc_ii, 
		 TOP_ldwc_i, 
		 TOP_ldwc_ii, 
		 TOP_movc, 
		 TOP_movcf, 
		 TOP_multi_ldlc_i, 
		 TOP_multi_ldlc_ii, 
		 TOP_multi_stlc_i, 
		 TOP_multi_stlc_ii, 
		 TOP_pftc_i, 
		 TOP_pftc_ii, 
		 TOP_stbc_i, 
		 TOP_stbc_ii, 
		 TOP_sthc_i, 
		 TOP_sthc_ii, 
		 TOP_stlc_i, 
		 TOP_stlc_ii, 
		 TOP_stwc_i, 
		 TOP_stwc_ii, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  ijump = ISA_Property_Create ("ijump"); 
  Instruction_Group (ijump, 
		 TOP_igoto, 
		 TOP_return, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  intop = ISA_Property_Create ("intop"); 
  Instruction_Group (intop, 
		 TOP_addpc_i, 
		 TOP_addpc_ii, 
		 TOP_add_r, 
		 TOP_add_i, 
		 TOP_add_ii, 
		 TOP_andc_r, 
		 TOP_andc_i, 
		 TOP_andc_ii, 
		 TOP_andl_r_b, 
		 TOP_andl_i_b, 
		 TOP_andl_ii_b, 
		 TOP_andl_r_r, 
		 TOP_andl_i_r, 
		 TOP_andl_ii_r, 
		 TOP_and_r, 
		 TOP_and_i, 
		 TOP_and_ii, 
		 TOP_bswap, 
		 TOP_clz, 
		 TOP_cmpeq_r_b, 
		 TOP_cmpeq_i_b, 
		 TOP_cmpeq_ii_b, 
		 TOP_cmpeq_r_r, 
		 TOP_cmpeq_i_r, 
		 TOP_cmpeq_ii_r, 
		 TOP_cmpgeu_r_b, 
		 TOP_cmpgeu_i_b, 
		 TOP_cmpgeu_ii_b, 
		 TOP_cmpgeu_r_r, 
		 TOP_cmpgeu_i_r, 
		 TOP_cmpgeu_ii_r, 
		 TOP_cmpge_r_b, 
		 TOP_cmpge_i_b, 
		 TOP_cmpge_ii_b, 
		 TOP_cmpge_r_r, 
		 TOP_cmpge_i_r, 
		 TOP_cmpge_ii_r, 
		 TOP_cmpgtu_r_b, 
		 TOP_cmpgtu_i_b, 
		 TOP_cmpgtu_ii_b, 
		 TOP_cmpgtu_r_r, 
		 TOP_cmpgtu_i_r, 
		 TOP_cmpgtu_ii_r, 
		 TOP_cmpgt_r_b, 
		 TOP_cmpgt_i_b, 
		 TOP_cmpgt_ii_b, 
		 TOP_cmpgt_r_r, 
		 TOP_cmpgt_i_r, 
		 TOP_cmpgt_ii_r, 
		 TOP_cmpleu_r_b, 
		 TOP_cmpleu_i_b, 
		 TOP_cmpleu_ii_b, 
		 TOP_cmpleu_r_r, 
		 TOP_cmpleu_i_r, 
		 TOP_cmpleu_ii_r, 
		 TOP_cmple_r_b, 
		 TOP_cmple_i_b, 
		 TOP_cmple_ii_b, 
		 TOP_cmple_r_r, 
		 TOP_cmple_i_r, 
		 TOP_cmple_ii_r, 
		 TOP_cmpltu_r_b, 
		 TOP_cmpltu_i_b, 
		 TOP_cmpltu_ii_b, 
		 TOP_cmpltu_r_r, 
		 TOP_cmpltu_i_r, 
		 TOP_cmpltu_ii_r, 
		 TOP_cmplt_r_b, 
		 TOP_cmplt_i_b, 
		 TOP_cmplt_ii_b, 
		 TOP_cmplt_r_r, 
		 TOP_cmplt_i_r, 
		 TOP_cmplt_ii_r, 
		 TOP_cmpne_r_b, 
		 TOP_cmpne_i_b, 
		 TOP_cmpne_ii_b, 
		 TOP_cmpne_r_r, 
		 TOP_cmpne_i_r, 
		 TOP_cmpne_ii_r, 
		 TOP_divu, 
		 TOP_div, 
		 TOP_maxu_r, 
		 TOP_maxu_i, 
		 TOP_maxu_ii, 
		 TOP_max_r, 
		 TOP_max_i, 
		 TOP_max_ii, 
		 TOP_minu_r, 
		 TOP_minu_i, 
		 TOP_minu_ii, 
		 TOP_min_r, 
		 TOP_min_i, 
		 TOP_min_ii, 
		 TOP_mov_r, 
		 TOP_mov_i, 
		 TOP_mov_ii, 
		 TOP_mul32_r, 
		 TOP_mul32_i, 
		 TOP_mul32_ii, 
		 TOP_mul64hu_r, 
		 TOP_mul64hu_i, 
		 TOP_mul64hu_ii, 
		 TOP_mul64h_r, 
		 TOP_mul64h_i, 
		 TOP_mul64h_ii, 
		 TOP_mulfrac_r, 
		 TOP_mulfrac_i, 
		 TOP_mulfrac_ii, 
		 TOP_mulhhs_r, 
		 TOP_mulhhs_i, 
		 TOP_mulhhs_ii, 
		 TOP_mulhhu_r, 
		 TOP_mulhhu_i, 
		 TOP_mulhhu_ii, 
		 TOP_mulhh_r, 
		 TOP_mulhh_i, 
		 TOP_mulhh_ii, 
		 TOP_mulhs_r, 
		 TOP_mulhs_i, 
		 TOP_mulhs_ii, 
		 TOP_mulhu_r, 
		 TOP_mulhu_i, 
		 TOP_mulhu_ii, 
		 TOP_mulh_r, 
		 TOP_mulh_i, 
		 TOP_mulh_ii, 
		 TOP_mullhus_r, 
		 TOP_mullhus_i, 
		 TOP_mullhus_ii, 
		 TOP_mullhu_r, 
		 TOP_mullhu_i, 
		 TOP_mullhu_ii, 
		 TOP_mullh_r, 
		 TOP_mullh_i, 
		 TOP_mullh_ii, 
		 TOP_mulllu_r, 
		 TOP_mulllu_i, 
		 TOP_mulllu_ii, 
		 TOP_mulll_r, 
		 TOP_mulll_i, 
		 TOP_mulll_ii, 
		 TOP_mullu_r, 
		 TOP_mullu_i, 
		 TOP_mullu_ii, 
		 TOP_mull_r, 
		 TOP_mull_i, 
		 TOP_mull_ii, 
		 TOP_nandl_r_b, 
		 TOP_nandl_i_b, 
		 TOP_nandl_ii_b, 
		 TOP_nandl_r_r, 
		 TOP_nandl_i_r, 
		 TOP_nandl_ii_r, 
		 TOP_norl_r_b, 
		 TOP_norl_i_b, 
		 TOP_norl_ii_b, 
		 TOP_norl_r_r, 
		 TOP_norl_i_r, 
		 TOP_norl_ii_r, 
		 TOP_orc_r, 
		 TOP_orc_i, 
		 TOP_orc_ii, 
		 TOP_orl_r_b, 
		 TOP_orl_i_b, 
		 TOP_orl_ii_b, 
		 TOP_orl_r_r, 
		 TOP_orl_i_r, 
		 TOP_orl_ii_r, 
		 TOP_or_r, 
		 TOP_or_i, 
		 TOP_or_ii, 
		 TOP_remu, 
		 TOP_rem, 
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
		 TOP_shru_r, 
		 TOP_shru_i, 
		 TOP_shru_ii, 
		 TOP_shr_r, 
		 TOP_shr_i, 
		 TOP_shr_ii, 
		 TOP_slctf_r, 
		 TOP_slctf_i, 
		 TOP_slctf_ii, 
		 TOP_slct_r, 
		 TOP_slct_i, 
		 TOP_slct_ii, 
		 TOP_sub_r, 
		 TOP_sub_i, 
		 TOP_sub_ii, 
		 TOP_sxtb, 
		 TOP_sxth, 
		 TOP_xor_r, 
		 TOP_xor_i, 
		 TOP_xor_ii, 
		 TOP_zxtb, 
		 TOP_zxth, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  jump = ISA_Property_Create ("jump"); 
  Instruction_Group (jump, 
		 TOP_goto, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  l_group = ISA_Property_Create ("l_group"); 
  Instruction_Group (l_group, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  likely = ISA_Property_Create ("likely"); 
  Instruction_Group (likely, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  load = ISA_Property_Create ("load"); 
  Instruction_Group (load, 
		 TOP_ldbc_i, 
		 TOP_ldbc_ii, 
		 TOP_ldbuc_i, 
		 TOP_ldbuc_ii, 
		 TOP_ldbu_d_i, 
		 TOP_ldbu_d_ii, 
		 TOP_ldbu_i, 
		 TOP_ldbu_ii, 
		 TOP_ldb_d_i, 
		 TOP_ldb_d_ii, 
		 TOP_ldb_i, 
		 TOP_ldb_ii, 
		 TOP_ldhc_i, 
		 TOP_ldhc_ii, 
		 TOP_ldhuc_i, 
		 TOP_ldhuc_ii, 
		 TOP_ldhu_d_i, 
		 TOP_ldhu_d_ii, 
		 TOP_ldhu_i, 
		 TOP_ldhu_ii, 
		 TOP_ldh_d_i, 
		 TOP_ldh_d_ii, 
		 TOP_ldh_i, 
		 TOP_ldh_ii, 
		 TOP_ldlc_i, 
		 TOP_ldlc_ii, 
		 TOP_ldl_i, 
		 TOP_ldl_ii, 
		 TOP_ldwc_i, 
		 TOP_ldwc_ii, 
		 TOP_ldw_d_i, 
		 TOP_ldw_d_ii, 
		 TOP_ldw_i, 
		 TOP_ldw_ii, 
		 TOP_multi_ldlc_i, 
		 TOP_multi_ldlc_ii, 
		 TOP_multi_ldl_i, 
		 TOP_multi_ldl_ii, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  madd = ISA_Property_Create ("madd"); 
  Instruction_Group (madd, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  max = ISA_Property_Create ("max"); 
  Instruction_Group (max, 
		 TOP_maxu_r, 
		 TOP_maxu_i, 
		 TOP_maxu_ii, 
		 TOP_max_r, 
		 TOP_max_i, 
		 TOP_max_ii, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  mem_fill_type = ISA_Property_Create ("mem_fill_type"); 
  Instruction_Group (mem_fill_type, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  min = ISA_Property_Create ("min"); 
  Instruction_Group (min, 
		 TOP_minu_r, 
		 TOP_minu_i, 
		 TOP_minu_ii, 
		 TOP_min_r, 
		 TOP_min_i, 
		 TOP_min_ii, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  move = ISA_Property_Create ("move"); 
  Instruction_Group (move, 
		 TOP_movp, 
		 TOP_mov_r, 
		 TOP_mov_i, 
		 TOP_mov_ii, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  mul = ISA_Property_Create ("mul"); 
  Instruction_Group (mul, 
		 TOP_mul32_r, 
		 TOP_mul32_i, 
		 TOP_mul32_ii, 
		 TOP_mulllu_r, 
		 TOP_mulllu_i, 
		 TOP_mulllu_ii, 
		 TOP_mulll_r, 
		 TOP_mulll_i, 
		 TOP_mulll_ii, 
		 TOP_mullu_r, 
		 TOP_mullu_i, 
		 TOP_mullu_ii, 
		 TOP_mull_r, 
		 TOP_mull_i, 
		 TOP_mull_ii, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  multi = ISA_Property_Create ("multi"); 
  Instruction_Group (multi, 
		 TOP_multi_ldlc_i, 
		 TOP_multi_ldlc_ii, 
		 TOP_multi_ldl_i, 
		 TOP_multi_ldl_ii, 
		 TOP_multi_stlc_i, 
		 TOP_multi_stlc_ii, 
		 TOP_multi_stl_i, 
		 TOP_multi_stl_ii, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  noop = ISA_Property_Create ("noop"); 
  Instruction_Group (noop, 
		 TOP_noop, 
		 TOP_nop, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  prefetch = ISA_Property_Create ("prefetch"); 
  Instruction_Group (prefetch, 
		 TOP_pftc_i, 
		 TOP_pftc_ii, 
		 TOP_pft_i, 
		 TOP_pft_ii, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  refs_fcr = ISA_Property_Create ("refs_fcr"); 
  Instruction_Group (refs_fcr, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  select = ISA_Property_Create ("select"); 
  Instruction_Group (select, 
		 TOP_slctf_r, 
		 TOP_slctf_i, 
		 TOP_slctf_ii, 
		 TOP_slct_r, 
		 TOP_slct_i, 
		 TOP_slct_ii, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  sext = ISA_Property_Create ("sext"); 
  Instruction_Group (sext, 
		 TOP_sxtb, 
		 TOP_sxth, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  shl = ISA_Property_Create ("shl"); 
  Instruction_Group (shl, 
		 TOP_shl_r, 
		 TOP_shl_i, 
		 TOP_shl_ii, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  shr = ISA_Property_Create ("shr"); 
  Instruction_Group (shr, 
		 TOP_shr_r, 
		 TOP_shr_i, 
		 TOP_shr_ii, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  shru = ISA_Property_Create ("shru"); 
  Instruction_Group (shru, 
		 TOP_shru_r, 
		 TOP_shru_i, 
		 TOP_shru_ii, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  side_effects = ISA_Property_Create ("side_effects"); 
  Instruction_Group (side_effects, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  simulated = ISA_Property_Create ("simulated"); 
  Instruction_Group (simulated, 
		 TOP_spadjust, 
		 TOP_composep, 
		 TOP_extractp, 
		 TOP_getpc, 
		 TOP_asm, 
		 TOP_movp, 
		 TOP_pushregs, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  ssa = ISA_Property_Create ("ssa"); 
  Instruction_Group (ssa, 
		 TOP_phi, 
		 TOP_psi, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  store = ISA_Property_Create ("store"); 
  Instruction_Group (store, 
		 TOP_multi_stlc_i, 
		 TOP_multi_stlc_ii, 
		 TOP_multi_stl_i, 
		 TOP_multi_stl_ii, 
		 TOP_stbc_i, 
		 TOP_stbc_ii, 
		 TOP_stb_i, 
		 TOP_stb_ii, 
		 TOP_sthc_i, 
		 TOP_sthc_ii, 
		 TOP_sth_i, 
		 TOP_sth_ii, 
		 TOP_stlc_i, 
		 TOP_stlc_ii, 
		 TOP_stl_i, 
		 TOP_stl_ii, 
		 TOP_stwc_i, 
		 TOP_stwc_ii, 
		 TOP_stw_i, 
		 TOP_stw_ii, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  sub = ISA_Property_Create ("sub"); 
  Instruction_Group (sub, 
		 TOP_sub_r, 
		 TOP_sub_i, 
		 TOP_sub_ii, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  unalign = ISA_Property_Create ("unalign"); 
  Instruction_Group (unalign, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  unsafe = ISA_Property_Create ("unsafe"); 
  Instruction_Group (unsafe, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  unsign = ISA_Property_Create ("unsign"); 
  Instruction_Group (unsign, 
		 TOP_cmpgeu_r_b, 
		 TOP_cmpgeu_i_b, 
		 TOP_cmpgeu_ii_b, 
		 TOP_cmpgeu_r_r, 
		 TOP_cmpgeu_i_r, 
		 TOP_cmpgeu_ii_r, 
		 TOP_cmpgtu_r_b, 
		 TOP_cmpgtu_i_b, 
		 TOP_cmpgtu_ii_b, 
		 TOP_cmpgtu_r_r, 
		 TOP_cmpgtu_i_r, 
		 TOP_cmpgtu_ii_r, 
		 TOP_cmpleu_r_b, 
		 TOP_cmpleu_i_b, 
		 TOP_cmpleu_ii_b, 
		 TOP_cmpleu_r_r, 
		 TOP_cmpleu_i_r, 
		 TOP_cmpleu_ii_r, 
		 TOP_cmpltu_r_b, 
		 TOP_cmpltu_i_b, 
		 TOP_cmpltu_ii_b, 
		 TOP_cmpltu_r_r, 
		 TOP_cmpltu_i_r, 
		 TOP_cmpltu_ii_r, 
		 TOP_divu, 
		 TOP_ldbuc_i, 
		 TOP_ldbuc_ii, 
		 TOP_ldbu_d_i, 
		 TOP_ldbu_d_ii, 
		 TOP_ldbu_i, 
		 TOP_ldbu_ii, 
		 TOP_ldhuc_i, 
		 TOP_ldhuc_ii, 
		 TOP_ldhu_d_i, 
		 TOP_ldhu_d_ii, 
		 TOP_ldhu_i, 
		 TOP_ldhu_ii, 
		 TOP_maxu_r, 
		 TOP_maxu_i, 
		 TOP_maxu_ii, 
		 TOP_minu_r, 
		 TOP_minu_i, 
		 TOP_minu_ii, 
		 TOP_mul64hu_r, 
		 TOP_mul64hu_i, 
		 TOP_mul64hu_ii, 
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
		 TOP_remu, 
		 TOP_shru_r, 
		 TOP_shru_i, 
		 TOP_shru_ii, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  var_opnds = ISA_Property_Create ("var_opnds"); 
  Instruction_Group (var_opnds, 
		 TOP_asm, 
		 TOP_intrncall, 
		 TOP_phi, 
		 TOP_psi, 
		 TOP_pushregs, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  widemove = ISA_Property_Create ("widemove"); 
  Instruction_Group (widemove, 
		 TOP_movp, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  xfer = ISA_Property_Create ("xfer"); 
  Instruction_Group (xfer, 
		 TOP_brf, 
		 TOP_br, 
		 TOP_call, 
		 TOP_goto, 
		 TOP_icall, 
		 TOP_igoto, 
		 TOP_return, 
		 TOP_rfi, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  zext = ISA_Property_Create ("zext"); 
  Instruction_Group (zext, 
		 TOP_zxtb, 
		 TOP_zxth, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  /*         Memory Access Size 1          */ 
  /* ====================================== */ 
  ISA_Memory_Access (1, 
		 TOP_ldbc_i, 
		 TOP_ldbc_ii, 
		 TOP_ldbuc_i, 
		 TOP_ldbuc_ii, 
		 TOP_ldbu_d_i, 
		 TOP_ldbu_d_ii, 
		 TOP_ldbu_i, 
		 TOP_ldbu_ii, 
		 TOP_ldb_d_i, 
		 TOP_ldb_d_ii, 
		 TOP_ldb_i, 
		 TOP_ldb_ii, 
		 TOP_stbc_i, 
		 TOP_stbc_ii, 
		 TOP_stb_i, 
		 TOP_stb_ii, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  /*         Memory Access Size 2          */ 
  /* ====================================== */ 
  ISA_Memory_Access (2, 
		 TOP_ldhc_i, 
		 TOP_ldhc_ii, 
		 TOP_ldhuc_i, 
		 TOP_ldhuc_ii, 
		 TOP_ldhu_d_i, 
		 TOP_ldhu_d_ii, 
		 TOP_ldhu_i, 
		 TOP_ldhu_ii, 
		 TOP_ldh_d_i, 
		 TOP_ldh_d_ii, 
		 TOP_ldh_i, 
		 TOP_ldh_ii, 
		 TOP_sthc_i, 
		 TOP_sthc_ii, 
		 TOP_sth_i, 
		 TOP_sth_ii, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  /*         Memory Access Size 4          */ 
  /* ====================================== */ 
  ISA_Memory_Access (4, 
		 TOP_ldwc_i, 
		 TOP_ldwc_ii, 
		 TOP_ldw_d_i, 
		 TOP_ldw_d_ii, 
		 TOP_ldw_i, 
		 TOP_ldw_ii, 
		 TOP_stwc_i, 
		 TOP_stwc_ii, 
		 TOP_stw_i, 
		 TOP_stw_ii, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  /*         Memory Access Size 8          */ 
  /* ====================================== */ 
  ISA_Memory_Access (8, 
		 TOP_ldlc_i, 
		 TOP_ldlc_ii, 
		 TOP_ldl_i, 
		 TOP_ldl_ii, 
		 TOP_multi_ldlc_i, 
		 TOP_multi_ldlc_ii, 
		 TOP_multi_ldl_i, 
		 TOP_multi_ldl_ii, 
		 TOP_multi_stlc_i, 
		 TOP_multi_stlc_ii, 
		 TOP_multi_stl_i, 
		 TOP_multi_stl_ii, 
		 TOP_stlc_i, 
		 TOP_stlc_ii, 
		 TOP_stl_i, 
		 TOP_stl_ii, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  /*         Memory Alignment 1          */ 
  /* ====================================== */ 
  ISA_Memory_Alignment (1, 
		 TOP_ldbc_i, 
		 TOP_ldbc_ii, 
		 TOP_ldbuc_i, 
		 TOP_ldbuc_ii, 
		 TOP_ldbu_d_i, 
		 TOP_ldbu_d_ii, 
		 TOP_ldbu_i, 
		 TOP_ldbu_ii, 
		 TOP_ldb_d_i, 
		 TOP_ldb_d_ii, 
		 TOP_ldb_i, 
		 TOP_ldb_ii, 
		 TOP_stbc_i, 
		 TOP_stbc_ii, 
		 TOP_stb_i, 
		 TOP_stb_ii, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  /*         Memory Alignment 2          */ 
  /* ====================================== */ 
  ISA_Memory_Alignment (2, 
		 TOP_ldhc_i, 
		 TOP_ldhc_ii, 
		 TOP_ldhuc_i, 
		 TOP_ldhuc_ii, 
		 TOP_ldhu_d_i, 
		 TOP_ldhu_d_ii, 
		 TOP_ldhu_i, 
		 TOP_ldhu_ii, 
		 TOP_ldh_d_i, 
		 TOP_ldh_d_ii, 
		 TOP_ldh_i, 
		 TOP_ldh_ii, 
		 TOP_sthc_i, 
		 TOP_sthc_ii, 
		 TOP_sth_i, 
		 TOP_sth_ii, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  /*         Memory Alignment 4          */ 
  /* ====================================== */ 
  ISA_Memory_Alignment (4, 
		 TOP_ldwc_i, 
		 TOP_ldwc_ii, 
		 TOP_ldw_d_i, 
		 TOP_ldw_d_ii, 
		 TOP_ldw_i, 
		 TOP_ldw_ii, 
		 TOP_stwc_i, 
		 TOP_stwc_ii, 
		 TOP_stw_i, 
		 TOP_stw_ii, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  /*         Memory Alignment 8          */ 
  /* ====================================== */ 
  ISA_Memory_Alignment (8, 
		 TOP_ldlc_i, 
		 TOP_ldlc_ii, 
		 TOP_ldl_i, 
		 TOP_ldl_ii, 
		 TOP_multi_ldlc_i, 
		 TOP_multi_ldlc_ii, 
		 TOP_multi_ldl_i, 
		 TOP_multi_ldl_ii, 
		 TOP_multi_stlc_i, 
		 TOP_multi_stlc_ii, 
		 TOP_multi_stl_i, 
		 TOP_multi_stl_ii, 
		 TOP_stlc_i, 
		 TOP_stlc_ii, 
		 TOP_stl_i, 
		 TOP_stl_ii, 
		 TOP_UNDEFINED); 

  ISA_Properties_End();
  return 0;
}
