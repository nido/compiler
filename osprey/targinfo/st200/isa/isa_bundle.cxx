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

//  
//  Generate ISA bundle information 
/////////////////////////////////////// 

#include <stddef.h> 
#include "topcode.h" 
#include "isa_bundle_gen.h" 

main() 
{ 
  ISA_Bundle_Begin("st200", 128);

  ISA_Bundle_Pack_Create(ISA_Bundle_Pack_Little_Endian);
  Pack_Slot(0, 0, 0, 32);
  Pack_Slot(1, 0, 32, 32);
  Pack_Slot(2, 0, 64, 32);
  Pack_Slot(3, 0, 96, 32);

 /* ===== Specification for ALL_Unit Type ===== */ 
  ISA_EXEC_UNIT_TYPE ALL_Unit = ISA_Exec_Unit_Type_Create("ALL_Unit", NULL); 
  Instruction_Exec_Unit_Group(ALL_Unit, 
		 TOP_prgins, 
		 TOP_syscall_ib, 
		 TOP_syscall_i, 
		 TOP_UNDEFINED); 

 /* ===== Specification for ALUX_Unit Type ===== */ 
  ISA_EXEC_UNIT_TYPE ALUX_Unit = ISA_Exec_Unit_Type_Create("ALUX_Unit", NULL); 
  Instruction_Exec_Unit_Group(ALUX_Unit, 
		 TOP_addpc_ii, 
		 TOP_add_ii, 
		 TOP_andc_ii, 
		 TOP_andl_ii_b, 
		 TOP_andl_ii_r, 
		 TOP_and_ii, 
		 TOP_asm_16_ii, 
		 TOP_asm_17_ii, 
		 TOP_asm_18_ii, 
		 TOP_asm_19_ii, 
		 TOP_asm_20_ii, 
		 TOP_asm_21_ii, 
		 TOP_asm_22_ii, 
		 TOP_asm_23_ii, 
		 TOP_asm_24_ii, 
		 TOP_asm_25_ii, 
		 TOP_asm_26_ii, 
		 TOP_asm_27_ii, 
		 TOP_asm_28_ii, 
		 TOP_asm_29_ii, 
		 TOP_asm_30_ii, 
		 TOP_asm_31_ii, 
		 TOP_cmpeq_ii_b, 
		 TOP_cmpeq_ii_r, 
		 TOP_cmpgeu_ii_b, 
		 TOP_cmpgeu_ii_r, 
		 TOP_cmpge_ii_b, 
		 TOP_cmpge_ii_r, 
		 TOP_cmpgtu_ii_b, 
		 TOP_cmpgtu_ii_r, 
		 TOP_cmpgt_ii_b, 
		 TOP_cmpgt_ii_r, 
		 TOP_cmpleu_ii_b, 
		 TOP_cmpleu_ii_r, 
		 TOP_cmple_ii_b, 
		 TOP_cmple_ii_r, 
		 TOP_cmpltu_ii_b, 
		 TOP_cmpltu_ii_r, 
		 TOP_cmplt_ii_b, 
		 TOP_cmplt_ii_r, 
		 TOP_cmpne_ii_b, 
		 TOP_cmpne_ii_r, 
		 TOP_ldbc_ii, 
		 TOP_ldbuc_ii, 
		 TOP_ldbu_d_ii, 
		 TOP_ldbu_ii, 
		 TOP_ldb_d_ii, 
		 TOP_ldb_ii, 
		 TOP_ldhc_ii, 
		 TOP_ldhuc_ii, 
		 TOP_ldhu_d_ii, 
		 TOP_ldhu_ii, 
		 TOP_ldh_d_ii, 
		 TOP_ldh_ii, 
		 TOP_ldpc_ii, 
		 TOP_ldp_ii, 
		 TOP_ldwc_ii, 
		 TOP_ldw_d_ii, 
		 TOP_ldw_ii, 
		 TOP_maxu_ii, 
		 TOP_max_ii, 
		 TOP_minu_ii, 
		 TOP_min_ii, 
		 TOP_mov_ii, 
		 TOP_mul32_ii, 
		 TOP_mul64hu_ii, 
		 TOP_mul64h_ii, 
		 TOP_mulfrac_ii, 
		 TOP_mulhhs_ii, 
		 TOP_mulhhu_ii, 
		 TOP_mulhh_ii, 
		 TOP_mulhs_ii, 
		 TOP_mulhu_ii, 
		 TOP_mulh_ii, 
		 TOP_mullhus_ii, 
		 TOP_mullhu_ii, 
		 TOP_mullh_ii, 
		 TOP_mulllu_ii, 
		 TOP_mulll_ii, 
		 TOP_mullu_ii, 
		 TOP_mull_ii, 
		 TOP_multi_ldpc_ii, 
		 TOP_multi_ldp_ii, 
		 TOP_multi_stpc_ii, 
		 TOP_multi_stp_ii, 
		 TOP_nandl_ii_b, 
		 TOP_nandl_ii_r, 
		 TOP_norl_ii_b, 
		 TOP_norl_ii_r, 
		 TOP_orc_ii, 
		 TOP_orl_ii_b, 
		 TOP_orl_ii_r, 
		 TOP_or_ii, 
		 TOP_pftc_ii, 
		 TOP_pft_ii, 
		 TOP_prgadd_ii, 
		 TOP_prginsadd_ii, 
		 TOP_prginspg_ii, 
		 TOP_prgset_ii, 
		 TOP_sh1add_ii, 
		 TOP_sh2add_ii, 
		 TOP_sh3add_ii, 
		 TOP_sh4add_ii, 
		 TOP_shl_ii, 
		 TOP_shru_ii, 
		 TOP_shr_ii, 
		 TOP_slctf_ii, 
		 TOP_slct_ii, 
		 TOP_stbc_ii, 
		 TOP_stb_ii, 
		 TOP_sthc_ii, 
		 TOP_sth_ii, 
		 TOP_stpc_ii, 
		 TOP_stp_ii, 
		 TOP_stwc_ii, 
		 TOP_stw_ii, 
		 TOP_sub_ii, 
		 TOP_xor_ii, 
		 TOP_UNDEFINED); 

 /* ===== Specification for ALU_Unit Type ===== */ 
  ISA_EXEC_UNIT_TYPE ALU_Unit = ISA_Exec_Unit_Type_Create("ALU_Unit", NULL); 
  Instruction_Exec_Unit_Group(ALU_Unit, 
		 TOP_addcg, 
		 TOP_addpc_r, 
		 TOP_addpc_i, 
		 TOP_add_r, 
		 TOP_add_i, 
		 TOP_spadjust, 
		 TOP_andc_r, 
		 TOP_andc_i, 
		 TOP_andl_r_b, 
		 TOP_andl_r_r, 
		 TOP_andl_i_b, 
		 TOP_andl_i_r, 
		 TOP_and_r, 
		 TOP_and_i, 
		 TOP_asm_0, 
		 TOP_asm_10, 
		 TOP_asm_11, 
		 TOP_asm_12, 
		 TOP_asm_13, 
		 TOP_asm_14, 
		 TOP_asm_15, 
		 TOP_asm_16_i, 
		 TOP_asm_17_i, 
		 TOP_asm_18_i, 
		 TOP_asm_19_i, 
		 TOP_asm_1, 
		 TOP_asm_20_i, 
		 TOP_asm_21_i, 
		 TOP_asm_22_i, 
		 TOP_asm_23_i, 
		 TOP_asm_24_i, 
		 TOP_asm_25_i, 
		 TOP_asm_26_i, 
		 TOP_asm_27_i, 
		 TOP_asm_28_i, 
		 TOP_asm_29_i, 
		 TOP_asm_2, 
		 TOP_asm_30_i, 
		 TOP_asm_31_i, 
		 TOP_asm_3, 
		 TOP_asm_4, 
		 TOP_asm_5, 
		 TOP_asm_6, 
		 TOP_asm_7, 
		 TOP_asm_8, 
		 TOP_asm_9, 
		 TOP_break, 
		 TOP_bswap, 
		 TOP_clz, 
		 TOP_cmpeq_r_b, 
		 TOP_cmpeq_r_r, 
		 TOP_cmpeq_i_b, 
		 TOP_cmpeq_i_r, 
		 TOP_cmpgeu_r_b, 
		 TOP_cmpgeu_r_r, 
		 TOP_cmpgeu_i_b, 
		 TOP_cmpgeu_i_r, 
		 TOP_cmpge_r_b, 
		 TOP_cmpge_r_r, 
		 TOP_cmpge_i_b, 
		 TOP_cmpge_i_r, 
		 TOP_cmpgtu_r_b, 
		 TOP_cmpgtu_r_r, 
		 TOP_cmpgtu_i_b, 
		 TOP_cmpgtu_i_r, 
		 TOP_cmpgt_r_b, 
		 TOP_cmpgt_r_r, 
		 TOP_cmpgt_i_b, 
		 TOP_cmpgt_i_r, 
		 TOP_cmpleu_r_b, 
		 TOP_cmpleu_r_r, 
		 TOP_cmpleu_i_b, 
		 TOP_cmpleu_i_r, 
		 TOP_cmple_r_b, 
		 TOP_cmple_r_r, 
		 TOP_cmple_i_b, 
		 TOP_cmple_i_r, 
		 TOP_cmpltu_r_b, 
		 TOP_cmpltu_r_r, 
		 TOP_cmpltu_i_b, 
		 TOP_cmpltu_i_r, 
		 TOP_cmplt_r_b, 
		 TOP_cmplt_r_r, 
		 TOP_cmplt_i_b, 
		 TOP_cmplt_i_r, 
		 TOP_cmpne_r_b, 
		 TOP_cmpne_r_r, 
		 TOP_cmpne_i_b, 
		 TOP_cmpne_i_r, 
		 TOP_composep, 
		 TOP_dbgsbrk, 
		 TOP_divs, 
		 TOP_divu, 
		 TOP_div, 
		 TOP_extractp, 
		 TOP_asm, 
		 TOP_ldbc_i, 
		 TOP_ldbuc_i, 
		 TOP_ldbu_d_i, 
		 TOP_ldbu_i, 
		 TOP_ldb_d_i, 
		 TOP_ldb_i, 
		 TOP_ldhc_i, 
		 TOP_ldhuc_i, 
		 TOP_ldhu_d_i, 
		 TOP_ldhu_i, 
		 TOP_ldh_d_i, 
		 TOP_ldh_i, 
		 TOP_ldpc_i, 
		 TOP_ldp_i, 
		 TOP_ldwc_i, 
		 TOP_ldwl, 
		 TOP_ldw_d_i, 
		 TOP_ldw_i, 
		 TOP_maxu_r, 
		 TOP_maxu_i, 
		 TOP_max_r, 
		 TOP_max_i, 
		 TOP_mfb, 
		 TOP_minu_r, 
		 TOP_minu_i, 
		 TOP_min_r, 
		 TOP_min_i, 
		 TOP_movp, 
		 TOP_mov_r, 
		 TOP_mov_i, 
		 TOP_mtb, 
		 TOP_multi_ldpc_i, 
		 TOP_multi_ldp_i, 
		 TOP_multi_stpc_i, 
		 TOP_multi_stp_i, 
		 TOP_nandl_r_b, 
		 TOP_nandl_r_r, 
		 TOP_nandl_i_b, 
		 TOP_nandl_i_r, 
		 TOP_nop, 
		 TOP_norl_r_b, 
		 TOP_norl_r_r, 
		 TOP_norl_i_b, 
		 TOP_norl_i_r, 
		 TOP_orc_r, 
		 TOP_orc_i, 
		 TOP_orl_r_b, 
		 TOP_orl_r_r, 
		 TOP_orl_i_b, 
		 TOP_orl_i_r, 
		 TOP_or_r, 
		 TOP_or_i, 
		 TOP_pftc_i, 
		 TOP_pft_i, 
		 TOP_prgadd_i, 
		 TOP_prginsadd_i, 
		 TOP_prginspg_i, 
		 TOP_prgset_i, 
		 TOP_remu, 
		 TOP_rem, 
		 TOP_sbrk_ib, 
		 TOP_sbrk_i, 
		 TOP_sh1add_r, 
		 TOP_sh1add_i, 
		 TOP_sh2add_r, 
		 TOP_sh2add_i, 
		 TOP_sh3add_r, 
		 TOP_sh3add_i, 
		 TOP_sh4add_r, 
		 TOP_sh4add_i, 
		 TOP_shl_r, 
		 TOP_shl_i, 
		 TOP_shru_r, 
		 TOP_shru_i, 
		 TOP_shr_r, 
		 TOP_shr_i, 
		 TOP_slctf_r, 
		 TOP_slctf_i, 
		 TOP_slct_r, 
		 TOP_slct_i, 
		 TOP_stbc_i, 
		 TOP_stb_i, 
		 TOP_sthc_i, 
		 TOP_sth_i, 
		 TOP_stpc_i, 
		 TOP_stp_i, 
		 TOP_stwc_i, 
		 TOP_stwl, 
		 TOP_stw_i, 
		 TOP_sub_r, 
		 TOP_sub_i, 
		 TOP_sxtb, 
		 TOP_sxth, 
		 TOP_sync, 
		 TOP_wmb, 
		 TOP_xor_r, 
		 TOP_xor_i, 
		 TOP_zxtb, 
		 TOP_zxth, 
		 TOP_UNDEFINED); 

 /* ===== Specification for CTL_Unit Type ===== */ 
  ISA_EXEC_UNIT_TYPE CTL_Unit = ISA_Exec_Unit_Type_Create("CTL_Unit", NULL); 
  Instruction_Exec_Unit_Group(CTL_Unit, 
		 TOP_brf, 
		 TOP_br, 
		 TOP_call, 
		 TOP_getpc, 
		 TOP_goto, 
		 TOP_icall, 
		 TOP_idle, 
		 TOP_igoto, 
		 TOP_pswclr, 
		 TOP_pswset, 
		 TOP_pushregs, 
		 TOP_return, 
		 TOP_returnadd, 
		 TOP_rfi, 
		 TOP_syncins, 
		 TOP_UNDEFINED); 

 /* ===== Specification for EVEN_Unit Type ===== */ 
  ISA_EXEC_UNIT_TYPE EVEN_Unit = ISA_Exec_Unit_Type_Create("EVEN_Unit", NULL); 
  Instruction_Exec_Unit_Group(EVEN_Unit, 
		 TOP_addf_n, 
		 TOP_convfi_n, 
		 TOP_convif_n, 
		 TOP_subf_n, 
		 TOP_UNDEFINED); 

 /* ===== Specification for ODD_Unit Type ===== */ 
  ISA_EXEC_UNIT_TYPE ODD_Unit = ISA_Exec_Unit_Type_Create("ODD_Unit", NULL); 
  Instruction_Exec_Unit_Group(ODD_Unit, 
		 TOP_mul32_r, 
		 TOP_mul32_i, 
		 TOP_mul64hu_r, 
		 TOP_mul64hu_i, 
		 TOP_mul64h_r, 
		 TOP_mul64h_i, 
		 TOP_mulfrac_r, 
		 TOP_mulfrac_i, 
		 TOP_mulf_n, 
		 TOP_mulhhs_r, 
		 TOP_mulhhs_i, 
		 TOP_mulhhu_r, 
		 TOP_mulhhu_i, 
		 TOP_mulhh_r, 
		 TOP_mulhh_i, 
		 TOP_mulhs_r, 
		 TOP_mulhs_i, 
		 TOP_mulhu_r, 
		 TOP_mulhu_i, 
		 TOP_mulh_r, 
		 TOP_mulh_i, 
		 TOP_mullhus_r, 
		 TOP_mullhus_i, 
		 TOP_mullhu_r, 
		 TOP_mullhu_i, 
		 TOP_mullh_r, 
		 TOP_mullh_i, 
		 TOP_mulllu_r, 
		 TOP_mulllu_i, 
		 TOP_mulll_r, 
		 TOP_mulll_i, 
		 TOP_mullu_r, 
		 TOP_mullu_i, 
		 TOP_mull_r, 
		 TOP_mull_i, 
		 TOP_UNDEFINED); 

  ISA_Bundle_Type_Create("Template_0", ".Template_0", 1);
  Slot(0, ALL_Unit);
  Stop(0);

  ISA_Bundle_Type_Create("Template_1", ".Template_1", 1);
  Slot(0, ALU_Unit);
  Stop(0);

  ISA_Bundle_Type_Create("Template_2", ".Template_2", 1);
  Slot(0, CTL_Unit);
  Stop(0);

  ISA_Bundle_Type_Create("Template_3", ".Template_3", 1);
  Alignment(0, 64);
  Slot(0, EVEN_Unit);
  Stop(0);

  ISA_Bundle_Type_Create("Template_4", ".Template_4", 1);
  Alignment(32, 64);
  Slot(0, ODD_Unit);
  Stop(0);

  ISA_Bundle_Type_Create("Template_5", ".Template_5", 2);
  Slot(0, ALU_Unit);
  Slot(1, ALU_Unit);
  Stop(1);

  ISA_Bundle_Type_Create("Template_6", ".Template_6", 2);
  Slot(0, ALU_Unit);
  Slot(1, CTL_Unit);
  Stop(1);

  ISA_Bundle_Type_Create("Template_7", ".Template_7", 2);
  Slot(0, ALU_Unit);
  Slot(1, EVEN_Unit);
  Stop(1);

  ISA_Bundle_Type_Create("Template_8", ".Template_8", 2);
  Slot(0, ALU_Unit);
  Slot(1, ODD_Unit);
  Stop(1);

  ISA_Bundle_Type_Create("Template_9", ".Template_9", 2);
  Slot(0, ALUX_Unit);
  Slot(1, ALUX_Unit);
  Stop(1);

  ISA_Bundle_Type_Create("Template_10", ".Template_10", 2);
  Alignment(32, 64);
  Slot(0, CTL_Unit);
  Slot(1, EVEN_Unit);
  Stop(1);

  ISA_Bundle_Type_Create("Template_11", ".Template_11", 2);
  Alignment(0, 64);
  Slot(0, CTL_Unit);
  Slot(1, ODD_Unit);
  Stop(1);

  ISA_Bundle_Type_Create("Template_12", ".Template_12", 2);
  Slot(0, EVEN_Unit);
  Slot(1, ODD_Unit);
  Stop(1);

  ISA_Bundle_Type_Create("Template_13", ".Template_13", 3);
  Slot(0, ALU_Unit);
  Slot(1, ALU_Unit);
  Slot(2, ALU_Unit);
  Stop(2);

  ISA_Bundle_Type_Create("Template_14", ".Template_14", 3);
  Slot(0, ALU_Unit);
  Slot(1, ALU_Unit);
  Slot(2, CTL_Unit);
  Stop(2);

  ISA_Bundle_Type_Create("Template_15", ".Template_15", 3);
  Slot(0, ALU_Unit);
  Slot(1, ALU_Unit);
  Slot(2, EVEN_Unit);
  Stop(2);

  ISA_Bundle_Type_Create("Template_16", ".Template_16", 3);
  Slot(0, ALU_Unit);
  Slot(1, ALU_Unit);
  Slot(2, ODD_Unit);
  Stop(2);

  ISA_Bundle_Type_Create("Template_17", ".Template_17", 3);
  Slot(0, ALU_Unit);
  Slot(1, ALUX_Unit);
  Slot(2, ALUX_Unit);
  Stop(2);

  ISA_Bundle_Type_Create("Template_18", ".Template_18", 3);
  Slot(0, ALU_Unit);
  Slot(1, CTL_Unit);
  Slot(2, EVEN_Unit);
  Stop(2);

  ISA_Bundle_Type_Create("Template_19", ".Template_19", 3);
  Slot(0, ALU_Unit);
  Slot(1, CTL_Unit);
  Slot(2, ODD_Unit);
  Stop(2);

  ISA_Bundle_Type_Create("Template_20", ".Template_20", 3);
  Alignment(0, 64);
  Slot(0, ALU_Unit);
  Slot(1, EVEN_Unit);
  Slot(2, EVEN_Unit);
  Stop(2);

  ISA_Bundle_Type_Create("Template_21", ".Template_21", 3);
  Slot(0, ALU_Unit);
  Slot(1, EVEN_Unit);
  Slot(2, ODD_Unit);
  Stop(2);

  ISA_Bundle_Type_Create("Template_22", ".Template_22", 3);
  Alignment(32, 64);
  Slot(0, ALU_Unit);
  Slot(1, ODD_Unit);
  Slot(2, ODD_Unit);
  Stop(2);

  ISA_Bundle_Type_Create("Template_23", ".Template_23", 3);
  Slot(0, ALUX_Unit);
  Slot(1, ALUX_Unit);
  Slot(2, CTL_Unit);
  Stop(2);

  ISA_Bundle_Type_Create("Template_24", ".Template_24", 3);
  Alignment(0, 64);
  Slot(0, ALUX_Unit);
  Slot(1, ALUX_Unit);
  Slot(2, EVEN_Unit);
  Stop(2);

  ISA_Bundle_Type_Create("Template_25", ".Template_25", 3);
  Alignment(32, 64);
  Slot(0, ALUX_Unit);
  Slot(1, ALUX_Unit);
  Slot(2, ODD_Unit);
  Stop(2);

  ISA_Bundle_Type_Create("Template_26", ".Template_26", 3);
  Slot(0, CTL_Unit);
  Slot(1, EVEN_Unit);
  Slot(2, ODD_Unit);
  Stop(2);

  ISA_Bundle_Type_Create("Template_27", ".Template_27", 3);
  Alignment(0, 64);
  Slot(0, EVEN_Unit);
  Slot(1, EVEN_Unit);
  Slot(2, ODD_Unit);
  Stop(2);

  ISA_Bundle_Type_Create("Template_28", ".Template_28", 3);
  Alignment(32, 64);
  Slot(0, EVEN_Unit);
  Slot(1, ODD_Unit);
  Slot(2, ODD_Unit);
  Stop(2);

  ISA_Bundle_Type_Create("Template_29", ".Template_29", 4);
  Slot(0, ALU_Unit);
  Slot(1, ALU_Unit);
  Slot(2, ALU_Unit);
  Slot(3, ALU_Unit);
  Stop(3);

  ISA_Bundle_Type_Create("Template_30", ".Template_30", 4);
  Slot(0, ALU_Unit);
  Slot(1, ALU_Unit);
  Slot(2, ALU_Unit);
  Slot(3, CTL_Unit);
  Stop(3);

  ISA_Bundle_Type_Create("Template_31", ".Template_31", 4);
  Slot(0, ALU_Unit);
  Slot(1, ALU_Unit);
  Slot(2, ALU_Unit);
  Slot(3, EVEN_Unit);
  Stop(3);

  ISA_Bundle_Type_Create("Template_32", ".Template_32", 4);
  Slot(0, ALU_Unit);
  Slot(1, ALU_Unit);
  Slot(2, ALU_Unit);
  Slot(3, ODD_Unit);
  Stop(3);

  ISA_Bundle_Type_Create("Template_33", ".Template_33", 4);
  Slot(0, ALU_Unit);
  Slot(1, ALU_Unit);
  Slot(2, ALUX_Unit);
  Slot(3, ALUX_Unit);
  Stop(3);

  ISA_Bundle_Type_Create("Template_34", ".Template_34", 4);
  Slot(0, ALU_Unit);
  Slot(1, ALU_Unit);
  Slot(2, CTL_Unit);
  Slot(3, EVEN_Unit);
  Stop(3);

  ISA_Bundle_Type_Create("Template_35", ".Template_35", 4);
  Slot(0, ALU_Unit);
  Slot(1, ALU_Unit);
  Slot(2, CTL_Unit);
  Slot(3, ODD_Unit);
  Stop(3);

  ISA_Bundle_Type_Create("Template_36", ".Template_36", 4);
  Slot(0, ALU_Unit);
  Slot(1, ALU_Unit);
  Slot(2, EVEN_Unit);
  Slot(3, EVEN_Unit);
  Stop(3);

  ISA_Bundle_Type_Create("Template_37", ".Template_37", 4);
  Slot(0, ALU_Unit);
  Slot(1, ALU_Unit);
  Slot(2, EVEN_Unit);
  Slot(3, ODD_Unit);
  Stop(3);

  ISA_Bundle_Type_Create("Template_38", ".Template_38", 4);
  Slot(0, ALU_Unit);
  Slot(1, ALU_Unit);
  Slot(2, ODD_Unit);
  Slot(3, ODD_Unit);
  Stop(3);

  ISA_Bundle_Type_Create("Template_39", ".Template_39", 4);
  Slot(0, ALU_Unit);
  Slot(1, ALUX_Unit);
  Slot(2, ALUX_Unit);
  Slot(3, CTL_Unit);
  Stop(3);

  ISA_Bundle_Type_Create("Template_40", ".Template_40", 4);
  Slot(0, ALU_Unit);
  Slot(1, ALUX_Unit);
  Slot(2, ALUX_Unit);
  Slot(3, EVEN_Unit);
  Stop(3);

  ISA_Bundle_Type_Create("Template_41", ".Template_41", 4);
  Slot(0, ALU_Unit);
  Slot(1, ALUX_Unit);
  Slot(2, ALUX_Unit);
  Slot(3, ODD_Unit);
  Stop(3);

  ISA_Bundle_Type_Create("Template_42", ".Template_42", 4);
  Alignment(32, 64);
  Slot(0, ALU_Unit);
  Slot(1, CTL_Unit);
  Slot(2, EVEN_Unit);
  Slot(3, EVEN_Unit);
  Stop(3);

  ISA_Bundle_Type_Create("Template_43", ".Template_43", 4);
  Slot(0, ALU_Unit);
  Slot(1, CTL_Unit);
  Slot(2, EVEN_Unit);
  Slot(3, ODD_Unit);
  Stop(3);

  ISA_Bundle_Type_Create("Template_44", ".Template_44", 4);
  Alignment(0, 64);
  Slot(0, ALU_Unit);
  Slot(1, CTL_Unit);
  Slot(2, ODD_Unit);
  Slot(3, ODD_Unit);
  Stop(3);

  ISA_Bundle_Type_Create("Template_45", ".Template_45", 4);
  Slot(0, ALU_Unit);
  Slot(1, EVEN_Unit);
  Slot(2, EVEN_Unit);
  Slot(3, ODD_Unit);
  Stop(3);

  ISA_Bundle_Type_Create("Template_46", ".Template_46", 4);
  Slot(0, ALU_Unit);
  Slot(1, EVEN_Unit);
  Slot(2, ODD_Unit);
  Slot(3, ODD_Unit);
  Stop(3);

  ISA_Bundle_Type_Create("Template_47", ".Template_47", 4);
  Slot(0, ALUX_Unit);
  Slot(1, ALUX_Unit);
  Slot(2, ALUX_Unit);
  Slot(3, ALUX_Unit);
  Stop(3);

  ISA_Bundle_Type_Create("Template_48", ".Template_48", 4);
  Alignment(32, 64);
  Slot(0, ALUX_Unit);
  Slot(1, ALUX_Unit);
  Slot(2, CTL_Unit);
  Slot(3, EVEN_Unit);
  Stop(3);

  ISA_Bundle_Type_Create("Template_49", ".Template_49", 4);
  Alignment(0, 64);
  Slot(0, ALUX_Unit);
  Slot(1, ALUX_Unit);
  Slot(2, CTL_Unit);
  Slot(3, ODD_Unit);
  Stop(3);

  ISA_Bundle_Type_Create("Template_50", ".Template_50", 4);
  Slot(0, ALUX_Unit);
  Slot(1, ALUX_Unit);
  Slot(2, EVEN_Unit);
  Slot(3, ODD_Unit);
  Stop(3);

  ISA_Bundle_Type_Create("Template_51", ".Template_51", 4);
  Alignment(32, 64);
  Slot(0, CTL_Unit);
  Slot(1, EVEN_Unit);
  Slot(2, EVEN_Unit);
  Slot(3, ODD_Unit);
  Stop(3);

  ISA_Bundle_Type_Create("Template_52", ".Template_52", 4);
  Alignment(0, 64);
  Slot(0, CTL_Unit);
  Slot(1, EVEN_Unit);
  Slot(2, ODD_Unit);
  Slot(3, ODD_Unit);
  Stop(3);

  ISA_Bundle_Type_Create("Template_53", ".Template_53", 4);
  Slot(0, EVEN_Unit);
  Slot(1, EVEN_Unit);
  Slot(2, ODD_Unit);
  Slot(3, ODD_Unit);
  Stop(3);


  ISA_Bundle_End(); 
  return 0; 
} 
