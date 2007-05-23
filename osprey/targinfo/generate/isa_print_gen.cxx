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
 * 
 * This file has been modified by STMicroelectronics
 *
 */


// isa_print_gen.cxx
/////////////////////////////////////
//
//  Generate an interface for printing the instructions in an ISA.
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

#ifdef DYNAMIC_CODE_GEN
#include "dyn_isa_topcode.h"   /* Dynamic TOPcode */
#else
#include "topcode.h"
#endif

#include "targ_isa_properties.h"
#include "targ_isa_operands.h"
#include "gen_util.h"
#include "isa_print_gen.h"

// In following loops, we iterate on the number of
// TOP. This number differs whether we generate
// static or dynamic TOPs.
#ifndef DYNAMIC_CODE_GEN
static TOP TOP_count_limit = TOP_static_count;
#else
static TOP TOP_count_limit = TOP_dyn_count;
#endif


/* The maximum number of operands and results, retrieved from
 * generated targ_isa_operands.h file
 */
#define MAX_OPNDS   (ISA_OPERAND_max_operands)
#define MAX_RESULTS (ISA_OPERAND_max_results)

typedef enum {
	END	= 0,			// end of list marker
	NAME	= 1,			// instruction name/mnemonic
	OPND    = 2,			// OPND+n => operand n
   	RESULT  = OPND+MAX_OPNDS,	// RESULT+n => result n
} COMP_TYPE;

#define MAX_LISTING_OPERANDS (RESULT+MAX_RESULTS)

struct isa_print_type {
  const char *name;         // Name given for documentation and debugging
  const char *format_string; // format string to print this print type
};

struct list_info {
  isa_print_type *type;
  unsigned char	 args;  // Number of sprintf arguments
  unsigned char	 arg[MAX_LISTING_OPERANDS]; 
  //char *fmt[MAX_LISTING_OPERANDS]; 
  int index;		
  bool have_name;
};

typedef list_info *LISTING_INFO;

// map to link TOPs_ with list_info properties
struct op_pr {
  list_info *desc;
  struct op_pr *next;
};

static list<LISTING_INFO> all_prints;  // all the different print formats
static LISTING_INFO current_print_desc;

//static op_pr *op_prints[TOP_count+1];
//static bool top_specified[TOP_count];
static op_pr **op_prints     = NULL;    // Dynamic mem. alloc in ISA_Print_Begin
static bool   *top_specified = NULL;    // Dynamic mem. alloc in ISA_Print_Begin

static list<op_pr*> op_prints_list;
static int list_index;
static const char *(*asmname)(TOP topcode);

static const char * const interface[] = {
  "/* ====================================================================",
  " * ====================================================================",
  " *",
  " * Description:",
  " *",
  " *   A description of how to print the operands of ISA instructions",
  " *   in ascii. The description exports the following:",
  " *",
  " *   typedef (enum) ISA_PRINT_COMP",
  " *       An enumeration of the instruction components to be printed.",
  " *",
  " *   typedef (struct) ISA_PRINT_INFO",
  " *       Describes how one particular instruction is printed.",
  " *       The contents are private.",
  " *",
  " *   const INT ISA_PRINT_COMP_MAX",
  " *       The maximum number of components to be printed for any instruction.",
  " *",
  " *   const ISA_PRINT_INFO *ISA_PRINT_Info(TOP topcode)",
  " *       Returns a pointer to the printing description for the",
  " *       instruction specified by 'topcode'.",
  " *",
  " *       The instruction is printed by calling one of the printf routines",
  " *       with the format string returned from ISA_PRINT_INFO_Format.",
  " *       Additional printf arguments, necessitated by the format string,",
  " *       are described by ISA_PRINT_INFO_Comp.",
  " *",
  " *   INT ISA_PRINT_INFO_Comp(const ISA_PRINT_INFO *info, INT index)",
  " *       Identifies a instruction component to be printed.",
  " *",
  " *       'index' specifies the component. The first component has index",
  " *       0; the end of the components is signalled by the return of",
  " *       ISA_PRINT_COMP_end.",
  " *",
  " *   const char *ISA_PRINT_INFO_Format(const ISA_PRINT_INFO *info)",
  " *       The printf format string for printing the instruction",
  " *       described by 'info'.",
  " *",
  " *   const char *ISA_PRINT_AsmName(TOP topcode)",
  " *       Returns the assembly language name for <topcode>.",
  " *",
  " *   BOOL ISA_PRINT_Operand_Is_Part_Of_Name(TOP topcode, INT opindex)",
  " *       Returns whether the operand is part of the full asm name.",
  " *",
  " * ====================================================================",
  " * ====================================================================",
  " */",
  NULL
};

