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


#include <stdio.h>
#include <stdlib.h>
#include <cmplrs/rcodes.h>

#define TRUE  1
#define FALSE 0

#include "defs.h"
#include "wintrinsic.h"
#ifdef TARG_ST
#include "intrn_info.h"
#include "symtab.h"
#endif
#include "wio.h"
#include "wutil.h"
#ifdef TARG_ST
//TB: dynamic intrinsics support
BE_EXPORTED INTRINSIC INTRINSIC_COUNT;
#endif
static const struct {
  INTRINSIC   opcode;
  char      * name;
} intrinsic_name_table [] = {
  INTRINSIC_NONE,	"NONE",

/* All intrinsic names are moved to intrn_entry.def */
#define NEED_INTRN_ID_NAME
#  include "intrn_entry.def"
#undef NEED_INTRN_ID_NAME

  INTRINSIC_GENERAL_LAST,       "INTRINSIC_GENERAL_LAST",

#ifdef TARG_ST
#include "targ_wutil.def"
#endif

  INTRINSIC_STATIC_COUNT,		"INTRINSIC_LAST"

};

struct {
  IOSTATEMENT   opcode;
  char        * name;
} iostatement_name_table [] = {
  (IOSTATEMENT) 0,	NULL,
  IOS_BACKSPACE,	"BACKSPACE",
  IOS_CLOSE,		"CLOSE",
  IOS_DEFINEFILE,	"DEFINEFILE",
  IOS_DELETE,		"DELETE",
  IOS_ENDFILE,		"ENDFILE",
  IOS_FIND,		"FIND",
  IOS_INQUIRE,		"INQUIRE",
  IOS_NAMELIST,		"NAMELIST",
  IOS_OPEN,		"OPEN",
  IOS_REWIND,		"REWIND",
  IOS_UNLOCK,		"UNLOCK",
  IOS_ACCEPT,		"ACCEPT",
  IOS_DECODE,		"DECODE",
  IOS_ENCODE,		"ENCODE",
  IOS_PRINT,		"PRINT",
  IOS_READ,		"READ",
  IOS_REWRITE,		"REWRITE",
  IOS_TYPE,		"TYPE",
  IOS_WRITE,		"WRITE",
  IOS_CR_FWU,		"UNFORMATTED_WRITE",
  IOS_CR_FRU,		"UNFORMATTED_READ",
  IOS_CR_FWF,		"FORMATTED_WRITE",
  IOS_CR_FRF,		"FORMATTED_READ",
  IOS_CR_FWN,		"NAMELIST_WRITE",
  IOS_CR_FRN,		"NAMELIST_READ",
  IOS_INQLENGTH,	"INQUIRE_LENGTH",
  IOS_CR_OPEN,		"OPEN",
  IOS_CR_CLOSE,		"CLOSE",
  IOS_CR_ENDFILE,	"ENDFILE",
  IOS_CR_REWIND,	"REWIND",
  IOS_CR_INQUIRE,	"INQUIRE",
  IOS_CR_BACKSPACE,	"BACKSPACE",
  IOS_CR_BUFFERIN,	"BUFFERIN",
  IOS_CR_BUFFEROUT,	"BUFFEROUT"
};

