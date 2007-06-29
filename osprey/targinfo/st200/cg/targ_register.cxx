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
  /* [JV] This code should be removed because it managed only reg class
     known by the ABI.
  */
  if (rclass == ISA_REGISTER_CLASS_integer) {
    Set_CLASS_REG_PAIR_reg(CLASS_REG_PAIR_v0, 
			   REGISTER_SET_Choose(REGISTER_CLASS_function_value(rclass)));
    Set_CLASS_REG_PAIR_rclass(CLASS_REG_PAIR_v0, rclass);
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


#ifdef TARG_ST
/* ====================================================================
 *   Extension_Regname_Register_Class
 *
 *   given register name (asm), get its register class for an extension.
 * ====================================================================
 */
static ISA_REGISTER_CLASS
Extension_Regname_Register_Class (char *regname, int *regnum)
{
  ISA_REGISTER_CLASS cl;
  FOR_ALL_ISA_REGISTER_CLASS( cl ) {
    int reg;
    int first_isa_reg  = ISA_REGISTER_CLASS_info[cl].min_regnum;
    int last_isa_reg   = ISA_REGISTER_CLASS_info[cl].max_regnum;

    for (reg = first_isa_reg; reg < last_isa_reg; reg++) {
      char *temp;
      if (regname[0] == '%') {
	const char *rc_name = ISA_REGISTER_CLASS_info[cl].name;
	temp = (char*)alloca(strlen(rc_name) + strlen(ISA_REGISTER_CLASS_info[cl].reg_name[reg]) + 1);
	sprintf(temp,"%%%s%%%s",rc_name, ISA_REGISTER_CLASS_info[cl].reg_name[reg]);
      } else {
	temp = (char*)ISA_REGISTER_CLASS_info[cl].reg_name[reg];
      }
      if (strcasecmp(temp, regname) == 0) {
	*regnum = reg;
	return cl;
      }
    }
  }
  return ISA_REGISTER_CLASS_UNDEFINED;
}
#endif
				  

/* ====================================================================
 *   CGTARG_Register_Class_Num_From_Name
 *
 *   given register name (asm), get its register class.
 *   returns ISA_REGISTER_CLASS_UNDEFINED if not register match the name.
 * ====================================================================
 */
ISA_REGISTER_CLASS 
CGTARG_Register_Class_Num_From_Name (char *regname, int *regnum) 
{
  ISA_REGISTER_CLASS rclass = ISA_REGISTER_CLASS_UNDEFINED;

  switch (regname[0]) {
  case 'r':
    if (isdigit(regname[1])) {
      rclass = ISA_REGISTER_CLASS_integer;
      *regnum = atoi(regname+1);
    }
    break;
  case 'b':
    if (isdigit(regname[1])) {
      rclass = ISA_REGISTER_CLASS_branch;
      *regnum = atoi(regname+1);
    }
    break;
  }

  if (rclass == ISA_REGISTER_CLASS_UNDEFINED && Extension_Is_Present)
    return Extension_Regname_Register_Class(regname, regnum);
  
  return rclass;
}


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

#ifdef TARG_ST
/* ====================================================================
 *   CGTARG_Prefered_GRA_Registers 
 * ====================================================================
 */
REGISTER_SET
CGTARG_Prefered_GRA_Registers(ISA_REGISTER_CLASS rclass)
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
  }
  return s;
}

REGISTER_SET
CGTARG_Prefered_LRA_Registers(ISA_REGISTER_CLASS rclass)
{
  return CGTARG_Prefered_GRA_Registers (rclass);
}

//
// Reconfigurability stuff
//

/* ====================================================================
 *   CGTARG_DW_DEBUG_Get_Canonical_Index 
 *   Returns the canonical index for an appaired register (or the
 *  "normal" register index for a "normal" register). Also returns the
 *   subclass identifier in the subclass table, and the index of the
 *   appaired register in the considered subclass.
 * ====================================================================
 */