/////////////////////////////////////
const char* Print_Name(int print_index)
/////////////////////////////////////
{
  static char *comp_name[MAX_LISTING_OPERANDS];
  static bool initialized;

  if (!initialized) {
    int i;
    for (i = 0; i < MAX_LISTING_OPERANDS; ++i) {
      char buf[80];
      if (i == END) {
	comp_name[i] = "ISA_PRINT_COMP_end";
      } else if (i == NAME) {
	comp_name[i] = "ISA_PRINT_COMP_name";
      } else if (i == OPND) {
	comp_name[i] = "ISA_PRINT_COMP_opnd";
      } else if (i > OPND && i < (OPND + MAX_OPNDS)) {
	sprintf(buf, "ISA_PRINT_COMP_opnd+%d", i - OPND);
	comp_name[i] = strdup(buf);
      } else if (i == RESULT) {
	comp_name[i] = "ISA_PRINT_COMP_result";
      } else {
	assert(i > RESULT && i < (RESULT + MAX_RESULTS));
	sprintf(buf, "ISA_PRINT_COMP_result+%d", i - RESULT);
	comp_name[i] = strdup(buf);
      }
    }
    initialized = true;
  }

  return comp_name[print_index];
}

/////////////////////////////////////
void ISA_Print_Begin( const char* /* name */ )
/////////////////////////////////////
//  See interface description.
/////////////////////////////////////
{
   // Memory allocation
   op_prints     = new op_pr* [TOP_count_limit+1];
   for (unsigned int i=0; i<TOP_count_limit+1; i++) op_prints[i] = NULL;
   top_specified = new bool   [TOP_count_limit  ];
   for (unsigned int i=0; i<TOP_count_limit; i++) top_specified[i] = false;

}

/////////////////////////////////////
ISA_PRINT_TYPE ISA_Print_Type_Create ( 
  const char* name, 
  const char* format_string)
/////////////////////////////////////
//  See interface description.
/////////////////////////////////////
{
  LISTING_INFO result = new list_info;
  result->type = new isa_print_type;
  result->type->name = name;
  result->type->format_string = format_string;
  result->index = list_index;
  result->args = 0;
  result->have_name = false;
  current_print_desc = result;
  all_prints.push_back (current_print_desc);
  ++list_index;
  return result->type;
}

/////////////////////////////////////
void Instruction_Print_Group(ISA_PRINT_TYPE print_type, TOP top, ... )
/////////////////////////////////////
//  See interface description.
/////////////////////////////////////
{
  va_list ap;
  TOP     opcode;

  // For static code, variable length list
  // halts on TOP_UNDEFINED. For dynamic code,
  // variable length list ends with -1.
  TOP     stop = Is_Static_Code() ? TOP_UNDEFINED : static_cast<TOP> (-1);

  // An additional check to memory has been allocated.
  if(NULL==op_prints || NULL==top_specified)
   { fprintf(stderr,"### Fatal error: internal table op_print not initialized\n");
     fprintf(stderr,"### Please check that function \"%s\" has been called previously\n",
             "ISA_Print_Begin" );
     exit(EXIT_FAILURE);
   }

  if (!current_print_desc->have_name) {
    fprintf(stderr, "### Warning: no instruction name specified for %s\n",
		    current_print_desc->type->name);
    // exit(EXIT_FAILURE);
  }
  
  op_pr *op_print = new op_pr;
  op_prints_list.push_back(op_print);
  op_print->desc = current_print_desc;
  op_prints[(int)top] = op_print;
  top_specified[(int)top] = true;

  va_start(ap, top);
  while ( (opcode = static_cast<TOP>(va_arg(ap,int))) != stop ) {
    op_print = new op_pr;
    op_prints_list.push_back(op_print);
    op_print->desc = current_print_desc;
    op_prints[(int)opcode] = op_print;
    top_specified[(int)opcode] = true;
  }
  va_end(ap);
}

