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


/* ====================================================================
 *
 * Module: freq.cxx
 * $Revision$
 * $Date$
 * $Author$
 * $Source$
 *
 * Description:
 *
 * Compute basic block and edge frequencies by estimating and/or
 * feedback. Based on: "Static Branch Frequency and Program Profile
 * Analysis" by Wu and Larus.
 *
 * ====================================================================
 */

#include <math.h>
#include <alloca.h>
#include <map.h>

#include "defs.h"
#include "config.h"
#include "errors.h"
#include "mempool.h"
#include "tracing.h"
#include "glob.h"
#include "config_asm.h"
#include "bitset.h"
#include "bb.h"
#include "bb_set.h"
#include "bb_map.h"
#include "region_util.h"
#include "cg_region.h"
#include "lai.h"		/* for LAI_PU_Has_Feedback */
#include "lai_flags.h"
#include "cgtarget.h"
#include "annotations.h"
#include "whirl2ops.h"
#include "note.h"
#include "label_util.h"
#include "fb_whirl.h"
#include "DaVinci.h"
#include "freq.h"

/* ====================================================================
 * ====================================================================
 *
 * Global data
 *
 * ====================================================================
 * ====================================================================
 */

BOOL FREQ_freqs_computed; // True if freqs computed for region

/* ====================================================================
 *   FREQ_Print_BB_Note
 *
 *   See interface description.
 * ====================================================================
 */
void
FREQ_Print_BB_Note(
  BB *bb,
  FILE *file
)
{
  const char *prefix = file == Asm_File ? ASM_CMNT_LINE : "";
  BBLIST *bb_succs = BB_succs(bb);
  INT bb_id = BB_id(bb);

  if (!FREQ_freqs_computed && !LAI_PU_Has_Feedback) return;

  fprintf(file, "%s<freq>\n", prefix);

  fprintf(file, "%s<freq> BB:%d frequency = %#.5f (%s)\n",
	  prefix, bb_id, BB_freq(bb),
	  (BB_freq_fb_based(bb) ? "feedback" : "heuristic"));

  /* Don't bother printing only one successor edge frequency; it's obvious
   * what it is and we don't need more clutter.
   */
  if (BBlist_Len(bb_succs) > 1) {
    BBLIST *succ;

    FOR_ALL_BBLIST_ITEMS(bb_succs, succ) {
      fprintf(file, "%s<freq> BB:%d => BB:%d probability = %#.5f\n",
	      prefix,
	      bb_id,
	      BB_id(BBLIST_item(succ)),
	      BBLIST_prob(succ));
    }
  }

  fprintf(file, "%s<freq>\n", prefix);
}

/* ====================================================================
 *   FREQ_Incorporate_Feedback
 * ====================================================================
 */
void
FREQ_Incorporate_Feedback(const WN* entry)
{
  /* dummy */
  return;
}

/* ====================================================================
 *   FREQ_Region_Initialize
 * ====================================================================
 */
void 
FREQ_Region_Initialize(void)
{
  static BOOL inited = FALSE;

  if (!inited) {
    inited = TRUE;
  }
  FREQ_freqs_computed = FALSE;
}
