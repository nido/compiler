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


/* ====================================================================
 * ====================================================================
 *
 * Module: cg.cxx
 * $Revision$
 * $Date$
 * $Author$
 * $Source$
 *
 * Description:
 *
 * This	file contains the main driver and initialization,termination
 * routines for	the code generator.
 *
 * ====================================================================
 * ====================================================================
 */

#include "defs.h"
#include "wn.h"

#include "pu_info.h"     /* for Current_PU_Info */
#include "dep_graph.h"   /* for CG_Dep_Graph */

#include "cg.h"
#include "cg_flags.h"
#include "config.h"
#include "config_list.h"
#include "tracing.h"
#include "timing.h"
#include "strtab.h"
#include "cgir.h"
#include "erglob.h"
#include "ercg.h"
#include "data_layout.h"
#include "whirl2ops.h"
#include "calls.h"
#include "bitset.h"
#include "tn_set.h"
#include "gtn_universe.h"
#include "bb_set.h"
#include "register.h"
#include "gra.h"
#include "freq.h"
#include "fb_whirl.h"
#include "lra.h"
#include "cgemit.h"
#include "cg_loop.h"
#include "glob.h"
#include "cgexp.h"
#include "igls.h"
#include "tn_map.h"
#include "cg_region.h"
#include "wn_util.h"
#include "cg_spill.h"
#include "localize.h"
#include "gra_live.h"
#include "opt_alias_interface.h"
#include "ir_reader.h"
#include "cflow.h"
#include "dwarf_DST_mem.h"
#include "region_util.h"
#include "eh_region.h"
#include "reg_live.h"
#include "findloops.h"
#include "cgdriver.h"
#include "label_util.h"
#include "cgtarget.h"
#include "ebo.h"
#include "hb.h"
#ifdef SUPPORTS_PREDICATION
#include "pqs_cg.h"
#endif
#ifdef SUPPORTS_SELECT
#include "cg_select.h"
#endif
#include "tag.h"

#ifdef TARG_ST
#include "cg_ssa.h"
#endif

MEM_POOL MEM_local_region_pool;	/* allocations local to processing a region */
MEM_POOL MEM_local_region_nz_pool;

BOOL Trace_REGION_Interface = FALSE;

BOOL PU_Has_Calls;
BOOL PU_References_GP;

BOOL CG_PU_Has_Feedback;

BOOL Reuse_Temp_TNs = FALSE;

RID *Current_Rid;

TN_MAP TN_To_PREG_Map;

/* WOPT alias manager */
struct ALIAS_MANAGER *Alias_Manager;

/* static BOOL Orig_Enable_SWP; */

/* ====================================================================
 *   CG_PU_Initialize (wn_pu)
 *
 *   Stuff that needs to be done at the start of each PU in cg. 
 * ====================================================================
 */
void
CG_PU_Initialize (
  WN *wn_pu
)
{
  static INT pu_num;

  MEM_POOL_Push ( &MEM_phase_pool );
  MEM_POOL_Push ( &MEM_local_pool );
  MEM_POOL_Push ( &MEM_phase_nz_pool );
  MEM_POOL_Push ( &MEM_local_nz_pool );

  PU_Has_Calls = FALSE;
  PU_References_GP = FALSE;

  CG_Configure_Opt_Level((pu_num < CG_skip_before ||
			   pu_num > CG_skip_after  ||
			   pu_num == CG_skip_equal) ? 0 : Opt_Level);
  pu_num++;

  Reuse_Temp_TNs = (CG_opt_level == 0);
  if (Get_Trace (TP_CGEXP, 1024)) Reuse_Temp_TNs = FALSE;

  CGTARG_Initialize();
  BB_PU_Initialize ();
  Init_TNs_For_PU ();
  LOOP_DESCR_Init_For_PU();
  TN_MAP_Init();
  BB_MAP_Init();
  OP_MAP_Init();
  CGSPILL_Initialize_For_PU ();
  CFLOW_Initialize();
  CG_LOOP_Init();
  HB_Init();
#ifdef SUPPORTS_SELECT
  Select_Init();
#endif

  if (CG_enable_peephole) EBO_Init();
  Init_Label_Info();

#ifdef EMULATE_LONGLONG
  extern void Init_TN_Pair();
  Init_TN_Pair ();
#endif

  /* initialize register package for current pu */
  REGISTER_Pu_Begin();

  Init_Entry_Exit_Code (wn_pu);
  REGISTER_Reset_FP();	// in case $fp is used, must be after entry_exit init

  /* Initialize global tn universe */
  GTN_UNIVERSE_Pu_Begin();

  Trace_REGION_Interface = Get_Trace( TP_REGION, TT_REGION_CG_DEBUG ) ||
    Get_Trace(TP_REGION, TT_REGION_BOUND_DEBUG );

  Init_gen_quad_preg(); // init statics to hold quad preg STs

  /* data layout and lowering now happens earlier, in bedriver. */
  /* don't finalize stack frame until just before emit, after all spills. */

  if (Get_Trace (TKIND_ALLOC, TP_CG)) {
    // start tracing at beginning of cg.
    MEM_Tracing_Enable();
  }

  return;
}

