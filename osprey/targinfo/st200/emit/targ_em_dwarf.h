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
#include "targ_isa_registers.h" // For ISA_REGISTER_CLASS definition
#define DWARF_TARGET_FLAG	DW_DLC_ISA_IA64

#define STACK_SCRATCH_AREA_SIZE 16 /* [CL] size in bytes */

// Define all instruction bytes that go into a CIE.
// DW_CFA_def_cfa
//      - opcode is 0x0c
//      - register number 12
//      - offset STACK_SCRATCH_AREA_SIZE (in LEB128) form
#define TARG_INIT_BYTES { 0x0c,  0xc, STACK_SCRATCH_AREA_SIZE }

// Return Address is in r63
#ifdef DW_FRAME_RA_COL
#undef DW_FRAME_RA_COL
#endif
#define DW_FRAME_RA_COL 63

typedef void (*Cg_Dwarf_Sym_To_Elfsym_Ofst)(Dwarf_Unsigned,
					    Dwarf_Unsigned *,
					    Dwarf_Unsigned *);

typedef Dwarf_Unsigned (*symbol_index_recorder)(Elf64_Word);

typedef int            (*advancer_to_next_stream)(Dwarf_Signed);
typedef Dwarf_Ptr      (*next_buffer_retriever)(void);
typedef Dwarf_Unsigned (*next_bufsize_retriever)(void);

Dwarf_Ptr Em_Dwarf_Symbolic_Relocs_To_Elf(next_buffer_retriever,
					  next_bufsize_retriever,
					  advancer_to_next_stream,
					  Dwarf_Signed,
					  Dwarf_Relocation_Data,
					  Dwarf_Unsigned,
					  int,
					  Cg_Dwarf_Sym_To_Elfsym_Ofst,
					  Dwarf_Unsigned *);

/**
 * Get debug identifier for given register.
 * The debug identifier is a virtual representation of the hardware register.
 * This representation is understood by the debugger whereas hardware
 * identifier are not.
 *
 * @param  reg_class Register class of given reg
 * @param  reg Register to be translated
 * @param  bitSize Size in bit of the register
 *
 * @pre    true
 * @post   true
 *
 * @return Related debug identifier
 */
typedef Dwarf_Unsigned DebugRegId;
extern DebugRegId
Get_Debug_Reg_Id(ISA_REGISTER_CLASS reg_class, INT reg, INT bitSize);

