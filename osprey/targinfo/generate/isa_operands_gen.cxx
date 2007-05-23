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


//  isa_operands_gen.cxx
/////////////////////////////////////
//
//  Interface for specifying operands and results for various
//  instructions in the ISA.
//
/////////////////////////////////////
//

typedef struct operand_value_type *OPERAND_VALUE_TYPE;

#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
// [HK]
#if __GNUC__ >=3 || defined(_MSC_VER)
#include <list>
#include <vector>
#include <algorithm>
using std::list;
using std::vector;
#else
#include <list.h>
#include <vector.h>
#include <algo.h>
#endif // __GNUC__ >=3 || defined(_MSC_VER)
#include "topcode.h"
#include "targ_isa_properties.h"
#include "gen_util.h"
#include "isa_operands_gen.h"

#include "isa_ext_limits.h"

#ifdef DYNAMIC_CODE_GEN
#include "dyn_isa_topcode.h"
#include "dyn_stub3.h"
#endif

// In following loops, we iterate on the number of
// TOP. This number differs whether we generate
// static or dynamic TOPs.
#ifndef DYNAMIC_CODE_GEN
static mUINT32 TOP_count_limit = TOP_static_count;
#else
static mUINT32 TOP_count_limit = TOP_dyn_count;
#endif



struct operand_value_type {
  const char* name;         // Name given for documentation and debugging
  ISA_REGISTER_CLASS register_class;	
  ISA_REGISTER_SUBCLASS register_subclass;
  ISA_LIT_CLASS literal_class;
  ISA_ENUM_CLASS enum_class;
  bool is_register;
  bool is_signed;
  bool is_pcrel;
  bool is_fpu_int;
  int size;
  int index;
};

typedef struct operands_group {
  const char* name;         // Name given for documentation and debugging
  int opnd_count;
  int result_count;
  vector <OPERAND_VALUE_TYPE> operands;
  int relocatable_opnd;
  vector <OPERAND_VALUE_TYPE> results;
  //  vector <OPERAND_USE_TYPE> opnd_use;
  //  vector <OPERAND_USE_TYPE> res_use;
  vector <OPERANDS_GROUP_OPERAND_USES> opnd_use;
  vector <OPERANDS_GROUP_OPERAND_USES> res_use;
  vector <mUINT8> same_res;
  bool is_load;
  bool is_store;
  bool base;
  bool offset;
  bool storeval;
  int index;
} *OPERANDS_GROUP;

// Description of operand types.
// Mask 0x1 is reserved to OU_UNDEFINED.
// As a result, the relation between mask and
// index is mask = 1U<<(index+1). Mask of first
// item is following OPERAND_USE_TYPE lists is
// therefore 0x2.
struct operand_use_type {
  const char *name;
  mUINT32     index;
  mUINT32     mask;
};

static vector <OPERANDS_GROUP> op_groups;
static OPERANDS_GROUP cur_oper_group;
static list <OPERAND_VALUE_TYPE> all_operand_types;
static list <OPERAND_USE_TYPE> all_use_types;
static list <OPERANDS_GROUP> all_groups; // All the instruction groups

// max_operands and max_results are initialized to the highest indexes
// available 
static int max_operands = (EXTENSION_NB_OPERAND_MAX > 0)?EXTENSION_NB_OPERAND_MAX-1:0;
static int max_results  = (EXTENSION_NB_RESULT_MAX  > 0)?EXTENSION_NB_RESULT_MAX-1:0;
static int max_valtypes = 0;
static int max_groups = 0;
static int max_uses = 0;

static int cur_res_index;

#ifdef DYNAMIC_CODE_GEN
// Support for register class definition and remapping in extension
static bool referenced_core_reg_classes[ISA_REGISTER_CLASS_STATIC_MAX+1];
static bool referenced_core_reg_subclasses[ISA_REGISTER_SUBCLASS_STATIC_MAX+1];
#endif


/* The generated interface description:
 */
static const char * const interface[] = {
  "/* ====================================================================",
  " * ====================================================================",
  " *",
  " * Description:",
  " *",
  " *   A description of the ISA instruction operands. The description",
  " *   exports the following:",
  " *",
  " *   typedef (struct) ISA_OPERAND_VALTYP",
  " *       Describes a particular operand/result type, including",
  " *       the type of value it may contain and whether or not is",
  " *       a register, literal or enum. The contents are private.",
  " *",
  " *   typedef (struct) ISA_OPERAND_INFO",
  " *       Identifies the operand types of a particular instruction.",
  " *       The contents are private.",
  " *",
  " *,  typedef (enum) ISA_OPERAND_USE",
  " *       Identifies the useage of an operand of a particular instruction.",
  " *       The names have the form OU_xxxx.",
  " *",
  " *   const INT OU_UNDEFINED",
  " *       Identifies an undefined/unknown operand use.",
  " *",
  " *   const INT ISA_OPERAND_max_operands",
  " *       The maximum number of operands of any instruction.",
  " *",
  " *   const INT ISA_OPERAND_max_results",
  " *       The maximum number of results of any instruction.",
  " *",
  " *   const ISA_OPERAND_INFO *ISA_OPERAND_Info(TOP topcode)",
  " *       Return a pointer to the operand info for the instruction",
  " *       specified by 'topcode'.",
  " *",
  " *   INT ISA_OPERAND_INFO_Operands(const ISA_OPERAND_INFO *oinfo)",
  " *       Return the number of operands specified by the operand",
  " *       info 'oinfo'.",
  " *",
  " *   const ISA_OPERAND_VALTYP *ISA_OPERAND_INFO_Operand(",
  " *     const ISA_OPERAND_INFO *oinfo,",
  " *     int opnd",
  " *   )",
  " *       Get the operand type of operand 'opnd' specified by the",
  " *       operand info 'oinfo'.",
  " *",
  " *   INT ISA_OPERAND_INFO_Results(const ISA_OPERAND_INFO *oinfo)",
  " *       Return the number of results specified by the operand",
  " *       info 'oinfo'.",
  " *",
  " *   const ISA_OPERAND_VALTYP *ISA_OPERAND_INFO_Result(",
  " *     const ISA_OPERAND_INFO *oinfo,",
  " *     int result",
  " *   )",
  " *       Get the operand type for the result 'result' specified by the",
  " *       operand info 'oinfo'.",
  " *",
  " *   ISA_OPERAND_USE ISA_OPERAND_INFO_Use(",
  " *     const ISA_OPERAND_INFO *oinfo,",
  " *     INT opnd",
  " *   )",
  " *       Get the operand use type of operand 'opnd' specified by the",
  " *       operand info 'oinfo'.",
  " *",
  " *   BOOL ISA_OPERAND_Any_Use(ISA_OPERAND_USE ouse)",
  " *       Returns a boolean that indicates if any instruction in the",
  " *       architecture has an an operand with usage 'use'. Useful",
  " *       for omitting sections of code that aren't applicable to",
  " *       some architectures.",
  " *",
  " *   ISA_REGISTER_CLASS ISA_OPERAND_VALTYP_Register_Class(",
  " *     const ISA_OPERAND_VALTYP *otype",
  " *   )",
  " *       Get the register class for the operand specified by 'otype'.",
  " *",
  " *   ISA_REGISTER_SUBCLASS ISA_OPERAND_VALTYP_Register_Subclass(",
  " *     const ISA_OPERAND_VALTYP *otype",
  " *   )",
  " *       Get the register subclass for the operand specified by 'otype'.",
  " *",
  " *   ISA_LIT_CLASS ISA_OPERAND_VALTYP_Literal_Class(const ISA_OPERAND_VALTYP *otype)",
  " *       Get the literal class for the operand specified by 'otype'.",
  " *",
  " *   ISA_ENUM_CLASS ISA_OPERAND_VALTYP_Enum_Class(",
  " *     const ISA_OPERAND_VALTYP *otype",
  " *   )",
  " *       Get the enum class for the operand specified by 'otype'.",
  " *",
  " *   INT ISA_OPERAND_VALTYP_Size(const ISA_OPERAND_VALTYP *otype)",
  " *       Get the size for the operand specified by 'otype'.",
  " *",
  " *   BOOL ISA_OPERAND_VALTYP_Is_Register(const ISA_OPERAND_VALTYP *otype)",
  " *       Return a boolean to specify if the operand specifed",
  " *       by 'otype' is a register.",
  " *",
  " *   BOOL ISA_OPERAND_VALTYP_Is_Signed(const ISA_OPERAND_VALTYP *otype)",
  " *       Return a boolean to specify if the operand specifed",
  " *       by 'otype' is signed.",
  " *",
  " *   BOOL ISA_OPERAND_VALTYP_Is_FPU_Int(const ISA_OPERAND_VALTYP *otype)",
  " *       Return a boolean to specify if the operand specifed",
  " *       by 'otype' is an FPU integer.",
  " *",
  " *   BOOL ISA_OPERAND_VALTYP_Is_PCRel(const ISA_OPERAND_VALTYP *otype)",
  " *       Return a boolean to specify if the operand specifed",
  " *       by 'otype' is pc-relative.",
  " *",
  " *   BOOL ISA_OPERAND_VALTYP_Is_Literal (const ISA_OPERAND_VALTYP *otype)",
  " *       Return a boolean to specify if the operand specifed",
  " *       by 'otype' is a literal.",
  " *",
  " *   BOOL ISA_OPERAND_VALTYP_Is_Enum (const ISA_OPERAND_VALTYP *otype)",
  " *       Return a boolean to specify if the operand specifed",
  " *       by 'otype' is an enum.",
  " *",
  " *   BOOL TOP_Can_Have_Immediate(INT64 value, TOP topcode)",
  " *       Return a boolean to specify if the 64-bit integer value can fit",
  " *       in the literal field of an instruction with the given topcode.",
  " *",
  " *   INT TOP_Immediate_Operand(TOP topcode, ISA_LIT_CLASS *lclass)",
  " *       If 'topcode' has an immediate operand, return its operand",
  " *       number by value and literal class by reference through 'lclass'",
  " *       (a null pointer can be passed for 'lclass' if the literal",
  " *       class is not needed). If there is no immediate operand, return -1.",
  " *",
  " *   INT TOP_Relocatable_Operand(TOP topcode, ISA_LIT_CLASS *lclass)",
  " *       If 'topcode' has a relocatable operand, return its operand",
  " *       number by value and literal class by reference through 'lclass'",
  " *       (a null pointer can be passed for 'lclass' if the literal",
  " *       class is not needed). If there is no relocatable operand, return -1.",
  " *",
  " *   INT TOP_Find_Operand_Use(TOP topcode, ISA_OPERAND_USE use)",
  " *       For the instruction specified by 'topcode', give the",
  " *       operand number with the use 'use'. If there is no such",
  " *       operand, return -1.",
  " *",
  " *   void TOP_Operand_Uses(TOP topcode, ISA_OPERAND_USE *uses)",
  " *       For the instruction specified by 'topcode', return",
  " *       the usage of all its operands in the array pointed to",
  " *       by 'uses'. The use of operand n corresponds to 'uses'[n].",
#ifdef TARG_ST // [CL]
  " *",
  " *   INT TOP_Find_Result_With_Usage(TOP topcode, ISA_OPERAND_USE use)",
  " *       For the instruction specified by 'topcode', give the",
  " *       result number with the use 'use'. If there is no such",
  " *       result, return -1.",
#endif
  " *",
  " * ====================================================================",
  " * ====================================================================",
  " */",
  NULL
};

