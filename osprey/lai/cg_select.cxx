/*
  Copyright (C) 2002, STMicroelectronics, All Rights Reserved.

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
 * Module: cg_select.cxx
 * $Revision$
 * $Date$
 * $Author$
 * $Source$
 *
 * Description:
 *
 * If-convert if-then-else (hammocks) regions using selects.
 * Simplifies successive conditional jumps into logical expressions.
 *
 * ====================================================================
 * ====================================================================
 */

#include <list.h>
#include "defs.h"
#include "config.h"
#include "config_TARG.h"
#include "tracing.h"
#include "timing.h"
#include "cgir.h"
#include "cg.h"
#include "cgtarget.h"
#include "cg_flags.h"
#include "ttype.h"
#include "bitset.h"
#include "bb_map.h"
#include "bb_set.h"
#include "label_util.h"
#include "freq.h"
#include "whirl2ops.h"
#include "dominate.h"
#include "gtn_universe.h"
#include "gtn_set.h"
#include "gtn_tn_set.h"
#include "gra_live.h"
#include "cxx_memory.h"
#include "cg_ssa.h"
#include "exp_targ.h"
#include "cg_select.h"
#include "DaVinci.h"

static BB_MAP postord_map;      // Postorder ordering
static BB     **cand_vec;       // Vector of potential hammocks BBs.
static INT32  max_cand_id;	// Max value in hammock candidates.
static BB_MAP if_bb_map;        // Map fall_thru and targets of logif

// Mapping between old and new PHIs
static OP_MAP phi_op_map = NULL;

// List of of memory accesses found in if-then-else region. 
// Load and stores lists are used in a slightly different manner:
// Stores are mapped with their equivalent. (this might be improved with a
// dummy store location).
// Loads are mapped with their dismissible form. They don't need to have an
// equivalent (a load on the other side of the hammock).
// I keep these operation in a list because we don't want to touch the basic
// blocks now. (we don't know yet if the hammock will be reduced).
// OPs will be updated in BB_Fix_Spec_Loads and BB_Fix_Spec_Stores.

pair<op_list, op_list> store_i;
pair<op_list, op_list> load_i;

BB * bbs_tail_dup[2];

/* ================================================================
 *
 *   Traces
 *
 * ================================================================
 */
static BOOL Trace_Select_Candidates; /* -Wb,-tt61:0x001 */
static BOOL Trace_Select_Gen;        /* -Wb,-tt61:0x002 */
static BOOL Trace_Select_Spec;       /* -Wb,-tt61:0x004 */
static BOOL Trace_Select_Merge;      /* -Wb,-tt61:0x008 */
static BOOL Trace_Select_Dup;        /* -Wb,-tt61:0x010 */
static BOOL Trace_Select_Stats;      /* -Wb,-tt61:0x020 */
static BOOL Trace_Select_daVinci;    /* -Wb,-tt61:0x040 */

static void
Trace_Select_Init() {
  Trace_Select_Candidates = Get_Trace(TP_SELECT, Select_Candidates);
  Trace_Select_Gen        = Get_Trace(TP_SELECT, Select_Gen);
  Trace_Select_Spec       = Get_Trace(TP_SELECT, Select_Spec);
  Trace_Select_Merge      = Get_Trace(TP_SELECT, Select_Merge);
  Trace_Select_Dup        = Get_Trace(TP_SELECT, Select_Dup);
  Trace_Select_Stats      = Get_Trace(TP_SELECT, Select_Stats);
  Trace_Select_daVinci    = Get_Trace(TP_SELECT, Select_daVinci);
}

MEM_POOL MEM_Select_pool;
UINT select_count;
UINT logif_count;
UINT spec_instrs_count;

static void
CG_SELECT_Statistics()
{
  if (select_count || logif_count)
    fprintf (stdout, "========= %s ======= \n", ST_name(Get_Current_PU_ST()));

  if (select_count) 
    fprintf (stdout, "<cg_select> converted %d moves \n", select_count);

  if (logif_count) 
    fprintf (stdout, "<cg_select> reduced %d logifs \n", logif_count);

  if (spec_instrs_count) 
    fprintf (stdout, "<cg_select> %d instrs were converted \n", spec_instrs_count);

}

/* ================================================================
 *
 *   Initialize/Finalize
 *
 * ================================================================
 */
void 
Select_Init()
{
  Trace_Select_Init();

  static BOOL did_init = FALSE;

  if ( ! did_init ) {
    MEM_POOL_Initialize(&MEM_Select_pool,"Select pool",FALSE);
    did_init = TRUE;
  }
  MEM_POOL_Push(&MEM_Select_pool);
}

static void
Initialize_Memory()
{
  if_bb_map = BB_MAP_Create();
}

static void
Finalize_Memory()
{
  BB_MAP_Delete(if_bb_map);
  Free_Dominators_Memory();
  MEM_POOL_Pop(&MEM_Select_pool);
}

static void
Finalize_Select(void)
{
  Finalize_Memory();

  BB_MAP_Delete(postord_map);
  max_cand_id = 0;
  select_count = 0;
  logif_count = 0;
  spec_instrs_count = 0;
}

static void
Initialize_Hammock_Memory()
{
  phi_op_map = OP_MAP_Create();
}

static void
clear_spec_lists()
{
  load_i.first.clear();
  load_i.second.clear();
  store_i.first.clear();
  store_i.second.clear();

  bbs_tail_dup[0] = NULL;
  bbs_tail_dup[1] = NULL;
}

static void
Finalize_Hammock_Memory()
{
  OP_MAP_Delete(phi_op_map);
}

/* ================================================================
 *
 *   Misc routines to manipulate the CFG.
 *
 * ================================================================
 */
// Return the immediate postdominator of bb that can eventually be a
// 'then' or an 'else' side of a 'if-then-else' region.
static BB*
Find_Immediate_Postdominator(BB* bb, BB* then_bb, BB* else_bb)
{
  BB *ipdom;

  if (BB_SET_MemberP(BB_pdom_set(bb), then_bb))
    return then_bb;

  if (BB_SET_MemberP(BB_pdom_set(bb), else_bb))
    return else_bb;

  if ((ipdom = BB_Unique_Successor(then_bb)) == BB_Unique_Successor(else_bb))
    if (ipdom && ! BB_SET_MemberP(BB_pdom_set(bb), ipdom)) 
      return NULL;

  return ipdom;
}

static BOOL
BB_Fall_Thru_and_Target_Succs(BB *bb, BB **fall_thru, BB **target)
{
  BBLIST *edge;
  logif_info *logif;

  logif = (logif_info *)BB_MAP_Get(if_bb_map, bb);
  if (logif) {
    *fall_thru = logif->fall_thru;
    *target    = logif->target;
    return logif->inverted;
  }

  *fall_thru = BB_Fall_Thru_Successor(bb);
  FOR_ALL_BB_SUCCS(bb, edge) {
    *target = BBLIST_item(edge);
    if (*target != *fall_thru) break;
  }

  logif = (logif_info*)MEM_POOL_Alloc(&MEM_Select_pool, sizeof(logif_info));
  logif->fall_thru = *fall_thru;
  logif->target    = *target;
  logif->inverted  = FALSE;
  BB_MAP_Set(if_bb_map, bb, logif);

  return logif->inverted;

}

