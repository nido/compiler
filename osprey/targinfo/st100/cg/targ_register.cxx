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

/* should be generated together with targ_isa_register.[hc] */
char *ISA_REGISTER_CLASS_symbol[] = {
  "",   /* ISA_REGISTER_CLASS_UNDEFINED */
  "_d",  /* ISA_REGISTER_CLASS_du */
  "_a",  /* ISA_REGISTER_CLASS_au */
  "_b",  /* ISA_REGISTER_CLASS_guard */
  "_c",  /* ISA_REGISTER_CLASS_control */
  "_l"   /* ISA_REGISTER_CLASS_loop */
};

// REGISTER values are biased by REGISTER_MIN, so apply
// it to get REGISTER value given a machine reg number
#define FIRST_INPUT_REG (32+REGISTER_MIN)
#define FIRST_OUTPUT_REG (127+REGISTER_MIN)
#define LAST_STACKED_REG (127+REGISTER_MIN)
#define FIRST_ROTATING_INTEGER_REG (32+REGISTER_MIN)
#define FIRST_ROTATING_FLOAT_REG (32+REGISTER_MIN)
#define FIRST_ROTATING_PREDICATE_REG (16+REGISTER_MIN)
#define LAST_ROTATING_INTEGER_REG (127+REGISTER_MIN)
#define LAST_ROTATING_FLOAT_REG (127+REGISTER_MIN)
#define LAST_ROTATING_PREDICATE_REG (63+REGISTER_MIN)


static INT num_output_parameters;
static INT num_caller;


static char outregname[6] = "out0";

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

  map[MTYPE_B] = ISA_REGISTER_CLASS_guard;
  map[MTYPE_I1] = ISA_REGISTER_CLASS_du;
  map[MTYPE_I2] = ISA_REGISTER_CLASS_du;
  map[MTYPE_I4] = ISA_REGISTER_CLASS_du;
  map[MTYPE_I5] = ISA_REGISTER_CLASS_du;
  map[MTYPE_I8] = ISA_REGISTER_CLASS_UNDEFINED;
  map[MTYPE_U1] = ISA_REGISTER_CLASS_du;
  map[MTYPE_U2] = ISA_REGISTER_CLASS_du;
  map[MTYPE_U4] = ISA_REGISTER_CLASS_du;
  map[MTYPE_U5] = ISA_REGISTER_CLASS_du;
  map[MTYPE_U8] = ISA_REGISTER_CLASS_UNDEFINED;
  map[MTYPE_A4] = ISA_REGISTER_CLASS_au;
  map[MTYPE_F4] = ISA_REGISTER_CLASS_UNDEFINED;
  map[MTYPE_F8] = ISA_REGISTER_CLASS_UNDEFINED;
  map[MTYPE_F10] = ISA_REGISTER_CLASS_UNDEFINED;
  map[MTYPE_F16] = ISA_REGISTER_CLASS_UNDEFINED;

  return;
}

//
// Target-specific class register pairs
//
CLASS_REG_PAIR      CLASS_REG_PAIR_gr;

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
  if (rclass == ISA_REGISTER_CLASS_du) {
    Set_CLASS_REG_PAIR_reg(CLASS_REG_PAIR_v0, 
	     REGISTER_SET_Choose(REGISTER_CLASS_function_value(rclass)));
    Set_CLASS_REG_PAIR_rclass(CLASS_REG_PAIR_v0, rclass);
  }

  // Initialize target-specific class-reg pairs:
  if (rclass == ISA_REGISTER_CLASS_control) {
    Set_CLASS_REG_PAIR_reg(CLASS_REG_PAIR_gr, 9);
    Set_CLASS_REG_PAIR_rclass(CLASS_REG_PAIR_gr, rclass);
  }

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
  ISA_REGISTER_CLASS rclass;

  switch (regname[0]) {
	case 'r':
		rclass = ISA_REGISTER_CLASS_du;
		break;
	case 'f':
		rclass = ISA_REGISTER_CLASS_UNDEFINED;
		break;
	case 'p':
		rclass = ISA_REGISTER_CLASS_au;
		break;
	default:
		FmtAssert(FALSE, ("unexpected reg letter %c", regname[0]));
  }

  return rclass;
}

