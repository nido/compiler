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


// gen_util.h
////////////////////////////////////
//
// Utility functions to help in generating .c and .h files.
// These functions are only to be used by the generators.
//
//  void Emit_Header (FILE *hfile, 
//		      const char *name,
//		      const char * const *interface_desc,
//                    const char *extname=NULL)
//     Write out the standard h file header to <hfile>. The basename
//     of the header being created is specified by <name>. An optional
//     interface description is specified by <interface_desc> (pass NULL
//     if it is not desired). An second optional argument is used to
//     pass extension name (dynamic code generation only). For legacy
//     purpose, it is possible not to specify extension name (in such
//     a case, a NULL argument is passed).
//
//  void Emit_Footer   (FILE *file)
//     Write out the standard h file footer to <hfile>.
//
//  void Emit_C_Header (FILE *file)
//     Write out "extern \"C\" {" directive to file <cfile>.
//
//  void Emit_C_Footer (FILE *file)
//     Write out "}" (end of "C" directive) in file <cfile>.
//
//  void Emit_Stub_Header (FILE *file, const char **headers)
//     Write out copyright and headers inclusion
//
//  void Emit_Definitions (FILE *hfile, const char *prefix)
//     Write out client specified definitions (specified with Define_xxx)
//     to <hfile>. The definition names will all be prefxed by <prefix>.
//
//  void  Set_Dynamic(char* extname)
//     Useful only for the so-called "dynamic extensions". Must be
//     invoked at the very beginning of main() routine. First parameter
//     is mandatory and contains the name of the considered extension.
//
//  bool Is_Static_Code()
//      Return true if generator is running for "static" code (that is, we
//      are not considering generation of code for a dynamic extension).
//      Return false otherwise.
//      
//  boot Is_Dynamic_Code()
//      equivalent to !Is_Static_Code().
//
////////////////////////////////////


#ifndef gen_util_INCLUDED
#define gen_util_INCLUDED

#include "gen_util_gen.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef BZERO
#ifdef __CYGWIN__
#define BZERO(s,n) bzero((char*)s,n)
#else
#if defined(__MINGW32__) || defined(_MSC_VER)
#define BZERO(s,n) memset ((void*)(s),0,(n))
#else
#define BZERO(s,n) bzero((void*)s,n)
#endif
#endif
#endif

// We need special format in printf function
// when running Windows .NET compiler.
#if defined(__MINGW32__) || defined(_MSC_VER)
#define PRINTF_LONGLONG "I64"
#else
#define PRINTF_LONGLONG "ll"
#endif
#define PRINTF_LONGLONG_FORMAT(prefix,format,type) prefix "%" format PRINTF_LONGLONG type
#define PRINTF_LONGLONG_HEXA                       PRINTF_LONGLONG_FORMAT("0x","016","xULL")
#define PRINTF_LONGLONG_DECIMAL                    PRINTF_LONGLONG_FORMAT(""  ,""   ,"dLL")
#define PRINTF_LONGLONG_SIGNED                     PRINTF_LONGLONG_FORMAT("+" ,""   ,"iLL")
#define PRINTF_LONGLONG_UNSIGNED                   PRINTF_LONGLONG_FORMAT(""  ,""   ,"uULL")
#define PRINTF_LONGLONG_SUFFIX( value ) ( ( (value) & 0xFFFFFFFF00000000ULL ) ? "LL" : "" )

/* In order to keep code legacy, and in the same time */
/* add a new parameter (extname), this last parameter */
/* is optional and set to NULL if not specified.      */
extern void Emit_Header (FILE *hfile, 
			 const char *name, 
			 const char *const *interface_desc,
                         const char *extname = NULL /* Optional parameter */);

extern void Emit_Footer (FILE *hfile);
extern void Emit_Definitions (FILE *hfile, const char *prefix);

