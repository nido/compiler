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

// GRA_BB implementation functions
/////////////////////////////////////
//
//  Description:
//
//      Whatever wasn't inline ended up here.
//
/////////////////////////////////////



#ifdef USE_PCH
#include "cg_pch.h"
#endif // USE_PCH
#pragma hdrstop

#ifdef _KEEP_RCS_ID
#endif

#include "defs.h"
#include "cg.h"
#include "cg_region.h"
#include "mempool.h"
#include "cgir.h"
#include "bb_set.h"
#include "gra_live.h"
#include "gra_bb.h"
#include "gra_lrange.h"
#include "gra_loop.h"
#include "gtn_universe.h"
#include "gtn_set.h"
#include "gra_region.h"
#include "util.h"
#include "mempool.h"
#include "lra.h"
#include "gra.h"
#include "gra_interfere.h"
#include "gra_trace.h"
#ifdef TARG_ST
#include "gra_color.h"
#include "cg_flags.h"
#endif

GBB_MGR gbb_mgr;

#ifdef TARG_ST
static INT subclass_size[ISA_REGISTER_SUBCLASS_MAX_LIMIT+1];
#endif

/////////////////////////////////////
INT 
GRA_BB::Register_Girth( ISA_REGISTER_CLASS rc ) 
{
#ifdef TARG_ST
  // We get the information from LRA for the estimate of the pressure.
  // The returned value must be bounded by the allocatable register size.
  // Note that this estimate is not more corrected by local forced max.
  // This is done in a second step if necessary
  LRA_Request_Info request_info;
  LRA_Register_Request (bb, &request_info);
  INT rr = request_info.summary[rc];
  INT rs = REGISTER_SET_Size(REGISTER_CLASS_allocatable(rc));
  // This should not be necessary anymore. But removing it
  // may make performance vary with older versions.
  // So for now we are forced to keep it.
  if ( rr < Forced_Locals(rc) && rr != 0 )
    rr = Forced_Locals(rc);
  return rr <= rs ? rr : rs; 
#else
  INT rr = LRA_Register_Request(bb,rc);
  INT rs = REGISTER_SET_Size(REGISTER_CLASS_allocatable(rc));
  if ( rr < GRA_local_forced_max && rr != 0 )
    rr = GRA_local_forced_max;
  return rr <= rs ? rr : rs; 
#endif
}

/////////////////////////////////////
// Does this BB come from the complement region?
BOOL
GRA_BB::Region_Is_Complement(void)
{
  return region == gra_region_mgr.Complement_Region();
}

/////////////////////////////////////
// Add <lunit> to <gbb>.  See the iterator GRA_BB_RC_LUNIT_ITER.
void
GRA_BB::Add_LUNIT( LUNIT*  lunit)
{
  ISA_REGISTER_CLASS rc = lunit->Lrange()->Rc();
  lunits[rc] = lunits[rc]->BB_List_Push(lunit);
}

/////////////////////////////////////
// Add <reg> to the set of registers used in the given <gbb> and <rc>.
void
GRA_BB::Make_Register_Used( ISA_REGISTER_CLASS  rc, REGISTER reg)
{
  region-> Make_Register_Used(rc,reg);
  loop->Make_Register_Used(rc, reg);
  registers_used[rc] = REGISTER_SET_Union1(registers_used[rc],reg);
}
#ifdef TARG_ST

/////////////////////////////////////
// Add [<reg>..<reg>+<nregs>-1] to the set of registers used in the
// given <gbb> and <rc>.
void
GRA_BB::Make_Registers_Used (ISA_REGISTER_CLASS rc, REGISTER reg, INT nregs)
{
  for (INT i = 0; i < nregs; i++) {
    Make_Register_Used (rc, reg + i);
  }
}
#endif

/////////////////////////////////////
REGISTER_SET
GRA_BB::Registers_Used( ISA_REGISTER_CLASS  rc)
{
  if ( region == gra_region_mgr.Complement_Region() )
    return registers_used[rc];
  else {
    REGISTER_SET used = region->Registers_Used(rc);

    if ( region->Has_Call() )
      return REGISTER_SET_Union(used,REGISTER_CLASS_caller_saves(rc));
    else
      return used;
  }
}

