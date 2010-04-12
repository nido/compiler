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


/* ====================================================================
 * ====================================================================
 * Module: cgdwarf.c
 *
 * Description:
 *
 * Interface between cgemit and em_dwarf.c 
 *
 * ====================================================================
 * ====================================================================
 */

#define LIBDWARF_SORTS_RELOCS

#if !defined(LIBDWARF_SORTS_RELOCS)
#include <stdlib.h>		// qsort
#endif

#include <stdio.h>
#include <elf.h>
#include <elfaccess.h>
#include <libelf.h>
// [HK]
#if __GNUC__ >= 3
#include <vector>
#else
#include <vector.h>
#endif // __GNUC__ >= 3

#define	USE_STANDARD_TYPES 1
#include "defs.h"
#include "erglob.h"
#include "glob.h"
#include "flags.h"
#include "tracing.h"
#include "util.h"
#include "config.h"
#include "config_asm.h"
#include "config_list.h"
#include "config_debug.h"
#include "cgir.h"
#include "mempool.h"
#include "tn_set.h"
#include "srcpos.h"
#include "em_elf.h"
#include "em_dwarf.h"
#include "targ_const.h"
#include "calls.h"
#include "stblock.h"
#include "data_layout.h"
#include "dwarf_DST.h"
#include "dwarf_DST_mem.h"
#include "const.h"
#include "cg.h"
#include "cgtarget.h"
#include "cgemit.h"
#include "cgdwarf.h"
#include "label_util.h"
#include "vstring.h"
#include "cgemit_targ.h"
#include "cgdwarf_debug_frame.h"
#include "cgdwarf_targ.h"
#ifdef TARG_ST
#include "whirl2ops.h" // [CL] needed for Find_PREG_For_Symbol()
#endif
#ifdef TARG_STxP70
#    include "config_TARG.h"
#endif

#ifdef TARG_ST
#include "whirl2ops.h" // [CL] needed for Find_PREG_For_Symbol()
#endif

BOOL Trace_Dwarf;

#ifdef TARG_ST
// Since we need to interpret array of bytes with the "dwarf specification" of
// the target, we have to know internal libdwarf structure. Actually, we need
// only the size of some operands 
#include "pro_incl.h"
#define SIZEOF_SECTION_LENGTH dw_dbg->de_offset_size
#endif

static Dwarf_P_Debug dw_dbg;
static Dwarf_Error dw_error;
static BOOL Disable_DST = FALSE;
static DST_INFO_IDX cu_idx;
static Elf64_Word cur_text_index;
static DST_language Dwarf_Language;
//static INT Current_Tree_Level;

/* used as array to hold pointers to enclosing procedure's DIEs */
static Dwarf_P_Die *CGD_enclosing_proc = NULL;
static mINT32 CGD_enclosing_proc_max = 0;

#define GLOBAL_LEVEL 0
#define LOCAL_LEVEL 1

struct CGD_SYMTAB_ENTRY {
  CGD_SYMTAB_ENTRY_TYPE type;
  Dwarf_Unsigned        index;
  union {
    struct {
      PU_IDX         pu_idx;	// PU in which the label occurs
      STR_IDX        name_idx;  // use global Str_Table index,
				// (using Index_To_Str())
				// not string, as global strings
				// table can move, but index safe
      Dwarf_Unsigned offset;	// offset within ELF section
      Elf64_Word     base_sym;	// ELF symbol index for label's ELF section
    } label_info;
    struct {
      // I think the ELF symbol index tells us everything we need to know.
    } elfsym_info;
  };

  CGD_SYMTAB_ENTRY(CGD_SYMTAB_ENTRY_TYPE entry_type, Dwarf_Unsigned idx) :
    type(entry_type), index(idx)
      { }
};

vector <CGD_SYMTAB_ENTRY, mempool_allocator<CGD_SYMTAB_ENTRY> > CGD_Symtab;

Dwarf_Unsigned Cg_Dwarf_Symtab_Entry(CGD_SYMTAB_ENTRY_TYPE  type,
				     Dwarf_Unsigned         index,
				     Dwarf_Unsigned         pu_base_sym_idx,
				     PU_IDX                 pu,
				     char		   *label_name,
				     Dwarf_Unsigned         last_offset)
{
  if (type == CGD_LABIDX && pu == (PU_IDX) 0) {
    pu = ST_pu(Get_Current_PU_ST());
  }
  vector <CGD_SYMTAB_ENTRY,
	  mempool_allocator<CGD_SYMTAB_ENTRY> >::iterator p;
  for (p = CGD_Symtab.begin(); p != CGD_Symtab.end(); ++p) {
    if (p->type == type &&
	p->index == index) {
      switch (type) {
      case CGD_LABIDX:
	if (p->label_info.pu_idx == pu) {
	  return p - CGD_Symtab.begin();
	}
	break;
      case CGD_ELFSYM:
	return p - CGD_Symtab.begin();
      default:
	Fail_FmtAssertion("Illegal CGD_Symtab entry type");
	break;
      }
    }
  }
  CGD_Symtab.push_back(CGD_SYMTAB_ENTRY(type, index));
  if (Trace_Dwarf) {
    fprintf(TFile,
	    "New CGD_Symtab entry: %u --> (CGD_%s,%llu)\n",
	    CGD_Symtab.size() - 1,
	    (type == CGD_LABIDX ? "LABIDX" : "ELFSYM"),
	    index);
  }
  Dwarf_Unsigned handle = CGD_Symtab.size() - 1;
  if (type == CGD_LABIDX) {
    // Put label-specific fields in place
    CGD_Symtab[handle].label_info.pu_idx = pu;
    if (label_name == NULL) {
      CGD_Symtab[handle].label_info.name_idx = LABEL_name_idx(index);
      CGD_Symtab[handle].label_info.offset = Get_Label_Offset(index);
    }
    else {
      STR_IDX lstr = Save_Str(label_name);
      CGD_Symtab[handle].label_info.name_idx = lstr;
      CGD_Symtab[handle].label_info.offset = last_offset;
    }
    CGD_Symtab[handle].label_info.base_sym = pu_base_sym_idx;
    if (Trace_Dwarf) {
	STR_IDX sidx = CGD_Symtab[handle].label_info.name_idx;
      fprintf(TFile,
	      "pu_idx: %d; label: %d %s; ofst: %llu; base: %u\n",
	      CGD_Symtab[handle].label_info.pu_idx,
	      (int)sidx,
	      Index_To_Str(sidx),
	      CGD_Symtab[handle].label_info.offset,
	      CGD_Symtab[handle].label_info.base_sym);
    }
    FmtAssert(pu_base_sym_idx != 0,
	      ("ELF symbol for base of label's section must be specified"));
  }
  else if (type == CGD_ELFSYM) {
    // Put elfsym-specific fields in place
  }
  else {
    Fail_FmtAssertion("Illegal CGD_Symtab entry type");
  }
  return handle;
}

#ifdef TARG_ST
// Forward declaration of local function
static void Cg_Dwarf_Register_Source_File(USRCPOS usrcpos);

// Return TRUE if the size of section <section_name> cannot be statically known.
// This is the case when generating symbolic offsets in .debug_line section,
// because offsets are generated using LEB128 type, that use a variable length
// encoding (known at assembly time depending on label value).
BOOL Cg_Dwarf_Section_Need_Symbolic_Size(const char *section_name) {
  if (Dwarf_Require_Symbolic_Offsets() && !strcmp(section_name, ".debug_line")) {
    return (TRUE);
  }
  return (FALSE);
}
#endif

Dwarf_Unsigned Cg_Dwarf_Enter_Elfsym(Elf64_Word index)
{
  return Cg_Dwarf_Symtab_Entry(CGD_ELFSYM, index);
}

Elf64_Word Cg_Dwarf_Translate_Symidx(Dwarf_Unsigned idx_from_sym_reloc)
{
  Is_True(CGD_Symtab.size() > idx_from_sym_reloc,
	  ("Cg_Dwarf_Translate_Symidx: Index %llu out of bounds (%lu)",
	   idx_from_sym_reloc, CGD_Symtab.size()));
  if (Trace_Dwarf) {
    fprintf(TFile, "Translating %llu ", idx_from_sym_reloc);
    fflush(TFile);
    fprintf(TFile, "to %llu\n", CGD_Symtab[idx_from_sym_reloc].index);
  }
  Is_True(CGD_Symtab[idx_from_sym_reloc].type == CGD_ELFSYM,
	  ("Cg_Dwarf_Translate_Symidx: Unexpected entry type"));
  return CGD_Symtab[idx_from_sym_reloc].index;
}

Elf64_Word Cg_Dwarf_Translate_Offset(Dwarf_Unsigned idx_from_sym_reloc)
{
  Is_True(CGD_Symtab.size() > idx_from_sym_reloc,
	  ("Cg_Dwarf_Translate_Offset: Index %llu out of bounds (%lu)",
	   idx_from_sym_reloc, CGD_Symtab.size()));
  if (Trace_Dwarf) {
    fprintf(TFile, "Translating %llu ", idx_from_sym_reloc);
    fflush(TFile);
    fprintf(TFile, "through index %llu ",
	    CGD_Symtab[idx_from_sym_reloc].index);
    fprintf(TFile, "to %lld\n",
	    Get_Label_Offset(CGD_Symtab[idx_from_sym_reloc].index));
  }
  Is_True(CGD_Symtab[idx_from_sym_reloc].type == CGD_LABIDX,
	  ("Cg_Dward_Translate_Offset: Unexpected entry type"));
  return Get_Label_Offset(CGD_Symtab[idx_from_sym_reloc].index);
}

void
Cg_Dwarf_Translate_To_Elf(Dwarf_Unsigned  idx_from_sym_reloc,
			  Dwarf_Unsigned *elf_symbol,
			  Dwarf_Unsigned *elf_offset)
{
  Is_True(CGD_Symtab.size() > idx_from_sym_reloc,
	  ("Cg_Dwarf_Translate_Offset: Index %llu out of bounds (%lu)",
	   idx_from_sym_reloc, CGD_Symtab.size()));
  if (Trace_Dwarf) {
    fprintf(TFile, "Translating %llu ", idx_from_sym_reloc);
    fflush(TFile);
    fprintf(TFile, "through index %llu ",
	    CGD_Symtab[idx_from_sym_reloc].index);
  }
  if (CGD_Symtab[idx_from_sym_reloc].type == CGD_LABIDX) {
    *elf_symbol = CGD_Symtab[idx_from_sym_reloc].label_info.base_sym;
    *elf_offset = CGD_Symtab[idx_from_sym_reloc].label_info.offset;
    if (Trace_Dwarf) {
      fprintf(TFile, "to elfsym %s + 0x%llx\n",
	      Em_Get_Symbol_Name(*elf_symbol), *elf_offset);
    }
  }
  else  {
    Is_True(CGD_Symtab[idx_from_sym_reloc].type == CGD_ELFSYM,
	    ("illegal CGD_Symtab entry type"));
    *elf_symbol = CGD_Symtab[idx_from_sym_reloc].index;
    *elf_offset = 0;
    if (Trace_Dwarf) {
      fprintf(TFile, "to elfsym %s\n",
	      Em_Get_Symbol_Name(*elf_symbol));
    }
  }
}

// The string returned is not valid across
// reallocs of the global string table,
// so callers must not save the string pointer returned.
char *
Cg_Dwarf_Name_From_Handle(Dwarf_Unsigned idx)
{
  Is_True(CGD_Symtab.size() > idx,
	  ("Cg_Dwarf_Name_From_Handle: Index %llu out of bounds (%lu)",
	   idx, CGD_Symtab.size()));
  if (CGD_Symtab[idx].type == CGD_ELFSYM) {
    return Em_Get_Symbol_Name(CGD_Symtab[idx].index);
  }
  else {
	// is CGD_LABEL
    STR_IDX sidx = CGD_Symtab[idx].label_info.name_idx;
    return Index_To_Str(sidx);
  }
}

#ifdef TARG_ST
// [SC] TLS support
static int
Cg_Dwarf_Type_From_Handle(Dwarf_Unsigned idx)
{
  Is_True(CGD_Symtab.size() > idx,
	  ("Cg_Dwarf_Type_From_Handle: Index %llu out of bounds (%lu)",
	   idx, CGD_Symtab.size()));
  if (CGD_Symtab[idx].type == CGD_ELFSYM)
    return Em_Get_Symbol_Type(CGD_Symtab[idx].index);
  else
    return STT_NOTYPE;
}
#endif

#define put_flag(flag, die) dwarf_add_AT_flag(dw_dbg, die, flag, 1, &dw_error)

static ST *
Get_ST_From_DST (DST_ASSOC_INFO assoc_info)
{
	ST *st;
	st = &St_Table(
		DST_ASSOC_INFO_st_level(assoc_info),
		DST_ASSOC_INFO_st_index(assoc_info) );
	FmtAssert ((st != NULL), 
		("Get_ST_From_DST: bad dst info from fe?  assoc_info = (%d,%d)",
			DST_ASSOC_INFO_st_level(assoc_info), 
			DST_ASSOC_INFO_st_index(assoc_info) ));
	return st;
}

/* Given a 'ref_idx', return the dieptr for that die. */
static Dwarf_P_Die get_ref_die (DST_INFO_IDX ref_idx)
{
  DST_INFO *info;
  Dwarf_P_Die ref_die;

  info = DST_INFO_IDX_TO_PTR(ref_idx);
  FmtAssert(DST_INFO_tag(info) != 0, 
	("get_ref_die found 0 tag for idx %d,%d", 
	ref_idx.block_idx, ref_idx.byte_idx));
  ref_die = (Dwarf_P_Die) DST_INFO_dieptr (info);
  if (ref_die == NULL) {
    ref_die = dwarf_new_die (dw_dbg, DST_INFO_tag(info), NULL, NULL, 
			NULL, NULL, &dw_error);
    DST_INFO_dieptr(info) = ref_die;
    if (Trace_Dwarf) {
      fprintf (TFile,"NEW ref die for [%d,%d]: %p, tag:%d\n",
	ref_idx.block_idx, ref_idx.byte_idx, ref_die, DST_INFO_tag(info));
    }
  }
  return ref_die;
}

/* Given a 'ref_idx', write it out as the 'ref_attr' attribute for 'die'. */
static void
put_reference (DST_INFO_IDX ref_idx, Dwarf_Half ref_attr, Dwarf_P_Die die)
{
  Dwarf_P_Die ref_die;
    
  if (DST_IS_NULL(ref_idx)) return;

  FmtAssert ( ! DST_IS_FOREIGN_OBJ(ref_idx), 
	("Dwarf reference to foreign object"));
  ref_die = get_ref_die (ref_idx);
  dwarf_add_AT_reference (dw_dbg, die, ref_attr, ref_die, &dw_error);

}

/* enumeration to decide which pubnames section to add the name to. */
typedef enum {
  pb_none,
  pb_pubname,
  pb_funcname,
  pb_weakname,
  pb_varname,
  pb_typename
} which_pb;

/* write out the name to one of the public-names sections if needed.
   If -g is not specified, do not write out names to the varname 
   and typename sections.
*/
static void 
put_pubname (char *name, Dwarf_P_Die die, which_pb pb_type)
{
    switch (pb_type) {
    case pb_pubname:
      dwarf_add_pubname (dw_dbg, die, name, &dw_error);
      break;
    case pb_funcname:
      dwarf_add_funcname (dw_dbg, die, name, &dw_error);
      break;
    case pb_weakname:
      dwarf_add_weakname (dw_dbg, die, name, &dw_error);
      break;
    case pb_varname:
      if (Debug_Level > 0) dwarf_add_varname (dw_dbg, die, name, &dw_error);
      break;
    case pb_typename:
      if (Debug_Level > 0) dwarf_add_typename (dw_dbg, die, name, &dw_error);
      break;
    default:
      break;
    }
}

/* Write out 'str_idx' as a 'str_attr' attribute. */
static void
put_string (DST_STR_IDX str_idx, Dwarf_Half str_attr, Dwarf_P_Die die)
{
  char *name;

  if (DST_IS_NULL(str_idx)) return;

  name = DST_STR_IDX_TO_PTR (str_idx);
  dwarf_add_AT_string (dw_dbg, die, str_attr, name, &dw_error);
}

/* Given a 'str_idx', write it out as the AT_name attribute for 'die'. */
static void
put_name (DST_STR_IDX str_idx, Dwarf_P_Die die, which_pb pb_type)
{
  char *name;

  if (DST_IS_NULL(str_idx)) return;

  name = DST_STR_IDX_TO_PTR (str_idx);
  put_string (str_idx, DW_AT_name, die);
  put_pubname (name, die, pb_type);
  if (Trace_Dwarf) {
    fprintf (TFile,"AT_name attribute: %s\n", DST_STR_IDX_TO_PTR (str_idx));
  }
}

/* write out the source position attributes for a declaration. */
static void
put_decl(USRCPOS decl, Dwarf_P_Die die)
{
#ifdef TARG_ST
  // [CL] Ensure that corresponding source file has been added to
  // dwarf info
  Cg_Dwarf_Register_Source_File(decl);
#endif

   if (USRCPOS_filenum(decl) != 0)
//DevWarn("file # %d", USRCPOS_filenum(decl));
     dwarf_add_AT_unsigned_const (dw_dbg, die, DW_AT_decl_file, 
#ifdef TARG_ST // [CL] use the dwarf file number
			   File_Dwarf_Idx(USRCPOS_filenum(decl)), &dw_error);
#else
			   (UINT32)USRCPOS_filenum(decl), &dw_error);
#endif
   if (USRCPOS_linenum(decl) != 0)
     dwarf_add_AT_unsigned_const (dw_dbg, die, DW_AT_decl_line, 
			   (UINT32)USRCPOS_linenum(decl), &dw_error);
  if (USRCPOS_column(decl) != 0)
    dwarf_add_AT_unsigned_const (dw_dbg, die, DW_AT_decl_column, 
			   (UINT32)USRCPOS_column(decl), &dw_error);
}

static void
put_const_attribute (DST_CONST_VALUE cval, Dwarf_Half ref_attr, Dwarf_P_Die die)
{
  switch (DST_CONST_VALUE_form(cval)) {
    case DST_FORM_STRING:
      put_string (DST_CONST_VALUE_form_string(cval), ref_attr, die);
      break;
    case DST_FORM_DATA1:
      dwarf_add_AT_unsigned_const (dw_dbg, die, ref_attr,
	      DST_CONST_VALUE_form_data1(cval), &dw_error);
      break;
    case DST_FORM_DATA2:
      dwarf_add_AT_unsigned_const (dw_dbg, die, ref_attr,
	      DST_CONST_VALUE_form_data2(cval), &dw_error);
      break;
    case DST_FORM_DATA4:
      dwarf_add_AT_unsigned_const (dw_dbg, die, ref_attr,
	      DST_CONST_VALUE_form_data4(cval), &dw_error);
      break;
    case DST_FORM_DATA8:
      dwarf_add_AT_unsigned_const (dw_dbg, die, ref_attr,
	      DST_CONST_VALUE_form_data8(cval), &dw_error);
      break;
  }
}

// output dopetype info if it exists
static void
put_dopetype (DST_INFO_IDX dope_idx, DST_flag flag, Dwarf_P_Die die)
{
   if (DST_IS_f90_pointer(flag))
	put_reference (dope_idx, DW_AT_MIPS_ptr_dopetype, die);
   if (DST_IS_allocatable(flag))
	put_reference (dope_idx, DW_AT_MIPS_allocatable_dopetype, die);
   if (DST_IS_assumed_shape(flag))
	put_reference (dope_idx, DW_AT_MIPS_assumed_shape_dopetype, die);
}

   /*----------------------------------
    * One put routine for each DW_TAG
    *----------------------------------*/


static void
put_compile_unit(DST_COMPILE_UNIT *attr, Dwarf_P_Die die)
{
   put_name (DST_COMPILE_UNIT_name(attr), die, pb_none);
   put_string (DST_COMPILE_UNIT_comp_dir(attr), DW_AT_comp_dir, die);
   if (DEBUG_Optimize_Space && Debug_Level == 0)
	// don't emit producer string if not debug and want to save space
	dwarf_add_AT_string (dw_dbg, die, DW_AT_producer, "", &dw_error);
   else
  	put_string (DST_COMPILE_UNIT_producer(attr), DW_AT_producer, die);
   dwarf_add_AT_unsigned_const (dw_dbg, die, DW_AT_language, 
	     DST_COMPILE_UNIT_language(attr), &dw_error);
   dwarf_add_AT_unsigned_const (dw_dbg, die, DW_AT_identifier_case, 
	     DST_COMPILE_UNIT_identifier_case(attr), &dw_error);

   if (FILE_INFO_has_inlines (File_info))
	put_flag (DW_AT_MIPS_has_inlines, die);
}

static BOOL
subprogram_def_is_inlined (DST_SUBPROGRAM *attr)
{
    switch (DST_SUBPROGRAM_def_inline(attr)) {
    case DW_INL_inlined:
    case DW_INL_declared_inlined:
	return TRUE;
    default:
	return FALSE;
    }
}

static void
put_subprogram(DST_flag flag,
	       DST_SUBPROGRAM *attr,
	       Dwarf_P_Die die)
{
  Dwarf_P_Expr expr;

  if (DST_IS_memdef(flag))  /* Not yet supported */ {
    ErrMsg (EC_Unimplemented, 
	" put_subprogram: a class member with AT_specification!");
  }
  else if (DST_IS_declaration(flag)) {
    /* If the origin field is not NULL, then this declaration is really
       a weakname and the origin points to the DST entry for the strong
       symbol. Emit an entry in the weaknames section pointing to the
       original subprogram die.
    */
    if (!DST_IS_NULL(DST_SUBPROGRAM_decl_origin(attr))) {
      if (!DST_IS_NULL(DST_SUBPROGRAM_decl_name(attr))) {
        put_pubname (DST_STR_IDX_TO_PTR(DST_SUBPROGRAM_decl_name(attr)),
		     get_ref_die (DST_SUBPROGRAM_decl_origin(attr)),
		     pb_weakname);
      }
    }
    put_decl(DST_SUBPROGRAM_decl_decl(attr), die);
    put_name (DST_SUBPROGRAM_decl_name(attr), die, pb_none);
    put_reference (DST_SUBPROGRAM_decl_type(attr), DW_AT_type, die);
    put_flag (DW_AT_declaration, die);
    if (DST_IS_external(flag)) put_flag (DW_AT_external, die);
    if (DST_IS_prototyped(flag)) put_flag (DW_AT_prototyped, die);
    if (DST_SUBPROGRAM_decl_virtuality(attr) != DW_VIRTUALITY_none) {
       dwarf_add_AT_unsigned_const(dw_dbg, die, DW_AT_virtuality,
			           DST_SUBPROGRAM_decl_virtuality(attr),
				   &dw_error);
       expr = dwarf_new_expr(dw_dbg, &dw_error);
       dwarf_add_expr_gen(expr,
              	 	  DW_OP_const2u,
			  DST_SUBPROGRAM_decl_vtable_elem_location(attr),
			  0,
			  &dw_error);
       dwarf_add_AT_location_expr(dw_dbg, die, DW_AT_vtable_elem_location,
				  expr, &dw_error);
    }
    put_string (DST_SUBPROGRAM_decl_linkage_name(attr),
		DW_AT_MIPS_linkage_name, die);

    switch (DST_SUBPROGRAM_decl_inline(attr)) {
    case DW_INL_inlined:
    case DW_INL_declared_inlined:
   	dwarf_add_AT_unsigned_const (dw_dbg, die, DW_AT_inline, 
			DST_SUBPROGRAM_decl_inline(attr), &dw_error);
	break;
    }
  }
  else /* definition */ {
    char *pubname;

    put_decl(DST_SUBPROGRAM_def_decl(attr), die);
    /* check if there is a pubnames name for subroutine (for member fns) */
    if (!DST_IS_NULL(DST_SUBPROGRAM_def_pubname(attr))) {
      put_name (DST_SUBPROGRAM_def_name(attr), die, pb_none);
      pubname = DST_STR_IDX_TO_PTR (DST_SUBPROGRAM_def_pubname(attr));
      dwarf_add_pubname (dw_dbg, die, pubname, &dw_error);
    }
    else {
      put_name (DST_SUBPROGRAM_def_name(attr), 
	        die, 
	        DST_IS_external(flag) ? pb_pubname : pb_funcname);
    }
    put_reference (DST_SUBPROGRAM_def_type(attr), DW_AT_type, die);
    if (DST_IS_external(flag))  put_flag (DW_AT_external, die);
    if (DST_IS_prototyped(flag)) put_flag (DW_AT_prototyped, die);
    /* check if we have a pointer from def to decl (for member function) */
    put_reference (DST_SUBPROGRAM_def_specification(attr),
		      DW_AT_specification, die);
    if (DST_SUBPROGRAM_def_virtuality(attr) != DW_VIRTUALITY_none) {
       dwarf_add_AT_unsigned_const(dw_dbg, die, DW_AT_virtuality,
			           DST_SUBPROGRAM_def_virtuality(attr),
				   &dw_error);
       expr = dwarf_new_expr(dw_dbg, &dw_error);
       dwarf_add_expr_gen(expr,
              	 	  DW_OP_const2u,
			  DST_SUBPROGRAM_def_vtable_elem_location(attr),
			  0,
			  &dw_error);
       dwarf_add_AT_location_expr(dw_dbg, die, DW_AT_vtable_elem_location,
				  expr, &dw_error);
    }
       
    put_string (DST_SUBPROGRAM_def_linkage_name(attr),
		DW_AT_MIPS_linkage_name, die);
    if (!DST_IS_NULL(DST_SUBPROGRAM_def_clone_origin(attr))) {
	put_reference (DST_SUBPROGRAM_def_clone_origin(attr),
			DW_AT_MIPS_clone_origin, die);
   	dwarf_add_AT_unsigned_const (dw_dbg, die, DW_AT_calling_convention, 
			DW_CC_nocall, &dw_error);
    }
    if (subprogram_def_is_inlined(attr)) {
   	dwarf_add_AT_unsigned_const (dw_dbg, die, DW_AT_inline, 
			DST_SUBPROGRAM_def_inline(attr), &dw_error);
    }
    if (PU_has_inlines(Get_Current_PU()))
	put_flag (DW_AT_MIPS_has_inlines, die);
  }
}


