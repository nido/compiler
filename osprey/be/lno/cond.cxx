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


/**
*** The data structure used here, private to this file (for now) is
*** A COND_BOUNDS_INFO.  It is nothing more than a system of equations
*** representing the loop bounds and further outer if conditions, and
*** a stack of symbols indicating which column in the system of equations
*** corresponds to which variable.  Associate with each symbol is the
*** outermost region in which each symbol is known to be invariant.
*** For example, if we have
***
***	if (n > 0)
***	   ...
***
*** then when we see the if, we associate the if node with the symbol n.
*** If further in we have a use of n in some conditional expression, and
*** the writes that could alter that n (from the use-def chains) could
*** have happened inside the above if, we know that the n's are different.
*** Collect_Outer_Info works, and must work, from the outside in.  It
*** calls Kill_Written_Symbols.  That code makes sure that all symbols
*** used are from definitions outside of all other conditions that use
*** the same symbol, so that the same values are being reused.  If that
*** condition might not be true, we discard all equations involving that
*** symbol.
**/

#ifdef USE_PCH
#include "lno_pch.h"
#endif // USE_PCH
#pragma hdrstop

#define cond_CXX      "cond.cxx"

#include <sys/types.h>
#include "soe.h"
#include "cond.h"
#include "lwn_util.h"
#include "lnopt_main.h"
#include "opt_du.h"
#include "lnoutils.h"
#include "region_util.h"
#include "lego_util.h"
#include "snl_utils.h"
#include "small_trips.h"
#include "eliminate.h"
#include "fb_whirl.h"

extern WN* Find_SCF_Inside(WN* parent_wn, OPCODE opc); // in ff_utils.cxx

void COND_SYMBOL_INFO::Print(FILE* f) const
{
  if (Outer_Nondef)
    fprintf(f, "`%s' at line %d",
            Symbol.Name(),
            Srcpos_To_Line(WN_Get_Linenum(Outer_Nondef)));
  else
    fprintf(f, "%s no wn", Symbol.Name());
}

//-----------------------------------------------------------------------
// Constructors and initialization
//-----------------------------------------------------------------------

COND_BOUNDS_INFO::COND_BOUNDS_INFO(MEM_POOL* mp)
  : _pool(mp),
    _bounds(0,0,0,mp),
    _symbol_info(mp)
{
}

COND_BOUNDS_INFO::~COND_BOUNDS_INFO()
{
}

//-----------------------------------------------------------------------
// Adding an equation
//-----------------------------------------------------------------------

INT COND_BOUNDS_INFO::Lookup_Entry(SYMBOL s, WN* control)
{
  INT entry;
  for (entry = 0; entry < _symbol_info.Elements(); entry++) {
    if (_symbol_info.Bottom_nth(entry).Symbol == s)
      return entry;
  }

  // previously unseen
  _symbol_info.Push(COND_SYMBOL_INFO(s, control));
  _bounds.Add_Vars(1);
  return entry;
}

// If this thing is written inside where our previous records require it
// not be written, then just remove all equations involving this variable.
// We only do this for non-induction variables, of course, since induction
// variables should be taken care of already by can.

void COND_BOUNDS_INFO::Kill_Written_Symbols(ACCESS_VECTOR* av,
					      WN* code,
					      WN* control)
{
  if (av->Contains_Lin_Symb() == FALSE)
    return;

  for (LWN_ITER* i = LWN_WALK_TreeIter(code); i; i = LWN_WALK_TreeNext(i)) {
    WN* wn = i->wn;

    if (WN_operator(wn) != OPR_LDID)
      continue;

    // make sure it's on Lin_Symb list, because we don't care about
    // induction variables.

    SYMBOL symbol(wn);

    BOOL found = FALSE;
    INTSYMB_ITER ii(av->Lin_Symb);
    for (INTSYMB_NODE* nn = ii.First(); !ii.Is_Empty(); nn = ii.Next()) {
      if (symbol == nn->Symbol) {
	found = TRUE;
	break;
      }
    }
    if (!found) {
      continue;
    }

    INT entry = Lookup_Entry(symbol, control);
    WN* wnouter = Symbol_Info().Bottom_nth(entry).Outer_Nondef;

    if (wnouter == control)
      continue;

    DEF_LIST *defs = Du_Mgr->Ud_Get_Def(wn);
    BOOL bad_write = FALSE;
    if (defs == NULL) {
      bad_write = TRUE;
#ifdef Is_True_On
    if (TY_kind(ST_type(WN_st(wn))) == KIND_SCALAR) { 
      WN *tmp = wn;
      while (tmp && WN_opcode(tmp) != OPC_IO) tmp = LWN_Get_Parent(tmp);
      Is_True(tmp, ("Missing defs for %s (wn=%ld=0x%lx)",
		   SYMBOL(wn).Name(), wn, wn));
    } 
#endif
    } else {
      DEF_LIST_ITER iter(defs);
      for(const DU_NODE *n = iter.First();
			!bad_write && !iter.Is_Empty(); n = iter.Next()) {
        WN *def = n->Wn();
        while (!bad_write && def) {
          def = LWN_Get_Parent(def);
          if (def == wnouter) {
            bad_write = TRUE;
	    if (LNO_Verbose) fprintf(stderr, "def at %d, wnouter at %d\n",
	      Srcpos_To_Line(WN_Get_Linenum(def)),
	      Srcpos_To_Line(WN_Get_Linenum(wnouter)));
	    }
	 }
      }
    }

    if (bad_write) {
      if (LNO_Verbose) fprintf(stderr, "Bad write for %s\n", symbol.Name());
      // go through every equation and remove those containing
      // this variable.
      Symbol_Info().Bottom_nth(entry).Outer_Nondef = control;

      for (INT i = Bounds().Num_Le_Constraints() - 1; i >= 0; i--) {
	if (Bounds().Ale()(i,entry) != 0) {
	  // remove equation i by zeroing it (all zeros is always consistent)
	  // we don't actuall remove the equation since parent code relies
	  // on the fact that the number of equations only grows
	  for (INT j=0; j<Bounds().Num_Vars(); j++) {
	    Bounds().Ale()(i,j) = 0;
          }
	  Bounds().Ble()[i] = 0;
	}
      }
    }
  }
}

// Add this equation to the soe, possibly adding additional symbols to 
// the Symbol_Info().  An access vector expresses ai+bj <= const_offset,
// which is exactly what an soe wants.  But the soe expects the variables
// to come in a certain order, so look up variables in the list.  Note
// that, for now, loop indices are represented by the symbol (0,0,i).

INT COND_BOUNDS_INFO::Add_Access(ACCESS_VECTOR* av, WN* code, WN* control)
{
  if (av->Too_Messy || av->Contains_Non_Lin_Symb())
    return 0;

  Kill_Written_Symbols(av, code, control);

  INT		vsz = (av->Lin_Symb ? av->Lin_Symb->Len() : 0) +
			Symbol_Info().Elements() + av->Nest_Depth() + 1;
  mINT32*	v = CXX_NEW_ARRAY(mINT32, vsz, &LNO_local_pool);

  for (INT vszx = 0; vszx < vsz; vszx++)
    v[vszx] = 0;

  // put loop indices into v.

  for (INT i = 0; i <= av->Nest_Depth(); i++) {
    INT coeff = av->Loop_Coeff(i);
    if (coeff) {
      INT entry = Lookup_Entry(SYMBOL((ST*)NULL, i, MTYPE_V), control);
      Is_True(entry < vsz,("Overflow1 in Add_Access\n"));
      v[entry] = coeff;
    }
  }

  // add symbolic constants

  if (av->Contains_Lin_Symb()) {
    INTSYMB_ITER ii(av->Lin_Symb);
    for (INTSYMB_NODE* n = ii.First(); !ii.Is_Empty(); n = ii.Next()) {
      INT entry = Lookup_Entry(n->Symbol, control);
      Is_True(entry < vsz,("Overflow2 in Add_Access\n"));
      v[entry] = n->Coeff;
    }
  }

  _bounds.Add_Le(v, av->Const_Offset);

  CXX_DELETE_ARRAY(v, &LNO_local_pool);
  return 1;
}

