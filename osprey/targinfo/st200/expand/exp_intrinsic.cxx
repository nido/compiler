/*
  Copyright (C) 2002-2009, STMicroelectronics, All Rights Reserved.

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
*/


#include "defs.h"
#include "config.h"
#include "erglob.h"
#include "ercg.h"
#include "glob.h"
#include "tracing.h"
#include "util.h"

#include "symtab.h"
#include "opcode.h"
#include "intrn_info.h"
#include "const.h" /* needed to manipulate target/host consts */
#include "targ_const.h" /* needed to manipulate target/host consts */
#include "cgir.h"
#include "cgexp.h"

#include "topcode.h"
#include "targ_isa_lits.h"
#include "targ_isa_properties.h"
#include "config_TARG.h"
#include "data_layout.h"

static void
Expand_Unimplemented_Intrinsic (TOP opcode,
				OPS *ops)
{
  Build_OP (	TOP_break,  ops) ;
}

/*
 * Expansion of builtin__divh based on validated and scheduled basic assembly source.
*/
static void
Expand__builtin__divh(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{  
  if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_div_r_r_r)) {
    Build_OP (    TOP_div_r_r_r,        o0,     i0,     i1,     ops) ;
  } else {
    TN *b0_0_10 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_0_11 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_0_12 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_0_13 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_0_14 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_0_15 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_0_16 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_0_17 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_0_18 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_0_19 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_0_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_0_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_0_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_0_6 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_0_7 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_0_8 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_0_9 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_1_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_2_10 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_2_11 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_2_12 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_2_13 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_2_14 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_2_15 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_2_16 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_2_17 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_2_18 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_2_19 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_2_20 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_2_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_2_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_2_6 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_2_7 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_2_8 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_2_9 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_3_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *r0_15_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_15_10 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_15_12 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_15_14 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_15_16 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_15_18 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_15_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_15_20 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_15_21 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_15_22 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_15_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_15_6 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_15_8 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_16_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_16_24 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_17_11 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_17_13 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_17_15 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_17_17 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_17_19 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_17_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_17_7 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_17_9 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_18_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_19_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_19_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_20_10 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_20_11 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_20_12 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_20_13 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_20_14 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_20_15 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_20_16 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_20_17 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_20_18 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_20_19 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_20_20 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_20_23 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_20_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_20_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_20_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_20_6 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_20_7 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_20_8 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_20_9 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
    TN *c0x8000 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *c0x8000_ltn = Gen_Literal_TN(__EXTS32TOS64(0x8000), 4) ;
    Build_OP (    TOP_mov_ii_r,      c0x8000,        c0x8000_ltn,    ops) ;
    TN *c16 = Gen_Literal_TN(__EXTS32TOS64(16), 4) ;
#undef __EXTS32TOS64
    Build_OP (	TOP_shl_i_r_r,	r0_16_0,	i0,	c16,	ops) ;
    Build_OP (	TOP_cmplt_r_r_r,	r0_20_0,	i0,	Zero_TN,	ops) ;
    Build_OP (	TOP_cmplt_r_r_r,	r0_18_0,	i1,	Zero_TN,	ops) ;
    Build_OP (	TOP_convib_r_b,	b0_1_0,	Zero_TN,	ops) ;
    Build_OP (	TOP_sub_r_r_r,	r0_19_1,	Zero_TN,	i1,	ops) ;
    Build_OP (	TOP_sub_r_r_r,	r0_15_1,	Zero_TN,	r0_16_0,	ops) ;
    Build_OP (	TOP_max_r_r_r,	r0_15_2,	r0_15_1,	r0_16_0,	ops) ;
    Build_OP (	TOP_max_r_r_r,	r0_19_2,	r0_19_1,	i1,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_20_3,	b0_0_3,	r0_15_2,	r0_15_2,	b0_1_0,	ops) ;
    Build_OP (	TOP_cmpeq_r_r_b,	b0_3_3,	r0_18_0,	r0_20_0,	ops) ;
    Build_OP (	TOP_divs_b_r_r_b_r,	r0_20_4,	b0_0_4,	Zero_TN,	r0_19_2,	b0_0_3,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_15_4,	b0_2_4,	r0_20_3,	r0_20_3,	b0_1_0,	ops) ;
    Build_OP (	TOP_divs_b_r_r_b_r,	r0_20_5,	b0_2_5,	r0_20_4,	r0_19_2,	b0_2_4,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_5,	b0_0_5,	r0_15_4,	r0_15_4,	b0_0_4,	ops) ;
    Build_OP (	TOP_divs_b_r_r_b_r,	r0_20_6,	b0_0_6,	r0_20_5,	r0_19_2,	b0_0_5,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_15_6,	b0_2_6,	r0_17_5,	r0_17_5,	b0_2_5,	ops) ;
    Build_OP (	TOP_divs_b_r_r_b_r,	r0_20_7,	b0_2_7,	r0_20_6,	r0_19_2,	b0_2_6,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_7,	b0_0_7,	r0_15_6,	r0_15_6,	b0_0_6,	ops) ;
    Build_OP (	TOP_divs_b_r_r_b_r,	r0_20_8,	b0_0_8,	r0_20_7,	r0_19_2,	b0_0_7,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_15_8,	b0_2_8,	r0_17_7,	r0_17_7,	b0_2_7,	ops) ;
    Build_OP (	TOP_divs_b_r_r_b_r,	r0_20_9,	b0_2_9,	r0_20_8,	r0_19_2,	b0_2_8,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_9,	b0_0_9,	r0_15_8,	r0_15_8,	b0_0_8,	ops) ;
    Build_OP (	TOP_divs_b_r_r_b_r,	r0_20_10,	b0_0_10,	r0_20_9,	r0_19_2,	b0_0_9,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_15_10,	b0_2_10,	r0_17_9,	r0_17_9,	b0_2_9,	ops) ;
    Build_OP (	TOP_divs_b_r_r_b_r,	r0_20_11,	b0_2_11,	r0_20_10,	r0_19_2,	b0_2_10,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_11,	b0_0_11,	r0_15_10,	r0_15_10,	b0_0_10,	ops) ;
    Build_OP (	TOP_divs_b_r_r_b_r,	r0_20_12,	b0_0_12,	r0_20_11,	r0_19_2,	b0_0_11,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_15_12,	b0_2_12,	r0_17_11,	r0_17_11,	b0_2_11,	ops) ;
    Build_OP (	TOP_divs_b_r_r_b_r,	r0_20_13,	b0_2_13,	r0_20_12,	r0_19_2,	b0_2_12,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_13,	b0_0_13,	r0_15_12,	r0_15_12,	b0_0_12,	ops) ;
    Build_OP (	TOP_divs_b_r_r_b_r,	r0_20_14,	b0_0_14,	r0_20_13,	r0_19_2,	b0_0_13,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_15_14,	b0_2_14,	r0_17_13,	r0_17_13,	b0_2_13,	ops) ;
    Build_OP (	TOP_divs_b_r_r_b_r,	r0_20_15,	b0_2_15,	r0_20_14,	r0_19_2,	b0_2_14,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_15,	b0_0_15,	r0_15_14,	r0_15_14,	b0_0_14,	ops) ;
    Build_OP (	TOP_divs_b_r_r_b_r,	r0_20_16,	b0_0_16,	r0_20_15,	r0_19_2,	b0_0_15,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_15_16,	b0_2_16,	r0_17_15,	r0_17_15,	b0_2_15,	ops) ;
    Build_OP (	TOP_divs_b_r_r_b_r,	r0_20_17,	b0_2_17,	r0_20_16,	r0_19_2,	b0_2_16,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_17,	b0_0_17,	r0_15_16,	r0_15_16,	b0_0_16,	ops) ;
    Build_OP (	TOP_divs_b_r_r_b_r,	r0_20_18,	b0_0_18,	r0_20_17,	r0_19_2,	b0_0_17,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_15_18,	b0_2_18,	r0_17_17,	r0_17_17,	b0_2_17,	ops) ;
    Build_OP (	TOP_divs_b_r_r_b_r,	r0_20_19,	b0_2_19,	r0_20_18,	r0_19_2,	b0_2_18,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_19,	b0_0_19,	r0_15_18,	r0_15_18,	b0_0_18,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_15_20,	b0_2_20,	r0_17_19,	r0_17_19,	b0_2_19,	ops) ;
    Build_OP (	TOP_cmpltu_r_r_r,	r0_20_20,	r0_20_19,	c0x8000,	ops) ;
    Build_OP (	TOP_orc_r_r_r,	r0_15_21,	r0_15_20,	Zero_TN,	ops) ;
    Build_OP (	TOP_sh1add_r_r_r,	r0_15_22,	r0_15_21,	r0_20_20,	ops) ;
    Build_OP (	TOP_sub_r_r_r,	r0_20_23,	Zero_TN,	r0_15_22,	ops) ;
    Build_OP (	TOP_targ_slct_r_r_b_r,	r0_16_24,	b0_3_3,	r0_15_22,	r0_20_23,	ops) ;
    Build_OP (	TOP_sxth_r_r,	o0,	r0_16_24,	ops) ;
  }
} /* Expand__builtin__divh */

/*
@@@  case INTRN_BUILTIN__DIVH:
@@@    Expand__builtin__divh(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of builtin__divuh based on validated and scheduled basic assembly source.
*/
static void
Expand__builtin__divuh(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_divu_r_r_r)) {
    Build_OP (    TOP_divu_r_r_r,        o0,     i0,     i1,     ops) ;
  } else {
    TN *b0_0_10 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_0_11 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_0_12 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_0_13 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_0_14 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_0_15 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_0_16 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_0_17 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_0_18 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_0_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_0_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_0_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_0_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_0_6 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_0_7 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_0_8 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_0_9 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_2_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_2_10 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_2_11 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_2_12 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_2_13 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_2_14 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_2_15 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_2_16 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_2_17 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_2_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_2_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_2_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_2_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_2_6 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_2_7 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_2_8 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_2_9 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_3_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *r0_11_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_11_10 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_11_11 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_11_12 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_11_13 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_11_14 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_11_15 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_11_16 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_11_17 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_11_18 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_11_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_11_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_11_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_11_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_11_6 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_11_7 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_11_8 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_11_9 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_15_10 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_15_12 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_15_14 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_15_16 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_15_18 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_15_19 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_15_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_15_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_15_6 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_15_8 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_16_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_16_20 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_18_11 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_18_13 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_18_15 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_18_17 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_18_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_18_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_18_7 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_18_9 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
    TN *c0x8000 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *c0x8000_ltn = Gen_Literal_TN(__EXTS32TOS64(0x8000), 4) ;
    Build_OP (    TOP_mov_ii_r,      c0x8000,        c0x8000_ltn,    ops) ;
    TN *c16 = Gen_Literal_TN(__EXTS32TOS64(16), 4) ;
#undef __EXTS32TOS64
    Build_OP (	TOP_shl_i_r_r,	r0_16_0,	i0,	c16,	ops) ;
    Build_OP (	TOP_convib_r_b,	b0_3_0,	Zero_TN,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_11_1,	b0_2_1,	r0_16_0,	r0_16_0,	b0_3_0,	ops) ;
    Build_OP (	TOP_divs_b_r_r_b_r,	r0_11_2,	b0_2_2,	Zero_TN,	i1,	b0_2_1,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_15_2,	b0_0_2,	r0_11_1,	r0_11_1,	b0_3_0,	ops) ;
    Build_OP (	TOP_divs_b_r_r_b_r,	r0_11_3,	b0_0_3,	r0_11_2,	i1,	b0_0_2,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_18_3,	b0_2_3,	r0_15_2,	r0_15_2,	b0_2_2,	ops) ;
    Build_OP (	TOP_divs_b_r_r_b_r,	r0_11_4,	b0_2_4,	r0_11_3,	i1,	b0_2_3,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_15_4,	b0_0_4,	r0_18_3,	r0_18_3,	b0_0_3,	ops) ;
    Build_OP (	TOP_divs_b_r_r_b_r,	r0_11_5,	b0_0_5,	r0_11_4,	i1,	b0_0_4,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_18_5,	b0_2_5,	r0_15_4,	r0_15_4,	b0_2_4,	ops) ;
    Build_OP (	TOP_divs_b_r_r_b_r,	r0_11_6,	b0_2_6,	r0_11_5,	i1,	b0_2_5,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_15_6,	b0_0_6,	r0_18_5,	r0_18_5,	b0_0_5,	ops) ;
    Build_OP (	TOP_divs_b_r_r_b_r,	r0_11_7,	b0_0_7,	r0_11_6,	i1,	b0_0_6,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_18_7,	b0_2_7,	r0_15_6,	r0_15_6,	b0_2_6,	ops) ;
    Build_OP (	TOP_divs_b_r_r_b_r,	r0_11_8,	b0_2_8,	r0_11_7,	i1,	b0_2_7,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_15_8,	b0_0_8,	r0_18_7,	r0_18_7,	b0_0_7,	ops) ;
    Build_OP (	TOP_divs_b_r_r_b_r,	r0_11_9,	b0_0_9,	r0_11_8,	i1,	b0_0_8,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_18_9,	b0_2_9,	r0_15_8,	r0_15_8,	b0_2_8,	ops) ;
    Build_OP (	TOP_divs_b_r_r_b_r,	r0_11_10,	b0_2_10,	r0_11_9,	i1,	b0_2_9,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_15_10,	b0_0_10,	r0_18_9,	r0_18_9,	b0_0_9,	ops) ;
    Build_OP (	TOP_divs_b_r_r_b_r,	r0_11_11,	b0_0_11,	r0_11_10,	i1,	b0_0_10,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_18_11,	b0_2_11,	r0_15_10,	r0_15_10,	b0_2_10,	ops) ;
    Build_OP (	TOP_divs_b_r_r_b_r,	r0_11_12,	b0_2_12,	r0_11_11,	i1,	b0_2_11,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_15_12,	b0_0_12,	r0_18_11,	r0_18_11,	b0_0_11,	ops) ;
    Build_OP (	TOP_divs_b_r_r_b_r,	r0_11_13,	b0_0_13,	r0_11_12,	i1,	b0_0_12,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_18_13,	b0_2_13,	r0_15_12,	r0_15_12,	b0_2_12,	ops) ;
    Build_OP (	TOP_divs_b_r_r_b_r,	r0_11_14,	b0_2_14,	r0_11_13,	i1,	b0_2_13,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_15_14,	b0_0_14,	r0_18_13,	r0_18_13,	b0_0_13,	ops) ;
    Build_OP (	TOP_divs_b_r_r_b_r,	r0_11_15,	b0_0_15,	r0_11_14,	i1,	b0_0_14,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_18_15,	b0_2_15,	r0_15_14,	r0_15_14,	b0_2_14,	ops) ;
    Build_OP (	TOP_divs_b_r_r_b_r,	r0_11_16,	b0_2_16,	r0_11_15,	i1,	b0_2_15,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_15_16,	b0_0_16,	r0_18_15,	r0_18_15,	b0_0_15,	ops) ;
    Build_OP (	TOP_divs_b_r_r_b_r,	r0_11_17,	b0_0_17,	r0_11_16,	i1,	b0_0_16,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_18_17,	b0_2_17,	r0_15_16,	r0_15_16,	b0_2_16,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_15_18,	b0_0_18,	r0_18_17,	r0_18_17,	b0_0_17,	ops) ;
    Build_OP (	TOP_cmpltu_r_r_r,	r0_11_18,	r0_11_17,	c0x8000,	ops) ;
    Build_OP (	TOP_orc_r_r_r,	r0_15_19,	r0_15_18,	Zero_TN,	ops) ;
    Build_OP (	TOP_sh1add_r_r_r,	r0_16_20,	r0_15_19,	r0_11_18,	ops) ;
    Build_OP (	TOP_zxth_r_r,	o0,	r0_16_20,	ops) ;
  }
} /* Expand__builtin__divuh */

/*
@@@  case INTRN_BUILTIN__DIVUH:
@@@    Expand__builtin__divuh(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of builtin__modh based on validated and scheduled basic assembly source.
*/
static void
Expand__builtin__modh(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_rem_r_r_r)) {
    Build_OP (    TOP_rem_r_r_r,        o0,     i0,     i1,     ops) ;
  } else {
    TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_1_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_1_20 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_2_10 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_2_11 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_2_12 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_2_13 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_2_14 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_2_15 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_2_16 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_2_17 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_2_18 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_2_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_2_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_2_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_2_6 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_2_7 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_2_8 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_2_9 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_3_10 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_3_11 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_3_12 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_3_13 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_3_14 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_3_15 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_3_16 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_3_17 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_3_18 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_3_19 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_3_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_3_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_3_6 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_3_7 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_3_8 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_3_9 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *r0_15_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_15_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_15_20 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_15_22 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_16_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_16_23 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_17_11 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_17_13 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_17_15 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_17_17 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_17_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_17_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_17_7 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_17_9 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_18_10 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_18_12 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_18_14 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_18_16 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_18_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_18_6 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_18_8 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_20_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_20_10 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_20_11 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_20_12 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_20_13 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_20_14 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_20_15 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_20_16 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_20_17 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_20_18 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_20_19 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_20_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_20_21 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_20_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_20_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_20_6 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_20_7 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_20_8 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_20_9 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_25_18 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
    TN *c16 = Gen_Literal_TN(__EXTS32TOS64(16), 4) ;
#undef __EXTS32TOS64
    Build_OP (	TOP_shl_i_r_r,	r0_16_0,	i0,	c16,	ops) ;
    Build_OP (	TOP_cmplt_r_r_b,	b0_0_0,	i0,	Zero_TN,	ops) ;
    Build_OP (	TOP_convib_r_b,	b0_1_0,	Zero_TN,	ops) ;
    Build_OP (	TOP_sub_r_r_r,	r0_20_1,	Zero_TN,	r0_16_0,	ops) ;
    Build_OP (	TOP_sub_r_r_r,	r0_15_1,	Zero_TN,	i1,	ops) ;
    Build_OP (	TOP_max_r_r_r,	r0_20_2,	r0_20_1,	r0_16_0,	ops) ;
    Build_OP (	TOP_max_r_r_r,	r0_15_2,	r0_15_1,	i1,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_3,	b0_2_3,	r0_20_2,	r0_20_2,	b0_1_0,	ops) ;
    Build_OP (	TOP_divs_b_r_r_b_r,	r0_20_4,	b0_2_4,	Zero_TN,	r0_15_2,	b0_2_3,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_18_4,	b0_3_4,	r0_17_3,	r0_17_3,	b0_1_0,	ops) ;
    Build_OP (	TOP_divs_b_r_r_b_r,	r0_20_5,	b0_3_5,	r0_20_4,	r0_15_2,	b0_3_4,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_5,	b0_2_5,	r0_18_4,	r0_18_4,	b0_2_4,	ops) ;
    Build_OP (	TOP_divs_b_r_r_b_r,	r0_20_6,	b0_2_6,	r0_20_5,	r0_15_2,	b0_2_5,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_18_6,	b0_3_6,	r0_17_5,	r0_17_5,	b0_3_5,	ops) ;
    Build_OP (	TOP_divs_b_r_r_b_r,	r0_20_7,	b0_3_7,	r0_20_6,	r0_15_2,	b0_3_6,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_7,	b0_2_7,	r0_18_6,	r0_18_6,	b0_2_6,	ops) ;
    Build_OP (	TOP_divs_b_r_r_b_r,	r0_20_8,	b0_2_8,	r0_20_7,	r0_15_2,	b0_2_7,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_18_8,	b0_3_8,	r0_17_7,	r0_17_7,	b0_3_7,	ops) ;
    Build_OP (	TOP_divs_b_r_r_b_r,	r0_20_9,	b0_3_9,	r0_20_8,	r0_15_2,	b0_3_8,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_9,	b0_2_9,	r0_18_8,	r0_18_8,	b0_2_8,	ops) ;
    Build_OP (	TOP_divs_b_r_r_b_r,	r0_20_10,	b0_2_10,	r0_20_9,	r0_15_2,	b0_2_9,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_18_10,	b0_3_10,	r0_17_9,	r0_17_9,	b0_3_9,	ops) ;
    Build_OP (	TOP_divs_b_r_r_b_r,	r0_20_11,	b0_3_11,	r0_20_10,	r0_15_2,	b0_3_10,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_11,	b0_2_11,	r0_18_10,	r0_18_10,	b0_2_10,	ops) ;
    Build_OP (	TOP_divs_b_r_r_b_r,	r0_20_12,	b0_2_12,	r0_20_11,	r0_15_2,	b0_2_11,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_18_12,	b0_3_12,	r0_17_11,	r0_17_11,	b0_3_11,	ops) ;
    Build_OP (	TOP_divs_b_r_r_b_r,	r0_20_13,	b0_3_13,	r0_20_12,	r0_15_2,	b0_3_12,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_13,	b0_2_13,	r0_18_12,	r0_18_12,	b0_2_12,	ops) ;
    Build_OP (	TOP_divs_b_r_r_b_r,	r0_20_14,	b0_2_14,	r0_20_13,	r0_15_2,	b0_2_13,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_18_14,	b0_3_14,	r0_17_13,	r0_17_13,	b0_3_13,	ops) ;
    Build_OP (	TOP_divs_b_r_r_b_r,	r0_20_15,	b0_3_15,	r0_20_14,	r0_15_2,	b0_3_14,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_15,	b0_2_15,	r0_18_14,	r0_18_14,	b0_2_14,	ops) ;
    Build_OP (	TOP_divs_b_r_r_b_r,	r0_20_16,	b0_2_16,	r0_20_15,	r0_15_2,	b0_2_15,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_18_16,	b0_3_16,	r0_17_15,	r0_17_15,	b0_3_15,	ops) ;
    Build_OP (	TOP_divs_b_r_r_b_r,	r0_20_17,	b0_3_17,	r0_20_16,	r0_15_2,	b0_3_16,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_17,	b0_2_17,	r0_18_16,	r0_18_16,	b0_2_16,	ops) ;
    Build_OP (	TOP_divs_b_r_r_b_r,	r0_20_18,	b0_2_18,	r0_20_17,	r0_15_2,	b0_2_17,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_25_18,	b0_3_18,	r0_17_17,	r0_17_17,	b0_3_17,	ops) ;
    Build_OP (	TOP_divs_b_r_r_b_r,	r0_20_19,	b0_3_19,	r0_20_18,	r0_15_2,	b0_3_18,	ops) ;
    Build_OP (	TOP_cmpge_r_r_b,	b0_1_20,	r0_20_19,	Zero_TN,	ops) ;
    Build_OP (	TOP_add_r_r_r,	r0_15_20,	r0_20_19,	r0_15_2,	ops) ;
    Build_OP (	TOP_targ_slct_r_r_b_r,	r0_20_21,	b0_1_20,	r0_20_19,	r0_15_20,	ops) ;
    Build_OP (	TOP_sub_r_r_r,	r0_15_22,	Zero_TN,	r0_20_21,	ops) ;
    Build_OP (	TOP_targ_slct_r_r_b_r,	r0_16_23,	b0_0_0,	r0_15_22,	r0_20_21,	ops) ;
    Build_OP (	TOP_sxth_r_r,	o0,	r0_16_23,	ops) ;
  }
} /* Expand__builtin__modh */

/*
@@@  case INTRN_BUILTIN__MODH:
@@@    Expand__builtin__modh(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of builtin__moduh based on validated and scheduled basic assembly source.
*/
static void
Expand__builtin__moduh(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_remu_r_r_r)) {
    Build_OP (    TOP_remu_r_r_r,        o0,     i0,     i1,     ops) ;
  } else {
    TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_0_18 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_2_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_2_10 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_2_11 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_2_12 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_2_13 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_2_14 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_2_15 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_2_16 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_2_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_2_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_2_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_2_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_2_6 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_2_7 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_2_8 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_2_9 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_3_10 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_3_11 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_3_12 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_3_13 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_3_14 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_3_15 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_3_16 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_3_17 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_3_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_3_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_3_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_3_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_3_6 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_3_7 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_3_8 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_3_9 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *r0_16_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_16_10 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_16_11 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_16_12 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_16_13 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_16_14 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_16_15 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_16_16 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_16_17 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_16_19 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_16_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_16_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_16_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_16_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_16_6 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_16_7 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_16_8 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_16_9 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_17_18 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_19_10 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_19_12 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_19_14 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_19_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_19_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_19_6 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_19_8 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_20_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_20_11 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_20_13 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_20_15 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_20_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_20_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_20_7 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_20_9 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_25_16 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
    TN *c16 = Gen_Literal_TN(__EXTS32TOS64(16), 4) ;
#undef __EXTS32TOS64
    Build_OP (	TOP_convib_r_b,	b0_0_0,	Zero_TN,	ops) ;
    Build_OP (	TOP_shl_i_r_r,	r0_16_0,	i0,	c16,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_20_1,	b0_2_1,	r0_16_0,	r0_16_0,	b0_0_0,	ops) ;
    Build_OP (	TOP_divs_b_r_r_b_r,	r0_16_2,	b0_2_2,	Zero_TN,	i1,	b0_2_1,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_19_2,	b0_3_2,	r0_20_1,	r0_20_1,	b0_0_0,	ops) ;
    Build_OP (	TOP_divs_b_r_r_b_r,	r0_16_3,	b0_3_3,	r0_16_2,	i1,	b0_3_2,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_20_3,	b0_2_3,	r0_19_2,	r0_19_2,	b0_2_2,	ops) ;
    Build_OP (	TOP_divs_b_r_r_b_r,	r0_16_4,	b0_2_4,	r0_16_3,	i1,	b0_2_3,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_19_4,	b0_3_4,	r0_20_3,	r0_20_3,	b0_3_3,	ops) ;
    Build_OP (	TOP_divs_b_r_r_b_r,	r0_16_5,	b0_3_5,	r0_16_4,	i1,	b0_3_4,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_20_5,	b0_2_5,	r0_19_4,	r0_19_4,	b0_2_4,	ops) ;
    Build_OP (	TOP_divs_b_r_r_b_r,	r0_16_6,	b0_2_6,	r0_16_5,	i1,	b0_2_5,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_19_6,	b0_3_6,	r0_20_5,	r0_20_5,	b0_3_5,	ops) ;
    Build_OP (	TOP_divs_b_r_r_b_r,	r0_16_7,	b0_3_7,	r0_16_6,	i1,	b0_3_6,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_20_7,	b0_2_7,	r0_19_6,	r0_19_6,	b0_2_6,	ops) ;
    Build_OP (	TOP_divs_b_r_r_b_r,	r0_16_8,	b0_2_8,	r0_16_7,	i1,	b0_2_7,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_19_8,	b0_3_8,	r0_20_7,	r0_20_7,	b0_3_7,	ops) ;
    Build_OP (	TOP_divs_b_r_r_b_r,	r0_16_9,	b0_3_9,	r0_16_8,	i1,	b0_3_8,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_20_9,	b0_2_9,	r0_19_8,	r0_19_8,	b0_2_8,	ops) ;
    Build_OP (	TOP_divs_b_r_r_b_r,	r0_16_10,	b0_2_10,	r0_16_9,	i1,	b0_2_9,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_19_10,	b0_3_10,	r0_20_9,	r0_20_9,	b0_3_9,	ops) ;
    Build_OP (	TOP_divs_b_r_r_b_r,	r0_16_11,	b0_3_11,	r0_16_10,	i1,	b0_3_10,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_20_11,	b0_2_11,	r0_19_10,	r0_19_10,	b0_2_10,	ops) ;
    Build_OP (	TOP_divs_b_r_r_b_r,	r0_16_12,	b0_2_12,	r0_16_11,	i1,	b0_2_11,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_19_12,	b0_3_12,	r0_20_11,	r0_20_11,	b0_3_11,	ops) ;
    Build_OP (	TOP_divs_b_r_r_b_r,	r0_16_13,	b0_3_13,	r0_16_12,	i1,	b0_3_12,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_20_13,	b0_2_13,	r0_19_12,	r0_19_12,	b0_2_12,	ops) ;
    Build_OP (	TOP_divs_b_r_r_b_r,	r0_16_14,	b0_2_14,	r0_16_13,	i1,	b0_2_13,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_19_14,	b0_3_14,	r0_20_13,	r0_20_13,	b0_3_13,	ops) ;
    Build_OP (	TOP_divs_b_r_r_b_r,	r0_16_15,	b0_3_15,	r0_16_14,	i1,	b0_3_14,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_20_15,	b0_2_15,	r0_19_14,	r0_19_14,	b0_2_14,	ops) ;
    Build_OP (	TOP_divs_b_r_r_b_r,	r0_16_16,	b0_2_16,	r0_16_15,	i1,	b0_2_15,	ops) ;
    Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_25_16,	b0_3_16,	r0_20_15,	r0_20_15,	b0_3_15,	ops) ;
    Build_OP (	TOP_divs_b_r_r_b_r,	r0_16_17,	b0_3_17,	r0_16_16,	i1,	b0_3_16,	ops) ;
    Build_OP (	TOP_cmpge_r_r_b,	b0_0_18,	r0_16_17,	Zero_TN,	ops) ;
    Build_OP (	TOP_add_r_r_r,	r0_17_18,	r0_16_17,	i1,	ops) ;
    Build_OP (	TOP_targ_slct_r_r_b_r,	r0_16_19,	b0_0_18,	r0_16_17,	r0_17_18,	ops) ;
    Build_OP (	TOP_zxth_r_r,	o0,	r0_16_19,	ops) ;
  }
} /* Expand__builtin__moduh */

/*
@@@  case INTRN_BUILTIN__MODUH:
@@@    Expand__builtin__moduh(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/


/*
 * Expansion of builtin__divuw based on validated and scheduled basic assembly source.
*/
static void
Expand__builtin__divuw(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_divu_r_r_r)) {
      Build_OP (	TOP_divu_r_r_r,	o0,	i0,	i1,	ops) ;
  } else {
      TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_1_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_10 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_11 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_12 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_13 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_14 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_15 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_16 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_17 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_18 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_19 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_20 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_21 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_22 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_23 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_24 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_25 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_26 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_27 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_28 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_29 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_30 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_31 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_32 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_33 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_34 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_35 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_6 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_7 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_8 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_9 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_10 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_11 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_12 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_13 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_14 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_15 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_16 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_17 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_18 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_19 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_20 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_21 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_22 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_23 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_24 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_25 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_26 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_27 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_28 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_29 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_30 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_31 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_32 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_33 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_34 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_35 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_36 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_6 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_7 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_8 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_9 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *r0_14_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_10 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_11 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_12 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_13 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_14 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_15 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_16 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_17 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_18 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_19 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_20 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_21 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_22 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_23 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_24 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_25 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_26 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_27 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_28 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_29 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_30 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_31 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_32 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_33 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_34 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_35 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_36 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_6 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_7 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_8 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_9 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_10 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_12 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_14 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_16 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_18 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_20 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_22 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_24 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_26 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_28 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_30 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_32 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_34 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_36 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_37 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_38 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_6 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_8 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_16_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_17_11 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_17_13 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_17_15 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_17_17 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_17_19 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_17_21 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_17_23 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_17_25 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_17_27 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_17_29 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_17_31 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_17_33 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_17_35 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_17_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_17_7 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_17_9 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_18_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      Build_OP (	TOP_cmplt_r_r_r,	r0_14_0,	i1,	Zero_TN,	ops) ;
      Build_OP (	TOP_convib_r_b,	b0_0_0,	Zero_TN,	ops) ;
      Build_OP (	TOP_shru_r_r_r,	r0_18_1,	i1,	r0_14_0,	ops) ;
      Build_OP (	TOP_shru_r_r_r,	r0_15_1,	i0,	r0_14_0,	ops) ;
      Build_OP (	TOP_cmpgeu_r_r_r,	r0_16_1,	i0,	i1,	ops) ;
      Build_OP (	TOP_cmplt_r_r_b,	b0_1_1,		i1,	Zero_TN,	ops) ;
      Build_OP (	TOP_addcg_b_r_r_b_r,	r0_14_3,	b0_2_3,	r0_15_1,	r0_15_1,	b0_0_0,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_4,	b0_2_4,	Zero_TN,	r0_18_1,	b0_2_3,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_15_4,	b0_3_4,	r0_14_3,	r0_14_3,	b0_0_0,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_5,	b0_3_5,	r0_14_4,	r0_18_1,	b0_3_4,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_5,	b0_2_5,	r0_15_4,	r0_15_4,	b0_2_4,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_6,	b0_2_6,	r0_14_5,	r0_18_1,	b0_2_5,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_15_6,	b0_3_6,	r0_17_5,	r0_17_5,	b0_3_5,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_7,	b0_3_7,	r0_14_6,	r0_18_1,	b0_3_6,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_7,	b0_2_7,	r0_15_6,	r0_15_6,	b0_2_6,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_8,	b0_2_8,	r0_14_7,	r0_18_1,	b0_2_7,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_15_8,	b0_3_8,	r0_17_7,	r0_17_7,	b0_3_7,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_9,	b0_3_9,	r0_14_8,	r0_18_1,	b0_3_8,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_9,	b0_2_9,	r0_15_8,	r0_15_8,	b0_2_8,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_10,	b0_2_10,	r0_14_9,	r0_18_1,	b0_2_9,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_15_10,	b0_3_10,	r0_17_9,	r0_17_9,	b0_3_9,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_11,	b0_3_11,	r0_14_10,	r0_18_1,	b0_3_10,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_11,	b0_2_11,	r0_15_10,	r0_15_10,	b0_2_10,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_12,	b0_2_12,	r0_14_11,	r0_18_1,	b0_2_11,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_15_12,	b0_3_12,	r0_17_11,	r0_17_11,	b0_3_11,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_13,	b0_3_13,	r0_14_12,	r0_18_1,	b0_3_12,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_13,	b0_2_13,	r0_15_12,	r0_15_12,	b0_2_12,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_14,	b0_2_14,	r0_14_13,	r0_18_1,	b0_2_13,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_15_14,	b0_3_14,	r0_17_13,	r0_17_13,	b0_3_13,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_15,	b0_3_15,	r0_14_14,	r0_18_1,	b0_3_14,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_15,	b0_2_15,	r0_15_14,	r0_15_14,	b0_2_14,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_16,	b0_2_16,	r0_14_15,	r0_18_1,	b0_2_15,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_15_16,	b0_3_16,	r0_17_15,	r0_17_15,	b0_3_15,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_17,	b0_3_17,	r0_14_16,	r0_18_1,	b0_3_16,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_17,	b0_2_17,	r0_15_16,	r0_15_16,	b0_2_16,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_18,	b0_2_18,	r0_14_17,	r0_18_1,	b0_2_17,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_15_18,	b0_3_18,	r0_17_17,	r0_17_17,	b0_3_17,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_19,	b0_3_19,	r0_14_18,	r0_18_1,	b0_3_18,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_19,	b0_2_19,	r0_15_18,	r0_15_18,	b0_2_18,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_20,	b0_2_20,	r0_14_19,	r0_18_1,	b0_2_19,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_15_20,	b0_3_20,	r0_17_19,	r0_17_19,	b0_3_19,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_21,	b0_3_21,	r0_14_20,	r0_18_1,	b0_3_20,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_21,	b0_2_21,	r0_15_20,	r0_15_20,	b0_2_20,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_22,	b0_2_22,	r0_14_21,	r0_18_1,	b0_2_21,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_15_22,	b0_3_22,	r0_17_21,	r0_17_21,	b0_3_21,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_23,	b0_3_23,	r0_14_22,	r0_18_1,	b0_3_22,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_23,	b0_2_23,	r0_15_22,	r0_15_22,	b0_2_22,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_24,	b0_2_24,	r0_14_23,	r0_18_1,	b0_2_23,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_15_24,	b0_3_24,	r0_17_23,	r0_17_23,	b0_3_23,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_25,	b0_3_25,	r0_14_24,	r0_18_1,	b0_3_24,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_25,	b0_2_25,	r0_15_24,	r0_15_24,	b0_2_24,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_26,	b0_2_26,	r0_14_25,	r0_18_1,	b0_2_25,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_15_26,	b0_3_26,	r0_17_25,	r0_17_25,	b0_3_25,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_27,	b0_3_27,	r0_14_26,	r0_18_1,	b0_3_26,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_27,	b0_2_27,	r0_15_26,	r0_15_26,	b0_2_26,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_28,	b0_2_28,	r0_14_27,	r0_18_1,	b0_2_27,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_15_28,	b0_3_28,	r0_17_27,	r0_17_27,	b0_3_27,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_29,	b0_3_29,	r0_14_28,	r0_18_1,	b0_3_28,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_29,	b0_2_29,	r0_15_28,	r0_15_28,	b0_2_28,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_30,	b0_2_30,	r0_14_29,	r0_18_1,	b0_2_29,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_15_30,	b0_3_30,	r0_17_29,	r0_17_29,	b0_3_29,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_31,	b0_3_31,	r0_14_30,	r0_18_1,	b0_3_30,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_31,	b0_2_31,	r0_15_30,	r0_15_30,	b0_2_30,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_32,	b0_2_32,	r0_14_31,	r0_18_1,	b0_2_31,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_15_32,	b0_3_32,	r0_17_31,	r0_17_31,	b0_3_31,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_33,	b0_3_33,	r0_14_32,	r0_18_1,	b0_3_32,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_33,	b0_2_33,	r0_15_32,	r0_15_32,	b0_2_32,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_34,	b0_2_34,	r0_14_33,	r0_18_1,	b0_2_33,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_15_34,	b0_3_34,	r0_17_33,	r0_17_33,	b0_3_33,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_35,	b0_3_35,	r0_14_34,	r0_18_1,	b0_3_34,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_35,	b0_2_35,	r0_15_34,	r0_15_34,	b0_2_34,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_15_36,	b0_3_36,	r0_17_35,	r0_17_35,	b0_3_35,	ops) ;
      Build_OP (	TOP_cmpge_r_r_r,	r0_14_36,	r0_14_35,	Zero_TN,	ops) ;
      Build_OP (	TOP_orc_r_r_r,	r0_15_37,	r0_15_36,	Zero_TN,	ops) ;
      Build_OP (	TOP_sh1add_r_r_r,	r0_15_38,	r0_15_37,	r0_14_36,	ops) ;
      Build_OP (	TOP_targ_slct_r_r_b_r,	o0,	b0_1_1,	r0_16_1,	r0_15_38,	ops) ;
  }
} /* Expand__builtin__divuw */