struct {
  IOITEM   opcode;
  char   * name;
} ioitem_name_table [] = {
  (IOITEM) 0,		NULL,
  IOU_NONE,		"NONE",
  IOU_DEFAULT,		"DEFAULT",
  IOU_EXTERNAL,		"EXTERNAL",
  IOU_INTERNAL,		"INTERNAL",
  IOU_DOPE,		"DOPE",
  (IOITEM) 6,		NULL,		/* spare */
  (IOITEM) 7,		NULL,		/* spare */
  (IOITEM) 8,		NULL,		/* spare */
  (IOITEM) 9,		NULL,		/* spare */
  IOF_NONE,		"NONE",
  IOF_ASSIGNED_VAR,	"ASSIGNED_VAR",
  IOF_CHAR_EXPR,	"CHAR_EXPR",
  IOF_LABEL,		"LABEL",
  IOF_LIST_DIRECTED,	"LIST_DIRECTED",
  IOF_NAMELIST_DIRECTED,"NAMELIST_DIRECTED",
  IOF_UNFORMATTED,	"UNFORMATTED",
  IOF_CR_PARSFMT,	"PREPARSED_FORMAT",
  IOF_CR_FMTSRC,	"FORMAT_SOURCE",
  IOF_CR_FMTSRC_DOPE,	"FORMAT_SOURCE_DOPE",
  (IOITEM) 20,		NULL,		/* spare */
  (IOITEM) 21,		NULL,		/* spare */
  (IOITEM) 22,		NULL,		/* spare */
  (IOITEM) 23,		NULL,		/* spare */
  (IOITEM) 24,		NULL,		/* spare */
  IOC_ACCESS,		"ACCESS",
  IOC_ASSOCIATEVARIABLE,"ASSOCIATEVARIABLE",
  IOC_BLANK,		"BLANK",
  IOC_CARRIAGECONTROL,	"CARRIAGECONTROL",
  IOC_DEFAULTFILE,	"DEFAULTFILE",
  IOC_DIRECT,		"DIRECT",
  IOC_DISPOSE,		"DISPOSE",
  IOC_END,		"END",
  IOC_ERR,		"ERR",
  IOC_EXIST,		"EXIST",
  IOC_FILE,		"FILE",
  IOC_FORM,		"FORM",
  IOC_FORMATTED,	"FORMATTED",
  IOC_IOSTAT,		"IOSTAT",
  IOC_KEY,		"KEY",
  IOC_KEYEQ,		"KEYEQ",
  IOC_KEYGE,		"KEYGE",
  IOC_KEYGT,		"KEYGT",
  IOC_KEY_START,	"KEY_START",
  IOC_KEY_END,		"KEY_END",
  IOC_KEY_CHARACTER,	"KEY_CHARACTER",
  IOC_KEY_INTEGER,	"KEY_INTEGER",
  IOC_KEYED,		"KEYED",
  IOC_KEYID,		"KEYID",
  IOC_MAXREC,		"MAXREC",
  IOC_NAME,		"NAME",
  IOC_NAMED,		"NAMED",
  IOC_NEXTREC,		"NEXTREC",
  IOC_NML,		"NML",
  IOC_NUMBER,		"NUMBER",
  IOC_OPENED,		"OPENED",
  IOC_ORGANIZATION,	"ORGANIZATION",
  IOC_READONLY,		"READONLY",
  IOC_REC,		"REC",
  IOC_RECCOUNT,		"RECCOUNT",
  IOC_RECL,		"RECL",
  IOC_RECORDTYPE,	"RECORDTYPE",
  IOC_SEQUENTIAL,	"SEQUENTIAL",
  IOC_SHARED,		"SHARED",
  IOC_STATUS,		"STATUS",
  IOC_TYPE,		"TYPE",
  IOC_U,		"U",
  IOC_UNFORMATTED,	"UNFORMATTED",
  IOC_VARFMT,		"VARFMT",
  IOC_VARFMT_ORIGFMT,	"VARFMT_ORIGFMT",
  IOC_CR_EEEFLAG,	"END_EOR_EOF_FLAG",
  IOC_ADVANCE,		"ADVANCE",
  IOC_SIZE,		"SIZE",
  IOC_CR_FLFLAG,	"FIRST_LAST_FLAG",
  IOC_EOR,		"EOR",
  IOC_INQLENGTH_VAR,	"INQLENGTH_VAR",
  IOC_CR_EDFLAG,	"ENCODE_DECODE_FLAG",
  IOC_PAD,		"PAD",
  IOC_DELIM,		"DELIM",
  IOC_ACTION,		"ACTION",
  IOC_POSITION,		"POSITION",
  IOC_READWRITE,	"READWRITE",
  IOC_WRITE,		"WRITE",
  IOC_READ,		"READ",
  IOC_ERRFLAG,		"ERRFLAG",
  (IOITEM) 85,		NULL,		/* spare */
  (IOITEM) 86,		NULL,		/* spare */
  (IOITEM) 87,		NULL,		/* spare */
  (IOITEM) 88,		NULL,		/* spare */
  (IOITEM) 89,		NULL,		/* spare */
  (IOITEM) 90,		NULL,		/* spare */
  (IOITEM) 91,		NULL,		/* spare */
  (IOITEM) 92,		NULL,		/* spare */
  (IOITEM) 93,		NULL,		/* spare */
  (IOITEM) 94,		NULL,		/* spare */
  IOL_ARRAY,		"ARRAY",
  IOL_CHAR,		"CHAR",
  IOL_CHAR_ARRAY,	"CHAR_ARRAY",
  IOL_EXPR,		"EXPR",
  IOL_IMPLIED_DO,	"IMPLIED_DO",
  IOL_IMPLIED_DO_1TRIP,	"IMPLIED_DO_1TRIP",
  IOL_LOGICAL,		"LOGICAL",
  IOL_RECORD,		"RECORD",
  IOL_VAR,		"VAR",
  IOL_DOPE,		"DOPE",
  (IOITEM) 105,		NULL,		/* spare */
  (IOITEM) 106,		NULL,		/* spare */
  (IOITEM) 107,		NULL,		/* spare */
  (IOITEM) 108,		NULL,		/* spare */
  (IOITEM) 109,		NULL,		/* spare */
  (IOITEM) 110,		NULL,		/* spare */
  (IOITEM) 111,		NULL,		/* spare */
  (IOITEM) 112,		NULL,		/* spare */
  (IOITEM) 113,		NULL,		/* spare */
  (IOITEM) 114,		NULL,		/* spare */
};