static UINT8
Get_In_Edge_Pos (BB* in_bb, BB* bb)
{
  BBLIST *pred_edge;
  UINT8 pos = 0;

  FOR_ALL_BB_PREDS(bb, pred_edge) {
    BB *pred = BBLIST_item(pred_edge);
    if (in_bb == pred)
      return pos;
    ++pos;
  }

  FmtAssert(FALSE, ("no edge for bb %d in bb %d\n", BB_id(bb), BB_id(in_bb)));
}

static BOOL
Can_Merge_BB (BB *bb_first, BB *bb_second, OP *br_op)
{
  BBLIST *edge;

  if (br_op && BB_call(bb_second))
    return FALSE;

  if (BB_succs_len (bb_first) != 1 || BB_preds_len (bb_second) != 1)
    return FALSE;

  //  FOR_ALL_BB_SUCCS(bb_second, edge) {
  //    BB *pred = BBLIST_item(edge);
  //    if (pred == bb_first)
  //      return FALSE;
  //  }

  return TRUE;
}

static void
BB_Merge (BB *bb_first, BB *bb_second)
{
  OP* op;
  BB *succ;
  BBLIST *bl;
  OP *br;

  if (Trace_Select_Merge) {
    fprintf (TFile, "BB_Merge %d %d\n", BB_id (bb_first), BB_id (bb_second));
    Print_BB (bb_first);
    Print_BB (bb_second);
  }

  //
  // Move all of its ops to bb_first.
  //
  OP* op_next;
  OP* last = BB_last_op(bb_first);
  for (op = BB_first_op(bb_second); op; op = op_next) {
    op_next = OP_next(op);
    if (last) {
      BB_Move_Op(bb_first, last, bb_second, op, FALSE);
    } else {
      BB_Append_Op (bb_first, op);
    }
    last = op;
  }

  // Clean up the firsts pred-succ list
  while (bl = BB_succs(bb_first)) {
    succ = BBLIST_item(bl);
    Unlink_Pred_Succ(bb_first, succ);
  }

  //
  // Take bb_second out of the list.
  //
  Remove_BB(bb_second);

  //
  // If bb_second an exit, move the relevant info to the
  // merged block.
  //
  if (BB_exit(bb_second)) {
    BB_Transfer_Exitinfo(bb_second, bb_first);
    Exit_BB_Head = BB_LIST_Delete(bb_second, Exit_BB_Head);
    Exit_BB_Head = BB_LIST_Push(bb_first, Exit_BB_Head, &MEM_pu_pool);
  }

  //
  // Transfer call info.
  //
  if (BB_call(bb_second)) {
    BB_Copy_Annotations(bb_first, bb_second, ANNOT_CALLINFO);
  }

  //
  // Move bb_second's successor arcs to bb_first.
  //
  FmtAssert(BBlist_Len(BB_succs(bb_second))<=2,("Too many successors"));
  if (BBlist_Len(BB_succs(bb_second)) == 1) {
    bl = BB_succs(bb_second);
    float old_prob = BBLIST_prob(bl);
    succ = BBLIST_item(bl);
    Unlink_Pred_Succ(bb_second, succ);
    Link_Pred_Succ_with_Prob(bb_first, succ, old_prob);
  } else {
    while (bl = BB_succs(bb_second)) {
      succ = BBLIST_item(bl);
      Unlink_Pred_Succ(bb_second, succ);
      //      float prob = Get_Branch_Prob(bb_second,succ,freq_map);;
      //      if (succ != Get_Branch_Block(bb_second,freq_map)) {
      //	prob = 1.0 - Get_Branch_Prob(bb_second,succ,freq_map);
      // }	
    //      Link_Pred_Succ_with_Prob(bb_first, succ, prob);
    Link_Pred_Succ(bb_first, succ);
    }
  }

  //  fprintf (TFile, "\n BB_Merge DONE\n");
  //  Print_BB (bb_first);
  //  fprintf (TFile, "\n");  
  //
  // Fix up hyperblock data structures.
  //
  //  if (HB_Exit(hb) == bb_second) {
  //    HB_Exit_Set(hb, bb_first);
  //   }
  // Replace_Block(bb_first,bb_second,candidate_regions);
}

// promoted tns was global because they ended up inside a phi.
// Now that they are converted into a select and the blocks have
// been merged, that might not be true.
static void
BB_Localize_Tns (BB *bb) 
{
  OP *op;

  FOR_ALL_BB_OPs_FWD(bb, op) {  
    UINT8 i;

    for (i = 0; i < OP_results(op); i++) {
      TN *tn = OP_result(op, 0);

      if (!GTN_SET_MemberP(BB_live_out(bb),tn))
        Reset_TN_is_global_reg(tn); 
    }
  }
}

/////////////////////////////////////
static void
Rename_Locals(OP* op, hTN_MAP dup_tn_map)
/////////////////////////////////////
//
//  Local TN's must be renamed in duplicated block, otherwise
//  they'll look like globals.  Note that we assume we're processing
//  the ops in forward order.  If not, the way we map the new names
//  below won't work.
//
/////////////////////////////////////
{
  INT i = 0;
  TN* res;

  for (i = 0; i < OP_results(op); i++) {
    res = OP_result(op, i);
    if (TN_is_register(res) &&
	!(TN_is_dedicated(res) || TN_is_global_reg(res))) {
      TN* new_tn = Dup_TN(res);
      hTN_MAP_Set(dup_tn_map, res, new_tn);
      Set_OP_result(op, i, new_tn);
    }
  }

  i = 0;
  for (INT opndnum = 0; opndnum < OP_opnds(op); opndnum++) {
    res = OP_opnd(op, opndnum);
    if (TN_is_register(res) &&
	!(TN_is_dedicated(res) || TN_is_global_reg(res))) {
      res = (TN*) hTN_MAP_Get(dup_tn_map, res);
      Set_OP_opnd(op, i, res);
    }
    i++;
  }
}

/* ================================================================
 *
 *   Tail duplication
 *
 * ================================================================
 */
