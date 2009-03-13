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
 *
 */


// isa_lits_gen.cxx
/////////////////////////////////////
//
//  Generate a list of lit classes and their values.
//
/////////////////////////////////////
//

#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
#include "W_limits.h"
#include "gen_util.h"
#include "isa_lits_gen.h"

struct lit_range {
  const char *name;
  long long min;
  long long max;
  // [JV] Add scaling info via bits mask on lowest bits
  long scaling_value;
  long long scaling_mask;
  long max_right_rotate;
  long long right_rotate_mask;
};

static const char * const interface[] = {
  "/* ====================================================================",
  " * ====================================================================",
  " *",
  " * Description:",
  " *",
  " *   A list of all the lit classes used in an ISA.",
  " *   It exports the following:",
  " *",
  " *   typedef (enum) ISA_LIT_CLASS",
  " *       An enumeration of the lit classes.",
  " *",
  " *   typedef (struct) ISA_LIT_CLASS_INFO",
  " *       Contains info about first and last ECV in the EC.",
  " *       The contents are private.",
  " *",
  " *   typedef (struct) ISA_LIT_CLASS_VALUE_INFO",
  " *       Contains info about name and min/max of the LC.",
  " *       The contents are private.",
  " *",
  " *   const char * ISA_LC_Name (ISA_LIT_CLASS lc)",
  " *       Returns name of <lc>.",
  " *",
  " *   INT64 ISA_LC_Min (ISA_LIT_CLASS lc)",
  " *       Returns the minimum value for the specified <lc>. For classes",
  " *       that have multiple sub-ranges, ISA_LC_Min returns the smallest",
  " *       minimum of all the sub-ranges.",
  " *",
  " *   INT64 ISA_LC_Max (ISA_LIT_CLASS lc)",
  " *       Returns the maximum value for the specified <lc>. For classes",
  " *       that have multiple sub-ranges, ISA_LC_Max returns the largest",
  " *       maximum of all the sub-ranges.",
  " *",
  " *   INT64 ISA_LC_Scaling_Value (ISA_LIT_CLASS lc, mUINT8 range )",
  " *       Returns the scaling value for given range.",
  " *",
  " *   INT64 ISA_LC_Scaling_Mask (ISA_LIT_CLASS lc, mUINT8 range )",
  " *       Returns the scaling mask for given range.",
  " *",
  " *   BOOL ISA_LC_Is_Signed (ISA_LIT_CLASS lc)",
  " *       Returns whether the lit-class <lc> is signed.",
  " *",
  " *   BOOL ISA_LC_Is_Negative (ISA_LIT_CLASS lc)",
  " *       Returns whether the lit-class <lc> is negative.",
  " *",
  " *   BOOL ISA_LC_Value_In_Class (INT64 val, ISA_LIT_CLASS lc)",
  " *       Returns whether <val> is a value that belongs to <lc>.",
  " *",
  " * ====================================================================",
  " *",
  " *   INT32 ISA_LC_Abstract_Value (INT64 val, ISA_LIT_CLASS lc)",
  " *        Returns the so-called *abstract value* for the given",
  " *        *concrete value* <val> and literal class <lc>.",
  " *        The *abstract value* is the value used internally in the",
  " *        compiler, the *concrete value* is the value used outside",
  " *",
  " *        The following example is limited to the case where",
  " *        a scaling factor mechanism is available. In such a",
  " *        case, the relation holds:",
  " *              lc_abst = lc_concrete << scf",
  " *              (lc_abst & mask) == 0",
  " *        where <lc_abst> is the abstract value, <mask>",
  " *        is the scaling mask and <&> is the conventional bitwise ",
  " *        operator. ",
  " *        For instance, we can assume that <lc_abst> is 0x8 , and",
  " *        that the scaling factor is 0x3. The scaling <mask> is therefore",
  " *        (0x1<<0x3)-1 = 0x7 and the relation (0x8 & 0x7)==0 is true. The",
  " *        concrete value is 0x8>>3 or 0x1.",
  " *",
  " *",
  " * ====================================================================",
  " */",
  NULL
};

