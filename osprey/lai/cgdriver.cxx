/*

  Copyright (C) 2001 STMicroelectronics, Inc.  All Rights Reserved.

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

  Contact information:  MDT/STMicroelectronics, Inc., Montbonnot,

  or:

  http://www.st.com

  For further information regarding this notice, see:

  http://oss.sgi.com/projects/GenInfo/NoticeExplan

*/


/* ====================================================================
 * ====================================================================
 *
 * Module: cgdriver.c
 * $Source$
 *
 * Description:
 *
 * Main driver -- command line processing and file name manipulation --
 * for the code generator.
 *
 * ====================================================================
 * ====================================================================
 */

#include <elf.h>
#include <sys/elf_whirl.h>	    /* for WHIRL_REVISION */
#include <ctype.h>
#include "defs.h"
#include "dso.h"		    /* for load_so() */
#include "config.h"
#include "config_debug.h"
#include "config_list.h"
#include "config_TARG.h"
#include "config_opt.h"
#include "controls.h"
#include "flags.h"
#include "erglob.h"
#include "erlib.h"
#include "errors.h"
#include "file_util.h"
#include "glob.h"
#include "timing.h"
#include "tracing.h"
#include "util.h"
#include "mempool.h"

#include "wn.h"			    /* for WN */
#include "opt_alias_interface.h"    /* for ALIAS_MANAGER stuff */
#include "dwarf_DST_mem.h"

#include "bb.h"			    /* for cgemit.h */
#include "cg.h"			    /* CG_Initialize(), etc. */
#include "cgemit.h"		    /* R_Assemble_File() */
#include "cg_swp_options.h"         /* for SWP_Options */
#include "gra.h"                    /* for GRA_optimize_placement... */
#include "ebo.h"        	    /* for EBO options */
#include "cgprep.h"		    /* for CGPREP knobs */
#include "cg_dep_graph.h"	    /* for CG_DEP knobs */
#include "cg_dep_graph_update.h"    /* more CG_DEP knobs */
/* #include "cio.h"      */              /* for rw, cicse etc ...*/
#include "cg_loop.h"                /* for unrolling */
#include "cg_loop_recur.h"	    /* recurrence fixing */
#include "cgtarget.h"		    /* target-dependent stuff */
#include "gcm.h"		    /* for GCM options */
#include "cg_sched_est.h"           /* for CG_SCHED_EST options */
/* #include "targ_proc_properties.h" */
/* #include "cgdriver_arch.h" */
#include "cgdriver.h"
#include "register.h"
#include "pqs_cg.h"

#ifdef TARG_ST
#include "lao_init.h"
#endif

#ifdef __CYGWIN__
#include "W_errno.h"
#endif

extern void Set_File_In_Printsrc(char *); /* defined in printsrc.c */
extern void Configure_CG_Target(void);    /* defined in cg_flags_arch.cxx */

extern char *WHIRL_File_Name;

/* ====================================================================
 *
 *   Back end process-specific global data from glob.h.
 *
 * ====================================================================
 */

/* Output requested: */
BOOL Assembly =	FALSE;		/* Assembly code */
BOOL Lai_Code = FALSE;          /* Lai code */
BOOL Object_Code = FALSE;	/* Object code */

/* Have	the OP_REGCOPY operations been translated? */
BOOL Regcopies_Translated = FALSE;


/* ====================================================================
 *
 *   Local data.
 *
 * ====================================================================
 */

static char *Argv0;		         /* argv[0] from main */

/* Default file	extensions: */
#define	ASM_FILE_EXTENSION ".s"	         /* Assembly code file */
#define LAI_FILE_EXTENSION ".lai"        /* LAI file */
#define	OBJ_FILE_EXTENSION ".o"	         /* Relocatable object file */
#define DSTDUMP_FILE_EXTENSION ".be.dst" /* DST dump-file extension */

/* Internal flags: */
static BOOL cg_opt_level_overridden = FALSE;

static BOOL CG_tail_call_overridden = FALSE;
static BOOL CG_enable_prefetch_overridden = FALSE;
static BOOL CG_enable_z_conf_prefetch_overridden  = FALSE;
static BOOL CG_enable_nz_conf_prefetch_overridden = FALSE;
static BOOL CG_enable_pf_L1_ld_overridden = FALSE;
static BOOL CG_enable_pf_L1_st_overridden = FALSE;
static BOOL CG_enable_pf_L2_ld_overridden = FALSE;
static BOOL CG_enable_pf_L2_st_overridden = FALSE;
static BOOL CG_L1_ld_latency_overridden;
static BOOL CG_L2_ld_latency_overridden;
static BOOL CG_L1_pf_latency_overridden;
static BOOL CG_L2_pf_latency_overridden;
static BOOL CG_maxinss_overridden = FALSE;
static BOOL Enable_CG_Peephole_overridden = FALSE;
static BOOL EBO_Opt_Level_overridden = FALSE;
static BOOL Integer_Divide_By_Constant_overridden = FALSE;
static BOOL Integer_Divide_Use_Float_overridden = FALSE;
static BOOL CG_DEP_Mem_Arc_Pruning_overridden = FALSE;
static BOOL clone_incr_overridden = FALSE;
static BOOL clone_min_incr_overridden = FALSE;
static BOOL clone_max_incr_overridden = FALSE;
static BOOL CFLOW_Enable_Clone_overridden = FALSE;

static BOOL CG_enable_ssa_overridden = FALSE;
static BOOL CG_enable_select_overridden = FALSE;

/* Keep	a copy of the command line options for assembly	output:	*/
static char *option_string;

/* Software pipelining options: */
static OPTION_DESC Options_CG_SWP[] = {

  /* General software pipelining options */
#ifdef TARG_ST
  { OVK_BOOL,	OV_INTERNAL,	FALSE, "enable", NULL,
    0, 0, 0,	&Enable_SWP, &Enable_SWP_overridden },
#else
  { OVK_BOOL,	OV_INTERNAL,	TRUE, "", NULL,
    0, 0, 0,	&Enable_SWP, &Enable_SWP_overridden },
#endif

  { OVK_INT32,	OV_INTERNAL,	TRUE, "sched_direction", "sched_dir",
    0, 0, INT32_MAX,	&SWP_Options.Sched_Direction, NULL },
  { OVK_INT32,	OV_INTERNAL,	TRUE, "heuristics", "heur",
    0, 0, INT32_MAX,	&SWP_Options.Heuristics, NULL },
  { OVK_INT32,	OV_INTERNAL,	TRUE, "opt", "opt",
    0, 0, INT32_MAX,	&SWP_Options.Opt_Level, NULL },
  { OVK_BOOL,	OV_INTERNAL,	TRUE, "while_loop", NULL,
    0, 0, 0,	&SWP_Options.Enable_While_Loop, NULL },
  { OVK_INT32,	OV_INTERNAL,	TRUE, "min_unroll_times", "min_unr",
    0, 0, INT32_MAX,	&SWP_Options.Min_Unroll_Times, &SWP_Options.Max_Unroll_Times_Set },
  { OVK_INT32,	OV_INTERNAL,	TRUE, "max_unroll_times", "max_unr",
    0, 0, INT32_MAX,	&SWP_Options.Max_Unroll_Times, &SWP_Options.Max_Unroll_Times_Set },
  { OVK_BOOL,	OV_INTERNAL,	TRUE, "bundle", NULL,
    TRUE, 0, 0,	&SWP_Options.Enable_Bundling, NULL },
  { OVK_BOOL,	OV_INTERNAL,	TRUE, "postincr", "posti",
    0, 0, 0,	&SWP_Options.Enable_Post_Incr, NULL },
  { OVK_INT32,	OV_INTERNAL,	TRUE, "start_ii", "start",
    0, 0, INT32_MAX,	&SWP_Options.Starting_II, NULL },
  { OVK_BOOL,	OV_INTERNAL,	TRUE, "workaround", "work", 
    0, 0, 0,	&SWP_Options.Enable_Workaround, NULL },
  { OVK_INT32,	OV_INTERNAL,	TRUE, "critical_threshold", "critical",
    0, 0, INT32_MAX,	&SWP_Options.Critical_Threshold, NULL },
  { OVK_BOOL,	OV_INTERNAL,	TRUE, "prep_only", "", 
    0, 0, 0,	&SWP_Options.Prep_Only, NULL },
  { OVK_BOOL,	OV_INTERNAL,	TRUE, "min_retry", "", 
    0, 0, 0,	&SWP_Options.Min_Retry, NULL },
  { OVK_BOOL,	OV_INTERNAL,	TRUE, "implicit_prefetch", "", 
    0, 0, 0,	&SWP_Options.Implicit_Prefetch, &SWP_Options.Implicit_Prefetch_Set },
  { OVK_BOOL,	OV_INTERNAL,	TRUE, "predicate_promotion", "", 
    0, 0, 0,	&SWP_Options.Predicate_Promotion, NULL },
  { OVK_BOOL,	OV_INTERNAL,	TRUE, "enable_brp", "", 
    0, 0, 0,	&SWP_Options.Enable_BRP, NULL },

  { OVK_COUNT }		/* List terminator -- must be last */
};

