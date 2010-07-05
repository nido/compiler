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
 *  Module: ebo_util.h
 *
 *  Revision comments:
 *
 *  17-June-1998 - Initial version
 *
 *  Description:
 *  ============
 *
 *  EBO_tn_available
 *	Return TRUE iff the TN associated with the tninfo entry is
 *	currently available for use.
 *
 *  OP_effectively_copy
 *	Return TRUE iff <op> is a copy operation.
 *
 *  EBO_hash_op
 *	This routine will generate a hash value for the OP.
 *
 *  add_to_hash_table
 *	This routine will add a new expression to the hash table.
 *	While doing so, it will create defining EBO_TN_INFO entries
 *	for the result of the OP.
 *
 *  tn_registers_identical
 *	This routine compares 2 TN's to determine if they represent the
 *	same register.
 *
 *  locate_opinfo_entry
 *	This routine will locate the EBO_OP_INFO entry that
 *	is associated with a particular EBO_TN_INFO entry.
 *	It is used to locate the OP that defines the current value
 *	that is in a TN.
 *
 *  mark_tn_live_into_BB
 *	This routine performs the book keeping work associated with
 *	creating a reference to a TN that crosses block boundaries.
 *
 *   tn_has_live_def_into_BB
 *	This routine checks to see if a TN has a valid definition that
 *	is input to the current block.
 *
 *  EBO_OP_predicate_dominates(op1, op1_tninfo, op2, op2_tninfo)
 *
 *   Check whether the execution condition p1 for op1 dominates
 *   the execution condition p2 for op2 by checking whether
 *   EBO_predicate_domintes(p1, p2).
 *
 * =======================================================================
 * =======================================================================
 */

#ifdef TARG_ST
extern void EBO_Set_OP_omega (OP *op, ...);
extern void EBO_OPS_omega (OPS *ops, TN *opnd, EBO_TN_INFO *opnd_tninfo);
extern void EBO_OP_merge_omega ( OP *old_op, EBO_TN_INFO **old_opnd_tninfo, OP *new_op );
extern BOOL TN_live_out_of(TN *tn, BB *bb);
extern BOOL EBO_OP_predicate_dominates(OP *op1, EBO_TN_INFO **op1_opnd_tninfo,
                                       OP *op2, EBO_TN_INFO **op2_opnd_tninfo);
#endif

inline
BOOL
EBO_tn_available(BB *bb,
                 EBO_TN_INFO *tninfo)
{
  TN *tn;
  BB *tn_bb;

 /* Safety check. */
  if ((tninfo == NULL) ||
      (tninfo->local_tn == NULL)) return FALSE;
  tn = tninfo->local_tn;
  tn_bb = tninfo->in_bb;
 /* Constants are always available. */
  if (TN_Is_Constant(tn)) return TRUE;
 /* Does a TN look-up get us back to where we start? */
#ifdef TARG_ST
  // [SC] We can ignore dummy entries on the tninfo same list,
  // when trying to find a match.
  {
    EBO_TN_INFO *avail_tninfo;
    for (avail_tninfo = get_tn_info (tn);
	 avail_tninfo != NULL;
	 avail_tninfo = avail_tninfo->same) {
      if (avail_tninfo == tninfo) {
	break; // found a match.
      }
      if (avail_tninfo->in_op != NULL) {
	break; // found a real definition, cannot ignore
      }
      // Must be a dummy tninfo, so keep looking.
    }
    if (avail_tninfo != tninfo) return FALSE;
  }
#else
  if (get_tn_info(tn) != tninfo) return FALSE;
#endif
 /* Global TN's aren't supported at low levels of optimization. */
  if ((Opt_Level < 2) && (bb != tn_bb)) return FALSE;
 /* Some forms of data movement aren't supported.
    - Mainly those involving dedicated TN's.
    So restrict use of registers across block boundaries
    unless registers have already been assigned.  */
  if (!EBO_in_peep &&
      ((bb != tn_bb) &&
       has_assigned_reg(tn))) return FALSE;
  return TRUE;
}


inline
BOOL
OP_effectively_copy(OP *op)
{
  if (OP_copy(op)) return TRUE;
  if (OP_Is_Copy(op)) return TRUE;
  return FALSE;
}



