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
 * ====================================================================
 *
 *       OP utility routines which include target dependencies.
 *
 * ====================================================================
 * ====================================================================
 */

#include "defs.h"
#include "config.h"
#include "erglob.h"
#include "xstats.h"
#include "tracing.h"

#include "cgir.h"

/* ====================================================================
 *   CGTARG_Predicate_OP
 * ====================================================================
 */
void
CGTARG_Predicate_OP (
  BB* bb, 
  OP* op, 
  TN* pred_tn
)
{
  if (OP_has_predicate(op)) {
    Set_OP_opnd(op, OP_PREDICATE_OPND, pred_tn);
  }
}

/* ====================================================================
 *   CGTARG_Copy_Operand
 *
 *   TODO: generate automatically ?? at leats some obvious ones
 *         coherently with the isa property ?
 * ====================================================================
 */
INT 
CGTARG_Copy_Operand (
  OP *op
)
{
  TOP opr = OP_code(op);
  switch (opr) {

  // NOTE: TOP_fandcm, TOP_for, and TOP_fxor could be handled like
  // their integer counterparts should that ever become useful.

  case TOP_GP32_ADD_GT_DR_DR_U8:
    //case TOP_or:
    //case TOP_xor:
    //case TOP_sub:
    //case TOP_shl_i:
    //case TOP_shr_i:
    if (TN_has_value(OP_opnd(op,2)) && TN_value(OP_opnd(op,2)) == 0) {
      return 1;
    }
    break;

  case TOP_GP32_COPYA_GT_AR_DR:
  case TOP_GP32_COPYC_GT_CRL_DR:
  case TOP_GP32_COPYD_GT_DR_AR:

  case TOP_GP32_MOVEA_GT_AR_AR:
  case TOP_GP32_MOVEG_GT_BR_BR:
  case TOP_GP32_MOVE_GT_DR_DR:
    //TOP_GP32_MOVEHH_GT_DR_DR,
    //TOP_GP32_MOVEHL_GT_DR_DR,
    //TOP_GP32_MOVELH_GT_DR_DR,
    //TOP_GP32_MOVELL_GT_DR_DR,
    //TOP_GP32_MOVEP_GT_DR_DR,
    return 1;

  }
  return -1;
}

/* ====================================================================
 *   CGTARG_Init_OP_cond_def_kind
 * ====================================================================
 */
void 
CGTARG_Init_OP_cond_def_kind (
  OP *op
)
{
  TOP top = OP_code(op);
  switch (top) {

    // The following OPs unconditionally define the predicate results.
    //
  case TOP_noop:

    Set_OP_cond_def_kind(op, OP_ALWAYS_UNC_DEF);
    break;

    // The following OPs do not always update the result predicates.
  case TOP_label:

    Set_OP_cond_def_kind(op, OP_ALWAYS_COND_DEF);
    break;

  default:
    if (OP_has_predicate(op))
      Set_OP_cond_def_kind(op, OP_PREDICATED_DEF);
    else
      Set_OP_cond_def_kind(op, OP_ALWAYS_UNC_DEF);
    break;
  }
}

#if !defined(GHS_SHIT_IS_WORKING)
/* ====================================================================
 *   RegMask_pretty
 *
 *   Funsciton borrowed from LAO to hack until the GHS crap as can take
 *   the ST100 instructions. probably forever ...
 * ====================================================================
 */