static INT32
CGTARG_DW_DEBUG_Get_Canonical_Index(ISA_REGISTER_CLASS rclass,
                                    INT32  index,
                                    UINT32 bit_size,
                                    ISA_REGISTER_SUBCLASS *subclass,
                                    INT32  *subclass_index)
{
    INT32 nb_basic_reg;
    INT32 nb_paired_reg;
    INT32 regclass_bit_size;
    INT32 can_index;
    INT32 regclass_size;
    INT32 j;
    UINT32 appaired_level;

    const ISA_REGISTER_CLASS_INFO *cinfo;

    FmtAssert(bit_size!=0,("uncorrect register size"));

    cinfo = ISA_REGISTER_CLASS_Info(rclass);

    FmtAssert(index>=ISA_REGISTER_CLASS_INFO_First_Reg(cinfo) && 
              index<=ISA_REGISTER_CLASS_INFO_Last_Reg(cinfo),
              ("uncorrect register index"));

    regclass_bit_size = ISA_REGISTER_CLASS_INFO_Bit_Size(cinfo);

    // The normal classical case.
    if(regclass_bit_size == bit_size)
     { *subclass       = ISA_REGISTER_SUBCLASS_UNDEFINED;
       *subclass_index = -1; 
        can_index  = index;
        return can_index;
     }
     
    // Looking for appaired registers.
    nb_basic_reg = ISA_REGISTER_CLASS_INFO_Last_Reg(cinfo) -
                   ISA_REGISTER_CLASS_INFO_First_Reg(cinfo)  + 
                   1;

    appaired_level = bit_size / regclass_bit_size;

    // Size of appaired register must be a
    // power of two of the basic register size.
    FmtAssert( 0U ==(appaired_level & (appaired_level-1U)) &&
               0U == (bit_size % regclass_bit_size),
               ("uncorrect size for appaired registers"));

    // Calculate canonical index
    can_index = 0;
    j = 1;
    do{
     can_index += nb_basic_reg/j;
     j<<=1;                             // j = j*2;
    } while(j<appaired_level);

   *subclass_index = index / appaired_level;
    can_index     += index / appaired_level; 

    // Looking for the canonical subclass that contains
    // appaired registers
   *subclass      = ISA_REGISTER_SUBCLASS_UNDEFINED;
    nb_paired_reg = nb_basic_reg/appaired_level;

    for(j =ISA_REGISTER_SUBCLASS_MIN;
        j<=ISA_REGISTER_SUBCLASS_MAX && 
        ISA_REGISTER_SUBCLASS_UNDEFINED==*subclass;
        j++) {
      const ISA_REGISTER_SUBCLASS_INFO *sinfo = ISA_REGISTER_SUBCLASS_Info(j);

      // Register class must be ok
      // Subclass size (nb of appaired reg.) must be ok with appairing level
      // Subclass must be canonical.
      if(ISA_REGISTER_SUBCLASS_INFO_Class(sinfo)==rclass        &&
         ISA_REGISTER_SUBCLASS_INFO_Count(sinfo)==nb_paired_reg &&
         ISA_REGISTER_SUBCLASS_INFO_Canonical(sinfo)) {
         *subclass = j;
         }
     }

    FmtAssert(*subclass!=ISA_REGISTER_SUBCLASS_UNDEFINED,
              ("can't find canonical subclass"));

    return index;
}

/* ====================================================================
 *   CGTARG_DW_DEBUG_Get_Extension_Reloc_String 
 *   Returns the relocation string associated to a given register class.
 *   Returns a NULL pointer if:
 *    1 - register class is a core register class,
 *    2 - register class is an extension register class and the
 *        relocation mechanism is not relevant or has not been
 *        specified.
 * ====================================================================
 */
const char*
CGTARG_DW_DEBUG_Get_Extension_Reloc_String(ISA_REGISTER_CLASS rclass)
{
   int ext_id;

   FmtAssert(rclass<ISA_REGISTER_CLASS_MAX+1,
             ("erroneous register class identifier"));

   ext_id = EXTENSION_reg_description_tab[rclass].ext_id;
   if(ext_id==-1)     // Register class belongs to the core.
     return (const char*)(NULL);
 
   return EXTENSION_description_tab[ext_id].reloc_string;
}

