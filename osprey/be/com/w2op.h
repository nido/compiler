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


#ifndef	w2op_INCLUDED
#define	w2op_INCLUDED

#include "topcode.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Given a WHIRL opcode, return the corresponding TOP.
 * This will only return machine ops, 
 * else TOP_UNDEFINED if not an exact correspondence.
 */
BE_EXPORTED extern TOP OPCODE_To_TOP (OPCODE opcode);

/* Given a WHIRL opcode, return the corresponding INTRINSIC.
 * This will only return INTRINSICs for operators that have
 * them defined.
 */
BE_EXPORTED extern INTRINSIC OPCODE_To_INTRINSIC (OPCODE opcode);

#ifdef TARG_ST
/* [CG]
 * Given a WHIRL opcode and WN kids, return the corresponding INTRINSIC.
 * kids array may be modified.
 * returns the INTRINSIC id and the kids to be used as params.
 * returns INTRINSIC_INVALID if not intrinsic match the opcode.
 * In addition to OPCODE_To_INTRINSIC, this function may look at kids
 * to get a more efficient intrinsic op.
 */
BE_EXPORTED extern INTRINSIC WN_To_INTRINSIC (OPCODE opcode, WN* kids[]);
#endif

/* Given a WHIRL node, return the corresponding TOP. 
 * This will only return machine ops, 
 * else TOP_UNDEFINED if not an exact correspondence.
 * (this handles more cases than OPCODE_To_TOP, cause it can look at kids).
 */
BE_EXPORTED extern TOP WHIRL_To_TOP (WN *wn);

/* return whether MPY, DIV, or REM will be translated into shifts and adds */
BE_EXPORTED extern BOOL Can_Do_Fast_Multiply (TYPE_ID mtype, INT64 val);
BE_EXPORTED extern BOOL Can_Do_Fast_Divide (TYPE_ID mtype, INT64 val);
BE_EXPORTED extern BOOL Expand_Power_Of_Two_Div_Without_Pred (TYPE_ID mtype, INT64 val);
BE_EXPORTED extern BOOL Can_Do_Fast_Remainder (TYPE_ID mtype, INT64 val);

#ifdef TARG_ST
/* Returns whether SELECT can be translated, otherwise if will be lowered as if/then/else. */
BE_EXPORTED extern BOOL Can_Do_Select (TYPE_ID mtype);
BE_EXPORTED extern BOOL Target_Inlines_Divide (TYPE_ID mtype, INT64 val);
BE_EXPORTED extern BOOL Target_Inlines_Remainder (TYPE_ID mtype, INT64 val);
#endif

#ifndef TARG_ST
  /* [CG] Obsolete for ST targets. See LOWER_CNST_DIV/LOWER_CNST_MUL. */
/* When trying to convert a multiply or divide operation into a series
 * of shifts/adds/subtracts, there is some limit (cycles? ops?) at
 * which the conversion is not profitable.  Return that limit.
 */
BE_EXPORTED extern INT Multiply_Limit ( BOOL is_64bit, INT64 val);
BE_EXPORTED extern INT Divide_Limit ( BOOL is_64bit);
#endif

/* Return whether or not the immediate specified by <val> would be a valid
 * operand of the machine instruction generated by the whirl operator
 * <opr> with the immediate as the <whichkid> whirl operand.
 * The datatype of the operator is specified by <dtype> and if the
 * operation is an STID, <stid_st> specifies the symbol being stored to.
 */
BE_EXPORTED extern BOOL Can_Be_Immediate(OPERATOR opr,
			     INT64 val,
			     TYPE_ID dtype,
			     INT whichkid,
			     ST *stid_st);

/* determine speculative execution taking into account eagerness level
 *
 */
#ifdef TARG_ST
/* Arthur: moved to cg */
#else
extern BOOL TOP_Can_Be_Speculative (TOP opcode);
#endif
BE_EXPORTED extern BOOL WN_Can_Be_Speculative (WN *wn, struct ALIAS_MANAGER *alias);
BE_EXPORTED extern BOOL WN_Expr_Can_Be_Speculative (WN *wn, struct ALIAS_MANAGER *alias);
BE_EXPORTED extern BOOL OPCODE_Can_Be_Speculative(OPCODE opcode);

#ifdef TARG_ST
/* TRUE if the low-order word of a multi-word parameter/result should be
   passed in the lowest numbered register. */
BE_EXPORTED extern BOOL Pass_Low_First(TYPE_ID type);
#endif

#ifdef __cplusplus
}
#endif
#endif /* w2op_INCLUDED */