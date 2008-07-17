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


#ifdef _KEEP_RCS_ID
static const char source_file[] = __FILE__;
#endif

#include <stdio.h>

#include <elf.h>
#include <libelf.h>

#define	USE_STANDARD_TYPES 1
#include "defs.h"
#include "erglob.h"
#include "srcpos.h"
#include "em_elf.h"
#include "em_dwarf.h"
#include "config_elf_targ.h"
#include "targ_em_dwarf.h"
#ifdef TARG_ST
#include "targ_isa_pack.h"
#endif

#include "config.h"     /* For Target_Byte_Sex */

INT data_alignment_factor;

static Dwarf_P_Debug dw_dbg;
static Dwarf_Error dw_error;

#define MAX_DWARF_SECTIONS 20
static pSCNINFO dwarf_scn[MAX_DWARF_SECTIONS];
static size_t num_dwarf_scns = 0;
/* Allocate only one cie for the whole file. */
static Dwarf_Unsigned cie_index;
#ifdef KEY
static Dwarf_Unsigned eh_cie_index;
#endif

#ifdef TARG_ST
static MEM_POOL mempool;
#endif

typedef struct {
  UINT16 dwarf_idx;
  char *path;
} INCL;

typedef struct {
  UINT16 dwarf_idx;
  UINT16 incl_idx;
  char *filename;
} FNAME;

static INCL *incl_table;
static INT incl_table_size;
static FNAME *file_table;
static INT file_table_size;

static void
identity_function(Dwarf_Unsigned symidx,
		  Dwarf_Unsigned *elf_symidx,
		  Dwarf_Unsigned *elf_symoff)
{
  *elf_symidx = symidx;
  *elf_symoff = 0;
}

static Dwarf_Unsigned
another_identity_function(Elf64_Word x)
{
  return x;
}

static symbol_index_recorder Record_Symidx;

static int /* TODO: should be size_t */
setup_new_section_for_dwarf (
    char *name,
    int is_64bit,
    Dwarf_Unsigned type,
    Dwarf_Unsigned flags,
    Dwarf_Unsigned link,
    Dwarf_Unsigned info,
    Dwarf_Unsigned *name_idx,
    int *error)
{
  pSCNINFO newscn;
  BOOL is_reloc;
  INT scnalign;

  if (num_dwarf_scns >= MAX_DWARF_SECTIONS) {
    fprintf (stderr, "ERROR: setup_new_section_for_dwarf: too many dwarf sections\n");
    exit(-1);
  }

  is_reloc = (type == SHT_REL) || (type == SHT_RELA);
  scnalign = (is_reloc) ? (is_64bit ? ELF64_FSZ_XWORD : ELF32_FSZ_WORD) : 1;
  newscn = Em_New_Section (name, type, flags, 0, scnalign);

  /* For relocation sections, we need to setup the link field to the 
     symbol table section index. The dwarf producer does not know about
     this.
  */
  if (is_reloc) {
    Em_Set_sh_link (newscn, Em_Get_Section_Index (Symtab_Info));
  }
  else {
    Em_Set_sh_link (newscn, link);
  }
  Em_Set_sh_info (newscn, info);
  dwarf_scn[num_dwarf_scns] = newscn;
  num_dwarf_scns++;
  *name_idx = (Dwarf_Unsigned) Record_Symidx(Em_Create_Section_Symbol(newscn));
  return Em_Get_Section_Index (newscn);
}

pSCNINFO
Em_Dwarf_Find_Dwarf_Scn (size_t scndx)
{
  size_t i;

  for (i = 0; i < num_dwarf_scns; i++) {
    if (Em_Get_Section_Index (dwarf_scn[i]) == scndx) return dwarf_scn[i];
  }
  return NULL;
}

pSCNINFO
Em_Dwarf_Find_Dwarf_Scn_By_Name (char *name)
{
  size_t i;
  char *scnname;

  for (i = 0; i < num_dwarf_scns; i++) {
    scnname = Em_Get_Section_Name(dwarf_scn[i]);
    if (0 == strcmp(scnname, name)) return dwarf_scn[i];
  }
  return NULL;
}



static void
Increase_Incl_Table_Size (UINT16 incl_idx)
{
  UINT16 new_idx;

  /* realloc incl_table if needed */
  if (incl_idx >= incl_table_size) {
    new_idx = incl_idx + 5;
    if (incl_table == NULL) {
      incl_table = (INCL *) malloc (new_idx * sizeof (INCL));
    }
    else {
      incl_table = (INCL *) realloc (incl_table, new_idx * sizeof(INCL));
    }
    if (incl_table == NULL) {
      ErrMsg ( EC_No_Mem, "Increase_Incl_Table_Size" );
    }
    BZERO (&incl_table[incl_table_size], new_idx - incl_table_size);
    incl_table_size = new_idx;
  }
}

void
Em_Dwarf_Add_Include (UINT16 incl_idx, char *name)
{
  Dwarf_Unsigned dwarf_idx;

  Increase_Incl_Table_Size (incl_idx);
  dwarf_idx = dwarf_add_directory_decl (dw_dbg, name, &dw_error);
  incl_table[incl_idx].dwarf_idx = dwarf_idx;
  incl_table[incl_idx].path = name;
}