static bool gen_static_code = true;          // Whether we generate code for an
static bool gen_dyn_code    = false;         // extension or for the core.

static char *extname        = NULL;          // Extension name (NULL if no ext).


// Arthur: some operand uses are built-in
OPERAND_USE_TYPE base;
OPERAND_USE_TYPE offset;
OPERAND_USE_TYPE storeval;

#if 0
OPERAND_USE_TYPE implicit;
OPERAND_USE_TYPE uniq_res;
#endif

/////////////////////////////////////
void ISA_Operands_Begin( const char* /* name */ )
/////////////////////////////////////
//  See interface description.
/////////////////////////////////////
{
   op_groups = vector <OPERANDS_GROUP>(TOP_count_limit,(OPERANDS_GROUP) false);

   gen_static_code = Is_Static_Code();
   gen_dyn_code    =!gen_static_code;

   if(gen_dyn_code)
     extname = Get_Extension_Name();

#ifdef DYNAMIC_CODE_GEN
   if (! gen_static_code) {
     memset(referenced_core_reg_classes, 0, (ISA_REGISTER_CLASS_STATIC_MAX+1)*sizeof(bool));
     memset(referenced_core_reg_subclasses, 0, (ISA_REGISTER_SUBCLASS_STATIC_MAX+1)*sizeof(bool));
  }
#endif

#if 0
  // Initialize built-in operand uses

  // 1. base for TOP_load,TOP_store
  base = new operand_use_type;
  base->name = "base";
  base->index = max_uses++;
  all_use_types.push_back(base);

  // 2. offset for TOP_load,TOP_store
  offset = new operand_use_type;
  offset->name = "offset";
  offset->index = max_uses++;
  all_use_types.push_back(offset);

  // 3. storeval for TOP_store
  storeval = new operand_use_type;
  storeval->name = "storeval";
  storeval->index = max_uses++;
  all_use_types.push_back(storeval);

  // 4. implicit
  implicit = new operand_use_type;
  implicit->name = "implicit";
  implicit->index = max_uses++;
  all_use_types.push_back(implicit);

  // 5. uniq_res
  uniq_res = new operand_use_type;
  uniq_res->name = "uniq_res";
  uniq_res->index = max_uses++;
  all_use_types.push_back(uniq_res);
#endif

  return;
}

/* ====================================================================
 *   ISA_Reg_Opnd_Type_Create
 * ====================================================================
 */
OPERAND_VALUE_TYPE ISA_Reg_Opnd_Type_Create ( 
  const char* name, 
  ISA_REGISTER_CLASS register_class, 
  ISA_REGISTER_SUBCLASS subclass,
  int size,
  RTYPE type,
  FP_TYPE fp_int
)
{
  if (type != SIGNED && type != UNSIGNED) {
    fprintf(stderr, 
            "### Error: "
            "RTYPE for register operand %s must be SIGNED or UNSIGNED\n",
            name);
    exit(EXIT_FAILURE);
  }

  OPERAND_VALUE_TYPE result = new operand_value_type;

  all_operand_types.push_back(result);

  result->name = name;
  result->register_class = register_class;
  result->register_subclass = subclass;
  result->literal_class = ISA_LC_UNDEFINED;
  result->enum_class = ISA_EC_UNDEFINED;
  result->size = size;
  result->is_register = true;
  result->is_signed = type == SIGNED;
  result->is_pcrel = false;
  result->is_fpu_int = fp_int != INVALID;
  result->index = max_valtypes++;

#ifdef DYNAMIC_CODE_GEN
  if ( (! gen_static_code) &&
       (register_class >= ISA_REGISTER_CLASS_MIN) &&
       (register_class <= ISA_REGISTER_CLASS_STATIC_MAX)) {

    referenced_core_reg_classes[register_class] = true;

    if ((subclass >= ISA_REGISTER_SUBCLASS_MIN) &&
	(subclass <= ISA_REGISTER_SUBCLASS_MAX)) {

      referenced_core_reg_subclasses[subclass] = true;
    }
  }
#endif

  return result;
}

/* ====================================================================
 *   ISA_Lit_Opnd_Type_Create
 * ====================================================================
 */
OPERAND_VALUE_TYPE ISA_Lit_Opnd_Type_Create ( 
  const char* name, 
  int size,
  RTYPE type,
  ISA_LIT_CLASS literal_class
)
{
  if (type != SIGNED && type != UNSIGNED && type != PCREL) {
    fprintf(stderr, "### Error: RTYPE for literal operand %s must be PCREL, SIGNED or UNSIGNED\n",
		    name);
    exit(EXIT_FAILURE);
  }

  OPERAND_VALUE_TYPE result = new operand_value_type;

  all_operand_types.push_back(result);

  result->name = name;
  result->register_class = ISA_REGISTER_CLASS_UNDEFINED;
  result->register_subclass = ISA_REGISTER_SUBCLASS_UNDEFINED;
  result->literal_class = literal_class;
  result->enum_class = ISA_EC_UNDEFINED;
  result->is_register = false;
  result->is_signed = (type == SIGNED) || (type == PCREL);
  result->is_pcrel = (type == PCREL);
  result->is_fpu_int = false;
  result->size = size;
  result->index = max_valtypes++;

  return result;
}

/* ====================================================================
 *   ISA_Enum_Opnd_Type_Create
 * ====================================================================
 */
OPERAND_VALUE_TYPE ISA_Enum_Opnd_Type_Create ( 
  const char* name, 
  int size,
  RTYPE type,
  ISA_ENUM_CLASS enum_class
)
{
  if (type != SIGNED && type != UNSIGNED) {
    fprintf(stderr, "### Error: RTYPE for enumerated operand %s must be SIGNED or UNSIGNED\n",
		    name);
    exit(EXIT_FAILURE);
  }

  OPERAND_VALUE_TYPE result = new operand_value_type;

  all_operand_types.push_back(result);

  result->name = name;
  result->register_class = ISA_REGISTER_CLASS_UNDEFINED;
  result->register_subclass = ISA_REGISTER_SUBCLASS_UNDEFINED;
  result->literal_class = ISA_LC_UNDEFINED;
  result->enum_class = enum_class;
  result->is_register = false;
  result->is_signed = type == SIGNED;
  result->is_pcrel = false;
  result->is_fpu_int = false;
  result->size = size;
  result->index = max_valtypes++;

  return result;
}

/* ====================================================================
 *   Create_Operand_Use
 * ====================================================================
 */
OPERAND_USE_TYPE Create_Operand_Use( const char *name )
{
#if 0
  if (!strcmp(name, "base") ||
      !strcmp(name, "offset") ||
      !strcmp(name, "storeval") ||
      !strcmp(name, "implicit") ||
      !strcmp(name, "uniq_res")) {
    fprintf(stderr, "### Error: built-in operand use %s redefined.\n",
		    name);
    exit(EXIT_FAILURE);
  }
#endif

  OPERAND_USE_TYPE result = new operand_use_type;

  all_use_types.push_back(result);

  result->name = name;
  result->index = max_uses++;
  result->mask = 1U<<(result->index+1U);

  // remember "usefull" use types:
  if (!strcmp(name, "base")) base = result;
  if (!strcmp(name, "offset")) offset = result;
  if (!strcmp(name, "storeval")) storeval = result;

  return result;
}

