/*

  Copyright (C) 2000, 2001 Silicon Graphics, Inc.  All Rights Reserved.

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

  Contact information:  Silicon Graphics, Inc., 1600 Amphitheatre Pky,
  Mountain View, CA 94043, or:

  http://www.sgi.com

  For further information regarding this notice, see:

  http://oss.sgi.com/projects/GenInfo/NoticeExplan

*/


/* =======================================================================
 * =======================================================================
 *
 *  Module: ebo.cxx
 *  $Revision$
 *  $Date$
 *  $Author$
 *  $Source$
 *
 *  Revision comments:
 *
 *  29-May-1998 - Initial version
 *
 *  Description:
 *  ============
 *
 *  EBO implementation.  See "ebo.h" for interface.
 *
 * =======================================================================
 * =======================================================================
 */

/* =======================================================================
 * =======================================================================
 *
 * This is the Extended Block Optimizer (EBO).
 *
 * It is a peep hole optimizer that works on a sequence of blocks that may
 * contain branches out, but can only be execxuted from the start of the
 * first block in the sequence.  These sequences are recognized during
 * processing.
 *
 * Instructions are processed in the forward direction through each block
 * and in the forward direction through the block's successor list.  New
 * blocks are processed until a "branched to" label is encountered, at
 * which time processing backs up and attempts to take a different path
 * down another successor list.
 *
 * The optimizations performed include forward propagation, common expression
 * elimination, constant folding, dead code elimination and a host of special
 * case transformations that are unique to the arcitecture of a particular
 * machine.
 *
 * In order to perform the optimizations it is nececssary to recognize the
 * definition and use connections between values.  Since this routine may be
 * called several times during compilation, and may have different information
 * available each time it is called, it is necessary to abstract the code
 * representation of the original program so that data values can be easily
 * tracked by common routines.
 *
 * The data structure used to abstract the values is the EBO_TN_INFO.  One is
 * created the first time a value is encoutered.  The EBO_TN_INFO entry is
 * associated with the register, if one has been assigned, or the TN.  This
 * allows us to support special, hard coded registers that are assigned early
 * in the code generation process, provided that references to the value
 * always use the register name to refer to that particular value.
 *
 * The data structure used to abstract each OP is the EBO_OP_INFO.  A unique
 * one is created for each OP that is encountered and contain pointers to
 * the unique EBO_TN_INFO entries that represent the values used and defined
 * by the original OP.
 *
 * These data structures make it simple to track predicated values and allow
 * for the redefintion of TNs and regeisters in a block, while providing
 * access to the previous definition.  It also simplifies the task of tracking
 * predicated code that reuses TN names and registers but define unique values.
 *
 * The construction of this abstraction is done as early s possible, allowing
 * the rest of EBO to work only with these *_INFO entries.  The design supports
 * tracking of constants, even when the original instructions can not directly
 * reference constants in their operand fields.
 *
 */

#ifdef USE_PCH
#include "cg_pch.h"
#endif // USE_PCH
#pragma hdrstop

#ifdef _KEEP_RCS_ID
static const char source_file[] = __FILE__;
#endif /* _KEEP_RCS_ID */

#include "W_alloca.h"
#include <stdarg.h>

#include "defs.h"
#include "errors.h"
#include "mempool.h"
#include "tracing.h"
#include "timing.h"
#include "cgir.h"
#include "tn_map.h"
#include "cg_loop.h"
#include "cg.h"
#include "cgexp.h"
#include "register.h"
#include "cg_region.h"
#include "wn.h"
#include "region_util.h"
#include "op_list.h"
#include "cgprep.h"
#include "gtn_universe.h"
#include "gtn_set.h"
#include "cg_db_op.h"
#include "whirl2ops.h"
#include "cgtarget.h"
#include "gra_live.h"
#include "reg_live.h"
#include "cflow.h"
#include "cg_spill.h"
#include "targ_proc_properties.h"
#include "top_properties.h"

#include "ebo.h"
#include "ebo_info.h"
#include "ebo_special.h"
#include "ebo_util.h"

/* ===================================================================== */
/* Global Data:								 */
/* ===================================================================== */

INT32 EBO_Opt_Level_Default = 5;
INT32 EBO_Opt_Level = 5;
BOOL  CG_skip_local_ebo = FALSE;

INT EBO_tninfo_number = 0;
EBO_TN_INFO *EBO_free_tninfo = NULL;
EBO_TN_INFO *EBO_first_tninfo = NULL;
EBO_TN_INFO *EBO_last_tninfo = NULL;
EBO_OP_INFO *EBO_free_opinfo = NULL;
EBO_OP_INFO *EBO_first_opinfo = NULL;
EBO_OP_INFO *EBO_last_opinfo = NULL;
EBO_OP_INFO *EBO_opinfo_table[EBO_MAX_OP_HASH];

/* Entry point indicators. */
BOOL EBO_in_pre  = FALSE;
BOOL EBO_in_before_unrolling = FALSE;
BOOL EBO_in_after_unrolling = FALSE;
BOOL EBO_in_peep = FALSE;

/* Are OMEGA entries present? */
BOOL EBO_in_loop = FALSE;

TN_MAP EBO_tninfo_table;
MEM_POOL EBO_pool;

INT EBO_num_tninfo_entries = 0;
INT EBO_tninfo_entries_reused = 0;
INT EBO_num_opinfo_entries = 0;
INT EBO_opinfo_entries_reused = 0;

char *EBO_trace_pfx;
BOOL EBO_Trace_Execution    = FALSE;
BOOL EBO_Trace_Optimization = FALSE;
BOOL EBO_Trace_Block_Flow   = FALSE;
BOOL EBO_Trace_Data_Flow    = FALSE;
BOOL EBO_Trace_Hash_Search  = FALSE;

#ifdef TARG_ST
// [CG] Use alias information from cg_dep
#define CG_DEP_ALIAS
#endif

#ifdef TARG_ST
/* ==============================================================
 *   Arthur: Some utility functions that I moved here from the
 *           target-specific part. These seem pretty generic.
 * ==============================================================
 */

/* =====================================================================
 *   Get_Right_Mask_Length
 *
 *   Determine the number of consecutive right justified bits that are 
 *   in the mask. Return -1, if the constant is not a properly 
 *   constructed mask.
 * =====================================================================
 */
INT
Get_Right_Mask_Length (
  UINT64 mask
)
{
  INT64 i;

  if (mask == 0) return -1;
  if ((mask & 1) == 0) return -1;

  for ( i = 1; i < 64; i++)
  {
    if (((mask >> i) & 1) == 0) return ((mask >> i) == 0) ? i : -1;
  }

  return -1;
}

/* =====================================================================
 *   Get_Mask_Shift_Count
 *
 *   Determine the number of trailing zeros after a mask of all "1" 
 *   bits.  Return -1, if the constant is not a properly constructed 
 *   mask.
 * =====================================================================
 */
INT
Get_Mask_Shift_Count (
UINT64 mask
)
{
  INT64 i;

  if (mask == 0) return -1;

  for ( i = 0; i < 63; i++)
  {
    if ((mask & (1 << i)) != 0) return i;
  }

  return -1;
}

/* =====================================================================
 *   EBO_Set_OP_omega
 * =====================================================================
 */
void
EBO_Set_OP_omega (
  OP *op, 
  ...
)
{
  INT opnds = OP_opnds(op);
  INT i;
  va_list tninfos;

  va_start(tninfos, op);
  CG_LOOP_Init_Op(op);
  for (i = 0; i < opnds; i++) {
    EBO_TN_INFO *tninfo = va_arg(tninfos, EBO_TN_INFO *);
    Set_OP_omega (op, i, ((tninfo != NULL) ? tninfo->omega : 0));
  }

  va_end(tninfos);
  return;
}

/* =====================================================================
 *   EBO_Copy_OP_omega
 * =====================================================================
 */
void
EBO_Copy_OP_omega (
  OP *new_op, 
  OP *old_op
)
{
  INT opnds = OP_opnds(new_op);
  INT i;

  CG_LOOP_Init_Op(new_op);
  for (i = 0; i < opnds; i++) {
    Set_OP_omega (new_op, i, OP_omega(old_op,i));
  }

  return;
}

/* =====================================================================
 *   EBO_Set_Predicate_omega
 * =====================================================================
 */
void
EBO_Set_Predicate_omega (
  OP *new_op, 
  EBO_TN_INFO *pred_tninfo
)
{
  if (pred_tninfo != NULL && OP_PREDICATE_OPND >= 0) {
    Set_OP_omega (new_op, (UINT32)OP_PREDICATE_OPND, pred_tninfo->omega);
  }
  return;
}


/* =====================================================================
 *   EBO_OPS_omega
 * =====================================================================
 */
void
EBO_OP_omega (
  OP *op, 
  TN *opnd,
  EBO_TN_INFO *opnd_tninfo
)
{
  INT opnds = OP_opnds(op);
  INT i;

  if (opnd_tninfo == NULL) return;


  CG_LOOP_Init_Op(op);
  for (i = 0; i < opnds; i++) {
    if (OP_opnd(op, i) == opnd) {
      Set_OP_omega (op, i, opnd_tninfo->omega);
    }
  }
}

/* =====================================================================
 *   EBO_OPS_omega
 * =====================================================================
 */
void
EBO_OPS_omega (
  OPS *ops, 
  TN *opnd,
  EBO_TN_INFO *opnd_tninfo
)
{
  OP *next_op = OPS_first(ops);
  if (opnd_tninfo == NULL) return;

  while (next_op != NULL) {
    EBO_OP_omega(next_op, opnd, opnd_tninfo);
    next_op = OP_next(next_op);
  }

  return;
}

#endif

#ifdef TARG_ST
// [CG] some helper functions
static INT64 sext(INT64 value, int bits)
{
  return (value << 64-bits) >> 64-bits;
}

static INT64 zext(INT64 value, int bits)
{
  return (UINT64)(value << 64-bits) >> 64-bits;
}

#endif

/* ===================================================================== */
/* Local Data:								 */
/* ===================================================================== */

static BOOL in_delay_slot = FALSE;
static BOOL rerun_cflow = FALSE;

/* ===================================================================== */

/* The BB flag: <local_flag1> is overloaded temporarily in this routine
 * as <visited> to keep track of the fact that we have seen this block
 * during processing. The bit will NOT be cleared by the time we exit.
 */
#define BB_visited          BB_local_flag1
#define Set_BB_visited      Set_BB_local_flag1
#define Reset_BB_visited    Reset_BB_local_flag1

inline void clear_bb_flag(BB *first_bb)
{
  BB *bb;
  for (bb = first_bb; bb != NULL; bb = BB_next(bb)) {
    BBLIST *succ_list;

    Reset_BB_visited(bb);

    FOR_ALL_BB_SUCCS(bb, succ_list) { 
      BB *succ = BBLIST_item(succ_list);
      Reset_BB_visited(succ);
    }

  }
}

/* ===================================================================== */

static
BOOL EBO_Fix_Select_Same_Args (OP *op,
			       TN **opnd_tn,
			       EBO_TN_INFO **opnd_tninfo)
{
  TN *res = OP_result(op, 0);
  TN *tn1;
  TN *tn2;

  /* For special case optimizations, check the OPTIMAL operands. */
  tn1 = opnd_tn[1];
  tn2 = opnd_tn[2];

  if (tn_registers_identical(tn1, tn2)) {
    /* We can optimize this! But return the ACTUAL operand. */
    OPS ops = OPS_EMPTY;
    Exp_COPY(res, OP_opnd(op, 1), &ops);
    BB_Insert_Ops(OP_bb(op), op, &ops, FALSE);
    if (EBO_Trace_Optimization) {
      #pragma mips_frequency_hint NEVER
      fprintf(TFile,"Optimize select - operands are the same\n");
    }
    return TRUE;
  }

  return FALSE;
}

/* ===================================================================== */

static
BOOL EBO_Fix_Same_Res_Op (OP *op,
                          TN **opnd_tn,
                          EBO_TN_INFO **opnd_tninfo)
{
  if (EBO_in_loop) return FALSE;;

  if (OP_unalign_ld(op)) {
    TN *res = OP_result(op, 0);
    TN *tnl = OP_opnd(op, OP_opnds(op)-1);

    if (!TN_is_zero_reg(tnl) && !tn_registers_identical(res, tnl)) {
     /* Allocate a new TN for the result. */
      OPS ops = OPS_EMPTY;
      TN *new_res = Dup_TN (res);
      OP *new_op = Dup_OP (op);
      Exp_COPY(new_res, tnl, &ops);
      Set_OP_result(new_op, 0, new_res);
      Set_OP_opnd(new_op, OP_opnds(op)-1, new_res);
      OPS_Append_Op(&ops, new_op);
      Exp_COPY(res, new_res, &ops);
      BB_Insert_Ops(OP_bb(op), op, &ops, FALSE);
      return TRUE;
    }

  } else if (OP_select(op)) {
    TN *res = OP_result(op, 0);
    TN *tn0 = OP_opnd(op, 0);
    TN *tn1;
    TN *tn2;

    if (EBO_Fix_Select_Same_Args(op, opnd_tn, opnd_tninfo)) {
      return TRUE;
    }

   /* For special case optimizations, check the OPTIMAL operands. */
    tn1 = opnd_tn[1];
    tn2 = opnd_tn[2];

    if (TN_is_global_reg(tn1) &&
        (opnd_tninfo[1] != NULL) &&
        (opnd_tninfo[1]->in_op == NULL) &&
        (opnd_tninfo[1]->in_bb != NULL) &&
	(opnd_tninfo[1]->in_bb != OP_bb(op)) &&
        !tn_has_live_def_into_BB(tn1, opnd_tninfo[1]->in_bb)) {
     /* Assume that this value will not be used -
        turn this instruction into a copy of the other operand.
        But return the ACTUAL operand. */
      OPS ops = OPS_EMPTY;
      Exp_COPY(res, OP_opnd(op, 2), &ops);
      BB_Insert_Ops(OP_bb(op), op, &ops, FALSE);
      if (EBO_Trace_Optimization) {
        #pragma mips_frequency_hint NEVER
        fprintf(TFile,"Optimize select - op1 %s can't be used: ",
                EBO_in_peep ? "REG" : "TN");
        Print_TN(OP_opnd(op, 2), FALSE);
        fprintf(TFile,"\n");
      }
      return TRUE;
    }

    if (TN_is_global_reg(tn2) &&
        (opnd_tninfo[2] != NULL) &&
        (opnd_tninfo[2]->in_op == NULL) &&
        (opnd_tninfo[2]->in_bb != NULL) &&
        (opnd_tninfo[2]->in_bb != OP_bb(op)) &&
        !tn_has_live_def_into_BB(tn2, opnd_tninfo[2]->in_bb)) {
    /* Assume that this value will not be used -
        turn this instruction into a copy of the other operand.
        But return the ACTUAL operand. */
      OPS ops = OPS_EMPTY;
      Exp_COPY(res, OP_opnd(op, 1), &ops);
      BB_Insert_Ops(OP_bb(op), op, &ops, FALSE);
      if (EBO_Trace_Optimization) {
        #pragma mips_frequency_hint NEVER
        fprintf(TFile,"Optimize select - op2 %s can't be used: ",
                EBO_in_peep ? "REG" : "TN");
        Print_TN(OP_opnd(op, 1), FALSE);
        fprintf(TFile,"\n");
      }
      return TRUE;
    }

   /* For ensuring that the result operand matches one of the input operands,
      test the ACTUAL operands used in the expression. */
    tn0 = OP_opnd(op, 0);
    tn1 = OP_opnd(op, 1);
    tn2 = OP_opnd(op, 2);
    if ((TN_Is_Constant(tn1) || !tn_registers_identical(res, tn1)) &&
        (TN_Is_Constant(tn2) || !tn_registers_identical(res, tn2))) {
      OPS ops = OPS_EMPTY;
      OP *new_op = Dup_OP (op);

      if (has_assigned_reg(res)) {
       /* Use the existing result TN as the duplicate input. */
        TN *new_res = OP_result(op,0);
        FmtAssert((TN_Is_Constant(tn0) || !tn_registers_identical(res, tn0)),
                  ("Condition code also used as result of select"));
        if (TN_Is_Constant(tn2)) {
          Exp_COPY(new_res, tn2, &ops);
          Set_OP_opnd(new_op, 2, new_res);
        } else {
          Exp_COPY(new_res, tn1, &ops);
          Set_OP_opnd(new_op, 1, new_res);
        }
        OPS_Append_Op(&ops, new_op);
      } else {
       /* Allocate a new TN for the result. */
        TN *new_res = Dup_TN (res);
        Exp_COPY(new_res, tn1, &ops);
        Set_OP_result(new_op, 0, new_res);
        Set_OP_opnd(new_op, 1, new_res);
        OPS_Append_Op(&ops, new_op);
        Exp_COPY(res, new_res, &ops);
      }

      BB_Insert_Ops(OP_bb(op), op, &ops, FALSE);
      if (EBO_Trace_Optimization) {
        #pragma mips_frequency_hint NEVER
        fprintf(TFile,"Rewrite select\n");
      }
      return TRUE;
    }
  }

  return FALSE;

}

inline BOOL TN_live_out_of(TN *tn, BB *bb)
/* -----------------------------------------------------------------------
 * Requires: global liveness info up-to-date
 * Return TRUE iff <tn> is live out of <bb>.
 * -----------------------------------------------------------------------
 */
{
  if (EBO_Trace_Execution) {
    #pragma mips_frequency_hint NEVER
    fprintf(TFile,"%sEnter TN_live_out_of BB:%d ",EBO_trace_pfx,BB_id(bb));
    Print_TN(tn, FALSE);
    fprintf(TFile,"\n");
  }

  if (EBO_in_peep) {
    if (EBO_Trace_Data_Flow) {
      #pragma mips_frequency_hint NEVER
      fprintf(TFile,"REG_LIVE_Outof_BB %s\n",
         REG_LIVE_Outof_BB (TN_register_class(tn), TN_register(tn), bb)?"TRUE":"FALSE");
    }
    return REG_LIVE_Outof_BB (TN_register_class(tn), TN_register(tn), bb);
  }
  else {
    if (CG_localize_tns) return (TN_is_dedicated(tn) || TN_is_global_reg(tn));
    return GRA_LIVE_TN_Live_Outof_BB (tn, bb);
  }
}



