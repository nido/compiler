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
	      END_TRANSITIONS); 

  Transitions(alu, 
	   0,	 Final(TOP_add_r),
	   1,	 Final(TOP_sub_r),
	   2,	 Final(TOP_shl_r),
	   3,	 Final(TOP_shr_r),
	   4,	 Final(TOP_shru_r),
	   5,	 Final(TOP_sh1add_r),
	   6,	 Final(TOP_sh2add_r),
	   7,	 Final(TOP_sh3add_r),
	   8,	 Final(TOP_sh4add_r),
	   9,	 Final(TOP_and_r),
	   10,	 Final(TOP_andc_r),
	   11,	 Final(TOP_or_r),
	   12,	 Final(TOP_orc_r),
	   13,	 Final(TOP_xor_r),
	   14,	 Final(TOP_max_r),
	   15,	 Final(TOP_maxu_r),
	   16,	 Final(TOP_min_r),
	   17,	 Final(TOP_minu_r),
	   18,	 Final(TOP_bswap_r),
	   19,	 Final(TOP_mull_r),
	   20,	 Final(TOP_mullu_r),
	   21,	 Final(TOP_mulh_r),
	   22,	 Final(TOP_mulhu_r),
	   23,	 Final(TOP_mulll_r),
	   24,	 Final(TOP_mulllu_r),
	   25,	 Final(TOP_mullh_r),
	   26,	 Final(TOP_mullhu_r),
	   27,	 Final(TOP_mulhh_r),
	   28,	 Final(TOP_mulhhu_r),
	   29,	 Final(TOP_mulhs_r),
	   30,	 Final(TOP_cmpeq_r_r),
	   31,	 Final(TOP_cmpne_r_r),
	   32,	 Final(TOP_cmpge_r_r),
	   33,	 Final(TOP_cmpgeu_r_r),
	   34,	 Final(TOP_cmpgt_r_r),
	   35,	 Final(TOP_cmpgtu_r_r),
	   36,	 Final(TOP_cmple_r_r),
	   37,	 Final(TOP_cmpleu_r_r),
	   38,	 Final(TOP_cmplt_r_r),
	   39,	 Final(TOP_cmpltu_r_r),
	   40,	 Final(TOP_andl_r_r),
	   41,	 Final(TOP_nandl_r_r),
	   42,	 Final(TOP_orl_r_r),
	   43,	 Final(TOP_norl_r_r),
	   44,	 Final(TOP_cmpeq_r_b),
	   45,	 Final(TOP_cmpne_r_b),
	   46,	 Final(TOP_cmpge_r_b),
	   47,	 Final(TOP_cmpgeu_r_b),
	   48,	 Final(TOP_cmpgt_r_b),
	   49,	 Final(TOP_cmpgtu_r_b),
	   50,	 Final(TOP_cmple_r_b),
	   51,	 Final(TOP_cmpleu_r_b),
	   52,	 Final(TOP_cmplt_r_b),
	   53,	 Final(TOP_cmpltu_r_b),
	   54,	 Final(TOP_andl_r_b),
	   55,	 Final(TOP_nandl_r_b),
	   56,	 Final(TOP_orl_r_b),
	   57,	 Final(TOP_norl_r_b),
	   58,	 Final(TOP_add_i),
	   59,	 Final(TOP_add_ii),
	   60,	 Final(TOP_sub_i),
	   61,	 Final(TOP_sub_ii),
	   62,	 Final(TOP_shl_i),
	   63,	 Final(TOP_shl_ii),
	   64,	 Final(TOP_shr_i),
	   65,	 Final(TOP_shr_ii),
	   66,	 Final(TOP_shru_i),
	   67,	 Final(TOP_shru_ii),
	   68,	 Final(TOP_sh1add_i),
	   69,	 Final(TOP_sh1add_ii),
	   70,	 Final(TOP_sh2add_i),
	   71,	 Final(TOP_sh2add_ii),
	   72,	 Final(TOP_sh3add_i),
	   73,	 Final(TOP_sh3add_ii),
	   74,	 Final(TOP_sh4add_i),
	   75,	 Final(TOP_sh4add_ii),
	   76,	 Final(TOP_and_i),
	   77,	 Final(TOP_and_ii),
	   78,	 Final(TOP_andc_i),
	   79,	 Final(TOP_andc_ii),
	   80,	 Final(TOP_or_i),
	   81,	 Final(TOP_or_ii),
	   82,	 Final(TOP_orc_i),
	   83,	 Final(TOP_orc_ii),
	   84,	 Final(TOP_xor_i),
	   85,	 Final(TOP_xor_ii),
	   86,	 Final(TOP_max_i),
	   87,	 Final(TOP_max_ii),
	   88,	 Final(TOP_maxu_i),
	   89,	 Final(TOP_maxu_ii),
	   90,	 Final(TOP_min_i),
	   91,	 Final(TOP_min_ii),
	   92,	 Final(TOP_minu_i),
	   93,	 Final(TOP_minu_ii),
	   94,	 Final(TOP_mull_i),
	   95,	 Final(TOP_mull_ii),
	   96,	 Final(TOP_mullu_i),
	   97,	 Final(TOP_mullu_ii),
	   98,	 Final(TOP_mulh_i),
	   99,	 Final(TOP_mulh_ii),
	   100,	 Final(TOP_mulhu_i),
	   101,	 Final(TOP_mulhu_ii),
	   102,	 Final(TOP_mulll_i),
	   103,	 Final(TOP_mulll_ii),
	   104,	 Final(TOP_mulllu_i),
	   105,	 Final(TOP_mulllu_ii),
	   106,	 Final(TOP_mullh_i),
	   107,	 Final(TOP_mullh_ii),
	   108,	 Final(TOP_mullhu_i),
	   109,	 Final(TOP_mullhu_ii),
	   110,	 Final(TOP_mulhh_i),
	   111,	 Final(TOP_mulhh_ii),
	   112,	 Final(TOP_mulhhu_i),
	   113,	 Final(TOP_mulhhu_ii),
	   114,	 Final(TOP_mulhs_i),
	   115,	 Final(TOP_mulhs_ii),
	   116,	 Final(TOP_cmpeq_i_r),
	   117,	 Final(TOP_cmpeq_ii_r),
	   118,	 Final(TOP_cmpne_i_r),
	   119,	 Final(TOP_cmpne_ii_r),
	   120,	 Final(TOP_cmpge_i_r),
	   121,	 Final(TOP_cmpge_ii_r),
	   122,	 Final(TOP_cmpgeu_i_r),
	   123,	 Final(TOP_cmpgeu_ii_r),
	   124,	 Final(TOP_cmpgt_i_r),
	   125,	 Final(TOP_cmpgt_ii_r),
	   126,	 Final(TOP_cmpgtu_i_r),
	   127,	 Final(TOP_cmpgtu_ii_r),
	   128,	 Final(TOP_cmple_i_r),
	   129,	 Final(TOP_cmple_ii_r),
	   130,	 Final(TOP_cmpleu_i_r),
	   131,	 Final(TOP_cmpleu_ii_r),
	   132,	 Final(TOP_cmplt_i_r),
	   133,	 Final(TOP_cmplt_ii_r),
	   134,	 Final(TOP_cmpltu_i_r),
	   135,	 Final(TOP_cmpltu_ii_r),
	   136,	 Final(TOP_andl_i_r),
	   137,	 Final(TOP_andl_ii_r),
	   138,	 Final(TOP_nandl_i_r),
	   139,	 Final(TOP_nandl_ii_r),
	   140,	 Final(TOP_orl_i_r),
	   141,	 Final(TOP_orl_ii_r),
	   142,	 Final(TOP_norl_i_r),
	   143,	 Final(TOP_norl_ii_r),
	   144,	 Final(TOP_cmpeq_i_b),
	   145,	 Final(TOP_cmpeq_ii_b),
	   146,	 Final(TOP_cmpne_i_b),
	   147,	 Final(TOP_cmpne_ii_b),
	   148,	 Final(TOP_cmpge_i_b),
	   149,	 Final(TOP_cmpge_ii_b),
	   150,	 Final(TOP_cmpgeu_i_b),
	   151,	 Final(TOP_cmpgeu_ii_b),
	   152,	 Final(TOP_cmpgt_i_b),
	   153,	 Final(TOP_cmpgt_ii_b),
	   154,	 Final(TOP_cmpgtu_i_b),
	   155,	 Final(TOP_cmpgtu_ii_b),
	   156,	 Final(TOP_cmple_i_b),
	   157,	 Final(TOP_cmple_ii_b),
	   158,	 Final(TOP_cmpleu_i_b),
	   159,	 Final(TOP_cmpleu_ii_b),
	   160,	 Final(TOP_cmplt_i_b),
	   161,	 Final(TOP_cmplt_ii_b),
	   162,	 Final(TOP_cmpltu_i_b),
	   163,	 Final(TOP_cmpltu_ii_b),
	   164,	 Final(TOP_andl_i_b),
	   165,	 Final(TOP_andl_ii_b),
	   166,	 Final(TOP_nandl_i_b),
	   167,	 Final(TOP_nandl_ii_b),
	   168,	 Final(TOP_orl_i_b),
	   169,	 Final(TOP_orl_ii_b),
	   170,	 Final(TOP_norl_i_b),
	   171,	 Final(TOP_norl_ii_b),
	   172,	 Final(TOP_slct_r),
	   173,	 Final(TOP_slctf_r),
	   174,	 Final(TOP_addcg),
	   175,	 Final(TOP_divs),
	   176,	 Final(TOP_imml),
	   177,	 Final(TOP_immr),
	   178,	 Final(TOP_slct_i),
	   179,	 Final(TOP_slct_ii),
	   180,	 Final(TOP_slctf_i),
	   181,	 Final(TOP_slctf_ii),
	   182,	 Final(TOP_prgins),
	   183,	 Final(TOP_sbrk),
	   184,	 Final(TOP_syscall),
	   185,	 Final(TOP_break),
	   186,	 Final(TOP_ldw_i),
	   187,	 Final(TOP_ldw_ii),
	   188,	 Final(TOP_ldw_d_i),
	   189,	 Final(TOP_ldw_d_ii),
	   190,	 Final(TOP_ldh_i),
	   191,	 Final(TOP_ldh_ii),
	   192,	 Final(TOP_ldh_d_i),
	   193,	 Final(TOP_ldh_d_ii),
	   194,	 Final(TOP_ldhu_i),
	   195,	 Final(TOP_ldhu_ii),
	   196,	 Final(TOP_ldhu_d_i),
	   197,	 Final(TOP_ldhu_d_ii),
	   198,	 Final(TOP_ldb_i),
	   199,	 Final(TOP_ldb_ii),
	   200,	 Final(TOP_ldb_d_i),
	   201,	 Final(TOP_ldb_d_ii),
	   202,	 Final(TOP_ldbu_i),
	   203,	 Final(TOP_ldbu_ii),
	   204,	 Final(TOP_ldbu_d_i),
	   205,	 Final(TOP_ldbu_d_ii),
	   206,	 Final(TOP_stw_i),
	   207,	 Final(TOP_stw_ii),
	   208,	 Final(TOP_sth_i),
	   209,	 Final(TOP_sth_ii),
	   210,	 Final(TOP_stb_i),
	   211,	 Final(TOP_stb_ii),
	   212,	 Final(TOP_pft),
	   213,	 Final(TOP_prgadd),
	   214,	 Final(TOP_prgset),
	   215,	 Final(TOP_sync),
	   216,	 Final(TOP_call),
	   217,	 Final(TOP_icall),
	   218,	 Final(TOP_goto),
	   219,	 Final(TOP_igoto),
	   220,	 Final(TOP_rfi),
	   221,	 Final(TOP_br),
	   222,	 Final(TOP_brf),
	   223,	 Final(TOP_sxtb_r),
	   224,	 Final(TOP_sxth_r),
	   225,	 Final(TOP_nop),
	   226,	 Final(TOP_mov_r),
	   227,	 Final(TOP_mov_i),
	   228,	 Final(TOP_mov_ii),
	   229,	 Final(TOP_mtb),
	   230,	 Final(TOP_mfb),
	   231,	 Final(TOP_return),
	      END_TRANSITIONS); 

  Initial_State(ex_unit); 

  ISA_Decode_End(); 
  return 0; 
} 
