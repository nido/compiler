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
  fprintf (hfile, "TARGINFO_EXPORTED extern const %s ISA_PROPERTIES_flags[];\n\n", int_type);
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
  // don't forget the one for TOP_UNDEFINED !
  fprintf (cfile, "  0x%0*llx%s  /* TOP_UNDEFINED */ \n", int_size / 4,
					    0ULL, 
					    int_suffix);
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
    //    else {
    //      fprintf (hfile, "#define PROP_%-16s (FALSE)\n", property->name);
    //    }
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

  // Emit the memory access properties:
  fprintf (hfile, "\n\n");
  fprintf (hfile, "TARGINFO_EXPORTED extern const UINT32 TOP_Mem_Bytes (TOP opcode);");
  fprintf (hfile, "\n");
  fprintf (hfile, "TARGINFO_EXPORTED extern const UINT32 TOP_Mem_Alignment (TOP opcode);");
  fprintf (hfile, "\n\n");

  fprintf (hfile, "TARGINFO_EXPORTED extern TOP TOP_Immediate_Compare (TOP opcode, INT64 const_val);");
  fprintf (hfile, "\n");
  fprintf (hfile, "TARGINFO_EXPORTED extern TOP TOP_Reverse_Compare (TOP opcode);");
  fprintf (hfile, "\n\n");

  fprintf (cfile, "\n");

  // --------------------------------------------------------------------
  //
  //       TOP_Mem_Bytes
  //
  // --------------------------------------------------------------------

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

  // --------------------------------------------------------------------
  //
  //       TOP_Mem_Alignment
  //
  // --------------------------------------------------------------------

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

  // --------------------------------------------------------------------
  //
  //       TOP_Immediate_Compare
  //
  // --------------------------------------------------------------------

  fprintf (cfile, "/* ============================================================\n");
  fprintf (cfile, " *  TOP_Immediate_Compare\n");
  fprintf (cfile, " * ============================================================\n");
  fprintf (cfile, " */\n");
  fprintf (cfile, "TOP TOP_Immediate_Compare (TOP opcode, INT64 const_val) {\n");

  fprintf (cfile, "  return TOP_UNDEFINED;\n");

  fprintf (cfile, "}\n");
  fprintf (cfile, "\n");

  // --------------------------------------------------------------------
  //
  //       TOP_Reverse_Compare
  //
  // --------------------------------------------------------------------

  fprintf (cfile, "/* ============================================================\n");
  fprintf (cfile, " *  TOP_Reverse_Compare\n");
  fprintf (cfile, " * ============================================================\n");
  fprintf (cfile, " */\n");
  fprintf (cfile, "TOP TOP_Reverse_Compare (TOP opcode) {\n");

  fprintf (cfile, "  return TOP_UNDEFINED;\n");

  fprintf (cfile, "}\n");
  fprintf (cfile, "\n");


  Emit_Footer (hfile);

  fclose(hfile);
  fclose(cfile);
  fclose(efile);
}
