/*

  Copyright (C) 2006 ST Microelectronics.  All Rights Reserved.

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

  Contact information:  , or:

  http://www.st.com

*/


/* ====================================================================
 * ====================================================================
 * Module: cgdwarf_debug_frame.cxx
 *
 * Description:
 *
 * Interface between cgemit.cxx and em_dwarf.cxx for all the
 * debug_frame related support
 *
 * ====================================================================
 * ====================================================================
 */

#if !defined(TARG_ST) // TODO POST MERGE: sync with targinfo/stxp70/cg/targ_cgdwarf.cxx

#include <stdio.h>
#include <stdlib.h>
#include <libelf.h>

#include "W_alloca.h"
#include "tracing.h"
#include "config_asm.h"
#include "config_list.h"
#include "glob.h"

#include "cgir.h"
#include "tn_map.h"
#include "em_elf.h"
#include "dwarf_DST_mem.h"         /* for DST_IDX */
#include "em_dwarf.h"

#include "cgemit.h"
#include "cgdwarf.h"               /* [CL] for CGD_LABIDX */
#include "cgdwarf_debug_frame.h"

BOOL Trace_Unwind = FALSE;

BOOL has_asm;

// use list not slist cause append to end
static list < UNWIND_ELEM > ue_list;
static list < UNWIND_ELEM >::iterator ue_iter;
static UINT last_when;
static UINT last_label = 0;
static TN_MAP tn_def_op;

BOOL PU_has_FP = FALSE;  // does the current PU use a frame pointer?
BOOL PU_has_restored_FP = FALSE; // has the current PU already restored FP?

static BOOL UE_validated; // was the Tag_Irrelevant_Saves_And_Restores
			  // pass alreadly run?

static const char *
UE_Register_Name (ISA_REGISTER_CLASS rc, REGISTER r)
{
  return REGISTER_name(rc,r);
}