static Elf64_Word 
get_elfindex_from_ASSOC_INFO (DST_ASSOC_INFO assoc_info)
{
  ST *st;
  st = Get_ST_From_DST (assoc_info);
  return EMT_Put_Elf_Symbol(st);
}

static mINT32 
get_ofst_from_ASSOC_INFO (DST_ASSOC_INFO assoc_info)
{
  ST *st;
  st = Get_ST_From_DST (assoc_info);
  return (st != NULL) ? ST_ofst(st) : 0;
}

/* TEMPORARY:  when we combine index field between labels and vars,
 * then can get rid of this, but for now need duplicate routine. */
static mINT32 
get_ofst_from_label_ASSOC_INFO (DST_ASSOC_INFO assoc_info)
{
  ST *st;
  if (DST_ASSOC_INFO_st_index(assoc_info) == 0) {
	/* Fortran format statements create dw_tag_labels that point to
	 * constant strings.  If index is 0, this must be such a constant */
	st = &St_Table(
		DST_ASSOC_INFO_st_level(assoc_info),
		DST_ASSOC_INFO_st_index(assoc_info) );
  } else {
	LABEL_IDX lab;
	lab = DST_ASSOC_INFO_st_index(assoc_info);
  	FmtAssert ((lab > 0 && lab <= LABEL_Table_Size(DST_ASSOC_INFO_st_level(assoc_info))), 
	    ("get_ofst_from_label_ASSOC_INFO: bad dst info from fe? (%d,%d)",
		DST_ASSOC_INFO_st_level(assoc_info), 
		DST_ASSOC_INFO_st_index(assoc_info) ));
	return Get_Label_Offset(lab);
  }
  FmtAssert ((st != NULL), 
	("get_ofst_from_label_ASSOC_INFO: bad dst info from fe? (%d,%d)",
		DST_ASSOC_INFO_st_level(assoc_info), 
		DST_ASSOC_INFO_st_index(assoc_info) ));
  return (st != NULL) ? ST_ofst(st) : 0;
}

extern INT
Offset_from_FP (ST *st)
{
  ST *base_st;
  INT64 base_ofst;
  Allocate_Object(st);
  Base_Symbol_And_Offset (st, &base_st, &base_ofst);

  if (base_st == SP_Sym) {
    return (base_ofst - Frame_Len);
  }
  else if (base_st == FP_Sym) {
    return base_ofst;
  }
  else {
    Is_True(FALSE, ("symbol %s is not allocated", ST_name(st)));
    return 0;
  }
}

/* Generate a location expression for the symbol represented by 'assoc_info'.
 * Only symbols that have a memory location have a location expression. For 
 * other cases, we return without doing anything.
 */
