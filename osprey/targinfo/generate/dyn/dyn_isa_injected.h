
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
 * This file is only useful for generation
 * of dynamic extension code.
 *
 */

#ifndef _DYN_ISA_INJECTED_H_
#define _DYN_ISA_INJECTED_H_

#ifndef DYNAMIC_CODE_GEN
#error "### DYNAMIC_CODE_GEN not defined\n"
#endif

/* Check that EXTN has been specified on command line */
#ifndef EXTN
#error "### Error in dynamic code generation: undefined extension name macro EXTN.\n"
#endif

extern const extension_regclass_t* dyn_get_ISA_injected_operation_tab();
extern TOP dyn_get_TOP_AM_automod_variant(TOP, BOOL, BOOL, ISA_REGISTER_CLASS);

#endif /* _DYN_ISA_INJECTED_H_ */

