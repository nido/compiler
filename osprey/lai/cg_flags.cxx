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
 *  $Revision$
 *  $Date$
 *  $Author$
 *  $Source$
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

BOOL FREQ_enable = TRUE;
BOOL FREQ_view_cfg = FALSE;
const char *FREQ_frequent_never_ratio = "1000.0";
const char *FREQ_eh_freq = "0.1";

BOOL CG_enable_rename = TRUE;
BOOL CG_enable_prefetch = FALSE;
BOOL CG_enable_z_conf_prefetch  = FALSE;
BOOL CG_enable_nz_conf_prefetch = FALSE;
BOOL CG_enable_pf_L1_ld = FALSE;
BOOL CG_enable_pf_L1_st = FALSE;
BOOL CG_enable_pf_L2_ld = FALSE;
BOOL CG_enable_pf_L2_st = FALSE;
BOOL CG_exclusive_prefetch = FALSE;

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
BOOL CG_enable_loop_optimizations = TRUE;
BOOL CG_enable_feedback = FALSE;
BOOL CG_tail_call = FALSE;
BOOL CG_enable_reverse_if_conversion = TRUE;
BOOL CG_enable_reverse_if_conversion_overridden = FALSE;

INT32 CG_maxinss = CG_maxinss_default;
INT32 CG_maxblocks = 30;

BOOL CFLOW_Enable = FALSE;
BOOL CFLOW_opt_before_cgprep = FALSE;
BOOL CFLOW_opt_after_cgprep = FALSE;
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

BOOL CGSPILL_Rematerialize_Constants = TRUE;

BOOL LOCS_Enable_Scheduling = TRUE;

BOOL GRA_redo_liveness = FALSE;
BOOL GRA_recalc_liveness = FALSE;

BOOL  HB_formation = TRUE;
BOOL  HB_static_freq_heuristics = TRUE;
INT   HB_max_blocks = 20;
const char* HB_max_sched_growth = "4.1";
const char* HB_min_path_priority_ratio = "0.05";
const char* HB_min_priority = "0.002";
const char* HB_call_hazard_multiplier = "0.25";
const char* HB_memory_hazard_multiplier = "1.0";
const char* HB_base_probability_contribution = "0.1";
BOOL  HB_require_alias = TRUE;
BOOL  HB_loops = FALSE;
BOOL  HB_general_use_pq = FALSE;
BOOL  HB_general_from_top = FALSE;
BOOL  HB_allow_tail_duplication = FALSE;
BOOL  HB_exclude_calls = FALSE;
BOOL  HB_exclude_pgtns = TRUE;	// until bugs fixed
BOOL  HB_skip_hammocks = TRUE;	// until bugs fixed
BOOL  HB_simple_ifc = TRUE;
BOOL  HB_simple_ifc_set = FALSE;
INT   HB_min_blocks = 2;

BOOL GRA_LIVE_Phase_Invoked = FALSE;
BOOL  GRA_LIVE_Predicate_Aware = FALSE;

BOOL EXP_gp_prolog_call_shared = TRUE;
BOOL EXP_normalize_logical = TRUE;
const char *EXP_sqrt_algorithm = "st100";

BOOL GCM_Motion_Across_Calls = TRUE;

BOOL LAI_emit_asm_dwarf    = FALSE;
BOOL LAI_emit_unwind_info  = TRUE;
BOOL LAI_emit_unwind_directives = FALSE;
BOOL LAI_emit_use_cold_section = FALSE;

// .lai does not need to have the stack layout be emitted.
// when generating executable code, we must do it.
BOOL LAI_emit_stack_frame = TRUE;
