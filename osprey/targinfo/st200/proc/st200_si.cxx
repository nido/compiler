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

//  st200 processor scheduling information 
///////////////////////////////////// 
//   
//  Description:  
//  
//  Generate a scheduling description of a st200 processor  
//  via the si_gen interface.  
//  
/////////////////////////////////////  

#include "si_gen.h" 
#include "targ_isa_subset.h" 
#include "topcode.h" 

static RESOURCE  res_ISSUE,
	        res_INT,
	        res_MUL,
	        res_MEM,
	        res_ODD,
	        res_XFER; 

int 
main (int argc, char *argv[]) 
{ 
  res_ISSUE = RESOURCE_Create("ISSUE", 4); 
  res_INT = RESOURCE_Create("INT", 4); 
  res_MUL = RESOURCE_Create("MUL", 2); 
  res_MEM = RESOURCE_Create("MEM", 1); 
  res_ODD = RESOURCE_Create("ODD", 2); 
  res_XFER = RESOURCE_Create("XFER", 1); 

  /* ======================================================
   * Resource description for the ISA_SUBSET_st220 
   * ======================================================
   */ 

  Machine("st200", ISA_SUBSET_st220, argc, argv); 

  ///////////////////////////////////////// 
  //   Instructions for Scd Class ALU 
  ///////////////////////////////////////// 

  Instruction_Group("ALU", 
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
		 TOP_UNDEFINED); 

  Any_Result_Available_Time(3); 
  Any_Operand_Access_Time(2); 
  Resource_Requirement(res_ISSUE, 0); 
  Resource_Requirement(res_INT, 0); 


  ///////////////////////////////////////// 
  //   Instructions for Scd Class ALU_IMM 
  ///////////////////////////////////////// 

  Instruction_Group("ALU_IMM", 
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
		 TOP_UNDEFINED); 

  Any_Result_Available_Time(3); 
  Any_Operand_Access_Time(2); 
  Resource_Requirement(res_ISSUE, 0); 
  Resource_Requirement(res_ISSUE, 0); 
  Resource_Requirement(res_INT, 0); 
  Resource_Requirement(res_ODD, 0); 


  ///////////////////////////////////////// 
  //   Instructions for Scd Class MUL 
  ///////////////////////////////////////// 

  Instruction_Group("MUL", 
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

  Any_Result_Available_Time(5); 
  Any_Operand_Access_Time(2); 
  Resource_Requirement(res_ISSUE, 0); 
  Resource_Requirement(res_MUL, 0); 
  Resource_Requirement(res_ODD, 0); 


  ///////////////////////////////////////// 
  //   Instructions for Scd Class MUL_IMM 
  ///////////////////////////////////////// 

  Instruction_Group("MUL_IMM", 
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
		 TOP_UNDEFINED); 

  Any_Result_Available_Time(5); 
  Any_Operand_Access_Time(2); 
  Resource_Requirement(res_ISSUE, 0); 
  Resource_Requirement(res_ISSUE, 0); 
  Resource_Requirement(res_MUL, 0); 
  Resource_Requirement(res_ODD, 0); 


  ///////////////////////////////////////// 
  //   Instructions for Scd Class LOAD 
  ///////////////////////////////////////// 

  Instruction_Group("LOAD", 
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
		 TOP_UNDEFINED); 

  Any_Result_Available_Time(5); 
  Any_Operand_Access_Time(2); 
  Resource_Requirement(res_ISSUE, 0); 
  Resource_Requirement(res_MEM, 0); 


  ///////////////////////////////////////// 
  //   Instructions for Scd Class LOAD_IMM 
  ///////////////////////////////////////// 

  Instruction_Group("LOAD_IMM", 
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
		 TOP_UNDEFINED); 

  Any_Result_Available_Time(5); 
  Any_Operand_Access_Time(2); 
  Resource_Requirement(res_ISSUE, 0); 
  Resource_Requirement(res_ISSUE, 0); 
  Resource_Requirement(res_MEM, 0); 
  Resource_Requirement(res_ODD, 0); 


  ///////////////////////////////////////// 
  //   Instructions for Scd Class LOAD2 
  ///////////////////////////////////////// 

  Instruction_Group("LOAD2", 
		 TOP_UNDEFINED); 

  Any_Result_Available_Time(-1); 
  Any_Operand_Access_Time(-1); 


  ///////////////////////////////////////// 
  //   Instructions for Scd Class LOAD2_IMM 
  ///////////////////////////////////////// 

  Instruction_Group("LOAD2_IMM", 
		 TOP_UNDEFINED); 

  Any_Result_Available_Time(-1); 
  Any_Operand_Access_Time(-1); 


  ///////////////////////////////////////// 
  //   Instructions for Scd Class STORE 
  ///////////////////////////////////////// 

  Instruction_Group("STORE", 
		 TOP_pft_i, 
		 TOP_prgadd_i, 
		 TOP_prgset_i, 
		 TOP_stb_i, 
		 TOP_sth_i, 
		 TOP_stw_i, 
		 TOP_UNDEFINED); 

  Any_Operand_Access_Time(2); 
  Resource_Requirement(res_ISSUE, 0); 
  Resource_Requirement(res_MEM, 0); 


  ///////////////////////////////////////// 
  //   Instructions for Scd Class STORE_IMM 
  ///////////////////////////////////////// 

  Instruction_Group("STORE_IMM", 
		 TOP_pft_ii, 
		 TOP_prgadd_ii, 
		 TOP_prgset_ii, 
		 TOP_stb_ii, 
		 TOP_sth_ii, 
		 TOP_stw_ii, 
		 TOP_UNDEFINED); 

  Any_Operand_Access_Time(2); 
  Resource_Requirement(res_ISSUE, 0); 
  Resource_Requirement(res_ISSUE, 0); 
  Resource_Requirement(res_MEM, 0); 
  Resource_Requirement(res_ODD, 0); 


  ///////////////////////////////////////// 
  //   Instructions for Scd Class EXT 
  ///////////////////////////////////////// 

  Instruction_Group("EXT", 
		 TOP_imml, 
		 TOP_immr, 
		 TOP_UNDEFINED); 

  Any_Operand_Access_Time(1); 
  Resource_Requirement(res_ISSUE, 0); 
  Resource_Requirement(res_ODD, 0); 


  ///////////////////////////////////////// 
  //   Instructions for Scd Class BRANCH 
  ///////////////////////////////////////// 

  Instruction_Group("BRANCH", 
		 TOP_br, 
		 TOP_brf, 
		 TOP_UNDEFINED); 

  Any_Operand_Access_Time(0); 
  Resource_Requirement(res_ISSUE, 0); 
  Resource_Requirement(res_XFER, 0); 
  Resource_Requirement(res_ODD, 0); 


  ///////////////////////////////////////// 
  //   Instructions for Scd Class CALL 
  ///////////////////////////////////////// 

  Instruction_Group("CALL", 
		 TOP_call, 
		 TOP_icall, 
		 TOP_rfi, 
		 TOP_UNDEFINED); 

  Any_Result_Available_Time(5); 
  Any_Operand_Access_Time(1); 
  Resource_Requirement(res_ISSUE, 0); 
  Resource_Requirement(res_ODD, 0); 


  ///////////////////////////////////////// 
  //   Instructions for Scd Class JUMP 
  ///////////////////////////////////////// 

  Instruction_Group("JUMP", 
		 TOP_goto, 
		 TOP_igoto, 
		 TOP_return, 
		 TOP_UNDEFINED); 

  Any_Operand_Access_Time(1); 
  Resource_Requirement(res_ISSUE, 0); 
  Resource_Requirement(res_ODD, 0); 


  ///////////////////////////////////////// 
  //   Instructions for Scd Class SYNC 
  ///////////////////////////////////////// 

  Instruction_Group("SYNC", 
		 TOP_prgins, 
		 TOP_sync, 
		 TOP_UNDEFINED); 

  Resource_Requirement(res_ISSUE, 0); 
  Resource_Requirement(res_ISSUE, 0); 
  Resource_Requirement(res_ISSUE, 0); 
  Resource_Requirement(res_ISSUE, 0); 


  ///////////////////////////////////////// 
  //   Instructions for Scd Class simulated 
  ///////////////////////////////////////// 

  Instruction_Group("simulated", 
		 TOP_spadjust, 
		 TOP_UNDEFINED); 

  Any_Result_Available_Time(3); 
  Any_Operand_Access_Time(2); 
  Resource_Requirement(res_ISSUE, 0); 
  Resource_Requirement(res_INT, 0); 


  ///////////////////////////////////////// 
  //   Instructions for Scd Class asm 
  ///////////////////////////////////////// 

  Instruction_Group("asm", 
		 TOP_asm, 
		 TOP_UNDEFINED); 

  Any_Result_Available_Time(3); 
  Any_Operand_Access_Time(2); 
  Resource_Requirement(res_ISSUE, 0); 
  Resource_Requirement(res_ISSUE, 0); 
  Resource_Requirement(res_ISSUE, 0); 
  Resource_Requirement(res_ISSUE, 0); 


  ///////////////////////////////////////// 
  //   Instructions for Scd Class ssa 
  ///////////////////////////////////////// 

  Instruction_Group("ssa", 
		 TOP_phi, 
		 TOP_psi, 
		 TOP_UNDEFINED); 

  Any_Result_Available_Time(0); 
  Any_Operand_Access_Time(0); 
  Resource_Requirement(res_ISSUE, 0); 
  Resource_Requirement(res_INT, 0); 


  ///////////////////////////////////////// 
  //   Instructions for Scd Class getpc 
  ///////////////////////////////////////// 

  Instruction_Group("getpc", 
		 TOP_getpc, 
		 TOP_UNDEFINED); 

  Any_Result_Available_Time(2); 
  Any_Operand_Access_Time(1); 
  Resource_Requirement(res_ISSUE, 0); 
  Resource_Requirement(res_ISSUE, 0); 
  Resource_Requirement(res_ISSUE, 0); 
  Resource_Requirement(res_ISSUE, 0); 

  Machine_Done("st220.c"); 

  /* ======================================================
   * Resource description for the ISA_SUBSET_st200 
   * ======================================================
   */ 

  Machine("st200", ISA_SUBSET_st200, argc, argv); 

  ///////////////////////////////////////// 
  //   Instructions for Scd Class ALU 
  ///////////////////////////////////////// 

  Instruction_Group("ALU", 
		 TOP_UNDEFINED); 

  Any_Result_Available_Time(-1); 
  Any_Operand_Access_Time(-1); 


  ///////////////////////////////////////// 
  //   Instructions for Scd Class ALU_IMM 
  ///////////////////////////////////////// 

  Instruction_Group("ALU_IMM", 
		 TOP_UNDEFINED); 

  Any_Result_Available_Time(-1); 
  Any_Operand_Access_Time(-1); 


  ///////////////////////////////////////// 
  //   Instructions for Scd Class MUL 
  ///////////////////////////////////////// 

  Instruction_Group("MUL", 
		 TOP_UNDEFINED); 

  Any_Result_Available_Time(-1); 
  Any_Operand_Access_Time(-1); 


  ///////////////////////////////////////// 
  //   Instructions for Scd Class MUL_IMM 
  ///////////////////////////////////////// 

  Instruction_Group("MUL_IMM", 
		 TOP_UNDEFINED); 

  Any_Result_Available_Time(-1); 
  Any_Operand_Access_Time(-1); 


  ///////////////////////////////////////// 
  //   Instructions for Scd Class LOAD 
  ///////////////////////////////////////// 

  Instruction_Group("LOAD", 
		 TOP_UNDEFINED); 

  Any_Result_Available_Time(-1); 
  Any_Operand_Access_Time(-1); 


  ///////////////////////////////////////// 
  //   Instructions for Scd Class LOAD_IMM 
  ///////////////////////////////////////// 

  Instruction_Group("LOAD_IMM", 
		 TOP_UNDEFINED); 

  Any_Result_Available_Time(-1); 
  Any_Operand_Access_Time(-1); 


  ///////////////////////////////////////// 
  //   Instructions for Scd Class LOAD2 
  ///////////////////////////////////////// 

  Instruction_Group("LOAD2", 
		 TOP_UNDEFINED); 

  Any_Result_Available_Time(-1); 
  Any_Operand_Access_Time(-1); 


  ///////////////////////////////////////// 
  //   Instructions for Scd Class LOAD2_IMM 
  ///////////////////////////////////////// 

  Instruction_Group("LOAD2_IMM", 
		 TOP_UNDEFINED); 

  Any_Result_Available_Time(-1); 
  Any_Operand_Access_Time(-1); 


  ///////////////////////////////////////// 
  //   Instructions for Scd Class STORE 
  ///////////////////////////////////////// 

  Instruction_Group("STORE", 
		 TOP_UNDEFINED); 

  Any_Operand_Access_Time(-1); 


  ///////////////////////////////////////// 
  //   Instructions for Scd Class STORE_IMM 
  ///////////////////////////////////////// 

  Instruction_Group("STORE_IMM", 
		 TOP_UNDEFINED); 

  Any_Operand_Access_Time(-1); 


  ///////////////////////////////////////// 
  //   Instructions for Scd Class EXT 
  ///////////////////////////////////////// 

  Instruction_Group("EXT", 
		 TOP_UNDEFINED); 

  Any_Operand_Access_Time(-1); 


  ///////////////////////////////////////// 
  //   Instructions for Scd Class BRANCH 
  ///////////////////////////////////////// 

  Instruction_Group("BRANCH", 
		 TOP_UNDEFINED); 

  Any_Operand_Access_Time(-1); 


  ///////////////////////////////////////// 
  //   Instructions for Scd Class CALL 
  ///////////////////////////////////////// 

  Instruction_Group("CALL", 
		 TOP_UNDEFINED); 

  Any_Result_Available_Time(-1); 
  Any_Operand_Access_Time(-1); 


  ///////////////////////////////////////// 
  //   Instructions for Scd Class JUMP 
  ///////////////////////////////////////// 

  Instruction_Group("JUMP", 
		 TOP_UNDEFINED); 

  Any_Operand_Access_Time(-1); 


  ///////////////////////////////////////// 
  //   Instructions for Scd Class SYNC 
  ///////////////////////////////////////// 

  Instruction_Group("SYNC", 
		 TOP_UNDEFINED); 



  ///////////////////////////////////////// 
  //   Instructions for Scd Class simulated 
  ///////////////////////////////////////// 

  Instruction_Group("simulated", 
		 TOP_spadjust, 
		 TOP_UNDEFINED); 

  Any_Result_Available_Time(3); 
  Any_Operand_Access_Time(2); 
  Resource_Requirement(res_ISSUE, 0); 
  Resource_Requirement(res_INT, 0); 


  ///////////////////////////////////////// 
  //   Instructions for Scd Class asm 
  ///////////////////////////////////////// 

  Instruction_Group("asm", 
		 TOP_asm, 
		 TOP_UNDEFINED); 

  Any_Result_Available_Time(3); 
  Any_Operand_Access_Time(2); 
  Resource_Requirement(res_ISSUE, 0); 
  Resource_Requirement(res_ISSUE, 0); 
  Resource_Requirement(res_ISSUE, 0); 
  Resource_Requirement(res_ISSUE, 0); 


  ///////////////////////////////////////// 
  //   Instructions for Scd Class ssa 
  ///////////////////////////////////////// 

  Instruction_Group("ssa", 
		 TOP_phi, 
		 TOP_psi, 
		 TOP_UNDEFINED); 

  Any_Result_Available_Time(0); 
  Any_Operand_Access_Time(0); 
  Resource_Requirement(res_ISSUE, 0); 
  Resource_Requirement(res_INT, 0); 


  ///////////////////////////////////////// 
  //   Instructions for Scd Class getpc 
  ///////////////////////////////////////// 

  Instruction_Group("getpc", 
		 TOP_getpc, 
		 TOP_UNDEFINED); 

  Any_Result_Available_Time(2); 
  Any_Operand_Access_Time(1); 
  Resource_Requirement(res_ISSUE, 0); 
  Resource_Requirement(res_ISSUE, 0); 
  Resource_Requirement(res_ISSUE, 0); 
  Resource_Requirement(res_ISSUE, 0); 

  Machine_Done("st200.c"); 

}