char *
get_intrinsic_name ( INTRINSIC opcode )
{
  static INT32 init_intrinsic = FALSE;

  if ( ! init_intrinsic ) {
    init_intrinsic = TRUE;

    for (INT opc = INTRINSIC_FIRST; opc <= INTRINSIC_STATIC_COUNT; opc++ ) {
      if ( opc != intrinsic_name_table [opc].opcode ) {
        printf ( "get_intrinsic_name : %d %d %s\n", opc,
                 intrinsic_name_table [opc].opcode,
                 intrinsic_name_table [opc].name );
        printf ( "intrinsic names out of sync\n" );
	exit (RC_INTERNAL_ERROR);
      }
    }
  }
#ifdef TARG_ST
  return opcode > INTRINSIC_STATIC_COUNT ? intrn_info[opcode].c_name : intrinsic_name_table [opcode].name;
#else
  return intrinsic_name_table [opcode].name;
#endif
}

char *
get_iostatement_name ( IOSTATEMENT opcode )
{
  static INT32 init_iostatement = FALSE;

  if ( ! init_iostatement ) {
    init_iostatement = TRUE;

    for (INT opc = IOSTATEMENT_FIRST; opc <= IOSTATEMENT_LAST; opc++ ) {
      if ( opc != iostatement_name_table [opc].opcode ) {
        printf ( "get_iostatement_name : %d %d %s\n", opc,
                 iostatement_name_table [opc].opcode,
                 iostatement_name_table [opc].name );
        printf ( "iostatement names out of sync\n" );
	exit (RC_INTERNAL_ERROR);
      }
    }
  }

  return iostatement_name_table [opcode].name;
}

char *
get_ioitem_name ( IOITEM opcode )
{
  static INT32 init_ioitem = FALSE;

  if ( ! init_ioitem ) {
    init_ioitem = TRUE;

    for (INT opc = IOITEM_FIRST; opc <= IOITEM_LAST; opc++ ) {
      if ( opc != ioitem_name_table [opc].opcode ) {
        printf ( "get_ioitem_name : %d %d %s\n", opc,
                 ioitem_name_table [opc].opcode,
                 ioitem_name_table [opc].name );
        printf ( "ioitem names out of sync\n" );
	exit (RC_INTERNAL_ERROR);
      }
    }
  }

  return ioitem_name_table [opcode].name;
}