INT COND_BOUNDS_INFO::Add_Access(ACCESS_ARRAY* aa, WN* code, WN* control)
{
  INT added_cnt = 0;

  for (INT i = 0; i < aa->Num_Vec(); i++)
    added_cnt += Add_Access(aa->Dim(i), code, control);

  return added_cnt;
}

//-----------------------------------------------------------------------
// Collecting equations from structured control flow
//-----------------------------------------------------------------------

void COND_BOUNDS_INFO::Collect_If_Info(WN* wn, BOOL in_then_part)
{
  Is_True(WN_opcode(wn) == OPC_IF,
	  ("bad opcode %d for Collect_If_Info()", WN_opcode(wn)));

  IF_INFO* ii = Get_If_Info(wn, TRUE);

  if (Pool() != &LNO_local_pool)
    MEM_POOL_Push(&LNO_local_pool);

  if (ii == NULL) {
    fprintf(stderr, "Missing IF annotation!");
  }
  else if (!in_then_part == !ii->Condition_On_Then) {
    // the access vector applies to us
    Add_Access(ii->Condition, WN_if_test(wn), wn);
  }
  else {
    // If more than one condition, a disjunction and we are hosed.
    // But if exactly one, then just reverse the condition
    if (ii->Condition->Num_Vec() == 1) {
      ACCESS_VECTOR av(ii->Condition->Dim(0), Pool());
      av.Negate_Me();
      av.Const_Offset--;
      Add_Access(&av, WN_if_test(wn), wn);
    }
  }
  if (Pool() != &LNO_local_pool)
    MEM_POOL_Pop(&LNO_local_pool);
}

void COND_BOUNDS_INFO::Collect_Do_Info(WN* wn)
{
  Is_True(WN_opcode(wn) == OPC_DO_LOOP,
	  ("bad opcode %d for Collect_Do_Info()", WN_opcode(wn)));

  // save some space, but only if legal.

  if (Pool() != &LNO_local_pool)
    MEM_POOL_Push(&LNO_local_pool);

  DO_LOOP_INFO* dli = Get_Do_Loop_Info(wn);
  Add_Access(dli->LB, WN_kid0(WN_start(wn)), wn);
  Add_Access(dli->UB, WN_end(wn), wn);

  if (Pool() != &LNO_local_pool)
    MEM_POOL_Pop(&LNO_local_pool);
}

// Examine all parents, looking for conditions that we can add to the
// system of equations.

void COND_BOUNDS_INFO::Collect_Outer_Info(WN* wn, WN* child)
{
  if (wn == NULL)
    return;

  Collect_Outer_Info(LWN_Get_Parent(wn), wn);

  if (child == NULL)		// innermost loop is skipped.
    return;

  switch (WN_opcode(wn)) {
   case OPC_IF:
    BOOL in_then_part;
    if (child == WN_then(wn))
      in_then_part = TRUE;
    else if (child == WN_else(wn))
      in_then_part = FALSE;
    else
      FmtAssert(0, ("Bad if/then/else prev condition"));
    Collect_If_Info(wn, in_then_part);
    break;
   case OPC_DO_LOOP:
    Collect_Do_Info(wn);
    break;
  }
}

void COND_BOUNDS_INFO::Print(FILE* f) const
{
  fprintf(f, "Variables: "); 
  for (INT i = 0; i < _symbol_info.Elements(); i++) { 
    if (i > 0)
      fprintf(f, ", ");
    _symbol_info.Bottom_nth(i).Print(f);
  }

  fprintf(f, "\nBounds:\n");
  _bounds.Print(f);
}

void COND_BOUNDS_INFO::Reset_Varcount_To(INT cols)
{
  Is_True(cols <= _symbol_info.Elements(),
	  ("Reset_Varcount_To() len=%d, cols=%d",
	   _symbol_info.Elements(), cols));

  for (INT i = _symbol_info.Elements(); i > cols; i--)
    _symbol_info.Pop();
}

void COND_BOUNDS_INFO::Reset_Bounds_To(INT rows_le, 
				       INT rows_eq, 
				       INT cols,
				       DYN_ARRAY<WN*>* kill_array)
{
  Reset_Varcount_To(cols);
  Bounds().Reset_To(rows_le, rows_eq, cols);
  for (INT i = 0; i < cols; i++)
    _symbol_info.Bottom_nth(i).Outer_Nondef = (*kill_array)[i];
}

///////////////////////////////////////////////////////////////////////
// The Global Functions
///////////////////////////////////////////////////////////////////////

COND_IF_INFO COND_If_Info(WN* wn_if, MEM_POOL* pool)
{
  if (pool == NULL)
    pool = &LNO_local_pool;

  COND_IF_INFO rval;

  MEM_POOL_Push(pool);

  {	// scope to make sure info goes out of scope before pool

    // step 1: gather information outside loop

    COND_BOUNDS_INFO info(pool);

    // step 2: If we add information from the then part of the if,
    // and it's inconsistent, then the else part must execute.  If we
    // add information from the else part and it's inconsistent, then
    // the then part must execute.

    info.Collect_Outer_Info(wn_if);

    IF_INFO* ii = Get_If_Info(wn_if, TRUE);
    if (ii == NULL) {
      Is_True(1, ("Missing IF annotation!"));
      rval = COND_IF_NOT_SURE;
    }
    else {
      INT le = info.Bounds().Num_Le_Constraints();
      INT eq = info.Bounds().Num_Eq_Constraints();
      INT c  = info.Symbol_Info().Elements();
      DYN_ARRAY<WN*> kill_array(&LNO_local_pool);
      for (INT i = 0; i < c; i++) {
        WN* wn = info.Symbol_Info().Bottom_nth(i).Outer_Nondef;
        kill_array.AddElement(wn);
      }

      info.Collect_If_Info(wn_if, TRUE);

      if (!info.Bounds().Is_Consistent())
	rval = COND_IF_ELSE_ONLY;
      else {
	info.Reset_Bounds_To(le, eq, c, &kill_array);
	info.Collect_If_Info(wn_if, FALSE);
	if (!info.Bounds().Is_Consistent())
	  rval = COND_IF_THEN_ONLY;
	else
	  rval = COND_IF_NOT_SURE;
      }
    }
  }

  MEM_POOL_Pop(pool);

  return rval;
}

COND_DO_INFO COND_Do_Info(WN* wn_do, MEM_POOL* pool)
{
  if (pool == NULL)
    pool = &LNO_local_pool;

  COND_DO_INFO rval;

  MEM_POOL_Push(pool);

  // add scope so that the info struct below
  // gets deallocated before the mem-pool pop
  {

    // step 1: gather information outside loop

    COND_BOUNDS_INFO info(pool);

    // step 2: If we add information from outside the loop.
    // When we also include these bounds, if the result is inconsistent,
    // the loop never executes.  If we instead include the statement
    // ub - lb < 0 and that's inconsistent, things go at least once.
    // Otherwise, we are not sure.

    info.Collect_Outer_Info(wn_do);

    INT le = info.Bounds().Num_Le_Constraints();
    INT eq = info.Bounds().Num_Eq_Constraints();
    INT c  = info.Symbol_Info().Elements();
    DYN_ARRAY<WN*> kill_array(&LNO_local_pool);
    for (INT i = 0; i < c; i++) {
      WN* wn = info.Symbol_Info().Bottom_nth(i).Outer_Nondef;
      kill_array.AddElement(wn);
    }
    
    info.Collect_Do_Info(wn_do);

    if (info.Bounds().Is_Consistent() == FALSE) {
      rval = COND_DO_NEVER;
      goto return_point;
    }

    {
   
      info.Reset_Bounds_To(le, eq, c, &kill_array);

      DO_LOOP_INFO* dli = Get_Do_Loop_Info(wn_do);
#ifdef TARG_ST
      if (dli->Min_Iterations > 0) {
	rval = COND_DO_AT_LEAST_ONCE;
	goto return_point;
      }
#endif
      if (dli->LB->Too_Messy || dli->UB->Too_Messy)
        rval = COND_DO_MAYBE;
      else {
        rval = COND_DO_AT_LEAST_ONCE;
  
        // the equation of interest is min(u1,u2,...) - max(l1,l2,...) < 0
        //    i.e. u1-l1 < 0 or u1-l2 < 0 or ...
        // Such a system is inconsistent only if the system with u1-l1 < 0
        // is inconsistent and the system with u1-l2<0 is inconsistent and ... 
  
        for (INT dimlb = 0; dimlb < dli->LB->Num_Vec(); dimlb++) {
          ACCESS_VECTOR* avlb = dli->LB->Dim(dimlb);
          if (avlb->Too_Messy) {
            rval = COND_DO_MAYBE;
            goto return_point;
          }
          for(INT dimub = 0; dimub < dli->UB->Num_Vec(); dimub++) {
            ACCESS_VECTOR* avub = dli->UB->Dim(dimub);
            if (avub->Too_Messy) {
              rval = COND_DO_MAYBE;
              goto return_point;
            }
  
            ACCESS_VECTOR* nox = Difference_Inequality(avlb, avub,
                                                       avlb->Nest_Depth()-1,
                                                       DIFFERENCE_EXEC_NEVER,
                                                       pool);
            // adding twice because we want to make sure ldid's all valid
            info.Add_Access(nox, WN_kid0(WN_start(wn_do)), wn_do);
            info.Add_Access(nox, WN_kid1(WN_end(wn_do)), wn_do);
            BOOL is_consistent = info.Bounds().Is_Consistent();
            info.Bounds().Remove_Last_Le(2);
            if (is_consistent) {
              rval = COND_DO_MAYBE;
              goto return_point;
            }
          }
        }
      }
    }
  
    return_point:
    /* add a dummy statement to avoid a warning */
    ;
  }

  MEM_POOL_Pop(pool);

  return rval;
}

