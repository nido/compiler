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


/* ====================================================================
 * ====================================================================
 *
 * Module: config_elf_targ.cxx
 * $Revision$
 * $Date$
 * $Author$
 * $Source$
 *
 * Description:
 *
 * ELF configuration specific to the target machine/system.
 *
 * ====================================================================
 * ====================================================================
 */

#include <elf.h>
#include <sys/elf_whirl.h>
#include "defs.h"
#include "erglob.h"
#include "tracing.h"
#include "config_elf_targ.h"

#ifdef TARG_ST
// [CL] include ST200 specific values
#include "targ_elf.h"
#include "config_target.h"
#include "config.h"
#endif

/* ====================================================================
 *   Config_Target_From_ELF
 *
 *   Based on the flags in the ELF header <ehdr>, return whether 
 *   is 64bit and the isa level.
 * ====================================================================
 */
void 
Config_Target_From_ELF (
  Elf64_Word e_flags, 
  BOOL *is_64bit, 
  INT *isa
)
{
#ifndef TARG_ST
  *is_64bit = (e_flags & EF_IRIX_ABI64);
  *isa = 1;
#else
  // [CL] check coherency between user options and file flags
  *is_64bit = FALSE;
  *isa = (INT)Target_ISA;

  switch(e_flags & ELF_ST200_CORE_MASK) {
  case ELF_ST200_CORE_ST220:
    if ((Target != TARGET_st220) && (Target != TARGET_UNDEF)) {
      ErrMsg(EC_Conf_Targ, "processor", Targ_Name(Target), Targ_Name(TARGET_st220));
    }
    if (Target_ABI == ABI_UNDEF) {
      Target = TARGET_st220;
    }
    break;
  case ELF_ST200_CORE_ST230:
    if ((Target != TARGET_st230) && (Target != TARGET_UNDEF)) {
      ErrMsg(EC_Conf_Targ, "processor", Targ_Name(Target), Targ_Name(TARGET_st230));
    }
    if (Target_ABI == ABI_UNDEF) {
      Target = TARGET_st230;
    }
    break;
  case ELF_ST200_CORE_ST231:
    if ((Target != TARGET_st231) && (Target != TARGET_UNDEF)) {
      ErrMsg(EC_Conf_Targ, "processor", Targ_Name(Target), Targ_Name(TARGET_st231));
    }
    if (Target_ABI == ABI_UNDEF) {
      Target = TARGET_st231;
    }
    break;
  case ELF_ST200_CORE_ST235:
    if ((Target != TARGET_st235) && (Target != TARGET_UNDEF)) {
      ErrMsg(EC_Conf_Targ, "processor", Targ_Name(Target), Targ_Name(TARGET_st235));
    }
    if (Target_ABI == ABI_UNDEF) {
      Target = TARGET_st235;
    }
    break;
  default:
    ErrMsg(EC_Conf_Targ, "processor", Abi_Name(Target_ABI), "undefined/unsupported");
    Target = TARGET_UNDEF;
    break;
  }

  switch(e_flags & ELF_ST200_ABI_MASK) {
  case ELF_ST200_ABI_EMBED:
    if ((Target_ABI != ABI_ST200_embedded) && (Target_ABI != ABI_UNDEF)) {
      ErrMsg(EC_Conf_Targ, "abi", Abi_Name(Target_ABI), Abi_Name(ABI_ST200_embedded));
    }
    if (Target_ABI == ABI_UNDEF) { 
      Target_ABI = ABI_ST200_embedded;
    }
    break;
  case ELF_ST200_ABI_PIC:
    if ((Target_ABI != ABI_ST200_PIC) && (Target_ABI != ABI_UNDEF)) {
      ErrMsg(EC_Conf_Targ, "abi", Abi_Name(Target_ABI), Abi_Name(ABI_ST200_PIC));
    }
    if (Target_ABI == ABI_UNDEF) { 
      Target_ABI = ABI_ST200_PIC;
    }
    break;
  default:
    ErrMsg(EC_Conf_Targ, "abi", Abi_Name(Target_ABI), "undefined/unsupported");
    Target_ABI = ABI_UNDEF;
    break;
  }

  switch (e_flags & ELF_ST200_CODEGEN_MASK) {
  case ELF_ST200_CODEGEN_CPIC:
    if ( (!Gen_PIC_Call_Shared && Gen_PIC_Call_Shared_Set) ||
	 (Gen_PIC_Shared && Gen_PIC_Shared_Set) ) {
      if (!No_Shared_Warning) {
	ErrMsg(EC_Conf_CodeGen, "-call_shared");
      }
    }
    break;
  case ELF_ST200_CODEGEN_PIC:
#if 0
    // [CL] don't emit a warning in this case
    if ( (!Gen_PIC_Shared && Gen_PIC_Shared_Set) ||
	 (Gen_PIC_Call_Shared && Gen_PIC_Call_Shared_Set) ) {
      if (!No_Shared_Warning) {
	ErrMsg(EC_Conf_CodeGen, "-shared");
      }
    }
#endif
    break;
  case ELF_ST200_CODEGEN_ABSOLUTE:
    if ( (Gen_PIC_Shared && Gen_PIC_Shared_Set) ||
	 (Gen_PIC_Call_Shared && Gen_PIC_Call_Shared_Set) ) {
      if (!No_Shared_Warning) {
	ErrMsg(EC_Conf_CodeGen, "-non_shared");
      }
    }
    break;
  }
#endif
}

