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
//  Module: hb_hazards.cxx
//  $Revision$
//  $Date$
//  $Author$
//  $Source$
//
//  Description:
//  ============
//
//  Interface to the all hazard detection routines.
//
// =======================================================================
// =======================================================================

#ifdef USE_PCH
#include "cg_pch.h"
#endif // USE_PCH
#pragma hdrstop

#include "W_alloca.h"
#include "W_math.h"
#include "defs.h"
#include "config.h"
#include "config_TARG.h"
#include "config_asm.h"
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
#include "hb_hazards.h"
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
#ifdef TARG_ST
#include "reg_live.h"
#endif
/* These should be included with cgtarget.h !
#include "targ_proc_properties.h"
#include "targ_isa_bundle.h"
#include "ti_bundle.h"
*/

// ======================================================================
// Given a scheduling note <info>, print it to <file>.
// ======================================================================
static void Print_Scheduling_Note (
  NOTE_ACTION action,
  NOTE_INFO *info,
  FILE *file)
{
  switch (action) {
  case NOTE_PRINT_TO_ANL_FILE:
  case NOTE_PRINT_TO_FILE:
    {
      NOTE_SCHED *note = (NOTE_SCHED *)info;
      const char *prefix = ASM_CMNT_LINE"<sched> ";
      const char *suffix = "\n";
      INT sched_length = note->schedule_length;
      INT blk_parallelism = note->block_parallelism;
      BOOL anl_note = action == NOTE_PRINT_TO_ANL_FILE;

      // By default only print out the loop scheduling notes.
      if (sched_length > 0 && !Get_Trace (TP_SCHED, 0x010)) return;

      if (anl_note) {
	if (sched_length > 0) return;
	prefix = "\"";
	suffix = "\"\n";
      } else {
	fprintf (file, "%s\n", prefix);
      }

      if (sched_length < 0) {
        sched_length  = -sched_length;
	if (anl_note) {
	  SRCPOS srcpos = note->loop_srcpos;
	  INT32 lineno = SRCPOS_linenum(srcpos);
	  INT32 fileno = SRCPOS_filenum(srcpos);
	  INT32 colno = SRCPOS_column(srcpos);
	  fprintf (file,
		   "\nmsg sched lines [%d %d %d] ",
		   fileno,
		   lineno,
		   colno);
	}
        fprintf (file, 
	         "%sLoop schedule length: %d cycles (ignoring nested loops)%s", 
	         prefix, 
	         sched_length,
		 suffix);
      }
      else {
        fprintf (file, "%s bb schedule length: %d cycles\n", prefix, sched_length);
        fprintf (file, "%s bb parallelism : %d \n", prefix, blk_parallelism);
      }
      if (!anl_note) fprintf (file, "%s\n", prefix);
      CGTARG_Print_PRC_INFO (file, &note->prc_info, sched_length, prefix, suffix);
      if (!anl_note) fprintf (file, "%s\n", prefix);
    }
    break;
  case NOTE_PRINT_HANDLER_NAME_TO_FILE:
    fprintf(file, "Print_Scheduling_Note");
    break;
  }
}


// ======================================================================
// Create a scheduling note for the given <bb>.
// ======================================================================
void
Add_Scheduling_Note (BB *bb, void *bbsch)
{
  NOTE_SCHED *info = NULL;
  BOOL new_note = FALSE;

  // check if there is a note already. If yes, we only need to update it.
  if (BB_has_note(bb)) {
    info = (NOTE_SCHED *)NOTE_Retrieve_Note_For_Handler (
					  bb, Print_Scheduling_Note);
  }
  if (info == NULL) {
    info = TYPE_MEM_POOL_ALLOC (NOTE_SCHED, &MEM_pu_pool);
    new_note = TRUE;
  }

  // this should really be got from BBSCH data structure
  info->schedule_length = OP_scycle(BB_last_op(bb))+1;
  info->block_parallelism = (bbsch != NULL) ? 
		BBSCH_block_parallelism ((BBSCH*)bbsch) : 0;

  CGTARG_Compute_PRC_INFO (bb, &(info->prc_info));
  FmtAssert (info->schedule_length > 0, 
		  ("illegal schedule length, BB:%d",BB_id(bb)));
  if (new_note) 
    NOTE_Add_To_BB (bb, Print_Scheduling_Note, (NOTE_INFO *)info);
}

void
Add_Scheduling_Notes_For_Loops (void)
{
  Calculate_Dominators ();
  L_Save ();
  LOOP_DESCR *loop_list = LOOP_DESCR_Detect_Loops (&MEM_local_pool);
  BB_SET *processed_bbs = BB_SET_Create_Empty (PU_BB_Count+2, &MEM_local_pool);
  for (LOOP_DESCR *cloop = loop_list;
       cloop != NULL;
       cloop = LOOP_DESCR_next(cloop))
  {
    BB *bb;
    NOTE_SCHED *info = TYPE_MEM_POOL_ALLOC (NOTE_SCHED, &MEM_pu_pool);
    BB *loop_head = LOOP_DESCR_loophead(cloop);
    info->loop_srcpos = BB_Loop_Srcpos(loop_head);

    FOR_ALL_BB_SET_members (LOOP_DESCR_bbset(cloop), bb) {
      // if bb has already been processed, ignore it.
      if (BB_SET_MemberP (processed_bbs, bb)) continue;
      NOTE_SCHED *bbinfo = (NOTE_SCHED *)NOTE_Retrieve_Note_For_Handler (
						    bb, Print_Scheduling_Note);
#define FLOAT_INT_MUL(f,i) ((INT)(f * (float)i + (float)0.5))
      if (bbinfo != NULL) {
	float head_freq = BB_freq(loop_head);
	float freq_ratio = head_freq == 0.0 ? 0.0 : BB_freq(bb)/head_freq;
	info->schedule_length -= 
			  FLOAT_INT_MUL (freq_ratio, bbinfo->schedule_length);
	info->block_parallelism = bbinfo->block_parallelism;
	for (INT i = 0; i < PRC_LAST; i++) {
	  info->prc_info.refs[i] += 
			  FLOAT_INT_MUL (freq_ratio, bbinfo->prc_info.refs[i]);
	}
      }
    }

    if (info->schedule_length < 0) {
      NOTE_Add_To_BB (loop_head, Print_Scheduling_Note, (NOTE_INFO *)info);
    }

    processed_bbs = BB_SET_Union (processed_bbs, LOOP_DESCR_bbset(cloop),
				&MEM_local_pool);
  }
  L_Free ();
  Free_Dominators_Memory ();
}

// ======================================================================
// Noop Insertion:
//
// The following set of routines handle the detection of hazards and 
// insertion of noops to eliminate the hazards. This is the last phase
// of the local scheduler invoked after register allocation.
// ======================================================================


/* Operand numbers are a positive value in the range 0..OP_opnds(op)-1.
 * Define special operand numbers we use so we can have common
 * routines to handle the various kinds of hazards.
 */
#define OPND_RESULT -1	/* The dependence is on the result */
#define OPND_NONE -2	/* There is no operand or result dependence */

#ifdef TARG_ST
// Arthur: Following needed for resource usage tracking
#endif
static BB_OP_MAP omap;
static TI_RES_RES *rr_tab;
static INT Clock;            // current bundle's clock cycle

// ======================================================================
// Returns TRUE if <op> is in the delay slot of the terminating branch
// in <bb>.
// ======================================================================
BOOL
Is_Delay_Slot_Op (OP *op, BB *bb)
{
  if (op != BB_last_op(bb)) return FALSE;
  OP *xfer_op = OP_prev(op);
  if (xfer_op == NULL || !OP_xfer(xfer_op)) return FALSE;
  return TRUE;
}


