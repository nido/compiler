/*

  Copyright (C) 2002 ST Microelectronics, Inc.  All Rights Reserved.

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

  Contact information:

  For further information regarding this notice, see:

*/


#include <stdio.h>
#include <stdlib.h>
// [HK]
#if __GNUC__ >= 3
#include <list>
using std::list;
#else
#include <list.h>
#endif // __GNUC__ >= 3

#include "wn.h"
#include "data_layout.h"
#include "cgir.h"
#include "cgdwarf.h"               /* [CL] for CGD_LABIDX */
#include "cgdwarf_debug_frame.h"

// is 'op' a move from FP to SP ? (ie restore SP from FP)
static BOOL Restore_SP_From_FP(OP *op)
{
  if (OP_move(op) && (OP_result(op,0) == SP_TN)
      && (OP_opnd(op,0) == FP_TN)) {
    return TRUE;
  }
  return FALSE;
}

// is 'op' a move from xx to FP ? (ie restore FP from xx)
// or a restore of FP from the stack
static BOOL Restore_FP(OP *op)
{
  if (OP_move(op) && (OP_result(op,0) == FP_TN)) {
    return TRUE;
  } else if (OP_load(op) && (OP_result(op,0) == FP_TN)) {
    if (OP_findopnd(op, OU_base) == SP_TN) {
      return TRUE;
    }
  }
  return FALSE;
}

// is 'op' a move from SP to FP ? (ie define FP as equal to SP)
static BOOL Copy_SP_To_FP(OP *op)
{
  if (OP_move(op) && (OP_result(op,0) == FP_TN)
      && (OP_opnd(op,0) == SP_TN)) {
    return TRUE;
  }
  return FALSE;
}

static void Record_Register_Save(OP* op, INT opndnum, BB* bb, UNWIND_ELEM* ue,
				 BOOL opnd_is_multi)
{
  TOP opc = OP_code(op);

  if (Trace_Unwind && opndnum < 0) {
    Print_OP_No_SrcLine(op);
  }
  FmtAssert(opndnum >= 0, ("incorrect operand id %d for %s", opndnum, TOP_Name(opc)));

  TN* save_tn = Get_Copied_Save_TN(OP_opnd(op,opndnum), op, bb);
  if (save_tn) {
    opndnum = OP_find_opnd_use(op, OU_base);

    if (Trace_Unwind && opndnum < 0) {
      Print_OP_No_SrcLine(op);
    }
    FmtAssert(opndnum >= 0, ("no OU_base for %s", TOP_Name(opc)));

    if (TN_is_save_reg(save_tn)) {
      ue->rc_reg = Get_Debug_Reg_Id(save_tn);
    } else {
      ue->rc_reg = Get_Debug_Reg_Id(save_tn);
    }
    TN *store_tn = OP_opnd(op,opndnum);
    if (store_tn == SP_TN || store_tn == FP_TN) {
      ue->kind = (store_tn == SP_TN) ? UE_SAVE_SP : UE_SAVE_FP;

      TN* offset_tn = OP_opnd(op, OP_find_opnd_use(op, OU_offset));

      // don't record constant offsets, they are not related to
      // restores of callee-saved (they appear in array access for
      // instance)
      if (TN_is_symbol(offset_tn)) {
	ST* st = TN_var(offset_tn);
	ST* base_st;
	INT64 base_ofst;

	Base_Symbol_And_Offset(st, &base_st, &base_ofst);
	if (Trace_Unwind && base_st != SP_Sym && base_st != FP_Sym) {
	  Print_OP_No_SrcLine(op);
	}
	FmtAssert(base_st == SP_Sym || base_st == FP_Sym,
		  ("not saving to the stack!"));

	ue->offset = CGTARG_TN_Value (offset_tn, base_ofst);

	// handle the multi-operand case.
	if (OP_multi(op)) {
	  if (!opnd_is_multi) {
	    // 1st operand
	    if (Target_Byte_Sex == BIG_ENDIAN) {
	      ue->offset += 4;
	    }
	  } else {
	    // 2nd operand (opnd_is_multi)
	    if (Target_Byte_Sex == LITTLE_ENDIAN) {
	      ue->offset += 4;
	    }
	  }
	}

#ifdef DEBUG_UNWIND
	fprintf(TFile, "** %s register %llu offset = %lld\n", __FUNCTION__,
		ue->rc_reg, ue->offset);
#endif
      }
    }
  } else {
#ifdef DEBUG_UNWIND
    fprintf(TFile, "** %s no save tn\n", __FUNCTION__);
#endif
  }
}

