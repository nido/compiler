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


// =======================================================================
// =======================================================================
//
//  Module: hb_sched.cxx
//
//  Description:
//  ============
//
//  Hyberblock (HB) Scheduling routines.
//
// =======================================================================
// =======================================================================

#include "W_alloca.h"
#include "W_math.h"
#include "defs.h"
#include "config.h"
#include "config_TARG.h"
#include "mempool.h"
#include "bb.h"
#include "bb_set.h"
#include "tracing.h"
#include "timing.h"
#include "cgir.h"
#include "glob.h"
#include "tn_map.h"
#include "cg.h"
#include "cg_flags.h"
#include "ercg.h"
#include "cgtarget.h"
#include "cg_vector.h"
#include "dominate.h"
#include "findloops.h"
#include "note.h"
#include "lra.h"
#include "gcm.h"
#include "ti_res.h"
#include "ti_res_res.h"
#include "ti_latency.h"
#include "ti_errors.h"
#include "cg_region.h"
#include "gtn_universe.h"
#include "gtn_set.h"
#include "cxx_memory.h"
#include "hb_sched.h"
#include "hb_hazards.h"
/* #include "targ_proc_properties.h" */ /* Target dependent ! */
#include "targ_isa_bundle.h"
#include "ti_bundle.h"
#include "whirl2ops.h"
#ifdef TARG_ST
#include "data_layout.h"
#endif

// ======================================================================
// Declarations (macros, variables)
// ======================================================================

BOOL Trace_HB = FALSE;

static INT BBs_Processed = 0;

// The current cycle in which we are trying to schedule OPs.
static INT Clock;
static INT MAX_Clock;
#ifdef TARG_ST
static INT32 computed_max_sched = INT32_MAX;
// FdF 15/12/2003: We need to know if we are in pre-pass or post-pass
// scheduling, because optimizations with SP_Sym in Ldst_Addiu_Pair
// are only valid in post-pass.
static BOOL Before_LRA;
#endif

static void
Print_OPSCH (OP *op, BB_MAP value_map)
{
  OPSCH *opsch = OP_opsch(op, value_map);
  Print_OP_No_SrcLine (op);
  fprintf (TFile, "\t<dfs:%3d cyc:%2d reg:%2d est:%2d lst:%2d succs:%d preds:%d flgs:%d>\n", 
    OPSCH_dfsnum(opsch), OPSCH_scycle(opsch), OPSCH_regcost(opsch),
    OPSCH_estart(opsch), OPSCH_lstart(opsch), 
    OPSCH_num_succs(opsch), OPSCH_num_preds(opsch), OPSCH_flags(opsch));
}

static void
Print_BB_For_HB (BB *bb, BB_MAP value_map)
{
  OP *op;

  fprintf (TFile, "*************** BB:%d ******************\n", BB_id(bb));
  FOR_ALL_BB_OPs_FWD (bb, op) {
    Print_OPSCH (op, value_map);
  }
  fprintf (TFile, "****************************************\n");
}

void
Print_BB_For_HB (std::list<BB*> bblist, BB_MAP value_map)
{
  std::list<BB*>::iterator bbiter;

  fprintf (TFile, "\n********** HyperBlock (HB) ******************\n");
  fprintf (TFile, "******* Contains :");
  FOR_ALL_BB_STLLIST_ITEMS_FWD(bblist, bbiter) {
    fprintf (TFile, " BB:%d ", BB_id(*bbiter));
  }
  fprintf (TFile, "**********\n");

  CG_DEP_Trace_HB_Graph (bblist);
  FOR_ALL_BB_STLLIST_ITEMS_FWD(bblist, bbiter) {
    Print_BB_For_HB(*bbiter, value_map);
  }
  fprintf (TFile, "****************************************\n");

}

// ======================================================================
// Check to see if there is a need to reschedule this block. Sometimes
// it's better to reschedule the block with different heuristics (#622253). 
// Currently, we limit it to single-BB loops where the benefits are more
// pronounced.
// ======================================================================
BOOL
Reschedule_BB(BB *bb)
{

  // always reschedule on stxp70 [vcdv]
#ifdef TARG_STxP70
  return TRUE;
#else
  // At the moment, target single_BB loops ONLY.
  if (BB_loop_head_bb(bb) == bb) {
    BBLIST *succ_list;
    FOR_ALL_BB_SUCCS (bb, succ_list) {
      BB *succ_bb = BBLIST_item(succ_list);
      if (succ_bb == bb) return TRUE;
    }
  }

  return FALSE;
#endif
}

// ======================================================================
// Check to see if the given HB can be scheduled, i.e prior not SWP'd.
// ======================================================================
BOOL
Can_Schedule_HB(std::list<BB*> hb_blocks)
{

  std::list<BB*>::iterator bb_iter;
  FOR_ALL_BB_STLLIST_ITEMS_FWD (hb_blocks, bb_iter) {
    // if <reschedule> flag is already set, then return FALSE.
    if (BB_scheduled(*bb_iter) && !BB_scheduled_hbs(*bb_iter)) return FALSE;
  }

  return TRUE;
}

//
// Return true if op is a ne/eq compare with a constant
//
static BOOL
OP_Is_Cmp_Eq_Ne(OP *op)
{
  VARIANT variant;
  TOP opcode = OP_code(op);
  INT opnd1_idx, opnd2_idx;
  
  if (OP_icmp(op)) {
    opnd1_idx = OP_find_opnd_use(op, OU_opnd1);
    opnd2_idx = OP_find_opnd_use(op, OU_opnd2);
#ifdef TARG_ST
    variant = OP_cmp_variant(op);
#else
    variant = TOP_cmp_variant(opcode);
#endif
    if (variant == V_CMP_EQ ||
	variant == V_CMP_NE) {
      if (TN_has_value(OP_opnd(op, opnd2_idx)))
	return TRUE;
    }
  }
  return FALSE;
}

//
// Returns true if op is a self add with a constant: rx = rx + cst
//
static BOOL
OP_Is_Addr_Incr(OP *op)
{
  TOP opcode = OP_code(op);
  INT opnd1_idx, opnd2_idx;
  if (OP_iadd(op)) {
    opnd1_idx = OP_find_opnd_use(op, OU_opnd1);
    opnd2_idx = OP_find_opnd_use(op, OU_opnd2);
    if (TN_has_value(OP_opnd(op, opnd2_idx)) &&
	OP_result(op, 0) == OP_opnd(op, opnd1_idx))
      return TRUE;
  }
  return FALSE;
}

INT
Memory_OP_Base_Opndnum (OP *op)
{
  INT opnd_num;
#ifdef TARG_ST
  opnd_num = TOP_Find_Operand_Use(OP_code(op), OU_base);
#else
  if (OP_store(op) || OP_prefetch(op)) {
    opnd_num = 1;
  }
  else {
    Is_True (OP_load(op), ("OP not a memory OP."));
    opnd_num = 0;
  }
#endif
  return opnd_num;
}


INT
Memory_OP_Offset_Opndnum (OP *op)
{
  INT opnd_num;
#ifdef TARG_ST
  opnd_num = TOP_Find_Operand_Use(OP_code(op), OU_offset);
#else
  if (OP_store(op) || OP_prefetch(op)) {
    opnd_num = 2;
  }
  else {
    Is_True (OP_load(op), ("OP not a memory OP."));
    opnd_num = 1;
  }
#endif
  return opnd_num;
}

// ======================================================================
// Initialize <regs_map> for the basic block. The defs for each TN
// are counted. Also mark all global TNs as having a register 
// already assigned to them.
// ======================================================================
void HB_Schedule::Init_Register_Map (BB *bb)
{
  OP *op;

  _regs_map = hTN_MAP_Create (&_hb_pool);
  FOR_ALL_BB_OPs_FWD (bb, op) {
    INT i;
    for (i = 0; i < OP_results(op); i++) {
      TN *result_tn = OP_result(op, i);
      REG_ENTRY reginfo;
      REG_ENTRY_ptr(reginfo) =  hTN_MAP_Get (_regs_map, result_tn);
      REG_ENTRY_def_count(reginfo)++;
      if (TN_is_global_reg(result_tn) || TN_is_dedicated(result_tn)) 
	REG_ENTRY_reg_assigned(reginfo) = TRUE;
      hTN_MAP_Set (_regs_map, result_tn, REG_ENTRY_ptr(reginfo));
    }
    for (i = 0; i < OP_opnds(op); i++) {
      TN *opnd_tn = OP_opnd(op,i);
      if (TN_is_constant(opnd_tn)) continue;
      if (TN_is_global_reg(opnd_tn) || TN_is_dedicated(opnd_tn)) {
        REG_ENTRY reginfo;
        REG_ENTRY_ptr(reginfo) = hTN_MAP_Get (_regs_map, opnd_tn);
        REG_ENTRY_reg_assigned(reginfo) = TRUE;
        hTN_MAP_Set (_regs_map, opnd_tn, REG_ENTRY_ptr(reginfo));
      }
    }
  }
}

// ======================================================================
// Estimate the register cost for scheduling <op> next. This cost depends
// on the following factors:
//  - how many registers are available for allocation at this point ?
//  - how many registers are used and freed up by <op> ?
//  - is the number of registers available below a threshold ?
// ======================================================================
void
HB_Schedule::Estimate_Reg_Cost_For_OP (OP *op)
{
  INT cost = 0;
  INT32 local_regs_avail[ISA_REGISTER_CLASS_MAX+1];
  ISA_REGISTER_CLASS cl;
  REG_ENTRY reginfo;

  FOR_ALL_ISA_REGISTER_CLASS(cl) {
    local_regs_avail[cl] = _Cur_Regs_Avail[cl];
  }

  INT i;
  for (i = 0; i < OP_results(op); i++) {
    TN *result_tn = OP_result(op, i);
    // If the result tn is also referenced in the OP, don't consider the 
    // register being freed above the def.
    if (!OP_Refs_TN (op, result_tn)) {
      REG_ENTRY_ptr(reginfo) = hTN_MAP_Get (_regs_map, result_tn);
      if (REG_ENTRY_def_count(reginfo) == 1 &&
	  REG_ENTRY_reg_assigned(reginfo)) 
      {
        cl = TN_register_class(result_tn);
	INT reg_pressure = (2 - local_regs_avail[cl]);
	if (reg_pressure > 0) {
	  cost -= reg_pressure;
	}
	local_regs_avail[cl]++;
      }
    }
  }
  for (i = 0; i < OP_opnds(op); i++) {
    TN *opnd_tn = OP_opnd(op,i);
    if (TN_is_constant(opnd_tn)) continue;
    REG_ENTRY_ptr(reginfo) = hTN_MAP_Get (_regs_map, opnd_tn);
    if (!REG_ENTRY_reg_assigned(reginfo)) {
      // check for an earlier occurence of the opnd_tn.
      BOOL reg_handled = FALSE;
      for (INT j = 0; j < i; j++) {
	if (OP_opnd(op,j) == opnd_tn)
	  reg_handled = TRUE;
      }
      if (!reg_handled) {
        cl = TN_register_class(opnd_tn);
        local_regs_avail[cl]--;
        INT reg_pressure = (2 - local_regs_avail[cl]);
        if (reg_pressure > 0) {
          cost += reg_pressure;
        }
      }
    }
  }   
  OPSCH *opsch = OP_opsch(op, _hb_map);
  OPSCH_regcost(opsch) = cost;
}

// ======================================================================
// Update the number of registers available for allocation after <op> is
// scheduled.
// ======================================================================
void
HB_Schedule::Update_Regs_For_OP (OP *op)
{
  REG_ENTRY reginfo;

  INT i;
  for (i = 0; i < OP_results(op); i++) {
    TN *result_tn = OP_result(op, i);
    REG_ENTRY_ptr(reginfo) = hTN_MAP_Get (_regs_map, result_tn);
    REG_ENTRY_def_count(reginfo)--;
    if (REG_ENTRY_def_count(reginfo) == 0 &&
	REG_ENTRY_reg_assigned(reginfo)) 
    {
      ISA_REGISTER_CLASS cl = TN_register_class(result_tn);
      _Cur_Regs_Avail[cl]++;
      REG_ENTRY_reg_assigned(reginfo) = FALSE;
    }
    hTN_MAP_Set (_regs_map, result_tn, REG_ENTRY_ptr(reginfo));
  }
  for (i = 0; i < OP_opnds(op); i++) {
    TN *opnd_tn = OP_opnd(op,i);
    if (TN_is_constant(opnd_tn)) continue;
    REG_ENTRY_ptr(reginfo) = hTN_MAP_Get (_regs_map, opnd_tn);
    if (!REG_ENTRY_reg_assigned(reginfo)) {
      ISA_REGISTER_CLASS cl = TN_register_class(opnd_tn);
      _Cur_Regs_Avail[cl]--;
#ifdef TARG_ST
      if (_Cur_Regs_Avail[cl] < _Min_Cur_Regs_Avail[cl]) {
	_Min_Cur_Regs_Avail[cl] = _Cur_Regs_Avail[cl];
      }
#endif
      REG_ENTRY_reg_assigned(reginfo) = TRUE;
      hTN_MAP_Set (_regs_map, opnd_tn, REG_ENTRY_ptr(reginfo));
    }
  }
}

