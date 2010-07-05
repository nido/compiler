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



#include "defs.h"
#include "tracing.h"
#include "cg_flags.h"
#include "tn.h"
#include "tn_set.h"
#include "op.h"
#include "bb.h"
#include "cg.h"
#include "cgprep.h"
#include "cg_loop.h"
#include "cg_loop_mii.h"
#include "cg_loop_recur.h"
#include "cg_dep_graph.h"
#include "cg_sched_est.h"
#include "cg_swp.h"
#include "cg_swp_target.h"
#include "cgexp.h"
#include "const.h"
#include "ti_res_count.h"

typedef std::pair<TN*, OP*> tn_def_op;
typedef std::vector<tn_def_op> op_vec_type;

// For use with stable_sort later
bool operator<(tn_def_op& t1, tn_def_op& t2)
{
  return (t1.first < t2.first);
}

class REPLACE_TN_LIST {

  struct REPLACE_TN {
    TN *old_tn;
    TN *new_tn;
    OP *begin;
    OP *end;
    REPLACE_TN(TN *o, TN *n, OP *b, OP *e):
      old_tn(o), new_tn(n), begin(b), end(e) {}
  };

  vector<REPLACE_TN> v;

public:
  // Add a change to be processed
  void record(TN *o, TN *n, OP *b, OP *e) {
    v.push_back(REPLACE_TN(o, n, b, e));
  }

  // commit the changes
  void commit() {
    for (INT32 i = 0; i < v.size(); i++) {
      TN *old_tn = v[i].old_tn;
      TN *new_tn = v[i].new_tn;
      for (OP *op = v[i].begin; op != v[i].end; op = OP_next(op)) {
	INT32 j = 0;
	for (j = 0; j < OP_opnds(op); j++)
	  if (OP_opnd(op, j) == old_tn)
	    Set_OP_opnd(op, j, new_tn);
	for (j = 0; j < OP_results(op); j++)
	  if (OP_result(op, j) == old_tn)
	    Set_OP_result(op, j, new_tn);
      }
    }
  }
};

/* =======================================================================
 *   Imm_Value_In_Range
 * =======================================================================
 */
static BOOL 
Imm_Value_In_Range (
  OP *op, 
  INT64 imm
)
{
  INT opnd = TOP_Find_Operand_Use(OP_code(op), OU_opnd2);
  if (opnd == -1)
    opnd = TOP_Find_Operand_Use(OP_code(op), OU_offset);
  if (opnd == -1)
    return FALSE;

  TOP new_opc = TOP_opnd_immediate_variant(OP_code(op), opnd, imm);
  return (new_opc != TOP_UNDEFINED);
}

/* =======================================================================
 *   Interleave_Base_Update
 * =======================================================================
 */
