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

#define Int_Preg_Min_Offset             1
#define Int_Preg_Max_Offset             64
#define Ptr_Preg_Min_Offset             1
#define Ptr_Preg_Max_Offset             0
#define Float_Preg_Min_Offset           1
#define Float_Preg_Max_Offset           0  
#define Branch_Preg_Min_Offset          65
#define Branch_Preg_Max_Offset          72
#define Fcc_Preg_Min_Offset             1
#define Fcc_Preg_Max_Offset             0
#define Last_Dedicated_Preg_Offset      Branch_Preg_Max_Offset

/* The offsets for return registers are fixed: */
#define First_Int_Preg_Return_Offset	17	/* register v0 */
#define Last_Int_Preg_Return_Offset	24	/* register v1 */
#define First_Ptr_Preg_Return_Offset    0
#define Last_Ptr_Preg_Return_Offset     0
#define First_Float_Preg_Return_Offset	0	/* register f0 */
#define Last_Float_Preg_Return_Offset	0	/* register f2 */

/* Parameter placement */
#define First_Int_Preg_Param_Offset	17	/* register a0 */
#define First_Ptr_Preg_Param_Offset     0
#define First_Float_Preg_Param_Offset	0	/* register fa0 */

#define Stack_Pointer_Preg_Offset	13	/* register sp */
/* [SC] match fp setting in targinfo/st200/abi/abi_properties.cxx */
#define Frame_Pointer_Preg_Offset	8	/* register fp */
#define Static_Link_Preg_Offset	        9
#define Struct_Return_Preg_Offset       16   /* returning structs */
#ifdef TARG_ST //[TB]
#define Function_Link_Preg_Offset       64   /* function link register (for mcount call) */
  /* (cbr) define exceptions handler parameters. see EH_RETURN_DATA_REGNO(N) */
#define Exc_Ptr_Param_Offset            9    /* exception struct ptr */
#define Exc_Filter_Param_Offset         10   /* exception filter value */
#endif

#define MAX_NUMBER_OF_REGISTER_PARAMETERS  8      // used in data_layout.cxx
#define MAX_NUMBER_OF_REGISTERS_FOR_RETURN 8

/* most of the interface is shared between targets */
#include "targ_sim_core.h"

#ifdef __cplusplus
}
#endif
#endif /* targ_sim_INCLUDED */

