/*
  Copyright (C) 2001, STMicroelectronics, All Rights Reserved.

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
 * Module: cgtarget.cxx
 * $Revision$
 * $Date$
 * $Author$
 * $Source$
 *
 * Description:
 *
 * If-convert simple regions into selects.
 *
 * ====================================================================
 * ====================================================================
 */

#include <list.h>
#include <stack.h>
#include "defs.h"
#include "config.h"
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
#include "findloops.h"
#include "gra_live.h"
#include "cxx_memory.h"
#include "cg_ssa.h"
#include "exp_targ.h"
#include "cg_select.h"
#include "DaVinci.h"

#include "hb.h"
#include "hb_id_candidates.h"

#include "hb_trace.h"

static BB_MAP postord_map;      // Postorder ordering
static BB     **cand_vec;       // Vector of potential hammocks BBs.
static INT32  max_cand_id;	// Max value in hammock candidates.

// Mapping between old and new PHIs
static OP_MAP phi_op_map = NULL;

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
static BOOL Trace_Select_Stats;      /* -Wb,-tt61:0x010 */
static BOOL Trace_Select_daVinci;    /* -Wb,-tt61:0x020 */

static void
Trace_Select_Init() {
  Trace_Select_Candidates = Get_Trace(TP_SELECT, Select_Candidates);
  Trace_Select_Gen        = Get_Trace(TP_SELECT, Select_Gen);
  Trace_Select_Spec       = Get_Trace(TP_SELECT, Select_Spec);
  Trace_Select_Merge      = Get_Trace(TP_SELECT, Select_Merge);
  Trace_Select_Stats      = Get_Trace(TP_SELECT, Select_Stats);
  Trace_Select_daVinci    = Get_Trace(TP_SELECT, Select_daVinci);
}

MEM_POOL MEM_Select_pool;
UINT select_count;
UINT spec_instrs_count;

static void
CG_SELECT_Statistics()
{
  if (select_count) {
    fprintf (stdout, "========= %s ======= \n", ST_name(Get_Current_PU_ST()));
    fprintf (stdout, "<cg_select> converted %d moves \n", select_count);
  }
  if (spec_instrs_count) {
    fprintf (stdout, "<cg_select> %d instrs were converted \n", spec_instrs_count);
  }
}

void 
Select_Init()
{
  Trace_Select_Init();
}

static void
Initialize_Memory()
{
  static BOOL did_init = FALSE;

  if ( ! did_init ) {
    MEM_POOL_Initialize(&MEM_Select_pool,"Select pool",FALSE);
    did_init = TRUE;
  }
  MEM_POOL_Push(&MEM_Select_pool);
  MEM_POOL_Push(&MEM_local_pool);
}

static void
Finalize_Memory()
{
  Free_Dominators_Memory();
  MEM_POOL_Pop(&MEM_local_pool);
  MEM_POOL_Pop(&MEM_Select_pool);
}

static void
Finalize_Select(void)
{
  Finalize_Memory();

  OP_MAP_Delete(phi_op_map);
  BB_MAP_Delete(postord_map);
  max_cand_id = 0;
  select_count = 0;
  spec_instrs_count = 0;
}

static BB*
Find_Immediate_Postdominator(BB* bb, BB* succ1, BB* succ2)
{
  BB *ipdom;

  if (BB_SET_MemberP(BB_pdom_set(bb), succ1))
    return succ1;

  if (BB_SET_MemberP(BB_pdom_set(bb), succ2))
    return succ2;

  if ((ipdom = BB_Unique_Successor(succ1)) == BB_Unique_Successor(succ2))
    if (ipdom && ! BB_SET_MemberP(BB_pdom_set(bb), ipdom)) 
      return NULL;

  return ipdom;
}

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

UINT max_select_instrs = 1000;

// List of of memory accesses found in if-then-else region. 
// Load and stores lists are used in a slightly different manner:
// Stores are mapped with their equivalent. (this might be improved with a dummy
// store location).
// Loads are mapped with their dismissible form. They don't need to have an
// equivalent (a load on the other side of the hammock).
// I keep these operation in a list because we don't want to touch the basic
// blocks now. (we don't know yet if the hammock will be reduced).
// OPs will be updated in BB_Fix_Spec_Loads and BB_Fix_Spec_Stores.

