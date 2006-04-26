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
 * Module: wn_lower_mul.h
 *
 * Description:
 *
 * This module provides support for lowering of integer multiplications
 * by constant for machines that do not feature integer multiply or
 * machines where multiply operations are costly.
 * This module implements:
 * - the LOWER_FAST_MUL action for fast pwer of 2 multiplies,
 * - the LOWER_CNST_MUL action for general constant multiplies.
 * The general constant multiply handles the power of 2 multiply though,
 * the fast constant multiply should be used first has it is always profitable
 * and less costly.
 *
 * Interface:
 *
 * This interface is private to be.so it should not be exported to
 * other compiler components.
 * 
 * Multiply by Constant Support
 * ============================
 *
 *
 * BOOL WNL_Is_Int_Mul_By_Constant(WN *wn)
 *
 *   Returns whether the given wn tree is a multiplication by constant.
 *
 *
 *   Multiply by Power of 2 Support
 *   ------------------------------
 *
 * BOOL WNL_Is_Fast_Int_Mul_By_Constant_Candidate(WN *wn)
 *
 *   Returns whether the wn tree is candidate for power of 2
 *   fast multiply.
 *   Check the target dependent informations to decide if it
 *   is beneficial to use simple shift sequence.
 *   If the passed wn is not a multiplication by constant or
 *   is not profitable, this function returns FALSE.
 *
 * WN *WNL_Lower_Fast_Int_Mul_By_Constant(WN *block, WN *wn, LOWER_ACTIONS actions)
 *
 *   Performs the lowering of a fast constant multiply into the given
 *   block. The passed wn must be a candidate for fast constant
 *   multiply has this function do not handle general constant multiply.
 *   I.e. the function WNL_Is_Fast_Int_Mul_by_Constant() must have returned
 *   true before calling this function.
 *
 *
 *   Multiply by General Constant Support
 *   ------------------------------------
 * BOOL WNL_Is_Int_Mul_By_Constant_Candidate(WN *wn)
 *
 *   Returns whether the wn tree is candidate for constant multiply.
 *   Check the target dependent informations to decide if it
 *   is beneficial to use a shift/add sequence given the level
 *   of optimization and the code size options.
 *   If the passed wn is not a multiplication by constant or
 *   is not profitable, this function returns FALSE.
 *
 *
 * WN *WNL_Lower_Int_Mul_By_Constant(WN *block, WN *wn, LOWER_ACTIONS actions)
 *
 *   Performs the lowering of a constant multiply into the given
 *   block. The passed wn must be a constant multiply tree, i.e.
 *   the function WNL_Is_Int_Mul_by_Constant() must have returned true.
 *   It is not necessary that the profitability check returns true,
 *   though it should be done generally only if 
 *   WNL_Is_Int_Mul_By_Constant_Candidate() is true.
 *
 * ==================================================================== */

#ifndef wn_lower_mul_INCLUDED
#define wn_lower_mul_INCLUDED

/* ====================================================================
 * Multiply by Constant Support
 * ====================================================================
 */

extern BOOL WNL_Is_Int_Mul_By_Constant(WN *wn);

extern BOOL WNL_Is_Fast_Int_Mul_By_Constant_Candidate(WN *wn);
extern WN *WNL_Lower_Fast_Int_Mul_By_Constant(WN *block, WN *wn, LOWER_ACTIONS actions);

extern BOOL WNL_Is_Int_Mul_By_Constant_Candidate(WN *wn);
extern WN *WNL_Lower_Int_Mul_By_Constant(WN *block, WN *wn, LOWER_ACTIONS actions);

/* ==================================================================== */


#endif /* wn_lower_mul_INCLUDED */
