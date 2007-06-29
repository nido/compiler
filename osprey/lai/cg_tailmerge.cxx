//-*-c++-*-
/* 

  Copyright (C) 2006 ST Microelectronics, Inc.  All Rights Reserved. 

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

  Contact information:  ST Microelectronics, Inc., 
  , or: 

  http://www.st.com 

  For further information regarding this notice, see: 

  http: 
*/
/** 
 * @file   cg_tailmerge.cxx
 * @author Quentin COLOMBET
 * @date   Thu Feb  1 08:41:30 2007
 * @brief  Contains the targetting of functions required for tailmerge algorithm
 *         as defined in commom/com/tailmerge.h file. This targetting supplies
 *         the facilities to apply this algorithm to the CGIR representation.
 */

#include "defs_exported.h"     // For common type definition (needed by op.h)
#include "op.h"                // For operation usage
#include "bb.h"                // For basicblock usage
#include "errors.h"            // For DevAssert
#include "defs.h"              // Needed by mempool.h for MEM_PTR definition
#include "dwarf.h"             // For dwarf operand definition
#include "mempool.h"           // For MEM_POOL definition
#include "cxx_memory.h"        // For call to CXX_NEW/DELETE_ARRAY
#include "tracing.h"           // For tracing
#include "cg_flags.h"          // For CG_tailmerge
#include "cg.h"                // For Check_for_Dump
#include "label_util.h"        // For Get_Label_BB
#include "tn.h"                // For TN usage
#include "cg_tailmerge.h"
#include "tailmerge.h"         // For tailmerge algorithm
#include "cgtarget.h"

static bool Trace_Tailmerge = false;

namespace TAILMERGE_NAMESPACE
{

template<>
static bool
IsEmpty<BB>(const BB& a_bb);

template<>
static BB*
InvalidBasicBlock<BB>();

template<>
static bool
AreEquivalent<OP>(OP* op1, OP* op2);

template<>
static OP*
GetLastOp<BB, OP>(BB& bb);

template<>
static void
DumpOperation<OP>(FILE* a_file, OP* op);

template<>
static int
BasicBlockId<PU, BB>(const PU& pu, const BB& bb);

template<>
static void
ReplaceJump<PU, BB>(PU& pu, BB& src, BB& tgt, BB& origBb,
                      bool jumpHere);

template<>
static void
AddGoto<PU, BB>(PU& pu, BB& src, BB& tgt, bool forExplicit);

template<>
static bool
IsJump<PU, BB, OP>(const OP* op, const PU*, const BB*);

template<>
static void
RemoveBBs<PU, CNode<BB, OP>::BasicBlocks>(PU& a_pu, CNode<BB, OP>::BasicBlocks& a_toRemove);

template<>
static void
RemoveOp<PU, BB, OP>(PU& a_pu, BB& a_bb, OP* op);

template<>
static void
AppendOp<PU, BB, OP>(PU& a_pu, BB& a_bb, OP* op);

template<>
static void
GetBasicBlocksList<PU, BB>(std::list<BB*>& listOfBBs, PU& a_pu);

template<>
static void
GetPredecessorsList<PU, BB>(std::list<BB*>& listOfPreds, PU& a_cfg,
                              const BB& a_bb);

template<>
static BB*
GenAndInsertBB<PU, BB>(PU& a_cfg, BB& a_bb, bool bBefore);
} // End TAILMERGE_NAMESPACE

static
BB*
GetBBFromJump(const OP* a_jump)
{
    DevAssert(TAILMERGE_NAMESPACE::IsJump<PU, BB, OP>(a_jump),
              ("Given operation is not a jump one"));
    TN* tgt = OP_opnd(a_jump, OP_find_opnd_use(a_jump, OU_target));
    DevAssert(TN_is_label(tgt), ("Target is not a label!"));
    return Get_Label_BB(TN_label(tgt));
}

static bool
TNequiv(const TN* tn1, const TN* tn2)
{
    bool result = tn1 == tn2;

    if(!result && tn1 && tn2)
        {
            result = (TN_is_register(tn1) &&
                      TN_is_register(tn2) &&
                      TNs_Are_Equivalent(tn1, tn2)) ||
                (TN_is_zero(tn1) && TN_is_zero(tn2));
        }
    return result;
}

void
Tailmerge(INT phase)
{
    if(CG_tailmerge & phase)
        {
            MEM_POOL mempool;
            MEM_POOL_Initialize(&mempool, "Tailmerge optimization", TRUE);
            MEM_POOL_Push (&mempool);
            
            Trace_Tailmerge = Get_Trace(TP_TAIL, 1) == TRUE;
            
            {
                TAILMERGE_NAMESPACE::CTailmerge<PU, BB, OP>
                    tailmergeOpt(Get_Current_PU(), true, true, Trace_Tailmerge,
                                 true, &mempool);
                tailmergeOpt.Optimize();
            }
            
            MEM_POOL_Pop(&mempool);
            MEM_POOL_Delete(&mempool);
            Check_for_Dump(TP_TAIL, NULL);
        }
}


