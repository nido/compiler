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
 *  Module: cg_flags.h
 *
 *  Description:
 *  ============
 *
 *  Exports variables global to all of CG.
 *
 *  Exported variables:
 *  ==================-
 *
 *  BOOL LAI_tail_call
 *	Enable tail call generation.
 *
 *  BOOL LAI_unique_exit
 *	Generate unique exit blocks.
 *
 *  BOOL LAI_warn_bad_freqs
 *	Whenever a phase notices that freq related data is wrong or
 *      inconsistent, it can warn when this flag is true.
 * 
*/
#ifdef BCO_ENABLED /* Thierry */
/*
 *  BOOL CG_emit_bb_freqs
 *	Emit Basic Block frequencies in .profile_info sextion
 *
 */
/*
 *  BOOL CG_emit_bb_freqs_arcs
 *	Emit Basic Block frequencies + arc info in .profile_info_arc sextion
 *
 */
#endif /* BCO_Enabled Thierry */
 /*
 *  BOOL CG_enable_loop_optimizations
 *	Enable the innermost loop optimizations (CGPREP). This includes
 *	loop unrolling, r/w elimination, recurrence breaking, SWP.
 *
 *  INT32 CG_skip_after
 *  INT32 CG_skip_before
 *  INT32 CG_skip_equal
 *	Controls which PUs we skip optimizing, i.e. we set CG_opt_level=0
 *	for.
 *
 *  INT32 CG_local_skip_after
 *  INT32 CG_local_skip_before
 *  INT32 CG_local_skip_equal
 *	Options to control the optimization done by any phase in CG. The
 *    	interpretation of the numbers to skip is left totally to the 
 *	local phase. For example, the local scheduler uses these options
 *	to control which basic blocks to skip scheduling for.
 *
 *  BOOL CG_skip_local_sched
 *	Enable skipping of scheduling of basic blocks based on the 
 *	-CG:skip_local_[after,before,equal] options.
 *
 *  BOOL CG_skip_local_swp
 *	Enable skipping of pipelining of inner loops based on the 
 *	-CG:skip_local_[after,before,equal] options.
 *
 *  INT CG_opt_level
 *      Gives the optimization level to use throughout CG.
 *
 *  BOOL CG_localize_tns
 *      Make sure we have no global TNs (and thus don't need GRA).
 *  
 *  CG_cond_defs_allowed
 *	Allow generation of conditional definitions (predicated insts)
 *
 *  BOOL CG_enable_reverse_if_conversion
 *  BOOL CG_enable_reverse_if_conversion_overridden
 *      Enable reverse if conversion.
 *  
 *  BOOL CG_enable_thr
 *      Enable tree-height reduction phase in CG.
 * 
 *  BOOL CG_enable_spec_imul
 *      allow loops with speculated integer mul 
 *      to be if converted.
 * 
 *  BOOL CG_enable_spec_idiv
 *      allow loops with speculated integer div 
 *      to be if converted.
 * 
 *  BOOL CG_enable_spec_fdiv
 *      allow loops with speculated fdiv 
 *      to be if converted. recips are also included
 *      with this flag.
 *
 *  BOOL CG_enable_spec_fsqrt
 *      allow loops with speculated fsqrt 
 *      to be if converted.
 *
 *  INT32 CG_maxinss
 *      Maximum number of ops in a loop to be if converted.
 *
 *  INT32 CG_maxblocks
 *      Maximum number of bbs in a loop to be if converted.
 *
 *  BOOL FREQ_enable
 *	Enable BB freqency estimates.
 *
 *  BOOL FREQ_view_cfg
 *	Indicates if daVanci should be invoked to view the frequency
 *	annotated CFG at appropriate times.
 *
 *  const char *FREQ_frequent_never_ratio
 *	The ratio in the probabilities of "frequent" to "never" successors
 *	tagged with frequency hint pragmas.
 *
 *  const char *FREQ_eh_freq
 *	The frequency (relative to the entry point) that an exception
 *	handler is executed.
 *
 *  EMIT_pjump_all
 *	Generate a PJUMP relocation for all calls, not just the 
 *	non-preemptible ones. This allows ld to change jalr to jal
 *	for calls that it can determine are non-preemptible, even
 *	though CG cannot tell. This change can be enabled by default
 *	only when the corresponding ld change is done. Defer making
 *	it default till 7.3 to prevent problems with 7.2 objects
 *	being linked with an earlier ld (default false)
 *
 *  EMIT_use_cold_section
 *	Put code region BBs into .text.cold. Turn off for debugging...
 *	(default true)
 *
 *  EMIT_interface_section
 *	Emit interface section (default true).
 *
 * =======================================================================
 * =======================================================================
 */

