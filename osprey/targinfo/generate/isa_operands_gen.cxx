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


//  isa_operands_gen.cxx
/////////////////////////////////////
//
//  Interface for specifying operands and results for various
//  instructions in the ISA.
//
/////////////////////////////////////
//
//  $Revision$
//  $Date$
//  $Author$
//  $Source$

typedef struct operand_value_type *OPERAND_VALUE_TYPE;

#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
#include <list.h>
#include <vector.h>
#include <algo.h>
#include "topcode.h"
#include "targ_isa_properties.h"
#include "gen_util.h"
#include "isa_operands_gen.h"


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

struct operand_use_type {
  const char *name;
  int index;
};

static vector <OPERANDS_GROUP> op_groups;
static OPERANDS_GROUP cur_oper_group;
static list <OPERAND_VALUE_TYPE> all_operand_types;
static list <OPERAND_USE_TYPE> all_use_types;
static list <OPERANDS_GROUP> all_groups; // All the instruction groups

static int max_operands = 0;
static int max_results = 0;
static int max_valtypes = 0;
static int max_groups = 0;
static int max_uses = 0;

static int cur_res_index;

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
  " *",
  " * ====================================================================",
  " * ====================================================================",
  " */",
  NULL
};


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
  op_groups = vector <OPERANDS_GROUP> (TOP_count, (OPERANDS_GROUP) false);

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
    fprintf(stderr, "### Error: RTYPE for register operand %s must be SIGNED or UNSIGNED\n",
		    name);
    exit(EXIT_FAILURE);
  }

  OPERAND_VALUE_TYPE result = new operand_value_type;

  all_operand_types.push_back(result);

  result->name = name;
  result->register_class = register_class;
  result->register_subclass = subclass;
  result->literal_class = LC_UNDEFINED;
  result->enum_class = EC_UNDEFINED;
  result->size = size;
  result->is_register = true;
  result->is_signed = type == SIGNED;
  result->is_pcrel = false;
  result->is_fpu_int = fp_int != INVALID;
  result->index = max_valtypes++;

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
  result->enum_class = EC_UNDEFINED;
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
  result->literal_class = LC_UNDEFINED;
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
  if ((opcode = static_cast<TOP>(va_arg(ap,int))) != TOP_UNDEFINED ) {
    if (op_groups[(int)opcode]) {
      fprintf(stderr, 
	      "### Error: Instruction_Group %s: redefines group (%s) for %s\n",
	      name, op_groups[(int)opcode]->name, TOP_Name(opcode));
    }
    if (TOP_is_load(opcode)) oper_group->is_load = true;
    if (TOP_is_store(opcode)) oper_group->is_store = true;
    op_groups[(int)opcode] = oper_group;
  }

  while ( (opcode = static_cast<TOP>(va_arg(ap,int))) != TOP_UNDEFINED ) {
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
  if (operand_index > max_operands) max_operands = operand_index;

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
    cur_oper_group->opnd_use[operand_index] |= 1 << (operand_use->index + 1);
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
  if (result_index > max_results) max_results = result_index;

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
    cur_oper_group->res_use[result_index] |= 1 << (result_use->index + 1);
  }

  //  cur_oper_group->res_use[result_index] = (OPERANDS_GROUP_OPERAND_USES)NULL;
  cur_res_index = result_index;
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
  list<OPERAND_VALUE_TYPE>::iterator ivti;
  list<OPERANDS_GROUP>::iterator ogi;
  list<OPERAND_USE_TYPE>::iterator iuti;
  int code;
  bool err;
  const char *info_index_type;
  int first_literal = max_operands;
  int last_literal = -1;
  int flag_mask = 0;
  unsigned long long use_mask = 0;
  unsigned long long def_mask = 0;
  const char *max_operands_name = "ISA_OPERAND_max_operands";
  const char *max_results_name = "ISA_OPERAND_max_results";
  enum {
    FLAG_IS_REG	    = 0x1,
    FLAG_IS_SIGNED  = 0x2,
    FLAG_IS_FPU_INT = 0x4,
    FLAG_IS_PCREL   = 0x8
  };

  if (max_uses > 32 - 1) {
    fprintf(stderr, "###Error: can't handle > 31 (%d) OPERAND_USE_TYPEs\n",
		    max_uses);
    exit(EXIT_FAILURE);
  }

  for (err = false, code = 0; code < TOP_count; ++code) {
    if (!op_groups[code]) {
      fprintf (stderr, "###Error: no specification for opcode: %s\n",
		       TOP_Name((TOP)code));
      err = true;
    }
  }
  if (err) exit(EXIT_FAILURE);

