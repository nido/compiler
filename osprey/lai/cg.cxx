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
#ifdef TARG_ST
#include "cg_ivs.h"
#endif
#include "loop_invar_hoist.h"
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
#include "cg_automod.h"
#ifdef SUPPORTS_PREDICATION
#include "pqs_cg.h"
#endif
#ifdef SUPPORTS_SELECT
#include "cg_select.h"
#endif
#include "tag.h"

#ifdef TARG_ST
#include "cg_ssa.h"
#include "cg_ssaopt.h"
#include "dominate.h"
#include "loop_invar_hoist.h"
#include "ipra.h"
#include "cg_color.h"
#include "cg_tailmerge.h"
#endif

#ifdef LAO_ENABLED
#include "lao_stub.h"
#endif

#ifdef TARG_ST
#include "top_properties.h"
#include "mexpand.h"
#endif

#ifdef TARG_ST
// [TB] gcov coverage utilities  
#include "gcov_profile.h"
#endif

MEM_POOL MEM_local_region_pool;	/* allocations local to processing a region */
MEM_POOL MEM_local_region_nz_pool;

BOOL Trace_REGION_Interface = FALSE;

BOOL PU_Has_Calls;
BOOL PU_References_GP;
#ifdef TARG_ST
BOOL PU_Has_Asm;
BOOL PU_Has_Hwloops;
BOOL PU_Has_EH_Return;
#endif
#ifdef KEY
BOOL PU_Has_Exc_Handler;
#endif

BOOL CG_PU_Has_Feedback;

BOOL Reuse_Temp_TNs = FALSE;

RID *Current_Rid;

TN_MAP TN_To_PREG_Map;

/* WOPT alias manager */
struct ALIAS_MANAGER *Alias_Manager;

#ifdef TARG_ST
IPRA cg_ipra;
#endif


#ifdef TARG_ST
/* ====================================================================
 *    CGStack_Align_Check()
 *
 *   This function check that the alignment constraint related
 *   to local symbols is compatible with the stack alignment.
 *
 *   Basically there are two phases: 
 *   1. determine the required alignment (accounts for function level
 *	alignment attribute and alignment of automatics);
 *   2. if CG_auto_align is true, set the required alignment, 
 *	otherwise, emit a warning if it exceeds the natural stack alignment.
 *
 *   FdF 20080212: Note that this function cannot get alignment
 *   constraints from varargs parameters
 * ====================================================================
 */
static void CGStack_Align_Check() {
  // [dt25] if symbol is a local defined as having an alignment 
  // constraint > PU_aligned_stack(Get_Current_PU()) then we should force the stack allignment

  INT i,Max_Stack_Align = PU_aligned_stack(Get_Current_PU());
  bool user_stack_align = false;
  bool Update = false;
  ST *sym;

  // We detect if the stack alignment has been set by the user
  if (Max_Stack_Align != Target_Stack_Alignment) user_stack_align = true;
  FOREACH_SYMBOL(CURRENT_SYMTAB,sym,i) {
    if (ST_class(sym) == CLASS_VAR && ST_sclass(sym) == SCLASS_AUTO && TY_align(ST_type(sym))>Max_Stack_Align) {
      Update = true;
      Max_Stack_Align = TY_align(ST_type(sym));
    }
  }
  
  if (Update && !CG_auto_align_stack ) {
    // Warn for undefined behavior
    ErrMsg(EC_Warn_Stack_Exceeded,ST_name(Get_Current_PU_ST()),(int)PU_aligned_stack(Get_Current_PU()),Max_Stack_Align);
  }

  if (Update && CG_auto_align_stack) {
    if (user_stack_align) {
      ErrMsg(EC_Warn_Stack_Align,ST_name(Get_Current_PU_ST()),(int)PU_aligned_stack(Get_Current_PU()),Max_Stack_Align);
      ErrMsg (EC_Warn_Stack_Modif,ST_name(Get_Current_PU_ST()),(int)PU_aligned_stack(Get_Current_PU()),Max_Stack_Align);
    }
    //ErrMsg (EC_Warn_Stack_Modif,ST_name(Get_Current_PU_ST()),(int)PU_aligned_stack(Get_Current_PU()),Max_Stack_Align);
    Set_PU_aligned_stack(Get_Current_PU(),Max_Stack_Align);
  }

}
#endif


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
#ifdef TARG_ST
  PU_Has_Asm = FALSE;
  PU_Has_Hwloops = FALSE;
  PU_Has_EH_Return = FALSE;
