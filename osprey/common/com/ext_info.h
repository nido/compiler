/* 

  Copyright (C) 2008 ST Microelectronics, Inc.  All Rights Reserved. 

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
/*
 * API to get information about extension
 */
#ifndef EXT_INFO_H
#define EXT_INFO_H

#define INVALID_EXTENSION_ID (0)
/*
 * Bit used to activate traces of extension support
 */
#define TRACE_EXTENSION_LOADER_MASK        0x0001 /* Trace of extension loader */
#define TRACE_EXTENSION_REG_PLACEMENT_MASK 0x0002 /* Trace of register placement
						     at whirl level for native 
						     support */

BE_EXPORTED extern INT32 EXTENSION_Get_ExtensionId_From_ExtensionName(const char *extname);

/* Return TRUE if conversion from src to tgt is allowed, FALSE otherwise.
 * At least one of the type is expected to be an extension one. */
BE_EXPORTED extern BOOL EXTENSION_Are_Equivalent_Mtype(TYPE_ID src_ty, TYPE_ID tgt_ty);

BE_EXPORTED extern INTRINSIC EXTENSION_Get_Convert_Intrinsic(TYPE_ID src_ty, TYPE_ID tgt_ty, BOOL ignore_sign=TRUE);

BE_EXPORTED extern TYPE_ID EXTENSION_Get_Equivalent_Mtype(TYPE_ID ext_ty);

BE_EXPORTED extern INTRINSIC EXTENSION_Get_CLR_Intrinsic(TYPE_ID ty);

BE_EXPORTED extern INTRINSIC EXTENSION_Get_Convert_From_U32_Intrinsic(TYPE_ID ty);

BE_EXPORTED extern BOOL EXTENSION_Is_ExtGen_Enabled_For_Intrinsic(INTRINSIC intrn, long long flags);

BE_EXPORTED extern long long
EXTENSION_Get_ExtOption_Flag_By_Name(int extid, const char* optionname);

BE_EXPORTED extern BOOL EXTENSION_Is_Defined(const char *extname);
BE_EXPORTED extern int EXTENSION_Get_ExtensionRank_From_Intrinsic(INTRINSIC);

/*
 * Return TRUE if native codegen is enabled for at least one extension
 */
BE_EXPORTED extern BOOL EXTENSION_Has_ExtGen_Enabled();

/*
 * Return TRUE if the parameter intrinsic Id belongs to an extension
 */
inline BOOL EXTENSION_Is_Extension_INTRINSIC(INTRINSIC id) {
  return (id > INTRINSIC_STATIC_COUNT && id <= INTRINSIC_COUNT);
}


#include <vector>
typedef std::vector<INTRINSIC> INTRINSIC_Vector_t;
#define OPCODE_MAPPED_ON_CORE INTRINSIC_INVALID
BE_EXPORTED extern INTRINSIC_Vector_t* Get_Intrinsic_from_OPCODE(OPCODE opc);
BE_EXPORTED extern void Add_Intrinsic_for_OPCODE(OPCODE opc, INTRINSIC intrn);


#endif /* EXT_INFO_H */
