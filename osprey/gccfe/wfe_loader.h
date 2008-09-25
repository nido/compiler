/* 

  Copyright (C) 2007 ST Microelectronics, Inc.  All Rights Reserved. 

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


#ifndef wfe_loader_INCLUDED
#define wfe_loader_INCLUDED

/* This file is in charge of providing extension sepcific feature */
/* Define the dynamique count: to be place in the loader section*/

#include "machmode.h"
#include "mtypes.h"
#ifdef __cplusplus
extern "C" {
#endif
  /* Loader initialization */
  extern void WFE_Init_Loader(void);
  /* Add the dynamic builtins */
  extern void WFE_Add_Builtins(void);
  /* matching between GCC BUITINS and OPEN64 INTRINSICS */
  extern INTRINSIC WFE_Intrinsic(enum built_in_function built);
  //For a given PREG in an extension, return the associated MTYPE
  extern TYPE_ID EXTENSION_Get_Mtype_For_Preg(PREG_NUM preg);
  extern void WFE_Loader_Initialize_Register (void);

  //Mapping between gcc reg and open64 PREG
  extern int *Map_Reg_To_Preg;
  extern int Map_Reg_To_Preg_Size;

#ifdef __cplusplus
}
#endif

#endif /* wfe_loader_INCLUDED */