// ======================================================================
// Check if there is a post-hazard between 'op1' (operand 'opnd') and 
// 'op2'. Return TRUE if this is a hazard, FALSE otherwise.
// ======================================================================
static BOOL
Detect_Post_Hazard (OP *op1, INT opnd, OP *op2)
{
  TN *tn;
  ISA_REGISTER_CLASS cl;
  REGISTER reg;

  switch (opnd) {
  case OPND_NONE:
    return FALSE;

  case OPND_RESULT:
#ifdef TARG_ST
    // [CG]: Seems that we should scan all defs
    // for result hazrds
    for (int i = 0; i < OP_results(op1); i++) {
      tn = OP_result(op1,i);
      if (TN_is_zero_reg (tn)) continue;
      cl = TN_register_class(tn);
      reg = TN_register(tn);
      if (OP_Refs_Reg (op2, cl, reg)) return TRUE;
    }
    return FALSE;
#else
    // is there an flow- or output-dep to scan_op ?
    tn = OP_result(op1,0 /*???*/);
    cl = TN_register_class(tn);
    reg = TN_register(tn);

    // if result register is $0 there is no hazard.
    if (TN_is_zero_reg (tn)) {
      return FALSE;
    }
#ifdef TARG_IA64
    return OP_Refs_Reg (op2, cl, reg) || CGTARG_OP_Refs_TN (op2, tn);
#else
    // Arthur: implicitely referenced operands should be explicitely
    //         included in OPs
    return OP_Refs_Reg (op2, cl, reg);
#endif
#endif
  default:
    // is there an flow- or output-dep to scan_op ?
    tn = OP_opnd(op1, opnd);
    cl = TN_register_class(tn);
    reg = TN_register(tn);
#ifdef TARG_IA64
    return OP_Defs_Reg (op2, cl, reg) || CGTARG_OP_Defs_TN (op2, tn);
#else
    // Arthur: implicitely defined results should be explicitely
    //         included in OPs
    return OP_Defs_Reg (op2, cl, reg);
#endif
  }
  /*NOTREACHED*/
}


// ======================================================================
// Determine if there is a pre-hazard between 'op1' and 'op2' 
// (operand 'opnd'). Return TRUE if this is a hazard, FALSE otherwise.
// ======================================================================
static BOOL
Detect_Pre_Hazard (OP *op1, OP *op2, INT opnd)
{
  TN *operand;
  ISA_REGISTER_CLASS cl;
  REGISTER reg;

  // is there a flow-dep from op1?
  operand = OP_opnd(op2,opnd);
  cl = TN_register_class(operand);
  reg = TN_register(operand);
#ifdef TARG_IA64
  return OP_Defs_Reg (op1, cl, reg) || CGTARG_OP_Defs_TN (op1, operand);
#else
  return OP_Defs_Reg (op1, cl, reg);
#endif
}


// ======================================================================
// Handle the pre-hazard for the 'op' (operand 'opnd'). 'ops_to_check'
// gives the size of the hazard.
// ======================================================================
static void
Handle_Pre_Hazard (OP *op, INT opnd, INT ops_to_check)
{
  OP *scan_op = op;
  BOOL add_noops = FALSE;

  while (ops_to_check > 0) {
    scan_op = OP_prev(scan_op);
    if (scan_op == NULL) {
      add_noops = TRUE;
      break;
    }
    // we don't count dummy ops because they are not emitted.
    if (OP_dummy(scan_op)) continue;

    if (Detect_Pre_Hazard (scan_op, op, opnd))
    {
      add_noops = TRUE;
      break;
    }
    ops_to_check-= OP_Real_Ops (scan_op);;
  }

  if (add_noops) {
    // add ops_to_check number of noops.
    BB_Insert_Noops (op, ops_to_check, TRUE);
  }
}


// ======================================================================
// Handle the post-hazard for the 'op' (operand 'opnd'). 'ops_to_check'
// gives the size of the hazard.
// ======================================================================
static void
Handle_Post_Hazard (OP *op, INT opnd, INT ops_to_check)
{
  OP *scan_op = op;
  BOOL add_noops = FALSE;

  while (ops_to_check > 0) {
    scan_op = OP_next(scan_op);
    if (scan_op == NULL) {
      add_noops = TRUE;
      break;
    }
    // we don't count dummy ops because they are not emitted.
    if (OP_dummy(scan_op)) continue;

    if (Detect_Post_Hazard (op, opnd, scan_op)) {
      add_noops = TRUE;
      break;
    }
    ops_to_check -= OP_Real_Ops (scan_op);
    if (OP_xfer(scan_op)) ops_to_check -= 1;  // account for the delay slot
  }
  if (add_noops) {
    // add ops_to_check number of noops.
    BB_Insert_Noops (op, ops_to_check, FALSE);
  }
}

// ======================================================================
// To deal with additive hazards, we maintain a queue of post-hazards
// and look at them while handling pre-hazards. We remember only 
// the last PQ_SIZE number of post-hazards.
// ======================================================================
#define PQ_SIZE 4
static struct {
  OP *op;
  mINT16 opnd;
  mINT16 numops;
} post_Q[PQ_SIZE+1];


// ======================================================================
// Add a hazard to the post-hazard queue. The order of elements in the
// queue is reversed (i.e. the last hazard added is at index 0).
// ======================================================================
static void
Add_Post_Hazard_To_Q (OP *op, INT opnd, INT numops)
{
  INT i;

  for (i = PQ_SIZE; i > 0; i--) {
    post_Q[i] = post_Q[i-1];
  }
  post_Q[0].op = op;
  post_Q[0].opnd = opnd;
  post_Q[0].numops = numops;
}


// ======================================================================
// Handle the case of possible additive hazards (when a post-hazard is
// added to a pre-hazard to come up with a combined hazard). An example
// of additive hazards is a ctc1,bc1t sequence for mips1-3.
// ======================================================================
static void
Handle_Additive_Hazards (OP *op, INT opnd, INT numops)
{
  INT i;

  for (i = 0; i < PQ_SIZE; i++) {
    OP *pq_op = post_Q[i].op;
    if (pq_op == NULL) break;
    if (Detect_Post_Hazard (pq_op, post_Q[i].opnd, op)) {
      OP *tmp_op;
      INT pq_noops = post_Q[i].numops;

      if (Detect_Pre_Hazard (pq_op, op, opnd)) pq_noops += numops;

      for (tmp_op = OP_next(pq_op); tmp_op != op; tmp_op = OP_next(tmp_op)) {
	pq_noops -= OP_Real_Ops (tmp_op);
      }
      if (pq_noops > 0) {
	// add pq_noops before op.
	BB_Insert_Noops (op, pq_noops, TRUE);
      }
    }
  }
}

