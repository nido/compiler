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


//
// Generate a list of literal classes and values for the ISA.
/////////////////////////////////////////////////////////

#include <stddef.h>
#include "isa_lits_gen.h"

main ()
{
  ISA_Lits_Begin();

  // ISA_Create_Lit_Class(name, type, [range,...] LIT_RANGE_END)

  ISA_Create_Lit_Class("s7",  SIGNED, SignedBitRange(7), LIT_RANGE_END);
  ISA_Create_Lit_Class("s9",  SIGNED, SignedBitRange(9), LIT_RANGE_END);
  ISA_Create_Lit_Class("s11",  SIGNED, SignedBitRange(11), LIT_RANGE_END);
  ISA_Create_Lit_Class("s16",  SIGNED, SignedBitRange(16), LIT_RANGE_END);
  ISA_Create_Lit_Class("s21",  SIGNED, SignedBitRange(21), LIT_RANGE_END);
  ISA_Create_Lit_Class("s25",  SIGNED, SignedBitRange(25), LIT_RANGE_END);
  ISA_Create_Lit_Class("s32",  SIGNED, SignedBitRange(32), LIT_RANGE_END);
  ISA_Create_Lit_Class("s40",  SIGNED, SignedBitRange(40), LIT_RANGE_END);

  ISA_Create_Lit_Class("u4",  UNSIGNED, UnsignedBitRange(4), LIT_RANGE_END);
  ISA_Create_Lit_Class("u5",  UNSIGNED, UnsignedBitRange(5), LIT_RANGE_END);
  ISA_Create_Lit_Class("u6",  UNSIGNED, UnsignedBitRange(6), LIT_RANGE_END);
  ISA_Create_Lit_Class("u7",  UNSIGNED, UnsignedBitRange(7), LIT_RANGE_END);
  ISA_Create_Lit_Class("u8",  UNSIGNED, UnsignedBitRange(8), LIT_RANGE_END);
  ISA_Create_Lit_Class("u9",  UNSIGNED, UnsignedBitRange(9), LIT_RANGE_END);
  ISA_Create_Lit_Class("u10",  UNSIGNED, UnsignedBitRange(10), LIT_RANGE_END);
  ISA_Create_Lit_Class("u11",  UNSIGNED, UnsignedBitRange(11), LIT_RANGE_END);
  ISA_Create_Lit_Class("u12",  UNSIGNED, UnsignedBitRange(12), LIT_RANGE_END);
  ISA_Create_Lit_Class("u15",  UNSIGNED, UnsignedBitRange(15), LIT_RANGE_END);
  ISA_Create_Lit_Class("u16",  UNSIGNED, UnsignedBitRange(16), LIT_RANGE_END);
  ISA_Create_Lit_Class("u20",  UNSIGNED, UnsignedBitRange(20), LIT_RANGE_END);
  ISA_Create_Lit_Class("u32",  UNSIGNED, UnsignedBitRange(32), LIT_RANGE_END);

  ISA_Lits_End();
}
