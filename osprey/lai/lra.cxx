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


/* =======================================================================
 * =======================================================================
 *
 *  Module: lra.c
 *  $Revision$
 *  $Date$
 *  $Author$
 *  $Source$
 *
 *  Description:
 *  ============
 *
 *  Local Register Allocation.
 *
 * =======================================================================
 * =======================================================================
 */

#ifdef USE_PCH
#include "cg_pch.h"
#endif // USE_PCH
#pragma hdrstop

#include <alloca.h>

#include "defs.h"
#include "mempool.h"
#include "config.h"
#include "tracing.h"
#include "timing.h"
#include "cgir.h"
#include "targ_sim.h"
#include "wn.h"
#include "erglob.h"
#include "ercg.h"
#include "data_layout.h"
#include "bb_set.h"
#include "tn_set.h"
#include "gtn_universe.h"
#include "gtn_set.h"
#include "gtn_tn_set.h"
#include "tn_set.h"
#include "cg.h"
#include "cg_flags.h"
/* #include "cg_internal.h" */
#include "cxx_memory.h"
#include "calls.h"
#include "cgexp.h"
#include "register.h"
#include "reg_live.h"
#include "bbregs.h"
#include "gra.h"
#include "whirl2ops.h"
#include "tn_map.h"
#include "cg_spill.h"
#include "cg_vector.h"
#include "lra.h"
#include "glob.h"
#include "cgtarget.h"
/* #include "targ_proc_properties.h" */
#include "hb_sched.h"
/* #include "ebo.h" */

#define TN_is_local_reg(r)   (!(TN_is_dedicated(r) | TN_is_global_reg(r)))

/* regs that need to be saved at prolog and restored at epilog: */
static REGISTER_SET Callee_Saved_Regs_Used[ISA_REGISTER_CLASS_MAX+1];


/* Data structure to keep a list of available registers in each 
 * register class.
 */
typedef struct {
  BOOL reg[REGISTER_MAX+1];
} AVAIL_REGS;

static AVAIL_REGS avail_regs[ISA_REGISTER_CLASS_MAX+1];

/* Set of available registers in each register class. */
static REGISTER_SET avail_set[ISA_REGISTER_CLASS_MAX+1];

/* Set of excluded registers in each register class. */
static REGISTER_SET exclude_set[ISA_REGISTER_CLASS_MAX+1];

// Keeps track of the last register assigned for each register class.
// This is used to simulate a round-robin allocation of registers.
static REGISTER last_assigned_reg[ISA_REGISTER_CLASS_MAX+1];

/* Array of the OPs in the basic block being processed. */
static VECTOR Insts_Vector;
#define OP_VECTOR_element(v,i)  ((OP *)VECTOR_element(v,i))
#define Set_OP_VECTOR_element(v,i,op)   (VECTOR_element(v,i) = (void *)op)

/* set of registers that are live through the basic block */
static REGISTER_SET livethrough[ISA_REGISTER_CLASS_MAX+1];
// State variable to indicate if we need to compute the livethrough set.
static BOOL livethrough_computed;

/* If a result of an undeletable OP is not needed, */
/* it may be replaced with one of these TNs.         */
static TN *unused_tn_def[ISA_REGISTER_CLASS_MAX];


static BOOL do_global_locking = FALSE;

/* Memory pool for allocating things during register allocation of 
 * a single basic block. Memory allocated from this pool is not
 * initialized to zero.
 */
static MEM_POOL lra_pool;

static BOOL Trace_LRA;                  /* -Wb,-tt54:0x01 */
static BOOL Trace_LRA_Detail;           /* -Wb,-tt54:0x02 */
static BOOL Trace_LRA_Spill;            /* -Wb,-tt54:0x04 */
static BOOL Trace_LRA_Entry_Exit;       /* -Wb,-tt54:0x08 */
static BOOL Trace_Move_GRA_Spills;      /* -Wb,-tt54:0x10 */
// 	Trace_Fat_Points		/* -Wb,-tt54:0x20 */


/* Data structure to keep track of the local live range for each TN. 
 * There is only one live range for a TN even if there are multiple
 * definitions. This is to enforce the assignment of a single register
 * to a TN.
 */
typedef struct live_range {
  TN *tn;               /* the live range tn */
  mINT16 first_def;     /* instruction number for first def in live range. */
  mINT16 last_use;      /* instruction number for last use in live range. */
  mINT16 exposed_use;   /* instruction number for last exposed use (if any) */
  mUINT8 def_cnt;       /* number of defs in the live range. */
  mUINT8 use_cnt;       /* number of uses in the live range. */
  mUINT8 flags;         /* misc. flags (see definition below) */
  mREGISTER prefer_reg; /* if def of live range is a preferencing copy. */
  mINT16 first_spill;    /* first spill of exposed global */
  struct live_range *next; /* pointer to next live range */
} LIVE_RANGE;

#define LR_tn(lr)               ((lr)->tn)
#define LR_first_def(lr)        ((lr)->first_def)
#define LR_last_use(lr)         ((lr)->last_use)
#define LR_exposed_use(lr)      ((lr)->exposed_use)
#define LR_def_cnt(lr)          ((lr)->def_cnt)
#define LR_use_cnt(lr)          ((lr)->use_cnt)
#define LR_flags(lr)            ((lr)->flags)
#define LR_prefer_reg(lr)       ((lr)->prefer_reg)
#define LR_first_spill(lr)	((lr)->first_spill)
#define LR_next(lr)             ((lr)->next)

#define LR_ASSIGNED      0x1    /* LR assigned register (Compute_Fat_Points) */
#define LR_ADDED         0x2    /* LR added to Live_LRs_Vector(Fix_LRA_Blues) */
#define LR_RELOADABLE    0x4    /* LR is reloadable. */
#define LR_FAT_ALLOCATED 0x8	// LR was allocated real register by fat point
                                // calculation
#define LR_assigned(lr)         (LR_flags(lr) & LR_ASSIGNED)
#define Set_LR_assigned(lr)     (LR_flags(lr) |= LR_ASSIGNED)
#define LR_added(lr)            (LR_flags(lr) & LR_ADDED)
#define Set_LR_added(lr)        (LR_flags(lr) |= LR_ADDED)
#define LR_reloadable(lr)       (LR_flags(lr) & LR_RELOADABLE)
#define Set_LR_reloadable(lr)   (LR_flags(lr) |= LR_RELOADABLE)
#define Reset_LR_reloadable(lr) (LR_flags(lr) &= ~LR_RELOADABLE)
#define LR_fat_allocated(lr)    (LR_flags(lr) & LR_FAT_ALLOCATED)
#define Set_LR_fat_allocated(lr)(LR_flags(lr) |= LR_FAT_ALLOCATED)
#define Reset_LR_fat_allocated(lr)(LR_flags(lr) &= ~LR_FAT_ALLOCATED)

#define LR_Is_Undefined_Local(lr) \
  (TN_is_local_reg(LR_tn(lr)) && LR_def_cnt(lr) == 0)

/* List of all the live ranges for a basic block */
static LIVE_RANGE *Live_Range_List;

static hTN_MAP live_range_map;

// The number of times we have tried to allocate registers for the 
// current basic block. We make different spilling decisions depending
// on the trip count.
static INT Trip_Count;
#define MAX_TRIP_COUNT 128

static ST *Magic_Spill_Location;

/* Array containing the LRs live across a fatpoint. */
static VECTOR Live_LRs_Vector;
#define LR_VECTOR_element(v,i)  ((LIVE_RANGE *)VECTOR_element(v,i))

/* map from OP -> opnum */
static BB_OP_MAP op_to_opnum;

/* Data structures to keep track of the best spill candidate */

typedef enum {
  SPILL_NONE, 
  SPILL_GLOBAL_REG, 
  SPILL_LIVE_RANGE, 
  SPILL_MOVE_DEF,
  SPILL_MOVE_USE
} SPILL_KIND;

typedef struct spill_candidate {
  SPILL_KIND spill_kind;
  INT spill_num;
  union {
    struct {
      mREGISTER global_spill_reg;
      mISA_REGISTER_CLASS spill_cl;
    } s1;
    LIVE_RANGE *spill_lr;
    struct {
      LIVE_RANGE *move_lr;
      mUINT16 from;
      mUINT16 to;
    } s2;
  } u1;
  float cost;
  INT benefit;
  struct spill_candidate *next;
} SPILL_CANDIDATE;

//
// globals for calculating fat_points in Allocate_Registers
//
typedef INT32 FAT_POINTS_TYPE;
#define FAT_POINTS_MAX INT32_MAX
#define FAT_POINTS_MIN INT32_MIN
static FAT_POINTS_TYPE *fat_points;
static REGISTER last_infinite_freed;
static ISA_REGISTER_CLASS failing_class;
static hTN_MAP fat_point_regs_map;
static BOOL use_fat_point_regs;
static REGISTER reg_infinite;
static INT fatpoint_min;
static TN_SET *bb_live;
static INT local_spills = 0;
static INT global_spills = 0;
static INT trace_tn = 0;
static INT trace_bb = 0;
static INT trace_op = 0;
static ISA_REGISTER_CLASS trace_cl = (ISA_REGISTER_CLASS)2;
#define Calculating_Fat_Points() (failing_class != ISA_REGISTER_CLASS_UNDEFINED)

//
// don't do standard traces after we begin to calculate fat points.  tn's
// will have bogus registers and such.
//
#define Always_Trace(t) (t)
#define Do_LRA_Trace(t) ((t) && !Calculating_Fat_Points())
#define Do_LRA_Fat_Point_Trace(t) ((t) && Calculating_Fat_Points())

//
// allow 32 bit register values during fat point calculation.  tn register
// field too small, and we're using an infinite register set.  these should
// contain the only references to TN_register and TN_Allocate_Register in
// the file.
//
inline REGISTER
LRA_TN_register(TN *tn)
{
  REGISTER reg;
  
  //
  // if we have a valid register set in the tn, use it.  for speed,
  // we only put "infinite register set" registers in the hash table.
  // 
  reg = TN_register(tn);
  if (use_fat_point_regs && reg == REGISTER_UNDEFINED) {
    return((REGISTER)(INTPTR) hTN_MAP_Get(fat_point_regs_map, tn));
  }
  return(reg);
}

inline void
LRA_TN_Allocate_Register(TN *tn, REGISTER reg)
{
  //
  // for speed, put valid registers in the tn, even if
  // we're computing fat points.
  //
  if (use_fat_point_regs && reg > REGISTER_MAX) {
    hTN_MAP_Set(fat_point_regs_map, tn, (void *) reg);
  } else {
    TN_Allocate_Register(tn, reg);
  }
}
#undef TN_register
#undef TN_Allocate_Register
#define TN_register(tn) TN_dont_use_tn_register_error(tn)
#define TN_Allocate_Register(tn) TN_dont_use_tn_Allocate_Register_error(tn)

//
// these two traces are kludgy.  trace_op and trace_tn don't always match up,
// and you'll get some traces that aren't really the one's you want, but its
// okay for now ... you gotta hack the sources to use it, anyway.
//
static void
LRA_Print_Liveness()
{
  INT live_count;
  TN *tn;
  
  for (tn = TN_SET_Choose(bb_live), live_count=0;
       tn != TN_SET_CHOOSE_FAILURE;
       tn = TN_SET_Choose_Next(bb_live, tn), live_count++);

  fprintf(TFile,"<lra> Live at OP:%d (lc=%d): ", trace_op, live_count);
  TN_SET_Print(bb_live, TFile);
  fprintf(TFile,"\n");
}

static void
Print_Live_Across()
{
  INT cnt = 0;
  fprintf(TFile,"<lra> Live across %d: ", trace_op);

  for (INT i = 0; i < VECTOR_count(Live_LRs_Vector); i++) {
    LIVE_RANGE *lr = LR_VECTOR_element (Live_LRs_Vector, i);
    if (LR_first_def(lr) < trace_op && LR_last_use(lr) > trace_op) {
      cnt++;
      fprintf(TFile," TN%d", TN_number(LR_tn(lr)));
    }
  }
  fprintf(TFile,"(lc=%d)\n", cnt);
}

//
// convert an op to a noop. It's ok to remove any TOP_noop's, so
// there's no need to indentify the the ones we specifically create.
//
static inline void
Mark_For_Removal(OP *op)
{
  OP_Change_To_Noop(op);
}

//
// determine if an op has been marked for removal
//
inline BOOL
Is_Marked_For_Removal(OP *op)
{
  return OP_code(op) == TOP_noop;
}

