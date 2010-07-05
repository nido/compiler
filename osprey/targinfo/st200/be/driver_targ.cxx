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


#include "defs.h"
#include "erglob.h"
#include "errors.h"
#include "config.h"
#include "config_target.h"
#include "config_TARG.h"
#include "ti_init.h"
#include "targ_isa_operands.h"
#include "targ_isa_print.h"
#include "ti_si.h"

/* ====================================================================
 *   Initialize_Targ_Info
 * ====================================================================
 */

BE_EXPORTED void*
Initialize_Targ_Info (void)
{
  ABI_PROPERTIES_ABI abi;
  ISA_SUBSET isa;
  PROCESSOR proc;
  void *result;

  switch (Target_ABI) {
  case ABI_ST200_embedded:
    abi = ABI_PROPERTIES_ABI_embedded;
    break;
  default:
    FmtAssert(FALSE, ("targinfo doesn't handle abi: %d\n", Target_ABI));
  }

  switch (Target_ISA) {
  case TARGET_ISA_ST220:
    isa = ISA_SUBSET_st220;
    break;
  case TARGET_ISA_ST231:
    isa = ISA_SUBSET_st231;
    break;
  case TARGET_ISA_ST240:
    isa = ISA_SUBSET_st240;
    break;
  default:
    FmtAssert(FALSE, ("targinfo doesn't handle isa: %s\n", Isa_Name(Target_ISA)));
  }

  switch (Target) {
  case TARGET_st220:
    proc = PROCESSOR_st220;
    break;
  case TARGET_st221:
    proc = PROCESSOR_st220;
    break;
  case TARGET_st231:
    proc = PROCESSOR_st231;
    break;
  case TARGET_st240:
    proc = PROCESSOR_st240;
    break;
  default:
    FmtAssert(FALSE, ("targinfo doesn't handle target: %s\n", Targ_Name(Target)));
  }

  result = TI_Initialize(abi, isa, proc, 0, NULL, Targ_Path);

  if (result) {
    // [SC] Handle scheduler cycle time overrides.
    const static struct { const char *opt; BOOL res; int num; } opts[] =
      {
	{ "result1_avail", TRUE, 0 },
	{ "result2_avail", TRUE, 1 },
	{ "result3_avail", TRUE, 2 },
	{ "result4_avail", TRUE, 3 },
	{ "result5_avail", TRUE, 4 },
	{ "op1_read", FALSE, 0 },
	{ "op2_read", FALSE, 1 },
	{ "op3_read", FALSE, 2 },
	{ "op4_read", FALSE, 3 },
	{ "op5_read", FALSE, 4 }
      };
								 
    OPTION_LIST *ol;
    for (ol = Cycle_Time_Overrides; ol != NULL; ol = OLIST_next(ol)) {
      char *opt = OLIST_opt (ol);
      char *val = strdup (OLIST_val (ol));
      char *p;
      long cycle;
      int opt_ix;
      for (opt_ix = 0; opt_ix < (sizeof(opts)/sizeof(opts[0])); opt_ix++) {
	if (strcmp (opt, opts[opt_ix].opt) == 0) {
	  break;
	}
      }
      if (opt_ix == (sizeof(opts)/sizeof(opts[0]))) {
	ErrMsg (EC_Inv_TARG, opt, "...");
	continue;
      }
      cycle = strtol (val, &val, 0);
      for (p = strtok (val, ","); p != NULL; p = strtok (NULL, ",")) {
	// Find all instructions whose mnemonic matches p.
	int plen = strlen(p);
	int i;
	BOOL found = FALSE;
	for (i = 0; i < TOP_count; i++) {
	  const char *topname = ISA_PRINT_AsmName ((TOP)i);
	  if (strcmp(topname, p) == 0) {
	    int j, start, count;
	    const char *internal_topname = TOP_Name((TOP)i);
	    found = TRUE;
	    if (opts[opt_ix].res) {
	      //printf ("TSI_Set_Result_Available_Time ( %s, %d, %ld );\n",
	      //      internal_topname, opts[opt_ix].num, cycle);
	      TSI_Set_Result_Available_Time ( (TOP)i, opts[opt_ix].num, cycle );
	    } else {
	      //printf ("TSI_Set_Operand_Access_Time ( %s, %d, %ld );\n",
	      //      internal_topname, opts[opt_ix].num, cycle);
	      TSI_Set_Operand_Access_Time ( (TOP)i, opts[opt_ix].num, cycle );
	    }
	  }
	}
	if (!found) {
	  ErrMsg (EC_Inv_TARG, opt, p);
	}
      }
    }
  }

  return result;
}