#ifndef cg_flags_INCLUDED
#define cg_flags_INCLUDED


extern BOOL CG_warn_bad_freqs;
#ifdef BCO_ENABLED /* Thierry */
extern BOOL CG_emit_bb_freqs;
extern BOOL CG_emit_bb_freqs_arcs;
#endif /* BCO_Enabled Thierry */

extern INT32 CG_skip_after;
extern INT32 CG_skip_before;
extern INT32 CG_skip_equal;
extern INT32 CG_local_skip_after;
extern INT32 CG_local_skip_before;
extern INT32 CG_local_skip_equal;
extern BOOL CG_skip_local_hbf;
extern BOOL CG_skip_local_loop;
extern BOOL CG_skip_local_sched;
extern BOOL CG_skip_local_swp;
extern INT CG_opt_level;
extern BOOL CG_localize_tns;
extern BOOL CG_localize_tns_Set;
extern BOOL LOCALIZE_using_stacked_regs;

extern BOOL CG_gen_callee_saved_regs_mask; /* generate register mask */

#ifdef TARG_ST
// [CL] force spill of return address (RA) so that unwinding/backtracing is still possible
extern BOOL CG_save_return_address;
#endif
extern BOOL CG_tail_call;
extern BOOL CG_unique_exit;
extern BOOL CG_cond_defs_allowed;
extern BOOL CG_enable_feedback;
extern BOOL CG_enable_loop_optimizations;
extern BOOL CG_enable_rename;
extern BOOL CG_enable_reverse_if_conversion;
extern BOOL CG_enable_reverse_if_conversion_overridden;
extern BOOL CG_enable_thr;
extern BOOL CG_enable_peephole;
extern BOOL CG_enable_ssa;	/* Enable SSA in cg */
extern BOOL CG_enable_select;
#ifdef TARG_ST
extern BOOL CG_enable_range_propagation;
// In range analysis, ee will lower values at most this number of times.
// After this, they decay to Bottom.
extern INT32 CG_range_recompute_limit;
extern BOOL CG_enable_rename_after_GRA;
#endif
CG_EXPORTED extern INT32 CG_LAO_optimizations;	/* LAO optimizations enable */
CG_EXPORTED extern INT32 CG_LAO_regiontype;	/* LAO scheduling region type */
CG_EXPORTED extern INT32 CG_LAO_conversion;	/* LAO SSA construction and destruction */
CG_EXPORTED extern INT32 CG_LAO_predication;	/* LAO predication algorithm */
CG_EXPORTED extern INT32 CG_LAO_scheduling;	/* LAO scheduling algorithm */
CG_EXPORTED extern INT32 CG_LAO_allocation;	/* LAO allocation algorithm */
CG_EXPORTED extern INT32 CG_LAO_formulation;	/* LAO integer formulation flags */
CG_EXPORTED extern INT32 CG_LAO_preloading;	/* LAO memory preloading level */
CG_EXPORTED extern INT32 CG_LAO_l1missextra;	/* LAO extra latency for preloading */
CG_EXPORTED extern INT32 CG_LAO_compensation;	/* LAO compensation level */
CG_EXPORTED extern INT32 CG_LAO_speculation;	/* LAO speculation level */
CG_EXPORTED extern INT32 CG_LAO_relaxation;	/* LAO relaxation level */
CG_EXPORTED extern INT32 CG_LAO_pipelining;	/* LAO software pipelining level */
CG_EXPORTED extern INT32 CG_LAO_renaming;	/* LAO register renaming level */
CG_EXPORTED extern INT32 CG_LAO_boosting;	/* LAO operation boosting level */
CG_EXPORTED extern INT32 CG_LAO_aliasing;	/* LAO memory aliasing level */
CG_EXPORTED extern INT32 CG_LAO_prepadding;	/* LAO data pre-padding in bytes */
CG_EXPORTED extern INT32 CG_LAO_postpadding;	/* LAO data post-padding in bytes */
CG_EXPORTED extern INT32 CG_LAO_overrun;	/* LAO pipeline overrun */

