/*

  Copyright (C) 2000, 2001 Silicon Graphics, Inc.  All Rights Reserved.

   Path64 is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   Path64 is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with Path64; see the file COPYING.  If not, write to the Free
   Software Foundation, 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.

   Special thanks goes to SGI for their continued support to open source

*/


#ifdef USE_PCH
#include "be_com_pch.h"
#endif /* USE_PCH */
#pragma hdrstop

#include "whirl2src.h"

#include "dso.h"	// load_so

#include "w2c_weak.h"	// W2C_
#include "w2f_weak.h"	// W2F_

extern BOOL Show_Progress;

static BOOL init_whirl2c = FALSE;   /* Has whirl2c been initialized already? */
static BOOL init_whirl2f = FALSE;   /* Has whirl2f been initialized already? */
static WN* w2src_func_nd = NULL;    /* The current PU being processed */

/***********************************************************************
 *
 * Initialize whirl2c for processing: 
 *  - load the DSO if it hasn't been loaded yet
 *  - store the supplied PU -- it is used by the _Emit routines.
 *  - store the fact that the DSO has been loaded, so that 
 *    subsequent calls do not try to load it again.
 *
 ***********************************************************************/
extern void 
Whirl2C_Init (WN* func_nd) 
{
  w2src_func_nd = func_nd;
  if (!init_whirl2c) {
    if (W2C_Process_Command_Line == NULL) {
      /* load and initialize whirl2c */
      extern char *W2C_Path;
      const char* const str = "";
      load_so("whirl2c.so", W2C_Path, Show_Progress);
      W2C_Process_Command_Line(0, &str, 0, &str);
      W2C_Init ();
    }
    init_whirl2c = TRUE;
  }
}

/***********************************************************************
 *
 * Initialize whirl2f for processing: 
 *  - load the DSO if it hasn't been loaded yet
 *  - store the supplied PU -- it is used by the _Emit routines.
 *  - store the fact that the DSO has been loaded, so that 
 *    subsequent calls do not try to load it again.
 *
 ***********************************************************************/
extern void 
Whirl2F_Init (WN* func_nd) 
{
  w2src_func_nd = func_nd;
  if (!init_whirl2f) {
    if (W2F_Process_Command_Line == NULL) {
      /* load and initialize whirl2f */
      extern char *W2F_Path;
      const char* str;
      str = "";
      load_so("whirl2f.so", W2F_Path, Show_Progress);
      W2F_Process_Command_Line(0, &str, 0, &str);
      W2F_Init ();
    }
    init_whirl2f = TRUE;
  }
}

/***********************************************************************
 *
 * Based on the source language of the current PU, 
 * initialize either whirl2c or whirl2f.
 *
 ***********************************************************************/
extern void 
Whirl2Src_Init (WN* func_nd) 
{
  switch (PU_src_lang(Get_Current_PU())) {
  case PU_C_LANG:
  case PU_CXX_LANG:
    Whirl2C_Init (func_nd);
    break;
  case PU_F90_LANG:
  case PU_F77_LANG:
    Whirl2F_Init (func_nd);
    break;
  default:
    FmtAssert (FALSE, ("Unknown source language type"));
    break;
  }
}

/***********************************************************************
 *
 * Emit the supplied wn in C using whirl2c into the supplied FILE*.
 *
 ***********************************************************************/
extern void 
Whirl2C_Emit (FILE* fp, WN* wn) 
{
  if (!init_whirl2c) {
    Is_True (FALSE, ("Whirl2C_Emit: whirl2c not initialized\n"));
    return;
  }
  W2C_Push_PU (w2src_func_nd, wn);
  W2C_Translate_Wn(fp, wn);
  W2C_Pop_PU();
  return;
}

/***********************************************************************
 *
 * Emit the supplied wn in Fortran using whirl2f into the supplied FILE*.
 *
 ***********************************************************************/
