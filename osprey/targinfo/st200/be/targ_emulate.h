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

/* ====================================================================
 *
 * Module: targ_emulate.h
 *
 * Description:
 *
 * Target dependent part of be/com/emulate.cxx.
 * This module is responsible of generating WHIRL emulation for
 * target dependent intrinsic ops.
 *
 * Interface:
 *
 * This interface is private to be.so it should not be exported to
 * other compiler components.
 * 
 *
 * Intrinsic op emulation
 * ======================
 *
 * WN *BETARG_emulate_intrinsic_op(WN *block, WN *tree)
 *
 *   Generate WHIRL emulation into the given block for the given
 *   intrinsic op tree.
 *   The given tree must be an intrinsic opcode.
 *   This function must return NULL, if the given intrinsic op is
 *   not target dependent. In this case the target independent
 *   be/com/emulate.cxx will do the emulation.
 *   Note that the returned tree (the last statement of the emulation)
 *   if not NULL must not be inserted into the block. It is
 *   done by the client of this interface.
 *
 */

#ifndef targ_emulate_INCLUDED
#define targ_emulate_INCLUDED

/* ====================================================================
 * Intrinsic op emulation.
 * ====================================================================
 */
extern WN *BETARG_emulate_intrinsic_op(WN *block, WN *tree);

#endif