// ======================================================================
// Placeholder routine to check if <op> has any dependence conflict with
// <prev_op>.
// ======================================================================
static BOOL
Is_There_OP_Dependence(OP *op, OP *prev_op)
{

  INT j;

#ifdef TARG_ST200
  INT16 latency;
  if (CGTARG_Dependence_Required(prev_op, op, &latency)) return TRUE;
#else
  if (CGTARG_Dependence_Required(prev_op, op)) return TRUE;
#endif

  for (j = 0; j < OP_results(prev_op); ++j) {
    TN *result_tn = OP_result(prev_op, j);
    ISA_REGISTER_CLASS cl = TN_register_class(result_tn);
    REGISTER reg = TN_register(result_tn);

    BOOL read_dependence = 
#ifdef TARG_IA64
      OP_Refs_Reg (op, cl, reg) || CGTARG_OP_Refs_TN (op, result_tn);
#else
      OP_Refs_Reg (op, cl, reg);
#endif

    BOOL write_dependence =
#ifdef TARG_IA64
      OP_Defs_Reg (op, cl, reg) || CGTARG_OP_Defs_TN (op, result_tn);
#else
      OP_Defs_Reg (op, cl, reg);
#endif

    if (read_dependence || write_dependence) {

      // Exclude specific cases here.
      // (1) Ignore dependence between an integer compare operation
      // which sets the predicate and the branch operation which uses
      // the same predicate.
#ifdef TARG_IA64
      if (OP_icmp(prev_op) && OP_xfer(op)) {
	TN *tn1, *tn2;
	OP *cmp_op;
	
	CGTARG_Analyze_Compare(op, &tn1, &tn2, &cmp_op);
	if (prev_op == cmp_op) continue;
      }
#endif

      // (2) Ignore all dependences originating from p0 predicate.
      if (TN_is_true_pred(result_tn)) continue;

      // (3) Ignore dependences originating from exclusive predicates
      if (OP_has_disjoint_predicate(prev_op, op)) continue;

      // (4) If a STOP bit already exists in the bundle for that specific
      // position, then the dependence can be relaxed.
      // if (TI_BUNDLE_stop_bit(bundle, i)) continue;
#ifdef TARG_IA64
      // (5) A check load and a subsequent instruction that reads the
      // target of the check load may exist in the same instruction group.
      if (read_dependence && CGTARG_Is_OP_Speculative(prev_op)) continue;
#endif

      return TRUE;
    }
  }

  return FALSE;
}

// ======================================================================
// Placeholder routine to check if <op> has any conflicts with prev_ops
// in the instruction group <bundle_vector>.
// ======================================================================
static BOOL
Is_There_Group_Dependence(OP *op, VECTOR *bundle_vector)
{
  
  for (INT i = 0; i < VECTOR_count(*bundle_vector); ++i) {
    OP *prev_op = (OP *) VECTOR_element(*bundle_vector, i);
    if (Is_There_OP_Dependence(op, prev_op)) return TRUE;
  }

  return FALSE;
}

#ifndef TARG_ST
// ======================================================================
// Placeholder routine to check if placement of <op> at <slot_pos> in
// a <bundle> can be delayed.
// ======================================================================
static BOOL
Delay_Scheduling_OP(OP *op, INT slot_pos, TI_BUNDLE *bundle)
{

  // If <op> is a <xfer_op>, we would like to not greedily bundle it.
  // Rather, delay the same, so that nops (if necessary) can be 
  // inserted before (instead of after). As a result, any <xfer_op>
  // will be the last_op in a legal bundle.

#ifdef TARG_IA64
  if (BB_last_op(OP_bb(op)) == op && OP_xfer(op) && 
      (slot_pos != (ISA_MAX_SLOTS - 1))) 
    return TRUE;
#endif

  // If <op> needs to be the last member of the group, check for slot
  // positions and any prior stop bits present.
  if (OP_l_group(op) && (slot_pos != (ISA_MAX_SLOTS - 1)) &&
      !TI_BUNDLE_stop_bit(bundle, slot_pos))
    return TRUE;

  return FALSE;

}

#endif /* !TARG_ST */

// ======================================================================
// Placeholder for all issues related to formation of legal bundles.
// Specific functionalities include: legal insts grouping within a bundle,
// Insertion of necessary nops and stop bits.
// ======================================================================
static void
Check_For_Bundle_Hazards(OP *op, TI_BUNDLE *bundle, VECTOR *bundle_vector)
{

  BOOL slot_avail = FALSE;

  // Legal values:
  // <slot_pos> : 0, 1,.. ISA_MAX_SLOTS - 1
  // <stop_pos> : -1, 0, 1, ... ISA_MAX_SLOTS - 1
  // the extra "-1" stop_pos indicates a group dependence outside the
  // context of the current bundle but still need to be preserved.

  INT slot_pos = -1; // not a legal value
  INT stop_pos = -2; // not a legal value

  // If <op> already bundled, ignore..
  if (OP_bundled(op)) return;

  INT i;
  ISA_EXEC_UNIT_PROPERTY prop;
  INT ti_err;
  BOOL stop_bit_reqd = FALSE;

  // No need to process black-box OPs (eg. TOP_asm, TOP_intrncall, etc)
  // For those, cases, finish, the current bundle, insert stop bits around
  // these OPs, and continue.

  BOOL bundling_reqd = (OP_code(op) != TOP_asm);

  if (bundling_reqd) {

    // Stop bit is required, when there exists a group dependence.
    stop_bit_reqd = Is_There_Group_Dependence(op, bundle_vector);

    FOR_ALL_SLOT_MEMBERS (bundle, i) {
 
      // STOP bit may be required, if <op> needs to be the first element
      // in an instruction group.
      stop_bit_reqd |= (i > 0) && OP_f_group(op);

      // Check for availability at slot <i> position in <bundle>.
      if (CGTARG_Bundle_Slot_Available (bundle, op, i, &prop,
					stop_bit_reqd, NULL)) {

	// If there is a need to delay the scheduling of <op>...
#ifdef TARG_ST
#else
	if (Delay_Scheduling_OP(op, i, bundle)) continue;
#endif
	slot_avail = TRUE;
	slot_pos = i;
	// Assumes that stop_bit position is also available.
    	if (stop_bit_reqd) {
	  if (i > 0) stop_pos = i - 1;
	  else stop_pos = -1;
	}
	break;
      }
    }
  }

  // If slot available, reserve the entry in TI_BUNDLE.
  if (slot_avail) {
    Set_OP_bundled (op);
    TI_BUNDLE_Reserve_Slot (bundle, slot_pos, prop);

    // Inter-Bundle dependence, need to set the stop bit.
    if (stop_pos >= 0) {
      TI_BUNDLE_Reserve_Stop_Bit (bundle, stop_pos);
    }

    // Intra-Bundle dependence, no need to set the stop bit but need to
    // reset the bundle_vector to begin new instruction group.

    if (stop_pos >= -1) {
      VECTOR_Reset (*bundle_vector);
    }

    VECTOR_Add_Element (*bundle_vector, op);

    // Check for any hazards (nops) that may need to be filled.
    CGTARG_Handle_Bundle_Hazard(op, bundle, bundle_vector, slot_avail, 
				slot_pos, slot_pos, stop_bit_reqd, prop);

    // Add the <stop_bit> marker appropriately.
    // TODO: need to be refined further.
    if (stop_pos >= -1) {
      OP *last_real_op = Last_Real_OP(op);
      Set_OP_end_group(last_real_op);
    }
  }

  BOOL bundle_full = TI_BUNDLE_Is_Full(bundle, &ti_err);
  FmtAssert(ti_err != TI_RC_ERROR, ("%s", TI_errmsg));

  // If slot not available or <op> is the last op in bb, do extra stuff.
  if (!slot_avail || (BB_last_real_op(OP_bb(op)) == op) || bundle_full) {

    // Pack NOPs till end of the bundle.
    CGTARG_Handle_Bundle_Hazard (op, bundle, bundle_vector, slot_avail,
				 slot_pos, ISA_MAX_SLOTS, stop_bit_reqd, prop);
    // Reset the bundle
    TI_BUNDLE_Clear (bundle);
    // VECTOR_Reset (*bundle_vector);

    // Set the <end_group> flag either, if
    // <op> has to be last inst in the instruction group. or,
    // OP_prev(op) has to be first instruction in a group.

    if (OP_l_group(op)) {
      Set_OP_end_group(op);
      VECTOR_Reset (*bundle_vector);
    }

    if (OP_f_group(op) && OP_prev(op)) {
      OP *last_real_op = Last_Real_OP (op);
      Set_OP_end_group(last_real_op);
      VECTOR_Reset (*bundle_vector);
      VECTOR_Add_Element (*bundle_vector, op);
    }

    // Check to see if the <next_op> has a dependence with the OPs in
    // the current <bundle>. If yes, it's always better to set the stop-bit
    // apriori and create a new instr. group.

    if (bundle_full && OP_next(op)) {
      OP *next_op = OP_next(op);
      if (Is_There_Group_Dependence(next_op, bundle_vector)) {
	Set_OP_end_group(op);
	VECTOR_Reset (*bundle_vector);
      }
    }

    // if black_box_op, set the end_group market and quit now.
    if (!bundling_reqd) {
      Set_OP_end_group(op);
      return;
    }

    // Reattempt packing the <op> after clearing the bundle.
    if (!slot_avail) {
      FOR_ALL_SLOT_MEMBERS (bundle, i) {
	stop_bit_reqd = (i > 0) && OP_f_group(op);
	if (CGTARG_Bundle_Slot_Available (bundle, op, i, &prop,
					  stop_bit_reqd, NULL)) {
	  
	  // If there is a need to delay the scheduling of <op>...
#ifdef TARG_ST
#else
	  if (Delay_Scheduling_OP(op, i, bundle)) continue;
#endif
	  slot_pos = i;
	  if (stop_bit_reqd) {
	    if (i > 0) stop_pos = i - 1;
	    else stop_pos = -1;
	  }
	  break;
	}
      }

      FmtAssert(slot_pos != -1, ("Slot Position not a legal value"));
      Set_OP_bundled (op);
      TI_BUNDLE_Reserve_Slot (bundle, slot_pos, prop);
      if (stop_pos >= 0) TI_BUNDLE_Reserve_Stop_Bit (bundle, stop_pos);
      VECTOR_Add_Element (*bundle_vector, op);

      // Do extra stuff.
      if ((BB_last_real_op((OP_bb(op))) == op) || (slot_pos != 0)) {
	INT max_pos = (BB_last_real_op((OP_bb(op))) == op && 
		       ISA_PACK_Inst_Words(OP_code(op)) == 1) ? 
	  ISA_MAX_SLOTS : slot_pos;
	CGTARG_Handle_Bundle_Hazard (op, bundle, bundle_vector, TRUE, 
				     slot_pos, max_pos, stop_bit_reqd, prop);
	if (stop_pos >= -1) {
	  OP *last_real_op = Last_Real_OP(op);
	  Set_OP_end_group(last_real_op);
	  VECTOR_Reset (*bundle_vector);
	  VECTOR_Add_Element (*bundle_vector, op);
	}

	if (TI_BUNDLE_Is_Full(bundle, &ti_err)) {
	  TI_BUNDLE_Clear(bundle);
	}
      }
    }
  }
}