/*
@@@  case INTRN_BUILTIN__DIVUW:
@@@    Expand__builtin__divuw(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of builtin__divw based on validated and scheduled basic assembly source.
*/
static void
Expand__builtin__divw(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_div_r_r_r)) {
      Build_OP (	TOP_div_r_r_r,	o0,	i0,	i1,	ops) ;
  } else {
      TN *b0_0_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_0_10 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_0_11 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_0_12 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_0_13 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_0_14 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_0_15 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_0_16 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_0_17 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_0_18 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_0_19 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_0_20 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_0_21 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_0_22 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_0_23 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_0_24 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_0_25 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_0_26 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_0_27 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_0_28 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_0_29 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_0_30 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_0_31 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_0_32 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_0_33 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_0_34 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_0_35 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_0_36 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_0_37 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_0_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_0_6 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_0_7 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_0_8 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_0_9 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_1_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_10 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_11 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_12 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_13 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_14 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_15 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_16 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_17 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_18 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_19 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_20 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_21 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_22 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_23 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_24 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_25 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_26 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_27 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_28 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_29 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_30 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_31 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_32 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_33 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_34 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_35 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_36 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_37 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_38 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_6 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_7 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_8 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_9 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *r0_14_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_10 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_11 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_12 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_13 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_14 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_15 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_16 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_17 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_18 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_19 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_20 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_21 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_22 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_23 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_24 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_25 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_26 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_27 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_28 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_29 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_30 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_31 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_32 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_33 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_34 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_35 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_36 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_37 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_38 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_41 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_6 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_7 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_8 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_9 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_10 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_12 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_14 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_16 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_18 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_20 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_22 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_24 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_26 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_28 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_30 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_32 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_34 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_36 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_38 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_39 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_40 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_6 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_8 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_17_11 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_17_13 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_17_15 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_17_17 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_17_19 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_17_21 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_17_23 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_17_25 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_17_27 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_17_29 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_17_31 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_17_33 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_17_35 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_17_37 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_17_7 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_17_9 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_18_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_19_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_19_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      Build_OP (	TOP_cmplt_r_r_r,	r0_14_0,	i0,	Zero_TN,	ops) ;
      Build_OP (	TOP_sub_r_r_r,	r0_15_0,	Zero_TN,	i0,	ops) ;
      Build_OP (	TOP_cmplt_r_r_r,	r0_18_1,	i1,	Zero_TN,	ops) ;
      Build_OP (	TOP_sub_r_r_r,	r0_19_1,	Zero_TN,	i1,	ops) ;
      Build_OP (	TOP_convib_r_b,	b0_1_1,	Zero_TN,	ops) ;
      Build_OP (	TOP_cmplt_r_r_b,	b0_0_1,		i0,	Zero_TN,	ops) ;
      Build_OP (	TOP_cmpeq_r_r_b,	b0_3_2,	r0_18_1,	r0_14_0,	ops) ;
      Build_OP (	TOP_cmplt_r_r_b,	b0_2_2,		i1,	Zero_TN,	ops) ;
      Build_OP (	TOP_targ_slct_r_r_b_r,	r0_15_4,	b0_0_1,	r0_15_0,	i0,	ops) ;
      Build_OP (	TOP_targ_slct_r_r_b_r,	r0_19_5,	b0_2_2,	r0_19_1,	i1,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_14_5,	b0_0_5,	r0_15_4,	r0_15_4,	b0_1_1,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_6,	b0_0_6,	Zero_TN,	r0_19_5,	b0_0_5,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_15_6,	b0_2_6,	r0_14_5,	r0_14_5,	b0_1_1,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_7,	b0_2_7,	r0_14_6,	r0_19_5,	b0_2_6,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_7,	b0_0_7,	r0_15_6,	r0_15_6,	b0_0_6,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_8,	b0_0_8,	r0_14_7,	r0_19_5,	b0_0_7,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_15_8,	b0_2_8,	r0_17_7,	r0_17_7,	b0_2_7,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_9,	b0_2_9,	r0_14_8,	r0_19_5,	b0_2_8,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_9,	b0_0_9,	r0_15_8,	r0_15_8,	b0_0_8,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_10,	b0_0_10,	r0_14_9,	r0_19_5,	b0_0_9,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_15_10,	b0_2_10,	r0_17_9,	r0_17_9,	b0_2_9,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_11,	b0_2_11,	r0_14_10,	r0_19_5,	b0_2_10,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_11,	b0_0_11,	r0_15_10,	r0_15_10,	b0_0_10,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_12,	b0_0_12,	r0_14_11,	r0_19_5,	b0_0_11,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_15_12,	b0_2_12,	r0_17_11,	r0_17_11,	b0_2_11,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_13,	b0_2_13,	r0_14_12,	r0_19_5,	b0_2_12,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_13,	b0_0_13,	r0_15_12,	r0_15_12,	b0_0_12,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_14,	b0_0_14,	r0_14_13,	r0_19_5,	b0_0_13,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_15_14,	b0_2_14,	r0_17_13,	r0_17_13,	b0_2_13,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_15,	b0_2_15,	r0_14_14,	r0_19_5,	b0_2_14,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_15,	b0_0_15,	r0_15_14,	r0_15_14,	b0_0_14,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_16,	b0_0_16,	r0_14_15,	r0_19_5,	b0_0_15,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_15_16,	b0_2_16,	r0_17_15,	r0_17_15,	b0_2_15,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_17,	b0_2_17,	r0_14_16,	r0_19_5,	b0_2_16,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_17,	b0_0_17,	r0_15_16,	r0_15_16,	b0_0_16,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_18,	b0_0_18,	r0_14_17,	r0_19_5,	b0_0_17,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_15_18,	b0_2_18,	r0_17_17,	r0_17_17,	b0_2_17,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_19,	b0_2_19,	r0_14_18,	r0_19_5,	b0_2_18,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_19,	b0_0_19,	r0_15_18,	r0_15_18,	b0_0_18,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_20,	b0_0_20,	r0_14_19,	r0_19_5,	b0_0_19,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_15_20,	b0_2_20,	r0_17_19,	r0_17_19,	b0_2_19,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_21,	b0_2_21,	r0_14_20,	r0_19_5,	b0_2_20,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_21,	b0_0_21,	r0_15_20,	r0_15_20,	b0_0_20,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_22,	b0_0_22,	r0_14_21,	r0_19_5,	b0_0_21,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_15_22,	b0_2_22,	r0_17_21,	r0_17_21,	b0_2_21,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_23,	b0_2_23,	r0_14_22,	r0_19_5,	b0_2_22,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_23,	b0_0_23,	r0_15_22,	r0_15_22,	b0_0_22,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_24,	b0_0_24,	r0_14_23,	r0_19_5,	b0_0_23,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_15_24,	b0_2_24,	r0_17_23,	r0_17_23,	b0_2_23,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_25,	b0_2_25,	r0_14_24,	r0_19_5,	b0_2_24,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_25,	b0_0_25,	r0_15_24,	r0_15_24,	b0_0_24,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_26,	b0_0_26,	r0_14_25,	r0_19_5,	b0_0_25,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_15_26,	b0_2_26,	r0_17_25,	r0_17_25,	b0_2_25,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_27,	b0_2_27,	r0_14_26,	r0_19_5,	b0_2_26,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_27,	b0_0_27,	r0_15_26,	r0_15_26,	b0_0_26,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_28,	b0_0_28,	r0_14_27,	r0_19_5,	b0_0_27,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_15_28,	b0_2_28,	r0_17_27,	r0_17_27,	b0_2_27,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_29,	b0_2_29,	r0_14_28,	r0_19_5,	b0_2_28,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_29,	b0_0_29,	r0_15_28,	r0_15_28,	b0_0_28,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_30,	b0_0_30,	r0_14_29,	r0_19_5,	b0_0_29,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_15_30,	b0_2_30,	r0_17_29,	r0_17_29,	b0_2_29,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_31,	b0_2_31,	r0_14_30,	r0_19_5,	b0_2_30,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_31,	b0_0_31,	r0_15_30,	r0_15_30,	b0_0_30,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_32,	b0_0_32,	r0_14_31,	r0_19_5,	b0_0_31,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_15_32,	b0_2_32,	r0_17_31,	r0_17_31,	b0_2_31,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_33,	b0_2_33,	r0_14_32,	r0_19_5,	b0_2_32,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_33,	b0_0_33,	r0_15_32,	r0_15_32,	b0_0_32,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_34,	b0_0_34,	r0_14_33,	r0_19_5,	b0_0_33,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_15_34,	b0_2_34,	r0_17_33,	r0_17_33,	b0_2_33,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_35,	b0_2_35,	r0_14_34,	r0_19_5,	b0_2_34,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_35,	b0_0_35,	r0_15_34,	r0_15_34,	b0_0_34,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_36,	b0_0_36,	r0_14_35,	r0_19_5,	b0_0_35,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_15_36,	b0_2_36,	r0_17_35,	r0_17_35,	b0_2_35,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_37,	b0_2_37,	r0_14_36,	r0_19_5,	b0_2_36,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_37,	b0_0_37,	r0_15_36,	r0_15_36,	b0_0_36,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_15_38,	b0_2_38,	r0_17_37,	r0_17_37,	b0_2_37,	ops) ;
      Build_OP (	TOP_cmpge_r_r_r,	r0_14_38,	r0_14_37,	Zero_TN,	ops) ;
      Build_OP (	TOP_orc_r_r_r,	r0_15_39,	r0_15_38,	Zero_TN,	ops) ;
      Build_OP (	TOP_sh1add_r_r_r,	r0_15_40,	r0_15_39,	r0_14_38,	ops) ;
      Build_OP (	TOP_sub_r_r_r,	r0_14_41,	Zero_TN,	r0_15_40,	ops) ;
      Build_OP (	TOP_targ_slct_r_r_b_r,	o0,	b0_3_2,	r0_15_40,	r0_14_41,	ops) ;
  }
} /* Expand__builtin__divw */

/*
@@@  case INTRN_BUILTIN__DIVW:
@@@    Expand__builtin__divw(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/


/*
 * Expansion of builtin__moduw based on validated and scheduled basic assembly source.
*/
static void
Expand__builtin__moduw(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_remu_r_r_r)) {
      Build_OP (	TOP_remu_r_r_r,	o0,	i0,	i1,	ops) ;
  } else {
      TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_0_36 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_1_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_1_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_10 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_11 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_12 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_13 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_14 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_15 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_16 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_17 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_18 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_19 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_20 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_21 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_22 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_23 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_24 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_25 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_26 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_27 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_28 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_29 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_30 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_31 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_32 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_33 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_34 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_6 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_7 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_8 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_9 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_10 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_11 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_12 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_13 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_14 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_15 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_16 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_17 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_18 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_19 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_20 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_21 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_22 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_23 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_24 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_25 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_26 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_27 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_28 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_29 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_30 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_31 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_32 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_33 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_34 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_35 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_6 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_7 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_8 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_9 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *r0_0_34 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_11 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_13 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_15 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_17 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_19 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_21 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_23 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_25 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_27 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_29 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_31 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_33 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_7 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_9 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_10 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_11 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_12 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_13 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_14 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_15 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_16 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_17 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_18 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_19 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_20 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_21 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_22 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_23 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_24 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_25 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_26 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_27 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_28 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_29 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_30 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_31 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_32 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_33 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_34 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_35 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_37 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_6 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_7 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_8 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_9 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_17_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_17_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_18_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_18_36 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_19_10 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_19_12 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_19_14 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_19_16 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_19_18 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_19_20 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_19_22 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_19_24 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_19_26 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_19_28 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_19_30 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_19_32 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_19_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_19_6 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_19_8 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      Build_OP (	TOP_cmplt_r_r_r,	r0_14_0,	i1,	Zero_TN,	ops) ;
      Build_OP (	TOP_convib_r_b,	b0_0_0,	Zero_TN,	ops) ;
      Build_OP (	TOP_shru_r_r_r,	r0_18_1,	i1,	r0_14_0,	ops) ;
      Build_OP (	TOP_shru_r_r_r,	r0_15_1,	i0,	r0_14_0,	ops) ;
      Build_OP (	TOP_cmpgeu_r_r_b,	b0_1_1,	i0,	i1,	ops) ;
      Build_OP (	TOP_sub_r_r_r,	r0_17_1,	i0,	i1,	ops) ;
      Build_OP (	TOP_targ_slct_r_r_b_r,	r0_17_2,	b0_1_1,	r0_17_1,	i0,	ops) ;
      Build_OP (	TOP_cmplt_r_r_b,	b0_1_2,		i1,	Zero_TN,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_14_3,	b0_2_3,	r0_15_1,	r0_15_1,	b0_0_0,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_15_4,	b0_2_4,	Zero_TN,	r0_18_1,	b0_2_3,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_19_4,	b0_3_4,	r0_14_3,	r0_14_3,	b0_0_0,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_15_5,	b0_3_5,	r0_15_4,	r0_18_1,	b0_3_4,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_14_5,	b0_2_5,	r0_19_4,	r0_19_4,	b0_2_4,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_15_6,	b0_2_6,	r0_15_5,	r0_18_1,	b0_2_5,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_19_6,	b0_3_6,	r0_14_5,	r0_14_5,	b0_3_5,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_15_7,	b0_3_7,	r0_15_6,	r0_18_1,	b0_3_6,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_14_7,	b0_2_7,	r0_19_6,	r0_19_6,	b0_2_6,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_15_8,	b0_2_8,	r0_15_7,	r0_18_1,	b0_2_7,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_19_8,	b0_3_8,	r0_14_7,	r0_14_7,	b0_3_7,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_15_9,	b0_3_9,	r0_15_8,	r0_18_1,	b0_3_8,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_14_9,	b0_2_9,	r0_19_8,	r0_19_8,	b0_2_8,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_15_10,	b0_2_10,	r0_15_9,	r0_18_1,	b0_2_9,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_19_10,	b0_3_10,	r0_14_9,	r0_14_9,	b0_3_9,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_15_11,	b0_3_11,	r0_15_10,	r0_18_1,	b0_3_10,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_14_11,	b0_2_11,	r0_19_10,	r0_19_10,	b0_2_10,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_15_12,	b0_2_12,	r0_15_11,	r0_18_1,	b0_2_11,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_19_12,	b0_3_12,	r0_14_11,	r0_14_11,	b0_3_11,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_15_13,	b0_3_13,	r0_15_12,	r0_18_1,	b0_3_12,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_14_13,	b0_2_13,	r0_19_12,	r0_19_12,	b0_2_12,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_15_14,	b0_2_14,	r0_15_13,	r0_18_1,	b0_2_13,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_19_14,	b0_3_14,	r0_14_13,	r0_14_13,	b0_3_13,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_15_15,	b0_3_15,	r0_15_14,	r0_18_1,	b0_3_14,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_14_15,	b0_2_15,	r0_19_14,	r0_19_14,	b0_2_14,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_15_16,	b0_2_16,	r0_15_15,	r0_18_1,	b0_2_15,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_19_16,	b0_3_16,	r0_14_15,	r0_14_15,	b0_3_15,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_15_17,	b0_3_17,	r0_15_16,	r0_18_1,	b0_3_16,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_14_17,	b0_2_17,	r0_19_16,	r0_19_16,	b0_2_16,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_15_18,	b0_2_18,	r0_15_17,	r0_18_1,	b0_2_17,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_19_18,	b0_3_18,	r0_14_17,	r0_14_17,	b0_3_17,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_15_19,	b0_3_19,	r0_15_18,	r0_18_1,	b0_3_18,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_14_19,	b0_2_19,	r0_19_18,	r0_19_18,	b0_2_18,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_15_20,	b0_2_20,	r0_15_19,	r0_18_1,	b0_2_19,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_19_20,	b0_3_20,	r0_14_19,	r0_14_19,	b0_3_19,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_15_21,	b0_3_21,	r0_15_20,	r0_18_1,	b0_3_20,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_14_21,	b0_2_21,	r0_19_20,	r0_19_20,	b0_2_20,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_15_22,	b0_2_22,	r0_15_21,	r0_18_1,	b0_2_21,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_19_22,	b0_3_22,	r0_14_21,	r0_14_21,	b0_3_21,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_15_23,	b0_3_23,	r0_15_22,	r0_18_1,	b0_3_22,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_14_23,	b0_2_23,	r0_19_22,	r0_19_22,	b0_2_22,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_15_24,	b0_2_24,	r0_15_23,	r0_18_1,	b0_2_23,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_19_24,	b0_3_24,	r0_14_23,	r0_14_23,	b0_3_23,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_15_25,	b0_3_25,	r0_15_24,	r0_18_1,	b0_3_24,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_14_25,	b0_2_25,	r0_19_24,	r0_19_24,	b0_2_24,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_15_26,	b0_2_26,	r0_15_25,	r0_18_1,	b0_2_25,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_19_26,	b0_3_26,	r0_14_25,	r0_14_25,	b0_3_25,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_15_27,	b0_3_27,	r0_15_26,	r0_18_1,	b0_3_26,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_14_27,	b0_2_27,	r0_19_26,	r0_19_26,	b0_2_26,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_15_28,	b0_2_28,	r0_15_27,	r0_18_1,	b0_2_27,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_19_28,	b0_3_28,	r0_14_27,	r0_14_27,	b0_3_27,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_15_29,	b0_3_29,	r0_15_28,	r0_18_1,	b0_3_28,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_14_29,	b0_2_29,	r0_19_28,	r0_19_28,	b0_2_28,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_15_30,	b0_2_30,	r0_15_29,	r0_18_1,	b0_2_29,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_19_30,	b0_3_30,	r0_14_29,	r0_14_29,	b0_3_29,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_15_31,	b0_3_31,	r0_15_30,	r0_18_1,	b0_3_30,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_14_31,	b0_2_31,	r0_19_30,	r0_19_30,	b0_2_30,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_15_32,	b0_2_32,	r0_15_31,	r0_18_1,	b0_2_31,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_19_32,	b0_3_32,	r0_14_31,	r0_14_31,	b0_3_31,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_15_33,	b0_3_33,	r0_15_32,	r0_18_1,	b0_3_32,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_14_33,	b0_2_33,	r0_19_32,	r0_19_32,	b0_2_32,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_15_34,	b0_2_34,	r0_15_33,	r0_18_1,	b0_2_33,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_0_34,	b0_3_34,	r0_14_33,	r0_14_33,	b0_3_33,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_15_35,	b0_3_35,	r0_15_34,	r0_18_1,	b0_3_34,	ops) ;
      Build_OP (	TOP_cmpge_r_r_b,	b0_0_36,	r0_15_35,	r0_0_34,	ops) ;
      Build_OP (	TOP_add_r_r_r,	r0_18_36,	r0_15_35,	r0_18_1,	ops) ;
      Build_OP (	TOP_targ_slct_r_r_b_r,	r0_15_37,	b0_0_36,	r0_15_35,	r0_18_36,	ops) ;
      Build_OP (	TOP_targ_slct_r_r_b_r,	o0,	b0_1_2,	r0_17_2,	r0_15_37,	ops) ;
  }
} /* Expand__builtin__moduw */

/*
@@@  case INTRN_BUILTIN__MODUW:
@@@    Expand__builtin__moduw(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of builtin__modw based on validated and scheduled basic assembly source.
*/
static void
Expand__builtin__modw(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_rem_r_r_r)) {
      Build_OP (	TOP_rem_r_r_r,	o0,	i0,	i1,	ops) ;
  } else {
      TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_1_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_1_37 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_10 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_11 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_12 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_13 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_14 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_15 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_16 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_17 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_18 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_19 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_20 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_21 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_22 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_23 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_24 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_25 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_26 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_27 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_28 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_29 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_30 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_31 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_32 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_33 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_34 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_35 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_6 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_7 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_8 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_9 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_10 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_11 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_12 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_13 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_14 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_15 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_16 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_17 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_18 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_19 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_20 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_21 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_22 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_23 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_24 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_25 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_26 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_27 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_28 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_29 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_30 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_31 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_32 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_33 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_34 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_35 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_36 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_6 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_7 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_8 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_9 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *r0_0_35 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_10 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_11 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_12 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_13 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_14 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_15 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_16 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_17 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_18 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_19 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_20 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_21 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_22 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_23 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_24 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_25 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_26 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_27 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_28 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_29 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_30 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_31 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_32 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_33 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_34 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_35 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_36 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_38 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_6 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_7 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_8 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_14_9 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_37 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_15_39 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_17_10 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_17_12 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_17_14 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_17_16 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_17_18 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_17_20 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_17_22 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_17_24 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_17_26 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_17_28 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_17_30 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_17_32 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_17_34 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_17_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_17_6 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_17_8 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_18_11 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_18_13 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_18_15 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_18_17 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_18_19 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_18_21 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_18_23 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_18_25 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_18_27 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_18_29 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_18_31 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_18_33 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_18_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_18_7 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_18_9 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      Build_OP (	TOP_cmplt_r_r_b,	b0_0_0,	i0,	Zero_TN,	ops) ;
      Build_OP (	TOP_sub_r_r_r,	r0_14_0,	Zero_TN,	i0,	ops) ;
      Build_OP (	TOP_cmplt_r_r_b,	b0_2_1,	i1,	Zero_TN,	ops) ;
      Build_OP (	TOP_sub_r_r_r,	r0_15_1,	Zero_TN,	i1,	ops) ;
      Build_OP (	TOP_targ_slct_r_r_b_r,	r0_14_1,	b0_0_0,	r0_14_0,	i0,	ops) ;
      Build_OP (	TOP_convib_r_b,	b0_1_1,	Zero_TN,	ops) ;
      Build_OP (	TOP_targ_slct_r_r_b_r,	r0_15_2,	b0_2_1,	r0_15_1,	i1,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_4,	b0_2_4,	r0_14_1,	r0_14_1,	b0_1_1,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_5,	b0_2_5,	Zero_TN,	r0_15_2,	b0_2_4,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_18_5,	b0_3_5,	r0_17_4,	r0_17_4,	b0_1_1,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_6,	b0_3_6,	r0_14_5,	r0_15_2,	b0_3_5,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_6,	b0_2_6,	r0_18_5,	r0_18_5,	b0_2_5,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_7,	b0_2_7,	r0_14_6,	r0_15_2,	b0_2_6,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_18_7,	b0_3_7,	r0_17_6,	r0_17_6,	b0_3_6,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_8,	b0_3_8,	r0_14_7,	r0_15_2,	b0_3_7,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_8,	b0_2_8,	r0_18_7,	r0_18_7,	b0_2_7,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_9,	b0_2_9,	r0_14_8,	r0_15_2,	b0_2_8,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_18_9,	b0_3_9,	r0_17_8,	r0_17_8,	b0_3_8,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_10,	b0_3_10,	r0_14_9,	r0_15_2,	b0_3_9,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_10,	b0_2_10,	r0_18_9,	r0_18_9,	b0_2_9,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_11,	b0_2_11,	r0_14_10,	r0_15_2,	b0_2_10,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_18_11,	b0_3_11,	r0_17_10,	r0_17_10,	b0_3_10,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_12,	b0_3_12,	r0_14_11,	r0_15_2,	b0_3_11,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_12,	b0_2_12,	r0_18_11,	r0_18_11,	b0_2_11,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_13,	b0_2_13,	r0_14_12,	r0_15_2,	b0_2_12,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_18_13,	b0_3_13,	r0_17_12,	r0_17_12,	b0_3_12,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_14,	b0_3_14,	r0_14_13,	r0_15_2,	b0_3_13,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_14,	b0_2_14,	r0_18_13,	r0_18_13,	b0_2_13,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_15,	b0_2_15,	r0_14_14,	r0_15_2,	b0_2_14,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_18_15,	b0_3_15,	r0_17_14,	r0_17_14,	b0_3_14,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_16,	b0_3_16,	r0_14_15,	r0_15_2,	b0_3_15,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_16,	b0_2_16,	r0_18_15,	r0_18_15,	b0_2_15,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_17,	b0_2_17,	r0_14_16,	r0_15_2,	b0_2_16,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_18_17,	b0_3_17,	r0_17_16,	r0_17_16,	b0_3_16,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_18,	b0_3_18,	r0_14_17,	r0_15_2,	b0_3_17,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_18,	b0_2_18,	r0_18_17,	r0_18_17,	b0_2_17,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_19,	b0_2_19,	r0_14_18,	r0_15_2,	b0_2_18,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_18_19,	b0_3_19,	r0_17_18,	r0_17_18,	b0_3_18,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_20,	b0_3_20,	r0_14_19,	r0_15_2,	b0_3_19,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_20,	b0_2_20,	r0_18_19,	r0_18_19,	b0_2_19,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_21,	b0_2_21,	r0_14_20,	r0_15_2,	b0_2_20,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_18_21,	b0_3_21,	r0_17_20,	r0_17_20,	b0_3_20,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_22,	b0_3_22,	r0_14_21,	r0_15_2,	b0_3_21,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_22,	b0_2_22,	r0_18_21,	r0_18_21,	b0_2_21,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_23,	b0_2_23,	r0_14_22,	r0_15_2,	b0_2_22,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_18_23,	b0_3_23,	r0_17_22,	r0_17_22,	b0_3_22,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_24,	b0_3_24,	r0_14_23,	r0_15_2,	b0_3_23,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_24,	b0_2_24,	r0_18_23,	r0_18_23,	b0_2_23,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_25,	b0_2_25,	r0_14_24,	r0_15_2,	b0_2_24,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_18_25,	b0_3_25,	r0_17_24,	r0_17_24,	b0_3_24,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_26,	b0_3_26,	r0_14_25,	r0_15_2,	b0_3_25,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_26,	b0_2_26,	r0_18_25,	r0_18_25,	b0_2_25,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_27,	b0_2_27,	r0_14_26,	r0_15_2,	b0_2_26,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_18_27,	b0_3_27,	r0_17_26,	r0_17_26,	b0_3_26,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_28,	b0_3_28,	r0_14_27,	r0_15_2,	b0_3_27,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_28,	b0_2_28,	r0_18_27,	r0_18_27,	b0_2_27,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_29,	b0_2_29,	r0_14_28,	r0_15_2,	b0_2_28,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_18_29,	b0_3_29,	r0_17_28,	r0_17_28,	b0_3_28,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_30,	b0_3_30,	r0_14_29,	r0_15_2,	b0_3_29,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_30,	b0_2_30,	r0_18_29,	r0_18_29,	b0_2_29,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_31,	b0_2_31,	r0_14_30,	r0_15_2,	b0_2_30,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_18_31,	b0_3_31,	r0_17_30,	r0_17_30,	b0_3_30,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_32,	b0_3_32,	r0_14_31,	r0_15_2,	b0_3_31,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_32,	b0_2_32,	r0_18_31,	r0_18_31,	b0_2_31,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_33,	b0_2_33,	r0_14_32,	r0_15_2,	b0_2_32,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_18_33,	b0_3_33,	r0_17_32,	r0_17_32,	b0_3_32,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_34,	b0_3_34,	r0_14_33,	r0_15_2,	b0_3_33,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_34,	b0_2_34,	r0_18_33,	r0_18_33,	b0_2_33,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_35,	b0_2_35,	r0_14_34,	r0_15_2,	b0_2_34,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_0_35,	b0_3_35,	r0_17_34,	r0_17_34,	b0_3_34,	ops) ;
      Build_OP (	TOP_divs_b_r_r_b_r,	r0_14_36,	b0_3_36,	r0_14_35,	r0_15_2,	b0_3_35,	ops) ;
      Build_OP (	TOP_cmpge_r_r_b,	b0_1_37,	r0_14_36,	r0_0_35,	ops) ;
      Build_OP (	TOP_add_r_r_r,	r0_15_37,	r0_14_36,	r0_15_2,	ops) ;
      Build_OP (	TOP_targ_slct_r_r_b_r,	r0_14_38,	b0_1_37,	r0_14_36,	r0_15_37,	ops) ;
      Build_OP (	TOP_sub_r_r_r,	r0_15_39,	r0_0_35,	r0_14_38,	ops) ;
      Build_OP (	TOP_targ_slct_r_r_b_r,	o0,	b0_0_0,	r0_15_39,	r0_14_38,	ops) ;
  }
} /* Expand__builtin__modw */

/*
@@@  case INTRN_BUILTIN__MODW:
@@@    Expand__builtin__modw(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 *Manual expansion of some experimental ST231 operators
 *Either expanded as ST231 operators or expanded as emulated insns.
*/
static void
Expand__st200mul32(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
    if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_mul32_r_r_r)) {
	Build_OP (	TOP_mul32_r_r_r,	o0,	i0,	i1,	ops) ;
    } else {
	/* Emulated by __mulw */
	TN *r0_24_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
	TN *r0_25_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
	Build_OP (	TOP_mullu_r_r_r,	r0_24_2,	i0,	i1,	ops) ;
	Build_OP (	TOP_mulhs_r_r_r,	r0_25_2,	i0,	i1,	ops) ;
	Build_OP (	TOP_add_r_r_r,	o0,	r0_24_2,	r0_25_2,	ops) ;
    }
} /* Expand__st200mul32 */

static void
Expand__st200mul64h(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
    if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_mul64h_r_r_r)) {
	Build_OP (	TOP_mul64h_r_r_r,	o0,	i0,	i1,	ops) ;
    } else {
	/* Emulated by __mulhw */
	TN *b0_2_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
	TN *b0_3_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
	TN *b0_3_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
	TN *r0_16_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
	TN *r0_24_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
	TN *r0_25_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
	TN *r0_26_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
	TN *r0_27_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
	Build_OP (	TOP_mullu_r_r_r,	r0_26_2,	i0,	i1,	ops) ;
	Build_OP (	TOP_mulhs_r_r_r,	r0_27_2,	i0,	i1,	ops) ;
	Build_OP (	TOP_mulhhs_r_r_r,	r0_24_3,	i0,	i1,	ops) ;
	Build_OP (	TOP_mullhus_r_r_r,	r0_25_3,	i0,	i1,	ops) ;
	Build_OP (	TOP_convib_r_b,	b0_2_2,	Zero_TN,	ops) ;
	Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_16_3,	b0_3_3,	r0_26_2,	r0_27_2,	b0_2_2,	ops) ;
	Build_OP (	TOP_targ_addcg_b_r_r_b_r,	o0,	b0_3_4,	r0_24_3,	r0_25_3,	b0_3_3,	ops) ;	
    }
} /* Expand__st200mul64h */

static void
Expand__st200mul64hu(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_mul64hu_r_r_r)) { 
      Build_OP (	TOP_mul64hu_r_r_r,	o0,	i0,	i1,	ops) ;
  } else {
      /* Emulated by __mulhuw */
      TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_1_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *r0_16_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_17_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_24_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_25_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_26_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_27_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_28_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_28_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_29_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      Build_OP (	TOP_mullu_r_r_r,	r0_26_2,	i0,	i1,	ops) ;
      Build_OP (	TOP_mulhs_r_r_r,	r0_27_2,	i0,	i1,	ops) ;
      Build_OP (	TOP_cmplt_r_r_b,	b0_0_0,	i0,	Zero_TN,	ops) ;
      Build_OP (	TOP_cmplt_r_r_b,	b0_1_0,	i1,	Zero_TN,	ops) ;
      Build_OP (	TOP_mulhhs_r_r_r,	r0_24_3,	i0,	i1,	ops) ;
      Build_OP (	TOP_mullhus_r_r_r,	r0_25_3,	i0,	i1,	ops) ;
      Build_OP (	TOP_targ_slct_r_r_b_r,	r0_28_1,	b0_1_0,	i0,	Zero_TN,	ops) ;
      Build_OP (	TOP_targ_slct_r_r_b_r,	r0_29_1,	b0_0_0,	i1,	Zero_TN,	ops) ;
      Build_OP (	TOP_add_r_r_r,	r0_28_2,	r0_28_1,	r0_29_1,	ops) ;
      Build_OP (	TOP_convib_r_b,	b0_2_2,	Zero_TN,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_16_3,	b0_3_3,	r0_26_2,	r0_27_2,	b0_2_2,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_4,	b0_3_4,	r0_24_3,	r0_25_3,	b0_3_3,	ops) ;
      Build_OP (	TOP_add_r_r_r,	o0,	r0_17_4,	r0_28_2,	ops) ;
  }
} /* Expand__st200mul64hu */

static void
Expand__st200mulfrac(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_mulfrac_r_r_r)) { 
      Build_OP (	TOP_mulfrac_r_r_r,	o0,	i0,	i1,	ops) ;
  } else {
      TN *b0_0_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *r0_16_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_16_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_16_6 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_16_7 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_17_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_17_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_20_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_21_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_21_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_21_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_24_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_25_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_26_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_27_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)                (((long long)(x)<<32) >> 32)
      TN *c0x7fffffff = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *c0x7fffffff_ltn = Gen_Literal_TN(__EXTS32TOS64(0x7fffffff), 4) ;
      Build_OP (    TOP_mov_ii_r,      c0x7fffffff,    c0x7fffffff_ltn,        ops) ;
      TN *c0x80000000 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *c0x80000000_ltn = Gen_Literal_TN(__EXTS32TOS64(0x80000000), 4) ;
      Build_OP (    TOP_mov_ii_r,      c0x80000000,    c0x80000000_ltn,        ops) ;
      TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
      TN *c31 = Gen_Literal_TN(__EXTS32TOS64(31), 4) ;
#undef __EXTS32TOS64
      Build_OP (    TOP_mullu_r_r_r,    r0_26_2,        i0,     i1,     ops) ;
      Build_OP (    TOP_mulhs_r_r_r,    r0_27_2,        i0,     i1,     ops) ;
      Build_OP (    TOP_mulhhs_r_r_r,   r0_24_3,        i0,     i1,     ops) ;
      Build_OP (    TOP_mullhus_r_r_r,  r0_25_3,        i0,     i1,     ops) ;
      Build_OP (    TOP_cmpeq_r_r_r,  r0_20_2,        i0,     c0x80000000,    ops) ;
      Build_OP (    TOP_cmpeq_r_r_r,  r0_21_2,        i0,     i1,     ops) ;
      Build_OP (    TOP_convib_r_b,        b0_2_2, Zero_TN,        ops) ;
      Build_OP (    TOP_andl_r_r_b,   b0_0_3, r0_20_2,        r0_21_2,        ops) ;
      Build_OP (    TOP_targ_addcg_b_r_r_b_r,      r0_16_3,        b0_3_3, r0_26_2,        r0_27_2,        b0_2_2, ops) ;
      Build_OP (    TOP_shl_i_r_r,      r0_17_4,        r0_16_3,        c1,     ops) ;
      Build_OP (    TOP_targ_addcg_b_r_r_b_r,      r0_21_4,        b0_3_4, r0_24_3,        r0_25_3,        b0_3_3, ops) ;
      Build_OP (    TOP_shru_i_r_r,     r0_16_4,        r0_16_3,        c31,    ops) ;
      Build_OP (    TOP_shl_i_r_r,      r0_21_5,        r0_21_4,        c1,     ops) ;
      Build_OP (    TOP_shru_i_r_r,     r0_17_5,        r0_17_4,        c31,    ops) ;
      Build_OP (    TOP_or_r_r_r,       r0_16_6,        r0_16_4,        r0_21_5,        ops) ;
      Build_OP (    TOP_add_r_r_r,      r0_16_7,        r0_16_6,        r0_17_5,        ops) ;
      Build_OP (    TOP_slctf_r_r_b_r,    o0,     b0_0_3, r0_16_7,        c0x7fffffff,    ops) ;      
  }
} /* Expand__st200mulfrac */


static void
Expand__st200prginspg(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_prginspg_r_i)) { 
      TN *c0 = Gen_Literal_TN(0LL, 4) ;
      Build_OP (	TOP_add_r_r_r,	o0,	i0,	i1,	ops) ;
      Build_OP (	TOP_prginspg_r_i,	c0, 	o0,	ops) ;
  } else {
      // Unsupported on st220
    Expand_Unimplemented_Intrinsic (TOP_prginspg_r_i, ops);
  }
} /* Expand__st200prginspg */

/*
 * Expansion of st220addcg based on validated and scheduled basic assembly source.
*/
static void
Expand__st220addcg(
 TN* ol0,
 TN* oh0,
 TN* i0,
 TN* i1,
 TN* i2,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_0_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  Build_OP (    TOP_convib_r_b,        b0_0_0, i2,     ops) ;
  Build_OP (    TOP_targ_addcg_b_r_r_b_r,      ol0,    b0_0_1, i0,     i1,     b0_0_0, ops) ;
  Build_OP (    TOP_convbi_b_r,        oh0,    b0_0_1, ops) ;
} /* Expand__st220addcg */