/* ====================================================================
 *   Config_ELF_From_Target
 *
 *   Return the ELF specific flags for the current target ABI and 
 *   ISA subset.
 * ====================================================================
 */
Elf32_Word 
Config_ELF_From_Target (
  BOOL is_64bit, 
  BOOL old_abi, 
  INT isa
)
{
  Elf32_Word e_flags = 0;

#ifndef TARG_ST
  if (is_64bit) e_flags |= EF_IRIX_ABI64;
#else
  // [CL] setup ELF flags according to user options

  if (isa != (INT)TARGET_ISA_ST220
      && isa != (INT)TARGET_ISA_ST230
      && isa != (INT)TARGET_ISA_ST231
      && isa != (INT)TARGET_ISA_ST235) {
      // [HK]
//        ErrMsg ( EC_Inv_TARG, "isa", Isa_Name((enum TARGET_ISA)isa) );
      ErrMsg ( EC_Inv_TARG, "isa", Isa_Name((TARGET_ISA)isa) );
  }

  // Handle ABI
  switch(Target_ABI) {
  case ABI_ST200_embedded:
    e_flags |= ELF_ST200_ABI_EMBED; break;
  case ABI_ST200_PIC:
    e_flags |= ELF_ST200_ABI_PIC; break;
  default:
    e_flags |= ELF_ST200_ABI_UNDEF; break;
  }

  // Skip Mode (User, Kernel)

  // Handle Core
  switch(Target) {
    // [CL} ST210 is not supported by the compiler
    //  case TARGET_st210:
    //    e_flags |= ELF_ST200_CORE_ST210; break;
  case TARGET_st220:
    e_flags |= ELF_ST200_CORE_ST220; break;
  case TARGET_st230:
    e_flags |= ELF_ST200_CORE_ST230; break;
  case TARGET_st231:
    e_flags |= ELF_ST200_CORE_ST231; break;
  case TARGET_st235:
    e_flags |= ELF_ST200_CORE_ST235; break;
  default:
    e_flags |= ELF_ST200_CORE_UNDEF; break;
  }

  // Ignore Cut

  // Handle Code Generation Model
  if (Gen_PIC_Call_Shared) {
    e_flags |= ELF_ST200_CODEGEN_CPIC;
  }
  else if (Gen_PIC_Shared) {
    e_flags |= ELF_ST200_CODEGEN_PIC;
  }
  else {
    e_flags |= ELF_ST200_CODEGEN_ABSOLUTE;
  }
#endif

  return e_flags;
}

/* ====================================================================
 *   Get_Elf_Target_Machine
 * ====================================================================
 */
Elf32_Half 
Get_Elf_Target_Machine (void)
{
  // [CL] this ID would need to be sync'ed with binutils
  return EM_ST200;
}