// ======================================================================
// Handle the case of possible additive hazards (when a post-hazard is
// added to a pre-hazard to come up with a combined hazard). An example
// of additive hazards is a ctc1,bc1t sequence for mips1-3.
// ======================================================================
static void
Check_For_Other_Hazards(OP *op)
{
  
  TOP opcode = OP_code(op);
  INT numops;
  INT opnd;
  INT result;
  INT errata_num;
  INT ti_err;

  // Check for operand hazards.
  numops = TI_LATENCY_Operand_Hazard (opcode, &opnd, &ti_err);
  FmtAssert(ti_err != TI_RC_ERROR, ("%s", TI_errmsg));
  if (numops < 0) {
    // Operand pre-hazard.
    // Check if there are any post-hazards in the queue that can add up
    // with this pre-hazard. If yes, add the two hazards and see if we 
    // need to insert any nops.
    Handle_Additive_Hazards (op, opnd, -numops);
    Handle_Pre_Hazard (op, opnd, -numops);
  } else if (numops > 0) {
    // Operand post-hazard.
    Handle_Post_Hazard (op, opnd, numops);
    Add_Post_Hazard_To_Q (op, opnd, numops);
  }

  // Check for result hazards.
  numops = TI_LATENCY_Result_Hazard (opcode, &result, &ti_err);
  FmtAssert(ti_err != TI_RC_ERROR, ("%s", TI_errmsg));
  if (numops != 0) {
    // Result post-hazard.
    FmtAssert (numops > 0, ("Handle_All_Hazards: can't handle result pre-hazard"));
    Is_True(result == 0, ("can't handle hazard for result number %d", result));
    Handle_Post_Hazard (op, OPND_RESULT, numops);
    Add_Post_Hazard_To_Q (op, OPND_RESULT, numops);
  }

  if (PROC_has_branch_delay_slot()) {
    // Check for delay slot hazards.
    if (OP_xfer(op)) {
      Handle_Post_Hazard (op, OPND_NONE, 1);
      Add_Post_Hazard_To_Q (op, OPND_NONE, 1);
    }
  }

  // Check for errata hazards.
  numops = TI_LATENCY_Errata_Hazard (opcode, &errata_num, &ti_err);
  FmtAssert(ti_err != TI_RC_ERROR, ("%s", TI_errmsg));
  if (numops != 0) {
    CGTARG_Handle_Errata_Hazard (op, errata_num, numops);
  }
}

// ======================================================================
// Handle the case when an instruction with hazard is placed in a delay
// slot.
// ======================================================================
void
Check_For_Delay_Slot_Hazards (BB *bb)
{

  OP *last_op = BB_last_op(bb);

  // If there is an instruction with hazards in the delay slot, move it.
  if (Is_Delay_Slot_Op (last_op, bb) && OP_has_hazard(last_op)) {
    BB_Move_Delay_Slot_Op (bb);
  }

  // R10k chip workaround: Avoid placing integer mult/div ops in delay
  // slots of unconditional branches. (see pv516598) for more details.
  if (Is_Delay_Slot_Op (last_op, bb) && 
      (OP_imul(last_op) || OP_idiv(last_op))) {
    OP *xfer_op = OP_prev(last_op);
    if (xfer_op && OP_uncond(last_op)) {
      BB_Move_Delay_Slot_Op (bb);
      BB_Insert_Noops(xfer_op, 1, FALSE); 
    }
  }
}

#ifdef TARG_ST

/* ====================================================================
 *   Init_Resource_Table
 *
 *   Reserve a resource reservation table for the bundle.
 *   See also Init_RFlag_Table() in hb_hazards.cxx
 * ====================================================================
 */
static void
Init_Resource_Table (
  BB *bb
) 
{
  INT rtable_size = 0;
  INT max_resource_cycles = 0;
  rr_tab = TI_RES_RES_Alloc(FALSE,&MEM_local_pool);

  if (BB_length(bb) == 1) {
    OP *op = BB_first_op(bb);
    rtable_size = TI_RES_Cycle_Count(OP_code(op));
  }
  else if (BB_length(bb) > 1) {
    OP *op;
    FOR_ALL_BB_OPs_FWD (bb, op) {
      INT cur_resource_cycles = TI_RES_Cycle_Count(OP_code(op));
      if (max_resource_cycles < cur_resource_cycles) {
	max_resource_cycles = cur_resource_cycles;
      }

      INT op_latency = cur_resource_cycles;
      ARC_LIST *arcs;
      for (arcs = OP_succs(op); arcs != NULL; arcs = ARC_LIST_rest(arcs)) {
	ARC *arc = ARC_LIST_first(arcs);
	if (ARC_latency(arc) > op_latency) {
	  op_latency = ARC_latency(arc);
	}
      }

      rtable_size += op_latency;
    }

    // increase table size by the maximum number of resource cycles needed by
    // any OP.
    rtable_size += max_resource_cycles;
  }

  TI_RES_RES_Set_BB_Cycle_Count(rr_tab,rtable_size);

  return;
}