// ======================================================================
// Return TRUE if <op1> and <op2> are addiu and load/store instructions
// such that the addiu and the load/store can be interchanged.
// ======================================================================
// FdF 20/10/2003 : Added support for sequences (cmpeq,cmpne; add)
BOOL
Is_Ldst_Addiu_Pair (OPSCH *opsch1, OPSCH *opsch2, OP *op1,OP *op2)
{
  OP *addiu_op;
  OP *ldst_op;
  INT64 multiplier;
  
  if (((OPSCH_flags(opsch1) | OPSCH_flags(opsch2)) & OPSCH_ADDIU_LDST_PAIR) !=
      OPSCH_ADDIU_LDST_PAIR) 
  {
    return FALSE;
  }

#ifdef TARG_ST
  // FdF 15/12/2003: Because in case of LDW .. 0(Rx); ADD Rx=Rx+..;
  // there is a MISC dependence between the two uses of Rx in LDW and
  // ADD. We are only interested in the REGANTI arc between the use in
  // LDW and the def in ADD.
  BOOL isRegInAnti = FALSE;
  ARC_LIST *arcs;
  for (arcs = OP_succs(op1); arcs != NULL; arcs = ARC_LIST_rest(arcs)) {
    ARC *arc = ARC_LIST_first(arcs);
    if (ARC_succ(arc) == op2 &&
	(ARC_kind(arc) == CG_DEP_REGIN || ARC_kind(arc) == CG_DEP_REGANTI))
      isRegInAnti = TRUE;
  }
  if (!isRegInAnti)
    return FALSE;
#endif

  if (OPSCH_addiu(opsch1)) {
    addiu_op = op1;
    ldst_op = op2;
    multiplier = 1;
  }
  else {
    addiu_op = op2;
    ldst_op = op1;
    multiplier = -1;
  }

#ifdef TARG_ST
  Is_True(OP_iadd(addiu_op), ("OPSCH_addiu but not OP_iadd"));
  INT add_opnd1_idx = OP_find_opnd_use(addiu_op, OU_opnd1);
  INT add_opnd2_idx = OP_find_opnd_use(addiu_op, OU_opnd2);
  Is_True(TN_has_value(OP_opnd(addiu_op, add_opnd2_idx)), ("OPSCH_addiu but second operand is not literal"));
  INT64 addiu_const = TN_value (OP_opnd(addiu_op, add_opnd2_idx));
  
  // FdF 20/10/2003 : Support for sequences (cmpeq,cmpne; add)
  if (OP_Is_Cmp_Eq_Ne(ldst_op)) {
    INT cmp_opnd1_idx = OP_find_opnd_use(ldst_op, OU_opnd1);
    INT cmp_opnd2_idx = OP_find_opnd_use(ldst_op, OU_opnd2);
    Is_True(TN_has_value(OP_opnd(ldst_op, cmp_opnd2_idx)), ("OP_Is_Cmp_Eq_Ne returned true but opnd2 is not literal"));
    if (OP_result(addiu_op,0) != OP_opnd(ldst_op, cmp_opnd1_idx))
      return FALSE;
    INT64 ldst_const = TN_value (OP_opnd(ldst_op, cmp_opnd2_idx));
    return OP_code(ldst_op) == TOP_opnd_immediate_variant(OP_code(ldst_op), cmp_opnd2_idx, ldst_const - addiu_const*multiplier);
  }
#endif

  Is_True(OP_memory(ldst_op), ("OPSCH_ldst but non memory op"));
  INT base_opndnum = Memory_OP_Base_Opndnum(ldst_op);
  INT offset_opndnum = Memory_OP_Offset_Opndnum(ldst_op);
  // Check that the result of the addiu is the same as the base of the ldst.
  // Also check that if the memory OP is a store, the source is not the same
  // as the result of the addiu. Assume that add result is result index 0.
  if (OP_result(addiu_op,0 /*???*/) != OP_opnd(ldst_op, base_opndnum) ||
      (OP_store(ldst_op) &&
#ifdef TARG_ST
       OP_result(addiu_op,0 /*???*/) == OP_opnd(ldst_op, TOP_Find_Operand_Use(OP_code(ldst_op), OU_storeval))
#else
       OP_result(addiu_op,0 /*???*/) == OP_opnd(ldst_op,0)
#endif
       ))
  {
    return FALSE;
  }

  INT64 ldst_const;
#ifdef TARG_ST
  // FdF 20060518: Support for automod addressing mode
  if (OP_automod(ldst_op))
    return FALSE;

  // FdF 15/12/2003: Added support for symbolic offsets in the stack.
  if (TN_is_symbol(OP_opnd(ldst_op, offset_opndnum))) {
    TN *old_ofst_tn = OP_opnd(ldst_op, offset_opndnum);
    ST *st = TN_var(old_ofst_tn);
    ST *base_st;
    INT64 base_ofst;

    base_st = Base_Symbol (st);
    if (base_st != SP_Sym) return FALSE;
    FmtAssert (Base_Offset_Is_Known (st), ("unknown offset in Is_Ldst_Addiu_Pair"));
    base_ofst = Base_Offset (st);
    ldst_const = CGTARG_TN_Value (old_ofst_tn, base_ofst);
    if (ldst_const + addiu_const*multiplier < 0)
      return FALSE;
  } else if (OP_opnd(ldst_op, base_opndnum) == SP_TN) {
    // [CG]: We may have SP based accesses with no symbol associated
    // in the case of accesses to the parameter passing area.
    // Thus we must also ensure that any SP_TN based access will
    // not be negative
    ldst_const = TN_value (OP_opnd(ldst_op, offset_opndnum));
    if (ldst_const + addiu_const*multiplier < 0) return FALSE;
  }
  else
#endif
    ldst_const = TN_value (OP_opnd(ldst_op, offset_opndnum));

#ifdef TARG_ST
  // [CG] Don't allow opcode size change
  if (OP_code(ldst_op) == TOP_opnd_immediate_variant(OP_code(ldst_op), offset_opndnum, ldst_const + addiu_const*multiplier)) {
    return TRUE;
  } else {
    return FALSE;
  }
#else
  return TOP_Can_Have_Immediate (ldst_const + addiu_const*multiplier, OP_code(ldst_op));
#endif
}

// ======================================================================
// Change the offset field in load/store OP after it has been moved across
// an addiu OP that defines the base register for the <ldst_op>. The 
// <multiplier> can be either +1 or -1 to indicate direction of movement.
// ======================================================================
// FdF 20/10/2003 : Added support for sequences (cmpeq,cmpne; add)
void
Fixup_Ldst_Offset (OP *ldst_op, INT64 addiu_const, INT64 multiplier, 
		   HBS_TYPE type)
{
  TN *old_ofst_tn, *ofst_tn;
  INT index;

#ifdef TARG_ST
  if (OP_Is_Cmp_Eq_Ne(ldst_op)) {
    index = OP_find_opnd_use(ldst_op, OU_opnd2);
    multiplier = -multiplier;
  } else {
    index = Memory_OP_Offset_Opndnum (ldst_op);
  }
#else
  index = Memory_OP_Offset_Opndnum (ldst_op);
#endif

  old_ofst_tn = OP_opnd(ldst_op, index);

  if (Trace_HB) {
    #pragma mips_frequency_hint NEVER
    fprintf (TFile, "old: %lld, new: %lld\n", TN_value(old_ofst_tn), 
		      TN_value(old_ofst_tn) + addiu_const * multiplier);
    fprintf (TFile, "offset changed:");
    Print_OP_No_SrcLine (ldst_op);
  }

#ifdef TARG_ST
// FdF 15/12/2003: Added support for symbolic offsets in the stack.
  if (TN_is_symbol(old_ofst_tn))
    ofst_tn = Gen_Symbol_TN(TN_var(old_ofst_tn),
                            TN_offset(old_ofst_tn) + addiu_const * multiplier,
                            TN_relocs(old_ofst_tn));
  else
#endif
  ofst_tn = Gen_Literal_TN (TN_value(old_ofst_tn) + addiu_const * multiplier,
			    TN_size(old_ofst_tn));
  Set_OP_opnd (ldst_op, index, ofst_tn);

}

// ======================================================================
// Traverse through the list of scheduled instructions and look for load
// or store OPs that have been moved across corresponding addiu OPs. For
// all such load/store OPs, adjust their offset field.
// ======================================================================
#ifdef TARG_ST
  // Handle fwd/bwd cases
void
HB_Schedule::Adjust_Ldst_Offsets (BOOL is_fwd)
#else
void
HB_Schedule::Adjust_Ldst_Offsets (void)
#endif
{
#ifdef TARG_ST
  // Handle fwd/bwd cases
  for (INT i = (is_fwd) ? 0 : VECTOR_count(_sched_vector) - 1; 
       (is_fwd) ? i < VECTOR_count(_sched_vector) : i >= 0; 
       (is_fwd) ? i++ : i--) 
#else
    for (INT i = VECTOR_count(_sched_vector)-1; i >= 0; i--)
#endif
      {
    OP *op = OP_VECTOR_element(_sched_vector, i);
    OPSCH *opsch = OP_opsch(op, _hb_map);
    Set_OPSCH_visited (opsch);
    if (!OPSCH_addiu (opsch)) continue;
    INT64 addiu_const = TN_value (OP_opnd(op,OP_find_opnd_use(op, OU_opnd2)));
    ARC_LIST *arcs;
    for (arcs = OP_succs(op); arcs != NULL; arcs = ARC_LIST_rest(arcs)) {
      ARC *arc = ARC_LIST_first(arcs);
      OP *succ_op = ARC_succ(arc);
      OPSCH *succ_opsch = OP_opsch (succ_op, _hb_map);
      if (OPSCH_ldst (succ_opsch) && OPSCH_visited (succ_opsch)) {
#ifdef TARG_ST
	// FdF 15/12/2003: Do not call twice when there are a REGANTI
	// and a MISC dependence for example (see Is_Ldst_addiu_Pair).
	if (ARC_kind(arc) == CG_DEP_REGIN || ARC_kind(arc) == CG_DEP_REGANTI)
#endif
	  Fixup_Ldst_Offset (succ_op, addiu_const, +1, type());
      }
    }
    for (arcs = OP_preds(op); arcs != NULL; arcs = ARC_LIST_rest(arcs)) {
      ARC *arc = ARC_LIST_first(arcs);
      OP *pred_op = ARC_pred(arc);
      OPSCH *pred_opsch = OP_opsch (pred_op, _hb_map);
      if (OPSCH_ldst (pred_opsch) && !OPSCH_visited (pred_opsch)) {
#ifdef TARG_ST
	// FdF 15/12/2003: Do not call more than once
	if (ARC_kind(arc) == CG_DEP_REGIN || ARC_kind(arc) == CG_DEP_REGANTI)
#endif
	Fixup_Ldst_Offset (pred_op, addiu_const, -1, type());
      }
    }
  }
}

// ======================================================================
// Set_Resource_Usage
//
// Given an 'op' and a start 'cycle', reserve all the resources needed
// to schedule the op at that cycle. It is assumed that we have already
// verified that the resources are available.
// ======================================================================
void
HB_Schedule::Set_Resource_Usage (OP *op)
{
  INT cycle = OPSCH_scycle (OP_opsch(op, _hb_map));

  TI_RES_RES_Reserve_Resources(_rr_tab, OP_code(op), cycle);
  Clock = cycle;
}

// ======================================================================
// Find_Schedule_Cycle
//
// Find the cycle in which 'op' can be scheduled. Also, update the scycle
// for the OP so that future searches are faster.
// ======================================================================
INT
HB_Schedule::Find_Schedule_Cycle (OP *op, BOOL is_fwd)
{
  OPSCH *opsch = OP_opsch(op, _hb_map);
  INT cycle = OPSCH_scycle(opsch);
  INT cyc;

  if (is_fwd) {
    // keep looking forward till we can schedule the op.
    for (cyc = cycle; cyc <= MAX_Clock; cyc++) {
      if (Check_Resource_Usage (op, cyc)) break;
    }
    FmtAssert (cyc <= MAX_Clock, ("HB_SCHED: no valid cycle for scheduling"));
  } else {
    // Keep looking back till we can schedule the op.
    for (cyc = cycle; cyc >= 0; cyc--) {
      if (Check_Resource_Usage (op, cyc)) break;
    }
    FmtAssert (cyc >= 0, ("HB_SCHED: no valid cycle for scheduling"));
  }

  // update the scycle for the OP.
  OPSCH_scycle(opsch) = cyc;
  return cyc;
}

