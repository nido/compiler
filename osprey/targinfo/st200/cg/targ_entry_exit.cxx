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

#include "defs.h"
#include "erglob.h"
#include "erbe.h"
#include "glob.h"
#include "tracing.h"
#include "config_target.h"
#include "config.h"
#include "config_TARG.h"

#include "symtab.h"
#include "strtab.h"
#include "be_symtab.h"
#include "targ_isa_lits.h"
#include "topcode.h"
#include "cgir.h"
#include "tn_set.h"
#include "tn_map.h"

#include "ttype.h"
#include "const.h"
#include "data_layout.h"
#include "targ_sim.h"
#include "whirl2ops.h"
#include "entry_exit_targ.h"
#include "targ_abi_properties.h"

#include "cg_flags.h"
#include "cg.h"
#include "calls.h"
#include "register.h"
#include "cgexp.h"
#include "cg_spill.h"
#include "cgtarget.h"
#include "mexpand.h"
#include "reg_live.h"

typedef std::pair<TN *, TN *> TN_TN_PAIR;
typedef std::vector<TN_TN_PAIR> TN_TN_PAIR_VECTOR;
typedef std::pair<TN *, ST *> TN_SYMBOL_PAIR;
typedef std::vector<TN_SYMBOL_PAIR> TN_SYMBOL_PAIR_VECTOR;
typedef std::vector<CLASS_REG_PAIR> CREG_VECTOR;
static TN_SYMBOL_PAIR_VECTOR callee_saved_vector;
struct compare_creg_lt {
  bool operator()(const CLASS_REG_PAIR& p1, const CLASS_REG_PAIR& p2) {
    return CLASS_REG_PAIR_class_n_reg(p1) < 
      CLASS_REG_PAIR_class_n_reg(p2);
  }
};

/* ====================================================================
 *   EETARG_Build_Jump_Instead_Of_Call (call_op)
 * ====================================================================
 */
OP *
EETARG_Build_Jump_Instead_Of_Call (
  OP *call_op
)
{
  OP *jump_op;
  TOP jump_top;
  TOP call_top = OP_code(call_op);
  switch (call_top) {
  case TOP_call_i:
    jump_op = Mk_OP(TOP_goto_i, OP_opnd(call_op, 0));
    break;
  case TOP_icall:
    jump_op = Mk_OP(TOP_igoto, OP_opnd(call_op, 0));
    break;
  default:
    FmtAssert(FALSE, ("don't know how to generate tail call for %s",
		     TOP_Name(call_top)));
    /*NOTREACHED*/
  }

  return jump_op;
}

static void
init_callee_saved_symbols(void)
{
  CREG_VECTOR save_creg_vector;
  callee_saved_vector.clear();

  INT callee_num;
  for (callee_num = 0; callee_num < Callee_Saved_Regs_Count; callee_num++) {
    TN *tn = CALLEE_tn(callee_num);
    ISA_REGISTER_CLASS cl = TN_save_rclass(tn);
    REGISTER reg = TN_save_reg(tn);
    if (EETARG_Save_With_Regmask (cl, reg)
	&& (REGISTER_SET_MemberP(Callee_Saved_Regs_Mask[cl], reg)
	    || (PU_Has_EH_Return
		&& REGISTER_SET_MemberP(REGISTER_CLASS_eh_return(cl), reg))))
      save_creg_vector.push_back(TN_save_creg(tn));
  }
  if (RA_TN != NULL &&
      EETARG_Save_With_Regmask (REGISTER_CLASS_ra, REGISTER_ra)
      && REGISTER_SET_MemberP(Callee_Saved_Regs_Mask[REGISTER_CLASS_ra], 
			      REGISTER_ra)) {
    save_creg_vector.push_back(TN_class_reg(RA_TN));
  }
    
  std::stable_sort(save_creg_vector.begin(),
		   save_creg_vector.end(),
		   compare_creg_lt());

  INT i;
  for (i = 0; i < save_creg_vector.size(); i++) {
    CLASS_REG_PAIR creg1 = save_creg_vector[i];
    if (Enable_64_Bits_Ops &&
	i < save_creg_vector.size()-1) {
      CLASS_REG_PAIR creg2 = save_creg_vector[i+1];
      if (CLASS_REG_PAIR_rclass(creg1) == CLASS_REG_PAIR_rclass(creg2) &&
	  REGISTER_SET_MemberP(REGISTER_SUBCLASS_members(ISA_REGISTER_SUBCLASS_paired), CLASS_REG_PAIR_reg(creg1)) &&
	  CLASS_REG_PAIR_reg(creg1) + 1 == CLASS_REG_PAIR_reg(creg2))
	{
	  TY_IDX ty = MTYPE_To_TY(MTYPE_I8);
	  ST *st = CGSPILL_Gen_Spill_Symbol(ty, "callee_paired_spill");
	  TN *tn = Gen_Register_TN (CLASS_REG_PAIR_rclass (creg1), 8);
	  Set_TN_register (tn, CLASS_REG_PAIR_reg (creg1));
	  Set_TN_save_creg (tn, creg1);
	  Set_TN_spill (tn, st);
	  callee_saved_vector.push_back(TN_SYMBOL_PAIR(tn,st));
	  i++;
	  continue;
	}
    }
    TY_IDX ty = MTYPE_To_TY(MTYPE_I4);
    ST *st = CGSPILL_Gen_Spill_Symbol(ty, "callee_spill");
    TN *tn = Gen_Register_TN (CLASS_REG_PAIR_rclass (creg1), 4);
    Set_TN_register (tn, CLASS_REG_PAIR_reg (creg1));
    Set_TN_save_creg (tn, creg1);
    Set_TN_spill (tn, st);
    callee_saved_vector.push_back(TN_SYMBOL_PAIR(tn,st));
  }
}

