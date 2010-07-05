/*

  Copyright (C) 2010 ST Microelectronics, Inc.  All Rights Reserved.

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

  Contact information:  

  For further information regarding this notice, see:

*/


/* ====================================================================
 * ====================================================================
 *
 *  Parser functions
 *
 * ====================================================================
 * ====================================================================
 */
#include "tn.h"

#include "defs.h"
#include "flags.h"

#include "cg.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#include "air.h"
#include "air_utils.h"
#include "parser.h"
#include "type_defs.h"

#include "ext_engine.h"

#include "mempool.h"

#include "targ_isa_registers.h"
#include "targ_isa_lits.h"
#include "targ_isa_print.h"
#include "targ_abi_properties.h"
#include "ti_asm.h"

#include "label_util.h"

#include "config_TARG.h"
#include "cgtarget.h"

void
CGTARG_Generate_Asm_Ops(ASM_OP_ANNOT* asm_info,
			INT num_results, INT num_opnds,
			TN** result, TN** opnd,
			OPS *oplist, BB* Cur_BB,
			BOOL user, WN* asmparse_pragma)
{
  const WN* asm_wn;
  asm_wn = ASM_OP_wn(asm_info);

  // now create ASM op
  
  OP* asm_op = Mk_VarOP(TOP_asm, num_results, num_opnds, result, opnd);
  if (WN_Asm_Volatile(asm_wn)) {
    Set_OP_volatile(asm_op);
  }
  
  
  PU_Has_Asm = TRUE;
  OPS_Append_Op(oplist, asm_op);
  OP_MAP_Set(OP_Asm_Map, asm_op, asm_info);
  // TODO: Determine what ASM_livein/out are ?
  ASMINFO* info = TYPE_PU_ALLOC (ASMINFO);
  ISA_REGISTER_CLASS rc;
  FOR_ALL_ISA_REGISTER_CLASS(rc) {
    ASMINFO_livein(info)[rc] = REGISTER_SET_EMPTY_SET;
    ASMINFO_liveout(info)[rc] = REGISTER_SET_EMPTY_SET;
    ASMINFO_kill(info)[rc] = ASM_OP_clobber_set(asm_info)[rc];
  }
  BB_Add_Annotation(Cur_BB, ANNOT_ASMINFO, info);
}

