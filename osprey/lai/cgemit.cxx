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

#include <ctype.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "W_errno.h"
#include "W_bstring.h"
#include <elf.h>
#include <libelf.h>
#include <elfaccess.h>
#include "W_alloca.h"
#include <stdlib.h>
#include <unistd.h> // for unlink()
#include <cmplrs/rcodes.h>
#include <stamp.h>
#include <vector>
// [HK]
#if __GNUC__ >= 3
#include <assert.h>
#endif //  __GNUC__ >= 3

#define	USE_STANDARD_TYPES 1
#include "defs.h"
#include "tracing.h"
// [HK]
#if __GNUC__ >= 3
#include <ext/hash_map>
#else
#include "hash_map.h"
#endif // __GNUC__ >= 3
#include "namespace_trans.h"

#include "config.h"
#include "config_asm.h"
#include "config_TARG.h"
#include "config_debug.h"
#include "config_list.h"
#include "vstring.h"
#ifdef TARG_ST
#include "file_util.h" // [CL] for New_Extension
#endif
#include "glob.h"
#include "xstats.h"
#include "targ_const.h"
#include "strtab.h"
#include "symtab.h"
#include "tag.h"
#include "wn.h"

#include "stblock.h"
#include "data_layout.h"
#include "sections.h"
#include "label_util.h"
#include "note.h"
#include "freq.h"

#include "erglob.h"
#include "erlib.h"
#include "em_elf.h"
#include "dwarf_DST_mem.h"         /* for DST_IDX */

#include "whirl2ops.h"		   /* for Get_WN_From_Memory_OP */
#include "cg.h"
#include "cg_flags.h"
#include "calls.h"                 /* for Frame_Len */
#include "cgexp.h"                 /* for Exp_Simulated_OP, etc. */

#include "eh_region.h"             /* For EH_Get_PU_Range_ST */

#include "cgtarget.h"
#include "cgemit.h"
#include "cgemit_targ.h"
#include "em_elf.h"
#include "cgdwarf.h"
#include "cgdwarf_targ.h"
#include "em_dwarf.h"

#include "ti_asm.h"
#include "ti_errors.h"
#include "targ_proc_properties.h"
#include "targ_abi_properties.h"
#include "targ_isa_print.h"
#include "targ_isa_enums.h"
#include "targ_isa_pack.h"
#include "targ_isa_bundle.h"
#include "targ_isa_operands.h"

#ifdef TARG_ST
/* (cbr) demangler interface */
#include "../gnu_common/include/demangle.h"

/* [SC] For ST_has_inito */
#include "be_symtab.h"
#endif

BE_EXPORTED extern void Early_Terminate (INT status);

#define PAD_SIZE_LIMIT	2048	/* max size to be padded in a section */

/* c++ mangled names can be any arbitrary length,
 * so this is just a first guess */ 
#define LBUF_LEN	(OP_MAX_FIXED_OPNDS*1024)

/* Instructions go into one of two ELF text sections depending on
 * whether the BB is in a hot or cold region. Hot BBs go into
 * ".text" (or ".text<pu-name>" for -TENV:section_for_each_function).
 * Cold BBs go into ".text.cold". As a result, numerous cgemit
 * routines need to track the PC of both sections, and 2 element
 * arrays are typically used. Here we define the indices for
 * the arrays, with the values chosen to coincide with the result
 * of BB_Is_Cold.
 */
enum { IHOT=FALSE, ICOLD=TRUE };

/* Overload BB flag <local_flag1> to indicate if a BB is cold or hot --
 * it's less overhead than BB_Is_Cold.
 */
#define BB_cold		BB_local_flag1
#define Set_BB_cold	Set_BB_local_flag1
#define Reset_BB_cold	Reset_BB_local_flag1

#ifndef _NO_WEAK_SUPPORT_
extern const char __Release_ID[];
#else
BE_EXPORTED extern const char **__Release_ID_p;
#define __Release_ID (*__Release_ID_p)
#endif

/* ====================================================================
 *    Global data
 * ====================================================================
 */

/* ====================================================================
 *    Local data
 * ====================================================================
 */

static BOOL generate_dwarf = FALSE;
static BOOL generate_elf_symbols = FALSE;

/* Local phase trace flags: */
static BOOL Trace_Init    = FALSE;	/* Data initialization trace */
static BOOL Trace_Inst    = FALSE;	/* Noop-insertion trace */
static BOOL Trace_Elf     = FALSE;	/* Miscellaneous ELF trace */
#ifdef TARG_ST
static BOOL Trace_Sched   = FALSE;	// Trace issue dates and bundle count. */
#endif
// static BOOL Trace_Longbr  = FALSE;	/* trace handling of long branches */

static FILE *Output_File;               /* write to Asm or Lai file */

BOOL Use_Page_Zero = FALSE;
static BOOL Use_Prefetch = FALSE;

// For dwarf output in assembly we need to keep track of the 
// offset of the current instruction from a known label.
// If Last_Label is LABEL_IDX_ZERO, it is assumed to be invalid.
static LABEL_IDX Last_Label = LABEL_IDX_ZERO;
static INT32 Offset_From_Last_Label = 0;

static LABEL_IDX Initial_Pu_Label;

/* NOTE: PCs are actually a bundle address and slot number pair. The 
 * slot number is in the low bits. 
 */
static INT32 PC = 0;			/* PC for current text section */
static INT32 text_PC = 0;		/* PC for hot text */
static INT32 cold_PC = 0;		/* PC for cold text */

static ST *PU_base = NULL;		/* base for current text section */
static ST *text_base = NULL;		/* base for hot text section */
static ST *cold_base = NULL;		/* base for cold text section */

static pSCNINFO PU_section = NULL;	/* current text section */
static pSCNINFO text_section = NULL;	/* hot text section */
static pSCNINFO cold_section = NULL;	/* cold text section */

static INT current_rid = 0;	/* current rid id */

typedef struct {
  pSCNINFO scninfo;
  Elf64_Word scn_ofst;	/* [CG] keeps track of current section offset. */
  ST *sym;
#ifdef TARG_ST100
  char *label;     // we need to label sections on some targets that
                   // do not have section relative relocations
#endif
} AUX_SCN;

static AUX_SCN *em_scn;
static INT last_scn = 0;	
static INT max_scn = 0;
/* 0 index to em_scn will be null values */

static ST *cur_section = NULL;

static PU_IDX current_pu = 0;

#ifdef TARG_ST
static int Bundle_Count;
// [SC]: Symbol_Def_After_Group is a symbol that should
// be defined immediately after the current group.
static ST *Symbol_Def_After_Group;
#endif

/* ====================================================================
 *    Use_Separate_PU_Section
 *
 *    put PU in separate elf text section? (pu could be null if no PUs)
 * ====================================================================
 */
#define Use_Separate_PU_Section(pu,st) \
	((pu != (PU_IDX) NULL) \
	&& (Section_For_Each_Function || PU_in_elf_section(pu)) \
	&& (ST_class(st) == CLASS_BLOCK) \
	&& (strcmp(ST_name(st), ELF_TEXT) == 0))


static char ism_name[40];

/* ====================================================================
 *    Get_Ism_Name ()
 *
 *    get ism name from __Release_ID 
 *    TODO: same code as in ../cg/cgemit.cxx -- factorize ?
 * ====================================================================
 */
static void
Get_Ism_Name (void)
{
  char *s = strchr(__Release_ID, ':');
  char *p;

  if (s == NULL) {
    ism_name[0] = '\0';
    return;
  }
  s++;	/* skip : */
  s = strchr(s, ':');
  if (s == NULL) {
    ism_name[0] = '\0';
    return;
  }
  s++;	/* skip : */
  p = strchr(s, ' ');
  if (p == NULL) {
    ism_name[0] = '\0';
    return;
  }
  strncpy (ism_name, s, p-s);
  ism_name[p-s] = '\0';

  return;
}

/*
 * For the elf indexes, we can reuse the temp field for local STs,
 * Need to map from ST's to the elf index for the ST.
 * So create array of index arrays, where the ST_level
 * gets you to the index array, and then the ST_index 
 * gets you to the elf index for that ST.
 */

/*
 * Maximum nesting depth includes these scopes, in the deepest case:
 *  0 : not used
 *  1 : global scope
 *  2 : top-level PU
 *  3 : F90 internal procedure
 *  4 : MP PU from internal procedure
 *  5 : Fortran I/O in MP PU
 */

#define MAX_SYMTAB_DEPTH	6

static INT32 *elf_index_array[MAX_SYMTAB_DEPTH] =
  {NULL,NULL,NULL,NULL,NULL,NULL};
static INT max_elf_index[MAX_SYMTAB_DEPTH] = {0,0,0,0,0,0};

/* ====================================================================
 *   Allocate_Elf_Index_Space
 * ====================================================================
 */
static void
Allocate_Elf_Index_Space (
  UINT level
)
{
  INT num_sts = ST_Table_Size(level);

  if (elf_index_array[level] == NULL) {
    max_elf_index[level] = num_sts + 100;
    /* for the size take the #st's + extra space for new symbols */
    elf_index_array[level] = 
          (INT32*) Src_Alloc(sizeof(INT32) * max_elf_index[level]);
  } else if (max_elf_index[level] < num_sts + 10) {  
    /* # globals has grown, so realloc. */
    /* we realloc even if close and not yet overflowed,
     * because may alloc new symbols as part of cg. */
    elf_index_array[level] = TYPE_MEM_POOL_REALLOC_N (INT32, 
		    &MEM_src_pool, elf_index_array[level], 
			      max_elf_index[level], num_sts + 100);
    max_elf_index[level] = num_sts + 100;
  }
}

/* ====================================================================
 *   Init_ST_elf_index
 * ====================================================================
 */
static void
Init_ST_elf_index (
  UINT stab
)
{
  UINT level = stab;
  INT i;

  Allocate_Elf_Index_Space(GLOBAL_SYMTAB);
  Is_True((level < MAX_SYMTAB_DEPTH), ("Init_ST_elf_index overflow"));
  if (level > GLOBAL_SYMTAB && elf_index_array[level] != NULL) {
    /* need to clear the values in case leftover 
     * from previous uplevel */
    for (i = 0; i < max_elf_index[level]; i++) {
      elf_index_array[level][i] = 0;
    }
  }
  Allocate_Elf_Index_Space(stab);
}

/* ====================================================================
 *   ST_elf_index
 * ====================================================================
 */
static INT32
ST_elf_index (
  ST *st
)
{
  INT level = ST_level(st);

  Is_True((level < MAX_SYMTAB_DEPTH), ("ST_elf_index overflow"));
  Is_True((ST_index(st) < max_elf_index[level]), 
                                      ("ST_elf_index overflow"));
  return elf_index_array[level][ST_index(st)];
}

/* ====================================================================
 *   Set_ST_elf_index
 * ====================================================================
 */
static void
Set_ST_elf_index (
  ST *st, 
  INT32 v
)
{
  INT level = ST_level(st);

  Is_True((level < MAX_SYMTAB_DEPTH), ("Set_ST_elf_index overflow"));
  Is_True((ST_index(st) < max_elf_index[level]),
		                      ("Set_ST_elf_index overflow"));
  elf_index_array[level][ST_index(st)] = v;
}

/* ====================================================================
 *   ST_is_gp_relative
 *
 *   Determine if the symbol has a base of gp 
 * ====================================================================
 */
static BOOL
ST_is_gp_relative (
  ST *st
)
{
  ST *base_st = Base_Symbol (st);

  return ((ST_class(base_st) == CLASS_BLOCK || 
	   ST_class(base_st) == CLASS_VAR) && 
	  ST_gprel(base_st));
}

/* ====================================================================
 *    Init_Section (st)
 *
 *    Initialize a section.
 * ====================================================================
 */
#define Is_Text_Section(st) (STB_exec(st) && strncmp(ST_name(st), ELF_TEXT,5)==0)

static void
Init_Section (
  ST *st
)
{
  Elf64_Word scn_type;
  Elf64_Word scn_flags;
  Elf64_Xword scn_entsize;
  char sname[32];

  if (ST_elf_index(st) != 0) {
    /* already created */
    return;
  }

  if (last_scn >= (max_scn-1)) {
    /* allocate new block of sections */
    max_scn += 30;
    if (em_scn == NULL)
      em_scn = (AUX_SCN *)Src_Alloc(sizeof(AUX_SCN)*max_scn);
    else
      em_scn = TYPE_MEM_POOL_REALLOC_N (AUX_SCN, 
		         &MEM_src_pool, em_scn, (max_scn-30), max_scn);
  }
  last_scn++;
  Set_STB_scninfo_idx(st, last_scn);

  /* hack for .text section */
#ifdef TARG_ST
  // (cbr) exectutable sections should be text aligned
  // [CG] Minimal alignment of text segment is DEFAULT_TEXT_ALIGNMENT.
  if (STB_exec (st)) {
    int text_align;
    if (Align_Instructions) 
      text_align = Align_Instructions;
    else if (OPT_Space)
      text_align = DEFAULT_TEXT_ALIGNMENT;
    else 
      text_align = CGTARG_Text_Alignment();
    if (text_align < DEFAULT_TEXT_ALIGNMENT) text_align = DEFAULT_TEXT_ALIGNMENT;
    Set_STB_align(st, text_align);
  }
#else
  if (Is_Text_Section(st)) {
    if (Align_Instructions) 
      Set_STB_align(st, Align_Instructions);
    else if (OPT_Space)
      Set_STB_align(st, ISA_INST_BYTES);
    else
      Set_STB_align(st, CGTARG_Text_Alignment());
  }
#endif
#ifdef TARG_ST
  // [CG]: For data sections we may have an alignment
  // Currently set it to DEFAULT_DATA_ALIGNMENT
  else {
    if (STB_align(st) == 0) {
      /* (cbr) if the section is not used, no alignement has been set */
      if (STB_size(st))
        DevWarn("default alignment not set for section %s\n", ST_name(st));
      Set_STB_align(st, DEFAULT_DATA_ALIGNMENT);
    }
  }
#endif

  /* save symbol for later reference */
  em_scn[last_scn].sym = st;

#ifdef TARG_ST100
  /* (cbr) section relative offset using labels only on st100 */

  // Make a label:
  sprintf(sname, "%s_SECTION_%d", Local_Label_Prefix, last_scn);
  em_scn[last_scn].label = strdup(sname);

  if (Trace_Init) {
    fprintf(TFile, "<init>: Section %s: labeling %s\n", 
	    ST_name(em_scn[last_scn].sym), em_scn[last_scn].label);
  }
#endif

  /* assume st is CLASS_BLOCK */
  scn_type = Get_Section_Elf_Type(STB_section_idx(st));
  scn_flags = Get_Section_Elf_Flags(STB_section_idx(st));
  if (Is_Text_Section(st) && 
      current_pu != (PU_IDX) NULL && PU_in_elf_section(current_pu)) {
    scn_flags |= SHF_MIPS_NODUPE;
  }
	
  scn_entsize = Get_Section_Elf_Entsize(STB_section_idx(st));

  if (generate_elf_symbols) {
    em_scn[last_scn].scninfo = Em_New_Section (ST_name(st), 
		  scn_type, scn_flags, scn_entsize, STB_align(st));

    /* initialize elf data buffer. */
    if (!STB_nobits(st)) {
      Em_New_Data_Buffer (em_scn[last_scn].scninfo, 
			  STB_size(st) + 100, 1);
    }
    Set_ST_elf_index(st,
	      Em_Create_Section_Symbol (em_scn[last_scn].scninfo));
  }
  else {
    /* set dummy value just so don't redo this */
    Set_ST_elf_index(st, 1);
  }

#if 0
  if (Lai_Code) {
    LAI_print_section(st, scn_type, scn_flags, scn_entsize, cur_section);
  }
#endif

  if (Assembly) {
    CGEMIT_Prn_Scn_In_Asm(st, scn_type, scn_flags, scn_entsize, cur_section);
  }

  return;
}

static unsigned char
st_other_for_sym (ST *sym)
{
  unsigned char symother;

  switch (ST_export(sym)) {
    case EXPORT_HIDDEN:
      symother = STO_HIDDEN;
      break;
    case EXPORT_PROTECTED:
      symother = STO_PROTECTED;
      break;
    case EXPORT_INTERNAL:
      symother = STO_INTERNAL;
      break;
    case EXPORT_OPTIONAL:
      symother = STO_OPTIONAL;
      break;
    default:
      symother = STO_DEFAULT;
      break;
  }
  return symother;
}

static INT64
Get_Offset_From_Full (ST *sym)
{
	/* full-split symbols have names of form "full_.offset"
	 * we need to extract the offset and pass that in value field.
	 * An alternative would be to search type structure for
	 * matching fields, but this is probably faster.
	 */
	char *offset_string;
        offset_string = strrchr (ST_name(sym), '.');
        FmtAssert(offset_string != NULL, ("Get_Offset_From_Full unexpected name format (%s)", ST_name(sym)));
        offset_string++;       /* skip the period */
	/* Remove atoll to favor cross-compilability: Except for behavior on  error, atoll() is equivalent to strtoll(str, (char **)NULL, 10) */
	return strtoll(offset_string, (char **)NULL, 10) ;
}

/***********************************************************************
 *
 * Instruction-PC utilities.
 *
 * On architectures that support bundling, we use a PC representation
 * that enables us to point at instructions within the bundle.
 * We accomplish this by using the low bits of the bundle address
 * (which are normally zero because of alignment constraints) to
 * hold the instruction offset, i.e. slot number, within the bundle.
 *
 * The following utility routines provide for updating and accessing
 * the composite PC values.
 *
 ***********************************************************************
 */

/* Given a composite PC, return the bundle address component.
 */
inline INT32 PC_Bundle(INT32 pc)
{
  return pc & ~(ISA_INST_BYTES - 1);
}

/* Given a composite PC, return the slot number component.
 */
inline INT32 PC_Slot(INT32 pc)
{
  return pc & (ISA_INST_BYTES - 1);
}

#ifdef TARG_ST
/* Given a composite PC, return the corresponding physical address */
inline INT32 PC2Addr(INT32 pc)
{
  return PC_Bundle(pc) + PC_Slot(pc)*ISA_INST_BYTES/ISA_MAX_SLOTS;
}

/* Given a physical address, return the corresponding composite PC */
inline INT32 Addr2PC(INT32 addr)
{
  // Note that there is no equivalence with the previous tranformation
  INT32 bundle = PC_Bundle(addr);
  INT32 slot = ((addr - bundle) * ISA_MAX_SLOTS) / ISA_INST_BYTES;
  return bundle + slot;
}
#endif

/* ====================================================================
 *   PC_Incr
 *
 *   Increment a composite PC by 1. The slot number is incremented, 
 *   and if it overflows, the bundle address is adjusted as well.
 * ====================================================================
 */
inline INT32 
PC_Incr (
  INT32 pc
)
{
  ++pc;

  if (PC_Slot(pc) == ISA_MAX_SLOTS) {
    pc += ISA_INST_BYTES - ISA_MAX_SLOTS;
  }

  return pc;
}

/* ====================================================================
 *   PC_Incr_N
 *
 *   Same as PC_Incr except the increment is an arbitrary constant.
 * ====================================================================
 */
inline INT32 
PC_Incr_N (
  INT32 pc, 
  UINT32 incr
)
{
  UINT slots = PC_Slot(pc) + incr;
  UINT bundles = slots / ISA_MAX_SLOTS;
  pc = PC_Bundle(pc) + (bundles * ISA_INST_BYTES) + (slots % ISA_MAX_SLOTS);
  return pc;
}

#ifdef TARG_ST
/* ====================================================================
 *   PC_Align_N
 *
 *   Align the PC to the given byte boundary
 * ====================================================================
 */
inline INT32 
PC_Align_N (
  INT32 pc, 
  UINT32 align
)
{
  INT32 addr = PC2Addr(pc);
  INT32 new_addr = (addr+align-1)/align*align;
  pc = Addr2PC(new_addr);
  return pc;
}
#endif

/* ====================================================================
 *    r_qualified_name
 * ====================================================================
 */
static void
r_qualified_name (
  ST *st,
  vstring *buf       /* buffer to format it into */
)
{
#if 0
  //
  // Arthur: now there is a possibility that we try to use this
  //         function with a section st
  //
  if (ST_class(st) == CLASS_BLOCK && STB_section(st)) {
    //
    // emit this section's label
    //
    vstr_sprintf (buf, vstr_len(*buf), "%s", 
		  em_scn[STB_scninfo_idx(st)].label);
    return;
  }
#endif

  if (ST_name(st) && *(ST_name(st)) != '\0') {
    *buf = vstr_concat(*buf, ST_name(st));
    if (ST_is_export_local(st) && ST_class(st) == CLASS_VAR) {
      // local var, but being written out.
      // so add suffix to help .s file distinguish names.
      // assume that statics in mult. pu's will 
      // get moved to global symtab, so don't need pu-num
      if (ST_level(st) == GLOBAL_SYMTAB) {
#ifdef TARG_ST
	// [CG] If not under -ipa mode we do not need to rename static
	// in the global symtab.
	// [CG] Force use of  '.' to ensure no name clashing
	if (Emit_Global_Data || Read_Global_Data)
	  vstr_sprintf (buf, vstr_len(*buf), 
			"%s%d", ".", ST_index(st));
#else
	vstr_sprintf (buf, vstr_len(*buf), 
                    "%s%d", Label_Name_Separator, ST_index(st));
#endif
      } else {
#ifdef TARG_ST
	// [CG] Force use of  '.' to ensure no name clashing
	vstr_sprintf (buf, vstr_len(*buf), 
		      "%s%d%s%d", ".", ST_pu(Get_Current_PU_ST()),
		           ".", ST_index(st) );
#else
	vstr_sprintf (buf, vstr_len(*buf), 
		      "%s%d%s%d", Label_Name_Separator, ST_pu(Get_Current_PU_ST()),
		           Label_Name_Separator, ST_index(st) );
#endif
      }
    }
    else if (*Symbol_Name_Suffix != '\0') {
      *buf = vstr_concat(*buf, Symbol_Name_Suffix);
    }
  } else {
    vstr_sprintf (buf, vstr_len(*buf), 
                 "%s %+lld", ST_name(ST_base(st)), ST_ofst(st));
  }
}

/* ====================================================================
 *    EMT_Write_Qualified_Name (FILE *f, ST *st)
 * ====================================================================
 */
void
EMT_Write_Qualified_Name (
  FILE *f, 
  ST *st
)
{
  vstring buf = vstr_begin(LBUF_LEN);
  r_qualified_name (st, &buf);
  fprintf (f, "%s", vstr_str(buf));
  vstr_end(buf);
}

#ifdef TARG_ST
// [CL] helper functions to generate anonymous constants
/* ====================================================================
 *    r_qualified_tcon_name
 * ====================================================================
 */
static void
r_qualified_tcon_name (
  ST *st,
  vstring *buf       /* buffer to format it into */
)
{
  if (ST_level(st) == GLOBAL_SYMTAB) {
    vstr_sprintf (buf, vstr_len(*buf), 
                 "%s_UNNAMED_CONST_%s_%d%s%d", Local_Label_Prefix,
		  Ipa_Label_Suffix,
		 ST_tcon(st), Label_Name_Separator, ST_index(st));
  } else {
    vstr_sprintf (buf, vstr_len(*buf), 
                 "%s_UNNAMED_CONST_%s_%d%s%d%s%d", Local_Label_Prefix,
		  Ipa_Label_Suffix,
		  ST_tcon(st), Label_Name_Separator,
		  ST_pu(Get_Current_PU_ST()),
		  Label_Name_Separator, ST_index(st));
  }
}

/* ====================================================================
 *    EMT_Write_Qualified_Name (FILE *f, ST *st)
 * ====================================================================
 */
void
EMT_Write_Qualified_Tcon_Name (
  FILE *f, 
  ST *st
)
{
  vstring buf = vstr_begin(LBUF_LEN);
  r_qualified_tcon_name (st, &buf);
  fprintf (f, "%s", vstr_str(buf));
  vstr_end(buf);
}

/* ====================================================================
 *    EMT_Visibility
 *
 * If the assembler can support the ELF visibilities (i.e. if
 * AS_HIDDEN et al are defined) then use them.
 *
 * ====================================================================
 */
static
void EMT_Visibility (
  FILE *f,			
  ST_EXPORT eclass,
  ST *st
  )
{
  const char *dir;

  switch (eclass)
    {
#ifdef AS_INTERNAL
    case EXPORT_INTERNAL:
      dir = AS_INTERNAL;
      break;
#endif
#ifdef AS_HIDDEN
    case EXPORT_HIDDEN:
      dir = AS_HIDDEN;
      break;
#endif
#ifdef AS_PROTECTED
    case EXPORT_PROTECTED:
      dir = AS_PROTECTED;
      break;
#endif
    default:
      dir = 0;
      break;
    }

  if (dir) {
    fprintf (f, "\t%s\t", dir);
    EMT_Write_Qualified_Name (f, st);
    fprintf (f, "\n");
  }
    
}
#endif

/* ====================================================================
 *    Print_Dynsym (pfile, st)
 *
 *    print the internal, hidden or protected attributes if present 
 * ====================================================================
 */
static void 
Print_Dynsym (
  FILE *pfile, 
  ST *st
)
{
  if (AS_DYNSYM) {
    fprintf (pfile, "\t%s\t", AS_DYNSYM);
    EMT_Write_Qualified_Name (pfile, st);
    switch (ST_export(st)) {
      case EXPORT_INTERNAL:
	fprintf (pfile, "\tsto_internal\n");
	break;
      case EXPORT_HIDDEN:
	fprintf (pfile, "\tsto_hidden\n");
	break;
      case EXPORT_PROTECTED:
	fprintf (pfile, "\tsto_protected\n");
	break;
      case EXPORT_OPTIONAL:
	fprintf (pfile, "\tsto_optional\n");
	break;
      default:
	fprintf (pfile, "\tsto_default\n");
	break;
    }
  }
}

/* ====================================================================
 *    Print_Label (pfile, st, size)
 * ====================================================================
 */