#endif
#ifdef KEY
  PU_Has_Exc_Handler = FALSE;
#endif

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

#ifdef TARG_ST
  /* Initialize cg_color module. */
  CGCOLOR_Initialize_For_PU();
#endif
#ifdef TARG_ST
  CGStack_Align_Check();
#endif

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

#ifdef TARG_ST
  // [CG]: In debug mode we perform on check on topcode
#if Is_True_On
  for(int i = 0; i < TOP_count; ++i) {
    FmtAssert(TOP_check_properties((TOP)i), ("topcode %s as inconsistant properties", TOP_Name((TOP)i)));
  }
#endif
#endif

#ifdef LAO_ENABLED
  if (CG_LAO_optimizations != 0) lao_init_pu();
#endif

  return;
}

/* ====================================================================
 *   CG_PU_Finalize(void)
 * ====================================================================
 */
void
CG_PU_Finalize(void)
{

#ifdef LAO_ENABLED
  if (CG_LAO_optimizations != 0) lao_fini_pu();
#endif

  TAG_Finish();
  GTN_UNIVERSE_Pu_End ();
  OP_MAP_Finish();
  CGCOLOR_Finalize_For_PU();
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
  
  /* TB: If coverage, do the instrumention + gcno dump */
  if (Profile_Arcs_Enabled_Cgir)
    gcov_pu();

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
#ifdef TARG_ST
    // FdF 2005/11/02: Cloning may decrease if-conversion
    // opportunities.
    if (CG_enable_ssa && CG_enable_select)
      CFLOW_Optimize(  (CFLOW_ALL_OPTS|CFLOW_IN_CGPREP)
		       & ~(CFLOW_FREQ_ORDER | CFLOW_REORDER | CFLOW_CLONE),
		       "CFLOW (first pass)");
    else
#endif
    CFLOW_Optimize(  (CFLOW_ALL_OPTS|CFLOW_IN_CGPREP)
		       & ~(CFLOW_FREQ_ORDER | CFLOW_REORDER),
		       "CFLOW (first pass)");
    if (frequency_verify && CG_PU_Has_Feedback)
      FREQ_Verify("CFLOW (first pass)");
  }

  // FdF 20051212: Moved the computation of frequencies before the
  // loop invariant code motion, because it uses these frequencies to
  // compute the profitability of code motion.
#ifdef BCO_ENABLED /* Thierry */
  //TB: CG_emit_bb_freqs_arcs trigger CG_emit_bb_freqs
  if (CG_emit_bb_freqs_arcs)
    CG_emit_bb_freqs = CG_emit_bb_freqs_arcs;
  // THierry begin : Enable compute bb frequencies even if opt_level <= 1 when -CG:emit_bb_freqs=on
  if (CG_opt_level > 1 || CG_emit_bb_freqs)
#else
  if (CG_opt_level > 1)
#endif
    {
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
    }

#ifdef TARG_ST
  // FdF: Code imported from ORC2.1. Perform before SSA and if-conversion.
  if (IPFEC_Enable_LICM && (IPFEC_Enable_LICM_passes & 0x01 ) && CG_opt_level > 1 && !CG_localize_tns) {
      Set_Error_Phase("Perform_Loop_Invariant_Code_Motion");
      Perform_Loop_Invariant_Code_Motion ();
      Check_for_Dump ( TP_LICM, NULL );
    }
