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


//  abi_properties_gen.cxx
/////////////////////////////////////
//
//  Description:
//
//      Generate a description of the ABI properties.
//
/////////////////////////////////////
//

#ifndef _MSC_VER
#include <strings.h>
#else
#include <string.h>
#endif
#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
// [HK]
#if __GNUC__ >=3 || defined(_MSC_VER)
#include <list>
using std::list;
#else
#include <list.h>
#endif // __GNUC__ >=3 || defined(_MSC_VER)
#include "gen_util.h"
#if 0
#include "targ_isa_registers.h"
#endif
#include "abi_properties_gen.h"

#ifdef DYNAMIC_CODE_GEN
#include "dyn_isa_registers.h"
#else
#include "targ_isa_registers.h"  
#endif

#include "isa_ext_limits.h"

//
// Information about a property
//
struct abi_property {
  const char* name;		// Name given for documentation and debugging
  bool is_reg;			// Register or non-register
  bool is_flag;			// Boolean flag or value
  unsigned long long v;		// Flag mask (is_flag) or value (!is_flag)
};

//
// Information about an ABI
//
typedef struct abi {
  const char *name;		// Name
  list<ABI_PROPERTY> flags;	// Non-register flag properties
  list<ABI_PROPERTY> values;	// Non-register value properties
  list<ABI_PROPERTY> reg_flags[ISA_REGISTER_CLASS_MAX_LIMIT+1][ISA_REGISTER_MAX+1];
				// Register flag properties
  list<ABI_PROPERTY> reg_values[ISA_REGISTER_CLASS_MAX_LIMIT+1][ISA_REGISTER_MAX+1];
				// Register value properties
  const char *reg_names[ISA_REGISTER_CLASS_MAX_LIMIT+1][ISA_REGISTER_MAX+1];
				// Register names
} *ABI;


static list<ABI_PROPERTY> props; // All the properties
static list<ABI> abis;		 // All the ABIs
static ABI current_abi;		 // The current ABI being described
static int prop_count[2 /* is_flag */][2 /* is_reg */] = {0};
				 // Counts of the various kinds of props

//
// Note on the current implementation
//
// Although prop_count is defined as a two dimension table,
// the only case that is useful so far is prop_count[true][true],
// which means that we only consider flagged register specific ABI
// properties. We have inserted an assertion to check that we are
// indeed in that case.
//
//

// Reconfigurability specific stuff: management of
// extensions.
// This information is useful for generation of
// DWARF debug information.
struct ext_description{
   char        *extension_name     ; /* Extension name    */
   char        *reloc_string       ; /* Relocation string */
   int          internal_id        ; /* Extension id      */
} ;

typedef struct ext_reg_description{
   EXT_DESCRIPTION    ext       ; /* Pointer to extension or NULL for the core*/
   ISA_REGISTER_CLASS rc        ; /* Register class.                          */
   unsigned int       dw_base_id; /* Register class dwarf base id.            */
} *EXT_REG_DESCRIPTION;

static list<EXT_DESCRIPTION>     ext_list;       // All the extensions
static list<EXT_REG_DESCRIPTION> ext_reg_list;   // All the registers files attached to extensions.


static const char * const interface[] = {
  "/* ====================================================================",
  " * ====================================================================",
  " *",
  " * Description:",
  " *",
  " *   A description of the ABI properties. The description exports",
  " *   the following:",
  " *",
  " *   typedef (enum) ABI_PROPERTIES_ABI",
  " *",
  " *       An enumeration of the ABIs described. The names have the form:",
  " *",
  " *          ABI_PROPERTIES_ABI_xxx",
  " *",
  " *       where 'xxx' is replaced with the ABI name.",
  " *",
  " *   const ABI_PROPERTIES_ABI ABI_PROPERTIES_ABI_UNDEFINED",
  " *       Useful value guaranteed not to be a valid ABI_PROPERTIES_ABI.",
  " *",
  " *   ABI_PROPERTIES_ABI ABI_PROPERTIES_ABI_Value",
  " *       A variable containing the current ABI value.",
  " *",
  " *   const char *ABI_PROPERTIES_ABI_Name(ABI_PROPERTIES_ABI abi)",
  " *      Returns a name for the given 'abi'.",
  " *",
  " *   void ABI_PROPERTIES_Initialize(void)",
  " *       Initialize for the target ABI specified by ABI_PROPERTIES_ABI_Value.",
  " *",
  " *   const char *ABI_PROPERTY_Reg_Name(",
  " *     ISA_REGISTER_CLASS rc,",
  " *     INT reg",
  " *   )",
  " *       Return the ABI specific name of register 'reg' in class 'rc'.",
  " *",
  " *   BOOL ABI_PROPERTY_Is_xxx(",
  " *     ISA_REGISTER_CLASS rc,",
  " *     INT reg",
  " *   )",
  " *       Return a boolean that indicates if register 'reg' in class",
  " *       'rc' had the property 'xxx'.",
  " *",
  " * ====================================================================",
  " *    Reconfigurability specific stuff",
  " *",
  " *  EXTENSION_static_count",
  " *    Defines the number of extensions build statically with the core",
  " *    (as opposed to extensions build dynamically).",
  " *",
  " *  EXTENSION_count",
  " *    Defines the number of extensions managed at compile time after",
  " *    extension dlls have been loaded.",
  " * ====================================================================",
  " */",
  NULL
};

static void exit_failure( const char *msg );   // Proto