static void RegMask_pretty (
  INT64 mask, 
  BOOL push,      /* is it a push ? if not it's a pop */
  FILE *file
)
{
  INT i;
  INT rl = -1, rh;
  ISA_REGISTER_CLASS rc = ISA_REGISTER_CLASS_UNDEFINED;
  BOOL first = TRUE;

  // push/pop operand is a u20
  for (i = 0; i < 20; i++) {
    INT r1, r2;
    ISA_REGISTER_CLASS cl;

    // check whether the bit is set
    if (!(mask & (1 << i))) continue;

    // find out which ISA_REGISTER_CLASS and REG it corresponds to:
    if (push) {
      switch (i) {
      case 0:
	r1 = 0; r2 = 1;
	cl = ISA_REGISTER_CLASS_du;
	break;
      case 1:
	r1 = 2; r2 = 3;
	cl = ISA_REGISTER_CLASS_du;
	break;
      case 2:
	r1 = 4; r2 = 5;
	cl = ISA_REGISTER_CLASS_du;
	break;
      case 3:
	r1 = 6; r2 = 7;
	cl = ISA_REGISTER_CLASS_du;
	break;
      case 4:
	r1 = 8; r2 = 9;
	cl = ISA_REGISTER_CLASS_du;
	break;
      case 5:
	r1 = 10; r2 = 11;
	cl = ISA_REGISTER_CLASS_du;
	break;
      case 6:
	r1 = 12; r2 = 13;
	cl = ISA_REGISTER_CLASS_du;
	break;
      case 7:
	r1 = 14; r2 = 15;
	cl = ISA_REGISTER_CLASS_du;
	break;
      case 8:
	r1 = 0; r2 = 0;
	cl = ISA_REGISTER_CLASS_control;
	break;
      case 9:
	r1 = 0; r2 = 0;
	cl = ISA_REGISTER_CLASS_loop;
	break;
      case 10:
	r1 = 1; r2 = 1;
	cl = ISA_REGISTER_CLASS_loop;
	break;
      case 11:
	r1 = 2; r2 = 2;
	cl = ISA_REGISTER_CLASS_loop;
	break;
      case 12:
	r1 = 0; r2 = 1;
	cl = ISA_REGISTER_CLASS_au;
	break;
      case 13:
	r1 = 2; r2 = 3;
	cl = ISA_REGISTER_CLASS_au;
	break;
      case 14:
	r1 = 4; r2 = 5;
	cl = ISA_REGISTER_CLASS_au;
	break;
      case 15:
	r1 = 6; r2 = 7;
	cl = ISA_REGISTER_CLASS_au;
	break;
      case 16:
	r1 = 8; r2 = 9;
	cl = ISA_REGISTER_CLASS_au;
	break;
      case 17:
	r1 = 10; r2 = 11;
	cl = ISA_REGISTER_CLASS_au;
	break;
      case 18:
	r1 = 12; r2 = 13;
	cl = ISA_REGISTER_CLASS_au;
	break;
      case 19:
	r1 = 14; r2 = 15;
	cl = ISA_REGISTER_CLASS_au;
	break;
      }
    }
    else { // pop
      switch (i) {
      case 19:
	r1 = 0; r2 = 1;
	cl = ISA_REGISTER_CLASS_du;
	break;
      case 18:
	r1 = 2; r2 = 3;
	cl = ISA_REGISTER_CLASS_du;
	break;
      case 17:
	r1 = 4; r2 = 5;
	cl = ISA_REGISTER_CLASS_du;
	break;
      case 16:
	r1 = 6; r2 = 7;
	cl = ISA_REGISTER_CLASS_du;
	break;
      case 15:
	r1 = 8; r2 = 9;
	cl = ISA_REGISTER_CLASS_du;
	break;
      case 14:
	r1 = 10; r2 = 11;
	cl = ISA_REGISTER_CLASS_du;
	break;
      case 13:
	r1 = 12; r2 = 13;
	cl = ISA_REGISTER_CLASS_du;
	break;
      case 12:
	r1 = 14; r2 = 15;
	cl = ISA_REGISTER_CLASS_du;
	break;
      case 11:
	r1 = 0; r2 = 0;
	cl = ISA_REGISTER_CLASS_control;
	break;
      case 10:
	r1 = 0; r2 = 0;
	cl = ISA_REGISTER_CLASS_loop;
	break;
      case 9:
	r1 = 1; r2 = 1;
	cl = ISA_REGISTER_CLASS_loop;
	break;
      case 8:
	r1 = 2; r2 = 2;
	cl = ISA_REGISTER_CLASS_loop;
	break;
      case 7:
	r1 = 0; r2 = 1;
	cl = ISA_REGISTER_CLASS_au;
	break;
      case 6:
	r1 = 2; r2 = 3;
	cl = ISA_REGISTER_CLASS_au;
	break;
      case 5:
	r1 = 4; r2 = 5;
	cl = ISA_REGISTER_CLASS_au;
	break;
      case 4:
	r1 = 6; r2 = 7;
	cl = ISA_REGISTER_CLASS_au;
	break;
      case 3:
	r1 = 8; r2 = 9;
	cl = ISA_REGISTER_CLASS_au;
	break;
      case 2:
	r1 = 10; r2 = 11;
	cl = ISA_REGISTER_CLASS_au;
	break;
      case 1:
	r1 = 12; r2 = 13;
	cl = ISA_REGISTER_CLASS_au;
	break;
      case 0:
	r1 = 14; r2 = 15;
	cl = ISA_REGISTER_CLASS_au;
	break;
      }
    }

    // Now, it depends on ISA_REGISTER_CLASS
    if (cl == ISA_REGISTER_CLASS_du) {
      if (!first) fprintf (file, ", ");
      fprintf(file, "r%d-r%d",r1,r2);
      first = false;
    }
    else if (cl == ISA_REGISTER_CLASS_au) {
      if (!first) fprintf (file, ", ");
      fprintf(file, "p%d-p%d",r1,r2);
      first = false;
    }
    else if (cl == ISA_REGISTER_CLASS_loop) {
      if (!first) fprintf (file, ", ");
      fprintf(file, "LR%d", r1);
      first = false;
    }
    else if (cl == ISA_REGISTER_CLASS_control) {
      if (!first) fprintf (file, ", ");
      fprintf(file, "GFR");
      first = false;
    }
    else {
      FmtAssert(FALSE,("Hack_For_Printing_Push_Pop: unknown rclass"));
    }
  }
}

BOOL
Hack_For_Printing_Push_Pop (
  OP *op,
  FILE *file
)
{

  if (OP_code(op) == TOP_GP32_PUSH_U20 || OP_code(op) == TOP_GP32_POP_U20) {
    fputc ('\t', file);
    if (OP_code(op) == TOP_GP32_PUSH_U20) {
      fprintf (file, " push ");
      RegMask_pretty(TN_value(OP_opnd(op,0)), TRUE, file);
    }
    else if (OP_code(op) == TOP_GP32_POP_U20) {
      fprintf (file, " pop ");
      RegMask_pretty(TN_value(OP_opnd(op,0)), FALSE, file);
    }	
    fputc ('\n', file);
    return TRUE;
  }

  return FALSE;
}
#endif
