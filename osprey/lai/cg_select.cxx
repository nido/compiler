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
 *
 * Description:
 *
 * Simplifies successive conditional jumps into logical expressions.
 * If-converts conditional if regions using selects and speculation.
 * This optimisation works and maintains the SSA representation.
 *
 * General Flags are:
 * -CG:select_if_convert=TRUE    enable if conversion
 * -CG:select_cond_loads=TRUE    enable conditional loads 
 * -CG:select_cond_stores=TRUE   enable conditional stores
 *
 * The following flags to drive the algorithm and heuristics.
 * -CG:select_allow_dup=TRUE     remove side entries. duplicate blocks
 *                               might increase code size in some cases.
 *
 * -CG:select_force_spec_load=TRUE  force conditional load to unconditional  *                                       if possible
 *
 * -CG:select_factor="1.1"      factor to reduce the cost of the 
 *                              ifconverted region
 *
 * -CG:select_freq=TRUE         heuristic based on execution frequency (optimize  *                              speed) or size of block (optimize space).
 *
 * ====================================================================
 * ====================================================================
 */

// [HK]
#if __GNUC__ >=3
#include <list>
#else
#include <list.h>
#endif // __GNUC__ >=3
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
#include "cflow.h"
#include "opt_alias_interface.h"
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

typedef struct {
  OP *phi;
  std::list<BB*> *bbs;
} phi_t;

static OP_MAP phi_op_map;

// List of of memory accesses found in if-then-else region. 
// Load and stores lists are used in a slightly different manner:
// Stores are mapped with their equivalent.
// Loads are just recorded. They will be replaced with their dismissible form
// in BB_Fix_Spec_Loads.
// I keep these operation in a list because we don't want to touch the basic
// blocks now. (we don't know yet if the hammock will be reduced).
// OPs will be updated in BB_Fix_Spec_Loads and BB_Fix_Spec_Stores.

typedef struct {
  OP *memop;
  TN *predtn;
} pred_t;
std::slist<pred_t> pred_i;

// Mapping of register class equivalent tns
static TN_MAP btn_map = NULL;

/* ====================================================================
 *   flags:
 * ====================================================================
 */
BOOL CG_select_spec_loads = TRUE;
BOOL CG_select_spec_loads_overridden = FALSE;
BOOL CG_select_spec_stores = TRUE;
BOOL CG_select_spec_stores_overidden = FALSE;
BOOL CG_select_allow_dup = TRUE;
BOOL CG_select_allow_dup_overridden = FALSE;
BOOL CG_select_force_spec_load = FALSE;
BOOL CG_select_force_spec_load_overridden = FALSE;
BOOL CG_select_freq = TRUE;
BOOL CG_select_cycles = TRUE;
const char* CG_select_factor = "1.1";
static float select_factor;
static int branch_penalty;

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
static BOOL Trace_Select_daVinci;    /* -Wb,-tt61:0x4000 */
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
  btn_map = TN_MAP_Create();
  phi_op_map = OP_MAP_Create();
}

static void
Finalize_Hammock_Memory()
{

  OP_MAP_Delete(phi_op_map);
  TN_MAP_Delete(btn_map);
}