/* Global register allocator options */
static OPTION_DESC Options_GRA[] = {
  { OVK_BOOL,	OV_INTERNAL, TRUE,  "optimize_placement", "",
    0,0,0,      &GRA_optimize_placement, NULL,
    "Enable/disable movement of spills and restores created during splitting [Default TRUE]."
  },
  { OVK_INT32,	OV_INTERNAL, TRUE, "local_forced_max", "",
    4, 0, 32,	&GRA_local_forced_max, NULL,
    "How many locals to force allocate (out of the number requested by LRA) [Default 4]"
  },
  { OVK_BOOL,	OV_INTERNAL, TRUE,  "avoid_glue_references_for_locals", "",
    0,0,0,      &GRA_avoid_glue_references_for_locals,NULL,
    "If possible grant the forced locals from the set of registers not referenced for glue copies in the same block.  [Default TRUE]"
  },
  { OVK_BOOL,	OV_INTERNAL, TRUE, "split_entry_exit_blocks", "",
    0,0,0,	&GRA_split_entry_exit_blocks,NULL,
    "Enable/Disable splitting of entry/exit blocks for callee saved preferencing [Default TRUE]"
  },
  { OVK_BOOL,	OV_INTERNAL, TRUE,  "split_lranges", "",
    0,0,0,      &GRA_split_lranges, NULL,
    "Turn on/off splitting of live ranges [Default TRUE]"
  },
  { OVK_INT32,	OV_INTERNAL, TRUE, "non_split_tn", "",
    4, 0, INT32_MAX,	&GRA_non_split_tn_id, NULL,
    "Turn off live range splitting for a given TN specified by its tn number (n).  [Default -1]"
  },
  { OVK_INT32,	OV_INTERNAL, TRUE, "non_preference_tn", "",
    4, 0, INT32_MAX,	&GRA_non_preference_tn_id, NULL,
    "Turn off preferencing for a given TN specified by its tn number (n). [Default -1]"
  },
  { OVK_BOOL,	OV_INTERNAL, TRUE,  "use_old_conflict", "",
    0,0,0,      &GRA_use_old_conflict, NULL,
    "Use old conflict graph algorithm ... not functioning at present."
  },
  { OVK_BOOL,	OV_INTERNAL, TRUE,  "shrink_wrap", "",
    0,0,0,      &GRA_shrink_wrap, NULL,
    "Turn on/off shrink wrapping (currently, only for callee saved regs) [Default TRUE]"
  },
  { OVK_BOOL,	OV_INTERNAL, TRUE,  "loop_splitting", "",
    0,0,0,      &GRA_loop_splitting, NULL,
    "Turn on/off loop directed live range splitting [Default TRUE]",
  },
  { OVK_BOOL,	OV_INTERNAL, TRUE,  "home", "",
    0,0,0,      &GRA_home, NULL,
    "Turn on/off gra homing [Default TRUE]"
  },
  { OVK_BOOL,	OV_INTERNAL, TRUE,  "remove_spills", "",
    0,0,0,      &GRA_remove_spills, NULL,
    "Turn on/off gra removal of spill instructions in Optimize_Placment [Default TRUE]"
  },
  { OVK_BOOL,	OV_INTERNAL, TRUE,  "ensure_spill_proximity", "",
    0,0,0,      &GRA_ensure_spill_proximity, NULL,
    "Turn on/off gra placing spills close to use/def in block [Default TRUE]"
  },    
  { OVK_BOOL,	OV_INTERNAL, TRUE,  "choose_best_split", "",
    0,0,0,      &GRA_choose_best_split, NULL,
    "Turn on/off gra choosing best/smallest interim split found [Default TRUE]"
  },    
  { OVK_BOOL,	OV_INTERNAL, TRUE,  "use_stacked_regs", "",
    0,0,0,      &GRA_use_stacked_regs, NULL,
    "Turn on/off gra using stacked registers [Default TRUE]"
  },    
  { OVK_BOOL,	OV_INTERNAL, TRUE,  "redo_liveness", "",
    0,0,0,      &GRA_redo_liveness, NULL,
    "Turn on/off recalculation of liveness [Default FALSE]"
  },    
  { OVK_BOOL,	OV_INTERNAL, TRUE,  "preference_globals", "",
    0,0,0,      &GRA_preference_globals, NULL,
    "Turn on/off gra preferencing of global TNs (other than glue code) [Default TRUE]"
  },
  { OVK_BOOL,	OV_INTERNAL, TRUE,  "preference_dedicated", "",
    0,0,0,      &GRA_preference_dedicated, NULL,
    "Turn on/off gra preferencing with dedicated TNs  [Default TRUE]"
  },
  { OVK_BOOL,	OV_INTERNAL, TRUE,  "preference_glue", "",
    0,0,0,      &GRA_preference_glue, NULL,
    "Turn on/off gra preferencing in glue code [Default TRUE]"
  },
  { OVK_BOOL,	OV_INTERNAL, TRUE,  "preference_all", "",
    0,0,0,      &GRA_preference_all, NULL,
    "Turn on/off all gra preferencing [Default TRUE]"
  },
  { OVK_INT32,	OV_INTERNAL, TRUE, "non_home_low", "",
    4, 0, INT32_MAX,	&GRA_non_home_lo, NULL,
    "Turn off homing for a TN range specified by its tn numbers.  [Default INT32_MAX]"
  },
  { OVK_INT32,	OV_INTERNAL, TRUE, "non_home_hi", "",
    4, 0, INT32_MAX,	&GRA_non_home_hi, NULL,
    "Turn off homing for a TN range specified by its tn numbers.  [Default -1]"
  },
  { OVK_BOOL,	OV_INTERNAL, TRUE,  "recalc_liveness", "",
    0,0,0,      &GRA_recalc_liveness, NULL,
    "Turn on/off recomputation of global liveness info [Default FALSE]"
  },    
  { OVK_NAME,   OV_INTERNAL, TRUE,"call_split_freq", "",
    0, 0, 0,	&GRA_call_split_freq_string,
    "Threshold frequency of block containing a call below which a caller saved register will be preferred and live ranges spanning it will be split [Default .1]"
  },    
  { OVK_NAME,   OV_INTERNAL, TRUE,"spill_count_factor", "",
    0, 0, 0,	&GRA_spill_count_factor_string,
    "Factor by which count of spills affects the priority of a split.  Only valid under OPT:space [Default 0.5]"
  },    
  { OVK_COUNT }		/* List terminator -- must be last */
};