void
EETARG_Fixup_Stack_Frame (void)
{
  init_callee_saved_symbols();
}

/* ====================================================================
 *   EETARG_Set_Frame_Len
 *
 *   Setup the computed frame len for the specific code defined here.
 * ====================================================================
 */
void
EETARG_Set_Frame_Len(INT64 frame_len)
{
  /* No op for ST200. */
}

/* ====================================================================
 *   EETARG_Fixup_Entry_Code
 *
 *   Generate push/pop sequence.
 * ====================================================================
 */
void
EETARG_Fixup_Entry_Code (
  BB *bb
)
{
  if (callee_saved_vector.size() == 0) return;

  ANNOTATION *ant = ANNOT_Get(BB_annotations(bb), ANNOT_ENTRYINFO);
  ENTRYINFO *entry_info = ANNOT_entryinfo(ant);
  OP *point = ENTRYINFO_sp_adj(entry_info);

  INT i;
  OPS ops = OPS_EMPTY;
  for (i = 0; i < callee_saved_vector.size(); i++) {
    TN_SYMBOL_PAIR pair = callee_saved_vector[i];
    TN *src_tn = pair.first;
    ST *spill_loc = pair.second;
    CGTARG_Store_To_Memory(src_tn, spill_loc, &ops);
    // Mark the actual store as a spill
    OP *op = OPS_last(&ops);
    while(!OP_store(op)) op = OP_prev(op);
    Set_OP_spill(op);
    Set_OP_spilled_tn(op, src_tn);
  }

  // If in the prolog a callee saved was used before the sp_adjust op
  // we need to rename it into a caller saved and restore it after the
  // generation of the callee spill.
  // This happens for the frame pointer that is moved to a temporary
  // before the sp adjust code. It happens that this temporary is
  // allocated top a preserved register
  // FdF 20070604: Add recoloration code to support instructions moved
  // before the spadjust operation.
  if (point != NULL) {
    REGISTER_SET avail_temps[ISA_REGISTER_CLASS_MAX+1];
    REG_LIVE_Prolog_Temps(bb, BB_first_op(bb), point, avail_temps);
    TN_TN_PAIR_VECTOR copy_vector;
    TN_MAP callee_to_caller_map = TN_MAP_Create();
    OP *op;
    TN *caller_tn;
    FOR_ALL_BB_OPs_FWD(bb, op) {
      if (op == point) break;

      for (i = 0; i < OP_opnds(op); i++) {
	TN *tn = OP_opnd(op, i);
	if (TN_is_register(tn) && (TN_register(tn) != REGISTER_UNDEFINED) &&
	    ((caller_tn = (TN *)TN_MAP_Get(callee_to_caller_map, tn)) != NULL))
	  Set_OP_opnd(op, i, caller_tn);
      }

      for (i = 0; i < OP_results(op); i++) {
	TN *tn = OP_result(op, i);
	if (TN_is_register(tn) && TN_register(tn) != REGISTER_UNDEFINED) {
	  if ((caller_tn = (TN *)TN_MAP_Get(callee_to_caller_map, tn)) != NULL) 
	    Set_OP_result(op, i, caller_tn);
	  else {
	    ISA_REGISTER_CLASS rc = TN_register_class(tn);
	    REGISTER reg = TN_register(tn);
	    if (REGISTER_SET_MemberP(Callee_Saved_Regs_Mask[rc], reg)) {
	      REGISTER new_reg = REGISTER_SET_Choose(avail_temps[rc]);
	      FmtAssert(new_reg != REGISTER_UNDEFINED, ("Not enough caller save register for code before spadjust"));
	      avail_temps[rc] = REGISTER_SET_Difference1(avail_temps[rc],
							 new_reg);
	      TN *caller_tn = Build_Dedicated_TN(rc, new_reg, TN_size(tn));
	      copy_vector.push_back(TN_TN_PAIR(tn, caller_tn));
	      TN_MAP_Set(callee_to_caller_map, tn, caller_tn);
	      Set_OP_result(op, i, caller_tn);
	    }
	  }
	}
      } 
    }
    for (INT i = 0; i < copy_vector.size(); i++) {
      Exp_COPY(copy_vector[i].first, copy_vector[i].second, &ops);
    }
    TN_MAP_Delete(callee_to_caller_map);
  }

  if (Get_Trace (TP_CGEXP, 64)) {
    #pragma mips_frequency_hint NEVER
    OP *op;
    fprintf(TFile, "\nTarget fixed up entry code:\n");
    FOR_ALL_OPS_OPs_FWD(&ops, op) Print_OP_No_SrcLine(op);
  }

  if (point == NULL) {
    BB_Prepend_Ops (bb, &ops);
  } else {
    BB_Insert_Ops_After(bb, point, &ops);
  }

  // Must call mexpand in case of multi load/store
  Convert_BB_To_Multi_Ops(bb);

  return;
}

