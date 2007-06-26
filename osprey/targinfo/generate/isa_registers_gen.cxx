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
 * This file has been modified by STMicroelectronics.
 */


//  isa_registers_gen.cxx
/////////////////////////////////////
//
//  Description:
//
//      Generate a description of the ISA registers.
//
/////////////////////////////////////
//


#include <stddef.h>
#include <stdlib.h>
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
#include "targ_isa_subset.h"
#include "isa_registers_gen.h"

// #ifdef DYNAMIC_CODE_GEN
// namespace DYNA {
// #include "dyn_isa_registers.h"
// };
// #endif

/*
 * For cores that are not extensible, all #define
 * imported from "isa_ext_limits.h" are set to 0. 
 * As a result, constants 'extension_max_xxx' defined
 * below are useless in following algorithms.
 *
 */
#include "isa_ext_limits.h"

typedef struct isa_register_set {
  int isa_mask;
  int min_regnum;
  int max_regnum;
  const char *def_name_format;
  const char **names;
} *ISA_REGISTER_SET;

typedef struct isa_register_subclass {
  const char *name;
  ISA_REGISTER_CLASS rclass;
  int count;
  const int *members;
  const char **names;
  bool  is_canonical;
} *ISA_REGISTER_SUBCLASS;

struct isa_register_class {
  const char *name;
  int bit_size;
  bool is_ptr;
  bool can_store;
  bool multiple_save;
  int min_reg;
  int max_reg;
  list<ISA_REGISTER_SET> regsets;
  list<ISA_REGISTER_SUBCLASS> subclasses;
};

static list<ISA_REGISTER_CLASS> rclasses;      // All the classes
static list<ISA_REGISTER_SUBCLASS> subclasses; // All the sub classes

static const int extension_max_registers  = EXTENSION_NB_REGISTER_MAX != 0 ? 
                                            EXTENSION_NB_REGISTER_MAX-1 : 0;

static const int extension_max_rclasses   = EXTENSION_NB_REGISTER_CLASS_MAX;
static const int extension_max_subclasses = EXTENSION_NB_REGISTER_SUBCLASS_MAX;

