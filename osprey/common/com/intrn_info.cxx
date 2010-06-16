/*

  Copyright (C) 2000, 2001 Silicon Graphics, Inc.  All Rights Reserved.

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


#include "intrn_info.h"

#ifdef TARG_ST
//TB: Add some proto info for multiple results
// Use to get parameter information on the gcc tree, to know if a
// parameter is an input, an output or an inout parameter.
// This array is internal to the WHIRL translator.
proto_intrn_info_t *Proto_Intrn_Info_Array;
#endif


#ifdef TARG_ST
//TB: dynamic intrinsics support
intrn_info_t *intrn_info;
const intrn_info_t intrn_info_static[INTRINSIC_STATIC_COUNT+1] = {
#else
const intrn_info_t intrn_info[INTRINSIC_LAST+1] = {
#endif

#ifdef TARG_ST
  { /* NONE */
	NOT_BYVAL, NOT_PURE, SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, NOT_CGINTRINSIC,
	IRETURN_UNKNOWN, NULL, NULL, NULL},
#else
  { /* NONE */
	NOT_BYVAL, NOT_PURE, SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, NOT_CGINTRINSIC, NOT_SLAVE,
	IRETURN_UNKNOWN, NULL, NULL, NULL},
#endif

/* All intrinsic info are moved to intrn_entry.def */
#define NEED_INTRN_INFO
#  include "intrn_entry.def"
#undef NEED_INTRN_INFO

#ifdef TARG_ST
  { /* INTRINSIC_GENERAL_LAST */
	NOT_BYVAL, NOT_PURE, SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, NOT_CGINTRINSIC,
	IRETURN_UNKNOWN, NULL, NULL, NULL},
#include "targ_intrn_info.def"
#else
  { /* INTRINSIC_GENERAL_LAST */
	NOT_BYVAL, NOT_PURE, SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, NOT_CGINTRINSIC, NOT_SLAVE,
	IRETURN_UNKNOWN, NULL, NULL, NULL},
#endif
};


#ifdef TARG_ST
#ifdef BACK_END
#include "cxx_hash.h"

BOOL is_intrinsic_rt_name(const char *name)
{
  typedef class USER_HASH_TABLE<const char *, const char *, String_Hash, String_Equal> INTRINSIC_NAME_TO_NAME_MAP;
  static INTRINSIC_NAME_TO_NAME_MAP *iname_to_name_map ;
  if (!iname_to_name_map) {
    iname_to_name_map = CXX_NEW(INTRINSIC_NAME_TO_NAME_MAP(INTRINSIC_LAST-INTRINSIC_FIRST+1, Malloc_Mem_Pool),
				Malloc_Mem_Pool);
    for (INT opc = INTRINSIC_FIRST; opc <= INTRINSIC_LAST; opc++ ) {
        char *runtime_name;
	// Care when the entries are NULL
	if (runtime_name = INTRN_rt_name (opc)) {
	    iname_to_name_map->Enter (runtime_name, runtime_name) ;
	}
    }
  }
  return iname_to_name_map->Find (name) != NULL ;
}
#endif
#endif

#ifdef TARG_ST
/*
 * Maps TYPE_ID to INTR_RETKIND and the reverse.
 * This should be either:
 * 1. generated along with the MTYPE as the information is basically the same.
 * 2. removed. I.e. use directly the TYPE_ID instead of the INTR_RETKIND in
 * intrinsics definition.
 * The second solution is used by the dynamic intrinsics and the multi results
 * intrinsics.
 * Dynamic mtypes are mapped to IRETURN_DYNAMIC.
 * IRETURN__DYNAMIC is mapped to MTYPE_UNKNOWN.
 * The mapping is partial as we don't use all the possible types in the intrinsinc
 * definitions. Though it should be complete if automated.
 *
 * TODO: Merge the extension/multi result intrinsic dfinition with the standard
 * intrinsic definition.
 *
 * [CG] Note that there are very few types supported here for the return value.
 * In particular MTYPE_B is not supported. This is not really a problem, though
 * it is actually a constraint on the poccible intrinsincs definitions.
 */
INTRN_RETKIND
INTRN_return_kind_for_mtype (const TYPE_ID mtype)
{
  if (MTYPE_is_dynamic(mtype))
    return IRETURN_DYNAMIC;
  
  switch (mtype) {
  case MTYPE_V:
    return IRETURN_V;
  case MTYPE_I1:
    return IRETURN_I1;
  case MTYPE_I2:
    return IRETURN_I2;
  case MTYPE_I4:
    return IRETURN_I4;
  case MTYPE_I8:
    return IRETURN_I8;
  case MTYPE_U1:
    return IRETURN_U1;
  case MTYPE_U2:
    return IRETURN_U2;
  case MTYPE_U4:
    return IRETURN_U4;
  case MTYPE_U8:
    return IRETURN_U8;
  case MTYPE_F4:
    return IRETURN_F4;
  case MTYPE_F8:
    return IRETURN_F8;
  default:
    FmtAssert((0),("Unexpexted mtype to intrinsic return kind conversion: %s ", Mtype_Name(mtype)));
  }
  return IRETURN_UNKNOWN;
}

TYPE_ID
INTRN_mtype_for_return_kind(const INTRN_RETKIND kind)
{
  switch (kind) {
  case IRETURN_UNKNOWN:
    return MTYPE_UNKNOWN;
  case IRETURN_V:
    return MTYPE_V;
  case IRETURN_I1:
    return MTYPE_I1;
  case IRETURN_I2:
    return MTYPE_I2;
  case IRETURN_I4:
    return MTYPE_I4;
  case IRETURN_U1:
    return MTYPE_U1;
  case IRETURN_U2:
    return MTYPE_I2;
  case IRETURN_U4:
    return MTYPE_U4;
  case IRETURN_DYNAMIC:
    return MTYPE_UNKNOWN;
  default:
    FmtAssert(0, ("unexpected return kind: %i", kind));
  }
  return MTYPE_UNKNOWN;
}
#endif