/* ====================================================================
 *   Fill_Cycle_With_Noops
 *
 *   Fill a clock cycle following 'op' with noops.
 * ====================================================================
 */
static void
Fill_Cycle_With_Noops (
  OP *op,
  TI_BUNDLE *bundle,
  VECTOR *bundle_vector
#ifdef TARG_ST200 // [CL]: used to track alignment. pc counts instruction words, not bytes
  , INT32* pc
#endif
)
{
  INT template_bit = TI_BUNDLE_Return_Template(bundle);
  FmtAssert (template_bit != -1, ("Illegal template encoding"));

  INT i;
  FOR_ALL_SLOT_MEMBERS(bundle, i) {
    //
    // Advance until the next after 'op' bundle slot found
    //
    if (!TI_BUNDLE_slot_filled(bundle, i)) {
#ifdef TARG_ST200
      // CL: only add a nop if the bundle is currently empty
      if (i==0) {
#endif
      OP *noop = Mk_OP(CGTARG_Noop_Top(ISA_EXEC_Slot_Prop(template_bit, i)));
      BB_Insert_Op_After(OP_bb(op), op, noop);
      OP_scycle(noop) = -1;
      Set_OP_bundled (noop);
      TI_BUNDLE_Reserve_Slot (bundle, i, 
				   ISA_EXEC_Slot_Prop(template_bit, i));

#ifdef TARG_ST200 // [CL]
      *pc += 1;
#endif
      // Set end group and reset bundle vector:
      Set_OP_end_group(noop);
#ifdef TARG_ST200
      }
      else {
	Set_OP_end_group(op);
      }
      TI_BUNDLE_Clear(bundle);
#endif

      VECTOR_Reset (*bundle_vector);

      break;
    }
#ifdef TARG_ST200 // [CL]
    else {
      *pc += 1;
    }
#endif

  }

  return;
}

/* ====================================================================
 *   Handle_Latency
 *
 *   Add noop cycles before 'op' until Clock reaches 'estart' time.
 *   
 * ====================================================================
 */
static void 
Handle_Latency (
  OP *op,                 // before this noops are inserted
  INT estart, 
  TI_BUNDLE *bundle, 
  VECTOR *bundle_vector
#ifdef TARG_ST200 // [CL] keep track of alignment
  , INT32* pc
#endif
)
{
  INT ti_err = TI_RC_OKAY;

#ifdef TARG_ST
  // [CG]: asm top are not bundled
  BOOL bundling_reqd = (OP_code(op) != TOP_asm);
  FmtAssert(OP_bundled(op) || !bundling_reqd, ("OP not bundled ?"));
#else
  // 'op' should have been bundled and no unfilled slots before
  // it in the template.
  FmtAssert(OP_bundled(op), ("OP not bundled ?"));
#endif

  if (Trace_HB) fprintf(TFile, "  handling latency: clock %d, estart %d\n", Clock, estart);

  //
  // Fill estart-Clock cycles following the 'op' with noops
  //
  for (; Clock < estart; Clock++) {

    if (Trace_HB) {
      fprintf(TFile, "  adding noops at clock %d (%d)\n", Clock, estart);
    }

    // This adds a noop group to the bundle
#ifdef TARG_ST200 // [CL] keep track of alignment
    Fill_Cycle_With_Noops (op, bundle, bundle_vector, pc);
#else
    Fill_Cycle_With_Noops (op, bundle, bundle_vector);
#endif

    // if the <bundle> is full, reset it
    if (TI_BUNDLE_Is_Full(bundle, &ti_err)) {
      FmtAssert(ti_err != TI_RC_ERROR, ("%s", TI_errmsg));

      if (Trace_HB) {
	fprintf(TFile, "  bundle full\n");
      }

      TI_BUNDLE_Clear (bundle);
    }

  }

  return;
}

#ifdef TARG_ST200 // [CL] used to keep track of alignment
/* ====================================================================
 * Number_Of_Slots_In_Bundle
 *
 * Returns the number of slots used in a bundle.
 * ====================================================================
 */
static INT Number_Of_Slots_In_Bundle(TI_BUNDLE *bundle)
{
  INT i, count=0;
  FOR_ALL_SLOT_MEMBERS(bundle, i) {
    if (!TI_BUNDLE_slot_filled(bundle, i)) {
      break;
    } else {
      count++;
    }
  }
  return count;
}
#endif

#ifdef TARG_ST200
// FdF: Support for superblock scheduling. This means that latencies
// at basic block boundary may be reduced by the scheduler. In this
// mode, the bundler will never put an instruction earlier than
// scheduled, even if latency permits.
#define SUPERBLOCK_SCHED

/* ====================================================================
 *  Branch_Taken_Latency
 *
 *  Returns the cycle at which op_branch can be scheduled. With no
 *  superblock scheduling, this is equal to pending_latency, minus 1
 *  to account for the cycle of the br, minus
 *  CGTARG_Branch_Taken_Penalty(), to account for the latency of the
 *  branch taken. In case of superblock scheduling, if the branch does
 *  not leave the superblock (backedge to the top of the superblock),
 *  then the branch latency is set to the scheduling date of the
 *  branch operation.
 *  ====================================================================
 *  */
static INT
Branch_Taken_Latency (
  BB *bb,
  OP *op_branch,
  INT estart,
  INT pending_latency
)
{
  INT branch_latency;
  branch_latency = pending_latency - 1 - CGTARG_Branch_Taken_Penalty();

#ifdef SUPERBLOCK_SCHED
  // Scheduled by LAO in superblock mode.
  if ((OP_scycle(op_branch) != -1) && CG_LAO_Region_Map && (BB_MAP32_Get(CG_LAO_Region_Map, bb) != 0)) {
    branch_latency = estart;
  }
#endif

  return branch_latency;
}

/* ====================================================================
 *  Fall_Thru_Latency
 *
 *  Returns the cycle at which the fall through block can start. If no
 *  fall thru block, do not append any empty bundle after the
 *  unconditional branch. In case of a call, the next block can start
 *  at the next bundle after the call. In case of a fall thru into the
 *  same scheduling region, the fall thru latency is computed from the
 *  scheduling dates of the last instruction of the current block and
 *  the first instruction of the fall thru block.
 *  ====================================================================
 *  */
static INT
Fall_Thru_Latency (
  BB *bb,
  INT pending_latency
)
{
  INT fallThru_latency = pending_latency;

  if (!BB_Fall_Thru_Successor(bb))
    fallThru_latency = Clock;

  else if (BB_call(bb))
    fallThru_latency = Clock;

#ifdef SUPERBLOCK_SCHED
  else if (CG_LAO_Region_Map && (BB_MAP32_Get(CG_LAO_Region_Map, bb) != 0)) {
    BB *fallThru_succ = BB_Fall_Thru_Successor(bb);
    if ((BB_MAP32_Get(CG_LAO_Region_Map, bb) == BB_MAP32_Get(CG_LAO_Region_Map, fallThru_succ))) {
      OP *first_op = BB_first_op(fallThru_succ);
      if ((first_op != NULL) && (OP_scycle(first_op) != -1))
	fallThru_latency = OP_scycle(first_op);
    }
  }
#endif

  return fallThru_latency;
}
#endif

/* ====================================================================
 *   Handle_Bundle_Hazards
 *
 *  Placeholder for all issues related to formation of legal bundles.
 *  Specific functionalities include: legal insts grouping within a bundle,
 *  Insertion of necessary nops and stop bits. 
 * ====================================================================
 */
