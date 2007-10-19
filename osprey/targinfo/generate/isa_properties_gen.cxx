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

enum {
  BIT_POS_NONE = -1
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
  " *",
  " *    Interface for dynamic extension DDLs:",
  " *",
  " *    For the extensions, there is no restriction on the number of used property,",
  " *    though the TOP_prop_tab table of TOP properties is a pointer to a variable",
  " *    length property mask. Depending on the number of properties as returned by",
  " *    ISA_PROPERTIES_attribute_tab_sz, the mask for each top is scaled to the",
  " *    corresponding multiple of 64 bits.",
  " *",
  " *    EXTENSION_ISA_PROPERTY_FLAGS* dyn_get_TOP_prop_tab(void)",
  " *        Get the mask of properties indexed by the TOP (extension top index).",
  " *        The kask is the set of active properties (indexed by extension property index).",
  " *        The mapping between the core properties index and the extension",
  " *        properties index is done by the loader with the interface below.",
  " *",
  " *    mUINT32 dyn_get_ISA_PROPERTIES_attribute_tab_sz();",
  " *        Get the size of the properties table. This correspond to the actual",
  " *        number of used properties in the extension.",
  " *",
  " *    ",
  " *    EXTENSION_ISA_PROPERTIES_ATTRIBUTE* dyn_get_ISA_PROPERTIES_attribute_tab();",
  " *        The table of properties, defined as a pair (name, identifier).",
  " *",
  " *   Important note: to keep backward compatibility with older extensions (previous to 20070901),",
  " *   we store in the properties attribute the mask value (1UL<<identifier) when the number of properties",
  " *   used by the extension is <= 64. Otherwise we store the actual identifier as a UINT64.",
  " *   Also, the loader must accept mask value of all 0 which means unused properties. In newer extension",
  " *   DLLs the unused properties are not referenced in the attribute table.",
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
  // How many non-constant properties?
  // Actually, now, for the core, it counts all declared properties, constant or not.
  // For an extension, only used properties are counted.
  int isa_property_count;	
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

  // For the static ISA we index all properties even the
  // ones that are always or never used.
  // - always used need am index in order to be matched
  // by name with the extension conterpart.
  // - never used need also an index as it may be used
  // by an extension.
  // For extensions ISA we index only used properties,
  // indeed in this case the non used property can be 
  // removed because it will never match the extension 
  // tops anyway and the static ISA as already defined 
  // an index for it. 
  // There is no more restriction concerning always FALSE
  // or always TRUE property in an extension or in the static
  // ISA. Every property defined in the static ISA get an index
  // and thus can be either always TRUE or always FALSE
  // in the core and/or in the extensions.
  isa_property_count = 0;
  for ( isi = properties.begin(); isi != properties.end(); ++isi ) {
    ISA_PROPERTY property = *isi;
    bool used = false;
    for (code = 0; code < TOP_count_limit ; code++) {
      used = property->members[code];
      if (used) break;
    }
    if (used || gen_static_code) {
      property->bit_position = isa_property_count;
      isa_property_count++;
    } else {
      /* Non used properties in extension are ignored. */
      property->bit_position = BIT_POS_NONE;
    }
  }

  // In the following, we generate the property list for extension
  // into a header file. At definition of the extension the header
  // can be include to provide the whole set of available 
  // properties (as defined by the static ISA).
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

     for(isi = properties.begin(); isi != properties.end(); ++isi ) {
       ISA_PROPERTY property = *isi;
       fprintf(cincfile,
	       "ISA_PROPERTY prop_%-15s = ISA_Property_Create(\"%s\");\n",
	       property->name,property->name);
     }

    fprintf(cincfile,"\n\n");

