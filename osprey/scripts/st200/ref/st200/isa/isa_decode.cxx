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
// Generate instruction decoding information. 
///////////////////////////////////// 
///////////////////////////////////// 

#include "topcode.h" 
#include "isa_decode_gen.h" 
#include "targ_isa_bundle.h" 

main() 
{ 

  ISA_Decode_Begin("st200"); 

  STATE ex_unit = Create_Unit_State("ex_unit", 0, 4); 

  STATE alu = Create_Inst_State("alu", 0, 0, 15); 

  Transitions(ex_unit, 
        ISA_EXEC_S0_Unit, alu, 
        ISA_EXEC_S1_Unit, alu, 
        ISA_EXEC_S2_Unit, alu, 
        ISA_EXEC_S3_Unit, alu, 
        ISA_EXEC_EXT0_Unit, alu, 
        ISA_EXEC_EXT1_Unit, alu, 
        ISA_EXEC_EXT2_Unit, alu, 
        ISA_EXEC_Odd_Unit, alu, 
        ISA_EXEC_Even_Unit, alu, 
        ISA_EXEC_ReqS0_Unit, alu, 
	      END_TRANSITIONS); 

  Transitions(alu, 
	   0,	 Final(TOP_add_r),
	   1,	 Final(TOP_add_i),
	   2,	 Final(TOP_add_ii),
	   3,	 Final(TOP_addcg),
	   4,	 Final(TOP_and_r),
	   5,	 Final(TOP_and_i),
	   6,	 Final(TOP_and_ii),
	   7,	 Final(TOP_andc_r),
	   8,	 Final(TOP_andc_i),
	   9,	 Final(TOP_andc_ii),
	   10,	 Final(TOP_andl_r_r),
	   11,	 Final(TOP_andl_r_b),
	   12,	 Final(TOP_andl_i_r),
	   13,	 Final(TOP_andl_ii_r),
	   14,	 Final(TOP_andl_i_b),
	   15,	 Final(TOP_andl_ii_b),
	   16,	 Final(TOP_asm_0),
	   17,	 Final(TOP_asm_1),
	   18,	 Final(TOP_asm_2),
	   19,	 Final(TOP_asm_3),
	   20,	 Final(TOP_asm_4),
	   21,	 Final(TOP_asm_5),
	   22,	 Final(TOP_asm_6),
	   23,	 Final(TOP_asm_7),
	   24,	 Final(TOP_asm_8),
	   25,	 Final(TOP_asm_9),
	   26,	 Final(TOP_asm_10),
	   27,	 Final(TOP_asm_11),
	   28,	 Final(TOP_asm_12),
	   29,	 Final(TOP_asm_13),
	   30,	 Final(TOP_asm_14),
	   31,	 Final(TOP_asm_15),
	   32,	 Final(TOP_asm_16),
	   33,	 Final(TOP_asm_17),
	   34,	 Final(TOP_asm_18),
	   35,	 Final(TOP_asm_19),
	   36,	 Final(TOP_asm_20),
	   37,	 Final(TOP_asm_21),
	   38,	 Final(TOP_asm_22),
	   39,	 Final(TOP_asm_23),
	   40,	 Final(TOP_asm_24),
	   41,	 Final(TOP_asm_25),
	   42,	 Final(TOP_asm_26),
	   43,	 Final(TOP_asm_27),
	   44,	 Final(TOP_asm_28),
	   45,	 Final(TOP_asm_29),
	   46,	 Final(TOP_asm_30),
	   47,	 Final(TOP_asm_31),
	   48,	 Final(TOP_br),
	   49,	 Final(TOP_break),
	   50,	 Final(TOP_brf),
	   51,	 Final(TOP_bswap_r),
	   52,	 Final(TOP_call),
	   53,	 Final(TOP_icall),
	   54,	 Final(TOP_clz_r),
	   55,	 Final(TOP_cmpeq_r_r),
	   56,	 Final(TOP_cmpeq_r_b),
	   57,	 Final(TOP_cmpeq_i_r),
	   58,	 Final(TOP_cmpeq_ii_r),
	   59,	 Final(TOP_cmpeq_i_b),
	   60,	 Final(TOP_cmpeq_ii_b),
	   61,	 Final(TOP_cmpge_r_r),
	   62,	 Final(TOP_cmpge_r_b),
	   63,	 Final(TOP_cmpge_i_r),
	   64,	 Final(TOP_cmpge_ii_r),
	   65,	 Final(TOP_cmpge_i_b),
	   66,	 Final(TOP_cmpge_ii_b),
	   67,	 Final(TOP_cmpgeu_r_r),
	   68,	 Final(TOP_cmpgeu_r_b),
	   69,	 Final(TOP_cmpgeu_i_r),
	   70,	 Final(TOP_cmpgeu_ii_r),
	   71,	 Final(TOP_cmpgeu_i_b),
	   72,	 Final(TOP_cmpgeu_ii_b),
	   73,	 Final(TOP_cmpgt_r_r),
	   74,	 Final(TOP_cmpgt_r_b),
	   75,	 Final(TOP_cmpgt_i_r),
	   76,	 Final(TOP_cmpgt_ii_r),
	   77,	 Final(TOP_cmpgt_i_b),
	   78,	 Final(TOP_cmpgt_ii_b),
	   79,	 Final(TOP_cmpgtu_r_r),
	   80,	 Final(TOP_cmpgtu_r_b),
	   81,	 Final(TOP_cmpgtu_i_r),
	   82,	 Final(TOP_cmpgtu_ii_r),
	   83,	 Final(TOP_cmpgtu_i_b),
	   84,	 Final(TOP_cmpgtu_ii_b),
	   85,	 Final(TOP_cmple_r_r),
	   86,	 Final(TOP_cmple_r_b),
	   87,	 Final(TOP_cmple_i_r),
	   88,	 Final(TOP_cmple_ii_r),
	   89,	 Final(TOP_cmple_i_b),
	   90,	 Final(TOP_cmple_ii_b),
	   91,	 Final(TOP_cmpleu_r_r),
	   92,	 Final(TOP_cmpleu_r_b),
	   93,	 Final(TOP_cmpleu_i_r),
	   94,	 Final(TOP_cmpleu_ii_r),
	   95,	 Final(TOP_cmpleu_i_b),
	   96,	 Final(TOP_cmpleu_ii_b),
	   97,	 Final(TOP_cmplt_r_r),
	   98,	 Final(TOP_cmplt_r_b),
	   99,	 Final(TOP_cmplt_i_r),
	   100,	 Final(TOP_cmplt_ii_r),
	   101,	 Final(TOP_cmplt_i_b),
	   102,	 Final(TOP_cmplt_ii_b),
	   103,	 Final(TOP_cmpltu_r_r),
	   104,	 Final(TOP_cmpltu_r_b),
	   105,	 Final(TOP_cmpltu_i_r),
	   106,	 Final(TOP_cmpltu_ii_r),
	   107,	 Final(TOP_cmpltu_i_b),
	   108,	 Final(TOP_cmpltu_ii_b),
	   109,	 Final(TOP_cmpne_r_r),
	   110,	 Final(TOP_cmpne_r_b),
	   111,	 Final(TOP_cmpne_i_r),
	   112,	 Final(TOP_cmpne_ii_r),
	   113,	 Final(TOP_cmpne_i_b),
	   114,	 Final(TOP_cmpne_ii_b),
	   115,	 Final(TOP_divs),
	   116,	 Final(TOP_goto),
	   117,	 Final(TOP_igoto),
	   118,	 Final(TOP_imml),
	   119,	 Final(TOP_immr),
	   120,	 Final(TOP_ldb_i),
	   121,	 Final(TOP_ldb_ii),
	   122,	 Final(TOP_ldb_d_i),
	   123,	 Final(TOP_ldb_d_ii),
	   124,	 Final(TOP_ldbu_i),
	   125,	 Final(TOP_ldbu_ii),
	   126,	 Final(TOP_ldbu_d_i),
	   127,	 Final(TOP_ldbu_d_ii),
	   128,	 Final(TOP_ldh_i),
	   129,	 Final(TOP_ldh_ii),
	   130,	 Final(TOP_ldh_d_i),
	   131,	 Final(TOP_ldh_d_ii),
	   132,	 Final(TOP_ldhu_i),
	   133,	 Final(TOP_ldhu_ii),
	   134,	 Final(TOP_ldhu_d_i),
	   135,	 Final(TOP_ldhu_d_ii),
	   136,	 Final(TOP_ldw_i),
	   137,	 Final(TOP_ldw_ii),
	   138,	 Final(TOP_ldw_d_i),
	   139,	 Final(TOP_ldw_d_ii),
	   140,	 Final(TOP_max_r),
	   141,	 Final(TOP_max_i),
	   142,	 Final(TOP_max_ii),
	   143,	 Final(TOP_maxu_r),
	   144,	 Final(TOP_maxu_i),
	   145,	 Final(TOP_maxu_ii),
	   146,	 Final(TOP_min_r),
	   147,	 Final(TOP_min_i),
	   148,	 Final(TOP_min_ii),
	   149,	 Final(TOP_minu_r),
	   150,	 Final(TOP_minu_i),
	   151,	 Final(TOP_minu_ii),
	   152,	 Final(TOP_mulh_r),
	   153,	 Final(TOP_mulh_i),
	   154,	 Final(TOP_mulh_ii),
	   155,	 Final(TOP_mulhh_r),
	   156,	 Final(TOP_mulhh_i),
	   157,	 Final(TOP_mulhh_ii),
	   158,	 Final(TOP_mulhhs_r),
	   159,	 Final(TOP_mulhhs_i),
	   160,	 Final(TOP_mulhhs_ii),
	   161,	 Final(TOP_mulhhu_r),
	   162,	 Final(TOP_mulhhu_i),
	   163,	 Final(TOP_mulhhu_ii),
	   164,	 Final(TOP_mulhs_r),
	   165,	 Final(TOP_mulhs_i),
	   166,	 Final(TOP_mulhs_ii),
	   167,	 Final(TOP_mulhu_r),
	   168,	 Final(TOP_mulhu_i),
	   169,	 Final(TOP_mulhu_ii),
	   170,	 Final(TOP_mull_r),
	   171,	 Final(TOP_mull_i),
	   172,	 Final(TOP_mull_ii),
	   173,	 Final(TOP_mullh_r),
	   174,	 Final(TOP_mullh_i),
	   175,	 Final(TOP_mullh_ii),
	   176,	 Final(TOP_mullhu_r),
	   177,	 Final(TOP_mullhu_i),
	   178,	 Final(TOP_mullhu_ii),
	   179,	 Final(TOP_mullhus_r),
	   180,	 Final(TOP_mullhus_i),
	   181,	 Final(TOP_mullhus_ii),
	   182,	 Final(TOP_mulll_r),
	   183,	 Final(TOP_mulll_i),
	   184,	 Final(TOP_mulll_ii),
	   185,	 Final(TOP_mulllu_r),
	   186,	 Final(TOP_mulllu_i),
	   187,	 Final(TOP_mulllu_ii),
	   188,	 Final(TOP_mullu_r),
	   189,	 Final(TOP_mullu_i),
	   190,	 Final(TOP_mullu_ii),
	   191,	 Final(TOP_nandl_r_r),
	   192,	 Final(TOP_nandl_r_b),
	   193,	 Final(TOP_nandl_i_r),
	   194,	 Final(TOP_nandl_ii_r),
	   195,	 Final(TOP_nandl_i_b),
	   196,	 Final(TOP_nandl_ii_b),
	   197,	 Final(TOP_norl_r_r),
	   198,	 Final(TOP_norl_r_b),
	   199,	 Final(TOP_norl_i_r),
	   200,	 Final(TOP_norl_ii_r),
	   201,	 Final(TOP_norl_i_b),
	   202,	 Final(TOP_norl_ii_b),
	   203,	 Final(TOP_or_r),
	   204,	 Final(TOP_or_i),
	   205,	 Final(TOP_or_ii),
	   206,	 Final(TOP_orc_r),
	   207,	 Final(TOP_orc_i),
	   208,	 Final(TOP_orc_ii),
	   209,	 Final(TOP_orl_r_r),
	   210,	 Final(TOP_orl_r_b),
	   211,	 Final(TOP_orl_i_r),
	   212,	 Final(TOP_orl_ii_r),
	   213,	 Final(TOP_orl_i_b),
	   214,	 Final(TOP_orl_ii_b),
	   215,	 Final(TOP_pft_i),
	   216,	 Final(TOP_pft_ii),
	   217,	 Final(TOP_prgadd_i),
	   218,	 Final(TOP_prgadd_ii),
	   219,	 Final(TOP_prgset_i),
	   220,	 Final(TOP_prgset_ii),
	   221,	 Final(TOP_prgins),
	   222,	 Final(TOP_rfi),
	   223,	 Final(TOP_sbrk),
	   224,	 Final(TOP_sh1add_r),
	   225,	 Final(TOP_sh1add_i),
	   226,	 Final(TOP_sh1add_ii),
	   227,	 Final(TOP_sh2add_r),
	   228,	 Final(TOP_sh2add_i),
	   229,	 Final(TOP_sh2add_ii),
	   230,	 Final(TOP_sh3add_r),
	   231,	 Final(TOP_sh3add_i),
	   232,	 Final(TOP_sh3add_ii),
	   233,	 Final(TOP_sh4add_r),
	   234,	 Final(TOP_sh4add_i),
	   235,	 Final(TOP_sh4add_ii),
	   236,	 Final(TOP_shl_r),
	   237,	 Final(TOP_shl_i),
	   238,	 Final(TOP_shl_ii),
	   239,	 Final(TOP_shr_r),
	   240,	 Final(TOP_shr_i),
	   241,	 Final(TOP_shr_ii),
	   242,	 Final(TOP_shru_r),
	   243,	 Final(TOP_shru_i),
	   244,	 Final(TOP_shru_ii),
	   245,	 Final(TOP_slct_r),
	   246,	 Final(TOP_slct_i),
	   247,	 Final(TOP_slct_ii),
	   248,	 Final(TOP_slctf_r),
	   249,	 Final(TOP_slctf_i),
	   250,	 Final(TOP_slctf_ii),
	   251,	 Final(TOP_stb_i),
	   252,	 Final(TOP_stb_ii),
	   253,	 Final(TOP_sth_i),
	   254,	 Final(TOP_sth_ii),
	   255,	 Final(TOP_stw_i),
	   256,	 Final(TOP_stw_ii),
	   257,	 Final(TOP_sub_r),
	   258,	 Final(TOP_sub_i),
	   259,	 Final(TOP_sub_ii),
	   260,	 Final(TOP_sxtb_r),
	   261,	 Final(TOP_sxth_r),
	   262,	 Final(TOP_sync),
	   263,	 Final(TOP_syscall),
	   264,	 Final(TOP_xor_r),
	   265,	 Final(TOP_xor_i),
	   266,	 Final(TOP_xor_ii),
	   267,	 Final(TOP_zxth_r),
	   268,	 Final(TOP_nop),
	   269,	 Final(TOP_mov_r),
	   270,	 Final(TOP_mov_i),
	   271,	 Final(TOP_mov_ii),
	   272,	 Final(TOP_mtb),
	   273,	 Final(TOP_mfb),
	   274,	 Final(TOP_return),
	      END_TRANSITIONS); 

  Initial_State(ex_unit); 

  ISA_Decode_End(); 
  return 0; 
} 