/* ====================================================================
 *   CG_PU_Finalize(void)
 * ====================================================================
 */
void
CG_PU_Finalize(void)
{
  TAG_Finish();
  GTN_UNIVERSE_Pu_End ();
  OP_MAP_Finish();
  CGSPILL_Finalize_For_PU();

  if (CG_enable_peephole) EBO_Finalize();

  //  if (PU_has_syscall_linkage(Get_Current_PU())) {
  //	Enable_SWP = Orig_Enable_SWP;
  //  }

  /* TN_To_PREG_Map is allocated from MEM_pu_pool and so can't be popped
     but it can be put on the free list and cleared at the end of the PU */
  TN_MAP_Delete(TN_To_PREG_Map);
  TN_To_PREG_Map = NULL;

  Free_BB_Memory();		    /* Free non-BB_Alloc space. */
  MEM_POOL_Pop ( &MEM_local_pool );
  MEM_POOL_Pop ( &MEM_local_nz_pool );
  MEM_POOL_Pop ( &MEM_phase_pool );
  MEM_POOL_Pop ( &MEM_phase_nz_pool );
}

/* ====================================================================
 *   CG_Region_Initialize (rwn, alias_mgr)
 *
 *   Stuff that needs to be done at the start of each REGION in cg. 
 * ====================================================================
 */
static void
CG_Region_Initialize (
  WN *rwn, 
  struct ALIAS_MANAGER *alias_mgr
)
{
  MEM_POOL_Push (&MEM_local_region_pool);
  MEM_POOL_Push (&MEM_local_region_nz_pool);

  Init_CG_Expand ();
  //  EXP_Init ();
  FREQ_Region_Initialize ();
  BB_REGION_Initialize ();
  LRA_Init();
  GRA_Initialize();
  Init_TNs_For_REGION ();

  /*
   * Create Array to map PREGs into TNs
   * Must be done after Init_Entry_Exit_Code, since
   * Init_Entry_Exit_Code creates special PREGs to represent
   * save locations in WHIRL
   */
  PREG_NUM last_preg_num;
  last_preg_num = Get_Preg_Num (PREG_Table_Size(CURRENT_SYMTAB))+1;
  PREG_To_TN_Array = (TN **) Pu_Alloc (sizeof (TN *) * last_preg_num);
  PREG_To_TN_Mtype = (TYPE_ID *) Pu_Alloc (sizeof (TYPE_ID) * last_preg_num);

  PREG_To_TN_Clear();	/* this enforces different preg maps between regions */
  if (TN_To_PREG_Map == NULL)
    TN_To_PREG_Map = TN_MAP_Create();

  TN_CORRESPOND_Free(); /* remove correspondence between tns (ex. divrem) */

  GTN_UNIVERSE_REGION_Begin();

  Whirl2ops_Initialize(alias_mgr);

  Current_Rid = REGION_get_rid( rwn );

  return;
}

/* ====================================================================
 *   CG_Region_Finalize (result_before, result_after,
 *	                            rwn, am, generate_glue_code)
 *
 *   Stuff that needs to be done at the end of each REGION in cg.
 *   This includes making glue code to map TNs in CG'd code
 *   from/to PREGs in WHIRL
 * ====================================================================
 */
