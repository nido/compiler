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
 * Target specific register class routines:
 *
 *      BOOL Is_Predicate_REGISTER_CLASS(ISA_REGISTER_CLASS rclass)
 *          Returns TRUE if the rclass is the predicate register class
 *
 *  Special purpose registers:
 *
 *	CLASS_REG_PAIR     CLASS_REG_PAIR_xxx;
 *      REGISTER           REGISTER_xxx
 *	ISA_REGISTER_CLASS REGISTER_CLASS_xxx
 *	mINT16             CLASS_AND_REG_xxx
 *
 *	    Registers which have a special purpose in the ABI or hardware.
 *	    'xxx' is one of the following:
 *
 *		zero - the read zero / write sink register
 *		ep   - entry point
 *		gp   - global pointer
 *		sp   - stack pointer
 *		fp   - frame pointer
 *		ra   - return address
 *		v0   - integer function return value
 *		undef- undefined class and undefined register
 *
 *	ISA_REGISTER_CLASS Register_Class_For_Mtype(
 *	    TYPE_ID mtype
 *	)
 *
 *	    Given an MTYPE, return the corresponding register class
 *	    used to hold values of that type. Return 
 *	    ISA_REGISTER_CLASS_UNDEFINED if there is no correspondence.
 *
 *
 *      REGISTER_SET REGISTER_CLASS_callee_saves(
 *          ISA_REGISTER_CLASS rclass
 *      )
 *      REGISTER_SET REGISTER_CLASS_caller_saves(
 *          ISA_REGISTER_CLASS rclass
 *      )
 *      REGISTER_SET REGISTER_CLASS_function_argument(
 *          ISA_REGISTER_CLASS rclass
 *      )
 *      REGISTER_SET REGISTER_CLASS_function_value(
 *          ISA_REGISTER_CLASS rclass
 *      )
 *      REGISTER_SET REGISTER_CLASS_shrink_wrap(
 *          ISA_REGISTER_CLASS rclass
 *      )
 *      REGISTER_SET REGISTER_CLASS_stacked(
 *          ISA_REGISTER_CLASS rclass
 *      )
 *      REGISTER_SET REGISTER_CLASS_rotating(
 *          ISA_REGISTER_CLASS rclass
 *      )
 *
 *          Returns the REGISTER_SET within the given 'rclass' of a
 *          particular type.
 *
 *  CLASS_REG_PAIR utilities
 *	
 *	BOOL CLASS_REG_PAIR_EqualP(
 *	    CLASS_REG_PAIR crp1,
 *	    CLASS_REG_PAIR crp2
 *	)
 *
 *	    Determine if the two registers are equal
 *
 *   NOTE: should only be included by ../cg/register.h
 * ====================================================================
 */

#ifndef register_targ_INCLUDED
#define register_targ_INCLUDED

extern char *ISA_REGISTER_CLASS_symbol[];
#define ISA_REGISTER_CLASS_Symbol(rc) (ISA_REGISTER_CLASS_symbol[rc])

inline char* _ISA_REGISTER_CLASS_ASM_Name (ISA_REGISTER_CLASS rc) {
  switch (rc) {
  case ISA_REGISTER_CLASS_du: return "dreg";
  case ISA_REGISTER_CLASS_au: return "areg";
  case ISA_REGISTER_CLASS_guard: return "breg";
  case ISA_REGISTER_CLASS_loop: return "loop";
  case ISA_REGISTER_CLASS_control: return "control";
  default:
    FmtAssert(FALSE, ("ISA_REGISTER_CLASS_ASM_Name: rclass"));
  }
}

inline BOOL _Is_Predicate_REGISTER_CLASS(ISA_REGISTER_CLASS rclass) {
  return FALSE;
}

inline BOOL _REGISTER_Has_Stacked_Registers(ISA_REGISTER_CLASS rclass) {
  return FALSE;
}

inline BOOL _REGISTER_Has_Rotating_Registers(ISA_REGISTER_CLASS rclass) {
  return FALSE;
}

inline BOOL _REGISTER_Is_Rotating(ISA_REGISTER_CLASS rclass, REGISTER reg)
{
  return FALSE;
}

inline REGISTER_SET _REGISTER_Get_Requested_Rotating_Registers (
  ISA_REGISTER_CLASS rclass
)
{
  return REGISTER_SET_EMPTY_SET;
}

// Declare target-specific register-class, register pairs:
extern CLASS_REG_PAIR		CLASS_REG_PAIR_gr;
#define REGISTER_gr		CLASS_REG_PAIR_reg(CLASS_REG_PAIR_gr)
#define REGISTER_CLASS_gr	CLASS_REG_PAIR_rclass(CLASS_REG_PAIR_gr)
#define CLASS_AND_REG_gr	CLASS_REG_PAIR_class_n_reg(CLASS_REG_PAIR_gr)


#endif /* register_targ_INCLUDED */