static const char * const interface[] = {
  "/* ====================================================================",
  " * ====================================================================",
  " *",
  " * Description:",
  " *",
  " *   A description of the ISA registers. The description exports",
  " *   the following:",
  " *",
  " *   typedef (enum) ISA_REGISTER_CLASS",
  " *       An enumeration of the register classes.",
  " *",
  " *   typedef mISA_REGISTER_CLASS",
  " *       The most compact (integral) representation that can hold",
  " *       all values of ISA_REGISTER_CLASS",
  " *",
  " *   typedef (struct) ISA_REGISTER_CLASS_INFO",
  " *       Describes a particular register class. The contents are private.",
  " *",
  " *   const INT ISA_REGISTER_CLASS_UNDEFINED",
  " *       A special register class that is out-of-range of valid",
  " *       register clases.",
  " *",
  " *   const INT ISA_REGISTER_CLASS_MIN",
  " *       The first register class. The range of register classes",
  " *       is ISA_REGISTER_CLASS_MIN..ISA_REGISTER_CLASS_MAX",
  " *	    (this range excludes ISA_REGISTER_CLASS_UNDEFINED)."
  " * ",
  " *   const INT ISA_REGISTER_CLASS_MAX",
  " *       The last register class. The range of register classes",
  " *       is ISA_REGISTER_CLASS_MIN..ISA_REGISTER_CLASS_MAX",
  " *	    (this range excludes ISA_REGISTER_CLASS_UNDEFINED)."
  " * ",
  " *   const INT ISA_REGISTER_CLASS_COUNT",
  " *       The number of register classes. The range of register classes",
  " *       is ISA_REGISTER_CLASS_MIN..ISA_REGISTER_CLASS_MAX",
  " *	    (this range excludes ISA_REGISTER_CLASS_UNDEFINED)."
  " * ",
  " *   (macro) FOR_ALL_ISA_REGISTER_CLASS(cl)",
  " *       Iterate over all the register class values using the",
  " *       ISA_REGISTER_CLASS variable <cl>.",
  " *",
  " *   (macro) FOR_ALL_ISA_REGISTER_CLASS_IN_REVERSE(cl)",
  " *       Iterate over all the register class values in reverse order using",
  " *       the ISA_REGISTER_CLASS variable <cl>.",
  " *",
  " *   const INT ISA_REGISTER_MAX",
  " *       The maximum (highest) register number of all classes.",
  " *       NOTE: the lowest number register is implicitly 0.",
  " *",
  " *   typedef (enum) ISA_REGISTER_SUBCLASS",
  " *       An enumeration of the register subclasses.",
  " *",
  " *   typedef mISA_REGISTER_SUBCLASS",
  " *       The most compact (integral) representation that can hold",
  " *       all values of ISA_REGISTER_SUBCLASS",
  " *",
  " *   typedef (struct) ISA_REGISTER_SUBCLASS_INFO",
  " *       Describes a particular register subclass. The contents are private.",
  " *",
  " *   const INT ISA_REGISTER_SUBCLASS_UNDEFINED",
  " *       A special register subclass that is out-of-range of valid",
  " *       register subclases.",
  " *",
  " *   const INT ISA_REGISTER_SUBCLASS_MIN",
  " *       The first register subclass. The range of register subclasses",
  " *       is ISA_REGISTER_SUBCLASS_MIN..ISA_REGISTER_SUBCLASS_MAX",
  " * ",
  " *   const INT ISA_REGISTER_SUBCLASS_MAX",
  " *       The last register subclass. The range of register subclasses",
  " *       is ISA_REGISTER_SUBCLASS_MIN..ISA_REGISTER_SUBCLASS_MAX",
  " * ",
  " *   const INT ISA_REGISTER_SUBCLASS_COUNT",
  " *       The number of register subclasses.",
  " * ",
  " *   (macro) FOR_ALL_ISA_REGISTER_SUBCLASS(sc)",
  " *       Iterate over all the register subclass values using the",
  " *       the ISA_REGISTER_SUBCLASS variable <sc>.",
  " *",
  " *   const ISA_REGISTER_CLASS_INFO *ISA_REGISTER_CLASS_Info(",
  " *     ISA_REGISTER_CLASS rc",
  " *   )",
  " *       Return a pointer to the register class info for class 'rc'.",
  " *",
  " *   INT ISA_REGISTER_CLASS_INFO_First_Reg(",
  " *     const ISA_REGISTER_CLASS_INFO *info",
  " *   )",
  " *       Get the first (lowest numbered) register for the class",
  " *       described by 'info'.",
  " *",
  " *   INT ISA_REGISTER_CLASS_INFO_Last_Reg(",
  " *     const ISA_REGISTER_CLASS_INFO *info",
  " *   )",
  " *       Get the last (highest numbered) register for the class",
  " *       described by 'info'.",
  " *",
  " *   INT ISA_REGISTER_CLASS_INFO_Bit_Size(",
  " *     const ISA_REGISTER_CLASS_INFO *info",
  " *   )",
  " *       Get the size, in bits, of the register in the class",
  " *       described by 'info'.",
  " *",
  " *   BOOL ISA_REGISTER_CLASS_INFO_Can_Store(",
  " *     const ISA_REGISTER_CLASS_INFO *info",
  " *   )",
  " *       Return a flag that indicates if the registers in the class",
  " *       described by 'info' can be stored to memory, i.e. there",
  " *       is a store instruction for the registers in the class.",
  " *",
  " *   BOOL ISA_REGISTER_CLASS_INFO_Multiple_Save(",
  " *     const ISA_REGISTER_CLASS_INFO *info",
  " *   )",
  " *       Return a flag that indicates if the registers in the class",
  " *       described by 'info' can be saved and restore to memory in",
  " *       multiples, i.e. as a group.",
  " *",
  " *   const char *ISA_REGISTER_CLASS_INFO_Name(",
  " *     const ISA_REGISTER_CLASS_INFO *info",
  " *   )",
  " *       Return the name of the class described by 'info'.",
  " *",
  " *   const char *ISA_REGISTER_CLASS_INFO_Reg_Name(",
  " *     const ISA_REGISTER_CLASS_INFO *info,",
  " *     INT reg_index",
  " *   )",
  " *       Return the name of the 'reg_index'th register in the",
  " *       class described by 'info'. NOTE: reg_index==0 corresponds",
  " *       to the first register of the class.",
  " *",
  " *   const ISA_REGISTER_SUBCLASS_INFO *ISA_REGISTER_SUBCLASS_Info(",
  " *     ISA_REGISTER_SUBCLASS sc",
  " *   )",
  " *",
  " *       Return a pointer to the register subclass info for the",
  " *       subclass 'sc'.",
  " *",
  " *   const char *ISA_REGISTER_SUBCLASS_INFO_Name(",
  " *     const ISA_REGISTER_SUBCLASS_INFO *info",
  " *   )",
  " *",
  " *       Return the name of the subclass described by 'info'.",
  " *",
  " *   ISA_REGISTER_CLASS ISA_REGISTER_SUBCLASS_INFO_Class(",
  " *     const ISA_REGISTER_SUBCLASS_INFO *info",
  " *   )",
  " *",
  " *       Return the base register class for the subclass described",
  " *       by 'info'.",
  " *",
  " *   INT ISA_REGISTER_SUBCLASS_INFO_Count(",
  " *     const ISA_REGISTER_SUBCLASS_INFO *info",
  " *   )",
  " *",
  " *       Return the number of registers in the subclass described",
  " *       by 'info'.",
  " *",
  " *   UINT ISA_REGISTER_SUBCLASS_INFO_Member(",
  " *     const ISA_REGISTER_SUBCLASS_INFO *info,",
  " *     INT n",
  " *   )",
  " *",
  " *       Return the 'n'th member (register) of the subclass described",
  " *       by 'info'. The order of the registers returned is arbitrary.",
  " *",
  " *   const char *ISA_REGISTER_SUBCLASS_INFO_Reg_Name(",
  " *     const ISA_REGISTER_SUBCLASS_INFO *info,",
  " *     INT index",
  " *   )",
  " *",
  " *       Return the 'n'th member's register name of the subclass",
  " *       described by 'info'. If the member does not have a subclass",
  " *       specific name, NULL is returned.",
  " *",
  " *   void ISA_REGISTER_Initialize(void)",
  " *       Initialize the register package for use with the ISA specified",
  " *       by ISA_SUBSET_Value.",
  " *",
  " * ====================================================================",
  " * ====================================================================",
  " */",
  NULL
};

static bool gen_static_code = true;          // Whether we generate code for an
static bool gen_dyn_code    = false;         // extension or for the core.

static char *extname        = NULL;          // Extension name (NULL if no ext).

/////////////////////////////////////
void ISA_Registers_Begin( const char* /* name */ )
/////////////////////////////////////
//  See interface description.
/////////////////////////////////////
{
   gen_static_code = Is_Static_Code();
   gen_dyn_code    =!gen_static_code;

   if(gen_dyn_code)
     extname = Get_Extension_Name();

   return;
}


