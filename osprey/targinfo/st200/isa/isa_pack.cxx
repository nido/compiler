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
// Group TOPs with similar packing format together.  
///////////////////////////////////////////////////////// 
// Instructions may be packed (compressed) in binary files. The packing rules 
// are specified in this file. 

#include <stddef.h> 
#include "topcode.h" 
#include "isa_pack_gen.h" 

main() 
{ 
  ISA_PACK_TYPE 
		p0, 	// Int3R: dest = src1, src2
		p1, 	// Int3I: idest = src1, isrc2
		p2, 	// Int3E: idest = src1, isrc2 / imml
		p3, 	// Cmp3R_Reg: dest = src1, src2
		p4, 	// Cmp3R_Br: bdest = src1, src2
		p5, 	// Cmp3I_Reg: idest = src1, isrc2
		p6, 	// Cmp3E_Reg: idest = src1, isrc2 / imml
		p7, 	// Cmp3I_Br: ibdest = src1, isrc2
		p8, 	// Cmp3E_Br: ibdest = src1, isrc2 / imml
		p9, 	// Imm: imm
		p10, 	// SelectR: dest = scond, src1, src2
		p11, 	// SelectI: idest = scond, src1, isrc2
		p12, 	// SelectE: idest = scond, src1, isrc2 / imml
		p13, 	// cgen: dest, bdest = src1, src2, scond
		p14, 	// SysOp: nada
		p15, 	// Load: idest = isrc2, src1
		p16, 	// LoadE: idest = isrc2, src1 / imml
		p17, 	// Store: isrc2, src1, src2
		p18, 	// StoreE: isrc2, src1, src2 / imml
		p19, 	// Call: lr = btarg
		p20, 	// Branch: bcond, btarg
		p21, 	// Jump: btarg
		p22, 	// Extr: immr / idest = isrc2
		p23, 	// sub_i: idest = isrc2, src1
		p24, 	// sub_ii: idest = isrc2, src1 / imml
		p25, 	// Icall: lr = lr
		p26, 	// Ijump: igoto lr
		p27, 	// rfi
		p28, 	// nop
		p29, 	// MoveR: dest = src2 
		p30, 	// MoveI: idest = isrc2 
		p31, 	// MoveE: idest = isrc2 / imml
		p32, 	// mtb: bdest = src1 
		p33, 	// mfb: idest = scond 
		p34, 	// cache opcodes 
		p35, 	// xcache opcodes 
		p36, 	// sync
		p37; 	// Monadic: dest = src1

  OPND_ADJ_TYPE	no_adj; 

  ISA_Pack_Begin("st200", 32); 

  /* Create the various adjustments that need to be performed between 
   * assembly language form and packed form. Note that simple shift  
   * adjustments have been accomplished directly in the operand packing 
   * specification. 
   */ 

  no_adj = Create_Operand_Adjustment("no adjustment", "O_VAL"); 

  /* =====  p27: ===== */ 
  p27 = ISA_Pack_Type_Create("p27"); 
  Instruction_Pack_Group(p27, 
	TOP_rfi, 	 0x10000000UL,
	TOP_UNDEFINED); 

  /* =====  p13: ===== */ 
  p13 = ISA_Pack_Type_Create("p13"); 
  Result(0, 12, 6); 
  Result(1, 18, 3); 
  Operand(0, 0, 0, 6); 
  Operand(1, 0, 6, 6); 
  Operand(2, 0, 21, 3); 
  Instruction_Pack_Group(p13, 
	TOP_addcg, 	 0x10000000UL,
	TOP_divs, 	 0x10000000UL,
	TOP_UNDEFINED); 

  /* =====  p12: ===== */ 
  p12 = ISA_Pack_Type_Create("p12"); 
  Result(0, 6, 6); 
  Operand(0, 0, 21, 3); 
  Operand(1, 0, 0, 6); 
  Operand(2, 0, 12, 9); 
  Next_Word(); 
  Operand(0, 9, 0, 23); 
  Instruction_Pack_Group(p12, 
	TOP_slct_ii, 	 0x10000000UL, 0x10000000UL,
	TOP_slctf_ii, 	 0x10000000UL, 0x10000000UL,
	TOP_UNDEFINED); 

  /* =====  p32: ===== */ 
  p32 = ISA_Pack_Type_Create("p32"); 
  Result(0, 18, 3); 
  Operand(0, 0, 0, 6); 
  Instruction_Pack_Group(p32, 
	TOP_mtb, 	 0x10000000UL,
	TOP_UNDEFINED); 

  /* =====  p30: ===== */ 
  p30 = ISA_Pack_Type_Create("p30"); 
  Result(0, 6, 6); 
  Operand(0, 0, 12, 9); 
  Instruction_Pack_Group(p30, 
	TOP_mov_i, 	 0x10000000UL,
	TOP_UNDEFINED); 

  /* =====  p11: ===== */ 
  p11 = ISA_Pack_Type_Create("p11"); 
  Result(0, 6, 6); 
  Operand(0, 0, 21, 3); 
  Operand(1, 0, 0, 6); 
  Operand(2, 0, 12, 9); 
  Instruction_Pack_Group(p11, 
	TOP_slct_i, 	 0x10000000UL,
	TOP_slctf_i, 	 0x10000000UL,
	TOP_UNDEFINED); 

  /* =====  p35: ===== */ 
  p35 = ISA_Pack_Type_Create("p35"); 
  Operand(0, 0, 0, 6); 
  Next_Word(); 
  Operand(0, 9, 0, 23); 
  Instruction_Pack_Group(p35, 
	TOP_pft_ii, 	 0x10000000UL, 0x10000000UL,
	TOP_prgadd_ii, 	 0x10000000UL, 0x10000000UL,
	TOP_prgset_ii, 	 0x10000000UL, 0x10000000UL,
	TOP_UNDEFINED); 

  /* =====  p26: ===== */ 
  p26 = ISA_Pack_Type_Create("p26"); 
  Instruction_Pack_Group(p26, 
	TOP_igoto, 	 0x10000000UL,
	TOP_return, 	 0x10000000UL,
	TOP_UNDEFINED); 

  /* =====  p0: ===== */ 
  p0 = ISA_Pack_Type_Create("p0"); 
  Result(0, 12, 6); 
  Operand(0, 0, 0, 6); 
  Operand(1, 0, 6, 6); 
  Instruction_Pack_Group(p0, 
	TOP_add_r, 	 0x10000000UL,
	TOP_and_r, 	 0x10000000UL,
	TOP_andc_r, 	 0x10000000UL,
	TOP_max_r, 	 0x10000000UL,
	TOP_maxu_r, 	 0x10000000UL,
	TOP_min_r, 	 0x10000000UL,
	TOP_minu_r, 	 0x10000000UL,
	TOP_mulh_r, 	 0x10000000UL,
	TOP_mulhh_r, 	 0x10000000UL,
	TOP_mulhhs_r, 	 0x10000000UL,
	TOP_mulhhu_r, 	 0x10000000UL,
	TOP_mulhs_r, 	 0x10000000UL,
	TOP_mulhu_r, 	 0x10000000UL,
	TOP_mull_r, 	 0x10000000UL,
	TOP_mullh_r, 	 0x10000000UL,
	TOP_mullhu_r, 	 0x10000000UL,
	TOP_mullhus_r, 	 0x10000000UL,
	TOP_mulll_r, 	 0x10000000UL,
	TOP_mulllu_r, 	 0x10000000UL,
	TOP_mullu_r, 	 0x10000000UL,
	TOP_or_r, 	 0x10000000UL,
	TOP_orc_r, 	 0x10000000UL,
	TOP_sh1add_r, 	 0x10000000UL,
	TOP_sh2add_r, 	 0x10000000UL,
	TOP_sh3add_r, 	 0x10000000UL,
	TOP_sh4add_r, 	 0x10000000UL,
	TOP_shl_r, 	 0x10000000UL,
	TOP_shr_r, 	 0x10000000UL,
	TOP_shru_r, 	 0x10000000UL,
	TOP_sub_r, 	 0x10000000UL,
	TOP_xor_r, 	 0x10000000UL,
	TOP_UNDEFINED); 

  /* =====  p36: ===== */ 
  p36 = ISA_Pack_Type_Create("p36"); 
  Instruction_Pack_Group(p36, 
	TOP_sync, 	 0x10000000UL,
	TOP_UNDEFINED); 

  /* =====  p7: ===== */ 
  p7 = ISA_Pack_Type_Create("p7"); 
  Result(0, 6, 3); 
  Operand(0, 0, 0, 6); 
  Operand(1, 0, 12, 9); 
  Instruction_Pack_Group(p7, 
	TOP_andl_i_b, 	 0x10000000UL,
	TOP_cmpeq_i_b, 	 0x10000000UL,
	TOP_cmpge_i_b, 	 0x10000000UL,
	TOP_cmpgeu_i_b, 	 0x10000000UL,
	TOP_cmpgt_i_b, 	 0x10000000UL,
	TOP_cmpgtu_i_b, 	 0x10000000UL,
	TOP_cmple_i_b, 	 0x10000000UL,
	TOP_cmpleu_i_b, 	 0x10000000UL,
	TOP_cmplt_i_b, 	 0x10000000UL,
	TOP_cmpltu_i_b, 	 0x10000000UL,
	TOP_cmpne_i_b, 	 0x10000000UL,
	TOP_nandl_i_b, 	 0x10000000UL,
	TOP_norl_i_b, 	 0x10000000UL,
	TOP_orl_i_b, 	 0x10000000UL,
	TOP_UNDEFINED); 

  /* =====  p31: ===== */ 
  p31 = ISA_Pack_Type_Create("p31"); 
  Result(0, 6, 6); 
  Operand(0, 0, 12, 9); 
  Next_Word(); 
  Operand(0, 9, 0, 23); 
  Instruction_Pack_Group(p31, 
	TOP_mov_ii, 	 0x10000000UL, 0x10000000UL,
	TOP_UNDEFINED); 

  /* =====  p19: ===== */ 
  p19 = ISA_Pack_Type_Create("p19"); 
  Operand(0, 0, 0, 23); 
  Instruction_Pack_Group(p19, 
	TOP_call, 	 0x10000000UL,
	TOP_UNDEFINED); 

  /* =====  p16: ===== */ 
  p16 = ISA_Pack_Type_Create("p16"); 
  Result(0, 6, 6); 
  Operand(0, 0, 12, 9); 
  Operand(1, 0, 0, 6); 
  Next_Word(); 
  Operand(0, 9, 0, 23); 
  Instruction_Pack_Group(p16, 
	TOP_ldb_ii, 	 0x10000000UL, 0x10000000UL,
	TOP_ldb_d_ii, 	 0x10000000UL, 0x10000000UL,
	TOP_ldbu_ii, 	 0x10000000UL, 0x10000000UL,
	TOP_ldbu_d_ii, 	 0x10000000UL, 0x10000000UL,
	TOP_ldh_ii, 	 0x10000000UL, 0x10000000UL,
	TOP_ldh_d_ii, 	 0x10000000UL, 0x10000000UL,
	TOP_ldhu_ii, 	 0x10000000UL, 0x10000000UL,
	TOP_ldhu_d_ii, 	 0x10000000UL, 0x10000000UL,
	TOP_ldw_ii, 	 0x10000000UL, 0x10000000UL,
	TOP_ldw_d_ii, 	 0x10000000UL, 0x10000000UL,
	TOP_UNDEFINED); 

  /* =====  p3: ===== */ 
  p3 = ISA_Pack_Type_Create("p3"); 
  Result(0, 12, 6); 
  Operand(0, 0, 0, 6); 
  Operand(1, 0, 6, 6); 
  Instruction_Pack_Group(p3, 
	TOP_andl_r_r, 	 0x10000000UL,
	TOP_cmpeq_r_r, 	 0x10000000UL,
	TOP_cmpge_r_r, 	 0x10000000UL,
	TOP_cmpgeu_r_r, 	 0x10000000UL,
	TOP_cmpgt_r_r, 	 0x10000000UL,
	TOP_cmpgtu_r_r, 	 0x10000000UL,
	TOP_cmple_r_r, 	 0x10000000UL,
	TOP_cmpleu_r_r, 	 0x10000000UL,
	TOP_cmplt_r_r, 	 0x10000000UL,
	TOP_cmpltu_r_r, 	 0x10000000UL,
	TOP_cmpne_r_r, 	 0x10000000UL,
	TOP_nandl_r_r, 	 0x10000000UL,
	TOP_norl_r_r, 	 0x10000000UL,
	TOP_orl_r_r, 	 0x10000000UL,
	TOP_UNDEFINED); 

  /* =====  p20: ===== */ 
  p20 = ISA_Pack_Type_Create("p20"); 
  Operand(0, 0, 23, 3); 
  Operand(1, 0, 0, 23); 
  Instruction_Pack_Group(p20, 
	TOP_br, 	 0x10000000UL,
	TOP_brf, 	 0x10000000UL,
	TOP_UNDEFINED); 

  /* =====  p8: ===== */ 
  p8 = ISA_Pack_Type_Create("p8"); 
  Result(0, 6, 3); 
  Operand(0, 0, 0, 6); 
  Operand(1, 0, 12, 9); 
  Next_Word(); 
  Operand(0, 9, 0, 23); 
  Instruction_Pack_Group(p8, 
	TOP_andl_ii_b, 	 0x10000000UL, 0x10000000UL,
	TOP_cmpeq_ii_b, 	 0x10000000UL, 0x10000000UL,
	TOP_cmpge_ii_b, 	 0x10000000UL, 0x10000000UL,
	TOP_cmpgeu_ii_b, 	 0x10000000UL, 0x10000000UL,
	TOP_cmpgt_ii_b, 	 0x10000000UL, 0x10000000UL,
	TOP_cmpgtu_ii_b, 	 0x10000000UL, 0x10000000UL,
	TOP_cmple_ii_b, 	 0x10000000UL, 0x10000000UL,
	TOP_cmpleu_ii_b, 	 0x10000000UL, 0x10000000UL,
	TOP_cmplt_ii_b, 	 0x10000000UL, 0x10000000UL,
	TOP_cmpltu_ii_b, 	 0x10000000UL, 0x10000000UL,
	TOP_cmpne_ii_b, 	 0x10000000UL, 0x10000000UL,
	TOP_nandl_ii_b, 	 0x10000000UL, 0x10000000UL,
	TOP_norl_ii_b, 	 0x10000000UL, 0x10000000UL,
	TOP_orl_ii_b, 	 0x10000000UL, 0x10000000UL,
	TOP_UNDEFINED); 

  /* =====  p24: ===== */ 
  p24 = ISA_Pack_Type_Create("p24"); 
  Result(0, 6, 6); 
  Operand(0, 0, 12, 9); 
  Operand(1, 0, 0, 6); 
  Next_Word(); 
  Operand(0, 9, 0, 23); 
  Instruction_Pack_Group(p24, 
	TOP_sub_ii, 	 0x10000000UL, 0x10000000UL,
	TOP_UNDEFINED); 

  /* =====  p5: ===== */ 
  p5 = ISA_Pack_Type_Create("p5"); 
  Result(0, 6, 6); 
  Operand(0, 0, 0, 6); 
  Operand(1, 0, 12, 9); 
  Instruction_Pack_Group(p5, 
	TOP_andl_i_r, 	 0x10000000UL,
	TOP_cmpeq_i_r, 	 0x10000000UL,
	TOP_cmpge_i_r, 	 0x10000000UL,
	TOP_cmpgeu_i_r, 	 0x10000000UL,
	TOP_cmpgt_i_r, 	 0x10000000UL,
	TOP_cmpgtu_i_r, 	 0x10000000UL,
	TOP_cmple_i_r, 	 0x10000000UL,
	TOP_cmpleu_i_r, 	 0x10000000UL,
	TOP_cmplt_i_r, 	 0x10000000UL,
	TOP_cmpltu_i_r, 	 0x10000000UL,
	TOP_cmpne_i_r, 	 0x10000000UL,
	TOP_nandl_i_r, 	 0x10000000UL,
	TOP_norl_i_r, 	 0x10000000UL,
	TOP_orl_i_r, 	 0x10000000UL,
	TOP_UNDEFINED); 

  /* =====  p4: ===== */ 
  p4 = ISA_Pack_Type_Create("p4"); 
  Result(0, 18, 3); 
  Operand(0, 0, 0, 6); 
  Operand(1, 0, 6, 6); 
  Instruction_Pack_Group(p4, 
	TOP_andl_r_b, 	 0x10000000UL,
	TOP_cmpeq_r_b, 	 0x10000000UL,
	TOP_cmpge_r_b, 	 0x10000000UL,
	TOP_cmpgeu_r_b, 	 0x10000000UL,
	TOP_cmpgt_r_b, 	 0x10000000UL,
	TOP_cmpgtu_r_b, 	 0x10000000UL,
	TOP_cmple_r_b, 	 0x10000000UL,
	TOP_cmpleu_r_b, 	 0x10000000UL,
	TOP_cmplt_r_b, 	 0x10000000UL,
	TOP_cmpltu_r_b, 	 0x10000000UL,
	TOP_cmpne_r_b, 	 0x10000000UL,
	TOP_nandl_r_b, 	 0x10000000UL,
	TOP_norl_r_b, 	 0x10000000UL,
	TOP_orl_r_b, 	 0x10000000UL,
	TOP_UNDEFINED); 

  /* =====  p37: ===== */ 
  p37 = ISA_Pack_Type_Create("p37"); 
  Result(0, 12, 6); 
  Operand(0, 0, 0, 6); 
  Instruction_Pack_Group(p37, 
	TOP_bswap_r, 	 0x10000000UL,
	TOP_clz_r, 	 0x10000000UL,
	TOP_sxtb_r, 	 0x10000000UL,
	TOP_sxth_r, 	 0x10000000UL,
	TOP_zxth_r, 	 0x10000000UL,
	TOP_UNDEFINED); 

  /* =====  p21: ===== */ 
  p21 = ISA_Pack_Type_Create("p21"); 
  Operand(0, 0, 0, 23); 
  Instruction_Pack_Group(p21, 
	TOP_goto, 	 0x10000000UL,
	TOP_UNDEFINED); 

  /* =====  p10: ===== */ 
  p10 = ISA_Pack_Type_Create("p10"); 
  Result(0, 12, 6); 
  Operand(0, 0, 21, 3); 
  Operand(1, 0, 0, 6); 
  Operand(2, 0, 6, 6); 
  Instruction_Pack_Group(p10, 
	TOP_slct_r, 	 0x10000000UL,
	TOP_slctf_r, 	 0x10000000UL,
	TOP_UNDEFINED); 

  /* =====  p28: ===== */ 
  p28 = ISA_Pack_Type_Create("p28"); 
  Instruction_Pack_Group(p28, 
	TOP_nop, 	 0x10000000UL,
	TOP_UNDEFINED); 

  /* =====  p34: ===== */ 
  p34 = ISA_Pack_Type_Create("p34"); 
  Operand(0, 0, 0, 6); 
  Instruction_Pack_Group(p34, 
	TOP_pft_i, 	 0x10000000UL,
	TOP_prgadd_i, 	 0x10000000UL,
	TOP_prgset_i, 	 0x10000000UL,
	TOP_UNDEFINED); 

  /* =====  p29: ===== */ 
  p29 = ISA_Pack_Type_Create("p29"); 
  Result(0, 12, 6); 
  Operand(0, 0, 6, 6); 
  Instruction_Pack_Group(p29, 
	TOP_mov_r, 	 0x10000000UL,
	TOP_UNDEFINED); 

  /* =====  p18: ===== */ 
  p18 = ISA_Pack_Type_Create("p18"); 
  Operand(0, 0, 12, 9); 
  Operand(1, 0, 0, 6); 
  Operand(2, 0, 6, 6); 
  Next_Word(); 
  Operand(0, 9, 0, 23); 
  Instruction_Pack_Group(p18, 
	TOP_stb_ii, 	 0x10000000UL, 0x10000000UL,
	TOP_sth_ii, 	 0x10000000UL, 0x10000000UL,
	TOP_stw_ii, 	 0x10000000UL, 0x10000000UL,
	TOP_UNDEFINED); 

  /* =====  p23: ===== */ 
  p23 = ISA_Pack_Type_Create("p23"); 
  Result(0, 6, 6); 
  Operand(0, 0, 12, 9); 
  Operand(1, 0, 0, 6); 
  Instruction_Pack_Group(p23, 
	TOP_sub_i, 	 0x10000000UL,
	TOP_UNDEFINED); 

  /* =====  p1: ===== */ 
  p1 = ISA_Pack_Type_Create("p1"); 
  Result(0, 6, 6); 
  Operand(0, 0, 0, 6); 
  Operand(1, 0, 12, 9); 
  Instruction_Pack_Group(p1, 
	TOP_add_i, 	 0x10000000UL,
	TOP_and_i, 	 0x10000000UL,
	TOP_andc_i, 	 0x10000000UL,
	TOP_max_i, 	 0x10000000UL,
	TOP_maxu_i, 	 0x10000000UL,
	TOP_min_i, 	 0x10000000UL,
	TOP_minu_i, 	 0x10000000UL,
	TOP_mulh_i, 	 0x10000000UL,
	TOP_mulhh_i, 	 0x10000000UL,
	TOP_mulhhs_i, 	 0x10000000UL,
	TOP_mulhhu_i, 	 0x10000000UL,
	TOP_mulhs_i, 	 0x10000000UL,
	TOP_mulhu_i, 	 0x10000000UL,
	TOP_mull_i, 	 0x10000000UL,
	TOP_mullh_i, 	 0x10000000UL,
	TOP_mullhu_i, 	 0x10000000UL,
	TOP_mullhus_i, 	 0x10000000UL,
	TOP_mulll_i, 	 0x10000000UL,
	TOP_mulllu_i, 	 0x10000000UL,
	TOP_mullu_i, 	 0x10000000UL,
	TOP_or_i, 	 0x10000000UL,
	TOP_orc_i, 	 0x10000000UL,
	TOP_sh1add_i, 	 0x10000000UL,
	TOP_sh2add_i, 	 0x10000000UL,
	TOP_sh3add_i, 	 0x10000000UL,
	TOP_sh4add_i, 	 0x10000000UL,
	TOP_shl_i, 	 0x10000000UL,
	TOP_shr_i, 	 0x10000000UL,
	TOP_shru_i, 	 0x10000000UL,
	TOP_xor_i, 	 0x10000000UL,
	TOP_UNDEFINED); 

  /* =====  p25: ===== */ 
  p25 = ISA_Pack_Type_Create("p25"); 
  Instruction_Pack_Group(p25, 
	TOP_icall, 	 0x10000000UL,
	TOP_UNDEFINED); 

  /* =====  p17: ===== */ 
  p17 = ISA_Pack_Type_Create("p17"); 
  Operand(0, 0, 12, 9); 
  Operand(1, 0, 0, 6); 
  Operand(2, 0, 6, 6); 
  Instruction_Pack_Group(p17, 
	TOP_stb_i, 	 0x10000000UL,
	TOP_sth_i, 	 0x10000000UL,
	TOP_stw_i, 	 0x10000000UL,
	TOP_UNDEFINED); 

  /* =====  p9: ===== */ 
  p9 = ISA_Pack_Type_Create("p9"); 
  Operand(0, 0, 0, 23); 
  Instruction_Pack_Group(p9, 
	TOP_imml, 	 0x10000000UL,
	TOP_immr, 	 0x10000000UL,
	TOP_UNDEFINED); 

  /* =====  p14: ===== */ 
  p14 = ISA_Pack_Type_Create("p14"); 
  Instruction_Pack_Group(p14, 
	TOP_break, 	 0x10000000UL,
	TOP_prgins, 	 0x10000000UL,
	TOP_sbrk, 	 0x10000000UL,
	TOP_syscall, 	 0x10000000UL,
	TOP_UNDEFINED); 

  /* =====  p6: ===== */ 
  p6 = ISA_Pack_Type_Create("p6"); 
  Result(0, 6, 6); 
  Operand(0, 0, 0, 6); 
  Operand(1, 0, 12, 9); 
  Next_Word(); 
  Operand(0, 9, 0, 23); 
  Instruction_Pack_Group(p6, 
	TOP_andl_ii_r, 	 0x10000000UL, 0x10000000UL,
	TOP_cmpeq_ii_r, 	 0x10000000UL, 0x10000000UL,
	TOP_cmpge_ii_r, 	 0x10000000UL, 0x10000000UL,
	TOP_cmpgeu_ii_r, 	 0x10000000UL, 0x10000000UL,
	TOP_cmpgt_ii_r, 	 0x10000000UL, 0x10000000UL,
	TOP_cmpgtu_ii_r, 	 0x10000000UL, 0x10000000UL,
	TOP_cmple_ii_r, 	 0x10000000UL, 0x10000000UL,
	TOP_cmpleu_ii_r, 	 0x10000000UL, 0x10000000UL,
	TOP_cmplt_ii_r, 	 0x10000000UL, 0x10000000UL,
	TOP_cmpltu_ii_r, 	 0x10000000UL, 0x10000000UL,
	TOP_cmpne_ii_r, 	 0x10000000UL, 0x10000000UL,
	TOP_nandl_ii_r, 	 0x10000000UL, 0x10000000UL,
	TOP_norl_ii_r, 	 0x10000000UL, 0x10000000UL,
	TOP_orl_ii_r, 	 0x10000000UL, 0x10000000UL,
	TOP_UNDEFINED); 

  /* =====  p2: ===== */ 
  p2 = ISA_Pack_Type_Create("p2"); 
  Result(0, 6, 6); 
  Operand(0, 0, 0, 6); 
  Next_Word(); 
  Operand(0, 9, 0, 23); 
  Instruction_Pack_Group(p2, 
	TOP_add_ii, 	 0x10000000UL, 0x10000000UL,
	TOP_and_ii, 	 0x10000000UL, 0x10000000UL,
	TOP_andc_ii, 	 0x10000000UL, 0x10000000UL,
	TOP_max_ii, 	 0x10000000UL, 0x10000000UL,
	TOP_maxu_ii, 	 0x10000000UL, 0x10000000UL,
	TOP_min_ii, 	 0x10000000UL, 0x10000000UL,
	TOP_minu_ii, 	 0x10000000UL, 0x10000000UL,
	TOP_mulh_ii, 	 0x10000000UL, 0x10000000UL,
	TOP_mulhh_ii, 	 0x10000000UL, 0x10000000UL,
	TOP_mulhhs_ii, 	 0x10000000UL, 0x10000000UL,
	TOP_mulhhu_ii, 	 0x10000000UL, 0x10000000UL,
	TOP_mulhs_ii, 	 0x10000000UL, 0x10000000UL,
	TOP_mulhu_ii, 	 0x10000000UL, 0x10000000UL,
	TOP_mull_ii, 	 0x10000000UL, 0x10000000UL,
	TOP_mullh_ii, 	 0x10000000UL, 0x10000000UL,
	TOP_mullhu_ii, 	 0x10000000UL, 0x10000000UL,
	TOP_mullhus_ii, 	 0x10000000UL, 0x10000000UL,
	TOP_mulll_ii, 	 0x10000000UL, 0x10000000UL,
	TOP_mulllu_ii, 	 0x10000000UL, 0x10000000UL,
	TOP_mullu_ii, 	 0x10000000UL, 0x10000000UL,
	TOP_or_ii, 	 0x10000000UL, 0x10000000UL,
	TOP_orc_ii, 	 0x10000000UL, 0x10000000UL,
	TOP_sh1add_ii, 	 0x10000000UL, 0x10000000UL,
	TOP_sh2add_ii, 	 0x10000000UL, 0x10000000UL,
	TOP_sh3add_ii, 	 0x10000000UL, 0x10000000UL,
	TOP_sh4add_ii, 	 0x10000000UL, 0x10000000UL,
	TOP_shl_ii, 	 0x10000000UL, 0x10000000UL,
	TOP_shr_ii, 	 0x10000000UL, 0x10000000UL,
	TOP_shru_ii, 	 0x10000000UL, 0x10000000UL,
	TOP_xor_ii, 	 0x10000000UL, 0x10000000UL,
	TOP_UNDEFINED); 

  /* =====  p33: ===== */ 
  p33 = ISA_Pack_Type_Create("p33"); 
  Result(0, 6, 6); 
  Operand(0, 0, 21, 3); 
  Instruction_Pack_Group(p33, 
	TOP_mfb, 	 0x10000000UL,
	TOP_UNDEFINED); 

  /* =====  p15: ===== */ 
  p15 = ISA_Pack_Type_Create("p15"); 
  Result(0, 6, 6); 
  Operand(0, 0, 12, 9); 
  Operand(1, 0, 0, 6); 
  Instruction_Pack_Group(p15, 
	TOP_ldb_i, 	 0x10000000UL,
	TOP_ldb_d_i, 	 0x10000000UL,
	TOP_ldbu_i, 	 0x10000000UL,
	TOP_ldbu_d_i, 	 0x10000000UL,
	TOP_ldh_i, 	 0x10000000UL,
	TOP_ldh_d_i, 	 0x10000000UL,
	TOP_ldhu_i, 	 0x10000000UL,
	TOP_ldhu_d_i, 	 0x10000000UL,
	TOP_ldw_i, 	 0x10000000UL,
	TOP_ldw_d_i, 	 0x10000000UL,
	TOP_UNDEFINED); 

  ISA_Pack_End(); 
  return 0; 
} 