static void
Increase_File_Table_Size (UINT16 file_idx)
{
  UINT16 new_idx;

  /* realloc file_table if needed */
  if (file_idx >= file_table_size) {
    new_idx = file_idx + 5;
    if (file_table == NULL) {
      file_table = (FNAME *) malloc (new_idx * sizeof (FNAME));
    }
    else {
      file_table = (FNAME *) realloc (file_table, new_idx * sizeof(FNAME));
    }
    if (file_table == NULL) {
      ErrMsg ( EC_No_Mem, "Increase_File_Table_Size" );
    }
    BZERO (&file_table[file_table_size], new_idx - file_table_size);
    file_table_size = new_idx;
  }
}


void
Em_Dwarf_Add_File (
  UINT16 file_idx, 
  char *name, 
  UINT16 incl_idx,
  Dwarf_Unsigned modification_time,
  Dwarf_Unsigned file_size)
{
  Dwarf_Unsigned dwarf_idx;

  Increase_File_Table_Size (file_idx);
  dwarf_idx = dwarf_add_file_decl (
			dw_dbg, 
			name, 
			incl_table[incl_idx].dwarf_idx, 
			modification_time,
			file_size,
			&dw_error);
  file_table[file_idx].dwarf_idx = dwarf_idx;
  file_table[file_idx].filename = name;
  file_table[file_idx].incl_idx = incl_idx;
}

#ifdef KEY
#include "strtab.h"
#include "symtab.h"
#include "irbdata.h"
#endif // KEY

#ifdef TARG_ST
// (cbr)
extern BOOL CG_emit_asm_dwarf;
#endif

#ifdef TARG_ST
// [CL] provide external interface to file_table
Dwarf_Unsigned File_Dwarf_Idx(UINT16 file_idx)
{
  return file_table[file_idx].dwarf_idx;
}
#endif

Dwarf_P_Debug 
Em_Dwarf_Begin (BOOL is_64bit, BOOL dwarf_trace, BOOL is_cplus,
		symbol_index_recorder record_symidx)
{
  Dwarf_Unsigned flags;
  char *augmenter="";
#ifdef KEY
  Dwarf_Unsigned personality=0;
#endif // KEY

#define EXT_OP(v)  (DW_CFA_extended | v)

  static unsigned char init_bytes[] = TARG_INIT_BYTES;

  if (record_symidx == NULL) {
    record_symidx = another_identity_function;
  }

  Record_Symidx = record_symidx;

  /* Initialize the incl_table and file_table to contain entries for index 0 */
  Increase_Incl_Table_Size (0);
  Increase_File_Table_Size (0);

  data_alignment_factor = is_64bit ? -8 : -4;
  flags = DW_DLC_WRITE | ((is_64bit) ? DW_DLC_SIZE_64 : DW_DLC_SIZE_32);
  flags |= DWARF_TARGET_FLAG;
  flags |= DW_DLC_SYMBOLIC_RELOCATIONS;
  if (Target_Byte_Sex == BIG_ENDIAN) {
    flags |= DW_DLC_TARGET_BIGENDIAN;
  } else {
    flags |= DW_DLC_TARGET_LITTLEENDIAN;
  };

  dw_dbg = dwarf_producer_init_b (flags, setup_new_section_for_dwarf, 
#ifdef TARG_ST
				  ISA_PACK_INST_WORD_SIZE/8,
#endif
				  0, 0, &dw_error);

// [CQ1]: Initialization of cie is no more statically known because of
// reconfigurability and interruption function.
#ifdef TARG_ST
  MEM_POOL_Initialize(&mempool, "CIE initialization", TRUE);
  MEM_POOL_Push (&mempool);

  Init_CIEs(dw_dbg, &mempool);
  if (CXX_Exceptions_On) {
      eh_cie_index = eh_CIE_index(Get_Current_PU());
  }
#else

  cie_index = dwarf_add_frame_cie (dw_dbg, augmenter,
		    4, data_alignment_factor,
		    DW_FRAME_RA_COL, 
		    init_bytes,
            sizeof(init_bytes)
                                   , &dw_error);

    if (is_cplus)
      {
        augmenter = DW_CIE_AUGMENTER_STRING_V0;
        personality = Save_Str ("__gxx_personality_v0");
      }
    else {
      augmenter = "";
      personality = 0;
    }

    eh_cie_index = dwarf_add_ehframe_cie (dw_dbg, augmenter,
		    1, data_alignment_factor,
                    DW_FRAME_RA_COL, 
		    personality,
                    init_bytes, sizeof(init_bytes),
		    &dw_error);
  }
#endif // end of else of ifdef TARG_ST

  return dw_dbg;
}

extern "C" Dwarf_P_Debug 
Em_Dwarf_Begin (BOOL is_64bit, BOOL dwarf_trace, BOOL is_cplus)
{
  return Em_Dwarf_Begin(is_64bit, dwarf_trace, is_cplus, NULL);
}

static Dwarf_Signed section_count = DW_DLV_NOCOUNT;
static Dwarf_Unsigned relocation_section_count = DW_DLV_NOCOUNT;