void
Print_Unwind_Elem (UNWIND_ELEM ue, char *msg)
{
	fprintf(TFile, "<%s> at bb %d when %d: ", msg, ue.bb->id, ue.when);
	switch (ue.kind) {
	case UE_CREATE_FRAME:
		fprintf(TFile, " create_frame, size %lld",
			ue.offset); 
		break;
	case UE_DESTROY_FRAME:
		fprintf(TFile, " destroy_frame, size %lld",
			ue.offset);
		break;
	case UE_CREATE_FP:
		fprintf(TFile, " create_fp, size %lld",
			ue.offset); 
		break;
	case UE_DESTROY_FP:
		fprintf(TFile, " destroy_fp");
		break;
	case UE_EPILOG:
		fprintf(TFile, " epilog"); 
		break;
	case UE_LABEL:  
		fprintf(TFile, " label state %d",
			ue.label);
		break;
	case UE_COPY:  
		fprintf(TFile, " copy label state %d",
			ue.label);
		break;
	case UE_SAVE_GR:  
		fprintf(TFile, " save %s in gr %s",
		    UE_Register_Name(
			CLASS_REG_PAIR_rclass(ue.rc_reg),
        		CLASS_REG_PAIR_reg(ue.rc_reg) ),
		    UE_Register_Name(
			CLASS_REG_PAIR_rclass(ue.save_rc_reg),
        		CLASS_REG_PAIR_reg(ue.save_rc_reg) ));
		break;
	case UE_SAVE_SP:
	case UE_SAVE_FP:
		fprintf(TFile, " save %s", UE_Register_Name(
				CLASS_REG_PAIR_rclass(ue.rc_reg),
        			CLASS_REG_PAIR_reg(ue.rc_reg) ));
#ifdef PROPAGATE_DEBUG
		fprintf(TFile, " to mem %lld(%s) %lld(entry-%s)", ue.offset,
			(ue.kind == UE_SAVE_SP ? "sp" : "fp"), ue.top_offset,
#else
		fprintf(TFile, " to mem %lld(%s)", ue.offset,
#endif
			(ue.kind == UE_SAVE_SP ? "sp" : "fp") );
		break;
	case UE_RESTORE_GR:  
		fprintf(TFile, " restore %s", UE_Register_Name(
			CLASS_REG_PAIR_rclass(ue.rc_reg),
        		CLASS_REG_PAIR_reg(ue.rc_reg) ));
			if (!CLASS_REG_PAIR_EqualP(ue.save_rc_reg,
						   CLASS_REG_PAIR_undef)) {
			  fprintf(TFile, " from %s",
				  UE_Register_Name(CLASS_REG_PAIR_rclass(ue.save_rc_reg),
						   CLASS_REG_PAIR_reg(ue.save_rc_reg) ));
			}
		break;
	case UE_RESTORE_SP:
	case UE_RESTORE_FP:
#ifdef PROPAGATE_DEBUG
		fprintf(TFile, " restore %s from mem %lld(%s) %lld(entry-%s)",
			UE_Register_Name(CLASS_REG_PAIR_rclass(ue.rc_reg),
					 CLASS_REG_PAIR_reg(ue.rc_reg) ),
			ue.offset, (ue.kind == UE_RESTORE_SP ? "sp" : "fp"),
			ue.top_offset,
			(ue.kind == UE_RESTORE_SP ? "sp" : "fp"));
#else
		fprintf(TFile, " restore %s from mem %s", UE_Register_Name(
			CLASS_REG_PAIR_rclass(ue.rc_reg),
			CLASS_REG_PAIR_reg(ue.rc_reg) ),
			(ue.kind == UE_RESTORE_SP ? "sp" : "fp"));
#endif
		break;
	}

#ifdef PROPAGATE_DEBUG
	switch (ue.kind) {
	case UE_SAVE_GR:  
	case UE_SAVE_SP:
	case UE_SAVE_FP:
	case UE_RESTORE_GR:  
	case UE_RESTORE_SP:
	case UE_RESTORE_FP:
	  if (UE_validated) {
	    fprintf(TFile, " %svalid", (ue.valid == TRUE) ? "" : "in");
	  }
	  break;
	}

	if (ue.handler) {
	  fprintf(TFile, " in exception handler");
	}
#endif
	fprintf(TFile, "\n");
}

void
Print_All_Unwind_Elem (char *msg)
{
  for (ue_iter = ue_list.begin(); ue_iter != ue_list.end(); ++ue_iter) {
    	Print_Unwind_Elem (*ue_iter, msg);
  }
}

BOOL TN_Is_Unwind_Reg (TN *tn)
{
  if (!TN_is_register(tn))
    return FALSE;

  if (REGISTER_SET_MemberP(
	   REGISTER_CLASS_callee_saves(TN_register_class(tn)),
	   TN_register(tn))) {
    return TRUE;
  }
  else if (CLASS_REG_PAIR_EqualP(TN_class_reg(tn), CLASS_REG_PAIR_ra)) {
    return TRUE;
  }
  else if (CLASS_REG_PAIR_EqualP(TN_class_reg(tn), CLASS_REG_PAIR_fp)) {
    return TRUE;
  }
  return FALSE;
}

OP* Find_Def_Of_TN (TN *tn, OP *cur_op, BB *bb)
{
  OP* result = NULL;
  OP* tmp;

  bool check = FALSE;

  // Browse BB bottom-up, skip OPs until we find cur_op. From this
  // point, check the results of previous OPs to see if one defines tn
  FOR_ALL_BB_OPs_REV(bb, tmp) {
    if (check && tmp != cur_op) {
      INT i;
      for (i = 0; i < OP_results(tmp); ++i) {
	TN* opRes = OP_result(tmp, i);
	if (opRes == tn) {
	  result = tmp;
	  break;
	}
      }
    }
    // Until we do not cross op, we do not perform any check, since we are
    // seeking for definition a previous definition
    else if (tmp == cur_op) {
      check = true;
    }
  }
  return result;

#if 0 // [CL] The map-based approach from IA64 is also buggy

	// Initially tried to use TN_Reaching_Value_At_Op,
	// but that misses some cases cause the live-in sets are not
	// kept up-to-date by cgemit time.  Also requires a lot
	// of traversing the bb and op lists.
	// So instead, just create map of tn to defining op,
	// which is a little more space but less time
	// and is more accurate.
	OP *op = (OP*) TN_MAP_Get (tn_def_op, tn);
	if ( op == NULL && REGISTER_SET_MemberP(
			 REGISTER_CLASS_function_value(TN_register_class(tn)),
       		     	 TN_register(tn) ) )
	{
		// okay if store of function return val
		return NULL;
	}
	if (op == NULL && Trace_Unwind)
		fprintf(TFile, "couldn't find def of tn %d\n", TN_number(tn));
	return op;
#endif

}

// search for save-tn that "tn" is copied from.
TN* Get_Copied_Save_TN (TN *tn, OP *cur_op, BB *bb)
{
#ifdef PROPAGATE_DEBUG
    if (TN_Is_Unwind_Reg(tn)) {
#ifdef DEBUG_UNWIND
      fprintf(TFile, "** %s is an unwind reg %d\n", __FUNCTION__,
	      TN_Get_Debug_Reg_Id(tn));
#endif
	return tn;
    }

    // FIXME: ensure that find_def_of_tn returns an op that occurs BEFORE cur_op
    if (TN_is_dedicated(tn)) 
    {
#ifdef DEBUG_UNWIND
      fprintf(TFile, "** %s is dedicated reg %d\n", __FUNCTION__,
	      TN_Get_Debug_Reg_Id(tn));
#endif
	return NULL;
    }
    
	// else find tn that this is a copy of.
    OP *op = Find_Def_Of_TN (tn, cur_op, bb);

#ifdef DEBUG_UNWIND
    if (op) {
      fprintf(TFile, "** %s def of TN %p:\n", __FUNCTION__, op);
      Print_OP_No_SrcLine(op);
    } else {
      fprintf(TFile, "** %s no def of\n", __FUNCTION__);
    }
#endif

    if (!op) return NULL;
    TN *otn = OP_Copy_Operand_TN(op);

#ifdef DEBUG_UNWIND
    if (otn) {
      fprintf(TFile, "** %s copy operand TN %p:\n", __FUNCTION__, otn);
      Print_TN(otn, FALSE);
      fprintf(TFile, "\n");
    } else {
      fprintf(TFile, "** %s no copy operand TN\n", __FUNCTION__);
    }
#endif

    if (!otn) {
	if (OP_move(op))
	    otn = OP_opnd(op,1);
    }
    if (otn && TN_Is_Unwind_Reg(otn)) 
    {
#ifdef DEBUG_UNWIND
      fprintf(TFile, "** %s TN is save reg %d\n", __FUNCTION__,
	      TN_Get_Debug_Reg_Id(otn));
#endif
	return otn;
    }
    else 
	return NULL;


#else
	// might already be a save-tn
    if (TN_is_save_reg(tn)) 
    {
#ifdef DEBUG_UNWIND
      fprintf(TFile, "** %s is a save reg %d\n", __FUNCTION__,
	      TN_Get_Debug_Reg_Id(tn));
#endif
	return tn;
    }

    // [CL] "temporary" fix to handle the "-O1" issue (it happens tn
    // does not always have the TN_is_save_reg() property -- could
    // also happen at any optim level). Check the ABI register
    // property when in the entry block. We should also do the same
    // thing in the exit block, but as this is a temporary fix...
    if (BB_entry(bb) && TN_Is_Unwind_Reg(tn)) {
#ifdef DEBUG_UNWIND
      fprintf(TFile, "** %s is an unwind reg %d\n", __FUNCTION__,
	      TN_Get_Debug_Reg_Id(tn));
#endif
	return tn;
    }

    if (TN_is_dedicated(tn)) 
    {
#ifdef DEBUG_UNWIND
      fprintf(TFile, "** %s is dedicated reg %d\n", __FUNCTION__,
	      TN_Get_Debug_Reg_Id(tn));
#endif
	return NULL;
    }
    
	// else find tn that this is a copy of.
    OP *op = Find_Def_Of_TN (tn);

#ifdef DEBUG_UNWIND
    fprintf(TFile, "** %s def of TN %p:\n", __FUNCTION__, op);
    if (op) Print_OP_No_SrcLine(op);
#endif

    if (!op) return NULL;
    TN *otn = OP_Copy_Operand_TN(op);

#ifdef DEBUG_UNWIND
    fprintf(TFile, "** %s copy operand TN %p:\n", __FUNCTION__, otn);
    if (otn) {
      Print_TN(otn, FALSE);
      fprintf(TFile, "\n");
    }
#endif

    if (!otn) {
	if (OP_move(op))
	    otn = OP_opnd(op,1);
    }
    if (otn && TN_is_register(otn) && TN_is_save_reg(otn)) 
    {
#ifdef DEBUG_UNWIND
      fprintf(TFile, "** %s TN is save reg %d\n", __FUNCTION__,
	      TN_Get_Debug_Reg_Id(otn));
#endif
	return otn;
    }
    else 
	return NULL;
#endif // PROPAGATE_DEBUG
}

void Record_UE(OP* op, UNWIND_ELEM* ue, BB* bb, UINT when)
{
  if (ue->kind != UE_UNDEFINED) {
    ue->qp = 0;
#if 0
    if (OP_has_predicate(op)) {
      ue.qp = REGISTER_Get_Debug_Reg_Id (ISA_REGISTER_CLASS_predicate,
			TN_register(OP_opnd(op, OP_PREDICATE_OPND)) );
    }
#endif
#if 0
    if (ue.kind == UE_SAVE_GR) {
      ISA_REGISTER_CLASS rc = CLASS_REG_PAIR_rclass(ue.save_rc_reg);
      REGISTER reg = CLASS_REG_PAIR_reg(ue.save_rc_reg);
      if (ABI_PROPERTY_Is_stacked(rc, REGISTER_machine_id(rc, reg))
	  && ! BB_rotating_kernel(bb)
	  && REGISTER_Is_Stacked_Output(rc, reg) )
	{
	  reg = REGISTER_Translate_Stacked_Output(reg);
	  Set_CLASS_REG_PAIR_reg(ue.save_rc_reg, reg);
	}
    }
#endif
    ue->when = when;
    ue->bb = bb;
    ue_list.push_back (*ue);
    if (Trace_Unwind) Print_Unwind_Elem (*ue, "unwind1");
  }
}

static UINT
Get_BB_When_Length (BB *bb) 
{
  UINT w = 0;
  OP *op;
  FOR_ALL_BB_OPs_FWD(bb, op) {
  	if (OP_dummy(op)) continue;
  	w += OP_Real_Inst_Words(op);
  }
  return w;
}

static void
Find_Unwind_Info (void)
{
  OP *op;
  UINT when = 0;
  BB_Mark_Unreachable_Blocks ();

  // Create mapping of TN to defining OP.
  // This is used to find spill-tns, 
  // cause add and store may be in separate blocks.
  // TN_Reaching_Value_At_Op() doesn't catch all cases
  // (partly cause live-in/out sets not up-to-date),
  // so instead we do brute-force mapping.
  tn_def_op = TN_MAP_Create();
  for (BB *bb = REGION_First_BB; bb; bb = BB_next(bb)) {
#ifndef USE_UNREACHABLE
    if (BB_unreachable(bb)) continue;
#endif
    FOR_ALL_BB_OPs_FWD(bb, op) {
	// if multiple stores, will take last one
	if (OP_has_result(op))
		TN_MAP_Set (tn_def_op, OP_result(op,0), op);
    }
  }

  for (BB *bb = REGION_First_BB; bb; bb = BB_next(bb)) {
#ifndef USE_UNREACHABLE
    if (BB_unreachable(bb)) {
	when += Get_BB_When_Length(bb); 
	continue;
    }
#endif
    if (Trace_Unwind) Print_BB(bb);
    FOR_ALL_BB_OPs_FWD(bb, op) {
	if (OP_dummy(op)) continue;
	Analyze_OP_For_Unwind_Info(op, when, bb);
	when += OP_Real_Inst_Words(op);
    }
    // if the current BB has restored FP and is an exit BB then the
    // next BB is entered from a path where FP was not restored
    if (PU_has_FP && PU_has_restored_FP && BB_exit(bb)) {
      PU_has_restored_FP = FALSE;
    }
  }
  last_when = when - 1;
  TN_MAP_Delete (tn_def_op);
}



#define INCR(p)	(p = static_cast<PR_TYPE>(static_cast<INT>(p) +1))

typedef UINT64 PR_BITSET;	// bit mask for PR enumeration
static inline BOOL Get_PR (PR_BITSET state, PR_TYPE p)
{
	return (BOOL) ((state >> p) & 1);
}
static inline BOOL Get_PR (PR_BITSET *state, BB *bb, PR_TYPE p)
{
	return (BOOL) ((state[bb->id] >> p) & 1);
}
static inline PR_BITSET Set_PR (PR_BITSET state, PR_TYPE p)
{
	return (state | (1LL << p));
}
static inline void Set_PR (PR_BITSET *state, BB *bb, PR_TYPE p)
{
	state[bb->id] |= (1LL << p);
}
static inline PR_BITSET Clear_PR (PR_BITSET state, PR_TYPE p)
{
	return (state & (~(1LL << p)));
}

#ifdef PROPAGATE_DEBUG
static INT64 current_frame_size = 0;

static BOOL is_handler = FALSE;

static void Tag_Irrelevant_Saves_And_Restores_For_BB(BB* bb,
				     list < UNWIND_ELEM > pr_stack[PR_LAST],
						     BOOL* visited,
						     INT64* frame_size_bb)
{
  // Record the UEs pushed/popped here (in this bb), in order to
  // restore pr_stack for recursion
  list < UNWIND_ELEM > pushed_popped_here;
  PR_TYPE p;

  if (Trace_Unwind) {
    fprintf(TFile, "Recursing to BB %d current frame size %d\n", BB_id(bb),
current_frame_size);
  }

  pushed_popped_here.clear();

  // if we've already visited this BB, stop recursion
  if (visited[BB_id(bb)]) {
    return;
  }

  // mark as visited
  visited[BB_id(bb)] = TRUE;

  for (ue_iter = ue_list.begin(); ue_iter != ue_list.end(); ++ue_iter) {
    if (ue_iter->bb != bb) continue;

    // If we have already seen this UE, no need to continue
    if (ue_iter->valid == FALSE)
      break;

    switch (ue_iter->kind) {
    case UE_CREATE_FRAME:
      current_frame_size += ue_iter->offset;
      if (Trace_Unwind) {
	Print_Unwind_Elem (*ue_iter, "frame_size");
      }
      break;

    case UE_DESTROY_FRAME:
      current_frame_size -= ue_iter->offset;
      if (Trace_Unwind) {
	Print_Unwind_Elem (*ue_iter, "frame_size");
      }
      break;

    case UE_CREATE_FP:
    case UE_DESTROY_FP:
      break;

    case UE_SAVE_GR:
    case UE_SAVE_SP:
    case UE_SAVE_FP:
      p = CR_To_PR (ue_iter->rc_reg);
      if (current_frame_size && ue_iter->kind == UE_SAVE_SP) {
	ue_iter->top_offset = ue_iter->offset - current_frame_size;
      } else {
	ue_iter->top_offset = ue_iter->offset;
      }

      if ( ! pr_stack[p].empty() ) {
	// Only the 1st one is valid, so if pr_track[p] is not empty,
	// this ue is invalid
	ue_iter->valid = FALSE;
	if (Trace_Unwind) {
	  Print_Unwind_Elem (*ue_iter, "invalid");
	}
      } else {
	if (Trace_Unwind) {
	  Print_Unwind_Elem (*ue_iter, "valid");
	}
      }
      pr_stack[p].push_front (*ue_iter);
      ue_iter->propagated = UE_PROP_PUSHED;
      pushed_popped_here.push_front(*ue_iter);
      break;


    case UE_RESTORE_GR:
    case UE_RESTORE_SP:
    case UE_RESTORE_FP:
      p = CR_To_PR (ue_iter->rc_reg);
      // FIXME if p was saved with SAVE_FP, maybe we should not use
      // current_frame_size. But so far there is no distinction
      // between a restore from SP and from FP
      if (current_frame_size && ue_iter->kind == UE_RESTORE_SP) {
	ue_iter->top_offset = ue_iter->offset - current_frame_size;
      } else {
	ue_iter->top_offset = ue_iter->offset;
      }

      if (!is_handler || !pr_stack[p].empty()) {
          list<UNWIND_ELEM>::iterator it = pr_stack[p].begin(), itPair;
          ue_iter->valid = FALSE;
          
          for(; !ue_iter->valid && it != pr_stack[p].end(); ++it) {
              // If current element is invalid, go to the next
              if(!it->valid) {
                  continue;
              }
              // this restore is not on the path of an exception handler, OR
              // if on such a path, the corresponding store is likely to
              // have already been processed (pr_strack[p] is not empty
              
              // check that this restore corresponds to the previous store. If
              // it does not, then ignore it (it is not related to callee-save
              // handling, it may be a definition of 'p')
              if (ue_iter->kind == UE_RESTORE_FP ||
                  ue_iter->kind == UE_RESTORE_SP) {
                  // Check that restore from memory corresponds to a save
                  // relative to SP or FP, and at the same offset
                  if ( ( (it->kind == UE_SAVE_SP)
                         || (it->kind == UE_SAVE_FP) )
                       && ue_iter->top_offset == it->top_offset ) {
                      ue_iter->valid = TRUE;
                      itPair = it;
                  }
              } else if (ue_iter->kind == UE_RESTORE_GR) {
                  // Check that restore from a register corresponds to a save
                  // to the same register
                  if (it->kind == UE_SAVE_GR
                      && CLASS_REG_PAIR_EqualP(ue_iter->save_rc_reg,
                                               it->save_rc_reg)) {
                      ue_iter->valid = TRUE;
                      itPair = it;
                  }
              }
          }
          if(!ue_iter->valid) {
              if (Trace_Unwind) {
                  Print_Unwind_Elem (*ue_iter, "invalid");
              }
          } else {
              itPair->propagated = UE_PROP_POPPED;
              pushed_popped_here.push_front(*itPair);
              pr_stack[p].erase(itPair);
              if (Trace_Unwind) {
                  Print_Unwind_Elem (*ue_iter, "valid");
              }
          }
      } else if (is_handler) {
	// this restore in of the path of an exception handler and
	// pr_stack[p] is empty: this means that we are probably
	// handling a restore which corresponds to a store made during
	// the function prologue, which is not on this path.

	// FIXME: accept it for now. Maybe we could check we are in an
	// exit_bb (implying that exception handlers propably disable
	// some optimizations
	if (Trace_Unwind) {
	  Print_Unwind_Elem (*ue_iter, "valid");
	}

      } else {
	FmtAssert(!pr_stack[p].empty(),
		  ("unable to match restoring UNWIND_ELEM\n"));
      }
      
      break;
 
    default:
      break;
    }

    ue_iter->frame_size = current_frame_size;

    ue_iter->handler = is_handler ? TRUE : FALSE;
    if (Trace_Unwind && is_handler) {
      Print_Unwind_Elem (*ue_iter, "is_handler");
    }
  }

  frame_size_bb[BB_id(bb)] = current_frame_size;

  // Recursively analyze all the successors (except oneself)
  BBLIST *blst;
  INT bbid;
  FOR_ALL_BB_SUCCS (bb, blst) {
    bbid = BB_id(BBLIST_item(blst));
    INT64 saved_frame_size = current_frame_size;

    if (bbid != BB_id(bb)) {
      Tag_Irrelevant_Saves_And_Restores_For_BB(BBLIST_item(blst), pr_stack,
					       visited, frame_size_bb);
    }
    current_frame_size = saved_frame_size;
  }

  if (Trace_Unwind && ! pushed_popped_here.empty()) {
    fprintf(TFile, "back in BB %d after recursion: restoring state\n",
	    BB_id(bb));
  }

  // Restore the previous state to resume recursion
  while (! pushed_popped_here.empty()){
    UNWIND_ELEM ue;
    ue = pushed_popped_here.front();

    p = CR_To_PR (ue.rc_reg);
    if (ue.propagated == UE_PROP_PUSHED) {
      pr_stack[p].pop_front();
      if (Trace_Unwind) {
	Print_Unwind_Elem (ue, "pop");
      }
    } else if (ue.propagated == UE_PROP_POPPED) {
      pr_stack[p].push_front(ue);
      if (Trace_Unwind) {
	Print_Unwind_Elem (ue, "push");
      }
    }
    else {
      FmtAssert(FALSE, ("UE neither pushed nor popped\n"));
    }

    pushed_popped_here.pop_front();
  }
}


static void Tag_Irrelevant_Saves_And_Restores_BB_List(BB_LIST *elist,
						      BOOL* visited,
						      INT64* frame_size_bb)
{
  // Depth first search of the CFG in order to track the 'irrelevant'
  // stores and restores.

  // Do it recursively because the head of the list contains the last
  // block: in presence of exception handlers, the handlers are first
  // in the list, the function entry point is the last. This leads to
  // cases where the EH branches to the function epilogue, which is
  // studied before the function prologue; thus it cannot match
  // restores.

  // With this recursion, the EH are handled after the function
  // prologue.
  if (BB_LIST_rest(elist)) {
    Tag_Irrelevant_Saves_And_Restores_BB_List(BB_LIST_rest(elist),
					      visited, frame_size_bb);
  }

  // Use a stack to record the state of each PR along the CFG
  list < UNWIND_ELEM > pr_stack[PR_LAST];
  for (PR_TYPE p = PR_FIRST; p < PR_LAST; INCR(p)) {
    pr_stack[p].clear();
  }

  BB* bb = BB_LIST_first(elist);

  current_frame_size = 0;

  if (BB_handler(bb)) {
    is_handler = TRUE;
  } else {
    is_handler = FALSE;
  }
  Tag_Irrelevant_Saves_And_Restores_For_BB(bb, pr_stack, visited,
					   frame_size_bb);
}

static void Tag_Irrelevant_Saves_And_Restores_TOP(INT64* frame_size_bb)
{
  BB_LIST *elist;
  BOOL* visited;

  visited = (BOOL *)alloca(sizeof(BOOL)*(PU_BB_Count+1));
  BZERO(visited, sizeof(BOOL)*(PU_BB_Count+1));

  Tag_Irrelevant_Saves_And_Restores_BB_List(Entry_BB_Head, visited, frame_size_bb);
}
#endif

// iterate thru list and mark all saves/restores in local state
static void
Mark_Local_Saves_Restores (PR_BITSET *local_save_state, 
	PR_BITSET *local_restore_state)
{
  PR_TYPE p;
  memset (local_save_state, 0, ((PU_BB_Count+1) * sizeof(PR_BITSET)));
  memset (local_restore_state, 0, ((PU_BB_Count+1) * sizeof(PR_BITSET)));

#ifdef PROPAGATE_DEBUG
  // remove the invalid UEs from the list
  for (ue_iter = ue_list.begin(); ue_iter != ue_list.end(); ++ue_iter) {
    while (!ue_iter->valid) {
      if (Trace_Unwind) {
	Print_Unwind_Elem(*ue_iter, "erasing");
      }
      ue_iter = ue_list.erase(ue_iter);
      if (ue_iter == ue_list.end()) {
	break;
      }
    }
  }
#endif
  if (Trace_Unwind) {
    Print_All_Unwind_Elem ("after cleanup");
  }
  for (ue_iter = ue_list.begin(); ue_iter != ue_list.end(); ++ue_iter) {
	switch (ue_iter->kind) {
	case UE_CREATE_FRAME:
	case UE_CREATE_FP:
	case UE_SAVE_GR:
	case UE_SAVE_SP:
	case UE_SAVE_FP:
  		p = CR_To_PR (ue_iter->rc_reg);
  		Set_PR(local_save_state, ue_iter->bb, p);
		break;
	case UE_DESTROY_FRAME:
	case UE_DESTROY_FP:
	case UE_RESTORE_GR:
	case UE_RESTORE_SP:
	case UE_RESTORE_FP:
  		p = CR_To_PR (ue_iter->rc_reg);
  		Set_PR(local_restore_state, ue_iter->bb, p);
		break;
	}
  }
}

static void
Propagate_Save_Restore_State (PR_BITSET *entry_state,
	PR_BITSET *local_save_state,
	PR_BITSET *local_restore_state,
	PR_BITSET *exit_state
#ifdef PROPAGATE_DEBUG
       ,UNWIND_ELEM* bb_entry_ue[][PR_LAST],
	UNWIND_ELEM* bb_exit_ue[][PR_LAST],
	list < UNWIND_ELEM > pr_last_info[PR_LAST]
#endif
)
{
  BB *bb;
  BBLIST *blst;
  INT bbid;
  BOOL changed = TRUE;
  INT count = 0;

#ifdef PROPAGATE_DEBUG
  PR_TYPE p;
#endif

  memset (entry_state, 0, ((PU_BB_Count+1) * sizeof(PR_BITSET)));
  memset (exit_state, -1, ((PU_BB_Count+1) * sizeof(PR_BITSET)));

  while (changed) {
	++count;
	if (count > 100) {	// to avoid infinite loops
		DevWarn("infinite loop in propagating unwind info");
		break;
	}
	changed = FALSE;
	for (bb = REGION_First_BB; bb; bb = BB_next(bb)) {
#ifndef USE_UNREACHABLE
	        if (BB_unreachable(bb)) continue;
#endif
		if (Trace_Unwind) 
			fprintf (TFile, "curbb: %d, preds: ", BB_id(bb));
		if (BB_preds(bb) != NULL) {
			PR_BITSET new_entry_state = (PR_BITSET) -1; // all 1's
#ifdef PROPAGATE_DEBUG
			UNWIND_ELEM* new_entry_ue[PR_LAST];
			memset(new_entry_ue, 0, PR_LAST * sizeof(UNWIND_ELEM*));
#endif
			FOR_ALL_BB_PREDS (bb, blst) {
				bbid = BB_id(BBLIST_item(blst));
				new_entry_state &= exit_state[bbid];
				if (Trace_Unwind) fprintf (TFile, "[%d %llx], ",
					bbid, exit_state[bbid]);
#ifdef PROPAGATE_DEBUG
				// Do the equivalent of the &= line
				// above, for UEs
				for (p = PR_FIRST; p < PR_LAST; INCR(p)) {
				  // if 'p' has an exit state in bbid
				  // and no entry state in bb, then
				  // the entry state in bb is the exit
				  // state of bbid
				  if (bb_exit_ue[bbid][p]) {
				    if (bb_entry_ue[BB_id(bb)][p] == NULL) {
				      bb_entry_ue[BB_id(bb)][p] =
					bb_exit_ue[bbid][p];
				      if (Trace_Unwind) 
					Print_Unwind_Elem(*bb_exit_ue[bbid][p], "entry_state");
				      // if there is already an entry
				      // state in bb, it should match
				      // the exit state of bbid
				    } else if (bb_entry_ue[BB_id(bb)][p]
					       != bb_exit_ue[bbid][p]) {
				      if (Trace_Unwind) {
					fprintf(TFile, "**UE do not match:\n");
					Print_Unwind_Elem(*bb_entry_ue[BB_id(bb)][p], "");
					Print_Unwind_Elem(*bb_exit_ue[bbid][p], "");
				      }
				    }
				  }
				}
#endif
			}
        		entry_state[BB_id(bb)] = new_entry_state;
		}
		bbid = BB_id(bb);
		if (Trace_Unwind) 
			fprintf (TFile, "\n entry_state: %llx\n", entry_state[bbid]);

		// exit state bit is 1 if entry or local_save is 1
		// and local restore is 0.
		PR_BITSET new_exit_state = 
			entry_state[bbid] | local_save_state[bbid];
		new_exit_state &= ~local_restore_state[bbid];
		if (new_exit_state != exit_state[bbid]) {
			changed = TRUE;
			if (Trace_Unwind) 
			  fprintf(TFile, "changed from %llx to %llx\n", exit_state[bbid], new_exit_state);
			exit_state[bbid] = new_exit_state;
#ifdef PROPAGATE_DEBUG
			// do the equivalent of the above, for UEs
			for (p = PR_FIRST; p < PR_LAST; INCR(p)) {

			  // look for the UE that corresponds to bb
			  list < UNWIND_ELEM >::iterator ue_for_p_iter;
			  for(ue_for_p_iter = pr_last_info[p].begin();
			      ue_for_p_iter != pr_last_info[p].end();
			      ue_for_p_iter++) {
			    if (ue_for_p_iter->bb == bb) break;
			  }
			  if (Get_PR(local_save_state[bbid],p)
			      || Get_PR(local_restore_state[bbid],p)) {
			    if (ue_for_p_iter == pr_last_info[p].end()) {
			      FmtAssert(FALSE, ("no UE found -- inconsistent!\n"));
			    }
			    bb_exit_ue[bbid][p] = &(*ue_for_p_iter);
			  } else {
			    bb_exit_ue[bbid][p] = bb_entry_ue[bbid][p];
			  }
			  if (bb_exit_ue[bbid][p]) {
			    if (Trace_Unwind) {
			      Print_Unwind_Elem(*bb_exit_ue[bbid][p], "exit_state");
			    }
			  }
			}
#endif
		}
		if (Trace_Unwind) 
		  fprintf (TFile, " exit_state: %llx\n", exit_state[bbid]);

	}
  }
  if (Trace_Unwind) {
	fprintf(TFile, "bb\tentry\t\tlocal_save\tlocal_restore\texit:\n");
	for (INT i = 1; i <= PU_BB_Count; ++i) {
		fprintf(TFile, "%4d:\t%12llx\t%12llx\t%12llx\t%12llx\n", 
			i, entry_state[i], local_save_state[i], local_restore_state[i], exit_state[i]);
	}
  }
}

static UNWIND_ELEM
Find_Prev_Save_UE_For_BB (list < UNWIND_ELEM > prev_ue, BB *bb, UINT level)
{
  BBLIST *blst;
  list < UNWIND_ELEM >::iterator prev_iter;
  FOR_ALL_BB_PREDS (bb, blst) {
	// find ue in nbb that does a save
  	for (prev_iter = prev_ue.begin(); prev_iter != prev_ue.end(); ++prev_iter) {
		if (prev_iter->bb != BBLIST_item(blst)) continue;
		if (prev_iter->kind == UE_SAVE_GR
		    || prev_iter->kind == UE_SAVE_SP
		    || prev_iter->kind == UE_SAVE_FP) {
			return *prev_iter;
		}
	}
  }
  UNWIND_ELEM ue;
  ue.kind = UE_UNDEFINED;
  ++level;
  if (level > 10) return ue;
  // if not found recurse
  FOR_ALL_BB_PREDS (bb, blst) {
	ue = Find_Prev_Save_UE_For_BB (prev_ue, BBLIST_item(blst), level);
	if (ue.kind != UE_UNDEFINED) return ue;
  }
  return ue;
}

#ifdef PROPAGATE_DEBUG
// Prepend a copy of *pue at current ue_iter position.
// This used in presence of COPY_STATE elements
// The copied UE has updated bb and when fields
static void Prepend_UE(UNWIND_ELEM* pue, UINT when, BB *bb)
{
  UNWIND_ELEM ue;
  ue.when = when;
  ue.bb = bb;
  ue.kind = pue->kind;
  ue.qp = pue->qp;
  ue.rc_reg = pue->rc_reg;
  ue.save_rc_reg = pue->save_rc_reg;
  ue.offset = pue->offset;
  ue.valid = TRUE;
  ue.top_offset = pue->top_offset;
  ue.is_copy = TRUE;
  ue.label_idx = LABEL_IDX_ZERO;
  ue.frame_size = pue->frame_size;
  ue.handler = ue_iter->handler;

  ue_list.insert(ue_iter, ue);
  if (Trace_Unwind) {
	fprintf(TFile, "prepend ue kind %d at bb %d when %d\n",
		ue.kind, BB_id(bb), when);
  }
}
#endif

// overload some routines to add unwind elements
static void
#ifdef PROPAGATE_DEBUG
Add_UE (list < UNWIND_ELEM > prev_ue, PR_TYPE p, UINT when, BB *bb, INT64 frame_size, BOOL is_handler)
#else
Add_UE (list < UNWIND_ELEM > prev_ue, PR_TYPE p, UINT when, BB *bb)
#endif
{
  list < UNWIND_ELEM >::iterator prev_iter;
  UNWIND_ELEM ue;
  ue.kind = UE_UNDEFINED;
#ifdef PROPAGATE_DEBUG
  ue.valid = TRUE;
  ue.top_offset = 0;
  ue.is_copy = FALSE;
  ue.frame_size = frame_size;
  ue.handler = is_handler;
#endif
  ue.label_idx = LABEL_IDX_ZERO;
  ue.save_rc_reg =  CLASS_REG_PAIR_undef;
  UINT num_found = 0;
  for (prev_iter = prev_ue.begin(); prev_iter != prev_ue.end(); ++prev_iter) {
	// look for save
	if (prev_iter->kind == UE_SAVE_GR
	    || prev_iter->kind == UE_SAVE_SP
	    || prev_iter->kind == UE_SAVE_FP) {
		ue = *prev_iter;
		++num_found;
	}
  }
  if (num_found == 0) {
	DevWarn("unwind: no pr_info found for %d", p);
	return;
  }
  if (num_found > 1) {
	// check if all are same
  	for (prev_iter = prev_ue.begin(); prev_iter != prev_ue.end(); ++prev_iter) {
		if (prev_iter->kind == ue.kind 
		  && CLASS_REG_PAIR_EqualP(prev_iter->rc_reg, ue.rc_reg))
			--num_found;
	}
	++num_found;	// original still counts
  }
  if (num_found > 1) {
	UNWIND_ELEM nue;
	nue = Find_Prev_Save_UE_For_BB (prev_ue, bb, 0);
	if (nue.kind == UE_UNDEFINED) {
		// just use memory save if exists
  		for (prev_iter = prev_ue.begin(); prev_iter != prev_ue.end(); ++prev_iter) {
			if (prev_iter->kind == UE_SAVE_SP
			    || prev_iter->kind == UE_SAVE_FP) {
				nue = *prev_iter;
				break;
			}
		}
		if (nue.kind == UE_UNDEFINED) {
			DevWarn("couldn't find unwind save for %d", p);
		}
		else
			ue = nue;
	}
	else
		ue = nue;
  }
  ue.when = when;
  ue.bb = bb;
  ue_list.insert(ue_iter, ue);
  if (Trace_Unwind) {
    CLASS_REG_PAIR rc_reg = PR_To_CR(p);
	fprintf(TFile, "state change for %s at entry to bb %d when %d\n",
		UE_Register_Name(CLASS_REG_PAIR_rclass(rc_reg),
				 CLASS_REG_PAIR_reg(rc_reg) ),
		 BB_id(bb), when);
  }
}

static void
#ifdef PROPAGATE_DEBUG
Add_UE (INT8 kind, PR_TYPE p, UINT when, BB *bb, INT64 frame_size, BOOL is_handler)
#else
Add_UE (INT8 kind, PR_TYPE p, UINT when, BB *bb)
#endif
{
  UNWIND_ELEM ue;
#ifdef PROPAGATE_DEBUG
  ue.valid = TRUE;
  ue.top_offset = 0;
  ue.is_copy = FALSE;
  ue.frame_size = frame_size;
  ue.handler = is_handler;
#endif
  ue.label_idx = LABEL_IDX_ZERO;
  ue.save_rc_reg =  CLASS_REG_PAIR_undef;
  ue.kind = kind;
  ue.qp = 0;
  ue.rc_reg = PR_To_CR(p);
  ue.when = when;
  ue.bb = bb;
  ue_list.insert(ue_iter, ue);
  if (Trace_Unwind) 
	fprintf(TFile, "state change for %s at entry to bb %d when %d\n",
		UE_Register_Name(CLASS_REG_PAIR_rclass(ue.rc_reg),
				 CLASS_REG_PAIR_reg(ue.rc_reg) ),
		BB_id(bb), when);
}

static void
#ifdef PROPAGATE_DEBUG
Add_UE (INT8 kind, UINT label, UINT when, BB *bb, INT64 frame_size, BOOL is_handler)
#else
Add_UE (INT8 kind, UINT label, UINT when, BB *bb)
#endif
{
  UNWIND_ELEM ue;
#ifdef PROPAGATE_DEBUG
  ue.valid = TRUE;
  ue.top_offset = 0;
  ue.is_copy = FALSE;
  ue.frame_size = frame_size;
  ue.handler = is_handler;
#endif
  ue.label_idx = LABEL_IDX_ZERO;
  ue.save_rc_reg =  CLASS_REG_PAIR_undef;
  ue.kind = kind;
  ue.label = label;
  ue.when = when;
  ue.bb = bb;
  ue_list.insert(ue_iter, ue);
  if (Trace_Unwind) 
	fprintf(TFile, "add ue kind %d label %d at bb %d when %d\n",
		kind, label, BB_id(bb), when);
}

static void
Do_Control_Flow_Analysis_Of_Unwind_Info (void)
{
  // we know what unwind changes happen in each block;
  // now have to propagate that info so each bb has correct info upon entry.

  // have 4 bit vectors for each bb:  
  // entry, local-save, local-restore, and exit state.
  // first fill in the local state info with changes that happen in that bb.

  PR_BITSET entry_state[PU_BB_Count+1];
  PR_BITSET local_save_state[PU_BB_Count+1];
  PR_BITSET local_restore_state[PU_BB_Count+1];
  PR_BITSET exit_state[PU_BB_Count+1];

#ifdef PROPAGATE_DEBUG
  INT64 frame_size_bb[PU_BB_Count+1];
  // [CL] Incorrect saves/restores may have been recorded by
  // Find_Unwind_Info(). Indeed, it has recorded all saves/restores
  // to/from callee-saved registers, some of which may not be related
  // to this ABI property.
  Tag_Irrelevant_Saves_And_Restores_TOP(frame_size_bb);
  UE_validated = TRUE;
  BOOL is_handler = FALSE;
#endif

  // mark all saves/restores in local state
  Mark_Local_Saves_Restores (local_save_state, local_restore_state);

#ifdef PROPAGATE_DEBUG
  PR_TYPE p;
  // keep list of ue's for each pr.
  list < UNWIND_ELEM > pr_last_info[PR_LAST];
  for (ue_iter = ue_list.begin(); ue_iter != ue_list.end(); ++ue_iter) {
		p = CR_To_PR (ue_iter->rc_reg);
		// put last ue for bb on list
		if ( ! pr_last_info[p].empty()
		    && pr_last_info[p].front().bb == ue_iter->bb)
		{
			pr_last_info[p].pop_front();
		}
		pr_last_info[p].push_front (*ue_iter);
  }

  // for each BB, keep ue for each pr on BB's exit
  UNWIND_ELEM* bb_exit_ue[PU_BB_Count+1][PR_LAST];
  UNWIND_ELEM* bb_entry_ue[PU_BB_Count+1][PR_LAST];

  memset(bb_exit_ue,  0, ((PU_BB_Count+1) * PR_LAST * sizeof(UNWIND_ELEM*)));
  memset(bb_entry_ue, 0, ((PU_BB_Count+1) * PR_LAST * sizeof(UNWIND_ELEM*)));
#endif

  // now propagate the save/restore state thru the control flow.
  // foreach pred bb, copy its exit-state to the entry-state.
#ifndef PROPAGATE_DEBUG
  Propagate_Save_Restore_State (entry_state, local_save_state,
				local_restore_state, exit_state);
#else
  Propagate_Save_Restore_State (entry_state, local_save_state,
				local_restore_state, exit_state,
				bb_entry_ue, bb_exit_ue, pr_last_info);
#endif

#ifndef PROPAGATE_DEBUG // moved above
  PR_TYPE p;
  // keep list of ue's for each pr.
  list < UNWIND_ELEM > pr_last_info[PR_LAST];
  for (ue_iter = ue_list.begin(); ue_iter != ue_list.end(); ++ue_iter) {
		p = CR_To_PR (ue_iter->rc_reg);
		// put last ue for bb on list
		if ( ! pr_last_info[p].empty()
		    && pr_last_info[p].front().bb == ue_iter->bb)
		{
			pr_last_info[p].pop_front();
		}
		pr_last_info[p].push_front (*ue_iter);
  }
#endif

  // now determine save/restore changes at each when point
  // and update ue_list with changes
  PR_BITSET current_state = 0;
  INT bbid;
  UINT lwhen = 0;
  ue_iter = ue_list.begin();
  for (BB *bb = REGION_First_BB; bb; bb = BB_next(bb)) {
#ifdef PROPAGATE_DEBUG
    INT64 last_frame_size = 0;
#endif

#ifndef USE_UNREACHABLE
	if (BB_unreachable(bb)) {
		lwhen += Get_BB_When_Length(bb); 
		continue;
	}
#endif
	if (BB_length(bb) == 0) {
		// empty, so ignore
		continue;
	}
	bbid = BB_id(bb);

	// if an alternate entry point,
	// reset current state so no implicit changes at entry.
	if (BB_entry(bb)) {
		current_state = entry_state[bbid];
	}
	// in case have exit that follows exit,
	// first copy previous label then do new label.
	if (BB_prev(bb) != NULL && BB_exit(BB_prev(bb))) {
		// in bb that follows exit, so copy above label
#ifdef PROPAGATE_DEBUG
		Add_UE (UE_COPY, last_label, lwhen, bb, last_frame_size, is_handler);
#else
		Add_UE (UE_COPY, last_label, lwhen, bb);
#endif
		current_state = entry_state[BB_id(BB_prev(bb))];
#ifdef PROPAGATE_DEBUG
		// in addition to the (fake for us) COPY, explicitly
		// add all the UEs required to restore the desired
		// state

		// FIXME: for an EH BB, the state of the pred is 0.
		// TODO: propagate bottom-up the entry states
		INT pred_id;
		if (BB_First_Pred(bb)) {
		  pred_id = BB_id(BB_First_Pred(bb));
		} else {
		  FmtAssert(BB_handler(bb), ("BB %d has no predecessor and is not EH", BB_id(bb)));
		  pred_id = BB_id(BB_prev(bb));
		}
		if (Trace_Unwind) 
		  fprintf(TFile, "copying entry state from bb %d\n", pred_id);

  		for (p = PR_FIRST; p < PR_LAST; INCR(p)) {
		  if (bb_entry_ue[pred_id][p]) {
		    Prepend_UE(bb_entry_ue[pred_id][p], lwhen, bb);
		  }
		}
#endif
	}
	if (BB_exit(bb) && BB_next(bb) != NULL) {
		// if have an exit that is followed by another bb
		// then want to create body label before exit and
		// copy from label after exit (i.e. skip destroy frame)
		if (BB_entry(bb) && BB_prev(bb) == NULL) {
			// if in first bb, then make sure label
			// comes after create_frame.
			while (ue_iter != ue_list.end() && ue_iter->bb == bb) {
				if (ue_iter->kind == UE_CREATE_FRAME) {
				        last_frame_size = ue_iter->frame_size;
					++ue_iter;
					Add_UE (UE_LABEL, ++last_label, 
#ifdef PROPAGATE_DEBUG
						ue_iter->when, bb, last_frame_size,
						is_handler);
#else
						ue_iter->when, bb);
#endif
					break;
				}
				++ue_iter;
			}
		}
		else {
#ifdef PROPAGATE_DEBUG
		        last_frame_size = ue_iter->frame_size;
			Add_UE (UE_LABEL, ++last_label, lwhen, bb, last_frame_size,
				is_handler);
#else
			Add_UE (UE_LABEL, ++last_label, lwhen, bb);
#endif
		}
	}

	if (Trace_Unwind) 
	  fprintf (TFile, "current_state for bb %d: %llx\n", bbid,
		   current_state);

	// add implicit changes upon entry
#ifdef PROPAGATE_DEBUG
	if (BB_handler(bb)) {
	  // FIXME If bb is an EH entry block, keep the just-computed
	  // entry_state. Indeed, the proper entry state of such a BB
	  // has not been computed accurately (and is zero currently,
	  // as we still need a bottom-up propagation phase) //
	  // entry_state[bbid] = current_state; this is wrong because
	  // it should also be propagated to the following BBs
	  current_state = entry_state[bbid];

	  is_handler = TRUE;
	} else if (BB_entry(bb)) {
	  is_handler = FALSE;
	}
#ifndef USE_UNREACHABLE
	else if (BB_unreachable(bb)) {
	  // FIXME
	  current_state = entry_state[bbid];
	}
#endif
#endif
	if (current_state != entry_state[bbid]) {
  		for (p = PR_FIRST; p < PR_LAST; INCR(p)) {
			// ignore implicit sp changes,
			// as label/copy should handle those.
			if (p == PR_SP) continue;
			if (Get_PR(current_state,p) != Get_PR(entry_state[bbid],p)) 
			{
				// add into ue_list
				if (Get_PR(entry_state[bbid],p) == FALSE) {
					// add restore
#ifdef PROPAGATE_DEBUG
					Add_UE (UE_RESTORE_GR, p, lwhen, bb,
						frame_size_bb[BB_id(BBLIST_item(BB_preds(bb)))],
						ue_iter->handler);
#else
					Add_UE (UE_RESTORE_GR, p, lwhen, bb);
#endif
				}
				else {
					// add save
#ifdef PROPAGATE_DEBUG
					Add_UE (pr_last_info[p], p, lwhen, bb,
						frame_size_bb[BB_id(BBLIST_item(BB_preds(bb)))],
						ue_iter->handler);
#else
					Add_UE (pr_last_info[p], p, lwhen, bb);
#endif
				}
			}
  		}
		current_state = entry_state[bbid];
	}

	// look for changes in bb
	while (ue_iter != ue_list.end() && ue_iter->bb == bb) {
		p = CR_To_PR (ue_iter->rc_reg);
		if (Trace_Unwind) fprintf(TFile, 
			"state change for %s in bb %d\n",
					  UE_Register_Name(CLASS_REG_PAIR_rclass(ue_iter->rc_reg),
							   CLASS_REG_PAIR_reg(ue_iter->rc_reg) ),
					  bbid);
		if (ue_iter->kind == UE_RESTORE_GR
		 || ue_iter->kind == UE_RESTORE_SP
		 || ue_iter->kind == UE_RESTORE_FP)
		{
			current_state = Clear_PR(current_state, p);
		}
		else {
			current_state = Set_PR(current_state, p);
		}
		++ue_iter;
	}
	lwhen += Get_BB_When_Length(bb);

	if (Trace_Unwind) 
	  fprintf (TFile, "final current_state for bb %d: %llx\n", bbid,
		   current_state);

  }
}

