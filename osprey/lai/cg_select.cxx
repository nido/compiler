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
 * If-convert conditional if regions using selects and speculation.
 * Simplifies successive conditional jumps into logical expressions.
 *
 * enabled with -CG:select=yes
 * speculation level: -TENV:X=spec_level. where spec_level is:
 * 0   : no select generated.
 * 1/2 : EAGER_SAFE/EAGER_ARITH: safe arithmetic ops (default)
 * 3   : EAGER_DIVIDE division allowed
 * 4   : EAGER_MEMORY speculate memory loads (using dismissible).
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
FILE *Select_TFile;

static void
Trace_Select_Init() {
  Trace_Select_Candidates = Get_Trace(TP_SELECT, Select_Trace_Candidates);
  Trace_Select_Gen        = Get_Trace(TP_SELECT, Select_Trace_Gen);
  Trace_Select_Spec       = Get_Trace(TP_SELECT, Select_Trace_Spec);
  Trace_Select_Merge      = Get_Trace(TP_SELECT, Select_Trace_Merge);
  Trace_Select_Dup        = Get_Trace(TP_SELECT, Select_Trace_Dup);
  Trace_Select_Stats      = Get_Trace(TP_SELECT, Select_Trace_Stats);
  Trace_Select_daVinci    = Get_Trace(TP_SELECT, Select_Trace_daVinci);

  if (Get_Trace(TP_SELECT, Select_Trace_stdout))
    Select_TFile = stdout;
  else
    Select_TFile = TFile;
}

MEM_POOL MEM_Select_pool;
UINT select_count;
UINT logif_count;
UINT spec_instrs_count;
UINT disloads_count;

static void
CG_SELECT_Statistics()
{
  if (select_count || logif_count) {
    fprintf (Select_TFile, "========= %s ======= \n",
             ST_name(Get_Current_PU_ST()));

    if (select_count) 
      fprintf (Select_TFile, "<cg_select> converted %d moves\n",
               select_count);

    if (logif_count) 
      fprintf (Select_TFile, "<cg_select> reduced %d logical expressions \n",
               logif_count);

    if (spec_instrs_count) 
      fprintf (Select_TFile, "<cg_select> speculated %d instrs \n",
               spec_instrs_count);

    if (disloads_count) 
      fprintf (Select_TFile, "<cg_select> speculated %d loads \n",
               disloads_count);
  }
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
  disloads_count = 0;
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

  if ((ipdom = BB_Unique_Successor(then_bb)) != BB_Unique_Successor(else_bb))
    return NULL;

  return ipdom;
}

static void
BB_Fall_Thru_and_Target_Succs(BB *bb, BB **fall_thru, BB **target)
{
  BBLIST *edge;
  logif_info *logif;

  DevAssert (BB_kind(bb) == BBKIND_LOGIF, ("BB_Fall_Thru_and_Target_Succs"));

  logif = (logif_info *)BB_MAP_Get(if_bb_map, bb);
  if (logif) {
    *fall_thru = logif->fall_thru;
    *target    = logif->target;
    return;
  }

  *fall_thru = BB_Fall_Thru_Successor(bb);
  FOR_ALL_BB_SUCCS(bb, edge) {
    *target = BBLIST_item(edge);
    if (*target != *fall_thru) break;
  }

  DevAssert (*fall_thru && *target, ("BB_Fall_Thru_and_Target_Succs"));
  
  logif = (logif_info*)MEM_POOL_Alloc(&MEM_Select_pool, sizeof(logif_info));
  logif->fall_thru = *fall_thru;
  logif->target    = *target;
  BB_MAP_Set(if_bb_map, bb, logif);

  return;
}

static BOOL
Can_Merge_BB (BB *bb_first, BB *bb_second)
{
  if (BB_Fall_Thru_Successor(bb_first) != bb_second)
    return FALSE;

  if (BB_succs_len (bb_first) != 1 || BB_preds_len (bb_second) != 1)
    return FALSE;

  if (BB_Has_Exc_Label(bb_second) ||
      BB_Has_Addr_Taken_Label(bb_second) ||
      BB_Has_Outer_Block_Label(bb_second))
    return FALSE;

  /* Reject if merged BB will be too large.
   */
  if (BB_length(bb_first) + BB_length(bb_second) >= CG_split_BB_length)
    return FALSE;

  return TRUE;
}

