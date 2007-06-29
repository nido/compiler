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

//  gra_cflow implementation
/////////////////////////////////////
//
//  Description:
//
//      Various functions for manipulating the flow graph on the behalf
//      of GRA.
//
/////////////////////////////////////



#ifdef USE_PCH
#include "cg_pch.h"
#endif // USE_PCH
#pragma hdrstop

#include "defs.h"
#include "errors.h"
#include "cgir.h"
#include "gra_live.h"
#include "annotations.h"
#include "gra_grant.h"

#include "gra_cflow.h"

BOOL GRA_split_entry_exit_blocks = TRUE;
    // Split entry and exit blocks before GRA and join them after so that
    // the copies to/from the callee-saves registers don't interfere with
    // other allocation decisions.

static BB_LIST* entry_bbs;  // New entry BBs created to hold prologs
static BB_LIST* exit_bbs;   // New exit BBs create to hold epilogs

/////////////////////////////////////
inline BOOL
OP_Is_Copy_To_Save_TN(const OP* op)
/////////////////////////////////////
//
//  Is <op> a copy from a callee-saves register into its save-TN?
//  
/////////////////////////////////////
{
  INT i;

  for ( i = OP_results(op) - 1; i >= 0; --i ) {
    TN* tn = OP_result(op,i);
    if ( TN_is_save_reg(tn)) return TRUE;
  }

  return FALSE;
}

/////////////////////////////////////
inline BOOL
OP_Is_Copy_From_Save_TN( const OP* op )
/////////////////////////////////////
//
//  Is <op> a copy to a callee-saves register from its save-TN?
//
/////////////////////////////////////
{
  INT i;

  // You'd think there'd be a better way than groveling through the operands,
  // but short of marking these when we make them, this seems to be the most
  // bullet-proof

  for ( i = OP_results(op) - 1; i >= 0; --i ) {
    if ( TN_is_dedicated(OP_result(op,i)) ) break;
  }
  if ( i < 0 ) return FALSE;

  for ( i = OP_opnds(op) - 1; i >= 0; --i ) {
    TN* tn = OP_opnd(op,i);
    if ( TN_Is_Allocatable(tn) && TN_is_save_reg(tn))
      return TRUE;
  }

  return FALSE;
}

/////////////////////////////////////
static void
Split_Entry( BB* bb )
/////////////////////////////////////
//
//  <bb> is an entry of the PU.  Split it in two with all the references to
//  wired callee-saves registers in a new entry that falls through into <bb>
//  which now becomes a normal block.  Record the new entry on the list
//  <entry_bbs>.
//  
/////////////////////////////////////
{
  OP* op;
  OP* prev_op = NULL;   // prevents stupid warning message
  BB* prev_bb   = BB_prev(bb);
  BB* new_entry = Create_Dummy_BB(bb);

  Insert_BB(new_entry,prev_bb);
  BB_Transfer_Entryinfo(bb,new_entry);
  BB_freq(new_entry) = BB_freq(bb);

  op = BB_last_op(bb);
#ifdef TARG_ST
  // [SC] Exception entry blocks can have no instructions in them.
  if (op != NULL)
#endif
  do {
    prev_op = OP_prev(op); 
    if (OP_Is_Copy_To_Save_TN(op)) 
      BB_Move_Op_To_Start(new_entry, bb, op);
    op = prev_op;
  } while (op != NULL && op != BB_entry_sp_adj_op(new_entry));

  for (op = BB_entry_sp_adj_op (new_entry); op != NULL; op = prev_op) {
    prev_op = OP_prev(op);
    BB_Move_Op_To_Start (new_entry, bb, op);
  }

  GRA_LIVE_Compute_Local_Info(bb);
  GRA_LIVE_Compute_Local_Info(new_entry);
  GRA_LIVE_Region_Start();
  GRA_LIVE_Region_Entry(new_entry);
  GRA_LIVE_Region_Exit(bb);
  GRA_LIVE_Region_Compute_Global_Live_Info();

  entry_bbs = BB_LIST_Push(new_entry,entry_bbs,GRA_pool);
}

/////////////////////////////////////
static void
Join_Entry( BB* bb )
/////////////////////////////////////
//
//  <bb> is an entry that was created by Split_Entry.  Undo what Split_Entry
//  did, that is join the entry block it created with the following block,
//  allowing the operations in the two blocks to be scheduled together by
//  final_sched.
//
/////////////////////////////////////
{
  BB* new_entry = BB_next(bb);

  BB_Transfer_Entryinfo(bb,new_entry);
  GRA_GRANT_Transfer(bb,new_entry);
  BB_Prepend_All(new_entry,bb);
  Remove_BB(bb);
  Unlink_Pred_Succ(bb,new_entry);
  Reset_BB_scheduled(new_entry);
  GRA_LIVE_Merge_Blocks(new_entry,bb,new_entry);
}

