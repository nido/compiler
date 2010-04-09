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


/* =======================================================================
 * =======================================================================
 *
 *  Module: reg_live.cxx
 *
 *  Description:
 *  ============
 *
 *  Physical register global live range analysis.
 *  Original extracted from Suneel's gcm.cxx.
 *
 * =======================================================================
 * =======================================================================
 */

#include "W_alloca.h"
#include "defs.h"
#include "config.h"
#include "mempool.h"
#include "tracing.h"
#include "timing.h"
#include "cgir.h"
#include "pu_info.h"
#include "cg.h"
#include "cg_flags.h"
#include "ttype.h"
#include "targ_sim.h"
#include "bb_set.h"
#include "freq.h"
#include "cgtarget.h"
#include "whirl2ops.h"
#include "dominate.h"
#include "findloops.h"
#include "cg_vector.h"
#include "gtn_universe.h"
#include "gtn_set.h"
#include "data_layout.h"

#include "reg_live.h"

static BOOL Trace_Register_Liveness = FALSE;

static REGSET Register_Livein;
static REGSET Register_Kill;
static MEM_POOL Reg_Live_Pool;
static BOOL pool_initialized = FALSE;

#define BB_Register_Livein(bb) \
	  (Register_Livein + BB_id(bb)*ISA_REGISTER_CLASS_COUNT - ISA_REGISTER_CLASS_MIN)
#define BB_Register_Kill(bb) \
	  (Register_Kill + BB_id(bb)*ISA_REGISTER_CLASS_COUNT - ISA_REGISTER_CLASS_MIN)

static INT32 default_size;

//
// Utility functions to manipulate REGSETs (arrays of REGISTER_SETs).
//

void REGSET_Print (REGSET set)
{
  ISA_REGISTER_CLASS rc;
  REGISTER reg;
  fprintf (TFile, "[");
  FOR_ALL_ISA_REGISTER_CLASS(rc) {
    FOR_ALL_REGISTER_SET_members (set[rc], reg) {
      fprintf (TFile, " %s", REGISTER_name (rc, reg));
    }
  }
  fprintf (TFile, " ]");
}

void 
REGSET_ASSIGN (REGSET set1, REGSET set2)
{
  ISA_REGISTER_CLASS rc;
  FOR_ALL_ISA_REGISTER_CLASS(rc) {
    set1[rc] = set2[rc];
  }
}

void 
REGSET_CLEAR (REGSET set)
{
  ISA_REGISTER_CLASS rc;
  FOR_ALL_ISA_REGISTER_CLASS(rc) {
    set[rc] = REGISTER_SET_EMPTY_SET;
  }
}

void 
REGSET_OR (REGSET set1, REGSET set2)
{
  ISA_REGISTER_CLASS rc;
  FOR_ALL_ISA_REGISTER_CLASS(rc) {
    set1[rc] = REGISTER_SET_Union(set1[rc], set2[rc]);
  }
}

BOOL 
REGSET_EQUALS (REGSET set1, REGSET set2)
{
  ISA_REGISTER_CLASS rc;
  FOR_ALL_ISA_REGISTER_CLASS(rc) {
    if (!REGISTER_SET_EqualP(set1[rc], set2[rc])) return FALSE;
  }
  return TRUE;
}

BOOL 
REGSET_INTERSECT (REGSET set1, REGSET set2)
{
  ISA_REGISTER_CLASS rc;
  FOR_ALL_ISA_REGISTER_CLASS(rc) {
    if (REGISTER_SET_IntersectsP(set1[rc], set2[rc])) return TRUE;
  }
  return FALSE;
}

static void 
REGSET_UPDATE_LIVEIN ( REGSET livein, REGSET liveout, REGSET kill)
{
  ISA_REGISTER_CLASS rc;
  FOR_ALL_ISA_REGISTER_CLASS(rc) {
    REGISTER_SET livein_set = REGISTER_SET_Difference(liveout[rc], kill[rc]);
    livein[rc] = REGISTER_SET_Union(livein[rc], livein_set);
  }
}


static void 
Add_PREG_To_REGSET (PREG_NUM preg, REGSET regset)
{
  if (preg != 0) {
    ISA_REGISTER_CLASS cl;
    REGISTER reg;

    FmtAssert(CGTARG_Preg_Register_And_Class(preg, &cl, &reg),
	      ("Don't know how to handle PREG%d", preg));

    regset[cl] = REGISTER_SET_Union1 (regset[cl], reg);
  }
}

// Utility routine to determine the parameter registers for:
//	- actual parameters for a procedure call
//	- formal parameters for a procedure
//
static void
Compute_Parameter_Regs (TY_IDX call_ty, WN *call_wn, REGSET parms)
{
  BOOL func_entry = WN_operator (call_wn) == OPR_FUNC_ENTRY;
  PLOC ploc;
  INT parm_count;
  if (func_entry) {
    parm_count = WN_num_formals(call_wn);
    ploc = Setup_Input_Parameter_Locations (call_ty);
  } else {
    parm_count = WN_num_actuals(call_wn);
    ploc = Setup_Output_Parameter_Locations (call_ty);
  }

  for (INT i = 0; i < parm_count; i++) {
    TY_IDX parm_ty = TY_Of_Parameter(WN_actual(call_wn,i));
    if (func_entry) {
      ploc = Get_Input_Parameter_Location (parm_ty);
      ploc = First_Input_PLOC_Reg (ploc, parm_ty);
    } else {
      ploc = Get_Output_Parameter_Location (parm_ty);
      ploc = First_Output_PLOC_Reg (ploc, parm_ty);
    }
    while (PLOC_is_nonempty(ploc)) {
      if (PLOC_on_stack(ploc)) break;	// no more register parameters.
      Add_PREG_To_REGSET (PLOC_reg(ploc), parms);
      ploc = func_entry ? Next_Input_PLOC_Reg () :
	Next_Output_PLOC_Reg ();
    }
  }
}

