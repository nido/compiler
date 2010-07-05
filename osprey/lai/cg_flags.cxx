/*

  Copyright (C) 2000 Silicon Graphics, Inc.  All Rights Reserved.

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


/* =======================================================================
 * =======================================================================
 *
 *  Module: cg_flags.c
 *
 *  Description:
 *  ============
 *
 *  Definition of variables global to all of CG.
 *
 * =======================================================================
 * =======================================================================
 */


#include "defs.h"
#include "cg_flags.h"
#include "cgtarget.h"

INT32 CG_skip_after = INT32_MAX;
INT32 CG_skip_before = 0;
INT32 CG_skip_equal = -1;
INT32 CG_local_skip_after = INT32_MAX;
INT32 CG_local_skip_before = 0;
INT32 CG_local_skip_equal = -1;
BOOL CG_skip_local_hbf = FALSE;
BOOL CG_skip_local_loop = FALSE;
BOOL CG_skip_local_sched = FALSE;
BOOL CG_skip_local_swp = FALSE;
INT  CG_opt_level;
BOOL CG_localize_tns;
BOOL CG_localize_tns_Set = FALSE;
BOOL LOCALIZE_using_stacked_regs = TRUE;
BOOL CG_unique_exit = TRUE;

BOOL CG_gen_callee_saved_regs_mask = FALSE; /* generate register mask */

BOOL FREQ_enable = TRUE;
BOOL FREQ_view_cfg = FALSE;
const char *FREQ_frequent_never_ratio = "1000.0";
#ifdef TARG_ST
// (cbr) this is enough
const char *FREQ_eh_freq = "0.01";
#else
const char *FREQ_eh_freq = "0.1";
#endif

BOOL CG_enable_rename = TRUE;
#ifdef TARG_ST
BOOL CG_enable_rename_after_GRA = TRUE;
BOOL CG_enable_min_max_abs = TRUE;
#endif
BOOL CG_enable_prefetch = FALSE;
BOOL CG_enable_z_conf_prefetch  = FALSE;
BOOL CG_enable_nz_conf_prefetch = FALSE;
BOOL CG_enable_pf_L1_ld = FALSE;
BOOL CG_enable_pf_L1_st = FALSE;
BOOL CG_enable_pf_L2_ld = FALSE;
BOOL CG_enable_pf_L2_st = FALSE;
BOOL CG_exclusive_prefetch = FALSE;
BOOL CG_enable_peephole = FALSE;
BOOL CG_enable_ssa = FALSE;	/* Enable SSA in cg */
#ifdef TARG_ST
// FdF 20070206
BOOL  CG_warn_prefetch_padding = 0;
// [TTh] Control of preferred register sets
BOOL  CG_COLOR_use_pref_regs = TRUE;
INT32 CG_COLOR_pref_regs_priority = PREF_REGS_PRIORITY_MEDIUM;
#endif

BOOL CG_enable_select = FALSE;
BOOL CG_enable_range_propagation = FALSE;
INT32 CG_range_recompute_limit = 2;
BOOL CG_enable_cbpo;

INT32 CG_LAO_activation = 0;
INT32 CG_LAO_regiontype = 0;
INT32 CG_LAO_conversion = 0;
INT32 CG_LAO_coalescing = 0;
INT32 CG_LAO_predication = 0;
INT32 CG_LAO_scheduling = 0;
INT32 CG_LAO_allocation = 0;
INT32 CG_LAO_rcmssolving = 0;
INT32 CG_LAO_preloading = 0;
INT32 CG_LAO_l1missextra = 0;
INT32 CG_LAO_compensation = 0;
INT32 CG_LAO_speculation = 0;
INT32 CG_LAO_relaxation = 0;
INT32 CG_LAO_pipelining = 0;
INT32 CG_LAO_logtimeout = 0;
INT32 CG_LAO_renaming = 0;
INT32 CG_LAO_boosting = 0;
INT32 CG_LAO_aliasing = 0;
INT32 CG_LAO_prepadding = 0;
INT32 CG_LAO_postpadding = 0;
INT32 CG_LAO_overrun = 0;
INT32 CG_LAO_opslimit = 0;

#ifdef CGG_ENABLED
BOOL CG_enable_cgg;
INT32 CG_cgg_level;
#endif

BOOL  CG_enable_BB_splitting = TRUE;
INT32 CG_split_BB_length = CG_bblength_default;

