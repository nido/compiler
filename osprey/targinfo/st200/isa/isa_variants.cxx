/* -- This file is NOT YET automatically generated :) -- */ 
/* 

  Copyright (C) 2002, 2004 ST Microelectronics, Inc.  All Rights Reserved. 

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

//  Generate ISA variants information 
/////////////////////////////////////// 
#include <stddef.h> 
#include "topcode.h" 
#include "isa_variants_gen.h" 

main() 
{ 
  ISA_VARIANT_ATT
    att_addr_negoffset,
    att_addr_postincr,
    att_addr_preincr,
    att_immediate,
    att_multi,
    att_next_immediate,
    att_prev_immediate;

  ISA_Variants_Begin ("st200");


  /* ==================================================================== 
   *              Variant Attributes 
   * ==================================================================== 
   */ 
  att_addr_negoffset = ISA_Variant_Attribute_Create("addr_negoffset");
  att_addr_postincr = ISA_Variant_Attribute_Create("addr_postincr");
  att_addr_preincr = ISA_Variant_Attribute_Create("addr_preincr");
  att_immediate = ISA_Variant_Attribute_Create("immediate");
  att_multi = ISA_Variant_Attribute_Create("multi");
  att_next_immediate = ISA_Variant_Attribute_Create("next_immediate");
  att_prev_immediate = ISA_Variant_Attribute_Create("prev_immediate");

  /* ==================================================================== 
   *              Instruction Variants
   * ==================================================================== 
   */ 
  /* ============================================================
     Multi variants.
     ============================================================ */
  /* Define variants for attribute: multi. */
  ISA_Instruction_Variant ( TOP_ldlc_i, TOP_multi_ldlc_i, att_multi, 0 );
  ISA_Instruction_Variant ( TOP_ldlc_ii, TOP_multi_ldlc_ii, att_multi, 0 );
  ISA_Instruction_Variant ( TOP_ldl_i, TOP_multi_ldl_i, att_multi, 0 );
  ISA_Instruction_Variant ( TOP_ldl_ii, TOP_multi_ldl_ii, att_multi, 0 );
  ISA_Instruction_Variant ( TOP_stlc_i,  TOP_multi_stlc_i, att_multi, 0 );
  ISA_Instruction_Variant ( TOP_stlc_ii, TOP_multi_stlc_ii, att_multi, 0 );
  ISA_Instruction_Variant ( TOP_stl_i, TOP_multi_stl_i, att_multi, 0 );
  ISA_Instruction_Variant ( TOP_stl_ii, TOP_multi_stl_ii, att_multi, 0 );

  ISA_Variants_End();
  return 0;
}