static FILE *hfile = NULL;
static FILE *cfile = NULL;
static FILE *sfile = NULL;
static FILE *efile = NULL;

static struct lit_range signed_range[65];
static struct lit_range unsigned_range[65];
static int max_ranges = 0;

static char *hfilename     = NULL;
static char *cfilename     = NULL;
static char *sfilename     = NULL;
static char *efilename     = NULL;

// Whether we generate code for the core or for
// an extension.
static bool gen_static_code = false;

// Currently we don't accept more than 256 class
// of literals. In fact, this limit is too high
// since we can't have more than 65 unsigned
// ranges and 65 signed ranges (sub-ranges are
// not managed yet).
static       int lc_count     = 0;
static const int lc_max_limit = 255;

// C file generated when static code generation is done,
// included in C ISA_LIT_info table when dynamic code
// generation is done.
static FILE *cincfile = NULL;
static char *cincfilename  = NULL;

/////////////////////////////////////
void ISA_Lits_Begin (void)
/////////////////////////////////////
//  See interface description.
/////////////////////////////////////
{
  gen_static_code = Is_Static_Code();

  const char* const extname  = gen_static_code ? NULL: Get_Extension_Name();

  // Opening files
  // Header file
  hfilename = Gen_Build_Filename(FNAME_TARG_ISA_LITS,
                                 extname,
                                 gen_util_file_type_hfile);
  hfile     = Gen_Open_File_Handle(hfilename,"w");

  // C file
  cfilename = Gen_Build_Filename(FNAME_TARG_ISA_LITS,
                                 extname,
                                 gen_util_file_type_cfile);
  cfile     = Gen_Open_File_Handle(cfilename,"w");

  // Export file
  if(gen_static_code) {
    efilename = Gen_Build_Filename(FNAME_TARG_ISA_LITS,
				   extname,
				   gen_util_file_type_efile);
    efile     = Gen_Open_File_Handle(efilename,"w");
  }
  else {
    const char *headers[] = {
      "<stddef.h>",
      "\"dyn_" FNAME_ISA_LITS ".h\"",
      "",
    };

    sfilename = Gen_Build_Filename(FNAME_STUB_ISA_LITS,
				   extname,
				   gen_util_file_type_cfile);
    sfile     = Gen_Open_File_Handle(sfilename,"w");
    Emit_Stub_Header(sfile,headers);
  }

  // C file generated when static code generation is done,
  // included in C ISA_LIT_info table when dynamic code
  // generation is done.
  cincfilename = Gen_Build_Filename(FNAME_TARG_ISA_LITS,
                                    NULL, /* Build at static code gen. time*/
                                    gen_util_file_type_c_i_file);
  if(gen_static_code) {
    cincfile  = Gen_Open_File_Handle(cincfilename,"w");
    
    fprintf(cincfile,
	    "\n\n"
	    "/* This file has been create automatically\n"
	    " *  Do not modifiy it.\n"
	    " */\n\n"
	    "/* Including the static part of literal table.\n"
	    " */\n\n");
  }

  /* For dynamic extensions, we want to emit C and not C++     */
  if(!gen_static_code)      /* For a pure interface, start     */
    Emit_C_Header(cfile);   /* "C" block.                      */


  fprintf(cfile,"#include \"%s\"\n\n",
          gen_static_code ? hfilename : "dyn_isa_lits.h");

  Emit_Header (hfile, FNAME_TARG_ISA_LITS, interface,extname);

  if(gen_static_code)
    fprintf(hfile,"#define %-20s %d\n","ISA_LC_UNDEFINED",lc_count++); 

  if(gen_static_code)
   {fprintf(cfile, 
        "static const ISA_LIT_CLASS_INFO ISA_LIT_CLASS_static_info[] = {\n");
   }
  else
   { fprintf(cfile, 
        "static const ISA_LIT_CLASS_INFO ISA_LIT_CLASS_dynamic_info [] = {\n");
   }

  // UNDEFINED entry is reserved to static table
  if(gen_static_code) {
    fprintf(cfile, "  { { { 0x0000000000000000LL, 0x0000000000000000LL, 0, 0, 0 } }, 0, 0, 0, \"ISA_LC_UNDEFINED\" },\n");
    fprintf(cincfile,"  { { { 0x0000000000000000LL, 0x0000000000000000LL, 0, 0, 0 } }, 0, 0, 0, \"ISA_LC_UNDEFINED\" },\n");
  }

  // For dynamic code generation, we include in the
  // table the static part of the table.
  if(!gen_static_code) {
    fprintf(cfile,
	    "\n"
	    "#include \"%s\"\n\n",
	    cincfilename
            );
  }

  for (int i = 1; i <= 64; ++i) {
    unsigned_range[i].min = 0;
    unsigned_range[i].max = (i == 64) ? -1ULL : (1ULL << i) - 1;
    unsigned_range[i].scaling_value = 0; // [JV] default 
    unsigned_range[i].scaling_mask = 0; // [JV] default 
    unsigned_range[i].max_right_rotate = 0; // [JV] default 
    unsigned_range[i].right_rotate_mask = 0; // [JV] default 

    signed_range[i].min = -1LL << (i - 1);
    signed_range[i].max = (1LL << (i - 1)) - 1;
    signed_range[i].scaling_value = 0; // [JV] default 
    signed_range[i].scaling_mask = 0; // [JV] default 
    signed_range[i].max_right_rotate = 0; // [JV] default 
    signed_range[i].right_rotate_mask = 0; // [JV] default 
  }
}