static BB *
Copy_BB_For_Tail_Duplication(BB* old_bb)
{
  BB* new_bb = NULL;

  new_bb = Gen_BB_Like(old_bb);

  //
  // Copy the ops to the new block.
  //
  OP *op;
  MEM_POOL_Push(&MEM_local_pool);
  hTN_MAP dup_tn_map = hTN_MAP_Create(&MEM_local_pool);

  FOR_ALL_BB_OPs_FWD(old_bb, op) {
    OP *new_op = Dup_OP(op);
    Copy_WN_For_Memory_OP(new_op, op);
    BB_Append_Op(new_bb, new_op);
    Rename_Locals(new_op, dup_tn_map);
  }
  MEM_POOL_Pop(&MEM_local_pool);

  //
  // Take care of the annotations.
  //
  switch (BB_kind(old_bb)) {
  case BBKIND_CALL:
    BB_Copy_Annotations(new_bb, old_bb, ANNOT_CALLINFO);
    break;

  case BBKIND_TAIL_CALL:
    BB_Copy_Annotations(new_bb, old_bb, ANNOT_CALLINFO);
    //
    // NOTE FALLTHRU
    //
  case BBKIND_RETURN:
    {
      BB_Copy_Annotations(new_bb, old_bb, ANNOT_EXITINFO);
      OP *b_op;
      OP *suc_op;
      ANNOTATION *ant = ANNOT_Get(BB_annotations(new_bb), ANNOT_EXITINFO);
      EXITINFO *exit_info = ANNOT_exitinfo(ant);
      EXITINFO *new_info = TYPE_PU_ALLOC(EXITINFO);
      OP *sp_adj = EXITINFO_sp_adj(exit_info);
      *new_info = *exit_info;
      if (sp_adj) {
	for (suc_op = BB_last_op(old_bb), b_op = BB_last_op(new_bb);
	     suc_op != sp_adj;
	     suc_op = OP_prev(suc_op), b_op = OP_prev(b_op))
	  ;
	EXITINFO_sp_adj(new_info) = b_op;
      }
      ant->info = new_info;

      Set_BB_exit(new_bb);
      Exit_BB_Head = BB_LIST_Push(new_bb, Exit_BB_Head, &MEM_pu_pool);
      break;
    }
  }

  return new_bb;

}

/////////////////////////////////////
static void
Fixup_Arcs(BB* entry, BB* old_bb, BB* new_bb, BB* fall_thru)
/////////////////////////////////////
//
//  Fix up input arcs to the new block from blocks outside the hyperblock,
//  and add successor arcs.
//
/////////////////////////////////////
{
  BBLIST* bl;
  float new_freq = 0.0;

  //
  // Move predecessor arcs from outside of the hyperblock to new block.
  //
  for (bl = BB_preds(old_bb); bl != NULL;) {
    BB* pred = BBLIST_item(bl);
    bl = BBLIST_next(bl);

    //
    // Calculate block frequency.
    //
    BBLIST* blsucc = BB_Find_Succ(pred, old_bb);

    //
    // Now, it's either a block not selected for the hyperblock, or
    // a block that's already been duplicated, or it is an unduplicated
    // member of the hyperblock and we need do nothing.
    //
    if (pred == entry) {
      continue;
    } else {
      new_freq += BB_freq(pred) * BBLIST_prob(blsucc);
      if (BB_Fall_Thru_Successor(pred) == old_bb) {
	Change_Succ(pred, old_bb, new_bb);
      } else {
	BB_Retarget_Branch(pred, old_bb, new_bb);
      }
    }
  }

  BB_freq(new_bb) = new_freq;
}

static void
Tail_Duplicate(BB* entry, BB* side_entrance, BB* fall_thru)
{
  BBLIST* bl;
  BB* dup =  Copy_BB_For_Tail_Duplication(side_entrance);

  if (Trace_Select_Dup) 
    fprintf(TFile, "<Select> Tail duplicating BB:%d.  Duplicate BB%d\n",
	    BB_id(side_entrance), BB_id(dup));
  

  //
  // Update the arcs and liveness.
  //
  Fixup_Arcs(entry, side_entrance, dup, fall_thru);

  //
  // Add block, and any block added to compensate for a fall thru out
  // of the hammock.
  //
  Insert_BB(dup, fall_thru);
}

/* ================================================================
 *
 *   Misc routines to manipulate the SSA's Phis.
 *
 * ================================================================
 */
// Make a copy of a phi op, using the same tn as return value in order to no
// break uses. The implementation (there is 2 same defs during a short time)
// make it possible without maintaining a phi's defs list.
// The original phi should be removed.
static OP*
Dup_PHI (OP* phi)
{
  TN *result[1];
  UINT8 j = 0;
  UINT8 nopnds = OP_opnds(phi);
  TN *opnd[nopnds];
  for (j = 0; j < OP_opnds(phi); j++) {
    opnd[j] = OP_opnd(phi, j);
  }
  result[0] = OP_result(phi, 0);

  return Mk_VarOP (TOP_phi, 1, nopnds, result, opnd);
}

// Remove old phis or replace it with a new one from phi_op_map.
// Note that we cannot use the same list to insert selects because they
// would be inserted in the 'head' bblock.
static void
BB_Update_Phis(BB *bb)
{
  OP *phi;
  op_list phi_list;
  
  FOR_ALL_BB_PHI_OPs(bb,phi) {
    OP *new_phi;

    new_phi = (OP *)OP_MAP_Get(phi_op_map, phi);

    if (! new_phi)
      new_phi = Dup_PHI(phi);

    phi_list.push_front(phi);
    SSA_Prepend_Phi_To_BB(new_phi, bb);
  }
  
  BB_Remove_Ops(bb, phi_list);
}

/* ================================================================
 *
 *   Hammock Selection
 *
 * ================================================================
 */
// Create a map of conditional blocks sorted in postorder. That makes it
// easy to iterate through possible hammocks regions.
static void
Identify_Hammock_Candidates(void)
{
  BB *bb;

  postord_map = BB_Postorder_Map(NULL, NULL);

  // maximum size.
  cand_vec = TYPE_MEM_POOL_ALLOC_N(BB *, &MEM_Select_pool, PU_BB_Count);
  max_cand_id = 0;

  // Make a list of hammock candidates.
  for (bb = REGION_First_BB; bb != NULL; bb = BB_next(bb)) {
    INT32 bb_id = BB_MAP32_Get(postord_map, bb);
    DevAssert(bb_id >= 0 && bb_id <= PU_BB_Count, ("bad <postord_map> value"));
    // we are interested only with reachable conditional head BBs
    if (bb_id > 0 && BB_kind (bb) == BBKIND_LOGIF) { 
      max_cand_id = MAX(bb_id, max_cand_id);
      cand_vec[bb_id - 1] = bb;
    }
    else {
      cand_vec[bb_id - 1] = NULL;
    }
  }
}