INT32 CG_L1_ld_latency = 0;
INT32 CG_L2_ld_latency = 0;
INT32 CG_z_conf_L1_ld_latency = 0;
INT32 CG_z_conf_L2_ld_latency = 0;
INT32 CG_ld_latency = 0;
INT32 CG_L1_pf_latency = 12;
INT32 CG_L2_pf_latency = 12;

INT32 CG_branch_mispredict_penalty= -1;		/* means not set */
INT32 CG_branch_mispredict_factor= -1;		/* means not set */

BOOL CG_warn_bad_freqs = FALSE;
BOOL CG_enable_loop_optimizations = FALSE;
#ifdef BCO_ENABLED /* Thierry */
BOOL CG_emit_bb_freqs = FALSE;
BOOL CG_emit_bb_freqs_arcs = FALSE;
#endif /* BCO_Enabled Thierry */
BOOL CG_enable_feedback = FALSE;
BOOL CG_tail_call = FALSE;
#ifdef TARG_ST
// [CL] force spill of return address (RA) so that
// unwinding/backtracing is still possible
BOOL CG_save_return_address = FALSE;
#endif

// TODO: set the default value here and ovewrite in target-specific
//       initialization part.
#ifdef TARG_IA64
BOOL CG_enable_thr = TRUE;
BOOL CG_cond_defs_allowed = TRUE;
#else
BOOL CG_enable_thr = FALSE;
BOOL CG_cond_defs_allowed = FALSE;
#endif

BOOL CG_enable_reverse_if_conversion = TRUE;
BOOL CG_enable_reverse_if_conversion_overridden = FALSE;

INT32 CG_maxinss = CG_maxinss_default;
INT32 CG_maxblocks = 30;

// ====================================================================
//   EBO:
// ====================================================================

BOOL CG_create_madds = TRUE;

// ====================================================================
//   CFLOW:
// ====================================================================

BOOL CFLOW_Enable = TRUE;
BOOL CFLOW_opt_before_cgprep = TRUE;
BOOL CFLOW_opt_after_cgprep = TRUE;
BOOL CFLOW_opt_before_igls = FALSE;
BOOL CFLOW_Enable_Unreachable = TRUE;
BOOL CFLOW_Enable_Branch = TRUE;
BOOL CFLOW_Enable_Merge = TRUE;
BOOL CFLOW_Enable_Reorder = FALSE;
BOOL CFLOW_Enable_Freq_Order = TRUE;
BOOL CFLOW_Enable_Clone = TRUE;
BOOL CFLOW_opt_all_br_to_bcond = FALSE;
const char *CFLOW_heuristic_tolerance;
const char *CFLOW_feedback_tolerance;
UINT32 CFLOW_clone_incr=10;
UINT32 CFLOW_clone_min_incr = 15;
UINT32 CFLOW_clone_max_incr = 100;
const char *CFLOW_cold_threshold;
#ifdef TARG_ST
// [CL]
BOOL CFLOW_Enable_Favor_Branches_Condition = FALSE;
// [TB]
BOOL CFLOW_Space = FALSE;
BOOL CFLOW_depgraph_use = TRUE;
BOOL CFLOW_enable_last_pass = FALSE;
BOOL CFLOW_Enable_Hoist_rts = TRUE;
#endif

// ====================================================================
//   CGSPILL:
// ====================================================================

BOOL CGSPILL_Rematerialize_Constants = TRUE;
BOOL CGSPILL_Enable_Force_Rematerialization = FALSE;

// ====================================================================
//   GCM:
// ====================================================================

// TODO: set the default value here and ovewrite in target-specific
//       initialization part.
#ifdef TARG_IA64
BOOL GCM_Speculative_Loads = FALSE;
BOOL GCM_Predicated_Loads = TRUE;
#else
BOOL GCM_Speculative_Loads = FALSE;
BOOL GCM_Predicated_Loads = FALSE;
#endif
BOOL GCM_Motion_Across_Calls = TRUE;
BOOL GCM_Min_Reg_Usage = TRUE;
BOOL GCM_Pointer_Spec= TRUE;
BOOL GCM_Eager_Ptr_Deref = TRUE;
BOOL GCM_Test = FALSE;
BOOL GCM_Enable_Cflow = TRUE;
BOOL GCM_PRE_Enable_Scheduling = TRUE;
BOOL GCM_POST_Enable_Scheduling = TRUE;
BOOL GCM_Enable_Scheduling = TRUE;
BOOL GCM_Enable_Fill_Delay_Slots = TRUE;

