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

/*
 * This file has beem modified by STMicroelectronics
 *
 */


//  isa_subset_gen.cxx
/////////////////////////////////////
//
//  Generate an interface for a description of the ISA subset hierarchy.
//
/////////////////////////////////////
//


#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
// [HK]
#if __GNUC__ >= 3 || defined(_MSC_VER)
#include <list>
#include <vector>
using std::vector;
#else
#include <list.h>
#include <vector.h>
#endif //  __GNUC__ >= 3 || defined(_MSC_VER)

#ifdef DYNAMIC_CODE_GEN
#include "dyn_isa_topcode.h"  // Specific adaptation
#else
#include "topcode.h"
#endif

#include "isa_subset_gen.h"
#include "gen_util.h"


struct isa_subset {
  const char* name;         // Name given for documentation and debugging
  int index;                // value in enum
  ISA_SUBSET superset;      // Parent in subset tree, NULL for roots

  vector<bool> members;     // true if opcode belongs to the subset.

  bool is_dyn_extensible;   // Whether a subset can be extended dynamically
                            // This property can only be declared for subtree
                            // with NULL root.
};

static int isa_subset_count = 0;    // How many subsets?
// [HK]
#if __GNUC__ >=3 || defined(_MSC_VER)
static std::list<ISA_SUBSET> subsets;    // All the subsets
#else
static list<ISA_SUBSET> subsets;    // All the subsets
#endif // __GNUC__ >=3 || defined(_MSC_VER)

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



static const char * const interface[] = {
  "/* ====================================================================",
  " * ====================================================================",
  " *",
  " * Description:",
  " *",
  " *   A description of the ISA subset hierarchy.  The description",
  " *   exports the following:",
  " *",
  " *   typedef (enum) ISA_SUBSET",
  " *       An enumberated type of the different subsets.",
  " *",
  " *   const ISA_SUBSET ISA_SUBSET_UNDEFINED",
  " *       Useful value guaranteed not to be a valid ISA_SUBSET.",
  " *",
  " *   extern ISA_SUBSET ISA_SUBSET_Value",
  " *       A variable containing the current subset value.",
  " *",
  " *   const char* ISA_SUBSET_Name( ISA_SUBSET subset )",
  " *       Returns a name suitable for printing.",
  " *",
  " *   int ISA_SUBSET_Member( ISA_SUBSET subset, TOP opcode )",
  " *       Is the given <opcode> a member of the given <subset>?",
  " *",
  " * ====================================================================",
  " * ====================================================================",
  " */",
  NULL
};


/////////////////////////////////////
void ISA_Subset_Begin( const char* /* name */ )
/////////////////////////////////////
//  See interface description.
/////////////////////////////////////
{
   return;
}

/////////////////////////////////////
ISA_SUBSET ISA_Subset_Create( ISA_SUBSET parent, const char* name )
/////////////////////////////////////
//  See interface description.
/////////////////////////////////////
{
  ISA_SUBSET result = new isa_subset;

  result->name = name;
  result->index = isa_subset_count++;
  result->superset = parent;
  result->members = vector<bool>(TOP_count_limit,false);
  result->is_dyn_extensible = false;

  subsets.push_front(result);

  return result;
}

/////////////////////////////////////
void ISA_Subset_Is_Extensible( ISA_SUBSET subset )
/////////////////////////////////////
// Set a subset as being able to handle extensions.
/////////////////////////////////////
{
   if(subset->superset!=NULL)
    { fprintf(stderr,
              " ### Extensibility is currently a global property that cannot applied to sub-subset.\n");
      exit(EXIT_FAILURE);
    }

   subset->is_dyn_extensible=true;
   return;
}

/////////////////////////////////////
void Instruction_Group( ISA_SUBSET subset, ... )
/////////////////////////////////////
//  See interface description.
/////////////////////////////////////
{
  va_list ap;
  TOP opcode;

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

  TOP stop = Is_Static_Code() ? TOP_UNDEFINED : static_cast<TOP>(-1);

  va_start(ap,subset);
  while ( (opcode = static_cast<TOP>(va_arg(ap,int))) != stop ) {
    ISA_SUBSET ss;

    for ( ss = subset; ss != NULL; ss = ss->superset )
        ss->members[(mUINT32)opcode] = true;

  }                          // End while.
  va_end(ap);
}