/////////////////////////////////////
LIT_RANGE SignedBitRange(unsigned int bit_size)
/////////////////////////////////////
//  See interface description.
/////////////////////////////////////
{
  if (bit_size == 0 || bit_size > 64) {
    fprintf(stderr, "### Error: invalid signed bit range: %d\n", bit_size);
    exit(EXIT_FAILURE);
  }
  return &signed_range[bit_size];
}


/////////////////////////////////////
LIT_RANGE UnsignedBitRange(unsigned int bit_size)
/////////////////////////////////////
//  See interface description.
/////////////////////////////////////
{
  if (bit_size == 0 || bit_size > 64) {
    fprintf(stderr, "### Error: invalid unsigned bit range: %d\n", bit_size);
    exit(EXIT_FAILURE);
  }
  return &unsigned_range[bit_size];
}


/////////////////////////////////////
LIT_RANGE ISA_Create_Lit_Range(const char *name, long long min, long long max, long scaling, long right_rotate_width)
/////////////////////////////////////
//  See interface description.
/////////////////////////////////////
{
  LIT_RANGE range = new lit_range;
  range->name = name;
  range->min = min;
  range->max = max;
  range->scaling_value = scaling;
  if(scaling > 31) {
    fprintf(stderr, "### Error: invalid scaling %d (greater than 31)\n", scaling);
    exit(EXIT_FAILURE);
  }
  range->scaling_mask = (1<<scaling)-1;

  if(right_rotate_width==0) { // no rotate
    range->max_right_rotate = 0;
    range->right_rotate_mask = 0;
  } 
  else if((right_rotate_width + scaling) > 6) {
    fprintf(stderr, "### Error: invalid right rotate width %d (greater than 6 bits)\n", right_rotate_width);
    exit(EXIT_FAILURE);
  }
  /* Width on which the rotation can be applied
   * Example: if scaling is 1, and rotate value width is 4, the rotate width
   * or bit field on which rotation can be applied is 1 << 4 << 1 = 32 bits.
   * So the rotate mask is 0xffffffff.
   */
  else if((right_rotate_width + scaling) == 6) {
    range->right_rotate_mask = 0xffffffffffffffffLL;
  }
  else {
    range->right_rotate_mask = (1LL << ((1 << right_rotate_width) << scaling)) - 1;
  }
  /* Maximum number of rotation used to test given immediate value */
  range->max_right_rotate = (1 << right_rotate_width)-1;
  return range;
}


