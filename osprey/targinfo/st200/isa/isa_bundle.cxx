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

//  
//  Generate ISA bundle information 
/////////////////////////////////////// 

#include <stddef.h> 
#include "topcode.h" 
#include "isa_bundle_gen.h" 

main() 
{ 
  ISA_EXEC_UNIT_TYPE 
		    S0_Unit, 
		    S1_Unit, 
		    S2_Unit, 
		    S3_Unit, 
		    EXT0_Unit, 
		    EXT1_Unit, 
		    EXT2_Unit, 
		    Odd_Unit, 
		    Even_Unit, 
		    ReqS0_Unit; 

  ISA_Bundle_Begin("st200", 128); 

  /* ===== Specification for bundle packing  ===== */ 
  ISA_Bundle_Pack_Create(ISA_Bundle_Pack_Little_Endian); 
  Pack_Template(0, 0, 0); 
  Pack_Slot(0, 0, 0, 32); 
  Pack_Slot(1, 0, 32, 32); 
  Pack_Slot(2, 0, 64, 32); 
  Pack_Slot(3, 0, 96, 32); 

  /* ===== Specification for S0_Unit Type ===== */ 
  S0_Unit = ISA_Exec_Unit_Type_Create("S0_Unit", NULL); 
  Instruction_Exec_Unit_Group(S0_Unit, 
		 TOP_add_r, 
		 TOP_add_i, 
		 TOP_addcg, 
		 TOP_and_r, 
		 TOP_and_i, 
		 TOP_andc_r, 
		 TOP_andc_i, 
		 TOP_andl_r_r, 
		 TOP_andl_r_b, 
		 TOP_andl_i_r, 
		 TOP_andl_i_b, 
		 TOP_break, 
		 TOP_bswap_r, 
		 TOP_clz_r, 
		 TOP_cmpeq_r_r, 
		 TOP_cmpeq_r_b, 
		 TOP_cmpeq_i_r, 
		 TOP_cmpeq_i_b, 
		 TOP_cmpge_r_r, 
		 TOP_cmpge_r_b, 
		 TOP_cmpge_i_r, 
		 TOP_cmpge_i_b, 
		 TOP_cmpgeu_r_r, 
		 TOP_cmpgeu_r_b, 
		 TOP_cmpgeu_i_r, 
		 TOP_cmpgeu_i_b, 
		 TOP_cmpgt_r_r, 
		 TOP_cmpgt_r_b, 
		 TOP_cmpgt_i_r, 
		 TOP_cmpgt_i_b, 
		 TOP_cmpgtu_r_r, 
		 TOP_cmpgtu_r_b, 
		 TOP_cmpgtu_i_r, 
		 TOP_cmpgtu_i_b, 
		 TOP_cmple_r_r, 
		 TOP_cmple_r_b, 
		 TOP_cmple_i_r, 
		 TOP_cmple_i_b, 
		 TOP_cmpleu_r_r, 
		 TOP_cmpleu_r_b, 
		 TOP_cmpleu_i_r, 
		 TOP_cmpleu_i_b, 
		 TOP_cmplt_r_r, 
		 TOP_cmplt_r_b, 
		 TOP_cmplt_i_r, 
		 TOP_cmplt_i_b, 
		 TOP_cmpltu_r_r, 
		 TOP_cmpltu_r_b, 
		 TOP_cmpltu_i_r, 
		 TOP_cmpltu_i_b, 
		 TOP_cmpne_r_r, 
		 TOP_cmpne_r_b, 
		 TOP_cmpne_i_r, 
		 TOP_cmpne_i_b, 
		 TOP_divs, 
		 TOP_max_r, 
		 TOP_max_i, 
		 TOP_maxu_r, 
		 TOP_maxu_i, 
		 TOP_min_r, 
		 TOP_min_i, 
		 TOP_minu_r, 
		 TOP_minu_i, 
		 TOP_nandl_r_r, 
		 TOP_nandl_r_b, 
		 TOP_nandl_i_r, 
		 TOP_nandl_i_b, 
		 TOP_norl_r_r, 
		 TOP_norl_r_b, 
		 TOP_norl_i_r, 
		 TOP_norl_i_b, 
		 TOP_or_r, 
		 TOP_or_i, 
		 TOP_orc_r, 
		 TOP_orc_i, 
		 TOP_orl_r_r, 
		 TOP_orl_r_b, 
		 TOP_orl_i_r, 
		 TOP_orl_i_b, 
		 TOP_sbrk, 
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
		 TOP_shr_r, 
		 TOP_shr_i, 
		 TOP_shru_r, 
		 TOP_shru_i, 
		 TOP_slct_r, 
		 TOP_slct_i, 
		 TOP_slctf_r, 
		 TOP_slctf_i, 
		 TOP_sub_r, 
		 TOP_sub_i, 
		 TOP_sxtb_r, 
		 TOP_sxth_r, 
		 TOP_syscall, 
		 TOP_xor_r, 
		 TOP_xor_i, 
		 TOP_zxth_r, 
		 TOP_nop, 
		 TOP_mov_r, 
		 TOP_mov_i, 
		 TOP_mtb, 
		 TOP_mfb, 
		 TOP_mulh_r, 
		 TOP_mulh_i, 
		 TOP_mulhh_r, 
		 TOP_mulhh_i, 
		 TOP_mulhhs_r, 
		 TOP_mulhhs_i, 
		 TOP_mulhhu_r, 
		 TOP_mulhhu_i, 
		 TOP_mulhs_r, 
		 TOP_mulhs_i, 
		 TOP_mulhu_r, 
		 TOP_mulhu_i, 
		 TOP_mull_r, 
		 TOP_mull_i, 
		 TOP_mullh_r, 
		 TOP_mullh_i, 
		 TOP_mullhu_r, 
		 TOP_mullhu_i, 
		 TOP_mullhus_r, 
		 TOP_mullhus_i, 
		 TOP_mulll_r, 
		 TOP_mulll_i, 
		 TOP_mulllu_r, 
		 TOP_mulllu_i, 
		 TOP_mullu_r, 
		 TOP_mullu_i, 
		 TOP_ldb_i, 
		 TOP_ldb_d_i, 
		 TOP_ldbu_i, 
		 TOP_ldbu_d_i, 
		 TOP_ldh_i, 
		 TOP_ldh_d_i, 
		 TOP_ldhu_i, 
		 TOP_ldhu_d_i, 
		 TOP_ldw_i, 
		 TOP_ldw_d_i, 
		 TOP_sync, 
		 TOP_pft_i, 
		 TOP_prgadd_i, 
		 TOP_prgset_i, 
		 TOP_stb_i, 
		 TOP_sth_i, 
		 TOP_stw_i, 
		 TOP_imml, 
		 TOP_immr, 
		 TOP_br, 
		 TOP_brf, 
		 TOP_goto, 
		 TOP_igoto, 
		 TOP_return, 
		 TOP_call, 
		 TOP_icall, 
		 TOP_prgins, 
		 TOP_rfi, 
		 TOP_UNDEFINED); 

  /* ===== Specification for S1_Unit Type ===== */ 
  S1_Unit = ISA_Exec_Unit_Type_Create("S1_Unit", NULL); 
  Instruction_Exec_Unit_Group(S1_Unit, 
		 TOP_add_r, 
		 TOP_add_i, 
		 TOP_addcg, 
		 TOP_and_r, 
		 TOP_and_i, 
		 TOP_andc_r, 
		 TOP_andc_i, 
		 TOP_andl_r_r, 
		 TOP_andl_r_b, 
		 TOP_andl_i_r, 
		 TOP_andl_i_b, 
		 TOP_break, 
		 TOP_bswap_r, 
		 TOP_clz_r, 
		 TOP_cmpeq_r_r, 
		 TOP_cmpeq_r_b, 
		 TOP_cmpeq_i_r, 
		 TOP_cmpeq_i_b, 
		 TOP_cmpge_r_r, 
		 TOP_cmpge_r_b, 
		 TOP_cmpge_i_r, 
		 TOP_cmpge_i_b, 
		 TOP_cmpgeu_r_r, 
		 TOP_cmpgeu_r_b, 
		 TOP_cmpgeu_i_r, 
		 TOP_cmpgeu_i_b, 
		 TOP_cmpgt_r_r, 
		 TOP_cmpgt_r_b, 
		 TOP_cmpgt_i_r, 
		 TOP_cmpgt_i_b, 
		 TOP_cmpgtu_r_r, 
		 TOP_cmpgtu_r_b, 
		 TOP_cmpgtu_i_r, 
		 TOP_cmpgtu_i_b, 
		 TOP_cmple_r_r, 
		 TOP_cmple_r_b, 
		 TOP_cmple_i_r, 
		 TOP_cmple_i_b, 
		 TOP_cmpleu_r_r, 
		 TOP_cmpleu_r_b, 
		 TOP_cmpleu_i_r, 
		 TOP_cmpleu_i_b, 
		 TOP_cmplt_r_r, 
		 TOP_cmplt_r_b, 
		 TOP_cmplt_i_r, 
		 TOP_cmplt_i_b, 
		 TOP_cmpltu_r_r, 
		 TOP_cmpltu_r_b, 
		 TOP_cmpltu_i_r, 
		 TOP_cmpltu_i_b, 
		 TOP_cmpne_r_r, 
		 TOP_cmpne_r_b, 
		 TOP_cmpne_i_r, 
		 TOP_cmpne_i_b, 
		 TOP_divs, 
		 TOP_max_r, 
		 TOP_max_i, 
		 TOP_maxu_r, 
		 TOP_maxu_i, 
		 TOP_min_r, 
		 TOP_min_i, 
		 TOP_minu_r, 
		 TOP_minu_i, 
		 TOP_nandl_r_r, 
		 TOP_nandl_r_b, 
		 TOP_nandl_i_r, 
		 TOP_nandl_i_b, 
		 TOP_norl_r_r, 
		 TOP_norl_r_b, 
		 TOP_norl_i_r, 
		 TOP_norl_i_b, 
		 TOP_or_r, 
		 TOP_or_i, 
		 TOP_orc_r, 
		 TOP_orc_i, 
		 TOP_orl_r_r, 
		 TOP_orl_r_b, 
		 TOP_orl_i_r, 
		 TOP_orl_i_b, 
		 TOP_sbrk, 
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
		 TOP_shr_r, 
		 TOP_shr_i, 
		 TOP_shru_r, 
		 TOP_shru_i, 
		 TOP_slct_r, 
		 TOP_slct_i, 
		 TOP_slctf_r, 
		 TOP_slctf_i, 
		 TOP_sub_r, 
		 TOP_sub_i, 
		 TOP_sxtb_r, 
		 TOP_sxth_r, 
		 TOP_syscall, 
		 TOP_xor_r, 
		 TOP_xor_i, 
		 TOP_zxth_r, 
		 TOP_nop, 
		 TOP_mov_r, 
		 TOP_mov_i, 
		 TOP_mtb, 
		 TOP_mfb, 
		 TOP_mulh_r, 
		 TOP_mulh_i, 
		 TOP_mulhh_r, 
		 TOP_mulhh_i, 
		 TOP_mulhhs_r, 
		 TOP_mulhhs_i, 
		 TOP_mulhhu_r, 
		 TOP_mulhhu_i, 
		 TOP_mulhs_r, 
		 TOP_mulhs_i, 
		 TOP_mulhu_r, 
		 TOP_mulhu_i, 
		 TOP_mull_r, 
		 TOP_mull_i, 
		 TOP_mullh_r, 
		 TOP_mullh_i, 
		 TOP_mullhu_r, 
		 TOP_mullhu_i, 
		 TOP_mullhus_r, 
		 TOP_mullhus_i, 
		 TOP_mulll_r, 
		 TOP_mulll_i, 
		 TOP_mulllu_r, 
		 TOP_mulllu_i, 
		 TOP_mullu_r, 
		 TOP_mullu_i, 
		 TOP_ldb_i, 
		 TOP_ldb_d_i, 
		 TOP_ldbu_i, 
		 TOP_ldbu_d_i, 
		 TOP_ldh_i, 
		 TOP_ldh_d_i, 
		 TOP_ldhu_i, 
		 TOP_ldhu_d_i, 
		 TOP_ldw_i, 
		 TOP_ldw_d_i, 
		 TOP_sync, 
		 TOP_pft_i, 
		 TOP_prgadd_i, 
		 TOP_prgset_i, 
		 TOP_stb_i, 
		 TOP_sth_i, 
		 TOP_stw_i, 
		 TOP_imml, 
		 TOP_immr, 
		 TOP_br, 
		 TOP_brf, 
		 TOP_goto, 
		 TOP_igoto, 
		 TOP_return, 
		 TOP_call, 
		 TOP_icall, 
		 TOP_prgins, 
		 TOP_rfi, 
		 TOP_UNDEFINED); 

  /* ===== Specification for S2_Unit Type ===== */ 
  S2_Unit = ISA_Exec_Unit_Type_Create("S2_Unit", NULL); 
  Instruction_Exec_Unit_Group(S2_Unit, 
		 TOP_add_r, 
		 TOP_add_i, 
		 TOP_addcg, 
		 TOP_and_r, 
		 TOP_and_i, 
		 TOP_andc_r, 
		 TOP_andc_i, 
		 TOP_andl_r_r, 
		 TOP_andl_r_b, 
		 TOP_andl_i_r, 
		 TOP_andl_i_b, 
		 TOP_break, 
		 TOP_bswap_r, 
		 TOP_clz_r, 
		 TOP_cmpeq_r_r, 
		 TOP_cmpeq_r_b, 
		 TOP_cmpeq_i_r, 
		 TOP_cmpeq_i_b, 
		 TOP_cmpge_r_r, 
		 TOP_cmpge_r_b, 
		 TOP_cmpge_i_r, 
		 TOP_cmpge_i_b, 
		 TOP_cmpgeu_r_r, 
		 TOP_cmpgeu_r_b, 
		 TOP_cmpgeu_i_r, 
		 TOP_cmpgeu_i_b, 
		 TOP_cmpgt_r_r, 
		 TOP_cmpgt_r_b, 
		 TOP_cmpgt_i_r, 
		 TOP_cmpgt_i_b, 
		 TOP_cmpgtu_r_r, 
		 TOP_cmpgtu_r_b, 
		 TOP_cmpgtu_i_r, 
		 TOP_cmpgtu_i_b, 
		 TOP_cmple_r_r, 
		 TOP_cmple_r_b, 
		 TOP_cmple_i_r, 
		 TOP_cmple_i_b, 
		 TOP_cmpleu_r_r, 
		 TOP_cmpleu_r_b, 
		 TOP_cmpleu_i_r, 
		 TOP_cmpleu_i_b, 
		 TOP_cmplt_r_r, 
		 TOP_cmplt_r_b, 
		 TOP_cmplt_i_r, 
		 TOP_cmplt_i_b, 
		 TOP_cmpltu_r_r, 
		 TOP_cmpltu_r_b, 
		 TOP_cmpltu_i_r, 
		 TOP_cmpltu_i_b, 
		 TOP_cmpne_r_r, 
		 TOP_cmpne_r_b, 
		 TOP_cmpne_i_r, 
		 TOP_cmpne_i_b, 
		 TOP_divs, 
		 TOP_max_r, 
		 TOP_max_i, 
		 TOP_maxu_r, 
		 TOP_maxu_i, 
		 TOP_min_r, 
		 TOP_min_i, 
		 TOP_minu_r, 
		 TOP_minu_i, 
		 TOP_nandl_r_r, 
		 TOP_nandl_r_b, 
		 TOP_nandl_i_r, 
		 TOP_nandl_i_b, 
		 TOP_norl_r_r, 
		 TOP_norl_r_b, 
		 TOP_norl_i_r, 
		 TOP_norl_i_b, 
		 TOP_or_r, 
		 TOP_or_i, 
		 TOP_orc_r, 
		 TOP_orc_i, 
		 TOP_orl_r_r, 
		 TOP_orl_r_b, 
		 TOP_orl_i_r, 
		 TOP_orl_i_b, 
		 TOP_sbrk, 
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
		 TOP_shr_r, 
		 TOP_shr_i, 
		 TOP_shru_r, 
		 TOP_shru_i, 
		 TOP_slct_r, 
		 TOP_slct_i, 
		 TOP_slctf_r, 
		 TOP_slctf_i, 
		 TOP_sub_r, 
		 TOP_sub_i, 
		 TOP_sxtb_r, 
		 TOP_sxth_r, 
		 TOP_syscall, 
		 TOP_xor_r, 
		 TOP_xor_i, 
		 TOP_zxth_r, 
		 TOP_nop, 
		 TOP_mov_r, 
		 TOP_mov_i, 
		 TOP_mtb, 
		 TOP_mfb, 
		 TOP_mulh_r, 
		 TOP_mulh_i, 
		 TOP_mulhh_r, 
		 TOP_mulhh_i, 
		 TOP_mulhhs_r, 
		 TOP_mulhhs_i, 
		 TOP_mulhhu_r, 
		 TOP_mulhhu_i, 
		 TOP_mulhs_r, 
		 TOP_mulhs_i, 
		 TOP_mulhu_r, 
		 TOP_mulhu_i, 
		 TOP_mull_r, 
		 TOP_mull_i, 
		 TOP_mullh_r, 
		 TOP_mullh_i, 
		 TOP_mullhu_r, 
		 TOP_mullhu_i, 
		 TOP_mullhus_r, 
		 TOP_mullhus_i, 
		 TOP_mulll_r, 
		 TOP_mulll_i, 
		 TOP_mulllu_r, 
		 TOP_mulllu_i, 
		 TOP_mullu_r, 
		 TOP_mullu_i, 
		 TOP_ldb_i, 
		 TOP_ldb_d_i, 
		 TOP_ldbu_i, 
		 TOP_ldbu_d_i, 
		 TOP_ldh_i, 
		 TOP_ldh_d_i, 
		 TOP_ldhu_i, 
		 TOP_ldhu_d_i, 
		 TOP_ldw_i, 
		 TOP_ldw_d_i, 
		 TOP_sync, 
		 TOP_pft_i, 
		 TOP_prgadd_i, 
		 TOP_prgset_i, 
		 TOP_stb_i, 
		 TOP_sth_i, 
		 TOP_stw_i, 
		 TOP_imml, 
		 TOP_immr, 
		 TOP_br, 
		 TOP_brf, 
		 TOP_goto, 
		 TOP_igoto, 
		 TOP_return, 
		 TOP_call, 
		 TOP_icall, 
		 TOP_prgins, 
		 TOP_rfi, 
		 TOP_UNDEFINED); 

  /* ===== Specification for S3_Unit Type ===== */ 
  S3_Unit = ISA_Exec_Unit_Type_Create("S3_Unit", NULL); 
  Instruction_Exec_Unit_Group(S3_Unit, 
		 TOP_add_r, 
		 TOP_add_i, 
		 TOP_addcg, 
		 TOP_and_r, 
		 TOP_and_i, 
		 TOP_andc_r, 
		 TOP_andc_i, 
		 TOP_andl_r_r, 
		 TOP_andl_r_b, 
		 TOP_andl_i_r, 
		 TOP_andl_i_b, 
		 TOP_break, 
		 TOP_bswap_r, 
		 TOP_clz_r, 
		 TOP_cmpeq_r_r, 
		 TOP_cmpeq_r_b, 
		 TOP_cmpeq_i_r, 
		 TOP_cmpeq_i_b, 
		 TOP_cmpge_r_r, 
		 TOP_cmpge_r_b, 
		 TOP_cmpge_i_r, 
		 TOP_cmpge_i_b, 
		 TOP_cmpgeu_r_r, 
		 TOP_cmpgeu_r_b, 
		 TOP_cmpgeu_i_r, 
		 TOP_cmpgeu_i_b, 
		 TOP_cmpgt_r_r, 
		 TOP_cmpgt_r_b, 
		 TOP_cmpgt_i_r, 
		 TOP_cmpgt_i_b, 
		 TOP_cmpgtu_r_r, 
		 TOP_cmpgtu_r_b, 
		 TOP_cmpgtu_i_r, 
		 TOP_cmpgtu_i_b, 
		 TOP_cmple_r_r, 
		 TOP_cmple_r_b, 
		 TOP_cmple_i_r, 
		 TOP_cmple_i_b, 
		 TOP_cmpleu_r_r, 
		 TOP_cmpleu_r_b, 
		 TOP_cmpleu_i_r, 
		 TOP_cmpleu_i_b, 
		 TOP_cmplt_r_r, 
		 TOP_cmplt_r_b, 
		 TOP_cmplt_i_r, 
		 TOP_cmplt_i_b, 
		 TOP_cmpltu_r_r, 
		 TOP_cmpltu_r_b, 
		 TOP_cmpltu_i_r, 
		 TOP_cmpltu_i_b, 
		 TOP_cmpne_r_r, 
		 TOP_cmpne_r_b, 
		 TOP_cmpne_i_r, 
		 TOP_cmpne_i_b, 
		 TOP_divs, 
		 TOP_max_r, 
		 TOP_max_i, 
		 TOP_maxu_r, 
		 TOP_maxu_i, 
		 TOP_min_r, 
		 TOP_min_i, 
		 TOP_minu_r, 
		 TOP_minu_i, 
		 TOP_nandl_r_r, 
		 TOP_nandl_r_b, 
		 TOP_nandl_i_r, 
		 TOP_nandl_i_b, 
		 TOP_norl_r_r, 
		 TOP_norl_r_b, 
		 TOP_norl_i_r, 
		 TOP_norl_i_b, 
		 TOP_or_r, 
		 TOP_or_i, 
		 TOP_orc_r, 
		 TOP_orc_i, 
		 TOP_orl_r_r, 
		 TOP_orl_r_b, 
		 TOP_orl_i_r, 
		 TOP_orl_i_b, 
		 TOP_sbrk, 
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
		 TOP_shr_r, 
		 TOP_shr_i, 
		 TOP_shru_r, 
		 TOP_shru_i, 
		 TOP_slct_r, 
		 TOP_slct_i, 
		 TOP_slctf_r, 
		 TOP_slctf_i, 
		 TOP_sub_r, 
		 TOP_sub_i, 
		 TOP_sxtb_r, 
		 TOP_sxth_r, 
		 TOP_syscall, 
		 TOP_xor_r, 
		 TOP_xor_i, 
		 TOP_zxth_r, 
		 TOP_nop, 
		 TOP_mov_r, 
		 TOP_mov_i, 
		 TOP_mtb, 
		 TOP_mfb, 
		 TOP_mulh_r, 
		 TOP_mulh_i, 
		 TOP_mulhh_r, 
		 TOP_mulhh_i, 
		 TOP_mulhhs_r, 
		 TOP_mulhhs_i, 
		 TOP_mulhhu_r, 
		 TOP_mulhhu_i, 
		 TOP_mulhs_r, 
		 TOP_mulhs_i, 
		 TOP_mulhu_r, 
		 TOP_mulhu_i, 
		 TOP_mull_r, 
		 TOP_mull_i, 
		 TOP_mullh_r, 
		 TOP_mullh_i, 
		 TOP_mullhu_r, 
		 TOP_mullhu_i, 
		 TOP_mullhus_r, 
		 TOP_mullhus_i, 
		 TOP_mulll_r, 
		 TOP_mulll_i, 
		 TOP_mulllu_r, 
		 TOP_mulllu_i, 
		 TOP_mullu_r, 
		 TOP_mullu_i, 
		 TOP_ldb_i, 
		 TOP_ldb_d_i, 
		 TOP_ldbu_i, 
		 TOP_ldbu_d_i, 
		 TOP_ldh_i, 
		 TOP_ldh_d_i, 
		 TOP_ldhu_i, 
		 TOP_ldhu_d_i, 
		 TOP_ldw_i, 
		 TOP_ldw_d_i, 
		 TOP_sync, 
		 TOP_pft_i, 
		 TOP_prgadd_i, 
		 TOP_prgset_i, 
		 TOP_stb_i, 
		 TOP_sth_i, 
		 TOP_stw_i, 
		 TOP_imml, 
		 TOP_immr, 
		 TOP_br, 
		 TOP_brf, 
		 TOP_goto, 
		 TOP_igoto, 
		 TOP_return, 
		 TOP_call, 
		 TOP_icall, 
		 TOP_prgins, 
		 TOP_rfi, 
		 TOP_UNDEFINED); 

  /* ===== Specification for EXT0_Unit Type ===== */ 
  EXT0_Unit = ISA_Exec_Unit_Type_Create("EXT0_Unit", NULL); 
  Instruction_Exec_Unit_Group(EXT0_Unit, 
		 TOP_add_ii, 
		 TOP_and_ii, 
		 TOP_andc_ii, 
		 TOP_andl_ii_r, 
		 TOP_andl_ii_b, 
		 TOP_cmpeq_ii_r, 
		 TOP_cmpeq_ii_b, 
		 TOP_cmpge_ii_r, 
		 TOP_cmpge_ii_b, 
		 TOP_cmpgeu_ii_r, 
		 TOP_cmpgeu_ii_b, 
		 TOP_cmpgt_ii_r, 
		 TOP_cmpgt_ii_b, 
		 TOP_cmpgtu_ii_r, 
		 TOP_cmpgtu_ii_b, 
		 TOP_cmple_ii_r, 
		 TOP_cmple_ii_b, 
		 TOP_cmpleu_ii_r, 
		 TOP_cmpleu_ii_b, 
		 TOP_cmplt_ii_r, 
		 TOP_cmplt_ii_b, 
		 TOP_cmpltu_ii_r, 
		 TOP_cmpltu_ii_b, 
		 TOP_cmpne_ii_r, 
		 TOP_cmpne_ii_b, 
		 TOP_max_ii, 
		 TOP_maxu_ii, 
		 TOP_min_ii, 
		 TOP_minu_ii, 
		 TOP_nandl_ii_r, 
		 TOP_nandl_ii_b, 
		 TOP_norl_ii_r, 
		 TOP_norl_ii_b, 
		 TOP_or_ii, 
		 TOP_orc_ii, 
		 TOP_orl_ii_r, 
		 TOP_orl_ii_b, 
		 TOP_sh1add_ii, 
		 TOP_sh2add_ii, 
		 TOP_sh3add_ii, 
		 TOP_sh4add_ii, 
		 TOP_shl_ii, 
		 TOP_shr_ii, 
		 TOP_shru_ii, 
		 TOP_slct_ii, 
		 TOP_slctf_ii, 
		 TOP_sub_ii, 
		 TOP_xor_ii, 
		 TOP_mov_ii, 
		 TOP_mulh_ii, 
		 TOP_mulhh_ii, 
		 TOP_mulhhs_ii, 
		 TOP_mulhhu_ii, 
		 TOP_mulhs_ii, 
		 TOP_mulhu_ii, 
		 TOP_mull_ii, 
		 TOP_mullh_ii, 
		 TOP_mullhu_ii, 
		 TOP_mullhus_ii, 
		 TOP_mulll_ii, 
		 TOP_mulllu_ii, 
		 TOP_mullu_ii, 
		 TOP_ldb_ii, 
		 TOP_ldb_d_ii, 
		 TOP_ldbu_ii, 
		 TOP_ldbu_d_ii, 
		 TOP_ldh_ii, 
		 TOP_ldh_d_ii, 
		 TOP_ldhu_ii, 
		 TOP_ldhu_d_ii, 
		 TOP_ldw_ii, 
		 TOP_ldw_d_ii, 
		 TOP_pft_ii, 
		 TOP_prgadd_ii, 
		 TOP_prgset_ii, 
		 TOP_stb_ii, 
		 TOP_sth_ii, 
		 TOP_stw_ii, 
		 TOP_UNDEFINED); 

  /* ===== Specification for EXT1_Unit Type ===== */ 
  EXT1_Unit = ISA_Exec_Unit_Type_Create("EXT1_Unit", NULL); 
  Instruction_Exec_Unit_Group(EXT1_Unit, 
		 TOP_add_ii, 
		 TOP_and_ii, 
		 TOP_andc_ii, 
		 TOP_andl_ii_r, 
		 TOP_andl_ii_b, 
		 TOP_cmpeq_ii_r, 
		 TOP_cmpeq_ii_b, 
		 TOP_cmpge_ii_r, 
		 TOP_cmpge_ii_b, 
		 TOP_cmpgeu_ii_r, 
		 TOP_cmpgeu_ii_b, 
		 TOP_cmpgt_ii_r, 
		 TOP_cmpgt_ii_b, 
		 TOP_cmpgtu_ii_r, 
		 TOP_cmpgtu_ii_b, 
		 TOP_cmple_ii_r, 
		 TOP_cmple_ii_b, 
		 TOP_cmpleu_ii_r, 
		 TOP_cmpleu_ii_b, 
		 TOP_cmplt_ii_r, 
		 TOP_cmplt_ii_b, 
		 TOP_cmpltu_ii_r, 
		 TOP_cmpltu_ii_b, 
		 TOP_cmpne_ii_r, 
		 TOP_cmpne_ii_b, 
		 TOP_max_ii, 
		 TOP_maxu_ii, 
		 TOP_min_ii, 
		 TOP_minu_ii, 
		 TOP_nandl_ii_r, 
		 TOP_nandl_ii_b, 
		 TOP_norl_ii_r, 
		 TOP_norl_ii_b, 
		 TOP_or_ii, 
		 TOP_orc_ii, 
		 TOP_orl_ii_r, 
		 TOP_orl_ii_b, 
		 TOP_sh1add_ii, 
		 TOP_sh2add_ii, 
		 TOP_sh3add_ii, 
		 TOP_sh4add_ii, 
		 TOP_shl_ii, 
		 TOP_shr_ii, 
		 TOP_shru_ii, 
		 TOP_slct_ii, 
		 TOP_slctf_ii, 
		 TOP_sub_ii, 
		 TOP_xor_ii, 
		 TOP_mov_ii, 
		 TOP_mulh_ii, 
		 TOP_mulhh_ii, 
		 TOP_mulhhs_ii, 
		 TOP_mulhhu_ii, 
		 TOP_mulhs_ii, 
		 TOP_mulhu_ii, 
		 TOP_mull_ii, 
		 TOP_mullh_ii, 
		 TOP_mullhu_ii, 
		 TOP_mullhus_ii, 
		 TOP_mulll_ii, 
		 TOP_mulllu_ii, 
		 TOP_mullu_ii, 
		 TOP_ldb_ii, 
		 TOP_ldb_d_ii, 
		 TOP_ldbu_ii, 
		 TOP_ldbu_d_ii, 
		 TOP_ldh_ii, 
		 TOP_ldh_d_ii, 
		 TOP_ldhu_ii, 
		 TOP_ldhu_d_ii, 
		 TOP_ldw_ii, 
		 TOP_ldw_d_ii, 
		 TOP_pft_ii, 
		 TOP_prgadd_ii, 
		 TOP_prgset_ii, 
		 TOP_stb_ii, 
		 TOP_sth_ii, 
		 TOP_stw_ii, 
		 TOP_UNDEFINED); 

  /* ===== Specification for EXT2_Unit Type ===== */ 
  EXT2_Unit = ISA_Exec_Unit_Type_Create("EXT2_Unit", NULL); 
  Instruction_Exec_Unit_Group(EXT2_Unit, 
		 TOP_add_ii, 
		 TOP_and_ii, 
		 TOP_andc_ii, 
		 TOP_andl_ii_r, 
		 TOP_andl_ii_b, 
		 TOP_cmpeq_ii_r, 
		 TOP_cmpeq_ii_b, 
		 TOP_cmpge_ii_r, 
		 TOP_cmpge_ii_b, 
		 TOP_cmpgeu_ii_r, 
		 TOP_cmpgeu_ii_b, 
		 TOP_cmpgt_ii_r, 
		 TOP_cmpgt_ii_b, 
		 TOP_cmpgtu_ii_r, 
		 TOP_cmpgtu_ii_b, 
		 TOP_cmple_ii_r, 
		 TOP_cmple_ii_b, 
		 TOP_cmpleu_ii_r, 
		 TOP_cmpleu_ii_b, 
		 TOP_cmplt_ii_r, 
		 TOP_cmplt_ii_b, 
		 TOP_cmpltu_ii_r, 
		 TOP_cmpltu_ii_b, 
		 TOP_cmpne_ii_r, 
		 TOP_cmpne_ii_b, 
		 TOP_max_ii, 
		 TOP_maxu_ii, 
		 TOP_min_ii, 
		 TOP_minu_ii, 
		 TOP_nandl_ii_r, 
		 TOP_nandl_ii_b, 
		 TOP_norl_ii_r, 
		 TOP_norl_ii_b, 
		 TOP_or_ii, 
		 TOP_orc_ii, 
		 TOP_orl_ii_r, 
		 TOP_orl_ii_b, 
		 TOP_sh1add_ii, 
		 TOP_sh2add_ii, 
		 TOP_sh3add_ii, 
		 TOP_sh4add_ii, 
		 TOP_shl_ii, 
		 TOP_shr_ii, 
		 TOP_shru_ii, 
		 TOP_slct_ii, 
		 TOP_slctf_ii, 
		 TOP_sub_ii, 
		 TOP_xor_ii, 
		 TOP_mov_ii, 
		 TOP_mulh_ii, 
		 TOP_mulhh_ii, 
		 TOP_mulhhs_ii, 
		 TOP_mulhhu_ii, 
		 TOP_mulhs_ii, 
		 TOP_mulhu_ii, 
		 TOP_mull_ii, 
		 TOP_mullh_ii, 
		 TOP_mullhu_ii, 
		 TOP_mullhus_ii, 
		 TOP_mulll_ii, 
		 TOP_mulllu_ii, 
		 TOP_mullu_ii, 
		 TOP_ldb_ii, 
		 TOP_ldb_d_ii, 
		 TOP_ldbu_ii, 
		 TOP_ldbu_d_ii, 
		 TOP_ldh_ii, 
		 TOP_ldh_d_ii, 
		 TOP_ldhu_ii, 
		 TOP_ldhu_d_ii, 
		 TOP_ldw_ii, 
		 TOP_ldw_d_ii, 
		 TOP_pft_ii, 
		 TOP_prgadd_ii, 
		 TOP_prgset_ii, 
		 TOP_stb_ii, 
		 TOP_sth_ii, 
		 TOP_stw_ii, 
		 TOP_UNDEFINED); 

  /* ===== Specification for Odd_Unit Type ===== */ 
  Odd_Unit = ISA_Exec_Unit_Type_Create("Odd_Unit", NULL); 
  Instruction_Exec_Unit_Group(Odd_Unit, 
		 TOP_mulh_r, 
		 TOP_mulh_i, 
		 TOP_mulhh_r, 
		 TOP_mulhh_i, 
		 TOP_mulhhs_r, 
		 TOP_mulhhs_i, 
		 TOP_mulhhu_r, 
		 TOP_mulhhu_i, 
		 TOP_mulhs_r, 
		 TOP_mulhs_i, 
		 TOP_mulhu_r, 
		 TOP_mulhu_i, 
		 TOP_mull_r, 
		 TOP_mull_i, 
		 TOP_mullh_r, 
		 TOP_mullh_i, 
		 TOP_mullhu_r, 
		 TOP_mullhu_i, 
		 TOP_mullhus_r, 
		 TOP_mullhus_i, 
		 TOP_mulll_r, 
		 TOP_mulll_i, 
		 TOP_mulllu_r, 
		 TOP_mulllu_i, 
		 TOP_mullu_r, 
		 TOP_mullu_i, 
		 TOP_UNDEFINED); 

  /* ===== Specification for Even_Unit Type ===== */ 
  Even_Unit = ISA_Exec_Unit_Type_Create("Even_Unit", NULL); 
  Instruction_Exec_Unit_Group(Even_Unit, 
		 TOP_imml, 
		 TOP_immr, 
		 TOP_UNDEFINED); 

  /* ===== Specification for ReqS0_Unit Type ===== */ 
  ReqS0_Unit = ISA_Exec_Unit_Type_Create("ReqS0_Unit", NULL); 
  Instruction_Exec_Unit_Group(ReqS0_Unit, 
		 TOP_br, 
		 TOP_brf, 
		 TOP_goto, 
		 TOP_igoto, 
		 TOP_return, 
		 TOP_call, 
		 TOP_icall, 
		 TOP_prgins, 
		 TOP_rfi, 
		 TOP_UNDEFINED); 

  /* === All legal bundle orderings (11 of them) are specified below. */ 

  /* ===== Template 0x00 (0) ===== */ 
  ISA_Bundle_Type_Create("temp0", ".temp0", 4); 
  Slot(0, S0_Unit); 
  Slot(1, S1_Unit); 
  Slot(2, S2_Unit); 
  Slot(3, S3_Unit); 
  Stop(3); 

  /* ===== Template 0x00 (1) ===== */ 
  ISA_Bundle_Type_Create("temp1", ".temp1", 1); 
  Slot(0, S0_Unit); 
  Stop(0); 

  /* ===== Template 0x00 (2) ===== */ 
  ISA_Bundle_Type_Create("temp2", ".temp2", 2); 
  Slot(0, S0_Unit); 
  Slot(1, S1_Unit); 
  Stop(1); 

  /* ===== Template 0x00 (3) ===== */ 
  ISA_Bundle_Type_Create("temp3", ".temp3", 3); 
  Slot(0, S0_Unit); 
  Slot(1, S1_Unit); 
  Slot(2, S2_Unit); 
  Stop(2); 

  /* ===== Template 0x00 (4) ===== */ 
  ISA_Bundle_Type_Create("temp4", ".temp4", 4); 
  Slot(0, EXT0_Unit); 
  Slot(1, EXT0_Unit); 
  Slot(2, S2_Unit); 
  Slot(3, S3_Unit); 
  Stop(3); 

  /* ===== Template 0x00 (5) ===== */ 
  ISA_Bundle_Type_Create("temp5", ".temp5", 2); 
  Slot(0, EXT0_Unit); 
  Slot(1, EXT0_Unit); 
  Stop(1); 

  /* ===== Template 0x00 (6) ===== */ 
  ISA_Bundle_Type_Create("temp6", ".temp6", 3); 
  Slot(0, EXT0_Unit); 
  Slot(1, EXT0_Unit); 
  Slot(2, S2_Unit); 
  Stop(2); 

  /* ===== Template 0x00 (7) ===== */ 
  ISA_Bundle_Type_Create("temp7", ".temp7", 4); 
  Slot(0, EXT0_Unit); 
  Slot(1, EXT0_Unit); 
  Slot(2, EXT2_Unit); 
  Slot(3, EXT2_Unit); 
  Stop(3); 

  /* ===== Template 0x00 (8) ===== */ 
  ISA_Bundle_Type_Create("temp8", ".temp8", 4); 
  Slot(0, S0_Unit); 
  Slot(1, EXT1_Unit); 
  Slot(2, EXT1_Unit); 
  Slot(3, S3_Unit); 
  Stop(3); 

  /* ===== Template 0x00 (9) ===== */ 
  ISA_Bundle_Type_Create("temp9", ".temp9", 3); 
  Slot(0, S0_Unit); 
  Slot(1, EXT1_Unit); 
  Slot(2, EXT1_Unit); 
  Stop(2); 

  /* ===== Template 0x00 (10) ===== */ 
  ISA_Bundle_Type_Create("temp10", ".temp10", 4); 
  Slot(0, S0_Unit); 
  Slot(1, S1_Unit); 
  Slot(2, EXT2_Unit); 
  Slot(3, EXT2_Unit); 
  Stop(3); 


  ISA_Bundle_End(); 
  return 0; 
} 