/* Generic CG options. */
static OPTION_DESC Options_CG[] = {

  // Generic CG options.

  { OVK_BOOL,	OV_INTERNAL, TRUE, "warn_bad_freqs", "",
    0, 0, 0,	&CG_warn_bad_freqs, NULL },
  { OVK_INT32,	OV_INTERNAL, TRUE, "skip_before", "skip_b",
    0, 0, INT32_MAX, &CG_skip_before, NULL }, 
  { OVK_INT32,	OV_INTERNAL, TRUE, "skip_after", "skip_a",
    0, 0, INT32_MAX, &CG_skip_after, NULL }, 
  { OVK_INT32,	OV_INTERNAL, TRUE, "skip_equal", "skip_e",
    0, 0, INT32_MAX, &CG_skip_equal, NULL }, 
  { OVK_INT32,	OV_INTERNAL, TRUE, "local_skip_before", "local_skip_b",
    0, 0, INT32_MAX, &CG_local_skip_before, NULL }, 
  { OVK_INT32,	OV_INTERNAL, TRUE, "local_skip_after", "local_skip_a",
    0, 0, INT32_MAX, &CG_local_skip_after, NULL }, 
  { OVK_INT32,	OV_INTERNAL, TRUE, "local_skip_equal", "local_skip_e",
    0, 0, INT32_MAX, &CG_local_skip_equal, NULL }, 
  { OVK_BOOL,	OV_INTERNAL, TRUE, "skip_local_hbf", "",
    0, 0, 0,	&CG_skip_local_hbf, NULL },
  { OVK_BOOL,	OV_INTERNAL, TRUE, "skip_local_loop", "",
    0, 0, 0,	&CG_skip_local_loop, NULL },
  { OVK_BOOL,	OV_INTERNAL, TRUE, "skip_local_swp", "",
    0, 0, 0,	&CG_skip_local_swp, NULL },
  /*
  { OVK_BOOL,	OV_INTERNAL, TRUE, "skip_local_ebo", "",
    0, 0, 0,	&CG_skip_local_ebo, NULL },
  */
  { OVK_BOOL,	OV_INTERNAL, TRUE, "skip_local_sched", "",
    0, 0, 0,	&CG_skip_local_sched, NULL },
  { OVK_INT32,	OV_INTERNAL, TRUE, "optimization_level", "",
    0, 0, MAX_OPT_LEVEL,
                &CG_opt_level, &cg_opt_level_overridden },

  { OVK_INT32,  OV_INTERNAL,	TRUE,	"bblength",		"bb",
    CG_bblength_default, CG_bblength_min, CG_bblength_max, &CG_split_BB_length, NULL,
    "Restrict BB length by splitting longer BBs" },

  { OVK_BOOL,	OV_INTERNAL, TRUE, "ssa_opt", "",
    0, 0, 0,	&CG_enable_ssa, &CG_enable_ssa_overridden },

  { OVK_INT32,  OV_INTERNAL,	TRUE,	"ssa_algorithm", "",
    1, 1, 3, &CG_ssa_algorithm, NULL,
    "Specify method for translating out of the SSA" },

  { OVK_BOOL,	OV_INTERNAL, TRUE, "select_if_convert", "",
    0, 0, 0,	&CG_enable_select, &CG_enable_select_overridden },

#ifdef TARG_ST
  { OVK_BOOL,	OV_INTERNAL,	FALSE, "LAO", "",
    0, 0, 0,	&CG_enable_LAO, NULL },
#endif

  // EBO options:

  { OVK_BOOL,	OV_INTERNAL, TRUE, "peephole_optimize", "",
    0, 0, 0,	&CG_enable_peephole, &Enable_CG_Peephole_overridden },

  { OVK_BOOL, 	OV_INTERNAL, TRUE, "create_madds", "create_madd",
    0, 0, 0,  &CG_create_madds, NULL },

  // CG Dependence Graph related options.

  // CGPREP options.

  { OVK_BOOL,	OV_INTERNAL, TRUE, "enable_feedback", "",
    0, 0, 0,	&CG_enable_feedback, NULL },

  // Cross Iteration Loop Optimization options.

  // Prefetching and load latency options.

  { OVK_BOOL,	OV_INTERNAL, TRUE,"prefetch", "",
    0, 0, 0, &CG_enable_prefetch, &CG_enable_prefetch_overridden },
  { OVK_BOOL,	OV_INTERNAL, TRUE,"z_conf_prefetch", "",
    0, 0, 0, &CG_enable_z_conf_prefetch,
	     &CG_enable_z_conf_prefetch_overridden },
  { OVK_BOOL,	OV_INTERNAL, TRUE,"nz_conf_prefetch", "",
    0, 0, 0, &CG_enable_nz_conf_prefetch,
	     &CG_enable_nz_conf_prefetch_overridden },
  { OVK_BOOL,	OV_INTERNAL, TRUE,"pf_L1_ld", "",
    0, 0, 0, &CG_enable_pf_L1_ld, &CG_enable_pf_L1_ld_overridden },
  { OVK_BOOL,	OV_INTERNAL, TRUE,"pf_L1_st", "",
    0, 0, 0, &CG_enable_pf_L1_st, &CG_enable_pf_L1_st_overridden },
  { OVK_BOOL,	OV_INTERNAL, TRUE,"pf_L2_ld", "",
    0, 0, 0, &CG_enable_pf_L2_ld, &CG_enable_pf_L2_ld_overridden },
  { OVK_BOOL,	OV_INTERNAL, TRUE,"pf_L2_st", "",
    0, 0, 0, &CG_enable_pf_L2_st, &CG_enable_pf_L2_st_overridden },
  { OVK_BOOL,	OV_INTERNAL, TRUE, "exclusive_prefetch", "",
    0, 0, 0, &CG_exclusive_prefetch, NULL },
  { OVK_INT32,	OV_INTERNAL, TRUE, "L1_pf_latency", "",
    0, 0, INT32_MAX, &CG_L1_pf_latency, &CG_L1_pf_latency_overridden  },
  { OVK_INT32,	OV_INTERNAL, TRUE, "L2_pf_latency", "",
    0, 0, INT32_MAX, &CG_L2_pf_latency, &CG_L2_pf_latency_overridden },
  { OVK_INT32,	OV_INTERNAL, TRUE, "L1_ld_latency", "",
    0, 0, INT32_MAX, &CG_L1_ld_latency, &CG_L1_ld_latency_overridden },
  { OVK_INT32,	OV_INTERNAL, TRUE, "L2_ld_latency", "",
    0, 0, INT32_MAX, &CG_L2_ld_latency, &CG_L2_ld_latency_overridden },
  { OVK_INT32,	OV_INTERNAL, TRUE, "z_conf_L1_ld_latency", "",
    0, 0, INT32_MAX, &CG_z_conf_L1_ld_latency, NULL },
  { OVK_INT32,	OV_INTERNAL, TRUE, "z_conf_L2_ld_latency", "",
    0, 0, INT32_MAX, &CG_z_conf_L2_ld_latency, NULL },
  { OVK_INT32,	OV_INTERNAL, TRUE, "ld_latency", "",
    0, 0, INT32_MAX, &CG_ld_latency, NULL },

  // CGLOOP options.

  { OVK_BOOL,	OV_INTERNAL, TRUE, "loop_opt", "loop_opt",
    0, 0, 0,	&CG_enable_loop_optimizations, NULL },
  { OVK_BOOL,	OV_INTERNAL, TRUE, "opt_non_trip_countable", "opt_non_trip",
    0, 0, 0,	&CG_LOOP_optimize_non_trip_countable, NULL },
  { OVK_BOOL,	OV_INTERNAL, TRUE, "opt_lno_winddown_cache", NULL,
    0, 0, 0,	&CG_LOOP_optimize_lno_winddown_cache, NULL },
  { OVK_BOOL,	OV_INTERNAL, TRUE, "opt_lno_winddown_reg", NULL,
    0, 0, 0,	&CG_LOOP_optimize_lno_winddown_reg, NULL },
  { OVK_BOOL,	OV_INTERNAL, TRUE, "opt_non_innermost", "opt_non_inner",
    0, 0, 0,	&CG_LOOP_optimize_non_innermost, NULL },

  { OVK_BOOL,	OV_INTERNAL, TRUE,  "fix_recurrences", "",
    0, 0, 0,    &CG_LOOP_fix_recurrences,
		&CG_LOOP_fix_recurrences_specified },
  { OVK_BOOL,	OV_INTERNAL, TRUE,  "back_substitution", "",
    0, 0, 0,    &CG_LOOP_back_substitution,
		&CG_LOOP_back_substitution_specified },
  { OVK_BOOL,	OV_INTERNAL, TRUE,  "back_substitution_variant", "",
    0, 0, 0,    &CG_LOOP_back_substitution_variant,
		&CG_LOOP_back_substitution_variant_specified },
  { OVK_BOOL,	OV_INTERNAL, TRUE,  "interleave_reductions", "",
    0, 0, 0,    &CG_LOOP_interleave_reductions,
		&CG_LOOP_interleave_reductions_specified },
  { OVK_BOOL,	OV_INTERNAL, TRUE,  "interleave_posti", "",
    0, 0, 0,    &CG_LOOP_interleave_posti,
		&CG_LOOP_interleave_posti_specified },
  { OVK_BOOL,	OV_INTERNAL, TRUE,  "reassociate", "reassoc",
    0, 0, 0,    &CG_LOOP_reassociate,
		&CG_LOOP_reassociate_specified },
  { OVK_INT32, OV_INTERNAL, TRUE, "recurrence_min_omega", "",
    0, 0, INT32_MAX, &CG_LOOP_recurrence_min_omega, NULL },

  // CG Unrolling options - see also OPT:unroll_times_max:unroll_size.

  // Control flow optimizations (CFLOW) options.

  { OVK_BOOL,	OV_INTERNAL, TRUE,"cflow_before_cgprep", NULL,
    0, 0, 0, &CFLOW_opt_before_cgprep, NULL },
  { OVK_BOOL,	OV_INTERNAL, TRUE,"cflow_after_cgprep", "cflow_after_cgprep",
    0, 0, 0, &CFLOW_opt_after_cgprep, NULL },
  { OVK_BOOL,	OV_INTERNAL, TRUE,"cflow", NULL,
    0, 0, 0, &CFLOW_Enable, NULL },
  { OVK_BOOL,	OV_INTERNAL, TRUE,"cflow_unreachable", "",
    0, 0, 0, &CFLOW_Enable_Unreachable, NULL },
  { OVK_BOOL,	OV_INTERNAL, TRUE,"cflow_branch", "",
    0, 0, 0, &CFLOW_Enable_Branch, NULL },
  { OVK_BOOL,	OV_INTERNAL, TRUE,"cflow_merge", "",
    0, 0, 0, &CFLOW_Enable_Merge, NULL },
  { OVK_BOOL,	OV_INTERNAL, TRUE,"cflow_reorder", "",
    0, 0, 0, &CFLOW_Enable_Reorder, NULL },
  { OVK_BOOL,	OV_INTERNAL, TRUE,"cflow_clone", "",
    0, 0, 0, &CFLOW_Enable_Clone, &CFLOW_Enable_Clone_overridden },
  { OVK_BOOL,	OV_INTERNAL, TRUE,"cflow_freq_order", "",
    0, 0, 0, &CFLOW_Enable_Freq_Order, NULL },
  { OVK_BOOL,	OV_INTERNAL, TRUE,"cflow_opt_all_br_to_bcond", "",
    0, 0, 0, &CFLOW_opt_all_br_to_bcond, NULL },
  { OVK_NAME,	OV_INTERNAL, TRUE,"cflow_heuristic_tolerance", "",
    0, 0, 0, &CFLOW_heuristic_tolerance, NULL },
  { OVK_NAME,	OV_INTERNAL, TRUE,"cflow_feedback_tolerance", "",
    0, 0, 0, &CFLOW_feedback_tolerance, NULL },
  { OVK_INT32,	OV_INTERNAL, TRUE,"cflow_clone_incr", "cflow_clone_i",
    0, 0, 100, &CFLOW_clone_incr, &clone_incr_overridden },
  { OVK_INT32,	OV_INTERNAL, TRUE,"cflow_clone_min_incr", "cflow_clone_mi",
    0, 0, INT32_MAX, &CFLOW_clone_min_incr, &clone_min_incr_overridden },
  { OVK_INT32,	OV_INTERNAL, TRUE,"cflow_clone_max_incr", "cflow_clone_ma",
    0, 0, INT32_MAX, &CFLOW_clone_max_incr, &clone_max_incr_overridden },
  { OVK_NAME,	OV_INTERNAL, TRUE,"cflow_cold_threshold", "",
    0, 0, 0, &CFLOW_cold_threshold, NULL },


  { OVK_BOOL,	OV_INTERNAL, TRUE, "unique_exit", "",
    0, 0, 0,	&CG_unique_exit, NULL },
  { OVK_BOOL,	OV_INTERNAL, TRUE, "tail_call", "",
    0, 0, 0,	&CG_tail_call, &CG_tail_call_overridden },

  // Frequency heuristic/feedback options.

  { OVK_BOOL,	OV_INTERNAL, TRUE,"enable_frequency", "",
    0, 0, 0, &FREQ_enable, NULL },
  { OVK_NAME,	OV_INTERNAL, TRUE,"eh_freq", "",
    0, 0, 0, &FREQ_eh_freq, NULL },
  { OVK_NAME,	OV_INTERNAL, TRUE,"freq_frequent_never_ratio", "",
    0, 0, 0, &FREQ_frequent_never_ratio, NULL },
  { OVK_BOOL,	OV_INTERNAL, TRUE, "freq_view_cfg", "",
    0, 0, 0, &FREQ_view_cfg, NULL },

  // Whirl2ops / Expander options.

  { OVK_NAME,	OV_INTERNAL, TRUE,"fdiv_algorithm", "fdiv",
    0, 0, 0, &CGEXP_fdiv_algorithm, NULL },
  { OVK_NAME,	OV_INTERNAL, TRUE,"sqrt_algorithm", "sqrt",
    0, 0, 0, &CGEXP_sqrt_algorithm, NULL },
  { OVK_BOOL,	OV_INTERNAL, TRUE,"use_copyfcc", "",
    0, 0, 0, &CGEXP_use_copyfcc, NULL },
  { OVK_INT32,	OV_INTERNAL, TRUE,"expconst", "",
    DEFAULT_CGEXP_CONSTANT, 0, INT32_MAX, &CGEXP_expandconstant, NULL },
  { OVK_BOOL,	OV_INTERNAL, TRUE,"normalize_logical", "normalize",
    0, 0, 0, &CGEXP_normalize_logical, NULL },
  { OVK_BOOL,	OV_INTERNAL, TRUE,"gp_prolog_call_shared", "gp_prolog",
    0, 0, 0, &CGEXP_gp_prolog_call_shared, NULL },
  { OVK_BOOL,	OV_INTERNAL, TRUE,"integer_divide_by_constant", "integer_divide_by_constant",
    0, 0, 0, &CGEXP_cvrt_int_div_to_mult, &Integer_Divide_By_Constant_overridden },
  { OVK_BOOL,	OV_INTERNAL, TRUE,"integer_divide_use_float", "integer_divide_use_float",
    0, 0, 0, &CGEXP_cvrt_int_div_to_fdiv, &Integer_Divide_Use_Float_overridden },
  { OVK_BOOL,	OV_INTERNAL, TRUE,"fast_imul", "",
    0, 0, 0, &CGEXP_fast_imul, NULL },
  { OVK_BOOL,	OV_INTERNAL, TRUE,"float_consts_from_ints", "",
    0, 0, 0, &CGEXP_float_consts_from_ints, NULL },
  { OVK_BOOL,	OV_INTERNAL, TRUE,"float_div_by_const", "",
    0, 0, 0, &CGEXP_opt_float_div_by_const, NULL },

  { OVK_BOOL,	OV_INTERNAL, TRUE, "localize", "localize",
    0, 0, 0, &CG_localize_tns, &CG_localize_tns_Set},
  { OVK_BOOL,	OV_INTERNAL, TRUE, "localize_using_stacked_regs", "localize_using_stack",
    0, 0, 0, &LOCALIZE_using_stacked_regs, NULL },

  // Local Register Allocation (LRA) options.

  { OVK_BOOL,	OV_INTERNAL, TRUE,"rematerialize", "remat",
    0, 0, 0, &CGSPILL_Rematerialize_Constants, NULL },
  { OVK_BOOL,	OV_INTERNAL, TRUE,"force_rematerialization", "force_remat",
    0, 0, 0, &CGSPILL_Enable_Force_Rematerialization, NULL },
  { OVK_BOOL,	OV_INTERNAL, TRUE,"lra_reorder", "",
    0, 0, 0, &LRA_do_reorder, NULL },

  // Global Code Motion (GCM) options.

  {OVK_BOOL,	OV_INTERNAL, TRUE, "gcm", "gcm",
    0, 0, 0, &GCM_Enable_Scheduling, NULL },
  {OVK_BOOL,	OV_INTERNAL, TRUE, "pre_gcm", "pre_gcm",
    0, 0, 0, &GCM_PRE_Enable_Scheduling, NULL },
  {OVK_BOOL,	OV_INTERNAL, TRUE, "post_gcm", "post_gcm",
    0, 0, 0, &GCM_POST_Enable_Scheduling, NULL },
  {OVK_BOOL,	OV_INTERNAL, TRUE, "force_post_gcm", "force_post_gcm",
    0, 0, 0, &GCM_POST_Force_Scheduling, NULL },
  {OVK_BOOL,	OV_INTERNAL, TRUE, "cflow_after_gcm", "cflow_after_gcm",
    0, 0, 0, &GCM_Enable_Cflow, NULL},
  {OVK_BOOL,	OV_INTERNAL, TRUE, "cross_call_motion", "",
    0, 0, 0, &GCM_Motion_Across_Calls, NULL},
  {OVK_BOOL,	OV_INTERNAL, TRUE, "use_sched_est", "use_sched_est",
    0, 0, 0, &GCM_Use_Sched_Est, NULL},
  {OVK_BOOL,    OV_INTERNAL, TRUE, "pre_spec_loads", "",
    0, 0, 0, &GCM_PRE_Spec_Loads, NULL},
  {OVK_BOOL,    OV_INTERNAL, TRUE, "post_spec_loads", "",
    0, 0, 0, &GCM_POST_Spec_Loads, NULL},
  {OVK_BOOL,	OV_INTERNAL, TRUE, "pointer_speculation", "",
    0, 0, 0, &GCM_Pointer_Spec, NULL},
  {OVK_BOOL,	OV_INTERNAL, TRUE, "speculative_ptr_deref", "",
    0, 0, 0, &GCM_Eager_Ptr_Deref, NULL},
  {OVK_BOOL,	OV_INTERNAL, TRUE, "speculative_loads", "",
    0, 0, 0, &GCM_Speculative_Loads, NULL},
  {OVK_BOOL,	OV_INTERNAL, TRUE, "predicated_loads", "",
    0, 0, 0, &GCM_Predicated_Loads, NULL},
  {OVK_BOOL,	OV_INTERNAL, TRUE, "forw_circ_motion", "",
    0, 0, 0, &GCM_Forw_Circ_Motion, NULL},
  {OVK_BOOL,	OV_INTERNAL, TRUE, "gcm_minimize_reg_usage", "",
    0, 0, 0, &GCM_Min_Reg_Usage, NULL},
  {OVK_BOOL,	OV_INTERNAL, TRUE, "gcm_test", "",
    0, 0, 0, &GCM_Test, NULL},
  {OVK_BOOL,	OV_INTERNAL, TRUE, "skip_gcm", "",
    0, 0, 0, &CG_Skip_GCM, NULL},
  { OVK_INT32,	OV_INTERNAL, TRUE,"gcm_from_bb", "",
    0, 0, INT32_MAX, &GCM_From_BB, NULL },
  { OVK_INT32,	OV_INTERNAL, TRUE,"gcm_to_bb", "",
    0, 0, INT32_MAX, &GCM_To_BB, NULL },
  { OVK_INT32,	OV_INTERNAL, TRUE,"gcm_result_tn", "",
    0, 0, INT32_MAX, &GCM_Result_TN, NULL },

  // Local Scheduling (LOCS) and HyperBlock Scheduling (HBS) options.

  { OVK_BOOL,	OV_INTERNAL, TRUE,"fill_delay_slots", "fill_delay",
    0, 0, 0, &Enable_Fill_Delay_Slots, NULL },

  { OVK_BOOL,	OV_INTERNAL, TRUE,"local_scheduler", "local_sched",
    0, 0, 0, &LOCS_Enable_Scheduling, NULL },
  { OVK_BOOL,	OV_INTERNAL, TRUE,"pre_local_scheduler", "pre_local_sched",
    0, 0, 0, &LOCS_PRE_Enable_Scheduling, NULL },
  { OVK_BOOL,	OV_INTERNAL, TRUE,"post_local_scheduler", "post_local_sched",
    0, 0, 0, &LOCS_POST_Enable_Scheduling, NULL },
  { OVK_NAME,   OV_INTERNAL, TRUE,"branch_taken_prob", "",
    0, 0, 0,	&CGTARG_Branch_Taken_Prob,
		&CGTARG_Branch_Taken_Prob_overridden},
  { OVK_BOOL,	OV_INTERNAL, TRUE,"locs_form_bundles", "locs_form_bundles",
    0, 0, 0, &LOCS_Enable_Bundle_Formation, NULL },
  {OVK_BOOL,	OV_INTERNAL, TRUE, "pre_hb_scheduler", "pre_hb_sched",
    0, 0, 0, &IGLS_Enable_PRE_HB_Scheduling, NULL },
  {OVK_BOOL,	OV_INTERNAL, TRUE, "post_hb_scheduler", "post_hb_sched",
    0, 0, 0, &IGLS_Enable_POST_HB_Scheduling, NULL },
  { OVK_BOOL,	OV_INTERNAL, TRUE,"hb_scheduler", "hb_sched",
    0, 0, 0, &IGLS_Enable_HB_Scheduling, NULL },

  // Turns of all scheduling (LOCS, HBS, GCM) for triaging.
  { OVK_BOOL,	OV_INTERNAL, TRUE,"all_scheduler", "all_sched",
    0, 0, 0, &IGLS_Enable_All_Scheduling, NULL },

  // Hyperblock formation (HB) options.

  { OVK_BOOL,	OV_INTERNAL, TRUE,  "hb_formation", "",
    0,0,0,      &HB_formation, NULL,
    "Turn on/off hyperblock formation [Default ON]"
  },    
  { OVK_BOOL,	OV_INTERNAL, TRUE,  "hb_static_freq_heuristics", "",
    0,0,0,      &HB_static_freq_heuristics, NULL,
    "Turn on/off hyperblock formation's use of different heuristics in the presence of static frequency analysis [Default ON]"
  },    
  { OVK_INT32,	OV_INTERNAL, TRUE, "hb_max_blocks", "",
    4, 0, 100,	&HB_max_blocks, NULL,
    "How many blocks allowed in a hyperblock [Default architecturally dependent]"
  },
  { OVK_INT32,	OV_INTERNAL, TRUE, "hb_min_blocks", "",
    4, 0, 32,	&HB_min_blocks, NULL,
    "Minimum blocks allowed in a hyperblock [Default 2]"
  },
  { OVK_BOOL,	OV_INTERNAL, TRUE,  "hb_tail_duplication", "",
    0,0,0,      &HB_allow_tail_duplication, NULL, 
    "Flag to control tail-duplication when forming hyperblocks"
  },   
  { OVK_NAME,	OV_INTERNAL, TRUE, "hb_max_sched_growth", "",
    0, 0, 0,	&HB_max_sched_growth, NULL,
    "Multiplier for max increase in HB sched height [Default:3.0]"
  },
  { OVK_NAME,	OV_INTERNAL, TRUE,"hb_min_path_priority_ratio", "",
    0, 0, 0,	&HB_min_path_priority_ratio, NULL,
    "Ratio to control relative size of paths included in hyperblock [Default: .1]"
  },
  { OVK_NAME,	OV_INTERNAL, TRUE,"hb_min_priority", "",
    0, 0, 0,	&HB_min_priority, NULL,
    "Minimum priority allowed for a hyperblock [Default: .1]"
  },
  { OVK_NAME,	OV_INTERNAL, TRUE,"hb_call_hazard_multiplier", "",
    0, 0, 0,	&HB_call_hazard_multiplier, NULL,
    "Factor by which to reduce path priority in presence of calls [Default: .25]"
  },
  { OVK_NAME,	OV_INTERNAL, TRUE,"hb_memory_hazard_multiplier", "",
    0, 0, 0,	&HB_memory_hazard_multiplier, NULL,
    "Factor by which to reduce path priority in presence of unresolvable memory stores [Default: 1.0]"
  },
  { OVK_NAME,	OV_INTERNAL, TRUE,"hb_base_probability_contribution", "",
    0, 0, 0,	&HB_base_probability_contribution, NULL,
    "Factor to ensure base contribution of path probability to priority [Default: 0.1]"
  },
  { OVK_BOOL,	OV_INTERNAL, TRUE,  "hb_require_alias", "",
    0,0,0,      &HB_require_alias, NULL,
    "Turn on/off requirement that alias information be present for complex hyperblock formation [Default ON]"
  },
  { OVK_BOOL,	OV_INTERNAL, TRUE,  "hb_complex_non_loop", "",
    0,0,0,      &HB_complex_non_loop, NULL,
    "Turn on/off complex hyperblock formation for non-loop regions [Default ON]"
  },
  { OVK_BOOL,	OV_INTERNAL, TRUE,  "hb_simple_ifc", "",
    0,0,0,      &HB_simple_ifc, &HB_simple_ifc_set,
    "Turn on/off simple, always profitable hyperblock formation for non-loop regions [Default ON]"
  },
  { OVK_BOOL,	OV_INTERNAL, TRUE,  "hb_general_use_pq", "",
    0,0,0,      &HB_general_use_pq, NULL,
    "Turn on/off using priority queue when following side paths in general region id for hyperblocks [Default OFF]"
  },
  { OVK_BOOL,	OV_INTERNAL, TRUE,  "hb_general_from_top", "",
    0,0,0,      &HB_general_from_top, NULL,
    "Turn on/off following side paths from top of main path in general region id for hyperblocks [Default OFF]"
  },
  { OVK_BOOL,	OV_INTERNAL, TRUE,  "hb_exclude_calls", "",
    0,0,0,      &HB_exclude_calls, NULL,
    "Disallow blocks with calls during hyperblock formation, temporary workaround before full support for predicate callee-register spilling is included [Default ON]"
  },
  { OVK_BOOL,	OV_INTERNAL, TRUE,  "hb_exclude_pgtns", "",
    0,0,0,      &HB_exclude_pgtns, NULL,
    "Disallow forming hyperblocks if it consists of any global predicate TNs (PGTNS) [Default ON]"
  },
  { OVK_BOOL,	OV_INTERNAL, TRUE,  "hb_skip_hammocks", "",
    0,0,0,      &HB_skip_hammocks, NULL,
    "Skip forming hyperblocks on hammocks, cause later pass will do them [Default ON]"
  },
  { OVK_INT32,	OV_INTERNAL, TRUE, "loop_force_ifc", "",
    0, 0, 2,    &CG_LOOP_force_ifc, NULL },

  // EBO Options

  { OVK_INT32,  OV_INTERNAL, TRUE,"ebo_level", "ebo",
    0, INT32_MIN, INT32_MAX, &EBO_Opt_Level, &EBO_Opt_Level_overridden },

  // Emit options
  { OVK_INT32,	OV_INTERNAL, TRUE,"longbranch_limit", "",
    DEFAULT_LONG_BRANCH_LIMIT, 0, INT32_MAX, &EMIT_Long_Branch_Limit, NULL },
  { OVK_BOOL,	OV_INTERNAL, TRUE,"pjump_all", "pjump_all",
    0, 0, 0, &EMIT_pjump_all, NULL },
  { OVK_BOOL,	OV_INTERNAL, TRUE,"use_cold_section", "use_cold_section",
    0, 0, 0, &EMIT_use_cold_section, NULL },
  { OVK_BOOL,   OV_INTERNAL, TRUE,  "emit_asm_dwarf", "",
    0,0,0,      &CG_emit_asm_dwarf, NULL,
    "Turn on/off emission of dwarf data into .s file [Default OFF]"
  },
  { OVK_BOOL,   OV_INTERNAL, TRUE,  "emit_unwind_directives", "",
    0,0,0,      &CG_emit_unwind_directives, NULL,
    "Turn on/off emission of unwind directives into .s file [Default OFF]"
  },
  { OVK_BOOL,   OV_INTERNAL, TRUE,  "emit_unwind_info", "",
    0,0,0,      &CG_emit_unwind_info, NULL,
    "Turn on/off emission of unwind into .s/.o file [Default OFF]"
  },
  { OVK_BOOL,	OV_INTERNAL, TRUE,"volatile_asm_stop", "",
    0, 0, 0, &EMIT_stop_bits_for_volatile_asm, NULL },
  { OVK_BOOL,	OV_INTERNAL, TRUE,"emit_stop_bits_for_asm", "",
    0, 0, 0, &EMIT_stop_bits_for_asm, NULL },
  { OVK_BOOL,	OV_INTERNAL, TRUE,"emit_explicit_bundles", "",
    0, 0, 0, &EMIT_explicit_bundles, NULL },
  { OVK_COUNT },

  // Misc:
  { OVK_BOOL,	OV_INTERNAL, TRUE,  "gra_live_predicate_aware", "",
    0,0,0,      &GRA_LIVE_Predicate_Aware, NULL,
    "Allow GRA_LIVE to be predicate-aware [Default ON]"
  },

  { OVK_INT32,	OV_INTERNAL, TRUE,"branch_taken_penalty", "",
    0, 0, INT32_MAX, &CGTARG_branch_taken_penalty,
    &CGTARG_branch_taken_penalty_overridden },

  { OVK_INT32, OV_INTERNAL, TRUE, "mispredict_branch", "mispredict",
    0, 0, INT32_MAX, &CG_branch_mispredict_penalty, NULL },
  { OVK_INT32, OV_INTERNAL, TRUE, "mispredict_factor", "",
    0, 0, INT32_MAX, &CG_branch_mispredict_factor, NULL },
  { OVK_BOOL,	OV_INTERNAL, TRUE,"enable_thr", "",
    0, 0, 0,	&CG_enable_thr, NULL },

  { OVK_BOOL,	OV_INTERNAL, TRUE,"reverse_if_conversion", "",
    0, 0, 0,	&CG_enable_reverse_if_conversion,
	       	&CG_enable_reverse_if_conversion_overridden },
  { OVK_INT32,	OV_INTERNAL, TRUE,"body_ins_count_max", "",
    0, 0, INT32_MAX, &CG_maxinss, &CG_maxinss_overridden },
  { OVK_INT32,	OV_INTERNAL, TRUE,"body_blocks_count_max", "",
    0, 0, INT32_MAX, &CG_maxblocks, NULL },

  { OVK_BOOL,	OV_INTERNAL, FALSE,"callee_reg_mask", "callee_reg_mask",
    0, 0, 0, &CG_gen_callee_saved_regs_mask, NULL },

  { OVK_COUNT }
};