/*
@@@  case INTRN_ST220ADDCG:
@@@    Expand__st220addcg(result[0],result[1],opnd[0],opnd[1],opnd[2],ops) ;
@@@  break ;
*/

/*
 * Expansion of st220divs based on validated and scheduled basic assembly source.
*/
static void
Expand__st220divs(
 TN* ol0,
 TN* oh0,
 TN* i0,
 TN* i1,
 TN* i2,
 OPS* ops
)
{
  if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_divs_b_r_r_b_r)) {
    TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_0_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    Build_OP (    TOP_convib_r_b,        b0_0_0, i2,     ops) ;
    Build_OP (    TOP_divs_b_r_r_b_r,       ol0,    b0_0_1, i0,     i1,     b0_0_0, ops) ;
    Build_OP (    TOP_convbi_b_r,        oh0,    b0_0_1, ops) ;
  } else {
      // Unsupported on st240
    Expand_Unimplemented_Intrinsic (TOP_divs_b_r_r_b_r, ops);
  }
} /* Expand__st220divs */

/*
@@@  case INTRN_ST220DIVS:
@@@    Expand__st220divs(result[0],result[1],opnd[0],opnd[1],opnd[2],ops) ;
@@@  break ;
*/

/*
 * Expansion of st220mulhhs based on validated and scheduled basic assembly source.
*/
static void
Expand__st220mulhhs(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_mulhhs_r_r_r)) {
    Build_OP (	TOP_mulhhs_r_r_r,	o0,	i0,	i1,	ops) ;
  } else {
    TN *r0_17_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
    TN *c0xffff0000 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *c0xffff0000_ltn = Gen_Literal_TN(__EXTS32TOS64(0xffff0000), 4) ;
    Build_OP (	TOP_mov_ii_r,	c0xffff0000,	c0xffff0000_ltn,	ops) ;
#undef __EXTS32TOS64
    Build_OP (	TOP_and_r_r_r,	r0_17_0,	i1,	c0xffff0000,	ops) ;
    Build_OP (	TOP_mul64h_r_r_r,	o0,	i0,	r0_17_0,	ops) ;
  }
} /* Expand__st220mulhhs */

/*
@@@  case INTRN_ST220MULHHS:
@@@    Expand__st220mulhhs(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of st220mulhs based on validated and scheduled basic assembly source.
*/
static void
Expand__st220mulhs(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_mulhs_r_r_r)) {
    Build_OP (	TOP_mulhs_r_r_r,	o0,	i0,	i1,	ops) ;
  } else {
    TN *r0_8_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
    TN *c0xffff0000 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *c0xffff0000_ltn = Gen_Literal_TN(__EXTS32TOS64(0xffff0000), 4) ;
    Build_OP (	TOP_mov_ii_r,	c0xffff0000,	c0xffff0000_ltn,	ops) ;
#undef __EXTS32TOS64
    Build_OP (	TOP_and_r_r_r,	r0_8_0,	i1,	c0xffff0000,	ops) ;
    Build_OP (	TOP_mul32_r_r_r,	o0,	i0,	r0_8_0,	ops) ;
  }
} /* Expand__st220mulhs */

/*
@@@  case INTRN_ST220MULHS:
@@@    Expand__st220mulhs(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of st220mullhus based on validated and scheduled basic assembly source.
*/
static void
Expand__st220mullhus(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_mullhus_r_r_r)) {
    Build_OP (	TOP_mullhus_r_r_r,	o0,	i0,	i1,	ops) ;
  } else {
    TN *r0_17_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    Build_OP (	TOP_zxth_r_r,	r0_17_0,	i1,	ops) ;
    Build_OP (	TOP_mul64h_r_r_r,	o0,	i0,	r0_17_0,	ops) ;
  }
} /* Expand__st220mullhus */

/*
@@@  case INTRN_ST220MULLHUS:
@@@    Expand__st220mullhus(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of st220mullu based on validated and scheduled basic assembly source.
*/
static void
Expand__st220mullu(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_mullu_r_r_r)) {
    Build_OP (	TOP_mullu_r_r_r,	o0,	i0,	i1,	ops) ;
  } else {
    TN *r0_17_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    Build_OP (	TOP_zxth_r_r,	r0_17_0,	i1,	ops) ;
    Build_OP (	TOP_mul32_r_r_r,	o0,	i0,	r0_17_0,	ops) ;
  }
} /* Expand__st220mullu */

/*
@@@  case INTRN_ST220MULLU:
@@@    Expand__st220mullu(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of st220pft based on validated and scheduled basic assembly source.
*/
static void
Expand__st220pft(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  TN *c0 = Gen_Literal_TN(0LL, 4) ;
  Build_OP (	TOP_add_r_r_r,	o0,	i0,	i1,	ops) ;
  Build_OP (	TOP_pft_r_i,	c0,	o0,	ops) ;
} /* Expand__st220pft */

/*
@@@  case INTRN_ST220PFT:
@@@    Expand__st220pft(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of st220prgadd based on validated and scheduled basic assembly source.
*/
static void
Expand__st220prgadd(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  TN *c0 = Gen_Literal_TN(0LL, 4) ;
  Build_OP (	TOP_add_r_r_r,	o0,	i0,	i1,	ops) ;
  Build_OP (	TOP_prgadd_r_i,	c0, 	o0,	ops) ;
} /* Expand__st220prgadd */

/*
@@@  case INTRN_ST220PRGADD:
@@@    Expand__st220prgadd(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of st220prgset based on validated and scheduled basic assembly source.
*/
static void
Expand__st220prgset(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  TN *c0 = Gen_Literal_TN(0LL, 4) ;
  Build_OP (	TOP_add_r_r_r,	o0,	i0,	i1,	ops) ;
  Build_OP (	TOP_prgset_r_i,	c0,	o0,	ops) ;
} /* Expand__st220prgset */

/*
@@@  case INTRN_ST220PRGSET:
@@@    Expand__st220prgset(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of st220syscall based on validated and scheduled basic assembly source.
*/
static void
Expand__st220syscall(
 TN* i0,
 OPS* ops
)
{
    /*
      First, syscall expects an immediate argument. It cannot be generated another way.
      I don't know if this can be checked in upper layers. Range should also be checked.
      As the expansion is seen as an INTRINSIC_CALL, the value if constant is seen
      as rematerializable.
      Note that in hight optimization levels, some constant-propagation stuff may
      work thus enabling to write __syscall(s) where is a const int, but
      this does not work under low optimization levels (the generated WHIRL
      cannot be used to determine the constant)
      Finally, this intrinsic cannot be used when intrinsics are not inlined
      since it does not have a register interface and an equivalent callable function
      (does this make sense ?)
      Mabe this (having a syscall as a function) could be solved with some self-modifying-code hack ?
      Additional surprises include that 0 is a special case that actually happens.
      The case value is written but does not seem to be triggered
    */
    TOP top = TOP_UNDEFINED;
    if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_syscall_i)) {
      top = TOP_syscall_i;
    } else if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_st240_syscall_i)) {
      top = TOP_st240_syscall_i;
    }
    if (TN_is_rematerializable(i0)) {
	WN *wn = TN_remat(i0) ;
	if (WN_operator_is(wn, OPR_INTCONST)) {
	    TN *cunknown = Gen_Literal_TN(WN_const_val(wn), 4) ;
	    Build_OP (	top,	cunknown, 	ops) ;
	} else {
	    DevWarn("targinfo/st200/expand/exp_intrinsics.cxx::Expand__st220syscall: TN_is_rematerializable BUT WN_operator_is *not* OPR_INTCONST.") ;
	}
    } else if (TN_is_zero(i0)) { 
	// Using Zero_TN leads to try to select $r0
	TN *czero = Gen_Literal_TN(0LL, 4) ;
	Build_OP (	top,	czero, 	ops) ;
    } else if (TN_has_value(i0)) {
	TN *cunknown = Gen_Literal_TN(TN_value(i0), 4) ;
	Build_OP (	top,	cunknown, 	ops) ;
    } else {
	DevWarn("targinfo/st200/expand/exp_intrinsics.cxx::Expand__st220syscall: expect an immediate argument.") ;
    }
} /* Expand__st220syscall */

/*
@@@  case INTRN_ST220SYSCALL:
@@@    Expand__st220syscall(opnd[0],ops) ;
@@@  break ;
*/

/*
 * Expansion of absch based on validated and scheduled basic assembly source.
*/
static void
Expand__absch(
 TN* o0,
 TN* i0,
 OPS* ops
)
{
  if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_subs_ph_r_r_r)) { 
    TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_20_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    Build_OP (	TOP_subs_ph_r_r_r,	r0_20_0,	Zero_TN,	i0,	ops) ;
    Build_OP (	TOP_cmplt_r_r_b,	b0_0_0,	i0,	Zero_TN,	ops) ;
    Build_OP (	TOP_sxth_r_r,	r0_20_1,	r0_20_0,	ops) ;
    Build_OP (	TOP_targ_slct_r_r_b_r,	o0,	b0_0_0,	r0_20_1,	i0,	ops) ;
  } else {
    TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_0_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_20_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)                (((long long)(x)<<32) >> 32)
    TN *c0x7fff = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *c0x7fff_ltn = Gen_Literal_TN(__EXTS32TOS64(0x7fff), 4) ;
    Build_OP (    TOP_mov_ii_r,      c0x7fff,        c0x7fff_ltn,    ops) ;
    TN *c0xffff8000 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *c0xffff8000_ltn = Gen_Literal_TN(__EXTS32TOS64(0xffff8000), 4) ;
    Build_OP (    TOP_mov_ii_r,      c0xffff8000,    c0xffff8000_ltn,        ops) ;
#undef __EXTS32TOS64
    Build_OP (    TOP_cmpeq_r_r_b,  b0_0_0, i0,     c0xffff8000,    ops) ;
    Build_OP (    TOP_sub_r_r_r,      r0_20_0,        Zero_TN,        i0,     ops) ;
    Build_OP (    TOP_cmplt_r_r_b,  b0_0_1, i0,     Zero_TN,     ops) ;
    Build_OP (    TOP_slctf_r_r_b_r,    r0_20_1,        b0_0_0, r0_20_0,        c0x7fff,        ops) ;
    Build_OP (    TOP_targ_slct_r_r_b_r,     o0,     b0_0_1, r0_20_1,        i0,     ops) ;
  }
} /* Expand__absch */

/*
@@@  case INTRN_ABSCH:
@@@    Expand__absch(result[0],opnd[0],ops) ;
@@@  break ;
*/

/*
 * Expansion of abscl based on validated and scheduled basic assembly source.
*/
static void
Expand__abscl(
 TN* ol0,
 TN* oh0,
 TN* il0,
 TN* ih0,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_0_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_1_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_22_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_22_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_23_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)                (((long long)(x)<<32) >> 32)
  TN *c0x7fffffff = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *c0x7fffffff_ltn = Gen_Literal_TN(__EXTS32TOS64(0x7fffffff), 4) ;
  Build_OP (    TOP_mov_ii_r,      c0x7fffffff,    c0x7fffffff_ltn,        ops) ;
  TN *c0x80000000 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *c0x80000000_ltn = Gen_Literal_TN(__EXTS32TOS64(0x80000000), 4) ;
  Build_OP (    TOP_mov_ii_r,      c0x80000000,    c0x80000000_ltn,        ops) ;
  TN *c_1 = Gen_Literal_TN(__EXTS32TOS64(-1), 4) ;
#undef __EXTS32TOS64
  Build_OP (    TOP_cmpgtu_r_r_b, b0_0_0, il0,    Zero_TN,     ops) ;
  Build_OP (    TOP_sub_r_r_r,      r0_20_0,        Zero_TN,     il0,    ops) ;
  Build_OP (    TOP_sub_r_r_r,      r0_21_0,        Zero_TN,     ih0,    ops) ;
  Build_OP (    TOP_sub_r_i_r,      r0_22_0,        c_1,    ih0,    ops) ;
  Build_OP (    TOP_targ_slct_r_r_b_r,     r0_21_1,        b0_0_0, r0_22_0,        r0_21_0,        ops) ;
  Build_OP (    TOP_cmpeq_r_r_r,  r0_22_1,        ih0,    c0x80000000,    ops) ;
  Build_OP (    TOP_cmpeq_r_r_r,  r0_23_1,        il0,    Zero_TN,     ops) ;
  Build_OP (    TOP_andl_r_r_b,   b0_0_2, r0_22_1,        r0_23_1,        ops) ;
  Build_OP (    TOP_cmplt_r_r_b,  b0_1_2, ih0,    Zero_TN,     ops) ;
  Build_OP (    TOP_slctf_i_r_b_r,    r0_20_3,        b0_0_2, r0_20_0,        c_1,    ops) ;
  Build_OP (    TOP_slctf_r_r_b_r,    r0_21_3,        b0_0_2, r0_21_1,        c0x7fffffff,        ops) ;
  Build_OP (    TOP_targ_slct_r_r_b_r,     ol0,    b0_1_2, r0_20_3,        il0,    ops) ;
  Build_OP (    TOP_targ_slct_r_r_b_r,     oh0,    b0_1_2, r0_21_3,        ih0,    ops) ;
} /* Expand__abscl */


/*
@@@  case INTRN_ABSCL:
@@@    Expand__abscl(result[0],result[1],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of abscw based on validated and scheduled basic assembly source.
*/
static void
Expand__abscw(
 TN* o0,
 TN* i0,
 OPS* ops
)
{
  if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_subs_r_r_r)) { 
    TN *b0_1_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    Build_OP (	TOP_cmplt_r_r_b,	b0_1_0,	i0,	Zero_TN,	ops) ;
    Build_OP (	TOP_subs_r_r_r,	r0_20_0,	Zero_TN,	i0,	ops) ;
    Build_OP (	TOP_targ_slct_r_r_b_r,	o0,	b0_1_0,	r0_20_0,	i0,	ops) ;
  } else {
    TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_1_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_20_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)                (((long long)(x)<<32) >> 32)
    TN *c0x7fffffff = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *c0x7fffffff_ltn = Gen_Literal_TN(__EXTS32TOS64(0x7fffffff), 4) ;
    Build_OP (    TOP_mov_ii_r,      c0x7fffffff,    c0x7fffffff_ltn,        ops) ;
    TN *c0x80000000 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *c0x80000000_ltn = Gen_Literal_TN(__EXTS32TOS64(0x80000000), 4) ;
    Build_OP (    TOP_mov_ii_r,      c0x80000000,    c0x80000000_ltn,        ops) ;
#undef __EXTS32TOS64
    Build_OP (    TOP_cmpeq_r_r_b,  b0_0_0, i0,     c0x80000000,    ops) ;
    Build_OP (    TOP_cmplt_r_r_b,  b0_1_0, i0,     Zero_TN,     ops) ;
    Build_OP (    TOP_sub_r_r_r,      r0_20_0,        Zero_TN,        i0,     ops) ;
    Build_OP (    TOP_slctf_r_r_b_r,    r0_20_1,        b0_0_0, r0_20_0,        c0x7fffffff,    ops) ;
    Build_OP (    TOP_targ_slct_r_r_b_r,     o0,     b0_1_0, r0_20_1,        i0,     ops) ;
  }
} /* Expand__abscw */

/*
@@@  case INTRN_ABSCW:
@@@    Expand__abscw(result[0],opnd[0],ops) ;
@@@  break ;
*/

/*
 * Expansion of absh based on validated and scheduled basic assembly source.
*/
static void
Expand__absh(
 TN* o0,
 TN* i0,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  Build_OP (	TOP_cmplt_r_r_b,	b0_0_0,	i0,	Zero_TN,	ops) ;
  Build_OP (	TOP_sub_r_r_r,	r0_20_0,	Zero_TN,	i0,	ops) ;
  Build_OP (	TOP_targ_slct_r_r_b_r,	o0,	b0_0_0,	r0_20_0,	i0,	ops) ;
} /* Expand__absh */

/*
@@@  case INTRN_ABSH:
@@@    Expand__absh(result[0],opnd[0],ops) ;
@@@  break ;
*/

/*
 * Expansion of absl based on validated and scheduled basic assembly source.
*/
static void
Expand__absl(
 TN* ol0,
 TN* oh0,
 TN* il0,
 TN* ih0,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_1_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_20_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_22_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c_1 = Gen_Literal_TN(__EXTS32TOS64(-1), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_cmpgtu_r_r_b,	b0_0_0,	il0,	Zero_TN,	ops) ;
  Build_OP (	TOP_cmplt_r_r_b,	b0_1_0,	ih0,	Zero_TN,	ops) ;
  Build_OP (	TOP_sub_r_r_r,	r0_21_0,	Zero_TN,	ih0,	ops) ;
  Build_OP (	TOP_sub_r_i_r,	r0_22_0,	c_1,	ih0,	ops) ;
  Build_OP (	TOP_targ_slct_r_r_b_r,	r0_21_1,	b0_0_0,	r0_22_0,	r0_21_0,	ops) ;
  Build_OP (	TOP_sub_r_r_r,	r0_20_1,	Zero_TN,	il0,	ops) ;
  Build_OP (	TOP_targ_slct_r_r_b_r,	ol0,	b0_1_0,	r0_20_1,	il0,	ops) ;
  Build_OP (	TOP_targ_slct_r_r_b_r,	oh0,	b0_1_0,	r0_21_1,	ih0,	ops) ;
} /* Expand__absl */

/*
@@@  case INTRN_ABSL:
@@@    Expand__absl(result[0],result[1],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of absw based on validated and scheduled basic assembly source.
*/
static void
Expand__absw(
 TN* o0,
 TN* i0,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  Build_OP (	TOP_cmplt_r_r_b,	b0_0_0,	i0,	Zero_TN,	ops) ;
  Build_OP (	TOP_sub_r_r_r,	r0_20_0,	Zero_TN,	i0,	ops) ;
  Build_OP (	TOP_targ_slct_r_r_b_r,	o0,	b0_0_0,	r0_20_0,	i0,	ops) ;
} /* Expand__absw */

/*
@@@  case INTRN_ABSW:
@@@    Expand__absw(result[0],opnd[0],ops) ;
@@@  break ;
*/

/*
 * Expansion of addch based on validated and scheduled basic assembly source.
*/
static void
Expand__addch(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_adds_ph_r_r_r)) { 
    TN *r0_16_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    Build_OP (	TOP_adds_ph_r_r_r,	r0_16_0,	i0,	i1,	ops) ;
    Build_OP (	TOP_sxth_r_r,	o0,	r0_16_0,	ops) ;
  } else {
    TN *r0_16_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_16_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)                (((long long)(x)<<32) >> 32)
    TN *c0x7fff = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *c0x7fff_ltn = Gen_Literal_TN(__EXTS32TOS64(0x7fff), 4) ;
    Build_OP (    TOP_mov_ii_r,      c0x7fff,        c0x7fff_ltn,    ops) ;
    TN *c0xffff8000 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *c0xffff8000_ltn = Gen_Literal_TN(__EXTS32TOS64(0xffff8000), 4) ;
    Build_OP (    TOP_mov_ii_r,      c0xffff8000,    c0xffff8000_ltn,        ops) ;
#undef __EXTS32TOS64
    Build_OP (    TOP_add_r_r_r,      r0_16_0,        i0,     i1,     ops) ;
    Build_OP (    TOP_max_r_r_r,      r0_16_1,        r0_16_0,        c0xffff8000,    ops) ;
    Build_OP (    TOP_min_r_r_r,      o0,     r0_16_1,        c0x7fff,        ops) ;
  }
} /* Expand__addch */

/*
@@@  case INTRN_ADDCH:
@@@    Expand__addch(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of addcl based on validated and scheduled basic assembly source.
*/
static void
Expand__addcl(
 TN* ol0,
 TN* oh0,
 TN* il0,
 TN* ih0,
 TN* il1,
 TN* ih1,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_0_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_0_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_0_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_0_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_21_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_22_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_23_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_24_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_26_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_27_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_28_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_29_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)                (((long long)(x)<<32) >> 32)
  TN *c0x7fffffff = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *c0x7fffffff_ltn = Gen_Literal_TN(__EXTS32TOS64(0x7fffffff), 4) ;
  Build_OP (    TOP_mov_ii_r,      c0x7fffffff,    c0x7fffffff_ltn,        ops) ;
  TN *c31 = Gen_Literal_TN(__EXTS32TOS64(31), 4) ;
#undef __EXTS32TOS64
  Build_OP (    TOP_convib_r_b,        b0_0_0, Zero_TN,        ops) ;
  Build_OP (    TOP_targ_addcg_b_r_r_b_r,      r0_22_1,        b0_0_1, il0,    il1,    b0_0_0, ops) ;
  Build_OP (    TOP_shr_i_r_r,      r0_21_1,        ih0,    c31,    ops) ;
  Build_OP (    TOP_shr_i_r_r,      r0_23_1,        ih1,    c31,    ops) ;
  Build_OP (    TOP_targ_addcg_b_r_r_b_r,      r0_24_2,        b0_0_2, ih0,    ih1,    b0_0_1, ops) ;
  Build_OP (    TOP_orc_r_r_r,      r0_29_2,        r0_21_1,        Zero_TN,     ops) ;
  Build_OP (    TOP_sub_r_r_r,      r0_27_2,        c0x7fffffff,        r0_21_1,        ops) ;
  Build_OP (    TOP_shr_i_r_r,      r0_28_3,        r0_24_2,        c31,    ops) ;
  Build_OP (    TOP_targ_addcg_b_r_r_b_r,      r0_26_4,        b0_0_4, r0_21_1,        r0_23_1,        b0_0_2, ops) ;
  Build_OP (    TOP_cmpne_r_r_b,  b0_0_5, r0_28_3,        r0_26_4,        ops) ;
  Build_OP (    TOP_targ_slct_r_r_b_r,     ol0,    b0_0_5, r0_29_2,        r0_22_1,        ops) ;
  Build_OP (    TOP_targ_slct_r_r_b_r,     oh0,    b0_0_5, r0_27_2,        r0_24_2,        ops) ;
} /* Expand__addcl */

/*
@@@  case INTRN_ADDCL:
@@@    Expand__addcl(result[0],result[1],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
@@@  break ;
*/

/*
 * Expansion of addcw based on validated and scheduled basic assembly source.
*/
static void
Expand__addcw(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_adds_r_r_r)) { 
    Build_OP (    TOP_adds_r_r_r,     o0,     i0,     i1,     ops) ;
  } else {
    TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_0_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_0_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *b0_0_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *r0_20_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_21_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_21_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_22_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_23_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_24_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)                (((long long)(x)<<32) >> 32)
    TN *c0x7fffffff = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *c0x7fffffff_ltn = Gen_Literal_TN(__EXTS32TOS64(0x7fffffff), 4) ;
    Build_OP (    TOP_mov_ii_r,      c0x7fffffff,    c0x7fffffff_ltn,        ops) ;
    TN *c31 = Gen_Literal_TN(__EXTS32TOS64(31), 4) ;
#undef __EXTS32TOS64
    Build_OP (    TOP_convib_r_b,        b0_0_0, Zero_TN,        ops) ;
    Build_OP (    TOP_targ_addcg_b_r_r_b_r,      r0_20_1,        b0_0_1, i0,     i1,     b0_0_0, ops) ;
    Build_OP (    TOP_shr_i_r_r,      r0_21_1,        i0,     c31,    ops) ;
    Build_OP (    TOP_shr_i_r_r,      r0_22_1,        i1,     c31,    ops) ;
    Build_OP (    TOP_shr_i_r_r,      r0_23_2,        r0_20_1,        c31,    ops) ;
    Build_OP (    TOP_targ_addcg_b_r_r_b_r,      r0_24_2,        b0_0_2, r0_21_1,        r0_22_1,        b0_0_1, ops) ;
    Build_OP (    TOP_sub_r_r_r,      r0_21_2,        c0x7fffffff,        r0_21_1,        ops) ;
    Build_OP (    TOP_cmpeq_r_r_b,  b0_0_3, r0_23_2,        r0_24_2,        ops) ;
    Build_OP (    TOP_targ_slct_r_r_b_r,     o0,     b0_0_3, r0_20_1,        r0_21_2,        ops) ;
  }
} /* Expand__addcw */


/*
@@@  case INTRN_ADDCW:
@@@    Expand__addcw(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of addl based on validated and scheduled basic assembly source.
*/
static void
Expand__addl(
 TN* ol0,
 TN* oh0,
 TN* il0,
 TN* ih0,
 TN* il1,
 TN* ih1,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_0_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_0_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  Build_OP (	TOP_convib_r_b,	b0_0_0,	Zero_TN,	ops) ;
  Build_OP (	TOP_targ_addcg_b_r_r_b_r,	ol0,	b0_0_1,	il0,	il1,	b0_0_0,	ops) ;
  Build_OP (	TOP_targ_addcg_b_r_r_b_r,	oh0,	b0_0_2,	ih0,	ih1,	b0_0_1,	ops) ;
} /* Expand__addl */

/*
@@@  case INTRN_ADDL:
@@@    Expand__addl(result[0],result[1],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
@@@  break ;
*/

/*
 * Expansion of addul based on validated and scheduled basic assembly source.
*/
static void
Expand__addul(
 TN* ol0,
 TN* oh0,
 TN* il0,
 TN* ih0,
 TN* il1,
 TN* ih1,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_0_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_0_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  Build_OP (	TOP_convib_r_b,	b0_0_0,	Zero_TN,	ops) ;
  Build_OP (	TOP_targ_addcg_b_r_r_b_r,	ol0,	b0_0_1,	il0,	il1,	b0_0_0,	ops) ;
  Build_OP (	TOP_targ_addcg_b_r_r_b_r,	oh0,	b0_0_2,	ih0,	ih1,	b0_0_1,	ops) ;
} /* Expand__addul */

/*
@@@  case INTRN_ADDUL:
@@@    Expand__addul(result[0],result[1],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
@@@  break ;
*/

/*
 * Expansion of bitclrh based on validated and scheduled basic assembly source.
*/
static void
Expand__bitclrh(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_add_i_r_r,	r0_20_0,	Zero_TN,	c1,	ops) ;
  Build_OP (	TOP_shl_r_r_r,	r0_20_1,	r0_20_0,	i1,	ops) ;
  Build_OP (	TOP_andc_r_r_r,	o0,	r0_20_1,	i0,	ops) ;
} /* Expand__bitclrh */

/*
@@@  case INTRN_BITCLRH:
@@@    Expand__bitclrh(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of bitclrw based on validated and scheduled basic assembly source.
*/
static void
Expand__bitclrw(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_add_i_r_r,	r0_20_0,	Zero_TN,	c1,	ops) ;
  Build_OP (	TOP_shl_r_r_r,	r0_20_1,	r0_20_0,	i1,	ops) ;
  Build_OP (	TOP_andc_r_r_r,	o0,	r0_20_1,	i0,	ops) ;
} /* Expand__bitclrw */

/*
@@@  case INTRN_BITCLRW:
@@@    Expand__bitclrw(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of bitcnth based on validated and scheduled basic assembly source.
*/
static void
Expand__bitcnth(
 TN* o0,
 TN* i0,
 OPS* ops
)
{
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_10 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_7 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_8 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_6 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_9 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)                (((long long)(x)<<32) >> 32)
  TN *c0x0f0f = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *c0x0f0f_ltn = Gen_Literal_TN(__EXTS32TOS64(0x0f0f), 4) ;
  Build_OP (    TOP_mov_ii_r,      c0x0f0f,        c0x0f0f_ltn,    ops) ;
  TN *c0x1f = Gen_Literal_TN(__EXTS32TOS64(0x1f), 4) ;
  TN *c0x3333 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *c0x3333_ltn = Gen_Literal_TN(__EXTS32TOS64(0x3333), 4) ;
  Build_OP (    TOP_mov_ii_r,      c0x3333,        c0x3333_ltn,    ops) ;
  TN *c0x5555 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *c0x5555_ltn = Gen_Literal_TN(__EXTS32TOS64(0x5555), 4) ;
  Build_OP (    TOP_mov_ii_r,      c0x5555,        c0x5555_ltn,    ops) ;
  TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
  TN *c2 = Gen_Literal_TN(__EXTS32TOS64(2), 4) ;
  TN *c4 = Gen_Literal_TN(__EXTS32TOS64(4), 4) ;
  TN *c8 = Gen_Literal_TN(__EXTS32TOS64(8), 4) ;
#undef __EXTS32TOS64
  Build_OP (    TOP_shru_i_r_r,     r0_20_0,        i0,     c1,     ops) ;
  Build_OP (    TOP_and_r_r_r,      r0_20_1,        r0_20_0,        c0x5555,        ops) ;
  Build_OP (    TOP_sub_r_r_r,      r0_20_2,        i0,     r0_20_1,        ops) ;
  Build_OP (    TOP_shru_i_r_r,     r0_21_3,        r0_20_2,        c2,     ops) ;
  Build_OP (    TOP_and_r_r_r,      r0_20_3,        r0_20_2,        c0x3333,        ops) ;
  Build_OP (    TOP_and_r_r_r,      r0_21_4,        r0_21_3,        c0x3333,        ops) ;
  Build_OP (    TOP_add_r_r_r,      r0_20_5,        r0_20_3,        r0_21_4,        ops) ;
  Build_OP (    TOP_shru_i_r_r,     r0_21_6,        r0_20_5,        c4,     ops) ;
  Build_OP (    TOP_add_r_r_r,      r0_20_7,        r0_20_5,        r0_21_6,        ops) ;
  Build_OP (    TOP_and_r_r_r,      r0_20_8,        r0_20_7,        c0x0f0f,        ops) ;
  Build_OP (    TOP_shru_i_r_r,     r0_21_9,        r0_20_8,        c8,     ops) ;
  Build_OP (    TOP_add_r_r_r,      r0_20_10,       r0_20_8,        r0_21_9,        ops) ;
  Build_OP (    TOP_and_i_r_r,      o0,     r0_20_10,       c0x1f,  ops) ;
} /* Expand__bitcnth */

/*
@@@  case INTRN_BITCNTH:
@@@    Expand__bitcnth(result[0],opnd[0],ops) ;
@@@  break ;
*/

/*
 * Expansion of bitcntw based on validated and scheduled basic assembly source.
*/
static void
Expand__bitcntw(
 TN* o0,
 TN* i0,
 OPS* ops
)
{
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_10 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_12 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_7 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_8 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_11 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_6 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_9 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)                (((long long)(x)<<32) >> 32)
  TN *c0x0f0f0f0f = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *c0x0f0f0f0f_ltn = Gen_Literal_TN(__EXTS32TOS64(0x0f0f0f0f), 4) ;
  Build_OP (    TOP_mov_ii_r,      c0x0f0f0f0f,    c0x0f0f0f0f_ltn,        ops) ;
  TN *c0x33333333 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *c0x33333333_ltn = Gen_Literal_TN(__EXTS32TOS64(0x33333333), 4) ;
  Build_OP (    TOP_mov_ii_r,      c0x33333333,    c0x33333333_ltn,        ops) ;
  TN *c0x3f = Gen_Literal_TN(__EXTS32TOS64(0x3f), 4) ;
  TN *c0x55555555 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *c0x55555555_ltn = Gen_Literal_TN(__EXTS32TOS64(0x55555555), 4) ;
  Build_OP (    TOP_mov_ii_r,      c0x55555555,    c0x55555555_ltn,        ops) ;
  TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
  TN *c16 = Gen_Literal_TN(__EXTS32TOS64(16), 4) ;
  TN *c2 = Gen_Literal_TN(__EXTS32TOS64(2), 4) ;
  TN *c4 = Gen_Literal_TN(__EXTS32TOS64(4), 4) ;
  TN *c8 = Gen_Literal_TN(__EXTS32TOS64(8), 4) ;
#undef __EXTS32TOS64
  Build_OP (    TOP_shru_i_r_r,     r0_20_0,        i0,     c1,     ops) ;
  Build_OP (    TOP_and_r_r_r,      r0_20_1,        r0_20_0,        c0x55555555,    ops) ;
  Build_OP (    TOP_sub_r_r_r,      r0_20_2,        i0,     r0_20_1,        ops) ;
  Build_OP (    TOP_shru_i_r_r,     r0_21_3,        r0_20_2,        c2,     ops) ;
  Build_OP (    TOP_and_r_r_r,      r0_20_3,        r0_20_2,        c0x33333333,    ops) ;
  Build_OP (    TOP_and_r_r_r,      r0_21_4,        r0_21_3,        c0x33333333,    ops) ;
  Build_OP (    TOP_add_r_r_r,      r0_20_5,        r0_20_3,        r0_21_4,        ops) ;
  Build_OP (    TOP_shru_i_r_r,     r0_21_6,        r0_20_5,        c4,     ops) ;
  Build_OP (    TOP_add_r_r_r,      r0_20_7,        r0_20_5,        r0_21_6,        ops) ;
  Build_OP (    TOP_and_r_r_r,      r0_20_8,        r0_20_7,        c0x0f0f0f0f,    ops) ;
  Build_OP (    TOP_shru_i_r_r,     r0_21_9,        r0_20_8,        c8,     ops) ;
  Build_OP (    TOP_add_r_r_r,      r0_20_10,       r0_20_8,        r0_21_9,        ops) ;
  Build_OP (    TOP_shru_i_r_r,     r0_21_11,       r0_20_10,       c16,    ops) ;
  Build_OP (    TOP_add_r_r_r,      r0_20_12,       r0_20_10,       r0_21_11,       ops) ;
  Build_OP (    TOP_and_i_r_r,      o0,     r0_20_12,       c0x3f,  ops) ;
} /* Expand__bitcntw */

/*
@@@  case INTRN_BITCNTW:
@@@    Expand__bitcntw(result[0],opnd[0],ops) ;
@@@  break ;
*/

static void
Expand__bitcntl(
		TN * o0,
		TN * i0,
		TN * i1,
		OPS* ops
		)
{
  TN *tmp1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *tmp2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  Expand__bitcntw (tmp1, i0, ops);
  Expand__bitcntw (tmp2, i1, ops);
  Build_OP ( TOP_add_r_r_r, o0, tmp1, tmp2, ops);
}

/*
 * Expansion of bitnoth based on validated and scheduled basic assembly source.
*/
static void
Expand__bitnoth(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_add_i_r_r,	r0_20_0,	Zero_TN,	c1,	ops) ;
  Build_OP (	TOP_shl_r_r_r,	r0_20_1,	r0_20_0,	i1,	ops) ;
  Build_OP (	TOP_xor_r_r_r,	o0,	i0,	r0_20_1,	ops) ;
} /* Expand__bitnoth */

/*
@@@  case INTRN_BITNOTH:
@@@    Expand__bitnoth(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of bitnotw based on validated and scheduled basic assembly source.
*/
static void
Expand__bitnotw(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_add_i_r_r,	r0_20_0,	Zero_TN,	c1,	ops) ;
  Build_OP (	TOP_shl_r_r_r,	r0_20_1,	r0_20_0,	i1,	ops) ;
  Build_OP (	TOP_xor_r_r_r,	o0,	i0,	r0_20_1,	ops) ;
} /* Expand__bitnotw */