static BOOL
Can_Speculate_BB(BB *bb, op_list *stores)
{
  OP* op;

  FOR_ALL_BB_OPs_FWD(bb, op) {
    if (! OP_Can_Be_Speculative(op)) {
      if (OP_memory (op)) {
        if (! Force_Memory_Dismiss || OP_volatile(op))
          return FALSE;

        if (OP_load (op)) {
          TOP ld_top;
          OP *lop;

          if ((ld_top = CGTARG_Speculative_Load (op)) == TOP_UNDEFINED)
            return FALSE;

          lop = Dup_OP (op);
          OP_Change_Opcode(lop, ld_top); 
          Set_OP_speculative(lop);  

          load_i.first.push_front(op);
          load_i.second.push_front(lop);
        }

        else if (OP_store (op)) {
           if (! stores)
             return FALSE;
           stores->push_front(op);
        }
      }
      else
        return FALSE;
    }
  }

  return TRUE;
}

static BOOL
Are_Aliased(OP* op1, OP* op2)
{
  if (OP_memory(op1) && OP_memory(op2)) {
    WN *wn1 = Get_WN_From_Memory_OP(op1);
    WN *wn2 = Get_WN_From_Memory_OP(op2);
    if (wn1 != NULL && wn2 != NULL) {
      ALIAS_RESULT alias = Aliased(Alias_Manager, wn1, wn2);
      return alias == SAME_LOCATION;
    }
  }

  return FALSE;
}

static UINT
BB_Check_Memops(void)
{
  UINT count = 0;

  // We don't have the same count of store. Can't spec any.
  if (store_i.first.size() != store_i.second.size())
    return 0;

  // Each store should have an equiv.
  op_list::iterator i1_iter = store_i.first.begin();
  op_list::iterator i1_end  = store_i.first.end();
  op_list::iterator i2_iter = store_i.second.begin();
  op_list::iterator i2_end  = store_i.second.end();
  UINT c = 0;

  while(i1_iter != i1_end) {
    if (!Are_Aliased (*i1_iter, *i2_iter)) {
      OP * old_op = *i2_iter;     
      i2_iter = store_i.second.erase(i2_iter);
      // *i1_iter didn't match. failed
      if (c++ == store_i.second.size())
        return FALSE;
      store_i.second.push_front(old_op);
    }
    else {
      ++count;
      ++i1_iter;
      ++i2_iter;
    }
  }

  return count;
}

// Test if a hammock is suitable for select conversion
// For now, the heuristic is simply the # of instructions to speculate.
//  later: compute dependence heigh, use freq informations.

UINT max_select_instrs = 1000;

static BOOL
Check_Suitable_Hammock (BB* ipdom, BB* target, BB* fall_thru)
{
  if (Trace_Select_Candidates) {
    fprintf(TFile, "<select> Found Hammock : ");
    fprintf(TFile, " target BB%d, fall_thru BB%d, tail BB%d\n",
            BB_id(target), BB_id(fall_thru), BB_id(ipdom));
  }
    
  //  Print_BB (target);
  //  Print_BB (fall_thru);

  if (target != ipdom) {
    if (! Can_Speculate_BB(target, &store_i.first))
      return FALSE;
  }
  
  // very naiive heuristic... change that later when the engine is finalized.
  if (BB_length(target) > max_select_instrs)
    return FALSE;

  if (fall_thru != ipdom) {
    if (! Can_Speculate_BB(fall_thru, &store_i.second))
      return FALSE;
  }
  
  // Check if we have the same set of memory stores in both sides.
  if (!store_i.first.empty() || !store_i.second.empty()) {
    if (!BB_Check_Memops())
      return FALSE;
  }

  return BB_length(fall_thru) <= max_select_instrs;
}

static BOOL
Is_Hammock (BB *head, BB **target, BB **fall_thru, BB **tail) 
{
  BBLIST *edge;
  BOOL found_taken = FALSE;
  BOOL found_not_taken = FALSE;

  if (Trace_Select_Candidates) {
    fprintf (TFile, "<select> is hammock BB%d ? \n", BB_id(head));
    Print_BB_Header (head, FALSE, TRUE);
  }

  // Find fall_thru and taken BBs.
  BB_Fall_Thru_and_Target_Succs(head, fall_thru, target);

  if (Trace_Select_Candidates) {
    fprintf (TFile, "<select> target BB%d ? fall_thru BB%d\n",
             BB_id(*target), BB_id(*fall_thru));
  }

  // to do: tail duplicate to create hammocks if sides have more than
  // one succ or pred.

  // Starting from immediate post dominator, get the 2 conditional blocks.
  *tail = Find_Immediate_Postdominator(head, *target, *fall_thru);
  if (*tail == NULL)
    return FALSE;

  FOR_ALL_BB_PREDS (*tail, edge) { 
    BB *bb = BBLIST_item(edge);
    if (bb == *target) {
      if (BB_succs_len (bb) != 1)
        return FALSE; 

      if (BB_preds_len (bb) != 1) {
        if (BB_preds_len (bb) == 2) {
          bbs_tail_dup[0] = bb;
        }
        else
          return FALSE; 
      }

      found_taken = TRUE;
    }
    else if (bb == *fall_thru) {
      if (BB_succs_len (bb) != 1)
        return FALSE; 

      if (BB_preds_len (bb) != 1) {
        if (BB_preds_len (bb) == 2) {
          bbs_tail_dup[1] = bb;
        }
        else
          return FALSE; 
      }

      found_not_taken = TRUE;
    }
    else if (bb == head && *tail == *target) {
      found_taken = TRUE;
    }
    else if (bb == head && *tail == *fall_thru) {
      found_not_taken = TRUE;
    }

    if (found_taken && found_not_taken) {
      return Check_Suitable_Hammock (*tail, *target, *fall_thru);
    }
  }  

  return FALSE;
}

/* ================================================================
 *
 *   Hammock Conversion
 *
 * ================================================================
 */
// Delete a BB, i.e. moving all its OPs to its predecessor
// and remove it from the list of BBs for the region/PU.
static void
Promote_BB(BB *bp, BB *to_bb)
{
  BBLIST *edge;
  
  // Sanity checks. We can only have one predecessor, where cond is set.
  BB *pred = BB_Unique_Predecessor(bp);
  DevAssert(to_bb == pred, ("Promote_BB"));

  // Sanity checks. Thoses cases cannot happen inside a hammock
  if (BB_loophead(bp) || BB_entry(bp) || BB_exit(bp)) {
    DevAssert(FALSE, ("Promote_BB"));
  }

  spec_instrs_count += BB_length (bp);

  // Move all OPs from BB.
  BB_Append_All (to_bb, bp);

  // Remove from BB chain.
  Remove_BB(bp);
  
  // Remove successor edges.
  FOR_ALL_BB_SUCCS(bp, edge) {
    BB *succ = BBLIST_item(edge);
    BBlist_Delete_BB(&BB_preds(succ), bp);
  }
  BBlist_Free(&BB_succs(bp));

  // Remove predecessor edges.
  FOR_ALL_BB_PREDS(bp, edge) {
    BB *pred = BBLIST_item(edge);
    BBlist_Delete_BB(&BB_succs(pred), bp);
  }
  BBlist_Free(&BB_preds(bp));
}

