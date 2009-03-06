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
 * Module: calls.cxx
 *
 * Revision history:
 *  04-Oct-91 - Original Version
 *  24-Oct-91 - Added support for entry/exit code.
 *  27-May-93 - Added support for PIC entry/exit code.
 *
 * Description:
 *
 * General support for call-related functionality in the back end.
 * Utilities for manipulating the call descriptors.
 *
 * ====================================================================
 * ====================================================================
 */

#include "defs.h"
#include "erglob.h"
#include "erbe.h"
#include "glob.h"
#include "tracing.h"
#ifdef TARG_ST 
#include "config_target.h"
#else
#include "config_targ.h"
#endif
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
#include "cg.h"
#include "calls.h"
#include "cgexp.h"
#include "reg_live.h"
#include "const.h"
#include "data_layout.h"
#include "whirl2ops.h"
#include "cg_spill.h"
#include "reg_live.h"
#include "lra.h"
#include "entry_exit_targ.h"

#include "targ_sim.h"
#include "cgtarget.h"


INT64 Frame_Len;

/* Callee-saved register <-> save symbol/TN map: */
SAVE_REG *Callee_Saved_Regs;
INT32 Callee_Saved_Regs_Count;

#ifdef TARG_ST
/* 
 * Regs that need to be saved at prolog and restored at epilog using
 * the regmask mechanism.
 */
REGISTER_SET Callee_Saved_Regs_Mask[ISA_REGISTER_CLASS_MAX_LIMIT+1];
#endif

/* Special PREGs associated with save locations for Callee Saved registers */
PREG_NUM *Callee_Saved_Pregs;
PREG_NUM Caller_FP_Preg;
PREG_NUM Caller_GP_Preg;
PREG_NUM Return_Preg;
PREG_NUM GP_Preg;
PREG_NUM Return_Int_Preg[2];
PREG_NUM Return_Float_Preg[2];

/* Return address register map: */
static SAVE_REG Return_Address_Reg_Map;
SAVE_REG *Return_Address_Reg = &Return_Address_Reg_Map;

/* Do we need to setup a new GP for this procedure? */
static enum {
  undefined_code,	/* not set yet */
  never_code,	        /* can access gp without setup */
  no_code,	        /* doesn't have a gp setup */
  need_code,	        /* needs a gp setup */
  final_code	        /* has final version of gp setup */
} GP_Setup_Code = undefined_code;

BOOL LC_Used_In_PU;

/* TNs to save the callers GP and FP if needed */
static TN *Caller_GP_TN;
static TN *Caller_FP_TN;
static TN *Caller_Pfs_TN;
static TN *ra_intsave_tn;

/* Keep track of a TN with the value of the current PU's stack pointer
 * adjustment (i.e. the frame length):
 */
static TN *Frame_Len_TN;
static TN *Neg_Frame_Len_TN;

static BOOL Gen_Frame_Pointer;

/* Trace flags: */
static BOOL Trace_EE = FALSE;	/* Trace entry/exit processing */

#if 1
/* [JV] When set, use this register for spadjust. */
/* This is the case when there is no more scratch register ...*/
/* It is possible to use callee save by using a push/pop like sequence
   before the spadjust. This does not work with varargs using va_list
   declared as void*.
*/
static TN *Use_Callee_Save_TN_For_SpAdjust = NULL;
#endif

/* macro to test if we will use a scratch register to hold gp for
 * the pu.  we do this in leaf routines if there are no regions
 * and gra will be run.
 */
#ifdef TARG_ST
#define Use_Scratch_GP(need_gp_setup) \
((need_gp_setup) && !PU_Has_Calls && !PU_has_region(Get_Current_PU()) \
 && !PU_Has_Asm)
#else
#define Use_Scratch_GP(need_gp_setup) \
((need_gp_setup) && !PU_Has_Calls && !PU_has_region(Get_Current_PU()))
#endif

/* ====================================================================
 *   Init_Pregs
 *
 *   Initialize various pregs for PU wide use.
 * ====================================================================
 */
static void
Init_Pregs ( void )
{
  INT i;
  
  Caller_FP_Preg = Create_Preg(TY_mtype(Spill_Ptr_Type ), "caller_FP");
  Caller_GP_Preg = Create_Preg(TY_mtype(Spill_Ptr_Type ), "caller_GP");

  /* Now initialize the return address map: */
  Return_Preg = Create_Preg(TY_mtype(Spill_Ptr_Type), "return");

  for ( i = 0; i < 2; i++ ) {
    Return_Int_Preg[i] = Create_Preg(TY_mtype(Spill_Int_Type), "return_int");
    Return_Float_Preg[i] = Create_Preg(TY_mtype(Spill_Float_Type), "return_float");
  }

  /* set up preg for gp tn */
  GP_Preg = Create_Preg(TY_mtype(Spill_Ptr_Type), "GP");
}

/* =======================================================================
 *   Setup_GP_TN_For_PU
 *
 *   set up GT_TN for the PU.  we may be able to place it in a caller saved
 *   register, and if so, allocate one for that purpose.  if not, use $gp.  
 * =======================================================================
 */
