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
 * Module: cgexp.cxx
 * $Revision$
 * $Date$
 * $Author$
 * $Source$
 *
 * History:  extracted target-independent parts from expand.cxx
 *
 * Description:
 *
 * This file contains the internal switch-stmt of code expansion. Its interface
 * is 'Exp_OP', which takes an OP, expands it into a list of OPs which
 * are appended to the oplist passed in.
 *
 *
 * ====================================================================
 * ====================================================================
 */

#include "defs.h"
#include "config.h"
#include "erglob.h"
#include "ercg.h"
#include "glob.h"
#include "tracing.h"
#include "util.h"

#include "topcode.h"
#include "tn.h"
#include "symtab.h"
#include "opcode.h"
#include "cg.h"
#include "op.h"
#include "cgtarget.h"
#include "cgexp.h"
#include "w2op.h"

BOOL Trace_Exp = FALSE;	            /* General code expansion trace */

/* ====================================================================
 *   Expand_OP
 *
 *   Given an operator, expand it.  There are two possibilities:
 *
 *    1)  A machine operator requires no further expansion.
 *
 *    2)  An OP_* operator is expanded into a sequence of 
 *        TOP_* instructions.
 * ====================================================================
 */
static void
Expand_OP (
  OPCODE opcode, 
  TN *result, 
  TN *op1, 
  TN *op2, 
  TN *op3, 
  VARIANT variant, 
  OPS *ops
)
{
  TYPE_ID desc = OPCODE_desc(opcode);
  TYPE_ID rtype = OPCODE_rtype(opcode);

  switch (OPCODE_operator(opcode)) {
  case OPR_TRUEBR:
  case OPR_FALSEBR:
	Expand_Branch (op1, op2, op3, variant, ops);
	break;
  case OPR_GOTO:
	Expand_Branch (op1, op2, op3, V_BR_ALWAYS, ops);
	break;
  case OPR_LDA:
	Expand_Lda (result, op1, ops);
	break;
  case OPR_LDA_LABEL:
	Expand_Lda_Label (result, op1, ops);
	break;
  case OPR_INTCONST:
	Expand_Immediate (result, op1, rtype, ops);
	break;
  case OPR_CONST:
	Expand_Const (result, op1, rtype, ops);
	break;
    case OPR_ADD:
      Expand_Add (result, op1, op2, rtype, ops);
      break;
    case OPR_SUB:
      Expand_Sub (result, op1, op2, rtype, ops);
      break;
  case OPR_NEG:
	Expand_Neg (result, op1, rtype, ops);
	break;
  case OPR_SHL:
	Expand_Shift (result, op1, op2, rtype, shift_left, ops);
	break;
  case OPR_ASHR:
	Expand_Shift (result, op1, op2, rtype, shift_aright, ops);
	break;
  case OPR_LSHR:
	Expand_Shift (result, op1, op2, rtype, shift_lright, ops);
	break;
  case OPR_ILOAD:
  case OPR_LDID:
	if ( variant != 0 ) {
		Expand_Misaligned_Load ( opcode, result, op1, op2, variant, ops);
	}
	else {
		Expand_Load (opcode, result, op1, op2, ops);
	}
	break;
  case OPR_ISTORE:
  case OPR_STID:
    if (variant != 0) {
      Expand_Misaligned_Store (desc, op1, op2, op3, variant, ops);
    }
    else {
      Expand_Store (desc, op1, op2, op3, ops);
    }
    break;
  case OPR_ABS:
	Expand_Abs (result, op1, rtype, ops);
	break;
#ifdef TARG_ST100
	FmtAssert(FALSE, ("Expand_OP:  MPY shouldn't have been reached"));
#else
    case OPR_MPY:
      Expand_Multiply (result, op1, op2, rtype, ops);
      break;
#endif
  case OPR_HIGHMPY:
	Expand_High_Multiply (result, op1, op2, rtype, ops);
	break;
  case OPR_REM:
	Expand_Rem (result, op1, op2, rtype, ops);
	break;
  case OPR_MOD:
	if (MTYPE_is_signed(rtype))
		Expand_Mod (result, op1, op2, rtype, ops);
	else
		// unsigned MOD acts like REM
		Expand_Rem (result, op1, op2, rtype, ops);
	break;
  case OPR_DIV:
      Expand_Divide (result, op1, op2, rtype, ops);
      break;
  case OPR_DIVREM:
	Expand_DivRem(result, op1, op2, op3, rtype, ops);
	break;
  case OPR_SQRT:
	Expand_Sqrt (result, op1, rtype, ops);
	break;
  case OPR_LNOT:
	Expand_Logical_Not (result, op1, variant, ops);
	break;
  case OPR_LAND:
	Expand_Logical_And (result, op1, op2, variant, ops);
	break;
  case OPR_LIOR:
	Expand_Logical_Or (result, op1, op2, variant, ops);
	break;
  case OPR_BNOT:
	Expand_Binary_Complement (result, op1, rtype, ops);
	break;
  case OPR_BAND:
	Expand_Binary_And (result, op1, op2, rtype, ops);
	break;
  case OPR_BIOR:
	Expand_Binary_Or (result, op1, op2, rtype, ops);
	break;
  case OPR_BXOR:
	Expand_Binary_Xor (result, op1, op2, rtype, ops);
	break;
  case OPR_BNOR:
	Expand_Binary_Nor (result, op1, op2, rtype, ops);
	break;
  case OPR_LT:
	if (MTYPE_is_float(desc))
		Expand_Float_Less (result, op1, op2, variant, desc, ops);
	else
		Expand_Int_Less (result, op1, op2, desc, ops);
	break;
  case OPR_LE:
	if (MTYPE_is_float(desc))
		Expand_Float_Less_Equal (result, op1, op2, variant, desc, ops);
	else
		Expand_Int_Less_Equal (result, op1, op2, desc, ops);
	break;
  case OPR_EQ:
	if (MTYPE_is_float(desc))
		Expand_Float_Equal (result, op1, op2, variant, desc, ops);
	else if (desc == MTYPE_B)
		Expand_Bool_Equal (result, op1, op2, ops);
	else
		Expand_Int_Equal (result, op1, op2, desc, ops);
	break;
  case OPR_NE:

    if (MTYPE_is_float(desc))
      Expand_Float_Not_Equal (result, op1, op2, variant, desc, ops);
    else if (MTYPE_is_class_pointer(desc)) {
      Expand_Ptr_Not_Equal (result, op1, op2, desc, ops);
    }
    else if (desc == MTYPE_B)
      Expand_Bool_Not_Equal (result, op1, op2, ops);
    else
      Expand_Int_Not_Equal (result, op1, op2, desc, ops);
    break;

  case OPR_GE:
	if (MTYPE_is_float(desc))
		Expand_Float_Greater_Equal (result, op1, op2, variant, desc, ops);
	else
		Expand_Int_Greater_Equal (result, op1, op2, desc, ops);
	break;
  case OPR_GT:
	if (MTYPE_is_float(desc))
		Expand_Float_Greater (result, op1, op2, variant, desc, ops);
	else
		Expand_Int_Greater (result, op1, op2, desc, ops);
	break;

  case OPR_CVTL:
    	Expand_Convert_Length (result, 
                     op1, op2, rtype, MTYPE_is_signed(rtype), ops);
    break;

  case OPR_CVT:
    Is_True(rtype != MTYPE_B, ("conversion to bool unsupported"));
    Expand_Convert (result, op1, op2, rtype, desc, ops);
    break;

  case OPR_RND:
	Expand_Float_To_Int_Round (result, op1, rtype, desc, ops);
	break;
  case OPR_TRUNC:
	Expand_Float_To_Int_Trunc (result, op1, rtype, desc, ops);
	break;
  case OPR_CEIL:
	Expand_Float_To_Int_Ceil (result, op1, rtype, desc, ops);
	break;
  case OPR_FLOOR:
	Expand_Float_To_Int_Floor (result, op1, rtype, desc, ops);
	break;
  case OPR_MIN:
	Expand_Min (result, op1, op2, rtype, ops);
	break;
  case OPR_MAX:
	Expand_Max (result, op1, op2, rtype, ops);
	break;

  case OPR_MINMAX:
	Expand_MinMax (result, op1, op2, op3, rtype, ops);
	break;

  case OPR_SELECT:
	Expand_Select (result, op1, op2, op3, rtype, V_select_uses_fcc(variant), ops);
	break;

#ifdef TARG_ST100
    case OPR_MADD:
      FmtAssert(FALSE, 
	   ("Expand_OP:  MADD shouldn't have been reached"));
      break;
#else
    case OPR_MADD:
#endif
    case OPR_NMADD:
    case OPR_MSUB:
    case OPR_NMSUB:
    case OPR_RECIP:
    case OPR_RSQRT:
      FmtAssert(FALSE, 
	   ("Expand_OP:  unimplemented opcode %s", OPCODE_name(opcode)));
      break;

    default:
      FmtAssert(FALSE, 
	   ("Expand_OP:  unexpected opcode %s", OPCODE_name(opcode)));
      break;
  }
}

