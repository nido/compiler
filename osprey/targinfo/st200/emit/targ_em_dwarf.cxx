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


#include <stdlib.h>
#include <stdio.h>
#include <elf.h>
#include <elfaccess.h>
#include <libelf.h>
#include <libdwarf.h>
#include <assert.h>	// temporary
#define USE_STANDARD_TYPES 1
#include "defs.h"
#include "errors.h"
#include "register.h" // For REGISTER_MIN and CGTARG_DW_DEBUG_Get_Extension_Id
#include "targ_em_dwarf.h"

#pragma pack(1)
struct UINT32_unaligned {
  UINT32 val;
};

struct UINT64_unaligned {
  UINT64 val;
};
#pragma pack(0)

static Elf32_Rel *
translate_reloc32(Dwarf_Relocation_Data       rentry,
		  Cg_Dwarf_Sym_To_Elfsym_Ofst translate_symndx,
		  Dwarf_Ptr                   buffer,
		  Dwarf_Unsigned              bufsize)
{
  static Elf32_Rel retval;
  Dwarf_Unsigned   elf_symidx, elf_symoff;

  return &retval;
}

static Elf64_Rel *
translate_reloc64(Dwarf_Relocation_Data       rentry,
		  Cg_Dwarf_Sym_To_Elfsym_Ofst translate_symndx,
		  Dwarf_Ptr                   buffer,
		  Dwarf_Unsigned              bufsize)
{
  static Elf64_Rel retval;
  Dwarf_Unsigned   elf_symidx, elf_symoff;

  return &retval;
}

Dwarf_Ptr
Em_Dwarf_Symbolic_Relocs_To_Elf(next_buffer_retriever     get_buffer,
				next_bufsize_retriever    get_bufsize,
				advancer_to_next_stream   advance_stream,
				Dwarf_Signed              buffer_scndx,
				Dwarf_Relocation_Data     reloc_buf,
				Dwarf_Unsigned            entry_count,
				int                       is_64bit,
				Cg_Dwarf_Sym_To_Elfsym_Ofst translate_symndx,
				Dwarf_Unsigned           *result_buf_size)
{
  unsigned i;
  unsigned step_size = (is_64bit ? sizeof(Elf64_Rel) : sizeof(Elf32_Rel));

  Dwarf_Ptr  result_buf = (Dwarf_Ptr *) malloc(step_size * entry_count);
  char *cur_reloc = (char *) result_buf;

  Dwarf_Unsigned offset_offset = 0;
  Dwarf_Unsigned bufsize = 0;
  Dwarf_Ptr      buffer = NULL;

  return (Dwarf_Ptr) result_buf;
}

//TB: Add Get_Debug_Reg_Id from stxp70
DebugRegId
Get_Debug_Reg_Id(ISA_REGISTER_CLASS reg_class, INT reg, INT bitSize)
{
    // Input register identifier are given for REGISTER API. To use it with
    // ISA_REGISTER API and so have a machine representation, we have to applied
    // the offset used between the two APIs.
    reg -= REGISTER_MIN;
    mINT32 regId = CGTARG_DW_DEBUG_Get_Reg_Id(reg_class, reg, bitSize);
    return DebugRegId(regId);
}