// make sure LNO_Build_If_Access(WN *wn, DOLOOP_STACK *stack, MEM_POOL *pool)
// has been run

void COND_Test(WN* wn)
{
  OPCODE opcode = WN_opcode(wn);

  switch (opcode) {
   case OPC_BLOCK:
    {
      for (WN* kid = WN_first(wn); kid; kid = WN_next(kid)) {
        COND_Test(kid);
      }
    }
    break;
   case OPC_DO_LOOP:
    fprintf(stderr, "DO line %d (%s) ... ",
	    Srcpos_To_Line(WN_Get_Linenum(wn)),
	    SYMBOL(WN_index(wn)).Name());
    fflush(stderr);
    switch (COND_Do_Info(wn)) {
     case COND_DO_MAYBE:
      fprintf(stderr, "maybe\n");
      break;
     case COND_DO_NEVER:
      fprintf(stderr, "never\n");
      break;
     case COND_DO_AT_LEAST_ONCE:
      fprintf(stderr, "at least once\n");
      break;
     default:
      Is_True(0, ("Bug in COND_Do_Info result"));
    }
    COND_Test(WN_do_body(wn));
    break;
   case OPC_IF:
    fprintf(stderr, "IF line %d ... ",
	    Srcpos_To_Line(WN_Get_Linenum(wn)));
    fflush(stderr);
    switch (COND_If_Info(wn)) {
     case COND_IF_NOT_SURE:
      fprintf(stderr, "not sure\n");
      break;
     case COND_IF_THEN_ONLY:
      fprintf(stderr, "then only\n");
      break;
     case COND_IF_ELSE_ONLY:
      fprintf(stderr, "else only\n");
      break;
     default:
      Is_True(0, ("Bug in COND_If_Info result"));
    }
    COND_Test(WN_then(wn));
    COND_Test(WN_else(wn));
    break;
   default:
    if (!OPCODE_is_expression(opcode)) {
      for (INT kidno = 0; kidno < WN_kid_count(wn); kidno++) {
	COND_Test(WN_kid(wn,kidno));
      }
    }
    break;
  }
}

static BOOL Eliminate_Dead_SCF_rec(WN*, void Remove_Region(WN *),
	COND_BOUNDS_INFO *, LABEL_LIST* label_list);
static BOOL Eliminate_Dead_If(WN*, void Remove_Region(WN *), 
        COND_BOUNDS_INFO *, LABEL_LIST* label_list);
static BOOL Eliminate_Dead_Do(WN*, void Remove_Region(WN *), 
        COND_BOUNDS_INFO *, LABEL_LIST* label_list);

// Get rid of unexecutable code
// Return TRUE if eliminated something
BOOL Eliminate_Dead_SCF(WN *wn, void Remove_Region(WN *))
{
  if (Get_Trace(TP_LNOPT,TT_LNO_DEAD)) {
    fprintf(TFile,"Eliminating_Dead_SCF\n");
  }
  MEM_POOL_Push(&LNO_local_pool);
  LABEL_LIST* label_list = 
    CXX_NEW(LABEL_LIST(&LNO_local_pool, Current_Func_Node), &LNO_local_pool);
  COND_BOUNDS_INFO *info = 
       CXX_NEW(COND_BOUNDS_INFO(&LNO_local_pool),&LNO_local_pool);
  BOOL result =  Eliminate_Dead_SCF_rec(wn, Remove_Region, info, label_list);
  MEM_POOL_Pop(&LNO_local_pool);
  return result;
}

// How many iterations in a loop,
// return -1 if don't know or if not constant
// This is more conservative than the code in snl_utils,it
// doesn't change the loop
static INT64 Iters(WN *loop)
{
  INT64 rval=-1;
  INT64  stepsz = Step_Size(loop);
  if (stepsz < 1) return -1;

  DO_LOOP_INFO *dli = Get_Do_Loop_Info(loop);
  if (dli->LB->Num_Vec() > 1 ||
      dli->UB->Num_Vec() > 1) {
    return -1;
  }
  ACCESS_VECTOR *ub = dli->UB->Dim(0);
  ACCESS_VECTOR *lb = dli->LB->Dim(0);
   
  MEM_POOL_Push(&LNO_local_pool);
  ACCESS_VECTOR *sum = Add(lb,ub,&LNO_local_pool);
  if (sum->Is_Const()) {
    rval = sum->Const_Offset >= 0 ? (sum->Const_Offset + stepsz)/stepsz : 0;
  }
  MEM_POOL_Pop(&LNO_local_pool);
  return rval;
}




// Return TRUE if eliminated something
static BOOL Eliminate_Dead_SCF_rec(WN *wn, 
				   void Remove_Region(WN *), 
				   COND_BOUNDS_INFO *info,
				   LABEL_LIST* label_list)
{
  BOOL result = FALSE;
  OPCODE opcode = WN_opcode(wn);
  if (opcode == OPC_BLOCK) {
    WN *kid = WN_first(wn);
    while (kid) {
      WN *next = WN_next(kid);
      if (WN_operator(kid) == OPR_LABEL
	  && label_list->Label_Is_Targeted_Outside_Scope(kid))
	return result; 
      if (kid && WN_opcode(kid)==OPC_DO_LOOP 
	  && Iters(kid)==1
	  && !Do_Loop_Is_Mp(kid)
          && !Is_Nested_Doacross(kid)) {
        WN *first, *last;
        Remove_Unity_Trip_Loop(kid,TRUE,&first,&last,Array_Dependence_Graph,
				Du_Mgr);
	WN *tmp=first;
	while (tmp && tmp!= next) {
	  WN *next_tmp = WN_next(tmp);
          result |= Eliminate_Dead_SCF_rec(tmp, Remove_Region, info, 
	    label_list);
	  tmp = next_tmp;
	}
	result = TRUE; 
      } else {
        result |= Eliminate_Dead_SCF_rec(kid, Remove_Region, info,
	  label_list);
      }
      kid = next;
    }
  } else if (opcode == OPC_REGION && Is_Mp_Region(wn)) {
    WN* wn_start = WN_first(WN_region_body(wn));
    WN* wnn = 0;
    for (wnn = wn_start; wnn != NULL; wnn = WN_next(wnn))
      if (WN_operator(wnn) == OPR_DO_LOOP)
	break;
    if (wnn != NULL)
      result |= Eliminate_Dead_Do(wnn, Remove_Region, info, label_list); 
  } else if (opcode == OPC_DO_LOOP) {
    result |= Eliminate_Dead_Do(wn, Remove_Region, info, label_list);
  } else if (opcode == OPC_IF) {
    result |= Eliminate_Dead_If(wn, Remove_Region, info, label_list);
  } else if (OPCODE_operator(opcode) == OPR_STID) {  // eliminate dead stores
    USE_LIST *uses = Du_Mgr->Du_Get_Use(wn);
    if (uses && !uses->Incomplete() && uses->Is_Empty()) { //dead
      label_list->Remove_Tree(wn);
      Remove_Region(wn);
    }
  } else if (opcode == OPC_GOTO) {
    // no-op gotos
    WN *nxt = WN_next(wn);
    if (nxt && WN_opcode(nxt) == OPC_LABEL) {
      if (WN_label_number(wn) == WN_label_number(nxt)) {
	result = TRUE;
        label_list->Remove_Tree(wn);
	Remove_Region(wn);
      }
    }
  } else if (opcode == OPC_TRUEBR) {
    WN *kid = WN_kid0(wn);
    if (WN_operator(kid) == OPR_INTCONST &&
	WN_const_val(kid) == 0) {
      result = TRUE;
      label_list->Remove_Tree(wn);
      Remove_Region(wn);
    }
  } else if (opcode == OPC_FALSEBR) {
    WN *kid = WN_kid0(wn);
    if (WN_operator(kid) == OPR_INTCONST &&
	WN_const_val(kid) == 1) {
      result = TRUE;
      label_list->Remove_Tree(wn);
      Remove_Region(wn);
    }
  } else if (OPCODE_is_scf(opcode)) {
    for (INT kidno=0; kidno<WN_kid_count(wn); kidno++) {
      WN *kid = WN_kid(wn,kidno);
      result |= Eliminate_Dead_SCF_rec(kid,Remove_Region, info,
	label_list);
    }
  }
  return result;
}

