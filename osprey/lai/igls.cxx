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


// =======================================================================
// =======================================================================
//
//  Module: igls.cxx
//
//  Description:
//  ============
//
//  Integrated Global and Local Scheduling Framework (IGLS). The main driver 
//  controls the execution of the local scheduling phase (LOCS), the
//  hyperblock scheduling phase (HBS) and the global scheduling phase (GCM).
//
// =======================================================================
// =======================================================================

#include "W_alloca.h"
#include "W_math.h"
#include "defs.h"
#include "config.h"
#include "config_TARG.h"
#include "mempool.h"
#include "bb.h"
#include "bb_set.h"
#include "tracing.h"
#include "timing.h"
#include "cgir.h"
#include "glob.h"
#include "tn_map.h"
#include "cg.h"
#include "cg_flags.h"
#include "ercg.h"
#include "cgtarget.h"
#include "cg_vector.h"
#include "dominate.h"
#include "findloops.h"
#include "note.h"
#include "lra.h"
#include "gcm.h"
#include "ti_res.h"
#include "ti_res_res.h"
#include "ti_latency.h"
#include "ti_errors.h"
#include "cg_region.h"
#include "gtn_universe.h"
#include "gtn_set.h"
#include "cxx_memory.h"
#include "hb_sched.h"
#include "hb_hazards.h"
#include "reg_live.h"
#ifdef TARG_ST
#include "freq.h"
#include "whirl2ops.h"
#include "pf_cg.h"
#include "config_cache.h"
#include "cg_dep_graph.h"
#endif
#ifdef LAO_ENABLED
#include "lao_stub.h"
#endif

#ifdef TARG_ST

/* FdF: Find a prefetch instruction in the current basic block which
   is a duplication of op_pref for the iteration number iter. */
static OP *
Get_unrolled_op(OP *op, int iter) {
  if (op) {
    WN *wn = Get_WN_From_Memory_OP(op);
    for (OP *op_next = BB_first_op(OP_bb(op)); op_next; op_next = OP_next(op_next)) {
      if (OP_memory(op_next) && OP_code(op_next) == OP_code(op) &&
	  (Get_WN_From_Memory_OP(op_next) == wn) && !OP_flag1(op_next) && (OP_unrolling(op_next) == iter))
	return op_next;
    }
  }
  return NULL;
}

static OP *
Next_unrolled_op(OP *op, int iter) {
  if (op) {
    WN *wn = Get_WN_From_Memory_OP(op);
    for (OP *op_next = OP_next(op); op_next; op_next = OP_next(op_next)) {
      if (OP_memory(op_next) && OP_code(op_next) == OP_code(op) &&
	  (Get_WN_From_Memory_OP(op_next) == wn) && !OP_flag1(op_next) && (OP_unrolling(op_next) == iter))
	return op_next;
    }
  }
  return NULL;
}

// Returns the number of cycles of a basic block
static int
BB_cycles(BB *bb) {

  // Get the number of cycles in this basic block.
  OP *op = BB_last_op(bb);
  int bb_cycles = op ? OP_scycle(op) : 0;

  if (bb_cycles < 0) {
    // When the last instruction in a basic block is a branch
    // operation, it may not have a valid scheduling date. In this
    // case just use the previous one.
    Is_True (OP_xfer(op), ("bb: %d, None branch instruction have no scheduling date", BB_id(bb)));

    // Assumes that prev_op is scheduled in the same cycle as op
    bb_cycles = OP_prev(op) ? OP_scycle(OP_prev(op)) : 1;
  }

  Is_True(bb_cycles >= 0, ("No valid scheduling info for bb %d\n", BB_id(bb)));

  return bb_cycles;
}

/* FdF: Compute the prefetch distance of PFT instructions, now that
   the number of cycles of loops is known. */

