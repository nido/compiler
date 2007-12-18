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
  " *   A description of the ISA subset hierarchy.  ",
  " *   The original implementation allowed only one active subset at",
  " *   a time. This restriction has beeen removed, as there is a need",
  " *   to be able to represent optional parts of an architecture and",
  " *   not only architecture evolutions.",
  " *   For this the subset has been generalized to represent both:",
  " *   - growing set of opcodes along architecture evolution,",
  " *   - optional parts of an architecture that can be activated",
  " *     or not,",
  " *   - in addition note that reconfigurability is added to this,",
  " *     and an extension may also declare a set of additional subsets and ",
  " *     activate some.",
  " *",
  " *   In all cases the abstraction is kept very simple, i.e.:",
  " *   - an ISA_SUBSET is simply a subset of the available isa opcodes,",
  " *   - for a given core+options+extensions correspond a list of ISA_SUBSET",
  " *     that must be available and activated.",
  " *   - a subset can be available (in the ISA) but not activated for the",
  " *     given configuration.",
  " *",
  " *   For cores where only one subset was active, the list simply contains",
  " *   one SUBSET.",
  " *   For cores with options, the list should contain the minimal core SUBSET",
  " *   plus one subset for each option.",
  " *   For extensions, in addition to the above, each extension provides a new",
  " *   set of subsets. One can activate some or all of the extension subsets.",
  " *",
  " *   Resources that depend on subsets are for instance register files. We",
  " *   declare in the register description the activating subsets, i.e. the set",
  " *   of subsets that will activate the register file.",
  " *   These resources are active as soon as the one of the activating subset is",
  " *   active.",
  " *",
  " *   The description exports the following:",
  " *",
  " *   UINT32 ISA_SUBSET",
  " *       A unique id for the different subsets.",
  " *",
  " *   ISA_SUBSET_count",
  " *	 The total count of available subsets (static ISA + extensions ISAs). ",
  " *       Note that an available subset is not necessarily activated. To find",
  " *       the activated subsets use the ISA_SUBSET_List variable and the",
  " *       corresponding ISA_SUBSET_LIST functions. ",
  " *",
  " *   ISA_SUBSET_static_count",
  " *	 The total number of available subsets in the static ISA only.",
  " *",
  " *   ISA_SUBSET_MIN",
  " *   ISA_SUBSET_MAX",
  " *   ISA_SUBSET_UNDEFINED",
  " *       ISA_SUBSET_MIN and ISA_SUBSET_MAX can be used as iterators over the available",
  " *       subsets (static ISA + extensions ISA).",
  " *       ISA_SUBSET_UNDEFINED, is unique and unmatched ISA_SUBSET id.",
  " *",
  " *   ISA_SUBSET_LIST *ISA_SUBSET_List",
  " *       A globally available variable containing the current list of active",
  " *       subsets. It is invalid to generate an opcode that is not a member of",
  " *       this subset list.",
  " *",
  " *   const char* ISA_SUBSET_Name(ISA_SUBSET subset)",
  " *       Returns a name suitable for printing.",
  " *",
  " *   ISA_SUBSET ISA_SUBSET_From_Name(const char *name)",
  " *       Returns the subset id from the given subset name.",
  " *",
  " *   int ISA_SUBSET_Member( ISA_SUBSET subset, TOP opcode )",
  " *       Returns true iif the given <opcode> a member of the given <subset>.",
  " *",
  " *   int ISA_SUBSET_LIST_Member( ISA_SUBSET_LIST *list, TOP opcode )",
  " *       Returns true iif the given <opcode> is a member of the given active subset <list>.",
  " *",
  " *   void ISA_SUBSET_LIST_Add( ISA_SUBSET_LIST *list, ISA_SUBSET subset )",
  " *       Adds the subset to the <list> of active subsets.",
  " *",
  " *   INT ISA_SUBSET_LIST_Count( ISA_SUBSET_LIST *list )",
  " *   ISA_SUBSET *ISA_SUBSET_LIST_First( ISA_SUBSET_LIST *list)",
  " *   ISA_SUBSET *ISA_SUBSET_LIST_Past( ISA_SUBSET_LIST *list)",
  " *       Respectively count, pointer to first and pointer past last (for iteration) subset",
  " *       of the given active subset <list>",
  " *",
  " *   UINT32  ISA_SUBSET_LIST_Mask( ISA_SUBSET_LIST *list )",
  " *       Helper function to create a mask (1 bit for each subset) of axtive subsets from ",
  " *       the given active subsets <list>.",
  " *",
  " *",
  " *   The support for extensions requires the additional interface below which is",
  " *   available only from the static part of the compiler and not generated in the extension DLLs:",
  " *",
  " *   ISA_SUBSET EXTENSION_ISA_SUBSET_New(const char *subsetname)",
  " *       Declare a new available subset. Note that this is not sufficient to activate the ",
  " *       subset. Invoke ISA_SUBSET_LIST_Add(ISA_SUBSET_List, new_subset) to actually activate",
  " *       the subset once declared.",
  " *",
  " *   void EXTENSION_ISA_SUBSET_Set_Members(ISA_SUBSET subset, int base_top, int top_count,",
  " *					   const unsigned char *ext_top_members)",
  " *	 Declare the opcode members that are included into the new subset.",
  " *	 The <base_top> must be the base opcode for the extension, the <top_count>",
  " *	 must be the count of opcodes in the extension ISA. The <ext_top_members>",
  " *	 is an array of boolean values, one for each extension ISA opcode id (indexed by the",
  " *	 opcode index in the extension). Each boolean is 1 iif the <subset> contains the",
  " *	 corresponding extension opcode.",
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

  if(gen_static_code) {
    fprintf(cfile,"#include <string.h>\n");
    fprintf(cfile,"#include \"%s.h\"\n", bname);
  } else {
    char *static_name;
    
    static_name = Gen_Build_Filename(bname,NULL,gen_util_file_type_hfile);
    
    fprintf(cfile,"#include \"%s\"\n",static_name);
    fprintf(cfile,"#include \"%s\"\n\n",hfilename);
    
    Gen_Free_Filename(static_name);
  }

  Emit_Header (hfile, bname, interface,extname);
  fprintf(hfile,"#include \"topcode.h\"\n");

  if (gen_static_code) { 
    fprintf(hfile,"\ntypedef UINT32 ISA_SUBSET;\n");
    fprintf(hfile,"\nBE_EXPORTED extern INT ISA_SUBSET_count;\n");
    fprintf(efile,"ISA_SUBSET_count\n");
    fprintf(hfile,"\n#define ISA_SUBSET_MIN ((ISA_SUBSET)0)\n");
    fprintf(hfile,"#define ISA_SUBSET_MAX (ISA_SUBSET_MIN+ISA_SUBSET_count-1)\n");
    fprintf(hfile,"#define ISA_SUBSET_UNDEFINED	(ISA_SUBSET_MIN+ISA_SUBSET_count)\n");
    fprintf(hfile,"#define ISA_SUBSET_COUNT_MAX (32) /* At most sizeof(UINT32) for masks of subsets. */\n");
  }

  if (gen_static_code) {
    fprintf(hfile,"\n");
  }
  for ( k = 0, isi = subsets.begin(); isi != subsets.end(); ++isi, ++k ) {
    ISA_SUBSET subset = *isi;
    if (gen_static_code) {
      fprintf(hfile,"#define ISA_SUBSET_%s ISA_SUBSET_MIN+%d\n", 
	      subset->name, k);
    }
  }
  if (gen_static_code) {
    fprintf(hfile, "#define ISA_SUBSET_static_count (%d)\n", isa_subset_count);
    fprintf(hfile, "\n");
  }

  if (gen_static_code) {
    fprintf(cfile,"\nINT ISA_SUBSET_count = ISA_SUBSET_static_count;\n");
  } else {
    fprintf(cfile,"\n#define ISA_SUBSET_COUNT (%d)\n", isa_subset_count);
  }
  fprintf(cfile,"\nstatic const char* isa_subset_names[%s] = {\n",
          gen_static_code ? "ISA_SUBSET_COUNT_MAX":"ISA_SUBSET_COUNT");

  for ( isi = subsets.begin(); isi != subsets.end(); ++isi ) {
    ISA_SUBSET subset = *isi;
    fprintf(cfile,"  \"%s\",", subset->name);
  }
  fprintf(cfile,"\n};\n");

  if(gen_static_code) {
    fprintf(cfile,"#define ISA_SUBSET_UNDEFINED_Name \"UNDEFINED\"\n\n");
  }

  if(gen_static_code) {
    fprintf(hfile,"BE_EXPORTED extern const char* ISA_SUBSET_Name( ISA_SUBSET subset );\n");
    fprintf(efile,"ISA_SUBSET_Name\n");
    fprintf(cfile, "const char*\n");
    fprintf(cfile, "ISA_SUBSET_Name( ISA_SUBSET subset ) {\n");
    fprintf(cfile, "  if (subset == ISA_SUBSET_UNDEFINED) return ISA_SUBSET_UNDEFINED_Name;\n");
    fprintf(cfile, "  return isa_subset_names[(INT)subset];\n");
    fprintf(cfile, "}\n");
    fprintf(cfile, "\n");

    fprintf(hfile,"BE_EXPORTED extern ISA_SUBSET ISA_SUBSET_From_Name( const char *subset_name );\n");
    fprintf(efile,"ISA_SUBSET_From_Name\n");
    fprintf(cfile, "ISA_SUBSET\n");
    fprintf(cfile, "ISA_SUBSET_From_Name(const char *subsetname)\n");
    fprintf(cfile, "{\n");
    fprintf(cfile, "  ISA_SUBSET i;\n");
    fprintf(cfile, "  for (i = ISA_SUBSET_MIN; i <= ISA_SUBSET_MAX; i++) {\n");
    fprintf(cfile, "    if (strcmp(isa_subset_names[i], subsetname) == 0)\n");
    fprintf(cfile, "      return i;\n");
    fprintf(cfile, "  }\n");
    fprintf(cfile, "  return ISA_SUBSET_UNDEFINED;\n");
    fprintf(cfile, "}\n");
    fprintf(cfile, "\n");
  }    // gen_static_code
  

  for ( isi = subsets.begin(),k=0; isi != subsets.end(); ++isi,++k ) {
    ISA_SUBSET subset = *isi;

    fprintf(cfile,
            "static const unsigned char isa_subset_%d_opcode_table[%d] =\n",
            k, TOP_count_limit);

    fprintf(cfile,"  { /* %s */\n", subset->name);
    for ( unsigned int i = 0; i < TOP_count_limit; ++i ) {
      fprintf(cfile,
              "   %d,  /* %-30s */\n",subset->members[i]==true ? 1 : 0,
              TOP_Name((TOP)i));
    }
    fprintf(cfile,"};\n\n");
  }

  /* 
   * Now generate the global table.
   * Number of entries in table depends on
   * the number of subsets. 
   */
  if(gen_static_code) {
    fprintf(cfile,"\n\n");
    fprintf(cfile, "typedef struct \n");
    fprintf(cfile, "{\n");
    fprintf(cfile, "  UINT32 base_opcode;\n");
    fprintf(cfile, "  UINT32 count;\n");
    fprintf(cfile, "  const unsigned char *members;  \n");
    fprintf(cfile, "} ISA_SUBSET_MEMBERS;\n");
    fprintf(cfile, "\n");
    fprintf(cfile, "static ISA_SUBSET_MEMBERS empty_members = { 0, 0, (void *)0 };\n");
    fprintf(cfile, "\n");
    fprintf(cfile, "static ISA_SUBSET_MEMBERS isa_subset_members[ISA_SUBSET_COUNT_MAX] = {\n");
    for(k=0; k < isa_subset_count; k++) { 
      fprintf(cfile, "   { %d, %d, isa_subset_%d_opcode_table} ,\n", 0, TOP_count_limit, k);
    }
    fprintf(cfile,"};\n\n\n");
  }
  else {
    fprintf(cfile,
	    "\n\n"
	    "static const unsigned char *ISA_SUBSET_dyn_opcode_table[%d] = {\n",
	    isa_subset_count);
    for(k=0;k<isa_subset_count;k++) {
      fprintf(cfile,"isa_subset_%d_opcode_table,\n",k);
    }
    fprintf(cfile,"};\n\n\n");
  }
  
  if(gen_static_code) {
    fprintf(hfile,"BE_EXPORTED extern INT ISA_SUBSET_Member( ISA_SUBSET subset,\n"
	    "                              TOP opcode );\n");
    fprintf(efile,"ISA_SUBSET_Member\n");
    fprintf(cfile, "int\n");
    fprintf(cfile, "ISA_SUBSET_Member(ISA_SUBSET subset, TOP opcode)\n");
    fprintf(cfile, "{\n");
    fprintf(cfile, "  ISA_SUBSET_MEMBERS *members;\n");
    fprintf(cfile, "  if (subset == ISA_SUBSET_UNDEFINED) return 0;\n");
    fprintf(cfile, "  members = &isa_subset_members[subset-ISA_SUBSET_MIN];\n");
    fprintf(cfile, "  if (opcode < members->base_opcode || opcode >= members->base_opcode + members->count) return 0;\n");
    fprintf(cfile, "  return members->members[opcode-members->base_opcode];\n");
    fprintf(cfile, "}\n");
    fprintf(cfile, "\n");

    fprintf(hfile,"\nstruct ISA_SUBSET_LIST;\n");
    fprintf(cfile, "typedef struct ISA_SUBSET_LIST {\n");
    fprintf(cfile, "  INT count;\n");
    fprintf(cfile, "  ISA_SUBSET *first;\n");
    fprintf(cfile, "} ISA_SUBSET_LIST;\n");
    fprintf(cfile, "\n");

    fprintf(hfile,"BE_EXPORTED extern struct ISA_SUBSET_LIST *ISA_SUBSET_List;\n");
    fprintf(efile,"ISA_SUBSET_List\n");
    fprintf(cfile, "static ISA_SUBSET ISA_SUBSET_List__[ISA_SUBSET_COUNT_MAX];\n");
    fprintf(cfile, "static ISA_SUBSET_LIST ISA_SUBSET_List_ = { 0, &ISA_SUBSET_List__[0] };\n");
    fprintf(cfile, "ISA_SUBSET_LIST *ISA_SUBSET_List = &ISA_SUBSET_List_;\n");
    fprintf(cfile, "\n");

    fprintf(hfile,"BE_EXPORTED extern void ISA_SUBSET_LIST_Add(struct ISA_SUBSET_LIST *,  ISA_SUBSET subset);\n");
    fprintf(efile,"ISA_SUBSET_LIST_Add\n");
    fprintf(cfile, "void\n");
    fprintf(cfile, "ISA_SUBSET_LIST_Add(ISA_SUBSET_LIST *list, ISA_SUBSET subset)\n");
    fprintf(cfile, "{ \n");
    fprintf(cfile, "  list->first[list->count++] = subset;\n");
    fprintf(cfile, "}\n");

    fprintf(hfile,"BE_EXPORTED extern INT ISA_SUBSET_LIST_Contains(struct ISA_SUBSET_LIST *, ISA_SUBSET subset);\n");
    fprintf(efile,"ISA_SUBSET_LIST_Contains\n");
    fprintf(cfile, "INT\n");
    fprintf(cfile, "ISA_SUBSET_LIST_Contains(ISA_SUBSET_LIST *list, ISA_SUBSET subset)\n");
    fprintf(cfile, "{ \n");
    fprintf(cfile, "  ISA_SUBSET *it;\n");
    fprintf(cfile, "  for (it = ISA_SUBSET_LIST_First(list); it != ISA_SUBSET_LIST_Past(list); it++) {\n");
    fprintf(cfile, "    if (*it == subset) return TRUE;\n");
    fprintf(cfile, "  }\n");
    fprintf(cfile, "  return FALSE;\n");
    fprintf(cfile, "}\n");
    fprintf(cfile, "\n");

    fprintf(hfile,"BE_EXPORTED extern INT ISA_SUBSET_LIST_Member(struct ISA_SUBSET_LIST *, TOP opcode);\n");
    fprintf(efile,"ISA_SUBSET_LIST_Member\n");
    fprintf(cfile, "INT\n");
    fprintf(cfile, "ISA_SUBSET_LIST_Member(ISA_SUBSET_LIST *list,TOP opcode)\n");
    fprintf(cfile, "{ \n");
    fprintf(cfile, "  ISA_SUBSET *it;\n");
    fprintf(cfile, "  for (it = ISA_SUBSET_LIST_First(list); it != ISA_SUBSET_LIST_Past(list); it++) {\n");
    fprintf(cfile, "    if (ISA_SUBSET_Member(*it, opcode)) return TRUE;\n");
    fprintf(cfile, "  }\n");
    fprintf(cfile, "  return FALSE;\n");
    fprintf(cfile, "}\n");
    fprintf(cfile, "\n");

    fprintf(hfile,"BE_EXPORTED extern ISA_SUBSET *ISA_SUBSET_LIST_First(struct ISA_SUBSET_LIST *);\n");
    fprintf(efile,"*ISA_SUBSET_LIST_First\n");
    fprintf(cfile, "ISA_SUBSET *\n");
    fprintf(cfile, "ISA_SUBSET_LIST_First(ISA_SUBSET_LIST *list)\n");
    fprintf(cfile, "{\n");
    fprintf(cfile, "  return list->first;\n");
    fprintf(cfile, "}\n");
    fprintf(cfile, "\n");

    fprintf(hfile,"BE_EXPORTED extern ISA_SUBSET *ISA_SUBSET_LIST_Past(struct ISA_SUBSET_LIST *);\n");
    fprintf(efile,"*ISA_SUBSET_LIST_Past\n");
    fprintf(cfile, "ISA_SUBSET *\n");
    fprintf(cfile, "ISA_SUBSET_LIST_Past(ISA_SUBSET_LIST *list)\n");
    fprintf(cfile, "{\n");
    fprintf(cfile, "  return list->first + list->count;\n");
    fprintf(cfile, "}\n");
    fprintf(cfile, "\n");

    fprintf(hfile,"BE_EXPORTED extern INT ISA_SUBSET_LIST_Count(struct ISA_SUBSET_LIST *);\n");
    fprintf(efile,"ISA_SUBSET_LIST_Count\n");
    fprintf(cfile, "INT\n");
    fprintf(cfile, "ISA_SUBSET_LIST_Count(ISA_SUBSET_LIST *list)\n");
    fprintf(cfile, "{\n");
    fprintf(cfile, "  return list->count;\n");
    fprintf(cfile, "}\n");
    fprintf(cfile, "\n");

    fprintf(hfile,"BE_EXPORTED extern UINT32 ISA_SUBSET_LIST_Mask(struct ISA_SUBSET_LIST *);\n");
    fprintf(efile,"ISA_SUBSET_LIST_Mask\n");
    fprintf(cfile, "UINT32\n");
    fprintf(cfile, "ISA_SUBSET_LIST_Mask(ISA_SUBSET_LIST *list)\n");
    fprintf(cfile, "{ \n");
    fprintf(cfile, "  UINT32 mask = 0;\n");
    fprintf(cfile, "  ISA_SUBSET *it;\n");
    fprintf(cfile, "  for (it = ISA_SUBSET_LIST_First(list); it != ISA_SUBSET_LIST_Past(list); it++) {\n");
    fprintf(cfile, "    mask |= (UINT32)1 << *it;\n");
    fprintf(cfile, "  }\n");
    fprintf(cfile, "  return mask;\n");
    fprintf(cfile, "}\n");
    fprintf(cfile, "\n");

    // Interface for reconfigurability support.
    fprintf(hfile,"\nBE_EXPORTED extern ISA_SUBSET EXTENSION_ISA_SUBSET_New(const char *subsetname);\n");
    fprintf(efile,"EXTENSION_ISA_SUBSET_New\n");
    fprintf(cfile, "ISA_SUBSET\n");
    fprintf(cfile, "EXTENSION_ISA_SUBSET_New(const char *subsetname)\n");
    fprintf(cfile, "{\n");
    fprintf(cfile, "  isa_subset_names[ISA_SUBSET_count] = subsetname;\n");
    fprintf(cfile, "  isa_subset_members[ISA_SUBSET_count] = empty_members;\n");
    fprintf(cfile, "  return ISA_SUBSET_count++;\n");
    fprintf(cfile, "}\n");
    fprintf(cfile, "\n");

    fprintf(hfile,"BE_EXPORTED extern void EXTENSION_ISA_SUBSET_Set_Members(ISA_SUBSET subset, int base_top, int top_count, const unsigned char *ext_top_members);\n");
    fprintf(efile,"EXTENSION_ISA_SUBSET_Set_Members\n");
    fprintf(cfile, "void \n");
    fprintf(cfile, "EXTENSION_ISA_SUBSET_Set_Members(ISA_SUBSET subset, int base_top, int top_count, const unsigned char *ext_top_members)\n");
    fprintf(cfile, "{\n");
    fprintf(cfile, "  ISA_SUBSET_MEMBERS *members = &isa_subset_members[subset];\n");
    fprintf(cfile, "  members->base_opcode = base_top;\n");
    fprintf(cfile, "  members->count = top_count;\n");
    fprintf(cfile, "  members->members = ext_top_members;\n");
    fprintf(cfile, "}\n");
    fprintf(cfile, "\n");

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
