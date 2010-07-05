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
//  st240 processor scheduling information 
///////////////////////////////////// 
//   
//  Description:  
//  
//  Generate a scheduling description of a st240 processor  
//  via the si_gen interface.  
//  
/////////////////////////////////////  

#include "si_gen.h" 
#include "targ_isa_subset.h" 
#include "topcode.h" 

int 
main (int argc, char *argv[]) 
{ 
  RESOURCE Resource_st240_ISSUE = RESOURCE_Create("Resource_st240_ISSUE", 4);
  RESOURCE Resource_st240_MEM = RESOURCE_Create("Resource_st240_MEM", 1);
  RESOURCE Resource_st240_CTL = RESOURCE_Create("Resource_st240_CTL", 1);
  RESOURCE Resource_st240_ODD = RESOURCE_Create("Resource_st240_ODD", 2);
  RESOURCE Resource_st240_EVEN = RESOURCE_Create("Resource_st240_EVEN", 2);
  RESOURCE Resource_st240_LANE0 = RESOURCE_Create("Resource_st240_LANE0", 2);

  /* ======================================================
   * Resource description for the ISA_SUBSET_st240
   * ======================================================
   */

  Machine("st200", ISA_SUBSET_st240, argc, argv);

  Instruction_Group("group0",
		TOP_brf_i_b,
		TOP_br_i_b,
		TOP_UNDEFINED);

  Operand_Access_Time (0, 0);
  Resource_Requirement(Resource_st240_ISSUE, 0);
  Resource_Requirement(Resource_st240_CTL, 0);
  Resource_Requirement(Resource_st240_ODD, 0);
  Resource_Requirement(Resource_st240_EVEN, 0);


  Instruction_Group("group1",
		TOP_igoto,
		TOP_return,
		TOP_UNDEFINED);

  Operand_Access_Time (0, 1);
  Resource_Requirement(Resource_st240_ISSUE, 0);
  Resource_Requirement(Resource_st240_CTL, 0);
  Resource_Requirement(Resource_st240_ODD, 0);
  Resource_Requirement(Resource_st240_EVEN, 0);


  Instruction_Group("group2",
		TOP_pftc_r_ii_b,
		TOP_UNDEFINED);

  Operand_Access_Time (0, 2);
  Operand_Access_Time (2, 2);
  Resource_Requirement(Resource_st240_ISSUE, 0);
  Resource_Requirement(Resource_st240_ISSUE, 0);
  Resource_Requirement(Resource_st240_MEM, 0);
  Resource_Requirement(Resource_st240_ODD, 0);
  Resource_Requirement(Resource_st240_EVEN, 0);
  Resource_Requirement(Resource_st240_LANE0, 0);
  Resource_Requirement(Resource_st240_LANE0, 0);


  Instruction_Group("group3",
		TOP_pftc_r_i_b,
		TOP_UNDEFINED);

  Operand_Access_Time (0, 2);
  Operand_Access_Time (2, 2);
  Resource_Requirement(Resource_st240_ISSUE, 0);
  Resource_Requirement(Resource_st240_MEM, 0);
  Resource_Requirement(Resource_st240_EVEN, 0);
  Resource_Requirement(Resource_st240_LANE0, 0);


  Instruction_Group("group4",
		TOP_multi_stlc_r_b_r_ii,
		TOP_UNDEFINED);

  Operand_Access_Time (1, 2);
  Operand_Access_Time (2, 2);
  Operand_Access_Time (3, 2);
  Operand_Access_Time (4, 2);
  Resource_Requirement(Resource_st240_ISSUE, 0);
  Resource_Requirement(Resource_st240_ISSUE, 0);
  Resource_Requirement(Resource_st240_MEM, 0);
  Resource_Requirement(Resource_st240_ODD, 0);
  Resource_Requirement(Resource_st240_EVEN, 0);
  Resource_Requirement(Resource_st240_LANE0, 0);
  Resource_Requirement(Resource_st240_LANE0, 0);


  Instruction_Group("group5",
		TOP_multi_stlc_r_b_r_i,
		TOP_UNDEFINED);

  Operand_Access_Time (1, 2);
  Operand_Access_Time (2, 2);
  Operand_Access_Time (3, 2);
  Operand_Access_Time (4, 2);
  Resource_Requirement(Resource_st240_ISSUE, 0);
  Resource_Requirement(Resource_st240_MEM, 0);
  Resource_Requirement(Resource_st240_EVEN, 0);
  Resource_Requirement(Resource_st240_LANE0, 0);


  Instruction_Group("group6",
		TOP_multi_stl_r_r_ii,
		TOP_stbc_r_b_r_ii,
		TOP_sthc_r_b_r_ii,
		TOP_stlc_p_b_r_ii,
		TOP_stwc_r_b_r_ii,
		TOP_UNDEFINED);

  Operand_Access_Time (1, 2);
  Operand_Access_Time (2, 2);
  Operand_Access_Time (3, 2);
  Resource_Requirement(Resource_st240_ISSUE, 0);
  Resource_Requirement(Resource_st240_ISSUE, 0);
  Resource_Requirement(Resource_st240_MEM, 0);
  Resource_Requirement(Resource_st240_ODD, 0);
  Resource_Requirement(Resource_st240_EVEN, 0);
  Resource_Requirement(Resource_st240_LANE0, 0);
  Resource_Requirement(Resource_st240_LANE0, 0);


  Instruction_Group("group7",
		TOP_multi_stl_r_r_i,
		TOP_stbc_r_b_r_i,
		TOP_sthc_r_b_r_i,
		TOP_stlc_p_b_r_i,
		TOP_stwc_r_b_r_i,
		TOP_UNDEFINED);

  Operand_Access_Time (1, 2);
  Operand_Access_Time (2, 2);
  Operand_Access_Time (3, 2);
  Resource_Requirement(Resource_st240_ISSUE, 0);
  Resource_Requirement(Resource_st240_MEM, 0);
  Resource_Requirement(Resource_st240_EVEN, 0);
  Resource_Requirement(Resource_st240_LANE0, 0);


  Instruction_Group("group8",
		TOP_stb_r_r_ii,
		TOP_sth_r_r_ii,
		TOP_stl_p_r_ii,
		TOP_stw_r_r_ii,
		TOP_UNDEFINED);

  Operand_Access_Time (1, 2);
  Operand_Access_Time (2, 2);
  Resource_Requirement(Resource_st240_ISSUE, 0);
  Resource_Requirement(Resource_st240_ISSUE, 0);
  Resource_Requirement(Resource_st240_MEM, 0);
  Resource_Requirement(Resource_st240_ODD, 0);
  Resource_Requirement(Resource_st240_EVEN, 0);
  Resource_Requirement(Resource_st240_LANE0, 0);
  Resource_Requirement(Resource_st240_LANE0, 0);


  Instruction_Group("group9",
		TOP_stb_r_r_i,
		TOP_sth_r_r_i,
		TOP_stl_p_r_i,
		TOP_stw_r_r_i,
		TOP_UNDEFINED);

  Operand_Access_Time (1, 2);
  Operand_Access_Time (2, 2);
  Resource_Requirement(Resource_st240_ISSUE, 0);
  Resource_Requirement(Resource_st240_MEM, 0);
  Resource_Requirement(Resource_st240_EVEN, 0);
  Resource_Requirement(Resource_st240_LANE0, 0);


  Instruction_Group("group10",
		TOP_prginsadd_r_i,
		TOP_prginsadd_r_ii,
		TOP_prginsadd_l1_r_i,
		TOP_prginsadd_l1_r_ii,
		TOP_prginsset_r_i,
		TOP_prginsset_r_ii,
		TOP_prginsset_l1_r_i,
		TOP_prginsset_l1_r_ii,
		TOP_UNDEFINED);

  Operand_Access_Time (1, 2);
  Resource_Requirement(Resource_st240_ISSUE, 0);
  Resource_Requirement(Resource_st240_ISSUE, 0);
  Resource_Requirement(Resource_st240_ISSUE, 0);
  Resource_Requirement(Resource_st240_ISSUE, 0);


  Instruction_Group("group11",
		TOP_flushadd_r_ii,
		TOP_flushadd_l1_r_ii,
		TOP_invadd_r_ii,
		TOP_invadd_l1_r_ii,
		TOP_pft_r_ii,
		TOP_prgadd_r_ii,
		TOP_prgadd_l1_r_ii,
		TOP_prgset_r_ii,
		TOP_prgset_l1_r_ii,
		TOP_UNDEFINED);

  Operand_Access_Time (1, 2);
  Resource_Requirement(Resource_st240_ISSUE, 0);
  Resource_Requirement(Resource_st240_ISSUE, 0);
  Resource_Requirement(Resource_st240_MEM, 0);
  Resource_Requirement(Resource_st240_ODD, 0);
  Resource_Requirement(Resource_st240_EVEN, 0);
  Resource_Requirement(Resource_st240_LANE0, 0);
  Resource_Requirement(Resource_st240_LANE0, 0);


  Instruction_Group("group12",
		TOP_flushadd_r_i,
		TOP_flushadd_l1_r_i,
		TOP_invadd_r_i,
		TOP_invadd_l1_r_i,
		TOP_pft_r_i,
		TOP_prgadd_r_i,
		TOP_prgadd_l1_r_i,
		TOP_prgset_r_i,
		TOP_prgset_l1_r_i,
		TOP_UNDEFINED);

  Operand_Access_Time (1, 2);
  Resource_Requirement(Resource_st240_ISSUE, 0);
  Resource_Requirement(Resource_st240_MEM, 0);
  Resource_Requirement(Resource_st240_EVEN, 0);
  Resource_Requirement(Resource_st240_LANE0, 0);


  Instruction_Group("group13",
		TOP_break,
		TOP_dbgsbrk,
		TOP_nop,
		TOP_st240_sbrk_i,
		TOP_UNDEFINED);

  Resource_Requirement(Resource_st240_ISSUE, 0);


  Instruction_Group("group14",
		TOP_goto_i,
		TOP_idle,
		TOP_UNDEFINED);

  Resource_Requirement(Resource_st240_ISSUE, 0);
  Resource_Requirement(Resource_st240_CTL, 0);
  Resource_Requirement(Resource_st240_ODD, 0);
  Resource_Requirement(Resource_st240_EVEN, 0);


  Instruction_Group("group15",
		TOP_rfi,
		TOP_syncins,
		TOP_UNDEFINED);

  Resource_Requirement(Resource_st240_ISSUE, 0);
  Resource_Requirement(Resource_st240_CTL, 0);
  Resource_Requirement(Resource_st240_ODD, 0);
  Resource_Requirement(Resource_st240_EVEN, 0);
  Resource_Requirement(Resource_st240_MEM, 0);


  Instruction_Group("group16",
		TOP_asm,
		TOP_retention,
		TOP_st240_syscall_i,
		TOP_UNDEFINED);

  Resource_Requirement(Resource_st240_ISSUE, 0);
  Resource_Requirement(Resource_st240_ISSUE, 0);
  Resource_Requirement(Resource_st240_ISSUE, 0);
  Resource_Requirement(Resource_st240_ISSUE, 0);


  Instruction_Group("group17",
		TOP_dib,
		TOP_sync,
		TOP_waitl,
		TOP_wmb,
		TOP_UNDEFINED);

  Resource_Requirement(Resource_st240_ISSUE, 0);
  Resource_Requirement(Resource_st240_MEM, 0);
  Resource_Requirement(Resource_st240_EVEN, 0);
  Resource_Requirement(Resource_st240_LANE0, 0);


  Instruction_Group("group18",
		TOP_icall,
		TOP_UNDEFINED);

  Result_Available_Time (0, 3);
  Operand_Access_Time (0, 1);
  Resource_Requirement(Resource_st240_ISSUE, 0);
  Resource_Requirement(Resource_st240_CTL, 0);
  Resource_Requirement(Resource_st240_ODD, 0);
  Resource_Requirement(Resource_st240_EVEN, 0);


  Instruction_Group("group19",
		TOP_avgu_pb_r_r_b_r,
		TOP_extl_pb_r_r_b_r,
		TOP_extr_pb_r_r_b_r,
		TOP_slctf_r_r_b_r,
		TOP_slctf_pb_r_r_b_r,
		TOP_slct_pb_r_r_b_r,
		TOP_st240_slct_r_r_b_r,
		TOP_UNDEFINED);

  Result_Available_Time (0, 3);
  Operand_Access_Time (0, 2);
  Operand_Access_Time (1, 2);
  Operand_Access_Time (2, 2);
  Resource_Requirement(Resource_st240_ISSUE, 0);


  Instruction_Group("group20",
		TOP_absubu_pb_r_r_r,
		TOP_addso_r_r_r,
		TOP_adds_r_r_r,
		TOP_adds_ph_r_r_r,
		TOP_add_r_r_r,
		TOP_add_ph_r_r_r,
		TOP_andc_r_r_r,
		TOP_andl_r_r_b,
		TOP_andl_r_r_r,
		TOP_andl_b_b_b,
		TOP_and_r_r_r,
		TOP_asm_0_r_r_r,
		TOP_asm_1_r_r_r,
		TOP_asm_2_r_r_r,
		TOP_asm_3_r_r_r,
		TOP_asm_4_r_r_r,
		TOP_asm_5_r_r_r,
		TOP_asm_6_r_r_r,
		TOP_asm_7_r_r_r,
		TOP_cmpeqf_n_r_r_b,
		TOP_cmpeqf_n_r_r_r,
		TOP_cmpeq_r_r_b,
		TOP_cmpeq_r_r_r,
		TOP_cmpeq_pb_r_r_r,
		TOP_cmpeq_pb_r_r_b,
		TOP_cmpeq_ph_r_r_r,
		TOP_cmpeq_ph_r_r_b,
		TOP_cmpgef_n_r_r_b,
		TOP_cmpgef_n_r_r_r,
		TOP_cmpgeu_r_r_b,
		TOP_cmpgeu_r_r_r,
		TOP_cmpge_r_r_b,
		TOP_cmpge_r_r_r,
		TOP_cmpgtf_n_r_r_b,
		TOP_cmpgtf_n_r_r_r,
		TOP_cmpgtu_r_r_b,
		TOP_cmpgtu_r_r_r,
		TOP_cmpgtu_pb_r_r_r,
		TOP_cmpgtu_pb_r_r_b,
		TOP_cmpgt_r_r_b,
		TOP_cmpgt_r_r_r,
		TOP_cmpgt_ph_r_r_r,
		TOP_cmpgt_ph_r_r_b,
		TOP_cmplef_n_r_r_b,
		TOP_cmplef_n_r_r_r,
		TOP_cmpleu_r_r_b,
		TOP_cmpleu_r_r_r,
		TOP_cmple_r_r_b,
		TOP_cmple_r_r_r,
		TOP_cmpltf_n_r_r_b,
		TOP_cmpltf_n_r_r_r,
		TOP_cmpltu_r_r_b,
		TOP_cmpltu_r_r_r,
		TOP_cmplt_r_r_b,
		TOP_cmplt_r_r_r,
		TOP_cmpne_r_r_b,
		TOP_cmpne_r_r_r,
		TOP_composep,
		TOP_ext1_pb_r_r_r,
		TOP_ext2_pb_r_r_r,
		TOP_ext3_pb_r_r_r,
		TOP_maxu_r_r_r,
		TOP_max_r_r_r,
		TOP_max_ph_r_r_r,
		TOP_minu_r_r_r,
		TOP_min_r_r_r,
		TOP_min_ph_r_r_r,
		TOP_movc,
		TOP_movcf,
		TOP_nandl_r_r_b,
		TOP_nandl_r_r_r,
		TOP_nandl_b_b_b,
		TOP_norl_r_r_b,
		TOP_norl_r_r_r,
		TOP_norl_b_b_b,
		TOP_orc_r_r_r,
		TOP_orl_r_r_b,
		TOP_orl_r_r_r,
		TOP_orl_b_b_b,
		TOP_or_r_r_r,
		TOP_packrnp_phh_r_r_r,
		TOP_packsu_pb_r_r_r,
		TOP_packs_ph_r_r_r,
		TOP_pack_pb_r_r_r,
		TOP_pack_ph_r_r_r,
		TOP_perm_pb_r_r_r,
		TOP_rotl_r_r_r,
		TOP_sh1addso_r_r_r,
		TOP_sh1adds_r_r_r,
		TOP_sh1add_r_r_r,
		TOP_sh1subso_r_r_r,
		TOP_sh1subs_r_r_r,
		TOP_sh2add_r_r_r,
		TOP_sh3add_r_r_r,
		TOP_shl_r_r_r,
		TOP_shl_ph_r_r_r,
		TOP_shru_r_r_r,
		TOP_shr_r_r_r,
		TOP_shr_ph_r_r_r,
		TOP_shuffeve_pb_r_r_r,
		TOP_shuffodd_pb_r_r_r,
		TOP_shuff_pbh_r_r_r,
		TOP_shuff_pbl_r_r_r,
		TOP_shuff_phh_r_r_r,
		TOP_shuff_phl_r_r_r,
		TOP_slctf_i_r_b_r,
		TOP_slctf_pb_i_r_b_r,
		TOP_slct_i_r_b_r,
		TOP_slct_pb_i_r_b_r,
		TOP_subso_r_r_r,
		TOP_subs_r_r_r,
		TOP_subs_ph_r_r_r,
		TOP_sub_r_r_r,
		TOP_sub_ph_r_r_r,
		TOP_sxt_r_r_r,
		TOP_xor_r_r_r,
		TOP_zxt_r_r_r,
		TOP_UNDEFINED);

  Result_Available_Time (0, 3);
  Operand_Access_Time (0, 2);
  Operand_Access_Time (1, 2);
  Resource_Requirement(Resource_st240_ISSUE, 0);


  Instruction_Group("group21",
		TOP_slctf_ii_r_b_r,
		TOP_slctf_pb_ii_r_b_r,
		TOP_slct_ii_r_b_r,
		TOP_slct_pb_ii_r_b_r,
		TOP_UNDEFINED);

  Result_Available_Time (0, 3);
  Operand_Access_Time (0, 2);
  Operand_Access_Time (1, 2);
  Resource_Requirement(Resource_st240_ISSUE, 0);
  Resource_Requirement(Resource_st240_ISSUE, 0);
  Resource_Requirement(Resource_st240_ODD, 0);
  Resource_Requirement(Resource_st240_EVEN, 0);


  Instruction_Group("group22",
		TOP_abss_ph_r_r,
		TOP_add_i_r_r,
		TOP_spadjust,
		TOP_andc_i_r_r,
		TOP_and_i_r_r,
		TOP_asm_16_i_r_r,
		TOP_asm_17_i_r_r,
		TOP_asm_18_i_r_r,
		TOP_asm_19_i_r_r,
		TOP_asm_20_i_r_r,
		TOP_asm_21_i_r_r,
		TOP_asm_22_i_r_r,
		TOP_asm_23_i_r_r,
		TOP_bswap_r_r,
		TOP_clz_r_r,
		TOP_cmpeq_i_r_b,
		TOP_cmpeq_i_r_r,
		TOP_cmpgeu_i_r_b,
		TOP_cmpgeu_i_r_r,
		TOP_cmpge_i_r_b,
		TOP_cmpge_i_r_r,
		TOP_cmpgtu_i_r_b,
		TOP_cmpgtu_i_r_r,
		TOP_cmpgt_i_r_b,
		TOP_cmpgt_i_r_r,
		TOP_cmpleu_i_r_b,
		TOP_cmpleu_i_r_r,
		TOP_cmple_i_r_b,
		TOP_cmple_i_r_r,
		TOP_cmpltu_i_r_b,
		TOP_cmpltu_i_r_r,
		TOP_cmplt_i_r_b,
		TOP_cmplt_i_r_r,
		TOP_cmpne_i_r_b,
		TOP_cmpne_i_r_r,
		TOP_convbi_b_r,
		TOP_convib_r_b,
		TOP_extractlu_i_r_r,
		TOP_extractl_i_r_r,
		TOP_extractu_i_r_r,
		TOP_extract_i_r_r,
		TOP_maxu_i_r_r,
		TOP_max_i_r_r,
		TOP_mfb_b_r,
		TOP_minu_i_r_r,
		TOP_min_i_r_r,
		TOP_movp,
		TOP_mov_r_r,
		TOP_st240_mov_r_b,
		TOP_st240_mov_b_r,
		TOP_mov_b_b,
		TOP_mtb_r_b,
		TOP_or_i_r_r,
		TOP_perm_pb_i_r_r,
		TOP_rotl_i_r_r,
		TOP_satso_r_r,
		TOP_sats_r_r,
		TOP_sh1add_i_r_r,
		TOP_sh2add_i_r_r,
		TOP_sh3add_i_r_r,
		TOP_shl_i_r_r,
		TOP_shl_ph_i_r_r,
		TOP_shru_i_r_r,
		TOP_shr_i_r_r,
		TOP_shr_ph_i_r_r,
		TOP_sxtb_r_r,
		TOP_sxth_r_r,
		TOP_sxt_i_r_r,
		TOP_unpacku_pbh_r_r,
		TOP_unpacku_pbl_r_r,
		TOP_xor_i_r_r,
		TOP_zxtb_r_r,
		TOP_zxth_r_r,
		TOP_zxt_i_r_r,
		TOP_UNDEFINED);

  Result_Available_Time (0, 3);
  Operand_Access_Time (0, 2);
  Resource_Requirement(Resource_st240_ISSUE, 0);


  Instruction_Group("group23",
		TOP_add_ii_r_r,
		TOP_andc_ii_r_r,
		TOP_and_ii_r_r,
		TOP_asm_16_ii_r_r,
		TOP_asm_17_ii_r_r,
		TOP_asm_18_ii_r_r,
		TOP_asm_19_ii_r_r,
		TOP_asm_20_ii_r_r,
		TOP_asm_21_ii_r_r,
		TOP_asm_22_ii_r_r,
		TOP_asm_23_ii_r_r,
		TOP_cmpeq_ii_r_b,
		TOP_cmpeq_ii_r_r,
		TOP_cmpgeu_ii_r_b,
		TOP_cmpgeu_ii_r_r,
		TOP_cmpge_ii_r_b,
		TOP_cmpge_ii_r_r,
		TOP_cmpgtu_ii_r_b,
		TOP_cmpgtu_ii_r_r,
		TOP_cmpgt_ii_r_b,
		TOP_cmpgt_ii_r_r,
		TOP_cmpleu_ii_r_b,
		TOP_cmpleu_ii_r_r,
		TOP_cmple_ii_r_b,
		TOP_cmple_ii_r_r,
		TOP_cmpltu_ii_r_b,
		TOP_cmpltu_ii_r_r,
		TOP_cmplt_ii_r_b,
		TOP_cmplt_ii_r_r,
		TOP_cmpne_ii_r_b,
		TOP_cmpne_ii_r_r,
		TOP_extractlu_ii_r_r,
		TOP_extractl_ii_r_r,
		TOP_extractu_ii_r_r,
		TOP_extract_ii_r_r,
		TOP_maxu_ii_r_r,
		TOP_max_ii_r_r,
		TOP_minu_ii_r_r,
		TOP_min_ii_r_r,
		TOP_or_ii_r_r,
		TOP_perm_pb_ii_r_r,
		TOP_rotl_ii_r_r,
		TOP_sh1add_ii_r_r,
		TOP_sh2add_ii_r_r,
		TOP_sh3add_ii_r_r,
		TOP_shl_ii_r_r,
		TOP_shl_ph_ii_r_r,
		TOP_shru_ii_r_r,
		TOP_shr_ii_r_r,
		TOP_shr_ph_ii_r_r,
		TOP_sxt_ii_r_r,
		TOP_xor_ii_r_r,
		TOP_zxt_ii_r_r,
		TOP_UNDEFINED);

  Result_Available_Time (0, 3);
  Operand_Access_Time (0, 2);
  Resource_Requirement(Resource_st240_ISSUE, 0);
  Resource_Requirement(Resource_st240_ISSUE, 0);
  Resource_Requirement(Resource_st240_ODD, 0);
  Resource_Requirement(Resource_st240_EVEN, 0);


  Instruction_Group("group24",
		TOP_sub_r_i_r,
		TOP_UNDEFINED);

  Result_Available_Time (0, 3);
  Operand_Access_Time (1, 2);
  Resource_Requirement(Resource_st240_ISSUE, 0);


  Instruction_Group("group25",
		TOP_sub_r_ii_r,
		TOP_UNDEFINED);

  Result_Available_Time (0, 3);
  Operand_Access_Time (1, 2);
  Resource_Requirement(Resource_st240_ISSUE, 0);
  Resource_Requirement(Resource_st240_ISSUE, 0);
  Resource_Requirement(Resource_st240_ODD, 0);
  Resource_Requirement(Resource_st240_EVEN, 0);


  Instruction_Group("group26",
		TOP_addpc_i_r,
		TOP_mov_i_r,
		TOP_UNDEFINED);

  Result_Available_Time (0, 3);
  Resource_Requirement(Resource_st240_ISSUE, 0);


  Instruction_Group("group27",
		TOP_call_i,
		TOP_getpc,
		TOP_UNDEFINED);

  Result_Available_Time (0, 3);
  Resource_Requirement(Resource_st240_ISSUE, 0);
  Resource_Requirement(Resource_st240_CTL, 0);
  Resource_Requirement(Resource_st240_ODD, 0);
  Resource_Requirement(Resource_st240_EVEN, 0);


  Instruction_Group("group28",
		TOP_addpc_ii_r,
		TOP_mov_ii_r,
		TOP_UNDEFINED);

  Result_Available_Time (0, 3);
  Resource_Requirement(Resource_st240_ISSUE, 0);
  Resource_Requirement(Resource_st240_ISSUE, 0);
  Resource_Requirement(Resource_st240_ODD, 0);
  Resource_Requirement(Resource_st240_EVEN, 0);


  Instruction_Group("group29",
		TOP_addcg_b_r_r_b_r,
		TOP_UNDEFINED);

  Result_Available_Time (0, 3);
  Result_Available_Time (1, 3);
  Operand_Access_Time (0, 2);
  Operand_Access_Time (1, 2);
  Operand_Access_Time (2, 2);
  Resource_Requirement(Resource_st240_ISSUE, 0);


  Instruction_Group("group30",
		TOP_extractp,
		TOP_UNDEFINED);

  Result_Available_Time (0, 3);
  Result_Available_Time (1, 3);
  Operand_Access_Time (0, 2);
  Resource_Requirement(Resource_st240_ISSUE, 0);


  Instruction_Group("group31",
		TOP_pushregs,
		TOP_UNDEFINED);

  Result_Available_Time (0, 3);
  Result_Available_Time (1, 3);
  Operand_Access_Time (0, 2);
  Resource_Requirement(Resource_st240_ISSUE, 0);
  Resource_Requirement(Resource_st240_CTL, 0);
  Resource_Requirement(Resource_st240_ODD, 0);
  Resource_Requirement(Resource_st240_EVEN, 0);


  Instruction_Group("group32",
		TOP_avg4u_pb_r_r_b_r,
		TOP_UNDEFINED);

  Result_Available_Time (0, 4);
  Operand_Access_Time (0, 2);
  Operand_Access_Time (1, 2);
  Operand_Access_Time (2, 2);
  Resource_Requirement(Resource_st240_ISSUE, 0);


  Instruction_Group("group33",
		TOP_sadu_pb_r_r_r,
		TOP_shlso_r_r_r,
		TOP_shls_r_r_r,
		TOP_shls_ph_r_r_r,
		TOP_shrrne_ph_r_r_r,
		TOP_shrrnp_ph_r_r_r,
		TOP_UNDEFINED);

  Result_Available_Time (0, 4);
  Operand_Access_Time (0, 2);
  Operand_Access_Time (1, 2);
  Resource_Requirement(Resource_st240_ISSUE, 0);


  Instruction_Group("group34",
		TOP_shlso_i_r_r,
		TOP_shls_i_r_r,
		TOP_shls_ph_i_r_r,
		TOP_shrrne_ph_i_r_r,
		TOP_shrrnp_i_r_r,
		TOP_shrrnp_ph_i_r_r,
		TOP_UNDEFINED);

  Result_Available_Time (0, 4);
  Operand_Access_Time (0, 2);
  Resource_Requirement(Resource_st240_ISSUE, 0);


  Instruction_Group("group35",
		TOP_shlso_ii_r_r,
		TOP_shls_ii_r_r,
		TOP_shls_ph_ii_r_r,
		TOP_shrrne_ph_ii_r_r,
		TOP_shrrnp_ii_r_r,
		TOP_shrrnp_ph_ii_r_r,
		TOP_UNDEFINED);

  Result_Available_Time (0, 4);
  Operand_Access_Time (0, 2);
  Resource_Requirement(Resource_st240_ISSUE, 0);
  Resource_Requirement(Resource_st240_ISSUE, 0);
  Resource_Requirement(Resource_st240_ODD, 0);
  Resource_Requirement(Resource_st240_EVEN, 0);


  Instruction_Group("group36",
		TOP_addf_n_r_r_r,
		TOP_asm_10_r_r_r,
		TOP_asm_11_r_r_r,
		TOP_asm_8_r_r_r,
		TOP_asm_9_r_r_r,
		TOP_subf_n_r_r_r,
		TOP_UNDEFINED);

  Result_Available_Time (0, 5);
  Operand_Access_Time (0, 2);
  Operand_Access_Time (1, 2);
  Resource_Requirement(Resource_st240_ISSUE, 0);
  Resource_Requirement(Resource_st240_EVEN, 0);
  Resource_Requirement(Resource_st240_LANE0, 0);


  Instruction_Group("group37",
		TOP_divu_r_r_r,
		TOP_div_r_r_r,
		TOP_remu_r_r_r,
		TOP_rem_r_r_r,
		TOP_stwl_r_r_b,
		TOP_UNDEFINED);

  Result_Available_Time (0, 5);
  Operand_Access_Time (0, 2);
  Operand_Access_Time (1, 2);
  Resource_Requirement(Resource_st240_ISSUE, 0);
  Resource_Requirement(Resource_st240_MEM, 0);
  Resource_Requirement(Resource_st240_EVEN, 0);
  Resource_Requirement(Resource_st240_LANE0, 0);


  Instruction_Group("group38",
		TOP_asm_12_r_r_r,
		TOP_asm_13_r_r_r,
		TOP_asm_14_r_r_r,
		TOP_asm_15_r_r_r,
		TOP_mul32_r_r_r,
		TOP_mul64hu_r_r_r,
		TOP_mul64h_r_r_r,
		TOP_muladdus_pb_r_r_r,
		TOP_muladd_ph_r_r_r,
		TOP_mulfracadds_ph_r_r_r,
		TOP_mulfracrm_ph_r_r_r,
		TOP_mulfracrne_ph_r_r_r,
		TOP_mulfrac_r_r_r,
		TOP_mulf_n_r_r_r,
		TOP_mulhhu_r_r_r,
		TOP_mulhh_r_r_r,
		TOP_mulh_r_r_r,
		TOP_mullhu_r_r_r,
		TOP_mullh_r_r_r,
		TOP_mulllu_r_r_r,
		TOP_mulll_r_r_r,
		TOP_mull_r_r_r,
		TOP_mul_ph_r_r_r,
		TOP_UNDEFINED);

  Result_Available_Time (0, 5);
  Operand_Access_Time (0, 2);
  Operand_Access_Time (1, 2);
  Resource_Requirement(Resource_st240_ISSUE, 0);
  Resource_Requirement(Resource_st240_ODD, 0);


  Instruction_Group("group39",
		TOP_ldbc_r_ii_b_r,
		TOP_ldbuc_r_ii_b_r,
		TOP_ldhc_r_ii_b_r,
		TOP_ldhuc_r_ii_b_r,
		TOP_ldlc_r_ii_b_p,
		TOP_ldwc_r_ii_b_r,
		TOP_UNDEFINED);

  Result_Available_Time (0, 5);
  Operand_Access_Time (0, 2);
  Operand_Access_Time (2, 2);
  Resource_Requirement(Resource_st240_ISSUE, 0);
  Resource_Requirement(Resource_st240_ISSUE, 0);
  Resource_Requirement(Resource_st240_MEM, 0);
  Resource_Requirement(Resource_st240_ODD, 0);
  Resource_Requirement(Resource_st240_EVEN, 0);
  Resource_Requirement(Resource_st240_LANE0, 0);
  Resource_Requirement(Resource_st240_LANE0, 0);


  Instruction_Group("group40",
		TOP_ldbc_r_i_b_r,
		TOP_ldbuc_r_i_b_r,
		TOP_ldhc_r_i_b_r,
		TOP_ldhuc_r_i_b_r,
		TOP_ldlc_r_i_b_p,
		TOP_ldwc_r_i_b_r,
		TOP_UNDEFINED);

  Result_Available_Time (0, 5);
  Operand_Access_Time (0, 2);
  Operand_Access_Time (2, 2);
  Resource_Requirement(Resource_st240_ISSUE, 0);
  Resource_Requirement(Resource_st240_MEM, 0);
  Resource_Requirement(Resource_st240_EVEN, 0);
  Resource_Requirement(Resource_st240_LANE0, 0);


  Instruction_Group("group41",
		TOP_asm_24_i_r_r,
		TOP_asm_25_i_r_r,
		TOP_asm_26_i_r_r,
		TOP_asm_27_i_r_r,
		TOP_UNDEFINED);

  Result_Available_Time (0, 5);
  Operand_Access_Time (0, 2);
  Resource_Requirement(Resource_st240_ISSUE, 0);


  Instruction_Group("group42",
		TOP_convfi_n_r_r,
		TOP_convif_n_r_r,
		TOP_UNDEFINED);

  Result_Available_Time (0, 5);
  Operand_Access_Time (0, 2);
  Resource_Requirement(Resource_st240_ISSUE, 0);
  Resource_Requirement(Resource_st240_EVEN, 0);
  Resource_Requirement(Resource_st240_LANE0, 0);


  Instruction_Group("group43",
		TOP_pswmask_ii_r_r,
		TOP_UNDEFINED);

  Result_Available_Time (0, 5);
  Operand_Access_Time (0, 2);
  Resource_Requirement(Resource_st240_ISSUE, 0);
  Resource_Requirement(Resource_st240_ISSUE, 0);
  Resource_Requirement(Resource_st240_MEM, 0);
  Resource_Requirement(Resource_st240_ODD, 0);
  Resource_Requirement(Resource_st240_EVEN, 0);
  Resource_Requirement(Resource_st240_LANE0, 0);
  Resource_Requirement(Resource_st240_LANE0, 0);


  Instruction_Group("group44",
		TOP_asm_24_ii_r_r,
		TOP_asm_25_ii_r_r,
		TOP_asm_26_ii_r_r,
		TOP_asm_27_ii_r_r,
		TOP_asm_28_ii_r_r,
		TOP_asm_29_ii_r_r,
		TOP_asm_30_ii_r_r,
		TOP_asm_31_ii_r_r,
		TOP_mul32_ii_r_r,
		TOP_mul64hu_ii_r_r,
		TOP_mul64h_ii_r_r,
		TOP_mulfrac_ii_r_r,
		TOP_mulllu_ii_r_r,
		TOP_mulll_ii_r_r,
		TOP_st240_mull_ii_r_r,
		TOP_UNDEFINED);

  Result_Available_Time (0, 5);
  Operand_Access_Time (0, 2);
  Resource_Requirement(Resource_st240_ISSUE, 0);
  Resource_Requirement(Resource_st240_ISSUE, 0);
  Resource_Requirement(Resource_st240_ODD, 0);
  Resource_Requirement(Resource_st240_EVEN, 0);


  Instruction_Group("group45",
		TOP_ldwl_r_r,
		TOP_pswmask_i_r_r,
		TOP_UNDEFINED);

  Result_Available_Time (0, 5);
  Operand_Access_Time (0, 2);
  Resource_Requirement(Resource_st240_ISSUE, 0);
  Resource_Requirement(Resource_st240_MEM, 0);
  Resource_Requirement(Resource_st240_EVEN, 0);
  Resource_Requirement(Resource_st240_LANE0, 0);


  Instruction_Group("group46",
		TOP_asm_28_i_r_r,
		TOP_asm_29_i_r_r,
		TOP_asm_30_i_r_r,
		TOP_asm_31_i_r_r,
		TOP_mul32_i_r_r,
		TOP_mul64hu_i_r_r,
		TOP_mul64h_i_r_r,
		TOP_mulfrac_i_r_r,
		TOP_mulllu_i_r_r,
		TOP_mulll_i_r_r,
		TOP_mull_i_r_r,
		TOP_UNDEFINED);

  Result_Available_Time (0, 5);
  Operand_Access_Time (0, 2);
  Resource_Requirement(Resource_st240_ISSUE, 0);
  Resource_Requirement(Resource_st240_ODD, 0);


  Instruction_Group("group47",
		TOP_ldbu_r_ii_r,
		TOP_ldb_r_ii_r,
		TOP_ldhu_r_ii_r,
		TOP_ldh_r_ii_r,
		TOP_ldl_r_ii_p,
		TOP_ldw_r_ii_r,
		TOP_UNDEFINED);

  Result_Available_Time (0, 5);
  Operand_Access_Time (1, 2);
  Resource_Requirement(Resource_st240_ISSUE, 0);
  Resource_Requirement(Resource_st240_ISSUE, 0);
  Resource_Requirement(Resource_st240_MEM, 0);
  Resource_Requirement(Resource_st240_ODD, 0);
  Resource_Requirement(Resource_st240_EVEN, 0);
  Resource_Requirement(Resource_st240_LANE0, 0);
  Resource_Requirement(Resource_st240_LANE0, 0);


  Instruction_Group("group48",
		TOP_ldbu_r_i_r,
		TOP_ldb_r_i_r,
		TOP_ldhu_r_i_r,
		TOP_ldh_r_i_r,
		TOP_ldl_r_i_p,
		TOP_ldw_r_i_r,
		TOP_UNDEFINED);

  Result_Available_Time (0, 5);
  Operand_Access_Time (1, 2);
  Resource_Requirement(Resource_st240_ISSUE, 0);
  Resource_Requirement(Resource_st240_MEM, 0);
  Resource_Requirement(Resource_st240_EVEN, 0);
  Resource_Requirement(Resource_st240_LANE0, 0);


  Instruction_Group("group49",
		TOP_multi_ldlc_r_ii_b_r,
		TOP_UNDEFINED);

  Result_Available_Time (0, 5);
  Result_Available_Time (1, 5);
  Operand_Access_Time (0, 2);
  Operand_Access_Time (2, 2);
  Resource_Requirement(Resource_st240_ISSUE, 0);
  Resource_Requirement(Resource_st240_ISSUE, 0);
  Resource_Requirement(Resource_st240_MEM, 0);
  Resource_Requirement(Resource_st240_ODD, 0);
  Resource_Requirement(Resource_st240_EVEN, 0);
  Resource_Requirement(Resource_st240_LANE0, 0);
  Resource_Requirement(Resource_st240_LANE0, 0);


  Instruction_Group("group50",
		TOP_multi_ldlc_r_i_b_r,
		TOP_UNDEFINED);

  Result_Available_Time (0, 5);
  Result_Available_Time (1, 5);
  Operand_Access_Time (0, 2);
  Operand_Access_Time (2, 2);
  Resource_Requirement(Resource_st240_ISSUE, 0);
  Resource_Requirement(Resource_st240_MEM, 0);
  Resource_Requirement(Resource_st240_EVEN, 0);
  Resource_Requirement(Resource_st240_LANE0, 0);


  Instruction_Group("group51",
		TOP_multi_ldl_r_ii_r,
		TOP_UNDEFINED);

  Result_Available_Time (0, 5);
  Result_Available_Time (1, 5);
  Operand_Access_Time (1, 2);
  Resource_Requirement(Resource_st240_ISSUE, 0);
  Resource_Requirement(Resource_st240_ISSUE, 0);
  Resource_Requirement(Resource_st240_MEM, 0);
  Resource_Requirement(Resource_st240_ODD, 0);
  Resource_Requirement(Resource_st240_EVEN, 0);
  Resource_Requirement(Resource_st240_LANE0, 0);
  Resource_Requirement(Resource_st240_LANE0, 0);


  Instruction_Group("group52",
		TOP_multi_ldl_r_i_r,
		TOP_UNDEFINED);

  Result_Available_Time (0, 5);
  Result_Available_Time (1, 5);
  Operand_Access_Time (1, 2);
  Resource_Requirement(Resource_st240_ISSUE, 0);
  Resource_Requirement(Resource_st240_MEM, 0);
  Resource_Requirement(Resource_st240_EVEN, 0);
  Resource_Requirement(Resource_st240_LANE0, 0);


  Machine_Done("st240.c");

}