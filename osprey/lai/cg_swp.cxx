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



#define USE_STANDARD_TYPES
// [HK]
#if __GNUC__ >= 3
#include <vector>
// using std::vector;
#else
#include <vector.h>
#endif // __GNUC__ >= 3
#include "defs.h"
#include "errors.h"
#include "tracing.h"
#include "timing.h"
#include "glob.h"    // for Cur_PU_Name
#include "op.h"
#include "bb.h"
#include "cg.h"
#include "cgexp.h"   // for Exp_COPY 
#include "cg_swp.h"
#include "cg_swp_bundle.h"
#include "cg_swp_options.h"
#include "cgprep.h"
#include "cg_swp_target.h"
#include "cg_dep_graph.h"
#include "cg_loop.h"
#include "cg_loop_mii.h"
#include "cgtarget.h"
#include "findloops.h"
#include "ti_res_count.h"
#include "tag.h"

static INT loop_index;

// Contains SWP options
//
SWP_OPTIONS SWP_Options;


void SWP_OPTIONS::PU_Configure()
{
  if (Opt_Level == 0) {
    Sched_Direction = 2;
    Heuristics = 2;
  }

  if (!CGTARG_Can_Predicate())
    Enable_While_Loop = FALSE;

  if (!Max_Unroll_Times_Set)
    Max_Unroll_Times = (CG_opt_level > 2) ? 8 : 4;  

    Min_Unroll_Times = max(1, Min_Unroll_Times);
    Max_Unroll_Times = max(1, Max_Unroll_Times);

  if (Min_Unroll_Times_Set)
    Max_Unroll_Times = Max(Max_Unroll_Times, Min_Unroll_Times);
  
  if (Max_Unroll_Times_Set)
    Min_Unroll_Times = Min(Min_Unroll_Times, Max_Unroll_Times);

  if (!Implicit_Prefetch_Set) {
    // Not all processors implement implicit prefetch -- disable
    // by default on those processors
    if (Is_Target_Itanium()) Implicit_Prefetch = FALSE;
  }
}


void SWP_OP::Print(FILE *fp) const {
  if (op) 
    fprintf(fp, "[%d](%s) %s scale=%g cycle=%d mod=%d slot=%d trials=%d dir=%s\n",
	    Index(), TOP_Name(OP_code(op)),
	    placed?"placed":"not-placed", scale, cycle, modulo_cycle, 
	    slot, trials,
	    (direction==SWP_TOP_DOWN) ? "top_down" : 
	    ((direction==SWP_BOTTOM_UP) ? "bottom_up" : "unknown"));
  else
    fprintf(fp, "not an SWP op");
}

void SWP_OP_vector::Verify() const {
  for (INT i = 0; i < size(); i++) {
    if (v[i].op)
      FmtAssert(v[i].Index() == i, ("SWP_OP_vector::Verify: v[i].Index() != i"));
  }
}

void SWP_OP_vector::Print(FILE *fp) const {
  for (INT i = 0; i < size(); i++) {
    if (v[i].op) 
      v[i].Print(fp);
  }
  fprintf(TFile, "Invariants: ");
  TN_SET_Print(tn_invariants, fp);
  fprintf(TFile, "\n");
  fprintf(TFile, "Non-rotating: ");
  TN_SET_Print(tn_non_rotating, fp);
  fprintf(TFile, "\n");
}


INT *swp_map_tbl;
INT  swp_map_tbl_max;