// Given an MP do loop, delete it and its mp-region
static void Delete_MP_Region (WN* do_wn, 
			      void Remove_Region(WN *),
			      LABEL_LIST* label_list) {
  Is_True (do_wn && WN_operator(do_wn) == OPR_DO_LOOP &&
           Do_Loop_Is_Mp(do_wn),
           ("Delete_MP_Region: must be called with an MP do-loop"));

  WN *region=LWN_Get_Parent(LWN_Get_Parent(do_wn));
  WN *region_parent = LWN_Get_Parent(region);
  WN *body = WN_region_body(region);
  WN *tmp = WN_first(body);
  WN *next_tmp;
  while (tmp) {
    next_tmp = WN_next(tmp);
    if ((WN_opcode(tmp) != OPC_PRAGMA) ||
        !(WN_pragmas[WN_pragma(tmp)].users & PUSER_MP)) {
      LWN_Insert_Block_Before(region_parent,region,
                              LWN_Extract_From_Block(tmp));
    }
    tmp = next_tmp;
  }
  label_list->Remove_Tree(region);
  Remove_Region(region);
}


// Return TRUE if eliminated something
static BOOL Eliminate_Dead_Do(WN *do_wn,
			      void Remove_Region(WN *),
			      COND_BOUNDS_INFO *info,
			      LABEL_LIST* label_list)
{
  BOOL result = FALSE;
  INT le = info->Bounds().Num_Le_Constraints();
  INT eq = info->Bounds().Num_Eq_Constraints();
  INT c  = info->Symbol_Info().Elements();
  DYN_ARRAY<WN*> kill_array(&LNO_local_pool);
  for (INT i = 0; i < c; i++) {
    WN* wn = info->Symbol_Info().Bottom_nth(i).Outer_Nondef;
    kill_array.AddElement(wn);
  }
  info->Collect_Do_Info(do_wn);


  if (!info->Bounds().Is_Consistent()) { // do will not execute
    if (Get_Trace(TP_LNOPT,TT_LNO_DEAD)) {
      fprintf(TFile,"Do will not execute \n");
      Dump_WN(do_wn, TFile, 3);
      info->Print(TFile);
    }

    DO_LOOP_INFO* dli = Get_Do_Loop_Info(do_wn); 
    if (!Is_Nested_Doacross(do_wn)) { 

      // Check if this is an MP loop. If so, remove the pragmas
      if (Do_Loop_Is_Mp(do_wn)) {
	Delete_MP_Region (do_wn, Remove_Region, label_list);
      }

      WN *lb;
      if ( Index_Variable_Live_At_Exit(do_wn)) { // replace with lower bound
	lb = LWN_Copy_Tree(WN_start(do_wn),TRUE,LNO_Info_Map);

	// copy the du of the rhs of the assignment
	LWN_Copy_Def_Use(WN_kid0(WN_start(do_wn)),WN_kid0(lb),Du_Mgr);
	// copy the du of the lhs of the assignment
	USE_LIST *uses = Du_Mgr->Du_Get_Use(WN_start(do_wn));
	Is_True(uses,("Live variable but no uses "));
	USE_LIST_ITER iter(uses);
	for (const DU_NODE *node=iter.First(); 
			  !iter.Is_Empty(); node=iter.Next()){
	  WN *use = (WN *) node->Wn();
	  Du_Mgr->Add_Def_Use(lb,use);
	}
	if (uses->Incomplete()) {
	  USE_LIST *lb_uses = Du_Mgr->Du_Get_Use(lb);
	  lb_uses->Set_Incomplete();
	}

	LWN_Insert_Block_After(LWN_Get_Parent(do_wn),do_wn,lb);
      }
      result = TRUE;
      // delete do loop
      label_list->Remove_Tree(do_wn);
      Remove_Region(do_wn);

    } 
  } else {
    if (Get_Trace(TP_LNOPT,TT_LNO_DEAD)) {
      fprintf(TFile,"Do will execute \n");
    }
    result |= Eliminate_Dead_SCF_rec(WN_do_body(do_wn),Remove_Region,info,
      label_list);

    // Get rid of do if empty and if index variable is dead on exit
    if ((WN_first(WN_do_body(do_wn)) == NULL)) {
      BOOL cant_eliminate = FALSE; 
      if (Index_Variable_Live_At_Exit(do_wn)) {
        if (!Upper_Bound_Standardize(WN_end(do_wn), TRUE))
          cant_eliminate = TRUE; 
        else 
	  Finalize_Index_Variable(do_wn, TRUE);
      }
      if (!cant_eliminate) { 
	if (Get_Trace(TP_LNOPT,TT_LNO_DEAD)) {
	  fprintf(TFile,"Do is empty\n");
	}

	if (Do_Loop_Is_Mp(do_wn)) {
	  Delete_MP_Region (do_wn, Remove_Region, label_list);
	}

	result = TRUE;
        label_list->Remove_Tree(do_wn);
	Remove_Region(do_wn);
      }
    }
  }
  info->Reset_Bounds_To(le, eq, c, &kill_array);
  return result;
}

