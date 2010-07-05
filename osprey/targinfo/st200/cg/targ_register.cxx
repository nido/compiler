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

#ifdef TARG_ST
#include "W_alloca.h"         // using alloca()
#endif


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

#include "dyn_isa_api.h"  // reconfigurability
#include "lai_loader_api.h"
#include <ctype.h>                /* for isdigit() */

#include "register_targ.h"	// Implemented interface.


static mISA_REGISTER_CLASS CGTARG_REGISTER_Mtype_RegClass_Map[MTYPE_MAX_LIMIT+1];
//TB associate also a register subclass to an mtype
static mISA_REGISTER_SUBCLASS CGTARG_REGISTER_Mtype_RegSubclass_Map[MTYPE_MAX_LIMIT+1];

/* ====================================================================
 *   CGTARG_Register_Subclass_For_Mtype
 *   CGTARG_Register_Class_For_Mtype
 * ====================================================================
 */
ISA_REGISTER_CLASS
CGTARG_Register_Class_For_Mtype(TYPE_ID mtype)
{
  return   (mtype < FIRST_COMPOSED_MTYPE)
	 ? (ISA_REGISTER_CLASS)CGTARG_REGISTER_Mtype_RegClass_Map[mtype] 
	 : ISA_REGISTER_CLASS_UNDEFINED;
}

ISA_REGISTER_SUBCLASS
CGTARG_Register_Subclass_For_Mtype(TYPE_ID mtype)
{
  return   (mtype < FIRST_COMPOSED_MTYPE)
	 ? (ISA_REGISTER_CLASS)CGTARG_REGISTER_Mtype_RegSubclass_Map[mtype] 
	 : ISA_REGISTER_CLASS_UNDEFINED;
}


/* ====================================================================
 *   CGTARG_REGISTER_Init_Mtype_RegClass_Map
 * ====================================================================
 */
static void 
CGTARG_REGISTER_Init_Mtype_RegClass_Map ()
{
  INT i;
  mISA_REGISTER_CLASS * const map = CGTARG_REGISTER_Mtype_RegClass_Map;

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

  // Reconfigurability: initialize for extension mtype
  if (NB_PURE_DYNAMIC_MTYPES > 0) {
	for (i = MTYPE_STATIC_LAST + 1; i < FIRST_COMPOSED_MTYPE; i++) {
	  map[i] = EXTENSION_MTYPE_to_REGISTER_CLASS(i);
	}
  }

  return;
}

/* ====================================================================
 *   CGTARG_REGISTER_Init_Mtype_RegSubclass_Map
 * ====================================================================
 */
static void 
CGTARG_REGISTER_Init_Mtype_RegSubclass_Map ()
{
  INT i;
  mISA_REGISTER_SUBCLASS * const map = CGTARG_REGISTER_Mtype_RegSubclass_Map;

  for (i = 0; i <= MTYPE_LAST; ++i) 
    map[i] = ISA_REGISTER_SUBCLASS_UNDEFINED;

  // Reconfigurability: initialize for extension mtype
  if (NB_PURE_DYNAMIC_MTYPES > 0) {
    for (i = MTYPE_STATIC_LAST + 1; i < FIRST_COMPOSED_MTYPE; i++) {
      map[i] = EXTENSION_MTYPE_to_REGISTER_SUBCLASS(i);
    }
  }

  return;
}