static void
Compute_Return_Regs (TY_IDX call_ty, REGSET return_regs)
{

  PREG_NUM retpreg[MAX_NUMBER_OF_REGISTERS_FOR_RETURN];
  TYPE_ID retmtype[MAX_NUMBER_OF_REGISTERS_FOR_RETURN];
  INT i;

  if (WHIRL_Return_Info_On) {

    RETURN_INFO return_info = Get_Return_Info (TY_ret_type(call_ty),
					       No_Simulated);
    FmtAssert (RETURN_INFO_count(return_info) <= MAX_NUMBER_OF_REGISTERS_FOR_RETURN, 
	("Compute_Return_Regs:  more return registers than can handle"));

    for (i = 0; i < RETURN_INFO_count(return_info); i++) {
	retmtype[i] = RETURN_INFO_mtype (return_info, i);
	retpreg[i] = RETURN_INFO_preg (return_info, i);
	Add_PREG_To_REGSET (retpreg[i], return_regs);
    }
  }

  else {
#ifdef TARG_ST
    FmtAssert(FALSE,("WHIRL_Return_Info must be on"));
#else
    Get_Return_Mtypes (TY_ret_type(call_ty),
		       No_Simulated, &retmtype[0], &retmtype[1]);
    Get_Return_Pregs (retmtype[0], retmtype[1], &retpreg[0], &retpreg[1]);
    Add_PREG_To_REGSET (retpreg[0], return_regs);
    Add_PREG_To_REGSET (retpreg[1], return_regs);
#endif
  }
}

/* ====================================================================
 *   Compute_PU_Regs
 * ====================================================================
 */
static void 
Compute_PU_Regs (
  REGSET livein, 
  REGSET liveout
)
{
  ST *pu_st = Get_Current_PU_ST ();
  ISA_REGISTER_CLASS rc;

  // Find the livein registers for the current procedure.
  if (livein != NULL) {
    // Add all the formal parameters to the livein set.
    Compute_Parameter_Regs (ST_pu_type(pu_st), 
	                  PU_Info_tree_ptr(Current_PU_Info), livein);
    // add sp, gp, ep, ra to the livein set.
    livein[REGISTER_CLASS_sp] = 
	REGISTER_SET_Union1 (livein[REGISTER_CLASS_sp], REGISTER_sp);
#ifdef TARG_ST
    //
    // Arthur: some of these may not be defined on a given target.
    //         In this case the class/reg pairs are not defined.
    //         check for validity
    //
    if (GP_TN != NULL) {
      livein[REGISTER_CLASS_gp] = 
	REGISTER_SET_Union1 (livein[REGISTER_CLASS_gp], REGISTER_gp);
    }
    if (Ep_TN != NULL) {
      livein[REGISTER_CLASS_ep] = 
	REGISTER_SET_Union1 (livein[REGISTER_CLASS_ep], REGISTER_ep);
    }
    if (RA_TN != NULL) {
      livein[REGISTER_CLASS_ra] = 
	REGISTER_SET_Union1 (livein[REGISTER_CLASS_ra], REGISTER_ra);
    }
#else
    livein[REGISTER_CLASS_gp] = 
	REGISTER_SET_Union1 (livein[REGISTER_CLASS_gp], REGISTER_gp);
    livein[REGISTER_CLASS_ep] = 
	REGISTER_SET_Union1 (livein[REGISTER_CLASS_ep], REGISTER_ep);
    livein[REGISTER_CLASS_ra] = 
	REGISTER_SET_Union1 (livein[REGISTER_CLASS_ra], REGISTER_ra);
#endif

    // add all the callee-save registers to the livein set.
    FOR_ALL_ISA_REGISTER_CLASS(rc) {
      livein[rc] = REGISTER_SET_Union (livein[rc], 
				    REGISTER_CLASS_callee_saves(rc));
    }
    // If current procedure is a nested function, add the static-link 
    // register to the livein set for the procedure.
    if (PU_is_nested_func(Pu_Table[ST_pu(pu_st)])) {
      livein[REGISTER_CLASS_static_link] = 
		  REGISTER_SET_Union1 (livein[REGISTER_CLASS_static_link], 
				       REGISTER_static_link);
    }
  }

  // Find the return registers for the current procedure.
  if (liveout != NULL) {
    Compute_Return_Regs (ST_pu_type(pu_st), liveout);

    // check return regs for each entry
    if (PU_has_altentry(Get_Current_PU())) {
	BB_LIST *bbl;
	BB *bb;
	ANNOTATION *ant;
	for (bbl = Entry_BB_Head; bbl; bbl = BB_LIST_rest(bbl)) {
		bb = BB_LIST_first(bbl);
		ant = ANNOT_Get (BB_annotations(bb), ANNOT_ENTRYINFO);
		pu_st = ENTRYINFO_name(ANNOT_entryinfo(ant));
	  Compute_Return_Regs (ST_pu_type(pu_st), liveout);
	}
    }

    // add all the callee-save registers to the liveout set.
    FOR_ALL_ISA_REGISTER_CLASS(rc) {
      liveout[rc] = REGISTER_SET_Union (liveout[rc], 
				     REGISTER_CLASS_callee_saves(rc));
#ifdef TARG_ST
      if (PU_Has_EH_Return) {
	liveout[rc] = REGISTER_SET_Union (liveout[rc],
					  REGISTER_CLASS_eh_return(rc));
      }
#endif      
    }

    // add sp to list of liveout registers.
    liveout[REGISTER_CLASS_sp] = 
	REGISTER_SET_Union1 (liveout[REGISTER_CLASS_sp], REGISTER_sp);
  }
}

