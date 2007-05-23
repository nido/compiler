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
  This file has been modified by STMicroelectronics
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gen_util.h"

//////////////////////////////////////
// Management of core's extensions.
//////////////////////////////////////

static bool  Gen_Dyn_Code = false;   // By default, code generation is done
                                     // for the core and is "static".
static char* extension_name = NULL;

// Test whether current code generation
// is done for an extension.
bool 
Is_Dynamic_Code( void )
{
   return Gen_Dyn_Code;
}

bool 
Is_Static_Code( void )
{
   return !Gen_Dyn_Code;
}

// Return name of current extension.
char* 
Get_Extension_Name( void )
{
   if(false==Gen_Dyn_Code)
     return NULL;

   return extension_name;
}

/* We have added one optional null parameter for extension name */
/* See header file for definition of extension optional NULL    */
/* parameter.                                                   */
void Emit_Header (FILE *hfile, 
		  const char *name,
		  const char *const *interface_desc,
                  const char *extname /* Optional nul parameter */)
{
  int i;

  if (interface_desc) {
    for (i = 0; interface_desc[i] != NULL; ++i) {
      fprintf(hfile, "%s\n", interface_desc[i]);
    }
  }

  if(NULL==extname)
   { fprintf(hfile, "\n#ifndef %s_INCLUDED\n", name);
     fprintf(hfile, "#define %s_INCLUDED\n", name);
   }
  else
   { fprintf(hfile, "\n#ifndef %s_%s_INCLUDED\n",extname,name);
     fprintf(hfile, "#define %s_%s_INCLUDED\n",extname,name);
   }

  Emit_C_Header(hfile);

  /* Pull in appropriate stuff from common/com/defs.h here, so that
   * the header can be used whether defs.h has been included or not.
   *
   * For reconfigurability purpose, the file defs.h has been
   * splitted into two files:
   *   - defs.h (modified) has been modified
   *   - defs_exported.h (included in the modified version of defs.h).
   */
   fprintf(hfile,
     "#ifndef defs_INCLUDED\n"
     "#define defs_INCLUDED\n"
     "#include \"defs_exported.h\"\n"
     "#if (defined(_LANGUAGE_C) || defined(__GNUC__)) && !defined(inline)\n"
     "#define inline static __inline\n"
     "#endif\n"
     "#endif\n\n"
    );

   return;
}

void Emit_Footer (FILE *hfile)
{
  Emit_C_Footer(hfile);
  fprintf (hfile, "#endif\n");
  return;
}

typedef enum {
  DK_MACRO
} DEFINITION_KIND;

typedef struct definition {
  DEFINITION_KIND kind;
  const char *name;
  const char *s;
  struct definition *next;
} DEFINITION;

static DEFINITION *defs;
static DEFINITION *lastdef;

void Define_Macro (const char *name, const char *def)
{
  DEFINITION *newdef = new DEFINITION;
  newdef->kind = DK_MACRO;
  newdef->name = name;
  newdef->s = def;
  newdef->next = NULL;
  if (defs == NULL) {
    defs = newdef;
  } else {
    lastdef->next = newdef;
  }
  lastdef = newdef;
}

void Emit_Definitions (FILE *hfile, const char *prefix)
{
  DEFINITION *def;

  if (defs != NULL) fprintf(hfile, "\n");

  for (def = defs; def != NULL; def = def->next) {
    int c;
    int pos;
    const char *s = def->s;
    pos = fprintf(hfile, "#define %s%s ", prefix, def->name);
    while (pos++ < 40) fputc(' ', hfile);
    fprintf(hfile, "(\"");
    while (c = *s++) {
      if (c == '\\') {
	fprintf(hfile, "\\\\");
      } else if (c < ' ') {
	switch (c) {
	case '\n':
	  fprintf(hfile, "\\n");
	  break;
	case '\t':
	  fprintf(hfile, "\\t");
	  break;
	default:
	  fprintf(hfile, "\\%03o", c);
	  break;
	}
      } else {
	fputc(c, hfile);
      }
    }
    fprintf(hfile, "\")\n");
  }
}

/////////////////////////////////////
// Must be specified in each
// targinfo extension file at the 
// very beginning of main() function.
/////////////////////////////////////
void 
Set_Dynamic( char* extname )
{
   unsigned int size;

   Gen_Dyn_Code = true;

   if(NULL==extname || '\0'==extname[0])
     { fprintf(stderr,"### error: extension name is not specified or name is empty\n");
       exit(EXIT_FAILURE);
     }
   size = strlen(extname) + 1;
   extension_name = (char*)malloc(size);
   if(NULL==extension_name)
     { fprintf(stderr,"### error: unable to allocate %d bytes\n",size);
       exit(EXIT_FAILURE);
     }
   strcpy(extension_name,extname);

   return;
}