OPTION_GROUP CG_Option_Groups[] = {
  { "SWP", ':', '=', Options_CG_SWP },
  { "CG", ':', '=', Options_CG },
  { "GRA", ':', '=', Options_GRA },
  { NULL }		/* List terminator -- must be last */
};


INT _prefetch_ahead = 2;
#ifndef _NO_WEAK_SUPPORT_
extern INT prefetch_ahead;
#pragma weak prefetch_ahead = _prefetch_ahead
#else
INT* prefetch_ahead_p = &_prefetch_ahead;
#define prefetch_ahead (*prefetch_ahead_p)
#endif

/* =======================================================================
 *
 *  Configure_Prefetch
 *
 *  Configure the prefetch flags controlled by prefetch_ahead exported
 *  from LNO. It MUST be called after lno.so has been loaded.
 *
 * =======================================================================
 */
static void
Configure_prefetch_ahead(void)
{
  static INT32 save_L1_pf_latency = -1;
  static INT32 save_L2_pf_latency = -1;
  if ( save_L1_pf_latency < 0 ) {
    save_L1_pf_latency = CG_L1_pf_latency;
    save_L2_pf_latency = CG_L2_pf_latency;
  }
#ifdef TARG_ST
  if (PROC_has_enable_prefetch_ahead()) {
#else
  if (Enable_Prefetch_Ahead_For_Target()) {
#endif
    if ( ! CG_L2_pf_latency_overridden )
      if ( prefetch_ahead ) 
	CG_L2_pf_latency = 0;
      else
	CG_L2_pf_latency = save_L2_pf_latency;
    if ( ! CG_L1_pf_latency_overridden )
      if (prefetch_ahead)
	CG_L1_pf_latency = 0;
      else
	CG_L1_pf_latency = save_L1_pf_latency;
  }
}

/* =======================================================================
 *
 *  Configure_Prefetch
 *
 *  Configure the prefetch flags.
 *
 * =======================================================================
 */
static void
Configure_Prefetch(void)
{
  if ( ! OPT_shared_memory) {
	CG_exclusive_prefetch = TRUE;
  }
  /* Detect any of the various cases that cause us to disable 
   * prefetching entirely:
   *   isa < mips4
   *   -CG:prefetch=off
   *   -CG:z_conf_prefetch=off:nz_conf_prefetch=off
   */ 
#ifdef TARG_ST
  if (!PROC_has_prefetch()
#else 
  if (   ! Target_Has_Prefetch()
#endif
      || (CG_enable_prefetch_overridden && ! CG_enable_prefetch)
      || (   CG_enable_z_conf_prefetch_overridden 
	  && ! CG_enable_z_conf_prefetch
          && CG_enable_nz_conf_prefetch_overridden 
	  && ! CG_enable_nz_conf_prefetch)
  ) {
disable_prefetch:
    CG_enable_prefetch = FALSE;
    CG_enable_z_conf_prefetch  = FALSE;
    CG_enable_nz_conf_prefetch = FALSE;
    CG_enable_pf_L1_ld = FALSE;
    CG_enable_pf_L1_st = FALSE;
    CG_enable_pf_L2_ld = FALSE;
    CG_enable_pf_L2_st = FALSE;
    return;
  }

  /* At this point, -CG:prefetch was explicitly set to true, or
   * unspecified.
   */
  if ( ! CG_enable_prefetch_overridden ) {
    CG_enable_prefetch = FALSE;

    /* -CG:z_conf_prefetch or -CG:nz_conf_prefetch implicitly
     * set to TRUE, implies we should enable prefetching.
     */
    if (   (   CG_enable_z_conf_prefetch_overridden 
	    && CG_enable_z_conf_prefetch)
        || (   CG_enable_nz_conf_prefetch_overridden 
	    && CG_enable_nz_conf_prefetch)
    ) {
      CG_enable_prefetch = TRUE;
    }

    /* Some targets implicitly enable prefetching.
     */
#ifdef TARG_ST
    else if (PROC_has_enable_prefetch()) {
#else
    else if (Enable_Prefetch_For_Target()) {
#endif
      CG_enable_prefetch = TRUE;
    }

    /* No implicit enable of prefetching this time...
     */
    else goto disable_prefetch;
  }

  /* Prefetching is enabled, implicitly or explicitly. Handle any
   * defaults, both target independent and target specific.
   */
  if ( ! CG_enable_z_conf_prefetch_overridden )
    CG_enable_z_conf_prefetch = FALSE;
  if ( ! CG_enable_nz_conf_prefetch_overridden )
    CG_enable_nz_conf_prefetch = TRUE;

#ifdef TARG_ST
  if (PROC_has_enable_prefetch()) {
#else
  if (Enable_Prefetch_For_Target()) {
#endif
    if ( ! CG_L1_ld_latency_overridden ) CG_L1_ld_latency = 8;
    if ( ! CG_enable_pf_L1_ld_overridden ) CG_enable_pf_L1_ld = FALSE;
    if ( ! CG_enable_pf_L1_st_overridden ) CG_enable_pf_L1_st = FALSE;
    if ( ! CG_enable_pf_L2_ld_overridden ) CG_enable_pf_L2_ld = TRUE;
    if ( ! CG_enable_pf_L2_st_overridden ) CG_enable_pf_L2_st = TRUE;
  } else {
    if ( ! CG_enable_pf_L1_ld_overridden ) CG_enable_pf_L1_ld = TRUE;
    if ( ! CG_enable_pf_L1_st_overridden ) CG_enable_pf_L1_st = TRUE;
    if ( ! CG_enable_pf_L2_ld_overridden ) CG_enable_pf_L2_ld = TRUE;
    if ( ! CG_enable_pf_L2_st_overridden ) CG_enable_pf_L2_st = TRUE;
  }

  /* Finally, check to see if we actually will do any prefetching, and
   * if not, disable prefetching all together.
   */
  if (   ! CG_enable_pf_L1_ld
      && ! CG_enable_pf_L1_st
      && ! CG_enable_pf_L2_ld
      && ! CG_enable_pf_L2_st ) goto disable_prefetch;
}

/* =======================================================================
 *
 *  Configure_CG_Options
 *
 *  After the comand line has been processed and CG_opt_level set, configure
 *  the various CG flags that depend on these two things.
 *  This is also called per PU if the PU opt level changes.
 *
 * =======================================================================
 */
static void
Configure_CG_Options(void)
{
  /* Set code generation options -- see	cg.h: */

  if (!CG_localize_tns_Set)
    CG_localize_tns = (CG_opt_level <= 1);

#ifdef TARG_ST
  if (!Enable_SWP_overridden)
  {
    Enable_SWP = FALSE;
  }
  else {
    Enable_SWP = CG_opt_level >= 2;

    if (!Enable_SWP)
      DevWarn("Configure_CG_Options: SWP=ON but Opt_Level < 2; ignoring SWP");

  }
#else
  if (!Enable_SWP_overridden)
  {
    Enable_SWP = (CG_opt_level > 2) && ! OPT_Space;
#ifdef TARG_IA64
    Enable_SWP = CG_opt_level >= 2;
#endif
  }
#endif

  if (CG_opt_level > 2 && !OPT_unroll_size_overridden )
    OPT_unroll_size = 128;
  
  if ( OPT_Unroll_Analysis_Set )
  {
    CG_LOOP_unroll_analysis = OPT_Unroll_Analysis;
  }
  CG_LOOP_unroll_times_max = OPT_unroll_times;
  CG_LOOP_unrolled_size_max = OPT_unroll_size;

  CG_LOOP_ooo_unroll_heuristics = PROC_is_out_of_order();

  if (OPT_Space)
  {
    CGEXP_expandconstant = 2;
  }

#if 0
  if (!Integer_Divide_By_Constant_overridden) {
    CGEXP_cvrt_int_div_to_mult = (!OPT_Space) && (CG_opt_level > 0);
  } 

  if (!Integer_Divide_Use_Float_overridden) {
    CGEXP_cvrt_int_div_to_fdiv =    !Kernel_Code
				 && Enable_Idiv_In_FPU_For_Target()
				 && !OPT_Space
				 && CG_opt_level > 0;
  }
#endif

  if (Kernel_Code && !CG_tail_call_overridden) CG_tail_call = FALSE;

  if (Kernel_Code && !GCM_Speculative_Ptr_Deref_Set)
    GCM_Eager_Ptr_Deref = FALSE;

  if (!CGTARG_Branch_Taken_Prob_overridden)
    CGTARG_Branch_Taken_Prob = "0.95";
  CGTARG_Branch_Taken_Probability = atof(CGTARG_Branch_Taken_Prob);
  
#if 0
  if ( !CG_enable_spec_idiv_overridden && Enable_Spec_Idiv_For_Target() )
    CG_enable_spec_idiv = FALSE;
#endif

  if ( ! CG_LOOP_fix_recurrences_specified
       && (      CG_LOOP_back_substitution
              && CG_LOOP_back_substitution_specified
           ||    CG_LOOP_interleave_reductions
              && CG_LOOP_interleave_reductions_specified
           ||    CG_LOOP_interleave_posti
	      && CG_LOOP_interleave_posti_specified
           ||    CG_LOOP_reassociate 
              && CG_LOOP_reassociate_specified)) {
    CG_LOOP_fix_recurrences = TRUE;
  }

  if ( Enable_SWP && ! Enable_LOH_overridden )
#ifdef TARG_ST
    Enable_LOH = FALSE; // don't know what it is exactly and how to
                        // parametrize the proc_properties.cxx 
#else
    Enable_LOH = Enable_LOH_For_Target();
#endif

  if (!EBO_Opt_Level_overridden) {
    EBO_Opt_Level = (CG_opt_level > 0) ? EBO_Opt_Level_Default : 0;
  }
#ifdef TARG_ST
  if (!Enable_CG_Peephole_overridden) {
    CG_enable_peephole = (CG_opt_level > 0) ? TRUE : FALSE;
  }

  // If user explicitely set SSA.
  // However, don't allow if opt_level < 1
#if 0
  // development: ssa off by default
  if (!CG_enable_ssa_overridden) {
    CG_enable_ssa = (CG_opt_level > 1) ? TRUE : FALSE;
  }
  else {
#endif
    if (CG_enable_ssa && (CG_opt_level < 2)) {
      DevWarn("CG: Ignoring ssa=ON, need optimization level -O2 or higher");
      CG_enable_ssa = FALSE;
    }
#if 0
  }
#endif

#if 0
  // development: select off by default
  if (!CG_enable_select_overridden) {
    CG_enable_select =  (CG_opt_level > 1) ? CGTARG_Can_Select() : FALSE;
  }
#endif

  if (CG_enable_select && !CG_enable_ssa) {
    DevWarn("CG: Ignoring select=ON, need ssa");
    CG_enable_select = FALSE;
  }

#endif  /* TARG_ST */

  /* Enable_Fill_Delay_Slots controls the filling of delay slots in locs
     and gcm */
#ifdef TARG_ST
  if (!PROC_has_branch_delay_slot() || !Enable_Fill_Delay_Slots) 
#else
  if (!Enable_Fill_Delay_Slots_For_Target() || !Enable_Fill_Delay_Slots) 
#endif
    GCM_Enable_Fill_Delay_Slots = FALSE;

  /* Clamp body_ins_count_max to max BB length
   */
  if (CG_maxinss_overridden) {
    if (CG_maxinss > CG_split_BB_length) {
      CG_split_BB_length = CG_maxinss;
    }
  } else {
    CG_maxinss = CG_maxinss_default * CG_opt_level;
    if (CG_maxinss == 0 || CG_maxinss > CG_split_BB_length) {
      CG_maxinss = CG_split_BB_length;
    }
  }

  /* Set BB clone limits
   */
  if ( Kernel_Code && ! CFLOW_Enable_Clone_overridden ) {
    // if kernel code then want really minimal space,
    // so turn off cloning altogether
    CFLOW_Enable_Clone = FALSE;
  } else if (OPT_Space) {
    if (!clone_incr_overridden) CFLOW_clone_incr = 1;
    if (!clone_min_incr_overridden) CFLOW_clone_min_incr = 1;
    if (!clone_max_incr_overridden) CFLOW_clone_max_incr = 3;
  }

  Configure_Prefetch();

  // target specific CG option setting
  Configure_CG_Target();

  // Check the LOCS_Enable_Bundle_Formation for consistency
#if 0
  if (Lai_Code && LOCS_Enable_Bundle_Formation) {
    // DevWarn for now until I figure out how to emit a real
    // warning. I will also need to use overriden flag because
    // the LOCS_Enable_Bundle_Formation is ON by default, but
    // I do not need to issue a warning if it is not explicitely
    // requested in the command line by the user.
    DevWarn("CG: Ignoring LOCS_Enable_Bundle_Formation set for lai.");
    LOCS_Enable_Bundle_Formation = FALSE;
  }

  // Disable scheduling, etc. for some targets
  // Temporarily here. When the cg is complete, such options should be
  // set using -CG:
  //    locs_form_bundles=
  //    all_scheduler=
  //    enable_thr=
  //    lra_reorder=
  //    callee_reg_mask=
  //    etc.
#ifdef TARG_ST
  IGLS_Enable_All_Scheduling = FALSE;
  CG_enable_thr = FALSE;
  LRA_do_reorder = FALSE;
  // Do not force if-conversion for loops, see CG_LOOP_Optimize()
  CG_LOOP_force_ifc = 0;

  HB_skip_hammocks = FALSE;
#endif
#endif

#ifdef TARG_ST100
  //IGLS_Enable_All_Scheduling = FALSE;
  if (!Lai_Code) {
    // Generate the push/pop sequence
    CG_gen_callee_saved_regs_mask = TRUE;
  }
#endif

  return;
}

/* =======================================================================
 *  CG_Configure_Opt_Level
 *
 *  See interface description.
 * =======================================================================
 */
void
CG_Configure_Opt_Level ( 
  INT opt_level 
)
{
  static BOOL opt_level_configured = FALSE;

  if ( opt_level_configured && opt_level == CG_opt_level )
    return;

  if ( opt_level_configured && cg_opt_level_overridden ) {
    /* forget it */
    DevWarn("Attempt to override CG:opt_level=%d flag. Ignored.",CG_opt_level);
    return;
  }

  opt_level_configured = TRUE;

  if ( ! cg_opt_level_overridden )
    CG_opt_level = opt_level;

  Configure_CG_Options();
}

/* ====================================================================
 *
 * Build_Option_String
 *
 * Just	build a	string of all the options passed to the	Common Core
 * process, so the options can be printed at the beginning of the "*.s"
 * file(s).
 *
 * ====================================================================
 */

static void
Build_Option_String (INT argc, char **argv)
{
    INT16 i;
    INT16 arg_size = 0;

    Argv0 = argv[0];		    /* save argv[0] for R_Assemble_File() */

    for (i=1; i<argc; ++i)	    /* skip arg 0 (the program name) */
	if ( argv[i][0] == '-'  && argv[i][1] != 'f')
	    arg_size += ( strlen(argv[i]) + 1 );

    if ( arg_size > 0 ) {
	register char *p;
	
	p = option_string = (char *) malloc(arg_size+1);

	if ( option_string == NULL ) {
	    ErrMsg ( EC_No_Mem, "Build_Option_String" );
	    exit ( 1 );
	}

	p[0] = '\0';
	for (i=1; i<argc; ++i)
	    if ( argv[i][0] == '-'  && argv[i][1] != 'f') {
		register INT len = strlen (argv[i]) + 1;
		if (p != option_string)
		    *p++ = ' ';
		BCOPY (argv[i], p, len);
		p += len - 1;
	    }
	
    } else {			    /* no options specified */
	option_string = "none";
    }
}

/* ====================================================================
 *   Process_Command_Line
 *
 *   Process the command line arguments specific to CG.
 * ====================================================================
 */

static void
Process_Command_Line (
  INT argc, 
  char **argv
)
{
  INT16 i;
  char *cp;

  /* Check the command line flags: */
  for ( i=0; i<argc; i++ ) {
    if ( argv[i] != NULL && *(argv[i]) == '-' ) {
      cp = argv[i]+1;	    /* Pointer to next flag character */

      /* First try to process as command-line option group */
      if (Process_Command_Line_Group(cp, CG_Option_Groups))
	continue;

      switch ( *cp++ ) {
        case 'f':		    /* file options */
	  /* error case already handled by main driver */
	  switch (*cp) {
	    case 'a':	    /* Assembly file */
	    case 's':
	      Assembly = TRUE;
	      Asm_File_Name = cp + 2;
	      break;

	    case 'o':	    /* object file */
	      Object_Code = TRUE;
	      Obj_File_Name = cp + 2;
	      break;

	    case 'L':	            /* Lai file */
	      Lai_Code = TRUE;
	      Lai_File_Name = cp + 2;
	      break;
	  }
	  break;

        case 's':		    /* -s: Produce assembly file: */
        case 'S':		    /* -S: Produce assembly file: */
	  Assembly = TRUE;
	  break;

        case 't':
	  /* handle the -tfprev10 option to fix tfp hardware bugs. */
	  if ( strncmp ( cp-1, "tfprev10", 8 ) == 0 ) {
	    No_Quad_Aligned_Branch = TRUE;
	  }

	  break;

        default:
	  ErrMsg (EC_Unknown_Flag, *(cp-1), argv[i]);
      }
    }
  }

  return;
}

/* ====================================================================
 *   Prepare_Source
 *
 *   Process the source argument and associated files.
 * ====================================================================
 */
static void
Prepare_Source (void)
{
  char *fname;

  /* We've got a source file name -- open other files.
   * We want them to be created in the current directory, so we
   * strip off the filename only from Src_File_Name for use:
   */
  fname = Last_Pathname_Component ( Src_File_Name );

  /* If we're producing information for CITE, we need an assembly
   * file even if it wasn't explicitly requested:
   */
  if ( List_Cite ) {
    Assembly = TRUE;
  }

#if defined(TARG_ST100) || defined(TARG_ST200)
  if (Object_Code) {
    FmtAssert(FALSE,("SORRY: object code not generated for this target"));
  }
#ifdef TARG_ST200
  if (Lai_Code) {
    FmtAssert(FALSE,("SORRY: LAI not available for this target"));
  }
#endif

#else
  if (Lai_Code) {
    FmtAssert(FALSE,("SORRY: LAI not available for this target"));
  }
#endif

  if (Lai_Code) {
    if (Lai_File_Name == NULL) {
      /* Replace source file extension to get assembly file name: */
      Lai_File_Name = New_Extension (fname, LAI_FILE_EXTENSION);
    }

    /* Open the LAI file for compilation: */
    if ((Lai_File = fopen (Lai_File_Name, "w")) == NULL) {
      ErrMsg (EC_Asm_Open, Lai_File_Name, errno);
      Terminate (1);
    }
  }

  if ( Assembly ) {
    if ( Asm_File_Name == NULL ) {
      /* Replace source file extension to get assembly file name: */
      Asm_File_Name = New_Extension (fname, ASM_FILE_EXTENSION );
    }

    /* Open	the ASM	file for compilation: */
    if ( ( Asm_File	= fopen	( Asm_File_Name, "w" ) ) == NULL ) {
      ErrMsg ( EC_Asm_Open, Asm_File_Name, errno );
      Terminate (1);
    }
  }

  /* Prepare relocatable object file name: */
  if ( Obj_File_Name == NULL ) {
    /* Replace source file extension to get	object file: */
    Obj_File_Name =	New_Extension (fname, OBJ_FILE_EXTENSION);
  }

  return;
}

/* ====================================================================
 *   CG_Process_Command_Line (cg_argc, cg_argv, be_argc, be_argv)
 *
 *   Main entry point and driver for the Code Generator.
 * ====================================================================
 */
void
CG_Process_Command_Line (
  INT    cg_argc, 
  char **cg_argv, 
  INT    be_argc, 
  char **be_argv
)
{
  BE_EXPORTED extern char *Whirl_Revision;

  if (strcmp (Whirl_Revision, WHIRL_REVISION) != 0)
    FmtAssert (!DEBUG_Ir_Version_Check,
	       ("WHIRL revision mismatch between be.so (%s) and cg.so (%s)",
		Whirl_Revision, WHIRL_REVISION));

  /* Perform preliminary command line processing: */
  Build_Option_String (be_argc, be_argv);
  Process_Command_Line (cg_argc, cg_argv);

  CG_Configure_Opt_Level(Opt_Level);

  Prepare_Source ();
  return;
} /* CG_Process_Command_Line */

// **************** symbols defined in lao.so ****************

#ifdef TARG_ST

#if defined(__linux__) || defined(_NO_WEAK_SUPPORT_)

CG_EXPORTED void (*lao_init_p) (void);
CG_EXPORTED void (*lao_fini_p) (void);

CG_EXPORTED extern void (*lao_init_p) (void);
#define lao_init (*lao_init_p)

CG_EXPORTED extern void (*lao_fini_p) ();
#define lao_fini (*lao_fini_p)

#else 

#pragma weak lao_init
#pragma weak lao_fini

#endif // __linux__ || _NO_WEAK_SUPPORT_

static void* lao_handler = NULL;

// Also, define here function pointers to other entry points in the
// LAO library.

CG_EXPORTED bool (*lao_optimize_LOOP_p) (CG_LOOP *cg_loop, unsigned lao_actions);
CG_EXPORTED bool (*lao_optimize_HB_p) (HB *hb, unsigned lao_actions);
CG_EXPORTED bool (*lao_optimize_FUNC_p) (unsigned lao_actions);

#endif

#if defined(__MINGW32__) || defined(__CYGWIN__)
#define SO_EXT ".dll"
#else
#define SO_EXT ".so"
#endif

/* ====================================================================
 *   CG_Init ()
 *
 *   Initialization that needs to be done after the global symtab is 
 *   read 
 * ====================================================================
 */
void
CG_Init (void)
{
  Set_Error_Phase ( "CG Initialization" );

  MEM_POOL_Initialize (&MEM_local_region_pool, "local_region_pool", TRUE);
  MEM_POOL_Initialize (&MEM_local_region_nz_pool, "local_region_nz_pool", FALSE);

  REGISTER_Begin();	/* initialize the register package */
  Init_Dedicated_TNs ();
  Mark_Specified_Registers_As_Not_Allocatable ();

  // These are for the .lai processing:
  EMT_Begin_File (Argv0, option_string);

  /* this has to be done after LNO has been loaded to grep
   * prefetch_ahead fromn LNO */
  //    Configure_prefetch_ahead();

#ifdef TARG_ST
  if (CG_enable_LAO) {
    lao_handler = load_so ("lao"SO_EXT, CG_Path, Show_Progress);
    lao_init();
  }
#endif

  return;
}

/* ====================================================================
 *   CG_Fini ()
 *
 *   Terimination routines for cg 
 * ====================================================================
 */
void
CG_Fini (void)
{

#ifdef TARG_ST

  if (CG_enable_LAO) {
    lao_fini();
    close_so(lao_handler);
    lao_handler = NULL;
  }

#endif

  /* List global symbols if desired: */
  if (List_Symbols) {
    Print_global_symtab (Lst_File);
  }

  Set_Error_Phase ( "Codegen Emit" );
  /* Finish off the relocatable object file: */
  EMT_End_File();

  MEM_POOL_Delete (&MEM_local_region_pool);
  MEM_POOL_Delete (&MEM_local_region_nz_pool);

  return;
} 