/////////////////////////////////////
void ISA_Create_Lit_Class(const char* name, LIT_CLASS_TYPE type, ...)
/////////////////////////////////////
//  See interface description.
/////////////////////////////////////
{
  va_list ap;
  LIT_RANGE range;
  bool is_signed = (type != UNSIGNED);
  bool is_negative = (type == NEGATIVE);
  long long min;
  long long max;

  switch(type) {
  case SIGNED:
    min = LONGLONG_MAX;
    max = LONGLONG_MIN;
    break;
  case UNSIGNED:
    min = ULONGLONG_MAX;
    max = 0;
    break;
  case NEGATIVE:
    min = 0;
    max = LONGLONG_MIN;
    break;
  }
    
  int num_ranges = 0;
  const char *string_template;

  // Find the smallest min and largest max for all ranges, and
  // count the number of ranges.
  va_start(ap,type);
  while ((range = va_arg(ap,LIT_RANGE)) != LIT_RANGE_END) {
    ++num_ranges;
    switch(type) {
    case SIGNED:
      if (range->min < min) min = range->min;
      if (range->max > max) max = range->max;
      break;
    case UNSIGNED:
      if ((unsigned long long)range->min < (unsigned long long)min) {
	min = range->min;
      }
      if ((unsigned long long)range->max > (unsigned long long)max) {
	max = range->max;
      }
      break;
    case NEGATIVE:
      long long sw = range->min;
      range->min = - range->max;
      range->max = - sw;

      if (range->min < max) {
	min = range->min;
      }
      if (range->max > max) {
	max = range->max;
      }
      break;
    }
  }
  va_end(ap);
  if (num_ranges > max_ranges) max_ranges = num_ranges;
  assert (num_ranges <= 1 && 
	  "LIT_CLASS with multiple ranges not yet supported"
	  "(this will break extension compatibility).");

  if(gen_static_code)
   fprintf(hfile,"#define LC_%-17s %d\n",name,lc_count);
  else
   fprintf(hfile,"#define LC_dyn_%-17s (ISA_LC_STATIC_MAX+1+%d)\n",name,lc_count);

  // Increment the number of range.
  ++lc_count;
  if(lc_count>lc_max_limit)
   { fprintf(stderr, 
             "### Error: number of literal classes is greater than allowed limit (%d)\n", 
             lc_max_limit);
      exit(EXIT_FAILURE);
   }

  // Initialize ISA_LIT_CLASS_info for this class. Note that .range[0]
  // holds the smallest min/largest max; .range[1] is the first sub-range.
  fprintf(cfile, "  { { { " PRINTF_LONGLONG_HEXA ", " PRINTF_LONGLONG_HEXA " }", min, max);
  if(gen_static_code)
    fprintf(cincfile, "  { { { " PRINTF_LONGLONG_HEXA ", " PRINTF_LONGLONG_HEXA " }", min, max);

  va_start(ap,type);
  while ((range = va_arg(ap,LIT_RANGE)) != LIT_RANGE_END) {
    fprintf(cfile, ",\n      { " PRINTF_LONGLONG_HEXA ", " PRINTF_LONGLONG_HEXA ", %ld, " PRINTF_LONGLONG_HEXA ", %d, " PRINTF_LONGLONG_HEXA " }", 
	    range->min, range->max, range->scaling_value, range->scaling_mask,
	    range->max_right_rotate, range->right_rotate_mask);
    
    if(gen_static_code)
    { fprintf(cincfile, ",\n      { " PRINTF_LONGLONG_HEXA ", " PRINTF_LONGLONG_HEXA ", %ld, " PRINTF_LONGLONG_HEXA ", %d, " PRINTF_LONGLONG_HEXA " }", 
	      range->min, range->max, range->scaling_value, range->scaling_mask,
	      range->max_right_rotate, range->right_rotate_mask);
    }
  }
  va_end(ap);

  string_template = gen_static_code ?
                    " }, %d, %d, %d, \"LC_%s\" },\n":
                    " }, %d, %d, %d, \"LC_dyn_%s\" },\n";

  fprintf(cfile,string_template,num_ranges,is_signed,is_negative,name);
  if(gen_static_code)
    fprintf(cincfile,string_template,num_ranges,is_signed,is_negative,name);

  return;
}


