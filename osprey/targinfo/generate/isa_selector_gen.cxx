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



#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#if __GNUC__ >=3 || defined(_MSC_VER)
#include <map>
using std::map;
#else
#include <map.h>
#endif // __GNUC__ >=3 || defined(_MSC_VER)
#include "topcode.h"
#include "targ_isa_subset.h"
#include "gen_util.h"
#include "isa_selector_gen.h"


// In following loops, we iterate on the number of
// TOP. This number differs whether we generate
// static or dynamic TOPs.
#ifndef DYNAMIC_CODE_GEN
static mUINT32 TOP_count_limit = TOP_static_count;
#else
static mUINT32 TOP_count_limit = TOP_dyn_count;
#endif


extern void ISA_Selector_End(void);


/* The generated interface description:
 */
static const char * const interface[] = {
  "/* ====================================================================",
  " * ====================================================================",
  " *",
  " * Description:",
  " *",
  " *   A description of ISA operator selector depending on active ISA subsets.",
  " *",
  " * ====================================================================",
  " * ====================================================================",
  " */",
  NULL
};

static bool gen_static_code = true;          // Whether we generate code for an
static bool gen_dyn_code    = false;         // extension or for the core.

static char *extname        = NULL;          // Extension name (NULL if no ext).
char *cfilename = NULL;                             /* C file name */
char *hfilename = NULL;                             /* Header file name */
FILE *cfile     = NULL;                             /* C file. handler*/
FILE *hfile     = NULL;                             /* Header f. handler*/



/* ====================================================================
 *   ISA_Selector_Begin
 * ====================================================================
 */
void ISA_Selector_Begin( const char* /* name */ )
{
  const char *const bname = FNAME_TARG_ISA_SELECTOR;  /* Base name        */

  gen_static_code = Is_Static_Code();
  gen_dyn_code    =!gen_static_code;
  
  if(gen_dyn_code)
    extname = Get_Extension_Name();

  hfilename = Gen_Build_Filename(bname,extname,gen_util_file_type_hfile);
  hfile     = Gen_Open_File_Handle(hfilename, "w");

  cfilename = Gen_Build_Filename(bname,extname,gen_util_file_type_cfile);
  cfile     = Gen_Open_File_Handle(cfilename, "w");

  fprintf(hfile, "\n#ifndef %s_INCLUDED\n", bname);
  fprintf(hfile, "#define %s_INCLUDED\n", bname);
  fprintf(hfile, "\n\n");
  
  fprintf(cfile,"\n");
  Emit_C_Header(cfile);           /* Emit extern "C" directive */
  fprintf(cfile,"\n\n");

  return;
}


/* ====================================================================
 *   ISA_Selector_Operator
 * ====================================================================
 */
void ISA_Selector_Operator( const char *operator_name, ... )
{
  bool first = true;
  va_list ap;
  ISA_SUBSET subset;
  TOP opcode;

  ISA_SUBSET termination = ISA_SUBSET_UNDEFINED;

  map<ISA_SUBSET,TOP> subsets;

  va_start(ap, operator_name);

  fprintf(hfile, "/* =======================\n");
  fprintf(hfile, " *   %s\n",operator_name);
  fprintf(hfile, " * =======================\n");
  fprintf(hfile, " */\n\n");

  fprintf(hfile, "\n#define is_%s(top) (",operator_name);

  while( (subset = static_cast<ISA_SUBSET>(va_arg(ap,int))) != termination ) {
    opcode = static_cast<TOP>(va_arg(ap,int));
    if(subsets.find(subset) != subsets.end()) {
      //fprintf(stderr,"Error: %s: More than one TOP defined for
      //subset '%s'\n",operator_name,ISA_SUBSET_Name(subset));
      // [vcdv] this is no morean error. tops are sorted at MDS level
      //to ensure the first top in a subset is displayed first in the
      //isa_selector.cxx and gets selected in the selector.
    } else {
      subsets[subset] = opcode;
    }
    
    if (first) {
      first=false;
    } else {
      fprintf(hfile, "||\\\n\t\t");
    }
    fprintf(hfile, "(top == TOP_%s)",TOP_Name(opcode));
  }
  fprintf(hfile, ")\n\n", operator_name);
     

  fprintf(hfile, "#undef %s\n",operator_name);
  first = true;

  for(map<ISA_SUBSET,TOP>::iterator iter = subsets.begin();
      iter != subsets.end(); iter++) {
    subset = iter->first;
    opcode = iter->second;
    
    if(first) {
      first = false;
      fprintf(hfile, "#if ");
    }
    else {
      fprintf(hfile, "#elif ");
    }
    fprintf(hfile, "defined(TARG_%s)\n",ISA_SUBSET_Name(subset));
    fprintf(hfile, "#define %s\tTOP_%s\n",operator_name,TOP_Name(opcode));
  }

  fprintf(hfile, "#else\n");
  fprintf(hfile, "#define %s ",operator_name);

  first = true;
  for(map<ISA_SUBSET,TOP>::iterator iter = subsets.begin();
      iter != subsets.end(); iter++) {
    subset = iter->first;
    opcode = iter->second;

    if (first) {
      first=false;
    } else {
      fprintf(hfile, "\t\t");
    }

    fprintf(hfile, "(ISA_SUBSET_LIST_Contains(ISA_SUBSET_List, ISA_SUBSET_%s) ? TOP_%s : \\\n",
	    ISA_SUBSET_Name(subset),TOP_Name(opcode));
  }
  
  fprintf(hfile, "\tTOP_UNDEFINED", operator_name);

  for(map<ISA_SUBSET,TOP>::iterator iter = subsets.begin();
      iter != subsets.end(); iter++) {
    fprintf(hfile,")");
  }
  fprintf(hfile,"\n");

  fprintf(hfile,"#endif\n\n");

  va_end(ap);
}

/* ====================================================================
 *   ISA_Selector_End
 * ====================================================================
 */
void ISA_Selector_End(void)
{
  fprintf(hfile, "\n\n#endif\n");

  Emit_C_Footer(cfile);

 // Closing file handlers.
  Gen_Close_File_Handle(hfile,hfilename);

  // Memory deallocation.
  Gen_Free_Filename(hfilename);

  return;
}