typedef list<OP*> op_list;

pair<op_list, op_list> store_i;
pair<op_list, op_list> load_i;

// temporary: I don't like the interface.
static BOOL
Can_Speculate(BOOL first, OP *op)
{
  // not problem.
  if (OP_Can_Be_Speculative(op))
    return TRUE;
  
  if (OP_memory (op)) {
    if (Eager_Level < EAGER_MEMORY || OP_volatile(op))
      return FALSE;

    if (OP_load (op)) {
      TOP ld_top;
      OP *lop;

      if ((ld_top = CGTARG_Speculative_Load (op)) == TOP_UNDEFINED)
        return FALSE;

      lop = Dup_OP (op);
      OP_Change_Opcode(lop, ld_top); 
      Set_OP_speculative(lop);             // set the OP_speculative flag.

      load_i.first.push_back(op);
      load_i.second.push_back(lop);
      
      return TRUE;
    }

    else if (OP_store (op)) {
      if (first)
        store_i.first.push_back(op);
      else
        store_i.second.push_back(op);

      return TRUE;
    }
  }

  return FALSE;
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
      store_i.second.push_back(old_op);
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
static BOOL
Check_Suitable_Hammock (BB* ipdom, BB* target, BB* fall_thru)
{
  UINT count = 0;
  OP* op;

  if (Trace_Select_Candidates) {
    fprintf(TFile, "<select> Found Hammock : ");
    fprintf(TFile, " target BB%d, fall_thru BB%d, tail BB%d\n",
            BB_id(target), BB_id(fall_thru), BB_id(ipdom));
  }
    
  //  Print_BB (target);
  //  Print_BB (fall_thru);

  // Count instructions in each side.
  // and check types.
  if (target != ipdom) {
    FOR_ALL_BB_OPs_FWD(target, op) {
      //      Print_OP (op);
      if (!Can_Speculate(TRUE, op)) return FALSE;
      ++count;
    }
  }
  
  if (count > max_select_instrs)
    return FALSE;

  count = 0;
 
 if (fall_thru != ipdom) {
    FOR_ALL_BB_OPs_FWD(fall_thru, op) {
      //      Print_OP (op);
      if (!Can_Speculate(FALSE, op)) return FALSE;
      ++count;
    }
  }
  
 if (!store_i.first.empty() || !store_i.second.empty()) {
   UINT nmem = BB_Check_Memops();
   if (nmem == 0)
     return FALSE;
   count += nmem*4;
 }

 return count <= max_select_instrs;
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
  *fall_thru = BB_Fall_Thru_Successor(head);
  FOR_ALL_BB_SUCCS(head, edge) {
    *target = BBLIST_item(edge);
    if (*target != *fall_thru) break;
  }

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
      if (BB_succs_len (bb) != 1 || BB_preds_len (bb) != 1)
        return FALSE; 

      found_taken = TRUE;
    }
    else if (bb == *fall_thru) {
      if (BB_succs_len (bb) != 1 || BB_preds_len (bb) != 1)
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

/* ====================================================================
 *
 * Promote_BB
 *
 * Delete a BB, i.e. moving all its OPs to its predecessor
 * and remove it from the list of BBs for the region/PU.
 *
 * ====================================================================
 */
static void
Promote_BB(BB *bp, BB *to_bb)
{
  BBLIST *edge;
  
  /* Sanity checks.
     We can only have one predecessor, where cond is set.
   */
  BB *pred = BB_Unique_Predecessor(bp);
  DevAssert(to_bb == pred, ("Promote_BB"));

  // Sanity checks.Thoses cases cannot happen inside a hammock
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
Region_Change_Phis (BB *bb, OP *phi)
{
  BBLIST *edge;
  TN *old_tn = OP_result (phi, 0);
  OP *new_phi;
  OP *op;
  UINT i;

  FOR_ALL_BB_SUCCS(bb, edge) {
    BB *succ = BBLIST_item(edge);
    
    FOR_ALL_BB_PHI_OPs(succ,op) {

      fprintf (TFile, "found phi \n");
      Print_OP (op);

      for (i = 0; i < OP_opnds(op); i++) {
        Print_TN (OP_opnd (op, i), FALSE);

        if (OP_opnd (op, i) == old_tn) {
          UINT8 nopnds = OP_opnds(op);
          UINT j;
          TN *result[1];
          TN *opnd[nopnds];
          result[0] = OP_result(op, 0);

          for (j = 0; j < nopnds; j++) {
            opnd[j] = OP_opnd(op, j);
          }

          new_phi = Mk_VarOP (TOP_phi, 1, nopnds, result, opnd);

          fprintf (TFile, "OLD PHI WAS \n");
          Print_OP (phi);

          fprintf (TFile, "NEW PHI IS \n");
          Print_OP (new_phi);

          SSA_Prepend_Phi_To_BB (new_phi, bb);
          BB_Remove_Op(bb, phi);
          break;
        }
      }
    }
  }
}

// Remove old phis, and if necessary replace with new one.
// Note that we cannot use the same list to insert selects because they
// would be inserted in the 'head' bblock.
static void
BB_Update_Phis(BB *bb)
{
  OP *phi;
  list<OP *> phi_list;
  
  FOR_ALL_BB_PHI_OPs(bb,phi) {
    OP *new_phi = (OP *)OP_MAP_Get(phi_op_map, phi);

    if (new_phi != phi) {
      if (Trace_Select_Gen) {
        fprintf(TFile, "<select> Insert phi ");
        Print_OP (new_phi); 
        fprintf (TFile, "\n");
      }

      SSA_Prepend_Phi_To_BB(new_phi, bb);
    }
    else {
      Region_Change_Phis(bb, phi);
    }
    
    if (Trace_Select_Gen) {
      fprintf(TFile, "<select> Remove phi ");
      Print_OP (phi); 
      fprintf (TFile, "\n");
    }

    phi_list.push_back(phi);

    // cleanup phi_op_map and ssa_pool ?
    // maintain DF ?
  }

  op_list::iterator phi_iter = phi_list.begin();
  op_list::iterator phi_end  = phi_list.end();
  for (; phi_iter != phi_end; ++phi_iter) {
    BB_Remove_Op(bb, *phi_iter);
  }

  phi_list.clear();
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
  // Transfer call info if merged block will now contain a call.
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

    TN *temp = CGTARG_gen_select_dest_TN();

    Expand_Select (temp, cond_tn, true_tn, false_tn, MTYPE_I4, FALSE, &ops);
    Expand_Store (MTYPE_I4, temp, OP_opnd(i1, 1), OP_opnd(i1, 0), &ops);

    BB_Insert_Ops_After (bb, i2, &ops);

    BB_Remove_Op (bb, i1);
    BB_Remove_Op (bb, i2);

    store_i.first.pop_front();
    store_i.second.pop_front();
  }
}

static void
Select_Fold (BB *head, BB *target_bb, BB *fall_thru_bb, BB *tail)
{
  OP *phi;
  OP *new_phi;
  BOOL edge_needed = FALSE;

  //  fprintf (TFile, "\nin Select_Fold\n");
  //  Print_All_BBs();

  //  fprintf (TFile, "Select_Fold %d %d %d %d\n", BB_id(head), BB_id (target_bb),
  //           BB_id (fall_thru_bb), BB_id (tail));

  UINT8 taken_pos = Get_In_Edge_Pos(target_bb == tail ? head : target_bb, tail);
  UINT8 nottaken_pos = Get_In_Edge_Pos(fall_thru_bb == tail ? head : fall_thru_bb, tail);

  //  fprintf (TFile, "pos: taken = %d, not_taken = %d\n", taken_pos, nottaken_pos);

  // keep a list of newly created conditional move / compare
  // and phis to remove
  OPS cmov_ops = OPS_EMPTY;

  TN *cond_tn;
  TN *tn2;
  OP *cmp;
  OP *br_op = BB_branch_op (head);

  // find the instruction that set the condition.
  VARIANT variant = CGTARG_Analyze_Compare(br_op, &cond_tn, &tn2, &cmp);

  FOR_ALL_BB_PHI_OPs(tail, phi) {
    UINT8 i;
    TN *true_tn = NULL;
    TN *false_tn = NULL;
    
    if (Trace_Select_Gen) {
      fprintf(TFile, "<select> handle phi ");
      Print_OP (phi);
      fprintf(TFile, "\n");      
    }

    for (i = 0; i < OP_opnds(phi); i++) {
      TN *tn = OP_opnd(phi,i);
      if (i == taken_pos) {
        if (variant == V_BR_P_TRUE)
          true_tn = tn;
        else if (variant == V_BR_P_FALSE)
          false_tn = tn;
        else
          DevAssert(FALSE, ("variant"));    
      }
      if (i == nottaken_pos)  {
        if (variant == V_BR_P_TRUE)
          false_tn = tn;
        else if (variant == V_BR_P_FALSE)
          true_tn = tn;
        else
          DevAssert(FALSE, ("variant"));    
      }
    }

    // deal with undef operands.
    if (!true_tn && !false_tn)  {
      DevAssert(FALSE, ("undef true and false tns. not yet"));
    }

    // for now we only handle single result tn.
    // we don't need to go through (i = 0; i < OP_results(op); i++) loop.
    DevAssert (OP_results(phi) == 1, ("unsupported multiple results select"));
        
    TN *select_tn = OP_result (phi, 0);
    
    if (BB_preds_len(tail) != 2) 
      select_tn =  Dup_TN(select_tn);
    
    Expand_Select (select_tn, cond_tn, true_tn, false_tn,
                   MTYPE_I4, FALSE, &cmov_ops);

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

      new_phi = Mk_VarOP (TOP_phi, 1, nopnds, result, opnd);
    }
    else
      new_phi = phi;

    if (Trace_Select_Gen) {
      fprintf(TFile, "<select> new phi ");
      Print_OP (new_phi);
      fprintf(TFile, "\n");  
    }

    OP_MAP_Set(phi_op_map, phi, new_phi);
  }

  // now it's time to remove the useless conditional branch.
  // BB_Remove_Branch(head);
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

  // Finally, insert the selects.
  BB_Append_Ops(head, &cmov_ops);

  BB_Fix_Spec_Loads (head);
  BB_Fix_Spec_Stores (head, cond_tn, variant);

  // create a new edge.
  if (!edge_needed)
    Unlink_Pred_Succ (head, tail);

  Link_Pred_Succ_with_Prob(head, tail, 1.0);

  //  fprintf (TFile, "simple fall through BB%d BB%d done\n", BB_id(head), BB_id(tail));
  //  Print_All_BBs();

  // update phis.
  BB_Update_Phis(tail);

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
  if (Can_Merge_BB (head, tail, br_op))
    BB_Merge(head, tail);
}

void
Convert_Select(RID *rid, const BB_REGION& bb_region)
{
  INT i;

  Set_Error_Phase ("Select Region Formation");

  Trace_Select_Init();

  if (Trace_Select_Spec) {
    fprintf (TFile, "<select> speculative model is eager");
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

  phi_op_map = OP_MAP_Create();

  if (HB_Trace(HB_TRACE_BEFORE)) {
    Trace_IR(TP_SELECT, "Before Select Region Formation", NULL);
  }

  Identify_Hammock_Candidates();

  if (! max_cand_id)
    return;
  
  Calculate_Dominators();

  // Compute_Branch_Probabilities ?

  // Iterate though candidates and find Hammocks
  // a Hammock is designated by its head.
  for (i = 0; i < max_cand_id; i++) {
    BB *bb = cand_vec[i];
    BB *tail;
    BB *target_bb;
    BB *fall_thru_bb;
    
    if (bb == NULL) continue;

    if (Is_Hammock (bb, &target_bb, &fall_thru_bb, &tail)) {
      if (Trace_Select_Gen) {
        fprintf (TFile, "\n********** BEFORE SELECT FOLD ************\n");
        Print_All_BBs();
        fprintf (TFile, "******************************************\n");
      }
      Select_Fold (bb, target_bb, fall_thru_bb, tail);
      if (Trace_Select_Gen) {
        fprintf (TFile, "---------- AFTER SELECT FOLD -------------\n");
        Print_All_BBs();
        fprintf (TFile, "------------------------------------------\n");
      }
    }
    store_i.first.clear();
    store_i.second.clear();
  }

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