void
Schedule_Prefetch_Prepass () {
  WN_MAP WN_to_OP_map;
  BOOL Trace_PFT = Get_Trace(TP_SCHED, 4);

  if (!(Prefetch_Optimize&0x2)) return;

  Calculate_Dominators ();
  L_Save ();
  LOOP_DESCR *loop_list = LOOP_DESCR_Detect_Loops (&MEM_local_pool);
  BB_SET *processed_bbs = BB_SET_Create_Empty (PU_BB_Count+2, &MEM_local_pool);
  for (LOOP_DESCR *cloop = loop_list;
       cloop != NULL;
       cloop = LOOP_DESCR_next(cloop)) {

    // Do not shedule prefetch instructions when #pragma LOOPSEQ read is
    // used.
    // FdF 20050222: Imported from cg_dep_graph.cxx
    extern UINT32 CG_Get_BB_Loopseq_Mask(BB *bb);
#define LOOPSEQ_READ	1
    BB *loop_head = LOOP_DESCR_loophead(cloop);
    if (CG_Get_BB_Loopseq_Mask(loop_head) & (1<<LOOPSEQ_READ))
      continue;

    WN_to_OP_map = WN_MAP_Create(&MEM_local_pool);

    BB *bb;
    // Compute the number of prefetchs in the loop.
    int nb_prefetch = 0;
    int nb_cycles = CGTARG_Branch_Taken_Penalty(); // Count for the loop back-edge
    BOOL is_inner_loop = TRUE;
    float head_freq = BB_freq(LOOP_DESCR_loophead(cloop));

    /* FdF: Compute the number of prefetchs in the loop, initialize a
       MAP WN_to_OP_map, and compute a mean number of cycles for
       multi-bb loops. */

    FOR_ALL_BB_SET_members (LOOP_DESCR_bbset(cloop), bb) {
      OP *op;

      if (BB_loop_head_bb(bb) != LOOP_DESCR_loophead(cloop)) {
	is_inner_loop = FALSE;
	continue;
      }

#define FLOAT_INT_MUL(f,i) ((INT)(f * (float)i + (float)0.5))
      float freq_ratio = head_freq == 0.0 ? 0.0 : BB_freq(bb)/head_freq;
      nb_cycles +=  FLOAT_INT_MUL (freq_ratio, BB_cycles(bb));

      FOR_ALL_BB_OPs(bb, op) {
	if (OP_prefetch(op)) {
	  nb_prefetch++;
	  if (!Get_WN_From_Memory_OP(op)) {
	    if (Trace_PFT)
	      fprintf(TFile, "No WN associated with a PREFETCH op\n");
	    continue;
	  }
	  Is_True(Get_WN_From_Memory_OP(op), ("No WN associated with a PREFETCH op"));

	  // In case of unrolling, there may be more than one prefetch
	  // for the same WN, from different loop iterations. Just
	  // keep the first one, the other ones will be analyzed in a
	  // specific way.
	  if (!WN_MAP_Get(WN_to_OP_map, Get_WN_From_Memory_OP(op))) {
	    WN_MAP_Set(WN_to_OP_map, Get_WN_From_Memory_OP(op), op);
	    Reset_OP_flag1(op);
	  }
	}
      }
    }

    if (is_inner_loop && nb_prefetch) {
      int nb_prefetched = 0;

      if (Trace_PFT)
	fprintf(TFile, "loop %d cycles, %d prefetchs\n", nb_cycles, nb_prefetch);
      // Compute the maximum number of iterations the prefetch
      // instructions must be computed ahead.
      FOR_ALL_BB_SET_members (LOOP_DESCR_bbset(cloop), bb) {
	OP *op_ref;
	FOR_ALL_BB_OPs(bb, op_ref) {

	  if (!OP_load(op_ref))
	    continue;

	  WN *op_wn = Get_WN_From_Memory_OP(op_ref);
	  if (!op_wn) continue;

	  PF_POINTER *pref_info = (PF_POINTER *)WN_MAP_Get (WN_MAP_PREFETCH, op_wn);
	  if (!pref_info && Trace_PFT) fprintf(TFile, "igls: ref %p, no prefetch found\n", op_wn);
	  if (!pref_info) continue;

	  // TODO: Try to fix the warning in opt_htable.cxx:2895
	  if (!PF_PTR_wn_pref_1L(pref_info)) continue;

	  WN *pref_wn = PF_PTR_wn_pref_1L(pref_info);
	  OP *op_pref = NULL;

	  if (Trace_PFT)
	    fprintf(TFile, "igls: ref %p, found a prefetch wn %p\n", op_wn, pref_wn);

	  op_pref = (OP*) WN_MAP_Get(WN_to_OP_map, pref_wn);

#if 0 // Debug Only
	  PF_POINTER *pf_info = (PF_POINTER *)WN_MAP_Get (WN_MAP_PREFETCH, pref_wn);
	  if (pf_info != pref_info) {
	    printf("memop->memwn->prefinfo != pfwn->pfinfo\n");
	    if (!pf_info || (PF_PTR_wn_pref_1L(pf_info) != pref_wn))
	      printf("!!! prefinfo->pref1L != pfinfo->pref1L\n");
	  }
#endif
	  // Look for the first prefetch operation in this basic block
	  // with the same OP_unrolling
	  op_pref = Get_unrolled_op(op_pref, OP_unrolling(op_ref));
	  if (!op_pref) continue;
	  if (OP_flag1(op_pref)) continue; // Already visited

	  // The prefetch instruction is moved before the first memory
	  // operation that is associated to it in the following
	  // cases:

	  // - The scheduling date of the memory reference is greater
	  // than the prefetch latency. The prefetch can occur in the
	  // same iteration as the memory reference, if placed at the
	  // beginning of the loop.

	  // - The loop trip count and/or the number of cycle of the
	  // loops is so small that any prefetch in the loop will not
	  // prefetch any useful data for this loop.

	  // Otherwise, the prefetch instruction is moved after the
	  // first memory reference that uses it, and prefetch
	  // instructions are inserted in the loop entry.

	  MHD_LEVEL* Cur_Mhd = &Mhd.L[0]; 
	  FmtAssert(Cur_Mhd->Valid(), ("Not a valid MHD level"));

	  Set_OP_flag1(op_pref);
	  Set_OP_pft_scheduled(op_pref);

	  int pf_latency = (int)Cur_Mhd->Clean_Miss_Penalty;
	  if ((OP_bb(op_pref) == bb) && (OP_scycle(op_ref) > pf_latency)) {
	    // Prefetched can be scheduled in the same iteration as
	    // the memory reference.
	    if (OP_Ordering(op_ref, op_pref) < 0) {
	      BB_Remove_Op(bb, op_pref);
	      BB_Insert_Op_Before(bb, op_ref, op_pref);
	      Reset_BB_scheduled (bb);
	    }
	    Set_OP_pft_before(op_pref);
	  }
	  else {
	    // Move the prefetch instruction after the memory
	    // reference associated with it.
	    if ((OP_bb(op_pref) != bb) || (OP_Ordering(op_ref, op_pref) > 0)) {
	      BB_Remove_Op(OP_bb(op_pref), op_pref);
	      BB_Insert_Op_After(bb, op_ref, op_pref);
	      Reset_BB_scheduled (bb);
	      Reset_BB_scheduled (OP_bb(op_pref));
	    }

	    //	    if (Trace_PFT)
	    //	      fprintf(TFile, "Insert %d prefetch at loop entry(pf_stride = %f)\n", WN_pf_stride_1L(pref_wn) ? (int)(iterAhead/pf_stride + 0.99) : 1, pf_stride);
	    // TBD: Insert iterAhead/pf_stride prefetch instructions
	    // before the loop. First, look for a place where to
	    // insert the prefetch instruction. Go up in the dominator
	    // tree.
	  }
	}
      }
    }
    WN_MAP_Delete(WN_to_OP_map);
  }
  L_Free ();
  Free_Dominators_Memory ();
}