// Return TRUE if eliminated something
static BOOL Eliminate_Dead_If(WN *if_wn,
			      void Remove_Region(WN *),
			      COND_BOUNDS_INFO *info,
			      LABEL_LIST* label_list)
{
  BOOL result = FALSE;
  WN *kid;
  INT le = info->Bounds().Num_Le_Constraints();
  INT eq = info->Bounds().Num_Eq_Constraints();
  INT c  = info->Symbol_Info().Elements();
  DYN_ARRAY<WN*> kill_array(&LNO_local_pool);
  for (INT i = 0; i < c; i++) {
    WN* wn = info->Symbol_Info().Bottom_nth(i).Outer_Nondef;
    kill_array.AddElement(wn);
  }

  info->Collect_If_Info(if_wn, TRUE);
  if (!info->Bounds().Is_Consistent()  // then is inconsistent
      && !label_list->Has_Targeted_Label(WN_then(if_wn))) { 
    if (Get_Trace(TP_LNOPT,TT_LNO_DEAD)) {
      fprintf(TFile,"then is inconsistent \n");
      Dump_WN(if_wn, TFile, 3);
      info->Print(TFile);
    }
    info->Reset_Bounds_To(le, eq, c, &kill_array);

    // recurse on else
    WN *else_block_wn = WN_else(if_wn);
    Eliminate_Dead_SCF_rec(else_block_wn, Remove_Region, info,
      label_list);

    // get rid of if since we know the else is taken, 
    // move else statments to a position preceding if
    WN *parent = LWN_Get_Parent(if_wn);
    while ((kid = WN_last(else_block_wn)) != NULL) {
      LWN_Insert_Block_After(parent,if_wn,LWN_Extract_From_Block(kid));
    }
    result = TRUE;
    label_list->Remove_Tree(if_wn);
    Remove_Region(if_wn);

    info->Reset_Bounds_To(le, eq, c, &kill_array);
  } else { // then is consistent 
    if (Get_Trace(TP_LNOPT,TT_LNO_DEAD)) {
      fprintf(TFile,"then is consistent \n");
    }
    // recurse on then
    WN *then_block_wn = WN_then(if_wn);
    result |= Eliminate_Dead_SCF_rec(then_block_wn, Remove_Region, info,
      label_list); 
    info->Reset_Bounds_To(le, eq, c, &kill_array);

    info->Collect_If_Info(if_wn, FALSE);
    if (!info->Bounds().Is_Consistent() // else is inconsistent
        && !label_list->Has_Targeted_Label(WN_else(if_wn))) { 
      if (Get_Trace(TP_LNOPT,TT_LNO_DEAD)) {
        fprintf(TFile,"else is inconsistent \n");
        Dump_WN(if_wn, TFile, 3);
        info->Print(TFile);
      }
      // get rid of all the statements in the else block
      WN *parent = LWN_Get_Parent(if_wn);
      while ((kid = WN_last(then_block_wn)) != NULL) {
        LWN_Insert_Block_After(parent,if_wn,LWN_Extract_From_Block(kid));
      }
      result = TRUE;
      label_list->Remove_Tree(if_wn);
      Remove_Region(if_wn);
    } else { // else is consistent 
      if (Get_Trace(TP_LNOPT,TT_LNO_DEAD)) {
        fprintf(TFile,"else is consistent \n");
      }

      // recurse on else
      result |= Eliminate_Dead_SCF_rec(WN_else(if_wn), Remove_Region, info,
        label_list);

      // check for a NULL if statement
      if (!WN_first(WN_else(if_wn)) && !WN_first(WN_then(if_wn))) {
	label_list->Remove_Tree(if_wn);
	Remove_Region(if_wn);
        if (Get_Trace(TP_LNOPT,TT_LNO_DEAD)) {
          fprintf(TFile,"if is empty\n");
        }
      }
    }
    info->Reset_Bounds_To(le, eq, c, &kill_array);
  }
  return result;
} 

static void Mark_Dos(WN *do_wn, HASH_TABLE<WN *,BOOL> *htable);
static void Guard_Dos_Rec(WN *do_wn, HASH_TABLE<WN *,BOOL> *htable);
static WN *Highest_Guard_Point(WN *do_wn, DO_LOOP_INFO *dli);
static WN *Highest_Condition_Point(WN *highest_wn, INT non_const_loops);

// Make sure that DO loops have at least one iteration
// Doesn't update anything
// Two step process, in step 1 we use a hash table to mark which DOs need
// to be guarded.  In step 2 we guard all such DOs.  We use a two step
// process so that we don't waste time using the guards from
// previously processed DOs to decide  if a new DO needs to be guarded
// (the conditions on the guards are competely redundant given the bounds)
void Guard_Dos(WN *func_nd)
{
  MEM_POOL_Push(&LNO_local_pool);
  // add scope for dealloc before pop
  {
    HASH_TABLE<WN *,BOOL> htable(200,&LNO_local_pool);
    Mark_Dos(func_nd,&htable);
    Guard_Dos_Rec(func_nd,&htable);
  }
  MEM_POOL_Pop(&LNO_local_pool);
}

// Mark all the dos that need to be guarded
static void Mark_Dos(WN *wn, HASH_TABLE<WN *,BOOL> *htable)
{
  OPCODE opcode = WN_opcode(wn);
  if (opcode == OPC_BLOCK) {
    WN *kid = WN_first(wn);
    while (kid) {
      WN *next = WN_next(kid);
      Mark_Dos(kid,htable);
      kid = next;
    }
  } else {
    for (INT kidno=0; kidno<WN_kid_count(wn); kidno++) {
      WN *kid = WN_kid(wn,kidno);
      Mark_Dos(kid,htable);
    }
    if (opcode == OPC_DO_LOOP) {
     if (!Do_Loop_Is_Mp(wn)) {
      BOOL guard = FALSE;
      MEM_POOL_Push(&LNO_local_pool);
      // if we can't prove it has a non-zero trip count, put in a test
      if (COND_Do_Info(wn,&LNO_local_pool) != COND_DO_AT_LEAST_ONCE) {
	guard = TRUE;
      }
      MEM_POOL_Pop(&LNO_local_pool);
      if (guard) htable->Enter(wn,TRUE);

      // set up the OPC_LOOP_INFO
      if (WN_kid_count(wn) == 6) { // has a loop info
	WN *trip_count = WN_LOOP_TripCount(wn);
        if (trip_count) {
          LWN_Parentize(trip_count);
          LWN_Set_Parent(trip_count, NULL);
        }
	BOOL const_trip = (trip_count) && 
		(WN_operator(trip_count) == OPR_INTCONST);
        if (!const_trip) LWN_Delete_Tree(trip_count);
	WN *loop_info = WN_do_loop_info(wn);
	if (loop_info && WN_kid_count(loop_info)) {
	  // make sure loop_info has a trip_count kid iff const_trip is TRUE
	  if (!const_trip && (WN_kid_count(loop_info) == 2)) { // delete trip
	    WN *new_loop_info = 
	       LWN_CreateLoopInfo(LWN_Copy_Tree(WN_kid0(loop_info)),
	       NULL,0,0,0);
	    LWN_Delete_Tree(loop_info);
	    WN_set_do_loop_info(wn,new_loop_info);
	    LWN_Set_Parent(new_loop_info,wn);
	    loop_info = new_loop_info;
	  } else if (const_trip && (WN_kid_count(loop_info)==1)){ // add trip
	    WN *new_loop_info = LWN_CreateLoopInfo(
		LWN_Copy_Tree(WN_kid0(loop_info)),trip_count,0,0,0);
	    LWN_Delete_Tree(loop_info);
	    WN_set_do_loop_info(wn,new_loop_info);
	    LWN_Set_Parent(new_loop_info,wn);
	    loop_info = new_loop_info;
          } else if (const_trip) {
	    LWN_Delete_Tree(WN_loop_trip(loop_info));
	    WN_set_loop_trip(loop_info,trip_count);
	    LWN_Set_Parent(trip_count,loop_info);
	  } 
	  // loop_info points to a valid loop info with valid kids
	  // not set the internal fields
          
          // nenad, 02/16/2000:
          // Use Get_Good_Num_Iters instead of Est_Num_Iterations
          // since the latter is arbitrary if it's symbolic.
          extern INT64 Get_Good_Num_Iters (DO_LOOP_INFO *dli);
	  DO_LOOP_INFO *dli = Get_Do_Loop_Info(wn);
          INT64 est = Get_Good_Num_Iters(dli);
	  WN_loop_trip_est(loop_info) = MIN(UINT16_MAX, est);
          if (dli->Num_Iterations_Symbolic) {
	    WN_Set_Loop_Symb_Trip(loop_info);
          } else {
	    WN_Reset_Loop_Symb_Trip(loop_info);
          }
	  if (dli->Is_Inner) {
	    WN_Set_Loop_Innermost(loop_info);
          } else {
	    WN_Reset_Loop_Innermost(loop_info);
          }
          INT depth = 1;
	  WN *tmp = LWN_Get_Parent(wn);
	  while (tmp) {
	    if ((WN_opcode(tmp) == OPC_DO_LOOP)  ||
	        (WN_opcode(tmp) == OPC_DO_WHILE) ||
	        (WN_opcode(tmp) == OPC_WHILE_DO)) {
              depth++;
            }
	    tmp = LWN_Get_Parent(tmp);
	  }
	  WN_loop_depth(loop_info) = depth;
	  WN_Set_Loop_Nz_Trip(loop_info);
	  if (dli->Is_Cache_Winddown() || dli->Is_In_Cache_Winddown()) {
	    WN_Set_Loop_Winddown_Cache(loop_info);
          } else {
	    WN_Reset_Loop_Winddown_Cache(loop_info);
          }
	  if (dli->Is_Register_Winddown()||dli->Is_In_Register_Winddown()) {
	    WN_Set_Loop_Winddown_Reg(loop_info);
          } else {
	    WN_Reset_Loop_Winddown_Reg(loop_info);
          }
	  if (dli->Is_Generally_Unimportant()) {
	    WN_Set_Loop_Unimportant_Misc(loop_info);
          } else {
	    WN_Reset_Loop_Unimportant_Misc(loop_info);
          }
	}
      }
     }
     if (WN_kid_count(wn) == 6) { // has a loop info
        WN *do_idname = WN_kid0(wn);
	WN *loop_info = WN_do_loop_info(wn);
        if (WN_kid0(loop_info)) {
          WN *idname = WN_kid0(loop_info);
          // if it doesn't match, erase it
          if ((WN_st(idname) != WN_st(do_idname)) ||
              (WN_offset(idname) != WN_offset(do_idname))) {
            LWN_Delete_Tree(idname);
            WN_kid0(loop_info) = NULL;
	  }
	}

        // if no idname, create one
        if (!WN_kid0(loop_info)) {
          WN_kid0(loop_info) = 
 	     WN_CreateIdname(WN_offset(do_idname),WN_st(do_idname));
          LWN_Set_Parent(WN_kid0(loop_info), loop_info);
        }
     }
    }
  }
}

