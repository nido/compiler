/*

  Copyright (C) 2005-2006 ST Microelectronics, Inc.  All Rights Reserved.

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

  Contact information:  ST Microelectronics, Inc., 
  address, or:

  http://www.st.com

*/


#include "defs.h"
#include "config.h"
#include "errors.h"
#include "erglob.h"
#include "tracing.h"
#include "data_layout.h"
#include "wn.h"
#include "wn_util.h"
#include "const.h"
#include "intrn_info.h"
#include "targ_emulate.h"


/* ============================================================
 *
 * Utilitary functions.
 * Contains code extracted from emulate.cxx.
 *
 * ============================================================ */

/*
 * Returns whether the opcode is an intrinsic op/call,
 */
#define OPCODE_is_intrinsic(op)                                         \
                ((OPCODE_operator((op)) == OPR_INTRINSIC_CALL) ||       \
                (OPCODE_operator((op)) == OPR_INTRINSIC_OP))

/*
 * WN_arg()
 *
 * return Nth kid , skiping PARM
 */
static WN *
WN_arg(WN *tree, INT32 arg)
{
  WN	*child= WN_kid(tree, arg);

  if (WN_operator_is(child, OPR_PARM))
  {
    return WN_kid0(child);
  }

  return child;
}


/* ============================================================
 *
 * Implementation of target specific intrinsics.
 *
 * ============================================================ */


/* ============================================================
 *
 * Exported interface.
 *
 * ============================================================ */

/*
 * BETARG_emulate_intrinsic_op()
 *
 * See Interface Description.
 */
WN *
BETARG_emulate_intrinsic_op(WN *block, WN *tree)
{
  INTRINSIC     id = (INTRINSIC) WN_intrinsic(tree);
  TYPE_ID	rtype = WN_rtype(tree);
  WN		*function;

  DevAssert((OPCODE_is_intrinsic(WN_opcode(tree))),
	    ("expected intrinsic call node, not %s", 
	     OPCODE_name(WN_opcode(tree))));
  DevAssert((INTRN_is_actual(WN_intrinsic(tree))==FALSE),
	    ("cannot emulate INTRN_is_actual"));
  
  switch(id) {
    /* Nothing specific for ST200. */
  default:
    break;
  }
  return NULL;
}
