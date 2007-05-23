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


#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
// [HK]
#if __GNUC__ >=3 || defined(_MSC_VER)
#include <list>
#include <vector>
using std::list;
using std::vector;
#else
#include <list.h>
#include <vector.h>
#endif // __GNUC__ >=3 || defined(_MSC_VER)
#include "topcode.h"
#include "gen_util.h"
#include "isa_properties_gen.h"


/* ====================================================================
 *   Specific adaptation for dynamic code generation
 * ====================================================================
 */
#ifdef DYNAMIC_CODE_GEN
#include "dyn_isa_topcode.h"
#endif

// In following loops, we iterate on the number of
// TOP. This number differs following we generate
// static or dynamic TOP.
#ifndef DYNAMIC_CODE_GEN
static TOP TOP_count_limit = TOP_static_count;
#else
static TOP TOP_count_limit = TOP_dyn_count;
#endif
// ====================================================================
// ====================================================================

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
  " *   ISA_PROPERTIES_ATTRIBUTE_COUNT",
  " *       Constant value representing the number of properties effectively",
  " *       used by TOPs",
  " *",
  " *   const ISA_PROPERTIES_ATTRIBUTE *ISA_PROPERTIES_get_attribute_table()",
  " *       Return the table describing properties used by TOPs",
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
  mem_bytes = vector <short> (TOP_count_limit, 0);
  mem_align = vector <short> (TOP_count_limit, 0);

  return;
}

/* ====================================================================
 *   ISA_Memory_Access
 * ====================================================================
 */