static void
put_location (
  DST_ASSOC_INFO assoc_info, 
  INT offs, 
  DST_flag flag,
  Dwarf_P_Die die,
  Dwarf_Half loc_attr)
{
  Dwarf_P_Expr expr;
  ST *st;
  ST *base_st;
#ifndef TARG_ST
  INT64 base_ofst;
#endif
  BOOL deref;

  st = Get_ST_From_DST (assoc_info);
  if (ST_sclass(st) == SCLASS_FORMAL_REF)
	st =	Get_ST_formal_ref_base(st);
  if (st == NULL) return;
  if (ST_is_not_used(st)) return;

#ifdef TARG_ST
  base_st = Base_Symbol (st);
#else
  Base_Symbol_And_Offset (st, &base_st, &base_ofst);
#endif

  if (ST_is_not_used(base_st)) return;	/* For fixing undefined refernece bug, Added By: Mike Murphy, 22 Apr 2001 */

  deref = FALSE;
  if (DST_IS_deref(flag))  /* f90 formals, dope, etc */
	deref = TRUE;

  expr = dwarf_new_expr (dw_dbg, &dw_error);

  if (st == base_st && ST_class(st) != CLASS_BLOCK 
	&& ST_sclass(st) != SCLASS_COMMON && ST_sclass(st) != SCLASS_EXTERN) 
  {
	/* symbol was not allocated, so doesn't have dwarf location */
	return;
  }

  switch (ST_sclass(st)) {
    case SCLASS_FORMAL:
	if (base_st != SP_Sym && base_st != FP_Sym) {
		dwarf_add_expr_addr_b (expr,
				       ST_ofst(st) + offs,
				       Cg_Dwarf_Symtab_Entry(CGD_ELFSYM,
							     EMT_Put_Elf_Symbol(base_st)),
				       &dw_error);
		if (Trace_Dwarf) {
	  		fprintf (TFile,"LocExpr: symbol = %s, offset = %lld\n", 
			      ST_name(base_st), ST_ofst(st) + offs);
		}
		break;
	}
	/* else fall through to the case of stack variables. */

    case SCLASS_AUTO:

        if (DST_IS_base_deref(flag)) { /* f90 formal dope  */

	  dwarf_add_expr_gen (expr, DW_OP_fbreg, Offset_from_FP(st), 
			    0, &dw_error);

	  dwarf_add_expr_gen (expr, DW_OP_deref, 0,0, &dw_error);
	  dwarf_add_expr_gen (expr, DW_OP_plus_uconst, offs, 0, &dw_error);

	} else {

	  dwarf_add_expr_gen (expr, DW_OP_fbreg, Offset_from_FP(st) + offs, 
			      0, &dw_error);
	}
	if (Trace_Dwarf) {
	  fprintf (TFile,"LocExpr: DW_OP_fbreg,  offset = %d\n", 
		   Offset_from_FP(st) + offs);
	}
	break;

    case SCLASS_CPLINIT:
    case SCLASS_EH_REGION:
    case SCLASS_EH_REGION_SUPP:
    case SCLASS_DGLOBAL:
    case SCLASS_UGLOBAL:
    case SCLASS_FSTATIC:
    case SCLASS_PSTATIC:
#ifdef TARG_ST
      if (ST_is_thread_private(st)) {
	// [SC] For thread symbols, use st always, because base_st will not
	// have STT_TLS type if it is a section symbol.
	dwarf_add_expr_addr_b (expr, offs,
			       Cg_Dwarf_Symtab_Entry(CGD_ELFSYM,
						     EMT_Put_Elf_Symbol(st)),
			       &dw_error);
	dwarf_add_expr_gen (expr, DW_OP_GNU_push_tls_address, 0,0, &dw_error);

      }
      // [CL] support variables assigned to a dedicated register
      else if (ST_assigned_to_dedicated_preg(st))
	{
	  PREG_NUM preg = Find_PREG_For_Symbol (st);
	  ISA_REGISTER_CLASS rclass;
	  REGISTER reg;
	  if (CGTARG_Preg_Register_And_Class(preg, &rclass, &reg)) {
	    DebugRegId reg_num = REGISTER_Get_Debug_Reg_Id(rclass, reg);
	    dwarf_add_expr_gen (expr, DW_OP_regx, reg_num,0, &dw_error);
	  } else {
	    DevWarn("Could not find dedicated register for variable %s", ST_name(st));
	  }
	}
      // [CL] output symbol name if it exists, rather than base name:
      // in case attributes have been added, the name of the base
      // make include spurious characters
      else if ( (base_st != NULL) && ((ST_name(st)==NULL) || *(ST_name(st)) == '\0')) {
#else
      if (base_st != NULL) {
#endif
	dwarf_add_expr_addr_b (expr,
			       ST_ofst(st) + offs, 
			       Cg_Dwarf_Symtab_Entry(CGD_ELFSYM,
						     EMT_Put_Elf_Symbol(base_st)),
			       &dw_error);
	if (Trace_Dwarf) {
	  fprintf (TFile,"LocExpr: symbol = %s, offset = %lld\n", 
			      ST_name(base_st), ST_ofst(st) + offs); 
	}
      }
      else {
	dwarf_add_expr_addr_b (expr, offs,
			       Cg_Dwarf_Symtab_Entry(CGD_ELFSYM,
						     EMT_Put_Elf_Symbol(st)),
			       &dw_error);
	if (Trace_Dwarf) {
	  fprintf (TFile,"LocExpr: symbol = %s, offset = %d\n", ST_name(st), offs);
	}
      }
      break;

    case SCLASS_COMMON:
    case SCLASS_EXTERN:
      dwarf_add_expr_addr_b (expr, offs,
			     Cg_Dwarf_Symtab_Entry(CGD_ELFSYM,
						   EMT_Put_Elf_Symbol(st)),
			     &dw_error);

      if (Trace_Dwarf) {
	fprintf (TFile,"LocExpr: symbol = %s, offset = %d\n", ST_name(st), offs);
      }
      break;
    default:
      ErrMsg (EC_Unimplemented, "put_location: sclass");
      return;
  }
  if (deref) {
    dwarf_add_expr_gen (expr, DW_OP_deref, 0, 0, &dw_error);
    if (Trace_Dwarf) {
      fprintf (TFile,"LocExpr: DW_OP_deref\n");
    }
  }
  dwarf_add_AT_location_expr (dw_dbg, die, loc_attr, expr, &dw_error);
  return;
}

static void
put_pc_value_symbolic (Dwarf_Unsigned pc_attr,
		       Dwarf_Unsigned pc_label,
		       Dwarf_Addr     pc_offset,
		       Dwarf_P_Die    die)
{
  dwarf_add_AT_targ_address_b(dw_dbg,
			      die,
			      pc_attr,
			      pc_offset,
			      pc_label,
			      &dw_error);
}

static void
put_pc_value (Dwarf_Unsigned pc_attr, INT32 pc_value, Dwarf_P_Die die)
{
   dwarf_add_AT_targ_address(
      dw_dbg,
      die,
      pc_attr,
      pc_value,
      cur_text_index,
      &dw_error);
}

static void
put_lexical_block(DST_flag flag, DST_LEXICAL_BLOCK *attr, Dwarf_P_Die die)
{
  put_name (DST_LEXICAL_BLOCK_name(attr), die, pb_none);
#if 1
#ifdef TARG_ST
  LABEL_IDX low = (LABEL_IDX) DST_ASSOC_INFO_fe_ptr(DST_LEXICAL_BLOCK_low_pc(attr));
  LABEL_IDX high = (LABEL_IDX) DST_ASSOC_INFO_fe_ptr(DST_LEXICAL_BLOCK_high_pc(attr));
  // [CL] if the lexical block was optimized out (ie labels not
  // created), don't emit low and high bounds
  //  if (!LABEL_name_idx(low) || !LABEL_name_idx(high)) {
  if (!LABEL_emitted(low) || !LABEL_emitted(high)) {
    return;
  }
  put_pc_value_symbolic (DW_AT_low_pc,
			 Cg_Dwarf_Symtab_Entry(CGD_LABIDX,
					       low,
					       cur_text_index),
			 (Dwarf_Addr) 0,
			 die);
  put_pc_value_symbolic (DW_AT_high_pc,
			 Cg_Dwarf_Symtab_Entry(CGD_LABIDX,
					       high,
					       cur_text_index),
			 (Dwarf_Addr) 0,
			 die);
#else
  put_pc_value_symbolic (DW_AT_low_pc,
			 Cg_Dwarf_Symtab_Entry(CGD_LABIDX,
					       (LABEL_IDX) DST_ASSOC_INFO_st_index(DST_LEXICAL_BLOCK_low_pc(attr)),
					       cur_text_index),
			 (Dwarf_Addr) 0,
			 die);
  put_pc_value_symbolic (DW_AT_high_pc,
			 Cg_Dwarf_Symtab_Entry(CGD_LABIDX,
					       (LABEL_IDX) DST_ASSOC_INFO_st_index(DST_LEXICAL_BLOCK_high_pc(attr)),
					       cur_text_index),
			 (Dwarf_Addr) 0,
			 die);
#endif
#else
  put_pc_value (DW_AT_low_pc, 
	get_ofst_from_label_ASSOC_INFO(DST_LEXICAL_BLOCK_low_pc(attr)),
	die);
  put_pc_value (DW_AT_high_pc,
	get_ofst_from_label_ASSOC_INFO(DST_LEXICAL_BLOCK_high_pc(attr)),
	die);
#endif
}

static void
put_inlined_subroutine(DST_INLINED_SUBROUTINE *attr, Dwarf_P_Die die)
{
#if 1
  put_pc_value_symbolic (DW_AT_low_pc,
			 Cg_Dwarf_Symtab_Entry(CGD_LABIDX,
					       (LABEL_IDX) DST_ASSOC_INFO_st_index(DST_LEXICAL_BLOCK_low_pc(attr)),
					       cur_text_index),
			 (Dwarf_Addr) 0,
			 die);
  put_pc_value_symbolic (DW_AT_high_pc,
			 Cg_Dwarf_Symtab_Entry(CGD_LABIDX,
					       (LABEL_IDX) DST_ASSOC_INFO_st_index(DST_LEXICAL_BLOCK_high_pc(attr)),
					       cur_text_index),
			 (Dwarf_Addr) 0,
			 die);
#else
   put_pc_value (DW_AT_low_pc,
	get_ofst_from_label_ASSOC_INFO(DST_INLINED_SUBROUTINE_low_pc(attr)),
	die);
   put_pc_value (DW_AT_high_pc,
	get_ofst_from_label_ASSOC_INFO(DST_INLINED_SUBROUTINE_high_pc(attr)),
	die);
#endif

   if (DST_IS_FOREIGN_OBJ (DST_INLINED_SUBROUTINE_abstract_origin(attr))) {
	/* cross file inlining */
	put_string (
		DST_INLINED_SUBROUTINE_abstract_name(attr), 
		DW_AT_MIPS_abstract_name, 
		die);
	put_decl (DST_INLINED_SUBROUTINE_decl(attr), die);
   }
   else {
	/* same-file inlining */
	put_reference(
		DST_INLINED_SUBROUTINE_abstract_origin(attr),
		DW_AT_abstract_origin,
		die);
   }
}

static void
put_concrete_subprogram (DST_INFO_IDX abstract_idx,
			 INT32        low_pc,
			 INT32        high_pc,
			 Dwarf_P_Die  die)
{
   put_pc_value (DW_AT_low_pc, low_pc, die);
   put_pc_value (DW_AT_high_pc, high_pc, die);
   put_reference( abstract_idx, DW_AT_abstract_origin, die);
}

static void
put_label(DST_flag flag, DST_LABEL *attr, Dwarf_P_Die die)
{
  put_name (DST_LABEL_name(attr), die, pb_none);
  dwarf_add_AT_targ_address_b (dw_dbg, die, DW_AT_low_pc,
	     get_ofst_from_label_ASSOC_INFO(DST_LABEL_low_pc(attr)), 
	     cur_text_index, &dw_error);
}


static void
put_variable(DST_flag flag, DST_VARIABLE *attr, Dwarf_P_Die die)
{
  if (DST_IS_const(flag))  /* Not yet supported */ {
    ErrMsg (EC_Unimplemented, "put_variable: DST_IS_const");
  }
  else if (DST_IS_memdef(flag))  /* Not yet supported */ {
    ErrMsg (EC_Unimplemented, 
	"put_variable: a class member with AT_specification");
  }
  else if (DST_IS_declaration(flag)) {
    put_decl(DST_VARIABLE_decl_decl(attr), die);
    put_name (DST_VARIABLE_decl_name(attr), die, pb_none);
    put_reference (DST_VARIABLE_decl_type(attr), DW_AT_type, die);
    put_flag (DW_AT_declaration, die);
    if (DST_IS_external(flag)) put_flag (DW_AT_external, die);
#ifdef TARG_ST // [CL] support for linkage_name
    put_string (DST_VARIABLE_decl_linkage_name(attr),
		DW_AT_MIPS_linkage_name, die);
#endif
  }
  else if (DST_IS_comm(flag)) { /* definition of a common block variable. */
    put_decl(DST_VARIABLE_comm_decl(attr), die);
    put_name (DST_VARIABLE_comm_name(attr), die, pb_varname);
    put_reference (DST_VARIABLE_comm_type(attr), DW_AT_type, die);
    put_dopetype (DST_VARIABLE_comm_dopetype(attr), flag, die);

    put_location (DST_VARIABLE_comm_st(attr), 
	          (INT) DST_VARIABLE_comm_offs(attr),
		  flag, die, DW_AT_location);
    if (DST_IS_external(flag))  put_flag (DW_AT_external, die);
    if (DST_IS_assumed_size(flag)) put_flag (DW_AT_MIPS_assumed_size, die);
  }
  else /* definition */ {
    which_pb pbtype;
    put_decl(DST_VARIABLE_def_decl(attr), die);
    if (DST_IS_external(flag)) 
      pbtype = pb_pubname;
    else if (!DST_IS_automatic(flag))
      pbtype = pb_varname;
    else
      pbtype = pb_none;
    put_name (DST_VARIABLE_def_name(attr), die, pbtype);
    put_reference (DST_VARIABLE_def_type(attr), DW_AT_type, die);
    put_location (DST_VARIABLE_def_st(attr), 
		  (INT) DST_VARIABLE_def_offs(attr),
		  flag, die, DW_AT_location);
    if (DST_IS_external(flag))  put_flag (DW_AT_external, die);
    if (DST_IS_assumed_size(flag)) put_flag (DW_AT_MIPS_assumed_size, die);
    put_dopetype (DST_VARIABLE_def_dopetype(attr), flag, die);
    /* check if we have a pointer from def to decl (for static data member) */
    if (!DST_IS_NULL(DST_VARIABLE_def_specification(attr)))
       put_reference (DST_VARIABLE_def_specification(attr),
                      DW_AT_specification, die);
    if ( ! DST_IS_FOREIGN_OBJ(DST_VARIABLE_def_abstract_origin(attr))) {
	put_reference(
		DST_VARIABLE_def_abstract_origin(attr),
		DW_AT_abstract_origin,
		die);
    }
#ifdef TARG_ST // [CL] support for linkage_name
    put_string (DST_VARIABLE_def_linkage_name(attr),
		DW_AT_MIPS_linkage_name, die);
#endif
    /* else if is cross-file inlined, will use name for matching */
  }
}

static void
put_constant (DST_flag flag, DST_CONSTANT *attr, Dwarf_P_Die die)
{
   FmtAssert (!DST_IS_declaration(flag), ("put_constant of non-def"));
   put_decl(DST_CONSTANT_def_decl(attr), die);
   put_name (DST_CONSTANT_def_name(attr), die, pb_none);
   put_reference (DST_CONSTANT_def_type(attr), DW_AT_type, die);
   put_const_attribute (DST_CONSTANT_def_cval(attr), DW_AT_const_value, die);
}

static void
put_formal_parameter(DST_flag flag, DST_FORMAL_PARAMETER *attr, Dwarf_P_Die die)
{
  if (DST_IS_declaration(flag)) {
   put_decl(DST_FORMAL_PARAMETER_decl(attr), die);
   put_name (DST_FORMAL_PARAMETER_name(attr), die, pb_none);
#ifdef TARG_ST // [CL] don't forget parameter type
   put_reference (DST_FORMAL_PARAMETER_type(attr), DW_AT_type, die);
#endif
   put_reference (DST_FORMAL_PARAMETER_default_val(attr),
		  DW_AT_default_value, 
	          die);
#ifndef TARG_ST // [CL] no such flag for formal_parameter
   put_flag (DW_AT_declaration, die);
#endif
  } else {

   put_decl(DST_FORMAL_PARAMETER_decl(attr), die);
   put_name (DST_FORMAL_PARAMETER_name(attr), die, pb_none);
   put_reference (DST_FORMAL_PARAMETER_type(attr), DW_AT_type, die);
   put_location (DST_FORMAL_PARAMETER_st(attr), 0, flag, die, DW_AT_location);
   if (DST_IS_optional_parm(flag)) put_flag (DW_AT_is_optional, die);
   if (DST_IS_variable_parm(flag)) put_flag (DW_AT_variable_parameter, die);
   if (DST_IS_assumed_size(flag)) put_flag (DW_AT_MIPS_assumed_size, die);
   put_reference (DST_FORMAL_PARAMETER_default_val(attr),
		  DW_AT_default_value, 
	          die);
   put_dopetype (DST_FORMAL_PARAMETER_dopetype(attr), flag, die);
   if ( ! DST_IS_FOREIGN_OBJ(DST_FORMAL_PARAMETER_abstract_origin(attr))) {
	put_reference(
		DST_FORMAL_PARAMETER_abstract_origin(attr),
		DW_AT_abstract_origin,
		die);
   }
  }
   /* else if is cross-file inlined, will use name for matching */
}


static void
put_unspecified_parameters(
    DST_flag flag, 
    DST_UNSPECIFIED_PARAMETERS *attr,
    Dwarf_P_Die die)
{
  put_decl(DST_UNSPECIFIED_PARAMETERS_decl(attr), die);
  /* TODO abstarct origin. */
}


static void
put_basetype(DST_flag flag, DST_BASETYPE *attr, Dwarf_P_Die die)
{
  put_name (DST_BASETYPE_name(attr), die, pb_none);
  dwarf_add_AT_unsigned_const (dw_dbg, die, DW_AT_encoding,
		DST_BASETYPE_encoding(attr), &dw_error);
  dwarf_add_AT_unsigned_const (dw_dbg, die, DW_AT_byte_size,
		DST_BASETYPE_byte_size(attr), &dw_error);
}


static void
put_const_type(DST_flag flag, DST_CONST_TYPE *attr, Dwarf_P_Die die)
{
  put_reference (DST_CONST_TYPE_type(attr), DW_AT_type, die);
}


static void
put_volatile_type(DST_flag flag, DST_VOLATILE_TYPE *attr, Dwarf_P_Die die)
{
  put_reference (DST_VOLATILE_TYPE_type(attr), DW_AT_type, die);
}


static void
put_pointer_type(DST_flag flag, DST_POINTER_TYPE *attr, Dwarf_P_Die die)
{
  put_reference (DST_POINTER_TYPE_type(attr), DW_AT_type, die);
  dwarf_add_AT_unsigned_const (dw_dbg, die, DW_AT_byte_size,
	      DST_POINTER_TYPE_byte_size(attr), &dw_error);
  dwarf_add_AT_unsigned_const (dw_dbg, die, DW_AT_address_class,
	      DST_POINTER_TYPE_address_class(attr), &dw_error);
}


static void
put_reference_type(DST_flag flag, DST_REFERENCE_TYPE *attr, Dwarf_P_Die die)
{
  put_reference (DST_REFERENCE_TYPE_type(attr), DW_AT_type, die);
  dwarf_add_AT_unsigned_const (dw_dbg, die, DW_AT_byte_size,
	      DST_REFERENCE_TYPE_byte_size(attr), &dw_error);
  dwarf_add_AT_unsigned_const (dw_dbg, die, DW_AT_address_class,
	      DST_REFERENCE_TYPE_address_class(attr), &dw_error);
}


static void
put_typedef (DST_flag flag, DST_TYPEDEF *attr, Dwarf_P_Die die)
{
  put_decl(DST_TYPEDEF_decl(attr), die);
  put_name (DST_TYPEDEF_name(attr), die, pb_typename);
  put_reference (DST_TYPEDEF_type(attr), DW_AT_type, die);
   /* TODO: 
   DST_put_idx_attribute(" abstract_origin",
			 DST_TYPEDEF_abstract_origin(attr), FALSE);
   DST_write_line();
   */
}

static void
put_ptr_to_member_type(DST_flag flag,
		       DST_PTR_TO_MEMBER_TYPE *attr,
		       Dwarf_P_Die die)
{
  put_name(DST_PTR_TO_MEMBER_TYPE_name(attr), die, pb_typename);
  put_reference(DST_PTR_TO_MEMBER_TYPE_type(attr), DW_AT_type, die);
  put_reference(DST_PTR_TO_MEMBER_TYPE_class_type(attr), 
		DW_AT_containing_type, die);
}

static void
put_array_type(DST_flag flag, DST_ARRAY_TYPE *attr, Dwarf_P_Die die)
{
  put_decl(DST_ARRAY_TYPE_decl(attr), die);
  put_name (DST_ARRAY_TYPE_name(attr), die, pb_typename);
  put_reference (DST_ARRAY_TYPE_type(attr), DW_AT_type, die);
  if (DST_ARRAY_TYPE_byte_size(attr) != 0) 
    dwarf_add_AT_unsigned_const (dw_dbg, die, DW_AT_byte_size,
		        DST_ARRAY_TYPE_byte_size(attr), &dw_error);
  if (DST_IS_declaration(flag)) put_flag (DW_AT_declaration, die);
  /* TODO: 
   DST_put_idx_attribute(" abstract_origin",
			 DST_ARRAY_TYPE_abstract_origin(attr), FALSE);
  */
}


/* write out the AT_lower_bound attribute. If the bound is a constant and
   is a default value for the language, do not emit the attribute.
*/
static void
put_lower_bound (DST_flag flag, DST_SUBRANGE_TYPE *attr, Dwarf_P_Die die)
{
  DST_bounds_t cval;

  if (DST_IS_lb_cval(flag)) {
    cval = DST_SUBRANGE_TYPE_lower_cval(attr);
    switch (Dwarf_Language) {
      case DW_LANG_C89:
      case DW_LANG_C:
      case DW_LANG_C_plus_plus:
	if (cval == 0) return;
	break;
      case DW_LANG_Fortran77:
      case DW_LANG_Fortran90:
	if (cval == 1) return;
	break;
      default:
	break;
    }
    dwarf_add_AT_signed_const (dw_dbg, die, DW_AT_lower_bound, cval, &dw_error);
  }
  else {
    put_reference (DST_SUBRANGE_TYPE_lower_ref(attr), DW_AT_lower_bound, die);
  }
}


static void
put_subrange_type(DST_flag flag, DST_SUBRANGE_TYPE *attr, Dwarf_P_Die die)
{

  Dwarf_Half att ;

  put_lower_bound (flag, attr, die);

  att = DW_AT_upper_bound;
  if (DST_IS_count(flag))
    att = DW_AT_count;
    
#ifdef TARG_ST
  if(!DST_IS_NULL(DST_SUBRANGE_TYPE_type(attr)))
      {
          put_reference (DST_SUBRANGE_TYPE_type(attr), DW_AT_type, die);
      }
  if (DST_IS_count(flag)) {
    dwarf_add_AT_unsigned_const (dw_dbg, die, att,
		       DST_SUBRANGE_TYPE_count_val(attr), &dw_error);
  } else {
#endif
  if (DST_IS_ub_cval(flag)) {
    dwarf_add_AT_signed_const (dw_dbg, die, att,
		       DST_SUBRANGE_TYPE_upper_cval(attr), &dw_error);
  }
  else {
    put_reference (DST_SUBRANGE_TYPE_upper_ref(attr), att, die);
  }
#ifdef TARG_ST
  }
#endif

  /* stride provided if descriptor of (possibly) non-contiguous object */

  if (DST_IS_stride_1byte(flag)) 
    att = DW_AT_MIPS_stride_byte;

  else if (DST_IS_stride_2byte(flag))
    att = DW_AT_MIPS_stride_elem;

  else
    att = DW_AT_MIPS_stride;

  put_reference (DST_SUBRANGE_TYPE_stride_ref(attr),att, die);

}


static void
put_structure_type(DST_flag flag, DST_STRUCTURE_TYPE *attr, Dwarf_P_Die die)
{
  put_decl(DST_STRUCTURE_TYPE_decl(attr), die);
  put_name (DST_STRUCTURE_TYPE_name(attr), die, pb_typename);
#ifdef TARG_ST
  // [CL] according to the Dwarf standard, incomplete structure, union
  // or class type is represented by a structure, union or class entry
  // that does not have a byte size attribute and that has a
  // DW_AT_declaration attribute. GDB expects this for enums too.

  // We can have 0 sized structs, so check the declaration flag to
  // decide whether to generate size information.
  if (!DST_IS_declaration(flag))
#endif
  dwarf_add_AT_unsigned_const (dw_dbg, die, DW_AT_byte_size,
		  DST_STRUCTURE_TYPE_byte_size(attr), &dw_error);
  if (DST_IS_declaration(flag)) put_flag (DW_AT_declaration, die);
/* TODO: 
   DST_put_idx_attribute(" abstract_origin",
			 DST_STRUCTURE_TYPE_abstract_origin(attr), FALSE);
*/
#ifdef TARG_ST // [CL] add containing_type if any
  put_reference (DST_STRUCTURE_TYPE_containing_type(attr),
		 DW_AT_containing_type, die);
#endif
}


static void
put_class_type(DST_flag flag, DST_CLASS_TYPE *attr, Dwarf_P_Die die)
{
  put_decl(DST_CLASS_TYPE_decl(attr), die);
  put_name (DST_CLASS_TYPE_name(attr), die, 
      DST_IS_declaration(flag) ? pb_none : pb_typename);
#ifdef TARG_ST
  // [CL] according to the Dwarf standard, incomplete structure, union
  // or class type is represented by a structure, union or class entry
  // that does not have a byte size attribute and that has a
  // DW_AT_declaration attribute. GDB expects this for enums too.

  // We can have 0 sized structs, so check the declaration flag too
  if ( (DST_STRUCTURE_TYPE_byte_size(attr) != 0) 
       && (!DST_IS_declaration(flag)) )
#endif
  dwarf_add_AT_unsigned_const (dw_dbg, die, DW_AT_byte_size,
		  DST_CLASS_TYPE_byte_size(attr), &dw_error);
  if (DST_IS_declaration(flag)) put_flag (DW_AT_declaration, die);
/* TODO: 
   DST_put_idx_attribute(" abstract_origin",
			 DST_CLASS_TYPE_abstract_origin(attr), FALSE);
*/
}


static void
put_union_type(DST_flag flag, DST_UNION_TYPE *attr, Dwarf_P_Die die)
{
  put_decl(DST_UNION_TYPE_decl(attr), die);
  put_name (DST_UNION_TYPE_name(attr), die, pb_typename);
#ifdef TARG_ST
  // [CL] according to the Dwarf standard, incomplete structure, union
  // or class type is represented by a structure, union or class entry
  // that does not have a byte size attribute and that has a
  // DW_AT_declaration attribute. GDB expects this for enums too.

  // We can have 0 sized structs, so check the declaration flag too
  if ( (DST_STRUCTURE_TYPE_byte_size(attr) != 0) 
       && (!DST_IS_declaration(flag)) )
#endif
  dwarf_add_AT_unsigned_const (dw_dbg, die, DW_AT_byte_size,
		  DST_UNION_TYPE_byte_size(attr), &dw_error);
  if (DST_IS_declaration(flag)) put_flag (DW_AT_declaration, die);
/* TODO:
   DST_put_idx_attribute(" abstract_origin",
			 DST_UNION_TYPE_abstract_origin(attr), FALSE);
*/
#ifdef TARG_ST // [CL] add containing_type if any
  put_reference (DST_STRUCTURE_TYPE_containing_type(attr),
		 DW_AT_containing_type, die);
#endif
}


static void
put_member(DST_flag flag, DST_MEMBER *attr, Dwarf_P_Die die)
{
  Dwarf_P_Expr expr;

  put_decl(DST_MEMBER_decl(attr), die);
  put_name (DST_MEMBER_name(attr), die, pb_none);
  put_reference (DST_MEMBER_type(attr), DW_AT_type, die);
  if (DST_IS_bitfield(flag)) {
    dwarf_add_AT_unsigned_const (dw_dbg, die, DW_AT_byte_size,
		    DST_MEMBER_byte_size(attr), &dw_error);
    dwarf_add_AT_unsigned_const (dw_dbg, die, DW_AT_bit_offset,
		    DST_MEMBER_bit_offset(attr), &dw_error);
    dwarf_add_AT_unsigned_const (dw_dbg, die, DW_AT_bit_size,
		    DST_MEMBER_bit_size(attr), &dw_error);
  }
  put_dopetype (DST_MEMBER_dopetype(attr), flag, die);
#ifdef TARG_ST
  // [CL]
  if (DST_MEMBER_accessibility(attr) != DW_ACCESS_public) {
    dwarf_add_AT_unsigned_const(dw_dbg, die, DW_AT_accessibility,
				DST_MEMBER_accessibility(attr),
				&dw_error);
  }
#endif
  if (DST_IS_declaration(flag)) {
	put_flag (DW_AT_declaration, die);
  }
  else {
	/* C++ static members don't have location;
	 * decl and static flags are set at same time,
	 * so use decl flag. */
  	/* For now, assume that the member location is always a constant. */
  	expr = dwarf_new_expr (dw_dbg, &dw_error);
  	dwarf_add_expr_gen (expr, DW_OP_consts, DST_MEMBER_memb_loc(attr), 0, 
		&dw_error);
#if pv292951
	/* according to the spec, we should do this, but dbx doesn't like it */
	dwarf_add_expr_gen (expr, DW_OP_plus, 0, 0, &dw_error);
#endif

  	if (expr != NULL) {

	  if (Dwarf_Language == DW_LANG_Fortran90)
	    if (DST_IS_deref(flag)) 
	      dwarf_add_expr_gen (expr, DW_OP_deref, 0, 0, &dw_error);	  

	  dwarf_add_AT_location_expr (dw_dbg, die, 
				      DW_AT_data_member_location, expr, &dw_error);
	}
  }
}


static void
put_template_type_param(DST_flag flag, DST_TEMPLATE_TYPE_PARAMETER *attr,
			Dwarf_P_Die die)
{
  put_name (DST_TEMPLATE_TYPE_PARAMETER_name(attr), die, pb_none);
  put_reference (DST_TEMPLATE_TYPE_PARAMETER_type(attr), DW_AT_type, die);
}

static void
put_template_value_param(DST_flag flag, DST_TEMPLATE_VALUE_PARAMETER *attr,
			 Dwarf_P_Die die)
{
  put_reference (DST_TEMPLATE_VALUE_PARAMETER_type(attr), DW_AT_type, die);
  put_name (DST_TEMPLATE_VALUE_PARAMETER_name(attr), die, pb_none);
  put_const_attribute (
	  DST_TEMPLATE_VALUE_PARAMETER_cval(attr), DW_AT_const_value, die);
}


#ifdef TARG_ST // [CL] code fragment taken from gcc's dwarf2out.c
/* Return a location descriptor that designates a constant.  */

static int
int_loc_descriptor (INT i)
{
  int op;

  /* Pick the smallest representation of a constant, rather than just
     defaulting to the LEB encoding.  */
  if (i >= 0)
    {
      if (i <= 31)
	op = DW_OP_lit0 + i;
      else if (i <= 0xff)
	op = DW_OP_const1u;
      else if (i <= 0xffff)
	op = DW_OP_const2u;
      else if (i <= 0xffffffff)
	op = DW_OP_const4u;
      else
	op = DW_OP_constu;
    }
  else
    {
      if (i >= -0x80)
	op = DW_OP_const1s;
      else if (i >= -0x8000)
	op = DW_OP_const2s;
      else if ( i >= -0x80000000)
	op = DW_OP_const4s;
      else
	op = DW_OP_consts;
    }

  return op;
}
#endif

static void
put_inheritance(DST_flag flag, DST_INHERITANCE *attr, Dwarf_P_Die die)
{
  Dwarf_P_Expr expr;

  put_reference (DST_INHERITANCE_type(attr), DW_AT_type, die);
  expr = dwarf_new_expr (dw_dbg, &dw_error);
#ifdef TARG_ST // [CL]
  if (DST_INHERITANCE_virtual_offset(attr)) {
    dwarf_add_expr_gen (expr, DW_OP_dup, 0, 0, &dw_error);
    dwarf_add_expr_gen (expr, DW_OP_deref, 0, 0, &dw_error);
    dwarf_add_expr_gen (expr,
			int_loc_descriptor(DST_INHERITANCE_virtual_offset(attr)),
			DST_INHERITANCE_virtual_offset(attr), 0, &dw_error);
    dwarf_add_expr_gen (expr, DW_OP_minus, 0, 0, &dw_error);
    dwarf_add_expr_gen (expr, DW_OP_deref, 0, 0, &dw_error);
    dwarf_add_expr_gen (expr, DW_OP_plus, 0, 0, &dw_error);
  }
  else
#endif
  dwarf_add_expr_gen (expr, DW_OP_consts, DST_INHERITANCE_memb_loc(attr), 0, 
	&dw_error);
  if (expr != NULL) {
    dwarf_add_AT_location_expr (dw_dbg, die, DW_AT_data_member_location, 
	expr, &dw_error);
  if (DST_INHERITANCE_virtuality(attr) != DW_VIRTUALITY_none)
    dwarf_add_AT_unsigned_const(dw_dbg, die, DW_AT_virtuality,
			        DST_INHERITANCE_virtuality(attr),
				&dw_error);
  }
#ifdef TARG_ST
  // [CL]
  if (DST_INHERITANCE_accessibility(attr) != DW_ACCESS_private) {
    dwarf_add_AT_unsigned_const(dw_dbg, die, DW_AT_accessibility,
				DST_INHERITANCE_accessibility(attr),
				&dw_error);
  }
#endif
}

static void
put_enumeration_type(DST_flag flag, DST_ENUMERATION_TYPE *attr, Dwarf_P_Die die)
{
  put_decl(DST_ENUMERATION_TYPE_decl(attr), die);
  put_name (DST_ENUMERATION_TYPE_name(attr), die, pb_typename);
#ifdef TARG_ST
  // [CL] according to the Dwarf standard, incomplete structure, union
  // or class type is represented by a structure, union or class entry
  // that does not have a byte size attribute and that has a
  // DW_AT_declaration attribute. GDB expects this for enums too.

  // We can have 0 sized structs, so check the declaration flag too
  if ( (DST_STRUCTURE_TYPE_byte_size(attr) != 0) 
       && (!DST_IS_declaration(flag)) )
#endif
  dwarf_add_AT_unsigned_const (dw_dbg, die, DW_AT_byte_size,
		  DST_ENUMERATION_TYPE_byte_size(attr), &dw_error);
  if (DST_IS_declaration(flag)) put_flag (DW_AT_declaration, die);
/* TODO: 
   DST_put_idx_attribute(" abstract_origin",
			 DST_ENUMERATION_TYPE_abstract_origin(attr), FALSE);
*/
}


static void
put_enumerator(DST_flag flag, DST_ENUMERATOR *attr, Dwarf_P_Die die)
{
  put_decl(DST_ENUMERATOR_decl(attr), die);
  put_name (DST_ENUMERATOR_name(attr), die, pb_typename);
#ifndef TARG_ST
  put_const_attribute (DST_ENUMERATOR_cval(attr), DW_AT_const_value, die);
#else
  // [CL] encode enumerator values in LEB128 because GDB assumes
  // values are signed (encoding with DATA1, DATA2, DATA4 or DATA8
  // means that large unsigned values would appear to have negative
  // values in the debugger)
  DST_CONST_VALUE cval = DST_ENUMERATOR_cval(attr);
  switch (DST_CONST_VALUE_form(cval)) {
    case DST_FORM_DATA1:
      dwarf_add_AT_const_value_signedint (die,
					  DST_CONST_VALUE_form_data1(cval),
					  &dw_error);
      break;
    case DST_FORM_DATA2:
      dwarf_add_AT_const_value_signedint (die,
					  DST_CONST_VALUE_form_data2(cval),
					  &dw_error);
      break;
    case DST_FORM_DATA4:
      dwarf_add_AT_const_value_signedint (die,
					  DST_CONST_VALUE_form_data4(cval),
					  &dw_error);
      break;
    case DST_FORM_DATA8:
      dwarf_add_AT_const_value_signedint (die,
					  DST_CONST_VALUE_form_data8(cval),
					  &dw_error);
      break;
  }
#endif
}


static void
put_subroutine_type(DST_flag flag, DST_SUBROUTINE_TYPE *attr, Dwarf_P_Die die)
{
  put_decl(DST_SUBROUTINE_TYPE_decl(attr), die);
  put_name (DST_SUBROUTINE_TYPE_name(attr), die, pb_typename);
  put_reference (DST_SUBROUTINE_TYPE_type(attr), DW_AT_type, die);
  if (DST_IS_prototyped(flag)) put_flag (DW_AT_prototyped, die);
/* TODO: 
   DST_put_idx_attribute(" abstract_origin",
			 DST_SUBROUTINE_TYPE_abstract_origin(attr), FALSE);
*/
}

static void
put_entry_point (DST_flag flag, DST_ENTRY_POINT *attr, Dwarf_P_Die die)
{
  put_decl (DST_ENTRY_POINT_decl(attr), die);
  put_name (DST_ENTRY_POINT_name(attr), die, pb_pubname);
  put_reference (DST_ENTRY_POINT_type(attr), DW_AT_type, die);
  dwarf_add_AT_targ_address_b (dw_dbg, die, DW_AT_low_pc, 0,
			       Cg_Dwarf_Symtab_Entry(CGD_ELFSYM,
						     get_elfindex_from_ASSOC_INFO (DST_ENTRY_POINT_st(attr))),
			       &dw_error);
}

static void
put_common_block (DST_flag flag, DST_COMMON_BLOCK *attr, Dwarf_P_Die die)
{
  put_name (DST_COMMON_BLOCK_name(attr), die, pb_none);
  put_location (DST_COMMON_BLOCK_st(attr), 0, flag, die, DW_AT_location);
}

static void
put_common_inclusion (DST_flag flag, DST_COMMON_INCL *attr, Dwarf_P_Die die)
{
  put_decl (DST_COMMON_INCL_decl(attr), die);
  put_reference (DST_COMMON_INCL_com_blk(attr), DW_AT_common_reference, die);
}

static void 
put_string_type (DST_flag flag, DST_STRING_TYPE *attr, Dwarf_P_Die die)
{
  DST_INFO_IDX ref_idx;
  DST_INFO *info;
  DST_VARIABLE *vattr;

  put_decl (DST_STRING_TYPE_decl(attr), die);
  put_name (DST_STRING_TYPE_name(attr), die, pb_none);
  if (DST_IS_cval (flag)) {
    dwarf_add_AT_unsigned_const (dw_dbg, die, DW_AT_byte_size,
		DST_STRING_TYPE_len_cval(attr), &dw_error);
  }
  else {
    /* In this case, the string length is in a variable, whose location
       expression is to be emitted as the string_length attribute. */
    ref_idx = DST_STRING_TYPE_len_ref(attr);
    if (DST_IS_NULL(ref_idx)) return;
    info = DST_INFO_IDX_TO_PTR(ref_idx);
    vattr = DST_ATTR_IDX_TO_PTR(DST_INFO_attributes(info), DST_VARIABLE);
    put_location (DST_VARIABLE_def_st(vattr), 0, flag, die, DW_AT_string_length);
  }
}

static void
Write_Attributes (
    DST_DW_tag   tag,
    DST_flag     flag,
    DST_ATTR_IDX iattr,
    Dwarf_P_Die  die)
{
  switch (tag) {
    case DW_TAG_compile_unit:
      put_compile_unit(DST_ATTR_IDX_TO_PTR(iattr, DST_COMPILE_UNIT), die);
      dwarf_add_die_to_debug (dw_dbg, die, &dw_error);
      break;
    case DW_TAG_subprogram:
      put_subprogram(flag,
		     DST_ATTR_IDX_TO_PTR(iattr, DST_SUBPROGRAM),
		     die);
      break;
    case DW_TAG_lexical_block:
      put_lexical_block(flag, 
			DST_ATTR_IDX_TO_PTR(iattr, DST_LEXICAL_BLOCK), 
			die);
      break;
    case DW_TAG_inlined_subroutine:
      put_inlined_subroutine(DST_ATTR_IDX_TO_PTR(iattr, DST_INLINED_SUBROUTINE),
			     die);
      break;
    case DW_TAG_label:
      put_label(flag, DST_ATTR_IDX_TO_PTR(iattr, DST_LABEL), die);
      break;
    case DW_TAG_variable:
      put_variable(flag, DST_ATTR_IDX_TO_PTR(iattr, DST_VARIABLE), die);
      break;
    case DW_TAG_formal_parameter:
      put_formal_parameter(
         flag, DST_ATTR_IDX_TO_PTR(iattr, DST_FORMAL_PARAMETER), die);
      break;
    case DW_TAG_constant:
      put_constant( flag, DST_ATTR_IDX_TO_PTR(iattr, DST_CONSTANT), die);
      break;
    case DW_TAG_unspecified_parameters:
      put_unspecified_parameters(
	 flag, 
	 DST_ATTR_IDX_TO_PTR(iattr, DST_UNSPECIFIED_PARAMETERS), die);
      break;
    case DW_TAG_base_type:
      put_basetype(flag, DST_ATTR_IDX_TO_PTR(iattr, DST_BASETYPE), die);
      break;
    case DW_TAG_const_type:
      put_const_type(flag, DST_ATTR_IDX_TO_PTR(iattr, DST_CONST_TYPE), die);
      break;
    case DW_TAG_volatile_type:
      put_volatile_type(flag, 
		DST_ATTR_IDX_TO_PTR(iattr, DST_VOLATILE_TYPE), die);
      break;
    case DW_TAG_pointer_type:
      put_pointer_type(flag, DST_ATTR_IDX_TO_PTR(iattr, DST_POINTER_TYPE), die);
      break;
    case DW_TAG_reference_type:
      put_reference_type(flag, 
			 DST_ATTR_IDX_TO_PTR(iattr, DST_REFERENCE_TYPE) ,die);
      break;
    case DW_TAG_typedef:
      put_typedef(flag, DST_ATTR_IDX_TO_PTR(iattr, DST_TYPEDEF), die);
      break;
    case DW_TAG_ptr_to_member_type:
      put_ptr_to_member_type(flag,
			     DST_ATTR_IDX_TO_PTR(iattr,
						 DST_PTR_TO_MEMBER_TYPE),
			     die);
      break;
    case DW_TAG_array_type:
      put_array_type(flag, DST_ATTR_IDX_TO_PTR(iattr, DST_ARRAY_TYPE), die);
      break;
    case DW_TAG_subrange_type:
      put_subrange_type(flag, 
			DST_ATTR_IDX_TO_PTR(iattr, DST_SUBRANGE_TYPE), die);
      break;
    case DW_TAG_structure_type:
      put_structure_type(flag, 
			 DST_ATTR_IDX_TO_PTR(iattr, DST_STRUCTURE_TYPE), die);
      break;
    case DW_TAG_class_type:
      put_class_type(flag, 
			 DST_ATTR_IDX_TO_PTR(iattr, DST_CLASS_TYPE), die);
      break;
    case DW_TAG_union_type:
      put_union_type(flag, DST_ATTR_IDX_TO_PTR(iattr, DST_UNION_TYPE), die);
      break;
    case DW_TAG_member:
      put_member(flag, DST_ATTR_IDX_TO_PTR(iattr, DST_MEMBER), die);
      break;
    case DW_TAG_inheritance:
      put_inheritance(flag, DST_ATTR_IDX_TO_PTR(iattr, DST_INHERITANCE), die);
      break;
    case DW_TAG_template_type_param:
      put_template_type_param (
	 flag,
	 DST_ATTR_IDX_TO_PTR(iattr, DST_TEMPLATE_TYPE_PARAMETER), die);
      break;
    case DW_TAG_template_value_param:
      put_template_value_param (
	 flag,
	 DST_ATTR_IDX_TO_PTR(iattr, DST_TEMPLATE_VALUE_PARAMETER), die);
      break;
    case DW_TAG_enumeration_type:
      put_enumeration_type(
         flag, 
	 DST_ATTR_IDX_TO_PTR(iattr, DST_ENUMERATION_TYPE), die);
      break;
    case DW_TAG_enumerator:
      put_enumerator(flag, DST_ATTR_IDX_TO_PTR(iattr, DST_ENUMERATOR), die);
      break;
    case DW_TAG_subroutine_type:
      put_subroutine_type(flag, 
	  DST_ATTR_IDX_TO_PTR(iattr, DST_SUBROUTINE_TYPE), die);
      break;
    case DW_TAG_entry_point:
      put_entry_point (flag, 
	  DST_ATTR_IDX_TO_PTR(iattr, DST_ENTRY_POINT), die);
      break;
    case DW_TAG_common_block:
      put_common_block (flag, 
	  DST_ATTR_IDX_TO_PTR(iattr, DST_COMMON_BLOCK), die);
      break;
    case DW_TAG_common_inclusion:
      put_common_inclusion (flag, 
	  DST_ATTR_IDX_TO_PTR(iattr, DST_COMMON_INCL), die);
      break;
    case DW_TAG_string_type:
      put_string_type (flag, 
	  DST_ATTR_IDX_TO_PTR(iattr, DST_STRING_TYPE), die);
    break;
      
   default:
      ErrMsg (EC_Unimplemented, "Write_Attributes: TAG not handled");
      break;
  }
  if (DST_IS_artificial(flag)) 
      put_flag(DW_AT_artificial, die);
}


/* Add 'die' to the CGD_enclosing_proc array at position 'level'. */
static void Set_Enclosing_Die (Dwarf_P_Die die, mINT32 level)
{
  /* see if we need to increase the size of our enclosing-proc table.  */
  if ( level >= CGD_enclosing_proc_max ) {
    /* get some minimal # more than necessary */
    CGD_enclosing_proc_max = level + 8;

    if ( CGD_enclosing_proc == NULL ) {
      CGD_enclosing_proc = 
	  (Dwarf_P_Die *) calloc( CGD_enclosing_proc_max, 
				  sizeof(*CGD_enclosing_proc) );
    }
    else {
      CGD_enclosing_proc = 
	  (Dwarf_P_Die *) realloc( CGD_enclosing_proc,
	    CGD_enclosing_proc_max * sizeof(*CGD_enclosing_proc) );
    }
  }
  CGD_enclosing_proc[level] = die;
}


/* Do a preorder traversal of the DST node 'idx' and all its children. The
 * 'parent' and/or 'left_sibling' parameters indicate where in the 
 * debug_info tree the die for idx should be attached. The 'tree_level'
 * parameter indicates the level for the 'idx' node in the DST tree.
 */
static Dwarf_P_Die
preorder_visit (
  DST_INFO_IDX idx, 
  Dwarf_P_Die parent, 
  Dwarf_P_Die left_sibling,
  INT tree_level,
  BOOL visit_children)
{
  DST_INFO *info;
  DST_INFO_IDX child_idx;
  Dwarf_P_Die die;
  DST_DW_tag tag;
  DST_flag flag;

  info = DST_INFO_IDX_TO_PTR (idx);
  tag = DST_INFO_tag (info);
  flag = DST_INFO_flag (info);
  die = (Dwarf_P_Die) DST_INFO_dieptr (info);

  /* check if a die has already been generated for the DST. This can 
   * happen, if it was a type die that was referenced earlier. 
   * However, we still need to link in the die at the right place and 
   * write out its attributes.
   */
  if (die == NULL) {
    die = dwarf_new_die (dw_dbg, tag, parent, NULL, left_sibling, NULL, 
			    &dw_error);
    DST_INFO_dieptr(info) = die;
    if (Trace_Dwarf) {
      fprintf (TFile,"NEW die for [%d,%d]: %p, parent: %p, left_sibling:%p, tag:%d\n",
	idx.block_idx, idx.byte_idx, die, parent, left_sibling, tag);
    }
  }
  else {
    dwarf_die_link (die, parent, NULL, left_sibling, NULL, &dw_error);
    if (Trace_Dwarf) {
      fprintf (TFile,"link die for [%d,%d]: %p, parent: %p, left_sibling:%p, tag:%d\n",
	idx.block_idx, idx.byte_idx, die, parent, left_sibling, tag);
    }
  }

  //Current_Tree_Level = tree_level;
  Write_Attributes (tag,
		   flag,
		   DST_INFO_attributes(info),
		   die);
  if (tag == DW_TAG_inlined_subroutine && visit_children) {
	/* if inlined and no body left, then don't visit children */
	ST *st;
	DST_INFO_IDX ref_idx = DST_INLINED_SUBROUTINE_abstract_origin(
		DST_ATTR_IDX_TO_PTR(
		    DST_INFO_attributes(info), DST_INLINED_SUBROUTINE));
        if (!DST_IS_NULL(ref_idx) && !DST_IS_FOREIGN_OBJ (ref_idx)) {
  		info = DST_INFO_IDX_TO_PTR (ref_idx);
  		st = Get_ST_From_DST(DST_SUBPROGRAM_def_st(
			DST_ATTR_IDX_TO_PTR(
			    DST_INFO_attributes(info), DST_SUBPROGRAM)));
  		if (st != NULL && ST_is_not_used(st)) {
			visit_children = FALSE;
		}
	}
  }
 
  if (!visit_children) return die;
  /* Now, visit each child */
  parent = die;
  left_sibling = NULL;
  for (child_idx = DST_first_child (idx);
       !DST_IS_NULL(child_idx); 
       child_idx = DST_INFO_sibling(DST_INFO_IDX_TO_PTR(child_idx)))
  {
    info = DST_INFO_IDX_TO_PTR (child_idx);
    if (DST_INFO_tag(info) != DW_TAG_subprogram
	|| DST_IS_declaration(DST_INFO_flag(info)) )
    {
      left_sibling = 
        preorder_visit (child_idx, parent, left_sibling, tree_level + 1, visit_children);

      /* after the first child is visited, the parent is set to NULL */
      parent = NULL;
    }
  }
  return die;
}

#ifdef TARG_ST
// [CL]
/* Traverse the DST node 'idx' and all its children, in order to check
 * whether any children was referenced. If it is the case, then 'idx'
 * debug info needs to be generated
 */
static BOOL
postorder_visit (DST_INFO_IDX idx)
{
  DST_INFO *info;
  DST_INFO_IDX child_idx;
  Dwarf_P_Die die;
  DST_DW_tag tag;
  DST_flag flag;

  info = DST_INFO_IDX_TO_PTR (idx);
  tag = DST_INFO_tag (info);
  flag = DST_INFO_flag (info);
  die = (Dwarf_P_Die) DST_INFO_dieptr (info);

  BOOL is_referenced = (die != NULL);
  if (Trace_Dwarf) {
    fprintf (TFile,"[%d,%d] is %slocally referenced (die=%p), tag:%d\n",
	     idx.block_idx, idx.byte_idx, is_referenced ? "" : "not ",
	     die, tag);
  }
  for (child_idx = DST_first_child (idx);
       !DST_IS_NULL(child_idx); 
       child_idx = DST_INFO_sibling(DST_INFO_IDX_TO_PTR(child_idx)))
  {
    info = DST_INFO_IDX_TO_PTR (child_idx);
    if (DST_INFO_tag(info) != DW_TAG_subprogram
	|| DST_IS_declaration(DST_INFO_flag(info)) )
    {
      is_referenced |= postorder_visit (child_idx);
    }
  }

  if (Trace_Dwarf) {
    fprintf (TFile,"[%d,%d] is %sreferenced (%semission needed)\n",
	     idx.block_idx, idx.byte_idx,
	     is_referenced ? "" : "not ",
	     is_referenced ? "" : "no ");
  }
  return is_referenced;
}
#endif

#ifdef TARG_ST
// [CL]
/* Check if 'idx' is a type, and if so, check if the types it
   references (if any) have been emitted. If it is the case, 'idx'
   should be emitted even though it was not used by the emitted
   code. It may be used by the debugger.
 */
static BOOL
type_with_emitted_refs (DST_INFO_IDX idx)
{
  DST_INFO *info;
  Dwarf_P_Die die;
  DST_DW_tag tag;
  DST_ATTR_IDX attr;

  info = DST_INFO_IDX_TO_PTR (idx);
  tag = DST_INFO_tag (info);
  die = (Dwarf_P_Die) DST_INFO_dieptr (info);
  attr = DST_INFO_attributes(info);

  DST_INFO *ref_info;
  Dwarf_P_Die ref_die;
  DST_INFO_IDX ref_idx;

  switch (tag) {
  case DW_TAG_pointer_type:
    ref_idx = DST_POINTER_TYPE_type(DST_ATTR_IDX_TO_PTR(attr,
							DST_POINTER_TYPE));
    break;
  case DW_TAG_const_type:
    ref_idx = DST_CONST_TYPE_type(DST_ATTR_IDX_TO_PTR(attr, DST_CONST_TYPE));
    break;
  case DW_TAG_volatile_type:
    ref_idx = DST_VOLATILE_TYPE_type(DST_ATTR_IDX_TO_PTR(attr,
							 DST_VOLATILE_TYPE));
    break;
  case DW_TAG_reference_type:
    ref_idx = DST_REFERENCE_TYPE_type(DST_ATTR_IDX_TO_PTR(attr,
							  DST_REFERENCE_TYPE));
    break;
  case DW_TAG_typedef:
    ref_idx = DST_TYPEDEF_type(DST_ATTR_IDX_TO_PTR(attr, DST_TYPEDEF));
    break;
  case DW_TAG_array_type:
    ref_idx = DST_ARRAY_TYPE_type(DST_ATTR_IDX_TO_PTR(attr, DST_ARRAY_TYPE));
    break;
  case DW_TAG_subroutine_type:
    ref_idx = DST_SUBROUTINE_TYPE_type(DST_ATTR_IDX_TO_PTR(attr,
							   DST_SUBROUTINE_TYPE));
    break;
  default:
    return FALSE;
  }

  if (DST_IS_NULL(ref_idx)) {
    /* [CL] for instance, handle pointer to void */
    return TRUE;
  }

  /* Check if ref_idx's die has been generated */
  ref_info = DST_INFO_IDX_TO_PTR(ref_idx);
  ref_die = (Dwarf_P_Die) DST_INFO_dieptr (info);
  if (ref_die) {
    return TRUE;
  }

  return FALSE;
}

struct is_named_symbol {
private:
  const char *name;
public:
  is_named_symbol(const char *nm) : name(nm) {}
  BOOL operator()(UINT32, const ST *st) const {
    return (ST_class (st) != CLASS_CONST
	    && ! strcmp(name, ST_name (st)));
  }
};

static ST *
find_global_symbol_by_name (const char *name)
{
  ST_IDX st_idx = For_all_until (St_Table, GLOBAL_SYMTAB,
				 is_named_symbol (name));
  if (st_idx != 0)
    return &St_Table[st_idx];
  else
    return NULL;
}
#endif

/* traverse all DSTs and handle the non-pu info */
static void
Traverse_Global_DST (void)
{
  DST_INFO_IDX idx;
  DST_INFO *info;
  Dwarf_P_Die parent;

  if (Trace_Dwarf) {
	fprintf(TFile, "Trace Global DST\n");
  }

#ifdef TARG_ST
  // [CL] Only emit 'necessary' info:
  // only emit types if they are referred to;
  // thus we keep adding debug info until no
  // new reference is generated.
  BOOL found_new_ref = TRUE;

  while (found_new_ref) {
    found_new_ref = FALSE;
#endif

  /* visit subsequent siblings at this level */
  for (idx = DST_first_child (cu_idx);	/* start at beginning */
       !DST_IS_NULL(idx);
       idx = DST_INFO_sibling(DST_INFO_IDX_TO_PTR(idx)))
  {
    info = DST_INFO_IDX_TO_PTR (idx);

#ifdef TARG_ST
    // [CL] Skip this DST if it has already been
    // handled in a previous pass
    if (DST_IS_info_mark(DST_INFO_flag(info))) {
      continue;	/* already traversed */
      }
#endif

    /* only traverse non-subp's */
    if (DST_INFO_tag(info) == DW_TAG_subprogram
      || DST_INFO_tag(info) == DW_TAG_entry_point)
      continue;	/* will traverse later */

    /* All DST entries other than subprograms are attached as children
     * of the compile_unit die.
     */
    parent = CGD_enclosing_proc[GLOBAL_LEVEL];
#ifdef TARG_ST
    if (postorder_visit (idx) || DST_INFO_tag(info) == DW_TAG_variable
	|| type_with_emitted_refs(idx)) {
#endif
    (void) preorder_visit (idx, parent, NULL, LOCAL_LEVEL, TRUE /* visit children */);
    DST_SET_info_mark(DST_INFO_flag(info));	/* mark has been traversed */

#ifdef TARG_ST
    // [CL] keep on looping
      found_new_ref = TRUE;
    }
  }
#endif
  }
}

/* traverse all DSTs and handle anything we missed */
static void
Traverse_Extra_DST (void)
{
  DST_INFO_IDX idx;
  DST_INFO *info;
  Dwarf_P_Die parent;
  BOOL inlined_subp;
  BOOL visit_children;

  if (Trace_Dwarf) {
	fprintf(TFile, "Trace Extra DST\n");
  }
  /* visit subsequent siblings at this level */
  for (idx = DST_first_child (cu_idx);	/* start at beginning */
       !DST_IS_NULL(idx);
       idx = DST_INFO_sibling(DST_INFO_IDX_TO_PTR(idx)))
  {
    info = DST_INFO_IDX_TO_PTR (idx);
    if (DST_IS_info_mark(DST_INFO_flag(info)))
	continue;	/* already traversed */

    /* check for inlined subprogram that hasn't been traversed yet;
     * this implies that the PU was optimized away, so we can't access
     * any of the info below the subprogram, but we still want the basic
     * subprogram info to be available to dbx. */
    if (DST_INFO_tag(info) == DW_TAG_subprogram
	&& ! DST_IS_declaration(DST_INFO_flag(info)) )
    {
	DST_SUBPROGRAM *PU_attr = DST_ATTR_IDX_TO_PTR(DST_INFO_attributes(info), DST_SUBPROGRAM);
	inlined_subp = subprogram_def_is_inlined(PU_attr);
	visit_children = FALSE;
    }
    else {
	visit_children = TRUE;
	inlined_subp = FALSE;
    }
    /* only traverse inlined subp's or non-subp's */
    if ((DST_INFO_tag(info) != DW_TAG_subprogram
	&& DST_INFO_tag(info) != DW_TAG_entry_point)
	 || DST_IS_declaration(DST_INFO_flag(info))
	 || inlined_subp )
    {
      /* All DST entries other than subprograms are attached as children
       * of the compile_unit die.
       */
      parent = CGD_enclosing_proc[GLOBAL_LEVEL];
      (void) preorder_visit (idx, parent, NULL, LOCAL_LEVEL, visit_children);
      DST_SET_info_mark(DST_INFO_flag(info));	/* mark has been traversed */
    }
  }
}

/* ====================================================================
 *
 * Traverse the DST from for the pu idx.
 *
 * ====================================================================
 */
static void
Traverse_DST (ST *PU_st, DST_IDX pu_idx)
{
  DST_INFO *info;
  DST_SUBPROGRAM *PU_attr;
  Dwarf_P_Die die;
  Dwarf_P_Die parent;
  INT nestlevel;
  DST_ASSOC_INFO assoc_info;

  info = DST_INFO_IDX_TO_PTR (pu_idx);
  FmtAssert ( (DST_INFO_tag(info) == DW_TAG_subprogram)
	&& !DST_IS_declaration(DST_INFO_flag(info)),
	("Traverse_DST:  pu_idx is not a subprogram def?"));
  PU_attr = DST_ATTR_IDX_TO_PTR(DST_INFO_attributes(info), DST_SUBPROGRAM);
  assoc_info = DST_SUBPROGRAM_def_st(PU_attr);
  FmtAssert ( (DST_ASSOC_INFO_st_level(assoc_info) == ST_level(PU_st))
	&& (DST_ASSOC_INFO_st_index(assoc_info) == ST_index(PU_st)),
	("Traverse_DST:  pu_idx doesn't match PU_st"));
  if (Trace_Dwarf)
    fprintf ( TFile, "Traverse_DST:  found idx [%d,%d] for %s\n",
	pu_idx.block_idx, pu_idx.byte_idx, ST_name(PU_st));

  DST_SET_info_mark(DST_INFO_flag(info));	/* mark has been traversed */
  // in new symtab, local level starts at 2.
  // so reset that to 1, and keep globals combined into 0.
  nestlevel = CURRENT_SYMTAB - 1;
  parent = CGD_enclosing_proc[nestlevel-1];
  die = preorder_visit (pu_idx, parent, NULL, nestlevel, TRUE /*visit_children*/);
  Set_Enclosing_Die (die, nestlevel);

  if (PU_has_altentry(Get_Current_PU())) {
	/* also process any sibling entries at this time */
	DST_INFO_IDX idx;
	idx = DST_INFO_sibling(DST_INFO_IDX_TO_PTR(pu_idx));
	while ( !DST_IS_NULL(idx) ) {
		info = DST_INFO_IDX_TO_PTR(idx);
		// why do we have base_type and subroutine_type here?
		// I assume cause can be in middle of sibling list
		// and want to keep searching. 
		// but then need to later check whether has been 
		// already traversed.
		if ( ! (DST_INFO_tag(info) == DW_TAG_entry_point
		     || DST_INFO_tag(info) == DW_TAG_base_type
		     || DST_INFO_tag(info) == DW_TAG_subroutine_type
		     || (DST_INFO_tag(info) == DW_TAG_subprogram
			&& DST_IS_declaration(DST_INFO_flag(info)) ) ))
		{
			break;	/* non-entry found, so stop */
		}
    		if ( ! DST_IS_info_mark(DST_INFO_flag(info))) {
			DST_SET_info_mark(DST_INFO_flag(info));	/* mark has been traversed */
			die = preorder_visit (idx, NULL, die /*sibling*/, nestlevel, TRUE);
		}
		idx = DST_INFO_sibling(info);
	}
  }
}


void 
Cg_Dwarf_Process_PU (Elf64_Word	scn_index,
		     LABEL_IDX  begin_label,
		     LABEL_IDX  end_label,
		     INT32      end_offset,
		     ST        *PU_st,
		     DST_IDX    pu_dst,
		     Elf64_Word eh_index,
		     INT        eh_offset,
		     // The following two arguments need to go away
		     // once libunwind provides an interface that lets
		     // us specify ranges symbolically.
		     INT        low_pc,
		     INT        high_pc)
{
  DST_INFO *info;
  Dwarf_P_Die PU_die;
  Dwarf_P_Expr expr;
  Dwarf_P_Fde fde;
#ifdef TARG_ST
  Dwarf_P_Fde eh_fde;
#endif

  DST_SUBPROGRAM *PU_attr;
  
  cur_text_index = scn_index;
  Trace_Dwarf = Get_Trace ( TP_EMIT, 8 );	// for each pu

  /* turn off generation of dwarf information from DSTs. */
  if (Disable_DST) return;

#ifndef TARG_ST
  static BOOL processed_globals = FALSE;
  // [CL] No longer process globals first: In order to generate only
  // 'necessary' info, we generate all PUs first, then remaining
  // globals, and only needed types.  This is to avoid generating type
  // pointers to types with only local scope where local scope has
  // been discarded by DFE
  if ( ! processed_globals) {
	// do this once, before PU info.
	// we do this here rather than in Begin routine
	// cause have to wait until globals are allocated.
#ifdef TARG_ST
  // (cbr) we enter here either for debug dwarf emission or exceptions frame dwarf unwinding 
  if (CG_emit_asm_dwarf)
#endif  
	Traverse_Global_DST ();	// emit global types before PUs
	processed_globals = TRUE;
  }
#endif

  if (Trace_Dwarf)
	fprintf(TFile, "dwarf for %s:\n", ST_name(PU_st));

  if (DST_IS_NULL(pu_dst)) {
    /* cloning may create subp with no DST info */
    if (strncmp(ST_name(PU_st), "*clone*", 7) != 0)
      DevWarn("NULL DST passed to CG for %s", ST_name(PU_st));
    return;
  }

#ifdef TARG_ST
  // (cbr) we enter here either for debug dwarf emission or exceptions frame dwarf unwinding 
  if (CG_emit_asm_dwarf)
#endif  
  Traverse_DST (PU_st, pu_dst);

  if (DST_IS_NULL(pu_dst)) return;

  info = DST_INFO_IDX_TO_PTR (pu_dst);
  /* if current die is not for the subprogram, we cannot add any attributes. */
  if (DST_INFO_tag(info) != DW_TAG_subprogram) return;

  PU_attr = DST_ATTR_IDX_TO_PTR(DST_INFO_attributes(info), DST_SUBPROGRAM);
  PU_die = (Dwarf_P_Die) DST_INFO_dieptr(info);

  if (subprogram_def_is_inlined(PU_attr)) {
	/* don't generate frame info for inlined subp,
	 * but do generate a new die for concrete out-of-line instance
	 * that does have frame info. */
	Dwarf_P_Die idie = dwarf_new_die (dw_dbg, DW_TAG_subprogram, NULL, NULL, PU_die, NULL, &dw_error);
	if (Trace_Dwarf) {
		fprintf (TFile,"NEW subprogram concrete instance: die: %p, left_sibling:%p\n",
		idie, PU_die);
	}
	/* has same attributes as inlined_subprogram,
	 * though a different tag. */
	put_concrete_subprogram(pu_dst,
				0 /* low_pc */,
				0 /* high_pc */,
				idie);
	PU_die = idie;
  }

  /* setup the frame_base attribute. */
  expr = dwarf_new_expr (dw_dbg, &dw_error);
  if (Current_PU_Stack_Model != SMODEL_SMALL)
    dwarf_add_expr_gen (expr, DW_OP_bregx,
#ifdef TARG_ST
			//TB: better DWARF register support
			Get_Debug_Reg_Id(FP_TN),
#else
			REGISTER_machine_id (TN_register_class(FP_TN), TN_register(FP_TN)),
#endif
			0, &dw_error);
  else
    dwarf_add_expr_gen (expr, DW_OP_bregx,
#ifdef TARG_ST
			//TB: better DWARF register support
			Get_Debug_Reg_Id (SP_TN),
#else
			REGISTER_machine_id (TN_register_class(SP_TN), TN_register(SP_TN)),
#endif
	Frame_Len, &dw_error);

#ifdef TARG_ST
  if (CG_emit_asm_dwarf) {
#endif
  dwarf_add_AT_location_expr(dw_dbg, PU_die, DW_AT_frame_base, expr, &dw_error);
  if (PU_is_mainpu(ST_pu(PU_st))) {
   	dwarf_add_AT_unsigned_const (dw_dbg, PU_die, DW_AT_calling_convention, 
	     DW_CC_program, &dw_error);
  }
#ifdef TARG_ST
  }
#endif

  Dwarf_Unsigned begin_entry = Cg_Dwarf_Symtab_Entry(CGD_LABIDX,
						     begin_label,
						     scn_index);
  Dwarf_Unsigned end_entry   = Cg_Dwarf_Symtab_Entry(CGD_LABIDX,
						     end_label,
						     scn_index);

  fde = Build_Fde_For_Proc (dw_dbg, REGION_First_BB,
			    begin_entry,
			    end_entry,
			    end_offset,
#ifdef TARG_ST
    // [CL] need scn_index to connect debug_frame label to Dwarf symtab
			    low_pc, high_pc, scn_index, true);
#else
                            low_pc, high_pc);

#endif

#ifdef TARG_ST
  eh_fde = Build_Fde_For_Proc (dw_dbg, REGION_First_BB,
			    begin_entry,
			    end_entry,
			    end_offset,
			    low_pc, high_pc, scn_index);
  char *personality_str = NULL;
  const PU& pu = Pu_Table[ST_pu(PU_st)];
  if (PU_has_exc_scopes(pu)) {
    if (PU_cxx_lang(pu))
      personality_str = "__gxx_personality_v0";
    else if (PU_c_lang(pu))
      personality_str = "__gcc_personality_v0";
  }
  if (personality_str
      && (Gen_PIC_Shared || Gen_PIC_Call_Shared)) {
      // Problem: we want to put a pointer to the
      // personality routine in the CIE.  However, this address is
      // external and preemptible, so it will require a dynamic relocation
      // even if we use PC-relative addressing.
      // For efficiency at load time, we do not want to have a dynamic
      // relocation in every CIE, so we use a standard trick:
      // we make the encoding indirect, so that in the CIE we point
      // to a data symbol, and that data symbol is initialized to the
      // address of the personality routine.
      // We make the symbol global+weak+hidden+linkonce, so that we should have
      // just one occurrence of it in a shared object; thus it requires
      // just one dynamic relocation per shared object.
      // The data symbol should be named DW.ref.<personality_str>
      // it should be hidden, weak, linkonce, and it should be placed
      // in the section called .gnu.linkonce.d.rel.ro.DW.ref.<personality_str>
      
      ST *personality_st = find_global_symbol_by_name (personality_str);
      if (personality_st == NULL) {
	TY_IDX ret_ty_idx = MTYPE_To_TY (Def_Int_Mtype);
	TY_IDX personality_ty_idx = Make_Function_Type (ret_ty_idx);
	PU_IDX pu_idx;
	PU& pu = New_PU (pu_idx);
	PU_Init (pu, personality_ty_idx, GLOBAL_SYMTAB + 1);
	personality_st = New_ST (GLOBAL_SYMTAB);
	ST_Init (personality_st, Save_Str (personality_str),
		 CLASS_FUNC, SCLASS_EXTERN, EXPORT_PREEMPTIBLE,
		 TY_IDX (pu_idx));
      }
      STR_IDX personality = Save_Str2 ("DW.ref.", personality_str);
      ST *ptr_personality_st = find_global_symbol_by_name (Index_To_Str (personality));
      if (ptr_personality_st == NULL) {
	ptr_personality_st = New_ST (GLOBAL_SYMTAB);
	ST_Init (ptr_personality_st, personality,
		 CLASS_VAR, SCLASS_DGLOBAL, EXPORT_HIDDEN,
		 Make_Pointer_Type (ST_pu_type(personality_st)));
	Set_ST_is_weak_symbol (ptr_personality_st);
	Set_ST_is_comdat (ptr_personality_st);
	ST_ATTR_IDX st_attr_idx;
	ST_ATTR& st_attr = New_ST_ATTR (GLOBAL_SYMTAB, st_attr_idx);
	ST_ATTR_Init (st_attr, ST_st_idx (ptr_personality_st), ST_ATTR_SECTION_NAME,
		      Save_Str2 (".gnu.linkonce.d.rel.ro.DW.ref.", personality_str));
	Set_ST_has_named_section (ptr_personality_st);
	INITV_IDX st_iv = New_INITV ();
	INITV_Init_Symoff (st_iv, personality_st, 0);
	INITO_IDX inito = New_INITO (ptr_personality_st, st_iv);
	Set_ST_is_initialized (ptr_personality_st);
	Allocate_Object (ptr_personality_st);
      }
  }
#endif

  Dwarf_Unsigned eh_handle;

  if (eh_offset != DW_DLX_NO_EH_OFFSET) {
    eh_handle = Cg_Dwarf_Symtab_Entry(CGD_ELFSYM, eh_index);
  }
  else {
    eh_handle = eh_index;
  }

  Em_Dwarf_Process_PU (begin_entry,
		       end_entry,
		       0,	// begin_offset
		       end_offset,
		       PU_die,
		       fde,
#ifdef TARG_ST
		       eh_fde,
#endif
		       eh_handle,
		       eh_offset);

}