static void
Setup_GP_TN_For_PU ( 
  ST *pu
)
{
  REGISTER reg;

  if (GP_Setup_Code != undefined_code) {
	/* This can only happen if called more than once per PU,
	 * i.e. when regions.  In this case, keep the initial setting
	 * (so that all bbs are consistent), and then possibly adjust
	 * everything in the PU in Adjust_GP_Entry_Exit */
	;
  }
  else if (Force_GP_Prolog) {
    GP_Setup_Code = need_code;
  }
#ifdef TARG_ST
  else if (Gen_GP_Relative && !Is_Caller_Save_GP &&
#else
  else if (!Is_Caller_Save_GP &&
#endif
	(Gen_PIC_Call_Shared || Gen_PIC_Shared) &&
	ST_visible_outside_dso(pu) ) {

    /* if non-leaf that is visible outside dso, then must setup gp
     * because may call internal routine that uses gp. */
    if (PU_Has_Calls || PU_References_GP) {
      PU_References_GP = TRUE;
      GP_Setup_Code = need_code;
    }
    else {
      /* Don't setup GP unless sure that it is needed.
       * Adjust_GP_Entry_Exit can add gp setup/save/restore code
       * at end after we are sure that it is needed.
       * Otherwise we have to remove gp code, and that gets
       * complicated with regions, because it can also affect
       * the boundary sets. */
      GP_Setup_Code = no_code;
    }
  }
#ifdef TARG_ST
  else if (!Gen_GP_Relative && PU_References_GP && Is_Caller_Save_GP) {
    // [SC] TLS support
    // Even absolute code can reference the GP for thread support.
    GP_Setup_Code = need_code;
  }
#endif
  else {
    GP_Setup_Code = never_code;
  }

  /* initialize the gp map */
  TN_MAP_Set(TN_To_PREG_Map, GP_TN, (void *)GP_Preg);
  PREG_To_TN_Array[GP_Preg] = GP_TN;
  PREG_To_TN_Mtype[GP_Preg] = TY_mtype(Spill_Ptr_Type);

  /* we will put gp in a scratch register if this is a leaf routine
   * and gra is to be called (check for REGISTER_UNDEFINED protects
   * agains multiple entry PUs).  we also make gp available as a callee
   * saved register so that using the scratch register will not cause
   * any degradation due to register constraints.  we don't currently
   * have enough information to perform this optimization if regions
   * are present.
   */
  if (Use_Scratch_GP(GP_Setup_Code == need_code)) {
    REGISTER_SET caller_saves;
    REGISTER_SET func_val;
    REGISTER_SET func_arg;

    REGISTER_Set_Allocatable(REGISTER_CLASS_gp, REGISTER_gp, TRUE);

    /* exclude function return and argument registers from our choices */
    caller_saves = REGISTER_CLASS_caller_saves(REGISTER_CLASS_gp);
    func_val = REGISTER_CLASS_function_value(REGISTER_CLASS_gp);
    caller_saves = REGISTER_SET_Difference(caller_saves, func_val);
    func_arg = REGISTER_CLASS_function_argument(REGISTER_CLASS_gp);
    caller_saves = REGISTER_SET_Difference(caller_saves, func_arg);

    reg = REGISTER_SET_Choose(caller_saves);
    if ( reg == REGISTER_UNDEFINED ) {
      /* no caller saved register available for some reason.  this
       * should not happen, but we'll fail gracefully and just use
       * gp.
       */
      DevWarn("No caller saved register to replace $gp in leaf routine.\n");
      reg = REGISTER_gp;
    }
  }
  else {
    /* use gp */
    reg = REGISTER_gp;
  }
  REGISTER_Set_Allocatable(REGISTER_CLASS_gp, reg, FALSE);
  Set_TN_register(GP_TN, reg);
}

/* =======================================================================
 *   Init_Callee_Saved_Regs_for_REGION 
 *
 *   See interface description.
 * =======================================================================
 */
void
Init_Callee_Saved_Regs_for_REGION ( 
  ST *pu, 
  BOOL is_region 
)
{
  INT i;
  ISA_REGISTER_CLASS cl;
  TN *stn;

#ifdef TARG_ST
  // [SC] Allow GP to be used even in absolute code (some TLS models want it)
  // [VCdV] on xp70, GP is always activated.
#ifndef TARG_STxP70
  if (Gen_GP_Relative || PU_References_GP)
#endif
    Setup_GP_TN_For_PU( pu );
#else
  if (Gen_GP_Relative)
    Setup_GP_TN_For_PU( pu );
#endif

  if (NULL != RA_TN /* IA-32 doesn't have ra reg. */) {
    /* initialize the return address map: */
    if ( stn = PREG_To_TN_Array[ Return_Preg ] )
      SAVE_tn(Return_Address_Reg) = stn;
    else {
      SAVE_tn(Return_Address_Reg) = Build_TN_Like(RA_TN);
      Set_TN_save_creg (SAVE_tn(Return_Address_Reg), TN_class_reg(RA_TN));
      TN_MAP_Set( TN_To_PREG_Map, SAVE_tn(Return_Address_Reg),
		  (void *)Return_Preg );
      PREG_To_TN_Array[ Return_Preg ] = SAVE_tn(Return_Address_Reg);
      PREG_To_TN_Mtype[ Return_Preg ] = Spill_Ptr_Mtype;
    }
  }

  ra_intsave_tn = NULL;
  Caller_GP_TN = NULL;
  Caller_FP_TN	= NULL;
  Caller_Pfs_TN = NULL;
  if (Pfs_TN) {
    Caller_Pfs_TN = CGTARG_Gen_Pfs_TN ();
  }
 
  /* if called for a region, no need to set up TN's for callee saved
   * registers.  these will be handled in the PU.
   */
  if (is_region) {
    return;
  }

  /* allocate the callee-saved register map: */
  Callee_Saved_Regs = (SAVE_REG *)Pu_Alloc (  (ISA_REGISTER_CLASS_MAX + 1) 
					    * (REGISTER_MAX + 1)
					    * sizeof(SAVE_REG) );

  /* build and initialize the save tn's for the callee saved registers */
  Callee_Saved_Regs_Count = 0;
  i = 0;
  FOR_ALL_ISA_REGISTER_CLASS( cl ) {
    REGISTER reg;
    REGISTER_SET regset = REGISTER_CLASS_callee_saves(cl);

    if (REGISTER_CLASS_multiple_save(cl)) continue;

    if (EETARG_Do_Not_Save_Callee_Reg_Class(cl)) continue;

#ifdef TARG_ST
    if (PU_Has_EH_Return) {
      regset = REGISTER_SET_Union (regset,
				   REGISTER_CLASS_eh_return(cl));
    }
#endif
    for ( reg = REGISTER_SET_Choose(regset);
	  reg != REGISTER_UNDEFINED;
	  reg = REGISTER_SET_Choose_Next(regset, reg), ++i
    ) {
      TN *callee_tn;
      TN *ded_tn = Build_Dedicated_TN(cl, reg, 0);
      DevAssert(ded_tn, ("Missing dedicated TN for callee-saved register %s",
			REGISTER_name(cl, reg)));
      CALLEE_ded_tn(i) = ded_tn;
      callee_tn = Build_RCLASS_TN(cl);
      Set_TN_save_creg (callee_tn, TN_class_reg(ded_tn));
      CALLEE_tn(i) = callee_tn;
    }

  }

  Callee_Saved_Regs_Count = i;


  return;
}

/* ====================================================================
 *   Generate_Entry
 *
 *   Generate entry code (in SGIR form) in the given BB for the given
 *   entry symbol.  This currently includes the following:
 *
 *   1) Insert an update of the stack pointer.  If a frame pointer is
 *	required, first copy it from the stack pointer.  This also
 *	requires saving the caller's FP, since it's a callee-saved
 *	register, so we copy it to another temporary register.  (We
 *	can't just store it, since our stack frame hasn't been
 *	established yet.)
 *
 *   2)	Insert stores from dedicated parameter registers for all
 *	declared and possible undeclared formals for varargs routines.
 *
 *   3) If <gra_run>, then GRA will be run and each of the callee_saves
 *	registers is copied to its save TN.
 *
 * ====================================================================
 */
static void
Generate_Entry (BB *bb, BOOL gra_run)
{
  INT callee_num;
  TN *stn;
  OP *op;
  OPS ops = OPS_EMPTY;
  ANNOTATION *ant = ANNOT_Get (BB_annotations(bb), ANNOT_ENTRYINFO);
  ENTRYINFO *ent_info = ANNOT_entryinfo(ant);
  ST *st = ENTRYINFO_name(ent_info);	/* The entry's symtab entry */

  if ((BB_rid(bb) != NULL) && ( RID_level(BB_rid(bb)) >= RL_CGSCHED )) {
    /* don't change bb's which have already been through CG */
    return;
  }

  if (Trace_EE) {
    #pragma mips_frequency_hint NEVER
    fprintf (TFile,
	      "\n%s<calls> Generating entry for %s (BB:%d)\n",
	      DBar, ST_name(st), BB_id(bb) );
  }

  if (!BB_handler(bb)) {

    EETARG_Save_Pfs (Caller_Pfs_TN, &ops);	// alloc

    /* Initialize the stack pointer (this is a placeholder; Adjust_Entry
     * will replace it to the actual sequence once we know the size of
     * the frame):
     */

    Exp_Spadjust (SP_TN, Neg_Frame_Len_TN, V_NONE, &ops);
#ifdef TARG_ST
    // FdF 20081211: To be consistent with what is restored in
    // Generate_Exits, this code must not be dependent on
    // BB_handler(bb)
  }
#endif
    /* Initialize the frame pointer if required: */
    if (Gen_Frame_Pointer && !PUSH_FRAME_POINTER_ON_STACK) {
      // check if fp is callee reg
      // (check abi_property rather than register set
      //  because register_set unsets fp as callee when already allocated).
      if (Is_Callee_Register(FP_TN)) {
	// save old fp
	if (Caller_FP_TN == NULL) {
	  /* Build a temp symbol for caller's FP if needed: */
	  if (stn = PREG_To_TN_Array[Caller_FP_Preg])
	    Caller_FP_TN = stn;
	  else {
	    Caller_FP_TN = CGTARG_Gen_FP_TN ();
	    Set_TN_save_creg (Caller_FP_TN, TN_class_reg(FP_TN));
	    TN_MAP_Set(TN_To_PREG_Map, Caller_FP_TN, 
				             (void *)Caller_FP_Preg);
	    PREG_To_TN_Array[Caller_FP_Preg] = Caller_FP_TN;
	    PREG_To_TN_Mtype[Caller_FP_Preg] = TY_mtype(Spill_Ptr_Type);
	  }
	}
	/* Save the caller's FP in a temporary: */
	Exp_COPY (Caller_FP_TN, FP_TN, &ops);
       }
#ifdef TARG_ST
    }
  if (!BB_handler(bb)) {
    if (Gen_Frame_Pointer && !PUSH_FRAME_POINTER_ON_STACK) {
#endif

      /* Now recover the new FP from the new SP: */
      Exp_Spadjust (FP_TN, Frame_Len_TN, V_NONE, &ops);
    }
    ENTRYINFO_sp_adj(ent_info) = OPS_last(&ops);
  }

  if (gra_run) {
    /* Copy from the callee saves registers to register TNs */
    for (callee_num = 0; 
	 callee_num < Callee_Saved_Regs_Count; 
	 ++callee_num ) {
      TN *callee_tn = CALLEE_tn(callee_num);
      if (TN_is_save_reg(callee_tn) &&
	  !REGISTER_CLASS_multiple_save(TN_register_class(callee_tn))
	  && ! EETARG_Save_With_Regmask (TN_register_class(callee_tn),
					  TN_register(callee_tn))) {
	Exp_COPY ( callee_tn, CALLEE_ded_tn(callee_num), &ops );
	Set_OP_no_move_before_gra(OPS_last(&ops));
      }
    }
  }

  /* If the return address builtin is required, save RA_TN to the 
   * memory location for __return_address. Otherwise, we copy RA_TN
   * to the save-tn for it. 
   */
  if (NULL != RA_TN) {
    if ( PU_has_return_address(Get_Current_PU()) ) {
      // This is broken for IA-32.  On IA-32, the return address is always
      // at a constant offset from the frame pointer, specifically it is
      // accessible as 4(%ebp), but it is never in a register.  Nor
      // does it need to be saved.
      ST *ra_sv_sym = Find_Special_Return_Address_Symbol();
      TN *ra_sv_tn = Build_TN_Like(RA_TN);
      Set_TN_save_creg (ra_sv_tn, TN_class_reg(RA_TN));
      Set_TN_spill(ra_sv_tn, ra_sv_sym);
      Exp_COPY (ra_sv_tn, RA_TN, &ops);
      if (MTYPE_byte_size(Pointer_Mtype) < MTYPE_byte_size(Spill_Int_Mtype) ) {
	/* In n32 the __return_address is 4 bytes (pointer),
	 * but we need 8-byte save/restore to make kernel and dbx happy.
	 * So use dummy 8-byte base that was created. */
	ra_sv_sym = ST_base(ra_sv_sym);		/* use 8-byte block */
	Set_TN_spill(ra_sv_tn, ra_sv_sym);	/* so dwarf uses new addr */
      }
      CGSPILL_Store_To_Memory (ra_sv_tn, ra_sv_sym, &ops, CGSPILL_LCL, bb);
    }
    else {
#ifdef TARG_ST
      /*
       * RA_TN must be saved as a callee saved TN whatever is actual classification.
       */
      if (! EETARG_Save_With_Regmask (REGISTER_CLASS_ra, REGISTER_ra)) {
        Exp_COPY (SAVE_tn(Return_Address_Reg), RA_TN, &ops);
        Set_OP_no_move_before_gra(OPS_last(&ops));
      }

#else /* !TARG_ST */

      if (gra_run && PU_Has_Calls 
	&& TN_register_class(RA_TN) != ISA_REGISTER_CLASS_integer)
      {
	// because has calls, gra will need to spill this.
	// but if it is not already in an integer reg,
	// then gra will spill to memory whereas for ia64
	// it could use a stacked reg; ideally gra would handle
	// this, but it doesn't and is easy to just copy to int reg
	// by hand and then let gra use stacked reg.
	if (ra_intsave_tn == NULL) {
        	ra_intsave_tn = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer);
		Set_TN_save_creg (ra_intsave_tn, TN_class_reg(RA_TN));
	}
	Exp_COPY (ra_intsave_tn, RA_TN, &ops );
      } else {
        Exp_COPY (SAVE_tn(Return_Address_Reg), RA_TN, &ops );
      }
      Set_OP_no_move_before_gra(OPS_last(&ops));

#endif /* TARG_ST */
    }
  }

  if ( gra_run ) 
    EETARG_Save_Extra_Callee_Tns (&ops);

  /* Save the old GP and setup a new GP if required */
  if (GP_Setup_Code == need_code) {

    /* we will put the gp in any available register if a leaf routine
     * to avoid the save/restore of the gp.  we will also make gp
     * available as a callee saved register so that in the worst case,
     * we will not generate worse code by using another register for
     * gp.
     */
    if (!Use_Scratch_GP(TRUE)) {
      if (Caller_GP_TN == NULL) {
	if (stn = PREG_To_TN_Array[Caller_GP_Preg])
	  Caller_GP_TN = stn;
	else {
	  Caller_GP_TN = CGTARG_Gen_GP_TN ();
	  Set_TN_save_creg (Caller_GP_TN, TN_class_reg(GP_TN));
	  TN_MAP_Set(TN_To_PREG_Map, Caller_GP_TN, (void *)Caller_GP_Preg);
	  PREG_To_TN_Array[Caller_GP_Preg] = Caller_GP_TN;
	  PREG_To_TN_Mtype[Caller_GP_Preg] = TY_mtype(Spill_Ptr_Type);
	}
      }
      Exp_COPY (Caller_GP_TN, GP_TN, &ops);
    }

    // is possible for altentry to be marked as not_used
    // but we have gp_prolog for whole PU.  In that case,
    // don't generate prolog (that references altentry ST).
    if (ST_is_not_used(st))
	;
    else if (Gen_PIC_Call_Shared && CGEXP_gp_prolog_call_shared) {
      /* pv 466125 */
      /* we actually ignore the st in this case, but pass it anyways
       * so that tn structure is regular. 
       */
      TN *gp_value_tn = CGTARG_Gen_GP_Value_TN(st,&ops);
      //Exp_Immediate (GP_TN, gp_value_tn, Pointer_Mtype, &ops);
      /*	Exp_OP1 (OPC_I4INTCONST, GP_TN, gp_value_tn, &ops); */
      Exp_Immediate (GP_TN, gp_value_tn, MTYPE_signed(Pointer_Mtype) ? TRUE : FALSE, &ops);
    }
    else if (Ep_TN != NULL) {
      /* Create a symbolic expression for ep-gp */
      TN *cur_pu_got_disp_tn = CGTARG_Gen_Got_Disp_TN (st,&ops);
      TN *got_disp_tn = Build_TN_Like(cur_pu_got_disp_tn);
      Exp_Immediate (got_disp_tn, cur_pu_got_disp_tn, MTYPE_signed(Pointer_Mtype) ? TRUE : FALSE, &ops);

      /* Add it to ep to get the new GP: */
      Exp_ADD (Pointer_Mtype, GP_TN, Ep_TN, got_disp_tn, &ops);
    }
    else
      Exp_GP_Init (GP_TN, st, &ops);
  } 
#ifdef TARG_ST
  else if (Gen_GP_Relative && 
	   Is_Caller_Save_GP && 
	   PU_Has_Calls && 
	   !Constant_GP
#else
  else if (Is_Caller_Save_GP && PU_Has_Calls && !Constant_GP
#endif
	&& PREG_To_TN_Array[Caller_GP_Preg] != NULL) 
  {
	// need to save old gp but don't need to setup new gp.
	// caller_gp_tn should already be created by call code.
      	Exp_COPY (PREG_To_TN_Array[Caller_GP_Preg], GP_TN, &ops);
  }

  /* set the srcpos field for all the entry OPs */
  FOR_ALL_OPS_OPs(&ops, op)
    OP_srcpos(op) = ENTRYINFO_srcpos(ent_info);

#ifdef TARG_ST
  /* [CL] set the prologue field for all the entry OPs */
  FOR_ALL_OPS_OPs(&ops, op) {
    Set_OP_prologue(op);
  }
#endif

  /* If we're tracing, print the new stuff before merging it: */
  if ( Trace_EE ) {
    #pragma mips_frequency_hint NEVER
    Print_OPS (&ops);
  }

  /* Merge the new operations into the beginning of the entry BB: */
  BB_Prepend_Ops(bb, &ops);
}

/* ====================================================================
 *   Can_Be_Tail_Call
 *
 *   Determine if the specified exit block <exit_bb> can be converted
 *   to a tail call. If so, the corresponding call BB is returned;
 *   otherwise, NULL is returned.
 * ====================================================================
 */
static BB *
Can_Be_Tail_Call (
  ST *pu_st, 
  BB *exit_bb
)
{
  OP *op;
  hTN_MAP fvals;
  PLOC ploc;
  TY_IDX func_type;
  ST *st;
  ANNOTATION *ant;
  CALLINFO *call_info;
  ST *call_st;
  WN *call_wn;
  BB *pred;

  /* The exit block can have only one pred, and it must be a call block.
   */
  pred = BB_Unique_Predecessor(exit_bb);
#ifdef TARG_ST
  // FdF 20051010, ddts 23277: In case of a "noreturn" call, the exit
  // block is also the call block.
  if (BB_call(exit_bb)) {
    Is_True(WN_Call_Never_Return(CALLINFO_call_wn(ANNOT_callinfo(ANNOT_Get (BB_annotations(exit_bb), ANNOT_CALLINFO) ))),
	    ("Only 'noreturn' call can be both CALL and EXIT before tail call optimization"));
    pred = exit_bb;
    exit_bb = NULL;
  }
#endif
  if (!pred || !BB_call(pred)) return NULL;

  /* Get some info about the call and the callee.
   */
  ant = ANNOT_Get(BB_annotations(pred), ANNOT_CALLINFO);
  call_info = ANNOT_callinfo(ant);
  call_st = CALLINFO_call_st(call_info);
  call_wn = CALLINFO_call_wn(call_info);
#ifdef TARG_ST
  OP *call_op = BB_last_op(pred);
  if (!OP_Can_Be_Tail_Call(call_op))
    return NULL;
#endif

  /* Assume a call sequence A->B->C (i.e. A calls B which calls C)
   * We would like to change the call B->C to be a tail-call.
   *
   * If C does not setup a new gp (static/internal), it can be a tail-call
   * only if B also does not setup a new GP. Otherwise, C would get A's
   * GP which might be wrong.
   *
   * One complication is that there is another scenario where C will
   * use the caller's GP: if C is preemptible, then its RLD stub
   * uses GP as part of its interface. In these cases we [later] change
   * the relocation for the symbol so it is treated as a data reference,
   * and no stub is generated.
   */
  if (   call_st && !Is_Caller_Save_GP
      && (Gen_PIC_Call_Shared || Gen_PIC_Shared)
      && ST_visible_outside_dso(pu_st))
  {
    if (!ST_visible_outside_dso(call_st)) return NULL;

    if (ST_is_preemptible(call_st)) {
      // [SC] Original open64 has code here to cope with some cases
      // of preemptible calls.
      return NULL;
    }
  }

  /* If any stack variables have had their address taken, it is
   * possible they might be used by the called PU, but if we do the
   * tail call conversion, the stack frame will have been removed.
   */
  if (!BE_ST_pu_has_valid_addr_flags(Get_Current_PU_ST())) return NULL;
  INT i;
  FOREACH_SYMBOL (CURRENT_SYMTAB, st, i) {     /* all local symbols in Current_Symtab */
    if (ST_class(st) != CLASS_VAR) continue;

    // [VL] We limit the constraint on usage of local variable address to
    // the case where it is not declared static (defined or not). This over-
    // comes one of the restrictions raised in art #35008
    if ((ST_sclass(st)!=SCLASS_DGLOBAL && ST_sclass(st)!=SCLASS_PSTATIC) &&
        (BE_ST_addr_used_locally(st) || BE_ST_addr_passed(st))) return NULL;
  }

  /* Make sure we don't use the stack to pass arguments to the called PU.
   * Defs have parm info in TY, but calls without prototypes do not,
   * so use whirl call node in that case.
   */
  func_type = call_st ? ST_pu_type(call_st) : WN_ty(call_wn);
#ifdef TARG_ST
  // [CG] We must use the First_Output_PLOC_Reg/Next_Output_PLOC_REG interface
  // to take into account strutures that overlaps the stack
  ploc = Setup_Output_Parameter_Locations(func_type);
  if (call_wn == NULL) {
    TYLIST_IDX tl;
    for (tl = TY_parms(func_type); tl != (TYLIST_IDX) NULL; tl = TYLIST_next(tl)) {
      TY_IDX ty = TYLIST_item(tl);
      ploc = Get_Output_Parameter_Location(ty);
      ploc = First_Output_PLOC_Reg (ploc, ty);
      while(PLOC_is_nonempty(ploc)) {
	if (PLOC_on_stack(ploc)) return NULL;
	ploc = Next_Output_PLOC_Reg ();
      }
    }
  } else {
    INT i;
    INT num_parms = WN_num_actuals(call_wn);
    for (i = 0; i < num_parms; i++) {
      TY_IDX ty = TY_Of_Parameter(WN_actual(call_wn,i));
      ploc = Get_Output_Parameter_Location (ty);
      ploc = First_Output_PLOC_Reg (ploc, ty);
      while(PLOC_is_nonempty(ploc)) {
	if (PLOC_on_stack(ploc)) return NULL;
	ploc = Next_Output_PLOC_Reg ();
      }
    }
  }
#else
  ploc = Setup_Output_Parameter_Locations(func_type);
  if (call_wn == NULL) {
    TYLIST_IDX tl;
    for (tl = TY_parms(func_type); tl != (TYLIST_IDX) NULL; tl = TYLIST_next(tl)) {
      ploc = Get_Output_Parameter_Location(TYLIST_item(tl));
      if (PLOC_on_stack(ploc)) return NULL;
    }
  } else {
    INT i;
    INT num_parms = WN_num_actuals(call_wn);
    for (i = 0; i < num_parms; i++) {
      ploc = Get_Output_Parameter_Location (TY_Of_Parameter(WN_actual(call_wn,i)));
      if (PLOC_on_stack(ploc)) return NULL;
    }
  }
#endif
  /* We need to make sure that the function values for the current
   * PU are the same or a subset of the function values for the
   * called PU. We accomplish this by examining uses and defs
   * of the function value TNs. We only allow copies were the
   * ultimate source and destination TN are the same function value
   * TN (the copy may be through an interim local TN). If a
   * function value TN is used in any other way, we reject this case.
   */
  MEM_POOL_Push(&MEM_local_pool);
  fvals = hTN_MAP_Create(&MEM_local_pool);

#ifdef TARG_ST
  // FdF 20051010, ddts23277: In case of a "noreturn" call, there is no exit_bb.
  if (exit_bb)
#endif

  FOR_ALL_BB_OPs_FWD(exit_bb, op) {
    // [SC] 20070201: codex bug #23066.  Loop body rewritten to handle
    // compose and extract ops.
    if (OP_copy(op) || OP_widemove(op)) {
      TN *src = OP_opnd(op, OP_copy(op) ? OP_Copy_Operand(op) : 0);
      TN *dst = OP_result(op, OP_copy(op) ? OP_Copy_Result(op) : 0);

      if (! Is_Function_Value(src)) {
	src = (TN *) hTN_MAP_Get(fvals, src);
      }
      // Here src is either a function value TN, or NULL.
      if (!src) goto failure;
      if (Is_Function_Value(dst)) {
	if (TN_register_and_class(src) != TN_register_and_class(dst))
	  goto failure;
      } else {
	hTN_MAP_Set(fvals, dst, src);
      }
    }
    else if (OP_compose(op)) {
      TN *src[ISA_OPERAND_max_operands];
      BOOL all_src_fvals = TRUE;
      INT opnd;
      for (opnd = 0; opnd < OP_opnds(op); opnd++) {
	src[opnd] = OP_opnd(op,opnd);
	if (! Is_Function_Value(src[opnd])) {
	  src[opnd] = (TN *) hTN_MAP_Get(fvals, src[opnd]);
	}
      }
      // All sources must map to consecutive registers
      if (! src[0]) goto failure;
      REGISTER next_reg = TN_register(src[0]);
      for (opnd = 0; opnd < OP_opnds(op); opnd++) {
	if (!src[opnd]
	    || TN_register(src[opnd]) != next_reg) {
	  goto failure;
	}
	next_reg += TN_nhardregs(src[opnd]);
      }
      TN *dst = OP_result(op,0);
      if (Is_Function_Value(dst)) {
	if (TN_register_and_class(src[0]) != TN_register_and_class(dst))
	  goto failure;
      } else {
	hTN_MAP_Set(fvals, dst, src[0]);
      }
    } else if (OP_extract(op)) {
      TN *dst[ISA_OPERAND_max_results];
      TN *src = OP_opnd(op, 0);
      if (! Is_Function_Value(src)) {
	src = (TN *) hTN_MAP_Get(fvals, src);
      }
      if (!src) goto failure;
      REGISTER next_reg = TN_register(src);
      for (INT res = 0; res < OP_results(op); res++) {
	TN *dst = OP_result(op,res);
	if (Is_Function_Value(dst)) {
	  if (TN_register(dst) != next_reg) {
	    goto failure;
	  }
	} else {
	  if (TN_register(dst) != REGISTER_UNDEFINED)
	    goto failure;
	  // Fabricate a TN to represent this result of the extract.
	  hTN_MAP_Set(fvals, dst, Build_Dedicated_TN (TN_register_class (dst),
						      next_reg,
						      TN_size (dst)));
	}
	next_reg += TN_nhardregs (dst);
      }
    } else
      goto failure;
  }
  MEM_POOL_Pop(&MEM_local_pool);

  return pred;

 failure:
  MEM_POOL_Pop(&MEM_local_pool);
  return NULL;
}

/* ====================================================================
 *   Can_Do_Tail_Calls_For_PU
 *
 *   Determine if there are some conditions for the whole PU that
 *   would disallow tail-call optimization. This acts as a first
 *   level filter before we analyze each exit block to see if there
 *   is a potential tail-call there.
 * ====================================================================
 */
static BOOL 
Can_Do_Tail_Calls_For_PU ()
{
  // If the PU performs dynamic stack allocations then it could somehow
  // pass a pointer to the stack to the tail-called routine.
  if (Current_PU_Stack_Model == SMODEL_DYNAMIC) return FALSE;

  // If PU has regions, cannot do tail-call optimization.
  if (PU_has_region(Get_Current_PU())) return FALSE;

  // If the PU calls a routine with the the following pragma:
  //	#pragma unknown_control_flow (func)
  // then we cannot do tail-call optimization for it.
  if (PU_has_unknown_control_flow (Get_Current_PU())) return FALSE;

#ifdef TARG_ST
  // If PU is an interrupt handler, cannot do tail-call opt
  if (PU_is_interrupt(Get_Current_PU())
      || PU_is_interrupt_nostkaln(Get_Current_PU())) return FALSE;
  // If PU is an exception handler, avoid tail-call opt
  if (PU_Has_EH_Return) return FALSE;
#endif

#ifdef TARG_STxP70
  // If farcall option, cannot so tail-call opt
  if (farcall)
    return FALSE;
#endif

  // If a PU has a call to setjmp and some other tail call from this
  // PU ultimately has a call to longjmp, then the stack frame will be
  // gone when we get back to the code following the setjmp. Therefore,
  // disable tail call opt for any PU with a setjmp. Also note that
  // setjmp can come in several variants (setjmp, _setjmp, sigsetjmp, etc)
  // so just look for any symbol ending in "setjmp" -- this may be
  // over cautious, but it's better to err on the safe side.
  for (BB *bb = REGION_First_BB; bb; bb = BB_next(bb)) {
    if (BB_call(bb)) {
      const char *name;
      INT len;
      ANNOTATION *callant = ANNOT_Get(BB_annotations(bb), ANNOT_CALLINFO);
      CALLINFO *callinfo = ANNOT_callinfo(callant);
      ST *st = CALLINFO_call_st(callinfo);
      if (st == NULL) continue;
      name = ST_name(st);
      len = strlen(name);
      if (len >= 6 && strcmp(name + len - 6, "setjmp") == 0) return FALSE;
      if (len >= 10 && strcmp(name+len-10, "getcontext") == 0) return FALSE;
    }
  }
  return TRUE;
}

/* ====================================================================
 *   Optimize_Tail_Calls
 *
 *   Optimize tail calls for a PU by converting the sequence of a call
 *   immediately followed by a return into a jump.
 *
 *   Note that since we may reset PU_Has_Calls, it requires that
 *   the tail call optimization occur before the normal entry/exit
 *   processing because stack allocation and GP logic is intertwinned
 *   with it.
 * ====================================================================
 */
void
Optimize_Tail_Calls (
  ST *pu
)
{
  BB_LIST *elist;
  BOOL have_tail_call = FALSE;

  // Don't optimize if disabled, -O0 or cannot do tail-calls for PU.
  if (!CG_tail_call
      || CG_opt_level == 0 
      || !Can_Do_Tail_Calls_For_PU ()) return;

  /* Check each of the exit blocks
   */
  for ( elist = Exit_BB_Head; elist; elist = BB_LIST_rest(elist) ) {
    BB *call_bb;
    BB *exit_bb = BB_LIST_first(elist);

    /* If we have a tail call we can optimize, then do so.
     */
    if (call_bb = Can_Be_Tail_Call(pu, exit_bb)) {
      OP *jmp_op;
      OP *call_op = BB_last_op(call_bb);
      Is_True(OP_call(call_op), ("call block didn't end in a call inst (%s)",TOP_Name(OP_code(call_op))));

      if (Trace_EE) {
	#pragma mips_frequency_hint NEVER
	fprintf(TFile, "%s<calls> call block before tail call optimization:\n",
			DBar);
	Print_BB(call_bb);
	fprintf(TFile, "\n<calls> exit block before tail call optimization:\n");
	Print_BB(exit_bb);
      }

      /* Replace the call OP with a jump.
       */
      jmp_op = EETARG_Build_Jump_Instead_Of_Call (call_op);
      Set_OP_tail_call(jmp_op);
      BB_Insert_Op_Before(call_bb, call_op, jmp_op);
      BB_Remove_Op(call_bb, call_op);

      /* Transfer the exit info from the exit block to the call block.
       * The call block becomes the new exit block. The exit block
       * is removed from the succ chain and will be removed by cflow later.
       */
#ifdef TARG_ST
      // FdF 20051010, ddts 23277: In case of a "noreturn" call, the
      // CALL and the EXIT bb are the same
      if (call_bb != exit_bb) {
#endif
      BB_Transfer_Exitinfo(exit_bb, call_bb);
      Unlink_Pred_Succ(call_bb, exit_bb);
      Exit_BB_Head = BB_LIST_Delete(exit_bb, Exit_BB_Head);
      Exit_BB_Head = BB_LIST_Push(call_bb, Exit_BB_Head, &MEM_pu_pool);
      Remove_BB(exit_bb);
#ifdef TARG_ST
      }
#endif

      if (Trace_EE) {
	#pragma mips_frequency_hint NEVER
	fprintf(TFile, "\n<calls> exit block after tail call optimization:\n");
	Print_BB(call_bb);
      }

      have_tail_call = TRUE;
    }
  }

  /* If we optimized at least one tail call, then we might have
   * removed the last "normal" call, in which case we might not
   * need a stack. Reseting PU_Has_Calls will make the right
   * things happen.
   */
  if (have_tail_call) {
    BB *bb;
    for (bb = REGION_First_BB; bb; bb = BB_next(bb)) {
      if (BB_call(bb) && !BB_exit(bb)) goto done;
    }
    PU_Has_Calls = FALSE;
  done:
    ;
  }
}

/* ====================================================================
 *   Target_Unique_Exit
 *
 *   'bb' is an exit block. Modify it so that it jumps to the unique
 *   return block. This would be simple except that the dedicated return
 *   value TNs cannot be live out of this block.
 * ====================================================================
 */
static void
Target_Unique_Exit (
  BB *bb,
  BB *unique_exit_bb,
#ifdef TARG_ST
  TN *rtn_tns[ISA_REGISTER_CLASS_MAX_LIMIT+1][REGISTER_MAX+1]
#else
  TN *rtn_tns[ISA_REGISTER_CLASS_MAX+1][REGISTER_MAX+1]
#endif
)
{
  OP *op;

  /* Scan the OPs (in reverse order) in the exit block and handle any 
   * reads of or writes to a function value TN.
   */
  FOR_ALL_BB_OPs_REV(bb, op) {
    INT i;

    /* Handle writes to a function value TN.
     */
    for (i = OP_results(op) - 1; i >= 0; --i) {
      TN *tn = OP_result(op,i);
      if ( Is_Function_Value(tn) ) {
	TN *new_tn;
	REGISTER reg = TN_register(tn);
	ISA_REGISTER_CLASS rc = TN_register_class(tn);

	/* We have a write to a function value TN. These are special
	 * and can't be live out of this block, so we must make a copy.
	 * Get the TN for the copy.
	 */
	new_tn = rtn_tns[rc][reg];
	if ( new_tn == NULL ) {
	  new_tn = Dup_TN_Even_If_Dedicated(tn);
	  if (TN_is_float(tn)) {
	    INT tn_size = OP_result_size(op,i) / 8;
	    Set_TN_size(new_tn, tn_size);
	  }
	}

	/* Adjust the result TN of the original function value write.
	 */
	Set_OP_result(op, i, new_tn);

	/* If this is the first time we've encountered this function
	 * value TN then make sure it's set, from the copy,
	 * in the new unique return block.
	 */
	if ( rtn_tns[rc][reg] == NULL ) {
	  ANNOTATION *ant = ANNOT_Get (BB_annotations(unique_exit_bb),
				       ANNOT_EXITINFO);
	  EXITINFO *exit_info = ANNOT_exitinfo(ant);
	  OPS ops = OPS_EMPTY;
	  rtn_tns[rc][reg] = new_tn;
	  Exp_COPY(tn, new_tn, &ops);
	  OP_srcpos(OPS_last(&ops)) = EXITINFO_srcpos(exit_info);
	  BB_Prepend_Ops(unique_exit_bb, &ops);
	}
      }
    }

    /* A select or unaligned load may have a use of func value TN.
     */
#ifdef TARG_ST
    // Arthur: the same_res info is part of OPERAND_INFO now:
    BOOL same_res = FALSE;
    for (i = 0; i < OP_results(op); i++) {
      if (OP_same_res(op, i) >= 0) {
	same_res = TRUE;
	break;
      }
    }

    if (same_res) {
#else
    if ( OP_same_res(op) ) {
#endif
      INT i;
      for ( i = 0; i < OP_opnds(op); ++i ) {
	TN *tn = OP_opnd(op,i);
	if ( Is_Function_Value(tn) ) {
	  REGISTER reg = TN_register(tn);
	  ISA_REGISTER_CLASS rc = TN_register_class(tn);
	  TN *new_tn = rtn_tns[rc][reg];
	  FmtAssert(new_tn, ("use before def of return value TN in BB:%d",
			    BB_id(bb)));
	  Set_OP_opnd(op, i, new_tn);
	}
      }
    }
  }

  /* Make sure this exit gets to the unique exit.
   */
  if ( BB_next(bb) == unique_exit_bb ) {
    Target_Simple_Fall_Through_BB(bb, unique_exit_bb);
  } else {
    Add_Goto(bb, unique_exit_bb);
  }
}

/* ====================================================================
 *   Generate_Unique_Exit
 *
 *   Generate a unique exit and re-target all existing exits to it.
 * ====================================================================
 */
static void
Generate_Unique_Exit (void)
{
  BB_LIST *elist;
  BB *unique_exit_bb;
  INT exits;

  /* Keep track of things we need for creating a unique exit from a PU.
   * We track the return value TNs as well as the BB containing the
   * real exit.
   */
  TN *rtn_tns[ISA_REGISTER_CLASS_MAX_LIMIT+1][REGISTER_MAX+1];

  /* Should we generate a unique exit?
   */
  if (!CG_unique_exit
      || PU_has_region(Get_Current_PU())) /* until we're sure it works with
					  * regions.
					  */
  {
    return;
  }

  /* If there is less than 2 simple (not tail-call) exits, then this is
   * a pointless exercise.
   */
  exits = 0;
  for ( elist = Exit_BB_Head; elist; elist = BB_LIST_rest(elist) ) {
    BB *bb = BB_LIST_first(elist);
    exits += !BB_call(bb);
  }
  if (exits < 2) return;

  /* Generate the unique exits.
   */
  BZERO(rtn_tns, sizeof(rtn_tns));
  unique_exit_bb = NULL;
  for ( elist = Exit_BB_Head; elist; elist = BB_LIST_rest(elist) ) {
    BB *bb = BB_LIST_first(elist);

    /* Exclude tail call blocks
     */
    if (BB_call(bb)) continue;

#ifdef TARG_ST
    // [SC] Exclude EH_return exits.
    if (EXITINFO_is_eh_return(ANNOT_exitinfo(ANNOT_Get(BB_annotations(bb), ANNOT_EXITINFO))))
      continue;
#endif
    /* This block will no longer be a exit block. So for the first
     * block, transfer the exitinfo to the unique exit block. For
     * the others, just remove the exitinfo.
     */
    if (unique_exit_bb == NULL) {
      unique_exit_bb = Gen_And_Insert_BB_After(bb);
      BB_rid(unique_exit_bb) = BB_rid(bb);
      BB_Transfer_Exitinfo(bb, unique_exit_bb);
      Exit_BB_Head = BB_LIST_Push(unique_exit_bb, Exit_BB_Head, &MEM_pu_pool);
    } else {
      ANNOTATION *ant = ANNOT_Get(BB_annotations(bb), ANNOT_EXITINFO);
      BB_annotations(bb) = ANNOT_Unlink(BB_annotations(bb), ant);
      Reset_BB_exit(bb);
    }
    Exit_BB_Head = BB_LIST_Delete(bb, Exit_BB_Head);

    /* Target the unique exit block.
     */
    Target_Unique_Exit(bb, unique_exit_bb, rtn_tns);
  }
}

/* ====================================================================
 *   Generate_Exit
 *
 *   Generate exit code (in SGIR form) in the given BB for the given
 *   subprogram.  This currently includes the following:
 *
 *   1) Insert an update of the stack pointer.
 *
 *   2) Restore GP if needed.
 *
 *   3) If <gra_run>, then GRA will be run and each of the callee_saves
 *	registers is copied its save TN.
 *
 * ====================================================================
 */
static void
Generate_Exit (
  ST *st,		/* The subprogram's symtab entry */
  BB *bb,		/* The exit BB to receive code */
  BOOL gra_run,         /* Make the preferencing copies for GRA */
  BOOL is_region)       /* Is a nested region */

{
  INT callee_num;
  TN *stn;
  OP *op;
  OPS ops = OPS_EMPTY;
  ANNOTATION *ant = ANNOT_Get (BB_annotations(bb), ANNOT_EXITINFO);
  EXITINFO *exit_info = ANNOT_exitinfo(ant);
  BB *bb_epi;

#ifdef TARG_ST
  // FdF 20041105: No need for an epilog in case of a "noreturn" call.
  // [SC] When we need to unwind through a tailcall, we must ensure that
  // the return address is valid at the time of the tailcall.
  // If the "tailcall" BB ends in a call instruction then the call instruction
  // sets a valid return address, but if it ends in a goto instruction, we
  // should have an epilog.
  if (BB_call(bb) &&
      WN_Call_Never_Return(CALLINFO_call_wn(ANNOT_callinfo(ANNOT_Get (BB_annotations(bb), ANNOT_CALLINFO))))
      && ( OP_call(BB_last_op(bb))
	   || (!CG_emit_asm_dwarf && !CXX_Exceptions_On)))
    return;
#endif

  if ( is_region && gra_run ) {
    /* get out if region and running gra.  epilog code handled with
     * PU in gra.
     */
    return;
  }

  if ((BB_rid(bb) != NULL) && ( RID_level(BB_rid(bb)) >= RL_CGSCHED )) {
    if (gra_run) {
      /* if the exit is from a region, then we will create a new block
       * that is part of the PU to hold the exit code for gra.
       */
      bb_epi = Gen_And_Insert_BB_After(bb);
      BB_Transfer_Exitinfo(bb, bb_epi);
      Target_Simple_Fall_Through_BB(bb,bb_epi);
      BB_rid(bb_epi) = Current_Rid;
      Exit_BB_Head = BB_LIST_Delete(bb, Exit_BB_Head);
      Exit_BB_Head = BB_LIST_Push(bb_epi, Exit_BB_Head, &MEM_pu_pool);
    } else {
      /* if gra is not being run, then the epilog code has been added
       * to the return block.
       */
      return;
    }
  } else {
    bb_epi = bb;
  }

  if ( Trace_EE ) {
    #pragma mips_frequency_hint NEVER
    fprintf ( TFile,
	      "\n%s<calls> Generating exit for %s (BB:%d)\n",
	      DBar, ST_name(st), BB_id(bb_epi) );
  }

  /* Restore the caller's GP: */
  if (GP_Setup_Code == need_code) {
    /* we will put the gp in any available register if a leaf routine
     * to avoid the save/restore of the gp.  we will also make gp
     * available as a callee saved register so that in the worst case,
     * we will not generate worse code by using another register for
     * gp.  we don't yet have the information necessary to perform
     * this optimization in the presence of regions.
     */
    if ( !Use_Scratch_GP( TRUE ) ) {
      if (Caller_GP_TN == NULL) {
	if (stn = PREG_To_TN_Array[Caller_GP_Preg])
	  Caller_GP_TN = stn;
	else {
	  Caller_GP_TN = CGTARG_Gen_GP_TN ();
	  TN_MAP_Set(TN_To_PREG_Map, Caller_GP_TN, (void *)Caller_GP_Preg);
	  PREG_To_TN_Array[Caller_GP_Preg] = Caller_GP_TN;
	  PREG_To_TN_Mtype[Caller_GP_Preg] = TY_mtype(Spill_Ptr_Type);
	}
      }
      Exp_COPY (GP_TN, Caller_GP_TN, &ops);
    }
  }

  if ( gra_run )
    EETARG_Restore_Extra_Callee_Tns (&ops);

#ifdef TARG_ST
  // [SC]: EH return exits have the RA set up already.
  if (RA_TN != NULL
      && ! EXITINFO_is_eh_return(exit_info)) {
#else
  if (RA_TN != NULL) {
#endif
    if (PU_has_return_address(Get_Current_PU())) {
      /* If the return address builtin is required, restore RA_TN from the 
       * memory location for __return_address. 
       */
      ST *ra_sv_sym = Find_Special_Return_Address_Symbol();
      TN *ra_sv_tn = Build_TN_Like(RA_TN);
      Set_TN_save_creg (ra_sv_tn, TN_class_reg(RA_TN));
      Set_TN_spill(ra_sv_tn, ra_sv_sym);
      if (MTYPE_byte_size(Pointer_Mtype) < MTYPE_byte_size(Spill_Int_Mtype) ) {
  	/* In n32 the return_address is 4 bytes (pointer),
  	 * but we need 8-byte save/restore to make kernel and dbx happy. */
  	ra_sv_sym = ST_base(ra_sv_sym);		/* use 8-byte block */
  	Set_TN_spill(ra_sv_tn, ra_sv_sym);	/* so dwarf uses new addr */
      }
      CGSPILL_Load_From_Memory (ra_sv_tn, ra_sv_sym, &ops, CGSPILL_LCL, bb_epi);
      Exp_COPY (RA_TN, ra_sv_tn, &ops);
    }
    else {
#ifdef TARG_ST
      /*
       * RA_TN must be saved as a callee saved TN whatever is actual classification.
       */
      if (!EETARG_Save_With_Regmask (REGISTER_CLASS_ra, REGISTER_ra)) {
	Exp_COPY (RA_TN, SAVE_tn(Return_Address_Reg), &ops);
	Set_OP_no_move_before_gra(OPS_last(&ops));
      }

#else /* !TARG_ST */

      if (gra_run && PU_Has_Calls 
	&& TN_register_class(RA_TN) != ISA_REGISTER_CLASS_integer)
      {
	// because has calls, gra will need to spill this.
	// but if it is not already in an integer reg,
	// then gra will spill to memory whereas for ia64
	// it could use a stacked reg; ideally gra would handle
	// this, but it doesn't and is easy to just copy to int reg
	// by hand and then let gra use stacked reg.
	Exp_COPY (RA_TN, ra_intsave_tn, &ops );
      } else {
	/* Copy back the return address register from the save_tn. */
	Exp_COPY (RA_TN, SAVE_tn(Return_Address_Reg), &ops);
      }
      Set_OP_no_move_before_gra(OPS_last(&ops));

#endif /* TARG_ST */
    }
  }

  if (gra_run) {
    /* Copy from register TNs to the callee saves registers */
    for ( callee_num = 0; 
	  callee_num < Callee_Saved_Regs_Count; 
	  ++callee_num ) {
      TN *callee_tn = CALLEE_tn(callee_num);
      if (TN_is_save_reg(callee_tn) &&
	  !REGISTER_CLASS_multiple_save(TN_register_class(callee_tn))
	  && ! EETARG_Save_With_Regmask (TN_register_class(callee_tn),
					 TN_register(callee_tn)))
      {
        Exp_COPY ( CALLEE_ded_tn(callee_num), callee_tn, &ops );
	Set_OP_no_move_before_gra(OPS_last(&ops));
      }
    }
  }

  if (PU_Has_Calls) {
    EETARG_Restore_Pfs (Caller_Pfs_TN, &ops);
  }

  /* Restore the stack pointer.
   */
  if (Gen_Frame_Pointer && !PUSH_FRAME_POINTER_ON_STACK) {
    Exp_COPY (SP_TN, FP_TN, &ops);
  } 
  else {

    /* This is a placeholder; Adjust_Exit will replace it with the 
     * actual sequence once we know the size of the frame.
     */
    Exp_Spadjust (SP_TN, Frame_Len_TN, V_NONE, &ops);
  }
  EXITINFO_sp_adj(exit_info) = OPS_last(&ops);
#ifdef TARG_ST
  if (EXITINFO_is_eh_return(exit_info)
      && EH_Return_Stackadj_TN) {
    Exp_ADD (Pointer_Mtype, SP_TN, SP_TN, EH_Return_Stackadj_TN, &ops);
  }
#endif

  /* Restore the caller's frame pointer register if we used FP: */
  if ( Gen_Frame_Pointer 
        && !PUSH_FRAME_POINTER_ON_STACK
        && ABI_PROPERTY_Is_callee (
	TN_register_class(FP_TN),
	REGISTER_machine_id(TN_register_class(FP_TN), TN_register(FP_TN)) ))
  {
    if (Caller_FP_TN == NULL) {
      if (stn = PREG_To_TN_Array[Caller_FP_Preg ])
	Caller_FP_TN = stn;
      else {
	Caller_FP_TN = CGTARG_Gen_FP_TN ();
	TN_MAP_Set(TN_To_PREG_Map, Caller_FP_TN, (void *)Caller_FP_Preg);
	PREG_To_TN_Array[Caller_FP_Preg] = Caller_FP_TN;
	PREG_To_TN_Mtype[Caller_FP_Preg] = TY_mtype(Spill_Ptr_Type);
      }
    }
    Exp_COPY (FP_TN, Caller_FP_TN, &ops);
    Set_OP_no_move_before_gra(OPS_last(&ops));
  }

  /* Generate the return instruction, unless is this a tail call
   * block, in which case the xfer instruction is already there.
   */
  if (!BB_call(bb_epi)) {
#ifdef TARG_ST
    if (PU_is_interrupt(Get_Current_PU()) ||
	PU_is_interrupt_nostkaln(Get_Current_PU())) {
      Exp_Return_Interrupt(RA_TN, &ops);
    }
    else
#endif
      Exp_Return (RA_TN, &ops);
  }

  /* set the srcpos field for all the exit OPs */
  FOR_ALL_OPS_OPs(&ops, op)
    OP_srcpos(op) = EXITINFO_srcpos(exit_info);

#ifdef TARG_ST
  /* [CL] set the epilogue field for all the exit OPs */
  FOR_ALL_OPS_OPs(&ops, op) {
    Set_OP_epilogue(op);
  }
#endif

  /* If we're tracing, print the new stuff before merging it: */
  if ( Trace_EE ) {
    #pragma mips_frequency_hint NEVER
    Print_OPS(&ops);
  }

  /* Merge the new operations into the end of the exit BB: */
  if (BB_call(bb_epi)) {

    /* If it's a tail call block we insert the new operations in front
     * of the jump.
     */
    OP *point = BB_last_op(bb_epi);
    Is_True(OP_br(point), ("last tail call OP of BB:%d not a jump", BB_id(bb_epi)));
    BB_Insert_Ops_Before(bb_epi, point, &ops);
  } else {
    BB_Append_Ops(bb_epi, &ops);
  }
}

/* ====================================================================
 *   Set_Frame_Len
 *
 *   sets spadjust TN values.
 * ====================================================================
 */
void 
Set_Frame_Len (
  INT64 val
)
{
  Frame_Len = val;
  Set_TN_value(Frame_Len_TN, val);
  Set_TN_value(Neg_Frame_Len_TN, -val);
#ifdef TARG_ST
  /* Inform the target dependent part. */
  EETARG_Set_Frame_Len(val);
#endif
  return;
}

/* ====================================================================
 *   Init_Entry_Exit_Code
 *
 *   we now generate the final code after pu is processed,
 *   but still need to init some stuff before processing pu. 
 * ====================================================================
 */
void
Init_Entry_Exit_Code (
  WN *pu_wn
)
{
  Trace_EE = Get_Trace (TP_CGEXP, 64);

  GP_Setup_Code = undefined_code;
  Caller_GP_TN = NULL;
  Caller_FP_TN	= NULL;

  // initialize values to dummy value
  Frame_Len_TN = Gen_Unique_Literal_TN(0,8);
  Neg_Frame_Len_TN = Gen_Unique_Literal_TN(0,8);

  Gen_Frame_Pointer = (Current_PU_Stack_Model != SMODEL_SMALL);

  // target-specific code (e.g. for stacked registers)
  EETARG_Init_Entry_Exit_Code (pu_wn, Gen_Frame_Pointer);

  Init_Pregs ();

  LC_Used_In_PU = FALSE;
}

/* ====================================================================
 *   Generate_Entry_Exit_Code
 *
 *   Generate entry and exit code for the current PU.  This involves
 *   calling Generate_Entry for each entry BB, and Generate_Exit
 *   for each exit BB.
 * ====================================================================
 */
void
Generate_Entry_Exit_Code ( ST *pu, BOOL is_region )
{
  BB_LIST *elist;
  BOOL gra_run = ! CG_localize_tns;

  Is_True(pu != NULL,("Generate_Entry_Exit_Code, null PU ST"));
  /* assume GP_Setup_Code already set */

  for (elist = Entry_BB_Head; elist; elist = BB_LIST_rest(elist)) {
    Generate_Entry (BB_LIST_first(elist), gra_run );
  }

  Generate_Unique_Exit ();

  for (elist = Exit_BB_Head; elist; elist = BB_LIST_rest(elist)) {
    Generate_Exit (pu, BB_LIST_first(elist), gra_run, is_region );
  }

  if (GP_Setup_Code == need_code && !is_region)
    /* don't change setting till done with regions */
    GP_Setup_Code = final_code;

  return;
}

/* ====================================================================
 *   Adjust_GP_Entry
 *
 *   if added gp reference after usual gp setup time, then need to 
 *   add in gp setup. 
 * ====================================================================
 */
static void
Adjust_GP_Entry (
  BB *bb
)
{
  if (GP_Setup_Code == need_code) {

    FmtAssert(FALSE, ("Adjust_GP_Entry: shouldn't reach here"));

#if 0 
    /* The entry's symtab entry */
    ST *st = ENTRYINFO_name (
         ANNOT_entryinfo(ANNOT_Get(BB_annotations(bb),ANNOT_ENTRYINFO)));
    TN *got_disp_tn = CGTARG_Gen_Got_Disp_TN ();
#endif
  }
  return;
}

/* ====================================================================
 *   Adjust_GP_Exit
 *
 *   if added gp reference after usual gp setup time, then need to 
 *   add in gp setup. 
 * ====================================================================
 */
static void
Adjust_GP_Exit (
  BB *bb
)
{
  if (GP_Setup_Code == need_code) {

    FmtAssert(FALSE, ("Adjust_GP_Exit: shouldn't reach here"));

  }

  return;
}

/* ====================================================================
 *   Adjust_GP_Setup_Code
 *
 *   possibly add the GP setup code
 * ====================================================================
 */
void
Adjust_GP_Setup_Code (
  ST *pu, 
  BOOL allocate_registers
)
{
  BB_LIST *elist;

  if (GP_Setup_Code == no_code && PU_References_GP) {
    /*
     * This can happen for several reasons:
     * 1) when regions, don't see full PU at first;
     * 2) spilling a fcc introduces a gp reference;
     * 3) rematerialization may use gp.
     * So we can introduce GP references as late as LRA.
     * In this case, we have to add the gp setup/save/restore code,
     * and allocate registers for it (since after GRA/LRA).
     * The placement may not be optimal, but GRA can't shrinkwrap
     * anyways since new uses may occur after GRA.
     */
    /*
     * CHANGE:  setup gp after cgprep, and don't allow new gp
     * uses from fcc spills.  This way LRA sees that t9 and gp
     * are used.
     */
    FmtAssert(!allocate_registers, ("Created new GP reference during register allocation"));
    GP_Setup_Code = need_code;
  }
  else return;

  for (elist = Entry_BB_Head; elist; elist = BB_LIST_rest(elist)) {
    Adjust_GP_Entry(BB_LIST_first(elist));
  }

  for (elist = Exit_BB_Head; elist; elist = BB_LIST_rest(elist)) {
    Adjust_GP_Exit(BB_LIST_first(elist));
  }

  if (GP_Setup_Code == need_code)
    GP_Setup_Code = final_code;

  return;
}

/* ====================================================================
 *   Adjust_LC_Entry
 * ====================================================================
 */
static void
Adjust_LC_Entry (BB *bb)
{
  OP *op;
  INT i;
  FOR_ALL_BB_OPs_FWD(bb, op) {
    for ( i = 0; i < OP_opnds(op); ++i ) {
      if (OP_opnd(op,i) == LC_TN && OP_no_move_before_gra(op)) {
	BB_Remove_Op(bb, op);
	if (Trace_EE) {
#pragma mips_frequency_hint NEVER
	  fprintf(TFile, "<calls> remove save of LC in BB:%d\n", BB_id(bb));
	}
      }
    }
  }
}

/* ====================================================================
 *   Adjust_LC_Exit
 * ====================================================================
 */
static void
Adjust_LC_Exit (BB *bb)
{
  OP *op;
  INT i;
  FOR_ALL_BB_OPs_FWD(bb, op) {
    for ( i = 0; i < OP_results(op); ++i ) {
      if (OP_result(op,i) == LC_TN && OP_no_move_before_gra(op)) {
	BB_Remove_Op(bb, op);
	if (Trace_EE) {
#pragma mips_frequency_hint NEVER
	  fprintf(TFile, "<calls> remove restore of LC in BB:%d\n", BB_id(bb));
	}
      }
    }
  }
}

/* ====================================================================
 *   Adjust_LC_Setup_Code
 *
 *   possibly remove save/restore of LC.
 *   Must call this after cgprep
 * ====================================================================
 */
void
Adjust_LC_Setup_Code (void)
{
  if (LC_TN == NULL) return;	// doesn't exist for target
  if (LC_Used_In_PU) return;	// keep save/restore
  if (CG_localize_tns)	return;	// never generated initial save/restore

  BB_LIST *elist;
  for (elist = Entry_BB_Head; elist; elist = BB_LIST_rest(elist)) {
    Adjust_LC_Entry(BB_LIST_first(elist));
  }
  for (elist = Exit_BB_Head; elist; elist = BB_LIST_rest(elist)) {
    Adjust_LC_Exit(BB_LIST_first(elist));
  }
}

/* ====================================================================
 *   Assign_Prolog_Temps
 *
 *   Very simplistic register allocation. Intended soley for use
 *   on the sequence of insts to load the stack frame size constant.
 *   TODO:  merge this with Assign_Temp_Regs.
 * ====================================================================
 */
static void
Assign_Prolog_Temps(OP *first, OP *last, REGISTER_SET *temps)
{
  OP *op;
  ISA_REGISTER_CLASS cl;
  REGISTER reg;

  /* Assume we will only need one register and that its class will
   * be the same as $sp. Reserve the register now. Note that no
   * checking is performed to validate this assumption.
   */
  cl = TN_register_class(SP_TN);
  reg = REGISTER_SET_Choose(temps[cl]);
  /* Normally already initialized before the call. */
  FmtAssert(reg != REGISTER_UNDEFINED,("Undefined register"));

  /* Loop over the OPs in the sequence, allocating registers.
   */
  for (op = first; op != OP_next(last); op = OP_next(op)) {
    TN *tn;
    INT k;

    REGISTER_CLASS_OP_Update_Mapping(op);

    for (k = 0; k < OP_results(op); k++) {
      tn = OP_result(op,k);
      if (TN_register(tn) == REGISTER_UNDEFINED) {
	FmtAssert(TN_register_class(tn) == cl,
		  ("unexpected register class for unallocated register"));
	TN_Allocate_Register(tn, reg);
      }
    }

    for (k = 0; k < OP_opnds(op); k++) {
      tn = OP_opnd(op,k);
      if (TN_is_register(tn) && TN_register(tn) == REGISTER_UNDEFINED) {
	FmtAssert(TN_register_class(tn) == cl,
		  ("unexpected register class for unallocated register"));
	TN_Allocate_Register(tn, reg);
      }
    }
  }
}

/* ====================================================================
 *   Gen_Prolog_LDIMM64
 *
 *   Generate an instruction sequence to load an arbitrary 64-bit value
 * ====================================================================
 */
static TN *
Gen_Prolog_LDIMM64 (
  UINT64 val, 
  OPS *ops
)
{
  TN *src = Gen_Literal_TN(val, 8);
  TN *result = Build_TN_Of_Mtype (MTYPE_I8);

  Exp_Immediate (result, src, TRUE, ops);

  return result;
}

/* ====================================================================
 *   Adjust_Entry
 *
 *   Adjust the stack frame allocation code as necessary now that the
 *   actual frame size is known.
 * ====================================================================
 */
static void
Adjust_Entry (
  BB *bb
)
{
  UINT64 frame_len = Frame_Len;
  ANNOTATION *ant = ANNOT_Get(BB_annotations(bb), ANNOT_ENTRYINFO);
  ENTRYINFO *ent_info = ANNOT_entryinfo(ant);
  OP *ent_adj = ENTRYINFO_sp_adj(ent_info);
  OP *fp_adj;
  OP *sp_adj;
  TN *sp_incr;
  TN *fp_incr;

#ifdef TARG_ST200
  /* (cbr) hugly hack for st200 c++ exceptions. the handler sequence starts by
     saving r63 which is something after the catch. instead we are in the
     handler context and r63 must be preserved. I could have read one frame over
     in the runtime exceptions but it makes one extra frame to unwind.
     I tried to disable the saving of r63 in Generate_Entry by it is then not restored if exit blocks from the handler and the try function are merged.
  */
  if (BB_handler(bb)) {
    OP* op = BB_first_op(bb);
    while (op) {
      if (OP_store(op) &&
          TN_register(OP_opnd(op, OP_find_opnd_use(op, OU_storeval))) ==
          TN_register(RA_TN)) {
        BB_Remove_Op(bb, op);
        break;
      }
      op = OP_next(op);
    }
  }
#endif

  if (BB_handler(bb)) return;

  FmtAssert(ent_adj != NULL, ("Adjust Entry OP missing in BB::%d\n", BB_id(bb)));
  if (Trace_EE) {
    #pragma mips_frequency_hint NEVER
    fprintf(TFile,
	 "\n%s<calls> Adjusting entry for %s (BB:%d)\n",
	      DBar, ST_name(ENTRYINFO_name(ent_info)), BB_id(bb));
    fprintf(TFile, "\nFinal frame size: %llu (0x%llx)\n", 
                                            frame_len, frame_len);

    fprintf(TFile, "Adjust OP: ");
    Print_OP_No_SrcLine(ent_adj);
  }

  /* The ENTRYINFO annotation identifies the last instruction of the
   * stack frame allocation sequence. Therefore the instruction could
   * be either the adjustment of SP or FP. Find both the FP and SP
   * adjust OPs (will be the same when we have a virtual FP).
   */
  sp_adj = ent_adj;
  fp_adj = sp_adj;
  if (Gen_Frame_Pointer && !PUSH_FRAME_POINTER_ON_STACK) {
    do {
      sp_adj = OP_prev(sp_adj);
      //
      // Spills can be introduced now by GRA.  Skip 'em.
      //
    }
#ifdef TARG_ST
    // [CG]: Fix bug in result access
    while (sp_adj != NULL &&
	   !(OP_results(sp_adj) > 0 && TN_is_sp_reg(OP_result(sp_adj,0))));
    FmtAssert(sp_adj != NULL && OP_code(sp_adj) == TOP_spadjust, 
	      ("Did not find sp adjust OP in BB:%d\n", BB_id(bb)));
#else
    while (!OP_result(sp_adj, 0) || !TN_is_sp_reg(OP_result(sp_adj,0)));
#endif
  }

  /* Get the operands that are the frame size increment.
   */
  sp_incr = CGTARG_Get_SP_Incr (sp_adj);
  fp_incr = (fp_adj != sp_adj) ? CGTARG_Get_FP_Incr (fp_adj) : NULL;

  /* Trace the sequence before we modify it.
   */
  if (Trace_EE) {
    #pragma mips_frequency_hint NEVER
    fprintf(TFile, "\nOld stack frame allocation:\n");
    Print_OP_No_SrcLine(sp_adj);
    if (fp_adj != sp_adj) Print_OP_No_SrcLine(fp_adj);
  }

  /* We make assumptions about what we generated in Generate_Entry.
   * Try to make sure our assumptions were right:
   *   SP-adjust: TOP_spadjust $sp, $sp, -Frame_Len_TN
   *   FP-adjust: TOP_spadjust $fp, $sp, Frame_Len_TN
   */
  FmtAssert(   OP_code(sp_adj) == TOP_spadjust
	    && OP_results(sp_adj) == 1
	    && TN_is_sp_reg(OP_result(sp_adj,0))
	    && TN_is_sp_reg(OP_opnd(sp_adj, OP_has_predicate(sp_adj) ? 1:0))
	    && sp_incr == Neg_Frame_Len_TN
	    && (!OP_has_predicate(sp_adj) 
		|| OP_opnd(sp_adj, OP_find_opnd_use(sp_adj,OU_predicate)) == True_TN), 
	    ("Unexpected form of entry SP-adjust OP"));

  if (fp_adj != sp_adj) {
    FmtAssert(   OP_code(fp_adj) == TOP_spadjust 
	      && OP_results(fp_adj) == 1
	      /* && OP_result(fp_adj,0) == FP_TN */
	      && TN_is_dedicated_class_and_reg (OP_result(fp_adj,0), 
					TN_register_and_class(FP_TN))
	      && TN_is_sp_reg(OP_opnd(fp_adj, OP_has_predicate(fp_adj) ? 1:0))
	      && fp_incr != NULL 
	      && fp_incr == Frame_Len_TN 
	      && ( ! OP_has_predicate(fp_adj) 
		   || OP_opnd(fp_adj, OP_find_opnd_use(fp_adj,OU_predicate)) == True_TN), 
	      ("Unexpected form of entry FP-adjust OP"));
  }

  /* Perform any adjustments. We will either remove, change, or
   * leave alone, the adjustment OP.
   */
  if (frame_len == 0) {
    BB_Remove_Op(bb, sp_adj);

    if (sp_adj == fp_adj) {
      ent_adj = NULL;
    } else {
      OPS ops = OPS_EMPTY;

      /* Replace the FP adjust placeholder with the new adjustment OP.
       * Note that we just do this for an artifical situation that
       * at one time could be created with -TENV:large_stack, and in
       * in fact even that currently doesn't cause a problem. But just
       * in case, we make the case work.
       */
      Exp_COPY(FP_TN, SP_TN, &ops);
      ent_adj = OPS_last(&ops);
      OP_srcpos(ent_adj) = OP_srcpos(fp_adj);
      BB_Insert_Ops_Before(bb, fp_adj, &ops);
      BB_Remove_Op(bb, fp_adj);
    }

    if (Trace_EE) {
      #pragma mips_frequency_hint NEVER
      fprintf(TFile, "\nNew stack frame allocation:\n"
		     "-- removed --\n");
    }
  } else {
    OPS ops = OPS_EMPTY;
    TN *incr;
    OP *op;

    /* Load the increment into a register if it is too large to
     * be used as an immediate operand.
     */
    if (!CGTARG_Can_Fit_Immediate_In_Add_Instruction (frame_len)) {
      REGISTER_SET temps[ISA_REGISTER_CLASS_MAX+1];
      ISA_REGISTER_CLASS cl;
      REGISTER reg;

      /* Get the frame size into a register
       */
      REG_LIVE_Prolog_Temps(bb, sp_adj, fp_adj, temps);

      cl = TN_register_class(SP_TN);

      reg = REGISTER_SET_Choose(temps[cl]);

      if(reg == REGISTER_UNDEFINED) {
	if(Callee_Saved_Regs_Count > 0) {
	  Use_Callee_Save_TN_For_SpAdjust = CALLEE_tn(0);
	}

#ifdef TARG_ST
	// [JV] Check that we do not use the same TN as the one used
	// to save FP.
	// Check also that we get a register of same reg class as SP.
	INT callee_num = 1;
	TN *fp_save = NULL;
	while(callee_num < Callee_Saved_Regs_Count &&
	      ( TN_register_class(SP_TN) != TN_register_class(Use_Callee_Save_TN_For_SpAdjust) ||
		(fp_adj != sp_adj &&
		 TN_is_save_reg(Use_Callee_Save_TN_For_SpAdjust) &&
		 TN_save_rclass(Use_Callee_Save_TN_For_SpAdjust) == TN_register_class(FP_TN) &&
		 TN_save_reg(Use_Callee_Save_TN_For_SpAdjust) == TN_register(FP_TN) )
		)
	      ) {

	  if(fp_adj != sp_adj &&
	     TN_is_save_reg(Use_Callee_Save_TN_For_SpAdjust) &&
	     TN_save_rclass(Use_Callee_Save_TN_For_SpAdjust) == TN_register_class(FP_TN) &&
	     TN_save_reg(Use_Callee_Save_TN_For_SpAdjust) == TN_register(FP_TN) ) {
	    fp_save = Use_Callee_Save_TN_For_SpAdjust;
	  }

	  Use_Callee_Save_TN_For_SpAdjust = CALLEE_tn(callee_num);
	  callee_num++;
	}

	if(callee_num >= Callee_Saved_Regs_Count ||
	   Use_Callee_Save_TN_For_SpAdjust == NULL) {
	  // Let target give an available temp tn
	  Use_Callee_Save_TN_For_SpAdjust = EETARG_get_temp_for_spadjust(bb);
	  if(Use_Callee_Save_TN_For_SpAdjust != NULL &&
	     fp_save != NULL &&
	     TN_register_class(fp_save) == TN_register_class(Use_Callee_Save_TN_For_SpAdjust)) {
	    FmtAssert(TN_register(fp_save) != TN_register(Use_Callee_Save_TN_For_SpAdjust),("Selected register is the same as the one used to save FP"));
	  }
	}
	FmtAssert(Use_Callee_Save_TN_For_SpAdjust != NULL,("Cannot find callee saved reg"));
#endif
	if (Trace_EE) {
	  fprintf(TFile, "No more scrashed registers, get: ");
	  Print_TN(Use_Callee_Save_TN_For_SpAdjust,FALSE);
	}

	FmtAssert(Use_Callee_Save_TN_For_SpAdjust != NULL,("cannot find callee save"));
	if(TN_is_dedicated(Use_Callee_Save_TN_For_SpAdjust)) {
	  reg = TN_register(Use_Callee_Save_TN_For_SpAdjust);
	}
	else if(TN_is_save_reg(Use_Callee_Save_TN_For_SpAdjust)) {
	  reg = TN_save_reg(Use_Callee_Save_TN_For_SpAdjust);
	}
	else {
	  FmtAssert(FALSE,("Don't know how to get reg"));
	}
	temps[cl] = REGISTER_SET_Union1(temps[cl], reg);
      }

      if (Trace_EE) {
	ISA_REGISTER_CLASS cl;

	fprintf(TFile, "\nInteger temp register usage at prolog SP adjust:\n");
	FOR_ALL_ISA_REGISTER_CLASS(cl) {
	  if (cl == TN_register_class(SP_TN)) {
	    fprintf(TFile, "  avail=");
	    REGISTER_SET_Print(temps[cl], TFile);
	    fprintf(TFile, "\n");
	  }
	}
      }

      TN *src = Gen_Literal_TN(frame_len, Pointer_Size);
      incr = Build_TN_Of_Mtype (Pointer_Mtype);
      Exp_Immediate (incr, src, MTYPE_signed(Pointer_Mtype) ? TRUE : FALSE, &ops);
      /*
      incr = Gen_Prolog_LDIMM64(frame_len, &ops);
      */
      Assign_Prolog_Temps(OPS_first(&ops), OPS_last(&ops), temps);
    } else {

      /* Use the frame size symbol
       */
      incr = Frame_Len_TN;
    }

    /* Replace the SP adjust placeholder with the new adjustment OP
     */
    Exp_SUB (Pointer_Mtype, SP_TN, SP_TN, incr, &ops);
    FOR_ALL_OPS_OPs_FWD(&ops, op) OP_srcpos(op) = OP_srcpos(sp_adj);
    ent_adj = OPS_last(&ops);
    BB_Insert_Ops_Before(bb, sp_adj, &ops);
    BB_Remove_Op(bb, sp_adj);
#ifdef TARG_ST
    /* [CL] set the prologue field for all the entry OPs */
    FOR_ALL_OPS_OPs_FWD(&ops, op) {
      Set_OP_prologue(op);
    }
#endif
    if (Trace_EE) {
      #pragma mips_frequency_hint NEVER
      fprintf(TFile, "\nNew stack frame allocation:\n");
      FOR_ALL_OPS_OPs_FWD(&ops, op) Print_OP_No_SrcLine(op);
    }


    if (fp_adj != sp_adj) {

      /* Replace the FP adjust placeholder with the new adjustment OP
       */
      Exp_ADD (Pointer_Mtype, FP_TN, SP_TN, incr, OPS_Init(&ops));
      ent_adj = OPS_last(&ops);
      OP_srcpos(ent_adj) = OP_srcpos(fp_adj);
      BB_Insert_Ops_Before(bb, fp_adj, &ops);
      BB_Remove_Op(bb, fp_adj);
#ifdef TARG_ST
      /* [CL] set the prologue field for all the entry OPs */
      FOR_ALL_OPS_OPs_FWD(&ops, op) {
	Set_OP_prologue(op);
      }
#endif
      if (Trace_EE) {
	#pragma mips_frequency_hint NEVER
      	FOR_ALL_OPS_OPs_FWD(&ops, op) Print_OP_No_SrcLine(op);
      }
    }
  }

  /* Point to the [possibly] new SP adjust OP
   */
  ENTRYINFO_sp_adj(ent_info) = ent_adj;

#ifdef TARG_ST
  if ( Trace_EE ) {
#pragma mips_frequency_hint NEVER
    fprintf(TFile, "\nEntry sequence before EETARG_Fixup_Entry_Code:\n");
    OP *op, *sp_op = ENTRYINFO_sp_adj(ent_info);
    if (sp_op == NULL) fprintf(TFile, "\n--- empty sp_adjust sequence\n");
    else {
      BOOL emit = TRUE;
      FOR_ALL_BB_OPs_FWD(bb, op) {
	if (emit) Print_OP_No_SrcLine(op);
	if (sp_op == op) emit = FALSE;
      }
    }
  }
  
  // possible do target-dependent fixups
  EETARG_Fixup_Entry_Code (bb);

  if ( Trace_EE ) {
#pragma mips_frequency_hint NEVER
    fprintf(TFile, "\nEntry sequence after EETARG_Fixup_Entry_Code:\n");
    OP *op, *sp_op = ENTRYINFO_sp_adj(ent_info);
    if (sp_op == NULL) fprintf(TFile, "\n--- empty sp_adjust sequence\n");
    else {
      BOOL emit = TRUE;
      FOR_ALL_BB_OPs_FWD(bb, op) {
	if (emit) Print_OP_No_SrcLine(op);
	if (sp_op == op) emit = FALSE;
      }
    }
  }
#endif

}


/* ====================================================================
 *   Adjust_Exit
 *
 *   Adjust the stack frame de-allocation code as necessary now that the
 *   actual frame size is known.
 * ====================================================================
 */
static void
Adjust_Exit (
  ST *pu_st, 
  BB *bb
)
{

#ifdef TARG_ST
  // FdF 20041105: No EXITINFO_sp_adj was generated in case of a
  // "noreturn" call.
  if (!BB_exit_sp_adj_op(bb)) {
    Is_True(WN_Call_Never_Return(CALLINFO_call_wn(ANNOT_callinfo(ANNOT_Get(BB_annotations(bb), ANNOT_CALLINFO) ))),
	    ("Missing SP adjust"));
    return;
  }
#endif

  ANNOTATION *ant = ANNOT_Get(BB_annotations(bb), ANNOT_EXITINFO);
  EXITINFO *exit_info = ANNOT_exitinfo(ant);
  OP *sp_adj = EXITINFO_sp_adj(exit_info);
  UINT64 frame_len = Frame_Len;
  TN *sp_incr = NULL;

  /* Trace the sequence before we modify it.
   */
  if (Trace_EE) {
    #pragma mips_frequency_hint NEVER
    fprintf(TFile,
	    "\n%s<calls> Adjusting exit for %s (BB:%d)\n",
            DBar, ST_name(pu_st), BB_id(bb));
    fprintf(TFile, "\nFinal frame size: %llu (0x%llx)\n", frame_len, frame_len);
    fprintf(TFile, "\nOld stack frame de-allocation:\n");
    Print_OP_No_SrcLine(sp_adj);
  }

  /* Get the operand that is the frame size increment.
   */
  if (!Gen_Frame_Pointer || PUSH_FRAME_POINTER_ON_STACK) {
    // Arthur: opnd is 1 when no predication
    sp_incr = OP_has_predicate(sp_adj) ? OP_opnd(sp_adj, 2) : OP_opnd(sp_adj, 1);
  }

  /* We make assumptions about what we generated in Generate_Exit.
   * Try to make sure our assumptions were right:
   *   TOP_spadjust $sp, $sp, Frame_Len_TN
   * or
   *   <copy> $sp, ...
   */
  if (Gen_Frame_Pointer && !PUSH_FRAME_POINTER_ON_STACK) {
#ifdef TARG_ST
    FmtAssert(OP_copy(sp_adj) &&
	      TN_is_sp_reg(OP_result(sp_adj,OP_Copy_Result(sp_adj))),
	      ("Unexpected exit SP adjust OP"));
#else
    FmtAssert(OP_copy(sp_adj) &&
	      TN_is_sp_reg(OP_result(sp_adj,0)),
	      ("Unexpected exit SP adjust OP"));
#endif
  } else {
    FmtAssert(   OP_code(sp_adj) == TOP_spadjust 
	      && OP_results(sp_adj) == 1
	      && TN_is_sp_reg(OP_result(sp_adj,0))
	      && TN_is_sp_reg(OP_has_predicate(sp_adj) ? OP_opnd(sp_adj, 1) : OP_opnd(sp_adj, 0))
	      && sp_incr == Frame_Len_TN
	      && ( ! OP_has_predicate(sp_adj) 
		   || OP_opnd(sp_adj, OP_find_opnd_use(sp_adj,OU_predicate)) == True_TN), 
	      ("Unexpected form of exit SP-adjust OP"));
  }

  /* Perform any adjustments. We will either remove the adjustment
   * or leave it unchanged.
   */
  if (PUSH_FRAME_POINTER_ON_STACK) {
    OP* op = EETARG_High_Level_Procedure_Exit ();
    BB_Insert_Op_After (bb, sp_adj, op);
    BB_Remove_Op (bb, sp_adj);
    sp_adj = op;
  } else if (Gen_Frame_Pointer) {
    if ( Trace_EE ) {
      #pragma mips_frequency_hint NEVER
      fprintf(TFile, "\nNew stack frame de-allocation:\n"
	      "-- unchanged --\n");
    }
  } else if (frame_len == 0) {
    BB_Remove_Op(bb, sp_adj);
    sp_adj = NULL;

    if (Trace_EE) {
      #pragma mips_frequency_hint NEVER
      fprintf(TFile, "\nNew stack frame de-allocation:\n"
		     "-- removed --\n");
    }
  }
  else {
    OPS ops = OPS_EMPTY;
    TN *incr;
    OP *op;

    /* Load the increment into a register if it is too large to
     * be used as an immediate operand.
     */
    if (!CGTARG_Can_Fit_Immediate_In_Add_Instruction (frame_len)) {
      REGISTER_SET temps[ISA_REGISTER_CLASS_MAX+1];

      /* Get the frame size into a register */
      REG_LIVE_Epilog_Temps(pu_st, bb, sp_adj, temps);

      if (Trace_EE) {
	ISA_REGISTER_CLASS cl;

	fprintf(TFile, "\nInteger temp register usage at epilog SP adjust:\n");
	FOR_ALL_ISA_REGISTER_CLASS(cl) {
	  if (cl == TN_register_class(SP_TN)) {
	    fprintf(TFile, "  avail=");
	    REGISTER_SET_Print(temps[cl], TFile);
	    fprintf(TFile, "\n");
	  }
	}
      }

      TN *src = Gen_Literal_TN(frame_len, Pointer_Size);
      incr = Build_TN_Of_Mtype (Pointer_Mtype);
      Exp_Immediate (incr, src, MTYPE_signed(Pointer_Mtype) ? TRUE : FALSE, &ops);

      // Although it is called prolog_temps, it does the job right
      Assign_Prolog_Temps(OPS_first(&ops), OPS_last(&ops), temps);
    } 
    else {
      /* Use the frame size symbol
       */
      incr = Frame_Len_TN;
    }

    /* Replace the SP adjust placeholder with the new adjustment OP
     */
    Exp_ADD (Pointer_Mtype, SP_TN, SP_TN, incr, &ops);
    BB_Insert_Ops_Before(bb, sp_adj, &ops);
    BB_Remove_Op(bb, sp_adj);
    FOR_ALL_OPS_OPs_FWD(&ops, op) OP_srcpos(op) = OP_srcpos(sp_adj);
    sp_adj = OPS_last(&ops);

#ifdef TARG_ST
    /* [CL] set the epilogue field for all the exit OPs */
    FOR_ALL_OPS_OPs_FWD(&ops, op) {
      Set_OP_epilogue(op);
    }
#endif

    if ( Trace_EE ) {
      #pragma mips_frequency_hint NEVER
      fprintf(TFile, "\nNew stack frame de-allocation:\n");
      FOR_ALL_OPS_OPs_FWD(&ops, op) Print_OP_No_SrcLine(op);
    }
  } 

  /* Point to the [possibly] new SP adjust OP
   */
  EXITINFO_sp_adj(exit_info) = sp_adj;

#ifdef TARG_ST
  if ( Trace_EE ) {
#pragma mips_frequency_hint NEVER
    fprintf(TFile, "\nExit sequence before EETARG_Fixup_Exit_Code:\n");
    OP *op, *sp_op = EXITINFO_sp_adj(exit_info);
    BOOL emit = TRUE;//FALSE;
    if (sp_op == NULL) fprintf(TFile, "\n--- empty sp_adjust sequence\n");
    else {
      FOR_ALL_BB_OPs_FWD(bb, op) {
	if (sp_op == op) emit = TRUE;
	if (emit) Print_OP_No_SrcLine(op);
      }
    }
  }
  
  // possible do target-dependent fixups
  EETARG_Fixup_Exit_Code (bb);

  if ( Trace_EE ) {
#pragma mips_frequency_hint NEVER
    fprintf(TFile, "\nExit sequence after EETARG_Fixup_Exit_Code:\n");
    OP *op, *sp_op = EXITINFO_sp_adj(exit_info);
    BOOL emit = TRUE;//FALSE;
    if (sp_op == NULL) fprintf(TFile, "\n--- empty sp_adjust sequence\n");
    else {
      FOR_ALL_BB_OPs_FWD(bb, op) {
	if (sp_op == op) emit = TRUE;
	if (emit) Print_OP_No_SrcLine(op);
      }
    }
  }
#endif
}

/* ====================================================================
 *   Adjust_Alloca_Code
 * ====================================================================
 */
static void
Adjust_Alloca_Code (void)
{
  BB *bb;
  OP *op;
  OPS ops;
  OP *new_op;

  for (bb = REGION_First_BB; bb != NULL; bb = BB_next(bb)) {
    // When we remove op and insert a new one,
    // that confuses the BB_OPs iterator.
    // So iterate by hand instead.
    op = BB_first_op(bb);
    while (op != NULL) {
      if (OP_code(op) != TOP_spadjust) {
	op = OP_next(op);
	continue;
      }
      OPS_Init(&ops);
#ifdef TARG_ST
      // [CG]: Fix predicated/not predicated spadjust 
      // TOP_spadjust (plus) is like so:
      //  $sp = TOP_spadjust pred, $sp, $old_sp	(predicated)
      // or:
      //  $sp = TOP_spadjust $sp, $old_sp		(not predicated)
      // dealloca does copy of $old_sp  to $sp

      if (OP_variant(op) == V_SPADJUST_PLUS) {
	Exp_COPY (OP_result(op,0),
		  OP_opnd(op, OP_has_predicate(op) ? 2: 1),
		  &ops);
      }
      else if (OP_variant(op) == V_SPADJUST_MINUS) {
	// TOP_spadjust (minus) is like so:
	//  $sp = TOP_spadjust pred, $sp, ofst	(predicated)
	// or:
	//  $sp = TOP_spadjust $sp, ofst		(not predicated)
	Exp_SUB (Pointer_Mtype, OP_result(op,0),
		 OP_opnd(op, OP_has_predicate(op) ? 1: 0), 
		 OP_opnd(op, OP_has_predicate(op) ? 2: 1),
		 &ops);
      }
#else
      if (OP_variant(op) == V_SPADJUST_PLUS) {
	// dealloca does copy of kid to $sp (op1 is old sp value)

	// TOP_spadjust is like so: $sp = TOP_spadjust pred, $sp, ofst
	Exp_COPY (OP_result(op,0), OP_opnd(op, 2), &ops);
      }
      else if (OP_variant(op) == V_SPADJUST_MINUS) {
	Exp_SUB (Pointer_Mtype, OP_result(op,0),
		              OP_opnd(op, 1), OP_opnd(op, 2), &ops);
      }
#endif
      else {
	FmtAssert(FALSE, ("non-alloca spadjust"));
      }

      FOR_ALL_OPS_OPs_FWD(&ops, new_op) {
	OP_srcpos(new_op) = OP_srcpos(op);
	Is_True(OP_has_predicate(new_op) || !OP_has_predicate(op),
		                ("spadjust can't copy predicates"));
	// copy predicate to new copy/sub ops
	if (OP_has_predicate(op)) {
#ifdef TARG_ST
          // (cbr) Support for guards on false
          CGTARG_Predicate_OP(bb, new_op,
                              OP_opnd(op, OP_find_opnd_use(op, OU_predicate)),
                              OP_Pred_False(op, OP_find_opnd_use(op, OU_predicate)));
#else
          CGTARG_Predicate_OP(bb, new_op,
                              OP_opnd(op, OP_find_opnd_use(op, OU_predicate)));
#endif
        }
      }

      BB_Insert_Ops_Before(bb, op, &ops);
      BB_Remove_Op(bb, op);
      op = OPS_last(&ops);
    }
  }

  return;
}

/* ====================================================================
 *   Adjust_Entry_Exit_Code
 *
 *   Adjust entry and exit code for the current PU.  This involves
 *   calling Adjust_Entry for each entry BB, and Adjust_Exit
 *   for each exit BB.
 * ====================================================================
 */
void
Adjust_Entry_Exit_Code ( 
  ST *pu 
)
{
  BB_LIST *elist;

  Use_Callee_Save_TN_For_SpAdjust = NULL;

#ifdef TARG_ST

  if (Trace_EE) {
    INT callee_num;

    fprintf(TFile, "<calls> Callee saved regs used by %s\n", ST_name(pu));
    ISA_REGISTER_CLASS cl;
    FOR_ALL_ISA_REGISTER_CLASS(cl) {
      fprintf(TFile, "    ISA_REGISTER_CLASS_%s: ", 
               ISA_REGISTER_CLASS_INFO_Name(ISA_REGISTER_CLASS_Info(cl)));
      REGISTER_SET_Print(Callee_Saved_Regs_Mask[cl], TFile);
      fprintf(TFile, "\n");
    }
    fprintf(TFile, "\n");

    for (callee_num = 0; callee_num < Callee_Saved_Regs_Count; callee_num++) {
      TN *tn = CALLEE_tn(callee_num);
      REGISTER reg = TN_save_reg(tn);
      cl = TN_save_rclass(tn);

      fprintf(TFile, "    ");
      Print_TN(tn, FALSE);
      fprintf(TFile, " (%d:%d): ", TN_save_rclass(tn), TN_save_reg(tn));

      //      ISA_REGISTER_CLASS cl = TN_register_class(tn);
      //      REGISTER reg = TN_register(tn);
      if (REGISTER_SET_MemberP(Callee_Saved_Regs_Mask[cl], reg)) {
	fprintf (TFile, "saved TN%d saved from reg %d:%d", 
                                                  TN_number(tn), cl, reg);
      }
      fprintf(TFile, "\n");
    }
  }
#endif

  for (elist = Entry_BB_Head; elist; elist = BB_LIST_rest(elist)) {
    Adjust_Entry(BB_LIST_first(elist));
  }

  for (elist = Exit_BB_Head; elist; elist = BB_LIST_rest(elist)) {
    Adjust_Exit(pu, BB_LIST_first(elist));
  }

  FmtAssert (!(GP_Setup_Code == no_code && PU_References_GP),
		("had gp reference without setting up new gp"));

  if (PU_has_alloca(Get_Current_PU())) {
    // replace spadjusts with "real" code.
    Adjust_Alloca_Code ();
  }
}

#ifdef KEY
// See the interface description

INT Cgdwarf_Num_Callee_Saved_Regs (void)
{
  if (PU_has_altentry(Get_Current_PU())) {
    if (Debug_Level > 0 && Opt_Level > 1) {
      fprintf(stderr, 
            "Warning! -g is not supported at optimization level -O2 or above when ENTRY statements are used\n");
      DevWarn("NYI: we need different Saved_Callee_Saved_Regs for different entry");
    }
    return 0;
  }
  return ISA_REGISTER_CLASS_MAX;
}

#endif

#ifdef TARG_ST
/*
 * Update callee saved registers usages.
 */
static void
Compute_Callee_Saved_Registers(void)
{
  ISA_REGISTER_CLASS rc;
  
  FOR_ALL_ISA_REGISTER_CLASS(rc) {
    Callee_Saved_Regs_Mask[rc] = REGISTER_SET_EMPTY_SET;
  }
  
  for (BB *bb = REGION_First_BB; bb != NULL; bb = BB_next(bb)) {
    /* Do not account for call clobbered at it may contain RA_TN for instance
       that we want to ignore here as it is explicitly added below. */
    BB_Modified_Registers(bb, Callee_Saved_Regs_Mask, TRUE /* self */);
  }

  FOR_ALL_ISA_REGISTER_CLASS(rc) {
    REGISTER_SET potential_saved = REGISTER_CLASS_callee_saves(rc);
    if (RA_TN != NULL && TN_register_class(RA_TN) == rc) {
      potential_saved = REGISTER_SET_Union1(potential_saved, TN_register(RA_TN));
    }
    Callee_Saved_Regs_Mask[rc] = REGISTER_SET_Intersection (Callee_Saved_Regs_Mask[rc],
							    potential_saved);
  }
  
  /* Here we use this global flag to find out wether RA_TN needs saving or not.
     We use this because:
     - RA_TN ressource is not explicit and thus BB_Modified_Registers will not have it,
     - tail call optimization will have set it to FALSE if there is no need to save it.
  */
  if ((PU_Has_Calls || CG_localize_tns) && RA_TN != NULL) {
    rc = TN_register_class(RA_TN);
    Callee_Saved_Regs_Mask[rc] = REGISTER_SET_Union1(Callee_Saved_Regs_Mask[rc], TN_register(RA_TN));
  }
}

void
Adjust_Stack_Frame ( 
  ST *pu 
)
{
  /* Compute the actual callee saved usage. */
  Compute_Callee_Saved_Registers();
    
  EETARG_Fixup_Stack_Frame ();
}
#endif


#ifdef TARG_ST

/* Prolog ops are:
 * - any copy from a dedicated register that is not constant.
 * - copy, add or adjust that defines the frame pointer
 * - first copy, add or adjust that defines the stack pointer
 */

void
Check_Prolog(BB *bb) {
  OP *entry_sp_adj = BB_entry_sp_adj_op(bb);
  OP *stack_op = NULL;
  OP *op;

  // Entries of exception handlers do not have an sp_adj operation
  if ((entry_sp_adj == NULL) && BB_handler(bb))
    return;

  FmtAssert(entry_sp_adj!=NULL, ("sp_adjust op not set for entry BB:%d in PU:%s", BB_id(bb), Cur_PU_Name));

  FOR_ALL_BB_OPs_FWD(bb, op) {

    if (op == entry_sp_adj)
      break;

    if (OP_Is_Copy(op)) {
      TN *argument = OP_Copy_Operand_TN(op);
      if (TN_is_register(argument) &&
	  TN_is_dedicated(argument) &&
	  !TN_is_const_reg(argument))
	continue;
    }

    if (OP_results(op) == 1) {
      TN *result = OP_result(op, 0);
      if (result == FP_TN) {
	if (OP_Is_Copy(op) || OP_iadd(op))
	  continue;
      }
      if ((result == SP_TN) && (stack_op == NULL)) {
	stack_op = op;
	if (OP_Is_Copy(op) || OP_iadd(op) || (OP_code(op) == TOP_spadjust))
	  continue;
      }
    }

    FmtAssert(0, ("Illegal prolog sequence in BB:%d, PU:%s\n", BB_id(bb), Cur_PU_Name));
  }
}

/* Epilog ops are:
 * - last control operation
 * - any copy to a dedicated register
 * - last copy, add or adjust of stack pointer
 */
void
Check_Epilog(BB *bb) {
  OP *stack_op = NULL;
  OP *branch_op = NULL;
  OP *op;
  OP *exit_sp_adj = BB_exit_sp_adj_op(bb);

  if (exit_sp_adj == NULL)
    return;

  FOR_ALL_BB_OPs_REV(bb, op) {

    if (op == exit_sp_adj)
      break;

    if (OP_xfer(op) && (branch_op == NULL)) {
      branch_op = op;
      continue;
    }

    if (OP_Is_Copy(op)) {
      TN *result = OP_Copy_Result_TN(op);
      if (TN_is_dedicated(result))
	continue;
    }

    if (OP_results(op) == 1) {
      TN *result = OP_result(op, 0);
      if ((result == SP_TN) && (stack_op == NULL)) {
	stack_op = op;
	if (OP_Is_Copy(op) || OP_iadd(op) || (OP_code(op) == TOP_spadjust))
	  continue;
      }
    }

    FmtAssert(0, ("Illegal epilog sequence in BB:%d, PU:%s\n", BB_id(bb), Cur_PU_Name));
  }
}

void
Check_Prolog_Epilog () {

  for (BB *bb = REGION_First_BB; bb != NULL; bb = BB_next(bb)) {

    if (BB_entry(bb))
      Check_Prolog(bb);

    if (BB_exit(bb))
      Check_Epilog(bb);

  }
}
#endif
