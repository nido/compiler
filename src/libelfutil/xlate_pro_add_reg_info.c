/*
 * Copyright (C) 2007. QLogic Corporation. All Rights Reserved.
 */

/*

  Copyright (C) 2000, 2001 Silicon Graphics, Inc.  All Rights Reserved.

   Path64 is free software; you can redistribute it and/or modify it
   under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation version 2.1

   Path64 is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with Path64; see the file COPYING.  If not, write to the Free
   Software Foundation, 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.

   Special thanks goes to SGI for their continued support to open source

*/





/*
   xlate_pro_add_reg_info.c
   $Revision$


*/

#include "xlateincl.h"

#ifdef _LIBELF_XTND_EXPANDED_DATA
#pragma weak xlate_pro_add_reg_info_xtnd = _xlate_pro_add_reg_info_xtnd
#elif defined(BUILD_OS_DARWIN)
#else
#pragma weak xlate_pro_add_reg_info = _xlate_pro_add_reg_info
#endif

/*
  The distinction here between calling leb128*32 and leb*64 on
  offsets is to support large offsets in 64bit apps and
  to use the slightly faster 32bit encode for 32 bit apps.
*/

int xlate_pro_add_reg_info(xlate_table_pro     table,
    Dwarf_Small         op,
    Dwarf_Unsigned      val1,
    Dwarf_Unsigned      val2)
{
   unsigned char	regAssemble[100];
   Uword		regAssembleSize = 0;
   int			leb128_length;
   Dwarf_Unsigned	delta;
   int is64bit = table->tb_is64bit;

   if(table->tb_magic != PRO_MAGIC_VAL) {
        return XLATE_TB_STATUS_INVALID_TABLE;
   }
   if(table->tb_no_new_entries_allowed) {
	return XLATE_TB_STATUS_ADD_TOO_LATE;
   }

    regAssemble[0] = op;
    switch (op) {
	case DW_CFA_advance_loc :
	    delta = val1 >> 2;
	    if (delta > 0x3f) {
		return XLATE_TB_STATUS_BAD_REG_VAL;
	    }

	    regAssemble[0] |= delta;
	    regAssembleSize = 1;
	    break;

	case DW_CFA_offset :
	    if (val1 > 0x3f) {
		return XLATE_TB_STATUS_BAD_REG_VAL;
	    }

	    regAssemble[0] |= val1;
	    if(is64bit) {
	      leb128_length = 
		_leb128_unsigned_encode64(val2>>2, (char *) &regAssemble[1]);
	    } else {
	      leb128_length = 
		_leb128_unsigned_encode32(val2>>2, (char *) &regAssemble[1]);
	    }
	    regAssembleSize = 1 + leb128_length;
	    break;

	case DW_CFA_restore:
	    if (val1 > 0x3f) {
		return XLATE_TB_STATUS_BAD_REG_VAL;
	    }

	    regAssemble[0] |= val1;
	    regAssembleSize = 1;
	    break;

	case DW_CFA_set_loc :
	    if(is64bit) { 
	      memcpy(&regAssemble[1], &val1, sizeof(val1));
	      regAssembleSize = 1 + sizeof(val1);
	    } else {
	      uint32_t val32 = val1;
	      memcpy(&regAssemble[1], &val32, sizeof(val32));
	      regAssembleSize = 1 + sizeof(val32);
	    }
	    break;

	case DW_CFA_advance_loc1 :
	    delta = val1 >> 2;
	    if (delta > 0xff) {
		return XLATE_TB_STATUS_BAD_REG_VAL;
	    }

	    regAssemble[1] = delta;
	    regAssembleSize = 2;
	    break;

	case DW_CFA_advance_loc2 :
	    delta = val1 >> 2;
	    { unsigned short ldelta = delta;
	      if (delta > 0xffff) {
		return XLATE_TB_STATUS_BAD_REG_VAL;
	      }

	      memcpy(&regAssemble[1],&ldelta,2);
	      regAssembleSize = 3;
            }
	    break;

	case DW_CFA_advance_loc4 :
	    delta = val1 >> 2;
	    { uint32_t ldelta = delta;
	      if (delta > 0xffffffff) {
		return XLATE_TB_STATUS_BAD_REG_VAL;
	      }
	      memcpy(&regAssemble[1],&ldelta,4);
	      regAssembleSize = 5;
	    }
	    break;

	case DW_CFA_offset_extended :
	case DW_CFA_def_cfa :
	    if (val1 >= DW_FRAME_LAST_REG_NUM) {
		return XLATE_TB_STATUS_BAD_REG_VAL;
	    }
	    leb128_length =
		_leb128_unsigned_encode32(val1, (char *) &regAssemble[1]);
	    regAssembleSize = 1 + leb128_length;

	    if(is64bit) {
	      leb128_length = _leb128_unsigned_encode64((val2 >> 2), 
		(char *) &regAssemble[regAssembleSize]);
	    } else {
	      leb128_length = _leb128_unsigned_encode32((val2 >> 2), 
		(char *) &regAssemble[regAssembleSize]);
	    }
	    regAssembleSize += leb128_length;
	    break;

	case DW_CFA_restore_extended :
	case DW_CFA_undefined :
	case DW_CFA_same_value :
	case DW_CFA_def_cfa_register :
	    if (val1 >= DW_FRAME_LAST_REG_NUM) {
		return XLATE_TB_STATUS_BAD_REG_VAL;
	    }
	    leb128_length =
		_leb128_unsigned_encode32(val1, (char *) &regAssemble[1]);
	    regAssembleSize = 1 + leb128_length;
	    break;

	case DW_CFA_register :
		/* both vals are reg numbers */
	    if (val1 >= DW_FRAME_LAST_REG_NUM) {
		return XLATE_TB_STATUS_BAD_REG_VAL;
	    }
	    leb128_length =
		_leb128_unsigned_encode32(val1, (char *) &regAssemble[1]);
	    regAssembleSize = 1 + leb128_length;

	    if (val2 >= DW_FRAME_LAST_REG_NUM) {
		return XLATE_TB_STATUS_BAD_REG_VAL;
	    }
	    leb128_length = _leb128_unsigned_encode32(val2, 
		(char *) &regAssemble[regAssembleSize]);
	    regAssembleSize += leb128_length;
	    break;

	case DW_CFA_remember_state :
	case DW_CFA_restore_state :
	case DW_CFA_nop :
	    regAssembleSize = 1;
	    break;

	case DW_CFA_def_cfa_offset :
	    if(is64bit) {
	      leb128_length =
		_leb128_unsigned_encode64((val1 >> 2),
		  (char *) &regAssemble[1]);
	    } else {
	      leb128_length =
		_leb128_unsigned_encode32((val1 >> 2),
		  (char *) &regAssemble[1]);
	    }
	    regAssembleSize = 1 + leb128_length;
	    break;

	default :
	    return XLATE_TB_STATUS_BAD_REG_OP;
    }

    if ((table->tb_regInfoOffset + regAssembleSize) >= 
		table->tb_regInfoSize) {
	char *newloc = 0;
	/* always get here on first call */
	table->tb_regInfoSize += TB_REGINFO_SIZE;
	newloc = realloc(table->tb_regInfo, table->tb_regInfoSize);
	if(0 == newloc ) {
	   return XLATE_TB_STATUS_ALLOC_FAIL;
	}
	table->tb_regInfo = newloc;
    }

    memcpy(table->tb_regInfo + table->tb_regInfoOffset, regAssemble, 
	regAssembleSize);
    table->tb_regInfoOffset += regAssembleSize;

    return XLATE_TB_STATUS_NO_ERROR;
}


