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
 *   NOTE: should only be included by ../cg/register.h
 * ====================================================================
 */

#ifndef targ_register_INCLUDED
#define targ_register_INCLUDED

inline char* ISA_REGISTER_CLASS_ASM_Name (ISA_REGISTER_CLASS rc) {
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

inline BOOL Is_Predicate_REGISTER_CLASS(ISA_REGISTER_CLASS rclass) {
  return FALSE;
}

inline BOOL REGISTER_Has_Stacked_Registers(ISA_REGISTER_CLASS rclass) {
  return FALSE;
}

inline BOOL REGISTER_Has_Rotating_Registers(ISA_REGISTER_CLASS rclass) {
  return FALSE;
}

// Declare target-specific register-class, register pairs:
extern CLASS_REG_PAIR		CLASS_REG_PAIR_gr;
#define REGISTER_gr		CLASS_REG_PAIR_reg(CLASS_REG_PAIR_gr)
#define REGISTER_CLASS_gr	CLASS_REG_PAIR_rclass(CLASS_REG_PAIR_gr)
#define CLASS_AND_REG_gr	CLASS_REG_PAIR_class_n_reg(CLASS_REG_PAIR_gr)


#endif /* targ_register_INCLUDED */