void 
Interleave_Base_Update (
  op_vec_type::iterator first, 
  op_vec_type::iterator last,
  BB *prolog, 
  BB *body, 
  BB *epilog, 
  bool trace
)
{
  // no need to perform interleaving on one OP
  if (last - first <= 1) return;

  // We got a chain of definitions
  //
  // The following checks the requirement to perform interleaving!
  //
  TN *tn = (*first).first;
  op_vec_type::iterator p;
  INT32 incr = 0;  // keep track of total incr for tn
  INT32 first_incr = 0;
  INT32 liveout_adj = 0;
  bool need_reg_update = false;

  for (p = first; p != last; p++) {
    OP *op = (*p).second;
    BASE_UPDATE kind = OP_base_update_kind(op);
    if (kind == NO_BASE_UPDATE || OP_cond_def(op)) 
      return;

#ifdef TARG_ST
    INT32 base_opnd_num = TOP_Find_Operand_Use(OP_code(op), OU_base);
    INT32 base_res_num = TOP_base_update_tn(OP_code(op));
    INT32 incr_opnd_num = TOP_Find_Operand_Use(OP_code(op), OU_offset);
#else
    INT32 base_opnd_num = OP_base_opnd_num(op);
    INT32 base_res_num = OP_base_res_num(op);
    INT32 incr_opnd_num = OP_incr_opnd_num(op);
#endif

    Is_True(base_opnd_num >= 0 && base_res_num >= 0, 
	    ("can't find base opnd/result."));

    if (OP_result(op, base_res_num) != tn)
      return;
    
    Is_True(OP_result(op, base_res_num) == OP_opnd(op, base_opnd_num),
	    ("opnd != res in base update form."));
    
    if ((p != first && OP_omega(op, base_opnd_num) != 0) ||
	(p == first && OP_omega(op, base_opnd_num) != 1))
      // already interleaved!
      return;

    if (kind == REG_BASE_UPDATE) {
      need_reg_update = true;
    } else {
      INT32 value = TN_value(OP_opnd(op, incr_opnd_num));
      liveout_adj = -incr;
      incr += value;
      if (p == first) 
	first_incr = value;
    }    
  }
  for (p = first; p != last; p++) {
    OP *op = (*p).second;
    if (!Imm_Value_In_Range(op, incr))
      need_reg_update = true;
  }
  
  if (need_reg_update) {
    DevWarn("recurrence breaking does not support register base update form.");
    return;
  }

  // cannot interleave if it has uses
  // in the loop body
  {
    OP *op;
    FOR_ALL_BB_OPs(body, op) {
      if (OP_Defs_TN(op, tn)) continue; 
      if (OP_Refs_TN(op, tn)) return;
    }
  }

  // Remove copy-in of the original TN
  CG_LOOP_BACKPATCH *bp = CG_LOOP_Backpatch_First(prolog, NULL);
  while (bp && CG_LOOP_BACKPATCH_body_tn(bp) != tn ) 
    bp = CG_LOOP_Backpatch_Next(bp);
  Is_True(bp, ("unable to find backpatch."));
  TN *non_body_tn = CG_LOOP_BACKPATCH_non_body_tn(bp);
  CG_LOOP_Backpatch_Delete(prolog, bp);

  REPLACE_TN_LIST use_update;
  TN *incr_tn = Gen_Literal_TN(incr, 4);
  OPS prolog_ops = OPS_EMPTY;
  INT32 partial_sum = 0;

  for (p = first; p != last; p++) {
    // create new TNs
    OP *op = (*p).second;
    TN *newtn = Dup_TN(tn);
    (*p).first = newtn;

    if (trace) {
      fprintf(TFile, "<recur> TN%d replaces the %d-th occurrence of TN%d\n",
	      TN_number(newtn), p - first, TN_number(tn));
    }

    // Update uses in other operations
    {
      op_vec_type::iterator q = p + 1;
      if (q >= last) q -= (last - first);
      OP *op = (*p).second;
      if (q != first)
	use_update.record(tn, newtn, OP_next(op), (*q).second);
      else {
	use_update.record(tn, newtn, OP_next(op), NULL);
	use_update.record(tn, newtn, BB_first_op(body), op);
      }
    }

    // Update prolog to get correct initialized value
    {
      TN *prolog_tn = Dup_TN(tn);
      CG_LOOP_Backpatch_Add(prolog, prolog_tn, newtn, 1);
#ifdef TARG_ST
      Expand_Add(prolog_tn,                        /* result */
		 non_body_tn,                      /* src1 */
		 Gen_Literal_TN(partial_sum, 4),   /* src2 */
		 MTYPE_I4,  /* don't know which MTYPE to use, fix later */
		 &prolog_ops);
#else
      Build_OP(TOP_adds,
	       prolog_tn,
	       True_TN,
	       Gen_Literal_TN(partial_sum, 4),
	       non_body_tn,
	       &prolog_ops);
#endif
    }

    // Update base-update operations
    {
#ifdef TARG_ST
      INT32 base_opnd_num = TOP_Find_Operand_Use(OP_code(op), OU_base);
      INT32 base_res_num = TOP_base_update_tn(OP_code(op));
      INT32 incr_opnd_num = TOP_Find_Operand_Use(OP_code(op), OU_offset);
#else
      INT32 base_opnd_num = OP_base_opnd_num(op);
      INT32 base_res_num = OP_base_res_num(op);
      INT32 incr_opnd_num = OP_incr_opnd_num(op);
#endif
      partial_sum += TN_value(OP_opnd(op, incr_opnd_num));
      Set_OP_opnd(op, incr_opnd_num, incr_tn); 
      Set_OP_opnd(op, base_opnd_num, newtn);
      Set_OP_result(op, base_res_num, newtn);
      Set_OP_omega(op, base_opnd_num, 1);
    }
  }
  use_update.commit();
  BB_Append_Ops(prolog, &prolog_ops);

  // Update epilog
  {
    CG_LOOP_BACKPATCH *bp = CG_LOOP_Backpatch_First(epilog, NULL);
    while (bp && CG_LOOP_BACKPATCH_body_tn(bp) != tn ) 
      bp = CG_LOOP_Backpatch_Next(bp);
    if (bp) { // if the TN is live-out
      CG_LOOP_BACKPATCH_Set_body_tn(bp, (*(last-1)).first);
      OPS epilog_ops = OPS_EMPTY;
#ifdef TARG_ST
      Expand_Add(non_body_tn, 
		 Gen_Literal_TN(liveout_adj, 4),
		 non_body_tn,
		 MTYPE_I4,  /* don't know which to use ?? fix later*/
		 &epilog_ops);
#else
      Build_OP(TOP_adds,
	       non_body_tn,
	       True_TN,
	       Gen_Literal_TN(liveout_adj, 4),
	       non_body_tn,
	       &epilog_ops);
#endif
      BB_Prepend_Ops(epilog, &epilog_ops);
    }
  }
}

#if 0
enum RECUR_ACTION {
  RECUR_NONE,
  RECUR_BACK_SUB_INVARIANT,
  RECUR_INTERLEAVE,
  RECUR_BACK_SUB_VARIANT
};


//  Recurrence Breaking OP descriptor:
//    Determine the properties of an OP needed to perform
//    recurrence breaking, such as which operand is the invariant ...
//
class RECUR_OP_DESC {
  RECUR_ACTION action;
  INT invar_opnd_num;
  INT second_invar_opnd_num;  // only valid for fma 
  INT reg_opnd_num;
  INT res_num;
  INT new_omega;
  OP *op;
  TYPE_ID mtype;
  bool is_add;
  bool has_one_invar_opnd;
  bool allow_back_sub_variant;
  bool need_copy_variant;
  TN *identity;
  
public:
  OP *Op() { return op; }
  RECUR_ACTION Action() { return action; }
  INT  Invar_opnd_num() { return invar_opnd_num; }
  INT  Second_invar_opnd_num() { return second_invar_opnd_num; }
  INT  Reg_opnd_num() { return reg_opnd_num; }
  INT  Res_num() { return res_num; }
  TYPE_ID Mtype() { return mtype; }
  bool Is_add() { return is_add; }
  INT  New_omega() { return new_omega; }
  TN  *Identity() { return identity; }
  bool Need_copy_variant() { return need_copy_variant; }

