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
  default:
    FmtAssert(FALSE, ("don't know how to generate tail call for %s",
		     TOP_Name(call_top)));
    /*NOTREACHED*/
  }

  Is_True(OP_opnds(call_op) == 6, ("unexpected number of call opnds"));
  jump_op = Mk_OP(jump_top, OP_opnd(call_op, 0),
			    OP_opnd(call_op, 1),
			    OP_opnd(call_op, 2),
			    OP_opnd(call_op, 3),
			    OP_opnd(call_op, 4));
  Is_True(OP_opnds(jump_op) == 5, ("unexpected number of jump opnds"));
  return jump_op;
}

/* ====================================================================
 *   make_push_mask
 * ====================================================================
 */
static UINT32
make_push_mask ()
{
  INT callee_num;
  UINT8 l;
  UINT32 u20 = 0;
  BOOL found = FALSE;

  for (callee_num = 0; callee_num < Callee_Saved_Regs_Count; callee_num++) {
    TN *tn = CALLEE_tn(callee_num);
    ISA_REGISTER_CLASS cl = TN_save_rclass(tn);
    REGISTER i = TN_save_reg(tn);
    if (REGISTER_SET_MemberP(Callee_Saved_Regs_Mask[cl], i)) {
      REGISTER reg = i - REGISTER_MIN;

      found = TRUE;

      switch (cl) {
      case ISA_REGISTER_CLASS_au: /* Px */
	if (reg % 2 == 0) {
	  l =  (reg + 24) / 2;
	} else {
	  l =  (reg + 23) / 2;
	} 
	break;

      case ISA_REGISTER_CLASS_loop: /* LRx */
	if (reg == 0) {
          l = 9;
	} else if (reg == 1) {
          l = 10;
	} else if (reg == 2) {
          l = 11;
	} else {
          FmtAssert(FALSE,("EETARG_Fixup_Entry_Code: wrong loop register"));
	}
	break;

      case ISA_REGISTER_CLASS_guard: /* GFR */
	if (reg == 0 || reg == 1 || reg == 2 || reg == 3) {
	  l = 8;
	}
	else {
          FmtAssert(FALSE,("EETARG_Fixup_Entry_Code: wrong guard register"));
	}
	break;

      case ISA_REGISTER_CLASS_du: /* Rx */
	if (reg % 2 == 0) {
	  l = reg / 2;
	} else {
	  l = (reg - 1) / 2;
	} 
	break;

      case ISA_REGISTER_CLASS_control:
      default :
	FmtAssert(FALSE,("EETARG_Fixup_Entry_Code: wrong REGISTER_CLASS"));
      }

      /* set bit number l to 1 in u20 */
      u20 |= (1 << l);
    }
  }

  // RA_TN is handled in a strange way though ...
  if (RA_TN != NULL &&
      REGISTER_SET_MemberP(Callee_Saved_Regs_Mask[REGISTER_CLASS_ra], 
			   REGISTER_ra)) {
    l =  ((REGISTER_ra-REGISTER_MIN) + 23) / 2;
    u20 |= (1 << l);
    found = TRUE;
  }

  return u20;
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

  UINT32 u20 = 0;
  u20 = make_push_mask();
  if (u20 != 0) {
    // make a TN from the mask
    TN *pmask = Gen_Unique_Literal_TN(u20, 4);

    // create the push OP
    OPS ops = OPS_EMPTY;
    Build_OP(TOP_GP32_PUSH_U20, pmask, &ops);

    /* insert the ops in the op list for the current BB */
    BB_Prepend_Ops (bb, &ops);
  }

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

  INT callee_num;
  UINT8 l;
  UINT u20 = 0;
  BOOL found = FALSE;

  for (callee_num = 0; callee_num < Callee_Saved_Regs_Count; callee_num++) {
    TN *tn = CALLEE_tn(callee_num);
    ISA_REGISTER_CLASS cl = TN_save_rclass(tn);
    REGISTER i = TN_save_reg(tn);
    if (REGISTER_SET_MemberP(Callee_Saved_Regs_Mask[cl], i)) {
      REGISTER reg = i - REGISTER_MIN;

      found = TRUE;
      switch (cl) {
      case ISA_REGISTER_CLASS_au: /* Px */
	if (reg % 2 == 0) {
	  l =  (14 - reg) / 2;
	} else {
	  l =  (15 - reg) / 2;
	} 
	break;

      case ISA_REGISTER_CLASS_loop: /* LRx */
	if (reg == 0) {
          l = 10;
	} else if (reg == 1) {
          l = 9;
	} else if (reg == 2) {
          l = 8;
	} else {
	  FmtAssert(FALSE,("EETARG_Fixup_Exit_Code: wrong loop register"));
	}
	break;

      case ISA_REGISTER_CLASS_guard: /* GFR */
	if (reg == 0 || reg == 1 || reg == 2 || reg == 3) {
          l = 11;
	}
	else {
	  FmtAssert(FALSE,("EETARG_Fixup_Exit_Code: wrong guard register"));
	}
	break;

      case ISA_REGISTER_CLASS_du: /* Rx */
	if (reg % 2 == 0) {
	  l =  (38 - reg) / 2;
	} else {
	  l =  (39 - reg) / 2;
	} 
	break;

      default :
	FmtAssert(FALSE,("EETARG_Fixup_Exit_Code: wrong REGISTER CLASS"));
      }

      /* set bit number l to 1 in u20 */
      u20 |= (1 << l);
    }
  }

  // RA_TN is handled in a strange way though ...
  if (RA_TN != NULL &&
      REGISTER_SET_MemberP(Callee_Saved_Regs_Mask[REGISTER_CLASS_ra], 
			   REGISTER_ra)) {
    l =  (15 - (REGISTER_ra-REGISTER_MIN)) / 2;
    u20 |= (1 << l);
    found = TRUE;
  }

  if (found) {
    // make literal TN
    TN *pmask = Gen_Unique_Literal_TN(u20, 4);

    // make pop OP
    OPS ops = OPS_EMPTY;
    Build_OP(TOP_GP32_POP_U20, pmask, &ops);

    //    if (BB_call(bb)) {
    /* If it's a tail call block we insert the new operations in front
     * of the jump.
     */
    OP *point = BB_last_op(bb);
      //      Is_True(OP_br(point), ("last tail call OP of BB:%d not a jump", BB_id(bb)));
    BB_Insert_Ops_Before(bb, point, &ops);
      //    } else {
      /*
       * Insert before return
       */
      //      BB_Append_Ops(bb, &ops);
    //    }
  }

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
  INT i;
  INT size = 0;
  UINT32 u20 = 0;

  u20 = make_push_mask();
  // push/pop operand is a u20
  for (i = 0; i < 20; i++) {

    // check whether the bit is set
    if (!(u20 & (1 << i))) continue;

    // bit's set -- add this element's size
    if (i < 8) {
      // DU:
      size += 2*MTYPE_byte_size(MTYPE_I5);
    }
    else if (i < 9) {
      // GFR
      size += MTYPE_byte_size(MTYPE_I4);
    }
    else if (i < 12) {
      // loop
      FmtAssert(FALSE,("don't know how to push a loop register ?"));
    }
    else {
      // AU:
      size += 2*MTYPE_byte_size(MTYPE_A4);
    }
  }

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