    Gen_Close_File_Handle(cincfile,efilename);
   }
  

  const char *ident_int_type;
  const char *ident_int_suffix;
  int ident_int_size;
  const char *mask_int_type;
  const char *mask_int_suffix;
  int mask_word_size;
  int mask_word_count;
  int mask_size;
  
  // Type for the properties masks used to be dynamically
  // defined as a function of the number of properties. 
  //
  // From now, there is two distinct types: 
  // 1. the property mask as in the TOP properties tables:
  //    used to store a set of properties,
  //    defined below by the mask_int_type and the
  //    mask_word_count. The length of the property mask
  //    is variable and depends on the property count.
  //    Note that the mask for the static ISA and the mask
  //    for an extension can differ in size. Inded, in extensions
  //    only the used properties are counted.
  // 2. the property identifier (or value) as in the properties
  //    attributes table: it used to be stored as a mask
  //    computed as (1<<identifier) on UINT64 (type identified by
  //    ident_int_type). 
  //    There are now two distinct cases:
  //    2a. Static ISA: the representation has been changed to store
  //        the actual property identifier in a UINT64.
  //    2b. Extension: for backward compatibility we keep this "mask"
  //        representation while the number of used properties is <= 64. 
  //        Above 64 properties in the extension, the identifier 
  //        is stored as its actual value on UINT64.
  //        Also to keep backward compatibility, a loader must keep interpreting
  //        a property "mask" identifier of all zero as an unused
  //        property (or identifier -1). Though in newly generated DLL,
  //        the property attributes are not generated at all.
  
  ident_int_type = "mUINT64";
  ident_int_suffix = "ULL";
  ident_int_size = 64;

  
  /* Compute mask word_count and size for storing properties set. */
  mask_word_size = 64;
  mask_int_type = "mUINT64";
  mask_int_suffix = "ULL";
  mask_word_count = (isa_property_count + (mask_word_size - 1)) / mask_word_size;
  mask_size = mask_word_size * mask_word_count;
  assert(isa_property_count <= mask_size);

  // Statically defined property tables.
  // The static ISA table uses the full property set.
  // The extension ISA table uses a limited set.
  // We use a technical type for the non exported implementation.
  fprintf (cfile,"\ntypedef struct { %s mask[%d]; } LOCAL_ISA_PROPERTY_FLAGS; /* A %d bits property set. */\n\n", mask_int_type, mask_word_count, mask_size);
  if(gen_static_code) {
    fprintf (cfile,"static const LOCAL_ISA_PROPERTY_FLAGS ISA_PROPERTIES_static_flags[] = {\n");
  } else {
    fprintf (cfile,"static const LOCAL_ISA_PROPERTY_FLAGS ISA_PROPERTIES_dynamic_flags[] = {\n");
  }

  const char* top_name_template = gen_static_code ? " /* TOP_%-25s */  " :  " /* TOP_dyn_%-25s */  " ;
  
  for (code = 0; code < TOP_count_limit; code++) {
    const char *sep;
    unsigned long long flag_value[mask_word_count]; // variable length flag, low indexes in low words.
    int i;
    
    memset(flag_value, 0, sizeof(unsigned long long)*mask_word_count);
    
    fprintf(cfile,top_name_template, TOP_Name((TOP)code));

    for ( isi = properties.begin(); isi != properties.end(); ++isi ) {
      ISA_PROPERTY property = *isi;
      if (property->members[code]) {
	int word_idx = property->bit_position / mask_word_size;
	int bit_pos = property->bit_position % mask_word_size;
	flag_value[word_idx] |= (1ULL << bit_pos);
      }
    }
    
    fprintf (cfile, "{ ");
    sep = "";
    for (i = 0; i < mask_word_count; i++) {
      fprintf (cfile, "%s0x%0*llx%s", sep, mask_word_size / 4, flag_value[i], mask_int_suffix);
      sep = ", ";
    }
    fprintf (cfile, " }, ");

    fprintf (cfile, "/* ");
    sep = "";
    for ( isi = properties.begin(); isi != properties.end(); ++isi ) {
      ISA_PROPERTY property = *isi;
      if (property->members[code]) {
	fprintf(cfile, "%sPROP_%s", sep, property->name);
	sep = ", ";
      }
    }
    fprintf (cfile, " */\n");
  }        // End of for(code...) loop.

  if(gen_static_code) {
    // don't forget the one for TOP_UNDEFINED !
    const char *sep;
    int i;
    
    fprintf(cfile,top_name_template, "TOP_UNDEFINED");
    fprintf (cfile, "{ ");
    sep = "";
    for (i = 0; i < mask_word_count; i++) {
      fprintf (cfile, "%s0x%0*llx%s", sep, mask_word_size / 4, 0ULL, mask_int_suffix);
      sep = ", ";
    }
    fprintf (cfile, " },\n");
  }
  fprintf(cfile, "};\n");   // Ending table.

  if(gen_static_code) {
    // Global properties mask type, this type is used for the static ISA only.
    fprintf(cfile,"const ISA_PROPERTY_FLAGS *ISA_PROPERTIES_flags = (const ISA_PROPERTY_FLAGS *)ISA_PROPERTIES_static_flags ;\n");
    fprintf(hfile, "\ntypedef struct { %s mask[%d]; } ISA_PROPERTY_FLAGS;  /* A %d bits property set. */\n\n", mask_int_type, mask_word_count, mask_size); 
    fprintf(hfile, "BE_EXPORTED extern const ISA_PROPERTY_FLAGS *ISA_PROPERTIES_flags;\n\n");
    fprintf(efile, "ISA_PROPERTIES_flags\n");
  } else { 
    // Definition of interface routine.
    const char * const name_routine = "dyn_get_TOP_prop_tab";
    
    fprintf(cfile,"\n"
	    "const EXTENSION_ISA_PROPERTY_FLAGS * %s( void ) {\n"
	    " return (const EXTENSION_ISA_PROPERTY_FLAGS *) ISA_PROPERTIES_dynamic_flags;\n"
	    "}\n\n", name_routine);
    
    fprintf(hfile,"\n\n"
	    "/* Variable length type for the properties mask. */\n"
	    "typedef struct { %s flags[1]; } EXTENSION_ISA_PROPERTY_FLAGS;\n\n", 
	    mask_int_type);
    fprintf(hfile,"\n\n"
	    "/* API routine for dll */\n"
	    "extern const EXTENSION_ISA_PROPERTY_FLAGS * %s( void );\n\n", 
	    name_routine);
  }

  if(gen_static_code) {
    fprintf (hfile, "\n\n");
    for ( isi = properties.begin(); isi != properties.end(); ++isi ) {
      ISA_PROPERTY property = *isi;
      int word_idx = property->bit_position / mask_word_size;
      int bit_pos = property->bit_position % mask_word_size;
      fprintf (hfile, "#define TOP_is_%s(t)\t (ISA_PROPERTIES_flags[(INT)t].mask[%d] & 0x%llx%s)\n",
	       property->name, word_idx, (1ULL << bit_pos), mask_int_suffix);
    }
  } else {
    // For extension we generate the TOP_is_ macro but just at
    // extension generation time. Thus we use the DYNAMIC_CODE_GEN macro.
    // For instsance isa_print_gen.cxx need TOP_is_simulated() to be available
    // both for the STATIC ISA or for the extension when generating the extension
    // itself.
    fprintf (hfile, "\n\n");
    fprintf (hfile, "/* Available at extension generation time only. */\n");
    fprintf (hfile, "#ifdef DYNAMIC_CODE_GEN\n");
    fprintf (hfile, "/* We undef first the static ISA definition of each TOP_is...(). */\n");
    for ( isi = properties.begin(); isi != properties.end(); ++isi ) {
      ISA_PROPERTY property = *isi;
      fprintf (hfile, "#undef TOP_is_%s\n", property->name);
      if (property->bit_position == BIT_POS_NONE) {
	fprintf (hfile, "#define TOP_is_%s(t)\t (0)\n", property->name);
      } else { 
	int word_idx = property->bit_position / mask_word_size;
	int bit_pos = property->bit_position % mask_word_size;
	fprintf (hfile, "#define TOP_is_%s(t)\t (dyn_get_TOP_prop_tab()[(INT)t].flags[%d] & 0x%llx%s)\n",
		 property->name, word_idx, (1ULL << bit_pos), mask_int_suffix);
      }
    }
    fprintf (hfile, "#endif /* DYNAMIC_CODE_GEN */\n");
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

  fprintf (cfile,
	   "/* ============================================================\n"
	   " *  ISA PROPERTIES attributes description\n"
	   " * ============================================================\n"
	   " */\n");

  // Generate datatype specification
  if (gen_static_code) {
    fprintf (hfile,"\n/* Description of all statically defined attributes */\n");
    fprintf(hfile,
	    "typedef struct { const char *name; %s ident; } ISA_PROPERTIES_ATTRIBUTE;\n", ident_int_type);
  } else {
    fprintf (hfile,"\n/* Description of attributes used by the extension */\n");
    fprintf(hfile,
	    "/* Variable length type for the properties attributes. */\n"
	    "typedef struct { const char *name; %s ident; } EXTENSION_ISA_PROPERTIES_ATTRIBUTE;\n", 
	    ident_int_type);
  }
  
  // Generate attribute description table
  fprintf (cfile,"typedef struct { const char *name; %s ident;} LOCAL_ISA_PROPERTIES_ATTRIBUTE;\n", ident_int_type);
  fprintf (cfile, "static const LOCAL_ISA_PROPERTIES_ATTRIBUTE ISA_PROPERTIES_attribute_tab[] = {\n");
  for ( isi = properties.begin(); isi != properties.end(); ++isi ) {
    ISA_PROPERTY property = *isi;
    if (property->bit_position != BIT_POS_NONE) {
      unsigned long long identifier_value;
      bool store_mask = false;

      /* For the static ISA, the actual identifier is always stored. 
       * For extensions, if property count <= 64, a mask is stored (backward compatibility),
       * otherwise the actual identifier is stored.
       */
      if (!gen_static_code && isa_property_count <= ident_int_size) {
	store_mask = true;
      }

      fprintf (cfile, "  { \"%s\", ", property->name);
      if (store_mask) {
	fprintf (cfile, "0x%0*llx%s /* (1%s << %d) */ },\n", ident_int_size / 4, 
		 (1ULL << property->bit_position), ident_int_suffix,
		 ident_int_suffix, property->bit_position);
      } else {
	fprintf (cfile, "0x%0*x%s /* (%d%s) */ },\n", ident_int_size / 4, 
		 property->bit_position, ident_int_suffix,
		 property->bit_position, ident_int_suffix);
      }
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
     "  return (const ISA_PROPERTIES_ATTRIBUTE *)ISA_PROPERTIES_attribute_tab;\n"
     "}\n"
     "\n");

    fprintf(efile, "ISA_PROPERTIES_get_attribute_table\n");
  }
  else {
    fprintf (hfile,
	     "\n"
	     "extern const mUINT32 dyn_get_ISA_PROPERTIES_attribute_tab_sz();\n"
	     "\n"
	     "extern const EXTENSION_ISA_PROPERTIES_ATTRIBUTE* dyn_get_ISA_PROPERTIES_attribute_tab();\n"
	     "\n");
    fprintf (cfile,
	     "\n"
	     "const mUINT32 dyn_get_ISA_PROPERTIES_attribute_tab_sz() {\n"
	     "  return (%d);\n"
	     "}\n"
	     "\n", isa_property_count);
    fprintf (cfile,
	     "const EXTENSION_ISA_PROPERTIES_ATTRIBUTE* dyn_get_ISA_PROPERTIES_attribute_tab() {\n"
	     "  return (const EXTENSION_ISA_PROPERTIES_ATTRIBUTE *)ISA_PROPERTIES_attribute_tab;\n"
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
