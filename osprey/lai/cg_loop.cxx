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


//-*-c++-*-
/* =======================================================================
 * =======================================================================
 *
 *  Module: cg_loop.cxx
 *
 *
 * =======================================================================
 * ======================================================================= */

/*
  
  The CG loop analysis and transformation module. 
  
  This file contains the main driver of all CG loop optimizations.  The
  loop optimizations involved are:
  
    - scalar variable renaming
    - vector read/write removal
    - induction variable removal
    - recurrence breaking (or height reduction)
    - predicate promotion
    - unrolling 
    - prefetch pruning and generation
    - various preconditioning to make modulo scheduling more effectively
    - modulo scheduling
    - rotating register allocation
  
  The IA-64 loop optimizer is developed by the team of Raymond Lo, Rune
  Dahl, and David Stephenson during the period from Apr 1999 to Feb
  2000.
  
  The cyclic intermediate representation is carried over from the
  Mongoose compiler, along with most of the utility functions.  Several
  key components are newly developed for the IA-64 architecture, namely,
  the modulo scheduler, the rotating register allocator, recurrence
  breaking, vector read/write removal, and the unrolling heuristics.
  Other optimizations are modified significantly to support the
  predicated instruction set.
  
  The modulo scheduler is based on "Lifetime-Sensitive Modulo
  Scheduling", by Richard Huff, in Programming Language Design and
  Implementation. SIGPLAN, 1993.
  
  The rotating register allocator is based on "Register Allocation for
  Software Pipelined Loops", by B. R. Rau, M. Lee, P. P. Tirumalai,
  M. S. Schlansker, in ACM SIGPLAN '92 PLDI-6/92/CA.  Some idea of the
  paper was conceived by Ross Towle and Jim Dehnert when they're working
  on the Cydra 5 compiler.
  
  The basic idea of the recurrence breaking and vector read/write
  removal can be found in "Compiling for the Cydra 5", by Jim Dehnert,
  Ross Towle, in the Journal of Supercomputing, 7, 181-227 (1993).
  
  The relatively straightforward part of the loop optimizer is the
  modulo scheduler and the register allocator because there have been
  many years of compiler researches available.  The modulo scheduler and
  register allocator almost find the optimal schedules and
  close-to-optimal allocations, right after their initial
  implementation.  The more complicated part of the loop optimizer is
  the preconditioning of a loop for modulo scheduling.  The throughput
  of a modulo scheduled loop is bound by MII=max(RecMII, ResMII).  The
  objective of preconditioning is to lower the RecMII and/or the ResMII
  by various means.  One example is to apply recurrence height reduction
  to decrease RecMII such that RecMII is no longer the dominant factor
  in determining MII.  Sometimes extra resources, at the expense of
  ResMII, can be consumed to further reduce the recurrence height if the
  final MII is reduced.  Another example is to use unrolling to fill all
  slots of VLIW instructions to improve ResMII.  Sometimes instructions
  can be combined after unrolling.  The difficulty is that unrolling,
  recurrence height reduction, and other optimizations interacts with
  each other because they are changing the number and mix of
  instructions at the same time.  An optimization that seems to be
  profitable might be not after the loop is unrolled.  Therefore, the
  final phases of the loop optimizer development will focus mostly on
  the fine-tuning of the interactions among optimizations.
   
*/

#include "W_math.h"
#include <stdarg.h>
// [HK]
#if __GNUC__ >= 3
#include <set>
#else
#include <set.h>
#endif // __GNUC__ >= 3

#include "defs.h"
#include "config.h"
#include "errors.h"
#include "mempool.h"
#include "cg_flags.h"
#include "cgir.h"
#include "tracing.h"
#include "config_asm.h"
#include "tn_map.h"
#include "cgprep.h"
#include "op.h"
#include "op_list.h"
#include "bb.h"
#include "cgtarget.h"
#include "cg_swp_target.h"
#include "wn.h"
#include "wn_core.h"
#include "wn_util.h"
#include "whirl2ops.h"

#include "cg.h"
#include "register.h"	/* needed for "gra.h" */
#include "tn_set.h"	/* needed for "gra.h" */
#include "gra.h"
#include "gra_live.h"
#include "bb_set.h"
#include "gtn_universe.h"
#include "gtn_set.h"
#include "pf_cg.h"
#include "note.h"
#include "cgexp.h"
#include "cio.h"
#include "cio_rwtran.h"
#include "cg_cflow.h"
#include "resource.h"
#include "cg_db_op.h"
#include "dominate.h"
#include "ti_res_count.h"
#include "cg_loop_scc.h"
#include "cg_loop_recur.h"
#include "cg_loop_mii.h"
#include "freq.h"
#include "cg_region.h"
#include "cg_sched_est.h"
#include "cg_loop.h"
#include "cg_swp.h"
#include "cg_swp_options.h"
#include "findloops.h"
#include "dominate.h"
#include "ebo.h"
#include "hb.h"
#include "gra_live.h"
#ifdef TARG_ST
#include "cg_ivs.h"
#endif

/* Error tolerance for feedback-based frequency info */
#define FB_TOL 0.05

CG_LOOP *Current_CG_LOOP;

BB *CG_LOOP_prolog;
BB *CG_LOOP_epilog;
BB *CG_LOOP_prolog_start;
BB *CG_LOOP_epilog_end;

OP_MAP _CG_LOOP_info_map;
  
BOOL CG_LOOP_unroll_fully = TRUE;
#ifdef TARG_ST
BOOL CG_LOOP_unroll_remainder_fully = FALSE;
#else
BOOL CG_LOOP_unroll_remainder_fully = TRUE;
#endif

#ifdef TARG_ST
BOOL CG_LOOP_unroll_do_unwind = FALSE;
BOOL CG_LOOP_unroll_remainder_after = FALSE;
enum UNROLL_HEURISTICS {
  DEFAULT_HEURISTIC = 0,
  SCHED_HEURISTIC   = 1,
  UNROLL2_HEURISTIC = 2,
  DOWHILE_HEURISTIC = 4
};
INT32 CG_LOOP_unroll_heuristics = SCHED_HEURISTIC | UNROLL2_HEURISTIC | DOWHILE_HEURISTIC;
/* CG_LOOP load_store_packing:
   0x1 : Enable load packing
   0x2 : Enable store packing
   0x4 : Disable load paking if alignment is unknown
   0x8 : Perform packing analysis and loop transformation, but do not actually pack
   0x10: Disable loop peeling if loop trip count is a compile time literal value
   0x20: If loop unrolling would not unroll a loop, but packing asks
         for an even unroll factor, forces loop unrolling to 2.
   0x40: Packing is not performed if the loop is not memory bounded,
         but loop transformations are performed.
   0x80: Allow loop specialization
*/
INT32 CG_LOOP_load_store_packing = 0;
INT32 CG_LOOP_unaligned_packing = 0;
INT32 CG_LOOP_stream_align = 4;
#endif
BOOL CG_LOOP_ignore_pragmas = FALSE;

#ifdef MIPS_UNROLL
BOOL CG_LOOP_unroll_analysis = TRUE;
#else
BOOL CG_LOOP_unroll_analysis = FALSE;
#endif
BOOL CG_LOOP_multi_bb_unroll_analysis = TRUE;
UINT32 CG_LOOP_unroll_analysis_threshold = 10;
BOOL CG_LOOP_ooo_unroll_heuristics = FALSE;
BOOL CG_LOOP_ooo_unroll_heuristics_set = FALSE;
UINT32 CG_LOOP_reorder_buffer_size = 16;
UINT32 CG_LOOP_cache_miss_threshold = 33;

BOOL CG_LOOP_unroll_multi_bb = FALSE;
BOOL CG_LOOP_unroll_non_trip_countable = TRUE;
BOOL CG_LOOP_unroll_multi_make_remainder_loop = TRUE;
BOOL CG_LOOP_create_loop_prologs;	/* see Configure_CG_Options() */
BOOL CG_LOOP_create_loop_epilogs = TRUE;
INT32 CG_LOOP_force_ifc = 1;

BOOL CG_LOOP_optimize_non_trip_countable = TRUE;
BOOL CG_LOOP_optimize_non_innermost = TRUE;
BOOL CG_LOOP_optimize_multi_targ = FALSE;
BOOL CG_LOOP_optimize_lno_winddown_cache = TRUE;
BOOL CG_LOOP_optimize_lno_winddown_reg = TRUE;

/* Note: To set default unroll parameters, modify the initialization
 *	 of OPT_unroll_times/size in "config.c".
 */
UINT32 CG_LOOP_unroll_times_max;
UINT32 CG_LOOP_unrolled_size_max;

static BOOL unroll_names_valid = FALSE;

static CG_LOOP_BACKPATCH *prolog_backpatches, *epilog_backpatches;

/* Statically check a few assumptions .. */
#if MAX_OMEGA > 255
error OP_omega/Set_OP_omega assume MAX_OMEGA <= 255
#endif

_CG_LOOP_INFO *free_loop_info;
#define next_free_loop_info(info) (*(_CG_LOOP_INFO **)(info))


void CG_LOOP_Init(void)
/* -----------------------------------------------------------------------
 * See "cg_loop.h" for interface.
 * -----------------------------------------------------------------------
 */
{
  free_loop_info = NULL;
  // No loop optimizations at -O0 and -O1
  if (CG_opt_level < 2) return;
  unroll_names_valid = FALSE;
}

void CG_LOOP_Init_Op(OP *op)
/* -----------------------------------------------------------------------
 * See "cg_loop.h" for interface.
 * -----------------------------------------------------------------------
 */
{
  _CG_LOOP_INFO *info = free_loop_info;
  INT sizeof_info = _CG_LOOP_info_sizeof(op);
  if (info && OP_opnds(op) <= OP_MAX_FIXED_OPNDS) {

    // Since we don't track the size of the _CG_LOOP_INFO structs on
    // the free list, don't use one for a variable operand OP uses more
    // than OP_MAX_FIXED_OPNDS, instead allocate a new one. We can however 
    // guarantee that all _CG_LOOP_INFO structs allocate are large enough 
    // to handle all fixed operand OPs, so anything on the free list will 
    // do in that case.
    free_loop_info = next_free_loop_info(info);
  } else {
    info = (_CG_LOOP_INFO *)MEM_POOL_Alloc(&MEM_phase_nz_pool, sizeof_info);
  }
  BZERO(info, sizeof_info);
  OP_MAP_Set(_CG_LOOP_info_map, op, info);
}


void CG_LOOP_Init_OPS(OPS *ops)
{
  for (OP *op = OPS_first(ops); op != NULL; op = OP_next(op)) 
    CG_LOOP_Init_Op(op);
}


INT Branch_Target_Operand(OP *br_op)
/* -----------------------------------------------------------------------
 * Return the branch target operand of <br_op>.
 * -----------------------------------------------------------------------
 */
{
  Is_True(br_op, ("br_op is NULL."));
  INT opnd = OP_find_opnd_use(br_op, OU_target);
  Is_True(opnd >= 0,
	  ("couldn't find branch target for %s", TOP_Name(OP_code(br_op))));
  return opnd;
}


static BOOL Negate_Branch(OP *br)
/* -----------------------------------------------------------------------
 * Negate the branch <br> and return TRUE successful
 * -----------------------------------------------------------------------
 */
{
  TOP top = OP_code(br);
  TOP new_top = CGTARG_Invert(top);

  if (new_top != TOP_UNDEFINED) {
    OP_Change_Opcode(br, new_top);
    return TRUE;
  }

  if (OP_has_predicate(br)) {
    OP *cmp;
    TN *tn1, *tn2;
    BB *br_bb = OP_bb(br);
    CGTARG_Analyze_Compare(br, &tn1, &tn2, &cmp);
    if (cmp != NULL && OP_results(cmp) == 2) {
      BB *cmp_bb = OP_bb(cmp);
      TN *r0 = OP_result(cmp,0);
      TN *r1 = OP_result(cmp,1);
      TN *pred = OP_opnd(br,OP_PREDICATE_OPND);
      TN *neg_tn = r0 == pred ? r1 : r0;

      if (neg_tn == True_TN) {
        DevWarn("negative cmp result is a sink");
        return FALSE;
      }

      Set_OP_opnd(br, OP_PREDICATE_OPND, neg_tn);

      if (br_bb != cmp_bb && !CG_localize_tns) {
        GRA_LIVE_Compute_Local_Info(br_bb);
        GRA_LIVE_Region_Start();
        GRA_LIVE_Region_Entry(cmp_bb);
        GRA_LIVE_Region_Exit(br_bb);
        GRA_LIVE_Region_Compute_Global_Live_Info();
      }

      return TRUE;
    }
  }

  return FALSE;
}


static void loop_info_detach(OP *op)
/* -----------------------------------------------------------------------
 * Detach _CG_LOOP_INFO from <op>.
 * -----------------------------------------------------------------------
 */
{
  _CG_LOOP_INFO *info = (_CG_LOOP_INFO *)OP_MAP_Get(_CG_LOOP_info_map, op);
  Is_True(info, ("no loop info to delete"));
  next_free_loop_info(info) = free_loop_info;
  free_loop_info = info;
}

static float sum_succ_probs(BB *bb)
/* -----------------------------------------------------------------------
 * Return the sum of the probabilities on the successor edges from <bb>.
 * (Should be 1.0 in consistent CFG.)
 * -----------------------------------------------------------------------
 */
{
  BBLIST *succs;
  float sum = 0.0;

  FOR_ALL_BB_SUCCS(bb, succs) {
    if (CG_warn_bad_freqs && !BB_freq_fb_based(bb)
	&& BBLIST_prob(succs) == 0.0)
      DevWarn("succ edge from BB:%d to BB:%d has probability 0",
	      BB_id(bb), BB_id(BBLIST_item(succs)));
    sum += BBLIST_prob(succs);
  }
  return sum;
}

static void retarget_loop_exits(LOOP_DESCR *loop, BB *from, BB *to)
/* -----------------------------------------------------------------------
 * Requires: <to> is inserted where intended in BB chain
 *
 * Change all exits from <loop> to <from> so they exit to <to> instead.
 * Updates pred/succ lists and frequency info.
 * -----------------------------------------------------------------------
 */
{
  BBLIST *preds = BB_preds(from);
  while (preds) {
    BB *pred = BBLIST_item(preds);
    preds = BBLIST_next(preds);
    if (BB_SET_MemberP(LOOP_DESCR_bbset(loop), pred))
      if (!BB_Retarget_Branch(pred, from, to))
	/* must fall-through to <from> */
	Change_Succ(pred, from, to);
  }
}

static void insert_fall_thru(BB *pred, BB *new_ftbb)
/* -----------------------------------------------------------------------
 * Requires: BB_Fall_Thru_Successor(pred) != NULL && new_ftbb not in BB chain
 *
 * Inserts <new_ftbb> between <pred> and its old fall-through successor,
 * so that <pred> falls through to <new_ftbb> which falls through to the
 * old fall-through successor.  Updates pred/succ lists and frequency info
 * (if present).
 * -----------------------------------------------------------------------
 */
{
  BOOL freqs = FREQ_Frequencies_Computed();
  BB *old_ftbb = BB_Fall_Thru_Successor(pred);
  float other_succ_probs = 0.0;
  OP *new_br_op = BB_branch_op(new_ftbb);

  Is_True(old_ftbb, ("<pred> has no fall-through successor"));
  Is_True(new_br_op == NULL || OP_cond(new_br_op),
	    ("<new_ftbb> BB:%d doesn't fall-through to anything",
	     BB_id(new_ftbb)));
  Is_True(BB_next(new_ftbb) == NULL,
	    ("<new_ftbb> BB:%d has a BB_next", BB_id(new_ftbb)));
  Is_True(BB_prev(new_ftbb) == NULL,
	    ("<new_ftbb> BB:%d has a BB_prev", BB_id(new_ftbb)));
  if (freqs) {
    other_succ_probs = sum_succ_probs(new_ftbb);
    if (CG_warn_bad_freqs && 1.0 - other_succ_probs <= 0.0)
      DevWarn("existing succ edge probabilities from BB:%d total %s1.0",
	      BB_id(new_ftbb), other_succ_probs > 1.0 ? ">" : "");
  } else if (BB_freq_fb_based(old_ftbb)) {
    /* Guess that P(succ) is same for all succs */
    UINT32 len = BBlist_Len(BB_succs(new_ftbb)) + 1;
    other_succ_probs = (len - 1.0) / len;
  }
  Chain_BBs(pred, new_ftbb);
  Change_Succ(pred, old_ftbb, new_ftbb);
  Chain_BBs(new_ftbb, old_ftbb);
  Link_Pred_Succ_with_Prob(new_ftbb, old_ftbb, 1.0-other_succ_probs);
  if (freqs || BB_freq_fb_based(old_ftbb))
    BB_freq(old_ftbb) += BB_freq(new_ftbb) * (1.0 - other_succ_probs);
}


inline void append_to_prolog(BB *bb)
/* -----------------------------------------------------------------------
 * Attach <bb> to the end of CG_LOOP_prolog, initializing GRA liveness
 * info and updating pred/succ lists and frequency info.  Upon return,
 * CG_LOOP_prolog is <bb>.
 * -----------------------------------------------------------------------
 */
{
  GRA_LIVE_Compute_Local_Info(bb);
  insert_fall_thru(CG_LOOP_prolog, bb);
  CG_LOOP_prolog = bb;
}



inline void extend_prolog(void)
/* -----------------------------------------------------------------------
 * Insert a new block at the end of CG_LOOP_prolog, and point
 * CG_LOOP_prolog to it.  Also initializes GRA liveness info and
 * updates pred/succ lists and frequency info.
 * -----------------------------------------------------------------------
 */
{
  BB *new_bb = Gen_BB_Like(CG_LOOP_prolog);
  if (BB_freq_fb_based(CG_LOOP_prolog)) Set_BB_freq_fb_based(new_bb);
  append_to_prolog(new_bb);
}



static void prepend_to_epilog(LOOP_DESCR *loop, BB *bb)
/* -----------------------------------------------------------------------
 * Attach <bb> to the start of CG_LOOP_epilog, initializing GRA liveness
 * info and updating pred/succ lists and frequency info.  Upon return,
 * CG_LOOP_epilog is <bb>.  Retargets exits from <loop> to CG_LOOP_epilog
 * to <bb>.
 * -----------------------------------------------------------------------
 */
{
  BB *ftp = BB_Fall_Thru_Predecessor(CG_LOOP_epilog);
  GRA_LIVE_Compute_Local_Info(bb);
  if (ftp) {
    insert_fall_thru(ftp, bb);
  } else {
    Chain_BBs(BB_prev(CG_LOOP_epilog), bb);
    Chain_BBs(bb, CG_LOOP_epilog);
  }
  retarget_loop_exits(loop, CG_LOOP_epilog, bb);
  CG_LOOP_epilog = bb;
}



inline void extend_epilog(LOOP_DESCR *loop)
/* -----------------------------------------------------------------------
 * Insert a new block at the start of CG_LOOP_epilog, and point
 * CG_LOOP_epilog to it.  Initializes GRA liveness info and
 * updates pred/succ lists and frequency info.  Also retargets
 * exits from <loop> to CG_LOOP_epilog to the new BB.
 * -----------------------------------------------------------------------
 */
{
  BB *new_bb = Gen_BB_Like(CG_LOOP_epilog);
  if (BB_freq_fb_based(CG_LOOP_epilog)) Set_BB_freq_fb_based(new_bb);
  prepend_to_epilog(loop, new_bb);
}



void CG_LOOP_Trace_Prolog(void)
/* -----------------------------------------------------------------------
 * See "cg_loop.h" for interface.
 * -----------------------------------------------------------------------
 */
{
  BB *bb;
  fprintf(TFile, "<prolog> loop prolog:\n");
  for (bb = CG_LOOP_prolog_start;
       bb && BB_prev(bb) != CG_LOOP_prolog;
       bb = BB_next(bb))
    Print_BB(bb);
}



void CG_LOOP_Trace_Epilog(void)
/* -----------------------------------------------------------------------
 * See "cg_loop.h" for interface.
 * -----------------------------------------------------------------------
 */
{
  BB *bb;
  fprintf(TFile, "<epilog> loop epilog:\n");
  for (bb = CG_LOOP_epilog;
       bb && BB_prev(bb) != CG_LOOP_epilog_end;
       bb = BB_next(bb))
    Print_BB(bb);
}


static BB_SET *
mark_prolog_epilog_bbs(
  BB     *self,
  BB     *limit,
  BB_SET *visited
)
{
  BB        *pred;
  BBLIST    *lst;

  visited = BB_SET_Union1(visited,self,&MEM_local_pool);

  if ( self != limit ) {
    FOR_ALL_BB_PREDS (self, lst) {
      pred = BBLIST_item(lst);
      if ( ! BB_SET_MemberP(visited,pred) ) {
        visited = mark_prolog_epilog_bbs(pred,limit,
					 visited);
      }
    }
  }

  return visited;
}

/* -----------------------------------------------------------------------
 * static void remove_prolog_or_epilog_bbs(BB *head, BB *tail, BB *keep)
 *
 *  Remove all the prolog/epilog BB's between {<head>,<tail>} except <keep>. 
 *
 * -----------------------------------------------------------------------
 */
static void
remove_prolog_or_epilog_bbs(BB *head, BB *tail, BB *keep, BB_SET *inp_visited)
{
  BB *bb;
  BB *bb_next;
  BB_SET *visited;
  BB *pred;
  BB *succ;
  BBLIST *lst;
  BBLIST *lst_next;
  RID *bbrid;

  if ( inp_visited == NULL ) {
    MEM_POOL_Push(&MEM_local_pool);
    visited = BB_SET_Create_Empty(PU_BB_Count+2,&MEM_local_pool);
    visited = mark_prolog_epilog_bbs(tail,head,visited);
  } else {
    visited = inp_visited;
  }

  for (bb = REGION_First_BB; bb; bb = bb_next) {
    bb_next = BB_next(bb);

    if (    ( bbrid = BB_rid( bb ) )
	&& ( RID_level( bbrid ) >= RL_CGSCHED ) )
      /*
       * don't look inside already compiled bb's
       */
      continue;

    if ( bb != keep ) {
      if ( BB_SET_MemberP(visited,bb) ) {
	for ( lst = BB_succs(bb); lst != NULL; lst = lst_next ) {
	  lst_next = BBLIST_next(lst);
	  if (succ = BBLIST_item(lst))
	    Unlink_Pred_Succ(bb, succ);
	}

	for ( lst = BB_preds(bb); lst != NULL; lst = lst_next ) {
	  lst_next = BBLIST_next(lst);
	  if (pred = BBLIST_item(lst))
	    Unlink_Pred_Succ(pred, bb);
	}
	BB_Remove_All(bb);
	Remove_BB(bb);
      }
    }
  }

  if ( inp_visited == NULL )
    MEM_POOL_Pop(&MEM_local_pool);

}


/* -----------------------------------------------------------------------
 * void CG_LOOP_Remove_Prolog_OPs(BB *head)
 *
 *  Remove all the prolog BB's except CG_LOOP_prolog_start, 
 *  Update the BB chain and pred/succ lists to reflect this.
 *  Reset CG_LOOP_prolog to CG_LOOP_prolog_start. Where <head> is the
 *  first DoBodyBB(1) in the loop.
 *
 * -----------------------------------------------------------------------
 */
void CG_LOOP_Remove_Prolog_OPs(BB *head)
{
  BB *succ;
  BBLIST *lst;
  BBLIST *lst_next;
  float prob;

  /* Grab probability for CG_LOOP_prolog=>head edge before the edge
   * gets removed.
   */
  lst = BB_Find_Succ(CG_LOOP_prolog, head);
  if (lst) {
    prob = BBLIST_prob(lst);
  } else {
    Is_True(FALSE, ("couldn't find BB:%d => BB:%d edge",
		      BB_id(CG_LOOP_prolog), BB_id(head)));
    prob = 0;
  }

  remove_prolog_or_epilog_bbs(CG_LOOP_prolog_start, 
			      CG_LOOP_prolog, 
			      CG_LOOP_prolog_start,
			      NULL);

  for ( lst = BB_succs(CG_LOOP_prolog_start); lst != NULL; lst = lst_next ) {
    lst_next = BBLIST_next(lst);
    if (succ = BBLIST_item(lst))
      Unlink_Pred_Succ(CG_LOOP_prolog_start, succ);
  }

  Unlink_Pred_Succ(CG_LOOP_prolog, head);
  Link_Pred_Succ_with_Prob(CG_LOOP_prolog_start, head, prob);
  BB_next(CG_LOOP_prolog_start) = head;
  BB_prev(head) = CG_LOOP_prolog_start;

  CG_LOOP_prolog = CG_LOOP_prolog_start;

}

/* -----------------------------------------------------------------------
 * void CG_LOOP_Remove_Epilog_OPs(BB *tail)
 *
 *  Remove all the epilog BB's except CG_LOOP_epilog_end, 
 *  Update the BB chain and pred/succ lists to reflect this.
 *  Reset CG_LOOP_epilog to CG_LOOP_epilog_end. Where <tail> is the 
 *  last DoBodyBB(Original_DB_count) in the loop.
 *
 * -----------------------------------------------------------------------
 */
void CG_LOOP_Remove_Epilog_OPs(BB *tail)
{
  BB *pred;
  BBLIST *lst;
  BBLIST *lst_next;
  float prob;

  BB_Remove_All(CG_LOOP_epilog_end);

  /* Grab probability for tail=>CG_LOOP_epilog edge before the edge
   * gets removed.
   */
  lst = BB_Find_Succ(tail, CG_LOOP_epilog);
  if (lst) {
    prob = BBLIST_prob(lst);
  } else {
    Is_True(FALSE, ("couldn't find BB:%d => BB:%d edge",
		      BB_id(tail), BB_id(CG_LOOP_epilog)));
    prob = 0;
  }

  remove_prolog_or_epilog_bbs(CG_LOOP_epilog, 
			      CG_LOOP_epilog_end, 
			      CG_LOOP_epilog_end,
			      NULL);

  for ( lst = BB_preds(CG_LOOP_epilog_end); lst != NULL; lst = lst_next ) {
    lst_next = BBLIST_next(lst);
    if (pred = BBLIST_item(lst))
      Unlink_Pred_Succ(pred, CG_LOOP_epilog_end);
  }

  Unlink_Pred_Succ(tail, CG_LOOP_epilog);
  Link_Pred_Succ_with_Prob(tail, CG_LOOP_epilog_end, prob);
  BB_next(tail) = CG_LOOP_epilog_end;
  BB_prev(CG_LOOP_epilog_end) = tail;
  CG_LOOP_epilog = CG_LOOP_epilog_end;

}


void CG_LOOP_Trace_Loop(LOOP_DESCR *loop, const char *fmt, ...)
/* -----------------------------------------------------------------------
 * See "cg_loop.h" for interface.
 * -----------------------------------------------------------------------
 */
{
  BB *bb;
  va_list args;

  fputs(DBar, TFile);
  va_start(args, fmt);
  vfprintf(TFile, fmt, args);
  va_end(args);
  fputs("\n", TFile);

  CG_LOOP_Trace_Prolog();
  fputs(SBar, TFile);
  CG_LOOP_Backpatch_Trace(CG_LOOP_prolog, NULL);
  fputs("Loop body:\n", TFile);

  FOR_ALL_BB_SET_members(LOOP_DESCR_bbset(loop), bb) Print_BB(bb);

  fputs(SBar, TFile);
  if (CG_LOOP_epilog == NULL) {
    fputs("<epilog> no loop epilog\n", TFile);
  } else {
    CG_LOOP_Backpatch_Trace(CG_LOOP_epilog, NULL);
    fputs(SBar, TFile);
    CG_LOOP_Trace_Epilog();
  }

  fputs(DBar, TFile);
}


static BOOL any_bbs_in(BBLIST *list, BB_SET *bbs)
/* -----------------------------------------------------------------------
 * Return TRUE iff any BBs in <list> are members of <bbs>.
 * -----------------------------------------------------------------------
 */
{
  BBLIST *item;
  FOR_ALL_BBLIST_ITEMS(list, item)
    if (BB_SET_MemberP(bbs, BBLIST_item(item))) return TRUE;
  return FALSE;
}


static BOOL all_bbs_in(BBLIST *list, BB_SET *bbs)
/* -----------------------------------------------------------------------
 * Return TRUE iff all BBs in <list> are members of <bbs>.
 * -----------------------------------------------------------------------
 */
{
  BBLIST *item;
  FOR_ALL_BBLIST_ITEMS(list, item)
    if (!BB_SET_MemberP(bbs, BBLIST_item(item))) return FALSE;
  return TRUE;
}


/* -----------------------------------------------------------------------
 *  BOOL CG_LOOP::Attach_Prolog_And_Epilog(LOOP_DESCR *loop)
 *    Create and attach CG_LOOP_prolog/epilog BBs to <loop>'s head
 *    and tail.  It returns TRUE unless <loop> is of some weird
 *    form that it can't handle (e.g. multiple tails).
 * -----------------------------------------------------------------------
 */
void CG_LOOP::Attach_Prolog_And_Epilog(LOOP_DESCR *loop)
{
  BB *loop_head = LOOP_DESCR_loophead(loop);
  BB *loop_tail = LOOP_DESCR_Find_Unique_Tail(loop), *bb;
#ifdef TARG_ST
  /* (cbr) shut up -fwritable-strings */
  const 
#endif
  char *trace_pfx = Get_Trace(TP_CGLOOP, 1) ? "<cgprep> " : NULL;
  BBLIST *preds;
  BOOL freqs = FREQ_Frequencies_Computed();

  /* Disallow loops without (exactly) one loop tail.
   *
   * TODO: Allow multiple tail loops by adding new BB which branches
   *	   to <loop_head> (and making internal branches retarget to
   *	   this new BB).  Or get rid of algorithms requiring a unique
   *	   tail.  I think this is pretty easy for most of them.
   */
  if (loop_tail == NULL)
    return;

  // Identify the BB containing the trip count TN.
  if (CG_LOOP_Trip_Count(loop) != NULL) {
    preds = BB_preds(loop_head);
    while (preds && BB_SET_MemberP(LOOP_DESCR_bbset(loop), BBLIST_item(preds)))
      preds = BBLIST_next(preds);
    if (preds) 
      trip_count_bb = BBLIST_item(preds);
    Is_True(trip_count_bb, 
	    ("unable to identify trip count bb."));
  }

  /* create backpatches */
  prolog_backpatches = epilog_backpatches = NULL;

  CG_LOOP_prolog = CG_LOOP_prolog_start = NULL;

  Set_has_prolog();
  CG_LOOP_prolog = CG_LOOP_Gen_And_Prepend_To_Prolog(loop_head, loop);
  CG_LOOP_prolog_start = CG_LOOP_prolog;
  GRA_LIVE_Compute_Liveness_For_BB(CG_LOOP_prolog);
  if (trace_pfx)
    fprintf(TFile, "%screating loop prolog BB:%d\n", trace_pfx,
	    BB_id(CG_LOOP_prolog));

  /* See whether all loop exits go to the same BB.  If so, we can
   * either use that BB as an epilog, or create a new epilog just
   * before that BB.  Otherwise we can't insert a single epilog BB
   * without adding extra branches, so we'll return with a NULL
   * epilog to disallow transformations requiring a common epilog.
   */
  CG_LOOP_epilog = CG_LOOP_epilog_end = NULL;
  FOR_ALL_BB_SET_members(LOOP_DESCR_bbset(loop), bb) {
    BBLIST *succs;
    FOR_ALL_BB_SUCCS(bb, succs) {
      BB *succ = BBLIST_item(succs);
      if (!BB_SET_MemberP(LOOP_DESCR_bbset(loop), succ)) {
	if (CG_LOOP_epilog && succ != CG_LOOP_epilog) {
	  CG_LOOP_epilog  = NULL;
	  if (trace_pfx)
	    fprintf(TFile, "%scan't find or create suitable loop epilog; "
		    "disallows some optimization\n", trace_pfx);
	  return;
	}
	CG_LOOP_epilog = succ;
      }
    }
  }
  Set_has_epilog();
  if (CG_LOOP_epilog == NULL) {
    /*
     * We have an infinite loop, but we'll create and insert an epilog
     * at the end of BB chain to allow maximum optimization of the loop
     * body.  The epilog will later be removed since it's unreachable.
     *
     * TODO: (Compspeed) Avoid walking BB chain to find last one by
     *       tracking REGION_Last_BB?  Can we just insert it before
     *	     REGION_First_BB?
     */
    BB *last_bb = REGION_First_BB;
    while (BB_next(last_bb)) last_bb = BB_next(last_bb);
    CG_LOOP_epilog = Gen_And_Insert_BB_After(last_bb);
    if (trace_pfx)
      fprintf(TFile, "%screating loop epilog BB:%d\n", trace_pfx,
	      BB_id(CG_LOOP_epilog));
    BB_rid(CG_LOOP_epilog) = BB_rid(loop_head);
    if (!CG_localize_tns) Set_BB_gra_spill(CG_LOOP_epilog);
    if (BB_freq_fb_based(CG_LOOP_prolog)) Set_BB_freq_fb_based(CG_LOOP_epilog);
    GRA_LIVE_Compute_Liveness_For_BB(CG_LOOP_epilog);
  } else {
    BB *next = CG_LOOP_epilog;
    BB *ftp = BB_Fall_Thru_Predecessor(next);
    BBKIND ftp_kind = ftp ? BB_kind(ftp) : BBKIND_UNKNOWN;
    LOOP_DESCR *enclosing = LOOP_DESCR_Next_Enclosing_Loop(loop);
    CG_LOOP_epilog = Gen_And_Insert_BB_Before(next);
    if (trace_pfx)
      fprintf(TFile, "%screating loop epilog BB:%d\n", trace_pfx,
	      BB_id(CG_LOOP_epilog));
    BB_rid(CG_LOOP_epilog) = BB_rid(loop_head);
    if (!CG_localize_tns) Set_BB_gra_spill(CG_LOOP_epilog);
    if (BB_freq_fb_based(CG_LOOP_prolog)) Set_BB_freq_fb_based(CG_LOOP_epilog);
    if (ftp && !BB_SET_MemberP(LOOP_DESCR_bbset(loop), ftp)) {
      /*
       * Since <ftp> isn't in loop, we don't want it to fall through
       * to CG_LOOP_epilog.  It was falling through to <next>, so
       * we'll make it either branch to <next> or fall through to a
       * new BB that branches to <next>.
       */
      if (ftp_kind == BBKIND_GOTO) {
	BBLIST *bbl = BB_Find_Succ(ftp, next);
	Add_Goto(ftp, next);
	BBLIST_prob(bbl) = 1.0F;	/* otherwise is incremented to 2.0 */
      } else {
	BB *new_bb = Gen_And_Insert_BB_After(ftp);
	BB_rid(new_bb) = BB_rid(loop_head);
	if (BB_freq_fb_based(CG_LOOP_epilog)) Set_BB_freq_fb_based(new_bb);
	Change_Succ(ftp, next, new_bb);
	Add_Goto(new_bb, next);
	GRA_LIVE_Compute_Liveness_For_BB(new_bb);
      }
    }
    retarget_loop_exits(loop, next, CG_LOOP_epilog);
    Link_Pred_Succ_with_Prob(CG_LOOP_epilog, next, 1.0);
    if (freqs || BB_freq_fb_based(next))
      BB_freq(next) += BB_freq(CG_LOOP_epilog);
    GRA_LIVE_Compute_Liveness_For_BB(CG_LOOP_epilog);

    /* Add CG_LOOP_epilog to appropriate LOOP_DESCRs, if any.
     * It can belong only to loops enclosing this one, so
     * we don't bother checking any others.  */
    if (enclosing &&
	all_bbs_in(BB_preds(CG_LOOP_epilog), LOOP_DESCR_bbset(enclosing)))
      LOOP_DESCR_Add_BB(enclosing, CG_LOOP_epilog);
  }
  CG_LOOP_epilog_end = CG_LOOP_epilog;

  if (CG_warn_bad_freqs && CG_LOOP_epilog &&
      (freqs || BB_freq_fb_based(CG_LOOP_prolog)) &&
      !FREQ_Match(BB_freq(CG_LOOP_prolog), BB_freq(CG_LOOP_epilog)))
    DevWarn("CG_LOOP prolog (BB:%d) and epilog (BB:%d) frequencies disagree",
	    BB_id(CG_LOOP_prolog), BB_id(CG_LOOP_epilog));
}


