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

#ifndef EXP_EXTENSION_H_INCLUDED
#define EXP_EXTENSION_H_INCLUDED

#include "defs.h"
#include "config.h"

#include "intrn_info.h"
#include "cgir.h"

#include "dyn_isa_api.h" /* for reconfigurability */
#include "lai_loader_api.h"

// ===================================================================
// ==
// ==  Code selection API for extension register class
// ==
// ===================================================================

/* Generate simulated compose/extract */
void Build_OP_simulated_extract(TN *tgt0, TN *tgt1, TN *guard, TN *src, OPS *ops);
void Build_OP_simulated_extract(TN *tgt0, TN *tgt1, TN *tgt2, TN *tgt3, TN *guard, TN *src, OPS *ops);
void Build_OP_simulated_compose(TN *tgt, TN *guard, TN *src0, TN *src1, OPS *ops);
void Build_OP_simulated_compose(TN *tgt, TN *guard, TN *src0, TN *src1, TN *src2, TN *src3, OPS *ops);

/* Expand Copy */
BOOL EXTENSION_Expand_Copy(TN *tgt_tn, TN *guard, TN *src_tn, OPS *ops);

/* Expand immediate 32bits to extension registers */
BOOL EXTENSION_Expand_Immediate(TN *result, TN *src, BOOL is_signed, OPS *ops);

/* Expand load with immediate. Return FALSE if fail. */
BOOL EXTENSION_Expand_Load_Imm(AM_Base_Reg_Type base_type, TN *result, TN *base, TN *ofst, OPS *ops, UINT32 data_align);

/* Expand load with register. Return FALSE if fail. */
BOOL EXTENSION_Expand_Load_Reg(AM_Base_Reg_Type base_type, TN *result, TN *base, TN *ofst, OPS *ops, UINT32 data_align);

/* Expand store with immediate. Return FALSE if fail. */
BOOL EXTENSION_Expand_Store_Imm(AM_Base_Reg_Type base_type, TN *src, TN *base, TN *ofst, OPS *ops, UINT32 data_align);

/* Expand store with register. Return FALSE if fail. */
BOOL EXTENSION_Expand_Store_Reg(AM_Base_Reg_Type base_type, TN *src, TN *base, TN *ofst, OPS *ops, UINT32 data_align);


#endif