static void
BB_Fix_Spec_Loads (BB *bb)
{
  DevAssert(load_i.first.size() == load_i.second.size(),
            ("not speculative load"));    

  while (!load_i.first.empty()) {
    OP* lop = load_i.first.front();
    OP* slop = load_i.second.front();

    BB_Insert_Op_Before (bb, lop, slop);
    BB_Remove_Op (bb, lop);

    load_i.first.pop_front();
    load_i.second.pop_front();
  }
}

static void
BB_Fix_Spec_Stores (BB *bb, TN* cond_tn, VARIANT variant)
{
  while (!store_i.first.empty()) {
    OPS ops = OPS_EMPTY;
    TN *true_tn;
    TN *false_tn;

    OP* i1 = store_i.first.front();
    OP* i2 = store_i.second.front();
    
    DevAssert(Are_Aliased (i1, i2), ("stores are not alias"));    

    if (variant == V_BR_P_TRUE) {
      true_tn = OP_opnd(i1, 2);
      false_tn = OP_opnd(i2, 2);
    }
    else if (variant == V_BR_P_FALSE) {
      true_tn = OP_opnd(i2, 2);
      false_tn = OP_opnd(i1, 2);
    }
    else
      DevAssert(FALSE, ("variant"));    

    TN *temp_tn = Build_TN_Like (true_tn);

    Expand_Select (temp_tn, cond_tn, true_tn, false_tn, MTYPE_I4, FALSE, &ops);
    select_count++;

    Expand_Store (MTYPE_I4, temp_tn, OP_opnd(i1, 1), OP_opnd(i1, 0), &ops);

    BB_Insert_Ops_After (bb, i2, &ops);

    BB_Remove_Op (bb, i1);
    BB_Remove_Op (bb, i2);

    store_i.first.pop_front();
    store_i.second.pop_front();
  }
}

// Check that bb has no liveout or side effects beside the conditional jump.
static BOOL
Dead_BB (BB *bb)
{
  OP *op;

  FOR_ALL_BB_OPs_FWD(bb, op) {  
    UINT8 i;

    for (i = 0; i < OP_results(op); i++) {
      TN *tn = OP_result(op, 0);
      if (GTN_SET_MemberP(BB_live_out(bb), tn))
        return FALSE;
    }

    return TRUE;
  }
}

static BOOL
Negate_Cmp_BB (OP *br)
{
  DEF_KIND kind;
  
  OP *cmp_op = TN_Reaching_Value_At_Op(OP_opnd(br, 0), br, &kind, TRUE);
  TOP new_cmp = CGTARG_Invert(OP_code(cmp_op));
  if (new_cmp == TOP_UNDEFINED)
    return FALSE;

  OP_Change_Opcode(cmp_op, new_cmp);
  return TRUE;
}

static BOOL
Negate_Branch_BB (OP *br)
{
  TOP new_cmp = CGTARG_Invert(OP_code(br));
  if (new_cmp == TOP_UNDEFINED)
    return FALSE;

  OP_Change_Opcode(br, new_cmp);
  return TRUE;
}

static BOOL
Prep_And_Normalize_Jumps(BB *bb1, BB *bb2, BB *fall_thru1, BB *target1,
                         BB **fall_thru2, BB **target2)
{
  OP *br1 = BB_branch_op (bb1);
  OP *br2 = BB_branch_op (bb2);
  
  FmtAssert(br1 && OP_cond(br1), ("BB doesn't end in a conditional branch."));
  FmtAssert(br2 && OP_cond(br2), ("BB doesn't end in a conditional branch."));

  BOOL needInvert = OP_code (br1) != OP_code (br2);

  BB_Fall_Thru_and_Target_Succs(bb2, fall_thru2, target2);
  
  if (target1 == *target2 || fall_thru1 == *fall_thru2) {
    if (needInvert) {
      if (! Negate_Cmp_BB(br2))
        return FALSE;
    }
    return TRUE;
  }

  if (target1 == *fall_thru2 || fall_thru1 == *target2) {
    // if needInvert, don't revert the condition.
    if (! needInvert)
      if (! Negate_Cmp_BB(br2))
        return FALSE;

    BB *tmpbb = *fall_thru2;
    *fall_thru2 = *target2;
    *target2 = tmpbb;

    logif_info *logif = (logif_info *)BB_MAP_Get(if_bb_map, fall_thru1);
    logif->inverted = TRUE;
    logif->fall_thru = *fall_thru2;
    logif->target = *target2;

    return TRUE;
  }
  
  return FALSE;
}

// Test if bb is the head of a region that can be converted with logical
// expressions. (ex: if (p1) if (p0) ... = if (p1 && p0) ...)
// Returns the head of the second if region or null
//
// A well formed logical expression is a sequence of 2 blocks containing
// a conditional jump, for which one direct successor is in common.
// This is done regardless of the branches directions, as it is
// possible (with respect to the available concrete instructions) to
// transform any pattern in a pure and or or expression.
// Normalizejumps will try to retrieve such a pattern.
static BB*
Is_Double_Logif(BB* bb)
{
  BB *fall_thru, *target, *sec_fall_thru, *sec_target;

  // Find fall_thru and taken BBs.
  BB_Fall_Thru_and_Target_Succs(bb, &fall_thru, &target);

  if (BB_kind(target) == BBKIND_LOGIF) {
    if (BB_preds_len(target) == 1
        && Can_Speculate_BB(target, NULL)
        && Prep_And_Normalize_Jumps(bb, target, fall_thru, target,
                                    &sec_fall_thru, &sec_target)
        && Dead_BB (target)
        && sec_fall_thru == fall_thru)
      return target;
  }

  // try the other side
  if (BB_kind(fall_thru) == BBKIND_LOGIF) {
    if (BB_preds_len(fall_thru) == 1
        && Can_Speculate_BB(fall_thru, NULL)
        && Prep_And_Normalize_Jumps(bb, fall_thru, fall_thru, target,
                                    &sec_fall_thru, &sec_target)
        && Dead_BB (fall_thru)
        && sec_target == target)
      return fall_thru;
  }
  
  return NULL;
}