//
// determine if reordering is allowed
//
static BOOL
Check_Allow_Reorder() {
#ifdef TARG_ST100
  //
  // Arthur: must also check if scheduling is enabled all together
  //
  if ((CG_opt_level > 1) &&
      IGLS_Enable_All_Scheduling && LOCS_Enable_Scheduling &&
      (Trip_Count <= CG_opt_level) &&
      !Get_Trace (TP_ALLOC, 0x2000)) {
#else
  if ((CG_opt_level > 1) &&
      (Trip_Count <= CG_opt_level) &&
      !Get_Trace (TP_ALLOC, 0x2000)) {
#endif
    return TRUE;
  }
  return FALSE;
}

//
// determine if we can call the local scheduler again
// on the block.
//
static BOOL
Check_Allow_Reschedule()
{
  if (Check_Allow_Reorder() &&
      Trip_Count == 1 &&
      !Get_Trace (TP_ALLOC, 0x0200)) {
    return TRUE;
  }
  return FALSE;
}

//
// delete op from Insts_Vector and set its fat_point to -1
//
inline void
Clobber_Op_Info(INT opnum, ISA_REGISTER_CLASS spill_cl)
{
  Set_OP_VECTOR_element(Insts_Vector, opnum, NULL);
  fat_points[opnum] = -1;
}

//
// returns TRUE if the TN referenced in lr is used by the op specified
// by opnum.
//
static BOOL
Op_Uses_TN(TN *spill_tn, INT opnum)
{
  BOOL is_local_tn = TN_is_local_reg (spill_tn);
  REGISTER spill_reg = REGISTER_UNDEFINED;
  OP *op = OP_VECTOR_element(Insts_Vector, opnum);
  ISA_REGISTER_CLASS cl = TN_register_class(spill_tn);

  if (!is_local_tn) {
    spill_reg = LRA_TN_register(spill_tn);
  } else {
    spill_reg = REGISTER_UNDEFINED;
  }
  //
  // op is null if instruction deleted from block during spilling of
  // a live range
  //
  return (op == NULL || (is_local_tn && OP_Refs_TN (op, spill_tn)) ||
	  (!is_local_tn && OP_Refs_Reg (op, cl, spill_reg)));
}

static void Print_Avail_Set (BB *bb)
{
  REGISTER reg;
  ISA_REGISTER_CLASS cl;

  FOR_ALL_ISA_REGISTER_CLASS(cl) {
    fprintf (TFile, "(BB:%d, cl:%d) \n\tAvail:", BB_id(bb), cl);
    REGISTER_SET_Print (avail_set[cl], TFile);
    fprintf (TFile, "\n\tAvail(Registers):");
    FOR_ALL_REGISTER_SET_members (avail_set[cl], reg) {
      fprintf (TFile, " %s", REGISTER_name (cl, reg));
    }
    if (!CG_localize_tns) {
      fprintf (TFile, "\n\tGRA Grant:");
      REGISTER_SET_Print (GRA_Local_Register_Grant (bb, cl), TFile);
      fprintf (TFile, "\n\tLRA Request: %d", LRA_Register_Request (bb, cl));
    }
    fprintf (TFile, "\n");
  }
}


/* ======================================================================
 * Init_Avail_Set
 *
 * Initialize avail_set to list of registers that are available to LRA.
 * ======================================================================*/
static void Init_Avail_Set (BB *bb)
{
  REGISTER reg;
  ISA_REGISTER_CLASS cl;
  OP *op;
  INT i;

  FOR_ALL_ISA_REGISTER_CLASS(cl) {
    if (CG_localize_tns) {
      /* Don't include callee save registers that have not already been
       * used by LRA (since they cost extra).  if this block is an exit,
       * we don't even add the used ones until we see the sp adjustment
       * (to avoid the use of a callee saved register after the restores).
       */
      avail_set[cl] = 
	REGISTER_SET_Difference (REGISTER_CLASS_allocatable(cl),
				 REGISTER_CLASS_callee_saves(cl));
#ifdef HAS_STACKED_REGISTERS
      REGISTER_SET avail_stacked =
	REGISTER_Get_Stacked_Avail_Set(ABI_PROPERTY_stacked, cl);
      avail_set[cl] = REGISTER_SET_Difference(avail_set[cl],
					      REGISTER_CLASS_stacked(cl));
      avail_set[cl] = REGISTER_SET_Union(avail_set[cl], avail_stacked);
#endif
      if (!BB_exit(bb)) {
	avail_set[cl] = REGISTER_SET_Union(avail_set[cl],
					   Callee_Saved_Regs_Used[cl]);
      }
    }
    else {
      /* Use the registers granted by GRA */
      avail_set[cl] = GRA_Local_Register_Grant (bb, cl);
      exclude_set[cl] = REGISTER_SET_EMPTY_SET;
      if (BB_call(bb)) {
        /* Note: all this code may be unnecessary because gra_grant already
                 adds caller_save registers into the local register grant. */
        OP *call_op = BB_last_op(bb);
        if ((call_op != NULL) &&
            OP_call(call_op) &&
            OP_cond_def(call_op)) {
          /* If the call is conditionally executed, the return value registers
             may already contain values that must be preserved through the block. */
          avail_set[cl] = REGISTER_SET_Union (
                            avail_set[cl],
                            REGISTER_SET_Difference (REGISTER_CLASS_function_value(cl),
                                                     REGISTER_CLASS_caller_saves(cl)));
        } else {
          avail_set[cl] = REGISTER_SET_Union (
                            avail_set[cl], REGISTER_CLASS_caller_saves(cl));
        }
      }
      if (BB_mod_rotating_registers(bb) ||
	  Is_Predicate_REGISTER_CLASS(cl) && BB_mod_pred_rotating_registers(bb)) {
	exclude_set[cl] = REGISTER_Get_Requested_Rotating_Registers(cl);
	avail_set[cl] = REGISTER_SET_Difference (avail_set[cl], exclude_set[cl]);
      }
    }
  }

  /* Remove all dedicated and global TNs that are defined from the list of 
   * registers available at the bottom of the block. This needs to be done 
   * because GRA will include these registers in the grant.
   */
  FOR_ALL_BB_OPs_FWD (bb, op) {
    for (i = 0; i < OP_results(op); ++i) {
      TN *result_tn = OP_result(op, i);
      if (!TN_is_local_reg(result_tn)) {
        reg = LRA_TN_register(result_tn);
        cl = TN_register_class (result_tn);
        avail_set[cl] = REGISTER_SET_Difference1 (avail_set[cl], reg);
      }
    }
  }

  if (Do_LRA_Trace(Trace_LRA)) {
    Print_Avail_Set (bb);
  }
}


static void
Init_Insts_Vector (BB *bb, MEM_POOL *pool)
{
  OP *op;

  Insts_Vector = VECTOR_Init (BB_length(bb)+1, pool);
  /* make the zero'th element NULL. */
  VECTOR_Add_Element (Insts_Vector, NULL);
  FOR_ALL_BB_OPs_FWD (bb, op) {
    VECTOR_Add_Element (Insts_Vector, op);
  }

  if (VECTOR_count(Insts_Vector) != (BB_length(bb)+1)) {
    fprintf (TFile, "BAD VECTOR: length=%d\n", VECTOR_count(Insts_Vector));
    Print_BB (bb);
  }
}


/* ======================================================================
 * Return the live range for the <tn>. If one does not exist yet, create
 * an new live range and return it. For non-local TNs, the live range
 * is for the dedicated TN corresponding to the assigned register.
 * ======================================================================*/

static LIVE_RANGE *
LR_For_TN (TN *tn)
{
  LIVE_RANGE *lr;

  if (!TN_is_local_reg(tn) && (LRA_TN_register(tn) != REGISTER_UNDEFINED)) {
    tn = Build_Dedicated_TN (TN_register_class(tn), LRA_TN_register(tn), 0);
  }
  lr = (LIVE_RANGE *) hTN_MAP_Get (live_range_map, tn);
  return lr;
}


/* For global TNs, the LR is for the dedicated TN corresponding to the
 * assigned register. If all references to this register are the same
 * global TN, use it for the LR_tn(lr) field. Otherwise use the dedicated
 * tn for the LR_tn field.
 */
static LIVE_RANGE *
Create_LR_For_TN (TN *tn, BB *bb, BOOL in_lra, MEM_POOL *pool)
{
  LIVE_RANGE *lr;
  BOOL local_lr = TN_is_local_reg(tn);
  TN *orig_tn = tn;
  ISA_REGISTER_CLASS cl = TN_register_class(tn);
  REGISTER reg = LRA_TN_register(tn);

  if (!local_lr && (reg != REGISTER_UNDEFINED)) {
    tn = Build_Dedicated_TN (cl, reg, 0);
  }
  lr = (LIVE_RANGE *) hTN_MAP_Get (live_range_map, tn);
  if (lr == NULL) {
    lr = TYPE_MEM_POOL_ALLOC (LIVE_RANGE, pool);
    bzero (lr, sizeof(LIVE_RANGE));
    LR_tn(lr) = orig_tn;
    LR_next(lr) = Live_Range_List;

    //
    // default is that globals spilled on entry.  will set otherwise
    // if left in register for some period of time.
    //
    LR_first_spill(lr) = 1;
    
    // If <cl,reg> is available for allocation at the end of the bb, then it
    // is not liveout of the bb.
    if (!local_lr && (!in_lra || !REGISTER_SET_MemberP (avail_set[cl], reg))) {
      /* make this live range extend just beyond the last OP in the bb */
      LR_last_use(lr) = BB_length(bb)+1;
      /* increment use count to account for the TN being live-out */
      LR_use_cnt(lr)++;
    }
    Live_Range_List = lr;
    hTN_MAP_Set (live_range_map, tn, lr);
  }
  else if (LR_tn(lr) != orig_tn) {
    LR_tn(lr) = tn;
  }
  return lr;
}


static void
Print_BB_For_LRA (BB *bb) 
{
  OP *op;
  INT i;
  fprintf (TFile, "--------------------------------------------\n");
  fprintf (TFile, "LRA for BB:%d  trip_count:%d\n", BB_id(bb), Trip_Count);
  fprintf (TFile, "--------------------------------------------\n");
  i = 0;
  FOR_ALL_BB_OPs_FWD (bb, op) {
    i++;
    fprintf (TFile, "OP:%2d> ", i);
    Print_OP_No_SrcLine (op);
  }
}


static void
Print_Live_Range (LIVE_RANGE *lr)
{
  fprintf (TFile, "  %s_LR>TN%d  %3d(%d) to %3d(%d), exposed:%d \n",
                TN_is_local_reg(LR_tn(lr)) ? "LOCAL" : "GLOBAL",
                TN_number(LR_tn(lr)), LR_first_def(lr), LR_def_cnt(lr),
	        LR_last_use(lr), LR_use_cnt(lr), LR_exposed_use(lr));
}


static void
Print_Live_Ranges (BB *bb)
{
  LIVE_RANGE *lr;

  fprintf (TFile, "--------------------------------------------\n");
  fprintf (TFile, "LIVE RANGES for BB%d\n", BB_id(bb));
  fprintf (TFile, "--------------------------------------------\n");
  for (lr = Live_Range_List; lr != NULL; lr = LR_next(lr)) {
    Print_Live_Range (lr);
  }
}



/* ======================================================================
 * Create the live ranges for all the local TNs in the basic block.
 *
 * Output:   
 *
 *    1. live_range_map: live ranges for all TNs referenced in bb.
 *    2. Live_Range_List: list of all live ranges in bb.
 *    3. Insts_Vector: vector of instructions in the bb.
 *    4. TN_register field for all local TNs set to REGISTER_UNDEFINED.
 * ======================================================================*/
static void
Setup_Live_Ranges (BB *bb, BOOL in_lra, MEM_POOL *pool)
{
  INT opnum;

  /* create a new live_range_map every time through this routine. The 
   * old one is automatically discarded. 
   * TODO: reclaim space for the the old live_range_map.
   */
  live_range_map = hTN_MAP_Create (pool);
  Live_Range_List = NULL;

  Init_Insts_Vector (bb, pool);

  for (opnum = 1; opnum <= BB_length(bb); opnum++) {
    OP *op = OP_VECTOR_element (Insts_Vector, opnum);
    INT opndnum;
    INT resnum;

    /* process all the operand TNs. */
    for (opndnum = 0; opndnum < OP_opnds(op); opndnum++) {
      TN *tn = OP_opnd(op, opndnum);
      LIVE_RANGE *clr;

      if (!TN_is_register(tn)) continue;

      clr = Create_LR_For_TN (tn, bb, in_lra, pool);

      if (!TN_is_local_reg(tn)) {
        if (LR_def_cnt(clr) == 0) {
          LR_exposed_use(clr) = opnum;
        }
        /* Add this use to the live range for this TN. */
        LR_use_cnt(clr)++;
      }
      else {
        LRA_TN_Allocate_Register (tn, REGISTER_UNDEFINED);
        if (LR_def_cnt(clr) == 0) {

	  if (!OP_dummy(op) && in_lra && (!BB_entry(bb) || (TFile != stdout))) {
	    /* This can happen only if we have a use before a definition. */
            /* Note: ignore things in the first block, where we may
               legitimately need to save input registers. */
	    DevWarn ("TN%d(PREG%d) used before definition in BB:%d",
		     TN_number(tn), TN_To_PREG(tn), BB_id(bb));
	  }
          LR_exposed_use(clr) = opnum;
        }
        /* Add this use to the live range for this TN. */
        LR_last_use(clr) = opnum;
        LR_use_cnt(clr)++;
      }
    }

    /* process the result TN */
    for (resnum = 0; resnum < OP_results(op); ++resnum) {
      TN *tn = OP_result(op, resnum);
      LIVE_RANGE *clr = Create_LR_For_TN (tn, bb, in_lra, pool);
      if (OP_cond_def(op) &&
          TN_is_global_reg(tn) &&
          (!TN_is_dedicated(tn) ||
           (CGTARG_Is_Preference_Copy(op) &&
            TN_is_local_reg(OP_opnd(op,CGTARG_Copy_Operand(op))) &&
            TN_spill(OP_opnd(op,CGTARG_Copy_Operand(op))))) &&
          (LRA_TN_register(tn) != REGISTER_UNDEFINED) &&
          !REGISTER_SET_MemberP(avail_set[TN_register_class(tn)], LRA_TN_register(tn))) {
       /* Note: This series of checks was added because
          conditionally defined values may require that a previous value be
          carried into a block. We also need to detect when a spilled value
          was reloaded and is copied into a dedicated register, because the
          code that creates spills and reloads is not smart enough to add
          the proper predicates to the memory ops. 

          The intent of this check is to extend the live range of the register
          to the top of the block.  Unfortunately, this is overly conservative
          and sometimes results in a free register being unavailable for use.
       */
        if (Do_LRA_Trace(Trace_LRA_Detail)) {
          fprintf (TFile, "OP:%d>> don't reuse result reg >> ", opnum); Print_OP_No_SrcLine (op);
        }

        LR_exposed_use(clr) = opnum;

      }
      if (LR_def_cnt(clr) == 0) {
        LR_first_def(clr) = opnum;
      }
      LR_def_cnt(clr)++;
      if (TN_is_local_reg(tn)) {
        LRA_TN_Allocate_Register (tn, REGISTER_UNDEFINED);
        LR_last_use(clr) = opnum;
        if (CGTARG_Is_Preference_Copy(op)) {
          TN *src_tn = OP_opnd(op, CGTARG_Copy_Operand(op));
          if (!TN_is_local_reg(src_tn))
            LR_prefer_reg(clr) = LRA_TN_register(src_tn);
        }
      }
      else {
        if (CGTARG_Is_Preference_Copy(op)) {
          TN *src_tn = OP_opnd(op,CGTARG_Copy_Operand(op));
          if (TN_is_local_reg(src_tn)) {
            LIVE_RANGE *src_lr = LR_For_TN (src_tn);
            LR_prefer_reg(src_lr) = LRA_TN_register(tn);
          }
        }
      }
    }

  }
}


static BOOL
Is_OP_Spill_Load (OP *op, ST *spill_loc)
{
  if (!OP_load(op)) return FALSE;

  INT n = TOP_Find_Operand_Use(OP_code(op), OU_offset);
  if (n < 0) {
    return (TN_spill(OP_result(op,0)) == spill_loc);
  }

  TN *ctn = OP_opnd(op, n);
  return (TN_is_constant(ctn) && 
          TN_is_symbol(ctn) &&
          TN_var(ctn) == spill_loc);
}

static BOOL
Is_OP_Spill_Store (OP *op, ST *spill_loc)
{
  if (!OP_store(op)) return FALSE;

  INT n = TOP_Find_Operand_Use(OP_code(op), OU_offset);
  if (n < 0) {
    n = TOP_Find_Operand_Use(OP_code(op), OU_storeval);
    if (n < 0) return FALSE;
    return (TN_spill(OP_opnd(op,n)) == spill_loc);
  }

  TN *ctn = OP_opnd(op, n);
  return (TN_is_constant(ctn) && 
          TN_is_symbol(ctn) &&
          TN_var(ctn) == spill_loc);
}


static void
Mark_Reloadable_Live_Ranges (ISA_REGISTER_CLASS cl)
{
  // Disable Reloading for spilling with -Wb,-ttlra:0x1000.
  if (Get_Trace (TP_ALLOC, 0x1000)) return;

  for (LIVE_RANGE *lr = Live_Range_List; lr != NULL; lr = LR_next(lr)) 
  {
    // Look for live ranges with a single definition and no exposed uses.
    if (LR_exposed_use(lr) || LR_def_cnt(lr) != 1) continue;

    OP *def_op = OP_VECTOR_element (Insts_Vector, LR_first_def(lr));
    // For now, restrict ourselves to considering only non-volatile loads.
    if (!OP_load(def_op) || OP_has_implicit_interactions(def_op))
      continue;

    TN *result_tn;
    INT i;
    for (i = OP_results(def_op) - 1; i >= 0; --i) {
      if (OP_result(def_op, i) == LR_tn(lr)) {
	result_tn = OP_result(def_op, i);
      }
    }
    if (TN_register_class(result_tn) != cl) continue;

    BOOL reloadable = TRUE;
    for (i = 0; i < OP_opnds(def_op); i++) {
      TN *opnd_tn = OP_opnd(def_op,i);
      if (TN_is_register(opnd_tn) && TN_register_class(opnd_tn) == cl) {
        LIVE_RANGE *opnd_lr = LR_For_TN (opnd_tn);
        if (LR_last_use(opnd_lr) < LR_last_use(lr)) {
          reloadable = FALSE;
          break;
        }
      }
    }
    if (!reloadable) continue;

    ARC_LIST *arcs;
    for (arcs = OP_succs(def_op); arcs != NULL; arcs = ARC_LIST_rest(arcs)) {
      ARC *arc = ARC_LIST_first(arcs);
      if (ARC_kind(arc) == CG_DEP_REGIN) continue;
      OP *succ_op = ARC_succ(arc);
      INT succ_opnum = BB_OP_MAP32_Get (op_to_opnum, succ_op);
      if (succ_opnum < LR_last_use(lr)) {
        reloadable = FALSE;
        break;
      }
    }

    if (reloadable) Set_LR_reloadable (lr);
  }
}


// A live-range that has the LR_reloadable attribute might no longer be
// reloadable.  We have to check if there is a store to the spill location 
// corresponding to any of the operands of the defining op before the last 
// use. If there is such a spill-store, reset the LR_reloadable attribute.
static BOOL Is_LR_Reloadable (LIVE_RANGE *lr)
{
  if (!LR_reloadable(lr)) return FALSE;

  OP *def_op = OP_VECTOR_element (Insts_Vector, LR_first_def(lr));

  for (INT i = 0; i < OP_opnds(def_op); i++) {
    TN *opnd_tn = OP_opnd(def_op,i);
    ST *spill_loc;
    if (TN_is_register(opnd_tn) &&
        !TN_is_rematerializable (opnd_tn) &&
	(spill_loc = TN_spill(opnd_tn)) != NULL) 
    {
      OP *last_use = (LR_last_use(lr) < VECTOR_size(Insts_Vector)) ?
			OP_VECTOR_element (Insts_Vector, LR_last_use(lr)) : 
			NULL;

      for (OP *op = OP_next(def_op); op != last_use; op = OP_next(op)) {
	if (Is_OP_Spill_Store (op, spill_loc)) {
	  Reset_LR_reloadable (lr);
	  return FALSE;
	}
      }
    }
  }
  return TRUE;
}


/* Remove self copies that might have been left behind, and noop
 * ops left as place holders for redundant assignments
 */
static void 
Remove_Redundant_Code (BB *bb)
{
  OP *op = BB_first_op (bb);
  do {
    OP *next_op = OP_next(op);
    if ((CGTARG_Is_Preference_Copy(op) &&
#ifdef TARG_ST100
	 // Arthur: we need to consider the COPYA,COPYD, etc. as
	 //         target copies. It seems that IA64 does not
	 //         consider int->fp moves as a copy ... why ?
	 //
	 //         Thus, not only equivalence of TN_register matters
	 //         but also its register class.
	 //
	 LRA_TN_register(OP_result(op,0)) ==
	 LRA_TN_register(OP_opnd(op,CGTARG_Copy_Operand(op))) &&

	 TN_register_class(OP_result(op,0)) ==
	 TN_register_class(OP_opnd(op,CGTARG_Copy_Operand(op)))) ||
#else
	 LRA_TN_register(OP_result(op,0)) ==
	 LRA_TN_register(OP_opnd(op,CGTARG_Copy_Operand(op)))) ||
#endif
	Is_Marked_For_Removal(op)) {
      BB_Remove_Op (bb, op);
      Reset_BB_scheduled (bb);
    }
    op = next_op;
  } while (op != NULL);
}


/******************** Routines to process avail_regs **********************/

/* ======================================================================
 * Init_Avail_Regs
 *
 * Initialize the avail_regs from the avail_set.
 * ======================================================================*/
static void
Init_Avail_Regs (void)
{
  REGISTER reg;
  ISA_REGISTER_CLASS cl;

  bzero(avail_regs, sizeof(avail_regs));
  FOR_ALL_ISA_REGISTER_CLASS(cl) {
    FOR_ALL_REGISTER_SET_members (avail_set[cl], reg) {
      avail_regs[cl].reg[reg] = TRUE;
    }
  }
}


/* ======================================================================
 * Add_Avail_Reg
 *
 * Add the register (regclass,reg) back to the avail_regs list.
 * ======================================================================*/
static void
Add_Avail_Reg (ISA_REGISTER_CLASS regclass, REGISTER reg, INT cur_op)
{
  // are we establishing fat points?
  if (Calculating_Fat_Points()) {
    if (fat_points[cur_op] != FAT_POINTS_MIN) fat_points[cur_op]--;
    //
    // infinite register set used when establishing fat points.
    //
    if (reg > REGISTER_MAX) return;
  }

  /* don't put non-allocatable registers back on the free list. */
  if (!REGISTER_allocatable(regclass, reg)) return;

  /* register excluded (by swp) */
  if (REGISTER_SET_MemberP(exclude_set[regclass], reg)) return;

  if (Do_LRA_Trace(Trace_LRA_Detail)) {
    fprintf (TFile, "Deallocated register %s\n", REGISTER_name(regclass, reg));
  }
  Is_True (!avail_regs[regclass].reg[reg], (" LRA: Error in Add_Avail_Reg"));
  avail_regs[regclass].reg[reg] = TRUE;
}


/* ======================================================================
 * Delete_Avail_Reg
 *
 * Delete the register <regclass, reg> from the avail_regs list. 
 * ======================================================================*/
inline void
Delete_Avail_Reg (ISA_REGISTER_CLASS regclass, REGISTER reg, INT cur_op)
{
  // are we establishing fat points?
  if (Calculating_Fat_Points()) {
    if (fat_points[cur_op] != FAT_POINTS_MAX) fat_points[cur_op]++;
    //
    // infinite register set used when establishing fat points.
    //
    if (reg > REGISTER_MAX) return;
  }
  avail_regs[regclass].reg[reg] = FALSE;
}


/* ======================================================================
 * Is_Reg_Available
 *
 * Return TRUE if <regclass,reg> is available for allocation to <lr>.
 * In addition to checking the avail_regs array for availability, one
 * of the following conditions must be true:
 *   1. the register has not been allocated to a global TN in this block. 
 *   2. the first definition of <lr> is after the exposed use of <reg>.
 *   3. the first definition of <lr> is the same as the exposed use of <lr>
 *      and the definition is not an OP_uniq_res.
 * ======================================================================*/
static BOOL
Is_Reg_Available (ISA_REGISTER_CLASS regclass, 
                  REGISTER_SET usable_regs, 
                  REGISTER reg, 
                  LIVE_RANGE *lr)
{
  //
  // infinite register set used when establishing fat points.
  //
  if (reg > REGISTER_MAX) {
    //
    // we should only get here if we're trying to use the result
    // register for one of the operands.  if so, then it should
    // have just been freed if its available.  otherwise, we don't
    // reuse the register numbers beyond the physical register set
    // (i.e. the "infinite" ones).  as a result, we don't really
    // keep track of their availability.
    //
    if (last_infinite_freed == reg) {
      return TRUE;
    } else {
      return FALSE;
    }
  }
  if (reg != REGISTER_UNDEFINED && 
      avail_regs[regclass].reg[reg] &&
      REGISTER_SET_MemberP(usable_regs, reg)) {
    TN *tn = Build_Dedicated_TN (regclass, reg, 0);
    LIVE_RANGE *ded_lr = LR_For_TN (tn);
    OP *op;
    INT lr_def; 

    //
    // for undefined locals, treat exposed use as first def since we
    // want to free the register immediately after the exposed use.
    //
    if (LR_Is_Undefined_Local(lr)) {
      lr_def = LR_exposed_use(lr);
    } else {
      lr_def = LR_first_def(lr);
    }
    if (ded_lr == NULL ||
	lr_def > LR_exposed_use(ded_lr) ||
	(lr_def == LR_exposed_use(ded_lr) && 
         (op = OP_VECTOR_element (Insts_Vector, LR_first_def(lr))) &&
	  !OP_uniq_res(op)))
    {
      return TRUE;
    }
  }
  return FALSE;
}


/* If the <reg> is non-allocatable, we can use it for the live-range <lr>,
 * as long as it is safe to do so.
 */
static BOOL
Is_Non_Allocatable_Reg_Available (
  ISA_REGISTER_CLASS regclass, 
  REGISTER_SET usable_regs,
  REGISTER reg,
  LIVE_RANGE *lr)
{
  if (! REGISTER_SET_MemberP(usable_regs, reg)) {
    return FALSE;
  }

  TN *tn = Build_Dedicated_TN (regclass, reg, 0);

  if (TN_is_zero_reg(tn)) {
    /* For the special case of a single definition of the live range
     * which is a copy from $0, we can assign $0 to the live range.
     */
    if (LR_def_cnt(lr) == 1) {
      OP *op = OP_VECTOR_element (Insts_Vector, LR_first_def(lr));
      if (CGTARG_Is_Preference_Copy(op) &&
	  LRA_TN_register(OP_opnd(op, CGTARG_Copy_Operand(op))) == reg)
        return TRUE;
    }
    return FALSE;
  }

  LIVE_RANGE *ded_lr = LR_For_TN (tn);

  if (!REGISTER_allocatable (regclass, reg) &&
      ded_lr != NULL &&
      LR_first_def(lr) >= LR_exposed_use(ded_lr) &&
      LR_last_use(lr) <= LR_first_def(ded_lr))
  {
    /* extend the exposed use of the non-allocatable TN */
    LR_exposed_use(ded_lr) = LR_last_use(lr);
    return TRUE;
  }
  return FALSE;
}


/* ======================================================================
 * Get_Avail_Reg
 *
 * Get a register of the given 'regclass' from the avail_regs list. 
 * We make sure that the register can be used for allocation to <lr>.
 * We also make sure that the register is not the same as <skip_reg>.
 * ======================================================================*/