static void
Insert_Epilogs (void)
{
  list < UNWIND_ELEM >::iterator prev_ue;
  UNWIND_ELEM ue;
  for (ue_iter = ue_list.begin(); ue_iter != ue_list.end(); ++ue_iter) {
    switch (ue_iter->kind) {
    case UE_DESTROY_FRAME:
	// go backwards, until first restore in exit bb
	prev_ue = ue_iter;
	--prev_ue;
	while (prev_ue != ue_list.begin()) {
		if (prev_ue->bb != ue_iter->bb) break;
		if (prev_ue->kind != UE_RESTORE_GR && prev_ue->kind != UE_RESTORE_SP && prev_ue->kind != UE_RESTORE_FP)
			break;
		--prev_ue;
	}
	++prev_ue;
  	ue.kind = UE_EPILOG;
  	ue.when = prev_ue->when;
  	ue.bb = prev_ue->bb;
#ifdef PROPAGATE_DEBUG
	ue.is_copy = FALSE;
	ue.handler = prev_ue->handler;
#endif
  	ue_list.insert(prev_ue, ue);
  	if (Trace_Unwind) 
		fprintf(TFile, "add epilog at bb %d, when %d\n", BB_id(ue.bb), ue.when);
    }
  }
}

static UINT next_when;

static INT Idx_save_ra = 0;
static INT Idx_restore_ra = 0;
static INT Idx_adjust_sp = 0;
static INT Idx_restore_sp = 0;
static INT Idx_adjust_fp = 0;
static INT Idx_restore_fp = 0;
static INT Idx_save_csr = 0;
static INT Idx_restore_csr = 0;