static void
BB_Merge (BB *bb_first, BB *bb_second)
{
  BB *succ;
  BBLIST *bl;
  OP *br;

  if (Trace_Select_Merge) {
    fprintf (Select_TFile, "BB_Merge %d %d\n", BB_id (bb_first), BB_id (bb_second));
    Print_BB (bb_first);
    Print_BB (bb_second);
  }

  //
  // Move all of its ops to bb_first.
  //
  OP* op_next;
  OP* last = BB_last_op(bb_first);
  for (OP *op = BB_first_op(bb_second); op; op = op_next) {
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
    for (UINT8 i = 0; i < OP_results(op); i++) {
      TN *tn = OP_result(op, 0);

      if (!GTN_SET_MemberP(BB_live_out(bb),tn))
        Reset_TN_is_global_reg(tn); 
    }
  }
}

/* ================================================================
 *
 *   Misc routines to manipulate the SSA's Phis.
 *
 * ================================================================
 */
// After replacing 2 preds with one, SSA information must be maintained
// by replacing the phis instead of fixing them.
static void
BB_Recomp_Phis (BB *bb)
{
  OP *phi;

  FOR_ALL_BB_PHI_OPs(bb, phi) {
    BBLIST* edge;
    TN *result[1];
    UINT8 nopnds = OP_opnds(phi)-1;
    TN *opnd[nopnds];
    UINT8 pred_num = 0;

    FOR_ALL_BB_PREDS (bb, edge) { 
      BB *pred = BBLIST_item(edge);
      for (UINT8 i = 0; i < OP_opnds(phi); i++) { 
        if (Get_PHI_Predecessor (phi, i) == pred) {
          opnd[pred_num] = OP_opnd(phi, i);
          break;
        }
      }
      pred_num++;
    }

    result[0] = OP_result(phi, 0);
    OP *new_phi = Mk_VarOP (TOP_phi, 1, nopnds, result, opnd);
    OP_MAP_Set(phi_op_map, phi, new_phi);
  }
}

static UINT8 
Get_TN_Pos_in_PHI (OP *phi, BB *bb)
{
  UINT8 nopnds = OP_opnds(phi);

  for (UINT8 i = 0; i < nopnds; i++) {
    if (Get_PHI_Predecessor (phi, i) == bb)
      return i;
  }

  FmtAssert(FALSE, ("didn't find pos for BB%d in phi\n", BB_id(bb)));
}

static void
Change_PHI_Predecessor (OP *phi, BB *old_pred, BB *pred)
{
  UINT8 nopnds = OP_opnds(phi);

  for (UINT8 i = 0; i < nopnds; i++) {
    if (Get_PHI_Predecessor (phi, i) == old_pred)
      Set_PHI_Predecessor (phi, i, pred);
  }
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

    if (new_phi) {
      phi_list.push_front(phi);
      SSA_Prepend_Phi_To_BB(new_phi, bb);
    }
  }
  
  BB_Remove_Ops(bb, phi_list);
}

/* ================================================================
 *
 *   If blocks Selection
 *
 * ================================================================
 */