static void
clear_spec_lists()
{
  pred_i.clear();
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

  // No postdom found
  if (tail_bb == NULL) return NULL;

  BB *last_succ = then_bb;
  if (last_succ != tail_bb)
    while ((succ = BB_Unique_Successor (last_succ)) && succ != tail_bb)
      last_succ = succ;

  if (last_succ != tail_bb && BB_succs_len(last_succ) != 1)
    return NULL;

  last_succ = else_bb;
  if (last_succ != tail_bb)
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

/* ================================================================
 *
 *   Misc routines to manipulate the SSA's Phis.
 *
 * ================================================================
 */
static UINT8 
Get_TN_Pos_in_PHI (OP *phi, BB *bb)
{
  BB *tb;

  while ((tb = BB_Unique_Successor (bb)) && tb != OP_bb (phi))
    bb = tb;

  return Get_PHI_Predecessor_Idx (phi, bb);
}

static BB*
Get_BB_Pos_in_PHI (OP *phi, BB *bb)
{
  BB *tb;

  while ((tb = BB_Unique_Successor (bb)) && tb != OP_bb (phi))
    bb = tb;

  return bb;
}

static bool
Can_Predicate_Op (OP *op)
{
  if (OP_load (op)) return PROC_has_predicate_loads();
  if (OP_store (op)) return PROC_has_predicate_stores();
  if (OP_prefetch (op)) return PROC_has_predicate_loads();

  return FALSE;
}

static bool
Need_Predicate_Op (OP *op)
{
  if (OP_Can_Be_Speculative(op))
    return FALSE;

  if (Can_Predicate_Op (op)) {
    std::slist<pred_t>::const_iterator i_iter;
    std::slist<pred_t>::const_iterator i_end;

    i_iter = pred_i.begin();
    i_end = pred_i.end();
    while(i_iter != i_end) {
      if ((*i_iter).memop == op) return !OP_has_predicate(op);
      i_iter++;
    }
  }
  return FALSE;
}

static void
Create_PSI_or_Select (TN *target_tn, TN* test_tn, TN* true_tn, TN* false_tn, OPS *cmov_ops)
{
  TN* cond_tn=True_TN;
  TN* fcond_tn=True_TN;

  INT num_results = 1;
  INT num_opnds = 4;

  TN *result[num_results];
  TN *opnd[num_opnds];

  OP *opt = TN_ssa_def(true_tn);
  OP *opf = TN_ssa_def(false_tn);

  if (!opt || !opf || (!Need_Predicate_Op (opt) && !Need_Predicate_Op (opf))) {
    if (!opt) true_tn = false_tn;
    else if (!opf) false_tn = true_tn;

    Expand_Select (target_tn, test_tn, true_tn, false_tn, 
		   TN_size(target_tn) == 8 ? MTYPE_I8: MTYPE_I4,
                   FALSE, cmov_ops);
    return;
  }

  if (Trace_Select_Gen) {
    fprintf (Select_TFile, "Create_PSI_or_Select :\n");
    fprintf (Select_TFile, "true_tn:\n");
    Print_TN (true_tn, FALSE);
    if (opt) Print_OP (opt);
    fprintf (Select_TFile, "false_tn:\n");
    Print_TN (false_tn, FALSE);
    if (opf) Print_OP (opf);
  }

  std::slist<pred_t>::const_iterator i_iter;
  std::slist<pred_t>::const_iterator i_end;

  i_iter = pred_i.begin();
  i_end = pred_i.end();
  while(i_iter != i_end) {
    if ((*i_iter).memop == opt) {
      cond_tn = (*i_iter).predtn;
      //      break;
    }
    if ((*i_iter).memop == opf) {
      fcond_tn = (*i_iter).predtn;
      //      break;
    }
    i_iter++;
  }

  // this can happen if the conditional load dominates one of the 
  // thenelse blocks
  if (OP_memory (opf) && cond_tn == True_TN && fcond_tn == True_TN) {
    Exp_Pred_Complement(Dup_TN(test_tn), NULL, test_tn, cmov_ops);
    cond_tn = OP_result(OPS_last(cmov_ops), 0);
  }

  if (OP_memory (opt) && cond_tn == True_TN && fcond_tn == True_TN) {
    fcond_tn = test_tn;
  }

  result[0] = target_tn;
  
  if (cond_tn == True_TN) {
    opnd[0] = cond_tn;
    opnd[1] = true_tn;
    opnd[2] = fcond_tn;
    opnd[3] = false_tn;
  }
  else {
    opnd[0] = fcond_tn;
    opnd[1] = false_tn;
    opnd[2] = cond_tn;
    opnd[3] = true_tn;
  }

  OP *psi_op = Mk_VarOP (TOP_psi,
			 num_results,
			 num_opnds,
			 result,
			 opnd);


  if (Trace_Select_Gen) {
    fprintf (Select_TFile, "Create_PSI_or_Select\n");
    Print_OP (psi_op);
  }

  OPS_Append_Op(cmov_ops, psi_op);
}

static void
BB_Create_Phi (UINT8 nopnds, TN *result[], TN *opnd[],
               std::list<BB*> *bbs, OP *phi) 
{
  phi_t *phi_infos;
  phi_infos = (phi_t*)MEM_POOL_Alloc(&MEM_Select_pool, sizeof(phi_t));
  phi_infos->phi = Mk_VarOP (TOP_phi, 1, nopnds, result, opnd);
  phi_infos->bbs = bbs;

  OP_MAP_Set(phi_op_map, phi, phi_infos);
}
     
// Remove old phis or replace it with a new one from phi_op_map.
// Note that we cannot use the same list to insert selects because they
// would be inserted in the 'head' bblock.
static void
BB_Update_Phis(BB *bb)
{
  OP *phi;

  for (phi = BB_first_op(bb); phi && OP_code(phi) == TOP_phi; ) {
    phi_t *phi_infos;

    phi_infos = (phi_t *)OP_MAP_Get(phi_op_map, phi);

    OP *next = OP_next(phi);
    BB_Remove_Op(bb, phi);

    if (phi_infos) {
      OP *new_phi = phi_infos->phi;

      BB_Prepend_Op(bb, new_phi);
      Initialize_PHI_map(new_phi);

      int i=0;
      while(!phi_infos->bbs->empty())
        {
          BB *pred = phi_infos->bbs->back();
          Set_PHI_Predecessor (new_phi, i++, pred);
          phi_infos->bbs->pop_back();
        }

      delete phi_infos->bbs;

      OP_MAP_Set(phi_op_map, phi, NULL);
    }

    phi = next;
  }
}

static void
BB_Recomp_Phis (BB *bb, BB *bb1, TN *cond_tn1, BB *bb2, TN *cond_tn2,
                BOOL true_br)
{
  OP *phi;
  OPS cmov_ops = OPS_EMPTY;
  op_list old_phis;

  OP *br1_op = BB_branch_op(bb1);

  FOR_ALL_BB_PHI_OPs(bb, phi) {
    UINT8 npreds = OP_opnds(phi);
    TN *select_tn = npreds > 2 ? Dup_TN(OP_result(phi, 0)) : OP_result(phi, 0);

    UINT8 tpos;
    UINT8 fpos;

    // value defined in bb1 was if true.
    if (true_br) {
      tpos = Get_TN_Pos_in_PHI (phi, bb1);
      fpos = Get_TN_Pos_in_PHI (phi, bb2);
    }
    else {
      tpos = Get_TN_Pos_in_PHI (phi, bb2);
      fpos = Get_TN_Pos_in_PHI (phi, bb1);
    }

    TN *true_tn = OP_opnd(phi, tpos);
    TN *false_tn = OP_opnd(phi, fpos);

    // cond_tn1 need to be a pred register
    if (OP_code(TN_ssa_def (cond_tn1)) == TOP_mfb)
      cond_tn1 = OP_opnd(TN_ssa_def (cond_tn1), 0);

    Create_PSI_or_Select (select_tn, cond_tn1, true_tn, false_tn, &cmov_ops);

    std::list<BB*> *bbs = new std::list<BB*>;

    if (npreds > 2) {
      BBLIST* edge;
      UINT8 j = 0;
      TN *result[1];
      UINT8 nopnds = npreds - 1;
      TN *opnd[nopnds];

      Set_TN_is_global_reg (select_tn);
      
      for (UINT8 i = 0; i < npreds; i++) {
        BB *pred = Get_PHI_Predecessor (phi, i);
        if (pred == bb1) {
          bbs->push_front(bb1);
          opnd[j++] = select_tn;
        }
        else  if (pred != bb2) {
          bbs->push_front(pred);
          opnd[j++] = OP_opnd(phi, i);
        }
      }

      result[0] = OP_result(phi, 0);

      BB_Create_Phi (nopnds, result, opnd, bbs, phi);
    }
    else
      old_phis.push_front(phi);      
  }

  BB_Remove_Ops(bb, old_phis);
  BB_Insert_Ops_Before(bb1, br1_op, &cmov_ops);

  BB_Update_Phis(bb);
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
Can_Speculate_BB(BB *bb)
{
  OP* op;

  if (BB_Has_Exc_Label(bb) ||
      BB_Has_Addr_Taken_Label(bb) ||
      BB_Has_Outer_Block_Label(bb))
    return FALSE;

  // [CG]: Nothing can be speculated
  if (Eager_Level == EAGER_NONE) return FALSE;

  FOR_ALL_BB_OPs_FWD(bb, op) {
    // can't speculate hardware registers.
    for (INT opndnum = 0; opndnum < OP_results(op); opndnum++) {
      TN *res = OP_result(op, opndnum);
      if (TN_is_register(res) && TN_is_global_reg(res) && TN_is_dedicated(res))
        return FALSE;
    }

    /* check if we are trying to speculate predicated ops */
    if (OP_has_predicate (op)) {
      TN *cond_tn = OP_opnd(op, OP_find_opnd_use(op, OU_predicate));
      pred_t memi;
      memi.memop = op;
      memi.predtn = cond_tn;
      pred_i.push_front(memi);
    }

    else if (OP_load (op)) {
      // loads will be optimized only if hardware support
      if ((Enable_Dismissible_Load && PROC_has_dismissible_load()) ||
          (Enable_Conditional_Load && PROC_has_predicate_loads())
          && CG_select_spec_loads) {
        if (!OP_has_predicate (op) && !OP_Can_Be_Speculative(op)) {
          pred_t memi;
          memi.memop = op;
          memi.predtn = 0;
          pred_i.push_front(memi);
        }
      }
      else if (!OP_Can_Be_Speculative(op))
        return FALSE;
    }

    else if (OP_store (op)) {
      // stores can always be optimized
      if (CG_select_spec_stores) {
        if (!OP_has_predicate (op)) {
          pred_t memi;
          memi.memop = op;
          memi.predtn = 0;
          pred_i.push_front(memi);
        }
      }
      else if (!OP_Can_Be_Speculative(op))
        return FALSE;
    }
    
    else if (OP_prefetch (op) && Enable_Conditional_Prefetch && PROC_has_predicate_loads()) {
      pred_t memi;
      memi.memop = op;
      memi.predtn = 0;
      pred_i.push_front(memi);
    }
    
    else if (!OP_Can_Be_Speculative(op))
      return FALSE;

  }

  return TRUE;
}

// 
// Check wether 2 memory operations are aliased memory references
//
static BOOL
Are_Same_Location(OP* op1, OP* op2)
{
  if (OP_memory(op1) && OP_memory(op2)) {
    WN *wn1 = Get_WN_From_Memory_OP(op1);
    WN *wn2 = Get_WN_From_Memory_OP(op2);
    if (wn1 != NULL && wn2 != NULL) {
      ALIAS_RESULT alias = Aliased(Alias_Manager, wn1, wn2);
#ifdef TARG_ST
      // [CG] Check that ops are not black_hole
      if (!OP_black_hole(op1) && !OP_black_hole(op2))
#endif
      if (alias == SAME_LOCATION)
        return TRUE;
      
      // alias analyser doesn't seem to be able to detect obvious cases like
      // if (a) pt = tab1 else pt = tab2;
      // remove this code when Aliased code is fixed
      if (alias == POSSIBLY_ALIASED) {
        TN *offsetTN1 = OP_opnd(op1, OP_find_opnd_use(op1, OU_offset));
        TN *baseTN1   = OP_opnd(op1, OP_find_opnd_use(op1, OU_base));
        TN *offsetTN2 = OP_opnd(op2, OP_find_opnd_use(op2, OU_offset));
        TN *baseTN2   = OP_opnd(op2, OP_find_opnd_use(op2, OU_base));

        if (offsetTN1 == offsetTN2) {
          if (TN_is_register (baseTN1) && TN_is_register (baseTN2)) {
            OP *op3 = TN_ssa_def(baseTN1);
            OP *op4 = TN_ssa_def(baseTN2);

            if (op3 && op4 && (OP_opnds (op3) == OP_opnds (op4))) {
              {
                for (int i = 0; i < OP_opnds(op3); i++) {
                  TN *r1 = OP_opnd(op3, i);
                  TN *r2 = OP_opnd(op4, i);

                  if (OP_opnd (op3, i) != OP_opnd (op4, i))
                    return FALSE;
                }

                return TRUE;
              }
            }
          }
        }
      }
    }
  }

  return FALSE;
}

static BOOL
Are_Not_Aliased(OP* op1, OP* op2)
{
  if (OP_memory(op1) && OP_memory(op2)) {
    WN *wn1 = Get_WN_From_Memory_OP(op1);
    WN *wn2 = Get_WN_From_Memory_OP(op2);
    if (wn1 != NULL && wn2 != NULL) {
      ALIAS_RESULT alias = Aliased(Alias_Manager, wn1, wn2);
      // OK only if we can prove that addresses are not aliases.
      if (alias == NOT_ALIASED)
        return TRUE;
    }
  }

  return FALSE;
}

static BOOL
Check_Profitable_Logif (BB *bb1, BB *bb2)
{
  float prob = 0.0;

#if 0
  if (BB_length(bb1) + BB_length(bb2) >= CG_split_BB_length) {
    if (Trace_Select_Candidates) {
      fprintf (Select_TFile, "expected new block too big. reject\n");
    }
    return FALSE;
  }
#endif

  if (!CG_select_cycles) {
    return TRUE;
  }

  if (CG_select_freq) {
    BBLIST *bblist;
    FOR_ALL_BB_SUCCS(bb1, bblist) {
      BB *succ = BBLIST_item(bblist);
      if (succ == bb2) {
        prob = BBLIST_prob(bblist);
        break;
      }
    }
  }
  else {
    prob = 1;
  }
  
  if (Trace_Select_Gen) {
    fprintf (Select_TFile, "Check_Profitable_Logif BB%d BB%d\n", BB_id(bb1), BB_id(bb2));
  }

  CG_SCHED_EST *se1 = CG_SCHED_EST_Create(bb1, &MEM_Select_pool,
                                          SCHED_EST_FOR_IF_CONV);
  CG_SCHED_EST *se2 = CG_SCHED_EST_Create(bb2, &MEM_Select_pool,
                                          SCHED_EST_FOR_IF_CONV);

  float cycles1 = CG_SCHED_EST_Cycles(se1);
  float cycles2 = CG_SCHED_EST_Cycles(se2);

  float bp = (bb2 == BB_Fall_Thru_Successor(bb1)) ? 0 : branch_penalty;

  // ponderate cost of each region taken separatly.
  float est_cost_before = cycles1 + bp + (cycles2 * prob);

  OP *op = BB_branch_op(bb1);
  DevAssert(op, ("Invalid conditional block"));
  CG_SCHED_EST_Ignore_Op(se1, op);
  CG_SCHED_EST_Append_Scheds(se1, se2);

  float est_cost_after = CG_SCHED_EST_Cycles(se1) / select_factor;

  CG_SCHED_EST_Delete(se1);
  CG_SCHED_EST_Delete(se2);

  return est_cost_after <= est_cost_before;
}

static BOOL
Check_Profitable_Select (BB *head, BB_SET *taken_reg, BB_SET *fallthru_reg,
                         BB *tail)
{
  BBLIST *bb1, *bb2;
  BB *bb;
  // Find block probs
  bb2 = BBlist_Fall_Thru_Succ(head);
  FOR_ALL_BB_SUCCS(head, bb1) {
    if (bb1 != bb2)
      break;
  }

  float taken_prob, fallthr_prob;
  if (CG_select_freq) {
    taken_prob = BBLIST_prob(bb1);
    fallthr_prob = BBLIST_prob(bb2);
  }
  else {
    taken_prob = 0.5;
    fallthr_prob = 0.5;
  }

  CG_SCHED_EST *sehead = CG_SCHED_EST_Create(head, &MEM_Select_pool,
                                             SCHED_EST_FOR_IF_CONV);
  float cyclesh = CG_SCHED_EST_Cycles(sehead);

  if (Trace_Select_Candidates) {
    fprintf (Select_TFile, "sehead (%f cycles) = \n", cyclesh);
    CG_SCHED_EST_Print(Select_TFile, sehead);
  }

  CG_SCHED_EST *se1 = NULL;
  CG_SCHED_EST *se2 = NULL;

  float cycles1 = 0.0;
  float cycles2 = 0.0;

  INT exp_len = BB_length(head);

  // Compute schedule estimate of taken region
  if (! BB_SET_EmptyP(taken_reg)) {
    se1 = CG_SCHED_EST_Create_Empty(&MEM_Select_pool, SCHED_EST_FOR_IF_CONV);

    FOR_ALL_BB_SET_members(taken_reg, bb) {
      exp_len += BB_length(bb);

      CG_SCHED_EST* tmp_est = CG_SCHED_EST_Create(bb, &MEM_local_pool,
                                                SCHED_EST_FOR_IF_CONV);
      CG_SCHED_EST_Append_Scheds(se1, tmp_est);
      CG_SCHED_EST_Delete(tmp_est);
    }

    cycles1 = CG_SCHED_EST_Cycles(se1);
  }

  // Compute schedule estimate of fallthru region
  if (! BB_SET_EmptyP(fallthru_reg)) {
    se2 = CG_SCHED_EST_Create_Empty(&MEM_Select_pool, SCHED_EST_FOR_IF_CONV);

    FOR_ALL_BB_SET_members(fallthru_reg, bb) {
      exp_len += BB_length(bb);

      CG_SCHED_EST* tmp_est = CG_SCHED_EST_Create(bb, &MEM_local_pool,
                                                  SCHED_EST_FOR_IF_CONV);
      CG_SCHED_EST_Append_Scheds(se2, tmp_est);
      CG_SCHED_EST_Delete(tmp_est);
    }

    cycles2 = CG_SCHED_EST_Cycles(se2);
  }

  if (Trace_Select_Candidates) {
    fprintf (Select_TFile, "taken (%f cycles) = \n", cycles1);
    if (se1)
      CG_SCHED_EST_Print(Select_TFile, se1);
    fprintf (Select_TFile, "\n");

    fprintf (Select_TFile, "fallthru (%f cycles) = \n", cycles2);
    if (se2)
      CG_SCHED_EST_Print(Select_TFile, se2);
    fprintf (Select_TFile, "\n");
  }

#if 0
  //If new block is bigger than CG_bblength_max, reject.
  if (exp_len >= CG_split_BB_length) {
    if (Trace_Select_Candidates) {
      fprintf (Select_TFile, "expected new block too big. reject\n");
    }
    return FALSE;
  }
#endif

  if (!CG_select_cycles)
    return TRUE;

  float bp = (BBLIST_item(bb1) == tail) ? 0 : branch_penalty;

  // ponderate cost of each region taken separatly.
  float est_cost_before = ((cycles1 + branch_penalty) * taken_prob) + (cycles2 * fallthr_prob) + cyclesh + bp;

  if (Trace_Select_Candidates) {
    fprintf (Select_TFile, "noifc region: head %f, bb1 %f, bb2 %f\n",
             cyclesh, cycles1, cycles2);
  }

  // cost of if converted region. prob is one.
  // Resulting block will not have branches. 

  OP *op = BB_branch_op(head);
  DevAssert(op, ("Invalid conditional block"));
  CG_SCHED_EST_Ignore_Op(sehead, op);

  if (se1) {
    FOR_ALL_BB_SET_members(taken_reg, bb)
      if (op = BB_branch_op(bb))
        CG_SCHED_EST_Ignore_Op(se1, op);
    CG_SCHED_EST_Append_Scheds(sehead, se1);
  }

  if (se2) {
    FOR_ALL_BB_SET_members(fallthru_reg, bb)
      if (op = BB_branch_op(bb))
        CG_SCHED_EST_Ignore_Op(se2, op);
    CG_SCHED_EST_Append_Scheds(sehead, se2);
  }

  cyclesh = CG_SCHED_EST_Cycles(sehead);
  CG_SCHED_EST_Delete(sehead);

  if (se1)
    CG_SCHED_EST_Delete(se1);
  if (se2)
    CG_SCHED_EST_Delete(se2);

  // cost of if converted region. prob is one. 
  float est_cost_after = cyclesh / select_factor;

  if (Trace_Select_Candidates) {
    fprintf (Select_TFile, "ifc region: BBs %f / %f\n", cyclesh, select_factor);
    fprintf (Select_TFile, "Comparing without ifc:%f, with ifc:%f\n", est_cost_before, est_cost_after);
  }

  // If estimated cost of if convertion is a win, do it.
  return est_cost_after <= est_cost_before;
}

static BOOL
Check_Suitable_Hammock (BB* ipdom, BB* target, BB* fall_thru,
                        BB_SET* t_path, BB_SET* ft_path, bool allow_dup)
{
  if (Trace_Select_Candidates) {
    fprintf(Select_TFile, "<select> Found Hammock : ");
    fprintf(Select_TFile, " target BB%d, fall_thru BB%d, tail BB%d\n",
            BB_id(target), BB_id(fall_thru), BB_id(ipdom));
  }
    
  BB *bb = target;

  while (bb != ipdom) {
    DevAssert(bb, ("Invalid BB chain in hammock"));

    BB_Update_OP_Order(bb);

    // allow removing of side entries only on one of targets.
    if (BB_preds_len (bb) > 1 &&
        (!allow_dup || (OPT_Space && BB_length(bb) > 1))) {
      if (Trace_Select_Candidates) 
        fprintf(Select_TFile, "<select> Would duplicate more than 1 side block. reject.\n");
      return FALSE; 
    }

    if (! Can_Speculate_BB(bb)) {
      if (Trace_Select_Candidates) 
        fprintf(Select_TFile, "<select> Can't speculate BB%d\n", BB_id(bb));
      return FALSE;
    }

    t_path  = BB_SET_Union1D(t_path, bb, &MEM_Select_pool);
    bb = BB_Unique_Successor (bb);
  }
  
  bb = fall_thru;

  while (bb != ipdom) {
    DevAssert(bb, ("Invalid BB chain in hammock"));

    BB_Update_OP_Order(bb);

    if (BB_preds_len (bb) > 1 &&
        (!allow_dup || (OPT_Space && BB_length(bb) > 1))) {
      if (Trace_Select_Candidates) 
        fprintf(Select_TFile, "<select> Would duplicate more than 1 side block. reject.\n");
      return FALSE; 
    }

    if (! Can_Speculate_BB(bb)) {
      if (Trace_Select_Candidates) 
        fprintf(Select_TFile, "<select> Can't speculate BB%d\n", BB_id(bb));
      return FALSE;
    }

    ft_path  = BB_SET_Union1D(ft_path, bb, &MEM_Select_pool);
    bb = BB_Unique_Successor (bb);
  }

  return TRUE;
}

static BOOL
Is_Hammock (BB *head, BB_SET *t_set, BB_SET *ft_set, BB **tail, bool allow_dup) 
{
  BBLIST *edge;
  BOOL found_taken = FALSE;
  BOOL found_not_taken = FALSE;

  BB *target;
  BB *fall_thru;

  if (Trace_Select_Candidates) {
    fprintf (Select_TFile, "<select> is hammock BB%d ? \n", BB_id(head));
  }

  // Find fall_thru and taken BBs.
  BB_Fall_Thru_and_Target_Succs(head, &fall_thru, &target);

  if (Trace_Select_Candidates) {
    fprintf (Select_TFile, "<select> target BB%d ? fall_thru BB%d\n",
             BB_id(target), BB_id(fall_thru));
  }

  // Starting from immediate post dominator, get the 2 conditional blocks.
  *tail = Find_End_Select_Region(head, target, fall_thru);
  if (*tail == NULL) {
    if (Trace_Select_Candidates) {
      fprintf (Select_TFile, "<select> hammock BB%d: reject \n", BB_id(head));
    }
    return FALSE;
  }

  if (Check_Suitable_Hammock (*tail, target, fall_thru, t_set, ft_set, allow_dup)) {
    if (Trace_Select_Candidates) {
      fprintf (Select_TFile, "<select> hammock BB%d is suitable \n",  BB_id(head));
    }
    return Check_Profitable_Select(head, t_set, ft_set, *tail);
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
Rename_TNs(BB* bp, hTN_MAP dup_tn_map)
{
  TN *new_tn;
  OP *op;

  FOR_ALL_BB_OPs_FWD(bp, op) {
    if (OP_code (op) != TOP_phi) {
      for (INT opndnum = 0; opndnum < OP_opnds(op); opndnum++) {
        TN *res = OP_opnd(op, opndnum);
        if (TN_is_register(res) && !TN_is_dedicated(res)) {
          new_tn = (TN*) hTN_MAP_Get(dup_tn_map, res);
          if (new_tn != NULL) {
            Set_OP_opnd(op, opndnum, new_tn);
          }
        }
      }

      if (OP_has_predicate (op) || TOP_is_select (OP_code (op))) {
        std::slist<pred_t>::iterator i_iter;
        std::slist<pred_t>::iterator i_end;

        i_iter = pred_i.begin();
        i_end = pred_i.end();
        while(i_iter != i_end) {
          if ((*i_iter).memop == op) {
            TN *cond_tn = (*i_iter).predtn;
            new_tn = (TN*) hTN_MAP_Get(dup_tn_map, cond_tn);
            (*i_iter).predtn = new_tn;
            break;
          }
          i_iter++;
        }
      }
    }
  }
}

static void 
Update_Predicates (OP *op, OP *new_op)
{
  std::slist<pred_t>::iterator i_iter;
  std::slist<pred_t>::iterator i_end;

  i_iter = pred_i.begin();
  i_end = pred_i.end();
  while(i_iter != i_end) {
    if ((*i_iter).memop == op) {
      (*i_iter).memop = new_op;
      break;
    }
    i_iter++;
  }
}

static void
Rename_PHIs(hTN_MAP dup_tn_map, BB *new_bb, BB *tail, BB *dup)
{
  OP *phi;
  op_list old_phis;

  FOR_ALL_BB_PHI_OPs(tail, phi) { 
    UINT8 nopnds = OP_opnds(phi)+1;
    TN *result[1];
    TN *opnd[nopnds];

    result[0] = OP_result(phi, 0);

    std::list<BB*> *bbs = new std::list<BB*>;

    TN *res1 = NULL;
    BB* tpred = Get_BB_Pos_in_PHI (phi, dup);

    for (UINT8 i = 0; i < OP_opnds(phi); i++) {
      BB *pred = Get_PHI_Predecessor (phi, i);
      TN *res = OP_opnd(phi, i);

      if (pred == tpred) {
        res1 = (TN*) hTN_MAP_Get(dup_tn_map, res);
        if (!res1) res1 = res;
      }

      bbs->push_front(pred);
      opnd[i] = res;
    }

    bbs->push_front(new_bb);
    opnd[nopnds-1] = res1;
        
    BB_Create_Phi (nopnds, result, opnd, bbs, phi);
  }

  BB_Update_Phis(tail);
}

// Create a phi for each GTNs use if there is not. GTN needs to be renamed
static void
Force_End_Tns (BB* bb, BB *tail)
{
  OP *op;

  if (Trace_Select_Dup) {
    fprintf (Select_TFile, "<select> Force_End_Tns %d %d\n", BB_id(bb), BB_id(tail));
    Print_BB (bb);
    Print_BB (tail);
  }

  FOR_ALL_BB_OPs_FWD(bb, op) {
    for (UINT8 defnum = 0; defnum < OP_results(op); defnum++) {
      TN *res = OP_result(op, defnum);

      // if the GTN is alive after the hammock, need to give it a phi.
      if (TN_is_register(res) && TN_is_global_reg(res) &&
          !TN_is_dedicated(res) && GTN_SET_MemberP(BB_live_out(tail), res)) {
        OP *phi;
        bool founddef = false;

          FOR_ALL_BB_PHI_OPs(tail, phi) {
          for (UINT8 usenum = 0; usenum < OP_opnds(phi); usenum++) {
            if (OP_opnd(phi, usenum) == res) {
              founddef = true;
              break;
            }
          }
        }
   
        if (!founddef) {
          UINT8 npreds = BB_preds_len(tail);
          TN * new_tn = Dup_TN(res);

          Set_TN_is_global_reg (new_tn);
          SSA_unset(op);
          Set_OP_result(op, defnum, new_tn);
          SSA_setup(op);
          
          // Rename uses in current BB
          OP *op2;
          for (op2 = OP_next(op); op2 != NULL; op2 = OP_next(op2)) {
            for (UINT8 usenum = 0; usenum < OP_opnds(op2); usenum++) {
              if (OP_opnd(op2, usenum) == res)
                Set_OP_opnd(op2, usenum, new_tn);
            }
          }

          BBLIST *succs;
          FOR_ALL_BB_SUCCS(bb, succs) {
            BB *succ = BBLIST_item(succs);
            if (succ != tail) {
              FOR_ALL_BB_OPs(succ, op2) { 
                for (UINT8 usenum = 0; usenum < OP_opnds(op2); usenum++) {
                  if (OP_opnd(op2, usenum) == res)
                    Set_OP_opnd(op2, usenum, new_tn);
                }
              }
            }
          }

          // Make up a phi in the tail basic block
          TN *result[1];
          TN *opnd[npreds];
          result[0] = res;

          std::list<BB*> *bbs = new std::list<BB*>;

          BBLIST *preds;
          UINT8 pos=0;

          FOR_ALL_BB_PREDS(tail,preds) {
            BB *pred = BBLIST_item(preds);
            bbs->push_front(pred);
            opnd[pos++] = new_tn;
          }
      
          phi = Mk_VarOP (TOP_phi, 1, npreds, result, opnd);

          BB_Prepend_Op(tail, phi);
          Initialize_PHI_map(phi);

          int i=0;
          while(!bbs->empty())
            {
              BB *pred = bbs->back();
              Set_PHI_Predecessor (phi, i++, pred);
              bbs->pop_back();
            }
        }
      }
    }
  }

  if (Trace_Select_Dup) {
    fprintf (Select_TFile, "<select> After Force_End_Tns %d %d\n", BB_id(bb), BB_id(tail));
    Print_BB (bb);
    Print_BB (tail);
  }
}

//  Copy <old_bb> and all of its ops into BB.
// bp is original bb
// to_bb is copy bb in hammock.

static void
Copy_BB_For_Duplication(BB* bp, BB* pred, BB* to_bb, BB *tail, BB_SET **bset)
{
  BB *first_bb = bp;
  hTN_MAP dup_tn_map = hTN_MAP_Create(&MEM_local_pool);

  OP *branch_op = BB_Remove_Branch (to_bb);

  // add new bb to set
  *bset = BB_SET_Union1D(*bset, to_bb, &MEM_Select_pool);

  do {
    op_list old_phis;
    OPS old_ops = OPS_EMPTY;

    if (Trace_Select_Dup) {
      fprintf (Select_TFile, "<select> Duplicating BB%d into BB%d\n",
               BB_id(bp), BB_id(to_bb));

      Print_BB (bp);
      Print_BB (tail);
    }

    // remove from set
    *bset = BB_SET_Difference1D(*bset, bp);

    //
    // Copy the ops to the new block.
    //
    OP *op;
    FOR_ALL_BB_OPs_FWD(bp, op) {
      if (OP_code (op) == TOP_phi) {
        UINT8 nopnds = 0;
        TN *opnd[BB_preds_len(bp)];
      
        std::list<BB*> *bbs = new std::list<BB*>;

        for (UINT8 i = 0; i < OP_opnds(op); i++) {
          BB *ppred = Get_PHI_Predecessor (op, i);
          if (ppred == pred) {
            OPS new_ops = OPS_EMPTY;  
            TN *res = OP_result (op, 0);
            TN *new_tn = Dup_TN(res);
            if (TN_is_global_reg(res)) Set_TN_is_global_reg (new_tn);
            hTN_MAP_Set(dup_tn_map, res, new_tn);

            Exp_COPY (new_tn, OP_opnd (op, i), &new_ops);
            BB_Append_Ops (to_bb, &new_ops);
          }
          else {
            bbs->push_front(ppred);
            opnd[nopnds++] = OP_opnd (op, i);
          }
        }

        // First take care of old phis that stay out of the hammock
        // If the old phi had 2 TNs, each one is made into a copy.
        // It it had more than 2 TNs, must maintain SSA by creating a new phi.
        if (nopnds == 1) {
          Exp_COPY (OP_result (op, 0), opnd[0], &old_ops);
          old_phis.push_front(op);
        }
        else {
          // phi has only one def.
          TN *result[1];
          result[0] = OP_result (op, 0);
          BB_Create_Phi (nopnds, result, opnd, bbs, op);
        }
      }

      else if (!OP_br (op)) {
        OP* new_op = Dup_OP (op);

        if (OP_memory(op)) {
          if (Trace_Select_Dup) {
            fprintf (Select_TFile, "Duplicating Memory OP \n");
            Print_OP (op);
            fprintf (Select_TFile, "\n");
          }
          Copy_WN_For_Memory_OP(new_op, op);
          Update_Predicates (op, new_op);
        }
        else if (TOP_is_select (OP_code (op))) {
          Update_Predicates (op, new_op);
        }

        // Must rename new results before BB_Append_Op because
        // we don't want to break the ssa def mapping.
        // uses will be updated in Rename_TNs. 
        for (INT opndnum = 0; opndnum < OP_results(new_op); opndnum++) {
          TN *res = OP_result(new_op, opndnum);
          if (TN_is_register(res) && !TN_is_dedicated(res)) {
            TN *new_tn = Dup_TN(res);
            if (TN_is_global_reg(res)) Set_TN_is_global_reg (new_tn);
            hTN_MAP_Set(dup_tn_map, res, new_tn);
            Set_OP_result(new_op, opndnum, new_tn);
          }
        }

        BB_Append_Op (to_bb, new_op);
      }
    }

    BB_Remove_Ops(bp, old_phis);
    old_phis.clear();

    if (OPS_length(&old_ops) != 0) {
      BB_Prepend_Ops (bp, &old_ops);
    }
    else {
      BB_Update_Phis(bp);
    }

  } while ((bp = BB_Unique_Successor (bp)) != tail);

  // go through all definitions and rename.
  Rename_TNs (to_bb, dup_tn_map);

  // put back the branch
  if (branch_op)
    BB_Append_Op (to_bb, branch_op);

  Rename_PHIs(dup_tn_map, to_bb, tail, first_bb);

  Free_Dominators_Memory();
  Calculate_Dominators();

  if (Trace_Select_Dup) {
    fprintf (Select_TFile, "<select> After Duplication BB%d into BB%d\n",  BB_id(bp), BB_id(to_bb));
    Print_All_BBs();
  }

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
  if (Trace_Select_Gen) {
    fprintf(Select_TFile, "<select> promote BB%d\n", BB_id (bb));
  }

  BB *bb_next;
  do {
    bb_next = BB_Unique_Successor (bb);
    BB_Remove_Branch(bb);
    Promote_BB(bb, head);
  } while ((bb = bb_next) != tail);
}

static void
Rename_Local_Tns (TN *tn, TN *new_tn, OP*op)
{
  do {
    for (int i = 0; i < OP_opnds(op); i++) {
      if (OP_opnd(op, i) == tn) {
        Set_OP_opnd (op, i, new_tn);
      }
    }
  } while (op = OP_next(op));
}

static TN*
Generate_Merged_Predicates(TN *pred_tn, TN *tn, VARIANT variant, OPS *ops)
{
  TN *new_tn = Dup_TN(pred_tn);
          
  OP *opb = TN_ssa_def (tn);
  if (opb && OP_code(opb) == TOP_mtb) {
    tn = OP_opnd(opb, 0);
  }
  else {
    TN *tn2 = tn;
    if (!(tn = (TN *)TN_MAP_Get(btn_map, tn2))) {
      tn = Build_RCLASS_TN(ISA_REGISTER_CLASS_integer);
      Exp_COPY(tn, tn2, ops);
      TN_MAP_Set(btn_map, tn, tn2);
    }
  }

  opb = TN_ssa_def (pred_tn);

  if (opb && OP_code(opb) == TOP_mtb) {
    pred_tn = OP_opnd(opb, 0);
  }
  else {
    TN *tn2 = pred_tn;
    if (!(pred_tn = (TN *)TN_MAP_Get(btn_map, tn2))) {
      pred_tn = Build_RCLASS_TN(ISA_REGISTER_CLASS_integer);
      Exp_COPY(pred_tn, tn2, ops);
      TN_MAP_Set(btn_map, pred_tn, tn2);
    }
  }

  Expand_Logical_And (new_tn, pred_tn, tn, variant, ops);

  //  Print_OPS (ops);
  return new_tn;
}

static void 
Associate_Mem_Predicates(TN *cond_tn, BOOL false_br,
                         BB_SET *t_set, BB_SET *ft_set, OPS *ops)
{
  if (pred_i.empty())
    return;

  TN *tn2;
  Exp_Pred_Complement(Dup_TN(cond_tn), NULL, cond_tn, ops);
  tn2 = OP_result (OPS_last(ops), 0);  

  hTN_MAP dup_tn_map = hTN_MAP_Create(&MEM_local_pool);

  std::slist<pred_t>::iterator i_iter;
  std::slist<pred_t>::iterator i_end;

  i_iter = pred_i.begin();
  i_end = pred_i.end();

  bool speculate_load = false;

  while(i_iter != i_end) {
    OP* op = (*i_iter).memop;
    
    if (BB_SET_MemberP(t_set, OP_bb (op))) {
      TN *pred_tn = (*i_iter).predtn;

      if (!pred_tn) {
        if (CG_select_force_spec_load) {
          if (OP_load(op) && !OP_has_predicate(op)) {
            WN *wn = Get_WN_From_Memory_OP(op);
            if (wn && Alias_Manager && Safe_to_speculate (Alias_Manager, wn))
              speculate_load = true;
          }
        }

        if (false_br) 
          (*i_iter).predtn = speculate_load ? True_TN : tn2;
        else
          (*i_iter).predtn = cond_tn;
      }
      else {
        TN *tn = false_br ? tn2 : cond_tn;
          
        if (pred_tn != tn) {
          TN *new_tn = (TN*) hTN_MAP_Get(dup_tn_map, pred_tn);

          if (!new_tn) {
            OPS ops = OPS_EMPTY;  

            new_tn = Generate_Merged_Predicates (pred_tn, tn, 
                                                 false_br ? V_BR_FALSE : V_BR_NONE,
                                                 &ops);

            hTN_MAP_Set(dup_tn_map, pred_tn, new_tn);
              
            OP* opb = TN_ssa_def (pred_tn);
            BB_Insert_Ops_After(OP_bb(opb), opb, &ops);
            Rename_Local_Tns((*i_iter).predtn, new_tn, OPS_last(&ops));
          }
            
          (*i_iter).predtn = new_tn;
        }
      }
    }
    else if (BB_SET_MemberP(ft_set, OP_bb (op))) {
      TN *pred_tn = (*i_iter).predtn;

      if (!pred_tn) {
        if (CG_select_force_spec_load) {
          if (OP_load(op) && !OP_has_predicate(op)) {
            WN *wn = Get_WN_From_Memory_OP(op);
            if (wn && Alias_Manager && Safe_to_speculate (Alias_Manager, wn))
              speculate_load = true;
          }
        }

        if (false_br) 
          (*i_iter).predtn = cond_tn;
        else
          (*i_iter).predtn = speculate_load ? True_TN : tn2;
      }
      else  {
        TN *tn = false_br ? cond_tn : tn2;

        if (pred_tn != tn) {
          TN *new_tn = (TN*) hTN_MAP_Get(dup_tn_map, pred_tn);

          if (!new_tn) {
            OPS ops = OPS_EMPTY;  

            new_tn = Generate_Merged_Predicates (pred_tn, tn, 
                                                 false_br ? V_BR_FALSE : V_BR_NONE,
                                                 &ops);

            hTN_MAP_Set(dup_tn_map, pred_tn, new_tn);
              
            OP* opb = TN_ssa_def (pred_tn);
            BB_Insert_Ops_After(OP_bb(op), opb, &ops);
            Rename_Local_Tns((*i_iter).predtn, new_tn, OPS_last(&ops));
          }
            
          (*i_iter).predtn = new_tn;
        }
      }
    }
    else
      DevAssert(FALSE, ("couldnt map memory op to bb"));
    
    i_iter++;
  }
}

static void 
BB_Replace_Op (OP *op, OPS *ops)
{
  BB *bb = OP_bb(op);
  OP *op_point = OP_next(op);

  BB_Remove_Op (bb, op);
  if (!op_point)
    BB_Append_Ops(bb, ops);
  else
    BB_Insert_Ops_Before(bb, op_point, ops);
}

static void
Optimize_Spec_Loads(BB *bb)
{
}

static void
BB_Fix_Spec_Loads (BB *bb)
{
  Optimize_Spec_Loads (bb);

  std::slist<pred_t>::const_iterator i_iter;
  std::slist<pred_t>::const_iterator i_end;

  i_iter = pred_i.begin();
  i_end = pred_i.end();
  while(i_iter != i_end) {

    OPS ops = OPS_EMPTY;  
    OP* op = (*i_iter).memop;

    TOP ld_top = TOP_UNDEFINED;

    if (OP_load (op)) {

      if (PROC_has_dismissible_load()) { 
        ld_top = CGTARG_Speculative_Load (op);
        DevAssert(ld_top != TOP_UNDEFINED, ("couldnt find a speculative load"));

        OP_Change_Opcode(op, ld_top); 
        Set_OP_speculative(op);  
      }

      else if (PROC_has_predicate_loads()) {
        TN *btn = (*i_iter).predtn;

        if (btn != True_TN) {
          ld_top = OP_has_predicate(op) ? OP_code(op) :
            CGTARG_Predicated_Load (op);

          Build_OP (ld_top, OP_result(op,0),
                    btn,
                    OP_opnd(op, OP_find_opnd_use(op, OU_offset)),
                    OP_opnd(op, OP_find_opnd_use(op, OU_base)),
                    &ops);

          Copy_WN_For_Memory_OP(OPS_last(&ops), op);
          BB_Replace_Op (op, &ops);
        }
      }

      disloads_count++;
    }
    else if (OP_prefetch (op)) {
      TN *btn = (*i_iter).predtn;

      ld_top = OP_has_predicate(op) ? OP_code(op) : CGTARG_Predicated_Load (op);

      Build_OP (ld_top, btn, 
                  OP_opnd(op, OP_find_opnd_use(op, OU_offset)),
                  OP_opnd(op, OP_find_opnd_use(op, OU_base)),
                  &ops);

        Copy_WN_For_Memory_OP(OPS_last(&ops), op);
        BB_Replace_Op (op, &ops);
    }

    i_iter++;
  }
}

static void
Optimize_Spec_Stores(BB *bb)
{
  TN *tn1=NULL;
  TN *tn2=NULL;

  std::slist<pred_t>::iterator i_iter;
  std::slist<pred_t>::iterator i_end;
  i_iter = pred_i.begin();
  i_end = pred_i.end();

  next_store:

  while(i_iter != i_end) {

      std::slist<pred_t>::iterator i_iter2=i_iter;
      i_iter2++;

      while (i_iter2 != i_end) {

        OP *op1 = (*i_iter).memop;
        TN *tn1 = (*i_iter).predtn;
        OP *op2 = (*i_iter2).memop;
        TN *tn2 = (*i_iter2).predtn;

        if (OP_load(op1) || OP_load(op2))
          return;

        // don't reorder store that might alias
        if (tn1 == tn2 && !Are_Not_Aliased(op1, op2))
          return;

        if (Are_Same_Location (op1, op2) && tn1 != tn2 &&
            !OP_has_predicate(op1) && !OP_has_predicate(op2)) {
          OPS ops = OPS_EMPTY;  

          Expand_Cond_Store (tn1, op1, op2, OP_find_opnd_use(op1, OU_storeval),
                             &ops);
          
          Copy_WN_For_Memory_OP(OPS_last(&ops), op1);

          if (Trace_Select_Gen) {
            
            fprintf(Select_TFile, "<select> Insert selects stores in BB%d", BB_id(bb));
            fprintf(Select_TFile, "<select> From OPs \n");
            Print_OP (op1); Print_OP (op2);
            fprintf(Select_TFile, "<select> From bTNs \n");
            Print_TN (tn1, FALSE); Print_TN (tn2, FALSE);
            fprintf(Select_TFile, "\n");
            Print_OPS (&ops); 
            fprintf (Select_TFile, "\n");

            fprintf(Select_TFile, "<select> True_TN is \n");
            Print_TN (True_TN, FALSE);
          }

          BB_Append_Ops (bb, &ops);
          BB_Remove_Op (bb, op1);
          BB_Remove_Op (bb, op2);

          i_iter2 = pred_i.erase(i_iter2);
          i_iter = pred_i.erase(i_iter);
          goto next_store;
        }
        else
          i_iter2++;
      }
      i_iter++;
    }
}


static void
BB_Fix_Spec_Stores (BB *bb)
{
  Optimize_Spec_Stores (bb);

  std::slist<pred_t>::const_iterator i_iter;
  std::slist<pred_t>::const_iterator i_end;

  i_iter = pred_i.begin();
  i_end = pred_i.end();
  while(i_iter != i_end) {

    OPS ops = OPS_EMPTY;  
    OP* op = (*i_iter).memop;
    TN *btn = (*i_iter).predtn;

    if (OP_store (op)) {
      if (PROC_has_predicate_stores() && Enable_Conditional_Store) {
        TOP st_top = OP_has_predicate(op) ?
          OP_code(op) : CGTARG_Predicated_Store (op);

        Build_OP (st_top,
                  OP_opnd(op, OP_find_opnd_use(op, OU_offset)),
                  OP_opnd(op, OP_find_opnd_use(op, OU_base)),
                  btn,
                  OP_opnd(op, OP_find_opnd_use(op, OU_storeval)),
                  &ops);
      }
      else {
        UINT8 opnd_idx = OP_find_opnd_use(op, OU_base);
        Expand_Cond_Store (btn, op, NULL, opnd_idx, &ops);
      }

      Copy_WN_For_Memory_OP(OPS_last(&ops), op);

      if (OPS_length(&ops)) {
        if (Trace_Select_Gen) {
          fprintf(Select_TFile, "<select> Insert conditional store in BB%d", BB_id(bb));
          Print_OPS (&ops); 
          fprintf (Select_TFile, "\n");
        }

        BB_Replace_Op (op, &ops);        

      }
    }
    i_iter++;
  }
}

static BOOL
Negate_Cmp_BB (OP *br)
{
  TN *btn = OP_opnd(br, 0);

  if (TN_is_global_reg(btn)) {
    OPS ops = OPS_EMPTY;

    Exp_Pred_Complement(Dup_TN(btn), NULL, btn, &ops);
    BB_Insert_Ops_Before(OP_bb(br), br, &ops);
    btn = OP_result (OPS_last(&ops), 0);  
    Set_OP_opnd(br, 0, btn);    
    return TRUE;
  }
  
  OP *cmp_op = TN_ssa_def(btn);
  TOP new_cmp = CGTARG_Invert(OP_code(cmp_op));
  if (new_cmp == TOP_UNDEFINED)
    return FALSE;

  OP_Change_Opcode(cmp_op, new_cmp);
  return TRUE;
}

static BOOL
Negate_Branch_BB (OP *br)
{
  TOP new_br = CGTARG_Invert(OP_code(br));
  if (new_br == TOP_UNDEFINED)
    return FALSE;

  OP_Change_Opcode(br, new_br);
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
    if (Trace_Select_Gen) {
      fprintf (Select_TFile, "\nNormalize Jumps OK\n");
    }
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

      if (Trace_Select_Gen) {
        fprintf (Select_TFile, "\nNormalize Jumps OK (noinvert)\n");
      }
      *cmp_invert = FALSE;
      return TRUE;
    }

    if (Trace_Select_Gen) {
      fprintf (Select_TFile, "\nNormalize Jumps OK (invert)\n");
    }

    *cmp_invert = TRUE;
    return TRUE;
  }

  if (Trace_Select_Gen) {
    fprintf (Select_TFile, "\nNormalize Jumps failed\n");
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

  if (Trace_Select_Gen) {
    fprintf (Select_TFile, "\nIs_Double_Logif %d \n", BB_id(bb));
  }

  // Find fall_thru and taken BBs.
  BB_Fall_Thru_and_Target_Succs(bb, &fall_thru, &target);

  if (BB_kind(target) == BBKIND_LOGIF) {
    if (BB_preds_len(target) == 1
        && Can_Speculate_BB(target)
        && Prep_And_Normalize_Jumps(bb, target, fall_thru, target,
                                    &sec_fall_thru, &sec_target, &cmp_invert)
        && sec_fall_thru == fall_thru) {

      if (! Check_Profitable_Logif(bb, target)) {
        BB_MAP_Set(if_bb_map, target, NULL);
        return NULL;
      }

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
        && Can_Speculate_BB(fall_thru)
        && Prep_And_Normalize_Jumps(bb, fall_thru, fall_thru, target,
                                    &sec_fall_thru, &sec_target, &cmp_invert)
        && sec_target == target) {

      if (! Check_Profitable_Logif(bb, fall_thru)) {
        BB_MAP_Set(if_bb_map, fall_thru, NULL);
        return NULL;
      }

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

  if (Trace_Select_Gen) {
    fprintf (Select_TFile, "\nStart gen logical from BB%d \n", BB_id(bb1));
  }

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

  // Compute probability for the new bb1->else edge
  // prob(bb1->else) = prob(bb1->bb2) * prob(bb2->else)
  // Note that for the other edge, we have:
  // prob(bb1->joint) = 1 - prob(bb1->else)
  float prob1 = 0.0, prob2 = 0.0;
  BBLIST *bblist;
  FOR_ALL_BB_SUCCS(bb1, bblist) {
    BB *succ = BBLIST_item(bblist);
    if (succ == bb2) {
      prob1 = BBLIST_prob(bblist);
      break;
    }
  }
  FOR_ALL_BB_SUCCS(bb2, bblist) {
    BB *succ = BBLIST_item(bblist);
    if (succ == else_block) {
      prob2 = BBLIST_prob(bblist);
      break;
    }
  }
  prob1 *= prob2;

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
  OP *cmp_op1 = TN_ssa_def(branch_tn1);
  OP *cmp_op2 = TN_ssa_def(branch_tn2);

  BOOL false_br = V_false_br(variant)?TRUE:FALSE;
  TN *label_tn = OP_opnd(br1_op, OP_find_opnd_use(br1_op, OU_target));

  // instructions from bb2 will be promoted. check mem predicates are set.
  BB_SET *t_set = BB_SET_Create_Empty(2, &MEM_Select_pool);
  BB_SET *ft_set = BB_SET_Create_Empty(2, &MEM_Select_pool);
  t_set = BB_SET_Union1D(t_set, bb2, &MEM_Select_pool);
  ft_set = BB_SET_Union1D(ft_set, else_block, &MEM_Select_pool);

  OPS ops = OPS_EMPTY;

  // now associate with each BB the predicate tn
  Associate_Mem_Predicates(branch_tn1,
                           (!false_br && bb2 == bb1_fall_thru) ||
                           (false_br && bb2 == bb1_target),
                           t_set, ft_set, &ops);

  // add computation of !test
  BB_Insert_Ops_Before(bb1, br1_op, &ops);

  OPS_Init(&ops);

  // make joint_block a fall_thru of bb1.
  BOOL invert_branch = FALSE;
  if (BB_Fall_Thru_Successor (bb1) == bb2 &&
      BB_Fall_Thru_Successor (bb2) == joint_block)
    invert_branch = TRUE;

  BB_Remove_Branch(bb1);
  BB_Remove_Branch(bb2);
  Promote_BB(bb2, bb1);

  // Get the result of the comparaison for the logical operation.
  branch_tn1 = Expand_CMP_Reg (branch_tn1, cmp_op1, &ops);
  branch_tn2 = Expand_CMP_Reg (branch_tn2, cmp_op2, &ops);

  // we need a branch_tn that will be the result of the logical op
  TN *new_branch_tn = Build_TN_Like (OP_opnd(br1_op, 0));

  // !a op !a -> !(a !op b)
  AndNeeded = AndNeeded ^ false_br;

  // insert new logical op.
  if (AndNeeded)
    Expand_Logical_And (new_branch_tn, branch_tn1, branch_tn2, variant, &ops);
  else
    Expand_Logical_Or (new_branch_tn, branch_tn1, branch_tn2, variant, &ops);

  // Stats
  logif_count++;

  // Create the branch.
  variant = V_BR_P_TRUE;
  if (false_br) {
    Set_V_false_br(variant);
  } else {
    Set_V_true_br(variant);
  }
  Expand_Branch (label_tn, new_branch_tn, NULL, variant, &ops);

  BB_Append_Ops (bb1, &ops);

  // Commit dismissible loads and stores
  BB_Fix_Spec_Loads (bb1);
  BB_Fix_Spec_Stores (bb1);

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
    prob1 = 1.0 - prob1;
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
    
  // replace phis in joint_block with a new reduced one.
  BB_Recomp_Phis(joint_block, bb1, branch_tn1, bb2, branch_tn2, 
                 (!false_br && bb2 == bb1_fall_thru) ||
                 (false_br && bb2 == bb1_target));

  OP *phi;
  FOR_ALL_BB_PHI_OPs(else_block, phi) {
    TN *result[1];
    UINT8 nopnds = OP_opnds(phi);
    TN *opnd[nopnds];
    UINT8 j=0;

    std::list<BB*> *bbs = new std::list<BB*>;

    for (UINT8 i = 0; i < nopnds; i++) {
      BB *pred = Get_PHI_Predecessor (phi, i);
      opnd[j++] = OP_opnd(phi, i);
      bbs->push_front(pred == bb2 ? bb1 : pred);
    }

    result[0] = OP_result(phi, 0);

    BB_Create_Phi (nopnds, result, opnd, bbs, phi);

  }

  BB_Update_Phis(else_block);

  if (Trace_Select_Gen) {
    fprintf (Select_TFile, "\nEnd gen logical from BB%d \n", BB_id(bb1));
    Print_All_BBs();
  }
}

/* ================================================================
 *
 *   If flattening
 *
 * ================================================================
 */
static void
Select_Fold (BB *head, BB_SET *t_set, BB_SET *ft_set, BB *tail)
{
  OP *phi;
  BB *target_bb, *fall_thru_bb;

  BB_Fall_Thru_and_Target_Succs(head, &fall_thru_bb, &target_bb);

  if (Trace_Select_Gen) {
    fprintf (TFile, "\nStart Select_Fold from BB%d\n", BB_id(head));

    fprintf (TFile, "\n fall_thrus are\n");
    BB_SET_Print (ft_set, Select_TFile);
    
    fprintf (TFile, "\n targets are\n");
    BB_SET_Print (t_set, Select_TFile);

    fprintf (TFile, "\n tail is\n");
    Print_BB_Header (tail, TRUE, TRUE);
  }
   
  // keep a list of newly created conditional move / compare
  // and phis to remove
  OPS cmov_ops = OPS_EMPTY;
  OPS ops = OPS_EMPTY;
  op_list old_phis;

  TN *cond_tn;
  TN *tn2;

  BOOL edge_needed = (target_bb != tail && fall_thru_bb != tail);

   // find last bb before tail.
   BBLIST* pedge;
   BB *last_target = target_bb;
   BB *last_fall_thru = fall_thru_bb;

   FOR_ALL_BB_PREDS (tail, pedge) {
     BB *pred = BBLIST_item(pedge);

     if (target_bb != tail)
       if (BB_SET_MemberP(t_set, pred))
         last_target = pred;

     if (fall_thru_bb != tail)
       if (BB_SET_MemberP(ft_set, pred))
         last_fall_thru = pred;
   }

  if (fall_thru_bb != tail) {
    BB *pred = last_fall_thru;

    while (pred != head) {
      BB *other_pred = head;

      FOR_ALL_BB_PREDS (pred, pedge) {
        BB *pired = BBLIST_item(pedge);

        if (BB_SET_MemberP(ft_set, pired))
          other_pred = pired;
      }

      BB *phi_pred = other_pred;

      Force_End_Tns (pred, tail);

      if (BB_preds_len (pred) > 1) {
        other_pred = Gen_And_Insert_BB_After (phi_pred);
        if (!BB_Retarget_Branch(phi_pred, pred, other_pred))
          Change_Succ(phi_pred, pred, other_pred);
        Add_Goto (other_pred, tail);
        if (pred == fall_thru_bb) {
          fall_thru_bb = other_pred;
        }

        Copy_BB_For_Duplication(pred, phi_pred, other_pred, tail, &ft_set);
      }

      pred = phi_pred;
    }
  }

  if (target_bb != tail) {
    BB *pred = last_target;

    while (pred != head) {
      BB *other_pred = head;

      FOR_ALL_BB_PREDS (pred, pedge) {
        BB *pired = BBLIST_item(pedge);

        if (BB_SET_MemberP(t_set, pired))
          other_pred = pired;
      }

      BB *phi_pred = other_pred;

      Force_End_Tns (pred, tail);

      if (BB_preds_len (pred) > 1) {
        other_pred = Gen_And_Insert_BB_After (phi_pred);
        if (!BB_Retarget_Branch(phi_pred, pred, other_pred))
          Change_Succ(phi_pred, pred, other_pred);
        Add_Goto (other_pred, tail);
        if (pred == target_bb) {
          target_bb = other_pred;
        }

        Copy_BB_For_Duplication(pred, phi_pred, other_pred, tail, &t_set);
      }

      pred = phi_pred;
    }
  }

  OP *br_op = BB_branch_op(head);

  VARIANT variant = CGTARG_Analyze_Branch(br_op, &cond_tn, &tn2);
  BOOL false_br = V_false_br(variant)?TRUE:FALSE;

  // now associate with each BB the predicate tn
  Associate_Mem_Predicates(cond_tn, false_br, t_set, ft_set, &ops);
  // add computation of !test
  BB_Insert_Ops_Before(head, br_op, &ops);

  // can remove the branch op
  BB_Remove_Op(head, br_op);

  FOR_ALL_BB_PHI_OPs(tail, phi) {
    UINT8 npreds = OP_opnds(phi);

    BB *true_bb = target_bb == tail ? head : target_bb;
    BB *false_bb = fall_thru_bb == tail ? head : fall_thru_bb;

    if (false_br) {
      BB *temp_bb = false_bb;
      false_bb = true_bb;
      true_bb = temp_bb;
    }

    TN *select_tn = OP_result (phi, 0);

    if (npreds > 2) {
      TN *result[1];
      UINT8 j = 0;
      UINT8 nopnds = npreds - 1;
      TN *opnd[nopnds];

      std::list<BB*> *bbs = new std::list<BB*>;

      select_tn = Dup_TN(select_tn);
      Set_TN_is_global_reg (select_tn);
      
      BB* tpred = Get_BB_Pos_in_PHI (phi, true_bb);
      BB* fpred = Get_BB_Pos_in_PHI (phi, false_bb);

      // new args goes last if we know that a new edge will be inserted.
      // else, replace old phi tn with newly create select tn.
      if (edge_needed) {
        for (UINT8 i = 0; i < npreds; i++) {
          BB *pred = Get_PHI_Predecessor (phi, i);
          if (pred != tpred && pred != fpred) {
            bbs->push_front(pred);
            opnd[j++] = OP_opnd(phi, i);
          }
        }
        bbs->push_front(head);
        opnd[j] = select_tn;
      }
      else {
        for (UINT8 i = 0; i < npreds; i++) {
          BB *pred = Get_PHI_Predecessor (phi, i);
          if (pred != fpred && pred != tpred) {
            bbs->push_front(pred);
            opnd[j++] = OP_opnd(phi, i);
          }
          else if (pred == tpred) {
            bbs->push_front(head); 
            opnd[j++] = select_tn;
          }
        }
      }
      
      result[0] = OP_result(phi, 0);

      BB_Create_Phi (nopnds, result, opnd, bbs, phi);
    }
    else {
      // Mark phi to be deleted.
      old_phis.push_front(phi);
    }

    TN *true_tn   = OP_opnd(phi, Get_TN_Pos_in_PHI (phi, true_bb));
    TN *false_tn  = OP_opnd(phi, Get_TN_Pos_in_PHI (phi, false_bb));
    DevAssert(true_tn && false_tn, ("Select: undef TN"));

    Create_PSI_or_Select (select_tn, cond_tn, true_tn, false_tn, &cmov_ops);
  
    select_count++;
  }
  
  // Promote the instructions from the sides bblocks.
  if (fall_thru_bb != tail) {
    Promote_BB_Chain (head, fall_thru_bb, tail);
  }
  if (target_bb != tail) {
    Promote_BB_Chain (head, target_bb, tail);
  }

   if (Trace_Select_Gen) {
     fprintf(Select_TFile, "<select> Insert selects in BB%d", BB_id(head));
     Print_OPS (&cmov_ops); 
     fprintf (Select_TFile, "\n");
   }

   // Cleanup phis that are going to be replaced ...
   BB_Remove_Ops(tail, old_phis);
   // ... by the selects
   BB_Append_Ops(head, &cmov_ops);

  // Commit dismissible loads and stores
   BB_Fix_Spec_Loads (head);
   BB_Fix_Spec_Stores (head);

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
   }
   
   // Update edge probability to 1.0
   BBLIST_prob(BB_Find_Succ(head, tail)) = 1.0;

   // Update SSA with new phis.
   BB_Update_Phis(tail);

   if (Trace_Select_Gen) {
     fprintf (TFile, "\nEnd Select_Fold from BB%d\n", BB_id(head));
     Print_All_BBs();
   }
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

  if (OPT_Space) {
      CG_select_freq = FALSE;
      CG_select_cycles = FALSE;
  }

  // higher select_factor means ifc more aggressive.
  select_factor = atof(CG_select_factor);
  branch_penalty = CGTARG_Branch_Taken_Penalty();

  if (select_factor == 0.0) return;

  Trace_Select_Init();

  Initialize_Memory();

  if (Trace_Select_Candidates) {
    Trace_IR(TP_SELECT, "Before Select Region Formation", NULL);
    Print_All_BBs();
  }

  Identify_Logifs_Candidates();

  // make sure dominator and liveness information are correct 
  // before starting optimizing
  Calculate_Dominators();
  GRA_LIVE_Recalc_Liveness(rid);

  if_bb_map = BB_MAP_Create();

  bool allow_dup = false;

  if (Trace_Select_daVinci) {
    printf ("start ifc for%s\n",  ST_name(Get_Current_PU_ST()));
  }

 restart:

  bool region_changed = false;

  for (i = 0; i < max_cand_id; i++) {
    BB *bb = cand_vec[i];
    BB *bbb;
    
    if (bb == NULL) continue;
      
    // tests for logical expression 
    if (bbb = Is_Double_Logif(bb)) {
      Initialize_Hammock_Memory();

      if (Trace_Select_daVinci) {
        printf ("Simplify Logif for BB%d\n", BB_id(bb));
        //        draw_flow_graph();
        draw_CFG();
      }

      Simplify_Logifs(bb, bbb);

#ifdef Is_True_On
      //      Sanity_Check();
#endif
      region_changed = true;

      Finalize_Hammock_Memory();
      cand_vec[BB_MAP32_Get(postord_map, bbb)-1] = NULL;
    }

    clear_spec_lists();

  }

  for (i = 0; i < max_cand_id; i++) {
    BB *bb = cand_vec[i];
    BB *bbb;

    if (bb == NULL) continue;
    
    BB_SET *t_set = BB_SET_Create_Empty(PU_BB_Count+2, &MEM_Select_pool);
    BB_SET *ft_set = BB_SET_Create_Empty(PU_BB_Count+2, &MEM_Select_pool);

    // test of conditional expression
    if (Is_Hammock (bb, t_set, ft_set, &bbb, allow_dup)) {
      Initialize_Hammock_Memory();

      if (Trace_Select_daVinci) {
        printf ("Select_Fold for BB%d\n", BB_id(bb));
        //        draw_flow_graph();
        draw_CFG();
      }

      Select_Fold (bb, t_set, ft_set, bbb);

#ifdef Is_True_On
      //      Sanity_Check();
#endif
      region_changed = true;

      Finalize_Hammock_Memory();
      
      // if bb is still a logif, that means that there was a merge.
      // need to update logif map
      if (BB_kind (bb) == BBKIND_LOGIF) {
        cand_vec[BB_MAP32_Get(postord_map, bb)-1] = bb;
        cand_vec[BB_MAP32_Get(postord_map, bbb)-1] = NULL;
      }
      else {
        cand_vec[BB_MAP32_Get(postord_map, bb)-1] = NULL;
      }
    }

      clear_spec_lists();
  }

  if (region_changed) {
    allow_dup = false;
    goto restart;
  }
  else {
    if (!allow_dup && CG_select_allow_dup) {
      allow_dup = true;
      goto restart;
    }
  }

  BB_MAP_Delete(if_bb_map);

  if (Trace_Select_Stats) {
    CG_SELECT_Statistics();
  }

#ifdef Is_True_On
  //  SSA_Verify(rid, 0);
#endif

  if (Trace_Select_daVinci) {
    printf ("done\n");
        draw_CFG();
        //    draw_flow_graph();

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
