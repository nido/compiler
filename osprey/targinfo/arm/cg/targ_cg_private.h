/*
  Copyright (C) 2006, STMicroelectronics, All Rights Reserved.

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
*/


/* ====================================================================
 * ====================================================================
 *
 * Module: targ_cg_private.h
 *
 * Description:
 *
 *   Target-specific support functions for rest of target code
 *   generator.
 *   Not exported outside of target code generator.
 *   To avoid polluting name space in rest of compiler,
 *   prefix everything here with the -->target name<-- [CG].
 *   [CG] Changed prefix by 'targ_cg_' instead of target name.
 *   The hypothesis being that from one target to the other these
 *   functions may be reused.
 *
 * Exported functions:
 *
 *   BOOL targ_cg_TOP_is_shadd(TOP opcode)
 *
 *     Return TRUE if opcode is a shift-add instruction.
 *
 *   INT targ_cg_TOP_shadd_amount(TOP opcode)
 *
 *     Return the shift count for a shift-add instruction.
 *
 *   const MulProperties *targ_cg_mul_properties(const OP *op)
 *
 *     Return information on the properties of multiply instruction op.
 *     If op is not a multiply, return NULL.
 *
 *   TOP targ_cg_find_mul_with_properties(opnd1, opnd2, ...., &swap_operands)
 *
 *     Return the best instruction suitable for multiplying opnd1 and opnd2
 *     with the given properties.
 *     If opnd1 and opnd2 need to be swapped, then set swap_operands
 *     to TRUE.
 *     If no instruction can be found, return TOP_UNDEFINED.
 *
 * ====================================================================
 * ====================================================================
 */

#ifndef TARG_CG_PRIVATE_H_INCLUDED
#define TARG_CG_PRIVATE_H_INCLUDED

/* Initialize targ_op.cxx. */
extern void targ_cg_init_targ_op (void);

#endif // TARG_CG_PRIVATE_H_INCLUDED