static void 
Print_Label (
  FILE *pfile, 
  ST *st, 
  INT size
)
{
  ST *base_st;
  INT64 base_ofst;

  if (ST_is_weak_symbol(st)) {
    fprintf (pfile, "\t%s\t", AS_WEAK);
    EMT_Write_Qualified_Name(pfile, st);
    fprintf(pfile, "\n");
  }
  else if (!ST_is_export_local(st)) {
    fprintf (pfile, "\t%s\t", AS_GLOBAL);
    EMT_Write_Qualified_Name(pfile, st);
    fprintf(pfile, "\n");
    EMT_Visibility (pfile, ST_export(st), st);
  }
#ifdef TARG_ST
  // [CL] handle the case of generation of symtab.s
  // in IPA mode. Although local, such symbols must
  // be 'promoted' to global so that they can be
  // accessed by the module they belong to
  else if (ST_is_export_local(st) && Emit_Global_Data) {
    fprintf (pfile, "\t%s\t", AS_GLOBAL);
    EMT_Write_Qualified_Name(pfile, st);
    fprintf(pfile, "\n");
    // [CL] output as hidden/internal so that we are close to the
    // export_local meaning
#ifdef AS_INTERNAL
    if (ST_export(st) == EXPORT_LOCAL_INTERNAL) {
      fprintf (pfile, "\t%s\t", AS_INTERNAL);
      EMT_Write_Qualified_Name(pfile, st);
      fprintf(pfile, "\n");
    } else
#endif
#ifdef AS_HIDDEN
      {
	fprintf (pfile, "\t%s\t", AS_HIDDEN);
	EMT_Write_Qualified_Name(pfile, st);
	fprintf(pfile, "\n");
      }
#endif
#endif
  }
  if (ST_class(st) == CLASS_VAR) {
    fprintf (pfile, "\t%s\t", AS_TYPE);
    EMT_Write_Qualified_Name (pfile, st);
#ifdef TARG_ST
#  ifdef AS_MOVEABLE
    BOOL moveable;

    if (Emit_Global_Data) {
      /* TB: Tell this symbol is moveable when not in Emit_Global_Data (ipa) */
      moveable = FALSE;
    }
    else if (ST_is_initialized (st)) {
      /* [SC]: Not moveable if initializer is larger than symbol size */
      INITV_IDX inito_idx = ST_has_inito (st);
      if (inito_idx != 0 && Get_INITO_Size (inito_idx) > size)
	moveable = FALSE;
      else
	moveable = TRUE;
    }
    else {
      moveable = TRUE;
    }
    
    if (moveable) {
      fprintf (pfile, ", %s, %s", AS_TYPE_OBJECT, AS_MOVEABLE);
    }
    else {
      fprintf (pfile, ", %s", AS_TYPE_OBJECT);
    }
#  ifdef AS_USED
    /* TB: add gnu used  attibute when needed */
    if (ST_is_used(st)) {
      fprintf (pfile, ", %s", AS_USED);
    }
#endif
    fprintf (pfile, "\n");
#  else
    fprintf (pfile, ", %s\n", AS_TYPE_OBJECT);
#  endif
#else
    fprintf (pfile, ", %s\n", AS_TYPE_OBJECT);
#endif    
  }
  if (size != 0) {
    /* if size is given, then emit value for asm */
    fprintf (pfile, "\t%s\t", AS_SIZE);
    EMT_Write_Qualified_Name(pfile, st);
    fprintf (pfile, ", %d\n", size);
  }
  Base_Symbol_And_Offset (st, &base_st, &base_ofst);
  EMT_Write_Qualified_Name (pfile, st);
#ifdef TARG_ST
  if (List_Notes) {
    fprintf (pfile, ":\t%s 0x%llx\n", ASM_CMNT, base_ofst);
  } else {
    fprintf (pfile, ":\n");
  }
#else
  fprintf (pfile, ":\t%s 0x%llx\n", ASM_CMNT, base_ofst);
#endif
  Print_Dynsym (pfile, st);
}

/* ====================================================================
 *    Print_Common
 * ====================================================================
 */
static void
Print_Common (
  FILE *pfile, 
  ST *st
)
{
  ST    *base_st;
  INT64  base_ofst;

  Base_Symbol_And_Offset (st, &base_st, &base_ofst);
  if (st != base_st && ST_sclass(base_st) == SCLASS_COMMON) {
    // use base common
    if (ST_elf_index(base_st) == 0) {
      Print_Common (pfile, base_st);
    }
    return;
  }
  
  if (TY_size(ST_type(st)) > 0) {

    if (ST_is_weak_symbol(st)) {
      fprintf ( pfile, "\t%s\t", AS_WEAK);
      EMT_Write_Qualified_Name(pfile, st);
      fprintf ( pfile, "\n");
    }
#ifdef TARG_ST
    else
      // clarkes 090307
      EMT_Visibility (pfile, ST_export(st), st);
#endif
    fprintf ( pfile, "\t%s\t", AS_COM);
    EMT_Write_Qualified_Name(pfile, st);
    fprintf (pfile, ", %lld, %d\n", 
		     TY_size(ST_type(st)), TY_align(ST_type(st)));
    Print_Dynsym (pfile, st);

#ifdef TARG_ST
#  ifdef AS_MOVEABLE
    /* TB: Tell this symbol is moveable when not in Emit_Global_Data (ipa) */
    if (!Emit_Global_Data) {
      fprintf (pfile, "\t%s\t", AS_TYPE);
      EMT_Write_Qualified_Name (pfile, st);
      fprintf (pfile, ", %s, %s\n", AS_TYPE_OBJECT, AS_MOVEABLE);
    }
#endif
#endif
    // this is needed so that we don't emit commons more than once
    Set_ST_elf_index(st, 1);
  }
}

/* ====================================================================
 *    EMT_Put_Elf_Symbol (sym)
 * 
 *    Add a symbol to the ELF symbol table if it hasn't been added 
 *    already.
 * ====================================================================
 */
mINT32
EMT_Put_Elf_Symbol (
  ST *sym
)
{

  unsigned char symbind;
  unsigned char symother;
  Elf64_Word symindex;
  TY_IDX sym_type;
  ST *base_st;
  INT64 base_ofst = 0;
  ST_SCLASS sclass;

  symindex = ST_elf_index(sym);

  /* check if symbol is already there. */
  if (symindex != 0) return symindex;

  if ( Trace_Elf ) {
    #pragma mips_frequency_hint NEVER
    fprintf ( TFile, "EMT_Put_Elf_Symbol:\n" );
    Print_ST ( TFile, sym, FALSE );
  }

  if ( ! generate_elf_symbols) {
    // if only .s file, then just do dummy mark that we have
    // seen this symbol and emitted any type info for it.
    if (ST_class(sym) == CLASS_FUNC) {
      if (Assembly || Lai_Code) {
#ifdef TARG_ST
        // (cbr)
	  if (ST_is_weak_symbol(sym)) {
	    fprintf ( Asm_File, "\t%s\t", AS_WEAK);
	    EMT_Write_Qualified_Name(Asm_File, sym);
	    fprintf ( Asm_File, "\n");
	  }
	  // clarkes 090307
	  else
	    EMT_Visibility (Asm_File, ST_export(sym), sym);
#endif
	fprintf (Asm_File, "\t%s\t", AS_TYPE);
	EMT_Write_Qualified_Name (Asm_File, sym);
	fprintf (Asm_File, ", %s\n", AS_TYPE_FUNC);
      }
    }
    else if (ST_class(sym) == CLASS_VAR && 
	     ST_sclass(sym) == SCLASS_COMMON) {
      if (Assembly || Lai_Code) {
	Print_Common (Asm_File, sym);
      }
    }
#ifdef TARG_ST
    // (cbr) need to emit .weak for extern symbols as well
    else if (ST_class(sym) == CLASS_VAR &&
	     ST_sclass(sym) == SCLASS_EXTERN) {
      if (Assembly || Lai_Code) {
        if (ST_is_weak_symbol(sym)) {
          fprintf ( Asm_File, "\t%s\t", AS_WEAK);
          EMT_Write_Qualified_Name(Asm_File, sym);
          fprintf ( Asm_File, "\n");
        }
	// clarkes 090307
	else {
	  EMT_Visibility ( Asm_File, ST_export(sym), sym);
	}
      }
    }
#endif
    Set_ST_elf_index(sym, 1);
    return 0;
  }

  Is_True (!ST_is_not_used(sym) || ST_emit_symbol(sym), 
	      ("Reference to not_used symbol (%s)", ST_name(sym)));

  /* set the symbol binding. */
  if (ST_is_weak_symbol(sym)) {
    symbind = STB_WEAK;
  }
  else if (ST_is_export_local(sym)) {
    symbind = STB_LOCAL;
  }
  else {
    symbind = STB_GLOBAL;
  }

  symother = st_other_for_sym (sym);
  Base_Symbol_And_Offset (sym, &base_st, &base_ofst);
  // check if base is new section symbol that is not initialized yet
  if (ST_class(base_st) == CLASS_BLOCK && STB_section(base_st)
	&& ST_elf_index(base_st) == 0)
  {
	Init_Section(base_st);
  }

  if (ST_is_weak_symbol(sym) && ST_sclass(base_st) == SCLASS_EXTERN) {
	// ipa can cause it to be based on global extern,
	// in which case treat it as an extern
	sclass = ST_sclass(base_st);
  }
  else {
	sclass = ST_sclass(sym);
  }
  switch (ST_class(sym)) {
    case CLASS_VAR:
      sym_type = ST_type(sym);	// only valid for VARs
      switch (sclass) {
	case SCLASS_FSTATIC:
	case SCLASS_DGLOBAL:
	case SCLASS_UGLOBAL:
#ifdef TARG_ST
      case SCLASS_PSTATIC:
	  // [CL] looks like r_qualified_name()
	  // When generating debug information, keep in mind that
	  // static variables have been renamed
	  if (ST_is_export_local(sym)) {
	    // local var, but being written out.
	    // so add suffix to help .s file distinguish names.
	    // assume that statics in mult. pu's will 
	    // get moved to global symtab, so don't need pu-num

	    char local_name[strlen(ST_name(sym))+strlen(Label_Name_Separator)+20];
	    if (ST_level(sym) == GLOBAL_SYMTAB)
	      sprintf (local_name, "%s%s%d", ST_name(sym), Label_Name_Separator, ST_index(sym));
	    else
	      sprintf (local_name, "%s%s%d%s%d", ST_name(sym), Label_Name_Separator, ST_pu(Get_Current_PU_ST()), Label_Name_Separator, ST_index(sym) );

	    symindex = Em_Add_New_Symbol (
					  local_name, base_ofst, TY_size(sym_type), 
					  symbind, STT_OBJECT, symother,
					  Em_Get_Section_Index (em_scn[STB_scninfo_idx(base_st)].scninfo));
	  }
	  else
#endif
	  symindex = Em_Add_New_Symbol (
			ST_name(sym), base_ofst, TY_size(sym_type), 
			symbind, STT_OBJECT, symother,
			Em_Get_Section_Index (em_scn[STB_scninfo_idx(base_st)].scninfo));
	  break;
	case SCLASS_EXTERN:
	  symindex = Em_Add_New_Symbol (
		      ST_name(sym), 0, TY_size(sym_type),
		      symbind, STT_OBJECT, symother,
		      ST_is_gp_relative(sym) ? SHN_MIPS_SUNDEFINED : SHN_UNDEF);
	  if (Assembly) {
	    if (ST_is_weak_symbol(sym)) {
	      fprintf ( Asm_File, "\t%s\t", AS_WEAK);
	      EMT_Write_Qualified_Name(Asm_File, sym);
	      fprintf ( Asm_File, "\n");
	    }
	    else {
	      fprintf(Asm_File, "\t%s\t", AS_GLOBAL);
	      EMT_Write_Qualified_Name(Asm_File, sym);
	      fprintf ( Asm_File, "\n");
#ifdef TARG_ST
	      EMT_Visibility ( Asm_File, ST_export(sym), sym);
#endif
	    }
	  }
	  break;
	case SCLASS_COMMON:
	  if (sym != base_st && ST_sclass(base_st) == SCLASS_COMMON) {
		// use base common
		return EMT_Put_Elf_Symbol (base_st);
	  }
	  if (Assembly) {
		Print_Common (Asm_File, sym);
	  }
	  if (generate_elf_symbols) {
	    if (ST_is_split_common(sym)) {
		symbind = STB_SPLIT_COMMON;
		symother = STO_SC_ALIGN_UNUSED;
	  	symindex = Em_Add_New_Symbol (
			ST_name(sym), Get_Offset_From_Full(sym), TY_size(sym_type),
			symbind, STT_OBJECT, symother,
			EMT_Put_Elf_Symbol (ST_full(sym)) );
	    }
	    else {
		Elf64_Half symshndx;	/* sym section index */
		if (ST_is_thread_private(sym)) symshndx = SHN_MIPS_LCOMMON;
		else if (ST_is_gp_relative(sym)) symshndx = SHN_MIPS_SCOMMON;
		else symshndx = SHN_COMMON;
	  	symindex = Em_Add_New_Symbol (
			ST_name(sym), TY_align(sym_type), TY_size(sym_type),
			symbind, STT_OBJECT, symother, symshndx);
	    }
	  }
	  break;
	case SCLASS_UNKNOWN:
	default:
	  break;
      }
      break;

    case CLASS_NAME:
      if (ST_emit_symbol(sym)) {
	/* emit it even though it's an unknown local (C++) */
	symindex = Em_Add_New_Symbol (
		      ST_name(sym), 0, 0,
		      STB_LOCAL, STT_NOTYPE, symother, SHN_UNDEF);

      }
      break;

    case CLASS_FUNC:
      if (sclass == SCLASS_EXTERN) {
        symindex = Em_Add_New_Undef_Symbol (
			ST_name(sym), symbind, STT_FUNC, symother);
	if (Assembly) {
	  if (ST_is_weak_symbol(sym)) {
	    fprintf ( Asm_File, "\t%s\t", AS_WEAK);
	    EMT_Write_Qualified_Name(Asm_File, sym);
	    fprintf ( Asm_File, "\n");
	  }
	  else {
	    fprintf(Asm_File, "\t%s\t", AS_GLOBAL);
	    EMT_Write_Qualified_Name(Asm_File, sym);
	    fprintf ( Asm_File, "\n");
#ifdef TARG_ST
	    EMT_Visibility (Asm_File, ST_export(sym), sym);
#endif
	  }
#ifdef TARG_ST
	  // [CL] be consistent with the case when we
	  // don't generate elf symbols above
	  fprintf (Asm_File, "\t%s\t", AS_TYPE);
	  EMT_Write_Qualified_Name (Asm_File, sym);
	  fprintf (Asm_File, ", %s\n", AS_TYPE_FUNC);
#endif
	}
      }
      else 
	symindex = Em_Add_New_Symbol (
			ST_name(sym), base_ofst, 0,
			symbind, STT_FUNC, symother,
			Em_Get_Section_Index (em_scn[STB_scninfo_idx(base_st)].scninfo));
      break;

    case CLASS_BLOCK:
      if (STB_section(sym)) {
	Init_Section(sym);
	return ST_elf_index(sym);
      }
      // may be global binding (IPA global extern symbols)
      symindex = Em_Add_New_Undef_Symbol (
      				ST_name(sym), symbind, STT_OBJECT, symother);
      break;
    case CLASS_UNK:
    case CLASS_CONST:
    default:
      symindex = Em_Add_New_Undef_Symbol (
      				ST_name(sym), STB_LOCAL, STT_OBJECT, symother);
      break;
  }
  Set_ST_elf_index(sym, symindex);

  return symindex;
}

/* ====================================================================
 *    Trace_Init_Loc
 * ====================================================================
 */
static void
Trace_Init_Loc ( INT scn_idx, Elf64_Xword scn_ofst, INT32 repeat)
{
  /* Emit the section/offset/repeat as a line prefix -- the caller will
   * add context-specific information:
   */
  fprintf(TFile, "<init>: Section %s (offset %4lld x%d): ",
	    ST_name(em_scn[scn_idx].sym), scn_ofst, repeat );

  return;
}

/* ====================================================================
 *    Write_TCON
 *
 *    Emit a TCON value to the assembly/object file.
 * ====================================================================
 */
static Elf64_Word
Write_TCON (
  TCON	*tcon,		/* Constant to emit */
  INT scn_idx,		/* Section to emit it into */
  Elf64_Xword scn_ofst,	/* Section offset to emit it at */
  INT32	repeat		/* Repeat count */
)
{
  BOOL add_null = TCON_add_null(*tcon);
  // pSCNINFO scn = em_scn[scn_idx].scninfo;

  if (Trace_Init) {
    #pragma mips_frequency_hint NEVER
    Trace_Init_Loc ( scn_idx, scn_ofst, repeat);
    fprintf (TFile, "TCON: >>%s<<\n", Targ_Print (NULL, *tcon));
  }

  if (Assembly || Lai_Code) {
    INT32 scn_ofst32 = (INT32)scn_ofst;
    FmtAssert(scn_ofst32 == scn_ofst, 
       ("section offset exceeds 32 bits: 0x%llx", (INT64)scn_ofst));
    Targ_Emit_Const (Output_File, *tcon, add_null, repeat, scn_ofst32);
  } 
#if 0
  if (Object_Code) {
    Em_Targ_Emit_Const ( scn, *tcon, add_null, repeat );
  }
#endif

  if ( TCON_ty(*tcon) == MTYPE_STRING )
    scn_ofst += (Targ_String_Length (*tcon) + (add_null ? 1 : 0)) * repeat;
  else
    scn_ofst += TY_size(Be_Type_Tbl(TCON_ty(*tcon))) * repeat;

  return scn_ofst;
}

/* ====================================================================
 *    Write_Symbol
 *
 *    Emit a symbol value to the assembly/object file.
 * ====================================================================
 */
static Elf64_Word
Write_Symbol (
  ST * sym,		/* Emit the address of this symbol */
  Elf64_Sxword sym_ofst,/*   ... plus this offset */
  INT scn_idx,		/* Section to emit it in */
  Elf64_Word scn_ofst,	/* Section offset to emit it at */
  INT32	repeat		/* Repeat count */
#ifdef TARG_ST
  /* (cbr) support for half address relocation */
  ,  BOOL halfword = 0  /* Output size */
#endif
)
{
  INT32 i;
  ST *basesym;
  INT64 base_ofst = 0;
  // pSCNINFO scn = em_scn[scn_idx].scninfo;
#ifdef TARG_ST
  /* (cbr) support for half address relocation */
  INT address_size = halfword ? 2 : ((Use_32_Bit_Pointers) ? 4 : 8);
#else
  INT address_size = ((Use_32_Bit_Pointers) ? 4 : 8);
#endif

  if ( Trace_Init ) {
    #pragma mips_frequency_hint NEVER
    Trace_Init_Loc (scn_idx, scn_ofst, repeat);
    fprintf ( TFile, "SYM " );
    fprintf ( TFile, "%s %+lld\n", ST_name(sym), sym_ofst );
  }

  /* make sure is allocated */
  if (!Has_Base_Block(sym)) {
    Allocate_Object(sym);
    /* if did allocate on local stack, that messes up frame length */
    Is_True(!Has_Base_Block(sym) || Is_Global_Symbol(Base_Symbol(sym)),
		("Write_Symbol:  too late to allocate object on stack"));
  }

  /* If the symbol is a local label that has not been assigned an 
   * address, it is a label in a basic block that has been deleted. 
   * Emit zeros for this case instead of the address.
   */
  if (ST_sclass(sym) == SCLASS_TEXT && !Has_Base_Block(sym)) {
    INT32 padding;
    padding = repeat * address_size;
    if (Assembly && padding > 0) {
      ASM_DIR_ZERO(Asm_File, padding);
    }
#if 0
    if (Object_Code) {
      Em_Add_Zeros_To_Scn (scn, padding, 1);
    }
#endif
    scn_ofst += padding;
    return scn_ofst;
  }

  /* 
   * For local static symbols that do not have their own elf entry,
   * use the base symbol; funcs always have own elf entry. 
   */
  basesym = sym;
  if (Has_Base_Block(sym) && 
      ST_is_export_local(sym) && 
      ST_class(sym) != CLASS_FUNC) {
    Base_Symbol_And_Offset (sym, &basesym, &base_ofst);
  }
  if (Use_Separate_PU_Section (current_pu, basesym)) {
	/* use PU text section rather than generic one */
	basesym = PU_base;
  }
  base_ofst += sym_ofst;

#if 0
  if (Object_Code && repeat != 0) {
    if (Use_32_Bit_Pointers) {
      Em_Add_New_Content (CK_SADDR_32, scn_ofst, 4*repeat, 0, scn);
    }
    else {
      Em_Add_New_Content (CK_SADDR_64, scn_ofst, 8*repeat, 0, scn);
    }
  }
#endif

  for ( i = 0; i < repeat; i++ ) {
    // do object code first so base section initialized
#if 0
    if (Object_Code) {
	if (ST_sclass(sym) == SCLASS_EH_REGION_SUPP) {
      		Em_Add_Displacement_To_Scn (scn, EMT_Put_Elf_Symbol (basesym),
			base_ofst, 1);
	} else {
      		Em_Add_Address_To_Scn (scn, EMT_Put_Elf_Symbol (basesym), 
			base_ofst, 1);
	}
    }
#endif
    if (Assembly) {
#ifdef TARG_ST
      /* (cbr) support for half address relocation */
      if (halfword)
        fprintf (Output_File, "\t%s\t",  AS_HALF);
      else
#endif
      fprintf (Output_File, "\t%s\t", 
	       (scn_ofst % address_size) == 0 ? 
	       AS_ADDRESS : AS_ADDRESS_UNALIGNED);
      if (ST_class(sym) == CLASS_CONST) {
	EMT_Write_Qualified_Name (Output_File, basesym);
	fprintf (Output_File, " %+lld\n", base_ofst);
      }
#ifdef TARG_ST
      //
      // Arthur: the way a function pointer is emitted depends on
      //         whether or not we're generating GP-relative, GOT,
      //         etc. Does not it ??
      // clarkes: Yes, so encapsulate that in the AS_FPTR macro.
      //
      else if (ST_class(sym) == CLASS_FUNC) {
	if (AS_FPTR) {
	  fprintf (Output_File, " %s(", AS_FPTR);
	}
	EMT_Write_Qualified_Name (Output_File, sym);
	fprintf (Output_File, "%+lld", sym_ofst);
	if (AS_FPTR) {
	  fputc (')', Output_File);
	}
	fputc ('\n', Output_File);
      }
#else
      else if (ST_class(sym) == CLASS_FUNC && AS_FPTR) {
	fprintf (Output_File, " %s(", AS_FPTR);
	EMT_Write_Qualified_Name (Output_File, sym);
	fprintf (Output_File, " %+lld)\n", sym_ofst);
      }
#endif
      else {
	EMT_Write_Qualified_Name (Output_File, sym);
	fprintf (Output_File, " %+lld\n", sym_ofst);
      }

      if (ST_class(sym) == CLASS_FUNC) {
	fprintf (Output_File, "\t%s\t", AS_TYPE);
	EMT_Write_Qualified_Name (Output_File, sym);
	fprintf (Output_File, ", %s\n", AS_TYPE_FUNC);
      }
    }

    scn_ofst += address_size;
  }
  return scn_ofst;
}

/* ====================================================================
 *   Write_Label
 *
 *   Emit a label value to the assembly/object file.
 * ====================================================================
 */

static Elf64_Word
Write_Label (
  LABEL_IDX lab,	/* Emit the address of this label */
  Elf64_Sxword lab_ofst,/*   ... plus this offset */
  INT scn_idx,		/* Section to emit it in */
  Elf64_Word scn_ofst,	/* Section offset to emit it at */
  INT32	repeat		/* Repeat count */
)
{
  INT32 i;
  INT address_size = ((Use_32_Bit_Pointers) ? 4 : 8);

  for (i = 0; i < repeat; i++) {
    fprintf (Output_File, "\t%s\t", (scn_ofst % address_size) == 0 ? 
		                  AS_ADDRESS : AS_ADDRESS_UNALIGNED);
    fprintf (Output_File, "%s", LABEL_name(lab));
    if (lab_ofst != 0)
      fprintf (Output_File, " %+lld", lab_ofst);
    fprintf (Output_File, "\n");
    scn_ofst += address_size;
  }
  return scn_ofst;
}

/* ====================================================================
 *    Write_Symdiff (lab1, sym2idx, scn_idx, scn_ofst, repeat, size)
 * ====================================================================
 */
static Elf64_Word
Write_Symdiff (
  LABEL_IDX lab1,	/* left symbol */
  ST_IDX sym2idx,	/* right symbol */
  INT scn_idx,		/* Section to emit it in */
  Elf64_Word scn_ofst,	/* Section offset to emit it at */
  INT32	repeat,		/* Repeat count */
  INT size		/* 2 or 4 bytes */
)
{
  INT32 i;
  ST *basesym1;
  ST *basesym2;
  INT64 base1_ofst = 0;
  INT64 base2_ofst = 0;
  pSCNINFO scn = em_scn[scn_idx].scninfo;
  ST *sym2 = &St_Table[sym2idx];
  INT32 val;

  if ( Trace_Init ) {
    #pragma mips_frequency_hint NEVER
    Trace_Init_Loc (scn_idx, scn_ofst, repeat);
    fprintf ( TFile, "SYMDIFF " );
    fprintf ( TFile, "%s - %s\n", LABEL_name(lab1), ST_name(sym2));
  }

  /* symbols must have an address */
  Is_True (lab1, ("cgemit: Symdiff lab1 is null"));
  Is_True (sym2, ("cgemit: Symdiff sym2 is null"));
  Is_True (Has_Base_Block(sym2), ("cgemit: Symdiff sym2 not allocated"));

  basesym1 = BB_cold(Get_Label_BB(lab1)) ? cold_base : text_base;
  base1_ofst = Get_Label_Offset(lab1);
  Base_Symbol_And_Offset (sym2, &basesym2, &base2_ofst);
  if (Use_Separate_PU_Section(current_pu,basesym2)) {
	/* use PU text section rather than generic one */
	basesym2 = PU_base;
  }
  Is_True (basesym1 == basesym2, ("cgemit: Symdiff bases not same"));
  val = base1_ofst - base2_ofst;
  if (val < 0) val = 0;
  if (size == 2) {
	if (val > INT16_MAX) {
		DevWarn("symdiff value not 16-bits; will try recompiling with -TENV:long_eh_offsets");
		Early_Terminate (RC_OVERFLOW_ERROR);
	}
	val = val << 16;	/* for Add_Bytes */
  }

  for ( i = 0; i < repeat; i++ ) {
    if (Assembly) {
      fprintf (Output_File, "\t%s\t", (size == 2 ? AS_HALF : AS_WORD));
      fprintf(Output_File, "%s", LABEL_name(lab1));
      fprintf (Output_File, "-");
      EMT_Write_Qualified_Name (Output_File, sym2);
      fprintf (Output_File, "\n");
    }
    if (Object_Code) {
      Em_Add_Bytes_To_Scn (scn, (char *) &val, size, 1);
    }
    scn_ofst += size;
  }
  return scn_ofst;
}

/* ====================================================================
 *    Write_INITV
 *
 *    Emit an initial value record to the assembly/object file.
 * ====================================================================
 */