/////////////////////////////////////
// Check if <lrange> in the set of LRANGEs to be spilled above <gbb>.
BOOL
GRA_BB::Spill_Above_Check( LRANGE* lrange )
{
  ISA_REGISTER_CLASS rc = lrange->Rc();
  if (spill_above[rc]) {
    LRANGE_SUBUNIVERSE *su = region->Subuniverse(rc);
    return(LRANGE_SET_MemberPS(spill_above[rc], lrange, su));
  }
  return FALSE;
}

/////////////////////////////////////
// Add <lrange> to the set of LRANGEs to be spilled above <gbb>.
void
GRA_BB::Spill_Above_Set( LRANGE* lr )
{
  ISA_REGISTER_CLASS rc = lr->Rc();
  LRANGE_SUBUNIVERSE *su = region->Subuniverse(rc);
  INT size = su->Count();

  if (!spill_above[rc]) {
    spill_above[rc] = LRANGE_SET_Create_Empty(size, GRA_pool);
  }
  spill_above[rc] = LRANGE_SET_Union1S(spill_above[rc], lr, GRA_pool, su);
}

/////////////////////////////////////
// Remove <lrange> from the set of LRANGEs to be spilled above <gbb>.
void
GRA_BB::Spill_Above_Reset( LRANGE* lr )
{
  ISA_REGISTER_CLASS rc = lr->Rc();
  LRANGE_SUBUNIVERSE *su = region->Subuniverse(rc);

  spill_above[rc] = LRANGE_SET_Difference1DS(spill_above[rc], lr, su);
}


/////////////////////////////////////
// Check if <lrange> in the set of LRANGEs to be restored at the bottom of <gbb>
BOOL
GRA_BB::Restore_Below_Check( LRANGE* lr )
{
  ISA_REGISTER_CLASS rc = lr->Rc();
  if (restore_below[rc]) {
    LRANGE_SUBUNIVERSE *su = region->Subuniverse(rc);
    return(LRANGE_SET_MemberPS(restore_below[rc], lr, su));
  }
  return FALSE;
}

/////////////////////////////////////
// Add <lrange> to the set of LRANGEs to be restored at the bottom of <gbb>.
void
GRA_BB::Restore_Below_Set( LRANGE* lr )
{
  ISA_REGISTER_CLASS rc = lr->Rc();
  LRANGE_SUBUNIVERSE *su = region->Subuniverse(rc);
  INT size = su->Count();

  if (!restore_below[rc]) {
    restore_below[rc] = LRANGE_SET_Create_Empty(size, GRA_pool);
  }
  restore_below[rc] = LRANGE_SET_Union1S(restore_below[rc], lr,
					    GRA_pool, su);
}

/////////////////////////////////////
// Remove <lrange> from the set of LRANGEs to be restored at the
// bottom of <gbb>.
void
GRA_BB::Restore_Below_Reset( LRANGE* lr )
{
  ISA_REGISTER_CLASS rc = lr->Rc();
  LRANGE_SUBUNIVERSE *su = region->Subuniverse(rc);

  restore_below[rc] = LRANGE_SET_Difference1DS(restore_below[rc], lr, su);
}

/////////////////////////////////////
// Called at the start of each GRA invocation.
void
GBB_MGR::Initialize(void)
{
  split_mark_counter = 0;
  one_set_counter = 0;
  wired_local_count = 0;
  alloc_count = 0;
  map = BB_MAP_Create();
  blocks_with_calls = BB_SET_Create_Empty(PU_BB_Count + 2,GRA_pool);
  blocks_with_rot_reg_clob = BB_SET_Create_Empty(PU_BB_Count + 2,GRA_pool);
#ifdef TARG_ST
  ISA_REGISTER_SUBCLASS sc;
  FOR_ALL_ISA_REGISTER_SUBCLASS (sc) {
    subclass_size[sc] = REGISTER_SET_Size (REGISTER_SUBCLASS_members (sc));
  }
#endif
}


/////////////////////////////////////
// Called at the end of each GRA invocation.
void
GBB_MGR::Finalize(void)
{
  BB_MAP_Delete(map);
  GRA_Trace_Wired_Local_Count(wired_local_count);
}