struct index_to_buffer_map_entry {
  pSCNINFO       cursection;
  Dwarf_Signed   scndx;
  Dwarf_Ptr      buffer;
  Dwarf_Unsigned bufsize;
};

index_to_buffer_map_entry *index_to_buffer_map;

static inline int
find_indexed_section(      int          i,
		     const Dwarf_Signed scn)
{
  for (; i < section_count; ++i) {
    if (index_to_buffer_map[i].scndx == scn) {
      return i;
    }
  }
  return -1;
}

static int current_scn;

static int
advance_to_next_stream_buffer(const Dwarf_Signed scndx)
{
  if (current_scn >= section_count) {
    fprintf(stderr, "Too many calls to advance_to_next_stream_buffer");
    exit(-1);
  }
  current_scn =
    find_indexed_section(current_scn + 1, scndx);
  return current_scn != -1;
}

static Dwarf_Ptr
get_current_buffer(void)
{
  return index_to_buffer_map[current_scn].buffer;
}

static Dwarf_Unsigned
get_current_bufsize(void)
{
  return index_to_buffer_map[current_scn].bufsize;
}

void
Em_Dwarf_Write_Scns (Cg_Dwarf_Sym_To_Elfsym_Ofst translate_dwarf_sym)
{
  Dwarf_Signed i;
  Dwarf_Ptr buffer;
  Dwarf_Signed scndx;
  Dwarf_Unsigned bufsize;
  int result;
  int reloc_buffer_version;
  pSCNINFO cursection;

  if (translate_dwarf_sym == NULL) {
    translate_dwarf_sym = identity_function;
  }

  index_to_buffer_map =
    (index_to_buffer_map_entry *) malloc(section_count *
					 sizeof(index_to_buffer_map_entry));

  dwarf_reset_section_bytes(dw_dbg);

  for (i = 0; i < section_count; i++) {
    buffer = dwarf_get_section_bytes (dw_dbg, i, &scndx, &bufsize, &dw_error);
    cursection = Em_Dwarf_Find_Dwarf_Scn (scndx);
    if (cursection != NULL) {
      index_to_buffer_map[i].cursection = cursection;
      index_to_buffer_map[i].scndx = scndx;
      index_to_buffer_map[i].buffer = buffer;
      index_to_buffer_map[i].bufsize = bufsize;
    }
    else {
      fprintf (stderr, "ERROR No such section index: %d\n", (int)scndx);
      exit(-1);
    }
  }

  // Now get the relocation buffers, translate each one into an ELF
  // relocation section, and write the ELF section to the output file.
  result = dwarf_get_relocation_info_count(dw_dbg,
					   &relocation_section_count,
					   &reloc_buffer_version,
					   &dw_error);
  if (result != DW_DLV_OK) {
    fprintf(stderr, "ERROR Couldn't get relocation buffer count\n");
    exit(-1);
  }
  if (reloc_buffer_version != 2 /* DWARF_DRD_BUFFER_VERSION */) {
    fprintf(stderr,
	    "ERROR Relocation buffer format unrecognized "
	    "(version %d)\n", reloc_buffer_version);
    exit(-1);
  }

  for (i = 0; i < relocation_section_count; i++) {
    Dwarf_Relocation_Data reloc_buf;
    Dwarf_Unsigned entry_count;
    Dwarf_Signed link_scn;

    result = dwarf_get_relocation_info(dw_dbg, &scndx, &link_scn,
				       &entry_count, &reloc_buf,
				       &dw_error);
    if (result != DW_DLV_OK) {
      fprintf(stderr, "ERROR dwarf_get_relocation_info failed\n");
      exit(-1);
    }

    current_scn = -1;

    Dwarf_Signed j = find_indexed_section(current_scn + 1, link_scn);

    BOOL is_debug_line =
      (strcmp(".debug_line",
	      Em_Get_Section_Name(index_to_buffer_map[j].cursection)) == 0);

    buffer =
      Em_Dwarf_Symbolic_Relocs_To_Elf(get_current_buffer,
				      get_current_bufsize,
				      advance_to_next_stream_buffer,
				      link_scn,
				      reloc_buf, entry_count,
				      // Cheap hack to see if
				      // this is a 64-bit compile:
				      data_alignment_factor == -8,
				      (is_debug_line ?
				       identity_function :
				       translate_dwarf_sym),
				      &bufsize);
    cursection = Em_Dwarf_Find_Dwarf_Scn(scndx);
    if (cursection != NULL) {
      Em_Add_Bytes_To_Scn(cursection, buffer, bufsize, 1);
    }
    else {
      fprintf (stderr, "ERROR No such section index: %d\n", (int)scndx);
      exit(-1);
    }
    free((void *) buffer);
  }

  for (i = 0; i < section_count; ++i) {
    Em_Add_Bytes_To_Scn (index_to_buffer_map[i].cursection,
			 index_to_buffer_map[i].buffer,
			 index_to_buffer_map[i].bufsize,
			 1);
  }

  free(index_to_buffer_map);

  for (i = 0; i < num_dwarf_scns; i++) {
    Em_End_Section (dwarf_scn[i]);
  }
}

