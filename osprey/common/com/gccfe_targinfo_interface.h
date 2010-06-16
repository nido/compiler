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

// This file collects all data that targinfo need to initialize for a
// specific target.
#ifndef gccfe_targinfo_interface_INCLUDED
#define gccfe_targinfo_interface_INCLUDED
#include "gcc_config.h" // ../../gccfe/gnu
#ifdef __cplusplus
extern "C" {
#endif
//Return GCC register id
extern unsigned int GCCTARG_Gp_Arg_Regnum(void);
extern unsigned int GCCTARG_Return_Pointer_Regnum(void);
extern unsigned int GCCTARG_Stack_Pointer_Regnum(void);
extern unsigned int GCCTARG_Static_last_Regnum(void);
extern unsigned int GCCTARG_First_Branch_Regnum(void); 
extern unsigned int GCCTARG_Last_Branch_Regnum(void); 
extern unsigned int GCCTARG_First_Int_Regnum(void); 
extern unsigned int GCCTARG_Last_Int_Regnum(void);

//Funcs returning data needed by gccfe
extern gcc_register_map_t *GCCTARG_Additional_Register_Names(void);
extern int GCCTARG_Additional_Register_Names_Size(void);
extern char *GCCTARG_Initial_Call_Used_Regs(void);
extern char *GCCTARG_Initial_Fixed_Regs(void);
extern int *GCCTARG_Map_Reg_To_Preg(void);
extern int GCCTARG_Map_Reg_To_Preg_Size(void);
extern int GCCTARG_Initial_Number_Of_Registers(void);

//Funcs that returns the open64 register class and the register number
//associated to the gcc register index gcc_index
//extern void GCCTARG_Get_Rclass_From_Gcc_Reg(int gcc_index, ISA_REGISTER_CLASS *rclass, int *regnum);

//Configure the bridge between gcc and targinfo
 extern void GCCTARG_Configure_Gcc_From_Targinfo(void);

//function needed to initialize the targinfo with info comming from the extension.
// This function is called by GCCTARG_Configure_Gcc_From_Targinfo
extern void TI_Initialize_Extension_Loader(void);

// [TTh] Mark disabled Gcc registers based on command line option -mdisabled-reg
extern void GCCTARG_Mark_Disabled_Gcc_Reg(void);

//[TB] Return the dwarf id associated to gcc register gcc_reg
extern unsigned int GCCTARG_Dwarf_Get_Reg_Id_From_Gcc_Reg(int gcc_reg);

//[SC] Return the size in bits of gcc register gcc_reg
extern int GCCTARG_Gcc_Reg_Bit_Size (int gcc_reg);

#ifdef __cplusplus
}
#endif
#endif /* gccfe_targinfo_interface_INCLUDED */