static void
CG_Region_Finalize (WN *result_before, WN *result_after,
		    WN *rwn, struct ALIAS_MANAGER *am, BOOL generate_glue_code)
{
  RID *rid;
  CGRIN *cgrin;
  WN *entry_fixup, *exit_fixup;
  INT32 i, num_exits;

  Is_True(REGION_consistency_check(rwn),("CG_Region_Finalize"));
  rid = REGION_get_rid( rwn );
  cgrin = RID_cginfo( rid );
  FmtAssert(rid != NULL && cgrin != NULL,
	    ("CG_Region_Finalize, inconsistent region"));

  REGION_set_level(rid, RL_CGSCHED);

  if (generate_glue_code) {
    /* region entry glue code */
    entry_fixup = CGRIN_entry_glue( cgrin );
    REGION_Entry_PREG_Whirl( rid, entry_fixup, CGRIN_tns_in( cgrin ), am );
    if ( Trace_REGION_Interface ) {
      fprintf( TFile, "<region> Entry glue code for RGN %d\n", RID_id(rid) );
      fdump_tree( TFile, entry_fixup );
    }
    WN_INSERT_BlockFirst( result_before, entry_fixup );

    num_exits = RID_num_exits( rid );
    for (i=0; i<num_exits; i++) {
      exit_fixup = CGRIN_exit_glue_i( cgrin, i );
      REGION_Exit_PREG_Whirl( rid, i, exit_fixup,
			     CGRIN_tns_out_i( cgrin, i ), am );
      if ( Trace_REGION_Interface ) {
	fprintf( TFile, "<region> Exit glue code for exit %d RGN %d\n",
		i, RID_id(rid) );
	fdump_tree( TFile, exit_fixup );
      }
      WN_INSERT_BlockLast( result_after, exit_fixup );
    }
  }

  Whirl2ops_Finalize();

  MEM_POOL_Pop (&MEM_local_region_pool);
  MEM_POOL_Pop (&MEM_local_region_nz_pool);
}

/* =================================================================
 *   CG_Generate_Code
 *
 *   Can be called two ways:
 *   1) on a region (pu_dst is NULL, returns code)
 *   2) on a PU (pu_dst is no NULL, returns NULL)
 *
 *   TODO: for now I just ifdef'd what is not implemented.
 * =================================================================
 */