static BOOL
Handle_Bundle_Hazards(
  OP *op, 
  TI_BUNDLE *bundle, 
  VECTOR *bundle_vector
#ifdef TARG_ST200 // [CL] keep track of alignment
  , INT32* pc
#endif
)
{
  BOOL slot_avail = FALSE;

  // Legal values:
  // <slot_pos> : 0, 1,.. ISA_MAX_SLOTS - 1
  // <stop_pos> : -1, 0, 1, ... ISA_MAX_SLOTS - 1
  // the extra "-1" stop_pos indicates a group dependence outside the
  // context of the current bundle but still need to be preserved.

  INT slot_pos = -1; // not a legal value
  INT stop_pos = -2; // not a legal value

  // If <op> already bundled, ignore..
  if (OP_bundled(op)) return TRUE;

  INT i;
  ISA_EXEC_UNIT_PROPERTY prop;
  INT ti_err = TI_RC_OKAY;
  BOOL stop_bit_reqd = FALSE;

  // No need to process black-box OPs (eg. TOP_asm, TOP_intrncall, etc)
  // For those, cases, finish, the current bundle, insert stop bits around
  // these OPs, and continue.

  BOOL bundling_reqd = (OP_code(op) != TOP_asm);
  INT w;

  // if black_box_op, set the end_group market and quit now.
  if (!bundling_reqd) {
#ifdef TARG_ST200 // [CL] if necessary, fill gaps with nops, then update pc
    CGTARG_Finish_Bundle(op, bundle);
    *pc += Number_Of_Slots_In_Bundle(bundle);
#endif
    Set_OP_end_group(op);
#ifdef TARG_ST200 // CL: bundles have variable length
	          // and can be reset at any time
    TI_BUNDLE_Clear (bundle);
    VECTOR_Reset (*bundle_vector);
#endif
    Clock++;
    return TRUE;
  }

  // Check whether there is a slot available in current bundle
  FOR_ALL_SLOT_MEMBERS (bundle, i) {

    if (Trace_HB) {
      fprintf(TFile, "  slot %d ", i);
    }

    // Check for availability at slot <i> position in <bundle>.
#ifdef TARG_ST200 // [CL] keep track of pc,
    // and handle case were slots are not allocated in sequence
    if (CGTARG_Bundle_Slot_Available (bundle, op, i, &prop,
					stop_bit_reqd, NULL, TRUE, *pc, &slot_pos)) {
#else
    if (CGTARG_Bundle_Slot_Available (bundle, op, i, &prop,
					stop_bit_reqd, NULL)) {
#endif
      slot_avail = TRUE;
#ifndef TARG_ST200 // CL
      slot_pos = i;
#endif

      if (Trace_HB) {
#ifdef TARG_ST200 // CL
	fprintf(TFile, "  is available, clock %d [%s] PC=%x\n", 
		  Clock, TOP_Name(OP_code(op)), *pc);
#else
	fprintf(TFile, "  is available, clock %d [%s]\n", 
		  Clock, TOP_Name(OP_code(op)));
#endif
      }

      break;
    }

    if (Trace_HB) fprintf(TFile, "\n");
  }

  //
  // If slot not available, do extra stuff, return FALSE
  //
  if (!slot_avail) {
    if (Trace_HB) fprintf(TFile, "  did not find available slot \n");

    // if <bundle> is empty, no need to do anything. just check to see if
    // a stop_bit is required.
    if (TI_BUNDLE_Is_Empty(bundle, &ti_err)) {
      OP *last_real_op = Last_Real_OP(op);
      FmtAssert(ti_err != TI_RC_ERROR, ("%s", TI_errmsg));
      if (stop_bit_reqd) Set_OP_end_group(last_real_op);
      return FALSE;
    }

    // Process simulated ops
    if (OP_simulated(op)) {
      TOP adjust_top = CGTARG_Simulated_Top(op, prop);
      OP_Change_Opcode(op, adjust_top);
    }

    //    
    // Fill with nops, finish previous bundle
    //
    CGTARG_Handle_Bundle_Hazard(op, 
				bundle, 
				bundle_vector, 
				slot_avail, 
				slot_pos, 
#ifndef TARG_ST200
				ISA_MAX_SLOTS, 
#else
				0,
#endif
				stop_bit_reqd, 
				prop);

#ifdef TARG_ST200 // [CL]
    CGTARG_Finish_Bundle(OP_prev(op), bundle);
    *pc += Number_Of_Slots_In_Bundle(bundle);
#endif

    // Bundle is full at this time, reset bundle
    TI_BUNDLE_Clear (bundle);

#ifdef TARG_ST200
    Set_OP_end_group(OP_prev(op));
    VECTOR_Reset (*bundle_vector);
#endif

    return FALSE;
  }

  // slot available, verify counted resource availability
  BOOL resources_avail = 
	       TI_RES_RES_Resources_Available(rr_tab,OP_code(op),Clock);

  // If counted resource conflict - return FALSE
  if (!resources_avail) {
    // Just reset bundle_vector and restart the process
    // TODO: I should normally handle the resource conflict.
    //       For now I assume that resetting the instr group
    //       does it. -- generalize
    //

    if (Trace_HB) fprintf(TFile,"  counted resource conflict\n");

#ifdef TARG_ST200 // [CL]
    CGTARG_Finish_Bundle(OP_prev(op), bundle);
    *pc += Number_Of_Slots_In_Bundle(bundle);
#endif

    // set <end_group> marker
    Set_OP_end_group(OP_prev(op));
    VECTOR_Reset (*bundle_vector);
#ifdef TARG_ST200 // CL: bundles have variable length
	          // and can be reset at any time
    TI_BUNDLE_Clear (bundle);
#endif

    return FALSE;
  }

  // Everything's OK, reserve the entry in TI_BUNDLE.
  Set_OP_bundled (op);

  // reserve as many slots as necessary
  for (w = 0; w < ISA_PACK_Inst_Words(OP_code(op)); w++)
    TI_BUNDLE_Reserve_Slot (bundle, slot_pos+w, prop);

  VECTOR_Add_Element (*bundle_vector, op);

  // set the OP map and register resource usage
  BB_OP_MAP32_Set(omap, op, Clock);
  TI_RES_RES_Reserve_Resources(rr_tab, OP_code(op), Clock);

#ifndef TARG_ST200
  //
  // If op is last in BB, I will have to fill noops until end 
  // of bundle.
  //
  INT max_pos = (BB_last_real_op((OP_bb(op))) == op) ? 
	                                ISA_MAX_SLOTS : slot_pos;
#else
  INT max_pos = slot_pos;
#endif

  // Fill with noops:
  CGTARG_Handle_Bundle_Hazard(op, 
			      bundle, 
			      bundle_vector, 
			      TRUE, 
			      slot_pos, 
			      max_pos, 
			      stop_bit_reqd, 
			      prop);

  return TRUE;
}

/* ====================================================================
 *   Make_Bundles
 * ====================================================================
 */
void
Make_Bundles (
  BB *bb,
  TI_BUNDLE *bundle, 
  VECTOR *bundle_vector
) 
{
  INT ti_err = TI_RC_OKAY;
  BOOL dep_graph_built = FALSE;
#ifdef TARG_ST200 //CL: under -O0 -g, force new bundle for new source line
  static SRCPOS last_srcpos = 0;
#endif
#ifdef TARG_ST200 // [CL] handle alignement within hot and cold sections
  static INT32 hot_addr=0;
  static INT32 cold_addr=0;
  static PU_IDX bundler_current_pu=0;
  // [CL] We track alignment on a per-PU basis, so that we only need
  // to maintain two counters: hot & cold.  Indeed, as the user can
  // provide his own section name for each PU, in order to compute
  // actual PC values for each section, we would need to change the
  // way sections are generated: currently this process occurs much
  // later, in cgemit.cxx, and it would require to be somewhat
  // transfered here.
  INT *pc;
#endif

  if (BB_length(bb) == 0) return;

  MEM_POOL_Push(&MEM_local_pool);

  if (BB_length(bb) > 1) {

    CG_DEP_Compute_Graph (
          bb, 
	  INCLUDE_ASSIGNED_REG_DEPS,
	  NON_CYCLIC,
	  INCLUDE_MEMREAD_ARCS,
	  INCLUDE_MEMIN_ARCS,
	  NO_CONTROL_ARCS,
	  NULL);

    if (Trace_HB) CG_DEP_Trace_Graph (bb);
    dep_graph_built = TRUE;
  }

  // Initialize resource tracking
  //
  Init_Resource_Table (bb);

  // Create a mapping of this BB's OPs to sched cycles
  //
  omap = BB_OP_MAP32_Create(bb,&MEM_local_pool);

  //
  // start "scheduling" at this cycle
  //
  Clock = 0;

  // This helps handling latencies that may still be pending 
  // when all OPs are processed
  INT pending_latency = 0;

  // Now iterate through the ops.
  OP *next_op;
  OP *op = BB_first_op(bb);

  // FdF: Because the bundler now uses the scheduling dates to fill
  // the bundles, unless latencies contraints are not met.
#ifdef TARG_ST200
  Clock = OP_scycle(op) == -1 ? 0 : OP_scycle(op);
#endif
    
  // Just to get things started
  if (OP_dummy(op)) BB_OP_MAP32_Set(omap, op, Clock);

#ifdef TARG_ST200 // [CL]
  if (bundler_current_pu != ST_pu(Get_Current_PU_ST())) {
    // We changed PU since last call, so reset local PC counters
    cold_addr = 0;
    hot_addr = 0;
    bundler_current_pu = ST_pu(Get_Current_PU_ST());
  }
  if (BB_Is_Cold(bb)) {
    pc = &cold_addr;
  } else {
    pc = &hot_addr;
  }
#endif

  while (op != NULL) {
    // save this, so I do not process just added OPs
    next_op = OP_next(op);

    if (Trace_HB) {
#ifdef TARG_ST200 // [CL]
      fprintf(TFile, "<hazard at PC=%x> ", *pc);
#else
      fprintf(TFile, "<hazard> ");
#endif
      Print_OP_No_SrcLine(op);
    }

    if (OP_dummy(op)) continue;

    if (op != BB_first_op(bb)) {
      // 
      // First, make sure there are no dependencies between this
      // op and anything before it in the BB. If dependence exists,
      // insert a bundle full of noops, increment Clock
      //
      // NOTICE: predecessors must have been scheduled (except for
      //         the first OP in the BB
      //
      INT estart = Clock;
      ARC_LIST *arcs;
      for (arcs = OP_preds(op); arcs != NULL; arcs = ARC_LIST_rest(arcs)) {
	ARC *arc = ARC_LIST_first(arcs);
	OP *pred_op = ARC_pred(arc);
	INT pclock = BB_OP_MAP32_Get(omap, pred_op);
	if (pclock + ARC_latency(arc) > estart) {
	  estart = pclock + ARC_latency(arc);
	}
      }

      // FdF: Use scheduling date to fill bundles if superblock
      // scheduling is enabled and done.
#ifdef TARG_ST200
#ifdef SUPERBLOCK_SCHED
      if (BB_scheduled(bb) && CG_LAO_Region_Map) {
	Is_True((estart <= OP_scycle(op)) || (OP_scycle(op) == -1),
		("BB %d, Clock %d, op %s: illegal bundle generated by the scheduler",
		 BB_id(bb), Clock, TOP_Name(OP_code(op))));
	if (OP_scycle(op) != -1)
	  estart = OP_scycle(op);
      }
#endif
#endif

      if (Trace_HB) {
	fprintf(TFile, "  estart = %d (current %d)\n", estart, Clock);
      }

      // If there is a dependence, first end current group
#ifdef TARG_ST200
      // [CG] Treat asm
      BOOL bundling_reqd = (OP_code(op) != TOP_asm);

      // CL: start new bundle for new source line under -O0
      if ( (Clock < estart)
	   // CL: start new bundle for new source line under -O0
	   || ( (Opt_Level == 0)
      		&& (last_srcpos != OP_srcpos(op))
      		&& (OP_srcpos(op) != 0)
      		)
	   // [CL] finish previous bundle if last op was
	   // in prologue and current op insn't
	   || ( (Opt_Level == 0)
		&& !OP_prologue(op) && OP_prologue(OP_prev(op))
		)
	   // [CL] finish previous bundle if current op is
	   // in epilogue and prev op wasn't
	   || ( (Opt_Level == 0)
		&& OP_epilogue(op) && !OP_epilogue(OP_prev(op))
		)
	   || !bundling_reqd
      	   ) {
#else
      if (Clock < estart) {
#endif
	// Add the <stop_bit> marker appropriately.
	// TODO: need to be refined further.
	//	OP *last_real_op = Last_Real_OP(op);
	//	Set_OP_end_group(last_real_op);
	if (!OP_end_group(OP_prev(op))) {
#ifdef TARG_ST200 // [CL]
	  CGTARG_Finish_Bundle(OP_prev(op), bundle);
	  *pc += Number_Of_Slots_In_Bundle(bundle);
#endif
	  Set_OP_end_group(OP_prev(op));
	  VECTOR_Reset (*bundle_vector);
#ifdef TARG_ST200 // CL: bundles have variable length
	          // and can be reset at any time
	  // Reset the bundle
	  TI_BUNDLE_Clear (bundle);
#endif
	  if (Trace_HB) {
	    fprintf(TFile, "  advancing clock with end group\n");
	  }
	  Clock++;
	}
      }

      // Check for latencies that may still be pending
      if (OP_xfer(op)) {
	// before we bundle the transfer out of BB, handle the
	// latencies that may still be pending

	if (Trace_HB) fprintf(TFile, "  OP_xfer: pending latency = %d\n", pending_latency);

	//
	// If we need to wait longer than the dependence, do it
	//
#ifdef TARG_ST200
	// FdF: Earliest estart for a branch is pending_latency, minus
	// 1 to account for the cycle of the br, minus
	// CGTARG_Branch_Taken_Penalty(), to account for the latency
	// of the branch taken. If the branch is not taken, empty
	// bundles will be added after the branch while Clock < pending_latency.

	INT branch_latency = Branch_Taken_Latency(bb, op, estart, pending_latency);
	if (branch_latency > estart) estart = branch_latency;
#else
	if (pending_latency > estart) estart = pending_latency;
#endif
      }

      // If dependence is still pending, fill with noop cycles
#ifdef TARG_ST200 // [CL] keep track of alignment
      Handle_Latency (OP_prev(op), estart, bundle, bundle_vector, pc);
#else
      Handle_Latency (OP_prev(op), estart, bundle, bundle_vector);
#endif

    } /* for all OPs other than first */

#ifdef TARG_ST200
      last_srcpos = OP_srcpos(op);
#endif

    //
    // Now, we're guaranteed that there are no pending dependencies
    //

    // Set the <end_group> flag if op
    // has to be first instruction in a group.
    if (OP_f_group(op) && OP_prev(op) && !OP_end_group(OP_prev(op))) {
      // Add the <stop_bit> marker appropriately.
      // TODO: need to be refined further.
#ifdef TARG_ST200 // [CL]
      CGTARG_Finish_Bundle(OP_prev(op), bundle);
      *pc += Number_Of_Slots_In_Bundle(bundle);
#endif
      Set_OP_end_group(OP_prev(op));
      VECTOR_Reset (*bundle_vector);
#ifdef TARG_ST200
      // Reset the bundle
      TI_BUNDLE_Clear (bundle);
#endif
      Clock++;
    }

    //
    // Iterate over the following untill we can schedule the 'op'
    //
#ifdef TARG_ST200 // [CL] keep track of alignment
    while (!Handle_Bundle_Hazards (op, bundle, bundle_vector, pc)) {
#else
    while (!Handle_Bundle_Hazards (op, bundle, bundle_vector)) {
#endif

      // If we did not find a template slot, the bundle should
      // have been reset;
      // If we had a counted resource conflict, as well;
      // Advance the Clock and retry.
      //

      // advance the Clock
      Clock++;

#ifdef TARG_ST200 // [CL]
      if (Trace_HB) fprintf(TFile, "  reattempting bundling at clock %d and PC %x\n", Clock, *pc);
#else
      if (Trace_HB) fprintf(TFile, "  reattempting bundling at clock %d\n", Clock);
#endif
    }

    //
    // After having bundled this 'op', track its latency
    //
    // We do not need any more pending latency stuff, if it's a xfer
    if (!OP_xfer(op)) {
      INT latency = 0;
#ifdef TARG_ST
      BOOL liveout = FALSE;
      INT i;
      for (i = 0; i < OP_results(op); i++) {
	TN *result_tn = OP_result(op,i);
	if (TN_is_register(result_tn)) {
	  ISA_REGISTER_CLASS result_cl = TN_register_class (result_tn);
	  REGISTER result_reg = TN_register (result_tn);
	  liveout |= REG_LIVE_Outof_BB (result_cl, result_reg, bb);
	}
      }
      if (liveout)
#endif
	latency = CGTARG_Max_OP_Latency (op);
      if (Clock+latency > pending_latency) pending_latency = Clock+latency;

      if (Trace_HB) fprintf(TFile, "  max latency = %d [pending %d]\n", 
			    latency, pending_latency);
    }

    // Set the <end_group> flag if
    // <op> has to be last inst in the instruction group. 
    if (OP_l_group(op) || (next_op == NULL)) {
#ifdef TARG_ST200 // [CL]
      CGTARG_Finish_Bundle(op, bundle);
      *pc += Number_Of_Slots_In_Bundle(bundle);
#endif
      Set_OP_end_group(op);
      VECTOR_Reset (*bundle_vector);
#ifdef TARG_ST200
      // Reset the bundle
      TI_BUNDLE_Clear (bundle);
#endif
      Clock++;
    }

    //
    // If bundle is full (and if it was the last op in BB it must be
    // full as well), advance the clock.
    //
    BOOL bundle_full = TI_BUNDLE_Is_Full(bundle, &ti_err);
    FmtAssert(ti_err != TI_RC_ERROR, ("%s", TI_errmsg));
    if (bundle_full) {

      if (Trace_HB) fprintf(TFile, "  bundle full \n");

#ifdef TARG_ST200 // [CL]
      // No need to fill with nops, bundle is full
      *pc += Number_Of_Slots_In_Bundle(bundle);
#endif
      // Reset the bundle
      TI_BUNDLE_Clear (bundle);

      Clock++;
    }

    // go to next original OP
    op = next_op;

  } /* FOR_ALL_BB_OPs_FWD */

#ifdef TARG_ST200
  // FdF: Adjust pending_latency to take into account unconditional
  // branch, calls, and superblock scheduling.
  pending_latency = Fall_Thru_Latency(bb, pending_latency);
    
#endif
 
  // while there are pending latencies, add noop cycles
  if (Clock < pending_latency) {

    if (Trace_HB) fprintf(TFile,"  adding noops for pending latency ...\n");

    // First, end the current group, if need to
    if (!OP_end_group(BB_last_op(bb))) {
#ifdef TARG_ST200 // [CL]
      CGTARG_Finish_Bundle(BB_last_op(bb), bundle);
      *pc += Number_Of_Slots_In_Bundle(bundle);
#endif
      // Set end group and reset bundle vector:
      Set_OP_end_group(BB_last_op(bb));
      VECTOR_Reset (*bundle_vector);

#ifdef TARG_ST200
      // Reset the bundle
      TI_BUNDLE_Clear (bundle);
#endif
      // advance the Clock
      Clock++;
    }

    // at the end of BB
#ifdef TARG_ST200 // [CL] keep track of alignment
    Handle_Latency (BB_last_op(bb),
		    pending_latency, 
		    bundle, 
		    bundle_vector, pc);
#else
    Handle_Latency (BB_last_op(bb),
		    pending_latency, 
		    bundle, 
		    bundle_vector);
#endif
  }

  // And, need to complete the last bundle
  if (!TI_BUNDLE_Is_Empty(bundle, &ti_err)) {
#ifndef TARG_ST200 // CL: ST200 has variable-length bundles
    while (!TI_BUNDLE_Is_Full(bundle, &ti_err)) {
#endif
#ifdef TARG_ST200 // [CL] keep track of alignment
      Fill_Cycle_With_Noops (BB_last_op(bb), bundle, bundle_vector, pc);
#else
      Fill_Cycle_With_Noops (BB_last_op(bb), bundle, bundle_vector);
#endif
#ifndef TARG_ST200
    }
#endif
  }

#ifdef TARG_ST
  // FdF: Make sure the last operation of the basic block have a valid
  // scheduling date
  FmtAssert (BB_last_op(bb), 
	     ("Last op disappeared in BB:%d",BB_id(bb)));
  if (OP_scycle(BB_last_op(bb)) == -1)
    OP_scycle(BB_last_op(bb)) = Clock-1;
#endif

  if (dep_graph_built) {
    CG_DEP_Delete_Graph (bb);
  }

  MEM_POOL_Pop(&MEM_local_pool);

  return;
}

#endif

// ======================================================================
// Eliminate hazards for 'bb' by adding noops.
// ======================================================================
void
Handle_All_Hazards (BB *bb)
{
  OP *op;
  OP *last_op = BB_last_op(bb);

  if (last_op == NULL) return;

  if (Trace_HB) {
    fprintf(TFile, "<hazard> Handle All Hazards BB%d:\n", BB_id(bb));
  }

  // Check for any delay slot hazards.
  if (PROC_has_branch_delay_slot())
    Check_For_Delay_Slot_Hazards(bb);

  BZERO (post_Q, sizeof(post_Q));

  VECTOR bundle_vector = VECTOR_Init (BB_length(bb) + 1, &MEM_local_pool);
  TI_BUNDLE *bundle = TYPE_MEM_POOL_ALLOC (TI_BUNDLE, &MEM_local_pool);
  BZERO (bundle, sizeof(bundle));
  bundle->bundle_info = TYPE_MEM_POOL_ALLOC (ISA_BUNDLE_INFO, &MEM_local_pool);
  TI_BUNDLE_Clear(bundle);
  
#ifdef TARG_ST200
  // Arthur: need to do sort of scheduling but without reordering
  //         instructions.
  //
  Make_Bundles(bb, bundle, &bundle_vector);

#else

  FOR_ALL_BB_OPs_FWD (bb, op) {

    // Check for bundle hazards.
    if (PROC_has_bundles() && LOCS_Enable_Bundle_Formation) {

      if (Trace_HB) {
	fprintf(TFile, "<hazard> ");
	Print_OP_No_SrcLine(op);
      }

      // Avoid processing dummy and already bundled OPs.
      if (!OP_dummy(op) && !OP_bundled(op)) 
	Check_For_Bundle_Hazards (op, bundle, &bundle_vector);
    }

    // Check for other hazards.
    Check_For_Other_Hazards(op);

  }

  // Check for any extra hazards.
#ifdef TARG_ST
  CGTARG_Insert_Stop_Bits(bb);
#else
  Insert_Stop_Bits(bb);
#endif
#endif

}