// ====================================================================
//   LOCS:
// ====================================================================

BOOL Enable_Fill_Delay_Slots = TRUE;
#if defined(TARG_IA64)
BOOL LOCS_Enable_Bundle_Formation = TRUE;
#else
BOOL LOCS_Enable_Bundle_Formation = FALSE;
#endif
BOOL LOCS_PRE_Enable_Scheduling = TRUE;
BOOL LOCS_POST_Enable_Scheduling = TRUE;
BOOL LOCS_Enable_Scheduling = TRUE;
#ifdef TARG_ST
INT32 LOCS_POST_Scheduling =  Forward_Post_Sched;
#endif

// ====================================================================
//   IGLS:
// ====================================================================

BOOL IGLS_Enable_HB_Scheduling = TRUE;
BOOL IGLS_Enable_PRE_HB_Scheduling = TRUE;
BOOL IGLS_Enable_POST_HB_Scheduling = TRUE;
BOOL IGLS_Enable_All_Scheduling = TRUE;

// ====================================================================
//   GRA_LIVE:
// ====================================================================

BOOL GRA_LIVE_Phase_Invoked = FALSE;
BOOL GRA_LIVE_Predicate_Aware = FALSE;

// ====================================================================
//   LRA:
// ====================================================================

BOOL LRA_do_reorder = FALSE;
#ifdef TARG_ST
BOOL LRA_minregs    = FALSE;
BOOL LRA_merge_extract = TRUE;
BOOL LRA_resched_check = FALSE;
BOOL LRA_overlap_coalescing = TRUE;
BOOL LRA_no_uninit_strict_check = FALSE;
#endif

// ====================================================================
//   GRA:
// ====================================================================

BOOL GRA_use_old_conflict = FALSE;
BOOL GRA_shrink_wrap      = TRUE;
BOOL GRA_loop_splitting   = TRUE;
BOOL GRA_home             = TRUE;
BOOL GRA_remove_spills    = TRUE;
BOOL GRA_preference_globals = TRUE;
BOOL GRA_preference_dedicated = TRUE;
BOOL GRA_preference_glue = TRUE;
BOOL GRA_preference_all = TRUE;
BOOL GRA_ensure_spill_proximity = TRUE;
BOOL GRA_choose_best_split = TRUE;
BOOL GRA_use_stacked_regs = TRUE;
BOOL GRA_redo_liveness = FALSE;
BOOL GRA_recalc_liveness = FALSE;
#ifdef TARG_ST
BOOL GRA_use_runeson_nystrom_spill_metric = FALSE;
BOOL GRA_use_interprocedural_info = TRUE;
BOOL GRA_spill_to_caller_save = TRUE;
BOOL GRA_preference_subclass = TRUE;
BOOL GRA_use_subclass_register_request = TRUE;
const char *GRA_local_spill_multiplier_string = "1.0";
//TB: export variables
BOOL GRA_spill_count_factor_for_size = FALSE;
BOOL GRA_spill_count_factor_for_size_set = FALSE;
BOOL GRA_split_for_size = FALSE;
BOOL GRA_split_for_size_set = FALSE;
BOOL GRA_spill_count_min = FALSE;
BOOL GRA_spill_count_min_set = FALSE;
BOOL GRA_overlay_spills = TRUE;
#endif
INT32 GRA_non_home_hi = -1;
INT32 GRA_non_home_lo = INT32_MAX;
const char* GRA_call_split_freq_string = "0.1";
const char* GRA_spill_count_factor_string = "0.5";

// ====================================================================
//   HB:
// ====================================================================

BOOL  HB_formation = FALSE;    // not good for all targets yet
BOOL  HB_static_freq_heuristics = TRUE;
const char* HB_max_sched_growth = "4.1";
const char* HB_min_path_priority_ratio = "0.05";
const char* HB_min_priority = "0.002";
const char* HB_call_hazard_multiplier = "0.25";
const char* HB_memory_hazard_multiplier = "1.0";
const char* HB_base_probability_contribution = "0.1";
BOOL  HB_require_alias = TRUE;
BOOL  HB_loops = FALSE;
BOOL  HB_loops_with_exits = FALSE;
BOOL  HB_complex_non_loop = FALSE;
BOOL  HB_general_use_pq = FALSE;
BOOL  HB_general_from_top = FALSE;
BOOL  HB_allow_tail_duplication = FALSE;
BOOL  HB_exclude_calls = FALSE;
BOOL  HB_exclude_pgtns = TRUE;	// until bugs fixed
BOOL  HB_skip_hammocks = TRUE;	// until bugs fixed
BOOL  HB_simple_ifc = FALSE;
BOOL  HB_simple_ifc_set = FALSE;
BOOL  HB_superblocks = FALSE;