WN *
CG_Generate_Code( 
    WN *rwn, 
    struct ALIAS_MANAGER *alias_mgr, 
    DST_IDX pu_dst, 
    BOOL region 
)
{
  // Some target specific check-ups:

#ifdef TARG_ST
  if (region) {
    FmtAssert(FALSE,("CG_Generate_Code: region code generator for an ST target"));
  }
#endif

  if (Current_Dep_Graph != NULL && Get_Trace(TP_CG, 1)) {
    // Try to dump the CG Dependence graph, if still valid ??
    // Current_PU_Info should be valid ??
    fprintf(TFile, "%sLNO dep graph for CG, before CG\n%s", DBar, DBar);
    Current_Dep_Graph->Print(TFile);
    fprintf(TFile, "%s", DBar);
  }

  // This is the real code generation part:

  /*later:  BOOL region = DST_IS_NULL(pu_dst); */
  BOOL orig_reuse_temp_tns = Reuse_Temp_TNs;

  Alias_Manager = alias_mgr;

  Set_Error_Phase( "Code Generation" );
  Start_Timer( T_CodeGen_CU );

  // Use of feedback information can be disabled in CG using the 
  // -CG:enable_feedback=off flag. The flag CG_PU_Has_Feedback is used
  // all over CG instead of Cur_PU_Feedback for this reason.
  CG_PU_Has_Feedback = ((Cur_PU_Feedback != NULL) && CG_enable_feedback);
  BOOL frequency_verify = Get_Trace( TP_FEEDBACK, TP_CG_FEEDBACK );

  CG_Region_Initialize ( rwn, alias_mgr );

  Set_Error_Phase ( "Code_Expansion" );
  Start_Timer ( T_Expand_CU );

  // If this PU is simply a wrapper for assembly code to be placed
  // into the .s file, take care of that job and move on.
  if (WN_operator(rwn) == OPR_FUNC_ENTRY &&
      ST_asm_function_st(*WN_st(rwn))) {
    FmtAssert(Assembly && !Object_Code,
	      ("Cannot produce non-assembly output with file-scope asm"));
    if (Assembly)
      fprintf(Asm_File, "\n%s\n", ST_name(WN_st(rwn)));
    return rwn;
  }

  Convert_WHIRL_To_OPs ( rwn );

  // split large bb's to minimize compile speed and register pressure
  Split_BBs();

  if ( ! CG_localize_tns ) {
    // Localize dedicated tns involved in calls that cross bb's,
    // and replace dedicated TNs involved in REGION interface with the
    // corresponding allocated TNs from previously compiled REGIONs.
    Localize_or_Replace_Dedicated_TNs();
  }

#if 0
  fprintf(TFile, "%s CFG After Localize_or_Replace_Dedicated_TNs\n %s\n", DBar, DBar);
  Print_All_BBs ();
#endif

  // If using feedback, incorporate into the CFG as early as possible.
  // This phase also fills in any missing feedback using heuristics.
  if (CG_PU_Has_Feedback) {
    Set_Error_Phase ("FREQ");
    Start_Timer (T_Freq_CU);
    FREQ_Incorporate_Feedback( rwn );
    Stop_Timer (T_Freq_CU);
    Set_Error_Phase ( "Code_Expansion" );
    if (frequency_verify)
      FREQ_Verify("Feedback Incorporation");
  }

  EH_Prune_Range_List();

  Optimize_Tail_Calls( Get_Current_PU_ST() );

  Init_Callee_Saved_Regs_for_REGION( Get_Current_PU_ST(), region );
  Generate_Entry_Exit_Code ( Get_Current_PU_ST(), region );
  Stop_Timer ( T_Expand_CU );
  Check_for_Dump ( TP_CGEXP, NULL );

  if (CG_localize_tns) {
    /* turn all global TNs into local TNs */
    Set_Error_Phase ( "Localize" );
    Start_Timer ( T_Localize_CU );
    Localize_Any_Global_TNs(region ? REGION_get_rid( rwn ) : NULL);
    Stop_Timer ( T_Localize_CU );
    Check_for_Dump ( TP_LOCALIZE, NULL );
  } else {
    /* Initialize liveness info for new parts of the REGION */
    /* also compute global liveness for the REGION */
    Set_Error_Phase( "Global Live Range Analysis");
    Start_Timer( T_GLRA_CU );
    GRA_LIVE_Init(region ? REGION_get_rid( rwn ) : NULL);
    Stop_Timer ( T_GLRA_CU );
    Check_for_Dump ( TP_FIND_GLOB, NULL );
  }

  if (CG_enable_peephole) {
    Set_Error_Phase("Extended Block Optimizer");
    Start_Timer(T_EBO_CU);
    EBO_Pre_Process_Region (region ? REGION_get_rid(rwn) : NULL);
    Stop_Timer ( T_EBO_CU );
    Check_for_Dump ( TP_EBO, NULL );
  }

  // Optimize control flow (first pass)
  if (CG_opt_level > 0 && CFLOW_opt_before_cgprep) {
    // Perform all the optimizations that make things more simple.
    // Reordering doesn't have that property.
    CFLOW_Optimize(  (CFLOW_ALL_OPTS|CFLOW_IN_CGPREP)
		       & ~(CFLOW_FREQ_ORDER | CFLOW_REORDER),
		       "CFLOW (first pass)");
    if (frequency_verify && CG_PU_Has_Feedback)
      FREQ_Verify("CFLOW (first pass)");
  }

  // Invoke global optimizations before register allocation at -O2 and above.
  if (CG_opt_level > 1) {

#ifdef TARG_ST
    if (CG_enable_ssa) {
      //
      // Experimental SSA framework: invoked at optimization levels
      // above 1, when CG_localize_tns is not ON.
      //
      Set_Error_Phase( "CG SSA Construction");
      SSA_Enter (region ? REGION_get_rid(rwn) : NULL, region);
      Check_for_Dump(TP_SSA, NULL);
      GRA_LIVE_Recalc_Liveness(region ? REGION_get_rid( rwn) : NULL);
      //      Trace_IR(TP_SSA, "GRA_LIVE_Recalc_Liveness", NULL);
      //extern void GRA_LIVE_fdump_liveness(FILE *f);
      //GRA_LIVE_fdump_liveness(TFile);
    }
#endif

    // Compute frequencies using heuristics when not using feedback.
    // It is important to do this after the code has been given a
    // cleanup by cflow so that it more closely resembles what it will
    // to the later phases of cg.
    if (!CG_PU_Has_Feedback) {
      Set_Error_Phase("FREQ");
      Start_Timer (T_Freq_CU);
      FREQ_Compute_BB_Frequencies();
      Stop_Timer (T_Freq_CU);
      if (frequency_verify)
	FREQ_Verify("Heuristic Frequency Computation");
    }

#ifdef TARG_ST

    // Arthur: there is a possibility that (like if-conversion)
    //         select generation is more related to the superblock
    //         formation than we think it is. We may do both at
    //         the same time eventually. For now, first do select
    //         then find superblocks.

#ifdef SUPPORTS_SELECT
    if (CG_enable_select) {
      // Perform select generation (partial predication if-conversion). 
      Start_Timer(T_Select);
      Convert_Select(region ? REGION_get_rid(rwn) : NULL, NULL);
      Stop_Timer(T_Select);
      if (frequency_verify)
	FREQ_Verify("Select Conversion");
      draw_CFG();
    }
#endif

#endif

#if defined(TARG_IA64) || defined(IFCONV_IN_SSA)
    //
    // Perform hyperblock formation (if-conversion). 
    // Depending on the flags makes Hyperblocks or Superblocks.
    //
    // TODO: tail duplication does not work in SSA !!!
    //
    if (CGTARG_Can_Predicate() || CGTARG_Can_Select()) {
      // Initialize the predicate query system in the hyperblock 
      // formation phase
      HB_Form_Hyperblocks(region ? REGION_get_rid(rwn) : NULL, NULL);
#ifdef TARG_IA64
      if (!PQSCG_pqs_valid()) {
	PQSCG_reinit(REGION_First_BB);
      }
#endif
      if (frequency_verify)
	FREQ_Verify("Hyberblock Formation");
    }
#endif

#ifdef TARG_ST
    if (CG_enable_ssa) {
      //
      //  Experimental SSA framework: 
      //
      //   Right after if-conversion, the SSA is consistent.
      //
      //   We do not know how to maintain the valid SSA during
      //   loop unrolling and SWP, and other
      //   optimizations that may change the control flow.
      //   For now our phylosophy is: global scheduling is done
      //   within hyperblocks and is thus local to them; the
      //   loop unrolling, SWP are done to inner loops and
      //   we will use the algorithm for updating the SSA for
      //   just the inner loops (we know how). 
      //
      //   JUST AN IDEA: becuse the SSA is consistent entering
      //   scheduling, SWP, etc., we may be able to simply
      //   remove the PHI nodes from the inner loops before
      //   such transformations, and restore the SSA when we are 
      //   done ?? 
      //
      Set_Error_Phase("Out of SSA Translation");

      //
      // Need SSA_make_consistent() to prepare SSA_Remove_Phi_Nodes()
      // NOTE: make sure liveness is up to date
      //
      SSA_Make_Consistent (region ? REGION_get_rid(rwn) : NULL, region);

      //
      // For now (temporary solution), we remove PHI-nodes here.
      // Later, this will be done in Unroll, SWP, ... (after the DDG
      // is constructed, etc.). And the SSA will be restored upon
      // exit from the Loop_Optimizations.
      //
      SSA_Remove_Phi_Nodes(region ? REGION_get_rid(rwn) : NULL, region);

      //
      // Arthur: we might eventually be able to maintain liveness 
      //         info throughout the PHI removal (see Sreedhar's paper).
      //
      // Normally, we shouldn't need to recompute it.
      // In addition, we may only need it for such transformations
      // (eg. Loop_Optimize()) where we remove the PHI-nodes 
      // temporarily, and later restore the SSA. When we leave the
      // the SSA definitely (like right now), when it is done after
      // the reg. alloc., we may not need liveness update at all.
      //
      // However ... at the moment
      //   defreach_in
      //   defreach_out
      //   live_def
      //   live_use
      // are not being updated properly by the out of SSA
      // algorithm (I've been too lazy to look at it). So, for now
      // just recompute the liveness.
      // TODO: fix this.
      //
      GRA_LIVE_Recalc_Liveness(region ? REGION_get_rid(rwn) : NULL);
      Check_for_Dump(TP_SSA, NULL);
    }
#endif

#ifdef TARG_ST
#ifndef IFCONV_IN_SSA
    // Perform superblock formation after select if-conversion. 
    //
    // TODO: do it in SSA when tail duplication works in SSA.
    //       Depending on the CGTARG_Supports_Predication(), do
    //       either Hyper- or Super- block formation.
    //
    HB_Form_Hyperblocks(region ? REGION_get_rid(rwn) : NULL, NULL);
    if (frequency_verify)
      FREQ_Verify("Hyperblock Formation");
#endif
#endif

    // GRA_LIVE_Init only done if !CG_localize_tns
    if (CG_enable_loop_optimizations && !CG_localize_tns) {
      Set_Error_Phase("CGLOOP");
      Start_Timer(T_Loop_CU);
      // Optimize loops (mostly innermost)
      Perform_Loop_Optimizations();
      // detect GTN
      GRA_LIVE_Recalc_Liveness(region ? REGION_get_rid( rwn) : NULL);	
      GRA_LIVE_Rename_TNs();  // rename TNs -- required by LRA
      Stop_Timer(T_Loop_CU);
      Check_for_Dump(TP_CGLOOP, NULL);
      if (frequency_verify)
	FREQ_Verify("CGLOOP");
    }

    /* Optimize control flow (second pass) */
    if (CFLOW_opt_after_cgprep) {
      CFLOW_Optimize(CFLOW_ALL_OPTS, "CFLOW (second pass)");
      if (frequency_verify)
	FREQ_Verify("CFLOW (second pass)");
    }

    if (CG_enable_peephole) {
      Set_Error_Phase( "Extended Block Optimizer");
      Start_Timer( T_EBO_CU );
      EBO_Process_Region (region ? REGION_get_rid(rwn) : NULL);
#ifdef TARG_IA64
      PQSCG_reinit(REGION_First_BB);
#endif
      Stop_Timer ( T_EBO_CU );
      Check_for_Dump ( TP_EBO, NULL );
    }
  }

  if (!Get_Trace (TP_CGEXP, 1024))
    Reuse_Temp_TNs = TRUE;	/* for spills */

  // This is usefull for debugging, OFF by default
  if (CGSPILL_Enable_Force_Rematerialization)
    CGSPILL_Force_Rematerialization();

  if (!region) {
    /* in case cgprep introduced a gp reference */
    Adjust_GP_Setup_Code( Get_Current_PU_ST(), FALSE /* allocate registers */ );

    /* in case cgprep introduced a lc reference */
    Adjust_LC_Setup_Code();

    // TODO:  when generate control speculation (ld.s) and st8.spill
    // of NaT bits, then need to save and restore ar.unat. 
  }

  /* Global register allocation, Scheduling:
   *
   * The overall algorithm is as follows:
   *   - Global code motion before register allocation
   *   - Local scheduling before register allocation
   *   - Global register allocation
   *   - Local register allocation
   *   - Global code motion phase (GCM) 
   *   - Local scheduling after register allocation
   */
  IGLS_Schedule_Region (TRUE /* before register allocation */);
  // Arthur: here rather than in igls.cxx
  Check_for_Dump (TP_SCHED, NULL);

  if (!CG_localize_tns) {
    // Earlier phases (esp. GCM) might have introduced local definitions
    // and uses for global TNs. Rename them to local TNs so that GRA 
    // does not have to deal with them.

    if (GRA_recalc_liveness) {
      Start_Timer( T_GLRA_CU);
      GRA_LIVE_Recalc_Liveness(region ? REGION_get_rid( rwn) : NULL);
      Stop_Timer ( T_GLRA_CU );
      Check_for_Dump (TP_FIND_GLOB, NULL);
    } else {
      GRA_LIVE_Rename_TNs ();
#if 0
      fprintf(TFile, "%s CFG After GRA_LIVE_Rename_TNs\n%s\n", DBar, DBar);
      Print_All_BBs ();
#endif

    }

    if (GRA_redo_liveness) {
      Start_Timer( T_GLRA_CU );
      GRA_LIVE_Init(region ? REGION_get_rid( rwn ) : NULL);
      Stop_Timer ( T_GLRA_CU );
      Check_for_Dump ( TP_FIND_GLOB, NULL );
    }

    GRA_Allocate_Global_Registers( region );
    // Arthur: moved here rather than in gra.cxx
    Check_for_Dump (TP_GRA, NULL);
  }

  LRA_Allocate_Registers (!region);
  // Arthur: moved here rather than in lra.cxx
  Check_for_Dump (TP_ALLOC, NULL);

#ifdef TARG_ST
  if (CG_enable_ssa) {
    //
    // Collect statistical info about the SSA:
    //
    SSA_Collect_Info(region ? REGION_get_rid( rwn ) : NULL, region, TP_ALLOC);
  }
#endif

  if (!CG_localize_tns ) {
    Set_Error_Phase ( "GRA_Finish" );
    /* Done with all grant information */
    GRA_Finalize_Grants();
  }

  if (!region) {
    /* Check that we didn't introduce a new gp reference */
    Adjust_GP_Setup_Code( Get_Current_PU_ST(), TRUE /* allocate registers */ );

    /* The stack frame is final at this point, no more spilling after this.
     * We can set the Frame_Len now.
     * Then we can go through all the entry/exit blocks and fix the SP 
     * adjustment OP or delete it if the frame length is zero.
     */
    Set_Frame_Len (Finalize_Stack_Frame());
    Set_Error_Phase ( "Final SP adjustment" );
    Adjust_Entry_Exit_Code ( Get_Current_PU_ST() );
  }

  if (CG_enable_peephole) {
    Set_Error_Phase("Extended Block Optimizer");
    Start_Timer(T_EBO_CU);
    EBO_Post_Process_Region (region ? REGION_get_rid(rwn) : NULL);
    Stop_Timer ( T_EBO_CU );
    Check_for_Dump ( TP_EBO, NULL );
  }

  IGLS_Schedule_Region (FALSE /* after register allocation */);
  // Arthur: here rather than in igls.cxx
  Check_for_Dump (TP_SCHED, NULL);

  Reuse_Temp_TNs = orig_reuse_temp_tns;		/* restore */

  if (region) {
    /*--------------------------------------------------------------------*/
    /* old region: rwn, rid_orig					  */
    /* new region: rwn_new, rid_new (encloses old region)		  */
    /*--------------------------------------------------------------------*/
    WN	*inner_body, *outer_body, *exitBlock, *comment;
    WN  *rwn_new, *result_block_before, *result_block_after;
    RID *rid_orig;
    char str[100];

    Is_True(REGION_consistency_check(rwn),("CG_Generate_Code"));
    rid_orig = REGION_get_rid(rwn);

    /* don't delete rwn, it contains the stub that helps find the MOPS
       that the region has been lowered to */

    outer_body = WN_CreateBlock();
    /* put inner region inside outer containment block */
    WN_INSERT_BlockFirst(outer_body, rwn);
    /* we assembled the new exit block earlier in Build_CFG()		*/
    exitBlock = CGRIN_nested_exit(RID_cginfo(rid_orig));
    WN_region_exits(rwn) = exitBlock; /* PPP ??? */

    rwn_new = outer_body;

    /* put a note in the inner body that the code isn't there anymore */
    inner_body = WN_CreateBlock();
    WN_region_body(rwn) = inner_body; /* overwrite old body, now in MOPs */
    sprintf(str,"RGN %d has been lowered to MOPs, level=%s",
	    RID_id(rid_orig), RID_level_str(rid_orig));
    comment = WN_CreateComment(str);
    WN_INSERT_BlockFirst(inner_body, comment);

    /* Need to split result block for glue code into two parts: before and
       after the region body. The reason we can't just insert the glue code
       directly before or after the region directly is that we need to keep
       it separate for updating the alias info.
       If CG_LOOP has made some WHIRL glue, it is inserted in result_block. */
    result_block_before = WN_CreateBlock();
    result_block_after = WN_CreateBlock();

    /* fill-in blocks with glue code */
    Set_Error_Phase("Region Finalize");
    Start_Timer(T_Region_Finalize_CU);
    CG_Region_Finalize( result_block_before, result_block_after,
		       rwn, alias_mgr, TRUE /* generate_glue_code */ );
    Stop_Timer(T_Region_Finalize_CU);

    /* generate alias information for glue code */
    REGION_update_alias_info(result_block_before, alias_mgr);
    REGION_update_alias_info(result_block_after, alias_mgr);

    /* insert glue code before and after */
    WN_INSERT_BlockFirst( rwn_new, result_block_before );
    WN_INSERT_BlockLast( rwn_new, result_block_after );

    GRA_LIVE_Finish_REGION();
#ifdef SUPPORTS_PREDICATION
    PQSCG_term();
#endif
    Stop_Timer ( T_CodeGen_CU );
    Set_Error_Phase ( "Codegen Driver" );

    return rwn_new;
  } /* if (region */

  else { /* PU */
    /* Write the EH range table. */
    if (PU_has_exc_scopes(Get_Current_PU())) {
      EH_Write_Range_Table(rwn);
    }

    /* Emit the code for the PU. This may involve writing out the code to
     * an object file or to an assembly file or both. Additional tasks
     * performed by this module are:
     *
     *   - convert long branches into a chain of short branches.
     *   - add nada's to quad-align branch targets for TFP.
     */
    Set_Error_Phase ( "Assembly" );
    Start_Timer (	T_Emit_CU );

    EMT_Emit_PU (Get_Current_PU_ST(), pu_dst, rwn);
    Check_for_Dump (TP_EMIT, NULL);
    Stop_Timer ( T_Emit_CU );

    Set_Error_Phase("Region Finalize");
    Start_Timer(T_Region_Finalize_CU);
    CG_Region_Finalize( NULL, NULL, rwn, alias_mgr,
		       FALSE /* generate_glue_code */ );
    Stop_Timer(T_Region_Finalize_CU);

    GRA_LIVE_Finish_PU();
#ifdef SUPPORTS_PREDICATION
    PQSCG_term();
#endif
    /* List local symbols if desired: */
    if ( List_Symbols )
	Print_symtab (Lst_File, CURRENT_SYMTAB);

    Stop_Timer ( T_CodeGen_CU );
    Set_Error_Phase ( "Codegen Driver" );

    return rwn;
  }
}

