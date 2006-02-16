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

// Generate an ISA containing the instructions specified. 
///////////////////////////////////////////////////////// 
// The instructions are listed by category. The different 
// categories of instructions are specified in the ISA manual:
// Within each category, the instructions are in alphabetical order.
// This arrangement of instructions matches the order in the ISA manual.
/////////////////////////////////////


#include <stddef.h>
#include "isa_gen.h"


main ()
{
  ISA_Create ("st200", 
		 "addcg",
		 "addf_n",
		 "addpc_i",
		 "addpc_ii",
		 "add_r",
		 "add_i",
		 "add_ii",
		 "spadjust",	 // simulated
		 "andc_r",
		 "andc_i",
		 "andc_ii",
		 "andl_r_b",
		 "andl_i_b",
		 "andl_ii_b",
		 "andl_r_r",
		 "andl_i_r",
		 "andl_ii_r",
		 "and_r",
		 "and_i",
		 "and_ii",
		 "asm_0",
		 "asm_10",
		 "asm_11",
		 "asm_12",
		 "asm_13",
		 "asm_14",
		 "asm_15",
		 "asm_16_i",
		 "asm_16_ii",
		 "asm_17_i",
		 "asm_17_ii",
		 "asm_18_i",
		 "asm_18_ii",
		 "asm_19_i",
		 "asm_19_ii",
		 "asm_1",
		 "asm_20_i",
		 "asm_20_ii",
		 "asm_21_i",
		 "asm_21_ii",
		 "asm_22_i",
		 "asm_22_ii",
		 "asm_23_i",
		 "asm_23_ii",
		 "asm_24_i",
		 "asm_24_ii",
		 "asm_25_i",
		 "asm_25_ii",
		 "asm_26_i",
		 "asm_26_ii",
		 "asm_27_i",
		 "asm_27_ii",
		 "asm_28_i",
		 "asm_28_ii",
		 "asm_29_i",
		 "asm_29_ii",
		 "asm_2",
		 "asm_30_i",
		 "asm_30_ii",
		 "asm_31_i",
		 "asm_31_ii",
		 "asm_3",
		 "asm_4",
		 "asm_5",
		 "asm_6",
		 "asm_7",
		 "asm_8",
		 "asm_9",
		 "begin_pregtn",	 // dummy
		 "break",
		 "brf",
		 "br",
		 "bswap",
		 "bwd_bar",	 // dummy
		 "CALL",	 // dummy
		 "call",
		 "clz",
		 "cmpeqf_n_b",
		 "cmpeqf_n_r",
		 "cmpeq_r_b",
		 "cmpeq_i_b",
		 "cmpeq_ii_b",
		 "cmpeq_r_r",
		 "cmpeq_i_r",
		 "cmpeq_ii_r",
		 "cmpgef_n_b",
		 "cmpgef_n_r",
		 "cmpgeu_r_b",
		 "cmpgeu_i_b",
		 "cmpgeu_ii_b",
		 "cmpgeu_r_r",
		 "cmpgeu_i_r",
		 "cmpgeu_ii_r",
		 "cmpge_r_b",
		 "cmpge_i_b",
		 "cmpge_ii_b",
		 "cmpge_r_r",
		 "cmpge_i_r",
		 "cmpge_ii_r",
		 "cmpgtf_n_b",
		 "cmpgtf_n_r",
		 "cmpgtu_r_b",
		 "cmpgtu_i_b",
		 "cmpgtu_ii_b",
		 "cmpgtu_r_r",
		 "cmpgtu_i_r",
		 "cmpgtu_ii_r",
		 "cmpgt_r_b",
		 "cmpgt_i_b",
		 "cmpgt_ii_b",
		 "cmpgt_r_r",
		 "cmpgt_i_r",
		 "cmpgt_ii_r",
		 "cmplef_n_b",
		 "cmplef_n_r",
		 "cmpleu_r_b",
		 "cmpleu_i_b",
		 "cmpleu_ii_b",
		 "cmpleu_r_r",
		 "cmpleu_i_r",
		 "cmpleu_ii_r",
		 "cmple_r_b",
		 "cmple_i_b",
		 "cmple_ii_b",
		 "cmple_r_r",
		 "cmple_i_r",
		 "cmple_ii_r",
		 "cmpltf_n_b",
		 "cmpltf_n_r",
		 "cmpltu_r_b",
		 "cmpltu_i_b",
		 "cmpltu_ii_b",
		 "cmpltu_r_r",
		 "cmpltu_i_r",
		 "cmpltu_ii_r",
		 "cmplt_r_b",
		 "cmplt_i_b",
		 "cmplt_ii_b",
		 "cmplt_r_r",
		 "cmplt_i_r",
		 "cmplt_ii_r",
		 "cmpne_r_b",
		 "cmpne_i_b",
		 "cmpne_ii_b",
		 "cmpne_r_r",
		 "cmpne_i_r",
		 "cmpne_ii_r",
		 "composep",	 // simulated
		 "convfi_n",
		 "convif_n",
		 "COPY",	 // dummy
		 "divs",
		 "divu",
		 "div",
		 "end_pregtn",	 // dummy
		 "extractp",	 // simulated
		 "FALL",	 // dummy
		 "fwd_bar",	 // dummy
		 "getpc",	 // simulated
		 "asm",	 // simulated
		 "GOTO",	 // dummy
		 "goto",
		 "icall",
		 "idle",
		 "ifixup",	 // dummy
		 "igoto",
		 "intrncall",	 // dummy
		 "JUMP",	 // dummy
		 "KILL",	 // dummy
		 "label",	 // dummy
		 "ldbc_i",
		 "ldbc_ii",
		 "ldbuc_i",
		 "ldbuc_ii",
		 "ldbu_d_i",
		 "ldbu_d_ii",
		 "ldbu_i",
		 "ldbu_ii",
		 "ldb_d_i",
		 "ldb_d_ii",
		 "ldb_i",
		 "ldb_ii",
		 "ldhc_i",
		 "ldhc_ii",
		 "ldhuc_i",
		 "ldhuc_ii",
		 "ldhu_d_i",
		 "ldhu_d_ii",
		 "ldhu_i",
		 "ldhu_ii",
		 "ldh_d_i",
		 "ldh_d_ii",
		 "ldh_i",
		 "ldh_ii",
		 "ldlc_i",
		 "ldlc_ii",
		 "ldl_i",
		 "ldl_ii",
		 "ldwc_i",
		 "ldwc_ii",
		 "ldwl",
		 "ldw_d_i",
		 "ldw_d_ii",
		 "ldw_i",
		 "ldw_ii",
		 "LINK",	 // dummy
		 "LOOP",	 // dummy
		 "maxu_r",
		 "maxu_i",
		 "maxu_ii",
		 "max_r",
		 "max_i",
		 "max_ii",
		 "mfb",
		 "minu_r",
		 "minu_i",
		 "minu_ii",
		 "min_r",
		 "min_i",
		 "min_ii",
		 "movc",	 // dummy
		 "movcf",	 // dummy
		 "movp",	 // simulated
		 "mov_r",
		 "mov_i",
		 "mov_ii",
		 "mtb",
		 "mul32_r",
		 "mul32_i",
		 "mul32_ii",
		 "mul64hu_r",
		 "mul64hu_i",
		 "mul64hu_ii",
		 "mul64h_r",
		 "mul64h_i",
		 "mul64h_ii",
		 "mulfrac_r",
		 "mulfrac_i",
		 "mulfrac_ii",
		 "mulf_n",
		 "mulhhs_r",
		 "mulhhs_i",
		 "mulhhs_ii",
		 "mulhhu_r",
		 "mulhhu_i",
		 "mulhhu_ii",
		 "mulhh_r",
		 "mulhh_i",
		 "mulhh_ii",
		 "mulhs_r",
		 "mulhs_i",
		 "mulhs_ii",
		 "mulhu_r",
		 "mulhu_i",
		 "mulhu_ii",
		 "mulh_r",
		 "mulh_i",
		 "mulh_ii",
		 "mullhus_r",
		 "mullhus_i",
		 "mullhus_ii",
		 "mullhu_r",
		 "mullhu_i",
		 "mullhu_ii",
		 "mullh_r",
		 "mullh_i",
		 "mullh_ii",
		 "mulllu_r",
		 "mulllu_i",
		 "mulllu_ii",
		 "mulll_r",
		 "mulll_i",
		 "mulll_ii",
		 "mullu_r",
		 "mullu_i",
		 "mullu_ii",
		 "mull_r",
		 "mull_i",
		 "mull_ii",
		 "multi_ldlc_i",
		 "multi_ldlc_ii",
		 "multi_ldl_i",
		 "multi_ldl_ii",
		 "multi_stlc_i",
		 "multi_stlc_ii",
		 "multi_stl_i",
		 "multi_stl_ii",
		 "nandl_r_b",
		 "nandl_i_b",
		 "nandl_ii_b",
		 "nandl_r_r",
		 "nandl_i_r",
		 "nandl_ii_r",
		 "noop",	 // dummy
		 "nop",
		 "norl_r_b",
		 "norl_i_b",
		 "norl_ii_b",
		 "norl_r_r",
		 "norl_i_r",
		 "norl_ii_r",
		 "orc_r",
		 "orc_i",
		 "orc_ii",
		 "orl_r_b",
		 "orl_i_b",
		 "orl_ii_b",
		 "orl_r_r",
		 "orl_i_r",
		 "orl_ii_r",
		 "or_r",
		 "or_i",
		 "or_ii",
		 "pftc_i",
		 "pftc_ii",
		 "pft_i",
		 "pft_ii",
		 "phi",	 // ssa
		 "prgadd_i",
		 "prgadd_ii",
		 "prgins",
		 "prginsadd_i",
		 "prginsadd_ii",
		 "prginspg_i",
		 "prginspg_ii",
		 "prgset_i",
		 "prgset_ii",
		 "psi",	 // ssa
		 "pswclr",
		 "pswset",
		 "pushregs",	 // simulated
		 "remu",
		 "rem",
		 "RETURN",	 // dummy
		 "return",
		 "rfi",
		 "sbrk",
		 "st235_sbrk",
		 "sh1add_r",
		 "sh1add_i",
		 "sh1add_ii",
		 "sh2add_r",
		 "sh2add_i",
		 "sh2add_ii",
		 "sh3add_r",
		 "sh3add_i",
		 "sh3add_ii",
		 "sh4add_r",
		 "sh4add_i",
		 "sh4add_ii",
		 "shl_r",
		 "shl_i",
		 "shl_ii",
		 "shru_r",
		 "shru_i",
		 "shru_ii",
		 "shr_r",
		 "shr_i",
		 "shr_ii",
		 "SIGMA",	 // dummy
		 "slctf_r",
		 "slctf_i",
		 "slctf_ii",
		 "slct_r",
		 "slct_i",
		 "slct_ii",
		 "stbc_i",
		 "stbc_ii",
		 "stb_i",
		 "stb_ii",
		 "sthc_i",
		 "sthc_ii",
		 "sth_i",
		 "sth_ii",
		 "stlc_i",
		 "stlc_ii",
		 "stl_i",
		 "stl_ii",
		 "stwc_i",
		 "stwc_ii",
		 "stwl",
		 "stw_i",
		 "stw_ii",
		 "subf_n",
		 "sub_r",
		 "sub_i",
		 "sub_ii",
		 "sxtb",
		 "sxth",
		 "sync",
		 "syncins",
		 "syscall",
		 "st235_syscall",
		 "wmb",
		 "xor_r",
		 "xor_i",
		 "xor_ii",
		 "zxtb",
		 "zxth",
      NULL);
}