inline BOOL op_is_needed_globally(OP *op)
/* -----------------------------------------------------------------------
 * Requires: global liveness info
 * Return TRUE iff the result of <op> is necessary past the end of its BB.
 * ----------------------------------------------------------------------- */
{
  BB *bb = OP_bb(op);

  if (OP_copy(op)) {
   /* Copies don't have side effects unless a save_reg is involved. */
    if (OP_glue(op) && !EBO_in_peep)
      return TRUE;
#ifdef TARG_ST
    int result_num = OP_Copy_Result(op);
    int opnd_num = OP_Copy_Operand(op);
    if ((TN_is_save_reg(OP_result(op,result_num)) || 
	 TN_is_save_reg(OP_opnd(op,opnd_num))) &&
        !tn_registers_identical(OP_result(op,result_num), OP_opnd(op,opnd_num))) {
#else
    if ((TN_is_save_reg(OP_result(op,0)) || 
	 TN_is_save_reg(OP_opnd(op,OP_Copy_Operand(op)))) &&
        !tn_registers_identical(OP_result(op,0), OP_opnd(op,OP_Copy_Operand(op)))) {
#endif
      return TRUE;
    }
    return FALSE;
  }
#ifdef TARG_ST
  // [CG]: iterate over all results
  for (int i = 0; i < OP_results(op); i++) {
    if (TN_is_save_reg(OP_result(op,i)))
      return TRUE;
  }
#else
  if (TN_is_save_reg(OP_result(op,0)))
    return TRUE;
#endif
  if (OP_glue(op) && !has_assigned_reg(OP_result(op,0)))
    return TRUE;
  if (CGTARG_Is_OP_Intrinsic(op))
   /* Intrinsic ops may have side effects we don't understand */
    return TRUE;
  if (OP_call(op)) 
   /* Calls may have side effects we don't understand */
    return TRUE;
  if (op == BB_exit_sp_adj_op(bb) || op == BB_entry_sp_adj_op(bb))
    return TRUE;
  return FALSE;
}


/* ===================================================================== */


void
tn_info_entry_dump (EBO_TN_INFO *tninfo)
{
  fprintf(TFile,"entry %d\tBB:%d, use count = %d, redefined = %s, same as %d, predicate %d:  ",
          tninfo->sequence_num,tninfo->in_bb?BB_id(tninfo->in_bb):0,
          tninfo->reference_count,
          tninfo->redefined_before_block_end?"TRUE":"FALSE",
          tninfo->same?tninfo->same->sequence_num:0,
          tninfo->predicate_tninfo?tninfo->predicate_tninfo->sequence_num:0);
  Print_TN (tninfo->local_tn, TRUE);
  fprintf(TFile,"[%d]",tninfo->omega);
  if (tninfo->replacement_tn != NULL) {
    fprintf(TFile,"\n\treplace TN with: ");
    Print_TN (tninfo->replacement_tn, TRUE);
    if (tninfo->replacement_tninfo != NULL) {
      fprintf(TFile," (Entry Number %d)",
              tninfo->replacement_tninfo->sequence_num);
    }
  }
  fprintf(TFile,"\n");

#ifdef TARG_ST
  if (tninfo->copy_tn != NULL) {
    fprintf(TFile,"\n\tcopy of TN: ");
    Print_TN (tninfo->copy_tn, TRUE);
    if (tninfo->copy_tninfo != NULL) {
      fprintf(TFile," (Entry Number %d)",
              tninfo->copy_tninfo->sequence_num);
    }
  }
  fprintf(TFile,"\n");
#endif
  if (tninfo->in_op) {
    fprintf(TFile,"\t");
    Print_OP_No_SrcLine(tninfo->in_op);
  }

  FmtAssert((TN_number(tninfo->local_tn) <= Last_TN),
                  ("TN number exceeds allowed range"));

}


void
tn_info_table_dump ()
{
  EBO_TN_INFO *tninfo = EBO_first_tninfo;

  fprintf(TFile,"\n>>>>>> EBO INFO DUMP <<<<<\n");

  while (tninfo != NULL) {
    tn_info_entry_dump(tninfo);
    tninfo = tninfo->next;
  }

  fprintf(TFile,">>>>>> EBO INFO DUMP COMPLETE <<<<<\n\n");

}

void EBO_Init(void)
/* -----------------------------------------------------------------------
 * See "ebo.h" for interface.
 * -----------------------------------------------------------------------
 */
{
  EBO_Trace_Execution    = FALSE;
  EBO_Trace_Optimization = FALSE;
  EBO_Trace_Block_Flow   = FALSE;
  EBO_Trace_Data_Flow    = FALSE;
  EBO_Trace_Hash_Search  = FALSE;

  MEM_POOL_Initialize(&EBO_pool, "ebo", FALSE);
  MEM_POOL_Push(&EBO_pool);
  EBO_tninfo_table = NULL;

  EBO_tninfo_number = 0;
  EBO_free_tninfo = NULL;
  EBO_first_tninfo = NULL;
  EBO_last_tninfo = NULL;

  EBO_free_opinfo = NULL;
  EBO_first_opinfo = NULL;
  EBO_last_opinfo = NULL;

  BZERO(EBO_opinfo_table,sizeof(EBO_opinfo_table));

  EBO_num_tninfo_entries = 0;
  EBO_tninfo_entries_reused = 0;
  EBO_num_opinfo_entries = 0;
  EBO_opinfo_entries_reused = 0;
  EBO_trace_pfx = "<ebo> ";
}




static void EBO_Start()
/* -----------------------------------------------------------------------
 * -----------------------------------------------------------------------
 */
{
 /* Initialize data structures.  */
  MEM_POOL_Push(&MEM_local_pool);
  EBO_tninfo_table = TN_MAP_Create();

}



static void EBO_Finish(void)
/* -----------------------------------------------------------------------
 * -----------------------------------------------------------------------
 */
{
  TN_MAP_Delete (EBO_tninfo_table);
  EBO_tninfo_table = NULL;
  MEM_POOL_Pop(&MEM_local_pool);
}




void EBO_Finalize(void)
/* -----------------------------------------------------------------------
 * See "ebo.h" for interface.
 * -----------------------------------------------------------------------
 */
{
  MEM_POOL_Pop(&EBO_pool);
  MEM_POOL_Delete(&EBO_pool);
}


/* ===================================================================== */

  
static INT copy_operand(OP *op)
/* -------------------------------------------------------------------------
 *
 * Return the operand index for the operand that is copied.
 * -------------------------------------------------------------------------
 */
{
  INT opnd;

#ifdef TARG_ST
  return OP_Copy_Operand(op);
#else
  if (OP_copy(op)) {
    return OP_COPY_OPND(op);
  }
  opnd = CGTARG_Copy_Operand(op);
  if (opnd >= 0) {
    return opnd;
  }
  opnd = EBO_Copy_Operand(op);
  if (opnd >= 0) {
    return opnd;
  }
  return -1;
#endif
}

/* =======================================================================
 *   find_index_and_offset
 * =======================================================================
 */
static void
find_index_and_offset (
  EBO_TN_INFO *original_tninfo,
  TN **base_tn, 
  EBO_TN_INFO **base_tninfo,
  TN **offset_tn, 
  EBO_TN_INFO **offset_tninfo
)
{
 /* Look for an offset descriptor in the index. */
  EBO_OP_INFO *indx_opinfo = locate_opinfo_entry(original_tninfo);

#ifdef TARG_ST
  //
  // Use target-specific routine to get the indices
  //
  INT op1_idx;
  INT op2_idx;

  if ((indx_opinfo != NULL) &&
      (indx_opinfo->in_op != NULL) &&
      (OP_iadd(indx_opinfo->in_op) || 
       EBO_Can_Merge_Into_Offset(indx_opinfo->in_op, &op1_idx, &op2_idx))) {
#else
  if ((indx_opinfo != NULL) &&
      (indx_opinfo->in_op != NULL) &&
      (OP_iadd(indx_opinfo->in_op) || 
       EBO_Can_Merge_Into_Offset(indx_opinfo->in_op))) {
    INT op1_idx = TOP_Find_Operand_Use(OP_code(indx_opinfo->in_op),OU_opnd1);
    INT op2_idx = TOP_Find_Operand_Use(OP_code(indx_opinfo->in_op),OU_opnd2);
#endif

    if ((op1_idx >= 0) && (op2_idx >= 0)) {
      EBO_TN_INFO *op1_tninfo = indx_opinfo->actual_opnd[op1_idx];
      EBO_TN_INFO *op2_tninfo = indx_opinfo->actual_opnd[op2_idx];
      TN *op1_tn;
      TN *op2_tn;
      if (op1_tninfo != NULL) {
        if ((op1_tninfo->replacement_tn) &&
            (TN_is_symbol(op1_tninfo->replacement_tn) || TN_Is_Constant(op1_tninfo->replacement_tn))) {
          op1_tn = op1_tninfo->replacement_tn;
          op1_tninfo = op1_tninfo->replacement_tninfo;
        } else {
          op1_tn = op1_tninfo->local_tn;
        }
      } else {
        op1_tn = OP_opnd(indx_opinfo->in_op,op1_idx);
      }
      if (op2_tninfo != NULL) {
        if ((op2_tninfo->replacement_tn) &&
            (TN_is_symbol(op2_tninfo->replacement_tn) || TN_Is_Constant(op2_tninfo->replacement_tn))) {
          op2_tn = op2_tninfo->replacement_tn;
          op2_tninfo = op2_tninfo->replacement_tninfo;
        } else {
          op2_tn = op2_tninfo->local_tn;
        }
      } else {
        op2_tn = OP_opnd(indx_opinfo->in_op,op2_idx);
      }

      if (TN_is_symbol(op1_tn) || TN_Is_Constant(op1_tn)) {
        TN *save = op1_tn;
        op1_tn = op2_tn;
        op2_tn = save;
        op1_tninfo = op2_tninfo;
        op2_tninfo = NULL;
      }

      *base_tn = op1_tn;
      *base_tninfo = op1_tninfo;
      *offset_tn = op2_tn;
      *offset_tninfo = op2_tninfo;

      if (EBO_Trace_Hash_Search) {
        #pragma mips_frequency_hint NEVER
        fprintf(TFile,"%sUse inputs to index: ",EBO_trace_pfx);
        Print_TN(*base_tn,FALSE);
        fprintf(TFile," ");
        Print_TN(*offset_tn,FALSE);
        fprintf(TFile,"\n");
      }
    }
  }
  else if ((indx_opinfo != NULL) &&
      (indx_opinfo->in_op != NULL) &&
      (OP_memory(indx_opinfo->in_op))) {
    OP *input_op = indx_opinfo->in_op;
    INT op1_idx = TOP_Find_Operand_Use(OP_code(input_op),OU_base);
    INT op2_idx = TOP_Find_Operand_Use(OP_code(input_op),OU_postincr);

    if ((op1_idx >= 0) &&
        (op2_idx >= 0) &&
        TNs_Are_Equivalent(OP_opnd(input_op,op1_idx),original_tninfo->local_tn)) {

      EBO_TN_INFO *op1_tninfo = indx_opinfo->actual_opnd[op1_idx];
      EBO_TN_INFO *op2_tninfo = indx_opinfo->actual_opnd[op2_idx];
      TN *op1_tn;
      TN *op2_tn;

      if (op1_tninfo != NULL) {
        if ((op1_tninfo->replacement_tn) &&
            (TN_is_symbol(op1_tninfo->replacement_tn) || TN_Is_Constant(op1_tninfo->replacement_tn))) {
          op1_tn = op1_tninfo->replacement_tn;
          op1_tninfo = op1_tninfo->replacement_tninfo;
        } else {
          op1_tn = op1_tninfo->local_tn;
        }
      } else {
        op1_tn = OP_opnd(indx_opinfo->in_op,op1_idx);
      }
      if (op2_tninfo != NULL) {
        if ((op2_tninfo->replacement_tn) &&
            (TN_is_symbol(op2_tninfo->replacement_tn) || TN_Is_Constant(op2_tninfo->replacement_tn))) {
          op2_tn = op2_tninfo->replacement_tn;
          op2_tninfo = op2_tninfo->replacement_tninfo;
        } else {
          op2_tn = op2_tninfo->local_tn;
        }
      } else {
        op2_tn = OP_opnd(indx_opinfo->in_op,op2_idx);
      }

      if (TN_is_symbol(op1_tn) || TN_Is_Constant(op1_tn)) {
        TN *save = op1_tn;
        op1_tn = op2_tn;
        op2_tn = save;
        op1_tninfo = op2_tninfo;
        op2_tninfo = NULL;
      }

      *base_tn = op1_tn;
      *base_tninfo = op1_tninfo;
      *offset_tn = op2_tn;
      *offset_tninfo = op2_tninfo;

      if (EBO_Trace_Hash_Search) {
        #pragma mips_frequency_hint NEVER
        fprintf(TFile,"%sUse inputs to index: ",EBO_trace_pfx);
        Print_TN(*base_tn,FALSE);
        fprintf(TFile," ");
        Print_TN(*offset_tn,FALSE);
        fprintf(TFile,"\n");
      }
    }
  }
}

#ifndef TARG_ST
  // [CG]: Obsolete. This functionality is handled by EBO_Memory_Sequence

/* =======================================================================
 *   merge_memory_offsets
 * =======================================================================
 */
static void
merge_memory_offsets ( 
  OP *op,
  INT index_opnd,
  TN **opnd_tn,
  EBO_TN_INFO **opnd_tninfo,
  EBO_TN_INFO **actual_tninfo
)
{
  EBO_TN_INFO *index_tninfo = opnd_tninfo[index_opnd];
  OP *index_op = (index_tninfo != NULL) ? index_tninfo->in_op : NULL;

#ifdef TARG_ST
  //
  // This shouldn't suppose a particular OP layout
  //
  INT immed_opnd = OP_find_opnd_use(op, OU_offset);
  FmtAssert(immed_opnd >= 0, ("-1 immed_opnd"));
  TN *immed_tn = opnd_tn[immed_opnd];
#else
  TN *immed_tn = opnd_tn[index_opnd+1];
#endif

  ST *immed_sym = TN_is_symbol(immed_tn) ? TN_var(immed_tn) : NULL;
  INT64 immed_offset = TN_is_symbol(immed_tn) ? TN_offset(immed_tn) : TN_Value(immed_tn);
  EBO_OP_INFO *index_opinfo;
  TN *additive_index_tn;
  TN *additive_immed_tn;
  EBO_TN_INFO *additive_index_tninfo;

  TN *adjust_tn = NULL;
  ST *adjust_sym = NULL;
  INT64 adjust_offset = 0;
  TN *new_tn;

#ifdef TARG_ST
  //
  // The IA64 implementation looks very target-specific
  // Instead, we will get the additive parts from a target-specific
  // routine.
  //
  INT additive_index;
  INT additive_immed;

  if ((index_tninfo == NULL) ||
      (index_op == NULL) ||
      (!EBO_Can_Merge_Into_Offset (index_op, 
				   &additive_index, 
				   &additive_immed))) {
    return;
  }

  index_opinfo = locate_opinfo_entry (index_tninfo);
  if (index_opinfo == NULL) return;

  additive_index_tn = OP_opnd(index_op, additive_index);
  additive_index_tninfo = index_opinfo->actual_opnd[additive_index];

  additive_immed_tn = OP_opnd(index_op, additive_immed);
  if (!TN_Is_Constant(additive_immed_tn)) return;
#else
  if ((index_tninfo == NULL) ||
      (index_op == NULL) ||
      (!EBO_Can_Merge_Into_Offset (index_op))) {
    return;
  }

  index_opinfo = locate_opinfo_entry (index_tninfo);
  if (index_opinfo == NULL) return;

  additive_index_tn = OP_opnd(index_op,0);
  additive_index_tninfo = index_opinfo->actual_opnd[0];

  additive_immed_tn = OP_opnd(index_op,1);
  if (!TN_Is_Constant(additive_immed_tn)) return;
#endif

 /* Would the new index value be available for use? */
  if (!TN_Is_Constant(additive_index_tn) &&
      !EBO_tn_available(OP_bb(op), additive_index_tninfo)) {
    return;
  }

  if (TN_is_symbol(additive_immed_tn)) {
    if (immed_sym != NULL) return;
    if (OP_isub(index_op)) return;
    if (immed_offset != 0) {
     /* Don't move relocation from an ADD instruction to a memory
        offset field if there is a chance that the (relocation +
        offset) value might overflow the field. */
      return;
    }
    adjust_sym = TN_var(additive_immed_tn);
    adjust_tn = additive_immed_tn;
    adjust_offset = immed_offset + TN_offset(additive_immed_tn);
  } else {
    adjust_sym = immed_sym;
    adjust_tn = immed_tn;
    adjust_offset = TN_Value(additive_immed_tn);
    if (OP_isub(index_op)) adjust_offset = -adjust_offset;
    adjust_offset += immed_offset;
  }

  if (!TOP_Can_Have_Immediate(adjust_offset, OP_code(op))) {\
    if (EBO_Trace_Optimization) {
      #pragma mips_frequency_hint NEVER
      fprintf(TFile, "%sin BB:%d combined index expressions do not fit in the offset field\n",
              EBO_trace_pfx, BB_id(OP_bb(op)));
      Print_OP_No_SrcLine(op);
    }
    return;
  }

  if (EBO_Trace_Optimization) {
    #pragma mips_frequency_hint NEVER
    fprintf(TFile, "%sin BB:%d merge memory index expression (from BB:%d) with offset (in BB:%d)\n",
            EBO_trace_pfx, BB_id(OP_bb(op)),BB_id(OP_bb(index_op)),BB_id(OP_bb(op)));
    Print_OP_No_SrcLine(index_op);
    Print_OP_No_SrcLine(op);
  }

 /* Create a combined TN and upated the op and other data structures. */
  if (adjust_sym != NULL) {
    new_tn = Gen_Symbol_TN(adjust_sym, adjust_offset, TN_relocs(adjust_tn));
  } else {
    new_tn = Gen_Literal_TN (adjust_offset, TN_size(adjust_tn));
  }

  if (actual_tninfo[index_opnd] != NULL) {
    dec_ref_count(actual_tninfo[index_opnd]);
  }
  if (additive_index_tninfo != NULL) {
    inc_ref_count(additive_index_tninfo);
  }
  Set_OP_opnd(op, index_opnd, additive_index_tn);
  if (EBO_in_loop) {
    Set_OP_omega (op, index_opnd, (additive_index_tninfo != NULL) ? additive_index_tninfo->omega : 0);
  }

#ifdef TARG_ST
  Set_OP_opnd(op, immed_opnd, new_tn);
  if (EBO_in_loop) {
    Set_OP_omega (op, immed_opnd, 0);
  }
  opnd_tn[index_opnd] = additive_index_tn;
  opnd_tn[immed_opnd] = new_tn;
  opnd_tninfo[index_opnd] = additive_index_tninfo;
  opnd_tninfo[immed_opnd] = NULL;
  actual_tninfo[index_opnd] = additive_index_tninfo;
  actual_tninfo[immed_opnd] = NULL;
#else
  Set_OP_opnd(op, index_opnd+1, new_tn);
  if (EBO_in_loop) {
    Set_OP_omega (op, index_opnd+1, 0);
  }
  opnd_tn[index_opnd] = additive_index_tn;
  opnd_tn[index_opnd+1] = new_tn;
  opnd_tninfo[index_opnd] = additive_index_tninfo;
  opnd_tninfo[index_opnd+1] = NULL;
  actual_tninfo[index_opnd] = additive_index_tninfo;
  actual_tninfo[index_opnd+1] = NULL;
#endif

  if (EBO_Trace_Optimization) {
    #pragma mips_frequency_hint NEVER
    fprintf(TFile, "  to produce the new expression:\n");
    Print_OP_No_SrcLine(op);
  }

}
#endif

#ifdef TARG_ST

/* =====================================================================
 *   EBO_delete_subset_mem_op
 * =====================================================================
 */
BOOL
EBO_delete_subset_mem_op(
  OP *op,
  EBO_TN_INFO **opnd_tninfo,
  EBO_OP_INFO *opinfo,
  INT64 offset_pred,
  INT64 offset_succ
)
{
  OP *pred_op = opinfo->in_op;
  BB *bb = OP_bb(op);
  INT opcount = OP_opnds(op);
  TN *pred_result = OP_store(pred_op) 
                      ? OP_opnd(pred_op, TOP_Find_Operand_Use(OP_code(pred_op),OU_storeval))
                      : OP_result(pred_op,0);
  TN *succ_result = OP_store(op)
                      ? OP_opnd(op, TOP_Find_Operand_Use(OP_code(op),OU_storeval))
                      : OP_result(op,0);
  INT size_pred;
  INT size_succ;
  BOOL succ_is_subset = FALSE;
  INT byte_offset;
  OPS ops = OPS_EMPTY;

  if (EBO_Trace_Data_Flow) {
    fprintf(TFile,"%ssubset    OP in BB:%d    ",EBO_trace_pfx,BB_id(bb));
    Print_OP_No_SrcLine(op);
    fprintf(TFile,"      Matches   OP in BB:%d    ",BB_id(opinfo->in_bb));
    Print_OP_No_SrcLine(pred_op);
  }
  
  if ((Opt_Level < 2) && (bb != opinfo->in_bb)) {
   /* Global TN's aren't supported at low levels of optimization. */
    return FALSE;
  }

  if (!EBO_in_peep &&
      (bb != opinfo->in_bb) &&
      !TN_Is_Constant(pred_result) &&
      has_assigned_reg(pred_result)) {
    if (EBO_Trace_Data_Flow) {
      #pragma mips_frequency_hint NEVER
      fprintf(TFile,"%sShouldn't move dedicated register references across blocks.\n",
              EBO_trace_pfx);
    }
    return FALSE;
  }

  size_pred = OP_Mem_Ref_Bytes(pred_op);
  size_succ = OP_Mem_Ref_Bytes(op);
  if (size_pred < size_succ) return FALSE;

  if ((offset_pred <= offset_succ) &&
      ((offset_pred + size_pred) >= (offset_succ + size_succ))) {
    succ_is_subset = TRUE;
  }

  if (!succ_is_subset) {
    return FALSE;
  }

  byte_offset = offset_succ - offset_pred;

  if (OP_load(op) && OP_load(pred_op) &&
      (OP_results(op) == 1) && (OP_results(pred_op) == 2) &&
      (2*size_succ == size_pred)) {
   /* Just use one of the 2 results for the predecessor. */
    pred_result = (byte_offset >= size_succ) ? OP_result(pred_op,1) : OP_result(pred_op,0);

    if (EBO_Trace_Optimization) {
      #pragma mips_frequency_hint NEVER
      fprintf(TFile,"%sRemove LoadPair - Load combination\n",EBO_trace_pfx);
    }

    EBO_Exp_COPY((OP_has_predicate(op)?OP_opnd(op,OP_PREDICATE_OPND):NULL),
                 succ_result, pred_result, &ops);

    if (EBO_in_loop) 
      EBO_OPS_omega (&ops, 
		     (OP_has_predicate(op)? OP_opnd(op,OP_PREDICATE_OPND):NULL),
		     (OP_has_predicate(op)? opnd_tninfo[OP_PREDICATE_OPND]:NULL));
    OP_srcpos(OPS_first(&ops)) = OP_srcpos(op);
    BB_Insert_Ops(OP_bb(op), op, &ops, FALSE);
    return TRUE;
  }

  if (!OP_store(pred_op) || !OP_load(op)) {
   /* Can only optimize Store - Load pattern. */
    return FALSE;
  }

  if (TOP_Find_Operand_Use(OP_code(op), OU_postincr) >= 0) {
   /* The increment must be preserved. */
    return FALSE;
  }

  return EBO_replace_subset_load_with_extract (op,
					       pred_result,
					       succ_result,
					       offset_pred,
					       offset_succ,
					       size_pred,
					       size_succ);
}

/* =====================================================================
 *   hoist_predicate_of_duplicate_complement
 * =====================================================================
 */
static BOOL
hoist_predicate_of_duplicate_complement (
  OP *op,
  EBO_TN_INFO **opnd_tninfo,
  EBO_OP_INFO *opinfo
)
{
  OP *pred_op = opinfo->in_op;
  TN *predicate1_tn = OP_opnd(pred_op,OP_PREDICATE_OPND);
  TN *predicate2_tn = OP_opnd(op,OP_PREDICATE_OPND);
  EBO_TN_INFO *predicate1_info = opinfo->actual_opnd[OP_PREDICATE_OPND];
  EBO_TN_INFO *predicate2_info = opnd_tninfo[OP_PREDICATE_OPND];
  BB *bb = OP_bb(op);
  INT i;
  INT val_idx;

  if (!OP_has_predicate(op) || !OP_has_predicate(pred_op))
    return FALSE;

  if (EBO_Trace_Data_Flow) {
    fprintf(TFile,"Enter hoist_predicate_of_duplicate_complement\n");
    Print_OP_No_SrcLine(pred_op);
    Print_OP_No_SrcLine(op);
    Print_TN(predicate1_tn,false);fprintf(TFile,"  ");
    Print_TN(predicate2_tn,false);fprintf(TFile,"\n");
    if (predicate1_info != NULL) tn_info_entry_dump(predicate1_info);
    if (predicate2_info != NULL) tn_info_entry_dump(predicate2_info);
  }

  if (OP_code(op) != OP_code(pred_op)) return FALSE;

 /* Pick up the new predicate that dominates both of those in the common expressions. */
  EBO_OP_INFO *new_predicate_opinfo = locate_opinfo_entry(predicate1_info);
  if (new_predicate_opinfo == NULL) return FALSE;

  EBO_TN_INFO *new_predicate_tninfo = new_predicate_opinfo->actual_opnd[OP_PREDICATE_OPND];
  TN *new_predicate_tn = OP_opnd(predicate1_info->in_op,OP_PREDICATE_OPND);

  if (!(TN_Is_Constant(new_predicate_tn) ||
        EBO_tn_available (bb, new_predicate_tninfo))) return FALSE;

  if (OP_store(op)) {
   /* For stores, we will replace both stores with a new one. */
   /* Note that the location of the store will be moved!      */
    if (OP_bb(op) != OP_bb(pred_op)) return FALSE;
    if (opinfo->op_must_not_be_removed) return FALSE;

    val_idx = TOP_Find_Operand_Use(OP_code(op),OU_storeval);
    EBO_TN_INFO *value_tninfo = opinfo->actual_opnd[val_idx];
   /* We will need to copy the value that is to be stored. */

   /* First, be sure that it will be available this point. */
    if ((value_tninfo == NULL) ||
        !EBO_tn_available (bb, value_tninfo)) return FALSE;
   /* Second, be sure that the target of the copy will be a valid result. */
    if (TN_is_const_reg(OP_opnd(op, val_idx))) return FALSE;
   /* Third, be sure that any results have not been used prior to the current OP. */
    if (OP_results(pred_op) != 0) {
      for (i = 0; i < OP_results(pred_op); i++) {
        EBO_TN_INFO *result_tninfo = opinfo->actual_rslt[i];
        if ((result_tninfo == NULL) ||
            (result_tninfo->reference_count != 0)) return FALSE;
      }
    }
   /* Fourth, the index that we need to use must also be generated
      with a predicate that dominates the new store. */
    INT base_idx = TOP_Find_Operand_Use(OP_code(op),OU_base);
    EBO_TN_INFO *input_info = opinfo->actual_opnd[base_idx];
    if ((input_info == NULL) ||
        (input_info->in_op == NULL) ||
        (input_info->local_tn == NULL) ||
        (input_info->predicate_tninfo != new_predicate_tninfo) ||
        OP_Defs_TN(pred_op, input_info->local_tn)) {
     /* The inputs to the hoisted expression must use the same predicate
        as the new (or modified) expression will. */
      if (EBO_Trace_Data_Flow) {
        fprintf(TFile,"predicates not hoisted because address not hoisted\n");
      }
      return FALSE;
    }
  } else if (OP_effectively_copy(op)) {
#if 0
    INT val_indx = EBO_Copy_Operand(op);
#else
    INT val_indx = OP_Copy_Operand(op);
#endif
    if ((val_indx <= 0) ||
        !TNs_Are_Equivalent(OP_opnd(op,val_indx), OP_opnd(pred_op,val_indx))) {
     /* The operands are different. We don't handle this and it may not be worthwhile. */
      if (EBO_Trace_Data_Flow) {
        fprintf(TFile,"copy operands are different\n");
      }
      return FALSE;
    }
  } else {
   /* For non-stores, we will use the results of the predecessor OP. */
    for (i = 0; i < OP_results(pred_op); i++) {
      if (opinfo->actual_rslt[i] == NULL) return FALSE;
      if (!EBO_tn_available (bb, opinfo->actual_rslt[i])) {
        if (EBO_Trace_Data_Flow) {
          fprintf(TFile,"previous result is not available\n");
        }
        return FALSE;
      }
    }
   /* If we are to leave the first OP in place but hoist it's predicate,
      the inputs to the OP must not be conditionally generated. */
    for (i = OP_PREDICATE_OPND+1; i < OP_opnds(pred_op); i++) {
      EBO_TN_INFO *input_info = opinfo->actual_opnd[i];
      if ((input_info == NULL) ||
          (input_info->in_op == NULL) ||
          (input_info->local_tn == NULL) ||
          (input_info->predicate_tninfo != new_predicate_tninfo) ||
          OP_Defs_TN(pred_op, input_info->local_tn)) {
       /* The inputs to the hoisted expression must use the same predicate
          as the new (or modified) expression will. */
        if (EBO_Trace_Data_Flow) {
          fprintf(TFile,"predicates not hoisted because inputs not hoisted\n");
        }
        return FALSE;
      }
    }
  }

  OPS ops = OPS_EMPTY;
  if (OP_store(op)) {
   /* Create copies of the different values that are to be stored. */
    TN *value_tn = OP_opnd(op, val_idx);
    if (!TNs_Are_Equivalent(OP_opnd(op, val_idx),OP_opnd(pred_op, val_idx))) {
     /* Need to merge the two values that are to be stored. */

     /* Note: it is probably not worthwhile making this transformation because
        it will only increase the tree height of the expressions. */
      if (has_assigned_reg(value_tn) || EBO_in_loop) {
        if (EBO_Trace_Data_Flow) {
          fprintf(TFile,"predicates not hoisted because inputs can not be merged\n");
        }
        return FALSE;
      }

      OPS ops1 = OPS_EMPTY;
      value_tn = Dup_TN(value_tn);

      EBO_Exp_COPY (predicate1_tn,
                    value_tn,
                    OP_opnd(pred_op, val_idx),
                    &ops1);
      OP_srcpos(OPS_last(&ops1)) = OP_srcpos(pred_op);
      if (EBO_in_loop) EBO_Set_OP_omega (OPS_last(&ops1), predicate1_info, opinfo->actual_opnd[val_idx]);
      OPS_Append_Ops( &ops, &ops1);

      OPS_Init(&ops1);
      EBO_Exp_COPY (predicate2_tn,
                    value_tn,
                    OP_opnd(op, val_idx),
                    &ops1);
      OP_srcpos(OPS_last(&ops1)) = OP_srcpos(op);
      if (EBO_in_loop) EBO_Set_OP_omega (OPS_last(&ops1), predicate2_info, opnd_tninfo[val_idx]);
      OPS_Append_Ops( &ops, &ops1);
    }

   /* Create and insert a new store OP. */
    OP *new_op = Dup_OP(op);
    Set_OP_opnd (new_op, OP_PREDICATE_OPND, new_predicate_tn);
    Set_OP_opnd (new_op, val_idx, value_tn);
    if (OP_memory(op)) {
      Copy_WN_For_Memory_OP (new_op, op);
    }
    OP_srcpos(new_op) = OP_srcpos(op);
    if (EBO_in_loop) {
      EBO_Copy_OP_omega( new_op, op);
      EBO_Set_Predicate_omega ( new_op, new_predicate_tninfo);
    }
    OPS_Append_Op( &ops, new_op);

    if (OP_results(op) != 0) {
      OPS ops1 = OPS_EMPTY;
      for (i = 0; i < OP_results(pred_op); i++) {
        EBO_Exp_COPY (predicate1_tn,
                      OP_result(pred_op, i),
                      OP_result(op, i),
                      &ops1);
        OP_srcpos(OPS_last(&ops1)) = OP_srcpos(pred_op);
        if (EBO_in_loop) EBO_Set_OP_omega (OPS_last(&ops1), predicate1_info, NULL);
      }
      OPS_Append_Ops( &ops, &ops1);
    }

   /* Remove the first OP. */
    remove_op (opinfo);
    OP_Change_To_Noop(opinfo->in_op);
    opinfo->in_op = NULL;
    opinfo->in_bb = NULL;
  } else {
   /* Replace the predicate of the previous op. */
    Set_OP_opnd (pred_op, OP_PREDICATE_OPND, new_predicate_tn);
    opinfo->actual_opnd[OP_PREDICATE_OPND] = new_predicate_tninfo;
    for (i = 0; i < OP_results(pred_op); i++) {
      if (opinfo->actual_rslt[i] != NULL) {
        if (opinfo->actual_rslt[i]->predicate_tninfo != NULL) {
          dec_ref_count(opinfo->actual_rslt[i]->predicate_tninfo);
        }
        opinfo->actual_rslt[i]->predicate_tninfo = new_predicate_tninfo;
        if (new_predicate_tninfo != NULL) {
          inc_ref_count(new_predicate_tninfo);
        }
      }
    }

    if (OP_effectively_copy(op)) {
     /* Leave the second OP the way it was. */
if (EBO_Trace_Optimization) fprintf(TFile,"hoist predicate for complementary copy\n");
      return FALSE;
    }

   /* Create copies to the different outputs. */
    OPS ops1 = OPS_EMPTY;
    for (i = 0; i < OP_results(op); i++) {
      if (!TNs_Are_Equivalent(OP_result(op, i),OP_result(pred_op, i))) {
        EBO_Exp_COPY (predicate2_tn,
                      OP_result(op, i),
                      OP_result(pred_op, i),
                      &ops1);
        OP_srcpos(OPS_last(&ops1)) = OP_srcpos(op);
        if (EBO_in_loop) EBO_Set_OP_omega (OPS_last(&ops1), predicate2_info, NULL);
      }
    }
    OPS_Append_Ops( &ops, &ops1);
  }

  BB_Insert_Ops(bb, op, &ops, FALSE);
if (EBO_Trace_Optimization) fprintf(TFile,"hoist predicate for complementary operations\n");

  return TRUE;
}

/* =====================================================================
 *   EBO_delete_reload_across_dependency
 *
 *   For a given load or store and one it matches, attempt to replace 
 *   one of them.
 *   Return TRUE if this op is no longer needed.
 * =====================================================================
 */
BOOL
EBO_delete_reload_across_dependency (
  OP *op,
  EBO_TN_INFO **opnd_tninfo,
  EBO_OP_INFO *opinfo,
  EBO_OP_INFO *intervening_opinfo
)
{
  BB *bb = OP_bb(op);
  OP *pred_op = opinfo->in_op;
  OP *intervening_op = intervening_opinfo->in_op;
  TOP opcode = OP_code(op);
  TOP pred_opcode = OP_code(pred_op);
  TOP intervening_opcode = OP_code(intervening_op);

  INT size_pred;
  INT size_succ;
  INT size_intervening;

  TN *pred_result;
  TN *intervening_result;

  /* 
   * We can't assign registers, so don't optimize if it's 
   * already been done. 
   */
  if (EBO_in_loop) return FALSE;
  if (EBO_in_peep) return FALSE;

  if (EBO_Trace_Execution) {
    fprintf(TFile,"%sEnter delete_reload_across_dependency.\n",EBO_trace_pfx);
    Print_OP_No_SrcLine(pred_op);
    Print_OP_No_SrcLine(intervening_op);
    Print_OP_No_SrcLine(op);
  }

 /* Be sure we have a "Store .. Store .. Load" pattern. */
  if ((pred_op == NULL) ||
      (intervening_op == NULL) ||
      !OP_load(op) ||
      !(OP_load(pred_op) || OP_store(pred_op)) ||
      !OP_store(intervening_op)) return FALSE;

  pred_op = opinfo->in_op;
  intervening_op = intervening_opinfo->in_op;

  if (OP_prefetch(op) ||
      OP_prefetch(pred_op) ||
      OP_prefetch(intervening_op)) return FALSE;

  if (OP_unalign_mem(op) ||
      OP_unalign_mem(pred_op) ||
      OP_unalign_mem(intervening_op)) return FALSE;


  INT pred_base_idx = OP_find_opnd_use(pred_op, OU_base);
  INT intervening_base_idx =  OP_find_opnd_use(intervening_op, OU_base);
  INT pred_offset_idx = OP_find_opnd_use(pred_op, OU_offset);
  INT intervening_offset_idx = OP_find_opnd_use(intervening_op, OU_offset);
  INT pred_inc_idx = OP_find_opnd_use(pred_op, OU_postincr);
  INT intervening_inc_idx = OP_find_opnd_use(intervening_op, OU_postincr);

  TN *pred_base = OP_opnd(pred_op, pred_base_idx);
  TN *intervening_base = OP_opnd(intervening_op, intervening_base_idx);
  TN *pred_offset = OP_opnd(pred_op, pred_offset_idx);
  TN *intervening_offset = OP_opnd(intervening_op, intervening_offset_idx);

  size_succ = OP_Mem_Ref_Bytes(op);
  size_pred = OP_Mem_Ref_Bytes(pred_op);
  size_intervening = OP_Mem_Ref_Bytes(intervening_op);

  if ((size_succ != size_pred) ||
      (size_succ != size_intervening)) return FALSE;

  /* [CG] if size of the reloaded TN is greater. We return. We may generate
     a select with extract however. */
  if (TN_size(OP_result(op, 0)) != size_succ) {
    if (EBO_Trace_Data_Flow) {
      fprintf(TFile,"%sSize mismatch for Store - Store - Load combination: load size %d for tn size %d\n",
	      EBO_trace_pfx,size_succ,TN_size(OP_result(op, 0)));
    }
    return FALSE;
  }

  if (TOP_Find_Operand_Use(OP_code(op), OU_postincr) >= 0) {
   /* The increment must be preserved. */
    if (EBO_Trace_Data_Flow) {
      #pragma mips_frequency_hint NEVER
      fprintf(TFile,"%sIncremented Load for intervening store combination\n",
              EBO_trace_pfx);
    }
    return FALSE;
  }

  /* Paired loads are not supported. */
  // This is a IA64 stuff -- leave for now ...
  if (OP_results(op) == 2) return FALSE;
  if ((OP_store(pred_op) &&
      ((OP_results(pred_op) > 1) ||
       ((OP_results(pred_op) > 0) &&
        (pred_inc_idx < 0)))) ||
      (OP_store(intervening_op) &&
      ((OP_results(intervening_op) > 1) ||
       ((OP_results(intervening_op) > 0) &&
        (intervening_inc_idx < 0))))) return FALSE;
  if ((OP_load(pred_op) &&
      ((OP_results(pred_op) > 2) ||
       ((OP_results(pred_op) > 1) &&
        (pred_inc_idx < 0)))) ||
      (OP_load(intervening_op) &&
      ((OP_results(intervening_op) > 2) ||
       ((OP_results(intervening_op) > 1) &&
        (intervening_inc_idx < 0))))) return FALSE;

  /* Capture the values in the preceeding memory OPs. */
  pred_result = OP_store(pred_op) ? 
    OP_opnd(pred_op, TOP_Find_Operand_Use(pred_opcode, OU_storeval)) :
    OP_result(pred_op,0);
  intervening_result = OP_opnd(intervening_op,
    TOP_Find_Operand_Use(intervening_opcode, OU_storeval));

  if ((TN_register_class(intervening_result) != TN_register_class(pred_result)) ||
      (TN_register_class(intervening_result) != TN_register_class(OP_result(op,0)))) {
    if (EBO_Trace_Data_Flow) {
      fprintf(TFile,"%sInter-register copies are not supported\n",
              EBO_trace_pfx);
    }
    return FALSE;
  }

  if (TNs_Are_Equivalent(pred_result, intervening_result)) {
    /* 
     * It doesn't matter if the addresses are the same or different
     * because the value will always be the same! Just Copy the value. 
     */
    if (EBO_copy_value(op, pred_result, intervening_result, size_succ)) {

      if (EBO_Trace_Optimization) {
	fprintf(TFile,"%sReload across intervening store is not needed.\n",EBO_trace_pfx);
	Print_OP_No_SrcLine(pred_op);
	Print_OP_No_SrcLine(intervening_op);
	Print_OP_No_SrcLine(op);
      }

      return TRUE;
    }
  }

  /* 
   * We need to compare the addresses BEFORE they were incremented.
   * If both OPs are incremented by the same value, we can compare
   * the addresses AFTER the increment.
   */
  if ((pred_inc_idx >= 0) || (intervening_inc_idx >= 0)) {
    if ((pred_inc_idx < 0) || (intervening_inc_idx < 0)) {
      if (EBO_Trace_Data_Flow) {
        fprintf(TFile,"%sBase address not available for compare because of increment\n",
                EBO_trace_pfx);
      }
      return FALSE;
    }

    TN *pred_inc_tn = OP_opnd(pred_op,pred_inc_idx);
    TN *intervening_inc_tn = OP_opnd(intervening_op,intervening_inc_idx);

    if ((TN_Is_Constant(pred_inc_tn) ||
         TN_Is_Constant(intervening_inc_tn)) &&
        (pred_inc_tn != intervening_inc_tn)) {
      if (EBO_Trace_Data_Flow) {
        fprintf(TFile,"%sBase address not available for compare because of different constant increment\n",
                EBO_trace_pfx);
      }
      return FALSE;
    }

    if (opinfo->actual_opnd[pred_inc_idx] != intervening_opinfo->actual_opnd[intervening_inc_idx]) {
      if (EBO_Trace_Data_Flow) {
        fprintf(TFile,"%sBase address not available for compare because of different increment\n",
                EBO_trace_pfx);
      }
      return FALSE;
    }
  }

  /* Are the bases available from each store? */
  if (!EBO_tn_available (bb, opinfo->actual_opnd[pred_base_idx]) ||
      !EBO_tn_available (bb, intervening_opinfo->actual_opnd[intervening_base_idx])) {
    if (EBO_Trace_Data_Flow) {
      fprintf(TFile,"%sBase address not available for compare\n",
              EBO_trace_pfx);
    }
    return FALSE;
  }

  return EBO_select_value(op, 
			  pred_result, 
			  pred_base,
			  pred_offset,
			  intervening_result, 
			  intervening_base,
			  intervening_offset,
			  size_succ);

  return TRUE;
}

/* =====================================================================
 *   delete_memory_op
 *
 *   For a given load or store and one it matches, attempt to replace 
 *   one of them. Return TRUE if this op is no longer needed.
 * =====================================================================
 */
static BOOL
delete_memory_op (
  OP *op,
  EBO_TN_INFO **opnd_tninfo,
  EBO_OP_INFO *opinfo
)
{
  INT i;
  OPS ops = OPS_EMPTY;
  TOP opcode = OP_code(op);

  INT size_pred;
  INT size_succ;
  TN *predicate_tn = OP_has_predicate(op)?OP_opnd(op,OP_PREDICATE_OPND):NULL;

  if (OP_prefetch(op)) {
    /* 
     * Remove the second OP for:
     *    Prefetch - Prefetch,
     *    Load - Prefetch,
     *    Store - Prefetch
     */
    if (EBO_Trace_Optimization) {
      fprintf(TFile,"%sRemove following Prefetch combination\n",EBO_trace_pfx);
    }
    return TRUE;
  }

  if (OP_prefetch(opinfo->in_op)) {
   /* 
    * Don't optimize:
    *     Prefetch - Load,
    *     Prefetch - Store,
    */
    return FALSE;
  }

  if (OP_unalign_mem(op) || OP_unalign_mem(opinfo->in_op)) return FALSE;

  size_pred = OP_Mem_Ref_Bytes(opinfo->in_op);
  size_succ = OP_Mem_Ref_Bytes(op);

  if (OP_load(op) && OP_load(opinfo->in_op)) {
   /* 
    * Replace the result tn of the second OP for:
    *     Load - Load,
    */
    if (TOP_Find_Operand_Use(opcode, OU_postincr) >= 0) {
      /* The increment must be preserved. */
      if (EBO_Trace_Data_Flow) {
        fprintf(TFile,"%sIncremented Load for Load - Load combination\n",
                EBO_trace_pfx);
      }
      return FALSE;
    }

    if ((TN_register_class(OP_result(op,0)) !=
                 TN_register_class(OP_result(opinfo->in_op, 0))) ||
        (TN_is_fpu_int(OP_result(op,0)) !=
                 TN_is_fpu_int(OP_result(opinfo->in_op, 0)))) {
      if (EBO_Trace_Data_Flow) {
        fprintf(TFile,"%sType mismatch for Load - Load combination\n",
                EBO_trace_pfx);
      }
      return FALSE;
    }

    if ((size_pred != size_succ) ||
        (OP_results(op) != OP_results(opinfo->in_op)) ||
        (TN_size(OP_result(opinfo->in_op, 0)) != TN_size(OP_result(op, 0)))) {
      /* Size of the data item loaded by the two loads is different,
	 but the starting memory address is the same.  There is a chance
	 that the predecessor load is a load-pair and that the new load
	 matches one of the words that is loaded. */

      if (EBO_Trace_Data_Flow) {
        #pragma mips_frequency_hint NEVER
        fprintf(TFile,"%sSize mismatch for Load - Load combination: %d:%d %d:%d \n",
                EBO_trace_pfx,size_pred,size_succ,
                TN_size(OP_result(opinfo->in_op, 0)),TN_size(OP_result(op, 0)));
      }

      return EBO_delete_subset_mem_op (op, opnd_tninfo, opinfo, 0, 0);
    }

    if (!EBO_in_peep &&
        (OP_bb(op) != OP_bb(opinfo->in_op)) &&
        !TN_Is_Constant(OP_result(opinfo->in_op, 0)) &&
        has_assigned_reg(OP_result(opinfo->in_op, 0))) {
      if (EBO_Trace_Data_Flow) {
        fprintf(TFile,"%sShouldn't move dedicated register references across blocks.\n",
                EBO_trace_pfx);
      }
      return FALSE;
    }

    if (EBO_Trace_Optimization) {
      fprintf(TFile,"%sRemove Load - Load combination\n",EBO_trace_pfx);
    }

    for (i = 0; i < OP_results(op); i++) {
      EBO_Exp_COPY(predicate_tn, OP_result(op, i), OP_result(opinfo->in_op, i), &ops);
    }

    if (EBO_in_loop) 
      EBO_OPS_omega (&ops, 
		     (OP_has_predicate(op)? OP_opnd(op,OP_PREDICATE_OPND):NULL),
		     (OP_has_predicate(op)? opnd_tninfo[OP_PREDICATE_OPND]:NULL));

    OP_srcpos(OPS_first(&ops)) = OP_srcpos(op);
    BB_Insert_Ops(OP_bb(op), op, &ops, FALSE);
    return TRUE;
  } 

  else if (OP_load(op) && OP_store(opinfo->in_op)) {
   /* 
    * Replace the result tn of the second OP for:
    *     Store - Load
    */
    INT storeval_idx = 
           TOP_Find_Operand_Use(OP_code(opinfo->in_op),OU_storeval);
    TN *storeval_tn = OP_opnd(opinfo->in_op, storeval_idx);

    if (TOP_Find_Operand_Use(OP_code(op), OU_postincr) >= 0) {
      /* The increment must be preserved. */
      if (EBO_Trace_Data_Flow) {
        fprintf(TFile,"%sIncremented Load for Load - Store combination\n",
                EBO_trace_pfx);
      }
      return FALSE;
    }

    if ((TN_register_class(OP_result(op,0)) !=
                 TN_register_class(storeval_tn)) ||
        (TN_is_fpu_int(OP_result(op,0)) !=
                 TN_is_fpu_int(storeval_tn))) {
      if (EBO_Trace_Data_Flow) {
        fprintf(TFile,"%sType mismatch for Store - Load combination\n",
                EBO_trace_pfx);
      }
      return FALSE;
    }

    if (!EBO_in_peep &&
        (OP_bb(op) != OP_bb(opinfo->in_op)) &&
        !TN_Is_Constant(storeval_tn) &&
        has_assigned_reg(storeval_tn)) {
      if (EBO_Trace_Data_Flow) {
        fprintf(TFile,"%sShouldn't move dedicated register references across blocks.\n",
                EBO_trace_pfx);
      }
      return FALSE;
    }

    if (size_pred == size_succ) {
      /* Size of source and target are the same. */
      if (TN_size(storeval_tn) > size_succ) {
       /* 
	* Although the size of the data moved to and from memory is
	* the same, the size of the generated value is larger than
	* the size of the value we want to load.  Call another
	* routine to mask off the upper portion of the stored value. 
	*/

        if (EBO_Trace_Data_Flow) {
          fprintf(TFile,"%sSize mismatch for Store - Load combination: %d %d %d\n",
                  EBO_trace_pfx,size_pred,TN_size(storeval_tn),size_succ);
        }
        return EBO_delete_subset_mem_op (op, opnd_tninfo, opinfo, 0, 0);
      }

      if (EBO_Trace_Optimization) {
        fprintf(TFile,"%sRemove Store - Load combination\n",EBO_trace_pfx);
      }

      EBO_Exp_COPY(predicate_tn, OP_result(op, 0), storeval_tn, &ops);

      if (EBO_in_loop) 
	EBO_Set_OP_omega (OPS_first(&ops),
			  (OP_has_predicate(op)?opnd_tninfo[OP_PREDICATE_OPND]:NULL),
			  opinfo->actual_opnd[storeval_idx]);

      OP_srcpos(OPS_first(&ops)) = OP_srcpos(op);
      BB_Insert_Ops(OP_bb(op), op, &ops, FALSE);
      return TRUE;
    } 
    else {
     /* 
      * Size of source and target are different, but the starting 
      * memory address is the same. Call another routine to do the 
      * work. 
      */
      return EBO_delete_subset_mem_op (op, opnd_tninfo, opinfo, 0, 0);
    }
  } 

  else if (OP_store(op) && 
	   OP_store(opinfo->in_op) &&
	   (OP_bb(op) == OP_bb(opinfo->in_op))) {
   /* 
    * Remove the first OP for:
    *     Store - Store
    */
    if (size_pred != size_succ) return FALSE;
    if (opinfo->op_must_not_be_removed) return FALSE;
    if (TOP_Find_Operand_Use(OP_code(opinfo->in_op), OU_postincr) >= 0) {
      /* The increment must be preserved. */
      if (EBO_Trace_Data_Flow) {
        fprintf(TFile,"%sIncremented Store for Store - Store combination\n",
                EBO_trace_pfx);
      }
      return FALSE;
    }

    if (EBO_Trace_Optimization) {
      fprintf(TFile,"%sRemove Store - Store combination\n",EBO_trace_pfx);
    }

    remove_op (opinfo);
    OP_Change_To_Noop(opinfo->in_op);
    opinfo->in_op = NULL;
    opinfo->in_bb = NULL;
    return FALSE;
  } 

  else if (OP_load(opinfo->in_op) && OP_store(op)) {
   /* 
    * The store may not be needed if the same value is put back for:
    *     Load - Store
    */
    INT storeval_idx = TOP_Find_Operand_Use(OP_code(op),OU_storeval);
    TN *storeval_tn = OP_opnd(op, storeval_idx);

    if (TOP_Find_Operand_Use(OP_code(op), OU_postincr) >= 0) {
      /* The increment must be preserved. */
      if (EBO_Trace_Data_Flow) {
        fprintf(TFile,"%sIncremented Store for Load - Store combination\n",
                EBO_trace_pfx);
      }
      return FALSE;
    }

    if ((TN_register_class(OP_result(opinfo->in_op,0)) !=
                 TN_register_class(storeval_tn)) ||
        (TN_is_fpu_int(OP_result(opinfo->in_op,0)) !=
                 TN_is_fpu_int(storeval_tn))) {
      if (EBO_Trace_Data_Flow) {
        fprintf(TFile,"%sType mismatch for Load - Store combination\n",
                EBO_trace_pfx);
      }
      return FALSE;
    }

    if (!EBO_in_peep &&
        (OP_bb(op) != OP_bb(opinfo->in_op)) &&
        !TN_Is_Constant(storeval_tn) &&
        has_assigned_reg(storeval_tn)) {
      if (EBO_Trace_Data_Flow) {
        fprintf(TFile,"%sShouldn't move dedicated register references across blocks.\n",
                EBO_trace_pfx);
      }
      return FALSE;
    }

    if ((size_pred != size_succ) || (TN_size(storeval_tn) != size_succ)) {

      if (EBO_Trace_Data_Flow) {
        fprintf(TFile,"%sSize mismatch for Load - Store combination: %d %d %d\n",
                EBO_trace_pfx,size_pred,TN_size(storeval_tn),size_succ);
      }

      return FALSE;
    }

    if ((opnd_tninfo[storeval_idx] != NULL) &&
        (opnd_tninfo[storeval_idx]->in_op != NULL) &&
        (opinfo->in_op != NULL) &&
        (opnd_tninfo[storeval_idx]->in_op == opinfo->in_op)) {
     /* The Store is not needed! */

      if (EBO_Trace_Optimization) {
        fprintf(TFile,"%sRemove Load - Store combination\n",EBO_trace_pfx);
      }

      return TRUE;
    }
  }

  return FALSE;
}

/* =====================================================================
 *   EBO_delete_duplicate_op
 *
 *   For a given op and one it matches, attempt to replace one of them.
 *   Return TRUE if this op is no longer needed.
 * =====================================================================
 */
BOOL
EBO_delete_duplicate_op (
  OP *op,
  EBO_TN_INFO **opnd_tninfo,
  EBO_OP_INFO *opinfo
)
{
  INT resnum;
  TOP opcode = OP_code(op);
  OPS ops = OPS_EMPTY;

  if (EBO_Trace_Data_Flow) {
    fprintf(TFile,"%sDuplicate OP in BB:%d    ",EBO_trace_pfx,BB_id(OP_bb(op)));
    Print_OP_No_SrcLine(op);
    fprintf(TFile,"      Matches   OP in BB:%d    ",BB_id(opinfo->in_bb));
    Print_OP_No_SrcLine(opinfo->in_op);
  }

  if ((Opt_Level < 2) && (OP_bb(op) != opinfo->in_bb)) {
    /* Global TN's aren't supported at low levels of optimization. */
    return FALSE;
  }

  if (OP_has_predicate(op) &&
      (OP_opnd(op,OP_PREDICATE_OPND) != OP_opnd(opinfo->in_op,OP_PREDICATE_OPND))) {
    OP *pred_op = opinfo->in_op;
    TN *predicate1_tn = OP_opnd(pred_op,OP_PREDICATE_OPND);
    TN *predicate2_tn = OP_opnd(op,OP_PREDICATE_OPND);
    EBO_TN_INFO *predicate1_info = opinfo->actual_opnd[OP_PREDICATE_OPND];
    EBO_TN_INFO *predicate2_info = opnd_tninfo[OP_PREDICATE_OPND];

    if ((OP_code(op) == OP_code(pred_op)) &&
        EBO_predicate_complements(predicate1_tn, predicate1_info,
                                  predicate2_tn, predicate2_info)) {
      return hoist_predicate_of_duplicate_complement (op, opnd_tninfo, opinfo);
    }
    if (OP_store(pred_op) && OP_store(op)) {
      if (!EBO_predicate_dominates(predicate2_tn, predicate2_info,
                                   predicate1_tn, predicate1_info)) {
        if (EBO_Trace_Data_Flow) {
          fprintf(TFile,"%sStores can not be combined because predicates do not match\n",EBO_trace_pfx);
        }
        return FALSE;
      }
    } else {
      if (!EBO_predicate_dominates(predicate1_tn, predicate1_info,
                                   predicate2_tn, predicate2_info)) {
        if (EBO_Trace_Data_Flow) {
          fprintf(TFile,"%sExpressions can not be combined because predicates do not match\n",EBO_trace_pfx);
        }
        return FALSE;
      }
    }
  }

  if (OP_memory(op)) {
   /* 
    * Separate load/store processing, but logically it's just a 
    * special case. 
    */
    return delete_memory_op (op, opnd_tninfo, opinfo);
  } 

#ifdef TARG_ST
  // [CG]: skip special compare processing for now
#else
  else if (OP_icmp(op) || OP_fcmp(op)) {
   /* Integer and float comparison operations. */
    if (EBO_condition_redundant (op, opnd_tninfo, opinfo, &ops)) {
      if (OPS_length(&ops) != 0) {
	if (EBO_in_loop) EBO_OPS_omega (&ops, NULL,NULL);
        OP *next_op = OPS_first(&ops);
        while (next_op != NULL) {
          OP_srcpos(next_op) = OP_srcpos(op);
          next_op = OP_next(next_op);
        }
	BB_Insert_Ops(OP_bb(op), op, &ops, FALSE);
      }
      return TRUE;
    }
    else {
      if (EBO_Trace_Optimization) {
#pragma mips_frequency_hint NEVER
	fprintf(TFile,"%sCan not remove compare operation.\n",EBO_trace_pfx);
      }
      return FALSE;
    }
  } /* compare OP */
#endif

  else {

#if 0
    // [CG] Commented out.
    if (OP_has_predicate(op) &&
        (OP_opnd(op,OP_PREDICATE_OPND) != True_TN)) {
      /* 
       * Check for required copies of predicates that can not 
       * be supported. 
       */
      for (resnum = 0; resnum < OP_results(op); resnum++) {
        if (TN_register_class(OP_result(op, resnum)) == ISA_REGISTER_CLASS_predicate) {
          if (EBO_Trace_Data_Flow) {
            fprintf(TFile,"%sStores can not be combined because predicate copies can not be predicated\n",
                   EBO_trace_pfx);
          }
          return FALSE;
        }
      }
    }
#endif

    /* Create copies of the result TN's. */
    for (resnum = 0; resnum < OP_results(op); resnum++) {
      TN *rslt = OP_result(op, resnum);
      TN *src1 = OP_result(opinfo->in_op, resnum);
      
      EBO_Exp_COPY(NULL, rslt, src1, &ops);
#ifdef TARG_ST
      if (EBO_in_peep && OP_next(OPS_first(&ops)) != NULL) {
	/* If the copy needs multiple instructions, we 
	   may have generated non dedicated tns while register allocation is already done. */
	return FALSE;
      }
#endif

      OP_srcpos(OPS_last(&ops)) = OP_srcpos(op);
    }

    if (EBO_in_loop) 
      EBO_OPS_omega (&ops, NULL, NULL);

    BB_Insert_Ops(OP_bb(op), op, &ops, FALSE);
    return TRUE;
  }

  return FALSE;
}

#ifdef TARG_ST
/* =====================================================================
 *    EBO_Memory_Sequence
 *
 *    Look for sequence such as add/(load|store).
 * =====================================================================
 */
BOOL
EBO_Memory_Sequence (
  OP *op,
  TN **opnd_tn,
  EBO_TN_INFO **opnd_tninfo
)
{
  BB *bb = OP_bb(op);
  INT opndnum = OP_opnds(op);
  EBO_TN_INFO *tninfo;

  TOP opcode = OP_code(op);
  INT l0_base_idx;
  INT l0_offset_idx;
  TN *tn_base;
  TN *tn_offset;
  INT64 offset_val;
  ST *offset_sym = NULL;
  INT32 offset_relocs;
  

  OP *pred_op;
  TOP pred_opcode;
  EBO_OP_INFO *pred_opinfo;

  if (!OP_load(op) && !OP_store(op) && !OP_prefetch(op)) return FALSE;

  if (EBO_Trace_Execution) {
    INT i;
    fprintf(TFile, "%sin BB:%d load|store sequence OP :- %s ",
            EBO_trace_pfx, BB_id(bb),TOP_Name(OP_code(op)));
    for (i = 0; i < opndnum; i++) {
      fprintf(TFile," ");
      Print_TN(opnd_tn[i],FALSE);
    }
    fprintf(TFile,"\n");
  }

  l0_base_idx = OP_find_opnd_use(op, OU_base);
  l0_offset_idx = OP_find_opnd_use(op, OU_offset);

  if (l0_base_idx < 0 || l0_offset_idx < 0) return FALSE;

  tn_base = opnd_tn[l0_base_idx];
  tn_offset = opnd_tn[l0_offset_idx];

  if (TOP_opnd_use_bits(opcode, l0_base_idx) != TOP_opnd_use_bits(opcode, l0_offset_idx)) return FALSE;

  if (TN_Has_Value(tn_offset) || TN_is_symbol(tn_offset)) {
    offset_val = TN_is_symbol(tn_offset)? TN_offset(tn_offset): TN_Value(tn_offset);
    offset_val = sext(offset_val, TOP_opnd_use_bits(opcode, l0_offset_idx));
    offset_sym = TN_is_symbol(tn_offset)? TN_var(tn_offset): NULL;
    offset_relocs = TN_is_symbol(tn_offset)? TN_relocs(tn_offset): 0;
  } else return FALSE;
  
  tninfo = opnd_tninfo[l0_base_idx];
  if (tninfo == NULL) return FALSE;
  pred_op = tninfo->in_op;
  if (pred_op == NULL) return FALSE;
  pred_opcode = OP_code(pred_op);
  pred_opinfo = locate_opinfo_entry (tninfo);
  if (pred_opinfo == NULL) return FALSE;

  if (!OP_iadd(pred_op) && !OP_isub(pred_op)) return FALSE;
  if (pred_opcode == TOP_spadjust) return FALSE;

  INT ptn0_idx = TOP_Find_Operand_Use(pred_opcode,OU_opnd1);
  INT ptn1_idx = TOP_Find_Operand_Use(pred_opcode,OU_opnd2);
  FmtAssert(ptn0_idx >= 0 && ptn1_idx >= 0, ("add/sub opcode %shas no operand properties", TOP_Name(pred_opcode)));

  TN *ptn0 = OP_opnd(pred_op, ptn0_idx);
  TN *ptn1 = OP_opnd(pred_op, ptn1_idx);
  EBO_TN_INFO *ptn0_tninfo;
  INT64 pred_val;
  ST *pred_sym = NULL;
  INT32 pred_relocs;

  if (TOP_opnd_use_bits(pred_opcode, ptn1_idx) != TOP_opnd_use_bits(pred_opcode, ptn0_idx)) return FALSE;
  if (TOP_opnd_use_bits(pred_opcode, ptn1_idx) != TOP_opnd_use_bits(opcode, l0_offset_idx)) return FALSE;
      
  if (TN_Has_Value(ptn1) || TN_is_symbol(ptn1)) {
    pred_val = TN_is_symbol(ptn1)? TN_offset(ptn1): TN_Value(ptn1);
    pred_val = sext(pred_val, TOP_opnd_use_bits(pred_opcode, ptn1_idx));
    pred_sym = TN_is_symbol(ptn1)? TN_var(ptn1): NULL;
    pred_relocs = TN_is_symbol(ptn1)? TN_relocs(ptn1): 0;
    ptn0_tninfo = pred_opinfo->actual_opnd[ptn0_idx];
  } else if (OP_iadd(pred_op) && (TN_Has_Value(ptn0) || TN_is_symbol(ptn0))) {
    pred_val = TN_is_symbol(ptn0)? TN_offset(ptn0): TN_Value(ptn0);
    pred_val = sext(pred_val, TOP_opnd_use_bits(pred_opcode, ptn0_idx));
    pred_sym = TN_is_symbol(ptn0)? TN_var(ptn0): NULL;
    pred_relocs = TN_is_symbol(ptn0)? TN_relocs(ptn0): 0;
    ptn0 = ptn1;
    ptn0_tninfo = pred_opinfo->actual_opnd[ptn1_idx];
  } else return FALSE;
  
  // [CG]: It seems that we can't modify symbol offsets
  // as Base_Symbol_And_Offset_For_Addressing will change
  if (pred_sym != NULL || offset_sym != NULL) return FALSE;

  if (!EBO_tn_available (bb, ptn0_tninfo)) return FALSE;

  TOP new_opcode;
  OP *new_op;
  UINT64 new_offset_val;
  ST *new_sym = NULL;
  INT32 new_relocs;
  TN *new_offset_tn;

  if (pred_sym != NULL && offset_sym != NULL) return FALSE;
  if (offset_sym != NULL) {
    new_sym = offset_sym;
    new_relocs = offset_relocs;
  }
  if (pred_sym != NULL) {
    new_sym = pred_sym;
    new_relocs = pred_relocs;
  }
  if (OP_iadd(pred_op)) 
    new_offset_val = offset_val + pred_val;
  else if (OP_isub(pred_op) && pred_sym == NULL)
    new_offset_val = offset_val - pred_val;
  else return FALSE;

  new_offset_val = sext(new_offset_val, TN_size(OP_result(pred_op, 0))*8);
  
  if (new_sym != NULL) {
    new_offset_tn = Gen_Symbol_TN(new_sym, new_offset_val, new_relocs);
    new_opcode = TOP_opnd_immediate_variant(opcode, l0_offset_idx, 0xFFFFFFFF);
  } else {
    new_offset_tn = Gen_Literal_TN(new_offset_val, TN_size(ptn0));
    new_opcode = TOP_opnd_immediate_variant(opcode, l0_offset_idx, new_offset_val);
  }

  
  if (new_opcode != TOP_UNDEFINED) {
    new_op = Dup_OP(op);
    OP_code(new_op) = new_opcode;
    OP_srcpos(new_op) = OP_srcpos(op);
    Set_OP_opnd (new_op, l0_base_idx, ptn0);
    Set_OP_opnd (new_op, l0_offset_idx, new_offset_tn);
    if (OP_memory(op)) Copy_WN_For_Memory_OP (new_op, op);
    if (EBO_in_loop) {
      EBO_OP_omega(new_op, ptn0, ptn0_tninfo);
    }
    BB_Insert_Op_After(bb, op, new_op);
    if (EBO_Trace_Optimization) fprintf(TFile,"combined add/load|store sequence\n");
    return TRUE;
  }
  return FALSE;
}
#endif

#ifdef TARG_ST
/* =====================================================================
 * OPs_Are_Equivalent
 *
 * Returns true is 2 ops are equivalent.
 * If true, it is useless to replace one by hte other
 * for instance.
 * 2 ops are equivalent if the intruction is the same
 * and arguments are the same that is:
 * 1. opcode are the same
 * 2. all non register operands are the same
 * 3. all register operands are the same, or if allocated
 *    allocated registers are the same.
 * Note that flags attached to ops are not tested.
 * =====================================================================
 */
static BOOL 
OPs_Are_Equivalent(OP *op1, OP *op2)
{
  if (OP_code(op1) != OP_code(op2)) return FALSE;
  for (int i = 0; i < OP_opnds(op1); i++) {
    if (TN_is_register(OP_opnd(op1, i)) &&
	TN_is_register(OP_opnd(op2, i)) &&
	!TNs_Are_Equivalent(OP_opnd(op1, i), OP_opnd(op2, i)))
      return FALSE;
    else if (OP_opnd(op1, i) != OP_opnd(op2, i)) return FALSE;
  }
  return TRUE;
}
#endif

/* =====================================================================
 *    EBO_Constant_Operand0
 *
 *    Look at an exression that has a constant first operand and attempt
 *    to simplify the computations.
 * =====================================================================
 */
BOOL
EBO_Constant_Operand0 (
  OP *op,
  TN **opnd_tn,
  EBO_TN_INFO **opnd_tninfo
)
{
  BB *bb = OP_bb(op);
  INT opndnum = OP_opnds(op);
  EBO_TN_INFO *tninfo;

  TOP opcode = OP_code(op);
  TN *tnr = OP_result(op,0);
  INT l0_opnd1_idx;
  INT l0_opnd2_idx;
  TN *tn0;
  TN *tn1;
  INT64 const_val;

  OP *pred_op;
  TOP pred_opcode;
  EBO_OP_INFO *pred_opinfo;

  if (EBO_Trace_Execution) {
    INT i;
    fprintf(TFile, "%sin BB:%d special0 OP :- %s",
            EBO_trace_pfx, BB_id(OP_bb(op)),TOP_Name(OP_code(op)));
    for (i = 0; i < opndnum; i++) {
      fprintf(TFile," ");
      Print_TN(opnd_tn[i],FALSE);
    }
    fprintf(TFile,"\n");
  }

  // Arthur: spadjust shouldn't be subject to this
  if (OP_code(op) == TOP_spadjust) return FALSE;

  l0_opnd1_idx = OP_find_opnd_use(op, OU_opnd1);
  l0_opnd2_idx = OP_find_opnd_use(op, OU_opnd2);

  // [CG]: return if opnd1 and opnd2 semantic undefined
  if (l0_opnd1_idx < 0 || l0_opnd2_idx < 0) return FALSE;

  tn0 = opnd_tn[l0_opnd1_idx];
  tn1 = opnd_tn[l0_opnd2_idx];

  if (EBO_Trace_Execution) {
    fprintf(TFile, "%s operands OU_opnd1: const: %d, has_val: %d, , val: %lld\n",
	    EBO_trace_pfx, TN_Is_Constant(tn0), TN_Has_Value(tn0), TN_Has_Value(tn0) ? TN_Value(tn0): 0);
    if (tn1) {
      fprintf(TFile, "%s operands OU_opnd2: const: %d, has_val: %d, , val: %lld\n",
	      EBO_trace_pfx, TN_Is_Constant(tn1), TN_Has_Value(tn1), TN_Has_Value(tn1) ? TN_Value(tn1): 0);
    }
  }


  // Floating -point simplification:
  if (tn0 == FZero_TN) {
    if ((IEEE_Arithmetic >= IEEE_INEXACT) && OP_fmul(op)) {
      OPS ops = OPS_EMPTY;

      if (EBO_Trace_Optimization) 
	fprintf(TFile,"replace fmpy of 0.0 with 0.0\n");

      EBO_Exp_COPY((OP_has_predicate(op)?OP_opnd(op,OP_PREDICATE_OPND):NULL),
		   tnr, FZero_TN, &ops);
      if (EBO_in_loop) 
	EBO_Set_OP_omega (OPS_first(&ops),
			  OP_has_predicate(op)?opnd_tninfo[OP_PREDICATE_OPND]:NULL,
			  NULL);
      OP_srcpos(OPS_first(&ops)) = OP_srcpos(op);
      BB_Insert_Ops(OP_bb(op), op, &ops, FALSE);
      return TRUE;
    }

    if (OP_fadd(op)) {
      OPS ops = OPS_EMPTY;
      if (EBO_Trace_Optimization) 
	fprintf(TFile,"replace fadd of 0.0 with tn1\n");

      EBO_Exp_COPY((OP_has_predicate(op)?OP_opnd(op,OP_PREDICATE_OPND):NULL),
		   tnr, tn1, &ops);
      if (EBO_in_loop) 
	EBO_Set_OP_omega (OPS_first(&ops),
			  OP_has_predicate(op)?opnd_tninfo[OP_PREDICATE_OPND]:NULL,
			  opnd_tninfo[l0_opnd2_idx]);
      OP_srcpos(OPS_first(&ops)) = OP_srcpos(op);
      BB_Insert_Ops(OP_bb(op), op, &ops, FALSE);
      return TRUE;
    }
  }

  if (tn0 == FOne_TN) {
    if (OP_fmul(op)) {
      OPS ops = OPS_EMPTY;

      if (EBO_Trace_Optimization) 
	fprintf(TFile,"replace fmpy of 1.0 with tn1\n");

      EBO_Exp_COPY((OP_has_predicate(op)?OP_opnd(op,OP_PREDICATE_OPND):NULL),
		   tnr, tn1, &ops);
      if (EBO_in_loop) 
	EBO_Set_OP_omega (OPS_first(&ops),
			  OP_has_predicate(op)?opnd_tninfo[OP_PREDICATE_OPND]:NULL,
			  opnd_tninfo[l0_opnd2_idx]);
      OP_srcpos(OPS_first(&ops)) = OP_srcpos(op);
      BB_Insert_Ops(OP_bb(op), op, &ops, FALSE);
      return TRUE;
    }
  }

  // Integer arithmetic simplification

  if (TN_is_symbol(tn0)) {
   /* Re-located constants not supported. */
    return FALSE;
  }

  if (!TN_Is_Constant(tn1) &&
      ((TN_register_class(tnr) != TN_register_class(tn1)) ||
       (TN_is_fpu_int(tnr) != TN_is_fpu_int(tn1)))) {
    /* Type changes indicate that something tricky is going on. */
    if (EBO_Trace_Data_Flow) {
      fprintf(TFile,"%sType mismatch between result and operand\n",EBO_trace_pfx);
    }
    return FALSE;
  }
  
  const_val = TN_Value(tn0);

#ifdef TARG_ST
  OPS ops = OPS_EMPTY;

  //
  // First of all, check if there is a target specific simplification
  //
  OP *new_op = EBO_simplify_operand0(op, tnr, const_val, tn1);
  if (new_op != NULL) {
    OPS_Append_Op(&ops, new_op);
    goto OPS_Created;
  }

  // shifts
  if (OP_ishl(op) || OP_ishr(op) || OP_ishru(op)) {
    INT op1_bits = TOP_opnd_use_bits(opcode, l0_opnd1_idx);
    BOOL op1_signed = TOP_opnd_use_signed(opcode, l0_opnd1_idx);
    const_val = op1_signed ? ((INT64)const_val << 64-op1_bits) >> 64-op1_bits:
      ((UINT64)const_val << 64-op1_bits) >> 64-op1_bits;
    if (const_val == 0) {
      TN *tnc = Gen_Literal_TN (const_val, TN_size(tnr));
      Expand_Immediate (tnr, tnc, 0, &ops);
      goto OPS_Created;
    } else if (const_val == -1 && OP_ishr(op)) {
      TN *tnc = Gen_Literal_TN (const_val, TN_size(tnr));
      Expand_Immediate (tnr, tnc, 0, &ops);
      goto OPS_Created;
    }
  }

#endif


  /* Sequence optimizations. */

  // 
  // 1. make postincrement memory OPs where possible
  //
#if 0
  if ((opcode == TOP_adds) &&
      ISA_LC_Value_In_Class (const_val, LC_i9)) {
    TN *tnr = OP_result(op,0);
    OP *test_op = OP_prev(op);
    OP *memory_op = NULL;
    EBO_OP_INFO *memory_opinfo = NULL;
    TOP memory_opcode; // = TOP_add;
    EBO_TN_INFO *memory_index = NULL;
    EBO_TN_INFO *memory_r1 = NULL;
    EBO_TN_INFO *memory_r2 = NULL;
    EBO_TN_INFO *memory_sv = NULL;

    while (test_op) {
      if (OP_load(test_op) || OP_store(test_op)) {
        memory_op = test_op;
        test_op = NULL;
        break;
      }
      test_op = OP_prev(test_op);
    }

    if (memory_op != NULL) {
      memory_opinfo = find_opinfo_entry (memory_op);
      memory_opcode = OP_code(memory_op);
      if (memory_opinfo != NULL) {
        memory_index = memory_opinfo->actual_opnd[TOP_Find_Operand_Use(memory_opcode, OU_base)];
        if (OP_load(memory_op)) {
          memory_r1 = memory_opinfo->actual_rslt[0];
          if (OP_results(memory_op) > 1) {
	    memory_r2 = memory_opinfo->actual_rslt[1];
	  }
        } else if (OP_store(memory_op)) {
          memory_sv = memory_opinfo->actual_opnd[TOP_Find_Operand_Use(memory_opcode, OU_storeval)];
        }
      }
    }

   /* 
    * Make sure that this is load has a predicate that is compatible 
    * with the increment. 
    */
    if (memory_opinfo != NULL) {
      if ((!OP_has_predicate(op) && !OP_has_predicate(memory_op)) ||
	  ((OP_has_predicate(op) && OP_has_predicate(memory_op)) &&
	   ((TN_Is_Constant(OP_opnd(op,OP_PREDICATE_OPND)) &&
	     TN_Is_Constant(OP_opnd(memory_op,OP_PREDICATE_OPND)) &&
	     (TN_Value(OP_opnd(op,OP_PREDICATE_OPND)) == TN_Value(OP_opnd(memory_op,OP_PREDICATE_OPND)))) ||
	    (!TN_Is_Constant(OP_opnd(op,OP_PREDICATE_OPND)) &&
	     !TN_Is_Constant(OP_opnd(memory_op,OP_PREDICATE_OPND)) &&
	     EBO_tn_available(bb, memory_opinfo->actual_opnd[OP_PREDICATE_OPND]) &&
	     tn_registers_identical(OP_opnd(op,OP_PREDICATE_OPND), OP_opnd(memory_op,OP_PREDICATE_OPND)))) &&
	   (OP_memory(memory_op)) &&
	   ((memory_index != NULL) &&
	    EBO_tn_available(bb, memory_index) &&
	    tn_registers_identical(memory_index->local_tn, tn1)) &&
	   ((memory_r1 == NULL) || (memory_r1->reference_count == 0)) &&
	   ((memory_r2 == NULL) || (memory_r2->reference_count == 0)) &&
	   ((memory_sv == NULL) || EBO_tn_available(bb, memory_sv)) &&
	   ((memory_sv == NULL) || !tn_registers_identical(tnr, memory_sv->local_tn)))) {

	EBO_

      TOP new_opcode;
      BOOL use_increment = TRUE;

      switch (memory_opcode) {
      case TOP_ld1: new_opcode = TOP_ld1_i; break;
      case TOP_ld2: new_opcode = TOP_ld2_i; break;
      case TOP_ld4: new_opcode = TOP_ld4_i; break;
      case TOP_ld8: new_opcode = TOP_ld8_i; break;
      case TOP_ld8_fill: new_opcode = TOP_ld8_i_fill; break;
      case TOP_ldfs: new_opcode = TOP_ldfs_i; break;
      case TOP_ldfd: new_opcode = TOP_ldfd_i; break;
      case TOP_ldfe: new_opcode = TOP_ldfe_i; break;
      case TOP_ldf8: new_opcode = TOP_ldf8_i; break;
      case TOP_ldf_fill: new_opcode = TOP_ldf_i_fill; break;
      case TOP_ldfps:
        new_opcode = TOP_ldfps_i;
        if (CGTARG_Mem_Ref_Bytes(memory_op) != const_val) use_increment = FALSE;
        break;
      case TOP_ldfpd:
        new_opcode = TOP_ldfpd_i;
        if (CGTARG_Mem_Ref_Bytes(memory_op) != const_val) use_increment = FALSE;
        break;
      case TOP_ldfp8:
        new_opcode = TOP_ldfp8_i;
        if (CGTARG_Mem_Ref_Bytes(memory_op) != const_val) use_increment = FALSE;
        break;
      case TOP_lfetch: new_opcode = TOP_lfetch_i; break;
      case TOP_lfetch_excl: new_opcode = TOP_lfetch_i_excl; break;
      case TOP_lfetch_fault: new_opcode = TOP_lfetch_i_fault; break;
      case TOP_lfetch_fault_excl: new_opcode = TOP_lfetch_i_fault_excl; break;
      case TOP_st1: new_opcode = TOP_st1_i; break;
      case TOP_st2: new_opcode = TOP_st2_i; break;
      case TOP_st4: new_opcode = TOP_st4_i; break;
      case TOP_st8: new_opcode = TOP_st8_i; break;
      case TOP_st8_spill: new_opcode = TOP_st8_i_spill; break;
      case TOP_stfs: new_opcode = TOP_stfs_i; break;
      case TOP_stfd: new_opcode = TOP_stfd_i; break;
      case TOP_stfe: new_opcode = TOP_stfe_i; break;
      case TOP_stf8: new_opcode = TOP_stf8_i; break;
      case TOP_stf_spill: new_opcode = TOP_stf_i_spill; break;
      case TOP_ldfps_i:
      case TOP_ldfpd_i:
      case TOP_ldfp8_i:
        use_increment = FALSE; break;
      default: 
        if (TOP_Find_Operand_Use(memory_opcode, OU_postincr) >= 0) {
          new_opcode = memory_opcode;
          const_val += TN_Value(OP_opnd(memory_op,TOP_Find_Operand_Use(memory_opcode, OU_postincr)));
          break;
        }
        use_increment = FALSE; break;
      }

     /* CGPREP doesn't like to see these instructions. */
      if (EBO_in_pre || EBO_in_loop) use_increment = FALSE;

     /* Scheduling may be better if we don't introduce dependencies. */
      if (!EBO_in_peep) use_increment = FALSE;

      if (use_increment &&
          (TOP_Find_Operand_Use(new_opcode, OU_postincr) >= 0)) {
        OP *new_op;
        OPS ops = OPS_EMPTY;

        if (!tn_registers_identical(tnr, tn1)) {
          EBO_Exp_COPY(OP_opnd(op,OP_PREDICATE_OPND), tnr, tn1, &ops);
          if (EBO_in_loop) EBO_Set_OP_omega (OPS_first(&ops),
                                             opnd_tninfo[OP_PREDICATE_OPND],
                                             opnd_tninfo[l0_opnd2_idx]);
        }

        TN *save_opnds[OP_MAX_FIXED_OPNDS];
        INT i;

 	Is_True(OP_opnds(memory_op) <= OP_MAX_FIXED_OPNDS,
		("save_opnds is too small for %s", TOP_Name(OP_code(memory_op))));
        for (i=0; i<OP_opnds(memory_op); i++) save_opnds[i] = OP_opnd(memory_op, i);

        if (OP_load(memory_op)) {
          if (OP_results(memory_op) == 1) {
            new_op = Mk_OP (new_opcode, OP_result(memory_op, 0), tnr,
                            save_opnds[0], save_opnds[1], save_opnds[2], NULL, NULL);
          } else {
            new_op = Mk_OP (new_opcode, OP_result(memory_op, 0), OP_result(memory_op, 1), tnr,
                            save_opnds[0], save_opnds[1], save_opnds[2], NULL, NULL);
          }
        } else {
          if (OP_opnds(memory_op) == 4) {
            new_op = Mk_OP(new_opcode, tnr, save_opnds[0], save_opnds[1], save_opnds[2], NULL, NULL);
          } else {
            new_op = Mk_OP(new_opcode, tnr, save_opnds[0], save_opnds[1], save_opnds[2], NULL, NULL, NULL);
          }

          Set_OP_opnd(new_op, TOP_Find_Operand_Use(new_opcode, OU_storeval),
                      save_opnds[TOP_Find_Operand_Use(memory_opcode, OU_storeval)]);
        }

        Set_OP_opnd(new_op, TOP_Find_Operand_Use(new_opcode, OU_base), tnr);
        Set_OP_opnd(new_op, TOP_Find_Operand_Use(new_opcode, OU_postincr), Gen_Literal_TN(const_val, 4));
        Copy_WN_For_Memory_OP (new_op, memory_op);
        OP_srcpos(new_op) = OP_srcpos(memory_op);
        OPS_Append_Op( &ops, new_op);

        BB_Insert_Ops(bb, op, &ops, FALSE);

        remove_op (memory_opinfo);
        OP_Change_To_Noop(memory_op);
        memory_opinfo->in_op = NULL;
        memory_opinfo->in_bb = NULL;

if (EBO_Trace_Optimization) fprintf(TFile,"memory op merged with increment to produce %s (inc=%lld).\n",
TOP_Name(new_opcode),const_val);
        return TRUE;
      }
    }
  }
#endif

#if 0
  tninfo = opnd_tninfo[l0_opnd2_idx];
  if (tninfo == NULL) return FALSE;
  pred_op = tninfo->in_op;
  if (pred_op == NULL) return FALSE;
  pred_opcode = OP_code(pred_op);
  if (pred_opcode == TOP_spadjust) return FALSE;
  pred_opinfo = locate_opinfo_entry (tninfo);
  if (pred_opinfo == NULL) return FALSE;

 /* First, look for some special cases involving loads. */
  if (OP_load(pred_op)) {
    if (OP_iand(op)) {
      if (((pred_opcode == TOP_ld1) &&
           (const_val == (~((~0) << 8)))) ||
          ((pred_opcode == TOP_ld2) &&
           (const_val ==  (~((~0) << 16))))) {
       /* No need to mask - it was done with the load. */
        OPS ops = OPS_EMPTY;
if (EBO_Trace_Optimization) fprintf(TFile,"no need to mask after load\n");
        EBO_Exp_COPY((OP_has_predicate(op)?OP_opnd(op,OP_PREDICATE_OPND):NULL),
                     tnr, tn1, &ops);
        if (EBO_in_loop) EBO_Set_OP_omega (OPS_first(&ops),
                                           OP_has_predicate(op)?opnd_tninfo[OP_PREDICATE_OPND]:NULL,
                                           opnd_tninfo[l0_opnd2_idx]);
        OP_srcpos(OPS_first(&ops)) = OP_srcpos(op);
        BB_Insert_Ops(bb, op, &ops, FALSE);
        return TRUE;
      }
    }
    return FALSE;
  }

  if ((opcode == TOP_dep) ||
      (opcode == TOP_dep_i)) {

   /* Is the input to this dep instruction another dep instruction? */
    if ((pred_opcode == TOP_dep) ||
        (pred_opcode == TOP_dep_i)) {
      TN *pred_insert = OP_opnd(pred_op,1);
      EBO_TN_INFO *pred_insert_tninfo = pred_opinfo->actual_opnd[1];
      TN *pred_tn = OP_opnd(pred_op,2);
      INT pred_start = TN_Value(OP_opnd(pred_op,3));
      INT pred_length = TN_Value(OP_opnd(pred_op,4));
      INT op_start = TN_Value(OP_opnd(op,3));
      INT op_length = TN_Value(OP_opnd(op,4));

     /* Pick up the best value we can for the predicessor's field value. */
      if (pred_opinfo->optimal_opnd[1] != NULL) {
        pred_insert_tninfo = pred_opinfo->optimal_opnd[1];
        pred_insert = pred_insert_tninfo->local_tn;
      }
      if ((pred_insert_tninfo != NULL) &&
          (pred_insert_tninfo->replacement_tn != NULL)) {
        pred_insert = pred_insert_tninfo->replacement_tn;
        pred_insert_tninfo = pred_insert_tninfo->replacement_tninfo;
      }

     /* The transformations that we want to make will require that
        we utilize a new register.  This is not problem if registers
        have not been assigned, and is not a problem if we can reuse
        the result register of the dep instruction - which we can not
        do if it is going to be an input to the new dep instruction.
     */
      if (has_assigned_reg(tnr) &&
          TNs_Are_Equivalent(tnr,pred_tn)) return FALSE;

      if (TN_Is_Constant(pred_insert) &&
          ((pred_start+pred_length) == op_start) &&
          ((op_length+pred_length) < 16)) {
       /* The new field is to the left of the previous field. */
        OPS ops = OPS_EMPTY;
        OP *new_op;
        TN *predicate_tn = OP_opnd(op,OP_PREDICATE_OPND);
        TN *new_const_tn = has_assigned_reg(tnr) ? tnr : Dup_TN_Even_If_Dedicated(tnr);
        UINT64 mask;

        mask = (~0);
        mask = mask << op_length;
        const_val = (const_val & ~mask) << pred_length;

        mask = (~0);
        mask = mask << pred_length;
        const_val |= (TN_Value(pred_insert) & ~mask);
if (EBO_Trace_Optimization) fprintf(TFile,"combine dep to the left of previous dep\n");
        EBO_Exp_COPY(predicate_tn, new_const_tn, Gen_Literal_TN(const_val, TN_size(tnr)), &ops);
        if (EBO_in_loop) EBO_Set_OP_omega (OPS_first(&ops),
                                           opnd_tninfo[OP_PREDICATE_OPND],
                                           pred_opinfo->actual_opnd[2]);
        OP_srcpos(OPS_first(&ops)) = OP_srcpos(op);

        Build_OP(TOP_dep, tnr, predicate_tn, new_const_tn, OP_opnd(pred_op,2),
                 OP_opnd(pred_op,3), Gen_Literal_TN(op_length+pred_length, 4), &ops);
        if (predicate_tn != True_TN) Set_OP_cond_def_kind(OPS_last(&ops), OP_PREDICATED_DEF);
        OP_srcpos(OPS_last(&ops)) = OP_srcpos(op);
        if (EBO_in_loop) EBO_Set_OP_omega (OPS_last(&ops),
                                           opnd_tninfo[OP_PREDICATE_OPND], NULL,
                                           pred_opinfo->actual_opnd[2]);

        BB_Insert_Ops(bb, op, &ops, FALSE);
        return TRUE;
      }

      if (TN_Is_Constant(pred_insert) &&
          ((op_start+op_length) == pred_start) &&
          ((op_length+pred_length) < 16)) {
       /* The new field is to the right of the previous field. */
        OPS ops = OPS_EMPTY;
        OP *new_op;
        TN *predicate_tn = OP_opnd(op,OP_PREDICATE_OPND);
        TN *new_const_tn = has_assigned_reg(tnr) ? tnr : Dup_TN_Even_If_Dedicated(tnr);
        UINT64 mask;

        mask = (~0);
        mask = mask << op_length;
        const_val = (const_val & ~mask);

        mask = (~0);
        mask = mask << pred_length;
        const_val |= (TN_Value(pred_insert) & ~mask) << op_length;
if (EBO_Trace_Optimization) fprintf(TFile,"combine dep to the right of previous dep\n");
        EBO_Exp_COPY(predicate_tn, new_const_tn, Gen_Literal_TN(const_val, TN_size(tnr)), &ops);
        if (EBO_in_loop) EBO_Set_OP_omega (OPS_first(&ops),
                                           opnd_tninfo[OP_PREDICATE_OPND],
                                           pred_opinfo->actual_opnd[2]);
        OP_srcpos(OPS_first(&ops)) = OP_srcpos(op);

        Build_OP(TOP_dep, tnr, predicate_tn, new_const_tn, OP_opnd(pred_op,2),
                 OP_opnd(op,3), Gen_Literal_TN(op_length+pred_length, 4), &ops);
        if (predicate_tn != True_TN) Set_OP_cond_def_kind(OPS_last(&ops), OP_PREDICATED_DEF);
        OP_srcpos(OPS_last(&ops)) = OP_srcpos(op);
        if (EBO_in_loop) EBO_Set_OP_omega (OPS_last(&ops),
                                           opnd_tninfo[OP_PREDICATE_OPND], NULL,
                                           pred_opinfo->actual_opnd[2]);

        BB_Insert_Ops(bb, op, &ops, FALSE);
        return TRUE;
      }
    }
  }
#endif

#if 0
 /* Now, look into optimizations that depend on specific
    values for the actual operands of the predecessor op. */
  if (TOP_Find_Operand_Use(pred_opcode,OU_opnd2) >= 0) {
    INT ptn0_idx = TOP_Find_Operand_Use(pred_opcode,OU_opnd1);
    INT ptn1_idx = TOP_Find_Operand_Use(pred_opcode,OU_opnd2);
    TN *ptn0 = OP_opnd(pred_op, ptn0_idx);
    TN *ptn1 = OP_opnd(pred_op, ptn1_idx);
    EBO_TN_INFO *ptn0_tninfo;
    EBO_TN_INFO *ptn1_tninfo;
    INT64 pred_val = 0;

    OP *new_op;
    TOP new_opcode;
    INT64 new_const_val;

    ptn0_tninfo = pred_opinfo->actual_opnd[ptn0_idx];
    ptn1_tninfo = pred_opinfo->actual_opnd[ptn1_idx];

   /* Are the operands available for use? */
    if (!(TN_Is_Constant(ptn0) || EBO_tn_available (bb, ptn0_tninfo)) ||
        !(TN_Is_Constant(ptn1) || EBO_tn_available (bb, ptn1_tninfo))) return FALSE;

    if (opcode == TOP_adds) {
      if (((pred_opcode == TOP_add) && (const_val == 1)) ||
          ((pred_opcode == TOP_sub) && (const_val == -1))) {
       /* Use the special add+1 instruction. */
          new_opcode = (pred_opcode == TOP_add) ? TOP_add_1 : TOP_sub_1;
          new_op = Mk_OP (new_opcode, tnr, OP_opnd(op,OP_PREDICATE_OPND), ptn0, ptn1);
          OP_srcpos(new_op) = OP_srcpos(op);
          if (EBO_in_loop) EBO_Set_OP_omega ( new_op, opnd_tninfo[OP_PREDICATE_OPND], ptn0_tninfo, ptn1_tninfo );
          BB_Insert_Op_After(bb, op, new_op);
if (EBO_Trace_Optimization) fprintf(TFile,"combine add and add1\n");
          return TRUE;
      }
    }

    if (TN_Is_Constant(ptn1)) {
    } else if (TN_Is_Constant(ptn0) &&
               (OP_iadd(pred_op) || OP_ior(pred_op)  || OP_iand(pred_op))) {
     /* Treat ptn1 as if it were the constant operand. */
      TN *s = ptn0;
      ptn0 = ptn1;
      ptn0_tninfo = ptn1_tninfo;
      ptn1 = s;
    } else {
      return FALSE;
    }

    pred_val = TN_is_symbol(ptn1) ? TN_offset(ptn1) : TN_Value(ptn1);

   /* Look for compare operations where the constants can be combined. */
    if (OP_icmp(op) && OP_iadd(pred_op) && !TN_is_symbol(ptn1)) {
      INT cmp_size;

      if ((pred_val != 0) &&
          ((opcode == TOP_cmp4_ltu)       ||
           (opcode == TOP_cmp4_ltu_unc)   ||
           (opcode == TOP_cmp4_leu)       ||
           (opcode == TOP_cmp4_leu_unc)   ||
           (opcode == TOP_cmp4_gtu)       ||
           (opcode == TOP_cmp4_gtu_unc)   ||
           (opcode == TOP_cmp4_geu)       ||
           (opcode == TOP_cmp4_geu_unc)   ||
           (opcode == TOP_cmp_ltu)        ||
           (opcode == TOP_cmp_ltu_unc)    ||
           (opcode == TOP_cmp_leu)        ||
           (opcode == TOP_cmp_leu_unc)    ||
           (opcode == TOP_cmp_gtu)        ||
           (opcode == TOP_cmp_gtu_unc)    ||
           (opcode == TOP_cmp_geu)        ||
           (opcode == TOP_cmp_geu_unc)    ||
           (opcode == TOP_cmp4_i_ltu)     ||
           (opcode == TOP_cmp4_i_ltu_unc) ||
           (opcode == TOP_cmp4_i_leu)     ||
           (opcode == TOP_cmp4_i_leu_unc) ||
           (opcode == TOP_cmp4_i_gtu)     ||
           (opcode == TOP_cmp4_i_gtu_unc) ||
           (opcode == TOP_cmp4_i_geu)     ||
           (opcode == TOP_cmp4_i_geu_unc) ||
           (opcode == TOP_cmp_i_ltu)      ||
           (opcode == TOP_cmp_i_ltu_unc)  ||
           (opcode == TOP_cmp_i_leu)      ||
           (opcode == TOP_cmp_i_leu_unc)  ||
           (opcode == TOP_cmp_i_gtu)      ||
           (opcode == TOP_cmp_i_gtu_unc)  ||
           (opcode == TOP_cmp_i_geu)      ||
           (opcode == TOP_cmp_i_geu_unc))) {
       /* There may be sign-bit games going on with unsigned compares. */
        return FALSE;
      }
      new_const_val = const_val - pred_val; 

      switch (opcode) {
      case TOP_cmp4_i_geu_unc:
        new_const_val = Normalize_Immediate(opcode, new_const_val);
      case TOP_cmp_i_eq:
      case TOP_cmp_i_eq_unc:
      case TOP_cmp_i_eq_and:
      case TOP_cmp_i_eq_or:
      case TOP_cmp_i_eq_or_andcm:
      case TOP_cmp_i_ne_and:
      case TOP_cmp_i_ne_or:
      case TOP_cmp_i_ne_or_andcm:
      case TOP_cmp_i_lt:
      case TOP_cmp_i_lt_unc:
      case TOP_cmp_i_ltu:
      case TOP_cmp_i_ltu_unc:
      case TOP_cmp_i_eq_orcm:
      case TOP_cmp_i_eq_andcm:
      case TOP_cmp_i_eq_and_orcm:
      case TOP_cmp_i_ne:
      case TOP_cmp_i_ne_unc:
      case TOP_cmp_i_ne_orcm:
      case TOP_cmp_i_ne_andcm:
      case TOP_cmp_i_ne_and_orcm:
      case TOP_cmp_i_le:
      case TOP_cmp_i_le_unc:
      case TOP_cmp_i_gt:
      case TOP_cmp_i_gt_unc:
      case TOP_cmp_i_ge:
      case TOP_cmp_i_ge_unc:
      case TOP_cmp_i_leu:
      case TOP_cmp_i_leu_unc:
      case TOP_cmp_i_gtu:
      case TOP_cmp_i_gtu_unc:
      case TOP_cmp_i_geu:
      case TOP_cmp_i_geu_unc:
        if (TOP_Can_Have_Immediate(new_const_val, opcode)) {
          OPS ops = OPS_EMPTY;
if (EBO_Trace_Optimization) fprintf(TFile,"merge cmp_i with add: (%lld)\n",new_const_val);
          Build_OP (opcode, OP_result(op,0), OP_result(op,1), OP_opnd(op,OP_PREDICATE_OPND),
                    Gen_Literal_TN(new_const_val, TN_size(tn1)), ptn0, &ops);
          if (EBO_in_loop) EBO_Set_OP_omega (OPS_first(&ops), opnd_tninfo[OP_PREDICATE_OPND],
                                             NULL, ptn0_tninfo );
          OP_srcpos(OPS_first(&ops)) = OP_srcpos(op);
          BB_Insert_Ops(OP_bb(op), op, &ops, FALSE);
          return TRUE;
        }
        break;
      case TOP_cmp4_lt:
      case TOP_cmp4_lt_unc:
      case TOP_cmp4_le:
      case TOP_cmp4_le_unc:
      case TOP_cmp4_gt:
      case TOP_cmp4_gt_unc:
      case TOP_cmp4_ge:
      case TOP_cmp4_ge_unc:
      case TOP_cmp_lt:
      case TOP_cmp_lt_unc:
      case TOP_cmp_le:
      case TOP_cmp_le_unc:
      case TOP_cmp_gt:
      case TOP_cmp_gt_unc:
      case TOP_cmp_ge:
      case TOP_cmp_ge_unc:
        cmp_size = EBO_bit_length (op);
        if ((cmp_size > 1) &&
            (((const_val >> (cmp_size-1)) &1) != ((new_const_val >> (cmp_size-1)) &1))) {
         /* The sign bit on the constant changed.  This may be a problem! */
          break;
        }
      default:
        if (!has_assigned_reg(tn1)) {
          OPS ops = OPS_EMPTY;
          TN *tnr = Dup_TN_Even_If_Dedicated (OP_opnd(op,1));
          TN *tnc = Gen_Literal_TN(new_const_val, TN_size(tn1));
if (EBO_Trace_Optimization) fprintf(TFile,"merge cmp with add (0x%llx)\n",new_const_val);
          Expand_Immediate (tnr, tnc, (new_const_val < 0) ? TRUE : FALSE, &ops);
          if (OP_has_predicate(op)) {
            EBO_OPS_predicate (OP_opnd(op, OP_PREDICATE_OPND), &ops);
          }
          if (EBO_in_loop) EBO_OPS_omega (&ops, 
					  (OP_has_predicate(op)? OP_opnd(op,OP_PREDICATE_OPND):NULL),
					  (OP_has_predicate(op)?opnd_tninfo[OP_PREDICATE_OPND]:NULL));
          Build_OP (opcode, OP_result(op,0), OP_result(op,1), OP_opnd(op,OP_PREDICATE_OPND),
                    tnr, ptn0, &ops);
          if (EBO_in_loop) EBO_Set_OP_omega (OPS_last(&ops), opnd_tninfo[OP_PREDICATE_OPND],
                                             NULL, ptn0_tninfo );
          OP_srcpos(OPS_first(&ops)) = OP_srcpos(op);
          BB_Insert_Ops(OP_bb(op), op, &ops, FALSE);
          return TRUE;
        }
        break;
      }

    }

   /* Look for identical operations where the constants can be combined. */
    if (OP_iadd(op) && OP_iadd(pred_op)) {
      new_const_val = pred_val + const_val;

      if (ISA_LC_Value_In_Class (new_const_val, LC_i22)) {
        TN *new_tn;
        new_opcode = (ISA_LC_Value_In_Class (new_const_val, LC_i14)) ? TOP_adds : TOP_addl;
        if (new_opcode == TOP_addl) return FALSE; /* Skip transformations to addl. */

        if (TN_is_symbol(ptn1)) {
          ST *st = TN_var(ptn1);
          ST *base_st;
          INT64 base_ofst;
          INT32 rel = TN_relocs(ptn1);

          Base_Symbol_And_Offset (st, &base_st, &base_ofst);
          if (ST_on_stack(st) &&
              ((ST_sclass(st) == SCLASS_AUTO) ||
               (EBO_in_peep && (ST_sclass(st) == SCLASS_FORMAL))) &&
              ISA_LC_Value_In_Class (new_const_val+base_ofst, LC_i14)) {
            new_tn = Gen_Symbol_TN(st, new_const_val, rel);
          } else return FALSE;
        } else {
          new_tn = Gen_Literal_TN(new_const_val, TN_size(tn1));
        }

        new_op = Mk_OP (new_opcode, tnr, OP_opnd(op,OP_PREDICATE_OPND), new_tn, ptn0);
        OP_srcpos(new_op) = OP_srcpos(op);
        if (EBO_in_loop) EBO_Set_OP_omega ( new_op, opnd_tninfo[OP_PREDICATE_OPND], NULL, ptn0_tninfo );
        BB_Insert_Op_After(bb, op, new_op);
if (EBO_Trace_Optimization) fprintf(TFile,"combine immediate adds\n");
        return TRUE;
      }

      return FALSE;
    }

    if (TN_is_symbol(ptn1)) return FALSE;

    if (pred_val == const_val) {

     /* Look for identical operations that are redundant. */
      if ((OP_iand(op) && OP_iand(pred_op) && (const_val != ~0)) ||
          (OP_ior(op) && OP_ior(pred_op) && (const_val != 0))) {
        OPS ops = OPS_EMPTY;
if (EBO_Trace_Optimization) fprintf(TFile,"replace redundant and/or\n");
        EBO_Exp_COPY((OP_has_predicate(op)?OP_opnd(op,OP_PREDICATE_OPND):NULL),
                     tnr, OP_result(pred_op, 0), &ops);
        if (EBO_in_loop) EBO_OPS_omega (&ops, 
					(OP_has_predicate(op)?OP_opnd(op,OP_PREDICATE_OPND):NULL),
					(OP_has_predicate(op)?opnd_tninfo[OP_PREDICATE_OPND]:NULL));
        OP_srcpos(OPS_first(&ops)) = OP_srcpos(op);
        BB_Insert_Ops(bb, op, &ops, FALSE);
        return TRUE;
      }
    
    } /* end : if (pred_val == const_val) */

  } else if (((const_val >> EBO_bit_length(pred_op)) == 0) &&
             ((pred_opcode == TOP_sxt1) || (pred_opcode == TOP_sxt2) || (pred_opcode == TOP_sxt4) || 
              (pred_opcode == TOP_zxt1) || (pred_opcode == TOP_zxt2) || (pred_opcode == TOP_zxt4)) &&
             ((opcode == TOP_cmp_eq)    || (opcode == TOP_cmp_eq_unc) ||
              (opcode == TOP_cmp_i_eq)  || (opcode == TOP_cmp_i_eq_unc) ||
              (opcode == TOP_cmp_ne)    || (opcode == TOP_cmp_ne_unc) ||
              (opcode == TOP_cmp_i_ne)  || (opcode == TOP_cmp_i_ne_unc) ||
              (opcode == TOP_cmp4_eq)   || (opcode == TOP_cmp4_eq_unc) ||
              (opcode == TOP_cmp4_i_eq) || (opcode == TOP_cmp4_i_eq_unc) ||
              (opcode == TOP_cmp4_ne)   || (opcode == TOP_cmp4_ne_unc) ||
              (opcode == TOP_cmp4_i_ne) || (opcode == TOP_cmp4_i_ne_unc))) {
   /* Compares of 0 against a sign extended value may not need the sign-extension. */
    INT ptn0_idx = 1;  /* TOP_Find_Operand_Use(pred_opcode,OU_opnd1) does not work. */
    TN *ptn0 = OP_opnd(pred_op, ptn0_idx);
    EBO_TN_INFO *ptn0_tninfo;

    OP *new_op;
    TOP new_opcode;
    INT64 new_const_val;

    ptn0_tninfo = pred_opinfo->actual_opnd[ptn0_idx];

   /* Are the operands available for use? */
    if ((ptn0_tninfo == NULL) || 
        (ptn0_tninfo->in_op == NULL) ||
        !EBO_tn_available (bb, ptn0_tninfo)) return FALSE;

   /* Find the OP associated with the input to pred_op. */
    OP *pred_pred_op = ptn0_tninfo->in_op;

   /* Is the input to the pred_op a load? */
    if (!OP_load(pred_pred_op)) return FALSE;

   /* Are the lengths the same? */
    if (EBO_bit_length(pred_op) != EBO_bit_length(pred_pred_op)) return FALSE;

   /* Skip the sign extension instruction and just use the load. */
    new_op = Dup_OP (op);
    Set_OP_opnd (new_op, l0_opnd2_idx, ptn0);
    OP_srcpos(new_op) = OP_srcpos(op);
    if (EBO_in_loop) EBO_Set_OP_omega ( new_op, opnd_tninfo[OP_PREDICATE_OPND], NULL, ptn0_tninfo );
    BB_Insert_Op_After(bb, op, new_op);
if (EBO_Trace_Optimization) fprintf(TFile,"sign-extension in load-extend-cmp sequence is not needed\n");
    return TRUE;
  } /* end: Sequence optimizations. */

#endif

  return FALSE;

#ifdef TARG_ST
  OPS_Created:
  FmtAssert(OPS_first(&ops) != NULL, ("ops empty after simplify operand 0"));

  if (OP_next(OPS_first(&ops)) != NULL) {
    /* What's the point in replacing one instruction with several? */
    return FALSE;
  }
  /* No need to replace if same op, avoids infinite loops. */
  if (OPs_Are_Equivalent(op, OPS_first(&ops))) return FALSE;

  OP_srcpos(OPS_first(&ops)) = OP_srcpos(op);
  if (EBO_in_loop) {
    EBO_OPS_omega(&ops, (OP_has_predicate(op)? OP_opnd(op, OP_PREDICATE_OPND):NULL),
		  (OP_has_predicate(op)? opnd_tninfo[OP_PREDICATE_OPND]:NULL));
    EBO_OPS_omega(&ops, opnd_tn[l0_opnd2_idx], opnd_tninfo[l0_opnd2_idx]);
  }

  BB_Insert_Ops(OP_bb(op), op, &ops, FALSE);
  return TRUE;
#endif
}


/* =====================================================================
 *    EBO_Constant_Operand1
 *
 *    Look at an exression that has a constant second operand and 
 *    attempt to simplify the computations.
 * =====================================================================
 */
BOOL
EBO_Constant_Operand1 (
  OP *op,
  TN **opnd_tn,
  EBO_TN_INFO **opnd_tninfo
)
{
  BB *bb = OP_bb(op);
  INT opndnum = OP_opnds(op);
  EBO_TN_INFO *tninfo;

  TOP opcode = OP_code(op);
  TN *tnr = OP_result(op,0);
  INT l0_opnd1_idx;
  INT l0_opnd2_idx;
  TN *tn0;
  TN *tn1;
  INT64 const_val;

  OP *pred_op;
  TOP pred_opcode;
  EBO_OP_INFO *pred_opinfo;

  if (EBO_Trace_Execution) {
    INT i;
    fprintf(TFile, "%sin BB:%d special1 OP :- %s ",
            EBO_trace_pfx, BB_id(bb),TOP_Name(OP_code(op)));
    for (i = 0; i < opndnum; i++) {
      fprintf(TFile," ");
      Print_TN(opnd_tn[i],FALSE);
    }
    fprintf(TFile,"\n");
  }

  // Arthur: spadjust shouldn't be subject to this
  if (OP_code(op) == TOP_spadjust) return FALSE;

  l0_opnd1_idx = OP_find_opnd_use(op, OU_opnd1);
  l0_opnd2_idx = OP_find_opnd_use(op, OU_opnd2);

  // [CG]: return if opnd1 or opnd2 semantic undefined
  if (l0_opnd1_idx < 0 || l0_opnd2_idx < 0) return FALSE;

  tn0 = opnd_tn[l0_opnd1_idx];
  tn1 = opnd_tn[l0_opnd2_idx];

  if (EBO_Trace_Execution) {
    fprintf(TFile, "%s operands OU_opnd1: const: %d, has_val: %d, , val: %lld\n",
	    EBO_trace_pfx, TN_Is_Constant(tn0), TN_Has_Value(tn0), TN_Has_Value(tn0) ? TN_Value(tn0): 0);
    if (tn1) {
      fprintf(TFile, "%s operands OU_opnd2: const: %d, has_val: %d, , val: %lld\n",
	      EBO_trace_pfx, TN_Is_Constant(tn1), TN_Has_Value(tn1), TN_Has_Value(tn1) ? TN_Value(tn1): 0);
    }
  }

#if 0
  // Floating-point simplification
  if (tn1 == FZero_TN) {
    if ((IEEE_Arithmetic >= IEEE_INEXACT) && OP_fmul(op)) {
      OPS ops = OPS_EMPTY;

      if (EBO_Trace_Optimization) 
	fprintf(TFile,"replace fmpy of 0.0 with 0.0\n");

      EBO_Exp_COPY((OP_has_predicate(op)?OP_opnd(op,OP_PREDICATE_OPND):NULL),
		   tnr, FZero_TN, &ops);
      if (EBO_in_loop) 
	EBO_Set_OP_omega (OPS_first(&ops),
			  OP_has_predicate(op)?opnd_tninfo[OP_PREDICATE_OPND]:NULL,
			  NULL);
      OP_srcpos(OPS_first(&ops)) = OP_srcpos(op);
      BB_Insert_Ops(OP_bb(op), op, &ops, FALSE);
      return TRUE;
    }

    if (OP_fadd(op) || OP_fsub(op)) {
      OPS ops = OPS_EMPTY;

      if (EBO_Trace_Optimization) 
	fprintf(TFile,"replace fadd of 0.0 with tn0\n");

      EBO_Exp_COPY((OP_has_predicate(op)?OP_opnd(op,OP_PREDICATE_OPND):NULL),
		   tnr, tn0, &ops);
      if (EBO_in_loop) 
	EBO_Set_OP_omega (OPS_first(&ops),
			  OP_has_predicate(op)?opnd_tninfo[OP_PREDICATE_OPND]:NULL,
			  opnd_tninfo[l0_opnd1_idx]);
      OP_srcpos(OPS_first(&ops)) = OP_srcpos(op);
      BB_Insert_Ops(OP_bb(op), op, &ops, FALSE);
      return TRUE;
    }
  }

  if (tn1 == FOne_TN) {
    if (OP_fmul(op)) {
      OPS ops = OPS_EMPTY;

      if (EBO_Trace_Optimization) 
	fprintf(TFile,"replace fmpy of 1.0 with tn0\n");

      EBO_Exp_COPY((OP_has_predicate(op)?OP_opnd(op,OP_PREDICATE_OPND):NULL),
		   tnr, tn0, &ops);
      if (EBO_in_loop) 
	EBO_Set_OP_omega (OPS_first(&ops),
			  OP_has_predicate(op)?opnd_tninfo[OP_PREDICATE_OPND]:NULL,
			  opnd_tninfo[l0_opnd1_idx]);
      OP_srcpos(OPS_first(&ops)) = OP_srcpos(op);
      BB_Insert_Ops(OP_bb(op), op, &ops, FALSE);
      return TRUE;
    }
  }
#endif

  // Integer arithmetic simplification

  if (TN_is_symbol(tn1)) {
   /* Re-located constants not supported. */
    return FALSE;
  }

  if (!TN_Is_Constant(tn0) &&
      (TN_register_class(tn0) != TN_register_class(tnr)) ||
      (TN_is_fpu_int(tnr) != TN_is_fpu_int(tn0))) {
   /* Type changes indicate that something tricky is going on. */
    if (EBO_Trace_Data_Flow) {
      fprintf(TFile,"%sType mismatch between result and operand\n",EBO_trace_pfx);
    }
    return FALSE;
  }

  const_val = TN_Value(tn1);

#ifdef TARG_ST
  OPS ops = OPS_EMPTY;

  //
  // First of all, check if there is a target specific simplification
  //
  OP *new_op = EBO_simplify_operand1 (op, tnr, tn0, const_val);
  if (new_op != NULL) {
    OPS_Append_Op(&ops, new_op);
    goto OPS_Created;
  }

  // shifts
  if (OP_ishl(op) || 
      OP_ishr(op) ||
      OP_ishru(op)) {
    INT op1_bits = TOP_opnd_use_bits(opcode, l0_opnd1_idx);
    INT op2_bits = TOP_opnd_use_bits(opcode, l0_opnd2_idx);
    const_val = ((UINT64)const_val << 64-op2_bits) >> 64-op2_bits;
    if (const_val == 0) {
      if (TN_is_register(tn0)) {
	Exp_COPY(tnr, tn0, &ops);
	goto OPS_Created;
      }
    } else if (const_val >= op1_bits &&
	       (OP_ishl(op) || OP_ishru(op))) {
      TN *tnc = Gen_Literal_TN(0, TN_size(tnr));
      Expand_Immediate (tnr, tnc, 0, &ops);
      goto OPS_Created;
    } else if (TN_has_value(OP_opnd(op, l0_opnd2_idx)) && 
	       TN_value(OP_opnd(op, l0_opnd2_idx)) != const_val) {
      OP *new_op = Dup_OP (op);
      Set_OP_opnd(new_op, l0_opnd2_idx, Gen_Literal_TN(const_val, TN_size(tnr)));
      OPS_Append_Op(&ops, new_op);
      goto OPS_Created;
    }
  }
  
  /* Value 0 -> copy. */
  if ((const_val == 0) &&
      (OP_isub(op) || 
       OP_iadd(op) ||
       OP_ior(op) ||
       OP_ixor(op))) {
    if (TN_is_register(tn0)) {
      Exp_COPY(tnr, tn0, &ops);
      goto OPS_Created;
    }
  }

  /* Value 0 -> 0. */
  if ((const_val == 0) &&
      (OP_iand(op))) {
    TN *tnc = Gen_Literal_TN(0, TN_size(tnr));
    Expand_Immediate (tnr, tnc, 0, &ops);
    goto OPS_Created;
  }
#endif

#if 0
  if ((opcode == TOP_add) &&
      (ISA_LC_Value_In_Class ( const_val, LC_i22))) {
    OP *new_op;
    TOP new_opcode = (ISA_LC_Value_In_Class (const_val, LC_i14)) ? TOP_adds : TOP_addl;
    if (new_opcode == TOP_addl) return FALSE; /* Skip transformations to addl. */

    new_op = Mk_OP(new_opcode, tnr, OP_opnd(op,OP_PREDICATE_OPND),
                   Gen_Literal_TN(const_val, TN_size(tn0)), tn0);
    OP_srcpos(new_op) = OP_srcpos(op);
    if (EBO_in_loop) EBO_Set_OP_omega ( new_op, opnd_tninfo[OP_PREDICATE_OPND], NULL, opnd_tninfo[l0_opnd1_idx]);
    BB_Insert_Op_After(OP_bb(op), op, new_op);
if (EBO_Trace_Optimization) fprintf(TFile,"Reverse and replace add with adds\n");
    return TRUE;
  }

  if ((opcode == TOP_sub) &&
      (ISA_LC_Value_In_Class (-const_val, LC_i22))) {
    OP *new_op;
    TOP new_opcode = (ISA_LC_Value_In_Class ( -const_val, LC_i14)) ? TOP_adds : TOP_addl;
    if (new_opcode == TOP_addl) return FALSE; /* Skip transformations to addl. */

    new_op = Mk_OP(new_opcode, tnr, OP_opnd(op,OP_PREDICATE_OPND),
                   Gen_Literal_TN(-const_val, TN_size(tn0)), tn0);
    OP_srcpos(new_op) = OP_srcpos(op);
    if (EBO_in_loop) EBO_Set_OP_omega ( new_op, opnd_tninfo[OP_PREDICATE_OPND], NULL, opnd_tninfo[l0_opnd1_idx]);
    BB_Insert_Op_After(OP_bb(op), op, new_op);
if (EBO_Trace_Optimization) fprintf(TFile,"replace sub with add_i\n");
    return TRUE;
  }

  if (opcode == TOP_and) {
    if (ISA_LC_Value_In_Class (const_val, LC_i8)) {
      OP *new_op;
if (EBO_Trace_Optimization) fprintf(TFile,"replace and with and_i\n");
      new_op = Mk_OP(TOP_and_i, tnr, OP_opnd(op,OP_PREDICATE_OPND),
                     Gen_Literal_TN(const_val, TN_size(tn0)), tn0);
      OP_srcpos(new_op) = OP_srcpos(op);
      if (EBO_in_loop) EBO_Set_OP_omega ( new_op, opnd_tninfo[OP_PREDICATE_OPND], NULL, opnd_tninfo[l0_opnd1_idx]);
      BB_Insert_Op_After(OP_bb(op), op, new_op);
      return TRUE;
    } else {
   /* Look for a mask in the constant and replace the and(load-immediate,tn) with an extr. */
      OPS ops = OPS_EMPTY;
      if (Expand_Special_And_Immed (tnr, tn0, const_val, &ops)) {
if (EBO_Trace_Optimization) fprintf(TFile,"replace and with bit move\n");
        Set_OP_opnd(OPS_first(&ops), OP_PREDICATE_OPND, OP_opnd(op,OP_PREDICATE_OPND));
        if (EBO_in_loop) EBO_Set_OP_omega (OPS_first(&ops), opnd_tninfo[OP_PREDICATE_OPND],
                                           NULL, opnd_tninfo[l0_opnd1_idx]);
        OP_srcpos(OPS_first(&ops)) = OP_srcpos(op);
        BB_Insert_Ops(OP_bb(op), op, &ops, FALSE);
        return TRUE;
      }
    }
  }

  if (opcode == TOP_andcm) {
    if (ISA_LC_Value_In_Class (~const_val, LC_i8)) {
      OP *new_op;
if (EBO_Trace_Optimization) fprintf(TFile,"replace and with complement and_i\n");
      new_op = Mk_OP(TOP_and_i, tnr, OP_opnd(op,OP_PREDICATE_OPND),
                     Gen_Literal_TN(~const_val, TN_size(tn0)), tn0);
      OP_srcpos(new_op) = OP_srcpos(op);
      if (EBO_in_loop) EBO_Set_OP_omega ( new_op, opnd_tninfo[OP_PREDICATE_OPND], NULL, opnd_tninfo[l0_opnd1_idx]);
      BB_Insert_Op_After(OP_bb(op), op, new_op);
      return TRUE;
    } else if (!EBO_in_loop) {
     /* Look for a mask in the constant and replace the andcm(tn,load-immediate) with an extr. */
     /* Note: We don't know exactly what will be generated by Expand_Special_And_Immed, so we  */
     /* don't know how big to make the new omega entry.                                        */
      OPS ops = OPS_EMPTY;
      if (Expand_Special_And_Immed (tnr, tn0, ~const_val, &ops)) {
if (EBO_Trace_Optimization) fprintf(TFile,"replace andcm with bit move\n");
        Set_OP_opnd(OPS_first(&ops), OP_PREDICATE_OPND, OP_opnd(op,OP_PREDICATE_OPND));
        OP_srcpos(OPS_first(&ops)) = OP_srcpos(op);
        BB_Insert_Ops(OP_bb(op), op, &ops, FALSE);
        return TRUE;
      }
    }
  }

  if ((opcode == TOP_or) &&
      (ISA_LC_Value_In_Class (const_val, LC_i8))) {
    OP *new_op;
if (EBO_Trace_Optimization) fprintf(TFile,"replace or with or_i\n");
    new_op = Mk_OP(TOP_or_i, tnr, OP_opnd(op,OP_PREDICATE_OPND),
                   Gen_Literal_TN(const_val, TN_size(tn0)), tn0);
    OP_srcpos(new_op) = OP_srcpos(op);
    if (EBO_in_loop) EBO_Set_OP_omega ( new_op, opnd_tninfo[OP_PREDICATE_OPND], NULL, opnd_tninfo[l0_opnd1_idx]);
    BB_Insert_Op_After(OP_bb(op), op, new_op);
    return TRUE;
  }

  if ((opcode == TOP_xor) &&
      (ISA_LC_Value_In_Class (const_val, LC_i8))) {
    OP *new_op;
if (EBO_Trace_Optimization) fprintf(TFile,"replace xor with xor_i\n");
    new_op = Mk_OP(TOP_xor_i, tnr, OP_opnd(op,OP_PREDICATE_OPND),
                   Gen_Literal_TN(const_val, TN_size(tn0)), tn0);
    OP_srcpos(new_op) = OP_srcpos(op);
    if (EBO_in_loop) EBO_Set_OP_omega ( new_op, opnd_tninfo[OP_PREDICATE_OPND], NULL, opnd_tninfo[l0_opnd1_idx]);
    BB_Insert_Op_After(OP_bb(op), op, new_op);
    return TRUE;
  }

  if ((TOP_shl == opcode) && (const_val <= 63)) {
if (EBO_Trace_Optimization) fprintf(TFile,"Convert shl to shl_i\n");
    OP *new_op;
    new_op = Mk_OP(TOP_shl_i, tnr, OP_opnd(op,OP_PREDICATE_OPND), tn0,
                   Gen_Literal_TN(const_val, TN_size(tn0)));
    OP_srcpos(new_op) = OP_srcpos(op);
    if (EBO_in_loop) EBO_Set_OP_omega ( new_op, opnd_tninfo[OP_PREDICATE_OPND], opnd_tninfo[l0_opnd1_idx], NULL);
    BB_Insert_Op_After(bb, op, new_op);
    return TRUE;
  }

  if ((TOP_shr == opcode) && (const_val <= 63)) {
if (EBO_Trace_Optimization) fprintf(TFile,"Convert shr to shr_i\n");
    OP *new_op;
    new_op = Mk_OP(TOP_shr_i, tnr, OP_opnd(op,OP_PREDICATE_OPND), tn0,
                   Gen_Literal_TN(const_val, TN_size(tn0)));
    OP_srcpos(new_op) = OP_srcpos(op);
    if (EBO_in_loop) EBO_Set_OP_omega ( new_op, opnd_tninfo[OP_PREDICATE_OPND], opnd_tninfo[l0_opnd1_idx], NULL);
    BB_Insert_Op_After(bb, op, new_op);
    return TRUE;
  }

  if ((TOP_shr_u == opcode) && (const_val <= 63)) {
if (EBO_Trace_Optimization) fprintf(TFile,"Convert shr_u to shr_i_u\n");
    OP *new_op;
    new_op = Mk_OP(TOP_shr_i_u, tnr, OP_opnd(op,OP_PREDICATE_OPND), tn0,
                   Gen_Literal_TN(const_val, TN_size(tn0)));
    OP_srcpos(new_op) = OP_srcpos(op);
    if (EBO_in_loop) EBO_Set_OP_omega ( new_op, opnd_tninfo[OP_PREDICATE_OPND], opnd_tninfo[l0_opnd1_idx], NULL);
    BB_Insert_Op_After(bb, op, new_op);
    return TRUE;
  }

  if (OP_icmp(op)) {
    INT64 new_const_val = const_val;
    TOP new_opcode = Immediate_Compare(Reverse_Compare(opcode));
    if (new_opcode == TOP_UNDEFINED) return FALSE;
    if (TOP_Can_Have_Immediate(new_const_val, new_opcode)) {
     /* Transform the compare to use the immediate form. */
      OPS ops = OPS_EMPTY;
      TN *r1 = OP_result(op,0);
      TN *r2 = OP_result(op,1);
      Build_OP (new_opcode, r1, r2, OP_opnd(op,OP_PREDICATE_OPND),
                Gen_Literal_TN(new_const_val, TN_size(tn0)), tn0, &ops);
      if (EBO_in_loop) EBO_Set_OP_omega (OPS_first(&ops), opnd_tninfo[OP_PREDICATE_OPND],
                                         NULL, opnd_tninfo[l0_opnd1_idx]);
      OP_srcpos(OPS_first(&ops)) = OP_srcpos(op);
      BB_Insert_Ops(OP_bb(op), op, &ops, FALSE);
if (EBO_Trace_Optimization) fprintf(TFile,"reverse operands and replace cmp with cmp_i\n");
      return TRUE;
    }
  }
#endif

 /* Sequence optimizations. */

  tninfo = opnd_tninfo[l0_opnd1_idx];
  if (tninfo == NULL) return FALSE;
  pred_op = tninfo->in_op;
  if (pred_op == NULL) return FALSE;
  pred_opcode = OP_code(pred_op);
  pred_opinfo = locate_opinfo_entry (tninfo);
  if (pred_opinfo == NULL) return FALSE;
#if 0
 /* First, look for some special cases involving loads. */
  if (OP_load(pred_op)) {
    if (OP_iand(op)) {
      if (((pred_opcode == TOP_ld1) &&
           (const_val == (~((~0) << 8)))) ||
          ((pred_opcode == TOP_ld2) &&
           (const_val ==  (~((~0) << 16))))) {
       /* No need to mask - it was done with the load. */
        OPS ops = OPS_EMPTY;
if (EBO_Trace_Optimization) fprintf(TFile,"no need to mask after load\n");
        EBO_Exp_COPY((OP_has_predicate(op)?OP_opnd(op,OP_PREDICATE_OPND):NULL),
                     tnr, tn0, &ops);
        if (EBO_in_loop) EBO_Set_OP_omega (OPS_first(&ops),
                                           OP_has_predicate(op)?opnd_tninfo[OP_PREDICATE_OPND]:NULL,
                                           opnd_tninfo[l0_opnd1_idx]);
        OP_srcpos(OPS_first(&ops)) = OP_srcpos(op);
        BB_Insert_Ops(bb, op, &ops, FALSE);
        return TRUE;
      }
    }
    return FALSE;
  }
#endif
 /* Now, look into optimizations that depend on specific
    values for the actual operands of the predecessor op. */
  if (TOP_Find_Operand_Use(OP_code(pred_op),OU_opnd2) >= 0) {
    TOP pred_opcode = OP_code(pred_op);
    INT ptn0_idx = TOP_Find_Operand_Use(pred_opcode,OU_opnd1);
    INT ptn1_idx = TOP_Find_Operand_Use(pred_opcode,OU_opnd2);
    TN *ptn0 = OP_opnd(pred_op, ptn0_idx);
    TN *ptn1 = OP_opnd(pred_op, ptn1_idx);
    EBO_TN_INFO *ptn0_tninfo;
    EBO_TN_INFO *ptn1_tninfo;
    INT64 pred_val = 0;

    OP *new_op;
    TOP new_opcode;
    UINT64 new_const_val;

    ptn0_tninfo = pred_opinfo->actual_opnd[ptn0_idx];
    ptn1_tninfo = pred_opinfo->actual_opnd[ptn1_idx];

   /* Are the operands available for use? */
    if (!(TN_Is_Constant(ptn0) || EBO_tn_available (bb, ptn0_tninfo)) ||
        !(TN_Is_Constant(ptn1) || EBO_tn_available (bb, ptn1_tninfo))) return FALSE;
    if (pred_opcode == TOP_spadjust) return FALSE;

    if (TN_Is_Constant(ptn1) && !TN_is_symbol(ptn1)) {
    } else if (TN_Is_Constant(ptn0) && !TN_is_symbol(ptn0) &&
               (OP_iadd(pred_op) || OP_ior(pred_op)  || OP_iand(pred_op))) {
      TN *s = ptn0;
      ptn0 = ptn1;
      ptn0_tninfo = ptn1_tninfo;
      ptn1 = s;
    } else return FALSE;

   /* Is the non-constant operand available for use? */
    if (!EBO_tn_available (bb, ptn0_tninfo)) {
      return FALSE;
    }

    pred_val = TN_Value(ptn1);

   /* Look for identical operations where the constants can be combined. */
#ifdef TARG_ST
    // We combine currently identical add/sub only for unrolled ops to avoid enlarging live ranges.
    if ((opcode == pred_opcode) &&
	OP_unrolling(op) &&
	(OP_iadd(op) || OP_isub(op))) {

      new_const_val = sext(pred_val + const_val,  TN_size(tnr)*8);
      new_opcode = TOP_opnd_immediate_variant(opcode, l0_opnd2_idx, new_const_val);

      if (new_opcode != TOP_UNDEFINED) {
	new_op = Mk_OP (new_opcode, tnr, ptn0, Gen_Literal_TN(new_const_val, TN_size(tnr)));
	OP_srcpos(new_op) = OP_srcpos(op);
	if (EBO_in_loop) EBO_Set_OP_omega ( new_op, opnd_tninfo[OP_PREDICATE_OPND], ptn0_tninfo, NULL);
	BB_Insert_Op_After(bb, op, new_op);
	if (EBO_Trace_Optimization) fprintf(TFile,"combine identical add/sub\n");
	return TRUE;
      }
    }

    if ((opcode == pred_opcode) &&
	(OP_ishl(op) || OP_ishr(op) || OP_ishru(op))) {
      INT op1_bits = TOP_opnd_use_bits(opcode, l0_opnd1_idx);
      INT op2_bits = TOP_opnd_use_bits(opcode, l0_opnd2_idx);
      const_val = ((UINT64)const_val << 64-op2_bits) >> 64-op2_bits;
      pred_val = ((UINT64)pred_val << 64-op2_bits) >> 64-op2_bits;
      new_const_val = const_val + pred_val;
      
      // Verify that we are in the known semantic domain
      if (new_const_val == ((UINT64)new_const_val << 64-op2_bits) >> 64-op2_bits) {
	new_opcode = TOP_opnd_immediate_variant(opcode, l0_opnd2_idx, new_const_val);
	if (new_opcode != TOP_UNDEFINED) {
	  new_op = Mk_OP (new_opcode, tnr, ptn0, Gen_Literal_TN(new_const_val, TN_size(tnr)));
	  OP_srcpos(new_op) = OP_srcpos(op);
	  if (EBO_in_loop) EBO_Set_OP_omega ( new_op, opnd_tninfo[OP_PREDICATE_OPND], ptn0_tninfo, NULL);
	  BB_Insert_Op_After(bb, op, new_op);
	  if (EBO_Trace_Optimization) fprintf(TFile,"combine identical shifts\n");
	  return TRUE;
	}
      }
    }

#else
    if ((opcode == pred_opcode) &&
        ((opcode == TOP_shl)    ||
         (opcode == TOP_shl_i)  ||
         (opcode == TOP_shr_u)  ||
         (opcode == TOP_shr_i_u))) {
      new_const_val = pred_val + const_val;

      if (new_const_val <= 63) {

        switch (opcode) {
        case TOP_shl:
        case TOP_shl_i: new_opcode = TOP_shl_i; break;
        case TOP_shr_u:
        case TOP_shr_i_u: new_opcode = TOP_shr_i_u; break;
        default: return FALSE;
        }

        new_op = Mk_OP (new_opcode, tnr, OP_opnd(op,OP_PREDICATE_OPND),
                        ptn0, Gen_Literal_TN(new_const_val, TN_size(tn0)));
        OP_srcpos(new_op) = OP_srcpos(op);
        if (EBO_in_loop) EBO_Set_OP_omega ( new_op, opnd_tninfo[OP_PREDICATE_OPND], ptn0_tninfo, NULL);
        BB_Insert_Op_After(bb, op, new_op);
if (EBO_Trace_Optimization) fprintf(TFile,"combine identical shifts\n");
        return TRUE;
      }
    }
#endif

    if (pred_val == const_val) {

      /* Look for identical operations that are redundant. */
      if ((OP_iand(op) && OP_iand(pred_op) && (const_val != ~0)) ||
          (OP_ior(op) && OP_ior(pred_op) && (const_val != 0))) {
        OPS ops = OPS_EMPTY;
	if (EBO_Trace_Optimization) fprintf(TFile,"replace redundant and/or\n");
        EBO_Exp_COPY((OP_has_predicate(op)?OP_opnd(op,OP_PREDICATE_OPND):NULL),
                     tnr, OP_result(pred_op, 0), &ops);
        if (EBO_in_loop) EBO_OPS_omega (&ops, 
					(OP_has_predicate(op)?OP_opnd(op,OP_PREDICATE_OPND):NULL),
					(OP_has_predicate(op)?opnd_tninfo[OP_PREDICATE_OPND]:NULL));
        OP_srcpos(OPS_first(&ops)) = OP_srcpos(op);
        BB_Insert_Ops(bb, op, &ops, FALSE);
        return TRUE;
      }

     /* Look for complementary operations. */
      if ((OP_iadd(op) && OP_isub(pred_op)) ||
          (OP_isub(op) && OP_iadd(pred_op))) {
       /* Copy the original input. */
        OPS ops = OPS_EMPTY;
	if (EBO_Trace_Optimization) fprintf(TFile,"replace complementary operations\n");
        EBO_Exp_COPY((OP_has_predicate(op)?OP_opnd(op,OP_PREDICATE_OPND):NULL),
                     tnr, ptn0, &ops);
        if (EBO_in_loop) EBO_Set_OP_omega (OPS_first(&ops), opnd_tninfo[OP_PREDICATE_OPND], ptn0_tninfo);
        OP_srcpos(OPS_first(&ops)) = OP_srcpos(op);
        BB_Insert_Ops(bb, op, &ops, FALSE);
        return TRUE;
      }
    
    } /* end : if (pred_val == const_val) */

  } /* end: Sequence optimizations. */
  
  return FALSE;

#ifdef TARG_ST
  OPS_Created:
  FmtAssert(OPS_first(&ops) != NULL, ("ops empty after simplify operand 0"));

  if (OP_next(OPS_first(&ops)) != NULL) {
    /* What's the point in replacing one instruction with several? */
    return FALSE;
  }

  /* No need to replace if same op, avoids infinite loops. */
  if (OPs_Are_Equivalent(op, OPS_first(&ops))) return FALSE;

  OP_srcpos(OPS_first(&ops)) = OP_srcpos(op);
  if (EBO_in_loop) {
    EBO_OPS_omega(&ops, (OP_has_predicate(op)? OP_opnd(op, OP_PREDICATE_OPND):NULL),
		  (OP_has_predicate(op)? opnd_tninfo[OP_PREDICATE_OPND]:NULL));
    EBO_OPS_omega(&ops, opnd_tn[l0_opnd1_idx], opnd_tninfo[l0_opnd1_idx]);
  }
  
  BB_Insert_Ops(OP_bb(op), op, &ops, FALSE);
  return TRUE;
#endif
}

/* =====================================================================
 *    EBO_Fold_Constant_Compare
 * =====================================================================
 */
BOOL
EBO_Fold_Constant_Compare (
  OP *op, 
  TN **opnd_tn,
  INT64 *result_val_ptr
)
{
  TOP opcode = OP_code(op);
  TN *tn0, *tn1;
  INT64 tn0_val, tn1_val;
  UINT64 tn0_uval, tn1_uval;
  INT op1_idx, op2_idx;
  INT64 result_val;

  /* Currently only handle integer compares. */
  if (!OP_icmp(op)) return FALSE;

  op1_idx = OP_find_opnd_use(op, OU_opnd1);
  op2_idx = OP_find_opnd_use(op, OU_opnd2);
  FmtAssert(op1_idx >= 0,("operand OU_opnd1 not defined for cmp TOP %s", TOP_Name(opcode)));
  FmtAssert(op2_idx >= 0,("operand OU_opnd2 not defined for cmp TOP %s", TOP_Name(opcode)));

  tn0 = opnd_tn[op1_idx];
  tn1 = opnd_tn[op2_idx];

  /* We could handle constant symbols (sush as floating point constant),
     but we don't. */
  if (TN_is_symbol(tn0) || TN_is_symbol(tn1)) return FALSE;
  

  tn0_val = TN_Value (tn0);
  tn1_val = TN_Value (tn1);
  
  // Then sign extend to compare bits
  INT bit_size = TOP_opnd_use_bits(opcode, op1_idx); // same as for op2_idx
  
  tn0_val = (tn0_val << 64-bit_size) >> 64-bit_size;
  tn1_val = (tn1_val << 64-bit_size) >> 64-bit_size;
  tn0_uval = (UINT64)(tn0_val << 64-bit_size) >> 64-bit_size;
  tn1_uval = (UINT64)(tn1_val << 64-bit_size) >> 64-bit_size;

  VARIANT variant = TOP_cmp_variant(opcode);

  switch (variant) {
  case V_CMP_EQ: result_val = (tn0_val == tn1_val); break;
  case V_CMP_NE: result_val = (tn0_val != tn1_val); break;
  case V_CMP_LT: result_val = (tn0_val < tn1_val); break;
  case V_CMP_GT: result_val = (tn0_val > tn1_val); break;
  case V_CMP_LE: result_val = (tn0_val <= tn1_val); break;
  case V_CMP_GE: result_val = (tn0_val >= tn1_val); break;
  case V_CMP_LTU: result_val = (tn0_uval < tn1_uval); break;
  case V_CMP_GTU: result_val = (tn0_uval > tn1_uval); break;
  case V_CMP_LEU: result_val = (tn0_uval <= tn1_uval); break;
  case V_CMP_GEU: result_val = (tn0_uval >= tn1_uval); break;
  case V_CMP_ANDL: result_val = (tn0_val != 0 && tn1_val != 0); break;
  case V_CMP_NANDL: result_val = !(tn0_val != 0 && tn1_val != 0); break;
  case V_CMP_ORL: result_val = (tn0_val != 0 || tn1_val != 0); break;
  case V_CMP_NORL: result_val = !(tn0_val != 0 || tn1_val != 0); break;
  default: return FALSE; break;
  }

  *result_val_ptr = result_val;
  return TRUE;
}


/* =====================================================================
 *    EBO_Fold_Constant_Expression
 *
 *    Look at an exression that has all constant operands and attempt to
 *    evaluate the expression.
 *
 *    Supported operations are:
 *      add, sub, mult, and, or, xor, nor, sll, srl, slt
 * =====================================================================
 */
BOOL
EBO_Fold_Constant_Expression (
  OP *op,
  TN **opnd_tn,
  EBO_TN_INFO **opnd_tninfo
)
{
  TOP opcode = OP_code(op);

  TN *tnr = OP_result(op,0);
  TN *tn0 = NULL;
  TN *tn1 = NULL;
  UINT64 tn0_uval = 0;
  INT64 tn0_val = 0;
  UINT64 tn1_uval = 0;
  INT64 tn1_val = 0;
  INT64 result_val;
  ST *result_sym = NULL;
  INT32 result_relocs;
  INT op1_idx, op2_idx;

  if (EBO_Trace_Execution) {
    INT i;
    INT opndnum = OP_opnds(op);
    fprintf(TFile, "%sin BB:%d Constant OP :- %s ",
            EBO_trace_pfx, BB_id(OP_bb(op)),TOP_Name(opcode));
    for (i = 0; i < opndnum; i++) {
      fprintf(TFile," ");
      Print_TN(opnd_tn[i],TRUE);
    }
    fprintf(TFile,"\n");
  }

  if (OP_has_predicate(op) && (opnd_tn[OP_PREDICATE_OPND] == Zero_TN)) {

    if (OP_Is_Unconditional_Compare(op)) {
      /* 
       * Unconditional compares define results even if the predicate 
       * is "FALSE". 
       */
      OPS ops = OPS_EMPTY;

#ifdef TARG_ST
      FmtAssert(FALSE,("TODO: not implemented"));
#else
      if (OP_result(op,0) != True_TN) {
	Build_OP (TOP_cmp_eq_unc, True_TN, OP_result(op,0), True_TN, Zero_TN, Zero_TN, &ops);
      }
      if (OP_result(op,1) != True_TN) {
	Build_OP (TOP_cmp_eq_unc, True_TN, OP_result(op,1), True_TN, Zero_TN, Zero_TN, &ops);
      }
#endif

      if (OP_glue(op)) {
	OP *next_op = OPS_first(&ops);
	while (next_op != NULL) {
	  Set_OP_glue(next_op);
	  next_op = OP_next(next_op);
	}
      }

      if (EBO_Trace_Optimization) {
	INT i;
	fprintf(TFile, "%sin BB:%d Result of compare is FALSE for all: ", EBO_trace_pfx,BB_id(OP_bb(op)));
	for (i = 0; i < OP_results(op); i++) {
	  Print_TN(OP_result(op,i),FALSE); fprintf(TFile,", ");
	}
      }

      if (OPS_length(&ops) != 0) {
	OP_srcpos(OPS_first(&ops)) = OP_srcpos(op);
	BB_Insert_Ops(OP_bb(op), op, &ops, FALSE);
      }
      return TRUE;
    }

    /* The OP can be deleted. */
    return (!OP_glue(op));
  }

  //
  // First of all, check if this is a target-specific opcode
  //
  if (EBO_Fold_Special_Opcode(op, opnd_tn, &result_val)) {
    goto Constant_Created;
  }

  // [CG]: use opnd1, opnd2 semantic to get effective operands
  // opnd2 may be undefined 
  op1_idx = OP_find_opnd_use(op, OU_opnd1);
  op2_idx = OP_find_opnd_use(op, OU_opnd2);
  if (op1_idx < 0) return FALSE;
  tn0 = opnd_tn[op1_idx];
  tn1 = op2_idx < 0 ? NULL : opnd_tn[op2_idx];

  if (TN_Has_Value(tn0) && (tn1 == NULL || TN_Has_Value(tn1))) {

    if (OP_iadd(op)) {
      result_val = TOP_fold_iadd(opcode, opnd_tn);
      goto Constant_Created;
    }

    else if (OP_isub(op)) {
      result_val = TOP_fold_isub(opcode, opnd_tn);
      goto Constant_Created;
    }

    else if (OP_imul(op)) {
      result_val = TOP_fold_imul(opcode, opnd_tn);
      goto Constant_Created;
    }

    else if (OP_iand(op)) {
      result_val = TOP_fold_and(opcode, opnd_tn);
      goto Constant_Created;
    }

    else if (OP_ior(op)) {
      result_val = TOP_fold_or(opcode, opnd_tn);
      goto Constant_Created;
    }

    else if (OP_ixor(op)) {
      result_val = TOP_fold_xor(opcode, opnd_tn);
      goto Constant_Created;
    }

    else if (OP_ishl(op)) {
      result_val = TOP_fold_shl(opcode, opnd_tn);
      goto Constant_Created;
    }
    
    else if (OP_ishr(op)) {
      result_val = TOP_fold_shr(opcode, opnd_tn);
      goto Constant_Created;
    }
    
    else if (OP_ishru(op)) {
      result_val = TOP_fold_shru(opcode, opnd_tn);
      goto Constant_Created;
    }
    
    else if (OP_zext(op)) {
      result_val = TOP_fold_zext(opcode, opnd_tn);
      goto Constant_Created;
    }

    else if (OP_sext(op)) {
      result_val = TOP_fold_sext(opcode, opnd_tn);
      goto Constant_Created;
    }

    else if (OP_icmp(op)) {
      result_val = TOP_fold_icmp(opcode, opnd_tn);
      goto Constant_Created;
    }

    else if (OP_select(op)) {
      INT64 op1 = TOP_fetch_opnd(opcode, opnd_tn, op1_idx);
      INT64 op2 = TOP_fetch_opnd(opcode, opnd_tn, op2_idx);
      if (op1 == op2) {
	result_val = op1;
	goto Constant_Created;
      }
    }
  
  } else if (TN_is_symbol(tn0) || TN_is_symbol(tn1)) {
    if (TN_is_symbol(tn0) && TN_is_symbol(tn1)) return FALSE;
    if (OP_iadd(op) || OP_isub(op)) {
      if (OP_iadd(op) && TN_is_symbol(tn1)) { TN *tmp = tn0; tn0 = tn1; tn1 = tmp; }
      if (TN_is_symbol(tn1)) return FALSE;
      tn0_val = TN_offset(tn0);
      tn0_val = sext(tn0_val, TOP_opnd_use_bits(opcode, op1_idx));
      tn1_val = sext(TN_Value(tn1), TOP_opnd_use_bits(opcode, op1_idx));
      result_val = OP_iadd(op) ? tn0_val + tn1_val : tn0_val - tn1_val;
      result_sym = TN_var(tn0);
      result_relocs = TN_relocs(tn0);
      goto Constant_Created;
    }
  }


  return FALSE;

Constant_Created:

  OPS ops = OPS_EMPTY;
  TN *tnc;

  result_val = sext(result_val, TN_size(tnr)*8);

  if (result_sym != NULL) {
    tnc = Gen_Symbol_TN(result_sym, result_val, result_relocs);
  } else {
    tnc = Gen_Literal_TN(result_val, TN_size(tnr));
  }

  Expand_Immediate (tnr, tnc, OP_result_is_signed(op,0), &ops);
  if (OP_next(OPS_first(&ops)) != NULL) {
   /* What's the point in replacing one instruction with several? */
    return FALSE;
  }
  if (OP_has_predicate(op)) {
    EBO_OPS_predicate (OP_opnd(op, OP_PREDICATE_OPND), &ops);
  }

  /* No need to replace if same op, avoids infinite loops. */
  if (OPs_Are_Equivalent(op, OPS_first(&ops))) return FALSE;

  if (EBO_in_loop) EBO_OPS_omega (&ops, 
				  (OP_has_predicate(op)? OP_opnd(op,OP_PREDICATE_OPND):NULL),
				  (OP_has_predicate(op) ? opnd_tninfo[OP_PREDICATE_OPND] : NULL));
  OP_srcpos(OPS_first(&ops)) = OP_srcpos(op);

  BB_Insert_Ops(OP_bb(op), op, &ops, FALSE);

  if (EBO_Trace_Optimization) {
    #pragma mips_frequency_hint NEVER
    fprintf(TFile, "%sin BB:%d Redefine ",
            EBO_trace_pfx, BB_id(OP_bb(op)));
    Print_TN(tnr,TRUE);
    fprintf(TFile," with load of ");
    Print_TN(tnc,FALSE);
    fprintf(TFile, "\n");
  }

  return TRUE;
}

#endif /* TARG_ST */

/* =====================================================================
 *   find_duplicate_mem_op
 *
 * For a given memory op, look for a preceeding memory op to 
 * the same location and attempt to replace one of them.
 * Return TRUE if this memory op is no longer needed.
 * =====================================================================
 */
static BOOL
find_duplicate_mem_op (BB *bb,
                       OP *op,
                       TN **opnd_tn,
                       EBO_TN_INFO **opnd_tninfo,
                       EBO_TN_INFO **actual_tninfo)
/* -----------------------------------------------------------------------
 * Requires: 
 * Returns TRUE if the operands of each OP are identical.
 * -----------------------------------------------------------------------
 */
{
  INT hash_value = 0;
  INT hash_search_length = 0;
  EBO_OP_INFO *opinfo;
  EBO_OP_INFO *intervening_opinfo = NULL;
  EBO_OP_INFO *adjacent_location = NULL;
  INT64 adjacent_offset_pred;
  INT64 adjacent_offset_succ;

  if (op == NULL) return FALSE;

  if (EBO_Trace_Execution) {
    #pragma mips_frequency_hint NEVER
    fprintf(TFile,"%sEnter find_duplicate_mem_op\n",EBO_trace_pfx);
  }
  
  if (!(OP_load(op) || OP_store(op))) return FALSE;

 /* Determine the indexes of the address components of this memory op. */
  INT succ_base_idx = TOP_Find_Operand_Use(OP_code(op),OU_base);
  INT succ_offset_idx = TOP_Find_Operand_Use(OP_code(op),OU_offset);

#ifndef TARG_ST
  // [CG]: Obsolete
  if ((succ_base_idx >= 0) && (succ_offset_idx >= 0) &&
      TN_Is_Constant(opnd_tn[succ_offset_idx])) {
   /* Look for merge-able expressions. */
    merge_memory_offsets (op, succ_base_idx, opnd_tn, actual_tninfo, opnd_tninfo);
  }
#endif

 /* Determine the address components of this memory op. */
  TN *succ_base_tn = (succ_base_idx >= 0) ? opnd_tn[succ_base_idx] : NULL;
  EBO_TN_INFO *succ_base_tninfo = (succ_base_idx >= 0) ? opnd_tninfo[succ_base_idx] : NULL;
  TN *succ_offset_tn = (succ_offset_idx >= 0) ? opnd_tn[succ_offset_idx] : NULL;
  EBO_TN_INFO *succ_offset_tninfo = (succ_offset_idx >= 0) ? opnd_tninfo[succ_offset_idx] : NULL;

  if ((succ_offset_tn == NULL) && (succ_base_tn != NULL)) {
    find_index_and_offset(succ_base_tninfo,
                          &succ_base_tn, &succ_base_tninfo,
                          &succ_offset_tn, &succ_offset_tninfo);
  }

 /* Determine the proper hash value. */
  hash_value = EBO_hash_op( op, opnd_tninfo);

  if (EBO_Trace_Hash_Search) {
    #pragma mips_frequency_hint NEVER
    fprintf(TFile,"%sLook for redundant memory ops in hash chain %d for\n\t",
                  EBO_trace_pfx,hash_value);
    Print_OP_No_SrcLine(op);
  }

  opinfo = (OP_has_implicit_interactions(op) || OP_unalign_mem(op)) ? NULL : EBO_opinfo_table[hash_value];

  while (opinfo) {
    OP *pred_op = opinfo->in_op;
    INT64 offset_pred = 0;
    INT64 offset_succ = 0;

   /* Identify the address components of the predecessor memory op. */
    INT pred_base_idx = 0;
    INT pred_offset_idx = 0;
    TN *pred_base_tn = NULL;
    EBO_TN_INFO *pred_base_tninfo = NULL;
    TN *pred_offset_tn = NULL;
    EBO_TN_INFO *pred_offset_tninfo = NULL;

    if ((pred_op != NULL) &&
        (OP_load(pred_op) || OP_store(pred_op))) {
     /* Determine the address components of the predecessor memory op. */
      pred_base_idx = TOP_Find_Operand_Use(OP_code(pred_op),OU_base);
      pred_offset_idx = TOP_Find_Operand_Use(OP_code(pred_op),OU_offset);
      pred_base_tn = (pred_base_idx >= 0) ? OP_opnd(pred_op,pred_base_idx) : NULL;
      pred_base_tninfo = (pred_base_idx >= 0) ? opinfo->optimal_opnd[pred_base_idx] : NULL;
      pred_offset_tn = (pred_offset_idx >= 0) ? OP_opnd(pred_op,pred_offset_idx) : NULL;
      pred_offset_tninfo = (pred_offset_idx >= 0) ? opinfo->optimal_opnd[pred_offset_idx] : NULL;
    }

    if ((pred_offset_tn == NULL) && (pred_base_tn != NULL)) {
      find_index_and_offset(pred_base_tninfo,
                            &pred_base_tn, &pred_base_tninfo,
                            &pred_offset_tn, &pred_offset_tninfo);
    }

    BOOL hash_op_matches = ((pred_op != NULL) &&
                            OP_memory(pred_op) &&
                            (pred_base_tn == succ_base_tn) &&           /* The base  index must match */
                            (pred_base_tninfo == succ_base_tninfo) &&   /* The base   info must match */
                            (pred_offset_tninfo == succ_offset_tninfo)) /* The offset info must match */
                            ? TRUE : FALSE;
    BOOL op_is_subset = FALSE;
    BOOL offsets_may_overlap = TRUE;

    hash_search_length++;

    if (hash_op_matches &&
        (pred_offset_tn != succ_offset_tn)) {
     /* The offset tn's need to be looked at in more detail. */

      ST *symbol_pred = ((pred_offset_tn != NULL) && TN_is_symbol(pred_offset_tn)) ?TN_var(pred_offset_tn) : NULL;
      ST *symbol_succ = ((succ_offset_tn != NULL) && TN_is_symbol(succ_offset_tn)) ?TN_var(succ_offset_tn) : NULL;
      mUINT8 relocs_pred = (pred_offset_tn != NULL) ? TN_relocs(pred_offset_tn) : 0;
      mUINT8 relocs_succ = (succ_offset_tn != NULL) ? TN_relocs(succ_offset_tn) : 0;
      offset_pred = (pred_offset_tn != NULL) ? TN_offset(pred_offset_tn) : 0;
      offset_succ = (succ_offset_tn != NULL) ? TN_offset(succ_offset_tn) : 0;

     /* This time, the relocations must be the same. */
      hash_op_matches = (symbol_pred == symbol_succ) && (relocs_pred == relocs_succ);

      if ((OP_prefetch(op) || OP_prefetch(pred_op)) &&
          (offset_pred != offset_succ)) {
        hash_op_matches = FALSE;
      } else if (hash_op_matches) {
       /* If the relocations are the same, we need to examine the offsets and sizes. */
#ifdef TARG_ST
        INT size_pred = OP_Mem_Ref_Bytes(pred_op);
        INT size_succ = OP_Mem_Ref_Bytes(op);
#else
        INT size_pred = CGTARG_Mem_Ref_Bytes(pred_op);
        INT size_succ = CGTARG_Mem_Ref_Bytes(op);
#endif

        if ((offset_pred == offset_succ) &&
            (size_pred == size_succ)) {
         /* The perfect match: location and size. */
        }  else if ((offset_pred <= offset_succ) &&
                    ((offset_pred + size_pred) >= (offset_succ + size_succ))) {
         /* The current reference is a subset of the preceeding one. */
          op_is_subset = TRUE;
        } else if (OP_load(op) && OP_load(pred_op) && (size_pred == size_succ) &&
                   (((offset_pred + size_pred) == offset_succ) ||
                    ((offset_succ + size_succ) == offset_pred))) {
          offsets_may_overlap = FALSE;
          if (adjacent_location == NULL) {
            adjacent_location = opinfo;
            adjacent_offset_pred = offset_pred;
            adjacent_offset_succ = offset_succ;
          }
        } else if (((offset_pred + size_pred) <= offset_succ) ||
                   ((offset_succ + size_succ) <= offset_pred)) {
         /* There is no potential overlap. */
          offsets_may_overlap = FALSE;
        } else {
         /* Any other case may be a potential conflict. */
          hash_op_matches = FALSE;
        }
      }
    }

    if (hash_op_matches && !offsets_may_overlap ) {
      if (EBO_Trace_Hash_Search) {
        #pragma mips_frequency_hint NEVER
        fprintf(TFile,"%sMemory overlap not possible with\n\t",
                      EBO_trace_pfx);
        Print_OP_No_SrcLine(pred_op);
      }
      opinfo = opinfo->same;
      continue;
    }

    if (hash_op_matches && OP_has_predicate(op)) {
     /* Check predicates for safety. */

      if (OP_store(op) && OP_store(pred_op)) {
        if (!EBO_predicate_dominates(OP_opnd(op,OP_PREDICATE_OPND),
                                     actual_tninfo[OP_PREDICATE_OPND],
                                     OP_opnd(pred_op,OP_PREDICATE_OPND),
                                     opinfo->optimal_opnd[OP_PREDICATE_OPND]) &&
            !EBO_predicate_complements(OP_opnd(op,OP_PREDICATE_OPND),
                                       actual_tninfo[OP_PREDICATE_OPND],
                                       OP_opnd(pred_op,OP_PREDICATE_OPND),
                                       opinfo->optimal_opnd[OP_PREDICATE_OPND])) {

          if (EBO_Trace_Hash_Search) {
            #pragma mips_frequency_hint NEVER
            fprintf(TFile,"%sStore predicates do not match\n\t", EBO_trace_pfx);
            Print_OP_No_SrcLine(pred_op);
          }

         /* We need to be extra safe with stores.  Assume a conflict. */
          hash_op_matches = FALSE;
          break;
        }
      } else if (!OP_store(op) && !OP_store(pred_op)) {
        if ((intervening_opinfo != NULL) &&
            !EBO_predicate_dominates(OP_opnd(pred_op,OP_PREDICATE_OPND),
                                     opinfo->optimal_opnd[OP_PREDICATE_OPND],
                                     OP_opnd(op,OP_PREDICATE_OPND),
                                     actual_tninfo[OP_PREDICATE_OPND])) {

          if (EBO_Trace_Hash_Search) {
            #pragma mips_frequency_hint NEVER
            fprintf(TFile,"%sRe-load predicates do not match\n\t", EBO_trace_pfx);
            Print_OP_No_SrcLine(pred_op);
          }

         /* A mixture of predicates on the loads may cause problems. */
          hash_op_matches = FALSE;
          break;
        }
        if (!EBO_predicate_dominates(OP_opnd(pred_op,OP_PREDICATE_OPND),
                                     opinfo->optimal_opnd[OP_PREDICATE_OPND],
                                     OP_opnd(op,OP_PREDICATE_OPND),
                                     actual_tninfo[OP_PREDICATE_OPND]) &&
            !EBO_predicate_complements(OP_opnd(op,OP_PREDICATE_OPND),
                                       actual_tninfo[OP_PREDICATE_OPND],
                                       OP_opnd(pred_op,OP_PREDICATE_OPND),
                                       opinfo->optimal_opnd[OP_PREDICATE_OPND])) {

          if (EBO_Trace_Hash_Search) {
            #pragma mips_frequency_hint NEVER
            fprintf(TFile,"%sLoad predicates do not match\n\t", EBO_trace_pfx);
            Print_OP_No_SrcLine(pred_op);
          }

         /* We can't use this load, but we can keep looking. */
          hash_op_matches = FALSE;

        }
      } else if (!OP_store(op) && OP_store(pred_op)) {
        if (!EBO_predicate_dominates(OP_opnd(pred_op,OP_PREDICATE_OPND),
                                     opinfo->optimal_opnd[OP_PREDICATE_OPND],
                                     OP_opnd(op,OP_PREDICATE_OPND),
                                     actual_tninfo[OP_PREDICATE_OPND]) &&
            !EBO_predicate_complements(OP_opnd(op,OP_PREDICATE_OPND),
                                       actual_tninfo[OP_PREDICATE_OPND],
                                       OP_opnd(pred_op,OP_PREDICATE_OPND),
                                       opinfo->optimal_opnd[OP_PREDICATE_OPND])) {

          if (EBO_Trace_Hash_Search) {
            #pragma mips_frequency_hint NEVER
            fprintf(TFile,"%sStore/load predicates do not match\n\t", EBO_trace_pfx);
            Print_OP_No_SrcLine(pred_op);
          }

         /* We need to be extra safe with stores.  Assume a conflict. */
          hash_op_matches = FALSE;
          break;
        }
      }
    }

    if ((pred_op != NULL) &&
        (hash_value == EBO_DEFAULT_MEM_HASH) &&
        (OP_store(pred_op) != OP_store(op)) &&
        (!OP_prefetch(op))) {
     /* Need to be careful about alias issues. */
      WN *pred_wn;
      WN *succ_wn;
      ALIAS_RESULT result;

      if (EBO_Trace_Hash_Search) {
        #pragma mips_frequency_hint NEVER
        fprintf(TFile,"%sPotential Alias problem with\n\t",EBO_trace_pfx);
        Print_OP_No_SrcLine(pred_op);
      }

#ifdef CG_DEP_ALIAS
      {
	{
	  BOOL definite;
	  int aliased = CG_DEP_Mem_Ops_Alias(pred_op, op, &definite);
          result = aliased ? POSSIBLY_ALIASED: NOT_ALIASED;
          result = definite ? SAME_LOCATION: result;
#else
      result = POSSIBLY_ALIASED;
      if (Alias_Manager != NULL) {
        pred_wn = OP_hoisted(pred_op) ? NULL : Get_WN_From_Memory_OP(pred_op);
        succ_wn = OP_hoisted(op) ? NULL : Get_WN_From_Memory_OP(op);
        if ((pred_wn != NULL) && (succ_wn != NULL)) {
          result = Aliased(Alias_Manager, pred_wn, succ_wn);
#endif
          if ((!hash_op_matches) && (result == SAME_LOCATION)) {
           /* This also implies that the size of the items is the same. */
            hash_op_matches = TRUE;
            if (EBO_Trace_Hash_Search) {
              #pragma mips_frequency_hint NEVER
              fprintf(TFile,"%sAlias_Manager identifies SAME_LOCATION\n",
                             EBO_trace_pfx);    
            }
           /* The Alias_Manager may think that the locations are the same,
              but we know that they may not be.  This is because if-conversion
              may create something called a "black hole".  It is hard to believe
              that the optimizations we do will be OK in this situation. */
	    /* CG: TODO mark OP as conditional store to handle this case. */
            if (OP_store(op)) opinfo->op_must_not_be_moved = TRUE;
            break;
          }
        }
      }

      if ((result == POSSIBLY_ALIASED) && (!hash_op_matches)) {
 
        if ((intervening_opinfo == NULL) && OP_store(pred_op) && OP_load(op)) {
          intervening_opinfo = opinfo;
          if (EBO_Trace_Hash_Search) {
            #pragma mips_frequency_hint NEVER
            fprintf(TFile,"%sContinue looking for duplicate memory op across interving dependency %d.\n",
                          EBO_trace_pfx,result);
          }
          goto do_next;
        }

        if (OP_store(pred_op)) opinfo->op_must_not_be_removed = TRUE;
        if (EBO_Trace_Hash_Search) {
          #pragma mips_frequency_hint NEVER
          fprintf(TFile,"%sAlias information(%d) prevents us from continuing.\n",
                        EBO_trace_pfx,result);
        }
        break;
      }

      if (hash_op_matches && op_is_subset &&
          OP_store(op) && OP_store(pred_op)) {
        opinfo->op_must_not_be_removed = TRUE;
        if (EBO_Trace_Hash_Search) {
          #pragma mips_frequency_hint NEVER
          fprintf(TFile,"%sOverlapping store prevents us from continuing.\n",
                        EBO_trace_pfx);
        }
        break;
      }

      if ((intervening_opinfo != NULL) && (!hash_op_matches)) {
        opinfo->op_must_not_be_removed = TRUE;
        if (EBO_Trace_Hash_Search) {
          #pragma mips_frequency_hint NEVER
          fprintf(TFile,"%sMultiple possible intervening stores prevents us from continuing.\n",
                        EBO_trace_pfx);
        }
        break;
      }

      if (EBO_Trace_Hash_Search) {
        #pragma mips_frequency_hint NEVER
        fprintf(TFile,"%sAlias information allows us to continue.\n",EBO_trace_pfx);
      }
    }

    if (in_delay_slot && !OP_store(op)) {
     /* In general, we can't remove an OP from a delay slot, but
        duplicate stores are removed by noop'ing the pred_op. */
      break;
    }

    if (hash_op_matches && 
        (pred_op != NULL) &&
        (OP_load(pred_op) ||
         (OP_store(pred_op) && !OP_store(op)))) {

      if (OP_store(pred_op)) {
        INT pred_stored_idx = TOP_Find_Operand_Use(OP_code(pred_op),OU_storeval);
        TN *pred_tn = OP_opnd(pred_op,pred_stored_idx);
        if (!TN_Is_Constant(pred_tn)) {
         /* The stored register needs to be available at this point. */
          if (!EBO_tn_available(bb,opinfo->actual_opnd[pred_stored_idx])) {
            opinfo->op_must_not_be_removed = TRUE;
            if (EBO_Trace_Hash_Search) {
              #pragma mips_frequency_hint NEVER
              fprintf(TFile,"%sMemory match found, but stored value is not available\n\t",
                            EBO_trace_pfx);
              Print_OP_No_SrcLine(pred_op);
            }
            break;
          }
        }
      } else {
        TN *pred_tn = OP_result(pred_op,0);
        if (!TN_Is_Constant(pred_tn)) {
         /* The previous result needs to be available at this point. */
          if (!EBO_tn_available(bb,opinfo->actual_rslt[0])) {
            if (EBO_Trace_Hash_Search) {
              #pragma mips_frequency_hint NEVER
              fprintf(TFile,"%sMemory match found, but loaded value is not available\n\t",
                            EBO_trace_pfx);
              Print_OP_No_SrcLine(pred_op);
            }
            break;
          }
        }
      }
    }

    if (hash_op_matches) {
      BOOL op_replaced = FALSE;

      if (OP_volatile(pred_op)) {
       /* If we match a volatile memory op, this
          one should have been volatile, too. */
        break;
      }

      if (EBO_Trace_Hash_Search) {
        #pragma mips_frequency_hint NEVER
        fprintf(TFile,"%sMatch after searching %d items on hash chain %d\n",
                      EBO_trace_pfx,hash_search_length,hash_value);
      }

      if (intervening_opinfo != NULL) {
        op_replaced = EBO_delete_reload_across_dependency (op, opnd_tninfo, opinfo, intervening_opinfo);
      } else if (op_is_subset) {
        op_replaced = EBO_delete_subset_mem_op (op, opnd_tninfo, opinfo, offset_pred, offset_succ);
      } else {
        op_replaced = EBO_delete_duplicate_op (op, opnd_tninfo, opinfo);
      }

      if (op_replaced) {
        return TRUE;
      } else {
       /* If we matched once and failed to eliminate it,
          we may need to keep both around. */
        if (intervening_opinfo != NULL) {
          opinfo = intervening_opinfo;
          pred_op = intervening_opinfo->in_op;
        }
        if (OP_store(op)) opinfo->op_must_not_be_moved = TRUE;
        if (op_is_subset || (hash_value == EBO_DEFAULT_MEM_HASH)) {
          if (OP_store(pred_op)) opinfo->op_must_not_be_removed = TRUE;
          break;
        }
        if (OP_store(pred_op) || OP_store(op)) {
          break;
        }
      }
    }

do_next:
  opinfo = opinfo->same;
  }

  if (adjacent_location != NULL) {
    BOOL op_replaced = EBO_combine_adjacent_loads (op, 
						   opnd_tninfo, 
						   adjacent_location,
						   adjacent_offset_pred, 
						   adjacent_offset_succ);
    if (op_replaced) {
      return TRUE;
    }
  }

  if (EBO_Trace_Hash_Search) {
    #pragma mips_frequency_hint NEVER
    fprintf(TFile,"%sFail after searching %d items on memory hash chain %d\n",
                  EBO_trace_pfx,hash_search_length,hash_value);
  }

  return FALSE;
}




/* 
 * find_duplicate_op
 *
 * For a given expression op, look for a preceeding indentical
 * expressionn and attempt to replace the new one.
 * Return TRUE if this expression is no longer needed.
 */
static BOOL
find_duplicate_op (BB *bb,
                   OP *op,
                   TN **opnd_tn,
                   EBO_TN_INFO **opnd_tninfo,
                   EBO_TN_INFO **actual_tninfo)
/* -----------------------------------------------------------------------
 * Requires: 
 * Returns TRUE if the operands of each OP are identical.
 * -----------------------------------------------------------------------
 */
{
  INT opcount;
  INT opndnum;
  INT hash_value = 0;
  INT hash_search_length = 0;
  EBO_OP_INFO *opinfo;
  BOOL hash_op_matches = FALSE;

  if (op == NULL) return FALSE;

  if (EBO_Trace_Execution) {
    #pragma mips_frequency_hint NEVER
    fprintf(TFile,"%sEnter find_duplicate_op\n",EBO_trace_pfx);
  }
  
  opcount = OP_opnds(op);

  if (OP_memory(op)) return FALSE;

  if (!OP_results(op)) return FALSE;

 /* Compute a hash value for the OP. */
  hash_value = EBO_hash_op( op, opnd_tninfo);

  if (EBO_Trace_Hash_Search) {
    #pragma mips_frequency_hint NEVER
    fprintf(TFile,"%sLook for duplicates at hash value %d of\n\t",EBO_trace_pfx,hash_value);
    Print_OP_No_SrcLine(op);
  }

  opinfo = (opcount == 0) ? NULL : EBO_opinfo_table[hash_value];

  while (opinfo) {
    OP *pred_op = opinfo->in_op;

    hash_search_length++;
    hash_op_matches =    (pred_op != NULL)
                      && (OP_results(op) == OP_results(pred_op))
		      && (OP_opnds(op) == OP_opnds(pred_op))
		      && (OP_results(op) == 2 || OP_code(op) == OP_code(pred_op));

    if (hash_op_matches) {

      for (opndnum = 0; opndnum < opcount; opndnum++) {
        if (OP_has_predicate(op) && (opndnum == OP_PREDICATE_OPND)) {
         /* Check predicates later. */
          continue;
        }
        if (opinfo->optimal_opnd[opndnum] == opnd_tninfo[opndnum]) {
          if (!TN_Is_Constant(opnd_tn[opndnum])) {
           /* If operands are not constants and the tninfo_entries match,
              then the values represented must also be identical. */
            continue;
          }

         /* Items that are constant (i.e. have a NULL tninfo_entry pointer)
            must be checked to verify that the constants are the same.
            Note that there are several "reasonable" combinations that
            can come up:
              1. The constants have identical TNs.
              2. The OPs have identical TNs and the predecessor hasn't changed.
              3. The TNs have been resolved to the same constant, but the
                 operand of the OP (for some reason) could not be changed to
                 reference a constant.
         */
          if (/* case 1 */(opnd_tn[opndnum] == OP_opnd(pred_op, opndnum)) ||
              /* case 2 */((OP_opnd(op, opndnum) == OP_opnd(pred_op, opndnum)) &&
                           EBO_tn_available(bb, opinfo->actual_opnd[opndnum])) ||
              /* case 3 */((opinfo->actual_opnd[opndnum] != NULL) && 
                           (opnd_tn[opndnum] == opinfo->actual_opnd[opndnum]->replacement_tn))) {
            continue;
          }
        }

       /* Operands don't match - get out of inner loop and try next OP. */
        hash_op_matches = FALSE;
        break;
      }

    }

    if (hash_op_matches && 
        (pred_op != NULL)) {
      int resnum;

      for (resnum = 0; resnum < OP_results(op); resnum++) {
       /* All of the results need to be available at this point. */
        if (!TN_is_const_reg(OP_result(op,resnum)) &&
            !EBO_tn_available(bb,opinfo->actual_rslt[resnum])) {

          if (EBO_Trace_Hash_Search) {
            #pragma mips_frequency_hint NEVER
            fprintf(TFile,"%sExpression match found, but the result[%d] value is not available\n\t",
                           EBO_trace_pfx,resnum);
            Print_OP_No_SrcLine(pred_op);
          }

          hash_op_matches = FALSE;
          break;
        }
      }

    }

    if (in_delay_slot) {
     /* We can't insert or remove items from the delay slot. */
      break;
    }

    if (hash_op_matches && OP_has_predicate(op)) {
     /* Check predicates for safety. */

        if (!EBO_predicate_dominates(OP_opnd(pred_op,OP_PREDICATE_OPND),
                                     opinfo->optimal_opnd[OP_PREDICATE_OPND],
                                     OP_opnd(op,OP_PREDICATE_OPND),
                                     actual_tninfo[OP_PREDICATE_OPND]) &&
            !EBO_predicate_complements(OP_opnd(pred_op,OP_PREDICATE_OPND),
                                       opinfo->optimal_opnd[OP_PREDICATE_OPND],
                                       OP_opnd(op,OP_PREDICATE_OPND),
                                       actual_tninfo[OP_PREDICATE_OPND])) {
          hash_op_matches = FALSE;

          if (EBO_Trace_Hash_Search) {
            #pragma mips_frequency_hint NEVER
            fprintf(TFile,"%sExpression match found, but the predicates do not match\n\t",
                           EBO_trace_pfx);
            Print_OP_No_SrcLine(pred_op);
          }

        }
    }

    if (hash_op_matches) {

      if (EBO_Trace_Hash_Search) {
        #pragma mips_frequency_hint NEVER
        fprintf(TFile,"%sMatch after searching %d items on expression hash chain %d\n",
                      EBO_trace_pfx,hash_search_length,hash_value);
      }

      if (EBO_delete_duplicate_op (op, opnd_tninfo, opinfo)) {
        return TRUE;
      } else {
       /* If we matched once and failed to eliminate it,
          we need to keep both around. */
        break;
      }
    }

  opinfo = opinfo->same;
  }

  if (EBO_Trace_Hash_Search) {
    #pragma mips_frequency_hint NEVER
    fprintf(TFile,"%sFail after searching %d items on hash chain %d\n",
                  EBO_trace_pfx,hash_search_length,hash_value);
  }

  return FALSE;
}




/* 
 * find_previous_constant
 *
 * For a given expression op, look for a preceeding indentical
 * expressionn and attempt to replace the new one.
 * Return TRUE if this expression is no longer needed.
 */
static BOOL
find_previous_constant (OP *op,
                        EBO_TN_INFO **actual_tninfo)
{
  TN *const_tn = OP_opnd(op,copy_operand(op));
  EBO_TN_INFO *predicate_tninfo = (OP_has_predicate(op)?actual_tninfo[OP_PREDICATE_OPND]:NULL);
  EBO_TN_INFO *check_tninfo;

  if (!TN_is_constant(const_tn)) return FALSE;
  if (TN_is_const_reg(const_tn)) return FALSE;
  if (OP_cond_def(op)) return FALSE;

  if (EBO_Trace_Execution) {
    #pragma mips_frequency_hint NEVER
    fprintf(TFile,"%sEnter find_previous_constant %d\n",
                  EBO_trace_pfx,(INT32)TN_value(const_tn));
  }

  for (check_tninfo = EBO_last_tninfo;
       check_tninfo != NULL;
       check_tninfo = check_tninfo->prior) {
    if (check_tninfo->replacement_tn == const_tn) {
       /* The asigned register needs to be available at this point. */
      TN *pred_tn = check_tninfo->local_tn;
      OP *pred_op = check_tninfo->in_op;

      if (TN_register_class(OP_result(op, 0)) != TN_register_class(pred_tn)) {
        continue;
      }
      if ((pred_op != NULL) && OP_has_predicate(op) && OP_has_predicate(pred_op)) {
       /* Check predicates for safety. */
        EBO_OP_INFO *opinfo = locate_opinfo_entry(check_tninfo);
        if ((opinfo == NULL) ||
            !EBO_predicate_dominates(OP_opnd(pred_op,OP_PREDICATE_OPND),
                                     opinfo->optimal_opnd[OP_PREDICATE_OPND],
                                     OP_opnd(op,OP_PREDICATE_OPND),
                                     predicate_tninfo)) {
          if ((OP_code(op) == OP_code(pred_op)) &&
              EBO_predicate_complements(OP_opnd(pred_op,OP_PREDICATE_OPND),
                                     opinfo->optimal_opnd[OP_PREDICATE_OPND],
                                     OP_opnd(op,OP_PREDICATE_OPND),
                                     predicate_tninfo) &&
              EBO_delete_duplicate_op (op, actual_tninfo, opinfo)) {
            return TRUE;
          }

         /* This previous definition is not always available, keep looking. */
          continue;
        }
      }
      if (EBO_tn_available(OP_bb(op),check_tninfo) &&
          (TN_is_rematerializable(pred_tn))) {
          OPS ops = OPS_EMPTY;

          EBO_Exp_COPY((OP_has_predicate(op)?OP_opnd(op,OP_PREDICATE_OPND):NULL),
                       OP_result(op, 0), pred_tn, &ops);
          if (EBO_in_loop) {
            CG_LOOP_Init_Op(OPS_first(&ops));
#ifdef TARG_ST
	    // Can't pass the PREDICATE_OPND if no predication
#ifdef SUPPORTS_PREDICATION
	    Set_OP_omega (OPS_first(&ops),
			  OP_PREDICATE_OPND,
			  (predicate_tninfo != NULL)?predicate_tninfo->omega:0);
#endif
	    // IA64 always does this.
#else
	    Set_OP_omega (OPS_first(&ops),
                          OP_PREDICATE_OPND,
                          (predicate_tninfo != NULL)?predicate_tninfo->omega:0);
#endif
          }
          OP_srcpos(OPS_first(&ops)) = OP_srcpos(op);
          BB_Insert_Ops(OP_bb(op), op, &ops, FALSE);

          if (EBO_Trace_Optimization) {
            #pragma mips_frequency_hint NEVER
            fprintf(TFile,"%sAvoid reloading constant ",EBO_trace_pfx);
            Print_TN(const_tn,FALSE);
            fprintf(TFile," into ");
            Print_TN(OP_result(op, 0),FALSE);
            fprintf(TFile,"\n");
          }
        return TRUE;
      }
      return FALSE;
    }
  }

  return FALSE;
}

/* 
 * Iterate through a Basic Block and build EBO_TN_INFO entries.
 */
static BOOL
Find_BB_TNs (BB *bb)
{
  OP *op;
  BOOL no_barriers_encountered = TRUE;

  if (EBO_Trace_Execution) {
    #pragma mips_frequency_hint NEVER
    fprintf(TFile,"%sEnter Find_BB_TNs BB:%d%s\n",
            EBO_trace_pfx,BB_id(bb),EBO_in_peep?" - peep ":" ");
    Print_BB(bb);
  }

 /* Allocate the dynamic arrays for various operand info. The minimum
    size we allocate is large enough for all OPs with a fixed number
    of operands. If we just allocated the size based on the BB OP with the
    greatest number of operands, then a transformation could produce
    an OP with more operands. It is expected that these new OPs will 
    always have fixed operands, which is why we use OP_MAX_FIXED_OPNDS
    (we of course verify this assumption). */
  INT max_opnds = OP_MAX_FIXED_OPNDS;
  FOR_ALL_BB_OPs (bb, op) {
    INT nopnds = OP_opnds(op);
    if (nopnds > max_opnds) max_opnds = nopnds;
  }
  TN **opnd_tn = TYPE_ALLOCA_N(TN *, max_opnds);
  EBO_TN_INFO **opnd_tninfo = TYPE_ALLOCA_N(EBO_TN_INFO *, max_opnds);
  EBO_TN_INFO **orig_tninfo = TYPE_ALLOCA_N(EBO_TN_INFO *, max_opnds);

  in_delay_slot = FALSE;

  FOR_ALL_BB_OPs (bb, op) {
    INT opndnum;
    INT resnum;
    INT num_opnds = OP_opnds(op);
    TN *rslt_tn = NULL;
    INT rslt_num = 0;
    BOOL opnds_constant = TRUE;
    BOOL op_replaced = FALSE;
    BOOL op_is_predicated = OP_has_predicate(op)?TRUE:FALSE;
    TN *op_predicate_tn = NULL;
    EBO_TN_INFO *op_predicate_tninfo = NULL;
    BOOL check_omegas = (EBO_in_loop && _CG_LOOP_info(op))?TRUE:FALSE;

    /* 
     * The assumption is that this can never occur, but make sure it 
     * doesn't! 
     */
    FmtAssert(num_opnds <= max_opnds, ("dynamic array allocation was too small!"));

#ifdef TARG_ST
    // [CG]: Don't understand what that's for. Always set no_barriers_encountered to false.
    no_barriers_encountered = FALSE;
#else
#ifdef TARG_ST
    if (OP_Is_Barrier(op) || OP_access_reg_bank(op)) {
#else
    if (CGTARG_Is_OP_Barrier(op) || OP_access_reg_bank(op)) {
#endif
      if (EBO_Special_Sequence(op, NULL, NULL)) {
	/* We were able to restrict propagation of the specific registers. */
        if (EBO_Trace_Execution) {
          #pragma mips_frequency_hint NEVER
          fprintf(TFile,"%sWe were able to restrict propagation of the specific registers in BB:%d\t",
                  EBO_trace_pfx,BB_id(OP_bb(op)));
          Print_OP_No_SrcLine(op);
        }  
      } else {
       /* We could not identify the specific registers involved. */
        if (EBO_Trace_Execution) {
          #pragma mips_frequency_hint NEVER
          fprintf(TFile,"%sFind_BB_TNs Barrier OP encountered\t",EBO_trace_pfx);
          Print_OP_No_SrcLine(op);
        }
        no_barriers_encountered = FALSE;
      }
    }
#endif

#ifdef TARG_ST200
    if (OP_code(op) == TOP_asm) {
      ASM_OP_ANNOT* asm_info = (ASM_OP_ANNOT*) OP_MAP_Get(OP_Asm_Map, op);
      ISA_REGISTER_CLASS rc;
      FOR_ALL_ISA_REGISTER_CLASS(rc) {
	REGISTER_SET regset = ASM_OP_clobber_set(asm_info)[rc];
	for (REGISTER reg = REGISTER_SET_Choose(regset);
	     reg != REGISTER_UNDEFINED;
	     reg = REGISTER_SET_Choose_Next(regset, reg)) {
	  TN* ctn = Build_Dedicated_TN (rc, reg,0);
	  set_tn_info(ctn, NULL);

	  if (EBO_Trace_Data_Flow) {
	    fprintf(TFile,"%sASM clobbers tninfo for\n\t",EBO_trace_pfx); Print_TN(ctn,FALSE);fprintf(TFile,"\n");
	  }

	}
      }
    }
#endif /* TARG_ST200 */

    if ((num_opnds == 0) && (OP_results(op) == 0)) continue;

    if (EBO_Trace_Data_Flow) {
      fprintf(TFile,"%sProcess OP\n\t",EBO_trace_pfx); Print_OP_No_SrcLine(op);
    }

    /* Process all the operand TNs. */
    for (opndnum = 0; opndnum < num_opnds; opndnum++) {
      opnd_tn[opndnum] = NULL;
      opnd_tninfo[opndnum] = NULL;
      orig_tninfo[opndnum] = NULL;
    }

    for (opndnum = 0; opndnum < num_opnds; opndnum++) {
      BOOL replace_result = FALSE;
      TN *new_tn = NULL;
      EBO_TN_INFO *new_tninfo = NULL;
      TN *tn_replace = NULL;
      EBO_TN_INFO *tninfo_replace = NULL;
      TN *actual_tn = NULL;
      EBO_TN_INFO *actual_tninfo = NULL;
#ifdef TARG_ST
      TN *tn_copy = NULL;
      EBO_TN_INFO *tninfo_copy = NULL;
#endif

      actual_tn = OP_opnd(op, opndnum);
      actual_tninfo = NULL;

      opnd_tn[opndnum] = actual_tn;
      opnd_tninfo[opndnum] = actual_tninfo;
      orig_tninfo[opndnum] = actual_tninfo;

      if (actual_tn == NULL || TN_is_constant(actual_tn) || TN_is_label(actual_tn)) {
        continue;
      }

      if (actual_tn != True_TN) {
	mUINT8 operand_omega = check_omegas ? OP_omega(op,opndnum) : 0;
        actual_tninfo = tn_info_use (bb, op, actual_tn, op_predicate_tn, op_predicate_tninfo, operand_omega);
        tn_replace = actual_tninfo->replacement_tn;
	tninfo_replace = actual_tninfo->replacement_tninfo;
#ifdef TARG_ST
	tn_copy = actual_tninfo->copy_tn;
	tninfo_copy = actual_tninfo->copy_tninfo;
#endif
      }

      opnd_tninfo[opndnum] = actual_tninfo;
      orig_tninfo[opndnum] = actual_tninfo;

#ifdef TARG_ST
      /* Arthur: OP_same_res() return what is needed
      /*         This logic assumes that only one result matches 
       *         an operand. 
       */
      for (INT i = 0; i < OP_results(op); i++) {
	if (OP_same_res(op, i) == opndnum) {
	  rslt_num = i;
	  replace_result = TRUE;

	  if (EBO_Trace_Data_Flow) {
	    fprintf(TFile, "%sresult %d is the same as opnd %d\n", 
		                    EBO_trace_pfx, rslt_num, opndnum);
	  }
	}
      }
#else
      if (OP_same_res(op)) {
        INT i;
        for (i=0; i<OP_results(op); i++) {
          if (tn_registers_identical(tn, OP_result(op,i))) {
           /* This logic assumes that only one result matches an operand. */
            replace_result = TRUE;
            rslt_num = i;
            break;
          }
        }
      }
#endif

      if ((tn_replace != NULL) &&
          (TN_Is_Constant(tn_replace) ||
           EBO_tn_available(bb,tninfo_replace) ||
           ((tn_registers_identical(actual_tn, tn_replace)) && !check_omegas)) &&
          (TN_Is_Constant(tn_replace) ||
           ((tninfo_replace != NULL) &&
            (tninfo_replace->in_bb == bb)) ||
           ((has_assigned_reg(actual_tn) == has_assigned_reg(tn_replace)) &&
            (EBO_in_peep || (!BB_reg_alloc(bb) && !TN_is_dedicated(tn_replace))))) ) {
       /* The original TN can be "logically" replaced with another TN. */

        if (EBO_Trace_Data_Flow) {
          #pragma mips_frequency_hint NEVER
          fprintf(TFile, "%sin BB:%d propagate value for opnd[%d] ",
                  EBO_trace_pfx, BB_id(OP_bb(op)),opndnum);
          Print_TN(actual_tn, FALSE);
          fprintf(TFile," with ");
          Print_TN(tn_replace, FALSE);
          fprintf(TFile,"\n");
        }

	// [CG] Can logically replace
	new_tn = tn_replace;
	new_tninfo = tninfo_replace;

#ifdef TARG_ST
	// [CG]: if tn_replace is constant, it will not
	// be actually replaced, so use tn_copy instead if possible
	if (TN_is_constant(tn_replace)) {
	  if (tn_copy != NULL) {
	    FmtAssert(TN_is_register(tn_copy) && tninfo_copy != NULL, 
		      ("Invalid tn_copy in BB %d", BB_id(bb)));
	    /* Ensure:
	     * 1.(available) to ensure validity and 
	     * 2.(same bb or in peep) to not enlarge live ranges
	     */
	    if (EBO_tn_available(bb,tninfo_copy) &&
		(tninfo_copy->in_bb == bb || EBO_in_peep)
		) {
	      tn_replace = tn_copy;
	      tninfo_replace = tninfo_copy;
	    }
	  }
	}
#endif
        if (!TN_is_constant(tn_replace) &&
#ifndef TARG_ST
	    // [CG]: unecessary as now TN_has_spill is obsolete
            (!OP_store(op) ||
             (opndnum != TOP_Find_Operand_Use(OP_code(op),OU_storeval)) ||
             !TN_has_spill(actual_tn)) &&
#endif
            (!TN_save_reg(tn_replace)) &&
            (!replace_result || tn_registers_identical(actual_tn, tn_replace)) &&
            (EBO_in_peep ||
             (has_assigned_reg(actual_tn) == has_assigned_reg(tn_replace)) ||
             (TN_is_const_reg(tn_replace)) ||
             (!OP_copy(op))) &&
            (EBO_in_peep ||
             !TN_is_gra_homeable(tn_replace) ||
             (tninfo_replace->in_bb == bb)) &&
            (TN_register_class(actual_tn) == TN_register_class(tn_replace)) &&
#ifdef TARG_ST200
	    (!(has_assigned_reg(actual_tn) && OP_code(op) == TOP_asm)) &&
#endif
            (!has_assigned_reg(actual_tn) ||
             (ISA_OPERAND_VALTYP_Register_Subclass(ISA_OPERAND_INFO_Operand(ISA_OPERAND_Info(OP_code(op)),opndnum)) == ISA_REGISTER_SUBCLASS_UNDEFINED) ||
	     (has_assigned_reg(tn_replace) &&
		(REGISTER_SET_MemberP(REGISTER_SUBCLASS_members(ISA_OPERAND_VALTYP_Register_Subclass(ISA_OPERAND_INFO_Operand(ISA_OPERAND_Info(OP_code(op)),opndnum))), TN_register(tn_replace))))) &&
            (TN_size(actual_tn) <= TN_size(tn_replace)) &&
            (TN_is_float(actual_tn) == TN_is_float(tn_replace)) &&
            (TN_is_fpu_int(actual_tn) == TN_is_fpu_int(tn_replace)) &&
            ((OP_results(op) == 0) ||
#ifdef TARG_ST
	     // Arthur: the entire logic escapes me ??
	     !OP_uniq_res(op, 0) ||
#else
             !OP_uniq_res(op) ||
#endif
             !tn_registers_identical(tn_replace, OP_result(op,0))) ) {
         /* The original TN can be "physically" replaced with another TN. */
         /* Put the new TN in the expression,           */
         /* decrement the use count of the previous TN, */
         /* increment the use count of the new TN.      */

          if (EBO_Trace_Optimization) {
            #pragma mips_frequency_hint NEVER
            fprintf(TFile, "%sin BB:%d replace opnd[%d] ",
                    EBO_trace_pfx, BB_id(OP_bb(op)),opndnum);
            Print_TN(OP_opnd(op, opndnum), FALSE);
            fprintf(TFile," with ");
            Print_TN(tn_replace, FALSE);
            if (!TN_Is_Constant(tn_replace) &&
                (tninfo_replace != NULL) &&
                (tninfo_replace->in_bb != NULL)) {
              fprintf(TFile," from BB:%d",BB_id(tninfo_replace->in_bb));
            }
            fprintf(TFile,"\n");
          }

          dec_ref_count(actual_tninfo);
          Set_OP_opnd(op, opndnum, tn_replace);
          if (check_omegas) {
            Set_OP_omega (op, opndnum, (tninfo_replace != NULL) ? tninfo_replace->omega : 0);
          }

          if (tninfo_replace != NULL) {
            inc_ref_count(tninfo_replace);
          }

          if (replace_result) {
           /* This use is also the new result. */
            rslt_tn = tn_replace;
          }

	  /* Update information about the actual expression. */
	  actual_tn = tn_replace;
	  actual_tninfo = tninfo_replace;
        } /* replace the operand with another TN. */

        opnd_tn[opndnum] = new_tn;
        opnd_tninfo[opndnum] = new_tninfo;
        orig_tninfo[opndnum] = actual_tninfo;
      }  /* logically replace tn/tninfo. */

      FmtAssert(opnd_tn[opndnum] != NULL, ("Null opnd TN"));

      if (!TN_Is_Constant(opnd_tn[opndnum]) &&
          (!op_is_predicated || (opndnum != OP_PREDICATE_OPND))) {
        opnds_constant = FALSE;
      }

      if (op_is_predicated && (opndnum == OP_PREDICATE_OPND)) {
        if ((opnd_tn[opndnum] == Zero_TN) && !OP_xfer(op)) {
         /* 
	  * The instruction will not be executed - it can be deleted!
	  * However, Branch instructions should go through
	  * Resolve_Conditional_Branch so that links between blocks 
	  * can be updated. 
	  */
	  op_replaced = EBO_Fold_Constant_Expression (op, opnd_tn, opnd_tninfo);
          num_opnds = opndnum + 1;

          if (EBO_Trace_Optimization) {
            #pragma mips_frequency_hint NEVER
            fprintf(TFile, "%sin BB:%d Op can not execute with 0 predicate\n",
                    EBO_trace_pfx, BB_id(OP_bb(op)));
          }
          if (op_replaced) break;
        }

        op_predicate_tn = opnd_tn[opndnum];
        op_predicate_tninfo = opnd_tninfo[opndnum];
      }

    } /* End: Process all the operand TNs. */

#ifdef TARG_ST
    // Arthur: it looks inefficient but really the entire logic
    //         should be replaced. I need the same_res to replace
    //         the old OP_same_res(op)
    BOOL same_res = FALSE;
    for (INT i = 0; i < OP_results(op); i++) {
      if (OP_same_res(op,i) >= 0) {
	same_res = TRUE;
	break;
      }
    }
#endif

    if (OP_memory(op)) {
#ifdef TARG_ST
      if (!op_replaced && same_res)
#else
      if (!op_replaced && OP_same_res(op))
#endif
      {
        op_replaced = EBO_Fix_Same_Res_Op (op, opnd_tn, opnd_tninfo);
      }

#ifdef TARG_ST
      if (!op_replaced) {
	op_replaced = EBO_Memory_Sequence (op, opnd_tn, opnd_tninfo);
      }
#endif
      if (!op_replaced &&
          !in_delay_slot) {
        op_replaced = EBO_Special_Sequence ( op, opnd_tn, orig_tninfo);
      }
      if (!op_replaced &&
          no_barriers_encountered) {
        op_replaced = find_duplicate_mem_op (bb, op, opnd_tn, opnd_tninfo, orig_tninfo);
      }
    } else if (OP_effectively_copy(op)) {
      if (!op_replaced &&
          opnds_constant && !in_delay_slot) {
        op_replaced = find_previous_constant(op, opnd_tninfo);
      }
      if (!op_replaced) {
        op_replaced = EBO_Special_Sequence (op, opnd_tn, opnd_tninfo);
      }
    } else if (!op_replaced &&
               !OP_effectively_copy(op) &&
               !OP_glue(op) &&
               !OP_side_effects(op) &&
               !OP_access_reg_bank(op)) {
      if (!in_delay_slot) {
       /* Can we evaluate the expression at compile time? */
        if (opnds_constant && (num_opnds >= 1)) {
          if (OP_xfer(op)) {
           /* If we remove a conditional branch and alter the flow, we
              may have created dead code that could cause later processing
              to get into trouble. This needs to be looked into. */
            op_replaced = EBO_Resolve_Conditional_Branch (op, opnd_tn);
            rerun_cflow |= op_replaced;
          } else if (OP_results(op) >= 1) {
            op_replaced = EBO_Fold_Constant_Expression (op, opnd_tn, opnd_tninfo);
          }
        }
	if (!op_replaced && OP_results(op) > 0 && num_opnds > 1) {
	    /* Consider special case optimizations. */
#ifdef TARG_ST
	    // The ST target description takes care of this
            INT o1_idx = OP_find_opnd_use(op, OU_opnd1);
            INT o2_idx = OP_find_opnd_use(op, OU_opnd2);
#else
            INT o2_idx; /* TOP_Find_Operand_Use(OP_code(op),OU_opnd2) won't work for all the cases we care about */
            INT o1_idx; /* TOP_Find_Operand_Use(OP_code(op),OU_opnd1) won't work for all the cases we care about */
            if (op_is_predicated) {
              if (OP_fadd(op) || OP_fsub(op) || OP_fmul(op) || OP_fdiv(op)) {
               /* The first operand is not wnat we are looking for. */
                o1_idx = (num_opnds > 2) ? 2 : -1;
                o2_idx = (num_opnds > 3) ? 3 : -1;
              } else {
                o1_idx = (num_opnds > 1) ? 1 : -1;
                o2_idx = (num_opnds > 2) ? 2 : -1;
              }
            } else {
              o1_idx = (num_opnds > 0) ? 0 : -1;
              o2_idx = (num_opnds > 1) ? 1 : -1;
            }
#endif

#ifdef TARG_ST
            if (same_res) 
#else
            if (OP_same_res(op))
#endif
	    {
              op_replaced = EBO_Fix_Same_Res_Op (op, opnd_tn, opnd_tninfo);
            }
            if (!op_replaced && o2_idx >= 0) {
              TN *tn = opnd_tn[o2_idx];
              if (tn != NULL && TN_Has_Value(tn)) {
                op_replaced = EBO_Constant_Operand1 (op, opnd_tn, opnd_tninfo);
              }
            }
            if (!op_replaced && o1_idx >= 0) {
              TN *tn = opnd_tn[o1_idx];
              if (tn != NULL && TN_Has_Value(tn)) {
                op_replaced = EBO_Constant_Operand0 (op, opnd_tn, opnd_tninfo);
              }
            }
        }
        if (!op_replaced) {
          op_replaced = EBO_Special_Sequence (op, opnd_tn, orig_tninfo);
        }
#ifdef TARG_ST
	if (!op_replaced && OP_select(op))
	  op_replaced = EBO_Fix_Select_Same_Args(op, opnd_tn, opnd_tninfo);
#endif

      }

      if (no_barriers_encountered && !op_replaced && !OP_effectively_copy(op)) {
       /* Look for redundant OPs. */
        op_replaced = find_duplicate_op(bb, op, opnd_tn, opnd_tninfo, orig_tninfo);
      }

    }

    if (op_replaced) {

      if (EBO_Trace_Optimization) {
        fprintf(TFile,"%sin BB:%d remove simplified op - ",
		                                EBO_trace_pfx,BB_id(bb));
        Print_OP_No_SrcLine(op);
      }
      remove_uses (num_opnds, orig_tninfo);
      OP_Change_To_Noop(op);
    } else {
      /* Add this OP to the hash table and define all the result TN's. */
      add_to_hash_table (in_delay_slot, op, orig_tninfo, opnd_tninfo);

      FmtAssert(((EBO_last_opinfo != NULL) && (EBO_last_opinfo->in_op == op)),
                  ("OP wasn't added to hash table"));

      /* Special processing for the result TNs */
      resnum = OP_results(op);
      if (OP_effectively_copy(op) || 
	  (resnum && OP_glue(op) && !OP_memory(op))) {
	/* Propagate copy assignements. */
        INT cix = copy_operand(op);
        TN *tnr = OP_result(op, OP_Copy_Result(op));

        if ((tnr != NULL) && (tnr != True_TN) && (tnr != Zero_TN)) {
          EBO_TN_INFO *tninfo = EBO_last_opinfo->actual_rslt[OP_Copy_Result(op)];

          if (!OP_glue(op) && (cix >= 0)) {
            tninfo->replacement_tn = opnd_tn[cix];
            tninfo->replacement_tninfo = opnd_tninfo[cix];
#ifdef TARG_ST
	    if (!TN_is_constant(OP_opnd(op, cix)) &&
		TN_register_class(tnr) == TN_register_class(OP_opnd(op, cix))) {
	      // We propagated a replacement TN, however we're not sure that 
	      // the replacement TN can be a physical replacement, so we keep track
	      // of an alternative reg class copy tn that may be substituted.
	      tninfo->copy_tn = OP_opnd(op, cix);
	      tninfo->copy_tninfo = orig_tninfo[cix];
	    } else {
	      tninfo->copy_tn = NULL;
	      tninfo->copy_tninfo = NULL;
	    }
#endif
            if (EBO_Trace_Data_Flow) {
              #pragma mips_frequency_hint NEVER
              fprintf(TFile,"%sPropagate Copy of ",EBO_trace_pfx);
              Print_TN(tninfo->replacement_tn,FALSE);
              fprintf(TFile,"[%d] into ",(tninfo->replacement_tninfo != NULL)?tninfo->replacement_tninfo->omega:0);
              Print_TN(tnr,FALSE); fprintf(TFile,"\n");
            }
          }
        }

#ifdef TARG_IA64
        if ((resnum == 2) && ((tnr=OP_result(op,1)) != NULL) && (tnr != True_TN)  && (tnr != Zero_TN)) {
         /* This logic must be in sync with what ebo_special calls a "copy".       
            This instruction must actually be placing a "FALSE" condition in a predicate. */
          EBO_TN_INFO *tninfo = EBO_last_opinfo->actual_rslt[1];
          tninfo->replacement_tn = Zero_TN;

          if (EBO_Trace_Data_Flow) {
            #pragma mips_frequency_hint NEVER
            fprintf(TFile,"%sPropagate Copy of ",EBO_trace_pfx);
            Print_TN(tninfo->replacement_tn,FALSE); fprintf(TFile," into ");
            Print_TN(tnr,FALSE); fprintf(TFile,"\n");
          }
        }
#endif /* TARG_IA64 */

#ifdef TARG_ST
      } else if (rslt_tn == NULL) {
	TN *replacement_tn;
	if ((replacement_tn = EBO_literal_replacement_tn(op)) != NULL) {
	  TN *tnr = OP_result(op, 0);
	  if ((tnr != NULL) && (tnr != True_TN) && (tnr != Zero_TN)) {
	    EBO_TN_INFO *tninfo = EBO_last_opinfo->actual_rslt[0];
	    tninfo->replacement_tn = replacement_tn;
	    tninfo->replacement_tninfo = NULL;
	    if (EBO_Trace_Data_Flow) {
		#pragma mips_frequency_hint NEVER
	      fprintf(TFile,"%sPropagate Replacement TN ",EBO_trace_pfx);
	      Print_TN(tninfo->replacement_tn,FALSE);
	      fprintf(TFile," into ");
	      Print_TN(tnr,FALSE); fprintf(TFile,"\n");
	    }
	  }
	}
#endif
      } else if (rslt_tn != NULL) {
	/* A result tn needs to be replaced. */
        TN *tnr = OP_result(op, rslt_num);
        EBO_TN_INFO *tninfo = EBO_last_opinfo->actual_rslt[rslt_num];

	/* 
	 * This is subtle - yes we do want the replacement_tninfo
         * entry to point to the tninfo entry we just created. Yes,
         * it does create a circular link in the chain. Code that
         * searches the chain will need to be aware of this. 
	 */
        tninfo->replacement_tn = rslt_tn;
        tninfo->replacement_tninfo = tninfo;
        Set_OP_result (op, rslt_num, rslt_tn);

        if (EBO_Trace_Data_Flow) {
          #pragma mips_frequency_hint NEVER
          fprintf(TFile,"%sReplace result[%d] tn ",EBO_trace_pfx,rslt_num);
          Print_TN(tnr,FALSE); fprintf(TFile," with ");
          Print_TN(rslt_tn, FALSE); fprintf(TFile,"\n");
        }

      }
    }

    if (PROC_has_branch_delay_slot()) in_delay_slot = OP_xfer(op);
  }

  return no_barriers_encountered;
}
  




static
void EBO_Remove_Unused_Ops (BB *bb, BOOL BB_completely_processed)
/* -----------------------------------------------------------------------
 * -----------------------------------------------------------------------
 */
{
  EBO_OP_INFO *opinfo;
  EBO_TN_INFO *tninfo;
  TN *tn;

  if (EBO_first_opinfo == NULL) return;

  if (EBO_Trace_Execution) {
    #pragma mips_frequency_hint NEVER
    fprintf(TFile,"%sEnter EBO_Remove_Unused_Ops in BB:%d\n",EBO_trace_pfx,BB_id(bb));
    tn_info_table_dump();
  }

  for (opinfo = EBO_last_opinfo; opinfo != NULL; opinfo = opinfo->prior) {
    INT rslt_count = 0;
    INT idx;
    OP *op = opinfo->in_op;

    if (op == NULL) continue;

    if (OP_bb(op) != bb) {
      if (EBO_Trace_Block_Flow) {
        #pragma mips_frequency_hint NEVER
        fprintf(TFile,"Stop looking for unused ops, next op is in BB:%d\n",
                OP_bb(op) ? BB_id(OP_bb(op)) : -1);
        Print_OP_No_SrcLine(op);
      }
      break;  /* get out of  loop over opinfo entries. */
    }

    if (EBO_Trace_Data_Flow) {
      #pragma mips_frequency_hint NEVER
      fprintf(TFile,"%sConsider removing OP:\n\t",
              EBO_trace_pfx);
      Print_OP_No_SrcLine(op);
    }


    rslt_count = OP_results(op);
    if (rslt_count == 0) goto op_is_needed;
    if (op_is_needed_globally(op)) goto op_is_needed;

#ifdef TARG_ST
    // [CG]: skip early implicit interactions
    if (OP_has_implicit_interactions(op))
      goto op_is_needed;
#endif

   /* Check that all the result operands can be safely removed. */
    for (idx = 0; idx < rslt_count; idx++) {
      tninfo = opinfo->actual_rslt[idx];

     /* A couple of safety checks. */
      if (tninfo == NULL) continue;
      if (tninfo->in_bb != bb) goto op_is_needed;
      if (tninfo->in_op == NULL) goto op_is_needed;
      tn = tninfo->local_tn;

      if (EBO_Trace_Data_Flow) {
        #pragma mips_frequency_hint NEVER
        fprintf(TFile,"%sTry to remove definition of entry[%d] ",
                EBO_trace_pfx,tninfo->sequence_num);
        Print_TN(tn,FALSE);
        fprintf(TFile,"\n");
      }

     /* Zero_TN or True_TN for a result is a no-op. */
      if (tn == Zero_TN) continue;
      if (tn == True_TN) continue;

     /* Copies to and from the same register are not needed. */
      if (EBO_in_peep &&
          OP_effectively_copy(op) &&
	  OP_Copy_Result(op) == idx &&
          has_assigned_reg(tn) &&
          (copy_operand(op) >= 0) &&
          has_assigned_reg(OP_opnd(op,copy_operand(op))) &&
          (tn_registers_identical(tn, OP_opnd(op,copy_operand(op))))) {
        INT cpo = copy_operand(op);

       /* We may be able to get rid of the copy, but be
          sure that the TN is marked live into this block. */
        if ((opinfo->actual_opnd[cpo] != NULL) &&
            (bb != opinfo->actual_opnd[cpo]->in_bb)) {
          mark_tn_live_into_BB (tn, bb, opinfo->actual_opnd[cpo]->in_bb);
        }

       /* Propagate use count for this TN to it's input TN. */
        if (tninfo->same != NULL) {
          tninfo->same->reference_count += tninfo->reference_count;
        }

        if (!tninfo->redefined_before_block_end &&
            (tninfo->same != NULL) &&
            (tninfo->same->in_bb == bb)) {
         /* Removing the copy causes the previous definition
            of the TN (or reg) to reach the end of the block. */
          tninfo->same->redefined_before_block_end = FALSE;
        }
        goto can_be_removed;
      }

     /* There must be no direct references to the TN. */
      if (!BB_completely_processed) goto op_is_needed;
      if (tninfo->reference_count != 0) goto op_is_needed;
      if (OP_has_implicit_interactions(tninfo->in_op))
	goto op_is_needed;

     /* Check for indirect and global references.   */
      if (!tninfo->redefined_before_block_end &&
          TN_live_out_of(tn, tninfo->in_bb)) goto op_is_needed;

      if (TN_is_pfs_reg(tn)) goto op_is_needed;
      if (TN_is_lc_reg(tn)) goto op_is_needed;
      if (TN_is_ec_reg(tn)) goto op_is_needed;
    }

   /* None of the results are needed. */
    if (opinfo->op_must_not_be_removed) goto op_is_needed;
    if (OP_store(op)) goto op_is_needed;

can_be_removed:

    remove_op (opinfo);

    if (EBO_Trace_Optimization) {
      #pragma mips_frequency_hint NEVER
      fprintf(TFile, "%sin BB:%d removing    ",
              EBO_trace_pfx, BB_id(bb));
      Print_OP_No_SrcLine(op);
    }

    if (opinfo->in_delay_slot) {
      OP_Change_To_Noop(op);
    } else {
      BB_Remove_Op(bb, op);
    }
    opinfo->in_op = NULL;
    opinfo->in_bb = NULL;

   /* Propagate "reaches block end" information. */
    for (idx = 0; idx < rslt_count; idx++) {
      tninfo = opinfo->actual_rslt[idx];
      if ((tninfo != NULL) &&
          tninfo->redefined_before_block_end &&
          (tninfo->same != NULL) &&
          (tninfo->same->in_bb == bb)) {
        EBO_TN_INFO *next_tninfo = tninfo->same;
        next_tninfo->redefined_before_block_end = TRUE;
      }
    }

    continue;

op_is_needed:

   /* Predicate resolution may not have been possible, and */
   /* preceeding definitions of the same result TN's may   */
   /* need to be saved.                                    */
    for (idx = 0; idx < rslt_count; idx++) {
      tninfo = opinfo->actual_rslt[idx];
      if ((tninfo != NULL) &&
          (tninfo->local_tn != NULL) &&
          (tninfo->same != NULL)) {
        EBO_TN_INFO *next_tninfo = tninfo->same;

        while (next_tninfo != NULL) {
          if ((next_tninfo->in_op != NULL) &&
              (!EBO_predicate_dominates((tninfo->predicate_tninfo != NULL)?tninfo->predicate_tninfo->local_tn:True_TN,
                                         tninfo->predicate_tninfo,
                                         (next_tninfo->predicate_tninfo != NULL)?
                                               next_tninfo->predicate_tninfo->local_tn:True_TN,
                                         next_tninfo->predicate_tninfo)) &&
              (!EBO_predicate_complements((tninfo->predicate_tninfo != NULL)?tninfo->predicate_tninfo->local_tn:True_TN,
                                          tninfo->predicate_tninfo,
                                          (next_tninfo->predicate_tninfo != NULL)?
                                               next_tninfo->predicate_tninfo->local_tn:True_TN,
                                          next_tninfo->predicate_tninfo))) {

           /* A store into an unresolved predicate is a potential problem     */
           /* because the last store might not completely redefine the first. */
           /* The predicates could be completely independant.  But we         */
           /* don't know how to check for that, currently.                    */

           /* Stop searching and preserve the preceeding definition. */
            EBO_OP_INFO *opinfo = locate_opinfo_entry(next_tninfo);
            if (opinfo != NULL) {
              opinfo->op_must_not_be_removed = TRUE;
            } else {
             /* Couldn't find the opinfo entry.  Make sure that the TN has
                a use count so that the defining OP entry will not be deleted. */
              next_tninfo->reference_count += tninfo->reference_count;
            }

            if (EBO_Trace_Data_Flow) {
              #pragma mips_frequency_hint NEVER
              fprintf(TFile,"%sMark result same_tn as needed - original [%d]: ",
                      EBO_trace_pfx,tninfo->sequence_num);
              Print_TN(tninfo->local_tn,FALSE);
              fprintf(TFile," same as [%d]: ",next_tninfo->sequence_num);
              Print_TN(next_tninfo->local_tn,FALSE);
              fprintf(TFile,"\n");
            }

            if (EBO_predicate_dominates((next_tninfo->predicate_tninfo != NULL)?
                                               next_tninfo->predicate_tninfo->local_tn:True_TN,
                                         next_tninfo->predicate_tninfo,
                                        (tninfo->predicate_tninfo != NULL)?tninfo->predicate_tninfo->local_tn:True_TN,
                                         tninfo->predicate_tninfo)) {
              /* If the first store dominates the last, there can be no       */
              /* preceeding definitions that are partially redefined by       */
              /* the last store.  We can stop searching for other dominators. */
              break;
            }
          }
          next_tninfo = next_tninfo->same;
        }

      }
    }

   /* Predicate resolution may not have been possible, and */
   /* preceeding inputs to this instruction may be defined */
   /* within the current extended block, and must be saved.*/
    for (idx = 0; idx < OP_opnds(op); idx++) {
      tninfo = opinfo->actual_opnd[idx];
      if ((tninfo != NULL) &&
          (tninfo->local_tn != NULL) &&
          (tninfo->same != NULL)) {
        EBO_TN_INFO *next_tninfo = tninfo->same;

        while (next_tninfo != NULL) {
          if ((next_tninfo->in_op != NULL) &&
              (next_tninfo->omega == tninfo->omega)) {
            if (EBO_predicate_dominates((next_tninfo->predicate_tninfo != NULL)?
                                               next_tninfo->predicate_tninfo->local_tn:True_TN,
                                         next_tninfo->predicate_tninfo,
                                         (tninfo->predicate_tninfo != NULL)?tninfo->predicate_tninfo->local_tn:True_TN,
                                         tninfo->predicate_tninfo)) {
              /* This predicate dominates the OP we need to save. It's   */
              /* use count should be sufficiant to cause it to be saved. */
              /* We can stop searching for other dominators.             */
              break;
            } else if (EBO_predicate_complements((next_tninfo->predicate_tninfo != NULL)?
                                               next_tninfo->predicate_tninfo->local_tn:True_TN,
                                         next_tninfo->predicate_tninfo,
                                         (tninfo->predicate_tninfo != NULL)?tninfo->predicate_tninfo->local_tn:True_TN,
                                         tninfo->predicate_tninfo)) {
              /* These predicates are mutually exclusive.  There is no   */
              /* need to mark it as used, but se need to keep looking.   */
            } else {
             /* A store into an unresolved predicate is a potential problem. */
             /* The predicates could be completely independant.  But we      */
             /* don't know how to check for that, currently.                 */

             /* Stop searching and preserve the preceeding definition. */
              EBO_OP_INFO *opinfo = locate_opinfo_entry(next_tninfo);
              if (opinfo != NULL) {
                opinfo->op_must_not_be_removed = TRUE;
              } else {
               /* Couldn't find the opinfo entry.  Make sure that the TN has
                  a use count so that the defining OP entry will not be deleted. */
                next_tninfo->reference_count += tninfo->reference_count;
              }

             /* Check for block spanning values and make sure they are marked global. */
              if ((next_tninfo != NULL) &&
                  (bb != next_tninfo->in_bb)) {
                mark_tn_live_into_BB (tninfo->local_tn, bb, next_tninfo->in_bb);
              }

              if (EBO_Trace_Data_Flow) {
                #pragma mips_frequency_hint NEVER
                fprintf(TFile,"%sMark operand same_tn as needed - original [%d]: ",
                        EBO_trace_pfx,tninfo->sequence_num);
                Print_TN(tninfo->local_tn,FALSE);
                fprintf(TFile," same as [%d]: ",next_tninfo->sequence_num);
                Print_TN(next_tninfo->local_tn,FALSE);
                fprintf(TFile,"\n");
              }

            }
          }
          next_tninfo = next_tninfo->same;
        }

      }
    }


   /* Check for newly created references that cross a block
      boundary.  If one is found, mark it as global and carry
      registers in/out of blocks. */
    for (idx = 0; idx < OP_opnds(op); idx++) {
      tninfo = opinfo->actual_opnd[idx];
      if ((tninfo != NULL) &&
          (bb != tninfo->in_bb)) {
        mark_tn_live_into_BB (tninfo->local_tn, bb, tninfo->in_bb);
      }
    }

  } /* end: for each opinfo entry */

 /* Make a quick scan of the OPS in a BB and remove noops. */
  {
    OP *op;
    OP * next_op = NULL;
    in_delay_slot = FALSE;
#ifdef TARG_ST
    // Arthur: return the simulated OP anyway
    TOP noop_top = TOP_noop;
#else
    TOP noop_top = CGTARG_Noop_Top();
#endif

    for (op = BB_first_op(bb); op != NULL; op = next_op) {
      next_op = OP_next(op);
      if (   (OP_code(op) == noop_top || OP_code(op) == TOP_noop) 
	  && !in_delay_slot)
      {
        if (EBO_Trace_Optimization) {
          #pragma mips_frequency_hint NEVER
          fprintf(TFile, "%sin BB:%d removing noop    ",
                  EBO_trace_pfx, BB_id(bb));
          Print_OP_No_SrcLine(op);
        }

        BB_Remove_Op(bb, op);
      } else if (PROC_has_branch_delay_slot()) {
	if (in_delay_slot && OP_code(op) == TOP_noop) {
	   // ugly hack for mips
	   OP_Change_Opcode(op, noop_top);
	}
        in_delay_slot = OP_xfer(op);
      }
    }

  }

  return;
}
  
  
/* ===================================================================== */

  
/* 
 */
static
void
EBO_Add_BB_to_EB (BB * bb)
{
  EBO_TN_INFO *save_last_tninfo = EBO_last_tninfo;
  EBO_OP_INFO *save_last_opinfo = EBO_last_opinfo;
  BBLIST *succ_list;
  BOOL normal_conditions;

  if (EBO_Trace_Execution) {
    #pragma mips_frequency_hint NEVER
    fprintf(TFile,"%sEnter EBO_Add_BB_to_EB BB:%d. It has %d successors\n",
            EBO_trace_pfx,BB_id(bb),BB_succs(bb)?BB_succs_len(bb):0);
  }

  Set_BB_visited(bb);

  if (CG_skip_local_ebo &&
      ((BB_id(bb) < CG_local_skip_before) ||
       (BB_id(bb) > CG_local_skip_after)  ||
       (BB_id(bb) == CG_local_skip_equal))) return;

 /* Add this block to the current Extended Block (EB). */
  normal_conditions = Find_BB_TNs (bb);
  if (normal_conditions &&
      !CG_localize_tns) {
   /* Walk through the successors, trying to grow the EB. */
    FOR_ALL_BB_SUCCS(bb, succ_list) { 
      BB *succ = BBLIST_item(succ_list);

      if (EBO_Trace_Block_Flow) {
        #pragma mips_frequency_hint NEVER
        fprintf(TFile,"%sCheck successor BB:%d. It has %d preds and has%s been visited.\n",
               EBO_trace_pfx,BB_id(succ),BB_preds_len(succ),BB_visited(succ)?" ":" not");
      }

      if (!BB_call(bb) &&
          (BB_preds_len(succ) == 1) &&
          !BB_visited(succ) &&
          (BB_rid(bb) == BB_rid(succ))) {
        EBO_Add_BB_to_EB (succ);
      }
    }
  }

 /* When we are unable to grow the EB any more, optimize what we have. */
  if (EBO_Trace_Block_Flow) {
    #pragma mips_frequency_hint NEVER
    fprintf(TFile,"%sEBO optimization at BB:%d\n",EBO_trace_pfx,BB_id(bb));
  }

  EBO_Remove_Unused_Ops(bb, normal_conditions);

 /* Remove information about TN's and OP's in this block. */
  backup_tninfo_list(save_last_tninfo);
  backup_opinfo_list(save_last_opinfo);

  return;
}
  
  
/* 
 * Perform EBO
 */
static
void
EBO_Process ( BB *first_bb )
{
  BB *bb;

  rerun_cflow = FALSE;

  EBO_Trace_Execution    = Get_Trace(TP_EBO, 0x001);
  EBO_Trace_Optimization = Get_Trace(TP_EBO, 0x002);
  EBO_Trace_Block_Flow   = Get_Trace(TP_EBO, 0x004);
  EBO_Trace_Data_Flow    = Get_Trace(TP_EBO, 0x008);
  EBO_Trace_Hash_Search  = Get_Trace(TP_EBO, 0x010);

  FmtAssert(((EBO_first_tninfo == NULL) && (EBO_first_opinfo == NULL)),
                  ("Initial pointers not NULL %o %o",EBO_first_tninfo,EBO_first_opinfo));

  EBO_Start();

  if (EBO_Trace_Data_Flow || EBO_Trace_Optimization) {
    #pragma mips_frequency_hint NEVER
    fprintf(TFile,">>>> Before EBO");
    if (EBO_in_peep) {
      fprintf(TFile," - in peep ");
    } else if (EBO_in_before_unrolling) {
      fprintf(TFile," - before unrolling ");
    } else if (EBO_in_after_unrolling) {
      fprintf(TFile," - after unrolling ");
    } else if (EBO_in_pre) {
      fprintf(TFile," - preprocessing ");
    } else {
      fprintf(TFile," - main ");
    }
    fprintf(TFile,"<<<<\n");

    if (EBO_Trace_Data_Flow) {
      Print_All_BBs ();
    }
  }

  /* TEMPORARY - EBO doesn't understand rotating registers, so skip blocks that use them. */
  for (bb = first_bb; bb != NULL; bb = BB_next(bb)) {
    if (BB_rotating_kernel(bb)) Set_BB_visited(bb);
  }

#ifdef TARG_ST
  // Arthur: this may be a unnecessary check, but ...
  //         EBO shouldn't need to do anything for BBs that are
  //         scheduled or SWP'd and there was no spill. The SWP'd
  //         loops on IA-64 are protected because of the check
  //         above. Scheduled one's are not ?
  //         So, protect them here, in the worst case we win some
  //         compile time.
  for (bb = first_bb; bb != NULL; bb = BB_next(bb)) {
    if (BB_scheduled(bb)) Set_BB_visited(bb);
  }
#endif

  for (bb = first_bb; bb != NULL; bb = BB_next(bb)) {
    RID *bbrid;
    if (( bbrid = BB_rid( bb )) &&
        ( RID_level( bbrid ) >= RL_CGSCHED ) ) {
     /*
      * There is no overlap in TN's between the current REGION
      * and REGIONs which have already been through CG
      */
      continue;
    }
    if (!BB_visited(bb)) {
      EBO_Add_BB_to_EB (bb);
      if (EBO_in_loop) break;
    }
  }

 /* Clear the bb flag, in case some other phase uses it. */
  clear_bb_flag (first_bb);

  if (rerun_cflow) {
    CFLOW_Optimize(CFLOW_BRANCH | CFLOW_UNREACHABLE, "CFLOW (from ebo)");
  }

  EBO_Finish();

  if (EBO_Trace_Data_Flow) {
    #pragma mips_frequency_hint NEVER
    fprintf(TFile,">>>> After EBO");
    if (EBO_in_peep) {
      fprintf(TFile," - in peep ");
    } else if (EBO_in_before_unrolling) {
      fprintf(TFile," - before unrolling ");
    } else if (EBO_in_after_unrolling) {
      fprintf(TFile," - after unrolling ");
    } else if (EBO_in_pre) {
      fprintf(TFile," - preprocessing ");
    } else {
      fprintf(TFile," - main ");
    }
    fprintf(TFile,"<<<<\n");
    fprintf(TFile,">>>> %d tninfo entries allocated and reused %d times\n",
    EBO_num_tninfo_entries,EBO_tninfo_entries_reused);
    fprintf(TFile,">>>> %d opinfo entries allocated and reused %d times\n",
    EBO_num_opinfo_entries,EBO_opinfo_entries_reused);
    Print_All_BBs ();
  }

}

/* =======================================================================
 * =======================================================================
 *
 * There are a number of different entry points to EBO.  They differ because
 * the specific information available changes throughout the compilation.
 * The flags that are set at each entry are used during EBO if a use is to
 * be made of information that is only available at certain times.
 *
 * =======================================================================
 * =======================================================================
 */

/* 
 * perform EB optimizations right after instruction translation.
 */
void
EBO_Pre_Process_Region ( RID *rid )
{
  BB *first_bb = (rid) ? CGRIN_first_bb(RID_cginfo( rid )) : REGION_First_BB;

  EBO_in_pre  = TRUE;
  EBO_in_before_unrolling = FALSE;
  EBO_in_after_unrolling = FALSE;
  EBO_in_peep = FALSE;

  EBO_in_loop = FALSE;

  if ((EBO_Opt_Level < 5) && ((EBO_Opt_Level > 0) || (EBO_Opt_Level != -5))) return;

  clear_bb_flag (first_bb);
  EBO_Process (first_bb);
}

  
/* 
 * perform EBO optimizations during unrolling and pipelining
 */
void
EBO_before_unrolling(BB_REGION *bbr )
{
  INT i;
  EBO_in_pre  = FALSE;
  EBO_in_before_unrolling = TRUE;
  EBO_in_after_unrolling = FALSE;
  EBO_in_peep = FALSE;

  EBO_in_loop = TRUE;

  if ((EBO_Opt_Level < 4) && ((EBO_Opt_Level > 0) || (EBO_Opt_Level != -4))) return;

#ifdef TARG_IA64
  for (i = 0; i < bbr->entries.size(); i++) {
    clear_bb_flag (bbr->entries[i]);
  }
  for (i = 0; i < bbr->exits.size(); i++) {
    Set_BB_visited (bbr->exits[i]);
  }
  EBO_Process (bbr->entries[0]);
#endif
}

  
/* 
 * perform EBO optimizations after unrolling and pipelining
 */
void
EBO_after_unrolling(BB_REGION *bbr )
{
  INT i;

  EBO_in_pre  = FALSE;
  EBO_in_before_unrolling = FALSE;
  EBO_in_after_unrolling = TRUE;
  EBO_in_peep = FALSE;

  EBO_in_loop = TRUE;

  if ((EBO_Opt_Level < 3) && ((EBO_Opt_Level > 0) || (EBO_Opt_Level != -3))) return;
 
#ifdef TARG_IA64
  for (i = 0; i < bbr->entries.size(); i++) {
    clear_bb_flag (bbr->entries[i]);
  }
  for (i = 0; i < bbr->exits.size(); i++) {
    Set_BB_visited (bbr->exits[i]);
  }
  EBO_Process (bbr->entries[0]);
#endif
}

  
/* 
 * perform EB optimizations on a region
 */
void
EBO_Process_Region ( RID *rid )
{
  BB *first_bb = (rid) ? CGRIN_first_bb(RID_cginfo( rid )) : REGION_First_BB;

  EBO_in_pre  = FALSE;
  EBO_in_before_unrolling = FALSE;
  EBO_in_after_unrolling = FALSE;
  EBO_in_peep = FALSE;

  EBO_in_loop = FALSE;

  if ((EBO_Opt_Level < 2) && ((EBO_Opt_Level > 0) || (EBO_Opt_Level != -2))) return;

  clear_bb_flag (first_bb);
  EBO_Process (first_bb);
}

  
/* 
 * perform EB optimizations after register assignment.
 */
void
EBO_Post_Process_Region ( RID *rid )
{
  BB *first_bb = (rid) ? CGRIN_first_bb(RID_cginfo( rid )) : REGION_First_BB;

  EBO_in_pre  = FALSE;
  EBO_in_before_unrolling = FALSE;
  EBO_in_after_unrolling = FALSE;
  EBO_in_peep = TRUE;

  EBO_in_loop = FALSE;

  if ((EBO_Opt_Level < 1) && ((EBO_Opt_Level >= 0) || (EBO_Opt_Level != -1))) return;

  /* compute live-in sets for physical registers */
  MEM_POOL_Push(&MEM_local_pool);
  REG_LIVE_Analyze_Region();

  clear_bb_flag (first_bb);
  EBO_Process (first_bb);
  REG_LIVE_Finish();
  MEM_POOL_Pop(&MEM_local_pool);
}