static REGISTER
Get_Avail_Reg (ISA_REGISTER_CLASS regclass, 
               REGISTER_SET usable_regs, 
               LIVE_RANGE *lr, 
               REGISTER skip_reg)
{
  REGISTER next_reg = last_assigned_reg[regclass] + 1;
  REGISTER reg;

  // Get the next available register starting from the last_assigned_reg.
  // This gets registers in a round-robin fashion, which is better for 
  // the scheduling pass after LRA, since it reduces dependences created
  // due to register assignment. The following 2 loops get us the 
  // circular traversal through the registers.
  for (reg = next_reg; reg < REGISTER_MAX+1; reg++) {
    if (Is_Reg_Available(regclass, usable_regs, reg, lr) && reg != skip_reg) {
      return reg;
    }
  }
  for (reg = REGISTER_MIN; reg < next_reg; reg++) {
    if (Is_Reg_Available(regclass, usable_regs, reg, lr) && reg != skip_reg) {
      return reg;
    }
  }

#ifdef HAS_STACKED_REGISTERS
  //
  // Try a register stack register
  //
  reg = REGISTER_Request_Stacked_Register(ABI_PROPERTY_stacked, regclass);
  if (reg != REGISTER_UNDEFINED && REGISTER_SET_MemberP(usable_regs, reg)) {
    avail_set[regclass] = REGISTER_SET_Union1(avail_set[regclass], reg);
    if (Do_LRA_Trace(Trace_LRA_Spill)) {
      fprintf (TFile, 
	       "LRA_SPILL>> Got new stack register: %s %d,%d\n", 
                REGISTER_name(regclass, reg), regclass, reg);
      }
    avail_regs[regclass].reg[reg] = TRUE;
    livethrough_computed = FALSE; /* We have added a new live reg and need to re-compute. */
    return reg;
  }
#endif

  /* if not running GRA, try to get additional callee save registers */
  if (CG_localize_tns) {
    REGISTER_SET unused_callees = REGISTER_SET_Difference (
                                      REGISTER_CLASS_callee_saves(regclass), 
                                      Callee_Saved_Regs_Used[regclass]);
    unused_callees = REGISTER_SET_Intersection(unused_callees, usable_regs);
    reg = REGISTER_SET_Choose (unused_callees);
    if (reg != REGISTER_UNDEFINED) { 
      Callee_Saved_Regs_Used[regclass] = 
                    REGISTER_SET_Union1(Callee_Saved_Regs_Used[regclass], reg);
      avail_set[regclass] = REGISTER_SET_Union1 (avail_set[regclass], reg);
      if (Do_LRA_Trace(Trace_LRA_Spill)) {
        fprintf (TFile, 
          "LRA_SPILL>> Got new callee save register: %d,%d\n", regclass, reg);
      }
      avail_regs[regclass].reg[reg] = TRUE;
      return reg;
    }
  }
  if (Calculating_Fat_Points()) {
    return ++reg_infinite;
  } else {
    return REGISTER_UNDEFINED;
  }
}


/* ======================================================================
 * Allocate_Register
 *
 * Allocate a register for the TN If the argument 'uniq_result' is TRUE, 
 * exclude the register 'result_reg' from being allocated.
 * ======================================================================*/
static REGISTER
Allocate_Register (
  TN *tn, 
  BOOL uniq_result, 
  ISA_REGISTER_CLASS result_regclass,
  REGISTER_SET usable_regs,
  REGISTER result_reg,
  INT opnum)
{
  ISA_REGISTER_CLASS regclass;
  REGISTER reg;
  LIVE_RANGE *clr = LR_For_TN (tn);

  regclass = TN_register_class(tn);

  /* Check if the result_reg can be used as the register to allocate.
   * If yes, that should be our first preference since it minimizes 
   * the number of registers required for round-robin.
   */
  if (!uniq_result &&
      regclass == result_regclass &&
      Is_Reg_Available (regclass, usable_regs, result_reg, clr))
  {
    reg = result_reg;
  }
  else {
    REGISTER skip_reg;

    if (uniq_result && regclass == result_regclass) {
      skip_reg = result_reg;
    }
    else {
      skip_reg = REGISTER_UNDEFINED;
    }

    reg = Get_Avail_Reg (regclass, usable_regs, clr, skip_reg);
    if (reg == REGISTER_UNDEFINED) return REGISTER_UNDEFINED;
  }
  Delete_Avail_Reg (regclass, reg, opnum);
  if (Do_LRA_Trace(Trace_LRA_Detail)) {
    fprintf (TFile, "Allocated register %s for ", REGISTER_name (regclass, reg));
    Print_TN(tn,FALSE);fprintf(TFile,"\n");
  }
  if (Always_Trace(Trace_LRA) && trace_tn && trace_cl == regclass) {
    bb_live = TN_SET_Union1D(bb_live, tn, &lra_pool);
  }

  //
  // we only try to reuse real physical registers when calculating
  // fat points. 
  //
  if (reg <= REGISTER_MAX) {
    last_assigned_reg[regclass] = reg;
  }

  return reg;
}


//
// if we're not going to reschedule, set up data structures to calculate
// fat point data for the instructions above the failure point.
//
static BOOL
Init_Fat_Point_Calculation(ISA_REGISTER_CLASS cl, INT opnum, BB *bb)
{
  //
  // don't bother calculating fat points if we're just going to
  // reschedule
  //
  if (Check_Allow_Reschedule()) {
    return(FALSE);
  }
  fat_point_regs_map = hTN_MAP_Create (&lra_pool);
  use_fat_point_regs = TRUE;
  fat_points = TYPE_MEM_POOL_ALLOC_N(FAT_POINTS_TYPE, &lra_pool, BB_length(bb) + 1);
  bzero(fat_points, (BB_length(bb)+1)*sizeof(FAT_POINTS_TYPE));
  failing_class = cl;
  reg_infinite = REGISTER_MAX;

  return(TRUE);
}

//
// clear out global data for fat point calculation
//
static void
Clear_Fat_Point_Calculation()
{
  fat_points = NULL;
  failing_class = ISA_REGISTER_CLASS_UNDEFINED;
  reg_infinite = REGISTER_UNDEFINED;
  use_fat_point_regs = FALSE;
}


//
// update availability data structures for used callee saved registers in
// entry and exit blocks.  we must not have any callee saved register used
// above the stack pointer adjustment in the entry block, or below it in
// the exit block (as otherwise, the use will be on the wrong side of the
// saves/restores).
//
static void
Update_Callee_Availability(BB *bb)
{
  ISA_REGISTER_CLASS cl;
  REGISTER reg;
  if (BB_exit(bb)) {
    //
    // can't allow callee saved registers to be used below stack adjustment
    // in exit block (restores are above it).
    //
    FOR_ALL_ISA_REGISTER_CLASS(cl) {
      avail_set[cl] = REGISTER_SET_Union(avail_set[cl],
					 Callee_Saved_Regs_Used[cl]);
    }
    FOR_ALL_REGISTER_SET_members(Callee_Saved_Regs_Used[cl], reg) {
      avail_regs[cl].reg[reg] = TRUE;
    }	  
  } else if (BB_entry(bb)) {
    //
    // can't allow callee saved registers above stack adjustment in
    // entry block (saves are below it).
    //
    FOR_ALL_ISA_REGISTER_CLASS(cl) {
      avail_set[cl] = REGISTER_SET_Difference(avail_set[cl],
					      Callee_Saved_Regs_Used[cl]);
    }
    FOR_ALL_REGISTER_SET_members(Callee_Saved_Regs_Used[cl], reg) {
      avail_regs[cl].reg[reg] = FALSE;
    }	  
  }
}


// Compute the register set that satisfies target-specific constraints
// for the use of <tn> within its live range <lr>. There are two kinds
// of constraints:
// 
// 1) If <tn> is used as an operand or a result from a particular
//    register subclass, then it must be assigned a register that
//    belongs to that cubclass.
// 
// 2) If <tn> is used in an OP that has an operand or a result
//    (different from <tn>) that requires a specific register,
//    then that register must not be assigned to <tn>.
//
static REGISTER_SET
Usable_Registers (TN* tn, LIVE_RANGE* lr)
{
  ISA_REGISTER_CLASS cl = TN_register_class(tn);
  REGISTER_SET usable_regs = REGISTER_CLASS_universe(cl);

  INT first_op = MAX(LR_first_def(lr), 1);
  INT last_op = MIN(LR_last_use(lr)+1, VECTOR_size(Insts_Vector));

  for (INT opnum = first_op; opnum < last_op; opnum++) {  

    OP* op = OP_VECTOR_element(Insts_Vector, opnum);

    ASM_OP_ANNOT* asm_info = (OP_code(op) == TOP_asm) ?
      (ASM_OP_ANNOT*) OP_MAP_Get(OP_Asm_Map, op) : NULL;

    // cannot use registers clobbered by an ASM statement
    if (asm_info) {
      usable_regs = REGISTER_SET_Difference(usable_regs, 
                                            ASM_OP_clobber_set(asm_info)[cl]);
    }

    for (INT resnum = 0; resnum < OP_results(op); resnum++) {
      if (OP_result_reg_class(op, resnum) != cl) {
        continue;
      }
      ISA_REGISTER_SUBCLASS sc = asm_info ?
        ASM_OP_result_subclass(asm_info)[resnum] :
        OP_result_reg_subclass(op, resnum);
      if (sc == ISA_REGISTER_SUBCLASS_UNDEFINED) {
        continue;
      }
      REGISTER_SET subclass_regs = REGISTER_SUBCLASS_members(sc);
      TN* result = OP_result(op, resnum);
      if (result == tn) {
        usable_regs = REGISTER_SET_Intersection(usable_regs, subclass_regs);
      }
      else if (REGISTER_SET_Size(subclass_regs) == 1) {
        usable_regs = REGISTER_SET_Difference(usable_regs, subclass_regs);
      }
    }
          
    for (INT opndnum = 0; opndnum < OP_opnds(op); opndnum++) {
      if (OP_opnd_reg_class(op, opndnum) != cl) {
        continue;
      }
      ISA_REGISTER_SUBCLASS sc = asm_info ?
        ASM_OP_opnd_subclass(asm_info)[opndnum] :
        OP_opnd_reg_subclass(op, opndnum);
      if (sc == ISA_REGISTER_SUBCLASS_UNDEFINED) {
        continue;
      }
      REGISTER_SET subclass_regs = REGISTER_SUBCLASS_members(sc);
      TN* opnd = OP_opnd(op, opndnum);
      if (opnd == tn) {
        usable_regs = REGISTER_SET_Intersection(usable_regs, subclass_regs);
      }
      else if (REGISTER_SET_Size(subclass_regs) == 1) {
        usable_regs = REGISTER_SET_Difference(usable_regs, subclass_regs);
      }
    }
  }          
  
  FmtAssert(!REGISTER_SET_EmptyP(usable_regs),
            ("Could not find a register for TN%d in live range [%d-%d]",
              TN_number(tn), LR_first_def(lr), LR_last_use(lr)));

  return usable_regs;
}


static BOOL
Assign_Registers_For_OP (OP *op, INT opnum, TN **spill_tn, BB *bb)
{
  INT opndnum;
  INT resnum;
  REGISTER reg, result_reg;
  ISA_REGISTER_CLASS result_cl;
  BOOL uniq_result;
  LIVE_RANGE *clr;
  BOOL result_failed = FALSE;
  INT nresults = OP_results(op);
  mBOOL *free_result = TYPE_ALLOCA_N(mBOOL, nresults);
  INT *free_opnum = TYPE_ALLOCA_N(INT, nresults);
  TN **free_result_tn = TYPE_ALLOCA_N(TN *, nresults);
  REGISTER *free_result_reg = TYPE_ALLOCA_N(REGISTER, nresults);
  ISA_REGISTER_CLASS *free_result_cl = TYPE_ALLOCA_N(ISA_REGISTER_CLASS, nresults);
  for (resnum = 0; resnum < nresults; resnum++) {
    free_result[resnum] = FALSE;
  }

  //TODO: change trace to give fat point calculation, not this
  if (Do_LRA_Trace(Trace_LRA_Detail)) {
    fprintf (TFile, "OP:%d>> ", opnum);
    Print_OP_No_SrcLine (op);
  }

  // get ASM OP annotation
  ASM_OP_ANNOT* asm_info = (OP_code(op) == TOP_asm) ?
    (ASM_OP_ANNOT*) OP_MAP_Get(OP_Asm_Map, op) : NULL;
  
  //
  // Process the result TN. If this is the first def of the live range,
  // put the register back on the free list. If the OP has the 
  // "uniq_res" attribute, keep track of this result_reg and do not
  // assign it to any of the operands of this OP.
  //
  uniq_result = FALSE;
  result_reg = REGISTER_UNDEFINED;
  result_cl = ISA_REGISTER_CLASS_UNDEFINED;
  for (resnum = 0; resnum < nresults; resnum++) {
    TN *result_tn = OP_result (op,resnum);

    clr = LR_For_TN (result_tn);
    result_reg = LRA_TN_register(result_tn);
    result_cl = TN_register_class(result_tn);
    REGISTER_SET must_use = Usable_Registers(result_tn, clr);

    //
    // no need to look at result if we're calculating fat points, and
    // this result is not of the register class that we'll be spilling.
    //
    if (!Calculating_Fat_Points() ||
	failing_class == result_cl) {

      // Check if a register is assigned to the result tn. The only
      // case it may not have a register is if it is an unused def that
      // was not deleted.
      if (result_reg == REGISTER_UNDEFINED) {
	REGISTER prefer_reg = LR_prefer_reg (clr);

        if (prefer_reg == REGISTER_UNDEFINED) {
          if (TN_is_local_reg(result_tn) && (unused_tn_def[result_cl] != NULL) && !OP_side_effects(op)) {
            result_tn = unused_tn_def[result_cl];
            Set_OP_result(op,resnum,result_tn);

            if (Do_LRA_Trace(Trace_LRA_Detail)) {
              fprintf(TFile,"unused def BB:%d %s >>> ",
                      BB_id(bb),REGISTER_name (result_cl, LRA_TN_register(result_tn)));
              Print_OP_No_SrcLine (op);
            }

            if (!OP_has_implicit_interactions (op) &&
                !OP_store(op) &&
                !CGTARG_Is_OP_Intrinsic(op)) {
             /* If all the results are unneeded, delete the op. */
              INT k;
              BOOL all_results_unneeded = TRUE;
              for (k = OP_results(op); 0 < k; k--) {
                TN *ck_tn = OP_result (op,k-1);
                ISA_REGISTER_CLASS ck_cl;
                if (ck_tn != NULL) {
                  ck_cl = TN_register_class(ck_tn);
                  if ((unused_tn_def[ck_cl] == NULL) ||
                      (unused_tn_def[ck_cl] != ck_tn)) {
                    all_results_unneeded = FALSE;
                    break;
                  }
                }
              }
              if (all_results_unneeded) {
                if (Do_LRA_Trace(Trace_LRA_Detail)) {
                  fprintf (TFile, "DELETE OP(unneeded results):%d>> ", opnum);
                  Print_OP_No_SrcLine (op);
                }

                BB_Remove_Op (bb, op);
                return TRUE;
              }
            }

            continue;
          }
        }

	if (prefer_reg != REGISTER_UNDEFINED &&
	    (Is_Reg_Available (result_cl, must_use, prefer_reg, clr) ||
	     Is_Non_Allocatable_Reg_Available(result_cl, must_use, prefer_reg, clr))){
	  result_reg = prefer_reg;
	  Delete_Avail_Reg (result_cl, result_reg, opnum);
	  if (Do_LRA_Trace(Trace_LRA_Detail)) {
	    fprintf (TFile, "Preferred register %s\n",
		     REGISTER_name (result_cl, prefer_reg));
	  }
	  if (Always_Trace(Trace_LRA) && trace_tn && result_cl == trace_cl) {
	    bb_live = TN_SET_Union1D(bb_live, result_tn, &lra_pool);
	  }
	} else {
	  result_reg =
	    Allocate_Register(result_tn,FALSE, result_cl, must_use, result_reg,opnum);
	  if (result_reg == REGISTER_UNDEFINED) {
	    *spill_tn = result_tn;
	    if (!Calculating_Fat_Points()) {
	      //
	      // continue going through the loop with the purpose of gathering
	      // fat point info, if appropriate.  if so, redo failing op.
	      //
	      if (Init_Fat_Point_Calculation(TN_register_class(*spill_tn),
					     opnum,bb)){
		result_reg = Allocate_Register(result_tn, FALSE, result_cl,
					       must_use, result_reg, opnum);
		result_failed = TRUE;
	      } else {
		return FALSE;
	      }
	    }
	    FmtAssert(result_reg != REGISTER_UNDEFINED,
		      ("LRA: no register found during fat point calculation.\n"
		       ));
	  }
	}
	LRA_TN_Allocate_Register (result_tn, result_reg);
      } else if (result_reg == REGISTER_sp && CG_localize_tns) {
	Update_Callee_Availability(bb);
      } 

      // it's always safe to free a result register for non-ASM OPs
      // for ASM OPs it's only OK if the result is not early_clobber
      bool ok_to_free_result = 
        !asm_info || !ASM_OP_result_clobber(asm_info)[resnum];
        
      if (ok_to_free_result && opnum == LR_first_def(clr)) {

/*
 * Remember all the information needed to free registers.
 * They can't be freed now because they may accidently be
 * reassigned for use as another result in the same OP.
 * (This can happen if one of the results is not used.)
 */
        free_opnum[resnum] = opnum;
        free_result_tn[resnum] = result_tn;
        free_result[resnum] = TRUE;
        free_result_reg[resnum] = result_reg;
        free_result_cl [resnum] = result_cl;
      }

      if (OP_uniq_res(op)) uniq_result = TRUE;
    }
  }

/*
 * NOW, free up the result registers for re-use.
 */
  BOOL assignment_undone = FALSE;
  for (resnum = 0; resnum < nresults; resnum++) {
    if (free_result[resnum]) {
        assignment_undone = TRUE;
	Add_Avail_Reg (free_result_cl[resnum], free_result_reg[resnum], free_opnum[resnum]);
	if (Always_Trace(Trace_LRA) && trace_tn && (free_result_cl[resnum] == trace_cl))  {
	  bb_live = TN_SET_Difference1D(bb_live, free_result_tn[resnum]);
	  if (TN_number(free_result_tn[resnum]) == trace_tn) {
	    LRA_Print_Liveness();
	  }
	}
    }
  }
  if (assignment_undone && result_failed) {
   // set fat_points for this guy to one.  these weird things will 
   // get allocated then free'd, so Fix_LRA_Blues will flip because
   // it'll see a spill required for an instruction that needs 
   // no registers (i.e. fat_points is 0 at the failure point).
   // should only happen in rare occasions such as when wopt is
   // run, but optimization is turned off in cg.
    fat_points[opnum] = 1;
  }

  //
  // process all the operand TNs.
  //
  for (opndnum = 0; opndnum < OP_opnds(op); opndnum++) {
    TN *tn = OP_opnd(op, opndnum);

    // If it is an invalid TN or constant TN  or it has already been 
    // assigned a register, continue.
    if (tn == NULL || 
        TN_is_constant(tn) ||
        LRA_TN_register(tn) != REGISTER_UNDEFINED) 
    {
      continue;
    }

    clr = LR_For_TN (tn);

    REGISTER prefer_reg = LR_prefer_reg (clr);
    ISA_REGISTER_CLASS regclass = TN_register_class(tn);
    REGISTER_SET must_use = Usable_Registers(tn, clr);
    
    //
    // no need to look at operands that aren't of the class
    // being spilled.
    //
    if (Calculating_Fat_Points() &&
	failing_class != regclass) {
      continue;
    }
    if (prefer_reg != REGISTER_UNDEFINED &&
	(!uniq_result || (prefer_reg != result_reg)) &&
        (Is_Reg_Available (regclass, must_use, prefer_reg, clr) ||
         Is_Non_Allocatable_Reg_Available (regclass, must_use, prefer_reg, clr)))
    {
      LRA_TN_Allocate_Register (tn, prefer_reg);
      Delete_Avail_Reg (TN_register_class(tn), prefer_reg, opnum);
      if (Do_LRA_Trace(Trace_LRA_Detail)) {
	fprintf (TFile, "Preferred register %s\n",
		 REGISTER_name (regclass, prefer_reg));
      }
      if (Always_Trace(Trace_LRA) && trace_tn &&
	  TN_register_class(tn) == trace_cl ) {
	bb_live = TN_SET_Union1D(bb_live, tn, &lra_pool);
      }
      continue;
    }

    // This tn is either the last use of a live range or an exposed use.
    // Pick a register from the free list and assign it to the TN. 
    // NOTE1: We also check if the result of the OP needs to be unique.
    // If yes, make sure we don't pick the same register as the result.
    reg = Allocate_Register (tn, uniq_result, result_cl, must_use, result_reg, opnum);
    if (reg == REGISTER_UNDEFINED) {
      *spill_tn = tn;
      if (!Calculating_Fat_Points()) {
	//
	// continue going through the loop with the purpose of gathering
	// fat point info, if appropriate.  if so, redo failing op.
	//
	if (Init_Fat_Point_Calculation(TN_register_class(*spill_tn),opnum,bb)){
	  fat_points[opnum] = 0;
	  reg = Allocate_Register (tn, uniq_result, result_cl, must_use, result_reg,
				   opnum);
	} else {
	  return FALSE;
	}
      }
      FmtAssert(reg != REGISTER_UNDEFINED,
		("LRA: no register found during fat point calculation.\n"));
    }

    LRA_TN_Allocate_Register (tn, reg);

    // If the register we allocated was for an exposed use of a local TN,
    // we deallocate it immediately, since we don't want to block the 
    // register.
    if (opnum <= LR_exposed_use(clr)) {
      Add_Avail_Reg (TN_register_class(tn), reg, opnum);
      if (Always_Trace(Trace_LRA) && trace_tn &&
	  TN_register_class(tn) == result_cl)  {
	bb_live = TN_SET_Difference1D(bb_live, tn);
	if (TN_number(tn) == trace_tn) {
	  LRA_Print_Liveness();
	}
      }
    }
  }

  return TRUE;
}