  RECUR_OP_DESC(const RECUR_OP_DESC &r) { *this = r; }
#endif

  RECUR_OP_DESC::
  RECUR_OP_DESC(BB *body, BB *epilog, OP *operation, CG_LOOP_DEF& tn_def, 
		TN_SET *multi_def, double estimate_ResMII, bool trace) {

    op = operation;
    action = RECUR_NONE;

    // all recurrence ops have 1 result
    if (OP_results(op) != 1) return;

    // recurrence ops cannot have its result multi-defined.
    if (TN_SET_MemberP(multi_def, OP_result(op, 0))) return;

    // Fix 768488:  dedicated TN cannot have multiple omega
    if (TN_is_dedicated(OP_result(op,0))) return;

    // Fix 796426: Disallow recurrence breaking if there exists more than
    // one reference of the result TN, eg. x = x + x;
    INT i, j;
    INT8 ref_count = 0;
    for (i = 0; i < OP_results(op); i++) {
      TN *result_tn = OP_result(op, i);
      for (j = 0; j < OP_opnds(op); j++) {
	TN *opnd_tn = OP_opnd(op, j);
	if (TN_is_constant(opnd_tn)) continue;
	if (opnd_tn == result_tn) ref_count++;
      }
    }
    if (ref_count > 1) return;

    res_num = -1;
    has_one_invar_opnd = true;
    allow_back_sub_variant = false;
    need_copy_variant = false;
    identity = NULL;

    is_add = (OP_iadd(op) || OP_fadd(op));
    // Target-specific initialization
    Init_OP();
#if 0
    RECUR_OP_DESC_Init_OP(op, 
			  &has_one_invar_opnd, 
			  &is_add,
			  &invar_opnd_num,
			  &second_invar_opnd_num,
			  &reg_opnd_num,
			  &res_num);
#endif

    // TODO: expand it to handle larger omegas
    if (res_num >= 0 &&	!TN_is_dedicated(OP_result(op, res_num))) {

      bool can_interleave = true;
      bool can_back_sub_variant = true;
      
      if (OP_omega(op, reg_opnd_num) == 1) {
	if (tn_def.Is_invariant(OP_opnd(op, invar_opnd_num)) && has_one_invar_opnd)
	  action = RECUR_BACK_SUB_INVARIANT;  // See fix 813388 below
	else {
	  OP *other_op;
	  TN *use = OP_opnd(op, reg_opnd_num);
	  FOR_ALL_BB_OPs(body, other_op) {
	    if (other_op != op &&
		OP_Refs_TN(other_op, use)) {
	      can_interleave = false;
	      break;
	    }
	  }
	  if (can_interleave) {
	    for (CG_LOOP_BACKPATCH *bp = CG_LOOP_Backpatch_First(epilog, NULL);
		 bp != NULL;
		 bp = CG_LOOP_Backpatch_Next(bp)) {
	      if (CG_LOOP_BACKPATCH_body_tn(bp) == use &&
		  CG_LOOP_BACKPATCH_omega(bp) != 0) {
		can_interleave = false;
		break;
	      }
	    }
	  }
	  if (can_interleave)
	    action = RECUR_INTERLEAVE;
	}
      }

      if (allow_back_sub_variant && action == RECUR_NONE) {
	// if it is unable to change the prolog backpatch,
	// disable RECUR_BACK_SUB_VARIANT.
	INT old_omega = OP_omega(op, reg_opnd_num);
	TN *body_tn = OP_opnd(op, reg_opnd_num);
	for (int i = 1+old_omega; i <= old_omega * 2; i++) {
	  CG_LOOP_BACKPATCH *bp;
	  for (bp = CG_LOOP_Backpatch_First(CG_LOOP_prolog, NULL);
	       bp != NULL;
	       bp = CG_LOOP_Backpatch_Next(bp)) {
	    if (CG_LOOP_BACKPATCH_body_tn(bp) == body_tn && 
		CG_LOOP_BACKPATCH_omega(bp) == i) {
	      can_back_sub_variant = false;
	      break;
	    }
	  }
	}
	if (can_back_sub_variant) {
	  action = RECUR_BACK_SUB_VARIANT;
	  TN *body_tn = OP_opnd(op, invar_opnd_num);
	  need_copy_variant = OP_omega(op, invar_opnd_num) != 0;
	  
	  // will need to copy the "variant" if
	  // 1) it is conditionally defined, or
	  // 2) it is defined multiple times, or
	  // 3) it is live-in
	  //
	  if (!need_copy_variant) {
	    OP *other_op;
	    INT def_count = 0;
	    FOR_ALL_BB_OPs(body, other_op) {
	      if (OP_Defs_TN(other_op, body_tn)) {
		if (OP_cond_def(other_op) ||
		    def_count++ > 0) {
		  need_copy_variant = true;
		  break;
		}
	      }
	    }
	  }
	  if (!need_copy_variant) {
	    CG_LOOP_BACKPATCH *bp;
	    for (bp = CG_LOOP_Backpatch_First(CG_LOOP_prolog, NULL);
		 bp != NULL;
		 bp = CG_LOOP_Backpatch_Next(bp)) {
	      if (CG_LOOP_BACKPATCH_body_tn(bp) == body_tn) {
		need_copy_variant = true;
		break;
	      }
	    }
	  }
	}
      }
    }

    // Fix 813388: Abort RECUR_BACK_SUB_INVARIANT if there is a prolog
    // backpatch conflict (just as with RECUR_BACK_SUB_VARIANT above).
    // TODO: This problem could be circumvented by loop peeling.
    if (action == RECUR_BACK_SUB_INVARIANT &&
	CG_LOOP_Backpatch_Find_Non_Body_TN(CG_LOOP_prolog,
					   OP_opnd(op, reg_opnd_num), 2)) {
      action = RECUR_NONE;
    }

    // for triaging
    //
    if (!CG_LOOP_back_substitution && action == RECUR_BACK_SUB_INVARIANT)
      action = RECUR_NONE;
    if (!CG_LOOP_back_substitution_variant && action == RECUR_BACK_SUB_VARIANT)
      action = RECUR_NONE;
    if (!CG_LOOP_interleave_reductions && action == RECUR_INTERLEAVE)
      action = RECUR_NONE;
    
    if (action != RECUR_NONE && action != RECUR_BACK_SUB_VARIANT) {
      INT latency = CG_DEP_Latency(op, op, CG_DEP_REGIN, reg_opnd_num);
      INT illegal_omega_value = 20;
      for (new_omega = max(1, CG_LOOP_recurrence_min_omega); new_omega < illegal_omega_value; new_omega++) {
	// estimate_ResMII is a double
	if (latency <= new_omega * estimate_ResMII)
	  break;
      }
      if (trace)
	fprintf(TFile, "<recur> new_omega=%d, latency=%d, ResMII=%g\n",
		new_omega, latency, estimate_ResMII);

      Is_True(new_omega < illegal_omega_value, ("RECUR_OP_DESC: check latency and ResMII"));
      if (new_omega == 1)
	action = RECUR_NONE;
    }
    if (action != RECUR_NONE) 
      identity = TN_is_float( OP_result(op, res_num)) ? FZero_TN : Zero_TN;
  }
#if 0
};
#endif

