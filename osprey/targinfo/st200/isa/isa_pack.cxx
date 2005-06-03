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
// Group TOPs with similar packing format together.  
///////////////////////////////////////////////////////// 
// Instructions may be packed (compressed) in binary files. The packing rules 
// are specified in this file. 

#include <stddef.h> 
#include "topcode.h" 
#include "isa_pack_gen.h" 

main() 
{ 
  ISA_Pack_Begin("st200", 32); 

  /* Create the various adjustments that need to be performed between 
   * assembly language form and packed form. Note that simple shift  
   * adjustments have been accomplished directly in the operand packing 
   * specification. 
   */ 

  OPND_ADJ_TYPE no_adj = Create_Operand_Adjustment("no adjustment", "O_VAL"); 

  /* ===== p1: ===== */
  ISA_PACK_TYPE p1 = ISA_Pack_Type_Create("p1");
  Instruction_Pack_Group(p1,
	TOP_asm_0, 0x2c000000UL, 
	TOP_asm_10, 0x2d400000UL, 
	TOP_asm_11, 0x2d600000UL, 
	TOP_asm_12, 0x2d800000UL, 
	TOP_asm_13, 0x2da00000UL, 
	TOP_asm_14, 0x2dc00000UL, 
	TOP_asm_15, 0x2de00000UL, 
	TOP_asm_16_i, 0x2e000000UL, 
	TOP_asm_16_ii, 0x2e000000UL, 
	TOP_asm_17_i, 0x2e200000UL, 
	TOP_asm_17_ii, 0x2e200000UL, 
	TOP_asm_18_i, 0x2e400000UL, 
	TOP_asm_18_ii, 0x2e400000UL, 
	TOP_asm_19_i, 0x2e600000UL, 
	TOP_asm_19_ii, 0x2e600000UL, 
	TOP_asm_1, 0x2c200000UL, 
	TOP_asm_20_i, 0x2e800000UL, 
	TOP_asm_20_ii, 0x2e800000UL, 
	TOP_asm_21_i, 0x2ea00000UL, 
	TOP_asm_21_ii, 0x2ea00000UL, 
	TOP_asm_22_i, 0x2ec00000UL, 
	TOP_asm_22_ii, 0x2ec00000UL, 
	TOP_asm_23_i, 0x2ee00000UL, 
	TOP_asm_23_ii, 0x2ee00000UL, 
	TOP_asm_24_i, 0x2f000000UL, 
	TOP_asm_24_ii, 0x2f000000UL, 
	TOP_asm_25_i, 0x2f200000UL, 
	TOP_asm_25_ii, 0x2f200000UL, 
	TOP_asm_26_i, 0x2f400000UL, 
	TOP_asm_26_ii, 0x2f400000UL, 
	TOP_asm_27_i, 0x2f600000UL, 
	TOP_asm_27_ii, 0x2f600000UL, 
	TOP_asm_28_i, 0x2f800000UL, 
	TOP_asm_28_ii, 0x2f800000UL, 
	TOP_asm_29_i, 0x2fa00000UL, 
	TOP_asm_29_ii, 0x2fa00000UL, 
	TOP_asm_2, 0x2c400000UL, 
	TOP_asm_30_i, 0x2fc00000UL, 
	TOP_asm_30_ii, 0x2fc00000UL, 
	TOP_asm_31_i, 0x2fe00000UL, 
	TOP_asm_31_ii, 0x2fe00000UL, 
	TOP_asm_3, 0x2c600000UL, 
	TOP_asm_4, 0x2c800000UL, 
	TOP_asm_5, 0x2ca00000UL, 
	TOP_asm_6, 0x2cc00000UL, 
	TOP_asm_7, 0x2ce00000UL, 
	TOP_asm_8, 0x2d000000UL, 
	TOP_asm_9, 0x2d200000UL, 
	TOP_break, 0x1fe00000UL, 
	TOP_icall, 0x30800000UL, 
	TOP_idle, 0x31000000UL, 
	TOP_igoto, 0x31800000UL, 
	TOP_nop, 0x00000000UL, 
	TOP_prgins, 0x1f800000UL, 
	TOP_prginsadd, 0x32800000UL, 
	TOP_return, 0x31800000UL, 
	TOP_rfi, 0x32000000UL, 
	TOP_sync, 0x28000000UL, 
	TOP_syncins, 0x31000001UL, 
	TOP_wmb, 0x2a000000UL, 
	TOP_UNDEFINED);

  /* ===== p2: ===== */
  ISA_PACK_TYPE p2 = ISA_Pack_Type_Create("p2");
  Operand(0, 0, 0, 21);
  Instruction_Pack_Group(p2,
	TOP_sbrk, 0x9fa00000UL, 
	TOP_syscall, 0x1fc00000UL, 
	TOP_UNDEFINED);

  /* ===== p3: ===== */
  ISA_PACK_TYPE p3 = ISA_Pack_Type_Create("p3");
  Operand(0, 0, 0, 23);
  Instruction_Pack_Group(p3,
	TOP_call, 0x30000000UL, 
	TOP_goto, 0x31000000UL, 
	TOP_UNDEFINED);

  /* ===== p4: ===== */
  ISA_PACK_TYPE p4 = ISA_Pack_Type_Create("p4");
  Operand(0, 0, 12, 9);
  Operand(1, 0, 0, 6);
  Instruction_Pack_Group(p4,
	TOP_pft_i, 0x26800000UL, 
	TOP_prgadd_i, 0x27000000UL, 
	TOP_prginspg_i, 0x28800000UL, 
	TOP_prgset_i, 0x27800000UL, 
	TOP_UNDEFINED);

  /* ===== p5: ===== */
  ISA_PACK_TYPE p5 = ISA_Pack_Type_Create("p5");
  Operand(0, 0, 12, 9);
  Operand(1, 0, 0, 6);
  Next_Word(); 
  Operand(0, 9, 0, 23);
  Instruction_Pack_Group(p5,
	TOP_pft_ii, 0x26800000UL, 0UL, 
	TOP_prgadd_ii, 0x27000000UL, 0UL, 
	TOP_prginspg_ii, 0x28800000UL, 0UL, 
	TOP_prgset_ii, 0x27800000UL, 0UL, 
	TOP_UNDEFINED);

  /* ===== p6: ===== */
  ISA_PACK_TYPE p6 = ISA_Pack_Type_Create("p6");
  Operand(0, 0, 12, 9);
  Operand(1, 0, 0, 6);
  Operand(2, 0, 21, 3);
  Operand(3, 0, 6, 6);
  Instruction_Pack_Group(p6,
	TOP_multi_stpc_i, 0x28000000UL, 
	TOP_stbc_i, 0x2c000000UL, 
	TOP_sthc_i, 0x2a000000UL, 
	TOP_stpc_i, 0x28000000UL, 
	TOP_stwc_i, 0x29000000UL, 
	TOP_UNDEFINED);

  /* ===== p7: ===== */
  ISA_PACK_TYPE p7 = ISA_Pack_Type_Create("p7");
  Operand(0, 0, 12, 9);
  Operand(1, 0, 0, 6);
  Operand(2, 0, 21, 3);
  Operand(3, 0, 6, 6);
  Next_Word(); 
  Operand(0, 9, 0, 23);
  Instruction_Pack_Group(p7,
	TOP_multi_stpc_ii, 0x28000000UL, 0UL, 
	TOP_stbc_ii, 0x2c000000UL, 0UL, 
	TOP_sthc_ii, 0x2a000000UL, 0UL, 
	TOP_stpc_ii, 0x28000000UL, 0UL, 
	TOP_stwc_ii, 0x29000000UL, 0UL, 
	TOP_UNDEFINED);

  /* ===== p8: ===== */
  ISA_PACK_TYPE p8 = ISA_Pack_Type_Create("p8");
  Operand(0, 0, 12, 9);
  Operand(1, 0, 0, 6);
  Operand(2, 0, 6, 6);
  Instruction_Pack_Group(p8,
	TOP_multi_stp_i, 0x28000000UL, 
	TOP_stb_i, 0x26000000UL, 
	TOP_sth_i, 0x25800000UL, 
	TOP_stp_i, 0x28000000UL, 
	TOP_stw_i, 0x25000000UL, 
	TOP_UNDEFINED);

  /* ===== p9: ===== */
  ISA_PACK_TYPE p9 = ISA_Pack_Type_Create("p9");
  Operand(0, 0, 12, 9);
  Operand(1, 0, 0, 6);
  Operand(2, 0, 6, 6);
  Next_Word(); 
  Operand(0, 9, 0, 23);
  Instruction_Pack_Group(p9,
	TOP_multi_stp_ii, 0x28000000UL, 0UL, 
	TOP_stb_ii, 0x26000000UL, 0UL, 
	TOP_sth_ii, 0x25800000UL, 0UL, 
	TOP_stp_ii, 0x28000000UL, 0UL, 
	TOP_stw_ii, 0x25000000UL, 0UL, 
	TOP_UNDEFINED);

  /* ===== p10: ===== */
  ISA_PACK_TYPE p10 = ISA_Pack_Type_Create("p10");
  Operand(0, 0, 21, 3);
  Operand(1, 0, 12, 9);
  Operand(2, 0, 0, 6);
  Instruction_Pack_Group(p10,
	TOP_pftc_i, 0x20000000UL, 
	TOP_UNDEFINED);

  /* ===== p11: ===== */
  ISA_PACK_TYPE p11 = ISA_Pack_Type_Create("p11");
  Operand(0, 0, 21, 3);
  Operand(1, 0, 12, 9);
  Operand(2, 0, 0, 6);
  Next_Word(); 
  Operand(1, 9, 0, 23);
  Instruction_Pack_Group(p11,
	TOP_pftc_ii, 0x20000000UL, 0UL, 
	TOP_UNDEFINED);

  /* ===== p12: ===== */
  ISA_PACK_TYPE p12 = ISA_Pack_Type_Create("p12");
  Operand(0, 0, 23, 3);
  Operand(1, 0, 0, 23);
  Instruction_Pack_Group(p12,
	TOP_brf, 0x3c000000UL, 
	TOP_br, 0x38000000UL, 
	TOP_UNDEFINED);

  /* ===== p13: ===== */
  ISA_PACK_TYPE p13 = ISA_Pack_Type_Create("p13");
  Operand(0, 0, 6, 6);
  Instruction_Pack_Group(p13,
	TOP_pswclr, 0x29800000UL, 
	TOP_pswset, 0x29000000UL, 
	TOP_UNDEFINED);

  /* ===== p14: ===== */
  ISA_PACK_TYPE p14 = ISA_Pack_Type_Create("p14");
  Result(0, 12, 3);
  Operand(0, 0, 0, 6);
  Operand(1, 0, 6, 6);
  Instruction_Pack_Group(p14,
	TOP_stwl, 0x2b000000UL, 
	TOP_UNDEFINED);

  /* ===== p15: ===== */
  ISA_PACK_TYPE p15 = ISA_Pack_Type_Create("p15");
  Result(0, 12, 6);
  Operand(0, 0, 0, 6);
  Instruction_Pack_Group(p15,
	TOP_convfi_n, 0x02880000UL, 
	TOP_convif_n, 0x02680000UL, 
	TOP_UNDEFINED);

  /* ===== p16: ===== */
  ISA_PACK_TYPE p16 = ISA_Pack_Type_Create("p16");
  Result(0, 12, 6);
  Operand(0, 0, 0, 6);
  Operand(1, 0, 6, 6);
  Instruction_Pack_Group(p16,
	TOP_addf_n, 0x02080000UL, 
	TOP_add_r, 0x00000000UL, 
	TOP_andc_r, 0x01400000UL, 
	TOP_andl_r_r, 0x05400000UL, 
	TOP_and_r, 0x01200000UL, 
	TOP_cmpeq_r_r, 0x04000000UL, 
	TOP_cmpgeu_r_r, 0x04600000UL, 
	TOP_cmpge_r_r, 0x04400000UL, 
	TOP_cmpgtu_r_r, 0x04a00000UL, 
	TOP_cmpgt_r_r, 0x04800000UL, 
	TOP_cmpleu_r_r, 0x04e00000UL, 
	TOP_cmple_r_r, 0x04c00000UL, 
	TOP_cmpltu_r_r, 0x05200000UL, 
	TOP_cmplt_r_r, 0x05000000UL, 
	TOP_cmpne_r_r, 0x04200000UL, 
	TOP_divu, 0x03480000UL, 
	TOP_div, 0x03080000UL, 
	TOP_maxu_r, 0x02200000UL, 
	TOP_max_r, 0x02000000UL, 
	TOP_minu_r, 0x02600000UL, 
	TOP_min_r, 0x02400000UL, 
	TOP_mul32_r, 0x05c00000UL, 
	TOP_mul64hu_r, 0x07c00000UL, 
	TOP_mul64h_r, 0x05e00000UL, 
	TOP_mulfrac_r, 0x07e00000UL, 
	TOP_mulf_n, 0x02480000UL, 
	TOP_mulhhs_r, 0x02800000UL, 
	TOP_mulhhu_r, 0x03c00000UL, 
	TOP_mulhh_r, 0x03a00000UL, 
	TOP_mulhs_r, 0x03e00000UL, 
	TOP_mulhu_r, 0x03000000UL, 
	TOP_mulh_r, 0x02e00000UL, 
	TOP_mullhus_r, 0x01e00000UL, 
	TOP_mullhu_r, 0x03800000UL, 
	TOP_mullh_r, 0x03600000UL, 
	TOP_mulllu_r, 0x03400000UL, 
	TOP_mulll_r, 0x03200000UL, 
	TOP_mullu_r, 0x02c00000UL, 
	TOP_mull_r, 0x02a00000UL, 
	TOP_nandl_r_r, 0x05600000UL, 
	TOP_norl_r_r, 0x05a00000UL, 
	TOP_orc_r, 0x01800000UL, 
	TOP_orl_r_r, 0x05800000UL, 
	TOP_or_r, 0x01600000UL, 
	TOP_remu, 0x03680000UL, 
	TOP_rem, 0x03280000UL, 
	TOP_sh1add_r, 0x00a00000UL, 
	TOP_sh2add_r, 0x00c00000UL, 
	TOP_sh3add_r, 0x00e00000UL, 
	TOP_sh4add_r, 0x01000000UL, 
	TOP_shl_r, 0x00400000UL, 
	TOP_shru_r, 0x00800000UL, 
	TOP_shr_r, 0x00600000UL, 
	TOP_subf_n, 0x02280000UL, 
	TOP_xor_r, 0x01a00000UL, 
	TOP_UNDEFINED);

  /* ===== p17: ===== */
  ISA_PACK_TYPE p17 = ISA_Pack_Type_Create("p17");
  Result(0, 12, 6);
  Operand(0, 0, 21, 3);
  Operand(1, 0, 0, 6);
  Operand(2, 0, 6, 6);
  Instruction_Pack_Group(p17,
	TOP_slctf_r, 0x11000000UL, 
	TOP_slct_r, 0x10000000UL, 
	TOP_UNDEFINED);

  /* ===== p18: ===== */
  ISA_PACK_TYPE p18 = ISA_Pack_Type_Create("p18");
  Result(0, 12, 6);
  Operand(0, 0, 6, 6);
  Instruction_Pack_Group(p18,
	TOP_addpc_r, 0x01000000UL, 
	TOP_mov_r, 0x00000000UL, 
	TOP_UNDEFINED);

  /* ===== p19: ===== */
  ISA_PACK_TYPE p19 = ISA_Pack_Type_Create("p19");
  Result(0, 12, 6);
  Operand(0, 0, 6, 6);
  Operand(1, 0, 0, 6);
  Instruction_Pack_Group(p19,
	TOP_sub_r, 0x00200000UL, 
	TOP_UNDEFINED);

  /* ===== p20: ===== */
  ISA_PACK_TYPE p20 = ISA_Pack_Type_Create("p20");
  Result(0, 12, 6);
  Result(1, 18, 3);
  Operand(0, 0, 0, 6);
  Operand(1, 0, 6, 6);
  Operand(2, 0, 21, 3);
  Instruction_Pack_Group(p20,
	TOP_addcg, 0x12000000UL, 
	TOP_divs, 0x14000000UL, 
	TOP_UNDEFINED);

  /* ===== p21: ===== */
  ISA_PACK_TYPE p21 = ISA_Pack_Type_Create("p21");
  Result(0, 18, 3);
  Operand(0, 0, 0, 6);
  Instruction_Pack_Group(p21,
	TOP_mtb, 0x07800000UL, 
	TOP_UNDEFINED);

  /* ===== p22: ===== */
  ISA_PACK_TYPE p22 = ISA_Pack_Type_Create("p22");
  Result(0, 18, 3);
  Operand(0, 0, 0, 6);
  Operand(1, 0, 6, 6);
  Instruction_Pack_Group(p22,
	TOP_andl_r_b, 0x07400000UL, 
	TOP_cmpeq_r_b, 0x06000000UL, 
	TOP_cmpgeu_r_b, 0x06600000UL, 
	TOP_cmpge_r_b, 0x06400000UL, 
	TOP_cmpgtu_r_b, 0x06a00000UL, 
	TOP_cmpgt_r_b, 0x06800000UL, 
	TOP_cmpleu_r_b, 0x06e00000UL, 
	TOP_cmple_r_b, 0x06c00000UL, 
	TOP_cmpltu_r_b, 0x07200000UL, 
	TOP_cmplt_r_b, 0x07000000UL, 
	TOP_cmpne_r_b, 0x06200000UL, 
	TOP_nandl_r_b, 0x07600000UL, 
	TOP_norl_r_b, 0x07a00000UL, 
	TOP_orl_r_b, 0x07800000UL, 
	TOP_UNDEFINED);

  /* ===== p23: ===== */
  ISA_PACK_TYPE p23 = ISA_Pack_Type_Create("p23");
  Result(0, 6, 3);
  Operand(0, 0, 0, 6);
  Operand(1, 0, 12, 9);
  Instruction_Pack_Group(p23,
	TOP_andl_i_b, 0x0f400000UL, 
	TOP_cmpeq_i_b, 0x0e000000UL, 
	TOP_cmpgeu_i_b, 0x0e600000UL, 
	TOP_cmpge_i_b, 0x0e400000UL, 
	TOP_cmpgtu_i_b, 0x0ea00000UL, 
	TOP_cmpgt_i_b, 0x0e800000UL, 
	TOP_cmpleu_i_b, 0x0ee00000UL, 
	TOP_cmple_i_b, 0x0ec00000UL, 
	TOP_cmpltu_i_b, 0x0f200000UL, 
	TOP_cmplt_i_b, 0x0f000000UL, 
	TOP_cmpne_i_b, 0x0e200000UL, 
	TOP_nandl_i_b, 0x0f600000UL, 
	TOP_norl_i_b, 0x0fa00000UL, 
	TOP_orl_i_b, 0x0f800000UL, 
	TOP_UNDEFINED);

  /* ===== p24: ===== */
  ISA_PACK_TYPE p24 = ISA_Pack_Type_Create("p24");
  Result(0, 6, 3);
  Operand(0, 0, 0, 6);
  Operand(1, 0, 12, 9);
  Next_Word(); 
  Operand(1, 9, 0, 23);
  Instruction_Pack_Group(p24,
	TOP_andl_ii_b, 0x0f400000UL, 0UL, 
	TOP_cmpeq_ii_b, 0x0e000000UL, 0UL, 
	TOP_cmpgeu_ii_b, 0x0e600000UL, 0UL, 
	TOP_cmpge_ii_b, 0x0e400000UL, 0UL, 
	TOP_cmpgtu_ii_b, 0x0ea00000UL, 0UL, 
	TOP_cmpgt_ii_b, 0x0e800000UL, 0UL, 
	TOP_cmpleu_ii_b, 0x0ee00000UL, 0UL, 
	TOP_cmple_ii_b, 0x0ec00000UL, 0UL, 
	TOP_cmpltu_ii_b, 0x0f200000UL, 0UL, 
	TOP_cmplt_ii_b, 0x0f000000UL, 0UL, 
	TOP_cmpne_ii_b, 0x0e200000UL, 0UL, 
	TOP_nandl_ii_b, 0x0f600000UL, 0UL, 
	TOP_norl_ii_b, 0x0fa00000UL, 0UL, 
	TOP_orl_ii_b, 0x0f800000UL, 0UL, 
	TOP_UNDEFINED);

  /* ===== p25: ===== */
  ISA_PACK_TYPE p25 = ISA_Pack_Type_Create("p25");
  Result(0, 6, 6);
  Operand(0, 0, 0, 6);
  Instruction_Pack_Group(p25,
	TOP_bswap, 0x09c02000UL, 
	TOP_clz, 0x09c04000UL, 
	TOP_ldwl, 0x2a800000UL, 
	TOP_sxtb, 0x09c00000UL, 
	TOP_sxth, 0x09c01000UL, 
	TOP_zxtb, 0x092ff000UL, 
	TOP_zxth, 0x09c03000UL, 
	TOP_UNDEFINED);

  /* ===== p26: ===== */
  ISA_PACK_TYPE p26 = ISA_Pack_Type_Create("p26");
  Result(0, 6, 6);
  Operand(0, 0, 0, 6);
  Operand(1, 0, 12, 9);
  Instruction_Pack_Group(p26,
	TOP_add_i, 0x08000000UL, 
	TOP_andc_i, 0x09400000UL, 
	TOP_andl_i_r, 0x0d400000UL, 
	TOP_and_i, 0x09200000UL, 
	TOP_cmpeq_i_r, 0x0c000000UL, 
	TOP_cmpgeu_i_r, 0x0c600000UL, 
	TOP_cmpge_i_r, 0x0c400000UL, 
	TOP_cmpgtu_i_r, 0x0ca00000UL, 
	TOP_cmpgt_i_r, 0x0c800000UL, 
	TOP_cmpleu_i_r, 0x0ce00000UL, 
	TOP_cmple_i_r, 0x0cc00000UL, 
	TOP_cmpltu_i_r, 0x0d200000UL, 
	TOP_cmplt_i_r, 0x0d000000UL, 
	TOP_cmpne_i_r, 0x0c200000UL, 
	TOP_maxu_i, 0x0a200000UL, 
	TOP_max_i, 0x0a000000UL, 
	TOP_minu_i, 0x0a600000UL, 
	TOP_min_i, 0x0a400000UL, 
	TOP_mul32_i, 0x0dc00000UL, 
	TOP_mul64hu_i, 0x0fc00000UL, 
	TOP_mul64h_i, 0x0de00000UL, 
	TOP_mulfrac_i, 0x0fe00000UL, 
	TOP_mulhhs_i, 0x0a800000UL, 
	TOP_mulhhu_i, 0x0bc00000UL, 
	TOP_mulhh_i, 0x0ba00000UL, 
	TOP_mulhs_i, 0x0be00000UL, 
	TOP_mulhu_i, 0x0b000000UL, 
	TOP_mulh_i, 0x0ae00000UL, 
	TOP_mullhus_i, 0x09e00000UL, 
	TOP_mullhu_i, 0x0b800000UL, 
	TOP_mullh_i, 0x0b600000UL, 
	TOP_mulllu_i, 0x0b400000UL, 
	TOP_mulll_i, 0x0b200000UL, 
	TOP_mullu_i, 0x0ac00000UL, 
	TOP_mull_i, 0x0aa00000UL, 
	TOP_nandl_i_r, 0x0d600000UL, 
	TOP_norl_i_r, 0x0da00000UL, 
	TOP_orc_i, 0x09800000UL, 
	TOP_orl_i_r, 0x0d800000UL, 
	TOP_or_i, 0x09600000UL, 
	TOP_returnadd, 0x33800000UL, 
	TOP_sh1add_i, 0x08a00000UL, 
	TOP_sh2add_i, 0x08c00000UL, 
	TOP_sh3add_i, 0x08e00000UL, 
	TOP_sh4add_i, 0x09000000UL, 
	TOP_shl_i, 0x08400000UL, 
	TOP_shru_i, 0x08800000UL, 
	TOP_shr_i, 0x08600000UL, 
	TOP_xor_i, 0x09a00000UL, 
	TOP_UNDEFINED);

  /* ===== p27: ===== */
  ISA_PACK_TYPE p27 = ISA_Pack_Type_Create("p27");
  Result(0, 6, 6);
  Operand(0, 0, 0, 6);
  Operand(1, 0, 12, 9);
  Next_Word(); 
  Operand(1, 9, 0, 23);
  Instruction_Pack_Group(p27,
	TOP_add_ii, 0x08000000UL, 0UL, 
	TOP_andc_ii, 0x09400000UL, 0UL, 
	TOP_andl_ii_r, 0x0d400000UL, 0UL, 
	TOP_and_ii, 0x09200000UL, 0UL, 
	TOP_cmpeq_ii_r, 0x0c000000UL, 0UL, 
	TOP_cmpgeu_ii_r, 0x0c600000UL, 0UL, 
	TOP_cmpge_ii_r, 0x0c400000UL, 0UL, 
	TOP_cmpgtu_ii_r, 0x0ca00000UL, 0UL, 
	TOP_cmpgt_ii_r, 0x0c800000UL, 0UL, 
	TOP_cmpleu_ii_r, 0x0ce00000UL, 0UL, 
	TOP_cmple_ii_r, 0x0cc00000UL, 0UL, 
	TOP_cmpltu_ii_r, 0x0d200000UL, 0UL, 
	TOP_cmplt_ii_r, 0x0d000000UL, 0UL, 
	TOP_cmpne_ii_r, 0x0c200000UL, 0UL, 
	TOP_maxu_ii, 0x0a200000UL, 0UL, 
	TOP_max_ii, 0x0a000000UL, 0UL, 
	TOP_minu_ii, 0x0a600000UL, 0UL, 
	TOP_min_ii, 0x0a400000UL, 0UL, 
	TOP_mul32_ii, 0x0dc00000UL, 0UL, 
	TOP_mul64hu_ii, 0x0fc00000UL, 0UL, 
	TOP_mul64h_ii, 0x0de00000UL, 0UL, 
	TOP_mulfrac_ii, 0x0fe00000UL, 0UL, 
	TOP_mulhhs_ii, 0x0a800000UL, 0UL, 
	TOP_mulhhu_ii, 0x0bc00000UL, 0UL, 
	TOP_mulhh_ii, 0x0ba00000UL, 0UL, 
	TOP_mulhs_ii, 0x0be00000UL, 0UL, 
	TOP_mulhu_ii, 0x0b000000UL, 0UL, 
	TOP_mulh_ii, 0x0ae00000UL, 0UL, 
	TOP_mullhus_ii, 0x09e00000UL, 0UL, 
	TOP_mullhu_ii, 0x0b800000UL, 0UL, 
	TOP_mullh_ii, 0x0b600000UL, 0UL, 
	TOP_mulllu_ii, 0x0b400000UL, 0UL, 
	TOP_mulll_ii, 0x0b200000UL, 0UL, 
	TOP_mullu_ii, 0x0ac00000UL, 0UL, 
	TOP_mull_ii, 0x0aa00000UL, 0UL, 
	TOP_nandl_ii_r, 0x0d600000UL, 0UL, 
	TOP_norl_ii_r, 0x0da00000UL, 0UL, 
	TOP_orc_ii, 0x09800000UL, 0UL, 
	TOP_orl_ii_r, 0x0d800000UL, 0UL, 
	TOP_or_ii, 0x09600000UL, 0UL, 
	TOP_sh1add_ii, 0x08a00000UL, 0UL, 
	TOP_sh2add_ii, 0x08c00000UL, 0UL, 
	TOP_sh3add_ii, 0x08e00000UL, 0UL, 
	TOP_sh4add_ii, 0x09000000UL, 0UL, 
	TOP_shl_ii, 0x08400000UL, 0UL, 
	TOP_shru_ii, 0x08800000UL, 0UL, 
	TOP_shr_ii, 0x08600000UL, 0UL, 
	TOP_xor_ii, 0x09a00000UL, 0UL, 
	TOP_UNDEFINED);

  /* ===== p28: ===== */
  ISA_PACK_TYPE p28 = ISA_Pack_Type_Create("p28");
  Result(0, 6, 6);
  Operand(0, 0, 12, 9);
  Instruction_Pack_Group(p28,
	TOP_addpc_i, 0x09000000UL, 
	TOP_mov_i, 0x08000000UL, 
	TOP_UNDEFINED);

  /* ===== p29: ===== */
  ISA_PACK_TYPE p29 = ISA_Pack_Type_Create("p29");
  Result(0, 6, 6);
  Operand(0, 0, 12, 9);
  Next_Word(); 
  Operand(0, 9, 0, 23);
  Instruction_Pack_Group(p29,
	TOP_addpc_ii, 0x09000000UL, 0UL, 
	TOP_mov_ii, 0x08000000UL, 0UL, 
	TOP_UNDEFINED);

  /* ===== p30: ===== */
  ISA_PACK_TYPE p30 = ISA_Pack_Type_Create("p30");
  Result(0, 6, 6);
  Operand(0, 0, 12, 9);
  Operand(1, 0, 0, 6);
  Instruction_Pack_Group(p30,
	TOP_ldbu_d_i, 0x24800000UL, 
	TOP_ldbu_i, 0x24000000UL, 
	TOP_ldb_d_i, 0x23800000UL, 
	TOP_ldb_i, 0x23000000UL, 
	TOP_ldhu_d_i, 0x22800000UL, 
	TOP_ldhu_i, 0x22000000UL, 
	TOP_ldh_d_i, 0x21800000UL, 
	TOP_ldh_i, 0x21000000UL, 
	TOP_ldp_i, 0x20000000UL, 
	TOP_ldw_d_i, 0x20800000UL, 
	TOP_ldw_i, 0x20000000UL, 
	TOP_multi_ldp_i, 0x20000000UL, 
	TOP_sub_i, 0x08200000UL, 
	TOP_UNDEFINED);

  /* ===== p31: ===== */
  ISA_PACK_TYPE p31 = ISA_Pack_Type_Create("p31");
  Result(0, 6, 6);
  Operand(0, 0, 12, 9);
  Operand(1, 0, 0, 6);
  Next_Word(); 
  Operand(0, 9, 0, 23);
  Instruction_Pack_Group(p31,
	TOP_ldbu_d_ii, 0x24800000UL, 0UL, 
	TOP_ldbu_ii, 0x24000000UL, 0UL, 
	TOP_ldb_d_ii, 0x23800000UL, 0UL, 
	TOP_ldb_ii, 0x23000000UL, 0UL, 
	TOP_ldhu_d_ii, 0x22800000UL, 0UL, 
	TOP_ldhu_ii, 0x22000000UL, 0UL, 
	TOP_ldh_d_ii, 0x21800000UL, 0UL, 
	TOP_ldh_ii, 0x21000000UL, 0UL, 
	TOP_ldp_ii, 0x20000000UL, 0UL, 
	TOP_ldw_d_ii, 0x20800000UL, 0UL, 
	TOP_ldw_ii, 0x20000000UL, 0UL, 
	TOP_multi_ldp_ii, 0x20000000UL, 0UL, 
	TOP_sub_ii, 0x08200000UL, 0UL, 
	TOP_UNDEFINED);

  /* ===== p32: ===== */
  ISA_PACK_TYPE p32 = ISA_Pack_Type_Create("p32");
  Result(0, 6, 6);
  Operand(0, 0, 21, 3);
  Instruction_Pack_Group(p32,
	TOP_mfb, 0x19001000UL, 
	TOP_UNDEFINED);

  /* ===== p33: ===== */
  ISA_PACK_TYPE p33 = ISA_Pack_Type_Create("p33");
  Result(0, 6, 6);
  Operand(0, 0, 21, 3);
  Operand(1, 0, 0, 6);
  Operand(2, 0, 12, 9);
  Instruction_Pack_Group(p33,
	TOP_slctf_i, 0x19000000UL, 
	TOP_slct_i, 0x18000000UL, 
	TOP_UNDEFINED);

  /* ===== p34: ===== */
  ISA_PACK_TYPE p34 = ISA_Pack_Type_Create("p34");
  Result(0, 6, 6);
  Operand(0, 0, 21, 3);
  Operand(1, 0, 0, 6);
  Operand(2, 0, 12, 9);
  Next_Word(); 
  Operand(2, 9, 0, 23);
  Instruction_Pack_Group(p34,
	TOP_slctf_ii, 0x19000000UL, 0UL, 
	TOP_slct_ii, 0x18000000UL, 0UL, 
	TOP_UNDEFINED);

  /* ===== p35: ===== */
  ISA_PACK_TYPE p35 = ISA_Pack_Type_Create("p35");
  Result(0, 6, 6);
  Operand(0, 0, 21, 3);
  Operand(1, 0, 12, 9);
  Operand(2, 0, 0, 6);
  Instruction_Pack_Group(p35,
	TOP_ldbc_i, 0x24000000UL, 
	TOP_ldbuc_i, 0x25000000UL, 
	TOP_ldhc_i, 0x22000000UL, 
	TOP_ldhuc_i, 0x23000000UL, 
	TOP_ldpc_i, 0x20000000UL, 
	TOP_ldwc_i, 0x21000000UL, 
	TOP_multi_ldpc_i, 0x20000000UL, 
	TOP_UNDEFINED);

  /* ===== p36: ===== */
  ISA_PACK_TYPE p36 = ISA_Pack_Type_Create("p36");
  Result(0, 6, 6);
  Operand(0, 0, 21, 3);
  Operand(1, 0, 12, 9);
  Operand(2, 0, 0, 6);
  Next_Word(); 
  Operand(1, 9, 0, 23);
  Instruction_Pack_Group(p36,
	TOP_ldbc_ii, 0x24000000UL, 0UL, 
	TOP_ldbuc_ii, 0x25000000UL, 0UL, 
	TOP_ldhc_ii, 0x22000000UL, 0UL, 
	TOP_ldhuc_ii, 0x23000000UL, 0UL, 
	TOP_ldpc_ii, 0x20000000UL, 0UL, 
	TOP_ldwc_ii, 0x21000000UL, 0UL, 
	TOP_multi_ldpc_ii, 0x20000000UL, 0UL, 
	TOP_UNDEFINED);

  ISA_Pack_End(); 
  return 0; 
} 