/* FdF: Compute the prefetch distance of PFT instructions, now that
   the number of cycles of loops is known. */
void
Schedule_Prefetch_Postpass () {
  WN_MAP WN_to_OP_map;
  BOOL Trace_PFT = Get_Trace(TP_SCHED, 4);

  if (!(Prefetch_Optimize&0x2)) return;

  Calculate_Dominators ();
  L_Save ();
  LOOP_DESCR *loop_list = LOOP_DESCR_Detect_Loops (&MEM_local_pool);
  BB_SET *processed_bbs = BB_SET_Create_Empty (PU_BB_Count+2, &MEM_local_pool);
  for (LOOP_DESCR *cloop = loop_list;
       cloop != NULL;
       cloop = LOOP_DESCR_next(cloop)) {

    WN_to_OP_map = WN_MAP_Create(&MEM_local_pool);

    BB *bb;
    // Compute the number of prefetchs in the loop.
    int nb_prefetch = 0;
    int nb_cycles = CGTARG_Branch_Taken_Penalty(); // Count for the loop back-edge
    BOOL is_inner_loop = TRUE;
    float head_freq = BB_freq(LOOP_DESCR_loophead(cloop));

    /* FdF: Compute the number of prefetchs in the loop, initialize a
       MAP WN_to_OP_map, and compute a mean number of cycles for
       multi-bb loops. */

    FOR_ALL_BB_SET_members (LOOP_DESCR_bbset(cloop), bb) {
      OP *op;

      op = BB_last_op(bb);
      if (!op) continue;
      if (OP_scycle(op) < 0) op = OP_prev(op);
      Is_True(op && OP_scycle(op) >= 0, ("No valid scheduling info for bb %d\n", BB_id(bb)));

      if (BB_loop_head_bb(bb) != LOOP_DESCR_loophead(cloop)) {
	is_inner_loop = FALSE;
	continue;
      }

#define FLOAT_INT_MUL(f,i) ((INT)(f * (float)i + (float)0.5))
      float freq_ratio = head_freq == 0.0 ? 0.0 : BB_freq(bb)/head_freq;
      nb_cycles +=  FLOAT_INT_MUL (freq_ratio, BB_cycles(bb));

      FOR_ALL_BB_OPs(bb, op) {
	if (OP_prefetch(op)) {
	  nb_prefetch++;
	  if (!Get_WN_From_Memory_OP(op)) continue;
	  Is_True(Get_WN_From_Memory_OP(op), ("No WN associated with a PREFETCH op"));

	  // In case of unrolling, there may be more than one prefetch
	  // for the same WN, from different loop iterations. Just
	  // keep the first one, the other ones will be analyzed in a
	  // specific way.
	  if (!WN_MAP_Get(WN_to_OP_map, Get_WN_From_Memory_OP(op))) {
	    WN_MAP_Set(WN_to_OP_map, Get_WN_From_Memory_OP(op), op);
	    Reset_OP_flag1(op);
	  }
	}
      }
    }

    if (is_inner_loop && nb_prefetch) {
      int nb_prefetched = 0;

      if (Trace_PFT)
	fprintf(TFile, "loop %d cycles, %d prefetchs\n", nb_cycles, nb_prefetch);
      // Compute the maximum number of iterations the prefetch
      // instructions must be computed ahead.
      FOR_ALL_BB_SET_members (LOOP_DESCR_bbset(cloop), bb) {
	OP *op_ref;
	FOR_ALL_BB_OPs(bb, op_ref) {

	  //	  if (!OP_load(op_ref) || OP_unrolling(op_ref))
	  if (!OP_load(op_ref))
	    continue;

	  WN *op_wn = Get_WN_From_Memory_OP(op_ref);
	  if (!op_wn) continue;

	  PF_POINTER *pref_info = (PF_POINTER *)WN_MAP_Get (WN_MAP_PREFETCH, op_wn);
	  if (!pref_info && Trace_PFT) fprintf(TFile, "igls: ref %p, no prefetch found\n", op_wn);
	  if (!pref_info) continue;

	  // TODO: Try to fix the warning in opt_htable.cxx:2879
	  if (!PF_PTR_wn_pref_1L(pref_info)) continue;

	  Is_True (PF_PTR_wn_pref_1L(pref_info), ("wn_pref_1L is not set"));
	  WN *pref_wn = PF_PTR_wn_pref_1L(pref_info);
	  OP *op_pref = NULL;

	  if (Trace_PFT)
	    fprintf(TFile, "igls: ref %p, found a prefetch wn %p\n", op_wn, pref_wn);

          if (!WN_pf_stride(pref_wn))
	    continue;

	  op_pref = (OP*) WN_MAP_Get(WN_to_OP_map, pref_wn);
	  op_pref = Get_unrolled_op(op_pref, OP_unrolling(op_ref));
	  if (!op_pref) continue;

	  if (OP_flag1(op_pref)) continue;

	  MHD_LEVEL* Cur_Mhd = &Mhd.L[0]; 
	  FmtAssert(Cur_Mhd->Valid(), ("Not a valid MHD level"));

	  int pf_latency = (int)Cur_Mhd->Clean_Miss_Penalty;
	  if (OP_bb(op_pref) == bb)
	    pf_latency += OP_scycle(op_pref) - OP_scycle(op_ref);

	  float pf_stride = (float)WN_pf_stride_1L(pref_wn) / (float)(BB_unrollings(bb) ? BB_unrollings(bb) : 1);
	  int iterAhead = (int)((pf_latency + nb_cycles - 1) / nb_cycles);

	  // Do not prefetch too much iterations ahead if the loop
	  // trip count is known to be small.
	  LOOPINFO *info = LOOP_DESCR_loopinfo(cloop);
	  INT32 max_iter;

	  TN *trip_count_tn = info ? LOOPINFO_trip_count_tn(info) : NULL;
	  if (trip_count_tn && TN_is_constant(trip_count_tn))
	    max_iter = TN_value(trip_count_tn) / 2;
	  else max_iter = WN_loop_trip_est(LOOPINFO_wn(info)) / 2;
	  if (iterAhead > max_iter)
	    iterAhead = max_iter;

	  // Do not prefetch too much iterations ahead if this will
	  // exceed the number of prefetch buffers.
	  if ((int)(iterAhead / pf_stride + 0.99) * nb_prefetch > Mhd.DCache_Prefetch_Buffers)
	    iterAhead = (int)(Mhd.DCache_Prefetch_Buffers * pf_stride / nb_prefetch);

	  if (iterAhead > Mhd.DCache_Prefetch_Buffers)
	    iterAhead = Mhd.DCache_Prefetch_Buffers;

          int distAhead = iterAhead * (int)(Cur_Mhd->Line_Size / pf_stride + 0.99);
	  if (distAhead < Cur_Mhd->Line_Size) distAhead = Cur_Mhd->Line_Size;
//          else if (ANNOT_Get(BB_annotations(OP_bb(op_pref)), ANNOT_REMAINDERINFO))
//	    distAhead = Cur_Mhd->Line_Size;

	  if (PF_PTR_distance_1L(pref_info) < 0) distAhead *= -1;

	  do {
	    nb_prefetched ++;
	    Set_OP_flag1(op_pref);
	      
	    if (WN_pf_stride_1L(pref_wn)/* || ANNOT_Get(BB_annotations(OP_bb(op_pref)), ANNOT_REMAINDERINFO)*/) {
	      int offset = TN_value(OP_opnd(op_pref, 0));
	      int fixedOffset = TN_value(OP_opnd(op_pref, 0)) + distAhead - PF_PTR_distance_1L(pref_info);
	      if (Trace_PFT) {
		fprintf(TFile, "Prefetch offset is %d\n", fixedOffset);
		if (WN_pf_manual(pref_wn) )
		  if (fixedOffset != offset)
		    fprintf(TFile, "Manual prefetch: Offset was %d\n", offset);
		  else
		    fprintf(TFile, "Manual prefetch: Offset unchanged\n");
	      }
	      Set_OP_opnd(op_pref, 0, Gen_Literal_TN(fixedOffset, 4));
	      TOP etop;
	      if (CGTARG_need_extended_Opcode(op_pref, &etop)) {
		// Reset to previous value.
		Set_OP_opnd(op_pref, 0, Gen_Literal_TN(offset, 4));
		if (CGTARG_need_extended_Opcode(op_pref, &etop))
		  // Can use the new value if olf value also required
		  // extended offset
		  Set_OP_opnd(op_pref, 0, Gen_Literal_TN(fixedOffset, 4));
	      }
	      if (Trace_PFT)
		fprintf(TFile, "Loop cycles %d, iterAhead %d, pft stride %f, distAhead %d\n", nb_cycles, iterAhead, pf_stride, distAhead);
	    }
	    op_pref = Next_unrolled_op(op_pref, OP_unrolling(op_ref));
	  } while (op_pref);
	}
      }
      if (Trace_PFT && nb_prefetched != nb_prefetch) {
	fprintf(TFile, "Found %d prefetchs, but only %d prefetched memory references\n", nb_prefetch, nb_prefetched);
      }
    }
    WN_MAP_Delete(WN_to_OP_map);
  }
  L_Free ();
  Free_Dominators_Memory ();
}
#endif