/*
@@@  case INTRN_BITNOTW:
@@@    Expand__bitnotw(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

static void
Expand__bitrevw(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_10 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_11 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_12 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_13 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_6 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_7 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_8 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_9 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_10 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_12 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_6 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_7 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_9 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c0x00FF00FF = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *c0x00FF00FF_ltn = Gen_Literal_TN(__EXTS32TOS64(0x00FF00FF), 4) ;
  Build_OP (	TOP_mov_ii_r,	c0x00FF00FF,	c0x00FF00FF_ltn,	ops) ;
  TN *c0x0F0F0F0F = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *c0x0F0F0F0F_ltn = Gen_Literal_TN(__EXTS32TOS64(0x0F0F0F0F), 4) ;
  Build_OP (	TOP_mov_ii_r,	c0x0F0F0F0F,	c0x0F0F0F0F_ltn,	ops) ;
  TN *c0x33333333 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *c0x33333333_ltn = Gen_Literal_TN(__EXTS32TOS64(0x33333333), 4) ;
  Build_OP (	TOP_mov_ii_r,	c0x33333333,	c0x33333333_ltn,	ops) ;
  TN *c0x55555555 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *c0x55555555_ltn = Gen_Literal_TN(__EXTS32TOS64(0x55555555), 4) ;
  Build_OP (	TOP_mov_ii_r,	c0x55555555,	c0x55555555_ltn,	ops) ;
  TN *c0xAAAAAAAA = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *c0xAAAAAAAA_ltn = Gen_Literal_TN(__EXTS32TOS64(0xAAAAAAAA), 4) ;
  Build_OP (	TOP_mov_ii_r,	c0xAAAAAAAA,	c0xAAAAAAAA_ltn,	ops) ;
  TN *c0xCCCCCCCC = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *c0xCCCCCCCC_ltn = Gen_Literal_TN(__EXTS32TOS64(0xCCCCCCCC), 4) ;
  Build_OP (	TOP_mov_ii_r,	c0xCCCCCCCC,	c0xCCCCCCCC_ltn,	ops) ;
  TN *c0xF0F0F0F0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *c0xF0F0F0F0_ltn = Gen_Literal_TN(__EXTS32TOS64(0xF0F0F0F0), 4) ;
  Build_OP (	TOP_mov_ii_r,	c0xF0F0F0F0,	c0xF0F0F0F0_ltn,	ops) ;
  TN *c0xFF00FF00 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *c0xFF00FF00_ltn = Gen_Literal_TN(__EXTS32TOS64(0xFF00FF00), 4) ;
  Build_OP (	TOP_mov_ii_r,	c0xFF00FF00,	c0xFF00FF00_ltn,	ops) ;
  TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
  TN *c16 = Gen_Literal_TN(__EXTS32TOS64(16), 4) ;
  TN *c2 = Gen_Literal_TN(__EXTS32TOS64(2), 4) ;
  TN *c4 = Gen_Literal_TN(__EXTS32TOS64(4), 4) ;
  TN *c8 = Gen_Literal_TN(__EXTS32TOS64(8), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_and_r_r_r,	r0_20_0,	i1,	c0xAAAAAAAA,	ops) ;
  Build_OP (	TOP_and_r_r_r,	r0_21_0,	i1,	c0x55555555,	ops) ;
  Build_OP (	TOP_shru_i_r_r,	r0_20_1,	r0_20_0,	c1,	ops) ;
  Build_OP (	TOP_shl_i_r_r,	r0_21_1,	r0_21_0,	c1,	ops) ;
  Build_OP (	TOP_or_r_r_r,	r0_20_2,	r0_20_1,	r0_21_1,	ops) ;
  Build_OP (	TOP_and_r_r_r,	r0_20_3,	r0_20_2,	c0xCCCCCCCC,	ops) ;
  Build_OP (	TOP_and_r_r_r,	r0_21_3,	r0_20_2,	c0x33333333,	ops) ;
  Build_OP (	TOP_shru_i_r_r,	r0_20_4,	r0_20_3,	c2,	ops) ;
  Build_OP (	TOP_shl_i_r_r,	r0_21_4,	r0_21_3,	c2,	ops) ;
  Build_OP (	TOP_or_r_r_r,	r0_20_5,	r0_20_4,	r0_21_4,	ops) ;
  Build_OP (	TOP_and_r_r_r,	r0_20_6,	r0_20_5,	c0xF0F0F0F0,	ops) ;
  Build_OP (	TOP_and_r_r_r,	r0_21_6,	r0_20_5,	c0x0F0F0F0F,	ops) ;
  Build_OP (	TOP_shru_i_r_r,	r0_20_7,	r0_20_6,	c4,	ops) ;
  Build_OP (	TOP_shl_i_r_r,	r0_21_7,	r0_21_6,	c4,	ops) ;
  Build_OP (	TOP_or_r_r_r,	r0_20_8,	r0_20_7,	r0_21_7,	ops) ;
  Build_OP (	TOP_and_r_r_r,	r0_20_9,	r0_20_8,	c0xFF00FF00,	ops) ;
  Build_OP (	TOP_and_r_r_r,	r0_21_9,	r0_20_8,	c0x00FF00FF,	ops) ;
  Build_OP (	TOP_shru_i_r_r,	r0_20_10,	r0_20_9,	c8,	ops) ;
  Build_OP (	TOP_shl_i_r_r,	r0_21_10,	r0_21_9,	c8,	ops) ;
  Build_OP (	TOP_or_r_r_r,	r0_20_11,	r0_20_10,	r0_21_10,	ops) ;
  Build_OP (	TOP_shru_i_r_r,	r0_20_12,	r0_20_11,	c16,	ops) ;
  Build_OP (	TOP_shl_i_r_r,	r0_21_12,	r0_20_11,	c16,	ops) ;
  Build_OP (	TOP_or_r_r_r,	r0_20_13,	r0_20_12,	r0_21_12,	ops) ;
  Build_OP (	TOP_shru_r_r_r,	o0,	r0_20_13,	i0,	ops) ;
} /* Expand__bitrevw */

/*
@@@  case INTRN_BITREVW:
@@@    Expand__bitrevw(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of bitseth based on validated and scheduled basic assembly source.
*/
static void
Expand__bitseth(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_add_i_r_r,	r0_20_0,	Zero_TN,	c1,	ops) ;
  Build_OP (	TOP_shl_r_r_r,	r0_20_1,	r0_20_0,	i1,	ops) ;
  Build_OP (	TOP_or_r_r_r,	o0,	i0,	r0_20_1,	ops) ;
} /* Expand__bitseth */

/*
@@@  case INTRN_BITSETH:
@@@    Expand__bitseth(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of bitsetw based on validated and scheduled basic assembly source.
*/
static void
Expand__bitsetw(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_add_i_r_r,	r0_20_0,	Zero_TN,	c1,	ops) ;
  Build_OP (	TOP_shl_r_r_r,	r0_20_1,	r0_20_0,	i1,	ops) ;
  Build_OP (	TOP_or_r_r_r,	o0,	i0,	r0_20_1,	ops) ;
} /* Expand__bitsetw */

/*
@@@  case INTRN_BITSETW:
@@@    Expand__bitsetw(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of bitvalh based on validated and scheduled basic assembly source.
*/
static void
Expand__bitvalh(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_shru_r_r_r,	r0_20_0,	i0,	i1,	ops) ;
  Build_OP (	TOP_and_i_r_r,	o0,	r0_20_0,	c1,	ops) ;
} /* Expand__bitvalh */

/*
@@@  case INTRN_BITVALH:
@@@    Expand__bitvalh(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of bitvalw based on validated and scheduled basic assembly source.
*/
static void
Expand__bitvalw(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_shru_r_r_r,	r0_20_0,	i0,	i1,	ops) ;
  Build_OP (	TOP_and_i_r_r,	o0,	r0_20_0,	c1,	ops) ;
} /* Expand__bitvalw */

/*
@@@  case INTRN_BITVALW:
@@@    Expand__bitvalw(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of clamplw based on validated and scheduled basic assembly source.
*/
static void
Expand__clamplw(
 TN* o0,
 TN* il0,
 TN* ih0,
 OPS* ops
)
{
  TN *b0_1_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_23_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)                (((long long)(x)<<32) >> 32)
  TN *c0x7fffffff = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *c0x7fffffff_ltn = Gen_Literal_TN(__EXTS32TOS64(0x7fffffff), 4) ;
  Build_OP (    TOP_mov_ii_r,      c0x7fffffff,    c0x7fffffff_ltn,        ops) ;
  TN *c31 = Gen_Literal_TN(__EXTS32TOS64(31), 4) ;
#undef __EXTS32TOS64
  Build_OP (    TOP_cmplt_r_r_r,  r0_23_0,        ih0,    Zero_TN,     ops) ;
  Build_OP (    TOP_shr_i_r_r,      r0_21_0,        il0,    c31,    ops) ;
  Build_OP (    TOP_add_r_r_r,      r0_20_1,        c0x7fffffff,        r0_23_0,        ops) ;
  Build_OP (    TOP_cmpne_r_r_b,  b0_1_1, r0_21_0,        ih0,    ops) ;
  Build_OP (    TOP_targ_slct_r_r_b_r,     o0,     b0_1_1, r0_20_1,        il0,    ops) ;
} /* Expand__clamplw */


/*
@@@  case INTRN_CLAMPLW:
@@@    Expand__clamplw(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of clampwh based on validated and scheduled basic assembly source.
*/
static void
Expand__clampwh(
 TN* o0,
 TN* i0,
 OPS* ops
)
{
  if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_sats_r_r)) { 
    Build_OP (	TOP_sats_r_r,	o0,	i0,	ops) ;
  } else {
    TN *r0_16_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
    TN *c0x7fff = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *c0x7fff_ltn = Gen_Literal_TN(__EXTS32TOS64(0x7fff), 4) ;
    Build_OP (	TOP_mov_ii_r,	c0x7fff,	c0x7fff_ltn,	ops) ;
    TN *c0xffff8000 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *c0xffff8000_ltn = Gen_Literal_TN(__EXTS32TOS64(0xffff8000), 4) ;
    Build_OP (	TOP_mov_ii_r,	c0xffff8000,	c0xffff8000_ltn,	ops) ;
#undef __EXTS32TOS64
    Build_OP (	TOP_max_r_r_r,	r0_16_0,	i0,	c0xffff8000,	ops) ;
    Build_OP (	TOP_min_r_r_r,	o0,	r0_16_0,	c0x7fff,	ops) ;
  }
} /* Expand__clampwh */

/*
@@@  case INTRN_CLAMPWH:
@@@    Expand__clampwh(result[0],opnd[0],ops) ;
@@@  break ;
*/

/*
 * Expansion of ctzl based on validated and scheduled basic assembly source.
*/
static void
Expand__ctzl(
 TN* o0,
 TN* il0,
 TN* ih0,
 OPS* ops
)
{
  TN *b0_0_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_1_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_1_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_1_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_16_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_16_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_16_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_16_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_17_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_17_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_18_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_19_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)                (((long long)(x)<<32) >> 32)
  TN *c32 = Gen_Literal_TN(__EXTS32TOS64(32), 4) ;
  TN *c64 = Gen_Literal_TN(__EXTS32TOS64(64), 4) ;
  TN *c_1 = Gen_Literal_TN(__EXTS32TOS64(-1), 4) ;
#undef __EXTS32TOS64
  Build_OP (    TOP_convib_r_b, b0_1_0, Zero_TN,        ops) ;
  Build_OP (    TOP_mov_i_r,    r0_20_0,        c_1,    ops) ;
  Build_OP (    TOP_targ_addcg_b_r_r_b_r,       r0_19_1,        b0_1_1, il0,    r0_20_0,        b0_1_0, ops) ;
  Build_OP (    TOP_targ_addcg_b_r_r_b_r,       r0_18_2,        b0_1_2, ih0,    r0_20_0,        b0_1_1, ops) ;
  Build_OP (    TOP_andc_r_r_r, r0_16_2,        il0,    r0_19_1,        ops) ;
  Build_OP (    TOP_clz_r_r,    r0_16_3,        r0_16_2,        ops) ;
  Build_OP (    TOP_andc_r_r_r, r0_17_3,        ih0,    r0_18_2,        ops) ;
  Build_OP (    TOP_convib_r_b, b0_0_4, r0_17_3,        ops) ;
  Build_OP (    TOP_clz_r_r,    r0_17_4,        r0_17_3,        ops) ;
  Build_OP (    TOP_add_i_r_r,  r0_16_4,        r0_16_3,        c32,    ops) ;
  Build_OP (    TOP_slctf_r_r_b_r, r0_16_5,        b0_0_4, r0_16_4,        r0_17_4,        ops) ;
  Build_OP (    TOP_sub_r_i_r,  o0,     c64,    r0_16_5,        ops) ;
} /* Expand__ctzl */
 
/*
@@@  case INTRN_CTZ64:
@@@    Expand__ctzl(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of ctzw based on validated and scheduled basic assembly source.
*/
static void
Expand__ctzw(
 TN* o0,
 TN* i0,
 OPS* ops
)
{
  TN *r0_16_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_16_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_17_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)                (((long long)(x)<<32) >> 32)
  TN *c32 = Gen_Literal_TN(__EXTS32TOS64(32), 4) ;
  TN *c_1 = Gen_Literal_TN(__EXTS32TOS64(-1), 4) ;
#undef __EXTS32TOS64
  Build_OP (    TOP_add_i_r_r,  r0_17_0,        i0,     c_1,    ops) ;
  Build_OP (    TOP_andc_r_r_r, r0_16_1,        i0,     r0_17_0,        ops) ;
  Build_OP (    TOP_clz_r_r,    r0_16_2,        r0_16_1,        ops) ;
  Build_OP (    TOP_sub_r_i_r,  o0,     c32,    r0_16_2,        ops) ;
} /* Expand__ctzw */
 
/*
@@@  case INTRN_CTZ32:
@@@    Expand__ctzw(result[0],opnd[0],ops) ;
@@@  break ;
*/

/*
 * Expansion of disth based on validated and scheduled basic assembly source.
*/
static void
Expand__disth(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  Build_OP (	TOP_cmple_r_r_b,	b0_0_0,	i0,	i1,	ops) ;
  Build_OP (	TOP_sub_r_r_r,	r0_20_0,	i0,	i1,	ops) ;
  Build_OP (	TOP_sub_r_r_r,	r0_21_0,	i1,	i0,	ops) ;
  Build_OP (	TOP_targ_slct_r_r_b_r,	o0,	b0_0_0,	r0_21_0,	r0_20_0,	ops) ;
} /* Expand__disth */

/*
@@@  case INTRN_DISTH:
@@@    Expand__disth(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of distuh based on validated and scheduled basic assembly source.
*/
static void
Expand__distuh(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  Build_OP (	TOP_cmpleu_r_r_b,	b0_0_0,	i0,	i1,	ops) ;
  Build_OP (	TOP_sub_r_r_r,	r0_20_0,	i0,	i1,	ops) ;
  Build_OP (	TOP_sub_r_r_r,	r0_21_0,	i1,	i0,	ops) ;
  Build_OP (	TOP_targ_slct_r_r_b_r,	o0,	b0_0_0,	r0_21_0,	r0_20_0,	ops) ;
} /* Expand__distuh */

/*
@@@  case INTRN_DISTUH:
@@@    Expand__distuh(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of distuw based on validated and scheduled basic assembly source.
*/
static void
Expand__distuw(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  Build_OP (	TOP_cmpleu_r_r_b,	b0_0_0,	i0,	i1,	ops) ;
  Build_OP (	TOP_sub_r_r_r,	r0_20_0,	i0,	i1,	ops) ;
  Build_OP (	TOP_sub_r_r_r,	r0_21_0,	i1,	i0,	ops) ;
  Build_OP (	TOP_targ_slct_r_r_b_r,	o0,	b0_0_0,	r0_21_0,	r0_20_0,	ops) ;
} /* Expand__distuw */

/*
@@@  case INTRN_DISTUW:
@@@    Expand__distuw(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of distw based on validated and scheduled basic assembly source.
*/
static void
Expand__distw(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  Build_OP (	TOP_cmple_r_r_b,	b0_0_0,	i0,	i1,	ops) ;
  Build_OP (	TOP_sub_r_r_r,	r0_20_0,	i0,	i1,	ops) ;
  Build_OP (	TOP_sub_r_r_r,	r0_21_0,	i1,	i0,	ops) ;
  Build_OP (	TOP_targ_slct_r_r_b_r,	o0,	b0_0_0,	r0_21_0,	r0_20_0,	ops) ;
} /* Expand__distw */

/*
@@@  case INTRN_DISTW:
@@@    Expand__distw(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of edgesh based on validated and scheduled basic assembly source.
*/
static void
Expand__edgesh(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)                (((long long)(x)<<32) >> 32)
  TN *c0x7fff = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *c0x7fff_ltn = Gen_Literal_TN(__EXTS32TOS64(0x7fff), 4) ;
  Build_OP (    TOP_mov_ii_r,      c0x7fff,        c0x7fff_ltn,    ops) ;
  TN *c0xffff = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *c0xffff_ltn = Gen_Literal_TN(__EXTS32TOS64(0xffff), 4) ;
  Build_OP (    TOP_mov_ii_r,      c0xffff,        c0xffff_ltn,    ops) ;
  TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
#undef __EXTS32TOS64
  Build_OP (    TOP_shru_i_r_r,     r0_20_0,        i1,     c1,     ops) ;
  Build_OP (    TOP_xor_r_r_r,      r0_20_1,        i1,     r0_20_0,        ops) ;
  Build_OP (    TOP_and_r_r_r,      r0_20_2,        r0_20_1,        c0x7fff,        ops) ;
  Build_OP (    TOP_and_r_r_r,      r0_21_2,        i0,     c0xffff,        ops) ;
  Build_OP (    TOP_or_r_r_r,       o0,     r0_21_2,        r0_20_2,        ops) ;
} /* Expand__edgesh */

/*
@@@  case INTRN_EDGESH:
@@@    Expand__edgesh(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of edgesw based on validated and scheduled basic assembly source.
*/
static void
Expand__edgesw(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)                (((long long)(x)<<32) >> 32)
  TN *c0x7fffffff = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *c0x7fffffff_ltn = Gen_Literal_TN(__EXTS32TOS64(0x7fffffff), 4) ;
  Build_OP (    TOP_mov_ii_r,      c0x7fffffff,    c0x7fffffff_ltn,        ops) ;
  TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
#undef __EXTS32TOS64
  Build_OP (    TOP_shru_i_r_r,     r0_20_0,        i1,     c1,     ops) ;
  Build_OP (    TOP_xor_r_r_r,      r0_20_1,        i1,     r0_20_0,        ops) ;
  Build_OP (    TOP_and_r_r_r,      r0_20_2,        r0_20_1,        c0x7fffffff,    ops) ;
  Build_OP (    TOP_or_r_r_r,       o0,     i0,     r0_20_2,        ops) ;
} /* Expand__edgesw */


/*
@@@  case INTRN_EDGESW:
@@@    Expand__edgesw(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of eql based on validated and scheduled basic assembly source.
*/
static void
Expand__eql(
 TN* o0,
 TN* il0,
 TN* ih0,
 TN* il1,
 TN* ih1,
 OPS* ops
)
{
  TN *r0_16_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_17_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  Build_OP (	TOP_cmpeq_r_r_r,	r0_16_0,	il0,	il1,	ops) ;
  Build_OP (	TOP_cmpeq_r_r_r,	r0_17_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_andl_r_r_r,	o0,	r0_16_0,	r0_17_0,	ops) ;
} /* Expand__eql */

/*
@@@  case INTRN_EQL:
@@@    Expand__eql(result[0],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
@@@  break ;
*/

/*
 * Expansion of equl based on validated and scheduled basic assembly source.
*/
static void
Expand__equl(
 TN* o0,
 TN* il0,
 TN* ih0,
 TN* il1,
 TN* ih1,
 OPS* ops
)
{
  TN *r0_16_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_17_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  Build_OP (	TOP_cmpeq_r_r_r,	r0_16_0,	il0,	il1,	ops) ;
  Build_OP (	TOP_cmpeq_r_r_r,	r0_17_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_andl_r_r_r,	o0,	r0_16_0,	r0_17_0,	ops) ;
} /* Expand__equl */

/*
@@@  case INTRN_EQUL:
@@@    Expand__equl(result[0],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
@@@  break ;
*/

/*
 * Expansion of ffsl based on validated and scheduled basic assembly source.
*/
static void
Expand__ffsl(
 TN* o0,
 TN* il0,
 TN* ih0,
 OPS* ops
)
{
  TN *b0_0_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_1_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_16_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_16_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_16_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_16_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_17_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_17_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_18_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_18_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_19_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)                (((long long)(x)<<32) >> 32)
  TN *c32 = Gen_Literal_TN(__EXTS32TOS64(32), 4) ;
  TN *c64 = Gen_Literal_TN(__EXTS32TOS64(64), 4) ;
  TN *c_1 = Gen_Literal_TN(__EXTS32TOS64(-1), 4) ;
#undef __EXTS32TOS64
  Build_OP (    TOP_sub_r_r_r,  r0_19_0,        Zero_TN,        il0,    ops) ;
  Build_OP (    TOP_sub_r_r_r,  r0_20_0,        Zero_TN,        ih0,    ops) ;
  Build_OP (    TOP_sub_r_i_r,  r0_18_0,        c_1,    ih0,    ops) ;
  Build_OP (    TOP_cmpgtu_r_r_b,       b0_1_0, il0,    Zero_TN,        ops) ;
  Build_OP (    TOP_targ_slct_r_r_b_r,  r0_18_1,        b0_1_0, r0_18_0,        r0_20_0,        ops) ;
  Build_OP (    TOP_and_r_r_r,  r0_16_1,        il0,    r0_19_0,        ops) ;
  Build_OP (    TOP_clz_r_r,    r0_16_2,        r0_16_1,        ops) ;
  Build_OP (    TOP_and_r_r_r,  r0_17_2,        ih0,    r0_18_1,        ops) ;
  Build_OP (    TOP_convib_r_b, b0_0_3, r0_17_2,        ops) ;
  Build_OP (    TOP_clz_r_r,    r0_17_3,        r0_17_2,        ops) ;
  Build_OP (    TOP_add_i_r_r,  r0_16_3,        r0_16_2,        c32,    ops) ;
  Build_OP (    TOP_slctf_r_r_b_r, r0_16_4,        b0_0_3, r0_16_3,        r0_17_3,        ops) ;
  Build_OP (    TOP_sub_r_i_r,  o0,     c64,    r0_16_4,        ops) ;
} /* Expand__ffsl */
 
/*
@@@  case INTRN_FFS64:
@@@    Expand__ffsl(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of ffsw based on validated and scheduled basic assembly source.
*/
static void
Expand__ffsw(
 TN* o0,
 TN* i0,
 OPS* ops
)
{
  TN *r0_16_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_16_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_17_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)                (((long long)(x)<<32) >> 32)
  TN *c32 = Gen_Literal_TN(__EXTS32TOS64(32), 4) ;
#undef __EXTS32TOS64
  Build_OP (    TOP_sub_r_r_r,  r0_17_0,        Zero_TN,        i0,     ops) ;
  Build_OP (    TOP_and_r_r_r,  r0_16_1,        i0,     r0_17_0,        ops) ;
  Build_OP (    TOP_clz_r_r,    r0_16_2,        r0_16_1,        ops) ;
  Build_OP (    TOP_sub_r_i_r,  o0,     c32,    r0_16_2,        ops) ;
} /* Expand__ffsw */
 
/*
@@@  case INTRN_FFS32:
@@@    Expand__ffsw(result[0],opnd[0],ops) ;
@@@  break ;
*/

static void
Expand__isnanf(
 TN* o0,
 TN* i0,
 OPS* ops
)
{
  TN *t1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)                (((long long)(x)<<32) >> 32)
  TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
  TN *c0xff000000_ltn = Gen_Literal_TN(__EXTS32TOS64(0xff000000), 4) ;
  TN *c0xff000000 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  Build_OP (    TOP_mov_ii_r,      c0xff000000,        c0xff000000_ltn,    ops) ;
#undef __EXTS32TOS64
  Build_OP (    TOP_shl_i_r_r,  t1,      i0,    c1,     ops) ;
  Build_OP (    TOP_cmpgtu_r_r_r, o0,    t1,    c0xff000000, ops) ;
} /* Expand__isnanf */
 
/*
@@@  case INTRN_ISNANF:
@@@    Expand__isnanf(result[0],opnd[0],ops) ;
@@@  break ;
*/

static void Expand__gtul(TN*, TN*, TN*, TN*, TN*, OPS*);

static void
Expand__isnand(
 TN* o0,
 TN* il0,
 TN* ih0,
 OPS* ops
)
{
  TN *t1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)                (((long long)(x)<<32) >> 32)
  TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
  TN *c0xffc00000_ltn = Gen_Literal_TN(__EXTS32TOS64(0xffc00000), 4) ;
  TN *c0xffc00000 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  Build_OP (    TOP_mov_ii_r,      c0xffc00000,        c0xffc00000_ltn,    ops) ;
#undef __EXTS32TOS64
  Build_OP (    TOP_shl_i_r_r,  t1,     ih0,    c1,     ops) ;
  Expand__gtul (o0, il0, t1, Zero_TN, c0xffc00000, ops) ;
} /* Expand__isnand */
 
/*
@@@  case INTRN_ISNAND:
@@@    Expand__isnand(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

static void
Expand__isinff(
 TN* o0,
 TN* i0,
 OPS* ops
)
{
  TN *t1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)                (((long long)(x)<<32) >> 32)
  TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
  TN *c0xff000000_ltn = Gen_Literal_TN(__EXTS32TOS64(0xff000000), 4) ;
  TN *c0xff000000 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  Build_OP (    TOP_mov_ii_r,      c0xff000000,        c0xff000000_ltn,    ops) ;
#undef __EXTS32TOS64
  Build_OP (    TOP_shl_i_r_r,   t1,   i0,    c1,     ops) ;
  Build_OP (    TOP_cmpeq_r_r_r, o0,   t1,    c0xff000000,  ops) ;
} /* Expand__isinff */
 
/*
@@@  case INTRN_ISINFF:
@@@    Expand__isinff(result[0],opnd[0],ops) ;
@@@  break ;
*/

/*
 * Expansion of isinfd based on validated and scheduled basic assembly source.
*/
static void
Expand__isinfd(
 TN* o0,
 TN* il0,
 TN* ih0,
 OPS* ops
)
{
  TN *t1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)                (((long long)(x)<<32) >> 32)
  TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
  TN *c0xffc00000_ltn = Gen_Literal_TN(__EXTS32TOS64(0xffc00000), 4) ;
  TN *c0xffc00000 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  Build_OP (    TOP_mov_ii_r,      c0xffc00000,        c0xffc00000_ltn,    ops) ;
#undef __EXTS32TOS64
  Build_OP (    TOP_shl_i_r_r,  t1,     ih0,    c1,     ops) ;
  Expand__eql (o0, il0, t1, Zero_TN, c0xffc00000,   ops) ;
} /* Expand__isinfd */
 
/*
@@@  case INTRN_ISINFD:
@@@    Expand__isinfd(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of gel based on validated and scheduled basic assembly source.
*/
static void
Expand__gel(
 TN* o0,
 TN* il0,
 TN* ih0,
 TN* il1,
 TN* ih1,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_16_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_17_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  Build_OP (	TOP_cmpgeu_r_r_r,	r0_16_0,	il0,	il1,	ops) ;
  Build_OP (	TOP_cmpge_r_r_r,	r0_17_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_cmpeq_r_r_b,	b0_0_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_targ_slct_r_r_b_r,	o0,	b0_0_0,	r0_16_0,	r0_17_0,	ops) ;
} /* Expand__gel */

/*
@@@  case INTRN_GEL:
@@@    Expand__gel(result[0],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
@@@  break ;
*/

/*
 * Expansion of gethh based on validated and scheduled basic assembly source.
*/
static void
Expand__gethh(
 TN* o0,
 TN* i0,
 OPS* ops
)
{
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c16 = Gen_Literal_TN(__EXTS32TOS64(16), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_shr_i_r_r,	o0,	i0,	c16,	ops) ;
} /* Expand__gethh */

/*
@@@  case INTRN_GETHH:
@@@    Expand__gethh(result[0],opnd[0],ops) ;
@@@  break ;
*/

/*
 * Expansion of gethw based on validated and scheduled basic assembly source.
*/
static void
Expand__gethw(
 TN* o0,
 TN* il0,
 TN* ih0,
 OPS* ops
)
{
  Build_OP (	TOP_add_r_r_r,	o0,	ih0,	Zero_TN,	ops) ;
} /* Expand__gethw */

/*
@@@  case INTRN_GETHW:
@@@    Expand__gethw(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of getlh based on validated and scheduled basic assembly source.
*/
static void
Expand__getlh(
 TN* o0,
 TN* i0,
 OPS* ops
)
{
  Build_OP (	TOP_sxth_r_r,	o0,	i0,	ops) ;
} /* Expand__getlh */

/*
@@@  case INTRN_GETLH:
@@@    Expand__getlh(result[0],opnd[0],ops) ;
@@@  break ;
*/

/*
 * Expansion of getlw based on validated and scheduled basic assembly source.
*/
static void
Expand__getlw(
 TN* o0,
 TN* il0,
 TN* ih0,
 OPS* ops
)
{
  Build_OP (	TOP_add_r_r_r,	o0,	il0,	Zero_TN,	ops) ;
} /* Expand__getlw */

/*
@@@  case INTRN_GETLW:
@@@    Expand__getlw(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of geul based on validated and scheduled basic assembly source.
*/
static void
Expand__geul(
 TN* o0,
 TN* il0,
 TN* ih0,
 TN* il1,
 TN* ih1,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_16_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_17_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  Build_OP (	TOP_cmpgeu_r_r_r,	r0_16_0,	il0,	il1,	ops) ;
  Build_OP (	TOP_cmpgeu_r_r_r,	r0_17_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_cmpeq_r_r_b,	b0_0_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_targ_slct_r_r_b_r,	o0,	b0_0_0,	r0_16_0,	r0_17_0,	ops) ;
} /* Expand__geul */

/*
@@@  case INTRN_GEUL:
@@@    Expand__geul(result[0],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
@@@  break ;
*/

/*
 * Expansion of gtl based on validated and scheduled basic assembly source.
*/
static void
Expand__gtl(
 TN* o0,
 TN* il0,
 TN* ih0,
 TN* il1,
 TN* ih1,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_16_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_17_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  Build_OP (	TOP_cmpgtu_r_r_r,	r0_16_0,	il0,	il1,	ops) ;
  Build_OP (	TOP_cmpgt_r_r_r,	r0_17_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_cmpeq_r_r_b,	b0_0_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_targ_slct_r_r_b_r,	o0,	b0_0_0,	r0_16_0,	r0_17_0,	ops) ;
} /* Expand__gtl */

/*
@@@  case INTRN_GTL:
@@@    Expand__gtl(result[0],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
@@@  break ;
*/

/*
 * Expansion of gtul based on validated and scheduled basic assembly source.
*/
static void
Expand__gtul(
 TN* o0,
 TN* il0,
 TN* ih0,
 TN* il1,
 TN* ih1,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_16_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_17_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  Build_OP (	TOP_cmpgtu_r_r_r,	r0_16_0,	il0,	il1,	ops) ;
  Build_OP (	TOP_cmpgtu_r_r_r,	r0_17_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_cmpeq_r_r_b,	b0_0_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_targ_slct_r_r_b_r,	o0,	b0_0_0,	r0_16_0,	r0_17_0,	ops) ;
} /* Expand__gtul */

/*
@@@  case INTRN_GTUL:
@@@    Expand__gtul(result[0],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
@@@  break ;
*/

/*
 * Expansion of insequw based on validated and scheduled basic assembly source.
*/
static void
Expand__insequw(
 TN* o0,
 TN* i0,
 TN* i1,
 TN* i2,
 OPS* ops
)
{
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_shl_i_r_r,	r0_20_0,	i0,	c1,	ops) ;
  Build_OP (	TOP_cmpeq_r_r_r,	r0_21_0,	i1,	i2,	ops) ;
  Build_OP (	TOP_or_r_r_r,	o0,	r0_20_0,	r0_21_0,	ops) ;
} /* Expand__insequw */

/*
@@@  case INTRN_INSEQUW:
@@@    Expand__insequw(result[0],opnd[0],opnd[1],opnd[2],ops) ;
@@@  break ;
*/

/*
 * Expansion of inseqw based on validated and scheduled basic assembly source.
*/
static void
Expand__inseqw(
 TN* o0,
 TN* i0,
 TN* i1,
 TN* i2,
 OPS* ops
)
{
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_shl_i_r_r,	r0_20_0,	i0,	c1,	ops) ;
  Build_OP (	TOP_cmpeq_r_r_r,	r0_21_0,	i1,	i2,	ops) ;
  Build_OP (	TOP_or_r_r_r,	o0,	r0_20_0,	r0_21_0,	ops) ;
} /* Expand__inseqw */

/*
@@@  case INTRN_INSEQW:
@@@    Expand__inseqw(result[0],opnd[0],opnd[1],opnd[2],ops) ;
@@@  break ;
*/

/*
 * Expansion of insgeuw based on validated and scheduled basic assembly source.
*/
static void
Expand__insgeuw(
 TN* o0,
 TN* i0,
 TN* i1,
 TN* i2,
 OPS* ops
)
{
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_shl_i_r_r,	r0_20_0,	i0,	c1,	ops) ;
  Build_OP (	TOP_cmpgeu_r_r_r,	r0_21_0,	i1,	i2,	ops) ;
  Build_OP (	TOP_or_r_r_r,	o0,	r0_20_0,	r0_21_0,	ops) ;
} /* Expand__insgeuw */

/*
@@@  case INTRN_INSGEUW:
@@@    Expand__insgeuw(result[0],opnd[0],opnd[1],opnd[2],ops) ;
@@@  break ;
*/

/*
 * Expansion of insgew based on validated and scheduled basic assembly source.
*/
static void
Expand__insgew(
 TN* o0,
 TN* i0,
 TN* i1,
 TN* i2,
 OPS* ops
)
{
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_shl_i_r_r,	r0_20_0,	i0,	c1,	ops) ;
  Build_OP (	TOP_cmpge_r_r_r,	r0_21_0,	i1,	i2,	ops) ;
  Build_OP (	TOP_or_r_r_r,	o0,	r0_20_0,	r0_21_0,	ops) ;
} /* Expand__insgew */

/*
@@@  case INTRN_INSGEW:
@@@    Expand__insgew(result[0],opnd[0],opnd[1],opnd[2],ops) ;
@@@  break ;
*/

/*
 * Expansion of insgtuw based on validated and scheduled basic assembly source.
*/
static void
Expand__insgtuw(
 TN* o0,
 TN* i0,
 TN* i1,
 TN* i2,
 OPS* ops
)
{
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_shl_i_r_r,	r0_20_0,	i0,	c1,	ops) ;
  Build_OP (	TOP_cmpgtu_r_r_r,	r0_21_0,	i1,	i2,	ops) ;
  Build_OP (	TOP_or_r_r_r,	o0,	r0_20_0,	r0_21_0,	ops) ;
} /* Expand__insgtuw */

/*
@@@  case INTRN_INSGTUW:
@@@    Expand__insgtuw(result[0],opnd[0],opnd[1],opnd[2],ops) ;
@@@  break ;
*/

/*
 * Expansion of insgtw based on validated and scheduled basic assembly source.
*/
static void
Expand__insgtw(
 TN* o0,
 TN* i0,
 TN* i1,
 TN* i2,
 OPS* ops
)
{
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_shl_i_r_r,	r0_20_0,	i0,	c1,	ops) ;
  Build_OP (	TOP_cmpgt_r_r_r,	r0_21_0,	i1,	i2,	ops) ;
  Build_OP (	TOP_or_r_r_r,	o0,	r0_20_0,	r0_21_0,	ops) ;
} /* Expand__insgtw */

/*
@@@  case INTRN_INSGTW:
@@@    Expand__insgtw(result[0],opnd[0],opnd[1],opnd[2],ops) ;
@@@  break ;
*/

/*
 * Expansion of insleuw based on validated and scheduled basic assembly source.
*/
static void
Expand__insleuw(
 TN* o0,
 TN* i0,
 TN* i1,
 TN* i2,
 OPS* ops
)
{
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_shl_i_r_r,	r0_20_0,	i0,	c1,	ops) ;
  Build_OP (	TOP_cmpleu_r_r_r,	r0_21_0,	i1,	i2,	ops) ;
  Build_OP (	TOP_or_r_r_r,	o0,	r0_20_0,	r0_21_0,	ops) ;
} /* Expand__insleuw */

/*
@@@  case INTRN_INSLEUW:
@@@    Expand__insleuw(result[0],opnd[0],opnd[1],opnd[2],ops) ;
@@@  break ;
*/

/*
 * Expansion of inslew based on validated and scheduled basic assembly source.
*/
static void
Expand__inslew(
 TN* o0,
 TN* i0,
 TN* i1,
 TN* i2,
 OPS* ops
)
{
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_shl_i_r_r,	r0_20_0,	i0,	c1,	ops) ;
  Build_OP (	TOP_cmple_r_r_r,	r0_21_0,	i1,	i2,	ops) ;
  Build_OP (	TOP_or_r_r_r,	o0,	r0_20_0,	r0_21_0,	ops) ;
} /* Expand__inslew */

/*
@@@  case INTRN_INSLEW:
@@@    Expand__inslew(result[0],opnd[0],opnd[1],opnd[2],ops) ;
@@@  break ;
*/

/*
 * Expansion of insltuw based on validated and scheduled basic assembly source.
*/
static void
Expand__insltuw(
 TN* o0,
 TN* i0,
 TN* i1,
 TN* i2,
 OPS* ops
)
{
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_shl_i_r_r,	r0_20_0,	i0,	c1,	ops) ;
  Build_OP (	TOP_cmpltu_r_r_r,	r0_21_0,	i1,	i2,	ops) ;
  Build_OP (	TOP_or_r_r_r,	o0,	r0_20_0,	r0_21_0,	ops) ;
} /* Expand__insltuw */

/*
@@@  case INTRN_INSLTUW:
@@@    Expand__insltuw(result[0],opnd[0],opnd[1],opnd[2],ops) ;
@@@  break ;
*/

/*
 * Expansion of insltw based on validated and scheduled basic assembly source.
*/
static void
Expand__insltw(
 TN* o0,
 TN* i0,
 TN* i1,
 TN* i2,
 OPS* ops
)
{
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_shl_i_r_r,	r0_20_0,	i0,	c1,	ops) ;
  Build_OP (	TOP_cmplt_r_r_r,	r0_21_0,	i1,	i2,	ops) ;
  Build_OP (	TOP_or_r_r_r,	o0,	r0_20_0,	r0_21_0,	ops) ;
} /* Expand__insltw */

/*
@@@  case INTRN_INSLTW:
@@@    Expand__insltw(result[0],opnd[0],opnd[1],opnd[2],ops) ;
@@@  break ;
*/

/*
 * Expansion of insneuw based on validated and scheduled basic assembly source.
*/
static void
Expand__insneuw(
 TN* o0,
 TN* i0,
 TN* i1,
 TN* i2,
 OPS* ops
)
{
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_shl_i_r_r,	r0_20_0,	i0,	c1,	ops) ;
  Build_OP (	TOP_cmpne_r_r_r,	r0_21_0,	i1,	i2,	ops) ;
  Build_OP (	TOP_or_r_r_r,	o0,	r0_20_0,	r0_21_0,	ops) ;
} /* Expand__insneuw */