static Elf64_Word
Write_INITV (
  INITV_IDX invidx, 
  INT scn_idx, 
  Elf64_Word scn_ofst
)
{
  INT32 i;
  INITV_IDX ninv;
  INITV inv = Initv_Table[invidx];
  LABEL_IDX lab;
  TCON tcon;
  ST *st;
  // pSCNINFO scn = em_scn[scn_idx].scninfo;

  switch ( INITV_kind(inv) ) {
    case INITVKIND_ZERO:
      tcon = Host_To_Targ (INITV_mtype (inv), 0);
      scn_ofst = Write_TCON (&tcon, scn_idx, scn_ofst, INITV_repeat2 (inv));
      break;

    case INITVKIND_ONE:
      tcon = Host_To_Targ (INITV_mtype (inv), 1);
      scn_ofst = Write_TCON (&tcon, scn_idx, scn_ofst, INITV_repeat2 (inv));
      break;
    case INITVKIND_VAL:
      scn_ofst = Write_TCON (&INITV_tc_val(inv), scn_idx, scn_ofst, 
			      INITV_repeat2(inv));
      break;

    case INITVKIND_SYMOFF:
      st = &St_Table[INITV_st(inv)];
      switch (ST_sclass(st)) {
	case SCLASS_AUTO:
	{ /* EH stack variable */
	  tcon = Host_To_Targ(MTYPE_I4, 0);
	  scn_ofst = Write_TCON (&tcon, scn_idx, scn_ofst, INITV_repeat1(inv));
	  break;
	}

	case SCLASS_FORMAL:
	{ /* EH this-pointer */
	  // ST * base = ST_base(st);
	  INT  ofst = ST_ofst(st);
	  tcon = Host_To_Targ(MTYPE_I4, ofst);
	  scn_ofst = Write_TCON (&tcon, scn_idx, scn_ofst, INITV_repeat1(inv));
	  break;
	}
      
	default:
          scn_ofst = Write_Symbol ( st, INITV_ofst(inv),
	  			      scn_idx, scn_ofst, INITV_repeat1(inv));
	   break;
      }
      break;

#ifdef TARG_ST
      /* (cbr) support for half address relocation */
    case INITVKIND_SYMOFF16:
      st = &St_Table[INITV_st(inv)];
      switch (ST_sclass(st)) {
	case SCLASS_AUTO:
	case SCLASS_FORMAL:
          DevAssert(FALSE, ("wrong class symoff16"));
	  break;
          
	default:
          scn_ofst = Write_Symbol ( st, INITV_ofst(inv),
                                    scn_idx, scn_ofst, INITV_repeat1(inv), true);
	   break;
      }
      break;
#endif

    case INITVKIND_LABEL:
	lab = INITV_lab(inv);
	scn_ofst = Write_Label (lab, 0, scn_idx, scn_ofst, INITV_repeat1(inv));
	break;
    case INITVKIND_SYMDIFF:
      scn_ofst = Write_Symdiff ( INITV_lab1(inv), INITV_st2(inv),
				scn_idx, scn_ofst, INITV_repeat1(inv), 4);
      break;
    case INITVKIND_SYMDIFF16:
      scn_ofst = Write_Symdiff ( INITV_lab1(inv), INITV_st2(inv),
				scn_idx, scn_ofst, INITV_repeat1(inv), 2);
      break;

    case INITVKIND_BLOCK:
      for (i = 0; i < INITV_repeat1(inv); i++) {
	for (ninv = INITV_blk(inv); ninv; ninv = INITV_next(ninv)) {
          scn_ofst = Write_INITV (ninv, scn_idx, scn_ofst);
	}
      }
      break;

    case INITVKIND_PAD:
      if ((Assembly || Lai_Code) && (INITV_pad(inv)*INITV_repeat1(inv) > 0)) {
	fprintf(Output_File, "\t%s %d\n", 
                           AS_SPACE, INITV_pad(inv) * INITV_repeat1(inv));
	//ASM_DIR_ZERO(Output_File, INITV_pad(inv) * INITV_repeat1(inv));
      }
#if 0
      if (Object_Code) {
	Em_Add_Zeros_To_Scn (scn, INITV_pad(inv) * INITV_repeat1(inv), 1);
      }
#endif
      scn_ofst += INITV_pad(inv) * INITV_repeat1(inv);
      break;

    default:
      break;
  }
  return scn_ofst;
}

/* ====================================================================
 *    Write_INITO (inop, scn_idx, scn_ofst)
 *
 *    Emit the initialized object to the object file 
 * ====================================================================
 */
static Elf64_Word
Write_INITO (
  INITO* inop,		/* Constant to emit */
  INT scn_idx,		/* Section to emit it into */
  Elf64_Xword scn_ofst	/* Section offset to emit it at */
)
{
  // pSCNINFO scn = em_scn[scn_idx].scninfo;
  Elf64_Xword inito_ofst;
  ST *sym;
  ST *base;
  INITO ino = *inop;

  if ( Trace_Init ) {
    #pragma mips_frequency_hint NEVER
    Trace_Init_Loc (scn_idx, scn_ofst, 0);
    fprintf ( TFile, "INITO: " );
    Print_INITO (ino);
  }

  Base_Symbol_And_Offset(INITO_st(ino), &base, (INT64*)&inito_ofst);

  if (inito_ofst > scn_ofst) {
    if (Assembly || Lai_Code) {
      fprintf(Output_File, "\t%s %lld\n", AS_SPACE, inito_ofst - scn_ofst);
      //ASM_DIR_ZERO(Output_File, inito_ofst - scn_ofst);
    }
#if 0
    if (Object_Code) {
      Em_Add_Zeros_To_Scn ( scn, inito_ofst - scn_ofst, 1 );
    }
#endif
    scn_ofst = inito_ofst;
  } else {
    FmtAssert ( inito_ofst >= scn_ofst, 
      ("Write_INITO: DATA overlap 1, inito ofst @ %lld, scn ofst @ %lld",
	  inito_ofst, scn_ofst));
  }

  sym = INITO_st(ino);
  if (Assembly || Lai_Code) {
    char *name = ST_name(sym);
    if (name != NULL && *name != 0) {
      Print_Label (Output_File, sym, TY_size(ST_type(sym)));
    }
  }
#if 0
  if (Object_Code && ! ST_is_export_local(sym)) {
    EMT_Put_Elf_Symbol (sym);
  }
#endif

  /* If there's no initial value, this should be a constant symbol,
   * and the represented constant is the initial value:
   */
  if ( INITO_val(ino) == (INITO_IDX) NULL ) {
    if ( ST_class(sym) == CLASS_CONST ) {
      scn_ofst = Write_TCON (&ST_tcon_val(sym), scn_idx, scn_ofst, 1);
    }
  } else {
    INITV_IDX inv;
    FOREACH_INITV (INITO_val(ino), inv) {
      scn_ofst = Write_INITV (inv, scn_idx, scn_ofst);
    }
  }

  if (Assembly || Lai_Code) {
#ifdef TARG_ST
    if (List_Notes)
#endif
    fprintf (Output_File, "\t%s end of initialization for %s\n", 
                                              ASM_CMNT, ST_name(sym));
  }

  return scn_ofst;
}

#ifdef TARG_ST
/* ====================================================================
 * Set_Section_Offset.
 *
 * [CG] Set the new offset for a section.
 * ====================================================================
 */
static void
Set_Section_Offset(ST *base, INT64 ofst)
{
  em_scn[STB_scninfo_idx(base)].scn_ofst = ofst;
}

/* ====================================================================
 * Add_To_Section_Offset.
 *
 * [CG] Inform that an object has been emitted into the section.
 * Thus the current section offset must be updated.
 * For each data emitted do:
 * Change_Section_Origin(st_sect, ofst);
 * Check_Section_Alignement(st_sect, object_align)
 * ... emit object ...
 * Add_To_Section_Offset(st_sect, object_size);
 * ====================================================================
 */
static void
Add_To_Section_Offset (ST *base, INT64 size) 
{
  em_scn[STB_scninfo_idx(base)].scn_ofst += size;
}		       

static void
Check_Section_Alignment (ST *base, int align)
{
  INT64 current = em_scn[STB_scninfo_idx(base)].scn_ofst;
  INT64 aligned = align == 0 ? current : 
    ((current + (align - 1)) / align) * align;
  FmtAssert(aligned == current,
	    ("object alignment mismatch offset in section %s: offset is %lld, alignment is %d", ST_name(base), current, align));
}
#endif

/* ====================================================================
 *    Change_Section_Origin (base, ofst)
 *
 *    change to a new section and new origin 
 * ====================================================================
 */
static void
Change_Section_Origin (
  ST *base, 
  INT64 ofst
)
{
  if (Assembly || Lai_Code) {
    if (base != cur_section) {
      /* switch to new section. */
      fprintf (Output_File, "\n\t%s %s\n", AS_SECTION, ST_name(base));
    }
    
#ifdef TARG_ST
    // IA64 generates .org so next data is placed at this offset
    // within the section
    // GHS assembler interprets .org as an absolute address
    // We must use use .align instead that follows.
#else
    // generate .org so next data is placed at this offset
    fprintf (Output_File, "\t%s 0x%llx\n", AS_ORIGIN, ofst);
#endif

    /* [CG]: We should not insert alignement, offsets have already alignment
       into account. The alignment should only appear at the start of the section. */
#if 0
    /* generate a '.align 0' to make sure we don't autoalign */
#ifdef GHS_BUG_FIXED
    // I can't for now since GHS assembler does not recognize
    // .align 0
    fprintf (Output_File, "\t%s\t0\n", AS_ALIGN);
#else
    // Arthur: we are aligning at 4 bytes for now.
    // TODO: parametrize, so aligns at the strongest required
    //       alignment on this target
    fprintf (Output_File, "\t%s 4\n", AS_ALIGN);
#endif
#endif

#ifdef TARG_ST
    // 
    // Emit this section's label before offset 0
    //
    if (ofst == 0) {
      /* [CG] We set current section offset to 0. */
      em_scn[STB_scninfo_idx(base)].scn_ofst = 0;

#ifdef TARG_ST100
  /* (cbr) section relative offset using labels only on st100 */
      fprintf(Asm_File, "%s:\n", em_scn[STB_scninfo_idx(base)].label);
      if (Trace_Init) {
	fprintf(TFile, "<init>: Emitting label %s for section %s\n",
		em_scn[STB_scninfo_idx(base)].label, ST_name(base));
      }
#endif
    }
#endif

#ifdef TARG_ST
    {
      /* [CG] We must pad to new ofst. */
      INT64 current_ofst = em_scn[STB_scninfo_idx(base)].scn_ofst;
      if (current_ofst <= ofst) {
	INT64 padding = ofst - current_ofst;
	if (padding > 0) {
	  fprintf(Output_File, "\t%s %lld\n", AS_SPACE, padding);
	}
      } else {
	FmtAssert(0,
		  ("object overlaps in section %s: expected offset is %lld, current offset is %lld", ST_name(base), ofst, current_ofst));
      }
      em_scn[STB_scninfo_idx(base)].scn_ofst = ofst;
    }
#endif

    cur_section = base;
  }

#if 0
  /* for nobits, add final size at end because we don't write any data. */
  if (Object_Code && !STB_nobits(base)) {
    Em_Change_Section_Origin (em_scn[STB_scninfo_idx(base)].scninfo, ofst);
  }
#endif

  return;
}

/* ====================================================================
 *    section_lt (s1, s2)
 * ====================================================================
 */
inline bool section_lt (ST *s1, ST* s2) 
{ 
  // order doesn't really matter, just that grouped by base section
  return Base_Symbol(s1) < Base_Symbol(s2); 
}

/* ====================================================================
 *    offset_lt (s1, s2)
 * ====================================================================
 */
inline bool offset_lt (ST *s1, ST* s2) 
{ 
  return Offset_From_Base_Symbol(s1) < Offset_From_Base_Symbol(s2); 
}

/* ====================================================================
 *    size_lt (s1, s2)
 * ====================================================================
 */
inline bool size_lt (ST *s1, ST* s2) 
{ 
  // This is only needed so that we get silly 0-sized structs
  // at the correct offset.
  return TY_size(ST_type(s1)) < TY_size(ST_type(s2)); 
}

#if 0
/* ====================================================================
 *    LAI_print_section
 * ====================================================================
 */
static void
LAI_print_section (
  ST *st, 
  Elf64_Word scn_type, 
  Elf64_Word scn_flags,
  Elf64_Xword scn_entsize, 
  ST *cur_section
)
{
  const char *scn_name = ST_name(st);
  Elf64_Word  scn_align = STB_align(st);
  const char *cur_scn_name = (cur_section != NULL) ? 
                                         ST_name(cur_section) : NULL;
  char scn_flags_string[5];
  char scn_type_string[10];  // min of strlen("progbits") + 1
  char *p = &scn_flags_string[0];

  fprintf (Lai_File, "\n\t%s \"%s\",", AS_SECTION, scn_name);

  if (scn_flags & SHF_WRITE) *p++ = 'w';
  if (scn_flags & SHF_ALLOC) *p++ = 'a';
  if (scn_flags & SHF_EXECINSTR) *p++ = 'x';

  *p = '\0'; // null terminate the string.
  fprintf (Lai_File, " \"%s\"", scn_flags_string);

  /*
  fprintf (Lai_File, " \"%s\",", scn_flags_string);

  p = &scn_type_string[0];
  if (scn_type == SHT_NOBITS) {
    strcpy(p, "nobits");
  }
  else if (scn_type == SHT_PROGBITS) {
    strcpy(p, "progbits");
  }
  else {
    DevWarn("Intel assembler definition inadequate for "
	    "ELF section type 0x%lx; using \"progbits\"", scn_type);
    strcpy(p, "progbits");
  }
  fprintf (Lai_File, " \"%s\"\n", scn_type_string);
  */

  fprintf (Lai_File, "\n");
  fprintf (Lai_File, "\t%s\t%d\n", AS_ALIGN, scn_align);  

  return;
}
#endif

/* ====================================================================
 *    Process_Initos_And_Literals (stab)
 *
 *    This routine can be called multiple times for the global symtab;
 *    we do this so that objects are emitted in order.
 *    For each section, some objects are in local symtab, and some in global.
 *    We keep track of the last global symbol that has been processed
 *    so that we only process new symbols.
 *
 * ====================================================================
 */
static void
Process_Initos_And_Literals (
  SYMTAB_IDX stab
)
{
  static std::vector<bool> st_processed;
  if (st_processed.size() != ST_Table_Size(GLOBAL_SYMTAB)) {
    st_processed.resize(ST_Table_Size(GLOBAL_SYMTAB), false);
  }

  std::vector<ST*> st_list;
  std::vector<ST*>::iterator st_iter;

  typedef 
  __GNUEXT::hash_map < ST_IDX, INITO*, __GNUEXT::hash<ST_IDX>, __GNUEXT::equal_to<ST_IDX> > ST_INITO_MAP;
  ST_INITO_MAP st_inito_map;

  UINT i;
  static UINT last_inito = 1;
  
  // First walk the INITOs from the global table
  for (i = last_inito; i < INITO_Table_Size(GLOBAL_SYMTAB); ++i) {
    INITO* ino = &Inito_Table(GLOBAL_SYMTAB,i);
    ST* st = INITO_st(ino);
    /* don't emit initialization if st not used or extern */
    if (ST_is_not_used(st) ||
        ST_sclass(st) == SCLASS_EXTERN ||
        ST_sclass(st) == SCLASS_DISTR_ARRAY) {
      continue;
    }
    st_list.push_back(st);
    st_inito_map[ST_st_idx(st)] = ino;
  }

  last_inito = INITO_Table_Size(GLOBAL_SYMTAB);

  // Then walk the INITOs from the local table
  if (stab != GLOBAL_SYMTAB) {
    for (i = 1; i < INITO_Table_Size(stab); ++i) {
      INITO* ino = &Inito_Table(stab,i);
      ST* st = INITO_st(ino);
      /* don't emit initialization if st not used or extern */
      if (ST_is_not_used(st) ||
          ST_sclass(st) == SCLASS_EXTERN) {
        continue;
      }
      st_list.push_back(st);
      st_inito_map[ST_st_idx(st)] = ino;
    }
  }

  // Then walk the CONSTANTs from the global table
  for (i = 1; i < ST_Table_Size(GLOBAL_SYMTAB); ++i) {
    ST* st = &St_Table(GLOBAL_SYMTAB,i);
    if (ST_class(st) == CLASS_CONST && !st_processed[ST_index(st)]) {
      INT64 ofst;
      ST* base;
      Base_Symbol_And_Offset(st, &base, &ofst);
      if (ST_class(base) != CLASS_BLOCK || !STB_section(base)) {
        continue; // not allocated
      }
      if (Emit_Global_Data && SEC_is_merge(STB_section_idx(base))) {
        continue; // allocate in each .o
      }
      st_list.push_back(st);
    }
  }

  // Print_ST_List(st_list, "UNSORTED");
  stable_sort (st_list.begin(), st_list.end(), size_lt);
  // Print_ST_List(st_list, "SORTED BY SIZE");
  stable_sort (st_list.begin(), st_list.end(), offset_lt);
  // Print_ST_List(st_list, "SORTED BY OFFSET");
  stable_sort (st_list.begin(), st_list.end(), section_lt);
  // Print_ST_List(st_list, "SORTED BY SECTION");

  for (st_iter = st_list.begin(); st_iter != st_list.end(); ++st_iter) {

    INT64 ofst;
    ST* base;
    ST* st = *st_iter;
    ST_INITO_MAP::iterator st_inito_entry = st_inito_map.find(ST_st_idx(st));

    if (st_inito_entry != st_inito_map.end()) {
      INITO* ino = (*st_inito_entry).second;
      Base_Symbol_And_Offset(st, &base, &ofst);
      if (ST_sclass(base) == SCLASS_EXTERN) {
        // ipa can cause it to be based on global extern,
        // in which case it is already emitted. 
        continue;
      }
      FmtAssert(ST_class(base) == CLASS_BLOCK && STB_section(base),
                ("inito (%s) not allocated?", ST_name(st)));
      Init_Section(base); //make sure base is inited 
      // may need padding between objects in same section,
      // so always change origin
      Change_Section_Origin (base, ofst);
#ifdef TARG_ST
      Check_Section_Alignment (base, TY_align(ST_type(st)));
#endif
#if 0
      // Arthur: It's a hack but I can not align things in the above
      //         (see comment in Change_Section_Origin(). I am 
      //         aligning it at 4 bytes here. I should really get
      //         be getting the alignment requirement for 'st'
      fprintf (Output_File, "\t%s 4\n", AS_ALIGN);
#endif
      ofst = Write_INITO (ino, STB_scninfo_idx(base), ofst);
#ifdef TARG_ST
      Set_Section_Offset(base, ofst);
#endif
    }

    else {
      st_processed[ST_index(st)] = TRUE;
      Base_Symbol_And_Offset(st, &base, &ofst);
      Init_Section(base); // make sure base is inited
      // we cannot assume that constants are sequentially ordered
      // by offset, because they are allocated on the fly as we
      // expand the whirl nodes.  So always reset the origin.
      Change_Section_Origin (base, ofst);
#ifdef TARG_ST
      Check_Section_Alignment (base, TY_align(ST_type(st)));
#endif
#if 0
      // Arthur: It's a hack but I can not align things in the above
      //         (see comment in Change_Section_Origin(). I am 
      //         aligning it at 4 bytes here. I should really get
      //         be getting the alignment requirement for 'st'
      // Is this necessary here ?
      fprintf (Output_File, "\t%s 4\n", AS_ALIGN);
#endif

      // emit TCON associated symbolic name as label
      //      char *cname = Get_TCON_name (ST_tcon(st));
      //      fprintf(Output_File, "%s:\n", cname);
#ifdef TARG_ST
      // [CL] handle the case of generation of symtab.s
      // in IPA mode. Although local, such symbols must
      // be 'promoted' to global so that they can be
      // accessed by the module they belong to
      if (Emit_Global_Data) {
	fprintf(Output_File, "\t%s\t", AS_GLOBAL);
	EMT_Write_Qualified_Tcon_Name(Output_File, st);
	fprintf(Output_File, "\n");
	// [CL] output as hidden/internal so that we are close to the
	// export_local meaning
#ifdef AS_INTERNAL
	if (ST_export(st) == EXPORT_LOCAL_INTERNAL) {
	  fprintf (Output_File, "\t%s\t", AS_INTERNAL);
	  EMT_Write_Qualified_Tcon_Name(Output_File, st);
	  fprintf(Output_File, "\n");
	} else
#endif
#ifdef AS_HIDDEN
	  {
	    fprintf (Output_File, "\t%s\t", AS_HIDDEN);
	    EMT_Write_Qualified_Tcon_Name(Output_File, st);
	    fprintf(Output_File, "\n");
	  }
#endif
      }
      EMT_Write_Qualified_Tcon_Name(Output_File, st);
      fprintf(Output_File, ":\n");
#else
      fprintf(Output_File, "%s_UNNAMED_CONST_%d:\n", Local_Label_Prefix, ST_tcon(st));
#endif
      ofst = Write_TCON (&ST_tcon_val(st), STB_scninfo_idx(base), ofst, 1);
#ifdef TARG_ST
      Set_Section_Offset(base, ofst);
#endif
    }
  }

  return;
}

/* ====================================================================
 *    Process_Distr_Array ()
 * 
 *    Write all distr_array INITOs from the global symtab
 * ====================================================================
 */
static void
Process_Distr_Array ()
{
  for (UINT i = 1; i < INITO_Table_Size(GLOBAL_SYMTAB); ++i) {
    INITO* ino = &Inito_Table(GLOBAL_SYMTAB,i);
    ST* st = INITO_st(ino);
    if (!ST_is_not_used(st) &&
        ST_sclass(st) == SCLASS_DISTR_ARRAY) {
      INT64 ofst;
      ST* base;
      Base_Symbol_And_Offset(st, &base, &ofst);
      FmtAssert(ST_class(base) == CLASS_BLOCK && STB_section(base),
                ("inito (%s) not allocated?", ST_name(st)));
      Init_Section(base);
      Change_Section_Origin(base, ofst);
#ifdef TARG_ST
      Check_Section_Alignment (base, TY_align(ST_type(st)));
#endif
      ofst = Write_INITO(ino, STB_scninfo_idx(base), ofst);
#ifdef TARG_ST
      Set_Section_Offset(base, ofst);
#endif

    }
  }

  return;
}

/* ====================================================================
 *    Process_Bss_Data (stab)
 *
 *    iterate through the global symbols and write (s)bss symbols 
 *    to sections 
 * ====================================================================
 */
