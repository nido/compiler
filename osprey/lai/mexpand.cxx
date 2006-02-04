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
 * Module: mexpand.cxx
 *
 * Description:
 *
 * Converts ops with register-tuple (e.g. register pair) results or
 * operand into "multi-ops" which have a result (resp. operand)
 * TN for each member of the tuple.
 *
 * ====================================================================
 * ====================================================================
 */

#include "defs.h"
#include "cg.h"
#include "cg_flags.h"
#include "tracing.h"
#include "op_map.h"
#include "cgtarget.h"
#include "cgexp.h"
#include "whirl2ops.h"
#include "op.h"

struct Multi_TN_Info {
  INT count;
  TN **element;
};

static BOOL Trace_MOP;

static MEM_POOL mexpand_pool;

static Multi_TN_Info *
Multi_TN_MAP_Get (
		  TN_MAP map,
		  const TN *tn
		  )
{
  INT i;
  Multi_TN_Info *result = NULL;

  if (TN_is_register (tn) && TN_nhardregs (tn) > 1) {
    result = (Multi_TN_Info *)TN_MAP_Get (map, tn);
    if (result == NULL) {
      const INT nregs = TN_nhardregs (tn);
      result = (Multi_TN_Info *)
	MEM_POOL_Alloc (&mexpand_pool, sizeof(Multi_TN_Info));
      result->count = nregs;
      result->element = (TN **)
	MEM_POOL_Alloc (&mexpand_pool, nregs * sizeof(TN *));
      ISA_REGISTER_CLASS rclass = TN_register_class (tn);
      REGISTER reg = TN_register (tn);
      for (i = 0; i < nregs; i++) {
	result->element[i] = Build_RCLASS_TN (rclass);
	if (TN_is_zero_reg (tn)) {
	  Set_TN_register (result->element[i], reg);
	} else {
	  Set_TN_register (result->element[i], reg + i);
	}
      }
      TN_MAP_Set (map, (TN *)tn, result);
    }
  }
  return result;
}    