static DST_language
Get_Dwarf_Language (DST_INFO *cu_info)
{
	return DST_COMPILE_UNIT_language( DST_ATTR_IDX_TO_PTR(
		DST_INFO_attributes(cu_info), DST_COMPILE_UNIT));
}

/* Initialize generation of dwarf information. The file table is emitted
 * in this routine. The compile_unit die is also emitted here.
 */
void
Cg_Dwarf_Begin (BOOL is_64bit)
{
  DST_INFO *   cu_info;
  Dwarf_P_Die  cu_die;

  Trace_Dwarf = Get_Trace ( TP_EMIT, 8 );
  Disable_DST = Get_Trace ( TP_EMIT, 0x200 );

  cu_idx = DST_get_compile_unit ();
  cu_info = DST_INFO_IDX_TO_PTR (cu_idx);
  Dwarf_Language = Get_Dwarf_Language (cu_info);

  dw_dbg = Em_Dwarf_Begin(is_64bit, Trace_Dwarf, 
			  (Dwarf_Language == DW_LANG_C_plus_plus),
			  Cg_Dwarf_Enter_Elfsym);

  /* Read in the compile unit entry */
  cu_die = get_ref_die (cu_idx);
  Set_Enclosing_Die (cu_die, GLOBAL_LEVEL);

#ifdef TARG_ST
  // (cbr) we enter here either for debug dwarf emission or exceptions frame dwarf unwinding 
  if (CG_emit_asm_dwarf)
#endif  
  Write_Attributes (DST_INFO_tag(cu_info), 
		    DST_INFO_flag (cu_info), 
		    DST_INFO_attributes(cu_info), 
		    cu_die);

  // Invalid entry up front to keep from using the zero index.
  CGD_Symtab.push_back(CGD_SYMTAB_ENTRY(CGD_ELFSYM, (UINT64)-1));
}