// Do the actual guarding
static void Guard_Dos_Rec(WN *wn, HASH_TABLE<WN *,BOOL> *htable)
{
  OPCODE opcode = WN_opcode(wn);
  if (opcode == OPC_DO_LOOP) {
    if (htable->Find(wn)) {
      if (!Do_Loop_Is_Mp(wn)) {
        Guard_A_Do(wn);
      }
    }
  }
  if (opcode == OPC_BLOCK) {
    WN *kid = WN_first(wn);
    while (kid) {
      WN *next = WN_next(kid);
      if (!OPCODE_is_expression(WN_opcode(kid)))  // no dos under expressions
        Guard_Dos_Rec(kid,htable);
      kid = next;
    }
  } else {
    for (INT kidno=0; kidno<WN_kid_count(wn); kidno++) {
      WN *kid = WN_kid(wn,kidno);
      if (!OPCODE_is_expression(WN_opcode(kid)))  // no dos under expressions
        Guard_Dos_Rec(kid,htable);
    }
  }
}



// Guard a particular do
extern WN* Guard_A_Do(WN *do_wn)
{
  // find the lower bound of the do
  WN *lower_stid = WN_start(do_wn);
  FmtAssert(WN_operator(lower_stid) == OPR_STID,
	    ("Lower bound of a DO_LOOP is not an STID"));
  WN *lower = WN_kid0(lower_stid);

  DO_LOOP_INFO *dli = Get_Do_Loop_Info(do_wn);

  // the new test is created by subbing in the lower bound for the
  // loop variable in the upper bound
  WN *test = LWN_Copy_Tree(WN_end(do_wn),TRUE,LNO_Info_Map);
  LWN_Copy_Def_Use(WN_end(do_wn),test,Du_Mgr);
  if (Array_Dependence_Graph) {
    if (!Array_Dependence_Graph->Add_Deps_To_Copy_Block(
		WN_end(do_wn),test,FALSE)) {
      LNO_Erase_Dg_From_Here_In(do_wn,Array_Dependence_Graph);
    }
  }
  Replace_Ldid_With_Exp_Copy(SYMBOL(lower_stid), test, lower, Du_Mgr,Array_Dependence_Graph);

  // Create the if
  WN *new_then = WN_CreateBlock();
  WN *new_else = WN_CreateBlock();
  WN *new_if = LWN_CreateIf(test,new_then,new_else);
  WN_Set_If_Guard(new_if);
  dli->Guard = new_if;

  // if the loop variable is live out, put the lower bound in the else clause
  if (Index_Variable_Live_At_Exit(do_wn)) {
    WN *lb = LWN_Copy_Tree(WN_start(do_wn),TRUE,LNO_Info_Map);

    // add the def-use for the RHS of the bound in the else clause
    // it's just a copy of what's in the lower bound.
    LWN_Copy_Def_Use(WN_kid0(WN_start(do_wn)),WN_kid0(lb),Du_Mgr);
    if (Array_Dependence_Graph) {
      if (!Array_Dependence_Graph->Add_Deps_To_Copy_Block(
			WN_kid0(WN_start(do_wn)),WN_kid0(lb),FALSE)) {
        LNO_Erase_Dg_From_Here_In(do_wn,Array_Dependence_Graph);
      }
    }

    // now do the LHS, it's a copy of every use not in the loop
    USE_LIST *uses = Du_Mgr->Du_Get_Use(WN_start(do_wn));
    Is_True(uses,("Live variable but no uses "));
    USE_LIST_ITER iter(uses);
    for (const DU_NODE *node=iter.First(); !iter.Is_Empty(); node=iter.Next()){
      WN *use = (WN *) node->Wn();
      if (!Is_Descendent(use,do_wn)) {
	Du_Mgr->Add_Def_Use(lb,use);
      }
    }
    if (uses->Incomplete()) {
      Du_Mgr->Create_Use_List(lb);
      USE_LIST *lb_uses = Du_Mgr->Du_Get_Use(lb);
      lb_uses->Set_Incomplete();
    }

    // now insert the code
    LWN_Insert_Block_Before(new_else,NULL,lb);
  }

  if (Cur_PU_Feedback) {
      Update_Guarded_Do_FB(new_if, do_wn, Cur_PU_Feedback);
  }

  // Insert the if as high as possible
  WN *highest_wn = do_wn;
  if (Statically_Safe_Exp(WN_if_test(new_if))) { // can speculate it, 
    WN *wn_block = LWN_Get_Parent(do_wn);
    Is_True(WN_opcode(wn_block) == OPC_BLOCK,("Parent must be block "));
    if (WN_first(wn_block) == WN_last(wn_block)) { // perfectly nested
      highest_wn = Highest_Guard_Point(do_wn,dli);
    }
  }
  if (WN_opcode(highest_wn) != OPC_BLOCK) {  // put the if around the block
    LWN_Insert_Block_Before(LWN_Get_Parent(highest_wn),highest_wn,new_if);
    LWN_Insert_Block_Before(new_then, NULL,LWN_Extract_From_Block(highest_wn));
#ifdef TARG_ST
    // FdF 23/04/2004: Put PRAGMA preceeding the loop inside the guard
    WN *wn = WN_prev(new_if);
    while (wn && WN_operator(wn) == OPR_PRAGMA) {
      WN *prev = WN_prev(wn);
      LWN_Insert_Block_After(WN_then(new_if),NULL,LWN_Extract_From_Block(wn));
      wn = prev;
    }
#endif
  } else { // put the if in the block, moving everything else into if
    LWN_Insert_Block_Before(highest_wn,NULL,new_if);
    WN *wn = WN_first(highest_wn);
    while (wn != new_if) {
      WN *next = WN_next(wn);
      LWN_Insert_Block_After(WN_then(new_if),NULL,LWN_Extract_From_Block(wn));
      wn = next;
    }
  }

  // annotate the if
  IF_INFO *ii = CXX_NEW(IF_INFO(&LNO_default_pool,TRUE,
                        Find_SCF_Inside(new_if,OPC_REGION)!=NULL),
                        &LNO_default_pool);
  WN_MAP_Set(LNO_Info_Map,new_if,(void *)ii);
  DOLOOP_STACK* stack = CXX_NEW(DOLOOP_STACK(&LNO_local_pool),
				&LNO_local_pool);
  Build_Doloop_Stack(new_if, stack);
  LNO_Build_If_Access(new_if, stack);
  CXX_DELETE(stack, &LNO_local_pool);
  return new_if; 
}