/////////////////////////////////////
void ABI_Properties_Begin(const char * /* name */)
/////////////////////////////////////
//  See interface description.
/////////////////////////////////////
{
#ifdef DYNAMIC_CODE_GEN
   /* For dynamic extensions, we need to initialize tables
      that describe register files in a similar way to what
      is done by dynamic loader. Indeed, following code requires
      routine

        ISA_REGISTER_CLASS_INFO_Name(rcinfo),

      to return correct results.
    */
   ISA_REGISTER_Initialize_Stub();
#endif

   return;
}


/////////////////////////////////////
ABI_PROPERTY Create_Reg_Property(const char *name)
/////////////////////////////////////
//  See interface description.
/////////////////////////////////////
{
  ABI_PROPERTY result = new abi_property;

  result->name = name;
  result->is_reg = true;
  result->is_flag = true;
  result->v = 0;

  props.push_back(result);

  return result;
}


/////////////////////////////////////
void Begin_ABI(const char *name)
/////////////////////////////////////
//  See interface description.
/////////////////////////////////////
{
  // Adding one sanity check.
  if(NULL==name)
   exit_failure("no name for ABI");

  ABI result = new abi;

  result->name = name;
  BZERO(result->reg_names, sizeof(result->reg_names));

  current_abi = result;

  abis.push_back(result);
}

/////////////////////////////////////
void Begin_ABI_dynamic( const char *name )
/////////////////////////////////////
// For dynamic extensions, we define currently
// only one ABI. More precisely, if more than one
// one ABI is supported for the core, we
// assume that for all ABI variants, extension
// registers have the same status and the same
// properties.
/////////////////////////////////////
{
   static int abi_count = 0;
   const char *errmsg = 
   "Current extension model supports only one ABI for extension registers.";

   if(abi_count>0)
      exit_failure(errmsg);
   ++abi_count;            /* Update ABI count    */

   Begin_ABI(name);
   return;
}


/////////////////////////////////////
void Reg_Property(ABI_PROPERTY prop, ISA_REGISTER_CLASS rc, ...)
/////////////////////////////////////
//  See interface description.
/////////////////////////////////////
{
  va_list ap;
  int reg_num;
  bool used = false;

  va_start(ap,rc);
  while ( (reg_num = va_arg(ap,int)) != -1 ) {
    current_abi->reg_flags[rc][reg_num].push_back(prop);
    used = true;
  }
  va_end(ap);

  if (used && prop->v == 0) {
    prop->v = 1ULL << prop_count[true][true];
    ++prop_count[true][true];
  }
}


/////////////////////////////////////
void Reg_Names(ISA_REGISTER_CLASS rc, INT minreg, INT maxreg, const char **names)
/////////////////////////////////////
//  See interface description.
/////////////////////////////////////
{
  int reg_num;
  for (reg_num = minreg; reg_num <= maxreg; ++reg_num) {
    current_abi->reg_names[rc][reg_num] = names[reg_num - minreg];
  }
}


/////////////////////////////////////
static const char *Type_Name(int bits)
/////////////////////////////////////
// Previous versions of this function
// used to return type name as a function
// of the number of bits. For 
// reconfigurability purpose, we now
// choose 32-bit integral type.
/////////////////////////////////////
{
  if(bits>32)
   { fprintf(stderr,
             "###Fatal: type is too wide (%d bits).\n",
             bits);
     exit(EXIT_FAILURE);
   }
   return "mUINT32";
}


/////////////////////////////////////
static const char *Type_Suffix(int bits)
/////////////////////////////////////
// Previous versions of this function
// used to return type suffix as a function
// of the number of bits. We now take
// the wider type.
/////////////////////////////////////
{
  if(bits>32)
   { fprintf(stderr,
             "###Fatal: type is too wide (%d bits).\n",
             bits);
     exit(EXIT_FAILURE);
   }
  return "U";
}


/////////////////////////////////////
static int Type_Size(int bits)
/////////////////////////////////////
// Previous versions of this function
// used to return type size as a function
// of the number of bits. We now take
// the wider type.
/////////////////////////////////////
{
  if(bits>32)
   { fprintf(stderr,"###Fatal: type is too wide (%d bits).\n",
             bits);
     exit(EXIT_FAILURE);
   }
  return 32;
}

/////////////////////////////////////
static unsigned int 
get_low_register_class_index(bool gen_static_code)
/////////////////////////////////////
// Return the lower index in register
// class table for the considered
// register class range.
/////////////////////////////////////
{
   return gen_static_code ? 0 : ISA_REGISTER_CLASS_STATIC_MAX+1;
}

/////////////////////////////////////
static unsigned int
get_upper_register_class_index(bool gen_static_code)
/////////////////////////////////////
// Return the lower index in register
// class table for the considered
// register class range.
/////////////////////////////////////
{
   int idx_max;

   if(gen_static_code)
    { idx_max = ISA_REGISTER_CLASS_STATIC_MAX;
    }
   else
    {
#ifdef DYNAMIC_CODE_GEN
      idx_max = 
        get_low_register_class_index(gen_static_code) +
        dyn_get_ISA_REGISTER_CLASS_tab_sz()           -
        1;
#else
     /* We should nether enter in that case */
     fprintf(stderr,
        "### Error: "
        "dynamic code generation while DYNAMIC_CODE_GEN is not set.\n");
     exit(EXIT_FAILURE);

     idx_max = 0;
#endif
    }

   return idx_max;
}

//////////////////////////////////////
static void exit_failure( const char *msg )
//////////////////////////////////////
// 
//////////////////////////////////////
{
   if(msg)
     fprintf(stderr,"### Error: %s\n",msg);
   else
     fprintf(stderr,"### Can't recover from internal error\n");
   exit(EXIT_FAILURE);
}