/* go through any remaining DST entries after the last subprogram. This
   also handles the case of a file with no PUs.
*/
void Cg_Dwarf_Finish (pSCNINFO text_scninfo)
{
  if (Disable_DST) return;

#ifdef TARG_ST
  // (cbr) we enter here either for debug dwarf emission or exceptions frame dwarf unwinding 
  if (CG_emit_asm_dwarf)
#endif  

#ifndef TARG_ST
  Traverse_Extra_DST();	/* do final pass for any info not emitted yet */
#else
  // [CL] emit global types and variables after PUs
  // so that only needed (ie referenced) types
  // are emitted
  Traverse_Global_DST ();
#endif
}

#ifndef TARG_ST
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

file_info Cg_Dwarf_File_Table(INT idx){
  return file_table[file_idx];
}

include_info Cg_Dwarf_Include_Table(INT idx){
  return incl_table[idx];
}

#endif
static file_info *file_table;
static include_info *incl_table;
static INT cur_file_index = 0;

#ifdef TARG_ST
//TB: access to file_table and incl_table for gcov support
file_info Cg_Dwarf_File_Table(INT idx){
  return file_table[idx];
}

include_info Cg_Dwarf_Include_Table(INT idx){
  return incl_table[idx];
}

#endif
void Cg_Dwarf_Gen_Asm_File_Table (void)
{
  INT count;
  DST_IDX idx;
  DST_INCLUDE_DIR *incl;
  DST_FILE_NAME *file;
  char *name;
  INT incl_table_size;
  INT file_table_size;
  INT new_size;

  incl_table_size = 0;
  incl_table = NULL;
  file_table_size = 0;
  file_table = NULL;
  count = 1;
  for (idx = DST_get_include_dirs (); 
       !DST_IS_NULL(idx); 
       idx = DST_INCLUDE_DIR_next(incl))
  {
    incl = DST_DIR_IDX_TO_PTR (idx);
    name = DST_STR_IDX_TO_PTR (DST_INCLUDE_DIR_path(incl));
    if (count >= incl_table_size) {
      new_size = count + 10;
      if (incl_table == NULL)
        incl_table = (include_info *) malloc (new_size * sizeof (include_info));
      else 
	incl_table = (include_info *) realloc (incl_table, new_size * sizeof (include_info));
      if (incl_table == NULL) ErrMsg (EC_No_Mem, "Cg_Dwarf_Gen_Asm_File_Table");
      incl_table_size = new_size;
    }
    incl_table[count].path_name = name;
    incl_table[count].already_processed = FALSE;
    count++;
  }

  count = 1;
  for (idx = DST_get_file_names (); 
       !DST_IS_NULL(idx); 
       idx = DST_FILE_NAME_next(file))
  {
    file = DST_FILE_IDX_TO_PTR (idx);
    if (DST_IS_NULL(DST_FILE_NAME_name(file))) {
      name = "NULLNAME";
    }
    else {
      name = DST_STR_IDX_TO_PTR (DST_FILE_NAME_name(file));
    }
    if (count >= file_table_size) {
      new_size = count + 10;
      if (file_table == NULL)
        file_table = (file_info *) malloc (new_size * sizeof (file_info));
      else 
	file_table = (file_info *) realloc (file_table, 
					    new_size * sizeof (file_info));
      if (file_table == NULL) ErrMsg (EC_No_Mem, "Cg_Dwarf_Gen_Asm_File_Table");
      file_table_size = new_size;
    }
    file_table[count].filename = name;
    file_table[count].incl_index = DST_FILE_NAME_dir(file);
    file_table[count].fileptr = NULL;
    file_table[count].max_line_printed = 0;
    file_table[count].already_processed = FALSE;
    file_table[count].mod_time = DST_FILE_NAME_modt(file);
    file_table[count].file_size = DST_FILE_NAME_size(file);
    count++;

#ifdef TEMPORARY_STABS_FOR_GDB
    // This is an ugly hack to enable basic debugging for IA-32 target
    if (Debug_Level > 0 && count == 3) {
      fprintf(Asm_File, ".stabs \"%s/\",100,0,0,.Ltext0\n", 
              incl_table[DST_FILE_NAME_dir(file)].path_name);
      fprintf(Asm_File, ".stabs \"%s\",100,0,0,.Ltext0\n", name);
    }
#endif

  }

}

#ifdef TARG_ST
/*
 * Register the source file referenced by the argument in dwarf information
 * if not yet done.
 * This piece of code has been extracted from Cg_Dwarf_Add_Line_Entry() to
 * accessible to other functions.
 */
static void
Cg_Dwarf_Register_Source_File(USRCPOS usrcpos) {

  if (USRCPOS_srcpos(usrcpos) == 0) {
    return;
  }
  
  INT file_idx = USRCPOS_filenum(usrcpos);
  INT include_idx;
  // file change
  if ( ! file_table[file_idx].already_processed) {
    // new file
    include_idx = file_table[file_idx].incl_index;
// [CL] merged and fixed from Open64-4.2.1, for bug #55000: we use
// incl_table's 0th entry for current working dir.
#ifdef TARG_ST
    if (include_idx != 0)
#endif
    if (! incl_table[include_idx].already_processed) {
      // new include
      if (CG_emit_asm_dwarf) {
        Em_Dwarf_Add_Include (include_idx, 
                              incl_table[include_idx].path_name);
      }
      incl_table[include_idx].already_processed = TRUE;
    }
      
    if (CG_emit_asm_dwarf) {
      Em_Dwarf_Add_File (file_idx, 
                         file_table[file_idx].filename,
                         include_idx,
                         file_table[file_idx].mod_time,
                         file_table[file_idx].file_size);
    }
    file_table[file_idx].already_processed = TRUE;
    /* (cbr) be consistant with solaris when cross compiling */
    /* See corresponding change in Cg_Dwarf_Add_Line_Entry() */
    //#ifndef linux 
    // for irix, only need .file when new file,
    // as subsequent .locs use file number.
//       if (Assembly) {
// 	CGEMIT_Prn_File_Dir_In_Asm(usrcpos,
// 				   incl_table[include_idx].path_name,
// 				   file_table[file_idx].filename);
//       }
  }
}
#endif

static void
print_source (SRCPOS srcpos)
{
  USRCPOS usrcpos;
  char srcfile[1024];
  char text[1024];
  file_info *cur_file;
  INT i;
  INT newmax;

  USRCPOS_srcpos(usrcpos) = srcpos;

  /* TODO: we don't handle this yet. */
  if (USRCPOS_filenum(usrcpos) == 0) return;

  cur_file = &file_table[USRCPOS_filenum(usrcpos)];
  if (USRCPOS_filenum(usrcpos) != cur_file_index) {
    if (cur_file_index != 0) {
      /* close the previous file. */
      file_info *prev_file = &file_table[cur_file_index];
      fclose (prev_file->fileptr);
      prev_file->fileptr = NULL;
    }
    cur_file_index = USRCPOS_filenum(usrcpos);
    cur_file = &file_table[cur_file_index];
    /* open the new file. */
// [CL] merged from Open64-4.2.1, for bug #55000: we use incl_table's
// 0th entry for current working dir.
#ifdef TARG_SL
    if (cur_file->incl_index == 0)
      sprintf (srcfile, "%s",cur_file->filename);
    else
#endif
    sprintf (srcfile, "%s/%s", incl_table[cur_file->incl_index].path_name,
				cur_file->filename);
    cur_file->fileptr = fopen (srcfile, "r");
    if (cur_file->fileptr == NULL) {
      cur_file_index = 0;	/* indicate invalid cur_file */
      return;
    }
    for (i = 0; i < cur_file->max_line_printed; i++) {
      fgets (text, sizeof(text), cur_file->fileptr);
    }
  }
  newmax = USRCPOS_linenum(usrcpos) - 5;
  if (cur_file->max_line_printed < newmax) {
    for (i = cur_file->max_line_printed; i < newmax; i++) {
      fgets (text, sizeof(text), cur_file->fileptr);
    }
    cur_file->max_line_printed = newmax;
  }
  if (cur_file->max_line_printed < USRCPOS_linenum(usrcpos)) {
    for (i = cur_file->max_line_printed; i < USRCPOS_linenum(usrcpos); i++) {
      if (fgets (text, sizeof(text), cur_file->fileptr) != NULL) {
	// check for really long line
	if (strlen(text) >= 1023) text[1022] = '\n'; 
#ifdef TARG_ST
	{
	  int len = strlen(text);
	  // [CL] ensure that text ends with a new line
	  if (text[len-1] != '\n') {
	    text[len] = '\n';
	    if (len <= 1022) {
	      text[len+1] = '\0';
	    }
	  }
	}
#endif
        fprintf (Asm_File, "%s%4d  %s", ASM_CMNT_LINE, i+1, text);
      }
    }
    cur_file->max_line_printed = USRCPOS_linenum(usrcpos);
  }
  return;
}

/* ====================================================================
 *   Cg_Dwarf_Add_Line_Entry
 *
 *   THis adds line info and, as a side effect,
 *   builds tables in dwarf2 for the file numbers
 *
 *   For linux IA-64, we should emit a series of .file directives
 *   on first call in the order used in tables internaly such
 *   as references from macro info and declarations.
 *   This series would have the side effect of registering
 *   file names with the right file number.
 *   Because the gnu assembler actually searches on *name*
 *   before number in building its file tables
 *   to create .debug_line.
 * ====================================================================
 */
void
Cg_Dwarf_Add_Line_Entry (
  INT code_address, 
  SRCPOS srcpos
#ifdef TARG_ST // [CL] if true, emit debug info even if we are on
  // the source line than the previous emission. Useful in the
  // prologue/epilogue areas
  ,BOOL force_emission
#endif
)
{
  static SRCPOS last_srcpos = 0;
  USRCPOS usrcpos;

  if (srcpos == 0 && last_srcpos == 0 )
	DevWarn("no valid srcpos at PC %d\n", code_address);

#ifdef TARG_ST // [CL]
  if (srcpos == 0 || ((srcpos == last_srcpos) && !force_emission ))
      {
	return;
      }
#else
  if (srcpos == 0 || srcpos == last_srcpos) return;
#endif

  // TODO:  figure out what to do about line changes in middle of bundle ???
  // For assembly, can put .loc in middle of bundle.
  // But can't generate object code with that,
  // because libdwarf expects addresses to be aligned with instructions,
  // so ignore such cases.
#ifndef TARG_ST	   // CL: For ST targets, this function is only called
                   //     at bundle starts
  if ((code_address % ISA_MAX_INST_BYTES) != 0) {
#if 0
  	if (Object_Code) return;
#else
	// Arthur: temporary hack
	return;
#endif
  }
#endif /* !TARG_ST */

#ifdef TARG_ST // [CL]
  if (CG_emit_asm_dwarf) {
    New_Debug_Line_Set_Label(code_address, FALSE);
  }
#endif

  USRCPOS_srcpos(usrcpos) = srcpos;

  // only emit file changes when happen, so don't bloat objects
  // with unused header files.
  USRCPOS last_usrcpos;
  USRCPOS_srcpos(last_usrcpos) = last_srcpos;
  if (USRCPOS_filenum(last_usrcpos) != USRCPOS_filenum(usrcpos)) {
#ifdef TARG_ST
    Cg_Dwarf_Register_Source_File(usrcpos);

    /* (cbr) be consistant with solaris when cross compiling */
    //#ifdef linux
    // For linux, emit .file whenever file changes,
    // as it applies to all following  line directives,
    // whatever the spelling.
    if (Assembly) {
      INT file_idx = USRCPOS_filenum(usrcpos);
      INT include_idx = file_table[file_idx].incl_index;
// [CL] merged from Open64-4.2.1, for bug #55000: we use incl_table's
// 0th entry for current working dir.
#ifdef TARG_SL
      if (include_idx == 0)
	CGEMIT_Prn_File_Dir_In_Asm(usrcpos,
				   NULL,
				   file_table[file_idx].filename);
      else
#endif
      CGEMIT_Prn_File_Dir_In_Asm(usrcpos,
                                 incl_table[include_idx].path_name,
                                 file_table[file_idx].filename);
    }
#else
    INT file_idx = USRCPOS_filenum(usrcpos);
    INT include_idx;
    // file change
    if ( ! file_table[file_idx].already_processed) {
      // new file
      include_idx = file_table[file_idx].incl_index;
      if ( ! incl_table[include_idx].already_processed) {
	// new include
	if (Object_Code) {
	  Em_Dwarf_Add_Include (include_idx, 
				incl_table[include_idx].path_name);
	}
	incl_table[include_idx].already_processed = TRUE;
      }

      if (Object_Code) {
	Em_Dwarf_Add_File (file_idx, 
			   file_table[file_idx].filename,
			   include_idx,
			   file_table[file_idx].mod_time,
			   file_table[file_idx].file_size);
      }
      file_table[file_idx].already_processed = TRUE;
      //#ifndef linux 
      // for irix, only need .file when new file,
      // as subsequent .locs use file number.
      if (Assembly) {
	CGEMIT_Prn_File_Dir_In_Asm(usrcpos,
				   incl_table[include_idx].path_name,
				   file_table[file_idx].filename);
      }
    }
#endif
  }


  // now do line number:
#ifdef TARG_ST
  if (CG_emit_asm_dwarf) {
#else
  if (Object_Code) {
#endif
    Em_Dwarf_Add_Line_Entry (code_address, srcpos);
  }

  if (Assembly) {
    CGEMIT_Prn_Line_Dir_In_Asm(usrcpos);
    if (List_Source)
    	print_source (srcpos);
  }
  last_srcpos = srcpos;
}

static inline Elf64_Word
reloc_offset(char *reloc, BOOL is_64bit)
{
  if (is_64bit) {
    return REL_offset(*((Elf64_Rel *) reloc));
  }
  else {
    return REL32_offset(*((Elf32_Rel *) reloc));
  }
}

static inline Elf64_Word
reloc_sym_index(char *reloc, BOOL is_64bit)
{
  if (is_64bit) {
    return REL64_sym(*((Elf64_Rel *) reloc));
  }
  else {
    return REL32_sym(*((Elf32_Rel *) reloc));
  }
}

static void check_reloc_fmt_and_size(Elf64_Word     reloc_scn_type,
				     Dwarf_Ptr      reloc_buffer,
				     Dwarf_Unsigned reloc_buffer_size,
				     Dwarf_Unsigned scn_buffer_size)
{
}

#ifdef __GNUC__
struct UINT32_unaligned {
  UINT32 val;
#if 0 // [CL] aligned() is not meant to reduce alignment
} __attribute__ ((aligned(1)));
#else
} __attribute__ ((packed));
#endif

struct UINT64_unaligned {
  UINT64 val;
#if 0 // [CL] aligned() is not meant to reduce alignment
} __attribute__ ((aligned(1)));
#else
} __attribute__ ((packed));
#endif
#else
#pragma pack(1)
struct UINT32_unaligned {
  UINT32 val;
};

struct UINT64_unaligned {
  UINT64 val;
};
#pragma pack(0)
#endif /* linux */



#ifdef TARG_STxP70 
#   include <string>
#   include <list>
#   include <map>

 static BOOL
 HasReloc(const char* section_name);

/**
 * Constant used to specify a 'not set' file position.
 */
static const long NOT_SET = -1;

/**
 * Standard identifier (DWARF 2.0 norm) used to distinguish a CIE from a FDE
 */
static const unsigned long g_CIE_id = 0xffffffff;

#endif

// These are intended to be file-local, and the unnamed namespace
// tells c++ to make them file-local

// See the comments at
// Cg_Dwarf_Output_Asm_Bytes_Sym_Relocs
// for the motivation for this virtual_section_position
// complexity.

// The following allows multiple sections
namespace {

#ifdef TARG_STxP70

//------------------------------------------------------------------------------
// Types declaration
//------------------------------------------------------------------------------

// Forward declaration
 class CDwarfOperand;
 class CDwarfInstruction; 

// We do not set a memory pool allocator, since it complicates for nothing (no
// dynamic allocation) 
 typedef Dwarf_Small Byte; 
 typedef std::list<Byte> Bytes;
 typedef Bytes::iterator BytesIt;
 typedef Bytes::const_iterator BytesCIt; 

 typedef std::list<CDwarfOperand> DwarfOperands; 
 typedef DwarfOperands::iterator DwarfOperandsIt; 
 typedef DwarfOperands::const_iterator DwarfOperandsCIt; 

 typedef std::list<CDwarfInstruction> DwarfInstructions;
 typedef DwarfInstructions::iterator DwarfInstructionsIt;
 typedef DwarfInstructions::const_iterator DwarfInstructionsCIt;

//------------------------------------------------------------------------------
// Global variables
//------------------------------------------------------------------------------
/**
 * Maximum length in bytes of a LEB128 variable
 */
 static const INT DW_LEB128_SIZE_IN_BYTES = 16;

 static const char* PADDING_STR = "\t%s\t0x00\n";

//------------------------------------------------------------------------------
// Classes declaration / definition
//------------------------------------------------------------------------------
/**
 * @class CDwarfOperand
 * A CDwarfOperand represents an operand in a dwarf instruction. It is aware of
 * what it represents (in a limited way) and know how to emit itself, with the
 * right relocation.
 */
 class CDwarfOperand
 {
 public:

     /**
      * @enum EEncoding
      * Represents the different encoding support by the decoding algorithm
      * embedded in dwarf operand class
      */
     typedef enum
         {
             NORMAL,  /**< Default target encoding, i.e. big or little endian */
             LEB128,  /**< Dwarf signed LEB128 encoding. This encoding is used
                       *   for variable length data
                       */
             ULEB128, /**< Dwarf unsigned LEB128 encoding.
                       * @see LEB128 encoding for more details
                       */
             STRING   /**< Null terminated string encoding */
         } EEncoding;

     /**
      * Constructor.
      * Create a new initialized instance of CDwarfOperand class
      *
      * @param  a_remainingBytesToRead Maximum number of bytes that should be
      *         read to have the operand definition. Set it to -1 for unknown
      *         value like for string encoding
      * @param  a_encod Type of the encoding used for the operand
      * @param  a_isRegister Specify whether the operand represents a register
      *         or not
      *
      * @pre    true
      * @post   All members are initialized with given parameters and default
      *         values
      *
      */
     CDwarfOperand(INT a_remainingBytesToRead = 0, EEncoding a_encod = NORMAL,
                   BOOL a_isRegister = FALSE)
         : m_remainingBytesToRead(a_remainingBytesToRead), m_raw(),
           m_decoded(0), m_shiftSize(0),
           m_encoding(a_encod), m_isRegister(a_isRegister)
     {}
     
     /**
      * Copy constructor.
      * Set all members with members' value of given object
      *
      * @param  a_operandClass Object to be copied
      *
      * @pre    true
      * @post   RemainingBytesToRead() = a_operandClass.RemainingBytesToRead()
      *         and Raw() = a_operandClass.Raw() and
      *         Decoded() = a_operandClass.Decoded() and
      *         Encoding() = a_operandClass.Encoding() and
      *         IsRegister() = a_operandClass.IsRegister() and
      *         ShiftSize() = a_operandClass.ShiftSize()
      *
      */
     CDwarfOperand(const CDwarfOperand& a_operandClass)
     {
         CopyMembers(a_operandClass);
     }

     /**
      * Operator=.
      * Assign members' value of right hand side operand to members of left hand
      * side operand.
      *
      * @param  a_operandClass Object to be copied
      *
      * @pre    true
      * @post   RemainingBytesToRead() = a_operandClass.RemainingBytesToRead()
      *         and Raw() = a_operandClass.Raw() and
      *         Decoded() = a_operandClass.Decoded() and
      *         Encoding() = a_operandClass.Encoding() and
      *         IsRegister() = a_operandClass.IsRegister() and
      *         ShiftSize() = a_operandClass.ShiftSize()
      *
      * @return The left hand side
      */
     CDwarfOperand&
     operator=(const CDwarfOperand& a_operandClass)
     {
         if(this != &a_operandClass)
             {
                 CopyMembers(a_operandClass);
             }
         return *this;
     }

     /**
      * Getter for m_remainingBytesToRead member.
      *
      * @pre    true
      * @post   result = m_remainingBytesToRead
      *
      * @return The value of m_remainingBytesToRead member
      *
      * @see m_remainingBytesToRead for more details
      */
     INT
     RemainingBytesToRead() const
     {
         return m_remainingBytesToRead;
     }

     /**
      * Getter for m_raw member.
      *
      * @pre    true
      * @post   result is meaningless unless RemainingBytesToRead() = 0
      *
      * @return A constant reference on m_raw member
      *
      * @see m_raw for more details
      */
     const Bytes&
     Raw() const
     {
         return m_raw;
     }

     /**
      * Getter for m_decoded member.
      *
      * @pre    true
      * @post   result is meaningless if RemainingBytesToRead() <> 0 or
      *         Encoding() = CDwarfOperand::STRING
      *
      * @return The value of m_decoded member
      *
      * @see m_decoded for more details
      */
     Dwarf_Unsigned
     Decoded() const
     {
         return m_decoded;
     }

     /**
      * Add a byte to the operand.
      * The byte is added to the operand if RemainingBytesToRead() <> 0. The
      * value of RemainingBytesToRead() is automatically updated according to
      * what we read and what is the current encoding.
      * 
      * @warning You cannot assume RemainingBytesToRead() =
      *          RemainingBytesToRead()@pre - 1.
      *
      * @param  a_byte Byte to add to the operand
      *
      * @pre    true
      * @post   RemainingBytesToRead()@pre <> 0 implies Raw() =
      *         Raw()@pre->append(a_byte)
      *
      */
     void
     AddByte(Dwarf_Small a_byte)
     {
         if(RemainingBytesToRead() != 0)
             {
                 m_raw.push_back(a_byte);
                 if(Encoding() == CDwarfOperand::STRING)
                     {
                         AddStringByte(a_byte);
                     }
                 else
                     {
                         AddNormalByte(a_byte);
                     }
             }
     }

     /**
      * Getter for m_encoding member.
      *
      * @pre    true
      * @post   result = m_encoding
      *
      * @return The value of m_encoding member
      *
      * @see m_encoding for more details
      */
     EEncoding
     Encoding() const
     {
         return m_encoding;
     }

     /**
      * Getter for m_isRegister member.
      *
      * @pre    true
      * @post   result = m_isRegister
      *
      * @return The value of m_isRegister member
      *
      * @see m_isRegister for more details
      */
     BOOL
     IsRegister() const
     {
         return m_isRegister;
     }

     /**
      * Emit current operand in given file.
      * 
      * @warning No checks are done on RemainingBytesToRead(), so if you
      *          do not call this function when RemainingBytesToRead() = 0, you
      *          will write in asm_file something that may be wrong
      *
      * @param  asm_file [in/out] File where we will write the operand
      *
      * @pre    true
      * @post   if IsRegister() and Decoded() represents an extension
      *         register then a dwarf register relocation has been emitted in
      *         asm_file else Raw() has been emitted in asm_file endif
      *
      * @return The number of emitted bytes (without comment)
      */
     int
     Emit(FILE* asm_file) const
     {
         BOOL emitted = FALSE;
         int size = 0;
         if(IsRegister())
             {
                 CExtensionAndRegister extAndReg(Decoded());
                 if(extAndReg.IsExtension())
                     {
                         emitted = TRUE;
                         size += CDwarfOperand::EmitReloc(asm_file, extAndReg);
                     }
             }
         if(!emitted)
             {
                 BytesCIt it;
                 for(it = Raw().begin(); it != Raw().end(); ++it)
                     {
                         if(it != Raw().begin())
                             {
                                 fprintf(asm_file, ", ");
                             }
                         fprintf(asm_file, "0x%02x", (INT)(*it));
                         ++size;
                     }
             }
         return size;
     }

 protected:

     /**
      * Copy members' value.
      * Assign members' value of given object to members of this object.
      *
      * @param  a_operandClass Object to be copied
      *
      * @pre    true
      * @post   RemainingBytesToRead() = a_operandClass.RemainingBytesToRead()
      *         and Raw() = a_operandClass.Raw() and
      *         Decoded() = a_operandClass.Decoded() and
      *         Encoding() = a_operandClass.Encoding() and
      *         IsRegister() = a_operandClass.IsRegister() and
      *         ShiftSize() = a_operandClass.ShiftSize()
      *
      */
     void
     CopyMembers(const CDwarfOperand& a_operandClass)
     {
         m_remainingBytesToRead = a_operandClass.RemainingBytesToRead();
         m_raw = a_operandClass.Raw();
         m_decoded = a_operandClass.Decoded();
         m_shiftSize = a_operandClass.ShiftSize();
         m_encoding = a_operandClass.Encoding();
         m_isRegister = a_operandClass.IsRegister();
     }