/* =======================================================================
 *
 * CG_LOOP_DEF::Get(TN *tn)
 *   Returns the first def-op for 'tn' in the loop body.
 *   Returns NULL if the TN is not defined.
 *
 * =======================================================================
 */
OP *CG_LOOP_DEF::Get(TN *tn)
{
  if (TN_is_register(tn) && !TN_is_const_reg(tn))
    return (OP*) TN_MAP_Get(tn_map, tn); 
  else
    return NULL;
}

/* =======================================================================
 *
 * CG_LOOP_DEF::Is_invariant(TN *tn)
 *  Returns TRUE if TN is a loop invariant.
 *
 * =======================================================================
 */
BOOL CG_LOOP_DEF::Is_invariant(TN *tn)
{
  return Get(tn) == NULL;
}

/* =======================================================================
 *
 * CG_LOOP_DEF::CG_LOOP_DEF(BB *)
 *   Allocate TN_MAP and setup the TN -> first_def_op mapping.
 *
 * =======================================================================
 */
CG_LOOP_DEF::CG_LOOP_DEF(BB *body)
{
  tn_map = TN_MAP_Create();
  OP *op;
  FOR_ALL_BB_OPs(body, op) {

    for (INT i = 0; i < OP_results(op); i++) {
      TN *res = OP_result(op,i);
      if (TN_is_register(res) && 
	  !TN_is_const_reg(res) &&
	  !Get(res))
	TN_MAP_Set(tn_map, res, op);
    }
  }
}

/* =======================================================================
 *
 * CG_LOOP_DEF::~CG_LOOP_DEF() 
 *   Release TN_MAP.
 *
 * =======================================================================
 */
CG_LOOP_DEF::~CG_LOOP_DEF()
{
  TN_MAP_Delete(tn_map);
}


/* =======================================================================
 *
 *  CG_LOOP::Build_CG_LOOP_Info
 *
 *    Attach the loop info to each OP in the loop.
 *    Operand TNs that are not invariants and used before definition
 *    in the loop body are entered to the prolog backpatch.
 *    Result TNs that are liveout are entered to the epilog backpatch
 *
 * =======================================================================
 */
void CG_LOOP::Build_CG_LOOP_Info()
{
  Is_True(Single_BB(), ("LOOP has multiple BB."));

  // Build_CG_LOOP_Info might be called multiple times.
  if (_CG_LOOP_info_map) {
    OP_MAP_Delete(_CG_LOOP_info_map);
    _CG_LOOP_info_map = NULL;
  }

  Recompute_Liveness();

  _CG_LOOP_info_map = OP_MAP_Create(/* body */);
  op_map = _CG_LOOP_info_map;

  BB *body = Loop_header();
  BB *prolog = Prolog_end();
  BB *epilog = Epilog_start();

  CG_LOOP_DEF tn_def(body);

  /* Create _CG_LOOP_info_map entries.  For exposed uses, fill in
   * omega=1 and create a prolog backpatch.  For live-out defs, create
   * an epilog backpatch.
   */
  OP *op;
  FOR_ALL_BB_OPs(body, op) {
    CG_LOOP_Init_Op(op);
    for (INT i = 0; i < OP_results(op); i++) {
      TN *res = OP_result(op,i);
      if (TN_is_register(res) &&
	  !TN_is_const_reg(res)) {
	if (GTN_SET_MemberP(BB_live_in(epilog), res)) {
	  Is_True(TN_is_global_reg(res), ("TN in GTN_SET not a global_reg."));

	  if (!TN_is_dedicated(res))
	    CG_LOOP_Backpatch_Add(epilog, res, res, 0);

	  if (GTN_SET_MemberP(BB_live_in(body), res))
	    if (!TN_is_dedicated(res))
	      CG_LOOP_Backpatch_Add(prolog, res, res, 1);
	}
      }
    }
    for (INT opnd = 0; opnd < OP_opnds(op); opnd++) {
      TN *tn = OP_opnd(op,opnd);
      if (TN_is_register(tn) &&
	  !TN_is_const_reg(tn)) {
	OP *def_op = tn_def.Get(tn);
	// TN is not an invariant and
	// TN is not defined before this OP.
	if (def_op && 
	    !OP_Precedes(def_op, op)) {
	  if ( !CG_LOOP_Backpatch_Find_Non_Body_TN(prolog, tn, 1) )
	    if (!TN_is_dedicated(tn))
	      CG_LOOP_Backpatch_Add(prolog, tn, tn, 1);
	  Set_OP_omega(op, opnd, 1);
	}
      }
    }
  }

#ifdef Is_True_On
  Verify();
#endif
}


/* =======================================================================
 *
 *  CG_LOOP::CG_LOOP
 *
 *    Setup prolog/epilog of the loop.
 *
 * =======================================================================
 */
CG_LOOP::CG_LOOP(LOOP_DESCR *loop)
{
  Current_CG_LOOP = this;

  BB *body = LOOP_DESCR_loophead(loop);

  unroll_fully = FALSE;
  unroll_factor = -1; 
  flags = CG_LOOP_NONE;
  trip_count_bb = NULL;
  prolog_start = prolog_end = NULL;
  epilog_start = epilog_end = NULL;

#ifdef TARG_ST
  this->even_factor = FALSE;
  this->remainder_after = FALSE;
  this->unroll_sched_est = -1;
  this->special_streams = 0;
  this->stream_tn[0] = NULL;
  this->stream_op[0] = NULL;
#endif

  Attach_Prolog_And_Epilog(loop);

  prolog_start = CG_LOOP_prolog_start;
  prolog_end = CG_LOOP_prolog;
  epilog_start = CG_LOOP_epilog;
  epilog_end = CG_LOOP_epilog_end;

  /* info_map */
  op_map = NULL;
  _CG_LOOP_info_map = NULL;

  prolog_backpatches = epilog_backpatches = NULL;

  // copy to CG_LOOP structure
  this->loop = loop;
  this->unroll_remainder_bb = NULL;
}



/* =======================================================================
 *
 *  CG_LOOP::CG_LOOP
 *
 * =======================================================================
 */
CG_LOOP::~CG_LOOP()
{
  if (_CG_LOOP_info_map) {
    OP_MAP_Delete(_CG_LOOP_info_map);
    _CG_LOOP_info_map = NULL;
  }
  prolog_backpatches = epilog_backpatches = NULL;
  Current_CG_LOOP = NULL;
}


/* =======================================================================
 *
 *  CG_LOOP::Verify
 *
 * =======================================================================
 */
void CG_LOOP::Verify()
{
  BB *body = Loop_header();
  if (_CG_LOOP_info_map) {
    OP *op;
    FOR_ALL_BB_OPs(body, op) {
      // Verify TN omega
      //    
      for (INT opnd = 0; opnd < OP_opnds(op); opnd++) {
	TN *tn = OP_opnd(op,opnd);
	if (TN_is_dedicated(tn))
	  FmtAssert(OP_omega(op, opnd) <= 1,
		    ("CG_LOOP: Dedicated TN cannot have omega > 1."));
      }
    }
  }
}


/* =======================================================================
 *
 *  Recompute all the live info for the blocks between and including the
 *  loop head and tail.   Include the trip-count expressions, otherwise
 *  it will be deleted after GRA live.
 *
 * =======================================================================
 */
void
CG_LOOP::Recompute_Liveness()
{
  BB *entry = Trip_count_bb() ? Trip_count_bb() : Prolog_start();
  BB_REGION region(Malloc_Mem_Pool);
  region.entries.push_back(entry);
  BBLIST *succs;
  FOR_ALL_BB_SUCCS(Epilog_end(), succs) {
    BB *succ = BBLIST_item(succs);
    region.exits.push_back(succ);
  }

  // Promote to omega TN to GTN
  if (_CG_LOOP_info_map) {
    BB *body = Loop_header();
    OP *op;
    FOR_ALL_BB_OPs(body, op) {
      for (INT opnd = 0; opnd < OP_opnds(op); opnd++) {
	TN *tn = OP_opnd(op,opnd);
	if (Is_CG_LOOP_Op(op) &&
	    OP_omega(op, opnd) != 0 &&
	    !TN_is_dedicated(tn) &&
	    !TN_is_global_reg(tn)) 
	  GTN_UNIVERSE_Add_TN(tn);
      }
    }
  }

  GRA_LIVE_Init_Loop(CG_LOOP_prolog, 
		     _CG_LOOP_info_map ? Loop_header() : NULL,
		     CG_LOOP_epilog,
		     prolog_backpatches, 
		     epilog_backpatches);

  BB_REGION_Recompute_Global_Live_Info(region, TRUE /* recompute local info */);

  GRA_LIVE_Fini_Loop();
}


// old interface
void
CG_LOOP_Recompute_Liveness(LOOP_DESCR *loop)
{
  Is_True(loop == Current_CG_LOOP->Loop(),
	  ("loop != Current_CG_LOOP"));
  Current_CG_LOOP->Recompute_Liveness();
}


TN *CG_LOOP_Backpatch_Find_Body_TN(BB *bb, TN *tn, UINT8 *omptr)
/* -----------------------------------------------------------------------
 * See "cg_loop.h" for interface.
 * -----------------------------------------------------------------------
 */
{
  CG_LOOP_BACKPATCH *bp = CG_LOOP_Backpatch_First(bb, NULL);
  while (bp && CG_LOOP_BACKPATCH_non_body_tn(bp) != tn) 
    bp = CG_LOOP_Backpatch_Next(bp);
  if (bp && omptr) *omptr = CG_LOOP_BACKPATCH_omega(bp);
  return bp ? CG_LOOP_BACKPATCH_body_tn(bp) : NULL;
}


TN *CG_LOOP_Backpatch_Find_Non_Body_TN(BB *bb, TN *tn, UINT8 om)
/* -----------------------------------------------------------------------
 * See "cg_loop.h" for interface.
 * -----------------------------------------------------------------------
 */
{
  CG_LOOP_BACKPATCH *bp = CG_LOOP_Backpatch_First(bb, tn);
  while (bp && CG_LOOP_BACKPATCH_omega(bp) != om) 
    bp = CG_LOOP_Backpatch_Next(bp);
  return bp ? CG_LOOP_BACKPATCH_non_body_tn(bp) : NULL;
}

CG_LOOP_BACKPATCH *CG_LOOP_Backpatch_Add(BB *bb, TN *non_body_tn,
					 TN *body_tn, UINT8 omega)
/* -----------------------------------------------------------------------
 * See "cg_loop.h" for interface.
 * -----------------------------------------------------------------------
 */
{
  BOOL isprolog = (bb == CG_LOOP_prolog);
  CG_LOOP_BACKPATCH *bps = isprolog ? prolog_backpatches : epilog_backpatches;
  CG_LOOP_BACKPATCH *bp;

  Is_True(bb == CG_LOOP_prolog || bb == CG_LOOP_epilog,
	    ("bb not CG_LOOP_prolog or CG_LOOP_epilog"));

  /* Special case: see PV 389526. */
  if (!isprolog && TN_is_zero_reg(non_body_tn)) return NULL;

  for (bp = bps; bp; bp = CG_LOOP_Backpatch_Next(bp)) {
    if (isprolog && bp->body_tn == body_tn && bp->omega == omega) {
      FmtAssert(bp->non_body_tn == non_body_tn,
		("conflicting prolog backpatch for TN%d[%d]",
		 TN_number(body_tn), omega));
      return bp;
    } else if (!isprolog && bp->non_body_tn == non_body_tn) {
      FmtAssert(bp->body_tn == body_tn && bp->omega == omega,
		("conflicting epilog backpatch for TN%d",
		 TN_number(non_body_tn)));
      return bp;
    }
  }

  bp = TYPE_MEM_POOL_ALLOC(CG_LOOP_BACKPATCH, &MEM_phase_nz_pool);
  bp->non_body_tn = non_body_tn;
  bp->body_tn = body_tn;
  bp->omega = omega;
  bp->next = bps;
  if (isprolog)
    prolog_backpatches = bp;
  else
    epilog_backpatches = bp;

  return bp;
}


CG_LOOP_BACKPATCH *CG_LOOP_Backpatch_First(BB *bb, TN *body_tn)
/* -----------------------------------------------------------------------
 * See "cg_loop.h" for interface.
 * -----------------------------------------------------------------------
 */
{
  CG_LOOP_BACKPATCH *bp;
  Is_True(bb == CG_LOOP_prolog || bb == CG_LOOP_epilog,
	  ("bb not CG_LOOP_prolog or CG_LOOP_epilog"));
  bp = (bb == CG_LOOP_prolog) ? prolog_backpatches : epilog_backpatches;
  while (bp && body_tn && bp->body_tn != body_tn) 
      bp = bp->next;
  if (bp == NULL)
    return bp;
  return body_tn ? _CG_LOOP_BP_limit_iter(bp) : bp;
}


CG_LOOP_BACKPATCH *CG_LOOP_Backpatch_Next(CG_LOOP_BACKPATCH *bp)
/* -----------------------------------------------------------------------
 * See "cg_loop.h" for interface.
 * -----------------------------------------------------------------------
 */
{
  if (_CG_LOOP_BP_iter_limited(bp)) {
    TN *body_tn;
    bp = _CG_LOOP_BP_actual_ptr(bp);
    body_tn = bp->body_tn;
    do {
      bp = bp->next;
    } while (bp && bp->body_tn != body_tn);
    if (bp == NULL)
      return bp;
    return _CG_LOOP_BP_limit_iter(bp);
  } else {
    return bp->next;
  }
}


void CG_LOOP_Backpatch_Delete(BB *bb, CG_LOOP_BACKPATCH *the_bp)
/* -----------------------------------------------------------------------
 * See "cg_loop.h" for interface.
 * -----------------------------------------------------------------------
 */
{
  CG_LOOP_BACKPATCH *bp;
  CG_LOOP_BACKPATCH *prev_bp;
  CG_LOOP_BACKPATCH **bpp;

  Is_True(bb == CG_LOOP_prolog || bb == CG_LOOP_epilog,
	  ("bb not CG_LOOP_prolog or CG_LOOP_epilog"));

  the_bp = _CG_LOOP_BP_actual_ptr(the_bp);
  bpp = (bb == CG_LOOP_prolog) ? &prolog_backpatches : &epilog_backpatches;

  prev_bp = NULL;
  for (bp = *bpp; bp; bp = bp->next) {
    if (bp == the_bp) break;
    prev_bp = bp;
  }

  if (!bp) return;

  if (prev_bp) {
    prev_bp->next = bp->next;
  } else {
    *bpp = bp->next;
  }
}



void CG_LOOP_Backpatch_Replace_Non_Body_TN(BB *bb, TN *tn, TN *newtn)
/* -----------------------------------------------------------------------
 * See "cg_loop.h" for interface.
 * -----------------------------------------------------------------------
 */
{
  CG_LOOP_BACKPATCH *bp;

  Is_True(bb == CG_LOOP_prolog || bb == CG_LOOP_epilog,
	  ("bb not CG_LOOP_prolog or CG_LOOP_epilog"));

  bp = (bb == CG_LOOP_prolog) ? prolog_backpatches : epilog_backpatches;
  
  for ( ; bp ; bp = bp->next )
    if ( bp->non_body_tn == tn ) bp->non_body_tn = newtn;

  return;
}

void CG_LOOP_Backpatch_Replace_Body_TN(BB *bb, TN *tn, TN *newtn,
				       INT16 om_adj)
/* -----------------------------------------------------------------------
 * See "cg_loop.h" for interface.
 * -----------------------------------------------------------------------
 */
{
  CG_LOOP_BACKPATCH *bp;

  Is_True(bb == CG_LOOP_prolog || bb == CG_LOOP_epilog,
	  ("bb not CG_LOOP_prolog or CG_LOOP_epilog"));

  bp = (bb == CG_LOOP_prolog) ? prolog_backpatches : epilog_backpatches;
  
  for ( ; bp ; bp = bp->next ) 
    if ( bp->body_tn == tn ) {
      bp->body_tn = newtn;
      bp->omega+= om_adj;
    }

  return;

}


void CG_LOOP_Backpatch_Trace(BB *bb, CG_LOOP_BACKPATCH *bp)
/* -----------------------------------------------------------------------
 * See "cg_loop.h" for interface.
 * -----------------------------------------------------------------------
 */
{
  Is_True(bb == NULL || bb == CG_LOOP_prolog || bb == CG_LOOP_epilog,
	  ("bb not NULL, CG_LOOP_prolog or CG_LOOP_epilog"));

  bp = _CG_LOOP_BP_actual_ptr(bp);
  if (bp) {
    if (bb == CG_LOOP_epilog) {
      fprintf(TFile, "<bp>   TN%d <- TN%d[%d]\n", TN_number(bp->non_body_tn),
	      TN_number(bp->body_tn), bp->omega);
    } else {
      fprintf(TFile, "<bp>   TN%d[%d] %s TN%d\n", TN_number(bp->body_tn),
	      bp->omega, bb == CG_LOOP_prolog ? "<-" : "",
	      TN_number(bp->non_body_tn));
    }
  } else if (bb) {
    BOOL prolog = bb == CG_LOOP_prolog;
    fprintf(TFile, "<bp> %slog backpatches:\n", prolog ? "pro" : "epi");
    for (bp = prolog ? prolog_backpatches : epilog_backpatches; bp;
	 bp = bp->next)
      CG_LOOP_Backpatch_Trace(bb, bp);
    fprintf(TFile,"\n");
  } else {
    CG_LOOP_Backpatch_Trace(CG_LOOP_prolog, NULL);
    CG_LOOP_Backpatch_Trace(CG_LOOP_epilog, NULL);
  }
}


/* ====================================================================
 *
 * CG_LOOP_Remove_Notations
 *
 * When CG_LOOP_Remove_Notations has finished, all omega and backpatch
 * notation can be safely discarded without changing the behavior of
 * the loop.  In particular:
 * - All prolog backpatches will have the form  TN200[1] <-- TN200;
 * - All epilog backpatches will have the form  TN200 <-- TN200[0]; and
 * - Non-zero omegas, namely omega == 1, will only occur in loop
 *   operands referencing TNs whose definitions do not precede the use.
 *
 * CG_LOOP_Remove_Notations accomplishes this goal by identifying TNs
 * appearing as body TNs in prolog and epilog backpatches, or occurring
 * as operands with omega > 1 (or with omega == 1 but succeeding its
 * first definition).  Then new TNs and TN copies are inserted to store
 * past values of these identified TNs.  Finally, these new TNs are
 * substituted for old TNs and omegas appearing in loop body operands
 * and prolog and epilog backpatches.
 *
 * In order to keep the number of new TNs and TN copies as low as
 * possible, the TN copies are inserted immediately before the first
 * definition of the original TN.  (There may be multiple definitions.)
 * For example:
 *
 *                             BEFORE                     AFTER
 *
 * Prolog:                                           TN100 <-- TN201
 *                                                   TN101 <-- TN202
 *                                                   TN102 <-- TN203
 *                                                   TN103 <-- TN204
 *
 * Prolog backpatches:  TN100[1] <-- TN201           TN100[1] <-- TN100
 *                      TN100[2] <-- TN202           TN101[1] <-- TN101
 *                      TN100[3] <-- TN203           TN102[1] <-- TN102
 *                      TN100[4] <-- TN204           TN103[1] <-- TN103
 *
 * In the loop body:    ..... <-- TN100[1]           ..... <-- TN100[1]
 *                      ..... <-- TN100[2]           ..... <-- TN101[1]
 *                      ..... <-- TN100[3]           ..... <-- TN102[1]
 *                      ..... <-- TN100[4]           ..... <-- TN103[1]
 *
 *                                                   TN104 <-- TN103[1]
 *                                                   TN103 <-- TN102[1]
 *                                                   TN102 <-- TN101[1]
 *                                                   TN101 <-- TN100[1]
 *                      TN100 <-- ........           TN100 <-- .....
 *
 *                      ..... <-- TN100[0]           ..... <-- TN100[0]
 *                      ..... <-- TN100[1]           ..... <-- TN101[0]
 *                      ..... <-- TN100[2]           ..... <-- TN102[0]
 *                      ..... <-- TN100[3]           ..... <-- TN103[0]
 *
 * Epilog backpatches:  TN300 <-- TN100[0]           TN100 <-- TN100[0]
 *                      TN301 <-- TN100[1]           TN101 <-- TN101[0]
 *                      TN302 <-- TN100[2]           TN102 <-- TN102[0]
 *                      TN303 <-- TN100[3]           TN103 <-- TN103[0]
 *
 * Epilog:                                           TN300 <-- TN100
 *                                                   TN301 <-- TN101
 *                                                   TN302 <-- TN102
 *                                                   TN303 <-- TN103
 *
 * The first definition of a TN is handled uniquely.  If the first
 * definition of TN100 has TN100[1] as an operand, the operand TN100[1]
 * is left alone (and not replaced by TN101[0]).  However, TN100[2],
 * TN100[3], TN100[4] would still be replaced by TN102[0], TN103[0],
 * TN104[0] if they appeared as operands in the first definition of
 * TN100.
 *
 * In order to minimize the number of TN copies inserted into the
 * prolog and epilog, the implementation attempts to reuse the non-body
 * TNs from the prolog and epilog backpatches.  (In the above example,
 * TN300, ..., TN303 would replace TN101, ..., TN104 if the former TNs
 * do not already appear within the loop.  The secondary choices would
 * be TN201,..., TN204, with new TNs generated as a last resort.)
 *
 *
 * Several TN_MAPs are used to store information during the work of
 * Remove_Notations:
 *
 * tn_def_map stores the first definition of each TN within the loop
 * body.
 *
 * tn_copies_map stores the number of new TNs and TN copies that will
 * be needed for each non-global TN within the loop body.  In the
 * example above, tn_copies_map of TN100 is 3.
 *
 * new_tn_map stores all the new TNs after they are generated.  In the
 * example above, new_tn_map of TN100 is the TN* array new_tn_array,
 * where new_tn_array[0] == TN101, new_tn_array[1] == TN102, and
 * new_tn_array[2] == TN103.  After tn_copies_map is initialized,
 * if copies = hTN_MAP32_Get(tn_copies_map, tn) is positive, then
 * hTN_MAP_Get(new_tn_map, tn) is an array of TN* with copies - 1
 * entries.
 *
 * need_copies is a TN_LIST of all TNs with tn_copies_map[] > 0.
 *
 *
 * CG_LOOP_Remove_Notations uses two helper procedures to complete its
 * work: Count_Copies_Needed and Generate_Copy_TNs
 * These helper procedures are also invoked during loop unrolling by
 * Unroll_Make_Remainder_Loop.
 *
 * TARG_ST: [CG]
 * Unroll_Make_Remainder_Loop: this function creates an
 * REMAINDERINFO annotation attached to each BB of the remainder
 * loop. The REMAINDER_head_bb() gives the head BB of the unrolled
 * loop. Each OP as also the OP_unrolling(), OP_orig_idx(), and
 * OP_unrolled_bb() set.
 * If a BB as a REMAINDERINFO it is part of a remainder loop.
 * If BB_unrollings() > 0, the remainder block is an unrolled block.
 * If BB_unrollings() == 0, the block is part of a multi bb remainder.
 * If OP_unrolled_bb() is set the OP is an instance of op in an
 * unrolled BB remainder or a multi BB reminder and its OP_unrolling()
 * and OP_orig_idx() are set.
 *
 * Count_Copies_Needed examines prolog and epilog backpatches and loop
 * body operands for omega values that will require additional TNs and
 * TN copies to remove.  It initializes the values of tn_def_map and
 * tn_copies_map for the current loop, and it allocates, creates and
 * returns the TN_LIST need_copies.  Count_Copies_Needed also removes
 * unnecessary prolog backpatches.
 *
 * Generate_Copies_TN generates new TNs for new_tn_map.  It attempts to
 * reuse non-body TNs from epilog and prolog backpatches, in order to
 * minimize the number of copies that will need to be inserted.
 *
 * Remove_Notations_Without_Copies performs the code transformation for
 * the case in which no additional TNs are needed.  This only requires
 * updating the prolog and epilog backpatches, and possible inserting
 * TN copy OPs into the prolog and epilog.
 *
 * Remove_Notations_With_Copies performs the code transformation for
 * the general case, which requires the insertion of TN copy OPs into
 * the loop body, and the transformations if loop body operands and
 * prolog and epilog backpatches described above.
 *
 * ====================================================================
 */


static TN_LIST *Count_Copies_Needed(BB *body, hTN_MAP tn_def_map,
				    hTN_MAP32 tn_copies_map,
				    MEM_POOL *pool)
{
  TN_LIST *need_copies = NULL;

  // Examine each OP within the loop body, and initialize values
  // for the maps tn_def_map and tn_copies_map.

  for (OP *op = BB_first_op(body); op != NULL; op = OP_next(op)) {

    // Initialize tn_def_map to be the first definition OP for each TN.
    // Handle results before operands, because of the case
    //   TN100 <-- TN100[i]
    for (INT res = OP_results(op) - 1; res >= 0; --res) {
      TN *tn = OP_result(op, res);
      if (hTN_MAP_Get(tn_def_map, tn) == NULL)
	hTN_MAP_Set(tn_def_map, tn, (void *) op);
    }

    for (INT opnd = OP_opnds(op) - 1; opnd >= 0; --opnd) {
      TN *tn = OP_opnd(op, opnd);
      if ( ! TN_is_register(tn)) continue;
      INT copies = OP_omega(op, opnd);
      OP *tn_def_op = (OP *) hTN_MAP_Get(tn_def_map, tn);
      if (tn_def_op == NULL || (tn_def_op == op && copies == 1))
	--copies;
      if (copies > 0) {
	INT max_copies = hTN_MAP32_Get(tn_copies_map, tn);
	if (copies > max_copies) {
	  hTN_MAP32_Set(tn_copies_map, tn, copies);
	  if (max_copies == 0)
	    need_copies = TN_LIST_Push(tn, need_copies, pool);
	}
      }
    }
  }

  // Update tn_copies_map for all TNs appearing in loop epilog
  // backpatches

  CG_LOOP_BACKPATCH *bp;
  for (bp = CG_LOOP_Backpatch_First(CG_LOOP_epilog, NULL);
       bp; bp = CG_LOOP_Backpatch_Next(bp)) {
    INT copies = CG_LOOP_BACKPATCH_omega(bp);
    if (copies > 0) {
      TN *body_tn = CG_LOOP_BACKPATCH_body_tn(bp);
      INT max_copies = hTN_MAP32_Get(tn_copies_map, body_tn);
      if (copies > max_copies) {
	hTN_MAP32_Set(tn_copies_map, body_tn, copies);
	if (max_copies == 0)
	  need_copies = TN_LIST_Push(body_tn, need_copies, pool);
      }
    }
  }

  // Remove any unnecessary backpatches from the loop prolog.
  // These may appear as a result of the earlier elimination of loop TNs

  if (need_copies == NULL) {
    CG_LOOP_BACKPATCH *bp_next;
    for (bp = CG_LOOP_Backpatch_First(CG_LOOP_prolog, NULL);
	 bp; bp = bp_next) {
      bp_next = CG_LOOP_Backpatch_Next(bp);
      if (CG_LOOP_BACKPATCH_omega(bp) > 1)
	CG_LOOP_Backpatch_Delete(CG_LOOP_prolog, bp);
    }
  } else {
    CG_LOOP_BACKPATCH *bp_next;
    for (bp = CG_LOOP_Backpatch_First(CG_LOOP_prolog, NULL);
	 bp; bp = bp_next) {
      bp_next = CG_LOOP_Backpatch_Next(bp);
      TN *body_tn = CG_LOOP_BACKPATCH_body_tn(bp);
      INT omega = CG_LOOP_BACKPATCH_omega(bp);
      if (hTN_MAP32_Get(tn_copies_map, body_tn) < omega - 1)
	CG_LOOP_Backpatch_Delete(CG_LOOP_prolog, bp);
    }
  }

  return need_copies;
}


void Generate_Copy_TNs(BB *body, hTN_MAP32 tn_copies_map,
		       hTN_MAP new_tn_map, TN_LIST *need_copies,
		       MEM_POOL *pool)
{
  // Allocate memory for all replacement TNs, initialized to NULL

  TN_LIST *p;
  for (p = need_copies; p != NULL; p = TN_LIST_rest(p)) {
    TN *tn = TN_LIST_first(p);
    INT copies = hTN_MAP32_Get(tn_copies_map, tn);

    // Verify that Count_Copies_Needed worked
    Is_True(copies > 0, ("Generate_Copy_TNs: need_copies has TN%d with"
			 " copies == 0", TN_number(tn)));
    Is_True(hTN_MAP_Get(new_tn_map, tn) == NULL,
	    ("Generate_Copy_TNs: need_copies contains duplicates"));

    TN **new_tn_array = TYPE_MEM_POOL_ALLOC_N(TN *, pool, copies);
    hTN_MAP_Set(new_tn_map, tn, (void *) new_tn_array);
    for (INT i = copies - 1; i >= 0; --i)
      new_tn_array[i] = NULL;
  }

  // Try to use the same TN within the loop body as in the epilog so
  // that epilog backpatches will not require additional copy OPs, but
  // only if this TN is not already used within the loop

  CG_LOOP_BACKPATCH *bp;
  for (bp = CG_LOOP_Backpatch_First(CG_LOOP_epilog, NULL); bp;
       bp = CG_LOOP_Backpatch_Next(bp)) {
    INT copy = CG_LOOP_BACKPATCH_omega(bp);
    TN *non_body_tn = CG_LOOP_BACKPATCH_non_body_tn(bp);
    if (copy > 0 && ! TN_is_global_reg(non_body_tn)) {
      TN *body_tn = CG_LOOP_BACKPATCH_body_tn(bp);
      TN **new_tn_array = (TN **) hTN_MAP_Get(new_tn_map, body_tn);
      if (new_tn_array[copy - 1] == NULL) {
	GTN_UNIVERSE_Add_TN(non_body_tn);
	new_tn_array[copy - 1] = non_body_tn;
      }
    }
  }

  // Try to use the same TN within the loop body as in the prolog so
  // that prolog backpatches will not require additional copy OPs, but
  // make sure this TN is not already live within the loop, and don't
  // use the same one twice.

  for (bp = CG_LOOP_Backpatch_First(CG_LOOP_prolog, NULL); bp;
       bp = CG_LOOP_Backpatch_Next(bp)) {
    INT copy = CG_LOOP_BACKPATCH_omega(bp) - 1;
    TN *non_body_tn = CG_LOOP_BACKPATCH_non_body_tn(bp);
    if (copy > 0 && TN_is_register(non_body_tn)
	&& ! TN_is_dedicated(non_body_tn)
	&& ! TN_is_global_reg(non_body_tn)) {
      TN *body_tn = CG_LOOP_BACKPATCH_body_tn(bp);
      TN **new_tn_array = (TN **) hTN_MAP_Get(new_tn_map, body_tn);
      if (new_tn_array[copy - 1] == NULL) {
	GTN_UNIVERSE_Add_TN(non_body_tn);
	new_tn_array[copy - 1] = non_body_tn;
      }
    }
  }

  // Initialize all remaining replacement TNs to newly created TNs

  for (p = need_copies; p != NULL; p = TN_LIST_rest(p)) {
    TN *tn = TN_LIST_first(p);
    INT copies = hTN_MAP32_Get(tn_copies_map, tn);
    TN **new_tn_array = (TN **) hTN_MAP_Get(new_tn_map, tn);
    for (INT i = 0; i < copies; ++i)
      if (new_tn_array[i] == NULL)
	new_tn_array[i] = Build_TN_Like(tn);
  }
}


void Remove_Notations_Without_Copies(BB *head, BB *tail)
{
  // No new TNs are required, so only the prolog and epilog
  // backpatches may require updating

  // Update the epilog backpatches, inserting new copy OPs into the
  // epilog when necessary

  CG_LOOP_BACKPATCH *bp, *bp_next;
  for (bp = CG_LOOP_Backpatch_First(CG_LOOP_epilog, NULL); bp;
       bp = CG_LOOP_Backpatch_Next(bp)) {
    Is_True(CG_LOOP_BACKPATCH_omega(bp) == 0,
	    ("Remove_Notations_Without_Copies: omega > 0"));
    TN *non_body_tn = CG_LOOP_BACKPATCH_non_body_tn(bp);
    TN *body_tn = CG_LOOP_BACKPATCH_body_tn(bp);

    // Update the epilog backpatch, inserting a TN copy if necessary
    if (non_body_tn != body_tn) {
      CGPREP_Copy_TN_Into_BB(non_body_tn, body_tn, tail, NULL, 0, TRUE);
      CG_LOOP_BACKPATCH_Set_non_body_tn(bp, body_tn);
    }
  }

  // Update loop prolog backpatches, inserting new copy OPs into the
  // prolog when necessary

  for (bp = CG_LOOP_Backpatch_First(CG_LOOP_prolog, NULL); bp;
       bp = bp_next) {
    bp_next = CG_LOOP_Backpatch_Next(bp);
    TN *non_body_tn = CG_LOOP_BACKPATCH_non_body_tn(bp);
    TN *body_tn = CG_LOOP_BACKPATCH_body_tn(bp);

    // Update the prolog backpatch, inserting a TN copy if necessary
    if (body_tn != non_body_tn) {
      CGPREP_Copy_TN_Into_BB(body_tn, non_body_tn,
			     head, BB_last_op(head), 0, FALSE);
      CG_LOOP_BACKPATCH_Set_non_body_tn(bp, body_tn);
    }
  }
}


void Remove_Notations_With_Copies(BB *body, BB *head, BB *tail,
				  hTN_MAP tn_def_map, hTN_MAP32 tn_copies_map,
				  TN_LIST *need_copies, hTN_MAP new_tn_map)
{
  // Replace the epilog backpatches with new TNs, inserting new
  // copy OPs into the epilog when necessary.

  CG_LOOP_BACKPATCH *bp, *bp_next;
  for (bp = CG_LOOP_Backpatch_First(CG_LOOP_epilog, NULL); bp;
       bp = CG_LOOP_Backpatch_Next(bp)) {

    // Look up new body_tn, and update omega
    TN *body_tn = CG_LOOP_BACKPATCH_body_tn(bp);
    TN *non_body_tn = CG_LOOP_BACKPATCH_non_body_tn(bp);
    INT omega = CG_LOOP_BACKPATCH_omega(bp);
    if (omega > 0) {
      CG_LOOP_BACKPATCH_Set_omega(bp, 0);
      TN **new_tn_array = (TN **) hTN_MAP_Get(new_tn_map, body_tn);
      body_tn = new_tn_array[omega - 1];
      CG_LOOP_BACKPATCH_Set_body_tn(bp, body_tn);
    }

    // Update non_body_tn, inserting copy if necessary
    if (non_body_tn != body_tn) {
      CGPREP_Copy_TN_Into_BB(non_body_tn, body_tn, tail, NULL, 0, TRUE);
      CG_LOOP_BACKPATCH_Set_non_body_tn(bp, body_tn);
    }
  }

  // Replace old operand TNs and omegas with newly generated TNs

  for (OP *op = BB_first_op(body); op != NULL; op = OP_next(op))
    for (INT opnd = OP_opnds(op) - 1; opnd >= 0; --opnd) {

      // Skip if omega == 0; also skip special case TN100 <-- TN100[1]
      INT omega = OP_omega(op, opnd);
      if (omega == 0) continue;
      TN *tn = OP_opnd(op, opnd);
      OP *def_op = (OP *) hTN_MAP_Get(tn_def_map, tn);
      Is_True(def_op, ("CG_LOOP_Remove_Notations: NULL def_op"));
      if (omega == 1 && op == def_op) continue;

      // Update operand omega
      if (OP_Precedes(op, def_op)) {
	if (omega == 1) continue;
	--omega;
	Set_OP_omega(op, opnd, 1);
      } else
	Set_OP_omega(op, opnd, 0);

      // Update operand TN
      TN **new_tn_array = (TN **) hTN_MAP_Get(new_tn_map, tn);
      Set_OP_opnd(op, opnd, new_tn_array[omega - 1]);
    }

  // Update loop prolog backpatches with new TNs

  for (bp = CG_LOOP_Backpatch_First(CG_LOOP_prolog, NULL);
       bp; bp = bp_next) {
    bp_next = CG_LOOP_Backpatch_Next(bp);

    // Look up new body_tn, and update omega
    TN *body_tn = CG_LOOP_BACKPATCH_body_tn(bp); // old body_tn
    INT omega = CG_LOOP_BACKPATCH_omega(bp);
    if (omega > 1) {

      CG_LOOP_BACKPATCH_Set_omega(bp, 1);
      TN **new_tn_array = (TN **) hTN_MAP_Get(new_tn_map, body_tn);
      body_tn = new_tn_array[omega - 2];  // new body_tn
      CG_LOOP_BACKPATCH_Set_body_tn(bp, body_tn);
    }

    // Update non_body_tn, inserting copy if necessary
    TN *non_body_tn = CG_LOOP_BACKPATCH_non_body_tn(bp);
    if (body_tn != non_body_tn) {
      CGPREP_Copy_TN_Into_BB(body_tn, non_body_tn,
			     head, BB_last_op(head), 0, FALSE);
      CG_LOOP_BACKPATCH_Set_non_body_tn(bp, body_tn);
    }
  }

  // Insert TN copies before that TN's first definition

  for (TN_LIST *p = need_copies; p != NULL; p = TN_LIST_rest(p)) {
    TN *tn = TN_LIST_first(p);
    OP *op = (OP *) hTN_MAP_Get(tn_def_map, tn);
    INT copies = hTN_MAP32_Get(tn_copies_map, tn);
    TN **new_tn_array = (TN **) hTN_MAP_Get(new_tn_map, tn);

    // Insert new TN copies, in order, before first definition OP
    // Index cp must be positive and decreasing
    for (INT cp = copies - 1; cp > 0; --cp)
      CGPREP_Copy_TN(new_tn_array[cp], new_tn_array[cp - 1], op, 1, TRUE);
    CGPREP_Copy_TN(new_tn_array[0], tn, op, 1, TRUE);
  }
}