#ifdef CGG_ENABLED
extern BOOL CG_enable_cgg;	/* Enable whirl2ops based on CGG */
extern INT32 CG_cgg_level;
#endif

extern BOOL CG_enable_BB_splitting;
#define CG_bblength_default 300      /* default value for CG_split_BB_length */
#define CG_bblength_min	 100	     /* don't let the value get too small */
#define CG_bblength_max	5000	     /* don't let the value get too big */
extern INT32 CG_split_BB_length;     /* split BBs that are > than this */

#define CG_maxinss_default 100
extern INT32 CG_maxinss;
extern INT32 CG_maxblocks;

/* EBO: */
extern BOOL CG_create_madds;

/* CFLOW: */
extern BOOL CFLOW_Enable;
extern BOOL CFLOW_opt_before_cgprep;
extern BOOL CFLOW_opt_after_cgprep;
extern BOOL CFLOW_opt_before_igls;
extern BOOL CFLOW_Enable_Unreachable;
extern BOOL CFLOW_Enable_Branch;
extern BOOL CFLOW_Enable_Merge;
extern BOOL CFLOW_Enable_Reorder;
extern BOOL CFLOW_Enable_Freq_Order;
extern BOOL CFLOW_Enable_Clone;
extern BOOL CFLOW_opt_all_br_to_bcond;
extern const char *CFLOW_heuristic_tolerance;
extern const char *CFLOW_feedback_tolerance;
extern UINT32 CFLOW_clone_incr;
extern UINT32 CFLOW_clone_max_incr;
extern UINT32 CFLOW_clone_min_incr;
extern const char *CFLOW_cold_threshold;
#ifdef TARG_ST
extern BOOL CFLOW_Enable_Favor_Branches_Condition;
#endif

/* FREQ: */
extern BOOL FREQ_enable;
extern BOOL FREQ_view_cfg;
extern const char *FREQ_frequent_never_ratio;
extern const char *FREQ_eh_freq;

/* Prefetch and load latency */

extern BOOL CG_enable_prefetch;
extern BOOL CG_enable_z_conf_prefetch;
extern BOOL CG_enable_nz_conf_prefetch;
extern BOOL CG_enable_pf_L1_ld;
extern BOOL CG_enable_pf_L1_st;
extern BOOL CG_enable_pf_L2_ld;
extern BOOL CG_enable_pf_L2_st;
extern BOOL CG_exclusive_prefetch;
#ifdef TARG_ST
// FdF 20070206: Raise a warning when prefetch distance of user
// prefetch cannot be checked against the Prefetch_Padding value.
extern BOOL  CG_warn_prefetch_padding;
#endif

extern INT32 CG_L1_ld_latency;
extern INT32 CG_L2_ld_latency;
extern INT32 CG_z_conf_L1_ld_latency;
extern INT32 CG_z_conf_L2_ld_latency;
extern INT32 CG_ld_latency;
extern INT32 CG_L1_pf_latency;
extern INT32 CG_L2_pf_latency;

extern INT32 CG_branch_mispredict_penalty;
extern INT32 CG_branch_mispredict_factor;

/* CGSPILL: */
extern BOOL CGSPILL_Rematerialize_Constants;
extern BOOL CGSPILL_Enable_Force_Rematerialization;

