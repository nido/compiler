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



#ifndef cgdwarf_INCLUDED
#define cgdwarf_INCLUDED

#include <libelf.h>
#include <libdwarf.h>
#include "dwarf_DST_mem.h"
#include "em_elf.h"

extern void Cg_Dwarf_Begin (BOOL is_64bit);

extern void Cg_Dwarf_Finish (pSCNINFO text_scninfo);

#ifdef TARG_ST // [CL] added force_emission parameter
extern void Cg_Dwarf_Add_Line_Entry (INT code_address, SRCPOS srcpos, BOOL force_emission=FALSE);
#else
extern void Cg_Dwarf_Add_Line_Entry (INT code_address, SRCPOS srcpos);
#endif

extern void Cg_Dwarf_Process_PU (Elf64_Word  scn_index,
				 LABEL_IDX   begin_label,
				 LABEL_IDX   end_label,
				 INT32       end_offset,
				 ST         *PU_st,
				 DST_IDX     pu_dst,
				 Elf64_Word  eh_index,
				 INT         eh_offset,
				 // The following two arguments need
				 // to go away once libunwind provides
				 // an interface that lets us specify
				 // ranges symbolically.
				 INT        low_pc,
				 INT        high_pc);

extern void Cg_Dwarf_Gen_Asm_File_Table (void);

extern void Cg_Dwarf_Write_Assembly_From_Symbolic_Relocs(FILE *asm_file,
							 INT   section_count,
							 BOOL  is_64bit);

extern INT Offset_from_FP (ST *st);

typedef enum {
  CGD_ELFSYM,
  CGD_LABIDX
} CGD_SYMTAB_ENTRY_TYPE;

extern Dwarf_Unsigned
Cg_Dwarf_Symtab_Entry(CGD_SYMTAB_ENTRY_TYPE  type,
		      Dwarf_Unsigned         index,
		      Dwarf_Unsigned         pu_base_sym_idx = 0,
		      PU_IDX                 pu = (PU_IDX) 0,
		      char                  *label_name = NULL,
		      Dwarf_Unsigned         last_offset = 0);

extern void Cg_Dwarf_Translate_To_Elf(Dwarf_Unsigned,
				      Dwarf_Unsigned *,
				      Dwarf_Unsigned *);

#ifdef TARG_ST
//TB: export these type for accessing file_table and incl_table for
//gcov support
typedef struct {
  char *path_name;
  BOOL already_processed;
} include_info;
  
typedef struct {
  char *filename;
  INT incl_index;
  FILE *fileptr;
  INT max_line_printed;
  BOOL already_processed;
  Dwarf_Unsigned mod_time;
  Dwarf_Unsigned file_size;
} file_info;

extern file_info Cg_Dwarf_File_Table(INT /* idx */);
extern include_info Cg_Dwarf_Include_Table(INT /* idx */);

#endif /* TARG_ST */
#endif /* cgdwarf_INCLUDED */