static void Record_Register_Restore(OP* op, INT opndnum, BB* bb,
				    UNWIND_ELEM* ue, BOOL opnd_is_multi)
{
  TOP opc = OP_code(op);

  if (Trace_Unwind && opndnum < 0) {
    Print_OP_No_SrcLine(op);
  }
  FmtAssert(opndnum >= 0, ("incorrect result id %d for %s", opndnum, TOP_Name(opc)));

  TN* result_tn = OP_result(op,opndnum);

  if (TN_Is_Unwind_Reg(result_tn)) {
    opndnum = OP_find_opnd_use(op, OU_base);

    if (Trace_Unwind && opndnum < 0) {
      Print_OP_No_SrcLine(op);
    }
    FmtAssert(opndnum >= 0, ("no OU_base for %s", TOP_Name(opc)));

    TN *load_tn = OP_opnd(op,opndnum);
    if (load_tn == SP_TN || load_tn == FP_TN) {
      ue->kind = (load_tn == SP_TN) ? UE_RESTORE_SP : UE_RESTORE_FP;
      ue->rc_reg = Get_Debug_Reg_Id(result_tn);

      TN* offset_tn = OP_opnd(op, OP_find_opnd_use(op, OU_offset));

      // don't record constant offsets, they are not related to
      // restores of callee-saved (they appear in array access for
      // instance)
      if (TN_is_symbol(offset_tn)) {
	ST* st = TN_var(offset_tn);
	ST* base_st;
	INT64 base_ofst;

	Base_Symbol_And_Offset(st, &base_st, &base_ofst);
	if (Trace_Unwind && base_st != SP_Sym && base_st != FP_Sym) {
	  Print_OP_No_SrcLine(op);
	}
	FmtAssert(base_st == SP_Sym || base_st == FP_Sym,
		  ("not restoring from the stack!"));

	ue->offset = CGTARG_TN_Value (offset_tn, base_ofst);

	// handle the multi-operand case. 
	if (OP_multi(op)) {
	  if (!opnd_is_multi) {
	    // 1st operand
	    if (Target_Byte_Sex == BIG_ENDIAN) {
	      ue->offset += 4;
	    }
	  } else {
	    // 2nd operand (opnd_is_multi)
	    if (Target_Byte_Sex == LITTLE_ENDIAN) {
	      ue->offset += 4;
	    }
	  }
	}

#ifdef DEBUG_UNWIND
	if (ue->kind != UE_UNDEFINED) {
	  fprintf(TFile, "** %s restore register %llu from mem offset %lld\n",
		  __FUNCTION__, ue->rc_reg, ue->offset);
	}
#endif
      }
    }
  }
}