// Create a map of conditional blocks sorted in postorder. That makes it
// easy to iterate through possible if regions.
static void
Identify_Logifs_Candidates(void)
{
  postord_map = BB_Postorder_Map(NULL, NULL);

  // maximum size.
  cand_vec = TYPE_MEM_POOL_ALLOC_N(BB *, &MEM_Select_pool, PU_BB_Count);
  max_cand_id = 0;

  // Make a list of logifs blocks
  for (BB *bb = REGION_First_BB; bb != NULL; bb = BB_next(bb)) {
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
          disloads_count++;

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
  UINT c = 0;

  while(i1_iter != i1_end) {
    if (!Are_Aliased (*i1_iter, *i2_iter)) {
      OP * old_op = *i2_iter;     
      i2_iter = store_i.second.erase(i2_iter);
      // *i1_iter didn't match. failed
      if (c++ == store_i.second.size())
        return FALSE;
      store_i.second.insert(store_i.second.end(), old_op);
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
//  later: compute dependence heigh, use freq information.

UINT max_select_instrs = 1000;

static BOOL
Check_Suitable_Hammock (BB* ipdom, BB* target, BB* fall_thru)
{
  if (Trace_Select_Candidates) {
    fprintf(Select_TFile, "<select> Found Hammock : ");
    fprintf(Select_TFile, " target BB%d, fall_thru BB%d, tail BB%d\n",
            BB_id(target), BB_id(fall_thru), BB_id(ipdom));
  }
    
  //  Print_BB (target);
  //  Print_BB (fall_thru);

  if (target != ipdom) {
    if (! Can_Speculate_BB(target, &store_i.first))
      return FALSE;
  }
  
  // very naive heuristic... change that later when the engine is finalized.
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
    fprintf (Select_TFile, "<select> is hammock BB%d ? \n", BB_id(head));
    Print_BB_Header (head, FALSE, TRUE);
  }

  // Find fall_thru and taken BBs.
  BB_Fall_Thru_and_Target_Succs(head, fall_thru, target);

  if (Trace_Select_Candidates) {
    fprintf (Select_TFile, "<select> target BB%d ? fall_thru BB%d\n",
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

      if (BB_preds_len (bb) != 1)
        return FALSE; 

      found_taken = TRUE;
    }
    else if (bb == *fall_thru) {
      if (BB_succs_len (bb) != 1)
        return FALSE; 

      if (BB_preds_len (bb) != 1)
        return FALSE; 

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
    for (UINT8 i = 0; i < OP_results(op); i++) {
      TN *tn = OP_result(op, 0);
      if (GTN_SET_MemberP(BB_live_out(bb), tn))
        return FALSE;
    }
  }
  return TRUE;
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
    // if needInvert, don't need to do anything. 
    if (needInvert) {
      BB *tmpbb = *fall_thru2;
      *fall_thru2 = *target2;
      *target2 = tmpbb;

      logif_info *logif = (logif_info *)BB_MAP_Get(if_bb_map, bb2);
      DevAssert(logif, ("Select: unmapped logif"));

      logif->target = *target2;
      logif->fall_thru = *fall_thru2;

      return TRUE;
    }

    return Negate_Cmp_BB(br2);
  }

  return FALSE;
}

// Test if bb is the head of a region that can be converted with logical
// expressions. (ex: if (p1) if (p0) ... = if (p1 && p0 && ...)
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
        && Dead_BB (target)
        && Prep_And_Normalize_Jumps(bb, target, fall_thru, target,
                                    &sec_fall_thru, &sec_target)
        && sec_fall_thru == fall_thru)
      return target;
  }

  // try the other side
  if (BB_kind(fall_thru) == BBKIND_LOGIF) {
    if (BB_preds_len(fall_thru) == 1
        && Can_Speculate_BB(fall_thru, NULL)
        && Dead_BB (fall_thru)
        && Prep_And_Normalize_Jumps(bb, fall_thru, fall_thru, target,
                                    &sec_fall_thru, &sec_target)
        && sec_target == target)
      return fall_thru;
  }
  
  return NULL;
}

