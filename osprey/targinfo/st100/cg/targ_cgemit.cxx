/*

  Copyright (C) 2002 ST Microelectronics, Inc.  All Rights Reserved.

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

  Contact information:

  For further information regarding this notice, see:

*/

#include <elf.h>

#define	USE_STANDARD_TYPES 1
#include "defs.h"
#include "targ_isa_lits.h"
#include "vstring.h"
#include "config_asm.h"
#include "em_elf.h"
#include "dwarf_DST_mem.h"         /* for DST_IDX */
#include "symtab.h"
#include "cgir.h"
#include "cgemit.h"
#include "cgemit_targ.h"
#include "data_layout.h"
#include "bb.h"
#include "op.h"
/* #include "iface_scn.h" */
#include "cg_flags.h"
#include "glob.h"

/* ====================================================================
 *    CGEMIT_Weak_Alias (sym, stringsym)
 *
 * ====================================================================
 */
void
CGEMIT_Weak_Alias (
  ST *sym, 
  ST *strongsym
) 
{
  fprintf (Asm_File, "\t%s\t%s#\n", AS_WEAK, ST_name(sym));
  fprintf (Asm_File, "\t.set %s#, %s#\n", ST_name(sym), ST_name(strongsym));
}

/* ====================================================================
 *    CGEMIT_Alias (sym, stringsym)
 *
 * ====================================================================
 */
void
CGEMIT_Alias (ST *sym, ST *strongsym) 
{
  fprintf (Asm_File, "\t.set %s#, %s#\n", ST_name(sym), ST_name(strongsym));
}

/* ====================================================================
 *   CGEMIT_Prn_Ent_In_Asm
 *
 *   Generate the entry (.proc) directive.
 * ====================================================================
 */
void 
CGEMIT_Prn_Ent_In_Asm (
  ST *pu
)
{
  BB_LIST *ent_list;

  fprintf (Asm_File, "\t%s ", AS_ENT);

  for (ent_list = Entry_BB_Head; 
       ent_list; 
       ent_list = BB_LIST_rest(ent_list)) {
    BB *bb = BB_LIST_first(ent_list);
    ANNOTATION *ant = ANNOT_Get (BB_annotations(bb), ANNOT_ENTRYINFO);
    ENTRYINFO *ent = ANNOT_entryinfo(ant);
    ST *entry_sym = ENTRYINFO_name(ent);

    if (!ST_is_not_used(entry_sym)) {
      const char *entry_name = ST_name(entry_sym);
      if (strcmp(Cur_PU_Name, entry_name ) != 0) {
	fprintf (Asm_File, ", %s", entry_name);
      }
    }
  }

  fprintf (Asm_File, "\n");

  return;
}

/* ====================================================================
 *    CGEMIT_Exit_In_Asm ()
 * ====================================================================
 */
void
CGEMIT_Exit_In_Asm (ST *pu) 
{
  fprintf (Asm_File, "\t%s\t", AS_END);
  EMT_Write_Qualified_Name(Asm_File, pu);
  fprintf (Asm_File, "\n");
}

/* ====================================================================
 *   CGEMIT_Prn_Scn_In_Asm
 * ====================================================================
 */
void
CGEMIT_Prn_Scn_In_Asm (
  FILE       *asm_file,
  const char *scn_name,
  Elf64_Word  scn_type,
  Elf64_Word  scn_flags,
  Elf64_Xword scn_entsize,
  Elf64_Word  scn_align,
  const char *cur_scn_name
)
{
  char scn_flags_string[5];
  char scn_type_string[10];  // min of strlen("progbits") + 1
  char *p = &scn_flags_string[0];

#if 0
  fprintf (asm_file, "\n\t%s %s,", AS_SECTION, scn_name);
  if (scn_flags & SHF_WRITE) *p++ = 'w';
  if (scn_flags & SHF_ALLOC) *p++ = 'a';
  if (scn_flags & SHF_EXECINSTR) *p++ = 'x';
  // short sections are only recognized by name, not by "s" qualifier
  // if (scn_flags & SHF_IRIX_GPREL) *p++ = 's';
  *p = '\0'; // null terminate the string.
  fprintf (asm_file, " \"%s\",", scn_flags_string);

  p = &scn_type_string[0];
  if (scn_type == SHT_NOBITS) {
    strcpy(p, "nobits");
  }
  else if (scn_type == SHT_PROGBITS) {
    strcpy(p, "progbits");
  }
  else {
    DevWarn("Intel assembler definition inadequate for "
	    "ELF section type 0x%llx; using \"progbits\"", (UINT64)scn_type);
    strcpy(p, "progbits");
  }
  fprintf (asm_file, " \"%s\"\n", scn_type_string);
#if 0   // contrary to document, it should be align, not power of it.
  UINT32 tmp, power;
  power = 0;
  for (tmp = scn__align; tmp > 1; tmp >>= 1) power++;
  fprintf (asm_file, "\t%s\t%d\n", AS_ALIGN, power);
#else
  fprintf (asm_file, "\t%s\t%d\n", AS_ALIGN, scn_align);  
#endif

#endif
}

/* ====================================================================
 *   CGEMIT_Prn_Scn_In_Asm
 * ====================================================================
 */
void
CGEMIT_Prn_Scn_In_Asm (
  ST *st, 
  Elf64_Word scn_type, 
  Elf64_Word scn_flags,
  Elf64_Xword scn_entsize, 
  ST *cur_section
)
{
  CGEMIT_Prn_Scn_In_Asm(Asm_File, ST_name(st), scn_type, scn_flags,
			scn_entsize, STB_align(st),
			cur_section != NULL ? ST_name(cur_section) : NULL);

}

/* ====================================================================
 *   CGEMIT_Prn_Line_Dir_In_Asm
 * ====================================================================
 */
void
CGEMIT_Prn_Line_Dir_In_Asm (
  USRCPOS usrcpos
)
{
  if(!CG_emit_asm_dwarf) {
    fprintf (Asm_File, ASM_CMNT_LINE); //turn the rest into comment
  }
  fprintf (Asm_File, "\t.loc\t%d\t%d\t%d\n", 
		USRCPOS_filenum(usrcpos)-1,
		USRCPOS_linenum(usrcpos),
		USRCPOS_column(usrcpos));
}


/* ====================================================================
 *   CGEMIT_Prn_File_Dir_In_Asm
 * ====================================================================
 */
void
CGEMIT_Prn_File_Dir_In_Asm(
  USRCPOS usrcpos,
  const char *pathname,
  const char *filename
)
{
  if(!CG_emit_asm_dwarf) {
    fprintf (Asm_File, "// "); //turn the rest into comment
  }
  fprintf (Asm_File, "\t%s\t%d \"%s/%s\"\n", AS_FILE, 
		USRCPOS_filenum(usrcpos)-1,
		pathname,filename);
}

/* ====================================================================
 *   CGEMIT_Prn_File_Dir_In_Asm
 *
 *   whether to use the base st for the reloc
 * ====================================================================
 */
extern BOOL
CGEMIT_Use_Base_ST_For_Reloc (
  INT reloc, 
  ST *st
)
{
  return ST_is_export_local(st);
}