/* =======================================================================
 *   Apply_Back_Sub_Invariant
 *
 *   Given an OP and a new omega,  update the loop body and the prolog
 * =======================================================================
 */
void 
Apply_Back_Sub_Invariant (
  OP *op, 
  INT new_omega, 
  BB *prolog, 
  RECUR_OP_DESC& op_desc
)
{
  INT invar_opnd_num = op_desc.Invar_opnd_num();
  INT reg_opnd_num = op_desc.Reg_opnd_num();
  INT res_num = op_desc.Res_num();
  TYPE_ID mtype = op_desc.Mtype();

  TN *invar_tn = OP_opnd(op, invar_opnd_num);
  Is_True(OP_omega(op, reg_opnd_num) == 1,
	  ("Increase_Recurrence_Omega:  expecting omega = 1."));
  Set_OP_omega(op, reg_opnd_num, new_omega);
  OPS prolog_ops = OPS_EMPTY;

  // update loop body depending on the opcode
  OP_Apply_Back_Sub_Invariant (op);

  // Find non-body TN
  TN *body_tn = OP_result(op, res_num);
  TN *non_body_tn = CG_LOOP_Backpatch_Find_Non_Body_TN(prolog, body_tn, 1);
  Is_True(non_body_tn, ("Interleave_Associative_Op: can't find non-body tn"));

  // Update PROLOG BACKPATCH
  for (INT i = 2; i <= new_omega; i++) {
    TN *new_tn = Dup_TN(non_body_tn);
    if (op_desc.Is_add())
      Exp_SUB(mtype, new_tn, non_body_tn, invar_tn, &prolog_ops);
    else
      Exp_ADD(mtype, new_tn, non_body_tn, invar_tn, &prolog_ops);
    CG_LOOP_Backpatch_Add(prolog, new_tn, body_tn, i);
    non_body_tn = new_tn;
  }
 
  BB_Append_Ops(prolog, &prolog_ops);
}

/* =======================================================================
 *   Apply_Back_Sub_Variant
 *
 *   Given an OP and a new omega,  update the loop body and the prolog
 * =======================================================================
 */
bool 
Apply_Back_Sub_Variant (
  RECUR_OP_DESC& op_desc, 
  BB *prolog, 
  BB *body, 
  CG_SCHED_EST *loop_se, 
  bool trace
)
{
  OP *op = op_desc.Op();
  INT invar_opnd_num = op_desc.Invar_opnd_num();
  INT reg_opnd_num = op_desc.Reg_opnd_num();
  INT res_num = op_desc.Res_num();
  TYPE_ID mtype = op_desc.Mtype();
  TN *identity = op_desc.Identity();
  bool need_copy_variant = op_desc.Need_copy_variant();

  // Update loop body depending on opcode
  return OP_Apply_Back_Sub_Variant (op);
}

/* =======================================================================
 *   Apply_Interleave
 *
 *   Given an OP and a new omega,  update the loop body and the prolog
 * =======================================================================
 */