#define FNAME "targ_isa_operands"
  char filename[1000];
  sprintf (filename, "%s.h", FNAME);
  FILE* hfile = fopen(filename, "w");
  sprintf (filename, "%s.c", FNAME);
  FILE* cfile = fopen(filename, "w");
  sprintf (filename, "%s.Exported", FNAME);
  FILE* efile = fopen(filename, "w");
  int maxenum;

  fprintf(cfile,"#include \"%s.h\"\n", FNAME);
  fprintf(cfile,"#include \"targ_isa_registers.h\"\n");
  fprintf(cfile,"#include \"targ_isa_properties.h\"\n");
  fprintf(cfile,"#include \"targ_isa_lits.h\"\n\n");

  Emit_Header (hfile, "targ_isa_operands", interface);

  fprintf(hfile, "#include \"topcode.h\"\n");
  fprintf(hfile, "#include \"targ_isa_registers.h\"\n");
  fprintf(hfile, "#include \"targ_isa_enums.h\"\n");
  fprintf(hfile, "#include \"targ_isa_lits.h\"\n");

  //  fprintf(hfile, "\ntypedef enum {\n"
  //		 "  OU_UNDEFINED,\n");

  fprintf(hfile, "\ntypedef UINT32 ISA_OPERAND_USE;\n");
  fprintf(hfile, "\n#define OU_UNDEFINED\t 0x00000000\n");

  for (maxenum = 0, iuti = all_use_types.begin(); 
       iuti != all_use_types.end(); 
       ++maxenum, ++iuti)
  {
    OPERAND_USE_TYPE use_type = *iuti;
    fprintf(hfile, "#define OU_%s\t 0x%08x\n", use_type->name,
                                       1 << (use_type->index + 1));
  }
  //  fprintf(hfile, "  OU_MAX = %d\n"
  fprintf(hfile, "#define OU_MAX 0x%08x\n", 1 << maxenum);
	  //		 "} ISA_OPERAND_USE;\n",
	  //		 maxenum);

  fprintf(hfile, "\ntypedef struct {\n"
		 "  mUINT8 rclass;\n"
		 "  mUINT8 rsubclass;\n"
		 "  mUINT8 lclass;\n"
		 "  mUINT8 eclass;\n"
		 "  mUINT8 size;\n"
		 "  mUINT8 flags;\n"
		 "} ISA_OPERAND_VALTYP;\n");

  fprintf(efile, "ISA_OPERAND_operand_types\n");

  fprintf(cfile, "\nconst ISA_OPERAND_VALTYP ISA_OPERAND_operand_types[] = {\n");
  for (ivti = all_operand_types.begin(); ivti != all_operand_types.end(); ++ivti) {
    unsigned int flags;
    OPERAND_VALUE_TYPE val_type = *ivti;
    const ISA_REGISTER_CLASS_INFO *rcinfo = ISA_REGISTER_CLASS_Info(val_type->register_class);
    const ISA_REGISTER_SUBCLASS_INFO *scinfo = ISA_REGISTER_SUBCLASS_Info(val_type->register_subclass);
    flags = 0;
    if (val_type->is_register) flags |= FLAG_IS_REG;
    if (val_type->is_signed) flags |= FLAG_IS_SIGNED;
    if (val_type->is_fpu_int) flags |= FLAG_IS_FPU_INT;
    if (val_type->is_pcrel) flags |= FLAG_IS_PCREL;
    flag_mask |= flags;
    fprintf(cfile, "  { ISA_REGISTER_CLASS_%-10s, ISA_REGISTER_SUBCLASS_%-10s,\n"
		   "    %3d, %s, %2d, 0x%02x }, /* %s */\n",
		   ISA_REGISTER_CLASS_INFO_Name(rcinfo),
		   ISA_REGISTER_SUBCLASS_INFO_Name(scinfo),
		   val_type->literal_class,
		   ISA_EC_Name(val_type->enum_class),
		   val_type->size,
		   flags,
		   val_type->name);
  }
  fprintf(cfile, "};\n");

  max_operands++;
  max_results++;
  fprintf (hfile, "\nenum {\n"
		  "  %s=%d,\n"
		  "  %s=%d\n"
		  "};\n",
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

  fprintf(cfile, "\nconst ISA_OPERAND_INFO ISA_OPERAND_info[] = {\n");
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
	  fprintf(stderr, "### Error: operand missing for %s\n", oper_group->name);
	  exit(EXIT_FAILURE);
        }
	val_type_index = val_type->index;
	++oper_iter;

	if (!val_type->is_register && val_type->literal_class != LC_UNDEFINED) {

	  /* track the range of operands that can possibly be literal
	   */
	  if (i < first_literal) first_literal = i;
	  if (i > last_literal) last_literal = i;
	}
      }
      pos += fprintf(cfile, "%s%3d", i == 0 ? " " : ", ", val_type_index);
    }
