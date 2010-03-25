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

#ifdef TARG_ST200
// [SC]: Control emission of assembler nop insertion directives.
extern BOOL CG_nop_insertion_directives;
#endif

// whether to use the base st for the reloc.
extern BOOL CGEMIT_Use_Base_ST_For_Reloc (INT reloc, ST *st);

// put reloc info into asm buffer, returning # parens to close with.
extern INT CGEMIT_Relocs_In_Asm (TN *t, ST *st, vstring *buf, INT64 *val);
// put reloc info into object file.
extern void CGEMIT_Relocs_In_Object (
	TN *t, ST *st, INT32 PC, pSCNINFO PU_section, INT64 *val);

// add events and relocs as needed for call.
extern void CGEMIT_Add_Call_Information (
	OP *op, BB *bb, INT32 PC, pSCNINFO PU_section);

extern void
CGEMIT_Prn_Scn_In_Asm (ST *st, 
		       Elf64_Word scn_type, 
		       Elf64_Word scn_flags,
		       Elf64_Xword scn_entsize, 
		       ST *cur_section);
extern void
CGEMIT_Prn_Scn_In_Asm (FILE       *asm_file,
		       const char *scn_name,
		       Elf64_Word  scn_type,
		       Elf64_Word  scn_flags,
		       Elf64_Xword scn_entsize,
		       Elf64_Word  scn_align,
		       const char *cur_scn_name);
extern void
CGEMIT_Prn_Line_Dir_In_Asm (USRCPOS usrcpos);
extern void
CGEMIT_Prn_File_Dir_In_Asm(USRCPOS usrcpos,
                        const char *pathname,
                        const char *filename);

// Generate the .frame, .mask and the .fmask directives for the assembler.
extern void CGEMIT_Gen_Asm_Frame (INT64 frame_len);

// Prepass before emit to fix up fp registers for IA-32.
// Noop for every other target.
extern void STACK_FP_Fixup_PU();

// Generate the entry (.proc) directive.
extern void CGEMIT_Prn_Ent_In_Asm (ST *pu);

// generate predicate suffix depending of the TOP
extern char* CGEMIT_Asm_Predicate_Suffix( OP *op );

// generate value in hexa decimal for particular op or operand.
// Case of push/pop mask.
extern BOOL CGEMIT_TN_Value_In_Hexa_Format( OP *op, TN *t );

// generate weak alias directive.
extern void CGEMIT_Weak_Alias (ST *sym, ST *strongsym);

// generate alias directive.
extern void CGEMIT_Alias (ST *sym, ST *strongsym);

#ifdef TARG_ST
#ifdef TARG_STxP70
// Special registers fixup just before verify_instruction 
extern void CGEMIT_Special_Register_Function_Fixup( OP *op );

// Directive for HW Loop cross-end
extern void CGEMIT_Loop_CrossEnd_Directive(BB* bb);
// Directive for HW Loop end alignement
extern void CGEMIT_Loop_End_Directive(BB* bb);
// Directive for call returns addresses alignement
extern void CGEMIT_CallReturns_Directive(BB* bb);

#endif

#ifdef TARG_ARM
// Emit register list from constant value.
extern BOOL CGEMIT_Register_List(OP *op, INT opnd_idx, vstring *buf);
#endif

// generate procedure end directive
extern void CGEMIT_Exit_In_Asm (ST *pu);

// generate asm directives just before asm
extern void CGEMIT_Asm_String_Prefix (OP *op, INT32 *PC);

// generate asm directives just after asm
extern void CGEMIT_Asm_String_Suffix (OP *op, INT32 *PC);

// Called at begining of file.
extern void CGEMIT_Begin_File_In_Asm (void);
// Called at end of file.
extern void CGEMIT_End_File_In_Asm (void);

// Returns into buf the qualified name for a symbol to be used in
// the asm output. Moved from r_qualified_name in cgemit.cxx.
// The returned string 
extern void CGEMIT_Qualified_Name(ST *st, vstring *buf);

/* returns whether END_GROUP (;;) must be preceeded by a \n */
extern bool CGEMIT_NewLine_Before_ISA_PRINT_END_GROUP (void);

#endif
