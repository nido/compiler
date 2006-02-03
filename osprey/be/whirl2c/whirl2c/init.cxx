/*

  Copyright (C) 2000, 2001 Silicon Graphics, Inc.  All Rights Reserved.

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


// This file contains only Linux-specific code and should be entirely
// #ifdef'd out for Irix.

// Work around the "undefined weak symbol" bug in Linux.
//
// see comments in be/com/weak.cxx.
//
// This file define initialization of pointer variables to symbols defined
// in lno.so but referenced in be/be.so.

#if defined(__linux__) || defined(_NO_WEAK_SUPPORT_)

#include "defs.h"
#include "wn.h"
#include "pu_info.h"
#include "w2c_driver.h"

BE_EXPORTED extern void (*W2C_Cleanup_p)(void);
BE_EXPORTED extern void (*W2C_Fini_p)(void);
BE_EXPORTED extern void (*W2C_Init_p)(void);
BE_EXPORTED extern const char * (*W2C_Object_Name_p)(const ST *func_st);
BE_EXPORTED extern void (*W2C_Mark_Loads_p)(void);
BE_EXPORTED extern void (*W2C_Nomark_Loads_p)(void);
BE_EXPORTED extern void (*W2C_Outfile_Fini_p)(BOOL emit_global_decls);
BE_EXPORTED extern void (*W2C_Outfile_Init_p)(BOOL emit_global_decls);
BE_EXPORTED extern void (*W2C_Outfile_Translate_Pu_p)(WN *pu, BOOL emit_global_decls);
BE_EXPORTED extern void (*W2C_Pop_PU_p)(void);
BE_EXPORTED extern void (*W2C_Process_Command_Line_p)(INT phase_argc, char * const 
  phase_argv[], INT argc, char * const argv[]);
BE_EXPORTED extern void (*W2C_Push_PU_p)(const WN *pu, WN *body_part_of_interest);
BE_EXPORTED extern void (*W2C_Set_Prompf_Emission_p)(const WN_MAP *construct_map);
BE_EXPORTED extern void (*W2C_Set_Frequency_Map_p)(WN_MAP frequency_map);
BE_EXPORTED extern const char *(*W2C_Get_Transformed_Src_Path_p)(void);
BE_EXPORTED extern void (*W2C_Reset_Purple_Emission_p)(void);
BE_EXPORTED extern void (*W2C_Set_Purple_Emission_p)(void);
BE_EXPORTED extern BOOL (*W2C_Should_Emit_Nested_PUs_p)(void);
BE_EXPORTED extern void (*W2C_Translate_Global_Defs_p)(FILE *outfile);
BE_EXPORTED extern void (*W2C_Translate_Global_Types_p)(FILE *outfile);
BE_EXPORTED extern void (*W2C_Translate_Istore_Lhs_p)(char *strbuf,
  UINT bufsize, const WN* lhs, mINT64 istore_ofst, TY_IDX istore_addr_ty,
  TYPE_ID istore_mtype);
BE_EXPORTED extern void (*W2C_Translate_Purple_Main_p)(FILE *outfile, const WN *pu, 
  const char *region_name);
BE_EXPORTED extern void (*W2C_Translate_Stid_Lhs_p)(char *strbuf, UINT bufsize,
  const ST *stid_st, mINT64 stid_ofst, TY_IDX stid_ty, TYPE_ID stid_mtype);
BE_EXPORTED extern void (*W2C_Translate_Wn_p)(FILE *outfile, const WN *wn);
BE_EXPORTED extern void (*W2C_Translate_Wn_Str_p)(char *strbuf, UINT bufsize, const
  WN *wn);

struct W2C_INIT
{
    W2C_INIT () {
      W2C_Cleanup_p = W2C_Cleanup;
      W2C_Fini_p = W2C_Fini;
      W2C_Init_p = W2C_Init;
      W2C_Object_Name_p = W2C_Object_Name;
      W2C_Mark_Loads_p = W2C_Mark_Loads;
      W2C_Nomark_Loads_p = W2C_Nomark_Loads;
      W2C_Outfile_Fini_p = W2C_Outfile_Fini;
      W2C_Outfile_Init_p = W2C_Outfile_Init;
      W2C_Outfile_Translate_Pu_p = W2C_Outfile_Translate_Pu;
      W2C_Pop_PU_p = W2C_Pop_PU;
      W2C_Process_Command_Line_p = W2C_Process_Command_Line;
      W2C_Push_PU_p = W2C_Push_PU;
      W2C_Set_Prompf_Emission_p = W2C_Set_Prompf_Emission;
      W2C_Set_Frequency_Map_p = W2C_Set_Frequency_Map;
      W2C_Get_Transformed_Src_Path_p = W2C_Get_Transformed_Src_Path;
      W2C_Reset_Purple_Emission_p = W2C_Reset_Purple_Emission;
      W2C_Set_Purple_Emission_p = W2C_Set_Purple_Emission;
      W2C_Should_Emit_Nested_PUs_p = W2C_Should_Emit_Nested_PUs;
      W2C_Translate_Global_Defs_p = W2C_Translate_Global_Defs;
      W2C_Translate_Global_Types_p = W2C_Translate_Global_Types;
      W2C_Translate_Istore_Lhs_p = W2C_Translate_Istore_Lhs;
      W2C_Translate_Purple_Main_p = W2C_Translate_Purple_Main;
      W2C_Translate_Stid_Lhs_p = W2C_Translate_Stid_Lhs;
      W2C_Translate_Wn_p = W2C_Translate_Wn;
      W2C_Translate_Wn_Str_p = W2C_Translate_Wn_Str;
    }
} W2C_Initializer;

#endif // __linux__ || _NO_WEAK_SUPPORT_