/////////////////////////////////////
// Create and return a new GRA_BB to encapsulate <gbb> a
// GRA_BB associated with the given region.
GRA_BB*
GBB_MGR::Create(BB* bb, GRA_REGION* region)
{
  ISA_REGISTER_CLASS rc;
  GRA_BB* gbb = TYPE_MEM_POOL_ALLOC(GRA_BB,GRA_pool);

  gbb->bb = bb;
  gbb->region = region;
  gbb->split_mark = -1;
  gbb->one_set_mark = -1;
  gbb->Clear_Flags();
  alloc_count++;

  //  Previously allocated regions use fine grained interference rules and
  //  the blocks play a much smaller role.

  FOR_ALL_ISA_REGISTER_CLASS( rc ) {
    gbb->registers_used[rc] = REGISTER_SET_EMPTY_SET;
    gbb->glue_registers_used[rc] = REGISTER_SET_EMPTY_SET;
    gbb->lunits[rc] = NULL;
    gbb->local_lrange_count[rc] = 0;
    gbb->local_lranges[rc] = NULL;
    gbb->unpreferenced_wired_lranges[rc] = NULL;
    gbb->spill_above[rc] = NULL;
    gbb->restore_below[rc] = NULL;
  }

  BB_MAP_Set(map,bb,(void*) gbb);
  if ( BB_call(bb) ) {
    blocks_with_calls = BB_SET_Union1D(blocks_with_calls,bb,GRA_pool);

    // if call is to setjmp, set setjmp flag in gbb so that live ranges over 
    // it will never be allocated to a callee-saved register; use same means
    // for recognizing setjmp call as in Can_Do_Tail_Calls_For_PU()
    ANNOTATION *callant = ANNOT_Get(BB_annotations(bb), ANNOT_CALLINFO);
    CALLINFO *callinfo = ANNOT_callinfo(callant);
    ST *st = CALLINFO_call_st(callinfo);
    if (st != NULL) {
      const char *name = ST_name(st);
      INT len = strlen(name);
      if ((len >= 6 && strcmp(name + len - 6, "setjmp") == 0) ||
	  (len >= 10 && strcmp(name+len-10, "getcontext") == 0)) 
	gbb->Setjmp_Set();
    }
  }
  if (BB_mod_rotating_registers(bb) || BB_mod_pred_rotating_registers(bb))
    blocks_with_rot_reg_clob = BB_SET_Union1D(blocks_with_rot_reg_clob,bb,GRA_pool);

  return gbb;
}


/////////////////////////////////////
// Create <count> local LRANGEs for registers in <gbb> and
// the given REGISTER_CLASS, <cl>.  See the iterator type
// GRA_BB_RC_LOCAL_LRANGE_ITER, defined below.
#ifdef TARG_ST

// Used to sort the LRA request, so that we can replace
// the leading requests with forced locals.
// Sort order is:
//   1. Smaller numbered register class first.
//   2. Within a register class, more general (i.e. bigger)
//      subclasses first.  As a special case of this, the
//      undefined subclass is always placed before other
//      subclasses.
//   3. Demand appears before a request (to reduce the chance
//      of it being replaced by a forced local).
// Note that a forced local is effectively a demand for
// a register of undefined subclass, so when pruning the LRA request,
// we prefer to prune those.  Perhaps we should prune only
// those ...
static
int compare_request (const void *r1, const void *r2)
{
  const LRA_Request_Element *req1 = (const LRA_Request_Element *)r1;
  const LRA_Request_Element *req2 = (const LRA_Request_Element *)r2;
  if (req1->cl != req2->cl) {
    return (req1->cl - req2->cl);
  }
  if (req1->sc != req2->sc) {
    if (req1->sc == ISA_REGISTER_SUBCLASS_UNDEFINED) {
      return -1;
    } else if (req2->sc == ISA_REGISTER_SUBCLASS_UNDEFINED) {
      return 1;
    } else {
      INT sc1_size = subclass_size[req1->sc];
      INT sc2_size = subclass_size[req2->sc];
      if (sc1_size != sc2_size) {
	return sc2_size - sc1_size;
      } else {
	return req1->sc - req2->sc;
      }
    }
  } else if (req1->demand != req2->demand) {
    return (req1->demand) ? -1 : 1;
  } else {
    return req1 - req2;
  }
}