// if_wn is the guard for do_wn
// return the highest place we can put the guard around
// we can keep placing it higher if the condition is invariant in the higher
// region and if things are perfectly nested
static WN *Highest_Guard_Point(WN *do_wn, DO_LOOP_INFO *dli)
{
  // check bounds to see where condition for guard is written
  // we look at the bounds rather than the if since we don't build
  // access vectors for the if
  INT depth = Do_Loop_Depth(do_wn);
  ACCESS_ARRAY *lb_array = dli->LB;
  ACCESS_ARRAY *ub_array = dli->UB;
  if (lb_array->Too_Messy || ub_array->Too_Messy) {
    return(do_wn);
  }

  // set non_const_loops = the number of loops in which the condition doesn't
  // vary
  // first look at the symbol terms
  INT non_const_loops = 
    MAX(lb_array->Non_Const_Loops(),ub_array->Non_Const_Loops());
  // now check triangular components
  INT i;
  for (i=0; i<lb_array->Num_Vec(); i++) {
    ACCESS_VECTOR *lb = lb_array->Dim(i);
    if (lb->Too_Messy) return(do_wn);
    for (INT j=non_const_loops; j<depth; j++) {
      if (lb->Loop_Coeff(j)) {
	non_const_loops = j+1;
      }
    }
  }
  for (i=0; i<ub_array->Num_Vec(); i++) {
    ACCESS_VECTOR *ub = ub_array->Dim(i);
    if (ub->Too_Messy) return(do_wn);
    for (INT j=non_const_loops; j<depth; j++) {
      if (ub->Loop_Coeff(j)) {
	non_const_loops = j+1;
      }
    }
  }

  // nenad, 02/15/2000:
  // Sometimes, when DU chains are incomplete, non_const_loops
  // may be incorrect. To avoid the problem reported in 780055,
  // we examine all dependence edge incident on the loop guard.

  ARRAY_DIRECTED_GRAPH16* dg = Array_Dependence_Graph;
  if (dg) {
    INT ncl = non_const_loops;
    WN_ITER *wni;
    for (wni=WN_WALK_TreeIter(WN_end(do_wn)); wni; wni=WN_WALK_TreeNext(wni)) {
      WN* ldid = WN_ITER_wn(wni);
      if (WN_operator(ldid) == OPR_LDID) {
        VINDEX16 v = dg->Get_Vertex(ldid);
        if (v) {
          EINDEX16 e;
          for (e = dg->Get_Out_Edge(v); e; e = dg->Get_Next_Out_Edge(e)) {
            WN* sink = dg->Get_Wn(dg->Get_Sink(e));
            INT d = Do_Loop_Depth(Enclosing_Do_Loop(sink)) + 1;
            non_const_loops = MAX(non_const_loops, d);
          }
          for (e = dg->Get_In_Edge(v); e; e = dg->Get_Next_In_Edge(e)) {
            WN* source = dg->Get_Wn(dg->Get_Source(e));
            INT d = Do_Loop_Depth(Enclosing_Do_Loop(source)) + 1;
            non_const_loops = MAX(non_const_loops, d);
          }
        }
      }
    } 
    if (ncl != non_const_loops) {
      DevWarn(("non_const_loops changed from %d to %d"), ncl, non_const_loops);
    }
  }

  return (Highest_Condition_Point(do_wn,non_const_loops));
}

// given some if statement that can legally be placed surrounding input_wn
// given that the condition for the if varies in the non_const_loops outermost
// loops (non_const_loops == 0 implies that it doesn't vary in any do loop)
// return the highest wn we can put the condtion around
// (we can keep putting it higher if things are pefectly nested and if
// the condition doesn't vary in the higher region)
static WN *Highest_Condition_Point(WN *input_wn, INT non_const_loops)
{
  BOOL seen_do = FALSE;

  WN *parent_wn=LWN_Get_Parent(input_wn);
  WN *wn  = input_wn;
  BOOL done = FALSE;
  OPCODE opcode;
  while (!done) {
    opcode = WN_opcode(parent_wn);
    if (opcode == OPC_BLOCK) {
      if (WN_first(parent_wn) != WN_last(parent_wn)) {  // not perfectly nested
	done = TRUE;
      } 
    } else if (opcode == OPC_IF) {
      if (WN_first(WN_else(parent_wn)) != NULL) { // not perfectly nested
	done = TRUE;
      }
    } else if (opcode == OPC_DO_LOOP) { // can't put the if outside a loop
					// with a live index variable
      if (Index_Variable_Live_At_Exit(parent_wn)) {
	done = TRUE;
      } else if (Do_Loop_Is_Mp(parent_wn)) {
	done = TRUE;
      }
    } else if ((opcode != OPC_DO_LOOP) && (opcode != OPC_DO_WHILE) && 
		(opcode != OPC_WHILE_DO)) {  // not perfectly nested
      done = TRUE;
    }
    if (WN_opcode(wn) == OPC_DO_LOOP) {
      if (Do_Loop_Depth(wn) < non_const_loops) {  
	return input_wn;  // we've gone to far, could't really move it
      } else if (Do_Loop_Depth(wn) == non_const_loops) {  
        // we can place it ouside of this do, but not any higher
        done = TRUE;
      }
      seen_do = TRUE;
    }
    if (!done) {
      wn = parent_wn;
      parent_wn = LWN_Get_Parent(parent_wn);
    }
  }
  if (!seen_do) return input_wn;
  return wn;
}

static INT Non_Const_Loops(WN *expr);

// Hoist loop invariant conditionals outside of loops
// Given:
// do ...
//  if ...
// if the if is invariant and things are perfectly nested, we interchange
// the if and the do and hoist the if as high up as possible
BOOL Hoist_Conditionals(WN *wn)
{
  BOOL result = FALSE;
  OPCODE opcode = WN_opcode(wn);
  if (opcode == OPC_BLOCK) {
    WN *kid = WN_first(wn);
    while (kid) {
      WN *next = WN_next(kid);
      result |= Hoist_Conditionals(kid);
      kid = next;
    }
  } else {
    for (INT kidno=0; kidno<WN_kid_count(wn); kidno++) {
      WN *kid = WN_kid(wn,kidno);
      result |= Hoist_Conditionals(kid);
    }
  }

  if ((opcode == OPC_DO_LOOP) && !Do_Loop_Is_Mp(wn)) {
    WN *block_wn = WN_do_body(wn);
    WN *if_wn = WN_first(block_wn);
    if (if_wn && (WN_opcode(if_wn) == OPC_IF) && (if_wn == WN_last(block_wn))) {
      if (!WN_first(WN_else(if_wn))) {  // can't hoist if there are elses
        if (Statically_Safe_Exp(WN_if_test(if_wn))) { // can speculate it, 
	  INT non_const_loops=Non_Const_Loops(WN_if_test(if_wn));
	  if (non_const_loops != -1) {
	    WN *new_pos = Highest_Condition_Point(if_wn, non_const_loops);
	    if (new_pos != if_wn) {  // we can move it higher
	      result = TRUE;
	      // move the body of the then clause into the do
	      WN *wn = WN_first(WN_then(if_wn));
	      while (wn) {
	        WN *next = WN_next(wn);
	        LWN_Insert_Block_Before(block_wn,if_wn,
			LWN_Extract_From_Block(wn));
                wn = next;
              }

	      // now move the if to the appropriate place
	      LWN_Extract_From_Block(if_wn); 
	      LWN_Copy_Frequency(if_wn,new_pos);
	      LWN_Copy_Frequency_Tree(WN_if_test(if_wn),new_pos);
	      if (WN_opcode(new_pos) != OPC_BLOCK) {
		// put the if above new_pos
                LWN_Insert_Block_Before(LWN_Get_Parent(new_pos),new_pos,if_wn);
                LWN_Insert_Block_Before(WN_then(if_wn), NULL,
			LWN_Extract_From_Block(new_pos));
               } else { // put the if inside the block, put others in then
                 LWN_Insert_Block_Before(new_pos,NULL,if_wn);
    		 WN *wn = WN_first(new_pos);
                 while (wn != if_wn) {
                   WN *next = WN_next(wn);
                   LWN_Insert_Block_After(WN_then(if_wn),NULL,
					LWN_Extract_From_Block(wn));
      	           wn = next;
                 }
	       }
	       // update the if_info
               IF_INFO* ii = Get_If_Info(if_wn, TRUE);
	       ii->Contains_Do_Loops = TRUE;
	    }
          }
	}
      }
    }
  }
  return result;
}

// in what region is an expression invariant
// return 0 if the expression is invariant in all surrounding DO loops
// return 1 if all but 1, etc
// return -1 if we don't know
static INT Non_Const_Loops(WN *expr)
{
  int count=0;
  int depth;
  BOOL first_time=TRUE;
  WN *loops = LWN_Get_Parent(expr);
  while (loops) {
    if (WN_operator(loops) == OPR_DO_LOOP) {
      if (first_time) {
	depth = Do_Loop_Depth(loops);
        first_time = FALSE;
      }
      if (Is_Loop_Invariant_Exp(expr,loops)) {
	count++;
      } else {
	if (count == 0) {
	  return -1;
        } else {
	  return depth+1-count;
	}
      }
    }
    loops = LWN_Get_Parent(loops);
  }
  return depth-count+1;
}