void CG_LOOP_Remove_Notations(LOOP_DESCR* loop, BB *head, BB *tail)
{
  BB *body = LOOP_DESCR_loophead(loop);

  if (Get_Trace(TP_CGLOOP, 0x4)) {
    #pragma mips_frequency_hint NEVER
    CG_LOOP_Trace_Loop(loop, "Before CG_LOOP_Remove_Notations");
  }

  // Determine the number of TN copies required for the TNs in this loop;

  // tn_def_map stores the first definition of each TN within the loop body
  // tn_copies_map stores the number of new TNs and TN copies that will be
  //   needed for each non-global TN within the loop body.  In the example
  //   above, tn_copies_map of TN100 is 3
  // need_copies is a list of all TNs with tn_copies_map[] > 0

  CXX_MEM_POOL pool("CG_LOOP_Remove_Notations", FALSE);
  hTN_MAP   tn_def_map    = hTN_MAP_Create(pool());
  hTN_MAP32 tn_copies_map = hTN_MAP32_Create(pool());
  TN_LIST *need_copies = Count_Copies_Needed(body, tn_def_map,
					     tn_copies_map, pool());

  if (need_copies == NULL) {

    // If no new TNs are required, then only the prolog and epilog
    // backpatches may require updating
    Remove_Notations_Without_Copies(head, tail);

  } else {  // need_copies != NULL

    // Generate a map to keep track of new TNs.  The values of this
    // map are arrays of new TNs.

    hTN_MAP new_tn_map = hTN_MAP_Create(pool());
    Generate_Copy_TNs(body, tn_copies_map, new_tn_map, need_copies, pool());

    Remove_Notations_With_Copies(body, head, tail, tn_def_map,
				 tn_copies_map, need_copies, new_tn_map);
  }

  if (Get_Trace(TP_CGLOOP, 0x4)) {
    #pragma mips_frequency_hint NEVER
    CG_LOOP_Trace_Loop( loop, "After CG_LOOP_Remove_Notations" );
  }
}


/* ====================================================================
 *
 *  Readers_Reach
 *
 *  Can all the readers of 'op' be rewritten to offset their loop
 *  invariant operands by 'count' * 'op's loop invariant operand?
 *  This is really checking of memory references that cannot be
 *  converted to the indexed form and whose literal fields cannot hold
 *  the offset implied by skipping count instances of 'op'.
 *
 *  Example:
 *
 *      iv = iv + 0x2000
 *         = 4(iv)
 *
 *      The memory reference reaches with a count of < 4 but not with
 *      a higher count, since 0x8000 is a negative 16 bit number.
 *
 * ====================================================================
 */

static BOOL
Readers_Reach(
  OP    *op,            /* IV update */
  INT32  count          /* Of loop overhead operations */
)
{
  INT64     offset;
  INT32     store_offset;
  TN       *liv_tn, *lit_tn;
  ARC_LIST *al;
  TN       *iv_tn = OP_result(op,0 /*???*/);

  if ( OP_opnd(op,0 /*???*/) == iv_tn )
    liv_tn = OP_opnd(op,1 /*???*/);
  else
    liv_tn = OP_opnd(op,0 /*???*/);

  for ( al = OP_succs(op) ; al; al = ARC_LIST_rest(al) ) {
    ARC *arc    = ARC_LIST_first(al);
    OP  *reader = ARC_succ(arc);

    if ( reader == op )
      continue;

    if ( OP_br(reader) )
      continue;

    if (    CGTARG_Have_Indexed_Mem_Insts()
         && OP_Is_Float_Mem(reader)
    ) {
      /* These can always be made to work by using a register to hold
       * the offset and doing an indexed load.
       */
      continue;
    }

    /* TODO: do we need to handle symbolic constants in the offset
     * fields here?  Having them there seems like an incorrect design,
     * so I'll guess we won't have to handle them.
     */

    store_offset = OP_store(reader) ? 1 : 0; /*???*/

    if ( OP_opnd(reader,store_offset) == iv_tn )
      lit_tn = OP_opnd(reader,store_offset + 1);
    else
      lit_tn = OP_opnd(reader,store_offset);

    FmtAssert(OP_iadd(op) || OP_isub(op),("Expected iadd or isub."));

    if ( OP_iadd(op) )
      offset = TN_value(lit_tn) + TN_value(liv_tn) * count;
    else
      offset = TN_value(lit_tn) - TN_value(liv_tn) * count;

    if ( ! TOP_Can_Have_Immediate(offset,OP_code(reader)) )
       return FALSE;
   }

   return TRUE;
 }



void CG_LOOP_Clear_SCCs(LOOP_DESCR *loop)
/* -----------------------------------------------------------------------
 * See "cg_loop.h" for interface description.
 * -----------------------------------------------------------------------
 */
{
  BB *bb = LOOP_DESCR_loophead(loop);
  OP *op;
  FOR_ALL_BB_OPs(bb, op) {
    _CG_LOOP_INFO *info = _CG_LOOP_info(op);
    /*
     * Each ARC is in one ancestor list and one descendent list.
     * To avoid deleting the same ARC twice, we'll just delete ARCs
     * from ancestor lists (could just as well use descendent
     * lists instead).
     */
    CG_DEP_Delete_SCC_Arcs(info->scc_ancestors);
    info->scc_ancestors = NULL;
    info->scc_descendents = NULL;
    info->scc = NULL;
  }
}

/* =====================================================================
 *			 Inner Loop Unrolling
 * =====================================================================
 */

 
/* ---------------------------------------------------------------------
 *			      TN mapping
 *
 * The following functions provide a service to rename TNs defined in
 * the loop body, and map the original names to new names given an
 * unrolling number.
 *
 * void unroll_names_init(LOOP_DESCR *loop, UINT16 ntimes, MEM_POOL *pool)
 *   Initialization function to be called before attempting to unroll
 *   <loop> <ntimes>.  Allocates memory from <pool> that must not be
 *   released until unroll_names_finish is called.
 *
 * TN *unroll_names_get(TN *tn, UINT16 unrolling)
 *   Requires: unrolling < ntimes (from last call to unroll_names_init).
 *   Return a TN representing <tn> in the given <unrolling>.  If <tn>
 *   isn't defined in the loop, it is returned as is.
 *
 * void unroll_names_finish(void)
 *   Cleanup function to be called after last call to unroll_names_get
 *   for a given unrolling attempt.
 *
 * ---------------------------------------------------------------------
 */

static TN_MAP unroll_names;


static void unroll_names_finish(void)
{
  TN_MAP_Delete(unroll_names);
  unroll_names_valid = FALSE;
}

#ifdef KEY
/* Given a result <tn>, check whether it shows up in the epilog backpatch with
   omega > 0.
 */
static BOOL Has_Nonzero_Omega_in_Epilog( TN* tn )
{
  if( !TN_is_gra_homeable(tn) )
    return FALSE;

  CG_LOOP_BACKPATCH* bpatches = CG_LOOP_Backpatch_First(CG_LOOP_epilog,tn);
  while( bpatches != NULL ){
    const TN* body_tn = CG_LOOP_BACKPATCH_body_tn(bpatches);
    if( body_tn == tn ){
      if( CG_LOOP_BACKPATCH_omega(bpatches) > 0 )
	return TRUE;
    }
    bpatches = CG_LOOP_Backpatch_Next(bpatches);
  }

  return FALSE;
}
#endif

static void unroll_names_init_tn(TN *result, UINT16 ntimes, MEM_POOL *pool)
{
  TN **entry = TYPE_MEM_POOL_ALLOC_N(TN *, pool, ntimes);
  UINT16 unrolling;
  TN_MAP_Set(unroll_names, result, entry);
#ifdef KEY
  const BOOL reset_gra_home =
    !TN_is_dedicated(result) && Has_Nonzero_Omega_in_Epilog(result);
#endif // KEY
  for (unrolling = 0; unrolling < ntimes; ++unrolling){
    if (TN_is_dedicated(result)){
      entry[unrolling] = result;
    }
#ifdef KEY
    /* Avoid generating code like
           <result> = <result_body_tn>
       in the loop epilog, where <result> is a gra homeable non_body tn;
       otherwise, redundant spill code could be generated.
     */
    else if( unrolling == ntimes - 1 && 
	     TN_is_gra_homeable(result) ){
      entry[unrolling] = result;
    }
#endif // KEY
    else{
      entry[unrolling] = Dup_TN(result);
#ifdef KEY
      /* If the <result> has a use outside of the body, and the omega is
	 greater than 0, we should nullify the home of the copied tn of
	 <result>; otherwise, gra cannot tell which home is the real home.
	 (bug#2698)
      */
      if( reset_gra_home &&
	  unrolling < ntimes - 1 ){
	Reset_TN_is_gra_homeable( entry[unrolling] );
	Set_TN_home( entry[unrolling], NULL );
      }
#endif // KEY      
    }
  }
}

// Bug 1064 & Bug 1221
#ifdef KEY
static BOOL TN_is_cond_def_of_another_op(BB *bb, TN *tn, OP *cand_op)
{
  OP *op;
  FOR_ALL_BB_OPs(bb, op) {
    if (cand_op == op)
      break;
    if (!OP_cond_def(op))
      continue;
    for (INT i = 0; i < OP_results(op); ++i) {
      TN *result_tn = OP_result(op,i);
      if (result_tn == tn)
        return TRUE;
    }
  }
  return FALSE;
}
#endif

static void unroll_names_init(LOOP_DESCR *loop, UINT16 ntimes, MEM_POOL *pool)
/* -----------------------------------------------------------------------
 * See above for interface description.
 * -----------------------------------------------------------------------
 */
{
  BB_SET *bbs = LOOP_DESCR_bbset(loop);
  Is_True(BB_SET_Size(bbs) == 1, ("unroll_names_init:  only support single BB loop."));

  BB *bb = LOOP_DESCR_loophead(loop);
  OP *op;

  Is_True(!unroll_names_valid, ("unroll_names_finish not called."));

  unroll_names = TN_MAP_Create();
  unroll_names_valid = TRUE;

  FOR_ALL_BB_OPs(bb, op) {
    for (INT i = 0; i < OP_results(op); ++i) {
      TN *result_tn = OP_result(op,i);
      
      if (!OP_cond_def(op) || 
	  !TN_is_global_reg(result_tn)) {
	
	if (OP_base_update_kind(op) == NO_BASE_UPDATE || 
	    (OP_load(op) && i == 0))  // prevent renaming of base-update incr
#ifdef KEY
          if (!TN_MAP_Get(unroll_names, result_tn) && !TN_is_cond_def_of_another_op(bb, result_tn, op))
#else
          if (!TN_MAP_Get(unroll_names, result_tn))
#endif
	    unroll_names_init_tn(result_tn, ntimes, pool);
      }
    }
  }

#ifdef Is_True_On
  FOR_ALL_BB_OPs(bb, op) {
    for (INT i = 0; i < OP_opnds(op); ++i) {
      TN *tn = OP_opnd(op,i);
      if (TN_is_register(tn) &&
	  OP_omega(op, i) >= 2)
	Is_True(TN_MAP_Get(unroll_names, tn),
		("unroll_names_init: must rename omega TN."));
    }
  }
#endif
}

#ifdef TARG_ST
static void unroll_names_multibb_init(BB *bb, MEM_POOL *pool)
/* -----------------------------------------------------------------------
 * See above for interface description.
 * -----------------------------------------------------------------------
 */
{
  OP *op;

  Is_True(!unroll_names_valid, ("unroll_names_finish not called."));

  unroll_names = TN_MAP_Create();
  unroll_names_valid = TRUE;

  FOR_ALL_BB_OPs(bb, op) {
    for (INT i = 0; i < OP_results(op); ++i) {
      TN *result_tn = OP_result(op,i);
      
      if (!OP_cond_def(op) || 
	  !TN_is_global_reg(result_tn)) {
	
	if (OP_base_update_kind(op) == NO_BASE_UPDATE || 
	    (OP_load(op) && i == 0))  // prevent renaming of base-update incr

	  if (!TN_MAP_Get(unroll_names, result_tn)) 
	    unroll_names_init_tn(result_tn, 1, pool);
      }
    }
  }
}
#endif


inline TN *unroll_names_get(TN *tn, UINT16 unrolling)
{
  if (TN_is_register(tn)) {
    TN **entry = (TN **)TN_MAP_Get(unroll_names, tn);
    return entry ? entry[unrolling] : tn;
  }
  return tn;
}

/* hack just to export unroll_names_get quickly for beta */
TN *CG_LOOP_unroll_names_get(TN *tn, UINT16 unrolling)
{
  if ( tn == NULL ) return NULL;
  return unroll_names_get(tn, unrolling);
}


/* ---------------------------------------------------------------------
 *			   Unrolling Notes
 * ---------------------------------------------------------------------
 */

typedef struct {
  mUINT16 ntimes;
  mBOOL const_trip;
} NOTE_REMAINDER_HEAD;
  
typedef struct {
  char *reason;
} NOTE_NOT_UNROLLED;
  

#define NOTE_ntimes(n)		((n)->ntimes)
#define NOTE_const_trip(n)	((n)->const_trip)
#define NOTE_reason(n)		((n)->reason)

static void remainder_head_note_handler(NOTE_ACTION action, NOTE_INFO *info,
					FILE *file)
/* -----------------------------------------------------------------------
 *  Handler for NOTE_REMAINDER_HEAD note.
 * -----------------------------------------------------------------------
 */
{
  NOTE_REMAINDER_HEAD *info_u = (NOTE_REMAINDER_HEAD *)info;
  UINT16 ntimes = NOTE_ntimes(info_u);
  BOOL ctrip = NOTE_const_trip(info_u);

  switch (action) {
  case NOTE_PRINT_TO_FILE:
    if (ctrip) {
      fprintf(file,
	      "%s<loop> Unrolling remainder loop (%d iteration%s)\n",
	      ASM_CMNT_LINE, ntimes, ntimes == 1 ? "" : "s");
    } else {
      fprintf(file,
	      "%s<loop> Unrolling remainder loop (at most %d iteration%s)\n",
	      ASM_CMNT_LINE, ntimes-1, ntimes-1 == 1 ? "" : "s");
    }
    if (ntimes == 0)
      DevWarn("Found remainder head note with ntimes = 0");
    break;
  case NOTE_PRINT_TO_ANL_FILE:
    /* ignore for now */
    break;
  case NOTE_PRINT_HANDLER_NAME_TO_FILE:
    fprintf(file, "remainder_head_note_handler");
    break;
  default:
    Is_True(FALSE, ("Didn't recognize action"));
  }
}



static void note_remainder_head(BB *head, UINT16 ntimes, UINT16 trips_if_const)
/* -----------------------------------------------------------------------
 * Attach a note to <head> saying it's the remainder loop for a loop
 * unrolled <ntimes>.  If <trips_if_const> is nonzero, it indicates
 * the trip count is the constant given.
 * -----------------------------------------------------------------------
 */
{
  NOTE_REMAINDER_HEAD *note;
  note = TYPE_MEM_POOL_ALLOC(NOTE_REMAINDER_HEAD, &MEM_pu_nz_pool);
  NOTE_const_trip(note) = trips_if_const > 0;
  NOTE_ntimes(note) = trips_if_const ? trips_if_const : ntimes;
  NOTE_Add_To_BB(head, remainder_head_note_handler, (NOTE_INFO *)note);
}

static void not_unrolled_note_handler(NOTE_ACTION action, NOTE_INFO *info,
				      FILE *file)
/* -----------------------------------------------------------------------
 *  Handler for NOTE_NOT_UNROLLED note.
 * -----------------------------------------------------------------------
 */
{
  NOTE_NOT_UNROLLED *info_u = (NOTE_NOT_UNROLLED *)info;
  char *reason = NOTE_reason(info_u);

  switch (action) {
  case NOTE_PRINT_TO_FILE:
    fprintf(file, "%s<loop> Not unrolled: %s\n", ASM_CMNT_LINE, reason);
    break;
  case NOTE_PRINT_TO_ANL_FILE:
    /* ignore for now */
    break;
  case NOTE_PRINT_HANDLER_NAME_TO_FILE:
    fprintf(file, "not_unrolled_note_handler");
    break;
  default:
    Is_True(FALSE, ("Didn't recognize action"));
  }
}



static void note_not_unrolled(BB *head, const char *reason, ...)
/* -----------------------------------------------------------------------
 * Attach a note to <head> saying it wasn't unrolled because <reason>.
 * <reason> and the following args are printf arguments.  <reason> must
 * expand to less than 256 characters (and should really by less than 80
 * for readability).
 * -----------------------------------------------------------------------
 */
{
  NOTE_NOT_UNROLLED *note;
  va_list args;
  char buf[256];

  /*
   * Emit reason into buf
   */
  va_start(args, reason);
  vsprintf(buf, reason, args);
  va_end(args);
  Is_True(strlen(buf) < 256, ("note_not_unrolled buffer overrun"));
  buf[255] = (char)0;	/* for robustness (though it's a little late) */

  note = TYPE_MEM_POOL_ALLOC(NOTE_NOT_UNROLLED, &MEM_pu_nz_pool);
  NOTE_reason(note) = TYPE_MEM_POOL_ALLOC_N(char, &MEM_pu_nz_pool,
					    strlen(buf)+1);
  strcpy(NOTE_reason(note), buf);
  NOTE_Add_To_BB(head, not_unrolled_note_handler, (NOTE_INFO *)note);
}


/* --------------------------------------------------------------------- */



#define is_power_of_two(i) (((i) & ((i)-1)) == 0)

#ifdef __CYGWIN__
#undef log2
#endif

inline UINT16 log2(UINT32 n)
/* -----------------------------------------------------------------------
 * Requires: n > 0.
 * Return the base-2 logarithm of <n> (truncated if <n> isn't a power of
 * two).
 * -----------------------------------------------------------------------
 */
{
  UINT16 result = 0;
  Is_True(n > 0, ("can't take logarithm of zero"));
  while ((1 << result) <= n)
    ++result;
  return --result;
}



static void unroll_guard_unrolled_body(LOOP_DESCR *loop,
				       LOOPINFO *unrolled_info,
				       TN *orig_trip_count_tn,
				       UINT32 ntimes)
/* -----------------------------------------------------------------------
 * Requires: !TN_is_constant(orig_trip_count_tn) && is_power_of_two(ntimes)
 *
 * Generates prolog/epilog code to avoid executing loop body when
 * <orig_trip_count_tn> / <ntimes> < 1.  Also replaces the trip count
 * TN in <unrolled_info> with a new one defined in CG_LOOP_prolog as the
 * number of trips for the unrolled loop.
 * -----------------------------------------------------------------------
 */
{
  if (!TN_is_constant(orig_trip_count_tn)) {
    INT64 trip_est = WN_loop_trip_est(LOOPINFO_wn(unrolled_info));
    TN *new_trip_count_tn = Build_TN_Like(orig_trip_count_tn);
    INT32 trip_size = TN_size(orig_trip_count_tn);
    float ztrip_prob = 1.0 / MAX(trip_est, 1);
    float orig_post_prolog_freq = BB_freq(BB_next(CG_LOOP_prolog));
    OPS ops = OPS_EMPTY;
    BB *continuation_bb;
    LABEL_IDX continuation_lbl;

    Is_True(is_power_of_two(ntimes), ("not power of two"));
    Is_True(!TN_is_constant(orig_trip_count_tn), ("trip count is constant"));

    LOOPINFO_trip_count_tn(unrolled_info) = new_trip_count_tn;

    extend_epilog(loop);
    continuation_bb = CG_LOOP_epilog;
    continuation_lbl = Gen_Label_For_BB(continuation_bb);

    /* We know <orig_trip_count_tn's> value is positive, and <ntimes> is a power
   * of two, so we can divide <orig_trip_count_tn> by <ntimes> with:
   *   <new_trip_count_tn> <- [d]sra <orig_trip_count_tn> log2(ntimes)
   * and guard the unrolled loop with:
   *   beq continuation_lbl <new_trip_count_tn> 0
   */
    Exp_OP2(trip_size == 4 ? OPC_I4ASHR : OPC_I8ASHR,
	    new_trip_count_tn,
	    orig_trip_count_tn,
	    Gen_Literal_TN(log2(ntimes), trip_size),
	    &ops);
#ifdef TARG_ST
    Exp_OP3v(OPC_FALSEBR,
	     NULL,
	     Gen_Label_TN(continuation_lbl,0),
	     new_trip_count_tn,
	     Zero_TN,
	     trip_size == 4 ? V_BR_I4EQ : V_BR_I8EQ,
	     &ops);
#else 
    Exp_OP3v(OPC_FALSEBR,
	     NULL,
	     Gen_Label_TN(continuation_lbl,0),
	     new_trip_count_tn,
	     Zero_TN,
	     V_BR_I8EQ,
	     &ops);
#endif
    BB_Append_Ops(CG_LOOP_prolog, &ops);
    Link_Pred_Succ_with_Prob(CG_LOOP_prolog, continuation_bb, ztrip_prob);
    Change_Succ_Prob(CG_LOOP_prolog, BB_next(CG_LOOP_prolog), 1.0 - ztrip_prob);
    BB_freq(BB_next(CG_LOOP_prolog)) = orig_post_prolog_freq;

    /* Extend prolog and epilog in case any further optimizations
   * want to use them.
   */
    extend_prolog();
    extend_epilog(loop);
  }
}



static void unroll_xfer_annotations(BB *unrolled_bb, BB *orig_bb)
/* -----------------------------------------------------------------------
 * Requires: <unrolled_bb> has unrolled OPs
 *
 * <unrolled_bb> is an unrolled version of <orig_bb>.  Look at the
 * annotations on <orig_bb>, handling them as follows:
 *   LABEL	do nothing (BBs must have unique labels)
 *   PRAGMA	copy to <unrolled_bb> (pragma WN shared, not copied)
 *   ENTRYINFO	do nothing (unrolled replicas shouldn't be entries)
 *   EXITINFO	copy to <unrolled_bb> and point sp_adj to spadjust OP
 *   CALLINFO	copy to <unrolled_bb> (call ST/WN shared, not copied)
 *   NOTE	copy to <unrolled_bb> (NOTE shared, not copied)
 *   LOOPINFO	do nothing (handled specially by unrolling routines)
 * -----------------------------------------------------------------------
 */
{
  if (BB_has_pragma(orig_bb)) {
    BB_Copy_Annotations(unrolled_bb, orig_bb, ANNOT_PRAGMA);
  }

  if (BB_exit(orig_bb)) {
    ANNOTATION *ant = ANNOT_Get(BB_annotations(orig_bb), ANNOT_EXITINFO);
    if (ant == NULL) {
      DevWarn("BB_exit(BB:%d) set but no ANNOT_EXITINFO attached",
	      BB_id(orig_bb));
    } else {
      EXITINFO *orig_info = ANNOT_exitinfo(ant);
      EXITINFO *unrolled_info = TYPE_PU_ALLOC(EXITINFO);
      *unrolled_info = *orig_info;
      if (EXITINFO_sp_adj(orig_info)) {
	OP *sp_adj;
	FOR_ALL_BB_OPs_REV(unrolled_bb, sp_adj)
	  if (OP_code(sp_adj) == TOP_spadjust) break;
	if (sp_adj == NULL)
	  DevWarn("missing spadjust OP in unrolled BB:%d", BB_id(unrolled_bb));
	EXITINFO_sp_adj(unrolled_info) = sp_adj;
      }
      BB_Add_Annotation(unrolled_bb, ANNOT_PRAGMA, orig_info);
    }
  }

  if (BB_call(orig_bb)) {
    BB_Copy_Annotations(unrolled_bb, orig_bb, ANNOT_CALLINFO);
  }

  if (BB_has_note(orig_bb)) {
    BB_Copy_Annotations(unrolled_bb, orig_bb, ANNOT_NOTE);
  }

#ifdef TARG_ST
  if (BB_asm(orig_bb)) {
    BB_Copy_Annotations(unrolled_bb, orig_bb, ANNOT_ASMINFO);
  }
#endif
}
  

static BB *Unroll_Replicate_Body(LOOP_DESCR *loop, INT32 ntimes, BOOL unroll_fully)
/* -----------------------------------------------------------------------
 * Requires: unroll_make_remainder_loop has not yet been called
 *	     BB_SET_Size(LOOP_DESCR_bbset(loop)) == 1
 * 
 * Construct <ntimes> unrollings of <loop>, attaching result in place
 * of <loop> before returning it.  Old loop body is left disconnected
 * from the control flow graph.  All TNs defined in <loop> will have
 * new names that can be found with unroll_names_get.  The resulting
 * loop will have a new label.
 * -----------------------------------------------------------------------
 */
{
  BB *body = LOOP_DESCR_loophead(loop);
  BB *unrolled_body = Gen_BB_Like(body);
  UINT16 unrolling;
  OP *op;
  ANNOTATION *annot = ANNOT_Get(BB_annotations(body), ANNOT_LOOPINFO);
  LOOPINFO *info = ANNOT_loopinfo(annot);
  TN *trip_count = LOOPINFO_trip_count_tn(info);
  INT16 new_trip_count_val;
  LOOPINFO *unrolled_info = TYPE_P_ALLOC(LOOPINFO);
  WN *wn = WN_COPY_Tree(LOOPINFO_wn(info));
  TYPE_ID ttype = WN_rtype(WN_loop_trip(wn));
  OPCODE opc_intconst = OPCODE_make_op(OPR_INTCONST, ttype, MTYPE_V);
  WN *ntimes_wn = WN_CreateIntconst(opc_intconst, ntimes);
  OPCODE opc_div = OPCODE_make_op(OPR_DIV, ttype, MTYPE_V);

  /* Catch calls that should be going to unroll_multi_bb instead */
  Is_True(BB_SET_Size(LOOP_DESCR_bbset(loop)) == 1,
	    ("unroll_replicate_body passed multi-bb loop body"));

  if (BB_freq_fb_based(body)) Set_BB_freq_fb_based(unrolled_body);

  if (TN_is_constant(trip_count)) {
    new_trip_count_val = TN_value(trip_count) / ntimes;
    Is_True((unroll_fully && new_trip_count_val == 1) ||
	    (!unroll_fully && new_trip_count_val > 1),
	    ("new_trip_count_val does not make sense."));
  }

  /* Setup new <unrolled_body> LOOPINFO.
   */
  WN_set_loop_trip(wn, WN_CreateExp2(opc_div, WN_loop_trip(wn), ntimes_wn));
#ifdef TARG_ST
  // TB WN_loop_trip_est(wn) cannot be NULL
  WN_loop_trip_est(wn) = MAX(WN_loop_trip_est(wn) / ntimes, 1);
#else
  WN_loop_trip_est(wn) = WN_loop_trip_est(wn) / ntimes;
#endif
  LOOPINFO_wn(unrolled_info) = wn;
  LOOPINFO_srcpos(unrolled_info) = LOOPINFO_srcpos(info);
  if (TN_is_constant(trip_count))
    LOOPINFO_trip_count_tn(unrolled_info) =
      Gen_Literal_TN(new_trip_count_val, TN_size(trip_count));
  Set_BB_unrollings(unrolled_body, ntimes);
  if (unroll_fully) Set_BB_unrolled_fully(unrolled_body);
  BB_Add_Annotation(unrolled_body, ANNOT_LOOPINFO, unrolled_info);

  bool trace_pref = Get_Trace(TP_CGLOOP, 2);

  /* Replicate the body <ntimes>.  Note that we don't emit a branch
   * if this isn't <!unroll_fully>.
   */
  for (unrolling = 0; unrolling < ntimes; unrolling++) {
    FOR_ALL_BB_OPs(body, op) {

      // Perform Prefetch pruning at unroll time
      if (OP_prefetch(op)) {

	WN *mem_wn = Get_WN_From_Memory_OP(op);
	Is_True(!mem_wn || WN_operator(mem_wn) == OPR_PREFETCH,
		("wrong prefetch WHIRL node."));

	if (trace_pref && unrolling == 0)  // trace once per loop
	  if (mem_wn)
	    fprintf(TFile, "<cgpref> - 1L cache stride = %d, 2L cache stride = %d,"
		    " confidence = %d\n",
		    WN_pf_stride_1L(mem_wn),
		    WN_pf_stride_2L(mem_wn),
		    WN_pf_confidence(mem_wn));
	  else
	    fprintf(TFile, "<cgpref> pref wn not found.\n");

	if (mem_wn && Prefetch_Kind_Enabled(mem_wn)) {
	  int stride = WN_pf_stride_2L( mem_wn ) ?  WN_pf_stride_2L( mem_wn ) :  WN_pf_stride_1L(mem_wn);
	  if (stride != 0 && (unrolling % stride) != 0) {
	    if (trace_pref)
	      fprintf(TFile, "<cgpref> pref pruned at unrolling %d.\n", unrolling);
	    continue;
	  }
	}
      }
      if (!OP_br(op) || !unroll_fully && unrolling == ntimes-1) {
	UINT8 opnd;
	UINT8 res;
	OP *new_op = Dup_OP(op);
	CGPREP_Init_Op(new_op);
	CG_LOOP_Init_Op(new_op);
	Copy_WN_For_Memory_OP(new_op, op);

        if (OP_loh(op)) 
          Set_OP_loh(new_op);

	Set_OP_unrolling(new_op, unrolling);
	Set_OP_orig_idx(new_op, OP_map_idx(op));
	Set_OP_unroll_bb(new_op, unrolled_body);
	for (res = 0; res < OP_results(op); ++res) {
	  TN *new_res = unroll_names_get(OP_result(op,res), unrolling);
	  Set_OP_result(new_op, res, new_res);
	}
	for (opnd = 0; opnd < OP_opnds(op); opnd++) {
	  INT omega = OP_omega(op,opnd);
	  INT adjust = omega + (ntimes - unrolling - 1);
	  INT new_omega = adjust / ntimes;
	  INT which = ntimes - 1 - (adjust - (new_omega * ntimes));

	  //  I think the above is equivalent to this. -Raymond
	  Is_True(new_omega * ntimes + (unrolling - which) == omega,
		  ("new_omega * ntimes + (unrolling - which) == omega."));

	  TN *new_tn = unroll_names_get(OP_opnd(op,opnd), which);
	  Is_True(omega == 0 || new_tn != NULL,
		  ("Replicate_Unroll_Body: TN with non-zero omega must be renamed."));
	  Set_OP_opnd(new_op, opnd, new_tn);

	  // omega of OP_br unchanged because it has only 1 copy
	  if (OP_br(op))
	    new_omega = omega;

	  Set_OP_omega(new_op, opnd, new_omega);
	}
	BB_Append_Op(unrolled_body, new_op);
      }
    }
  }

  unroll_xfer_annotations(unrolled_body, body);

  if (!unroll_fully) {
    /*
     * Generate new label for top of loop and fix branch instruction,
     * and place <unrolled_body> on its own succs/preds list.
     */
    op = BB_branch_op(unrolled_body);
    Is_True(op, ("didn't insert loopback branch correctly"));
    Set_OP_opnd(op,
		Branch_Target_Operand(op),
		Gen_Label_TN(Gen_Label_For_BB(unrolled_body), 0));
    INT loop_count = WN_loop_trip_est(wn) ? WN_loop_trip_est(wn) : 100;
    Link_Pred_Succ_with_Prob(unrolled_body, unrolled_body,
			     (loop_count - 1.0) / loop_count);
  }

  float new_body_freq = BB_freq(body) / ntimes;
  
  /* Replace <body> with <unrolled_body> in BB chain & CFG */
  Chain_BBs(BB_prev(body), unrolled_body);
  Chain_BBs(unrolled_body, BB_next(body));
  LOOP_DESCR_Retarget_Loop_Entrances(loop, unrolled_body);
  Unlink_Pred_Succ(body, BB_next(body));
  INT loop_count = WN_loop_trip_est(wn) ? WN_loop_trip_est(wn) : 100;
  Link_Pred_Succ_with_Prob(unrolled_body, BB_next(body),
			   1.0 / loop_count);
  BB_next(body) = BB_prev(body) = NULL;

  if (FREQ_Frequencies_Computed() || BB_freq_fb_based(body)) {
    BB_freq(unrolled_body) = new_body_freq;
  }

  /* GRA liveness info
   */
  GRA_LIVE_Compute_Local_Info(unrolled_body);

  return unrolled_body;
}

#ifdef TARG_ST
BOOL
Check_remainder_after(BB *body, BB *trip_count_bb, TN *trip_count_tn, UINT32 ntimes) {
  OP *br_op = BB_branch_op(body);
  OP *cmp_op;
  TN *tn1, *tn2;

  if (!is_power_of_two(ntimes)) 
    return FALSE;

  if (!br_op || !OP_cond(br_op))
    return FALSE;

  VARIANT variant = CGTARG_Analyze_Compare(br_op, &tn1, &tn2, &cmp_op);
  if (!cmp_op)
    return FALSE;

  if (tn1 != trip_count_tn && tn2 != trip_count_tn) {
    OP *def;
    DEF_KIND kind;
    if (TN_is_register(trip_count_tn)) {
      def = TN_Reaching_Value_At_Op(trip_count_tn, cmp_op, &kind, TRUE);
      if (def && OP_move(def)) {
	trip_count_tn = OP_opnd(def, OP_Copy_Operand(def));
      }
    }
  }

  if (tn1 != trip_count_tn && tn2 != trip_count_tn) {
    OP *def;
    DEF_KIND kind;
    if (TN_is_register(tn1)) {
      def = TN_Reaching_Value_At_Op(tn1, cmp_op, &kind, TRUE);
      if (def && OP_move(def)) {
	tn1 = OP_opnd(def, OP_Copy_Operand(def));
      }
    }
  }

  if (tn1 != trip_count_tn && tn2 != trip_count_tn) {
    OP *def;
    DEF_KIND kind;
    if (tn2 /* cmp_op may be mtb */ && TN_is_register(tn2)) {
      def = TN_Reaching_Value_At_Op(tn2, cmp_op, &kind, TRUE);
      if (def && OP_move(def)) {
	tn2 = OP_opnd(def, OP_Copy_Operand(def));
      }
    }
  }

  if (tn1 != trip_count_tn && tn2 != trip_count_tn)
    return FALSE;

  TN *trip_count_floor;
  
  if (TN_is_constant(trip_count_tn)) {
    INT32 new_trip_count_val = TN_value(trip_count_tn) & (-ntimes);
    trip_count_floor = Gen_Literal_TN(new_trip_count_val, 4);
  }
  else {

  /* Create a new TN trip_count_floor = trip_count_tn & (-times) */
    trip_count_floor = Build_TN_Like(trip_count_tn);
    OPS prolog_ops = OPS_EMPTY;
    INT32 trip_size = TN_size(trip_count_tn);

    Exp_OP2(trip_size == 4 ? OPC_U4BAND : OPC_U8BAND,
	    trip_count_floor,
	    trip_count_tn,
	    Gen_Literal_TN(-ntimes, trip_size),
	    &prolog_ops);
    BB_Append_Ops(CG_LOOP_prolog, &prolog_ops);
  }

  if (tn1 == trip_count_tn)
    Set_OP_opnd(cmp_op, 0, trip_count_floor);
  else
    Set_OP_opnd(cmp_op, 1, trip_count_floor);

  return TRUE;
}
#endif