/////////////////////////////////////
EXT_DESCRIPTION DW_DEBUG_Extension(const char *extname, const char* reloc_string)
/////////////////////////////////////
// First  parameter: extension name
// Second parameter: relocation string associated
//                   to the extension.
//
// Dynamic case:
// Call to this routine is mandatory for extensions and
// must be done before call to ISA_Create. If
// relocation string is not relevant, parameter can
// be set to NULL.
//
// Static case:
// This routine may be called in the static case
// if core instruction set (as described in TOPs)
// include some extension instructions. In any
// cases, call to this routine is to be done before
// calling ISA_Create.
/////////////////////////////////////
// Note that current implementation assumes that
// EXTENSION_NB_SLOT_MAX and CORE_NB_EXTENSION_SLOT_MAX
// are below 32.
/////////////////////////////////////
{
  va_list ap;

  unsigned int slot_count;
  unsigned int slot;

  EXT_DESCRIPTION my_extension;
  
  if(Is_Dynamic_Code() && ext_list.size()>=1)
    exit_failure("ISA_Extension_Slot routine can only be called once in dynamical context");

  // Allocate extension structure and set internal id.
  my_extension = new ext_description;
  if(NULL==my_extension)
    exit_failure("unable to allocate memory");
  my_extension->internal_id = ext_list.size();

  // Copy extension name in structure.
  if(NULL==extname || '\0'==extname[0])
    exit_failure("uncorrect name for extension");
  my_extension->extension_name = new char[strlen(extname)+1]; 
  if(NULL==my_extension->extension_name)
    exit_failure("unable to allocate memory");
  strcpy(my_extension->extension_name,extname);

  // Copy relocation string
  if(NULL==reloc_string)
   { my_extension->reloc_string   = NULL;
   } 
  else
   { my_extension->reloc_string = new char[strlen(reloc_string)+1]; 
     if(NULL==my_extension->reloc_string)
       exit_failure("unable to allocate memory");
     strcpy(my_extension->reloc_string,reloc_string);
   }

  ext_list.push_back(my_extension);

  return my_extension;
}

/////////////////////////////////////
void DW_DEBUG_Extension_Reg(ISA_REGISTER_CLASS rc, 
                            EXT_DESCRIPTION    ext,
                            unsigned int       dw_base_id)
/////////////////////////////////////
// First  parameter: register class
// Second parameter: pointer on extension description or
//                   NULL if register file belongs to
//                   core architectural ressources.
// Third  parameter: dwarf base identifier for
//                   the considered register class.
//                   Depending on implementation, this
//                   id can be local to compilation unit
//                   and updated at link time.
/////////////////////////////////////
{
   EXT_REG_DESCRIPTION ext_reg = new ext_reg_description;

   const char *dyn_err_msg =
   "cannot describe core registers in dynamic context";

   // In dynamic context, check that we refer register
   // classes.
   if(Is_Dynamic_Code())
    { if (NULL==ext)
       exit_failure(dyn_err_msg);
    }

   ext_reg->rc          = rc;
   ext_reg->ext         = ext;
   ext_reg->dw_base_id  = dw_base_id;

   ext_reg_list.push_back(ext_reg);
   return;
}

