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

/*
 * TN relocations info:
 */
const TN_RELOCS_INFO TN_RELOCS_info[] = {
  { ""                },   /* TN_RELOC_NONE          */
  { "@gprel"          },   /* TN_RELOC_GOT_DISP      */
  { "@gotoff"         },   /* TN_RELOC_GOTOFF        */
  { "@gotoff(@fptr"   },   /* TN_RELOC_GOTOFF_FPTR   */
  { NULL              },   /* TN_RELOC_GPIDENT       */
  { NULL              },   /* TN_RELOC_GPSUB         */
  { "@neggprel"       },   /* TN_RELOC_NEG_GOT_DISP  */
  { "@tprel"          },   /* TN_RELOC_TPREL         */
  { "@gotoff(@tprel"  },   /* TN_RELOC_GOTOFF_TPREL  */
  { "@gotoff(@dtpldm" },   /* TN_RELOC_GOTOFF_DTPLDM */
  { "@gotoff(@dtpndx" },   /* TN_RELOC_GOTOFF_DTPNDX */
  { "@dtprel"         },   /* TN_RELOC_DTPREL        */
};

/* ====================================================================
 *   Gen_Predicate_TN
 * ====================================================================
 */
TN*
Gen_Predicate_TN()
{
  //
  // We will generate a integer TN always, so we can conduct boolean
  // arithmetic on it
  //
  return Build_RCLASS_TN(ISA_REGISTER_CLASS_integer);
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
  INT paren = 1;	// num parens
  const char *str;
  const char *c;

  str = TN_RELOCS_info[TN_relocs(t)].name;
  FmtAssert (str, ("TN_Relocs_In_Asm: illegal reloc TN (%d)", (int)TN_relocs(t)));
  *buf = vstr_concat (*buf, str);
  for (c = strchr (str, '('); c; c = strchr (c + 1, '('))
    paren++;

  return paren;
}

//
// Various target-dependent dedicated:
// 


