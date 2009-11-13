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
#include "cgir.h"
#include "tn_map.h"
#include "em_elf.h"
#include "dwarf_DST_mem.h"         /* for DST_IDX */
#include "em_dwarf.h"
#include "targ_em_dwarf.h"
#include "cg.h"


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

INT
CodeAlignmentFactor(PU& pu)
{
    return 1;
}

INT
DataAlignmentFactor(PU& pu)
{
    return Use_32_Bit_Pointers? -4: -8;
}

INT
HashValue(PU& pu)
{
  INT personality_hash = 0;
  if (PU_has_exc_scopes(pu)) {
    if (PU_cxx_lang(pu)) personality_hash |= 2;
    if (PU_c_lang(pu)) personality_hash |= 1;
  }
  if (Gen_PIC_Shared || Gen_PIC_Call_Shared)
    personality_hash |= 4;
  return personality_hash;
}

BOOL
IsSaved(const DebugRegId& a_id, ISA_REGISTER_CLASS regClass, REGISTER reg,
        PU& a_pu)
{
    return REGISTER_SET_MemberP(REGISTER_CLASS_callee_saves(regClass), reg) ||
        a_id == Get_Debug_Reg_Id(CLASS_REG_PAIR_fp) ||
        a_id == Get_Debug_Reg_Id(CLASS_REG_PAIR_ra);
}

BOOL
ShouldGenerateInformation(ISA_REGISTER_CLASS register_class, PU& pu)
{
    return TRUE;
}

void
CfaDef(DebugRegId& a_base, UINT& a_offset, PU& a_pu)
{
    a_base = 12;
    a_offset = STACK_SCRATCH_AREA_SIZE;
}

BOOL
HasSpecialDef(const DebugRegId& a_id, UINT& offset,
              ISA_REGISTER_CLASS regClass, REGISTER reg, PU& a_pu)
{
    return FALSE;
}

extern BOOL
ReturnAddressDef(DebugRegId& a_id, UINT& offset, PU& a_pu)
{
    a_id = DW_FRAME_RA_COL;
    offset = 0;
    return FALSE;
}


DebugRegId
Get_Debug_Reg_Id(ISA_REGISTER_CLASS reg_class, REGISTER reg, INT bitSize)
{
    return DebugRegId(REGISTER_machine_id(reg_class, reg));
}