static void
Convert_BB_Ops(TN_MAP multi_tn_map, BB *bb)
{
  OP *op, *next;
  
  Trace_MOP = Get_Trace (TP_ALLOC, 0x4000, bb);
  
  for (op = BB_first_op(bb); op != NULL; op = next) {
    BOOL replace = FALSE;
    next = OP_next(op);
    OPS new_ops = OPS_EMPTY;
    TOP opr = OP_code (op);
    
    if (OP_extract(op) || OP_compose(op) || OP_widemove(op)) {
      TN *dest[ISA_OPERAND_max_operands];
      TN *source[ISA_OPERAND_max_operands];
      INT i;
      INT ncopies;
      // Set up dest and source arrays.
      if (OP_extract(op)) {
	Multi_TN_Info *info = Multi_TN_MAP_Get (multi_tn_map, OP_opnd(op, 0));
	ncopies = 0;
	for (i = 0; i < OP_results(op); i++) {
	  // Extract to constant result register can be eliminated.
	  // Note that TN_is_const_reg does not work here because
	  // it rejects non-dedicated TNs.
	  if (! TN_is_zero_reg (OP_result (op, i))) {
	    dest[ncopies] = OP_result(op, i);
	    source[ncopies] = info->element[i];
	    ncopies++;
	  }
	}
      } else if (OP_compose(op)) {
	ncopies = 0;
	// Compose to constant result register can be eliminated.
	// Note that TN_is_const_reg does not work here because
	// it rejects non-dedicated TNs.
	if (! TN_is_zero_reg (OP_result (op, 0))) {
	  Multi_TN_Info *info = Multi_TN_MAP_Get (multi_tn_map, OP_result(op, 0));
	  for (i = 0; i < OP_opnds(op); i++) {
	      dest[ncopies] = info->element[i];
	      source[ncopies] = OP_opnd(op, i);
	      ncopies++;
	  }
	}
      } else {
	// widemove
	if (! TN_is_zero_reg (OP_result (op, 0))) {
	  Multi_TN_Info *dstinfo = Multi_TN_MAP_Get (multi_tn_map, OP_result(op, 0));
	  Multi_TN_Info *srcinfo = Multi_TN_MAP_Get (multi_tn_map, OP_opnd(op, 0));
	  FmtAssert (dstinfo->count == srcinfo->count,
		     ("Destination and source mismatch expanding widemove"));
	  ncopies = 0;
	  for (i = 0; i < dstinfo->count; i++) {
	    dest[ncopies] = dstinfo->element[i];
	    source[ncopies] = srcinfo->element[i];
	    ncopies++;
	  }
	}
      }
      INT to_do = ncopies;
      while (to_do > 0) {
	INT copies_done_this_pass = 0;
	for (i = 0; i < ncopies; i++) {
	  if (dest[i]) {
	    REGISTER rd = TN_register (dest[i]);
	    INT j;
	    for (j = 0; j < ncopies; j++) {
	      if (j != i && source[j] && TN_register(source[j]) == rd) {
		// Another copy needs this destination, so we cannot
		// emit it yet.
		break;
	      }
	    }
	    if (j == ncopies) {
	      Exp_COPY (dest[i], source[i], &new_ops);
	      dest[i] = NULL;
	      source[i] = NULL;
	      copies_done_this_pass++;
	    }
	  }
	}
	FmtAssert (copies_done_this_pass > 0,
		   ("Blocked during expansion"));
	to_do -= copies_done_this_pass;
      }
      replace = TRUE;
      Reset_BB_scheduled (bb);
    } else {
      
      TOP multi_opr = CGTARG_TOP_To_Multi (opr);
      
      if (multi_opr != opr) {
	TN *new_opnds[ISA_OPERAND_max_operands];
	INT new_opndnum = 0;
	TN *new_results[ISA_OPERAND_max_results];
	INT new_resnum = 0;
	INT opndnum;
	INT resnum;
	
	for (opndnum = 0; opndnum < OP_opnds(op); opndnum++) {
	  TN *tn = OP_opnd(op, opndnum);
	  Multi_TN_Info *info = Multi_TN_MAP_Get (multi_tn_map, tn);
	  if (info) {
	    for (INT i = 0; i < info->count; i++) {
	      new_opnds[new_opndnum++] = info->element[i];
	    }
	  } else {
	    new_opnds[new_opndnum++] = tn;
	  }
	}
	for (resnum = 0; resnum < OP_results(op); resnum++) {
	  TN *tn = OP_result(op, resnum);
	  Multi_TN_Info *info = Multi_TN_MAP_Get (multi_tn_map, tn);
	  if (info) {
	    for (INT i = 0; i < info->count; i++) {
	      new_results[new_resnum++] = info->element[i];
	    }
	  } else {
	    new_results[new_resnum++] = tn;
	  }
	}
	OP *new_op = Mk_VarOP (multi_opr, new_resnum, new_opndnum,
			       new_results, new_opnds);
	// Propagate properties from original op to new op.
	OP_Copy_Properties(new_op, op);
	// Also copy WN for alias info.
	Copy_WN_For_Memory_OP (new_op, op);
	OPS_Append_Op (&new_ops, new_op);
	replace = TRUE;
	}
    }
    if (replace) {
      OP *new_op;
      FOR_ALL_OPS_OPs (&new_ops, new_op) {
	OP_scycle (new_op) = OP_scycle (op);
	OP_srcpos (new_op) = OP_srcpos (op);
      }
      BB_Insert_Ops_Before (bb, op, &new_ops);
      BB_Remove_Op (bb, op);
    }
  }
}


void Convert_To_Multi_Ops (void)
{
  BB *bb;

  MEM_POOL_Initialize (&mexpand_pool, "MOP_Expand_pool", FALSE);

  TN_MAP multi_tn_map = TN_MAP_Create();

  for (bb = REGION_First_BB; bb != NULL; bb = BB_next(bb)) {
    Convert_BB_Ops(multi_tn_map, bb);
  }

  TN_MAP_Delete (multi_tn_map);
  MEM_POOL_Delete (&mexpand_pool);
}
  
void Convert_BB_To_Multi_Ops (BB *bb)
{
  MEM_POOL_Initialize (&mexpand_pool, "MOP_Expand_pool", FALSE);

  TN_MAP multi_tn_map = TN_MAP_Create();

  Convert_BB_Ops(multi_tn_map, bb);

  TN_MAP_Delete (multi_tn_map);
  MEM_POOL_Delete (&mexpand_pool);
}
  
