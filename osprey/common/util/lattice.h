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
 * Module: lattice.h
 *
 * Description:
 *
 * Implement a lattice template for data-flow analysis.
 *
 * ====================================================================
 * ====================================================================
 */

#ifndef LATTICE_H_INCLUDED
#define LATTICE_H_INCLUDED

template<class V>
class Lattice {
 private:
  const V top;
  const V bottom;
  const V (*const meet)(const V&, const V&);
  const V (*const join)(const V&, const V&);
 public:
  Lattice (const V (*meetfunc)(const V&, const V&),
	   const V (*joinfunc)(const V&, const V&),
	   V top_val, V bottom_val)
    : top (top_val), bottom (bottom_val), meet (meetfunc), join (joinfunc) {}
  V Top () { return top; }
  V Bottom () { return bottom; }
  V Meet(const V&a, const V&b) { return meet (a, b); }
  V Join(const V&a, const V&b) { return join (a, b); }
};

#endif /* LATTICE_H_INCLUDED */
