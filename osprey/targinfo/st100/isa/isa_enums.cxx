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



//
// Generate a list of enumeration classes and values for the ISA.
/////////////////////////////////////////////////////////

#include <stddef.h>
#include "isa_enums_gen.h"

main ()
{
  ISA_Enums_Begin();

  /* ============ Enumerate Addressing Modes ============= */

  ISA_Create_Enum_Class ("addrm",
    "AddrMode_P13_P_U8",       0,
    "AddrMode_P13_PR_U5",      1,
    "AddrMode_ARL_P_U4",       2,
    "AddrMode_ARL_P_P3",       3,
    "AddrMode_ARH_P_P3",       4,
    "AddrMode_P15_P_U6",       5,
    "AddrMode_MQ_P15",         6,
    "AddrMode_P15_QP",         7,
    "AddrMode_AR_P_AR",        8,
    "AddrMode_AR_M_AR",        9,
    "AddrMode_AR_P_U9",       10,
    "AddrMode_AR_M_U9",       11,
    "AddrMode_P13_P_U15",     12,
    "AddrMode_AR_MQ_U5",      13,
    "AddrMode_AR_QP_AR",      14,
    "AddrMode_AR_QM_AR",      15,
    "AddrMode_AR_QP_U5",      16,
    "AddrMode_AR_QM_U5",      17,
    "AddrMode_AR_BP_AR",      18,
    "AddrMode_AR_BM_AR",      19,
    "AddrMode_AR_BP_U5",      20,
    "AddrMode_AR_BM_U5",      21,
    "AddrMode_DSP_AR_P_AR",   22,
    "AddrMode_DSP_AR_M_AR",   23,
    "AddrMode_DSP_AR_P_U5",   24,
    "AddrMode_DSP_AR_M_U5",   25,
    "AddrMode_DSP_AR_QP_AR",  26,
    "AddrMode_DSP_AR_QM_AR",  27,
    "AddrMode_DSP_AR_QP_U5",  28,
    "AddrMode_DSP_AR_QM_U5",  29,
    "AddrMode_DSP_AR_BP_AR",  30,
    "AddrMode_DSP_AR_BM_AR",  31,
    "AddrMode_DSP_AR_BP_U5",  32,
    "AddrMode_DSP_AR_BM_U5",  33,
      NULL,		-1);	// default value (AddrMode_)


  /* ============ Enumerate Address Modifiers ============= */

  ISA_Create_Enum_Class ("amod",
      "Modifier_NONE",    0,
      "Modifier_BITR",    1,
      "Modifier_8",       2,
      "Modifier_16",      3,
      "Modifier_32",      4,
      "Modifier_64",      5,
      "Modifier_128",     6,
      "Modifier_256",     7,
      "Modifier_512",     8,
      "Modifier_1024",    9,
      "Modifier_2048",   10,
      "Modifier_4096",   11,
      "Modifier_8192",   12,
      "Modifier_16384",  13,
      "Modifier_32768",  14,
      "Modifier_65536",  15,
       NULL,		-1);	// default value (AddrMode_)

  ISA_Enums_End();
  return 0;
}