static void 
Simplify_Logifs(BB *bb1, BB *bb2)
{
  BB *bb1_fall_thru, *bb1_target, *bb2_fall_thru, *bb2_target;
  BB *fall_thru_block, *joint_block;
  BOOL invert;
  BOOL AndNeeded;

  BB_Fall_Thru_and_Target_Succs(bb1, &bb1_fall_thru, &bb1_target);
  invert = BB_Fall_Thru_and_Target_Succs(bb2, &bb2_fall_thru, &bb2_target);

  if (bb1_fall_thru == bb2_fall_thru) {
    // if (a && b)
    AndNeeded = TRUE;
    fall_thru_block = bb2_target;
    joint_block = bb2_fall_thru;
  }
  else if (bb1_target == bb2_target) {
    // if (a || b)
    AndNeeded = FALSE;
    fall_thru_block = bb2_fall_thru;
    joint_block = bb2_target;
  }

  if (Trace_Select_Gen) {
    fprintf (TFile, "Convert if BB%d BB%d BB%d BB%d BB%d BB%d\n",
             BB_id(bb1), BB_id(bb1_fall_thru), BB_id(bb1_target),
             BB_id(bb2), BB_id(bb2_fall_thru), BB_id(bb2_target));
  }

  OP *br1_op = BB_branch_op(bb1);
  OP *br2_op = BB_branch_op(bb2);

  // get compares return values.
  TN *b1tn1, *b2tn1;
  TN *tn2;
  OP *cmp1, *cmp2;
  VARIANT variant1 = CGTARG_Analyze_Compare(br1_op, &b1tn1, &tn2, &cmp1);
  VARIANT variant2 = CGTARG_Analyze_Compare(br2_op, &b2tn1, &tn2, &cmp2);

  TN *branch_tn = Build_TN_Like (b1tn1);
  TN *label_tn = OP_opnd(br1_op, OP_find_opnd_use(br1_op, OU_target));

  UINT8 bb_pos = Get_In_Edge_Pos (bb2, joint_block);

  BB_Remove_Branch(bb1);
  BB_Remove_Branch(bb2);
  Promote_BB(bb2, bb1);
  Unlink_Pred_Succ (bb1, fall_thru_block);

  OPS ops = OPS_EMPTY;

  b1tn1 = Expand_CMP_Reg (cmp1, b1tn1, &ops);
  b2tn1 = Expand_CMP_Reg (cmp2, b2tn1, &ops);

  // !a op !a -> !(a !op b)
  AndNeeded = AndNeeded ^ (variant1 == V_BR_P_FALSE);

  // insert new logical op.
  if (AndNeeded)
    Expand_Logical_And (branch_tn, b1tn1, b2tn1, variant1, &ops);
  else
    Expand_Logical_Or (branch_tn, b1tn1, b2tn1, variant1, &ops);

  logif_count++;

  Expand_Branch (label_tn, branch_tn, NULL, variant1, &ops);

  BB_Append_Ops (bb1, &ops);

  if (joint_block == BB_next(bb1)) {
    br1_op = BB_branch_op(bb1);
    Negate_Branch_BB (br1_op);
    Target_Logif_BB(bb1, fall_thru_block, 1.0L - BB_freq(joint_block), joint_block);
  }
  else {
    Target_Logif_BB(bb1, joint_block, BB_freq(joint_block), fall_thru_block);
  }

  BB_MAP_Set(if_bb_map, bb1, NULL);
    
  // if needed, update phi operands and new edge from head.
  BB *phi_block = invert ? fall_thru_block : joint_block;

  if (joint_block == phi_block) {
    OP *phi;

    FOR_ALL_BB_PHI_OPs(phi_block, phi) {
      TN *result[1];
      UINT8 j = 0;
      UINT8 i = 0;
      UINT8 nopnds = OP_opnds(phi)-1;
      TN *opnd[nopnds];
      for (i = 0; i < OP_opnds(phi); i++) {
        if (i != bb_pos)
          opnd[j++] = OP_opnd(phi, i);
      }

      result[0] = OP_result(phi, 0);
      OP *new_phi = Mk_VarOP (TOP_phi, 1, nopnds, result, opnd);
      OP_MAP_Set(phi_op_map, phi, new_phi);
    }
    BB_Update_Phis(joint_block);  
  }
  else
    BB_Update_Phis(fall_thru_block);
}

/* ================================================================
 *
 *   If flattening
 *
 * ================================================================
 */