/* ====================================================================
 *   EETARG_Fixup_Exit_Code
 *
 *   Generate push/pop sequence.
 * ====================================================================
 */
void
EETARG_Fixup_Exit_Code (
  BB *bb
)
{
  /* check for need to generate restore code for callee-saved regs */

  if (callee_saved_vector.size() == 0) return;

  ANNOTATION *ant = ANNOT_Get(BB_annotations(bb), ANNOT_EXITINFO);
  EXITINFO *exit_info = ANNOT_exitinfo(ant);
  OP *point = EXITINFO_sp_adj(exit_info);
  if (point == NULL) {
    point = BB_last_op(bb);
  }
  FmtAssert(point != NULL,("Unexpected empty or non xfer op in bb exit"));
  
  INT i = 0;
  OPS ops = OPS_EMPTY;
  for (i = 0; i < callee_saved_vector.size(); i++) {
    TN_SYMBOL_PAIR pair = callee_saved_vector[i];
    TN *dst_tn = pair.first;
    ST *spill_loc = pair.second;
    CGTARG_Load_From_Memory(dst_tn, spill_loc, &ops);
    // Mark the actual load as a spill
    OP *op = OPS_last(&ops);
    while(!OP_load(op)) op = OP_prev(op);
    Set_OP_spill(op);
    Set_OP_spilled_tn (op, dst_tn);
  }
  

  // If in the epilogue we used FP to restore callee saved, we must ensure
  // that FP has not been restored to its callee saved value.
  // It may happen if in the epilogue, no use of FP appear except the ones
  // we generate. In this case we must find the restore operation of FP
  // and generate a temporary copy.
  OP *op;
  BOOL fp_used;
  FOR_ALL_OPS_OPs(&ops, op) {
    for (INT i = 0; i < OP_opnds(op); i++) {
      if (OP_opnd(op, i) == FP_TN) {
	fp_used = true;
      }
    }
  }
  if (fp_used) {
    OP *def_op = NULL;
    INT def_idx;
    TN *def_tn;
    FOR_ALL_BB_OPs_FWD(bb, op) {
      if (op == point) break; /* Stop at sp adjust. */
      for (i = 0; i < OP_results(op); i++) {
	TN *tn = OP_result(op, i);
	if (TN_is_register(tn) && TN_register(tn) != REGISTER_UNDEFINED) {
	  ISA_REGISTER_CLASS rc = TN_register_class(tn);
	  REGISTER reg = TN_register(tn);
	  if (TN_is_save_reg(tn) && 
	      TN_save_rclass(tn) == TN_register_class(FP_TN) &&
	      TN_save_reg(tn) == TN_register(FP_TN) &&
	      rc == TN_register_class(FP_TN) &&
	      reg == TN_register(FP_TN)
	      ) {
	    def_op = op;
	    def_idx = i;
	    def_tn = tn;
	  }
	}
      }
    }
    if (def_op != NULL) {
      REGISTER_SET avail_temps[ISA_REGISTER_CLASS_MAX+1];
      REG_LIVE_Epilog_Temps(Get_Current_PU_ST(), bb, def_op, avail_temps);
      ISA_REGISTER_CLASS rc = TN_register_class(def_tn);
      REGISTER caller_reg = REGISTER_SET_Choose(avail_temps[rc]);
      TN *caller_tn = Build_Dedicated_TN(rc, caller_reg, TN_size(def_tn));
      Set_OP_result(def_op, def_idx, caller_tn);
      if (point == NULL) {
	// If no sp adjust, simply add copy after callee restore
	Exp_COPY(def_tn, caller_tn, &ops);
      } else {
	// If there is an sp adjust op, we must insert after it
	// as it will used FP_TN
	OPS copy_ops = OPS_EMPTY;
	Exp_COPY(def_tn, caller_tn, &copy_ops);
	BB_Insert_Ops_After(bb, point, &copy_ops);
      }
    }
  }
  
  if (Get_Trace (TP_CGEXP, 64)) {
    #pragma mips_frequency_hint NEVER
    fprintf(TFile, "\nTarget fixed up exit code:\n");
    FOR_ALL_OPS_OPs_FWD(&ops, op) Print_OP_No_SrcLine(op);
  }

  BB_Insert_Ops_Before(bb, point, &ops);

  // Must call mexpand in case of multi load/store
  Convert_BB_To_Multi_Ops(bb);

  return;
}

