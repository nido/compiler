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
 * Simplifies successive conditional jumps into logical expressions.
 * If-converts conditional if regions using selects and speculation.
 * This optimisation works and maintains the SSA representation.
 *
 * General Flags are:
 * -CG:select_if_convert=TRUE    enable if conversion
 * -TARG:dismiss_mem_faults=TRUE allow dismissible loads
 *
 * The following flags to drive the algorithm.
 * -CG:select_allow_dup=TRUE     remove side entries. duplicate blocks
 *                               might increase code size in some cases.
 *
 * The following flags to drive the heuristics.
 * -CG:select_factor="16.0"       gain for replacing a branch by a select
 * -CG:select_disload_cost="4.0" cost to speculate a load
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
#include "cgexp.h"
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
#include "cg_sched_est.h"
#include "cg_ssa.h"
#include "cg_select.h"
#include "DaVinci.h"

static BB_MAP postord_map;      // Postorder ordering
static BB     **cand_vec;       // Vector of potential hammocks BBs.
static INT32  max_cand_id;	// Max value in hammock candidates.
static BB_MAP if_bb_map;        // Map fall_thru and targets of logif

// Mapping between old and new PHIs
// When we change the number of predecessors of a basic block, we record
// the new phis in this map. Old phis are then replaced all in once with 
// BB_Update_Phis.
static OP_MAP phi_op_map = NULL;

// When dublicating bbs, we need to represent the new virtual number of
// predecessors and new phis with this map. We don't really add the new
// corresponding edges because they would be removed by select latter.
typedef struct {
  UINT8 npreds;
  TN    *res;
  BB    **preds;
  TN    **tns;
} phi_info;

static OP_MAP dup_bb_phi_map;

// List of of memory accesses found in if-then-else region. 
// Load and stores lists are used in a slightly different manner:
// Stores are mapped with their equivalent.
// Loads are just recorded. They will be replaced with their dismissible form
// in BB_Fix_Spec_Loads.
// I keep these operation in a list because we don't want to touch the basic
// blocks now. (we don't know yet if the hammock will be reduced).
// OPs will be updated in BB_Fix_Spec_Loads and BB_Fix_Spec_Stores.

typedef struct {
  op_list tkstrs;
  op_list ntkstrs;
  list<int> ifarg_idx;
} store_t;
store_t store_i;
op_list load_i;

/* ====================================================================
 *   flags:
 * ====================================================================
 */
BOOL CG_select_allow_dup = TRUE;
const char* CG_select_factor = "16.0";
const char* CG_select_disload_cost = "4.0";

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
UINT dup_bbs;