/* ====================================================================
 *   Instruction_Group
 * ====================================================================
 */
void Instruction_Group( const char *name, ... )
{
  va_list ap;
  TOP opcode;
  TOP termination = gen_static_code ? TOP_UNDEFINED : static_cast<TOP>(-1);

  OPERANDS_GROUP oper_group = new operands_group;

  cur_oper_group = oper_group;
  oper_group->name = name;
  oper_group->opnd_count = 0;
  oper_group->operands = vector<OPERAND_VALUE_TYPE>();
  oper_group->relocatable_opnd = -1;
  oper_group->result_count = 0;
  oper_group->results = vector<OPERAND_VALUE_TYPE>();
  oper_group->opnd_use = vector<OPERANDS_GROUP_OPERAND_USES>();
  oper_group->res_use = vector<OPERANDS_GROUP_OPERAND_USES>();
  oper_group->same_res = vector<mUINT8>();
  oper_group->is_load = false;
  oper_group->is_store = false;
  oper_group->base = false;
  oper_group->offset = false;
  oper_group->storeval = false;
  oper_group->index = max_groups++;

  va_start(ap, name);

  // The first OPcode determines whether it is a load/store group
  if ((opcode = static_cast<TOP>(va_arg(ap,int))) != termination ) {
    if (op_groups[(int)opcode]) {
      fprintf(stderr, 
	      "### Error: Instruction_Group %s: redefines group (%s) for %s\n",
	      name, op_groups[(int)opcode]->name, TOP_Name(opcode));
    }
    if (TOP_is_load(opcode)) oper_group->is_load = true;
    if (TOP_is_store(opcode)) oper_group->is_store = true;
    op_groups[(int)opcode] = oper_group;
  }

  while ( (opcode = static_cast<TOP>(va_arg(ap,int))) != termination ) {
    if (op_groups[(int)opcode]) {
      fprintf(stderr, 
	      "### Error: Instruction_Group %s: redefines group (%s) for %s\n",
	      name, op_groups[(int)opcode]->name, TOP_Name(opcode));
    }
    if ((TOP_is_load(opcode) && oper_group->is_load == false) ||
	(TOP_is_store(opcode) && oper_group->is_store == false)) {
      fprintf(stderr, 
	      "### Error: Instruction_Group %s: %s name is not a memory group opcode\n", name, TOP_Name(opcode));
    }
    op_groups[(int)opcode] = oper_group;
  }
  va_end(ap);

  all_groups.push_back (oper_group);

  cur_res_index = -1;
}

/* ====================================================================
 *   Operand
 *
 *   Arthur: allow several calls to this for the same operand
 * ====================================================================
 */
void Operand (int operand_index, 
	      OPERAND_VALUE_TYPE operand_type,
	      OPERAND_USE_TYPE operand_use)
{
  if (operand_index > max_operands) {
    if (EXTENSION_NB_OPERAND_MAX > 0) {
      // When dealing with extension (either for Core or extension part),
      // it is not allowed to overflow the predefined 'max_operands'
      fprintf(stderr, "### Error: %s has too much operands in the context of"
	      " dynamic extension. (maximum number of operand is %d)\n",
	      cur_oper_group->name, EXTENSION_NB_OPERAND_MAX);
      exit(EXIT_FAILURE);
    }

    max_operands = operand_index;
  }

  if (operand_index >= cur_oper_group->opnd_count) {
    cur_oper_group->opnd_count = operand_index + 1;
  }

  int incr = (operand_index+1) - cur_oper_group->operands.size();
  if (incr > 0) {
    cur_oper_group->operands.insert(cur_oper_group->operands.end(),
				    incr,
				    (OPERAND_VALUE_TYPE)NULL);
    cur_oper_group->opnd_use.insert(cur_oper_group->opnd_use.end(),
				    incr,
				    (OPERANDS_GROUP_OPERAND_USES)NULL);
  }

  // check if this is not the first call to this routine that this
  // operand type is consistent.
  if (cur_oper_group->operands[operand_index] != NULL) {
    if (cur_oper_group->operands[operand_index] != operand_type) {
      fprintf(stderr, "### Error: %s has more than one operand type\n",
		    cur_oper_group->name);
      exit(EXIT_FAILURE);
    }
  }
  else {
    cur_oper_group->operands[operand_index] = operand_type;
  }

  // Add the use to the set of this operand uses
  if (operand_use != (OPERAND_USE_TYPE)NULL) {
    cur_oper_group->opnd_use[operand_index] |= /*1 << (operand_use->index + 1)*/ operand_use->mask;
    if (operand_use == base) cur_oper_group->base = true;
    if (operand_use == offset) cur_oper_group->offset = true;
    if (operand_use == storeval) cur_oper_group->storeval = true;
  }
}

/* ====================================================================
 *   Relocatable
 * ====================================================================
 */
void Relocatable (int operand_index)
{
  if (cur_oper_group->relocatable_opnd >= 0) {
    fprintf(stderr, "### Error: %s has more than one relocatable operand\n",
		    cur_oper_group->name);
    exit(EXIT_FAILURE);
  }
  cur_oper_group->relocatable_opnd = operand_index;
}

/* ====================================================================
 *   Result
 *
 *   Arthur: allow several calls to this for the same result
 *           specify result use, eg. implicit, uniq_res
 * ====================================================================
 */
void Result (
  int result_index, 
  OPERAND_VALUE_TYPE result_type,
  OPERAND_USE_TYPE result_use
)
{
  if (result_index > max_results) {
    if (EXTENSION_NB_RESULT_MAX > 0) {
      fprintf(stderr, "### Error: %s has too much results in the context of"
	      " dynamic extension. (maximum number of result is %d)\n",
	      cur_oper_group->name, EXTENSION_NB_RESULT_MAX);
      exit(EXIT_FAILURE);
    }

    max_results = result_index;
  }

  if (result_index >= cur_oper_group->result_count) {
    cur_oper_group->result_count = result_index + 1;
  }

  int incr = (result_index+1) - cur_oper_group->results.size();
  if (incr > 0) {
    cur_oper_group->results.insert(cur_oper_group->results.end(),
				    incr,
				    (OPERAND_VALUE_TYPE)NULL);
    cur_oper_group->res_use.insert(cur_oper_group->res_use.end(),
				    incr,
				    (OPERANDS_GROUP_OPERAND_USES)NULL);
    cur_oper_group->same_res.insert(cur_oper_group->same_res.end(),
				    incr,
				    (mUINT8)NULL);
  }

  // check if this is not the first call to this routine that this
  // result type is consistent.
  if (cur_oper_group->results[result_index] != NULL) {
    if (cur_oper_group->results[result_index] != result_type) {
      fprintf(stderr, "### Error: %s has more than one result type\n",
		    cur_oper_group->name);
      exit(EXIT_FAILURE);
    }
  }
  else {
    cur_oper_group->results[result_index] = result_type;
  }

  // Add this result use to the set of this operand uses
  if (result_use != (OPERAND_USE_TYPE)NULL) {
    cur_oper_group->res_use[result_index] |= /*1 << (result_use->index + 1)*/ result_use->mask;
  }

  //  cur_oper_group->res_use[result_index] = (OPERANDS_GROUP_OPERAND_USES)NULL;
  cur_res_index = result_index;
}

/* ====================================================================
 *   Printing operand use as a list of bitwise elementary flags
 *   'OU_xxx | OU_xxx ....'
 * ====================================================================
 */
static mUINT32
print_use_type( FILE *f, OPERANDS_GROUP_OPERAND_USES type )
{
   mUINT32       pos = 0;                   /* Return value */
   bool          first;
   const char   *tmp;
   list<OPERAND_USE_TYPE>::iterator iuti;

   /* OPERAND_GROUP_OPERAND_USES is normally an unsigned integral type but */
   /* it remains safer to make a local copy (in order to prevent any       */
   /* further change).                                                     */
   mUINT32 localtype  = (mUINT32) type;

   if(sizeof(mUINT32) != sizeof(OPERANDS_GROUP_OPERAND_USES))
    { fprintf(stderr," ### Error: types should have identical size\n");
      exit(EXIT_FAILURE);
    }

   /* We simply iterate on */
   for(iuti  = all_use_types.begin(),first=true; 
       iuti != all_use_types.end() && localtype != 0;
       ++iuti)
    { if((localtype & (*iuti)->mask))
       { if(first)
          { tmp   = "";
            first = false;
          }
         else
          { tmp   = " | ";
          }
         pos+=(mUINT32)fprintf(f,"%sOU_%s",tmp,(*iuti)->name);
       }
    }

   if(true==first)
    { if(localtype!=0)
       { fprintf(stderr," ### Internal error: unknown type\n");
         exit(EXIT_FAILURE);
       }
     pos+=fprintf(f,"0x%08x",localtype);
    }

   return pos;
}

/* ====================================================================
 *   Same_Res
 * ====================================================================
 */
