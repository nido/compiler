/*

  Copyright (C) 2001 ST Microelectronics, Inc.  All Rights Reserved.

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

  Contact information: 
  or:

  http://www.st.com

  For further information regarding this notice, see:


*/

#include <elf.h>
#include "defs.h"
#include "stab.h"
#include "stblock.h"
#include "config_targ.h"
#include "data_layout.h"
#include "sections.h"
#include "targ_sections.h"
#include "targ_isa_bundle.h" // for ISA_INST_BYTES

/* ====================================================================
 *    Corresponding_Short_Section
 * ====================================================================
 */
SECTION_IDX
Corresponding_Short_Section (
  ST *st,
  SECTION_IDX sec
)
{
  SECTION_IDX newsec;

  switch ( sec ) {
   case _SEC_DATA:      newsec = _SEC_SDATA;
   // cygnus doesn't handle srdata
   // case _SEC_RDATA:     newsec = _SEC_SRDATA;
   case _SEC_BSS:       newsec = _SEC_SBSS;
   default:		newsec == sec;
   }

  if (newsec == sec) return sec;  // won't be shortened
#if 0
  if (sec == _SEC_SDATA1 && ST_class(st) == CLASS_CONST) {
     /* by default put all short .rodata items into .srdata, unless 
      * we can put it into an appropriate merge section.
      */
     TCON tcon = STC_val(st);
     switch (TCON_ty (tcon)) {
       case MTYPE_F4:
       case MTYPE_I4:
       case MTYPE_U4:
	 newsec = _SEC_SDATA4;
	 break;
       case MTYPE_F8:
       case MTYPE_I8:
       case MTYPE_U8:
	 newsec = _SEC_SDATA4;
	 break;
       case MTYPE_FQ:
	 newsec = _SEC_SDATA4;
	 break;
     }
   }
#endif
   Set_ST_gprel(st);
   return newsec;
}

/* ====================================================================
 *    Assign_Static_Variable
 * ====================================================================
 */
SECTION_IDX 
Assign_Static_Variable (ST *st)
{
  SECTION_IDX sec;

  if (ST_is_thread_private(st)) {
    FmtAssert(FALSE,("Assign_Static_Variable: thread private ST"));
  }
  else if (ST_is_initialized(st) && !ST_init_value_zero (st))
    sec = (ST_is_constant(st) ? _SEC_RDATA : _SEC_DATA);
  else
    sec = _SEC_BSS;

  return sec;
}

/* ====================================================================
 *    Assign_Global_Variable
 * ====================================================================
 */
SECTION_IDX 
Assign_Global_Variable (
  ST *st, 
  ST *base_st
)
{
  SECTION_IDX sec;

  switch ( ST_sclass(base_st) ) {
  case SCLASS_UGLOBAL:
    if (ST_is_thread_private(st)) {
      FmtAssert(FALSE,("Assign_Global_Variable: thread private ST"));
    } 
    else sec = _SEC_BSS;
    break;

  case SCLASS_DGLOBAL:
    if (ST_is_thread_private(st)) {
      FmtAssert(FALSE,("Assign_Global_Variable: thread private ST"));
      //sec = _SEC_LDATA;
    }
    else if (ST_is_constant(st)) sec = _SEC_RDATA;
    else sec = _SEC_DATA;
    break;

  default:
    FmtAssert(FALSE,("Assign_Global_Variable: SCLASS"));
  }

  return sec;
}