SWP_OP_vector::SWP_OP_vector(BB *body, BOOL doloop, MEM_POOL *pool)
{
  OP *op;
  INT max_idx = 0;
  FOR_ALL_BB_OPs(body, op) {
    max_idx = max(max_idx, OP_map_idx(op));
  }
  swp_map_tbl_max = max_idx + 1;
  swp_map_tbl = TYPE_MEM_POOL_ALLOC_N(INT, pool, swp_map_tbl_max);
  INT count = 0;
  FOR_ALL_BB_OPs(body, op) {
    swp_map_tbl[OP_map_idx(op)] = count++;
  }
  const bool trace = Get_Trace(TP_SWPIPE, 2);
  if (trace) {
    for (INT i = 0; i < swp_map_tbl_max; i++)
      fprintf(TFile, "swp_map[%d] = %d\n", i, swp_map_tbl[i]);
  }
  
  INT size = count;
  start = size++;
  stop = size++;
  previous_trials = 0;

#if SWP_USE_STL
  v.insert(v.begin(), size, SWP_OP());
#else
  {
    Is_True(size <= SWP_OPS_LIMIT, 
	    ("SWP_OP_vector: loop has too many (%d) ops.", size));
    v_size = size;
    for (INT i = 0; i < size; i++)
      v[i] = SWP_OP();
  }
#endif

  tn_non_rotating = TN_SET_Create_Empty(Last_TN + 1, pool);
  TN_SET *tn_defs = TN_SET_Create_Empty(Last_TN + 1, pool);
  TN_SET *tn_uses = TN_SET_Create_Empty(Last_TN + 1, pool);
  num_mops = 0;
  num_flops = 0;
  FOR_ALL_BB_OPs(body, op) {
    INT idx = SWP_index(op);
    v[idx].op = op;
    if (OP_memory(op))
      num_mops++;
    if (OP_flop(op))
      num_flops++;
    for (INT i = 0; i < OP_results(op); i++) {
      TN *tn = OP_result(op, i);
      if (TN_is_register(tn) && !TN_is_dedicated(tn))
	tn_defs = TN_SET_Union1D(tn_defs, tn, pool);
      if (TN_is_dedicated(tn))
	tn_non_rotating = TN_SET_Union1D(tn_non_rotating, tn, pool);
    }
    for (INT j = 0; j < OP_opnds(op); j++) {
      TN *tn = OP_opnd(op, j);
      if (TN_is_register(tn) && !TN_is_dedicated(tn))
	tn_uses = TN_SET_Union1D(tn_uses, tn, pool);
      if (TN_is_dedicated(tn))
	tn_non_rotating = TN_SET_Union1D(tn_non_rotating, tn, pool);
    }
#ifdef TARG_ST
    TN *bu_tn = OP_base_update_tn(op);
#else
    TN *bu_tn = Base_update_tn(op);
#endif
    if (bu_tn)
      tn_non_rotating = TN_SET_Union1D(tn_non_rotating, bu_tn, pool);
  }
  // Identify loop invariants!
  tn_invariants = TN_SET_Difference(tn_uses, tn_defs, pool);
  tn_non_rotating = TN_SET_UnionD(tn_non_rotating, tn_invariants, pool);
  OP *br_op = BB_branch_op(body);
  branch = SWP_index(br_op);
#ifdef TARG_ST
  /* (cbr) predicate operand # is not necessary constant */
  control_predicate_tn = OP_has_predicate(br_op) ? OP_opnd(br_op, OP_find_opnd_use(br_op, OU_predicate)) : NULL;
#else
  control_predicate_tn = OP_has_predicate(br_op) ? OP_opnd(br_op, OP_PREDICATE_OPND) : NULL;
#endif
  is_doloop = doloop;
  succeeded = false;
  loop_one_more_time = false;
}

template <class T1>
inline T1 linear_func(T1 x, double alpha, double beta)
{
  return (T1)((double)x * beta + alpha);
}

void SWP_Show_Statistics(const SWP_OP_vector& swp_op_vector, BB *body)
{
  INT nops = 0;
  INT trials = 0;
  for (INT i = 0; i < swp_op_vector.size(); i++) {
    if (swp_op_vector[i].op) {
      nops++;
      trials += swp_op_vector[i].trials;
    }
  }
  const char *banner = "<swps>";
  INT ii = swp_op_vector.ii;
  fprintf(TFile, "%s SWP for PU %s BB %d: %s\n", banner, 
	  Cur_PU_Name ? Cur_PU_Name : "noname", BB_id(body), 
	  (ii == 0) ? "failed" : 
	  ((ii == CG_LOOP_min_ii) ? "optimal" : "non-optimal"));
  fprintf(TFile, "%s  min II: %d\n", banner, swp_op_vector.min_ii);
  fprintf(TFile, "%s  ResMII: %d\n", banner, swp_op_vector.res_min_ii);
  fprintf(TFile, "%s  RecMII: %d\n", banner, swp_op_vector.rec_min_ii);
  fprintf(TFile, "%s  min SL: %d\n", banner, swp_op_vector.min_sl);   
  fprintf(TFile, "%s  found II: %d\n", banner, swp_op_vector.ii);
  fprintf(TFile, "%s  found SL: %d\n", banner, swp_op_vector.sl);
  fprintf(TFile, "%s  found SC: %d\n", banner, swp_op_vector.sc);
  fprintf(TFile, "%s  # ops: %d\n", banner, nops);
  fprintf(TFile, "%s  # trials: %d\n", banner, trials);
  fprintf(TFile, "%s  # total trials: %d\n", banner, trials + swp_op_vector.previous_trials);
  fprintf(TFile, "%s  prep time: %g\n", banner, swp_op_vector.prep_time);
  fprintf(TFile, "%s  sched time: %g\n", banner, swp_op_vector.sched_time);
  fprintf(TFile, "%s  reg alloc time: %g\n", banner, swp_op_vector.reg_alloc_time);
  fprintf(TFile, "%s  code gen time: %g\n", banner, swp_op_vector.code_gen_time);
}



/* ====================================================================
 * ====================================================================
 *                Arthur: CODE BROUGHT IN FROM swp_targ
 *                Looks target-independent to me ??
 * ====================================================================
 * ====================================================================
 */