void
GRA_BB::Create_Local_LRANGEs ()
{
  ISA_REGISTER_CLASS rc;
  ISA_REGISTER_SUBCLASS sc;
  INT nregs;

  if (GRA_use_subclass_register_request) {
    LRA_Request_Info request;

    LRA_Register_Request (Bb (), &request);

    // // Adjust the request to take account of forced locals.

    // Ensure we request at least Register_Girth registers,
    // to keep compatibility with non-use_subclass_register_request
    // version of code.
    // SC: seems bizarre to me, since register_girth can bump up
    // the local reg request beyond the real BB register pressure.
    FOR_ALL_ISA_REGISTER_CLASS (rc) {
      INT32 girth = Register_Girth (rc);
      if (girth > request.summary[rc]) {
	request.reqs[request.n_reqs].cl = rc;
	request.reqs[request.n_reqs].sc = ISA_REGISTER_SUBCLASS_UNDEFINED;
	request.reqs[request.n_reqs].nregs = 1;
	request.reqs[request.n_reqs].demand = FALSE;
	request.reqs[request.n_reqs++].count = girth - request.summary[rc];
	request.summary[rc] = girth;
      }
    }
    
    // Sort the request, so that more general subclasses appear
    // before more specific subclasses, and demands appear before requests.
    // Then we remove requests, until forced_locals goes to zero.
    qsort (&request.reqs, request.n_reqs, sizeof (LRA_Request_Element),
	   compare_request);

    ISA_REGISTER_CLASS forced_locals_cl = ISA_REGISTER_CLASS_UNDEFINED;
    INT forced_locals = 0;

    for (INT i = 0; i < request.n_reqs; i++) {
      const LRA_Request_Element *req = &request.reqs[i];
      GRA_Trace_Local_LRange_Request ("before forced locals adjustment", this, req->cl, req->sc, req->count,
				      req->nregs, req->demand);
    }

    for (INT i = 0; i < request.n_reqs; i++) {
      ISA_REGISTER_CLASS cl = request.reqs[i].cl;
      if (forced_locals_cl != cl) {
	forced_locals_cl = cl;
	forced_locals = Forced_Locals (forced_locals_cl);
      }
      if (forced_locals > 0) {
	INT count = request.reqs[i].count * request.reqs[i].nregs;
	if (count > forced_locals)
	  count = forced_locals;
	request.reqs[i].count -= count / request.reqs[i].nregs;
	forced_locals -= count;
      }
    }

    for (INT i = 0; i < request.n_reqs; i++) {
      const LRA_Request_Element *req = &request.reqs[i];
      GRA_Trace_Local_LRange_Request ("after forced locals adjustment", this, req->cl, req->sc, req->count,
				      req->nregs, req->demand);
    }

    // Now create lranges for the request.
    for (INT i = 0; i < request.n_reqs; i++) {
      rc = request.reqs[i].cl;
      sc = request.reqs[i].sc;
      nregs = request.reqs[i].nregs;
      local_lrange_count[rc] += request.reqs[i].count;
      for (INT count = 0; count < request.reqs[i].count; count++) {
	LRANGE *new_local = lrange_mgr.Create_Local(this,rc,sc,nregs);
	if (request.reqs[i].demand) {
	  new_local->Must_Allocate_Set ();
	}
	local_lranges[rc] = local_lranges[rc]->BB_Local_List_Push (new_local);
      }
    }
  } else {
    FOR_ALL_ISA_REGISTER_CLASS (rc) {
      INT32 count = Register_Girth (rc);
      if (count > 0) {
	count -= Forced_Locals (rc);
	if (count <= 0)
	  continue;
	local_lrange_count[rc] += count;
	while ( count-- > 0 ) {
	  LRANGE* new_local = lrange_mgr.Create_Local(this,rc,
						      ISA_REGISTER_SUBCLASS_UNDEFINED, 1);
	  local_lranges[rc] =
	    local_lranges[rc]->BB_Local_List_Push(new_local);
	}
      }
    }
  }
}
#else
void
GRA_BB::Create_Local_LRANGEs(ISA_REGISTER_CLASS rc, INT32 count)
{
  // As a compile speed hack, we'll preallocate the first few locals that are
  // requested.  So they won't need live ranges.  See gra_color for the actual
  // preallocateion.
  //
#ifdef TARG_ST
  count -= Forced_Locals (rc);
#else
  count -= GRA_local_forced_max;
#endif
  if ( count <= 0 )
    return;

  local_lrange_count[rc] += count;

  while ( count-- > 0 ) {
    LRANGE* new_local = lrange_mgr.Create_Local(this,rc);

    local_lranges[rc] =
      local_lranges[rc]->BB_Local_List_Push(new_local);
  }
}
#endif