/////////////////////////////////////
void Name ()
/////////////////////////////////////
//  See interface description.
/////////////////////////////////////
{
  if (current_print_desc->args == MAX_LISTING_OPERANDS) {
    fprintf(stderr, "### Error: too many listing operands for %s\n",
		    current_print_desc->type->name);
    exit(EXIT_FAILURE);
  }
  current_print_desc->arg[current_print_desc->args] = NAME;
  //current_print_desc->fmt[current_print_desc->args] = fmt;
  current_print_desc->args++;
  current_print_desc->have_name = true;
}

/////////////////////////////////////
void Operand (int operand_index)
/////////////////////////////////////
//  See interface description.
/////////////////////////////////////
{
  if (operand_index >= MAX_OPNDS) {
    fprintf(stderr, "### Error: operand index (%d) exceeds %d\n",
		    operand_index, MAX_OPNDS-1);
    exit(EXIT_FAILURE);
  }
  if (current_print_desc->args == MAX_LISTING_OPERANDS) {
    fprintf(stderr, "### Error: too many listing operands for %s\n",
		    current_print_desc->type->name);
    exit(EXIT_FAILURE);
  }
  current_print_desc->arg[current_print_desc->args] = OPND+operand_index;
  //current_print_desc->fmt[current_print_desc->args] = fmt;
  current_print_desc->args++;
}

/////////////////////////////////////
void Result (int result_index)
/////////////////////////////////////
//  See interface description.
/////////////////////////////////////
{
  if (result_index >= MAX_RESULTS) {
    fprintf(stderr, "### Error: result index (%d) exceeds %d\n",
		    result_index, MAX_RESULTS-1);
    exit(EXIT_FAILURE);
  }
  if (current_print_desc->args == MAX_LISTING_OPERANDS) {
    fprintf(stderr, "### Error: too many listing operands for %s\n",
		    current_print_desc->type->name);
    exit(EXIT_FAILURE);
  }
  current_print_desc->arg[current_print_desc->args] = RESULT+result_index;
  //current_print_desc->fmt[current_print_desc->args] = fmt;
  current_print_desc->args++;
}

/////////////////////////////////////
void Set_AsmName_Func(const char *(*asmname_func)(TOP topcode))
/////////////////////////////////////
//  See interface description.
/////////////////////////////////////
{
  asmname = asmname_func;
}