/* ======================================================================
 * Check_Undefined_Results
 * 
 * See if all results of an op have no use.
 *
 * ======================================================================*/
static BOOL
Check_Undefined_Results(OP* op)
{
  BOOL defined = FALSE;
  for (INT i = OP_results(op) - 1; i >= 0; --i) {
    if (LRA_TN_register(OP_result(op,i)) != REGISTER_UNDEFINED) {
      defined = TRUE;
      break;
    }
  }
  return !defined;
}

/* ======================================================================
 * Classes_Match
 * 
 * Check that the register class being currently allocated is the same
 * as the result register class of this instruction.  The problem we
 * currently have is with multiple result instructions.  If we have an
 * instruction that writes registers of multiple classes, then we can't
 * easily delete the instruction.  I don't think at present it is an issue
 * with any architecture that we support, so we'll punt on that for now.
 * Such things would probably have to be handled by EBO anyway since we're
 * allocating one class at a time, and detecting that multiple results of
 * different classes are all not used would be messy to say the least.
 *
 * ======================================================================*/
static BOOL
Classes_Match(OP* op)
{
  for (INT i = OP_results(op) - 1; i >= 0; --i) {
    if (TN_register_class(OP_result(op, i)) != failing_class) {
      return FALSE;
    }
  }
  return TRUE;
}

/* ======================================================================
 * Assign_Registers
 *
 * This pass through the OPs allocates registers for all the 
 * local TNs in the basic block. It returns TRUE if successful.
 * ======================================================================*/
static BOOL
Assign_Registers (BB *bb, TN **spill_tn, BOOL *redundant_code)
{
  if (Do_LRA_Trace(Trace_LRA_Detail)) {
    fprintf (TFile, "--------------------------------------------\n");
    fprintf (TFile, "Register Assignment for BB%d\n", BB_id(bb));
    fprintf (TFile, "--------------------------------------------\n");
  }

  for (INT opnum = BB_length(bb); opnum > 0; opnum--) 
  {
    OP *op = OP_VECTOR_element (Insts_Vector, opnum);

    //
    // if we're calculating fatpoints, start out with the
    // value of the last instruction seen in the backward
    // walk.
    //
    if (Calculating_Fat_Points()) {
      fat_points[opnum] = fat_points[opnum+1];
    }

    //
    // Check if we have a redundant definition. If yes, try to delete it.
    // A definition is known to be undefined because we are doing a backward
    // walk, and if it has not been allocated a register then it must be
    // redundant.  if we're calculating fat points, make sure we only delete
    // instructions of the appropriate class (these are the only one's that
    // are being "assigned" registers).
    //
    if (CG_opt_level > 0 && 
        OP_has_result(op) &&
	Check_Undefined_Results(op) &&
	(!Calculating_Fat_Points() ||
	 Classes_Match(op)) &&
	!OP_has_implicit_interactions (op) &&
        !OP_store(op) &&
        !CGTARG_Is_OP_Intrinsic(op) &&
        !Get_Trace (TP_ALLOC, 0x400))
    {
      // Go through all the operands of this <op> and update the 
      // LR_use_cnt and LR_last_use fields for their live ranges.
      for (INT opndnum = 0; opndnum < OP_opnds(op); opndnum++) {
        TN *tn = OP_opnd(op, opndnum);

        // If it is an invalid TN or constant TN  or it has already been 
        // assigned a register, continue.
        if (tn == NULL || 
            TN_is_constant(tn) ||
            LRA_TN_register(tn) != REGISTER_UNDEFINED) 
        {  
          continue;
        }
        LIVE_RANGE *opnd_lr = LR_For_TN (tn);
        LR_use_cnt(opnd_lr)--;
        if (LR_use_cnt(opnd_lr) != 0) {
          // udpate the LR_last_use field.1
          for (INT pnum = opnum-1; pnum > 0; pnum--) {
            OP *prev_op = OP_VECTOR_element (Insts_Vector, pnum);
            if (OP_Refs_TN (prev_op, tn)) {
              LR_last_use(opnd_lr) = pnum;
              break;
            }
          }
        }
      }
      if (Do_LRA_Trace(Trace_LRA_Detail)) {
        fprintf (TFile, "DELETE OP:%d>> ", opnum);
        Print_OP_No_SrcLine (op);
      }
      // Now delete the unused definition.
      if (!Calculating_Fat_Points()) {
	BB_Remove_Op (bb, op);
      } else {
	//
	// mark the op for removal, and convert it to a noop so that
	// it no longer effects allocation
	//
	Mark_For_Removal(op);
	*redundant_code = TRUE;
      }
      continue;
    }

    if (!Assign_Registers_For_OP (op, opnum, spill_tn, bb)) {
      return FALSE;
    }

    // Check if the instruction is a self copy.  don't care about this
    // if calculating fat points
    if (!Calculating_Fat_Points() &&
        (Is_Marked_For_Removal(op) ||
         (CGTARG_Is_Preference_Copy(op) &&
	  LRA_TN_register(OP_result(op,0)) ==
	          LRA_TN_register(OP_opnd(op, CGTARG_Copy_Operand(op)))))) {
      *redundant_code = TRUE;
    }
  }

  if (Calculating_Fat_Points()) {
    failing_class = ISA_REGISTER_CLASS_UNDEFINED;
    return FALSE;
  }
  return TRUE;
}


/***************************** Spill *****************************************/


/* Compute the set of global registers that are live through the bb. */
static void
Compute_Livethrough_Set (BB *bb)
{
  OP *op;
  ISA_REGISTER_CLASS cl;

  FOR_ALL_ISA_REGISTER_CLASS(cl) {
    livethrough[cl] = REGISTER_SET_Difference (
                                    REGISTER_CLASS_allocatable(cl),
                                    avail_set[cl]);
  }
  FOR_ALL_BB_OPs_FWD (bb, op) {
    INT opndnum;
    INT resnum;
    for (opndnum = 0; opndnum < OP_opnds(op); opndnum++) {
      TN *tn = OP_opnd(op, opndnum);
      if (tn != NULL && 
          TN_is_register(tn) &&
          (TN_is_dedicated(tn) | TN_is_global_reg(tn)))
      {
        ISA_REGISTER_CLASS cl = TN_register_class(tn);
        REGISTER reg = LRA_TN_register(tn);
        livethrough[cl] = REGISTER_SET_Difference1 (livethrough[cl], reg);
      }
    }
    for (resnum = 0; resnum < OP_results(op); resnum++) {
      TN *tn = OP_result(op, resnum);
      if (TN_is_dedicated(tn) | TN_is_global_reg(tn)) {
        ISA_REGISTER_CLASS cl = TN_register_class(tn);
        REGISTER reg = LRA_TN_register(tn);
        livethrough[cl] = REGISTER_SET_Difference1 (livethrough[cl], reg);
      }
    }
  }
}


static void
Analyze_Spilling_Global_Register (
  ISA_REGISTER_CLASS cl, 
  SPILL_CANDIDATE *best, 
  INT spill_opnum, 
  INT fatpoint)
{
  /* TODO: Try getting a caller save register from GRA's list. This is better
   * because the save/restore does not have be tracked for the .debug_info
   * information. 
   */
  REGISTER reg = REGISTER_SET_Choose (livethrough[cl]);
  if (reg != REGISTER_UNDEFINED) {
    float store_cost, restore_cost;
    INT benefit = 0;
    TN *spill_tn = Build_Dedicated_TN (cl, reg, 0);
    CGSPILL_Cost_Estimate (spill_tn, NULL, &store_cost, &restore_cost, CGSPILL_LRA);
    best->cost = store_cost + restore_cost;     
    best->spill_kind = SPILL_GLOBAL_REG;
    best->u1.s1.global_spill_reg = reg;
    best->u1.s1.spill_cl = cl;
    for (INT i = 1; i <= spill_opnum; i++) {
      if (fat_points[i] >= fatpoint) benefit++;
    }
    best->benefit = benefit;
    if (Do_LRA_Trace(Trace_LRA_Spill)) {
      fprintf (TFile, "Analyze >> (cost:%g, benefit:%d) spill register %s\n", 
        best->cost, benefit, REGISTER_name (cl, reg));
    }
  }
}

/* ======================================================================
 * Spill_Global_Register
 *
 * Pick one of the registers from GRA's list and spill it at the start
 * of the basic block and restore it at the bottom of the basic block.
 * ======================================================================*/
static void
Spill_Global_Register (BB *bb, SPILL_CANDIDATE *best)
{
  OPS spill_ops;
  TN *new_tn;
  REGISTER reg = best->u1.s1.global_spill_reg;
  ISA_REGISTER_CLASS cl = (ISA_REGISTER_CLASS)best->u1.s1.spill_cl;

  if (Do_LRA_Trace(Trace_LRA_Spill)) {
    fprintf (TFile, "LRA_SPILL>> Spilled Global Register : %s\n", 
                    REGISTER_name (cl, reg));
  }

  /* TODO: Don't use the dedicated tn to store/restore, use a proxy tn 
   * instead.
   */
  TN *spill_tn = Build_Dedicated_TN (cl, reg, 0);
  ST *spill_loc;
  ST *sv_spill_location = NULL;
  BOOL magic_spill_used = FALSE;

  if (Trip_Count == MAX_TRIP_COUNT &&
      Magic_Spill_Location != NULL &&
      !TN_is_float(spill_tn)) 
  {
    sv_spill_location = TN_spill(spill_tn);
    spill_loc = Magic_Spill_Location;
    Set_TN_spill(spill_tn, spill_loc);
    magic_spill_used = TRUE;
    DevWarn ("Used short-offset spill location in BB%d", BB_id(bb));
  }
  else {
    spill_loc = CGSPILL_Get_TN_Spill_Location (spill_tn, CGSPILL_LGRA);
  }

  CGSPILL_Store_To_Memory (spill_tn, spill_loc, OPS_Init(&spill_ops),
			   CGSPILL_LRA, bb);
  CGSPILL_Prepend_Ops (bb, &spill_ops);
  new_tn = Build_TN_Like (spill_tn);
  Set_TN_spill(new_tn, spill_loc);
  CGSPILL_Load_From_Memory (new_tn, spill_loc, OPS_Init(&spill_ops),
			    CGSPILL_LRA, bb);
  CGSPILL_Append_Ops (bb, &spill_ops);
  Set_TN_is_global_reg (new_tn);
  LRA_TN_Allocate_Register (new_tn, reg);

  if (magic_spill_used) {
    Set_TN_spill(spill_tn, sv_spill_location);
    Magic_Spill_Location = NULL;
  }
}

static BOOL
Can_Use_Be_Moved (
  LIVE_RANGE *lr, 
  INT spill_opnum,
  INT *moveto)
{
  ARC_LIST *arcs;
  INT use_opnum = LR_last_use(lr);
  OP *use_op = OP_VECTOR_element (Insts_Vector, use_opnum);
  ISA_REGISTER_CLASS cl = TN_register_class (LR_tn(lr));
  INT tgt_opnum = LR_first_def(lr);

  // If there is no def in the live range lr>, tgt_opnum is 0.
  if (tgt_opnum == 0 || OP_flag1 (use_op) || OP_xfer(use_op)) return FALSE;

  for (arcs = OP_preds(use_op); arcs != NULL; arcs = ARC_LIST_rest(arcs)) {
    ARC *arc = ARC_LIST_first(arcs);
    OP *pred_op = ARC_pred(arc);
    // Prevent moving above a just moved def:
    if (OP_flag1(pred_op)) return FALSE;
    INT pred_opnum = BB_OP_MAP32_Get (op_to_opnum, pred_op);
    // Determine the last definition of an input to the move candidate:
    if (pred_opnum > tgt_opnum) tgt_opnum = pred_opnum;
  }

  /* If there is more than one use in the live range, find the 
   * second last use. There is no point moving the last use 
   * above this point.
   */
  if (LR_use_cnt(lr) > 1) {
    for (INT i = use_opnum-1; i > tgt_opnum; i--) {
      OP *op = OP_VECTOR_element (Insts_Vector, i);
      if (OP_Refs_TN (op, LR_tn(lr))) {
        tgt_opnum = i;
        break;
      }
    }
  }

  /* check if the tgt_opnum has already been marked as being moved. */
  while (tgt_opnum < spill_opnum) {
    OP *op = OP_VECTOR_element (Insts_Vector, tgt_opnum);
    if (!OP_flag1(op)) break;
    tgt_opnum++;
  }

  if (tgt_opnum >= (spill_opnum - 1)) return FALSE;

  INT regs_needed = 0;
  INT i;
  for (i = 0; i < OP_results(use_op); i++) {
    TN *tn = OP_result(use_op, i);
    if (!TN_is_const_reg(tn) && (TN_register_class(tn) == cl)) {
      LIVE_RANGE *result_lr = LR_For_TN (tn);
      if (LR_first_def(result_lr) == use_opnum)
        regs_needed--;
    }
  }
  for (i = 0; i < OP_opnds(use_op); i++) {
    TN *tn = OP_opnd(use_op, i);
    if (!TN_is_const_reg(tn) && TN_is_register(tn) && (TN_register_class(tn) == cl)) {
       LIVE_RANGE *lr = LR_For_TN (tn);
       if (LR_last_use(lr) == use_opnum) regs_needed++;
    }
  }
  if (regs_needed <= 0) return FALSE;

  *moveto = tgt_opnum;
  return TRUE;
}


/* Check if it is legal and beneficial to move the instruction that is
 * the first definition of the live range <lr>. 
 */
static BOOL
Can_Def_Be_Moved (
  LIVE_RANGE *lr,
  INT spill_opnum,
  INT *moveto)
{
  ARC_LIST *arcs;
  INT def_opnum = LR_first_def(lr);
  OP *def_op = OP_VECTOR_element (Insts_Vector, def_opnum);
  ISA_REGISTER_CLASS cl = TN_register_class (LR_tn(lr));
  INT tgt_opnum = LR_last_use(lr);

  if (OP_side_effects(def_op)) return FALSE;
  if (OP_flag1 (def_op)) return FALSE;
  if (LR_def_cnt(lr) != 1) return FALSE;
  if (tgt_opnum <= spill_opnum) return FALSE;

  for (arcs = OP_succs(def_op); arcs != NULL; arcs = ARC_LIST_rest(arcs)) {
    ARC *arc = ARC_LIST_first(arcs);
    OP *succ_op = ARC_succ(arc);
    // Prevent moving below a just moved use:
    if (OP_flag1(succ_op)) return FALSE;
    INT succ_opnum = BB_OP_MAP32_Get (op_to_opnum, succ_op);
    // Determine the first use after this def:
    if (succ_opnum < tgt_opnum) tgt_opnum = succ_opnum;
  }

  INT i;
  for (i = 0; i < OP_opnds(def_op); i++) {
    TN *tn = OP_opnd(def_op, i);

    if (!TN_is_const_reg(tn)) {
      /* if an operand is an exposed use of a result tn, moving the 
       * definition is of no use.
       */
      for (INT j = 0; j < OP_results(def_op); j++) {
        if (OP_result(def_op, j) == tn) return FALSE;
      }

      /* adjust the tgt_opnum to be MIN(tgt_opnum, opnd_last_use) */
      if (TN_is_register(tn) && TN_register_class(tn) == cl) {
         LIVE_RANGE *opnd_lr = LR_For_TN (tn);
         INT opnd_last_use = LR_last_use(opnd_lr);
         if (opnd_last_use < tgt_opnum) tgt_opnum = opnd_last_use;
      }
    }
  }

  /* check if the tgt_opnum has already been marked as being moved. */
  if (tgt_opnum != VECTOR_count(Insts_Vector)) {
    while (tgt_opnum > spill_opnum) {
      OP *op = OP_VECTOR_element (Insts_Vector, tgt_opnum);
      if (!OP_flag1(op)) break;
      tgt_opnum--;
    }
  }

  if (tgt_opnum <= spill_opnum) return FALSE;

  *moveto = tgt_opnum;
  return TRUE;
}


/* Check if it is legal and beneficial to reorder some instructions in
 * the live range. This can either mean moving the definition down OR
 * moving the last use of a live range up. This effectively shortens
 * the live range.
 */
static void 
Analyze_Reordering (
  LIVE_RANGE *lr, 
  SPILL_CANDIDATE *best,
  INT spill_opnum)
{
  INT moveto;

  INT use_opnum = LR_last_use(lr);

  if (LR_use_cnt(lr) != 0 &&
      TN_is_local_reg(LR_tn(lr)) &&
      Can_Use_Be_Moved (lr, spill_opnum, &moveto) &&
      (best->spill_kind == SPILL_NONE || 
       best->benefit < (spill_opnum - moveto)))
  {
    best->spill_kind = SPILL_MOVE_USE;
    best->u1.s2.move_lr = lr;
    best->u1.s2.from = use_opnum;
    best->u1.s2.to = moveto;
    best->cost = 0.0;
    best->benefit = spill_opnum - moveto;
  }

  INT def_opnum = LR_first_def(lr);

  if (LR_def_cnt(lr) != 0 &&
      def_opnum < spill_opnum &&
      Can_Def_Be_Moved (lr, spill_opnum, &moveto) &&
      (best->spill_kind == SPILL_NONE ||
       best->benefit < (spill_opnum - def_opnum)))
  {
    best->spill_kind = SPILL_MOVE_DEF;
    best->u1.s2.move_lr = lr;
    best->u1.s2.from = def_opnum;
    best->u1.s2.to = moveto;
    best->cost = 0.0;
    best->benefit = spill_opnum - def_opnum;
  }

  /* TODO: look at the case where it is not enough to move just one end
   * of the live range. We can try moving the whole live range in that
   * case.
   */
}

static void
Move_Def (BB *bb, SPILL_CANDIDATE *best)
{
  INT from = best->u1.s2.from;
  INT to = best->u1.s2.to;
  OP *from_op = OP_VECTOR_element (Insts_Vector, from);

  if (Do_LRA_Trace(Trace_LRA_Spill)) {
    fprintf (TFile, "LRA_SPILL>> move def %d to before %d\n", from, to);
  }
  BB_Remove_Op (bb, from_op);
  if (to < VECTOR_size(Insts_Vector)) {
    OP *to_op = OP_VECTOR_element (Insts_Vector, to);
    BB_Insert_Op_Before (bb, to_op, from_op);
  }
  else {
    OPS ops = OPS_EMPTY;
    OPS_Append_Op (&ops, from_op);
    CGSPILL_Append_Ops (bb, &ops);
  }
}


static void
Move_Use (BB *bb, SPILL_CANDIDATE *best)
{
  INT from = best->u1.s2.from;
  INT to = best->u1.s2.to;
  OP *from_op = OP_VECTOR_element (Insts_Vector, from);
  OP *to_op = OP_VECTOR_element (Insts_Vector, to);

  if (Do_LRA_Trace(Trace_LRA_Spill)) {
    fprintf (TFile, "LRA_SPILL>> move use %d to after %d\n", from, to);
  }
  BB_Remove_Op (bb, from_op);
  BB_Insert_Op_After (bb, to_op, from_op);
}


// Go through the list of move <transformations> for this basic block
// and apply them all in one pass.
static void 
Apply_Move_Transformations ( BB *bb, SPILL_CANDIDATE *transformations)
{
  SPILL_CANDIDATE *cand;

  for (cand = transformations; cand != NULL; cand = cand->next) {
    switch (cand->spill_kind) {
      case SPILL_MOVE_DEF:
        Move_Def (bb, cand);
        break;
      case SPILL_MOVE_USE:
        Move_Use (bb, cand);
        break;
    }
  }
}


/* Remove the live ranges for the result tn and the operand tns for
 * the OP being moved.
 */
static void
Remove_LRs_For_OP (OP *op)
{
  INT i;
  if (op == NULL) return;
  for (i = 0; i < OP_results(op); i++) {
    VECTOR_Delete_Element (Live_LRs_Vector, LR_For_TN (OP_result(op, i)));
  }
  for (i = 0; i < OP_opnds(op); i++) {
    TN *opnd_tn = OP_opnd(op,i);
    if (TN_is_register(opnd_tn)) {
      VECTOR_Delete_Element (Live_LRs_Vector, LR_For_TN(opnd_tn));
    }
  }
}


