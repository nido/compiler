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
// Group TOPS with similar operands/results format. 
///////////////////////////////////////////////////////// 


#include <stddef.h> 
#include "topcode.h" 
#include "isa_operands_gen.h" 

main() 
{ 

  ISA_Operands_Begin("st200"); 

  /* Create the operand types: */ 

  OPERAND_VALUE_TYPE Opd_branch = 
      ISA_Reg_Opnd_Type_Create("Opd_branch", 
                               ISA_REGISTER_CLASS_branch, 
                               ISA_REGISTER_SUBCLASS_UNDEFINED, 
                               -1, UNSIGNED, INVALID); 
  OPERAND_VALUE_TYPE Opd_brknum = 
      ISA_Lit_Opnd_Type_Create("Opd_brknum", 
                               12, 
                               UNSIGNED, 
                               LC_brknum,
                               ISA_RELOC_UNDEFINED,
                               ISA_RELOC_UNDEFINED);
  OPERAND_VALUE_TYPE Opd_btarg = 
      ISA_Lit_Opnd_Type_Create("Opd_btarg", 
                               23, 
                               PCREL, 
                               LC_btarg,
                               ISA_RELOC_R_ST200_23_PCREL,
                               ISA_RELOC_R_ST200_23_PCREL,
                               ISA_RELOC_UNDEFINED);
  OPERAND_VALUE_TYPE Opd_integer = 
      ISA_Reg_Opnd_Type_Create("Opd_integer", 
                               ISA_REGISTER_CLASS_integer, 
                               ISA_REGISTER_SUBCLASS_UNDEFINED, 
                               -1, SIGNED, INVALID); 
  OPERAND_VALUE_TYPE Opd_isrc2 = 
      ISA_Lit_Opnd_Type_Create("Opd_isrc2", 
                               9, 
                               SIGNED, 
                               LC_isrc2,
                               ISA_RELOC_R_ST200_LO9,
                               ISA_RELOC_R_ST200_LO9,
                               ISA_RELOC_UNDEFINED);
  OPERAND_VALUE_TYPE Opd_link = 
      ISA_Reg_Opnd_Type_Create("Opd_link", 
                               ISA_REGISTER_CLASS_integer, 
                               ISA_REGISTER_SUBCLASS_link, 
                               -1, SIGNED, INVALID); 
  OPERAND_VALUE_TYPE Opd_nolink = 
      ISA_Reg_Opnd_Type_Create("Opd_nolink", 
                               ISA_REGISTER_CLASS_integer, 
                               ISA_REGISTER_SUBCLASS_nolink, 
                               -1, SIGNED, INVALID); 
  OPERAND_VALUE_TYPE Opd_nzpaired = 
      ISA_Reg_Opnd_Type_Create("Opd_nzpaired", 
                               ISA_REGISTER_CLASS_integer, 
                               ISA_REGISTER_SUBCLASS_nzpaired, 
                               64, SIGNED, INVALID); 
  OPERAND_VALUE_TYPE Opd_nzpairedfirst = 
      ISA_Reg_Opnd_Type_Create("Opd_nzpairedfirst", 
                               ISA_REGISTER_CLASS_integer, 
                               ISA_REGISTER_SUBCLASS_nzpairedfirst, 
                               -1, SIGNED, INVALID); 
  OPERAND_VALUE_TYPE Opd_nzpairedsecond = 
      ISA_Reg_Opnd_Type_Create("Opd_nzpairedsecond", 
                               ISA_REGISTER_CLASS_integer, 
                               ISA_REGISTER_SUBCLASS_nzpairedsecond, 
                               -1, SIGNED, INVALID); 
  OPERAND_VALUE_TYPE Opd_paired = 
      ISA_Reg_Opnd_Type_Create("Opd_paired", 
                               ISA_REGISTER_CLASS_integer, 
                               ISA_REGISTER_SUBCLASS_paired, 
                               64, SIGNED, INVALID); 
  OPERAND_VALUE_TYPE Opd_pairedfirst = 
      ISA_Reg_Opnd_Type_Create("Opd_pairedfirst", 
                               ISA_REGISTER_CLASS_integer, 
                               ISA_REGISTER_SUBCLASS_pairedfirst, 
                               -1, SIGNED, INVALID); 
  OPERAND_VALUE_TYPE Opd_pairedsecond = 
      ISA_Reg_Opnd_Type_Create("Opd_pairedsecond", 
                               ISA_REGISTER_CLASS_integer, 
                               ISA_REGISTER_SUBCLASS_pairedsecond, 
                               -1, SIGNED, INVALID); 
  OPERAND_VALUE_TYPE Opd_predicate = 
      ISA_Reg_Opnd_Type_Create("Opd_predicate", 
                               ISA_REGISTER_CLASS_branch, 
                               ISA_REGISTER_SUBCLASS_predicate, 
                               -1, UNSIGNED, INVALID); 
  OPERAND_VALUE_TYPE Opd_sbrknum = 
      ISA_Lit_Opnd_Type_Create("Opd_sbrknum", 
                               21, 
                               UNSIGNED, 
                               LC_sbrknum,
                               ISA_RELOC_UNDEFINED,
                               ISA_RELOC_UNDEFINED);
  OPERAND_VALUE_TYPE Opd_xsrc2 = 
      ISA_Lit_Opnd_Type_Create("Opd_xsrc2", 
                               32, 
                               PCREL, 
                               LC_xsrc2,
                               ISA_RELOC_R_ST200_HI23,
                               ISA_RELOC_R_ST200_LO9,
                               ISA_RELOC_R_ST200_HI23,
                               ISA_RELOC_R_ST200_GPREL_LO9,
                               ISA_RELOC_R_ST200_GPREL_HI23,
                               ISA_RELOC_R_ST200_GOTOFF_LO9,
                               ISA_RELOC_R_ST200_GOTOFF_HI23,
                               ISA_RELOC_R_ST200_NEGGPREL_LO9,
                               ISA_RELOC_R_ST200_NEGGPREL_HI23,
                               ISA_RELOC_UNDEFINED);
  /* Create the operand uses: */ 

  /* ------------------------------------------------------
   *   Following built-in use types must be specified: 
   *     1. base operand use for TOP_load, TOP_store; 
   *     2. offset operand use for TOP_load, TOP_store; 
   *     3. storeval operand use for TOP_store; 
   *     4. condition operand use for TOP_select; 
   *     5. uniq_res operand
   *     6. predicate operand
   * 
   *   Following built-in use types may be specified: 
   *     5. implicit operand use for TOPs when the operand is implicitely used; 
   * 
   *   Here you can specify any additional operand uses. 
   * ------------------------------------------------------
   */

  OPERAND_USE_TYPE base = Create_Operand_Use("base"); 
  OPERAND_USE_TYPE condition = Create_Operand_Use("condition"); 
  OPERAND_USE_TYPE multi = Create_Operand_Use("multi"); 
  OPERAND_USE_TYPE negoffset = Create_Operand_Use("negoffset"); 
  OPERAND_USE_TYPE offset = Create_Operand_Use("offset"); 
  OPERAND_USE_TYPE opnd1 = Create_Operand_Use("opnd1"); 
  OPERAND_USE_TYPE opnd2 = Create_Operand_Use("opnd2"); 
  OPERAND_USE_TYPE postincr = Create_Operand_Use("postincr"); 
  OPERAND_USE_TYPE predicate = Create_Operand_Use("predicate"); 
  OPERAND_USE_TYPE preincr = Create_Operand_Use("preincr"); 
  OPERAND_USE_TYPE scale_1 = Create_Operand_Use("scale_1"); 
  OPERAND_USE_TYPE scale_2 = Create_Operand_Use("scale_2"); 
  OPERAND_USE_TYPE storeval = Create_Operand_Use("storeval"); 
  OPERAND_USE_TYPE target = Create_Operand_Use("target"); 
  OPERAND_USE_TYPE uniq_res = Create_Operand_Use("uniq_res"); 

  /* ====================================== */ 
  Instruction_Group("O_0", 
		 TOP_intrncall,
		 TOP_begin_pregtn,
		 TOP_break,
		 TOP_bwd_bar,
		 TOP_COPY,
		 TOP_dbgsbrk,
		 TOP_dib,
		 TOP_end_pregtn,
		 TOP_fwd_bar,
		 TOP_asm,
		 TOP_idle,
		 TOP_ifixup,
		 TOP_KILL,
		 TOP_label,
		 TOP_nop,
		 TOP_phi,
		 TOP_prgins,
		 TOP_psi,
		 TOP_retention,
		 TOP_rfi,
		 TOP_SIGMA,
		 TOP_noop,
		 TOP_sync,
		 TOP_syncins,
		 TOP_waitl,
		 TOP_wmb,
		 TOP_UNDEFINED);


  /* ====================================== */ 
  Instruction_Group("O_1", 
		 TOP_brf_i_b,
		 TOP_br_i_b,
		 TOP_UNDEFINED);

  Operand (0, Opd_branch, condition);
  Operand (1, Opd_btarg, target);

  /* ====================================== */ 
  Instruction_Group("O_2", 
		 TOP_st240_sbrk_i,
		 TOP_st240_syscall_i,
		 TOP_UNDEFINED);

  Operand (0, Opd_brknum);

  /* ====================================== */ 
  Instruction_Group("O_3", 
		 TOP_goto_i,
		 TOP_UNDEFINED);

  Operand (0, Opd_btarg, target);

  /* ====================================== */ 
  Instruction_Group("O_4", 
		 TOP_pswclr_r,
		 TOP_pswset_r,
		 TOP_UNDEFINED);

  Operand (0, Opd_integer);

  /* ====================================== */ 
  Instruction_Group("O_5", 
		 TOP_flushadd_r_i,
		 TOP_flushadd_l1_r_i,
		 TOP_invadd_r_i,
		 TOP_invadd_l1_r_i,
		 TOP_pft_r_i,
		 TOP_prgadd_r_i,
		 TOP_prgadd_l1_r_i,
		 TOP_prginsadd_r_i,
		 TOP_prginsadd_l1_r_i,
		 TOP_prginspg_r_i,
		 TOP_prginsset_r_i,
		 TOP_prginsset_l1_r_i,
		 TOP_prgset_r_i,
		 TOP_prgset_l1_r_i,
		 TOP_UNDEFINED);

  Operand (0, Opd_isrc2, offset);
  Operand (1, Opd_integer, base);

  /* ====================================== */ 
  Instruction_Group("O_6", 
		 TOP_stb_r_r_i,
		 TOP_sth_r_r_i,
		 TOP_stw_r_r_i,
		 TOP_UNDEFINED);

  Operand (0, Opd_isrc2, offset);
  Operand (1, Opd_integer, base);
  Operand (2, Opd_integer, storeval);

  /* ====================================== */ 
  Instruction_Group("O_7", 
		 TOP_stl_p_r_i,
		 TOP_UNDEFINED);

  Operand (0, Opd_isrc2, offset);
  Operand (1, Opd_integer, base);
  Operand (2, Opd_paired, storeval);

  /* ====================================== */ 
  Instruction_Group("O_8", 
		 TOP_multi_stl_r_r_i,
		 TOP_UNDEFINED);

  Operand (0, Opd_isrc2, offset);
  Operand (1, Opd_integer, base);
  Operand (2, Opd_pairedfirst, storeval);
  Operand (3, Opd_pairedsecond, multi);

  /* ====================================== */ 
  Instruction_Group("O_9", 
		 TOP_stbc_r_b_r_i,
		 TOP_sthc_r_b_r_i,
		 TOP_stwc_r_b_r_i,
		 TOP_UNDEFINED);

  Operand (0, Opd_isrc2, offset);
  Operand (1, Opd_integer, base);
  Operand (2, Opd_predicate, predicate);
  Operand (3, Opd_integer, storeval);

  /* ====================================== */ 
  Instruction_Group("O_10", 
		 TOP_stlc_p_b_r_i,
		 TOP_UNDEFINED);

  Operand (0, Opd_isrc2, offset);
  Operand (1, Opd_integer, base);
  Operand (2, Opd_predicate, predicate);
  Operand (3, Opd_paired, storeval);

  /* ====================================== */ 
  Instruction_Group("O_11", 
		 TOP_multi_stlc_r_b_r_i,
		 TOP_UNDEFINED);

  Operand (0, Opd_isrc2, offset);
  Operand (1, Opd_integer, base);
  Operand (2, Opd_predicate, predicate);
  Operand (3, Opd_pairedfirst, storeval);
  Operand (4, Opd_pairedsecond, multi);

  /* ====================================== */ 
  Instruction_Group("O_12", 
		 TOP_igoto,
		 TOP_return,
		 TOP_UNDEFINED);

  Operand (0, Opd_link);

  /* ====================================== */ 
  Instruction_Group("O_13", 
		 TOP_pftc_r_i_b,
		 TOP_UNDEFINED);

  Operand (0, Opd_predicate, predicate);
  Operand (1, Opd_isrc2, offset);
  Operand (2, Opd_integer, base);

  /* ====================================== */ 
  Instruction_Group("O_14", 
		 TOP_pftc_r_ii_b,
		 TOP_UNDEFINED);

  Operand (0, Opd_predicate, predicate);
  Operand (1, Opd_xsrc2, offset);
  Operand (2, Opd_integer, base);

  /* ====================================== */ 
  Instruction_Group("O_15", 
		 TOP_sbrk_i,
		 TOP_syscall_i,
		 TOP_UNDEFINED);

  Operand (0, Opd_sbrknum);

  /* ====================================== */ 
  Instruction_Group("O_16", 
		 TOP_flushadd_r_ii,
		 TOP_flushadd_l1_r_ii,
		 TOP_invadd_r_ii,
		 TOP_invadd_l1_r_ii,
		 TOP_pft_r_ii,
		 TOP_prgadd_r_ii,
		 TOP_prgadd_l1_r_ii,
		 TOP_prginsadd_r_ii,
		 TOP_prginsadd_l1_r_ii,
		 TOP_prginspg_r_ii,
		 TOP_prginsset_r_ii,
		 TOP_prginsset_l1_r_ii,
		 TOP_prgset_r_ii,
		 TOP_prgset_l1_r_ii,
		 TOP_UNDEFINED);

  Operand (0, Opd_xsrc2, offset);
  Operand (1, Opd_integer, base);

  /* ====================================== */ 
  Instruction_Group("O_17", 
		 TOP_stb_r_r_ii,
		 TOP_sth_r_r_ii,
		 TOP_stw_r_r_ii,
		 TOP_UNDEFINED);

  Operand (0, Opd_xsrc2, offset);
  Operand (1, Opd_integer, base);
  Operand (2, Opd_integer, storeval);

  /* ====================================== */ 
  Instruction_Group("O_18", 
		 TOP_stl_p_r_ii,
		 TOP_UNDEFINED);

  Operand (0, Opd_xsrc2, offset);
  Operand (1, Opd_integer, base);
  Operand (2, Opd_paired, storeval);

  /* ====================================== */ 
  Instruction_Group("O_19", 
		 TOP_multi_stl_r_r_ii,
		 TOP_UNDEFINED);

  Operand (0, Opd_xsrc2, offset);
  Operand (1, Opd_integer, base);
  Operand (2, Opd_pairedfirst, storeval);
  Operand (3, Opd_pairedsecond, multi);

  /* ====================================== */ 
  Instruction_Group("O_20", 
		 TOP_stbc_r_b_r_ii,
		 TOP_sthc_r_b_r_ii,
		 TOP_stwc_r_b_r_ii,
		 TOP_UNDEFINED);

  Operand (0, Opd_xsrc2, offset);
  Operand (1, Opd_integer, base);
  Operand (2, Opd_predicate, predicate);
  Operand (3, Opd_integer, storeval);

  /* ====================================== */ 
  Instruction_Group("O_21", 
		 TOP_stlc_p_b_r_ii,
		 TOP_UNDEFINED);

  Operand (0, Opd_xsrc2, offset);
  Operand (1, Opd_integer, base);
  Operand (2, Opd_predicate, predicate);
  Operand (3, Opd_paired, storeval);

  /* ====================================== */ 
  Instruction_Group("O_22", 
		 TOP_multi_stlc_r_b_r_ii,
		 TOP_UNDEFINED);

  Operand (0, Opd_xsrc2, offset);
  Operand (1, Opd_integer, base);
  Operand (2, Opd_predicate, predicate);
  Operand (3, Opd_pairedfirst, storeval);
  Operand (4, Opd_pairedsecond, multi);

  /* ====================================== */ 
  Instruction_Group("O_23", 
		 TOP_mov_b_b,
		 TOP_UNDEFINED);

  Result (0, Opd_branch);
  Operand (0, Opd_branch, opnd1);

  /* ====================================== */ 
  Instruction_Group("O_24", 
		 TOP_andl_b_b_b,
		 TOP_nandl_b_b_b,
		 TOP_norl_b_b_b,
		 TOP_orl_b_b_b,
		 TOP_UNDEFINED);

  Result (0, Opd_branch);
  Operand (0, Opd_branch, opnd1);
  Operand (1, Opd_branch, opnd2);

  /* ====================================== */ 
  Instruction_Group("O_25", 
		 TOP_convib_r_b,
		 TOP_mov_r_b,
		 TOP_st240_mov_r_b,
		 TOP_mtb_r_b,
		 TOP_UNDEFINED);

  Result (0, Opd_branch);
  Operand (0, Opd_integer);

  /* ====================================== */ 
  Instruction_Group("O_26", 
		 TOP_stwl_r_r_b,
		 TOP_UNDEFINED);

  Result (0, Opd_branch);
  Operand (0, Opd_integer, base);
  Operand (1, Opd_integer, storeval);

  /* ====================================== */ 
  Instruction_Group("O_27", 
		 TOP_andl_r_r_b,
		 TOP_cmpeqf_n_r_r_b,
		 TOP_cmpeq_r_r_b,
		 TOP_cmpeq_pb_r_r_b,
		 TOP_cmpeq_ph_r_r_b,
		 TOP_cmpgef_n_r_r_b,
		 TOP_cmpgeu_r_r_b,
		 TOP_cmpge_r_r_b,
		 TOP_cmpgtf_n_r_r_b,
		 TOP_cmpgtu_r_r_b,
		 TOP_cmpgtu_pb_r_r_b,
		 TOP_cmpgt_r_r_b,
		 TOP_cmpgt_ph_r_r_b,
		 TOP_cmplef_n_r_r_b,
		 TOP_cmpleu_r_r_b,
		 TOP_cmple_r_r_b,
		 TOP_cmpltf_n_r_r_b,
		 TOP_cmpltu_r_r_b,
		 TOP_cmplt_r_r_b,
		 TOP_cmpne_r_r_b,
		 TOP_nandl_r_r_b,
		 TOP_norl_r_r_b,
		 TOP_orl_r_r_b,
		 TOP_UNDEFINED);

  Result (0, Opd_branch);
  Operand (0, Opd_integer, opnd1);
  Operand (1, Opd_integer, opnd2);

  /* ====================================== */ 
  Instruction_Group("O_28", 
		 TOP_andl_i_r_b,
		 TOP_cmpeq_i_r_b,
		 TOP_cmpgeu_i_r_b,
		 TOP_cmpge_i_r_b,
		 TOP_cmpgtu_i_r_b,
		 TOP_cmpgt_i_r_b,
		 TOP_cmpleu_i_r_b,
		 TOP_cmple_i_r_b,
		 TOP_cmpltu_i_r_b,
		 TOP_cmplt_i_r_b,
		 TOP_cmpne_i_r_b,
		 TOP_nandl_i_r_b,
		 TOP_norl_i_r_b,
		 TOP_orl_i_r_b,
		 TOP_UNDEFINED);

  Result (0, Opd_branch);
  Operand (0, Opd_integer, opnd1);
  Operand (1, Opd_isrc2, opnd2);

  /* ====================================== */ 
  Instruction_Group("O_29", 
		 TOP_andl_ii_r_b,
		 TOP_cmpeq_ii_r_b,
		 TOP_cmpgeu_ii_r_b,
		 TOP_cmpge_ii_r_b,
		 TOP_cmpgtu_ii_r_b,
		 TOP_cmpgt_ii_r_b,
		 TOP_cmpleu_ii_r_b,
		 TOP_cmple_ii_r_b,
		 TOP_cmpltu_ii_r_b,
		 TOP_cmplt_ii_r_b,
		 TOP_cmpne_ii_r_b,
		 TOP_nandl_ii_r_b,
		 TOP_norl_ii_r_b,
		 TOP_orl_ii_r_b,
		 TOP_UNDEFINED);

  Result (0, Opd_branch);
  Operand (0, Opd_integer, opnd1);
  Operand (1, Opd_xsrc2, opnd2);

  /* ====================================== */ 
  Instruction_Group("O_30", 
		 TOP_convbi_b_r,
		 TOP_mfb_b_r,
		 TOP_mov_b_r,
		 TOP_st240_mov_b_r,
		 TOP_UNDEFINED);

  Result (0, Opd_integer);
  Operand (0, Opd_branch);

  /* ====================================== */ 
  Instruction_Group("O_31", 
		 TOP_avgu_pb_r_r_b_r,
		 TOP_extl_pb_r_r_b_r,
		 TOP_extr_pb_r_r_b_r,
		 TOP_slctf_pb_r_r_b_r,
		 TOP_slct_pb_r_r_b_r,
		 TOP_UNDEFINED);

  Result (0, Opd_integer);
  Operand (0, Opd_branch);
  Operand (1, Opd_integer);
  Operand (2, Opd_integer);

  /* ====================================== */ 
  Instruction_Group("O_32", 
		 TOP_slctf_pb_i_r_b_r,
		 TOP_slct_pb_i_r_b_r,
		 TOP_UNDEFINED);

  Result (0, Opd_integer);
  Operand (0, Opd_branch);
  Operand (1, Opd_integer);
  Operand (2, Opd_isrc2);

  /* ====================================== */ 
  Instruction_Group("O_33", 
		 TOP_slctf_pb_ii_r_b_r,
		 TOP_slct_pb_ii_r_b_r,
		 TOP_UNDEFINED);

  Result (0, Opd_integer);
  Operand (0, Opd_branch);
  Operand (1, Opd_integer);
  Operand (2, Opd_xsrc2);

  /* ====================================== */ 
  Instruction_Group("O_34", 
		 TOP_slctf_r_r_b_r,
		 TOP_slct_r_r_b_r,
		 TOP_st240_slct_r_r_b_r,
		 TOP_UNDEFINED);

  Result (0, Opd_integer);
  Operand (0, Opd_branch, condition);
  Operand (1, Opd_integer, opnd1);
  Operand (2, Opd_integer, opnd2);

  /* ====================================== */ 
  Instruction_Group("O_35", 
		 TOP_slctf_i_r_b_r,
		 TOP_slct_i_r_b_r,
		 TOP_UNDEFINED);

  Result (0, Opd_integer);
  Operand (0, Opd_branch, condition);
  Operand (1, Opd_integer, opnd1);
  Operand (2, Opd_isrc2, opnd2);

  /* ====================================== */ 
  Instruction_Group("O_36", 
		 TOP_slctf_ii_r_b_r,
		 TOP_slct_ii_r_b_r,
		 TOP_UNDEFINED);

  Result (0, Opd_integer);
  Operand (0, Opd_branch, condition);
  Operand (1, Opd_integer, opnd1);
  Operand (2, Opd_xsrc2, opnd2);

  /* ====================================== */ 
  Instruction_Group("O_37", 
		 TOP_ldwl_r_r,
		 TOP_UNDEFINED);

  Result (0, Opd_integer);
  Operand (0, Opd_integer, base);

  /* ====================================== */ 
  Instruction_Group("O_38", 
		 TOP_abss_ph_r_r,
		 TOP_bswap_r_r,
		 TOP_clz_r_r,
		 TOP_mov_r_r,
		 TOP_satso_r_r,
		 TOP_sats_r_r,
		 TOP_sxtb_r_r,
		 TOP_sxth_r_r,
		 TOP_unpacku_pbh_r_r,
		 TOP_unpacku_pbl_r_r,
		 TOP_zxtb_r_r,
		 TOP_zxth_r_r,
		 TOP_UNDEFINED);

  Result (0, Opd_integer);
  Operand (0, Opd_integer, opnd1);

  /* ====================================== */ 
  Instruction_Group("O_39", 
		 TOP_absubu_pb_r_r_r,
		 TOP_addso_r_r_r,
		 TOP_adds_r_r_r,
		 TOP_adds_ph_r_r_r,
		 TOP_add_r_r_r,
		 TOP_add_ph_r_r_r,
		 TOP_andc_r_r_r,
		 TOP_andl_r_r_r,
		 TOP_and_r_r_r,
		 TOP_asm_0_r_r_r,
		 TOP_asm_10_r_r_r,
		 TOP_asm_11_r_r_r,
		 TOP_asm_12_r_r_r,
		 TOP_asm_13_r_r_r,
		 TOP_asm_14_r_r_r,
		 TOP_asm_15_r_r_r,
		 TOP_asm_1_r_r_r,
		 TOP_asm_2_r_r_r,
		 TOP_asm_3_r_r_r,
		 TOP_asm_4_r_r_r,
		 TOP_asm_5_r_r_r,
		 TOP_asm_6_r_r_r,
		 TOP_asm_7_r_r_r,
		 TOP_asm_8_r_r_r,
		 TOP_asm_9_r_r_r,
		 TOP_cmpeqf_n_r_r_r,
		 TOP_cmpeq_r_r_r,
		 TOP_cmpeq_pb_r_r_r,
		 TOP_cmpeq_ph_r_r_r,
		 TOP_cmpgef_n_r_r_r,
		 TOP_cmpgeu_r_r_r,
		 TOP_cmpge_r_r_r,
		 TOP_cmpgtf_n_r_r_r,
		 TOP_cmpgtu_r_r_r,
		 TOP_cmpgtu_pb_r_r_r,
		 TOP_cmpgt_r_r_r,
		 TOP_cmpgt_ph_r_r_r,
		 TOP_cmplef_n_r_r_r,
		 TOP_cmpleu_r_r_r,
		 TOP_cmple_r_r_r,
		 TOP_cmpltf_n_r_r_r,
		 TOP_cmpltu_r_r_r,
		 TOP_cmplt_r_r_r,
		 TOP_cmpne_r_r_r,
		 TOP_ext1_pb_r_r_r,
		 TOP_ext2_pb_r_r_r,
		 TOP_ext3_pb_r_r_r,
		 TOP_maxu_r_r_r,
		 TOP_max_r_r_r,
		 TOP_max_ph_r_r_r,
		 TOP_minu_r_r_r,
		 TOP_min_r_r_r,
		 TOP_min_ph_r_r_r,
		 TOP_nandl_r_r_r,
		 TOP_norl_r_r_r,
		 TOP_orc_r_r_r,
		 TOP_orl_r_r_r,
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
		 TOP_sh4add_r_r_r,
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
		 TOP_subso_r_r_r,
		 TOP_subs_r_r_r,
		 TOP_subs_ph_r_r_r,
		 TOP_sub_r_r_r,
		 TOP_sub_ph_r_r_r,
		 TOP_sxt_r_r_r,
		 TOP_xor_r_r_r,
		 TOP_zxt_r_r_r,
		 TOP_UNDEFINED);

  Result (0, Opd_integer);
  Operand (0, Opd_integer, opnd1);
  Operand (1, Opd_integer, opnd2);

  /* ====================================== */ 
  Instruction_Group("O_40", 
		 TOP_add_i_r_r,
		 TOP_spadjust,
		 TOP_andc_i_r_r,
		 TOP_andl_i_r_r,
		 TOP_and_i_r_r,
		 TOP_asm_16_i_r_r,
		 TOP_asm_17_i_r_r,
		 TOP_asm_18_i_r_r,
		 TOP_asm_19_i_r_r,
		 TOP_asm_20_i_r_r,
		 TOP_asm_21_i_r_r,
		 TOP_asm_22_i_r_r,
		 TOP_asm_23_i_r_r,
		 TOP_asm_24_i_r_r,
		 TOP_asm_25_i_r_r,
		 TOP_asm_26_i_r_r,
		 TOP_asm_27_i_r_r,
		 TOP_asm_28_i_r_r,
		 TOP_asm_29_i_r_r,
		 TOP_asm_30_i_r_r,
		 TOP_asm_31_i_r_r,
		 TOP_cmpeq_i_r_r,
		 TOP_cmpgeu_i_r_r,
		 TOP_cmpge_i_r_r,
		 TOP_cmpgtu_i_r_r,
		 TOP_cmpgt_i_r_r,
		 TOP_cmpleu_i_r_r,
		 TOP_cmple_i_r_r,
		 TOP_cmpltu_i_r_r,
		 TOP_cmplt_i_r_r,
		 TOP_cmpne_i_r_r,
		 TOP_extractlu_i_r_r,
		 TOP_extractl_i_r_r,
		 TOP_extractu_i_r_r,
		 TOP_extract_i_r_r,
		 TOP_maxu_i_r_r,
		 TOP_max_i_r_r,
		 TOP_minu_i_r_r,
		 TOP_min_i_r_r,
		 TOP_nandl_i_r_r,
		 TOP_norl_i_r_r,
		 TOP_orc_i_r_r,
		 TOP_orl_i_r_r,
		 TOP_or_i_r_r,
		 TOP_perm_pb_i_r_r,
		 TOP_rotl_i_r_r,
		 TOP_sh1add_i_r_r,
		 TOP_sh2add_i_r_r,
		 TOP_sh3add_i_r_r,
		 TOP_sh4add_i_r_r,
		 TOP_shl_i_r_r,
		 TOP_shl_ph_i_r_r,
		 TOP_shru_i_r_r,
		 TOP_shr_i_r_r,
		 TOP_shr_ph_i_r_r,
		 TOP_sxt_i_r_r,
		 TOP_xor_i_r_r,
		 TOP_zxt_i_r_r,
		 TOP_UNDEFINED);

  Result (0, Opd_integer);
  Operand (0, Opd_integer, opnd1);
  Operand (1, Opd_isrc2, opnd2);

  /* ====================================== */ 
  Instruction_Group("O_41", 
		 TOP_add_ii_r_r,
		 TOP_andc_ii_r_r,
		 TOP_andl_ii_r_r,
		 TOP_and_ii_r_r,
		 TOP_asm_16_ii_r_r,
		 TOP_asm_17_ii_r_r,
		 TOP_asm_18_ii_r_r,
		 TOP_asm_19_ii_r_r,
		 TOP_asm_20_ii_r_r,
		 TOP_asm_21_ii_r_r,
		 TOP_asm_22_ii_r_r,
		 TOP_asm_23_ii_r_r,
		 TOP_asm_24_ii_r_r,
		 TOP_asm_25_ii_r_r,
		 TOP_asm_26_ii_r_r,
		 TOP_asm_27_ii_r_r,
		 TOP_asm_28_ii_r_r,
		 TOP_asm_29_ii_r_r,
		 TOP_asm_30_ii_r_r,
		 TOP_asm_31_ii_r_r,
		 TOP_cmpeq_ii_r_r,
		 TOP_cmpgeu_ii_r_r,
		 TOP_cmpge_ii_r_r,
		 TOP_cmpgtu_ii_r_r,
		 TOP_cmpgt_ii_r_r,
		 TOP_cmpleu_ii_r_r,
		 TOP_cmple_ii_r_r,
		 TOP_cmpltu_ii_r_r,
		 TOP_cmplt_ii_r_r,
		 TOP_cmpne_ii_r_r,
		 TOP_extractlu_ii_r_r,
		 TOP_extractl_ii_r_r,
		 TOP_extractu_ii_r_r,
		 TOP_extract_ii_r_r,
		 TOP_maxu_ii_r_r,
		 TOP_max_ii_r_r,
		 TOP_minu_ii_r_r,
		 TOP_min_ii_r_r,
		 TOP_nandl_ii_r_r,
		 TOP_norl_ii_r_r,
		 TOP_orc_ii_r_r,
		 TOP_orl_ii_r_r,
		 TOP_or_ii_r_r,
		 TOP_perm_pb_ii_r_r,
		 TOP_rotl_ii_r_r,
		 TOP_sh1add_ii_r_r,
		 TOP_sh2add_ii_r_r,
		 TOP_sh3add_ii_r_r,
		 TOP_sh4add_ii_r_r,
		 TOP_shl_ii_r_r,
		 TOP_shl_ph_ii_r_r,
		 TOP_shru_ii_r_r,
		 TOP_shr_ii_r_r,
		 TOP_shr_ph_ii_r_r,
		 TOP_sxt_ii_r_r,
		 TOP_xor_ii_r_r,
		 TOP_zxt_ii_r_r,
		 TOP_UNDEFINED);

  Result (0, Opd_integer);
  Operand (0, Opd_integer, opnd1);
  Operand (1, Opd_xsrc2, opnd2);

  /* ====================================== */ 
  Instruction_Group("O_42", 
		 TOP_ldw_r_i_r,
		 TOP_ldw_d_r_i_r,
		 TOP_UNDEFINED);

  Result (0, Opd_integer);
  Operand (0, Opd_isrc2, offset);
  Operand (1, Opd_integer, base);

  /* ====================================== */ 
  Instruction_Group("O_43", 
		 TOP_addpc_i_r,
		 TOP_mov_i_r,
		 TOP_UNDEFINED);

  Result (0, Opd_integer);
  Operand (0, Opd_isrc2, opnd1);

  /* ====================================== */ 
  Instruction_Group("O_44", 
		 TOP_sub_r_i_r,
		 TOP_UNDEFINED);

  Result (0, Opd_integer);
  Operand (0, Opd_isrc2, opnd1);
  Operand (1, Opd_integer, opnd2);

  /* ====================================== */ 
  Instruction_Group("O_45", 
		 TOP_movc,
		 TOP_movcf,
		 TOP_UNDEFINED);

  Result (0, Opd_integer);
  Operand (0, Opd_predicate, predicate);
  Operand (1, Opd_integer);

  /* ====================================== */ 
  Instruction_Group("O_46", 
		 TOP_ldwc_r_i_b_r,
		 TOP_UNDEFINED);

  Result (0, Opd_integer);
  Operand (0, Opd_predicate, predicate);
  Operand (1, Opd_isrc2, offset);
  Operand (2, Opd_integer, base);

  /* ====================================== */ 
  Instruction_Group("O_47", 
		 TOP_ldwc_r_ii_b_r,
		 TOP_UNDEFINED);

  Result (0, Opd_integer);
  Operand (0, Opd_predicate, predicate);
  Operand (1, Opd_xsrc2, offset);
  Operand (2, Opd_integer, base);

  /* ====================================== */ 
  Instruction_Group("O_48", 
		 TOP_ldw_r_ii_r,
		 TOP_ldw_d_r_ii_r,
		 TOP_UNDEFINED);

  Result (0, Opd_integer);
  Operand (0, Opd_xsrc2, offset);
  Operand (1, Opd_integer, base);

  /* ====================================== */ 
  Instruction_Group("O_49", 
		 TOP_addpc_ii_r,
		 TOP_mov_ii_r,
		 TOP_UNDEFINED);

  Result (0, Opd_integer);
  Operand (0, Opd_xsrc2, opnd1);

  /* ====================================== */ 
  Instruction_Group("O_50", 
		 TOP_sub_r_ii_r,
		 TOP_UNDEFINED);

  Result (0, Opd_integer);
  Operand (0, Opd_xsrc2, opnd1);
  Operand (1, Opd_integer, opnd2);

  /* ====================================== */ 
  Instruction_Group("O_51", 
		 TOP_addcg_b_r_r_b_r,
		 TOP_divs_b_r_r_b_r,
		 TOP_UNDEFINED);

  Result (0, Opd_integer);
  Result (1, Opd_branch);
  Operand (0, Opd_integer);
  Operand (1, Opd_integer);
  Operand (2, Opd_branch);

  /* ====================================== */ 
  Instruction_Group("O_52", 
		 TOP_extractp,
		 TOP_UNDEFINED);

  Result (0, Opd_integer);
  Result (1, Opd_integer);
  Operand (0, Opd_paired);

  /* ====================================== */ 
  Instruction_Group("O_53", 
		 TOP_pushregs,
		 TOP_UNDEFINED);

  Result (0, Opd_integer);
  Result (1, Opd_link);
  Operand (0, Opd_integer);
  Operand (1, Opd_btarg);

  /* ====================================== */ 
  Instruction_Group("O_54", 
		 TOP_getpc,
		 TOP_UNDEFINED);

  Result (0, Opd_link);
  Operand (0, Opd_btarg);

  /* ====================================== */ 
  Instruction_Group("O_55", 
		 TOP_call_i,
		 TOP_UNDEFINED);

  Result (0, Opd_link);
  Operand (0, Opd_btarg, target);

  /* ====================================== */ 
  Instruction_Group("O_56", 
		 TOP_icall,
		 TOP_UNDEFINED);

  Result (0, Opd_link);
  Operand (0, Opd_link);

  /* ====================================== */ 
  Instruction_Group("O_57", 
		 TOP_avg4u_pb_r_r_b_r,
		 TOP_UNDEFINED);

  Result (0, Opd_nolink);
  Operand (0, Opd_branch);
  Operand (1, Opd_integer);
  Operand (2, Opd_integer);

  /* ====================================== */ 
  Instruction_Group("O_58", 
		 TOP_convfi_n_r_r,
		 TOP_convif_n_r_r,
		 TOP_UNDEFINED);

  Result (0, Opd_nolink);
  Operand (0, Opd_integer);

  /* ====================================== */ 
  Instruction_Group("O_59", 
		 TOP_pswmask_i_r_r,
		 TOP_UNDEFINED);

  Result (0, Opd_nolink);
  Operand (0, Opd_integer);
  Operand (1, Opd_isrc2);

  /* ====================================== */ 
  Instruction_Group("O_60", 
		 TOP_pswmask_ii_r_r,
		 TOP_UNDEFINED);

  Result (0, Opd_nolink);
  Operand (0, Opd_integer);
  Operand (1, Opd_xsrc2);

  /* ====================================== */ 
  Instruction_Group("O_61", 
		 TOP_addf_n_r_r_r,
		 TOP_divu_r_r_r,
		 TOP_div_r_r_r,
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
		 TOP_mulhhs_r_r_r,
		 TOP_mulhhu_r_r_r,
		 TOP_mulhh_r_r_r,
		 TOP_mulhs_r_r_r,
		 TOP_mulhu_r_r_r,
		 TOP_mulh_r_r_r,
		 TOP_mullhus_r_r_r,
		 TOP_mullhu_r_r_r,
		 TOP_mullh_r_r_r,
		 TOP_mulllu_r_r_r,
		 TOP_mulll_r_r_r,
		 TOP_mullu_r_r_r,
		 TOP_mull_r_r_r,
		 TOP_mul_ph_r_r_r,
		 TOP_remu_r_r_r,
		 TOP_rem_r_r_r,
		 TOP_sadu_pb_r_r_r,
		 TOP_shlso_r_r_r,
		 TOP_shls_r_r_r,
		 TOP_shls_ph_r_r_r,
		 TOP_shrrne_ph_r_r_r,
		 TOP_shrrnp_ph_r_r_r,
		 TOP_subf_n_r_r_r,
		 TOP_UNDEFINED);

  Result (0, Opd_nolink);
  Operand (0, Opd_integer, opnd1);
  Operand (1, Opd_integer, opnd2);

  /* ====================================== */ 
  Instruction_Group("O_62", 
		 TOP_mul32_i_r_r,
		 TOP_mul64hu_i_r_r,
		 TOP_mul64h_i_r_r,
		 TOP_mulfrac_i_r_r,
		 TOP_mulhhs_i_r_r,
		 TOP_mulhhu_i_r_r,
		 TOP_mulhh_i_r_r,
		 TOP_mulhs_i_r_r,
		 TOP_mulhu_i_r_r,
		 TOP_mulh_i_r_r,
		 TOP_mullhus_i_r_r,
		 TOP_mullhu_i_r_r,
		 TOP_mullh_i_r_r,
		 TOP_mulllu_i_r_r,
		 TOP_mulll_i_r_r,
		 TOP_mullu_i_r_r,
		 TOP_mull_i_r_r,
		 TOP_shlso_i_r_r,
		 TOP_shls_i_r_r,
		 TOP_shls_ph_i_r_r,
		 TOP_shrrne_ph_i_r_r,
		 TOP_shrrnp_i_r_r,
		 TOP_shrrnp_ph_i_r_r,
		 TOP_UNDEFINED);

  Result (0, Opd_nolink);
  Operand (0, Opd_integer, opnd1);
  Operand (1, Opd_isrc2, opnd2);

  /* ====================================== */ 
  Instruction_Group("O_63", 
		 TOP_mul32_ii_r_r,
		 TOP_mul64hu_ii_r_r,
		 TOP_mul64h_ii_r_r,
		 TOP_mulfrac_ii_r_r,
		 TOP_mulhhs_ii_r_r,
		 TOP_mulhhu_ii_r_r,
		 TOP_mulhh_ii_r_r,
		 TOP_mulhs_ii_r_r,
		 TOP_mulhu_ii_r_r,
		 TOP_mulh_ii_r_r,
		 TOP_mullhus_ii_r_r,
		 TOP_mullhu_ii_r_r,
		 TOP_mullh_ii_r_r,
		 TOP_mulllu_ii_r_r,
		 TOP_mulll_ii_r_r,
		 TOP_mullu_ii_r_r,
		 TOP_mull_ii_r_r,
		 TOP_st240_mull_ii_r_r,
		 TOP_shlso_ii_r_r,
		 TOP_shls_ii_r_r,
		 TOP_shls_ph_ii_r_r,
		 TOP_shrrne_ph_ii_r_r,
		 TOP_shrrnp_ii_r_r,
		 TOP_shrrnp_ph_ii_r_r,
		 TOP_UNDEFINED);

  Result (0, Opd_nolink);
  Operand (0, Opd_integer, opnd1);
  Operand (1, Opd_xsrc2, opnd2);

  /* ====================================== */ 
  Instruction_Group("O_64", 
		 TOP_ldbu_r_i_r,
		 TOP_ldbu_d_r_i_r,
		 TOP_ldb_r_i_r,
		 TOP_ldb_d_r_i_r,
		 TOP_ldhu_r_i_r,
		 TOP_ldhu_d_r_i_r,
		 TOP_ldh_r_i_r,
		 TOP_ldh_d_r_i_r,
		 TOP_UNDEFINED);

  Result (0, Opd_nolink);
  Operand (0, Opd_isrc2, offset);
  Operand (1, Opd_integer, base);

  /* ====================================== */ 
  Instruction_Group("O_65", 
		 TOP_ldbc_r_i_b_r,
		 TOP_ldbuc_r_i_b_r,
		 TOP_ldhc_r_i_b_r,
		 TOP_ldhuc_r_i_b_r,
		 TOP_UNDEFINED);

  Result (0, Opd_nolink);
  Operand (0, Opd_predicate, predicate);
  Operand (1, Opd_isrc2, offset);
  Operand (2, Opd_integer, base);

  /* ====================================== */ 
  Instruction_Group("O_66", 
		 TOP_ldbc_r_ii_b_r,
		 TOP_ldbuc_r_ii_b_r,
		 TOP_ldhc_r_ii_b_r,
		 TOP_ldhuc_r_ii_b_r,
		 TOP_UNDEFINED);

  Result (0, Opd_nolink);
  Operand (0, Opd_predicate, predicate);
  Operand (1, Opd_xsrc2, offset);
  Operand (2, Opd_integer, base);

  /* ====================================== */ 
  Instruction_Group("O_67", 
		 TOP_ldbu_r_ii_r,
		 TOP_ldbu_d_r_ii_r,
		 TOP_ldb_r_ii_r,
		 TOP_ldb_d_r_ii_r,
		 TOP_ldhu_r_ii_r,
		 TOP_ldhu_d_r_ii_r,
		 TOP_ldh_r_ii_r,
		 TOP_ldh_d_r_ii_r,
		 TOP_UNDEFINED);

  Result (0, Opd_nolink);
  Operand (0, Opd_xsrc2, offset);
  Operand (1, Opd_integer, base);

  /* ====================================== */ 
  Instruction_Group("O_68", 
		 TOP_ldl_r_i_p,
		 TOP_UNDEFINED);

  Result (0, Opd_nzpaired);
  Operand (0, Opd_isrc2, offset);
  Operand (1, Opd_integer, base);

  /* ====================================== */ 
  Instruction_Group("O_69", 
		 TOP_ldlc_r_i_b_p,
		 TOP_UNDEFINED);

  Result (0, Opd_nzpaired);
  Operand (0, Opd_predicate, predicate);
  Operand (1, Opd_isrc2, offset);
  Operand (2, Opd_integer, base);

  /* ====================================== */ 
  Instruction_Group("O_70", 
		 TOP_ldlc_r_ii_b_p,
		 TOP_UNDEFINED);

  Result (0, Opd_nzpaired);
  Operand (0, Opd_predicate, predicate);
  Operand (1, Opd_xsrc2, offset);
  Operand (2, Opd_integer, base);

  /* ====================================== */ 
  Instruction_Group("O_71", 
		 TOP_ldl_r_ii_p,
		 TOP_UNDEFINED);

  Result (0, Opd_nzpaired);
  Operand (0, Opd_xsrc2, offset);
  Operand (1, Opd_integer, base);

  /* ====================================== */ 
  Instruction_Group("O_72", 
		 TOP_multi_ldl_r_i_r,
		 TOP_UNDEFINED);

  Result (0, Opd_nzpairedfirst);
  Result (1, Opd_nzpairedsecond, multi);
  Operand (0, Opd_isrc2, offset);
  Operand (1, Opd_integer, base);

  /* ====================================== */ 
  Instruction_Group("O_73", 
		 TOP_multi_ldlc_r_i_b_r,
		 TOP_UNDEFINED);

  Result (0, Opd_nzpairedfirst);
  Result (1, Opd_nzpairedsecond, multi);
  Operand (0, Opd_predicate, predicate);
  Operand (1, Opd_isrc2, offset);
  Operand (2, Opd_integer, base);

  /* ====================================== */ 
  Instruction_Group("O_74", 
		 TOP_multi_ldlc_r_ii_b_r,
		 TOP_UNDEFINED);

  Result (0, Opd_nzpairedfirst);
  Result (1, Opd_nzpairedsecond, multi);
  Operand (0, Opd_predicate, predicate);
  Operand (1, Opd_xsrc2, offset);
  Operand (2, Opd_integer, base);

  /* ====================================== */ 
  Instruction_Group("O_75", 
		 TOP_multi_ldl_r_ii_r,
		 TOP_UNDEFINED);

  Result (0, Opd_nzpairedfirst);
  Result (1, Opd_nzpairedsecond, multi);
  Operand (0, Opd_xsrc2, offset);
  Operand (1, Opd_integer, base);

  /* ====================================== */ 
  Instruction_Group("O_76", 
		 TOP_composep,
		 TOP_UNDEFINED);

  Result (0, Opd_paired);
  Operand (0, Opd_integer);
  Operand (1, Opd_integer);

  /* ====================================== */ 
  Instruction_Group("O_77", 
		 TOP_movp,
		 TOP_UNDEFINED);

  Result (0, Opd_paired);
  Operand (0, Opd_paired, opnd1);

  ISA_Operands_End();
  return 0; 
} 