     /**
      * Add a byte to a "normal" operand.
      * A "normal" operand is an operand with a known maximal size, i.e. not
      * with a string encoding.
      *
      * @param  a_byte Byte to be added to the operand
      *
      * @pre    Encoding() != CDwarfOperand::STRING
      * @post   RemainingBytesToRead() < RemainingBytesToRead()@pre and
      *         Decoded() has been augmented at the right place with
      *         given byte.
      *
      */
     void
     AddNormalByte(Dwarf_Small a_byte)
     {
         // We will write one byte, so we test with the global length of the
         // target type minus one byte
         DevAssert(ShiftSize() <= (sizeof(Dwarf_Unsigned) - sizeof(a_byte)) * 8,
                   ("Type overflow during LEB128 decoding!"));
         // We keep both coded and decoded operands to avoid re-encoding
         // at emission time
         m_remainingBytesToRead -= sizeof(a_byte);
         if(Encoding() != CDwarfOperand::NORMAL)
             {
                 // ULEB128 decoding
                 m_decoded |= ((Dwarf_Unsigned)(a_byte & DATA_MASK))
                     << ShiftSize();
                 m_shiftSize += DW_BIT_BY_LEB;
                 if((a_byte & MORE_BYTE) == 0)
                     {
                         m_remainingBytesToRead = 0;
                     }
             }
         else
             {
#ifdef WORDS_BIGENDIAN
                 m_decoded |= ((Dwarf_Unsigned)a_byte) <<
                     (RemainingBytesToRead() * 8);
#else
                 m_decoded |= ((Dwarf_Unsigned)a_byte) << ShiftSize();
                 m_shiftSize += 8;
#endif
             }
         // Even if we did not perform the right decoding, this is not
         // a problem, since the decoded value is used only for
         // registers, thus unsigned.
         if(Encoding() == CDwarfOperand::LEB128 && !RemainingBytesToRead() &&
            ShiftSize() < sizeof(Dwarf_Signed) * 8 && a_byte & SIGN_BYTE)
             {
                 // Sign extension
                 m_decoded |= -(((Dwarf_Unsigned)1) << ShiftSize());
             }
     }

     /**
      * Add a byte to a "string" operand.
      * A "string" operand is an operand with an unknown size which end is
      * marked by a zero byte. (Definition of a null terminated string)
      *
      * @param  a_byte Byte which is part of the string
      *
      * @pre    Encoding() = CDwarfOperand::STRING
      * @post   a_byte = 0 implies RemainingBytesToRead() = 0
      *
      */
     void
     AddStringByte(Dwarf_Small a_byte)
     {
         if(a_byte == '\0')
             {
                 m_remainingBytesToRead = 0;
             }
     }
 
     /**
      * Getter for m_shiftSize.
      * Used to remember the last offset used for decoding the operand
      *
      * @pre    true
      * @post   result = m_shiftSize
      *
      * @return The value of m_shiftSize member
      *
      * @see m_shiftSize member for more details
      */
     INT
     ShiftSize() const
     {
         return m_shiftSize;
     }

     /**
      * Emit relocation string with given extension and register in given
      * asm_file.
      *
      * @param  asm_file  [in/out] Will contains the relocation string
      * @param  extAndReg Slot and register information
      *
      * @pre    true
      * @post   A relocation string has been emitted in asm_file with extAndReg
      *         information
      *
      * @return The number of emitted bytes
      */
     static int
     EmitReloc(FILE* asm_file, const CExtensionAndRegister& extAndReg)
     {
         int i;
         for(i = 0; CDwarfOperand::RELOC_FORMAT &&
                 CDwarfOperand::RELOC_FORMAT[i]; ++i)
             {
                 if(i)
                     {
                         fprintf(asm_file, ", ");
                     }
                 fprintf(asm_file, CDwarfOperand::RELOC_FORMAT[i],
                         extAndReg.Extension(), (UINT)extAndReg.Register());
             }
         return i;
     }


     /**
      * Number of bytes remaining to read to have a meaningful operand.
      * This number is a maximum, i.e. for a LEB128 encoding, one should set 16.
      * When your operand is a string and thus, you do not know the size, set
      * any non-zero value, e.g. -1
      */
     INT m_remainingBytesToRead;

     /**
      * Raw member.
      * Represents all bytes added to the object.
      *
      * @invariant m_raw.size() = number of call of AddByte made when
      *            RemainingBytesToRead() <> 0
      */
     Bytes m_raw;

     /**
      * Decoded operand member.
      * Contains operand decoded value if encoding type is not
      * CDwarfOperand::STRING. The decoding method depends on the encoding type.
      *
      * @todo This member cannot handle all values which size is greater than
      *       the size of Dwarf_Unsigned. Currently, when the decoded operand
      *       become too big for this variable, an assertion is raised. This
      *       problem occurs if you set very big value (2 power size of
      *       Dwarf_Unsigned (actually long long type) * 8) in LEB128 operand.
      *       This should not occur, because LEB128 operand are used to encode
      *       register number or data/code alignment.
      *
      * @see m_encoding member.
      */
     Dwarf_Unsigned m_decoded;

     /**
      * Shift size member.
      * Contains the last size used to set the byte in the decoded operand.
      */
     INT m_shiftSize;

     /**
      * Encoding type of the operand.
      * This member affects path used in decoding algorithms.
      */
     EEncoding m_encoding;

     /**
      * Is register member.
      * Specify whether this operand represents a register or not.
      */
     BOOL m_isRegister;

     /**
      * Number of significant bits store in one byte of the dwarf LEB128
      * representation
      */
     static const INT DW_BIT_BY_LEB;
     
     /**
      * Mask used for dwarf 2 translation to set padding bit for LEB128
      */
     static const INT MORE_BYTE;

     /**
      * Mask used to know whether the last LEB128 byte of an operand represents
      * a negative value
      */
     static const INT SIGN_BYTE;
     
     /**
      * Mask used for dwarf 2 translation in or from LEB128
      */
     static const INT DATA_MASK;

     /**
      * Dwarf register relocation format.
      * The expected format is a printf like string, where two unsigned operands
      * are expected in that order: The slot number of an extension and the
      * dwarf register identifier in the related extension.
      */
     static const char* RELOC_FORMAT[];
 };

/**
 * @class CDwarfInstruction
 * Represents a dwarf instruction, i.e. an operator and several operands.
 */
 class CDwarfInstruction
 {
 public:

     /**
      * @enum EInstKind
      * Represents the different type of instructions
      */
     typedef enum
         {
             LENGTH,  /**< The instruction represents a length */
             ID,      /**< The instruction represents an identifier */
             INST     /**< The instruction represents an instruction (we do not
                       *   care the type)
                       */
         } EInstKind;

     /**
      * Constructor.
      * Create a new initialized instance of CDwarfInstruction class
      *
      * @param  a_remainingBytesToRead Maximum number of bytes that remain to
      *         read before having the operator
      * @param  a_isDwarfOperator Specify whether operator is a dwarf one or not
      * @param  a_name The name of the instruction
      * @param  a_kind Specify whether the instruction represents a
      *         section length, an id or anything else.
      * @param  encod Specify the encoding type of the operator
      *
      * @pre    a_name <> NULL
      * @post   Operator().RemainingBytesToRead() = a_remainingBytesToRead and
      *         Operator().Encoding() = encod and Name() = a_name and
      *         IsDwarfOperator() = a_isDwarfOperator and Kind() =
      *         a_kind and Operands().empty()
      *
      * @remarks The API of this class does not allow to modify the operands.
      *          Indeed, operands are automatically set when we have a dwarf
      *          operator. To reuse this class in a true instruction context,
      *          you have to generalize this implementation.
      */
     CDwarfInstruction(INT a_remainingBytesToRead = 0,
                       BOOL a_isDwarfOperator = TRUE,
                       const char* a_name = "",
                       CDwarfInstruction::EInstKind a_kind = INST,
                       CDwarfOperand::EEncoding encod = CDwarfOperand::NORMAL)
         : m_operator(a_remainingBytesToRead, encod, FALSE),
           m_isDwarfOperator(a_isDwarfOperator),
           m_kind(a_kind), m_name(a_name), m_operands()
     {
         m_curOperand = m_operands.begin();
     }

     /**
      * Copy constructor.
      * Set all members with members' value of given object
      *
      * @param  a_dwInst Object to be copied
      *
      * @pre    true
      * @post   Operator() = a_dwInst.Operator() and
      *         GlobalRemainingBytesToRead() =
      *         a_dwInst.GlobalRemainingBytesToRead() and
      *         Operands() = a_dwInst.Operands() and Name() = a_dwInst.Name()
      *         and IsDwarfOperator() = a_dwInst.IsDwarfOperator() and
      *         Kind() = a_dwInst.Kind()
      *
      */
     CDwarfInstruction(const CDwarfInstruction& a_dwInst)
     {
         CopyMembers(a_dwInst);
     }

     /**
      * Operator=.
      * Assign members' value of right hand side operand to members of left hand
      * side operand.
      *
      * @param  a_dwInst Object to be copied
      *
      * @pre    true
      * @post   Operator() = a_dwInst.Operator() and
      *         GlobalRemainingBytesToRead() =
      *         a_dwInst.GlobalRemainingBytesToRead() and
      *         Operands() = a_dwInst.Operands() and Name() = a_dwInst.Name()
      *         and IsDwarfOperator() = a_dwInst.IsDwarfOperator() and
      *         Kind() = a_dwInst.Kind()
      *
      * @return The left hand side
      */
     CDwarfInstruction&
     operator=(const CDwarfInstruction& a_dwInst)
     {
         if(this != &a_dwInst)
             {
                 CopyMembers(a_dwInst);
             }
         return *this;
     }

     /**
      * Give the maximal number of bytes that remain to read..
      * In case of this number is not already known, e.g. we do not know yet the
      * operator, thus the operands or we have a string operand/operator,
      * returned value is -1.
      *
      * @pre    true
      * @post   true
      *
      * @return The maximal number of remaining bytes to read or -1 if unknown
      */
     INT
     GlobalRemainingBytesToRead() const
     {
         INT result = Operator().RemainingBytesToRead();
         if(!result)
             {
                 result = 0;
                 DwarfOperandsCIt it;
                 for(it = Operands().begin(); it != Operands().end(); ++it)
                     {
                         if(it->RemainingBytesToRead() < 0)
                             {
                                 result = -1;
                                 break;
                             }
                         else
                             {
                                 result += it->RemainingBytesToRead();
                             }
                     }
             }
         else if(IsDwarfOperator())
             {
                 // We have not finished to read the operator, so operands are
                 // not set (remember, we set the operands only for dwarf
                 // operator)
                 result = -1;
             }
         return result;
     }

     /**
      * Add a byte to the instruction.
      * This method needs a FILE because it emits the operator or an operands
      * as soon as the remaing bytes to read number is equal to 0. If
      * GlobalRemainingBytesToRead() = 0 then given byte is not added to the
      * instruction.
      *
      * @warning You cannot assume GlobalRemainingBytesToRead() =
      *          GlobalRemainingBytesToRead()@pre - 1.
      *
      * @param  asm_file [in/out] Assembler file which will contains the emitted
      *         bytes if any
      * @param  a_byte Byte to be added to the instruction
      *
      * @pre    true
      * @post   GlobalRemainingBytesToRead() <> 0 implies if
      *         Operator().RemainingBytesToRead()@pre <> 0 then
      *         Operator().Raw() = Operator().Raw()@pre->append(a_byte)
      *         else let curOperand: CDwarfOperand =
      *         Operands()->select(RemainingBytesToRead()@pre <> 0)->first() in
      *         curOperand.Raw() = curOperand.Raw()@pre->append(a_byte)
      *         endif
      *
      * @return The number of emitted bytes, if any (without comment)
      */
     int
     AddByte(FILE* asm_file, Dwarf_Small a_byte)
     {
         int size = 0;
         if(Operator().RemainingBytesToRead())
             {
                 m_operator.AddByte(a_byte);
                 // We finished to read the operator, so, now, we can set the
                 // operands specification.
                 if(!Operator().RemainingBytesToRead())
                     {
                         if(IsDwarfOperator())
                             {
                                 InitDwarfOperands();
                             }
                         // We cannot gather all printing functions in the same
                         // method, because some of the emitted bytes do not use
                         // this mecanism. I.e. they are directly emitted, so if
                         // we wait to have the full instruction to emit it, we
                         // will break the order, thus the information
                         // Note: It is important to emit the operator after
                         // setting the operands to set the right format
                         size = EmitOperator(asm_file);
                     }
             }
         else
             {
                 if(m_curOperand != Operands().end())
                     {
                         m_curOperand->AddByte(a_byte);
                         if(!m_curOperand->RemainingBytesToRead())
                             {
                                 size = EmitOperand(asm_file);
                                 ++m_curOperand;
                             }
                     }
             }
         return size;
     }

     /**
      * Getter for m_operator member.
      *
      * @pre    true
      * @post   true
      *
      * @return A constant reference to m_operator member.
      *
      * @see m_operator member for more details
      */
     const CDwarfOperand&
     Operator() const
     {
         return m_operator;
     }

     /**
      * Getter for m_operands member.
      *
      * @pre    true
      * @post   true
      *
      * @return A constant reference to m_operands member.
      *
      * @see m_operands member for more details
      */
     const DwarfOperands&
     Operands() const
     {
         return m_operands;
     }

     /**
      * Getter for m_name member.
      *
      * @pre    true
      * @post   true
      *
      * @return A constant reference to m_name member.
      *
      * @see m_name member for more details
      */
     const std::string&
     Name() const
     {
         return m_name;
     }

     /**
      * Getter for m_isDwarfOperator member.
      *
      * @pre    true
      * @post   true
      *
      * @return A constant reference to m_isDwarfOperator member.
      *
      * @see m_isDwarfOperator member for more details
      */
     BOOL
     IsDwarfOperator() const
     {
         return m_isDwarfOperator;
     }

     /**
      * Getter for m_kind member.
      *
      * @pre    true
      * @post   true
      *
      * @return A copy of the value of m_kind member.
      *
      * @see m_kind member for more details
      */
     EInstKind
     Kind() const
     {
         return m_kind;
     }

 protected:
     /**
      * Copy members' value.
      * Assign members' value of given object to members of this object.
      *
      * @param  a_dwInst Object to be copied
      *
      * @pre    true
      * @post   Operator() = a_dwInst.Operator() and
      *         GlobalRemainingBytesToRead() =
      *         a_dwInst.GlobalRemainingBytesToRead() and
      *         Operands() = a_dwInst.Operands() and Name() = a_dwInst.Name()
      *         and IsDwarfOperator() = a_dwInst.IsDwarfOperator() and
      *         Kind() = a_dwInst.Kind() and
      *         m_curOperand = a_dwInst.m_curOperand
      */
     void
     CopyMembers(const CDwarfInstruction& a_dwInst)
     {

         m_operator = a_dwInst.Operator();
         m_isDwarfOperator = a_dwInst.IsDwarfOperator();
         m_kind = a_dwInst.Kind();
         m_name = a_dwInst.Name();
         m_operands = a_dwInst.Operands();
         m_curOperand = a_dwInst.m_curOperand;
     }

     /**
      * Set operands for current dwarf operator.
      * According to the value of Operator().Raw()
      *
      * @pre    IsDwarfOperator() and Operator().RemainingBytesToRead() = 0
      * @post   m_operands is initialized with the related dwarf operands
      *
      * @remarks Supported dwarf operator are only DW_CFA_* ones
      */
     void
     InitDwarfOperands()
     {
         DevAssert(Operands().empty(), ("New operands definition implies list "
                                        "of operands is empty"));
         DevAssert(IsDwarfOperator() && !Operator().Raw().empty(),
                   ("Definition of operand is done only for dwarf operator"));
         BOOL bFound = TRUE;
         Dwarf_Small curInst = *(Operator().Raw().begin());
         // Check instructions that part of the operand is embedded in the
         // operator.
         // Note: embedded operands are not relocated. We assume relocatable
         // registers (we relocate only registers) use related extended
         // operators
         switch(curInst & (0x3 << 6))
             {
             case DW_CFA_advance_loc:
                 Name("DW_CFA_advance_loc");
             case DW_CFA_restore:
                 // Name is set by this setter only if m_name is empty.
                 // So we do not override the previous value.
                 Name("DW_CFA_restore");
                 break;
             case DW_CFA_offset:
                 Name("DW_CFA_offset");
                 m_operands.push_back(CDwarfOperand(DW_LEB128_SIZE_IN_BYTES,
                                                    CDwarfOperand::ULEB128,
                                                    FALSE));
                 break;
             default:
                 bFound = FALSE;
             }
         if(!bFound)
             {
                 switch(curInst)
                     {
                     case DW_CFA_def_cfa_offset:
                         Name("DW_CFA_def_cfa_offset");
                         m_operands
                             .push_back(CDwarfOperand(DW_LEB128_SIZE_IN_BYTES,
                                                      CDwarfOperand::ULEB128,
                                                      FALSE));
                         break;
                     case DW_CFA_advance_loc1:
                         Name("DW_CFA_advance_loc1");
                         m_operands
                             .push_back(CDwarfOperand(1, CDwarfOperand::NORMAL,
                                                      FALSE));
                         break;
                     case DW_CFA_advance_loc2:
                         Name("DW_CFA_advance_loc2");
                         m_operands
                             .push_back(CDwarfOperand(2, CDwarfOperand::NORMAL,
                                                      FALSE));
                         break;
                     case DW_CFA_set_loc:
                         Name("DW_CFA_set_loc");
                     case DW_CFA_advance_loc4:
                         Name("DW_CFA_advance_loc4");
                         m_operands
                             .push_back(CDwarfOperand(4, CDwarfOperand::NORMAL,
                                                      FALSE));
                         break;
                     case DW_CFA_restore_extended:
                         Name("DW_CFA_restore_extended");
                     case DW_CFA_undefined:
                         Name("DW_CFA_undefined");
                     case DW_CFA_same_value:
                         Name("DW_CFA_same_value");
                         m_operands
                             .push_back(CDwarfOperand(DW_LEB128_SIZE_IN_BYTES,
                                                      CDwarfOperand::ULEB128,
                                                      TRUE));
                         break;
                     case DW_CFA_offset_extended:
                         Name("DW_CFA_offset_extended");
                     case DW_CFA_def_cfa:
                         Name("DW_CFA_def_cfa");
                         m_operands
                             .push_back(CDwarfOperand(DW_LEB128_SIZE_IN_BYTES,
                                                      CDwarfOperand::ULEB128,
                                                      TRUE));
                         m_operands
                             .push_back(CDwarfOperand(DW_LEB128_SIZE_IN_BYTES,
                                                      CDwarfOperand::ULEB128,
                                                      FALSE));
                         break;
                     case DW_CFA_def_cfa_register:
                         Name("DW_CFA_def_cfa_register");
                         m_operands
                             .push_back(CDwarfOperand(DW_LEB128_SIZE_IN_BYTES,
                                                      CDwarfOperand::ULEB128,
                                                      TRUE));
                         break;
                     case DW_CFA_register:
                         Name("DW_CFA_register");
                         m_operands
                             .push_back(CDwarfOperand(DW_LEB128_SIZE_IN_BYTES,
                                                      CDwarfOperand::ULEB128,
                                                      TRUE));
                         m_operands
                             .push_back(CDwarfOperand(DW_LEB128_SIZE_IN_BYTES,
                                                      CDwarfOperand::ULEB128,
                                                      TRUE));
                         break;
                     case DW_CFA_nop:
                         Name("DW_CFA_nop");
                         // These two does not exists
                         // * case DW_CFA_lo_user:
                         // * case DW_CFA_hi_user:
                         // No operands
                         break;
                     default:
                         DevWarn("Unknown dwarf operator: 0x%02x\n", curInst);
                     }
             }
         m_curOperand = m_operands.begin();
     }

     /**
      * Emit current operand in given assembler file.
      *
      * @warning No checks are done on RemainingBytesToRead() for the current
      *          operand, so if you do not call this function when
      *          RemainingBytesToRead() = 0, you will write in asm_file
      *          something that may be wrong
      *
      * @param  asm_file [in/out] File where we will write the operand
      *
      * @pre    true
      * @post   current operand has been emitted in asm_file
      *
      * @return The number of emitted bytes (without comment)
      */
     int
     EmitOperand(FILE* asm_file) const
     {
         fprintf(asm_file, ", ");
         return m_curOperand->Emit(asm_file);
     }

     /**
      * Emit operator in given assembler file.
      *
      * @warning No checks are done on RemainingBytesToRead() for the operator,
      *          so if you do not call this function when RemainingBytesToRead()
      *          = 0, you will write in asm_file something that may be wrong
      *
      * @param  asm_file [in/out] File where we will write the operand
      *
      * @pre    true
      * @post   operator has been emitted in asm_file
      *
      * @return The number of emitted bytes (without comment)
      */
     int
     EmitOperator(FILE* asm_file) const
     {
         if(!Name().empty() && Dwarf_Comment)
             {
                 fprintf(asm_file, "\t%s %s\n", ASM_CMNT_LINE, Name().c_str());
             }
         fprintf(asm_file, "\t%s\t", AS_BYTE);
         return Operator().Emit(asm_file);
     }

     /**
      * Setter for m_name member.
      *
      * @param  a_name Name to be assigned to m_name
      *
      * @pre    true
      * @post   if m_name@pre->isEmpty() then m_name = a_name else
      *         m_name = m_name@pre endif
      *
      */
     void
     Name(const std::string& a_name)
     {
         if(m_name.empty())
             {
                 m_name = a_name;
             }
     }

     /**
      * Operator member.
      * Represents intruction operator. Its type is CDwarfOperand because it
      * has the same behavior of a dwarf operand (especially encoding aspect)
      */
     CDwarfOperand m_operator;

     /**
      * Is dwarf operator member.
      * Specify whether operator of the instruction is a dwarf one. In that
      * case, the operator value is interpreted as soon as it is complete to
      * set operands definition.
      */
     BOOL m_isDwarfOperator;

     /**
      * Kind member.
      * Specify whether the instruction represents a section length, an id or
      * anything else (INST). It is useful at application level to retrieve
      * current section length or kind of a section regarding the id.
      */
     EInstKind m_kind;

     /**
      * Name member.
      * Contains the name of the instruction. Its purpose is to comment
      * instruction at assembler level
      */
     std::string m_name;

     /**
      * List of operands member.
      */
     DwarfOperands m_operands;

     /**
      * Current operand member.
      * References the operand currently "under construction" when adding some
      * bytes
      */
     DwarfOperandsIt m_curOperand;
 };

//------------------------------------------------------------------------------
// Static variable class initialization
//------------------------------------------------------------------------------
 const INT CDwarfOperand::DW_BIT_BY_LEB = 7;
 const INT CDwarfOperand::MORE_BYTE = (1 << DW_BIT_BY_LEB);
 const INT CDwarfOperand::SIGN_BYTE = (1 << (DW_BIT_BY_LEB - 1));
 const INT CDwarfOperand::DATA_MASK = MORE_BYTE - 1;
 const char* CDwarfOperand::RELOC_FORMAT[] =
     {"%%dwl(__DWR%u+%u)", "%%dwh(__DWR%u+%u)", NULL};

#endif

  // scn_handles are buffer holders for section buffers.

  // The 'buffer' pointer does not own the space, but
  // simply points to space owned by other code.

  struct scn_handle {
    Dwarf_Unsigned sc_bufsize;
    Dwarf_Signed   sc_scndx;
    Dwarf_Ptr      sc_buffer;
    pSCNINFO       sc_cursection;
    BOOL           sc_output;
  };

  struct virtual_section_position {
    Dwarf_Unsigned vsp_virtpos; // byte offset in
			// a 'virtual buffer' which
			// is the same size as the total of
			// the buffers->bufsize fields.

    //Dwarf_Unsigned vsp_base; // could be used
		// for consistency check, but seems
		// unnecessary at the moment.

    Dwarf_Unsigned vsp_curbufpos;// byte offset in the current
			// real buffer

    Dwarf_Signed   vsp_remaining_buffercount; // number of remaining
			// buffers. Usually 1.

    scn_handle **  vsp_buffers; // pointer to the array of 
		// pointers to  buffers.

#ifdef TARG_STxP70
      typedef std::map<Dwarf_Unsigned, Dwarf_Unsigned> PosToPos;

      /**
       * List of expected instructions
       */
      DwarfInstructions m_instructions;

      /**
       * Total section length.
       */
      Dwarf_Unsigned m_totalSectionLength;

      /**
       * Current section length
       */
      Dwarf_Unsigned m_sectionLength;

      /**
       * Number of bytes emitted from the begining.
       * This value is updated only in vsp_print_bytes function
       */
      Dwarf_Unsigned m_sectionPos;

      /**
       * Number of emitted bytes, since the begin of current section
       */
      int m_emittedBytes;

      /**
       * Total number of emitted bytes. This variable contains the sum of all
       * value of m_emittedBytes
       */
      Dwarf_Unsigned m_totalEmittedBytes;

      /**
       * Specify whether current emitted section may have dwarf relocation or
       * not
       */
      BOOL m_hasReloc;

      /**
       * Position in assembly file of the begining of the section.
       */
      long m_sectionBegining;