extern "C" void
Em_Dwarf_Write_Scns (void)
{
  Em_Dwarf_Write_Scns(NULL);
}

INT
Em_Dwarf_Prepare_Output(void)
{
  section_count = dwarf_transform_to_disk_form (dw_dbg, &dw_error);
  if (section_count == DW_DLV_NOCOUNT) {
    fprintf(stderr, "ERROR in dwarf_transform_to_disk_form\n");
    exit(-1);
  }
  return (INT) section_count;
}

void 
Em_Dwarf_End (void)
{
  dwarf_producer_finish (dw_dbg, &dw_error);
#ifdef TARG_ST
    Clear_CIEs(&mempool);
    MEM_POOL_Pop(&mempool);
    MEM_POOL_Delete(&mempool);
#endif
}



#ifndef MONGOOSE_BE
/* Given a file_index, return the filename and the path for the file. */
void
Em_Dwarf_File_Index_To_Name (INT file_index, char **filename, char **path)
{
  *filename = file_table[file_index].filename;
  *path = incl_table[file_table[file_index].incl_idx].path;
}
#endif /* MONGOOSE_BE */


/* the starting offset in the text region */
static INT Text_Start_Offset;
static Dwarf_Unsigned Text_Start_Label;
static Dwarf_Addr     Offset_From_Text_Start_Label;

void
Em_Dwarf_Add_Line_Entry ( INT code_address, SRCPOS srcpos)
{
  USRCPOS usrcpos;

  USRCPOS_srcpos(usrcpos) = srcpos;

  dwarf_add_line_entry (dw_dbg, 
			file_table[USRCPOS_filenum(usrcpos)].dwarf_idx, 
			code_address - Text_Start_Offset, 
			USRCPOS_linenum(usrcpos), 
			USRCPOS_column(usrcpos), 
			USRCPOS_stmt_begin(usrcpos), 
			USRCPOS_bb_begin(usrcpos), 
			&dw_error);
}

void
Em_Dwarf_Start_Text_Lines (pSCNINFO scninfo, INT start_offset)
{
  Text_Start_Offset = start_offset;
  dwarf_lne_set_address (dw_dbg, start_offset, 
		      Text_Start_Label, &dw_error);
}

void
Em_Dwarf_Start_Text_Region (pSCNINFO scninfo, INT start_offset)
{
  Em_Dwarf_Start_Text_Lines (scninfo, start_offset);
}

void
Em_Dwarf_Start_Text_Region_Semi_Symbolic (pSCNINFO       scninfo,
					  INT            start_offset,
					  Dwarf_Unsigned first_label,
					  Dwarf_Addr     offset_from_first_label)
{
  Text_Start_Offset = start_offset;
  Text_Start_Label = first_label;
  Offset_From_Text_Start_Label = offset_from_first_label;
  dwarf_lne_set_address (dw_dbg, offset_from_first_label, Text_Start_Label,
			 &dw_error);
}

void
Em_Dwarf_End_Text_Lines (pSCNINFO scninfo, INT end_offset)
{
  dwarf_lne_end_sequence (dw_dbg, end_offset, &dw_error);
}

void
Em_Dwarf_End_Text_Arange (pSCNINFO scninfo, INT end_offset)
{
  INT region_size = end_offset - Text_Start_Offset;

  dwarf_add_arange (dw_dbg, Text_Start_Offset, region_size, 
		    Em_Create_Section_Symbol(scninfo), &dw_error);
}

void
Em_Dwarf_End_Text_Arange_Symbolic(Dwarf_Unsigned last_label,
#ifdef TARG_ST
				  Dwarf_Unsigned end_label,
#endif
				  Dwarf_Addr     offset_from_last_label)
{
#ifdef TARG_ST
  if (end_label != 0) {
      dwarf_add_arange_b (dw_dbg,
			  Offset_From_Text_Start_Label,
			  0 /* dummy length */, 
			  Text_Start_Label,
			  end_label,
			  0,
			  &dw_error);
  }
  else {
#endif
  dwarf_add_arange_b (dw_dbg,
		      Offset_From_Text_Start_Label,
		      0 /* dummy length */, 
		      Text_Start_Label,
		      last_label,
		      offset_from_last_label,
		      &dw_error);
#ifdef TARG_ST
  }
#endif
}

void
Em_Dwarf_End_Text_Region (pSCNINFO scninfo, INT end_offset)
{
  Em_Dwarf_End_Text_Arange (scninfo, end_offset);
  Em_Dwarf_End_Text_Lines (scninfo, end_offset);
}

void
Em_Dwarf_End_Text_Region_Semi_Symbolic(pSCNINFO       scninfo,
				       INT            end_offset,
				       Dwarf_Unsigned last_label,
#ifdef TARG_ST
				       Dwarf_Unsigned end_label,
#endif
				       Dwarf_Addr     offset_from_last_label)
{
#ifdef TARG_ST
  Em_Dwarf_End_Text_Arange_Symbolic(last_label,
				    end_label,
				    offset_from_last_label);
  dwarf_lne_end_sequence_symbolic (dw_dbg, end_offset - Text_Start_Offset,
				   last_label, end_label, &dw_error);
#else
  Em_Dwarf_End_Text_Arange_Symbolic(last_label,
				    offset_from_last_label);
  Em_Dwarf_End_Text_Lines(scninfo, end_offset - Text_Start_Offset);
#endif
}