#if 0
    fprintf(cfile, " },%*s/* %s */\n", 50 - (pos + 3), "", oper_group->name);
#endif
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
      int use_type_index = 0;
      if (use_iter != oper_group->opnd_use.end()) {
	//	OPERAND_USE_TYPE use_type = *use_iter;
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
      //      pos += fprintf(cfile, "%s%3d", i == 0 ? " " : ", ", use_type_index);
      pos += fprintf(cfile, "%s0x%08x", i == 0 ? " " : ", ", use_type);
    }
#if 0
    fprintf(cfile, " },%*s/* %s */\n", 50 - (pos + 3), "", oper_group->name);
#endif
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
    //    fprintf(cfile, " } },%*s/* %s */\n", 50 - (pos + 5), "", oper_group->name);
#if 0
    fprintf(cfile, " }, %*s/* %s */\n", 50 - (pos + 5), "", oper_group->name);
#endif
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
      int def_type_index = 0;
      if (use_iter != oper_group->res_use.end()) {
	def_type = *use_iter;
	if (def_type) {
	  def_mask |= def_type;
	} else {
	  def_mask |= 1; // OU_UNDEFINED
	}
	++use_iter;
      }
      pos += fprintf(cfile, "%s0x%08x", i == 0 ? " " : ", ", def_type);
    }
#if 0
    fprintf(cfile, " },%*s/* %s */\n", 50 - (pos + 3), "", oper_group->name);