/////////////////////////////////////
static void
Split_Exit( BB* bb )
/////////////////////////////////////
//
//  <bb> is an exit block of the current PU.  Split it in half with a new
//  final block to hold the function epilog code.  The original exit becomes a
//  normal BB and falls through into the new exit.  Record the new exit block
//  on the list <exit_bbs>
//  
/////////////////////////////////////
{
#ifdef TARG_ST
  // FdF 20041105: No EXITINFO_sp_adj was generated in case of a
  // "noreturn" call.
  if (!BB_exit_sp_adj_op(bb)) {
    Is_True(WN_Call_Never_Return( CALLINFO_call_wn(ANNOT_callinfo(ANNOT_Get(BB_annotations(bb), ANNOT_CALLINFO)))),
	    ("Missing SP adjust"));
    return;
  }
#endif
  OP* op;
  OP* next_op = NULL;
  BB* new_exit= Gen_And_Insert_BB_After(bb);

  BB_Transfer_Exitinfo(bb,new_exit);
  BB_freq(new_exit) = BB_freq(bb);

  op = BB_first_op(bb);
  do {
    next_op = OP_next(op); 
    if (OP_Is_Copy_From_Save_TN(op)) 
      BB_Move_Op_To_End(new_exit, bb, op);
    op = next_op;
  } while (op != NULL && op != BB_exit_sp_adj_op(new_exit));

  for (op = BB_exit_sp_adj_op (new_exit); op != NULL; op = next_op) {
    next_op = OP_next(op);
    BB_Move_Op_To_End (new_exit, bb, op);
  }

  // Must be after we have moved the jr to be bottom of new_exit:
  Target_Simple_Fall_Through_BB(bb,new_exit);

  GRA_LIVE_Compute_Local_Info(bb);
  GRA_LIVE_Compute_Local_Info(new_exit);
  GRA_LIVE_Region_Start();
  GRA_LIVE_Region_Entry(bb);
  GRA_LIVE_Region_Exit(new_exit);
  GRA_LIVE_Region_Compute_Global_Live_Info();

  exit_bbs = BB_LIST_Push(new_exit,exit_bbs,GRA_pool);
}

/////////////////////////////////////
static void
Join_Exit( BB* bb )
/////////////////////////////////////
//
//  <bb> is an exit block generated by Split_Exit.  Undo what Split_Exit did,
//  making <bb> and its predecessor into a single exit block, allowing the
//  operations in the two blocks to be scheduled together by final_sched.
//  
/////////////////////////////////////
{
  BB* new_exit BB_prev(bb);

  BB_Transfer_Exitinfo(bb,new_exit);
  GRA_GRANT_Transfer(bb,new_exit);
  BB_Append_All(new_exit,bb);
  Remove_BB(bb);
  Unlink_Pred_Succ(new_exit,bb);
  Reset_BB_scheduled(new_exit);
  GRA_LIVE_Merge_Blocks(new_exit,new_exit,bb);
}

/////////////////////////////////////
void
GRA_Split_Entry_And_Exit_BBs(BOOL is_region)
/////////////////////////////////////
//  See interface description.
/////////////////////////////////////
{
  BB* bb;
  BB* next_bb = NULL;   // avoid stupid used before set warning
  BB_LIST *elist;

  if ( ! GRA_split_entry_exit_blocks ) return;

  entry_bbs = NULL;
  exit_bbs = NULL;

  for ( bb = REGION_First_BB; bb != NULL; bb = next_bb ) {
    next_bb = BB_next(bb);
    if ( BB_entry(bb) ) {
      DevAssert(is_region == FALSE,
		("Procedure/Function entry block in region.\n"));
      Split_Entry(bb);
    }
  }

  // 
  // we only deal with exits when processing the PU.  we split the
  // exit blocks, and they become part of the complement region.
  // we have to do this so that spill/restore placement can be based
  // on callee saved register use throughout the program unit.
  // 
  if ( is_region == FALSE ) {
    for ( elist = Exit_BB_Head; elist; elist = BB_LIST_rest(elist) ) {
      Split_Exit(BB_LIST_first(elist));
    }
  }
}

/////////////////////////////////////
void
GRA_Join_Entry_And_Exit_BBs(void)
/////////////////////////////////////
//  See interface description.
/////////////////////////////////////
{
  BB_LIST *bb_list;

  if ( ! GRA_split_entry_exit_blocks ) return;

  for ( bb_list = entry_bbs;
        bb_list != NULL;
        bb_list = BB_LIST_rest(bb_list)
  ) {
    BB* entry_bb = BB_LIST_first(bb_list);

    Join_Entry(entry_bb);
  }

  for ( bb_list = exit_bbs;
        bb_list != NULL;
        bb_list = BB_LIST_rest(bb_list)
  ) {
    BB* exit_bb = BB_LIST_first(bb_list);

    Join_Exit(exit_bb);
  }
}

#include "tracing.h"

/////////////////////////////////////
void
GRA_Add_Call_Spill_Block(BB* bb, BB* succ)
/////////////////////////////////////
//  See interface description.
/////////////////////////////////////
{
  BB *new_succ = Gen_And_Insert_BB_After(bb);

#ifdef TARG_ST
  //
  // Arthur: seems like a bug to me. Change_Succ() recomputes
  //         'new_succ' and old "succ' frequencies wrong (perhaps
  //         Change_Succ() shouldn't be used here at all ?), so do 
  //         BB_freq(new_succ) frequency setting after, and 
  //         remember BB_freq(succ).
  //
  float old_freq = BB_freq(succ);
  Change_Succ(bb, succ, new_succ);
  BB_freq(new_succ) = BB_freq(bb);
  BB_freq(succ) = old_freq;
#else
  BB_freq(new_succ) = BB_freq(bb);
  Change_Succ(bb, succ, new_succ);
#endif
  Link_Pred_Succ_with_Prob(new_succ, succ, 1.0);

  GRA_LIVE_Compute_Local_Info(bb);
  GRA_LIVE_Compute_Local_Info(new_succ);
  GRA_LIVE_Region_Start();
  GRA_LIVE_Region_Entry(bb);
  GRA_LIVE_Region_Exit(new_succ);
  GRA_LIVE_Region_Compute_Global_Live_Info();
}