/*
@@@  case INTRN_INSNEUW:
@@@    Expand__insneuw(result[0],opnd[0],opnd[1],opnd[2],ops) ;
@@@  break ;
*/

/*
 * Expansion of insnew based on validated and scheduled basic assembly source.
*/
static void
Expand__insnew(
 TN* o0,
 TN* i0,
 TN* i1,
 TN* i2,
 OPS* ops
)
{
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_shl_i_r_r,	r0_20_0,	i0,	c1,	ops) ;
  Build_OP (	TOP_cmpne_r_r_r,	r0_21_0,	i1,	i2,	ops) ;
  Build_OP (	TOP_or_r_r_r,	o0,	r0_20_0,	r0_21_0,	ops) ;
} /* Expand__insnew */

/*
@@@  case INTRN_INSNEW:
@@@    Expand__insnew(result[0],opnd[0],opnd[1],opnd[2],ops) ;
@@@  break ;
*/

/*
 * Expansion of lel based on validated and scheduled basic assembly source.
*/
static void
Expand__lel(
 TN* o0,
 TN* il0,
 TN* ih0,
 TN* il1,
 TN* ih1,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_16_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_17_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  Build_OP (	TOP_cmpleu_r_r_r,	r0_16_0,	il0,	il1,	ops) ;
  Build_OP (	TOP_cmple_r_r_r,	r0_17_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_cmpeq_r_r_b,	b0_0_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_targ_slct_r_r_b_r,	o0,	b0_0_0,	r0_16_0,	r0_17_0,	ops) ;
} /* Expand__lel */

/*
@@@  case INTRN_LEL:
@@@    Expand__lel(result[0],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
@@@  break ;
*/

/*
 * Expansion of leul based on validated and scheduled basic assembly source.
*/
static void
Expand__leul(
 TN* o0,
 TN* il0,
 TN* ih0,
 TN* il1,
 TN* ih1,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_16_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_17_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  Build_OP (	TOP_cmpleu_r_r_r,	r0_16_0,	il0,	il1,	ops) ;
  Build_OP (	TOP_cmpleu_r_r_r,	r0_17_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_cmpeq_r_r_b,	b0_0_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_targ_slct_r_r_b_r,	o0,	b0_0_0,	r0_16_0,	r0_17_0,	ops) ;
} /* Expand__leul */

/*
@@@  case INTRN_LEUL:
@@@    Expand__leul(result[0],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
@@@  break ;
*/

/*
 * Expansion of ltl based on validated and scheduled basic assembly source.
*/
static void
Expand__ltl(
 TN* o0,
 TN* il0,
 TN* ih0,
 TN* il1,
 TN* ih1,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_16_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_17_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  Build_OP (	TOP_cmpltu_r_r_r,	r0_16_0,	il0,	il1,	ops) ;
  Build_OP (	TOP_cmplt_r_r_r,	r0_17_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_cmpeq_r_r_b,	b0_0_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_targ_slct_r_r_b_r,	o0,	b0_0_0,	r0_16_0,	r0_17_0,	ops) ;
} /* Expand__ltl */

/*
@@@  case INTRN_LTL:
@@@    Expand__ltl(result[0],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
@@@  break ;
*/

/*
 * Expansion of ltul based on validated and scheduled basic assembly source.
*/
static void
Expand__ltul(
 TN* o0,
 TN* il0,
 TN* ih0,
 TN* il1,
 TN* ih1,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_16_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_17_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  Build_OP (	TOP_cmpltu_r_r_r,	r0_16_0,	il0,	il1,	ops) ;
  Build_OP (	TOP_cmpltu_r_r_r,	r0_17_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_cmpeq_r_r_b,	b0_0_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_targ_slct_r_r_b_r,	o0,	b0_0_0,	r0_16_0,	r0_17_0,	ops) ;
} /* Expand__ltul */

/*
@@@  case INTRN_LTUL:
@@@    Expand__ltul(result[0],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
@@@  break ;
*/

/*
 * Expansion of lzcnth based on validated and scheduled basic assembly source.
*/
static void
Expand__lzcnth(
 TN* o0,
 TN* i0,
 OPS* ops
)
{
  TN *r0_16_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_16_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c_16 = Gen_Literal_TN(__EXTS32TOS64(-16), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_clz_r_r,	r0_16_0,	i0,	ops) ;
  Build_OP (	TOP_add_i_r_r,	r0_16_1,	r0_16_0,	c_16,	ops) ;
  Build_OP (	TOP_max_r_r_r,	o0,	r0_16_1,	Zero_TN,	ops) ;
} /* Expand__lzcnth */

/*
@@@  case INTRN_LZCNTH:
@@@    Expand__lzcnth(result[0],opnd[0],ops) ;
@@@  break ;
*/

/*
 * Expansion of lzcntl based on validated and scheduled basic assembly source.
*/
static void
Expand__lzcntl(
 TN* o0,
 TN* il0,
 TN* ih0,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c32 = Gen_Literal_TN(__EXTS32TOS64(32), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_clz_r_r,	r0_20_0,	ih0,	ops) ;
  Build_OP (	TOP_clz_r_r,	r0_21_0,	il0,	ops) ;
  Build_OP (	TOP_cmpeq_r_r_b,	b0_0_0,	ih0,	Zero_TN,	ops) ;
  Build_OP (	TOP_add_i_r_r,	r0_21_1,	r0_21_0,	c32,	ops) ;
  Build_OP (	TOP_targ_slct_r_r_b_r,	o0,	b0_0_0,	r0_21_1,	r0_20_0,	ops) ;
} /* Expand__lzcntl */

/*
@@@  case INTRN_LZCNTL:
@@@    Expand__lzcntl(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of lzcntw based on validated and scheduled basic assembly source.
*/
static void
Expand__lzcntw(
 TN* o0,
 TN* i0,
 OPS* ops
)
{
  Build_OP (	TOP_clz_r_r,	o0,	i0,	ops) ;
} /* Expand__lzcntw */

/*
@@@  case INTRN_LZCNTW:
@@@    Expand__lzcntw(result[0],opnd[0],ops) ;
@@@  break ;
*/

/*
 * Expansion of mafcw based on validated and scheduled basic assembly source.
*/
static void
Expand__mafcw(
 TN* o0,
 TN* i0,
 TN* i1,
 TN* i2,
 OPS* ops
)
{
  TN *b0_0_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_1_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_1_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_1_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_20_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_24_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_25_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_26_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_27_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_28_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_29_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_30_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_30_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c0x7fffffff = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *c0x7fffffff_ltn = Gen_Literal_TN(__EXTS32TOS64(0x7fffffff), 4) ;
  Build_OP (	TOP_mov_ii_r,	c0x7fffffff,	c0x7fffffff_ltn,	ops) ;
  TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
  TN *c31 = Gen_Literal_TN(__EXTS32TOS64(31), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_mulll_r_r_r,	r0_20_2,	i1,	i2,	ops) ;
  Build_OP (	TOP_mov_r_r,	r0_30_1,	c0x7fffffff,	ops) ;
  Build_OP (	TOP_convib_r_b,	b0_0_1,	Zero_TN,	ops) ;
  Build_OP (	TOP_shr_i_r_r,	r0_26_1,	i0,	c31,	ops) ;
  Build_OP (	TOP_shr_i_r_r,	r0_25_2,	r0_20_2,	c31,	ops) ;
  Build_OP (	TOP_shl_i_r_r,	r0_24_2,	r0_20_2,	c1,	ops) ;
  Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_27_3,	b0_1_3,	i0,	r0_24_2,	b0_0_1,	ops) ;
  Build_OP (	TOP_shr_i_r_r,	r0_28_4,	r0_27_3,	c31,	ops) ;
  Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_29_4,	b0_1_4,	r0_26_1,	r0_25_2,	b0_1_3,	ops) ;
  Build_OP (	TOP_cmpeq_r_r_b,	b0_1_5,	r0_28_4,	r0_29_4,	ops) ;
  Build_OP (	TOP_sub_r_r_r,	r0_30_5,	r0_30_1,	r0_26_1,	ops) ;
  Build_OP (	TOP_targ_slct_r_r_b_r,	o0,	b0_1_5,	r0_27_3,	r0_30_5,	ops) ;
} /* Expand__mafcw */

/*
@@@  case INTRN_MAFCW:
@@@    Expand__mafcw(result[0],opnd[0],opnd[1],opnd[2],ops) ;
@@@  break ;
*/

/*
 * Expansion of maxh based on validated and scheduled basic assembly source.
*/
static void
Expand__maxh(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  Build_OP (	TOP_max_r_r_r,	o0,	i0,	i1,	ops) ;
} /* Expand__maxh */

/*
@@@  case INTRN_MAXH:
@@@    Expand__maxh(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of maxl based on validated and scheduled basic assembly source.
*/
static void
Expand__maxl(
 TN* ol0,
 TN* oh0,
 TN* il0,
 TN* ih0,
 TN* il1,
 TN* ih1,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_1_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  Build_OP (	TOP_cmpgt_r_r_b,	b0_0_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_cmpeq_r_r_b,	b0_1_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_maxu_r_r_r,	r0_20_0,	il0,	il1,	ops) ;
  Build_OP (	TOP_targ_slct_r_r_b_r,	oh0,	b0_0_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_targ_slct_r_r_b_r,	r0_21_1,	b0_0_0,	il0,	il1,	ops) ;
  Build_OP (	TOP_targ_slct_r_r_b_r,	ol0,	b0_1_0,	r0_20_0,	r0_21_1,	ops) ;
} /* Expand__maxl */

/*
@@@  case INTRN_MAXL:
@@@    Expand__maxl(result[0],result[1],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
@@@  break ;
*/

/*
 * Expansion of maxuh based on validated and scheduled basic assembly source.
*/
static void
Expand__maxuh(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  Build_OP (	TOP_maxu_r_r_r,	o0,	i0,	i1,	ops) ;
} /* Expand__maxuh */

/*
@@@  case INTRN_MAXUH:
@@@    Expand__maxuh(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of maxul based on validated and scheduled basic assembly source.
*/
static void
Expand__maxul(
 TN* ol0,
 TN* oh0,
 TN* il0,
 TN* ih0,
 TN* il1,
 TN* ih1,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_1_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  Build_OP (	TOP_cmpgtu_r_r_b,	b0_0_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_cmpeq_r_r_b,	b0_1_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_maxu_r_r_r,	r0_20_0,	il0,	il1,	ops) ;
  Build_OP (	TOP_targ_slct_r_r_b_r,	oh0,	b0_0_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_targ_slct_r_r_b_r,	r0_21_1,	b0_0_0,	il0,	il1,	ops) ;
  Build_OP (	TOP_targ_slct_r_r_b_r,	ol0,	b0_1_0,	r0_20_0,	r0_21_1,	ops) ;
} /* Expand__maxul */

/*
@@@  case INTRN_MAXUL:
@@@    Expand__maxul(result[0],result[1],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
@@@  break ;
*/

/*
 * Expansion of minh based on validated and scheduled basic assembly source.
*/
static void
Expand__minh(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  Build_OP (	TOP_min_r_r_r,	o0,	i0,	i1,	ops) ;
} /* Expand__minh */

/*
@@@  case INTRN_MINH:
@@@    Expand__minh(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of minl based on validated and scheduled basic assembly source.
*/
static void
Expand__minl(
 TN* ol0,
 TN* oh0,
 TN* il0,
 TN* ih0,
 TN* il1,
 TN* ih1,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_1_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  Build_OP (	TOP_cmplt_r_r_b,	b0_0_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_cmpeq_r_r_b,	b0_1_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_minu_r_r_r,	r0_20_0,	il0,	il1,	ops) ;
  Build_OP (	TOP_targ_slct_r_r_b_r,	oh0,	b0_0_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_targ_slct_r_r_b_r,	r0_21_1,	b0_0_0,	il0,	il1,	ops) ;
  Build_OP (	TOP_targ_slct_r_r_b_r,	ol0,	b0_1_0,	r0_20_0,	r0_21_1,	ops) ;
} /* Expand__minl */

/*
@@@  case INTRN_MINL:
@@@    Expand__minl(result[0],result[1],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
@@@  break ;
*/

/*
 * Expansion of minuh based on validated and scheduled basic assembly source.
*/
static void
Expand__minuh(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  Build_OP (	TOP_minu_r_r_r,	o0,	i0,	i1,	ops) ;
} /* Expand__minuh */

/*
@@@  case INTRN_MINUH:
@@@    Expand__minuh(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of minul based on validated and scheduled basic assembly source.
*/
static void
Expand__minul(
 TN* ol0,
 TN* oh0,
 TN* il0,
 TN* ih0,
 TN* il1,
 TN* ih1,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_1_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  Build_OP (	TOP_cmpltu_r_r_b,	b0_0_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_cmpeq_r_r_b,	b0_1_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_minu_r_r_r,	r0_20_0,	il0,	il1,	ops) ;
  Build_OP (	TOP_targ_slct_r_r_b_r,	oh0,	b0_0_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_targ_slct_r_r_b_r,	r0_21_1,	b0_0_0,	il0,	il1,	ops) ;
  Build_OP (	TOP_targ_slct_r_r_b_r,	ol0,	b0_1_0,	r0_20_0,	r0_21_1,	ops) ;
} /* Expand__minul */

/*
@@@  case INTRN_MINUL:
@@@    Expand__minul(result[0],result[1],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
@@@  break ;
*/

/*
 * Expansion of mpfcw hand-modified to detect constant operands
 * and to select better sequence for st231.
*/
static void
Expand__mpfcw(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  BOOL overflow_possible = TRUE;

  if (TN_is_rematerializable (i0)) {
    WN *wn = TN_remat (i0);
    if (WN_operator_is (wn, OPR_INTCONST)) {
      INT64 co0 = WN_const_val (wn);
      if ((co0 & 0xffff) != 0x8000) {
	overflow_possible = FALSE;
      }
    }
  }
  if (TN_is_rematerializable (i1)) {
    WN *wn = TN_remat (i1);
    if (WN_operator_is (wn, OPR_INTCONST)) {
      INT64 co1 = WN_const_val (wn);
      if ((co1 & 0xffff) != 0x8000) {
	overflow_possible = FALSE;
      }
    }
  }

  if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_adds_r_r_r)) { 
    TN *r0_16_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    Build_OP (	TOP_mulll_r_r_r,	r0_16_2,	i0,	i1,	ops) ;
    Build_OP (	TOP_adds_r_r_r,	o0,	r0_16_2,	r0_16_2,	ops) ;
  } else if (! overflow_possible) {
    TN *r0_26_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *c1 = Gen_Literal_TN(1, 4) ;
    Build_OP (	TOP_mulll_r_r_r,	r0_26_2,	i0,	i1,	ops) ;
    Build_OP (	TOP_shl_i_r_r,	o0,	r0_26_2,	c1,	ops) ;
  } else if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_mulfrac_r_r_r)) { 
    TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
    TN *c16 = Gen_Literal_TN(__EXTS32TOS64(16), 4) ;
#undef __EXTS32TOS64
    Build_OP (	TOP_shl_i_r_r,	r0_20_0,	i0,	c16,	ops) ;
    Build_OP (	TOP_shl_i_r_r,	r0_21_0,	i1,	c16,	ops) ;
    Build_OP (	TOP_mulfrac_r_r_r,	o0,	r0_20_0,	r0_21_0,	ops) ;
  }  else {
    TN *b0_0_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *r0_16_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_16_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
    TN *c0x7fffffff = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *c0x7fffffff_ltn = Gen_Literal_TN(__EXTS32TOS64(0x7fffffff), 4) ;
    Build_OP (    TOP_mov_ii_r,      c0x7fffffff,    c0x7fffffff_ltn,        ops) ;
    TN *c0x40000000 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *c0x40000000_ltn = Gen_Literal_TN(__EXTS32TOS64(0x40000000), 4) ;
    Build_OP (    TOP_mov_ii_r,      c0x40000000,    c0x40000000_ltn,        ops) ;
    TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
#undef __EXTS32TOS64
    Build_OP (    TOP_mulll_r_r_r,    r0_16_2,        i0,     i1,     ops) ;
    Build_OP (    TOP_cmpne_r_r_b,  b0_0_3, r0_16_2,        c0x40000000,    ops) ;
    Build_OP (    TOP_shl_i_r_r,      r0_16_3,        r0_16_2,        c1,     ops) ;
    Build_OP (    TOP_targ_slct_r_r_b_r,     o0,     b0_0_3, r0_16_3,        c0x7fffffff,    ops) ;
  }
} /* Expand__mpfcw */

/*
@@@  case INTRN_MPFCWL:
@@@    Expand__mpfcw(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of mpfcwl based on validated and scheduled basic assembly source.
*/
static void
Expand__mpfcwl(
 TN* ol0,
 TN* oh0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_mul32_r_r_r)
      && ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_mul64hu_r_r_r)) {
      TN *b0_0_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *r0_16_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_16_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_17_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_20_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_20_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_21_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
      TN *c0x7fffffff = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *c0x7fffffff_ltn = Gen_Literal_TN(__EXTS32TOS64(0x7fffffff), 4) ;
      Build_OP (    TOP_mov_ii_r,      c0x7fffffff,    c0x7fffffff_ltn,        ops) ;
      TN *c0x80000000 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *c0x80000000_ltn = Gen_Literal_TN(__EXTS32TOS64(0x80000000), 4) ;
      Build_OP (    TOP_mov_ii_r,      c0x80000000,    c0x80000000_ltn,        ops) ;
      TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
      TN *c31 = Gen_Literal_TN(__EXTS32TOS64(31), 4) ;
      TN *c_1 = Gen_Literal_TN(__EXTS32TOS64(-1), 4) ;
#undef __EXTS32TOS64
      Build_OP (    TOP_mul32_r_r_r,   r0_16_3 ,    i0,     i1,     ops) ;
      Build_OP (    TOP_mul64h_r_r_r,   r0_17_4,    i0,     i1,     ops) ;
      Build_OP (	TOP_cmpeq_r_r_r,	r0_21_2,	i0,	i1,	ops) ;
      Build_OP (	TOP_cmpeq_r_r_r,	r0_20_2,	i0,	c0x80000000,	ops) ;
      Build_OP (	TOP_andl_r_r_b,	b0_0_3,	r0_20_2,	r0_21_2,	ops) ;
      Build_OP (	TOP_shru_i_r_r,	r0_21_2,	r0_16_3,	c31,	ops) ;
      Build_OP (	TOP_shl_i_r_r,	r0_20_2,	r0_17_4,	c1,	ops) ;
      Build_OP (	TOP_add_i_r_r,	r0_20_5,	Zero_TN,	c_1,	ops) ;
      Build_OP (	TOP_shl_i_r_r,	r0_16_4,	r0_16_3,	c1,	ops) ;
      Build_OP (	TOP_or_r_r_r,	r0_17_4,	r0_21_2,	r0_20_2,	ops) ;
      Build_OP (	TOP_targ_slct_r_r_b_r,	ol0,	b0_0_3,	r0_20_5,	r0_16_4,	ops) ;
      Build_OP (	TOP_slctf_r_r_b_r,	oh0,	b0_0_3,	r0_17_4,	c0x7fffffff,	ops) ;
  } else { 
      TN *b0_0_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *r0_16_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_16_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_17_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_17_6 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_20_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_20_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_21_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_21_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_21_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_24_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_25_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_26_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_27_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
      TN *c0x7fffffff = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *c0x7fffffff_ltn = Gen_Literal_TN(__EXTS32TOS64(0x7fffffff), 4) ;
      Build_OP (    TOP_mov_ii_r,      c0x7fffffff,    c0x7fffffff_ltn,        ops) ;
      TN *c0x80000000 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *c0x80000000_ltn = Gen_Literal_TN(__EXTS32TOS64(0x80000000), 4) ;
      Build_OP (    TOP_mov_ii_r,      c0x80000000,    c0x80000000_ltn,        ops) ;
      TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
      TN *c31 = Gen_Literal_TN(__EXTS32TOS64(31), 4) ;
      TN *c_1 = Gen_Literal_TN(__EXTS32TOS64(-1), 4) ;
#undef __EXTS32TOS64
      Build_OP (	TOP_mullu_r_r_r,	r0_26_2,	i0,	i1,	ops) ;
      Build_OP (	TOP_mulhs_r_r_r,	r0_27_2,	i0,	i1,	ops) ;
      Build_OP (	TOP_mulhhs_r_r_r,	r0_24_3,	i0,	i1,	ops) ;
      Build_OP (	TOP_mullhus_r_r_r,	r0_25_3,	i0,	i1,	ops) ;
      Build_OP (	TOP_cmpeq_r_r_r,	r0_21_2,	i0,	i1,	ops) ;
      Build_OP (	TOP_cmpeq_r_r_r,	r0_20_2,	i0,	c0x80000000,	ops) ;
      Build_OP (	TOP_convib_r_b,	b0_2_2,	Zero_TN,	ops) ;
      Build_OP (	TOP_andl_r_r_b,	b0_0_3,	r0_20_2,	r0_21_2,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_16_3,	b0_3_3,	r0_26_2,	r0_27_2,	b0_2_2,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_21_4,	b0_3_4,	r0_24_3,	r0_25_3,	b0_3_3,	ops) ;
      Build_OP (	TOP_shru_i_r_r,	r0_17_4,	r0_16_3,	c31,	ops) ;
      Build_OP (	TOP_shl_i_r_r,	r0_16_4,	r0_16_3,	c1,	ops) ;
      Build_OP (	TOP_shl_i_r_r,	r0_21_5,	r0_21_4,	c1,	ops) ;
      Build_OP (	TOP_add_i_r_r,	r0_20_5,	Zero_TN,	c_1,	ops) ;
      Build_OP (	TOP_or_r_r_r,	r0_17_6,	r0_17_4,	r0_21_5,	ops) ;
      Build_OP (	TOP_targ_slct_r_r_b_r,	ol0,	b0_0_3,	r0_20_5,	r0_16_4,	ops) ;
      Build_OP (	TOP_slctf_r_r_b_r,	oh0,	b0_0_3,	r0_17_6,	c0x7fffffff,	ops) ;
  }
} /* Expand__mpfcwl */

/*
@@@  case INTRN_MPFCWL:
@@@    Expand__mpfcwl(result[0],result[1],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of mpfml based on validated and scheduled basic assembly source.
*/
static void
Expand__mpfml(
 TN* ol0,
 TN* oh0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_mul32_r_r_r)
      && ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_mul64hu_r_r_r)) {
      TN *r0_16_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_17_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_20_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_21_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
      TN *c15 = Gen_Literal_TN(__EXTS32TOS64(15), 4) ;
      TN *c17 = Gen_Literal_TN(__EXTS32TOS64(17), 4) ;
#undef __EXTS32TOS64
      Build_OP (    TOP_mul32_r_r_r,   r0_16_3 ,    i0,     i1,     ops) ;
      Build_OP (    TOP_mul64h_r_r_r,   r0_17_4,    i0,     i1,     ops) ;
      Build_OP (	TOP_shl_i_r_r,	ol0,	r0_16_3,	c17,	ops) ;
      Build_OP (	TOP_shru_i_r_r,	r0_20_4,	r0_16_3,	c15,	ops) ;
      Build_OP (	TOP_shl_i_r_r,	r0_21_5,	r0_17_4,	c17,	ops) ;
      Build_OP (	TOP_or_r_r_r,	oh0,	r0_20_4,	r0_21_5,	ops) ;
  } else { 
      TN *b0_2_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *r0_16_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_17_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_20_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_21_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_24_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_25_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_26_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_27_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
      TN *c15 = Gen_Literal_TN(__EXTS32TOS64(15), 4) ;
      TN *c17 = Gen_Literal_TN(__EXTS32TOS64(17), 4) ;
#undef __EXTS32TOS64
      Build_OP (	TOP_mullu_r_r_r,	r0_26_2,	i0,	i1,	ops) ;
      Build_OP (	TOP_mulhs_r_r_r,	r0_27_2,	i0,	i1,	ops) ;
      Build_OP (	TOP_mulhhs_r_r_r,	r0_24_3,	i0,	i1,	ops) ;
      Build_OP (	TOP_mullhus_r_r_r,	r0_25_3,	i0,	i1,	ops) ;
      Build_OP (	TOP_convib_r_b,	b0_2_2,	Zero_TN,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_16_3,	b0_3_3,	r0_26_2,	r0_27_2,	b0_2_2,	ops) ;
      Build_OP (	TOP_shl_i_r_r,	ol0,	r0_16_3,	c17,	ops) ;
      Build_OP (	TOP_shru_i_r_r,	r0_20_4,	r0_16_3,	c15,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_4,	b0_3_4,	r0_24_3,	r0_25_3,	b0_3_3,	ops) ;
      Build_OP (	TOP_shl_i_r_r,	r0_21_5,	r0_17_4,	c17,	ops) ;
      Build_OP (	TOP_or_r_r_r,	oh0,	r0_20_4,	r0_21_5,	ops) ;
  }
} /* Expand__mpfml */

/*
@@@  case INTRN_MPFML:
@@@    Expand__mpfml(result[0],result[1],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of mpfrch based on validated and scheduled basic assembly source.
*/
static void
Expand__mpfrch(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_adds_r_r_r)) {
    TN *r0_21_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_21_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
    TN *c16 = Gen_Literal_TN(__EXTS32TOS64(16), 4) ;
    TN *c16384 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *c16384_ltn = Gen_Literal_TN(__EXTS32TOS64(16384), 4) ;
    Build_OP (    TOP_mov_ii_r,      c16384,        c16384_ltn,    ops) ;
#undef __EXTS32TOS64
    Build_OP (	TOP_mulll_r_r_r,	r0_21_2,	i0,	i1,	ops) ;
    Build_OP (	TOP_add_r_r_r,	r0_21_1,	r0_21_2,	c16384,	ops) ;
    Build_OP (	TOP_adds_r_r_r,	r0_21_2,	r0_21_1,	r0_21_1,	ops) ;
    Build_OP (	TOP_shr_i_r_r,	o0,	r0_21_2,	c16,	ops) ;
  } else {
    TN *b0_0_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *r0_16_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_21_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_21_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_26_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)                (((long long)(x)<<32) >> 32)
    TN *c0x4000 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *c0x4000_ltn = Gen_Literal_TN(__EXTS32TOS64(0x4000), 4) ;
    Build_OP (    TOP_mov_ii_r,      c0x4000,        c0x4000_ltn,    ops) ;
    TN *c0x7fff = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *c0x7fff_ltn = Gen_Literal_TN(__EXTS32TOS64(0x7fff), 4) ;
    Build_OP (    TOP_mov_ii_r,      c0x7fff,        c0x7fff_ltn,    ops) ;
    TN *c0xffff8000 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *c0xffff8000_ltn = Gen_Literal_TN(__EXTS32TOS64(0xffff8000), 4) ;
    Build_OP (    TOP_mov_ii_r,      c0xffff8000,    c0xffff8000_ltn,        ops) ;
    TN *c15 = Gen_Literal_TN(__EXTS32TOS64(15), 4) ;
#undef __EXTS32TOS64
    Build_OP (    TOP_mulll_r_r_r,    r0_26_2,        i0,     i1,     ops) ;
    Build_OP (    TOP_cmpeq_r_r_r,  r0_20_0,        i0,     c0xffff8000,    ops) ;
    Build_OP (    TOP_cmpeq_r_r_r,  r0_21_1,        i0,     i1,     ops) ;
    Build_OP (    TOP_andl_r_r_b,   b0_0_3, r0_20_0,        r0_21_1,        ops) ;
    Build_OP (    TOP_add_r_r_r,      r0_21_3,        r0_26_2,        c0x4000,        ops) ;
    Build_OP (    TOP_shr_i_r_r,      r0_16_4,        r0_21_3,        c15,    ops) ;
    Build_OP (    TOP_slctf_r_r_b_r,    o0,     b0_0_3, r0_16_4,        c0x7fff,        ops) ;
  }
} /* Expand__mpfrch */

/*
@@@  case INTRN_MPFRCH:
@@@    Expand__mpfrch(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of mpml based on validated and scheduled basic assembly source.
*/
static void
Expand__mpml(
 TN* ol0,
 TN* oh0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_mul32_r_r_r)
      && ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_mul64hu_r_r_r)) {
      Build_OP (    TOP_mul32_r_r_r,    ol0,    i0,     i1,     ops) ;
      Build_OP (    TOP_mul64h_r_r_r,   oh0,    i0,     i1,     ops) ;
  } else { 
  TN *b0_2_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_3_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_3_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_24_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_25_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_26_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_27_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  Build_OP (	TOP_mullu_r_r_r,	r0_26_2,	i0,	i1,	ops) ;
  Build_OP (	TOP_mulhs_r_r_r,	r0_27_2,	i0,	i1,	ops) ;
  Build_OP (	TOP_mulhhs_r_r_r,	r0_24_3,	i0,	i1,	ops) ;
  Build_OP (	TOP_mullhus_r_r_r,	r0_25_3,	i0,	i1,	ops) ;
  Build_OP (	TOP_convib_r_b,	b0_2_2,	Zero_TN,	ops) ;
  Build_OP (	TOP_targ_addcg_b_r_r_b_r,	ol0,	b0_3_3,	r0_26_2,	r0_27_2,	b0_2_2,	ops) ;
  Build_OP (	TOP_targ_addcg_b_r_r_b_r,	oh0,	b0_3_4,	r0_24_3,	r0_25_3,	b0_3_3,	ops) ;
  }
} /* Expand__mpml */

/*
@@@  case INTRN_MPML:
@@@    Expand__mpml(result[0],result[1],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of mpuml based on validated and scheduled basic assembly source.
*/
static void
Expand__mpuml(
 TN* ol0,
 TN* oh0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_mul64hu_r_r_r)
      && ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_mul32_r_r_r)) {
    Build_OP (	TOP_mul32_r_r_r,	ol0,	i0,	i1,	ops) ;
    Build_OP (	TOP_mul64hu_r_r_r,	oh0,	i0,	i1,	ops) ;
  } else {
    TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *r0_20_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_21_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_28_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    Build_OP (	TOP_mullu_r_r_r,	r0_21_2,	i0,	i1,	ops) ;
    Build_OP (	TOP_mullhus_r_r_r,	r0_20_2,	i0,	i1,	ops) ;
    Build_OP (	TOP_cmplt_r_r_b,	b0_0_0,	i0,	Zero_TN,	ops) ;
    Build_OP (	TOP_targ_slct_r_r_b_r,	r0_28_1,	b0_0_0,	i1,	Zero_TN,	ops) ;
    Build_OP (	TOP_add_r_r_r,	oh0,	r0_20_2,	r0_28_1,	ops) ;
    Build_OP (	TOP_add_r_r_r,	ol0,	r0_21_2,	Zero_TN,	ops) ;
  }
} /* Expand__mpuml */

/*
@@@  case INTRN_MPUML:
@@@    Expand__mpuml(result[0],result[1],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of mulfch based on validated and scheduled basic assembly source.
*/
static void
Expand__mulfch(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_mulfracrm_ph_r_r_r)) { 
    TN *r0_16_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    Build_OP (    TOP_mulfracrm_ph_r_r_r, r0_16_2,        i0,     i1,     ops) ;
    Build_OP (    TOP_sxth_r_r,   o0,     r0_16_2,        ops) ;
  } else if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_adds_r_r_r)) { 
    TN *r0_16_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
    TN *c16 = Gen_Literal_TN(__EXTS32TOS64(16), 4) ;
#undef __EXTS32TOS64
    Build_OP (	TOP_mulll_r_r_r,	o0,	i0,	i1,	ops) ;
    Build_OP (	TOP_adds_r_r_r,	r0_16_1,	o0,	o0,	ops) ;
    Build_OP (	TOP_shr_i_r_r,	o0,	r0_16_1,	c16,	ops) ;
  } else if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_mulfrac_r_r_r)) { 
    TN *r0_16_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_16_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_17_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
    TN *c16 = Gen_Literal_TN(__EXTS32TOS64(16), 4) ;
#undef __EXTS32TOS64
    Build_OP (	TOP_shl_i_r_r,	r0_16_0,	i0,	c16,	ops) ;
    Build_OP (	TOP_shl_i_r_r,	r0_17_0,	i1,	c16,	ops) ;
    Build_OP (	TOP_mulfrac_r_r_r,	r0_16_3,	r0_16_0,	r0_17_0,	ops) ;
    Build_OP (	TOP_shr_i_r_r,	o0,	r0_16_3,	c16,	ops) ;
  } else {
    TN *b0_0_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *r0_16_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_26_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
    TN *c0x7fff = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *c0x7fff_ltn = Gen_Literal_TN(__EXTS32TOS64(0x7fff), 4) ;
    Build_OP (    TOP_mov_ii_r,      c0x7fff,        c0x7fff_ltn,    ops) ;
    TN *c0xffff8000 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *c0xffff8000_ltn = Gen_Literal_TN(__EXTS32TOS64(0xffff8000), 4) ;
    Build_OP (    TOP_mov_ii_r,      c0xffff8000,    c0xffff8000_ltn,        ops) ;
    TN *c15 = Gen_Literal_TN(__EXTS32TOS64(15), 4) ;
#undef __EXTS32TOS64
    Build_OP (	TOP_mulll_r_r_r,	r0_26_2,	i0,	i1,	ops) ;
    Build_OP (	TOP_cmpeq_r_r_r,	r0_20_0,	i0,	c0xffff8000,	ops) ;
    Build_OP (	TOP_cmpeq_r_r_r,	r0_21_0,	i0,	i1,	ops) ;
    Build_OP (	TOP_andl_r_r_b,	b0_0_3,	r0_20_0,	r0_21_0,	ops) ;
    Build_OP (	TOP_shr_i_r_r,	r0_16_3,	r0_26_2,	c15,	ops) ;
    Build_OP (	TOP_slctf_r_r_b_r,	o0,	b0_0_3,	r0_16_3,	c0x7fff,	ops) ;
  }
} /* Expand__mulfch */

/*
@@@  case INTRN_MULFCH:
@@@    Expand__mulfch(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of mulfcm based on validated and scheduled basic assembly source.
*/
static void
Expand__mulfcm(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_mul32_r_r_r)
      && ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_mul64hu_r_r_r)) {
      TN *b0_0_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *r0_16_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_16_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_17_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_20_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_21_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_21_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_22_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_23_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
      TN *c0x7fffffff = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *c0x7fffffff_ltn = Gen_Literal_TN(__EXTS32TOS64(0x7fffffff), 4) ;
      Build_OP (    TOP_mov_ii_r,      c0x7fffffff,    c0x7fffffff_ltn,        ops) ;
      TN *c0x80000000 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *c0x80000000_ltn = Gen_Literal_TN(__EXTS32TOS64(0x80000000), 4) ;
      Build_OP (    TOP_mov_ii_r,      c0x80000000,    c0x80000000_ltn,        ops) ;
      TN *c0xffff8000 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *c0xffff8000_ltn = Gen_Literal_TN(__EXTS32TOS64(0xffff8000), 4) ;
      Build_OP (    TOP_mov_ii_r,      c0xffff8000,    c0xffff8000_ltn,        ops) ;
      TN *c15 = Gen_Literal_TN(__EXTS32TOS64(15), 4) ;
      TN *c17 = Gen_Literal_TN(__EXTS32TOS64(17), 4) ;
#undef __EXTS32TOS64
      Build_OP (    TOP_mul32_r_r_r,    r0_22_3,    i0,     i1,     ops) ;
      Build_OP (    TOP_mul64h_r_r_r,   r0_23_3,    i0,     i1,     ops) ;
      Build_OP (	TOP_cmpeq_r_r_r,	r0_20_2,	i0,	c0x80000000,	ops) ;
      Build_OP (	TOP_cmpeq_r_r_r,	r0_21_2,	i1,	c0xffff8000,	ops) ;
      Build_OP (	TOP_andl_r_r_b,	b0_0_3,	r0_20_2,	r0_21_2,	ops) ;
      Build_OP (	TOP_shl_i_r_r,	r0_21_4,	r0_23_3,	c17,	ops) ;
      Build_OP (	TOP_shru_i_r_r,	r0_16_4,	r0_22_3,	c15,	ops) ;
      Build_OP (	TOP_or_r_r_r,	r0_16_5,	r0_16_4,	r0_21_4,	ops) ;
      Build_OP (	TOP_slctf_r_r_b_r,	o0,	b0_0_3,	r0_16_5,	c0x7fffffff,	ops) ;
  } else {
      TN *b0_0_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *r0_16_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_16_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_17_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_20_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_21_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_21_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_22_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_23_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
      TN *c0x7fffffff = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *c0x7fffffff_ltn = Gen_Literal_TN(__EXTS32TOS64(0x7fffffff), 4) ;
      Build_OP (    TOP_mov_ii_r,      c0x7fffffff,    c0x7fffffff_ltn,        ops) ;
      TN *c0x80000000 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *c0x80000000_ltn = Gen_Literal_TN(__EXTS32TOS64(0x80000000), 4) ;
      Build_OP (    TOP_mov_ii_r,      c0x80000000,    c0x80000000_ltn,        ops) ;
      TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
      TN *c16 = Gen_Literal_TN(__EXTS32TOS64(16), 4) ;
      TN *c31 = Gen_Literal_TN(__EXTS32TOS64(31), 4) ;
#undef __EXTS32TOS64
      Build_OP (	TOP_shl_i_r_r,	r0_17_0,	i1,	c16,	ops) ;
      Build_OP (	TOP_mulhs_r_r_r,	r0_22_3,	i0,	r0_17_0,	ops) ;
      Build_OP (	TOP_mulhhs_r_r_r,	r0_23_3,	i0,	r0_17_0,	ops) ;
      Build_OP (	TOP_cmpeq_r_r_r,	r0_20_2,	i0,	c0x80000000,	ops) ;
      Build_OP (	TOP_cmpeq_r_r_r,	r0_21_2,	i0,	r0_17_0,	ops) ;
      Build_OP (	TOP_andl_r_r_b,	b0_0_3,	r0_20_2,	r0_21_2,	ops) ;
      Build_OP (	TOP_shl_i_r_r,	r0_21_4,	r0_23_3,	c1,	ops) ;
      Build_OP (	TOP_shru_i_r_r,	r0_16_4,	r0_22_3,	c31,	ops) ;
      Build_OP (	TOP_or_r_r_r,	r0_16_5,	r0_16_4,	r0_21_4,	ops) ;
      Build_OP (	TOP_slctf_r_r_b_r,	o0,	b0_0_3,	r0_16_5,	c0x7fffffff,	ops) ;
  }
} /* Expand__mulfcm */

