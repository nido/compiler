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

#ifndef config_intrinsic_INCLUDED
#define config_intrinsic_INCLUDED "config_intrinsic.h"

#include "defs.h"
#include "wintrinsic.h"

inline char *TARG_Intrinsic_Runtime_Name( const INTRINSIC i) {

  switch(i) {
  case INTRN_STOD:
    return "__aeabi_f2d";
  case INTRN_UWTOD:
    return "__aeabi_ui2d";
  case INTRN_WTOD:
    return "__aeabi_i2d";
  case INTRN_ULTOD:
    return "__aeabi_ul2d";
  case INTRN_LTOD:
    return "__aeabi_l2d";
  case INTRN_DIVUW:
    return "__aeabi_uidiv";
  case INTRN_DIVW:
    return "__aeabi_idiv";
  case INTRN_DIVUL:
    return "__aeabi_uldiv";
  case INTRN_DIVL:
    return "__aeabi_ldiv";
  case INTRN_MODUW:
    return "__aeabi_uidivmod";
  case INTRN_MODW:
    return "__aeabi_idivmod";
  case INTRN_MODUL:
    return "__aeabi_uldivmod";
  case INTRN_MODL:
    return "__aeabi_ldivmod";
  case INTRN_MULD:
    return "__aeabi_dmul";
  case INTRN_DIVD:
    return "__aeabi_ddiv";
  case INTRN_ADDD:
    return "__aeabi_dadd";
  case INTRN_SUBD:
    return "__aeabi_dsub";
  case INTRN_MULS:
    return "__aeabi_fmul";
  case INTRN_DIVS:
    return "__aeabi_fdiv";
  case INTRN_ADDS:
    return "__aeabi_fadd";
  case INTRN_SUBS:
    return "__aeabi_fsub";
  case INTRN_LED:
    return "__aeabi_dcmple";
  case INTRN_LTD:
    return "__aeabi_dcmplt";
  case INTRN_GED:
    return "__aeabi_dcmpge";
  case INTRN_GTD:
    return "__aeabi_dcmpgt";
  case INTRN_EQD:
    return "__aeabi_dcmpeq";
#if 0
  case INTRN_NED:
    // [JV] Use complement of dcmpeq in rt_lower
    FmtAssert(FALSE,("Not supported by runtime"));
    return "__aeabi_dcmpeq";
#endif
  case INTRN_LES:
    return "__aeabi_fcmple";
  case INTRN_LTS:
    return "__aeabi_fcmplt";
  case INTRN_GES:
    return "__aeabi_fcmpge";
  case INTRN_GTS:
    return "__aeabi_fcmpgt";
  case INTRN_EQS:
    return "__aeabi_fcmpeq";
#if 0
  case INTRN_NES:
    // [JV] Use complement of fcmpeq in rt_lower
    FmtAssert(FALSE,("Not supported by runtime"));
    return "__aeabi_fcmpeq";
#endif

  default:
    return NULL;
  }

  return NULL;
}

inline BOOL TARG_Intrinsic_CG_Intrinsic( const INTRINSIC i, BOOL *cg_intrinsic) {

  *cg_intrinsic = NOT_CGINTRINSIC;
  if(TARG_Intrinsic_Runtime_Name(i) != NULL) {
    *cg_intrinsic = NOT_CGINTRINSIC;
    return TRUE;
  }

  return FALSE;
}


#endif /* config_intrinsic_INCLUDED */