/* ================================================================= */
/* routines for dumping/tracing the program */

/* =================================================================
 *   Trace_IR
 * =================================================================
 */
void
Trace_IR(
  INT phase,		/* Phase after which we're printing */
  const char *pname,	/* Print name for phase	*/
  BB *cur_bb)		/* BB to limit traces to */
{
  INT cur_bb_id = cur_bb ? BB_id(cur_bb) : 0;
  if (   Get_Trace(TKIND_IR, phase)
      && (cur_bb_id == 0 || Get_BB_Trace(cur_bb_id)))
  {
    fprintf(TFile, "\n%s%s\tIR after %s\n%s%s\n",
	    DBar, DBar, pname, DBar, DBar);
    if (cur_bb != NULL) {
      Print_BB(cur_bb);
    } else {
      BB *bb;
      for (bb = REGION_First_BB; bb; bb = BB_next(bb))	{
	if (Get_BB_Trace(BB_id(bb)) && Get_Trace(TKIND_IR, phase)) {
	  Print_BB(bb);
	}
      }
    }
    fprintf(TFile, "%s%s\n", DBar, DBar);
  }
}

/* =================================================================
 *   Trace_TN
 * =================================================================
 */
static void
Trace_TN (
  INT phase,		/* Phase after which we're printing */
  const char *pname )	/* Print name for phase	*/
{
  if ( Get_Trace ( TKIND_TN, phase ) ) {
    fprintf ( TFile, "\n%s%s\tTNs after %s\n%s%s\n",
	      DBar, DBar, pname, DBar, DBar );
    Print_TNs ();
  }
}

