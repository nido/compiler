/* -- This file is automatically generated -- */ 
/* 

  Copyright (C) 2002 ST Microelectronics, Inc.  All Rights Reserved. 

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
  , or: 

  http://www.st.com 

  For further information regarding this notice, see: 

  http: 
*/ 

// AUTOMATICALLY GENERATED FROM CHESS DATA BASE !!! 


// 
// Generate a list of literal classes and values for the ISA. 
///////////////////////////////////////////////////////// 

#include <stddef.h> 
#include "isa_lits_gen.h" 

main () 
{ 
  ISA_Lits_Begin(); 

  // ISA_Create_Lit_Class(name, type, [range,...] LIT_RANGE_END) 

  ISA_Create_Lit_Class("s32", SIGNED, SignedBitRange(32), LIT_RANGE_END); 
  ISA_Create_Lit_Class("u32", UNSIGNED, SignedBitRange(32), LIT_RANGE_END); 
  ISA_Create_Lit_Class("s23", SIGNED, SignedBitRange(23), LIT_RANGE_END); 
  ISA_Create_Lit_Class("u23", UNSIGNED, SignedBitRange(23), LIT_RANGE_END); 
  ISA_Create_Lit_Class("s9", SIGNED, SignedBitRange(9), LIT_RANGE_END); 

  ISA_Lits_End(); 
  return 0; 
} 