      /**
       * Maps old relocation offset to new one.
       */
      PosToPos m_oldToNewSectionOffset;

      /**
       * Current instruction under construction
       */
      DwarfInstructionsIt m_curInst;
#endif

    virtual_section_position(Dwarf_Signed ct,scn_handle **buffers
#ifdef TARG_STxP70
                             , BOOL a_hasReloc = FALSE
#endif
                             )
        :
        //vsp_base(0), 
        vsp_virtpos(0),vsp_curbufpos(0),
        vsp_remaining_buffercount(ct),
        vsp_buffers(buffers)
#ifdef TARG_STxP70
        , m_instructions(), m_totalSectionLength(0), m_sectionLength(0),
        m_sectionPos(0), m_emittedBytes(0), m_totalEmittedBytes(0),
        m_hasReloc(a_hasReloc), m_sectionBegining(NOT_SET),
        m_oldToNewSectionOffset()
#endif
      {
#ifdef TARG_STxP70
          m_curInst = m_instructions.begin();
#endif
      }

    ~virtual_section_position()
      {
#ifdef TARG_STxP70
          FinalizeCurrentSection(NULL);
#endif
      }

   
    Dwarf_Unsigned vsp_get_bytes(Dwarf_Unsigned offset, int size)  
    {
	// We insist that offset march in step
	// with vsp_virtpos to ensure there is no
	// uncaught bug in the calling-client.
      FmtAssert((vsp_virtpos == offset), 
		("Error in vsp location (dwarf output)"));

      if(vsp_curbufpos >= vsp_buffers[0]->sc_bufsize) {
        // ran off the end of a buffer.


        if(vsp_remaining_buffercount < 2) {
		Fail_FmtAssertion("Ran off end of vsp buffer generating dwarf");
        }
        // we used up one buffer, proceed to the next.
        --vsp_remaining_buffercount;
      	//vsp_base += vsp_buffers[0]->sc_bufsize;
        vsp_buffers++;
      	vsp_curbufpos = 0;
      }
      // Should be pointing at valid buffer now
      if((vsp_curbufpos + size) > vsp_buffers[0]->sc_bufsize) {
	   // No field should cross buffers!
	   Fail_FmtAssertion("Impossible vsp buffer configuration");
      }
      Dwarf_Unsigned value;
      char *loc = ((char *)vsp_buffers[0]->sc_buffer) + vsp_curbufpos;

      switch(size) {
      case 1:
      	  value = loc[0];
      	  break;
      case 4:
      	    value =  ((UINT32_unaligned *)loc)->val;
      	    break;
      case 8:
      	    value =  ((UINT64_unaligned *)loc)->val;
      	    break;
      default:
      	   Fail_FmtAssertion("Impossible vsp buffer vsp_get_bytes size %d",
			(int)size);
      }
      vsp_curbufpos += size;
      vsp_virtpos += size;

      return value;
    }
    Dwarf_Unsigned vsp_print_bytes(
		FILE * asm_file,
		Dwarf_Unsigned current_reloc_target,
                Dwarf_Unsigned cur_byte_in
#ifdef TARG_ST
		, INT skip_n_bytes=0
#endif
		);

#ifdef TARG_STxP70

      /**
       * Emit relocation.
       * This function interprets the bytes of the section and rebuilds the
       * dwarf information to emit relocation for dwarf register if needed.
       * Bytes are emitted when part of the related dwarf instruction is
       * completed. I.e. emission occurs for the operator of an instruction
       * when it is completed, same for each of its operands.
       *
       * @param  asm_file [in/out] Assembler file where emission will be done
       * @param  size Number of bytes to be emitted
       * @param  cur_byte [in/out] Current position in the section. This value
       *         will be updated by reading done in that function.
       *
       * @pre    asm_file <> NULL and cur_byte = vsp_virtpos
       * @post   size bytes have been read and prepared for emission. Some of
       *         that bytes have been emitted.
       *
       */
      void
      EmitReloc(FILE* asm_file, int size, Dwarf_Unsigned& cur_byte);
      
      /**
       * Set current expected instructions right remaing bytes to read.
       * This call sets the right position in dwarf information when some bytes
       * are not emitted in vsp_print_bytes.
       *
       * @pre    number of bytes to be removed must match some complete
       *         expected instructions. I.e. removed bytes cannot be a sub part
       *         of an instruction.
       * @post   vsp_virtpos = m_sectionPos
       *
       */
      void
      RemoveEmittedInstructions();

      /**
       * Set instruction used to determine section type.
       *
       * @pre    m_totalSectionLength = m_sectionPos and
       *         m_curInst = m_instructions.end()
       * @post   m_instructions is set
       *
       */
      void
      DetermineHeader();

      /**
       * Set specification header.
       * Set in current expected instructions the header specification of new
       * section
       *
       * @param  a_isCIE Specify whether current section is a CIE or a FDE
       *
       * @pre    m_instructions.empty()
       * @post   m_instructions is set
       *
       */
      void
      ReadHeader(BOOL a_isCIE);

      /**
       * Emit new section length if it changed.
       * The emission consists in overwritting the length instruction. It is
       * possible because length instruction has a fixed size.
       *
       * @param  asm_file [in/out] Will contain new length
       *
       * @pre    true
       * @post   true
       *
       * @return The number of padding bytes needed.
       */
      int
      EmitNewLength(FILE* asm_file);

      /**
       * Emit padding bytes in given asm_file at current position.
       * The syntax of the padding bytes if specified by PADDING_STR
       *
       * @param  asm_file [in/out] Will contain padding bytes
       * @param  padding Number of padding bytes to be emitted
       *
       * @pre    true
       * @post   padding bytes have been emitted in asm_file according to
       *         PADDING_STR format
       *
       */
      void
      EmitPadding(FILE* asm_file, int padding);

      /**
       * Put an end to the redirection.
       *
       * @param  asm_file [out] Will contains the original file pointer
       * @param  symb_size_update [in] Set to TRUE if section size is symbolic
       *
       * @pre    true
       * @post   true
       *
       */
      void
      FinalizeCurrentSection(FILE* asm_file,
			     BOOL symb_size_update = FALSE);

      /**
       * Relocate given offset if needed.
       * The relocation is needed when given offset applied to a CIE pointer and
       * we have emitted some dwarf relocation, i.e. we changed the size of some
       * CIEs, FDEs. Therefore old offset is no more valid
       *
       * @param  a_ofst A relocation offset
       *
       * @pre    true
       * @post   result <> a_ofst implies given offset is applied to a CIE's
       *         pointer
       *
       * @return A relocated offset
       */
      Dwarf_Unsigned
      RelocateOffset(Dwarf_Unsigned a_ofst);
#endif

  };
}


// print the assembly form.
// return the number of bytes printed
Dwarf_Unsigned 
virtual_section_position::vsp_print_bytes(
		FILE * asm_file,
	        Dwarf_Unsigned current_reloc_target,
		Dwarf_Unsigned cur_byte_in
#ifdef TARG_ST
		, INT skip_n_bytes
#endif
		)
{

    const int bytes_per_line = 8;
    Dwarf_Unsigned cur_byte = cur_byte_in;

    int nlines_this_reloc = (current_reloc_target - cur_byte) / bytes_per_line;

#ifdef TARG_STxP70
    if(Dwarf_Old_Style_Emission || !m_hasReloc)
        {
#endif
    int i;
#ifdef TARG_ST
    if (skip_n_bytes > 0) {
      // Skip the n first bytes
      for (i=0; i<skip_n_bytes; i++) {
	vsp_get_bytes(cur_byte,1);
	++cur_byte;
      }
      nlines_this_reloc = (current_reloc_target - cur_byte) / bytes_per_line;
    }
#endif
    for (i = 0; i < nlines_this_reloc; ++i) {
      fprintf(asm_file, "\t%s\t", AS_BYTE);
      int j;
      for (j = 1; j < bytes_per_line; ++j) {
        fprintf(asm_file, "0x%02x, ",
		(int)vsp_get_bytes(cur_byte,1));
	++cur_byte;
      }
      fprintf(asm_file, "0x%02x\n", 
		(int)vsp_get_bytes(cur_byte,1));
      ++cur_byte;
    }
    if (cur_byte != current_reloc_target) {
      fprintf(asm_file, "\t%s\t", AS_BYTE);
      for (; cur_byte != current_reloc_target - 1; ) {
        fprintf(asm_file, "0x%02x, ", 
		(int)vsp_get_bytes(cur_byte,1));
	++cur_byte;
      }
      fprintf(asm_file, "0x%02x\n", 
		(int)vsp_get_bytes(cur_byte,1));
      ++cur_byte;
    }
#ifdef TARG_STxP70
        } // if Dwarf_Old_Style_Emission
    else
        {
	    DevAssert(skip_n_bytes==0, ("vsp_print_bytes(): Unexpected skip_n_bytes>0"));
            EmitReloc(asm_file, current_reloc_target - cur_byte, cur_byte);
        }
#endif
    return cur_byte - cur_byte_in;
}

#ifdef TARG_STxP70
void
virtual_section_position::EmitReloc(FILE* asm_file, int size,
                                    Dwarf_Unsigned& cur_byte)
{
    int i;
    for(i = 0; i < size; ++i)
        {
            if(m_sectionPos < vsp_virtpos)
                {
                    m_emittedBytes += vsp_virtpos - m_sectionPos;
                    RemoveEmittedInstructions();
                }
            if(m_sectionPos == m_totalSectionLength)
                {
                    DevAssert(m_curInst == m_instructions.end(),
                              ("Instruction cannot be on several section"));
                    m_instructions.clear();
                    FinalizeCurrentSection(asm_file);
                    // We are at the begin of a section (CIE or FDE), determine
                    // which one. Actually, set the instructions that will give
                    // this information
                    DetermineHeader();
                    m_sectionBegining = ftell(asm_file);
                    FmtAssert(m_sectionBegining != -1,
                              ("Error when trying to know current position in "
                               "assembly file"));
                    m_emittedBytes = 0;
                    m_curInst = m_instructions.begin();
                }
            Dwarf_Small readByte = (Dwarf_Small)(vsp_get_bytes(cur_byte, 1)
                                                 & 0xFF);
            ++cur_byte;
            ++m_sectionPos;
            // Current instruction is not set, so current byte should be an
            // instruction
            if(m_curInst == m_instructions.end())
                {
                    // Instruction emitted, seek for next one
                    m_instructions.clear();
                    // Instruction handling
                    m_instructions.push_back(CDwarfInstruction(1, TRUE));
                    m_curInst = m_instructions.begin();
                }
            m_emittedBytes += m_curInst->AddByte(asm_file, readByte);
            if(!m_curInst->GlobalRemainingBytesToRead())
                {
                    fprintf(asm_file, "\n");
                    if(m_curInst->Kind() == CDwarfInstruction::LENGTH)
                        {
                            m_sectionLength = m_curInst->Operator().Decoded();
                            m_totalSectionLength += m_sectionLength +
                                SIZEOF_SECTION_LENGTH;
                        }
                    else if(m_curInst->Kind() == CDwarfInstruction::ID)
                        {
                            BOOL isCIE =
                                m_curInst->Operator().Decoded() == g_CIE_id;
                            ReadHeader(isCIE);
                        }
                    ++m_curInst;
                }
        }
}

void
virtual_section_position::DetermineHeader()
{
    // length, uword
    m_instructions.push_back(CDwarfInstruction(SIZEOF_SECTION_LENGTH,
                                               FALSE, "length",
                                               CDwarfInstruction::LENGTH));
    
    // CIE id or FDE pointer, uword
    m_instructions.push_back(CDwarfInstruction(dw_dbg->de_offset_size,
                                               FALSE, "id or pointer",
                                               CDwarfInstruction::ID));
}

void
virtual_section_position::ReadHeader(BOOL a_isCIE)
{
    if(a_isCIE)
        {
            // CIE header specification:
            // length, uword: Already read at determine header time
            // id, uword:  Already read at determine header time

            // version, ubyte
            m_instructions.push_back(CDwarfInstruction(1, FALSE,
                                                       "CIE version"));
            // augmentation, null terminated string
            m_instructions.push_back(CDwarfInstruction(-1, FALSE,
                                                       "CIE augmentation",
                                                       CDwarfInstruction::INST,
                                                       CDwarfOperand::STRING));
            // code alignment factor, LEB128
            m_instructions.push_back(CDwarfInstruction(DW_LEB128_SIZE_IN_BYTES,
                                                       FALSE, "code alignement"
                                                       " factor",
                                                       CDwarfInstruction::INST,
                                                       CDwarfOperand::LEB128));
            // data alignement factor, LEB128
            m_instructions.push_back(CDwarfInstruction(DW_LEB128_SIZE_IN_BYTES,
                                                       FALSE, "data alignement"
                                                       " factor",
                                                       CDwarfInstruction::INST,
                                                       CDwarfOperand::LEB128));
            // return address, ubyte
            m_instructions.push_back(CDwarfInstruction(1, FALSE,
                                                       "return address"));
        }
    else
        {
            // FDE header specification
            // length, uword: Already read at determine header time
            // cie pointer, uword: Already read at determine header time

            // initial location, addressing-unit sized
            m_instructions.push_back(CDwarfInstruction(dw_dbg->de_pointer_size,
                                                       FALSE,
                                                       "initial location"));
            // address range, addressing-unit sized
            m_instructions.push_back(CDwarfInstruction(dw_dbg->de_pointer_size,
                                                       FALSE, "address range"));
        }
}

void
virtual_section_position::RemoveEmittedInstructions()
{
    if(m_curInst != m_instructions.end())
        {
            Dwarf_Unsigned offset = vsp_virtpos - m_sectionPos;
            BOOL isId = m_curInst->Kind() == CDwarfInstruction::ID;
            while(m_curInst != m_instructions.end() && offset)
                {
                    DevAssert(m_curInst->GlobalRemainingBytesToRead() >= 0,
                              ("Do not know how many bytes this instruction is "
                               "composed of"));
                    DevAssert(offset >= m_curInst->GlobalRemainingBytesToRead(),
                              ("Emitted bytes are part of an instruction, do "
                               "not how to continue"));
                    offset -= m_curInst->GlobalRemainingBytesToRead();
                    m_curInst = m_instructions.erase(m_curInst);
                }
            // If first current instruction is an identifier, this means we have
            // not determine the header kind, that's why we do not raise any
            // error, we can only assume next instruction is a dwarf one
            DevAssert(offset == 0 || isId,
                      ("do not know where we stopped the emission"));
        }
    m_sectionPos = vsp_virtpos;
}

int
virtual_section_position::EmitNewLength(FILE* asm_file)
{
    int bytesReduction = m_sectionLength + SIZEOF_SECTION_LENGTH -
        m_emittedBytes;
    DevAssert(bytesReduction >= 0, ("We emitted more bytes than the section "
                                    "length!!!"));
    int padding = 0;
    if(bytesReduction)
        {
            int newLength = m_sectionLength - bytesReduction;
            padding = newLength % SIZEOF_SECTION_LENGTH;
            newLength += padding;
            // WARNING: The instruction name must be the same previously
            // defined, since we may override written character without
            // enlarging or reducing asm_file length
            CDwarfInstruction dwInst(SIZEOF_SECTION_LENGTH, FALSE, "length",
                                     CDwarfInstruction::LENGTH);
            int emittedBytes = 0;
            DevAssert(SIZEOF_SECTION_LENGTH <= sizeof(newLength),
                      ("Type too narrow"));
            for(int i = 0; i < SIZEOF_SECTION_LENGTH; ++i)
                {
                    emittedBytes +=
                        dwInst.AddByte(asm_file, 
#ifdef WORDS_BIGENDIAN
                                       (newLength >> (sizeof(newLength) - i) * 
                                        8) & 0xff
#else
                                       (newLength >> (i * 8)) & 0xff
#endif
                                       );
                }
            DevAssert(emittedBytes == SIZEOF_SECTION_LENGTH,
                      ("Number of emitted bytes does not match definition"));
        }
    return padding;
}

void
virtual_section_position::EmitPadding(FILE* asm_file, int padding)
{
    int i;
    for(i = 0; i < padding; ++i)
        {
            fprintf(asm_file, PADDING_STR, AS_BYTE);
        }
}

void
virtual_section_position::FinalizeCurrentSection(FILE* asm_file, BOOL symb_size_update)
{
    if(m_sectionBegining != NOT_SET)
        {
            DevAssert(asm_file, ("Final emission, if not null, required a valid"
                                 " file"));
            // Go at the begining of the section
            int result = fseek(asm_file, m_sectionBegining, SEEK_SET);
            FmtAssert(!result, ("Cannot update section length"));
            // vsp_virtpos - m_sectionPos == 0 except for the last emission
            // of an empty fde
            m_emittedBytes += vsp_virtpos - m_sectionPos;
            // Overwrite the length
            int padding;
	    if (symb_size_update) {
	      padding = 0; // Padding done in Cg_Dwarf_Output_Asm_Bytes_Sym_Relocs()
	    }
	    else {
	      padding = EmitNewLength(asm_file);
	    }
            // Go at the end of the section
            result = fseek(asm_file, 0, SEEK_END);
            FmtAssert(!result, ("Cannot update section padding"));
            // Emit additional padding
            EmitPadding(asm_file, padding);
            m_sectionBegining = NOT_SET;
        }
    m_totalEmittedBytes += m_emittedBytes;
    m_oldToNewSectionOffset[m_sectionPos] = m_totalEmittedBytes;
}

Dwarf_Unsigned
virtual_section_position::RelocateOffset(Dwarf_Unsigned a_ofst)
{
    if(!Dwarf_Old_Style_Emission && m_hasReloc &&
       m_curInst != m_instructions.end() &&
       m_curInst->Kind() == CDwarfInstruction::ID)
        {
            // We move on the next instruction because if we are here, the id
            // instruction has been emitted without using the vsp_print_bytes
            // method and next relocation offset will not applied to the id
            ++m_curInst;
            DevAssert(m_oldToNewSectionOffset.find(a_ofst) !=
                      m_oldToNewSectionOffset.end(),
                      ("Current offset relocation must have been set in "
                       "EmitReloc function!"));
            a_ofst = m_oldToNewSectionOffset[a_ofst];
        }
    return a_ofst;
}

/**
 * Specify whether given section has relocation (dwarf register relocation) to
 * be emitted.
 *
 * @param  section_name Name of the section to be checked
 *
 * @pre    true
 * @post   result implies vsp_print_bytes should handle relocation
 *
 * @return TRUE is section may have relocation to be emitted, FALSE otherwise
 */
static BOOL
HasReloc(const char* section_name)
{
    return strcmp(section_name, ".debug_frame") == 0;
}
#endif

//
// Find the list of sections that are section secnum
// caller must pass in array scn_bufferp, and it must
// be large enough.
static Dwarf_Signed 
match_scndx(Elf64_Word scndx,
                scn_handle * scn_handles /* input */,
                Dwarf_Unsigned section_count /* input */,
                scn_handle** scn_bufferp /* output info */)
{
  for (Dwarf_Signed i = 0; i < section_count; i++) {
    if (scn_handles[i].sc_scndx == scndx) {
      Dwarf_Signed k = 0;
      scn_bufferp[k] =  scn_handles +i;
      ++i;
      ++k;
      while( i < section_count && scn_handles[i].sc_scndx == scndx  ){
         scn_bufferp[k] = scn_handles +i;
	 ++k;
	 ++i;
      }
      return k;
    }
  }
  Fail_FmtAssertion("Bogus section index %ld\n", scndx);
  /*NOTREACHED*/
}


// We need the size of a virtual buffer
// so we can use it to check the validity of
// relocation offsets.
static Dwarf_Unsigned
compute_buffer_net_size(Dwarf_Signed   buffer_cnt,
  scn_handle **  buffers)
{
	if(buffer_cnt < 1) {
		return 0;
	}
	Dwarf_Unsigned next = 0;
	Dwarf_Unsigned totsize = 0;
	for( ; next < buffer_cnt; ++next) {
		totsize += buffers[next]->sc_bufsize;
	}
	return totsize;
}


// Emit assembly code using elf-relocations
// created by libdwarf.

// At present, this is only used  in a limited way 
// for MIPS/SGI for assembly output (if ever used), and
// is really a holdover approach.
// The more general code is Cg_Dwarf_Output_Asm_Bytes_Sym_Relocs.

// See Cg_Dwarf_Output_Asm_Bytes_Sym_Relocs
// for comments on the reason for
// the virtual_section_position class.
static void
Cg_Dwarf_Output_Asm_Bytes_Elf_Relocs (FILE          *asm_file,
				      const char    *section_name,
				      Elf64_Word     section_type,
				      Elf64_Word     section_flags,
				      Elf64_Word     section_entsize,
				      Elf64_Word     section_align,
				      Dwarf_Signed   buffer_cnt,
				      scn_handle **  buffers,
				      Elf64_Word     reloc_scn_type,
				      Dwarf_Ptr      reloc_buffer,
				      Dwarf_Unsigned reloc_buffer_size,
				      BOOL           is_64bit)
{

  const Dwarf_Unsigned buffer = 0; // Leave at 0: is virtual buffer

  CGEMIT_Prn_Scn_In_Asm(asm_file, section_name, section_type,
			section_flags, section_entsize,
			section_align, NULL);


  Dwarf_Unsigned bufsize =  // compute size of virtual buffer
	compute_buffer_net_size(buffer_cnt, buffers);

  check_reloc_fmt_and_size(reloc_scn_type,
			   reloc_buffer,
			   reloc_buffer_size,
			   bufsize);

  Dwarf_Unsigned cur_byte =  buffer;

  // initialize position
  virtual_section_position vsp(buffer_cnt, buffers
#ifdef TARG_STxP70
                               , HasReloc(section_name)
#endif
                               );

  char * current_reloc = (char *) reloc_buffer;

	// With MIPS binary relocation output, the size is 4 or 8,
	// never the cygnus semi-64-bit, so we do not need to support
	// that here  (and don't have the data to do so).
  UINT current_reloc_size = (is_64bit ? 8 : 4);

  do {
    Dwarf_Unsigned current_reloc_target;
    if (current_reloc != ((char *) reloc_buffer) + reloc_buffer_size) {
      current_reloc_target = (buffer) + reloc_offset(current_reloc,
							      is_64bit);
    }
    else {
      current_reloc_target = buffer + bufsize;
    }
#if defined(Is_True_On)
    if ((UINT64) current_reloc_target < (UINT64) cur_byte) {
      fprintf(stderr, "ERROR: relocation records not sorted\n");
      exit(-1);
    }
#endif

    cur_byte += vsp.vsp_print_bytes(asm_file,current_reloc_target,cur_byte);

    if (cur_byte != (buffer) + bufsize) {
#if defined(Is_True_On)
      // Check that if the final bytes are relocated, the relocation
      // doesn't overrun the end of the buffer.
      if ((UINT64) current_reloc_target + current_reloc_size >
	  (UINT64) buffer + bufsize) {
	fprintf(stderr, "ERROR: relocation record overruns end of section data\n");
	exit(-1);
      }
#endif

      //
      // Now put out the symbol reference plus offset for the relocation.
      //
      Elf64_Word current_reloc_sym_index = reloc_sym_index(current_reloc,
							   is_64bit);
      char *current_reloc_sym_name = Em_Get_Symbol_Name(current_reloc_sym_index);
      Dwarf_Unsigned ofst;

      fprintf(asm_file, "\t%s\t%s", AS_ADDRESS,
	      current_reloc_sym_name);

      switch (reloc_scn_type) {
      case SHT_REL:
	if (is_64bit) {
	  Check_Dwarf_Rel(*((Elf64_Rel *) current_reloc));
          FmtAssert(current_reloc_size == sizeof(UINT64),
			("reloc size error"));
	  ofst = vsp.vsp_get_bytes(cur_byte,sizeof(UINT64));
	
	  current_reloc += sizeof(Elf64_Rel);
	}
	else {
	  Check_Dwarf_Rel(*((Elf32_Rel *) current_reloc));
          FmtAssert(current_reloc_size == sizeof(UINT32),
			("reloc size error"));
	  ofst = vsp.vsp_get_bytes(cur_byte,sizeof(UINT32));

	  current_reloc += sizeof(Elf32_Rel);
	}
	break;
      case SHT_RELA:
	if (is_64bit) {
	  Check_Dwarf_Rela(*((Elf64_Rela *) current_reloc));

	  // position vsp to match cur_byte, discard value
	  vsp.vsp_get_bytes(cur_byte,sizeof(current_reloc_size));

	  ofst = ((Elf64_Rela *) current_reloc)->r_addend;
	  current_reloc += sizeof(Elf64_Rela);
	}
	else {
	  Check_Dwarf_Rela(*((Elf32_Rela *) current_reloc));

	  // position vsp to match cur_byte, discard value
	  vsp.vsp_get_bytes(cur_byte,sizeof(current_reloc_size));

	  ofst = ((Elf32_Rela *) current_reloc)->r_addend;
	  current_reloc += sizeof(Elf32_Rela);
	}
	break;
      default:
	fprintf(stderr,
		"ERROR: unrecognized relocation section type: %u\n",
		reloc_scn_type);
	exit (-1);
      }
#ifdef TARG_STxP70
      ofst = vsp.RelocateOffset(ofst);
#endif
      if (ofst != 0) {
	fprintf(asm_file, " + 0x%llx", (unsigned long long)ofst);
      }
      fprintf(asm_file, "\n");
      cur_byte += current_reloc_size;
    }
  } while (cur_byte != ( buffer) + bufsize);

#ifdef TARG_STxP70
  vsp.FinalizeCurrentSection(asm_file);
#endif
  fflush(asm_file);
}

