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
    address, 
    asmop, 
    atomicseq, 
    automod, 
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
    packedop, 
    prefetch, 
    refs_fcr, 
    rem, 
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
		 TOP_and_r_r_r, 
		 TOP_and_i_r_r, 
		 TOP_and_ii_r_r, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  Or = ISA_Property_Create ("or"); 
  Instruction_Group (Or, 
		 TOP_or_r_r_r, 
		 TOP_or_i_r_r, 
		 TOP_or_ii_r_r, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  Xor = ISA_Property_Create ("xor"); 
  Instruction_Group (Xor, 
		 TOP_xor_r_r_r, 
		 TOP_xor_i_r_r, 
		 TOP_xor_ii_r_r, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  access_reg_bank = ISA_Property_Create ("access_reg_bank"); 
  Instruction_Group (access_reg_bank, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  add = ISA_Property_Create ("add"); 
  Instruction_Group (add, 
		 TOP_add_r_r_r, 
		 TOP_add_i_r_r, 
		 TOP_add_ii_r_r, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  address = ISA_Property_Create ("address"); 
  Instruction_Group (address, 
		 TOP_flushadd_r_i, 
		 TOP_flushadd_r_ii, 
		 TOP_flushadd_l1_r_i, 
		 TOP_flushadd_l1_r_ii, 
		 TOP_invadd_r_i, 
		 TOP_invadd_r_ii, 
		 TOP_invadd_l1_r_i, 
		 TOP_invadd_l1_r_ii, 
		 TOP_ldbc_r_i_b_r, 
		 TOP_ldbc_r_ii_b_r, 
		 TOP_ldbuc_r_i_b_r, 
		 TOP_ldbuc_r_ii_b_r, 
		 TOP_ldbu_r_i_r, 
		 TOP_ldbu_r_ii_r, 
		 TOP_ldbu_d_r_i_r, 
		 TOP_ldbu_d_r_ii_r, 
		 TOP_ldb_r_i_r, 
		 TOP_ldb_r_ii_r, 
		 TOP_ldb_d_r_i_r, 
		 TOP_ldb_d_r_ii_r, 
		 TOP_ldhc_r_i_b_r, 
		 TOP_ldhc_r_ii_b_r, 
		 TOP_ldhuc_r_i_b_r, 
		 TOP_ldhuc_r_ii_b_r, 
		 TOP_ldhu_r_i_r, 
		 TOP_ldhu_r_ii_r, 
		 TOP_ldhu_d_r_i_r, 
		 TOP_ldhu_d_r_ii_r, 
		 TOP_ldh_r_i_r, 
		 TOP_ldh_r_ii_r, 
		 TOP_ldh_d_r_i_r, 
		 TOP_ldh_d_r_ii_r, 
		 TOP_ldlc_r_i_b_p, 
		 TOP_ldlc_r_ii_b_p, 
		 TOP_ldl_r_i_p, 
		 TOP_ldl_r_ii_p, 
		 TOP_ldwc_r_i_b_r, 
		 TOP_ldwc_r_ii_b_r, 
		 TOP_ldw_r_i_r, 
		 TOP_ldw_r_ii_r, 
		 TOP_ldw_d_r_i_r, 
		 TOP_ldw_d_r_ii_r, 
		 TOP_multi_ldlc_r_i_b_r, 
		 TOP_multi_ldlc_r_ii_b_r, 
		 TOP_multi_ldl_r_i_r, 
		 TOP_multi_ldl_r_ii_r, 
		 TOP_multi_stlc_r_b_r_i, 
		 TOP_multi_stlc_r_b_r_ii, 
		 TOP_multi_stl_r_r_i, 
		 TOP_multi_stl_r_r_ii, 
		 TOP_pftc_r_i_b, 
		 TOP_pftc_r_ii_b, 
		 TOP_pft_r_i, 
		 TOP_pft_r_ii, 
		 TOP_prgadd_r_i, 
		 TOP_prgadd_r_ii, 
		 TOP_prgadd_l1_r_i, 
		 TOP_prgadd_l1_r_ii, 
		 TOP_prginsadd_r_i, 
		 TOP_prginsadd_r_ii, 
		 TOP_prginsadd_l1_r_i, 
		 TOP_prginsadd_l1_r_ii, 
		 TOP_prginspg_r_i, 
		 TOP_prginspg_r_ii, 
		 TOP_prginsset_r_i, 
		 TOP_prginsset_r_ii, 
		 TOP_prginsset_l1_r_i, 
		 TOP_prginsset_l1_r_ii, 
		 TOP_prgset_r_i, 
		 TOP_prgset_r_ii, 
		 TOP_prgset_l1_r_i, 
		 TOP_prgset_l1_r_ii, 
		 TOP_stbc_r_b_r_i, 
		 TOP_stbc_r_b_r_ii, 
		 TOP_stb_r_r_i, 
		 TOP_stb_r_r_ii, 
		 TOP_sthc_r_b_r_i, 
		 TOP_sthc_r_b_r_ii, 
		 TOP_sth_r_r_i, 
		 TOP_sth_r_r_ii, 
		 TOP_stlc_p_b_r_i, 
		 TOP_stlc_p_b_r_ii, 
		 TOP_stl_p_r_i, 
		 TOP_stl_p_r_ii, 
		 TOP_stwc_r_b_r_i, 
		 TOP_stwc_r_b_r_ii, 
		 TOP_stw_r_r_i, 
		 TOP_stw_r_r_ii, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  asmop = ISA_Property_Create ("asmop"); 
  Instruction_Group (asmop, 
		 TOP_asm_0_r_r_r, 
		 TOP_asm_10_r_r_r, 
		 TOP_asm_11_r_r_r, 
		 TOP_asm_12_r_r_r, 
		 TOP_asm_13_r_r_r, 
		 TOP_asm_14_r_r_r, 
		 TOP_asm_15_r_r_r, 
		 TOP_asm_16_i_r_r, 
		 TOP_asm_16_ii_r_r, 
		 TOP_asm_17_i_r_r, 
		 TOP_asm_17_ii_r_r, 
		 TOP_asm_18_i_r_r, 
		 TOP_asm_18_ii_r_r, 
		 TOP_asm_19_i_r_r, 
		 TOP_asm_19_ii_r_r, 
		 TOP_asm_1_r_r_r, 
		 TOP_asm_20_i_r_r, 
		 TOP_asm_20_ii_r_r, 
		 TOP_asm_21_i_r_r, 
		 TOP_asm_21_ii_r_r, 
		 TOP_asm_22_i_r_r, 
		 TOP_asm_22_ii_r_r, 
		 TOP_asm_23_i_r_r, 
		 TOP_asm_23_ii_r_r, 
		 TOP_asm_24_i_r_r, 
		 TOP_asm_24_ii_r_r, 
		 TOP_asm_25_i_r_r, 
		 TOP_asm_25_ii_r_r, 
		 TOP_asm_26_i_r_r, 
		 TOP_asm_26_ii_r_r, 
		 TOP_asm_27_i_r_r, 
		 TOP_asm_27_ii_r_r, 
		 TOP_asm_28_i_r_r, 
		 TOP_asm_28_ii_r_r, 
		 TOP_asm_29_i_r_r, 
		 TOP_asm_29_ii_r_r, 
		 TOP_asm_2_r_r_r, 
		 TOP_asm_30_i_r_r, 
		 TOP_asm_30_ii_r_r, 
		 TOP_asm_31_i_r_r, 
		 TOP_asm_31_ii_r_r, 
		 TOP_asm_3_r_r_r, 
		 TOP_asm_4_r_r_r, 
		 TOP_asm_5_r_r_r, 
		 TOP_asm_6_r_r_r, 
		 TOP_asm_7_r_r_r, 
		 TOP_asm_8_r_r_r, 
		 TOP_asm_9_r_r_r, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  atomicseq = ISA_Property_Create ("atomicseq"); 
  Instruction_Group (atomicseq, 
		 TOP_ldwl_r_r, 
		 TOP_stwl_r_r_b, 
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
		 TOP_dbgsbrk, 
		 TOP_dib, 
		 TOP_flushadd_r_i, 
		 TOP_flushadd_r_ii, 
		 TOP_flushadd_l1_r_i, 
		 TOP_flushadd_l1_r_ii, 
		 TOP_fwd_bar, 
		 TOP_invadd_r_i, 
		 TOP_invadd_r_ii, 
		 TOP_invadd_l1_r_i, 
		 TOP_invadd_l1_r_ii, 
		 TOP_ldwl_r_r, 
		 TOP_prgadd_r_i, 
		 TOP_prgadd_r_ii, 
		 TOP_prgadd_l1_r_i, 
		 TOP_prgadd_l1_r_ii, 
		 TOP_prgins, 
		 TOP_prginsadd_r_i, 
		 TOP_prginsadd_r_ii, 
		 TOP_prginsadd_l1_r_i, 
		 TOP_prginsadd_l1_r_ii, 
		 TOP_prginspg_r_i, 
		 TOP_prginspg_r_ii, 
		 TOP_prginsset_r_i, 
		 TOP_prginsset_r_ii, 
		 TOP_prginsset_l1_r_i, 
		 TOP_prginsset_l1_r_ii, 
		 TOP_prgset_r_i, 
		 TOP_prgset_r_ii, 
		 TOP_prgset_l1_r_i, 
		 TOP_prgset_l1_r_ii, 
		 TOP_pswclr_r, 
		 TOP_pswmask_i_r_r, 
		 TOP_pswmask_ii_r_r, 
		 TOP_pswset_r, 
		 TOP_pushregs, 
		 TOP_retention, 
		 TOP_sbrk_i, 
		 TOP_st240_sbrk_i, 
		 TOP_stwl_r_r_b, 
		 TOP_sync, 
		 TOP_syncins, 
		 TOP_syscall_i, 
		 TOP_st240_syscall_i, 
		 TOP_waitl, 
		 TOP_wmb, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  branch_predict = ISA_Property_Create ("branch_predict"); 
  Instruction_Group (branch_predict, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  cache = ISA_Property_Create ("cache"); 
  Instruction_Group (cache, 
		 TOP_flushadd_r_i, 
		 TOP_flushadd_r_ii, 
		 TOP_flushadd_l1_r_i, 
		 TOP_flushadd_l1_r_ii, 
		 TOP_invadd_r_i, 
		 TOP_invadd_r_ii, 
		 TOP_invadd_l1_r_i, 
		 TOP_invadd_l1_r_ii, 
		 TOP_pftc_r_i_b, 
		 TOP_pftc_r_ii_b, 
		 TOP_pft_r_i, 
		 TOP_pft_r_ii, 
		 TOP_prgadd_r_i, 
		 TOP_prgadd_r_ii, 
		 TOP_prgadd_l1_r_i, 
		 TOP_prgadd_l1_r_ii, 
		 TOP_prginsadd_r_i, 
		 TOP_prginsadd_r_ii, 
		 TOP_prginsadd_l1_r_i, 
		 TOP_prginsadd_l1_r_ii, 
		 TOP_prginspg_r_i, 
		 TOP_prginspg_r_ii, 
		 TOP_prginsset_r_i, 
		 TOP_prginsset_r_ii, 
		 TOP_prginsset_l1_r_i, 
		 TOP_prginsset_l1_r_ii, 
		 TOP_prgset_r_i, 
		 TOP_prgset_r_ii, 
		 TOP_prgset_l1_r_i, 
		 TOP_prgset_l1_r_ii, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  call = ISA_Property_Create ("call"); 
  Instruction_Group (call, 
		 TOP_call_i, 
		 TOP_icall, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  cmp = ISA_Property_Create ("cmp"); 
  Instruction_Group (cmp, 
		 TOP_andl_r_r_b, 
		 TOP_andl_i_r_b, 
		 TOP_andl_ii_r_b, 
		 TOP_andl_r_r_r, 
		 TOP_andl_i_r_r, 
		 TOP_andl_ii_r_r, 
		 TOP_cmpeqf_n_r_r_b, 
		 TOP_cmpeqf_n_r_r_r, 
		 TOP_cmpeq_r_r_b, 
		 TOP_cmpeq_i_r_b, 
		 TOP_cmpeq_ii_r_b, 
		 TOP_cmpeq_r_r_r, 
		 TOP_cmpeq_i_r_r, 
		 TOP_cmpeq_ii_r_r, 
		 TOP_cmpgef_n_r_r_b, 
		 TOP_cmpgef_n_r_r_r, 
		 TOP_cmpgeu_r_r_b, 
		 TOP_cmpgeu_i_r_b, 
		 TOP_cmpgeu_ii_r_b, 
		 TOP_cmpgeu_r_r_r, 
		 TOP_cmpgeu_i_r_r, 
		 TOP_cmpgeu_ii_r_r, 
		 TOP_cmpge_r_r_b, 
		 TOP_cmpge_i_r_b, 
		 TOP_cmpge_ii_r_b, 
		 TOP_cmpge_r_r_r, 
		 TOP_cmpge_i_r_r, 
		 TOP_cmpge_ii_r_r, 
		 TOP_cmpgtf_n_r_r_b, 
		 TOP_cmpgtf_n_r_r_r, 
		 TOP_cmpgtu_r_r_b, 
		 TOP_cmpgtu_i_r_b, 
		 TOP_cmpgtu_ii_r_b, 
		 TOP_cmpgtu_r_r_r, 
		 TOP_cmpgtu_i_r_r, 
		 TOP_cmpgtu_ii_r_r, 
		 TOP_cmpgt_r_r_b, 
		 TOP_cmpgt_i_r_b, 
		 TOP_cmpgt_ii_r_b, 
		 TOP_cmpgt_r_r_r, 
		 TOP_cmpgt_i_r_r, 
		 TOP_cmpgt_ii_r_r, 
		 TOP_cmplef_n_r_r_b, 
		 TOP_cmplef_n_r_r_r, 
		 TOP_cmpleu_r_r_b, 
		 TOP_cmpleu_i_r_b, 
		 TOP_cmpleu_ii_r_b, 
		 TOP_cmpleu_r_r_r, 
		 TOP_cmpleu_i_r_r, 
		 TOP_cmpleu_ii_r_r, 
		 TOP_cmple_r_r_b, 
		 TOP_cmple_i_r_b, 
		 TOP_cmple_ii_r_b, 
		 TOP_cmple_r_r_r, 
		 TOP_cmple_i_r_r, 
		 TOP_cmple_ii_r_r, 
		 TOP_cmpltf_n_r_r_b, 
		 TOP_cmpltf_n_r_r_r, 
		 TOP_cmpltu_r_r_b, 
		 TOP_cmpltu_i_r_b, 
		 TOP_cmpltu_ii_r_b, 
		 TOP_cmpltu_r_r_r, 
		 TOP_cmpltu_i_r_r, 
		 TOP_cmpltu_ii_r_r, 
		 TOP_cmplt_r_r_b, 
		 TOP_cmplt_i_r_b, 
		 TOP_cmplt_ii_r_b, 
		 TOP_cmplt_r_r_r, 
		 TOP_cmplt_i_r_r, 
		 TOP_cmplt_ii_r_r, 
		 TOP_cmpne_r_r_b, 
		 TOP_cmpne_i_r_b, 
		 TOP_cmpne_ii_r_b, 
		 TOP_cmpne_r_r_r, 
		 TOP_cmpne_i_r_r, 
		 TOP_cmpne_ii_r_r, 
		 TOP_nandl_r_r_b, 
		 TOP_nandl_i_r_b, 
		 TOP_nandl_ii_r_b, 
		 TOP_nandl_r_r_r, 
		 TOP_nandl_i_r_r, 
		 TOP_nandl_ii_r_r, 
		 TOP_norl_r_r_b, 
		 TOP_norl_i_r_b, 
		 TOP_norl_ii_r_b, 
		 TOP_norl_r_r_r, 
		 TOP_norl_i_r_r, 
		 TOP_norl_ii_r_r, 
		 TOP_orl_r_r_b, 
		 TOP_orl_i_r_b, 
		 TOP_orl_ii_r_b, 
		 TOP_orl_r_r_r, 
		 TOP_orl_i_r_r, 
		 TOP_orl_ii_r_r, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  compose = ISA_Property_Create ("compose"); 
  Instruction_Group (compose, 
		 TOP_composep, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  cond = ISA_Property_Create ("cond"); 
  Instruction_Group (cond, 
		 TOP_brf_i_b, 
		 TOP_br_i_b, 
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
		 TOP_ldbu_d_r_i_r, 
		 TOP_ldbu_d_r_ii_r, 
		 TOP_ldb_d_r_i_r, 
		 TOP_ldb_d_r_ii_r, 
		 TOP_ldhu_d_r_i_r, 
		 TOP_ldhu_d_r_ii_r, 
		 TOP_ldh_d_r_i_r, 
		 TOP_ldh_d_r_ii_r, 
		 TOP_ldw_d_r_i_r, 
		 TOP_ldw_d_r_ii_r, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  div = ISA_Property_Create ("div"); 
  Instruction_Group (div, 
		 TOP_divu_r_r_r, 
		 TOP_div_r_r_r, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  dummy = ISA_Property_Create ("dummy"); 
  Instruction_Group (dummy, 
		 TOP_intrncall, 
		 TOP_begin_pregtn, 
		 TOP_bwd_bar, 
		 TOP_COPY, 
		 TOP_end_pregtn, 
		 TOP_fwd_bar, 
		 TOP_ifixup, 
		 TOP_KILL, 
		 TOP_label, 
		 TOP_phi, 
		 TOP_psi, 
		 TOP_SIGMA, 
		 TOP_noop, 
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
		 TOP_addf_n_r_r_r, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  flop = ISA_Property_Create ("flop"); 
  Instruction_Group (flop, 
		 TOP_addf_n_r_r_r, 
		 TOP_cmpeqf_n_r_r_b, 
		 TOP_cmpeqf_n_r_r_r, 
		 TOP_cmpgef_n_r_r_b, 
		 TOP_cmpgef_n_r_r_r, 
		 TOP_cmpgtf_n_r_r_b, 
		 TOP_cmpgtf_n_r_r_r, 
		 TOP_cmplef_n_r_r_b, 
		 TOP_cmplef_n_r_r_r, 
		 TOP_cmpltf_n_r_r_b, 
		 TOP_cmpltf_n_r_r_r, 
		 TOP_mulf_n_r_r_r, 
		 TOP_subf_n_r_r_r, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  fmul = ISA_Property_Create ("fmul"); 
  Instruction_Group (fmul, 
		 TOP_mulf_n_r_r_r, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  fsub = ISA_Property_Create ("fsub"); 
  Instruction_Group (fsub, 
		 TOP_subf_n_r_r_r, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  guard_f = ISA_Property_Create ("guard_f"); 
  Instruction_Group (guard_f, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  guard_t = ISA_Property_Create ("guard_t"); 
  Instruction_Group (guard_t, 
		 TOP_ldbc_r_i_b_r, 
		 TOP_ldbc_r_ii_b_r, 
		 TOP_ldbuc_r_i_b_r, 
		 TOP_ldbuc_r_ii_b_r, 
		 TOP_ldhc_r_i_b_r, 
		 TOP_ldhc_r_ii_b_r, 
		 TOP_ldhuc_r_i_b_r, 
		 TOP_ldhuc_r_ii_b_r, 
		 TOP_ldlc_r_i_b_p, 
		 TOP_ldlc_r_ii_b_p, 
		 TOP_ldwc_r_i_b_r, 
		 TOP_ldwc_r_ii_b_r, 
		 TOP_movc, 
		 TOP_movcf, 
		 TOP_multi_ldlc_r_i_b_r, 
		 TOP_multi_ldlc_r_ii_b_r, 
		 TOP_multi_stlc_r_b_r_i, 
		 TOP_multi_stlc_r_b_r_ii, 
		 TOP_pftc_r_i_b, 
		 TOP_pftc_r_ii_b, 
		 TOP_stbc_r_b_r_i, 
		 TOP_stbc_r_b_r_ii, 
		 TOP_sthc_r_b_r_i, 
		 TOP_sthc_r_b_r_ii, 
		 TOP_stlc_p_b_r_i, 
		 TOP_stlc_p_b_r_ii, 
		 TOP_stwc_r_b_r_i, 
		 TOP_stwc_r_b_r_ii, 
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
		 TOP_addpc_i_r, 
		 TOP_addpc_ii_r, 
		 TOP_addso_r_r_r, 
		 TOP_adds_r_r_r, 
		 TOP_add_r_r_r, 
		 TOP_add_i_r_r, 
		 TOP_add_ii_r_r, 
		 TOP_andc_r_r_r, 
		 TOP_andc_i_r_r, 
		 TOP_andc_ii_r_r, 
		 TOP_andl_r_r_b, 
		 TOP_andl_i_r_b, 
		 TOP_andl_ii_r_b, 
		 TOP_andl_r_r_r, 
		 TOP_andl_i_r_r, 
		 TOP_andl_ii_r_r, 
		 TOP_and_r_r_r, 
		 TOP_and_i_r_r, 
		 TOP_and_ii_r_r, 
		 TOP_bswap_r_r, 
		 TOP_clz_r_r, 
		 TOP_cmpeq_r_r_b, 
		 TOP_cmpeq_i_r_b, 
		 TOP_cmpeq_ii_r_b, 
		 TOP_cmpeq_r_r_r, 
		 TOP_cmpeq_i_r_r, 
		 TOP_cmpeq_ii_r_r, 
		 TOP_cmpgeu_r_r_b, 
		 TOP_cmpgeu_i_r_b, 
		 TOP_cmpgeu_ii_r_b, 
		 TOP_cmpgeu_r_r_r, 
		 TOP_cmpgeu_i_r_r, 
		 TOP_cmpgeu_ii_r_r, 
		 TOP_cmpge_r_r_b, 
		 TOP_cmpge_i_r_b, 
		 TOP_cmpge_ii_r_b, 
		 TOP_cmpge_r_r_r, 
		 TOP_cmpge_i_r_r, 
		 TOP_cmpge_ii_r_r, 
		 TOP_cmpgtu_r_r_b, 
		 TOP_cmpgtu_i_r_b, 
		 TOP_cmpgtu_ii_r_b, 
		 TOP_cmpgtu_r_r_r, 
		 TOP_cmpgtu_i_r_r, 
		 TOP_cmpgtu_ii_r_r, 
		 TOP_cmpgt_r_r_b, 
		 TOP_cmpgt_i_r_b, 
		 TOP_cmpgt_ii_r_b, 
		 TOP_cmpgt_r_r_r, 
		 TOP_cmpgt_i_r_r, 
		 TOP_cmpgt_ii_r_r, 
		 TOP_cmpleu_r_r_b, 
		 TOP_cmpleu_i_r_b, 
		 TOP_cmpleu_ii_r_b, 
		 TOP_cmpleu_r_r_r, 
		 TOP_cmpleu_i_r_r, 
		 TOP_cmpleu_ii_r_r, 
		 TOP_cmple_r_r_b, 
		 TOP_cmple_i_r_b, 
		 TOP_cmple_ii_r_b, 
		 TOP_cmple_r_r_r, 
		 TOP_cmple_i_r_r, 
		 TOP_cmple_ii_r_r, 
		 TOP_cmpltu_r_r_b, 
		 TOP_cmpltu_i_r_b, 
		 TOP_cmpltu_ii_r_b, 
		 TOP_cmpltu_r_r_r, 
		 TOP_cmpltu_i_r_r, 
		 TOP_cmpltu_ii_r_r, 
		 TOP_cmplt_r_r_b, 
		 TOP_cmplt_i_r_b, 
		 TOP_cmplt_ii_r_b, 
		 TOP_cmplt_r_r_r, 
		 TOP_cmplt_i_r_r, 
		 TOP_cmplt_ii_r_r, 
		 TOP_cmpne_r_r_b, 
		 TOP_cmpne_i_r_b, 
		 TOP_cmpne_ii_r_b, 
		 TOP_cmpne_r_r_r, 
		 TOP_cmpne_i_r_r, 
		 TOP_cmpne_ii_r_r, 
		 TOP_divu_r_r_r, 
		 TOP_div_r_r_r, 
		 TOP_extractlu_i_r_r, 
		 TOP_extractlu_ii_r_r, 
		 TOP_extractl_i_r_r, 
		 TOP_extractl_ii_r_r, 
		 TOP_extractu_i_r_r, 
		 TOP_extractu_ii_r_r, 
		 TOP_extract_i_r_r, 
		 TOP_extract_ii_r_r, 
		 TOP_maxu_r_r_r, 
		 TOP_maxu_i_r_r, 
		 TOP_maxu_ii_r_r, 
		 TOP_max_r_r_r, 
		 TOP_max_i_r_r, 
		 TOP_max_ii_r_r, 
		 TOP_minu_r_r_r, 
		 TOP_minu_i_r_r, 
		 TOP_minu_ii_r_r, 
		 TOP_min_r_r_r, 
		 TOP_min_i_r_r, 
		 TOP_min_ii_r_r, 
		 TOP_mov_r_r, 
		 TOP_mov_i_r, 
		 TOP_mov_ii_r, 
		 TOP_mul32_r_r_r, 
		 TOP_mul32_i_r_r, 
		 TOP_mul32_ii_r_r, 
		 TOP_mul64hu_r_r_r, 
		 TOP_mul64hu_i_r_r, 
		 TOP_mul64hu_ii_r_r, 
		 TOP_mul64h_r_r_r, 
		 TOP_mul64h_i_r_r, 
		 TOP_mul64h_ii_r_r, 
		 TOP_mulfrac_r_r_r, 
		 TOP_mulfrac_i_r_r, 
		 TOP_mulfrac_ii_r_r, 
		 TOP_mulhhs_r_r_r, 
		 TOP_mulhhs_i_r_r, 
		 TOP_mulhhs_ii_r_r, 
		 TOP_mulhhu_r_r_r, 
		 TOP_mulhhu_i_r_r, 
		 TOP_mulhhu_ii_r_r, 
		 TOP_mulhh_r_r_r, 
		 TOP_mulhh_i_r_r, 
		 TOP_mulhh_ii_r_r, 
		 TOP_mulhs_r_r_r, 
		 TOP_mulhs_i_r_r, 
		 TOP_mulhs_ii_r_r, 
		 TOP_mulhu_r_r_r, 
		 TOP_mulhu_i_r_r, 
		 TOP_mulhu_ii_r_r, 
		 TOP_mulh_r_r_r, 
		 TOP_mulh_i_r_r, 
		 TOP_mulh_ii_r_r, 
		 TOP_mullhus_r_r_r, 
		 TOP_mullhus_i_r_r, 
		 TOP_mullhus_ii_r_r, 
		 TOP_mullhu_r_r_r, 
		 TOP_mullhu_i_r_r, 
		 TOP_mullhu_ii_r_r, 
		 TOP_mullh_r_r_r, 
		 TOP_mullh_i_r_r, 
		 TOP_mullh_ii_r_r, 
		 TOP_mulllu_r_r_r, 
		 TOP_mulllu_i_r_r, 
		 TOP_mulllu_ii_r_r, 
		 TOP_mulll_r_r_r, 
		 TOP_mulll_i_r_r, 
		 TOP_mulll_ii_r_r, 
		 TOP_mullu_r_r_r, 
		 TOP_mullu_i_r_r, 
		 TOP_mullu_ii_r_r, 
		 TOP_mull_r_r_r, 
		 TOP_mull_i_r_r, 
		 TOP_mull_ii_r_r, 
		 TOP_st240_mull_ii_r_r, 
		 TOP_nandl_r_r_b, 
		 TOP_nandl_i_r_b, 
		 TOP_nandl_ii_r_b, 
		 TOP_nandl_r_r_r, 
		 TOP_nandl_i_r_r, 
		 TOP_nandl_ii_r_r, 
		 TOP_norl_r_r_b, 
		 TOP_norl_i_r_b, 
		 TOP_norl_ii_r_b, 
		 TOP_norl_r_r_r, 
		 TOP_norl_i_r_r, 
		 TOP_norl_ii_r_r, 
		 TOP_orc_r_r_r, 
		 TOP_orc_i_r_r, 
		 TOP_orc_ii_r_r, 
		 TOP_orl_r_r_b, 
		 TOP_orl_i_r_b, 
		 TOP_orl_ii_r_b, 
		 TOP_orl_r_r_r, 
		 TOP_orl_i_r_r, 
		 TOP_orl_ii_r_r, 
		 TOP_or_r_r_r, 
		 TOP_or_i_r_r, 
		 TOP_or_ii_r_r, 
		 TOP_remu_r_r_r, 
		 TOP_rem_r_r_r, 
		 TOP_rotl_r_r_r, 
		 TOP_rotl_i_r_r, 
		 TOP_rotl_ii_r_r, 
		 TOP_satso_r_r, 
		 TOP_sats_r_r, 
		 TOP_sh1addso_r_r_r, 
		 TOP_sh1adds_r_r_r, 
		 TOP_sh1add_r_r_r, 
		 TOP_sh1add_i_r_r, 
		 TOP_sh1add_ii_r_r, 
		 TOP_sh1subso_r_r_r, 
		 TOP_sh1subs_r_r_r, 
		 TOP_sh2add_r_r_r, 
		 TOP_sh2add_i_r_r, 
		 TOP_sh2add_ii_r_r, 
		 TOP_sh3add_r_r_r, 
		 TOP_sh3add_i_r_r, 
		 TOP_sh3add_ii_r_r, 
		 TOP_sh4add_r_r_r, 
		 TOP_sh4add_i_r_r, 
		 TOP_sh4add_ii_r_r, 
		 TOP_shlso_r_r_r, 
		 TOP_shlso_i_r_r, 
		 TOP_shlso_ii_r_r, 
		 TOP_shls_r_r_r, 
		 TOP_shls_i_r_r, 
		 TOP_shls_ii_r_r, 
		 TOP_shl_r_r_r, 
		 TOP_shl_i_r_r, 
		 TOP_shl_ii_r_r, 
		 TOP_shrrnp_i_r_r, 
		 TOP_shrrnp_ii_r_r, 
		 TOP_shru_r_r_r, 
		 TOP_shru_i_r_r, 
		 TOP_shru_ii_r_r, 
		 TOP_shr_r_r_r, 
		 TOP_shr_i_r_r, 
		 TOP_shr_ii_r_r, 
		 TOP_slctf_r_r_b_r, 
		 TOP_slctf_i_r_b_r, 
		 TOP_slctf_ii_r_b_r, 
		 TOP_slct_r_r_b_r, 
		 TOP_slct_i_r_b_r, 
		 TOP_slct_ii_r_b_r, 
		 TOP_st240_slct_r_r_b_r, 
		 TOP_subso_r_r_r, 
		 TOP_subs_r_r_r, 
		 TOP_sub_r_r_r, 
		 TOP_sub_r_i_r, 
		 TOP_sub_r_ii_r, 
		 TOP_sxtb_r_r, 
		 TOP_sxth_r_r, 
		 TOP_sxt_r_r_r, 
		 TOP_sxt_i_r_r, 
		 TOP_sxt_ii_r_r, 
		 TOP_xor_r_r_r, 
		 TOP_xor_i_r_r, 
		 TOP_xor_ii_r_r, 
		 TOP_zxtb_r_r, 
		 TOP_zxth_r_r, 
		 TOP_zxt_r_r_r, 
		 TOP_zxt_i_r_r, 
		 TOP_zxt_ii_r_r, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  jump = ISA_Property_Create ("jump"); 
  Instruction_Group (jump, 
		 TOP_goto_i, 
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
		 TOP_ldbc_r_i_b_r, 
		 TOP_ldbc_r_ii_b_r, 
		 TOP_ldbuc_r_i_b_r, 
		 TOP_ldbuc_r_ii_b_r, 
		 TOP_ldbu_r_i_r, 
		 TOP_ldbu_r_ii_r, 
		 TOP_ldbu_d_r_i_r, 
		 TOP_ldbu_d_r_ii_r, 
		 TOP_ldb_r_i_r, 
		 TOP_ldb_r_ii_r, 
		 TOP_ldb_d_r_i_r, 
		 TOP_ldb_d_r_ii_r, 
		 TOP_ldhc_r_i_b_r, 
		 TOP_ldhc_r_ii_b_r, 
		 TOP_ldhuc_r_i_b_r, 
		 TOP_ldhuc_r_ii_b_r, 
		 TOP_ldhu_r_i_r, 
		 TOP_ldhu_r_ii_r, 
		 TOP_ldhu_d_r_i_r, 
		 TOP_ldhu_d_r_ii_r, 
		 TOP_ldh_r_i_r, 
		 TOP_ldh_r_ii_r, 
		 TOP_ldh_d_r_i_r, 
		 TOP_ldh_d_r_ii_r, 
		 TOP_ldlc_r_i_b_p, 
		 TOP_ldlc_r_ii_b_p, 
		 TOP_ldl_r_i_p, 
		 TOP_ldl_r_ii_p, 
		 TOP_ldwc_r_i_b_r, 
		 TOP_ldwc_r_ii_b_r, 
		 TOP_ldw_r_i_r, 
		 TOP_ldw_r_ii_r, 
		 TOP_ldw_d_r_i_r, 
		 TOP_ldw_d_r_ii_r, 
		 TOP_multi_ldlc_r_i_b_r, 
		 TOP_multi_ldlc_r_ii_b_r, 
		 TOP_multi_ldl_r_i_r, 
		 TOP_multi_ldl_r_ii_r, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  madd = ISA_Property_Create ("madd"); 
  Instruction_Group (madd, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  max = ISA_Property_Create ("max"); 
  Instruction_Group (max, 
		 TOP_maxu_r_r_r, 
		 TOP_maxu_i_r_r, 
		 TOP_maxu_ii_r_r, 
		 TOP_max_r_r_r, 
		 TOP_max_i_r_r, 
		 TOP_max_ii_r_r, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  mem_fill_type = ISA_Property_Create ("mem_fill_type"); 
  Instruction_Group (mem_fill_type, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  min = ISA_Property_Create ("min"); 
  Instruction_Group (min, 
		 TOP_minu_r_r_r, 
		 TOP_minu_i_r_r, 
		 TOP_minu_ii_r_r, 
		 TOP_min_r_r_r, 
		 TOP_min_i_r_r, 
		 TOP_min_ii_r_r, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  move = ISA_Property_Create ("move"); 
  Instruction_Group (move, 
		 TOP_movp, 
		 TOP_mov_r_r, 
		 TOP_mov_i_r, 
		 TOP_mov_ii_r, 
		 TOP_mov_b_b, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  mul = ISA_Property_Create ("mul"); 
  Instruction_Group (mul, 
		 TOP_mul32_r_r_r, 
		 TOP_mul32_i_r_r, 
		 TOP_mul32_ii_r_r, 
		 TOP_mulllu_r_r_r, 
		 TOP_mulllu_i_r_r, 
		 TOP_mulllu_ii_r_r, 
		 TOP_mulll_r_r_r, 
		 TOP_mulll_i_r_r, 
		 TOP_mulll_ii_r_r, 
		 TOP_mullu_r_r_r, 
		 TOP_mullu_i_r_r, 
		 TOP_mullu_ii_r_r, 
		 TOP_mull_r_r_r, 
		 TOP_mull_i_r_r, 
		 TOP_mull_ii_r_r, 
		 TOP_st240_mull_ii_r_r, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  multi = ISA_Property_Create ("multi"); 
  Instruction_Group (multi, 
		 TOP_multi_ldlc_r_i_b_r, 
		 TOP_multi_ldlc_r_ii_b_r, 
		 TOP_multi_ldl_r_i_r, 
		 TOP_multi_ldl_r_ii_r, 
		 TOP_multi_stlc_r_b_r_i, 
		 TOP_multi_stlc_r_b_r_ii, 
		 TOP_multi_stl_r_r_i, 
		 TOP_multi_stl_r_r_ii, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  noop = ISA_Property_Create ("noop"); 
  Instruction_Group (noop, 
		 TOP_nop, 
		 TOP_noop, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  packedop = ISA_Property_Create ("packedop"); 
  Instruction_Group (packedop, 
		 TOP_abss_ph_r_r, 
		 TOP_absubu_pb_r_r_r, 
		 TOP_adds_ph_r_r_r, 
		 TOP_add_ph_r_r_r, 
		 TOP_avg4u_pb_r_r_b_r, 
		 TOP_avgu_pb_r_r_b_r, 
		 TOP_cmpeq_pb_r_r_r, 
		 TOP_cmpeq_pb_r_r_b, 
		 TOP_cmpeq_ph_r_r_r, 
		 TOP_cmpeq_ph_r_r_b, 
		 TOP_cmpgtu_pb_r_r_r, 
		 TOP_cmpgtu_pb_r_r_b, 
		 TOP_cmpgt_ph_r_r_r, 
		 TOP_cmpgt_ph_r_r_b, 
		 TOP_ext1_pb_r_r_r, 
		 TOP_ext2_pb_r_r_r, 
		 TOP_ext3_pb_r_r_r, 
		 TOP_extl_pb_r_r_b_r, 
		 TOP_extr_pb_r_r_b_r, 
		 TOP_max_ph_r_r_r, 
		 TOP_min_ph_r_r_r, 
		 TOP_muladdus_pb_r_r_r, 
		 TOP_muladd_ph_r_r_r, 
		 TOP_mulfracadds_ph_r_r_r, 
		 TOP_mulfracrm_ph_r_r_r, 
		 TOP_mulfracrne_ph_r_r_r, 
		 TOP_mul_ph_r_r_r, 
		 TOP_packrnp_phh_r_r_r, 
		 TOP_packsu_pb_r_r_r, 
		 TOP_packs_ph_r_r_r, 
		 TOP_pack_pb_r_r_r, 
		 TOP_pack_ph_r_r_r, 
		 TOP_perm_pb_r_r_r, 
		 TOP_perm_pb_i_r_r, 
		 TOP_perm_pb_ii_r_r, 
		 TOP_sadu_pb_r_r_r, 
		 TOP_shls_ph_r_r_r, 
		 TOP_shls_ph_i_r_r, 
		 TOP_shls_ph_ii_r_r, 
		 TOP_shl_ph_r_r_r, 
		 TOP_shl_ph_i_r_r, 
		 TOP_shl_ph_ii_r_r, 
		 TOP_shrrne_ph_r_r_r, 
		 TOP_shrrne_ph_i_r_r, 
		 TOP_shrrne_ph_ii_r_r, 
		 TOP_shrrnp_ph_r_r_r, 
		 TOP_shrrnp_ph_i_r_r, 
		 TOP_shrrnp_ph_ii_r_r, 
		 TOP_shr_ph_r_r_r, 
		 TOP_shr_ph_i_r_r, 
		 TOP_shr_ph_ii_r_r, 
		 TOP_shuffeve_pb_r_r_r, 
		 TOP_shuffodd_pb_r_r_r, 
		 TOP_shuff_pbh_r_r_r, 
		 TOP_shuff_pbl_r_r_r, 
		 TOP_shuff_phh_r_r_r, 
		 TOP_shuff_phl_r_r_r, 
		 TOP_slctf_pb_r_r_b_r, 
		 TOP_slctf_pb_i_r_b_r, 
		 TOP_slctf_pb_ii_r_b_r, 
		 TOP_slct_pb_r_r_b_r, 
		 TOP_slct_pb_i_r_b_r, 
		 TOP_slct_pb_ii_r_b_r, 
		 TOP_subs_ph_r_r_r, 
		 TOP_sub_ph_r_r_r, 
		 TOP_unpacku_pbh_r_r, 
		 TOP_unpacku_pbl_r_r, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  prefetch = ISA_Property_Create ("prefetch"); 
  Instruction_Group (prefetch, 
		 TOP_pftc_r_i_b, 
		 TOP_pftc_r_ii_b, 
		 TOP_pft_r_i, 
		 TOP_pft_r_ii, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  refs_fcr = ISA_Property_Create ("refs_fcr"); 
  Instruction_Group (refs_fcr, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  rem = ISA_Property_Create ("rem"); 
  Instruction_Group (rem, 
		 TOP_remu_r_r_r, 
		 TOP_rem_r_r_r, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  select = ISA_Property_Create ("select"); 
  Instruction_Group (select, 
		 TOP_slctf_r_r_b_r, 
		 TOP_slctf_i_r_b_r, 
		 TOP_slctf_ii_r_b_r, 
		 TOP_slct_r_r_b_r, 
		 TOP_slct_i_r_b_r, 
		 TOP_slct_ii_r_b_r, 
		 TOP_st240_slct_r_r_b_r, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  sext = ISA_Property_Create ("sext"); 
  Instruction_Group (sext, 
		 TOP_sxtb_r_r, 
		 TOP_sxth_r_r, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  shl = ISA_Property_Create ("shl"); 
  Instruction_Group (shl, 
		 TOP_shl_r_r_r, 
		 TOP_shl_i_r_r, 
		 TOP_shl_ii_r_r, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  shr = ISA_Property_Create ("shr"); 
  Instruction_Group (shr, 
		 TOP_shr_r_r_r, 
		 TOP_shr_i_r_r, 
		 TOP_shr_ii_r_r, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  shru = ISA_Property_Create ("shru"); 
  Instruction_Group (shru, 
		 TOP_shru_r_r_r, 
		 TOP_shru_i_r_r, 
		 TOP_shru_ii_r_r, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  side_effects = ISA_Property_Create ("side_effects"); 
  Instruction_Group (side_effects, 
		 TOP_dib, 
		 TOP_flushadd_r_i, 
		 TOP_flushadd_r_ii, 
		 TOP_flushadd_l1_r_i, 
		 TOP_flushadd_l1_r_ii, 
		 TOP_invadd_r_i, 
		 TOP_invadd_r_ii, 
		 TOP_invadd_l1_r_i, 
		 TOP_invadd_l1_r_ii, 
		 TOP_ldwl_r_r, 
		 TOP_prgadd_r_i, 
		 TOP_prgadd_r_ii, 
		 TOP_prgadd_l1_r_i, 
		 TOP_prgadd_l1_r_ii, 
		 TOP_prgins, 
		 TOP_prginsadd_r_i, 
		 TOP_prginsadd_r_ii, 
		 TOP_prginsadd_l1_r_i, 
		 TOP_prginsadd_l1_r_ii, 
		 TOP_prginspg_r_i, 
		 TOP_prginspg_r_ii, 
		 TOP_prginsset_r_i, 
		 TOP_prginsset_r_ii, 
		 TOP_prginsset_l1_r_i, 
		 TOP_prginsset_l1_r_ii, 
		 TOP_prgset_r_i, 
		 TOP_prgset_r_ii, 
		 TOP_prgset_l1_r_i, 
		 TOP_prgset_l1_r_ii, 
		 TOP_pswclr_r, 
		 TOP_pswmask_i_r_r, 
		 TOP_pswmask_ii_r_r, 
		 TOP_pswset_r, 
		 TOP_retention, 
		 TOP_stwl_r_r_b, 
		 TOP_sync, 
		 TOP_syncins, 
		 TOP_syscall_i, 
		 TOP_st240_syscall_i, 
		 TOP_waitl, 
		 TOP_wmb, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  simulated = ISA_Property_Create ("simulated"); 
  Instruction_Group (simulated, 
		 TOP_spadjust, 
		 TOP_composep, 
		 TOP_extractp, 
		 TOP_getpc, 
		 TOP_asm, 
		 TOP_movc, 
		 TOP_movcf, 
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
		 TOP_multi_stlc_r_b_r_i, 
		 TOP_multi_stlc_r_b_r_ii, 
		 TOP_multi_stl_r_r_i, 
		 TOP_multi_stl_r_r_ii, 
		 TOP_stbc_r_b_r_i, 
		 TOP_stbc_r_b_r_ii, 
		 TOP_stb_r_r_i, 
		 TOP_stb_r_r_ii, 
		 TOP_sthc_r_b_r_i, 
		 TOP_sthc_r_b_r_ii, 
		 TOP_sth_r_r_i, 
		 TOP_sth_r_r_ii, 
		 TOP_stlc_p_b_r_i, 
		 TOP_stlc_p_b_r_ii, 
		 TOP_stl_p_r_i, 
		 TOP_stl_p_r_ii, 
		 TOP_stwc_r_b_r_i, 
		 TOP_stwc_r_b_r_ii, 
		 TOP_stw_r_r_i, 
		 TOP_stw_r_r_ii, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  sub = ISA_Property_Create ("sub"); 
  Instruction_Group (sub, 
		 TOP_sub_r_r_r, 
		 TOP_sub_r_i_r, 
		 TOP_sub_r_ii_r, 
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
		 TOP_cmpgeu_r_r_b, 
		 TOP_cmpgeu_i_r_b, 
		 TOP_cmpgeu_ii_r_b, 
		 TOP_cmpgeu_r_r_r, 
		 TOP_cmpgeu_i_r_r, 
		 TOP_cmpgeu_ii_r_r, 
		 TOP_cmpgtu_r_r_b, 
		 TOP_cmpgtu_i_r_b, 
		 TOP_cmpgtu_ii_r_b, 
		 TOP_cmpgtu_r_r_r, 
		 TOP_cmpgtu_i_r_r, 
		 TOP_cmpgtu_ii_r_r, 
		 TOP_cmpleu_r_r_b, 
		 TOP_cmpleu_i_r_b, 
		 TOP_cmpleu_ii_r_b, 
		 TOP_cmpleu_r_r_r, 
		 TOP_cmpleu_i_r_r, 
		 TOP_cmpleu_ii_r_r, 
		 TOP_cmpltu_r_r_b, 
		 TOP_cmpltu_i_r_b, 
		 TOP_cmpltu_ii_r_b, 
		 TOP_cmpltu_r_r_r, 
		 TOP_cmpltu_i_r_r, 
		 TOP_cmpltu_ii_r_r, 
		 TOP_divu_r_r_r, 
		 TOP_extractlu_i_r_r, 
		 TOP_extractlu_ii_r_r, 
		 TOP_extractu_i_r_r, 
		 TOP_extractu_ii_r_r, 
		 TOP_ldbuc_r_i_b_r, 
		 TOP_ldbuc_r_ii_b_r, 
		 TOP_ldbu_r_i_r, 
		 TOP_ldbu_r_ii_r, 
		 TOP_ldbu_d_r_i_r, 
		 TOP_ldbu_d_r_ii_r, 
		 TOP_ldhuc_r_i_b_r, 
		 TOP_ldhuc_r_ii_b_r, 
		 TOP_ldhu_r_i_r, 
		 TOP_ldhu_r_ii_r, 
		 TOP_ldhu_d_r_i_r, 
		 TOP_ldhu_d_r_ii_r, 
		 TOP_maxu_r_r_r, 
		 TOP_maxu_i_r_r, 
		 TOP_maxu_ii_r_r, 
		 TOP_minu_r_r_r, 
		 TOP_minu_i_r_r, 
		 TOP_minu_ii_r_r, 
		 TOP_mul64hu_r_r_r, 
		 TOP_mul64hu_i_r_r, 
		 TOP_mul64hu_ii_r_r, 
		 TOP_mulhhu_r_r_r, 
		 TOP_mulhhu_i_r_r, 
		 TOP_mulhhu_ii_r_r, 
		 TOP_mulhu_r_r_r, 
		 TOP_mulhu_i_r_r, 
		 TOP_mulhu_ii_r_r, 
		 TOP_mullhu_r_r_r, 
		 TOP_mullhu_i_r_r, 
		 TOP_mullhu_ii_r_r, 
		 TOP_mulllu_r_r_r, 
		 TOP_mulllu_i_r_r, 
		 TOP_mulllu_ii_r_r, 
		 TOP_mullu_r_r_r, 
		 TOP_mullu_i_r_r, 
		 TOP_mullu_ii_r_r, 
		 TOP_remu_r_r_r, 
		 TOP_shru_r_r_r, 
		 TOP_shru_i_r_r, 
		 TOP_shru_ii_r_r, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  var_opnds = ISA_Property_Create ("var_opnds"); 
  Instruction_Group (var_opnds, 
		 TOP_intrncall, 
		 TOP_asm, 
		 TOP_KILL, 
		 TOP_phi, 
		 TOP_psi, 
		 TOP_pushregs, 
		 TOP_SIGMA, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  widemove = ISA_Property_Create ("widemove"); 
  Instruction_Group (widemove, 
		 TOP_movp, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  xfer = ISA_Property_Create ("xfer"); 
  Instruction_Group (xfer, 
		 TOP_brf_i_b, 
		 TOP_br_i_b, 
		 TOP_call_i, 
		 TOP_goto_i, 
		 TOP_icall, 
		 TOP_igoto, 
		 TOP_return, 
		 TOP_rfi, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  xmove = ISA_Property_Create ("xmove"); 
  Instruction_Group (xmove, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  zext = ISA_Property_Create ("zext"); 
  Instruction_Group (zext, 
		 TOP_zxtb_r_r, 
		 TOP_zxth_r_r, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  /*         Memory Access Size 1          */ 
  /* ====================================== */ 
  ISA_Memory_Access (1, 
		 TOP_ldbc_r_i_b_r, 
		 TOP_ldbc_r_ii_b_r, 
		 TOP_ldbuc_r_i_b_r, 
		 TOP_ldbuc_r_ii_b_r, 
		 TOP_ldbu_r_i_r, 
		 TOP_ldbu_r_ii_r, 
		 TOP_ldbu_d_r_i_r, 
		 TOP_ldbu_d_r_ii_r, 
		 TOP_ldb_r_i_r, 
		 TOP_ldb_r_ii_r, 
		 TOP_ldb_d_r_i_r, 
		 TOP_ldb_d_r_ii_r, 
		 TOP_stbc_r_b_r_i, 
		 TOP_stbc_r_b_r_ii, 
		 TOP_stb_r_r_i, 
		 TOP_stb_r_r_ii, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  /*         Memory Access Size 2          */ 
  /* ====================================== */ 
  ISA_Memory_Access (2, 
		 TOP_ldhc_r_i_b_r, 
		 TOP_ldhc_r_ii_b_r, 
		 TOP_ldhuc_r_i_b_r, 
		 TOP_ldhuc_r_ii_b_r, 
		 TOP_ldhu_r_i_r, 
		 TOP_ldhu_r_ii_r, 
		 TOP_ldhu_d_r_i_r, 
		 TOP_ldhu_d_r_ii_r, 
		 TOP_ldh_r_i_r, 
		 TOP_ldh_r_ii_r, 
		 TOP_ldh_d_r_i_r, 
		 TOP_ldh_d_r_ii_r, 
		 TOP_sthc_r_b_r_i, 
		 TOP_sthc_r_b_r_ii, 
		 TOP_sth_r_r_i, 
		 TOP_sth_r_r_ii, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  /*         Memory Access Size 4          */ 
  /* ====================================== */ 
  ISA_Memory_Access (4, 
		 TOP_ldwc_r_i_b_r, 
		 TOP_ldwc_r_ii_b_r, 
		 TOP_ldw_r_i_r, 
		 TOP_ldw_r_ii_r, 
		 TOP_ldw_d_r_i_r, 
		 TOP_ldw_d_r_ii_r, 
		 TOP_stwc_r_b_r_i, 
		 TOP_stwc_r_b_r_ii, 
		 TOP_stw_r_r_i, 
		 TOP_stw_r_r_ii, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  /*         Memory Access Size 8          */ 
  /* ====================================== */ 
  ISA_Memory_Access (8, 
		 TOP_ldlc_r_i_b_p, 
		 TOP_ldlc_r_ii_b_p, 
		 TOP_ldl_r_i_p, 
		 TOP_ldl_r_ii_p, 
		 TOP_multi_ldlc_r_i_b_r, 
		 TOP_multi_ldlc_r_ii_b_r, 
		 TOP_multi_ldl_r_i_r, 
		 TOP_multi_ldl_r_ii_r, 
		 TOP_multi_stlc_r_b_r_i, 
		 TOP_multi_stlc_r_b_r_ii, 
		 TOP_multi_stl_r_r_i, 
		 TOP_multi_stl_r_r_ii, 
		 TOP_stlc_p_b_r_i, 
		 TOP_stlc_p_b_r_ii, 
		 TOP_stl_p_r_i, 
		 TOP_stl_p_r_ii, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  /*         Memory Alignment 1          */ 
  /* ====================================== */ 
  ISA_Memory_Alignment (1, 
		 TOP_ldbc_r_i_b_r, 
		 TOP_ldbc_r_ii_b_r, 
		 TOP_ldbuc_r_i_b_r, 
		 TOP_ldbuc_r_ii_b_r, 
		 TOP_ldbu_r_i_r, 
		 TOP_ldbu_r_ii_r, 
		 TOP_ldbu_d_r_i_r, 
		 TOP_ldbu_d_r_ii_r, 
		 TOP_ldb_r_i_r, 
		 TOP_ldb_r_ii_r, 
		 TOP_ldb_d_r_i_r, 
		 TOP_ldb_d_r_ii_r, 
		 TOP_stbc_r_b_r_i, 
		 TOP_stbc_r_b_r_ii, 
		 TOP_stb_r_r_i, 
		 TOP_stb_r_r_ii, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  /*         Memory Alignment 2          */ 
  /* ====================================== */ 
  ISA_Memory_Alignment (2, 
		 TOP_ldhc_r_i_b_r, 
		 TOP_ldhc_r_ii_b_r, 
		 TOP_ldhuc_r_i_b_r, 
		 TOP_ldhuc_r_ii_b_r, 
		 TOP_ldhu_r_i_r, 
		 TOP_ldhu_r_ii_r, 
		 TOP_ldhu_d_r_i_r, 
		 TOP_ldhu_d_r_ii_r, 
		 TOP_ldh_r_i_r, 
		 TOP_ldh_r_ii_r, 
		 TOP_ldh_d_r_i_r, 
		 TOP_ldh_d_r_ii_r, 
		 TOP_sthc_r_b_r_i, 
		 TOP_sthc_r_b_r_ii, 
		 TOP_sth_r_r_i, 
		 TOP_sth_r_r_ii, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  /*         Memory Alignment 4          */ 
  /* ====================================== */ 
  ISA_Memory_Alignment (4, 
		 TOP_ldwc_r_i_b_r, 
		 TOP_ldwc_r_ii_b_r, 
		 TOP_ldw_r_i_r, 
		 TOP_ldw_r_ii_r, 
		 TOP_ldw_d_r_i_r, 
		 TOP_ldw_d_r_ii_r, 
		 TOP_stwc_r_b_r_i, 
		 TOP_stwc_r_b_r_ii, 
		 TOP_stw_r_r_i, 
		 TOP_stw_r_r_ii, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  /*         Memory Alignment 8          */ 
  /* ====================================== */ 
  ISA_Memory_Alignment (8, 
		 TOP_ldlc_r_i_b_p, 
		 TOP_ldlc_r_ii_b_p, 
		 TOP_ldl_r_i_p, 
		 TOP_ldl_r_ii_p, 
		 TOP_multi_ldlc_r_i_b_r, 
		 TOP_multi_ldlc_r_ii_b_r, 
		 TOP_multi_ldl_r_i_r, 
		 TOP_multi_ldl_r_ii_r, 
		 TOP_multi_stlc_r_b_r_i, 
		 TOP_multi_stlc_r_b_r_ii, 
		 TOP_multi_stl_r_r_i, 
		 TOP_multi_stl_r_r_ii, 
		 TOP_stlc_p_b_r_i, 
		 TOP_stlc_p_b_r_ii, 
		 TOP_stl_p_r_i, 
		 TOP_stl_p_r_ii, 
		 TOP_UNDEFINED); 

  ISA_Properties_End();
  return 0;
}