#endif
  // Invoke global optimizations before register allocation at -O2 and above.
  if (CG_opt_level > 1 && !CG_localize_tns) {
#ifdef TARG_ST
    if (CG_enable_ssa)
      CG_enable_ssa = SSA_Check(region ? REGION_get_rid(rwn) : NULL, region);
    if (CG_enable_ssa) {
      // Enter SSA.
      // Precondition: live-analysis ok.
      Set_Error_Phase( "CG SSA Construction");
      SSA_Enter (region ? REGION_get_rid(rwn) : NULL, region);
      SSA_Verify (region ? REGION_get_rid(rwn) : NULL, region);
      GRA_LIVE_Recalc_Liveness(region ? REGION_get_rid( rwn) : NULL);
      Check_for_Dump(TP_SSA, NULL);
      // Postcondition: code in PSI-SSA, dominators ok, live-analysis ok.

      // Perform SSA dataflow optimizations
      // Precondition: code in PSI-SSA, dominators ok, live-analysis ok.
      Set_Error_Phase( "CG SSA DataFlow Optimizations");
      SSA_Optimize();
      SSA_Verify (region ? REGION_get_rid(rwn) : NULL, region);
      GRA_LIVE_Recalc_Liveness(region ? REGION_get_rid( rwn) : NULL);
      Check_for_Dump(TP_SSA, NULL);
      // Postcondition: code in PSI-SSA, dominators ok, live-analysis ok.

      // Clean up the dominator/posdominator information before control flow optimizations.
      Free_Dominators_Memory();
      // Postcondition: code in PSI-SSA, live-analysis ok. dominators freed.
    }
#endif
  } //CG_opt_level > 1
  
  if (CG_opt_level > 1 && !CG_localize_tns) {
#ifdef TARG_ST
      if (CG_enable_ssa) {
	// Perform SSA controlflow optimizations
	// Precondition: code in PSI-SSA, live-analysis ok.
	Set_Error_Phase( "CG SSA ControlFlow Optimizations");
#ifdef SUPPORTS_SELECT
	if (CG_enable_select) {
	  // Perform select generation (partial predication if-conversion). 
	  Start_Timer(T_Select_CU);
	  Convert_Select(region ? REGION_get_rid(rwn) : NULL, NULL);
	  Stop_Timer(T_Select_CU);
	  if (frequency_verify)
	    FREQ_Verify("Select Conversion");
	  Check_for_Dump(TP_SELECT, NULL);
	}
#endif
	// Postcondition: code in PSI-SSA, live-analysis ok.
      }
#endif
  }
  
  if (CG_opt_level > 1 && !CG_localize_tns) {
#if defined(TARG_IA64)
    //
    // Perform hyperblock formation (if-conversion). 
    // Depending on the flags makes Hyperblocks or Superblocks.
    //
    if (CGTARG_Can_Predicate() || CGTARG_Can_Select()) {
      // Initialize the predicate query system in the hyperblock 
      // formation phase
      HB_Form_Hyperblocks(region ? REGION_get_rid(rwn) : NULL, NULL);
#if defined TARG_IA64 || (defined TARG_ST && defined SUPPORTS_PREDICATION)
      if (!PQSCG_pqs_valid()) {
	PQSCG_reinit(REGION_First_BB);
      }
#endif
      if (frequency_verify)
	FREQ_Verify("Hyberblock Formation");
    }
#endif

#ifdef TARG_ST
    if (CG_enable_ssa && !CG_localize_tns) {
      // Exit SSA.
      // Precondition: code in PSI-SSA, live-analysis ok, dominator freed.
      Set_Error_Phase("Out of SSA Translation");
      SSA_Make_Conventional (region ? REGION_get_rid(rwn) : NULL, region);
      SSA_Remove_Pseudo_OPs(region ? REGION_get_rid(rwn) : NULL, region);
      GRA_LIVE_Recalc_Liveness(region ? REGION_get_rid(rwn) : NULL);
      Check_for_Dump(TP_SSA, NULL);
      // Postcondition: live-analysis ok, dominator freed.
#ifdef SUPPORTS_PREDICATION
      PQSCG_reinit(REGION_First_BB);
#endif
    }
#endif

#ifdef TARG_ST
    // [CG] Run control flow opt after SSA.
    // Currently enabled only if if_convert is true
    // Run also an EBO pre pass after if-conversion as
    // merge points may have been removed
    if (CG_enable_ssa && CG_enable_select && !CG_localize_tns) {
#ifdef TARG_ST
      // FdF 2005/11/02: Perform cloning after if-conversion.
      CFLOW_Optimize(CFLOW_MERGE|CFLOW_CLONE, "CFLOW (after ssa)");
#else
      CFLOW_Optimize(CFLOW_MERGE, "CFLOW (after ssa)");
#endif
      if (CG_enable_peephole) {
	Set_Error_Phase("Extended Block Optimizer (after ssa)");
	Start_Timer(T_EBO_CU);
	EBO_Pre_Process_Region (region ? REGION_get_rid(rwn) : NULL);
	Stop_Timer ( T_EBO_CU );
	Check_for_Dump ( TP_EBO, NULL );
      }
      if (IPFEC_Enable_LICM && (IPFEC_Enable_LICM_passes & 0x02) ) {
	// Another pass of invariant code motion is useful after if-conversion
	// as there are more speculated instructions in loop to hoist.
	Set_Error_Phase("Perform_Loop_Invariant_Code_Motion after SSA");
	Perform_Loop_Invariant_Code_Motion ();
	Check_for_Dump ( TP_LICM, NULL );
        }
    }
#endif

    // Now done just before IGLS because we want to include regions in
    // loops.

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
#ifdef TARG_ST
      if (CG_AutoMod) {
          GRA_LIVE_Recalc_Liveness(region ? REGION_get_rid( rwn) : NULL);
	Perform_AutoMod_Optimization();
	EBO_Process_Region (region ? REGION_get_rid(rwn) : NULL);
	GRA_LIVE_Recalc_Liveness(region ? REGION_get_rid( rwn) : NULL);
      }
#endif
#if defined TARG_IA64 || (defined TARG_ST && defined SUPPORTS_PREDICATION)
      PQSCG_reinit(REGION_First_BB);
#endif
      Stop_Timer ( T_EBO_CU );
      Check_for_Dump ( TP_EBO, NULL );

#ifdef TARG_ST
    // [vcdv] second call to PLICM after hwloop detection so that
    // hwloop invariant instructions can  be factorized
      if (IPFEC_Enable_LICM && (IPFEC_Enable_LICM_passes & 0x04) &&
          CG_opt_level > 1 && !CG_localize_tns) {
      Set_Error_Phase("Perform_Loop_Invariant_Code_Motion");
      Perform_Loop_Invariant_Code_Motion ();
    }
#endif

      // FdF: Useful to run CFLOW_optimize again because EBO may have
      // propagated constants such that conditional branches become
      // inconditional.
      /* Optimize control flow (third pass) */
      if (CFLOW_opt_after_cgprep) {
	CFLOW_Optimize(CFLOW_ALL_OPTS, "CFLOW (third pass)");
	if (frequency_verify)
	  FREQ_Verify("CFLOW (third pass)");
      }
    }

  }  /* CG_opt_level > 1 */ 

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