inline
INT
EBO_hash_op (OP *op,
             EBO_TN_INFO **based_on_tninfo)
{
  INT hash_value = 0;
  if (OP_memory(op)) {
    hash_value = EBO_DEFAULT_MEM_HASH;
    if (OP_no_alias(op)) hash_value = EBO_NO_ALIAS_MEM_HASH;
#ifdef TARG_ST
    // [CG] Spill op are marked by OP_spill
    // Getting the spill_tn is unsafe for stores
    if (OP_spill(op)) hash_value = EBO_SPILL_MEM_HASH;
#else
    TN * spill_tn = NULL;
    if (OP_load(op)) {
      spill_tn = OP_result(op,0);
    } else if (OP_store(op)) {
      spill_tn = OP_opnd(op,TOP_Find_Operand_Use(OP_code(op), OU_storeval));
    }
    if (spill_tn && TN_has_spill(spill_tn)) hash_value = EBO_SPILL_MEM_HASH;
#endif
  } else if (OP_effectively_copy(op)) {
    hash_value = EBO_COPY_OP_HASH;
  } else {
    INT opcount = OP_opnds(op);
    INT opndnum;
    hash_value =
#ifdef TARG_IA64
                 ((OP_results(op) == 2) &&
                  ((OP_result(op,0) != NULL) &&
                   ((OP_result(op,0) == True_TN) ||
                    (TN_register_class(OP_result(op,0)) == ISA_REGISTER_CLASS_predicate)))) ? 0 :
#endif
                 (INT)OP_code(op);
#ifdef TARG_ST
    // (cbr) fix pred #
    for (opndnum = 0; opndnum < opcount; opndnum++) {
      if (opndnum != OP_find_opnd_use(op, OU_predicate))
	hash_value+=(INT)(INTPS)based_on_tninfo[opndnum];
    }
#else
    for (opndnum = OP_has_predicate(op)?1:0; opndnum < opcount; opndnum++) {
      hash_value+=(INT)(INTPS)based_on_tninfo[opndnum];
    }
#endif
    hash_value = EBO_RESERVED_OP_HASH + EBO_EXP_OP_HASH(hash_value);
  }
  return hash_value;
}



inline
void
add_to_hash_table ( BOOL in_delay_slot,
                    OP *op,
#ifdef TARG_ST
		    // FdF 20070402
		    TN **opnd_tn,
#endif
                    EBO_TN_INFO **actual_tninfo,
                    EBO_TN_INFO **optimal_tninfo)
{
  INT hash_value = EBO_hash_op(op, optimal_tninfo);
  INT idx;
  EBO_OP_INFO *opinfo;
  BB *bb = OP_bb(op);
  TN *op_predicate_tn;
  EBO_TN_INFO *op_predicate_tninfo;

 /* Create a new opinfo entry and add the new op to the hash table. */
  opinfo = get_new_opinfo(op);
  opinfo->in_bb = OP_bb(op);
  opinfo->in_op = op;
  opinfo->in_delay_slot = in_delay_slot;
  opinfo->hash_index = hash_value;
  opinfo->same  = EBO_opinfo_table[hash_value];

 /* Define the result TN's and set the result fields. */
  if (OP_has_predicate(op)) {
    op_predicate_tn = OP_opnd(op, TOP_Find_Operand_Use(OP_code(op), OU_predicate));
    op_predicate_tninfo = actual_tninfo[TOP_Find_Operand_Use(OP_code(op), OU_predicate)];
  } else {
    op_predicate_tn = NULL;
    op_predicate_tninfo = NULL;
  }

  for (idx = 0; idx < OP_results(op); idx++) {
    EBO_TN_INFO *tninfo = NULL;
    TN *tnr = OP_result(op, idx);
    if ((tnr != NULL) && (tnr != True_TN) && (tnr != Zero_TN)) {
      tninfo = tn_info_def (bb, op, tnr, op_predicate_tn, op_predicate_tninfo);
      tninfo->in_opinfo = opinfo;
    }
    opinfo->actual_rslt[idx] = tninfo;
  }

 /* Copy all the tninfo entries for the operands. */
  for (idx = 0; idx < OP_opnds(op); idx++) {
    opinfo->actual_opnd[idx] = actual_tninfo[idx];
    opinfo->optimal_opnd[idx] = optimal_tninfo[idx];
#ifdef TARG_ST
    // FdF 20070402
    if (idx < OP_MAX_FIXED_OPNDS)
      if (opnd_tn != NULL)
	opinfo->optimal_tn[idx] = opnd_tn[idx];
      else
	opinfo->optimal_tn[idx] = OP_opnd(op, idx);
#endif
  }

  EBO_opinfo_table[hash_value] = opinfo;

  if (EBO_Trace_Data_Flow) {
    #pragma mips_frequency_hint NEVER
    fprintf(TFile,"%sCreate new opinfo entry at %d for:\n\t",EBO_trace_pfx,hash_value);
    Print_OP_No_SrcLine(op);
  }
}




