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

// AUTOMATICALLY GENERATED FROM CHESS DATA BASE !!! 

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
		 "add_r",
		 "sub_r",
		 "shl_r",
		 "shr_r",
		 "shru_r",
		 "sh1add_r",
		 "sh2add_r",
		 "sh3add_r",
		 "sh4add_r",
		 "and_r",
		 "andc_r",
		 "or_r",
		 "orc_r",
		 "xor_r",
		 "sxtb_r",
		 "sxth_r",
		 "max_r",
		 "maxu_r",
		 "min_r",
		 "minu_r",
		 "bswap_r",
		 "mull_r",
		 "mullu_r",
		 "mulh_r",
		 "mulhu_r",
		 "mulll_r",
		 "mulllu_r",
		 "mullh_r",
		 "mullhu_r",
		 "mulhh_r",
		 "mulhhu_r",
		 "mulhs_r",
		 "cmpeq_r_r",
		 "cmpne_r_r",
		 "cmpge_r_r",
		 "cmpgeu_r_r",
		 "cmpgt_r_r",
		 "cmpgtu_r_r",
		 "cmple_r_r",
		 "cmpleu_r_r",
		 "cmplt_r_r",
		 "cmpltu_r_r",
		 "andl_r_r",
		 "nandl_r_r",
		 "orl_r_r",
		 "norl_r_r",
		 "cmpeq_r_b",
		 "cmpne_r_b",
		 "cmpge_r_b",
		 "cmpgeu_r_b",
		 "cmpgt_r_b",
		 "cmpgtu_r_b",
		 "cmple_r_b",
		 "cmpleu_r_b",
		 "cmplt_r_b",
		 "cmpltu_r_b",
		 "andl_r_b",
		 "nandl_r_b",
		 "orl_r_b",
		 "norl_r_b",
		 "add_i",
		 "sub_i",
		 "shl_i",
		 "shr_i",
		 "shru_i",
		 "sh1add_i",
		 "sh2add_i",
		 "sh3add_i",
		 "sh4add_i",
		 "and_i",
		 "andc_i",
		 "or_i",
		 "orc_i",
		 "xor_i",
		 "max_i",
		 "maxu_i",
		 "min_i",
		 "minu_i",
		 "mull_i",
		 "mullu_i",
		 "mulh_i",
		 "mulhu_i",
		 "mulll_i",
		 "mulllu_i",
		 "mullh_i",
		 "mullhu_i",
		 "mulhh_i",
		 "mulhhu_i",
		 "mulhs_i",
		 "cmpeq_i_r",
		 "cmpne_i_r",
		 "cmpge_i_r",
		 "cmpgeu_i_r",
		 "cmpgt_i_r",
		 "cmpgtu_i_r",
		 "cmple_i_r",
		 "cmpleu_i_r",
		 "cmplt_i_r",
		 "cmpltu_i_r",
		 "andl_i_r",
		 "nandl_i_r",
		 "orl_i_r",
		 "norl_i_r",
		 "cmpeq_i_b",
		 "cmpne_i_b",
		 "cmpge_i_b",
		 "cmpgeu_i_b",
		 "cmpgt_i_b",
		 "cmpgtu_i_b",
		 "cmple_i_b",
		 "cmpleu_i_b",
		 "cmplt_i_b",
		 "cmpltu_i_b",
		 "andl_i_b",
		 "nandl_i_b",
		 "orl_i_b",
		 "norl_i_b",
		 "slct_r",
		 "slctf_r",
		 "addcg",
		 "divs",
		 "imml",
		 "immr",
		 "slct_i",
		 "slctf_i",
		 "prgins",
		 "sbrk",
		 "syscall",
		 "break",
		 "ldw",
		 "ldw_d",
		 "ldh",
		 "ldh_d",
		 "ldhu",
		 "ldhu_d",
		 "ldb",
		 "ldb_d",
		 "ldbu",
		 "ldbu_d",
		 "stw",
		 "sth",
		 "stb",
		 "pft",
		 "prgadd",
		 "prgset",
		 "sync",
		 "send_i",
		 "recv_i",
		 "call",
		 "icall",
		 "goto",
		 "igoto",
		 "rfi",
		 "br",
		 "brf",
		 "nop",
		 "mov_r",
		 "mov_i",
		 "mtb",
		 "mfb",
		 "return",
		 "asm",	 // simulated
		 "intrncall",	 // simulated
		 "spadjust",	 // simulated
		 "copy_br",	 // simulated
		 "noop",	 // simulated
		 "begin_pregtn",	 // dummy
		 "end_pregtn",	 // dummy
		 "bwd_bar",	 // dummy
		 "fwd_bar",	 // dummy
		 "dfixup",	 // dummy
		 "ffixup",	 // dummy
		 "ifixup",	 // dummy
		 "label",	 // dummy
      NULL);
}
