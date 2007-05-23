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


// abi_properties_gen.h
/////////////////////////////////////
//
//  Interface for specifying properties (attributes) for various ABIs
//  supported by an architecture.
//
//  void ABI_Properties_Begin(const char *name)
//	Initialize to generate ABI properties information for the architecture 
//	with the given <name>.  The information will be written to the 
//	files targ_abi_properties.[ch].  
//
//  TYPE ABI_PROPERTY
//	An abstract type that represents a property of an ABI.
//	No client visible fields.
//
//  ABI_PROPERTY Create_Reg_Property(const char *name)
//	Used to create a new ABI_PROPERTY.  <name> is the property name.
//	It will be used to define a ABI_PROPERTY_<name> constant.
//
//  Begin_ABI(const char *name)
//	Begin the definition of the ABI named <name>. An ABI definition
//	is ended by another Begin_ABI() call or a call to ABI_Properties_End().
//
//  void Reg_Property(ABI_PROPERTY prop, ISA_REGISTER_CLASS rc, ...)
//	Give a number of registers in class <rc>, the ABI property <prop>,
//	i.e. it identifies the role of particular registers in the ABI.
//	The variable argument specifies the register numbers, terminated
//	by -1.
//
//  void Reg_Names(ISA_REGISTER_CLASS rc, 
//		   INT minreg, 
//		   INT maxreg, 
//		   const char **names)
//	Give ABI names to the registers in class <rc>. These names typically
//	identify the role of the register in the ABI. If a register is
//	not explicitly given an ABI name, the hardware name is used.
//	The register names are specified in an array pointed to by <names>.
//	<minreg> gives the register number of the first name in the
//	<names> array; <maxreg> gives the last.
//
//  void ABI_Properties_End(void)
//      Complete processing of ABI properties.
//
/////////////////////////////////////
//
//  Debug information:
//
//  EXT_DESCRIPTION DW_DEBUG_Extension     ( const char *extname, 
//                                           const char *reloc_string
//     For extensible architectures, this routine makes it possible to
//     specify 
//       - 1 the extension name
//       - 2 the relocation string associated to the extension (can
//           be NULL.
//
//  void            DW_DEBUG_Extension_Reg  ( ISA_REGISTER_CLASS rc,
//                                            EXT_DESCRIPTION    ext,
//                                            unsigned int       base_id);
//     Specify for a given register class:
//       - 1 the extension from which it is an architectural 
//           ressource (or a NULL pointer for the core). 
//       - 2 the DWARF base identifier of the register class.
//
//     Note: for the STxP70 architecture, the DWARF identifier will
//           be in many cases 0 since the relocation process at link
//           time will assign the right base identifier. This base_id
//           identifier is only meaningful for extensions that have
//           more than one register file.
//
/////////////////////////////////////


#include "targ_isa_registers.h"

typedef struct abi_property    *ABI_PROPERTY;
typedef struct ext_description *EXT_DESCRIPTION;

extern void ABI_Properties_Begin(const char *name);
extern ABI_PROPERTY Create_Reg_Property(const char *name);
extern void Begin_ABI(const char *name);
extern void Begin_ABI_dynamic(const char *name);
extern void Reg_Property(ABI_PROPERTY prop, ISA_REGISTER_CLASS rc, ...);
extern void Reg_Names(ISA_REGISTER_CLASS rc, 
		      INT minreg, 
		      INT maxreg, 
		      const char **names);
extern void ABI_Properties_End(void);

/////////////////////////////////////////

extern EXT_DESCRIPTION DW_DEBUG_Extension      (const char *extname,
                                                const char *reloc_string);
extern void            DW_DEBUG_Extension_Reg  (ISA_REGISTER_CLASS rc,
                                                EXT_DESCRIPTION    ext,
                                                unsigned int       dw_base_id);

/////////////////////////////////////////