void Analyze_OP_For_Unwind_Info (OP *op, UINT when, BB *bb)
{
  UNWIND_ELEM ue;
  ue.kind = UE_UNDEFINED;
#ifdef PROPAGATE_DEBUG
  ue.valid = TRUE;
  ue.propagated = UE_PROP_NONE;
  ue.top_offset = 0;
  ue.is_copy = FALSE;
  ue.after_sp_adj = FALSE;
  ue.handler = FALSE;
#endif
  ue.label_idx = LABEL_IDX_ZERO;
  TN *tn;
  TOP opc = OP_code(op);
  INT opnd_idx;

  if (OP_simulated(op) || opc == TOP_asm) {
    has_asm = TRUE;
  }

#ifdef DEBUG_UNWIND
  fprintf(TFile, "WHEN %d BB %d\n", when, BB_id(bb));
  Print_OP_No_SrcLine(op);
#endif

  ue.frame_changed = FALSE;

  if ( OP_has_result(op)
       && ( (OP_result(op,0) == SP_TN) || (OP_result(op,0) == FP_TN) ) ) {

    // several cases need to be handled:
    // 1. normal one: definition of SP/FP with add/sub constant or register
    // 2. if FP is defined, any insn that modifies SP is to be ignored:
    //    it only serves to reserve space on the stack but cannot be used
    //    to compute the CFA
    // 3. exception to the previous rule: when SP is restored from FP
    // 4. restore of FP: ignore here, handled below

    // case 2: if FP is already defined, ignore this redefinition of SP
    if (PU_has_FP && !PU_has_restored_FP && !Restore_SP_From_FP(op)) {
      ue.kind = UE_UNDEFINED;

      // case 3:
    } else if (PU_has_FP && !PU_has_restored_FP && Restore_SP_From_FP(op)) {
      ue.kind = UE_DESTROY_FP;
      ue.rc_reg = Get_Debug_Reg_Id(SP_TN);
      PU_has_restored_FP = TRUE;

      // case 4:
    } else if ( (!PU_has_FP || PU_has_restored_FP) && Restore_FP(op)
		&& !Copy_SP_To_FP(op) ) {
      // a copy of SP to FP is considered as a definition of FP,
      // to be handled as case 1 below
      ue.kind = UE_UNDEFINED;

      // case 1:
    } else {
      OP* frame_op = NULL;
      if (OP_opnds(op) > 1 && ( OP_isub(op) || OP_iadd(op) ) ) {
	TN* frame_tn = OP_opnd(op,1);
	if (TN_is_constant(frame_tn)) {
	  ue.offset = TN_value (frame_tn);
	  goto case1_OK;
	} else {
	  // [CL] frame size is in a register defined earlier
	  frame_op = Find_Def_Of_TN (frame_tn, op, bb);
	  if (frame_op && OP_move(frame_op)) {
	    frame_tn = OP_opnd(frame_op,0);
	    if (TN_is_constant(frame_tn)) {
	      ue.offset = TN_value (frame_tn);
	      goto case1_OK;
	    }
	  }
	  // [SC] Allow a special case:
	  // the post-adjustment of SP in an EH_return exit.
	  // This happens only in an EH_return exit, and it
	  // happens after the real exit stack adjust.
	  // Note: there must be a real exit stack adjust, since an
	  // EH_return requires registers to be saved in the stack frame.
	  EXITINFO *exit_info;
	  OP *sp_adj;
	  OP *p;
	  if (OP_iadd (op)
	      && OP_result (op, 0) == SP_TN
	      && OP_opnd(op, 0) == SP_TN
	      && (exit_info = ANNOT_exitinfo(ANNOT_Get (BB_annotations(bb),
							ANNOT_EXITINFO)))
	      && EXITINFO_is_eh_return (exit_info)
	      && (sp_adj = EXITINFO_sp_adj (exit_info))) {
	    for (p = OP_next(sp_adj); p != NULL; p = OP_next (p)) {
	      if (p == op) {
		// Satisfied all the conditions for an EH_return
		// post-adjustment of SP.  We generate no unwind info
		// for this op.
		return;
	      }
	    }
	  }	      
	}
      } else {
	// FP/SP is defined as a move from another reg
	if (OP_move(op)) {
	  ue.offset = 0;
	  goto case1_OK;
	}
      }

      if (Trace_Unwind) {
	Print_OP_No_SrcLine(op);
	if (frame_op) Print_OP_No_SrcLine(frame_op);
      }
      FmtAssert(FALSE, ("unwind: unable to compute frame size"));

    case1_OK:
      if (OP_result(op,0) == SP_TN) {
          ue.rc_reg = Get_Debug_Reg_Id(SP_TN);
      
	if ( ( ue.offset > 0 && OP_isub(op) )
	     || ( ue.offset < 0 && OP_iadd(op) )
	     ) {
	  // SP is decreased
	  ue.kind = UE_CREATE_FRAME;
	} else if ( ( ue.offset < 0 && OP_isub(op) )
		    || ( ue.offset > 0 && OP_iadd(op) )
		    ) {
	  // SP is increased
	  ue.kind = UE_DESTROY_FRAME;
	} else {
	  if (Trace_Unwind) {
	    Print_OP_No_SrcLine(op);
	  }
	  FmtAssert(FALSE, ("unwind: unsupported operation on SP"));
	}

#ifdef DEBUG_UNWIND
	fprintf(TFile, "** %s change SP size %lld\n", __FUNCTION__, ue.offset);
#endif

      } else if (OP_result(op,0) == FP_TN) {

	// FP must be defined as an offset from SP
	if (Trace_Unwind) {
	  if (
	      (OP_opnd(op,0) == SP_TN)
	      &&
	      (OP_iadd(op) || OP_isub(op) || OP_move(op))
	      ) { /* OK */ } else 
	      {
		Print_OP_No_SrcLine(op);
	      }
	}

	FmtAssert(OP_opnd(op,0) == SP_TN,
		  ("unwind: FP not defined relative to SP"));
	FmtAssert(OP_iadd(op) || OP_isub(op) || OP_move(op),
		  ("unwind: FP not defined relative to SP"));

	ue.rc_reg = Get_Debug_Reg_Id(FP_TN);

	if ( ( ue.offset >= 0 && OP_iadd(op) )
	     || ( ue.offset <= 0 && OP_isub(op) )
	     || ( ue.offset == 0 && OP_move(op) )
	     ) {
	  // FP is higher than SP
	  ue.kind = UE_CREATE_FP;
	  PU_has_FP = TRUE;
	}
	// FP destruction is handled above
#ifdef DEBUG_UNWIND
	fprintf(TFile, "** %s change FP size %lld\n", __FUNCTION__, ue.offset);
#endif
      }

      ue.offset = abs(ue.offset);
    }
  }

  if (ue.kind != UE_UNDEFINED) 
	;	// already found
  else if (OP_store(op)) 
  {
#ifdef DEBUG_UNWIND
    fprintf(TFile, "** %s store\n", __FUNCTION__);
#endif

    // find def of storeval and see if it is copy of save reg.
    // also check if base comes from spill symbol.
    INT opndnum = OP_find_opnd_use(op, OU_storeval);
    Record_Register_Save(op, opndnum, bb, &ue, FALSE);

    if (OP_multi(op)) {
      // if there are 2 inputs, record the 1st one, and prepare the next one
      Record_UE(op, &ue, bb, when);

      opndnum = OP_find_opnd_use(op, OU_multi);
      Record_Register_Save(op, opndnum, bb, &ue, TRUE);
    }
  } else if (OP_load(op)) {
#ifdef DEBUG_UNWIND
    fprintf(TFile, "** %s load\n", __FUNCTION__);
#endif

    // check if we are loading a callee saved
    // from the stack
    INT opndnum = 0;
    Record_Register_Restore(op, opndnum, bb, &ue, FALSE);

    if (OP_multi(op)) {
      // if there are 2 results, record the 1st one, and prepare the next one
      Record_UE(op, &ue, bb, when);

      opndnum = OP_find_result_with_usage(op, OU_multi);
      Record_Register_Restore(op, opndnum, bb, &ue, TRUE);
    }
  }
  else if ((opnd_idx = OP_Copy_Operand(op)) != -1) 
  {
#ifdef DEBUG_UNWIND
    fprintf(TFile, "** %s copy\n", __FUNCTION__);
#endif
    TN* copy_tn = OP_opnd(op, opnd_idx);
    TN* result_tn = OP_result(op, OP_Copy_Result(op));
      
    // check that we are copying a callee saved,
    // and that the destination is not a dedicated
    // there is a special case for FP, which is removed
    // from the callee_save register set when used in a PU
    if ( (TN_Is_Unwind_Reg(copy_tn) || copy_tn==FP_TN)
	 && TN_is_register(result_tn)
	 && !TN_is_dedicated(result_tn))
      {
	ue.kind = UE_SAVE_GR;
	ue.rc_reg = Get_Debug_Reg_Id(copy_tn);
	ue.save_rc_reg = Get_Debug_Reg_Id(result_tn);

	// or check that we are restoring a callee-saved,
	// not from a dedicated
      } else if ( (TN_Is_Unwind_Reg(result_tn) || result_tn==FP_TN)
		  && TN_is_register(copy_tn)
		  && !TN_is_dedicated(copy_tn))
      {
	ue.kind = UE_RESTORE_GR;
	ue.rc_reg = Get_Debug_Reg_Id(result_tn);
	ue.save_rc_reg = Get_Debug_Reg_Id(copy_tn);
      }
  }

  Record_UE(op, &ue, bb, when);
}

