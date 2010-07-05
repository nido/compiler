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
//  Generate ABI information
///////////////////////////////////////



/* =====================================================================
 *
 *   ABI_PROPERTY
 *	An abstract type that represents a property of an ABI.
 *	ST100 ABI properties:
 *
 *      allocatable    -- list allocatable registers
 *      callee         -- list callee registers
 *      caller         -- ??
 *      func_arg       -- 
 *      func_val       --
 *      frame_ptr      -- frame pointer
 *      global_ptr     -- global pointer
 *      stack_ptr      -- stack pointer
 *      ret_addr       -- return address register
 *      loop_count     -- ??
 *      true_predicate -- true predicate register
 *      static_link    -- for the nested procedures
 *      link           -- for indirect branching (P3)
 *
 *      xxx_names[128] -- assembler names for xxx registers.
 *
 * =====================================================================
 */

#include <stdio.h>

#include <stddef.h>
#include "abi_properties_gen.h"
#include "targ_isa_registers.h"

static ABI_PROPERTY
	allocatable,
	callee,
	caller,
	func_arg,
	func_val,
	frame_ptr,
	global_ptr,
	stack_ptr,
	static_link,
	link,
	ret_addr,
        ret_struct,        /* this is for returning structures by value */
	loop_count,
        true_predicate,
        stacked,
        entry_ptr,
        zero,
        prev_funcstate,
        epilog_count,
        fzero,
        fone;

static void st100_abi(void)
{
  static const char *du_names[] = {
    NULL,   NULL,   NULL,   "sr",   NULL,   NULL,   NULL,   NULL,
    NULL,   NULL,   NULL,   NULL,   "sp",   NULL,   NULL,   NULL
  };

  static const char *au_names[] = {
    NULL,   NULL,   NULL,   "pr",   NULL,   NULL,   NULL,   "fp",
    NULL,   NULL,   NULL,   "lr",   "sl",   "gp",   NULL,   "sp"
  };

  static const char *guard_names[16] = {};
  static const char *loop_names[3] = {};

  /* DU register class */

  Reg_Names(ISA_REGISTER_CLASS_du, 0, 15, du_names);

  Reg_Property(allocatable, ISA_REGISTER_CLASS_du,
	       0,  1,   2,   3,   4,   5,   6,   7,  
	       8,   9,  10,  11,  12,  13,  14,  15,
	       -1);

  Reg_Property(callee, ISA_REGISTER_CLASS_du,
               4, 5, 6, 7, 8,   9,  10,  11,
	       -1);
  Reg_Property(caller, ISA_REGISTER_CLASS_du,
	       0, 1, 2, 3, 12,  13,  14,  15, 
	       -1);
  Reg_Property(func_arg, ISA_REGISTER_CLASS_du,
		0, 1, 2, 3,
	       -1);
  Reg_Property(func_val, ISA_REGISTER_CLASS_du,
	       0, 1,
	       -1);

  /* AU register class */

  Reg_Names(ISA_REGISTER_CLASS_au, 0, 15, au_names);

  Reg_Property(allocatable, ISA_REGISTER_CLASS_au,
		 0,   1,   2,   3,   4,   5,   6,   7,
		 8,   9,  10,  11,  12,  13,  14,
	       -1);

  // GP may be or not callee saved but this is indicated
  // elsewhere
  Reg_Property(global_ptr, ISA_REGISTER_CLASS_au, 
		13,
	       -1);
  Reg_Property(frame_ptr, ISA_REGISTER_CLASS_au, 
		7,
	       -1);

  Reg_Property(static_link, ISA_REGISTER_CLASS_au, 
		12,
	       -1);

  Reg_Property(link, ISA_REGISTER_CLASS_au, 
		3,
	       -1);

  Reg_Property(stack_ptr, ISA_REGISTER_CLASS_au, 
		15,
	       -1);

  Reg_Property(callee, ISA_REGISTER_CLASS_au,
	       4, 5, 6, 7, 8,   9,  10, 13,
	       -1);
  Reg_Property(caller, ISA_REGISTER_CLASS_au,
		 0, 1, 2, 3, 11, 12,  14,
	       -1);
  Reg_Property(func_arg, ISA_REGISTER_CLASS_au,
		 0, 1, 2,
	       -1);
  Reg_Property(func_val, ISA_REGISTER_CLASS_au,
		 0,
	       -1);
  // ret address is always callee saved, so don't add to callee
  Reg_Property(ret_addr, ISA_REGISTER_CLASS_au, 
	       11,
	       -1);

  // guard register class:

  Reg_Names(ISA_REGISTER_CLASS_guard, 0, 15, guard_names);

  Reg_Property(allocatable, ISA_REGISTER_CLASS_guard,
		 0,   1,   2,   3,   4,   5,   6,   7,
       		 8,   9,  10,  11,  12,  13,  14,
	       -1);
  Reg_Property(true_predicate, ISA_REGISTER_CLASS_guard, 
	       15,
	       -1);

  Reg_Property(callee, ISA_REGISTER_CLASS_guard,
	         5,   6,   7,   8,   9,  10,  11,  12, 
	       13,  14,
	       -1);
  Reg_Property(caller, ISA_REGISTER_CLASS_guard,
		 0,   1,   2,   3,   4,
	       -1);

  // loop register class:

  Reg_Names(ISA_REGISTER_CLASS_loop, 0, 2, loop_names);

  Reg_Property(allocatable, ISA_REGISTER_CLASS_loop,
		 0,   1,   2,
	       -1);
  Reg_Property(loop_count, ISA_REGISTER_CLASS_loop, 
		0,
	       -1);

  /* nothing ! */
  return;
}

main()
{
  ABI_Properties_Begin("st100");

  allocatable = Create_Reg_Property("allocatable");
  callee = Create_Reg_Property("callee");
  caller = Create_Reg_Property("caller");
  func_arg = Create_Reg_Property("func_arg");
  func_val = Create_Reg_Property("func_val");
  frame_ptr = Create_Reg_Property("frame_ptr");
  global_ptr = Create_Reg_Property("global_ptr");
  stack_ptr = Create_Reg_Property("stack_ptr");
  static_link = Create_Reg_Property("static_link");
  link = Create_Reg_Property("link");
  ret_addr = Create_Reg_Property("ret_addr");
  ret_struct = Create_Reg_Property("ret_struct");
  loop_count = Create_Reg_Property("loop_count");
  true_predicate = Create_Reg_Property("true_predicate");
  stacked = Create_Reg_Property("stacked");
  entry_ptr = Create_Reg_Property("entry_ptr");
  zero = Create_Reg_Property("zero");
  prev_funcstate = Create_Reg_Property("prev_funcstate");
  epilog_count = Create_Reg_Property("epilog_count");
  fzero = Create_Reg_Property("fzero");
  fone = Create_Reg_Property("fone");

  ///////////////////////////////////////
  Begin_ABI("st100");
  st100_abi();

  ABI_Properties_End();
}