static void
Select_Fold (BB *head, BB *target_bb, BB *fall_thru_bb, BB *tail)
{
  OP *phi;
  BOOL edge_needed = FALSE;

  //  fprintf (TFile, "\nin Select_Fold\n");
  //  Print_All_BBs();

  //  fprintf (TFile, "Select_Fold %d %d %d %d\n", BB_id(head), BB_id (target_bb),
  //           BB_id (fall_thru_bb), BB_id (tail));

  UINT8 taken_pos = Get_In_Edge_Pos(target_bb == tail ? head : target_bb, tail);
  UINT8 nottaken_pos = Get_In_Edge_Pos(fall_thru_bb == tail ? head : fall_thru_bb, tail);

  // keep a list of newly created conditional move / compare
  // and phis to remove
  OPS cmov_ops = OPS_EMPTY;
  op_list old_phis;

  TN *cond_tn;
  TN *tn2;
  OP *cmp;
  OP *br_op = BB_branch_op (head);

  // find the instruction that set the condition.
  VARIANT variant = CGTARG_Analyze_Compare(br_op, &cond_tn, &tn2, &cmp);

  FOR_ALL_BB_PHI_OPs(tail, phi) {
    UINT8 i;
    TN *select_tn;
    TN *true_tn = NULL;
    TN *false_tn = NULL;

    if (Trace_Select_Gen) {
      fprintf(TFile, "<select> handle phi ");
      Print_OP (phi);
      fprintf(TFile, "in \n");      
      Print_BB (tail);
    }

    // We need to find the tn's that map to the correct position.
    // tn's are not necessary in the same order than the predecessors.
    // need to check where cg_ssa has mapped them.
    BB *taken_bb    = Get_PHI_Predecessor (phi, taken_pos);
    BB *nottaken_bb = Get_PHI_Predecessor (phi, nottaken_pos);

    for (i = 0; i < OP_opnds(phi); i++) {
      TN *tn = OP_opnd(phi,i);
      if (i == taken_pos)
          true_tn = tn;
      if (i == nottaken_pos) 
          false_tn = tn;
    }

    // deal with undef operands.
    DevAssert(true_tn && false_tn, ("Select undef TNs. not yet"));

    // make sure that select operands are in the same order than phi operands.
    // Select operands must match predecessors order.
    BOOL reord_ops = (Get_In_Edge_Pos (taken_bb,tail) != taken_pos) ||
      (Get_In_Edge_Pos (nottaken_bb,tail) != nottaken_pos);
      
    // (if brf && !reord) || (if br && reord))
    if ((variant == V_BR_P_FALSE) != reord_ops) {
      TN *temp_tn = false_tn;
      false_tn = true_tn;
      true_tn = temp_tn;
    }
      
    // for now we only handle single result tn.
    // we don't need to go through (i = 0; i < OP_results(op); i++) loop.
    DevAssert (OP_results(phi) == 1, ("unsupported multiple results select"));
        
    select_tn = OP_result (phi, 0);
    
    if (BB_preds_len(tail) != 2) 
      select_tn =  Dup_TN(select_tn);
    
    Expand_Select (select_tn, cond_tn, true_tn, false_tn,
                   MTYPE_I4, FALSE, &cmov_ops);
    select_count++;

    // If the block has only 2 incoming merges, just replace phi's tn by the
    // new tn. else we need to create a new def and insert it into the phi
    // operands at the  place of the old edges.
    if (target_bb != tail && fall_thru_bb != tail)
      edge_needed = TRUE;

    if (BB_preds_len(tail) > 2) {
      // replace old tn in the phi OP      
      // New tn is last in phi.
      TN *result[1];
      UINT8 j = 0;
      UINT8 nopnds = OP_opnds(phi)-1;
      TN *opnd[nopnds];
      for (i = 0; i < OP_opnds(phi); i++) {
        if (i != taken_pos && i != nottaken_pos)
          opnd[j++] = OP_opnd(phi, i);
      }
      opnd[j] = select_tn;

      result[0] = OP_result(phi, 0);

      OP *new_phi = Mk_VarOP (TOP_phi, 1, nopnds, result, opnd);
      OP_MAP_Set(phi_op_map, phi, new_phi);
    }
    else {
      // Mark phi to be deleted.
      old_phis.push_front(phi);
    }
  }
  
  // It's time to remove the useless conditional branch.
  // before the following blocks are promoted.
  BB_Remove_Op(head, br_op);

  br_op = NULL;

  // promote the instructions from the sides bblocks.
  // Promote_BB will remove the old empty bblock
  // BB containing a goto should be last.
  // keep the goto for later.
  if (target_bb != tail) {
    br_op = BB_Remove_Branch(target_bb);
    Promote_BB(target_bb, head);
  }
  if (fall_thru_bb != tail) {
    br_op = BB_Remove_Branch(fall_thru_bb);
    Promote_BB(fall_thru_bb, head);
  }

  // Promoted instructions might not be global anymore.
  BB_Localize_Tns (head);

  //  fprintf (TFile, "\nafter promotion\n");
  //  Print_All_BBs();

  // to avoid extending condition's lifetime, we keep the comp instruction
  // at the end, just before the selects.
  //  BB_Move_Op_To_End(head, head, cmp);
 
  if (Trace_Select_Gen) {
    fprintf(TFile, "<select> Insert selects ");
    Print_OPS (&cmov_ops); 
    fprintf (TFile, "\n");
  }

  // Cleanup phis that are going to be replaced ...
  BB_Remove_Ops(tail, old_phis);
  // ... by the selects
  BB_Append_Ops(head, &cmov_ops);

  BB_Fix_Spec_Loads (head);
  BB_Fix_Spec_Stores (head, cond_tn, variant);

  // create a new edge.
  if (!edge_needed)
    Unlink_Pred_Succ (head, tail);

  Link_Pred_Succ_with_Prob(head, tail, 1.0);

  //  fprintf (TFile, "simple fall through BB%d BB%d done\n", BB_id(head), BB_id(tail));
  //  Print_All_BBs();

  // finally, if we had a branch, put it back.
  // goto should be the last instruction.
  if (br_op) {
    BB *fall_thru = BB_Fall_Thru_Successor(head);
    if (fall_thru != tail) {
      Add_Goto(head, tail);
      //      BB_Append_Op (head, br_op);
    }
    else
      br_op = NULL;
  }

  // If the 2 blocks can be merged, only for those that would be interesting
  // candidates for eventual nested hammocks.
  if (Can_Merge_BB (head, tail, br_op)) {
    BB_Merge(head, tail);
    BBLIST *succ;
    
    FOR_ALL_BB_SUCCS(head, succ) {
      BB *bb = BBLIST_item(succ);
      OP *op;
      
      FOR_ALL_BB_PHI_OPs(bb,op) {
        // make phi here instead of that...
        Change_PHI_Predecessor (bb, tail, head, op);
      }
    }

    tail = head;
  }
  
  // Insert new phis and update ssa information.
  BB_Update_Phis(tail);
}

/* ================================================================
 *
 *   Entry
 *
 * ================================================================
 */
void
Convert_Select(RID *rid, const BB_REGION& bb_region)
{
  INT i;

  Set_Error_Phase ("Select Region Formation");

  Trace_Select_Init();

  if (Trace_Select_Spec) {
    fprintf (TFile, "<select> speculative model is eager ");
    switch (Eager_Level) {
    case EAGER_NONE:
      fprintf (TFile, "none\n");      
      break;
    case EAGER_SAFE:
      fprintf (TFile, "safe\n");      
      break;
    case EAGER_ARITH:
      fprintf (TFile, "arith\n");      
      break;
    case EAGER_DIVIDE:
      fprintf (TFile, "divide\n");      
      break;
    case EAGER_MEMORY:
      fprintf (TFile, "memory\n");      
      break;
    default:
      fprintf (TFile, "%d\n", Eager_Level);      
      break;
    }
  }

  Initialize_Memory();

  if (Trace_Select_Candidates)
    Trace_IR(TP_SELECT, "Before Select Region Formation", NULL);

  Identify_Hammock_Candidates();

  Calculate_Dominators();

  //  draw_CFG();

  for (i = 0; i < max_cand_id; i++) {
    BB *bb = cand_vec[i];
    BB *bbb;

    if (bb == NULL) continue;

    if (bbb = Is_Double_Logif(bb)) {
      if (Trace_Select_Gen) {
        fprintf (TFile, "\n********** BEFORE LOGICAL OPTS BB%d ***********\n",
                 BB_id(bb));
        Print_All_BBs();
        fprintf (TFile, "******************************************\n");
      }
        
      Initialize_Hammock_Memory();
      Simplify_Logifs(bb, bbb);
      Finalize_Hammock_Memory();
      cand_vec[BB_MAP32_Get(postord_map, bbb)-1] = NULL;
      
      if (Trace_Select_Gen) {
        fprintf (TFile, "---------- AFTER LOGICAL OPTS -------------\n");
        Print_All_BBs();
        fprintf (TFile, "------------------------------------------\n");
      }
    }
    clear_spec_lists();
  }

  for (i = 0; i < max_cand_id; i++) {
    BB *bb = cand_vec[i];
    BB *tail;
    BB *target_bb;
    BB *fall_thru_bb;
    
    if (bb == NULL) continue;

    if (Is_Hammock (bb, &target_bb, &fall_thru_bb, &tail)) {
      if (bbs_tail_dup[0]) {
        DevWarn("<select> tail duplicate BB%d not implemented\n", BB_id (bbs_tail_dup[0]));
        //        Tail_Duplicate(bb, bbs_tail_dup[0], tail);
        continue;
      }

      if (bbs_tail_dup[1]) {
        DevWarn("<select> tail duplicate BB%d not implemented\n", BB_id (bbs_tail_dup[1]));
        //        Tail_Duplicate(bb, bbs_tail_dup[1], tail);
        continue;
      }

      if (Trace_Select_Gen) {
        fprintf (TFile, "\n********** BEFORE SELECT FOLD BB%d ************\n",
                 BB_id(bb));
        Print_All_BBs();
        fprintf (TFile, "******************************************\n");
      }
      
      Initialize_Hammock_Memory();
      Select_Fold (bb, target_bb, fall_thru_bb, tail);
      Finalize_Hammock_Memory();

      if (Trace_Select_Gen) {
        fprintf (TFile, "---------- AFTER SELECT FOLD -------------\n");
        Print_All_BBs();
        fprintf (TFile, "------------------------------------------\n");
      }
    }
    clear_spec_lists();
  }

  //  draw_CFG();

  GRA_LIVE_Recalc_Liveness(rid);

  if (Trace_Select_Stats) {
    CG_SELECT_Statistics();
  }
  
  Finalize_Select();
}