static void
Process_Bss_Data (
  SYMTAB_IDX stab
)
{
  /* 
   * To guarantee in increasing order, create std::vector of st*, 
   * then stable_sort on section, then stable_sort on offset. 
   */
  std::vector< ST* > bss_list;
  std::vector< ST* >::iterator bssp;

  // This routine can be called multiple times for the global symtab;
  // we do this so that objects are emitted in order.
  // For each section, some objects are in local symtab, and some in global.
  // We keep track of the last global symbol that has been processed
  // so that we only process new symbols.

  static UINT last_global_index = 1;
  UINT first_index = (stab == GLOBAL_SYMTAB ? last_global_index : 1);
  
  for (UINT i = first_index; i < ST_Table_Size(stab); ++i) {
    ST* sym = &St_Table(stab,i);
    if (ST_class(sym) == CLASS_BLOCK)
      continue;	// not a leaf symbol
    if (!Has_Base_Block(sym))
      continue;	// not a data symbol
#ifdef TARG_ST
    // [CG] If it is an alias, we don't emit it.
    // The alias generation is done elsewhere
    if (Has_Base_Block(sym) && ST_class(ST_base(sym)) != CLASS_BLOCK
	&& ST_emit_symbol(sym))
      continue;
#endif
    if (ST_sclass(sym) == SCLASS_UGLOBAL ||
        ST_sclass(sym) == SCLASS_FSTATIC ||
        ST_sclass(sym) == SCLASS_PSTATIC) {
      bss_list.push_back (sym);	// bss symbol
    }
  }

  if (stab == GLOBAL_SYMTAB) {
    last_global_index = ST_Table_Size(GLOBAL_SYMTAB);
  }

  // It's a bit counter-intuitive, but to get the list sorted
  // by section and then by offset within section,
  // should stable_sort in reverse order (offset then section).
  stable_sort (bss_list.begin(), bss_list.end(), size_lt);
  stable_sort (bss_list.begin(), bss_list.end(), offset_lt);
  stable_sort (bss_list.begin(), bss_list.end(), section_lt);

  ST*   sym;
  ST*   next_sym;
  ST*   base;
  ST*   next_base;
  INT64 ofst;
  INT64 next_ofst;
  INT64 size;

#ifdef TARG_ST
  //
  // can have overlapping symbols that are equivalenced.
  // assume here that if multiple symbols with same offset,
  // are sorted so that largest size is last.
  //
  // align must come before the label. But if multiple labels
  // with the same offset overlap, need to align at the
  // strongest alignment. Thus, Arthur's using a list to
  // accumulate the labels while they refer to the equivalenced
  // symbols and looking for the strongest alignment.
  //
  std::list<ST*> equivalenced_symbols;
  INT32 align = 0;
#endif

  for (bssp = bss_list.begin(); bssp != bss_list.end(); ++bssp) {

    sym = *bssp;
    Base_Symbol_And_Offset(sym, &base, &ofst);
    if (ST_class(base) != CLASS_BLOCK || !STB_section(base))
      continue;	/* not allocated */
    if (!STB_nobits(base))
      continue;	/* not a bss symbol */

#ifdef TARG_ST200
    if (ST_assigned_to_dedicated_preg(sym))
      continue;
#endif

#ifdef TARG_ST
    // [CG]: Ensure section is initialized
    Init_Section(base); 
#endif

    Change_Section_Origin (base, ofst);
    if (Assembly || Lai_Code) {
      size = TY_size(ST_type(sym));
#ifdef TARG_ST
      // Save alignment before continuing
      if (TY_align(ST_type(sym)) > align) {
	align = TY_align(ST_type(sym));
      }
      // Do not print the label yet
#else
      Print_Label (Output_File, sym, size);
#endif

      // before emitting space,
      // first check whether next symbol has same offset.
      // can have overlapping symbols that are equivalenced.
      if (bssp+1 != bss_list.end()) {
	next_sym = *(bssp+1);
	Base_Symbol_And_Offset(next_sym, &next_base, &next_ofst);
	if (next_base == base && next_ofst == ofst) {
	  // skip to next iteration
	  // so label printed before space emitted.
#ifdef TARG_ST
	  // Save label in the equvalenced symbol list before
	  // continueing
	  equivalenced_symbols.push_back(sym);
#endif
	  continue;
	}
	else if (next_base == base && next_ofst < (ofst+size)) {
	  // have label in middle of structure,
	  // so don't emit space for full structure.
	  size = next_ofst - ofst;
	}
      }

#ifdef TARG_ST
      // assume here that if multiple symbols with same offset,
      // are sorted so that largest size is last.
      if (size > 0) {
#ifdef TARG_ST
	// [CG]: Don't emit alignement, the padding was already inserted
	// by the Change_Section_Origin()
#else
	// Time to print out the alignment and the labels
	fprintf(Output_File, "\t%s %d\n", AS_ALIGN, align);
#endif
	std::list<ST*>::iterator esyms;
	// Print the previously eqivalenced symbols
	for (esyms = equivalenced_symbols.begin();
	     esyms != equivalenced_symbols.end();
	     esyms++) {
	  Print_Label (Output_File, *esyms, TY_size(ST_type(*esyms)));
	}
	// Print the last/current one
	Print_Label (Output_File, sym, size);

	Check_Section_Alignment (base, align);

	// clear the list
	equivalenced_symbols.clear();
	// reset align
	align = 0;

#else
      // assume here that if multiple symbols with same offset,
      // are sorted so that largest size is last.
      if (size > 0) {
	fprintf(Output_File, "\t%s %d\n", AS_ALIGN, TY_align(ST_type(sym)));
#endif
	if (Assembly)
	  ASM_DIR_SKIP(Asm_File, (INT32)size);
	if (Lai_Code)
	  fprintf(Lai_File, "\t%s %lld\n", AS_SPACE, size);
	  /*      ASM_DIR_SKIP(Lai_File, size); */
      }
#ifdef TARG_ST
      Add_To_Section_Offset(base, size);
#endif
    }

    if (generate_elf_symbols && !ST_is_export_local(sym)) {
      (void)EMT_Put_Elf_Symbol (sym);
    }
  }
}

/* ====================================================================
 *   Branch_Skips_First_Op
 *
 *   Check if <pred> is a branch to <succ> that skips the first instruction
 *   in the <succ>. This typically happens after filling the branch delay
 *   slot with the first instruction in the target basic block.
 * ====================================================================
 */
static BOOL
Branch_Skips_First_Op (
  BB *pred, 
  BB *succ
) 
{
  OP *op;

  if (BB_next(pred) == succ) return FALSE;

  op = BB_branch_op (pred);
  if (op != NULL) {
    INT i;
    for (i = 0; i < OP_opnds(op); i++) {
      TN *opnd_tn = OP_opnd(op,i);
      if (TN_is_label(opnd_tn) && TN_offset(opnd_tn) != 0) return TRUE;
    }
  }
  return FALSE;
}

#ifdef TARG_ST
/* ====================================================================
 *   Check_If_Should_Align_BB
 *
 *   Check if bb should be aligned, 
 *   and return number of bytes the bb should be aligned with.
 *   Or 0 if no alignment is requested.
 * ====================================================================
 */
// [CG] This function interface has changed and returns the alignment
// in bytes. 
// The options that may define a bb alignment are in priority
// order:
// -OPT:align_loops
// -OPT:align_jumps
// -OPT:align_labels
// -OPT:align_instructions
//
INT32
Check_If_Should_Align_BB (
  BB *bb
)
{
  BOOL bb_is_loop = FALSE;
  BOOL bb_has_jump_to = FALSE;

  if (OPT_Space) return 0;

  /* Detect bbs that are loop head. */
  if (BB_loop_head_bb(bb) == bb &&
      !BB_unrolled_fully(bb)) bb_is_loop = TRUE;
  if (bb_is_loop && Align_Loops) return Align_Loops;

  /* Treat targets of jumps.*/
  if ((BB_preds_len(bb) >= 2) ||
      (BB_preds_len(bb) == 1 &&
       BB_prev(bb) != BB_First_Pred(bb))) bb_has_jump_to = TRUE;
  if (bb_has_jump_to && Align_Jumps) return Align_Jumps;

  /* If Align_Labels is specified all BB are aligned. */
  if (Align_Labels) return Align_Labels;
  
  /* Is Align instructions is specified, check BB that should be aligned. */
  if (Align_Instructions) {
    /* No predecessor, don't align */
    if (BB_prev(bb) == NULL)
      return 0;	
    /* Not frequent enough, so don't align */
    if (BB_freq(bb) <= 1.0)
      return 0;	
#define FREQUENT_BB_DIFF 5.0
    /* Not frequent enough compared to predecessor. */
    if (BB_freq(bb) / BB_freq(BB_prev(bb)) < FREQUENT_BB_DIFF)
      return 0;
    return Align_Instructions;
  }
  return 0;
}
#else

/* ====================================================================
 *   Check_If_Should_Align_BB
 *
 *   Check if bb should be aligned, 
 *   and return number of instructions it should be aligned with.
 * ====================================================================
 */
INT32
Check_If_Should_Align_BB (
  BB *bb, 
  INT32 curpc
)
{
  BBLIST *preds;
  INT32 targpc;
  float skip_freq = 0.01;		/* not zero so can divide */
  float noskip_freq = 0.01;
  INT targ_alignment;
  INT num_of_ops = 0; 		/* zero to begin with */
#define FREQUENT_BB_DIFF 5.0

  /*
   * Align loops for best processor efficiency.
   * Originally checked if bb_loophead, but now just
   * check frequency of block (in case doesn't look like proper loop,
   * but still has high-frequency).
   */
  if (OPT_Space)
    return 0;	/* don't align */
  if (BB_freq(bb) <= 1.0)
    return 0;	/* not frequent enough, so don't align */
  if (BB_prev(bb) == NULL) 
    return 0;	/* nowhere to put nops */
  /* don't want to add nops to previous bb 
   * unless current bb is significantly more frequent. 
   */
  if (BB_freq(bb) / BB_freq(BB_prev(bb)) < FREQUENT_BB_DIFF)
    return 0;

  /* first check whether target is always label+4 */
  FOR_ALL_BB_PREDS (bb, preds) {
    if (Branch_Skips_First_Op (BBLIST_item(preds), bb)) {
      skip_freq += BB_freq(BBLIST_item(preds));
    } else {
      noskip_freq += BB_freq(BBLIST_item(preds));
    }
  }
  if ((skip_freq / noskip_freq) > FREQUENT_BB_DIFF)
    targpc = PC_Incr(curpc);
  else if ((noskip_freq / skip_freq) > FREQUENT_BB_DIFF)
    targpc = curpc;
  else {
    /* mixture of skip and noskip branches, 
     * or just not frequent enough,
     * so don't align */
    return 0;
  }

  if (Trace_Inst) {
#pragma mips_frequency_hint NEVER
    fprintf(TFile, "align_bb %d:  bb_freq = %f, prev_bb_freq = %f, skip_freq = %f, noskip_freq = %f\n", BB_id(bb), BB_freq(bb), BB_freq(BB_prev(bb)), skip_freq, noskip_freq);
  }

  if (Align_Instructions)
    targ_alignment = Align_Instructions;
  else
    targ_alignment = CGTARG_Text_Alignment();
  targ_alignment /= ISA_INST_BYTES;	/* so word-sized */
  targpc /= ISA_INST_BYTES;		/* so word-sized */

  //  fprintf(TFile, "  targ_alignment %d\n", targ_alignment);
  //  fprintf(TFile, "  targpc %d\n", targpc);

  if (Get_Trace(TP_EMIT, 0x400)) {
    /* To do this right for R10K, 
     * need to check whether paths/bb-chains
     * cross cache lines.  An easier check, which is also
     * compatible with beast and tfp is to just do the
     * quad-word alignment, which guarantees at least that
     * the cache-line crossing will be quad-aligned.
     * So leave this off by default.
     */
    /* align to cache line size (16 words) */
    /* only do it if block would no longer cross cache line */
#define R10K_PRIMARY_ICACHE_LINE_WORDS	16
    INT targ_cachesize = R10K_PRIMARY_ICACHE_LINE_WORDS;
    UINT ops_till_cacheline = (targ_cachesize - (targpc % targ_cachesize));
    INT aligned_line_crossings = BB_length(bb) / targ_cachesize;
    INT orig_line_crossings = (BB_length(bb) + targ_cachesize - ops_till_cacheline)  / targ_cachesize;
    if (aligned_line_crossings < orig_line_crossings &&
	ops_till_cacheline <= 4) {
      num_of_ops = ops_till_cacheline;
    }
  }
  else {
    num_of_ops = ((targ_alignment - (targpc % targ_alignment)) % targ_alignment);
  }

  return num_of_ops;
}
#endif

#ifdef TARG_ST
/* ====================================================================
 *   Check_If_Should_Align_PU
 * ====================================================================
 */
// [CG]: Changed this function interface.
// This function now returns the requested alignment for the PU if
// any.
// This is an optional feature activated in priority order by:
// -OPT:align_functions,
// -OPT:align_instructions.
// If no specific option is given, 0 must be returned and the function
// will be aligned by the DEFAULT_FUNCTION_ALIGNMENT value.
static INT
Check_If_Should_Align_PU (
  ST *pu
)
{
  INT align;

  if (OPT_Space) return 0;

  if (Align_Functions)
    align = Align_Functions;
  else if (Align_Instructions)
    align = Align_Instructions;
  else 
    align = 0;
  return align;
}
#else
/* ====================================================================
 *   Check_If_Should_Align_PU
 * ====================================================================
 */
static INT
Check_If_Should_Align_PU (
  INT curpc
)
{
  INT q;

  if (Align_Instructions) {
    q = Align_Instructions;
  }
  else if (OPT_Space) {
    return 0;
  }
  else {
    q = CGTARG_Text_Alignment();
  }
  q /= ISA_INST_BYTES;	                   /* so word-sized */
  return (q - ((curpc/ISA_INST_BYTES) % q)) % q;
}
#endif

/* ====================================================================
 *   Create_Cold_Text_Section
 *
 *    Scan the BBs in the region looking for cold BBs. If one is found,
 *    create the cold text section if it hasn't already been created.
 *    Also set the BB_cold BB flag accordingly.
 * ====================================================================
 */
static void
Create_Cold_Text_Section (void)
{
  BB *bb;

  FmtAssert(text_base != NULL, ("emit: text_base is null"));
  for (bb = REGION_First_BB; bb; bb = BB_next(bb)) {
#ifdef TARG_ST
    // [CG] hot/cold placement for .text section only
    if (Is_Text_Section(text_base) &&
#else 
    if (
#endif
	EMIT_use_cold_section && BB_Is_Cold(bb)) {
      if (cold_base == NULL) {
	ST *st = Copy_ST(text_base);
	Set_ST_blk(st, Copy_BLK(ST_blk(text_base)));
	Set_ST_name (st, Save_Str2(ELF_TEXT, ".cold"));
	Set_STB_size (st, 0);
	Set_STB_scninfo_idx(st, 0);
	Set_STB_section_idx(st, STB_section_idx(text_base));
	Init_Section (st);
	cold_base = st;

#ifdef TARG_ST
	// [CL] don't forget to initialize cold_section!
	cold_section = em_scn[STB_scninfo_idx(cold_base)].scninfo;
#endif
      }

      /* Check the remaining BBs in the region to verify they are
       * are cold. cgemit doesn't require this attribute, but currently
       * this is how the cold region is generated, so it's helpful
       * to verify it since someone forgetting to set the rid on a
       * new BB will cause trouble.
       */
      do {
	FmtAssert(BB_Is_Cold(bb),
		  ("emit: hot BB:%d found in cold region", BB_id(bb)));
	Set_BB_cold(bb);
      } while (bb = BB_next(bb));

      return;
    }

    Reset_BB_cold(bb);
  }
}

/* ====================================================================
 *   Setup_Text_Section_For_BB
 *
 *   Set PU_base, PU_section and PC according to whether <bb> is
 *   in the hot or cold region.
 * ====================================================================
 */
static void
Setup_Text_Section_For_BB (
  BB *bb
)
{
  BOOL cold_bb = BB_cold(bb);
  PU_base = cold_bb ? cold_base : text_base;
  if (cur_section != PU_base) {
    if (Assembly) {
      fprintf (Asm_File, "\n\t%s %s\n", AS_SECTION, ST_name(PU_base));
#ifdef TARG_ST
      // [CL] force alignment if we have just switched to cold
      // section for the current PU (as alignment constraints in
      // hb_hazards.cxx:Make_Bundles() are reset for each new
      // PU, we must reset alignment here)
      if ( (PU_base == cold_base) && (DEFAULT_FUNCTION_ALIGNMENT) ) {
	fprintf(Asm_File, "\t%s %d\n", AS_ALIGN, DEFAULT_FUNCTION_ALIGNMENT);
      }
#endif
    }
    if(Lai_Code) {
      fprintf (Lai_File, "\n\t%s %s\n", AS_SECTION, ST_name(PU_base));
    }
    if (cold_bb) {
      PU_section = cold_section;
      text_PC = PC;
      PC = cold_PC;
    } else {
      PU_section = text_section;
      cold_PC = PC;
      PC = text_PC;
    }
    cur_section = PU_base;
  }
}

static LABEL_IDX       prev_pu_last_label  = LABEL_IDX_ZERO;
static Dwarf_Unsigned  prev_pu_base_elfsym = 0;
static PU_IDX          prev_pu_pu_idx      = (PU_IDX) 0;
static Dwarf_Unsigned  prev_pu_end_offset_from_last_label = 0;
static char           *prev_pu_last_label_name = NULL;
static Dwarf_Unsigned  prev_pu_last_offset = 0;

static void
cache_last_label_info(LABEL_IDX      label_idx,
                      Dwarf_Unsigned base_elf_idx,
                      PU_IDX         pu_idx,
                      Dwarf_Unsigned end_offset)
{
  prev_pu_last_label  = label_idx;
  prev_pu_base_elfsym = base_elf_idx;
  prev_pu_pu_idx      = pu_idx;
  prev_pu_end_offset_from_last_label = end_offset;
  prev_pu_last_label_name = LABEL_name(label_idx);
  prev_pu_last_offset = Get_Label_Offset(label_idx);
}

static void
end_previous_text_region(pSCNINFO scninfo,
                         INT      end_offset)
{
#ifdef TARG_ST
  // CL: convert composite PC to actual PC address
  Em_Dwarf_End_Text_Region_Semi_Symbolic(scninfo,
                                         PC2Addr(end_offset),
                                         Cg_Dwarf_Symtab_Entry(CGD_LABIDX,
                                                               prev_pu_last_label,
                                                               prev_pu_base_elfsym,
                                                               prev_pu_pu_idx,
                                                               prev_pu_last_label_name,
                                                               prev_pu_last_offset),
                                         prev_pu_end_offset_from_last_label);
#else
  Em_Dwarf_End_Text_Region_Semi_Symbolic(scninfo,
                                         end_offset,
                                         Cg_Dwarf_Symtab_Entry(CGD_LABIDX,
                                                               prev_pu_last_label,
                                                               prev_pu_base_elfsym,
                                                               prev_pu_pu_idx,
                                                               prev_pu_last_label_name,
                                                               prev_pu_last_offset),
                                         prev_pu_end_offset_from_last_label);
#endif
}

#ifdef TARG_ST

// [CL] create a new label for a new source line
// code borrowed from Gen_Label_For_BB
static LABEL_IDX
Gen_Label_For_Source_Line() {
  char *buf;
  LABEL_IDX lab;
  LABEL *label;
#define  EXTRA_NAME_LEN  32
  static PU_IDX current_debug_pu = 0;
  static int debug_lab_id=0;

  /* Reset debug label id at each new PU */
  if (current_debug_pu != current_pu) {
    current_debug_pu = current_pu;
    debug_lab_id = 0;
  }

  /* Name this label: */
  buf = (char *)alloca(strlen(Cur_PU_Name) + EXTRA_NAME_LEN);
  sprintf(buf, Source_Line_Label_Format, debug_lab_id, Cur_PU_Name);

  label = &New_LABEL(CURRENT_SYMTAB, lab);
  LABEL_Init(*label, Save_Str(buf), LKIND_DEFAULT);

  debug_lab_id ++;
  return lab;
}

// [CL] Create a new label for each new source line
void New_Debug_Line_Set_Label(INT code_address)
{
  if (Last_Label > 0) {
    cache_last_label_info (Last_Label,
			   Em_Create_Section_Symbol(PU_section),
			   current_pu,
			   PC2Addr(Offset_From_Last_Label));

    pSCNINFO old_PU_section = em_scn[STB_scninfo_idx(PU_base)].scninfo;
    end_previous_text_region(old_PU_section, 
			     //  Em_Get_Section_Offset(old_PU_section));
			     Addr2PC(code_address));
  }

  Last_Label = Gen_Label_For_Source_Line ();
  Offset_From_Last_Label = 0;

  if (Assembly || Lai_Code) {
#ifdef TARG_ST
    fprintf (Output_File, "%s:\n", 
	     LABEL_name(Last_Label));
#else
    fprintf (Output_File, "%s:\t%s\n", 
	     LABEL_name(Last_Label), ASM_CMNT);
#endif
  }

  Em_Dwarf_Start_Text_Region_Semi_Symbolic (PU_section, code_address,
					    Cg_Dwarf_Symtab_Entry(CGD_LABIDX,
								  Last_Label,
								  ST_elf_index(text_base)),
					    PC2Addr(Offset_From_Last_Label));
}
#endif

/* ====================================================================
 *   Setup_Text_Section_For_PU
 *
 *   Setup, and create if necessary, the text section of the PU.  
 * ====================================================================
 */
static void
Setup_Text_Section_For_PU (
  ST *pu
)
{
  static ST *orig_text_base = NULL;
  ST        *old_base = PU_base;
  INT        i;

  if (text_base == NULL) {
    text_base = SEC_block(_SEC_TEXT);
  }
  orig_text_base = text_base;

  current_pu = ST_pu(pu);

  Initial_Pu_Label = LABEL_IDX_ZERO;

  if (! Object_Code && generate_elf_symbols) {
    // didn't actually write instructions,
    // but want the offset to be up-to-date.
    Em_Change_Section_Origin (
		em_scn[STB_scninfo_idx(orig_text_base)].scninfo,
		text_PC);
  }

  if (Section_For_Each_Function || PU_in_elf_section(current_pu)) {
    /* create new text section */
    text_base = Copy_ST(orig_text_base);
    Set_ST_blk(text_base, Copy_BLK(ST_blk(orig_text_base)));
#if 0
    if (Get_Trace ( TP_EMIT,128 )) {
      // use same .text name for each section
      Set_ST_name (text_base, ST_name_idx(orig_text_base));
    }
    else {
      char buf[16];
      sprintf(buf, "%s.", ELF_TEXT);
      Set_ST_name (text_base, Save_Str2(buf, ST_name(pu)));
    }
#else
    // use same .text name for each section
    char buf[16];
    sprintf(buf, "%s.", ELF_TEXT);
    Set_ST_name (text_base, Save_Str2(buf, ST_name(pu)));
#endif

    Set_STB_size (text_base, 0);
    Set_STB_scninfo_idx(text_base, 0);
    Set_STB_section_idx(text_base, STB_section_idx(orig_text_base));
    Init_Section (text_base);
    text_PC = 0;	/* starting new offsets */
  }
  else if (ST_base(pu) != text_base) {
    // e.g. for section attributes.
    text_base = ST_base(pu);
    text_PC = STB_size(text_base);
  }

  Set_STB_scninfo_idx(SP_Sym, STB_scninfo_idx(text_base));
  Set_STB_scninfo_idx(FP_Sym, STB_scninfo_idx(text_base));

#ifdef TARG_ST
  // [CG] Setup cold section from here as we need text_base
  Create_Cold_Text_Section();
#endif

  cur_section = text_base;
  if (generate_elf_symbols) {
    text_section = em_scn[STB_scninfo_idx(text_base)].scninfo;
    PU_section = text_section;
    i = Em_Get_Section_Offset (PU_section);
    Is_True(i == text_PC, ("Setup_Text_Section_For_PU: PC doesn't match"));
    text_PC = i;
  }

  if (Assembly) fprintf (Asm_File, "\t%s %s\n", AS_SECTION, ST_name(text_base));
  if (Lai_Code) fprintf (Lai_File, "\n\t%s %s\n", AS_SECTION, ST_name(text_base));

#ifdef TEMPORARY_STABS_FOR_GDB
  // This is an ugly hack to enable basic debugging for IA-32 target
  if (Assembly && Debug_Level > 0) {
    static BOOL marked_text_start = FALSE;
    if (!marked_text_start) {
      fprintf(Asm_File, ".Ltext0:\n");
      marked_text_start = TRUE;
    }
  }
#endif

#ifdef TARG_ST
  // [CG] Check_If_Should_Align_PU() is now used from EMT_Emit_PU()
#else
  i = Check_If_Should_Align_PU (text_PC);
  if (i != 0) {
    if (Assembly || Lai_Code) {
      UINT32 tmp, power;
      power = 0;
      for (tmp = STB_align(text_base); tmp > 1; tmp >>= 1) power++;
      if (Assembly) ASM_DIR_ALIGN(power, text_base);
      if (Lai_Code) fprintf(Lai_File, "\t%s %d\n", 
                                    AS_ALIGN,STB_align(text_base));
    }
#if 0
    if (Object_Code) {
      // these bytes will never be executed so just insert 0's and
      // then we don't have to worry about how to generate a nop for
      // the target arch.
      Em_Add_Zeros_To_Scn (PU_section, i * ISA_INST_BYTES, 1);
    }
#endif
    // increment text_PC by 'num' bundles
    text_PC = text_PC + (i * ISA_INST_BYTES);
  }
#endif

  // hack for supporting dwarf generation in assembly (suneel)
#ifndef TARG_ST
  // [CL] The label generated here is NOT to be used for
  // debug. Last_Label has been reset in EMT_Emit_PU after
  // emission of the previous PU
  // Initial_Pu_Label has been reset above
  Last_Label = Gen_Label_For_BB (REGION_First_BB);
  Offset_From_Last_Label = 0;
  if (Initial_Pu_Label == LABEL_IDX_ZERO) {
    Initial_Pu_Label = Last_Label;
  }
#else
  Initial_Pu_Label = Gen_Label_For_BB (REGION_First_BB);
#endif

  /* CL: Start a new "text region" for Dwarf at each
     function start (instead of only when changing
     sections */
#ifndef TARG_ST
  /* check if we are changing sections. */
  if (text_base != old_base) {
#endif
    if (generate_elf_symbols && old_base != NULL) {
      pSCNINFO old_section = em_scn[STB_scninfo_idx(old_base)].scninfo;
      // Arange is symbolic; line number entries (if used) are not.
#ifndef TARG_ST
      // [CL] this is already performed in New_Debug_Line_Set_Label
      end_previous_text_region(old_section, 
                                   Em_Get_Section_Offset(old_section));
#endif
    }
#ifndef TARG_ST
    // [CL] new regions are started in New_Debug_Line_Set_Label
    if (generate_dwarf) {
    	Em_Dwarf_Start_Text_Region_Semi_Symbolic (PU_section, text_PC,
			Cg_Dwarf_Symtab_Entry(CGD_LABIDX,
			Last_Label,
			ST_elf_index(text_base)),
			Offset_From_Last_Label);
    }
  }
#endif

  PC = text_PC;
  PU_base = text_base;

  return;
}

/* ====================================================================
 *   put_TN_comment
 *
 *   Add comment about the TN to comment buffer.
 * ====================================================================
 */
static void
put_TN_comment (
  TN *t, 
  BOOL add_name, 
  vstring *comment
)
{
  if (!add_name) return;	// don't duplicate name
  if (!TN_is_constant(t)) return;

  INT64 val = TN_offset(t);

  if (TN_is_symbol(t)) {
    if (ST_class(TN_var(t)) == CLASS_CONST) {
      *comment = vstr_concat (*comment, 
		   Targ_Print (NULL, ST_tcon_val(TN_var(t))) );
    }
    else {
#ifdef TARG_ST
      /* (cbr) demangle name */
      char *cp_name = cplus_demangle(ST_name(TN_var(t)), DMGL_NO_OPTS);
      if (cp_name)
        *comment = vstr_concat(*comment, cp_name);
      else
#endif
      *comment = vstr_concat (*comment, ST_name(TN_var(t)));
    }
    if (TN_offset(t) != 0) {
      vstr_sprintf (comment, vstr_len(*comment), "%+lld", val);
    }
  }
  else if (TN_is_label(t) && val != 0) {
    *comment = vstr_concat (*comment, LABEL_name(TN_label(t)));
    vstr_sprintf (comment, vstr_len(*comment), "%+lld", val); 
  }
}

/* ====================================================================
 *   r_apply_l_const
 *
 *   Format the given constant TN's "value" into the given buffer.
 *   Return whether the symbol name should be added to comments.
 * ====================================================================
 */
static BOOL
r_apply_l_const (
  OP *op,		/* OP with constant operand */
  INT opidx,		/* OP index of constant TN of some sort */
  vstring *buf		/* A buffer to format it into */
)
{
  TN *t = OP_opnd(op, opidx);
  INT paren = 0;
  BOOL hexfmt = FALSE;
  BOOL print_TN_offset = TRUE;
  BOOL add_name = FALSE;
  ST *st;
  INT64 val;

#if 0
  fprintf(TFile,"  const TN ");
  Print_TN(t, TRUE);
  fprintf(TFile, "\n");
#endif

  /* special case for stack symbols */
  if (TN_is_symbol(t)) {
    ST *base_st;
    INT64 base_ofst;

    st = TN_var(t);
    Base_Symbol_And_Offset (st, &base_st, &base_ofst);
    if (base_st == SP_Sym || base_st == FP_Sym) {
        if (Pointer_Size <= 4) {
            INT32 val32 = CGTARG_TN_Value (t, base_ofst);
            vstr_sprintf (buf, 
                          vstr_len(*buf), 
                          "%d",
                          val32);
        } else { 
            INT64 val64 = CGTARG_TN_Value (t, base_ofst);
            vstr_sprintf (buf, 
                          vstr_len(*buf), 
                          "%lld",
                          val64);
        }
      return TRUE;
    }
  }

  val = TN_offset(t);
  if ( TN_is_reloc_neg(t) ) {
    *buf = vstr_concat (*buf, "-");
  }

  if (TN_is_symbol(t)) { // not SP or FP relative
    st = TN_var(t);

    if (ST_class(st) == CLASS_CONST) {
#ifdef TARG_ST
      //
      // Arthur: you can not always address a constant as an offset
      //         from the section
      //

      TCON tc = ST_tcon_val(st);
      //
      // For floats make a bit pattern, others - symbolic name
      //
      if (MTYPE_is_float(TCON_ty(tc)) && !CG_floating_const_in_memory) {

	union {
	  INT32 i;
	  float f;
	} val;

#if __GNUC__ < 3
	// [CL] copying floats on x86 with gcc-2.95 implies conversions
	// which may change the copied value (especially in our case where
	// the lo/hi parts of a double are not generally "plain" floats
	// They are likely to be NaNs and as such candidate to conversions
	// which should apply. It would be much cleaner to split a double
	// into 2 parts which are *not* float and have special types for
	/// the compiler
	// In addition, the union above implies that
	// sizeof(INT32) == sizeof(float)
	memcpy(&val.f, &tc.vals.f.fval, sizeof(val.f));
#else
	val.f = TCON_fval(tc);
#endif
	vstr_sprintf (buf, vstr_len(*buf), "%#x", val.i);

      }
      else {
	//
	// Print out a symbolic name
	//
	if (TN_relocs(t) != 0) {

	  hexfmt = TRUE;
	  print_TN_offset = TRUE;
	  // add name to comments in case was confused by gp_disp.
	  add_name = TRUE;
	  paren = TN_Relocs_In_Asm (t, st, buf, &val);
	  *buf = vstr_concat (*buf, "(" );
	}
#ifndef TARG_ST
	vstr_sprintf (buf, vstr_len(*buf), "%s_UNNAMED_CONST_%d", Local_Label_Prefix, ST_tcon(st));
#else
	r_qualified_tcon_name(st, buf);
#endif
	//	// call put_symbol so that we emit .type info, once per symbol
	// (void) EMT_Put_Elf_Symbol (st);
      }
#else
      *buf = vstr_concat(*buf, ST_name(st));
      if (*Symbol_Name_Suffix != '\0')
	*buf = vstr_concat(*buf, Symbol_Name_Suffix);
#endif
    }
    else {
      if (ST_sclass(st) != SCLASS_COMMON) {
	// call put_symbol so that we emit .type info, once per symbol
	(void)EMT_Put_Elf_Symbol (st);
      }

      if (TN_relocs(t) != 0) {
#if 0
	// use base if referring to current pu or local data
	if (CGEMIT_Use_Base_ST_For_Reloc (TN_relocs(t), st)) {
	  ST *base_st;
	  INT64 base_ofst;
	  Base_Symbol_And_Offset (st, &base_st, &base_ofst);
	  val += base_ofst;
	  st = base_st;
	}
#endif
	if (Use_Separate_PU_Section(current_pu,st)) {
	  /* use PU text section rather than generic one */
	  st = PU_base;
	}

	hexfmt = TRUE;
	print_TN_offset = TRUE;
	// add name to comments in case was confused by gp_disp.
	add_name = TRUE;
	paren = TN_Relocs_In_Asm (t, st, buf, &val);
	*buf = vstr_concat (*buf, "(" );
	r_qualified_name (st, buf);
      } 
      else {
	r_qualified_name (st, buf);
      }
    } /* not CLASS_CONST */
  }
  else if ( TN_is_label(t) ) {
    if (val != 0) {
      // don't use "." cause that can have varying meaning
      // when have multiple instruction slots.
      // Instead just do label+offset.
      *buf = vstr_concat (*buf, LABEL_name(TN_label(t)));
      vstr_sprintf (buf, vstr_len(*buf), "%+lld", val); 
    }
    else {
      *buf = vstr_concat(*buf, LABEL_name(TN_label(t)));
      if (isdigit(LABEL_name(TN_label(t))[0])) {
      	*buf = vstr_concat(*buf, 
                (PC > Get_Label_Offset(TN_label(t))) ? "b" : "f");
      }
    }
    print_TN_offset = FALSE;
  }
  else if (TN_is_tag(t)) {
    *buf = vstr_concat(*buf, LABEL_name(TN_label(t)));
    print_TN_offset = FALSE;
  }
  else if (TN_is_enum(t)) {
    if (ISA_PRINT_Operand_Is_Part_Of_Name(OP_code(op), opidx)) {
      vstr_sprintf (buf, vstr_len(*buf), "%s", ISA_ECV_Name(TN_enum(t)) );
    } else {
      vstr_sprintf (buf, vstr_len(*buf), "%d", ISA_ECV_Intval(TN_enum(t)) );
    }
    print_TN_offset = FALSE;	/* because value used instead */
  }
  else if ( TN_has_value(t) ) {
#if 0
    if ( TN_size(t) <= 4 ) {
      vstr_sprintf (buf, 
		    vstr_len(*buf), 
		    hexfmt ? "0x%x" : "%d",
		    (mINT32)TN_value(t) );
    }
    else {
      vstr_sprintf (buf, 
		    vstr_len(*buf), 
		    hexfmt ? "0x%llx" : "%lld",
		    TN_value(t) );
    }
#else
    if ( TN_size(t) <= 4 )
      vstr_sprintf (buf, vstr_len(*buf), 
		(hexfmt ? "0x%x" : "%d"), (mINT32)TN_value(t) );
    else
      vstr_sprintf (buf, vstr_len(*buf), 
      		(hexfmt ? "0x%llx" : "%lld"), TN_value(t) );
#endif
    print_TN_offset = FALSE;	/* because value used instead */
  }
  else {
    #pragma mips_frequency_hint NEVER
    FmtAssert (FALSE, ("r_apply_l_const: illegal constant TN"));
  }

  if (print_TN_offset && (val != 0)) {
    vstr_sprintf (buf, vstr_len(*buf), "%+lld", val );
  }

  while ( paren > 0 ) {
    *buf = vstr_concat(*buf, ")");
    --paren;
  }
  return add_name;
}

/* ====================================================================
 *   print_prefetch_info(char *comment, WN *wn)
 *
 *   add prefetch info comments to the assembler listing.
 * ====================================================================
 */
static void 
print_prefetch_info(
  vstring *comment, 
  WN *wn
)
{

  if (!wn) return;

  if (WN_pf_stride_1L (wn) != 0) {
    *comment = vstr_concat(*comment, "L1");
    return;
  }

  if (WN_pf_stride_2L (wn) != 0 ) {
    *comment = vstr_concat(*comment, "L2");
  }
}

/* ====================================================================
 *   r_assemble_opnd
 * ====================================================================
 */
static char*
r_assemble_opnd (
  OP      *op,
  INT      i,
  vstring *buf,
  BOOL    *add_name
)
{
  char  vname[10];
  INT   start = vstr_len(*buf);	// start of operand string
  TN   *t = OP_opnd(op,i);

  if (TN_is_constant(t)) {
    *add_name |= r_apply_l_const (op, i, buf);
  }
  else if (TN_is_true_pred(t)) {
    /* nada */ ;
  }
  else {
    const char *rname;
    ISA_REGISTER_CLASS rc = TN_register_class(t);
    REGISTER reg = TN_register(t);

    // If register is physical, print its real name, otherwise
    // virtual:
    if (reg != REGISTER_UNDEFINED) {

      ISA_REGISTER_SUBCLASS sc = OP_opnd_reg_subclass(op,i);

      if (REGISTER_SET_MemberP(REGISTER_SUBCLASS_members(sc), reg)
	      && REGISTER_SUBCLASS_reg_name(sc, reg)) {
	rname = REGISTER_SUBCLASS_reg_name(sc, reg);
      } 
      else if (List_Software_Names) {
	rname = ABI_PROPERTY_Reg_Name(rc, REGISTER_machine_id(rc, reg));
      } 
      else {
	rname = REGISTER_name(rc, reg);
      }
    }
    else {
      sprintf(vname, "%s%d", ISA_REGISTER_CLASS_Symbol(rc), TN_number(t));
      rname = vname;
    }

    if (OP_has_predicate(op) && i == OP_PREDICATE_OPND) {
#ifdef TARG_IA64
      vstr_sprintf(buf, start, ISA_PRINT_PREDICATE, rname);
#else
      if (TOP_is_guard_t(OP_code(op)))
	vstr_sprintf(buf, start, True_Predicate_Format, rname);
      else if (TOP_is_guard_f(OP_code(op)))
	vstr_sprintf(buf, start, False_Predicate_Format, rname);
#endif
    } 
    else {
      *buf = vstr_concat(*buf, rname);
    }
  }

  // need end-of-string between each operand
  *buf = vstr_append(*buf, '\0');

  return vstr_str(*buf)+start;
}

/* ====================================================================
 *   r_assemble_result
 * ====================================================================
 */
static char*
r_assemble_result (
  OP *op,
  INT i,
  vstring *buf
)
{
  char  vname[10];
  INT start = vstr_len(*buf);	// start of operand string
  TN *t = OP_result(op,i);

  const char *rname;
  ISA_REGISTER_SUBCLASS sc = OP_result_reg_subclass(op,i);
  ISA_REGISTER_CLASS rc = TN_register_class(t);
  REGISTER reg = TN_register(t);

  FmtAssert (t != NULL && !TN_is_constant(t),
	                     ("r_assemble_list: illegal result tn"));

  if (reg != REGISTER_UNDEFINED) {
    if (REGISTER_SET_MemberP(REGISTER_SUBCLASS_members(sc), reg)
	  && REGISTER_SUBCLASS_reg_name(sc, reg)) {
      rname = REGISTER_SUBCLASS_reg_name(sc, reg);
    } 
    else if (List_Software_Names) {
      rname = ABI_PROPERTY_Reg_Name(rc, REGISTER_machine_id(rc, reg));
    } 
    else {
      rname = REGISTER_name(rc, reg);
    }
  }
  else {
    sprintf(vname, "%s%d", ISA_REGISTER_CLASS_Symbol(rc), TN_number(t));
    rname = vname;
  }

  *buf = vstr_concat(*buf, rname);
  *buf = vstr_append(*buf, '\0');	// increment vstr length
  return vstr_str(*buf)+start;
}

/* ====================================================================
 *   CGEMIT_Print_Intrinsic_OP
 *
 *   TODO: target-dependent.
 *         On a given target, a TOP_intrncall format must be known.
 *         For example, ST100 must maintain the following convension:
 *         TOP_intrncall -- var_opnds;
 *         opnd[0]       -- predicate
 *         opnd[1]       -- symbolic TN with intrinsic name
 *
 * ====================================================================
 */
static INT
CGEMIT_Print_Intrinsic_OP (
  OP *op,
  const char **result,
  const char **opnd,
  FILE *f
)
{
  INT i;
  INT st;

  // TODO: add description of intrinsic predicated or not.
  //       add description of printing format ?
  //
  // For now assume all intrinsics are predicated ? opnd[0]
  //
  st = fprintf (f, "%s\t", opnd[0]);

  //
  // print name
  //
  st += fprintf (f, "%s\t", opnd[1]);

  //
  // print results
  //
  for (i = 0; i < OP_results(op); i++) {
    st += fprintf (f, "%s, ", result[i]);
  }

  //
  // print opnds
  //
  for (i = 2; i < OP_opnds(op)-1; i++) {
    st += fprintf (f, "%s, ", opnd[i]);
  }
  if (OP_opnds(op) > 2) {
    st += fprintf (f, "%s", opnd[OP_opnds(op)-1]);
  }

  if (st == -1) {
    FmtAssert(FALSE,("CGEMIT_Print_Intrinsic_OP: fprintf failed"));
  }
  else return st;
}

/* ====================================================================
 *   r_assemble_list
 *
 *   Emit a pseudo-assembly listing for the given OP.
 * ====================================================================
 */
static void 
r_assemble_list (
  OP *op,		/* The operation being listed */
  BB *bb
)
{
  const char *result[ISA_OPERAND_max_results];
  const char *opnd[ISA_OPERAND_max_operands];
  vstring buf = vstr_begin(LBUF_LEN);
  INT i;
  INT lc = 0;
  BOOL add_name = FALSE;

#if 0
  Emit_Unwind_Directives_For_OP(op, Asm_File);
#endif

#ifdef GAS_TAGS_WORKED
// un-ifdef this when gas can handle tags inside explicit bundle
#ifdef TARG_ST
  if (Get_OP_Tag(op)) {
#else
  if (OP_has_tag(op)) {
#endif
	fprintf(Asm_File, "%s:\n", LABEL_name(Get_OP_Tag(op)));
  }
#endif

#if defined(TARG_ST100) && !defined(__ASM_SUPPORTS_REGMASK_OPERAND)
extern BOOL Hack_For_Printing_Push_Pop (OP *op, FILE *file);
// Arthur: the GHS assembler isn't able to take
//         a register mask as operand. However, push/pop
//         is described in the architecture as having a 
//         mask as an operand. So, I need this hack ...
  if (Assembly && Hack_For_Printing_Push_Pop (op, Asm_File)) {
    return ;
  }
#endif

  for (i = 0; i < OP_opnds(op); i++) {
    opnd[i] = r_assemble_opnd(op, i, &buf, &add_name);
  }

  for (i = 0; i < OP_results(op); i++) {
    result[i] = r_assemble_result (op, i, &buf);
  }

  if (Assembly) {
    fputc ('\t', Asm_File);
    lc = TI_ASM_Print_Inst(OP_code(op), result, opnd, Asm_File);
  }
  if (Lai_Code) {
    fputc ('\t', Lai_File);
    if (OP_code(op) == TOP_intrncall) {
      lc = CGEMIT_Print_Intrinsic_OP(op, result, opnd, Lai_File);
    }
    else {
      lc = TI_ASM_Print_Inst(OP_code(op), result, opnd, Lai_File);
    }
  }
  FmtAssert (lc != TI_RC_ERROR, ("%s", TI_errmsg));
  vstr_end(buf);

#if 0
  if (OP_end_group(op) && Assembly) {
    lc += fprintf(Asm_File, " %s", ISA_PRINT_END_GROUP);
  }
#endif

  while (lc < 30) {
    if (Assembly || Lai_Code) fputc (' ', Output_File);
    lc++;
  }

  /* emit TN comments: */
  vstring comment = vstr_begin(LBUF_LEN);

#ifdef TARG_ST
  char cycle_info[30];
  cycle_info[0] = '\0';
#endif

  for (i = 0; i < OP_results(op); i++) {
    TN *tn = OP_result(op, i);
    if (TN_is_save_reg(tn)) {
      vstr_sprintf (&comment, vstr_len(comment), "(sv:%s)", 
                  REGISTER_name(TN_save_rclass(tn), TN_save_reg(tn)));
    }
  }
  for (i = 0; i < OP_opnds(op); i++) {
    put_TN_comment (OP_opnd(op,i), add_name, &comment);
  }

#ifndef TARG_ST
  /* SC: Do this only if we have a comment.  We do not know
     yet, so do it later.
  */
  if (Assembly || Lai_Code) fprintf (Output_File, "\t%s", ASM_CMNT);
#endif
  if (vstr_len(comment) == 0) {
    WN *wn = Get_WN_From_Memory_OP (op);
    if (wn && Alias_Manager) {
      char tbuf[LBUF_LEN];
      tbuf[0] = '\0';
      Print_alias_info (tbuf, Alias_Manager, wn);
      comment = vstr_concat(comment, tbuf);
    }
    if (wn && OP_prefetch(op) && vstr_len(comment) == 0) {
      print_prefetch_info(&comment, wn);
    }
  }

  /* Add target PU name as comment for normal call and tail call OPs.
   */
  if ((OP_call(op) || OP_tail_call(op)) && vstr_len(comment) == 0) {
    ANNOTATION *ant = ANNOT_Get (BB_annotations(bb), ANNOT_CALLINFO);
    if (ant != NULL) {
	ST *call_sym = CALLINFO_call_st(ANNOT_callinfo(ant));
	if (call_sym != NULL) {
#ifdef TARG_ST
          /* (cbr) demangle name */
          char *cp_name = cplus_demangle(ST_name(call_sym), DMGL_NO_OPTS);
          if (cp_name)
            comment = vstr_concat(comment, cp_name);
          else
#endif
            comment = vstr_concat(comment, ST_name(call_sym));
	}
    }
  }

#ifdef TARG_ST
  if (OP_ssa_move(op) && vstr_len(comment) == 0) {
    comment = vstr_concat(comment, "ssa_move");
  }
#endif

#ifdef TARG_ST
  if (Assembly || Lai_Code) {
    if (Trace_Sched && (OP_scycle(op) >= 0)) {
      sprintf (cycle_info, "(cycle %d)  ", OP_scycle(op));
    }
    if ((OP_scycle(op) >= 0) &&
	(OP_scycle(op) < Bundle_Count) &&
	(!OP_xfer(op) || Trace_Sched))
      DevWarn("BB %d, bundle %d: operation %s was scheduled at cycle %d",
	      BB_id(bb), Bundle_Count, TOP_Name(OP_code(op)), OP_scycle(op));
  }
#endif

#ifdef TARG_ST
  if ((Assembly || Lai_Code)
      && List_Notes
      && (vstr_len(comment) > 0 || strlen(cycle_info) > 0)) {
    fprintf (Output_File, "\t%s %s%s", ASM_CMNT, cycle_info, vstr_str(comment));
  }
  fputc ('\n', Output_File);
#else
    fprintf (Output_File, "  %s\n", vstr_str(comment));
#endif

  vstr_end(comment);

  return;
}

/* ====================================================================
 *   Verify_Operand
 *
 *   Verify that the specified operand is valid for the particular
 *   instruction operand.
 * ====================================================================
 */
static void 
Verify_Operand (
  const ISA_OPERAND_INFO *oinfo,
  OP *op, 
  INT opnd, 
  BOOL is_result
)
{
  const ISA_OPERAND_VALTYP *vtype =   is_result 
				    ? ISA_OPERAND_INFO_Result(oinfo, opnd)
				    : ISA_OPERAND_INFO_Operand(oinfo, opnd);
  TN *tn = is_result ? OP_result(op, opnd) : OP_opnd(op, opnd);
  const char * const res_or_opnd = is_result ? "result" : "operand";

  if (ISA_OPERAND_VALTYP_Is_Register(vtype)) {
    REGISTER_SET class_regs;
    ISA_REGISTER_SUBCLASS sc = ISA_OPERAND_VALTYP_Register_Subclass(vtype);
    ISA_REGISTER_CLASS rc = ISA_OPERAND_VALTYP_Register_Class(vtype);
    REGISTER reg = TN_register(tn);

    if (!TN_is_register(tn)) goto incorrect_register;
    if (TN_register_class(tn) != rc) goto incorrect_register;
    if (reg != REGISTER_UNDEFINED) {
      class_regs =   (sc == ISA_REGISTER_SUBCLASS_UNDEFINED)
		 ? REGISTER_CLASS_universe(rc)
		 : REGISTER_SUBCLASS_members(sc);
      if (!REGISTER_SET_MemberP(class_regs, reg)) goto incorrect_register;
    }
#ifdef TARG_ST
    if (sc != ISA_REGISTER_SUBCLASS_UNDEFINED) {
      const ISA_REGISTER_SUBCLASS_INFO *info = ISA_REGISTER_SUBCLASS_Info (sc);
      if (ISA_REGISTER_SUBCLASS_INFO_Count (info) == 1) {
 	// Register TN for a singleton subclass must use a
 	// dedicated register.
 	if (! TN_is_dedicated(tn)) goto incorrect_register;
      }
    }
#endif
    return;
  incorrect_register:
#if 0
     {
      int i;
      int opndnum = OP_opnds(op);
      int resnum = OP_results(op);
      fprintf(TFile, "Incorrect register operand in:\n");
      fprintf(TFile, "BB:%d  %s ",
	      BB_id(OP_bb(op)),TOP_Name(OP_code(op)));
      for (i = 0; i < resnum; i++) {
	fprintf(TFile," ");
	Print_TN(OP_result(op, i),TRUE);
      }
      fprintf(TFile,"=");
      for (i = 0; i < opndnum; i++) {
	fprintf(TFile," ");
	Print_TN(OP_opnd(op, i),TRUE);
      }
      fprintf(TFile,"\n");
    }
#endif
    FmtAssert(0,
	      ("incorrect register for %s %d in opcode %s", 
	       res_or_opnd, opnd, TOP_Name(OP_code(op))));
  } else if (ISA_OPERAND_VALTYP_Is_Literal(vtype)) {
    FmtAssert(TN_is_constant(tn),
	     ("%s %d is not a constant", res_or_opnd, opnd));

    if (TN_has_value(tn)) {
      ISA_LIT_CLASS lc = ISA_OPERAND_VALTYP_Literal_Class(vtype);
      INT64 imm = TN_value(tn);

      if ((TFile != stdout) && !ISA_LC_Value_In_Class(imm, lc)) {
        Print_OP_No_SrcLine (op);
      }
      FmtAssert(ISA_LC_Value_In_Class(imm, lc),
	        ("literal for %s %d is not in range", res_or_opnd, opnd));
    } else if (TN_is_label(tn)) {

#ifndef TARG_ST
      // [CG]: This check is wrong, does not handle variable bundles
#if Is_True_On
      LABEL_IDX lab = TN_label(tn);
      INT64 offset = TN_offset(tn);
      INT64 val = Get_Label_Offset(lab) + offset;
      if (CG_opt_level > 0 && CFLOW_Enable && !OP_likely(op)) {
	INT nextpc = PC + sizeof(ISA_BUNDLE);
	if (PROC_has_branch_delay_slot()) nextpc += sizeof(ISA_PACK_INST);

	if (val == nextpc) {
	  DevWarn("branch to next instruction at PC=0x%x", PC);

	  Print_OP_No_SrcLine(op);
	  fprintf(TFile, " PC = %d; TN_offset = %lld\n", PC, offset);
	  fprintf(TFile, " val = %lld; nextpc = %d\n", val, nextpc);

	}
      }
      if (Get_Label_Offset(lab) == 0 && offset == 0) {
	BBLIST *item;
	BB *label_bb = Get_Label_BB(lab);
	BOOL okay = FALSE;
	FOR_ALL_BB_SUCCS(OP_bb(op), item) {
	  if (BBLIST_item(item) == label_bb) okay = TRUE;
	}
	if (!okay) {
	  DevWarn("branch to 0? (possible bug at PC=0x%x, label %s)", 
		   PC, LABEL_name(lab));
	}
      }
#endif
#endif
    }
  } else if (ISA_OPERAND_VALTYP_Is_Enum(vtype)) {
    ISA_ENUM_CLASS_VALUE ecv;
    ISA_ENUM_CLASS ec = ISA_OPERAND_VALTYP_Enum_Class(vtype);
    ISA_ENUM_CLASS_VALUE first_ecv = ISA_EC_First_Value(ec);
    ISA_ENUM_CLASS_VALUE last_ecv = ISA_EC_Last_Value(ec);

    FmtAssert(TN_is_enum(tn),
	      ("%s %d is not an enum", res_or_opnd, opnd));

    ecv = TN_enum(tn);
    FmtAssert(ecv >= first_ecv && ecv <= last_ecv,
	      ("enum for %s %d is not in range", res_or_opnd, opnd));
  } else {
    FmtAssert(FALSE, ("unhandled vtype in Verify_Operand"));
  }
}

/* ====================================================================
 *   Verify_Instruction
 *
 *   Verify that the specified OP contains valid information for
 *   the instruction it represents.
 * ====================================================================
 */
void 
Verify_Instruction (
  OP *op
)
{
  INT i;
  const ISA_OPERAND_INFO *oinfo;
  TOP top = OP_code(op);

  // ??? check for valid topcode?

  FmtAssert(ISA_SUBSET_Member(ISA_SUBSET_Value, top),
	    ("%s is a member of ISA %s", 
	     TOP_Name(top), 
	     ISA_SUBSET_Name(ISA_SUBSET_Value)));

  oinfo = ISA_OPERAND_Info(top);

  INT results = OP_results(op);
  if (results != OP_fixed_results(op)) {
    FmtAssert(TOP_is_var_opnds(top) && results > OP_fixed_results(op),
	      ("wrong number of results (%d) for %s",
	       results,
	       TOP_Name(top)));
    results = OP_fixed_results(op); // can only verify fixed results
  }
  for (i = 0; i < results; ++i) {
    Verify_Operand(oinfo, op, i, TRUE);
  }

  INT opnds = OP_opnds(op);
  if (opnds != OP_fixed_opnds(op)) {
    FmtAssert(TOP_is_var_opnds(top) && opnds > OP_fixed_opnds(op),
	      ("wrong number of operands (%d) for %s",
	       opnds,
	       TOP_Name(top)));
    opnds = OP_fixed_opnds(op); // can only verify fixed operands
  }
  for (i = 0; i < opnds; ++i) {
    Verify_Operand(oinfo, op, i, FALSE);
  }

#if Is_True_On
  // [CG]: Test cond/jump to next bb
  //if (CG_opt_level > 0 && CFLOW_Enable && !OP_likely(op)) {
    if (OP_jump(op) || OP_cond(op)) {
      int idx = TOP_Find_Operand_Use(OP_code(op), OU_target);
      TN *dest = OP_opnd(op, idx);
      BB *bb = OP_bb(op);
      BB *bb_next = BB_next(bb);
      //FdF: TN_is_label(dest) may not be true after tail_call optimization
      // DevAssert(TN_is_label(dest), ("expected label on branch at BB:%d", BB_id(bb)));
      if (TN_is_label(dest) &&
	  bb_next && BB_cold(bb) == BB_cold(bb_next) &&
	  Is_Label_For_BB(TN_label(dest), bb_next) &&
	  TN_offset(dest) == 0) {
	DevWarn("branch to next BB at BB:%d", BB_id(bb));
      }
    }
    //}
#endif
}

/* ====================================================================
 *   Assemble_OP
 *
 *   Write out the 'op' into the object file and/or into the assembly 
 *   file. see r_assemble_op() in cg/cgemit.cxx
 * ====================================================================
 */
static INT
Assemble_OP (
  OP *op, 
  BB *bb, 
  ISA_BUNDLE *bundle,
  INT slot
)
{
  INT words;
  // INT i;

  if (Trace_Inst) {
    fprintf(TFile, "<cgemit> ");
    Print_OP(op);
  }

  Verify_Instruction(op);

  if (OP_prefetch(op)) Use_Prefetch = TRUE;

#ifdef TARG_ST200 // CL: with variable-length bundles
                  //     we call Cg_Dwarf_Add_Line_Entry
                  //     only at bundle starts in
                  //     Assemble_Bundles
#else
  Cg_Dwarf_Add_Line_Entry (PC, OP_srcpos(op));
#endif

  if (Assembly || Lai_Code) {
    r_assemble_list ( op, bb );
    if (!Object_Code) words = ISA_PACK_Inst_Words(OP_code(op));
  }

  if (OP_end_group(op) && Assembly) {
    fprintf(Asm_File, "\t %s", ISA_PRINT_END_GROUP);
#ifdef TARG_ST
    if (Trace_Sched && List_Notes) {
      fprintf(Asm_File, " %s (bundle %d)", ASM_CMNT, Bundle_Count);
      Bundle_Count ++;
    }
#endif
    fputc('\n', Asm_File);
#ifdef TARG_ST
    // [SC]: Output symbol after the group.
    if (Symbol_Def_After_Group) {
      EMT_Write_Qualified_Name (Asm_File, Symbol_Def_After_Group);
      fprintf (Asm_File, ":\n");
      Symbol_Def_After_Group = NULL;
    }
#endif
  }

#if 0
  if (Object_Code) {
    ISA_PACK_INST inst[ISA_PACK_MAX_INST_WORDS];
    words = r_assemble_binary ( op, bb, inst );
    for (i = 0; i < words; ++i) {
      ISA_BUNDLE_PACK_COMP slot_comp = (ISA_BUNDLE_PACK_COMP)
	  (ISA_BUNDLE_PACK_COMP_slot + slot++);
      TI_ASM_Set_Bundle_Comp(bundle, slot_comp, inst[i]);
      if (slot == ISA_MAX_SLOTS) {
	slot = 0;
	++bundle;
      }
    }

    /* May need to add information about this call (or tail call) site */
    if (OP_call(op) || OP_tail_call(op)) {
      CGEMIT_Add_Call_Information (op, bb, PC, PU_section);
    }
if (Get_Trace ( TP_EMIT,0x100 )) {
/* don't do this till decide on format of EK_SWITCH */
    if (OP_ijump(op) && !OP_call(op) && !BB_exit(bb)) {
      ANNOTATION *ant = ANNOT_Get(BB_annotations(bb), ANNOT_SWITCH);
      if (ant != NULL) {
	ST *jumptable = ANNOT_switch(ant);
	BOOL gprel = ST_is_gp_relative(jumptable);
	INT num_entries = TY_AR_ubnd_val(ST_type(jumptable),0)+1;
	Elf_Event_Kind event_type;
	event_type = (Use_32_Bit_Pointers ? EK_SWITCH_32 : EK_SWITCH_64);

	Em_Add_New_Event (event_type, PC, 
		gprel, EMT_Put_Elf_Symbol(jumptable), num_entries, PU_section);
      }
    }
}
  }
#endif

  PC = PC_Incr_N(PC, words);

  // hack to keep track of last label and offset for assembly dwarf (suneel)
  // If the current op is a TOP_asm, mark the Last_Label as invalid.
  if (OP_code(op) == TOP_asm) {
    Last_Label = LABEL_IDX_ZERO;
  }
  else {
    Offset_From_Last_Label = PC_Incr_N(Offset_From_Last_Label, words);
  }

  return words;
}

/* ====================================================================
 *   Generate_Asm_String
 * ====================================================================
 */
static char*
Generate_Asm_String (
  OP* asm_op,
  BB *bb
)
{
  UINT i;
  ASM_OP_ANNOT* asm_info = (ASM_OP_ANNOT*) OP_MAP_Get(OP_Asm_Map, asm_op);
  char* asm_string = strdup(WN_asm_string(ASM_OP_wn(asm_info)));

  for (i = 0; i < OP_results(asm_op); i++) {
    asm_string = CGTARG_Modify_Asm_String(asm_string, 
                                   ASM_OP_result_position(asm_info)[i], 
                                   ASM_OP_result_memory(asm_info)[i], 
                                   OP_result(asm_op, i));
  }

  for (i = 0; i < OP_opnds(asm_op); i++) {
    asm_string = CGTARG_Modify_Asm_String(asm_string, 
                                   ASM_OP_opnd_position(asm_info)[i], 
                                   ASM_OP_opnd_memory(asm_info)[i], 
                                   OP_opnd(asm_op, i));
  }

  CGTARG_Postprocess_Asm_String(asm_string);

  return asm_string;
}

/* ====================================================================
 *   Assemble_Simulated_OP
 *
 *   Assemble a simulated OP.
 * ====================================================================
 */
#ifdef TARG_ST
/* [SC] if TARG_ST, Assemble_Simulated_OP has two choices:
 *      1. it may output ops directly (as in the original open64 code), or
 *      2. it may return ops to be inserted in the current bundle.
 *         This allows the target to fully bundle simulated ops if
 *      desired.
 */
#endif
static void
Assemble_Simulated_OP (
  OP *op, 
  BB *bb
#ifdef TARG_ST
  , OPS *ops
#endif
)
{
  /* ASM is an odd case so we handle it specially. It doesn't expand
   * into a series of OPs, but rather we just emit the asm string into
   * the assembly output.
   */
  if (OP_code(op) == TOP_asm) {
    FmtAssert((Assembly || Lai_Code) && !Object_Code,
	      ("can't emit object code when ASM"));
    if (Assembly) {
#ifndef TARG_ST
      // [CG] We don't emit stop bit before asm
      if (AS_STOP_BIT && 
	  (EMIT_stop_bits_for_asm ||
	   (EMIT_stop_bits_for_volatile_asm && OP_volatile(op)) ) ) {
	fprintf(Asm_File, "\t%s\n", AS_STOP_BIT);
      }
#endif
#ifdef TARG_ST
      // [SC] We need to emit directives just before the asm string
      CGEMIT_Asm_String_Prefix(op, &PC);
#endif
      fprintf(Asm_File, "\t%s\n", Generate_Asm_String(op, bb));
      if (AS_STOP_BIT && 
	  (EMIT_stop_bits_for_asm ||
	   (EMIT_stop_bits_for_volatile_asm && OP_volatile(op)) ) ) {
	fprintf(Asm_File, "\t%s\n", AS_STOP_BIT);
      }
#ifdef TARG_ST
      // [SC] We need to emit directives just after the asm string
      CGEMIT_Asm_String_Suffix(op, &PC);
#endif
#ifdef TARG_ST200
      // [CG] Align end of asm to 8 bytes boundary such that 
      // for ST200 the alignment is correctly tracked
      // Currently the criterion is DEFAULT_FUNCTION_ALIGNMENT
      if (DEFAULT_FUNCTION_ALIGNMENT) {
	// [CG] Work around the align restriction in ST200 gas
	// .align does not work use balignl instead
	//fprintf(Asm_File, "\t%s %d\n", AS_ALIGN, DEFAULT_FUNCTION_ALIGNMENT);
	fprintf(Asm_File, "\t.balignl %d, 0x80000000\n", DEFAULT_FUNCTION_ALIGNMENT);
	// Align current PC
	PC = PC_Align_N(PC, DEFAULT_FUNCTION_ALIGNMENT);
      }
#endif
    }
    if (Lai_Code) {
      fprintf(Lai_File, "\t%s\n", Generate_Asm_String(op, bb));
    }
    return;
  }

#ifdef TARG_ST
  // Arthur: IA64 does not generate any such OPs so I don't really
  //         know what these are ??
  //         ST100 should only generate TOP_intrncall when generating
  //         the Lai_Code, and in this case we just emit a string
  //         for this intrinsic function much like we do for the
  //         TOP_asm.
  if (OP_code(op) == TOP_intrncall) {
    ISA_BUNDLE bundle[ISA_PACK_MAX_INST_WORDS];
    INT words;

    FmtAssert(Lai_Code,("Assemble_Simulated_OP: shouldn't have generated an TOP_intrncall"));

    ASM_DIR_NOTRANSFORM();
    words = Assemble_OP (op, bb, bundle, 0);
    //    r_assemble_list (op, bb);
    ASM_DIR_TRANSFORM();
    return;
  }

  if (Trace_Inst) {
    fprintf (TFile, "<cgemit> transform simulated OP: ");
    Print_OP (op);
  }

  Exp_Simulated_Op (op, ops, PC, &Symbol_Def_After_Group);

  if (Trace_Inst) {
    fprintf (TFile, "... to: ");
    Print_OPS (ops);
  }

#else
  bool is_intrncall = OP_code(op) == TOP_intrncall;
  OPS ops = OPS_EMPTY;

  if (Trace_Inst) {
    fprintf (TFile, "<cgemit> transform simulated OP: ");
    Print_OP (op);
  }

  Exp_Simulated_Op (op, &ops, PC);

  if (is_intrncall && Object_Code) {
    Em_Add_New_Content (CK_NO_XFORM, PC, OPS_length(&ops)*4, 0, PU_section);
  }

  if (Trace_Inst) {
    fprintf (TFile, "... to: ");
    Print_OPS (&ops);
  }

  if (is_intrncall && Assembly) {
    ASM_DIR_NOTRANSFORM();
  }

  FOR_ALL_OPS_OPs_FWD (&ops, op) {
    ISA_BUNDLE bundle[ISA_PACK_MAX_INST_WORDS];
    INT words;
    Perform_Sanity_Checks_For_OP(op, FALSE);
    words = Assemble_OP (op, bb, bundle, 0);
    if (Object_Code) {
      /* write out the instruction. */
      Em_Add_Bytes_To_Scn (PU_section, (char *)&bundle,
			   ISA_INST_BYTES * words, ISA_INST_BYTES);
    }
  }

  if (is_intrncall && (Assembly || Lai_Code)) {
    ASM_DIR_TRANSFORM();
  }
#endif /* TARG_ST100 */

  return;
}

/* ====================================================================
 *   Assemble_Ops
 *
 *   Assemble the OPs in a BB an OP at a time.
 * ====================================================================
 */
static void
Assemble_Ops (
  BB *bb
)
{
  OP *op;

#ifdef TARG_ST
  Bundle_Count = 0;
  if ((BB_first_op(bb) != NULL) && (OP_scycle(BB_first_op(bb)) != -1))
    Bundle_Count = OP_scycle(BB_first_op(bb));
  Symbol_Def_After_Group = NULL;
#endif

  FmtAssert(ISA_MAX_SLOTS == 1 || Lai_Code || !LOCS_Enable_Bundle_Formation,
	    ("Assemble_Ops shouldn't have been called"));

  FOR_ALL_BB_OPs_FWD(bb, op) {
    ISA_BUNDLE bundle[ISA_PACK_MAX_INST_WORDS];
    INT words;

    if (OP_dummy(op)) continue;		// these don't get emitted

    if (OP_simulated(op)) {
#ifdef TARG_ST
      OPS new_ops = OPS_EMPTY;
      Assemble_Simulated_OP(op, bb, &new_ops);
      BB_Insert_Ops_After (bb, op, &new_ops);
#else
      Assemble_Simulated_OP(op, bb);
#endif
      continue;
    }

    // Perform_Sanity_Checks_For_OP(op, TRUE);
    words = Assemble_OP (op, bb, bundle, 0);
#if 0
    if (Object_Code) {
      Em_Add_Bytes_To_Scn(PU_section, (char *)bundle,
			  ISA_INST_BYTES * words, ISA_INST_BYTES);
    }
#endif
  }

  return;
}

/* ====================================================================
 *   Assemble_Bundles
 *
 *   Assemble the OPs in a BB a bundle at a time.
 * ====================================================================
 */
static void
Assemble_Bundles(BB *bb)
{
  OP *op;

#ifdef TARG_ST
  Bundle_Count = 0;
  if ((BB_first_op(bb) != NULL) && (OP_scycle(BB_first_op(bb)) != -1))
    Bundle_Count = OP_scycle(BB_first_op(bb));
  Symbol_Def_After_Group = NULL;
#endif

  FmtAssert(Assembly,
	    ("Assemble_Bundles shouldn't have been called"));

  FmtAssert(ISA_MAX_SLOTS > 1,
	    ("Assemble_Bundles shouldn't have been called (%d)",
	     ISA_MAX_SLOTS));

  if (Trace_Inst) {
    fprintf(TFile, "<cgemit> Assemble_Bundles BB%d:\n", BB_id(bb));
    //Print_BB(bb);
  }

#ifdef TARG_ST
  // [CL] track prologue and epilogue
  BOOL bundle_has_prologue;
  BOOL bundle_has_epilogue;
  BOOL previous_bundle_has_prologue=FALSE;
  BOOL epilogue_has_started=FALSE;
#endif

#ifdef TARG_ST
  for (op = BB_first_op(bb);op != NULL;) {
#else
  for (op = BB_first_op(bb);;) {
#endif
    ISA_BUNDLE bundle;
#ifdef TARG_ST
    ISA_EXEC_MASK slot_mask;
#else
    UINT64 slot_mask;
#endif
    UINT stop_mask;
    INT slot;
    OP *slot_op[ISA_MAX_SLOTS];
    INT ibundle;
#ifdef TARG_ST200
    // CL: track bundle stop because we have variable length bundles
    int seen_end_group=0;
#endif

#ifdef TARG_ST200  //[CG]
    if (OP_code(op) == TOP_asm) {
      FmtAssert(!Object_Code, ("can't emit asm in object code")); 
      Cg_Dwarf_Add_Line_Entry (PC2Addr(PC), OP_srcpos(op));
      Assemble_Simulated_OP(op, bb, NULL);
      op = OP_next(op);
      continue;
    }
#endif

    /* Gather up the OPs for the bundle.
     */
    stop_mask = 0;
#ifdef TARG_ST
    memset(&slot_mask, 0, sizeof(slot_mask));
#else
    slot_mask = 0;
#endif
#ifdef TARG_ST200
    for (slot = 0; op && !seen_end_group; op = OP_next(op)) {
#else
    for (slot = 0; op && slot < ISA_MAX_SLOTS; op = OP_next(op)) {
#endif
      INT words;
      INT w;

#ifdef TARG_ST200
      seen_end_group = OP_end_group(op);
#endif

      if (OP_dummy(op)) continue;		// these don't get emitted

      if (OP_simulated(op)) {
#ifdef TARG_ST
	OPS new_ops = OPS_EMPTY;
	Assemble_Simulated_OP(op, bb, &new_ops);
	BB_Insert_Ops_After (bb, op, &new_ops);
#ifdef TARG_ST200
	if (OPS_first(&new_ops)) {
	  seen_end_group = FALSE;
	}
#endif
#else
	FmtAssert(slot == 0, ("can't bundle a simulated OP in BB:%d (op %s).",
			      BB_id(bb), TOP_Name(OP_code(op))));
	Assemble_Simulated_OP(op, bb);
#endif
	continue;
      }

      if (OP_prologue(op)) {
	bundle_has_prologue = TRUE;
      } else {
	bundle_has_prologue = FALSE;
      }
      if (OP_epilogue(op)) {
	bundle_has_epilogue = TRUE;
      } else {
	bundle_has_epilogue = FALSE;
      }

      words = ISA_PACK_Inst_Words(OP_code(op));
      for (w = 0; w < words; ++w) {
	FmtAssert(slot < ISA_MAX_SLOTS, 
		  ("multi-word inst extends past end of bundle in BB:%d.",
		   BB_id(bb)));
        slot_op[slot++] = op;
#ifdef TARG_ST
	slot_mask = TI_BUNDLE_Set_Slot_Mask_Property(slot_mask, slot - 1,
						     ISA_EXEC_Unit_Prop(OP_code(op)));
#else
        slot_mask = (slot_mask << ISA_TAG_SHIFT) | ISA_EXEC_Unit_Prop(OP_code(op));
#endif
        stop_mask = stop_mask << 1;
      }
      stop_mask |= (seen_end_group != 0);

#ifndef GAS_TAGS_WORKED
// remove this when gas can handle tags inside explicit bundle
#ifdef TARG_ST
      if (Get_OP_Tag(op)) {
#else
      if (OP_has_tag(op)) {
#endif
	fprintf(Asm_File, "\n%s:\n", LABEL_name(Get_OP_Tag(op)));
      }
#endif

    }
#ifdef TARG_ST
      // clarkes 030910: to allow other ops in a bb after
      // a simulated op
      if (slot == 0) {
	  continue;
      }
#else
    if (slot == 0) break;
#endif

#ifdef TARG_ST200
    // CL: recalibrate to ISA_MAX_SLOTS bundle length
    for(int w=slot; w<ISA_MAX_SLOTS; w++) {
      // [SC] removed. slot_mask <<= ISA_TAG_SHIFT; 
      stop_mask <<= 1;
    }
#endif

#ifndef TARG_ST200
    // CL: now, bundles have variable length

    // Emit the warning only when bundle formation phase is enabled (ON by
    // default).
    if (LOCS_Enable_Bundle_Formation) {
      FmtAssert(slot == ISA_MAX_SLOTS, ("not enough OPs for bundle in BB:%d\n",BB_id(bb)));
    }
#endif

    /* Determine template.
     */
#ifdef TARG_ST
    // [SC] The ops are not guaranteed to be in the correct order
    // in slot[] and slot_mask.  To find the correct bundle
    // we need to try each permutation of the ops.
    // The ops can be permuted between stop bits only.
    // Multi-slot ops must be handled carefully to ensure
    // that they are not split into non-adjacent slots.
    // We also need to take alignment into account to find the
    // correct bundle.
    // That is all very hard to do, and all we achieve is an internal
    // consistency check that the previous bundling was correct.
    // So I give up and fix ibundle here.
    ibundle = 0;
#else
    for (ibundle = 0; ibundle < ISA_MAX_BUNDLES; ++ibundle) {
      UINT64 this_slot_mask = ISA_EXEC_Slot_Mask(ibundle);
      UINT32 this_stop_mask = ISA_EXEC_Stop_Mask(ibundle);
      if (   (slot_mask & this_slot_mask) == this_slot_mask 
	  && stop_mask == this_stop_mask) break;
    }
#endif

    if (Trace_Inst) {
      fprintf(TFile, "<cgemit> Bundle:\n");
      for (int i = 0; i < slot; i++) {
	Print_OP_No_SrcLine (slot_op[i]);
      }
    }

    // Emit the warning only when bundle formation phase is enabled (ON by
    // default).
    if (LOCS_Enable_Bundle_Formation) {
      //      if (Trace_Inst && ibundle == ISA_MAX_BUNDLES) {
      //      if (Trace_Inst) {
      //        fprintf(TFile, "<cgemit> Bundle:\n");
      //        for (slot = 0; slot < ISA_MAX_SLOTS; slot++) {
      //          Print_OP_No_SrcLine (slot_op[slot]);
      //        }
      //      }
#ifdef TARG_ST
      FmtAssert(ibundle != ISA_MAX_BUNDLES,
	       ("couldn't find bundle for slot mask=0x%0llx, stop mask=0x%x in BB:%d\n",
	        slot_mask.v[0], stop_mask, BB_id(bb)));
#else
      FmtAssert(ibundle != ISA_MAX_BUNDLES,
	       ("couldn't find bundle for slot mask=0x%0llx, stop mask=0x%x in BB:%d\n",
	        slot_mask, stop_mask, BB_id(bb)));
#endif
    }

    /* Bundle prefix
     */
    if (Assembly && EMIT_explicit_bundles) {
      fprintf(Asm_File, ISA_PRINT_BEGIN_BUNDLE, ISA_EXEC_AsmName(ibundle));
      fprintf(Asm_File, "\n");
    }


    /* Assemble the bundle.
     */
#ifdef TARG_ST200
    BOOL force_dwarf=FALSE;
    SRCPOS line;

    // [CL] The bundler has already taken care of prologue/epilogue
    // concerns when forming bundles:
    // - we force a new bundle after prologue
    // - we force a new bundle where epilogue begins

    // [CL] If this bundle has epilogue code, and the previous one
    // hadn't, then we consider that epilogue starts here,
    // and thus we force output of debug info for this bundle
    if (bundle_has_epilogue && !epilogue_has_started) {
      force_dwarf = TRUE;
    }

    // [CL] If the prologue ended in the previous bundle,
    // force output of debug info for this bundle.
    if (!bundle_has_prologue && previous_bundle_has_prologue) {
      force_dwarf = TRUE;
    }

    // [CL] Don't output debug info within prologue
    if (bundle_has_prologue) {
      force_dwarf = FALSE;
    }

    line = OP_srcpos(slot_op[0]);

    if ( (!force_dwarf || (line == 0)) && (Opt_Level > 0) ) {
      OP *sl_op;
      slot = 0;
      do {
	sl_op = slot_op[slot];
	if ((OP_srcpos(sl_op) != 0) && (!OP_prologue(sl_op)) && (!OP_epilogue(sl_op))){
	  force_dwarf = TRUE;
	  line = OP_srcpos(sl_op);
	  break;
	}
	slot += ISA_PACK_Inst_Words(OP_code(sl_op));
      } while (!OP_end_group(sl_op));
    }

    /* Generate debug info for the 1st op of the bundle */
    Cg_Dwarf_Add_Line_Entry (PC2Addr(PC), line, force_dwarf);
#endif

    OP *sl_op;
    slot = 0;
    do {
      sl_op = slot_op[slot];
      //      Perform_Sanity_Checks_For_OP(sl_op, TRUE);
      slot += Assemble_OP(sl_op, bb, &bundle, slot);
#ifdef TARG_ST200
    } while (!OP_end_group(sl_op));
#else
    } while (slot < ISA_MAX_SLOTS);
#endif

    /* Bundle suffix
     */
#if 0
    if (Object_Code) {
      TI_ASM_Set_Bundle_Comp(&bundle,
			     ISA_BUNDLE_PACK_COMP_template, 
			     ibundle);

      Em_Add_Bytes_To_Scn (PU_section, (char *)&bundle, 
                                        ISA_INST_BYTES, ISA_INST_BYTES);
    }
#endif
    if (Assembly) {
      if (EMIT_explicit_bundles) {
	fprintf(Asm_File, "%s", ISA_PRINT_END_BUNDLE);
#ifdef TARG_ST
	fprintf(Asm_File, "\n");
      }
#else
      }
      fprintf(Asm_File, "\n");
#endif
    }

#ifdef TARG_ST
    // [CL]
    previous_bundle_has_prologue = bundle_has_prologue;
    if (bundle_has_epilogue) {
      epilogue_has_started = TRUE;
    }
#endif
  }
#ifndef TARG_ST
  if (Assembly) {
    fprintf(Asm_File, "\n");
  }
#endif
}

/* ====================================================================
 *   Emit_Loop_Note
 *
 *   Emit a loop note to the .s file, anl file, etc.
 * ====================================================================
 */
static void
Emit_Loop_Note (
  BB *bb, 
  FILE *file
)
{
  //  BOOL anl_note = file == anl_file;
  BB *head = BB_loop_head_bb(bb);
  UINT16 unrollings = BB_unrollings(bb);
  ANNOTATION *info_ant = ANNOT_Get(BB_annotations(head), ANNOT_LOOPINFO);
  LOOPINFO *info = info_ant ? ANNOT_loopinfo(info_ant) : NULL;
  BOOL unroll_pragma = FALSE;
  ANNOTATION *unroll_ant = ANNOT_Get(BB_annotations(head), ANNOT_PRAGMA);

  while (unroll_ant && WN_pragma(ANNOT_pragma(unroll_ant)) != WN_PRAGMA_UNROLL)
    unroll_ant = ANNOT_Get(ANNOT_next(unroll_ant), ANNOT_PRAGMA);
  if (unroll_ant) {
    WN *wn = ANNOT_pragma(unroll_ant);
    if (WN_pragma_arg1(wn) > 1) {
      if (WN_pragma_arg1(wn) == unrollings)
	unroll_pragma = TRUE;
#ifdef TARG_ST
      else if (BB_innermost(bb) && (bb == head))
#else
      else if (BB_innermost(bb))
#endif
	DevWarn("BB:%d unrolled %d times but pragma says to unroll %d times",
		BB_id(bb), unrollings, WN_pragma_arg1(wn));
    }
  }

  if (bb == head) {
    SRCPOS srcpos = BB_Loop_Srcpos(bb);
    INT32 lineno = SRCPOS_linenum(srcpos);

#if 0
    if (anl_note) {
      INT32 fileno = SRCPOS_filenum(srcpos);
      INT32 colno = SRCPOS_column(srcpos);
      fprintf (anl_file,
	       "\nmsg loop lines [%d %d %d]",
	       fileno,
	       lineno,
	       colno);
    } else {
      fprintf (file, "%s<loop> Loop body line %d", ASM_CMNT_LINE, lineno);
    }
#else
    fprintf (file, "%s<loop> Loop body line %d", ASM_CMNT_LINE, lineno);
#endif

    if (info) {
      WN *wn = LOOPINFO_wn(info);
      TN *trip_tn = LOOPINFO_trip_count_tn(info);
      BOOL constant_trip = trip_tn && TN_is_constant(trip_tn);
      INT depth = WN_loop_depth(wn);
      const char *estimated = constant_trip ? "" : "estimated ";
      INT64 trip_count = constant_trip ? TN_value(trip_tn) :
					 (INT64)WN_loop_trip_est(wn);
#if 0
      const char *fmt =   anl_note
			? " \"nesting depth: %d, %siterations: %lld\""
			: ", nesting depth: %d, %siterations: %lld";
#else
      const char *fmt = ", nesting depth: %d, %siterations: %lld";
#endif
      fprintf (file, fmt, depth, estimated, trip_count);
    }

    fprintf (file, "\n");
  } 
#if 0
  else if (anl_note) {

    /* Only interested in loop head messages for anl file
     */
    return;
  } 
#endif
  else {
    ANNOTATION *lbl_ant = ANNOT_Get(BB_annotations(head), ANNOT_LABEL);
    DevAssert(lbl_ant, ("loop head BB:%d has no label", BB_id(head)));
    fprintf(file,
	    "%s<loop> Part of loop body line %d"
	    ", head labeled %s\n",
	    ASM_CMNT_LINE, BB_Loop_Lineno(head), LABEL_name(ANNOT_label(lbl_ant)));
  }

  if (unrollings > 1) {
#if 0
    if (anl_note) {
      fprintf(anl_file, "\"unrolled %d times%s%s\"\n", unrollings,
	      BB_unrolled_fully(bb) ? " (fully)" : "",
	      unroll_pragma ? " (pragma)" : "");
    } else {
      fprintf(file, "%s<loop> unrolled %d times%s%s\n", 
	      ASM_CMNT_LINE,
	      unrollings,
	      BB_unrolled_fully(bb) ? " (fully)" : "",
	      unroll_pragma ? " (pragma)" : "");
    }
#else
      fprintf(file, "%s<loop> unrolled %d times%s%s\n", 
	      ASM_CMNT_LINE,
	      unrollings,
	      BB_unrolled_fully(bb) ? " (fully)" : "",
	      unroll_pragma ? " (pragma)" : "");
#endif
  }
}

#ifdef BCO_ENABLED /* Thierry */
static BB_NUM PU_BB_Count_for_profile;	// Number of BB with profile info
static BB_NUM PU_BB_current_idx_for_profile = 0; /* Current index of BB being treated */ 

/* Add profil_info commands */
#define PI_FUNCBEGIN 0x01 
#define PI_FUNCEND 0x02 


/* ====================================================================
 *   get_char_from_float
 *
 * Convert a float into 4 bytes for a little endian target 
 * Use for .profile_info whose data is little endian
 *
 * ====================================================================
 */
static void 
get_char_from_float(float *f, char *ptr)
{
  union u {
    float f;
    char b[4];
  } p;
  p.f = *f;
#if HOST_IS_LITTLE_ENDIAN
  *((union u *)ptr) = p;
#elif HOST_IS_BIG_ENDIAN
  ptr[0] = p.b[3];
  ptr[1] = p.b[2];
  ptr[2] = p.b[1];
  ptr[3] = p.b[0];
#else
#error "HOST_IS_LITTLE_ENDIAN or HOST_IS_BIG_ENDIAN need to be defined"
#endif  
}

/* ====================================================================
 *   EMT_ProfileInfo_BB
 *
 *   Emit .profile_info for this BB.
 *   Emit frequency info about BB.
 *
 * ====================================================================
 */
static void
EMT_ProfileInfo_BB ( 
  BB *bb, 
  WN *rwn 
)
{
  char * label_name;
  ANNOTATION *ant;
  LABEL_IDX lab;
  FILE *file = Asm_File;
  char buf[4];
  float f;
  const char *prefix = file == Asm_File ? ASM_CMNT_LINE : "";

  if (!FREQ_Frequencies_Computed() && !CG_PU_Has_Feedback) return;
  FmtAssert(Assembly && CG_emit_bb_freqs, ("EMT_ProfileInfo_BB calls only with -CG:emit-bb-freqs"));

  /* first get a label attached to this BB */

  ant = ANNOT_First (BB_annotations(bb), ANNOT_LABEL);
  assert(ant != NULL);
  lab = ANNOT_label(ant);
  label_name = LABEL_name(lab);
    
  PU_BB_current_idx_for_profile++;
  FmtAssert( PU_BB_current_idx_for_profile <= PU_BB_Count_for_profile, ("Current BB index is over BB count!"));
    
  /* Convert BB frequency into 4 bytes */
  f = BB_freq(bb);
  get_char_from_float(&f, buf);

  /* Dump profile info */
  fprintf(file, "\t%s\t%s, 0x%x\n", AS_WORD,
	  label_name, PU_BB_current_idx_for_profile);
  
  fprintf(file, "\t%s\t0x%x, 0x%x, 0x%x, 0x%x\n", AS_BYTE,
	  buf[0], buf[1], buf[2], buf[3]);
}

/* ====================================================================
 *   EMT_ProfileInfo_Header_PU
 *
 *   Emit .profile_info header for this PU.
 * ====================================================================
 */
static void
EMT_ProfileInfo_PU ( 
  ST      *pu,
  DST_IDX  pu_dst, 
  WN      *rwn
)
{
  BB *bb; 
  
  if (!FREQ_Frequencies_Computed() && !CG_PU_Has_Feedback) return;
  FmtAssert( Assembly && CG_emit_bb_freqs, ("EMT_ProfileInfo_Header_PU calls only with -CG:emit-bb-freqs"));
  /* Emit header of .profile_info for this PU */
  // HEADER
  /* Reset current BB index to 0 */
  PU_BB_current_idx_for_profile = 0;
  
  /* compute PU_BB_Count because PU_BB_Count is wrong here??? */
  PU_BB_Count_for_profile = 0;
  for (bb = REGION_First_BB; bb != NULL; bb = BB_next(bb)) {
    PU_BB_Count_for_profile++;
  }
  fprintf (Asm_File, "\t%s %s\n", AS_SECTION, ".profile_info");
  fprintf (Asm_File, "\t%s\t0x%x, %s, 0x%x \n",AS_WORD,
	   PI_FUNCBEGIN, ST_name(pu), PU_BB_Count_for_profile);
  // BBS
  /* Emit profile_info for each basic block in the PU */
  for (bb = REGION_First_BB; bb != NULL; bb = BB_next(bb)) {
    EMT_ProfileInfo_BB (bb, rwn);
  }

  // FOOTER
  fprintf(Asm_File, "\t%s\t0x%x\n", AS_WORD, PI_FUNCEND);

}
#endif /* BCO_Enabled Thierry */

/* ====================================================================
 *   EMT_Assemble_BB
 *
 *   Assemble the contents of the given BB.
 * ====================================================================
 */
static void
EMT_Assemble_BB ( 
  BB *bb, 
  WN *rwn 
)
{
  ST *st;
  ANNOTATION *ant;
  RID *rid = BB_rid(bb);

  if (Trace_Inst) {
    #pragma mips_frequency_hint NEVER
    fprintf(TFile, "assemble BB %d\n", BB_id(bb));
  }

  if (Assembly || Lai_Code) {
    if (rid != NULL && RID_cginfo(rid) != NULL) {
      if (current_rid == RID_id(rid)) {
	/* same, so do nothing */
      }
      else if (current_rid > 0) {
	/* changing regions */
	fprintf (Output_File, "%s END REGION %d\n",
				    ASM_CMNT_LINE, current_rid);
	if (RID_id(rid) > 0 && !RID_is_glue_code(rid)) {
	  fprintf (Output_File, "%s BEGIN REGION %d\n",
				    ASM_CMNT_LINE, RID_id(rid));
	  current_rid = RID_id(rid);
	}
	else if (RID_is_glue_code(rid)) {
	  current_rid = 0;	/* pretend not a region */
	}
	else {
	  current_rid = RID_id(rid);
	}
      }
      else if (RID_id(rid) > 0 && !RID_is_glue_code(rid)) {
	/* beginning a region */
	fprintf (Output_File, "%s BEGIN REGION %d\n", 
				   ASM_CMNT_LINE, RID_id(rid));
	current_rid = RID_id(rid);
      }
    }
    else if (current_rid > 0) {
      /* ending a region */
      fprintf (Output_File, "%s END REGION %d\n", ASM_CMNT_LINE, current_rid);
      current_rid = 0;	/* no rid */
    }
  }

#ifdef TARG_ST
  INT align_bb = Check_If_Should_Align_BB(bb);
  if (align_bb != 0) {
    fprintf(Asm_File, "\t%s %d\n", AS_ALIGN, align_bb);
    // Align current PC
    PC = PC_Align_N(PC, align_bb);
  }
#endif

  if (BB_entry(bb)) {
    char *entry_name;
    ST *entry_sym; 
    ENTRYINFO *ent;
    SRCPOS entry_srcpos;
    ant = ANNOT_Get (BB_annotations(bb), ANNOT_ENTRYINFO);
    ent = ANNOT_entryinfo(ant);
    entry_srcpos = ENTRYINFO_srcpos(ent);
    entry_sym = ENTRYINFO_name(ent);
    entry_name = ST_name(entry_sym);

    /* Set an initial line number so that if the first inst in the BB
     * has no srcpos, then we'll be ok.
     */
#ifdef TARG_ST
    // [CL] force generation of line information for entry_BB
    // useful for C++ when several functions are generated
    // in sequence for the same line number (generated constructors
    // for instance)
    if (entry_srcpos)
      Cg_Dwarf_Add_Line_Entry (PC2Addr(PC), entry_srcpos, TRUE);
#else
    if (entry_srcpos)
      Cg_Dwarf_Add_Line_Entry (PC, entry_srcpos);
#endif

    if (ST_is_not_used(entry_sym)) {
      // don't emit alt-entry if marked as not-used
      DevWarn("CG reached entry marked as unused; will ignore it (%s)\n", 
		entry_name);
    }
    else {
      Set_ST_ofst(entry_sym, PC);
      if (strcmp(Cur_PU_Name, entry_name) != 0) {
	// alt-entry
	if (Assembly || Lai_Code) {
	  fprintf (Output_File, "\t%s\t%s\n", AS_AENT, entry_name);
	  Print_Label (Output_File, entry_sym, 0 );
	}
	(void)EMT_Put_Elf_Symbol (entry_sym);
#if 0
	if ( Object_Code ) {
	  Em_Define_Symbol (
	       EMT_Put_Elf_Symbol(entry_sym), PC, 0, PU_section);
	  Em_Add_New_Event (EK_ENTRY, PC, 0, 0, 0, PU_section);
	}
#endif
      }
#if 0
      if (Object_Code) {
	if ( EMIT_interface_section && !BB_handler(bb))
	  Interface_Scn_Add_Def( entry_sym, rwn );
      }
#endif
    }
  }

  /* List labels attached to BB: */
  for (ant = ANNOT_First (BB_annotations(bb), ANNOT_LABEL);
       ant != NULL;
       ant = ANNOT_Next (ant, ANNOT_LABEL)) {
    LABEL_IDX lab = ANNOT_label(ant);
    if (Assembly || Lai_Code) {
#ifdef TARG_ST
      if (List_Notes)
	fprintf (Output_File, "%s:\t%s 0x%llx\n", 
		 LABEL_name(lab), ASM_CMNT, Get_Label_Offset(lab) );
      else
	fprintf (Output_File, "%s:\n", LABEL_name(lab));
#else
      fprintf (Output_File, "%s:\t%s 0x%llx\n", 
		   LABEL_name(lab), ASM_CMNT, Get_Label_Offset(lab) );
#endif
    }
#ifndef TARG_IA64

    // Arthur: this is probably just the MIPS thing ??
#ifdef TARG_MIPS
    if (Get_Label_Offset(lab) != PC) {
	DevWarn ("label %s offset %lld doesn't match PC %d", 
		LABEL_name(lab), Get_Label_Offset(lab), PC);
    }
#endif
#endif
  }

  // hack to keep track of last label and offset for assembly dwarf (suneel)
  if (Last_Label == LABEL_IDX_ZERO) {
    Last_Label = Gen_Label_For_BB (bb);
    Offset_From_Last_Label = 0;
    if (Initial_Pu_Label == LABEL_IDX_ZERO) {
      Initial_Pu_Label = Last_Label;
    }
  }

  st = BB_st(bb);
  if (st) {
    if (Assembly || Lai_Code) {
#ifdef TARG_ST
      if (List_Notes) {
	fprintf (Output_File, "%s:\t%s 0x%llx\n", ST_name(st), ASM_CMNT, ST_ofst(st));
      } else {
	fprintf (Output_File, "%s:\n", ST_name(st));
      }
#else
      fprintf (Output_File, "%s:\t%s 0x%llx\n", ST_name(st), ASM_CMNT, ST_ofst(st));
#endif
    }

    Is_True (ST_ofst(st) == PC, ("st %s offset %lld doesn't match PC %d", 
	ST_name(st), ST_ofst(st), PC));
    Is_True (!Has_Base_Block(st) 
	     || (ST_base(st) == (BB_cold(bb) ? cold_base : text_base)),
	     ("sym %s base doesn't match BB:%d",
	     ST_name(st), BB_id(bb)));
    FmtAssert(ST_is_export_local(st),
	      ("ST for BB:%d not EXPORT_LOCAL", BB_id(bb)));
  }

  /* write out all the notes for this BB */
  if (Assembly && List_Notes) {
    if (BB_loop_head_bb(bb)) {
      Emit_Loop_Note(bb, Asm_File);
    }
#if 0
    if (BB_annotations(bb) && 
	ANNOT_Get(BB_annotations(bb), ANNOT_ROTATING_KERNEL))
      Emit_SWP_Note(bb, Asm_File);
#endif
    if (BB_has_note(bb)) {
      NOTE_BB_Act (bb, NOTE_PRINT_TO_FILE, Asm_File);
    }

    FREQ_Print_BB_Note(bb, Asm_File);
  }
  if (Lai_Code && List_Notes) {
    if (BB_loop_head_bb(bb)) {
      Emit_Loop_Note(bb, Lai_File);
    }
#if 0
    if (BB_annotations(bb) && 
	ANNOT_Get(BB_annotations(bb), ANNOT_ROTATING_KERNEL))
      Emit_SWP_Note(bb, Lai_File);
#endif
    if (BB_has_note(bb)) {
      NOTE_BB_Act (bb, NOTE_PRINT_TO_FILE, Lai_File);
    }

    FREQ_Print_BB_Note(bb, Lai_File);
  }

#if 0
  if (Run_prompf) {
    if (BB_loop_head_bb(bb)) {
      Emit_Loop_Note(bb, anl_file);
    }

    if (BB_has_note(bb)) {
      NOTE_BB_Act (bb, NOTE_PRINT_TO_ANL_FILE, anl_file);
    }
  }
#endif

#if Is_True_On
  /*  Init_Sanity_Checking_For_BB (); */
#endif

  if ((ISA_MAX_SLOTS > 1) && LOCS_Enable_Bundle_Formation) {
    Assemble_Bundles(bb);
  } else {
    Assemble_Ops(bb);
  }

#if 0
  if (Object_Code && BB_exit(bb)) {
    Em_Add_New_Event (EK_EXIT, PC - 2*ISA_INST_BYTES, 0, 0, 0, PU_section);
  }
#endif

  return;
}

/* ====================================================================
 *   Recompute_Label_Offset
 * ====================================================================
 */
static void
Recompute_Label_Offset (
  INT32 pcs[2]
)
{
  BB *bb;
  INT32 cur_pcs[2];

  /* recompute the addresses of all labels in the procedure. */
  cur_pcs[IHOT] = text_PC;
  cur_pcs[ICOLD] = cold_PC;
  for (bb = REGION_First_BB; bb != NULL; bb = BB_next(bb)) {
    ANNOTATION *ant;
    OP *op;
    INT isect = BB_cold(bb) ? ICOLD : IHOT;
    INT32 cur_pc = cur_pcs[isect];
    LABEL_IDX lab;
    ST *st = BB_st(bb);
    if (st) {
      Set_ST_ofst(st, cur_pc);
      Set_ST_base(st, BB_cold(bb) ? cold_base : text_base);
    }
    for (ant = ANNOT_First (BB_annotations(bb), ANNOT_LABEL);
         ant != NULL;
         ant = ANNOT_Next (ant, ANNOT_LABEL))
    {
      lab = ANNOT_label(ant);
      Set_Label_Offset(lab, cur_pc);
    }
    for (op = BB_first_op(bb); op; op = OP_next(op)) {
#ifdef TARG_ST
      lab = Get_OP_Tag(op);
      if (lab) {
#else
      if (OP_has_tag(op)) {
	lab = Get_OP_Tag(op);
#endif
      	Set_Label_Offset(lab, cur_pc);
      }
      INT num_inst_words = OP_Real_Inst_Words (op);
      cur_pc = PC_Incr_N(cur_pc, num_inst_words);
    }
    cur_pcs[isect] = cur_pc;
  }

  Is_True(cur_pcs[IHOT] == pcs[IHOT],
	 ("Recompute_Label_Offsets: hot region PC mismatch (%d vs %d).",
	  cur_pcs[IHOT], pcs[IHOT]));
  Is_True(cur_pcs[ICOLD] == pcs[ICOLD],
	 ("Recompute_Label_Offsets: cold region PC mismatch (%d vs %d).",
	  cur_pcs[ICOLD], pcs[ICOLD]));
}

/* ====================================================================
 *   Fixup_Long_Branches
 * ====================================================================
 */
static void
Fixup_Long_Branches (
  INT32 *hot_size, 
  INT32 *cold_size
)
{
  FmtAssert(FALSE,
	    ("Fixup_Long_Branches: not implemented"));
}

/* ====================================================================
 *   Pad_BB_With_Noops
 *
 *   When we have bundles, 'num' is the number of bundles, not
 *   insructions.
 * ====================================================================
 */
static void 
Pad_BB_With_Noops (
  BB *bb, 
  INT num
)
{
  OP *new_op;
  OPS new_ops = OPS_EMPTY;

  if (ISA_MAX_SLOTS > 1) {
    INT ibundle;
#ifdef TARG_ST
    ISA_EXEC_MASK slot_mask;
#else
    UINT64 slot_mask;
#endif

    /* Choose a bundle for the nops. For now we just pick the first
     * bundle without a stop bit. We could chose more smartly based
     * on the previous contents of the BB.
     */
    for (ibundle = 0; ibundle < ISA_MAX_BUNDLES; ++ibundle) {
      UINT32 stop_mask = ISA_EXEC_Stop_Mask(ibundle);
      slot_mask = ISA_EXEC_Slot_Mask(ibundle);
      if (stop_mask == 0) break;
    }

    do {
      INT slot = ISA_MAX_SLOTS - 1;
      do {
	ISA_EXEC_UNIT_PROPERTY unit;
#ifdef TARG_ST
	unit = TI_BUNDLE_Get_Slot_Mask_Property (slot_mask, slot);
#else
	unit =  (ISA_EXEC_UNIT_PROPERTY)(
		  (slot_mask >> (ISA_TAG_SHIFT * slot)) 
		& ((1 << ISA_TAG_SHIFT) - 1));
#endif
	Exp_Noop(&new_ops);
	new_op = OPS_last(&new_ops);
	OP_scycle(new_op) = OP_scycle(BB_last_op(bb));
	OP_Change_Opcode(new_op, CGTARG_Noop_Top(unit));
	BB_Append_Op (bb, new_op);
	slot -= OP_Real_Inst_Words(new_op);
      } while (slot >= 0);
    } while (--num);
  } else {
    do {
      Exp_Noop(&new_ops);
      new_op = OPS_last(&new_ops);
      OP_scycle(new_op) = OP_scycle(BB_last_op(bb));
      BB_Append_Op (bb, new_op);
    } while (--num);
  }
}

/* ====================================================================
 *   R_Resolve_Branches
 *
 *   Assign addresses to all local labels. Fixup long branches.
 * ====================================================================
 */
static void
R_Resolve_Branches (
  ST *pu_st
)
{
  BB *bb;
  BB *prev_nonempty_bbs[2] = { NULL, NULL };
  INT32 curpcs[2];
  INT32 hot_size, cold_size;

  /* check for T5 workarounds */
  Hardware_Workarounds();

  curpcs[IHOT] = text_PC;
  curpcs[ICOLD] = cold_PC;
  for (bb = REGION_First_BB; bb; bb = BB_next(bb)) {
    INT32 bb_start_pc;
    OP *op;
    INT32 isect = BB_cold(bb) ? ICOLD : IHOT;
    INT32 curpc = curpcs[isect];
    BB *prev_nonempty_bb = prev_nonempty_bbs[isect];

#ifndef TARG_ST
    // [CG] Removed this code. Alignment of BB is handled in EMT_Emit_BB.
    /* need prev bb to align */
    if (prev_nonempty_bb != NULL) {
      INT32 num = Check_If_Should_Align_BB (bb, curpc);
      if (num != 0) {
	if (Trace_Inst) {
	  #pragma mips_frequency_hint NEVER
	  fprintf(TFile, "insert %d noops at bb %d\n", num, BB_id(bb));
	}
	// increment curpc by 'num' bundles
	curpc += ISA_INST_BYTES * num;
	Pad_BB_With_Noops(prev_nonempty_bb, num);
      }
    }
#endif

    bb_start_pc = curpc;

    /* If there is no label, make one: */
    Gen_Label_For_BB ( bb );

    for (op = BB_first_op(bb); op; op = OP_next(op)) {
      INT num_inst_words = OP_Real_Inst_Words (op);
      curpc = PC_Incr_N(curpc, num_inst_words);
    }
    
    if (curpc != bb_start_pc) prev_nonempty_bbs[isect] = bb;

    curpcs[isect] = curpc;
  }

  Recompute_Label_Offset(curpcs);

  hot_size = curpcs[IHOT] - text_PC;
  cold_size = curpcs[ICOLD] - cold_PC;

  // if large text size or has branch predicts (which are limited in size),
  // then check for long branches.
  if (hot_size >= MIN_BRANCH_DISP || cold_size >= MIN_BRANCH_DISP
    || CGTARG_Has_Branch_Predict()) 
  {
    Fixup_Long_Branches (&hot_size, &cold_size);
  }

  if (generate_elf_symbols) {
    ;
  }

  if (Trace_Inst) {
    #pragma mips_frequency_hint NEVER
    fprintf (TFile, "INSTS IN PROCEDURE: %d\n", hot_size + cold_size);
  }
}

/* ====================================================================
 *    emit_global_symbols
 *
 *    initialize any new global sections.
 *    We don't do this in EMT_Begin_File because may not have done
 *    data layout at that point.
 * ====================================================================
 */
static void
emit_global_symbols ()
{
  INT i;
  static UINT last_global_index = 1;

  for (i = last_global_index; i < ST_Table_Size(GLOBAL_SYMTAB); ++i) {
    ST* sym = &St_Table(GLOBAL_SYMTAB,i);

    if (ST_class(sym) == CLASS_BLOCK && STB_section(sym)) {
      Init_Section(sym);
    }

    // emit commons here so order is preserved for datapools
    if (ST_sclass(sym) == SCLASS_COMMON) {
      if (ST_is_not_used (sym)) continue;
      (void)EMT_Put_Elf_Symbol (sym);
    }
  }
  last_global_index = ST_Table_Size(GLOBAL_SYMTAB);

  if (Assembly)
    fprintf(Asm_File,"\n");
  if (Lai_Code)
    fprintf(Lai_File,"\n");

  return;
}

/* ====================================================================
 *    emit_function_prototypes
 * ====================================================================
 */
static void
emit_function_prototypes ()
{
  INT i, j;
  static UINT last_global_index = 1;

  for (i = last_global_index; i < ST_Table_Size(GLOBAL_SYMTAB); ++i) {
    ST* sym = &St_Table(GLOBAL_SYMTAB,i);

    if (ST_is_not_used(sym))
      continue;

    if (ST_class(sym) == CLASS_FUNC) {
      ISA_REGISTER_CLASS rc;

      TY_IDX prototype = PU_prototype(Pu_Table[ST_pu(sym)]);

      if (!TY_has_prototype(prototype)) continue;

      FmtAssert(prototype != 0,
		("emit_function_prototypes: empty function prototype"));

      fprintf(Lai_File,"\t.proto\t%s", ST_name(sym));

      // parameters:
      TYLIST_ITER fn_iter = Make_tylist_iter(TY_parms(prototype));
      while (*fn_iter) {
	rc = Register_Class_For_Mtype(TY_mtype(Ty_Table[*fn_iter]));
	fprintf(Lai_File,", %s", ISA_REGISTER_CLASS_ASM_Name (rc));
	*fn_iter++;
      }
      if (TY_is_varargs(prototype))
	fprintf(Lai_File,", ...");


      fprintf(Lai_File," => "); //, ST_name(sym));
      // return info:
      RETURN_INFO return_info = Get_Return_Info(TY_ret_type(prototype),
                                              Complex_Not_Simulated);
      for (j = 0; j < RETURN_INFO_count(return_info)-1; j++) {
	rc = Register_Class_For_Mtype(RETURN_INFO_mtype (return_info, j));
	fprintf(Lai_File,"%s, ", ISA_REGISTER_CLASS_ASM_Name (rc));
      }

      if (RETURN_INFO_count(return_info)) {
	rc = Register_Class_For_Mtype(RETURN_INFO_mtype (return_info, 
                                      RETURN_INFO_count(return_info)-1));
	fprintf(Lai_File,"%s\n", ISA_REGISTER_CLASS_ASM_Name (rc));
      }
      else {
	fprintf(Lai_File,"void\n");
      }
    }
  }
  last_global_index = ST_Table_Size(GLOBAL_SYMTAB);

  return;
}

/* ====================================================================
 *    emit_input_directives
 * ====================================================================
 */
static void
emit_input_directives ()
{
  INT    i;
  ST    *sym;
  ST    *base;
  INT64  ofst;

  FOREACH_SYMBOL (CURRENT_SYMTAB, sym, i) {
    if (ST_is_not_used(sym)) continue;
    if (ST_sclass(sym) == SCLASS_FORMAL) {
      Base_Symbol_And_Offset (sym, &base, &ofst);
      /*
      base = ST_base(sym);
      ofst = ST_ofst(sym);
      */

      fprintf (Lai_File, "\t%s\t.input %s at %s, %lld\n", 
                   ASM_CMNT, ST_name(sym), ST_name(base), ofst);
    }
  }

  return;
}

/* ====================================================================
 *    emit_output_directives
 * ====================================================================
 */
static void
emit_output_directives ()
{
  BB *bb;
  OP *op;

  for (bb = REGION_First_BB; bb != NULL; bb = BB_next(bb)) {
    if (!BB_exit(bb)) continue;
    FOR_ALL_BB_OPs_FWD(bb, op) {
	TN *tn = OP_opnd(op, 0);
	ISA_REGISTER_CLASS rc = TN_register_class(tn);
	fprintf(Lai_File, "\t.output %s%d\t\t\t\t%s\n",
             ISA_REGISTER_CLASS_Symbol(rc), TN_number(tn), ASM_CMNT);
    }
  }

  return;
}

/* ====================================================================
 *    emit_virtual_registers
 *
 *    Check the TN_Vec[] to see what TNs exist.
 * ====================================================================
 */
static void
emit_virtual_registers ()
{
  INT i, j;
  TN *tn;
  ISA_REGISTER_CLASS rc;
  INT tct[ISA_REGISTER_CLASS_MAX+1];
  TN *tns[ISA_REGISTER_CLASS_MAX+1][Last_TN - First_Regular_TN + 1];

  if (Trace_Inst) {
    Print_TNs ();
  }

  FOR_ALL_ISA_REGISTER_CLASS(rc) {
    tct[rc] = 0;
  }

  for (i = First_Regular_TN; i <= Last_TN; i++) {
    tn = TNvec(i);

    if (TN_is_register(tn)) {
      rc = TN_register_class(tn);
      tns[rc][tct[rc]] = tn;
      tct[rc] += 1;
    }
  }

  FOR_ALL_ISA_REGISTER_CLASS(rc) {

    if (tct[rc] == 0) continue;

    //    fprintf(Lai_File, "\t.%s\t", ISA_REGISTER_CLASS_ASM_Name (rc));
    for (j = 0; j < tct[rc]; j++) {
      tn = tns[rc][j];

      if (j % 6 == 0)
	fprintf(Lai_File, "\n\t.%s\t %s%d",
           ISA_REGISTER_CLASS_ASM_Name (rc),
		ISA_REGISTER_CLASS_Symbol(rc), TN_number(tn));
      else
	fprintf(Lai_File, ", %s%d",
                     ISA_REGISTER_CLASS_Symbol(rc), TN_number(tn));
    }
    fprintf(Lai_File, "\n");
    //    tn = tns[rc][tct[rc]-1];
    //    fprintf(Lai_File, "%s%d \n",
    //                     ISA_REGISTER_CLASS_Symbol(rc), TN_number(tn));
  }

  return;
}

/* ====================================================================
 *    EMT_Begin_File
 *
 *    TODO: unify with making the Object and Assembly !
 * ====================================================================
 */
void
EMT_Begin_File (
  char *process_name,	/* Back end process name */
  char *options)	/* Option string used for compilation */
{
  char *buff;

  /* Initialize: */
  Trace_Elf	= Get_Trace ( TP_EMIT, 2 );
  Trace_Init	= Get_Trace ( TP_EMIT, 4 );
#ifdef TARG_ST
  Trace_Sched   = Get_Trace ( TP_SCHED, 2 );
#endif

  // Which one to write into ?
  if (Assembly) Output_File = Asm_File;
  if (Lai_Code) Output_File = Lai_File;

#if 0
  // Enumerate all instructions:
  if (Get_Trace (TP_CG, 0x100)) Enumerate_Insts();
#endif

  text_PC = 0;
  cold_PC = 0;
  Get_Ism_Name();
  if (FILE_INFO_gp_group (File_info)) {
        Has_GP_Groups = TRUE;
  }

  if (Object_Code || CG_emit_asm_dwarf) {
	generate_dwarf = TRUE;
	generate_elf_symbols = TRUE;
  }

  if ( generate_elf_symbols ) {
#ifdef TARG_ST
    // [CL] when generating .s file, do not
    // overwrite/remove existing .o file
    if ( ! Object_Code) {
      Obj_File_Name = tempnam(NULL, "cctpo");
    }
#endif
 
    Obj_File = fdopen (Em_Begin_File (
			    	Obj_File_Name, 
			    	FALSE, 
			    	!Use_32_Bit_Pointers, 
				FALSE /* old_abi */,
				(INT) Target_ISA,
				(Target_Byte_Sex == BIG_ENDIAN),
				Gen_PIC_Shared, 
				Gen_PIC_Call_Shared,
                		!Guaranteed_Small_GOT, 
				Has_GP_Groups,
			    	Trace_Elf),
		       "r+");
    if (Obj_File == NULL) return;

    // If we are meant to be generating only a .s file but that .s
    // file needs to have dwarf information represented in it, we
    // currently use the em_elf and libelf routines to maintain
    // section indices, symbol indices, etc. for libdwarf. In such a
    // situation, we generate the object file, but we unlink it here
    // so it never shows up after the compilation is done.
    if ( ! Object_Code) {
      unlink(Obj_File_Name);
    }

    buff = (char *) alloca (strlen("be") + sizeof(INCLUDE_STAMP) + 
			    strlen(ism_name) + strlen(Obj_File_Name) + 4);
    if (*ism_name != '\0')
	sprintf(buff, "be::%s-%s:%s", INCLUDE_STAMP, ism_name, Obj_File_Name);
    else
	sprintf(buff, "be::%s:%s", INCLUDE_STAMP, Obj_File_Name);
    Em_Add_Comment (buff);
    if ( ! DEBUG_Optimize_Space) {
    	buff = (char *) alloca (strlen("be-options") + strlen(options) + 4);
    	sprintf(buff, "be-options:::%s", options);
    	Em_Add_Comment (buff);
    }

#if 0
    if ( EMIT_interface_section ) Interface_Scn_Begin_File();
#endif
  }

  Init_ST_elf_index(GLOBAL_SYMTAB);
  if (generate_dwarf) {
    Cg_Dwarf_Begin (!Use_32_Bit_Pointers);
  }
  Cg_Dwarf_Gen_Asm_File_Table ();

  if (Assembly) {
    ASM_DIR_NOREORDER();
    ASM_DIR_NOAT();
    fprintf ( Asm_File, "\t%s  %s::%s\n", ASM_CMNT, process_name, 
			    INCLUDE_STAMP );
    if (*ism_name != '\0')
    	fprintf ( Asm_File, "\t%s%s\t%s\n", ASM_CMNT, "ism", ism_name);
    List_Compile_Options ( Asm_File, "\t"ASM_CMNT, FALSE, TRUE, TRUE );
    /* TODO: do we need to do .interface stuff for asm files? */
  }

  if (Lai_Code) {
    // These are for the .lai processing:
    //    LAI_Begin_File (process_name, options);


    fprintf (Lai_File, "\t%s  %s::%s\n", ASM_CMNT, process_name, 
			    INCLUDE_STAMP);
    if (*ism_name != '\0')
      fprintf (Lai_File, "\t%s%s\t%s\n", ASM_CMNT, "ism", ism_name);

    List_Compile_Options (Lai_File, "\t"ASM_CMNT, FALSE, TRUE, TRUE);
  }

  //  Init_Tcon_Info ();

#ifdef TARG_ST
  // Target dependent begin file.
  if (Assembly) CGEMIT_Begin_File_In_Asm ();
#endif

  return;
}

/* ====================================================================
 *    EMT_Emit_PU (pu, pu_dst, rwn)
 * ====================================================================
 */
void
EMT_Emit_PU (
  ST      *pu,
  DST_IDX  pu_dst, 
  WN      *rwn
)
{
  ST *sym;
  ST *base;
  BB *bb;
  INT Initial_Pu_PC;
  INT64 ofst;
  INT i;

  Trace_Inst	= Get_Trace ( TP_EMIT,1 );
  // BOOL trace_unwind = Get_Trace (TP_EMIT, 64);

  if (Trace_Inst) {
    fprintf(TFile, "%s CFG before cgemit \n%s", DBar, DBar);
    Print_All_BBs ();
  }

#if 0
  Init_Unwind_Info (trace_unwind);

  /* In the IA-32 case, we need to convert fp register references
   * so that they reference entries in the fp register stack with the
   * proper offset.
   */
  STACK_FP_Fixup_PU();

  if ( Run_prompf ) {
    const char *path = Anl_File_Path();
    anl_file = fopen(path, "a");
    fprintf(anl_file, "\n");
  }
#endif

  Init_ST_elf_index(CURRENT_SYMTAB);

  cur_section = NULL;

  if (Lai_Code) {
    // I cannot do this while it is constantly changing !
    // When they learn to work out the specs and abide by them,
    // and communicate, I might enable this again, Arthur.
    //  emit_function_prototypes ();
  }

  // initialize any new global sections:
  // We don't do this in EMT_Begin_File because may not have done
  // data layout at that point.
  emit_global_symbols ();

  // emit global bss first so .org is correct
  Process_Bss_Data (GLOBAL_SYMTAB);

  /* Initialize any sections that might have been created by the backend. */
  FOREACH_SYMBOL (CURRENT_SYMTAB, sym, i) {
    base = Base_Symbol(sym);
    if (ST_class(base) == CLASS_BLOCK && STB_section(base)) {
      Init_Section(base);
    }
  }

#ifdef TARG_ST
  // [CG]: Create_Cold_Text_Section() into Setup_Text_Section_For_PU()
  Setup_Text_Section_For_PU (pu);
#else
  Create_Cold_Text_Section();

  Setup_Text_Section_For_PU (pu);
#endif

  Initial_Pu_PC = PC;
  Set_ST_ofst(pu, PC);

  /* 
   * Assign addresses to all local labels. Fixup long branches.
   * Perform branch target optimization.
   */
  R_Resolve_Branches (pu);

#if 0
  if (Object_Code) {
    Em_Add_New_Event (EK_ENTRY, PC, 0, 0, 0, PU_section);
  }
#endif
  if ( Assembly ) {
#ifdef TARG_ST
    if ( List_Notes ) {
#endif
#ifdef TARG_ST
      /* (cbr) demangle name */
      char *cp_name = cplus_demangle(ST_name(pu), DMGL_NO_OPTS);
      if (cp_name)
        fprintf ( Asm_File, "\n\t%s Program Unit: %s\n", ASM_CMNT, cp_name );
      else
#endif
        fprintf ( Asm_File, "\n\t%s Program Unit: %s\n", ASM_CMNT, ST_name(pu) );
    }
#ifdef TARG_ST
    // [CL] force alignment when starting a new function
    // (as alignment constraints in hb_hazards.cxx:Make_Bundles()
    // are reset for each new PU, we must reset alignment here)
    // [CG] Added call to Check_If_Should_Align_PU()
    INT pu_align = Check_If_Should_Align_PU(pu);
    if (pu_align < DEFAULT_FUNCTION_ALIGNMENT) pu_align = DEFAULT_FUNCTION_ALIGNMENT;
    if (pu_align) {
      fprintf(Asm_File, "\t%s %d\n", AS_ALIGN, pu_align);
    }
#endif
    if (AS_ENT) CGEMIT_Prn_Ent_In_Asm (pu);
#ifdef TEMPORARY_STABS_FOR_GDB
    // This is an ugly hack to enable basic debugging for IA-32 target
    if (Debug_Level > 0) {
      fprintf(Asm_File, ".stabs \"%s:F(0,1)\",36,0,0,%s\n", ST_name(pu), ST_name(pu));
      fprintf(Asm_File, "\t%s\t%s,%s\n", AS_TYPE, ST_name(pu), AS_TYPE_FUNC);
    }
#endif
    Print_Label (Asm_File, pu, 0);
    // .fframe is only used for unwind info,
    // and we plan on emitting that info directly.
    //    CGEMIT_Gen_Asm_Frame (Frame_Len);
  }

  if (Lai_Code) {
    fprintf (Lai_File, "\n\t%s Program Unit: %s\n", ASM_CMNT, ST_name(pu) );
    if (AS_ENT) CGEMIT_Prn_Ent_In_Asm (pu);

    Print_Label (Lai_File, pu, 0);
    // .fframe is only used for unwind info,
    // and we plan on emitting that info directly.
    //    CGEMIT_Gen_Asm_Frame (Frame_Len);
  }

  if (Assembly) {
    if (cur_section != PU_base) {
	/* reset to text section */
    	fprintf (Asm_File, "\n\t%s %s\n", AS_SECTION, ST_name(PU_base));
	cur_section = PU_base;
    }
  }
  if (Lai_Code) {
    if (cur_section != PU_base) {
      /* reset to text section */
      fprintf (Lai_File, "\n\t%s %s\n", AS_SECTION, ST_name(PU_base));
      cur_section = PU_base;
    }
    emit_input_directives ();
  }

  FOREACH_SYMBOL (CURRENT_SYMTAB, sym, i) {
    if (ST_is_not_used(sym)) continue;
    if (ST_sclass(sym) == SCLASS_COMMON) {
      (void)EMT_Put_Elf_Symbol (sym);
    }

    /* put all extern symbols into the elf symbol table. */
    if ((PU_src_lang(Get_Current_PU()) == PU_F77_LANG 
	|| PU_src_lang(Get_Current_PU()) == PU_F90_LANG) &&
	ST_sclass(sym) == SCLASS_EXTERN && 
	! ST_is_export_local(sym)) {
#ifdef TARG_ST
      if (Assembly) {
	fprintf(Asm_File, "\t%s\t", AS_GLOBAL);
	EMT_Write_Qualified_Name(Asm_File, sym);
	fprintf ( Asm_File, "\n");
	EMT_Visibility (Asm_File, ST_export(sym), sym);
      }
      if (Lai_Code) {
	fprintf(Lai_File, "\t%s\t", AS_GLOBAL);
	EMT_Write_Qualified_Name(Lai_File, sym);
	fprintf(Lai_File, "\n");
	EMT_Visibility (Lai_File, ST_export(sym), sym);
      }
#else
      if (Assembly) fprintf (Asm_File, "\t%s\t %s\n", AS_GLOBAL, ST_name(sym));
      if (Lai_Code) fprintf (Lai_File, "\t%s\t %s\n", AS_GLOBAL, ST_name(sym));
#endif
      if (Object_Code) EMT_Put_Elf_Symbol (sym);
    }

    if (Assembly) {
      if (ST_class(sym) == CLASS_VAR && ST_sclass(sym) == SCLASS_AUTO) {
	if (Has_Base_Block(sym)) {
	  Base_Symbol_And_Offset(sym, &base, &ofst);
	  if (List_Notes) {
	    fprintf ( Asm_File, "\t%s %s = %lld\n",
		      ASM_CMNT, ST_name(sym), ofst);
	  }
	}
      }
    }
    if (Lai_Code) {
      if (ST_class(sym) == CLASS_VAR && ST_sclass(sym) == SCLASS_AUTO) {
	if (Has_Base_Block(sym)) {
	  Base_Symbol_And_Offset(sym, &base, &ofst);
	  fprintf (Lai_File, "\t%s\t.auto %s at %s, %lld\n", 
                        ASM_CMNT, ST_name(sym), ST_name(base), ofst);
	}
      }
    }
  }

  if (Lai_Code) {
    /* emit vertual register declarations */
    emit_virtual_registers ();
  }

  /* Assemble each basic block in the PU */
  for (bb = REGION_First_BB; bb != NULL; bb = BB_next(bb)) {
    Setup_Text_Section_For_BB(bb);
    EMT_Assemble_BB (bb, rwn);
  }

#ifdef TARG_ST
  // [CL] record end of final BB for this PU, so that this PU and the
  // next one can be reordered later without invalidating Dwarf
  // information (otherwise, the final text_sequence of the current PU
  // would be ended when starting the next PU, resulting in a size of
  // the final text_sequence dependent upon the location of the
  // beginning of the next PU)
  if (generate_dwarf) {
    cache_last_label_info (Last_Label,
			   Em_Create_Section_Symbol(PU_section),
			   ST_pu(pu),
			   PC2Addr(Offset_From_Last_Label));
    end_previous_text_region(PU_section, PC);
  }
#endif

#ifdef SPLIT_BCO_ENABLED /* Thierry */
  Gen_EndSplit_Label(Asm_File);  
#endif /* BCO_Enabled Thierry */


  /* Revert back to the text section to end the PU. */
  Setup_Text_Section_For_BB(REGION_First_BB);
  Is_True(PU_base == text_base, ("first region BB was not in text section"));

  /* Emit the stuff needed at the end of the PU. */
  if (AS_END) {
    CGEMIT_Exit_In_Asm(pu);
  }

#ifdef BCO_ENABLED /* Thierry */
  /* Emit the profile_info associated with this PU. */
  if (Assembly && CG_emit_bb_freqs) 
    EMT_ProfileInfo_PU(pu, pu_dst, rwn);
#endif /* BCO_Enabled Thierry */

  /* Emit the initialized data associated with this PU. */
  Process_Initos_And_Literals (CURRENT_SYMTAB);
  Process_Bss_Data (CURRENT_SYMTAB);

  if (generate_dwarf) {
    Elf64_Word symindex;
    INT eh_offset;
    BOOL has_exc_scopes = PU_has_exc_scopes(ST_pu(pu));
#if 0
    if (Object_Code)
    	Em_Add_New_Event (EK_PEND, PC - ISA_INST_BYTES, 0, 0, 0, PU_section);
#endif
    /* get exception handling info */ 
    if (!CXX_Exceptions_On && has_exc_scopes) {
      eh_offset = symindex = (Elf64_Word)DW_DLX_EH_OFFSET_UNAVAILABLE;
    }
    else if (!has_exc_scopes) {
	eh_offset = symindex = (Elf64_Word)DW_DLX_NO_EH_OFFSET;
    }
    else {
      sym = EH_Get_PU_Range_ST();
      if (sym != NULL) {
	Base_Symbol_And_Offset (sym, &base, &ofst);
	eh_offset = ofst;
	Init_Section(base);	/* make sure eh_region is inited */
	symindex = ST_elf_index(base);
      } else {
	eh_offset = symindex = (Elf64_Word)DW_DLX_NO_EH_OFFSET;
      }
    }
    // Cg_Dwarf_Process_PU (PU_section, Initial_Pu_PC, PC, pu, pu_dst, symindex, eh_offset);
#ifdef TARG_ST
    Cg_Dwarf_Process_PU (
		Em_Create_Section_Symbol(PU_section),
		Initial_Pu_Label,
		Last_Label, PC2Addr(Offset_From_Last_Label),
		pu, pu_dst, symindex, eh_offset,
		// The following two arguments need to go away
		// once libunwind provides an interface that lets
		// us specify ranges symbolically.
		PC2Addr(Initial_Pu_PC), PC2Addr(PC));

    // [CL] reset labels after emission of a PU, so that
    // next PU starts from a clean state
    Last_Label = LABEL_IDX_ZERO;
    Offset_From_Last_Label = 0;
#else
    Cg_Dwarf_Process_PU (
		Em_Create_Section_Symbol(PU_section),
		Initial_Pu_Label,
		Last_Label, Offset_From_Last_Label,
		pu, pu_dst, symindex, eh_offset,
		// The following two arguments need to go away
		// once libunwind provides an interface that lets
		// us specify ranges symbolically.
		Initial_Pu_PC, PC);
#endif
  }

  PU_Size = PC - Initial_Pu_PC;
  Set_STB_size (PU_base, PC);
  text_PC = PC;

#ifndef TARG_ST // [CL] replaced by end_previous_text_region(PU_section, Addr2PC(PC)) above
  if (generate_dwarf) {
    // The final label in this PU is liable to get used in computing
    // arguments to Em_Dwarf_End_Text_Region_Semi_Symbolic, so we need
    // to squirrel away information about it.
    cache_last_label_info (Last_Label,
		Em_Create_Section_Symbol(PU_section),
		ST_pu(pu),
		PC2Addr(Offset_From_Last_Label));
  }
#endif

#if 0
  Finalize_Unwind_Info();
#endif

  return;
}

/* ====================================================================
 *    EMT_End_File ()
 * ====================================================================
 */
void
EMT_End_File( void )
{
  INT16 i;
  ST *sym;

  cur_section = NULL;
  Init_ST_elf_index(GLOBAL_SYMTAB);

  /* make sure all global symbols are initialized */
  FOREACH_SYMBOL (GLOBAL_SYMTAB, sym, i) {
    if (ST_class(sym) == CLASS_BLOCK && STB_section(sym)) {
      if (Emit_Global_Data && SEC_is_merge(STB_section_idx(sym)) )
	continue;	// merge sections go in each .o
      Init_Section(sym);
    }
    // emit commons here so order is preserved for datapools
    if (ST_sclass(sym) == SCLASS_COMMON) {
      if (ST_is_not_used (sym)) continue;
      (void)EMT_Put_Elf_Symbol (sym);
    }
  }

  if (Emit_Global_Data) {
    char *newname;
    // create dummy symbol to represent the section
    FOREACH_SYMBOL (GLOBAL_SYMTAB, sym, i) {
      if (ST_class(sym) != CLASS_BLOCK) continue;
      if (!STB_section(sym)) continue;
      // mergeable sections will be emitted into each .o
      if (SEC_is_merge(STB_section_idx(sym))) continue;
#ifdef TARG_ST
      // [CL] generate unique names
      UINT len = strlen ("_symbol_") + strlen(Ipa_Label_Suffix) + 1;
      char *new_str = (char *) alloca (len);
      strcpy (new_str, "_symbol_");
      strcat (new_str, Ipa_Label_Suffix);
      newname = Index_To_Str(Save_Str2(ST_name(sym), new_str));
#else
      newname = Index_To_Str(Save_Str2(ST_name(sym), "_symbol"));
#endif
#if 0
      if (Object_Code) {
	(void) Em_Add_New_Symbol (
		 newname,
		 0 /* offset */, 
		 0 /* size */,
		 STB_GLOBAL, STT_OBJECT, STO_INTERNAL,
		 Em_Get_Section_Index (em_scn[STB_scninfo_idx(sym)].scninfo));
      }
#endif
#ifdef TARG_ST
      // [CG]: Ensure section is initialized
      Init_Section(sym); 
#endif
      if (Assembly) {
	Change_Section_Origin (sym, 0);
	fprintf (Asm_File, "\t%s\t%s\n", AS_GLOBAL, newname);
#ifdef TARG_ST
	fprintf (Asm_File, "\t%s\t%s\n", AS_INTERNAL, newname);
#else
	ASM_DIR_STOINTERNAL(newname);
#endif
	fprintf (Asm_File, "%s:\n", newname);
      }
      if (Lai_Code) {
	Change_Section_Origin (sym, 0);
	fprintf (Lai_File, "\t%s\t%s\n", AS_GLOBAL, newname);
	ASM_DIR_STOINTERNAL(newname);
	fprintf (Lai_File, "%s:\n", newname);
      }
    }
  }

  /* 
   * If there weren't any PUs, we may have data initialization
   * associated with file scope data here:
   */
  Process_Bss_Data (GLOBAL_SYMTAB);
  Process_Initos_And_Literals (GLOBAL_SYMTAB);
  // We need two calls to  Process_Initos_And_Literals (GLOBAL_SYMTAB)
  // because while writing out INITOs new literals may be allocated
  Process_Initos_And_Literals (GLOBAL_SYMTAB);
  Process_Distr_Array ();

  FOREACH_SYMBOL (GLOBAL_SYMTAB, sym, i) {
    if (ST_class(sym) == CLASS_NAME) {
      if (ST_emit_symbol(sym)) {
	/* may be notype symbol */
	(void)EMT_Put_Elf_Symbol(sym);
      }
#if 0
      else if (ST_sclass(sym) == SCLASS_COMMENT && Object_Code 
	       && ! Read_Global_Data	// just put once in symtab.o
	       && ! DEBUG_Optimize_Space)
	{
	  char *buf = (char *) alloca (strlen("ident::: ") + strlen(ST_name(sym)));
	  sprintf(buf, "ident:::%s", ST_name(sym));
	  Em_Add_Comment (buf);
	}
#endif
    }

    if (ST_class(sym) == CLASS_VAR &&
        ST_is_fill_align(sym) &&
        !Has_Base_Block(sym)) {
      /* fill/align symbols are supposed to be allocated in be
       * but are not done if there were no PUs in the file.
       * Report that error here.
       */

      ErrMsg (EC_Is_Bad_Pragma_Abort, 
              "Fill/Align symbol",
              ST_name(sym),
              "requires the file to contain at least one function");
    }
	
    if (Has_Strong_Symbol(sym)) {
      ST *strongsym = ST_strong(sym);
      unsigned char symtype;

#ifndef TARG_ST
      /* (cbr) emit alias information evenif strongsym is extern */
      if (!Has_Base_Block(strongsym))
	continue;	/* strong not allocated, so ignore weak */
#endif
      
      if (ST_class(sym) == CLASS_FUNC) {
	symtype = STT_FUNC;
      }
      else {
	symtype = STT_OBJECT;
      }

      if (Assembly) {
	CGEMIT_Weak_Alias (sym, strongsym);
	Print_Dynsym (Asm_File, sym);
      }
#if 0
      if (Object_Code) {
	Em_Add_New_Weak_Symbol (
	  ST_name(sym), 
	  symtype,
	  st_other_for_sym (sym),
	  EMT_Put_Elf_Symbol(strongsym));
      }
#endif
    }
    else if (Has_Base_Block(sym) && ST_class(ST_base(sym)) != CLASS_BLOCK
	&& ST_emit_symbol(sym)) {
      // alias
      if (Assembly) {
	if ( ! ST_is_export_local(sym)) {
	  fprintf(Asm_File, "\t%s\t", AS_GLOBAL);
	  EMT_Write_Qualified_Name(Asm_File, sym);
	  fprintf ( Asm_File, "\n");
#ifdef TARG_ST
	  EMT_Visibility (Asm_File, ST_export(sym), sym);
#endif
	}
	CGEMIT_Alias (sym, ST_base(sym));
      }
#if 0
      if (Lai_Code) {
	if (!ST_is_export_local(sym)) {
	  fprintf(Lai_File, "\t%s\t", AS_GLOBAL);
	  EMT_Write_Qualified_Name(Lai_File, sym);
	  fprintf ( Lai_File, "\n");

	}
      }
#endif
    }
    else if (ST_class(sym) == CLASS_FUNC && ST_emit_symbol(sym)
	// possible to have local not-used emit_symbols,
	// cause mp does that to mark them as unused,
	// and don't want to emit those.
	&& ST_sclass(sym) == SCLASS_EXTERN) {
      // some unreferenced fortran externs need to be emitted
      (void)EMT_Put_Elf_Symbol(sym);
      if (Assembly) {
	fprintf(Asm_File, "\t%s\t", AS_GLOBAL);
	EMT_Write_Qualified_Name(Asm_File, sym);
	fprintf(Asm_File, "\n");
#ifdef TARG_ST
	EMT_Visibility(Asm_File, ST_export(sym), sym);
#endif
      }
      if (Lai_Code) {
	fprintf(Lai_File, "\t%s\t", AS_GLOBAL);
	EMT_Write_Qualified_Name(Lai_File, sym);
	fprintf(Lai_File, "\n");
#ifdef TARG_ST
	EMT_Visibility(Lai_File, ST_export(sym), sym);
#endif
      }
    }
  }

#ifdef TEMPORARY_STABS_FOR_GDB
  // This is an ugly hack to enable basic debugging for IA-32 target
  if (PU_base == NULL && Assembly && Debug_Level > 0) {
    fprintf(Asm_File, ".Ltext0:\n");
  }
#endif

#ifndef TARG_ST // [CL] previous text_region was closed it the end of EMT_Emit_PU()
  if (generate_elf_symbols && PU_section != NULL) {
    end_previous_text_region(PU_section, text_PC);
  }
#endif

#if 0
  if (Object_Code) {
    Em_Options_Scn();
  }
#endif
  if (generate_dwarf) {
    // must write out dwarf unwind info before text section is ended
    Cg_Dwarf_Finish (PU_section);
  }

  /* Write out the initialized data to the object file. */
  for (i = 1; i <= last_scn; i++) {
    sym = em_scn[i].sym;
#if 0
    if (Object_Code) {

#ifdef PV_205345
      /* Data section alignment is initially set to the maximum
       * alignment required by the objects allocated to the section.
       * Whenever Find_Alignment is called for an object in a section
       * with smaller alignment than it's quantum of interest, it
       * updates the section alignment to guarantee that the
       * determined alignment is valid.  This override can be enabled
       * with -DPV_205345 to force alignment to at least 8 bytes.
       */
      if (STB_align(sym) < 8) Set_STB_align(sym, 8);
#endif /* PV_205345 */

      if (STB_nobits(sym)) {
	/* For the .bss section, the size field should be set explicitly. */
	Em_Add_Bytes_To_Scn (em_scn[i].scninfo, NULL,  
		STB_size(sym), STB_align(sym));
      }
      else {
	Em_Change_Section_Alignment (em_scn[i].scninfo, STB_align(sym));
      }
      Em_End_Section (em_scn[i].scninfo);
    }
#endif

    if (Assembly) {
      UINT32 tmp, power;
      power = 0;
      for (tmp = STB_align(sym); tmp > 1; tmp >>= 1) power++;
      fprintf (Asm_File, "\t%s %s\n", AS_SECTION, ST_name(sym));
      ASM_DIR_ALIGN(power, sym);
    }
    if (Lai_Code) {
      UINT32 tmp, power;
      power = 0;
      for (tmp = STB_align(sym); tmp > 1; tmp >>= 1) power++;
      fprintf (Lai_File, "\t%s %s\n", AS_SECTION, ST_name(sym));
      fprintf(Lai_File, "\t%s %d\n", AS_ALIGN,STB_align(sym));
    }
  }

  INT dwarf_section_count = 0;

  if (generate_dwarf) {
    dwarf_section_count = Em_Dwarf_Prepare_Output ();
  }

  if (Assembly) {
#ifdef TARG_ST
    if (List_Notes)
#endif
    fprintf(Asm_File, "\t## %s %d\n", AS_GPVALUE, GP_DISP);
    //    ASM_DIR_GPVALUE();
    if (CG_emit_asm_dwarf) {
      Cg_Dwarf_Write_Assembly_From_Symbolic_Relocs(Asm_File,
						   dwarf_section_count,
						   !Use_32_Bit_Pointers);
    }
  }
  if (Lai_Code) {
    fprintf(Lai_File, "//\t%s %d\n", AS_GPVALUE, GP_DISP);
  }
#if 0
  if (Object_Code && !CG_emit_asm_dwarf) {
    /* TODO: compute the parameters more accurately. For now we assume 
     * that all integer and FP registers are used. If we change the GP
     * value to be non-zero, make sure we adjust the addends for the 
     * GP_REL cases.
     */
    Em_Write_Reginfo (GP_DISP, 0xffffffff, 0xffffffff, Pure_ABI); 

    Em_Dwarf_Write_Scns (Cg_Dwarf_Translate_To_Elf);

    /* finalize .interfaces section (must be before Em_End_File) */
    if ( EMIT_interface_section )
      Interface_Scn_End_File();

    Em_End_File ();
    Em_Dwarf_End ();
    Em_Cleanup_Unwind ();
  }
#endif

  if (Emit_Global_Data) {
    // prepare block data to be written to .G file.
    // need to remove section info so can be reset when read.
    FOREACH_SYMBOL (GLOBAL_SYMTAB, sym, i) {
      if (ST_class(sym) == CLASS_CONST
		  && SEC_is_merge(STB_section_idx(ST_base(sym))) ) {
	// reallocate in each file
	Set_ST_base(sym,sym);	
	Set_ST_ofst(sym,0);	
      }
      else if (ST_class(sym) == CLASS_VAR ||
	       ST_class(sym) == CLASS_CONST ||
	       ST_class(sym) == CLASS_FUNC) {
	if (ST_sclass (sym) != SCLASS_COMMON && !ST_is_weak_symbol(sym) ) {
	  Set_ST_sclass(sym, SCLASS_EXTERN);
	}
      }
      if (ST_class(sym) != CLASS_BLOCK) continue;
      Set_STB_scninfo_idx(sym,0);
      Set_STB_compiler_layout(sym);
      if (STB_section(sym)) {
	Reset_STB_section(sym);
	Reset_STB_root_base(sym);
	Set_STB_section_idx(sym,0);
#ifdef TARG_ST
	// [CL] generate unique names
	UINT len = strlen ("_symbol_") + strlen(Ipa_Label_Suffix) + 1;
	char *new_str = (char *) alloca (len);
	strcpy (new_str, "_symbol_");
	strcat (new_str, Ipa_Label_Suffix);
	Set_ST_name(sym, Save_Str2(ST_name(sym), new_str));
#else
	Set_ST_name(sym, Save_Str2(ST_name(sym), "_symbol"));
#endif
	Set_ST_sclass(sym, SCLASS_EXTERN);
	Set_ST_export(sym, EXPORT_INTERNAL);
      }
      else {
	// in case non-section block
	Set_ST_sclass(sym, SCLASS_EXTERN);
      }
    }
  }

  // Finish off the TCON to symbolic names table:
  //  Fini_Tcon_Info ();

#ifdef TARG_ST
  // Target dependent end file.
  if (Assembly) CGEMIT_End_File_In_Asm ();
#endif

  return;
}

