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


//  
//  Generate PROCESSOR properties information
///////////////////////////////////////
//  $Revision$
//  $Date$
//  $Author$
//  $Source$

#include <stddef.h>
#include "targ_proc.h"
#include "proc_properties_gen.h"

main()
{
  PROC_PROPERTY 
    branch_delay_slot,		/* branch delay slot */
    same_cycle_branch_shadow,   /* execute branch shadow parallel with branch */
    out_of_order, 		/* out of order execution */
    superscalar,		/* multiple insts per cycle */
    ia64_predication,           /* supports IA64 predication model */
    select,                     /* supports partial predication model */
    predicate_branches,         /* can predicate branches */
    predicate_calls,            /* can predicate calls */
    predicate_returns,          /* can predicate returns */
    bundles,			/* executes insts as sequence of bundles */
#ifdef TARG_ST
    scoreboard,			/* Inter bundle hazards are scoreboarded */
#endif
    prefetch,                   /* supports prefetch instructions */
    enable_prefetch,            /* implicitely enables prefetch */
    enable_prefetch_ahead,      /* enables prefetch to L2 cache */
    counted_loops,              /* supports counted loop branches */
    swp_branches,               /* supports SWP branches */
    delayed_exception,		/* has delayed exception support */
    fast_recip;			/* recip inst is fast */

  PROC_Properties_Begin ("st200");

/* ====================================================================
 *              Operator attributes descriptors
 * ====================================================================
 */

  /* Does the target have branch delay slots?
   */
  branch_delay_slot = PROC_Property_Create ("has_branch_delay_slot");
  Processor_Group (branch_delay_slot, 
			PROCESSOR_UNDEFINED);

  /* Can the branch shadow be executed in the same cycle as the branch on
   * the target?
   */
  same_cycle_branch_shadow = PROC_Property_Create ("has_same_cycle_branch_shadow");
  Processor_Group (same_cycle_branch_shadow, 
			PROCESSOR_UNDEFINED);

  /* Is the target an out-of-order machine?
   */
  out_of_order = PROC_Property_Create ("is_out_of_order");
  Processor_Group (out_of_order, 
			PROCESSOR_UNDEFINED);

  /* Can the current target issue multiple instructions per cycle?
   */
  superscalar = PROC_Property_Create ("is_superscalar");
  Processor_Group (superscalar,
			PROCESSOR_st220,
			PROCESSOR_st221,
			PROCESSOR_UNDEFINED);

  ia64_predication = PROC_Property_Create ("is_ia64_predication");
  Processor_Group (ia64_predication, 
			PROCESSOR_UNDEFINED);

  select = PROC_Property_Create ("is_select");
  Processor_Group (select,
			PROCESSOR_st220,
			PROCESSOR_st221,
			PROCESSOR_UNDEFINED);

  predicate_branches = PROC_Property_Create ("has_predicate_branches");
  Processor_Group (predicate_branches, 
			PROCESSOR_UNDEFINED);

  predicate_calls = PROC_Property_Create ("has_predicate_calls");
  Processor_Group (predicate_calls, 
			PROCESSOR_UNDEFINED);

  predicate_returns = PROC_Property_Create ("has_predicate_returns");
  Processor_Group (predicate_returns, 
			PROCESSOR_UNDEFINED);


  /* Does the target execute insts as sequence of bundles, or require 
   * bundle alignment? The info is used to align instructions to bundles, 
   * resolve any bundle packing requirements, etc...
   */
  bundles = PROC_Property_Create ("has_bundles");
  Processor_Group (bundles,
			PROCESSOR_st220,
			PROCESSOR_st221,
			PROCESSOR_UNDEFINED);

#ifdef TARG_ST
  bundles = PROC_Property_Create ("has_scoreboard");
  Processor_Group (bundles,
			PROCESSOR_st221,
			PROCESSOR_UNDEFINED);
#endif

  prefetch = PROC_Property_Create ("has_prefetch");
  Processor_Group (prefetch,
			PROCESSOR_st220,
			PROCESSOR_st221,
			PROCESSOR_UNDEFINED);

  enable_prefetch = PROC_Property_Create ("has_enable_prefetch");
  Processor_Group (enable_prefetch,
		        PROCESSOR_st220,
			PROCESSOR_st221,
			PROCESSOR_UNDEFINED);

  enable_prefetch_ahead = PROC_Property_Create ("has_enable_prefetch_ahead");
  Processor_Group (enable_prefetch_ahead,
			PROCESSOR_UNDEFINED);

  counted_loops = PROC_Property_Create ("has_counted_loops");
  Processor_Group (counted_loops,
			PROCESSOR_UNDEFINED);

  swp_branches = PROC_Property_Create ("has_swp_branches");
  Processor_Group (swp_branches,
			PROCESSOR_UNDEFINED);

  /* Does the target support delayed_exception mechanism, i.e ability to
   * suppress possible exceptions for speculative instructions with
   * delayed recovery mechanism.
   */
  delayed_exception = PROC_Property_Create ("has_delayed_exception");
  Processor_Group (delayed_exception,
			PROCESSOR_st220,
			PROCESSOR_st221,
			PROCESSOR_UNDEFINED);

  /* Does the target have a fast recip instruction? 
   * i.e. is it profitable to convert a/b -> a*recip(b)
   */
  fast_recip = PROC_Property_Create ("has_fast_recip");
  Processor_Group (fast_recip,
			PROCESSOR_UNDEFINED);

  PROC_Properties_End();
  return 0;
}