namespace TAILMERGE_NAMESPACE
{

template<>
static bool
IsEmpty<BB>(const BB& a_bb)
{
    return BB_length(&a_bb) == 0;
}

template<>
static BB*
InvalidBasicBlock<BB>()
{
    return (BB*)NULL;
}

template<>
static bool
AreEquivalent<OP>(OP* op1, OP* op2)
{
    bool result = op1 == op2;
    if(!result && op1 && op2 && OP_code(op1) == OP_code(op2))
        {
            if(IsJump<PU, BB, OP>(op1))
                {
                    TN *tgt1, *tgt2;
                    tgt1 = OP_opnd(op1, OP_find_opnd_use(op1, OU_target));
                    tgt2 = OP_opnd(op2, OP_find_opnd_use(op2, OU_target));
                    DevAssert(TN_is_label(tgt1) &&  TN_is_label(tgt2),
                              ("Target is not a label!"));
                    result = Get_Label_BB(TN_label(tgt1)) == 
                        Get_Label_BB(TN_label(tgt2)) &&
                        TN_offset(tgt1) == TN_offset(tgt2);
                }
            else
                {
                    int i;
                    result = true;
                    for(i = 0; i < OP_results(op1) && result; ++i)
                        {
                            result &= TNequiv(OP_result(op1, i),
                                              OP_result(op2, i));
                        }
                    for(i = 0; i < OP_opnds(op1) && result; ++i)
                        {
                            result &= TNequiv(OP_opnd(op1, i), OP_opnd(op2, i));
                        }
                }
        }
    return result;
}

template<>
static void
RemoveBBs<PU, CNode<BB, OP>::BasicBlocks>
 (PU& a_pu, CNode<BB, OP>::BasicBlocks& a_toRemove)
{
    // Too complicate
}

template<>
static bool
IsJump<PU, BB, OP>(const OP* op, const PU* a_pu, const BB* tgt)
{
    return OP_xfer(op) && OP_cond(op);
}

template<>
static void
AddGoto<PU, BB>(PU& pu, BB& src, BB& tgt, bool forExplicit)
{
    Add_Goto(&src, &tgt);
}

template<>
static void
ReplaceJump<PU, BB>(PU& pu, BB& src, BB& tgt, BB& origBb,
                      bool jumpHere)
{
    DevAssert(!jumpHere, ("Not yet implemented"));
    Change_Succ(&src, &origBb, &tgt);
    Add_Goto(&src, &tgt);
}

template<>
static int
BasicBlockId<PU, BB>(const PU& pu, const BB& bb)
{
    return BB_id(&bb);
}

template<>
static void
DumpOperation<OP>(FILE* a_file, OP* op)
{
    Print_OP_No_SrcLine(op);
}

template<>
static OP*
GetLastOp<BB, OP>(BB& bb)
{
    return BB_last_op(&bb);
}

template<>
static void
RemoveOp<PU, BB, OP>(PU& a_pu, BB& a_bb, OP* op)
{
    BB_Remove_Op(&a_bb, op);
}

template<>
static void
AppendOp<PU, BB, OP>(PU& a_pu, BB& a_bb, OP* op)
{
    if(IsJump<PU, BB, OP>(op))
        {
            BB* tgt = GetBBFromJump(op);
            Link_Pred_Succ_with_Prob(&a_bb, tgt, 1.0F);
        }
    BB_Append_Op(&a_bb, op);
}

template<>
static void
GetBasicBlocksList<PU, BB>(std::list<BB*>& listOfBBs, PU& a_pu)
{
    BB* bb;
    for(bb = REGION_First_BB; bb; bb = BB_next(bb))
        {
            listOfBBs.push_back(bb);
        }
}

template<>
static void
GetPredecessorsList<PU, BB>(std::list<BB*>& listOfPreds, PU& a_cfg,
                              const BB& a_bb)
{
    BBLIST* bbList;
    FOR_ALL_BB_PREDS(&a_bb, bbList)
    {
        listOfPreds.push_back(BBLIST_item(bbList));
    }
}

template<>
static BB*
GenAndInsertBB<PU, BB>(PU& a_cfg, BB& a_bb, bool bBefore)
{
    return bBefore? Gen_And_Insert_BB_Before(&a_bb):
        Gen_And_Insert_BB_After(&a_bb);
}
   
} // End namespace TAILMERGE_NAMESPACE