void 
Apply_Interleave (
  OP *op, 
  INT new_omega, 
  BB *prolog, 
  BB *epilog, 
  RECUR_OP_DESC& op_desc
)
{
  INT reg_opnd_num = op_desc.Reg_opnd_num();
  INT res_num = op_desc.Res_num();
  TYPE_ID mtype = op_desc.Mtype();
  TN *identity = op_desc.Identity();

  Is_True(OP_omega(op, reg_opnd_num) == 1,
	  ("Increase_Recurrence_Omega:  expecting omega = 1."));
  Set_OP_omega(op, reg_opnd_num, new_omega);

  OPS epilog_ops = OPS_EMPTY;

  // Update PROLOG BACKPATCH
  TN *body_tn = OP_result(op, res_num);
  for (INT i = 2; i <= new_omega; i++) 
    CG_LOOP_Backpatch_Add(prolog, identity, body_tn, i);
	  
  // Update EPILOG BACKPATCH
  TN *non_body_tn = CG_LOOP_Backpatch_Find_Non_Body_TN(epilog, body_tn, 0);
  if (!non_body_tn) {
    DevWarn("Interleave_Associative_Op: can't find non-body tn");
  } else {
    vector<TN*> backpatch_tns;
    backpatch_tns.push_back(non_body_tn);
    INT i;
    for (i = 1; i < new_omega; i++) {
      TN *new_tn = Dup_TN(non_body_tn);
      backpatch_tns.push_back(new_tn);
      Is_True(CG_LOOP_Backpatch_Find_Non_Body_TN(epilog, body_tn, i) == NULL,
	      ("Apply_Interleave: epilog backpatch existed."));
      CG_LOOP_Backpatch_Add(epilog, new_tn, body_tn, i);
    }
    for (int size = backpatch_tns.size(); size > 1; size = (size + 1) / 2) {
      for (int i = 0; 2 * i < size; i++) {
	TN *dst = backpatch_tns[2 * i];
	if (2 * i + 1 < size) {
	  TN *src = backpatch_tns[2 * i + 1];
	  Exp_ADD(mtype, dst, dst, src, &epilog_ops);
	}
	backpatch_tns[i] = dst;
      }
    }
  }
 
  BB_Append_Ops(epilog, &epilog_ops);
}

/* =======================================================================
 *   Computer critical recurrence
 * =======================================================================
 */
//  Computer critical recurrence

class Critical_Recurrence {
  static const INT NEG_INF = -999;
  vector< vector<INT> >  mindist;
  INT size() const { return mindist.size(); }
public:
  INT operator()(INT i) const { return mindist[i][i] > 0; }
  void Print(FILE *fp) const;
  Critical_Recurrence(const SWP_OP_vector& v, INT start, INT stop, INT branch, INT ii);
};

/* =======================================================================
 *   Critical_Recurrence::Print
 * =======================================================================
 */
void 
Critical_Recurrence::Print(FILE *fp) const
{
  const int n_col = 16;
  fprintf(fp, "Critical_Recurrence %dx%d:\n", size(), size());
  fprintf(fp, "     ");
  for (INT j = 0; j < size(); j++) {
    if (j != 0 && j % n_col == 0)
      fprintf(fp, "\n     ");
    fprintf(fp,"%4d", j);
  }
  fprintf(fp, "\n");
  for (INT i = 0; i < size(); i++) {
    fprintf(fp, "%3d: ", i);
    for (INT j = 0; j < size(); j++) {
      if (j != 0 && j % n_col == 0)
	fprintf(fp, "\n     ");
      fprintf(fp,"%4d", mindist[i][j]);
    }
    fprintf(fp, "\n");
  }
}

/* =======================================================================
 *   Critical_Recurrence::Critical_Recurrence
 * =======================================================================
 */
Critical_Recurrence::Critical_Recurrence (
  const SWP_OP_vector& v, 
  INT start, INT stop,
  INT branch, 
  INT ii) : mindist(v.size(), vector<INT>(v.size(), NEG_INF))
{
  int n_ops = v.size();

  // initialization 
  //
  INT i;
  for (i = 0; i < v.size(); i++) {
    OP *op = v[i].op;
    if (op) {
      for ( ARC_LIST *al = OP_succs(op) ; al; al = ARC_LIST_rest(al) ) {
	ARC *arc    = ARC_LIST_first(al);
	// Skip all PREBR dependence because they will be taken care by
	// the SWP stage control predicates
	if (ARC_kind(arc) == CG_DEP_PREBR) 
	  continue;
	OP  *succ = ARC_succ(arc);
	mindist[i][SWP_index(succ)] =
	  max(mindist[i][SWP_index(succ)], ARC_latency(arc) - ARC_omega(arc) * ii);
	Is_True(succ == v[SWP_index(succ)].op, ("MinDIST: wrong SWP_index."));
      }
      mindist[start][i] = 0;
      mindist[i][stop] = 0;
      mindist[i][branch] = max(mindist[i][branch], 0);
    }
  }

  // Floyd's all pairs shortest paths algorithm.
  // It is based on dynamic programming.
  for (INT k = 0; k < n_ops; k++) 
    for (INT i = 0; i < n_ops; i++) 
      for (INT j = 0; j < n_ops; j++) 
	mindist[i][j] = max(mindist[i][j], mindist[i][k] + mindist[k][j]);
}

/* =======================================================================
 *   OPs_can_be_reassociated
 *
 *   Reassociation can happen before these two OPs
 * =======================================================================
 */
