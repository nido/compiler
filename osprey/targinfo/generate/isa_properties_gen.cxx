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


//  isa_properties_gen.cxx
/////////////////////////////////////
//
//  Generate an interface for specifying properties (attributes) for 
//  various instructions in the ISA.
//
/////////////////////////////////////
//
//  $Revision$
//  $Date$
//  $Author$
//  $Source$


#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
#include <list.h>
#include <vector.h>
#include "topcode.h"
#include "gen_util.h"
#include "isa_properties_gen.h"


struct isa_property {
  const char* name;         // Name given for documentation and debugging
  int bit_position;         // bit postion in flag word
  vector <bool> members;    // set of opcodes that have this property
};

// special values for bit_position above:
enum {
  BIT_POS_ALL = -1,         // all members have this property
  BIT_POS_NONE = -2         // no members have this property
};

static list<ISA_PROPERTY> properties; // All the properties

static const char * const interface[] = {
  "/* ====================================================================",
  " * ====================================================================",
  " *",
  " * Description:",
  " *",
  " *   A description of the properties (attributes) for the instructions",
  " *   in the ISA. The description exports the following:",
  " *",
  " *   BOOL TOP_is_xxx(TOP topcode)",
  " *       Return true/false if 'topcode' has/does-not-have the property",
  " *       'xxx'.",
  " *",
  " * ====================================================================",
  " * ====================================================================",
  " */",
  NULL
};

static vector <short> mem_bytes;
static vector <short> mem_align;

/////////////////////////////////////
void ISA_Properties_Begin( const char* /* name */ )
/////////////////////////////////////
//  See interface description.
/////////////////////////////////////
{
  mem_bytes = vector <short> (TOP_count, 0);
  mem_align = vector <short> (TOP_count, 0);

  return;
}

/* ====================================================================
 *   ISA_Memory_Access
 * ====================================================================
 */
void ISA_Memory_Access (int bytes, ... ) {
  va_list ap;
  TOP opcode;

  // do not allow memory accesses/alignments more than these
  assert (bytes <= 16);

  va_start(ap, bytes);
  while ( (opcode = static_cast<TOP>(va_arg(ap,int))) != TOP_UNDEFINED ) {
    //    if (!prop_load->members[(int)opcode] && 
    //	!prop_store->members[(int)opcode]) {
    //      fprintf(stderr, "### Error: memory access specified for not load/store opcode %s \n", TOP_Name(opcode));
    //      exit(EXIT_FAILURE);
  //    }
    mem_bytes[(int)opcode] = bytes;
  }
  va_end(ap);
}

/* ====================================================================
 *   ISA_Memory_Alignment
 * ====================================================================
 */
void ISA_Memory_Alignment(int bytes, ... ) {
  va_list ap;
  TOP opcode;

  // do not allow memory alignments more than these
  assert (bytes <= 16);

  va_start(ap, bytes);
  while ( (opcode = static_cast<TOP>(va_arg(ap,int))) != TOP_UNDEFINED ) {
//    if (!prop_load->members[(int)opcode] && 
//	!prop_store->members[(int)opcode]) {
//      fprintf(stderr, "### Error: alignment specified for not load/store opcode %s \n", TOP_Name(opcode));
//      exit(EXIT_FAILURE);
//    }
    mem_align[(int)opcode] = bytes;
  }
  va_end(ap);
}

/////////////////////////////////////
ISA_PROPERTY ISA_Property_Create( const char* name )
/////////////////////////////////////
//  See interface description.
/////////////////////////////////////
{
  ISA_PROPERTY result = new isa_property;

  result->name = name;
  result->members = vector <bool> (TOP_count, false);

  properties.push_back(result);

  return result;
}

/////////////////////////////////////
void Instruction_Group( ISA_PROPERTY property, ... )
/////////////////////////////////////
//  See interface description.
/////////////////////////////////////
{
  va_list ap;
  TOP opcode;

  va_start(ap,property);
  while ( (opcode = static_cast<TOP>(va_arg(ap,int))) != TOP_UNDEFINED ) {
    property->members[(int)opcode] = true;
  }
  va_end(ap);
}



static FILE* hfile;
static FILE* cfile;
static FILE* efile;

/* ====================================================================
 *   emit_top_define
 * ====================================================================
 */
void emit_top_define (
  const char *prop
)
{
  fprintf (hfile, "#ifndef %s \n", prop);
  fprintf (hfile, "#define %s(t) \t (FALSE) \n", prop);
  fprintf (hfile, "#endif \n");
  fprintf (hfile, "\n");
}

/* ====================================================================
 *   ISA_Properties_End
 * ====================================================================
 */