// call per-PU
void 
Init_Unwind_Info (BOOL trace)
{
  if (!CG_emit_unwind_info) return;

  Trace_Unwind = trace;
  has_asm = FALSE;
  PU_has_FP = FALSE;
  PU_has_restored_FP = FALSE;
  UE_validated = FALSE;

  if (Get_Trace (TP_EMIT, 128)) draw_flow_graph();

  Find_Unwind_Info ();

  if (Trace_Unwind) {
	Print_All_Unwind_Elem ("unwind0");
  }

  last_label = 0;
  next_when = 0;

  if (Trace_Unwind) fprintf (TFile, "propagating unwind info\n");
  // need to propagate unwind info to each block,
  // and update ue_list with state changes
  Do_Control_Flow_Analysis_Of_Unwind_Info ();

  Insert_Epilogs();

  if (Trace_Unwind) {
	Print_All_Unwind_Elem ("unwind2");
  }

#ifdef DEBUG_UNWIND
  Print_All_Unwind_Elem("** unwind");
#endif

  // for use in emit_unwind
  ue_iter = ue_list.begin();

  Idx_save_ra = 0;
  Idx_restore_ra = 0;
  Idx_adjust_sp = 0;
  Idx_restore_sp = 0;
  Idx_adjust_fp = 0;
  Idx_restore_fp = 0;
  Idx_save_csr = 0;
  Idx_restore_csr = 0;
}