/* ================================================================
 *
 * daVincy output of CFG.
 *
 * ================================================================
 */
static char *
sPrint_TN ( 
  TN *tn, 
  char *buf 
)
{
  char *result = buf;

  if (tn == NULL) {
    buf += sprintf ( buf, "--nil--");
    return result;
  }

  if (TN_is_constant(tn)) {
    if ( TN_has_value(tn)) {
      buf += sprintf ( buf, "(0x%llx)", TN_value(tn) );
      if (TN_size(tn) == 4 && 
	  TN_value(tn) >> 32 != 0 &&
	  TN_value(tn) >> 31 != -1)
	buf += sprintf ( buf, "!!! TN_value=0x%llx is too big to fit in a word",
			 TN_value(tn));
    }
    else if (TN_is_enum(tn)) {
      buf += sprintf ( buf, "(enum:%s)", ISA_ECV_Name(TN_enum(tn)) );
    }
    else if ( TN_is_label(tn) ) {
      LABEL_IDX lab = TN_label(tn);
      const char *name = LABEL_name(lab);
      INT64 offset = TN_offset(tn);
      if ( offset == 0 ) {
	buf += sprintf ( buf, "(lab:%s)", name );
      }
      else {
	buf += sprintf ( buf, "(lab:%s+%lld)", name, offset );
      }
    } 
    else if ( TN_is_tag(tn) ) {
      LABEL_IDX lab = TN_label(tn);
      const char *name = LABEL_name(lab);
      buf += sprintf ( buf, "(tag:%s)", name );
    }
    else if ( TN_is_symbol(tn) ) {
      ST *var = TN_var(tn);

      buf += sprintf ( buf, "(sym" );
      buf += sprintf ( buf, TN_RELOCS_Name(TN_relocs(tn)) );

      if (ST_class(var) == CLASS_CONST)
      	buf += sprintf ( buf, ":%s)", Targ_Print(NULL, ST_tcon_val(var)));
      else
      	buf += sprintf ( buf, ":%s%+lld)", ST_name(var), TN_offset(tn) );
    } 
  }
  else {  /* register TN */
    if (TN_is_global_reg(tn)) {
      buf += sprintf ( buf, "GTN%d", TN_number(tn) );
    }
    else {
      buf += sprintf ( buf, "TN%d", TN_number(tn) );
    }
    if (TN_register(tn) != REGISTER_UNDEFINED) {
      if (TN_register(tn) <= REGISTER_CLASS_last_register(TN_register_class(tn))) {
	buf += sprintf (buf, "(%s)", 
		REGISTER_name(TN_register_class(tn), TN_register(tn)));
      } else {
	buf += sprintf (buf, "(%d,%d)", TN_register_class(tn), TN_register(tn));
      }
    }
    if (TN_is_save_reg(tn)) {
	buf += sprintf (buf, "(sv:%s)", 
		REGISTER_name(TN_save_rclass(tn), TN_save_reg(tn)));
    }
  }
  
  return result;
}

static const char *
Node(BB* bb)
{
  static char *buffer = NULL;
  static INT buf_size = 0;

  /* estimate the size of buffer needed:
   */
  const INT size =  25	+ (BB_length (bb) * 128);

  if (buf_size == 0) {
    buffer = (char *)malloc(size);
    buf_size = size;
  } else if (size > buf_size) {
    while (size > buf_size) buf_size *= 2;
    buffer = (char *)realloc(buffer, buf_size);
  }

  char *p = buffer;
  OP* op;

  FOR_ALL_BB_OPs_FWD(bb, op) {  
    int i;
 
   for (i = 0; i < OP_results(op); i++) {
     p = sPrint_TN (OP_result(op, i), p);
     p += strlen (p);
     p += sprintf(p, " ");
    }

   if (OP_results(op))
     p += sprintf(p, " = ");

   p += sprintf(p, " %s ", TOP_Name(OP_code(op)));

    for (i = 0; i < OP_opnds(op); i++) {
     p = sPrint_TN (OP_opnd(op, i), p);
     p += strlen (p);
     p += sprintf(p, " ");
    }

    p += sprintf(p, "\\n");
  }

  FmtAssert(p < buffer + size, ("Node_Label buffer size estimate too small"));

  return buffer;
}

extern char *Cur_PU_Name;

void
draw_CFG(void)
{
  if (! Trace_Select_daVinci) return;
  if (! DaVinci::enabled (TRUE)) return;

  MEM_POOL dv_pool;
  dv_pool.magic_num = 0;		// force it to be unintialized
  MEM_POOL_Constructor pool (&dv_pool, "DaVinci", FALSE);

  DaVinci dv (&dv_pool, NULL);

  dv.Title (Cur_PU_Name);
  dv.Show_Status("select");

  // Now we start drawing
  NODE_TYPE nt_plain, nt_entry, nt_exit, nt_multi,nt, nt_call;
  EDGE_TYPE et_known;

  nt_entry.Color ("palegreen");
  nt_exit.Color ("pink");
  nt_exit.Name ("exit");
  nt_call.Boarder(NB_DOUBLE);
  nt_multi.Color ("lightgoldenrod");
  nt_multi.Shape(NS_CIRCLE);

  dv.Graph_Begin ();

  // add all nodes
  for (BB* bb = REGION_First_BB; bb != NULL; bb = BB_next(bb)) {
    char buf[10];
    nt = nt_plain;
    if (BBlist_Len(BB_preds(bb)) > 1 || BBlist_Len(BB_succs(bb)) > 1) nt = nt_multi;
    if (BB_entry(bb)) nt = nt_entry;
    if (BB_exit(bb))  nt = nt_exit;
    if (BB_call(bb))  nt = nt_call;
    sprintf (buf, "%d", BB_id(bb));
    nt = nt.Name(buf);
    dv.Node_Begin (NODE_ID (bb), Node(bb), nt);
    BBLIST* sedge;
    FOR_ALL_BB_SUCCS(bb, sedge) {
      dv.Out_Edge (EDGE_ID (NODE_ID (bb), NODE_ID (BBLIST_item(sedge))),
		   et_known,
		   NODE_ID (BBLIST_item(sedge)));
    }
    dv.Node_End ();
  }
  dv.Graph_End ();

  dv.Event_Loop (NULL);
}