void ISA_Properties_End(void)
{
  list<ISA_PROPERTY>::iterator isi;
  int isa_property_count;	// How many non-constant properties?
  int code;
  int i;

#define FNAME "targ_isa_properties"
  char filename[1000];
  sprintf (filename, "%s.h", FNAME);
  hfile = fopen(filename, "w");
  sprintf (filename, "%s.c", FNAME);
  cfile = fopen(filename, "w");
  sprintf (filename, "%s.Exported", FNAME);
  efile = fopen(filename, "w");

  fprintf(cfile,"#include \"%s.h\"\n\n", FNAME);

  Emit_Header (hfile, "targ_isa_properties", interface);

  fprintf(hfile,"#include \"topcode.h\"\n\n");

  isa_property_count = 0;  
  for ( isi = properties.begin(); isi != properties.end(); ++isi ) {
    ISA_PROPERTY property = *isi;
    bool member;
    bool prev_member = property->members[0];
    for (code = 1; code < TOP_count; code++) {
      member = property->members[code];
      if (member != prev_member) break;
    }
    if (member != prev_member) {
      property->bit_position = isa_property_count++;
    } else {
      property->bit_position = member ? BIT_POS_ALL : BIT_POS_NONE;
    }
  }

  char *int_type;
  char *int_suffix;
  int int_size;
  if (isa_property_count <= 8) {
    int_type = "mUINT8";
    int_suffix = "";
    int_size = 8;
  } else if (isa_property_count <= 16) {
    int_type = "mUINT16";
    int_suffix = "";
    int_size = 16;
  } else if (isa_property_count <= 32) {
    int_type = "mUINT32";
    int_suffix = "U";
    int_size = 32;
  } else {
    assert (isa_property_count <= 64);
    int_type = "mUINT64";
    int_suffix = "ULL";
    int_size = 64;
  }
  fprintf (hfile, "extern const %s ISA_PROPERTIES_flags[];\n\n", int_type);
  fprintf (efile, "ISA_PROPERTIES_flags\n");
  fprintf (cfile,"const %s ISA_PROPERTIES_flags[] = {\n", int_type);

  for (code = 0; code < TOP_count; code++) {
    unsigned long long flag_value = 0;

    for ( isi = properties.begin(); isi != properties.end(); ++isi ) {
      ISA_PROPERTY property = *isi;
      if (property->bit_position >= 0 && property->members[code]) {
	flag_value |= (1ULL << property->bit_position);
      }
    }
    fprintf (cfile, "  0x%0*llx%s, /* %s:", int_size / 4,
					    flag_value, 
					    int_suffix,
					    TOP_Name((TOP)code));
    for ( isi = properties.begin(); isi != properties.end(); ++isi ) {
      ISA_PROPERTY property = *isi;
      if (property->members[code]) fprintf (cfile, " %s", property->name);
    }
    fprintf (cfile, " */\n");
  }
  fprintf (cfile, "};\n");

  for ( isi = properties.begin(); isi != properties.end(); ++isi ) {
    ISA_PROPERTY property = *isi;
    int bit_position = property->bit_position;
    if (bit_position >= 0) {
      fprintf (hfile, "#define PROP_%-16s 0x%llx%s\n", 
		      property->name, 
		      (1ULL << bit_position),
		      int_suffix);
    }
  }

  fprintf (hfile, "\n\n");
  for ( isi = properties.begin(); isi != properties.end(); ++isi ) {
    ISA_PROPERTY property = *isi;
    int bit_position = property->bit_position;
    if (bit_position < 0) {
      fprintf (hfile, "#define TOP_is_%s(t)\t (%s)\n",
		      property->name, 
		      bit_position == BIT_POS_ALL ? "TRUE" : "FALSE");
    } else {
      fprintf (hfile, "#define TOP_is_%s(t)\t (ISA_PROPERTIES_flags[(INT)t] & PROP_%s)\n",
		      property->name, 
		      property->name);
    }
  }

  // Emit the rest of properties that are required by the compiler
  // but have not been specified:
  //    fprintf (hfile, "/* pseudo NOP */ \n");
  //    fprintf (hfile, "#define TOP_is_noop(t) \t (t == TOP_noop) \n");
  //    fprintf (hfile, "\n");

    emit_top_define ("TOP_is_noop");
    emit_top_define ("TOP_is_predicated");
    emit_top_define ("TOP_is_likely");
    emit_top_define ("TOP_is_branch_predict");
    emit_top_define ("TOP_is_side_effects");

    emit_top_define ("TOP_is_defs_fcr");
    emit_top_define ("TOP_is_defs_fcc");
    emit_top_define ("TOP_is_refs_fcr");

    fprintf (hfile, "/* Memory instructions which are fill/spill type */\n");
    emit_top_define ("TOP_is_mem_fill_type");
    emit_top_define ("TOP_is_load");
    emit_top_define ("TOP_is_store");
    emit_top_define ("TOP_is_unalign_ld");
    emit_top_define ("TOP_is_unalign_store");

    emit_top_define ("TOP_is_jump");
    emit_top_define ("TOP_is_ijump");
    emit_top_define ("TOP_is_branch");
    emit_top_define ("TOP_is_call");
    emit_top_define ("TOP_is_select");

    emit_top_define ("TOP_is_isub");
    emit_top_define ("TOP_is_ior");
    emit_top_define ("TOP_is_flop");
    emit_top_define ("TOP_is_fadd");
    emit_top_define ("TOP_is_fsub");
    emit_top_define ("TOP_is_fmul");
    emit_top_define ("TOP_is_fdiv");
    emit_top_define ("TOP_is_imul");
    emit_top_define ("TOP_is_idiv");
    emit_top_define ("TOP_is_icmp");
    emit_top_define ("TOP_is_madd");
    emit_top_define ("TOP_is_itrap");
    emit_top_define ("TOP_is_ftrap");
    emit_top_define ("TOP_is_memtrap");

    fprintf (hfile, "/* Instruction must be first in an instruction group */ \n");    
    emit_top_define ("TOP_is_f_group");

    fprintf (hfile, "/* Instruction must be last in an instruction group */ \n");
    emit_top_define ("TOP_is_l_group");

    fprintf (hfile, "/* Instruction accesses rotating register bank */ \n");
    emit_top_define ("TOP_is_access_reg_bank");
    emit_top_define ("TOP_is_unsafe");
    emit_top_define ("TOP_save_predicates");
    emit_top_define ("TOP_restore_predicates");

  // Emit the memory access functions:
  fprintf (hfile, "\n\n");
  fprintf (hfile, "extern const UINT32 TOP_Mem_Bytes (TOP opcode);");
  fprintf (hfile, "\n");
  fprintf (hfile, "extern const UINT32 TOP_Mem_Alignment (TOP opcode);");
  fprintf (hfile, "\n\n");

  fprintf (cfile, "\n");
  fprintf (cfile, "/* ============================================================\n");
  fprintf (cfile, " *  TOP_Mem_Bytes\n");
  fprintf (cfile, " * ============================================================\n");
  fprintf (cfile, " */\n");
  fprintf (cfile, "const UINT32 TOP_Mem_Bytes (TOP opcode) {\n");
  fprintf (cfile, "  switch (opcode) {\n");
  for (i = 1; i <= 16; i++) {
    bool opcode_exist = false;
    for (code = 0; code < TOP_count; code++) {

      //      fprintf(stderr," TOP_%s ... \n",TOP_Name((TOP)code));

      if (mem_bytes[code] == i) {
	opcode_exist = true;
	fprintf (cfile, "    case TOP_%s:\n", TOP_Name((TOP)code));
      }
    }
    if (opcode_exist == true) {
      fprintf (cfile, "      return %d;\n\n", i);
    }
  }
  fprintf (cfile, "    default:\n");
  fprintf (cfile, "      return 0;\n");
  fprintf (cfile, "  };\n");
  fprintf (cfile, "}\n");
  fprintf (cfile, "\n");

  fprintf (cfile, "/* ============================================================\n");
  fprintf (cfile, " *  TOP_Mem_Alignment\n");
  fprintf (cfile, " * ============================================================\n");
  fprintf (cfile, " */\n");
  fprintf (cfile, "const UINT32 TOP_Mem_Alignment (TOP opcode) {\n");
  fprintf (cfile, "  switch (opcode) {\n");
  for (i = 1; i <= 16; i++) {
    bool opcode_exist = false;
    for (code = 0; code < TOP_count; code++) {
      if (mem_align[code] == i) {
	opcode_exist = true;
	fprintf (cfile, "    case TOP_%s:\n", TOP_Name((TOP)code));
      }
    }
    if (opcode_exist == true) {
      fprintf (cfile, "      return %d;\n\n", i);
    }
  }
  fprintf (cfile, "    default:\n");
  fprintf (cfile, "      return 0;\n");
  fprintf (cfile, "  };\n");
  fprintf (cfile, "}\n");
  fprintf (cfile, "\n");

  Emit_Footer (hfile);

  fclose(hfile);
  fclose(cfile);
  fclose(efile);
}
