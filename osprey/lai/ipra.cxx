/*
  Copyright (C) 2005, STMicroelectronics, All Rights Reserved.

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

/* ====================================================================
 * ====================================================================
 *
 * Module: ipra.cxx
 *
 * Description:
 *
 * Main data structures to enable inter-procedural register allocation
 * 
 * 
 *
 * ====================================================================
 * ====================================================================
 */

#include "defs.h"
#include "cg.h"
#include "cg_flags.h"
#include "tracing.h"
#include "ipra.h"
#include "bb.h"
#include "calls.h"

IPRA::IPRA ()
{
  st_to_ipra_info_map = CXX_NEW (ST_TO_IPRA_INFO_MAP(255, Malloc_Mem_Pool),
				 Malloc_Mem_Pool);
}

/* =================================================================
 *   CG_Note_Used_Registers
 *
 *   Remember the set of registers used by the current PU.
 */
static void
Note_Used_Registers (IPRA_INFO info)
{
  ISA_REGISTER_CLASS i;

  FOR_ALL_ISA_REGISTER_CLASS(i) {
    info->used_regs[i] = REGISTER_SET_EMPTY_SET;
  }

  for (BB *bb = REGION_First_BB; bb; bb = BB_next(bb)) {
    for (OP *op = BB_first_op(bb); op; op = OP_next(op)) {
      for (INT resnum = 0; resnum < OP_results(op); resnum++) {
	TN *res_tn = OP_result(op,resnum);
	if (TN_is_register(res_tn)) {
	  ISA_REGISTER_CLASS rc = TN_register_class(res_tn);
	  info->used_regs[rc] = REGISTER_SET_Union(info->used_regs[rc],
						   TN_registers (res_tn));
	}
      }
    }
    if (BB_call(bb)) {
      FOR_ALL_ISA_REGISTER_CLASS(i) {
	info->used_regs[i] =
	  REGISTER_SET_Union(info->used_regs[i],
			     BB_call_clobbered(bb, i));
      }
    }
  }

  // Remove the callee-saved registers, we assume they will be preserved.
  // [SC] It would be nice to use Callee_Saved_Regs_Mask here, but that
  // only contains the callee-saves that are spilled/restored in the prologue.
  // If a callee-save is saved by LRA just within a single block
  // (see lra.cxx/Spill_Global_Register), then it is not present in
  // Callee_Saved_Regs_Mask.
  FOR_ALL_ISA_REGISTER_CLASS(i) {
    info->used_regs[i] = REGISTER_SET_Difference (info->used_regs[i],
						  REGISTER_CLASS_callee_saves(i));
  }
}

void
IPRA::Save_Info (const ST *pu)
{
  IPRA_INFO info = CXX_NEW (ipra_info, Malloc_Mem_Pool);
  Note_Used_Registers (info);
  st_to_ipra_info_map->Enter (pu, info);
  if (Get_Trace(TP_GRA, 0x4000)) {
    ISA_REGISTER_CLASS rc;
    fprintf (TFile, "<ipra> Used registers for %s:",
	     ST_name(pu));
    FOR_ALL_ISA_REGISTER_CLASS(rc) {
      REGISTER r;
      FOR_ALL_REGISTER_SET_members (info->used_regs[rc], r) {
	fputc(' ', TFile); REGISTER_Print (rc, r, TFile);
      }
    }
    fputc ('\n', TFile);
  }
}

IPRA_INFO
IPRA::Get_Info (const ST *pu)
{
  return st_to_ipra_info_map->Find (pu);
}
