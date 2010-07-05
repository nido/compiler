
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

/**
 *
 * This file is only useful for dynamic code extension.
 *
 */
#include "../gccfe/extension_include.h"
#include "dyn_dll_api.h"
#include "dyn_type_api.h"
#ifndef DLL_EXPORTED
#define DLL_EXPORTED 
#endif


#ifdef __cplusplus
extern "C" {
#endif

static const extension_hooks_t static_interface = {
  /*magic number */
  MAGIC_NUMBER_EXT_API,

  /* -- xxx_mtypes.c ------------------------------------------------ */
  /* ---------------------------------------------------------------- */
  /* Return a pointer on the modes array */
  dyn_get_modes,
  
  /* Return the number of modes */
  dyn_get_modes_count,

  /* Return base offset for extension modes */
  dyn_get_modes_base_count,

  /* Return base offset for extension mtypes */
  dyn_get_mtypes_base_count,

  /* -- xxx_builtins.c ---------------------------------------------- */
  /* ---------------------------------------------------------------- */
  /* Return builtin array */
  dyn_get_builtins,

  /* Return the number of builtins */
  dyn_get_builtins_count,

  /* Return base offset for extension builtins */
  dyn_get_builtins_base_count,

  /* Return base offset for extension intrinsics */
  dyn_get_intrinsics_base_count,

  /* Return Patterns Recognition Rules array */
  dyn_get_recrules,

  /* Return number of rules */
  dyn_get_recrules_count,

  /* Return extension options array */
  dyn_get_extoption_array,

  /* Return extension options count */
  dyn_get_extoption_count

};

/* Exporting global pointer on static internal structure.             */

DLL_EXPORTED const extension_hooks_t *get_type_extension_instance() {
  return &static_interface;
}

#ifdef __cplusplus
}                                        /* End of "C" block          */
#endif