// Construct a data structure to locate all defs and uses of a TN,
// using the OP_VECTOR::index.   The properties of the OP_VECTOR::index
// is that it's ordered.
// 
struct TN_DU {
  typedef OP_VECTOR::index_type index_type;
  std::vector<index_type> defs;
  std::vector<index_type> uses;

  bool TN_is_invariant() const {
    return defs.size() == 0;
  }

  // Returns true if the TN is not modified in the range [first,last)
  bool TN_unchanged(index_type first, index_type last) {
    for (int i = 0; i < defs.size(); i++) {
      index_type t = defs[i];
      if (first <= t && t < last)
	return false;
    }
    return true;
  }

  // Returns true if the TN can be assigned a non-rotating register
  bool TN_can_use_non_rotating_reg(TN *tn, OP_VECTOR& op_vec) {
    
    // d is set to the earliest definition, set to MAX_INT if there is no definitions
    index_type d = defs.size() > 0 ? defs[0] : INT32_MAX;

    // an omega 0 use is always OK
    // an omega 1 use is OK if the use is before the earliest definition
    // an omega >1 use is never OK

    for (int j = 0; j < uses.size(); j++) {
      index_type u = uses[j];
      OP *op = op_vec[u];

      for (int i = 0; i < OP_opnds(op); i++) {
	if (tn == OP_opnd(op, i)) {
	  int omega = OP_omega(op, i);
	  if (omega >= 1) {
	    if (omega > 1)
	      return false;
	    if (d < u)  // omega == 1
	      return false;
	  }
	}
      }
    }
    return true;
  }
};


// Construct a TN to TN_DU mapping.
//
struct TN_DU_MAP {

  typedef std::map<TN *, TN_DU>::iterator iterator;
  std::map<TN *, TN_DU> TN_DU_map;

  iterator begin() {
    return TN_DU_map.begin();
  }

  iterator end() {
    return TN_DU_map.end();
  }

  TN_DU& operator[](TN *tn) {
    return TN_DU_map[tn];
  }

  // Build a TN_DU data structure for each TN
  // referenced in the BB.   And also assign an OP-number
  // to each OP *.   The TN_DU represents all occurrences
  // of defs and uses of the TN using the OP-number.
  //
  TN_DU_MAP(OP_VECTOR& op_vec, bool trace) {

    for (INT op_num = 0; op_num < op_vec.size(); op_num++) {
      OP *op = op_vec[op_num];
      INT i;
      for (i = 0; i < OP_results(op); i++) {
	TN *tn = OP_result(op,i);
	if (TN_is_register(tn) && 
	    !TN_is_dedicated(tn) &&
	    !TN_is_const_reg(tn)) {
	  if (TN_DU_map.find(tn) == TN_DU_map.end()) 
	    TN_DU_map[tn] = TN_DU();
	  TN_DU_map[tn].defs.push_back(op_num);
	}
      }
      for (i = 0; i < OP_opnds(op); i++) {
	TN *tn = OP_opnd(op,i);
	if (TN_is_register(tn) && 
	    !TN_is_dedicated(tn) &&
	    !TN_is_const_reg(tn)) {
	  if (TN_DU_map.find(tn) == TN_DU_map.end()) 
	    TN_DU_map[tn] = TN_DU();
	  TN_DU_map[tn].uses.push_back(op_num);
	}
      }
    }

    // Trace the TN_DU data structure
    if (trace) {
      for (iterator it = TN_DU_map.begin(); it != TN_DU_map.end(); it++) {
	TN *tn = (*it).first;
	TN_DU &lrs = (*it).second;
	fprintf(TFile, "Remove_Non_Definite_Dependence: TN_DU of TN%d: defs={", TN_number(tn));
	{
	  for (int i = 0; i < lrs.defs.size(); i++) {
	    fprintf(TFile, "%d", lrs.defs[i]);
	    if (i != lrs.defs.size()-1) fputc(',', TFile);
	  }
	}
	fprintf(TFile, "}, uses={");
	{
	  for (int i = 0; i < lrs.uses.size(); i++) {
	    fprintf(TFile, "%d", lrs.uses[i]);
	    if (i != lrs.uses.size()-1) fputc(',', TFile);
	  }
	}
	fprintf(TFile, "}\n");
      }
    }
  }
};

/* ====================================================================
 *   Remove_Non_Definite_Dependence
 *
 *   Remove cross-iteration non-defnite TN dependence by 
 *   introducing copies.
 * ====================================================================
 */