/*
@@@  case INTRN_MULFCM:
@@@    Expand__mulfcm(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of mulfcw based on validated and scheduled basic assembly source.
*/
static void
Expand__mulfcw(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_mul32_r_r_r)
      && ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_mul64hu_r_r_r)) {
      TN *b0_0_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *r0_16_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_17_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_20_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_21_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
      TN *c0x7fffffff = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *c0x7fffffff_ltn = Gen_Literal_TN(__EXTS32TOS64(0x7fffffff), 4) ;
      Build_OP (    TOP_mov_ii_r,      c0x7fffffff,    c0x7fffffff_ltn,        ops) ;
      TN *c0x80000000 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *c0x80000000_ltn = Gen_Literal_TN(__EXTS32TOS64(0x80000000), 4) ;
      Build_OP (    TOP_mov_ii_r,      c0x80000000,    c0x80000000_ltn,        ops) ;
      TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
      TN *c31 = Gen_Literal_TN(__EXTS32TOS64(31), 4) ;
#undef __EXTS32TOS64
      Build_OP (    TOP_mul32_r_r_r,   r0_16_3 ,    i0,     i1,     ops) ;
      Build_OP (    TOP_mul64h_r_r_r,   r0_17_4,    i0,     i1,     ops) ;
      Build_OP (	TOP_cmpeq_r_r_r,	r0_21_2,	i0,	i1,	ops) ;
      Build_OP (	TOP_cmpeq_r_r_r,	r0_20_2,	i0,	c0x80000000,	ops) ;
      Build_OP (	TOP_andl_r_r_b,	b0_0_3,	r0_20_2,	r0_21_2,	ops) ;
      Build_OP (	TOP_shru_i_r_r,	r0_21_2,	r0_16_3,	c31,	ops) ;
      Build_OP (	TOP_shl_i_r_r,	r0_20_2,	r0_17_4,	c1,	ops) ;
      Build_OP (	TOP_or_r_r_r,	r0_17_4,	r0_21_2,	r0_20_2,	ops) ;
      Build_OP (	TOP_slctf_r_r_b_r,	o0,	b0_0_3,	r0_17_4,	c0x7fffffff,	ops) ;
  } else { 
      TN *b0_0_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *r0_16_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_16_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_16_6 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_20_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_21_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_21_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_21_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_24_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_25_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_26_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_27_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
      TN *c0x7fffffff = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *c0x7fffffff_ltn = Gen_Literal_TN(__EXTS32TOS64(0x7fffffff), 4) ;
      Build_OP (    TOP_mov_ii_r,      c0x7fffffff,    c0x7fffffff_ltn,        ops) ;
      TN *c0x80000000 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *c0x80000000_ltn = Gen_Literal_TN(__EXTS32TOS64(0x80000000), 4) ;
      Build_OP (    TOP_mov_ii_r,      c0x80000000,    c0x80000000_ltn,        ops) ;
      TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
      TN *c31 = Gen_Literal_TN(__EXTS32TOS64(31), 4) ;
#undef __EXTS32TOS64
      Build_OP (	TOP_mullu_r_r_r,	r0_26_2,	i0,	i1,	ops) ;
      Build_OP (	TOP_mulhs_r_r_r,	r0_27_2,	i0,	i1,	ops) ;
      Build_OP (	TOP_mulhhs_r_r_r,	r0_24_3,	i0,	i1,	ops) ;
      Build_OP (	TOP_mullhus_r_r_r,	r0_25_3,	i0,	i1,	ops) ;
      Build_OP (	TOP_cmpeq_r_r_r,	r0_20_2,	i0,	c0x80000000,	ops) ;
      Build_OP (	TOP_cmpeq_r_r_r,	r0_21_2,	i0,	i1,	ops) ;
      Build_OP (	TOP_convib_r_b,	b0_2_2,	Zero_TN,	ops) ;
      Build_OP (	TOP_andl_r_r_b,	b0_0_3,	r0_20_2,	r0_21_2,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_16_3,	b0_3_3,	r0_26_2,	r0_27_2,	b0_2_2,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_21_4,	b0_3_4,	r0_24_3,	r0_25_3,	b0_3_3,	ops) ;
      Build_OP (	TOP_shru_i_r_r,	r0_16_4,	r0_16_3,	c31,	ops) ;
      Build_OP (	TOP_shl_i_r_r,	r0_21_5,	r0_21_4,	c1,	ops) ;
      Build_OP (	TOP_or_r_r_r,	r0_16_6,	r0_16_4,	r0_21_5,	ops) ;
      Build_OP (	TOP_slctf_r_r_b_r,	o0,	b0_0_3,	r0_16_6,	c0x7fffffff,	ops) ;
  } /* Expand__mulfcw */
}
/*
@@@  case INTRN_MULFCW:
@@@    Expand__mulfcw(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of mulh based on validated and scheduled basic assembly source.
*/
static void
Expand__mulh(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  Build_OP (	TOP_mulll_r_r_r,	o0,	i0,	i1,	ops) ;
} /* Expand__mulh */

/*
@@@  case INTRN_MULH:
@@@    Expand__mulh(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of mulhh based on validated and scheduled basic assembly source.
*/
static void
Expand__mulhh(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  TN *r0_16_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c16 = Gen_Literal_TN(__EXTS32TOS64(16), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_mulll_r_r_r,	r0_16_2,	i0,	i1,	ops) ;
  Build_OP (	TOP_shr_i_r_r,	o0,	r0_16_2,	c16,	ops) ;
} /* Expand__mulhh */

/*
@@@  case INTRN_MULHH:
@@@    Expand__mulhh(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of mulhuh based on validated and scheduled basic assembly source.
*/
static void
Expand__mulhuh(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  TN *r0_16_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c16 = Gen_Literal_TN(__EXTS32TOS64(16), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_mulllu_r_r_r,	r0_16_2,	i0,	i1,	ops) ;
  Build_OP (	TOP_shr_i_r_r,	o0,	r0_16_2,	c16,	ops) ;
} /* Expand__mulhuh */

/*
@@@  case INTRN_MULHUH:
@@@    Expand__mulhuh(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of mulhuw based on validated and scheduled basic assembly source.
*/
static void
Expand__mulhuw(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_mul64hu_r_r_r)) {
      Build_OP (	TOP_mul64hu_r_r_r,	o0,	i0,	i1,	ops) ;
  } else {
      TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_1_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *r0_16_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_17_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_24_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_25_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_26_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_27_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_28_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_28_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_29_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      Build_OP (	TOP_mullu_r_r_r,	r0_26_2,	i0,	i1,	ops) ;
      Build_OP (	TOP_mulhs_r_r_r,	r0_27_2,	i0,	i1,	ops) ;
      Build_OP (	TOP_cmplt_r_r_b,	b0_0_0,	i0,	Zero_TN,	ops) ;
      Build_OP (	TOP_cmplt_r_r_b,	b0_1_0,	i1,	Zero_TN,	ops) ;
      Build_OP (	TOP_mulhhs_r_r_r,	r0_24_3,	i0,	i1,	ops) ;
      Build_OP (	TOP_mullhus_r_r_r,	r0_25_3,	i0,	i1,	ops) ;
      Build_OP (	TOP_targ_slct_r_r_b_r,	r0_28_1,	b0_1_0,	i0,	Zero_TN,	ops) ;
      Build_OP (	TOP_targ_slct_r_r_b_r,	r0_29_1,	b0_0_0,	i1,	Zero_TN,	ops) ;
      Build_OP (	TOP_add_r_r_r,	r0_28_2,	r0_28_1,	r0_29_1,	ops) ;
      Build_OP (	TOP_convib_r_b,	b0_2_2,	Zero_TN,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_16_3,	b0_3_3,	r0_26_2,	r0_27_2,	b0_2_2,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_4,	b0_3_4,	r0_24_3,	r0_25_3,	b0_3_3,	ops) ;
      Build_OP (	TOP_add_r_r_r,	o0,	r0_17_4,	r0_28_2,	ops) ;
  }
} /* Expand__mulhuw */

/*
@@@  case INTRN_MULHUW:
@@@    Expand__mulhuw(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of mulhw based on validated and scheduled basic assembly source.
*/
static void
Expand__mulhw(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_mul64h_r_r_r)) {
      Build_OP (	TOP_mul64h_r_r_r,	o0,	i0,	i1,	ops) ;
  } else {
      TN *b0_2_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *r0_16_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_24_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_25_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_26_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_27_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      Build_OP (	TOP_mullu_r_r_r,	r0_26_2,	i0,	i1,	ops) ;
      Build_OP (	TOP_mulhs_r_r_r,	r0_27_2,	i0,	i1,	ops) ;
      Build_OP (	TOP_mulhhs_r_r_r,	r0_24_3,	i0,	i1,	ops) ;
      Build_OP (	TOP_mullhus_r_r_r,	r0_25_3,	i0,	i1,	ops) ;
      Build_OP (	TOP_convib_r_b,	b0_2_2,	Zero_TN,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_16_3,	b0_3_3,	r0_26_2,	r0_27_2,	b0_2_2,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	o0,	b0_3_4,	r0_24_3,	r0_25_3,	b0_3_3,	ops) ;
  }
} /* Expand__mulhw */

/*
@@@  case INTRN_MULHW:
@@@    Expand__mulhw(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of mull based on validated and scheduled basic assembly source.
*/
static void
Expand__mull(
 TN* ol0,
 TN* oh0,
 TN* il0,
 TN* ih0,
 TN* il1,
 TN* ih1,
 OPS* ops
)
{
  if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_mul32_r_r_r)
      && ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_mul64hu_r_r_r)) {
      TN *r0_17_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_20_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_20_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_21_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      Build_OP (    TOP_mul32_r_r_r,    r0_20_2,        il0,    ih1,    ops) ;
      Build_OP (    TOP_mul64hu_r_r_r,  r0_21_2,        il0,    il1,    ops) ;
      Build_OP (    TOP_mul32_r_r_r,    r0_17_3,        ih0,    il1,    ops) ;
      Build_OP (    TOP_mul32_r_r_r,    ol0,    il0,    il1,    ops) ;
      Build_OP (    TOP_add_r_r_r,      r0_20_3,        r0_20_2,        r0_21_2,        ops) ;
      Build_OP (    TOP_add_r_r_r,      oh0,    r0_17_3,        r0_20_3,        ops) ;      
  } else {
      TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_1_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *r0_17_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_17_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_17_6 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_24_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_25_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_26_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_27_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_28_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_28_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_29_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_30_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_30_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_31_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_32_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_32_6 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_33_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      Build_OP (	TOP_mullu_r_r_r,	r0_24_2,	il0,	il1,	ops) ;
      Build_OP (	TOP_mulhs_r_r_r,	r0_25_2,	il0,	il1,	ops) ;
      Build_OP (	TOP_cmplt_r_r_b,	b0_0_0,	il0,	Zero_TN,	ops) ;
      Build_OP (	TOP_cmplt_r_r_b,	b0_1_0,	il1,	Zero_TN,	ops) ;
      Build_OP (	TOP_mullhus_r_r_r,	r0_26_3,	il0,	il1,	ops) ;
      Build_OP (	TOP_mulhhs_r_r_r,	r0_27_3,	il0,	il1,	ops) ;
      Build_OP (	TOP_targ_slct_r_r_b_r,	r0_28_1,	b0_1_0,	il0,	Zero_TN,	ops) ;
      Build_OP (	TOP_targ_slct_r_r_b_r,	r0_29_1,	b0_0_0,	il1,	Zero_TN,	ops) ;
      Build_OP (	TOP_mullu_r_r_r,	r0_30_4,	ih1,	il0,	ops) ;
      Build_OP (	TOP_mulhs_r_r_r,	r0_31_4,	ih1,	il0,	ops) ;
      Build_OP (	TOP_convib_r_b,	b0_2_2,	Zero_TN,	ops) ;
      Build_OP (	TOP_add_r_r_r,	r0_28_2,	r0_28_1,	r0_29_1,	ops) ;
      Build_OP (	TOP_mullu_r_r_r,	r0_32_5,	ih0,	il1,	ops) ;
      Build_OP (	TOP_mulhs_r_r_r,	r0_33_5,	ih0,	il1,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	ol0,	b0_3_3,	r0_24_2,	r0_25_2,	b0_2_2,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_4,	b0_3_4,	r0_26_3,	r0_27_3,	b0_3_3,	ops) ;
      Build_OP (	TOP_add_r_r_r,	r0_30_5,	r0_30_4,	r0_31_4,	ops) ;
      Build_OP (	TOP_add_r_r_r,	r0_17_5,	r0_17_4,	r0_28_2,	ops) ;
      Build_OP (	TOP_add_r_r_r,	r0_32_6,	r0_32_5,	r0_33_5,	ops) ;
      Build_OP (	TOP_add_r_r_r,	r0_17_6,	r0_17_5,	r0_30_5,	ops) ;
      Build_OP (	TOP_add_r_r_r,	oh0,	r0_17_6,	r0_32_6,	ops) ;
  }
} /* Expand__mull */

/*
@@@  case INTRN_MULL:
@@@    Expand__mull(result[0],result[1],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
@@@  break ;
*/

/*
 * Expansion of mulm based on validated and scheduled basic assembly source.
*/
static void
Expand__mulm(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  Build_OP (	TOP_mull_r_r_r,	o0,	i0,	i1,	ops) ;
} /* Expand__mulm */

/*
@@@  case INTRN_MULM:
@@@    Expand__mulm(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of muln based on validated and scheduled basic assembly source.
*/
static void
Expand__muln(
 TN* ol0,
 TN* oh0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_mul32_r_r_r)
      && ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_mul64hu_r_r_r)) {
      Build_OP (    TOP_mul32_r_r_r,    ol0,    i0,     i1,     ops) ;
      Build_OP (    TOP_mul64h_r_r_r,   oh0,    i0,     i1,     ops) ;
  } else { 
      TN *b0_2_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *r0_24_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_25_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_26_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_27_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      Build_OP (	TOP_mullu_r_r_r,	r0_26_2,	i0,	i1,	ops) ;
      Build_OP (	TOP_mulhs_r_r_r,	r0_27_2,	i0,	i1,	ops) ;
      Build_OP (	TOP_mulhhs_r_r_r,	r0_24_3,	i0,	i1,	ops) ;
      Build_OP (	TOP_mullhus_r_r_r,	r0_25_3,	i0,	i1,	ops) ;
      Build_OP (	TOP_convib_r_b,	b0_2_2,	Zero_TN,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	ol0,	b0_3_3,	r0_26_2,	r0_27_2,	b0_2_2,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	oh0,	b0_3_4,	r0_24_3,	r0_25_3,	b0_3_3,	ops) ;
  }
} /* Expand__muln */

/*
@@@  case INTRN_MULN:
@@@    Expand__muln(result[0],result[1],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of muluh based on validated and scheduled basic assembly source.
*/
static void
Expand__muluh(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  Build_OP (	TOP_mulllu_r_r_r,	o0,	i0,	i1,	ops) ;
} /* Expand__muluh */

/*
@@@  case INTRN_MULUH:
@@@    Expand__muluh(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of mulul based on validated and scheduled basic assembly source.
*/
static void
Expand__mulul(
 TN* ol0,
 TN* oh0,
 TN* il0,
 TN* ih0,
 TN* il1,
 TN* ih1,
 OPS* ops
)
{
  if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_mul32_r_r_r)
      && ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_mul64hu_r_r_r)) {
      TN *r0_17_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_20_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_20_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_21_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      Build_OP (    TOP_mul32_r_r_r,    r0_20_2,        il0,    ih1,    ops) ;
      Build_OP (    TOP_mul64hu_r_r_r,  r0_21_2,        il0,    il1,    ops) ;
      Build_OP (    TOP_mul32_r_r_r,    r0_17_3,        ih0,    il1,    ops) ;
      Build_OP (    TOP_mul32_r_r_r,    ol0,    il0,    il1,    ops) ;
      Build_OP (    TOP_add_r_r_r,      r0_20_3,        r0_20_2,        r0_21_2,        ops) ;
      Build_OP (    TOP_add_r_r_r,      oh0,    r0_17_3,        r0_20_3,        ops) ;      
  } else {
      TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_1_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *r0_17_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_17_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_17_6 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_24_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_25_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_26_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_27_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_28_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_28_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_29_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_30_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_30_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_31_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_32_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_32_6 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_33_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
      TN *c0 = Gen_Literal_TN(__EXTS32TOS64(0), 4) ;
#undef __EXTS32TOS64
      Build_OP (	TOP_mullu_r_r_r,	r0_24_2,	il0,	il1,	ops) ;
      Build_OP (	TOP_mulhs_r_r_r,	r0_25_2,	il0,	il1,	ops) ;
      Build_OP (	TOP_cmplt_r_r_b,	b0_0_0,	il0,	Zero_TN,	ops) ;
      Build_OP (	TOP_cmplt_r_r_b,	b0_1_0,	il1,	Zero_TN,	ops) ;
      Build_OP (	TOP_mullhus_r_r_r,	r0_26_3,	il0,	il1,	ops) ;
      Build_OP (	TOP_mulhhs_r_r_r,	r0_27_3,	il0,	il1,	ops) ;
      Build_OP (	TOP_targ_slct_r_r_b_r,	r0_28_1,	b0_1_0,	il0,	Zero_TN,	ops) ;
      Build_OP (	TOP_targ_slct_r_r_b_r,	r0_29_1,	b0_0_0,	il1,	Zero_TN,	ops) ;
      Build_OP (	TOP_mullu_r_r_r,	r0_30_4,	ih1,	il0,	ops) ;
      Build_OP (	TOP_mulhs_r_r_r,	r0_31_4,	ih1,	il0,	ops) ;
      Build_OP (	TOP_convib_r_b,	b0_2_2,	Zero_TN,	ops) ;
      Build_OP (	TOP_add_r_r_r,	r0_28_2,	r0_28_1,	r0_29_1,	ops) ;
      Build_OP (	TOP_mullu_r_r_r,	r0_32_5,	ih0,	il1,	ops) ;
      Build_OP (	TOP_mulhs_r_r_r,	r0_33_5,	ih0,	il1,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	ol0,	b0_3_3,	r0_24_2,	r0_25_2,	b0_2_2,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_4,	b0_3_4,	r0_26_3,	r0_27_3,	b0_3_3,	ops) ;
      Build_OP (	TOP_add_r_r_r,	r0_30_5,	r0_30_4,	r0_31_4,	ops) ;
      Build_OP (	TOP_add_r_r_r,	r0_17_5,	r0_17_4,	r0_28_2,	ops) ;
      Build_OP (	TOP_add_r_r_r,	r0_32_6,	r0_32_5,	r0_33_5,	ops) ;
      Build_OP (	TOP_add_r_r_r,	r0_17_6,	r0_17_5,	r0_30_5,	ops) ;
      Build_OP (	TOP_add_r_r_r,	oh0,	r0_17_6,	r0_32_6,	ops) ;
  }
} /* Expand__mulul */

/*
@@@  case INTRN_MULUL:
@@@    Expand__mulul(result[0],result[1],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
@@@  break ;
*/

/*
 * Expansion of mulum based on validated and scheduled basic assembly source.
*/
static void
Expand__mulum(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_mullu_r_r_r)) {
    Build_OP (	TOP_mullu_r_r_r,	o0,	i0,	i1,	ops) ;
  } else {
    TN *r0_17_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    Build_OP (	TOP_zxth_r_r,	r0_17_0,	i1,	ops) ;
    Build_OP (	TOP_mul32_r_r_r,	o0,	i0,	r0_17_0,	ops) ;
  }
} /* Expand__mulum */

/*
@@@  case INTRN_MULUM:
@@@    Expand__mulum(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of mulun based on validated and scheduled basic assembly source.
*/
static void
Expand__mulun(
 TN* ol0,
 TN* oh0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_mul32_r_r_r)
      && ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_mul64hu_r_r_r)) {
      Build_OP (    TOP_mul32_r_r_r,    ol0,    i0,     i1,     ops) ;
      Build_OP (    TOP_mul64hu_r_r_r,   oh0,    i0,     i1,     ops) ;
  } else {
      TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_1_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_2_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *b0_3_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
      TN *r0_17_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_24_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_25_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_26_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_27_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_28_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_28_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_29_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      Build_OP (	TOP_mullu_r_r_r,	r0_26_2,	i0,	i1,	ops) ;
      Build_OP (	TOP_mulhs_r_r_r,	r0_27_2,	i0,	i1,	ops) ;
      Build_OP (	TOP_cmplt_r_r_b,	b0_0_0,	i0,	Zero_TN,	ops) ;
      Build_OP (	TOP_cmplt_r_r_b,	b0_1_0,	i1,	Zero_TN,	ops) ;
      Build_OP (	TOP_mulhhs_r_r_r,	r0_24_3,	i0,	i1,	ops) ;
      Build_OP (	TOP_mullhus_r_r_r,	r0_25_3,	i0,	i1,	ops) ;
      Build_OP (	TOP_targ_slct_r_r_b_r,	r0_28_1,	b0_1_0,	i0,	Zero_TN,	ops) ;
      Build_OP (	TOP_targ_slct_r_r_b_r,	r0_29_1,	b0_0_0,	i1,	Zero_TN,	ops) ;
      Build_OP (	TOP_add_r_r_r,	r0_28_2,	r0_28_1,	r0_29_1,	ops) ;
      Build_OP (	TOP_convib_r_b,	b0_2_2,	Zero_TN,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	ol0,	b0_3_3,	r0_26_2,	r0_27_2,	b0_2_2,	ops) ;
      Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_17_4,	b0_3_4,	r0_24_3,	r0_25_3,	b0_3_3,	ops) ;
      Build_OP (	TOP_add_r_r_r,	oh0,	r0_17_4,	r0_28_2,	ops) ;
  }
} /* Expand__mulun */

/*
@@@  case INTRN_MULUN:
@@@    Expand__mulun(result[0],result[1],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of muluw based on validated and scheduled basic assembly source.
*/
static void
Expand__muluw(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_mul32_r_r_r)) {
      Build_OP (	TOP_mul32_r_r_r,	o0,	i0,	i1,	ops) ;
  } else {
      TN *r0_24_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_25_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      Build_OP (	TOP_mullu_r_r_r,	r0_24_2,	i0,	i1,	ops) ;
      Build_OP (	TOP_mulhs_r_r_r,	r0_25_2,	i0,	i1,	ops) ;
      Build_OP (	TOP_add_r_r_r,	o0,	r0_24_2,	r0_25_2,	ops) ;
  }
} /* Expand__muluw */

/*
@@@  case INTRN_MULUW:
@@@    Expand__muluw(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of mulw based on validated and scheduled basic assembly source.
*/
static void
Expand__mulw(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_mul32_r_r_r)) {
      Build_OP (	TOP_mul32_r_r_r,	o0,	i0,	i1,	ops) ;
  } else {
      TN *r0_24_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      TN *r0_25_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
      Build_OP (	TOP_mullu_r_r_r,	r0_24_2,	i0,	i1,	ops) ;
      Build_OP (	TOP_mulhs_r_r_r,	r0_25_2,	i0,	i1,	ops) ;
      Build_OP (	TOP_add_r_r_r,	o0,	r0_24_2,	r0_25_2,	ops) ;
  }
} /* Expand__mulw */

/*
@@@  case INTRN_MULW:
@@@    Expand__mulw(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of nearclw based on validated and scheduled basic assembly source.
*/
static void
Expand__nearclw(
 TN* o0,
 TN* il0,
 TN* ih0,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_20_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c0x7fffffff = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *c0x7fffffff_ltn = Gen_Literal_TN(__EXTS32TOS64(0x7fffffff), 4) ;
  Build_OP (    TOP_mov_ii_r,      c0x7fffffff,    c0x7fffffff_ltn,        ops) ;
  TN *c0x80000000 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *c0x80000000_ltn = Gen_Literal_TN(__EXTS32TOS64(0x80000000), 4) ;
  Build_OP (    TOP_mov_ii_r,      c0x80000000,    c0x80000000_ltn,        ops) ;
  TN *c31 = Gen_Literal_TN(__EXTS32TOS64(31), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_cmpeq_r_r_b,	b0_0_0,	ih0,	c0x7fffffff,	ops) ;
  Build_OP (	TOP_shru_i_r_r,	r0_21_0,	il0,	c31,	ops) ;
  Build_OP (	TOP_add_r_r_r,	r0_21_1,	ih0,	r0_21_0,	ops) ;
  Build_OP (	TOP_cmpeq_r_r_r,	r0_20_1,	il0,	c0x80000000,	ops) ;
  Build_OP (	TOP_andc_r_r_r,	r0_21_2,	r0_20_1,	r0_21_1,	ops) ;
  Build_OP (	TOP_targ_slct_r_r_b_r,	o0,	b0_0_0,	ih0,	r0_21_2,	ops) ;
} /* Expand__nearclw */

/*
@@@  case INTRN_NEARCLW:
@@@    Expand__nearclw(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of nearcwh based on validated and scheduled basic assembly source.
*/
static void
Expand__nearcwh(
 TN* o0,
 TN* i0,
 OPS* ops
)
{
  TN *b0_0_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_22_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_22_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c0x7fff0000 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *c0x7fff0000_ltn = Gen_Literal_TN(__EXTS32TOS64(0x7fff0000), 4) ;
  Build_OP (    TOP_mov_ii_r,      c0x7fff0000,    c0x7fff0000_ltn,        ops) ;
  TN *c0x8000 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *c0x8000_ltn = Gen_Literal_TN(__EXTS32TOS64(0x8000), 4) ;
  Build_OP (    TOP_mov_ii_r,      c0x8000,        c0x8000_ltn,    ops) ;
  TN *c0xffff = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *c0xffff_ltn = Gen_Literal_TN(__EXTS32TOS64(0xffff), 4) ;
  Build_OP (    TOP_mov_ii_r,      c0xffff,        c0xffff_ltn,    ops) ;
  TN *c16 = Gen_Literal_TN(__EXTS32TOS64(16), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_add_r_r_r,	r0_20_0,	i0,	c0x8000,	ops) ;
  Build_OP (	TOP_and_r_r_r,	r0_22_0,	i0,	c0xffff,	ops) ;
  Build_OP (	TOP_shr_i_r_r,	r0_20_1,	r0_20_0,	c16,	ops) ;
  Build_OP (	TOP_shr_i_r_r,	r0_21_1,	i0,	c16,	ops) ;
  Build_OP (	TOP_cmpeq_r_r_r,	r0_22_1,	r0_22_0,	c0x8000,	ops) ;
  Build_OP (	TOP_cmpge_r_r_b,	b0_0_2,	i0,	c0x7fff0000,	ops) ;
  Build_OP (	TOP_andc_r_r_r,	r0_20_2,	r0_22_1,	r0_20_1,	ops) ;
  Build_OP (	TOP_targ_slct_r_r_b_r,	o0,	b0_0_2,	r0_21_1,	r0_20_2,	ops) ;
} /* Expand__nearcwh */

/*
@@@  case INTRN_NEARCWH:
@@@    Expand__nearcwh(result[0],opnd[0],ops) ;
@@@  break ;
*/

/*
 * Expansion of nearlw based on validated and scheduled basic assembly source.
*/
static void
Expand__nearlw(
 TN* o0,
 TN* il0,
 TN* ih0,
 OPS* ops
)
{
  TN *r0_16_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c0x80000000 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *c0x80000000_ltn = Gen_Literal_TN(__EXTS32TOS64(0x80000000), 4) ;
  Build_OP (    TOP_mov_ii_r,      c0x80000000,    c0x80000000_ltn,        ops) ;
  TN *c31 = Gen_Literal_TN(__EXTS32TOS64(31), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_shru_i_r_r,	r0_21_0,	il0,	c31,	ops) ;
  Build_OP (	TOP_add_r_r_r,	r0_16_1,	ih0,	r0_21_0,	ops) ;
  Build_OP (	TOP_cmpeq_r_r_r,	r0_20_1,	il0,	c0x80000000,	ops) ;
  Build_OP (	TOP_andc_r_r_r,	o0,	r0_20_1,	r0_16_1,	ops) ;
} /* Expand__nearlw */

/*
@@@  case INTRN_NEARLW:
@@@    Expand__nearlw(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of nearwh based on validated and scheduled basic assembly source.
*/
static void
Expand__nearwh(
 TN* o0,
 TN* i0,
 OPS* ops
)
{
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_22_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_22_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c0x8000 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *c0x8000_ltn = Gen_Literal_TN(__EXTS32TOS64(0x8000), 4) ;
  Build_OP (    TOP_mov_ii_r,      c0x8000,        c0x8000_ltn,    ops) ;
  TN *c0xffff = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *c0xffff_ltn = Gen_Literal_TN(__EXTS32TOS64(0xffff), 4) ;
  Build_OP (    TOP_mov_ii_r,      c0xffff,        c0xffff_ltn,    ops) ;
  TN *c16 = Gen_Literal_TN(__EXTS32TOS64(16), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_add_r_r_r,	r0_21_0,	i0,	c0x8000,	ops) ;
  Build_OP (	TOP_and_r_r_r,	r0_22_0,	i0,	c0xffff,	ops) ;
  Build_OP (	TOP_cmpeq_r_r_r,	r0_22_1,	r0_22_0,	c0x8000,	ops) ;
  Build_OP (	TOP_shr_i_r_r,	r0_21_1,	r0_21_0,	c16,	ops) ;
  Build_OP (	TOP_andc_r_r_r,	o0,	r0_22_1,	r0_21_1,	ops) ;
} /* Expand__nearwh */

/*
@@@  case INTRN_NEARWH:
@@@    Expand__nearwh(result[0],opnd[0],ops) ;
@@@  break ;
*/

/*
 * Expansion of negch based on validated and scheduled basic assembly source.
*/
static void
Expand__negch(
 TN* o0,
 TN* i0,
 OPS* ops
)
{
  if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_subs_ph_r_r_r)) { 
    TN *r0_16_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    Build_OP (	TOP_subs_ph_r_r_r,	r0_16_0,	Zero_TN,	i0,	ops) ;
    Build_OP (	TOP_sxth_r_r,	o0,	r0_16_0,	ops) ;
  } else {
    TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
    TN *c0x7fff = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *c0x7fff_ltn = Gen_Literal_TN(__EXTS32TOS64(0x7fff), 4) ;
    Build_OP (    TOP_mov_ii_r,      c0x7fff,        c0x7fff_ltn,    ops) ;
    TN *c0xffff8000 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *c0xffff8000_ltn = Gen_Literal_TN(__EXTS32TOS64(0xffff8000), 4) ;
    Build_OP (    TOP_mov_ii_r,      c0xffff8000,    c0xffff8000_ltn,        ops) ;
#undef __EXTS32TOS64
    Build_OP (	TOP_cmpeq_r_r_b,	b0_0_0,	i0,	c0xffff8000,	ops) ;
    Build_OP (	TOP_sub_r_r_r,	r0_20_0,	Zero_TN,	i0,	ops) ;
    Build_OP (	TOP_slctf_r_r_b_r,	o0,	b0_0_0,	r0_20_0,	c0x7fff,	ops) ;
  }
} /* Expand__negch */

/*
@@@  case INTRN_NEGCH:
@@@    Expand__negch(result[0],opnd[0],ops) ;
@@@  break ;
*/

/*
 * Expansion of negcl based on validated and scheduled basic assembly source.
*/
static void
Expand__negcl(
 TN* ol0,
 TN* oh0,
 TN* il0,
 TN* ih0,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_0_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_22_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_22_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_23_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c0x7fffffff = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *c0x7fffffff_ltn = Gen_Literal_TN(__EXTS32TOS64(0x7fffffff), 4) ;
  Build_OP (    TOP_mov_ii_r,      c0x7fffffff,    c0x7fffffff_ltn,        ops) ;
  TN *c0x80000000 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *c0x80000000_ltn = Gen_Literal_TN(__EXTS32TOS64(0x80000000), 4) ;
  Build_OP (    TOP_mov_ii_r,      c0x80000000,    c0x80000000_ltn,        ops) ;
  TN *c_1 = Gen_Literal_TN(__EXTS32TOS64(-1), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_cmpgtu_r_r_b,	b0_0_0,	il0,	Zero_TN,	ops) ;
  Build_OP (	TOP_sub_r_r_r,	r0_20_0,	Zero_TN,	il0,	ops) ;
  Build_OP (	TOP_sub_r_r_r,	r0_21_0,	Zero_TN,	ih0,	ops) ;
  Build_OP (	TOP_sub_r_i_r,	r0_22_0,	c_1,	ih0,	ops) ;
  Build_OP (	TOP_targ_slct_r_r_b_r,	r0_21_1,	b0_0_0,	r0_22_0,	r0_21_0,	ops) ;
  Build_OP (	TOP_cmpeq_r_r_r,	r0_22_1,	ih0,	c0x80000000,	ops) ;
  Build_OP (	TOP_cmpeq_r_r_r,	r0_23_1,	il0,	Zero_TN,	ops) ;
  Build_OP (	TOP_andl_r_r_b,	b0_0_2,	r0_22_1,	r0_23_1,	ops) ;
  Build_OP (	TOP_slctf_i_r_b_r,	ol0,	b0_0_2,	r0_20_0,	c_1,	ops) ;
  Build_OP (	TOP_slctf_r_r_b_r,	oh0,	b0_0_2,	r0_21_1,	c0x7fffffff,	ops) ;
} /* Expand__negcl */

/*
@@@  case INTRN_NEGCL:
@@@    Expand__negcl(result[0],result[1],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of negcw based on validated and scheduled basic assembly source.
*/
static void
Expand__negcw(
 TN* o0,
 TN* i0,
 OPS* ops
)
{
  if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_subs_r_r_r)) { 
    Build_OP (	TOP_subs_r_r_r,	o0,	Zero_TN,	i0,	ops) ;
  } else {
    TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
    TN *c0x7fffffff = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *c0x7fffffff_ltn = Gen_Literal_TN(__EXTS32TOS64(0x7fffffff), 4) ;
    Build_OP (    TOP_mov_ii_r,      c0x7fffffff,    c0x7fffffff_ltn,        ops) ;
    TN *c0x80000000 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *c0x80000000_ltn = Gen_Literal_TN(__EXTS32TOS64(0x80000000), 4) ;
    Build_OP (    TOP_mov_ii_r,      c0x80000000,    c0x80000000_ltn,        ops) ;
#undef __EXTS32TOS64
    Build_OP (	TOP_cmpeq_r_r_b,	b0_0_0,	i0,	c0x80000000,	ops) ;
    Build_OP (	TOP_sub_r_r_r,	r0_20_0,	Zero_TN,	i0,	ops) ;
    Build_OP (	TOP_slctf_r_r_b_r,	o0,	b0_0_0,	r0_20_0,	c0x7fffffff,	ops) ;
  }
} /* Expand__negcw */

/*
@@@  case INTRN_NEGCW:
@@@    Expand__negcw(result[0],opnd[0],ops) ;
@@@  break ;
*/

/*
 * Expansion of negl based on validated and scheduled basic assembly source.
*/
static void
Expand__negl(
 TN* ol0,
 TN* oh0,
 TN* il0,
 TN* ih0,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_18_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_19_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c_1 = Gen_Literal_TN(__EXTS32TOS64(-1), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_cmpgtu_r_r_b,	b0_0_0,	il0,	Zero_TN,	ops) ;
  Build_OP (	TOP_sub_r_r_r,	ol0,	Zero_TN,	il0,	ops) ;
  Build_OP (	TOP_sub_r_r_r,	r0_19_0,	Zero_TN,	ih0,	ops) ;
  Build_OP (	TOP_sub_r_i_r,	r0_18_0,	c_1,	ih0,	ops) ;
  Build_OP (	TOP_targ_slct_r_r_b_r,	oh0,	b0_0_0,	r0_18_0,	r0_19_0,	ops) ;
} /* Expand__negl */