#ifdef TARG_ST
void Unroll_Make_Remainder_Loop(CG_LOOP& cl, INT32 ntimes, INT32 remainder_val, BB *unrolled_head, BOOL remainder_after)
#else
void Unroll_Make_Remainder_Loop(CG_LOOP& cl, INT32 ntimes)
#endif
/* -----------------------------------------------------------------------
 * Turn LOOP_DESCR_loophead(loop) into a "remainder" loop in preparation
 * for a version being unrolled <ntimes>.  The remainder loop executes
 * <trip_count> % <ntimes> iterations.  If the remainder loop will execute
 * less than two times, we remove the ending branch, so it's not really a
 * loop.
 *
 * TODO: Add LOOP_DESCR for remainder loop?
 * -----------------------------------------------------------------------
 */
{
  LOOP_DESCR *loop = cl.Loop();
  BB *body = LOOP_DESCR_loophead(loop);
  ANNOTATION *annot = ANNOT_Get(BB_annotations(body), ANNOT_LOOPINFO);
  LOOPINFO *info = ANNOT_loopinfo(annot);
  TN *trip_count = LOOPINFO_trip_count_tn(info);
  TN *new_trip_count, *label_tn;
  OP *br_op = BB_branch_op(body);
  BB *remainder_tail;
  LABEL_IDX continuation_label = (LABEL_IDX)0;
  CG_LOOP_BACKPATCH *bp;
  BOOL freqs = FREQ_Frequencies_Computed();
  float loop_entry_freq;
  if (freqs)
    loop_entry_freq = BB_freq(CG_LOOP_prolog);

  Is_True(info == LOOP_DESCR_loopinfo(loop),
	    ("LOOPINFO for head BB disagrees with that in loop descriptor"));
  Is_True(BB_unrollings(body) < 2,
	    ("unrolled loophead passed to unroll_make_remainder_loop"));

  if (Get_Trace(TP_CGLOOP, 0x4)) {
    #pragma mips_frequency_hint NEVER
    CG_LOOP_Trace_Loop(loop, "Before Unroll_Make_Remainder_Loop");
  }

  /* Now we call CG_LOOP_Remove_Notations for the remainder loop (since
   * it won't be SWPd).  We use the original prolog backpatches, but
   * not the original epilog backpatches since those belong on the
   * unrolled loop.  We'll construct new epilog backpatches for the
   * remainder loop and new prolog backpatches for the unrolled loop to
   * shuttle exposed-use values from the remainder loop to the unrolled
   * loop.  For example, if the original prolog backpatches are:
   *   TN200[2] <- ...
   *   TN200[1] <- ...
   *   TN201[1] <- ...
   * we'll add the following epilog backpatches for the remainder loop:
   *   temp1 <- TN200[1]
   *   temp2 <- TN200[0]
   *   temp3 <- TN201[0]
   * (We simply subtract one from each omega, since we're now at the end
   * of an iteration.  Note that prolog backpatches must have positive
   * omegas.)  To complete the transfer, the prolog backpatches for the
   * unrolled loop are now:
   *   TN200[2] <- temp1
   *   TN200[1] <- temp2
   *   TN201[1] <- temp3
   * Note that these new prolog backpatches (as well as the old epilog
   * backpatches) are written in terms of the body TNs before unrolling.
   * The unroller will later rename these to the new body TNs.
   */ 

  // Determine the number of TN copies required for the TNs in this loop;

  // tn_def_map stores the first definition of each TN within the loop body
  // tn_copies_map stores the number of new TNs and TN copies that will be
  //   needed for each non-global TN within the loop body.  In the example
  //   above, tn_copies_map of TN100 is 3
  // need_copies is a list of all TNs with tn_copies_map[] > 0

  CXX_MEM_POOL pool("Unroll_Make_Remainder_Loop", FALSE);
  hTN_MAP   tn_def_map    = hTN_MAP_Create(pool());
  hTN_MAP32 tn_copies_map = hTN_MAP32_Create(pool());
  TN_LIST *need_copies = Count_Copies_Needed(body, tn_def_map,
					     tn_copies_map, pool());
  hTN_MAP new_tn_map = NULL;

  // Modify the prolog and epilog backpatches so that the non_body TNs
  // match the TNs that they will be assigned to.

  if (need_copies == NULL) {

    for (bp = CG_LOOP_Backpatch_First(CG_LOOP_prolog, NULL); bp;
	 bp = CG_LOOP_Backpatch_Next(bp)) {

      // Update non_body_tn, inserting copy if necessary
      TN *body_tn = CG_LOOP_BACKPATCH_body_tn(bp);
      TN *non_body_tn = CG_LOOP_BACKPATCH_non_body_tn(bp);
      if (non_body_tn != body_tn) {
	CGPREP_Copy_TN_Into_BB(body_tn, non_body_tn, CG_LOOP_prolog,
			       BB_last_op(CG_LOOP_prolog), 0, FALSE);
	CG_LOOP_BACKPATCH_Set_non_body_tn(bp, body_tn);
      }
    }

    for (bp = CG_LOOP_Backpatch_First(CG_LOOP_epilog, NULL); bp;
	 bp = CG_LOOP_Backpatch_Next(bp)) {

      // Update non_body_tn, inserting copy if necessary
      TN *body_tn = CG_LOOP_BACKPATCH_body_tn(bp);
      TN *non_body_tn = CG_LOOP_BACKPATCH_non_body_tn(bp);
      if (non_body_tn != body_tn) {
	CGPREP_Copy_TN_Into_BB(non_body_tn, body_tn,
			       CG_LOOP_epilog, NULL, 0, TRUE);
	CG_LOOP_BACKPATCH_Set_non_body_tn(bp, body_tn);
      }
    }

  } else {  // need_copies != NULL

    // Generate a map to keep track of new TNs.  The values of this
    // map are arrays of new TNs.

    new_tn_map = hTN_MAP_Create(pool());
    Generate_Copy_TNs(body, tn_copies_map, new_tn_map, need_copies, pool());

    // Modify the prolog backpatches so that the non_body TNs match the
    // TNs that they will be assigned to.

    for (bp = CG_LOOP_Backpatch_First(CG_LOOP_prolog, NULL); bp;
	 bp = CG_LOOP_Backpatch_Next(bp)) {

      // Look up non_body_tn and new body_tn
      TN *body_tn = CG_LOOP_BACKPATCH_body_tn(bp);
      TN *non_body_tn = CG_LOOP_BACKPATCH_non_body_tn(bp);
      INT omega = CG_LOOP_BACKPATCH_omega(bp) - 1;
      if (omega > 0) {
	TN **new_tn_array = (TN **) hTN_MAP_Get(new_tn_map, body_tn);
	body_tn = new_tn_array[omega - 1];
      }

      // Update non_body_tn, inserting copy if necessary
      if (non_body_tn != body_tn) {
	CGPREP_Copy_TN_Into_BB(body_tn, non_body_tn, CG_LOOP_prolog,
			       BB_last_op(CG_LOOP_prolog), 0, FALSE);
	CG_LOOP_BACKPATCH_Set_non_body_tn(bp, body_tn);
      }
    }

    // Modify the epilog backpatches so that the non_body TNs match the
    // TNs that they will be assigned to.

    for (bp = CG_LOOP_Backpatch_First(CG_LOOP_epilog, NULL); bp;
	 bp = CG_LOOP_Backpatch_Next(bp)) {

      // Look up non_body_tn and new body_tn
      TN *body_tn = CG_LOOP_BACKPATCH_body_tn(bp);
      TN *non_body_tn = CG_LOOP_BACKPATCH_non_body_tn(bp);
      INT omega = CG_LOOP_BACKPATCH_omega(bp);
      if (omega > 0) {
	TN **new_tn_array = (TN **) hTN_MAP_Get(new_tn_map, body_tn);
	body_tn = new_tn_array[omega - 1];
      }

      // Update non_body_tn, inserting copy if necessary
      if (non_body_tn != body_tn) {
	CGPREP_Copy_TN_Into_BB(non_body_tn, body_tn,
			       CG_LOOP_epilog, NULL, 0, TRUE);
	CG_LOOP_BACKPATCH_Set_non_body_tn(bp, body_tn);
      }
    }
  }

  if (Get_Trace(TP_CGLOOP, 0x4)) {
    #pragma mips_frequency_hint NEVER
    CG_LOOP_Trace_Loop(loop, "1st Check Unroll_Make_Remainder_Loop");
  }

  // Generate the backpatches for the remainer loop and the main loop;
  // This requires a hack: Since the CG_LOOP_BACKPATCH_* procedures only
  // act on CG_LOOP_prolog and CG_LOOP_epilog, the values of
  // prolog_backpatches and epilog_backpatches are redefined to the
  // backpatch lists we want to work on at any given time.

  CG_LOOP_BACKPATCH *main_loop_prolog_backpatches;
  CG_LOOP_BACKPATCH *main_loop_epilog_backpatches;
  CG_LOOP_BACKPATCH *remainder_prolog_backpatches;
  CG_LOOP_BACKPATCH *remainder_epilog_backpatches;

#ifdef TARG_ST
  if (remainder_after) {
    main_loop_prolog_backpatches = prolog_backpatches;
    main_loop_epilog_backpatches = NULL;
    remainder_prolog_backpatches = NULL;
    remainder_epilog_backpatches = epilog_backpatches;
  } else
#endif
  {
    remainder_prolog_backpatches = prolog_backpatches;
    remainder_epilog_backpatches = NULL;
    main_loop_prolog_backpatches = NULL;
    main_loop_epilog_backpatches = epilog_backpatches;
  }

  // Generate remainder_epilog_backpatches and main_loop_prolog_backpatches

  prolog_backpatches = NULL;
  epilog_backpatches = NULL;

  // Insert prolog GTN400[1] <-- GTN400 and epilog GTN400 <-- GTN400[0]
  // backpatches for all GTNs appearing in loop

  CG_LOOP_DEF tn_def(body);

  OP *op;
#ifdef PROLOG_OPT//TARG_ST
  // FdF: Prevent unrolling from creating uninitialized uses
  OPS uninit_ops = OPS_EMPTY;
#endif
  FOR_ALL_BB_OPs(body, op) {

    // For live-out defs, create an epilog backpatch.
    for (INT res = 0; res < OP_results(op); res++) {
      TN *tn = OP_result(op, res);
      if (TN_is_register(tn) && ! TN_is_dedicated(tn)
	  && ! TN_is_const_reg(tn)
	  && GTN_SET_MemberP(BB_live_in(CG_LOOP_epilog), tn)) {
	CG_LOOP_Backpatch_Add(CG_LOOP_epilog, tn, tn, 0);
	CG_LOOP_Backpatch_Add(CG_LOOP_prolog, tn, tn, 1);
#ifdef PROLOG_OPT//TARG_ST
	// FdF: Prevent unrolling from creating uninitialized uses. If
	// the loop has a non-zero trip count and tn was not live-in
	// of the loop, initialize the TN to zero.
	if (WN_Loop_Nz_Trip(LOOPINFO_wn(info)) &&
	    !GTN_SET_MemberP(BB_live_in(CG_LOOP_prolog), tn)) {
	  Exp_COPY(tn, Zero_TN, &uninit_ops);
	}
#endif
      }
    }

    // For exposed uses, create a prolog backpatch.
    for (INT opnd = 0; opnd < OP_opnds(op); opnd++) {
      TN *tn = OP_opnd(op, opnd);
      if (TN_is_register(tn) && ! TN_is_dedicated(tn) &&
	  ! TN_is_const_reg(tn)) {
	OP *def_op = tn_def.Get(tn);
	// TN is not an invariant and TN is not defined before this OP
	if (def_op && ! OP_Precedes(def_op, op)
	    && ! CG_LOOP_Backpatch_Find_Non_Body_TN(CG_LOOP_prolog, tn, 1) ) {
	  CG_LOOP_Backpatch_Add(CG_LOOP_epilog, tn, tn, 0);
	  CG_LOOP_Backpatch_Add(CG_LOOP_prolog, tn, tn, 1);
	}
      }
    }
  }

#ifdef PROLOG_OPT//TARG_ST
    // FdF: Prevent unrolling from creating uninitialized uses
    BB_Prepend_Ops(CG_LOOP_prolog, &uninit_ops);
#endif
  // Add backpatches for higher omega TN copies

  if (need_copies) {

    for (TN_LIST *p = need_copies; p != NULL; p = TN_LIST_rest(p)) {
      TN *body_tn = TN_LIST_first(p);
      INT copies = hTN_MAP32_Get(tn_copies_map, body_tn);
      TN **new_tn_array = (TN **) hTN_MAP_Get(new_tn_map, body_tn);

      for (INT cp = copies; cp > 0; --cp) {
	TN *tn = new_tn_array[cp - 1];
	CG_LOOP_Backpatch_Add(CG_LOOP_epilog, tn, body_tn, cp);
	CG_LOOP_Backpatch_Add(CG_LOOP_prolog, tn, body_tn, cp + 1);
      }
      if (! CG_LOOP_Backpatch_Find_Non_Body_TN(CG_LOOP_prolog, body_tn, 1)) {
	CG_LOOP_Backpatch_Add(CG_LOOP_epilog, body_tn, body_tn, 0);
	CG_LOOP_Backpatch_Add(CG_LOOP_prolog, body_tn, body_tn, 1);
      }
    }
  }
#ifdef TARG_ST
  if (remainder_after) {
    main_loop_epilog_backpatches = epilog_backpatches;
    remainder_prolog_backpatches = prolog_backpatches;
  } else
#endif
  {
    remainder_epilog_backpatches = epilog_backpatches;
    main_loop_prolog_backpatches = prolog_backpatches;
  }

  // Remove notations from the remainder loop
  remainder_tail = Gen_BB_Like(body);
  if (BB_freq_fb_based(CG_LOOP_prolog))
    Set_BB_freq_fb_based(remainder_tail);

  prolog_backpatches = remainder_prolog_backpatches;
  epilog_backpatches = remainder_epilog_backpatches;

  if (Get_Trace(TP_CGLOOP, 0x4)) {
    #pragma mips_frequency_hint NEVER
    CG_LOOP_Trace_Loop(loop, "2nd Check Unroll_Make_Remainder_Loop");
  }

  // WAS:  CG_LOOP_Remove_Notations(cl, CG_LOOP_prolog, remainder_tail);
  if (need_copies == NULL) {

    // If no new TNs are required, then only the prolog and epilog
    // backpatches may require updating
    Remove_Notations_Without_Copies(CG_LOOP_prolog, remainder_tail);

  } else {  // need_copies != NULL

    Remove_Notations_With_Copies(body, CG_LOOP_prolog, remainder_tail,
				 tn_def_map,
				 tn_copies_map, need_copies, new_tn_map);
  }

  // Recompute liveness
  cl.Recompute_Liveness();

  if (Get_Trace(TP_CGLOOP, 0x4)) {
    #pragma mips_frequency_hint NEVER
    CG_LOOP_Trace_Loop(loop, "3rd Check Unroll_Make_Remainder_Loop");
  }

  OPS zero_trip_guard_ops = OPS_EMPTY;
  OPS prolog_ops = OPS_EMPTY;
  BOOL const_trip = TN_is_constant(trip_count);

#ifdef TARG_ST
  BB* SAVE_prolog = CG_LOOP_prolog;
  if (remainder_after) {
    extend_epilog(loop);
    CG_LOOP_prolog = CG_LOOP_epilog;
  }
#endif

  if (const_trip) {
    /* If trip count is a constant, see how many times the remainder
     * loop will execute (if at all) before proceeding.  If it won't
     * execute at all, don't make a remainder loop.  Otherwise, create
     * a constant TN for the new trip count.
     */
    INT16 new_trip_count_val = TN_value(trip_count) % ntimes;
    Is_True(new_trip_count_val > 0,
	    ("unroll_make_remainder_loop: trip count is negative or zero"));
    new_trip_count = Gen_Literal_TN(new_trip_count_val, 4);

#ifdef TARG_ST
  } else if (remainder_val != -1) {
    new_trip_count = Gen_Literal_TN(remainder_val, 4);
    const_trip = 1;
#endif

  } else {

    /* Add zero-trip guard for remainder loop if necessary:
     *   if (trip_count % ntimes <= 0) skip remainder
     * We know <trip_count>'s value is positive, and <ntimes> is
     * a power of two, so we can perform this with:
     *   <new_trip_count> <- andi <trip_count> <ntimes>-1
     *   beq continuation_label <new_trip_count> 0
     */
    float ztrip_prob = 1.0 / ntimes;
    INT32 trip_size = TN_size(trip_count);
    new_trip_count = Build_TN_Like(trip_count);
    if (is_power_of_two(ntimes)) 
      Exp_OP2(trip_size == 4 ? OPC_U4BAND : OPC_U8BAND,
	      new_trip_count,
	      trip_count,
	      Gen_Literal_TN(ntimes-1, trip_size),
	      &prolog_ops);
    else
      Exp_OP2(trip_size == 4 ? OPC_U4MOD : OPC_U8MOD,
	      new_trip_count,
	      trip_count,
	      Gen_Literal_TN(ntimes, trip_size),
	      &prolog_ops);
    
    continuation_label = Gen_Label_For_BB(remainder_tail);
#ifdef TARG_ST
    Exp_OP3v(OPC_FALSEBR,
	     NULL,
	     Gen_Label_TN(continuation_label,0),
	     new_trip_count,
	     Zero_TN,
	     trip_size == 4 ? V_BR_I4EQ : V_BR_I8EQ,
	     &zero_trip_guard_ops);
#else
    Exp_OP3v(OPC_FALSEBR,
	     NULL,
	     Gen_Label_TN(continuation_label,0),
	     new_trip_count,
	     Zero_TN,
	     V_BR_I8EQ,
	     &zero_trip_guard_ops);
#endif

    Link_Pred_Succ_with_Prob(CG_LOOP_prolog, remainder_tail, ztrip_prob);
    if (freqs || BB_freq_fb_based(CG_LOOP_prolog))
      BB_freq(remainder_tail) += loop_entry_freq * ztrip_prob;
    if (freqs)
      Change_Succ_Prob(CG_LOOP_prolog, BB_next(CG_LOOP_prolog),
		       1.0 - ztrip_prob);
  }

  /* Remove the branch at the end of <body>, remembering the label TN
   * and dbnum (if any) for later use if we're going to replace it with
   * another branch.
   */
  Is_True(br_op && OP_br(br_op), ("loop body doesn't end in branch"));

  label_tn = OP_opnd(br_op, Branch_Target_Operand(br_op));
  Is_True(TN_is_label(label_tn), ("branch operand not a label"));
  BB_Remove_Op(body, br_op);

  BB *first_remainder_body = body;
  float body_freq = 0.0;
  
#ifdef TARG_ST
  REMAINDERINFO *remainder_info = TYPE_P_ALLOC(REMAINDERINFO);
  REMAINDERINFO_head_bb(remainder_info) = unrolled_head;
#endif
      
  if (const_trip && TN_value(new_trip_count) < 2) {
    /*
     * Remainder isn't really a loop, so remove it from its own BB_succs/
     * BB_preds and remove the LOOPINFO annotation.
     */
    BB_annotations(body) = ANNOT_Unlink(BB_annotations(body), annot);
    Unlink_Pred_Succ(body, body);
    Set_BB_loop_head_bb(body, NULL);
    body_freq = loop_entry_freq;
#ifdef TARG_ST
    // [CG]: Add REMAINDERINFO Annotation
    BB_Add_Annotation(body, ANNOT_REMAINDERINFO, remainder_info);
#endif
  } else {

    if (ntimes == 2 || CG_LOOP_unroll_remainder_fully) {
      // fully unroll the remainder loop
      INT32 unroll_times = (const_trip ? (TN_value(new_trip_count) % ntimes)
			    : (ntimes - 1));
      BB *unrolled_body = Gen_BB_Like(body);
      first_remainder_body = unrolled_body;

      OPS body_ops = OPS_EMPTY;
      if (!const_trip) {
#ifdef TARG_ST
	BB_Append_Ops(SAVE_prolog, &prolog_ops);
#else
	BB_Append_Ops(CG_LOOP_prolog, &prolog_ops);
#endif
	BB_Append_Ops(CG_LOOP_prolog, &zero_trip_guard_ops);

	INT32 trip_size = TN_size(new_trip_count);
	Exp_OP2(trip_size == 4 ? OPC_I4ADD : OPC_I8ADD,
		new_trip_count,
		new_trip_count,
		Gen_Literal_TN(-1, trip_size),
		&body_ops);
#ifdef TARG_ST
	Exp_OP3v(OPC_TRUEBR,
		 NULL,
		 Gen_Label_TN(continuation_label,0),
		 new_trip_count,
		 Zero_TN,
		 trip_size == 4 ? V_BR_I4EQ : V_BR_I8EQ,
		 &body_ops);
#else
	Exp_OP3v(OPC_TRUEBR,
		 NULL,
		 Gen_Label_TN(continuation_label,0),
		 new_trip_count,
		 Zero_TN,
		 V_BR_I8EQ,
		 &body_ops);
#endif
      }
      else {
        Set_BB_unrollings(unrolled_body, unroll_times);
      }

      for (INT32 unrolling = 1; unrolling <= unroll_times; unrolling++) {
	OP *op;
	FOR_ALL_BB_OPs(body, op) {
	  // keep the prefetches ops of the first unrolling
	  if (OP_prefetch(op) && unrolling != unroll_times)
	    continue;
	  OP *new_op = Dup_OP(op);
#ifdef TARG_ST
	  // [CG] Set op unrolling even on multi bb remainder
	  Set_OP_unrolling(new_op, unrolling-1);
	  Set_OP_orig_idx(new_op, OP_map_idx(op));
	  Set_OP_unroll_bb(new_op, unrolled_body);
#else
          if (const_trip) {
	    Set_OP_unrolling(new_op, unrolling-1);
	    Set_OP_orig_idx(new_op, OP_map_idx(op));
	    Set_OP_unroll_bb(new_op, unrolled_body);
          }
#endif
	  CG_LOOP_Init_Op(new_op);
	  Copy_WN_For_Memory_OP(new_op, op);
	  BB_Append_Op(unrolled_body, new_op);
	}
#ifdef TARG_ST
	// [CG]: Add REMAINDERINFO Annotation
	BB_Add_Annotation(unrolled_body, ANNOT_REMAINDERINFO, remainder_info);
#endif


	// do not generate branch for the last unrolling
	if (unrolling < unroll_times && !const_trip) {
#ifdef TARG_ST200
	  /* Override the operations already in body_ops, and put a
             more efficient sequence of code. */
	  INT32 trip_size = TN_size(new_trip_count);
	  OPS_Remove_All(&body_ops);
	  Exp_OP3v(OPC_TRUEBR,
		   NULL,
		   Gen_Label_TN(continuation_label,0),
		   new_trip_count,
		   Gen_Literal_TN(unrolling, trip_size),
		   trip_size == 4 ? V_BR_I4EQ : V_BR_I8EQ,
		   &body_ops);
#endif
	  FOR_ALL_OPS_OPs(&body_ops, op) {
	    OP *new_op = Dup_OP(op);
	    Copy_WN_For_Memory_OP(new_op, op);
	    BB_Append_Op(unrolled_body, new_op);
	  }
	  Link_Pred_Succ_with_Prob(unrolled_body,
				   remainder_tail, 
				   1.0 / (unroll_times - unrolling + 1.0));
	  append_to_prolog(unrolled_body);
	  if (freqs || BB_freq_fb_based(body))
	    BB_freq(unrolled_body)
	      = loop_entry_freq * (unroll_times + 1.0 - unrolling)
	      / (unroll_times + 1.0);
	  unrolled_body = Gen_BB_Like(body);
	}
      }
      if (freqs || BB_freq_fb_based(body))
	body_freq = const_trip ? 
	  BB_freq(CG_LOOP_prolog) : loop_entry_freq / (unroll_times + 1.0);

      body = unrolled_body;

    } else {

      /* Correct loop info for remainder loop.
       */
      INT64 trip_est = (const_trip ? TN_value(new_trip_count)
			: MIN((1 + ntimes) / 2, 2));
      if (info) {
	WN *wn = LOOPINFO_wn(info);
	TYPE_ID ttype = WN_rtype(WN_loop_trip(wn));
	OPCODE opc_intconst = OPCODE_make_op(OPR_INTCONST, ttype, MTYPE_V);
	WN *ntimes_wn = WN_CreateIntconst(opc_intconst, ntimes);
	OPCODE opc_rem = OPCODE_make_op(OPR_REM, ttype, MTYPE_V);
	WN_set_loop_trip(wn,
			 WN_CreateExp2(opc_rem, WN_loop_trip(wn), ntimes_wn));
	WN_loop_trip_est(wn) = trip_est;
	WN_Set_Loop_Unimportant_Misc(wn);
	LOOPINFO_trip_count_tn(info) = new_trip_count;
#ifdef TARG_ST
	// FdF: remove the PRAGMA UNROLL annotation from the remainder loop
	ANNOTATION *unroll_ant = ANNOT_Get(BB_annotations(body), ANNOT_PRAGMA);
	while (unroll_ant && WN_pragma(ANNOT_pragma(unroll_ant)) != WN_PRAGMA_UNROLL)
	  unroll_ant = ANNOT_Get(ANNOT_next(unroll_ant), ANNOT_PRAGMA);
	if (unroll_ant)
	  BB_annotations(body) = ANNOT_Unlink(BB_annotations(body), unroll_ant);
#endif
      }

      OPS body_ops = OPS_EMPTY;
      CGTARG_Generate_Remainder_Branch(new_trip_count, label_tn,
				       &prolog_ops, &body_ops);


#ifdef TARG_ST
      // Append prolog_ops and all the zero_trip_guard_ops, except the
      // branch op, in the original loop prolog. */
      BB_Append_Ops(SAVE_prolog, &prolog_ops);
      OP *guard_op, *next_op;
      for (guard_op = OPS_first(&zero_trip_guard_ops); guard_op; guard_op = next_op) {
	next_op = OP_next(guard_op);
	if (!OP_br(guard_op))
	  BB_Append_Op(SAVE_prolog, guard_op);
	else
	  BB_Append_Op(CG_LOOP_prolog, guard_op);
      }
#else
      BB_Append_Ops(CG_LOOP_prolog, &prolog_ops);
      BB_Append_Ops(CG_LOOP_prolog, &zero_trip_guard_ops);
#endif
      BB_Append_Ops(body, &body_ops);

      if (freqs || BB_freq_fb_based(body)) {
	/* BB_freq(body) doesn't yet include edge from prolog. */
	body_freq = BB_freq(CG_LOOP_prolog) * (trip_est - 1);
	if (freqs && trip_est > 0)
	  BBLIST_prob(BB_succs(body)) = (trip_est - 1.0) / trip_est;
      }

#ifdef TARG_ST
      // [CG]: Add REMAINDERINFO Annotation
      BB_Add_Annotation(body, ANNOT_REMAINDERINFO, remainder_info);
#endif

      if (Get_Trace(TP_CGLOOP, 0x4)) {
	#pragma mips_frequency_hint NEVER
	CG_LOOP_Trace_Loop( loop, "Remainder Loop Structure" );
      }
    }
  }

  /* Splice rest of remainder loop control structure into prolog.
   */
  append_to_prolog(body);
  if (freqs || BB_freq_fb_based(body))
    BB_freq(body) = body_freq;

  // Add remainder_tail, even if the block is empty and has no label,
  // because previous block may terminate in branch if remainder loop
  // was not completely unrolled.
  // INADEQUATE OLD CODE :
  // if (BB_length(remainder_tail) > 0 || BB_has_label(remainder_tail)) {
  append_to_prolog(remainder_tail);
  if (freqs || BB_freq_fb_based(body))
    BB_freq(remainder_tail) = loop_entry_freq;

  /* Add an unrolling note for the assembly listing.
   */
  note_remainder_head(first_remainder_body, ntimes,
		      TN_is_constant(new_trip_count)
		      ? TN_value(new_trip_count) : 0);

#ifdef TARG_ST
  if (remainder_after) {
    CG_LOOP_prolog = SAVE_prolog;
  }
#endif

  // Delete all remainder loop backpatchs
  for (bp = CG_LOOP_Backpatch_First(CG_LOOP_prolog, NULL); bp;
       bp = CG_LOOP_Backpatch_Next(bp))
    CG_LOOP_Backpatch_Delete(CG_LOOP_prolog, bp);
  for (bp = CG_LOOP_Backpatch_First(CG_LOOP_epilog, NULL); bp;
       bp = CG_LOOP_Backpatch_Next(bp))
    CG_LOOP_Backpatch_Delete(CG_LOOP_epilog, bp);

  // Restore main loop backpatches
  prolog_backpatches = main_loop_prolog_backpatches;
  epilog_backpatches = main_loop_epilog_backpatches;

  if (Get_Trace(TP_CGLOOP, 0x4)) {
    #pragma mips_frequency_hint NEVER
    CG_LOOP_Trace_Loop( loop, "After Unroll_Make_Remainder_Loop" );
  }
}

#ifdef TARG_ST
static void
Unroll_Peel_Guard(LOOP_DESCR *loop,
		  LOOPINFO *unrolled_info,
		  BB *guard_bb,
		  TN *trip_count) {

  INT64 trip_est = WN_loop_trip_est(LOOPINFO_wn(unrolled_info));
  float ztrip_prob = 1.0 / MAX(trip_est, 1);
  float orig_post_prolog_freq = BB_freq(BB_next(CG_LOOP_prolog));
  OPS ops = OPS_EMPTY;
  BB *continuation_bb;
  LABEL_IDX continuation_lbl;

  continuation_bb = CG_LOOP_epilog;
  continuation_lbl = Gen_Label_For_BB(continuation_bb);

  INT32 trip_size = TN_size(trip_count);
  Exp_OP3v(OPC_FALSEBR,
	   NULL,
	   Gen_Label_TN(continuation_lbl,0),
	   trip_count,
	   Zero_TN,
	   trip_size == 4 ? V_BR_I4LE : V_BR_I8LE,
	   &ops);

  BB_Append_Ops(guard_bb, &ops);

  Link_Pred_Succ_with_Prob(guard_bb, continuation_bb, ztrip_prob);
  Change_Succ_Prob(guard_bb, BB_next(guard_bb), 1.0 - ztrip_prob);
}

static void
Unroll_Update_Loop_Counter(CG_LOOP &cl) {
  LOOP_DESCR *loop = cl.Loop();
  BB *body = LOOP_DESCR_loophead(loop);

  ANNOTATION *annot = ANNOT_Get(BB_annotations(body), ANNOT_LOOPINFO);
  LOOPINFO *info = ANNOT_loopinfo(annot);
  WN *wn = WN_COPY_Tree(LOOPINFO_wn(info));
  TN *trip_count_tn = LOOPINFO_trip_count_tn(info);

  Is_True(trip_count_tn, ("trip_count_tn should not be NULL"));

  OP *br_op = BB_branch_op(body);
  Is_True(br_op && OP_cond(br_op), ("Missing loop exit branch"));

  /* Now, update the test to use the trip_count_tn. */
  TN *guard_br = OP_opnd(br_op, 0);
  TN *target_br = OP_opnd(br_op, 1);
  INT32 trip_size = TN_size(trip_count_tn);
  TN *trip_count_iv;

  if (TN_is_constant(trip_count_tn))
    trip_count_iv = Gen_Register_TN(ISA_REGISTER_CLASS_integer, TN_size(trip_count_tn));
  else
    trip_count_iv = Build_TN_Like(trip_count_tn);
  Set_TN_is_global_reg(trip_count_iv);

  OPS prolog_ops = OPS_EMPTY;
  Exp_Immediate(trip_count_iv, Gen_Literal_TN(0, trip_size), TRUE, &prolog_ops);
  BB_Append_Ops(CG_LOOP_prolog, &prolog_ops);

  GRA_LIVE_Add_Live_In_GTN(body, trip_count_iv);

  OPS body_ops = OPS_EMPTY;
  Exp_OP2(trip_size == 4 ? OPC_I4ADD : OPC_I8ADD,
	  trip_count_iv,
	  trip_count_iv,
	  Gen_Literal_TN(1, trip_size),
	  &body_ops);
  Exp_OP3v(OPC_TRUEBR,
	   NULL,
	   target_br,
	   trip_count_iv,
	   trip_count_tn,
	   trip_size == 4 ? V_BR_I4NE : V_BR_I8NE,
	   &body_ops);

  BB_Remove_Op(body, br_op);
  BB_Append_Ops(body, &body_ops);

  // Update loop info
  CG_LOOP_DEF tn_def(body);
  OP *op;
  FOR_ALL_OPS_OPs(&body_ops, op) {
    CG_LOOP_Init_Op(op);
    for (INT i = 0; i < OP_results(op); i++) {
      TN *res = OP_result(op,i);
      if (TN_is_register(res) &&
	  !TN_is_const_reg(res)) {
	if (GTN_SET_MemberP(BB_live_in(CG_LOOP_epilog), res)) {
	  Is_True(TN_is_global_reg(res), ("TN in GTN_SET not a global_reg."));

	  if (!TN_is_dedicated(res))
	    CG_LOOP_Backpatch_Add(CG_LOOP_epilog, res, res, 0);

	  if (GTN_SET_MemberP(BB_live_in(body), res))
	    if (!TN_is_dedicated(res))
	      CG_LOOP_Backpatch_Add(CG_LOOP_prolog, res, res, 1);
	}
      }
    }
    for (INT opnd = 0; opnd < OP_opnds(op); opnd++) {
      TN *tn = OP_opnd(op,opnd);
      if (TN_is_register(tn) &&
	  !TN_is_const_reg(tn)) {
	OP *def_op = tn_def.Get(tn);
	// TN is not an invariant and
	// TN is not defined before this OP.
	if (def_op && 
	    !OP_Precedes(def_op, op)) {
	  if ( !CG_LOOP_Backpatch_Find_Non_Body_TN(CG_LOOP_prolog, tn, 1) )
	    if (!TN_is_dedicated(tn))
	      CG_LOOP_Backpatch_Add(CG_LOOP_prolog, tn, tn, 1);
	  Set_OP_omega(op, opnd, 1);
	}
      }
    }
  }
}