// is av possibly consistent inside expr
// conservative to return YES
BOOL Is_Consistent_Condition(ACCESS_VECTOR *av, WN *expr)
{
  Is_True(OPCODE_is_expression(WN_opcode(expr)),
	  ("Non-expression in Is_Consistent \n"));
  if (!OPCODE_is_expression(WN_opcode(expr))) return TRUE;
  if (av->Too_Messy || av->Contains_Non_Lin_Symb()) return TRUE;

  BOOL answer;
  MEM_POOL_Push(&LNO_local_pool);
  {
    COND_BOUNDS_INFO info(&LNO_local_pool);
    WN *statement = LWN_Get_Statement(expr);
    info.Collect_Outer_Info(statement);
    info.Add_Access(av,expr,statement);
    if (info.Bounds().Is_Consistent()) {
      answer = TRUE;
    } else {
      answer = FALSE;
    }
  }
  MEM_POOL_Pop(&LNO_local_pool);
  return answer;
}

//-----------------------------------------------------------------------
// NAME: STD_Canonicalize_Upper_Bound
// FUNCTION: Canonicalize the upper bound of the loop 'wn_loop' by 
//   changing the relational operator to LT.  If we convert from LT 
//   to LE, add a guard test before doing the conversion.  
//-----------------------------------------------------------------------

static void STD_Canonicalize_Upper_Bound(WN* wn_loop)
{
  if (Do_Loop_Is_Mp(wn_loop))
    return; 
  OPCODE opc = WN_opcode(WN_end(wn_loop)); 
  OPERATOR opr = OPCODE_operator(opc);
  if (UBvar(WN_end(wn_loop)) == NULL)
    return; 
  if (opr == OPR_GT || opr == OPR_GE) {
    WN* lhs = WN_kid0(WN_end(wn_loop));
    WN* rhs = WN_kid1(WN_end(wn_loop)); 
    WN_kid0(WN_end(wn_loop)) = rhs; 
    WN_kid1(WN_end(wn_loop)) = lhs; 
    OPERATOR opr_inv = opr == OPR_GT ? OPR_LT : OPR_LE; 
    OPCODE opc_inv = OPCODE_make_op(opr_inv, OPCODE_rtype(opc), 
      OPCODE_desc(opc)); 
    WN_set_opcode(WN_end(wn_loop), opc_inv); 
  } 
  if (WN_operator(WN_end(wn_loop)) == OPR_LT) {
    if (COND_Do_Info(wn_loop, &LNO_local_pool) != COND_DO_AT_LEAST_ONCE)
      Guard_A_Do(wn_loop);
    TYPE_ID desc = WN_desc(WN_end(wn_loop));
    WN_set_opcode(WN_end(wn_loop), 
      OPCODE_make_op(OPR_LE, OPCODE_rtype(opc), desc));
    OPCODE subop = OPCODE_make_op(OPR_SUB, desc, MTYPE_V);
    WN* ub1 = LWN_CreateExp2(subop, WN_kid1(WN_end(wn_loop)), 
      LWN_Make_Icon(desc, 1));
    WN_kid1(WN_end(wn_loop)) = ub1;
    LWN_Copy_Frequency_Tree(ub1, WN_end(wn_loop));
    LWN_Set_Parent(ub1, WN_end(wn_loop));
  } 
}

//-----------------------------------------------------------------------
// NAME: STD_Traverse 
// FUNCTION: Traverse the tree rooted at 'wn_tree', canonicalizing all 
//   of the unsigned do loops in the tree. 
//-----------------------------------------------------------------------

static void STD_Traverse(WN* wn_tree) 
{ 
  if (WN_opcode(wn_tree) == OPC_DO_LOOP && Do_Loop_Is_Unsigned(wn_tree))
    STD_Canonicalize_Upper_Bound(wn_tree);  
  if (WN_opcode(wn_tree) == OPC_BLOCK) {
    for (WN* wn = WN_first(wn_tree); wn != NULL; wn = WN_next(wn))
      STD_Traverse(wn);
  } else {
    for (INT i = 0; i < WN_kid_count(wn_tree); i++)
      STD_Traverse(WN_kid(wn_tree, i));
  }
}

//-----------------------------------------------------------------------
// NAME: Canonicalize_Unsigned_Loops
// FUNCTION: Put the upper bounds of all loops with unsigned induction 
//   variables in the form 'index_variable .LE. expression'.  Add guard
//   tests in the case where we convert from .LT. to .LE. 
//-----------------------------------------------------------------------

extern void Canonicalize_Unsigned_Loops(WN* func_nd)
{
  STD_Traverse(func_nd);
}

//-----------------------------------------------------------------------
// NAME: Redundant_Condition
// FUNCTION: Accumulate information about the 'wn_cond' in 'info' and
//   return TRUE if 'wn_cond' is redundant with respect to the condition
//   already present under 'wn_if'.
// NOTE: See tile.cxx for an example of how this is used.
//-----------------------------------------------------------------------

extern BOOL Redundant_Condition(COND_BOUNDS_INFO* info,
                                WN* wn_cond,
                                WN* wn_if)
{

  // Save the parent of wn_cond
  WN *wn_cond_parent = LWN_Get_Parent(wn_cond);

  // Save the old state of the 'info'
  INT le = info->Bounds().Num_Le_Constraints();
  INT eq = info->Bounds().Num_Eq_Constraints();
  INT c  = info->Symbol_Info().Elements();
  DYN_ARRAY<WN*> kill_array(&LNO_local_pool);
  for (INT i = 0; i < c; i++) {
    WN* wn = info->Symbol_Info().Bottom_nth(i).Outer_Nondef;
    kill_array.AddElement(wn);
  }

  // Temporarily replace the if test with the new condition and compute
  // its access array.
  WN* wn_old_cond = WN_if_test(wn_if);
  WN_if_test(wn_if) = wn_cond;
  LWN_Set_Parent(wn_cond, wn_if);
  DOLOOP_STACK stack1(&LNO_local_pool);
  Build_Doloop_Stack(wn_if, &stack1);
  LNO_Build_If_Access(wn_if, &stack1);

  // Determine if the new condition is redundant.
  IF_INFO* ii = Get_If_Info(wn_if, TRUE);
  if (ii->Condition->Num_Vec() == 1) {
    ACCESS_VECTOR av_cond(ii->Condition->Dim(0), &LNO_local_pool);
    av_cond.Negate_Me();
    av_cond.Const_Offset--;
    info->Add_Access(&av_cond, wn_cond, wn_if);
    BOOL return_value = !info->Bounds().Is_Consistent();
    info->Reset_Bounds_To(le, eq, c, &kill_array);
    WN_if_test(wn_if) = wn_old_cond;
    LWN_Set_Parent(wn_old_cond, wn_if);
    LWN_Set_Parent(wn_cond, wn_cond_parent);
    if (return_value)
      return TRUE;
  }

  // Since the new condition is not redundant, accumulate it in 'info'
  info->Collect_If_Info(wn_if, TRUE);
  WN_if_test(wn_if) = wn_old_cond;
  LWN_Set_Parent(wn_old_cond, wn_if);
  LWN_Set_Parent(wn_cond, wn_cond_parent);
  return FALSE;
}

extern void Update_Guarded_Do_FB(WN *if_wn, WN *do_wn, FEEDBACK *feedback)
{
  Is_True(if_wn && WN_operator(if_wn) == OPR_IF, ("bad if_wn"));
  Is_True(do_wn && WN_operator(do_wn) == OPR_DO_LOOP, ("bad do_wn"));
  FB_Info_Loop loop_freq = feedback->Query_loop(do_wn);
  FB_Info_Branch if_freq(loop_freq.freq_positive, loop_freq.freq_zero);
  feedback->Annot_branch(if_wn, if_freq);
    // the guard prevents the 0-tripcount case from happening
  loop_freq.freq_zero = FB_FREQ_ZERO;
  loop_freq.freq_exit = loop_freq.freq_positive;
  feedback->Annot_loop (do_wn, loop_freq);
}