/////////////////////////////////////
void ISA_Lits_End(void)
/////////////////////////////////////
//  See interface description.
/////////////////////////////////////
{
  if(gen_static_code) {
    fprintf(hfile,
	    "\n"
	    "BE_EXPORTED extern mUINT32 ISA_LC_MAX;\n\n"
	    "#define %-20s %d\n" 
	    "#define %-20s %d\n\n"
	    "typedef mUINT32 ISA_LIT_CLASS; /* used to be an enum */\n\n",
	    "ISA_LC_STATIC_MAX",lc_count-1,"ISA_LC_MAX_LIMIT",lc_max_limit);
  }


  fprintf(cfile, "};\n\n");
  if(gen_static_code)
    fprintf(cincfile,"\n\n");

  if(gen_static_code) {
    fprintf(cfile,
	    "BE_EXPORTED const ISA_LIT_CLASS_INFO * ISA_LIT_CLASS_info = ISA_LIT_CLASS_static_info;\n");
  }
  else {
    const char *fct_name1= "dyn_get_ISA_LIT_CLASS_info_tab";
    const char *fct_name2= "dyn_get_ISA_LIT_CLASS_info_tab_sz";
    const char *fct_name3= "dyn_get_ISA_LIT_CLASS_static_max";

    fprintf(sfile,
	    "BE_EXPORTED const ISA_LIT_CLASS_INFO * ISA_LIT_CLASS_info = NULL;\n");
    
     fprintf(cfile,
             "\n\n"
             "const ISA_LIT_CLASS_INFO* %s ( void )\n"
             "{ return ISA_LIT_CLASS_dynamic_info;\n"
             "};\n\n",
             fct_name1);

     fprintf(cfile,
             "const mUINT32 %s ( void )\n"
             "{ return (const mUINT32) (ISA_LC_STATIC_MAX + 1/*UNDEFINED*/ + %d);\n"
             "}\n\n",
             fct_name2,lc_count);
     
     fprintf(cfile,
             "const mUINT32 %s ( void )\n"
             "{ return (const mUINT32) (ISA_LC_STATIC_MAX);\n"
             "}\n\n",
             fct_name3);
     
     fprintf(hfile,
             "\n\n"
             "extern const ISA_LIT_CLASS_INFO* %s ( void );\n"
             "extern const mUINT32 %s ( void );\n"
             "extern const mUINT32 %s ( void );\n",
             fct_name1,fct_name2,fct_name3);
   }

  if(gen_static_code) {
    fprintf(cfile,"\n"
	    "BE_EXPORTED mUINT32 ISA_LC_MAX = ISA_LC_STATIC_MAX;\n");
  }
  else {
    fprintf(sfile,"\n"
	    "BE_EXPORTED mUINT32 ISA_LC_MAX = 0;\n");
  }

  if(gen_static_code) 
   { fprintf(hfile,"#define MAX_RANGE_STATIC %d\n",
                   max_ranges + 1);
     fprintf(hfile, 
             "\ntypedef struct {\n"
             "  struct {\n"
	     "    INT64 min;\n"
	     "    INT64 max;\n"
	     "    INT32 scaling_value;\n"
	     "    INT64 scaling_mask;\n"
	     "    INT32 max_right_rotate;\n"
	     "    INT64 right_rotate_mask;\n"
	     "  } range[MAX_RANGE_STATIC];\n"
             "  mUINT8 num_ranges;\n"
             "  mBOOL is_signed;\n"
             "  mBOOL is_negative;\n"
             "  const char *name;\n"
             "} ISA_LIT_CLASS_INFO;\n");

     fprintf(hfile,
             "\n"
             "BE_EXPORTED extern const ISA_LIT_CLASS_INFO * ISA_LIT_CLASS_info;\n");
     fprintf(efile, "ISA_LIT_CLASS_info\n");
    }

  if(gen_static_code) {
  fprintf(hfile, "\ninline const char * ISA_LC_Name (ISA_LIT_CLASS lc)\n"
		 "{\n"
		 "  return ISA_LIT_CLASS_info[lc].name;\n"
		 "}\n");

  fprintf(hfile, "\ninline INT64 ISA_LC_Min (ISA_LIT_CLASS lc)\n"
		 "{\n"
		 "  return ISA_LIT_CLASS_info[lc].range[0].min;\n"
		 "}\n");

  fprintf(hfile, "\ninline INT64 ISA_LC_Max (ISA_LIT_CLASS lc)\n"
		 "{\n"
		 "  return ISA_LIT_CLASS_info[lc].range[0].max;\n"
		 "}\n");

  fprintf(hfile, "\ninline INT64 ISA_LC_Scaling_Value (ISA_LIT_CLASS lc, mUINT8 range)\n"
		 "{\n"
		 "  return ISA_LIT_CLASS_info[lc].range[range].scaling_value;\n"
		 "}\n");

  fprintf(hfile, "\ninline INT64 ISA_LC_Scaling_Mask (ISA_LIT_CLASS lc, mUINT8 range)\n"
		 "{\n"
		 "  return ISA_LIT_CLASS_info[lc].range[range].scaling_mask;\n"
		 "}\n");

  fprintf(hfile, "\ninline mUINT8 ISA_LC_Ranges (ISA_LIT_CLASS lc)\n"
		 "{\n"
		 "  return ISA_LIT_CLASS_info[lc].num_ranges;\n"
		 "}\n");

  fprintf(hfile, "\ninline BOOL ISA_LC_Is_Signed (ISA_LIT_CLASS lc)\n"
		 "{\n"
		 "  return ISA_LIT_CLASS_info[lc].is_signed;\n"
		 "}\n");

  fprintf(hfile, "\ninline BOOL ISA_LC_Is_Negative (ISA_LIT_CLASS lc)\n"
		 "{\n"
		 "  return ISA_LIT_CLASS_info[lc].is_negative;\n"
		 "}\n");

  fprintf(hfile, "\ninline BOOL ISA_LC_Value_In_Class (INT64 val, ISA_LIT_CLASS lc)\n"
		 "{\n"
                 "  const ISA_LIT_CLASS_INFO *plc = ISA_LIT_CLASS_info + lc;\n"
		 "  INT i;\n"
		 "  for (i = 1; i <= plc->num_ranges; ++i) {\n"
		 "    INT64 min = plc->range[i].min;\n"
		 "    INT64 max = plc->range[i].max;\n"
	         "    if(plc->range[i].max_right_rotate) {\n"
	         "      INT64 rotate_mask = (1LL << (1 << plc->range[i].scaling_value))-1;\n"
                 "      INT64 rotate_amount = 1 << plc->range[i].scaling_value;\n"
	         "      INT32 rotate_nb = 0;\n"
	         "      val &= plc->range[i].right_rotate_mask;\n"
	         "      /* Remove trailing zeros as possible */\n"
	         "      while(rotate_nb <= plc->range[i].max_right_rotate && (val & rotate_mask) == 0) {\n"
                 "        val >>= rotate_amount;\n"
                 "        rotate_nb++;\n"
                 "      }\n"
                 "    }\n"
	         "    else {\n"
	         "      if ( (val & plc->range[i].scaling_mask) != 0 ) { return FALSE; }\n"
                 "    }\n"
		 "    if ( plc->is_signed ) {\n"
	         "      if(!plc->range[i].max_right_rotate) {\n"
	         "        val = val >> plc->range[i].scaling_value;\n"
                 "      }\n"
		 "      if (val >= min && val <= max) return TRUE;\n"
		 "    } else {\n"
                 "      UINT64 valu = (UINT64)val;\n"
	         "      if(!plc->range[i].max_right_rotate) {\n"
                 "        valu >>= ((UINT64)(plc->range[i].scaling_value));\n"
                 "      }\n"
		 "      if (valu >= (UINT64)min && valu <= (UINT64)max) return TRUE;\n"
		 "    }\n"
		 "  }\n"
		 "  return FALSE;\n"
		 "}\n");

  fprintf(hfile, "inline INT64 ISA_LC_Abstract_Value (INT64 val, ISA_LIT_CLASS lc)\n"
                 "{\n"
                 "  const ISA_LIT_CLASS_INFO *plc = ISA_LIT_CLASS_info + lc;\n"
		 "  INT i;\n"
                 "\n"
		 "  for (i = 1; i <= plc->num_ranges; ++i) {\n"
		 "    INT64 min = plc->range[i].min;\n"
		 "    INT64 max = plc->range[i].max;\n"
                 "\n"
                 "    /* Managing scaling factor */\n"
	         "    if(plc->range[i].scaling_value == 0)\n"
                 "        continue;\n"
                 "\n"
                 "    /* Left shift is not sensitive to sign extension\n"
                 "     * Thus, we can factorize both the signed and the unsigned cases.\n"
                 "     */\n"
                 "    if( (plc->is_signed && val>=min && val<= max) ||\n"
                 "       ((UINT64)val>=(UINT64)min && (UINT64)val<=(UINT64)max)) {\n"
                 "            return val <<=plc->range[i].scaling_value;\n"
                 "        }\n"
                 "\n"
                 "  }    /* End for */\n"
                 "\n"
                 "    return val;            /* Default value */\n"
                 "}\n\n");

  }                            // if (gen_static_code)


  fprintf(hfile, "#ifdef DYNAMIC_CODE_GEN\n");
  fprintf(hfile, "extern void ISA_LITS_Initialize_Stub(void);\n");
  fprintf(hfile, "#endif\n");

  if(!gen_static_code) {
    fprintf(sfile,
	    "/*\n"
	    " * Exported routine.\n"
	    " */\n"
	    "void \n"
	    "ISA_LITS_Initialize_Stub( void )\n"
	    "{\n"
	    "  ISA_LIT_CLASS_info = (ISA_LIT_CLASS_INFO*)dyn_get_ISA_LIT_CLASS_info_tab();\n"
	    "  ISA_LC_MAX = dyn_get_ISA_LIT_CLASS_info_tab_sz();\n"
	    "  return;\n"
	    "}\n");
  }

  Emit_Footer (hfile);
  if(!gen_static_code)
    Emit_C_Footer(cfile);          // Ending "C" block.

  // Closing file handlers
  Gen_Close_File_Handle(cfile,cfilename);
  Gen_Close_File_Handle(hfile,hfilename);
  if(efile)    Gen_Close_File_Handle(efile,efilename);
  if(sfile)    Gen_Close_File_Handle(sfile,sfilename);
  if(cincfile) Gen_Close_File_Handle(cincfile,cincfilename);

  // Memory deallocation
  if(cfilename)    Gen_Free_Filename(cfilename);
  if(hfilename)    Gen_Free_Filename(hfilename);
  if(efilename)    Gen_Free_Filename(efilename);
  if(sfilename)    Gen_Free_Filename(sfilename);
  if(cincfilename) Gen_Free_Filename(cincfilename);

  return;
}