/* ====================================================================
 *  CGTARG_Initialize_Register_Class
 *
 *  Initialize the register class 'rclass'. This function may be
 *  called multiple time for the same register class. Thus it must be
 *  idempotent.
 *
 *  This function is called at the start of the target independent
 *  function Initialize_Register_Class in lai/register.cxx.
 *  Thus it can be used to modify ISA_REGISTER_CLASS_INFO just before it
 *  is cached into the REGISTER_CLASS_INFO structure in register.cxx.
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
  /* [JV] This code should be removed because it managed only reg class
     known by the ABI.
  */
  if (rclass == ISA_REGISTER_CLASS_integer) {
    const ISA_REGISTER_CLASS_INFO *rcinfo = ISA_REGISTER_CLASS_Info(rclass);
    INT i;
    INT first_isa_reg = ISA_REGISTER_CLASS_INFO_First_Reg(rcinfo);
    INT last_isa_reg = ISA_REGISTER_CLASS_INFO_Last_Reg(rcinfo);

    // Find the first integer function value register and set v0 if found
    for (i = 0; i < last_isa_reg - first_isa_reg + 1; ++i) {
      INT isa_reg = i + first_isa_reg;
      REGISTER reg = i + REGISTER_MIN;
      BOOL is_func_value = ABI_PROPERTY_Is_func_val(rclass, isa_reg);
      if (is_func_value) {
	Set_CLASS_REG_PAIR_reg(CLASS_REG_PAIR_v0, reg);
	Set_CLASS_REG_PAIR_rclass(CLASS_REG_PAIR_v0, rclass);
	break;
      }
    }
  }

  // Adjust size of register for the branch register class.
  // This must be done here, before the lai/register.cxx
  // function creates the REGISTER_CLASS_INFO cache.
  if (rclass == ISA_REGISTER_CLASS_branch) {
    if (Is_Target_st220 () || Is_Target_st231 ())
      ISA_REGISTER_CLASS_Set_Bit_Size (ISA_REGISTER_CLASS_branch, 1);
    else
      ISA_REGISTER_CLASS_Set_Bit_Size (ISA_REGISTER_CLASS_branch, 4);
  }

  return;
}

/* ====================================================================
 *   CGTARG_REGISTER_Pu_Begin()
 *
 *  Initialize the register package for the current pu.
 * ====================================================================
 */
void
CGTARG_REGISTER_Pu_Begin(void)
{
  // No op.
  return;
}

/* ====================================================================
 *   CGTARG_REGISTER_Begin()
 * ====================================================================
 */
void
CGTARG_REGISTER_Begin(void)
{
  CGTARG_REGISTER_Init_Mtype_RegSubclass_Map ();
  CGTARG_REGISTER_Init_Mtype_RegClass_Map ();
}


/* ====================================================================
 *   CGTARG_Register_Class_Num_From_Name
 *
 *   given register name (asm), get its register class.
 *   returns ISA_REGISTER_CLASS_UNDEFINED if not register match the name.
 * ====================================================================
 */
// Moved to file targ_register_common.cxx, now shared by both fe and be


/* ====================================================================
 *   CGTARG_Forbidden_GRA_Registers 
 * ====================================================================
 */
REGISTER_SET 
CGTARG_Forbidden_GRA_Registers (ISA_REGISTER_CLASS rclass)
{
  REGISTER_SET s = REGISTER_SET_EMPTY_SET;
  return s;
}

REGISTER_SET
CGTARG_Forbidden_LRA_Registers (ISA_REGISTER_CLASS rclass)
{
  return CGTARG_Forbidden_GRA_Registers (rclass);
}

/*
 * The set of forbidden registers for prolog/epilog temporaries
 */
REGISTER_SET
CGTARG_Forbidden_Prolog_Epilog_Registers (ISA_REGISTER_CLASS rclass)
{
  REGISTER_SET s = REGISTER_SET_EMPTY_SET;
  return s;
}


#ifdef TARG_ST
/* ====================================================================
 *   CGTARG_Preferred_GRA_Registers 
 * ====================================================================
 */
REGISTER_SET
CGTARG_Preferred_GRA_Registers(ISA_REGISTER_CLASS rclass)
{
  REGISTER_SET s = REGISTER_SET_EMPTY_SET;
  if (Is_Target_st240 ()
      && rclass == ISA_REGISTER_CLASS_integer) {
    /* For ST240 we first prefer over the set of callee saved the
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
    /* For consistency, also add the caller saved set in the preferencing set */
    s = REGISTER_SET_Union(s, REGISTER_CLASS_caller_saves(rclass));
  }
  return s;
}

REGISTER_SET
CGTARG_Preferred_LRA_Registers(ISA_REGISTER_CLASS rclass)
{
  return CGTARG_Preferred_GRA_Registers (rclass);
}


/* ====================================================================
 *   Is_Predicate_REGISTER_CLASS
 *   Returns TRUE if the rclass is the predicate register class.
 * ====================================================================
 */
BOOL Is_Predicate_REGISTER_CLASS(ISA_REGISTER_CLASS rclass) {
  return rclass == ISA_REGISTER_CLASS_branch;
}

#endif                   // TARG_ST