void Em_Dwarf_Process_PU (Dwarf_Unsigned begin_label,
			  Dwarf_Unsigned end_label,
			  INT32          begin_offset,
			  INT32          end_offset,
			  Dwarf_P_Die    PU_die, 
			  Dwarf_P_Fde    fde,
#ifdef TARG_ST
			  Dwarf_P_Fde    eh_fde,
#endif
			  Elf64_Word     eh_symindex,
			  INT            eh_offset)
{
  /* setup the low_pc and the high_pc attributes. */
  dwarf_add_AT_targ_address_b (dw_dbg, PU_die, DW_AT_low_pc,
			       begin_offset,
			       (Dwarf_Unsigned) begin_label,
			       &dw_error);
  dwarf_add_AT_targ_address_b (dw_dbg, PU_die, DW_AT_high_pc,
			       end_offset, (Dwarf_Unsigned) end_label,
			       &dw_error);

#ifndef TARG_ST
  if (fde == NULL)
	return;
#endif

#ifdef TARG_ST
  // (cbr) we enter here either for debug dwarf emission or exceptions frame dwarf unwinding 
  if (CG_emit_asm_dwarf && fde)
#endif  
  /* emit the debug_frame information for this procedure. */
  if (eh_offset == DW_DLX_NO_EH_OFFSET)	/* no exception handler */
  	dwarf_add_frame_fde_b (dw_dbg, fde, PU_die,
#   ifdef TARG_ST
                           CIE_index(Get_Current_PU()),
#   else
                           cie_index, 
#   endif
			       begin_offset,
			       0 /* dummy code length */,
			       (Dwarf_Unsigned) begin_label,
			       (Dwarf_Unsigned) end_label,
			       end_offset,
			       &dw_error);
  else
  	dwarf_add_frame_info_b (dw_dbg, fde, PU_die,
#   ifdef TARG_ST
                            CIE_index(Get_Current_PU()),
#   else
                            cie_index, 
#   endif
				begin_offset,
				0 /* dummy code length */,
				(Dwarf_Unsigned) begin_label,
				(Dwarf_Unsigned) end_label,
				end_offset,
				eh_offset, eh_symindex, &dw_error);

#ifdef TARG_ST
  /* (cbr) emit .eh_frame */
  extern BOOL PU_Has_Calls;
  if (eh_fde && CXX_Exceptions_On &&
      PU_Has_Calls) {
    if (eh_offset == DW_DLX_NO_EH_OFFSET)	/* no exception handler */
      dwarf_add_ehframe_fde_b (dw_dbg, eh_fde, PU_die,
                               eh_CIE_index(Get_Current_PU()),
			       begin_offset,
			       0 /* dummy code length */,
			       (Dwarf_Unsigned) begin_label,
			       (Dwarf_Unsigned) end_label,
			       end_offset,
			       &dw_error);
    else
      dwarf_add_ehframe_info_b (dw_dbg, eh_fde, PU_die,
                               eh_CIE_index(Get_Current_PU()),
				begin_offset,
				0 /* dummy code length */,
				(Dwarf_Unsigned) begin_label,
				(Dwarf_Unsigned) end_label,
				end_offset,
				eh_offset, eh_symindex, &dw_error);
  }
#endif // end of ifdef TARG_ST
}

#ifdef TARG_ST

#   include <assert.h>	// temporary
#   define USE_STANDARD_TYPES 1
#   include "dwarf_DST.h" // To access compilation unit's dwarf information
#   include "cxx_memory.h" // For call to CXX_NEW/DELETE_ARRAY
#   include "register.h" // For REGISTER_MIN and CGTARG_DW_DEBUG_Get_Extension_Id
#   include "symtab.h" // For Save_Str call

FILE* ciedbgOutput = stdout;

BOOL Trace_CIE = FALSE;

CIEIndexToInfo CIEs;

const DebugRegId DebugRegId_undef = (DebugRegId)-1;

typedef map<INT, Dwarf_Unsigned> HashValueToCIEIndex;

/**
 * Number of bit store in one byte of the dwarf LEB128 representation
 */
static const INT DW_BIT_BY_LEB = 7;

/**
 * Mask used for dwarf 2 translation to set padding bit for LEB128
 */
static const INT MORE_BYTE = (1 << DW_BIT_BY_LEB);

/**
 * Mask used for dwarf 2 translation in LEB128
 */
static const INT DATA_MASK = MORE_BYTE - 1;

/**
 * Pointer to global libdwarf debug information.
 * This information is needed to be able to create libdwarf objects like CIE
 */
static Dwarf_P_Debug g_current_dw_dbg = NULL;


//------------------------------------------------------------------------------
// CIE initialization support function declared in this file
//------------------------------------------------------------------------------

// Use for template readability
typedef Dwarf_Unsigned (*CIECreation)(PU&, const Dwarf_Small&, unsigned char*,
                                      Dwarf_Unsigned);

template<CIECreation fct>
static Dwarf_Unsigned
GenericInitCIE(PU& pu);