static void
Update_Fat_Points (SPILL_CANDIDATE *best, INT spill_opnum)
{
  LIVE_RANGE *lr;
  INT low;
  BOOL global_lr_spill;

  switch (best->spill_kind) {
  case SPILL_LIVE_RANGE:
    lr = best->u1.spill_lr;
    global_lr_spill = !TN_is_local_reg(LR_tn(lr));
    if (LR_exposed_use(lr)) {
      low = LR_first_spill(lr);
    } else {
      low = LR_first_def(lr) + 1;
    }
    VECTOR_Delete_Element (Live_LRs_Vector, lr);
    break;

  case SPILL_MOVE_DEF:
  case SPILL_MOVE_USE:
    OP *from_op = OP_VECTOR_element (Insts_Vector, best->u1.s2.from);
    Remove_LRs_For_OP (from_op);
    /* mark this flag as having been moved. */
    Set_OP_flag1 (from_op);
    if (best->u1.s2.to != VECTOR_count(Insts_Vector)) {
      OP *to_op = OP_VECTOR_element (Insts_Vector, best->u1.s2.to);
      Set_OP_flag1 (to_op);
    }
    if (best->spill_kind == SPILL_MOVE_DEF) {
      low = best->u1.s2.from;
    }
    else {
      low = best->u1.s2.to + 1;
    }
    lr = best->u1.s2.move_lr;
    break;
  }

  INT i;
  for (i = low; i <= spill_opnum; i++) {
    //
    // if spilled tn is used at this op, then there is no reduction in its
    // register usage.  if we're at the spill_opnum, and we've spilled a 
    // global, then we'll have a reference to the global register (we make
    // the local a dedicated tn).  allow the decrement in this case.
    //
    if (!Op_Uses_TN(LR_tn(lr), i) || (i == spill_opnum && global_lr_spill)) {
      fat_points[i]--;
    }
  }
}

//
// determine if the current op is the definition of one of the operands
// of the current fatpoint that we're spilling for.  if it is, we want
// to see if it was unallocatable during the fat point calculations.  
//
static BOOL
At_Unallocated_Op_Definition(INT fat_opnum, INT cur_opnum,
			     ISA_REGISTER_CLASS spill_cl)
{
  INT opndnum;
  OP *op = OP_VECTOR_element (Insts_Vector, fat_opnum);
  for (opndnum = 0; opndnum < OP_opnds(op); opndnum++) {
    TN *tn = OP_opnd(op, opndnum);
    if (tn == NULL || TN_is_constant(tn)) {
      continue;
    }
    LIVE_RANGE *lr = LR_For_TN(tn);
    // lr null if inserted for previous spill.  we're at an unallocated definition
    // if its the last use, another global has not been spilled for it, and the
    // current op is its definition.
    if (TN_register_class(tn) == spill_cl && lr != NULL
	&& !LR_fat_allocated(lr) && LR_last_use(lr) == fat_opnum &&
	LR_first_def(lr) == cur_opnum) {
      return(TRUE);
    }
  }
  return(FALSE);
}

/* This procedure tracks exactly what will happen in Spill_Live_Range to
 * get an accurate estimate of the spill-cost.
 */