void 
Finalize_Unwind_Info(void)
{
  if (!CG_emit_unwind_info) return;

  ue_list.clear();
}


static void Generate_Label_For_Unwinding2(LABEL_IDX* label, INT* idx, char* txt,
				      UNWIND_ELEM& ue_iter, BOOL post_process)
{
  LABEL* tmp_lab;
  char* buf;
  if (*label == LABEL_IDX_ZERO) {
    buf = (char *)alloca(strlen(Cur_PU_Name) + 
			 /* EXTRA_NAME_LEN */ 32);
    sprintf(buf, ".LEH_%s%s_%s_%d",
	    post_process ? "post_" : "", txt, Cur_PU_Name, *idx);
    tmp_lab = &New_LABEL(CURRENT_SYMTAB, *label);
    LABEL_Init (*tmp_lab, Save_Str(buf), LKIND_DEFAULT);
    fprintf( Asm_File, "%s:\n", LABEL_name(*label));

    // once we have emitted the pre-op and post-op labels,
    // we can update the counter
    if (post_process) {
      (*idx)++;
    }
  }
  // remember the label we have generated after 'op':
  // this is the one we need to update the debug_frame info

  if (post_process) {
    ue_iter.label_idx = *label;
  }
}

static void Generate_Label_For_Unwinding(LABEL_IDX* label, INT* idx, char* txt,
				      UNWIND_ELEM& ue_iter, BOOL post_process)
{
#ifdef PROPAGATE_DEBUG
  if (!ue_iter.is_copy) {
#endif
  Generate_Label_For_Unwinding2(label, idx, txt, ue_iter, post_process);
#ifdef PROPAGATE_DEBUG
  } else {
    if (!post_process) {
      LABEL_IDX already_generated_lab = *label;
      Generate_Label_For_Unwinding2(label, idx, txt, ue_iter, FALSE);
      *label = already_generated_lab;
      Generate_Label_For_Unwinding2(label, idx, txt, ue_iter, TRUE);
    }
  }
#endif
}