void Same_Res (int operand_index)
{
  if (cur_res_index < 0) {
    fprintf(stderr, "### Error: Same_Res() called before Result() for %s\n",
		                     cur_oper_group->name);
    exit(EXIT_FAILURE);
  }

  if (cur_oper_group->same_res[cur_res_index] > 0) {
    fprintf(stderr, "### Error: result %d of %s has more than one same_res\n",
		    cur_res_index, cur_oper_group->name);
    exit(EXIT_FAILURE);
  }
  cur_oper_group->same_res[cur_res_index] = operand_index+1;
}

/////////////////////////////////////
void ISA_Operands_End(void)
/////////////////////////////////////
//  See interface description.
/////////////////////////////////////
{
  list          <OPERAND_VALUE_TYPE>::iterator ivti;
  list          <OPERANDS_GROUP>::iterator ogi;
  list          <OPERAND_USE_TYPE>::iterator iuti;
  TOP           code;
  bool          err;
  const char   *info_index_type;
  const char   *info_reloc_opnd_type;
  int           first_literal = max_operands;
  int           last_literal = -1;
  int           flag_mask = 0;
  unsigned long long use_mask = 0;
  unsigned long long def_mask = 0;
  const char   *max_operands_name = "ISA_OPERAND_max_operands";
  const char   *max_results_name = "ISA_OPERAND_max_results";
  enum {
    FLAG_IS_REG	    = 0x1,
    FLAG_IS_SIGNED  = 0x2,
    FLAG_IS_FPU_INT = 0x4,
    FLAG_IS_PCREL   = 0x8
  };
  unsigned int  i;

  if (max_uses > 32 - 1) {
    fprintf(stderr, "###Error: can't handle > 31 (%d) OPERAND_USE_TYPEs\n",
		    max_uses);
    exit(EXIT_FAILURE);
  }

  for (err = false, code = 0; code < TOP_count_limit; ++code) {
    if (!op_groups[code]) {
      fprintf (stderr, "###Error: no specification for opcode: %s\n",
		       TOP_Name((TOP)code));
      err = true;
    }
  }
  if (err) exit(EXIT_FAILURE);

  char *cfilename = NULL;                             /* C file name      */
  char *hfilename = NULL;                             /* Header file name */
  char *efilename = NULL;                             /* Export file name */

  FILE *cfile     = NULL;                             /* C file handler   */
  FILE *hfile     = NULL;                             /* Header f. handler*/
  FILE *efile     = NULL;                             /* Export f. handler*/

  const char *const bname = FNAME_TARG_ISA_OPERANDS;  /* Base name        */
  int    maxenum;

  const char *const tabinc_static[] = {               /* Building the list*/
    FNAME_TOPCODE,                                    /* of files to be   */
    FNAME_TARG_ISA_REGISTERS,                         /* included.        */
    FNAME_TARG_ISA_PROPERTIES,
    FNAME_TARG_ISA_LITS,
    FNAME_TARG_ISA_ENUMS,
  };

  const char *const tabinc_dynamic[] = {              /* Building the list */
    FNAME_ISA_REGISTERS,                              /* of files to be    */
    FNAME_ISA_PROPERTIES,                             /* included for dyn. */
    FNAME_ISA_LITS,                                   /* extensions.       */
    FNAME_ISA_ENUMS,
  };

#ifdef DYNAMIC_CODE_GEN
   /* For dynamic extensions, we need to initialize tables
      that describe register files in a similar way to what
      is done by dynamic loader. Indeed, following code require
      that routines

        ISA_REGISTER_CLASS_INFO_Name(rcinfo),
        ISA_REGISTER_CLASS_SUBCLASS_INFO_Name(rcinfo),

      return correct results.
    */
   Initialize_Register_Class_Stub();
#endif

  hfilename = Gen_Build_Filename(bname,extname,gen_util_file_type_hfile);
  hfile     = Gen_Open_File_Handle(hfilename, "w");

  cfilename = Gen_Build_Filename(bname,extname,gen_util_file_type_cfile);
  cfile     = Gen_Open_File_Handle(cfilename, "w");

  if(gen_static_code)
   { efilename = Gen_Build_Filename(bname,extname,gen_util_file_type_efile);
     efile     = Gen_Open_File_Handle(efilename, "w");
   }


  fprintf(cfile,"\n");
  Emit_C_Header(cfile);           /* Emit extern "C" directive */
  fprintf(cfile,"\n\n");

  /* The following loop adds all necessary #include in header file. */
  if(gen_static_code)
   { for(i=0;i<sizeof(tabinc_static)/sizeof(char*);i++)
      { char *tmpname=Gen_Build_Filename(tabinc_static[i],NULL,
                                         gen_util_file_type_hfile);
        fprintf(hfile,"#include \"%s\"\n",tmpname);
        Gen_Free_Filename(tmpname);
      }
   }
  else
   { for(i=0;i<sizeof(tabinc_dynamic)/sizeof(char*);i++)
      { char *tmpname=Gen_Build_Filename(tabinc_dynamic[i],NULL,
                                         gen_util_file_type_dyn_hfile);
        fprintf(hfile,"#include \"%s\"\n",tmpname);
        Gen_Free_Filename(tmpname);
      }
   }
  fprintf(hfile,"\n\n");
  Emit_Header (hfile,bname,interface,extname);

  if(gen_static_code)
   {fprintf(cfile,"#include \"%s\"\n", hfilename);
   }
  else
   { char *tmpname = Gen_Build_Filename(FNAME_ISA_OPERANDS,NULL,
                                        gen_util_file_type_dyn_hfile);
     fprintf(cfile,"#include \"%s\"\n",tmpname);
     Gen_Free_Filename(tmpname);
   }
 
  // --------------------------------------------------------------------
  //
  //       Definition of operand use flags.
  //
  // --------------------------------------------------------------------

  if(gen_static_code)
   { fprintf(hfile, "\ntypedef UINT32 ISA_OPERAND_USE;\n");
     fprintf(hfile, "\n#define OU_UNDEFINED%-6s 0x00000000\n","");

     for (maxenum = 0, iuti = all_use_types.begin(); 
          iuti != all_use_types.end(); 
          ++maxenum, ++iuti)
      { OPERAND_USE_TYPE use_type = *iuti;
        fprintf(hfile, "#define OU_%-15s 0x%08x\n", 
                use_type->name,/* 1 << (use_type->index + 1)*/use_type->mask);
      }
     fprintf(hfile, "#define OU_MAX%-12s 0x%08x\n", "",1 << maxenum);
   }

  if(gen_static_code)
   { fprintf(hfile, 
             "\ntypedef struct {\n"
             "  mUINT8  rclass;\n"
             "  mUINT8  rsubclass;\n"
             "  mUINT8  lclass;\n"
             "  mUINT8  eclass;\n"
             "  mUINT16 size;\n"
             "  mUINT8  flags;\n"
             "} ISA_OPERAND_VALTYP;\n");

     fprintf(hfile,
             "\n"
             "#define ISA_OPERAND_TYPES_STATIC_COUNT (%d)\n",
             all_operand_types.size());

     fprintf(efile, 
             "ISA_OPERAND_operand_types\n");
    }

  const char *tabname = gen_static_code ? "ISA_OPERAND_operand_types_static" :
                                          "ISA_OPERAND_operand_types_dynamic";
  fprintf(cfile, "\nstatic ISA_OPERAND_VALTYP %s[] = {\n",tabname);
  for (ivti = all_operand_types.begin(); ivti != all_operand_types.end(); ++ivti) {
    unsigned int       flags;
    bool               is_core_class, 
                       is_core_sub_class;
    OPERAND_VALUE_TYPE val_type = *ivti;

    const ISA_REGISTER_CLASS_INFO *rcinfo = ISA_REGISTER_CLASS_Info(val_type->register_class);
    const ISA_REGISTER_SUBCLASS_INFO *scinfo  = ISA_REGISTER_SUBCLASS_Info(val_type->register_subclass);
    flags = 0;
    if (val_type->is_register) flags |= FLAG_IS_REG;
    if (val_type->is_signed)   flags |= FLAG_IS_SIGNED;
    if (val_type->is_fpu_int)  flags |= FLAG_IS_FPU_INT;
    if (val_type->is_pcrel)    flags |= FLAG_IS_PCREL;
    flag_mask |= flags;

    /* Coherency check - we want to be sure that we fall in one of the
     * two following cases:
     *   - both register class and subclass belong to the core
     *   - both register class and subclass belong to the extension.
     * In the "static" case, we always pass the test (by construction).
     *
     * For the "dynamic" case, we must also take into account the
     * "UNDEFINED" class and subclass whose index has been set to 0.
     */
    is_core_class    =
      val_type->register_class<=ISA_REGISTER_CLASS_STATIC_MAX?true:false;
    is_core_sub_class=
      val_type->register_subclass<=ISA_REGISTER_SUBCLASS_STATIC_MAX?true:false;

    if(is_core_class != is_core_sub_class &&
       val_type->register_subclass != ISA_REGISTER_SUBCLASS_UNDEFINED)
     { const char *errmsg = 
       "### Fatal: cannot have %s registers with %s register subclasses\n";
       const char *regname;
       const char *subregname;

       regname    = ISA_REGISTER_CLASS_INFO_Name(rcinfo);
       subregname = ISA_REGISTER_SUBCLASS_INFO_Name(scinfo);

       if(is_core_class)
          fprintf(stderr,errmsg,"core","extension");
       else
          fprintf(stderr,errmsg,"extension","core");

       fprintf(stderr,"register %s (%d), subregisters %s (%d)\n",
               regname    ? regname   :"<unknown>",
               val_type->register_class,
               subregname ? subregname:"<unknown>",
               val_type->register_subclass);
       exit(EXIT_FAILURE);
     }


    if(is_core_class)              /* Write first part of structure  */
      fprintf(cfile, 
        "  { ISA_REGISTER_CLASS_%-10s, ",
        ISA_REGISTER_CLASS_INFO_Name(rcinfo));
    else
      fprintf(cfile,
        "  { ISA_REGISTER_CLASS_%s_%-10s, ",
        extname,
        ISA_REGISTER_CLASS_INFO_Name(rcinfo));

     if(is_core_sub_class)
       fprintf(cfile,
         "ISA_REGISTER_SUBCLASS_%-10s,\n",
          ISA_REGISTER_SUBCLASS_INFO_Name(scinfo));
     else
       fprintf(cfile,
         "ISA_REGISTER_SUBCLASS_%s_%-10s,\n",
         extname,
         ISA_REGISTER_SUBCLASS_INFO_Name(scinfo));
 
    fprintf(                        /* Write second part of structure */
        cfile,
        "    %3d, %s, %2d, 0x%02x }, /* %s */\n",
        val_type->literal_class,
        ISA_EC_Name(val_type->enum_class),
        val_type->size,
        flags,
        val_type->name);
  }                                /* End of for loop                 */
  fprintf(cfile, "};\n");          /* Print end of table              */

  if(gen_static_code) {
   fprintf(cfile, 
       "\n"
       "BE_EXPORTED ISA_OPERAND_VALTYP *ISA_OPERAND_operand_types = %s;\n",
       tabname); 
   fprintf(hfile, 
       "\n"
       "BE_EXPORTED extern ISA_OPERAND_VALTYP *ISA_OPERAND_operand_types;\n");
  }
  else {
    fprintf(cfile,
       "\n"
       "const ISA_OPERAND_VALTYP *dyn_get_ISA_OPERAND_operand_types_tab( void )\n"
       "{ return %s;\n"
       "}\n"
       "\n"
       "const mUINT32 dyn_get_ISA_OPERAND_operand_types_tab_sz( void )\n"
       "{ return %dU;\n"
       "}\n"
       "\n",
        tabname,static_cast<mUINT32>(all_operand_types.size()) );

    fprintf(hfile,
       "\n\n"
       "extern const ISA_OPERAND_VALTYP *dyn_get_ISA_OPERAND_operand_types_tab(void);\n"
       "extern const mUINT32             dyn_get_ISA_OPERAND_operand_types_tab_sz(void);\n"
       "\n\n");
  }

  max_operands++;
  max_results++;

  if(gen_static_code) {
   fprintf (hfile, "\n"
                   "#define %s (%d)\n"
                   "#define %s (%d)\n"
                   "\n",
                   max_operands_name, max_operands,
                   max_results_name, max_results);

   fprintf (hfile, "\ntypedef struct {\n"
                   "  mUINT8 opnds;\n"
                   "  mUINT8 opnd[%s];\n"
                   "  mUINT16 ouse[%s];\n"
                   "  mUINT8 results;\n"
                   "  mUINT8 result[%s];\n"
                   "  mUINT16 ruse[%s];\n"
                   "  mINT8 same_res[%s];\n"
                   "} ISA_OPERAND_INFO;\n",
                   max_operands_name, max_operands_name, 
                   max_results_name, max_results_name, max_results_name);
    fprintf(efile, "ISA_OPERAND_info\n");

    fprintf(hfile, 
            "\n"
            "#define ISA_OPERAND_INFO_STATIC_COUNT (%d)\n", 
            all_groups.size());
   }

  // --------------------------------------------------------------------
  //
  //       Defining OPERAND_info table.
  //
  // --------------------------------------------------------------------

  tabname = gen_static_code ? "ISA_OPERAND_info_static" :
                              "ISA_OPERAND_info_dynamic";
  fprintf(cfile, "\nstatic ISA_OPERAND_INFO %s[] = {\n",tabname);
  for (ogi = all_groups.begin(); ogi != all_groups.end(); ++ogi) {
    int i;
    int pos;
    vector<OPERAND_VALUE_TYPE>::iterator oper_iter;
    //    vector<OPERAND_USE_TYPE>::iterator use_iter;
    vector<OPERANDS_GROUP_OPERAND_USES>::iterator use_iter;
    vector<mUINT8>::iterator idx_iter;

    OPERANDS_GROUP oper_group = *ogi;

    pos = fprintf(cfile, "  { %d, {", oper_group->opnd_count);
    for (i = 0, oper_iter = oper_group->operands.begin(); 
	 i < max_operands;
	 ++i
    ) {
      int val_type_index = -1;
      if (oper_iter != oper_group->operands.end()) {
        OPERAND_VALUE_TYPE val_type = *oper_iter;
        if (val_type == NULL) {
	  fprintf(stderr, "### Error: operand missing for %s\n", 
                  oper_group->name);
	  exit(EXIT_FAILURE);
        }
	val_type_index = val_type->index;
	++oper_iter;

	if (!val_type->is_register && 
             val_type->literal_class != ISA_LC_UNDEFINED) {

	  /* track the range of operands that can possibly be literal
	   */
	  if (i < first_literal) first_literal = i;
	  if (i > last_literal) last_literal = i;
	}
      }
      pos += fprintf(cfile, "%s%3d", i == 0 ? " " : ", ", val_type_index);
    }
    fprintf(cfile, " },\n");

    pos = fprintf(cfile, "       {");
    for (i = 0, use_iter = oper_group->opnd_use.begin(); 
	 i < max_operands;
	 ++i
    ) {

      if ((oper_group->is_load == true || oper_group->is_store == true) &&
	  (oper_group->base == false || oper_group->offset == false)) {
	fprintf(stderr, "### Error: base/offset missing for %s\n", 
		                                      oper_group->name);
	exit(EXIT_FAILURE);
      }
      if (oper_group->is_store == true && oper_group->storeval == false) {
	fprintf(stderr, "### Error: storeval missing for %s\n", 
		                                      oper_group->name);
	exit(EXIT_FAILURE);
      }

      OPERANDS_GROUP_OPERAND_USES use_type = 0;
      if (use_iter != oper_group->opnd_use.end()) {
	use_type = *use_iter;
	if (use_type) {
	  //	  use_type_index = use_type->index + 1; // +1 for OU_UNDEFINED
	  //	  use_mask |= 1ULL << use_type_index;
	  use_mask |= use_type;
	} else {
	  use_mask |= 1; // OU_UNDEFINED
	}
	++use_iter;
      }

      pos+=fprintf(cfile,"%s", i == 0 ? " " : ", ");
      pos+=print_use_type(cfile,use_type);
    }
    fprintf(cfile, " },\n");

    pos = fprintf(cfile, "    %d, {", oper_group->result_count);
    for (i = 0, oper_iter = oper_group->results.begin(); 
	 i < max_results;
	 ++i
    ) {
      int val_type_index = -1;
      if (oper_iter != oper_group->results.end()) {
        OPERAND_VALUE_TYPE val_type = *oper_iter;
        if (val_type == NULL) {
	  fprintf(stderr, "### Error: result missing for %s\n", oper_group->name);
	  exit(EXIT_FAILURE);
        }
	val_type_index = val_type->index;
	 ++oper_iter;
      }
      pos += fprintf(cfile, "%s%3d", i == 0 ? " " : ", ", val_type_index);
    }
    fprintf(cfile, " },\n");

    //
    // Arthur: print out the results use types
    //
    pos = fprintf(cfile, "       {");
    for (i = 0, use_iter = oper_group->res_use.begin(); 
	 i < max_results;
	 ++i
    ) {
      OPERANDS_GROUP_OPERAND_USES def_type = 0;
      if (use_iter != oper_group->res_use.end()) {
	def_type = *use_iter;
	if (def_type) {
	  def_mask |= def_type;
	} else {
	  def_mask |= 1; // OU_UNDEFINED
	}
	++use_iter;
      }

      pos+=fprintf(cfile,"%s", 0==i ? " " : ", ");/* Put a comma if necessary*/
      pos+=print_use_type(cfile,def_type);
    }
    fprintf(cfile, " },\n");

    //
    // Arthur: print out the same_res table
    //
    pos = fprintf(cfile, "       {");
    for (i = 0, idx_iter = oper_group->same_res.begin(); 
	 i < max_results;
	 ++i
    ) {
      int same_res_index = -1;
      if (idx_iter != oper_group->same_res.end()) {
	if (*idx_iter != 0) {
	  same_res_index = *idx_iter - 1;
	}
	++idx_iter;
      }
      pos += fprintf(cfile, "%s%3d", i == 0 ? " " : ", ", same_res_index);
    }
    fprintf(cfile, " } },%*s/* %s */\n", 50 - (pos + 5), "", oper_group->name);
  }

  fprintf(cfile, "};\n");

  if(gen_static_code)
  { fprintf(cfile, 
      "\n"
      "BE_EXPORTED ISA_OPERAND_INFO *ISA_OPERAND_info = %s;\n",
      tabname);
    fprintf(hfile,
      "\n"
      "BE_EXPORTED extern ISA_OPERAND_INFO *ISA_OPERAND_info;\n");
  }
  else
  { fprintf(cfile,
      "const ISA_OPERAND_INFO  *dyn_get_ISA_OPERAND_info_tab ( void )\n"
      "{ return %s;\n"
      "}\n"
      "\n"
      "const mUINT32 dyn_get_ISA_OPERAND_info_tab_sz( void )\n"
      "{ return %dU;\n"
      "}\n"
      "\n",
      tabname,static_cast<mUINT32>(all_groups.size()) );

    fprintf(hfile,
      "\n"
      "extern const ISA_OPERAND_INFO  *dyn_get_ISA_OPERAND_info_tab    ( void );\n"
      "extern const mUINT32            dyn_get_ISA_OPERAND_info_tab_sz ( void );\n");
  }

  info_index_type = "mUINT16";
  if(max_groups>0xffff)
   { fprintf(stderr," ### Error: number of operands groups exceeds %d\n",
             0xffff);
     exit(EXIT_FAILURE);
   }

  tabname = gen_static_code? "ISA_OPERAND_info_index_static" :
                             "ISA_OPERAND_info_index_dynamic";
  fprintf (cfile, "\nstatic %s %s[] = {\n", info_index_type,tabname);
  for (code = 0; code < TOP_count_limit; code++) {
    OPERANDS_GROUP oper_group = op_groups[code];
    fprintf (cfile, "  %3d,  /* %s: %s */\n",
		    oper_group->index,
		    TOP_Name((TOP)code), 
		    oper_group->name);
  }
  fprintf (cfile, "};\n");

  if(gen_static_code) {
     fprintf (cfile, "\nBE_EXPORTED %s *ISA_OPERAND_info_index = %s;\n", 
              info_index_type,tabname);
    fprintf (hfile,"\nBE_EXPORTED extern %s *ISA_OPERAND_info_index;\n", 
              info_index_type);
    fprintf (efile, "ISA_OPERAND_info_index\n");
   }
  else {
     fprintf(cfile,
       "\n"
       "const %s* dyn_get_ISA_OPERAND_info_index_tab ( void )\n"
       "{ return %s;\n"
       "};\n",
       info_index_type,tabname);
     fprintf(hfile,
       "\n"
       "extern const %s* dyn_get_ISA_OPERAND_info_index_tab(void);\n",
       info_index_type);
   }

  info_reloc_opnd_type = "mINT8";
  tabname = gen_static_code ? "ISA_OPERAND_relocatable_opnd_static":
                              "ISA_OPERAND_relocatable_opnd_dynamic";
  fprintf(cfile, "\nstatic %s %s[] = {\n",info_reloc_opnd_type,tabname);
  for (code = 0; code < TOP_count_limit; code++) {
    OPERANDS_GROUP oper_group = op_groups[code];
    fprintf(cfile, "  %2d,  /* %s */\n",
		   oper_group->relocatable_opnd,
		   TOP_Name((TOP)code));
  }
  fprintf (cfile, "};\n\n");

  if(gen_static_code) {
     fprintf(cfile,
             "BE_EXPORTED %s *ISA_OPERAND_relocatable_opnd = %s;\n",
             info_reloc_opnd_type,tabname);
     fprintf(hfile,
             "\nBE_EXPORTED extern %s *ISA_OPERAND_relocatable_opnd;\n",
	     info_reloc_opnd_type);
     fprintf(efile, "ISA_OPERAND_relocatable_opnd\n");
   }
  else {
    fprintf(cfile,
      "const %s *dyn_get_ISA_OPERAND_relocatable_opnd_tab( void )\n"
      "{ return %s;\n"
      "}\n",
      info_reloc_opnd_type,tabname);
    fprintf(hfile,
      "extern const %s *dyn_get_ISA_OPERAND_relocatable_opnd_tab( void );\n",
      info_reloc_opnd_type);
   }

  // --------------------------------------------------------------------
  //
  //       Definition of inline routines.
  //
  // --------------------------------------------------------------------

  if(gen_static_code) {
  fprintf(hfile, "\ninline const ISA_OPERAND_INFO *"
		   "ISA_OPERAND_Info(TOP topcode)\n"
		 "{\n"
		 "  BE_EXPORTED extern %s *ISA_OPERAND_info_index;\n"
		 "  BE_EXPORTED extern ISA_OPERAND_INFO *ISA_OPERAND_info;\n"
		 "  INT index = ISA_OPERAND_info_index[(INT)topcode];\n"

		 "  return &ISA_OPERAND_info[index];\n"
		 "}\n",
		 info_index_type);

  fprintf(hfile, "\ninline INT ISA_OPERAND_INFO_Operands("
		   "const ISA_OPERAND_INFO *oinfo)\n"
		 "{\n"
		 "  return oinfo->opnds;\n"
		 "}\n");

  fprintf(hfile, "\ninline const ISA_OPERAND_VALTYP *ISA_OPERAND_INFO_Operand(\n"
		 "  const ISA_OPERAND_INFO *oinfo,\n"
		 "  INT opnd)\n"
		 "{\n"
		 "  BE_EXPORTED extern ISA_OPERAND_VALTYP *ISA_OPERAND_operand_types;\n"
		 "  INT index = oinfo->opnd[opnd];\n"
		 "  return &ISA_OPERAND_operand_types[index];\n"
		 "}\n");

  fprintf(hfile, "\ninline INT ISA_OPERAND_INFO_Results("
		   "const ISA_OPERAND_INFO *oinfo)\n"
		 "{\n"
		 "  return oinfo->results;\n"
		 "}\n");

  fprintf(hfile, "\ninline const ISA_OPERAND_VALTYP *ISA_OPERAND_INFO_Result(\n"
		 "  const ISA_OPERAND_INFO *oinfo,\n"
		 "  INT result)\n"
		 "{\n"
		 "  BE_EXPORTED extern ISA_OPERAND_VALTYP *ISA_OPERAND_operand_types;\n"
		 "  INT index = oinfo->result[result];\n"
		 "  return &ISA_OPERAND_operand_types[index];\n"
		 "}\n");

  fprintf(hfile, "\ninline ISA_REGISTER_CLASS ISA_OPERAND_VALTYP_Register_Class(\n"
		 "  const ISA_OPERAND_VALTYP *otype)\n"
		 "{\n"
		 "  return (ISA_REGISTER_CLASS)otype->rclass;\n"
		 "}\n");

  fprintf(hfile, "\ninline ISA_REGISTER_SUBCLASS ISA_OPERAND_VALTYP_Register_Subclass(\n"
		 "  const ISA_OPERAND_VALTYP *otype)\n"
		 "{\n"
		 "  return (ISA_REGISTER_SUBCLASS)otype->rsubclass;\n"
		 "}\n");

  fprintf(hfile, "\ninline ISA_LIT_CLASS ISA_OPERAND_VALTYP_Literal_Class("
		   "const ISA_OPERAND_VALTYP *otype)\n"
		 "{\n"
		 "  return (ISA_LIT_CLASS)otype->lclass;\n"
		 "}\n");

  fprintf(hfile, "\ninline ISA_ENUM_CLASS ISA_OPERAND_VALTYP_Enum_Class(\n"
		 "  const ISA_OPERAND_VALTYP *otype)\n"
		 "{\n"
		 "  return (ISA_ENUM_CLASS)otype->eclass;\n"
		 "}\n");

  fprintf(hfile, "\ninline INT ISA_OPERAND_VALTYP_Size("
		   "const ISA_OPERAND_VALTYP *otype)\n"
		 "{\n"
		 "  return otype->size;\n"
		 "}\n");

  fprintf(hfile, "\ninline BOOL ISA_OPERAND_VALTYP_Is_Register("
		   "const ISA_OPERAND_VALTYP *otype)\n"
		 "{\n"
		 "  return (otype->flags & 0x%02x) != 0;\n"
		 "}\n",
		 FLAG_IS_REG);

  fprintf(hfile, "\ninline BOOL ISA_OPERAND_VALTYP_Is_Literal("
		   "const ISA_OPERAND_VALTYP *otype)\n"
		 "{\n"
		 "  return (otype->lclass != ISA_LC_UNDEFINED);\n"
		 "}\n");

  fprintf(hfile, "\ninline BOOL ISA_OPERAND_VALTYP_Is_Enum("
		   "const ISA_OPERAND_VALTYP *otype)\n"
		 "{\n"
		 "  return (otype->eclass != ISA_EC_UNDEFINED);\n"
		 "}\n");

  fprintf(hfile, "\ninline BOOL ISA_OPERAND_VALTYP_Is_Signed("
		   "const ISA_OPERAND_VALTYP *otype)\n"
		 "{\n"
		 "  return (otype->flags & 0x%02x) != 0;\n"
		 "}\n",
		 FLAG_IS_SIGNED);

  if (flag_mask & FLAG_IS_FPU_INT) {
    fprintf(hfile, "\ninline BOOL ISA_OPERAND_VALTYP_Is_FPU_Int("
		   "const ISA_OPERAND_VALTYP *otype)\n"
		   "{\n"
		   "  return (otype->flags & 0x%02x) != 0;\n"
		   "}\n",
		   FLAG_IS_FPU_INT);
  } else {
    fprintf(hfile, "\n/*ARGSUSED*/\n"
		   "inline BOOL ISA_OPERAND_VALTYP_Is_FPU_Int("
		   "const ISA_OPERAND_VALTYP *otype)\n"
		   "{\n"
		   "  return FALSE;\n"
		   "}\n");
  }

  fprintf(hfile, "\ninline BOOL ISA_OPERAND_VALTYP_Is_PCRel("
		   "const ISA_OPERAND_VALTYP *otype)\n"
		 "{\n"
		 "  return (otype->flags & 0x%02x) != 0;\n"
		 "}\n",
		 FLAG_IS_PCREL);

  fprintf(hfile, "\ninline ISA_OPERAND_USE ISA_OPERAND_INFO_Use(\n"
		 "  const ISA_OPERAND_INFO *oinfo,\n"
		 "  INT opnd)\n"
		 "{\n"
		 "  return (ISA_OPERAND_USE)oinfo->ouse[opnd];\n"
		 "}\n");

  fprintf(hfile, "\ninline ISA_OPERAND_USE ISA_OPERAND_INFO_Def(\n"
		 "  const ISA_OPERAND_INFO *oinfo,\n"
		 "  INT res)\n"
		 "{\n"
		 "  return (ISA_OPERAND_USE)oinfo->ruse[res];\n"
		 "}\n");

  fprintf(hfile, "\ninline mINT8 ISA_OPERAND_INFO_Same_Res(\n"
		 "  const ISA_OPERAND_INFO *oinfo,\n"
		 "  INT res)\n"
		 "{\n"
		 "  return (ISA_OPERAND_USE)oinfo->same_res[res];\n"
		 "}\n");

  fprintf(hfile, "\ninline BOOL ISA_OPERAND_Any_Use(ISA_OPERAND_USE ouse)\n"
		 "{\n"
	  //		 "  return (0x%016llxULL & (1ULL << ouse)) != 0;\n"
		 "  return (0x%016llxULL & ouse) != 0;\n"
		 "}\n",
		 use_mask);

  }                                       /* gen_static_code */

  // For dynamic code generation, we skip the test on literals.
  // We've got to see if this test could be applied as well in
  // such a case.
  if(gen_static_code)
   { if(first_literal >  last_literal) // incorrect if arch has no literals!
      { fprintf(stderr," ### Error: found no literals !\n");
        exit(EXIT_FAILURE);
      }
   }

  // --------------------------------------------------------------------
  //
  //       TOP_Immediate_Operand
  //
  // --------------------------------------------------------------------

  if(gen_static_code) {
  fprintf(hfile, "\nTARGINFO_EXPORTED extern INT TOP_Immediate_Operand(TOP topcode, ISA_LIT_CLASS *lclass);\n");
  fprintf(efile, "TOP_Immediate_Operand\n");
  fprintf(cfile, "\nINT TOP_Immediate_Operand(TOP topcode, ISA_LIT_CLASS *lclass)\n"
		 "{\n"
		 "  INT iopnd;\n"
		 "  const ISA_OPERAND_INFO *opinfo = ISA_OPERAND_Info(topcode);\n"
		 "  INT opnds = ISA_OPERAND_INFO_Operands(opinfo);\n"
		 "  const INT first = 0;\n");
  fprintf(cfile, "\n"
		 "  for (iopnd = first; iopnd < opnds; ++iopnd) {\n"
		 "    const ISA_OPERAND_VALTYP *vtype = ISA_OPERAND_INFO_Operand(opinfo, iopnd);\n"
		 "    ISA_LIT_CLASS lit_class = ISA_OPERAND_VALTYP_Literal_Class(vtype);\n"
		 "    if (lit_class != ISA_LC_UNDEFINED) {\n"
		 "      if (lclass) *lclass = lit_class;\n"
		 "      return iopnd;\n"
		 "    }\n"
		 "  }\n"
		 "\n"
		 "  return -1;\n"
		 "}\n");
  }                                       /* gen_static_code */

  // --------------------------------------------------------------------
  //
  //       TOP_Relocatable_Operand
  //
  // --------------------------------------------------------------------

  if(gen_static_code) {
  fprintf(hfile, "\nTARGINFO_EXPORTED extern INT TOP_Relocatable_Operand(TOP topcode, ISA_LIT_CLASS *lclass);\n");
  fprintf(efile, "TOP_Relocatable_Operand\n");
  fprintf(cfile, "\nINT TOP_Relocatable_Operand(TOP topcode, ISA_LIT_CLASS *lclass)\n"
		 "{\n"
		 "  TARGINFO_EXPORTED extern mINT8 *ISA_OPERAND_relocatable_opnd;\n"
		 "  INT iopnd = ISA_OPERAND_relocatable_opnd[(INT)topcode];\n"
		 "  if (lclass && iopnd >= 0) {\n"
		 "    const ISA_OPERAND_INFO *opinfo = ISA_OPERAND_Info(topcode);\n"
		 "    const ISA_OPERAND_VALTYP *vtype = ISA_OPERAND_INFO_Operand(opinfo,iopnd);\n"
		 "    *lclass = (ISA_LIT_CLASS)ISA_OPERAND_VALTYP_Literal_Class(vtype);\n"
		 "  }\n"
		 "  return iopnd;\n"
		 "}\n");

  fprintf(hfile, "\nBE_EXPORTED extern BOOL TOP_Can_Have_Immediate(INT64 value, TOP topcode);\n");
  fprintf(efile, "TOP_Can_Have_Immediate\n");
  fprintf(cfile, "\nBOOL TOP_Can_Have_Immediate(INT64 value, TOP topcode)\n"
		 "{\n"
		 "  ISA_LIT_CLASS lclass;\n"
		 "  if (TOP_Immediate_Operand(topcode, &lclass) < 0) return %d;\n"
		 "  return ISA_LC_Value_In_Class(value, lclass);\n"
		 "}\n",
		 false);

  }                                       /* gen_static_code */

  // --------------------------------------------------------------------
  //
  //       TOP_Find_Operand_Use
  //
  // --------------------------------------------------------------------

  if(gen_static_code) {
  fprintf(hfile, "\nBE_EXPORTED extern INT TOP_Find_Operand_Use(TOP topcode, "
		 "ISA_OPERAND_USE use);\n");
  fprintf(efile, "TOP_Find_Operand_Use\n");
  fprintf(cfile, "\nINT TOP_Find_Operand_Use(TOP topcode, ISA_OPERAND_USE use)\n"
		 "{\n"
		 "  INT i;\n"
		 "  const ISA_OPERAND_INFO *oinfo = ISA_OPERAND_Info(topcode);\n"
		 "  INT opnds = ISA_OPERAND_INFO_Operands(oinfo);\n"
		 "  for (i = 0; i < opnds; ++i) {\n"
		 "    ISA_OPERAND_USE this_use = ISA_OPERAND_INFO_Use(oinfo, i);\n"
	  //		 "    if (this_use == use) return i;\n"
		 "    if (this_use & use) return i;\n"
		 "  }\n"
		 "  return -1;\n"
		 "}\n");
  }                                       /* gen_static_code */

  // --------------------------------------------------------------------
  //
  //       TOP_Operand_Uses
  //
  // --------------------------------------------------------------------

/*  Is not used anywhere ??
  fprintf(hfile, "\nextern void TOP_Operand_Uses(TOP topcode, "
		 "ISA_OPERAND_USE *uses);\n");
  fprintf(efile, "TOP_Operand_Uses\n");
  fprintf(cfile, "\nvoid TOP_Operand_Uses(TOP topcode, ISA_OPERAND_USE *uses)\n"
		 "{\n"
		 "  INT i;\n"
		 "  const ISA_OPERAND_INFO *oinfo = ISA_OPERAND_Info(topcode);\n"
		 "  INT opnds = ISA_OPERAND_INFO_Operands(oinfo);\n"
		 "  for (i = 0; i < opnds; ++i) {\n"
		 "    ISA_OPERAND_USE this_use = ISA_OPERAND_INFO_Use(oinfo, i);\n"
		 "    uses[i] = this_use;\n"
		 "  }\n"
		 "}\n");
*/

#ifdef TARG_ST
  // --------------------------------------------------------------------
  //
  //       TOP_Find_Result_With_Usage
  //
  // --------------------------------------------------------------------

  if(gen_static_code) {
  fprintf(hfile, "\nBE_EXPORTED extern INT TOP_Find_Result_With_Usage(TOP topcode, "
		 "ISA_OPERAND_USE use);\n");
  fprintf(efile, "TOP_Find_Result_With_Usage\n");
  fprintf(cfile, "\nINT TOP_Find_Result_With_Usage(TOP topcode, ISA_OPERAND_USE use)\n"
		 "{\n"
		 "  INT i;\n"
		 "  const ISA_OPERAND_INFO *oinfo = ISA_OPERAND_Info(topcode);\n"
		 "  INT results = ISA_OPERAND_INFO_Results(oinfo);\n"
		 "  for (i = 0; i < results; ++i) {\n"
		 "    ISA_OPERAND_USE this_use = ISA_OPERAND_INFO_Def(oinfo, i);\n"
		 "    if (this_use & use) return i;\n"
		 "  }\n"
		 "  return -1;\n"
		 "}\n");

   
  }                                       /* gen_static_code */
#endif

  // --------------------------------------------------------------------
  //
  //       TOP_Same_Res_Operand
  //
  // --------------------------------------------------------------------

  if(gen_static_code) {
  fprintf(hfile, "\nTARGINFO_EXPORTED extern mINT8 TOP_Same_Res_Operand(TOP topcode, mUINT8 residx);\n");
  fprintf(efile, "TOP_Same_Res_Operand\n");
  fprintf(cfile, "\nmINT8 TOP_Same_Res_Operand(TOP topcode, mUINT8 residx)\n"
		 "{\n"
		 "  const ISA_OPERAND_INFO *oinfo = ISA_OPERAND_Info(topcode);\n"
		 "  return ISA_OPERAND_INFO_Same_Res(oinfo, residx);\n"
		 "}\n");

  fprintf(hfile, "\nTARGINFO_EXPORTED extern BOOL TOP_Result_Is_Uniq_Res(TOP topcode, mUINT8 residx);\n");
  fprintf(efile, "TOP_Result_Is_Uniq_Res\n");
  fprintf(cfile, "\nBOOL TOP_Result_Is_Uniq_Res(TOP topcode, mUINT8 residx)\n"
		 "{\n"
		 "  const ISA_OPERAND_INFO *oinfo = ISA_OPERAND_Info(topcode);\n"
		 "  ISA_OPERAND_USE this_def = ISA_OPERAND_INFO_Def(oinfo, residx);\n"
		 "  if (this_def & OU_uniq_res) return TRUE;\n"
		 "  return FALSE;\n"
		 "}\n");
  }                                       /* gen_static_code */


  // --------------------------------------------------------------------
  //
  //      ISA OPERAND USE attributes table and access functions
  //
  // Used in reconfigurability to check Core/Extension targinfo
  // compatibility and potentially allow attribute id remapping 
  // when loading extensions.
  //
  // --------------------------------------------------------------------

  fprintf (hfile,
	   "\n"
	   "/* Description of Operand use attributes */\n");
  fprintf (cfile,
	   "\n"
	   "/* ============================================================\n"
	   " *  ISA OPERAND USE attribute description\n"
	   " * ============================================================\n"
	   " */\n");

  // Generate datatype specification
  if (gen_static_code) {
    fprintf(hfile,
	    "typedef struct { const char *name; mUINT32 attribute; } ISA_OPERAND_USE_ATTRIBUTE;\n");
  }
  
  // Generate attribute description table
  fprintf (cfile, "static const ISA_OPERAND_USE_ATTRIBUTE ISA_OPERAND_USE_attribute_tab[] = {\n");
  for ( iuti = all_use_types.begin(); iuti != all_use_types.end(); ++iuti) {
    OPERAND_USE_TYPE use_type = *iuti;
    fprintf (cfile, "  { \"%s\", OU_%s},\n", use_type->name, use_type->name);
  }
  fprintf (cfile, "};\n\n");
  
  // Generate access to attribute description table
  if (gen_static_code) {
    fprintf (
      hfile,
     "#define ISA_OPERAND_USE_ATTRIBUTE_COUNT (%d)\n", all_use_types.size());

    fprintf (
      hfile,
     "BE_EXPORTED extern const ISA_OPERAND_USE_ATTRIBUTE * ISA_OPERAND_USE_get_attribute_table();\n"
     "\n");

    fprintf (
      cfile,
     "BE_EXPORTED const ISA_OPERAND_USE_ATTRIBUTE * ISA_OPERAND_USE_get_attribute_table() {\n"
     "  return (ISA_OPERAND_USE_attribute_tab);\n"
     "}\n"
     "\n");

    fprintf(efile, "ISA_OPERAND_USE_get_attribute_table\n");
  }
  else {
    fprintf (hfile,
	     "extern const mUINT32                    dyn_get_ISA_OPERAND_USE_attribute_tab_sz();\n"
	     "extern const ISA_OPERAND_USE_ATTRIBUTE* dyn_get_ISA_OPERAND_USE_attribute_tab();\n"
	     "\n");
    fprintf (cfile,
	     "\n"
	     "const mUINT32 dyn_get_ISA_OPERAND_USE_attribute_tab_sz() {\n"
	     "  return (%d);\n"
	     "}\n"
	     "\n", all_use_types.size());
    fprintf (cfile,
	     "const ISA_OPERAND_USE_ATTRIBUTE* dyn_get_ISA_OPERAND_USE_attribute_tab() {\n"
	     "  return (ISA_OPERAND_USE_attribute_tab);\n"
	     "}\n"
	     "\n");
  }


#ifdef DYNAMIC_CODE_GEN
  // --------------------------------------------------------------------
  //
  //       Static Register classes & subclasses info
  //
  // Used in reconfigurability to check Core/Extension targinfo
  // compatibility and potentially allow register classes id &
  // register subclasses id remapping when loading extensions.
  //
  // --------------------------------------------------------------------
  if (!gen_static_code) {
    int i;
    /* 
     * Generate a table of register class name, indexed by register class id.
     * The table only contains static register classes (ones defined for the
     * core) and contains a NULL pointer when the corresponding register class
     * is not referenced by operands.
     */
    fprintf(cfile,
	    "\n"
	    "/* Access to static reg class names indexed by their id      */\n"
	    "/* (Names are provided only for ones referenced in operands) */\n"
	    "static const char * ISA_OPERAND_static_rclass_tab[] = {\n"
	    "  0, /* 'UNDEFINED' */\n");
    for (i = 1; i <= ISA_REGISTER_CLASS_STATIC_MAX; i++) {
      if (referenced_core_reg_classes[i]) {
	fprintf(cfile,
		"  \"%s\",\n",
		ISA_REGISTER_CLASS_INFO_Name(ISA_REGISTER_CLASS_Info(i)));
      }
      else {
	fprintf(cfile,
		"  0, /* register class '%s' not used in extension */\n",
		ISA_REGISTER_CLASS_INFO_Name(ISA_REGISTER_CLASS_Info(i)));
      }
    }
    fprintf(cfile,
	    "};\n\n");

    fprintf(cfile,
	    "const char* const *dyn_get_ISA_OPERAND_static_rclass_tab( void ) {\n"
	    "  return (ISA_OPERAND_static_rclass_tab);\n"
	    "}\n"
	    "\n"
	    "const mUINT32 dyn_get_ISA_OPERAND_static_rclass_tab_sz( void )\n"
	    "{ return %dU;\n"
	    "}\n"
	    "\n",
	    ISA_REGISTER_CLASS_STATIC_MAX+1);

    fprintf(hfile,
	    "\n\n"
	    "/* Access to static reg class names indexed by their id */\n"
	    "extern const char* const *dyn_get_ISA_OPERAND_static_rclass_tab(void);\n"
	    "extern const mUINT32      dyn_get_ISA_OPERAND_static_rclass_tab_sz(void);\n"
	    "\n");
    /* 
     * Generate a table of register subclasses name, indexed by register subclass id.
     * The table only contains static register subclasses (ones defined for the
     * core) and contains a NULL pointer when the corresponding register subclass
     * is not referenced by operands.
     */
    fprintf(cfile,
	    "\n"
	    "/* Access to static reg subclass names indexed by their id   */\n"
	    "/* (Names are provided only for ones referenced in operands) */\n"
	    "static const char * ISA_OPERAND_static_rsubclass_tab[] = {\n"
	    "  0, /* 'UNDEFINED' */\n");
    for (i = 1; i <= ISA_REGISTER_SUBCLASS_STATIC_MAX; i++) {
      if (referenced_core_reg_subclasses[i]) {
	fprintf(cfile,
		"  \"%s\",\n",
		ISA_REGISTER_SUBCLASS_INFO_Name(ISA_REGISTER_SUBCLASS_Info(i)));
      }
      else {
	fprintf(cfile,
		"  0, /* register subclass '%s' not used in extension */\n",
		ISA_REGISTER_SUBCLASS_INFO_Name(ISA_REGISTER_SUBCLASS_Info(i)));
      }
    }
    fprintf(cfile,
	    "};\n\n");

    fprintf(cfile,
	    "const char* const *dyn_get_ISA_OPERAND_static_rsubclass_tab( void ) {\n"
	    "  return (ISA_OPERAND_static_rsubclass_tab);\n"
	    "}\n"
	    "\n"
	    "const mUINT32 dyn_get_ISA_OPERAND_static_rsubclass_tab_sz( void )\n"
	    "{ return %dU;\n"
	    "}\n"
	    "\n",
	    ISA_REGISTER_SUBCLASS_STATIC_MAX+1);

    fprintf(hfile,
	    "\n"
	    "/* Access to static reg subclass names indexed by their id */\n"
	    "extern const char* const *dyn_get_ISA_OPERAND_static_rsubclass_tab(void);\n"
	    "extern const mUINT32      dyn_get_ISA_OPERAND_static_rsubclass_tab_sz(void);\n"
	    "\n\n");
  }
  
#endif


  Emit_Footer  (hfile);
  Emit_C_Footer(cfile);

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