bool 
OPs_can_be_reassociated (
  OP *op1, 
  OP *op2
)
{
  TOP top1 = OP_code(op1);
  TOP top2 = OP_code(op2);

#ifdef TARG_ST
  if (top1 == top2 || top2 == TOP_opposite(top1)) {
#else
  if (top1 == top2 || top2 == Get_Opposite_TOP(top1)) {
#endif
    if (OP_has_predicate(op1) && OP_has_predicate(op2)) {
#ifdef TARG_ST
      if (OP_opnd(op1, OP_find_opnd_use(op1,OU_predicate)) ==
	  OP_opnd(op2, OP_find_opnd_use(op2,OU_predicate)))
#else
      if (OP_opnd(op1, OP_PREDICATE_OPND) ==
	  OP_opnd(op2, OP_PREDICATE_OPND))
#endif
	return true;
    }
  }

  return false;
}

/* =======================================================================
 *   Exchange_Opnd
 *
 *   Swap the operands!
 * =======================================================================
 */
void 
Exchange_Opnd (
  OP *op1, 
  int op1_opnd_num, 
  OP *op2, 
  int op2_opnd_num
)
{
  int op1_omega = OP_omega(op1, op1_opnd_num);
  TN *op1_tn = OP_opnd(op1, op1_opnd_num);

  int op2_omega = OP_omega(op2, op2_opnd_num);
  TN *op2_tn = OP_opnd(op2, op2_opnd_num);

  Set_OP_omega(op1, op1_opnd_num, op2_omega);
  Set_OP_opnd(op1, op1_opnd_num, op2_tn);

  Set_OP_omega(op2, op2_opnd_num, op1_omega);
  Set_OP_opnd(op2, op2_opnd_num, op1_tn);
}

/* =======================================================================
 *   OPs_Are_Dependent
 *
 *   Return true if op2 is dependent on op1.
 * =======================================================================
 */
bool 
OPs_Are_Dependent(OP *op1, OP *op2)
{
  for (int i = 0; i < OP_results(op1); i++) {
    TN *tn = OP_result(op1,i);
    if (TN_is_register(tn) && !TN_is_const_reg(tn)) {
      if (OP_Defs_TN(op2, tn) ||
          OP_Refs_TN(op2, tn))
        return true;
    }
  }
  for (int i = 0; i < OP_opnds(op1); i++) {
    TN *tn = OP_opnd(op1,i);
    if (TN_is_register(tn) && !TN_is_const_reg(tn)) {
      if (OP_Defs_TN(op2, tn))
        return true;
    }
  }
  return false;
}

/* =======================================================================
 *   OP_Can_Sink_Before
 *
 *   Return true if OP can sink to point.
 * =======================================================================
 */
bool 
OP_Can_Sink_Before(OP *sink_op, OP *point)
{
  for (OP *op = OP_next(sink_op); op != point; op = OP_next(op)) {
    if (OPs_Are_Dependent(sink_op, op))
      return false;
  }
  return true;
}

/* =======================================================================
 *   OPND_is_not_critical
 *
 *   Return true if OPND is involved in the critical path
 * =======================================================================
 */
bool 
OPND_is_not_critical (
  OP *succ, 
  int opnd, 
  Critical_Recurrence& critical_recurrence
)
{
  ARC_LIST *al;
  for (al = OP_preds(succ) ; al; al = ARC_LIST_rest(al) ) {
    ARC *arc = ARC_LIST_first(al);
    OP *pred = ARC_pred(arc);
    INT pred_idx = SWP_index(pred);
    if (ARC_opnd(arc) == opnd &&
	critical_recurrence(pred_idx))
      return false;
  }
  return true;
}

/* =======================================================================
 *   Reassociate
 *
 *   Reassociation
 * =======================================================================
 */
void Reassociate(BB *body,
		 SWP_OP_vector& v,
		 int op_idx, 
		 int opnd, 
		 Critical_Recurrence& critical_recurrence,
		 bool trace)
{
  OP *op = v[op_idx].op;
  if (OP_opnd_can_be_reassociated(op, opnd)) {
    INT use_count = 0;
    ARC *critical_arc = NULL;
    for ( ARC_LIST *al = OP_succs(op) ; al; al = ARC_LIST_rest(al) ) {
      ARC *arc = ARC_LIST_first(al);
      OP *succ = ARC_succ(arc);
      INT succ_idx = SWP_index(succ);
      if (ARC_kind(arc) == CG_DEP_REGIN) {
	use_count++;
	if (ARC_omega(arc) == 0 &&
	    critical_recurrence(succ_idx)) 
	  critical_arc = arc;
      }
    }
    bool succeeded = false;
    INT succ_idx;
    INT succ_opnd;
    if (use_count == 1 && 
	critical_arc) {
      OP *succ = ARC_succ(critical_arc);
      succ_idx = SWP_index(succ);
      if (OPs_can_be_reassociated(op, succ) &&
	  OP_Can_Sink_Before(op, succ)) {
      
	//  t = a + b    ==>    t = a + c
	//  d = c + t           d = b + t
	//
	INT critical_opnd = ARC_opnd(critical_arc);
	succ_opnd = OP_other_opnd(succ, critical_opnd);

	if (OPND_is_not_critical(succ, succ_opnd, critical_recurrence) &&
	    OP_opnd_can_be_reassociated(succ, critical_opnd)) {

	  Exchange_Opnd(op, opnd, succ, succ_opnd);
	  if (OP_next(op) != succ) 
	    BB_Sink_Op_Before(body, op, succ);

	  // adjusted for the signed-ness
#ifdef TARG_ST
	  if (!OP_imul(op) && !OP_fmul(op)) {
#else
	  if (!OP_is_multiplication(op)) {
#endif
	    
#ifdef TARG_ST
	    if (OP_iadd(op) || OP_fadd(op))
	      if (OP_iadd(succ) || OP_fadd(succ)) {
#else
	    if (OP_is_addition(op))
	      if (OP_is_addition(succ)) {
#endif
		// ++ ==> no change
	      } else {
		// +- ==> -+
		OP_Change_Opcode(op, TOP_opposite(OP_code(op)));
		OP_Change_Opcode(succ, TOP_opposite(OP_code(succ)));
		
		// the new opnd for -ve should not be in associative position!
		if (OP_opnd_can_be_reassociated(op, opnd))
		  Exchange_Opnd(op, opnd, op, OP_other_opnd(op, opnd));
	      }
	    else 
#ifdef TARG_ST
	      if (OP_iadd(succ) || OP_fadd(succ)) {
#else
	      if (OP_is_addition(succ)) {
#endif
		// -+ ==> no change
	      } else {
		// -- ==> +-
		OP_Change_Opcode(op, TOP_opposite(OP_code(op)));
		Exchange_Opnd(succ, succ_opnd, succ, OP_other_opnd(succ, succ_opnd));
		succ_opnd = OP_other_opnd(succ, succ_opnd);
	      }
	  }
	      
	  succeeded = true;
	}
      }
    } 
    if (trace)
      fprintf(TFile, "<reassoc> %s reassoc OP%d opnd%d\n", 
	      succeeded ? "" : "not", op_idx, opnd);
  
    if (succeeded) 
      Reassociate(body, v, succ_idx, succ_opnd, critical_recurrence, trace);

    BB_Update_OP_Order(body);
  }
}

/* =======================================================================
 *   Shorten_Critical_Recurrence_By_Reassociation
 *
 *   Shorten Critical Recurrence by Reassociation
 * =======================================================================
 */
void 
Shorten_Critical_Recurrence_By_Reassociation(
  CG_LOOP& cl,
  BOOL is_doloop,
  BOOL trace
)
{
  CXX_MEM_POOL local_pool("local pool", FALSE);
  
  BB *body = cl.Loop_header();

  // Ignore the address computations.
  OP *op;
  INT count = 0;
  FOR_ALL_BB_OPs(body, op) {
    Reset_OP_loh(op);
#ifdef TARG_ST
    // Can we be more generic ?
    if ((OP_iadd(op) || OP_fadd(op)) &&
	(OP_result(op, 0) == OP_opnd(op, 1) ||
	 OP_result(op, 0) == OP_opnd(op, 2)))
      Set_OP_loh(op);
#else
    if (OP_code(op) == TOP_adds &&
	OP_result(op, 0) == OP_opnd(op, 2))
      Set_OP_loh(op);
    if (OP_code(op) == TOP_add &&
	(OP_result(op, 0) == OP_opnd(op, 1) ||
	 OP_result(op, 0) == OP_opnd(op, 2)))
      Set_OP_loh(op);
#endif
    count++;
  }

  if (count + SWP_OPS_OVERHEAD > SWP_OPS_LIMIT) return;  // loop is too big

  SWP_OP_vector v(body, is_doloop, local_pool());


  CG_SCHED_EST *loop_se = CG_SCHED_EST_Create(body, local_pool(),
					      SCHED_EST_FOR_UNROLL |
					      SCHED_EST_IGNORE_LOH_OPS |
					      SCHED_EST_IGNORE_PREFETCH);
  
  int ii = CG_SCHED_EST_Resource_Cycles(loop_se);

  // invokes CG_DEP_Compute_Graph, deconstructor deletes graph
  CYCLIC_DEP_GRAPH manager( body, local_pool()); 
  
  // Identify critical recurrrences
  Critical_Recurrence critical_recurrence(v, 
					  v.start, 
					  v.stop,
					  v.branch,
					  ii);

  if (trace) {
    fprintf(TFile, "Shorten Critical Recurrence by Reassociation:\n");
    CG_DEP_Trace_Graph(body);
    for (int i = 0; i < v.size(); i++) {
      if (v[i].op) {
	fprintf(TFile, "%3d: ", i);
	Print_OP_No_SrcLine(v[i].op);
      }
    }
    fprintf(TFile,"II is %d\n", ii);
    fprintf(TFile, "<ti resource count> ");
    TI_RES_COUNT_Print(TFile, loop_se->res_count);
    fprintf(TFile, "\n");
    critical_recurrence.Print(TFile);
  }

  for (int i = 0; i < v.size(); i++) {
    if (v[i].op && 
	OP_is_associative(v[i].op) &&
	critical_recurrence(i)) {

      Is_True(OP_results(v[i].op) == 1,
	      ("associative OP must have 1 result."));

      if (trace)
	fprintf(TFile, "<critical recurrence> OP%d is critical.\n", i);

      ARC_LIST *al;
      for (al = OP_preds(v[i].op) ; al; al = ARC_LIST_rest(al) ) {
	ARC *arc = ARC_LIST_first(al);
	OP *pred = ARC_pred(arc);
	INT pred_idx = SWP_index(pred);
	if (ARC_kind(arc) == CG_DEP_REGIN &&
	    ARC_omega(arc) == 1 && 
	    critical_recurrence(pred_idx))
	  Reassociate(body, v, i, ARC_opnd(arc), critical_recurrence, trace);
      }
    }
  }
}

/* =======================================================================
 *   Fix_Recurrences_Before_Unrolling
 * =======================================================================
 */
void 
Fix_Recurrences_Before_Unrolling (
  CG_LOOP& cl
)
{
  if (!CG_LOOP_fix_recurrences) return;

  bool trace = Get_Trace(TP_CGLOOP, 0x800);

  if (CG_LOOP_reassociate)
    Shorten_Critical_Recurrence_By_Reassociation(cl, TRUE, trace);

  BB *body = cl.Loop_header();
  if (BB_length(body) > CG_maxinss) return;

  CXX_MEM_POOL local_pool("fix recurrence pool", FALSE);
  TN_SET *tn_def = TN_SET_Create_Empty(Last_TN + 1, local_pool());
  TN_SET *multi_def = TN_SET_Create_Empty(Last_TN + 1, local_pool());
  OP *op;
  FOR_ALL_BB_OPs(body, op) {
    for (INT i = 0; i < OP_results(op); i++) {
      TN *res = OP_result(op,i);
      if (TN_is_register(res) && !TN_is_const_reg(res)) {
	if (TN_SET_MemberP(tn_def, res)) {
	  multi_def =  TN_SET_Union1D(multi_def, res, local_pool());
	} else
	  tn_def = TN_SET_Union1D(tn_def, res, local_pool());
      }
    }
  }

  BB *prolog = CG_LOOP_prolog;
  BB *epilog = CG_LOOP_epilog;
  CG_LOOP_DEF loop_def(body);

  // Perform resource estimate using ideal unrolling,

  CG_SCHED_EST *loop_se = CG_SCHED_EST_Create(body, local_pool(), 
					      SCHED_EST_FOR_UNROLL | 
					      SCHED_EST_IGNORE_BRANCH |
					      SCHED_EST_IGNORE_INT_OPS |
					      SCHED_EST_IGNORE_PREFETCH);

  double estimate_ResMII = CG_SCHED_EST_Resources_Min_Cycles(loop_se);

  // if we find a loop with memory and FP ops, then we must count
  // the integer ops for resource utilization
  if (estimate_ResMII < 0.1) {
    loop_se = CG_SCHED_EST_Create(body, local_pool(), 
				  SCHED_EST_FOR_UNROLL | 
				  SCHED_EST_IGNORE_BRANCH |
				  SCHED_EST_IGNORE_PREFETCH);
    estimate_ResMII = CG_SCHED_EST_Resources_Min_Cycles(loop_se);
  }

  vector<RECUR_OP_DESC> delay_processing;

  FOR_ALL_BB_OPs(body, op) {

    if (! OP_has_predicate(op) || 
      /* (cbr) predicate operand # is not necessary constant */
#ifdef TARG_ST
	! TN_is_true_pred(OP_opnd(op, OP_find_opnd_use(op, OU_predicate))))
#else        
	! TN_is_true_pred(OP_opnd(op, OP_PREDICATE_OPND))) 
#endif
    {
	continue;
    }

    RECUR_OP_DESC op_desc(body, epilog, op, loop_def, multi_def, estimate_ResMII, trace);

    INT new_omega = op_desc.New_omega();

    switch (op_desc.Action()) {

    case RECUR_BACK_SUB_INVARIANT:
      Apply_Back_Sub_Invariant(op, new_omega, prolog, op_desc);
	
      if (trace)
	fprintf(TFile, "<back_sub_invar> TN%d, %s\n", 
		TN_number(OP_result(op, op_desc.Res_num())), TOP_Name(OP_code(op)));

      break;

    case RECUR_INTERLEAVE:

       Apply_Interleave(op, new_omega, prolog, epilog, op_desc);

       if (trace)
	fprintf(TFile, "<interleave> TN%d, %s\n", 
		TN_number(OP_result(op, op_desc.Res_num())), TOP_Name(OP_code(op)));

       break;

    case RECUR_BACK_SUB_VARIANT:

      delay_processing.push_back(op_desc);
      if (trace)
	fprintf(TFile, "<back_sub_variant> TN%d, %s\n", 
		TN_number(OP_result(op, op_desc.Res_num())), TOP_Name(OP_code(op)));
      
      break;
    }
  }

  bool changed = true;
  while (changed) {
    changed = false;
    for (int i = 0; i < delay_processing.size(); i++) {
      RECUR_OP_DESC& op_desc = delay_processing[i];
      changed |= Apply_Back_Sub_Variant(op_desc, prolog, body, loop_se, trace);
    }
  }
}

/* =======================================================================
 *   Fix_Recurrences_After_Unrolling
 * =======================================================================
 */
void 
Fix_Recurrences_After_Unrolling(CG_LOOP& cl)
{
  if (!CG_LOOP_fix_recurrences)
    return;

  if (!CG_LOOP_interleave_posti)
    return;

  LOOP_DESCR *loop = cl.Loop();
  BB *body = cl.Loop_header();
  BB *prolog = CG_LOOP_prolog;
  BB *epilog = CG_LOOP_epilog;
  op_vec_type op_vec;

  if (BB_length(body) > CG_maxinss) return;

  bool trace = Get_Trace(TP_CGLOOP, 0x800);

  // Detect simple recurrence from base-update form
  OP *op;
  FOR_ALL_BB_OPs(body, op) {
    for (INT32 i = 0; i < OP_results(op); i++) {
      TN *tn = OP_result(op, i);
      op_vec.push_back(tn_def_op(tn, op));
    }
  }

  // Use stable sort to maintain original OP ordering
  stable_sort(op_vec.begin(), op_vec.end());
  
  if (trace) {
    for (int i = 0; i < op_vec.size(); i++) {
      TN *tn = op_vec[i].first;
      OP *op = op_vec[i].second;
      fprintf(TFile, "<recur>:  TN%d, %s ", TN_number(tn), TOP_Name(OP_code(op)));
      Print_OP_No_SrcLine(op);
    }
  }

  op_vec_type::iterator p = op_vec.begin();

  while (p != op_vec.end()) {
    TN *cur_tn = (*p).first;
    op_vec_type::iterator q = p;
    while (q != op_vec.end() && cur_tn == (*q).first) q++;

    // [p,q) now defines the range OPs defining the same TN

    Interleave_Base_Update(p, q, prolog, body, epilog, trace);

    p = q;
  }
}