/////////////////////////////////////
void ISA_Subset_End(void)
/////////////////////////////////////
//  See interface description.
/////////////////////////////////////
{
// [HK]
#if __GNUC__ >=3 || defined(_MSC_VER)
  std::list<ISA_SUBSET>::iterator isi;
#else
  list<ISA_SUBSET>::iterator isi;
#endif // __GNUC__ >=3 || defined(_MSC_VER)

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

  const char * const bname = FNAME_TARG_ISA_SUBSET;

  int k ;

  // Beginning of code.
  // Opening files.
  hfilename = Gen_Build_Filename(bname,extname,gen_util_file_type_hfile);
  hfile     = Gen_Open_File_Handle(hfilename, "w");

  cfilename = Gen_Build_Filename(bname,extname,gen_util_file_type_cfile);
  cfile     = Gen_Open_File_Handle(cfilename, "w");

  if(gen_static_code)
   { efilename = Gen_Build_Filename(bname,extname,gen_util_file_type_efile);
     efile     = Gen_Open_File_Handle(efilename, "w");
   }

  if(gen_static_code)
   {fprintf(cfile,"#include \"%s.h\"\n", bname);
   }
  else
   { char *static_name;

     static_name = Gen_Build_Filename(bname,NULL,gen_util_file_type_hfile);

     fprintf(cfile,"#include \"%s\"\n",static_name);
     fprintf(cfile,"#include \"%s\"\n\n",hfilename);

     Gen_Free_Filename(static_name);
   }

  Emit_Header (hfile, bname, interface,extname);
  fprintf(hfile,"#include \"topcode.h\"\n");

  if(gen_static_code)
   { fprintf(hfile,"\ntypedef enum {\n");
   }

  // For dynamic code, check that we have one and only one
  // subset.
  if(!gen_static_code)
   { if(isa_subset_count!=1)
      { fprintf(stderr,
        " ### Dynamic extensions: can only define one subset currently.\n");
        exit(EXIT_FAILURE);
      }
   }

  fprintf(cfile,"\nstatic const char* const isa_subset_names[%d] = {\n",
          gen_static_code ? isa_subset_count+1:isa_subset_count);

  for ( isi = subsets.begin(); isi != subsets.end(); ++isi ) {
    ISA_SUBSET subset = *isi;
    if(gen_static_code)
     { fprintf(hfile,"  ISA_SUBSET_%s,\n", subset->name);
     }
    fprintf(cfile,"  \"%s\",", subset->name);
  }

  // For dynamic extensions, we assume that the
  // number of subsets is not modified. More
  // precisely, dynamic instructions extend
  // the instruction subsets that have been
  // marked as accepting dynamic extensions. At load time,
  // dynamic TOPs and static TOPs belonging to the
  // extensible subsets are merged in a coherent
  // representation.
  if(gen_static_code)
   { fprintf(hfile,"  ISA_SUBSET_UNDEFINED,\n"
		"  ISA_SUBSET_MIN=ISA_SUBSET_%s,\n"
		"  ISA_SUBSET_MAX=ISA_SUBSET_%s\n"
		"} ISA_SUBSET;\n",
		(*subsets.begin())->name,
		(*subsets.rbegin())->name);
   }

  if(gen_static_code)
    fprintf(cfile,"  \"UNDEFINED\"\n"
                  "};\n\n");
  else
    fprintf(cfile,"\n};\n\n");

  if(gen_static_code)
   { fprintf(cfile,
             "\nstatic const int isa_subset_is_extensible [ %d ] = {\n",
             isa_subset_count+1);

     const char * const str_template = "  %d, /* %-25s */\n";

     for ( isi = subsets.begin(); isi != subsets.end(); ++isi )
        fprintf(cfile,str_template,
                (*isi)->is_dyn_extensible? 1 : 0, 
                (*isi)->name);

     fprintf(cfile,str_template,0,"UNDEFINED"); 
     fprintf(cfile,"};\n\n");
   }

  if(gen_static_code) {
    fprintf(hfile,"BE_EXPORTED extern ISA_SUBSET ISA_SUBSET_Value;\n\n");
    fprintf(efile,"ISA_SUBSET_Value\n");
    fprintf(cfile,"ISA_SUBSET ISA_SUBSET_Value = ISA_SUBSET_UNDEFINED;\n\n");

    fprintf(hfile,"BE_EXPORTED extern const char* ISA_SUBSET_Name( ISA_SUBSET subset );\n");
    fprintf(efile,"ISA_SUBSET_Name\n");
    fprintf(cfile,"const char* ISA_SUBSET_Name( ISA_SUBSET subset ) {\n");
    fprintf(cfile,"  return isa_subset_names[(INT)subset];\n");
    fprintf(cfile,"}\n\n");

    fprintf(hfile,"BE_EXPORTED extern int ISA_SUBSET_Is_Extensible( ISA_SUBSET subset );\n");
    fprintf(efile,"ISA_SUBSET_Is_Extensible");
    fprintf(cfile,"int ISA_SUBSET_Is_Extensible( ISA_SUBSET subset ) {\n"
                  "  return  isa_subset_is_extensible[(INT)subset];\n"
                  "}\n\n");
   }    // gen_static_code


  for ( isi = subsets.begin(),k=0; isi != subsets.end(); ++isi,++k ) {
    ISA_SUBSET subset = *isi;

    fprintf(cfile,
            "static const unsigned char isa_subset_%d_opcode_table[%d] =\n",
            k,TOP_count_limit);

    fprintf(cfile,"  { /* %s */\n", subset->name);
    for ( unsigned int i = 0; i < TOP_count_limit; ++i ) {
      fprintf(cfile,
              "   %d,  /* %-30s */\n",subset->members[i]==true ? 1 : 0,
              TOP_Name((TOP)i));
    }
    fprintf(cfile,"};\n\n");
  }

  if(gen_static_code)
   { fprintf(cfile,
             "static const unsigned char isa_subset_%s_opcode_table[%d] = {\n",
             "undefined",TOP_count_limit);
     for(unsigned int i = 0 ; i<TOP_count_limit; ++i)
       fprintf(cfile,"   %d,\n",0);
     fprintf(cfile,"};\n");
   }

  /* 
   * Now generate the global table.
   * Number of entries in table depends on
   * the number of subsets. For static code generation.
   * don't forget entry for last "undefined" subset!
   */
  if(gen_static_code) {
  fprintf(cfile,
          "\n\n"
          "const unsigned char *ISA_SUBSET_opcode_table[%d] = {\n",
          isa_subset_count+1);
  }
  else {
  fprintf(cfile,
          "\n\n"
          "static const unsigned char *ISA_SUBSET_dyn_opcode_table[%d] = {\n",
          isa_subset_count);
  }
  for(k=0;k<isa_subset_count;k++)
   { fprintf(cfile,"isa_subset_%d_opcode_table,\n",k);
   }
  if(gen_static_code)                                 /* Add undefined entry */
   fprintf(cfile,"isa_subset_%s_opcode_table,\n","undefined");
  fprintf(cfile,"};\n\n\n");                          /* End of global table */

  if(gen_static_code) {
  fprintf(hfile, "\nBE_EXPORTED extern const unsigned char *ISA_SUBSET_opcode_table[%d];\n\n",
	  isa_subset_count+1);
  fprintf(efile,"ISA_SUBSET_opcode_table\n");
  fprintf(hfile,"BE_EXPORTED extern INT ISA_SUBSET_Member( ISA_SUBSET subset,\n"
                "                              TOP opcode );\n");
  fprintf(efile,"ISA_SUBSET_Member\n");
  fprintf(cfile,
	  "int ISA_SUBSET_Member( ISA_SUBSET subset, TOP opcode )\n"
	  "{ return ISA_SUBSET_opcode_table[(mUINT32)subset][(mUINT32)opcode];\n"
          "}\n");
  }
  else {
  const char * const fct1_name = "dyn_get_ISA_SUBSET_tab";
  const char * const fct2_name = "dyn_get_ISA_SUBSET_tab_sz";
  const char * const fct3_name = "dyn_get_ISA_SUBSET_op_tab";

  /* Declaration of routines in header files */
  fprintf(hfile,
          "\n"
          "extern const char* const *  %s ( void );\n"
          "extern const mUINT32 %s ( void );\n"
          "extern unsigned const char** %s ( void );\n"
          "\n",
          fct1_name,
          fct2_name,
          fct3_name);

  fprintf(cfile,                      /* Printing routine 1 */
          "\n"
          "const char * const * %s ( void )\n"
          "{ return %s;\n"
          "}\n"
          "\n",
          fct1_name,
          "isa_subset_names"
          );

   fprintf(cfile,                      /* Printing routine 2 */
           "const mUINT32 %s ( void )\n"
           "{ return (const mUINT32) %d;\n"
           "}\n"
           "\n",
           fct2_name,
           isa_subset_count
           );

   fprintf(cfile,                      /* Printing routine 3 */
           "unsigned const char** %s ( void )\n"
           "{ return %s;\n"
           "}\n"
           "\n",
           fct3_name,
           "ISA_SUBSET_dyn_opcode_table"
           ); 
  }

  Emit_Footer (hfile);

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