/* GCM, LOCS and IGLS */

extern BOOL GCM_Enable_Scheduling;
extern BOOL GCM_PRE_Enable_Scheduling;
extern BOOL GCM_POST_Enable_Scheduling;
extern BOOL GCM_Motion_Across_Calls;
extern BOOL GCM_Min_Reg_Usage;
extern BOOL GCM_Pointer_Spec;
extern BOOL GCM_Eager_Ptr_Deref;
extern BOOL GCM_Speculative_Loads;
extern BOOL GCM_Predicated_Loads;
extern BOOL GCM_Test;
extern BOOL GCM_Enable_Fill_Delay_Slots;
extern BOOL GCM_Enable_Cflow;

extern BOOL Enable_Fill_Delay_Slots;
extern BOOL LOCS_PRE_Enable_Scheduling;
extern BOOL LOCS_POST_Enable_Scheduling;
extern BOOL LOCS_Enable_Bundle_Formation;
extern BOOL LOCS_Enable_Scheduling;
#ifdef TARG_ST
#define Backward_Post_Sched 1
#define Forward_Post_Sched 2
#define Double_Post_Sched 3
extern INT32 LOCS_POST_Scheduling;
#endif 

extern BOOL IGLS_Enable_PRE_HB_Scheduling;
extern BOOL IGLS_Enable_POST_HB_Scheduling;
extern BOOL IGLS_Enable_HB_Scheduling;
extern BOOL IGLS_Enable_All_Scheduling;

/* LRA: */
extern BOOL LRA_do_reorder;
#ifdef TARG_ST
extern BOOL LRA_minregs;
extern BOOL LRA_merge_extract;
extern BOOL LRA_resched_check;
#endif

/* GRA: */
extern BOOL GRA_use_old_conflict;
extern BOOL GRA_shrink_wrap;
extern BOOL GRA_loop_splitting;
extern BOOL GRA_home;
extern BOOL GRA_remove_spills;
extern BOOL GRA_preference_globals;
extern BOOL GRA_preference_dedicated;
extern BOOL GRA_preference_glue;
extern BOOL GRA_preference_all;
extern BOOL GRA_ensure_spill_proximity;
extern BOOL GRA_choose_best_split;
extern BOOL GRA_use_stacked_regs;
extern BOOL GRA_redo_liveness;
extern BOOL GRA_recalc_liveness;
#ifdef TARG_ST
extern BOOL GRA_use_runeson_nystrom_spill_metric;
extern BOOL GRA_use_interprocedural_info;
extern BOOL GRA_spill_to_caller_save;
extern BOOL GRA_preference_subclass;
extern BOOL GRA_use_subclass_register_request;
extern const char* GRA_local_spill_multiplier_string;
extern BOOL GRA_overlay_spills;
#endif
extern INT32 GRA_non_home_hi;
extern INT32 GRA_non_home_lo;
extern const char* GRA_call_split_freq_string;
extern const char* GRA_spill_count_factor_string;

/* HB: */
extern BOOL  HB_formation;
extern BOOL  HB_static_freq_heuristics;
extern const char* HB_call_hazard_multiplier;
extern const char* HB_memory_hazard_multiplier;
extern const char* HB_min_path_priority_ratio;
extern const char* HB_base_probability_contribution;
extern const char* HB_min_priority;
extern const char* HB_max_sched_growth;
extern BOOL  HB_require_alias;
extern BOOL  HB_loops;
extern BOOL  HB_loops_with_exits;
extern BOOL  HB_complex_non_loop;
extern BOOL  HB_simple_ifc;
extern BOOL  HB_simple_ifc_set;
extern BOOL  HB_general_use_pq;
extern BOOL  HB_general_from_top;
extern BOOL  HB_allow_tail_duplication;
extern BOOL  HB_exclude_calls;
extern BOOL  HB_exclude_pgtns;
extern BOOL  HB_skip_hammocks;
extern BOOL  HB_superblocks;

