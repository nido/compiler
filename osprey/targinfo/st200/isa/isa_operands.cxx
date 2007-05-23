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
                               1, UNSIGNED, INVALID); 
  OPERAND_VALUE_TYPE Opd_brknum = 
      ISA_Lit_Opnd_Type_Create("Opd_brknum", 
                               12, 
                               UNSIGNED, 
                               LC_brknum); 
  OPERAND_VALUE_TYPE Opd_btarg = 
      ISA_Lit_Opnd_Type_Create("Opd_btarg", 
                               23, 
                               PCREL, 
                               LC_btarg); 
  OPERAND_VALUE_TYPE Opd_imm = 
      ISA_Lit_Opnd_Type_Create("Opd_imm", 
                               23, 
                               SIGNED, 
                               LC_imm); 
  OPERAND_VALUE_TYPE Opd_integer = 
      ISA_Reg_Opnd_Type_Create("Opd_integer", 
                               ISA_REGISTER_CLASS_integer, 
                               ISA_REGISTER_SUBCLASS_UNDEFINED, 
                               32, SIGNED, INVALID); 
  OPERAND_VALUE_TYPE Opd_isrc2 = 
      ISA_Lit_Opnd_Type_Create("Opd_isrc2", 
                               9, 
                               SIGNED, 
                               LC_isrc2); 
  OPERAND_VALUE_TYPE Opd_issrc2 = 
      ISA_Lit_Opnd_Type_Create("Opd_issrc2", 
                               9, 
                               SIGNED, 
                               LC_issrc2); 
  OPERAND_VALUE_TYPE Opd_link = 
      ISA_Reg_Opnd_Type_Create("Opd_link", 
                               ISA_REGISTER_CLASS_integer, 
                               ISA_REGISTER_SUBCLASS_link, 
                               32, SIGNED, INVALID); 
  OPERAND_VALUE_TYPE Opd_nolink = 
      ISA_Reg_Opnd_Type_Create("Opd_nolink", 
                               ISA_REGISTER_CLASS_integer, 
                               ISA_REGISTER_SUBCLASS_nolink, 
                               32, SIGNED, INVALID); 
  OPERAND_VALUE_TYPE Opd_nzpaired = 
      ISA_Reg_Opnd_Type_Create("Opd_nzpaired", 
                               ISA_REGISTER_CLASS_integer, 
                               ISA_REGISTER_SUBCLASS_nzpaired, 
                               64, SIGNED, INVALID); 
  OPERAND_VALUE_TYPE Opd_nzpairedfirst = 
      ISA_Reg_Opnd_Type_Create("Opd_nzpairedfirst", 
                               ISA_REGISTER_CLASS_integer, 
                               ISA_REGISTER_SUBCLASS_nzpairedfirst, 
                               32, SIGNED, INVALID); 
  OPERAND_VALUE_TYPE Opd_nzpairedsecond = 
      ISA_Reg_Opnd_Type_Create("Opd_nzpairedsecond", 
                               ISA_REGISTER_CLASS_integer, 
                               ISA_REGISTER_SUBCLASS_nzpairedsecond, 
                               32, SIGNED, INVALID); 
  OPERAND_VALUE_TYPE Opd_paired = 
      ISA_Reg_Opnd_Type_Create("Opd_paired", 
                               ISA_REGISTER_CLASS_integer, 
                               ISA_REGISTER_SUBCLASS_paired, 
                               64, SIGNED, INVALID); 
  OPERAND_VALUE_TYPE Opd_pairedfirst = 
      ISA_Reg_Opnd_Type_Create("Opd_pairedfirst", 
                               ISA_REGISTER_CLASS_integer, 
                               ISA_REGISTER_SUBCLASS_pairedfirst, 
                               32, SIGNED, INVALID); 
  OPERAND_VALUE_TYPE Opd_pairedsecond = 
      ISA_Reg_Opnd_Type_Create("Opd_pairedsecond", 
                               ISA_REGISTER_CLASS_integer, 
                               ISA_REGISTER_SUBCLASS_pairedsecond, 
                               32, SIGNED, INVALID); 
  OPERAND_VALUE_TYPE Opd_predicate = 
      ISA_Reg_Opnd_Type_Create("Opd_predicate", 
                               ISA_REGISTER_CLASS_branch, 
                               ISA_REGISTER_SUBCLASS_predicate, 
                               1, UNSIGNED, INVALID); 
  OPERAND_VALUE_TYPE Opd_sbrknum = 
      ISA_Lit_Opnd_Type_Create("Opd_sbrknum", 
                               21, 
                               UNSIGNED, 
                               LC_sbrknum); 
  OPERAND_VALUE_TYPE Opd_xsrc2 = 
      ISA_Lit_Opnd_Type_Create("Opd_xsrc2", 
                               32, 
                               SIGNED, 
                               LC_xsrc2); 
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
		 TOP_break,
		 TOP_bwd_bar,
		 TOP_CALL,
		 TOP_COPY,
		 TOP_fwd_bar,
		 TOP_asm,
		 TOP_GOTO,
		 TOP_idle,
		 TOP_intrncall,
		 TOP_JUMP,
		 TOP_KILL,
		 TOP_LINK,
		 TOP_noop,
		 TOP_nop,
		 TOP_phi,
		 TOP_prgins,
		 TOP_psi,
		 TOP_RETURN,
		 TOP_rfi,
		 TOP_SIGMA,
		 TOP_sync,
		 TOP_syncins,
		 TOP_wmb,
		 TOP_UNDEFINED);


  /* ====================================== */ 
  Instruction_Group("O_1", 
		 TOP_brf,
		 TOP_br,
		 TOP_UNDEFINED);

  Operand (0, Opd_branch, condition);
  Operand (1, Opd_btarg, target);

  /* ====================================== */ 
  Instruction_Group("O_2", 
		 TOP_st235_sbrk,
		 TOP_st235_syscall,
		 TOP_UNDEFINED);

  Operand (0, Opd_brknum);

  /* ====================================== */ 
  Instruction_Group("O_3", 
		 TOP_FALL,
		 TOP_goto,
		 TOP_label,
		 TOP_LOOP,
		 TOP_UNDEFINED);

  Operand (0, Opd_btarg, target);

  /* ====================================== */ 
  Instruction_Group("O_4", 
		 TOP_pswclr,
		 TOP_pswset,
		 TOP_UNDEFINED);

  Operand (0, Opd_integer);

  /* ====================================== */ 
  Instruction_Group("O_5", 
		 TOP_begin_pregtn,
		 TOP_end_pregtn,
		 TOP_UNDEFINED);

  Operand (0, Opd_integer);
  Operand (1, Opd_isrc2);

  /* ====================================== */ 
  Instruction_Group("O_6", 
		 TOP_pft_i,
		 TOP_prgadd_i,
		 TOP_prginsadd_i,
		 TOP_prginspg_i,
		 TOP_prgset_i,
		 TOP_UNDEFINED);

  Operand (0, Opd_isrc2, offset);
  Operand (1, Opd_integer, base);

  /* ====================================== */ 
  Instruction_Group("O_7", 
		 TOP_stb_i,
		 TOP_sth_i,
		 TOP_stw_i,
		 TOP_UNDEFINED);

  Operand (0, Opd_isrc2, offset);
  Operand (1, Opd_integer, base);
  Operand (2, Opd_integer, storeval);

  /* ====================================== */ 
  Instruction_Group("O_8", 
		 TOP_stl_i,
		 TOP_UNDEFINED);

  Operand (0, Opd_isrc2, offset);
  Operand (1, Opd_integer, base);
  Operand (2, Opd_paired, storeval);

  /* ====================================== */ 
  Instruction_Group("O_9", 
		 TOP_multi_stl_i,
		 TOP_UNDEFINED);

  Operand (0, Opd_isrc2, offset);
  Operand (1, Opd_integer, base);
  Operand (2, Opd_pairedfirst, storeval);
  Operand (3, Opd_pairedsecond, multi);

  /* ====================================== */ 
  Instruction_Group("O_10", 
		 TOP_stbc_i,
		 TOP_sthc_i,
		 TOP_stwc_i,
		 TOP_UNDEFINED);

  Operand (0, Opd_isrc2, offset);
  Operand (1, Opd_integer, base);
  Operand (2, Opd_predicate, predicate);
  Operand (3, Opd_integer, storeval);

  /* ====================================== */ 
  Instruction_Group("O_11", 
		 TOP_stlc_i,
		 TOP_UNDEFINED);

  Operand (0, Opd_isrc2, offset);
  Operand (1, Opd_integer, base);
  Operand (2, Opd_predicate, predicate);
  Operand (3, Opd_paired, storeval);

  /* ====================================== */ 
  Instruction_Group("O_12", 
		 TOP_multi_stlc_i,
		 TOP_UNDEFINED);

  Operand (0, Opd_isrc2, offset);
  Operand (1, Opd_integer, base);
  Operand (2, Opd_predicate, predicate);
  Operand (3, Opd_pairedfirst, storeval);
  Operand (4, Opd_pairedsecond, multi);

  /* ====================================== */ 
  Instruction_Group("O_13", 
		 TOP_igoto,
		 TOP_return,
		 TOP_UNDEFINED);

  Operand (0, Opd_link);

  /* ====================================== */ 
  Instruction_Group("O_14", 
		 TOP_pftc_i,
		 TOP_UNDEFINED);

  Operand (0, Opd_predicate, predicate);
  Operand (1, Opd_isrc2, offset);
  Operand (2, Opd_integer, base);

  /* ====================================== */ 
  Instruction_Group("O_15", 
		 TOP_pftc_ii,
		 TOP_UNDEFINED);

  Operand (0, Opd_predicate, predicate);
  Operand (1, Opd_xsrc2, offset);
  Operand (2, Opd_integer, base);

  /* ====================================== */ 
  Instruction_Group("O_16", 
		 TOP_sbrk,
		 TOP_syscall,
		 TOP_UNDEFINED);

  Operand (0, Opd_sbrknum);

  /* ====================================== */ 
  Instruction_Group("O_17", 
		 TOP_pft_ii,
		 TOP_prgadd_ii,
		 TOP_prginsadd_ii,
		 TOP_prginspg_ii,
		 TOP_prgset_ii,
		 TOP_UNDEFINED);

  Operand (0, Opd_xsrc2, offset);
  Operand (1, Opd_integer, base);

  /* ====================================== */ 
  Instruction_Group("O_18", 
		 TOP_stb_ii,
		 TOP_sth_ii,
		 TOP_stw_ii,
		 TOP_UNDEFINED);

  Operand (0, Opd_xsrc2, offset);
  Operand (1, Opd_integer, base);
  Operand (2, Opd_integer, storeval);

  /* ====================================== */ 
  Instruction_Group("O_19", 
		 TOP_stl_ii,
		 TOP_UNDEFINED);

  Operand (0, Opd_xsrc2, offset);
  Operand (1, Opd_integer, base);
  Operand (2, Opd_paired, storeval);

  /* ====================================== */ 
  Instruction_Group("O_20", 
		 TOP_multi_stl_ii,
		 TOP_UNDEFINED);

  Operand (0, Opd_xsrc2, offset);
  Operand (1, Opd_integer, base);
  Operand (2, Opd_pairedfirst, storeval);
  Operand (3, Opd_pairedsecond, multi);

  /* ====================================== */ 
  Instruction_Group("O_21", 
		 TOP_stbc_ii,
		 TOP_sthc_ii,
		 TOP_stwc_ii,
		 TOP_UNDEFINED);

  Operand (0, Opd_xsrc2, offset);
  Operand (1, Opd_integer, base);
  Operand (2, Opd_predicate, predicate);
  Operand (3, Opd_integer, storeval);

  /* ====================================== */ 
  Instruction_Group("O_22", 
		 TOP_stlc_ii,
		 TOP_UNDEFINED);

  Operand (0, Opd_xsrc2, offset);
  Operand (1, Opd_integer, base);
  Operand (2, Opd_predicate, predicate);
  Operand (3, Opd_paired, storeval);

  /* ====================================== */ 
  Instruction_Group("O_23", 
		 TOP_multi_stlc_ii,
		 TOP_UNDEFINED);

  Operand (0, Opd_xsrc2, offset);
  Operand (1, Opd_integer, base);
  Operand (2, Opd_predicate, predicate);
  Operand (3, Opd_pairedfirst, storeval);
  Operand (4, Opd_pairedsecond, multi);

  /* ====================================== */ 
  Instruction_Group("O_24", 
		 TOP_mtb,
		 TOP_UNDEFINED);

  Result (0, Opd_branch);
  Operand (0, Opd_integer);

  /* ====================================== */ 
  Instruction_Group("O_25", 
		 TOP_stwl,
		 TOP_UNDEFINED);

  Result (0, Opd_branch);
  Operand (0, Opd_integer);
  Operand (1, Opd_integer, storeval);

  /* ====================================== */ 
  Instruction_Group("O_26", 
		 TOP_andl_r_b,
		 TOP_cmpeqf_n_b,
		 TOP_cmpeq_r_b,
		 TOP_cmpgef_n_b,
		 TOP_cmpgeu_r_b,
		 TOP_cmpge_r_b,
		 TOP_cmpgtf_n_b,
		 TOP_cmpgtu_r_b,
		 TOP_cmpgt_r_b,
		 TOP_cmplef_n_b,
		 TOP_cmpleu_r_b,
		 TOP_cmple_r_b,
		 TOP_cmpltf_n_b,
		 TOP_cmpltu_r_b,
		 TOP_cmplt_r_b,
		 TOP_cmpne_r_b,
		 TOP_nandl_r_b,
		 TOP_norl_r_b,
		 TOP_orl_r_b,
		 TOP_UNDEFINED);

  Result (0, Opd_branch);
  Operand (0, Opd_integer, opnd1);
  Operand (1, Opd_integer, opnd2);

  /* ====================================== */ 
  Instruction_Group("O_27", 
		 TOP_andl_i_b,
		 TOP_cmpeq_i_b,
		 TOP_cmpgeu_i_b,
		 TOP_cmpge_i_b,
		 TOP_cmpgtu_i_b,
		 TOP_cmpgt_i_b,
		 TOP_cmpleu_i_b,
		 TOP_cmple_i_b,
		 TOP_cmpltu_i_b,
		 TOP_cmplt_i_b,
		 TOP_cmpne_i_b,
		 TOP_nandl_i_b,
		 TOP_norl_i_b,
		 TOP_orl_i_b,
		 TOP_UNDEFINED);

  Result (0, Opd_branch);
  Operand (0, Opd_integer, opnd1);
  Operand (1, Opd_isrc2, opnd2);

  /* ====================================== */ 
  Instruction_Group("O_28", 
		 TOP_andl_ii_b,
		 TOP_cmpeq_ii_b,
		 TOP_cmpgeu_ii_b,
		 TOP_cmpge_ii_b,
		 TOP_cmpgtu_ii_b,
		 TOP_cmpgt_ii_b,
		 TOP_cmpleu_ii_b,
		 TOP_cmple_ii_b,
		 TOP_cmpltu_ii_b,
		 TOP_cmplt_ii_b,
		 TOP_cmpne_ii_b,
		 TOP_nandl_ii_b,
		 TOP_norl_ii_b,
		 TOP_orl_ii_b,
		 TOP_UNDEFINED);

  Result (0, Opd_branch);
  Operand (0, Opd_integer, opnd1);
  Operand (1, Opd_xsrc2, opnd2);

  /* ====================================== */ 
  Instruction_Group("O_29", 
		 TOP_ifixup,
		 TOP_UNDEFINED);

  Result (0, Opd_integer);

  /* ====================================== */ 
  Instruction_Group("O_30", 
		 TOP_mfb,
		 TOP_UNDEFINED);

  Result (0, Opd_integer);
  Operand (0, Opd_branch);

  /* ====================================== */ 
  Instruction_Group("O_31", 
		 TOP_slctf_r,
		 TOP_slct_r,
		 TOP_UNDEFINED);

  Result (0, Opd_integer);
  Operand (0, Opd_branch, condition);
  Operand (1, Opd_integer, opnd1);
  Operand (2, Opd_integer, opnd2);

  /* ====================================== */ 
  Instruction_Group("O_32", 
		 TOP_slctf_i,
		 TOP_slct_i,
		 TOP_UNDEFINED);

  Result (0, Opd_integer);
  Operand (0, Opd_branch, condition);
  Operand (1, Opd_integer, opnd1);
  Operand (2, Opd_isrc2, opnd2);

  /* ====================================== */ 
  Instruction_Group("O_33", 
		 TOP_slctf_ii,
		 TOP_slct_ii,
		 TOP_UNDEFINED);

  Result (0, Opd_integer);
  Operand (0, Opd_branch, condition);
  Operand (1, Opd_integer, opnd1);
  Operand (2, Opd_xsrc2, opnd2);

  /* ====================================== */ 
  Instruction_Group("O_34", 
		 TOP_ldwl,
		 TOP_UNDEFINED);

  Result (0, Opd_integer);
  Operand (0, Opd_integer);

  /* ====================================== */ 
  Instruction_Group("O_35", 
		 TOP_asm_0,
		 TOP_asm_10,
		 TOP_asm_11,
		 TOP_asm_12,
		 TOP_asm_13,
		 TOP_asm_14,
		 TOP_asm_15,
		 TOP_asm_1,
		 TOP_asm_2,
		 TOP_asm_3,
		 TOP_asm_4,
		 TOP_asm_5,
		 TOP_asm_6,
		 TOP_asm_7,
		 TOP_asm_8,
		 TOP_asm_9,
		 TOP_UNDEFINED);

  Result (0, Opd_integer);
  Operand (0, Opd_integer);
  Operand (1, Opd_integer);

  /* ====================================== */ 
  Instruction_Group("O_36", 
		 TOP_spadjust,
		 TOP_asm_16_i,
		 TOP_asm_17_i,
		 TOP_asm_18_i,
		 TOP_asm_19_i,
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
		 TOP_asm_30_i,
		 TOP_asm_31_i,
		 TOP_UNDEFINED);

  Result (0, Opd_integer);
  Operand (0, Opd_integer);
  Operand (1, Opd_isrc2);

  /* ====================================== */ 
  Instruction_Group("O_37", 
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
		 TOP_UNDEFINED);

  Result (0, Opd_integer);
  Operand (0, Opd_integer);
  Operand (1, Opd_xsrc2);

  /* ====================================== */ 
  Instruction_Group("O_38", 
		 TOP_bswap,
		 TOP_clz,
		 TOP_mov_r,
		 TOP_sxtb,
		 TOP_sxth,
		 TOP_zxtb,
		 TOP_zxth,
		 TOP_UNDEFINED);

  Result (0, Opd_integer);
  Operand (0, Opd_integer, opnd1);

  /* ====================================== */ 
  Instruction_Group("O_39", 
		 TOP_add_r,
		 TOP_andc_r,
		 TOP_andl_r_r,
		 TOP_and_r,
		 TOP_cmpeqf_n_r,
		 TOP_cmpeq_r_r,
		 TOP_cmpgef_n_r,
		 TOP_cmpgeu_r_r,
		 TOP_cmpge_r_r,
		 TOP_cmpgtf_n_r,
		 TOP_cmpgtu_r_r,
		 TOP_cmpgt_r_r,
		 TOP_cmplef_n_r,
		 TOP_cmpleu_r_r,
		 TOP_cmple_r_r,
		 TOP_cmpltf_n_r,
		 TOP_cmpltu_r_r,
		 TOP_cmplt_r_r,
		 TOP_cmpne_r_r,
		 TOP_maxu_r,
		 TOP_max_r,
		 TOP_minu_r,
		 TOP_min_r,
		 TOP_nandl_r_r,
		 TOP_norl_r_r,
		 TOP_orc_r,
		 TOP_orl_r_r,
		 TOP_or_r,
		 TOP_sh1add_r,
		 TOP_sh2add_r,
		 TOP_sh3add_r,
		 TOP_sh4add_r,
		 TOP_shl_r,
		 TOP_shru_r,
		 TOP_shr_r,
		 TOP_sub_r,
		 TOP_xor_r,
		 TOP_UNDEFINED);

  Result (0, Opd_integer);
  Operand (0, Opd_integer, opnd1);
  Operand (1, Opd_integer, opnd2);

  /* ====================================== */ 
  Instruction_Group("O_40", 
		 TOP_add_i,
		 TOP_andc_i,
		 TOP_andl_i_r,
		 TOP_and_i,
		 TOP_cmpeq_i_r,
		 TOP_cmpgeu_i_r,
		 TOP_cmpge_i_r,
		 TOP_cmpgtu_i_r,
		 TOP_cmpgt_i_r,
		 TOP_cmpleu_i_r,
		 TOP_cmple_i_r,
		 TOP_cmpltu_i_r,
		 TOP_cmplt_i_r,
		 TOP_cmpne_i_r,
		 TOP_maxu_i,
		 TOP_max_i,
		 TOP_minu_i,
		 TOP_min_i,
		 TOP_nandl_i_r,
		 TOP_norl_i_r,
		 TOP_orc_i,
		 TOP_orl_i_r,
		 TOP_or_i,
		 TOP_sh1add_i,
		 TOP_sh2add_i,
		 TOP_sh3add_i,
		 TOP_sh4add_i,
		 TOP_shl_i,
		 TOP_shru_i,
		 TOP_shr_i,
		 TOP_xor_i,
		 TOP_UNDEFINED);

  Result (0, Opd_integer);
  Operand (0, Opd_integer, opnd1);
  Operand (1, Opd_isrc2, opnd2);

  /* ====================================== */ 
  Instruction_Group("O_41", 
		 TOP_add_ii,
		 TOP_andc_ii,
		 TOP_andl_ii_r,
		 TOP_and_ii,
		 TOP_cmpeq_ii_r,
		 TOP_cmpgeu_ii_r,
		 TOP_cmpge_ii_r,
		 TOP_cmpgtu_ii_r,
		 TOP_cmpgt_ii_r,
		 TOP_cmpleu_ii_r,
		 TOP_cmple_ii_r,
		 TOP_cmpltu_ii_r,
		 TOP_cmplt_ii_r,
		 TOP_cmpne_ii_r,
		 TOP_maxu_ii,
		 TOP_max_ii,
		 TOP_minu_ii,
		 TOP_min_ii,
		 TOP_nandl_ii_r,
		 TOP_norl_ii_r,
		 TOP_orc_ii,
		 TOP_orl_ii_r,
		 TOP_or_ii,
		 TOP_sh1add_ii,
		 TOP_sh2add_ii,
		 TOP_sh3add_ii,
		 TOP_sh4add_ii,
		 TOP_shl_ii,
		 TOP_shru_ii,
		 TOP_shr_ii,
		 TOP_xor_ii,
		 TOP_UNDEFINED);

  Result (0, Opd_integer);
  Operand (0, Opd_integer, opnd1);
  Operand (1, Opd_xsrc2, opnd2);

  /* ====================================== */ 
  Instruction_Group("O_42", 
		 TOP_ldw_d_i,
		 TOP_ldw_i,
		 TOP_UNDEFINED);

  Result (0, Opd_integer);
  Operand (0, Opd_isrc2, offset);
  Operand (1, Opd_integer, base);

  /* ====================================== */ 
  Instruction_Group("O_43", 
		 TOP_addpc_i,
		 TOP_mov_i,
		 TOP_UNDEFINED);

  Result (0, Opd_integer);
  Operand (0, Opd_isrc2, opnd1);

  /* ====================================== */ 
  Instruction_Group("O_44", 
		 TOP_sub_i,
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
		 TOP_ldwc_i,
		 TOP_UNDEFINED);

  Result (0, Opd_integer);
  Operand (0, Opd_predicate, predicate);
  Operand (1, Opd_isrc2, offset);
  Operand (2, Opd_integer, base);

  /* ====================================== */ 
  Instruction_Group("O_47", 
		 TOP_ldwc_ii,
		 TOP_UNDEFINED);

  Result (0, Opd_integer);
  Operand (0, Opd_predicate, predicate);
  Operand (1, Opd_xsrc2, offset);
  Operand (2, Opd_integer, base);

  /* ====================================== */ 
  Instruction_Group("O_48", 
		 TOP_ldw_d_ii,
		 TOP_ldw_ii,
		 TOP_UNDEFINED);

  Result (0, Opd_integer);
  Operand (0, Opd_xsrc2, offset);
  Operand (1, Opd_integer, base);

  /* ====================================== */ 
  Instruction_Group("O_49", 
		 TOP_addpc_ii,
		 TOP_mov_ii,
		 TOP_UNDEFINED);

  Result (0, Opd_integer);
  Operand (0, Opd_xsrc2, opnd1);

  /* ====================================== */ 
  Instruction_Group("O_50", 
		 TOP_sub_ii,
		 TOP_UNDEFINED);

  Result (0, Opd_integer);
  Operand (0, Opd_xsrc2, opnd1);
  Operand (1, Opd_integer, opnd2);

  /* ====================================== */ 
  Instruction_Group("O_51", 
		 TOP_addcg,
		 TOP_divs,
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
		 TOP_call,
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
		 TOP_convfi_n,
		 TOP_convif_n,
		 TOP_UNDEFINED);

  Result (0, Opd_nolink);
  Operand (0, Opd_integer);

  /* ====================================== */ 
  Instruction_Group("O_58", 
		 TOP_addf_n,
		 TOP_divu,
		 TOP_div,
		 TOP_mul32_r,
		 TOP_mul64hu_r,
		 TOP_mul64h_r,
		 TOP_mulfrac_r,
		 TOP_mulf_n,
		 TOP_mulhhs_r,
		 TOP_mulhhu_r,
		 TOP_mulhh_r,
		 TOP_mulhs_r,
		 TOP_mulhu_r,
		 TOP_mulh_r,
		 TOP_mullhus_r,
		 TOP_mullhu_r,
		 TOP_mullh_r,
		 TOP_mulllu_r,
		 TOP_mulll_r,
		 TOP_mullu_r,
		 TOP_mull_r,
		 TOP_remu,
		 TOP_rem,
		 TOP_subf_n,
		 TOP_UNDEFINED);

  Result (0, Opd_nolink);
  Operand (0, Opd_integer, opnd1);
  Operand (1, Opd_integer, opnd2);

  /* ====================================== */ 
  Instruction_Group("O_59", 
		 TOP_mul32_i,
		 TOP_mul64hu_i,
		 TOP_mul64h_i,
		 TOP_mulfrac_i,
		 TOP_mulhhs_i,
		 TOP_mulhhu_i,
		 TOP_mulhh_i,
		 TOP_mulhs_i,
		 TOP_mulhu_i,
		 TOP_mulh_i,
		 TOP_mullhus_i,
		 TOP_mullhu_i,
		 TOP_mullh_i,
		 TOP_mulllu_i,
		 TOP_mulll_i,
		 TOP_mullu_i,
		 TOP_mull_i,
		 TOP_UNDEFINED);

  Result (0, Opd_nolink);
  Operand (0, Opd_integer, opnd1);
  Operand (1, Opd_isrc2, opnd2);

  /* ====================================== */ 
  Instruction_Group("O_60", 
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
		 TOP_UNDEFINED);

  Result (0, Opd_nolink);
  Operand (0, Opd_integer, opnd1);
  Operand (1, Opd_xsrc2, opnd2);

  /* ====================================== */ 
  Instruction_Group("O_61", 
		 TOP_ldbu_d_i,
		 TOP_ldbu_i,
		 TOP_ldb_d_i,
		 TOP_ldb_i,
		 TOP_ldhu_d_i,
		 TOP_ldhu_i,
		 TOP_ldh_d_i,
		 TOP_ldh_i,
		 TOP_UNDEFINED);

  Result (0, Opd_nolink);
  Operand (0, Opd_isrc2, offset);
  Operand (1, Opd_integer, base);

  /* ====================================== */ 
  Instruction_Group("O_62", 
		 TOP_ldbc_i,
		 TOP_ldbuc_i,
		 TOP_ldhc_i,
		 TOP_ldhuc_i,
		 TOP_UNDEFINED);

  Result (0, Opd_nolink);
  Operand (0, Opd_predicate, predicate);
  Operand (1, Opd_isrc2, offset);
  Operand (2, Opd_integer, base);

  /* ====================================== */ 
  Instruction_Group("O_63", 
		 TOP_ldbc_ii,
		 TOP_ldbuc_ii,
		 TOP_ldhc_ii,
		 TOP_ldhuc_ii,
		 TOP_UNDEFINED);

  Result (0, Opd_nolink);
  Operand (0, Opd_predicate, predicate);
  Operand (1, Opd_xsrc2, offset);
  Operand (2, Opd_integer, base);

  /* ====================================== */ 
  Instruction_Group("O_64", 
		 TOP_ldbu_d_ii,
		 TOP_ldbu_ii,
		 TOP_ldb_d_ii,
		 TOP_ldb_ii,
		 TOP_ldhu_d_ii,
		 TOP_ldhu_ii,
		 TOP_ldh_d_ii,
		 TOP_ldh_ii,
		 TOP_UNDEFINED);

  Result (0, Opd_nolink);
  Operand (0, Opd_xsrc2, offset);
  Operand (1, Opd_integer, base);

  /* ====================================== */ 
  Instruction_Group("O_65", 
		 TOP_ldl_i,
		 TOP_UNDEFINED);

  Result (0, Opd_nzpaired);
  Operand (0, Opd_isrc2, offset);
  Operand (1, Opd_integer, base);

  /* ====================================== */ 
  Instruction_Group("O_66", 
		 TOP_ldlc_i,
		 TOP_UNDEFINED);

  Result (0, Opd_nzpaired);
  Operand (0, Opd_predicate, predicate);
  Operand (1, Opd_isrc2, offset);
  Operand (2, Opd_integer, base);

  /* ====================================== */ 
  Instruction_Group("O_67", 
		 TOP_ldlc_ii,
		 TOP_UNDEFINED);

  Result (0, Opd_nzpaired);
  Operand (0, Opd_predicate, predicate);
  Operand (1, Opd_xsrc2, offset);
  Operand (2, Opd_integer, base);

  /* ====================================== */ 
  Instruction_Group("O_68", 
		 TOP_ldl_ii,
		 TOP_UNDEFINED);

  Result (0, Opd_nzpaired);
  Operand (0, Opd_xsrc2, offset);
  Operand (1, Opd_integer, base);

  /* ====================================== */ 
  Instruction_Group("O_69", 
		 TOP_multi_ldl_i,
		 TOP_UNDEFINED);

  Result (0, Opd_nzpairedfirst);
  Result (1, Opd_nzpairedsecond, multi);
  Operand (0, Opd_isrc2, offset);
  Operand (1, Opd_integer, base);

  /* ====================================== */ 
  Instruction_Group("O_70", 
		 TOP_multi_ldlc_i,
		 TOP_UNDEFINED);

  Result (0, Opd_nzpairedfirst);
  Result (1, Opd_nzpairedsecond, multi);
  Operand (0, Opd_predicate, predicate);
  Operand (1, Opd_isrc2, offset);
  Operand (2, Opd_integer, base);

  /* ====================================== */ 
  Instruction_Group("O_71", 
		 TOP_multi_ldlc_ii,
		 TOP_UNDEFINED);

  Result (0, Opd_nzpairedfirst);
  Result (1, Opd_nzpairedsecond, multi);
  Operand (0, Opd_predicate, predicate);
  Operand (1, Opd_xsrc2, offset);
  Operand (2, Opd_integer, base);

  /* ====================================== */ 
  Instruction_Group("O_72", 
		 TOP_multi_ldl_ii,
		 TOP_UNDEFINED);

  Result (0, Opd_nzpairedfirst);
  Result (1, Opd_nzpairedsecond, multi);
  Operand (0, Opd_xsrc2, offset);
  Operand (1, Opd_integer, base);

  /* ====================================== */ 
  Instruction_Group("O_73", 
		 TOP_composep,
		 TOP_UNDEFINED);

  Result (0, Opd_paired);
  Operand (0, Opd_integer);
  Operand (1, Opd_integer);

  /* ====================================== */ 
  Instruction_Group("O_74", 
		 TOP_movp,
		 TOP_UNDEFINED);

  Result (0, Opd_paired);
  Operand (0, Opd_paired, opnd1);

  ISA_Operands_End();
  return 0; 
} 
