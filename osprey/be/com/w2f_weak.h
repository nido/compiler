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


#ifndef w2f_weak_INCLUDED
#define w2f_weak_INCLUDED

// See comments in be/com/weak.cxx

// Symbols exported from whirl2f.so  
// and used in be, be.so and lno.so
// (additional ones may be needed for prompf_anl.so)

#if defined(__linux__) || defined(_NO_WEAK_SUPPORT_)

BE_EXPORTED extern void (*W2F_Cleanup_p)(void);
BE_EXPORTED extern void (*W2F_Init_p)(void);
BE_EXPORTED extern void (*W2F_Outfile_Fini_p)(void);
BE_EXPORTED extern void (*W2F_Outfile_Init_p)(void);
BE_EXPORTED extern void (*W2F_Outfile_Translate_Pu_p)(WN *pu);
BE_EXPORTED extern void (*W2F_Pop_PU_p)(void);
BE_EXPORTED extern void (*W2F_Process_Command_Line_p)(INT phase_argc, char *phase_argv[], INT argc, char *argv[]);
BE_EXPORTED extern void (*W2F_Push_PU_p)(WN *pu, WN *body_part_of_interest);
BE_EXPORTED extern void (*W2F_Set_Frequency_Map_p)(WN_MAP frequency_map);
BE_EXPORTED extern void (*W2F_Set_Prompf_Emission_p)(WN_MAP *construct_map);
BE_EXPORTED extern BOOL (*W2F_Should_Emit_Nested_PUs_p)(void);
BE_EXPORTED extern void (*W2F_Translate_Istore_Lhs_p)(char *strbuf, UINT bufsize, WN* lhs, mINT64 istore_ofst, TY_IDX istore_addr_ty, TYPE_ID istore_mtype);
BE_EXPORTED extern void (*W2F_Translate_Wn_p)(FILE *outfile, WN *wn);
BE_EXPORTED extern void (*W2F_Translate_Wn_Str_p)(char *strbuf, UINT bufsize, WN *wn);

#define W2F_Cleanup (*W2F_Cleanup_p)
#define W2F_Init (*W2F_Init_p)
#define W2F_Outfile_Fini (*W2F_Outfile_Fini_p)
#define W2F_Outfile_Init (*W2F_Outfile_Init_p)
#define W2F_Outfile_Translate_Pu (*W2F_Outfile_Translate_Pu_p)
#define W2F_Pop_PU (*W2F_Pop_PU_p)
#define W2F_Process_Command_Line (*W2F_Process_Command_Line_p)
#define W2F_Push_PU (*W2F_Push_PU_p)
#define W2F_Set_Frequency_Map (*W2F_Set_Frequency_Map_p)
#define W2F_Set_Prompf_Emission (*W2F_Set_Prompf_Emission_p)
#define W2F_Should_Emit_Nested_PUs (*W2F_Should_Emit_Nested_PUs_p)
#define W2F_Translate_Istore_Lhs (*W2F_Translate_Istore_Lhs_p)
#define W2F_Translate_Wn (*W2F_Translate_Wn_p)
#define W2F_Translate_Wn_Str (*W2F_Translate_Wn_Str_p)

#else // __linux__

#pragma weak W2F_Cleanup
#pragma weak W2F_Init
#pragma weak W2F_Outfile_Fini
#pragma weak W2F_Outfile_Init
#pragma weak W2F_Outfile_Translate_Pu
#pragma weak W2F_Pop_PU
#pragma weak W2F_Process_Command_Line
#pragma weak W2F_Push_PU
#pragma weak W2F_Set_Frequency_Map
#pragma weak W2F_Set_Prompf_Emission
#pragma weak W2F_Should_Emit_Nested_PUs
#pragma weak W2F_Translate_Istore_Lhs
#pragma weak W2F_Translate_Wn
#pragma weak W2F_Translate_Wn_Str

#endif // __linux__

#endif // w2f_weak_INCLUDED