// Symbolic assembler output where the input
// is symbolic relocations created by libdwarf
// (as opposed to the elf binary relocations
// processed by Cg_Dwarf_Output_Asm_Bytes_Elf_Relocs).

// Each relocation section is complete (only one buffer).
// However, the data bytes can be split across
// multiple data buffers!
// Note that for binary Elf output, the bytes were just added
// to the end of an elf section, so having multiple buffers
// was no problem.
// Here, we are working with the bytes, and
// we have to carefully recognize that the relocations
// apply to the mythical 'complete' buffer.
// To make that work in the simplest way, 
// we do most of the work in terms
// of a mythical 'complete' buffer, and the
// virtual_section_position class does the messy bits.
//
static void
Cg_Dwarf_Output_Asm_Bytes_Sym_Relocs (FILE                 *asm_file,
				      const char           *section_name,
				      Elf64_Word            section_type,
				      Elf64_Word            section_flags,
				      Elf64_Word            section_entsize,
				      Elf64_Word            section_align,
				      Dwarf_Signed          buffer_cnt,
                                      scn_handle        **  buffers,
				      Dwarf_Relocation_Data reloc_buffer,
				      Dwarf_Unsigned        reloc_count,
				      BOOL                  is_64bit)
{
  // [re]establish the section this is part of.
  //
  CGEMIT_Prn_Scn_In_Asm(asm_file, section_name, section_type,
			section_flags, section_entsize,
			section_align, NULL);

#ifdef TARG_ST
  char begin_size_label[64];
  char end_size_label[64];
  char end_size_nopad_label[64];
  BOOL need_symbolic_size = Cg_Dwarf_Section_Need_Symbolic_Size(section_name);
  if (need_symbolic_size) {
    // [TTh] Statically computed section size might be incorrect
    // -> Force symbolic computation resolved at assembly time
    sprintf(begin_size_label, "L_begin_size_section_%s", &section_name[1]); // Skip '.' prefix
    sprintf(end_size_label, "L_end_size_section_%s", &section_name[1]);     // Skip '.' prefix
    sprintf(end_size_nopad_label, "L_end_size_nopad_section_%s", &section_name[1]);     // Skip '.' prefix
    FmtAssert((SIZEOF_SECTION_LENGTH==4), ("Unsupported SIZEOF_SECTION_LENGTH"));
    fprintf(asm_file, "\t%s\t%s - %s\n", AS_WORD_UNALIGNED, end_size_nopad_label, begin_size_label);
    fprintf(asm_file, "%s:\n", begin_size_label);
  }
#endif

  const Dwarf_Unsigned buffer   = 0; // virtual buffer, so 0 ok.
  Dwarf_Unsigned cur_byte =  buffer;
  Dwarf_Unsigned bufsize =  compute_buffer_net_size(buffer_cnt,buffers);

  virtual_section_position vsp(buffer_cnt, buffers
#ifdef TARG_STxP70
                               , HasReloc(section_name)
#endif
                               );

  Dwarf_Unsigned k = 0;
  while (k <= reloc_count) {
    // The last time thru, k == reloc_count which
    // emits the final part of the buffer (after the last reloc).
    // There is no relocation record for k == reloc_count as
    // the relocation records are numbered 0 thru  (reloc_count-1).

    Dwarf_Unsigned current_reloc_target;
    if (k != reloc_count) {
      current_reloc_target = ( buffer) + reloc_buffer[k].drd_offset;

    }
    else {
      current_reloc_target = ( buffer) + bufsize;
    }
#if defined(Is_True_On)
    Is_True((UINT64) current_reloc_target >= (UINT64) cur_byte,
	    ("Relocation records not sorted\n"));
#endif

    UINT current_reloc_size = is_64bit?8:4; 

    if((reloc_count > 0) && reloc_buffer && k < reloc_count) {
	// For cygnus semi-64-bit dwarf.
	// targets of relocs in semi-64-bit vary.
	// Some sections have no relocs and reloc_buffer can be null
	// in such cases.
      current_reloc_size = reloc_buffer[k].drd_length;
    }

#ifdef TARG_ST
    // When generating section size as symbolic, the total length bytes must not be dumped
    // by vsp_print_bytes()
    BOOL skip_n_bytes = (cur_byte == 0 && need_symbolic_size)?SIZEOF_SECTION_LENGTH:0;
    cur_byte += vsp.vsp_print_bytes(asm_file,current_reloc_target,cur_byte, skip_n_bytes);
#endif

    if (cur_byte != (buffer) + bufsize) {
#if defined(Is_True_On)
      // Check that if the final bytes are relocated, the relocation
      // doesn't overrun the end of the buffer.
      Is_True((UINT64) current_reloc_target + current_reloc_size <=
	      (UINT64) buffer + bufsize,
	      ("Relocation record overruns end of section data\n"));
#endif

      //
      // Now put out the symbol reference plus offset for the relocation.
      //
      // If pointer-length reloc, use data8.ua, else dwarf offset
      // size to be relocated, use data4.ua
#ifdef TARG_ST
      // [TTh] Added a third type of reloc, that uses LEB128 encoding
      const char *reloc_name;
      if (reloc_buffer[k].drd_type == dwarf_drt_first_of_length_pair_inst_word) {
	reloc_name = AS_SLEB128;
      }
      else {
        reloc_name = ((reloc_buffer[k].drd_length == 8)?AS_ADDRESS_UNALIGNED: AS_WORD_UNALIGNED);
      }
#else
      char *reloc_name = (reloc_buffer[k].drd_length == 8)?
			AS_ADDRESS_UNALIGNED: AS_WORD_UNALIGNED;
#endif

#ifdef TARG_STxP70
      if(vsp.m_instructions.end() != vsp.m_curInst &&
         vsp.m_curInst->Operator().RemainingBytesToRead() == 0)
          {
              // operator of current instruction has been emitted but this is
              // not the case for all operands. So remaining operands may be
              // emitted here, with a new assembler directive, that must be at
              // the begining of the line.
              fprintf(asm_file, "\n");
          }
#endif

      switch (reloc_buffer[k].drd_type) {
      case dwarf_drt_none:
	break;
      case dwarf_drt_data_reloc:
#ifdef TARG_ST
      case dwarf_drt_data_reloc_pcrel:
#ifdef AS_DWARF_THREAD_DATA_RELOC
	if (Cg_Dwarf_Type_From_Handle(reloc_buffer[k].drd_symbol_index)
	    == STT_TLS)
	  fprintf(asm_file, "\t%s\t%s(%s)", reloc_name,
		  AS_DWARF_THREAD_DATA_RELOC,
		  Cg_Dwarf_Name_From_Handle(reloc_buffer[k].drd_symbol_index));
	else
#endif
#endif
	  fprintf(asm_file, "\t%s\t%s", reloc_name,
		  Cg_Dwarf_Name_From_Handle(reloc_buffer[k].drd_symbol_index));
#ifdef TARG_ST
	if (reloc_buffer[k].drd_type == dwarf_drt_data_reloc_pcrel)
	  fprintf(asm_file, " - .");
#endif
	break;

      case dwarf_drt_segment_rel:
#ifdef TARG_ST
      case dwarf_drt_segment_rel_pcrel:
#endif
	{
	  // need unaligned AS_ADDRESS for dwarf, so add .ua
	  fprintf(asm_file, "\t%s\t%s", reloc_name,
	          Cg_Dwarf_Name_From_Handle(reloc_buffer[k].drd_symbol_index));
#ifdef TARG_ST
	if (reloc_buffer[k].drd_type == dwarf_drt_segment_rel_pcrel)
	  fprintf(asm_file, " - .");
#endif
	  break;
	}
#ifdef KEY
      case dwarf_drt_cie_label: // bug 2463
        fprintf(asm_file, "\t%s\t%s", reloc_name, ".LCIE");
	++k; // skip the DEBUG_FRAME label that is there just as a place-holder
	break;
      case dwarf_drt_data_reloc_by_str_id:
#ifdef TARG_ST
      case dwarf_drt_data_reloc_pcrel_by_str_id:
#endif
	// it should be __gxx_personality_v0
#ifndef TARG_ST
        // (cbr) on st200 we use PCabs
        if ((Gen_PIC_Call_Shared || Gen_PIC_Shared) && 
	     !strcmp (&Str_Table[reloc_buffer[k].drd_symbol_index], 
	     "__gxx_personality_v0"))
	    fprintf (asm_file, "\t%s\tDW.ref.__gxx_personality_v0-.", reloc_name);
 	else
#endif
	fprintf(asm_file, "\t%s\t%s", reloc_name,
		&Str_Table[reloc_buffer[k].drd_symbol_index]);
#ifdef TARG_ST
	if (reloc_buffer[k].drd_type == dwarf_drt_data_reloc_pcrel_by_str_id)
	  fprintf(asm_file, " - .");
#endif
	break;
      case dwarf_drt_first_of_length_pair_create_second:
	{
	static int count=1;
	Is_True(k + 1 < reloc_count, ("unpaired first_of_length_pair"));
	Is_True((reloc_buffer[k + 1].drd_type ==
		 dwarf_drt_second_of_length_pair),
		("unpaired first_of_length_pair"));
	int this_count=count++;
	fprintf(asm_file,".FDE%d:\n",this_count);
	// bug 2729
	fprintf(asm_file, "\t%s\t.FDE%d - .EHCIE", reloc_name, this_count);
	++k;
	}
	break;
#endif // KEY
      case dwarf_drt_first_of_length_pair:
	Is_True(k + 1 < reloc_count, ("unpaired first_of_length_pair"));
	Is_True((reloc_buffer[k + 1].drd_type ==
		 dwarf_drt_second_of_length_pair),
		("unpaired first_of_length_pair"));
	// need unaligned AS_ADDRESS for dwarf, so add .ua
	fprintf(asm_file, "\t%s\t%s - %s", reloc_name,
		Cg_Dwarf_Name_From_Handle(reloc_buffer[k + 1].drd_symbol_index),
		Cg_Dwarf_Name_From_Handle(reloc_buffer[k].drd_symbol_index));
	++k;
	break;
#ifdef TARG_ST
	// [TTh] Relocation : .sleb128 (end - begin) / Min_Inst_word
      case dwarf_drt_first_of_length_pair_inst_word:
	{
	  Is_True(k + 1 < reloc_count, ("unpaired first_of_length_pair_inst_word"));
	  Is_True((reloc_buffer[k + 1].drd_type ==
		   dwarf_drt_second_of_length_pair_inst_word),
		  ("unpaired first_of_length_pair_inst_word"));
	  // Divide using shift, because '/' not supported by gnuasm...
	  INT shift=0;
	  switch(ISA_PACK_INST_WORD_SIZE / 8) {
	  case 1: shift = 0; break;
	  case 2: shift = 1; break;
	  case 4: shift = 2; break;
	  default: Is_True((0), ("Unsupported inst word for first_of_length_pair_inst_word")); break;
	  }
	  const char *sym_start = Cg_Dwarf_Name_From_Handle(reloc_buffer[k].drd_symbol_index);
	  const char *sym_end = Cg_Dwarf_Name_From_Handle(reloc_buffer[k + 1].drd_symbol_index);
	  if (ASM_SLEB128_SUPPORTED()) {
	    fprintf(asm_file, "\t%s\t(%s - %s) >> %d", reloc_name,
		    sym_end, sym_start, shift);
	  }
	  else {
	    // Manual generation of SLEB128 encoding when not supported by assembler
	    reloc_name = AS_BYTE;
	    fprintf(asm_file, "\t%s\t0x80 + (((%s - %s) >> %d) & 0x7f)", reloc_name,
		    sym_end, sym_start, shift);
	    fprintf(asm_file, ", 0x80 + (((%s - %s) >> (%d+7)) & 0x7f)",
		    sym_end, sym_start, shift);
	    fprintf(asm_file, ", 0x80 + (((%s - %s) >> (%d+14)) & 0x7f)",
		    sym_end, sym_start, shift);
	    fprintf(asm_file, ", (((%s - %s) >> (%d+21)) & 0x7f)",
		    sym_end, sym_start, shift);
	  }
	  ++k;
	}
	break;
#endif
      case dwarf_drt_second_of_length_pair:
	Fail_FmtAssertion("unpaired first/second_of_length_pair");
	break;
      default:
        printf ("arg cgdwarf.cxx\n");
	break;
      }

      Dwarf_Unsigned ofst;
      Dwarf_Unsigned ofst_tmp;
      if (current_reloc_size == 8) {
	// 64 bit target of relocation.
	ofst = vsp.vsp_get_bytes(cur_byte,8);
      }
      else if(current_reloc_size == 4){
	// 32 bit target of relocation, which can happen
	// with 32 bit target or with
	// cygnus semi-64-bit dwarf.
	ofst = vsp.vsp_get_bytes(cur_byte,4);
#ifdef TARG_ST
      } else if (current_reloc_size == LEB128_SYMBOLIC_RELOC_DUMMY_SIZE) {
	ofst = vsp.vsp_get_bytes(cur_byte,LEB128_SYMBOLIC_RELOC_DUMMY_SIZE);
#endif
      } else {
	Fail_FmtAssertion("current_reloc_size %ld, 4 or 8 required!\n", 
		(long)current_reloc_size);
      }
     
      /* Handle endianness issues */
      if (Target_Byte_Sex != Host_Byte_Sex) {
	ofst_tmp = 0;
	for (int i=0; i<current_reloc_size; i++) {
	  ofst_tmp <<= 8;
	  ofst_tmp |= ofst & 0xFF;
	  ofst >>= 8;
	}
	ofst = ofst_tmp;
      }

#ifdef TARG_STxP70
      ofst = vsp.RelocateOffset(ofst);
#endif
#ifdef TARG_ST
      if (reloc_buffer[k].drd_type == dwarf_drt_none)
        fprintf(asm_file, "\t%s 0x%llx", reloc_name, (unsigned long long)ofst);
      else
#endif
        if (ofst != 0) {
          fprintf(asm_file, " + 0x%llx", (unsigned long long)ofst);
        }
      fprintf(asm_file, "\n");
      cur_byte += current_reloc_size;
    }
    ++k;
  }
#ifdef TARG_STxP70
  vsp.FinalizeCurrentSection(asm_file, need_symbolic_size);
#endif

#ifdef TARG_ST
  if (need_symbolic_size) {
    // [TTh] Add symbol for symbolic computation of size
    // and generate potential padding
    fprintf(asm_file, "%s:\n", end_size_nopad_label);
    fprintf(asm_file, "\t%s\t(%d - (%s - %s ) %% %d) %% %d\n", AS_SPACE, dw_dbg->de_offset_size,
	    end_size_nopad_label, begin_size_label, dw_dbg->de_offset_size, dw_dbg->de_offset_size );
    fprintf(asm_file, "%s:\n", end_size_label);
  }
#endif
  fflush(asm_file);
}


#if !defined(LIBDWARF_SORTS_RELOCS)
static int compare_rel64(const void *const a, const void *const b)
{
  if (REL_offset(*((Elf64_Rel *) a)) < REL_offset(*((Elf64_Rel *) b))) {
    return -1;
  }
  else if (REL_offset(*((Elf64_Rel *) a)) > REL_offset(*((Elf64_Rel *) b))) {
    return 1;
  }
  else {
    return 0;
  }
}

static int compare_rela64(const void *const a, const void *const b)
{
  if (REL_offset(*((Elf64_Rela *) a)) < REL_offset(*((Elf64_Rela *) b))) {
    return -1;
  }
  else if (REL_offset(*((Elf64_Rela *) a)) > REL_offset(*((Elf64_Rela *) b))) {
    return 1;
  }
  else {
    return 0;
  }
}

static int compare_rel32(const void *const a, const void *const b)
{
  if (REL32_offset(*((Elf32_Rel *) a)) < REL32_offset(*((Elf32_Rel *) b))) {
    return -1;
  }
  else if (REL32_offset(*((Elf32_Rel *) a)) > REL32_offset(*((Elf32_Rel *) b))) {
    return 1;
  }
  else {
    return 0;
  }
}

static int compare_rela32(const void *const a, const void *const b)
{
  if (REL32_offset(*((Elf32_Rela *) a)) < REL32_offset(*((Elf32_Rela *) b))) {
    return -1;
  }
  else if (REL32_offset(*((Elf32_Rela *) a)) > REL32_offset(*((Elf32_Rela *) b))) {
    return 1;
  }
  else {
    return 0;
  }
}
#endif

void
Cg_Dwarf_Write_Assembly_From_Elf (FILE *asm_file,
				  INT   section_count,
				  BOOL  is_64bit)
{
  Dwarf_Signed i;
  Dwarf_Ptr buffer;
  Dwarf_Signed scndx;
  Dwarf_Unsigned bufsize;
  pSCNINFO cursection;
  scn_handle *scn_handles = 
	(scn_handle *) malloc(section_count * sizeof(scn_handle));
  FmtAssert(scn_handles != 0,
	("malloc space for scn_handles failed"));
  scn_handle *s = scn_handles;

  scn_handle ** scn_buffers =
	(scn_handle **) malloc(section_count * sizeof(scn_handle *));
  FmtAssert(scn_buffers != 0,
	("malloc space for scn_buffers failed"));

  dwarf_reset_section_bytes(dw_dbg);

  for (i = 0; i < section_count; i++) {
    s = scn_handles + i;
    buffer = dwarf_get_section_bytes (dw_dbg, i, &scndx, &bufsize, &dw_error);
    cursection = Em_Dwarf_Find_Dwarf_Scn (scndx);
    if (cursection != NULL) {
      s->sc_buffer     = buffer;
      s->sc_bufsize    = bufsize;
      s->sc_scndx      = scndx;
      s->sc_cursection = cursection;
      s->sc_output     = FALSE;
    }
    else {
      fprintf (stderr, "ERROR No such section index: %d\n", (int)scndx);
      exit(-1);
    }
  }

  // BUG in the following loop: We don't output sections for which
  // there is no corresponding relocation section!
  for (i = 0; i < section_count; i++) {
    pSCNINFO relsection = scn_handles[i].sc_cursection;
    Elf64_Word relsection_type = Em_Get_Section_Type(relsection);

    if (relsection_type == SHT_REL || relsection_type == SHT_RELA) {
      // Sort the relocation records by offset
#if defined(LIBDWARF_SORTS_RELOCS)
      char *reloc_sorted = (char *) scn_handles[i].sc_buffer;
#else
      char *reloc_sorted = (char *) malloc(scn_handles[i].sc_bufsize);
      reloc_sorted = (char *) memcpy((void *) reloc_sorted,
				     (void *) scn_handles[i].sc_buffer,
				     (size_t) scn_handles[i].sc_bufsize);
      size_t n_relocs;
      if (relsection_type == SHT_REL) {
	if (is_64bit) {
	  n_relocs = scn_handles[i].sc_bufsize / sizeof(Elf64_Rel);
	  qsort(reloc_sorted, n_relocs, sizeof(Elf64_Rel), compare_rel64);
	}
	else {
	  n_relocs = scn_handles[i].sc_bufsize / sizeof(Elf32_Rel);
	  qsort(reloc_sorted, n_relocs, sizeof(Elf32_Rel), compare_rel32);
	}
      }
      else {
	if (is_64bit) {
	  n_relocs = scn_handles[i].sc_bufsize / sizeof(Elf64_Rela);
	  qsort(reloc_sorted, n_relocs, sizeof(Elf64_Rela), compare_rela64);
	}
	else {
	  qsort(reloc_sorted, n_relocs, sizeof(Elf32_Rela), compare_rela32);
	}
      }
#endif

      // Get the sections that are the target of the relocations
      // (cursection).

      Elf64_Word relsection_info = Em_Get_Section_Info(relsection);

      Dwarf_Signed scn_count  = match_scndx(relsection_info, 
		scn_handles, 
		section_count,
		scn_buffers);
      cursection                 = scn_buffers[0]->sc_cursection;

      char *section_name = Em_Get_Section_Name(cursection);
      Elf64_Word section_type  = Em_Get_Section_Type(cursection);
      Elf64_Word section_flags = Em_Get_Section_Flags(cursection);
      Elf64_Word section_entsize = Em_Get_Section_Entsize(cursection);
      Elf64_Word section_align = Em_Get_Section_Align(cursection);
      Cg_Dwarf_Output_Asm_Bytes_Elf_Relocs (asm_file, section_name,
					    section_type, section_flags,
					    section_entsize, section_align,
					    scn_count,
					    scn_buffers,
					    relsection_type,
					    reloc_sorted,
					    scn_handles[i].sc_bufsize,
					    is_64bit);
#if !defined(LIBDWARF_SORTS_RELOCS)
      free(reloc_sorted);
#endif
    }
  }
  free(scn_handles);
  scn_handles = NULL;
  free(scn_buffers);
  scn_buffers = 0;
}

void
Cg_Dwarf_Write_Assembly_From_Symbolic_Relocs (FILE *asm_file,
					      INT   section_count,
					      BOOL  is_64bit)
{
  Dwarf_Signed i;
  Dwarf_Ptr buffer;
  Dwarf_Signed scndx;
  Dwarf_Unsigned bufsize;
  pSCNINFO cursection;
  scn_handle *scn_handles;
  scn_handle *s;

  scn_handles = (scn_handle *) malloc(section_count * sizeof(scn_handle));
  FmtAssert(scn_handles != 0,
	("malloc space for scn_handles failed"));
  scn_handle ** scn_buffers =
        (scn_handle **) malloc(section_count * sizeof(scn_handle *));
  FmtAssert(scn_buffers != 0,
	("malloc space for scn_buffers failed"));


  dwarf_reset_section_bytes(dw_dbg);

  for (i = 0; i < section_count; i++) {
    s = scn_handles + i;

    buffer = dwarf_get_section_bytes (dw_dbg, i, &scndx, &bufsize, &dw_error);
    cursection = Em_Dwarf_Find_Dwarf_Scn (scndx);
    if (cursection != NULL) {
      s->sc_buffer     = buffer;
      s->sc_bufsize    = bufsize;
      s->sc_scndx      = scndx;
      s->sc_cursection = cursection;
      s->sc_output     = FALSE;
    } else {
      fprintf (stderr, "ERROR No such section index: %d\n", (int)scndx);
      exit(-1);
    }
  }

  // Now get the relocation buffers, translate each one into an ELF
  // relocation section, and write the ELF section to the output file.
  Dwarf_Unsigned relocation_section_count;
  int            reloc_buffer_version;
  int result = dwarf_get_relocation_info_count(dw_dbg,
					       &relocation_section_count,
					       &reloc_buffer_version,
					       &dw_error);
  FmtAssert(result == DW_DLV_OK,
	    ("Failure to get relocation info count"));
  Is_True(reloc_buffer_version == 2,
	  ("Symbolic relocation format mismatch"));

  for (i = 0; i < relocation_section_count; i++) {
    Dwarf_Relocation_Data reloc_buf;
    Dwarf_Unsigned entry_count;
    Dwarf_Signed link_scn;

    result = dwarf_get_relocation_info(dw_dbg, &scndx, &link_scn,
				       &entry_count, &reloc_buf,
				       &dw_error);
    FmtAssert(result == DW_DLV_OK,
	      ("Failure to get relocation info"));
    // Get the section that is the target of the relocations
    // (cursection).

    Dwarf_Signed scn_count  = match_scndx(link_scn,
                scn_handles,
                section_count,
                scn_buffers);


    cursection = scn_buffers[0]->sc_cursection;

    char *section_name = Em_Get_Section_Name(cursection);
    // Assembler generates .debug_line from directives itself, so we
    // don't output it or others not needed.
    if(TRUE == Is_Dwarf_Section_To_Emit(section_name)) {
      Elf64_Word section_type  = Em_Get_Section_Type(cursection);
      Elf64_Word section_flags = Em_Get_Section_Flags(cursection);
      Elf64_Word section_entsize = Em_Get_Section_Entsize(cursection);
      Elf64_Word section_align = Em_Get_Section_Align(cursection);
      Cg_Dwarf_Output_Asm_Bytes_Sym_Relocs (asm_file, section_name,
					    section_type, section_flags,
					    section_entsize, section_align,
					    scn_count,
					    scn_buffers,
					    reloc_buf,
					    entry_count, is_64bit);
    }
    for(int i = 0; i < scn_count; ++i) {
	scn_buffers[i]->sc_output = TRUE;
    }
  }

  for (i = 0; i < section_count; i++) {
    if (!scn_handles[i].sc_output) {
      cursection = scn_handles[i].sc_cursection;
      char *section_name = Em_Get_Section_Name(cursection);
      scn_handle *one_han = scn_handles +i;
      // Assembler generates .debug_line from directives itself, so we
      // don't output it.
      if(TRUE == Is_Dwarf_Section_To_Emit(section_name)) {
	Elf64_Word section_type  = Em_Get_Section_Type(cursection);
	Elf64_Word section_flags = Em_Get_Section_Flags(cursection);
	Elf64_Word section_entsize = Em_Get_Section_Entsize(cursection);
	Elf64_Word section_align = Em_Get_Section_Align(cursection);
	Cg_Dwarf_Output_Asm_Bytes_Sym_Relocs (asm_file, section_name,
					      section_type, section_flags,
					      section_entsize, section_align,
					      /* list of one entry */1,
					      /* the list entry */ &one_han,
					      /* reloc buff= */ NULL, 
						/* reloc count = */0, 
						is_64bit);
      }
    }
  }

  free(scn_handles);
  scn_handles = NULL;
  free(scn_buffers);
  scn_buffers = 0;
}