template<CIECreation fct>
static Dwarf_Unsigned
GenericCIEIndex(PU& pu);

Dwarf_Unsigned
CreateEhCIE(PU& pu, const Dwarf_Small& retAddr, unsigned char* initialBytes,
            Dwarf_Unsigned size_of_init_bytes);

Dwarf_Unsigned
CreateCIE(PU& pu, const Dwarf_Small& retAddr, unsigned char* initialBytes,
          Dwarf_Unsigned size_of_init_bytes);

//------------------------------------------------------------------------------
// CCIEInfo class definition
//------------------------------------------------------------------------------
MEM_POOL* CCIEInfo::m_memPool = NULL;
CCIEInfo::ListOfBytes CCIEInfo::m_listOfInitialBytes;

CCIEInfo::CCIEInfo(PU& a_pu)
    : m_initBytes(ByteAllocator(CCIEInfo::MemPool())), m_returnAddressRegId(0),
      m_saved(CCIEInfo::LessThanDebugRegId(),
              DebugRegIdAllocator(CCIEInfo::MemPool())), m_cfaOffset(0)
{
    BuildCIEBytesList(a_pu);
}

CCIEInfo::CCIEInfo(const CCIEInfo& a_cieInfo)
    : m_initBytes(ByteAllocator(CCIEInfo::MemPool())),
      m_returnAddressRegId(0),
      m_saved(CCIEInfo::LessThanDebugRegId(),
              DebugRegIdAllocator(CCIEInfo::MemPool())), m_cfaOffset(0)
{
    CopyMembers(a_cieInfo);
}

CCIEInfo&
CCIEInfo::operator=(const CCIEInfo& a_cieInfo)
{
    if(this != &a_cieInfo)
        {
            CopyMembers(a_cieInfo);
        }
    return *this;
}

CCIEInfo::~CCIEInfo()
{
}

unsigned char*
CCIEInfo::InitialBytes(Dwarf_Unsigned& sizeOfReturnedTab) const
{
    sizeOfReturnedTab = InitBytes().size();
    return static_cast<unsigned char*>(TranslateBytesList(InitBytes(),
                                                          MemPool()));
}

const Bytes&
CCIEInfo::InitBytes() const
{
    return m_initBytes;
}

BOOL
CCIEInfo::IsSaved(const DebugRegId& a_debugRegId) const
{
    return BOOL(Saved().find(a_debugRegId) != Saved().end());
}

void
CCIEInfo::MemPool(MEM_POOL* a_mempool)
{
    m_memPool = a_mempool;
}

MEM_POOL*
CCIEInfo::MemPool()
{
    return m_memPool;
}

const DebugRegId&
CCIEInfo::ReturnAddressRegId() const
{
    return m_returnAddressRegId;
}

const CCIEInfo::SavedList&
CCIEInfo::Saved() const
{
    return m_saved;
}

UINT
CCIEInfo::CfaOffset() const
{
    return m_cfaOffset;
}

void
CCIEInfo::CopyMembers(const CCIEInfo& a_cieInfo)
{
    m_initBytes = a_cieInfo.InitBytes();
    m_returnAddressRegId = a_cieInfo.ReturnAddressRegId();
    m_saved = a_cieInfo.Saved();
    m_cfaOffset = a_cieInfo.CfaOffset();
}

void
CCIEInfo::AddToSaved(const DebugRegId& a_debugRegId)
{
    m_saved.insert(a_debugRegId);
}

void
CCIEInfo::ReleaseListOfInitialBytes()
{
    ItListOfBytes it;
    for(it = m_listOfInitialBytes.begin(); it != m_listOfInitialBytes.end();
        ++it)
        {
            CXX_DELETE_ARRAY(*it, CCIEInfo::MemPool());
        }
}

void
CCIEInfo::BuildCIEBytesList(PU& pu)
{
    ISA_REGISTER_CLASS register_class;

    DbgPrintCIE((ciedbgOutput, "CIE initialization\n"));

    m_initBytes.push_back(DW_CFA_def_cfa);
    DebugRegId baseId;
    CfaDef(baseId, m_cfaOffset, pu);

    EmitBytes(baseId);
    EmitBytes(DebugRegId(CfaOffset()));

    DebugRegId raId;
    UINT offset;
    if(ReturnAddressDef(raId, offset, pu))
        {
            DbgPrintCIE((ciedbgOutput, "CIE has special return address\n"));
            m_initBytes.push_back(DW_CFA_offset_extended);
            EmitBytes(raId);
            EmitBytes(offset);
        }
    m_returnAddressRegId = raId;

    FOR_ALL_ISA_REGISTER_CLASS(register_class)
    {
        if(ShouldGenerateInformation(register_class, pu))
            {
                const ISA_REGISTER_CLASS_INFO* info =
                    ISA_REGISTER_CLASS_Info(register_class);
                int i = ISA_REGISTER_CLASS_INFO_First_Reg(info) + REGISTER_MIN;
                int bitSize = ISA_REGISTER_CLASS_INFO_Bit_Size(info);
                for(; i <= ISA_REGISTER_CLASS_INFO_Last_Reg(info) +
                        REGISTER_MIN; ++i)
                    {
                        DebugRegId regId = Get_Debug_Reg_Id(register_class, i,
                                                            bitSize);
                        EmitReg(regId, register_class, i, pu);
                    }
            }
    }
}