INT   HB_max_blocks = 20;
INT   HB_min_blocks = 2;

BOOL EMIT_pjump_all = TRUE;
BOOL EMIT_use_cold_section = TRUE;
BOOL EMIT_interface_section = TRUE;
BOOL EMIT_stop_bits_for_asm = TRUE;
BOOL EMIT_stop_bits_for_volatile_asm = TRUE;
BOOL EMIT_explicit_bundles = FALSE;
INT32 EMIT_Long_Branch_Limit = DEFAULT_LONG_BRANCH_LIMIT;
#ifdef TARG_ST
BOOL EMIT_space = FALSE;
#endif

BOOL CG_emit_asm_dwarf    = FALSE;
#ifdef TARG_ST // [CL] do not emit unwind info by default
BOOL CG_emit_unwind_info  = FALSE;
#else
BOOL CG_emit_unwind_info  = TRUE;
#endif
BOOL CG_emit_unwind_directives = FALSE;

#ifdef TARG_STxP70
INT32 CG_itstackalign_val = 8;
BOOL  CG_itstackalign_val_overridden = FALSE;
#endif

// ====================================================================
//   CGEXP:
// ====================================================================

INT32 CGEXP_expandconstant = DEFAULT_CGEXP_CONSTANT;
#ifdef TARG_ST
BOOL CGEXP_expandconstant_set = FALSE;
#endif
BOOL CGEXP_use_copyfcc = TRUE;
BOOL CGEXP_normalize_logical = FALSE;
BOOL CGEXP_gp_prolog_call_shared = TRUE;
BOOL CGEXP_fast_imul = TRUE;
BOOL CGEXP_float_consts_from_ints = TRUE;
BOOL CGEXP_cvrt_int_div_to_mult = TRUE;
BOOL CGEXP_cvrt_int_div_to_fdiv = TRUE;
BOOL CGEXP_opt_float_div_by_const = TRUE;

// temporary flags for controlling algorithm selection for fdiv, sqrt, etc
const char *CGEXP_fdiv_algorithm = "sgi";
const char *CGEXP_sqrt_algorithm = "sgi";

// ====================================================================
//   CG_LOOP:
// ====================================================================

#ifndef TARG_ST
// FdF 20090318: Moved into config.cxx
/* Recurrence Breaking flags */
#ifdef MIPS
// Disable fix recurrence because CG_DEF_Op_Opnd_Changed
//   is no longer supported for MIPS.  
//
BOOL CG_LOOP_fix_recurrences = FALSE;
#else
BOOL CG_LOOP_fix_recurrences = TRUE;
#endif

BOOL CG_LOOP_fix_recurrences_specified = FALSE;
#endif
BOOL CG_LOOP_back_substitution = TRUE;
BOOL CG_LOOP_back_substitution_specified = FALSE;
BOOL CG_LOOP_back_substitution_variant = TRUE;
BOOL CG_LOOP_back_substitution_variant_specified = FALSE;
BOOL CG_LOOP_interleave_reductions = TRUE;
BOOL CG_LOOP_interleave_reductions_specified = FALSE;
BOOL CG_LOOP_interleave_posti = TRUE;
BOOL CG_LOOP_interleave_posti_specified = FALSE;
BOOL CG_LOOP_reassociate = TRUE;
BOOL CG_LOOP_reassociate_specified = FALSE;
INT32 CG_LOOP_recurrence_min_omega = 0;

// ====================================================================
//   CGTARG:
// ====================================================================

const char *CGTARG_Branch_Taken_Prob = NULL;
double CGTARG_Branch_Taken_Probability;
BOOL CGTARG_Branch_Taken_Prob_overridden;

#ifdef TARG_ST
// ====================================================================
//   TAILMERGE:
// ====================================================================
INT CG_tailmerge = 0;
INT CG_simp_flow_in_tailmerge = 0;

// ====================================================================
//   GTN COALESCER:
// ====================================================================
INT32 CG_coalesce = (COALESCE_BEFORE_SCHED | COALESCE_AFTER_SCHED);
BOOL CG_coalesce_overridden = FALSE;
BOOL CG_coalesce_pair_only = FALSE;
INT32 CG_coalesce_max_transfo = INT32_MAX;
#endif