void 
Emit_Unwind_Directives_For_OP(OP *op, FILE *f, BOOL post_process,
			      BOOL inserted_late)
{
  char prefix[3];
  // remember when we finish exploring a bundle
  // so that we can rewind 'next_when' before the post-pass
  static BOOL saved_state = TRUE;
  static UINT saved_next_when;
  static UINT when_bundle_start = 0; // address of the beginning of
				     // the bundle. used later in
				     // Create_Unwind_Descriptors() in
				     // order to generate
				     // DW_CFA_advance_loc insn only
				     // when necessary
  static list < UNWIND_ELEM >::iterator saved_ue_iter;

  // Remember if we create/destroy the frame inside this bundle. This
  // is necessary to handle the case where the frame is created and
  // and a callee-saved is saved to the stack in the same bundle: the
  // offset encoded in the callee-saved save insn is an offset
  // relative to SP before bundle execution, while we want to record
  // the offset after execution, when SP has changed too
  static BOOL bundle_has_frame_change = FALSE;
  INT64 frame_size = 0;

  BOOL emit_directives = TRUE;

  static LABEL_IDX Label_save_ra;
  static LABEL_IDX Label_restore_ra;
  static LABEL_IDX Label_adjust_sp;
  static LABEL_IDX Label_restore_sp;
  static LABEL_IDX Label_adjust_fp;
  static LABEL_IDX Label_restore_fp;
  static LABEL_IDX Label_save_csr;
  static LABEL_IDX Label_restore_csr;

  Label_save_ra     = LABEL_IDX_ZERO;
  Label_restore_ra  = LABEL_IDX_ZERO;
  Label_adjust_sp   = LABEL_IDX_ZERO;
  Label_restore_sp  = LABEL_IDX_ZERO;
  Label_adjust_fp   = LABEL_IDX_ZERO;
  Label_restore_fp  = LABEL_IDX_ZERO;
  Label_save_csr    = LABEL_IDX_ZERO;
  Label_restore_csr = LABEL_IDX_ZERO;

  if (!CG_emit_unwind_info) return;

  // If this OP was inserted after unwind elements computation (eg
  // simulated_op), don't take it into account
  if (inserted_late) {
    return;
  }

  if (!saved_state && post_process) {
    // we start post-processing this bundle, so rewind next_when
    next_when = saved_next_when;
    ue_iter = saved_ue_iter;
  }
  else if (saved_state && !post_process) {
    // we start pre-processing a new bundle, so update saved_next_when
    saved_next_when = next_when;
    saved_ue_iter = ue_iter;
    when_bundle_start = next_when;
    // clear the create_frame
    bundle_has_frame_change = FALSE;
  }

  saved_state = post_process;

  if ( CG_emit_unwind_directives) {
	strcpy(prefix, "");
	emit_directives = TRUE;
  } else {
	strcpy(prefix, ASM_CMNT_LINE);	// emit as comments
	emit_directives = List_Notes;
  }

  if (OP_dummy(op)) return;

  while (ue_iter != ue_list.end() && ue_iter->when == next_when) {
    ISA_REGISTER_CLASS rc = CLASS_REG_PAIR_rclass(ue_iter->rc_reg);
    REGISTER reg = CLASS_REG_PAIR_reg(ue_iter->rc_reg);

    ue_iter->when_bundle_start = when_bundle_start;

    switch (ue_iter->kind) {
    case UE_CREATE_FRAME:
      bundle_has_frame_change = TRUE;
      frame_size += ue_iter->offset;

      Generate_Label_For_Unwinding(&Label_adjust_sp, &Idx_adjust_sp,
				   "adjust_sp", *ue_iter, post_process);
      break;

    case UE_DESTROY_FRAME:
      bundle_has_frame_change = TRUE;
      frame_size -= ue_iter->offset;

      Generate_Label_For_Unwinding(&Label_restore_sp, &Idx_restore_sp,
				   "restore_sp", *ue_iter, post_process);
      break;

    case UE_CREATE_FP:
      Generate_Label_For_Unwinding(&Label_adjust_fp, &Idx_adjust_fp,
				   "adjust_fp", *ue_iter, post_process);
      break;

    case UE_DESTROY_FP:
      Generate_Label_For_Unwinding(&Label_restore_fp, &Idx_restore_fp,
				   "restore_fp", *ue_iter, post_process);
      break;

    case UE_SAVE_SP:
    case UE_SAVE_FP:
      if (CLASS_REG_PAIR_EqualP(ue_iter->rc_reg, CLASS_REG_PAIR_ra)) {
	Generate_Label_For_Unwinding(&Label_save_ra, &Idx_save_ra,
				   "save_ra", *ue_iter, post_process);
      } else {
	Generate_Label_For_Unwinding(&Label_save_csr, &Idx_save_csr,
				   "save_csr", *ue_iter, post_process);
      }

      // adjust offset if SP is adjusted in the same bundle
      if (post_process && bundle_has_frame_change) {
	ue_iter->offset += frame_size;
      }

      /* flag this ue in order to adjust the offset later */
      if (bundle_has_frame_change) {
	ue_iter->frame_changed = TRUE;
      }
      break;

    case UE_RESTORE_SP:
    case UE_RESTORE_FP:
      if (CLASS_REG_PAIR_EqualP(ue_iter->rc_reg, CLASS_REG_PAIR_ra)) {
	Generate_Label_For_Unwinding(&Label_restore_ra, &Idx_restore_ra,
				   "restore_ra", *ue_iter, post_process);
      } else {
	Generate_Label_For_Unwinding(&Label_restore_csr, &Idx_restore_csr,
				   "restore_csr", *ue_iter, post_process);
      }

      // adjust offset if SP is adjusted in the same bundle
      if (post_process && bundle_has_frame_change) {
	ue_iter->offset -= frame_size;
      }

      /* flag this ue in order to adjust the offset later */
      if (bundle_has_frame_change) {
	ue_iter->frame_changed = TRUE;
      }
      break;

    case UE_EPILOG:
      if (emit_directives) fprintf(f, "%s\t.body\n", prefix);
      break;

    case UE_LABEL:
      if (emit_directives) {
	fprintf(f, "%s\t.body\n", prefix);
	fprintf(f, "%s\t.label_state %d\n", prefix, ue_iter->label);
      }
      break;

    case UE_COPY:
      if (emit_directives) {
	fprintf(f, "%s\t.body\n", prefix);
	fprintf(f, "%s\t.copy_state %d\n", prefix, ue_iter->label);
      }
      break;

    case UE_RESTORE_GR:
      if (ue_iter->qp != 0) {
	if (emit_directives) {
	  fprintf(f, "%s\t.restorereg.p p%d, %s\n", prefix, 
		  ue_iter->qp,
		  UE_Register_Name(rc, reg) );
	}
      }
      else {
	if (CLASS_REG_PAIR_EqualP(ue_iter->rc_reg, CLASS_REG_PAIR_ra)) {
	  Generate_Label_For_Unwinding(&Label_restore_ra, &Idx_restore_ra,
				       "restore_ra", *ue_iter, post_process);
	} else {
	  Generate_Label_For_Unwinding(&Label_restore_csr, &Idx_restore_csr,
				       "restore_csr", *ue_iter, post_process);
	}
      }
      break;
    case UE_SAVE_GR:
      if (ue_iter->qp != 0) {
	if (emit_directives) {
	  fprintf(f, "%s\t%s p%d, %s, %s\n", prefix, ".spillreg.p",
		  ue_iter->qp,
		  UE_Register_Name(rc, reg),
		  UE_Register_Name( CLASS_REG_PAIR_rclass(ue_iter->save_rc_reg),
				    CLASS_REG_PAIR_reg(ue_iter->save_rc_reg) ) );
	}
      }
      else {

	if (CLASS_REG_PAIR_EqualP(ue_iter->rc_reg, CLASS_REG_PAIR_ra)) {
	  Generate_Label_For_Unwinding(&Label_save_ra, &Idx_save_ra,
				       "save_ra", *ue_iter, post_process);
	} else {
	  Generate_Label_For_Unwinding(&Label_save_csr, &Idx_save_csr,
				       "save_csr", *ue_iter, post_process);
	}
      }
      break;

    default:
      FmtAssert(FALSE, ("Unhandled UNWIND_ELEM kind (%d)\n", ue_iter->kind));
      /*NOTREACHED*/
    }

    ++ue_iter;
  }

  next_when += OP_Real_Inst_Words(op);
}


