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

#ifdef TARG_ST
#include "W_alloca.h"         // using alloca()
#endif

#include "defs.h"
#include "errors.h"
#include "tracing.h"
#include "mempool.h"
#include "config.h"
#include "config_TARG.h"
#include "targ_isa_registers.h"
#include "targ_abi_properties.h"
#include "targ_isa_operands.h"
#include <ctype.h>                /* for isdigit() */

#ifdef TARG_ST
/* ====================================================================
 *   Extension_Regname_Register_Class
 *
 *   given register name (asm), get its register class for an extension.
 * ====================================================================
 */
static ISA_REGISTER_CLASS
Extension_Regname_Register_Class (const char *regname, int *regnum)
{
  ISA_REGISTER_CLASS cl;
  FOR_ALL_ISA_REGISTER_CLASS( cl ) {
    int reg;
    int first_isa_reg  = ISA_REGISTER_CLASS_info[cl].min_regnum;
    int last_isa_reg   = ISA_REGISTER_CLASS_info[cl].max_regnum;

    for (reg = first_isa_reg; reg <= last_isa_reg; reg++) {
      char *temp;
      if (regname[0] == '%') {
	const char *rc_name = ISA_REGISTER_CLASS_info[cl].name;
	temp = (char*)alloca(strlen(rc_name) + strlen(ISA_REGISTER_CLASS_info[cl].reg_name[reg]) + 1);
	sprintf(temp,"%%%s%%%s",rc_name, ISA_REGISTER_CLASS_info[cl].reg_name[reg]);
      } else {
	temp = (char*)ISA_REGISTER_CLASS_info[cl].reg_name[reg];
      }
      if (strcasecmp(temp, regname) == 0) {
	*regnum = reg;
	return cl;
      }
    }
  }
  return ISA_REGISTER_CLASS_UNDEFINED;
}
#endif
				  

/* ====================================================================
 *   CGTARG_Register_Class_Num_From_Name
 *
 *   given register name (asm), get its register class.
 *   returns ISA_REGISTER_CLASS_UNDEFINED if not register match the name.
 * ====================================================================
 */
ISA_REGISTER_CLASS 
CGTARG_Register_Class_Num_From_Name (const char *regname, int *regnum) 
{
  ISA_REGISTER_CLASS rclass = ISA_REGISTER_CLASS_UNDEFINED;

  switch (regname[0]) {
  case 'r':
    if (isdigit(regname[1])) {
      rclass = ISA_REGISTER_CLASS_integer;
      *regnum = atoi(regname+1);
    }
    break;
  case 'b':
    if (isdigit(regname[1])) {
      rclass = ISA_REGISTER_CLASS_branch;
      *regnum = atoi(regname+1);
    }
    break;
  }

  if (rclass == ISA_REGISTER_CLASS_UNDEFINED) {
    if (Extension_Is_Present) {
      return Extension_Regname_Register_Class(regname, regnum);
    }
  }
  else if (*regnum > ISA_REGISTER_CLASS_info[rclass].max_regnum) {
    // Invalid register number
    rclass = ISA_REGISTER_CLASS_UNDEFINED;
  }
  
  return rclass;
}
