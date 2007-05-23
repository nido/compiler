/* 

  Copyright (C) 2006 ST Microelectronics, Inc.  All Rights Reserved. 

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

#ifdef FRONT_END

#include "gccfe_targinfo_interface.h"
#include "defs.h"
#include "symtab.h"
#include "targ_sim.h"
#include "targ_isa_registers.h"
#include "targ_abi_properties.h"
#include "register_preg.h" // For CGTARG_Regclass_Preg_Min
#include <ctype.h> //TB: for tower()

//TB: for extension, make ADDITIONAL_REGISTER_NAMES a real array
static gcc_register_map_t *Additional_Register_Names_Var;
static int Additional_Register_Names_Size_Var;

static char *Initial_Call_Used_Regs_Var;
static char *Initial_Fixed_Regs_Var;
static int Initial_Number_Of_Registers_Var;


static int *Map_Reg_To_Preg_Var;
static int Map_Reg_To_Preg_Size_Var;

static void Initialize_Gcc_reg(void);

  //Return GCC register id
static int Gp_Arg_Regnum;
static int Return_Pointer_Regnum;
static int Stack_Pointer_Regnum;


extern int GCCTARG_Additional_Register_Names_Size() {
  return Additional_Register_Names_Size_Var;
}

extern gcc_register_map_t *GCCTARG_Additional_Register_Names() {
  return Additional_Register_Names_Var;
}

extern char *GCCTARG_Initial_Call_Used_Regs() {
  return Initial_Call_Used_Regs_Var;
}

extern char *GCCTARG_Initial_Fixed_Regs() {
  return Initial_Fixed_Regs_Var;
}

extern int GCCTARG_Initial_Number_Of_Registers() {
  return Initial_Number_Of_Registers_Var;
}

extern int *GCCTARG_Map_Reg_To_Preg() {
  return Map_Reg_To_Preg_Var;
}

extern int GCCTARG_Map_Reg_To_Preg_Size() {
  return Map_Reg_To_Preg_Size_Var;
}

//Return GCC register id
extern int GCCTARG_Gp_Arg_Regnum(void){
  return Gp_Arg_Regnum;
}
extern int GCCTARG_Return_Pointer_Regnum(void) {
  return Return_Pointer_Regnum;
}
extern int GCCTARG_Stack_Pointer_Regnum(void) {
  return Stack_Pointer_Regnum;
}

extern int GCCTARG_Static_last_Regnum() {
  return Get_Static_Last_Dedicated_Preg_Offset() - 1;
}

extern int GCCTARG_First_Branch_Regnum() {
  return Branch_Preg_Min_Offset - 1;
}

extern int GCCTARG_Last_Branch_Regnum() {
  return Branch_Preg_Max_Offset - 1;
}

extern int GCCTARG_First_Int_Regnum() {
  return Int_Preg_Min_Offset - 1;
}

extern int GCCTARG_Last_Int_Regnum() {
  return Int_Preg_Max_Offset - 1;
}

extern void
GCCTARG_Configure_Gcc_From_Targinfo(void) {
  // Initialize targinfo abi properties
  GCC_Configure_ABI ();

  //Targinfo initialization
  TI_Initialize_Extension_Loader();

  ABI_PROPERTIES_Initialize();

  //Initialize registers stuff
  Initialize_Gcc_reg();
}

//TB: Returns an array of registers num  
static void
Initialize_Gcc_reg(void) {
  ISA_REGISTER_CLASS rclass;
  //Copute total nb pf registers
  INT total_nb_of_regs = 0;
  FOR_ALL_ISA_REGISTER_CLASS( rclass ) {
    const ISA_REGISTER_CLASS_INFO *icinfo = ISA_REGISTER_CLASS_Info(rclass);
    INT first_isa_reg  = ISA_REGISTER_CLASS_INFO_First_Reg(icinfo);
    INT last_isa_reg   = ISA_REGISTER_CLASS_INFO_Last_Reg(icinfo);
    INT register_count = last_isa_reg - first_isa_reg + 1;
    total_nb_of_regs += register_count;
  }
  Additional_Register_Names_Var = TYPE_MEM_POOL_ALLOC_N(gcc_register_map_t, Malloc_Mem_Pool,
							 total_nb_of_regs);
  Additional_Register_Names_Size_Var = total_nb_of_regs;

  Initial_Call_Used_Regs_Var = TYPE_MEM_POOL_ALLOC_N(char, Malloc_Mem_Pool,
							 total_nb_of_regs);
  Initial_Fixed_Regs_Var = TYPE_MEM_POOL_ALLOC_N(char, Malloc_Mem_Pool,
							 total_nb_of_regs);
  Initial_Number_Of_Registers_Var = total_nb_of_regs;


  Map_Reg_To_Preg_Var = TYPE_MEM_POOL_ALLOC_N(int, Malloc_Mem_Pool,
					       total_nb_of_regs);
  Map_Reg_To_Preg_Size_Var = total_nb_of_regs;

  int gcc_index = 0;
  // Compute arrays needed by gcc
  FOR_ALL_ISA_REGISTER_CLASS( rclass ) {
    INT32 i;
    const ISA_REGISTER_CLASS_INFO *icinfo = ISA_REGISTER_CLASS_Info(rclass);
    INT first_isa_reg  = ISA_REGISTER_CLASS_INFO_First_Reg(icinfo);
    INT last_isa_reg   = ISA_REGISTER_CLASS_INFO_Last_Reg(icinfo);
    INT register_count = last_isa_reg - first_isa_reg + 1;
    const char        *rcname         = ISA_REGISTER_CLASS_INFO_Name(icinfo);
    int preg_min = CGTARG_Regclass_Preg_Min(rclass);
    int preg_max = CGTARG_Regclass_Preg_Max(rclass);
    FmtAssert (((preg_max - preg_min) == register_count - 1),
      ("PREG range is not compatible with register class info for %d reg cladd", rclass));
    for (i = 0; i < register_count; ++i) {
      INT      isa_reg        = i + first_isa_reg;
      const char *reg_name = ISA_REGISTER_CLASS_INFO_Reg_Name(icinfo, isa_reg);
      char *temp = (char*)malloc(strlen(rcname) + strlen(reg_name) + 3);
      sprintf(temp,"%%%s%%%s",rcname, reg_name);
      //lower reg_name
      int j;
      for (j = strlen(rcname) + 2; j < strlen(temp); j++) {
	temp[j] = tolower(temp[j]);
      }
      BOOL is_allocatable = ABI_PROPERTY_Is_allocatable(rclass, isa_reg);
      BOOL is_caller = ABI_PROPERTY_Is_caller(rclass, isa_reg);
      BOOL is_stack_ptr = ABI_PROPERTY_Is_stack_ptr(rclass, isa_reg);
      BOOL is_frame_ptr = ABI_PROPERTY_Is_frame_ptr(rclass, isa_reg);
      BOOL is_ret_addr = ABI_PROPERTY_Is_ret_addr(rclass, isa_reg);
      BOOL is_zero = ABI_PROPERTY_Is_zero(rclass, isa_reg);
      BOOL is_true_predicate = ABI_PROPERTY_Is_true_predicate(rclass, isa_reg);
      BOOL gcc_is_fixed = is_stack_ptr || is_ret_addr || is_zero || is_true_predicate || !is_allocatable;
      BOOL is_func_arg  = ABI_PROPERTY_Is_func_arg(rclass, isa_reg);

      Additional_Register_Names_Var[gcc_index].name = temp;
      Additional_Register_Names_Var[gcc_index].number = gcc_index;
      //For the moment all regs are not call used.
      Initial_Call_Used_Regs_Var[gcc_index] = gcc_is_fixed || is_caller;
      Initial_Fixed_Regs_Var[gcc_index] = gcc_is_fixed;

      if (is_ret_addr) Return_Pointer_Regnum = gcc_index;
      if (is_func_arg && Gp_Arg_Regnum == 0) Gp_Arg_Regnum = gcc_index;
      if (is_stack_ptr) Stack_Pointer_Regnum = gcc_index;

      Map_Reg_To_Preg_Var[gcc_index] = preg_min + i;
      gcc_index++;
    }
    FmtAssert (((Map_Reg_To_Preg_Var[gcc_index - 1]) == preg_max),
      ("PREG range is not compatible with register class info for %d reg cladd", rclass));
  }
}



#endif //FRONT_END