extern char *Cg_Dwarf_Name_From_Handle(Dwarf_Unsigned idx);

// process info we've collected and create the unwind descriptors
static void
Create_Unwind_Descriptors (Dwarf_P_Fde fde, Elf64_Word	scn_index,
			   Dwarf_Unsigned begin_label, bool emit_restores)
{
  ISA_REGISTER_CLASS rc, save_rc;
  REGISTER reg, save_reg;

  Dwarf_Error dw_error;
  INT64 frame_size = 0;
  INT64 frame_offset = 0;

  INT64 current_loc = -1;  // current address

  // record the reference label for advance_loc insns
  Dwarf_Unsigned last_label_idx = begin_label;

#ifdef DEBUG_UNWIND
  fprintf(TFile, "** %s init label %s\n", __FUNCTION__,
	  Cg_Dwarf_Name_From_Handle(begin_label));
#endif

  for (ue_iter = ue_list.begin(); ue_iter != ue_list.end(); ++ue_iter) {

#ifdef PROPAGATE_DEBUG
    // if this UE is a copy state, pretend its adress is the previous one
    if (ue_iter->is_copy) {
      ue_iter->when_bundle_start--;
    }
#endif

    frame_size = ue_iter->frame_size;

    switch (ue_iter->kind) {
    case UE_EPILOG:
    case UE_LABEL:
    case UE_COPY:
      continue;

    case UE_CREATE_FRAME:
      if (!ue_iter->handler) {
	if (frame_size <= 0 && Trace_Unwind) {
	  Print_Unwind_Elem(*ue_iter, "bad frame size");
	}
	Is_True(frame_size > 0,
		("unwind: frame size should be > 0 at creation point"));
      }

      if (ue_iter->when_bundle_start > current_loc) {
	dwarf_add_fde_inst(fde, DW_CFA_advance_loc4, last_label_idx,
			   Cg_Dwarf_Symtab_Entry(CGD_LABIDX,
						 ue_iter->label_idx,
						 scn_index),
			   &dw_error);
      }
      dwarf_add_fde_inst(fde, DW_CFA_def_cfa_offset,
			 frame_size + STACK_OFFSET_ADJUSTMENT,
			 0, &dw_error);
      if (Trace_Unwind) {
	fprintf(TFile, "create stack frame of size %lld at when %d\n",
		frame_size, ue_iter->when);
      }

#ifdef DEBUG_UNWIND
      fprintf(TFile, "** %s label %s to %s advance loc + create frame %lld\n",
	      __FUNCTION__, LABEL_name(ue_iter->label_idx),
	      Cg_Dwarf_Name_From_Handle(last_label_idx), frame_size);
#endif
      break;

    case UE_DESTROY_FRAME:
      if (!emit_restores) continue;

      if (!ue_iter->handler) {
	if (frame_size < 0 && Trace_Unwind) {
	  Print_Unwind_Elem(*ue_iter, "bad frame size");
	}
	Is_True(frame_size >= 0,
		("unwind: bad frame size at destroy point %d remaining after destruction of %d", frame_size, ue_iter->offset));
      }

      if (ue_iter->when_bundle_start > current_loc) {
	dwarf_add_fde_inst(fde, DW_CFA_advance_loc4, last_label_idx,
			   Cg_Dwarf_Symtab_Entry(CGD_LABIDX,
						 ue_iter->label_idx,
						 scn_index),
			   &dw_error);
      }
      dwarf_add_fde_inst(fde, DW_CFA_def_cfa_offset, 
			 frame_size + STACK_OFFSET_ADJUSTMENT,
			 0, &dw_error);

      if (Trace_Unwind) {
	fprintf(TFile, "destroy stack frame of size %lld at when %d, remaining %lld\n",
		ue_iter->offset, ue_iter->when, frame_size);
      }

#ifdef DEBUG_UNWIND
      fprintf(TFile, "** %s label %s to %s advance loc + destroy frame\n",
	      __FUNCTION__, LABEL_name(ue_iter->label_idx),
	      Cg_Dwarf_Name_From_Handle(last_label_idx));
#endif
      break;

    case UE_CREATE_FP:
      rc = CLASS_REG_PAIR_rclass(ue_iter->rc_reg);
      reg = CLASS_REG_PAIR_reg(ue_iter->rc_reg);
      Is_True(frame_size == ue_iter->offset,
	      ("unwind: dynamic frame size invalid at creation point (%lld vs %lld)",
	       ue_iter->offset, frame_size));

      if (ue_iter->when_bundle_start > current_loc) {
	dwarf_add_fde_inst(fde, DW_CFA_advance_loc4, last_label_idx,
			   Cg_Dwarf_Symtab_Entry(CGD_LABIDX,
						 ue_iter->label_idx,
						 scn_index),
			   &dw_error);
      }
      dwarf_add_fde_inst(fde, DW_CFA_def_cfa, REGISTER_Get_Debug_Reg_Id(rc,reg),
			 STACK_OFFSET_ADJUSTMENT,
			 &dw_error);
      if (Trace_Unwind) {
	fprintf(TFile, "create dynamic frame of size %lld at when %d\n",
		frame_size, ue_iter->when);
      }

#ifdef DEBUG_UNWIND
      fprintf(TFile, "** %s label %s to %s advance loc + create dynamic frame %lld\n",
	      __FUNCTION__, LABEL_name(ue_iter->label_idx),
	      Cg_Dwarf_Name_From_Handle(last_label_idx), frame_size);
#endif
      break;

    case UE_DESTROY_FP:
      if (!emit_restores) continue;

      if (ue_iter->when_bundle_start > current_loc) {
	dwarf_add_fde_inst(fde, DW_CFA_advance_loc4, last_label_idx,
			   Cg_Dwarf_Symtab_Entry(CGD_LABIDX,
						 ue_iter->label_idx,
						 scn_index),
			   &dw_error);
      }

      dwarf_add_fde_inst(fde, DW_CFA_def_cfa,
			 REGISTER_Get_Debug_Reg_Id(
					     CLASS_REG_PAIR_rclass(CLASS_REG_PAIR_sp),
					     CLASS_REG_PAIR_reg(CLASS_REG_PAIR_sp)),
			 STACK_OFFSET_ADJUSTMENT,
			 &dw_error);

      if (Trace_Unwind) {
	fprintf(TFile, "destroy dynamic frame of size %lld at when %d\n",
		frame_size, ue_iter->when);
      }

#ifdef DEBUG_UNWIND
      fprintf(TFile, "** %s label %s to %s advance loc + destroy dynamic frame\n",
	      __FUNCTION__, LABEL_name(ue_iter->label_idx),
	      Cg_Dwarf_Name_From_Handle(last_label_idx));
#endif
      break;

    case UE_SAVE_SP:
    case UE_SAVE_FP:
      rc = CLASS_REG_PAIR_rclass(ue_iter->rc_reg);
      reg = CLASS_REG_PAIR_reg(ue_iter->rc_reg);
      frame_offset = - ue_iter->offset + STACK_OFFSET_ADJUSTMENT;
      if (ue_iter->kind == UE_SAVE_SP) {
#ifdef PROPAGATE_DEBUG
	if (ue_iter->is_copy == FALSE) {
	  frame_offset += frame_size;
	} else {
	  // if is_copy is TRUE, we are sure that top_offset is valid
	  frame_offset = -ue_iter->top_offset + STACK_OFFSET_ADJUSTMENT;
	}
#else
	frame_offset += frame_size;
#endif
      }

      if (ue_iter->when_bundle_start > current_loc) {
	dwarf_add_fde_inst(fde, DW_CFA_advance_loc4, last_label_idx,
			   Cg_Dwarf_Symtab_Entry(CGD_LABIDX,
						 ue_iter->label_idx,
						 scn_index),
			   &dw_error);
      }

      dwarf_add_fde_inst(fde, DW_CFA_offset, REGISTER_Get_Debug_Reg_Id(rc,reg),
		 frame_offset / Data_Alignment_Factor,
			 &dw_error);

      if (Trace_Unwind) {
	fprintf(TFile, "save reg to sp mem offset %lld at when %d\n",
		frame_offset, ue_iter->when);
	if (ue_iter->frame_changed)
	  fprintf(TFile, "  frame changed in the same bundle\n");
      }

#ifdef DEBUG_UNWIND
      fprintf(TFile,
	      "** %s label %s to %s adv loc + save reg %d at offset %llu\n",
	      __FUNCTION__, LABEL_name(ue_iter->label_idx),
	      Cg_Dwarf_Name_From_Handle(last_label_idx),
	      REGISTER_Get_Debug_Reg_Id(rc,reg),
	      frame_offset);
#endif

      break;

    case UE_RESTORE_SP:
    case UE_RESTORE_FP:
      if (!emit_restores) continue;

      rc = CLASS_REG_PAIR_rclass(ue_iter->rc_reg);
      reg = CLASS_REG_PAIR_reg(ue_iter->rc_reg);

      if (ue_iter->when_bundle_start > current_loc) {
	dwarf_add_fde_inst(fde, DW_CFA_advance_loc4, last_label_idx,
			   Cg_Dwarf_Symtab_Entry(CGD_LABIDX,
						 ue_iter->label_idx,
						 scn_index),
			   &dw_error);
      }

      dwarf_add_fde_inst(fde, DW_CFA_restore, REGISTER_Get_Debug_Reg_Id(rc,reg),
			 0, &dw_error);

      if (Trace_Unwind) {
	fprintf(TFile, "restore reg from %s mem at when %d\n",
		ue_iter->kind == UE_RESTORE_FP ? "fp" : "sp", ue_iter->when);
	if (ue_iter->frame_changed)
	  fprintf(TFile, "  frame changed in the same bundle\n");
      }

#ifdef DEBUG_UNWIND
      fprintf(TFile, "** %s label %s to %s adv loc + restore reg %llu\n",
	      __FUNCTION__, LABEL_name(ue_iter->label_idx),
	      Cg_Dwarf_Name_From_Handle(last_label_idx),
	      REGISTER_Get_Debug_Reg_Id(rc,reg));
#endif

      break;

    case UE_SAVE_GR:
      rc = CLASS_REG_PAIR_rclass(ue_iter->rc_reg);
      reg = CLASS_REG_PAIR_reg(ue_iter->rc_reg);
      save_rc = CLASS_REG_PAIR_rclass(ue_iter->save_rc_reg);
      save_reg = CLASS_REG_PAIR_reg(ue_iter->save_rc_reg);

      if (ue_iter->when_bundle_start > current_loc) {
	dwarf_add_fde_inst(fde, DW_CFA_advance_loc4, last_label_idx,
			   Cg_Dwarf_Symtab_Entry(CGD_LABIDX,
						 ue_iter->label_idx,
						 scn_index),
			   &dw_error);
      }

      dwarf_add_fde_inst(fde, DW_CFA_register, REGISTER_Get_Debug_Reg_Id(rc,reg),
			 REGISTER_Get_Debug_Reg_Id(save_rc,save_reg),
			 &dw_error);

      if (Trace_Unwind) {
	fprintf(TFile, "save reg %s to reg %s at when %d\n",
		UE_Register_Name(rc, reg),
		UE_Register_Name(save_rc, save_reg),
		ue_iter->when);
      }

#ifdef DEBUG_UNWIND
      fprintf(TFile,
	      "** %s label %s to %s adv loc + save reg %s to reg %s\n",
	      __FUNCTION__, LABEL_name(ue_iter->label_idx),
	      Cg_Dwarf_Name_From_Handle(last_label_idx),
	      UE_Register_Name(rc,reg),
	      UE_Register_Name(save_rc,save_reg));
#endif

      break;

    case UE_RESTORE_GR:
      if (!emit_restores) continue;

      rc = CLASS_REG_PAIR_rclass(ue_iter->rc_reg);
      reg = CLASS_REG_PAIR_reg(ue_iter->rc_reg);
      save_rc = CLASS_REG_PAIR_rclass(ue_iter->save_rc_reg);
      save_reg = CLASS_REG_PAIR_reg(ue_iter->save_rc_reg);

      if (ue_iter->when_bundle_start > current_loc) {
	dwarf_add_fde_inst(fde, DW_CFA_advance_loc4, last_label_idx,
			   Cg_Dwarf_Symtab_Entry(CGD_LABIDX,
						 ue_iter->label_idx,
						 scn_index),
			   &dw_error);
      }

      dwarf_add_fde_inst(fde, DW_CFA_restore, REGISTER_Get_Debug_Reg_Id(rc,reg),
			 0, &dw_error);

      if (Trace_Unwind) {
	fprintf(TFile, "restore reg %s from reg %s when %d\n",
 		UE_Register_Name(rc, reg),
		UE_Register_Name(save_rc, save_reg),
		ue_iter->when);
      }

#ifdef DEBUG_UNWIND
      fprintf(TFile,
	      "** %s label %s to %s adv loc + restore reg %s from reg %s\n",
	      __FUNCTION__, LABEL_name(ue_iter->label_idx),
	      Cg_Dwarf_Name_From_Handle(last_label_idx),
	      UE_Register_Name(rc,reg),
	      UE_Register_Name(save_rc,save_reg));
#endif

      break;

    default:
      FmtAssert(FALSE, ("%s: Unhandled UNWIND_ELEM kind (%d)\n", __FUNCTION__, ue_iter->kind));
      break;
    }
    last_label_idx = Cg_Dwarf_Symtab_Entry(CGD_LABIDX, ue_iter->label_idx,
					   scn_index);
#ifdef DEBUG_UNWIND
    fprintf(TFile, "** %s upgraded last label to %s\n", __FUNCTION__,
	    Cg_Dwarf_Name_From_Handle(last_label_idx));
#endif

    current_loc = ue_iter->when_bundle_start;
  }
}

/* construct the fde for the current procedure. */
extern Dwarf_P_Fde
Build_Fde_For_Proc (Dwarf_P_Debug dw_dbg, BB *firstbb,
		    Dwarf_Unsigned begin_label,
		    Dwarf_Unsigned end_label,
		    INT32     end_offset,
		    // The following two arguments need to go away
		    // once libunwind gives us an interface that
		    // supports symbolic ranges.
		    INT       low_pc,
		    INT       high_pc,
		    Elf64_Word	scn_index,
                    bool emit_restores)
{
  Dwarf_P_Fde fde;
  Dwarf_Error dw_error;

  if ( ! CG_emit_unwind_info) return NULL;
  if ( CG_emit_unwind_directives) return NULL;

  fde = dwarf_new_fde(dw_dbg, &dw_error);

  // process info we've collected and create the unwind descriptors
  Create_Unwind_Descriptors(fde, scn_index, begin_label, emit_restores);

  if (has_asm)
	DevWarn("unwind info may be incorrect because PU has asm");

  return fde;
}

#endif // !defined(TARG_ST)