/* #include "targ_proc_properties.h" */


// ======================================================================
// IGLS_Schedule_Region 
//
// The main driver for invoking all the scheduling phases in CG. They mainly
// include HBS (for single-BBs and hyperblocks) and GCM. The data-speculation
// phase is also invoked here since it's tied very closely with the 
// scheduling phase. 
// The <before_regalloc> parameter indicates whether the scheduler is being 
// invoked before or after register allocation. The amount of work done by 
// the various phases depends on the optimization level. 
//
// -O0 : insert noops to remove hazards.
// -O1 : perform HBS scheduling for local BBs (ONLY) after register allocation.
//       fill branch delay slot nops (for MIPS).
// -O2 : perform hyperblock(s) scheduling before register allocation.
//       provide accurate register estimates for GRA.
//       invoke post-GRA global scheduling (post-GCM) phase
//       invoke THR phase to perform data-speculation (after register-
//       allocation).
// -O3 : perform hyperblock(s) scheduling before register allocation.
//	 provide accurate register estimates for GRA/GCM.
//	 invoke pre-GRA global scheduling (pre-GCM) phase.
//	 invoke post-GRA global scheduling (post-GCM) phase
//
// ======================================================================

void
IGLS_Schedule_Region (BOOL before_regalloc)
{
  BB *bb;
  BOOL should_we_local_schedule;  // controls local scheduling (single BBs).
  BOOL should_we_global_schedule; // controls HB scheduling and GCM.
  BOOL should_we_schedule;        // controls all scheduling (LOCS,  HBS, GCM)
  BOOL should_we_do_thr;          // controls the THR phase in CG.

  RID *rid;
  HBS_TYPE hbs_type;
  HB_Schedule *Sched = NULL;
  CG_THR      *thr = NULL;

  Set_Error_Phase ("Hyberlock Scheduler");
  Start_Timer (T_Sched_CU);
  Trace_HB = Get_Trace (TP_SCHED, 1);
  should_we_schedule = IGLS_Enable_All_Scheduling;
  should_we_do_thr = CG_enable_thr;
  L_Save();

  if (before_regalloc) {

    // schedule if (-O > O1) and
    // -CG:local_sched=on && -CG:pre_local_sched=on.
    should_we_local_schedule = (   CG_opt_level > 1
				   && LOCS_Enable_Scheduling
				   && LOCS_PRE_Enable_Scheduling);

    // global schedule if (-O > O2) and either of the following below are true.
    // -CG:hb_sched=on && -CG:pre_hb_sched=on (hyperblock scheduling).
    // -CG:gcm=on && -CG:pre_gcm=on for GCM.
    should_we_global_schedule = ( CG_opt_level > 2 &&
				  ((IGLS_Enable_HB_Scheduling &&
				    IGLS_Enable_PRE_HB_Scheduling) ||
				   (GCM_PRE_Enable_Scheduling &&
				    GCM_Enable_Scheduling)));

    hbs_type = HBS_BEFORE_GRA | HBS_BEFORE_LRA | HBS_DEPTH_FIRST;
    if (Trace_HB) {
      #pragma mips_frequency_hint NEVER
      fprintf (TFile, "***** HYPERBLOCK SCHEDULER (before GRA) *****\n");
    }
  }
  else {

    // schedule if (-O > O0) and
    // -CG:local_sched=on && -CG:post_local_sched=on.
    should_we_local_schedule = (   CG_opt_level > 0
				   && LOCS_Enable_Scheduling
				   && LOCS_POST_Enable_Scheduling);

    // global schedule if (-O > O1) and either of the following below are true.
    // -CG:hb_sched=on && -CG:post_hb_sched=on (hyperblock scheduling).
    // -CG:gcm=on && -CG:post_gcm=on for GCM.
    should_we_global_schedule = ( CG_opt_level > 1 &&
				  ((IGLS_Enable_HB_Scheduling &&
				   (IGLS_Enable_POST_HB_Scheduling ||
				    IGLS_Enable_PRE_HB_Scheduling)) ||
				   (GCM_Enable_Scheduling &&
				    GCM_POST_Enable_Scheduling)));
    hbs_type = HBS_CRITICAL_PATH;
    if (PROC_has_bundles()) hbs_type |= HBS_MINIMIZE_BUNDLES;

    // allow data-speculation if (-O > O1) and -OPT:space is turned off.
    should_we_do_thr = should_we_do_thr && (CG_opt_level > 1) && !OPT_Space;

    if (Trace_HB) {
      #pragma mips_frequency_hint NEVER
      fprintf (TFile, "***** HYPERBLOCK SCHEDULER (after GRA) *****\n");
    }
  }

  // Before register allocation:
  // - Do hyperblock scheduling first to get perfect schedules at each
  //   hyperblock level (register-sensitive). 
  // - Do GCM next to extract global parallelism. Some work needs to be
  //   done, so that it strictly enforces hyperblock boundaries.
  // - Do local scheduling for BBs which are not part of any hyperblocks.

  if (before_regalloc) {
    if (!should_we_schedule) return;

    // Do HB scheduling for all HBs generated (before register allocation).
    if (IGLS_Enable_HB_Scheduling && IGLS_Enable_PRE_HB_Scheduling &&
	should_we_global_schedule) {
      HB_Remove_Deleted_Blocks();
      std::list<HB*>::iterator hbi;
      FOR_ALL_BB_STLLIST_ITEMS_FWD(HB_list, hbi) {
	if (!Sched) {
	  Sched = CXX_NEW(HB_Schedule(), &MEM_local_pool);
	}

	// Check to see if not SWP'd.
	std::list<BB*> hb_blocks;
	Get_HB_Blocks_List(hb_blocks,*hbi);
	if (Can_Schedule_HB(hb_blocks)) {
	  Sched->Init(hb_blocks, hbs_type, NULL);
	  Sched->Schedule_HB(hb_blocks);
	}
      }
    }

    // Try GCM (before register allocation).
    if (GCM_Enable_Scheduling && should_we_global_schedule) {
	Stop_Timer (T_Sched_CU);

	GCM_Schedule_Region (hbs_type);

	Set_Error_Phase ("Hyperblock Scheduler (HBS)");
	Start_Timer (T_Sched_CU);
    }

    if (!should_we_local_schedule) return;

    // Do local scheduling for BBs which are not part of HBs. 
    // (before register allocation).
    for (bb = REGION_First_BB; bb != NULL; bb = BB_next(bb)) {
      
      if (    ( rid = BB_rid(bb) )
	      && ( RID_level(rid) >= RL_CGSCHED ) )
	continue;
      
      if (!BB_scheduled(bb)) {
	if (!Sched) {
	  Sched = CXX_NEW(HB_Schedule(), &MEM_local_pool);
	}
	Sched->Init(bb, hbs_type, INT32_MAX, NULL, NULL);
	Sched->Schedule_BB(bb, NULL);
      }
    }
  }
  else {

    // After register allocation:
    // - Perform data-speculation first, since it will expose more 
    //   parallelism and scheduling opportunities at the block level.
    // - Do hyperblock scheduling next to get perfect schedules at each
    //   hyperblock level (parallelism-driven).
    // - Do GCM next to extract global parallelism. Some work needs to be
    //   done, so that it strictly enforces hyperblock boundaries.
    // - Do local scheduling for BBs which are not part of any hyperblocks.

    // Perform data-speculation first, since it will expose parallelism
    // and scheduling opportunities at the block level.
    // TODO: Invoke data-speculation phase before register allocation,
    // requires GRA spill support, and conditionally invoke the phase
    // after register allocation.

    if (should_we_do_thr) {
      Stop_Timer (T_Sched_CU);
      Set_Error_Phase ("Tree-Height Reduction (THR)");
      Start_Timer (T_THR_CU);

      for (bb = REGION_First_BB; bb != NULL; bb = BB_next(bb)) {
	if (    ( rid = BB_rid(bb) )
		&& ( RID_level(rid) >= RL_CGSCHED ) )
	  continue;

	// Perform data-speculation (if profitable).
	// Avoid processing SWP scheduled blocks, all other scheduled blocks
	// are still considered as candidates for THR.

	if (BB_scheduled(bb) && !BB_scheduled_hbs(bb)) continue;
	if (!thr) {
	  thr = CXX_NEW(CG_THR(), &MEM_local_pool);
	}
	thr->Init(bb, THR_DATA_SPECULATION_NO_RB, FALSE);
	thr->Perform_THR();
	
      } /* for (bb= REGION_First_BB).. */

      Stop_Timer (T_THR_CU);
      Check_for_Dump (TP_THR, NULL);
      Start_Timer (T_Sched_CU);

    } /* should_we_do_thr */

    // Do HB scheduling for all HBs generated (after register allocation).
    if (IGLS_Enable_HB_Scheduling && IGLS_Enable_POST_HB_Scheduling &&
	should_we_schedule && should_we_global_schedule) {

      HB_Remove_Deleted_Blocks();
      std::list<HB*>::iterator hbi;
      FOR_ALL_BB_STLLIST_ITEMS_FWD(HB_list, hbi) {
	if (!Sched) {
	  Sched = CXX_NEW(HB_Schedule(), &MEM_local_pool);
	}
	// Check to see if not SWP'd.
	std::list<BB*> hb_blocks;
	Get_HB_Blocks_List(hb_blocks,*hbi);
	if (Can_Schedule_HB(hb_blocks)) {
	  Sched->Init(hb_blocks, hbs_type, NULL);
	  Sched->Schedule_HB(hb_blocks);
	}
      }
    }

    // Try GCM for the region (after register allocation).
    if (GCM_Enable_Scheduling && should_we_schedule &&
	should_we_global_schedule) {
	Stop_Timer (T_Sched_CU);

 	GCM_Schedule_Region (hbs_type);

        Set_Error_Phase ("Hyperblock Scheduler (HBS)");
	Start_Timer (T_Sched_CU);
    }

#ifdef TARG_ST
    CGTARG_Resize_Instructions ();
#endif

    // Do local scheduling for BBs which are not part of HBs. 
    // (after register allocation).
    REG_LIVE_Analyze_Region();
    for (bb = REGION_First_BB; bb != NULL; bb = BB_next(bb)) {
      if (    ( rid = BB_rid(bb) )
	      && ( RID_level(rid) >= RL_CGSCHED ) )
	continue;

      BOOL skip_bb = BB_scheduled(bb) && !BB_scheduled_hbs(bb);

#ifdef TARG_IA64
      // Arthur: The check instrs are IA-64 specific, so is this
      //         entire optimization.
      if (should_we_do_thr && !skip_bb) Remove_Unnecessary_Check_Instrs(bb);
#endif
      BOOL resched = !skip_bb && Reschedule_BB(bb); /* FALSE; */

#ifdef TARG_ST
      // FdF 20050502: BB_scheduled_hbs(bb) is always TRUE on
      // post-pass scheduling, so ignore it.
      skip_bb = BB_scheduled(bb);

      // FdF 20050502: resched means that post-pass scheduling is not
      // necessary. If performed, it can be ignored if it is not
      // better than the current scheduling.
      resched = skip_bb && Reschedule_BB(bb); /* FALSE; */
#endif

      if (should_we_schedule && should_we_local_schedule &&
	  (!skip_bb || resched)) {

	// TODO: try locs_type = LOCS_DEPTH_FIRST also.
	INT32 max_sched = (resched) ?  OP_scycle(BB_last_op(bb))+1 : INT32_MAX;
	if (LOCS_Enable_Scheduling) {
	  if (!Sched) {
	    Sched = CXX_NEW(HB_Schedule(), &MEM_local_pool);
	  }
	  Sched->Init(bb, hbs_type, max_sched, NULL, NULL);
	  Sched->Schedule_BB(bb, NULL);
	}
      }
      Handle_All_Hazards (bb);
    } /* for (bb= REGION_First_BB).. */
    REG_LIVE_Finish();

    // Do branch optimizations here.
    if (should_we_schedule && should_we_local_schedule) {
      if (GCM_Enable_Scheduling) GCM_Fill_Branch_Delay_Slots ();
      if (Assembly) Add_Scheduling_Notes_For_Loops ();
    }
  }

  // need to explicitly delete Sched and thr
  // so that destructors are called.
  if (Sched) {
	CXX_DELETE(Sched, &MEM_local_pool);
  }
  if (thr) {
	CXX_DELETE(thr, &MEM_local_pool);
  }
  L_Free();

#if 0
  Check_for_Dump (TP_SCHED, NULL);
#endif
  Stop_Timer (T_Sched_CU);
}

