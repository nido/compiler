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


/* ====================================================================
 * ====================================================================
 *
 *  TN utility routines which include target dependencies.
 *
 * ====================================================================
 * ====================================================================
 */

#include <elf.h>

#include "defs.h"
#include "config.h"
#include "erglob.h"
#include "xstats.h"
#include "tracing.h"

#include "strtab.h"
#include "tn.h"
#include "tn_list.h"
#include "ttype.h"

#include "const.h"
#include "targ_const.h"
#include "targ_sim.h"

#include "config_asm.h"

#include "em_elf.h"

#include "data_layout.h"

/* ====================================================================
 *   Gen_Predicate_TN
 * ====================================================================
 */
TN*
Gen_Predicate_TN()
{
  FmtAssert(FALSE,("Not implemented"));
#if 0
  //
  // We will generate a integer TN always, so we can conduct boolean
  // arithmetic on it
  //
  return Build_RCLASS_TN(ISA_REGISTER_CLASS_gr);

#endif
  return NULL;
}


/* ====================================================================
 *   CGTARG_TN_Value
 * ====================================================================
 */
INT64
CGTARG_TN_Value (
  TN      *t,
  INT64   base_ofst
)
{
  INT64 val = base_ofst + TN_offset(t);
  ST *base_st;

  Base_Symbol_And_Offset (TN_var(t), &base_st, &base_ofst);
#if 0
  if (base_st == SP_Sym ) {
    FmtAssert(val >= 0,("Cannot load/store with base SP and offset < 0"));
  }
  if (base_st == FP_Sym ) {
    if(val < 0) {
      /* Code selection should has generated load/store with base - ofst. */
      val = -val;
    }
  }
#endif

  FmtAssert (TN_is_reloc_none(t), ("CGTARG_TN_Value: unexpected reloc TN"));

  return val;

}

/* ====================================================================
 *   TN_Use_Base_ST_For_Reloc
 *
 *   whether to use the base st for the reloc
 * ====================================================================
 */
BOOL
TN_Use_Base_ST_For_Reloc (
  INT reloc, 
  ST *st
)
{
  return ST_is_export_local(st);
}

/* ====================================================================
 * TN_Reloc_has_parenthesis
 *   Returns true if reloc syntax is with parenthesis.
 *
 * ====================================================================
 */
BOOL
TN_Reloc_has_parenthesis( INT32 reloc ) {

  switch(reloc) {
  case ISA_RELOC_offset12PC:
  case ISA_RELOC_offset8PC:
    return FALSE;
    break;
  default:
    return TRUE;
  }

  return TRUE;
}

/* ====================================================================
 *   TN_Relocs_In_Asm
 * ====================================================================
 */
INT
TN_Relocs_In_Asm (
  TN *t, 
  ST *st, 
  vstring *buf, 
  INT64 *val
)
{
  INT paren = TN_Reloc_has_parenthesis(TN_relocs(t)) ? 1 : 0;	// num parens
  const char *str;
  const char *c;

  str = TN_RELOCS_Syntax(TN_relocs(t));
  FmtAssert (str, ("TN_Relocs_In_Asm: illegal reloc TN (%d)", (int)TN_relocs(t)));
  *buf = vstr_concat (*buf, str);
  for (c = strchr (str, '('); c; c = strchr (c + 1, '('))
    paren++;

  return paren;
}

//
// Various target-dependent dedicated:
// 


