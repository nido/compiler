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
#include "config_target.h"
#include "data_layout.h"
#include "sections.h"
#include "be_symtab.h"
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
  /* clarkes:
   * For ST200, assume all data can be placed in short sections.
   * Note 1: elsewhere the compiler assumes that it is not possible to
   * use gp-relative accesses for data that is not in a short section.
   * Note 2: Corresponding_Short_Section is only ever called for
   * symbols that can not be preempted, so for ST200 set gprel
   * attribute for every symbol that reaches here.
   */
  SECTION_IDX newsec;

  switch (sec) {
  case _SEC_DATA:  newsec = _SEC_SDATA;  break;
  case _SEC_RDATA: newsec = _SEC_SRDATA; break;
  case _SEC_BSS:   newsec = _SEC_SBSS;   break;
  default:         newsec = sec;         break;
  }

  Set_ST_gprel(st);
  return newsec;
}

/* ====================================================================
 *    Initv_Contains_Address
 * ====================================================================
 */
static BOOL
Initv_Contains_Address (INITV_IDX initv_idx)
{
  for ( ; initv_idx != INITV_IDX_ZERO; initv_idx = INITV_next (initv_idx)) {
    switch (INITV_kind(Initv_Table[initv_idx])) {
    case INITVKIND_SYMOFF:
    case INITVKIND_LABEL:
      return TRUE;
    case INITVKIND_ZERO:
    case INITVKIND_ONE:
    case INITVKIND_VAL:
    case INITVKIND_PAD:
      break;
    case INITVKIND_BLOCK:
      if (Initv_Contains_Address (INITV_blk (initv_idx)))
	return TRUE;
      else
	break;
    case INITVKIND_SYMDIFF:
    case INITVKIND_SYMDIFF16:
      break;
    default:
      FmtAssert(FALSE, ("Initializer_Contains_Address: unknown initvkind"));
      break;
    }
  }
  return FALSE;
}    

/* ====================================================================
 *    Initializer_Contains_Address
 * ====================================================================
 */
static BOOL
Initializer_Contains_Address (ST *st)
{
  INITV_IDX initv_idx;
  if (!ST_is_initialized (st)
      || ST_init_value_zero (st)
      || ST_class (st) == CLASS_CONST)
    return FALSE;

  initv_idx = ST_has_initv (st);

  if (initv_idx == 0)
    // Cannot find initializer.
    // Be pessimistic: assume it contains an address.  
    // This can happen for jump tables, as the initializer is not created
    // until after we have chosen the section for the jump table.
    // Another possibility here: look at the type of the symbol to see if
    // the type contains pointers.
    // FmtAssert(FALSE,("Initializer_Contains_Address: cannot find initializer"));
    return TRUE;

  return Initv_Contains_Address (initv_idx);
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
  else if (ST_is_initialized(st)
	   && !ST_init_value_zero (st))
    sec = (ST_is_constant(st)
	   && ! (Gen_GP_Relative && Initializer_Contains_Address (st)))
      ? _SEC_RDATA : _SEC_DATA;
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
    else if (ST_is_constant(st)
	     && !(Gen_GP_Relative && Initializer_Contains_Address (st)))
      sec = _SEC_RDATA;
    else sec = _SEC_DATA;
    break;

  default:
    FmtAssert(FALSE,("Assign_Global_Variable: SCLASS"));
  }

  return sec;
}