/* ====================================================================
 *   CGTARG_DW_DEBUG_Get_Reloc_String 
 *   Returns the name of the extension from which the considered
 *   register class is an architectural ressource.
 *   Returns a NULL pointer if register class belongs to the core.
 * ====================================================================
 */
const char*
CGTARG_DW_DEBUG_Get_Extension_Name(ISA_REGISTER_CLASS rclass)
{
   int ext_id;

   FmtAssert(rclass<ISA_REGISTER_CLASS_MAX+1,
             ("erroneous register class identifier"));

   ext_id = EXTENSION_reg_description_tab[rclass].ext_id;
   if(ext_id==-1)      // Register class belongs to the core.
     return (const char*)(NULL);
 
   return EXTENSION_description_tab[ext_id].extname;
}

/* ====================================================================
 *   CGTARG_DW_DEBUG_Get_Extension_Id 
 *   Return the internal extension identifier the input register
 *   class belongs to. Also return a boolean flag that indicates 
 *   whether extension is activated or not (useful for extensions
 *    built statically with the core).
 * ====================================================================
 */
INT32
CGTARG_DW_DEBUG_Get_Extension_Id(ISA_REGISTER_CLASS rclass, BOOL *activated)
{
   int ext_id;
   FmtAssert(rclass<ISA_REGISTER_CLASS_MAX+1,
             ("erroneous register class identifier"));

   ext_id = EXTENSION_reg_description_tab[rclass].ext_id;
  *activated = ext_id==-1 ? true : EXTENSION_description_tab[ext_id].enabled;

   return ext_id;
}

/* ====================================================================
 *   CGTARG_DW_DEBUG_Get_Reg_Id 
 *   Returns the name of the dwarf debug identifier of a register.
 * ====================================================================
 */
INT32 
CGTARG_DW_DEBUG_Get_Reg_Id(ISA_REGISTER_CLASS rclass,
                           INT32  index,
                           UINT32 bit_size)
{
     
   ISA_REGISTER_SUBCLASS      subclass;
   INT32                     subclass_index;

   // Index is the sum of the canonical index
   // plus the dwarf register class id assigned
   // for the compilation unit
   return CGTARG_DW_DEBUG_Get_Canonical_Index(
                               rclass,
                               index,
                               bit_size,
                               &subclass,
                               &subclass_index) +
          EXTENSION_reg_description_tab[rclass].dw_base_id;
}

/* ====================================================================
 *   CGTARG_DW_DEBUG_Get_Reg_Name 
 *   Returns register name for "normal" and appaired registers.
 * ====================================================================
 */
const char*
CGTARG_DW_DEBUG_Get_Reg_Name(ISA_REGISTER_CLASS rclass,
                             INT32  index,
                             UINT32 bit_size)
{
   ISA_REGISTER_SUBCLASS      subclass;
   INT32                     subclass_index;
   INT32                     can_index;
   const char                *res;

   const ISA_REGISTER_CLASS_INFO    *cinfo;
   const ISA_REGISTER_SUBCLASS_INFO *sinfo;

   // Get canonical index.
   can_index = CGTARG_DW_DEBUG_Get_Canonical_Index(
                                    rclass,
                                    index,
                                    bit_size,
                                    &subclass,
                                    &subclass_index);

   if(can_index==index) { // For a normal register
    cinfo = ISA_REGISTER_CLASS_Info(rclass); 
    res   = ISA_REGISTER_CLASS_INFO_Reg_Name(cinfo,index);
    }
   else {                 // For an appaired register
    sinfo = ISA_REGISTER_SUBCLASS_Info(subclass);
    res   = ISA_REGISTER_SUBCLASS_INFO_Reg_Name(sinfo,subclass_index);
    }

   return res;
}

/* ====================================================================
 *   CGTARG_DW_DEBUG_Begin
 *   Initialization step. Once per file.
 * ====================================================================
 */
void
CGTARG_DW_DEBUG_Begin( void )
{
   return;
}
#endif                   // TARG_ST

