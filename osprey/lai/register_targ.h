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

/*
 * Target dependent interface for CGIR register management.
 * 
 * Must be included by register.h only.
 * Used for target dependent implementation of register.cxx.
 * Implementation of this interface available in targinfo/<targ>/cg/targ_register.cxx.
 *
 */

#ifndef REGISTER_TARG_INCLUDED

#include "defs.h"
#include "targ_isa_registers.h"
#ifdef TARG_ST
#include "targ_register_common.h"
#endif

// Initialization for targ_register.
extern void CGTARG_REGISTER_Begin(void);
extern void CGTARG_REGISTER_Pu_Begin(void);
extern void CGTARG_Initialize_Register_Class(ISA_REGISTER_CLASS rclass);

// Target dependent implemetation of some functions above.
extern ISA_REGISTER_CLASS CGTARG_Register_Class_For_Mtype(TYPE_ID mtype);
extern ISA_REGISTER_CLASS CGTARG_Register_Subclass_For_Mtype(TYPE_ID mtype);
#ifdef TARG_ST
// [TTh] Now defined in 'targ_register_common.h'
#else
extern ISA_REGISTER_CLASS CGTARG_Register_Class_Num_From_Name (char *regname, int *regnum);
#endif


// Returns the set of forbidden registers for GRA.
// We forbid some registers from being used by the GRA. Such is the
// case of the $r0.63 on st200 which is a dedicated register for 
// the igoto instruction. GRA should never allocate this register 
// to a global live-range (unless dedicated to); or otherwise LRA
// may not be able to spill a local liverange (like the one used
// for an igoto) because of "spill beyond the end of BB, etc.
extern REGISTER_SET CGTARG_Forbidden_GRA_Registers(ISA_REGISTER_CLASS rclass);
// [SC] The set of forbidden registers for LRA.
extern REGISTER_SET CGTARG_Forbidden_LRA_Registers(ISA_REGISTER_CLASS rclass);


// Returns the set of registers that will be selected in priority by
// GRA (resp. LRA).
// This may be used for instance to select some registers subjects
// to multi load/store operations first.
// This is NOT used for instance to select first caller or callee saved
// register, this is handled specially in GRA or LRA.
extern REGISTER_SET CGTARG_Prefered_GRA_Registers(ISA_REGISTER_CLASS rclass);
extern REGISTER_SET CGTARG_Prefered_LRA_Registers(ISA_REGISTER_CLASS rclass);

extern const char *CGTARG_DW_DEBUG_Get_Extension_Reloc_String(ISA_REGISTER_CLASS rclass);
extern const char *CGTARG_DW_DEBUG_Get_Extension_Name(ISA_REGISTER_CLASS rclass);
extern INT32 CGTARG_DW_DEBUG_Get_Extension_Id(ISA_REGISTER_CLASS rclass, 
					      BOOL *activated);
extern INT32 CGTARG_DW_DEBUG_Get_Reg_Id  (ISA_REGISTER_CLASS rclass,
					  INT32  index,
					  UINT32 bit_size);
extern const char *CGTARG_DW_DEBUG_Get_Reg_Name(ISA_REGISTER_CLASS rclass,
                                                INT32  index,
                                                UINT32 bit_size);
extern void CGTARG_DW_DEBUG_Begin(void);


#endif /* REGISTER_TARG_INCLUDED */
