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

  Contact information:  or:

  http://www.st.com

  For further information regarding this notice, see:

  http://

*/

/* ====================================================================
 * ====================================================================
 *
 *   Description:
 *
 *   Target-dependent functions used by the HyperBlock Formation code.
 * ====================================================================
 * ====================================================================
 */

#include <ctype.h>

#include "defs.h"
#include "util.h"
#include "config.h"
#include "config_TARG.h"
#include "erglob.h"
#include "tracing.h"
#include "cgir.h"
#include "cg.h"
#include "void_list.h"
#include "bb.h"
#include "op.h"
#include "op_list.h"
#include "cgtarget.h"

/* ====================================================================
 *   CGTARG_Check_OP_For_HB_Suitability
 *
 *   Returns TRUE if OP is a suitable candidate for HBF. 
 *   Otherwise, return FALSE.
 * ====================================================================
 */
BOOL 
CGTARG_Check_OP_For_HB_Suitability (
  OP *op
)
{
  //  extern void dump_OP(const OP *op);
  //  fprintf (TFile, "CGTARG_Check_OP_For_HB_Suitability op \n");
  //  dump_OP (op);
#ifdef IFCONV_IN_SSA
  //
  // Arthur: we won't if-convert all of'em
  //
  if (OP_memory(op))
    return FALSE;
#endif

  return TRUE; 
}

