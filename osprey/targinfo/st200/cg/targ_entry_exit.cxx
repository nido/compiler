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
#include "exp_targ.h"
#include "cg_spill.h"
#include "cgtarget.h"
#include "mexpand.h"

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
  case TOP_call:
    jump_op = Mk_OP(TOP_goto, OP_opnd(call_op, 0));
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
    if (REGISTER_SET_MemberP(Callee_Saved_Regs_Mask[cl], reg)) 
      save_creg_vector.push_back(TN_save_creg(tn));
  }
  if (RA_TN != NULL &&
      REGISTER_SET_MemberP(Callee_Saved_Regs_Mask[REGISTER_CLASS_ra], 
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
	  callee_saved_vector.push_back(TN_SYMBOL_PAIR(Build_Dedicated_TN(CLASS_REG_PAIR_rclass(creg1), CLASS_REG_PAIR_reg(creg1), 8),st));
	  i++;
	  continue;
	}
    }
    TY_IDX ty = MTYPE_To_TY(MTYPE_I4);
    ST *st = CGSPILL_Gen_Spill_Symbol(ty, "callee_spill");
    callee_saved_vector.push_back(TN_SYMBOL_PAIR(Build_Dedicated_TN(CLASS_REG_PAIR_rclass(creg1), CLASS_REG_PAIR_reg(creg1), 4),st));
  }
}

void
EETARG_Fixup_Stack_Frame (void)
{
  if (!CG_gen_callee_saved_regs_mask) return;
  
  init_callee_saved_symbols();
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
  // if push mask is not required -- nada
  if (!CG_gen_callee_saved_regs_mask) return;

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
  if (!CG_gen_callee_saved_regs_mask) return;

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

  /* check for need to generate restore code for callee-saved regs */
  if (!CG_gen_callee_saved_regs_mask) return 0;
  
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