/* =================================================================
 *   Trace_ST
 * =================================================================
 */
static void
Trace_ST (
  INT phase,		/* Phase after which we're printing */
  const char *pname )	/* Print name for phase	*/
{
  if ( Get_Trace ( TKIND_SYMTAB, phase ) ) {
  	fprintf ( TFile, "\n%s%s\tSymbol table after %s\n%s%s\n",
              DBar, DBar, pname, DBar, DBar );
  	SYMTAB_IDX level = CURRENT_SYMTAB;
	while (level >= GLOBAL_SYMTAB) {
	  	Print_symtab (TFile, level);
		--level;
	}
  }
}

/* ====================================================================
 *   Check_for_Dump
 *
 *   Check whether symbol table, TN, or IR dumps have been requested for
 *   the given pass; if so, generate them to the trace file.  If a BB is
 *   given, limit the dumps to that BB.
 * ====================================================================
 */
void
Check_for_Dump ( INT32 pass, BB *bb )
{
  if (bb == NULL || Get_BB_Trace(BB_id(bb))) {
    const char *s = Get_Error_Phase();

    /* Check to see if we should dump the STAB.
     */
    Trace_ST ( pass, s );

    /* Check to see if we should dump the TNs.
     */
    Trace_TN ( pass, s );

    /* Check to see if we should dump the IR.  If yes, check each BB.
     */
    Trace_IR ( pass, s, bb );

    /* Check to see if we should give a memory allocation trace.
     */
    Trace_Memory_Allocation ( pass, s );
  }
}

/* =================================================================
 *   Get_Trace
 * =================================================================
 */
BOOL 
Get_Trace ( INT func, INT arg, BB *bb )
{
  BOOL result = Get_Trace(func, arg);

  /* Check the BB if necessary: */
  if ( result && bb ) {
    result = Get_BB_Trace ( BB_id(bb) );
  }

  return result;
}

/* =================================================================
 *   CG_Dump_Region
 * =================================================================
 */
void
CG_Dump_Region(FILE *fd, WN *wn)
{
  RID	*rid = REGION_get_rid(wn);
  Is_True(rid != NULL, ("CG_Dump_Region, NULL RID"));
  if (rid && RID_level(rid) >= RL_CGSCHED) {
    CGRIN  *cgrin = RID_cginfo(rid);
    if (cgrin && CGRIN_entry(cgrin)) {
      BB *bb;
      for (bb=CGRIN_entry(cgrin); bb; bb=BB_next(bb))
	Print_BB( bb );
    }
  }
}


/* just an externally-visible wrapper to cgemit function */
extern void
CG_Change_Elf_Symbol_To_Undefined (ST *st)
{
  /* EMT_Change_Symbol_To_Undefined(st); */
  FmtAssert(FALSE,("not implemented"));
}

