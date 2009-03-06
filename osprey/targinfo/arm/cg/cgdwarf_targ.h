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


#ifndef cgdwarf_targ_INCLUDED
#define cgdwarf_targ_INCLUDED

extern BOOL Trace_Dwarf;

void Analyze_OP_For_Unwind_Info (OP *op, UINT when, BB *bb);

// enum of all preserved regs (PR) that can be saved/restored
typedef enum {
    PR_R6,
    PR_R7,
    PR_R8,
    PR_R9,
    PR_R10,
    PR_R11,
    PR_R24,
    PR_R25,
    PR_R26,
    PR_R27,
    PR_R28,
    PR_R29,
    PR_R30,
    PR_R31,
    PR_LK,
    PR_SP,
    PR_GR,
    PR_LC0,
    PR_LS0,
    PR_LE0,
    PR_LR0,
    PR_GP,
    PR_LAST
} PR_TYPE;
#define PR_FIRST PR_R6

PR_TYPE CR_To_PR (CLASS_REG_PAIR crp);
CLASS_REG_PAIR PR_To_CR (PR_TYPE p);

/* construct the fde for the current procedure. */

#ifdef TARG_ST
// [CL] changed begin/end_label type and added
// scn_index param to be used by the debug_frame
// machinery along with begin_label
extern Dwarf_P_Fde Build_Fde_For_Proc (Dwarf_P_Debug  dw_dbg,
				       BB            *firstbb,
				       Dwarf_Unsigned begin_label,
				       Dwarf_Unsigned end_label,
				       INT32          end_offset,
				       // The following two arguments
				       // need to go away once
				       // libunwind gives us an
				       // interface that supports
				       // symbolic ranges.
				       INT       low_pc,
				       INT       high_pc,
				       Elf64_Word	scn_index,
				       bool emit_restores=false);
#else
extern Dwarf_P_Fde Build_Fde_For_Proc (Dwarf_P_Debug  dw_dbg,
				       BB            *firstbb,
				       LABEL_IDX      begin_label,
				       LABEL_IDX      end_label,
				       INT32          end_offset,
				       // The following two arguments
				       // need to go away once
				       // libunwind gives us an
				       // interface that supports
				       // symbolic ranges.
				       INT       low_pc,
				       INT       high_pc);
#endif

extern void Check_Dwarf_Rel(const Elf32_Rel &);
extern void Check_Dwarf_Rel(const Elf64_Rel &);
extern void Check_Dwarf_Rela(const Elf64_Rela &);
extern void Check_Dwarf_Rela(const Elf32_Rela &);
extern BOOL Is_Dwarf_Section_To_Emit(const char *name);
extern BOOL Dwarf_Require_Symbolic_Offsets();

extern void Init_Unwind_Info (BOOL trace);
extern void Finalize_Unwind_Info(void);
#ifdef TARG_ST
// [CL] need to emit labels after bundles too
extern void Emit_Unwind_Directives_For_OP(OP *op, FILE *f, BOOL after_op,
					  BOOL inserted_late);
#else
extern void Emit_Unwind_Directives_For_OP(OP *op, FILE *f);
#endif

#endif /* cgdwarf_targ_INCLUDED */