/* Added by STMicroelectronics.                     */
/* Defining the type of files that can be generated */
/* or included.                                     */
typedef enum{

gen_util_file_type_unknown=0 ,

gen_util_file_type_cfile     ,/* C file      */
gen_util_file_type_hfile     ,/* Header file */
gen_util_file_type_efile     ,/* Export file */
gen_util_file_type_pfile     ,/* Parser file */
gen_util_file_type_c_i_file  ,/* C file to be included in another C file     */
gen_util_file_type_cpp_i_file,/* C++ file to be included in another C++ file */
gen_util_file_type_dyn_hfile ,/* Header files in generate/dyn folder         */

gen_util_file_type_last = gen_util_file_type_dyn_hfile,

} GEN_UTIL_FILE_TYPE;

/* The following macros lists the base name of files generated */
/* by Open64 code generator.                                   */
#define FNAME_TOPCODE              "topcode"

#define FNAME_TARG                 "targ_"
#define FNAME_STUB                 "stub_"
#define FNAME_ISA                  "isa_"
#define FNAME_ABI                  "abi_"
#define FNAME_CG                   "cg_"

#define FNAME_ISA_PROPERTIES        FNAME_ISA "properties"
#define FNAME_ISA_VARIANTS          FNAME_ISA "variants"
#define FNAME_ISA_LITS              FNAME_ISA "lits"
#define FNAME_ISA_PRINT             FNAME_ISA "print"
#define FNAME_ISA_FCT_PTR           FNAME_ISA "fct_ptr"
#define FNAME_ISA_SUBSET            FNAME_ISA "subset"
#define FNAME_ISA_REGISTERS         FNAME_ISA "registers"
#define FNAME_ISA_OPERANDS          FNAME_ISA "operands"
#define FNAME_ISA_SELECTOR          FNAME_ISA "selector"
#define FNAME_ISA_ENUMS             FNAME_ISA "enums"
#define FNAME_ISA_HAZARDS           FNAME_ISA "hazards"
#define FNAME_ISA_SI                FNAME_ISA "si"
#define FNAME_ISA_BUNDLE            FNAME_ISA "bundle"
#define FNAME_ISA_TOPCODE           FNAME_ISA FNAME_TOPCODE
#define FNAME_ISA_PACK              FNAME_ISA "pack"
#define FNAME_ISA_DECODE            FNAME_ISA "decode"
#define FNAME_ISA_PARSE             FNAME_ISA "parse"
#define FNAME_ISA_RELOCS            FNAME_ISA "relocs"
#define FNAME_ISA_BINUTILS          FNAME_ISA "binutils"
#define FNAME_ABI_PROPERTIES        FNAME_ABI "properties"
#define FNAME_CG_TOP_MULTI          FNAME_CG  "top_multi"