static INT cur_dfsnum;

// ======================================================================
// Initialize the OPSCH data structure for each OP in <bb>. Identify
// OPs with the OPSCH_addiu, OPSCH_ldst, OPSCH_def_xfer_opnd attributes.
// ======================================================================
// FdF 20/10/2003 : Added support for sequences (cmpeq,cmpne; add)
static void
Init_OPSCH_For_BB (BB *bb, BB_MAP value_map, MEM_POOL *pool)
{
  OP *op;
  ARC_LIST *arcs;
  ARC *arc;
  OPSCH *opsch;

  FOR_ALL_BB_OPs_FWD (bb, op) {
    opsch = TYPE_MEM_POOL_ALLOC (OPSCH, pool);
    BZERO (opsch, sizeof (OPSCH));
    OPSCH_lstart(opsch) = 0x7fff;
    BB_OP_MAP bb_map = (BB_OP_MAP) BB_MAP_Get(value_map, bb);
    BB_OP_MAP_Set (bb_map, op, opsch);
  }

  FOR_ALL_BB_OPs_FWD (bb, op) {
    opsch = OP_opsch(op, value_map);
    // Identify LDST/ADDIU instructions with non-relocatable offsets.
#ifdef TARG_ST
    // FdF 15/12/2003: Support for symbolic offset in the stack, in
    // postpass scheduling only.
    if (OP_Is_Addr_Incr(op) && 
	(!TN_is_sp_reg(OP_result(op,0 /*???*/)) || !Before_LRA)) {
#else
    if (CGTARG_Is_OP_Addr_Incr(op) && 
	!TN_is_sp_reg(OP_result(op,0 /*???*/))) {
#endif
	
      BOOL addiu_ok = TRUE;
      for (arcs = OP_succs(op); arcs != NULL; arcs = ARC_LIST_rest(arcs)) {
	arc = ARC_LIST_first(arcs);
	if (ARC_kind(arc) == CG_DEP_REGOUT) {
	  addiu_ok = FALSE;
	  break;
	}
      }
      for (arcs = OP_preds(op); arcs != NULL; arcs = ARC_LIST_rest(arcs)) {
	arc = ARC_LIST_first(arcs);
	if (ARC_kind(arc) == CG_DEP_REGIN) {
	  addiu_ok = FALSE;
	  break;
	}
      }
      if (addiu_ok) Set_OPSCH_addiu (opsch);
    }
    else if (OP_memory(op)) {
      // check if the memory OP has an offset field (i.e. it is not an 
      // indexed load/store/prefx.
      INT offset_opndnum = Memory_OP_Offset_Opndnum (op);
      if (TN_has_value(OP_opnd(op,offset_opndnum))) {
	Set_OPSCH_ldst (opsch);
      }
#ifdef TARG_ST
      // FdF 15/12/2003: Added support for symbolic offset in the stack
      else if (TN_is_symbol(OP_opnd(op,offset_opndnum))) {
	Set_OPSCH_ldst (opsch);
      }
#endif
    }
#ifdef TARG_ST
    // Check if this is a cmpeq or cmpne with a constant value.
    else if (OP_Is_Cmp_Eq_Ne(op)) {
      Set_OPSCH_ldst (opsch);
    }
#endif

#ifdef TARG_MIPS
    if (Is_Target_T5() && OP_xfer(op) && Get_Trace (TP_SCHED, 0x1000)) {
      for (arcs = OP_preds(op); arcs != NULL; arcs = ARC_LIST_rest(arcs)) {
        arc = ARC_LIST_first(arcs);
	if (ARC_kind(arc) == CG_DEP_REGIN) {
	  OP *pred_op = ARC_pred(arc);
	  OPSCH *pred_opsch = OP_opsch(pred_op, value_map);
	  Set_OPSCH_def_xfer_opnd(pred_opsch);
	}
      }
    }
#endif
  }
}

// ======================================================================
// return TRUE if opsch1 has a larger 'estart' value than opsch2.
// ======================================================================
#ifdef TARG_ST
static INT
sort_by_estart (const void *opsch1, const void *opsch2)
{
  return (OPSCH_estart((OPSCH*) opsch1) < OPSCH_estart((OPSCH*) opsch2))
    ? -1
    : (OPSCH_estart((OPSCH*) opsch1) == OPSCH_estart((OPSCH*) opsch2))
    ? 0
    : 1;
}
#else
static BOOL
sort_by_estart (const void *opsch1, const void *opsch2)
{
  return (OPSCH_estart((OPSCH*) opsch1) > OPSCH_estart((OPSCH*) opsch2));
}
#endif
 
// ======================================================================
// return TRUE if opsch1 has a smaller 'slack' value than opsch2.
// ======================================================================
static BOOL
sort_by_slack (const void *opsch1, const void *opsch2)
{
  INT slack1 = OPSCH_lstart((OPSCH*) opsch1) - OPSCH_estart((OPSCH*) opsch1);
  INT slack2 = OPSCH_lstart((OPSCH*) opsch2) - OPSCH_estart((OPSCH*) opsch2);
 
  return ((OPSCH*) slack1 < (OPSCH*) slack2);
}
 
// ======================================================================
// Recursive depth first search of the dep-graph starting at <op>. The 
// OPSCH_dfsnum field is marked as each OP is visited.
// ======================================================================
static void
DFS_Search (OP *op, BB_MAP value_map, BOOL is_fwd)
{
  OPSCH *opsch = OP_opsch (op, value_map);
  ARC_LIST *arcs;

  if (OPSCH_visited(opsch)) return;

  OPSCH_dfsnum(opsch) = cur_dfsnum;
  Set_OPSCH_visited(opsch);
  cur_dfsnum++;

  if (is_fwd) {
    // visit all the successors.
    for (arcs = OP_succs(op); arcs != NULL; arcs = ARC_LIST_rest(arcs)) {
      ARC *arc = ARC_LIST_first(arcs);
      OP *succ_op = ARC_succ(arc);
      OPSCH *succ_opsch = OP_opsch (succ_op, value_map);
      if (!OPSCH_visited(succ_opsch)) DFS_Search (succ_op, value_map, TRUE);
    }
  } else {
    // visit all the predecessors.
    for (arcs = OP_preds(op); arcs != NULL; arcs = ARC_LIST_rest(arcs)) {
      ARC *arc = ARC_LIST_first(arcs);
      OP *pred_op = ARC_pred(arc);
      OPSCH *pred_opsch = OP_opsch (pred_op, value_map);
      if (!Is_Ldst_Addiu_Pair (pred_opsch, opsch, pred_op, op)) {
	if (!OPSCH_visited(pred_opsch)) DFS_Search (pred_op, value_map, FALSE);
      }
    }
  }
}

// ======================================================================
// Compute depth first ordering.
// ======================================================================
static void
Compute_DFO (HB_Schedule *sched, BB_MAP value_map, BOOL is_fwd)
{
  INT i;

  cur_dfsnum = 1;
  for (i = 0; i < VECTOR_count(sched->ready_vector()); i++) {
    DFS_Search (OP_VECTOR_element(sched->ready_vector(), i), value_map,is_fwd);
  }
}

// ======================================================================
// Traverse the OPs in the basic block backwards and build a list of OPs
// that are ready to schedule. 
// Sort the ready vector in decreasing order or 'estart'.
// ======================================================================
void
Priority_Selector::Add_Element_Sorted (VECTOR vector, void *element, VECTOR_ELEMENT_COMPARE comp_func)
{
  INT i;
  INT count = VECTOR_count(vector);
  FmtAssert (count < VECTOR_size(vector), ("VECTOR overflow"));
  for (i = count; i > 0; i--) {
    void *cur_element = VECTOR_element(vector, i - 1);
    void *cur_opsch = OP_opsch((OP*) cur_element, _cur_sched->hb_map());
    void *opsch = OP_opsch((OP*) element, _cur_sched->hb_map());
#ifdef TARG_ST
    // FdF 20051212: Keep the initial order of memory operations as
    // much as possible for the pre-pass scheduling.
    int cmp = comp_func(cur_opsch, opsch);
    if (cmp > 0) break;
    if (cmp == 0 && (OP_memory((OP *)cur_element) && OP_memory((OP *)element)) &&
	OP_map_idx((OP *)cur_element) > OP_map_idx((OP *)element)) break;
#else
    if (comp_func(cur_opsch, opsch)) break;
#endif
    VECTOR_element(vector, i) = cur_element;
  }
  VECTOR_element(vector, i) = element;
  count++;
  VECTOR_count(vector) = count;
}

// ======================================================================
// Traverse the OPs in the basic block backwards and build a list of OPs
// that are ready to schedule. 
// Sort the ready vector in decreasing order or 'estart'.
// ======================================================================
void
Priority_Selector::Build_Ready_Vector (BB* bb, BOOL is_fwd)
{
  OP *op;

  if (is_fwd) {
    FOR_ALL_BB_OPs_FWD (bb, op) {
      OPSCH *opsch = OP_opsch(op, _cur_sched->hb_map());
      // Add it to the ready vector if there are no predecessors.
      if (OPSCH_num_preds(opsch) == 0 && (!OP_xfer(op))) {
	Add_Element_Sorted (_cur_sched->ready_vector(), op, sort_by_slack);
      }
    }
  } else {
    FOR_ALL_BB_OPs_REV (bb, op) {
      OPSCH *opsch = OP_opsch(op, _cur_sched->hb_map());
      // Add it to the ready vector if there are no successors.
      if (OPSCH_num_succs(opsch) == 0) {
	Add_Element_Sorted (_cur_sched->ready_vector(), op, sort_by_estart);
      }
    }
  }
}

// ======================================================================
// Traverse the OPs in the basic block backwards and build a list of OPs
// that are ready to schedule. 
// Sort the ready vector in decreasing order or 'estart'.
// ======================================================================
void
Priority_Selector::Build_Ready_Vector (std::list<BB*> bblist, BOOL is_fwd)
{

  std::list<BB*>::iterator bb_iter;
  FOR_ALL_BB_STLLIST_ITEMS_FWD (bblist, bb_iter) {
    Build_Ready_Vector (*bb_iter, is_fwd);
  }
}

// ======================================================================
// Return the maximum number of cycles in which a resource is used for
// the OP. This is used to estimate the size of the resource table
// required. It could also be used as a coarse estimate of the latency
// of an OP that has no successors.
// ======================================================================
inline INT
Resource_Cycles_For_OP (OP *op)
{
  return TI_RES_Cycle_Count(OP_code(op));
}

// ======================================================================
// Calculate_Adjust_Latency
// Placeholder to make all latency adjustments (both static/OOO effects).
// ======================================================================
static inline INT
Calculate_Adjust_Latency (ARC *arc)
{
  INT adjust_latency;
  // OOO adjustments necessary (or not)
#ifdef TARG_IA64
  // Arthur: this is target dependent
  BOOL ooo_adjust = (PROC_is_out_of_order() && CG_DEP_Adjust_OOO_Latency);
#else
  BOOL ooo_adjust = (PROC_Is_Out_Of_Order() && CG_DEP_Adjust_OOO_Latency);
#endif

  // For OOO machine (eg. T5), non-definite memory dependences can be 
  // relaxed to edges with zero latency. The belief is that this can 
  // help avoid creating false dependences with biased critical info.

  adjust_latency = (ooo_adjust && ARC_is_mem(arc) &&
		    !ARC_is_definite(arc)) ? 0 : ARC_latency(arc);

  // Similary, anti-dependences (reg) can be relaxed as well. Since, OOO
  // machines do dynamic renaming, having static estimates due to such
  // dependences can be avoided.
  adjust_latency = (ooo_adjust && ARC_is_reg(arc) &&
		    (ARC_is_anti(arc) || ARC_is_output(arc))) ?
		    0 : adjust_latency;

  return adjust_latency;
}

// ======================================================================
// Do a forward pass and compute the OPSCH data structure for the bb.
// ======================================================================
static void
Compute_Fwd_OPSCH (BB *bb, BB_MAP value_map, INT *max_lstart)
{
  OP *op;

  // Initialize the OPSCH_estart and OPSCH_num_succs fields of all OPs.
  // Also compute the max_lstart.
  FOR_ALL_BB_OPs_FWD (bb, op) {
    OPSCH *opsch = OP_opsch(op, value_map);
    INT op_estart = OPSCH_estart(opsch);
    ARC_LIST *arcs;
    for (arcs = OP_succs(op); arcs != NULL; arcs = ARC_LIST_rest(arcs)) {
      ARC *arc = ARC_LIST_first(arcs);
      OP *succ_op = ARC_succ(arc);
      OPSCH *succ_opsch = OP_opsch(succ_op, value_map);
      if (!Is_Ldst_Addiu_Pair (opsch, succ_opsch, op, succ_op)) {
        INT cur_estart = Calculate_Adjust_Latency(arc) + op_estart;
        if (OPSCH_estart(succ_opsch) < cur_estart) {
          OPSCH_estart(succ_opsch) = cur_estart;
        }
        OPSCH_num_succs(opsch)++;
      }
    }
    *max_lstart = MAX (*max_lstart, op_estart);
  }
}

// ======================================================================
// Do a backward pass and compute the OPSCH data structure for the bb.
// ======================================================================
static void
Compute_Bkwd_OPSCH (BB *bb, BB_MAP value_map, INT max_lstart)
{
  OP *op;

  // Initialize the OPSCH_scycle, OPSCH_lstart and OPSCH_num_preds fields of
  // all OPs.
  FOR_ALL_BB_OPs_REV (bb, op) {
    OPSCH *opsch = OP_opsch(op, value_map);
    ARC_LIST *arcs;
    OPSCH_scycle(opsch) = Clock;
    if (OPSCH_lstart(opsch) > max_lstart) OPSCH_lstart(opsch) = max_lstart;
    for (arcs = OP_preds(op); arcs != NULL; arcs = ARC_LIST_rest(arcs)) {
      ARC *arc = ARC_LIST_first(arcs);
      OP *pred_op = ARC_pred(arc);
      OPSCH *pred_opsch = OP_opsch(pred_op, value_map);
      if (!Is_Ldst_Addiu_Pair (pred_opsch, opsch, pred_op, op)) {
        INT cur_lstart = OPSCH_lstart(opsch) - Calculate_Adjust_Latency(arc);
        if (OPSCH_lstart(pred_opsch) > cur_lstart) {
          OPSCH_lstart(pred_opsch) = cur_lstart;
        }
        OPSCH_num_preds(opsch)++;
      }
    }
  }
}

// ======================================================================
// Given a <bb>, build the OPSCH data structure for it.
// ======================================================================
void
Compute_OPSCH (BB *bb, BB_MAP value_map, MEM_POOL *pool)
{
  INT max_lstart = 0;

  Init_OPSCH_For_BB (bb, value_map, pool);

  Compute_Fwd_OPSCH (bb, value_map, &max_lstart);

  Compute_Bkwd_OPSCH (bb, value_map, max_lstart);
}

// ======================================================================
// Given a list of single-entry multiple exit blocks, build the OPSCH data 
// structure for it.
// ======================================================================
void
Compute_OPSCHs (std::list<BB*> bblist, BB_MAP value_map, MEM_POOL *pool)
{
  std::list<BB*>::iterator bb_iter;

  // Initialize all data structures.
  FOR_ALL_BB_STLLIST_ITEMS_FWD(bblist, bb_iter) {
    Init_OPSCH_For_BB (*bb_iter, value_map, pool);
  }

  // Do a forward pass first.
  INT max_lstart = 0;
  FOR_ALL_BB_STLLIST_ITEMS_FWD(bblist, bb_iter) {
    Compute_Fwd_OPSCH (*bb_iter, value_map, &max_lstart);
  }

  // Do a backward pass.
  std::list<BB*>::reverse_iterator bb_riter;
  FOR_ALL_BB_STLLIST_ITEMS_BKWD(bblist, bb_riter) {
    Compute_Bkwd_OPSCH (*bb_riter, value_map, max_lstart);
  }
}

// ======================================================================
// After the <bb> is scheduled, build the BBSCH data structure for GCM phase.
// ======================================================================
void
HB_Schedule::Compute_BBSCH (BB *bb, BBSCH *bbsch)
{
  INT critical_length = 0;

  BBSCH_schedule_length (bbsch) = OP_scycle(BB_last_op(bb)) + 1;
  // computes the longest (or critical) latency for this bb
  // in inverse order
  for (INT i = 0; i < VECTOR_count(_sched_vector); i++) {
    OP *op = OP_VECTOR_element(_sched_vector, i);
    OPSCH *opsch = OP_opsch(op, _hb_map);
    if (!critical_length &&
	(OPSCH_lstart(opsch) - OPSCH_estart(opsch)) == 0) { 
      critical_length = OPSCH_scycle(opsch) - Clock;
    }
#ifdef TARG_ST
    if (OP_Is_Barrier(op)) {
#else
    if (CGTARG_Is_OP_Barrier(op)) {
#endif
      Set_BB_MEM_BARRIER(bbsch);
      if (critical_length) break;
    }
  }

  // block parallelism is computed as the ratio of the number of
  // ops present in this block divided by the longest critical latency
  // the idea is that this would give a rough feel of the amount of 
  // parallelism present in this bb when compared to CGTARG_Peak_Rate
  BBSCH_block_parallelism (bbsch) =  (VECTOR_count(_sched_vector) != 0) ?
	(int)ceil(VECTOR_count(_sched_vector) / (critical_length + 1.)) : -1;

 if (Cur_Gcm_Type & GCM_MINIMIZE_REGS) {

   GTN_SET *local_set =  GTN_SET_Intersection(BB_live_in(bb), BB_live_out(bb),
					      &_hb_pool);
   local_set = GTN_SET_Union(local_set, BB_live_in(bb), &_hb_pool);
   local_set = GTN_SET_Intersection(local_set, BB_live_def(bb), 
				    &_hb_pool);

   BBSCH_global_regcost (bbsch) = GTN_SET_Alloc_Size(local_set);
 }

}

// ======================================================================
// Add the selected <op> to the list of scheduled instructions. Update
// various data structures that are affected by this addition.
// ======================================================================
void
HB_Schedule::Add_OP_To_Sched_Vector (OP *op, BOOL is_fwd)
{
  ARC_LIST *arcs;
  OPSCH *opsch = OP_opsch (op, _hb_map);

  Set_OPSCH_scheduled (opsch);

  // Adjust the resource table to account for this OP. Change 'Clock' 
  // to be the scycle of this OP.
  if (!OP_dummy(op)) Set_Resource_Usage (op);

  if (HBS_Minimize_Regs() && !OP_dummy(op)) {
    Update_Regs_For_OP (op);
  }

  // Remove <op> from Ready_Vector.
  VECTOR_Delete_Element (_ready_vector, op);

  INT count = VECTOR_count(_ready_vector);

  // revise the scycle for all OPs in the Ready_Vector.
  for (INT i = 0; i < count; i++) {
    OP *ready_op = OP_VECTOR_element(_ready_vector,i);
    OPSCH *ready_opsch = OP_opsch (ready_op, _hb_map);
    if (is_fwd) {
      OPSCH_scycle(ready_opsch) = MAX (Clock, OPSCH_scycle(ready_opsch));
    } else {
      OPSCH_scycle(ready_opsch) = MIN (Clock, OPSCH_scycle(ready_opsch));
    }
  }

  if (is_fwd) {
    // Add any OPs that are now ready to be scheduled to the Ready_Vector.
    for (arcs = OP_succs(op); arcs != NULL; arcs = ARC_LIST_rest(arcs)) {
      ARC *arc = ARC_LIST_first(arcs);
      OP *succ_op = ARC_succ(arc);
      OPSCH *succ_opsch = OP_opsch (succ_op, _hb_map);

      // Adjust the scycle, num_preds if <succ_op> not prior scheduled.

      if (!OPSCH_scheduled(succ_opsch)) {
	INT scycle = Clock + ARC_latency(arc);
	// update the OPSCH_scycle field for the predecessor OP.
	OPSCH_scycle(succ_opsch) = MAX (scycle, OPSCH_scycle(succ_opsch));
	if (!Is_Ldst_Addiu_Pair (opsch, succ_opsch, op, succ_op)) {
	  FmtAssert (OPSCH_num_preds(succ_opsch) != 0, 
		     ("HBS: invalid count of succs"));
	  
	  OPSCH_num_preds(succ_opsch)--;
	  if (OPSCH_num_preds(succ_opsch) == 0
#ifdef TARG_ST
	      // [vcdv] do not add xfer op top ready list, they are
	      // treated in Schedule_Block()            
	      && !OP_xfer(succ_op)
#endif
	      
	      ) {
	    VECTOR_Add_Element (_ready_vector, succ_op);
	  }
	}
      }
    }
  } else {
    // Add any OPs that are now ready to be scheduled to the Ready_Vector.
    for (arcs = OP_preds(op); arcs != NULL; arcs = ARC_LIST_rest(arcs)) {
      ARC *arc = ARC_LIST_first(arcs);
      OP *pred_op = ARC_pred(arc);
      OPSCH *pred_opsch = OP_opsch (pred_op, _hb_map);

      // Adjust the scycle, num_succs if <pred_op> not prior scheduled.

      if (!OPSCH_scheduled(pred_opsch)) {
	INT scycle = Clock - ARC_latency(arc);
	// update the OPSCH_scycle field for the predecessor OP.
	OPSCH_scycle(pred_opsch) = MIN (scycle, OPSCH_scycle(pred_opsch));
	if (!Is_Ldst_Addiu_Pair (pred_opsch, opsch, pred_op, op)) {
	  FmtAssert (OPSCH_num_succs(pred_opsch) != 0, 
		     ("HBS: invalid count of succs"));
	  
	  OPSCH_num_succs(pred_opsch)--;
	  if (OPSCH_num_succs(pred_opsch) == 0) {
	    VECTOR_Add_Element (_ready_vector, pred_op);
	  }
	}
      }
    }

    // If current OP is a load/store, check if it has been scheduled before 
    // an addiu. If yes, we need to account for the latency between the 
    // addiu and the current OP.
    if (OPSCH_ldst(opsch)) {
      for (arcs = OP_succs(op); arcs != NULL; arcs = ARC_LIST_rest(arcs)) {
	ARC *arc = ARC_LIST_first(arcs);
	OP *succ_op = ARC_succ(arc);
	OPSCH *succ_opsch = OP_opsch (succ_op, _hb_map);
	if (!OPSCH_scheduled(succ_opsch)) {
#ifdef TARG_ST
	  // FdF 18/06/2004: OPSCH_ldst is set also on cmpne/cmpeq
	  // operations, for which Memory_OP_Base_Opndnum is not
	  // defined (returns -1).
	  INT opndnum = OP_Is_Cmp_Eq_Ne(op)
	                  ? TOP_Find_Operand_Use(OP_code(op), OU_opnd1)
	                  : Memory_OP_Base_Opndnum (op);
#else
	  INT opndnum = Memory_OP_Base_Opndnum (op);
#endif
	  INT scycle = Clock - CG_DEP_Latency (succ_op, op, CG_DEP_REGIN, opndnum);
	  OPSCH_scycle(succ_opsch) = MIN (scycle, OPSCH_scycle(succ_opsch));
	}
      }
    }
  }

  VECTOR_Add_Element (_sched_vector, op);

  if (Trace_HB) {
    #pragma mips_frequency_hint NEVER
    fprintf (TFile, "Cycle: %d :: ", Clock);
    Print_OP_No_SrcLine (op);
  }
}

// ======================================================================
// Compare two OPs to see which one is better for scheduling.
// ======================================================================
BOOL
Priority_Selector::Is_OP_Better (OP *cur_op, OP *best_op)
{
  OPSCH *cur_opsch = OP_opsch(cur_op, _cur_sched->hb_map());
  OPSCH *best_opsch = OP_opsch(best_op, _cur_sched->hb_map());
  INT cur_scycle = OPSCH_scycle(cur_opsch);
  INT best_scycle = OPSCH_scycle(best_opsch);

  if (_hbs_type & HBS_MINIMIZE_REGS) {
    INT cur_op_better = (cur_scycle - best_scycle);
    if (cur_op_better == 0) {
      cur_op_better = (OPSCH_dfsnum(cur_opsch) < OPSCH_dfsnum(best_opsch));
    }
    cur_op_better += (OPSCH_regcost(best_opsch) - OPSCH_regcost(cur_opsch));
    return (cur_op_better > 0);
  }

  BOOL manual_pref = FALSE;

  // FdF 10/06/2004: I don't see any reason for a special case on
  // manual prefetch for the ST targets
  if (OP_prefetch(cur_op) && BB_reg_alloc(OP_bb(cur_op))) {
    //    WN *pref_wn = Get_WN_From_Memory_OP(cur_op);
    //    if (pref_wn && WN_pf_manual(pref_wn)) manual_pref = TRUE;
    manual_pref = TRUE;
  }

  if (cur_scycle > best_scycle)  {

    // Special case manual prefetch nodes. 
    if (manual_pref) return FALSE;
    else return TRUE;
  }

  if (cur_scycle < best_scycle) return FALSE;

  if (manual_pref) return FALSE;

  // For T5, try to schedule the OPs defining the operands of a terminating
  // branch as far away from the branch as possible.
  if (OPSCH_def_xfer_opnd(cur_opsch) ^ OPSCH_def_xfer_opnd(best_opsch)) {
    return OPSCH_def_xfer_opnd(best_opsch);
  }

  if (_hbs_type & HBS_DEPTH_FIRST) {
    return (OPSCH_dfsnum(cur_opsch) < OPSCH_dfsnum(best_opsch));
  }

  if (_hbs_type & HBS_CRITICAL_PATH) {
    // FdF 20051212: Keep memory operations in lexical order as much
    // as possible
#ifdef TARG_ST
    if (OP_memory(cur_op) && OP_memory(best_op) &&
	(OPSCH_lstart(cur_opsch) == OPSCH_lstart(best_opsch))) {
      // If only one has estart==lstart, return this one, otherwise
      // use map_idx.
      if ((OPSCH_estart(cur_opsch) != OPSCH_estart(best_opsch)) &&
	  ((OPSCH_estart(cur_opsch) == OPSCH_lstart(cur_opsch)) ||
	   (OPSCH_estart(best_opsch) == OPSCH_lstart(best_opsch))))
	return (OPSCH_estart(cur_opsch) == OPSCH_lstart(cur_opsch));
      return (OP_map_idx(cur_op) > OP_map_idx(best_op));
    }
#endif
    INT cur_slack, best_slack;
    cur_slack = OPSCH_lstart(cur_opsch) - OPSCH_estart(cur_opsch);
    best_slack = OPSCH_lstart(best_opsch) - OPSCH_estart(best_opsch);
    if (cur_slack < best_slack) return TRUE;
    if (cur_slack > best_slack) return FALSE;

    if (OPSCH_estart(cur_opsch) > OPSCH_estart(best_opsch)) return TRUE;
    if (OPSCH_estart(cur_opsch) < OPSCH_estart(best_opsch)) return FALSE;

#ifdef TARG_ST
    if (Is_Ldst_Addiu_Pair(cur_opsch, best_opsch, cur_op, best_op)) {
      // FdF 15/12/2003: return the one which is the source of a REGIN
      // dependence or the destination of a REGANTI dependence.
      ARC_LIST *arcs;
      for (arcs = OP_succs(cur_op); arcs != NULL; arcs = ARC_LIST_rest(arcs)) {
	ARC *arc = ARC_LIST_first(arcs);
	if ((ARC_succ(arc) == best_op))
	  return (ARC_kind(arc) == CG_DEP_REGIN);
      }
      for (arcs = OP_preds(cur_op); arcs != NULL; arcs = ARC_LIST_rest(arcs)) {
	ARC *arc = ARC_LIST_first(arcs);
	if ((ARC_pred(arc) == best_op))
	  return (ARC_kind(arc) == CG_DEP_REGANTI);
      }
    }
#endif
  }

  return FALSE;
}

// ======================================================================
// Pick an OP to schedule in the delay slot of the terminating branch.
// ======================================================================
OP*
Priority_Selector::Select_OP_For_Delay_Slot (OP *xfer_op)
{

  // If the <xfer_op> has a successor, then that op has to remain in the
  // delay slot. Make sure we return this OP.
  OPSCH *opsch = OP_opsch(xfer_op, _cur_sched->hb_map());
  if (OPSCH_num_succs(opsch) != 0) {
    ARC_LIST *arcs = OP_succs(xfer_op);
    ARC *first_arc = ARC_LIST_first(arcs);
    if (OPSCH_num_succs(opsch) > 1) {
    	ARC *second_arc = ARC_LIST_first(ARC_LIST_rest(arcs));
        DevAssert (ARC_succ(first_arc) == ARC_succ(second_arc), 
		("More than 1 successor for xfer_op"));
    }
    return ARC_succ(first_arc);
  }

  // If this optimization has been disabled or 
  // If we are scheduling to minimize registers, don't put anything 
  // in the delay slot. Trying to spill something in the delay slot 
  // is no fun.
  if (!Enable_Fill_Delay_Slots || 
      (_hbs_type & HBS_MINIMIZE_REGS)) return NULL;

  OP *best_op = NULL;

  for (INT i = VECTOR_count(_cur_sched->ready_vector())-1; i >= 0; i--) 
  {
    OP *cur_op = OP_VECTOR_element (_cur_sched->ready_vector(), i);

    // Don't schedule any dummy OPs or OPs that expand into 
    // more than 1 instruction in the delay slot.
    if (OP_xfer(cur_op) || OP_Real_Ops(cur_op) != 1) continue;

    // Don't put instructions that have hazards in the delay slot.
    if (OP_has_hazard(cur_op)) continue;

    // R10k chip bug workaround: Avoid placing integer mult/div in delay
    // slots of unconditional branches. (see pv516598) for more details.
    if (OP_uncond(xfer_op) && (OP_imul(cur_op) || OP_idiv(cur_op)))
      continue;

    // Don't put manual prefetches into delay slots as well.
    if (OP_prefetch(cur_op)) {
      WN *pref_wn = Get_WN_From_Memory_OP(cur_op);
      if (pref_wn && WN_pf_manual(pref_wn)) continue;
    }

    // When GCM is calling it, don't try to schedule the <op> that has
    // moved from a different block in the delay slot. frequent observance
    // is that this will unneccessarily restrict further code motion.
    if ((_hbs_type & HBS_FROM_GCM) && OP_moved(cur_op)) continue;

    if (best_op == NULL || Is_OP_Better(cur_op, best_op)) {
      best_op = cur_op;
    }
  }
  return best_op;
}

// ======================================================================
// Put the scheduled list of instructions back into the basic block. 
// This is done by emptying the basic block and inserting the scheduled
// instructions back into the basic block.
// ======================================================================
void
HB_Schedule::Put_Sched_Vector_Into_BB (BB *bb, BBSCH *bbsch, BOOL is_fwd)
{
  INT i;

#ifdef TARG_ST
  INT32 cur_cycle;
  {
    OP *op = BB_last_op(bb);
    OPSCH *opsch = OP_opsch(op, _hb_map);
    cur_cycle = ((is_fwd) ? OPSCH_scycle(opsch) : OPSCH_scycle(opsch) - Clock) + 1;
  }

  if (Trace_HB)
    fprintf(TFile, "BB:%d, hbs_type:0x%04x: cycles=%d fwd: %d, max_sched: %d\n", BB_id(bb), type(), cur_cycle, is_fwd, _max_sched);

  // [SC] When HBS_Minimize_Regs, the schedule is better if it reduces
  // the register pressure for a nominated class.
  // Otherwise, the schedule is better if it reduces the cycle count.
  BOOL sched_is_better;

  if (HBS_Minimize_Regs ()) {
    if (minimize_regs_class != ISA_REGISTER_CLASS_UNDEFINED) {
      // Threshold is set to be the number of excess registers we had
      // before rescheduling.  Usually, this value will be negative,
      // since we are lacking registers.
      // min_cur_regs_avail gives the number of registers available at
      // the point of highest register pressure in the BB, after
      // rescheduling.  If this value is greater than the threshold, then
      // we have reduced register pressure.
      sched_is_better = (_Min_Cur_Regs_Avail[minimize_regs_class]
			 > minimize_regs_threshold);
      if (Trace_HB) {
	fprintf (TFile, "Scheduled to minimize regs: new regs avail = %d, threshold = %d,  max length = %d cycles, new length = %d cycles\n",
		 _Min_Cur_Regs_Avail[minimize_regs_class],
		 minimize_regs_threshold,
		 _max_sched, cur_cycle);
      }
    } else {
      sched_is_better = TRUE;
    }
  } else {
    sched_is_better = (cur_cycle <= _max_sched);
  }
  if (Trace_HB) {
    fprintf (TFile, "sched_is_better = %d\n", (int)sched_is_better);
  }

  // FdF: update OP_scycle only if the new scheduled is better
  if (sched_is_better) {
#endif

  // Set the OP_scycle field for all the OPs. Also, reset the OPSCH_visited
  // flag. It is used in the Adjust_Ldst_Offsets routine.
  for (i = VECTOR_count(_sched_vector) - 1; i >= 0; i--) {
    OP *op = OP_VECTOR_element(_sched_vector, i);
    OPSCH *opsch = OP_opsch(op, _hb_map);
    Reset_OPSCH_visited (opsch);
    OP_scycle(op) = (is_fwd) ? OPSCH_scycle(opsch) : OPSCH_scycle(opsch) - Clock;
  }
#ifdef TARG_ST
  }
  FmtAssert (HBS_Minimize_Regs() || (cur_cycle >= _max_sched) || (cur_cycle == (OP_scycle(BB_last_op(bb)) + 1)),
	     ("HB_SCHED: Inconsistent scycle."));
#else

  INT32 cur_cycle = OP_scycle(BB_last_op(bb)) + 1;
#endif

  // If current cycle estimate is better than <max_sched>, then ONLY dump
  // the Sched_Vector buffer. Otherwise, preserve the previous one.

#ifdef TARG_ST
  if (sched_is_better)
#else
  if (cur_cycle < _max_sched)
#endif
  {
#ifdef TARG_ST
	computed_max_sched=cur_cycle;
    Adjust_Ldst_Offsets (is_fwd);
#else
    Adjust_Ldst_Offsets ();
#endif
    if (bbsch != NULL) {
      Compute_BBSCH (bb, bbsch);
    }

    BB_Remove_All(bb);

    // Dump the <sched_vector> backward (or forward) depending on the
    // type of the schedule.
    for (i = (is_fwd) ? 0 : VECTOR_count(_sched_vector) - 1; 
	 (is_fwd) ? i < VECTOR_count(_sched_vector) : i >= 0; 
	 (is_fwd) ? i++ : i--) {
      BB_Append_Op(bb, OP_VECTOR_element(_sched_vector, i));
    }
  }
}

// ======================================================================
// Put the scheduled list of instructions back into the basic block. 
// This is done by emptying the basic block and inserting the scheduled
// instructions back into the basic block.
// ======================================================================
void
HB_Schedule::Put_Sched_Vector_Into_HB (std::list<BB*>& bblist)
{
  INT i;

  // Set the OP_scycle field for all the OPs. Also, reset the OPSCH_visited
  // flag. It is used in the Adjust_Ldst_Offsets routine.
  for (i = VECTOR_count(_sched_vector)-1; i >= 0; i--) {
    OP *op = OP_VECTOR_element(_sched_vector, i);
    OPSCH *opsch = OP_opsch(op, _hb_map);
    Reset_OPSCH_visited (opsch);
    OP_scycle(op) = OPSCH_scycle(opsch);
  }

  std::list<BB*>::iterator bb_iter;
  FOR_ALL_BB_STLLIST_ITEMS_FWD(bblist, bb_iter) {
    BB_Remove_All(*bb_iter);
  }

  bb_iter = bblist.begin();
  for (i = 0; i < VECTOR_count(_sched_vector); i++) {
    OP *op = OP_VECTOR_element(_sched_vector, i);
    FmtAssert (bb_iter != bblist.end(), ("bb_iter is NULL info"));
    BB_Append_Op(*bb_iter, op);

    // Advance to the next block when noticed an <xfer_op>.
    if (OP_xfer(op)) { bb_iter++; }
  }
}

// ======================================================================
// Allocate a RFlag_Table with <cycles> entries. Initialize the entries.
// ======================================================================
void
HB_Schedule::Init_RFlag_Table (std::list<BB*>& bblist, BOOL is_fwd)
{
  INT rtable_size = 0;
  INT max_resource_cycles = 0;

  _rr_tab = TI_RES_RES_Alloc(FALSE, &_hb_pool);

  std::list<BB*>::iterator bbi;
  FOR_ALL_BB_STLLIST_ITEMS_FWD(bblist, bbi) {
    OP *op;
    FOR_ALL_BB_OPs_FWD (*bbi, op) {
      INT cur_resource_cycles = Resource_Cycles_For_OP (op);
      if (cur_resource_cycles > max_resource_cycles) {
	max_resource_cycles = cur_resource_cycles;
      }
      INT op_latency = cur_resource_cycles;
      ARC_LIST *arcs;
      for (arcs = OP_succs(op); arcs != NULL; arcs = ARC_LIST_rest(arcs)) {
	ARC *arc = ARC_LIST_first(arcs);
	if (ARC_latency(arc) > op_latency) {
	  op_latency = ARC_latency(arc);
	}
      }
      rtable_size += op_latency;
    }
  }

  // start scheduling OPs at this <rtable_size>.
  Clock = (is_fwd) ? 0 : rtable_size;
  MAX_Clock = rtable_size;

  // increase table size by the maximum number of resource cycles needed by
  // any OP.
  rtable_size += max_resource_cycles;

  TI_RES_RES_Set_BB_Cycle_Count(_rr_tab, rtable_size);
}

List_Based_Bkwd::List_Based_Bkwd (BB *bb, HB_Schedule *sched, HBS_TYPE type, 
				  MEM_POOL *pool) : 
  Priority_Selector(bb, sched, type, pool)
{

  // Build Ready list (list of ready candidates).
  Build_Ready_Vector (bb, this->Is_Fwd_Schedule());

  if (sched->HBS_Depth_First()) Compute_DFO (sched, sched->hb_map(),
					     this->Is_Fwd_Schedule());

  if (Trace_HB) Print_BB_For_HB (bb, sched->hb_map());

}

List_Based_Bkwd::List_Based_Bkwd (std::list<BB*> bblist, HB_Schedule *sched, 
				  HBS_TYPE type, MEM_POOL *pool) : 
  Priority_Selector(bblist, sched, type, pool)
{

  // Build Ready list (list of ready candidates).
  Build_Ready_Vector (bblist, this->Is_Fwd_Schedule());

  if (sched->HBS_Depth_First()) Compute_DFO (sched, sched->hb_map(),
					     this->Is_Fwd_Schedule());

  if (Trace_HB) Print_BB_For_HB (bblist, sched->hb_map());

}

// ======================================================================
// Select an OP to schedule next from the Ready_Vector.
// ======================================================================
void*
Priority_Selector::Get_Next_Element(HB_Schedule *Cur_Sched)
{
  _best_op = NULL;

  if (Trace_HB) {
    #pragma mips_frequency_hint NEVER
    fprintf (TFile, "-------------------------------------------\n");
    fprintf (TFile, "Candidates for Scheduling:\n");
    fprintf (TFile, "-------------------------------------------\n");
  }

  for (INT i = VECTOR_count(Cur_Sched->ready_vector())-1; i >= 0; i--) 
  {
    OP *cur_op = OP_VECTOR_element (Cur_Sched->ready_vector(), i);

    // update the scycle for the <cur_op>.
    if (!OP_dummy(cur_op)) Cur_Sched->Find_Schedule_Cycle (cur_op, FALSE);

    if (Cur_Sched->HBS_Minimize_Regs() && !OP_dummy(cur_op)) {
      Cur_Sched->Estimate_Reg_Cost_For_OP (cur_op);
    }

    if (Trace_HB) {
      #pragma mips_frequency_hint NEVER
      Print_OPSCH (cur_op, Cur_Sched->hb_map());
    }

    // Replace the best_op by the cur_op if any of the following is true:
    //   1. best_op is NULL, i.e. cur_op is the first one we have seen.
    //   2. The cur_op is better based on some heuristics.
    if (_best_op == NULL || Is_OP_Better (cur_op, _best_op)) {
      _best_op = cur_op;
    }
  }

  return (void *) _best_op;
}

// ======================================================================
// Compare two OPs to see which one is better for scheduling.
// ======================================================================
BOOL
List_Based_Fwd::Is_OP_Better (OP *cur_op, OP *best_op)
{
  OPSCH *cur_opsch = OP_opsch(cur_op, _cur_sched->hb_map());
  OPSCH *best_opsch = OP_opsch(best_op, _cur_sched->hb_map());
  INT cur_scycle = OPSCH_scycle(cur_opsch);
  INT best_scycle = OPSCH_scycle(best_opsch);

  if (_hbs_type & HBS_MINIMIZE_REGS) {
     INT cur_op_better = (best_scycle - cur_scycle);
     if (cur_op_better == 0) {
       cur_op_better = (OPSCH_dfsnum(cur_opsch) < OPSCH_dfsnum(best_opsch));
     }
     cur_op_better += (OPSCH_regcost(cur_opsch) - OPSCH_regcost(best_opsch));
     return (cur_op_better > 0);
  }

  if (cur_scycle < best_scycle) return TRUE;

  if (cur_scycle > best_scycle) return FALSE;

  // branch-predict instructions ALWAYS need to be scheduled early but not
  // too early. Need to check if the BB_length doesn't exceed the offset
  // limits imposed by the ISA. It's assumed that about 1/3 nops will be
  // added later, so include the expansion factor. 

  if (OP_branch_predict(cur_op) && 
      ((BB_length(OP_bb(cur_op)) * 1.3 * ISA_MAX_INST_BYTES) 
       < DEFAULT_BRP_BRANCH_LIMIT)) 
    return TRUE;

  if (_hbs_type & HBS_DEPTH_FIRST) {
    return (OPSCH_dfsnum(cur_opsch) < OPSCH_dfsnum(best_opsch));
  }

  if (_hbs_type & HBS_CRITICAL_PATH) {
    INT cur_slack, best_slack;
    cur_slack = OPSCH_lstart(cur_opsch) - OPSCH_estart(cur_opsch);
    best_slack = OPSCH_lstart(best_opsch) - OPSCH_estart(best_opsch);

#ifdef TARG_ST
  // [VB] Best operation between two:
  // - If only one is on the critical path (that is lstart-estart=0),
  //   the one on the critical path except if it has no successor
  //   (because it can be scheduled everywhere until the end of the bb)
  //   and if it is not an instruction with a latency (such as a load,
  //   a compare or a setle (for stxp70)) (because if an instruction has a
  //   latency, it is better that it does not appear at the end of a bb
  //   else it can introduce a latency with an instruction at the beginning
  //   of the following bb).
  // - If both are on critical path, the one with the biggest estart if any.
  // - Else the one with the smallest lstart, that is the one which has the
  //   longest path after it so that is the most urgent to schedule,
  //   except if the other one is an instruction with a latency
  //   (such as a load, a compare or a setle (for stxp70)), in order to
  //   avoid such instructions to be scheduled lately in a bb.
  // - Else the one with the smallest difference between its lstart and
  //   its estart (smallest freedom degree).
  // - Else the one with the biggest estart.
  // - Else the first one in the list.
    
    
    if(_hbs_type & HBS_HEURISTIC_CRITICAL_PATH) {
      //[dt] try to improve even more :
      // If one branch is not on the critical path but both have successors, we consider :
      // - Being on critical path
      // - The nuber of successors
      // - The freedom in schedul time (i.e. slack)
      // - The maximum latency between instruction and next one in worst case
      if (((best_slack!=0) ||(cur_slack !=0)) 
          &&  OPSCH_num_succs(cur_opsch) 
          &&  OPSCH_num_succs(best_opsch)
          &&  ((OPSCH_lstart(best_opsch)==OPSCH_lstart(cur_opsch)) || (_hbs_type & HBS_CRITICAL_PATH_PREF_LOAD))) {   	  

    	INT cur_val = 0;
        INT best_val = 0;
  
        if (cur_slack == 0) cur_val = 2;
        else if (cur_slack <= 2) cur_val = 1;
        if (best_slack == 0) best_val = 2;
        else if (best_slack <= 2) best_val = 1;
        
        // Here we know that both operation have at least on successor
        if (OPSCH_num_succs(cur_opsch) <=2 ) cur_val ++ ;
        else cur_val += 2;
        if (OPSCH_num_succs(best_opsch) <=2 ) best_val ++ ;
        else best_val += 2;
        
        if (_hbs_type & HBS_CRITICAL_PATH_PREF_LOAD) {
			/*[dt] Address more specific cases: If we are already after a latest start date, 
			 * increase choose coef and increase the one that have the sooner last start date
			 * Also we add a extra value if the operand is a load*/
	        
	        if (OPSCH_lstart(best_opsch) < OPSCH_lstart(cur_opsch)) best_val ++;
	        if (OPSCH_lstart(cur_opsch) < OPSCH_lstart(best_opsch)) cur_val ++;

	        if (OPSCH_lstart(best_opsch) < cur_scycle) best_val += 2;
	        if (OPSCH_lstart(cur_opsch) < cur_scycle) cur_val += 2;

	        if(OP_load(cur_op)) cur_val += 3;
	        if(OP_load(best_op)) best_val += 3;
        }
        
        if (OPSCH_num_succs(cur_opsch) > OPSCH_num_succs(best_opsch)) cur_val ++ ;
        if (OPSCH_num_succs(cur_opsch) < OPSCH_num_succs(best_opsch)) best_val ++ ;
  
  
        if (CGTARG_Max_OP_Latency(cur_op) < CGTARG_Max_OP_Latency(best_op))  best_val ++;
        if (CGTARG_Max_OP_Latency(cur_op) > CGTARG_Max_OP_Latency(best_op))  cur_val ++;
       
  	  	if (cur_val > best_val)  return TRUE;
        else if (cur_val < best_val)  return FALSE;
      }
    }

    if ((best_slack == 0) && (cur_slack !=0)) {
      if (OPSCH_num_succs(best_opsch) != 0) {
	return FALSE;
      }
      else {
	if (OP_Has_Latency(best_op)) {
	  return FALSE;
	}
	else {
	  return TRUE;
	}
      }
    }
    if ((best_slack != 0) && (cur_slack ==0)) {
      if (OPSCH_num_succs(cur_opsch) != 0) {
	return TRUE;
      }
      else {
	if (OP_Has_Latency(cur_op)) {
	  return TRUE;
	}
	else {
	  return FALSE;
	}
      }
    }
    if ((best_slack == 0) && (cur_slack ==0)) {
      if (OPSCH_estart(cur_opsch) > OPSCH_estart(best_opsch)) return TRUE;
      if (OPSCH_estart(cur_opsch) < OPSCH_estart(best_opsch)) return FALSE;
    }

    if ((OPSCH_lstart(cur_opsch) < OPSCH_lstart(best_opsch)) &&
	!(OP_Has_Latency(best_op)))
      return TRUE;
    if (OPSCH_lstart(cur_opsch) > OPSCH_lstart(best_opsch) &&
	!(OP_Has_Latency(cur_op)))
      return FALSE;
#endif

    if (cur_slack < best_slack) return TRUE;
    if (cur_slack > best_slack) return FALSE;
    
    if (OPSCH_estart(cur_opsch) > OPSCH_estart(best_opsch)) return TRUE;
    if (OPSCH_estart(cur_opsch) < OPSCH_estart(best_opsch)) return FALSE;
  }
  return FALSE;
}

List_Based_Fwd::List_Based_Fwd (BB *bb, HB_Schedule *sched, HBS_TYPE type, 
				MEM_POOL *pool) : 
  Priority_Selector(bb, sched, type, pool)
{

  // Build Ready list (list of ready candidates).
  Build_Ready_Vector (bb, this->Is_Fwd_Schedule());

  if (sched->HBS_Depth_First()) Compute_DFO (sched, sched->hb_map(),
					     this->Is_Fwd_Schedule());

  if (Trace_HB) Print_BB_For_HB (bb, sched->hb_map());

}

List_Based_Fwd::List_Based_Fwd (std::list<BB*> bblist, HB_Schedule *sched, 
				HBS_TYPE type, MEM_POOL *pool) : 
  Priority_Selector(bblist, sched, type, pool)
{

  // Build Ready list (list of ready candidates).
  Build_Ready_Vector (bblist, this->Is_Fwd_Schedule());

  if (sched->HBS_Depth_First()) Compute_DFO (sched, sched->hb_map(),
					     this->Is_Fwd_Schedule());

  if (Trace_HB) Print_BB_For_HB (bblist, sched->hb_map());

}

// ======================================================================
// Select an OP to schedule next from the Ready_Vector.
// ======================================================================
void*
List_Based_Fwd::Get_Next_Element(HB_Schedule *Cur_Sched)
{
  _best_op = NULL;

  if (Trace_HB) {
    #pragma mips_frequency_hint NEVER
    fprintf (TFile, "-------------------------------------------\n");
    fprintf (TFile, "Candidates for Scheduling:\n");
    fprintf (TFile, "-------------------------------------------\n");
  }

  for (INT i = VECTOR_count(Cur_Sched->ready_vector())-1; i >= 0; i--) 
  {
    OP *cur_op = OP_VECTOR_element (Cur_Sched->ready_vector(), i);

    // update the scycle for the <cur_op>.
    if (!OP_dummy(cur_op)) Cur_Sched->Find_Schedule_Cycle (cur_op, TRUE);

    if (Cur_Sched->HBS_Minimize_Regs() && !OP_dummy(cur_op)) {
      Cur_Sched->Estimate_Reg_Cost_For_OP (cur_op);
    }

    if (Trace_HB) {
      #pragma mips_frequency_hint NEVER
      Print_OPSCH (cur_op, Cur_Sched->hb_map());
    }

    // Replace the best_op by the cur_op if any of the following is true:
    //   1. best_op is NULL, i.e. cur_op is the first one we have seen.
    //   2. The cur_op is better based on some heuristics.
    if (_best_op == NULL || Is_OP_Better (cur_op, _best_op)) {
      _best_op = cur_op;
    }
  }

  return (void *) _best_op;
}

// ======================================================================
// ::Invoke_Pre_HBS_Phase
// All the scheduling preparatory stuff (eg. separating out the special
// instructions to prolog, epilog blocks, etc..) should be done here,
// before the actual scheduling begins.
// ===================================================================
void
HB_Schedule::Invoke_Pre_HBS_Phase(BB* bb)
{

  OP *op, *prev_op;
  OP *next_op;

  // When we are scheduling before register allocation, we don't want 
  // to schedule SP adjustment OPs in the entry/exit blocks and OPs
  // that are marked with the OP_glue attribute. We also don't want 
  // to schedule COPY instructions for save/restore of callee save 
  // registers. We check for them and move them away to temporary basic 
  // blocks. After we are done scheduling the remaining OPs, we merge the 
  // moved instructions back.

  if (HBS_Before_LRA()) {
    if (BB_entry(bb)) {
      _prolog_bb = Gen_BB_Like (bb);
      for (op = BB_entry_sp_adj_op (bb); op != NULL; op = prev_op) {
        prev_op = OP_prev(op);
        BB_Move_Op_To_Start (_prolog_bb, bb, op);
      }
    }
    if (BB_exit(bb)) {
      _epilog_bb = Gen_BB_Like (bb);
      for (op = BB_exit_sp_adj_op (bb); op != NULL; op = next_op) {
	next_op = OP_next(op);
	BB_Move_Op_To_End (_epilog_bb, bb, op);
      }
    }
    for (op = BB_first_op(bb); op != NULL; op = next_op) {
      if (!OP_copy(op) && !OP_glue(op) && !OP_no_move_before_gra(op) && !OP_access_reg_bank(op)) break;
      next_op = OP_next(op);
      if (_prolog_bb == NULL) {
	_prolog_bb = Gen_BB_Like (bb);
      }
      BB_Move_Op_To_End (_prolog_bb, bb, op);
    }
    for (op = BB_last_op(bb); op != NULL; op = prev_op) {
      prev_op = OP_prev(op);
      if (!OP_copy(op) && !OP_glue(op) && !OP_no_move_before_gra(op) && !OP_access_reg_bank(op)) {
	// check for glue copies before a branch also.
	if (!OP_xfer(op) || 
	    prev_op == NULL || 
	    (!OP_copy(prev_op) && !OP_glue(prev_op) && !OP_no_move_before_gra(op)))
	break;
      }
      if (_epilog_bb == NULL) {
	_epilog_bb = Gen_BB_Like (bb);
      }
      BB_Move_Op_To_Start (_epilog_bb, bb, op);

      // PRE-GCM can sometimes fill the delay slot with a copy op. In such
      // instances, need to move the branch as well so as to avoid later
      // filling of its delay slot.
      if (prev_op && OP_xfer(prev_op))
        BB_Move_Op_To_Start (_epilog_bb, bb, prev_op);
    }
  }
  else {

    // Alloca instructions can't be reordered.
    for (op = BB_first_op(bb); op != NULL; op = next_op) {
      if (!OP_side_effects(op)) break;
      next_op = OP_next(op);
      if (_prolog_bb == NULL) {
	_prolog_bb = Gen_BB_Like (bb);
      }
      BB_Move_Op_To_End (_prolog_bb, bb, op);
    }

    // Special case handling for regions.
    if (BB_rid(bb) != NULL && RID_cginfo(BB_rid(bb)) != NULL) {
      CGRIN *cgrin = RID_cginfo(BB_rid(bb));
      if (CGRIN_first_bb(cgrin) != NULL && 
	  BB_next(CGRIN_first_bb(cgrin)) == NULL) {
	/* only 1 bb */
	/* make sure pregtns don't get moved, even after regalloc */
	for (op = BB_first_op(bb); op != NULL; op = next_op) {
	  if (OP_code(op) != TOP_begin_pregtn) break;
	  next_op = OP_next(op);
	  if (_prolog_bb == NULL) {
	    _prolog_bb = Gen_BB_Like (bb);
	  }
	  BB_Move_Op_To_End (_prolog_bb, bb, op);
	}
      
	for (op = BB_last_op(bb); op != NULL; op = prev_op) {
	  prev_op = OP_prev(op);
	  if (OP_code(op) != TOP_end_pregtn) {
	    // check for glue copies before a branch also.
	    if (!OP_xfer(op) || prev_op == NULL || 
		(OP_code(prev_op) != TOP_end_pregtn))
	      break;
	  }
	  if (_epilog_bb == NULL) {
	    _epilog_bb = Gen_BB_Like (bb);
	  }
	  BB_Move_Op_To_Start (_epilog_bb, bb, op);
	}
      }
    }
  }
}

// ======================================================================
// ::Invoke_Pre_HBB_Phase
// All the scheduling preparatory stuff for hyperblocks (eg. separating out 
// the special instructions to prolog, epilog blocks) should be done here,
// before the actual scheduling begins. The assumption is that this needs
// to be done only for the entry/exit blocks in the hyperblock.
// ===================================================================
void
HB_Schedule::Invoke_Pre_HBB_Phase(std::list<BB*> bblist)
{

  std::list<BB*>::iterator bb_iter;
  std::list<BB*>::reverse_iterator bb_riter;

  bb_iter = bblist.begin();
  bb_riter = bblist.rbegin();

  BB *first_bb = *bb_iter; BB *last_bb = *bb_riter;

  OP *op, *prev_op;
  OP *next_op;

  // When we are scheduling before register allocation, we don't want 
  // to schedule SP adjustment OPs in the entry/exit blocks.
  // We check for them and move them away to temporary basic 
  // blocks. After we are done scheduling the remaining OPs, we merge the 
  // moved instructions back.

  if (HBS_Before_LRA()) {
    if (BB_entry(first_bb)) {
      _prolog_bb = Gen_BB_Like (first_bb);
      for (op = BB_entry_sp_adj_op (first_bb); op != NULL; op = prev_op) {
        prev_op = OP_prev(op);
        BB_Move_Op_To_Start (_prolog_bb, first_bb, op);
      }
    }
    if (BB_exit(last_bb)) {
      _epilog_bb = Gen_BB_Like (last_bb);
      for (op = BB_exit_sp_adj_op (last_bb); op != NULL; op = next_op) {
	next_op = OP_next(op);
	BB_Move_Op_To_End (_epilog_bb, last_bb, op);
      }
    }
  }

  for (op = BB_first_op(first_bb); op != NULL; op = next_op) {
    if (!OP_side_effects(op)) break;
    next_op = OP_next(op);
    if (_prolog_bb == NULL) {
      _prolog_bb = Gen_BB_Like (first_bb);
    }
    BB_Move_Op_To_End (_prolog_bb, first_bb, op);
  }
}

void
HB_Schedule::Invoke_Post_HBS_Phase(BB* bb)
{

  // If we had moved aside any instructions in the prolog or epilog of 
  // the bb, put them back in.
  if (_prolog_bb != NULL && BB_length(_prolog_bb)) {
    BB_Prepend_All (bb, _prolog_bb);
    if (HBS_Before_LRA()) Reset_BB_scheduled (bb);
  }

  if (_epilog_bb != NULL && BB_length(_epilog_bb)) {
    BB_Append_All(bb, _epilog_bb);
    if (HBS_Before_LRA()) Reset_BB_scheduled (bb);
  }

}

void
HB_Schedule::Invoke_Post_HBB_Phase(std::list<BB*> bblist)
{

  std::list<BB*>::iterator bb_iter;
  std::list<BB*>::reverse_iterator bb_riter;

  bb_iter = bblist.begin();
  bb_riter = bblist.rbegin();

  BB *first_bb = *bb_iter; BB *last_bb = *bb_riter;

  // If we had moved aside any instructions in the prolog or epilog of 
  // the bb, put them back in.
  if (_prolog_bb != NULL) {
    BB_Prepend_All (first_bb, _prolog_bb);
    if (HBS_Before_LRA()) Reset_BB_scheduled (first_bb);
  }

  if (_epilog_bb != NULL) {
    BB_Append_All(last_bb, _epilog_bb);
    if (HBS_Before_LRA()) Reset_BB_scheduled (last_bb);
  }

}

INT 
HB_Schedule::Calculate_Etime(OP *op) 
{
  OPSCH *opsch = OP_opsch(op, _hb_map);
  return OPSCH_estart(opsch);
}

INT 
HB_Schedule::Calculate_Ltime(OP *op) 
{
  OPSCH *opsch = OP_opsch(op, _hb_map);
  return OPSCH_lstart(opsch);
}

BOOL
HB_Schedule::Can_Schedule_Op (OP *cur_op, INT cur_time)
{
  if (Check_Resource_Usage (cur_op, cur_time)) return TRUE;
  
  return FALSE;
}

void
HB_Schedule::Schedule_Block (BB *bb, BBSCH *bbsch, bool is_fwd)
{
  // keep old behavior on non targ_st code.
#ifdef TARG_IA64
  is_fwd = TRUE;
#else
#ifndef TARG_ST
  is_fwd = FALSE;
#endif
#endif

  _sched_vector = VECTOR_Init (BB_length(bb), &_hb_pool);

  std::list<BB*> blocks;
  blocks.push_back(bb);

  Init_RFlag_Table (blocks, is_fwd);

  Before_LRA = HBS_Before_LRA();
  Compute_OPSCH (bb, _hb_map, &_hb_pool);

  if (HBS_Minimize_Regs()) {
    Init_Register_Map (bb);
  }

  Priority_Selector *priority_fn;
  Cycle_Selector *cycle_fn;

  if (is_fwd)
    {
    // Do forward scheduling and cycle selector.
    priority_fn = 
      CXX_NEW(List_Based_Fwd(bb, this, _hbs_type, &_hb_pool), &_hb_pool);
    cycle_fn = CXX_NEW(Fwd_Cycle_Sel(), &_hb_pool);

    }
  else
    {
      // Do backward scheduling and cycle selector.
      priority_fn = 
        CXX_NEW(List_Based_Bkwd(bb, this, _hbs_type, &_hb_pool), &_hb_pool);
      cycle_fn = CXX_NEW(Bkwd_Cycle_Sel(), &_hb_pool);
    }

  OP *cur_op;
  OP *xfer_op = BB_xfer_op(bb);

  // If backward schedule and the basic block ends in a control transfer,
  // try to schedule the delay slot (if present) first and then schedule
  // the xfer_op.

  if (!priority_fn->Is_Fwd_Schedule()) {
    if (xfer_op) {
#ifdef TARG_IA64
      if (PROC_has_branch_delay_slot()) {
#else
      if (PROC_Has_Branch_Delay_Slot()) {
#endif
	cur_op = priority_fn->Select_OP_For_Delay_Slot (xfer_op);
	if (cur_op) {
	  Add_OP_To_Sched_Vector(cur_op, priority_fn->Is_Fwd_Schedule());
	}
      }
      Add_OP_To_Sched_Vector(xfer_op, priority_fn->Is_Fwd_Schedule());
    }
  }

  INT cur_time;
  // Now iterate through the rest of the ops.
  for (cur_op = (OP*) priority_fn->Get_Next_Element(this); cur_op != NULL; 
       cur_op = (OP*) priority_fn->Get_Next_Element(this)) {

    if (!OP_dummy(cur_op)) {
      INT etime = Calculate_Etime(cur_op);
      INT ltime = Calculate_Ltime(cur_op);
      cycle_fn->Init(cur_op, etime, ltime);

      for (cur_time = cycle_fn->Get_Cycle(); cur_time >= cycle_fn->Bound(); 
	   cur_time = cycle_fn->Next_Cycle()) {
	if (Can_Schedule_Op(cur_op, cur_time)) break;
      }
    }

    //    Is_True(cur_time < cycle_fn->Bound(),("Invalid cycle boundary, HB_SCHED"));

    Add_OP_To_Sched_Vector(cur_op, priority_fn->Is_Fwd_Schedule());
  }
  
  if (priority_fn->Is_Fwd_Schedule()) {
    if (xfer_op) {
#ifdef TARG_IA64
      if (PROC_has_branch_delay_slot())
#else
      if (PROC_Has_Branch_Delay_Slot())
#endif
          {
            FmtAssert (TRUE,("HB_SCHED: delay slot not implemented for fwd scheduling"));
          }
      
      OPSCH *ready_opsch = OP_opsch (xfer_op, _hb_map);
      OPSCH_scycle(ready_opsch) = MAX (Clock, OPSCH_scycle(ready_opsch));
      
      Add_OP_To_Sched_Vector(xfer_op, priority_fn->Is_Fwd_Schedule());
    }
  }
 
  // Insert the scheduled list of instructions into the bb.
  Put_Sched_Vector_Into_BB (bb, bbsch, priority_fn->Is_Fwd_Schedule());

}

void
HB_Schedule::Schedule_Blocks (std::list<BB*>& bblist)
{
  std::list<BB*>::iterator bbi;
  UINT32 length = 0;

  FOR_ALL_BB_STLLIST_ITEMS_FWD(bblist, bbi) {
    length += BB_length(*bbi);
  }

  _sched_vector = VECTOR_Init (length, &_hb_pool);

  Init_RFlag_Table (bblist, TRUE);
  Before_LRA = HBS_Before_LRA();
  Compute_OPSCHs (bblist, _hb_map, &_hb_pool);

  /* TODO: Need to model register usages for hyperblocks soon. 
  if (Cur_Locs_Type & HBS_MINIMIZE_REGS) {
    Init_Register_Map (bb);
  } */

  List_Based_Fwd *priority_fn = 
    CXX_NEW(List_Based_Fwd(bblist, this, _hbs_type, &_hb_pool), &_hb_pool);

  Fwd_Cycle_Sel *cycle_fn = CXX_NEW(Fwd_Cycle_Sel(), &_hb_pool);

  OP *cur_op;
  INT cur_time;

  // Now iterate through the rest of the ops.
  for (cur_op = (OP*) priority_fn->Get_Next_Element(this); cur_op != NULL; 
       cur_op = (OP*) priority_fn->Get_Next_Element(this)) {

    if (!OP_dummy(cur_op)) {
      INT etime = Calculate_Etime(cur_op);
      INT ltime = Calculate_Ltime(cur_op);
      cycle_fn->Init(cur_op, etime, ltime);

      for (cur_time = cycle_fn->Get_Cycle(); cur_time != cycle_fn->Bound(); 
	   cur_time = cycle_fn->Next_Cycle()) {

	if (Can_Schedule_Op(cur_op, cur_time)) break;
      }
    }

    // Is_True(cur_time >= cycle_fn->Bound(), ("Invalid cycle boundary, HB_SCHED"));

    Add_OP_To_Sched_Vector(cur_op, priority_fn->Is_Fwd_Schedule());
  }
    
  // Insert the scheduled list of instructions into the bb.
  Put_Sched_Vector_Into_HB (bblist);

}

// ======================================================================
// HB_Schedule
//
// Algorithm:
//
//    1. Build a list of OPs that is ready to be scheduled. This is
//       the list of OPs with no successors in the dependence graph.
//    2. Select one of the OPs from the ready list based on some
//       heuristics as the next OP to schedule.
//    3. Delete the scheduled OP from the ready list and add any new
//       ones that might be ready now.
//    4. Repeat steps 2. and 3. till all OPs have been scheduled.
//    5. Put the scheduled list of OPs back into the basic block.
// ======================================================================

HB_Schedule::HB_Schedule()
{
  _prolog_bb = NULL;
  _epilog_bb = NULL;
#ifdef TARG_ST
  minimize_regs_class = ISA_REGISTER_CLASS_UNDEFINED;
#endif

  // Initialize memory pool for use in the scheduling this bb.
  MEM_POOL_Initialize (&_hb_pool, "HB_pool", FALSE);
  MEM_POOL_Initialize (&_hb_map_pool, "HB_map_pool", FALSE);
  MEM_POOL_Push(&_hb_pool);
  MEM_POOL_Push (&_hb_map_pool);

  _hb_map = BB_MAP_Create ();
  Trace_HB = Get_Trace (TP_SCHED, 1);
}

#ifdef TARG_ST
void
HB_Schedule::Init(BB *bb, HBS_TYPE hbs_type, INT32 max_sched,
		  BBSCH *bbsch, mINT8 *regs_avail,
		  ISA_REGISTER_CLASS cl, INT32 cl_limit)
{
  Init (bb, hbs_type, max_sched, bbsch, regs_avail);
  minimize_regs_class = cl;
  // cl_limit is the number of registers LRA wants to allocate the BB.
  // regs_avail[cl] is the number of registers GRA has granted to LRA.
  // We set the threshold to be the number of excess registers we have,
  // i.e. regs_avail - cl_limit.  Usually, this value will be negative,
  // since we are lacking registers.
  // During scheduling, we track cur_regs_avail(i), the number of
  // registers available at instruction i, and also its minimum,
  // min_cur_regs_avail.
  // When min_cur_regs_avail is negative, e.g. -5, it means we are lacking
  // 5 registers at the point of highest register pressure in the BB.
  // If min_cur_regs_avail > threshold, it means
  // we are lacking fewer registers than before, so the rescheduling
  // has reduced register pressure, which is what we want.
  if (cl != ISA_REGISTER_CLASS_UNDEFINED)
    minimize_regs_threshold = regs_avail[cl] - cl_limit;
}
#endif

#ifdef TARG_ST
void
HB_Schedule::Init(BB *bb, HBS_TYPE hbs_type)
{
	Init(bb, hbs_type, computed_max_sched, NULL,NULL);
}
#endif


void
HB_Schedule::Init(BB *bb, HBS_TYPE hbs_type, INT32 max_sched,
		  BBSCH *bbsch, mINT8 *regs_avail)
{
  _hbs_type = hbs_type;
  _max_sched = max_sched;
  if (regs_avail) {
    for (INT i = ISA_REGISTER_CLASS_MIN;i <= ISA_REGISTER_CLASS_MAX; i++)   
#ifdef TARG_ST
      {
	_Cur_Regs_Avail[i] = regs_avail[i];
	_Min_Cur_Regs_Avail[i] = regs_avail[i];
      }
#else
      _Cur_Regs_Avail[i] = regs_avail[i];
#endif
  }

  BB_OP_MAP omap = BB_OP_MAP_Create(bb, &_hb_map_pool);
  BB_MAP_Set(_hb_map, bb, omap);

  _ready_vector = VECTOR_Init (BB_length(bb), &_hb_pool);
}

void
HB_Schedule::Init(std::list<BB*> bblist, HBS_TYPE hbs_type, mINT8 *regs_avail)
{
  _hbs_type = hbs_type;
  if (regs_avail) {
    for (INT i = ISA_REGISTER_CLASS_MIN;i <= ISA_REGISTER_CLASS_MAX; i++)   
#ifdef TARG_ST
      {
	_Cur_Regs_Avail[i] = regs_avail[i];
	_Min_Cur_Regs_Avail[i] = regs_avail[i];
      }
#else
      _Cur_Regs_Avail[i] = regs_avail[i];
#endif
  }

  UINT32 length = 0;
  std::list<BB*>::iterator bbi;

  FOR_ALL_BB_STLLIST_ITEMS_FWD(bblist, bbi) {
    BB_OP_MAP omap = BB_OP_MAP_Create(*bbi, &_hb_map_pool);
    BB_MAP_Set(_hb_map, *bbi, omap);
    length += BB_length(*bbi);
  }

  _ready_vector = VECTOR_Init (length, &_hb_pool);
}

void
HB_Schedule::Schedule_BB (BB *bb, BBSCH *bbsch, bool is_fwd)
{

  Invoke_Pre_HBS_Phase(bb);

  std::list<BB*> bblist;
  bblist.push_back(bb);

  if (CG_DEP_Prune_Dependence &&  // if the flag is turned ON.
      CGTARG_Can_Predicate() &&
      !BB_predicate_promote(bb))   // if the target arch provides predication.
    {
      CG_DEP_Prune_Dependence_Arcs(bblist, PRUNE_PREDICATE_ARCS, FALSE);
      Set_BB_predicate_promote(bb);
    }

  // if there are only zero or one instructions in the basic block,
  // there is nothing to schedule.
  BOOL skip_sched = FALSE;;
  if (CG_skip_local_sched) {
    BBs_Processed++;
    skip_sched =  (BBs_Processed < CG_local_skip_before ||
		   BBs_Processed > CG_local_skip_after ||
		   BBs_Processed == CG_local_skip_equal);
    if (!skip_sched)
      fprintf (TFile, "[%d] BB:%d processed in HB_Schedule_BB\n", 
	BBs_Processed, BB_id(bb));
  }

  if (BB_length(bb) > 0 && !skip_sched) {
    if (BB_length(bb) > 1) {

      CG_DEP_Compute_Graph (
	  bb, 
	  (this->HBS_From_CGPREP()) ? NO_ASSIGNED_REG_DEPS : 
	                               INCLUDE_ASSIGNED_REG_DEPS,
	  NON_CYCLIC,
	  INCLUDE_MEMREAD_ARCS,
	  INCLUDE_MEMIN_ARCS,
	  (Is_Target_Itanium()) ? INCLUDE_CONTROL_ARCS : NO_CONTROL_ARCS,
	  NULL);

      if (Trace_HB) CG_DEP_Trace_Graph (bb);

      Schedule_Block (bb, bbsch, is_fwd);

      CG_DEP_Delete_Graph (bb);
    }
    Set_BB_scheduled (bb);
    Set_BB_scheduled_hbs (bb);  // scheduled from hbs
    if (Assembly) Add_Scheduling_Note (bb, (void*) bbsch);
  }
  
  Invoke_Post_HBS_Phase(bb);

  // If we are scheduling before GRA, compute an estimate of the registers
  // that will be needed by LRA. No need to compute register request, if
  // HBS is invoked from CGPREP or for the second time within GCM. We model
  // the local register usage counts separately.

  if (HBS_Before_GRA() && !HBS_From_CGPREP()) {

#ifdef TARG_ST
	  if(!(CG_sched_mask & 0x2000)) {
#else
	  if (!Get_Trace (TP_SCHED, 0x2000)) {
#endif
      // Assumes that <bbsch> is computed fopass      
      mINT8 *fatpoint = (_hbs_type & HBS_FROM_PRE_GCM_SCHED_AGAIN) ?
#ifdef TARG_ST
      BBSCH_local_regcost(bbsch) : LRA_Compute_Register_Request(bb, &_hb_pool)->summary;
#else
      BBSCH_local_regcost(bbsch) : LRA_Compute_Register_Request(bb, &_hb_pool);
#endif
      if (HBS_From_Pre_GCM_Sched()) {
        Set_BB_local_regcost(bbsch, fatpoint);
      }
    }
  }
}

void
HB_Schedule::Schedule_HB (std::list<BB*> bblist)
{

  Invoke_Pre_HBB_Phase(bblist);

  if (CG_DEP_Prune_Dependence &&  // if the flag is turned ON.
      CGTARG_Can_Predicate())   // if the target arch provides predication.
    {
      CG_DEP_Prune_Dependence_Arcs(bblist, PRUNE_PREDICATE_ARCS, FALSE);
    }

  CG_DEP_Compute_Region_Graph(bblist, 
			      (this->HBS_From_CGPREP()) ? 
			      NO_ASSIGNED_REG_DEPS : INCLUDE_ASSIGNED_REG_DEPS,
			      INCLUDE_MEMREAD_ARCS,
			      INCLUDE_CONTROL_ARCS);

  Schedule_Blocks (bblist);

  CG_DEP_Delete_Graph (&bblist);

  Invoke_Post_HBB_Phase(bblist);
  std::list<BB*>::iterator bbi;
  FOR_ALL_BB_STLLIST_ITEMS_FWD(bblist, bbi) {
    Set_BB_scheduled (*bbi);
    Set_BB_scheduled_hbs (*bbi);  // scheduled from hbs
  }

  // if (Assembly) Add_Scheduling_Note (bb, (void*) bbsch);
  
}

HB_Schedule::~HB_Schedule()
{
  BB_MAP_Delete (_hb_map);

  MEM_POOL_Pop (&_hb_pool);
  MEM_POOL_Pop (&_hb_map_pool);
  MEM_POOL_Delete (&_hb_pool);
  MEM_POOL_Delete (&_hb_map_pool);
}