/////////////////////////////////////
ISA_REGISTER_CLASS ISA_Register_Class_Create(
  const char *name,
  int bit_size,
  bool is_ptr,
  bool can_store,
  bool multiple_save
)
/////////////////////////////////////
//  See interface description.
/////////////////////////////////////
{
  ISA_REGISTER_CLASS result = new isa_register_class;
  rclasses.push_back(result);
  result->name = name;
  result->bit_size = bit_size;

  // Arthur: check that only one class is a ptr class
  if (is_ptr) {
    list <ISA_REGISTER_CLASS>::iterator rc_iter;
    for (rc_iter = rclasses.begin(); rc_iter != rclasses.end(); ++rc_iter) {
      ISA_REGISTER_CLASS rclass = *rc_iter;
      if (rclass->is_ptr) {
       fprintf(stderr, "### Error: attempt to specify two ptr rclasses.\n");
       fprintf(stderr, "### Error: current %s, previous %s\n", 
		name, rclass->name);
       exit(EXIT_FAILURE);
      }
    }
  }

  result->is_ptr = is_ptr;
  result->can_store = can_store;
  result->multiple_save = multiple_save;
  return result;
}


/////////////////////////////////////
void ISA_Register_Set(
  ISA_REGISTER_CLASS rclass,
  int min_regnum,
  int max_regnum,
  const char *def_name_format,
  const char **names,
  int isa_mask
)
/////////////////////////////////////
//  See interface description.
/////////////////////////////////////
{
  ISA_REGISTER_SET regset = new isa_register_set;

  regset->min_regnum = min_regnum;
  regset->max_regnum = max_regnum;
  regset->def_name_format = def_name_format;
  regset->names = names;
  regset->isa_mask = isa_mask;
  rclass->regsets.push_back(regset);
}


/////////////////////////////////////
void ISA_Register_Subclass_Create(
  const char *name,
  ISA_REGISTER_CLASS rclass,
  int count,
  const int *members,
  const char **names,
  bool is_canonical /* optional argument */
)
/////////////////////////////////////
//  See interface description.
/////////////////////////////////////
{
  ISA_REGISTER_SUBCLASS result = new isa_register_subclass;
  subclasses.push_back(result);
  result->name         = name;
  result->rclass       = rclass;
  result->count        = count;
  result->members      = members;
  result->names        = names;
  result->is_canonical = is_canonical;
  rclass->subclasses.push_back(result);
}