#include <elf.h>
#include <elfaccess.h>

void
Check_Dwarf_Rel(const Elf32_Rel &current_reloc)
{
  FmtAssert(REL32_type(current_reloc) == R_IA_64_DIR32MSB,
	    ("Unimplemented 32-bit relocation type %d",
	     REL32_type(current_reloc)));
}

void
Check_Dwarf_Rel(const Elf64_Rel &current_reloc)
{
  FmtAssert(REL64_type(current_reloc) == R_IA_64_DIR64MSB,
	    ("Unimplemented 64-bit relocation type %d",
	     REL64_type(current_reloc)));
}

void
Check_Dwarf_Rela(const Elf64_Rela &current_reloc)
{
  FmtAssert(FALSE,
	    ("Unimplemented 64-bit relocation type %d",
	     REL64_type(current_reloc)));
}

void
Check_Dwarf_Rela(const Elf32_Rela &current_reloc)
{
  FmtAssert(FALSE,
	    ("Unimplemented 32-bit relocation type %d",
	     REL32_type(current_reloc)));
}

static char *drop_these[] = {
      // Assembler generates .debug_line from directives itself, so we
      // don't output it.
#ifndef TARG_ST200 /* CLYON: ST200 gas does not understand .loc directives */
	".debug_line",
	".eh_frame", 
#endif
#if 0
     // gdb does not use the MIPS sections
     // debug_weaknames, etc.
	".debug_weaknames",
	".debug_varnames",
	".debug_typenames",
	".debug_funcnames",
     // we don't use debug_frame in IA-64.
	".debug_frame",
#endif
	0
};
// return TRUE if we want to emit the section (IA-64).
// return FALSE if we do not want to for IA-64.
extern BOOL Is_Dwarf_Section_To_Emit(const char *name)
{

	for(int  i = 0; drop_these[i]; ++i) {
	  if(strcmp(name,drop_these[i]) == 0) {
	    return FALSE;
	  }
	}
        return TRUE;
}