/////////////////////////////////////
// Create and return a new local LRANGE for <gbb> that must be
// allocated the register -- <cl> and <reg>.
#ifdef TARG_ST
LRANGE*
GRA_BB::Create_Wired_LRANGE(ISA_REGISTER_CLASS  rc, REGISTER reg, INT nregs)
#else
LRANGE*
GRA_BB::Create_Wired_LRANGE(ISA_REGISTER_CLASS  rc, REGISTER reg)
#endif
{
#ifdef TARG_ST
  LRANGE* result = lrange_mgr.Create_Local(this,rc,ISA_REGISTER_SUBCLASS_UNDEFINED, nregs);
#else
  LRANGE* result = lrange_mgr.Create_Local(this,rc);
#endif

  gbb_mgr.Incr_Wired_Local_Count();
  result->Wire_Register(reg);
  return result;
}

/////////////////////////////////////
// Compute which global (complement) LRANGEs pass through
// this 'gbb'. Save this information away as space effeciently
// as possible.  Must not be called until after after a
// LRANGE for each GTN live in <gbb> has been created.
void
GRA_BB::Create_Global_Interferences(void)
{
  TN* tn;
  ISA_REGISTER_CLASS rc;
  GTN_SET* live;

  MEM_POOL_Push(&MEM_local_nz_pool);

  //  Keeps it from having to be copied and keeps it small:
  live = GTN_SET_Create(Max(GTN_SET_Alloc_Size(BB_live_in(bb)),
                            GTN_SET_Alloc_Size(BB_live_out(bb))),
                        &MEM_local_nz_pool);

  //  Calculate the set of GTNs live in this.  This assumes that any purely
  //  local references have been renamed are are no longer part of any
  //  global live range.  It would be good to check for this somewhere.
  live = GTN_SET_CopyD(live,BB_live_in(bb),&MEM_local_nz_pool);
  live = GTN_SET_IntersectionD(live,BB_defreach_in(bb));
  live = GTN_SET_UnionD(live,BB_live_out(bb),&MEM_local_nz_pool);
  live = GTN_SET_IntersectionD(live,BB_defreach_out(bb));

  FOR_ALL_ISA_REGISTER_CLASS( rc ) {
    intf_mgr.Create_Begin(region->Subuniverse(rc));

    for ( tn = GTN_SET_Choose(live);
          tn != GTN_SET_CHOOSE_FAILURE;
          tn = GTN_SET_Choose_Next(live,tn)
    ) {
      if ( TN_register_class(tn) == rc && TN_Is_Allocatable(tn) ) {
        LRANGE* lrange = lrange_mgr.Get(tn);

        if ( lrange->Type() == LRANGE_TYPE_COMPLEMENT ) {

          //  Might not pass our definition of what's a register TN.
          if ( lrange != NULL ) {
            intf_mgr.Create_Add_Neighbor(lrange_mgr.Get(tn));
            GTN_SET_Difference1D(live,tn);
          }
        }
      }
    }
    global_lranges[rc] = intf_mgr.Create_End();
  }

  MEM_POOL_Pop(&MEM_local_nz_pool);
}

/////////////////////////////////////
// Replace <old_lr> with <new_lr> in the set of complement
// LRANGEs that are live in gbb.
void
GRA_BB::Replace_Global_Interference(LRANGE* old_lr, LRANGE* new_lr )
{
  ISA_REGISTER_CLASS rc = old_lr->Rc();

  if (! Region_Is_Complement())
    return;

  DevAssert(old_lr->Type() == LRANGE_TYPE_COMPLEMENT,
            ("Replacing non complement type in a BB's global interference"));
  DevAssert(new_lr->Type() == LRANGE_TYPE_COMPLEMENT,
            ("Replacing non complement type in a BB's global interference"));

  global_lranges[rc] =
    global_lranges[rc]->Replace_Neighbor(old_lr, new_lr,
                               		 region->Subuniverse(rc));
}  


/////////////////////////////////////
// Change all <orig_tn> references in <gbb> to use <new_tn> instead.
void
GRA_BB::Rename_TN_References(TN* orig_tn, TN* new_tn)
{
  GRA_BB_OP_FORWARD_ITER iter;

  for (iter.Init(this); ! iter.Done(); iter.Step()) {
    INT i;
    OP* op = iter.Current();

    for ( i = OP_opnds(op) - 1; i >= 0; --i )
      if ( OP_opnd(op,i) == orig_tn) Set_OP_opnd(op, i, new_tn);

    for ( i = OP_results(op) - 1; i >= 0; --i )
      if ( OP_result(op,i) == orig_tn) Set_OP_result(op, i, new_tn);
  }
}