static void
Analyze_Spilling_Live_Range (
  LIVE_RANGE *spill_lr,
  INT spill_opnum,
  SPILL_CANDIDATE *best,
  INT fatpoint)
{
  INT spill_fatpoint = fatpoint_min;
  TN *spill_tn = LR_tn(spill_lr);
  BOOL is_local_tn = TN_is_local_reg (spill_tn);
  ISA_REGISTER_CLASS spill_cl = TN_register_class(spill_tn);
  REGISTER spill_reg = REGISTER_UNDEFINED;
  ST *spill_loc;
  BOOL def_available = FALSE;
  BOOL pending_store = FALSE;
  BOOL spill_store_deleted = FALSE;
  BOOL already_spilled = (TN_spill(spill_tn) == NULL) ? FALSE : TRUE;
#ifdef TARG_ST100
  BOOL reloadable = Is_LR_Reloadable(spill_lr);
#else
  BOOL reloadable = (already_spilled || Is_LR_Reloadable(spill_lr)) ? TRUE : FALSE;
#endif
  float store_cost, restore_cost;
  float cost = 0.0;
  INT benefit = 0;
  INT cur_benefit = 0;
  INT first_def = LR_first_def(spill_lr);
  INT last_use = LR_last_use(spill_lr);
  INT last_def = 0;
  INT last_use_seen = 0;
  INT def_cnt = 0;
  BOOL lr_spans_spill_opnum = FALSE;
  BOOL upward_exposed_global = FALSE;
  
  CGSPILL_Cost_Estimate (spill_tn, NULL, &store_cost, &restore_cost, CGSPILL_LRA);

  // If the rematerialization cost is too high, reset that flag and
  // spill it like an ordinary TN instead. There is also a problem in 
  // the rematerialization sequence for a 64bit immediate load. It
  // uses and extra register which defeats the purpose of spilling
  // the live range. So, we need to reset rematerialization flag for
  // that reason as well.
  if (restore_cost > 3.0 && TN_is_rematerializable(spill_tn)) {
    Reset_TN_is_rematerializable (spill_tn);
    Set_TN_home(spill_tn, NULL);
    CGSPILL_Cost_Estimate (spill_tn, NULL, &store_cost, &restore_cost, CGSPILL_LRA);
  }

  spill_loc = TN_spill(spill_tn);

  //
  // Yikes!  If this TN was created to spill another TN,
  // try real hard not to spill it again.
  if (spill_loc != NULL) cost += 16.0;

  //
  // if this is a large stack size integer spill, then we'll require
  // an additional register to calculate the offset (don't have this
  // problem with loads, because we can use the register that we're
  // loading into for the offset).  need to reduce the minimum fat point
  // at which we'll force a pending store into memory.  note that this
  // isn't quite right for locals ... they'll not have had spill temps
  // allocated for them yet.  its not a really big deal, though, because
  // the spills will just happen sooner and will free up more registers
  // than the estimates (freeing less is *bad*).
  //
#ifdef TARG_ST100
  // Arthur: get rid of target dependency
  if (REGISTER_CLASS_is_ptr(TN_register_class(spill_tn))
#else
  if (TN_register_class(spill_tn) == ISA_REGISTER_CLASS_integer 
#endif
      && !TN_is_rematerializable(spill_tn) && !TN_is_gra_homeable(spill_tn)
      && Exp_Is_Large_Stack_Sym(spill_loc, 0)) {
    spill_fatpoint--;
  }

  if (reloadable) store_cost = 0.0;

  if (!is_local_tn) {
    spill_reg = LRA_TN_register(spill_tn);
    if (LR_exposed_use(spill_lr)) {
      def_available = TRUE;
      pending_store = TRUE;
      first_def = 1;
      upward_exposed_global = TRUE;
      last_def = 0;  // def is outside the bb.
    }
    if (last_use == VECTOR_size(Insts_Vector)) last_use--;
  }

  for (INT i = first_def; i <= last_use; i++) 
  {
    OP *op = OP_VECTOR_element(Insts_Vector, i);
    if (op == NULL) continue;

    BOOL at_fatpoint = (fat_points[i] > spill_fatpoint && i <= spill_opnum);

    // If the register usage at this point is greater than the fatpoint
    // and if there is a pending store, insert the store now.
    if (at_fatpoint) {
      cur_benefit++;
      if (pending_store) {
        cost += store_cost;
        pending_store = FALSE;
	upward_exposed_global = FALSE;
      }
    }

    // For local TNs, check if the current OP references the TN we want to 
    // spill. For global TNs, check if the current OP references the register
    // assigned to the global TN.
    if (Op_Uses_TN(spill_tn, i)) {
      //
      // check if the live range spans the spill_opnum.  get out if
      // if the spill_opnum references the live range.
      //
      if (i == spill_opnum) {
	return;
      } else if (i > spill_opnum && last_def < spill_opnum) {
        lr_spans_spill_opnum = TRUE;
      }

      if (!reloadable && Is_OP_Spill_Store (op, spill_loc) &&
          !already_spilled && (i == last_use)) cost += 1.0;

      // if this is an op that modifies one of its operands, then we
      // will have to insert a copy to ensure that we free up a register
      // when we spill it (otherwise, it'll be tied up between the first
      // definition of the modified tn and this op).  account for that
      // by adding 1 to it (we really should use something from targ_info
      // for this).
      //
      if (OP_same_res(op) || OP_cond_def(op)) {
	cost += 1.0;
      }

      benefit += cur_benefit;
      cur_benefit = 0;
      last_use_seen = i;
      if (!reloadable && (already_spilled && Is_OP_Spill_Store (op, spill_loc))) {
        // If the use of spill_tn is a store to the spill location, we 
        // don't have to load from memory. Actually, we can get rid
        // of the store as well, since the memory contents are already
        // current.
        spill_store_deleted = TRUE;
        cost -= store_cost;
        if (LR_exposed_use(spill_lr) == i) def_available = FALSE;
        continue;
      }

      // If there is no definition available, reload from spill location.
      if (!def_available) {
        def_available = TRUE;
        cost += restore_cost;
      }
      if (LR_exposed_use(spill_lr) == i) def_available = FALSE;
    }

    // Check if the current OP is a definition of the spill tn.
    if ((TN_is_local_reg (spill_tn) && 
         OP_Defs_TN (op, spill_tn)) ||
        (!TN_is_local_reg (spill_tn) && 
         OP_Defs_Reg (op, spill_cl, spill_reg)))
    {
      def_cnt++;
      if (def_cnt > 1 && last_use_seen < spill_opnum && i > spill_opnum) {
	//
	// if this had multiple definitions, and we're not yet to the last
	// one, then we need to add the benefit gained from spilling the
	// live range between the last use of the previous definition and
	// this definition.  
	//
	benefit += cur_benefit;
	lr_spans_spill_opnum = TRUE;
      }
      last_def = i;
      cur_benefit = 0;
#ifdef TARG_ST100
      if (reloadable || 
	  (already_spilled && Is_OP_Spill_Load (op, spill_loc)) && (i != first_def)) {
#else
      if (reloadable || (already_spilled && Is_OP_Spill_Load (op, spill_loc))) {
#endif
        // If the definition of the spill_tn is a load from the spill
        // location, we don't have to insert the store to memory. Actually
        // we can get rid of the load as well, since we will be loading 
        // from the spill location before each use of spill_tn.
        cost -= restore_cost;
        continue;
      }

      def_available = TRUE;
      pending_store = TRUE;
      upward_exposed_global = FALSE;
    }
    else if (already_spilled && Is_OP_Spill_Load(op, spill_loc) && pending_store &&
	     spill_store_deleted) {
      //
      // must put spill back if we spilled a global, and are now reloading
      // it back to a local.
      //
      cost += store_cost;
      pending_store = FALSE;
    }
    else if (at_fatpoint || (upward_exposed_global &&
	     At_Unallocated_Op_Definition(spill_opnum, i, spill_cl))) {
      // If the current instruction is a fatpoint, or we're at the definition
      // of an unallocated operand of the fatpoint, make any earlier
      // definition unavailable.  we need to do this for globals at exposed
      // uses since the exposed use will take the register allocated to the
      // global, and this is the register we've made available at the fat
      // point (uses of the global after spill reloads will use whatever
      // register is available at that point).
      def_available = FALSE;

      if (upward_exposed_global) {
	cost += store_cost;
	pending_store = FALSE;
	upward_exposed_global = FALSE;
      }
    }
  }

  // If there was a use that is a store to a spill location it has
  // been deleted. So, if there is still a pending_store at this point,
  // go back and insert the spill store after the last def. Otherwise,
  // the spill location will not get updated.
  if (pending_store && spill_store_deleted) {
    cost += store_cost;
    pending_store = FALSE;
  }

  // Check if spill_tn is live-out from this basic block. If it is,
  // we need to create a definition of spill_tn.
  if (LR_last_use(spill_lr) == VECTOR_size(Insts_Vector)) {
    // check if the live range spans the spill_opnum.
    if (last_def < spill_opnum) {
      lr_spans_spill_opnum = TRUE;
    }

    benefit += cur_benefit;
    if (!def_available) {
      cost += restore_cost;
    }
    else {
      // even if there is a def available, we still need to insert a COPY
      // which may not get deleted.
      cost += 0.5;
    }
  }

  if (Do_LRA_Trace(Trace_LRA_Spill)) {
    fprintf (TFile, "Analyze >> (cost:%g, benefit:%d, spans spill:%d)", 
        cost, benefit, lr_spans_spill_opnum);
    Print_Live_Range (spill_lr);
  }

  if (lr_spans_spill_opnum &&
      benefit > 0 &&
      (best->spill_kind == SPILL_NONE ||
       (float)(benefit - best->benefit) > 16.0 * (cost - best->cost)))
  {
    best->spill_kind = SPILL_LIVE_RANGE;
    best->u1.spill_lr = spill_lr;
    best->cost = cost;
    best->benefit = benefit;
  }
}


static void
Add_Spill_Store_After_Def (TN *tn, INT def, BB *bb)
{
  OPS spill_ops = OPS_EMPTY;
  ST *spill_loc = TN_spill(tn);

  CGSPILL_Store_To_Memory (tn, spill_loc, &spill_ops, CGSPILL_LRA, bb);
  // Insert the store right after the previous definition.
  if (def != 0) {
    OP *def_op = OP_VECTOR_element(Insts_Vector, def);
    CGSPILL_Insert_Ops_After (bb, def_op, &spill_ops);
    if (Do_LRA_Trace(Trace_LRA_Spill)) {
      fprintf (TFile, "LRA_SPILL>>    store TN%d after %d\n", TN_number(tn), def);
    }
  }
  else {
    CGSPILL_Prepend_Ops (bb, &spill_ops);
    if (Do_LRA_Trace(Trace_LRA_Spill)) {
      fprintf (TFile, "LRA_SPILL>>    store TN%d at top of bb\n", TN_number(tn));
    }
  }
}


static void
Add_Spill_Load_Before_Use (TN *tn, ST *spill_loc, OP *reload_op, INT use, BB *bb)
{
  OPS spill_ops = OPS_EMPTY;

  if (reload_op) {
    OP *new_op = Dup_OP (reload_op);
    Copy_WN_For_Memory_OP (new_op, reload_op);
    //
    // This assumes that the first result is the destination of the 
    // load.  This should probably be done via some interface, but
    // I'm leaving it for now.
    //
    Set_OP_result (new_op, 0 /*???*/, tn);
    OPS_Append_Op (&spill_ops, new_op);
  }
  else {
    Set_TN_spill(tn, spill_loc);
    CGSPILL_Load_From_Memory (tn, spill_loc, &spill_ops, CGSPILL_LRA, bb);
  }

  if (use == VECTOR_size(Insts_Vector)) {
    // append at at of bb.
    CGSPILL_Append_Ops (bb, &spill_ops);
    if (Do_LRA_Trace(Trace_LRA_Spill)) {
      fprintf (TFile, "LRA_SPILL>>    load TN%d at end of bb\n", TN_number(tn));
    }
  }
  else {
    OP *use_op = OP_VECTOR_element(Insts_Vector, use);
    CGSPILL_Insert_Ops_Before (bb, use_op, &spill_ops);
    if (Do_LRA_Trace(Trace_LRA_Spill)) {
      fprintf (TFile, "LRA_SPILL>>    load TN%d before %d\n", TN_number(tn), use);
    }
  }

}


static void
Replace_TN_References (TN *old_tn, TN *new_tn, OP *op)
{
  // replace all references to spill_tn in the op to new_tn.
  for (INT opndnum = 0; opndnum < OP_opnds(op); opndnum++) {
    TN *opnd_tn = OP_opnd(op, opndnum);
    if (!TN_is_register(opnd_tn)) continue;
    ISA_REGISTER_CLASS old_cl = TN_register_class(old_tn);
    REGISTER old_reg = LRA_TN_register(old_tn);
    if (opnd_tn == old_tn ||
        (!TN_is_local_reg(old_tn) && 
         TN_register_class(opnd_tn) == old_cl &&
         LRA_TN_register(opnd_tn) == old_reg))
    {
      Set_OP_opnd(op, opndnum, new_tn);
    }
  }

  // Mark the live range of the result TN as not reloadable. Otherwise,
  // if we later decide to spill that live range, we can introduce new
  // references to the TN we are in the process of spilling.
  for (INT i = 0; i < OP_results(op); i++) {
    TN *result_tn = OP_result(op, i);
    LIVE_RANGE *lr = LR_For_TN (result_tn);
    if (lr != NULL)
      Reset_LR_reloadable (lr);
  }
}


/* ======================================================================
 * Spill_Live_Range
 * 
 * Spill a live-range in the <bb>. It inserts stores after each 
 * definition of the live-range and inserts a load before each use. The 
 * following optimizations are performed:
 *
 *  1. If a defintion in the live-range is a load from the spill location, 
 *     don't generate the store and delete this defintion.
 *  2. If the use in the live-range is a store to the spill location, don't 
 *     insert a load before the use. Delete the existing store.
 *  3. If the use is very close to an earlier def/use, don't reload
 *     from the spill location.
 * ======================================================================*/
static void
Spill_Live_Range (
  BB *bb, 
  LIVE_RANGE *spill_lr,
  INT fatpoint,
  INT spill_opnum)
{
  INT spill_fatpoint = fatpoint_min;
  TN *spill_tn = LR_tn(spill_lr);
  BOOL is_local_tn = TN_is_local_reg (spill_tn);
  ISA_REGISTER_CLASS spill_cl = TN_register_class(spill_tn);
  REGISTER spill_reg = REGISTER_UNDEFINED;
  ST *spill_loc = NULL;
  ST *sv_spill_location = NULL;
  BOOL magic_spill_used = FALSE;
  BOOL upward_exposed_global = FALSE;
  TN *new_tn = NULL;
  INT first_def;
  INT last_use;
  BOOL pending_store = FALSE;
  BOOL spill_store_deleted = FALSE;
  BOOL def_available = FALSE;
  BOOL already_spilled = (TN_spill(spill_tn) == NULL) ? FALSE : TRUE;
#ifdef TARG_ST100
  //
  // Arthur: there seems to be a problem here.
  //         I do not understand it, just trying to make it work ...
  //
  BOOL reloadable = LR_reloadable(spill_lr);
#else
  BOOL reloadable = (already_spilled || LR_reloadable(spill_lr)) ? TRUE : FALSE;
#endif
  OP *reloadable_def = NULL;
  INT last_def;
  INT last_opnum;

  if (Do_LRA_Trace(Trace_LRA_Spill)) {
    fprintf (TFile, "LRA_SPILL>> spill %s lr at OP:%d", 
       (LR_reloadable(spill_lr)) ? "reloadable" : (already_spilled) ? "already spilled" : "", spill_opnum);
    Print_Live_Range (spill_lr);
  }

  first_def = LR_first_def(spill_lr);
  last_use = LR_last_use(spill_lr);
  if (reloadable) {
    reloadable_def = OP_VECTOR_element(Insts_Vector, first_def);

    // mark the live ranges of operands of the reloadable_def as being
    // not available for spill. If we try to spill on of the operand
    // TNs later, we will not find the uses added by cloning the 
    // reloadable def.
    Remove_LRs_For_OP (reloadable_def);
  }
  else {
    // Try using the magic symbol as a last ditch effort. It is 
    // guaranteed to be within 16 bits of sp/fp.
    if (Trip_Count == MAX_TRIP_COUNT &&
        Magic_Spill_Location != NULL &&
        !TN_is_float(spill_tn)) 
    {
      sv_spill_location = TN_spill(spill_tn);
      Set_TN_spill(spill_tn, Magic_Spill_Location);
      spill_loc = Magic_Spill_Location;
      magic_spill_used = TRUE;
      DevWarn ("Used short-offset spill location in BB%d", BB_id(bb));
    }
    else {
      /* kludge to fix bug 386428 */
#ifdef TARG_ST100
      // Arthur: since there is a possibility to generate not GP-realtive
      //         code
      if (TN_is_rematerializable (spill_tn) && BB_exit (bb) && GP_TN != NULL) {
#else
      if (TN_is_rematerializable (spill_tn) && BB_exit (bb)) {
#endif
	LIVE_RANGE *gp_lr = LR_For_TN (GP_TN);
	if (gp_lr != NULL && 
	    LR_def_cnt(gp_lr) != 0 &&
	    last_use > LR_first_def(gp_lr))
	{
	  Reset_TN_is_rematerializable (spill_tn);
	  Set_TN_home(spill_tn, NULL);
	}
      }
      spill_loc = CGSPILL_Get_TN_Spill_Location (spill_tn, 
                        (is_local_tn) ? CGSPILL_LRA : CGSPILL_LGRA);

      if (Do_LRA_Trace(Trace_LRA_Spill)) {
	fprintf (TFile, "LRA_SPILL>> spill_loc %s for ", 
                                                ST_name(spill_loc));
	Print_TN(spill_tn, FALSE);
	fprintf(TFile, "\n");
      }

    }
  }

  //
  // if this is a large stack size integer spill, then we'll require
  // an additional register to calculate the offset (don't have this
  // problem with loads, because we can use the register that we're
  // loading into for the offset).  need to reduce the minimum fat point
  // at which we'll force a pending store into memory.
  //
#ifdef TARG_ST100
  // Arthur: get rid of target dependency
  if (REGISTER_CLASS_is_ptr(TN_register_class(spill_tn))
#else
  if (TN_register_class(spill_tn) == ISA_REGISTER_CLASS_integer
#endif
      && !TN_is_rematerializable(spill_tn) && !TN_is_gra_homeable(spill_tn)
      && Exp_Is_Large_Stack_Sym(spill_loc, 0)) {
    spill_fatpoint--;
  }

  if (!is_local_tn) {
    spill_reg = LRA_TN_register(spill_tn);
    if (LR_exposed_use(spill_lr)) {
      new_tn = spill_tn;
      def_available = TRUE;
      pending_store = TRUE;
      upward_exposed_global = TRUE;
      last_def = 0;  // def is outside the bb.
      first_def = 1;
    }
    if (last_use == VECTOR_size(Insts_Vector)) last_use--;
  }

  last_opnum = 0;
  INT i;
  for (i = first_def; i <= last_use; i++) 
  {
    OP *op = OP_VECTOR_element(Insts_Vector, i);
    if (op == NULL) continue;

    BOOL at_fatpoint = (fat_points[i] > spill_fatpoint && i <= spill_opnum);

    // If the register usage at this point is greater than the fatpoint
    // and if there is a pending store, insert the store now.
    if (pending_store && at_fatpoint) {
      Add_Spill_Store_After_Def (new_tn, last_def, bb);
      if (upward_exposed_global) {
	LR_first_spill(spill_lr) = last_def + 1;
	upward_exposed_global = FALSE;
      }
      pending_store = FALSE;
    }

    // For local TNs, check if the current OP references the TN we want to 
    // spill. For global TNs, check if the current OP references the register
    // assigned to the global TN.
    if ((is_local_tn && 
         OP_Refs_TN (op, spill_tn)) ||
        (!is_local_tn && 
         OP_Refs_Reg (op, spill_cl, spill_reg)))
    {
#ifdef TARG_ST100
      // Arthur: If the use of spill_tn is a store to the spill location, we 
      //         don't have to load from memory. Actually, we can get rid
      //         of the store as well, since the memory contents are already
      //         current. 
      //         Don't delete if it's the last use.
      //         Don't delete if reloadable since there is no first store,
      //         i.e. memory is not up to date.
      //
      if (!reloadable && Is_OP_Spill_Store (op, spill_loc) &&
          already_spilled && (i != last_use)) {
#else
      if (!reloadable && Is_OP_Spill_Store (op, spill_loc) &&
          !already_spilled && (i == last_use)) {
#endif
        // If the use of spill_tn is a store to the spill location, we 
        // don't have to load from memory. Actually, we can get rid
        // of the store as well, since the memory contents are already
        // current.
        BB_Remove_Op (bb, op);
	Clobber_Op_Info(i, spill_cl);
        spill_store_deleted = TRUE;
        if (Do_LRA_Trace(Trace_LRA_Spill)) {
          fprintf (TFile, "LRA_SPILL>>    Removed spill store at %d\n", i);
          Print_OP_No_SrcLine(op);
        }
        if (LR_exposed_use(spill_lr) == i) {
	  def_available = FALSE;
	}
        continue;
      }

      // If there is no definition available, reload from spill location.
      if (!def_available) {
        def_available = TRUE;
        new_tn = Dup_TN_Even_If_Dedicated (spill_tn);
        Set_TN_spill(new_tn, spill_loc);
        Add_Spill_Load_Before_Use (new_tn, spill_loc, reloadable_def, i, bb);
      }
      else if (Do_LRA_Trace(Trace_LRA_Spill)) {
        fprintf (TFile, "LRA_SPILL>>    skipped load of TN%d before %d\n", 
                          TN_number(new_tn), i);
      }
      Replace_TN_References (spill_tn, new_tn, op);
      if (LR_exposed_use(spill_lr) == i) {
	def_available = FALSE;
      }
    }

    // Check if the current OP is a definition of the spill tn.
    if ((TN_is_local_reg (spill_tn) && 
         OP_Defs_TN (op, spill_tn)) ||
        (!TN_is_local_reg (spill_tn) && 
         OP_Defs_Reg (op, spill_cl, spill_reg)))
    {
      if (i == VECTOR_size(Insts_Vector)-1) {
	// if the defining op is the last thing in the block, do nothing
	// with it as we'll give it the register and thus have the correct
	// value in the register upon exit.  i can't think of any situation
	// in which we get here, and we've made a sane spill choice (this
	// would typically come up if we're spilling $31, and the block is
	// terminated by a call.  in such a case, the dedicated tn for $31
	// is likely not used anywhere else in the block (unless this is the
	// prolog), so this spill is probably not going to reduce register
	// pressure.  if we don't handle it this way, though, the reload
	// of the register will be placed *before* the terminating instruction
	// in the block, and we'll get wrong code.  better stupid than
	// erroneous.
	last_def = i;
	continue;
      }
       /* Note: Add the check for !already_spilled to the following "if".
         The call to Is_OP_Spill_Load, detects a spill by using
         the TN_spill macro.  Unfortunately, that information is set
         when a call is made to CGSPILL_Get_TN_Spill_Location, which
         happens early in this routine.  Because of this, the following
         code may delete a load of a value, thinking that it has already
         been saved.  If this is the first definition of the TN in the
         Live Range, there will have been no previous save.  So let's
         skip the following logic unless this TN has already been spilled. */
#ifdef TARG_ST100
      //
      // Arthur: there seems to be a problem.
      //         The first def can be marked already spilled if it's a
      //         GRA spill. This would not be a problem, but such GRA
      //         spill may be "dummy" - result of not knowing that a
      //         conditional def is the first def of a TN. GRA thinks
      //         that conditional defs do not kill live ranges, so a
      //         live range may be live-in even when there is no
      //         previous definition. In this case, there is a move
      //         inserted later down here for a conditional def with
      //         the source TN being the one "live-in" which is
      //         undefined. To avoid this, I add an explicit check
      //         for not deleting the first def.
      //
      if (LR_reloadable(spill_lr) || 
	  (already_spilled && Is_OP_Spill_Load (op, spill_loc)) && (i != first_def)) {
#else
      if (reloadable || (already_spilled && Is_OP_Spill_Load (op, spill_loc))) {
#endif
        // If the definition of the spill_tn is a load from the spill
        // location, we don't have to insert the store to memory. Actually
        // we can get rid of the load as well, since we will be loading 
        // from the spill location before each use of spill_tn.
        BB_Remove_Op (bb, op);
	Clobber_Op_Info(i, spill_cl);
        Set_OP_VECTOR_element(Insts_Vector, i, NULL);
        if (Do_LRA_Trace(Trace_LRA_Spill)) {
#if 0
	  if (LR_reloadable(spill_lr)) {
#else
	  if (reloadable) {
#endif
	    fprintf (TFile, "LRA_SPILL>>    Removed reloadable load at %d\n",
		     i);
	  } else {
	    fprintf (TFile, "LRA_SPILL>>    Removed spill load at %d\n", i);
	  }
          Print_OP_No_SrcLine(op);
        }
        continue;
      }

      // if the OP is a select or an unaligned load, we cannot get a 
      // new tn. This ensures that we get the result tn to be the same
      // as one of the operand tns.  note that if we had a previous def
      // in the block, we will already have created a new tn for that 
      // def.  in that case, that will be the source that must have the
      // copy inserted for it.
      //
      INT resnum = 0;
      TN* prev_tn = new_tn ? new_tn : spill_tn;
      new_tn = Dup_TN_Even_If_Dedicated (spill_tn);
      Set_TN_spill(new_tn, spill_loc);
      local_spills++;global_spills++;

      if (OP_same_res(op) && TN_Pair_In_OP(op, spill_tn, prev_tn)) {
	//
	// must insert copy to free register here.  otherwise,
	// the register will be tied up from the first definition
	// of spill_tn, to this op (which modifies it).
	//
	BOOL copy_added = FALSE;
	for (INT opndnum = 0; opndnum < OP_opnds(op); opndnum++) {
	  TN* op_tn = OP_opnd(op, opndnum);
	  if (op_tn == prev_tn) {
            if (!copy_added) {
	      OPS copy_ops = OPS_EMPTY;
	      Exp_COPY(new_tn, prev_tn, &copy_ops);
	      if (Do_LRA_Trace(Trace_LRA_Spill)) {
	        fprintf (TFile, "LRA_SPILL>> copy TN%d to TN%d for same_res OP\n", 
	  	         TN_number(prev_tn), TN_number(new_tn));
	      }
	      CGSPILL_Insert_Ops_Before(bb, op, &copy_ops);
	      copy_added = TRUE;
            }
	    Set_OP_opnd(op, opndnum, new_tn);
	  }
	}
	FmtAssert(copy_added, 
                  ("LRA: couln't find same_res operand for copy\n"));

        // For IA-32 target there may be more than one result that is the
        // the same as one of the operands. We need to find out which one 
        // it is and not assume that it's always result 0. 
        resnum = TN_Resnum_In_OP (op, spill_tn);
      }

      else if (OP_cond_def(op) && OP_result(op, 0) == spill_tn) {
        OPS copy_ops = OPS_EMPTY;
        Exp_COPY(new_tn, prev_tn, &copy_ops);
        if (Do_LRA_Trace(Trace_LRA_Spill)) {
          fprintf (TFile, "LRA_SPILL>> copy TN%d to TN%d for cond_def OP\n", 
                   TN_number(prev_tn), TN_number(new_tn));
        }
        CGSPILL_Insert_Ops_Before(bb, op, &copy_ops);
        resnum = TN_Resnum_In_OP (op, spill_tn);
      }

      else if ((OP_results(op) > 1) && (OP_result(op, 0) != spill_tn)) {

        for (resnum = OP_results(op); resnum > 0; resnum--) {
          if (spill_tn == OP_result(op,resnum)) break;
        }

      }

      Set_OP_result(op, resnum, new_tn);
      def_available = TRUE;
      upward_exposed_global = FALSE;
      pending_store = TRUE;
      last_def = i;
    }
    else if (Is_OP_Spill_Load(op, spill_loc) && pending_store &&
	     spill_store_deleted) {
      //
      // must put spill back if we spilled a global, and are now reloading
      // it back to a local.
      //
      Add_Spill_Store_After_Def (new_tn, last_def, bb);
      pending_store = FALSE;
    }
    else if (at_fatpoint || (upward_exposed_global &&
	     At_Unallocated_Op_Definition(spill_opnum, i, spill_cl))) {
      // If the current instruction is a fatpoint, or we're at the definition
      // of an unallocated operand of the fatpoint, make any earlier
      // definition unavailable.  we need to do this for globals at exposed
      // uses since the exposed use will take the register allocated to the
      // global, and this is the register we've made available at the fat
      // point (uses of the global after spill reloads will use whatever
      // register is available at that point).
      def_available = FALSE;

      if (upward_exposed_global) {
	//
	// spill the global before this instruction
	//
	LR_first_spill(spill_lr) = last_def + 1;
	upward_exposed_global = FALSE;
	Add_Spill_Store_After_Def (new_tn, last_opnum, bb);
	pending_store = FALSE;
      }
    }
    last_opnum = i;
  }

  // If there was a use that is a store to a spill location it has
  // been deleted. So, if there is still a pending_store at this point,
  // go back and insert the spill store after the last def. Otherwise,
  // the spill location will not get updated.
  if (pending_store && spill_store_deleted) {
    Add_Spill_Store_After_Def (new_tn, last_def, bb);
    pending_store = FALSE;
  }

  // Check if spill_tn is live-out from this basic block. If it is,
  // we need to create a definition of spill_tn.  if its last definition
  // is also the last instruction in the block (this is likely $31 ...
  // see comments above), then no need to copy (just let the last
  // instruction write the register).
  if (LR_last_use(spill_lr) == VECTOR_size(Insts_Vector) &&
      last_def != VECTOR_size(Insts_Vector)-1) {
    if (!def_available) {
      Add_Spill_Load_Before_Use (spill_tn, spill_loc, reloadable_def, 
                                                  LR_last_use(spill_lr), bb);
    }
    else {
      // Copy the new_tn to the spill_tn.
      OPS spill_ops = OPS_EMPTY;
      Exp_COPY (spill_tn, new_tn, &spill_ops);
      CGSPILL_Append_Ops (bb, &spill_ops);
      if (Do_LRA_Trace(Trace_LRA_Spill)) {
        fprintf (TFile, "LRA_SPILL>>    copy TN%d to TN%d at end of %d\n", 
                    TN_number(new_tn), TN_number(spill_tn), i);
      }
    }
  }

  if (magic_spill_used) {
    // Use the location only once for each basic block.
    Magic_Spill_Location = NULL;
    // For global TNs, restore the TN_spill value.
    if (!is_local_tn) {
      Set_TN_spill(spill_tn, sv_spill_location);
    }
  }

  //
  // for spilled global live ranges, we must ensure that they
  // are given to one of the unallocated operands of the current
  // fat point.  we do this because if we allow any register to
  // use this global above the point that we restore it at the bottom
  // of the block, we are not guaranteed that a register will be available
  // when we try to allocate for the operands of this instruction.  this
  // is mainly because spills are inserted for global live ranges based
  // on the live range of the local that they are spilling for.  thus, if
  // this specific global is unavailable, another global may not have
  // spills placed appropriately, and that global cannot be used.
  //
  if (!is_local_tn && do_global_locking) {
    OP *op = OP_VECTOR_element (Insts_Vector, spill_opnum);
    INT opndnum;
    for (opndnum = 0; opndnum < OP_opnds(op); opndnum++) {
      TN *tn = OP_opnd(op, opndnum);
      if (tn == NULL || TN_is_constant(tn)) {
	continue;
      }
      LIVE_RANGE *lr = LR_For_TN(tn);
      // lr null if inserted for previous spill
      if (TN_register_class(tn) == spill_cl && lr != NULL
	  && !LR_fat_allocated(lr) && LR_last_use(lr) == spill_opnum) {
	Set_TN_is_dedicated(tn);
	Set_TN_register(tn, spill_reg);
	Set_LR_fat_allocated(lr);
      }
    }
  }
}


static void
Add_To_Live_LRs_Vector (LIVE_RANGE *lr)
{
  if (!LR_added(lr)) {
    Set_LR_added(lr);
    VECTOR_Add_Element (Live_LRs_Vector, lr);
  }
}


static void
Update_Live_LRs_Vector (INT opnum, ISA_REGISTER_CLASS cl)
{
  if (opnum == 1) return;

  INT i;
  OP *op = OP_VECTOR_element(Insts_Vector, opnum);
  if (op != NULL) {
    for (i = 0; i < OP_opnds(op); i++) {
      TN *tn = OP_opnd(op,i);
      if (!TN_is_register(tn) || TN_register_class(tn) != cl) continue;
      LIVE_RANGE *lr = LR_For_TN (tn);
      // For new TNs added, lr will be NULL. 
      if (lr != NULL && 
          LR_last_use(lr) == opnum && 
          opnum > (LR_first_def(lr)+1)) 
      {
        Add_To_Live_LRs_Vector (lr);
      }
    }
  }

  /* remove the def for the next OP since we don't want to consider
   * that live range for spilling any longer.
   */
  op = OP_VECTOR_element (Insts_Vector, opnum-1);
  if (op != NULL) {
    for (i = 0; i < OP_results(op); i++) {
      TN *tn = OP_result(op, i);
      LIVE_RANGE *lr = LR_For_TN (tn);
      // For new TNs added, lr will be NULL. 
      if (lr != NULL &&
          LR_first_def(lr) == (opnum-1) && 
          LR_exposed_use(lr) == 0) 
      {
        VECTOR_Delete_Element (Live_LRs_Vector, lr);
      }
    }
  }
}

//
// remove the live range associated with opnum if this is the 
// initial definition of it.  after we've done all of the spilling
// for an op (or passed it over as we don't need to spill), we
// remove its live range as it can't possibly contribute to clearing
// any of the fat points above it.
//
static void
Remove_Current_LR(INT opnum)
{
  OP *op = OP_VECTOR_element (Insts_Vector, opnum-1);
  
  if (op != NULL) {
    for (INT i = 0; i < OP_results(op); i++) {
      TN *tn = OP_result(op, i);
      LIVE_RANGE *lr = LR_For_TN (tn);
      // For new TNs added, lr will be NULL. 
      if (lr != NULL &&
          LR_first_def(lr) == (opnum) && 
          LR_exposed_use(lr) == 0) 
      {
        VECTOR_Delete_Element (Live_LRs_Vector, lr);
      }
    }
  }
}


// Compute the set of live ranges that span the instruction that 
// caused the register allocation failure. First look for local 
// live ranges by traversing through the OPs. Then traverse through
// all registers in this class to add global live ranges that span
// the failure point. We filter out those live ranges which have 
// have a reference in the failing instruction.
static void
Init_Live_LRs_Vector (
  BB *bb, 
  INT failure_point, 
  ISA_REGISTER_CLASS cl, 
  MEM_POOL *pool)
{
  OP *failure_op = OP_VECTOR_element(Insts_Vector, failure_point);
  Live_LRs_Vector = VECTOR_Init (BB_length(bb)+REGISTER_MAX, pool);

  // find candidate global live ranges to spill.
  // don't consider global live ranges with the option -Wb,-ttlra:0x100.
  if (!Get_Trace (TP_ALLOC, 0x0100)) {
    // Look for live ranges of all allocatable registers in this class.
    REGISTER_SET rs = REGISTER_CLASS_allocatable(cl);
    for (REGISTER reg = REGISTER_SET_Choose(rs);
         reg != REGISTER_UNDEFINED;
         reg = REGISTER_SET_Choose_Next(rs, reg))
    {
      LIVE_RANGE *reg_lr = LR_For_TN (Build_Dedicated_TN(cl, reg, 0));
      if (reg_lr != NULL &&
          LR_last_use(reg_lr) > failure_point &&
          (LR_exposed_use(reg_lr) || LR_first_def(reg_lr) < failure_point) &&
          !OP_Refs_Reg (failure_op, cl, reg))
      {
        Add_To_Live_LRs_Vector (reg_lr);
      }
    }
  }

  // find candidate local live ranges to spill.
  for (INT opnum = 1; opnum < failure_point; opnum++) {
    LIVE_RANGE *clr;
    OP *op = OP_VECTOR_element(Insts_Vector, opnum);
    for (INT resnum = 0; resnum < OP_results(op); resnum++) {
      TN *result_tn = OP_result(op, resnum);
      if (TN_register_class(result_tn) == cl && 
          TN_is_local_reg(result_tn) &&
          (clr = LR_For_TN (result_tn)) &&
          LR_first_def(clr) == opnum &&
          LR_last_use(clr) > failure_point &&
          !OP_Refs_TN (failure_op, result_tn))
      {
	Add_To_Live_LRs_Vector (clr);
      }
    }
  }
}

/* ======================================================================
 * Fix_LRA_Blues
 *
 * This routine is called when register allocation fails because we ran
 * out of registers. The failure occured when trying to allocate a register
 * for <tn> at instruction number <opnum>.
 *
 * There are several ways to deal with the situation:
 *
 *   1. Spill a global register at the entry of the block and restore it
 *      at the bottom of the block. This effectively increases the number
 *      of registers available for allocation.
 *   2. Reorder instructions to shorten some live ranges. This reduces
 *      the number of overlapping live ranges and increases the chances
 *      for successful register allocation.
 *   3. Spill some live ranges. This has the same effect as 2 in terms of
 *      reducing the number of overlapping live ranges.
 * ======================================================================*/
static void
Fix_LRA_Blues (BB *bb, TN *tn, HB_Schedule *Sched)
{
  // Even when we are not reordering, we need the dep-graph do check 
  // if loads can be replicated instead of spilling.
  BOOL need_dep_graph = (CG_opt_level > 0);

  // The reorder transformations and spill transformations need to 
  // be done in separate passes. This is because spilling can lead
  // to deletion of instructions that might have been used as position
  // markers in the move transformations. So, do only reordering
  // transformations for the first few trips.
  BOOL allow_reordering = Check_Allow_Reorder() || (LRA_do_reorder == TRUE);

  ISA_REGISTER_CLASS cl = TN_register_class(tn);
  LIVE_RANGE *lr = LR_For_TN(tn);
  INT failure_point;
  INT opnum;

  FmtAssert (Trip_Count <= MAX_TRIP_COUNT, 
             ("LRA: Unable to spill TN:%d (cl:%d) in BB:%d, GRA grant:%d", 
                TN_number(tn), cl, BB_id(bb), 
                REGISTER_SET_Size(avail_set[cl])));

  if (Do_LRA_Trace(Trace_LRA_Spill)) {
     fprintf (TFile,"LRA_SPILL>> Attempt to spill (cl:%d) in BB:%d, GRA grant:%d\n",
                cl, BB_id(bb), REGISTER_SET_Size(avail_set[cl]));
  }

  // If this is the first attempt to spill for the bb, then try to 
  // reschedule while minimizing register pressure. 
  if (Check_Allow_Reschedule()) {
    if (Do_LRA_Trace(Trace_LRA_Spill)) {
      fprintf (TFile, "LRA_SPILL>> Out of Registers (BB:%d) trip:%d\nLRA_SPILL>>",
	       BB_id(bb), Trip_Count);
      Print_Live_Range (lr);
    }
    if (Do_LRA_Trace(Trace_LRA_Spill)) {
      fprintf (TFile, "LRA_SPILL>> Reschedule to minimize register usage\n");
    }

    mINT8 regs_avail[ISA_REGISTER_CLASS_MAX+1];
    for (INT i = ISA_REGISTER_CLASS_MIN; i <= ISA_REGISTER_CLASS_MAX; i++) {
      regs_avail[i] = REGISTER_SET_Size (avail_set[i]);
    }

    if (!Sched) {
      Sched = CXX_NEW(HB_Schedule(), &MEM_local_pool);
    }
    Sched->Init(bb, 
		HBS_BEFORE_LRA | HBS_DEPTH_FIRST | HBS_MINIMIZE_REGS, 
		INT32_MAX, 
		NULL, 
		regs_avail);
    Sched->Schedule_BB(bb, NULL);
    Reset_BB_scheduled (bb);
    return;
  }

  if (Do_LRA_Trace(Trace_LRA_Spill)) {
    fprintf(TFile, "LRA_SPILL>> reschedule failed.  Must %s in register class %d ",
	    allow_reordering?"reorder":"spill",cl);
    fprintf(TFile, "in block %d ", BB_id(bb));
    fprintf(TFile, "for procedure %s in file %s\n",
	    ST_name(Get_Current_PU_ST()), Src_File_Name);
  }

  // Move the delay slot instruction above if the branch has delay slot.
  if (PROC_has_branch_delay_slot())
    BB_Move_Delay_Slot_Op (bb);

  /* rebuild live ranges */
  Setup_Live_Ranges (bb, TRUE, &lra_pool);

  // 
  // establish failure point from newly created live range.
  //
  lr = LR_For_TN(tn);
  failure_point = LR_last_use (lr);
  if (failure_point == 0) failure_point = LR_exposed_use(lr);
  if (Do_LRA_Trace(Trace_LRA_Spill)) {
    fprintf (TFile,"LRA_SPILL>> Out of Registers (BB:%d) trip:%d\nLRA_SPILL>>",
	     BB_id(bb), Trip_Count);
    Print_Live_Range (lr);
  }

  if (failure_point !=0) {
    OP *op = OP_VECTOR_element (Insts_Vector, failure_point);
    if (!PROC_has_branch_delay_slot() && OP_xfer(op)) {
      if (Do_LRA_Trace(Trace_LRA_Spill)) {
        fprintf(TFile,"LRA_SPILL>> can not reload registers after a branch.\n");
        Print_BB(bb);
      }
      FmtAssert(!OP_xfer(op),
                ("LRA: Spill at end of block OP:%d in BB:%d\n",
                failure_point, BB_id(bb)));
    }
  }

  /* We compute the dependence graph to help determine the legality 
   * of reordering instructions.
   */
  MEM_POOL op_to_opnum_pool;
  if (need_dep_graph) {
    /* build the OP -> opnum map */
    MEM_POOL_Initialize (&op_to_opnum_pool, "LRA_op_to_opnum", FALSE);
    MEM_POOL_Push (&op_to_opnum_pool);
    op_to_opnum = BB_OP_MAP32_Create (bb, &op_to_opnum_pool);
    for (opnum = 1; opnum <= BB_length(bb); opnum++) {
      OP *op = OP_VECTOR_element (Insts_Vector, opnum);
      BB_OP_MAP32_Set (op_to_opnum, op, opnum);
      /* use OP_flag1 to indicate if an OP has been moved. */
      Reset_OP_flag1 (op);
    }
    CG_DEP_Compute_Graph (
      bb,
      INCLUDE_ASSIGNED_REG_DEPS,
      NON_CYCLIC,
      NO_MEMREAD_ARCS,
      INCLUDE_MEMIN_ARCS,
      NO_CONTROL_ARCS,
      NULL);

    // Mark all the live ranges that have a single definition that can
    // be redone before every use instead of spilling. This is useful
    // for FP loads.
    if (!allow_reordering) Mark_Reloadable_Live_Ranges (cl);
  }

  // compute the set of live ranges that span the failure point.
  Init_Live_LRs_Vector (bb, failure_point, cl, &lra_pool);

  if (Always_Trace(Trace_LRA) && trace_tn && trace_bb == BB_id(bb)) {
    Print_Live_Across();
  }

  INT fatpoint = fat_points[failure_point];
  SPILL_CANDIDATE *transformations = NULL;
  INT first_fatpoint = 1;

  if (Get_Trace (TP_ALLOC, 0x20, bb)) {
    fprintf (TFile, "Fat Points (BB:%d)\n", BB_id(bb));
    for (opnum = 1; opnum <= failure_point; opnum++) {
      OP *op = OP_VECTOR_element(Insts_Vector, opnum);
      fprintf(TFile, "OP:%d [fp=%d]>> ", opnum, fat_points[opnum]);
      Print_OP_No_SrcLine(op);
    }
  }

  //
  // the fatpoint should always be > zero (in fact, its the number of
  // additional registers of the failing class required at the failure
  // point).  all of the operations above the failure point will have a
  // fatpoint value that corresponds to the current deviation from the 
  // requirements at the fat point (0 or less means no spilling needed).
  // if it is not greater than zero, this is usually an indication that
  // the operand requiring the spill is not defined above in the block.
  // lra will allocate and immediately free a register in this case, thus
  // adding no registers and giving a fatpoint of 0.  bump it by one so
  // that we go through the spill loop at least once.
  //
  FmtAssert(fatpoint >= 0,
	    ("LRA: Spill at negative fatpoint at OP:%d in BB:%d\n",
	    failure_point, BB_id(bb)));
  if (fatpoint == 0) {
    DevWarn("Fat point is zero at OP:%d in BB:%d\n", failure_point,
	    BB_id(bb));
    fatpoint = 1;
    fat_points[failure_point] = 1;
  }

  /* traverse back from the instruction where register allocation 
   * failed and spill/reorder for all fatpoints above this.
   */
  fatpoint_min = 0;
  for (opnum = failure_point; opnum > 0; ) {
    FAT_POINTS_TYPE cur_usage = fat_points[opnum];
    BOOL reordering_failure = false;

    // 
    // TODO: modify loop for setting first fatpoint to have same
    //       functionality as code below for decrementing opnum.
    //       will make first_fatpoint more accurate, and thus help
    //	     compile speed.
    //

    // recompute first_fatpoint.
    while (fat_points[first_fatpoint] <= fatpoint_min) {
      first_fatpoint++;
      if (opnum < first_fatpoint) break;
    }
    // if we are already past the first fatpoint, we are done.
    if (opnum < first_fatpoint) break;
    
    // Check if there is a register problem at the current instruction.
    if (fat_points[opnum] > fatpoint_min) {

      SPILL_CANDIDATE *best = TYPE_ALLOCA(SPILL_CANDIDATE);
      best->spill_kind = SPILL_NONE;
      best->spill_num = opnum;

      if (Do_LRA_Trace(Trace_LRA_Spill)) {
	fprintf (TFile,
		 "Analyze >> analyze spilling for OP:%d in BB:%d (fp=%d)\n",
		 opnum, BB_id(bb), fat_points[opnum]);
      }
      if (!allow_reordering && !CG_localize_tns) {
	if (!livethrough_computed) {
	  // compute the set of registers that are livethrough the bb.
	  Compute_Livethrough_Set (bb);
	  livethrough_computed = TRUE;
	}
        Analyze_Spilling_Global_Register (cl, best, opnum, fatpoint);
      }

      // pick the best live range to reorder/spill.
      for (INT i = 0; i < VECTOR_count(Live_LRs_Vector); i++) {
        LIVE_RANGE *clr = LR_VECTOR_element (Live_LRs_Vector, i);
        if (allow_reordering) {
          Analyze_Reordering (clr, best, opnum);
        }
        else {
          Analyze_Spilling_Live_Range (clr, opnum, best, fatpoint);
        }
      }
    
      if (best->spill_kind == SPILL_MOVE_DEF || 
          best->spill_kind == SPILL_MOVE_USE)
      {
        // Delay the reordering till all candidates have been identified.
        // However, update the fat_points array and remove the relevant live 
        // ranges from the Live_LRs_Vector so that they are not considered 
        // again.
        Update_Fat_Points (best, opnum);
        best->next = transformations;
        transformations = best;
      }
      else if (best->spill_kind == SPILL_GLOBAL_REG) {
        Spill_Global_Register (bb, best);
        // Increase the maximum girth allowed, since we now have an 
        // additional register available.  Bump cur_usage, too, so
	// that we don't erroneously report no progress.  Remove the
	// global register from the livethrough set.
        fatpoint_min++;
	cur_usage++;
        livethrough[cl] = REGISTER_SET_Difference1 (
                                livethrough[cl], best->u1.s1.global_spill_reg);
      }
      else if (best->spill_kind == SPILL_LIVE_RANGE) {
        Spill_Live_Range (bb, best->u1.spill_lr, fatpoint, opnum);
        Update_Fat_Points (best, opnum);
      } else if (allow_reordering) {
	// since we don't intermix reordering with other types of
	// spilling, we must allow there to be no progress for a
	// given op
	reordering_failure = true;
      }

      // We have already used the magic spill location. So, don't look
      // for any other spill opportunities.
      // TODO: account for exposed use when updating reg usage.
      if (Trip_Count == MAX_TRIP_COUNT) break;
    }


    if (cur_usage <= fat_points[opnum]) {
      //
      // we're not making progress in reducing the register usage at
      // this operation.  something's hosed, so don't continue trying.
      //
      if (!reordering_failure && Do_LRA_Trace(Trace_LRA_Spill)) {
	fprintf (TFile, "LRA: no progress when spilling at OP:%d in BB:%d\n",
		 opnum, BB_id(bb));
      }
      Update_Live_LRs_Vector (opnum, cl);
      opnum--;
    }

    //
    // find next fatpoint above opnum (current last fatpoint).  note that
    // opnum will not move if the fatpoint that its sitting on has not been
    // sufficiently reduced.  
    //
    while (opnum > 0 && fat_points[opnum] <= fatpoint_min) {
      Update_Live_LRs_Vector (opnum, cl);
      opnum--;
    }
  }

  if (allow_reordering) {
    /* Apply all the transformations needed at once */
    Apply_Move_Transformations (bb, transformations);
  }
  if (need_dep_graph) {
    MEM_POOL_Pop (&op_to_opnum_pool);
    MEM_POOL_Delete (&op_to_opnum_pool);
    CG_DEP_Delete_Graph (bb);
  }

  /* if we spill we need to schedule again */
  Reset_BB_scheduled (bb);
  
  Clear_Fat_Point_Calculation();
}


/* ======================================================================
 * Alloc_Regs_For_BB
 *
 * Do local register allocation for a basic block.
 * ======================================================================*/
void Alloc_Regs_For_BB (BB *bb, HB_Schedule *Sched)
{
  BOOL lra_done;
  TN *tn;
  BOOL redundant_code = FALSE;

  MEM_POOL_Initialize (&lra_pool, "LRA_pool", FALSE);
  CGSPILL_Reset_Local_Spills ();
  Init_Avail_Set (bb);
  local_spills = 0;
  livethrough_computed = FALSE;
  Trip_Count = 0;
  Magic_Spill_Location = Local_Spill_Sym;

  do {
    MEM_POOL_Push (&lra_pool);
    Trip_Count++;
    Init_Avail_Regs ();
    Setup_Live_Ranges (bb, TRUE, &lra_pool);

    if (Always_Trace(Trace_LRA)) {
      if (trace_tn) {
	bb_live = TN_SET_Create_Empty(Last_TN + 1, &lra_pool);
	TN *tn;
	for (tn = GTN_SET_Choose(BB_live_out(bb));
	     tn != GTN_SET_CHOOSE_FAILURE;
	     tn = GTN_SET_Choose_Next(BB_live_out(bb), tn)) {
	  if (LRA_TN_register(tn) != REGISTER_UNDEFINED &&
	      TN_register_class(tn) == trace_cl) {
	    bb_live = TN_SET_Union1D(bb_live, tn, &lra_pool);
	  }
	}
      }
      Print_BB_For_LRA (bb);
      if (Do_LRA_Trace(Trace_LRA_Detail)) {
        Print_Live_Ranges (bb);
      }
    }

    lra_done = Assign_Registers (bb, &tn, &redundant_code);
    if (!lra_done) {
      Fix_LRA_Blues (bb, tn, Sched);
    }
    MEM_POOL_Pop (&lra_pool);
  } while (!lra_done);

  if (redundant_code) Remove_Redundant_Code (bb);

  MEM_POOL_Delete (&lra_pool);
  Set_BB_reg_alloc (bb);

  if (Always_Trace(Trace_LRA)) {
    if (Trip_Count > 1) {

      fprintf(TFile,"%d iterations and %d spills required to allocate regs for BB:%d\n",
              Trip_Count,local_spills,BB_id(bb));
    }
  }

}


/******************* Save/restore of Callee Saved Regs ********************/


// Allocate registers for any unallocated TNs in <ops>.
// <bb> is used to determine what temps are available.
void 
Assign_Temp_Regs (OPS *ops, BB *bb)
{
  REGISTER_SET temps[ISA_REGISTER_CLASS_MAX+1];
  OP *op;
  OP *sp_adj;

  if (BB_entry(bb)) {
        sp_adj = BB_entry_sp_adj_op(bb);
        REG_LIVE_Prolog_Temps (bb, sp_adj, sp_adj, temps);
  }
  else if (BB_exit(bb)) {
        sp_adj = BB_exit_sp_adj_op(bb);
        REG_LIVE_Epilog_Temps (Get_Current_PU_ST(), bb, sp_adj, temps);
  }
  else {
        FmtAssert(FALSE, ("unexpected bb"));
  }
  FOR_ALL_OPS_OPs (ops, op) {
    REGISTER_CLASS_OP_Update_Mapping(op);
    for (INT i = 0; i < OP_results(op); i++) {
      TN *result_tn = OP_result(op, i);
      if (LRA_TN_register(result_tn) == REGISTER_UNDEFINED) {
        ISA_REGISTER_CLASS cl = TN_register_class(result_tn);
        REGISTER reg = REGISTER_SET_Choose (temps[cl]);
        FmtAssert (reg != REGISTER_UNDEFINED, ("no temp regs available"));
        LRA_TN_Allocate_Register (result_tn, reg);
        temps[cl] = REGISTER_SET_Difference1 (temps[cl], reg);
      }
    }
  }
}


/* ====================================================================
 * Prolog_save_code - generate save code for callee-saved regs
 * ==================================================================== */
static void
Prolog_save_code(BB *entrybb)
{
  INT callee_num;

  for (callee_num = 0; callee_num < Callee_Saved_Regs_Count; callee_num++) {
    TN *tn = CALLEE_tn(callee_num);
    ISA_REGISTER_CLASS cl = TN_save_rclass(tn);
    REGISTER reg = TN_save_reg(tn);
    if (REGISTER_SET_MemberP(Callee_Saved_Regs_Used[cl], reg)) {
      ST *mem_loc = CGSPILL_Get_TN_Spill_Location (tn, CGSPILL_LCL);
      OPS ops = OPS_EMPTY;
      LRA_TN_Allocate_Register (tn, reg);
      /* Generate the spill ops */
      CGSPILL_Store_To_Memory (tn, mem_loc, &ops, CGSPILL_LRA, entrybb);

      /* allocate registers for any temps used in spill sequence */
      Assign_Temp_Regs (&ops, entrybb);

      /* insert the ops in the op list for the current BB */
      CGSPILL_Prepend_Ops (entrybb, &ops);
  
      if ( Do_LRA_Trace(Trace_LRA_Entry_Exit) )
        fprintf ( TFile,
                  "entry BB:%d saved TN%d saved from reg %d:%d\n", 
                  BB_id(entrybb), TN_number(tn), cl, reg );
    }
  }
}

/* ====================================================================
 * Epilog_restore_code - generate restore code for callee-saved regs
 * ==================================================================== */
static void
Epilog_restore_code(BB *exitbb)
{
  /* check for need to generate restore code for callee-saved regs */
  INT callee_num;

  for (callee_num = 0; callee_num < Callee_Saved_Regs_Count; callee_num++) {
    TN *tn = CALLEE_tn(callee_num);
    ISA_REGISTER_CLASS cl = TN_save_rclass(tn);
    REGISTER reg = TN_save_reg(tn);
    if (REGISTER_SET_MemberP(Callee_Saved_Regs_Used[cl], reg)) {
      ST *mem_loc = CGSPILL_Get_TN_Spill_Location (tn, CGSPILL_LCL);
      OPS ops = OPS_EMPTY;
      /* generate the reload ops */
      CGSPILL_Load_From_Memory (tn, mem_loc, &ops, CGSPILL_LRA, exitbb);

      /* allocate registers for any temps used in spill sequence */
      Assign_Temp_Regs (&ops, exitbb);

      /* insert the ops in the op list for the current BB */
      CGSPILL_Append_Ops (exitbb, &ops);
  
      if (Do_LRA_Trace(Trace_LRA_Entry_Exit))
        fprintf ( TFile,
                  "exit BB:%d saved TN%d restored to reg %d:%d\n", 
              BB_id(exitbb), TN_number(tn), cl, reg );
    }
  }
}

/* 
 * Insert code at entry and exit to save/restore any callee-saved registers
 * that were used.  This is done only for the Localize_TNs case.
 */
static void 
Spill_Callee_Saved_Regs (void)
{
  ISA_REGISTER_CLASS cl;
  BB_LIST *elist;

  if (Do_LRA_Trace(Trace_LRA_Entry_Exit)) {
    fprintf(TFile, "Callee_Saved_Regs_Used:\n");
    FOR_ALL_ISA_REGISTER_CLASS(cl) {
      fprintf(TFile, "  class %d: ", cl);
      REGISTER_SET_Print(Callee_Saved_Regs_Used[cl], TFile);
      fprintf(TFile, "\n");
    }
  }

  for ( elist = Entry_BB_Head; elist; elist = BB_LIST_rest(elist) ) {
    Prolog_save_code (BB_LIST_first(elist));
  }

  for ( elist = Exit_BB_Head; elist; elist = BB_LIST_rest(elist) ) {
    Epilog_restore_code (BB_LIST_first(elist));
  }

  /* Reset the Callee_Saved_Regs_Used set for the next procedure. */
  FOR_ALL_ISA_REGISTER_CLASS(cl)
    Callee_Saved_Regs_Used[cl] = REGISTER_SET_EMPTY_SET;
}


/* Consistency check to make sure that local TNs are used only in one 
 * basic block. 
 */
static void Consistency_Check (void)
{
  BB *bb;
  TN_MAP Local_TNs;

  Local_TNs = TN_MAP_Create ();
  for (bb = REGION_First_BB; bb != NULL; bb = BB_next(bb)) {
    OP *op;
    INT i;
    FOR_ALL_BB_OPs_FWD (bb, op) {

      for (i = 0; i < OP_results(op); i++) {
        TN *tn = OP_result(op, i);
        if (TN_is_local_reg(tn)) {
          BB *tn_bb = (BB *)TN_MAP_Get (Local_TNs, tn);
          if (tn_bb != NULL) {
            if (tn_bb != bb &&
                (!BB_reg_alloc(bb) || !BB_reg_alloc(tn_bb)))
              FmtAssert (FALSE, 
                      ("Local TN%d referenced in BB:%d and BB:%d",
                        TN_number(tn), BB_id(tn_bb), BB_id(bb)));
          }
          else {
            TN_MAP_Set (Local_TNs, tn, bb);
          }
        }
        else {
          FmtAssert (LRA_TN_register(tn) != REGISTER_UNDEFINED,
                ("Global TN%d not assigned a register", TN_number(tn)));
        }
      }

      for (i = 0; i < OP_opnds(op); i++) {
        TN *tn = OP_opnd(op,i);
        if (TN_is_register(tn)) {
          if (TN_is_local_reg(tn)) {
            BB *tn_bb = (BB *)TN_MAP_Get (Local_TNs, tn);
            if (tn_bb != NULL) {
              if (tn_bb != bb &&
                  (!BB_reg_alloc(bb) || !BB_reg_alloc(tn_bb)))
                FmtAssert (FALSE, 
                      ("Local TN%d referenced in BB:%d and BB%d",
                        TN_number(tn), BB_id(tn_bb), BB_id(bb)));
            }
            else {
              TN_MAP_Set (Local_TNs, tn, bb);
            }
          }
          else {
            FmtAssert (LRA_TN_register(tn) != REGISTER_UNDEFINED,
                ("Global TN%d not assigned by GRA", TN_number(tn)));
        }
        }
      }

    }
  }
  TN_MAP_Delete (Local_TNs);
}


// Move the spill loads/stores introduced by GRA right next to their
// corresponding uses and defs.
static void
Move_Spill_Loads_Stores (BB *bb)
{

  OP *op; 
  OP *prev_op = NULL;
  OP *next_op = NULL;

  // Move the delay slot instruction (if any) above the terminating 
  // branch. This makes the spill loads added at end of bb to be in
  // right place. Do it only if the branch has a delay slot.
  if (PROC_has_branch_delay_slot())
    BB_Move_Delay_Slot_Op (bb);

  // Make a forward pass through the basic block. Keep track of the
  // definitions in the bb. If we see a spill store, move it right
  // next to the previous definition. If there is no previous 
  // definition in the bb, move the store to the top of the bb.
  TN_MAP spills_map = TN_MAP_Create ();
  for (op = BB_first_op(bb); op != NULL; op = next_op) {
    next_op = OP_next(op);
    if (OP_has_result(op)) {
      for (INT i = 0; i < OP_results(op); i++) {
	TN *result_tn = OP_result(op, i);
	TN_MAP_Set (spills_map, result_tn, op);
      }
    }
    else if (OP_store(op)) {
      TN *src_tn = OP_opnd(op,TOP_Find_Operand_Use(OP_code(op), OU_storeval));
      ST *spill_loc = TN_spill(src_tn);
      if (Is_OP_Spill_Store (op, spill_loc)) {
        OP *def_op = (OP *)TN_MAP_Get (spills_map, src_tn);
        if (def_op != OP_prev(op)) {
          OPS ops = OPS_EMPTY;
          BB_Remove_Op (bb, op);
          if (Do_LRA_Trace(Trace_Move_GRA_Spills)) {
            fprintf (TFile, ">> MOVE Delete");
            Print_OP_No_SrcLine (op);
          }
          CGSPILL_Store_To_Memory (src_tn, spill_loc, &ops, CGSPILL_LRA, bb);
          if (def_op != NULL) {
            CGSPILL_Insert_Ops_After (bb, def_op, &ops);
            if (Do_LRA_Trace(Trace_Move_GRA_Spills)) {
              fprintf (TFile, ">> MOVE Append");
              Print_OP_No_SrcLine (OPS_first(&ops));
              fprintf (TFile, ">> MOVE After ");
              Print_OP_No_SrcLine (def_op);
            }
          }
          else {
            CGSPILL_Prepend_Ops (bb, &ops);
            if (Do_LRA_Trace(Trace_Move_GRA_Spills)) {
              fprintf (TFile, ">> MOVE_SPILL Prepend");
              Print_OP_No_SrcLine (OPS_first(&ops));
            }
          }
        }
      }
    }
  }
  TN_MAP_Delete (spills_map);

  // Make a backward pass through the basic block. Keep track of all 
  // the uses of register TNs. If we see a spill load, move it right
  // next to the next use. If there are no later uses in the bb, move
  // the spill load to the end of the bb.
  spills_map = TN_MAP_Create ();
  for (op = BB_last_op(bb); op != NULL; op = prev_op) {
    prev_op = OP_prev(op);
    INT i;
    for (i = 0; i < OP_opnds(op); i++) {
      TN *tn = OP_opnd(op,i);
      if (TN_is_register(tn)) {
        TN_MAP_Set (spills_map, tn, op);
      }
    }
    for (i = 0; i < OP_results(op); i++) {
      TN *result_tn = OP_result(op, i);
      ST *spill_loc = TN_spill(result_tn);
      if (Is_OP_Spill_Load (op, spill_loc)) {
        OP *use_op = (OP *)TN_MAP_Get (spills_map, result_tn);
        if (use_op != OP_next(op)) {
          OPS ops = OPS_EMPTY;
          BB_Remove_Op (bb, op);
          CGSPILL_Load_From_Memory (result_tn, spill_loc, &ops, CGSPILL_LRA,
				    bb);
          if (use_op != NULL) {
            CGSPILL_Insert_Ops_Before (bb, use_op, &ops);
          }
          else {
            CGSPILL_Append_Ops (bb, &ops);
          }
        }
      }
      //
      // make this definition the most recent use of the tn.  this is
      // to prevent useless reloads at the top of blocks from moving
      // past the redefinition of the tn.
      //
      TN_MAP_Set (spills_map, result_tn, op);
    }
  }
  TN_MAP_Delete (spills_map);

  if (Do_LRA_Trace(Trace_Move_GRA_Spills)) {
    Print_BB (bb);
  }
}


static REGISTER
Single_Register_Subclass (ISA_REGISTER_SUBCLASS subclass)
{
  REGISTER_SET subclass_regs = REGISTER_SUBCLASS_members(subclass);
  if (REGISTER_SET_Size(subclass_regs) == 1) {
    return REGISTER_SET_Choose(subclass_regs);
  }
  return REGISTER_UNDEFINED;
}
  

static void
Preallocate_Single_Register_Subclasses (BB* bb)
{
  OP* op;
  FOR_ALL_BB_OPs (bb, op) {

    ASM_OP_ANNOT* asm_info = (OP_code(op) == TOP_asm) ?
      (ASM_OP_ANNOT*) OP_MAP_Get(OP_Asm_Map, op) : NULL;

    INT i;    
    for (i = 0; i < OP_opnds(op); i++) {
      ISA_REGISTER_SUBCLASS subclass = asm_info ?
        ASM_OP_opnd_subclass(asm_info)[i] : OP_opnd_reg_subclass(op, i);
      REGISTER reg = Single_Register_Subclass(subclass);
      if (reg == REGISTER_UNDEFINED) {
        continue;
      }
      TN* old_tn = OP_opnd(op, i);
      if (LRA_TN_register(old_tn) != REGISTER_UNDEFINED) {
        Is_True(LRA_TN_register(old_tn) == reg,
                ("LRA: wrong register allocated to TN"));
        continue;
      }
      TN* new_tn = Build_TN_Like(old_tn);
      LRA_TN_Allocate_Register(new_tn, reg);
      INT j;    
      for (j = 0; j < OP_opnds(op); j++) {
        if (OP_opnd(op, j) == old_tn) {
          Set_OP_opnd(op, j, new_tn);
        }
      }
      BOOL has_def = FALSE;
      for (j = 0; j < OP_results(op); j++) {
        if (OP_result(op, j) == old_tn) {
          Set_OP_result(op, j, new_tn);
          has_def = TRUE;
        }
      }
      OPS pre_ops = OPS_EMPTY;
      Exp_COPY(new_tn, old_tn, &pre_ops);
      BB_Insert_Ops_Before(bb, op, &pre_ops);
      if (has_def) {
        OPS post_ops = OPS_EMPTY;
        Exp_COPY(old_tn, new_tn, &post_ops);
        BB_Insert_Ops_After(bb, op, &post_ops);
      }
    }

    for (i = 0; i < OP_results(op); i++) {
      ISA_REGISTER_SUBCLASS subclass = asm_info ?
        ASM_OP_result_subclass(asm_info)[i] : OP_result_reg_subclass(op, i);
      REGISTER reg = Single_Register_Subclass(subclass);
      if (reg == REGISTER_UNDEFINED) {
        continue;
      }
      TN* old_tn = OP_result(op, i);
      if (LRA_TN_register(old_tn) != REGISTER_UNDEFINED) {
        Is_True(LRA_TN_register(old_tn) == reg,
                ("LRA: wrong register allocated to TN"));
        continue;
      }
      TN* new_tn = Build_TN_Like(old_tn);
      LRA_TN_Allocate_Register(new_tn, reg);
      INT j;    
      for (j = 0; j < OP_results(op); j++) {
        if (OP_result(op, j) == old_tn) {
          Set_OP_result(op, j, new_tn);
        }
      }
      BOOL has_use = FALSE;
      for (j = 0; j < OP_opnds(op); j++) {
        if (OP_opnd(op, j) == old_tn) {
          Set_OP_opnd(op, j, new_tn);
          has_use = TRUE;
        }
      }
      OPS post_ops = OPS_EMPTY;
      Exp_COPY(old_tn, new_tn, &post_ops);
      BB_Insert_Ops_After(bb, op, &post_ops);
      if (has_use) {
        OPS pre_ops = OPS_EMPTY;
        Exp_COPY(new_tn, old_tn, &pre_ops);
        BB_Insert_Ops_Before(bb, op, &pre_ops);
      }
    }
  }
}

/* ======================================================================
 * LRA_Allocate_Registers
 *
 * Main driver for the local register allocation module. 
 * ======================================================================*/
void
LRA_Allocate_Registers (BOOL lra_for_pu)
{
  BB *bb;
  RID *rid;
  ISA_REGISTER_CLASS cl;

  Set_Error_Phase ( "Local Register Allocation" );
  Start_Timer ( T_LRA_CU );

#if Is_True_On 
  Consistency_Check ();
#endif

/* Initialize the unused TN definitions. */
  FOR_ALL_ISA_REGISTER_CLASS(cl) unused_tn_def[cl] = NULL;
  if (True_TN) unused_tn_def[TN_register_class(True_TN)] = True_TN;
  if (Zero_TN) unused_tn_def[TN_register_class(Zero_TN)] = Zero_TN;
  if (FZero_TN) unused_tn_def[TN_register_class(FZero_TN)] = FZero_TN;

  FOR_ALL_ISA_REGISTER_CLASS(cl) last_assigned_reg[cl] = REGISTER_UNDEFINED;
  HB_Schedule *Sched = NULL;
  global_spills = 0;

  for (bb = REGION_First_BB; bb != NULL; bb = BB_next(bb)) 
  {
    if (    ( rid = BB_rid(bb) )
         && ( RID_level(rid) >= RL_CGSCHED ) )
      continue;

    /* Someone already did it -- probably SWP */
    if ( BB_reg_alloc(bb) )
      continue;

    Trace_LRA = Get_Trace (TP_ALLOC, 0x01, bb);
    Trace_LRA_Detail = Get_Trace (TP_ALLOC, 0x02, bb);
    Trace_LRA_Spill = Get_Trace (TP_ALLOC, 0x04, bb);
    Trace_LRA_Entry_Exit = Get_Trace (TP_ALLOC, 0x08, bb);
    Trace_Move_GRA_Spills = Get_Trace (TP_ALLOC, 0x10, bb);

#ifdef TARG_IA32
    Preallocate_Single_Register_Subclasses (bb);
#endif

    Alloc_Regs_For_BB (bb, Sched);
  }

#ifdef TARG_ST100
  //
  // count callee save registers used
  //
  FOR_ALL_ISA_REGISTER_CLASS(cl) {
    Callee_Saved_Regs_Mask[cl] = 
        REGISTER_SET_Union(Callee_Saved_Regs_Mask[cl], 
			   Callee_Saved_Regs_Used[cl]);
  }
#endif

  // If we are not running GRA, we need to generate the instructions to
  // save and restore callee saved registers.
#ifdef TARG_ST100
  //
  // Arthur: if I am generating register save mask, no need to spill
  //
  if (lra_for_pu && CG_localize_tns && !CG_gen_callee_saved_regs_mask) {
#else
  if (lra_for_pu && CG_localize_tns) {
#endif
    Spill_Callee_Saved_Regs ();
  }

  Check_for_Dump (TP_ALLOC, NULL);
  Stop_Timer ( T_LRA_CU );
}



static mINT8 *Reg_Request_Table;
#define Reg_Request(bb,cl)      \
  (Reg_Request_Table[BB_id(bb)*ISA_REGISTER_CLASS_COUNT + cl - ISA_REGISTER_CLASS_MIN])

static INT Reg_Table_Size;

// Initialize the LRA data structures at the start of each PU/region. 
void
LRA_Init (void)
{
  Reg_Request_Table = NULL;
  Reg_Table_Size = 0;
}


/* estimate the maximum number of local registers required in each register
 * class for this basic block. If (regs_in_use != NULL) report the total
 * number of registers in use before this instruction is executed.
 */
void
LRA_Estimate_Fat_Points (
  BB *bb, 
  mINT8 *fatpoint,
  INT *regs_in_use,
  MEM_POOL* pool)
{
  INT inuse[ISA_REGISTER_CLASS_MAX+1];
  OP *op;
  INT i, opnum, opndnum;
  INT tot_use = 0;

  Setup_Live_Ranges (bb, FALSE, pool);

  if (Do_LRA_Trace(Trace_LRA_Detail)) {
    Print_Live_Ranges (bb);
  }

  for (i = ISA_REGISTER_CLASS_MIN; i <= ISA_REGISTER_CLASS_MAX; i++) {
    fatpoint[i] = 0;
    inuse[i] = 0;
  }

  for (opnum = BB_length(bb); opnum > 0; opnum--) 
  {
    op = OP_VECTOR_element(Insts_Vector, opnum);
    if (regs_in_use) regs_in_use[opnum] = tot_use;  /* In case there are no new uses and no defs. */

    for (i = 0; i < OP_results(op); i++) {
      TN *result_tn = OP_result(op, i);
      ISA_REGISTER_CLASS regclass = TN_register_class(result_tn);
      LIVE_RANGE *clr = LR_For_TN (result_tn);
      if (opnum == LR_first_def(clr)) {
        if ((LR_assigned(clr) ||
	     (!regs_in_use && !TN_is_local_reg(result_tn)))) 
        {
          inuse[regclass]--;
	  if (regs_in_use &&
              (LR_last_use(clr) != BB_length(bb)+1) &&
	      !LR_exposed_use(clr) &&
              (LR_last_use(clr) != LR_first_def(clr))){
	    regs_in_use[opnum] = --tot_use;
	  }
        }
      }

      /* reserve at least one register for this regclass. */
      if (fatpoint[regclass] == 0) fatpoint[regclass] = 1;
    }

    /* process all the operand TNs. */
    for (opndnum = 0; opndnum < OP_opnds(op); opndnum++) {
      TN *tn = OP_opnd(op, opndnum);
      if (TN_is_register(tn)) {
        ISA_REGISTER_CLASS regclass = TN_register_class(tn);
        LIVE_RANGE *clr = LR_For_TN (tn);
        if (!LR_assigned(clr)) {
          if (TN_is_local_reg(tn) || (!regs_in_use &&
              ((LR_def_cnt(clr) != 0 && opnum <= LR_exposed_use(clr)) ||
	      (OP_copy(op) && !TN_is_local_reg(OP_result(op,0))))))
          {
            Set_LR_assigned(clr);
	    inuse[regclass]++;
	    if (regs_in_use &&
                (LR_last_use(clr) != BB_length(bb)+1) &&
	        !LR_exposed_use(clr) &&
                (LR_last_use(clr) != LR_first_def(clr))){
		regs_in_use[opnum] = ++tot_use;
	    }
            if (fatpoint[regclass] < inuse[regclass]) {
              if (inuse[regclass] <= 127) {
                fatpoint[regclass] = inuse[regclass];
              }
            }
          }
        }
        /* reserve at least one register for this regclass. */
        if (fatpoint[regclass] == 0) fatpoint[regclass] = 1;
      }
    }
  }
  if (Get_Trace (TP_ALLOC, 0x20, bb)) {
    fprintf (TFile, "Fat Points (BB:%d)\t", BB_id(bb));
    for (i = ISA_REGISTER_CLASS_MIN; i <= ISA_REGISTER_CLASS_MAX; i++) {
      fprintf (TFile, "[%d]:%d  ", i, fatpoint[i]);
    }
    fprintf (TFile, "\n");
  }
}


static
mINT8
LRA_examine_last_op_needs (BB *bb, ISA_REGISTER_CLASS cl)
{
  OP *last_op = BB_last_op (bb);
  mINT8 min_regs = 0;

  if (PROC_has_branch_delay_slot() &&
      (last_op != NULL) &&
      (OP_prev(last_op) != NULL)) last_op = OP_prev(last_op);

  if (last_op != NULL && OP_xfer(last_op)) {
    INT i;
    TN *tn;

    for (i = 0; i < OP_opnds(last_op); i++) {
      tn = OP_opnd(last_op, i);
      if ((tn != NULL) &&
          !TN_is_constant(tn) &&
          !TN_is_dedicated(tn) &&
          !(LRA_TN_register(tn) != REGISTER_UNDEFINED) &&
           (TN_register_class(tn) == cl)) min_regs++;
    }

  }

#ifdef TARG_ST100
  // Arthur: get rid of target dependency
  if ((min_regs == 0) && REGISTER_CLASS_is_ptr(cl)) {
#else
  if ((min_regs == 0) && (cl == ISA_REGISTER_CLASS_integer)) {
#endif
   /* We need to have at least 1 so that we can insert a spill, if needed. */
    min_regs = 1;
  }

  return min_regs;
}


mINT8 *
LRA_Compute_Register_Request (BB *bb, MEM_POOL *pool)
{
  // Allocate the Reg_Request_Table if not yet allocated.
  if (Reg_Request_Table == NULL) {
    /* this needs to be initialized to 0, so use a zeroing pool */
    Reg_Request_Table = 
          TYPE_PU_ALLOC_N (mINT8, (PU_BB_Count+2)*ISA_REGISTER_CLASS_COUNT);
    Reg_Table_Size = PU_BB_Count;
  }
  Is_True (BB_id(bb) <= Reg_Table_Size, ("incorrect BB_id: %d", BB_id(bb)));

  mINT8 *fatpoint = &Reg_Request(bb,0);
  ISA_REGISTER_CLASS int_regclass = TN_register_class(SP_TN);

  LRA_Estimate_Fat_Points (bb, fatpoint, NULL, pool);

  /* Reserve at least 2 integer registers for LRA.  This is the minimum
   * required to allocate the operands of a conditional branch. Any spill
   * code that LRA introduces is above a terminating branch.
   * TODO: check if the basic block has a terminating branch with two
   * operands. For other cases, we only need to reserve 1 integer register.
   * That register might be required as a temp while generating spill code.
   */
  if (fatpoint[int_regclass] < 2) fatpoint[int_regclass] = LRA_examine_last_op_needs (bb, int_regclass);

  return fatpoint;
}


/* ======================================================================
 * LRA_Register_Request
 *
 * Returns the number of registers LRA is requesting from GRA for
 * the class <cl> in the basic block <bb>. If we run the scheduling
 * pass before register allocation for the bb, this returns an 
 * accurate estimate of how many registers LRA needs. Otherwise,
 * it is just a fixed number based on some heuristics.
 * ======================================================================*/
INT
LRA_Register_Request (BB *bb,  ISA_REGISTER_CLASS cl)
{
  INT regs_needed;

  FmtAssert (!BB_reg_alloc(bb), ("LRA_Register_Request: Invalid bb %d", BB_id(bb)));

  if (BB_id(bb) <= Reg_Table_Size) {
    /* Note: Re-work the estimation of minimum registers needed.
     *
     * Since register re-loads can not be inserted after a branch statement,
     * the minimal number of registers that we need is equivalent to the
     * number needed in the branch instruction.  Examine that instruction
     * and determine the number needed in each register class.
     */

    mINT8 min_regs = LRA_examine_last_op_needs (bb, cl);
    regs_needed = Reg_Request (bb, cl);
    if (regs_needed < min_regs) {
      regs_needed = min_regs;
      Reg_Request (bb, cl) = min_regs;
    }
  }
  else {
    /* TODO: tune this based on register class and length of basic block */ 
    regs_needed = 5;
  }
  return regs_needed;
}