/* Insert peel iteration before loop prolog ? */
static void
Unroll_Peel_Loop(CG_LOOP &cl) {

  LOOP_DESCR *loop = cl.Loop();
  BB *body = LOOP_DESCR_loophead(loop);

  ANNOTATION *annot = ANNOT_Get(BB_annotations(body), ANNOT_LOOPINFO);
  LOOPINFO *info = ANNOT_loopinfo(annot);
  WN *wn = WN_COPY_Tree(LOOPINFO_wn(info));
  TN *trip_count_tn = LOOPINFO_trip_count_tn(info);

  /* Check if a zero-trip guard around the peel code is needed. */
  BB *guard_bb = NULL;

  /* Check if a dynamic test on base alignment is needed. */
  BB *check_bb = NULL;
  if (cl.Peel_cond()) {
    check_bb = CG_LOOP_prolog;
    extend_prolog();
  }

  /* Generate the BB for the peel code. */
  BB *peel_bb = CG_LOOP_prolog;
  extend_prolog();

  /* Generate a continuation label. */
  LABEL_IDX continuation_lbl;
  continuation_lbl = Gen_Label_For_BB(CG_LOOP_prolog);

  if (cl.Peel_cond()) {

    /* Requires a check to detect the alignment. */
    TN *base_align = Build_TN_Like(cl.Peel_tn());
    OPS check_ops = OPS_EMPTY;
    Exp_OP2(OPC_U4BAND,
	    base_align,
	    cl.Peel_tn(),
	    Gen_Literal_TN(7, TN_size(base_align)),
	    &check_ops);
    Exp_OP3v(OPC_TRUEBR,
	     NULL,
	     Gen_Label_TN(continuation_lbl,0),
	     base_align,
	     Gen_Literal_TN(cl.Peel_align(), TN_size(base_align)),
	     V_BR_I4EQ,
	     &check_ops);
    BB_Append_Ops(check_bb, &check_ops);

    Link_Pred_Succ_with_Prob(check_bb, CG_LOOP_prolog, 0.5);
    Change_Succ_Prob(check_bb, BB_next(check_bb), 0.5);
  }

  /* Then, replicate one iteration of the loop, and fall-thru to the
     loop prolog. */

  OP *op;
  FOR_ALL_BB_OPs(body, op) {

    if (OP_br(op))
      continue;

    UINT8 opnd;
    UINT8 res;
    OP *new_op = Dup_OP(op);
    Copy_WN_For_Memory_OP(new_op, op);

    BB_Append_Op(peel_bb, new_op);
  }

  /* Then update the trip_count_tn. This will turn a constant
     trip_count_tn into a non constant value if a check for dynamic
     alignment was needed. */
  if (check_bb && TN_is_constant(trip_count_tn)) {
    TN *new_trip_count_tn = Gen_Register_TN(ISA_REGISTER_CLASS_integer, TN_size(trip_count_tn));
    /* add a copy in check_bb */
    OPS ops = OPS_EMPTY;
    Exp_Immediate(new_trip_count_tn, trip_count_tn, TRUE, &ops);
    BB_Prepend_Ops(check_bb, &ops);
    trip_count_tn = new_trip_count_tn;
  }
  
  if (!TN_is_constant(trip_count_tn)) {
    OPS ops = OPS_EMPTY;
    INT32 trip_size = TN_size(trip_count_tn);
    Exp_OP2(trip_size == 4 ? OPC_U4SUB : OPC_U8SUB,
	    trip_count_tn,
	    trip_count_tn,
	    Gen_Literal_TN(1, trip_size),
	    &ops);
    BB_Append_Ops(peel_bb, &ops);
  }
  else
    trip_count_tn = Gen_Literal_TN(TN_value(trip_count_tn)-1, TN_size(trip_count_tn));

  /* Then, regenerate the test at the end of the loop body. */

  LOOPINFO_trip_count_tn(info) = trip_count_tn;
  Unroll_Update_Loop_Counter(cl);
}
#endif

void
CG_LOOP::Unroll_Specialize_Loop() {

  LOOP_DESCR *loop = Loop();
  BB *body = LOOP_DESCR_loophead(loop);

  ANNOTATION *annot = ANNOT_Get(BB_annotations(body), ANNOT_LOOPINFO);
  LOOPINFO *info = ANNOT_loopinfo(annot);
  WN *wn = LOOPINFO_wn(info);
  TN *trip_count_tn = LOOPINFO_trip_count_tn(info);

  /* Insert a BB for alignement check before the loop */

  BB *check_bb = CG_LOOP_prolog;
  extend_prolog();

  BB *special_prolog = CG_LOOP_prolog;

  /* Before the loop epilog, create a goto_bb, a new prolog and a new
     body for the general loop. */

  BB *old_epilog = CG_LOOP_epilog;
  extend_epilog(loop);

  BB *general_body = CG_LOOP_epilog;
  extend_epilog(loop);

  BB *new_prolog = CG_LOOP_epilog;
  extend_epilog(loop);

  Unlink_Pred_Succ(CG_LOOP_epilog, new_prolog);

  // Insert code in check_bb to check that all dynamic aligned streams
  // are correctly aligned.
  TN *or_tn = Gen_Register_TN(ISA_REGISTER_CLASS_integer, 4);
  TN *not_tn = Gen_Register_TN(ISA_REGISTER_CLASS_integer, 4);
  OPS ops = OPS_EMPTY;
  for (int i=0; i < Special_streams(); i++) {
    TN *tn1 = stream_tn[i];
    if (stream_align[i] == 4) {
      Exp_OP1(OPC_I4BNOT,
	      not_tn,
	      tn1,
	      &ops);
      tn1 = not_tn;
    }
    if (i == 0)
      Exp_COPY(or_tn, tn1, &ops);
    else
      Exp_OP2(OPC_U4BIOR,
	      or_tn,
	      or_tn,
	      tn1,
	      &ops);
  }
  Exp_OP2(OPC_U4BAND,
	  or_tn,
	  or_tn,
	  Gen_Literal_TN(4, 4),
	  &ops);

  TN *body_label_tn = Gen_Label_TN(Gen_Label_For_BB(new_prolog),0);
  Exp_OP3v(OPC_TRUEBR,
	   NULL,
	   body_label_tn,
	   or_tn,
	   Zero_TN,
	   V_BR_I4NE,
	   &ops);
  BB_Append_Ops(check_bb, &ops);
  Link_Pred_Succ_with_Prob(check_bb, new_prolog, 0.5);
  Change_Succ_Prob(check_bb, special_prolog, 0.5);

  OP *op;
  FOR_ALL_BB_OPs(body, op) {
    UINT8 opnd;
    UINT8 res;
    OP *new_op = Dup_OP(op);
    CGPREP_Init_Op(new_op);
    CG_LOOP_Init_Op(new_op);
    Copy_WN_For_Memory_OP(new_op, op);

    if (OP_loh(op)) 
      Set_OP_loh(new_op);

    Set_OP_unrolling(new_op, OP_unrolling(op));
    Set_OP_orig_idx(new_op, OP_map_idx(op));
    Set_OP_unroll_bb(new_op, OP_unroll_bb(op));

    for (opnd = 0; opnd < OP_opnds(op); opnd++)
      Set_OP_omega(new_op, opnd, OP_omega(op, opnd));

    BB_Append_Op(general_body, new_op);
  }

  op = BB_branch_op(general_body);
  Set_OP_opnd(op,
	      Branch_Target_Operand(op),
	      Gen_Label_TN(Gen_Label_For_BB(general_body), 0));
  Link_Pred_Succ_with_Prob(general_body, general_body,
			   (WN_loop_trip_est(wn) - 1.0) /
			   WN_loop_trip_est(wn));
  Change_Succ_Prob(general_body, old_epilog, 1.0-((WN_loop_trip_est(wn) - 1.0) / WN_loop_trip_est(wn)));

  // Append a goto to the epilog in goto_bb.
  Add_Goto(CG_LOOP_epilog, old_epilog);
}



void unroll_rename_backpatches(CG_LOOP_BACKPATCH *bpatches, UINT16 n,
			       UINT16 ntimes)
/* -----------------------------------------------------------------------
 * Rename the body TNs and omegas in <bpatches> as if they are uses in
 * the <n>th unrolling out of <ntimes>.
 * -----------------------------------------------------------------------
 */
{
  while (bpatches) {
    TN *body_tn = CG_LOOP_BACKPATCH_body_tn(bpatches);
    UINT16 adjust = CG_LOOP_BACKPATCH_omega(bpatches) + (ntimes - n - 1);
    UINT8 new_omega = adjust / ntimes;
    UINT8 which = ntimes - 1 - (adjust - (new_omega * ntimes));
    TN *new_body_tn = unroll_names_get(body_tn, which);
    CG_LOOP_BACKPATCH_Set_body_tn(bpatches, new_body_tn);
    CG_LOOP_BACKPATCH_Set_omega(bpatches, new_omega);
    bpatches = CG_LOOP_Backpatch_Next(bpatches);
  }
}


void unroll_remove_notations(BB *fully_unrolled_body)
/* -----------------------------------------------------------------------
 * Requires: <fully_unrolled_body> is not a loop since it's been fully
 *	     unrolled (i.e., BB_branch_op(fully_unrolled_body) == NULL)
 *
 * Gets rid of backpatches and nonzero omegas in <fully_unrolled_body>.
 * -----------------------------------------------------------------------
 */
{
  CG_LOOP_BACKPATCH *bp;
  OP *op;

  /* We'll execute at most a single iteration, so each use of a TN with
   * nonzero omega is replaced with the source TN from the appropriate
   * prolog backpatch.
   */
  FOR_ALL_BB_OPs(fully_unrolled_body, op) {
    UINT8 i;
    for (i = 0; i < OP_opnds(op); i++) {
      UINT8 omega = OP_omega(op,i);
      if (omega) {
	TN *old_tn = OP_opnd(op,i);
	TN *new_tn;
#ifdef KEY
	// Dedicated TNs are not backpatched.  Bug 5176.
	if (!TN_is_register(old_tn) || TN_is_dedicated(old_tn))
	  new_tn = old_tn;
	else
#endif
	new_tn = CG_LOOP_Backpatch_Find_Non_Body_TN(CG_LOOP_prolog,
						    old_tn, omega);
	Is_True(new_tn, ("missing prolog backpatch for TN%d[%d]",
			 TN_number(old_tn), omega));
	Set_OP_opnd(op, i, new_tn);
	Set_OP_omega(op, i, 0);
      }
    }
  }

  /* Add copies at the end of <fully_unrolled_body> for the epilog
   * backpatches (but skip self-copies).  Many of these will turn
   * out to be unnecessary, but we'll let copy removal get rid of
   * them.  Remove each epilog backpatch after adding the appropriate
   * copy (if any).
   */

  bp = CG_LOOP_Backpatch_First(CG_LOOP_epilog, NULL);
  while (bp) {
    CG_LOOP_BACKPATCH *next = CG_LOOP_Backpatch_Next(bp);
    TN *src_tn = CG_LOOP_BACKPATCH_body_tn(bp);
    UINT8 omega = CG_LOOP_BACKPATCH_omega(bp);
    TN *dest_tn = CG_LOOP_BACKPATCH_non_body_tn(bp);
    if (omega) {
      TN *new_src_tn = CG_LOOP_Backpatch_Find_Non_Body_TN(CG_LOOP_prolog,
							  src_tn, omega);
      Is_True(new_src_tn, ("missing prolog backpatch for TN%d[%d]",
			   TN_number(src_tn), omega));
      src_tn = new_src_tn;
    }
    if (src_tn != dest_tn) {
      CGPREP_Copy_TN(dest_tn, src_tn, BB_last_op(fully_unrolled_body), 0,
		     FALSE);
    }
    CG_LOOP_Backpatch_Delete(CG_LOOP_epilog, bp);
    bp = next;
  }
  
  /* Get rid of the prolog backpatches.  We used them in the last two
   * steps, so we weren't able to get rid of them until now.
   */
  bp = CG_LOOP_Backpatch_First(CG_LOOP_prolog, NULL);
  while (bp) {
    CG_LOOP_BACKPATCH *next = CG_LOOP_Backpatch_Next(bp);
    CG_LOOP_Backpatch_Delete(CG_LOOP_prolog, bp);
    bp = next;
  }
}

#ifdef TARG_ST
static BOOL
Get_pragma_LoopMod(LOOP_DESCR *loop, int *modulus, int *residue)
{
  *modulus = 1;
  *residue = 0;

  if (!CG_LOOP_ignore_pragmas) {
    BB *head = LOOP_DESCR_loophead(loop);
    ANNOTATION *loopmod_ant = ANNOT_Get(BB_annotations(head), ANNOT_PRAGMA);
    while (loopmod_ant && WN_pragma(ANNOT_pragma(loopmod_ant)) != WN_PRAGMA_LOOPMOD)
      loopmod_ant = ANNOT_Get(ANNOT_next(loopmod_ant), ANNOT_PRAGMA);
    if (loopmod_ant) {
      WN *wn = ANNOT_pragma(loopmod_ant);
      *modulus = MAX(1, WN_pragma_arg1(wn));
      *residue = MAX(0, WN_pragma_arg2(wn));
    }
  }
  return (*modulus > 1);
}
#endif


static void trace_loop_cflow(LOOP_DESCR *loop, const char *prefix)
{
  BB *bb;

  FOR_ALL_BB_SET_members(LOOP_DESCR_bbset(loop), bb) {
    BBLIST *bbl;
    fprintf(TFile, "%sBB:%d  preds = { ", prefix, BB_id(bb));
    FOR_ALL_BB_PREDS(bb, bbl)
      fprintf(TFile, "BB:%d ", BB_id(BBLIST_item(bbl)));
    fprintf(TFile, "}  succs = { ");
    FOR_ALL_BB_SUCCS(bb, bbl)
      fprintf(TFile, "BB:%d ", BB_id(BBLIST_item(bbl)));
    fprintf(TFile, "}\n");
  }
}


static BOOL sort_topologically(LOOP_DESCR *loop, BB **result)
/* -----------------------------------------------------------------------
 * Fill in <result> so that it contains a topological ordering of the BBs
 * in <loop>.  Return TRUE if done, or FALSE if not possible (i.e., loop
 * has irreducible flowgraph).
 * -----------------------------------------------------------------------
 */
{
  BB_SET *bbs = LOOP_DESCR_bbset(loop);
  UINT32 num_bbs = BB_SET_Size(bbs);
  BB_MAP topo_map = BB_Topological_Map(bbs, LOOP_DESCR_loophead(loop));
  BB *bb;

  FOR_ALL_BB_SET_members(bbs, bb) {
    INT32 i = BB_MAP32_Get(topo_map, bb);
    Is_True(i >= 0 && i <= num_bbs, ("bad <topo_map> value"));
    if (i == 0) {
      BB_MAP_Delete(topo_map);
      return FALSE;
    }
    result[i-1] = bb;
  }

  if (Get_Trace(TP_CGLOOP, 2)) {
    UINT32 bbi;
    fprintf(TFile, "<unroll> topological sort of loop BBs:");
    for (bbi = 0; bbi < num_bbs; bbi++)
      fprintf(TFile, " %d", BB_id(result[bbi]));
    fprintf(TFile, "\n");
  }

  BB_MAP_Delete(topo_map);

  return TRUE;
}

#ifdef TARG_ST
static BOOL unroll_multi_make_remainder_loop(LOOP_DESCR *loop, UINT8 ntimes,
					     BB **topo_vec, UINT32 num_bbs, INT32 remainder_val)
#else
static BOOL unroll_multi_make_remainder_loop(LOOP_DESCR *loop, UINT8 ntimes,
					     BB **topo_vec, UINT32 num_bbs)
#endif
/* -----------------------------------------------------------------------
 * Requires: CG_LOOP_Trip_Count(loop) != NULL &&
 *
 *           Since <loop> is trip-countable, it has a unique loop tail,
 *	     the last entry in <topo_vec> that is the only loop exit.
 *	     Otherwise it's not really trip-countable since it's possible
 *	     to execute a partial iteration (or else the impossible exits
 *	     should be removed). &&
 *
 *	     BB_freq(bb) for all <bb> in <loop> has the same value it did
 *	     before unrolling started.
 *
 * TURN LOOP_DESCR_bbset(loop) into a "remainder" loop in preparation
 * for a version being unrolled <ntimes>.  The remainder loop executes
 * <trip_count> % <ntimes> iterations.  If the remainder loop will execute
 * less than two times, we remove the ending branch, so it's not really a
 * loop.  Returns FALSE if a remainder loop isn't necessary, TRUE otherwise.
 *
 * TODO: Add LOOP_DESCR for remainder loop?
 * -----------------------------------------------------------------------
 */
{
  BB *head = LOOP_DESCR_loophead(loop), *tail = topo_vec[num_bbs-1];
  ANNOTATION *annot = ANNOT_Get(BB_annotations(head), ANNOT_LOOPINFO);
  LOOPINFO *info = ANNOT_loopinfo(annot);
  TN *trip_count = LOOPINFO_trip_count_tn(info), *trip_counter;
  TN *new_trip_count = TN_is_constant(trip_count) ?
    Gen_Literal_TN(TN_value(trip_count) % ntimes, 4) : Build_TN_Like(trip_count);
  OP *op, *backedge_br_op = BB_branch_op(tail);
  BOOL freqs = FREQ_Frequencies_Computed();
  float ztrip_prob = !freqs || TN_is_constant(trip_count) ? 0.0 : 1.0 / ntimes;
  INT64 orig_trip_est = WN_loop_trip_est(LOOPINFO_wn(info));
  INT64 trip_est = TN_is_constant(new_trip_count) ?
    TN_value(new_trip_count) : MIN((1 + ntimes) / 2, 2);
  float orig_prolog_freq = BB_freq(CG_LOOP_prolog);
  float orig_post_prolog_freq = BB_freq(BB_next(CG_LOOP_prolog));
  float freq_factor = (1.0 - ztrip_prob) * trip_est / MAX(orig_trip_est, 1);
  float fts_freq, head_freq;
  BB *remainder_epilog = NULL, *fts, *orig_prolog = CG_LOOP_prolog;
  INT16 dbnum = 0;
  UINT32 bbi;

  if (TN_is_constant(new_trip_count) && TN_value(new_trip_count) % ntimes == 0)
    return FALSE;

  Is_True(CG_LOOP_epilog != NULL,
	    ("remainder loop generation requires non-NULL CG_LOOP_epilog"));
  Is_True(BB_unrollings(head) < 2,
	    ("unrolled loophead passed to unroll_multi_make_remainder_loop"));
  Is_True(topo_vec[0] == head, ("<head> not first in <topo_vec>"));
  Is_True(LOOP_DESCR_Find_Unique_Tail(loop), 
	    ("<loop> has no unique tail"));
  Is_True(tail == LOOP_DESCR_Find_Unique_Tail(loop),
	    ("<loop> tail not last in <topo_vec>"));
  
  /* If trip count is a constant, see how many times the remainder
   * loop will execute (if at all) before proceeding.  If it won't
   * execute at all, don't make a remainder loop.  Otherwise, create
   * a constant TN for the new trip count.
   */
  if (TN_is_constant(trip_count)) {
    INT16 new_trip_count_val = TN_value(trip_count) % ntimes;
    if (new_trip_count_val < 0)
      DevWarn("unroll_multi_make_remainder_loop: trip count is negative");
    new_trip_count = Gen_Literal_TN(new_trip_count_val, 4);

#ifdef TARG_ST
  } else if (remainder_val != -1) {
    new_trip_count = Gen_Literal_TN(remainder_val, 4);
#endif

  } else {
    /* Add zero-trip guard for remainder loop:
     *   if (trip_count % ntimes <= 0) skip remainder
     * We know <trip_count>'s value is positive, and <ntimes> is
     * a power of two, so we can perform this with:
     *   <new_trip_count> <- andi <trip_count> <ntimes>-1
     *   beq continuation_label <new_trip_count> 0
     */
    LABEL_IDX continuation_label;
    INT32 trip_size = TN_size(trip_count);
    OPS ops = OPS_EMPTY;
    remainder_epilog = Gen_BB_Like(CG_LOOP_prolog);
    if (BB_freq_fb_based(CG_LOOP_prolog))
      Set_BB_freq_fb_based(remainder_epilog);
    continuation_label = Gen_Label_For_BB(remainder_epilog);
    Is_True(is_power_of_two(ntimes), ("unroll amount not power of two"));
    new_trip_count = Build_TN_Like(trip_count);
    Exp_OP2(trip_size == 4 ? OPC_U4BAND : OPC_U8BAND,
	    new_trip_count,
	    trip_count,
	    Gen_Literal_TN(ntimes-1, trip_size),
	    &ops);
#ifdef TARG_ST
    Exp_OP3v(OPC_FALSEBR,
	     NULL,
	     Gen_Label_TN(continuation_label,0),
	     new_trip_count,
	     Zero_TN,
	     trip_size == 4 ? V_BR_I4EQ : V_BR_I8EQ,
	     &ops);
#else
    Exp_OP3v(OPC_FALSEBR,
	     NULL,
	     Gen_Label_TN(continuation_label,0),
	     new_trip_count,
	     Zero_TN,
	     V_BR_I8EQ,
	     &ops);
#endif
    BB_Append_Ops(CG_LOOP_prolog, &ops);
    Link_Pred_Succ_with_Prob(CG_LOOP_prolog, remainder_epilog, ztrip_prob);
    if (freqs)
      Change_Succ_Prob(CG_LOOP_prolog, BB_next(CG_LOOP_prolog),
		       1.0 - ztrip_prob);
  }

  /* Append loop BBs in topological order.  It shouldn't be necessary to
   * insert branches since there's a single loop tail/exit block.  Set
   * BB_freqs by multiplying their pre-unrolling values by <freq_factor>,
   * which compensates for the zero-trip guard and new trip count.
   */
  fts = BB_Fall_Thru_Successor(CG_LOOP_prolog);
  Is_True(fts, ("CG_LOOP_prolog has no fall-thru successor"));
  fts_freq = BB_freq(fts);
  head_freq = BB_freq(head);
  Change_Succ(CG_LOOP_prolog, fts, head);
  BB_freq(fts) = fts_freq;
  BB_freq(head) = head_freq;
  fts = BB_Fall_Thru_Successor(tail);
  FmtAssert(fts,
	    /* This indicates that the loop isn't really trip-countable. */
	    ("trip-countable loop (line %d) tail has no fall-thru successor",
	     BB_Loop_Lineno(head)));
  Unlink_Pred_Succ(tail, fts);
  for (bbi = 0; bbi < num_bbs; bbi++) {
    BB *loop_bb = topo_vec[bbi];
    BB *post_prolog = BB_next(CG_LOOP_prolog);
    BB *prev = BB_prev(loop_bb);
    BB *next = BB_next(loop_bb);
    RID *rid = BB_rid(loop_bb);
    BB *fall_thru = BB_Fall_Thru_Successor(loop_bb);
    Is_True(BB_SET_MemberP(LOOP_DESCR_bbset(loop), loop_bb),
	      ("topo_vec[%d] = BB:%d not in LOOP_DESCR_bbset",
	       bbi, BB_id(loop_bb)));
    if (prev && BB_next(prev) == loop_bb) BB_next(prev) = next;
    if (next && BB_prev(next) == loop_bb) BB_prev(next) = prev;
    BB_prev(loop_bb) = BB_next(loop_bb) = NULL;
    if (REGION_First_BB == loop_bb) REGION_First_BB = next;
    if (rid && RID_cginfo(rid)) {
      /* update cgrin pointers */
      CGRIN *cgrin = RID_cginfo(rid);
	if (CGRIN_first_bb(cgrin) == loop_bb) CGRIN_first_bb(cgrin) = next;
      if (CGRIN_last_bb(cgrin) == loop_bb) CGRIN_last_bb(cgrin) = prev;
    }
    Chain_BBs(CG_LOOP_prolog, loop_bb);
    Chain_BBs(loop_bb, post_prolog);
    BB_freq(loop_bb) *= freq_factor;
    CG_LOOP_prolog = loop_bb;
    if (bbi != num_bbs -1 && fall_thru && fall_thru != topo_vec[bbi+1]) {
      /* Insert fall-thru BB that branches to <fall_thru> */
      BB *new_bb = Gen_And_Insert_BB_After(loop_bb);
      BB_rid(new_bb) = BB_rid(loop_bb);
      if (BB_freq_fb_based(loop_bb)) Set_BB_freq_fb_based(new_bb);
      Add_Goto(new_bb, fall_thru);
      Change_Succ(loop_bb, fall_thru, new_bb);
      if (freqs || BB_freq_fb_based(fall_thru))
	BB_freq(fall_thru) += BB_freq(loop_bb);
      if (BB_SET_MemberP(LOOP_DESCR_bbset(loop), fall_thru))
	Set_BB_loop_head_bb(new_bb, head);
      CG_LOOP_prolog = new_bb;
    }
  }

  /* Remove the branch at the end of <tail>, remembering the dbnum
   * (if any) for later use if we're going to replace it with
   * another branch.
   */
  if (backedge_br_op) {
    if (Is_DB_OP_Init(backedge_br_op))
      dbnum = OP_dbnum(backedge_br_op);
    BB_Remove_Op(tail, backedge_br_op);
    Unlink_Pred_Succ(tail, head);
  }
  
  if (ntimes == 2 ||
      TN_is_constant(new_trip_count) && TN_value(new_trip_count) < 2) {
    /*
     * Remainder isn't really a loop, so remove the LOOPINFO annotation.
     */
    BB_annotations(head) = ANNOT_Unlink(BB_annotations(head), annot);
    for (bbi = 0; bbi < num_bbs; bbi++)
      Set_BB_loop_head_bb(topo_vec[bbi], NULL);
    Link_Pred_Succ_with_Prob(tail, BB_next(tail), 1.0);

  } else {

    /* Correct loop info for remainder loop.
     */
    OPS ops = OPS_EMPTY;
    INT32 trip_size = TN_size(new_trip_count);
    WN *wn = LOOPINFO_wn(info);
    TYPE_ID ttype = WN_rtype(WN_loop_trip(wn));
    OPCODE opc_intconst = OPCODE_make_op(OPR_INTCONST, ttype, MTYPE_V);
    WN *ntimes_wn = WN_CreateIntconst(opc_intconst, ntimes);
    OPCODE opc_rem = OPCODE_make_op(OPR_REM, ttype, MTYPE_V);
    float backedge_prob = (trip_est - 1.0) / trip_est;
    WN_set_loop_trip(wn,
		     WN_CreateExp2(opc_rem, WN_loop_trip(wn), ntimes_wn));
    WN_loop_trip_est(wn) = trip_est;
    WN_Set_Loop_Unimportant_Misc(wn);
    LOOPINFO_trip_count_tn(info) = new_trip_count;

    /*
     * Modify actual trip count of remainder loop:
     *   Append to prolog (if <new_trip_count> is constant, otherwise
     *     <trip_counter> is <new_trip_count>):
     *	   <trip_counter> <- <new_trip_count>
     *   Replace tail branch OP with:
     *	   <trip_counter> <- [d]addi <trip_counter> -1
     *	   bne <head> <trip_counter> $0
     */
    if (TN_is_constant(new_trip_count)) {
      OPS copy_ops = OPS_EMPTY;
      trip_counter = CGTARG_gen_trip_count_TN(trip_size);
      //      trip_counter = Gen_Register_TN (ISA_REGISTER_CLASS_integer, trip_size);
#ifdef TARG_ST
      Exp_Immediate(trip_counter, new_trip_count, TRUE, &copy_ops);
#else
      Exp_COPY(trip_counter, new_trip_count, &copy_ops);
#endif
      BB_Append_Ops(orig_prolog, &copy_ops);
    } else {
      trip_counter = new_trip_count;
    }
    Exp_OP2(trip_size == 4 ? OPC_I4ADD : OPC_I8ADD,
	    trip_counter,
	    trip_counter,
	    Gen_Literal_TN(-1, trip_size),
	    &ops);
    Exp_OP3v(OPC_TRUEBR,
	     NULL,
	     Gen_Label_TN(Gen_Label_For_BB(head), 0),
	     trip_counter,
	     Zero_TN,
	     trip_size == 4 ? V_BR_I4NE : V_BR_I8NE,
	     &ops);
    BB_Append_Ops(tail, &ops);
    if (dbnum > 0) {
      FOR_ALL_OPS_OPs(&ops, op) DB_Initialize_OP(op, dbnum);
    }
    Link_Pred_Succ_with_Prob(tail, head, backedge_prob);
    Link_Pred_Succ_with_Prob(tail, BB_next(tail), 1.0 - backedge_prob);
  }
  
  /* Attach <remainder_epilog> to prolog if a zero-trip guard was
   * generated.  We'll branch here when not executing the remainder
   * loop.
   */
  if (remainder_epilog) {
    append_to_prolog(remainder_epilog);
    BB_freq(remainder_epilog) = orig_prolog_freq;
  }
  BB_freq(BB_next(CG_LOOP_prolog)) = orig_post_prolog_freq;

  /* Add an unrolling note for the assembly listing.
   */
  note_remainder_head(head, ntimes, TN_is_constant(new_trip_count) ?
		      TN_value(new_trip_count) : 0);

  return TRUE;
}