/*
@@@  case INTRN_NEGL:
@@@    Expand__negl(result[0],result[1],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of negul based on validated and scheduled basic assembly source.
*/
static void
Expand__negul(
 TN* ol0,
 TN* oh0,
 TN* il0,
 TN* ih0,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_18_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_19_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c_1 = Gen_Literal_TN(__EXTS32TOS64(-1), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_cmpgtu_r_r_b,	b0_0_0,	il0,	Zero_TN,	ops) ;
  Build_OP (	TOP_sub_r_r_r,	ol0,	Zero_TN,	il0,	ops) ;
  Build_OP (	TOP_sub_r_r_r,	r0_19_0,	Zero_TN,	ih0,	ops) ;
  Build_OP (	TOP_sub_r_i_r,	r0_18_0,	c_1,	ih0,	ops) ;
  Build_OP (	TOP_targ_slct_r_r_b_r,	oh0,	b0_0_0,	r0_18_0,	r0_19_0,	ops) ;
} /* Expand__negul */

/*
@@@  case INTRN_NEGUL:
@@@    Expand__negul(result[0],result[1],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of nel based on validated and scheduled basic assembly source.
*/
static void
Expand__nel(
 TN* o0,
 TN* il0,
 TN* ih0,
 TN* il1,
 TN* ih1,
 OPS* ops
)
{
  TN *r0_16_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_17_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  Build_OP (	TOP_cmpeq_r_r_r,	r0_16_0,	il0,	il1,	ops) ;
  Build_OP (	TOP_cmpeq_r_r_r,	r0_17_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_nandl_r_r_r,	o0,	r0_16_0,	r0_17_0,	ops) ;
} /* Expand__nel */

/*
@@@  case INTRN_NEL:
@@@    Expand__nel(result[0],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
@@@  break ;
*/

/*
 * Expansion of neul based on validated and scheduled basic assembly source.
*/
static void
Expand__neul(
 TN* o0,
 TN* il0,
 TN* ih0,
 TN* il1,
 TN* ih1,
 OPS* ops
)
{
  TN *r0_16_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_17_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  Build_OP (	TOP_cmpeq_r_r_r,	r0_16_0,	il0,	il1,	ops) ;
  Build_OP (	TOP_cmpeq_r_r_r,	r0_17_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_nandl_r_r_r,	o0,	r0_16_0,	r0_17_0,	ops) ;
} /* Expand__neul */

/*
@@@  case INTRN_NEUL:
@@@    Expand__neul(result[0],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
@@@  break ;
*/

/*
 * Expansion of normh based on validated and scheduled basic assembly source.
*/
static void
Expand__normh(
 TN* o0,
 TN* i0,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_16_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_16_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c0 = Gen_Literal_TN(__EXTS32TOS64(0), 4) ;
  TN *c_17 = Gen_Literal_TN(__EXTS32TOS64(-17), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_orc_r_r_r,	r0_20_0,	i0,	Zero_TN,	ops) ;
  Build_OP (	TOP_cmpeq_r_r_b,	b0_0_0,	i0,	Zero_TN,	ops) ;
  Build_OP (	TOP_max_r_r_r,	r0_20_1,	i0,	r0_20_0,	ops) ;
  Build_OP (	TOP_clz_r_r,	r0_16_2,	r0_20_1,	ops) ;
  Build_OP (	TOP_add_i_r_r,	r0_16_3,	r0_16_2,	c_17,	ops) ;
  Build_OP (	TOP_targ_slct_r_r_b_r,	o0,	b0_0_0,	Zero_TN,	r0_16_3,	ops) ;
} /* Expand__normh */

/*
@@@  case INTRN_NORMH:
@@@    Expand__normh(result[0],opnd[0],ops) ;
@@@  break ;
*/

/*
 * Expansion of norml based on validated and scheduled basic assembly source.
*/
static void
Expand__norml(
 TN* o0,
 TN* il0,
 TN* ih0,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_0_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_1_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_16_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_16_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_22_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_22_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_23_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_23_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_24_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_25_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c32 = Gen_Literal_TN(__EXTS32TOS64(32), 4) ;
  TN *c_1 = Gen_Literal_TN(__EXTS32TOS64(-1), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_orc_r_r_r,	r0_22_0,	il0,	Zero_TN,	ops) ;
  Build_OP (	TOP_orc_r_r_r,	r0_23_0,	ih0,	Zero_TN,	ops) ;
  Build_OP (	TOP_cmplt_r_r_b,	b0_0_0,	ih0,	Zero_TN,	ops) ;
  Build_OP (	TOP_cmpeq_r_r_r,	r0_24_0,	il0,	Zero_TN,	ops) ;
  Build_OP (	TOP_cmpeq_r_r_r,	r0_25_1,	ih0,	Zero_TN,	ops) ;
  Build_OP (	TOP_targ_slct_r_r_b_r,	r0_22_1,	b0_0_0,	r0_22_0,	il0,	ops) ;
  Build_OP (	TOP_targ_slct_r_r_b_r,	r0_23_1,	b0_0_0,	r0_23_0,	ih0,	ops) ;
  Build_OP (	TOP_clz_r_r,	r0_20_2,	r0_23_1,	ops) ;
  Build_OP (	TOP_clz_r_r,	r0_21_2,	r0_22_1,	ops) ;
  Build_OP (	TOP_cmpeq_r_r_b,	b0_0_2,	r0_23_1,	Zero_TN,	ops) ;
  Build_OP (	TOP_andl_r_r_b,	b0_1_3,	r0_24_0,	r0_25_1,	ops) ;
  Build_OP (	TOP_add_i_r_r,	r0_21_3,	r0_21_2,	c32,	ops) ;
  Build_OP (	TOP_targ_slct_r_r_b_r,	r0_16_4,	b0_0_2,	r0_21_3,	r0_20_2,	ops) ;
  Build_OP (	TOP_add_i_r_r,	r0_16_5,	r0_16_4,	c_1,	ops) ;
  Build_OP (	TOP_targ_slct_r_r_b_r,	o0,	b0_1_3,	Zero_TN,	r0_16_5,	ops) ;
} /* Expand__norml */

/*
@@@  case INTRN_NORML:
@@@    Expand__norml(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of normw based on validated and scheduled basic assembly source.
*/
static void
Expand__normw(
 TN* o0,
 TN* i0,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_16_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_16_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c_1 = Gen_Literal_TN(__EXTS32TOS64(-1), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_orc_r_r_r,	r0_20_0,	i0,	Zero_TN,	ops) ;
  Build_OP (	TOP_cmpeq_r_r_b,	b0_0_0,	i0,	Zero_TN,	ops) ;
  Build_OP (	TOP_max_r_r_r,	r0_20_1,	i0,	r0_20_0,	ops) ;
  Build_OP (	TOP_clz_r_r,	r0_16_2,	r0_20_1,	ops) ;
  Build_OP (	TOP_add_i_r_r,	r0_16_3,	r0_16_2,	c_1,	ops) ;
  Build_OP (	TOP_targ_slct_r_r_b_r,	o0,	b0_0_0,	Zero_TN,	r0_16_3,	ops) ;
} /* Expand__normw */

/*
@@@  case INTRN_NORMW:
@@@    Expand__normw(result[0],opnd[0],ops) ;
@@@  break ;
*/

/*
 * Expansion of parityl based on validated and scheduled basic assembly source.
*/
static void
Expand__parityl(
 TN* o0,
 TN* il0,
 TN* ih0,
 OPS* ops
)
{
  TN *r0_16_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_16_10 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_16_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_16_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_16_7 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_16_9 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_17_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_17_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_17_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_17_7 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_17_9 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_18_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_18_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_18_6 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_18_8 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_19_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_19_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_19_6 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_19_8 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)                (((long long)(x)<<32) >> 32)
  TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
  TN *c16 = Gen_Literal_TN(__EXTS32TOS64(16), 4) ;
  TN *c2 = Gen_Literal_TN(__EXTS32TOS64(2), 4) ;
  TN *c4 = Gen_Literal_TN(__EXTS32TOS64(4), 4) ;
  TN *c8 = Gen_Literal_TN(__EXTS32TOS64(8), 4) ;
#undef __EXTS32TOS64
  Build_OP (    TOP_shru_i_r_r, r0_21_0,        il0,    c1,     ops) ;
  Build_OP (    TOP_shru_i_r_r, r0_20_0,        ih0,    c1,     ops) ;
  Build_OP (    TOP_xor_r_r_r,  r0_16_1,        r0_21_0,        il0,    ops) ;
  Build_OP (    TOP_xor_r_r_r,  r0_17_1,        r0_20_0,        ih0,    ops) ;
  Build_OP (    TOP_shru_i_r_r, r0_19_2,        r0_16_1,        c2,     ops) ;
  Build_OP (    TOP_shru_i_r_r, r0_18_2,        r0_17_1,        c2,     ops) ;
  Build_OP (    TOP_xor_r_r_r,  r0_16_3,        r0_16_1,        r0_19_2,        ops) ;
  Build_OP (    TOP_xor_r_r_r,  r0_17_3,        r0_17_1,        r0_18_2,        ops) ;
  Build_OP (    TOP_shru_i_r_r, r0_19_4,        r0_16_3,        c4,     ops) ;
  Build_OP (    TOP_shru_i_r_r, r0_18_4,        r0_17_3,        c4,     ops) ;
  Build_OP (    TOP_xor_r_r_r,  r0_16_5,        r0_16_3,        r0_19_4,        ops) ;
  Build_OP (    TOP_xor_r_r_r,  r0_17_5,        r0_17_3,        r0_18_4,        ops) ;
  Build_OP (    TOP_shru_i_r_r, r0_19_6,        r0_16_5,        c8,     ops) ;
  Build_OP (    TOP_shru_i_r_r, r0_18_6,        r0_17_5,        c8,     ops) ;
  Build_OP (    TOP_xor_r_r_r,  r0_16_7,        r0_16_5,        r0_19_6,        ops) ;
  Build_OP (    TOP_xor_r_r_r,  r0_17_7,        r0_17_5,        r0_18_6,        ops) ;
  Build_OP (    TOP_shru_i_r_r, r0_19_8,        r0_16_7,        c16,    ops) ;
  Build_OP (    TOP_shru_i_r_r, r0_18_8,        r0_17_7,        c16,    ops) ;
  Build_OP (    TOP_xor_r_r_r,  r0_16_9,        r0_16_7,        r0_19_8,        ops) ;
  Build_OP (    TOP_xor_r_r_r,  r0_17_9,        r0_17_7,        r0_18_8,        ops) ;
  Build_OP (    TOP_xor_r_r_r,  r0_16_10,       r0_16_9,        r0_17_9,        ops) ;
  Build_OP (    TOP_and_i_r_r,  o0,     r0_16_10,       c1,     ops) ;
} /* Expand__parityl */
 
/*
@@@  case INTRN_PARITY64:
@@@    Expand__parityl(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of parityw based on validated and scheduled basic assembly source.
*/
static void
Expand__parityw(
 TN* o0,
 TN* i0,
 OPS* ops
)
{
  TN *r0_16_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_16_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_16_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_16_7 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_16_9 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_17_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_17_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_17_6 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_17_8 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_18_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)                (((long long)(x)<<32) >> 32)
  TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
  TN *c16 = Gen_Literal_TN(__EXTS32TOS64(16), 4) ;
  TN *c2 = Gen_Literal_TN(__EXTS32TOS64(2), 4) ;
  TN *c4 = Gen_Literal_TN(__EXTS32TOS64(4), 4) ;
  TN *c8 = Gen_Literal_TN(__EXTS32TOS64(8), 4) ;
#undef __EXTS32TOS64
  Build_OP (    TOP_shru_i_r_r, r0_18_0,        i0,     c1,     ops) ;
  Build_OP (    TOP_xor_r_r_r,  r0_16_1,        i0,     r0_18_0,        ops) ;
  Build_OP (    TOP_shru_i_r_r, r0_17_2,        r0_16_1,        c2,     ops) ;
  Build_OP (    TOP_xor_r_r_r,  r0_16_3,        r0_16_1,        r0_17_2,        ops) ;
  Build_OP (    TOP_shru_i_r_r, r0_17_4,        r0_16_3,        c4,     ops) ;
  Build_OP (    TOP_xor_r_r_r,  r0_16_5,        r0_16_3,        r0_17_4,        ops) ;
  Build_OP (    TOP_shru_i_r_r, r0_17_6,        r0_16_5,        c8,     ops) ;
  Build_OP (    TOP_xor_r_r_r,  r0_16_7,        r0_16_5,        r0_17_6,        ops) ;
  Build_OP (    TOP_shru_i_r_r, r0_17_8,        r0_16_7,        c16,    ops) ;
  Build_OP (    TOP_xor_r_r_r,  r0_16_9,        r0_16_7,        r0_17_8,        ops) ;
  Build_OP (    TOP_and_i_r_r,  o0,     r0_16_9,        c1,     ops) ;
} /* Expand__parityw */
 
/*
@@@  case INTRN_PARITY32:
@@@    Expand__parityw(result[0],opnd[0],ops) ;
@@@  break ;
*/

/*
 * Expansion of priorh based on validated and scheduled basic assembly source.
*/
static void
Expand__priorh(
 TN* o0,
 TN* i0,
 OPS* ops
)
{
  TN *r0_16_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c_17 = Gen_Literal_TN(__EXTS32TOS64(-17), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_orc_r_r_r,	r0_20_0,	i0,	Zero_TN,	ops) ;
  Build_OP (	TOP_max_r_r_r,	r0_20_1,	i0,	r0_20_0,	ops) ;
  Build_OP (	TOP_clz_r_r,	r0_16_2,	r0_20_1,	ops) ;
  Build_OP (	TOP_add_i_r_r,	o0,	r0_16_2,	c_17,	ops) ;
} /* Expand__priorh */

/*
@@@  case INTRN_PRIORH:
@@@    Expand__priorh(result[0],opnd[0],ops) ;
@@@  break ;
*/

/*
 * Expansion of priorl based on validated and scheduled basic assembly source.
*/
static void
Expand__priorl(
 TN* o0,
 TN* il0,
 TN* ih0,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_0_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_16_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_22_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_22_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_23_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_23_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c32 = Gen_Literal_TN(__EXTS32TOS64(32), 4) ;
  TN *c_1 = Gen_Literal_TN(__EXTS32TOS64(-1), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_orc_r_r_r,	r0_22_0,	il0,	Zero_TN,	ops) ;
  Build_OP (	TOP_orc_r_r_r,	r0_23_0,	ih0,	Zero_TN,	ops) ;
  Build_OP (	TOP_cmplt_r_r_b,	b0_0_0,	ih0,	Zero_TN,	ops) ;
  Build_OP (	TOP_targ_slct_r_r_b_r,	r0_22_1,	b0_0_0,	r0_22_0,	il0,	ops) ;
  Build_OP (	TOP_targ_slct_r_r_b_r,	r0_23_1,	b0_0_0,	r0_23_0,	ih0,	ops) ;
  Build_OP (	TOP_clz_r_r,	r0_20_2,	r0_23_1,	ops) ;
  Build_OP (	TOP_clz_r_r,	r0_21_2,	r0_22_1,	ops) ;
  Build_OP (	TOP_cmpeq_r_r_b,	b0_0_2,	r0_23_1,	Zero_TN,	ops) ;
  Build_OP (	TOP_add_i_r_r,	r0_21_3,	r0_21_2,	c32,	ops) ;
  Build_OP (	TOP_targ_slct_r_r_b_r,	r0_16_4,	b0_0_2,	r0_21_3,	r0_20_2,	ops) ;
  Build_OP (	TOP_add_i_r_r,	o0,	r0_16_4,	c_1,	ops) ;
} /* Expand__priorl */

/*
@@@  case INTRN_PRIORL:
@@@    Expand__priorl(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of priorw based on validated and scheduled basic assembly source.
*/
static void
Expand__priorw(
 TN* o0,
 TN* i0,
 OPS* ops
)
{
  TN *r0_16_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c_1 = Gen_Literal_TN(__EXTS32TOS64(-1), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_orc_r_r_r,	r0_20_0,	i0,	Zero_TN,	ops) ;
  Build_OP (	TOP_max_r_r_r,	r0_20_1,	i0,	r0_20_0,	ops) ;
  Build_OP (	TOP_clz_r_r,	r0_16_2,	r0_20_1,	ops) ;
  Build_OP (	TOP_add_i_r_r,	o0,	r0_16_2,	c_1,	ops) ;
} /* Expand__priorw */

/*
@@@  case INTRN_PRIORW:
@@@    Expand__priorw(result[0],opnd[0],ops) ;
@@@  break ;
*/

/*
 * Expansion of puthl based on validated and scheduled basic assembly source.
*/
static void
Expand__puthl(
 TN* ol0,
 TN* oh0,
 TN* i0,
 OPS* ops
)
{
  Build_OP (	TOP_add_r_r_r,	oh0,	i0,	Zero_TN,	ops) ;
  Build_OP (	TOP_add_r_r_r,	ol0,	Zero_TN,	Zero_TN,	ops) ;
} /* Expand__puthl */

/*
@@@  case INTRN_PUTHL:
@@@    Expand__puthl(result[0],result[1],opnd[0],ops) ;
@@@  break ;
*/

/*
 * Expansion of puthw based on validated and scheduled basic assembly source.
*/
static void
Expand__puthw(
 TN* o0,
 TN* i0,
 OPS* ops
)
{
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c16 = Gen_Literal_TN(__EXTS32TOS64(16), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_shl_i_r_r,	o0,	i0,	c16,	ops) ;
} /* Expand__puthw */

/*
@@@  case INTRN_PUTHW:
@@@    Expand__puthw(result[0],opnd[0],ops) ;
@@@  break ;
*/

/*
 * Expansion of putll based on validated and scheduled basic assembly source.
*/
static void
Expand__putll(
 TN* ol0,
 TN* oh0,
 TN* i0,
 OPS* ops
)
{
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c31 = Gen_Literal_TN(__EXTS32TOS64(31), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_shr_i_r_r,	oh0,	i0,	c31,	ops) ;
  Build_OP (	TOP_add_r_r_r,	ol0,	i0,	Zero_TN,	ops) ;
} /* Expand__putll */

/*
@@@  case INTRN_PUTLL:
@@@    Expand__putll(result[0],result[1],opnd[0],ops) ;
@@@  break ;
*/

/*
 * Expansion of putlw based on validated and scheduled basic assembly source.
*/
static void
Expand__putlw(
 TN* o0,
 TN* i0,
 OPS* ops
)
{
  Build_OP (	TOP_add_r_r_r,	o0,	i0,	Zero_TN,	ops) ;
} /* Expand__putlw */

/*
@@@  case INTRN_PUTLW:
@@@    Expand__putlw(result[0],opnd[0],ops) ;
@@@  break ;
*/

/*
 * Expansion of rotlh based on validated and scheduled basic assembly source.
*/
static void
Expand__rotlh(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c16 = Gen_Literal_TN(__EXTS32TOS64(16), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_shl_r_r_r,	r0_20_0,	i0,	i1,	ops) ;
  Build_OP (	TOP_sub_r_i_r,	r0_21_0,	c16,	i1,	ops) ;
  Build_OP (	TOP_shru_r_r_r,	r0_21_1,	i0,	r0_21_0,	ops) ;
  Build_OP (	TOP_or_r_r_r,	o0,	r0_20_0,	r0_21_1,	ops) ;
} /* Expand__rotlh */

/*
@@@  case INTRN_ROTLH:
@@@    Expand__rotlh(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of rotlw based on validated and scheduled basic assembly source.
*/
static void
Expand__rotlw(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c32 = Gen_Literal_TN(__EXTS32TOS64(32), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_shl_r_r_r,	r0_20_0,	i0,	i1,	ops) ;
  Build_OP (	TOP_sub_r_i_r,	r0_21_0,	c32,	i1,	ops) ;
  Build_OP (	TOP_shru_r_r_r,	r0_21_1,	i0,	r0_21_0,	ops) ;
  Build_OP (	TOP_or_r_r_r,	o0,	r0_20_0,	r0_21_1,	ops) ;
} /* Expand__rotlw */

/*
@@@  case INTRN_ROTLW:
@@@    Expand__rotlw(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of roundclw based on validated and scheduled basic assembly source.
*/
static void
Expand__roundclw(
 TN* o0,
 TN* il0,
 TN* ih0,
 OPS* ops
)
{
  if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_adds_r_r_r)) { 
    TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
    TN *c31 = Gen_Literal_TN(__EXTS32TOS64(31), 4) ;
#undef __EXTS32TOS64
    Build_OP (	TOP_shru_i_r_r,	r0_21_0,	il0,	c31,	ops) ;
    Build_OP (	TOP_adds_r_r_r,	o0,	ih0,	r0_21_0,	ops) ;
  } else {
    TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_21_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
    TN *c0x7fffffff = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *c0x7fffffff_ltn = Gen_Literal_TN(__EXTS32TOS64(0x7fffffff), 4) ;
    Build_OP (    TOP_mov_ii_r,      c0x7fffffff,    c0x7fffffff_ltn,        ops) ;
    TN *c31 = Gen_Literal_TN(__EXTS32TOS64(31), 4) ;
#undef __EXTS32TOS64
    Build_OP (	TOP_cmpeq_r_r_b,	b0_0_0,	ih0,	c0x7fffffff,	ops) ;
    Build_OP (	TOP_shru_i_r_r,	r0_21_0,	il0,	c31,	ops) ;
    Build_OP (	TOP_add_r_r_r,	r0_21_1,	ih0,	r0_21_0,	ops) ;
    Build_OP (	TOP_targ_slct_r_r_b_r,	o0,	b0_0_0,	ih0,	r0_21_1,	ops) ;
  }
} /* Expand__roundclw */

/*
@@@  case INTRN_ROUNDCLW:
@@@    Expand__roundclw(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of roundcwh based on validated and scheduled basic assembly source.
*/
static void
Expand__roundcwh(
 TN* o0,
 TN* i0,
 OPS* ops
)
{
  if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_adds_r_r_r)) { 
    TN *r0_16_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
    TN *c0x8000 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *c0x8000_ltn = Gen_Literal_TN(__EXTS32TOS64(0x8000), 4) ;
    Build_OP (	TOP_mov_ii_r,	c0x8000,	c0x8000_ltn,	ops) ;
    TN *c16 = Gen_Literal_TN(__EXTS32TOS64(16), 4) ;
#undef __EXTS32TOS64
    Build_OP (	TOP_mov_r_r,	r0_20_0,	c0x8000,	ops) ;
    Build_OP (	TOP_adds_r_r_r,	r0_16_1,	i0,	r0_20_0,	ops) ;
    Build_OP (	TOP_shr_i_r_r,	o0,	r0_16_1,	c16,	ops) ;
  } else {
    TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *r0_16_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_20_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
    TN *c0x7fff0000 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *c0x7fff0000_ltn = Gen_Literal_TN(__EXTS32TOS64(0x7fff0000), 4) ;
    Build_OP (    TOP_mov_ii_r,      c0x7fff0000,    c0x7fff0000_ltn,        ops) ;
    TN *c0x8000 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *c0x8000_ltn = Gen_Literal_TN(__EXTS32TOS64(0x8000), 4) ;
    Build_OP (    TOP_mov_ii_r,      c0x8000,        c0x8000_ltn,    ops) ;
    TN *c16 = Gen_Literal_TN(__EXTS32TOS64(16), 4) ;
#undef __EXTS32TOS64
    Build_OP (	TOP_add_r_r_r,	r0_20_0,	i0,	c0x8000,	ops) ;
    Build_OP (	TOP_cmpge_r_r_b,	b0_0_0,	i0,	c0x7fff0000,	ops) ;
    Build_OP (	TOP_shr_i_r_r,	r0_20_1,	r0_20_0,	c16,	ops) ;
    Build_OP (	TOP_shr_i_r_r,	r0_16_1,	i0,	c16,	ops) ;
    Build_OP (	TOP_targ_slct_r_r_b_r,	o0,	b0_0_0,	r0_16_1,	r0_20_1,	ops) ;
  }
} /* Expand__roundcwh */

/*
@@@  case INTRN_ROUNDCWH:
@@@    Expand__roundcwh(result[0],opnd[0],ops) ;
@@@  break ;
*/

/*
 * Expansion of roundlw based on validated and scheduled basic assembly source.
*/
static void
Expand__roundlw(
 TN* o0,
 TN* il0,
 TN* ih0,
 OPS* ops
)
{
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c31 = Gen_Literal_TN(__EXTS32TOS64(31), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_shru_i_r_r,	r0_21_0,	il0,	c31,	ops) ;
  Build_OP (	TOP_add_r_r_r,	o0,	ih0,	r0_21_0,	ops) ;
} /* Expand__roundlw */

/*
@@@  case INTRN_ROUNDLW:
@@@    Expand__roundlw(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of roundwh based on validated and scheduled basic assembly source.
*/
static void
Expand__roundwh(
 TN* o0,
 TN* i0,
 OPS* ops
)
{
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c0x8000 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *c0x8000_ltn = Gen_Literal_TN(__EXTS32TOS64(0x8000), 4) ;
  Build_OP (    TOP_mov_ii_r,      c0x8000,        c0x8000_ltn,    ops) ;
  TN *c16 = Gen_Literal_TN(__EXTS32TOS64(16), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_add_r_r_r,	r0_21_0,	i0,	c0x8000,	ops) ;
  Build_OP (	TOP_shr_i_r_r,	o0,	r0_21_0,	c16,	ops) ;
} /* Expand__roundwh */

/*
@@@  case INTRN_ROUNDWH:
@@@    Expand__roundwh(result[0],opnd[0],ops) ;
@@@  break ;
*/

/*
 * Expansion of shlch based on validated and scheduled basic assembly source.
*/
static void
Expand__shlch(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_shls_ph_r_r_r)) { 
    TN *r0_16_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    Build_OP (    TOP_shls_ph_r_r_r,      r0_16_0,        i0,     i1,     ops) ;
    Build_OP (    TOP_sxth_r_r,   o0,     r0_16_0,        ops) ;
  } else if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_sats_r_r)) { 
    TN *r0_16_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    Build_OP (	TOP_shl_r_r_r,	r0_16_0,	i0,	i1,	ops) ;
    Build_OP (	TOP_sats_r_r,	o0,	r0_16_0,	ops) ;
  } else {
    TN *r0_16_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_16_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_17_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)                (((long long)(x)<<32) >> 32)
    TN *c0x00007fff = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *c0x00007fff_ltn = Gen_Literal_TN(__EXTS32TOS64(0x00007fff), 4) ;
    Build_OP (    TOP_mov_ii_r,      c0x00007fff,    c0x00007fff_ltn,        ops) ;
    TN *c0xffff8000 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *c0xffff8000_ltn = Gen_Literal_TN(__EXTS32TOS64(0xffff8000), 4) ;
    Build_OP (    TOP_mov_ii_r,      c0xffff8000,    c0xffff8000_ltn,        ops) ;
#undef __EXTS32TOS64
    Build_OP (    TOP_shl_r_r_r,      r0_16_1,        i0,     i1,        ops) ;
    Build_OP (    TOP_max_r_r_r,      r0_16_2,        r0_16_1,        c0xffff8000,    ops) ;
    Build_OP (    TOP_min_r_r_r,      o0,     r0_16_2,        c0x00007fff,    ops) ;
  }
} /* Expand__shlch */

/*
@@@  case INTRN_SHLCH:
@@@    Expand__shlch(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of shlcw based on validated and scheduled basic assembly source.
*/
static void
Expand__shlcw(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_shls_r_r_r)) { 
    Build_OP (    TOP_shls_r_r_r,      o0,        i0,     i1,     ops) ;
    return ;
  }

  if (TN_is_rematerializable (i1)) {
    WN *wn = TN_remat (i1);
    if (WN_operator_is (wn, OPR_INTCONST)) {
      INT64 co1 = WN_const_val (wn);
      if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_adds_r_r_r)
	  && co1 > 0
	  && co1 <= 3) {
	// Expand a small constant clamped left-shift as
	// a sequence of clamped adds.
	TN *src = i0;
	for (INT64 to_do = co1; to_do > 1; to_do--) {
	  TN *tmp = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer);
	  Build_OP ( TOP_adds_r_r_r, tmp, src, src, ops);
	  src = tmp;
	}
	Build_OP ( TOP_adds_r_r_r, o0, src, src, ops);
	return;
      }
    }
  }
  TN *b0_0_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_18_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_19_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_19_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_22_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c0x7fffffff = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *c0x7fffffff_ltn = Gen_Literal_TN(__EXTS32TOS64(0x7fffffff), 4) ;
  Build_OP (	TOP_mov_ii_r,	c0x7fffffff,	c0x7fffffff_ltn,	ops) ;
  TN *c31 = Gen_Literal_TN(__EXTS32TOS64(31), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_shr_i_r_r,	r0_18_0,	i0,	c31,	ops) ;
  Build_OP (	TOP_sub_r_i_r,	r0_19_0,	c31,	i1,	ops) ;
  Build_OP (	TOP_shl_r_r_r,	r0_20_0,	i0,	i1,	ops) ;
  Build_OP (	TOP_cmplt_r_r_r,	r0_21_0,	i0,	Zero_TN,	ops) ;
  Build_OP (	TOP_shr_r_r_r,	r0_19_1,	i0,	r0_19_0,	ops) ;
  Build_OP (	TOP_add_r_r_r,	r0_22_1,	r0_21_0,	c0x7fffffff,	ops) ;
  Build_OP (	TOP_cmpne_r_r_b,	b0_0_2,	r0_18_0,	r0_19_1,	ops) ;
  Build_OP (	TOP_targ_slct_r_r_b_r,	o0,	b0_0_2,	r0_22_1,	r0_20_0,	ops) ;
} /* Expand__shlcw */

/*
@@@  case INTRN_SHLCW:
@@@    Expand__shlcw(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of shll based on validated and scheduled basic assembly source.
*/
static void
Expand__shll(
 TN* ol0,
 TN* oh0,
 TN* il0,
 TN* ih0,
 TN* i1,
 OPS* ops
)
{
  TN *r0_19_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_22_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_23_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_24_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)                (((long long)(x)<<32) >> 32)
  TN *c32 = Gen_Literal_TN(__EXTS32TOS64(32), 4) ;
  TN *c_32 = Gen_Literal_TN(__EXTS32TOS64(-32), 4) ;
#undef __EXTS32TOS64
  Build_OP (    TOP_shl_r_r_r,      r0_19_0,        ih0,    i1,     ops) ;
  Build_OP (    TOP_sub_r_i_r,      r0_20_0,        c32,    i1,     ops) ;
  Build_OP (    TOP_add_i_r_r,      r0_21_0,        i1,     c_32,   ops) ;
  Build_OP (    TOP_shru_r_r_r,     r0_22_1,        il0,    r0_20_0,        ops) ;
  Build_OP (    TOP_shl_r_r_r,      r0_23_1,        il0,    r0_21_0,        ops) ;
  Build_OP (    TOP_shl_r_r_r,      ol0,    il0,    i1,     ops) ;
  Build_OP (    TOP_or_r_r_r,       r0_24_2,        r0_22_1,        r0_23_1,        ops) ;
  Build_OP (    TOP_or_r_r_r,       oh0,    r0_19_0,        r0_24_2,        ops) ;
} /* Expand__shll */


/*
@@@  case INTRN_SHLL:
@@@    Expand__shll(result[0],result[1],opnd[0],opnd[1],opnd[2],ops) ;
@@@  break ;
*/

/*
 * Expansion of shlw based on validated and scheduled basic assembly source.
*/
static void
Expand__shlw(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  Build_OP (	TOP_shl_r_r_r,	o0,	i0,	i1,	ops) ;
} /* Expand__shlw */

/*
@@@  case INTRN_SHLW:
@@@    Expand__shlw(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of shrl based on validated and scheduled basic assembly source.
*/
static void
Expand__shrl(
 TN* ol0,
 TN* oh0,
 TN* il0,
 TN* ih0,
 TN* i1,
 OPS* ops
)
{
  TN *r0_19_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_22_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_23_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_24_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_25_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_26_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)                (((long long)(x)<<32) >> 32)
  TN *c31 = Gen_Literal_TN(__EXTS32TOS64(31), 4) ;
  TN *c32 = Gen_Literal_TN(__EXTS32TOS64(32), 4) ;
  TN *c_32 = Gen_Literal_TN(__EXTS32TOS64(-32), 4) ;
#undef __EXTS32TOS64
  Build_OP (    TOP_shru_r_r_r,     r0_19_0,        il0,    i1,     ops) ;
  Build_OP (    TOP_sub_r_i_r,      r0_20_0,        c32,    i1,     ops) ;
  Build_OP (    TOP_add_i_r_r,      r0_21_0,        i1,     c_32,   ops) ;
  Build_OP (    TOP_shl_r_r_r,      r0_22_1,        ih0,    r0_20_0,        ops) ;
  Build_OP (    TOP_shr_r_r_r,      r0_23_1,        ih0,    r0_21_0,        ops) ;
  Build_OP (    TOP_shr_i_r_r,      r0_24_1,        r0_20_0,        c31,    ops) ;
  Build_OP (    TOP_shr_r_r_r,      oh0,    ih0,    i1,     ops) ;
  Build_OP (    TOP_or_r_r_r,       r0_25_2,        r0_19_0,        r0_22_1,        ops) ;
  Build_OP (    TOP_and_r_r_r,      r0_26_2,        r0_23_1,        r0_24_1,        ops) ;
  Build_OP (    TOP_or_r_r_r,       ol0,    r0_25_2,        r0_26_2,        ops) ;
} /* Expand__shrl */

/*
@@@  case INTRN_SHRL:
@@@    Expand__shrl(result[0],result[1],opnd[0],opnd[1],opnd[2],ops) ;
@@@  break ;
*/

/*
 * Expansion of shrrh based on validated and scheduled basic assembly source.
*/
static void
Expand__shrrh(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  TN *r0_20_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
  TN *c_1 = Gen_Literal_TN(__EXTS32TOS64(-1), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_add_i_r_r,	r0_21_0,	i1,	c_1,	ops) ;
  Build_OP (	TOP_shr_r_r_r,	r0_20_1,	i0,	i1,	ops) ;
  Build_OP (	TOP_shr_r_r_r,	r0_21_1,	i0,	r0_21_0,	ops) ;
  Build_OP (	TOP_and_i_r_r,	r0_21_2,	r0_21_1,	c1,	ops) ;
  Build_OP (	TOP_add_r_r_r,	o0,	r0_20_1,	r0_21_2,	ops) ;
} /* Expand__shrrh */

/*
@@@  case INTRN_SHRRH:
@@@    Expand__shrrh(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of shrrw based on validated and scheduled basic assembly source.
*/
static void
Expand__shrrw(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c1 = Gen_Literal_TN(__EXTS32TOS64(1), 4) ;
  TN *c_1 = Gen_Literal_TN(__EXTS32TOS64(-1), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_shr_r_r_r,	r0_20_0,	i0,	i1,	ops) ;
  Build_OP (	TOP_add_i_r_r,	r0_21_0,	i1,	c_1,	ops) ;
  Build_OP (	TOP_shr_r_r_r,	r0_21_1,	i0,	r0_21_0,	ops) ;
  Build_OP (	TOP_and_i_r_r,	r0_21_2,	r0_21_1,	c1,	ops) ;
  Build_OP (	TOP_add_r_r_r,	o0,	r0_20_0,	r0_21_2,	ops) ;
} /* Expand__shrrw */

/*
@@@  case INTRN_SHRRW:
@@@    Expand__shrrw(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of shrul based on validated and scheduled basic assembly source.
*/
static void
Expand__shrul(
 TN* ol0,
 TN* oh0,
 TN* il0,
 TN* ih0,
 TN* i1,
 OPS* ops
)
{
  TN *r0_19_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_22_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_23_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_24_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)                (((long long)(x)<<32) >> 32)
  TN *c32 = Gen_Literal_TN(__EXTS32TOS64(32), 4) ;
  TN *c_32 = Gen_Literal_TN(__EXTS32TOS64(-32), 4) ;
#undef __EXTS32TOS64
  Build_OP (    TOP_shru_r_r_r,     r0_19_0,        il0,    i1,     ops) ;
  Build_OP (    TOP_sub_r_i_r,      r0_20_0,        c32,    i1,     ops) ;
  Build_OP (    TOP_add_i_r_r,      r0_21_0,        i1,     c_32,   ops) ;
  Build_OP (    TOP_shl_r_r_r,      r0_22_1,        ih0,    r0_20_0,        ops) ;
  Build_OP (    TOP_shru_r_r_r,     r0_23_1,        ih0,    r0_21_0,        ops) ;
  Build_OP (    TOP_shru_r_r_r,     oh0,    ih0,    i1,     ops) ;
  Build_OP (    TOP_or_r_r_r,       r0_24_2,        r0_22_1,        r0_23_1,        ops) ;
  Build_OP (    TOP_or_r_r_r,       ol0,    r0_19_0,        r0_24_2,        ops) ;
} /* Expand__shrul */

/*
@@@  case INTRN_SHRUL:
@@@    Expand__shrul(result[0],result[1],opnd[0],opnd[1],opnd[2],ops) ;
@@@  break ;
*/

/*
 * Expansion of shruw based on validated and scheduled basic assembly source.
*/
static void
Expand__shruw(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  Build_OP (	TOP_shru_r_r_r,	o0,	i0,	i1,	ops) ;
} /* Expand__shruw */

/*
@@@  case INTRN_SHRUW:
@@@    Expand__shruw(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of shrw based on validated and scheduled basic assembly source.
*/
static void
Expand__shrw(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  Build_OP (	TOP_shr_r_r_r,	o0,	i0,	i1,	ops) ;
} /* Expand__shrw */