void ISA_Memory_Access (int bytes, ... ) {
  va_list ap;
  TOP opcode;

  // Whether we generate code for the core (static) or for an extension.
  bool  gen_static_code = Is_Static_Code();

  /*
   * For static code generation, TOP_UNDEFINED is the
   * last item of the vararg list. In case of dynamic
   * code generation, the static value of TOP_UNDEFINED
   * cannot be used anymore.  Indeed we can define in the
   * dynamic part of the code, more instructions than
   * they are in the static instruction set.
   * To solve the problem, we suppose that in case
   * of dynamic code generation, vararg list ends with -1.
   */

  TOP limit = gen_static_code ? TOP_UNDEFINED : static_cast<TOP>(-1);

  // do not allow memory accesses/alignments more than these
  assert (bytes <= 64);

  va_start(ap, bytes);
  while ( (opcode = static_cast<TOP>(va_arg(ap,int))) != limit ) {
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

  // Whether we generate code for the core (static) or for an extension.
  bool  gen_static_code = Is_Static_Code();

  /*
   * For static code generation, TOP_UNDEFINED is the
   * last item of the vararg list. In case of dynamic
   * code generation, the static value of TOP_UNDEFINED
   * cannot be used anymore.  Indeed we can define in the
   * dynamic part of the code, more instructions than
   * they are in the static instruction set.
   * To solve the problem, we suppose that in case
   * of dynamic code generation, vararg list ends with -1.
   */

  TOP limit = gen_static_code ? TOP_UNDEFINED : static_cast<TOP>(-1);

  // do not allow memory alignments more than these
  assert (bytes <= 64);

  va_start(ap, bytes);
  while ( (opcode = static_cast<TOP>(va_arg(ap,int))) != limit ) {
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
  result->members = vector <bool> (TOP_count_limit, false);

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

  // Whether we generate code for the core (static) or for an extension.
  bool  gen_static_code = Is_Static_Code();

  /*
   * For static code generation, TOP_UNDEFINED is the
   * last item of the vararg list. In case of dynamic
   * code generation, the static value of TOP_UNDEFINED
   * cannot be used anymore.  Indeed we can define in the
   * dynamic part of the code, more instructions than
   * they are in the static instruction set.
   * To solve the problem, we suppose that in case
   * of dynamic code generation, vararg list ends with -1.
   */

  TOP limit = gen_static_code ? TOP_UNDEFINED : static_cast<TOP>(-1);

  va_start(ap,property);
  while ( (opcode = static_cast<TOP>(va_arg(ap,int))) != limit ) {
    property->members[(int)opcode] = true;
  }
  va_end(ap);
}



/* ====================================================================
 *   ISA_Properties_End
 * ====================================================================
 */
void ISA_Properties_End(void)
{
  list<ISA_PROPERTY>::iterator isi;
  int isa_property_count;	// How many non-constant properties?
  unsigned int code;

  static FILE* hfile    = NULL ;
  static FILE* cfile    = NULL ;
  static FILE* efile    = NULL ;

  // Whether we generate code for the core (static) or for an extension.
  bool  gen_static_code = Is_Static_Code();

  // Get extension name or NULL for static code generation.
  char *extname = gen_static_code ? NULL : Get_Extension_Name();

  char *hfilename     = NULL ;    /* Header file name              */
  char *cfilename     = NULL ;    /* C file name                   */
  char *efilename     = NULL ;    /* Export file name              */

  const char * const bname = FNAME_TARG_ISA_PROPERTIES;

  hfilename = Gen_Build_Filename(bname,extname,gen_util_file_type_hfile);
  hfile     = Gen_Open_File_Handle(hfilename, "w");

  cfilename = Gen_Build_Filename(bname,extname,gen_util_file_type_cfile);
  cfile     = Gen_Open_File_Handle(cfilename, "w");

  if(gen_static_code)
   { efilename = Gen_Build_Filename(bname,extname,gen_util_file_type_efile);
     efile     = Gen_Open_File_Handle(efilename, "w");
   }


  Emit_C_Header(cfile);
  fprintf(cfile,  
          "\n"
          "\n"
          "#include \"%s\"\n"
          "\n", 
          gen_static_code ? hfilename : "dyn_isa_properties.h");


  Emit_Header (hfile, FNAME_TARG_ISA_PROPERTIES, interface, extname);
  fprintf(hfile,"#include \"topcode.h\"\n\n");

  isa_property_count = 0;
  for ( isi = properties.begin(); isi != properties.end(); ++isi ) {
    ISA_PROPERTY property = *isi;
    bool member = true;     // dummy init. to avoid gcc warning messages.
    bool prev_member = property->members[0];

    for (code = 1; code < TOP_count_limit ; code++) {
      member = property->members[code];
      if (member != prev_member) break;
    }

    // For dynamic code, we don't set BIT_POS_ALL
    if (member != prev_member || (!gen_static_code && member)) {
      property->bit_position = isa_property_count++;
    } else if (!gen_static_code) {
      property->bit_position = BIT_POS_NONE;
      isa_property_count++;
    } else {
      property->bit_position = member ? BIT_POS_ALL : BIT_POS_NONE;
    }
   }


  // Here we make two important assumptions:
  // 1 - we assume that properties that are always TRUE for the static
  //     isa are also TRUE for the dynamic isa.
  // 2 - we assume that properties that are always FALSE for the static
  //     isa are also FALSE for the dynamic isa.
  //
  // In the following, we generate the property list for extension.
  if(gen_static_code)
   { FILE *cincfile;
     char *cincfilename;
 
     cincfilename = Gen_Build_Filename(bname,extname,gen_util_file_type_cpp_i_file);
     cincfile     = Gen_Open_File_Handle(cincfilename, "w");

     fprintf(cincfile,
              "\n\n"
              "/* Warning, this file has been automatically generated\n"
              " * Do not modify it.\n"
              " * This file lists the isa properties.\n"
              " */\n\n");

     for(isi = properties.begin(); isi != properties.end(); ++isi )
      { ISA_PROPERTY property = *isi;
        if (property->bit_position >= 0) {
           fprintf(cincfile,
                   "ISA_PROPERTY prop_%-15s = ISA_Property_Create(\"%s\");\n",
                   property->name,property->name);
        }
      }

    fprintf(cincfile,"\n\n");

    Gen_Close_File_Handle(cincfile,efilename);
   }
  

  char *int_type;
  char *int_suffix;
  int int_size;

#if 0
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
#endif

  // Type used to be dynamically defined as a function of
  // the number of properties. For the sake of simplicity,
  // we work in all cases with unsigned long long.
  assert(isa_property_count <= 64);
  int_type = "mUINT64";
  int_suffix = "ULL";
  int_size = 64;


  // Table ISA_PROPERTIES_flags
  if(gen_static_code)
   {fprintf (cfile,"static const %s ISA_PROPERTIES_static_flags[] = {\n", int_type);
   }
  else
   {fprintf (cfile,"static const %s ISA_PROPERTIES_dynamic_flags[] = {\n",int_type);
   }

  const char* top_name_template = gen_static_code ? " /* TOP_%-25s */  " :  " /* TOP_dyn_%-25s */  " ;

  for (code = 0; code < TOP_count_limit; code++) {
    bool               no_prop    = true; // True as long as as no
                                          // property has been detected.

    fprintf(cfile,top_name_template, TOP_Name((TOP)code));

    for ( isi = properties.begin(); isi != properties.end(); ++isi ) {
      ISA_PROPERTY property = *isi;
      if (property->bit_position >= 0 && property->members[code]) {

        // When property is not the first one,
        // add '|' bitwise operator in generated code.
        fprintf(cfile,no_prop==false? " | PROP_%s" : "PROP_%s",property->name);
        no_prop = false;   /* Now we've got at least one prop. for the TOP */
      }
    }    /* End for isi=... */

    if(true == no_prop)
    { fprintf(cfile, "0x%0*x%s",int_size/4,0,int_suffix);
    }

    fprintf(cfile,",\n");  // Add final ',' for the TOP
  }        // End of for(code...) loop.

  if(gen_static_code) {
    // don't forget the one for TOP_UNDEFINED !
    fprintf (cfile, top_name_template , "UNDEFINED");
    fprintf (cfile, "0x%0*x%s,\n", 
             int_size / 4,
             0,
	     int_suffix);
   }
  fprintf(cfile, "};\n");   // Ending table.

  if(gen_static_code)
   { fprintf(cfile,"const %s *ISA_PROPERTIES_flags = ISA_PROPERTIES_static_flags ;\n",int_type);
     fprintf(hfile, "BE_EXPORTED extern const %s *ISA_PROPERTIES_flags;\n\n", int_type);
     fprintf(efile, "ISA_PROPERTIES_flags\n");
   }
 else
   { // Definition of interface routine.
     const char * const name_routine = "dyn_get_TOP_prop_tab";

     fprintf(cfile,"\n"
                   "const %s* %s( void ) {\n"
                   " return (const %s*) ISA_PROPERTIES_dynamic_flags;\n"
                   "}\n\n",
                   int_type,
                   name_routine,
                   int_type);

     fprintf(hfile,"\n\n"
                   "/* API routine for dll */\n"
                   "extern const %s* %s( void );\n\n",
                   int_type,
                   name_routine);
   }

  // Emit PROP_xxx bits in header file.
  if(gen_static_code) {
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
  }

  if(gen_static_code) {
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
 }

  // Emit the memory access properties:
  if(gen_static_code) {
  fprintf (hfile, "\n\n");
  fprintf (hfile, "BE_EXPORTED extern const UINT32 TOP_Mem_Bytes (TOP opcode);");
  fprintf (hfile, "\n");
  fprintf (hfile, "TARGINFO_EXPORTED extern const UINT32 TOP_Mem_Alignment (TOP opcode);");
  fprintf (hfile, "\n\n");

  fprintf (hfile, "TARGINFO_EXPORTED extern TOP TOP_Immediate_Compare (TOP opcode, INT64 const_val);");
  fprintf (hfile, "\n");
  fprintf (hfile, "TARGINFO_EXPORTED extern TOP TOP_Reverse_Compare (TOP opcode);");
  fprintf (hfile, "\n\n");

  fprintf (cfile, "\n");
  }   // if(gen_static_code)

  // --------------------------------------------------------------------
  //
  //       TOP_Mem_Bytes
  //
  // --------------------------------------------------------------------

  fprintf (cfile, "/* ============================================================\n");
  fprintf (cfile, " *  TOP_Mem_Bytes\n");
  fprintf (cfile, " * ============================================================\n");
  fprintf (cfile, " */\n");

#if 0
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
#endif

  // Look up table emission.

  const char *str_template = gen_static_code             ?
                             "%3d, /* TOP_%-40s */\n"    : 
                             "%3d, /* TOP_dyn_%-40s */\n";

  if(gen_static_code)
    fprintf (cfile, "static const mUINT16 ISA_PROPERTIES_TOP_static_mem_bytes [ ] = {\n");
  else
    fprintf (cfile, "static const mUINT16 ISA_PROPERTIES_TOP_dynamic_mem_bytes [ ] = {\n");

  for(code=0;code<TOP_count_limit;code++)
    fprintf (cfile, str_template,mem_bytes[code],TOP_Name((TOP)code));
  
  // Don't forget TOP_UNDEFINED for static code generation
  if(gen_static_code)
    fprintf(cfile,str_template,0,TOP_Name((TOP)TOP_UNDEFINED));

  fprintf(cfile,"};\n\n");  // End of table.

  // Printing routine.
  if(gen_static_code)
  {  fprintf(cfile,
      "const mUINT16* ISA_PROPERTIES_TOP_mem_bytes = ISA_PROPERTIES_TOP_static_mem_bytes;\n\n");
     fprintf(hfile,
      "\n"
      "BE_EXPORTED extern const unsigned short *ISA_PROPERTIES_TOP_mem_bytes;\n");

     fprintf(cfile, "const UINT32 TOP_Mem_Bytes (TOP opcode) {\n"
                    " return ISA_PROPERTIES_TOP_mem_bytes[opcode];\n"
                    "}\n"
                    "\n"
            );
  }                  // gen_static_code
  else
  { char* name_routine = "dyn_get_TOP_mem_bytes_tab";

    fprintf(cfile,"\n"
                  "\n"
                  "const mUINT16* %s ( void ) {\n"
                  "  return (const mUINT16*) ISA_PROPERTIES_TOP_dynamic_mem_bytes;\n"
                  "};\n"
                  "\n",
                  name_routine);

    fprintf(hfile,"\n"
                  "\n"
                  "extern const mUINT16* %s (void );\n"
                  "\n",
                  name_routine);
  }


  // --------------------------------------------------------------------
  //
  //       TOP_Mem_Alignment
  //
  // --------------------------------------------------------------------

  fprintf (cfile, "/* ============================================================\n");
  fprintf (cfile, " *  TOP_Mem_Alignment\n");
  fprintf (cfile, " * ============================================================\n");
  fprintf (cfile, " */\n");

#if 0
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
#endif

  // Look up table emission.
  if(gen_static_code)
    fprintf (cfile, "static const mUINT16 ISA_PROPERTIES_TOP_static_mem_align [ ] = {\n");
  else
    fprintf (cfile, "static const mUINT16 ISA_PROPERTIES_TOP_dynamic_mem_align [ ] = {\n");

  for(code=0;code<TOP_count_limit;code++)
    fprintf (cfile, str_template,mem_align[code],TOP_Name((TOP)code));

  // Don't forget TOP_UNDEFINED for static code generation
  if(gen_static_code)
    fprintf(cfile,str_template,0,TOP_Name((TOP)TOP_UNDEFINED));

  fprintf(cfile,"};\n\n");  // End of table.

  // Printing routine.
  if(gen_static_code)
  {   fprintf(cfile,
      "const mUINT16 *ISA_PROPERTIES_TOP_mem_align = ISA_PROPERTIES_TOP_static_mem_align;\n\n");
      fprintf(hfile,
      "\n"
      "BE_EXPORTED extern const unsigned short *ISA_PROPERTIES_TOP_mem_align;\n");

      fprintf(cfile, "const UINT32 TOP_Mem_Alignment (TOP opcode) {\n"
                     " return ISA_PROPERTIES_TOP_mem_align[opcode];\n"
                     "}\n"
                     "\n"
            );
  }                          // gen_static_code
  else
  { char* name_routine = "dyn_get_TOP_mem_align_tab";

    fprintf(cfile,"\n"
                  "\n"
                  "const mUINT16* %s ( void ) {\n"
                  "  return (const mUINT16*) ISA_PROPERTIES_TOP_dynamic_mem_align;\n"
                  "};\n"
                  "\n",
                  name_routine);

    fprintf(hfile,"\n"
                  "\n"
                  "extern const mUINT16* %s (void );\n"
                  "\n",
                  name_routine);
  }

  // --------------------------------------------------------------------
  //
  //       TOP_Immediate_Compare
  //
  // --------------------------------------------------------------------

  if(gen_static_code) {
  fprintf (cfile, "/* ============================================================\n");
  fprintf (cfile, " *  TOP_Immediate_Compare\n");
  fprintf (cfile, " * ============================================================\n");
  fprintf (cfile, " */\n");
  fprintf (cfile, "TOP TOP_Immediate_Compare (TOP opcode, INT64 const_val) {\n");

  fprintf (cfile, "  return TOP_UNDEFINED;\n");

  fprintf (cfile, "}\n");
  fprintf (cfile, "\n");
  }                                   /* gen_static_code */

  // --------------------------------------------------------------------
  //
  //       TOP_Reverse_Compare
  //
  // --------------------------------------------------------------------

  if(gen_static_code) {
  fprintf (cfile, "/* ============================================================\n");
  fprintf (cfile, " *  TOP_Reverse_Compare\n");
  fprintf (cfile, " * ============================================================\n");
  fprintf (cfile, " */\n");
  fprintf (cfile, "TOP TOP_Reverse_Compare (TOP opcode) {\n");

  fprintf (cfile, "  return TOP_UNDEFINED;\n");

  fprintf (cfile, "}\n");
  fprintf (cfile, "\n");
  }                                   /* gen_static_code */

  // --------------------------------------------------------------------
  //
  //      ISA PROPERTIES attributes table and access functions
  //
  // Used in reconfigurability to check Core/Extension targinfo
  // compatibility and potentially allow attribute id remapping 
  // when loading extensions.
  //
  // --------------------------------------------------------------------

  fprintf (hfile,
	   "\n"
	   "/* Description of Used attributes */");
  fprintf (cfile,
	   "/* ============================================================\n"
	   " *  ISA PROPERTIES attributes description\n"
	   " * ============================================================\n"
	   " */\n");

  // Generate datatype specification
  if (gen_static_code) {
    fprintf(hfile,
	    "typedef struct { const char *name; UINT64 attribute; } ISA_PROPERTIES_ATTRIBUTE;\n");
  }
  
  // Generate attribute description table
  fprintf (cfile, "static const ISA_PROPERTIES_ATTRIBUTE ISA_PROPERTIES_attribute_tab[] = {\n");
  for ( isi = properties.begin(); isi != properties.end(); ++isi ) {
    ISA_PROPERTY property = *isi;
    if (property->bit_position >= 0) {
      fprintf (cfile, "  { \"%s\", PROP_%s},\n", property->name, property->name);
    } else if (!gen_static_code) {
      fprintf (cfile, "  { \"%s\", 0ULL},  \t/* Not used in extension */\n", property->name);
    }
  }
  fprintf (cfile, "};\n\n");
  
  // Generate access to attribute description table
  if (gen_static_code) {

    fprintf (
     hfile,
     "\n"
     "#define ISA_PROPERTIES_ATTRIBUTE_COUNT (%d)\n", isa_property_count);

    fprintf (
     hfile,
     "\n"
     "BE_EXPORTED extern const ISA_PROPERTIES_ATTRIBUTE * ISA_PROPERTIES_get_attribute_table();\n"
     "\n");

    fprintf (
     cfile,
     "BE_EXPORTED const ISA_PROPERTIES_ATTRIBUTE * ISA_PROPERTIES_get_attribute_table() {\n"
     "  return (ISA_PROPERTIES_attribute_tab);\n"
     "}\n"
     "\n");

    fprintf(efile, "ISA_PROPERTIES_get_attribute_table\n");
  }
  else {
    fprintf (hfile,
	     "\n"
	     "extern const mUINT32 dyn_get_ISA_PROPERTIES_attribute_tab_sz();\n"
	     "\n"
	     "extern const ISA_PROPERTIES_ATTRIBUTE* dyn_get_ISA_PROPERTIES_attribute_tab();\n"
	     "\n");
    fprintf (cfile,
	     "\n"
	     "const mUINT32 dyn_get_ISA_PROPERTIES_attribute_tab_sz() {\n"
	     "  return (%d);\n"
	     "}\n"
	     "\n", properties.size());
    fprintf (cfile,
	     "const ISA_PROPERTIES_ATTRIBUTE* dyn_get_ISA_PROPERTIES_attribute_tab() {\n"
	     "  return (ISA_PROPERTIES_attribute_tab);\n"
	     "}\n"
	     "\n");
  }

  Emit_Footer (hfile);
  Emit_C_Footer(cfile);               /* "C" directive   */

  // Closing file handlers.
  Gen_Close_File_Handle(hfile,hfilename);
  Gen_Close_File_Handle(cfile,cfilename);
  if(efile)
    Gen_Close_File_Handle(efile,efilename);

  // Memory deallocation.
  Gen_Free_Filename(cfilename);
  Gen_Free_Filename(hfilename);
  if(efilename)
    Gen_Free_Filename(efilename);

  return;
}