BOOL 
Remove_Non_Definite_Dependence (
  CG_LOOP &cl, 
  bool cg_loop_init, 
  bool trace
)
{
  BB *body = cl.Loop_header();

  OP_VECTOR op_vec(body);
  TN_DU_MAP TN_DU_map(op_vec, trace);

  // Based on the TN_DU data structure, determine if
  // a TN has non-definite dependence.  If yes, generate a copy
  //
  int copy_inserted = 0;
  OPS ops = OPS_EMPTY;

  for (TN_DU_MAP::iterator it = TN_DU_map.begin(); it != TN_DU_map.end(); it++) {

    TN *tn = (*it).first;
    TN_DU &lrs = (*it).second;

    if (TN_is_dedicated(tn)) continue;

    bool need_copy = true;

    // there is no defs, then all uses are invariants
    if (lrs.TN_is_invariant())
      need_copy = false;

    // if the first def is uncond def, all uses are definite
    if (need_copy) 
      if (lrs.defs.size() > 0) {
	OP *op = op_vec[lrs.defs[0]];
	if (!OP_cond_def(op))
	  need_copy = false;
      }

    // the case where we have two complementary conditional defs
    // both coming before the first use
    if (need_copy) {
      if (lrs.defs.size() >= 2  && lrs.uses.size() > 0) {
        if (lrs.defs[0] < lrs.uses[0] && lrs.defs[1] < lrs.uses[0]) {
          OP *def1 = op_vec[lrs.defs[0]];
          OP *def2 = op_vec[lrs.defs[1]];
          if (OP_cond_def(def1) && OP_cond_def(def2)) {
#ifdef TARG_ST
  /* (cbr) predicate operand # is not necessary constant */
            TN *pred1 = OP_opnd(def1, OP_find_opnd_use(def1, OU_predicate));
            TN *pred2 = OP_opnd(def2, OP_find_opnd_use(def2, OU_predicate));
#else
            TN *pred1 = OP_opnd(def1, OP_PREDICATE_OPND);
            TN *pred2 = OP_opnd(def2, OP_PREDICATE_OPND);
#endif
            if (pred1 != pred2) {
              DEF_KIND kind;
              OP *pred1_def = TN_Reaching_Value_At_Op(pred1,def1,&kind,TRUE);
              OP *pred2_def = TN_Reaching_Value_At_Op(pred2,def2,&kind,TRUE);
              if (kind == VAL_KNOWN && pred1_def && pred1_def == pred2_def) {
                need_copy = false;
              }
            }
          }
        }
      }
    }

    // if the predicate of every use is covered by some defs,
    // the uses are definite
    if (need_copy) {
      bool all_uses_covered = true;
      for (int i = 0; i < lrs.uses.size(); i++) {
	bool found_covering_def = false;
	OP *op = op_vec[lrs.uses[i]];
	if (OP_cond_def(op)) {
#ifdef TARG_ST
  /* (cbr) predicate operand # is not necessary constant */
	  TN *pred_tn = OP_opnd(op, OP_find_opnd_use(op, OU_predicate));
#else
	  TN *pred_tn = OP_opnd(op, OP_PREDICATE_OPND);
#endif
	  for (int j = lrs.defs.size() - 1; j >= 0; j--) {
	    if (lrs.defs[j] < lrs.uses[i]) {
	      OP *defop = op_vec[lrs.defs[j]];
#ifdef TARG_ST
  /* (cbr) predicate operand # is not necessary constant */
	      TN *def_pred_tn = OP_opnd(defop, OP_find_opnd_use(defop, OU_predicate));
#else
	      TN *def_pred_tn = OP_opnd(defop, OP_PREDICATE_OPND);
#endif
	      if (def_pred_tn == pred_tn &&
		  TN_DU_map[pred_tn].TN_unchanged(lrs.defs[j], lrs.defs[i])) {
		found_covering_def = true;
		break;
	      }
	    }
	  }
	}
	if (!found_covering_def) {
	  all_uses_covered = false;
	  break;
	}
      }
      if (all_uses_covered) {
	BB *epilog = CG_LOOP_epilog;
	CG_LOOP_BACKPATCH *bp;
	for (bp = CG_LOOP_Backpatch_First(epilog, NULL); bp; bp = CG_LOOP_Backpatch_Next(bp)) {
	  TN *body_tn = CG_LOOP_BACKPATCH_body_tn(bp);
	  if (body_tn == tn) {
	    all_uses_covered = false;
	    break;
	  }
	}
      }
      if (all_uses_covered) 
	need_copy = false;
    }

    if (need_copy) {
      // create a definite copy
      Exp_COPY(tn, tn, &ops); 
      copy_inserted++;
      if (trace)
	fprintf(TFile, "Remove_Non_Definite_Dependence: TN%d\n", TN_number(tn));
    }
  }

  OP *old_first_op = BB_first_op(body);
  BB_Prepend_Ops(body, &ops);

  if (cg_loop_init) {
    for (OP *op = BB_first_op(body); op != old_first_op; op = OP_next(op)) {
      CG_LOOP_Init_Op(op);
      for (INT i = 0; i < OP_opnds(op); i++) {
#ifdef TARG_ST
  /* (cbr) predicate operand # is not necessary constant */
	if (i != OP_find_opnd_use(op, OU_predicate) && TN_is_register(OP_opnd(op,i))
#else
	if (i != OP_PREDICATE_OPND && TN_is_register(OP_opnd(op,i))
#endif
	    && ! TN_is_dedicated(OP_opnd(op,i))) 
	  Set_OP_omega(op, i, 1);
      }
    }
  }

  if (trace) 
    fprintf(TFile, "Remove_Non_Definite_Dependence: %d copy inserted\n", 
	    copy_inserted);

  return TRUE;
}

/* ====================================================================
 *   Gen_SWP_Branch
 *
 *   TODO: this is not entirely target-independent but will be made
 *         so through adding target-specific calls.
 * ====================================================================
 */
void 
Gen_SWP_Branch (
  CG_LOOP &cl, 
  bool is_doloop
)
{
  if (PROC_has_swp_branches()) {
    BB *body = cl.Loop_header();
    OP *br_op = BB_branch_op(body);
    TN *label_tn = OP_opnd(br_op, Branch_Target_Operand(br_op));
    OPS ops = OPS_EMPTY;
    OP *op;

    FmtAssert(FALSE,("not implemented"));
  
    op = OPS_last(&ops);
    Set_OP_Tag (op, Gen_Tag());

    BB_Remove_Op(body, br_op);
    BB_Append_Ops(body, &ops);
  }

  return;
}

/* ====================================================================
 *   Undo_SWP_Branch
 *
 *   TODO: this is not entirely target-independent but will be made
 *         so through adding target-specific calls.
 * ====================================================================
 */
void Undo_SWP_Branch(CG_LOOP &cl, bool is_doloop)
{
  BB *body = cl.Loop_header();
  OP *br_op = BB_branch_op(body);
  TN *label_tn = OP_opnd(br_op, Branch_Target_Operand(br_op));
  OPS ops = OPS_EMPTY;
  
  if (is_doloop) {

    // Generate br.ctop for doloop
    FmtAssert(FALSE,("not implemented"));
#if 0
    Build_OP (TOP_br_cloop, 
	      LC_TN,
	      Gen_Enum_TN(ECV_bwh_dptk),
	      Gen_Enum_TN(ECV_ph_few),
	      Gen_Enum_TN(ECV_dh),
	      label_tn, 
	      LC_TN, &ops);

    Is_True(OP_code(br_op) == TOP_br_ctop,
	    ("Undo_SWP_Branch: SWP doloop must use TOP_br_ctop."));
    LC_Used_In_PU = TRUE;
#endif
  } else {

#ifdef TARG_ST
  /* (cbr) predicate operand # is not necessary constant */
    TN *predicate = OP_opnd(br_op, OP_find_opnd_use(br_op, OU_predicate));
#else
    TN *predicate = OP_opnd(br_op, OP_PREDICATE_OPND);
#endif

    FmtAssert(FALSE,("not implemented"));
#if 0
    Build_OP (TOP_br_cond, 
	      predicate,
	      Gen_Enum_TN(ECV_bwh_dptk),
	      Gen_Enum_TN(ECV_ph_few),
	      Gen_Enum_TN(ECV_dh),
	      label_tn, &ops);

    Is_True(OP_code(br_op) == TOP_br_wtop,
	    ("Undo_SWP_Branch: SWP whileloop must use TOP_br_wtop."));
#endif

  }
  
  BB_Remove_Op(body, br_op);
  BB_Append_Ops(body, &ops);

  CG_LOOP_Init_Op(BB_branch_op(body));
}

/* ====================================================================
 * ====================================================================
 *                END OF CODE BROUGHT IN FROM swp_targ
 * ====================================================================
 * ====================================================================
 */


BOOL SWP_Failure(BB *body, SWP_RETURN_CODE code)
{
  // Generate SWP ROTATING KERNEL Annotation
  ROTATING_KERNEL_INFO *info = TYPE_PU_ALLOC(ROTATING_KERNEL_INFO);
  BZERO(info, sizeof(ROTATING_KERNEL_INFO));
  ROTATING_KERNEL_INFO_succeeded(info) = FALSE;
  ROTATING_KERNEL_INFO_failure_code(info) = code;
  BB_Add_Annotation(body, ANNOT_ROTATING_KERNEL, (void *)info);
  Reset_BB_rotating_kernel(body);
  return FALSE;
}


// Identify potential hardware/simulator problem
// 
SWP_RETURN_CODE Detect_SWP_Constraints(CG_LOOP &cl, bool trace)
{
  if (SWP_Options.Prep_Only)
    return SWP_PREP_ONLY;

  LOOP_DESCR *loop = cl.Loop();
  BB *body = LOOP_DESCR_loophead(loop);
  OP *op;
  INT op_count = 0;
  INT total_op_count = 0;
  FOR_ALL_BB_OPs(body, op) {
    if (!OP_br(op) && !OP_dummy(op))
      op_count++;
    total_op_count++;

    if (OP_code(op) == TOP_asm) {
      if (trace) 
	fprintf(TFile, "SWP: skip optimization due to TOP_asm.\n");
      return SWP_ASM;
    }

    if (SWP_Options.Enable_Workaround) {
#ifdef TARG_IA64
      if (OP_code(op) == TOP_setf_sig) {
	DevWarn("SWP: skip optimization due to simulation of frcpa");
	if (trace) 
	  fprintf(TFile, "SWP: skip optimization due to simulation of frcpa.\n");
	return SWP_WORKAROUND;
      }
#endif
    }
    
    TN *found_ded_tn = NULL;
    for (INT i = 0; i < OP_results(op); i++) {
      TN *tn = OP_result(op, i);
      if (TN_is_dedicated(tn) &&
	  !TN_is_const_reg(tn) &&
	  REGISTER_Is_Rotating(TN_register_class(tn), TN_register(tn))) {
	found_ded_tn = tn;
	break;
      }
    }
    for (INT j = 0; j < OP_opnds(op); j++) {
      TN *tn = OP_opnd(op, j);
      if (TN_is_dedicated(tn) && 
	  !TN_is_const_reg(tn) && 
	  REGISTER_Is_Rotating(TN_register_class(tn), TN_register(tn))) {
	found_ded_tn = tn;
	break;
      }
    }
    if (found_ded_tn) {
      if (trace) 
	fprintf(TFile, "SWP: skip optimization due to rotating dedicated TN%d.\n",
		TN_number(found_ded_tn));
      return SWP_DEDICATED_ROT_REG;
    }
  }

  if (op_count == 0) 
    return SWP_LOOP_EMPTY;     // don't bother to swp empty loops

  if (total_op_count + SWP_OPS_OVERHEAD > SWP_OPS_LIMIT)
    return SWP_LOOP_LIMIT;
    
  return SWP_OK;
}


// prune loop-carried REGOUT dependences on rotating registers
static void
Prune_Regout_Deps(BB *body, TN_SET *non_rotating)
{
  std::vector<ARC*> arcs_to_delete;
  OP *op;
  FOR_ALL_BB_OPs(body, op) {
    if (_CG_DEP_op_info(op)) {
      for (ARC_LIST *arcs = OP_succs(op); arcs; arcs = ARC_LIST_rest(arcs)) {
        ARC *arc = ARC_LIST_first(arcs);
        // look for loop-carried REGOUT dependences
        if (ARC_kind(arc) == CG_DEP_REGOUT && ARC_omega(arc) > 0) {
          // check that none of the OP results is in the non-rotating set
          bool redundant = true;
          for (INT i = 0; i < OP_results(op); i++) {
            TN *tn = OP_result(op,i);
            if (!TN_is_register(tn) ||
                TN_is_dedicated(tn) ||
                TN_SET_MemberP(non_rotating, tn)) {
              redundant = false;
              break;
            }
          }
          if (redundant) {
            arcs_to_delete.push_back(arc);
          }
        }
      }
    }
  }
  for (size_t i = 0; i < arcs_to_delete.size(); i++) {
    CG_DEP_Detach_Arc(arcs_to_delete[i]);
  }
}


static BOOL Is_Loop_Skipped(void)
/* -----------------------------------------------------------------------
 * Return a boolean that indicates if we should skip pipelining
 * the specified loop.
 * -----------------------------------------------------------------------
 */
{
  const BOOL skip_it = (   loop_index < CG_local_skip_before
			|| loop_index > CG_local_skip_after
			|| loop_index == CG_local_skip_equal);

  if (CG_skip_local_swp) {
    DevWarn("%s swp for loop: index=%d",
	    skip_it ? "Skipping" : "Attempting",
	    loop_index);
  }

  ++loop_index;

  return skip_it;
}


BOOL Perform_SWP(CG_LOOP& cl, SWP_FIXUP_VECTOR& fixup, bool is_doloop)
{
  Set_Error_Phase("Software Pipelining");
  LOOP_DESCR *loop = cl.Loop();
  BB *body = LOOP_DESCR_loophead(loop);
  BB *head = CG_LOOP_prolog;
  BB *tail = CG_LOOP_epilog;
  Is_True(BB_SET_Size(LOOP_DESCR_bbset(loop)) == 1,("can't SWP multi-bb loops."));

  const bool trace = Get_Trace(TP_SWPIPE, 2);
  const bool trace_details = Get_Trace(TP_SWPIPE, 4);
  const bool trace_bundling = Get_Trace(TP_SWPIPE, 0x1000);
  const bool show_result = Get_Trace(TP_SWPIPE, 1);

  if (CG_skip_local_swp && Is_Loop_Skipped()) return FALSE;

  SWP_RETURN_CODE code = Detect_SWP_Constraints(cl, trace);
  if (code != SWP_OK)
    return SWP_Failure(body, code);

  if (trace)
    CG_LOOP_Trace_Loop(loop, "**** Before SWP ****");

  // SWP compile-time tuning parameters
  double max_ii_alpha = SWP_Options.Max_II_Alpha;
  double max_ii_beta  =  SWP_Options.Max_II_Beta;
  double ii_incr_alpha =  SWP_Options.II_Incr_Alpha;
  double ii_incr_beta =  1.0 + (SWP_Options.II_Incr_Beta - 1.0) / max(1,SWP_Options.Opt_Level);
  INT sched_budget = SWP_Options.Budget * max(1,SWP_Options.Opt_Level);

  {
    Start_Timer(T_SWpipe_CU);
    double time0 = Get_User_Time(T_SWpipe_CU);

    CXX_MEM_POOL swp_local_pool("swp pool", FALSE);
    SWP_OP_vector swp_op_vector(body, is_doloop, swp_local_pool());

    // Make sure we have enough non-rotating registers for the loop
    SWP_REG_ASSIGNMENT swp_assign;
    if (!swp_assign.Enough_Non_Rotating_Registers(swp_op_vector.tn_non_rotating)) {
      // TODO: we might be able to convert some invariants into copy
      // and thus uses the rotating registers.
      return SWP_Failure(body, NON_ROT_REG_ALLOC_FAILED );
    }
    
    CG_LOOP_rec_min_ii = CG_LOOP_res_min_ii = CG_LOOP_min_ii = 0;

    // invokes CG_DEP_Compute_Graph, deconstructor deletes graph
    CYCLIC_DEP_GRAPH cyclic_graph( body, swp_local_pool()); 

    if (trace)
      CG_DEP_Trace_Graph(body);

#ifdef TARG_IA64
    // prune loop-carried REGOUT dependences on rotating registers
    Prune_Regout_Deps(body, swp_op_vector.tn_non_rotating);

    if (trace)
      CG_DEP_Trace_Graph(body);
#endif
    
    {
      // Compute CG_LOOP_min_ii.
      MEM_POOL_Push(&MEM_local_pool);
      BOOL ignore_non_def_mem_deps = FALSE;
      CG_LOOP_Make_Strongly_Connected_Components(body, &MEM_local_pool, ignore_non_def_mem_deps); 
      CG_LOOP_Calculate_Min_Resource_II(body, NULL, FALSE /*include pref*/, TRUE /*ignore pref stride*/);
      CG_LOOP_Calculate_Min_Recurrence_II(body, ignore_non_def_mem_deps);

      CG_LOOP_Clear_SCCs(loop);
      MEM_POOL_Pop(&MEM_local_pool);
    }

    double time1 = Get_User_Time(T_SWpipe_CU);

    // Modulo Scheduling
    CG_LOOP_min_ii = max(CG_LOOP_min_ii, SWP_Options.Starting_II);
    INT max_ii = (INT)linear_func(CG_LOOP_min_ii, max_ii_alpha, max_ii_beta);

    // update CG_LOOP_min_ii using MinDist
    MinDist mindist(swp_op_vector, swp_op_vector.start, swp_op_vector.stop,
		    swp_op_vector.branch, CG_LOOP_min_ii);

    if (mindist.Found_ii() != CG_LOOP_min_ii) {
      DevWarn("CG_LOOP_min_ii (%d) is different from RecMII (%d) identified by MinDist.",
	      CG_LOOP_min_ii, mindist.Found_ii());
      CG_LOOP_min_ii = mindist.Found_ii();
    }
    
    Modulo_Schedule(swp_op_vector, CG_LOOP_min_ii, max_ii, ii_incr_alpha, ii_incr_beta,
		    sched_budget, trace, trace_details);

    if (!swp_op_vector.succeeded)
      return SWP_Failure(body, MOD_SCHED_FAILED );

    // Bundling
    if (SWP_Options.Enable_Bundling)
      SWP_Bundle(swp_op_vector, trace_bundling);
    else
      SWP_Dont_Bundle(swp_op_vector);

    if (trace)
      swp_op_vector.Print(TFile);

    double time2 = Get_User_Time(T_SWpipe_CU);

    // Perform Register Allocation to rotating register banks.  The
    // resultant allocation will be in terms of a map from TNs to 
    // positive unbounded locations (swp_assign.reg_allocation), a 
    // lower bound on the number of rotating registers required for
    // the allocation (swp_assign.rotating_reg_used), and a location
    // assigned to swp_assign.control_predicate_loc.
    //
    if (!swp_assign.Allocate_Loop_Variants(swp_op_vector, head, tail)) {
      // failed to allocate loop variants to rotating register banks
      SWP_Undo_Bundle(swp_op_vector, body);
      return SWP_Failure(body, REG_ALLOC_FAILED );
    }
    
#ifdef TARG_IA64
    // Reserve rotating registers to cover the ones needed for this loop.
    // Only integer registers can vary the size of the rotating segment,
    // so there is no need to do it for other register classes.
    //
    REGISTER_Reserve_Rotating_Registers(ISA_REGISTER_CLASS_integer, 
		     swp_assign.rotating_reg_used[ISA_REGISTER_CLASS_integer]);
#endif

    double time3 = Get_User_Time(T_SWpipe_CU);

    // Code Generation
    LOOPINFO *info = LOOP_DESCR_loopinfo(loop);
    TN *trip_count_tn = info ? LOOPINFO_primary_trip_count_tn(info) : NULL;
    SWP_Emit(swp_op_vector, swp_assign, trip_count_tn, 
	     head, body, tail,
	     is_doloop, trace);

    fixup.push_back( SWP_FIXUP(head, body, tail, 
			       swp_assign.control_predicate_loc) );

    double time4 = Get_User_Time(T_SWpipe_CU);

    if (trace)
      CG_LOOP_Trace_Loop(loop, "**** After SWP ****");

    // Generate Statistics
    swp_op_vector.prep_time = time1 - time0;
    swp_op_vector.sched_time = time2 - time1;
    swp_op_vector.reg_alloc_time = time3 - time2;
    swp_op_vector.code_gen_time = time4 - time3;
    if (show_result) 
      SWP_Show_Statistics(swp_op_vector, body);

    Stop_Timer(T_SWpipe_CU);
  }

  return TRUE;
}


/* ====================================================================
 *
 * Emit_SWP_Note
 *
 * Emit a loop note to the .s file, ...
 *
 * ====================================================================
 */
void
Emit_SWP_Note(BB *bb, FILE *file)
{
  ANNOTATION *ant = ANNOT_Get(BB_annotations(bb), ANNOT_ROTATING_KERNEL);
  ROTATING_KERNEL_INFO *info = ANNOT_rotating_kernel(ant);

  if (ROTATING_KERNEL_INFO_succeeded(info)) {
    const char *prefix = "//<swps> ";
    fprintf(file, "%s\n", prefix);
    fprintf(file, "%s%3d cycles per 1 iteration in steady state\n",
	    prefix, ROTATING_KERNEL_INFO_ii(info));
    fprintf(file, "%s%3d pipeline stages\n", 
	    prefix, ROTATING_KERNEL_INFO_stage_count(info));
    fprintf(file, "%s\n", prefix);

    prefix = "//<swps>      ";

    fprintf(file, "%smin %d cycles required by resources\n",
	    prefix, ROTATING_KERNEL_INFO_res_min_ii(info));
    fprintf(file, "%smin %d cycles required by recurrences\n",
	    prefix, ROTATING_KERNEL_INFO_rec_min_ii(info));
    fprintf(file, "%smin %d cycles required by resources/recurrence\n", 
	    prefix, ROTATING_KERNEL_INFO_min_ii(info)); 
    fprintf(file, "%smin %d cycles (actual %d cycles) required to schedule one iteration\n",
	    prefix, ROTATING_KERNEL_INFO_min_sched_len(info), ROTATING_KERNEL_INFO_sched_len(info));
    fprintf(file, "%s\n", prefix);
    TI_RES_COUNT_Emit_Note(prefix, file, ROTATING_KERNEL_INFO_res_counts(info), 
			   ROTATING_KERNEL_INFO_ii(info));
    fprintf(file, "%s\n", prefix);
  } else {
    const char *prefix = "//<swpf> ";
    fprintf(file, "%s\n", prefix);
    char *failure_msg;
    switch (ROTATING_KERNEL_INFO_failure_code(info)) {
    case SWP_PREP_ONLY:
      failure_msg = "disable by -SWP:prep_only";
      break;
    case SWP_ASM:
      failure_msg = "unable to swp a loop containing ASM statements";
      break;
    case SWP_WORKAROUND:
      failure_msg = "disable swp to workaround hardware bugs";
      break;
    case SWP_DEDICATED_ROT_REG:
      failure_msg = "unable to swp a loop with dedicated rotating register binding";
      break;
    case MOD_SCHED_FAILED:
      failure_msg = "unable to find a modulo schedule";
      break;
    case REG_ALLOC_FAILED:
      failure_msg = "not enough rotating register";
      break;
    case NON_ROT_REG_ALLOC_FAILED:
      failure_msg = "not enough non-rotating register";
      break;
    case SWP_LOOP_EMPTY:
      failure_msg = "loop is empty";
      break;
    case SWP_LOOP_LIMIT:
      failure_msg = "loop is too big";
      break;
    default:
      Is_True(FALSE, ("unknown SWP RETURN CODE."));
    }
    fprintf(file, "%s %s\n", prefix, failure_msg);
    fprintf(file, "%s\n", prefix);
  }
}