/////////////////////////////////////
void ISA_Registers_End(void)
/////////////////////////////////////
//  See interface description.
/////////////////////////////////////
{
  list <ISA_REGISTER_CLASS>::iterator rc_iter;
  list <ISA_REGISTER_SUBCLASS>::iterator rsc_iter;
  int i;
#if 0
  bool ptr_specified = false;
#endif

  // Compute max_reg, max_reg & min_reg for each class,
  int max_reg = 0;
  for (rc_iter = rclasses.begin(); rc_iter != rclasses.end(); ++rc_iter) {
    ISA_REGISTER_CLASS rclass = *rc_iter;
    int class_max = 0;
    int class_min = 0;
    list<ISA_REGISTER_SET>::iterator reg_iter;

    /* We adopt here a conservative attitude.
     * Indeed, we want to be sure that there is
     * one and only one register set associated to
     * a given register class. Having more than one
     * register set is possible but not supported
     * yet.
     */
    if(1!=rclass->regsets.size()) {
      if(rclass->regsets.empty())
        fprintf(stderr,
          "### Fatal: No register set associated to register class %s.\n",
           rclass->name);
      else
        fprintf(stderr,
"### Error: currently we only support one register file by register class.\n"
"### Register class %s has %d register sets and does not respect this rule.\n",
                rclass->name,
                rclass->regsets.size()); 

      exit(EXIT_FAILURE);
     }

    for (reg_iter = rclass->regsets.begin();
	 reg_iter != rclass->regsets.end();
	 ++reg_iter
    ) {
      ISA_REGISTER_SET regset = *reg_iter;
      int this_max = regset->max_regnum;
      if (this_max > class_max) class_max = this_max;

      int this_min = regset->min_regnum;
      if (this_min > class_min) class_min = this_min;
    }
    rclass->max_reg = class_max;
    rclass->min_reg = class_min;
    if (class_max > max_reg) max_reg = class_max;

#if 0
    if (rclass->is_ptr) ptr_specified = true;
#endif
  }

#if 0
  // Arthur: check that the ptr class is specified
  if (!ptr_specified) {
    fprintf(stderr, "### Error: ptr rclass has not been specified.\n");
    exit(EXIT_FAILURE);
  }
#endif

  if(gen_dyn_code)  {

  // Compute max_reg, max_reg & min_reg for each class,
  max_reg = 0;
  for (; rc_iter != rclasses.end(); ++rc_iter) {
    ISA_REGISTER_CLASS rclass = *rc_iter;
    int class_max = 0;
    int class_min = 0;
    list<ISA_REGISTER_SET>::iterator reg_iter;
    for (reg_iter = rclass->regsets.begin();
	 reg_iter != rclass->regsets.end();
	 ++reg_iter
    ) {
      ISA_REGISTER_SET regset = *reg_iter;
      int this_max = regset->max_regnum;
      if (this_max > class_max) class_max = this_max;

      int this_min = regset->min_regnum;
      if (this_min > class_min) class_min = this_min;
    }
    rclass->max_reg = class_max;
    rclass->min_reg = class_min;
    if (class_max > max_reg) max_reg = class_max;
    if (max_reg > extension_max_registers) {
      fprintf(stderr, "### Error: rclass %s contains too much registers.\n",
              rclass->name);
      exit(EXIT_FAILURE);
    }

#if 0
    if (rclass->is_ptr) ptr_specified = true;
#endif
   }
  }                       // Code for dynamic extensions.

  /* Opening files */
  char *hfilename                 = NULL ;
  char *cfilename                 = NULL ;
  char *efilename                 = NULL ;
  char *targ_isa_subset_hfilename = NULL ;

  FILE *hfile    = NULL ;
  FILE *cfile    = NULL ;
  FILE *efile    = NULL ;

  const char * const bname = FNAME_TARG_ISA_REGISTERS ;      /* base name */

  hfilename = Gen_Build_Filename(bname,extname,gen_util_file_type_hfile);
  hfile     = Gen_Open_File_Handle(hfilename, "w");

  cfilename = Gen_Build_Filename(bname,extname,gen_util_file_type_cfile);
  cfile     = Gen_Open_File_Handle(cfilename, "w");

  if(gen_static_code)
   { efilename = Gen_Build_Filename(bname,extname,gen_util_file_type_efile);
     efile     = Gen_Open_File_Handle(efilename, "w");
   }

  targ_isa_subset_hfilename = Gen_Build_Filename(FNAME_TARG_ISA_SUBSET,NULL,
                                                 gen_util_file_type_hfile);

  /* Now, we're all set to write files. */
  Emit_Header (hfile,bname,interface,extname);

  if(gen_dyn_code) 
  { char *dyn_hfilename = Gen_Build_Filename(FNAME_ISA_REGISTERS,
                                             NULL,
                                             gen_util_file_type_dyn_hfile);

    fprintf(hfile,"#include \"%s\"\n",targ_isa_subset_hfilename);
    fprintf(cfile,"#include \"%s\"\n",dyn_hfilename);
    Gen_Free_Filename(dyn_hfilename);
  }                        // Dynamic code
  else
  { fprintf(cfile,"#include \"%s\"\n",hfilename);
    fprintf(hfile,"#include \"%s\"\n",targ_isa_subset_hfilename);
  }                        // Static code

  Emit_C_Header(cfile);


  // [TTh] max register might need to be resized for extension
  // register classes
  if (max_reg < extension_max_registers) {
	max_reg = extension_max_registers;
  }

  if(gen_static_code) {
   fprintf(hfile, "\n"
                  "#define ISA_EXTENSION_REGISTER_CLASS_MAX (%d)\n",
                  extension_max_rclasses);
   fprintf(hfile, "\n"
                  "#define ISA_EXTENSION_REGISTER_SUBCLASS_MAX (%d)\n",
                  extension_max_subclasses);
   fprintf(hfile, "\n"
                  "#define ISA_REGISTER_MAX (%d)\n", max_reg);
  }

  /**************************************************
   * classes:
   */

  if(gen_static_code)
   { fprintf(hfile, "\nenum {\n");
     fprintf(hfile, "  ISA_REGISTER_CLASS_UNDEFINED,\n");

     for (rc_iter = rclasses.begin(); rc_iter != rclasses.end(); ++rc_iter) {
       ISA_REGISTER_CLASS rclass = *rc_iter;
       fprintf(hfile, "  ISA_REGISTER_CLASS_%s,\n", rclass->name);
     }

     fprintf(hfile, "  ISA_REGISTER_CLASS_MIN = ISA_REGISTER_CLASS_%s,\n",
  	  rclasses.front()->name);
     fprintf(hfile, "  ISA_REGISTER_CLASS_STATIC_MAX = ISA_REGISTER_CLASS_%s,\n",
	  rclasses.back()->name);
     fprintf(hfile, "  ISA_REGISTER_CLASS_STATIC_COUNT = "
		 "ISA_REGISTER_CLASS_STATIC_MAX - ISA_REGISTER_CLASS_MIN + 1,\n");
     fprintf(hfile, "  ISA_REGISTER_CLASS_MAX_LIMIT = "
                    "ISA_REGISTER_CLASS_STATIC_MAX + ISA_EXTENSION_REGISTER_CLASS_MAX + 1\n");
     fprintf(hfile, "};\n");
     fprintf(hfile, "typedef mUINT32 ISA_REGISTER_CLASS; \n");

     fprintf(hfile, "\nextern mUINT32 ISA_REGISTER_CLASS_MAX;\n");
     fprintf(cfile, "\nBE_EXPORTED mUINT32 ISA_REGISTER_CLASS_MAX = ISA_REGISTER_CLASS_STATIC_MAX;\n");
     fprintf(hfile, "\nextern mUINT32 ISA_REGISTER_CLASS_COUNT;\n");
     fprintf(cfile, "\nBE_EXPORTED mUINT32 ISA_REGISTER_CLASS_COUNT = ISA_REGISTER_CLASS_STATIC_COUNT;\n");
   }
  else                            /* Code for dynamic extensions */
   {
      fprintf(hfile, "\n");
      for (rc_iter = rclasses.begin(),i=0; 
           rc_iter != rclasses.end(); ++rc_iter,++i) {
             ISA_REGISTER_CLASS rclass = *rc_iter;
             fprintf(
               hfile, 
               "#define ISA_REGISTER_CLASS_LOCAL_%s_%-9s %d\n"
               "#define ISA_REGISTER_CLASS_%s_%-15s "
               "(ISA_REGISTER_CLASS_LOCAL_%s_%s+ISA_REGISTER_CLASS_STATIC_MAX+1)\n",
                extname,rclass->name,i,extname,rclass->name,extname,rclass->name);
          }

      fprintf(hfile, 
              "\n"
              "#define ISA_REGISTER_CLASS_EXTENSION_%s_COUNT %d\n\n",
              extname,rclasses.size());
  }

  if(gen_static_code) {
  fprintf(hfile, "\ntypedef mUINT8 mISA_REGISTER_CLASS;\n");

  fprintf(hfile, "\n#define FOR_ALL_ISA_REGISTER_CLASS(cl) \\\n"
		 "\tfor (cl = ISA_REGISTER_CLASS_MIN; \\\n"
		 "\t     cl <= ISA_REGISTER_CLASS_MAX; \\\n"
		 "\t     cl = (ISA_REGISTER_CLASS)(cl + 1))\n");

  fprintf(hfile, "\n#define FOR_ALL_ISA_REGISTER_CLASS_IN_REVERSE(cl) \\\n"
		 "\tfor (cl = ISA_REGISTER_CLASS_MAX; \\\n"
		 "\t     cl >= ISA_REGISTER_CLASS_MIN; \\\n"
		 "\t     cl = (ISA_REGISTER_CLASS)(cl - 1))\n");

  fprintf(hfile, "\ntypedef struct {\n"
		 "  mUINT8 isa_mask;\n"
		 "  mUINT8 min_regnum;\n"
		 "  mUINT8 max_regnum;\n"
		 "  mUINT16 bit_size;\n"
		 "  mBOOL is_ptr;\n"
		 "  mBOOL can_store;\n"
		 "  mBOOL multiple_save;\n"
		 "  const char *name;\n"
		 "  const char *reg_name[ISA_REGISTER_MAX+1];\n"
  		 "} ISA_REGISTER_CLASS_INFO;\n");
  }

  /*
   * Building ISA_REGISTER_CLASS_info_static and 
   * ISA_REGISTER_CLASS_info_dynamic (for dynamic extensions)
   */
  const char* tabname =gen_static_code? "ISA_REGISTER_CLASS_info_static" : 
                                        "ISA_REGISTER_CLASS_info_dynamic";

  fprintf(cfile, "\nstatic const ISA_REGISTER_CLASS_INFO %s[] = {\n",tabname);
  if(gen_static_code)              /* Managed UNDEFINED entry */
    fprintf(cfile, 
            "  { 0x%02x, %3d, %3d, %2d, %1d, %1d, %1d, \"%s\", { 0 } },\n",
            0, 0, -1, 0, 0, 0, 0, "UNDEFINED");
  for (rc_iter = rclasses.begin(); rc_iter != rclasses.end(); ++rc_iter) {
    ISA_REGISTER_CLASS rclass = *rc_iter;
    list<ISA_REGISTER_SET>::iterator reg_iter;
    for (reg_iter = rclass->regsets.begin();
	 reg_iter != rclass->regsets.end();
	 ++reg_iter
    ) {
      ISA_REGISTER_SET regset = *reg_iter;
      fprintf(cfile, "  { 0x%02x, %3d, %3d, %2d, %1d, %1d, %1d, \"%s\",",
	      regset->isa_mask,
	      regset->min_regnum,
	      regset->max_regnum,
	      rclass->bit_size,
	      rclass->is_ptr,
	      rclass->can_store,
	      rclass->multiple_save,
	      rclass->name);

      int len = fprintf(cfile, "\n    { ");
      for (i = regset->min_regnum; i <= regset->max_regnum; ++i) {
	if (len > 70) len = fprintf(cfile, "\n      ");
        len += fprintf(cfile, "\"");
	if (regset->names && regset->names[i - regset->min_regnum]) {
	  len += fputs(regset->names[i - regset->min_regnum], cfile);
	} else {
	  len += fprintf(cfile, regset->def_name_format, i);
	}
	len += fprintf(cfile, "\"%s", i != regset->max_regnum ? ", " : "");
      }
      fprintf(cfile, " } },\n");
    }
  }
  fprintf(cfile, "};\n");

  if(gen_static_code)
   { fprintf(cfile, "\nBE_EXPORTED const ISA_REGISTER_CLASS_INFO * "
                    " ISA_REGISTER_CLASS_info = %s;\n\n",tabname);
     fprintf(hfile, "\nBE_EXPORTED extern const ISA_REGISTER_CLASS_INFO *"
                    " ISA_REGISTER_CLASS_info;\n\n");
     fprintf(efile, "ISA_REGISTER_CLASS_info\n");
   }
  else
   { fprintf(
      cfile,
      "\n"
      "DLL_EXPORTED const ISA_REGISTER_CLASS_INFO *dyn_get_ISA_REGISTER_CLASS_tab (void)\n"
      "{  return %s;\n"
      "}\n"
      "DLL_EXPORTED const mUINT32 dyn_get_ISA_REGISTER_CLASS_tab_sz (void)\n"
      "{  return  ISA_REGISTER_CLASS_EXTENSION_%s_COUNT;\n"
      "}\n\n",
      tabname,extname);

     fprintf(
       hfile,
       "#ifndef DLL_EXPORTED\n"
       "#define DLL_EXPORTED\n"
       "#endif\n\n");
     fprintf(
       hfile,
       "DLL_EXPORTED extern const ISA_REGISTER_CLASS_INFO *dyn_get_ISA_REGISTER_CLASS_tab ( void );\n"
       "DLL_EXPORTED extern const mUINT32 dyn_get_ISA_REGISTER_CLASS_tab_sz ( void );\n");
   }

  /* Building index table - In the static case, table has one additional */
  /* entry, since we must take into account UNDEFINED item.              */
  tabname   = gen_static_code ? "ISA_REGISTER_CLASS_info_index_static" :
                                "ISA_REGISTER_CLASS_info_index_dynamic";
  int index = gen_static_code ?  1 : 0;

  fprintf(cfile,"\nstatic mUINT8 %s[] = {\n",tabname);
  if(gen_static_code)
   fprintf(cfile, "  %d,  /* ISA_REGISTER_CLASS_%s */\n", 
           0,"UNDEFINED");

  for (rc_iter = rclasses.begin(); rc_iter != rclasses.end(); ++rc_iter) {
    ISA_REGISTER_CLASS rclass = *rc_iter;
    list<ISA_REGISTER_SET>::iterator reg_iter;

    if(gen_static_code)
      fprintf(cfile, "  %d,  /* ISA_REGISTER_CLASS_%s */\n", 
              index, rclass->name);
    else
      fprintf(cfile, "  %d,  /* ISA_REGISTER_CLASS_%s_%s */\n", 
              index,extname,rclass->name);

    for (reg_iter = rclass->regsets.begin();
	 reg_iter != rclass->regsets.end();
	 ++reg_iter) 
           ++index;
  }
  fprintf(cfile, "};\n");      /* End of index table */

  if(gen_static_code)
   { fprintf(cfile,
             "\n"
             "BE_EXPORTED mUINT8 *ISA_REGISTER_CLASS_info_index = %s;\n",
             tabname);
     fprintf(hfile, 
             "\n"
             "BE_EXPORTED extern mUINT8 *ISA_REGISTER_CLASS_info_index;\n\n"
             "#define ISA_REGISTER_CLASS_INDEX_DYNAMIC_NEXT (%d)\n", 
             index);
     fprintf(efile, 
             "ISA_REGISTER_CLASS_info_index\n");
   }
  else
   { fprintf(cfile,
             "const mUINT8 *dyn_get_ISA_REGISTER_CLASS_index_tab ( void )\n"
             "{   return %s;\n"
             "}\n",
             tabname);
     fprintf(hfile,
             "extern const mUINT8 *dyn_get_ISA_REGISTER_CLASS_index_tab ( void );\n");
   }

  /**************************************************
   * subclasses:
   */

  if(gen_static_code)
  { fprintf(hfile, "\nenum {\n");
    fprintf(hfile, "  ISA_REGISTER_SUBCLASS_UNDEFINED,\n");
    for (rsc_iter = subclasses.begin(); rsc_iter != subclasses.end(); ++rsc_iter) {
      ISA_REGISTER_SUBCLASS subclass = *rsc_iter;
      fprintf(hfile, "  ISA_REGISTER_SUBCLASS_%s,\n", subclass->name);
    }
    if (subclasses.empty()) {
      fprintf(hfile, "  ISA_REGISTER_SUBCLASS_MIN = 1,\n");
      fprintf(hfile, "  ISA_REGISTER_SUBCLASS_STATIC_MAX = 0,\n");
    } else {
      fprintf(hfile, "  ISA_REGISTER_SUBCLASS_MIN = ISA_REGISTER_SUBCLASS_%s,\n",
              subclasses.front()->name);
      fprintf(hfile, "  ISA_REGISTER_SUBCLASS_STATIC_MAX = ISA_REGISTER_SUBCLASS_%s,\n",
              subclasses.back()->name);
    }

   fprintf(hfile,
           "  ISA_REGISTER_SUBCLASS_STATIC_COUNT = "
           "  ISA_REGISTER_SUBCLASS_STATIC_MAX - ISA_REGISTER_SUBCLASS_MIN + 1,\n"
           "  ISA_REGISTER_SUBCLASS_MAX_LIMIT = "
           "  ISA_REGISTER_SUBCLASS_STATIC_MAX + ISA_EXTENSION_REGISTER_SUBCLASS_MAX + 1\n"
           "};\n"
           "typedef mUINT32 ISA_REGISTER_SUBCLASS;\n\n"
           "extern mUINT32 ISA_REGISTER_SUBCLASS_MAX;\n");

   fprintf(cfile, 
           "\n"
           "BE_EXPORTED mUINT32 ISA_REGISTER_SUBCLASS_MAX = ISA_REGISTER_SUBCLASS_STATIC_MAX;\n");
   fprintf(hfile, 
           "\n"
           "extern mUINT32 ISA_REGISTER_SUBCLASS_COUNT;\n");
   fprintf(cfile,
           "\n"
           "BE_EXPORTED mUINT32 ISA_REGISTER_SUBCLASS_COUNT = ISA_REGISTER_SUBCLASS_STATIC_COUNT;\n");
  }                            /* if (gen_static_code)   */

  if(gen_dyn_code)             /* For dynamic extensions */
   { fprintf(hfile,"\n");

     for (rsc_iter=subclasses.begin(),i=0; 
          rsc_iter != subclasses.end(); ++rsc_iter, ++i) {
          ISA_REGISTER_SUBCLASS subclass = *rsc_iter;
          fprintf(
           hfile, 
           "#define ISA_REGISTER_SUBCLASS_LOCAL_%s_%-9s %d\n"
           "#define ISA_REGISTER_SUBCLASS_%s_%-15s "
           "(ISA_REGISTER_SUBCLASS_LOCAL_%s_%s+ISA_REGISTER_SUBCLASS_STATIC_MAX+1)\n"
           "\n",
           extname,subclass->name,i,
           extname,subclass->name,extname,subclass->name);
           }

     fprintf(hfile, 
             "\n"
             "#define ISA_REGISTER_SUBCLASS_EXTENSION_%s_COUNT %d\n\n", 
             extname,i);
   }

  if(gen_static_code)
   { fprintf(hfile, 
             "\n"
             "typedef mUINT8 mISA_REGISTER_SUBCLASS;\n\n"
             "#define FOR_ALL_ISA_REGISTER_SUBCLASS(sc) \\\n"
             "\tfor (sc = ISA_REGISTER_SUBCLASS_MIN; \\\n"
             "\t     sc <= ISA_REGISTER_SUBCLASS_MAX; \\\n"
             "\t     sc = (ISA_REGISTER_SUBCLASS)(sc + 1))\n"
             "\n\n"
             "typedef struct {\n"
             "const char *name;\n"
             "  mISA_REGISTER_CLASS rclass;\n"
             "  mUINT8 count;\n"
             "  mUINT8 is_canonical;\n"
             "  mUINT8 members[ISA_REGISTER_MAX+1];\n"
             "  const char *reg_name[ISA_REGISTER_MAX+1];\n"
             "} ISA_REGISTER_SUBCLASS_INFO;\n");

     fprintf(efile, "ISA_REGISTER_SUBCLASS_info\n");
    }

  tabname = gen_static_code ? "ISA_REGISTER_SUBCLASS_info_static" :
                              "ISA_REGISTER_SUBCLASS_info_dynamic";

  fprintf(cfile,
         "\n"
         "static const ISA_REGISTER_SUBCLASS_INFO %s[] = {\n",
         tabname);
  if(gen_static_code)     /* Manage entry for UNDEFINED item. */
    fprintf(cfile,
         "  { \"%s\", ISA_REGISTER_CLASS_%s, 0, 0, { 0 }, { 0 } },\n", 
         "UNDEFINED", 
         "UNDEFINED");

  for (rsc_iter = subclasses.begin(); rsc_iter != subclasses.end();++rsc_iter) {
    ISA_REGISTER_SUBCLASS subclass = *rsc_iter;

    if(gen_static_code)
      fprintf(cfile, 
        "  { \"%s\", ISA_REGISTER_CLASS_%s, %d, %d /*canonical*/,", 
        subclass->name, 
        subclass->rclass->name, 
        subclass->count,
        subclass->is_canonical ? 1:0);
    else
      fprintf(cfile,
        "  { \"%s\", ISA_REGISTER_CLASS_LOCAL_%s_%s, %d , %d /*canonical*/,",
        subclass->name,
        extname,
        subclass->rclass->name,
        subclass->count,
        subclass->is_canonical ? 1:0);

    int len = fprintf(cfile, "\n    { ");
    for (i = 0; i < subclass->count; ++i) {
      if (len > 70) len = fprintf(cfile, "\n      ");
      len += fprintf(cfile, "%d%s",
			    subclass->members[i],
			    i != (subclass->count - 1) ? ", " : "");
    }
    fprintf(cfile, " },\n");

    len = fprintf(cfile, "    { ");
    if (subclass->names) {
      for (i = 0; i < subclass->count; ++i) {
	if (len > 70) len = fprintf(cfile, "\n      ");
	if (subclass->names[i]) {
	  len += fprintf(cfile, "\"%s\"", subclass->names[i]);
	} else {
	  len += fputs("0", cfile);
	}
	len += fprintf(cfile, "%s", i != (subclass->count - 1) ? ", " : "");
      }
    } else {
      fputs("0", cfile);
    }
    fprintf(cfile, " } },\n");
  }
  fprintf(cfile, "};\n");

  if(gen_static_code)
   { fprintf(cfile,
             "\n"
             "BE_EXPORTED const ISA_REGISTER_SUBCLASS_INFO *"
             "ISA_REGISTER_SUBCLASS_info = %s;\n",
             tabname);
     fprintf(hfile,
             "\n"
             "BE_EXPORTED extern const ISA_REGISTER_SUBCLASS_INFO"
             " *ISA_REGISTER_SUBCLASS_info;\n");
   }
  else
   { fprintf(cfile,
             "\n"
             "const ISA_REGISTER_SUBCLASS_INFO *dyn_get_ISA_REGISTER_SUBCLASS_tab (void)\n"
             "{  return %s;\n"
             "}\n"
             "const mUINT32 dyn_get_ISA_REGISTER_SUBCLASS_tab_sz (void)\n"
             "{  return  ISA_REGISTER_SUBCLASS_EXTENSION_%s_COUNT;\n"
             "}\n\n",
             tabname,extname);
     fprintf(hfile,
             "extern const ISA_REGISTER_SUBCLASS_INFO *dyn_get_ISA_REGISTER_SUBCLASS_tab ( void );\n"
             "extern const mUINT32 dyn_get_ISA_REGISTER_SUBCLASS_tab_sz ( void );\n");
   }

  /**************************************************
   * accessors:
   */

  if(gen_static_code) {
  fprintf(hfile, "\ninline const ISA_REGISTER_CLASS_INFO *ISA_REGISTER_CLASS_Info(\n"
		 "  ISA_REGISTER_CLASS rc\n"
		 ")\n"
		 "{\n"
		 "  BE_EXPORTED extern const ISA_REGISTER_CLASS_INFO *ISA_REGISTER_CLASS_info;\n"
		 "  BE_EXPORTED extern mUINT8 *ISA_REGISTER_CLASS_info_index;\n"
		 "  INT index = ISA_REGISTER_CLASS_info_index[(INT)rc];\n"
		 "  return &ISA_REGISTER_CLASS_info[index];\n"
		 "}\n");

  fprintf(hfile, "\ninline INT ISA_REGISTER_CLASS_INFO_First_Reg(\n"
		 "  const ISA_REGISTER_CLASS_INFO *info\n"
		 ")\n"
		 "{\n"
		 "  return info->min_regnum;\n"
		 "}\n");

  fprintf(hfile, "\ninline INT ISA_REGISTER_CLASS_INFO_Last_Reg(\n"
		 "  const ISA_REGISTER_CLASS_INFO *info\n"
		 ")\n"
		 "{\n"
		 "  return info->max_regnum;\n"
		 "}\n");

  fprintf(hfile, "\ninline INT ISA_REGISTER_CLASS_INFO_Bit_Size(\n"
		 "  const ISA_REGISTER_CLASS_INFO *info\n"
		 ")\n"
		 "{\n"
		 "  return info->bit_size;\n"
		 "}\n");

  fprintf(hfile, "\ninline BOOL ISA_REGISTER_CLASS_INFO_Is_Ptr(\n"
		 "  const ISA_REGISTER_CLASS_INFO *info\n"
		 ")\n"
		 "{\n"
		 "  return info->is_ptr;\n"
		 "}\n");

  fprintf(hfile, "\ninline BOOL ISA_REGISTER_CLASS_INFO_Can_Store(\n"
		 "  const ISA_REGISTER_CLASS_INFO *info\n"
		 ")\n"
		 "{\n"
		 "  return info->can_store;\n"
		 "}\n");

  fprintf(hfile, "\ninline BOOL ISA_REGISTER_CLASS_INFO_Multiple_Save(\n"
		 "  const ISA_REGISTER_CLASS_INFO *info\n"
		 ")\n"
		 "{\n"
		 "  return info->multiple_save;\n"
		 "}\n");

  fprintf(hfile, "\ninline const char *ISA_REGISTER_CLASS_INFO_Name(\n"
		 "  const ISA_REGISTER_CLASS_INFO *info\n"
		 ")\n"
		 "{\n"
		 "  return info->name;\n"
		 "}\n");

  fprintf(hfile, "\ninline const char *ISA_REGISTER_CLASS_INFO_Reg_Name(\n"
		 "  const ISA_REGISTER_CLASS_INFO *info,\n"
		 "  INT reg_index\n"
		 ")\n"
		 "{\n"
		 "  return info->reg_name[reg_index];\n"
		 "}\n");

  fprintf(hfile, "\ninline const ISA_REGISTER_SUBCLASS_INFO *ISA_REGISTER_SUBCLASS_Info(\n"
		 "  ISA_REGISTER_SUBCLASS sc\n"
		 ")\n"
		 "{\n"
		 "  BE_EXPORTED extern const ISA_REGISTER_SUBCLASS_INFO *ISA_REGISTER_SUBCLASS_info;\n"
		 "  return &ISA_REGISTER_SUBCLASS_info[sc];\n"
		 "}\n");

  fprintf(hfile, "\ninline const char *ISA_REGISTER_SUBCLASS_INFO_Name(\n"
		 "  const ISA_REGISTER_SUBCLASS_INFO *info\n"
		 ")\n"
		 "{\n"
		 "  return info->name;\n"
		 "}\n");

  fprintf(hfile, "\ninline ISA_REGISTER_CLASS ISA_REGISTER_SUBCLASS_INFO_Class(\n"
		 "  const ISA_REGISTER_SUBCLASS_INFO *info\n"
		 ")\n"
		 "{\n"
		 "  return (ISA_REGISTER_CLASS)info->rclass;\n"
		 "}\n");

  fprintf(hfile, "\ninline INT ISA_REGISTER_SUBCLASS_INFO_Count(\n"
		 "  const ISA_REGISTER_SUBCLASS_INFO *info\n"
		 ")\n"
		 "{\n"
		 "  return info->count;\n"
		 "}\n");

  fprintf(hfile, "\ninline UINT ISA_REGISTER_SUBCLASS_INFO_Member(\n"
		 "  const ISA_REGISTER_SUBCLASS_INFO *info,\n"
		 "  INT n\n"
		 ")\n"
		 "{\n"
		 "  return info->members[n];\n"
		 "}\n");

  fprintf(hfile, "\ninline const char *ISA_REGISTER_SUBCLASS_INFO_Reg_Name(\n"
		 "  const ISA_REGISTER_SUBCLASS_INFO *info,\n"
		 "  INT n\n"
		 ")\n"
		 "{\n"
		 "  return info->reg_name[n];\n"
		 "}\n");

  fprintf(hfile, "\ninline INT ISA_REGISTER_SUBCLASS_INFO_Canonical(\n"
                 "   const ISA_REGISTER_SUBCLASS_INFO *info\n"
                 ")\n"
                 "{\n"
                 "  return info->is_canonical ? 1 : 0;\n"
                 "}\n");

  fprintf(hfile, "\nBE_EXPORTED extern void ISA_REGISTER_Initialize(void);\n");

  fprintf(efile, "ISA_REGISTER_Initialize\n");

  fprintf(cfile, "\nvoid ISA_REGISTER_Initialize(void)\n"
		 "{\n"
		 "  INT rc;\n"
		 "  INT mask = 1 << (INT)ISA_SUBSET_Value;\n"
		 "  for (rc = ISA_REGISTER_CLASS_MIN; "
			 "rc <= ISA_REGISTER_CLASS_STATIC_MAX; ++rc) {\n"
		 "    INT i = ISA_REGISTER_CLASS_info_index[rc];\n"
		 "    const ISA_REGISTER_CLASS_INFO *info = "
			    "&ISA_REGISTER_CLASS_info[i];\n"
		 "    while ((info->isa_mask & mask) == 0) ++info, ++i;\n"
		 "    ISA_REGISTER_CLASS_info_index[rc] = i;\n"
		 "  }\n"
		 "}\n");
  }                             // gen_static_code

// #ifdef DYNAMIC_CODE_GEN
//   fprintf(cfile, "\nstatic const char * ISA_REGISTER_CLASS_static_name_tab[] = {\n");
//   for (i = DYNA::ISA_REGISTER_CLASS_MIN; i < DYNA::ISA_REGISTER_CLASS_STATIC_MAX; i++) {
//   fprintf(cfile,
// 	  "  \"%s\",\n", DYNA::ISA_REGISTER_CLASS_INFO_Name(DYNA::ISA_REGISTER_CLASS_Info(i)));
//   }
//   fprintf(cfile,
// 	  "};\n\n");
// #endif

  Emit_Footer(hfile);
  Emit_C_Footer(cfile);

  // Closing file handlers.
  Gen_Close_File_Handle(hfile,hfilename);
  Gen_Close_File_Handle(cfile,cfilename);
  if(efile)
    Gen_Close_File_Handle(efile,efilename);

  // Memory deallocation.
  Gen_Free_Filename(cfilename);
  Gen_Free_Filename(hfilename);
  Gen_Free_Filename(targ_isa_subset_hfilename);
  if(efilename)
    Gen_Free_Filename(efilename);

  return;
}