void
CCIEInfo::EmitReg(const DebugRegId& regId, ISA_REGISTER_CLASS register_class,
                  REGISTER i, PU& pu)
{
    UINT offset;
    if(HasSpecialDef(regId, offset, register_class, i, pu))
        {
            DbgPrintCIE((ciedbgOutput, "Special Def: %u\n", offset));
// There is a bad interaction with st200gdb when emitting the bytes for CIE
// definition. It seems to be misled when the list of saved register is
// emitted. Problem with the unwind mechanism?
#ifdef TARG_ST200
            if(getenv("O64_FULLCIE") != NULL)
              {
#endif
            m_initBytes.push_back(DW_CFA_offset_extended);
            EmitBytes(regId);
            EmitBytes(offset);
#ifdef TARG_ST200
              } // end if O64_FULLCIE
#endif
        }
    else
        {
            if(::IsSaved(regId, register_class, i, pu))
                {
                    DbgPrintCIE((ciedbgOutput, "Same value\n"));
// Same remarks
#ifdef TARG_ST200
                    if(getenv("O64_FULLCIE") != NULL)
                      {
#endif
                    m_initBytes.push_back(DW_CFA_same_value);
#ifdef TARG_ST200
                      } // end if O64_FULLCIE
#endif
                    AddToSaved(regId);
                }
            else
                {
                    DbgPrintCIE((ciedbgOutput, "Undefined\n"));
#ifdef TARG_ST200
                    if(getenv("O64_FULLCIE") != NULL)
                      {
#endif
                    m_initBytes.push_back(DW_CFA_undefined);
#ifdef TARG_ST200
                      } // end if O64_FULLCIE
#endif
                }
// Same remarks
#ifdef TARG_ST200
            if(getenv("O64_FULLCIE") != NULL)
              {
#endif
            // dwarf representation is ULEB128: Little Endian Base
            // 128. See dwarf 2 norm to have more details
            EmitBytes(regId);
#ifdef TARG_ST200
              } // end if O64_FULLCIE
#endif
        }
}

void
CCIEInfo::EmitBytes(const DebugRegId& a_regId)
{
    DebugRegId regId(a_regId);
    do
        {
            // Take the seven low bits
            Byte val = regId & DATA_MASK;
            regId >>= DW_BIT_BY_LEB;
            if(regId)
                {
                    // Set the high bit
                    val |= MORE_BYTE;
                }
            m_initBytes.push_back(val);
        }
    while(regId);
}

Byte*
CCIEInfo::TranslateBytesList(const Bytes& init_bytes, MEM_POOL* mempool)
{
    int i;
    CItBytes it;
    Byte* result = CXX_NEW_ARRAY(Byte, init_bytes.size(), mempool);
    for(i = 0, it = init_bytes.begin(); it != init_bytes.end(); ++i, ++it)
        {
            result[i] = *it;
        }
    // We do not keep allocated pointer in the object, because these objects
    // have a smaller liveness than the one requiered by libdwarf for initial
    // cie bytes
    m_listOfInitialBytes.push_back(result);
    return result;
}

//------------------------------------------------------------------------------
// CIE initialization function definitions
//------------------------------------------------------------------------------
Dwarf_Unsigned
CIE_index(PU& pu)
{
    return GenericCIEIndex<CreateCIE>(pu);
}

Dwarf_Unsigned
eh_CIE_index(PU& pu)
{
    return GenericCIEIndex<CreateEhCIE>(pu);
}

void
Init_CIEs(Dwarf_P_Debug dw_dbg, MEM_POOL* a_memPool)
{
    g_current_dw_dbg = dw_dbg;
    CCIEInfo::MemPool(a_memPool);
}

void
Clear_CIEs(MEM_POOL* a_memPool)
{
    CCIEInfo::ReleaseListOfInitialBytes();
    CIEs.clear();
}

/**
 * Generic CIE index initialization.
 * This initialization consists in:
 * @li Creating a CCIEInfo for given program unit and return address
 * @li Creating the libdwarf CIE object with template parameter using created
 *     CCIEInfo object information
 * @li Binding created CCIEInfo to the index of the created libdwarf CIE object
 *
 * @param  pu [in] Program unit for which we create a CIE
 *
 * @pre    Init_CIEs has been called
 * @post   CIEs[result] is set to created CCIEInfo object
 *
 * @return The index of the created libdwarf cie object
 */
template<CIECreation fct>
static Dwarf_Unsigned
GenericInitCIE(PU& pu)
{
    CCIEInfo cieInfo(pu);
    Dwarf_Unsigned size_of_init_bytes;
    unsigned char* initialBytes = cieInfo.InitialBytes(size_of_init_bytes);
    Dwarf_Unsigned cieIndex = fct(pu, (Dwarf_Small)cieInfo.ReturnAddressRegId(),
                                  initialBytes, size_of_init_bytes);
    FmtAssert(cieIndex != DW_DLV_NOCOUNT, ("Unable to create CIE information"));
    CIEs[cieIndex] = cieInfo;
    return cieIndex;
}