/////////////////////////////////////
void ABI_Properties_End(void)
/////////////////////////////////////
//  See interface description.
/////////////////////////////////////
{
  list<ABI_PROPERTY>::iterator        prop_iter;
  list<ABI>::iterator                 abi_iter;
  list<EXT_DESCRIPTION>::iterator     ext_iter;
  list<EXT_REG_DESCRIPTION>::iterator ext_reg_iter;
  const char *tabname;
  unsigned int j;

  FILE* hfile    = NULL ;
  FILE* cfile    = NULL ;
  FILE* efile    = NULL ;

  // Whether we generate code for the core (static) or for an extension.
  bool  gen_static_code = Is_Static_Code();

  // Get extension name or NULL for static code generation.
  char *extname = gen_static_code ? NULL : Get_Extension_Name();

  // File handler initialization.
  char *hfilename     = NULL ;    /* Header file name                  */
  char *cfilename     = NULL ;    /* C file name                       */
  char *efilename     = NULL ;    /* Export file name                  */

  const char * const bname = FNAME_TARG_ABI_PROPERTIES;

  unsigned int rc_class_min_idx;  /* lower index in register class tab */
  unsigned int rc_class_max_idx;  /* upper index in register class tab */

  rc_class_min_idx = get_low_register_class_index   (gen_static_code);
  rc_class_max_idx = get_upper_register_class_index (gen_static_code);

  hfilename = Gen_Build_Filename(bname,extname,gen_util_file_type_hfile);
  hfile     = Gen_Open_File_Handle(hfilename, "w");

  cfilename = Gen_Build_Filename(bname,extname,gen_util_file_type_cfile);
  cfile     = Gen_Open_File_Handle(cfilename, "w");

  if(gen_static_code)
   { efilename = Gen_Build_Filename(bname,extname,gen_util_file_type_efile);
     efile     = Gen_Open_File_Handle(efilename, "w");
   }

  if(gen_static_code)
   { fprintf(cfile,"#include \"%s\"\n\n",hfilename);
   }
  else
   { 
     // For the dynamic version, we need to include
     // the static header file.
     char *staticname = Gen_Build_Filename(bname,
                                           NULL,
                                           gen_util_file_type_hfile);

     fprintf(cfile,"#include \"%s\"\n",staticname);
     fprintf(cfile,"#include \"%s\"\n\n",hfilename);

     Gen_Free_Filename(staticname);
   }

  Emit_Header (hfile, bname , interface,extname);
  fprintf(cfile,"\n\n");
  Emit_C_Header(cfile);              /* "C" directive */

  // Including targ_isa_register.h header file
  char *targ_isa_reg_name = Gen_Build_Filename(FNAME_TARG_ISA_REGISTERS,
                                               NULL,
                                               gen_util_file_type_hfile);
  fprintf(hfile,"#include \"%s\"\n",targ_isa_reg_name);

  //
  // Generate the ABI_PROPERTIES decl
  //
  if(gen_static_code) {
    fprintf(hfile, "\ntypedef struct {\n");

    if (prop_count[true][false] != 0) {
      // Case not supported yet.
      { fprintf(stderr,"### Error : case not supported yet\n");
        exit(EXIT_FAILURE);
      }
      fprintf(hfile, "  %s flags;\n", Type_Name(prop_count[true][false]));
    }

    if (prop_count[true][true] != 0) {
      fprintf(hfile," %s **reg_flags;\n",
                      Type_Name(prop_count[true][true]));
    }

    fprintf(hfile, " const char * const **reg_names;\n");

    fprintf(hfile, "} ABI_PROPERTIES;\n");

    //
    // Generate the property flags decls
    //
    fprintf(hfile, "\n");
    for (prop_iter = props.begin(); prop_iter != props.end(); ++prop_iter) {
      ABI_PROPERTY prop = *prop_iter;
      assert(prop->is_flag);
      if (prop->v != 0) {
	fprintf(hfile, "#define ABI_PROPERTY_%-20s " PRINTF_LONGLONG_FORMAT( "0x", "0*", "x" ) "%s\n",
		prop->name,
		Type_Size(prop_count[true][prop->is_reg]) / 4,
		prop->v,
		Type_Suffix(prop_count[true][prop->is_reg]));
      }
    }   // End for
  }     // End if(gen_static_code)

  //
  // Generate the properties data and the ABI enumeration
  // Note: ABI enumeration is only for the static case.
  //
  int num_abi = 0;

  if(gen_static_code)
    fprintf(hfile, "\ntypedef enum {\n");

  for (abi_iter = abis.begin(); abi_iter != abis.end(); ++abi_iter) {
    unsigned int rc;
    ABI abi = *abi_iter;
    ++num_abi;

    if(gen_static_code)
       fprintf(hfile, "  ABI_PROPERTIES_ABI_%s,\n", abi->name);

    int count = prop_count[true][false];
    if (count != 0) {
      unsigned long long mask = 0;
      for (prop_iter = abi->flags.begin(); prop_iter != abi->flags.end(); ++prop_iter) {
	ABI_PROPERTY prop = *prop_iter;
	mask |= prop->v;
      }
      fprintf(cfile, "    " PRINTF_LONGLONG_FORMAT( "0x", "0*", "x" ) "%s,\n",
		     Type_Size(count) / 4,
		     mask,
		     Type_Suffix(count));
    }

   // Defining a string for alignment of data in output file.
#define ALIGN_STRING "     "

    count = prop_count[true][true];
    {
      for (rc = rc_class_min_idx; rc <= rc_class_max_idx ; ++rc) {
	int reg;
	const ISA_REGISTER_CLASS_INFO *cinfo 
	  = ISA_REGISTER_CLASS_Info((ISA_REGISTER_CLASS)rc);

        // Print out register class name for comment.
        if(gen_static_code)
	  fprintf(cfile, 
                  "\n\n"
                  "/* ISA_REGISTER_CLASS_%s */\n",
                  ISA_REGISTER_CLASS_INFO_Name(cinfo));
         else
	  fprintf(cfile, 
                  "\n\n"
                  "/* ISA_REGISTER_CLASS_%s_%s */\n",
                  extname,
                  ISA_REGISTER_CLASS_INFO_Name(cinfo));

        // The table name is mangled:
        // 1 - with abi name
        // 2 - with register file number.
        fprintf(cfile,"static %s tab_%s_reg_prop_%d [%d] = {\n"
                ALIGN_STRING,
                Type_Name(prop_count[true][true]),
                abi->name,
                rc,
                ISA_REGISTER_MAX+1);

	for (reg = 0; reg <= ISA_REGISTER_MAX; ++reg) {
	  unsigned long long mask = 0;
	  list<ABI_PROPERTY> props = abi->reg_flags[rc][reg];
	  for (prop_iter = props.begin(); prop_iter != props.end(); ++prop_iter) {
	    ABI_PROPERTY prop = *prop_iter;

            if(mask)                  /* If property is not the first one */
              fprintf(cfile," | ");   /* print bitwise '|' operator.      */

            fprintf(cfile,"ABI_PROPERTY_%s ",prop->name);   /* Print prop.*/
	    mask |= prop->v;          /* Updating mask.                   */
	  }

          if(0==mask)                 /* Manage the case with no prop.    */
             fprintf(cfile,"0x0%s",Type_Suffix(count));

          fprintf(cfile,              /* Add comma and start a new line.  */           
                   ",\n"
	           ALIGN_STRING);
	}
        fprintf(cfile,"};\n");        /* End of table.                    */
      }              // End loop for(..,<ISA_REGISTER_CLASS_MAX,..)
    }

    /* Now, consider register file name */
    for (rc = rc_class_min_idx; rc <= rc_class_max_idx; ++rc) {
      int reg;
      const ISA_REGISTER_CLASS_INFO *cinfo 
	= ISA_REGISTER_CLASS_Info((ISA_REGISTER_CLASS)rc);

      // Print out register class name for comment.
      if(gen_static_code)
          fprintf(cfile, 
                  "\n\n"
                  "/* ISA_REGISTER_CLASS_%s */\n",
                  ISA_REGISTER_CLASS_INFO_Name(cinfo));
      else
          fprintf(cfile, 
                  "\n\n"
                  "/* ISA_REGISTER_CLASS_%s_%s */\n",
                  extname,
                  ISA_REGISTER_CLASS_INFO_Name(cinfo));

       // The table name is mangled:
       // 1 - with abi name
       // 2 - with register file number.
      fprintf(cfile,
              "static const char* tab_%s_reg_name_%d [%d] = {\n",
              abi->name,
              rc,
              ISA_REGISTER_MAX+1);

      int cursor = 0;
      for (reg = 0; reg <= ISA_REGISTER_MAX; ++reg) {
	const char *name = abi->reg_names[rc][reg];
	if (name == NULL) name = ISA_REGISTER_CLASS_INFO_Reg_Name(cinfo, reg);
	if (name == NULL) name = "";
	if (cursor >= 80 - (4 + (int)strlen(name))) {
	  fprintf(cfile, "\n");
	  cursor = 0;
	}
	cursor += fprintf(cfile, " \"%s\",", name);
      }
      fprintf(cfile, " };\n");
    }
  }   // ABI iteration

  // Run a second iteration on ABI list
  // We build two tables:
  //  1 - the first one lists all tables dedicated to register file properties,
  //  2 - the second one lists all register name tables.

  fprintf(cfile,"\n\n\n");         // Cosmetic!

  for (abi_iter = abis.begin(); abi_iter != abis.end(); ++abi_iter) 
   { unsigned int rc;
     ABI abi = *abi_iter;

     // Register file properties table
     fprintf(cfile,
             "static %s *tab_%s_reg_prop[%d] = {\n",
             Type_Name(prop_count[true][true]),
             abi->name,
             rc_class_max_idx-rc_class_min_idx+1);

     for(rc=rc_class_min_idx; rc<=rc_class_max_idx; rc++)
       fprintf(cfile,
              ALIGN_STRING "tab_%s_reg_prop_%d,\n",
              abi->name,
              rc);
     fprintf(cfile,"};\n\n");

     // Register file name table
     fprintf(cfile,
             "static const char * const *tab_%s_reg_name[%d] = {\n",
             abi->name,
             rc_class_max_idx-rc_class_min_idx+1);
     for(rc=rc_class_min_idx; rc<=rc_class_max_idx; rc++)
       fprintf(cfile,
               ALIGN_STRING "tab_%s_reg_name_%d,\n",
               abi->name,
               rc);
     fprintf(cfile,"};\n\n");
  }

  // Run the final iteration on ABI list
  tabname = gen_static_code ? "abi_properties_static" :
                              "abi_properties_dynamic";
  fprintf(cfile,"\nstatic const ABI_PROPERTIES %s[] = {\n",tabname);

  for (abi_iter = abis.begin(); abi_iter != abis.end(); ++abi_iter) 
   { ABI abi = *abi_iter;

     fprintf(cfile,
             " { tab_%s_reg_prop , tab_%s_reg_name },\n",
             abi->name,
             abi->name);
   }
  fprintf(cfile, "};\n\n");   // End of table.

  // Ending enumeration in header file.
  if(gen_static_code)
   { fprintf(hfile, "  ABI_PROPERTIES_ABI_UNDEFINED,\n"
                    "  ABI_PROPERTIES_ABI_MAX=%d\n"
                    "} ABI_PROPERTIES_ABI;\n",
		    num_abi-1);

     fprintf(hfile,"\nconst extern ABI_PROPERTIES *ABI_PROPERTIES_base_props;\n");
   }

  if(gen_static_code)
   {fprintf(cfile,
            "\n"
            "const ABI_PROPERTIES *ABI_PROPERTIES_base_props = %s;\n",
            tabname);
   }
  else
   { // Definition of API function.
     const char* fctname = "dyn_get_ABI_PROPERTIES_tab";

     fprintf(cfile,
             "\n"
             "const ABI_PROPERTIES* %s(void)\n"
             "{  return %s;\n"
             "}\n"
             "\n",
             fctname,
             tabname);
     fprintf(hfile,
             "\n"
             "extern const ABI_PROPERTIES* %s(void);\n\n",
             fctname);
   }

  //
  // Generate the abi names
  //
  if(gen_static_code) {

    fprintf(cfile, "\n\nstatic const char * const abi_names[] = {\n");
    for (abi_iter = abis.begin(); abi_iter != abis.end(); ++abi_iter) {
      ABI abi = *abi_iter;
      fprintf(cfile, "  \"%s\",\n", abi->name);
    }
    fprintf(cfile, "  \"UNDEFINED\"\n"
            "};\n");
   }                     // if(gen_static_code)

  //
  // Generate decls for accessing the ABI data and initializiation
  //
  if(gen_static_code) {
  fprintf(hfile, "\nBE_EXPORTED extern ABI_PROPERTIES_ABI ABI_PROPERTIES_ABI_Value;\n");
  fprintf(cfile, "\nABI_PROPERTIES_ABI ABI_PROPERTIES_ABI_Value = ABI_PROPERTIES_ABI_UNDEFINED;\n");
  fprintf(efile, "ABI_PROPERTIES_ABI_Value\n");

  fprintf(hfile, "\nTARGINFO_EXPORTED extern const char *ABI_PROPERTIES_ABI_Name(ABI_PROPERTIES_ABI abi);\n");
  fprintf(cfile, "\nconst char *ABI_PROPERTIES_ABI_Name(ABI_PROPERTIES_ABI abi)\n"
		 "{\n"
		 "  return abi_names[(INT)abi];\n"
		 "}\n");
  fprintf(efile, "ABI_PROPERTIES_ABI_Name\n");

  fprintf(cfile, "\nBE_EXPORTED const ABI_PROPERTIES *ABI_PROPERTIES_target_props"
		 " = &abi_properties_static[ABI_PROPERTIES_ABI_UNDEFINED];\n");
  fprintf(efile, "ABI_PROPERTIES_target_props\n");

  fprintf(hfile, "\nBE_EXPORTED extern void ABI_PROPERTIES_Initialize(void);\n");
  fprintf(cfile, "\nvoid ABI_PROPERTIES_Initialize(void)\n"
		 "{\n"
		 "  ABI_PROPERTIES_target_props = &ABI_PROPERTIES_base_props[(INT)ABI_PROPERTIES_ABI_Value];\n"
		 "}\n");
  fprintf(efile, "ABI_PROPERTIES_Initialize\n");
  }                                                // if(gen_static_code)

  //
  // Generate the property accessors
  //
  if(gen_static_code) {
    fprintf(hfile, "\ninline const char *ABI_PROPERTY_Reg_Name(\n"
		 "  ISA_REGISTER_CLASS rc,\n"
		 "  INT reg)\n"
		 "{\n"
		 "  BE_EXPORTED extern const ABI_PROPERTIES *ABI_PROPERTIES_target_props;\n"
		 "  return ABI_PROPERTIES_target_props->reg_names[rc][reg];\n"
		 "}\n");
  }

  if(gen_static_code) {
  for (prop_iter = props.begin(); prop_iter != props.end(); ++prop_iter) {
    ABI_PROPERTY prop = *prop_iter;
    assert(prop->is_flag);
    fprintf(hfile, prop->v ? "\n" : "\n/*ARGSUSED*/\n");
    if (prop->is_reg) {
      fprintf(hfile, "inline BOOL ABI_PROPERTY_Is_%s(\n"
		     "  ISA_REGISTER_CLASS rc,\n"
		     "  INT reg)\n"
		     "{\n",
		     prop->name);
      if (prop->v == 0) {
	fprintf(hfile, "  return FALSE;\n"
		       "}\n");
      } else {
	fprintf(hfile, "  BE_EXPORTED extern const ABI_PROPERTIES *ABI_PROPERTIES_target_props;\n"
		       "  return (  ABI_PROPERTIES_target_props->reg_flags[rc][reg]\n"
		       "          & ABI_PROPERTY_%s) != 0;\n"
		       "}\n",
		       prop->name);
      }
    } else {
      fprintf(hfile, "inline BOOL ABI_PROPERTY_Is_%s(void)\n"
		     "{\n",
		     prop->name);
      if (prop->v == 0) {
	fprintf(hfile, "  return FALSE;\n"
		       "}\n");
      } else {
	fprintf(hfile, "  BE_EXPORTED extern const ABI_PROPERTIES *ABI_PROPERTIES_target_props;\n"
		       "  return (  ABI_PROPERTIES_target_props->flags\n"
		       "          & ABI_PROPERTY_%s) != 0;\n"
		       "}\n",
		       prop->name);
      }                                  // if(prop->v==0){..}else{..}
    }                                    // if(prop->is_reg){..}else{..}
   }                                     // for loop
  }                                      // gen_static_code


  //////////////////////////////////////////////////////////////////////////
  //
  //      ABI PROPERTIES attributes table and access functions
  //
  // Used in reconfigurability to check Core/Extension targinfo
  // compatibility and potentially allow attribute id remapping 
  // when loading extensions.
  //
  //////////////////////////////////////////////////////////////////////////

  fprintf (hfile,
	   "\n"
	   "/* Description of Used attributes */");
  fprintf (cfile,
	   "\n"
	   "/* ============================================================\n"
	   " *  ABI PROPERTIES attributes description\n"
	   " * ============================================================\n"
	   " */\n");

  // Generate datatype specification
  if (gen_static_code) {
    fprintf (hfile,
	     "typedef struct { const char *name; mUINT32 attribute; } ABI_PROPERTIES_ATTRIBUTE;\n");
  }
  
  // Generate attribute description table
  int nb_attribute_defined=0;
  fprintf (cfile, "static const ABI_PROPERTIES_ATTRIBUTE ABI_PROPERTIES_attribute_tab[] = {\n");
  for (prop_iter = props.begin(); prop_iter != props.end(); ++prop_iter) {
    ABI_PROPERTY prop = *prop_iter;
    if (prop->v > 0) {
      fprintf (cfile, "  { \"%s\", ABI_PROPERTY_%s},\n", prop->name, prop->name);
      nb_attribute_defined++;
    } else if (!gen_static_code) {
      fprintf (cfile, "  { \"%s\", 0ULL},  \t/* Not used in extension */\n", prop->name);
    }
  }
  fprintf (cfile, "};\n\n");
  
  // Generate access to attribute description table
  if (gen_static_code) {
    fprintf (hfile,
	     "\n"
	     "#define ABI_PROPERTIES_ATTRIBUTE_COUNT (%d)\n", nb_attribute_defined);
    fprintf (hfile,
	     "\n"
	     "BE_EXPORTED extern const ABI_PROPERTIES_ATTRIBUTE * ABI_PROPERTIES_get_attribute_table();\n"
	     "\n");

    fprintf (cfile,
	     "BE_EXPORTED const ABI_PROPERTIES_ATTRIBUTE * ABI_PROPERTIES_get_attribute_table() {\n"
	     "  return (ABI_PROPERTIES_attribute_tab);\n"
	     "}\n"
	     "\n");
    fprintf(efile, "ABI_PROPERTIES_get_attribute_table\n");
  }
  else {
    fprintf (hfile,
	     "\n"
	     "TARGINFO_EXT_EXPORTED extern const mUINT32 dyn_get_ABI_PROPERTIES_attribute_tab_sz();\n"
	     "\n"
	     "TARGINFO_EXT_EXPORTED extern const ABI_PROPERTIES_ATTRIBUTE* dyn_get_ABI_PROPERTIES_attribute_tab();\n"
	     "\n");
    fprintf (cfile,
	     "\n"
	     "const mUINT32 dyn_get_ABI_PROPERTIES_attribute_tab_sz() {\n"
	     "  return (%d);\n"
	     "}\n"
	     "\n", props.size());
    fprintf (cfile,
	     "const ABI_PROPERTIES_ATTRIBUTE* dyn_get_ABI_PROPERTIES_attribute_tab() {\n"
	     "  return (ABI_PROPERTIES_attribute_tab);\n"
	     "}\n"
	     "\n");
  }

  //////////////////////////////////////////////////////////////////////////
  //Management of reconfigurability stuff
  //////////////////////////////////////////////////////////////////////////

  // Static part
  if(gen_static_code)
  { fprintf(hfile,
    "\n"
    "\n"
    "/* Description of extensions */\n"
    "typedef struct{\n"
    "  mUINT32 enabled;      /* Whether extension is enabled (1/0) */\n"
    "  const char* extname;  /* Extension name                     */\n"
    "  const char* reloc_string; /* Extension relocation string    */\n"
    "} EXTENSION_DESC;\n"
    "\n"
    "\n"
    "/* Description of register classes */\n"
    "typedef struct{\n"
    "  mINT32  ext_id;       /* Extension index in EXT_DESC tab   */\n"
    "                        /* or -1 for the core.               */\n"
    "  mUINT32 dw_base_id;   /* Register class dwarf compile unit */\n"
    "                        /* base identifier.                  */\n"
    "} EXTENSION_REG_DESC;\n"
    );

   fprintf(hfile,
    "\n"
    "BE_EXPORTED extern EXTENSION_DESC     *EXTENSION_description_tab;\n"
    "BE_EXPORTED extern mUINT32             EXTENSION_count;\n"
    "BE_EXPORTED extern EXTENSION_REG_DESC *EXTENSION_reg_description_tab;\n"
    "\n"
    "#define EXTENSION_static_count %d\n"
    "\n",
    ext_list.size());

    fprintf(efile,
            "EXTENSION_Description_Tab\n"
            "EXTENSION_count\n");

   fprintf(cfile,
           "\n"
           "/* Extension specific stuff - useful for debug purpose */\n"
           "\n"
           "mUINT32 EXTENSION_count = EXTENSION_static_count;\n\n\n");


   /* If core does not support extension, work is easy! */
   if(ext_list.size()==0)
    { fprintf(cfile,
       "BE_EXPORTED EXTENSION_DESC      *EXTENSION_description_tab = (void*)0;\n");
    }
   /* If core support extension, we build a first table */
   /* that lists extension properties, and a second one */
   /* that lists for each register class the extension  */
   /* it belongs to , and its dwarf debug base          */
   /* base identifier.                                  */
   else
    {
      tabname = "EXTENSION_description_static_tab";

      fprintf(cfile,
       "static EXTENSION_DESC %s[%d] = {\n",
       tabname,ext_list.size());

      for(ext_iter=ext_list.begin(),j=0;
          ext_iter!=ext_list.end();
          ++ext_iter,++j)
       { fprintf(cfile,
           "  { 1 /* Ena. */, \"%s\", \"%s\" },\n",
           (*ext_iter)->extension_name,
           (*ext_iter)->reloc_string);
         fprintf(hfile,
           "#define DW_DEBUG_EXTENSION_%-10s %d\n",
           (*ext_iter)->extension_name,
           j);
       }

      fprintf(cfile,"};\n");

      fprintf(cfile,
        "BE_EXPORTED EXTENSION_DESC *EXTENSION_description_tab = %s;\n"
        "\n"
        "\n",
        tabname);
     }

     //Building the second table
     struct ext_reg_description ext_reg_tab[ISA_REGISTER_CLASS_STATIC_COUNT+1];

     // Default initialization for ext_reg_tab
     // dw_base_id is 0 unless otherwise specified in ext_reg_list.
     for(j=0;j<ISA_REGISTER_CLASS_STATIC_COUNT+1;j++)
      { ext_reg_tab[j].ext        = NULL;
        ext_reg_tab[j].rc         = j;
        ext_reg_tab[j].dw_base_id = 0;
      }

     // Transfer information from ext_reg_list into ext_reg_tab
     for(ext_reg_iter =ext_reg_list.begin();
         ext_reg_iter!=ext_reg_list.end();
         ++ext_reg_iter)
      { ISA_REGISTER_CLASS rclass = (*ext_reg_iter)->rc;

        ext_reg_tab[rclass].ext         = (*ext_reg_iter)->ext;
        ext_reg_tab[rclass].dw_base_id  = (*ext_reg_iter)->dw_base_id;
      }

     // Print the second table.
     tabname = "EXTENSION_reg_description_static_tab";
     fprintf(cfile,
      "static EXTENSION_REG_DESC %s[%s] = {\n",
      tabname,
      "ISA_REGISTER_CLASS_STATIC_COUNT+1");

     for(j=0;j<ISA_REGISTER_CLASS_STATIC_COUNT+1;j++)
       { const ISA_REGISTER_CLASS_INFO *cinfo ;

         cinfo = ISA_REGISTER_CLASS_Info((ISA_REGISTER_CLASS)j);

         fprintf(cfile,"   { /* ISA_REGISTER_CLASS_%-15s */ %3d , %d },\n",
                 ISA_REGISTER_CLASS_INFO_Name(cinfo),
                 ext_reg_tab[j].ext==NULL? -1 : ext_reg_tab[j].ext->internal_id,
                 ext_reg_tab[j].dw_base_id);
       }

     fprintf(cfile,"};\n");

     fprintf(cfile,
      "BE_EXPORTED EXTENSION_REG_DESC  *EXTENSION_reg_description_tab = %s;\n\n",
      tabname
      );

  }
  else              // Dynamic part
  {

#ifdef DYNAMIC_CODE_GEN

    /* The case ext_list.size()>1 is not possible */
    if(ext_list.size()!=1)
      exit_failure("dynamic extension has not been described by DW_DEBUG_Extension routine");

    /* Define an interface routine    */
    /* in order to retrieve extension */
    /* slot information.              */
    ext_iter = ext_list.begin(); /*Information is at index 0 by construction*/
    fprintf(cfile,
            "\n"
            "/*\n"
            " * Relocation string associated to the extension.\n"
            " * Can be NULL if extension is not concerned by\n"
            " * this mechanism.\n"
            " */\n"
            "const char* dyn_get_DW_DEBUG_EXT_reloc_string( void )\n"
            "{\n"
            " return \"%s\";\n"
            "}\n",
            NULL == (*ext_iter)->reloc_string ?
            "(const char*)0" : (*ext_iter)->reloc_string); 
     fprintf(hfile,
            "TARGINFO_EXT_EXPORTED extern const char* "
            "dyn_get_DW_DEBUG_EXT_reloc_string( void );\n");

     /* Define an interface routine   */
     /* for extension name.           */
     fprintf(cfile,
             "\n"
             "/*\n" 
             " * Returns extension name. Useful for debugging\n"
             " * purpose\n"
             " */\n"
             "const char* dyn_get_DW_DEBUG_EXT_name( void )\n"
             "{\n"
             " return \"%s\";\n"
             "}\n",
             (*ext_iter)->extension_name /* information is at index 0 */ );
     fprintf(hfile,
             "TARGINFO_EXT_EXPORTED extern const char* "
             "dyn_get_DW_DEBUG_EXT_name( void );\n");

     fprintf(cfile,
             "\n"
             "\n"
             "/* Description of extension registers */\n"
             );
 
     /* Build EXTENSION_reg_description_tab in a similar way to
      * what is done in the static case. There is however some
      * differences
      */
     tabname = "EXTENSION_reg_description_ext_tab";
     fprintf(cfile,
       "static EXTENSION_REG_DESC %s[%d] = {\n",
       tabname,
       dyn_get_ISA_REGISTER_CLASS_tab_sz()
       );

     /* Loop on all extension registers */
     for(j=ISA_REGISTER_CLASS_STATIC_COUNT+1;
         j<ISA_REGISTER_CLASS_STATIC_COUNT+1+dyn_get_ISA_REGISTER_CLASS_tab_sz();
         ++j)
      {
        /* Research record in ext_reg_list
         * O(n2) algo but still OK since we have
         * very few extension register class
         */
        for(ext_reg_iter =ext_reg_list.begin();
            ext_reg_iter!=ext_reg_list.end();
          ++ext_reg_iter)
         { ISA_REGISTER_CLASS rclass = (*ext_reg_iter)->rc;
           if(rclass==j)
              break;
         }

	const ISA_REGISTER_CLASS_INFO *cinfo = 
          ISA_REGISTER_CLASS_Info((ISA_REGISTER_CLASS)j);

        /* Exit if register class j is not described in
         * ext_reg_list.
         */
        if(ext_reg_iter==ext_reg_list.end())
         {fprintf(stderr,
            "### Error: missing information for register class %s\n",
            ISA_REGISTER_CLASS_INFO_Name(cinfo));
          exit_failure("cannot recover");
         }

        /* In the dynamic case, we have only one extension
         * at a time. Exit if identifier is not OK.
         */ 
        if((*ext_reg_iter)->ext->internal_id != 0)
          exit_failure("internal error: unexpected extension identifier");
 
        /* Print table record.
         */
        fprintf(cfile,"   { /* ISA_REGISTER_CLASS_%-15s */ %3d , %d },\n",
                ISA_REGISTER_CLASS_INFO_Name(cinfo),
                (*ext_reg_iter)->ext->internal_id,
                (*ext_reg_iter)->dw_base_id);
      }

     fprintf(cfile,"};\n\n");  /* End of table */

     /* Now, the interface routine
      */
     fprintf(cfile,
         "const EXTENSION_REG_DESC* dyn_get_DW_DEBUG_EXT_reg_tab( void ){\n"
         "   return (%s);\n"
         "}\n"
         "\n",
         tabname);

     fprintf(hfile,
         "extern const EXTENSION_REG_DESC* "
         "dyn_get_DW_DEBUG_EXT_reg_tab( void );\n");

#else
     /* We should nether enter in that case */
     exit_failure("dynamic code generation while "
                  "DYNAMIC_CODE_GEN is not set.");
#endif
  }                                      // if then else (static_code).

  Emit_Footer (hfile);
  Emit_C_Footer(cfile);

  //
  // Closing file handlers.
  //
  Gen_Close_File_Handle(hfile,hfilename);
  Gen_Close_File_Handle(cfile,cfilename);
  if(efile)
    Gen_Close_File_Handle(efile,efilename);

  //
  // Memory deallocation
  //
  Gen_Free_Filename(cfilename);
  Gen_Free_Filename(hfilename);
  if(efilename) 
    Gen_Free_Filename(efilename);
  Gen_Free_Filename(targ_isa_reg_name);

  return;
}