/////////////////////////////////////
FILE* 
Gen_Open_File_Handle( const char *const filename, const char *const mode )
/////////////////////////////////////
// Opening file handle.
////////////////////////////////////
{
   FILE *f = NULL;

   if(NULL==filename || '\0'==*filename)
    { fprintf(stderr,"### Missing file name, unable to open file\n");
      exit(EXIT_FAILURE);
    }

   f = fopen(filename,mode);
   if(NULL==f)
    { fprintf(stderr,"### Unable to open file %s\n",filename);
      exit(EXIT_FAILURE);
    }

   return f;
}

/////////////////////////////////////
void 
Gen_Close_File_Handle ( FILE *f, const char *const filename )
/////////////////////////////////////
// Closing file handles. Second parameter might be NULL.
/////////////////////////////////////
{
   if(NULL==f)                // Dummy case.
      return;

   if(fclose(f))
    { fprintf(stderr,"### Unable to close file handle");
      if(NULL!=filename && '\0'!=*filename)
        fprintf(stderr," (%s)\n",filename);
      else
        fprintf(stderr,"\n");
      exit(EXIT_FAILURE);
    }

   return;
}

/////////////////////////////////////
char * 
Gen_Build_Filename( const char * const fname,
                    const char * const extname,
                    const GEN_UTIL_FILE_TYPE type )
/////////////////////////////////////
// fname  : body file name (ex: targ_isa_print)
// extname: extension name. If not extension, extname is NULL or ""
/////////////////////////////////////
{
  unsigned int  length=0;
  const char   *str_end = NULL;
  char         *ret = NULL;
  bool          with_extension ; 
  const char   *true_extname;

  true_extname  = gen_util_file_type_dyn_hfile == type      ? "dyn" : extname;
  with_extension= true_extname!= NULL && *true_extname!='\0'? true  : false;

  //Firstly get temination string.
  switch(type)
   { case gen_util_file_type_cfile:
     str_end = "c";
     break;

     case gen_util_file_type_dyn_hfile:     /* Fall through */
     case gen_util_file_type_hfile:
     str_end = "h";
     break;

     case gen_util_file_type_efile:
     str_end = "Exported";
     break;

     case gen_util_file_type_c_i_file:
     str_end = "inc.c";
     break;

     case gen_util_file_type_cpp_i_file:
     str_end = "inc.cxx";
     break;

     default:
     fprintf(stderr,"### Internal error: unable to recognize file type\n");
     exit(EXIT_FAILURE);
     break;
   }            // end sw

  // Calculate string length.
  if(with_extension)
   { length += strlen(true_extname);
     length += strlen("_");
   }

  length+= strlen(fname);
  length+= strlen(".");
  length+= strlen(str_end);
  length++;          /* null terminating character */

  // Mem alloc.
  ret = (char*) malloc(length);
  if(NULL==ret)
   { fprintf(stderr,"### Unable to allocate %d bytes\n",length);
     exit(EXIT_FAILURE);
   }

  // Filling string.
  // Setting first character to '\0' allows to use
  // strcat in any case.
 *ret = '\0';

  if(with_extension)
   { strcat(ret,true_extname);
     strcat(ret,"_");
   }

  strcat(ret,fname);
  strcat(ret,".");
  strcat(ret,str_end);

  return ret;
}

/////////////////////////////////////
void 
Gen_Free_Filename( char* name )
/////////////////////////////////////
// name (char*) string allocated when
//              file name has been built.
/////////////////////////////////////
{
   if(name) free(name);

   return;
}


/////////////////////////////////////
void 
Emit_C_Header(FILE *hfile)
/////////////////////////////////////
// hfile (FILE*) file handle.
/////////////////////////////////////
{
  fprintf (hfile, "#ifdef __cplusplus\n"
          "extern \"C\" {\n"
           "#endif\n\n");

  return;
}

/////////////////////////////////////
void 
Emit_C_Footer(FILE *hfile)
/////////////////////////////////////
// hfile (FILE*) file handle.
/////////////////////////////////////
{
  fprintf (hfile, "\n#ifdef __cplusplus\n"
		  "}\n"
		  "#endif\n"
                  "\n");
  return;
}

/////////////////////////////////////
void 
Remove_Filename_Extension(const char *src_fname,
                          char       *tgt_fname)
/////////////////////////////////////
// src_fname (const char*) filename with extension
// tgt_fname (char*) buffer that will be filled with filename
//                   without the extension
/////////////////////////////////////
{
  while (*src_fname!='\0' && *src_fname!='.') {
	*tgt_fname++ = *src_fname++;
  }
  *tgt_fname = '\0';
  return;
}