inline
BOOL
tn_registers_identical (TN *tn1, TN *tn2)
{
  return ((tn1 == tn2) ||
          ((TN_is_register(tn1) && TN_is_register(tn2) &&
           (TN_is_dedicated(tn1) || (TN_register(tn1) != REGISTER_UNDEFINED)) &&
           (TN_is_dedicated(tn2) || (TN_register(tn2) != REGISTER_UNDEFINED)) &&
	   (TN_register_and_class(tn1) == TN_register_and_class(tn2)))));
}




inline
EBO_OP_INFO *
find_opinfo_entry (OP *op)
{
  EBO_OP_INFO *opinfo;
  INT limit_search = 50;  /* Give up after a while. */

  for (opinfo = EBO_last_opinfo;
       ((opinfo != NULL) && (limit_search != 0));
       opinfo = opinfo->prior, limit_search--) {
    if (opinfo->in_op == op) return opinfo;
  }

  return NULL;
}




inline
EBO_OP_INFO *
locate_opinfo_entry (EBO_TN_INFO *tninfo)
{

  if ((tninfo != NULL) && (tninfo->in_op != NULL)) {
    return (tninfo->in_opinfo != NULL) ? tninfo->in_opinfo : find_opinfo_entry(tninfo->in_op);
  }

  return NULL;
}




inline
void
#ifdef TARG_ST
// (cbr) handle inversed predicates
EBO_OPS_predicate(TN *predicate_tn, bool on_false, bool cond_def, OPS *ops)
#else
EBO_OPS_predicate(TN *predicate_tn, OPS *ops)
#endif
{
  OP *next_op = OPS_first(ops);

#ifdef TARG_ST
  // FdF 20081015: On the first OP, set the UNC_DEF property if
  // needed.
  if (!TN_is_true (predicate_tn) && !cond_def && next_op)
    Set_OP_cond_def_kind (next_op, OP_ALWAYS_UNC_DEF);
#endif

  while (next_op != NULL) {
    if (OP_has_predicate(next_op)) {
      Set_OP_opnd(next_op, 
		  TOP_Find_Operand_Use(OP_code(next_op), OU_predicate), 
		  predicate_tn);

#ifdef TARG_ST
      // (cbr) Support for guards on false
      if (on_false)
	Set_OP_Pred_False(next_op, TOP_Find_Operand_Use(OP_code(next_op), OU_predicate));

      if (!TN_is_true (predicate_tn))
	Set_OP_cond_def_kind (next_op, OP_PREDICATED_DEF);

#endif

    }
    next_op = OP_next(next_op);
  }
}

#ifdef TARG_ST
// FdF 20081017: Check whether a predicated copy is available or not,
// before generating a predicated copy with an UNC_DEF property or an
// unconditional copy.
inline BOOL
EBO_Has_Predicated_Copy (TN *predicate_tn, TN *tgt_tn, TN *src_tn) {
  OPS dummy_ops = OPS_EMPTY;
  Expand_Copy(tgt_tn, predicate_tn, src_tn, &dummy_ops);
  return (OPS_length (&dummy_ops) == 1
	  && OP_copy (OPS_first(&dummy_ops))
	  && OP_is_predicated (OPS_first(&dummy_ops)));
}
#endif

inline
void
#ifdef TARG_ST
// (cbr) Support for guards on false
// FdF 20081015: Support for ALWAYS_UNC_DEF
EBO_Exp_COPY(TN *predicate_tn, bool on_false, bool cond_def, TN *tgt_tn, TN *src_tn, OPS *ops)
#else
EBO_Exp_COPY(TN *predicate_tn, TN *tgt_tn, TN *src_tn, OPS *ops)
#endif
{
#ifdef TARG_ST
  // If registers are identical we can generate a noop instead of a copy,
  // unless we are before register allocation and one of the registers 
  // is dedicated. In the later case we must keep the explicit dedicated
  // use or def as it expresses parameter passing information.
  if (tn_registers_identical (tgt_tn, src_tn) &&
      !Is_Predicate_REGISTER_CLASS(TN_register_class(tgt_tn)) &&
      (EBO_in_peep ||
       (!TN_is_dedicated(tgt_tn) && !TN_is_dedicated(src_tn)) ||
       REGISTER_SET_EmptyP(REGISTER_CLASS_allocatable(TN_register_class(src_tn))))) {
    Build_OP(TOP_noop,ops);
    return;
  }
  // FdF 20081017: On ST200, since there is no predicated copy, it is
  // better to generate an unconditional move if cond_def is false.
  if (!cond_def && (predicate_tn != NULL) &&
      !EBO_Has_Predicated_Copy(predicate_tn, tgt_tn, src_tn)) {
    predicate_tn = NULL;
  }
  if (predicate_tn != NULL) {
    Expand_Copy(tgt_tn, predicate_tn, src_tn, ops);
    if (on_false) {
      OP *next_op = OPS_first(ops);
      while (next_op != NULL) {
	if (OP_has_predicate(next_op)) {
	  // (cbr) Support for guards on false
	  Set_OP_Pred_False(next_op, TOP_Find_Operand_Use(OP_code(next_op), OU_predicate));
	}
	next_op = OP_next(next_op);
      }
    }
    if (!cond_def)
      Set_OP_cond_def_kind(OPS_first(ops), OP_ALWAYS_UNC_DEF);
    return;
  }
#endif
  Exp_COPY(tgt_tn, src_tn, ops);
#ifndef TARG_ST
  if (predicate_tn != NULL) {
    EBO_OPS_predicate (predicate_tn, ops);
  }
#endif
}

