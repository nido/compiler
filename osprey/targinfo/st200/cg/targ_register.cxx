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


#define INCLUDING_IN_REGISTER // we modify register classes and we're
			      // really part of the register implementation

#include "defs.h"
#include "errors.h"
#include "tracing.h"
#include "mempool.h"
#include "config.h"
#include "config_TARG.h"
#include "glob.h"
#include "util.h"
#include "data_layout.h"
#include "tn.h"
#include "targ_sim.h"
#include "register.h"
#include "op.h"
#include "cg_flags.h"
#include "calls.h"
#include "cgtarget.h"
#include "reg_live.h"

#include "targ_isa_registers.h"
#include "targ_abi_properties.h"
#include "targ_isa_operands.h"

/* 
 * TODO: should be generated together with targ_isa_register.[hc] 
 */
char *ISA_REGISTER_CLASS_symbol[] = {
  "",   /* ISA_REGISTER_CLASS_UNDEFINED */
  "_d",  /* ISA_REGISTER_CLASS_du */
  "_a",  /* ISA_REGISTER_CLASS_au */
  "_b",  /* ISA_REGISTER_CLASS_guard */
  "_c",  /* ISA_REGISTER_CLASS_control */
  "_l"   /* ISA_REGISTER_CLASS_loop */
};

static INT num_output_parameters;
static INT num_caller;

mISA_REGISTER_CLASS Mtype_RegClass_Map[MTYPE_LAST+1];

/* ====================================================================
 *   Init_Mtype_RegClass_Map
 * ====================================================================
 */
void 
Init_Mtype_RegClass_Map ()
{
  INT i;
  mISA_REGISTER_CLASS * const map = Mtype_RegClass_Map;

  for (i = 0; i <= MTYPE_LAST; ++i) 
    map[i] = ISA_REGISTER_CLASS_UNDEFINED;

  map[MTYPE_B] = ISA_REGISTER_CLASS_branch;
  map[MTYPE_I1] = ISA_REGISTER_CLASS_integer;
  map[MTYPE_I2] = ISA_REGISTER_CLASS_integer;
  map[MTYPE_I4] = ISA_REGISTER_CLASS_integer;
  map[MTYPE_U1] = ISA_REGISTER_CLASS_integer;
  map[MTYPE_U2] = ISA_REGISTER_CLASS_integer;
  map[MTYPE_U4] = ISA_REGISTER_CLASS_integer;
  map[MTYPE_A4] = ISA_REGISTER_CLASS_integer;
  map[MTYPE_F4] = ISA_REGISTER_CLASS_integer;

  if (Enable_64_Bits_Ops) {
    map[MTYPE_F8] = ISA_REGISTER_CLASS_integer;
    map[MTYPE_I8] = ISA_REGISTER_CLASS_integer;
    map[MTYPE_U8] = ISA_REGISTER_CLASS_integer;
  }
  return;
}

/*
 * Target-specific class register pairs
 */

/* ====================================================================
 *  CGTARG_Initialize_Register_Class
 *
 *  Initialize the register class 'rclass'. A register class may be
 *  intialized multiple times.
 * ====================================================================
 */
void
CGTARG_Initialize_Register_Class(
  ISA_REGISTER_CLASS rclass
)
{
  /* There are multiple integer return regs -- v0 is the lowest
   * of the set.
   */
  if (rclass == ISA_REGISTER_CLASS_integer) {
    Set_CLASS_REG_PAIR_reg(CLASS_REG_PAIR_v0, 
	     REGISTER_SET_Choose(REGISTER_CLASS_function_value(rclass)));
    Set_CLASS_REG_PAIR_rclass(CLASS_REG_PAIR_v0, rclass);
  }

  // Initialize target-specific class-reg pairs:

  return;
}

/* ====================================================================
 *   CGTARG_Regname_Register_Class
 *
 *   given register name (asm), get its register class.
 * ====================================================================
 */
ISA_REGISTER_CLASS 
CGTARG_Regname_Register_Class (
  char *regname
) 
{
  ISA_REGISTER_CLASS rclass = ISA_REGISTER_CLASS_UNDEFINED;

  switch (regname[0]) {
  case 'r':
    rclass = ISA_REGISTER_CLASS_integer;
    break;
  case 'b':
    rclass = ISA_REGISTER_CLASS_branch;
    break;
  }

  return rclass;
}

/* ====================================================================
 *   REGISTER_Is_Rotating
 * ====================================================================
 */
BOOL 
REGISTER_Is_Rotating(
  ISA_REGISTER_CLASS rclass, 
  REGISTER reg
)
{
  return FALSE;
}

/* ====================================================================
 *   REGISTER_Get_Requested_Rotating_Registers
 * ====================================================================
 */
REGISTER_SET 
REGISTER_Get_Requested_Rotating_Registers (
  ISA_REGISTER_CLASS rclass
)
{
  return REGISTER_SET_EMPTY_SET;
}

/* ====================================================================
 *   CGTARG_Forbidden_GRA_Registers 
 * ====================================================================
 */
REGISTER_SET 
CGTARG_Forbidden_GRA_Registers (ISA_REGISTER_CLASS rclass)
{
  REGISTER_SET s = REGISTER_SET_EMPTY_SET;
  if (Is_Target_st235 ()) {
    if (rclass == ISA_REGISTER_CLASS_integer) {
      return REGISTER_SET_Union1(s, TN_register(RA_TN));
    }
  }
  return s;
}

REGISTER_SET
CGTARG_Forbidden_LRA_Registers (ISA_REGISTER_CLASS rclass)
{
  return CGTARG_Forbidden_GRA_Registers (rclass);
}

#ifdef TARG_ST
/* ====================================================================
 *   CGTARG_Prefered_GRA_Registers 
 * ====================================================================
 */
REGISTER_SET
CGTARG_Prefered_GRA_Registers(ISA_REGISTER_CLASS rclass)
{
  REGISTER_SET s = REGISTER_SET_EMPTY_SET;
  if (Is_Target_st235 () && rclass == ISA_REGISTER_CLASS_integer) {
    /* For ST235 we first prefer over the set of callee saved the
     * registers that are subject to pairing.
     * This will allow generation of paired save/restore of the
     * callee saved registers.
     */
    REGISTER_SET regset = REGISTER_CLASS_callee_saves(rclass);
    REGISTER reg;
    for (reg = REGISTER_SET_Choose(regset);
	 reg != REGISTER_UNDEFINED;
	 reg = REGISTER_SET_Choose_Next(regset, reg)
	 ) {
      if (REGISTER_SET_MemberP(REGISTER_SUBCLASS_members(ISA_REGISTER_SUBCLASS_paired), reg) &&
	  REGISTER_SET_Choose_Next(regset, reg) != REGISTER_UNDEFINED &&
	  REGISTER_SET_Choose_Next(regset, reg) == reg+1) {
	s = REGISTER_SET_Union1(s, reg);
	s = REGISTER_SET_Union1(s, reg+1);
	// Skip next register
	reg = REGISTER_SET_Choose_Next(regset, reg);
      }
    }
  }
  return s;
}

REGISTER_SET
CGTARG_Prefered_LRA_Registers(ISA_REGISTER_CLASS rclass)
{
  return CGTARG_Prefered_GRA_Registers (rclass);
}
#endif
