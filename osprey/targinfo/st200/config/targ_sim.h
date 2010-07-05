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


#ifndef targ_sim_INCLUDED
#define targ_sim_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif


/* some definitions for the dedicated hardware pregs: */

BE_EXPORTED extern INT Int_Preg_Min_Offset;
BE_EXPORTED extern INT Int_Preg_Max_Offset;
BE_EXPORTED extern INT Ptr_Preg_Min_Offset;
BE_EXPORTED extern INT Ptr_Preg_Max_Offset;
BE_EXPORTED extern INT Float_Preg_Min_Offset;
BE_EXPORTED extern INT Float_Preg_Max_Offset;
BE_EXPORTED extern INT Branch_Preg_Min_Offset;
BE_EXPORTED extern INT Branch_Preg_Max_Offset;
BE_EXPORTED extern INT Fcc_Preg_Min_Offset;
BE_EXPORTED extern INT Fcc_Preg_Max_Offset;

/* The offsets for return registers are fixed: */
BE_EXPORTED extern INT First_Int_Preg_Return_Offset;
BE_EXPORTED extern INT Last_Int_Preg_Return_Offset;
BE_EXPORTED extern INT First_Ptr_Preg_Return_Offset;
BE_EXPORTED extern INT Last_Ptr_Preg_Return_Offset;
BE_EXPORTED extern INT First_Float_Preg_Return_Offset;
BE_EXPORTED extern INT Last_Float_Preg_Return_Offset;

BE_EXPORTED extern INT First_Int_Preg_Param_Offset;
BE_EXPORTED extern INT First_Ptr_Preg_Param_Offset;
BE_EXPORTED extern INT First_Float_Preg_Param_Offset;

  /* Specific register offsets */
BE_EXPORTED extern INT Stack_Pointer_Preg_Offset;
BE_EXPORTED extern INT Frame_Pointer_Preg_Offset;
BE_EXPORTED extern INT Static_Link_Preg_Offset;
BE_EXPORTED extern INT Struct_Return_Preg_Offset;
BE_EXPORTED extern INT Function_Link_Preg_Offset;
BE_EXPORTED extern INT Exc_Ptr_Param_Offset;           /* exception struct ptr */
BE_EXPORTED extern INT Exc_Filter_Param_Offset;   /* exception filter value */

  /* [JV] This is the maximum formal save area size: thous in case of multiple register
   * files with different alignment constraints, this is not always equal to the total
   * size of possible register parameters but can contains some padding. So to set this
   * variable, it is necessary to compute all possible combinations.
   * For stxp70+fpx: F8 and I8 are aligned on 64 bits. F4 are mapped on fpx registers and
   * aligned on 32 bits. 
   * The worse case can be the following parameter list:
   * F4, F8, F4, F8, F4, F8, F4
   * Where an alignment is necessary between F4 and F8. So Default_Max_Formal_Save_Area_Size
   * is equal to Default_Formal_Register_Parameter_Total_Size + 3*4bytes = 40 + 12 = 52 bytes.
   *
   * Note: This value is used only for assertion purpose in data_layout.cxx
   */
BE_EXPORTED extern INT Default_Max_Formal_Save_Area_Size;

  /* [JV] This is the total size of varargs functions formal area size. */
BE_EXPORTED extern INT Default_Vararg_Formal_Save_Area_Size;


  /* [JV] These defines are used to set array size (targ_sim_core.h).
     Cannot for now change them by variables (is it necessary?).
  */
#define MAX_NUMBER_OF_REGISTER_PARAMETERS  8	// r0-r7
#define MAX_NUMBER_OF_REGISTERS_FOR_RETURN 8	// r0-r7
#define MAX_NUMBER_OF_INT_REGISTER_PARAMETERS 8    // r0-r7, used in data_layout.cxx
#define MAX_NUMBER_OF_FLOAT_REGISTER_PARAMETERS 0  // none, used in data_layout.cxx

/* most of the interface is shared between targets */
#include "targ_sim_core.h"

#ifdef __cplusplus
}
#endif
#endif /* targ_sim_INCLUDED */