#ifdef TARG_ST
  CGTARG_Resize_Instructions ();
#endif

  Set_Error_Phase( "Prepass Scheduling" );

  extern void Check_Prolog_Epilog ();
  Check_Prolog_Epilog();

#ifdef LAO_ENABLED
  // Call the LAO for software pipelining and prepass scheduling.
  if (CG_LAO_optimizations & OptimizeActivation_PrePass) {
    GRA_LIVE_Recalc_Liveness(region ? REGION_get_rid( rwn) : NULL);
    GRA_LIVE_Rename_TNs();
    LAO_Schedule_Region(TRUE /* before register allocation */, frequency_verify);
    // [SC]: Calculate local register requirements.
    for (BB *bb = REGION_First_BB; bb != NULL; bb = BB_next(bb)) {
      LRA_Compute_Register_Request (bb, &MEM_local_region_pool);
    }
  } else {
    IGLS_Schedule_Region (TRUE /* before register allocation */);
#ifdef TARG_ST
    // FdF 20080328: Reset loop trip count to remove an artificial use
    // for register allocation.
    BOOL need_live = FALSE;
    for (BB *bb = REGION_First_BB; bb != NULL; bb = BB_next(bb)) {
      if (BB_loophead(bb)) {
	ANNOTATION *info_ant = ANNOT_Get(BB_annotations(bb), ANNOT_LOOPINFO);
	LOOPINFO *info = info_ant ? ANNOT_loopinfo(info_ant) : NULL;
	TN *trip_count_tn = info ? LOOPINFO_primary_trip_count_tn(info) : NULL;
	if (trip_count_tn != NULL && TN_is_register(trip_count_tn)) {
	  LOOPINFO_primary_trip_count_tn(info) = NULL;
	  need_live = TRUE;
	}
      }
    }
    if (need_live) {
      GRA_LIVE_Recalc_Liveness(NULL);
      GRA_LIVE_Rename_TNs();
    }
#endif
  }
#else
  IGLS_Schedule_Region (TRUE /* before register allocation */);
#endif
  // Arthur: here rather than in igls.cxx
  Check_for_Dump (TP_SCHED, NULL);

#ifdef TARG_ST
  extern void Schedule_Prefetch_Prepass(void);
  Schedule_Prefetch_Prepass();
#endif

  // Register Allocation Phase
