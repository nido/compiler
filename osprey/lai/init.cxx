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


// This file contains only Linux-specific code and should be entirely
// #ifdef'd out for Irix.

// Work around the "undefined weak symbol" bug in Linux.
//
// see comments in be/com/weak.cxx.
//
// This file define initialization of pointer variables to symbols defined
// in lai.so but referenced in be/be.so.

#ifdef __linux__

#include "defs.h"
#include "wn.h"
#include "lai.h"
#include "eh_region.h"

// from be/lai/lai_driver.h
extern void (*LAI_Init_p) ();
extern void (*LAI_Fini_p) ();
extern void (*LAI_Process_Command_Line_p) (INT, char **, INT, char **);
extern void (*LAI_PU_Initialize_p) (WN*);
extern void (*LAI_PU_Finalize_p) ();
extern WN* (*LAI_Emit_Code_p) (WN*, ALIAS_MANAGER*, DST_IDX, BOOL);

// from be/cg/eh_region.h
extern void (*EH_Generate_Range_List_p) (WN *);

/*
 * This function is recognized by the Linux linker and placed in the
 * .init section of the .so. Thus, executed upon mapping the .so
 */
struct LAI_INIT {
  LAI_INIT () {
    LAI_Init_p = LAI_Init;
    LAI_Fini_p = LAI_Fini;
    LAI_Process_Command_Line_p = LAI_Process_Command_Line;
    LAI_PU_Initialize_p = LAI_PU_Initialize;
    LAI_PU_Finalize_p = LAI_PU_Finalize;
    LAI_Emit_Code_p = LAI_Emit_Code;
    EH_Generate_Range_List_p = EH_Generate_Range_List;
  }
} Lai_Initializer;


#endif // __linux__