/////////////////////////////////////
void ISA_Print_End(void)
/////////////////////////////////////
//  See interface description.
/////////////////////////////////////
{
  list<LISTING_INFO>::iterator isi;

  FILE* hfile    = NULL ;
  FILE* cfile    = NULL ;
  FILE* efile    = NULL ;

  // Whether we generate code for the core (static) or for an extension.
  bool  gen_static_code = Is_Static_Code();

  // Get extension name or NULL for static code generation.
  char *extname = gen_static_code ? NULL : Get_Extension_Name();

  // File handler initialization.
  char *hfilename     = NULL ;    /* Header file name              */
  char *cfilename     = NULL ;    /* C file name                   */
  char *efilename     = NULL ;    /* Export file name              */

  const char * const bname = FNAME_TARG_ISA_PRINT;

  hfilename = Gen_Build_Filename(bname,extname,gen_util_file_type_hfile);
  hfile     = Gen_Open_File_Handle(hfilename, "w");

  cfilename = Gen_Build_Filename(bname,extname,gen_util_file_type_cfile);
  cfile     = Gen_Open_File_Handle(cfilename, "w");

  if(gen_static_code)
   { efilename = Gen_Build_Filename(bname,extname,gen_util_file_type_efile);
     efile     = Gen_Open_File_Handle(efilename, "w");
   }

  const char comma = ',';
  const char space = ' ';
  const char *isa_print_type_format = "\t/* %s[%d] */";
  const char *isa_print_format_format = "  { %-14s ";
  const char *isa_print_args_format = " %s%c";
  unsigned int top;
  bool err;

  for (err = false, top = 0; top < TOP_count_limit; ++top) {
    bool is_dummy = TOP_is_dummy((TOP)top);
    bool is_simulated = TOP_is_simulated((TOP)top);
    if (!top_specified[top]) {
      if (!is_simulated && !is_dummy) {
	fprintf(stderr, "### Error: no print specification for %s\n",
		        TOP_Name((TOP)top));
	err = true;
      }
    } else if (is_dummy) {
      fprintf(stderr, "### Error: print specification for dummy op %s\n",
		      TOP_Name((TOP)top));
      err = true;
    } else if (is_simulated) {
      fprintf(stderr, "### Error: print specification for simulated op %s\n",
		      TOP_Name((TOP)top));
      err = true;
    }
  }
  if (err) exit(EXIT_FAILURE);

  /* For dynamic extensions, we want to emit C and not C++     */
  Emit_C_Header(cfile);     /* Emit ifdef _cplusplus directive */
  fprintf(cfile,
          "\n"
          "#include <string.h>\n"
          "#include \"topcode.h\"\n");

  fprintf(cfile,"#include \"%s\"\n\n", 
          gen_static_code ? hfilename : "dyn_isa_print.h");

  Emit_Header (hfile, bname, interface,extname);
  fprintf(hfile,"#include \"topcode.h\"\n");

  // We assume that macros are common to
  // static and dynamic code generation.
  if(gen_static_code)
   {Emit_Definitions (hfile, "ISA_PRINT_");
   }

  if(gen_static_code) {
  fprintf(hfile, "\ntypedef enum {\n"
	"  %-21s = %d,  /* %s */\n"
	"  %-21s = %d,  /* %s */\n"
	"  %-21s = %d,  /* %s */\n"
   	"  %-21s = %d,  /* %s */\n"
	"  %-21s = %d,  /* %s */\n"
   	"  %-21s = %d,  /* %s */\n"
   	"  %-21s = %d   /* %s */\n"
	"} ISA_PRINT_COMP;\n",
	Print_Name(END), END, "End of list marker",
	Print_Name(NAME), NAME, "Instruction name",
	Print_Name(OPND), OPND, "OPND+n => operand n",
	"ISA_PRINT_COMP_opnd_MAX", OPND + MAX_OPNDS - 1, "Max operand id",
	Print_Name(RESULT), RESULT, "RESULT+n => result n",
	"ISA_PRINT_COMP_result_MAX", RESULT + MAX_RESULTS - 1, "Max result id",
        "ISA_PRINT_COMP_MAX", MAX_LISTING_OPERANDS-1, "Last component");

  fprintf(hfile, "\ntypedef struct {\n"
		"  const char *format;\n"
  		"  mUINT8 comp[%d];\n" 
	  //  		"  const char *fmt[%d];\n" 
	  //"} ISA_PRINT_INFO;\n",MAX_LISTING_OPERANDS,MAX_LISTING_OPERANDS);
	        "} ISA_PRINT_INFO;\n",MAX_LISTING_OPERANDS);

  fprintf(hfile,"\nBE_EXPORTED extern const ISA_PRINT_INFO *ISA_PRINT_info;\n");
  fprintf(hfile,"\nBE_EXPORTED extern unsigned int ISA_PRINT_info_size;\n\n");

  fprintf(efile, "ISA_PRINT_info\n");
  fprintf(efile, "ISA_PRINT_info_size\n");
  }

  // Emission of static table.
  if(gen_static_code)
   { fprintf(cfile, "\nstatic const ISA_PRINT_INFO ISA_PRINT_static_info[%d] = {\n",
                    list_index + 1);
   }
  // Emission of dynamic table.
  else
   { fprintf(cfile, "\nstatic const ISA_PRINT_INFO ISA_PRINT_dynamic_info[%d] = {\n",
                     list_index);
   }

  // First empty element is reserved to static code table.
  if(gen_static_code)
   { fprintf (cfile, isa_print_format_format, "\"\",");
     fprintf (cfile, isa_print_args_format, Print_Name(END), space);
     fprintf (cfile, "},");
     fprintf (cfile, isa_print_type_format, "print_NULL", 0);
     fprintf (cfile, "\n");
   }

  char buf[1000];

  for ( isi = all_prints.begin(); isi != all_prints.end(); ++isi ) {
  	LISTING_INFO curr_type = *isi;
	sprintf (buf, "\"%s\",", curr_type->type->format_string);
	fprintf (cfile, isa_print_format_format, buf);

	// printout component types
	//fprintf (cfile, "\n%17s {", "");
	fprintf (cfile, "\n%2s{", "");
	for (int i = 0; i < curr_type->args; i++) {
	    fprintf (cfile, isa_print_args_format,
			Print_Name(curr_type->arg[i]),
			comma);
	    fprintf (cfile, isa_print_type_format, curr_type->type->name, i);
	    fprintf (cfile, "\n%3s", "");
	}
	fprintf (cfile, isa_print_args_format, Print_Name(END), space);
	fprintf (cfile, "\n  }},");

	fprintf (cfile, isa_print_type_format, 
			curr_type->type->name,
			curr_type->args);

	fprintf (cfile, "\n");
#if 0
	// printout component formats:
	fprintf (cfile, "%17s {", "");
	for (int i = 0; i < curr_type->args-1; i++) {
	  fprintf (cfile, " \"%s\",", curr_type->fmt[i]);
	  fprintf (cfile, isa_print_type_format, curr_type->type->name, i);
	  fprintf (cfile, "\n%19s", "");
	}
	fprintf (cfile, " \"%s\" ", curr_type->fmt[curr_type->args-1]);
	fprintf (cfile, "}");
	fprintf (cfile, isa_print_type_format, 
			curr_type->type->name,
			curr_type->args-1);
	fprintf (cfile, "\n  },\n");
#endif
  }
  fprintf (cfile, "};\n");

  if(gen_static_code)
   { fprintf(hfile,
	     "#define ISA_PRINT_info_static_size (%d)\n",
	     list_index+1);
     fprintf(cfile,
             "\n"
             "const ISA_PRINT_INFO *ISA_PRINT_info      = ISA_PRINT_static_info;\n"
             "mUINT32               ISA_PRINT_info_size = %d;\n\n",
             list_index+1);
   }
  else
   { const char * const routine1 = "dyn_get_ISA_PRINT_info_tab";
     const char * const routine2 = "dyn_get_ISA_PRINT_info_tab_sz";

     fprintf(cfile, 
             "\n"
             "const ISA_PRINT_INFO*\n"
             "%s ( void )\n"
             "{ return (const ISA_PRINT_INFO*) ISA_PRINT_dynamic_info;\n"
             "}\n"
             "\n",
             routine1
            );

     fprintf(cfile,
             "const mUINT32\n"
             "%s ( void )\n"
             "{ return (const mUINT32) %d;\n"
             "}",
             routine2,
             list_index
            );
                   
     fprintf(hfile,"\n"
                   "extern const ISA_PRINT_INFO* %s ( void );\n"
                   "extern const mUINT32 %s ( void );\n",
                   routine1,
                   routine2);
   }

  // ISA_PRINT_info_index used to be of type unsigned char*.
  // We transform it into unsigned short* to add room for extensions.
  if(gen_static_code)
   {  fprintf(hfile, 
        "\nBE_EXPORTED extern const unsigned short* ISA_PRINT_info_index;\n");
      fprintf(efile, "ISA_PRINT_info_index\n");
   }

  if(gen_static_code)
   fprintf(cfile, "\nstatic const mUINT16 ISA_PRINT_static_info_index[%d] = {\n", TOP_count_limit);
  else
   fprintf(cfile, "\nstatic const mUINT16 ISA_PRINT_dynamic_info_index[%d] = {\n",TOP_count_limit);

  for (top = 0; top < TOP_count_limit; ++top ) {
  	op_pr *op_print = op_prints[top];
    	if (op_print) {
  	    fprintf(cfile, "  %3d,  /* %s: %s */\n", 
			op_print->desc->index+1,
			TOP_Name((TOP)top),
			op_print->desc->type->name);
	} else {
  	    fprintf(cfile, "  %3d,  /* %s */\n", 
			0,
			TOP_Name((TOP)top));
	}
  }
  fprintf(cfile, "};\n\n");

  if(gen_static_code)
  { fprintf(cfile,"const mUINT16 * ISA_PRINT_info_index = ISA_PRINT_static_info_index;\n\n");
  }
  else
  { const char * const routine_name = "dyn_get_ISA_PRINT_info_index_tab"; 

    fprintf(cfile,"\n"
                  "const mUINT16*\n"
                  "%s ( void )\n"
                  "{ return (const mUINT16*) ISA_PRINT_dynamic_info_index;\n"
                  "}\n\n",
                  routine_name);
    fprintf(hfile,"\n"
                  "extern const mUINT16 *%s (void);\n\n",
                  routine_name);
  }

  if(gen_static_code)
  { fprintf(hfile, "\ninline const ISA_PRINT_INFO *ISA_PRINT_Info(TOP topcode)\n"
                 "{\n"
		 "  INT index = ISA_PRINT_info_index[(INT)topcode];\n"
		 "  return index == 0 ? 0 : &ISA_PRINT_info[index];\n"
		 "}\n");

   fprintf(hfile, "\ninline const char* ISA_PRINT_INFO_Format(const ISA_PRINT_INFO *info)\n"
		 "{\n"
		 "  return info->format;\n"
		 "}\n");

   fprintf(hfile, "\ninline INT ISA_PRINT_INFO_Comp(const ISA_PRINT_INFO *info, INT index)\n"
		 "{\n"
		 "  return info->comp[index];\n"
		 "}\n");
#if 0
  fprintf(hfile, "\ninline const char* ISA_PRINT_INFO_Fmt(const ISA_PRINT_INFO *info, INT index)\n"
		 "{\n"
		 "  return info->fmt[index];\n"
		 "}\n");
#endif
  }

  if (asmname) {
    // Printing name table.
    if(gen_static_code)
      { fprintf(cfile, 
         "\nstatic const char * const ISA_PRINT_static_asmname[] = {\n");
      }
    else
      { fprintf(cfile,
         "\nstatic const char * const ISA_PRINT_dynamic_asmname[] = {\n");
      }

    for (top = 0; top < TOP_count_limit; ++top) {
      fprintf(cfile, "  \"%s\",\n", asmname((TOP)top));
    }

    if(gen_static_code)
      fprintf(cfile, "  \"UNDEFINED\"\n");  // UNDEFINED item reserved to static code gen.
    fprintf(cfile, "};\n");                 // Ending the table.

    if(gen_static_code)
     { fprintf(cfile, "\nBE_EXPORTED const char * const * ISA_PRINT_asmname = ISA_PRINT_static_asmname;\n");
	   fprintf(hfile, "\nBE_EXPORTED extern const char * const * ISA_PRINT_asmname;\n");
       fprintf(efile, "ISA_PRINT_asmname\n");
     }
    else
     { const char* const routine_name = "dyn_get_ISA_PRINT_name_tab";

       fprintf(cfile,"\n"
                     "const char**\n"
                     "%s ( void )\n"
                     "{ return (const char**) ISA_PRINT_dynamic_asmname;\n"
                     "}\n"
                     "\n"
                     "\n",
                     routine_name);
       fprintf(hfile,"extern const char** %s (void );\n",
                     routine_name);
     }

    // Inline routine is reserved for static code.
    if(gen_static_code) {
      fprintf(hfile, "\ninline const char *ISA_PRINT_AsmName(TOP topcode)\n"
                     "{\n"
                     "  BE_EXPORTED extern const char * const * ISA_PRINT_asmname;\n"
                     "  return ISA_PRINT_asmname[(INT)topcode];\n"
                     "}\n");
    }                                      // gen_static_code

  } else {                                 // else (!asmname)
    // Inline routine is reserved for static code.
     if(gen_static_code) {
       fprintf(hfile, "\ninline const char *ISA_PRINT_AsmName(TOP topcode)\n"
                      "{\n"
                      "  return TOP_Name(topcode);\n"
                      "}\n");
      }
  }

  //
  // Emit function ISA_PRINT_Operand_Is_Part_Of_Name:
  // (Only for static code).
  if(gen_static_code)
  { fprintf(hfile, "\nBE_EXPORTED extern BOOL ISA_PRINT_Operand_Is_Part_Of_Name(TOP topcode, INT opindex);\n");
    fprintf(efile, "ISA_PRINT_Operand_Is_Part_Of_Name\n");
    fprintf(cfile, 
                "\nBOOL ISA_PRINT_Operand_Is_Part_Of_Name(TOP topcode, INT opindex)\n"
		"{\n"
  		"  const ISA_PRINT_INFO *info = ISA_PRINT_Info(topcode);\n"
  		"  const char *place_in_format = ISA_PRINT_INFO_Format(info);\n"
  		"  BOOL in_name_part = 0;\n"
  		"  INT comp;\n"
  		"  INT i = 0;\n"
  		"  for (;;) {\n"
  		"  	comp = ISA_PRINT_INFO_Comp(info,i);\n"
  		"  	if (comp == ISA_PRINT_COMP_end) break;\n"
  		"	place_in_format = strchr(place_in_format, '%%');\n"
		"	place_in_format += 2; /* assume %%s */\n"
  		"  	if (comp == ISA_PRINT_COMP_name) {\n"
		"	  if (*place_in_format == '\\0' || *place_in_format == ' ')\n"
		"		in_name_part = 0;\n"
		"	  else\n"
		"		in_name_part = 1;\n"
  		"  	}\n"
  		"  	if (comp >= ISA_PRINT_COMP_opnd && comp < ISA_PRINT_COMP_result) {\n"
  		"  	  if (in_name_part) {\n"
		"	    INT comp_opindex = comp - ISA_PRINT_COMP_opnd;\n"
		"	    if (comp_opindex == opindex)\n"
		"		return 1;\n"
		"	    if (*place_in_format == '\\0' || *place_in_format == ' ')\n"
		"		in_name_part = 0;\n"
  		"  	  }\n"
  		"  	}\n"
  		"  	++i;\n"
  		"  }\n"
  		"  return 0;\n"
		"}\n");
    }

#if 0
  //
  // Emit function ISA_PRINT_Operand_Format:
  //
  fprintf(hfile, "\nTARGINFO_EXPORTED extern const char* ISA_PRINT_Operand_Format(TOP topcode, INT opindex);\n");
  fprintf(efile, "ISA_PRINT_Operand_Format\n");
  fprintf(cfile, "\n/* ====================================================================\n"
	         " *  ISA_PRINT_Format_Opnd\n"
	         " * ====================================================================\n"
	         " */\n"
	  "const char* ISA_PRINT_Operand_Format(TOP topcode, INT opindex)\n"
	  "{\n"
	  "  INT i;\n"
	  "  INT comp;\n"
	  "  const ISA_PRINT_INFO *pinfo = ISA_PRINT_Info(topcode);\n"
	  "\n"
	  "  i = 0;\n"
	  "  do {\n"
	  "    comp = ISA_PRINT_INFO_Comp(pinfo, i);\n"
	  "    if (ISA_PRINT_COMP_opnd <= comp && comp < ISA_PRINT_COMP_result) {\n"
	  "	 if ((comp - ISA_PRINT_COMP_opnd) == opindex) {\n"
	  "        return ISA_PRINT_INFO_Fmt(pinfo, i);\n"
	  "      }\n"
	  "    }\n"
	  "  } while (++i, comp != ISA_PRINT_COMP_end);\n"
	  "}\n");
#endif

  Emit_Footer (hfile);
  Emit_C_Footer(cfile);

  /* Closing file handlers */
  Gen_Close_File_Handle(hfile,hfilename);
  Gen_Close_File_Handle(cfile,cfilename);
  if(efile)
    Gen_Close_File_Handle(efile,efilename);

  /* Memory deallocation */
  delete [] op_prints;
  delete [] top_specified;

  Gen_Free_Filename(cfilename);
  Gen_Free_Filename(hfilename);
  if(efilename) Gen_Free_Filename(efilename);

  return ;
}