static BOOL unroll_multi_bb(LOOP_DESCR *loop, UINT8 ntimes)
/* -----------------------------------------------------------------------
 * Requires: ntimes > 1
 *
 * Unroll <loop> <ntimes>.  Return TRUE if successful, FALSE otherwise.
 * -----------------------------------------------------------------------
 */
{
  BB *head = LOOP_DESCR_loophead(loop);
  UINT32 num_bbs = BB_SET_Size(LOOP_DESCR_bbset(loop));
  BB *replicas;
  BB_SET *new_bbs;
  BB **orig_bbs, **orig_br_targ_bbs, **orig_fall_thru_bbs, *bb, *replica;
  float *orig_br_probs;
  float orig_head_freq = BB_freq(head);
  BB_MAP orig_bb_index_map;
  UINT32 bbi, i, unrolling;
  BOOL unrolling_fully = FALSE;
  ANNOTATION *annot = ANNOT_Get(BB_annotations(head), ANNOT_LOOPINFO);
  LOOPINFO *info = annot ? ANNOT_loopinfo(annot) : NULL;
  TN *trip_count_tn = info ? LOOPINFO_trip_count_tn(info) : NULL;
  LOOPINFO *unrolled_info = NULL;
  BOOL freqs = FREQ_Frequencies_Computed();
  BOOL gen_remainder_loop = trip_count_tn && is_power_of_two(ntimes) &&
    CG_LOOP_unroll_multi_make_remainder_loop;
  UINT32 removed_exits = 0;
#ifdef TARG_ST
    INT16 remainder_trip_count_val = -1;
#endif

  MEM_POOL_Push(&MEM_local_nz_pool);
  orig_bbs = TYPE_MEM_POOL_ALLOC_N(BB *, &MEM_local_nz_pool, num_bbs);
  if (!sort_topologically(loop, orig_bbs)) {
    char *reason = "loop has irreducible flow graph";
    MEM_POOL_Pop(&MEM_local_nz_pool);
    if (Get_Trace(TP_CGLOOP, 2))
	fprintf(TFile, "<unroll> aborting; %s\n", reason);
    note_not_unrolled(head, reason);
    return FALSE;
  }

  replicas = Gen_BB_N(num_bbs * ntimes);
  if (info) {
    WN *wn = WN_COPY_Tree(LOOPINFO_wn(info));
    unrolled_info = TYPE_P_ALLOC(LOOPINFO);
    LOOPINFO_wn(unrolled_info) = wn;
    WN_loop_trip_est(wn) /= ntimes;
    LOOPINFO_srcpos(unrolled_info) = LOOPINFO_srcpos(info);
    if (trip_count_tn) {
      INT16 new_trip_count_val;
      TYPE_ID ttype = WN_rtype(WN_loop_trip(wn));
      OPCODE opc_intconst = OPCODE_make_op(OPR_INTCONST, ttype, MTYPE_V);
      WN *ntimes_wn = WN_CreateIntconst(opc_intconst, ntimes);
      OPCODE opc_div = OPCODE_make_op(OPR_DIV, ttype, MTYPE_V);
      if (TN_is_constant(trip_count_tn))
	new_trip_count_val = TN_value(trip_count_tn) / ntimes;
      unrolling_fully = CG_LOOP_unroll_fully &&
	TN_is_constant(trip_count_tn) && TN_value(trip_count_tn) <= ntimes;
      WN_set_loop_trip(wn, WN_CreateExp2(opc_div, WN_loop_trip(wn),
					 ntimes_wn));
      if (TN_is_constant(trip_count_tn))
	LOOPINFO_trip_count_tn(unrolled_info) =
	  Gen_Literal_TN(new_trip_count_val, TN_size(trip_count_tn));
    }
  }

  /* Attach new info to head of unrolled body */
  if (unrolled_info)
    BB_Add_Annotation(&replicas[0], ANNOT_LOOPINFO, unrolled_info);
  Set_BB_unrollings(&replicas[0], ntimes);

  /* Setup some data structures, such that, for all 0 <= bbi < num_bbs:
   *   BB_MAP32_Get(orig_bb_index_map, orig_bbs[bbi]) is <bbi> + 1
   *	 (we add 1 to distinguish index 0 from "uninitialized"; this
   *	 is much quicker than checking for membership in BB_SET).
   *   orig_br_targ_bbs[bbi] is the explicit branch target (if any) of
   *     orig_bbs[bbi].  NULL indicates no branch or an indirect branch.
   *   orig_fall_thru_bbs[bbi] is the fall-through successor of orig_bbs[bbi].
   *	 NULL indicates an unconditional branch or an indirect branch.
   */
  orig_bb_index_map = BB_MAP32_Create();
  orig_br_targ_bbs = TYPE_MEM_POOL_ALLOC_N(BB *, &MEM_local_nz_pool, num_bbs);
  orig_br_probs = TYPE_MEM_POOL_ALLOC_N(float, &MEM_local_nz_pool, num_bbs);
  orig_fall_thru_bbs = TYPE_MEM_POOL_ALLOC_N(BB *, &MEM_local_nz_pool,num_bbs);
  for (bbi = 0; bbi < num_bbs; bbi++) {
    BB *orig_bb = orig_bbs[bbi];
    BBLIST *fts_list = BBlist_Fall_Thru_Succ(orig_bb);
    BB *fall_thru = fts_list ? BBLIST_item(fts_list) : NULL;
    float fall_thru_prob = fts_list ? BBLIST_prob(fts_list) : 0.0;
    OP *br_op = BB_branch_op(orig_bb);
    BB *br_targ = NULL;
    float br_prob = 0.0;
    if (br_op) {
      INT br_targ_opnd = Branch_Target_Operand(br_op);
      if (TN_is_label(OP_opnd(br_op, br_targ_opnd))) {
	BBLIST *tlist = (fts_list == BB_succs(orig_bb)) ?
	  BBLIST_next(BB_succs(orig_bb)) : BB_succs(orig_bb);
	LABEL_IDX label = TN_label(OP_opnd(br_op, br_targ_opnd));
	Is_True(tlist, ("BB_succs(BB:%d) missing succ labelled %s",
			BB_id(orig_bb), LABEL_name(label)));
	br_targ = BBLIST_item(tlist);
	Is_True(Is_Label_For_BB(label, br_targ),
		 ("BB_succs(BB:%d) has succ BB:%d, expected one labelled %s",
		 BB_id(orig_bb), BB_id(br_targ), ST_name(label)));
	br_prob = freqs ? BBLIST_prob(tlist) : (fall_thru ? 0.5 : 1.0);
      }
    }
    if (CG_warn_bad_freqs && freqs && br_targ &&
	!FREQ_Match(1.0, br_prob + fall_thru_prob))
      DevWarn("for BB:%d, fall_thru_prob + br_prob != 1.0", BB_id(orig_bb));
    BB_MAP32_Set(orig_bb_index_map, orig_bb, bbi+1);
    orig_br_targ_bbs[bbi] = br_targ;
    orig_br_probs[bbi] = br_prob;
    orig_fall_thru_bbs[bbi] = fall_thru;
  }

  /* Splice the replicas into the old BB chain.  The old loop BBs are
   * intentionally left in the chain since we use their prev/next/pred/succ
   * info while unrolling.
   */
  new_bbs = BB_SET_Create_Empty(PU_BB_Count, &MEM_local_nz_pool);
  Chain_BBs(BB_prev(head), &replicas[0]);
  new_bbs = BB_SET_Union1D(new_bbs, &replicas[0], &MEM_local_nz_pool);
  for (i = 1; i < num_bbs * ntimes; i++) {
    new_bbs = BB_SET_Union1D(new_bbs, &replicas[i], &MEM_local_nz_pool);
    Chain_BBs(&replicas[i-1], &replicas[i]);
  }
  Chain_BBs(&replicas[i-1], head);

  /* Retarget non-loop preds of head to unrolled head. */
  LOOP_DESCR_Retarget_Loop_Entrances(loop, &replicas[0]);

  if (freqs || BB_freq_fb_based(head)) {
    /*
     * Compute frequency of unrolled head.  We'll use this to derive
     * the frequencies of the other replicas as we unroll.
     *
     * TODO: When we emit a remainder loop for trip-countable loops,
     *	     BB_freq(&replicas[0]) should be set to BB_freq(prolog) *
     *	     WN_loop_trip_est(LOOPINFO_wn(unrolled_info)).  The rounding
     *	     down of the trip_est to an integer correctly subtracts the
     *	     portion of the frequency that goes through the remainder loop.
     */
    if (info) {
      INT64 orig_trip_est = WN_loop_trip_est(LOOPINFO_wn(info));
      if (CG_warn_bad_freqs &&
	  !FREQ_Match(orig_head_freq,
		      orig_trip_est * BB_freq(CG_LOOP_prolog)))
	DevWarn("BB_freq(orig head BB:%d) != BB_freq(prolog BB:%d) * trip_est",
		BB_id(head), BB_id(CG_LOOP_prolog));
    }
    BB_freq(&replicas[0]) = gen_remainder_loop ?
      BB_freq(CG_LOOP_prolog) * WN_loop_trip_est(LOOPINFO_wn(unrolled_info)) :
      orig_head_freq / ntimes;
  }
#ifdef TARG_ST
  BB *tail = BB_Other_Predecessor(head, CG_LOOP_prolog);
  if (tail != orig_bbs[num_bbs-1]) {
    if (Get_Trace(TP_CGLOOP, 2))
      fprintf(TFile, "unroll_multi_bb: Could not find a correct tail for the loop.\n");
    tail = NULL;
  }
#endif

  /* Build the replicas.
   */
  replica = &replicas[0];
  for (unrolling = 0; unrolling < ntimes; unrolling++) {
    for (bbi = 0; bbi < num_bbs; bbi++, replica++) {
      BB *orig_bb = orig_bbs[bbi];
      BB *br_targ = orig_br_targ_bbs[bbi];
      float br_prob = orig_br_probs[bbi];
      BB *fall_thru_dest = orig_fall_thru_bbs[bbi];
      OP *op, *replica_br_op = NULL;
      BOOL fall_thru_dest_in_loop;

#ifdef TARG_ST
      /* Initialize the TN renamer */
      unroll_names_multibb_init(orig_bb, &MEM_phase_nz_pool);
#endif

      /* Initialize BB info in <replica> */
      Set_BB_loop_head_bb(replica, &replicas[0]);
      Set_BB_unrollings(replica, ntimes);
      if (unrolling_fully) Set_BB_unrolled_fully(replica);
      BB_rid(replica) = BB_rid(head);
      if (BB_freq_fb_based(orig_bb)) Set_BB_freq_fb_based(replica);

      /* Replicate OPs from <orig_bb> into <replica>, renaming TNs as we go
       */
      FOR_ALL_BB_OPs(orig_bb, op) {
#ifdef TARG_ST
	// Perform Prefetch pruning at unroll time
	if (OP_prefetch(op)) {

	  if (!tail || !BB_SET_MemberP(BB_dom_set(tail), orig_bb)) {
	    if (Get_Trace(TP_CGLOOP, 2))
	      fprintf(TFile, "unroll_multi_bb: PREFETCH is not executed in all iterations of the loop.\n");
	    continue;
	  }

	  WN *mem_wn = Get_WN_From_Memory_OP(op);
	  Is_True(!mem_wn || WN_operator(mem_wn) == OPR_PREFETCH,
		  ("wrong prefetch WHIRL node."));

	  if (Get_Trace(TP_CGLOOP, 2) && unrolling == 0)  // trace once per loop
	    if (mem_wn)
	      fprintf(TFile, "<cgpref> - 1L cache stride = %d, 2L cache stride = %d,"
		      " confidence = %d\n",
		      WN_pf_stride_1L(mem_wn),
		      WN_pf_stride_2L(mem_wn),
		      WN_pf_confidence(mem_wn));
	    else
	      fprintf(TFile, "<cgpref> pref wn not found.\n");

	  if (mem_wn && Prefetch_Kind_Enabled(mem_wn)) {
	    int stride = WN_pf_stride_2L( mem_wn ) ?  WN_pf_stride_2L( mem_wn ) :  WN_pf_stride_1L(mem_wn);
	    if (stride != 0 && (unrolling % stride) != 0) {
	      if (Get_Trace(TP_CGLOOP, 2))
		fprintf(TFile, "<cgpref> pref pruned at unrolling %d.\n", unrolling);
	      continue;
	    }
	  }
	}
#endif
	OP *rop;
	UINT8 opi;
	UINT8 resi;
	rop = Dup_OP(op);
	Set_OP_unrolling(rop, unrolling);
	Set_OP_orig_idx(rop, OP_map_idx(op));
	Set_OP_unroll_bb(rop, replica);
	Copy_WN_For_Memory_OP(rop, op);
        for (resi = 0; resi < OP_results(rop); resi++) {
	  TN *res = OP_result(rop,resi);
	  if (!TN_is_global_reg(res)) {
	    TN *new_res = unroll_names_get(res, 0);
	    Set_OP_result(rop, resi, new_res);
	  }
	}
	for (opi = 0; opi < OP_opnds(rop); opi++) {
	  TN *opnd = OP_opnd(rop, opi);
	  if (TN_is_register(opnd)) {
	    if (!TN_is_global_reg(opnd)) {
	      Set_OP_opnd(rop, opi, unroll_names_get(opnd, 0));
	    }
	  }
	}
	BB_Append_Op(replica, rop);
	if (OP_br(rop)) replica_br_op = rop;
      }
      Is_True(br_targ == NULL || replica_br_op, ("no replica branch op"));
      unroll_xfer_annotations(replica, orig_bb);

      /* Retarget <fall_thru_dest> if it's in loop */
      if (fall_thru_dest) {
	INT32 ftd_bbi = BB_MAP32_Get(orig_bb_index_map, fall_thru_dest) - 1;
	fall_thru_dest_in_loop = ftd_bbi >= 0;
	if (fall_thru_dest_in_loop) {
	  /* If <ftd_bbi> is zero, want loop head in next unrolling */
	  UINT32 iter = ftd_bbi > 0 ? unrolling : (unrolling + 1) % ntimes;
	  fall_thru_dest = &replicas[iter * num_bbs + ftd_bbi];
	}
      }

      /* Retarget intra-loop branches */
      if (replica_br_op) {
	INT replica_targ_opnd = Branch_Target_Operand(replica_br_op);
	if (br_targ == NULL) {	/* Indirect branch */
	  BBLIST *succs;
	  Is_True(!TN_is_label(OP_opnd(replica_br_op, replica_targ_opnd)),
		    ("expected indirect branch"));
	  FOR_ALL_BB_SUCCS(orig_bb, succs) {
	    BB *succ = BBLIST_item(succs);
	    Is_True(!BB_SET_MemberP(LOOP_DESCR_bbset(loop), succ),
		      /* CG_LOOP_Unroll should filter these out */
		      ("not yet retargeting intra-loop indirect branches"));
	    Link_Pred_Succ_with_Prob(replica, succ, BBLIST_prob(succs));
	  }
	} else {
	  INT32 bt_bbi = BB_MAP32_Get(orig_bb_index_map, br_targ) - 1;
	  BOOL br_targ_in_loop = bt_bbi >= 0;
	  if (br_targ_in_loop) {
	    /* If <bt_bbi> is zero, want loop head in next unrolling */
	    UINT32 iter = bt_bbi > 0 ? unrolling : (unrolling + 1) % ntimes;
	    br_targ = &replicas[iter * num_bbs + bt_bbi];
	  }
	  if (unrolling_fully && br_targ == &replicas[0]) {
	    /* No need for branch from tail to head if unrolling fully */
	    BB_Remove_Op(replica, replica_br_op);
	    br_prob = 0.0;
	  } else if (br_targ == BB_next(replica)) {
	    /*
	     * <br_targ> is next.  To minimize branching within the
	     * loop, we'll reverse this branch to target <fall_thru_dest>
	     * (or remove it if there's no <fall_thru_dest> or we know it's
	     * always taken), and let <replica> fall through to <br_targ>.
	     */
	    if (gen_remainder_loop && bt_bbi == 0) {
	      /* Remove always-taken branch to next iter head */
	      BB_Remove_Op(replica, replica_br_op);
	      br_prob = 0.0;
	    } else if (fall_thru_dest && fall_thru_dest != br_targ) {
	      if (Negate_Branch(replica_br_op)) {
		Set_OP_opnd(replica_br_op, replica_targ_opnd,
			    Gen_Label_TN(Gen_Label_For_BB(fall_thru_dest), 0));
		br_prob = 1.0 - br_prob;
		Link_Pred_Succ_with_Prob(replica, fall_thru_dest, br_prob);
		if (fall_thru_dest_in_loop && fall_thru_dest != &replicas[0] &&
		    (freqs || BB_freq_fb_based(fall_thru_dest)))
		  BB_freq(fall_thru_dest) += br_prob * BB_freq(replica);
	      } else {
		#pragma mips_frequency_hint NEVER
		DevWarn("unable to negate branch %s", TOP_Name(OP_code(replica_br_op)));
	      }
	    } else {
	      BB_Remove_Op(replica, replica_br_op);
	      br_prob = 0.0;
	    }
	    fall_thru_dest = br_targ;
	    fall_thru_dest_in_loop = br_targ_in_loop;
	  } else if (br_targ_in_loop) {
	    /*
	     * Retarget internal branch to <br_targ>.
	     */
	    Set_OP_opnd(replica_br_op, replica_targ_opnd,
			Gen_Label_TN(Gen_Label_For_BB(br_targ), 0));
	    Link_Pred_Succ_with_Prob(replica, br_targ, br_prob);
	    if (br_targ != &replicas[0] &&
		(freqs || BB_freq_fb_based(br_targ)))
	      BB_freq(br_targ) += br_prob * BB_freq(replica);
	  } else if (br_targ) {
	    /*
	     * Branch to external target - no change.
	     */
	    Link_Pred_Succ_with_Prob(replica, br_targ, br_prob);
	  }
	}
      }

      /* If non-nil <fall_thru_dest> isn't next in the BB chain,
       * insert a fall-through block that branches directly to it.
       */
      if (fall_thru_dest) {
	float ft_prob = 1.0 - br_prob;
	if (BB_next(replica) != fall_thru_dest) {
	  BB *new_bb = Gen_And_Insert_BB_After(replica);
	  BB_rid(new_bb) = BB_rid(head);
	  if (BB_freq_fb_based(replica)) Set_BB_freq_fb_based(new_bb);
	  if (freqs || BB_freq_fb_based(new_bb))
	    BB_freq(new_bb) = ft_prob * BB_freq(replica);
	  Add_Goto(new_bb, fall_thru_dest);
	  if (fall_thru_dest_in_loop) {
	    new_bbs = BB_SET_Union1D(new_bbs, new_bb, &MEM_local_nz_pool);
	    Set_BB_loop_head_bb(new_bb, &replicas[0]);
	    Set_BB_unrollings(new_bb, ntimes);
	  }
#ifdef TARG_ST
	  else if (fall_thru_dest == CG_LOOP_epilog)
	    CG_LOOP_epilog = new_bb;
#endif	  
	}
	Link_Pred_Succ_with_Prob(replica, BB_next(replica), ft_prob);
	if (fall_thru_dest_in_loop && fall_thru_dest != &replicas[0] &&
	    (freqs || BB_freq_fb_based(fall_thru_dest)))
	  BB_freq(fall_thru_dest) += ft_prob * BB_freq(replica);
      }

#ifdef TARG_ST
      unroll_names_finish();
#endif
    }
  }


  /* Restore original head freq so remainder freqs can be set correctly.
   */
  BB_freq(head) = orig_head_freq;

  /* Add "remainder" loop to prolog to perform first (trip_count %
   * ntimes) iterations (if necessary).  Note that this causes a
   * new set of prolog backpatches for the unrolled body to be
   * generated, and changes <head>.  Restore original head frequency
   * so remainder frequencies can be set correctly.
   */
#ifdef TARG_ST
  int modulus, residue;

  Get_pragma_LoopMod(loop, &modulus, &residue);

  if (trip_count_tn && TN_is_constant(trip_count_tn))
    remainder_trip_count_val = TN_value(trip_count_tn) % ntimes;
  else {
    if (modulus > 1) {

      if (ntimes <= modulus) {
	/* If ntimes is a divisor of modulus. */
	if (modulus % ntimes == 0) {
	  remainder_trip_count_val = residue % ntimes;
	}
      }

      else /* ntimes > modulus */ {
	if (ntimes % modulus == 0) {
	  /* new_residue=k*modulus+residue, new_residue<ntimes.
	     remainder_trip_count_val is set to max value for new_residue
	     (ntimes-modulus+residue).  This property is not
	     implemented for the moment. */
	}
      }
    }
  }

  if (remainder_trip_count_val == 0)
    gen_remainder_loop = FALSE;

  if (!gen_remainder_loop ||
      !unroll_multi_make_remainder_loop(loop, ntimes, orig_bbs, num_bbs, remainder_trip_count_val)) {
#else
  if (!gen_remainder_loop ||
      !unroll_multi_make_remainder_loop(loop, ntimes, orig_bbs, num_bbs)) {
#endif
    /*
     * Remainder loop wasn't necessary.  Remove original loop BBs
     * from flow graph and BB chain.
     */
    BB *post_prolog = BB_next(CG_LOOP_prolog);
    gen_remainder_loop = FALSE;
    for (bbi = 0; bbi < num_bbs; bbi++) {
      BB *loop_bb = orig_bbs[bbi];
      BB *prev = BB_prev(loop_bb);
      BB *next = BB_next(loop_bb);
      RID *rid = BB_rid(loop_bb);
      while (BB_succs(loop_bb))
	Unlink_Pred_Succ(loop_bb, BBLIST_item(BB_succs(loop_bb)));
      while (BB_preds(loop_bb))
	Unlink_Pred_Succ(BBLIST_item(BB_preds(loop_bb)), loop_bb);
      if (prev && BB_next(prev) == loop_bb) BB_next(prev) = next;
      if (next && BB_prev(next) == loop_bb) BB_prev(next) = prev;
      BB_prev(loop_bb) = BB_next(loop_bb) = NULL;
      if (REGION_First_BB == loop_bb) REGION_First_BB = next;
      if (rid && RID_cginfo(rid)) {
	/* update cgrin pointers */
	CGRIN *cgrin = RID_cginfo(rid);
	if (CGRIN_first_bb(cgrin) == loop_bb) CGRIN_first_bb(cgrin) = next;
	if (CGRIN_last_bb(cgrin) == loop_bb) CGRIN_last_bb(cgrin) = prev;
      }
    }
    BB_next(CG_LOOP_prolog) = post_prolog;
  }

  /* Update loop descriptor */
  LOOP_DESCR_loophead(loop) = &replicas[0];
  FOR_ALL_BB_SET_members(LOOP_DESCR_bbset(loop), bb)
    LOOP_DESCR_Delete_BB(loop, bb);
  FOR_ALL_BB_SET_members(new_bbs, bb)
    LOOP_DESCR_Add_BB(loop, bb);
  LOOP_DESCR_loopinfo(loop) = unrolled_info;
  LOOP_DESCR_num_exits(loop) =
    LOOP_DESCR_num_exits(loop) * ntimes - removed_exits;

#ifdef TARG_ST
  /* Update the loopmod pragma to take into account this unrolling. */
    if (modulus > 1) {
      INT new_modulus = 1;
      INT new_residue = 0;

      if (ntimes <= modulus) {
	/* If ntimes is a divisor of modulus. */
	if (modulus % ntimes == 0) {
	  new_modulus = modulus / ntimes;
	  new_residue = residue / ntimes;
	}
      }

      ANNOTATION *loopmod_ant = ANNOT_Get(BB_annotations(&replicas[0]), ANNOT_PRAGMA);
      while (loopmod_ant && WN_pragma(ANNOT_pragma(loopmod_ant)) != WN_PRAGMA_LOOPMOD)
	loopmod_ant = ANNOT_Get(ANNOT_next(loopmod_ant), ANNOT_PRAGMA);
      if (loopmod_ant) {
	WN *wn = ANNOT_pragma(loopmod_ant);
	Is_True(WN_pragma_arg1(wn) == modulus && WN_pragma_arg2(wn) == residue, ("CG_LOOP: Inconsistent #pragma loopmod"));
	WN_pragma_arg1(wn) = new_modulus;
	WN_pragma_arg2(wn) = new_residue;
      }
    }
#endif

  if (gen_remainder_loop && !TN_is_constant(trip_count_tn)) {
    /*
     * Add zero-trip guard around unrolled body.  Must wait until
     * remainder loop has been placed in prolog before doing this.
     * This also creates a new trip count TN for <unrolled_info>.
     */
    unroll_guard_unrolled_body(loop, unrolled_info, trip_count_tn, ntimes);
  }

  BB_MAP_Delete(orig_bb_index_map);
  MEM_POOL_Pop(&MEM_local_nz_pool);
  return TRUE;
}


void CG_LOOP_Finish(void)
/* -----------------------------------------------------------------------
 * See "cg_loop.h" for interface.
 * -----------------------------------------------------------------------
 */
{
  /* Nothing to do yet */
  /* TODO: Decide when to delete _CG_LOOP_bb_unrollings_map and
   * _CG_LOOP_op_unrolling_map.  Either don't do it here, or (better,
   * I think) don't call this until after scheduling (which should
   * make me finally invoke the scheduler correctly).  But unrollings
   * info needs to live past GRA through second optional scheduling
   * phase, right?  So maybe these should be OP/BB attributes.
   */
}

BB* 
CG_LOOP_Gen_And_Prepend_To_Prolog(BB *loop_head, LOOP_DESCR* loop)
/* -----------------------------------------------------------------------
 * See "cg_loop.h" for interface.
 * -----------------------------------------------------------------------
 */
{
  BB *ftp = BB_Fall_Thru_Predecessor(loop_head);
  LOOP_DESCR *enclosing = LOOP_DESCR_Next_Enclosing_Loop(loop);
  BB *new_prolog = Gen_BB_Like(loop_head);
  BOOL freqs = FREQ_Frequencies_Computed();

  if (!CG_localize_tns) Set_BB_gra_spill(new_prolog);
  if (BB_freq_fb_based(loop_head)) Set_BB_freq_fb_based(new_prolog);
  LOOP_DESCR_Retarget_Loop_Entrances(loop, new_prolog);

  if (ftp && BB_SET_MemberP(LOOP_DESCR_bbset(loop), ftp)) {
    /*
     * Can't make prolog fall-thru predecessor of <loop_head> without
     * adding branches in loop, so insert it at end of BB chain and
     * make it branch to <loop_head>.
     *
     * TODO: (Compspeed) Avoid walking BB chain to find last one by
     *       tracking REGION_Last_BB?  Can we just insert it before
     *       REGION_First_BB?
     */
    BB *last_bb = REGION_First_BB;
    while (BB_next(last_bb)) last_bb = BB_next(last_bb);
    Insert_BB(new_prolog, last_bb);
    Add_Goto(new_prolog, loop_head);
  } else {
    Insert_BB(new_prolog, BB_prev(loop_head));
    Link_Pred_Succ_with_Prob(new_prolog, loop_head, 1.0);
  }

  if (freqs || BB_freq_fb_based(new_prolog))
    BB_freq(loop_head) += BB_freq(new_prolog);

  /* Add CG_LOOP_prolog to appropriate LOOP_DESCRs, if any.
   * It can belong only to loops enclosing this one, so
   * we don't bother checking any others.
   */

  if (enclosing && 
      all_bbs_in(BB_preds(new_prolog), LOOP_DESCR_bbset(enclosing))) {
    LOOP_DESCR_Add_BB(enclosing, new_prolog);
    BB_nest_level(new_prolog) = LOOP_DESCR_nestlevel(enclosing);
  }

  return new_prolog;
}

BB*
CG_LOOP_Append_BB_To_Prolog(BB *loop_prolog, BB *loop_head)
/* -----------------------------------------------------------------------
 * See "cg_loop.h" for interface.
 * -----------------------------------------------------------------------
 */
{
  OP *br_op = BB_branch_op(loop_prolog);
  BB *new_bb = Gen_And_Insert_BB_After(loop_prolog);
  BOOL freqs = FREQ_Frequencies_Computed();

  LOOP_DESCR *prolog_loop = LOOP_DESCR_Find_Loop(loop_prolog);
  if (BB_freq_fb_based(loop_prolog)) Set_BB_freq_fb_based(new_bb);
  BB_rid(new_bb) = BB_rid(loop_prolog);
  Is_True(!TN_is_label(OP_opnd(br_op, Branch_Target_Operand(br_op))) ||
	  Is_Label_For_BB(TN_label(OP_opnd(br_op, Branch_Target_Operand(br_op))), loop_head),
	  ("explicit branch target should be <loop_head>"));

  BB_Remove_Op(loop_prolog, br_op);
  Change_Succ(loop_prolog, loop_head, new_bb);
  Add_Goto(new_bb, loop_head);

  if (freqs || BB_freq_fb_based(loop_prolog))
    BB_freq(loop_head) += BB_freq(new_bb);

  if (prolog_loop) LOOP_DESCR_Add_BB(prolog_loop, new_bb);

  return new_bb;
}


/* =======================================================================
 *
 *  CG_LOOP_Coalesce_Backedges
 *
 *    See interface description.
 *
 * =======================================================================
 */
void 
CG_LOOP_Coalesce_Backedges(LOOP_DESCR* loop)
{
  BB* bb_back;
  BB* head = LOOP_DESCR_loophead(loop);

  //
  // Add new bb before loop head and have it fall through.  Set freq
  // to 0.  When the branches are added to it in BB_Retarget_Branch,
  // the frequency will get updated.
  //
  bb_back = Gen_And_Insert_BB_Before(head);
  BB_freq(bb_back) = 0.0;
  LOOP_DESCR_Add_BB(loop, bb_back);

  //
  // Move all of the backedge branches to this new block.
  //
  BBLIST* preds = BB_preds(head);
  while (preds) {
    BB* pred = BBLIST_item(preds); 
    preds = BBLIST_next(preds);
    if (BB_SET_MemberP(LOOP_DESCR_bbset(loop), pred)) {
      if (!BB_Retarget_Branch(pred, head, bb_back)) {
	// must fall-thru to loop head 
	Change_Succ(pred, head, bb_back);
      }
    }
  }

  //
  // Make new backedge block branch to loop head, and 
  // calculate liveness information.
  //
  Add_Goto(bb_back, head);
  GRA_LIVE_Compute_Liveness_For_BB(bb_back);
}
	
/* For debugging (can't call varargs function CG_LOOP_Trace_Loop from dbx!) */
void trace_loop(LOOP_DESCR *loop)
{
  CG_LOOP_Trace_Loop(loop, "");
}

#ifdef TARG_ST
static void Unroll_Do_Loop_guard(LOOP_DESCR *loop,
				 LOOPINFO *unrolled_info,
				 BB *old_prolog,
				 TN *unrolled_trip_count,
				 TN *trip_count,
				 UINT32 ntimes)
#else
static void Unroll_Do_Loop_guard(LOOP_DESCR *loop,
				 LOOPINFO *unrolled_info,
				 TN *unrolled_trip_count)
#endif
{
  INT64 trip_est = WN_loop_trip_est(LOOPINFO_wn(unrolled_info));
  float ztrip_prob = 1.0 / MAX(trip_est, 1);
  float orig_post_prolog_freq = BB_freq(BB_next(CG_LOOP_prolog));
  OPS ops = OPS_EMPTY;
  BB *continuation_bb;
  LABEL_IDX continuation_lbl;

  LOOPINFO_trip_count_tn(unrolled_info) = unrolled_trip_count;

  extend_epilog(loop);
  continuation_bb = CG_LOOP_epilog;
  continuation_lbl = Gen_Label_For_BB(continuation_bb);

#ifdef TARG_ST
  INT32 trip_size = TN_size(trip_count);
  Exp_OP3v(OPC_FALSEBR,
	   NULL,
	   Gen_Label_TN(continuation_lbl,0),
	   trip_count,
	   Gen_Literal_TN(ntimes, trip_size),
	   trip_size == 4 ? V_BR_I4LT : V_BR_I8LT,
	   &ops);

  // FdF: Put the computation of the remainder loop trip count out of
  // the critical path
  OP *xfer_op = OPS_last(&ops);
  Is_True(OP_xfer(xfer_op), ("CG_LOOP: Expected a branch operation"));
  OPS_Remove_Op(&ops, xfer_op);

  OP *point = BB_last_op(old_prolog);
  BOOL before = TRUE;
  if (point != NULL && !OP_xfer(point))
    before = FALSE;
  BB_Insert_Ops(old_prolog, point, &ops, before);
  /* Must do this here because old_prolog and CG_LOOP_prolog may be
     the same. */
  BB_Append_Op(CG_LOOP_prolog, xfer_op);
#else
  Exp_OP3v(OPC_FALSEBR,
	   NULL,
	   Gen_Label_TN(continuation_lbl,0),
	   unrolled_trip_count,
	   Zero_TN,
	   V_BR_I8EQ,
	   &ops);
  BB_Append_Ops(CG_LOOP_prolog, &ops);
#endif
  Link_Pred_Succ_with_Prob(CG_LOOP_prolog, continuation_bb, ztrip_prob);
  Change_Succ_Prob(CG_LOOP_prolog, BB_next(CG_LOOP_prolog), 1.0 - ztrip_prob);
  BB_freq(BB_next(CG_LOOP_prolog)) = orig_post_prolog_freq;

  /* Extend prolog and epilog in case any further optimizations
   * want to use them.
   */
  extend_prolog();
  extend_epilog(loop);
}

//  Unroll a single-bb do-loop
//
void Unroll_Do_Loop(CG_LOOP& cl, UINT32 ntimes)
{
  LOOP_DESCR *loop = cl.Loop();

#ifdef TARG_ST
  int modulus, residue;

  Get_pragma_LoopMod(loop, &modulus, &residue);

  /* Since we have the constraint that ntimes is a power of 2, we
     cannot modify ntimes to be a multiple or divisor of modulus when
     it is not already. */
#if 0
  if (Get_pragma_LoopMod(loop, &modulus, &residue)) {

    if (ntimes < modulus) {
      /* It may be less costly, in terms of code size, to unroll more
         if this reduces the number of residue code. */
      if ((2*ntimes-1) >= (modulus+residue))
	ntimes = modulus;
      else
	/* Otherwise, reduce ntimes to be a divisor of modulus, so
           that the loop keeps (modulus, residue) properties after
           unrolling. */
	while (modulus % ntimes) ntimes--;
    }
    else if (ntimes > modulus) {
      /* Make ntimes be a multiple of modulus, so that residue code
         has some properties. */
      ntimes -= ntimes % modulus;
    }
  }
#endif
#endif

  if (Get_Trace(TP_CGLOOP, 2))
    CG_LOOP_Trace_Loop(loop,
		       "Unroll_Do_Loop: Before unrolling BB:%d %d times:",
		       BB_id(LOOP_DESCR_loophead(loop)), ntimes);

#ifdef TARG_ST
  if (Get_Trace(TP_CGLOOP, 0x10)) {
    if (cl.Even_factor() && (ntimes&1))
      fprintf(TFile, "<packing> Not even unroll factor (%d)\n", ntimes);
  }
#endif

  BB *head = LOOP_DESCR_loophead(loop);
  TN *trip_count_tn = CG_LOOP_Trip_Count(loop);
  BB *unrolled_body;
  ANNOTATION *annot;
  LOOPINFO *unrolled_info;
  TN *unrolled_trip_count = NULL;

  OPS ops = OPS_EMPTY;
  BOOL gen_remainder_loop = TRUE;
  BOOL gen_unrolled_loop_guard = TRUE;
#ifdef TARG_ST
    INT16 remainder_trip_count_val = -1;
#endif
  if (TN_is_constant(trip_count_tn)) {
    // Impose the restriction that the unrolled body must execute at
    // least two times.  There is no point in handling that here
    // because such loop should be fully unrolled.
    //
    if (TN_value(trip_count_tn) < 2 * ntimes) {
      if (Get_Trace(TP_CGLOOP, 2))
	CG_LOOP_Trace_Loop(loop, "disable unrolling because"
			   " trip_count(%d) < 2 * ntimes(%d).",
			   TN_value(trip_count_tn), ntimes);
      note_not_unrolled(head, "trip_count(%d) < 2 * unroll_factor(%d)",
			TN_value(trip_count_tn), ntimes);
      return;
    }
    gen_unrolled_loop_guard = FALSE;  // because unrolling is disabled for 0-trip loops
#ifdef TARG_ST
    remainder_trip_count_val = TN_value(trip_count_tn) % ntimes;
#endif
    if (TN_value(trip_count_tn) % ntimes == 0)
      gen_remainder_loop = FALSE;
  } else {
    INT32 trip_size = TN_size(trip_count_tn);
    unrolled_trip_count = Build_TN_Like(trip_count_tn);
    if (is_power_of_two(ntimes)) 
      Exp_OP2(trip_size == 4 ? OPC_I4ASHR : OPC_I8ASHR,
	      unrolled_trip_count,
	      trip_count_tn,
	      Gen_Literal_TN(log2(ntimes), trip_size),
	      &ops);
    else
      Exp_OP2(trip_size == 4 ? OPC_U4DIV : OPC_U8DIV,
	      unrolled_trip_count,
	      trip_count_tn,
	      Gen_Literal_TN(ntimes, trip_size),
	      &ops);

#ifdef TARG_ST
    if (modulus > 1) {

      if (ntimes <= modulus) {
	/* If ntimes is a divisor of modulus. */
	if (modulus % ntimes == 0) {
	  remainder_trip_count_val = residue % ntimes;
	}
      }

      else /* ntimes > modulus */ {
	if (ntimes % modulus == 0) {
	  /* new_residue=k*modulus+residue, new_residue<ntimes.
	     remainder_trip_count_val is set to max value for new_residue
	     (ntimes-modulus+residue).  This property is not
	     implemented for the moment. */
	}
      }

      if (remainder_trip_count_val == 0)
	gen_remainder_loop = FALSE;
    }
#endif
  }

  if (PROC_has_counted_loops()) {
    // Replace the loop-back branch with the counted loop branch
    // instruction.  It is a nop for the MIPS architecture.
    OPS body_ops = OPS_EMPTY;
    OP *br_op = BB_branch_op(head);
    TN *label_tn = OP_opnd(br_op, Branch_Target_Operand(br_op));

#ifdef TARG_ST
    if (CGTARG_Generate_Branch_Cloop(br_op, unrolled_trip_count, trip_count_tn, ntimes, label_tn, &ops, &body_ops)) {
#else
    CGTARG_Generate_Branch_Cloop(br_op, unrolled_trip_count, trip_count_tn,
				 ntimes, label_tn, &ops, &body_ops);
    if (OPS_length(&body_ops) > 0) {
#endif
      BB_Remove_Op(head, br_op);
      BB_Append_Ops(head, &body_ops);
      CGPREP_Init_Op(BB_branch_op(head));
      CG_LOOP_Init_Op(BB_branch_op(head));
    }
  }

  /* Initialize the TN renamer */
  unroll_names_init(loop, ntimes, &MEM_phase_nz_pool);

  /* Replicate the loop body <ntimes> and replace <head>
   * with unrolled version. */
  unrolled_body = Unroll_Replicate_Body(loop, ntimes, FALSE);
  annot = ANNOT_Get(BB_annotations(unrolled_body), ANNOT_LOOPINFO);
  FmtAssert(annot, ("unrolled body has no LOOPINFO annotation"));
  unrolled_info = ANNOT_loopinfo(annot);

  /* Add "remainder" loop to prolog to perform first (trip_count %
   * ntimes) iterations (if necessary).  Note that this causes a
   * new set of prolog backpatches for the unrolled body to be
   * generated, and changes <head>.
   */
#ifdef TARG_ST
  BB *old_prolog = CG_LOOP_prolog;
#endif
  if (gen_remainder_loop) {
    // CG_DEP_Delete_Graph(head);

#ifdef TARG_ST
    BOOL remainder_after = CG_LOOP_unroll_remainder_after;
    if (remainder_after || cl.Remainder_after()) {
      remainder_after = Check_remainder_after(unrolled_body, cl.Trip_count_bb(), trip_count_tn, ntimes);
      if (!remainder_after) {
	DevWarn("unroll_make_remainder_loop: remainder loop could not be put after unrolled loop");
	cl.Reset_special_stream();
      }
    }

    /* TBD: Insert peeled body if necessary. Be careful if remainder
       cannot be put after the loop. */

    // Pass new loop head and remainder_after flag.
    Unroll_Make_Remainder_Loop(cl, ntimes, remainder_trip_count_val, unrolled_body, remainder_after);
#else
    Unroll_Make_Remainder_Loop(cl, ntimes);
#endif
  }
#ifdef TARG_ST
  /* TBD: Insert peeled body if necessary. Be careful if remainder
     cannot be put after the loop. */

#endif

  /* Update loop descriptor for unrolled loop */
  LOOP_DESCR_loophead(loop) = unrolled_body;
  LOOP_DESCR_Delete_BB(loop, head);
  LOOP_DESCR_Add_BB(loop, unrolled_body);
  LOOP_DESCR_loopinfo(loop) = unrolled_info;

  // Insert unrolled trip count computation! 
  // It cannot be inserted earlier because unroll_make_remainder_loop
  // used/modified CG_LOOP_prolog!
  BB_Append_Ops(CG_LOOP_prolog, &ops);

  /* Add zero-trip guard around unrolled body if necessary.  Must wait
   * until remainder loop has been placed in prolog before doing this.
   * This also creates a new trip count TN for <unrolled_info>.
   */
  if (gen_unrolled_loop_guard)
#ifdef TARG_ST
    Unroll_Do_Loop_guard(loop, unrolled_info, old_prolog, unrolled_trip_count, trip_count_tn, ntimes);
#else
    Unroll_Do_Loop_guard(loop, unrolled_info, unrolled_trip_count);
#endif

  /* Fixup prolog backpatches.  Replace body TNs and omegas as if
   * they're uses in the zeroth unrolling.  */
  unroll_rename_backpatches(CG_LOOP_Backpatch_First(CG_LOOP_prolog, NULL),
			    0, ntimes);

  /* Fixup epilog backpatches.  Replace body TNs and omegas as if
   * they're uses in the last unrolling.
   */
  unroll_rename_backpatches(CG_LOOP_Backpatch_First(CG_LOOP_epilog, NULL),
			    ntimes-1, ntimes);

  unroll_names_finish();


#ifdef TARG_ST
  /* Update the loopmod pragma to take into account this unrolling. */
    if (modulus > 1) {
      INT new_modulus = 1;
      INT new_residue = 0;

      if (ntimes <= modulus) {
	/* If ntimes is a divisor of modulus. */
	if (modulus % ntimes == 0) {
	  new_modulus = modulus / ntimes;
	  new_residue = residue / ntimes;
	}
      }

      ANNOTATION *loopmod_ant = ANNOT_Get(BB_annotations(unrolled_body), ANNOT_PRAGMA);
      while (loopmod_ant && WN_pragma(ANNOT_pragma(loopmod_ant)) != WN_PRAGMA_LOOPMOD)
	loopmod_ant = ANNOT_Get(ANNOT_next(loopmod_ant), ANNOT_PRAGMA);
      if (loopmod_ant) {
	WN *wn = ANNOT_pragma(loopmod_ant);
	Is_True(WN_pragma_arg1(wn) == modulus && WN_pragma_arg2(wn) == residue, ("CG_LOOP: Inconsistent #pragma loopmod"));
	WN_pragma_arg1(wn) = new_modulus;
	WN_pragma_arg2(wn) = new_residue;
      }
    }
#endif

  if (Get_Trace(TP_CGLOOP, 2))
    CG_LOOP_Trace_Loop(loop, "Unroll_Do_Loop: After unrolling BB:%d %d times:",
		       BB_id(head), ntimes);
}

//  Fully unroll a single-bb do-loop
//
void Unroll_Do_Loop_Fully(LOOP_DESCR *loop, UINT32 ntimes)
{
  if (Get_Trace(TP_CGLOOP, 2))
    CG_LOOP_Trace_Loop(loop, "Unroll_Do_Loop_Fully:"
		       " Before unrolling BB:%d %d times:",
		       BB_id(LOOP_DESCR_loophead(loop)), ntimes);

  BB *head = LOOP_DESCR_loophead(loop);
  TN *trip_count_tn = CG_LOOP_Trip_Count(loop);
  BB *unrolled_body;
  ANNOTATION *annot;
  LOOPINFO *unrolled_info;

  Is_True(TN_is_constant(trip_count_tn) &&
	  TN_value(trip_count_tn) == ntimes,
	  ("Unroll_Do_Loop_Fully: unable to fully unroll do_loop."));

  /* Initialize the TN renamer */
  unroll_names_init(loop, ntimes, &MEM_phase_nz_pool);

  /* Replicate the loop body <ntimes> and replace <head>
   * with unrolled version. */
  unrolled_body = Unroll_Replicate_Body(loop, ntimes, TRUE);
  annot = ANNOT_Get(BB_annotations(unrolled_body), ANNOT_LOOPINFO);
  FmtAssert(annot, ("unrolled body has no LOOPINFO annotation"));
  unrolled_info = ANNOT_loopinfo(annot);

  /* Update loop descriptor for unrolled loop */
  LOOP_DESCR_loophead(loop) = unrolled_body;
  LOOP_DESCR_Delete_BB(loop, head);
  LOOP_DESCR_Add_BB(loop, unrolled_body);
  LOOP_DESCR_loopinfo(loop) = unrolled_info;

  /* Fixup prolog backpatches.  Replace body TNs and omegas as if
   * they're uses in the zeroth unrolling.  */
  unroll_rename_backpatches(CG_LOOP_Backpatch_First(CG_LOOP_prolog, NULL),
			    0, ntimes);

  /* Fixup epilog backpatches.  Replace body TNs and omegas as if
   * they're uses in the last unrolling.
   */
  unroll_rename_backpatches(CG_LOOP_Backpatch_First(CG_LOOP_epilog, NULL),
			    ntimes-1, ntimes);

  // Rename unrolled_body using prolog and epilog TNs.
  unroll_remove_notations(unrolled_body);

  unroll_names_finish();

  if (Get_Trace(TP_CGLOOP, 2))
    CG_LOOP_Trace_Loop(loop, "Unroll_Do_Loop_Fully:"
		       " After unrolling BB:%d %d times:",
		       BB_id(head), ntimes);
}


// Unroll single BB dowhile loop
//  * It is different from unroll_doloop because it does not rely on
//    CG_LOOP data structure, i.e.,  there is no CG_LOOP_prolog/epilog.
//    And there is CG_LOOP_info_map ...
//
void Unroll_Dowhile_Loop(LOOP_DESCR *loop, UINT32 ntimes)
{
  if (ntimes <= 1)
    return;

  MEM_POOL_Push(&MEM_local_nz_pool);

  if (Get_Trace(TP_CGLOOP, 2))
    CG_LOOP_Trace_Loop(loop, "Unroll_Dowhile_Loop:"
		       " Before unrolling BB:%d %d times:",
		       BB_id(LOOP_DESCR_loophead(loop)), ntimes);

  BB *head = LOOP_DESCR_loophead(loop);
  BB_SET *new_bbs;
  ANNOTATION *annot = ANNOT_Get(BB_annotations(head), ANNOT_LOOPINFO);
  LOOPINFO *info = annot ? ANNOT_loopinfo(annot) : NULL;
  LOOPINFO *unrolled_info = NULL;
  BOOL freqs = FREQ_Frequencies_Computed() || BB_freq_fb_based(head) ;

  BB *replicas = Gen_BB_N(ntimes-1);
  Set_BB_unrollings(&replicas[0], ntimes);

  if (info) {
    WN *wn = WN_COPY_Tree(LOOPINFO_wn(info));
    unrolled_info = TYPE_P_ALLOC(LOOPINFO);
    LOOPINFO_wn(unrolled_info) = wn;
    WN_loop_trip_est(wn) /= ntimes;
    WN_loop_trip_est(wn) += 1;
    LOOPINFO_srcpos(unrolled_info) = LOOPINFO_srcpos(info);
    BB_Add_Annotation(&replicas[0], ANNOT_LOOPINFO, unrolled_info);
  }

  /* Initialize the TN renamer */
  unroll_names_init(loop, ntimes, &MEM_phase_nz_pool);

  if (freqs || BB_freq_fb_based(head)) {
    /*
     * Compute frequency of unrolled head.  We'll use this to derive
     * the frequencies of the other replicas as we unroll.
     *
     * TODO: When we emit a remainder loop for trip-countable loops,
     *	     BB_freq(&replicas[0]) should be set to BB_freq(prolog) *
     *	     WN_loop_trip_est(LOOPINFO_wn(unrolled_info)).  The rounding
     *	     down of the trip_est to an integer correctly subtracts the
     *	     portion of the frequency that goes through the remainder loop.
     */
    if (info) {
      INT64 orig_trip_est = WN_loop_trip_est(LOOPINFO_wn(info));
      float orig_head_freq = BB_freq(head);
      if (CG_warn_bad_freqs &&
	  !FREQ_Match(orig_head_freq,
		      orig_trip_est * BB_freq(CG_LOOP_prolog)))
	DevWarn("BB_freq(orig head BB:%d) != BB_freq(prolog BB:%d) * trip_est",
		BB_id(head), BB_id(CG_LOOP_prolog));
    }
  }

  OP *br_op = BB_branch_op(head);
  INT br_targ_opnd = Branch_Target_Operand(br_op);
  BBLIST *fts_list = BBlist_Fall_Thru_Succ(head);
  BB *loop_merge_bb = BBLIST_item(fts_list);
  TN *loop_merge_label = Gen_Label_TN(Gen_Label_For_BB(loop_merge_bb), 0);
  float br_prob = freqs ? (1.0 - BBLIST_prob(fts_list)) : 0.5;
  float replica_prob = (freqs || BB_freq_fb_based(head)) ?
    (BB_freq(head) + ntimes - 1)/ ntimes : 1.0;

  /* Splice the replicas into the old BB chain.  The old loop BBs are
   * intentionally left in the chain since we use their prev/next/pred/succ
   * info while unrolling.
   */
  new_bbs = BB_SET_Create_Empty(PU_BB_Count, &MEM_local_nz_pool);
  Chain_BBs(BB_prev(head), &replicas[0]);
  for (INT i = 0; i < ntimes - 1; i++) {
    new_bbs = BB_SET_Union1D(new_bbs, &replicas[i], &MEM_local_nz_pool);
    Chain_BBs(&replicas[i], (i == ntimes - 2) ? head : &replicas[i+1]);
  }

  while (BB_preds(head)) {
    BB *pred = BBLIST_item(BB_preds(head));
    Change_Succ(pred, head, &replicas[0]);
  }

#ifdef TARG_ST
  int modulus, residue;

  Get_pragma_LoopMod(loop, &modulus, &residue);
#endif

  /* Build the replicas.
   */
  for (INT unrolling = 0; unrolling < ntimes - 1; unrolling++) {
    /* Initialize BB info in <replica> */
    BB *replica = &replicas[unrolling];
    Set_BB_loop_head_bb(replica, &replicas[0]);
    Set_BB_unrollings(replica, ntimes);
    BB_rid(replica) = BB_rid(head);
    if (BB_freq_fb_based(head)) Set_BB_freq_fb_based(replica);

    /* Replicate OPs from <orig_bb> into <replica>, renaming TNs as we go
     */
    OP *op;
    FOR_ALL_BB_OPs(head, op) {
#ifdef TARG_ST
      // Perform Prefetch pruning at unroll time
      if (OP_prefetch(op)) {

	WN *mem_wn = Get_WN_From_Memory_OP(op);
	Is_True(!mem_wn || WN_operator(mem_wn) == OPR_PREFETCH,
		("wrong prefetch WHIRL node."));

	if (Get_Trace(TP_CGLOOP, 2) && unrolling == 0)  // trace once per loop
	  if (mem_wn)
	    fprintf(TFile, "<cgpref> - 1L cache stride = %d, 2L cache stride = %d,"
		    " confidence = %d\n",
		    WN_pf_stride_1L(mem_wn),
		    WN_pf_stride_2L(mem_wn),
		    WN_pf_confidence(mem_wn));
	  else
	    fprintf(TFile, "<cgpref> pref wn not found.\n");

	if (mem_wn && Prefetch_Kind_Enabled(mem_wn)) {
	  int stride = WN_pf_stride_2L( mem_wn ) ?  WN_pf_stride_2L( mem_wn ) :  WN_pf_stride_1L(mem_wn);
	  if (stride != 0 && (unrolling % stride) != 0) {
	    if (Get_Trace(TP_CGLOOP, 2))
	      fprintf(TFile, "<cgpref> pref pruned at unrolling %d.\n", unrolling);
	    continue;
	  }
	}
      }
#endif
      OP *rop = Dup_OP(op);
      Set_OP_unrolling(rop, unrolling);
      Set_OP_orig_idx(rop, OP_map_idx(op));
      Set_OP_unroll_bb(rop, replica);
      Copy_WN_For_Memory_OP(rop, op);
      for (INT resi = 0; resi < OP_results(rop); resi++) {
	TN *res = OP_result(rop,resi);
	if (!TN_is_global_reg(res)) {
	  TN *new_res = unroll_names_get(res, unrolling);
	  Set_OP_result(rop, resi, new_res);
	}
      }
      for (INT opi = 0; opi < OP_opnds(rop); opi++) {
	TN *opnd = OP_opnd(rop, opi);
	if (TN_is_register(opnd)) {
	  if (!TN_is_global_reg(opnd)) {
	    Set_OP_opnd(rop, opi, unroll_names_get(opnd, unrolling));
	  }
	}
      }
      BB_Append_Op(replica, rop);
    }

#ifdef TARG_ST
    if ((modulus % ntimes == 0) && (residue % ntimes != unrolling+1)) {
      BB_Remove_Branch(replica);
      Link_Pred_Succ_with_Prob(replica, BB_next(replica), 1.0);
      if (freqs)
	BB_freq(replica) = replica_prob;
    }
    else {
#endif
    OP *replica_br_op = BB_branch_op(replica);
    BOOL ok = Negate_Branch(replica_br_op);
    Is_True(ok, ("unable to negate branch %s", TOP_Name(OP_code(replica_br_op))));
    Set_OP_opnd(replica_br_op, Branch_Target_Operand(replica_br_op),
		loop_merge_label);
    Link_Pred_Succ_with_Prob(replica, loop_merge_bb, 1.0 - br_prob);
    Link_Pred_Succ_with_Prob(replica, BB_next(replica), br_prob);
    if (freqs)
      BB_freq(replica) = replica_prob;
    replica_prob *= br_prob;
#ifdef TARG_ST
    }
#endif
  }
  if (freqs)
    BB_freq(head) = replica_prob;
  
#ifdef TARG_ST
  // [CG]: Update last unrolled replica (old head)
  OP *op, *next_op;
  Set_BB_loop_head_bb(head, &replicas[0]);
  Set_BB_unrollings(head, ntimes);

  for (op = BB_first_op(head); op != NULL; op = next_op) {
    next_op = OP_next(op);

    // Perform Prefetch pruning at unroll time
    if (OP_prefetch(op)) {
      WN *mem_wn = Get_WN_From_Memory_OP(op);
      Is_True(!mem_wn || WN_operator(mem_wn) == OPR_PREFETCH,
	      ("wrong prefetch WHIRL node."));
      
      if (mem_wn && Prefetch_Kind_Enabled(mem_wn)) {
	int stride = WN_pf_stride_2L( mem_wn ) ?  WN_pf_stride_2L( mem_wn ) :  WN_pf_stride_1L(mem_wn);
	if (stride != 0 && ((ntimes-1) % stride) != 0) {
	  if (Get_Trace(TP_CGLOOP, 2))
	    fprintf(TFile, "<cgpref> pref pruned at unrolling %d.\n", ntimes-1);
	  BB_Remove_Op(head, op);
	  continue;
	}
      }
    }

    Set_OP_unrolling(op, ntimes-1); 
    Set_OP_orig_idx(op, OP_map_idx(op));
    Set_OP_unroll_bb(op, head);
  }
#endif

  {
    // update loopback edge
    unroll_xfer_annotations(&replicas[0], head);
    OP *br = BB_branch_op(head);
#ifdef TARG_ST
    if ((modulus % ntimes == 0) && (residue % ntimes != 0)) {
      OPS ops = OPS_EMPTY;

      Build_OP (TOP_goto, 
		Gen_Label_TN(Gen_Label_For_BB(&replicas[0]),0),
		&ops);
      OP_srcpos(OPS_first(&ops)) = OP_srcpos(br);
      BB_Insert_Ops(OP_bb(br), br, &ops, FALSE);
      BB_Remove_Op(OP_bb(br), br);
      Unlink_Pred_Succ (head, loop_merge_bb);
      Change_Succ_Prob (head, &replicas[0], 1.0);
    }
    else
#endif
    Set_OP_opnd(br,
		Branch_Target_Operand(br),
		Gen_Label_TN(Gen_Label_For_BB(&replicas[0]),0));
  }

  {
    /* Update loop descriptor */
    LOOP_DESCR_loophead(loop) = &replicas[0];
    BB *bb;
    FOR_ALL_BB_SET_members(new_bbs, bb)
      LOOP_DESCR_Add_BB(loop, bb);
    LOOP_DESCR_loopinfo(loop) = unrolled_info;
  }

  unroll_names_finish();

#ifdef TARG_ST
  /* Update the loopmod pragma to take into account this unrolling. */
    if (modulus > 1) {
      INT new_modulus = 1;
      INT new_residue = 0;

      if (ntimes <= modulus) {
	/* If ntimes is a divisor of modulus. */
	if (modulus % ntimes == 0) {
	  new_modulus = modulus / ntimes;
	  new_residue = residue / ntimes;
	}
      }

      ANNOTATION *loopmod_ant = ANNOT_Get(BB_annotations(&replicas[0]), ANNOT_PRAGMA);
      while (loopmod_ant && WN_pragma(ANNOT_pragma(loopmod_ant)) != WN_PRAGMA_LOOPMOD)
	loopmod_ant = ANNOT_Get(ANNOT_next(loopmod_ant), ANNOT_PRAGMA);
      if (loopmod_ant) {
	WN *wn = ANNOT_pragma(loopmod_ant);
	Is_True(WN_pragma_arg1(wn) == modulus && WN_pragma_arg2(wn) == residue, ("CG_LOOP: Inconsistent #pragma loopmod"));
	WN_pragma_arg1(wn) = new_modulus;
	WN_pragma_arg2(wn) = new_residue;
      }
    }
#endif

  if (Get_Trace(TP_CGLOOP, 2))
    CG_LOOP_Trace_Loop(loop, "Unroll_Dowhile_Loop:"
		       " After unrolling BB:%d %d times:",
		       BB_id(head), ntimes);

  MEM_POOL_Pop(&MEM_local_nz_pool);
}

INT32 CG_LOOP::Get_Unroll_Times(ANNOTATION *&pragma_unroll)
{
  BB *head = LOOP_DESCR_loophead(loop);
  INT32 unroll_times;

  pragma_unroll = NULL;
  unroll_times = CG_LOOP_unroll_times_max;

  if (!CG_LOOP_ignore_pragmas) {
    ANNOTATION *unroll_ant = ANNOT_Get(BB_annotations(head), ANNOT_PRAGMA);
    while (unroll_ant && WN_pragma(ANNOT_pragma(unroll_ant)) != WN_PRAGMA_UNROLL)
      unroll_ant = ANNOT_Get(ANNOT_next(unroll_ant), ANNOT_PRAGMA);
    if (unroll_ant) {
      WN *wn = ANNOT_pragma(unroll_ant);
      unroll_times = MAX(1, WN_pragma_arg1(wn));
      pragma_unroll = unroll_ant;
    }
  }
  return unroll_times;
}


/*
 * Unroll constant trip count loop fully iff:
 *   (a) CG:unroll_fully not turned off, and either
 *   (b1) unrolled size <= OPT:unroll_size, or
 *   (b2) OPT:unroll_size=0 and OPT:unroll_times_max >= trip count
 */
bool CG_LOOP::Determine_Unroll_Fully()
{
  if (!CG_LOOP_unroll_fully) 
    return false;
  
  LOOPINFO *info = LOOP_DESCR_loopinfo(Loop());
  TN *trip_count_tn = info ? LOOPINFO_trip_count_tn(info) : NULL;
  BB *head = LOOP_DESCR_loophead(loop);
  ANNOTATION *pragma_unroll;
  INT32 unroll_times_max = Get_Unroll_Times(pragma_unroll);

  if (BB_Has_Exc_Label(head))
    return false;

  if (unroll_times_max < 2) 
    return false;

  if (trip_count_tn == NULL)  
    return false;

  if (!TN_is_constant(trip_count_tn))
    return false;

  INT32 const_trip_count = TN_value(trip_count_tn);
  INT32 body_len = BB_length(head);

  // FdF: Avoid overflow in body_len * const_trip_count
  if (((const_trip_count <= CG_LOOP_unrolled_size_max) &&
       (body_len * const_trip_count <= CG_LOOP_unrolled_size_max) &&
       !pragma_unroll) ||
      (((CG_LOOP_unrolled_size_max == 0) || pragma_unroll) &&
       (unroll_times_max >= const_trip_count))) {

    if (Get_Trace(TP_CGLOOP, 2))
      fprintf(TFile, "<unroll> unrolling fully (%d times)\n", const_trip_count);

    Set_unroll_fully();
    Set_unroll_factor(const_trip_count);
    return true;
  }

  return false;
}

#ifdef TARG_ST
// FdF 20060207: Use scheduling estimate to adjust the unrolling
// factor
static int
LOOP_DESCR_Estimate_Factor_For_Unrolling(LOOP_DESCR *loop, int unroll_factor)
{
  BB *bb;
  BB *head = LOOP_DESCR_loophead(loop);

  if (BB_SET_Size(LOOP_DESCR_bbset(loop)) > 1)
    return 0;

  // Now, count the number of LOAD operations in the loop, we consider
  // them part of the critical path, and count the total number of
  // operations in the loop.
  int op_cnt = BB_length(head);
  int load_cnt = 0;
  int mem_cnt = 0;
  OP *op;
  FOR_ALL_BB_OPs(head, op) {
    if (OP_memory(op))
      mem_cnt ++;
    if (OP_load(op))
      load_cnt ++;
  }

  // We consider that a do-loop body has the following properties:
  //   - All load operations are part of the critical path
  //   - The exit test has the form (add, cmp, br) and will not be
  //   duplicated, after optimizations, by the unrolling.

  // The unrolling factor is computed to be the lowest value such that
  // the critical path is smaller than the minimum cycles of the loop.

  // This loop has fewer than expected non memory operations
  if (op_cnt < (mem_cnt+3))
    return 0;

  BB_MAP sch_est = BB_MAP_Create();
  CG_SCHED_EST *se = CG_SCHED_EST_Create(head, &MEM_local_nz_pool, SCHED_EST_FOR_UNROLL);
  BB_MAP_Set(sch_est, head, se);

  // Critical Path is under estimated, fix it
  int cp_cycles = CG_SCHED_EST_Critical_Length(se) + load_cnt;
  int min_cycles = CG_SCHED_EST_Resource_Cycles(se);
  int sched_unroll_factor = 1;

  while (cp_cycles > min_cycles && sched_unroll_factor < unroll_factor) {
    sched_unroll_factor ++;

    // Add an iteration in the loop body and compute the new estimated
    // critical path and min resource cycles.
    OP *op;
    BOOL dup_INCIV = FALSE, dup_CMP = FALSE, dup_BR = FALSE;
    FOR_ALL_BB_OPs(head, op) {
      // Loop unrolling does not duplicate the BR and CMP operations,
      // and at least the ADD of the main IV will be factorized
      if ((OP_iadd(op) || OP_isub(op)) && !dup_INCIV)
	dup_INCIV = TRUE;
      else if (OP_icmp(op) && !dup_CMP)
	dup_CMP = TRUE;
      else if (OP_br(op) && !dup_BR)
	dup_BR = TRUE;
      else
	CG_SCHED_EST_Add_Op_Resources(se, OP_code(op));
    }

    // Critical Path is under estimated, fix it
    cp_cycles = CG_SCHED_EST_Critical_Length(se) + load_cnt*sched_unroll_factor;
    min_cycles = CG_SCHED_EST_Resource_Cycles(se);
    //    printf("cp_cycles %d, min_cycles %d\n", cp_cycles, min_cycles);
  }

  //  printf("sched estimate gives %d\n", sched_unroll_factor);
  return sched_unroll_factor;
}
#endif

void CG_LOOP::Determine_Unroll_Factor()
{ 
  LOOPINFO *info = LOOP_DESCR_loopinfo(Loop());
  TN *trip_count_tn = info ? LOOPINFO_trip_count_tn(info) : NULL;
  BB *head = LOOP_DESCR_loophead(loop);
  BOOL trace = Get_Trace(TP_CGLOOP, 2);
  INT32 unroll_times_max;
  ANNOTATION *pragma_unroll;

  Set_unroll_factor(1);

  if (BB_Has_Exc_Label(head)) {
    char *reason = "in exception region or handler";
    note_not_unrolled(head, reason);
    if (trace) fprintf(TFile, "<unroll> not unrolling; %s\n", reason);
    return;
  }

  unroll_times_max = Get_Unroll_Times(pragma_unroll);

#ifdef TARG_ST
  // FdF 20060207: Determine_Sched_Est_Unroll_Factor may have computed
  // an estimate Unroll_Factor.
  if (Unroll_sched_est() > 0 && Unroll_sched_est() < unroll_times_max)
    unroll_times_max = Unroll_sched_est();
#endif

#ifdef TARG_ST
  INT32 trip_estimate = -1;

  if (!pragma_unroll) {

    /* FdF 20050302: The very low frequency execution of a loop do not
       justify unrolling it. The threshold is the same as for hot/cold
       regions. */
    if (trip_estimate == -1 && BB_freq_fb_based(head) && BB_freq(head) <= 0.01)
      trip_estimate = unroll_times_max = 0;

    /* FdF 20050302: Otherwise, make sure unrolling factor is less than
       half the estimated trip count. */
    else if (info && (!trip_count_tn || !TN_is_constant(trip_count_tn))) {
      WN *wn = LOOPINFO_wn(info);
      if ( wn ) {
	INT32 trip_est = WN_loop_trip_est(wn);
	if (trip_est && (trip_est/2) < unroll_times_max) {
	  trip_estimate = trip_est;
	  unroll_times_max = trip_est/2;
	}
      }
    }
  }

#endif

  if (unroll_times_max < 2) {
    const char * reason = pragma_unroll ? "#pragma unroll(%d)" : "OPT:unroll_times_max=%d";
    INT32 reason_value = unroll_times_max;
#ifdef TARG_ST
    if (trip_estimate != -1) {
      reason = "Loop trip estimate=%d";
      reason_value = trip_estimate;
    }
#endif
    note_not_unrolled(head, reason, reason_value);
    if (trace) {
      fprintf(TFile, "<unroll> not unrolling; ");
      fprintf(TFile, reason, reason_value);
      fprintf(TFile, "\n");
    }
    return;
  }

#ifdef TARG_ST

  /* FdF: Multi-bb unrolling is activated either with -CG:unroll_multi_bb=1
     or with #pragma unroll. */

  if ((BB_SET_Size(LOOP_DESCR_bbset(loop)) > 1) &&
      !(pragma_unroll || CG_LOOP_unroll_multi_bb))
    return;

  INT32 body_len = 0;
  BB *bb;
  FOR_ALL_BB_SET_members(LOOP_DESCR_bbset(loop), bb) {

    if (BB_Has_Exc_Label(bb)) {
      char *reason = "in exception region or handler";
      note_not_unrolled(head, reason);
      if (trace) fprintf(TFile, "<unroll> not unrolling; %s\n", reason);
      return;
    }

    body_len += BB_length(bb);
  }

#else
  INT32 body_len = BB_length(head);
#endif

  if (trip_count_tn == NULL) {

    UINT32 ntimes = MAX(1, unroll_times_max);
    if (!pragma_unroll) {
      while (ntimes > 1 && ntimes * body_len > CG_LOOP_unrolled_size_max)
	ntimes--;
#ifdef TARG_ST
      if (Even_factor() && (ntimes&1)) {
	if (ntimes > 1 || ((CG_LOOP_load_store_packing&0x20) != 0)) {
	  if (Get_Trace(TP_CGLOOP, 0x10)) {
	    fprintf(TFile, "<packing> Making unroll factor even (%d)\n", ntimes);
	  }
	  ntimes ++;
	}
      }
#endif
    }
    Set_unroll_factor(ntimes);

  } else {

    BOOL const_trip = TN_is_constant(trip_count_tn);
    INT32 const_trip_count = const_trip ? TN_value(trip_count_tn) : 0;

    if (const_trip && (CG_LOOP_unroll_fully || pragma_unroll) &&
	/*
	 * Unroll constant trip count loop fully iff:
	 *   (a) CG:unroll_fully not turned off or pragma_unroll, and either
	 *   (b1) unrolled size <= OPT:unroll_size, or
	 *   (b2) (OPT:unroll_size=0 || pragma_unroll) and unroll_times_max >= trip count
	 */
	// FdF: Avoid overflow in body_len * const_trip_count
	(((const_trip_count <= CG_LOOP_unrolled_size_max) &&
	  (body_len * const_trip_count <= CG_LOOP_unrolled_size_max) &&
	  !pragma_unroll) ||
	 (((CG_LOOP_unrolled_size_max == 0) || pragma_unroll) &&
	  (unroll_times_max*2 > const_trip_count)))) {

      if (trace)
	fprintf(TFile, "<unroll> unrolling fully (%d times)\n", const_trip_count);

      Set_unroll_fully();
      Set_unroll_factor(const_trip_count);
    } else {
      UINT32 ntimes = unroll_times_max;
      if (const_trip) {
	// round down until const_trip_count % ntimes == 0, or ntimes
	// is 2^n
	while ((const_trip_count % ntimes) && !is_power_of_two(ntimes))
	  ntimes --;
#ifdef TARG_ST
	/* See if we can eliminate the residue by choosing ntimes-1 or
	   ntimes+1. */
	char *trace = NULL;
	if (const_trip_count % ntimes != 0) {
	  if ((ntimes > 2) && (const_trip_count % (ntimes-1) == 0)) {
	    trace = "<unroll> const_trip_count %d, ntimes-1=%d, residue=0\n";
	    ntimes --;
	  }
	  else if (const_trip_count % (ntimes+1) == 0) {
	    trace = "<unroll> const_trip_count %d, ntimes+1=%d, residue=0\n";
	    ntimes ++;
	  }
	}
	/* Otherwise, see if we can have a residue of 1, since this
	   does not make a loop. */
	if (const_trip_count % ntimes > 1 ) {
	  if (const_trip_count % (ntimes-1) == 1) {
	    trace = "<unroll> const_trip_count %d, ntimes-1=%d, residue=1\n";
	    ntimes --;
	  }
	  else if (const_trip_count % (ntimes+1) == 1) {
	    trace = "<unroll> const_trip_count %d, ntimes+1=%d, residue=1\n";
	    ntimes ++;
	  }
	}
	if (trace && Get_Trace(TP_CGLOOP, 1))
	  fprintf(TFile, trace, const_trip_count, ntimes);
#endif
      }
      else {
	if (!is_power_of_two(ntimes)) {
	  ntimes = 1 << log2(ntimes); 
	  if (trace)
	    fprintf(TFile, "<unroll> rounding down to power of two = %d times\n", ntimes);
	}
      }
      if (!pragma_unroll)
	while (ntimes > 1 && ntimes * body_len > CG_LOOP_unrolled_size_max)
	  ntimes /= 2;
      if (const_trip) {
	// At least two iterations plus some residue.
	while (ntimes > 1 && const_trip_count < 2 * ntimes) 
	  ntimes /= 2;
      }
#ifdef TARG_ST
      // FdF 20060207: Force unrolling by two if scheduling estimated
      // unroll factor says unrolling is profitable.
      if ((CG_LOOP_unroll_heuristics & UNROLL2_HEURISTIC) && (ntimes == 1) && (Unroll_sched_est() > 1)) {
	ntimes = 2;
      }
      if (Even_factor() && (ntimes&1) && !pragma_unroll) {
	if (ntimes > 1 || ((CG_LOOP_load_store_packing&0x20) != 0)) {
	  if (Get_Trace(TP_CGLOOP, 0x10)) {
	    fprintf(TFile, "<packing> Making unroll factor even (%d)\n", ntimes);
	  }
	  ntimes ++;
	}
      }
#endif
      Set_unroll_factor(ntimes);
    }
  }
#ifdef KEY
  if( Unroll_factor() > 1 ){
    for( OP* op = BB_first_op(head); op != NULL; op = OP_next(op) ){
      if( !OP_store(op) )
	continue;

      TN* tn = OP_opnd( op, OP_find_opnd_use(op,OU_storeval) );
      if( TN_is_dedicated(tn)   || 
	  !TN_is_global_reg(tn) ||
	  !TN_is_gra_homeable(tn) )
	continue;

      WN* wn = Get_WN_From_Memory_OP(op);
      if( wn == NULL )
	continue;

      /* After unrolling, more than one tn will store to the same home
	 location.  (bug#3471)
      */
      if( Aliased( Alias_Manager, TN_home(tn), wn ) == SAME_LOCATION ){
      	Reset_TN_is_gra_homeable( tn );
	Set_TN_home( tn, NULL );
      }
    }    
  }
#endif
}

// FdF 20060207: For single BB do-loop only, use scheduling estimate
// information to reduce the unrolling factor when it will not have a
// significant impact on the performance of the loop.
// 

void CG_LOOP::Determine_Sched_Est_Unroll_Factor()
{ 
  int sched_unroll_factor = CG_LOOP_unroll_times_max;
  ANNOTATION *pragma_unroll;

  // Just check if there is a pragma unroll on the loop.
  int unroll_times_max = Get_Unroll_Times(pragma_unroll);

  // Compute a maximum unroll factor based on a schedule estimate of
  // the loop
  if ((CG_LOOP_unroll_heuristics & SCHED_HEURISTIC) && !pragma_unroll) {
    sched_unroll_factor = LOOP_DESCR_Estimate_Factor_For_Unrolling (Loop(), unroll_times_max);
    if (sched_unroll_factor > 0)
      Set_unroll_sched_est(sched_unroll_factor);
  }

  // Then, call the normal function to determine the unrolling factor
  Determine_Unroll_Factor();
}


// Returns TRUE if OP is live
//   
inline bool CG_LOOP_OP_is_live(OP *op, TN_SET *live_set, bool keep_prefetch)
{
  if (OP_store(op))
    return true;
  if (OP_has_implicit_interactions(op))
    return true;
  if (keep_prefetch && OP_prefetch(op))
    return true;
#ifdef TARG_ST
  // FDF: Keep branch instruction because we do not run Gen_SWP_Branch
  if (OP_br(op))
    return true;
#endif
  for (INT i = 0; i < OP_results(op); i++) {
    TN *res = OP_result(op, i);
    if (TN_is_register(res) && !TN_is_const_reg(res))
      if (TN_SET_MemberP(live_set, res) || TN_is_dedicated(res))
	return true;
  }
  return false;
}

//  Remove dead variables or Mark them as LOH ops
//
void Induction_Variables_Removal(CG_LOOP& cl,
				 bool remove, 
				 bool keep_prefetch,
				 bool trace)
{
  BB *body = cl.Loop_header();
  BB *epilog = CG_LOOP_epilog;
  OP *op;

  if (!remove) {
    FOR_ALL_BB_OPs(body, op) {
      Reset_OP_loh(op);
    }
  }
  
  CXX_MEM_POOL pool("Temp TN_SET", FALSE);
  TN_SET *tnset = TN_SET_Create_Empty(Last_TN + 1, pool());
  
  // Initialize live TN set
  TN *tn;
  FOR_ALL_GTN_SET_members(BB_live_in(epilog),tn) {
    tnset = TN_SET_Union1D(tnset, tn, pool());
  }

  // Collect all live TNs
  //
  bool changed = true;
  while (changed) {
    changed = false;
    FOR_ALL_BB_OPs_REV(body, op) {
      if (CG_LOOP_OP_is_live(op, tnset, keep_prefetch)) {
	// Add the opnds to live-set
	for (INT i = 0; i < OP_opnds(op); i++) {
	  TN *opnd = OP_opnd(op, i);
	  if (TN_is_register(opnd) &&
	      !TN_is_const_reg(opnd) &&
	      !TN_SET_MemberP(tnset, opnd)) {
	    changed = true;
	    tnset = TN_SET_Union1D(tnset, opnd, pool());
	  }
	}
      }
    }
  }
  
  // Remove dead OP from BB
  //
  if (!remove) {
    FOR_ALL_BB_OPs(body, op) {
      if (!CG_LOOP_OP_is_live(op, tnset, keep_prefetch)) {
	Set_OP_loh(op);
	if (trace) {
	  fprintf(TFile, "<remove ind var> mark loh OP: ");
	  Print_OP_No_SrcLine(op);
	}
      }
    }
  } else {
    OP *next_op;
    for (op = BB_first_op(body); op != NULL;  op = next_op) {
      next_op = OP_next(op);
      if (!CG_LOOP_OP_is_live(op, tnset, keep_prefetch)) {
	if (trace) {
	  fprintf(TFile, "<remove ind var> delete OP: ");
	  Print_OP_No_SrcLine(op);
	}
	for (INT i = 0; i < OP_results(op); i++) {
	  TN *tn = OP_result(op, i);
	  if (TN_is_register(tn) && !TN_is_const_reg(tn)) {
	    GTN_SET_Difference1D(BB_defreach_out(body), tn);
	    GTN_SET_Difference1D(BB_live_in(body), tn);
	  }
	}
	BB_Remove_Op(body, op);
      } else {
	// the results of the OP might not be used
	for (INT i = 0; i < OP_results(op); i++) {
	  TN *tn = OP_result(op, i);
	  if (TN_is_register(tn) && 
	      !TN_is_const_reg(tn) &&
	      !TN_SET_MemberP(tnset, tn)) {
#ifndef TARG_ST
	    // FdF 20050915: True_TN does not really exist on ST200
	    if (TN_register_class(tn) == TN_register_class(True_TN)) {
	      Set_OP_result(op, i, True_TN);
	      if (trace) {
		fprintf(TFile, "<remove ind var> modify result TN: ");
		Print_OP_No_SrcLine(op);
	      }
	    }
#endif
	  }
	}
      }
    }
  }
}

void CG_LOOP::Determine_SWP_Unroll_Factor()
{
  MEM_POOL_Push(&MEM_local_nz_pool);

  Is_True(SWP_Options.Min_Unroll_Times <= SWP_Options.Max_Unroll_Times,
	  ("CG_LOOP:  -SWP:min_unroll_times > -SWP:max_unroll_times"));

  Is_True(!Unroll_fully(),
	  ("CG_LOOP:  loop will be fully unrolled."));

  BB *head =  Loop_header();

  INT loop_size = 0;
  OP *op;
  INT num_prefetches = 0;
  FOR_ALL_BB_OPs(head, op) {
    loop_size++;
    if (OP_prefetch(op)) 
      num_prefetches++;
  }
  
  // Prefetches are considered LOH Ops
  Induction_Variables_Removal(*this,
			      false/*mark loh ops*/,
			      false/*mark prefetch as loh*/,
			      Get_Trace(TP_CGLOOP, 2) );

  CG_SCHED_EST *loop_se = CG_SCHED_EST_Create(head, &MEM_local_nz_pool, 
					      SCHED_EST_FOR_UNROLL |
					      SCHED_EST_IGNORE_LOH_OPS |
					      SCHED_EST_IGNORE_PREFETCH);

#ifdef TARG_IA64
  // Model the implicit/explicit prefetching
  for (INT i = 0; i < num_prefetches; i++) 
    CG_SCHED_EST_Add_Op_Resources(loop_se, 
				  SWP_Options.Implicit_Prefetch ? 
				  TOP_adds : TOP_lfetch);
#endif

  CG_SCHED_EST *additional_se = CG_SCHED_EST_Create(head, &MEM_local_nz_pool, 
						    SCHED_EST_FOR_UNROLL |
						    SCHED_EST_IGNORE_LOH_OPS |
						    SCHED_EST_IGNORE_BRANCH |
						    SCHED_EST_IGNORE_PREFETCH);

  INT min_unr = SWP_Options.Min_Unroll_Times;
  INT max_unr = SWP_Options.Max_Unroll_Times;
  INT min_recurrence = num_prefetches > 0 && SWP_Options.Implicit_Prefetch ?
    4 : 0;

  const bool swp_trace = Get_Trace(TP_SWPIPE, 2);
  vector<double> swp_cycles(SWP_Options.Max_Unroll_Times+1, 0.0);
  INT i;
  for (i = min_unr; i <= max_unr; i++) {
    swp_cycles[i] = CG_SCHED_EST_Resource_Cycles(loop_se) * (1.0 / i);
    if (swp_trace) {
      fprintf(TFile, "<ti resource count> %d: ", i);
      TI_RES_COUNT_Print(TFile, loop_se->res_count);
      fprintf(TFile, "\n");
    }
    CG_SCHED_EST_Append_Scheds(loop_se, additional_se);
  }

  if (swp_trace)
    for (i = min_unr; i <= max_unr; i++) {
      fprintf(TFile, "<swp unroll resource>  swp_cycles[%d] = %g\n", i, swp_cycles[i]);
    }

  // take into account of the recurrence
  if (min_recurrence != 0) 
    for (i = min_unr; i <= max_unr; i++) {
      if (swp_cycles[i] < (double) min_recurrence / i)
	swp_cycles[i] = (double) min_recurrence / i;
    }

  if (swp_trace)
    for (i = min_unr; i <= max_unr; i++) {
      fprintf(TFile, "<swp unroll resource + recur>  swp_cycles[%d] = %g\n", i, swp_cycles[i]);
    }
  
  INT unroll_times = SWP_Options.Min_Unroll_Times;
  INT loop_size_limit = MIN(SWP_OPS_LIMIT, CG_LOOP_unrolled_size_max);
  for (i = min_unr; i <= max_unr; i++) {
    if (i * loop_size < loop_size_limit) {
      if (swp_cycles[i] < (swp_cycles[unroll_times] * (1.0 - (i - unroll_times) * 0.01)))
	unroll_times = i;
    }
  }

  if (swp_trace) 
    fprintf(TFile, "<swp unroll factor>  swp_cycles[%d] = %g\n", unroll_times, swp_cycles[unroll_times]);
  
  Set_unroll_factor(unroll_times);

  MEM_POOL_Pop(&MEM_local_nz_pool);
}

void CG_LOOP::EBO_Before_Unrolling()
{
  MEM_POOL_Push(&MEM_local_pool);

  {
    BB_REGION bb_region(&MEM_local_pool);
    bb_region.entries.push_back(Loop_header());
    bb_region.exits.push_back(Epilog_start());
    bb_region.Set_has_omega();

    // verify that all OPs in the region has omega and other invariants ...
    bb_region.Verify();  

    EBO_before_unrolling(&bb_region);
  }

  MEM_POOL_Pop(&MEM_local_pool);
}

void CG_LOOP::EBO_After_Unrolling()
{
  MEM_POOL_Push(&MEM_local_pool);
  {
    BB_REGION bb_region(&MEM_local_pool);
    bb_region.entries.push_back(Prolog_start());
    BBLIST *succs;
    FOR_ALL_BB_SUCCS(Epilog_end(), succs) {
      BB *succ = BBLIST_item(succs);
      bb_region.exits.push_back(succ);
    }

    // TODO: call ebo_after_unrolling(bb_region) here
    EBO_after_unrolling(&bb_region);
  }
  MEM_POOL_Pop(&MEM_local_pool);
}

void CG_LOOP::Print(FILE *fp)
{
  for (BB *bb = epilog_start; ; bb = BB_next(bb)) {
    Print_BB_No_Srclines(bb);
    if (bb == epilog_end)
      break;
  }
}


static BOOL Skip_Loop_For_Reason(LOOP_DESCR *loop)
{
  char	*reason = NULL;
  BB	*bb;
  BOOL trace_general = Get_Trace(TP_CGLOOP, 1);
  bool has_exit = FALSE;

  FOR_ALL_BB_SET_members(LOOP_DESCR_bbset(loop), bb)
  {
    if (BB_compile(bb) == FALSE)
    {
      reason = "contains BB from already-scheduled region";
      break;
    }
#ifdef TARG_ST
    BBKIND bbkind = BB_kind(bb);
    if (!(bbkind == BBKIND_CALL || bbkind == BBKIND_GOTO || bbkind == BBKIND_LOGIF)) {
      reason = "contains BB with unsupported branch instruction";
      break;
    }

#if 0
    // FdF 20050909: Support for duplication of ASM instructions has
    // been enabled.

    /* DDTS 17117: Duplication of ASM instructions may not be possible. */
    if (BB_asm(bb)) {
      reason = "contains BB with ASM instruction";
      break;
    }
#endif

    BBLIST *succs;
    FOR_ALL_BB_SUCCS(bb, succs) {
      BB *succ = BBLIST_item(succs);
      if (!BB_SET_MemberP(LOOP_DESCR_bbset(loop), succ))
	has_exit = TRUE;
    }
#endif
  }
	  
  if (!reason)
  {
    BB *head = LOOP_DESCR_loophead(loop);
    TN *trip_count = CG_LOOP_Trip_Count(loop);
    LOOPINFO *info = LOOP_DESCR_loopinfo(loop);

    if (info && WN_Loop_Unimportant_Misc(LOOPINFO_wn(info)))
    {
      reason = "marked UNIMPORTANT_MISC";
    }
    else if (info &&
	    !CG_LOOP_optimize_lno_winddown_cache &&
	     WN_Loop_Winddown_Cache(LOOPINFO_wn(info)))
    {
      reason = "marked WINDDOWN_CACHE; see -CG:opt_lno_winddown_cache";
    }
    else if (info &&
	    !CG_LOOP_optimize_lno_winddown_reg &&
	     WN_Loop_Winddown_Reg(LOOPINFO_wn(info)))
    {
      reason = "marked WINDDOWN_REG; see -CG:opt_lno_winddown_reg";
    }
    else if (!CG_LOOP_optimize_non_trip_countable &&
	      trip_count == NULL)
    {
      reason = "not trip-countable; see -CG:opt_non_trip_countable";
    }
    else if (!CG_LOOP_optimize_non_innermost &&
	     !BB_innermost(head))
    {
      reason = "not innermost loop; see -CG:opt_non_innermost";
    }
#ifdef MIPS_UNROLL
    else if (!CG_LOOP_Attach_Prolog_And_Epilog(loop))
    {
      reason = "loop has multiple back edges";
    }
#endif
    else if (!BB_innermost(head))
    {
      if (trace_general) {
	#pragma mips_frequency_hint NEVER
	fprintf(TFile, "<cgprep> no further optimizations done for non-innermost loops\n");
      }
      return TRUE;
    }
#ifdef MIPS_UNROLL
    else if (!CG_LOOP_epilog && !CG_LOOP_optimize_multi_targ)
    {
      reason = "loop exits to multiple targets; see -CG:opt_multi_targ";
    }
#endif
    else if ( BBlist_Fall_Thru_Succ(head) == NULL)
    {
      reason = "loop never exits";
    }
#ifdef TARG_ST
    /* Because the test to detect loops with no exit is not sufficient
       for multi bb loop. */
    else if (!has_exit)
    {
      reason = "loop never exits";
    }
#endif
  }
	
  if (reason)
  {
    if (trace_general) {
      #pragma mips_frequency_hint NEVER
      fprintf(TFile, "<cgprep> rejecting: %s\n", reason);
    }
    return TRUE;
  }
  return FALSE;
}


/* 
   This function is a filter function used to avoid processing large loops
   bodies. The heuristics used exactly match the ones used in if-conversion 
   (phase-2), a pre-cursor to SWP. TODO: Need to fix sched_est interface to
   make it more efficient.
*/
static BOOL Loop_Amenable_For_SWP(LOOP_DESCR *loop, BOOL trace)
{
  BB *bb;
  UINT32 bb_ctnt = 0;
  UINT32 insts_ctnt = 0;
  
  FOR_ALL_BB_SET_members(LOOP_DESCR_bbset(loop), bb) {
    bb_ctnt++;
    insts_ctnt += BB_length(bb);
    if (bb_ctnt > CG_maxblocks ||
	insts_ctnt > CG_maxinss) {
      if (trace) 
	fprintf(TFile,"<loop> swp disabled: Loop body size too big");
      return FALSE;
    }
  }

  /* Estimate the number of cycles in the loop before full if-conversion. 
   * This serves as a upper-bound (max_ii) for SWP to beat. Sometimes, SWP 
   * ends up generating schedules which are more worse than otherwise.
   * For more details, see 583256, 582711, 583446.
   * Currently, the sched_interface doesn't seem to handle loops with
   * internal cycles (?). Return INT32_MAX in those cases.
   */
  
  double cycles_before_if_cvt =  LOOP_DESCR_Estimate_Cycles (loop);
  INT32 old_num_loop_bbs = BB_SET_Size(LOOP_DESCR_bbset(loop));

  // If loh_ops is removed, we can get more accurate schedule estimate.
  // But don't unless loh_ops are removed from the consideration of
  // LOOP_DESCR_Estimate_Cycles as well.
  MEM_POOL_Push(&MEM_local_nz_pool);

  BB *head =  LOOP_DESCR_loophead(loop);
  BB_SET *bbs = LOOP_DESCR_bbset(loop);
  CG_SCHED_EST *loop_se = CG_SCHED_EST_Create(head, &MEM_local_nz_pool, 
					      SCHED_EST_FOR_UNROLL);
  FOR_ALL_BB_SET_members(bbs, bb) {
    if (bb != head) {
      CG_SCHED_EST *se = CG_SCHED_EST_Create(bb, &MEM_local_nz_pool, 
					     SCHED_EST_FOR_UNROLL);
      CG_SCHED_EST_Append_Scheds(loop_se, se);
    }
  }
  
  double estimate_swp_cycles = CG_SCHED_EST_Resource_Cycles(loop_se);

  MEM_POOL_Pop(&MEM_local_nz_pool);
  
  if (estimate_swp_cycles <= cycles_before_if_cvt) {
    if (trace) { 
      #pragma mips_frequency_hint NEVER
      fprintf(TFile,
	      "<loop> swp enabled: estimate_swp_cycles=%g <= cycles_before_if_cvt=%g",
	      estimate_swp_cycles, cycles_before_if_cvt);
    }
    return TRUE;
  } else {
    if (trace) {
      #pragma mips_frequency_hint NEVER
      fprintf(TFile, 
	      "<loop> swp disabled: estimate_swp_cycles=%g > cycles_before_if_cvt=%g",
	      estimate_swp_cycles, cycles_before_if_cvt);
    }
    return FALSE;
  }
}

/* ====================================================================
 *   Gen_Counted_Loop_Branch
 *
 *   If architecture supports counted loop branches, replace the 
 *   loop-back branch with the counted loop branch instruction. 
 *   It is a nop for the MIPS architecture and ST200.
 * ====================================================================
 */
void 
Gen_Counted_Loop_Branch(
  CG_LOOP& cl
)
{
  TN *trip_count_tn = cl.Trip_count_tn();
  BB *prolog = cl.Prolog_end();
  BB *head = cl.Loop_header();
  BB *tail = BB_Other_Predecessor(head, prolog);

  OPS ops = OPS_EMPTY;
  OPS body_ops = OPS_EMPTY;
  OP *br_op = BB_branch_op(tail);

  // Already converted into counted loop!
#ifdef TARG_ST
  if (OP_Is_Counted_Loop(br_op)) return;
#else
  if (CGTARG_OP_is_counted_loop(br_op)) return;
#endif

  TN *label_tn = OP_opnd(br_op, Branch_Target_Operand(br_op));

#ifdef TARG_ST
  if (CGTARG_Generate_Branch_Cloop(br_op, trip_count_tn, trip_count_tn, 1,
				   label_tn, &ops, &body_ops)) {
#else
  CGTARG_Generate_Branch_Cloop(br_op, trip_count_tn, trip_count_tn, 1,
			       label_tn, &ops, &body_ops);
  if (OPS_length(&body_ops) > 0) {
#endif
    BB_Remove_Op(tail, br_op);
    BB_Append_Ops(tail, &body_ops);
    // Insert loop counter initialization to prolog
    BB_Append_Ops(prolog, &ops);
  }
}

//  Fix backpatches.  Some backpatches are obsoleted because
//  EBO and other optimizations has deleted the def and uses
//
static inline std::pair<BB*, CG_LOOP_BACKPATCH *>
make_pair(BB* a, CG_LOOP_BACKPATCH* b)
{
  return std::pair<BB*, CG_LOOP_BACKPATCH *>(a, b);
}

void Fix_Backpatches(CG_LOOP& cl, bool trace)
{
  std::vector<std::pair<BB*, CG_LOOP_BACKPATCH *> > dead_bp;
  std::set<TN*> epilog_tns;
  BB *body = cl.Loop_header();
  BB *prolog = CG_LOOP_prolog;
  BB *epilog = CG_LOOP_epilog;
  CG_LOOP_BACKPATCH *bp;
  for (bp = CG_LOOP_Backpatch_First(epilog, NULL); bp; bp = CG_LOOP_Backpatch_Next(bp)) {
    TN *body_tn = CG_LOOP_BACKPATCH_body_tn(bp);
    if (!GTN_SET_MemberP(BB_defreach_out(body), body_tn))
      dead_bp.push_back(std::make_pair(epilog,bp));
    else
      epilog_tns.insert(body_tn);
  }
  for (bp = CG_LOOP_Backpatch_First(prolog, NULL); bp; bp = CG_LOOP_Backpatch_Next(bp)) {
    TN *body_tn = CG_LOOP_BACKPATCH_body_tn(bp);
    if (!GTN_SET_MemberP(BB_live_in(body), body_tn) &&
	!(GTN_SET_MemberP(BB_defreach_out(body), body_tn) &&
	  epilog_tns.find(body_tn) != epilog_tns.end()))
      dead_bp.push_back(std::make_pair(prolog,bp));
  }
  while (!dead_bp.empty()) {
    BB *bb = (*(dead_bp.end()-1)).first;
    bp = (*(dead_bp.end()-1)).second;
    if (trace) {
      TN *body_tn = CG_LOOP_BACKPATCH_body_tn(bp);
      fprintf(TFile, "Fix_Backpatches: delete backpatches with body TN%d\n", 
	      TN_number(body_tn));
    }
    CG_LOOP_Backpatch_Delete(bb, bp);
    dead_bp.pop_back();
  }
}

// Perform loop optimizations for one loop
//
BOOL CG_LOOP_Optimize(LOOP_DESCR *loop, vector<SWP_FIXUP>& fixup)
{
  enum LOOP_OPT_ACTION {
    NO_LOOP_OPT,
    SINGLE_BB_DOLOOP_SWP,
    SINGLE_BB_DOLOOP_UNROLL,
    SINGLE_BB_WHILELOOP_SWP,
    SINGLE_BB_WHILELOOP_UNROLL,
    MULTI_BB_DOLOOP,
    MULTI_BB_WHILELOOP
  };

  //    if (Is_Inner_Loop(loop)) {
  if (!BB_innermost(LOOP_DESCR_loophead(loop))) 
    return FALSE;

  if (Skip_Loop_For_Reason(loop))
    return FALSE;

  BOOL trace_loop_opt = Get_Trace(TP_CGLOOP, 0x4);
 
  // Determine how to optimize the loop
  //
  LOOP_OPT_ACTION action = NO_LOOP_OPT;
  BOOL has_trip_count = CG_LOOP_Trip_Count(loop) != NULL;
  BOOL single_bb = (BB_SET_Size(LOOP_DESCR_bbset(loop)) == 1);

  if (trace_loop_opt) {
    if (!single_bb) {
      fprintf(TFile, "loop is not a single BB loop.\n");
      BB_SET_Print(LOOP_DESCR_bbset(loop), TFile);
      fprintf(TFile, "\n");
    }
  }

  if (!single_bb && 
      CG_LOOP_force_ifc > 0 &&
      Loop_Amenable_For_SWP(loop, trace_loop_opt)) {

    BB *new_single_bb;
#ifdef TARG_ST
    // Arthur: if-conversion not implemented yet
    new_single_bb = NULL;
#else
    new_single_bb = Force_If_Convert(loop, CG_LOOP_force_ifc >=2);
#endif
    if (new_single_bb) {
      single_bb = TRUE;
    }
  }

  if (single_bb) {
#ifdef TARG_ST
    if (has_trip_count && !CG_LOOP_unroll_do_unwind)
#else
    if (has_trip_count)
#endif
    {
      if (Enable_SWP)
	action =  SINGLE_BB_DOLOOP_SWP;
      else 
	action =  SINGLE_BB_DOLOOP_UNROLL;
    } else {
      if (Enable_SWP && SWP_Options.Enable_While_Loop)
	action = SINGLE_BB_WHILELOOP_SWP;
      else if (CG_LOOP_unroll_non_trip_countable) 
	action = SINGLE_BB_WHILELOOP_UNROLL;
    }
  } else if (has_trip_count) {
    action = MULTI_BB_DOLOOP;
  } else {
    action = MULTI_BB_WHILELOOP;
  }

  switch (action) {

  case SINGLE_BB_DOLOOP_SWP:
    {
      CG_LOOP cg_loop(loop);
      if (!cg_loop.Has_prolog_epilog()) 
	return FALSE;
      
      if (trace_loop_opt) 
	CG_LOOP_Trace_Loop(loop, "*** Before loop canonicalization ***");

      if (!Prepare_Loop_For_SWP_1(cg_loop, trace_loop_opt))
	return FALSE;

      if (PROC_has_counted_loops())
	// Replace regular branch with loop-count branches.
	// There will be a call EBO to delete the loop-exit test evaluations.
	Gen_Counted_Loop_Branch(cg_loop);

#ifndef TARG_ST
      // Generate SWP branches, eg. doing stuff to prolog/epilog, etc.
      // target-specific
      Gen_SWP_Branch(cg_loop, true /* is_doloop */);
#endif

      cg_loop.Recompute_Liveness();
#ifdef TARG_ST
      // [CG]: Use new interface.
      GRA_LIVE_Rename_TNs_For_BB(cg_loop.Loop_header());
#else
      Rename_TNs_For_BB(cg_loop.Loop_header(), NULL);
#endif
      cg_loop.Recompute_Liveness();

      cg_loop.Build_CG_LOOP_Info();
      cg_loop.Recompute_Liveness();

      if (cg_loop.Determine_Unroll_Fully()) {
	Unroll_Do_Loop_Fully(loop, cg_loop.Unroll_factor());
	cg_loop.Recompute_Liveness();
	return TRUE;
      }

      Perform_Read_Write_Removal(loop);
      cg_loop.Recompute_Liveness();

      if (trace_loop_opt) 
	CG_LOOP_Trace_Loop(loop, "*** Before Postincr generation / After RW removal ***");

      //  Form postincr form
      if (!Prepare_Loop_For_SWP_2(cg_loop, trace_loop_opt))
	return FALSE;

      if (trace_loop_opt) 
	CG_LOOP_Trace_Loop(loop, "*** Before Fix_Recurrences / After Postincr ***");

      // Break recurrences will compute dep-graph itself
      Induction_Variables_Removal(cg_loop, 
				  true/*delete*/, 
				  true/*keep prefetch*/,
				  trace_loop_opt);
      Fix_Recurrences_Before_Unrolling(cg_loop);

      if (trace_loop_opt) 
	CG_LOOP_Trace_Loop(loop, "*** before ebo 1 and unrolling / after fix recurrences ***");

      cg_loop.Recompute_Liveness();
      cg_loop.EBO_Before_Unrolling();  

      if (SWP_Options.Predicate_Promotion) {
	std::list<BB*> bbl;
	bbl.push_front(cg_loop.Loop_header());
	CG_DEP_Prune_Dependence_Arcs(bbl, TRUE, trace_loop_opt);
	if (trace_loop_opt) 
	  CG_LOOP_Trace_Loop(loop, "*** after ebo 1 and prune predicate / before unrolling ***");
      }

      cg_loop.Determine_SWP_Unroll_Factor();

      if (cg_loop.Unroll_factor() > 1) {
	Unroll_Do_Loop(cg_loop, cg_loop.Unroll_factor());
	cg_loop.Recompute_Liveness();
      }

      if (trace_loop_opt) 
	CG_LOOP_Trace_Loop(loop, "*** Before ebo 2/ after unrolling  ***");

      cg_loop.EBO_Before_Unrolling();

      BB_Verify_OP_Order(cg_loop.Loop_header());

      // Break recurrences will compute dep-graph itself
      Fix_Recurrences_After_Unrolling(cg_loop);

      if (trace_loop_opt) 
	CG_LOOP_Trace_Loop(loop, "*** Before implicit prefetch / after fix recurrences 2 ***");

#ifndef TARG_ST
      Gen_Implicit_Prefetches(cg_loop, trace_loop_opt);
#endif

      if (trace_loop_opt) 
	CG_LOOP_Trace_Loop(loop, "*** Before Ind. Var. Removal / after implicit prefetch  ***");

      // Update backpatches
      cg_loop.Recompute_Liveness();
      Induction_Variables_Removal(cg_loop, 
				  true/*delete*/, 
				  true/*keep prefetch*/,
				  trace_loop_opt);
      cg_loop.Recompute_Liveness();
      Fix_Backpatches(cg_loop, trace_loop_opt);

      if (trace_loop_opt) 
	CG_LOOP_Trace_Loop(loop, "*** Before swp / after Ind. Var. Removal  ***");

#ifndef TARG_ST
      if (!Perform_SWP(cg_loop, fixup, true /*doloop*/)) {
	Undo_SWP_Branch(cg_loop, true /*is_doloop*/);
	CG_LOOP_Remove_Notations(loop, CG_LOOP_prolog, CG_LOOP_epilog);
	cg_loop.Recompute_Liveness();
      }
#endif

      break;
    }

  case  SINGLE_BB_DOLOOP_UNROLL:  
    {
      CG_LOOP cg_loop(loop);
      if (!cg_loop.Has_prolog_epilog()) 
	return FALSE;

      if (trace_loop_opt) 
	CG_LOOP_Trace_Loop(loop, "*** Before LOOP CANONICALIZATION ***");
      // Perform_Read_Write_Removal requires that
      //  non-definite dependence are removed
      if (!Remove_Non_Definite_Dependence(cg_loop, false, trace_loop_opt))
	return FALSE;

      cg_loop.Build_CG_LOOP_Info();

      if (trace_loop_opt) 
	CG_LOOP_Trace_Loop(loop, "*** Before SINGLE_BB_DOLOOP_UNROLL ***");

      // Already in Build_CG_LOOP_Info()
      //      cg_loop.Recompute_Liveness();

#ifdef TARG_ST
      BOOL can_be_packed = FALSE;
      if (CG_LOOP_load_store_packing)
	can_be_packed = IVS_Analyze_Load_Store_Packing(cg_loop);

      if (CG_LOOP_unroll_remainder_after || cg_loop.Remainder_after())
	Unroll_Update_Loop_Counter(cg_loop);

      if (cg_loop.Peel_stream())
	Unroll_Peel_Loop(cg_loop);
#endif

#ifdef TARG_ST
      // FdF 20060207: Use scheduling estimate to adjust the unrolling
      // factor
      cg_loop.Determine_Sched_Est_Unroll_Factor();
#else
      cg_loop.Determine_Unroll_Factor();
#endif
      //      printf("DO_LOOP_UNROLL: factor = %d\n", cg_loop.Unroll_factor());
      if (cg_loop.Unroll_fully()) {
	// No need to call RW removal because EBO
	// should find all such CSEs after full unrolling.
	Unroll_Do_Loop_Fully(loop, cg_loop.Unroll_factor());

      } else {

	Perform_Read_Write_Removal(loop);

	// Break recurrences will compute dep-graph itself
	Fix_Recurrences_Before_Unrolling(cg_loop);

	if (cg_loop.Unroll_factor() > 1) {
	  Unroll_Do_Loop(cg_loop, cg_loop.Unroll_factor());
	}

	cg_loop.Recompute_Liveness();
	CG_LOOP_Remove_Notations(loop, CG_LOOP_prolog, CG_LOOP_epilog);
      }

      cg_loop.Recompute_Liveness();
      cg_loop.EBO_After_Unrolling();

#ifdef TARG_ST
      if (can_be_packed && !cg_loop.Unroll_fully()) { 
	//	CG_LOOP_prolog = cg_loop.Prolog_start();
	//	CG_LOOP_epilog = cg_loop.Epilog_end();
	//	cg_loop.Build_CG_LOOP_Info();
	/* With flag 0x8, only perform loop transformation, but do not
	   actually perform loop packing. */
	if ((CG_LOOP_load_store_packing&0x8) == 0) {
	  // Perform loop specialization before loop packing. Loop
	  // specialization can be performed even if loop unrolling is
	  // not performed (even_factor was not necessary).
	  if (cg_loop.Special_streams() > 0) {
	    // Duplicate the loop body, then add checks to select the
	    // normal or the to be packed loop version.
	    cg_loop.Unroll_Specialize_Loop();
	    cg_loop.Recompute_Liveness();
	  }
	  IVS_Perform_Load_Store_Packing(cg_loop);
	}

	cg_loop.Recompute_Liveness();
	Induction_Variables_Removal(cg_loop, 
				    true/*delete*/, 
				    true/*keep prefetch*/,
				    trace_loop_opt);
	cg_loop.Recompute_Liveness();
      }
#endif
    }

    break;

  case SINGLE_BB_WHILELOOP_SWP:
    {
      CG_LOOP cg_loop(loop);

      // Prolog and Epilog are needed for SWP loop.
      if (!cg_loop.Has_prolog_epilog()) return FALSE;

      if (trace_loop_opt) 
	CG_LOOP_Trace_Loop(loop, "*** Before LOOP CANONICALIZATION ***");

      if (Prepare_Loop_For_SWP_1(cg_loop, trace_loop_opt)) {
#ifndef TARG_ST
	Gen_SWP_Branch(cg_loop, false/*is_doloop*/);
#endif

	cg_loop.Recompute_Liveness();
	cg_loop.Build_CG_LOOP_Info();

	// Break recurrences will compute dep-graph itself
	cg_loop.Recompute_Liveness();
	Induction_Variables_Removal(cg_loop, 
				    true/*delete*/, 
				    true/*keep prefetch*/,
				    trace_loop_opt);
	cg_loop.Recompute_Liveness();
	Prepare_Loop_For_SWP_2(cg_loop, trace_loop_opt);

#ifndef TARG_ST
	Convert_While_Loop_to_Fully_Predicated_Form(cg_loop);

	if (SWP_Options.Predicate_Promotion) {
	  std::list<BB*> bbl;
	  bbl.push_front(cg_loop.Loop_header());
	  CG_DEP_Prune_Dependence_Arcs(bbl, TRUE, trace_loop_opt);
	}
#endif

	if (trace_loop_opt) 
	  CG_LOOP_Trace_Loop(loop, "*** Before SINGLE_BB_WHILELOOP_SWP ***");

	cg_loop.Recompute_Liveness();
	Fix_Backpatches(cg_loop, trace_loop_opt);

#ifndef TARG_ST
	if (!Perform_SWP(cg_loop, fixup, false /*doloop*/)) {
	  
	  //  Undo SWP preparations, if SWP failed
	  Undo_SWP_Branch(cg_loop, false/*is_doloop*/);
	  CG_LOOP_Remove_Notations(loop, CG_LOOP_prolog, CG_LOOP_epilog);
	  cg_loop.Recompute_Liveness();
	}
#endif
      }
    }
    break;


  case SINGLE_BB_WHILELOOP_UNROLL:
    {
      CG_LOOP cg_loop(loop);

      if (trace_loop_opt) 
	CG_LOOP_Trace_Loop(loop, "*** Before SINGLE_BB_WHILELOOP_UNROLL ***");

      cg_loop.Build_CG_LOOP_Info();
      cg_loop.Determine_Unroll_Factor();
      // FdF 20060207: Divide the unrolling factor on while loops.
      if ((CG_LOOP_unroll_heuristics & DOWHILE_HEURISTIC) &&
	  (cg_loop.Unroll_factor() > 1))
	cg_loop.Set_unroll_factor(cg_loop.Unroll_factor()/2);
      Unroll_Dowhile_Loop(loop, cg_loop.Unroll_factor());
      cg_loop.Recompute_Liveness();
      cg_loop.EBO_After_Unrolling();

    }
    break;

  case MULTI_BB_DOLOOP:
  case MULTI_BB_WHILELOOP:
    {
      CG_LOOP cg_loop(loop);
      ANNOTATION *pragma_unroll;

      // Prolog is needed for multi_bb loop.
      if (!cg_loop.Has_prolog()) return FALSE;

      if (trace_loop_opt) {
	CG_LOOP_Trace_Loop(loop, "*** Before MULTI_BB_DOLOOP ***");
      }

#ifdef TARG_ST
      if (PROC_has_counted_loops())
#endif
      Gen_Counted_Loop_Branch(cg_loop);

      cg_loop.Determine_Unroll_Factor();
      // FdF 20060207: Divide the unrolling factor on while loops.
      if ((action == MULTI_BB_WHILELOOP) &&
	  (CG_LOOP_unroll_heuristics & DOWHILE_HEURISTIC) &&
	  (cg_loop.Unroll_factor() > 1))
	cg_loop.Set_unroll_factor(cg_loop.Unroll_factor()/2);
      if (cg_loop.Unroll_factor() > 1) {
#ifdef TARG_ST
	/* FdF: We cannot rely on prolog/epilog to create the frontier
           for the loop since the loop may contain multiple exits. So
           add specific code here, from Recompute_Liveness. */

	BB *entry = cg_loop.Trip_count_bb() ? cg_loop.Trip_count_bb() : cg_loop.Prolog_start();
	BB_REGION region(Malloc_Mem_Pool);
	region.entries.push_back(entry);

	BB *bb;
	FOR_ALL_BB_SET_members(LOOP_DESCR_bbset(loop), bb) {
	  BBLIST *succs;
	  FOR_ALL_BB_SUCCS(bb, succs) {
	    BB *succ = BBLIST_item(succs);
	    if (!BB_SET_MemberP(LOOP_DESCR_bbset(loop), succ))
	      region.exits.push_back(succ);
	  }
	}

	unroll_multi_bb(loop, cg_loop.Unroll_factor());

	BB_REGION_Recompute_Global_Live_Info(region, TRUE /* recompute local info */);
#else
	unroll_multi_bb(loop, cg_loop.Unroll_factor());
#endif
      }

      if (trace_loop_opt) {
	CG_LOOP_Trace_Loop(loop, "*** After MULTI_BB_DOLOOP ***");
      }
    }
    break;

  case NO_LOOP_OPT:
    return FALSE;

  default:
    Is_True(FALSE, ("unknown loop opt action."));
  }

  return TRUE;
}


/*
**	Obey -CG:skip_local_loop
*/
BOOL CG_LOOP_Skip(BB *bb)
{
  BOOL skip = FALSE;
  if (CG_skip_local_loop) {
	skip = (BB_id(bb) < CG_local_skip_before ||
		BB_id(bb) > CG_local_skip_after ||
		BB_id(bb) == CG_local_skip_equal);
	if (skip) DevWarn("skip CG loop at BB %d", BB_id(bb));
	else DevWarn("process CG loop at BB %d", BB_id(bb));
  }
  return skip;
}


void CG_LOOP_Statistics(LOOP_DESCR *loop)
{
  BB *head = LOOP_DESCR_loophead(loop);
  BB_SET *bbs = LOOP_DESCR_bbset(loop);
  TN *trip_count = CG_LOOP_Trip_Count(loop);
  bool inner = BB_innermost(head);
  bool has_call = false;
  bool early_exit = false;
  bool multi_exit = false;
  bool has_return = false;
  INT32  nbb= BB_SET_Size(bbs);
  INT32  nops = 0;
  BB *bb;
  FOR_ALL_BB_SET_members(bbs, bb) {
    if (BB_call(bb))
      has_call = true;
    nops += BB_length(bb);
  }

  BBLIST *succs;
  BB *loop_merge = NULL;
  FOR_ALL_BB_SUCCS(head, succs) {
    BB *succ = BBLIST_item(succs);
    if (!BB_SET_MemberP(bbs, succ)) {
      loop_merge = succ;
    }
  }
  FOR_ALL_BB_SET_members(bbs,bb) {
    if (bb != head) {
      FOR_ALL_BB_SUCCS(bb, succs) {
	BB *succ = BBLIST_item(succs);
	if (!BB_SET_MemberP(bbs, succ)) {
	  early_exit = true;
	  if (succ != loop_merge)
	    if (BB_exit(succ)) 
	      has_return = true;
	    else
	      multi_exit = true;
	}
      }
    }
  }

  fprintf(TFile, "<loopstat> %s %s\n", 
	  inner ? "inner" : "no_inner",
	  trip_count ? "has_loop_count" : "no_loop_count");
  if (inner) {
    fprintf(TFile, "<loopstat_inner> nbb=%d nops=%d %s %s %s %s\n",
	    nbb, nops,
	    has_call ? "has_call" : "no_call",
	    has_return ? "has_return" : "no_return",
	    early_exit ? "has_early_exit" : "no_early_exit",
	    multi_exit ? "has_multi_exit" : "no_multi_exit");
  }
}



// Perform loop optimizations for all inner loops
// in the PU.
//
void Perform_Loop_Optimizations()
{
  MEM_POOL loop_descr_pool;
  MEM_POOL_Initialize(&loop_descr_pool, "loop_descriptors", TRUE);
  MEM_POOL_Push (&loop_descr_pool);
  BOOL trace_general = Get_Trace(TP_CGLOOP, 1);

  SWP_Options.PU_Configure();

  Calculate_Dominators();		/* needed for loop recognition */

  SWP_FIXUP_VECTOR fixup;

  for (LOOP_DESCR *loop = LOOP_DESCR_Detect_Loops(&loop_descr_pool);
       loop;
       loop = LOOP_DESCR_next(loop)) {
    BB *head = LOOP_DESCR_loophead(loop);
	  
    if (CG_LOOP_Skip(head)) {
      DevWarn("CG_LOOP skip BB%d.", BB_id(head));
      continue;
    }
      
    if (trace_general)
      {
        #pragma mips_frequency_hint NEVER
	fprintf(TFile, "%s<cgprep> %sloop line %d, BBs ",
		DBar, BB_innermost(head) ? "innermost " : "",
		BB_Loop_Lineno(head));
	BB_SET_Print(LOOP_DESCR_bbset(loop), TFile);
	fprintf(TFile, ", head BB:%d, nest level %d\n",
		BB_id(head), LOOP_DESCR_nestlevel(loop));
      }

    if (trace_general)
      CG_LOOP_Statistics(loop);

    // CG_LOOP_Optimize adds fixup requirement to 'fixup'.
    CG_LOOP_Optimize(loop, fixup);
  }

  // Compute correct wrap around values for SWP loops
  for (INT i = 0; i < fixup.size(); i++)
    SWP_Fixup(fixup[i]);

#ifdef HAS_ROTATING_REGISTERS
  if (Enable_SWP) {
    // Let GRA knows how many rotating register are needed!
    REGISTER_Request_Stacked_Rotating_Register();
  }
#endif
  
  MEM_POOL_Pop (&loop_descr_pool);
  MEM_POOL_Delete(&loop_descr_pool);

  Free_Dominators_Memory ();
}