#ifdef LAO_ENABLED
extern BB_MAP CG_LAO_Region_Map;
void
LAO_Schedule_Region (BOOL before_regalloc, BOOL frequency_verify)
{
  if (before_regalloc) {
    Set_Error_Phase( "LAO Prepass Optimizations" );
    if (CG_LAO_optimizations & OptimizerPhase_MustPrePass) {
      lao_optimize_pu(CG_LAO_optimizations & OptimizerPhase_MustPrePass);
      if (frequency_verify)
	FREQ_Verify("LAO Prepass Optimizations");
    }
  }
  else {
    // Call the LAO for postpass scheduling.
    Set_Error_Phase( "LAO Postpass Optimizations" );
    if (CG_LAO_optimizations & OptimizerPhase_MustPostPass) {
      lao_optimize_pu(CG_LAO_optimizations & OptimizerPhase_MustPostPass);
      if (frequency_verify)
	FREQ_Verify("LAO Postpass Optimizations");
    }
#ifdef TARG_ST
    CGTARG_Resize_Instructions ();
#endif
    // Direct call to the bundler, and bypass the IGLS.
    Set_Error_Phase( "LAO Bundling Optimizations" );
    REG_LIVE_Analyze_Region();
    Trace_HB = Get_Trace (TP_SCHED, 1);
    for (BB *bb = REGION_First_BB; bb; bb = BB_next(bb)) {
      Handle_All_Hazards(bb);
      // Handle_All_Hazards will have fixed branch operations with scheduling date -1
      // Only add notes if the postpass scheduler effectively ran
      if (Assembly && CG_LAO_Region_Map && BB_length(bb)) Add_Scheduling_Note (bb, NULL);
    }
    REG_LIVE_Finish();
    // Only add notes if the postpass scheduler effectively ran
    if (Assembly && CG_LAO_Region_Map) Add_Scheduling_Notes_For_Loops ();
  }
}
#endif