/**
 * Create a libdwarf CIE with given parameter.
 *
 * @param  pu [in] Program unit for which we create the cie
 * @param  retAddr Debug register identifier of return address
 * @param  initialBytes Pointer to the array of the initial CIE bytes
 * @param  size_of_init_bytes Size of initialBytes parameter
 *
 * @pre    Init_CIEs has been called and size_of_init_bytes =
 *         initialBytes->length()
 * @post   result <> DW_DLV_NOCOUNT implies a libdwarf CIE has been created
 *
 * @return An index to the CIE just created on success. On error it returns
 *         DW_DLV_NOCOUNT.
 *
 * @remarks This function match CIECreation type definition
 */
Dwarf_Unsigned
CreateCIE(PU& pu, const Dwarf_Small& retAddr, unsigned char* initialBytes,
          Dwarf_Unsigned size_of_init_bytes)
{
    char *augmenter="";
    Dwarf_Error dw_error;
    return dwarf_add_frame_cie(g_current_dw_dbg, augmenter,
                               Dwarf_Small(CodeAlignmentFactor(pu)),
                               Dwarf_Small(DataAlignmentFactor(pu)),
                               retAddr, initialBytes, size_of_init_bytes,
                               &dw_error);
}

/**
 * Create a libdwarf CIE with given parameter for EH frame.
 *
 * @param  pu [in] Program unit for which we create the cie
 * @param  retAddr Debug register identifier of return address
 * @param  initialBytes Pointer to the array of the initial CIE bytes
 * @param  size_of_init_bytes Size of initialBytes parameter
 *
 * @pre    Init_CIEs has been called and size_of_init_bytes =
 *         initialBytes->length()
 * @post   result <> DW_DLV_NOCOUNT implies a libdwarf CIE has been created
 *
 * @return An index to the CIE just created on success. On error it returns
 *         DW_DLV_NOCOUNT.
 *
 * @remarks This function match CIECreation type definition
 */
Dwarf_Unsigned
CreateEhCIE(PU& pu, const Dwarf_Small& retAddr, unsigned char* initialBytes,
            Dwarf_Unsigned size_of_init_bytes)
{
    char *augmenter = DW_CIE_AUGMENTER_STRING_V0;
    Dwarf_Unsigned personality;
    DST_INFO* cuInfo = DST_INFO_IDX_TO_PTR(DST_get_compile_unit());
    DST_COMPILE_UNIT *cu = DST_ATTR_IDX_TO_PTR(DST_INFO_attributes(cuInfo),
                                               DST_COMPILE_UNIT);
    if(DST_COMPILE_UNIT_language(cu) == DW_LANG_C_plus_plus)
        {
            personality = Save_Str ("__gxx_personality_v0");
        }
    else
        {
            personality = Save_Str ("__gcc_personality_v0");
        }
    Dwarf_Error dw_error;
    return dwarf_add_ehframe_cie(g_current_dw_dbg, augmenter,
                                 Dwarf_Small(CodeAlignmentFactor(pu)),
                                 Dwarf_Small(DataAlignmentFactor(pu)), retAddr,
                                 personality, initialBytes, size_of_init_bytes,
                                 &dw_error);
}

/**
 * Generic CIE index retrieving.
 * Creates or returns a cie index according to given program unit properties.
 *
 * @param  pu [in] Program unit for which we want a CIE index
 *
 * @pre    Init_CIEs has been called
 * @post   result <> DW_DLV_NOCOUNT implies result is an index of a libdwarf CIE
 *         object which represents pu
 *
 * @return An index to the CIE just created on success. On error it returns
 *         DW_DLV_NOCOUNT.
 */
template<CIECreation fct>
static Dwarf_Unsigned
GenericCIEIndex(PU& pu)
{
    /**
     * Contains CIE index for PU hash value.
     */
    static HashValueToCIEIndex indexes;

    Dwarf_Unsigned cieIndex;
    INT hashValue = HashValue(pu);
    if(indexes.find(hashValue) == indexes.end())
        {
            indexes[hashValue] = GenericInitCIE<fct>(pu);
        }
    cieIndex = indexes[hashValue];
    return cieIndex;
}

DebugRegId
Get_Debug_Reg_Id(const CLASS_REG_PAIR& regPair)
{
    const ISA_REGISTER_CLASS_INFO* info =
        ISA_REGISTER_CLASS_Info(CLASS_REG_PAIR_rclass(regPair));
    int bitSize = ISA_REGISTER_CLASS_INFO_Bit_Size(info);

    return Get_Debug_Reg_Id(CLASS_REG_PAIR_rclass(regPair),
                            CLASS_REG_PAIR_reg(regPair), bitSize);
}

DebugRegId
Get_Debug_Reg_Id(TN* tn)
{
    DevAssert(TN_is_register(tn), ("%s: Must be called only with register",
                                   __FUNCTION__));
    return Get_Debug_Reg_Id(CLASS_REG_PAIR_rclass(TN_class_reg(tn)),
                            CLASS_REG_PAIR_reg(TN_class_reg(tn)),
                            TN_size(tn) * CHAR_BIT);
}

#endif