extern void 
Whirl2F_Emit (FILE* fp, WN* wn) 
{
  if (!init_whirl2f) {
    Is_True (FALSE, ("Whirl2F_Emit: whirl2f not initialized\n"));
    return;
  }
  W2F_Push_PU (w2src_func_nd, wn);
  W2F_Translate_Wn(fp, wn);
  W2F_Pop_PU();
  return;
}

/***********************************************************************
 *
 * Emit the supplied wn in source form into the supplied FILE*.
 *
 ***********************************************************************/
extern void 
Whirl2Src_Emit (FILE* fp, WN* wn) 
{
  switch (PU_src_lang(Get_Current_PU())) {
  case PU_C_LANG:
  case PU_CXX_LANG:
    Whirl2C_Emit (fp, wn);
    break;
  case PU_F90_LANG:
  case PU_F77_LANG:
    Whirl2F_Emit (fp, wn);
    break;
  default:
    FmtAssert (FALSE, ("Unknown source language type"));
    break;
  }
}

#if ! (defined(__linux__) || defined(BUILD_OS_DARWIN) || defined(__FreeBSD__) || defined(__sun))

// These functions are needed only for prompf_anl.so

/***********************************************************************
 *
 * This is a dummy routine that calls the following two routines:
 *  W2C_Translate_Istore_Lhs
 *  W2F_Translate_Istore_Lhs
 * thereby forcing them to actually get "weak"ly defined in the .o
 *
 ***********************************************************************/
static void 
dummy () 
{
  W2C_Translate_Istore_Lhs ((char*) NULL, (UINT) 0, (WN*) NULL, (WN_OFFSET) 0, 
                            (TY_IDX) NULL, (TYPE_ID) 0);
  W2F_Translate_Istore_Lhs ((char*) NULL, (UINT) 0, (WN*) NULL, (WN_OFFSET) 0, 
                            (TY_IDX) NULL, (TYPE_ID) 0);
  W2C_Translate_Wn_Str ((char*) NULL, (UINT) 0, (WN*) NULL);
  W2F_Translate_Wn_Str ((char*) NULL, (UINT) 0, (WN*) NULL);
}

extern void 
Whirl2Src_Translate_Iload_Str(char  *str_buf,
                              UINT  buf_size,
                              WN*   iload) 
{
  INT i;
  for (i=0; i<buf_size; i++) str_buf[i]='\0';
  switch (PU_src_lang(Get_Current_PU())) {
    case PU_C_LANG:
    case PU_CXX_LANG:
      W2C_Push_PU (w2src_func_nd, iload);
      W2C_Translate_Wn_Str(str_buf, buf_size, iload);
      W2C_Pop_PU ();
      break;
    case PU_F90_LANG:
    case PU_F77_LANG:
      W2F_Push_PU (w2src_func_nd, iload);
      W2F_Translate_Wn_Str(str_buf, buf_size, iload);
      W2F_Pop_PU ();
      break;
    default:
      FmtAssert (FALSE, ("Unknown source language type"));
  }
}

extern void 
Whirl2Src_Translate_Istore_Str(char *str_buf,
                               UINT  buf_size,
                               WN*   istore) 
{
  INT i;
  for (i=0; i<buf_size; i++) str_buf[i]='\0';
  switch (PU_src_lang(Get_Current_PU())) {
    case PU_C_LANG:
    case PU_CXX_LANG:
      W2C_Push_PU (w2src_func_nd, istore);
      W2C_Translate_Istore_Lhs(str_buf, buf_size,  WN_kid1(istore),
                               WN_offset(istore), WN_ty(istore),
                               WN_desc(istore));
      W2C_Pop_PU ();
      break;
    case PU_F90_LANG:
    case PU_F77_LANG:
      W2F_Push_PU (w2src_func_nd, istore);
      W2F_Translate_Istore_Lhs(str_buf, buf_size,  WN_kid1(istore),
                               WN_offset(istore), WN_ty(istore),
                               WN_desc(istore));
      W2F_Pop_PU ();
      break;
    default:
      FmtAssert (FALSE, ("Unknown source language type"));
  }
}

#endif // __linux__