/* =================================================================
 *   EETARG_Callee_Saved_Regs_Mask_Size
 *
 *   Calculate the size of the save area needed for the callee
 *   saved register mask.
 * =================================================================
 */
INT
EETARG_Callee_Saved_Regs_Mask_Size ()
{
  INT size = 0;

  return size;
}

/* ====================================================================
 *   EETARG_Init_Entry_Exit_Code ()
 * ====================================================================
 */
void
EETARG_Init_Entry_Exit_Code (
  WN *pu_wn, 
  BOOL need_frame_pointer
)
{
  return;
}

/* ====================================================================
 *   EETARG_Save_Pfs
 *
 *   nothing to do, no PFS
 * ====================================================================
 */
void
EETARG_Save_Pfs (TN *saved_pfs, OPS *ops)
{
  return;
}

/* ====================================================================
 *   EETARG_Restore_Pfs
 *
 *   nothing to do, no PFS
 * ====================================================================
 */
void
EETARG_Restore_Pfs (TN *saved_pfs, OPS *ops)
{
  return;
}

/* ====================================================================
 *   EETARG_Save_Extra_Callee_Tns
 *
 *   save predicates
 *   This is handled differently cause we need to
 *   save and restore the whole bank of predicates in one instruction.
 *   Note that we assume GRA will remove the predicate save/restore
 *   if no callee-save predicates are used.
 * ====================================================================
 */
void EETARG_Save_Extra_Callee_Tns (
  OPS *ops
)
{
  // 1. Save the guard registers:
#if 0
  TN *callee_tn = CALLEE_tn(Callee_Saved_Regs_Count);
  // save callee_tn in callee-saved-regs array;
  // this works cause originally allocated space for all regs,
  // yet only use space for callee-save (so available space).
  // could get broken if ever allocated minimal space originally.
  if (callee_tn == NULL) {
    callee_tn = Build_RCLASS_TN(ISA_REGISTER_CLASS_du);
    Set_TN_is_gra_cannot_split(callee_tn);
    CALLEE_tn(Callee_Saved_Regs_Count) = callee_tn;
  }
  Build_OP (TOP_cmove, callee_tn, True_TN, Pr_TN, ops);
  Set_OP_no_move_before_gra(OPS_last(ops));
#endif
}

/* ====================================================================
 *   EETARG_Restore_Extra_Callee_Tns
 * ====================================================================
 */
void EETARG_Restore_Extra_Callee_Tns (
  OPS *ops
)
{
#if 0
  // restore all predicates
  TN *callee_tn = CALLEE_tn(Callee_Saved_Regs_Count);
  Build_OP (TOP_cmove, Pr_TN, True_TN, callee_tn, ops);
  Set_OP_no_move_before_gra(OPS_last(ops));
#endif
}


/* ====================================================================
 * EETARG_Do_Not_Save_Callee_Reg_Class
 * ====================================================================
 */

BOOL EETARG_Do_Not_Save_Callee_Reg_Class ( ISA_REGISTER_CLASS cl ) {
  return FALSE;
}

/* ======================================================================
 * EETARG_get_temp_for_spadjust
 * Should returns a super scratch (not allocatable) or defined register.
 * This register will be used to initialize stack pointer in entry block.
 * ======================================================================
 */

TN *EETARG_get_temp_for_spadjust( BB *bb) {

  return (TN*)NULL;
}

/* ======================================================================
 * EETARG_Save_With_Regmask
 * Return TRUE if (cl,reg) should be saved using the regmask mechanism
 * rather than the save_reg mechanism.
 * 
 * ======================================================================
 */

BOOL EETARG_Save_With_Regmask (ISA_REGISTER_CLASS cl, REGISTER reg)
{
  // Never save return address register with regmask.
  if (cl == REGISTER_CLASS_ra && reg == REGISTER_ra)
    return FALSE;
  // Otherwise, we are controlled by the flag.
  return CG_gen_callee_saved_regs_mask;
}
