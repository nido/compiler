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

#ifndef targ_cg_INCLUDED
#define targ_cg_INCLUDED

#include "defs.h"

/* ====================================================================
 *   Branch handling:
 * ====================================================================
 */
#define MIN_BRANCH_DISP             (2097151 * ISA_INST_BYTES)
#define DEFAULT_BRP_BRANCH_LIMIT    (256 * ISA_INST_BYTES)
#define DEFAULT_LONG_BRANCH_LIMIT   (2000000 * ISA_INST_BYTES)

#if 0
// Return format string for operand i of op.
inline char *ISA_PRINT_PREDICATE (ISA_OPERAND_USE use) {
  FmtAssert(FALSE,("predication not supported"));
  return NULL;
}
#endif

// Given a conditional branch with a <branch_taken_probability>
// return TRUE if it would be beneficial to convert it to a brlikely.
inline BOOL 
CGTARG_Use_Brlikely(float branch_taken_probability)
{
  return FALSE;
}

#if 0
/* ====================================================================
 *    Predication:
 * ====================================================================
 */
inline BOOL CGTARG_Can_Predicate_Calls() { return FALSE; }
inline BOOL CGTARG_Can_Predicate_Returns() { return FALSE; }
inline BOOL CGTARG_Can_Predicate_Branches() { return FALSE; }
inline BOOL CGTARG_Can_Predicate() { return FALSE; }
#endif

/* ====================================================================
 *    ISA properties:
 * ====================================================================
 */

/* ====================================================================
 *    ASM:
 * ====================================================================
 */

inline const char*
CGTARG_Top_Name(TOP opr)
{
  return ISA_PRINT_AsmName(opr);
}

/* call init routine once per asm stmt */
extern void CGTARG_Init_Asm_Constraints(void);

/* Given a constraint for an ASM parameter, and the load of the matching
 * argument passed to ASM (possibly NULL), choose an appropriate TN for it
 */
extern TN* CGTARG_TN_For_Asm_Operand(const char* constraint, 
                                     const WN* load,
                                     TN* pref_tn,
                                     ISA_REGISTER_SUBCLASS* subclass);
/* given asm constraint and mtype, 
 * pick appropriate dedicated tn and string name */
extern void CGTARG_TN_And_Name_For_Asm_Constraint (char *constraint, 
                                                   TYPE_ID rtype, 
                                                   TYPE_ID desc,
                                                   TN **tn, 
                                                   char **name);

/* --------------------------------------------------------------------
 *   target specific modifiers for printing different versions
 *   of register names when they appear as AM operands.
 * --------------------------------------------------------------------
 */
extern char CGTARG_Asm_Opnd_Modifiers[];
extern INT  CGTARG_Num_Asm_Opnd_Modifiers;

extern const char* CGTARG_Modified_Asm_Opnd_Name(char, TN*, char *);

extern char* CGTARG_Modify_Asm_String (char* asm_string, 
			      UINT32 position, bool memory, TN* tn);

/* --------------------------------------------------------------------
 *   may have to clean up the asm string 
 * --------------------------------------------------------------------
 */
extern void CGTARG_Postprocess_Asm_String (char*);

/* ====================================================================
 *                    Code Generation stuff
 * ====================================================================
 */

extern TOP CGTARG_Invert_Table[TOP_count+1];

extern mTOP CGTARG_Inter_RegClass_Copy_Table
    [ISA_REGISTER_CLASS_MAX+1][ISA_REGISTER_CLASS_MAX+1][2];

/* --------------------------------------------------------------------
 *    Return the inverse of <opr>. TOP_UNDEFINED is returned
 *    if there is no inverse. Note that the inverse of an
 *    instruction that takes an immediate does not also necessarily
 *    take an immediate. 
 * --------------------------------------------------------------------
 */
inline 
TOP CGTARG_Invert(TOP opr)
{
  return CGTARG_Invert_Table[(INT)opr];
}

inline INT
CGTARG_Text_Alignment (void)
{
  return ISA_INST_BYTES;
}

/* --------------------------------------------------------------------
 *    Returns the copy instruction for moving data from register
 *    class <src> to <dst> in single or double precision according
 *    to <is_double>. Returns TOP_UNDEFINED if there is no
 *    such instruction.
 * --------------------------------------------------------------------
 */
inline TOP CGTARG_Inter_RegClass_Copy(ISA_REGISTER_CLASS dst,
				      ISA_REGISTER_CLASS src,
				      BOOL is_double)
{
  return (TOP)CGTARG_Inter_RegClass_Copy_Table[src][dst][is_double];
}

inline TN *CGTARG_gen_trip_count_TN (INT32 trip_size) 
{ 
  return Gen_Register_TN(ISA_REGISTER_CLASS_integer, trip_size);
}

inline BOOL
CGTARG_Has_Branch_Predict(void)
{
  return FALSE;
}

inline BOOL
CGTARG_Have_Indexed_Mem_Insts(void)
{
  return FALSE;
}

#endif /* targ_cg_INCLUDED */
