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

//  AUTOMATICALLY GENERATED FROM CHESS DATABASE !!!

//  Generate ISA properties information 
/////////////////////////////////////// 
#include <stddef.h> 
#include "topcode.h" 
#include "isa_properties_gen.h" 

main() 
{ 
  ISA_PROPERTY 
    noop,
    load,
    store,
    jump,
    ijump,
    branch,
    call,
    simulated,
    dummy,
    var_opnds,
    dismissible;

  ISA_Properties_Begin ("st200"); 


  /* ==================================================================== 
   *              Operator attributes descriptors 
   * ==================================================================== 
   */ 

  /* ====================================== */ 
  simulated = ISA_Property_Create ("simulated"); 
  Instruction_Group (simulated, 
		 TOP_asm, 
		 TOP_intrncall, 
		 TOP_spadjust, 
		 TOP_copy_br, 
		 TOP_noop, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  dismissible = ISA_Property_Create ("dismissible"); 
  Instruction_Group (dismissible, 
		 TOP_ldw_d, 
		 TOP_ldh_d, 
		 TOP_ldhu_d, 
		 TOP_ldb_d, 
		 TOP_ldbu_d, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  load = ISA_Property_Create ("load"); 
  Instruction_Group (load, 
		 TOP_ldw, 
		 TOP_ldw_d, 
		 TOP_ldh, 
		 TOP_ldh_d, 
		 TOP_ldhu, 
		 TOP_ldhu_d, 
		 TOP_ldb, 
		 TOP_ldb_d, 
		 TOP_ldbu, 
		 TOP_ldbu_d, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  var_opnds = ISA_Property_Create ("var_opnds"); 
  Instruction_Group (var_opnds, 
		 TOP_asm, 
		 TOP_intrncall, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  dummy = ISA_Property_Create ("dummy"); 
  Instruction_Group (dummy, 
		 TOP_begin_pregtn, 
		 TOP_end_pregtn, 
		 TOP_bwd_bar, 
		 TOP_fwd_bar, 
		 TOP_dfixup, 
		 TOP_ffixup, 
		 TOP_ifixup, 
		 TOP_label, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  noop = ISA_Property_Create ("noop"); 
  Instruction_Group (noop, 
		 TOP_nop, 
		 TOP_noop, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  jump = ISA_Property_Create ("jump"); 
  Instruction_Group (jump, 
		 TOP_goto, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  call = ISA_Property_Create ("call"); 
  Instruction_Group (call, 
		 TOP_call, 
		 TOP_icall, 
		 TOP_rfi, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  ijump = ISA_Property_Create ("ijump"); 
  Instruction_Group (ijump, 
		 TOP_igoto, 
		 TOP_return, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  store = ISA_Property_Create ("store"); 
  Instruction_Group (store, 
		 TOP_stw, 
		 TOP_sth, 
		 TOP_stb, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  branch = ISA_Property_Create ("branch"); 
  Instruction_Group (branch, 
		 TOP_br, 
		 TOP_brf, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  /*         Memory Access Size 1          */ 
  /* ====================================== */ 
  ISA_Memory_Access (1, 
		 TOP_ldb, 
		 TOP_ldb_d, 
		 TOP_ldbu, 
		 TOP_ldbu_d, 
		 TOP_stb, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  /*         Memory Access Size 2          */ 
  /* ====================================== */ 
  ISA_Memory_Access (2, 
		 TOP_ldh, 
		 TOP_ldh_d, 
		 TOP_ldhu, 
		 TOP_ldhu_d, 
		 TOP_sth, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  /*         Memory Access Size 4          */ 
  /* ====================================== */ 
  ISA_Memory_Access (4, 
		 TOP_ldw, 
		 TOP_ldw_d, 
		 TOP_stw, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  /*          Memory Alignment 1           */ 
  /* ====================================== */ 
  ISA_Memory_Alignment (1, 
		 TOP_ldb, 
		 TOP_ldb_d, 
		 TOP_ldbu, 
		 TOP_ldbu_d, 
		 TOP_stb, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  /*          Memory Alignment 2           */ 
  /* ====================================== */ 
  ISA_Memory_Alignment (2, 
		 TOP_ldh, 
		 TOP_ldh_d, 
		 TOP_ldhu, 
		 TOP_ldhu_d, 
		 TOP_sth, 
		 TOP_UNDEFINED); 

  /* ====================================== */ 
  /*          Memory Alignment 4           */ 
  /* ====================================== */ 
  ISA_Memory_Alignment (4, 
		 TOP_ldw, 
		 TOP_ldw_d, 
		 TOP_stw, 
		 TOP_UNDEFINED); 

  ISA_Properties_End();
  return 0;
}