/* ======================================================================
 *   Exp_OP
 * ======================================================================
 */
void 
Exp_OP (
  OPCODE opcode, 
  TN *result, 
  TN *op1, 
  TN *op2, 
  TN *op3, 
  VARIANT variant, 
  OPS *ops
)
{
  TOP top = OPCODE_To_TOP (opcode);

  if (Trace_Exp) {
    #pragma mips_frequency_hint NEVER
    fprintf(TFile, "expand %s: ", OPCODE_name(opcode)); 
    if (result) Print_TN(result,FALSE);
    fprintf(TFile, " :- ");
    if (op1) Print_TN(op1,FALSE);
    fprintf(TFile, " ");
    if (op2) Print_TN(op2,FALSE);
    fprintf(TFile, " ");
    if (op3) Print_TN(op3,FALSE);
    fprintf(TFile, " ");
    if (variant) fprintf(TFile, "(%llx)", variant);
    fprintf(TFile, "\n");
  }

  if (top != TOP_UNDEFINED) {
    TN *args[5];
    OP *op;
    INT i = 0;

    // Build up a list of arguments to Mk_OP. The code here
    // relies on the fact that Mk_OP will ignore unnecessary
    // trailing arguments.
    if (result) args[i++] = result;
    if (TOP_is_predicated(top)) {
      Is_True(OP_PREDICATE_OPND == 0, ("predicate operand is not 0"));
      args[i++] = True_TN;
    }
    args[i++] = op1;
    args[i++] = op2;
    args[i] = op3;
    op = Mk_OP(top, args[0], args[1], args[2], args[3], args[4]);

    /* Add the new OPs to the end of the list passed in */
    OPS_Append_Op(ops, op);
    if (Trace_Exp) {
      #pragma mips_frequency_hint NEVER
      fprintf(TFile, " into "); Print_OP (op);
    }
  }
  else {
    OPS new_ops;
    OPS_Init(&new_ops);
    Expand_OP (opcode, result, op1, op2, op3, variant, &new_ops);
    if (Trace_Exp) {
      #pragma mips_frequency_hint NEVER
      OP *op;
      FOR_ALL_OPS_OPs (&new_ops, op) {
	fprintf(TFile, " into "); Print_OP (op);
      }
    }
    /* check if there is at least one OP in the expansion */
    if (OPS_first(&new_ops) != NULL) {
      /* Add the new OPs to the end of the list passed in */
      OPS_Append_Ops(ops, &new_ops);
    }
  }
}

/* ====================================================================
 *   EXP_Init ()
 *
 *   Initialize the tracing flags for the expander.
 * ====================================================================
 */
void
EXP_Init (void)
{
  static BOOL Initialized = FALSE;

  // per PU:
  Trace_Exp = Get_Trace (TP_CGEXP, LAI_EXPAND);
  /* whirl2ops uses -ttexp:2 */

  if (Initialized) return;
  Initialized = TRUE;
  // once per file:
  Initialize_Branch_Variants();
}