char *
get_iolibrary_name( IOLIB lib)
{
  switch (lib) {
   case IOLIB_UNSPECIFIED:
     return ("unspec");
   case IOLIB_MIPS:
     return ("mips");
   case IOLIB_CRAY:
     return ("cray");
   default:
     return ("unknown");
  }
}

#ifdef TARG_ST
/* 
   [CM]
   This function creates a dependency on intrin_info global array
   We remove it in case of ir_tools, since this array is only exported
   in be, otherwise we may have to link ir_tools with be.
   Probaly a more correct resolution would be to move this function elsewhere
*/
#if !defined(IR_TOOLS)
/* ====================================================================
 *   WN_intrinsic_return_ty
 *
 *   TODO: Similar to the one in be/whirl2c/wn_attr.cxx
 * ====================================================================
 */
TY_IDX
WN_intrinsic_return_ty (
  INTRINSIC intr_opc
)
{
   TY_IDX ret_ty;
   
   Is_True(INTRINSIC_FIRST<=intr_opc && intr_opc<=INTRINSIC_LAST,
     ("Exp_Intrinsic_Op: Intrinsic Opcode (%d) out of range", intr_opc)); 

   switch (INTRN_return_kind(intr_opc))
   {
   case IRETURN_UNKNOWN:
     FmtAssert(FALSE,("Exp_Intrinsic_Op: cannot have UNKNOWN IRETURN type"));
     break;
   case IRETURN_V:
      ret_ty = MTYPE_To_TY(MTYPE_V);
      break;
   case IRETURN_I1:
      ret_ty = MTYPE_To_TY(MTYPE_I1);
      break;
   case IRETURN_I2:
      ret_ty = MTYPE_To_TY(MTYPE_I2);
      break;
   case IRETURN_I4:
      ret_ty = MTYPE_To_TY(MTYPE_I4);
      break;
   case IRETURN_I8:
      ret_ty = MTYPE_To_TY(MTYPE_I8);
      break;
   case IRETURN_U1:
      ret_ty = MTYPE_To_TY(MTYPE_U1);
      break;
   case IRETURN_U2:
      ret_ty = MTYPE_To_TY(MTYPE_U2);
      break;
   case IRETURN_U4:
      ret_ty = MTYPE_To_TY(MTYPE_U4);
      break;
   case IRETURN_U8:
      ret_ty = MTYPE_To_TY(MTYPE_U8);
      break;
   case IRETURN_F4:
      ret_ty = MTYPE_To_TY(MTYPE_F4);
      break;
   case IRETURN_F8:
      ret_ty = MTYPE_To_TY(MTYPE_F8);
      break;
   case IRETURN_FQ:
      ret_ty = MTYPE_To_TY(MTYPE_FQ);
      break;
   case IRETURN_C4:
      ret_ty = MTYPE_To_TY(MTYPE_C4);
      break;
   case IRETURN_C8:
      ret_ty = MTYPE_To_TY(MTYPE_C8);
      break;
   case IRETURN_CQ:
      ret_ty = MTYPE_To_TY(MTYPE_CQ);
      break;
      /*
   case IRETURN_PV:
      ret_ty = Stab_Pointer_To(Stab_Mtype_To_Ty(MTYPE_V));
      break;
   case IRETURN_PU1:
      ret_ty = Stab_Pointer_To(Stab_Mtype_To_Ty(MTYPE_U1));
      break;
   case IRETURN_DA1:
      ret_ty = WN_Tree_Type(WN_kid0(call));
      break;
      */
   case IRETURN_DYNAMIC:
      Is_True(FALSE, 
	      ("IRETURN_DYNAMIC not yet handled in WN_intrinsic_return_ty()"));
      ret_ty = MTYPE_To_TY(MTYPE_V);
      break;
   default:
      Is_True(FALSE, 
	      ("Unexpected INTRN_RETKIND in WN_intrinsic_return_ty()"));
      ret_ty = MTYPE_To_TY(MTYPE_V);
      break;
   }
   
   return ret_ty;
} /* WN_intrinsic_return_ty */
#endif /* !defined(IR _TOOLS) */
#endif
