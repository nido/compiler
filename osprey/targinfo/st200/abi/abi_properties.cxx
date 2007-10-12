/* -- This file is automatically generated -- */ 
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

// AUTOMATICALLY GENERATED FROM MDS DATA BASE !!! 
//  
//  Generate ABI information
///////////////////////////////////////

/* =====================================================================
 *
 *   ABI_PROPERTY
 *	An abstract type that represents a property of an ABI.
 *	ABI properties:
 *
 *      allocatable    -- list allocatable registers
 *      callee         -- list callee saved registers
 *      caller         -- list caller saved registers
 *      func_arg       -- function arguments
 *      func_val       -- function return values
 *      frame_ptr      -- frame pointer
 *      global_ptr     -- global pointer
 *      link           -- for indirect branching (P3)
 *      loop_count     -- loop counter registers
 *      ret_addr       -- return address register
 *      ret_struct     -- register containing ptr to returned struct
 *      stack_ptr      -- stack pointer
 *      static_link    -- for the nested procedures
 *      true_predicate -- true predicate register
 *      zero           -- register hard-wired to zero
 *
 *      xxx_names[128] -- assembler names for xxx registers.
 *
 * =====================================================================
 */

#include <stdio.h>

#include <stddef.h>
#include "abi_properties_gen.h"
#include "targ_isa_registers.h"

main()
{
  ABI_Properties_Begin("st200");

  ABI_PROPERTY allocatable = Create_Reg_Property("allocatable");
  ABI_PROPERTY callee = Create_Reg_Property("callee");
  ABI_PROPERTY caller = Create_Reg_Property("caller");
  ABI_PROPERTY entry_ptr = Create_Reg_Property("entry_ptr");
  ABI_PROPERTY epilog_count = Create_Reg_Property("epilog_count");
  ABI_PROPERTY fone = Create_Reg_Property("fone");
  ABI_PROPERTY frame_ptr = Create_Reg_Property("frame_ptr");
  ABI_PROPERTY func_arg = Create_Reg_Property("func_arg");
  ABI_PROPERTY func_val = Create_Reg_Property("func_val");
  ABI_PROPERTY fzero = Create_Reg_Property("fzero");
  ABI_PROPERTY global_ptr = Create_Reg_Property("global_ptr");
  ABI_PROPERTY link = Create_Reg_Property("link");
  ABI_PROPERTY loop_count = Create_Reg_Property("loop_count");
  ABI_PROPERTY prev_funcstate = Create_Reg_Property("prev_funcstate");
  ABI_PROPERTY ret_addr = Create_Reg_Property("ret_addr");
  ABI_PROPERTY ret_struct = Create_Reg_Property("ret_struct");
  ABI_PROPERTY stack_ptr = Create_Reg_Property("stack_ptr");
  ABI_PROPERTY stacked = Create_Reg_Property("stacked");
  ABI_PROPERTY static_link = Create_Reg_Property("static_link");
  ABI_PROPERTY thread_ptr = Create_Reg_Property("thread_ptr");
  ABI_PROPERTY true_predicate = Create_Reg_Property("true_predicate");
  ABI_PROPERTY zero = Create_Reg_Property("zero");

  ///////////////////////////////////////
  Begin_ABI("embedded");


  Reg_Property(allocatable, ISA_REGISTER_CLASS_branch, 0, 1, 2, 3, 4, 5, 6, 7, -1);
  Reg_Property(allocatable, ISA_REGISTER_CLASS_integer, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, -1);

  Reg_Property(callee, ISA_REGISTER_CLASS_integer, 1, 2, 3, 4, 5, 6, 7, 14, -1);

  Reg_Property(caller, ISA_REGISTER_CLASS_branch, 0, 1, 2, 3, 4, 5, 6, 7, -1);
  Reg_Property(caller, ISA_REGISTER_CLASS_integer, 8, 9, 10, 11, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, -1);

  Reg_Property(frame_ptr, ISA_REGISTER_CLASS_integer, 7, -1);

  Reg_Property(func_arg, ISA_REGISTER_CLASS_integer, 16, 17, 18, 19, 20, 21, 22, 23, -1);

  Reg_Property(func_val, ISA_REGISTER_CLASS_integer, 16, 17, 18, 19, 20, 21, 22, 23, -1);

  Reg_Property(fzero, ISA_REGISTER_CLASS_integer, 0, -1);

  Reg_Property(global_ptr, ISA_REGISTER_CLASS_integer, 14, -1);

  Reg_Property(ret_addr, ISA_REGISTER_CLASS_integer, 63, -1);

  Reg_Property(ret_struct, ISA_REGISTER_CLASS_integer, 15, -1);

  Reg_Property(stack_ptr, ISA_REGISTER_CLASS_integer, 12, -1);

  Reg_Property(static_link, ISA_REGISTER_CLASS_integer, 8, -1);

  Reg_Property(thread_ptr, ISA_REGISTER_CLASS_integer, 13, -1);

  Reg_Property(zero, ISA_REGISTER_CLASS_integer, 0, -1);

  ///////////////////////////////////////
  Begin_ABI("PIC");


  Reg_Property(allocatable, ISA_REGISTER_CLASS_branch, 0, 1, 2, 3, 4, 5, 6, 7, -1);
  Reg_Property(allocatable, ISA_REGISTER_CLASS_integer, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, -1);

  Reg_Property(callee, ISA_REGISTER_CLASS_integer, 1, 2, 3, 4, 5, 6, 7, -1);

  Reg_Property(caller, ISA_REGISTER_CLASS_branch, 0, 1, 2, 3, 4, 5, 6, 7, -1);
  Reg_Property(caller, ISA_REGISTER_CLASS_integer, 8, 9, 10, 11, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, -1);

  Reg_Property(frame_ptr, ISA_REGISTER_CLASS_integer, 7, -1);

  Reg_Property(func_arg, ISA_REGISTER_CLASS_integer, 16, 17, 18, 19, 20, 21, 22, 23, -1);

  Reg_Property(func_val, ISA_REGISTER_CLASS_integer, 16, 17, 18, 19, 20, 21, 22, 23, -1);

  Reg_Property(fzero, ISA_REGISTER_CLASS_integer, 0, -1);

  Reg_Property(global_ptr, ISA_REGISTER_CLASS_integer, 14, -1);

  Reg_Property(ret_addr, ISA_REGISTER_CLASS_integer, 63, -1);

  Reg_Property(ret_struct, ISA_REGISTER_CLASS_integer, 15, -1);

  Reg_Property(stack_ptr, ISA_REGISTER_CLASS_integer, 12, -1);

  Reg_Property(static_link, ISA_REGISTER_CLASS_integer, 8, -1);

  Reg_Property(thread_ptr, ISA_REGISTER_CLASS_integer, 13, -1);

  Reg_Property(zero, ISA_REGISTER_CLASS_integer, 0, -1);
  // Debug infornmation for core registers.
  //  DW_DEBUG_Extension_Reg(ISA_REGISTER_CLASS_branch     ,NULL,0 /*DWARF base index */);
  // DW_DEBUG_Extension_Reg(ISA_REGISTER_CLASS_integer    ,NULL,8  /*DWARF base index */);

  ABI_Properties_End();
}