/* ====================================================================
 *   Compute_Call_Regs
 *
 *   The livein, liveout and kill sets are computed for calls as follows:
 *
 *	livein:	parameter registers for the call
 *		gp, sp
 *		t9 (if pic-call)
 *		static-link (if call to nested function)
 *		ra, callee-saves (for tail calls)
#ifdef TARG_ST
 *              rs (if call returns struct by value)
#endif
 *
 *      liveout: return registers for call
 *
 *	  kill:	all caller-save registers
 * ====================================================================
 */
static void 
Compute_Call_Regs (
  BB *bb, 
  REGSET livein, 
  REGSET liveout, 
  REGSET kill
)
{  ANNOTATION *ant = ANNOT_Get (BB_annotations(bb), ANNOT_CALLINFO);
  WN *call_wn = CALLINFO_call_wn(ANNOT_callinfo(ant));
  INT num_parms = WN_num_actuals (call_wn);
  OPERATOR opr = WN_operator(call_wn);
  ST *call_st = (opr != OPR_ICALL) ? WN_st(call_wn) : NULL;
  TY_IDX call_ty = (call_st != NULL) ? ST_pu_type(call_st) : WN_ty(call_wn);
  ISA_REGISTER_CLASS cl;

  if (livein != NULL) {
    // add the parameter registers to the livein set.
    Compute_Parameter_Regs (call_ty, call_wn, livein);

    // If calling a nested function, add the static-link register to the 
    // livein set for the call.
    if (call_st != NULL && PU_is_nested_func(Pu_Table[ST_pu(call_st)])) {
      livein[REGISTER_CLASS_static_link] = 
		  REGISTER_SET_Union1 (livein[REGISTER_CLASS_static_link], 
				       REGISTER_static_link);
    }

    // add sp, gp to the livein set.
#ifdef TARG_ST
    // Arthur: GP may not be defined. Check it here.
    if (GP_TN != NULL) {
      livein[REGISTER_CLASS_gp] = 
		REGISTER_SET_Union1 (livein[REGISTER_CLASS_gp], REGISTER_gp);
    }
#else
    livein[REGISTER_CLASS_gp] = 
		REGISTER_SET_Union1 (livein[REGISTER_CLASS_gp], REGISTER_gp);
#endif
    livein[REGISTER_CLASS_sp] = 
		REGISTER_SET_Union1 (livein[REGISTER_CLASS_sp], REGISTER_sp);

    // add t9 if PIC call.
#ifdef TARG_ST
    if (opr != OPR_CALL && Gen_PIC_Calls && Ep_TN != NULL) {
#else
    if (opr != OPR_CALL && Gen_PIC_Calls) {
#endif
      livein[REGISTER_CLASS_ep] = 
		  REGISTER_SET_Union1 (livein[REGISTER_CLASS_ep], REGISTER_ep);
    }

    // add ra and callee saves if tail call.
    if (BB_tail_call(bb)) {
#ifdef TARG_ST
      // Arthur: RA_TN is not necessarily defined on this target
      if (RA_TN != NULL) {
	livein[REGISTER_CLASS_ra] = 
		  REGISTER_SET_Union1 (livein[REGISTER_CLASS_ra], REGISTER_ra);
      }
#else
      livein[REGISTER_CLASS_ra] = 
		  REGISTER_SET_Union1 (livein[REGISTER_CLASS_ra], REGISTER_ra);
#endif
      FOR_ALL_ISA_REGISTER_CLASS(cl) {
	REGISTER_SET callee_saves = REGISTER_CLASS_callee_saves(cl);
	livein[cl] = REGISTER_SET_Union(livein[cl], callee_saves);
      }
    }

#ifdef TARG_ST
    if (RS_TN != NULL) {
      // Arthur: use prototype to determine a call returning struct 
      //       coerced to void
      TY_IDX prototype;
      if (WN_operator(call_wn) == OPR_ICALL) 
	prototype = WN_ty(call_wn);
      else {
	ST_IDX func_stidx = WN_st_idx(call_wn);
	PU_IDX puidx = ST_pu(St_Table[func_stidx]);
	prototype = PU_prototype(Pu_Table[puidx]);
      }
      RETURN_INFO return_info = Get_Return_Info(TY_ret_type(prototype),
						No_Simulated);
      // If returning a structure by value through a dedicated register
      if (RETURN_INFO_return_via_first_arg(return_info)) {
	livein[REGISTER_CLASS_rs] = 
	  REGISTER_SET_Union1 (livein[REGISTER_CLASS_rs], REGISTER_rs);
      }
    }
#endif
  }

  if (liveout != NULL) {
    Compute_Return_Regs (call_ty, liveout);
  }

  if (kill != NULL) {
    ISA_REGISTER_CLASS rc;

    FOR_ALL_ISA_REGISTER_CLASS(rc) {
#ifdef TARG_ST
      kill[rc] = REGISTER_SET_Union (kill[rc],
				     BB_call_clobbered(bb, rc));
#else
      kill[rc] = REGISTER_SET_Union (kill[rc], 
				    REGISTER_CLASS_caller_saves(rc));
#endif
    }
  }
}


static void 
Compute_Asm_Regs (BB *bb, REGSET livein, REGSET liveout, REGSET kill)
{
  ANNOTATION *ant = ANNOT_Get (BB_annotations(bb), ANNOT_ASMINFO);
  Is_True(ant, ("ASMINFO annotation info not present"));
  ASMINFO *info = ANNOT_asminfo(ant);
  ISA_REGISTER_CLASS rc;

  FOR_ALL_ISA_REGISTER_CLASS(rc) {
    if (livein)  livein[rc] = ASMINFO_livein(info)[rc];
    if (liveout) liveout[rc] = ASMINFO_liveout(info)[rc];
    if (kill)    kill[rc] = ASMINFO_kill(info)[rc];
  }
}


//
// The rotating register livein and kill sets cannot be determined
// by analyzing the OPs. The information need to come from SWP while
// it generates the schedule (and allocates the registers). This routine
// retreives this info previously stored by SWP as a BB annotation.
//
static void 
Compute_Rotating_Regs (BB *bb, REGSET livein, REGSET liveout, REGSET kill)
{
  ANNOTATION *ant = ANNOT_Get (BB_annotations(bb), ANNOT_ROTATING_KERNEL);
  Is_True(ant, ("Rotating kernel annotation info not present"));
  ROTATING_KERNEL_INFO *info = ANNOT_rotating_kernel(ant);

  ISA_REGISTER_CLASS rc;
  FOR_ALL_ISA_REGISTER_CLASS(rc) {
    livein[rc] = ROTATING_KERNEL_INFO_live_in(info)[rc];
    kill[rc] = ROTATING_KERNEL_INFO_kill(info)[rc];
  }
}

static void
Update_REGSETs_For_Tail_Call (BB *bb, REGSET livein, REGSET kill)
{
  REGISTER_SET call_in[ISA_REGISTER_CLASS_MAX+1];
  REGSET_CLEAR (call_in);
  Compute_Call_Regs (bb, call_in, NULL, NULL);
  REGSET_UPDATE_LIVEIN (livein, call_in, kill);
}

static void
Update_REGSETs_For_Call (BB *bb, REGSET livein, REGSET kill)
{
  REGISTER_SET call_in[ISA_REGISTER_CLASS_MAX+1];
  REGISTER_SET call_kill[ISA_REGISTER_CLASS_MAX+1];
  REGSET_CLEAR (call_in);
  REGSET_CLEAR (call_kill);
  Compute_Call_Regs (bb, call_in, NULL, call_kill);
  REGSET_UPDATE_LIVEIN (livein, call_in, kill);
  REGSET_OR (kill, call_kill);
}

static void
Update_REGSETs_For_Asm (BB *bb, REGSET livein, REGSET kill)
{
  REGISTER_SET asm_in[ISA_REGISTER_CLASS_MAX+1];
  REGISTER_SET asm_kill[ISA_REGISTER_CLASS_MAX+1];
  REGSET_CLEAR (asm_in);
  REGSET_CLEAR (asm_kill);
  Compute_Asm_Regs (bb, asm_in, NULL, asm_kill);
  REGSET_UPDATE_LIVEIN (livein, asm_in, kill);
  REGSET_OR (kill, asm_kill);
}

static void
Update_REGSETs_For_Exit (REGSET livein, REGSET kill)
{
  REGISTER_SET pu_liveout[ISA_REGISTER_CLASS_MAX+1];
  REGSET_CLEAR (pu_liveout);
  // add the return registers to the livein set.
  Compute_PU_Regs (NULL, pu_liveout);
  REGSET_UPDATE_LIVEIN (livein, pu_liveout, kill);
}

static void
Update_REGSETs_For_Rotating_Kernel (BB *bb, REGSET livein, REGSET kill)
{
  REGISTER_SET kernel_in[ISA_REGISTER_CLASS_MAX+1];
  REGISTER_SET kernel_kill[ISA_REGISTER_CLASS_MAX+1];
  REGSET_CLEAR (kernel_in);
  REGSET_CLEAR (kernel_kill);
  Compute_Rotating_Regs (bb, kernel_in, NULL, kernel_kill);
  REGSET_OR (livein, kernel_in);
  REGSET_OR (kill, kernel_kill);
}

#ifdef TARG_ST
/* 
 * REG_LIVE_Compute_Local_Livein_Kill(BB *bb, REGSET livein, REGSET kill)
 *
 * [CG]: This function computes local livein and kill set for the
 * given bb.
 * The input livein and kill sets must be cleared first.
 * It is used by the global live analysis in REG_LIVE_Analyze_Region()
 * to initialize livein and kill sets before the iteration.
 * It is also used by the function REG_LIVE_Update_Livein_From_Liveout()
 * to locally update livein information for a BB.
 */
static void
REG_LIVE_Compute_Local_Livein_Kill(BB *bb, REGSET livein, REGSET kill)
{
  if (BB_rotating_kernel(bb)) {
    Update_REGSETs_For_Rotating_Kernel (bb, livein, kill);
  } else {
    OP *op;
    FOR_ALL_BB_OPs_FWD (bb, op) {
      INT i;
      for (i = 0; i < OP_opnds(op); i++) {
	TN *opnd_tn = OP_opnd(op,i);
	if (TN_is_register(opnd_tn)) {
	  ISA_REGISTER_CLASS cl = TN_register_class(opnd_tn);
	  REGISTER_SET gen = REGISTER_SET_Difference (TN_registers (opnd_tn), kill[cl]);
	  livein[cl] = REGISTER_SET_Union (livein[cl], gen);
	}
      }
      // Assume that conditional ops don't kill their definitions.
      if (!OP_cond_def(op)) {
	for (i = 0; i < OP_results(op); i++) {
	  TN *result_tn = OP_result(op,i);
	  ISA_REGISTER_CLASS cl = TN_register_class(result_tn);
	  kill[cl] = REGISTER_SET_Union (kill[cl], TN_registers (result_tn));
	}
      }
    }
  }
  
  if (BB_tail_call(bb)) {
    Update_REGSETs_For_Tail_Call (bb, livein, kill);
  }
  else if (BB_call(bb)) {
    Update_REGSETs_For_Call (bb, livein, kill);
  }
  else if (BB_asm(bb)) {
    Update_REGSETs_For_Asm (bb, livein, kill);
  }
  else if (BB_exit(bb)) {
    Update_REGSETs_For_Exit (livein, kill);
  }
}
#endif

void REG_LIVE_Analyze_Region(void)
{
  BB *bb;
  OP *op;
  BOOL changes;
  REGSET livein;
  REGSET kill;
  BB *last_bb = NULL;

  Is_True(Register_Livein == FALSE,
	  ("REG_LIVE_Analyze_Region called while facility is already in use"));

  if (!pool_initialized) {
    MEM_POOL_Initialize(&Reg_Live_Pool, "CG_Reg_Live_Pool", TRUE);
    MEM_POOL_Push(&Reg_Live_Pool);
    pool_initialized = TRUE;
  }

  Trace_Register_Liveness = Get_Trace (TP_GCM, 0x04);

  default_size = PU_BB_Count + 2;
  Register_Livein = 
    TYPE_MEM_POOL_ALLOC_N(REGISTER_SET, &Reg_Live_Pool,
			  (PU_BB_Count+2)*ISA_REGISTER_CLASS_COUNT);
  Register_Kill =
    TYPE_MEM_POOL_ALLOC_N(REGISTER_SET, &Reg_Live_Pool,
			  (PU_BB_Count+2)*ISA_REGISTER_CLASS_COUNT);

  // Compute local reg-livein and reg-kill sets.
  for (bb = REGION_First_BB; bb != NULL; bb = BB_next(bb)) {
    livein = BB_Register_Livein(bb);
    kill = BB_Register_Kill(bb);

#ifdef TARG_ST
    // [CG] Moved into function REG_LIVE_Compute_Local_Livein_Kill()
    REG_LIVE_Compute_Local_Livein_Kill(bb, livein, kill);
#else
    if (BB_rotating_kernel(bb)) {
      Update_REGSETs_For_Rotating_Kernel (bb, livein, kill);
    } else {
      FOR_ALL_BB_OPs_FWD (bb, op) {
	INT i;
	for (i = 0; i < OP_opnds(op); i++) {
	  TN *opnd_tn = OP_opnd(op,i);
	  if (TN_is_register(opnd_tn)) {
	    ISA_REGISTER_CLASS cl = TN_register_class(opnd_tn);
	    REGISTER reg = TN_register(opnd_tn);
            if ((reg != REGISTER_UNDEFINED) &&
		(!REGISTER_SET_MemberP (kill[cl], reg))) {
	      livein[cl] = REGISTER_SET_Union1 (livein[cl], reg);
	    }
	  }
	}
	// Assume that conditional ops don't kill their definitions.
	if (!OP_cond_def(op)) {
	  for (i = 0; i < OP_results(op); i++) {
	    TN *result_tn = OP_result(op,i);
	    if (TN_register(result_tn) != REGISTER_UNDEFINED) {
	      ISA_REGISTER_CLASS cl = TN_register_class(result_tn);
	      kill[cl] = REGISTER_SET_Union1 (kill[cl], TN_register(result_tn));
	    }
	  }
	}
      }
    }

    if (BB_tail_call(bb)) {
      Update_REGSETs_For_Tail_Call (bb, livein, kill);
    }
    else if (BB_call(bb)) {
      Update_REGSETs_For_Call (bb, livein, kill);
    }
    else if (BB_asm(bb)) {
      Update_REGSETs_For_Asm (bb, livein, kill);
    }
    else if (BB_exit(bb)) {
      Update_REGSETs_For_Exit (livein, kill);
    }
#endif
    last_bb = bb;
  }

  // Compute global reg-livein sets for each basic block.
  REGISTER_SET tmp[ISA_REGISTER_CLASS_MAX+1];
  REGISTER_SET liveout[ISA_REGISTER_CLASS_MAX+1];
  do {
    changes = FALSE;
    for (bb = last_bb; bb != NULL; bb = BB_prev(bb)) {
      BBLIST *bl;
      livein = BB_Register_Livein(bb);
      kill = BB_Register_Kill(bb);
      REGSET_ASSIGN (tmp, livein);
      REGSET_CLEAR (liveout);
      FOR_ALL_BB_SUCCS (bb, bl) {
	BB *succ_bb = BBLIST_item(bl);
	REGSET succ_livein = BB_Register_Livein(succ_bb);
	REGSET_OR (liveout, succ_livein);
      }
      REGSET_UPDATE_LIVEIN (livein, liveout, kill);
      if (!REGSET_EQUALS (tmp, livein)) {
	changes = TRUE;
      }
    }
  } while (changes);

  if (Trace_Register_Liveness) {
#pragma mips_frequency_hint NEVER
    for (bb = REGION_First_BB; bb != NULL; bb = BB_next(bb)) {
      fprintf (TFile, "BB:%-3d livein: ", BB_id(bb));
      REGSET_Print (BB_Register_Livein (bb));
      fprintf (TFile, "\n         kill: ");
      REGSET_Print (BB_Register_Kill (bb));
      fprintf (TFile, "\n");
    }
  }
}

/* =======================================================================
 *
 *  REG_LIVE_Prolog_Temps
 *
 *  See interface description.
 *
 * =======================================================================
 */
void
REG_LIVE_Prolog_Temps(
  BB *bb,
  OP *first,
  OP *last,
  REGISTER_SET *temps)
{
  REGISTER_SET live[ISA_REGISTER_CLASS_MAX+1];
  ISA_REGISTER_CLASS cl;
  OP *op;
  TN *tn;

  /* First compute the registers that are live at the end of the
   * SP adjust sequence. Start with an empty set.
   */
  FOR_ALL_ISA_REGISTER_CLASS(cl) {
    live[cl] = REGISTER_SET_EMPTY_SET;
  }

  /* We'll compute the live regs by starting at the end of the BB
   * and working our way forward. Add in the registers that are live
   * out of the block.
   */
  if (!CG_localize_tns ) {
    for (tn = GTN_SET_Choose(BB_live_out(bb));
	 tn != GTN_SET_CHOOSE_FAILURE;
	 tn = GTN_SET_Choose_Next(BB_live_out(bb), tn)
    ) {
      FmtAssert(TN_is_global_reg(tn),("TN%d is not global",TN_number(tn)));
      if (TN_register(tn) != REGISTER_UNDEFINED) {
	cl = TN_register_class(tn);
#ifdef TARG_ST
	/* [TTh] Support for multi-register TNs */
	live[cl] = REGISTER_SET_Union(live[cl], TN_registers(tn));
#else
	live[cl] = REGISTER_SET_Union1(live[cl], TN_register(tn));
#endif
      }
    }
  }

  /* Now scan the OPs backwards and update the liveness at each OP.
   * When we get to the end of the SP adjust sequence, we'll have
   * what's live at that point.
   */
  for (op = BB_last_op(bb); op != last; op = OP_prev(op)) {
    INT k;

    for (k = 0; k < OP_results(op); k++) {
      tn = OP_result(op,k);
      if (TN_register(tn) != REGISTER_UNDEFINED) {
	cl = TN_register_class(tn);
#ifdef TARG_ST
	/* [TTh] Support for multi-register TNs */
	live[cl] = REGISTER_SET_Difference(live[cl], TN_registers(tn));
#else
	live[cl] = REGISTER_SET_Difference1(live[cl], TN_register(tn));
#endif
      }
    }

    for (k = 0; k < OP_opnds(op); k++) {
      tn = OP_opnd(op,k);
      if ((TN_is_register(tn)) && (TN_register(tn) != REGISTER_UNDEFINED)) {
	cl = TN_register_class(tn);
#ifdef TARG_ST
	/* [TTh] Support for multi-register TNs */
	live[cl] = REGISTER_SET_Union(live[cl], TN_registers(tn));
#else
	live[cl] = REGISTER_SET_Union1(live[cl], TN_register(tn));
#endif
      }
    }

    /* We need to be very conservative when we encounter a call, because
     * at this late stage, LRA has removed NOOP copies of the argument
     * registers. Therefore we may not know that particular arg register
     * is live or not. To be safe, just say they are all live.
     */
    if (OP_call(op)) {
      FOR_ALL_ISA_REGISTER_CLASS(cl) {
	live[cl] = REGISTER_SET_Union(live[cl], 
				REGISTER_CLASS_function_argument(cl));
      }
    }
  }

  /* The first approximation to the available temps is the caller
   * saved registers with the live registers, determined above, removed.
   */
  FOR_ALL_ISA_REGISTER_CLASS(cl) {
    temps[cl] = REGISTER_SET_Difference(REGISTER_CLASS_caller_saves(cl), 
					live[cl]);
#ifdef TARG_ST
    // [TTh] Depending on the target, some extra caller save registers
    // might be removed.
    temps[cl] = REGISTER_SET_Difference(temps[cl],
					CGTARG_Forbidden_Prolog_Epilog_Registers(cl));
#endif
  }

  /* The last step is to reject any temp which is used or killed by
   * the OPs in the range the new temp must be live though.
   */
  for (op = first; op != last; op = OP_next(op)) {
    INT k;

    for (k = 0; k < OP_results(op); k++) {
      tn = OP_result(op,k);
      if (TN_register(tn) != REGISTER_UNDEFINED) {
	cl = TN_register_class(tn);
#ifdef TARG_ST
	/* [TTh] Support for multi-register TNs */
	temps[cl] = REGISTER_SET_Difference(temps[cl], TN_registers(tn));
#else
	temps[cl] = REGISTER_SET_Difference1(temps[cl], TN_register(tn));
#endif
      }
    }

    for (k = 0; k < OP_opnds(op); k++) {
      tn = OP_opnd(op,k);
      if ((TN_is_register(tn)) && (TN_register(tn) != REGISTER_UNDEFINED)) {
	cl = TN_register_class(tn);
#ifdef TARG_ST
	/* [TTh] Support for multi-register TNs */
	temps[cl] = REGISTER_SET_Difference(temps[cl], TN_registers(tn));
#else
	temps[cl] = REGISTER_SET_Difference1(temps[cl], TN_register(tn));
#endif
      }
    }
  }
}


/* =======================================================================
 *
 *  REG_LIVE_Epilog_Temps
 *
 *  See interface description.
 *
 * =======================================================================
 */
void
REG_LIVE_Epilog_Temps(
  ST *pu_st,
  BB *bb,
  OP *adj_op,
  REGISTER_SET *temps)
{
  ISA_REGISTER_CLASS cl;
  OP *op;
  TN *tn;

  /* Get the return registers for the exit block.  */
  REGSET_CLEAR(temps);

#ifdef TARG_ST
  if (BB_tail_call(bb)) {
    Compute_Call_Regs (bb,temps,NULL,NULL);
  }
  else
#endif
 {
    Compute_Return_Regs (ST_pu_type(pu_st), temps);
  }

  /* The set of available temps at the end of the exit block is
   * the caller saved regs with the return regs removed.
   */
  FOR_ALL_ISA_REGISTER_CLASS(cl) {
#ifdef TARG_ST
    // [SC] Avoid EH return registers if there is an EH_return in this function.
    if (PU_Has_EH_Return) {
      temps[cl] = REGISTER_SET_Union (temps[cl], REGISTER_CLASS_eh_return(cl));
    }
#endif
    temps[cl] = REGISTER_SET_Difference(REGISTER_CLASS_caller_saves(cl),
					temps[cl]);
#ifdef TARG_ST
    // [TTh] Depending on the target, some extra caller save registers
    // might be removed.
    temps[cl] = REGISTER_SET_Difference(temps[cl],
					CGTARG_Forbidden_Prolog_Epilog_Registers(cl));
#endif
  }

  /* Scan the OPs in the exit block backwards and update the available
   * temps at each OP, stopping after updating for the adjustment
   * OP itself.
   */
  for (op = BB_last_op(bb); op != OP_prev(adj_op); op = OP_prev(op)) {
    INT k;

    for (k = 0; k < OP_results(op); k++) {
      tn = OP_result(op,k);
      if (TN_register(tn) != REGISTER_UNDEFINED) {
	cl = TN_register_class(tn);
#ifdef TARG_ST
	/* [TTh] Support for multi-register TNs */
	temps[cl] = REGISTER_SET_Union(temps[cl], TN_registers(tn));
#else
	temps[cl] = REGISTER_SET_Union1(temps[cl], TN_register(tn));
#endif
      }
    }

    for (k = 0; k < OP_opnds(op); k++) {
      tn = OP_opnd(op,k);
      if ((TN_is_register(tn)) && (TN_register(tn) != REGISTER_UNDEFINED)) {
	cl = TN_register_class(tn);
#ifdef TARG_ST
	/* [TTh] Support for multi-register TNs */
	temps[cl] = REGISTER_SET_Difference(temps[cl], TN_registers(tn));
#else
	temps[cl] = REGISTER_SET_Difference1(temps[cl], TN_register(tn));
#endif
      }
    }
  }
}

// Returns true if there is an implicit use of <cl,reg> out of <bb>.
// The implicit uses are for function call parameters and return registers.
BOOL REG_LIVE_Implicit_Use_Outof_BB (ISA_REGISTER_CLASS cl, REGISTER reg, BB *bb)
{
  // Always mark unallocatable registers as liveout. This includes
  // registers like sp, fp, gp and dedicated register variables.
  if (!REGISTER_allocatable (cl, reg)) return TRUE;

  REGISTER_SET use[ISA_REGISTER_CLASS_MAX+1];

  REGSET_CLEAR(use);
  if (BB_tail_call(bb) || BB_call(bb)) {
    Compute_Call_Regs (bb, use, NULL, NULL);
  }
  else if (BB_asm(bb)) {
    Compute_Asm_Regs (bb, use, NULL, NULL);
  }
  else if (BB_exit(bb)) {
    Compute_PU_Regs (NULL, use);
  }
  return REGISTER_SET_MemberP (use[cl], reg);
}

// Returns TRUE if there is an implicit definition of <cl,reg> that
// reaches the top of <bb>. The implicit definitions are either for
// procedure entry point or for the return registers for a call.
BOOL REG_LIVE_Implicit_Def_Into_BB (ISA_REGISTER_CLASS cl, REGISTER reg, BB *bb)
{
  // Always mark unallocatable registers as an implicit def into all bbs. 
  // This includes registers like sp, fp, gp and dedicated register variables.
  if (!REGISTER_allocatable (cl, reg)) return TRUE;

  REGISTER_SET def[ISA_REGISTER_CLASS_MAX+1];
  REGSET_CLEAR (def);
  INT32 num_preds = BB_preds_len(bb);

  if (num_preds == 0) {
    Compute_PU_Regs (def, NULL);
  }
  else if (num_preds == 1) {
    BB *prev_bb = BB_Unique_Predecessor (bb);
    if (BB_call(prev_bb)) {
      Compute_Call_Regs (prev_bb, NULL, def, NULL);
    }
    else if (BB_asm(prev_bb)) {
      Compute_Asm_Regs (prev_bb, NULL, def, NULL);
    }
  }
  return REGISTER_SET_MemberP (def[cl], reg);
}



// Returns TRUE if the register (<cl>,<reg>) is live on entry to <bb>.
BOOL REG_LIVE_Into_BB(ISA_REGISTER_CLASS cl, REGISTER reg, BB *bb)
{
  // If we have not computed register liveness information, assume the 
  // worst case and return TRUE.
  if (Register_Livein == NULL) return TRUE;

  REGSET livein = BB_Register_Livein (bb);
  return REGISTER_SET_MemberP (livein[cl], reg);
}


// Returns TRUE if the register (<cl>,<reg>) is live on exit from <bb>.
BOOL REG_LIVE_Outof_BB (ISA_REGISTER_CLASS cl, REGISTER reg, BB *bb)
{
  BBLIST *succs;
  // If we have not computed register liveness information, assume the 
  // worst case and return TRUE.
  if (Register_Livein == NULL) return TRUE;

  FOR_ALL_BB_SUCCS(bb, succs) {
    REGSET livein = BB_Register_Livein (BBLIST_item(succs));
    if (REGISTER_SET_MemberP (livein[cl], reg)) 
      return TRUE;
  }
  return REG_LIVE_Implicit_Use_Outof_BB (cl, reg, bb);
}

#ifdef TARG_ST
// Return the number of registers in the range
//     [<cl,reg> : <cl,reg+nregs-1>]
// that are live on entry to basic block <bb>.
INT
NREGS_Live_Into_BB(ISA_REGISTER_CLASS cl, REGISTER reg, INT nregs, BB *bb)
{
  INT count = 0;
  REGISTER r;
  FOR_ALL_NREGS(reg, nregs, r) {
    if (REG_LIVE_Into_BB(cl, r, bb)) {
      count++;
    }
  }
  return count;
}

// Return the number of registers in the range
//     [<cl,reg> : <cl,reg+nregs-1>]
// that are live on exit from basic block <bb>.
INT
NREGS_Live_Outof_BB(ISA_REGISTER_CLASS cl, REGISTER reg, INT nregs, BB *bb)
{
  INT count = 0;
  REGISTER r;
  FOR_ALL_NREGS(reg, nregs, r) {
    if (REG_LIVE_Outof_BB(cl, r, bb)) {
      count++;
    }
  }
  return count;
}

// Return the number of registers from the set REGS
// that are live on entry to basic block BB.
INT
NREGS_Live_Into_BB(ISA_REGISTER_CLASS cl, REGISTER_SET regs, BB *bb)
{
  INT count = 0;
  REGISTER r;
  FOR_ALL_REGISTER_SET_members (regs, r) {
    if (REG_LIVE_Into_BB (cl, r, bb)) {
      count++;
    }
  }
  return count;
}

// Return the number of registers from the set REGS
// that are live on exit from basic block BB.
INT
NREGS_Live_Outof_BB(ISA_REGISTER_CLASS cl, REGISTER_SET regs, BB *bb)
{
  INT count = 0;
  REGISTER r;
  FOR_ALL_REGISTER_SET_members (regs, r) {
    if (REG_LIVE_Outof_BB (cl, r, bb)) {
      count++;
    }
  }
  return count;
}
#endif

// Adds (<cl>,<reg>) into live-in sets for <bb>.
void REG_LIVE_Update(ISA_REGISTER_CLASS cl, REGISTER reg, BB *bb)
{
  
  if (BB_id(bb) >= default_size) {
    INT32 old_size = default_size;
    INT32 new_size = MAX(default_size, 2 * old_size);
    Register_Livein = TYPE_MEM_POOL_REALLOC_N(REGISTER_SET, &Reg_Live_Pool,
					      Register_Livein,
					      old_size * ISA_REGISTER_CLASS_COUNT,
					      new_size * ISA_REGISTER_CLASS_COUNT);
    default_size = new_size;
  }
  
  REGSET livein = BB_Register_Livein (bb);
  if (reg != REGISTER_UNDEFINED)
    livein[cl] = REGISTER_SET_Union1 (livein[cl], reg);
}

#ifdef TARG_ST
/*
 * REG_LIVE_Update_Livein_From_Liveout(BB *bb)
 * Recompute live in information for the bb from the current
 * bb liveout set.
 * The livein sets of the successors BB must be up
 * to date. 
 * The updated live in set is conservative (a super set)
 * if the bb is in a cycle as no global iteration
 * is performed.
 * To update the live in set we compute:
 * 1. the current live out (union of live in of successors),
 * 2. local kill and local live in sets,
 * 3. then we update live in from these.
 */
void 
REG_LIVE_Update_Livein_From_Liveout(BB *bb)
{
  REGISTER_SET liveout[ISA_REGISTER_CLASS_MAX+1];
  REGSET livein;
  REGSET kill;

  // FdF 20100309: Extend the representation to include bb
  REG_LIVE_Update(ISA_REGISTER_CLASS_UNDEFINED, REGISTER_UNDEFINED, bb);

  // 1. Compute current live out set
  REGSET_CLEAR (liveout);
  BBLIST *bl;
  FOR_ALL_BB_SUCCS (bb, bl) {
    BB *succ_bb = BBLIST_item(bl);
    REGSET succ_livein = BB_Register_Livein(succ_bb);
    REGSET_OR (liveout, succ_livein);
  }
  
  // 2. Recompute local kill/livein
  livein = BB_Register_Livein(bb);
  kill = BB_Register_Kill(bb);
  REGSET_CLEAR (livein);
  REGSET_CLEAR (kill);
  REG_LIVE_Compute_Local_Livein_Kill(bb, livein, kill);
  
  // 3. Update live in set from liveout, and local livein and kill
  REGSET_UPDATE_LIVEIN (livein, liveout, kill);
}
#endif


// The client is finished using the facility -- clean up.
void REG_LIVE_Finish(void)
{
  MEM_POOL_Pop(&Reg_Live_Pool);
  MEM_POOL_Delete(&Reg_Live_Pool);
  pool_initialized = FALSE;
  Register_Livein = NULL;
  Register_Kill = NULL;
}