extern INT   HB_max_blocks;
extern INT   HB_min_blocks;

/* GRA_LIVE: */
extern BOOL GRA_LIVE_Phase_Invoked;
extern BOOL  GRA_LIVE_Predicate_Aware;

/* EMIT: */
extern BOOL EMIT_pjump_all;
extern BOOL EMIT_use_cold_section;
extern BOOL EMIT_interface_section;

extern INT32 EMIT_Long_Branch_Limit;    /* max distance (in bytes) for branches */
extern BOOL EMIT_stop_bits_for_asm;
extern BOOL EMIT_stop_bits_for_volatile_asm;
extern BOOL EMIT_explicit_bundles;

extern BOOL CG_emit_asm_dwarf;
extern BOOL CG_emit_unwind_info;
extern BOOL CG_emit_unwind_directives;

#ifdef TARG_ST
/* CBPO */
extern BOOL CG_enable_cbpo;
extern BOOL CG_cbpo_optimize_load_imm;
extern INT32 CG_cbpo_ratio;
extern INT CG_cbpo_block_method;
enum CBPO_BLOCK_METHOD
    {
        CBPO_BLOCK_NONE = 0,
        CBPO_BLOCK_LOCAL = 1,
        CBPO_BLOCK_GLOBAL = 2,
        CBPO_BLOCK_GLOBAL_THEN_LOCAL = 3
    };

extern BOOL CG_cbpo_facto_cst;
extern BOOL CG_cbpo_optimize_load_imm_cst;
#endif

/*
 * CGEXPand flags 
 */
extern INT32 CGEXP_expandconstant;	/* maximum # instructions to expand constants */
#define DEFAULT_CGEXP_CONSTANT	3

extern BOOL CGEXP_use_copyfcc;
extern BOOL CGEXP_normalize_logical;
extern BOOL CGEXP_gp_prolog_call_shared;
extern BOOL CGEXP_fast_imul;
extern BOOL CGEXP_float_consts_from_ints;
extern BOOL CGEXP_cvrt_int_div_to_mult;
extern BOOL CGEXP_cvrt_int_div_to_fdiv;
extern BOOL CGEXP_opt_float_div_by_const;

// temporary flags for controlling algorithm selection for fdiv, sqrt, etc
extern const char *CGEXP_fdiv_algorithm;
extern const char *CGEXP_sqrt_algorithm;

// ====================================================================
//   CG_LOOP:
// ====================================================================

/* Recurrence breaking flags */
extern BOOL CG_LOOP_fix_recurrences;
extern BOOL CG_LOOP_fix_recurrences_specified;
extern BOOL CG_LOOP_back_substitution;
extern BOOL CG_LOOP_back_substitution_specified;
extern BOOL CG_LOOP_back_substitution_variant;
extern BOOL CG_LOOP_back_substitution_variant_specified;
extern BOOL CG_LOOP_interleave_reductions;
extern BOOL CG_LOOP_interleave_reductions_specified;
extern BOOL CG_LOOP_interleave_posti;
extern BOOL CG_LOOP_interleave_posti_specified;
extern BOOL CG_LOOP_reassociate;
extern BOOL CG_LOOP_reassociate_specified;
extern INT32 CG_LOOP_recurrence_min_omega;

/*
 * CGTARG flags 
 */
extern const char *CGTARG_Branch_Taken_Prob;
extern double CGTARG_Branch_Taken_Probability;
extern BOOL CGTARG_Branch_Taken_Prob_overridden;

/* =======================================================================
 *   Miscellaneous
 * =======================================================================
 */
extern BOOL Use_Page_Zero;  /* set bit in object to allow use of page 0 */

/* =======================================================================
 *   Tracing Flags
 * =======================================================================
 */

/* ====================================================================
 *   TAILMERGE:
 * ====================================================================
 */
#ifdef TARG_ST
extern INT CG_tailmerge;
extern INT CG_simp_flow_in_tailmerge;
#endif

#endif /* cg_flags_INCLUDED */