static void 
Simplify_Logifs(BB *bb1, BB *bb2)
{
  BB *bb1_fall_thru, *bb1_target, *bb2_fall_thru, *bb2_target;
  BB *else_block, *joint_block;
  BOOL AndNeeded;

  BB_Fall_Thru_and_Target_Succs(bb1, &bb1_fall_thru, &bb1_target);
  BB_Fall_Thru_and_Target_Succs(bb2, &bb2_fall_thru, &bb2_target);

  if (bb1_fall_thru == bb2_fall_thru) {
    // if (a && b)
    AndNeeded = TRUE;
    else_block = bb2_target;
    joint_block = bb2_fall_thru;
  }
  else if (bb1_target == bb2_target) {
    // if (a || b)
    AndNeeded = FALSE;
    else_block = bb2_fall_thru;
    joint_block = bb2_target;
  }

  // Compute probabilities for the new edge.
  float prob1 = 0.0;
  float prob2 = 0.0;
  BBLIST *bblist;
  FOR_ALL_BB_SUCCS(bb1, bblist) {
    BB *succ = BBLIST_item(bblist);
    if (succ == joint_block) {
      prob1 = BBLIST_prob(bblist);
      break;
    }
  }
  FOR_ALL_BB_SUCCS(bb2, bblist) {
    BB *succ = BBLIST_item(bblist);
    if (succ == joint_block) {
      prob2 = BBLIST_prob(bblist);
      break;
    }
  }

  if (Trace_Select_Gen) {
    fprintf (Select_TFile, "Convert if BB%d BB%d BB%d BB%d BB%d BB%d\n",
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

  // remember succ list.
  BBLIST *succs = BB_succs(bb2);
  BB_SET *succ_set = BB_SET_Create_Empty(BB_succs_len (bb2), &MEM_Select_pool);
  FOR_ALL_BBLIST_ITEMS(succs,bblist) {
    BB *bb = BBLIST_item(bblist);
    succ_set = BB_SET_Union1(succ_set, bb, &MEM_Select_pool);
  }

  // make joint_block a fall_thru of bb1.
  BOOL invert_branch = FALSE;
  if (BB_Fall_Thru_Successor (bb1) == bb2 &&
      BB_Fall_Thru_Successor (bb2) == joint_block)
    invert_branch = TRUE;

  BB_Remove_Branch(bb1);
  BB_Remove_Branch(bb2);
  Promote_BB(bb2, bb1);

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

  if (AndNeeded)
    prob1 = 1.0 - (prob1 * prob2);
  else
    prob1 = MAX (prob1, prob2);

  Unlink_Pred_Succ (bb1, joint_block);
  Unlink_Pred_Succ (bb1, else_block);

  BB *taken_bb;
  BB *not_taken_bb;

  if (joint_block == bb1_fall_thru) {
    taken_bb = else_block;
    not_taken_bb = joint_block;
  }
  else {
    taken_bb = joint_block;
    not_taken_bb = else_block;
  }

  if (invert_branch) {
    br1_op = BB_branch_op(bb1);
    Negate_Branch_BB (br1_op);
    Target_Logif_BB(bb1, not_taken_bb, 1.0 - prob1, taken_bb);
  }
  else
    Target_Logif_BB(bb1, taken_bb, prob1, not_taken_bb);

  BB_MAP_Set(if_bb_map, bb1, NULL);
  BB_MAP_Set(if_bb_map, bb2, NULL);
    
  BB *bb;
  FOR_ALL_BB_SET_members(succ_set, bb) {
    if (bb == joint_block) {
      BB_Recomp_Phis (bb);
      BB_Update_Phis(bb);
    }
    else {
      OP *phi;
      FOR_ALL_BB_PHI_OPs(bb, phi) {
        Change_PHI_Predecessor (phi, bb2, bb1);
      }
    }
  }

  GRA_LIVE_Compute_Liveness_For_BB(bb1);
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

  //  fprintf (TFile, "\nin Select_Fold\n");
  //  Print_All_BBs();

  //  fprintf (TFile, "Select_Fold %d %d %d %d\n", BB_id(head), BB_id (target_bb),
  //           BB_id (fall_thru_bb), BB_id (tail));


  // keep a list of newly created conditional move / compare
  // and phis to remove
  OPS cmov_ops = OPS_EMPTY;
  op_list old_phis;

  TN *cond_tn;
  TN *tn2;
  OP *br_op = BB_Remove_Branch(head);

  // find the instruction that sets the condition.
  VARIANT variant = CGTARG_Analyze_Branch(br_op, &cond_tn, &tn2);

  BOOL edge_needed = (target_bb != tail && fall_thru_bb != tail);

  FOR_ALL_BB_PHI_OPs(tail, phi) {
    UINT8 taken_pos    = Get_TN_Pos_in_PHI (phi, target_bb == tail ?
                                            head : target_bb);
    UINT8 nottaken_pos = Get_TN_Pos_in_PHI (phi, fall_thru_bb == tail ?
                                            head : fall_thru_bb);

    TN *true_tn   = OP_opnd(phi, taken_pos);
    TN *false_tn  = OP_opnd(phi, nottaken_pos); 
    TN *select_tn = OP_result (phi, 0);

    if (variant == V_BR_P_FALSE) {
      TN *temp_tn = false_tn;
      false_tn = true_tn;
      true_tn = temp_tn;
    }

    DevAssert(true_tn && false_tn, ("Select: undef TN"));

    // is it possible ?
    DevAssert (OP_results(phi) == 1, ("Multiple results phi"));

    if (Trace_Select_Gen) {
      fprintf(Select_TFile, "<select> handle phi ");
      Print_OP (phi);
      fprintf(Select_TFile, "in \n");      
      Print_BB (tail);
    }

    if (BB_preds_len(tail) > 2) {
      TN *result[1];
      UINT8 j = 0;
      UINT8 nopnds = OP_opnds(phi)-1;
      TN *opnd[nopnds];

      select_tn = Dup_TN(select_tn);

      // new args goes last if we know that a new edge will be inserted.
      // else, replace old phi tn with newly create select tn.
      if (edge_needed) {
        for (UINT8 i = 0; i < OP_opnds(phi); i++) {
          if (i != taken_pos && i != nottaken_pos)
            opnd[j++] = OP_opnd(phi, i);
        }
        opnd[j] = select_tn;
      }
      else {
        for (UINT8 i = 0; i < OP_opnds(phi); i++) {
          if (i != taken_pos && i != nottaken_pos)
            opnd[j++] = OP_opnd(phi, i);
          else if ((i == nottaken_pos || i == taken_pos) &&
                   Get_PHI_Predecessor (phi, i) == head)
            opnd[j++] = select_tn;
        }
      }

      result[0] = OP_result(phi, 0);

      OP *new_phi = Mk_VarOP (TOP_phi, 1, nopnds, result, opnd);
      OP_MAP_Set(phi_op_map, phi, new_phi);
    }
    else {
      // Mark phi to be deleted.
      old_phis.push_front(phi);
    }

    Expand_Select (select_tn, cond_tn, true_tn, false_tn, MTYPE_I4,
                   FALSE, &cmov_ops);
    select_count++;
  }
  
  // promote the instructions from the sides bblocks.
  // Promote_BB will remove the old empty bblock
  // BB containing a goto should be last.
  // keep the goto for later.
  OP *br_op1, *br_op2;
  if (target_bb != tail) {
    br_op1 = BB_Remove_Branch(target_bb);
    Promote_BB(target_bb, head);
  }
  if (fall_thru_bb != tail) {
    br_op2 = BB_Remove_Branch(fall_thru_bb);
    Promote_BB(fall_thru_bb, head);
  }
  br_op = br_op2 ? br_op2 : br_op1;

  // Promoted instructions might not be global anymore.
  BB_Localize_Tns (head);

  //  fprintf (TFile, "\nafter promotion\n");
  //  Print_All_BBs();

  // to avoid extending condition's lifetime, we keep the comp instruction
  // at the end, just before the selects.
  //  BB_Move_Op_To_End(head, head, cmp);
 
  if (Trace_Select_Gen) {
    fprintf(Select_TFile, "<select> Insert selects ");
    Print_OPS (&cmov_ops); 
    fprintf (Select_TFile, "\n");
  }

  // Cleanup phis that are going to be replaced ...
  BB_Remove_Ops(tail, old_phis);
  // ... by the selects
  BB_Append_Ops(head, &cmov_ops);

  BB_Fix_Spec_Loads (head);
  BB_Fix_Spec_Stores (head, cond_tn, variant);

  // create or update the new head->tail edge.
  if (edge_needed) {
    if (tail == BB_next(head))
      Target_Simple_Fall_Through_BB(head, tail);
    else
      Add_Goto (head, tail);
  }
  else {
    if (tail == target_bb && tail != BB_Fall_Thru_Successor(head)) {
      INT opnd;
      INT opnd_count;
      CGTARG_Branch_Info(br_op, &opnd, &opnd_count);
      if (opnd_count > 0) {
        TN *br_targ = OP_opnd(br_op, opnd);
        Is_True(opnd_count == 1, ("Branch with multiple bbs"));
        LABEL_IDX label = Gen_Label_For_BB(tail);
        Set_OP_opnd(br_op, opnd, Gen_Label_TN(label,0));
        BB_Append_Op(head, br_op);
      }
      else {
        DevAssert (FALSE, (""));
      }
    }
    Change_Succ_Prob (head, tail, 1.0);
  }

  // Simplify CFG and maintain SSA information.
  if (Can_Merge_BB (head, tail)) {
    BB_Merge(head, tail);
    BBLIST *succ;
    
    BB_Update_Phis(head);

    FOR_ALL_BB_PHI_OPs(head, phi) {
      Change_PHI_Predecessor (phi, tail, head);
    }

    FOR_ALL_BB_SUCCS(head, succ) {
      BB *bb = BBLIST_item(succ);
      
      FOR_ALL_BB_PHI_OPs(bb, phi) {
        Change_PHI_Predecessor (phi, tail, head);
      }
    }
  }
  else {
    BB_Update_Phis(tail);
    GRA_LIVE_Compute_Liveness_For_BB(tail);
  }

  GRA_LIVE_Compute_Liveness_For_BB(head);
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
    fprintf (Select_TFile, "<select> speculative model is eager ");
    switch (Eager_Level) {
    case EAGER_NONE:
      fprintf (Select_TFile, "none\n");      
      break;
    case EAGER_SAFE:
      fprintf (Select_TFile, "safe\n");      
      break;
    case EAGER_ARITH:
      fprintf (Select_TFile, "arith\n");      
      break;
    case EAGER_DIVIDE:
      fprintf (Select_TFile, "divide\n");      
      break;
    case EAGER_MEMORY:
      fprintf (Select_TFile, "memory\n");      
      break;
    default:
      fprintf (Select_TFile, "%d\n", Eager_Level);      
      break;
    }
  }

  Initialize_Memory();

  if (Trace_Select_Candidates)
    Trace_IR(TP_SELECT, "Before Select Region Formation", NULL);

  Identify_Logifs_Candidates();

  Calculate_Dominators();

  for (i = 0; i < max_cand_id; i++) {
    BB *bb = cand_vec[i];
    BB *bbb;

    if (bb == NULL) continue;

    if (bbb = Is_Double_Logif(bb)) {
      if (Trace_Select_Gen) {
        fprintf (Select_TFile, "\n********** BEFORE LOGICAL OPTS BB%d ***********\n",
                 BB_id(bb));
        Print_All_BBs();
        fprintf (Select_TFile, "******************************************\n");
      }
        
      Initialize_Hammock_Memory();
      Simplify_Logifs(bb, bbb);
      Finalize_Hammock_Memory();
      cand_vec[BB_MAP32_Get(postord_map, bbb)-1] = NULL;
      
      if (Trace_Select_Gen) {
        fprintf (Select_TFile, "---------- AFTER LOGICAL OPTS -------------\n");
        Print_All_BBs();
        fprintf (Select_TFile, "------------------------------------------\n");
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
      if (Trace_Select_Gen) {
        fprintf (Select_TFile, "\n********** BEFORE SELECT FOLD BB%d ************\n",
                 BB_id(bb));
        Print_All_BBs();
        fprintf (Select_TFile, "******************************************\n");
      }
      
      Initialize_Hammock_Memory();
      Select_Fold (bb, target_bb, fall_thru_bb, tail);
      Finalize_Hammock_Memory();
      
      if (Trace_Select_Gen) {
        fprintf (Select_TFile, "---------- AFTER SELECT FOLD -------------\n");
        Print_All_BBs();
        fprintf (Select_TFile, "------------------------------------------\n");
      }
    }
    clear_spec_lists();
  }

  //  GRA_LIVE_Recalc_Liveness(rid);

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

BE_EXPORTED extern char *Cur_PU_Name;

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