static void
CG_SELECT_Statistics()
{
  if (select_count || logif_count || dup_bbs) {
    fprintf (Select_TFile, "========= %s ======= \n",
             ST_name(Get_Current_PU_ST()));

    if (select_count) 
      fprintf (Select_TFile, "<cg_select> converted %d move%s\n",
               select_count, select_count > 1 ? "s" : "");

    if (logif_count) 
      fprintf (Select_TFile, "<cg_select> reduced %d logical expression%s \n",
               logif_count, logif_count > 1 ? "s" : "");

    if (spec_instrs_count) 
      fprintf (Select_TFile, "<cg_select> speculated %d instr%s \n",
               spec_instrs_count, spec_instrs_count > 1 ? "s" : "");

    if (disloads_count) 
      fprintf (Select_TFile, "<cg_select> speculated %d load%s \n",
               disloads_count, disloads_count > 1 ? "s" : "");

    if (dup_bbs) 
      fprintf (Select_TFile, "<cg_select> duplicated %d basic block%s \n",
               dup_bbs, dup_bbs > 1 ? "s" : "");
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
}

static void
Finalize_Memory()
{
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
  dup_bbs = 0;
}

static void
Initialize_Hammock_Memory()
{
  phi_op_map = OP_MAP_Create();
}

static void
clear_spec_lists()
{
  load_i.clear();
  store_i.tkstrs.clear();
  store_i.ntkstrs.clear();
  store_i.ifarg_idx.clear();
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
Find_End_Select_Region(BB* bb, BB* then_bb, BB* else_bb)
{
  BB_SET *pdom_set = BB_pdom_set(bb);
  pdom_set = BB_SET_Intersection(pdom_set,
                                 BB_pdom_set(then_bb), &MEM_Select_pool);
  pdom_set = BB_SET_Intersection(pdom_set,
                                 BB_pdom_set(else_bb), &MEM_Select_pool);

  BB *succ;
  BB *tail_bb = NULL;

  // Returns the bb in pdom_set that dominates all the others
  FOR_ALL_BB_SET_members(pdom_set, succ) {
    if (tail_bb == NULL || BB_SET_MemberP(BB_pdom_set(succ), tail_bb))
      tail_bb = succ;
  }

  BB *last_succ = then_bb;
  while ((succ = BB_Unique_Successor (last_succ)) && succ != tail_bb)
    last_succ = succ;

  if (last_succ != tail_bb && BB_succs_len(last_succ) != 1)
    return NULL;

  last_succ = else_bb;
  while ((succ = BB_Unique_Successor (last_succ)) && succ != tail_bb)
    last_succ = succ;

  if (last_succ != tail_bb && BB_succs_len(last_succ) != 1)
    return NULL;

  return tail_bb;
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
  
  if (Trace_Select_Merge) {
    fprintf (Select_TFile, "BB_Merge %d %d\n",
             BB_id (bb_first), BB_id (bb_second));
    Print_BB (bb_first);
    Print_BB (bb_second);
  }

  // Branch should end up into bb_second
  OP *br = BB_Remove_Branch(bb_first);
  FmtAssert(!(br && OP_cond(br)),
            ("Unexpected conditional branch."));
  FmtAssert(BB_Unique_Successor(bb_first) == bb_second,
            ("Unexpected successor"));

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
      Link_Pred_Succ(bb_first, succ);
    }
  }
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

static UINT8 
Get_TN_Pos_in_PHI_INFO (phi_info *phi, BB *bb)
{
  UINT8 nopnds = phi->npreds;
  BB **preds    = phi->preds;

  do {
    for (UINT8 i = 0; i < nopnds; i++) {
      if (preds[i] == bb)
        return i;
    }
  } while (bb = BB_Unique_Successor (bb));

  FmtAssert(FALSE, ("didn't find pos for BB%d in phi\n", BB_id(bb)));
}

static UINT8 
Get_TN_Pos_in_PHI (OP *phi, BB *bb)
{
  UINT8 nopnds = OP_opnds(phi);

  do {
    for (UINT8 i = 0; i < nopnds; i++) {
      if (Get_PHI_Predecessor (phi, i) == bb)
        return i;
    }
  } while (bb = BB_Unique_Successor (bb));

  FmtAssert(FALSE, ("didn't find pos for BB%d in phi\n", BB_id(bb)));
}


// After changing the CFG, SSA information must be maintained
static void
BB_Recomp_Phis (BB *bb, BB *new_pred, BB *old_pred, BOOL replace)
{
  OP *phi;

  FOR_ALL_BB_PHI_OPs(bb, phi) {
    BBLIST* edge;
    TN *result[1];
    TN *opnd[OP_opnds(phi)];
    UINT8 edge_pos = 0;

    FOR_ALL_BB_PREDS (bb, edge) { 
      BB *pred = BBLIST_item(edge);
      if (! (replace && pred == old_pred)) {
        UINT8 pos = Get_TN_Pos_in_PHI (phi, pred == new_pred ? old_pred : pred);
        opnd[edge_pos++] = OP_opnd(phi, pos);
      }
    }

    result[0] = OP_result(phi, 0);
    OP *new_phi = Mk_VarOP (TOP_phi, 1, edge_pos, result, opnd);
    OP_MAP_Set(phi_op_map, phi, new_phi);
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

// 
// Check if all the OPs of a bb can be speculated. Save memory references
// for later use.
//
static BOOL
Can_Speculate_BB(BB *bb, op_list *stores)
{
  OP* op;

  // don't duplicate blocks with stores
  if (BB_preds_len(bb) > 1)
    stores = NULL;

  FOR_ALL_BB_OPs_FWD(bb, op) {
    if (! OP_Can_Be_Speculative(op)) {
      if (OP_memory (op) && !OP_volatile(op)) {

        if (OP_load (op)) {
          if (! Force_Memory_Dismiss)
            return FALSE;
          load_i.push_front(op);
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

// 
// Check wether 2 memory operations are aliased memory references
//
static BOOL
Are_Aliased(OP* op1, OP* op2)
{
  if (OP_memory(op1) && OP_memory(op2)) {
    WN *wn1 = Get_WN_From_Memory_OP(op1);
    WN *wn2 = Get_WN_From_Memory_OP(op2);
    if (wn1 != NULL && wn2 != NULL) {
      ALIAS_RESULT alias = Aliased(Alias_Manager, wn1, wn2);
      if (alias == SAME_LOCATION)
        return TRUE;
    }
  }

  return FALSE;
}

// Try to sort the stores operations using alias information.
// If the sort failed that means we cannot speculate the blocks. abort
// if conversion by returning FALSE.
// If the list could be sorted, the stores will be promoted. returns the
// number of stores to help the heuristics.
static UINT
Sort_Stores(void)
{
  UINT count = 0;

  // We don't have the same count of store. Can't spec any.
  if (store_i.tkstrs.size() != store_i.ntkstrs.size())
    return 0;

  // Each store should have an equiv.
  op_list::iterator i1_iter = store_i.tkstrs.begin();
  op_list::iterator i1_end  = store_i.tkstrs.end();
  op_list::iterator i2_iter = store_i.ntkstrs.begin();
  op_list::iterator i2_end  = store_i.ntkstrs.end();
  UINT8 c = 0;

  while(i1_iter != i1_end) {
    OP *op1 = *i1_iter;
    OP *op2 = *i2_iter;

    // Stores don't differ, nothing to do. Select the result
    if (Are_Aliased (op1, op2)) {
      store_i.ifarg_idx.push_back(-1);
      ++count;
      c = 0;
      ++i1_iter;
      ++i2_iter;
      continue;
    }

    // Try to select on one of the tns.
    UINT8 ci = 0;
    UINT8 lidx = 0;
    for (UINT8 i = 0; i < OP_opnds(op1); i++) {
      if (OP_opnd(op1, i) != OP_opnd(op2, i)) {
        ci++;
        lidx = i ;
      }
    }

    // same ops ?
    if (ci == 0) {
      ci = 1;
      lidx = 2;
    }

    // one arg differs. Select it.
    if (ci <= 1) {
      store_i.ifarg_idx.push_back(lidx);
      ++count;
      c = 0;
      ++i1_iter;
      ++i2_iter;
    }
    else {
      i2_iter = store_i.ntkstrs.erase(i2_iter);
      // *i1_iter didn't match. failed
      if (count + c++ == store_i.ntkstrs.size())
        return 0;
      store_i.ntkstrs.insert(store_i.ntkstrs.end(), op2);
    }
  }

  return count;
}

// Test if a hammock is suitable for select conversion

static BOOL
Check_Profitable_Select (BB *head, BB *taken, BB_SET *region1, BB_SET *region2)
{
  BB *bb;
  BBLIST *bblist;
  BBLIST *bb1;
  BBLIST *bb2;

  FOR_ALL_BB_SUCCS(head, bblist) {
    BB *bb = BBLIST_item(bblist);
    if (bb == taken)
      bb1 = bblist;
    else
      bb2 = bblist;
  }

  UINT32 exp_len = BB_length(head);

  float prob1 = BBLIST_prob(bb1);
  float prob2 = BBLIST_prob(bb2);

  UINT cycles1 = 0;
  UINT cycles2 = 0;

  FOR_ALL_BB_SET_members(region1, bb) {
    exp_len += BB_length(bb);

    CG_SCHED_EST *se = CG_SCHED_EST_Create(bb, &MEM_Select_pool, 
                                           SCHED_EST_FOR_IF_CONV);

    if (Trace_Select_Candidates)
      CG_SCHED_EST_Print(Select_TFile, se);

    cycles1 += CG_SCHED_EST_Cycles(se);
    CG_SCHED_EST_Delete(se);
  }

  FOR_ALL_BB_SET_members(region2, bb) {
    exp_len += BB_length(bb);

    CG_SCHED_EST *se = CG_SCHED_EST_Create(bb, &MEM_Select_pool, 
                                           SCHED_EST_FOR_IF_CONV);

    if (Trace_Select_Candidates)
      CG_SCHED_EST_Print(Select_TFile, se);

    cycles2 += CG_SCHED_EST_Cycles(se);
    CG_SCHED_EST_Delete(se);
  }

  // higher est_cost_branch means ifc more aggressive.
  UINT32 est_cost_branch = atoi(CG_select_factor);

  // If new block is bigger that CG_bblength_max, reject.
  //  if ((exp_len - est_cost_branch) >= CG_split_BB_length)
  //    return FALSE;

  UINT32 mem1 = 0;
  UINT32 mem2 = 0;

  // check speculative memory loads costs.
  op_list::iterator i_iter;
  op_list::iterator i_end;
  i_iter = load_i.begin();
  i_end = load_i.end();
  while(i_iter != i_end) {
    if (BB_SET_MemberP(region1, OP_bb(*i_iter)))
      mem1++;
    else if (BB_SET_MemberP(region2, OP_bb(*i_iter)))
      mem2++;
    else
      DevAssert(FALSE, ("invalid spec load."));

    i_iter++;
  }

  // cost to speculate a load.
  UINT32 disload_cost =  atoi(CG_select_disload_cost);
  mem1 *= disload_cost;
  mem2 *= disload_cost;

  if (Trace_Select_Candidates) {
    fprintf (Select_TFile, "region1: cycles %d, mem %d, prob %f\n",
             cycles1, mem1, prob1);    
    fprintf (Select_TFile, "region2: cycles %d, mem %d, prob %f\n",
             cycles2, mem2, prob2);
  }

  cycles1 += mem1;
  cycles2 += mem2;

  // pondarate cost of each region taken separatly.
  float est_cost_bbs = (((float)(cycles1) * prob1) + ((float)(cycles2) * prob2));

  // cost of if converted region. prob is one. Remove branch.
  float est_cost_ifc = (float)cycles1 + cycles2 - est_cost_branch;
  
  if (Trace_Select_Candidates) {
    fprintf (Select_TFile, "ifc region: cycles %d, saving branch %d\n",
             cycles1 + cycles2, est_cost_branch);
    fprintf (Select_TFile, "Comparing without ifc:%f, with ifc:%f\n",
             est_cost_bbs, est_cost_ifc);
  }

  // If estimated cost of if convertion is a win, do it.
  return est_cost_ifc <= est_cost_bbs;
}

static BOOL
Check_Suitable_Hammock (BB* ipdom, BB* target, BB* fall_thru,
                        BB_SET** t_path, BB_SET** ft_path)
{
  if (Trace_Select_Candidates) {
    fprintf(Select_TFile, "<select> Found Hammock : ");
    fprintf(Select_TFile, " target BB%d, fall_thru BB%d, tail BB%d\n",
            BB_id(target), BB_id(fall_thru), BB_id(ipdom));
  }
    
  BB *bb = target;

  while (bb != ipdom) {
    DevAssert(bb, ("Invalid BB chain in hammock"));

    // allow removing of side entries only on one of targets.
    if (BB_preds_len (bb) > 1 && (!CG_select_allow_dup || bb != target))
      return FALSE; 

   if (! Can_Speculate_BB(bb, &store_i.tkstrs))
     return FALSE;

   *t_path  = BB_SET_Union1(*t_path, bb, &MEM_Select_pool);
   bb = BB_Unique_Successor (bb);
  }
  
  bb = fall_thru;

  while (bb != ipdom) {
    DevAssert(bb, ("Invalid BB chain in hammock"));

    if (BB_preds_len (bb) > 1 && (!CG_select_allow_dup || bb != fall_thru))
      return FALSE; 

    if (! Can_Speculate_BB(bb, &store_i.ntkstrs))
      return FALSE;

    *ft_path  = BB_SET_Union1(*ft_path, bb, &MEM_Select_pool);
    bb = BB_Unique_Successor (bb);
  }
  
  // Check if we have the same set of memory stores in both sides.
  if (!store_i.tkstrs.empty() || !store_i.ntkstrs.empty()) {
    if (!Sort_Stores())
      return FALSE;
  }

  return TRUE;
}

static BOOL
Is_Hammock (BB *head, BB **target, BB **fall_thru, BB **tail) 
{
  BBLIST *edge;
  BOOL found_taken = FALSE;
  BOOL found_not_taken = FALSE;

  if (Trace_Select_Candidates) {
    fprintf (Select_TFile, "<select> is hammock BB%d ? \n", BB_id(head));
    Print_All_BBs();
    Print_BB_Header (head, FALSE, TRUE);
  }

  // Find fall_thru and taken BBs.
  BB_Fall_Thru_and_Target_Succs(head, fall_thru, target);

  if (Trace_Select_Candidates) {
    fprintf (Select_TFile, "<select> target BB%d ? fall_thru BB%d\n",
             BB_id(*target), BB_id(*fall_thru));
  }

  // Starting from immediate post dominator, get the 2 conditional blocks.
  *tail = Find_End_Select_Region(head, *target, *fall_thru);
  if (*tail == NULL)
    return FALSE;

  BB_SET *t_set  = BB_SET_Create_Empty(PU_BB_Count, &MEM_Select_pool);
  BB_SET *ft_set = BB_SET_Create_Empty(PU_BB_Count, &MEM_Select_pool);

  if (Check_Suitable_Hammock (*tail, *target, *fall_thru, &t_set, &ft_set)) {
    return Check_Profitable_Select(head, *target, t_set, ft_set);
  }
  else
    return FALSE;
}

/* ================================================================
 *
 *   Select If Conversion
 *
 * ================================================================
 */
#ifdef Is_True_On
static void
Sanity_Check()
{
  for (BB *bb = REGION_First_BB; bb != NULL; bb = BB_next(bb)) {
    OP *phi;
    BBLIST *edge;

    FOR_ALL_BB_PHI_OPs(bb, phi) {
      UINT8 count = 0;

      if (OP_opnds(phi) != BB_preds_len(bb)) {
        fprintf (TFile, "for bb BB%d\n", BB_id(bb));
        Print_All_BBs();
        DevAssert(FALSE, ("ssa: invalid phi\n"));
      }

      FOR_ALL_BB_PREDS(bb, edge) {      
        BB *bp = BBLIST_item(edge);
        BB *pred = Get_PHI_Predecessor (phi, count);

        if (pred != bp) {
          fprintf (TFile, "pred BB%d, bp BB%d at pos %d\n", BB_id(pred), BB_id(bp), count);
          fprintf (TFile, "\n+++++++++++++\n");
          Print_All_BBs();
          fprintf (TFile, "\n+++++++++++++\n");
          fprintf (TFile, "bb is: \n");
          Print_BB (bb);
          fprintf (TFile, "phi is: \n");
          Print_OP (phi);
          DevAssert (FALSE, ("ssa invalid phi operands"));
        }

        count++;
      }
    }
  }
}
#endif

/////////////////////////////////////
static void
Rename_Locals(OP* op, hTN_MAP dup_tn_map)
/////////////////////////////////////
//
//  Each TN's must be renamed in duplicated block in the selected region, 
//  Note that we assume we're processing the ops in forward order.
//  If not, the way we map the new names below won't work.
//
/////////////////////////////////////
{
  INT i = 0;
  TN *res, *new_tn;

  for (i = 0; i < OP_results(op); i++) {
    res = OP_result(op, i);
    if (TN_is_register(res) &&
	!(TN_is_dedicated(res) || TN_is_global_reg(res))) {
      new_tn = Dup_TN(res);
      hTN_MAP_Set(dup_tn_map, res, new_tn);
      Set_OP_result(op, i, new_tn);
    }
  }

  i = 0;
  for (INT opndnum = 0; opndnum < OP_opnds(op); opndnum++) {
    res = OP_opnd(op, opndnum);
    if (TN_is_register(res) &&
	!(TN_is_dedicated(res) || TN_is_global_reg(res))) {
      new_tn = (TN*) hTN_MAP_Get(dup_tn_map, res);
      Set_OP_opnd(op, i, new_tn);
    }
    i++;
  }
}

static void 
Update_Mem_Lists (OP *op, OP *new_op)
{
  op_list::iterator i_iter;
  op_list::iterator i_end;

  i_iter = store_i.tkstrs.begin();
  i_end = store_i.tkstrs.end();
  while(i_iter != i_end) {
    if (*i_iter == op) {
      store_i.tkstrs.erase(i_iter);
      store_i.tkstrs.insert(i_end, new_op);
      break;
    }
    i_iter++;
  }

  i_iter = store_i.ntkstrs.begin();
  i_end = store_i.ntkstrs.end();
  while(i_iter != i_end) {
    if (*i_iter == op) {
      store_i.ntkstrs.erase(i_iter);
      store_i.ntkstrs.insert(i_end, new_op);
      break;
    }
    i_iter++;
  }

  i_iter = load_i.begin();
  i_end = load_i.end();
  while(i_iter != i_end) {
    if (*i_iter == op) {
      load_i.erase(i_iter);
      load_i.insert(i_end, new_op);
      break;
    }
    i_iter++;
  }
}

/////////////////////////////////////
static void
Rename_Globals(OP* op, hTN_MAP dup_tn_map)
/////////////////////////////////////
//
//  Each TN's must be renamed in duplicated block in the selected region, 
//  Note that we assume we're processing the ops in forward order.
//  If not, the way we map the new names below won't work.
//
/////////////////////////////////////
{
  INT i;
  TN *res, *new_tn;

  for (i = 0; i < OP_results(op); i++) {
    res = OP_result(op, i);
    if (TN_is_register(res) && TN_is_global_reg(res) && 
	!TN_is_dedicated(res)) {
      new_tn = Dup_TN(res);
      // make sure new tn is marked as global.
      Set_TN_is_global_reg (new_tn);
      hTN_MAP_Set(dup_tn_map, res, new_tn);
      Set_OP_result(op, i, new_tn);
    }
  }

  i = 0;
  for (INT opndnum = 0; opndnum < OP_opnds(op); opndnum++) {
    res = OP_opnd(op, i);
    if (TN_is_register(res) && TN_is_global_reg(res) && 
	!TN_is_dedicated(res)) {
      new_tn = (TN*) hTN_MAP_Get(dup_tn_map, res);
      if (new_tn) 
        Set_OP_opnd(op, i, new_tn);
    }
    i++;
  }
}

static void
Rename_PHIs(hTN_MAP dup_tn_map, BB *head, BB *tail, BB *dup)
{
  OP *phi;

  FOR_ALL_BB_PHI_OPs(tail, phi) { 
    phi_info *phi_i = NULL;
    phi_info *new_phi;

    UINT8 nopnds;
    TN **opnd;
    BB **preds;

    // Check if this phi was already remapped.
    if (phi_i = (phi_info*)OP_MAP_Get(dup_bb_phi_map, phi)) {
      nopnds = phi_i->npreds;

      // nopnds+1 to make room for the new tn.
      opnd = (TN**)MEM_POOL_Realloc(&MEM_Select_pool,
                                    phi_i->tns, nopnds * sizeof(TN*),
                                    (nopnds+1) * sizeof(TN*));
      preds = (BB**)MEM_POOL_Realloc(&MEM_Select_pool,
                                     phi_i->preds, nopnds * sizeof(BB*),
                                     (nopnds+1) * sizeof(BB*));

      new_phi = phi_i;
    }
    else {
      nopnds = OP_opnds(phi);
      
      // nopnds+1 to make room for the new tn.
      opnd = (TN**)MEM_POOL_Alloc(&MEM_Select_pool,
                                  (nopnds+1) * sizeof(TN*));
      preds = (BB**)MEM_POOL_Alloc(&MEM_Select_pool,
                                   (nopnds+1) * sizeof(BB*));

      new_phi = (phi_info *)MEM_POOL_Alloc(&MEM_Select_pool, sizeof(phi_info));
      OP_MAP_Set(dup_bb_phi_map, phi, new_phi);
    }

    // Create and map the new phi.
    UINT8 i = 0;
    TN *old_tn = NULL;
    for (; i < nopnds; i++) {
      TN *res  = phi_i ? phi_i->tns[i] : OP_opnd(phi, i);
      BB *pred = phi_i ? phi_i->preds[i] : Get_PHI_Predecessor(phi, i); 

      if (pred == dup) {
        TN *new_tn = (TN*) hTN_MAP_Get(dup_tn_map, res);
        if (new_tn) {
          opnd[i] = new_tn;
          old_tn = res;
        }
        else {
          old_tn = opnd[i] = res;
        }
      }
      else {
        opnd[i] = res;
      }

      preds[i] = pred;
    }

    opnd[i] = old_tn;
    preds[i] = head;

    new_phi->npreds = nopnds+1;
    new_phi->res    = OP_result(phi, 0);
    new_phi->preds  = preds;
    new_phi->tns    = opnd;
  }
}

//  Copy <old_bb> and all of its ops into BB.
static void
Copy_BB_For_Duplication(BB* bp, BB* to_bb)
{
  BB* new_bb = NULL;
  hTN_MAP dup_tn_map = hTN_MAP_Create(&MEM_local_pool);
  OPS new_ops = OPS_EMPTY;  
  OPS old_ops = OPS_EMPTY;
  op_list old_phis;

  BB *succ = BB_Unique_Successor (bp);
  DevAssert (succ, ("Copy_bb"));

  if (Trace_Select_Candidates) {
    fprintf (Select_TFile, "<select> Duplicating BB%d\n", BB_id(bp));
  }

  //
  // Copy the ops to the new block.
  //
  OP *op;
  MEM_POOL_Push(&MEM_local_pool);

  FOR_ALL_BB_OPs_FWD(bp, op) {
    if (OP_code (op) == TOP_phi) {
      UINT8 nopnds = 0;
      TN *opnd[BB_preds_len(bp)-1];
      
      for (UINT8 i = 0; i < OP_opnds(op); i++) {
        if (Get_PHI_Predecessor (op, i) == to_bb) {
          Exp_COPY (OP_result (op, 0), OP_opnd (op, i), &new_ops);
        }
        else {
          opnd[nopnds++] = OP_opnd (op, i);
        }
      }

      // If the old phi had 2 TNs, each one is made into a copy.
      // It it had more than 2 TNs, must maintain SSA by creating a new phi.
      if (nopnds == 1) {
        Exp_COPY (OP_result (op, 0), opnd[0], &old_ops);
        // mark phi to be deleted.
        old_phis.push_front(op);
      }
      else {
        TN *result[1];

        result[0] = OP_result (op, 0);
        OP *new_phi = Mk_VarOP (TOP_phi, 1, nopnds, result, opnd); 
        Rename_Locals (new_phi, dup_tn_map);
        Rename_Globals (new_phi, dup_tn_map);
        OP_MAP_Set(phi_op_map, op, new_phi);
      }

      if (OPS_length(&old_ops) != 0) {
        Rename_Locals (OPS_last(&old_ops), dup_tn_map);
        Rename_Globals (OPS_last(&old_ops), dup_tn_map);
      }
    }
    else if (!OP_br (op)) {
      OP* new_op = Dup_OP (op);
      Copy_WN_For_Memory_OP(new_op, op);
      OPS_Append_Op(&new_ops, new_op);
      // Rename tns in the original block, that is becoming the
      // duplicated block. original tns are promoted using new_ops.
      Rename_Locals (op, dup_tn_map);
      Rename_Globals (op, dup_tn_map);
      if (OP_memory(op)) {
        Update_Mem_Lists (op, new_op);
      }
    }
  }

  BB_Remove_Ops(bp, old_phis);
  Rename_PHIs(dup_tn_map, to_bb, succ, bp);

  Unlink_Pred_Succ (to_bb,bp);

  BB_Update_Phis(bp);

  BB_Prepend_Ops (bp,   &old_ops);
  BB_Append_Ops (to_bb, &new_ops);

  MEM_POOL_Pop(&MEM_local_pool);

  dup_bbs++;
}

// Delete a BB, i.e. moving all its OPs to its predecessor
// and remove it from the list of BBs for the region/PU.
static void
Promote_BB(BB *bp, BB *to_bb)
{
  BBLIST *edge;
  
  // Sanity checks. Thoses cases cannot happen inside a hammock
  if (BB_loophead(bp) || BB_entry(bp) || BB_exit(bp)) {
    DevAssert(FALSE, ("Promote_BB"));
  }

  // Statistics
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
Promote_BB_Chain (BB *head, BB *bb, BB *tail)
{
  BB *bb_next;
  do {
    bb_next = BB_Unique_Successor (bb);
    BB_Remove_Branch(bb);
    Promote_BB(bb, head);
  } while ((bb = bb_next) != tail);
}

static void
BB_Fix_Spec_Loads (BB *bb)
{
  while (!load_i.empty()) {
    OP* op = load_i.front();

    TOP ld_top = CGTARG_Speculative_Load (op);
    DevAssert(ld_top != TOP_UNDEFINED, ("couldnt find a speculative load"));

    OP_Change_Opcode(op, ld_top); 
    Set_OP_speculative(op);  

    disloads_count++;

    load_i.pop_front();
  }
}

static void
BB_Fix_Spec_Stores (BB *bb, TN* cond_tn, BOOL false_br)
{
  while (!store_i.tkstrs.empty()) {
    OPS ops = OPS_EMPTY;
    TN *true_tn;
    TN *false_tn;

    OP* i1 = store_i.tkstrs.front();
    OP* i2 = store_i.ntkstrs.front();
    int i3 = store_i.ifarg_idx.front();
    DevAssert(Are_Aliased (i1, i2) || (i3 != -1), ("can't speculate stores"));

    TN *tns[3];
    tns[0] = OP_opnd(i1, 0);
    tns[1] = OP_opnd(i1, 1);
    tns[2] = OP_opnd(i1, 2);

    UINT8 idx = i3 != -1 ? i3 : 2;

    if (false_br) {
      true_tn = OP_opnd(i2, idx);
      false_tn = OP_opnd(i1, idx);
    }
    else {
      true_tn = OP_opnd(i1, idx);
      false_tn = OP_opnd(i2, idx);
    }

    TN *temp_tn = Build_TN_Like (true_tn);

    tns[idx] = temp_tn;

    Expand_Select (temp_tn, cond_tn, true_tn, false_tn, MTYPE_I4, FALSE, &ops);
    select_count++;

    Expand_Store (MTYPE_I4, tns[2], tns[1], tns[0], &ops);

   if (Trace_Select_Gen) {
     fprintf(Select_TFile, "<select> Insert selects stores in BB%d", BB_id(bb));
     Print_OPS (&ops); 
     fprintf (Select_TFile, "\n");
   }

   BB_Append_Ops (bb, &ops);

   BB_Remove_Op (bb, i1);
   BB_Remove_Op (bb, i2);

   store_i.tkstrs.pop_front();
   store_i.ntkstrs.pop_front();
   store_i.ifarg_idx.pop_front();
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
                         BB **fall_thru2, BB **target2, BOOL *cmp_invert)
{
  OP *br1 = BB_branch_op (bb1);
  OP *br2 = BB_branch_op (bb2);

  FmtAssert(br1 && OP_cond(br1),
            ("BB%d doesn't end in a conditional branch.", BB_id(bb1)));
  FmtAssert(br2 && OP_cond(br2),
            ("BB%d doesn't end in a conditional branch.", BB_id(bb2)));

  BOOL needInvert = OP_code (br1) != OP_code (br2);

  BB_Fall_Thru_and_Target_Succs(bb2, fall_thru2, target2);

  if (target1 == *target2 || fall_thru1 == *fall_thru2) {
    *cmp_invert = needInvert;
    return TRUE;
  }

  if (target1 == *fall_thru2 || fall_thru1 == *target2) {
    // if needInvert, just swap the branches
    if (needInvert) {
      BB *tmpbb = *fall_thru2;
      *fall_thru2 = *target2;
      *target2 = tmpbb;

      logif_info *logif = (logif_info *)BB_MAP_Get(if_bb_map, bb2);
      DevAssert(logif, ("Select: unmapped logif"));

      logif->target = *target2;
      logif->fall_thru = *fall_thru2;

      *cmp_invert = FALSE;
      return TRUE;
    }

    *cmp_invert = TRUE;
    return TRUE;
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
// possible (with respect to the available ops) to transform any pattern in
// a pure and or or expression.
// Normalizejumps will try to retrieve such a pattern.
static BB*
Is_Double_Logif(BB* bb)
{
  BB *fall_thru, *target, *sec_fall_thru, *sec_target;
  BOOL cmp_invert;

  // Find fall_thru and taken BBs.
  BB_Fall_Thru_and_Target_Succs(bb, &fall_thru, &target);

  if (BB_kind(target) == BBKIND_LOGIF) {
    if (BB_preds_len(target) == 1
        && Can_Speculate_BB(target, NULL)
        && Dead_BB (target)
        && Prep_And_Normalize_Jumps(bb, target, fall_thru, target,
                                    &sec_fall_thru, &sec_target, &cmp_invert)
        && sec_fall_thru == fall_thru) {
      if (cmp_invert) {
        if (! Negate_Cmp_BB(BB_branch_op (target)))
          return NULL;
      }
      return target;
    }
  }

  // try the other side
  if (BB_kind(fall_thru) == BBKIND_LOGIF) {
    if (BB_preds_len(fall_thru) == 1
        && Can_Speculate_BB(fall_thru, NULL)
        && Dead_BB (fall_thru)
        && Prep_And_Normalize_Jumps(bb, fall_thru, fall_thru, target,
                                    &sec_fall_thru, &sec_target, &cmp_invert)
        && sec_target == target) {
      if (cmp_invert) {
        if (! Negate_Cmp_BB(BB_branch_op (fall_thru)))
          return NULL;
      }
      return fall_thru;
    }
  }
  
  return NULL;
}

static void 
Simplify_Logifs(BB *bb1, BB *bb2)
{
  BB *bb1_fall_thru, *bb1_target, *bb2_fall_thru, *bb2_target;
  BB *else_block, *joint_block;
  BOOL AndNeeded;

  // Get condif cached information
  BB_Fall_Thru_and_Target_Succs(bb1, &bb1_fall_thru, &bb1_target);
  BB_Fall_Thru_and_Target_Succs(bb2, &bb2_fall_thru, &bb2_target);

  // Check optimisation type.
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
  TN *branch_tn1, *branch_tn2;
  TN *dummy;
  VARIANT variant = CGTARG_Analyze_Branch(br1_op, &branch_tn1, &dummy);
  CGTARG_Analyze_Branch(br2_op, &branch_tn2, &dummy);

  // Find the defining OPs for the tested values.
  DEF_KIND kind;
  OP *cmp_op1 = TN_Reaching_Value_At_Op(branch_tn1, br1_op, &kind, TRUE);
  OP *cmp_op2 = TN_Reaching_Value_At_Op(branch_tn2, br2_op, &kind, TRUE);

  DevAssert(cmp_op1 && cmp_op2, ("undef op for branch"));

  BOOL  false_br = V_false_br(variant);
  TN *label_tn = OP_opnd(br1_op, OP_find_opnd_use(br1_op, OU_target));

  // make joint_block a fall_thru of bb1.
  BOOL invert_branch = FALSE;
  if (BB_Fall_Thru_Successor (bb1) == bb2 &&
      BB_Fall_Thru_Successor (bb2) == joint_block)
    invert_branch = TRUE;

  BB_Remove_Branch(bb1);
  BB_Remove_Branch(bb2);
  Promote_BB(bb2, bb1);

  OPS ops = OPS_EMPTY;

  // we need a branch_tn that will be the result of the logical op
  branch_tn1 = Build_TN_Like (OP_opnd(br1_op, 0));

  // Get the result of the comparaison for the logical operation.
  TN *rtn1 = Expand_CMP_Reg (cmp_op1, &ops);
  TN *rtn2 = Expand_CMP_Reg (cmp_op2, &ops);

  // !a op !a -> !(a !op b)
  AndNeeded = AndNeeded ^ false_br;

  // insert new logical op.
  if (AndNeeded)
    Expand_Logical_And (branch_tn1, rtn1, rtn2, variant, &ops);
  else
    Expand_Logical_Or (branch_tn1, rtn1, rtn2, variant, &ops);

  // Stats
  logif_count++;

  // Create the branch.
  variant = V_BR_P_TRUE;
  if (false_br) {
    Set_V_false_br(variant);
  } else {
    Set_V_true_br(variant);
  }
  Expand_Branch (label_tn, branch_tn1, NULL, variant, &ops);

  // Update ops
  BB_Append_Ops (bb1, &ops);

  // Update branch probabilities
  if (AndNeeded)
    prob1 = 1.0 - (prob1 * prob2);
  else
    prob1 = MAX (prob1, prob2);

  // Update control flow 
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
    
  BBLIST *succ;
  FOR_ALL_BB_SUCCS(bb1, succ) {
    BB *bb = BBLIST_item(succ);
    BB_Recomp_Phis(bb, bb1, bb2, bb == joint_block);
    BB_Update_Phis(bb);
  }

  // Promoted instructions might not be global anymore.
  BB_Localize_Tns (bb1);

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

   if (Trace_Select_Gen) {
     fprintf (TFile, "\nSelect_Fold BB%d\n", BB_id(head));
     Print_All_BBs();
   }
   
  // keep a list of newly created conditional move / compare
  // and phis to remove
  OPS cmov_ops = OPS_EMPTY;
  op_list old_phis;

  TN *cond_tn;
  TN *tn2;
  OP *br_op = BB_Remove_Branch(head);

  VARIANT variant = CGTARG_Analyze_Branch(br_op, &cond_tn, &tn2);
  BOOL false_br = V_false_br(variant);

  BOOL edge_needed = (target_bb != tail && fall_thru_bb != tail);
  UINT8 n_preds_target    =  BB_preds_len (target_bb);
  UINT8 n_preds_fall_thru =  BB_preds_len (fall_thru_bb);
  BOOL did_duplicate_bb = FALSE;

  dup_bb_phi_map = OP_MAP_Create();

  if (target_bb != tail && n_preds_target > 1) {
    Copy_BB_For_Duplication(target_bb, head);
    target_bb = tail;
    did_duplicate_bb = TRUE;
  }
  if (fall_thru_bb != tail && n_preds_fall_thru > 1) {
    Copy_BB_For_Duplication(fall_thru_bb, head);
    fall_thru_bb = tail;
    did_duplicate_bb = TRUE;
  }

   if (Trace_Select_Gen) {
     if (did_duplicate_bb) {
       fprintf (TFile, "after tail duplication\n");
       Print_All_BBs();
     }
   }

  FOR_ALL_BB_PHI_OPs(tail, phi) {
    UINT8 npreds;
    UINT8 taken_pos, nottaken_pos;
    TN *true_tn, *false_tn;

    phi_info *phi_i = (phi_info*)OP_MAP_Get(dup_bb_phi_map, phi);

    if (phi_i) {
      taken_pos    = Get_TN_Pos_in_PHI_INFO (phi_i, target_bb == tail ?
                                             head : target_bb);
      nottaken_pos = Get_TN_Pos_in_PHI_INFO (phi_i, fall_thru_bb == tail ?
                                             head : fall_thru_bb);
      true_tn  = phi_i->tns[taken_pos];
      false_tn = phi_i->tns[nottaken_pos];
      npreds   = phi_i->npreds;
    }
    else {
      taken_pos    = Get_TN_Pos_in_PHI (phi, target_bb == tail ?
                                              head : target_bb);
      nottaken_pos = Get_TN_Pos_in_PHI (phi, fall_thru_bb == tail ?
                                              head : fall_thru_bb);
      true_tn   = OP_opnd(phi, taken_pos);
      false_tn  = OP_opnd(phi, nottaken_pos); 
      npreds    = BB_preds_len(tail);
    }

    TN *select_tn = OP_result (phi, 0);

    if (false_br) {
      TN *temp_tn = false_tn;
      false_tn = true_tn;
      true_tn = temp_tn;
    }

   if (Trace_Select_Gen) {
     if (did_duplicate_bb) {
       fprintf (TFile, "for phi res = ");
       Print_TN (phi_i->res, FALSE);
       fprintf (TFile, "\n");      
       for (UINT8 k = 0; k < phi_i->npreds; k++) {
         Print_TN (phi_i->tns[k], FALSE);
         fprintf (TFile, ":BB%d, ", BB_id(phi_i->preds[k]));
       }
       fprintf (TFile, "\n");      
     }
   }

    DevAssert(true_tn && false_tn, ("Select: undef TN"));

    if (Trace_Select_Gen) {
      fprintf (Select_TFile, "<select> converts phi %d preds ", npreds);
      fprintf (Select_TFile, " taken_pos = %d, nottaken_pos = %d\n",
               taken_pos, nottaken_pos);
      Print_OP (phi);

      if (phi_i) {
        fprintf (Select_TFile, "<select> phi cached: ");
        for (UINT i = 0; i < npreds; i++) {
          Print_TN (phi_i->tns[i], FALSE);
          fprintf (Select_TFile, " ");
        }
        fprintf (Select_TFile, "\n");
      }
    }

    if (npreds > 2) {
      TN *result[1];
      UINT8 j = 0;
      UINT8 nopnds = npreds - 1;
      TN *opnd[nopnds];

      select_tn = Dup_TN(select_tn);
      
      // new args goes last if we know that a new edge will be inserted.
      // else, replace old phi tn with newly create select tn.
      if (edge_needed) {
        for (UINT8 i = 0; i < npreds; i++) {
          if (i != taken_pos && i != nottaken_pos)
            opnd[j++] = phi_i ? phi_i->tns[i] : OP_opnd(phi, i);
        }
        opnd[j] = select_tn;
      }
      else {
        for (UINT8 i = 0; i < npreds; i++) {
          if (i != taken_pos && i != nottaken_pos)
            opnd[j++] = phi_i ? phi_i->tns[i] : OP_opnd(phi, i);
          else if ((i == nottaken_pos || i == taken_pos) &&
                   Get_PHI_Predecessor (phi, i) == head)
            opnd[j++] = select_tn;
        }
      }
      
      result[0] = OP_result(phi, 0);

      OP *new_phi = Mk_VarOP (TOP_phi, 1, nopnds, result, opnd);
      OP_MAP_Set(phi_op_map, phi, new_phi);

      if (Trace_Select_Gen) {
        fprintf(Select_TFile, "<select> handle phi =  ");
        Print_OP (new_phi);
      }
    }
    else {
      // Mark phi to be deleted.
      old_phis.push_front(phi);
    }

    // Cannot be done before select_tn has been checked.
    Expand_Select (select_tn, cond_tn, true_tn, false_tn, MTYPE_I4,
                   FALSE, &cmov_ops);
    select_count++;
  }
  
  // Promote the instructions from the sides bblocks.
  // If the block was duplicated, instructions were already copied.
  // Promote_BB will remove the old empty bblocks
  if (target_bb != tail && n_preds_target == 1)
    Promote_BB_Chain (head, target_bb, tail);

  if (fall_thru_bb != tail && n_preds_fall_thru == 1)
    Promote_BB_Chain (head, fall_thru_bb, tail);

   // Promoted instructions might not be global anymore.
   BB_Localize_Tns (head);

   //  fprintf (TFile, "\nafter promotion\n");
   //  Print_All_BBs();

   // to avoid extending condition's lifetime, we keep the comp instruction
   // at the end, just before the selects.
   //  BB_Move_Op_To_End(head, head, cmp);

   if (Trace_Select_Gen) {
     fprintf(Select_TFile, "<select> Insert selects in BB%d", BB_id(head));
     Print_OPS (&cmov_ops); 
     fprintf (Select_TFile, "\n");
   }

   // Cleanup phis that are going to be replaced ...
   BB_Remove_Ops(tail, old_phis);
   // ... by the selects
   BB_Append_Ops(head, &cmov_ops);

   BB_Fix_Spec_Loads (head);
   BB_Fix_Spec_Stores (head, cond_tn, false_br);

   // create or update the new head->tail edge.
   if (edge_needed) {
     if (tail == BB_next(head))
       Target_Simple_Fall_Through_BB(head, tail);
     else
       Add_Goto (head, tail);
   }
   else {
     if (tail != BB_Fall_Thru_Successor(head)) {
       Add_Goto (head, tail);
     }
    Change_Succ_Prob (head, tail, 1.0);
   }

  // Cleanup CFG and maintain SSA information.
  if (Can_Merge_BB (head, tail)) {
    BB_Merge(head, tail);
    BBLIST *succ;
    
    BB_Update_Phis(head);

    FOR_ALL_BB_SUCCS(head, succ) {
      BB *bb = BBLIST_item(succ);
      BB_Recomp_Phis (bb, head, tail, FALSE);
      BB_Update_Phis(bb);
    }
  }
  else {
    BB_Update_Phis(tail);
  }

  OP_MAP_Delete(dup_bb_phi_map);

  // We created new GTNs, must do a global recalc liveness.
  if (did_duplicate_bb) 
    GRA_LIVE_Compute_Liveness_For_BB(tail);

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

  Initialize_Memory();

  if (Trace_Select_Candidates)
    Trace_IR(TP_SELECT, "Before Select Region Formation", NULL);

  Identify_Logifs_Candidates();

  Calculate_Dominators();

  if_bb_map = BB_MAP_Create();

  for (i = 0; i < max_cand_id; i++) {
    BB *bb = cand_vec[i];
    BB *bbb;

    if (bb == NULL) continue;
      
    if (bbb = Is_Double_Logif(bb)) {
      if (Trace_Select_Gen) {
        fprintf (Select_TFile, "\nStart gen logical for BB%d \n", BB_id(bb));
      }
        
      Initialize_Hammock_Memory();
      Simplify_Logifs(bb, bbb);
#ifdef Is_True_On
      Sanity_Check();
#endif

      Finalize_Hammock_Memory();
      cand_vec[BB_MAP32_Get(postord_map, bbb)-1] = NULL;
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
        fprintf (Select_TFile, "\nStart gen select for BB%d \n", BB_id(bb));
      }
      
      Initialize_Hammock_Memory();
      Select_Fold (bb, target_bb, fall_thru_bb, tail);
#ifdef Is_True_On
      Sanity_Check();
#endif

      Finalize_Hammock_Memory();
      // if bb is still a logif, that means that there was a merge.
      // need to update logif map
      if (BB_kind (bb) == BBKIND_LOGIF) {
        cand_vec[BB_MAP32_Get(postord_map, bb)-1] = bb;
        cand_vec[BB_MAP32_Get(postord_map, tail)-1] = NULL;
      }
      else
        cand_vec[BB_MAP32_Get(postord_map, bb)-1] = NULL;
    }
    clear_spec_lists();
  }

  BB_MAP_Delete(if_bb_map);

  // TODO: track liveness
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