/////////////////////////////////////
// <lrange> is now live in of <bb>.  Alter <gbb> to reflect
// this by changing the _live_in GTN sets of the underlying BB.
void
GRA_BB::Add_Live_In_LRANGE(LRANGE* lrange)
{
  GRA_LIVE_Add_Live_In_GTN(bb, lrange->Tn());
}

/////////////////////////////////////
// <lrange> is now live out of <bb>.  Alter <gbb> to reflect
// this by changing the _live_out GTN sets of the underlying BB.
void
GRA_BB::Add_Live_Out_LRANGE(LRANGE* lrange)
{
  GRA_LIVE_Add_Live_Out_GTN(bb, lrange->Tn());
}

/////////////////////////////////////
// <lrange> is no longer live in of <bb>.  Alter <gbb>
// to reflect this by changing the _live_in GTN sets of the underlying BB.
void
GRA_BB::Remove_Live_In_LRANGE(LRANGE* lrange)
{
  GRA_LIVE_Remove_Live_In_GTN(bb, lrange->Tn());
}

/////////////////////////////////////
// <lrange> is no longer live out of <bb>.  Alter <gbb>
// to reflect this by changing the _live_out GTN sets of the underlying BB.
void
GRA_BB::Remove_Live_Out_LRANGE(LRANGE* lrange)
{
  GRA_LIVE_Remove_Live_Out_GTN(bb, lrange->Tn());
}

/////////////////////////////////////
// Return the number of live complement LRANGEs for the given
// <gbb> and <rc>.
INT32
GRA_BB::Global_Live_Lrange_Count(ISA_REGISTER_CLASS    rc)
{
  return global_lranges[rc]->Count();
}

/////////////////////////////////////
// Does <gbb> have more than one predecessor block?
BOOL
GRA_BB::Has_Multiple_Predecessors(void)
{
  GRA_BB_FLOW_NEIGHBOR_ITER iter;
  INT count = 0;
  for (iter.Preds_Init(this); ! iter.Done(); iter.Step()) {
    if ( ++count > 1 )
      return TRUE;
  }
  return FALSE;
}

/////////////////////////////////////
// returns true if block for a real region (as opposed to swp region).
BOOL
GRA_BB::Is_Region_Block(BOOL swp_too)
{
  RID* rid = Region()->Rid();
  if (rid && rid != Current_Rid &&
      (RID_type(rid) != RID_TYPE_swp || swp_too == TRUE)) {
    return TRUE;
  }
  return FALSE;
}

/////////////////////////////////////
// return true if this block is the entry to a region
BOOL
GRA_BB::Is_Region_Entry_Block(void)
{
  RID* rid = Region()->Rid();
  if (Is_Region_Block(FALSE) && Bb() == CGRIN_entry(RID_cginfo(rid))) 
    return TRUE;
  return FALSE;
}

/////////////////////////////////////
// determine if block acts as loop epilog or prolog
void 
GRA_BB::Check_Loop_Border(void)
{
  GRA_BB_FLOW_NEIGHBOR_ITER iter;

  //
  // TODO: right now we only look at prologs and epilogs for loop nests.
  // i.e. we don't care about prologs and epilogs for inner loops.
  // when we get prologs and epilogs on all loops, then we may be interested
  // in them as a way to profitably split TN's that have references in outer
  // loops of a nest, but not in its inner loops (in this case, we'd be
  // splitting a TN that's already been colored, and that's not going to
  // happen for a while anyway).
  //
  if (Loop() != NULL) {
    return;
  }

  for (iter.Preds_Init(this); ! iter.Done(); iter.Step()) {
    GRA_BB* pred = iter.Current();
    if (pred->Loop() != NULL) {
      Loop_Epilog_Set();
    }
  }
   
  // a block can be both prolog and epilog
  for (iter.Succs_Init(this); ! iter.Done(); iter.Step()) {
    GRA_BB* succ = iter.Current();
    if (succ->Loop() != NULL) {
      Loop_Prolog_Set();
    }
  }
}

