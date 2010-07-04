/*

  Copyright (C) 2000 Silicon Graphics, Inc.  All Rights Reserved.

   Copyright (C) STMicroelectronics All Rights Reserved.

   Path64 is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   Path64 is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with Path64; see the file COPYING.  If not, write to the Free
   Software Foundation, 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.

*/


#ifndef targ_em_elf_INCLUDED
#define targ_em_elf_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include <elf.h>

struct section_info {
    Elf_Scn *scnptr;		/* ptr to the elf section. */
    char *buffer;		/* data buffer for section. */
    Elf64_Xword limit;		/* maximum size of data buffer. */
    Elf64_Xword size;		/* current size of data buffer. */
    Elf64_Xword offset;		/* current offset in data buffer. */
    Elf64_Word align;		/* alignment of the section. */
    Elf64_Word scnidx;		/* symbol index of the section symbol. */
    pSCNINFO relinfo;		/* associated REL section. */
    pSCNINFO relainfo;		/* associated RELA section. */
    pSCNINFO events;		/* associated EVENTS section.  */
    Elf64_Word ev_offset;	/* offset of last entry in events scn. */
    pSCNINFO contents;		/* associated CONTENTS section. */
    Elf64_Word con_offset;	/* offset of last entry in contents scn. */
}; 

#define SCNINFO_scnptr(t)	((t)->scnptr)
#define SCNINFO_buffer(t)	((t)->buffer)
#define SCNINFO_limit(t)	((t)->limit)
#define SCNINFO_size(t)		((t)->size)
#define SCNINFO_offset(t)	((t)->offset)
#define SCNINFO_align(t)	((t)->align)
#define SCNINFO_scnidx(t)	((t)->scnidx)
#define SCNINFO_relinfo(t)	((t)->relinfo)
#define SCNINFO_relainfo(t)	((t)->relainfo)
#define SCNINFO_events(t)	((t)->events)
#define SCNINFO_ev_offset(t)	((t)->ev_offset)
#define SCNINFO_contents(t)	((t)->contents)
#define SCNINFO_con_offset(t)	((t)->con_offset)

#define SCNINFO_index(t)	(elf_ndxscn(SCNINFO_scnptr(t)))

extern pSCNINFO Interface_Scn;

/* this defines the common, other than name and enum value,
 *  parts of code for both IA-64 and Mips
 */

  /*
   * relocations that are the same other than the enum
   */
#define R_WORD32	(Big_Endian ? R_IA_64_DIR32MSB : R_IA_64_DIR32LSB)
#define R_WORD64	(Big_Endian ? R_IA_64_DIR64MSB : R_IA_64_DIR64LSB)
#define R_SCN_DISP	(Big_Endian ? R_IA_64_SECREL64MSB : R_IA_64_SECREL64LSB)
#define R_PLT_OFFSET      R_IA_64_PLTOFF22
#define R_NONE            R_IA_64_NONE

  /*
   * section flags that are the same other than the enum
   */
#ifndef SHF_MERGE
  /* May be defined in recent elf.h. */
#define SHF_MERGE         SHF_IRIX_MERGE
#endif
#ifndef SHF_NOSTRIP
  /* May be defined in recent elf.h. */
#define SHF_NOSTRIP       SHF_MIPS_NOSTRIP
#endif
  /*
   * section types that are the same other than the enum
   */
#define SHT_EVENTS        SHT_IA64_EVENTS
#define SHT_CONTENT       SHT_IA64_CONTENT
#define SHT_IFACE         SHT_IA64_IFACE

  /*
   * section names
   */
#define SECT_OPTIONS_NAME IA64_OPTIONS
#define SECT_EVENTS_NAME  IA64_EVENTS
#define SECT_IFACE_NAME   IA64_INTERFACES
#define SECT_CONTENT_NAME MIPS_CONTENT
  
inline void
Set_Elf_Version (unsigned char *e_ident)
{
	/* temporary version until final objects */
	e_ident[EI_TVERSION] = EV_T_CURRENT;
}

#ifdef __cplusplus
}
#endif
#endif /* targ_em_elf_INCLUDED */

