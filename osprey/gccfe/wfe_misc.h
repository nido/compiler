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


/* translate gnu decl trees to whirl */

#ifndef wfe_util_INCLUDED
#define wfe_util_INCLUDED

/* add some typedefs to avoid including C++ specifi items */
#ifndef __cplusplus
typedef int INT;
typedef signed int INT32;
typedef unsigned long long UINT64;
typedef int BOOL;
typedef struct wn WN;
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define Check_GCC_Error() \
  do { extern int errorcount; if (errorcount != 0) return; } while(0)

extern int  WFE_Keep_Zero_Length_Structs;

#ifdef TARG_ST
extern char * Orig_Src_File_Name;
extern void WFE_Prepare_Gcc_Options(int argc, char **argv, int *gnu_argc, char ***gnu_argv);
extern void WFE_Init_Errors ();
#endif
extern void WFE_Init_Errors ();
extern void WFE_Init (INT argc, char **arrgv);
extern void WFE_Finish (void);
extern void WFE_File_Init (INT argc, char **argv);
extern void WFE_File_Finish (void);
extern void WFE_Check_Errors (int *error_count, int *warning_count, BOOL *need_inliner);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

#include "wn.h"
#include "srcpos.h"
#ifdef TARG_ST // [CL]
#include "wfe_dst.h"
#endif

typedef enum {
  wfe_stmk_unknown,
  wfe_stmk_func_entry,
  wfe_stmk_func_pragmas,
#ifdef TARG_ST
  wfe_stmk_call_region_body,
#endif 
  wfe_stmk_func_body,
  wfe_stmk_region_pragmas,
  wfe_stmk_scope,
  wfe_stmk_if_cond,
  wfe_stmk_if_then,
  wfe_stmk_if_else,
  wfe_stmk_while_cond,
  wfe_stmk_while_body,
  wfe_stmk_dowhile_cond,
  wfe_stmk_dowhile_body,
  wfe_stmk_for_cond,
  wfe_stmk_for_body,
  wfe_stmk_switch,
  wfe_stmk_comma,
  wfe_stmk_rcomma,
  wfe_stmk_last
} WFE_STMT_KIND;

typedef struct wn_stmt {
  WN            *wn;
  WFE_STMT_KIND  kind;
} WN_STMT;

extern WN_STMT *WFE_Get_Stmt(void);
extern void WFE_Stmt_Push (WN* wn, WFE_STMT_KIND kind, SRCPOS srcpos);
extern WN*  WFE_Stmt_Top (void);
extern void WFE_Stmt_Append (WN* wn, SRCPOS srcpos);
extern WN*  WFE_Stmt_Last (void);
extern WN*  WFE_Stmt_Pop (WFE_STMT_KIND kind);

// FdF 20070302
extern void WFE_Stmt_Move_To_End(WN *first_wn, WN *last_wn);

#ifdef TARG_ST
extern bool Check_For_Call_Region ();
extern bool Did_Not_Terminate_Region;
#endif

extern UINT current_file;
extern int lineno;

extern int wfe_invoke_inliner;

// get the srcpos info from the global variable lineno
inline SRCPOS
Get_Srcpos (void)
{
  SRCPOS s;
  SRCPOS_clear(s);
#ifdef TARG_ST // [CL] use GCC's global vars as reference
  WFE_Set_Line_And_File (lineno, input_filename);
#endif
  SRCPOS_filenum(s) = current_file;
  SRCPOS_linenum(s) = lineno;
  return s;
}

#ifdef TARG_ST
// [CL] get the srcpos info from the tree
inline SRCPOS
Get_Srcpos_From_Tree (tree node)
{
  SRCPOS s;
  SRCPOS_clear(s);

  // [CL] We should not modify current_file here, but it is a side
  // effect of WFE_Set_Line_And_File(), which we use to build the
  // dir/file table. Keep a copy to restore it afterwards.
  UINT current_file_cache = current_file;

  FmtAssert (node != NULL, 
	     ("Get_Srcpos_From_Tree called with NULL node"));

  WFE_Set_Line_And_File (DECL_SOURCE_LINE(node), DECL_SOURCE_FILE(node));
  SRCPOS_filenum(s) = current_file;
  SRCPOS_linenum(s) = DECL_SOURCE_LINE(node);
  current_file = current_file_cache;
  return s;
}
#endif

#endif

#endif