/*
@@@  case INTRN_SHRW:
@@@    Expand__shrw(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of subch based on validated and scheduled basic assembly source.
*/
static void
Expand__subch(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_subs_ph_r_r_r)) { 
    TN *r0_16_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    Build_OP (	TOP_subs_ph_r_r_r,	r0_16_0,	i0,	i1,	ops) ;
    Build_OP (	TOP_sxth_r_r,	o0,	r0_16_0,	ops) ;
  } else {
    TN *r0_16_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_16_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
    TN *c0x7fff = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *c0x7fff_ltn = Gen_Literal_TN(__EXTS32TOS64(0x7fff), 4) ;
    Build_OP (    TOP_mov_ii_r,      c0x7fff,        c0x7fff_ltn,    ops) ;
    TN *c0xffff8000 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *c0xffff8000_ltn = Gen_Literal_TN(__EXTS32TOS64(0xffff8000), 4) ;
    Build_OP (    TOP_mov_ii_r,      c0xffff8000,    c0xffff8000_ltn,        ops) ;
#undef __EXTS32TOS64
    Build_OP (	TOP_sub_r_r_r,	r0_16_0,	i0,	i1,	ops) ;
    Build_OP (	TOP_max_r_r_r,	r0_16_1,	r0_16_0,	c0xffff8000,	ops) ;
    Build_OP (	TOP_min_r_r_r,	o0,	r0_16_1,	c0x7fff,	ops) ;
  }
} /* Expand__subch */

/*
@@@  case INTRN_SUBCH:
@@@    Expand__subch(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of subcl based on validated and scheduled basic assembly source.
*/
static void
Expand__subcl(
 TN* ol0,
 TN* oh0,
 TN* il0,
 TN* ih0,
 TN* il1,
 TN* ih1,
 OPS* ops
)
{
  TN *b0_0_5 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_22_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_23_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_24_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_25_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_26_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_27_4 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_28_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_30_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c0x7fffffff = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *c0x7fffffff_ltn = Gen_Literal_TN(__EXTS32TOS64(0x7fffffff), 4) ;
  Build_OP (    TOP_mov_ii_r,      c0x7fffffff,    c0x7fffffff_ltn,        ops) ;
  TN *c_1 = Gen_Literal_TN(__EXTS32TOS64(-1), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_xor_r_r_r,	r0_20_0,	ih0,	ih1,	ops) ;
  Build_OP (	TOP_cmpltu_r_r_r,	r0_28_0,	il0,	il1,	ops) ;
  Build_OP (	TOP_add_r_r_r,	r0_30_1,	ih1,	r0_28_0,	ops) ;
  Build_OP (	TOP_cmplt_r_r_r,	r0_26_1,	ih0,	Zero_TN,	ops) ;
  Build_OP (	TOP_sub_r_r_r,	r0_22_2,	il0,	il1,	ops) ;
  Build_OP (	TOP_add_r_r_r,	r0_21_2,	c0x7fffffff,	r0_26_1,	ops) ;
  Build_OP (	TOP_sub_r_r_r,	r0_24_2,	ih0,	r0_30_1,	ops) ;
  Build_OP (	TOP_add_i_r_r,	r0_23_2,	r0_26_1,	c_1,	ops) ;
  Build_OP (	TOP_xor_r_r_r,	r0_25_3,	r0_24_2,	ih0,	ops) ;
  Build_OP (	TOP_and_r_r_r,	r0_27_4,	r0_20_0,	r0_25_3,	ops) ;
  Build_OP (	TOP_cmplt_r_r_b,	b0_0_5,	r0_27_4,	Zero_TN,	ops) ;
  Build_OP (	TOP_targ_slct_r_r_b_r,	ol0,	b0_0_5,	r0_23_2,	r0_22_2,	ops) ;
  Build_OP (	TOP_targ_slct_r_r_b_r,	oh0,	b0_0_5,	r0_21_2,	r0_24_2,	ops) ;
} /* Expand__subcl */

/*
@@@  case INTRN_SUBCL:
@@@    Expand__subcl(result[0],result[1],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
@@@  break ;
*/

/*
 * Expansion of subcw based on validated and scheduled basic assembly source.
*/
static void
Expand__subcw(
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
  if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_subs_r_r_r)) { 
    Build_OP (	TOP_subs_r_r_r,	o0,	i0,	i1,	ops) ;
  } else {
    TN *b0_0_3 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
    TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_21_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_21_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_22_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_24_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_24_2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
    TN *c0x7fffffff = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *c0x7fffffff_ltn = Gen_Literal_TN(__EXTS32TOS64(0x7fffffff), 4) ;
    Build_OP (    TOP_mov_ii_r,      c0x7fffffff,    c0x7fffffff_ltn,        ops) ;
#undef __EXTS32TOS64
    Build_OP (	TOP_xor_r_r_r,	r0_20_0,	i0,	i1,	ops) ;
    Build_OP (	TOP_sub_r_r_r,	r0_22_0,	i0,	i1,	ops) ;
    Build_OP (	TOP_cmplt_r_r_r,	r0_21_1,	i0,	Zero_TN,	ops) ;
    Build_OP (	TOP_xor_r_r_r,	r0_24_1,	r0_22_0,	i0,	ops) ;
    Build_OP (	TOP_add_r_r_r,	r0_21_2,	r0_21_1,	c0x7fffffff,	ops) ;
    Build_OP (	TOP_and_r_r_r,	r0_24_2,	r0_24_1,	r0_20_0,	ops) ;
    Build_OP (	TOP_cmplt_r_r_b,	b0_0_3,	r0_24_2,	Zero_TN,	ops) ;
    Build_OP (	TOP_targ_slct_r_r_b_r,	o0,	b0_0_3,	r0_21_2,	r0_22_0,	ops) ;
  }
} /* Expand__subcw */

/*
@@@  case INTRN_SUBCW:
@@@    Expand__subcw(result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

/*
 * Expansion of subl based on validated and scheduled basic assembly source.
*/
static void
Expand__subl(
 TN* ol0,
 TN* oh0,
 TN* il0,
 TN* ih0,
 TN* il1,
 TN* ih1,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_0_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_19_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  Build_OP (	TOP_cmpltu_r_r_b,	b0_0_0,	il0,	il1,	ops) ;
  Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_19_1,	b0_0_1,	ih1,	Zero_TN,	b0_0_0,	ops) ;
  Build_OP (	TOP_sub_r_r_r,	ol0,	il0,	il1,	ops) ;
  Build_OP (	TOP_sub_r_r_r,	oh0,	ih0,	r0_19_1,	ops) ;
} /* Expand__subl */

/*
@@@  case INTRN_SUBL:
@@@    Expand__subl(result[0],result[1],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
@@@  break ;
*/

/*
 * Expansion of subul based on validated and scheduled basic assembly source.
*/
static void
Expand__subul(
 TN* ol0,
 TN* oh0,
 TN* il0,
 TN* ih0,
 TN* il1,
 TN* ih1,
 OPS* ops
)
{
  TN *b0_0_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *b0_0_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch) ;
  TN *r0_19_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  Build_OP (	TOP_cmpltu_r_r_b,	b0_0_0,	il0,	il1,	ops) ;
  Build_OP (	TOP_targ_addcg_b_r_r_b_r,	r0_19_1,	b0_0_1,	ih1,	Zero_TN,	b0_0_0,	ops) ;
  Build_OP (	TOP_sub_r_r_r,	ol0,	il0,	il1,	ops) ;
  Build_OP (	TOP_sub_r_r_r,	oh0,	ih0,	r0_19_1,	ops) ;
} /* Expand__subul */

/*
@@@  case INTRN_SUBUL:
@@@    Expand__subul(result[0],result[1],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
@@@  break ;
*/

/*
 * Expansion of swapbh based on validated and scheduled basic assembly source.
*/
static void
Expand__swapbh(
 TN* o0,
 TN* i0,
 OPS* ops
)
{
  if (ISA_SUBSET_LIST_Member (ISA_SUBSET_List, TOP_perm_pb_i_r_r)) { 
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
    TN *c225 = Gen_Literal_TN(__EXTS32TOS64(225), 4) ;
#undef __EXTS32TOS64
    Build_OP (	TOP_perm_pb_i_r_r,	o0,	i0,	c225,	ops) ;
  }
  else {
    TN *r0_16_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
    TN *c16 = Gen_Literal_TN(__EXTS32TOS64(16), 4) ;
#undef __EXTS32TOS64
    Build_OP (	TOP_bswap_r_r,	r0_16_0,	i0,	ops) ;
    Build_OP (	TOP_shru_i_r_r,	o0,	r0_16_0,	c16,	ops) ;
  }
} /* Expand__swapbh */

/*
@@@  case INTRN_SWAPBH:
@@@    Expand__swapbh(result[0],opnd[0],ops) ;
@@@  break ;
*/

/*
 * Expansion of swapbw based on validated and scheduled basic assembly source.
*/
static void
Expand__swapbw(
 TN* o0,
 TN* i0,
 OPS* ops
)
{
  Build_OP (	TOP_bswap_r_r,	o0,	i0,	ops) ;
} /* Expand__swapbw */

/*
@@@  case INTRN_SWAPBW:
@@@    Expand__swapbw(result[0],opnd[0],ops) ;
@@@  break ;
*/

/*
 * Expansion of swaphw based on validated and scheduled basic assembly source.
*/
static void
Expand__swaphw(
 TN* o0,
 TN* i0,
 OPS* ops
)
{
  if (ISA_SUBSET_LIST_Member (ISA_SUBSET_List, TOP_rotl_i_r_r)) { 
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
    TN *c16 = Gen_Literal_TN(__EXTS32TOS64(16), 4) ;
#undef __EXTS32TOS64
    Build_OP (	TOP_rotl_i_r_r,	o0,	i0,	c16,	ops) ;
  }
  else {
    TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
    TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
    TN *c16 = Gen_Literal_TN(__EXTS32TOS64(16), 4) ;
#undef __EXTS32TOS64
    Build_OP (	TOP_shru_i_r_r,	r0_20_0,	i0,	c16,	ops) ;
    Build_OP (	TOP_shl_i_r_r,	r0_21_0,	i0,	c16,	ops) ;
    Build_OP (	TOP_or_r_r_r,	o0,	r0_20_0,	r0_21_0,	ops) ;
  }
} /* Expand__swaphw */

/*
@@@  case INTRN_SWAPHW:
@@@    Expand__swaphw(result[0],opnd[0],ops) ;
@@@  break ;
*/

/*
 * Expansion of xshlh based on validated and scheduled basic assembly source.
*/
static void
Expand__xshlh(
 TN* o0,
 TN* i0,
 TN* i1,
 TN* i2,
 OPS* ops
)
{
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c16 = Gen_Literal_TN(__EXTS32TOS64(16), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_shl_r_r_r,	r0_20_0,	i0,	i2,	ops) ;
  Build_OP (	TOP_sub_r_i_r,	r0_21_0,	c16,	i2,	ops) ;
  Build_OP (	TOP_shru_r_r_r,	r0_21_1,	i1,	r0_21_0,	ops) ;
  Build_OP (	TOP_or_r_r_r,	o0,	r0_20_0,	r0_21_1,	ops) ;
} /* Expand__xshlh */

/*
@@@  case INTRN_XSHLH:
@@@    Expand__xshlh(result[0],opnd[0],opnd[1],opnd[2],ops) ;
@@@  break ;
*/

/*
 * Expansion of xshlw based on validated and scheduled basic assembly source.
*/
static void
Expand__xshlw(
 TN* o0,
 TN* i0,
 TN* i1,
 TN* i2,
 OPS* ops
)
{
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c32 = Gen_Literal_TN(__EXTS32TOS64(32), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_shl_r_r_r,	r0_20_0,	i0,	i2,	ops) ;
  Build_OP (	TOP_sub_r_i_r,	r0_21_0,	c32,	i2,	ops) ;
  Build_OP (	TOP_shru_r_r_r,	r0_21_1,	i1,	r0_21_0,	ops) ;
  Build_OP (	TOP_or_r_r_r,	o0,	r0_20_0,	r0_21_1,	ops) ;
} /* Expand__xshlw */

/*
@@@  case INTRN_XSHLW:
@@@    Expand__xshlw(result[0],opnd[0],opnd[1],opnd[2],ops) ;
@@@  break ;
*/

/*
 * Expansion of xshrh based on validated and scheduled basic assembly source.
*/
static void
Expand__xshrh(
 TN* o0,
 TN* i0,
 TN* i1,
 TN* i2,
 OPS* ops
)
{
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c16 = Gen_Literal_TN(__EXTS32TOS64(16), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_shru_r_r_r,	r0_20_0,	i1,	i2,	ops) ;
  Build_OP (	TOP_sub_r_i_r,	r0_21_0,	c16,	i2,	ops) ;
  Build_OP (	TOP_shl_r_r_r,	r0_21_1,	i0,	r0_21_0,	ops) ;
  Build_OP (	TOP_or_r_r_r,	o0,	r0_20_0,	r0_21_1,	ops) ;
} /* Expand__xshrh */

/*
@@@  case INTRN_XSHRH:
@@@    Expand__xshrh(result[0],opnd[0],opnd[1],opnd[2],ops) ;
@@@  break ;
*/

/*
 * Expansion of xshrw based on validated and scheduled basic assembly source.
*/
static void
Expand__xshrw(
 TN* o0,
 TN* i0,
 TN* i1,
 TN* i2,
 OPS* ops
)
{
  TN *r0_20_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_0 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *r0_21_1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
#define __EXTS32TOS64(x)		(((long long)(x)<<32) >> 32)
  TN *c32 = Gen_Literal_TN(__EXTS32TOS64(32), 4) ;
#undef __EXTS32TOS64
  Build_OP (	TOP_shru_r_r_r,	r0_20_0,	i1,	i2,	ops) ;
  Build_OP (	TOP_sub_r_i_r,	r0_21_0,	c32,	i2,	ops) ;
  Build_OP (	TOP_shl_r_r_r,	r0_21_1,	i0,	r0_21_0,	ops) ;
  Build_OP (	TOP_or_r_r_r,	o0,	r0_20_0,	r0_21_1,	ops) ;
} /* Expand__xshrw */

/*
@@@  case INTRN_XSHRW:
@@@    Expand__xshrw(result[0],opnd[0],opnd[1],opnd[2],ops) ;
@@@  break ;
*/

static void
Expand__asm_n(
 int asm_n,
 TN* o0,
 TN* i0,
 TN* i1,
 OPS* ops
)
{
    typedef struct { TOP r ; TOP i ; } TOPpairs ;
    TOPpairs toppairs [] = { { TOP_asm_0_r_r_r,  TOP_asm_16_i_r_r },
			     { TOP_asm_1_r_r_r,  TOP_asm_17_i_r_r },
			     { TOP_asm_2_r_r_r,  TOP_asm_18_i_r_r },
			     { TOP_asm_3_r_r_r,  TOP_asm_19_i_r_r },
			     { TOP_asm_4_r_r_r,  TOP_asm_20_i_r_r },
			     { TOP_asm_5_r_r_r,  TOP_asm_21_i_r_r },
			     { TOP_asm_6_r_r_r,  TOP_asm_22_i_r_r },
			     { TOP_asm_7_r_r_r,  TOP_asm_23_i_r_r },
			     { TOP_asm_8_r_r_r,  TOP_asm_24_i_r_r },
			     { TOP_asm_9_r_r_r,  TOP_asm_25_i_r_r },
			     { TOP_asm_10_r_r_r, TOP_asm_26_i_r_r },
			     { TOP_asm_11_r_r_r, TOP_asm_27_i_r_r },
			     { TOP_asm_12_r_r_r, TOP_asm_28_i_r_r },
			     { TOP_asm_13_r_r_r, TOP_asm_29_i_r_r },
			     { TOP_asm_14_r_r_r, TOP_asm_30_i_r_r },
			     { TOP_asm_15_r_r_r, TOP_asm_31_i_r_r }
    } ;
    /* Half of asm_n are by convention for register, register, the upper being used for register, immediate */
    TOP top = (asm_n >= 0) && (asm_n < sizeof(toppairs)/sizeof(toppairs[0])) 
	? toppairs[asm_n].r
	: TOP_UNDEFINED ;    
    /* 
       Some cores do not support all the asms : ST231 does not support asm_15/asm_31
       Since we detect the situation quite late, we choose to emit a TOP_sbreak
       This is similar to the handling of other bizarre cases (: see prginspg, )
     */
    if (!ISA_SUBSET_LIST_Member(ISA_SUBSET_List, top)) { 
        Expand_Unimplemented_Intrinsic (top, ops);
    } else {
	/* Can we produce the immediates ? */
	if (TN_is_rematerializable(i1)) {
	    WN *wn = TN_remat(i1) ;
	    if (WN_operator_is(wn, OPR_INTCONST)) {
		TN *cunknown = Gen_Literal_TN(WN_const_val(wn), 4) ;
		TOP top_ = TOP_opnd_immediate_variant(top, 1, WN_const_val(wn) ) ;	   
		if (top_ == TOP_UNDEFINED) {
		    TN *r = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
		    Build_OP (	TOP_mov_i_r,	r,	cunknown,	ops) ;
		    Build_OP (	top ,  		o0, 	i0,  		r, 	ops) ;
		} else {
		    Build_OP (	top_ ,  	o0, 	i0, 		cunknown, 	ops) ;
		}
	    }
	} else if (TN_is_zero(i1)) { 
	    Build_OP ( top ,  o0, i0, Zero_TN, ops) ;
	} else if (TN_has_value(i1)) {
	    TN *cunknown = Gen_Literal_TN(TN_value(i1), 4) ;
	    TOP top_ = TOP_opnd_immediate_variant(top, 1, TN_value(i1) ) ;
	    if (top_ == TOP_UNDEFINED) {
		TN *r = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
		Build_OP (	TOP_mov_i_r,	r,	cunknown,	ops) ;
		Build_OP (	top ,  		o0, 	i0,  		r, 	ops) ;
	    } else {
		Build_OP (	top_ ,  	o0, 	i0, 		cunknown, 	ops) ;
	    }
	} else {
	    Build_OP ( top,	o0,	i0,	i1,	ops) ;	
	}
    }
} /* Expand__asm_n */

/*
@@@  case INTRN_ASM_N:
@@@    Expand__asm_n(n,result[0],opnd[0],opnd[1],ops) ;
@@@  break ;
*/

static void
Expand__trap(
 OPS* ops
)
{
  TOP top = TOP_UNDEFINED;
  if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_sbrk_i)) {
    top = TOP_sbrk_i;
  } else if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_st240_sbrk_i)) {
    top = TOP_st240_sbrk_i;
  }
  /* The representation of such a trap is 1 */
  TN *c1 = Gen_Literal_TN(1LL, 4) ;
  Build_OP (	top,	c1, 	ops) ;
} /* Expand__trap */

/*
@@@  case INTRN_TRAP:
@@@    Expand__trap(ops) ;
@@@  break ;
*/

static void
Expand__builtin_dwarf_cfa(
 TN* result,
 OPS* ops
)
{
  Exp_Lda (Pointer_Mtype, result, Get_UpFormal_Base_Symbol (), 0,
	   OPERATOR_UNKNOWN, ops);
} /* Expand__builtin_dwarf_cfa */

static void
Expand__isunordered(
		    TN *o0,
		    TN *i0,
		    TN *i1,
		    TN *i2,
		    TN *i3,
		    OPS* ops
		    )
{
  TN *t1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *t2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  Expand__isnand (t1, i0, i1, ops);
  Expand__isnand (t2, i2, i3, ops);
  Build_OP ( TOP_or_r_r_r, o0, t1, t2, ops);
}

static void
Expand__isunorderedf(
		     TN *o0,
		     TN *i0,
		     TN *i1,
		     OPS* ops
		     )
{
  TN *t1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  TN *t2 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  Expand__isnanf (t1, i0, ops);
  Expand__isnanf (t2, i1, ops);
  Build_OP ( TOP_or_r_r_r, o0, t1, t2, ops);
}

static void
Expand__isordered(
		  TN *o0,
		  TN *i0,
		  TN *i1,
		  TN *i2,
		  TN *i3,
		  OPS* ops
		  )
{
  TN *t1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  Expand__isunordered (t1, i0, i1, i2, i3, ops);
  Build_OP ( TOP_cmpeq_r_r_r, o0, t1, Zero_TN, ops);
}

static void
Expand__isorderedf(
		   TN *o0,
		   TN *i0,
		   TN *i1,
		   OPS* ops
		   )
{
  TN *t1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer) ;
  Expand__isunorderedf (t1, i0, i1, ops);
  Build_OP ( TOP_cmpeq_r_r_r, o0, t1, Zero_TN, ops);
}

#define INTRN_EXPAND
#include "gen_intrinsics.inc"
#undef INTRN_EXPAND

void
Exp_Intrinsic_Op (
  INTRINSIC id,
  INT num_results,
  INT num_opnds,
  TN *result[],
  TN *opnd[],
  OPS *ops,
  SRCPOS srcpos,
  BB* bb
)
{
  switch (id) {
    case INTRN_BUILTIN__DIVUH:
      Expand__builtin__divuh(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_BUILTIN__DIVH:
      Expand__builtin__divh(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_BUILTIN__MODUH:
      Expand__builtin__moduh(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_BUILTIN__MODH:
      Expand__builtin__modh(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_BUILTIN__DIVUW:
      Expand__builtin__divuw(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_BUILTIN__DIVW:
      Expand__builtin__divw(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_BUILTIN__MODUW:
      Expand__builtin__moduw(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_BUILTIN__MODW:
      Expand__builtin__modw(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_ST200MUL32:
       Expand__st200mul32(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_ST200MUL64H:
       Expand__st200mul64h(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_ST200MUL64HU:
       Expand__st200mul64hu(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_ST200MULFRAC:
       Expand__st200mulfrac(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_ST200PRGINSPG:
	Expand__st200prginspg(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_ST220ADDCG:
    case INTRN_ST200ADDCG:
      Expand__st220addcg(result[0],result[1],opnd[0],opnd[1],opnd[2],ops) ;
    break ;
    case INTRN_ST220DIVS:
    case INTRN_ST200DIVS:
      Expand__st220divs(result[0],result[1],opnd[0],opnd[1],opnd[2],ops) ;
    break ;
    case INTRN_ST220MULHHS:
    case INTRN_ST200MULHHS:
      Expand__st220mulhhs(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_ST220MULHS:
    case INTRN_ST200MULHS:
      Expand__st220mulhs(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_ST220MULLHUS:
    case INTRN_ST200MULLHUS:
      Expand__st220mullhus(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_ST220MULLU:
    case INTRN_ST200MULLU:
      Expand__st220mullu(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_ST220PFT:
    case INTRN_ST200PFT:
      Expand__st220pft(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_ST220PRGADD:
    case INTRN_ST200PRGADD:
      Expand__st220prgadd(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_ST220PRGSET:
    case INTRN_ST200PRGSET:
      Expand__st220prgset(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_ST220SYSCALL:
    case INTRN_ST200SYSCALL:
      Expand__st220syscall(opnd[0],ops) ;
    break ;
    case INTRN_ABSCH:
      Expand__absch(result[0],opnd[0],ops) ;
    break ;
    case INTRN_ABSCL:
      Expand__abscl(result[0],result[1],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_ABSCW:
      Expand__abscw(result[0],opnd[0],ops) ;
    break ;
    case INTRN_ABSH:
      Expand__absh(result[0],opnd[0],ops) ;
    break ;
    case INTRN_ABSL:
      Expand__absl(result[0],result[1],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_ABSW:
      Expand__absw(result[0],opnd[0],ops) ;
    break ;
    case INTRN_ADDCH:
      Expand__addch(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_ADDCL:
      Expand__addcl(result[0],result[1],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
    break ;
    case INTRN_ADDCW:
      Expand__addcw(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_ADDL:
      Expand__addl(result[0],result[1],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
    break ;
    case INTRN_ADDUL:
      Expand__addul(result[0],result[1],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
    break ;
    case INTRN_BITCLRH:
      Expand__bitclrh(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_BITCLRW:
      Expand__bitclrw(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_BITCNTH:
      Expand__bitcnth(result[0],opnd[0],ops) ;
    break ;
    case INTRN_BITCNTW:
      Expand__bitcntw(result[0],opnd[0],ops) ;
    break ;
    case INTRN_BITNOTH:
      Expand__bitnoth(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_BITNOTW:
      Expand__bitnotw(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_BITREVW:
      Expand__bitrevw(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_BITSETH:
      Expand__bitseth(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_BITSETW:
      Expand__bitsetw(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_BITVALH:
      Expand__bitvalh(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_BITVALW:
      Expand__bitvalw(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_CLAMPLW:
      Expand__clamplw(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_CLAMPWH:
      Expand__clampwh(result[0],opnd[0],ops) ;
    break ;
    case INTRN_DISTH:
      Expand__disth(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_DISTUH:
      Expand__distuh(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_DISTUW:
      Expand__distuw(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_DISTW:
      Expand__distw(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_EDGESH:
      Expand__edgesh(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_EDGESW:
      Expand__edgesw(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_EQL:
      Expand__eql(result[0],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
    break ;
    case INTRN_EQUL:
      Expand__equl(result[0],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
    break ;
    case INTRN_GEL:
      Expand__gel(result[0],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
    break ;
    case INTRN_GETHH:
      Expand__gethh(result[0],opnd[0],ops) ;
    break ;
    case INTRN_GETHW:
      Expand__gethw(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_GETLH:
      Expand__getlh(result[0],opnd[0],ops) ;
    break ;
    case INTRN_GETLW:
      Expand__getlw(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_GEUL:
      Expand__geul(result[0],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
    break ;
    case INTRN_GTL:
      Expand__gtl(result[0],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
    break ;
    case INTRN_GTUL:
      Expand__gtul(result[0],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
    break ;
    case INTRN_INSEQUW:
      Expand__insequw(result[0],opnd[0],opnd[1],opnd[2],ops) ;
    break ;
    case INTRN_INSEQW:
      Expand__inseqw(result[0],opnd[0],opnd[1],opnd[2],ops) ;
    break ;
    case INTRN_INSGEUW:
      Expand__insgeuw(result[0],opnd[0],opnd[1],opnd[2],ops) ;
    break ;
    case INTRN_INSGEW:
      Expand__insgew(result[0],opnd[0],opnd[1],opnd[2],ops) ;
    break ;
    case INTRN_INSGTUW:
      Expand__insgtuw(result[0],opnd[0],opnd[1],opnd[2],ops) ;
    break ;
    case INTRN_INSGTW:
      Expand__insgtw(result[0],opnd[0],opnd[1],opnd[2],ops) ;
    break ;
    case INTRN_INSLEUW:
      Expand__insleuw(result[0],opnd[0],opnd[1],opnd[2],ops) ;
    break ;
    case INTRN_INSLEW:
      Expand__inslew(result[0],opnd[0],opnd[1],opnd[2],ops) ;
    break ;
    case INTRN_INSLTUW:
      Expand__insltuw(result[0],opnd[0],opnd[1],opnd[2],ops) ;
    break ;
    case INTRN_INSLTW:
      Expand__insltw(result[0],opnd[0],opnd[1],opnd[2],ops) ;
    break ;
    case INTRN_INSNEUW:
      Expand__insneuw(result[0],opnd[0],opnd[1],opnd[2],ops) ;
    break ;
    case INTRN_INSNEW:
      Expand__insnew(result[0],opnd[0],opnd[1],opnd[2],ops) ;
    break ;
    case INTRN_LEL:
      Expand__lel(result[0],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
    break ;
    case INTRN_LEUL:
      Expand__leul(result[0],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
    break ;
    case INTRN_LTL:
      Expand__ltl(result[0],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
    break ;
    case INTRN_LTUL:
      Expand__ltul(result[0],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
    break ;
    case INTRN_LZCNTH:
      Expand__lzcnth(result[0],opnd[0],ops) ;
    break ;
    case INTRN_LZCNTL:
    case INTRN_CLZ:
      Expand__lzcntl(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_LZCNTW:
    case INTRN_CLZ32:
      Expand__lzcntw(result[0],opnd[0],ops) ;
    break ;
    case INTRN_MAFCW:
      Expand__mafcw(result[0],opnd[0],opnd[1],opnd[2],ops) ;
    break ;
    case INTRN_MAXH:
      Expand__maxh(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_MAXL:
      Expand__maxl(result[0],result[1],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
    break ;
    case INTRN_MAXUH:
      Expand__maxuh(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_MAXUL:
      Expand__maxul(result[0],result[1],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
    break ;
    case INTRN_MINH:
      Expand__minh(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_MINL:
      Expand__minl(result[0],result[1],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
    break ;
    case INTRN_MINUH:
      Expand__minuh(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_MINUL:
      Expand__minul(result[0],result[1],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
    break ;
    case INTRN_MPFCW:
      Expand__mpfcw(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_MPFCWL:
      Expand__mpfcwl(result[0],result[1],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_MPFML:
      Expand__mpfml(result[0],result[1],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_MPFRCH:
      Expand__mpfrch(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_MPML:
      Expand__mpml(result[0],result[1],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_MPUML:
      Expand__mpuml(result[0],result[1],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_MULFCH:
      Expand__mulfch(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_MULFCM:
      Expand__mulfcm(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_MULFCW:
      Expand__mulfcw(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_MULH:
      Expand__mulh(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_MULHH:
      Expand__mulhh(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_MULHUH:
      Expand__mulhuh(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_MULHUW:
      Expand__mulhuw(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_MULHW:
      Expand__mulhw(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_MULL:
      Expand__mull(result[0],result[1],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
    break ;
    case INTRN_MULM:
      Expand__mulm(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_MULN:
      Expand__muln(result[0],result[1],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_MULUH:
      Expand__muluh(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_MULUL:
      Expand__mulul(result[0],result[1],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
    break ;
    case INTRN_MULUM:
      Expand__mulum(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_MULUN:
      Expand__mulun(result[0],result[1],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_MULUW:
      Expand__muluw(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_MULW:
      Expand__mulw(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_NEARCLW:
      Expand__nearclw(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_NEARCWH:
      Expand__nearcwh(result[0],opnd[0],ops) ;
    break ;
    case INTRN_NEARLW:
      Expand__nearlw(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_NEARWH:
      Expand__nearwh(result[0],opnd[0],ops) ;
    break ;
    case INTRN_NEGCH:
      Expand__negch(result[0],opnd[0],ops) ;
    break ;
    case INTRN_NEGCL:
      Expand__negcl(result[0],result[1],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_NEGCW:
      Expand__negcw(result[0],opnd[0],ops) ;
    break ;
    case INTRN_NEGL:
      Expand__negl(result[0],result[1],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_NEGUL:
      Expand__negul(result[0],result[1],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_NEL:
      Expand__nel(result[0],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
    break ;
    case INTRN_NEUL:
      Expand__neul(result[0],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
    break ;
    case INTRN_NORMH:
      Expand__normh(result[0],opnd[0],ops) ;
    break ;
    case INTRN_NORML:
      Expand__norml(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_NORMW:
      Expand__normw(result[0],opnd[0],ops) ;
    break ;
    case INTRN_PRIORH:
      Expand__priorh(result[0],opnd[0],ops) ;
    break ;
    case INTRN_PRIORL:
      Expand__priorl(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_PRIORW:
      Expand__priorw(result[0],opnd[0],ops) ;
    break ;
    case INTRN_PUTHL:
      Expand__puthl(result[0],result[1],opnd[0],ops) ;
    break ;
    case INTRN_PUTHW:
      Expand__puthw(result[0],opnd[0],ops) ;
    break ;
    case INTRN_PUTLL:
      Expand__putll(result[0],result[1],opnd[0],ops) ;
    break ;
    case INTRN_PUTLW:
      Expand__putlw(result[0],opnd[0],ops) ;
    break ;
    case INTRN_ROTLH:
      Expand__rotlh(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_ROTLW:
      Expand__rotlw(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_ROUNDCLW:
      Expand__roundclw(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_ROUNDCWH:
      Expand__roundcwh(result[0],opnd[0],ops) ;
    break ;
    case INTRN_ROUNDLW:
      Expand__roundlw(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_ROUNDWH:
      Expand__roundwh(result[0],opnd[0],ops) ;
    break ;
    case INTRN_SHLCH:
      Expand__shlch(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_SHLCW:
      Expand__shlcw(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_SHLL:
      Expand__shll(result[0],result[1],opnd[0],opnd[1],opnd[2],ops) ;
    break ;
    case INTRN_SHLW:
      Expand__shlw(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_SHRL:
      Expand__shrl(result[0],result[1],opnd[0],opnd[1],opnd[2],ops) ;
    break ;
    case INTRN_SHRRH:
      Expand__shrrh(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_SHRRW:
      Expand__shrrw(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_SHRUL:
      Expand__shrul(result[0],result[1],opnd[0],opnd[1],opnd[2],ops) ;
    break ;
    case INTRN_SHRUW:
      Expand__shruw(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_SHRW:
      Expand__shrw(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_SUBCH:
      Expand__subch(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_SUBCL:
      Expand__subcl(result[0],result[1],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
    break ;
    case INTRN_SUBCW:
      Expand__subcw(result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_SUBL:
      Expand__subl(result[0],result[1],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
    break ;
    case INTRN_SUBUL:
      Expand__subul(result[0],result[1],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
    break ;
    case INTRN_SWAPBH:
      Expand__swapbh(result[0],opnd[0],ops) ;
    break ;
    case INTRN_SWAPBW:
      Expand__swapbw(result[0],opnd[0],ops) ;
    break ;
    case INTRN_SWAPHW:
      Expand__swaphw(result[0],opnd[0],ops) ;
    break ;
    case INTRN_XSHLH:
      Expand__xshlh(result[0],opnd[0],opnd[1],opnd[2],ops) ;
    break ;
    case INTRN_XSHLW:
      Expand__xshlw(result[0],opnd[0],opnd[1],opnd[2],ops) ;
    break ;
    case INTRN_XSHRH:
      Expand__xshrh(result[0],opnd[0],opnd[1],opnd[2],ops) ;
    break ;
    case INTRN_XSHRW:
      Expand__xshrw(result[0],opnd[0],opnd[1],opnd[2],ops) ;
    break ;
    case INTRN_ASM_0:
      Expand__asm_n(0, result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_ASM_1:
      Expand__asm_n(1, result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_ASM_2:
      Expand__asm_n(2, result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_ASM_3:
      Expand__asm_n(3, result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_ASM_4:
      Expand__asm_n(4, result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_ASM_5:
      Expand__asm_n(5, result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_ASM_6:
      Expand__asm_n(6, result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_ASM_7:
      Expand__asm_n(7, result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_ASM_8:
      Expand__asm_n(8, result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_ASM_9:
      Expand__asm_n(9, result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_ASM_10:
      Expand__asm_n(10, result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_ASM_11:
      Expand__asm_n(11, result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_ASM_12:
      Expand__asm_n(12, result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_ASM_13:
      Expand__asm_n(13, result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_ASM_14:
      Expand__asm_n(14, result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_ASM_15:
      Expand__asm_n(15, result[0],opnd[0],opnd[1],ops) ;
    break ;
    case INTRN_TRAP:	
      Expand__trap(ops) ;
    break ;
  case INTRN_BUILTIN_DWARF_CFA:
    Expand__builtin_dwarf_cfa (result[0], ops) ;
    break;
  case INTRN_CTZ32:
    Expand__ctzw(result[0],opnd[0],ops) ;
    break;
  case INTRN_CTZ64:
    Expand__ctzl(result[0],opnd[0],opnd[1],ops) ;
    break;
  case INTRN_FFS32:
    Expand__ffsw(result[0],opnd[0],ops) ;
    break;
  case INTRN_FFS64:
    Expand__ffsl(result[0],opnd[0],opnd[1],ops) ;
    break;
  case INTRN_ISINFF:
    Expand__isinff(result[0],opnd[0],ops) ;
    break;
  case INTRN_ISINFD:
    Expand__isinfd(result[0],opnd[0],opnd[1],ops) ;
    break;
  case INTRN_ISNANF:
    Expand__isnanf(result[0],opnd[0],ops) ;
    break;
  case INTRN_ISNAND:
    Expand__isnand(result[0],opnd[0],opnd[1],ops) ;
    break;
  case INTRN_PARITY32:
    Expand__parityw(result[0],opnd[0],ops) ;
    break;
  case INTRN_PARITY64:
    Expand__parityl(result[0],opnd[0],opnd[1],ops) ;
    break;
  case INTRN_POPCOUNT32:
    Expand__bitcntw(result[0],opnd[0],ops) ;
    break;
  case INTRN_POPCOUNT64:
    Expand__bitcntl(result[0],opnd[0],opnd[1],ops) ;
    break;
  case INTRN_ISUNORDERED:
    Expand__isunordered(result[0],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
    break;
  case INTRN_ISUNORDEREDF:
    Expand__isunorderedf(result[0],opnd[0],opnd[1],ops) ;
    break;
  case INTRN_ISORDERED:
    Expand__isordered(result[0],opnd[0],opnd[1],opnd[2],opnd[3],ops) ;
    break;
  case INTRN_ISORDEREDF:
    Expand__isorderedf(result[0],opnd[0],opnd[1],ops) ;
    break;
#define INTRN_SWITCH
#include "gen_intrinsics.inc"
#undef INTRN_SWITCH
    default:
      FmtAssert (FALSE, ("Exp_Intrinsic_Op: unknown intrinsic op %s (id: %d)", INTRN_c_name(id), id));
  } /* switch*/
}