#define FNAME_TARG_ISA_PROPERTIES   FNAME_TARG  FNAME_ISA_PROPERTIES
#define FNAME_STUB_ISA_PROPERTIES   FNAME_STUB  FNAME_ISA_PROPERTIES
#define FNAME_TARG_ISA_VARIANTS     FNAME_TARG  FNAME_ISA_VARIANTS
#define FNAME_STUB_ISA_VARIANTS     FNAME_STUB  FNAME_ISA_VARIANTS
#define FNAME_TARG_ISA_LITS         FNAME_TARG  FNAME_ISA_LITS
#define FNAME_STUB_ISA_LITS         FNAME_STUB  FNAME_ISA_LITS
#define FNAME_TARG_ISA_PRINT        FNAME_TARG  FNAME_ISA_PRINT
#define FNAME_STUB_ISA_PRINT        FNAME_STUB  FNAME_ISA_PRINT
#define FNAME_TARG_ISA_FCT_PTR      FNAME_TARG  FNAME_ISA_FCT_PTR
#define FNAME_STUB_ISA_FCT_PTR      FNAME_STUB  FNAME_ISA_FCT_PTR
#define FNAME_TARG_ISA_SUBSET       FNAME_TARG  FNAME_ISA_SUBSET
#define FNAME_STUB_ISA_SUBSET       FNAME_STUB  FNAME_ISA_SUBSET
#define FNAME_TARG_ISA_REGISTERS    FNAME_TARG  FNAME_ISA_REGISTERS
#define FNAME_STUB_ISA_REGISTERS    FNAME_STUB  FNAME_ISA_REGISTERS
#define FNAME_TARG_ISA_OPERANDS     FNAME_TARG  FNAME_ISA_OPERANDS
#define FNAME_STUB_ISA_OPERANDS     FNAME_STUB  FNAME_ISA_OPERANDS
#define FNAME_TARG_ISA_SELECTOR     FNAME_TARG  FNAME_ISA_SELECTOR
#define FNAME_TARG_ISA_ENUMS        FNAME_TARG  FNAME_ISA_ENUMS
#define FNAME_STUB_ISA_ENUMS        FNAME_STUB  FNAME_ISA_ENUMS
#define FNAME_TARG_ISA_SI           FNAME_TARG  FNAME_ISA_SI
#define FNAME_STUB_ISA_SI           FNAME_STUB  FNAME_ISA_SI
#define FNAME_TARG_ISA_BUNDLE       FNAME_TARG  FNAME_ISA_BUNDLE
#define FNAME_STUB_ISA_BUNDLE       FNAME_STUB  FNAME_ISA_BUNDLE
#define FNAME_TARG_ISA_HAZARDS      FNAME_TARG  FNAME_ISA_HAZARDS
#define FNAME_STUB_ISA_HAZARDS      FNAME_STUB  FNAME_ISA_HAZARDS
#define FNAME_TARG_ISA_PACK         FNAME_TARG  FNAME_ISA_PACK
#define FNAME_STUB_ISA_PACK         FNAME_STUB  FNAME_ISA_PACK
#define FNAME_TARG_ISA_DECODE       FNAME_TARG  FNAME_ISA_DECODE
#define FNAME_STUB_ISA_DECODE       FNAME_STUB  FNAME_ISA_DECODE
#define FNAME_TARG_ISA_PARSE        FNAME_TARG  FNAME_ISA_PARSE
#define FNAME_STUB_ISA_PARSE        FNAME_STUB  FNAME_ISA_PARSE
#define FNAME_TARG_ISA_RELOCS       FNAME_TARG  FNAME_ISA_RELOCS
#define FNAME_STUB_ISA_RELOCS       FNAME_STUB  FNAME_ISA_RELOCS
#define FNAME_TARG_ISA_BINUTILS     FNAME_TARG  FNAME_ISA_BINUTILS
#define FNAME_STUB_ISA_BINUTILS     FNAME_STUB  FNAME_ISA_BINUTILS
#define FNAME_TARG_ABI_PROPERTIES   FNAME_TARG  FNAME_ABI_PROPERTIES
#define FNAME_STUB_ABI_PROPERTIES   FNAME_STUB  FNAME_ABI_PROPERTIES
#define FNAME_TARG_CG_TOP_MULTI     FNAME_TARG  FNAME_CG_TOP_MULTI
#define FNAME_STUB_CG_TOP_MULTI     FNAME_STUB  FNAME_CG_TOP_MULTI

/* Managing dynamic extensions */
extern bool  Is_Static_Code           (void         );
extern bool  Is_Dynamic_Code          (void         );
extern char* Get_Extension_Name       (void         );
extern void  Set_Dynamic              (char* extname);
extern void  Gen_Close_File_Handle    (FILE* f, const char * const name);
extern FILE* Gen_Open_File_Handle     (const char * const filename,
                                       const char * const mode);
extern char* Gen_Build_Filename       (const char * const bodyname,
                                       const char * const extname,
                                       const GEN_UTIL_FILE_TYPE type);
extern void  Gen_Free_Filename        (char *filename);
extern void  Emit_C_Header            (FILE* hfile);
extern void  Emit_C_Footer            (FILE* hfile);

extern void  Emit_Stub_Header         (FILE *file, const char **headers);

/* String manipulation */
extern void  Remove_Filename_Extension(const char * src_fname,
                                       char       * tgt_fname);

#ifdef __cplusplus
}
#endif
#endif /* gen_util_INCLUDED */