#ifdef TARG_ST
inline
void
EBO_Exp_SELECT(TN *tgt_tn, TN *predicate_tn, TN *true_tn, TN *false_tn, OPS *ops)
{
  TYPE_ID mtype;
  if (TN_is_float (true_tn)) {
    mtype = (TN_size (true_tn) == 8) ? MTYPE_F8 : MTYPE_F4;
  } else {
    mtype = (TN_size (true_tn) == 8) ? MTYPE_I8 : MTYPE_I4;
  }

  Expand_Select (tgt_tn, predicate_tn, true_tn, false_tn, mtype,
		 TN_is_float (predicate_tn), ops);
}
#endif

inline
void
mark_tn_live_into_BB (TN *tn, BB *into_bb, BB *outof_bb)
/* -----------------------------------------------------------------------
 * mark_tn_live_into_BB
 *
 * Mark a TN as "live into" the blocks that are between the two arguments.
 *
 * Assume that each block interving has exactly one predecessor.
 * 
 *-----------------------------------------------------------------------
 */
{
  if (into_bb != outof_bb) {
    BB *in_bb;
    BOOL already_global = TRUE;

    if (!TN_is_global_reg(tn)) {
      GTN_UNIVERSE_Add_TN(tn);
      already_global = FALSE;
    }

    if (EBO_in_peep) {
      if (!REG_LIVE_Into_BB( TN_register_class(tn), TN_register(tn), into_bb)) {
        ISA_REGISTER_CLASS tn_class = TN_register_class(tn);
        REGISTER tn_reg = TN_register(tn);
        FmtAssert((has_assigned_reg(tn)),
                  ("No assigned register in ebo utility mark_tn_live_into_BB"));

       /* Mark REGISTER as being used in intervening blocks. */
        in_bb = into_bb;
        while ((in_bb != NULL) && (in_bb != outof_bb)) {
          REG_LIVE_Update( tn_class, tn_reg, in_bb);
          in_bb = BB_First_Pred(in_bb);
        }

      }
    } else {
     /* Record TN information. */

     /* Always mark it used. */
      GRA_LIVE_Add_Live_Use_GTN(into_bb, tn);

     /* May also need to propagate liveness information. */
      if (!GRA_LIVE_TN_Live_Into_BB(tn, into_bb)) {
        GRA_LIVE_Add_Live_In_GTN(into_bb, tn);
        GRA_LIVE_Add_Defreach_In_GTN(into_bb, tn);

        if (!already_global) {
         /* If it was a local TN, it was defined in the "outof"
            block.  Otherwise, the Def information was already
            determined for this TN and we shouldn't change it. */
          GRA_LIVE_Add_Live_Def_GTN(outof_bb, tn);
        }
        GRA_LIVE_Add_Live_Out_GTN(outof_bb, tn);
        GRA_LIVE_Add_Defreach_Out_GTN(outof_bb, tn);

       /* Mark TN as being carried into and out of intervening blocks. */
        in_bb = BB_First_Pred(into_bb);
        while ((in_bb != NULL) && (in_bb != outof_bb)) {
          GRA_LIVE_Add_Live_In_GTN(in_bb, tn);
          GRA_LIVE_Add_Defreach_In_GTN(in_bb, tn);
          GRA_LIVE_Add_Live_Out_GTN(in_bb, tn);
          GRA_LIVE_Add_Defreach_Out_GTN(in_bb, tn);
          in_bb = BB_First_Pred(in_bb);
        }

      }
    }
  }
}


inline
BOOL
tn_has_live_def_into_BB (TN *tn, BB *into_bb)
/* -----------------------------------------------------------------------
 * tn_has_live_def_into_BB
 *
 * Return A TRUE if a TN has a reaching definition into the specified block.
 *
 *-----------------------------------------------------------------------
 */
{
  if (EBO_in_peep) {
    return TRUE;  /* There is no way to know. */
  } else {
    return (TN_is_dedicated(tn) ||
            GTN_SET_MemberP(BB_defreach_in(into_bb), tn));
  }
}