#ifdef LAO_ENABLED
  if (CG_LAO_optimizations & OptimizeActivation_RegAlloc) {
    // Live analysis and tn renaming
    GRA_LIVE_Recalc_Liveness(region ? REGION_get_rid( rwn) : NULL);	
    GRA_LIVE_Rename_TNs();
    Set_Error_Phase( "LAO RegAlloc Optimizations" );
    lao_optimize_pu(CG_LAO_optimizations & OptimizeActivation_RegAlloc);
    Check_for_Dump (TP_ALLOC, NULL);
    if (CG_LAO_allocation > 0) {
    // Full register allocation performed by LAO.
    } else  {
      // Localization performed by LAO, run LRA
      Set_Error_Phase( "LRA after LAO RegAlloc" );
      // Force LRA to not use GRA informations.
      bool old = CG_localize_tns;
      CG_localize_tns = TRUE;
      LRA_Allocate_Registers (!region);
      CG_localize_tns = old;
      Check_for_Dump (TP_ALLOC, NULL);
    }
  } else {
  // No allocation  performed by LAO, run full GRA/LRA
#endif
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
#ifdef TARG_ST
    if (CG_enable_rename_after_GRA) {
      // [SC] To split the local live ranges introduced
      // by GRA spilling.
      GRA_LIVE_Rename_TNs();
      Check_for_Dump (TP_LOCALIZE, NULL);
    }
#endif
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

#ifdef TARG_ST
  Set_Error_Phase ("MExpand");
  Convert_To_Multi_Ops ();
  Check_for_Dump (TP_ALLOC, NULL);
#endif

#ifdef LAO_ENABLED
  } /* !CG_LAO_optimizations */
#endif

  if (!region) {
    /* Check that we didn't introduce a new gp reference */
    Adjust_GP_Setup_Code( Get_Current_PU_ST(), TRUE /* allocate registers */ );

#ifdef TARG_ST
    /* Adjust stack frame temporaries before finalizing the frame layout. */
    Adjust_Stack_Frame(Get_Current_PU_ST());
#endif
    /* The stack frame is final at this point, no more spilling after this.
     * We can set the Frame_Len now.
     * Then we can go through all the entry/exit blocks and fix the SP 
     * adjustment OP or delete it if the frame length is zero.
     */
    Set_Frame_Len (Finalize_Stack_Frame());
    Set_Error_Phase ( "Final SP adjustment" );
    Adjust_Entry_Exit_Code ( Get_Current_PU_ST() );
  }

#ifdef TARG_ST
  // We have to pass after adjust entry exit code, because for stxp70 target,
  // we cannot update gra liveness between regalloc and the adjustment (pb
  // invalid avail temps in adjust entry code (R14 sets as available!))
  Set_Error_Phase ( "Tailmerge 1" );
  Tailmerge(1);
#endif

  if (CFLOW_Space)
    CFLOW_Optimize(CFLOW_MERGE_OPS|CFLOW_MERGE_EMPTY, "CFLOW (merge ops)");

  if (CG_enable_peephole) {
    Set_Error_Phase("Extended Block Optimizer");
    Start_Timer(T_EBO_CU);
    EBO_Post_Process_Region (region ? REGION_get_rid(rwn) : NULL);
    Stop_Timer ( T_EBO_CU );
    Check_for_Dump ( TP_EBO, NULL );
  }

#ifdef TARG_ST
  CGTARG_Resize_Instructions ();
  CGTARG_Pseudo_Make_Expand();
#endif

#ifdef LAO_ENABLED
  if (CG_LAO_optimizations & OptimizeActivation_PostPass) {
    GRA_LIVE_Recalc_Liveness(region ? REGION_get_rid( rwn) : NULL);
    LAO_Schedule_Region(FALSE /* after register allocation */, frequency_verify);
  } else {
    IGLS_Schedule_Region (FALSE /* after register allocation */);
  }
#else
  IGLS_Schedule_Region (FALSE /* after register allocation */);
#endif
  // Arthur: here rather than in igls.cxx
  Check_for_Dump (TP_SCHED, NULL);

#ifdef TARG_ST
  extern void Schedule_Prefetch_Postpass(void);
  if (CG_opt_level >= 3)
    Schedule_Prefetch_Postpass ();
  if (!region) {
    cg_ipra.Save_Info (Get_Current_PU_ST() );
  }
#endif

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
#ifdef TARG_STxP70
    CGTARG_Fixup_Immediates();
    Perform_HwLoop_Checking();
#endif
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