#endif
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

  info_index_type = max_groups < 256 ? "mUINT8" : "mUINT16";
  assert(max_groups <= 0xffff);

  fprintf (efile, "ISA_OPERAND_info_index\n");
  fprintf (cfile, "\nconst %s ISA_OPERAND_info_index[] = {\n", info_index_type);
  for (code = 0; code < TOP_count; code++) {
    OPERANDS_GROUP oper_group = op_groups[code];
    fprintf (cfile, "  %3d,  /* %s: %s */\n",
		    oper_group->index,
		    TOP_Name((TOP)code), 
		    oper_group->name);
  }
  fprintf (cfile, "};\n");

  fprintf(efile, "ISA_OPERAND_relocatable_opnd\n");
  fprintf(cfile, "\nconst mINT8 ISA_OPERAND_relocatable_opnd[] = {\n");
  for (code = 0; code < TOP_count; code++) {
    OPERANDS_GROUP oper_group = op_groups[code];
    fprintf(cfile, "  %2d,  /* %s */\n",
		   oper_group->relocatable_opnd,
		   TOP_Name((TOP)code));
  }
  fprintf (cfile, "};\n");

  fprintf(hfile, "\ninline const ISA_OPERAND_INFO *"
		   "ISA_OPERAND_Info(TOP topcode)\n"
		 "{\n"
		 "  extern const %s ISA_OPERAND_info_index[];\n"
		 "  extern const ISA_OPERAND_INFO ISA_OPERAND_info[];\n"
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
		 "  extern const ISA_OPERAND_VALTYP ISA_OPERAND_operand_types[];\n"
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
		 "  extern const ISA_OPERAND_VALTYP ISA_OPERAND_operand_types[];\n"
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
		 "  return (otype->lclass != LC_UNDEFINED);\n"
		 "}\n");

  fprintf(hfile, "\ninline BOOL ISA_OPERAND_VALTYP_Is_Enum("
		   "const ISA_OPERAND_VALTYP *otype)\n"
		 "{\n"
		 "  return (otype->eclass != EC_UNDEFINED);\n"
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

  assert(first_literal <= last_literal); // incorrect if arch has no literals!

  // --------------------------------------------------------------------
  //
  //       TOP_Immediate_Operand
  //
  // --------------------------------------------------------------------

  fprintf(hfile, "\nextern INT TOP_Immediate_Operand(TOP topcode, ISA_LIT_CLASS *lclass);\n");
  fprintf(efile, "TOP_Immediate_Operand\n");
  fprintf(cfile, "\nINT TOP_Immediate_Operand(TOP topcode, ISA_LIT_CLASS *lclass)\n"
		 "{\n"
		 "  INT iopnd;\n"
		 "  const ISA_OPERAND_INFO *opinfo = ISA_OPERAND_Info(topcode);\n"
		 "  INT opnds = ISA_OPERAND_INFO_Operands(opinfo);\n"
		 "  const INT first = %d;\n",
		 first_literal);
  if (last_literal != max_operands - 1) {
    fprintf(cfile, "  const INT last = %d;\n"
		   "\n"
		   "  if (last + 1 < opnds) opnds = last + 1;\n",
		   last_literal);
  }
  fprintf(cfile, "\n"
		 "  for (iopnd = first; iopnd < opnds; ++iopnd) {\n"
		 "    const ISA_OPERAND_VALTYP *vtype = ISA_OPERAND_INFO_Operand(opinfo, iopnd);\n"
		 "    ISA_LIT_CLASS lit_class = ISA_OPERAND_VALTYP_Literal_Class(vtype);\n"
		 "    if (lit_class != LC_UNDEFINED) {\n"
		 "      if (lclass) *lclass = lit_class;\n"
		 "      return iopnd;\n"
		 "    }\n"
		 "  }\n"
		 "\n"
		 "  return -1;\n"
		 "}\n");

  // --------------------------------------------------------------------
  //
  //       TOP_Relocatable_Operand
  //
  // --------------------------------------------------------------------

  fprintf(hfile, "\nextern INT TOP_Relocatable_Operand(TOP topcode, ISA_LIT_CLASS *lclass);\n");
  fprintf(efile, "TOP_Relocatable_Operand\n");
  fprintf(cfile, "\nINT TOP_Relocatable_Operand(TOP topcode, ISA_LIT_CLASS *lclass)\n"
		 "{\n"
		 "  extern const mINT8 ISA_OPERAND_relocatable_opnd[];\n"
		 "  INT iopnd = ISA_OPERAND_relocatable_opnd[(INT)topcode];\n"
		 "  if (lclass && iopnd >= 0) {\n"
		 "    const ISA_OPERAND_INFO *opinfo = ISA_OPERAND_Info(topcode);\n"
		 "    const ISA_OPERAND_VALTYP *vtype = ISA_OPERAND_INFO_Operand(opinfo,iopnd);\n"
		 "    *lclass = (ISA_LIT_CLASS)ISA_OPERAND_VALTYP_Literal_Class(vtype);\n"
		 "  }\n"
		 "  return iopnd;\n"
		 "}\n");

  fprintf(hfile, "\nextern BOOL TOP_Can_Have_Immediate(INT64 value, TOP topcode);\n");
  fprintf(efile, "TOP_Can_Have_Immediate\n");
  fprintf(cfile, "\nBOOL TOP_Can_Have_Immediate(INT64 value, TOP topcode)\n"
		 "{\n"
		 "  ISA_LIT_CLASS lclass;\n"
		 "  if (TOP_Immediate_Operand(topcode, &lclass) < 0) return %d;\n"
		 "  return ISA_LC_Value_In_Class(value, lclass);\n"
		 "}\n",
		 false);

  // --------------------------------------------------------------------
  //
  //       TOP_Find_Operand_Use
  //
  // --------------------------------------------------------------------

  fprintf(hfile, "\nextern INT TOP_Find_Operand_Use(TOP topcode, "
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

  // --------------------------------------------------------------------
  //
  //       TOP_Same_Res_Operand
  //
  // --------------------------------------------------------------------

  fprintf(hfile, "\nextern mINT8 TOP_Same_Res_Operand(TOP topcode, mUINT8 residx);\n");
  fprintf(efile, "TOP_Same_Res_Operand\n");
  fprintf(cfile, "\nmINT8 TOP_Same_Res_Operand(TOP topcode, mUINT8 residx)\n"
		 "{\n"
		 "  const ISA_OPERAND_INFO *oinfo = ISA_OPERAND_Info(topcode);\n"
		 "  return ISA_OPERAND_INFO_Same_Res(oinfo, residx);\n"
		 "}\n");

  fprintf(hfile, "\nextern BOOL TOP_Result_Is_Uniq_Res(TOP topcode, mUINT8 residx);\n");
  fprintf(efile, "TOP_Result_Is_Uniq_Res\n");
  fprintf(cfile, "\nBOOL TOP_Result_Is_Uniq_Res(TOP topcode, mUINT8 residx)\n"
		 "{\n"
		 "  const ISA_OPERAND_INFO *oinfo = ISA_OPERAND_Info(topcode);\n"
		 "  ISA_OPERAND_USE this_def = ISA_OPERAND_INFO_Def(oinfo, residx);\n"
		 "  if (this_def & OU_uniq_res) return TRUE;\n"
		 "  return FALSE;\n"
		 "}\n");

  Emit_Footer (hfile);
}
