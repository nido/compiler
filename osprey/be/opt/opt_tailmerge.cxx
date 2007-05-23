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
 * @file   opt_tailmerge.cxx
 * @author Quentin COLOMBET
 * @date   Wed Jan 24 13:54:00 2007
 * @brief  Contains the targetting of functions required for tailmerge algorithm
 *         as defined in commom/com/tailmerge.h file. This targetting supplies
 *         the facilities to apply this algorithm to the whirl representation at
 *         WOPT level.
 */

#include <map>                // For map usage
#include <list>               // For list usage
#include <set>                // For set usage

#include "opt_tailmerge.h"
#include "opt_cfg.h"           // For CFG
#include "opt_bb.h"            // For BB_NODE
#include "opt_htable.h"        // For STMTREP
#include "wn.h"                // For WN manipulation
#include "tailmerge.h"         // For tailmerge algorithm
#include "tracing.h"           // For tracing


// Override tailmerge condition to activate specific traces
#undef DEBUG_TAILMERGE_CONDITION
#define DEBUG_TAILMERGE_CONDITION Trace_Tailmerge

/**
 * Global tracing option
 */
static BOOL Trace_Tailmerge = FALSE;

/**
 * Value of an invalid label index
 */
static const LABEL_IDX INVALID_LABEL = 0;

// Shortcuts
typedef std::map<LABEL_IDX, TAILMERGE_NAMESPACE::CNode<BB_NODE, WN>::
BasicBlocks > LabelToBBs;
typedef LabelToBBs::const_iterator CItLabelToBBs;
typedef LabelToBBs::iterator ItLabelToBBs;
typedef std::list<LABEL_IDX> Labels;
typedef Labels::const_iterator CItLabels;

/**
 * Map the identifier of created label with the list of basic blocks, which jump
 * on that label
 */
LabelToBBs g_createdLabels;

//------------------------------------------------------------------------------
// Tailmerge functions to be targeted declaration
//------------------------------------------------------------------------------
namespace TAILMERGE_NAMESPACE
{
/**
 * @see tailmerge.h
 */
template<>
static bool
IsEmpty<BB_NODE>(const BB_NODE& a_bb);

/**
 * @see tailmerge.h
 */
template<>
static BB_NODE*
InvalidBasicBlock<BB_NODE>();

/**
 * @see tailmerge.h
 */
template<>
static bool
AreEquivalent<WN>(WN* op1, WN* op2);

/**
 * @see tailmerge.h
 */
template<>
static WN*
GetLastOp<BB_NODE, WN>(BB_NODE& bb);

/**
 * @see tailmerge.h
 */
template<>
static void
DumpOperation<WN>(FILE* a_file, WN* op);

/**
 * @see tailmerge.h
 */
template<>
static int
BasicBlockId<CFG, BB_NODE>(const CFG& cfg, const BB_NODE& bb);

/**
 * @see tailmerge.h
 */
template<>
static void
ReplaceJump<CFG, BB_NODE>(CFG& cfg, BB_NODE& src, BB_NODE& tgt, BB_NODE& origBb,
                       bool jumpHere);

/**
 * @see tailmerge.h
 */
template<>
static void
AddGoto<CFG, BB_NODE>(CFG& cfg, BB_NODE& src, BB_NODE& tgt, bool forExplicit);

/**
 * @see tailmerge.h
 */
template<>
static bool
IsJump<CFG, BB_NODE, WN>(const WN* op, const CFG* a_cfg, const BB_NODE* tgt);

/**
 * @see tailmerge.h
 */
template<>
static void
RemoveBBs<CFG, CNode<BB_NODE, WN>::BasicBlocks>(CFG& a_cfg, CNode<BB_NODE, WN>::
                                                BasicBlocks& a_toRemove);

/**
 * @see tailmerge.h
 */
template<>
static void
RemoveOp<CFG, BB_NODE, WN>(CFG& a_cfg, BB_NODE& a_bb, WN* op);

/**
 * @see tailmerge.h
 */
template<>
static void
AppendOp<CFG, BB_NODE, WN>(CFG& a_cfg, BB_NODE& a_bb, WN* op);

/**
 * @see tailmerge.h
 */
template<>
static void
GetBasicBlocksList<CFG, BB_NODE>(std::list<BB_NODE*>& listOfBBs, CFG& a_cfg);

/**
 * @see tailmerge.h
 */
template<>
static void
GetPredecessorsList<CFG, BB_NODE>(std::list<BB_NODE*>& listOfPreds, CFG& a_cfg,
                              const BB_NODE& a_bb);

/**
 * @see tailmerge.h
 */
template<>
static BB_NODE*
GenAndInsertBB<CFG, BB_NODE>(CFG& a_cfg, BB_NODE& a_bb, bool bBefore);
} // End TAILMERGE_NAMESPACE

//------------------------------------------------------------------------------
// Functions declared in this files
//------------------------------------------------------------------------------
static LABEL_IDX
SetLabel(CFG& a_cfg, BB_NODE& tgt, bool here);

static void
InternalAddGoto(CFG& a_cfg, BB_NODE& src, BB_NODE& tgt, bool here);

static bool
IsFake(const CFG& a_cfg, const BB_NODE& a_bb);

static void
ConnectPredSucc(CFG& a_cfg, BB_NODE& pred, BB_NODE& succ);

static void
InitializeTailmerge(CFG& a_cfg);

static void
FinalizeTailmerge(CFG& a_cfg);

static bool
CheckLabelUsage(LABEL_IDX label, BB_NODE& a_bb, CFG& a_cfg);

static void
RemoveLabels(const Labels& labelsToRemove, CFG& a_cfg);

static void
RemoveTrivialGoto(CFG& a_cfg);

static void
RemoveUselessLabels(CFG& a_cfg);

static bool
AreTrivialBranched(CFG& a_cfg, BB_NODE* src, BB_NODE* dst);

static void
EnsureBBsKind(CFG& a_cfg);

static void
SplitLastOpInNewFallThroughBB(CFG& a_cfg, BB_NODE& a_bb);

static BB_NODE*
CreateBb(CFG& a_cfg, BB_NODE& a_fixedPoint, bool bBefore,
         const char* msg = NULL);


//------------------------------------------------------------------------------
// Functions definition
//------------------------------------------------------------------------------
void
OPT_Tailmerge(CFG& a_cfg, WN* wn_tree)
{
    MEM_POOL mempool;
    MEM_POOL_Initialize(&mempool, "Tailmerge optimization", TRUE);
    MEM_POOL_Push (&mempool);
    Trace_Tailmerge = Get_Trace(TP_TAIL, 1);
    DbgPrintTailmerge((TAILMERGE_NAMESPACE::debugOutput,
                       "%sCFG Before Tailmerge\n%s", DBar, DBar));
    DbgTailmerge(a_cfg.Print, (TAILMERGE_NAMESPACE::debugOutput));

    InitializeTailmerge(a_cfg);

    {
        // Insertion point is set to false, to avoid to break flow with
        // falsebr and truebr branches and thus have to create fallthrough basic
        // blocks
        TAILMERGE_NAMESPACE::CTailmerge<CFG, BB_NODE, WN>
            tailmergeOpt(a_cfg, true, true, Trace_Tailmerge, false,
                         &mempool);
        tailmergeOpt.Optimize();
    }

    FinalizeTailmerge(a_cfg);
    
    MEM_POOL_Pop(&mempool);
    MEM_POOL_Delete(&mempool);

    DbgPrintTailmerge((TAILMERGE_NAMESPACE::debugOutput,
                       "%sCFG After Tailmerge\n%s", DBar,DBar));
    DbgTailmerge(a_cfg.Print, (TAILMERGE_NAMESPACE::debugOutput));
}

/**
 * Set a label in given basic block.
 * The placement of this label can be either at the beginning of the basic block
 * (here = false) or at the current position (the end) (here = true). If a label
 * already exists at the specified place, it does not create a new one.
 *
 * @param  a_cfg Control flow graph that contains tgt
 * @param  tgt Basic block for which we want a label idx for the specified
 *         position
 * @param  here Specify where to find/insert a label: 'false' at the beginning
 *         of tgt, 'true' at the end
 *
 * @pre    a_cfg contains tgt
 * @post   result is the label idx of a label at the beginning (here = false) or
 *         the end (here = true) of tgt
 *
 * @return The label idx of the related inserted/found label
 */
static LABEL_IDX
SetLabel(CFG& a_cfg, BB_NODE& tgt, bool here)
{
    LABEL_IDX label = tgt.Labnam();
    if(!label && !here)
        {
            label = a_cfg.Alloc_label();
            a_cfg.Append_label_map(label, &tgt);
            tgt.Set_labnam(label);
            a_cfg.Prepend_wn_in(&tgt, WN_CreateLabel(0, label, 0, NULL));
            // We keep trace of only easly removable labels
            g_createdLabels[label] =
                TAILMERGE_NAMESPACE::CNode<BB_NODE, WN>::BasicBlocks();
        }
    else if(here)
        {
            WN* op = TAILMERGE_NAMESPACE::GetLastOp<BB_NODE, WN>(tgt);
            if(op && WN_operator(op) == OPR_LABEL)
                {
                    label = WN_label_number(op);
                }
            else
                {
                    label = a_cfg.Alloc_label();
                    a_cfg.Append_label_map(label, &tgt);
                    a_cfg.Append_wn_in(&tgt, WN_CreateLabel(0, label, 0, NULL));
                }
        }
    return label;
}

/**
 * Add a goto operation between src and tgt (src -> tgt).
 * The here flag specifies whether the target of the goto is the beginning of
 * tgt (here = false) or its current position (i.e. its end) (here = true).
 * This function also updates the list of predecessors and successors of given
 * basic blocks.
 *
 * @param  a_cfg Control flow graph that contains src and tgt
 * @param  src Basic block that will recieve the goto operation
 * @param  tgt Basic block target of the goto operation
 * @param  here Specify where the inserted goto should arrive in tgt: 'false' at
 *         the beginning of tgt, 'true' at its end
 *
 * @pre    a_cfg contains src and tgt
 * @post   src's last operation is a goto to tgt beginning (here = false) or
 *         end (here = true) and src's list of successors contains tgt and tgt's
 *         list of predecessors contains src
 */
static void
InternalAddGoto(CFG& a_cfg, BB_NODE& src, BB_NODE& tgt, bool here)
{
    WN* wnGoto = WN_CreateGoto(SetLabel(a_cfg, tgt, here));
    TAILMERGE_NAMESPACE::AppendOp<CFG, BB_NODE, WN>(a_cfg, src, wnGoto);
}

/**
 * Check whether given basic block is a fake or not.
 *
 * @param  a_cfg Control flow graph that contains a_bb
 * @param  a_bb Basic block to be checked
 *
 * @pre    a_cfg contains a_bb
 * @post   true
 *
 * @return True if a_bb should not be used during tailmerge algorithm, false
 *         otherwise
 */
static bool
IsFake(const CFG& a_cfg, const BB_NODE& a_bb)
{
    return &a_bb == a_cfg.Fake_entry_bb() || &a_bb == a_cfg.Fake_exit_bb();
}

/**
 * Connect given pred and succ in their list of successors and predecessors.
 * The connection is only virtual, no operation insertion is done to ensure
 * this connection
 *
 * @param  a_cfg Control flow graph that contains pred and succ
 * @param  pred Basic block to be set in predecessors list of succ basic block
 * @param  succ Basic block to be set in successors list of pred basic block
 *
 * @pre    a_cfg contains pred and succ
 * @post   pred.Succ()->Contains(succ) and succ.Pred()->Contains(pred)
 */
static void
ConnectPredSucc(CFG& a_cfg, BB_NODE& pred, BB_NODE& succ)
{
    using namespace TAILMERGE_NAMESPACE;
    DbgPrintTailmerge((debugOutput, "Connect BB%d to BB%d (only in list of succ"
                       "/pred)\n", BasicBlockId<CFG, BB_NODE>(a_cfg, pred),
                       BasicBlockId<CFG, BB_NODE>(a_cfg, succ)));
    // Check whether they are in the list or not
    if(!succ.Pred()->Contains(&pred))
        {
            DbgPrintTailmerge((debugOutput, "- Update pred list\n"));
            succ.Append_pred(&pred, a_cfg.Mem_pool());
        }
    if(!pred.Succ()->Contains(&succ))
        {
            DbgPrintTailmerge((debugOutput, "- Update succ list\n"));
            pred.Append_succ(&succ, a_cfg.Mem_pool());
        }
}

static void
ExplicitJump(CFG& a_cfg)
{
    for(BB_NODE* tmp = a_cfg.First_bb(); tmp; tmp = tmp->Next())
        {
            using namespace TAILMERGE_NAMESPACE;
            if(tmp->Kind() != BB_EXIT && (IsEmpty<BB_NODE>(*tmp) ||
                WN_opcode(GetLastOp<BB_NODE, WN>(*tmp)) != OPC_GOTO) &&
               tmp->Next())
                {
                    AddGoto<CFG, BB_NODE>(a_cfg, *tmp, *tmp->Next(), true);
                }
        }
}

/**
 * Initialize structures that support the targetting of tailmerge algorithm.
 *
 * @param  a_cfg Control flow graph
 *
 * @pre    true
 * @post   a_cfg can be tailmerged and FinalizeTailmerge must be called
 *
 */
static void
InitializeTailmerge(CFG& a_cfg)
{
    g_createdLabels.clear();
//    ExplicitJump(a_cfg);
}

/**
 * Finalize tailmerge algorithm for given a_cfg.
 * The finalization consists in cleaning control flow graph and reflecting its
 * internal change.
 *
 * @param  a_cfg Control flow graph
 *
 * @pre    InitializeTailmerge(a_cfg) has been called and only one call to
 *         tailmerge optimization has been made after this call and the
 *         parameter of that tailmerge was this control flow graph
 * @post   a_cfg can be used in WOPT
 *
 * @remarks Another call to InitializeTailmerge after the optimization does not
 *          cause the finalization to fail, but it will produce less effecient
 *          code
 */
static void
FinalizeTailmerge(CFG& a_cfg)
{
    RemoveTrivialGoto(a_cfg);
    RemoveUselessLabels(a_cfg);
    EnsureBBsKind(a_cfg);

    a_cfg.Invalidate_and_update_aux_info();

}

/**
 * Remove useless label.
 * A label is useless when nobody points on it. This removal applies only on
 * labels added during the tailmerge optimization of the related control flow
 * graph
 *
 * @param  a_cfg Control flow graph
 *
 * @pre    Same as FinalizeTailmerge
 * @post   Useless lables may have been removed
 *
 */
static void
RemoveUselessLabels(CFG& a_cfg)
{
    CItLabelToBBs itLabel;
    Labels labelsToRemove;
    for(itLabel = g_createdLabels.begin(); itLabel != g_createdLabels.end();
        ++itLabel)
        {
            TAILMERGE_NAMESPACE::CNode<BB_NODE, WN>::CItBasicBlocks itBb;
            bool isUsed = false;
            for(itBb = itLabel->second.begin(); itBb != itLabel->second.end() &&
                    !isUsed; ++itBb)
                {
                    isUsed |= CheckLabelUsage(itLabel->first, **itBb, a_cfg);
                }
            if(!isUsed)
                {
                    labelsToRemove.push_back(itLabel->first);
                }
        }
    RemoveLabels(labelsToRemove, a_cfg);
}

/**
 * Check label usage in given basic block.
 * A label is considered as used when the last operation of given basic block is
 * a jump to that label. Thus, this function serves our purpose for trivial
 * removing of label but must not be used in a more general case
 *
 * @param  label Index of the label to be checked
 * @param  a_bb Basic block used for the check
 * @param  a_cfg Control flow graph that contains a_bb
 *
 * @pre    if label is used, it is done only by the last operation of a_bb,
 *         which, in that case, must be a jump one (IsJump = true)
 * @post   result = last operation of a_bb is a jump to label
 *
 * @return True if label is used by the last operation of a_bb, false otherwise
 */
static bool
CheckLabelUsage(LABEL_IDX label, BB_NODE& a_bb, CFG& a_cfg)
{
    WN *op = TAILMERGE_NAMESPACE::IsEmpty<BB_NODE>(a_bb)? NULL:
        TAILMERGE_NAMESPACE::GetLastOp<BB_NODE, WN>(a_bb);
    return op && TAILMERGE_NAMESPACE::IsJump<CFG, BB_NODE, WN>(op) &&
        WN_label_number(op) == label;
}

/**
 * Remove given labels from given cfg.
 *
 * @param  labelsToRemove List of lable idx to be removed
 * @param  a_cfg Control flow graph
 *
 * @pre    a_cfg contains all labels of labelsToRemove list and all those labels
 *         are the first operation of related basic block
 * @post   all labels in labelsToRemove have been removed from a_cfg
 */
static void
RemoveLabels(const Labels& labelsToRemove, CFG& a_cfg)
{
    CItLabels it;
    for(it = labelsToRemove.begin(); it != labelsToRemove.end(); ++it)
        {
            BB_NODE* bb = a_cfg.Get_bb_from_label(*it);
            DevAssert(bb && bb->Firststmt() &&
                      WN_operator(bb->Firststmt()) == OPR_LABEL &&
                      WN_label_number(bb->Firststmt()) == *it,
                      ("First statement is not the sought label"));
            TAILMERGE_NAMESPACE::RemoveOp<CFG, BB_NODE, WN>(a_cfg, *bb,
                                                            bb->Firststmt());
            bb->Set_labnam(INVALID_LABEL);
        }
}

/**
 * Remove trivial goto.
 * A trivial goto is a jump to the next basic block in the list of basic blocks
 * or a jump to a basic block that is after all the next basic blocks that we
 * can safely execute before the target block. A basic block is safe to execute
 * if it is empty or contains only a goto to a safe basic block
 *
 * @param  a_cfg Control flow graph
 *
 * @pre    Same as FinalizeTailmerge
 * @post   Trivial goto has been removed
 */
static void
RemoveTrivialGoto(CFG& a_cfg)
{
    DbgPrintTailmerge((TAILMERGE_NAMESPACE::debugOutput,
                       "*** RemoveTrivialGoto\n"));
    for(BB_NODE* tmp = a_cfg.First_bb(); tmp; tmp = tmp->Next())
        {
            DbgPrintTailmerge((TAILMERGE_NAMESPACE::debugOutput,
                               "Considered basic block: BB%d\n",
                               TAILMERGE_NAMESPACE::
                               BasicBlockId<CFG, BB_NODE>(a_cfg, *tmp)));

            WN* op = TAILMERGE_NAMESPACE::IsEmpty<BB_NODE>(*tmp)? NULL:
                TAILMERGE_NAMESPACE::GetLastOp<BB_NODE, WN>(*tmp);
            if(op && WN_opcode(op) == OPC_GOTO &&
               AreTrivialBranched(a_cfg, tmp,
                                  a_cfg.Get_bb_from_label(WN_label_number(op))))
                {
                    DbgPrintTailmerge((TAILMERGE_NAMESPACE::debugOutput,
                                       "Has trivial goto, remove it\n"));
                    TAILMERGE_NAMESPACE::RemoveOp<CFG, BB_NODE, WN>(a_cfg, *tmp,
                                                                    op);
                }
        }
}

/**
 * Check whether it exists a trivial path between src and dst.
 * A path is considered as trivial if the jump of src to dst can be removed.
 *
 * @param  a_cfg Control flow graph that contains src and dst
 * @param  src Source basic block
 * @param  dst Destination basic block
 *
 * @pre    Same as FinalizeTailmerge
 * @post   result implies it exists a trivial path betwee, src and dst
 *
 * @return True the jump of src to dst can be removed, false otherwise
 */
static bool
AreTrivialBranched(CFG& a_cfg, BB_NODE* src, BB_NODE* dst)
{
    using namespace TAILMERGE_NAMESPACE;
    DbgPrintTailmerge((debugOutput, "Check trivial branch: BB%d -> BB%d\n",
                       BasicBlockId<CFG, BB_NODE>(a_cfg, *src),
                       BasicBlockId<CFG, BB_NODE>(a_cfg, *dst)));

    std::set<int> hasBeenVisited;
    hasBeenVisited.insert(BasicBlockId<CFG, BB_NODE>(a_cfg, *src));
    src = src->Next();
    bool result;
    while(!(result = src == dst) && src &&
          hasBeenVisited.find(BasicBlockId<CFG, BB_NODE>(a_cfg, *src)) ==
          hasBeenVisited.end())
        {
            hasBeenVisited.insert(BasicBlockId<CFG, BB_NODE>(a_cfg, *src));
            DbgPrintTailmerge((debugOutput, "Current src: BB%d\n",
                               BasicBlockId<CFG, BB_NODE>(a_cfg, *src)));
            BB_NODE* next = src->Next();
            if(!IsEmpty<BB_NODE>(*src))
                {
                    WN* op = GetLastOp<BB_NODE, WN>(*src);
                    // basic block has only one instruction and this
                    // instruction is a goto: Trivial path
                    if(src->Laststmt() == src->Firststmt() &&
                       WN_opcode(op) == OPC_GOTO)
                        {
                            DbgPrintTailmerge((debugOutput, "Next basic block "
                                               "is in trivial path\n"));
                            next = a_cfg.Get_bb_from_label(WN_label_number(op));
                        }
                    else
                        {
                            next = NULL;
                        }
                }
            DbgPrintTailmerge((debugOutput, "next %p\n", next));
            src = next;
        }
    return result;
}

/**
 * Ensure that the kind of each basic blocks respects cfg constraint.
 *
 * @param  a_cfg Control flow graph
 *
 * @pre    true
 * @post   a_cfg can be used by other module of WOPT
 */
static void
EnsureBBsKind(CFG& a_cfg)
{
    using namespace TAILMERGE_NAMESPACE;
    for(BB_NODE* tmp = a_cfg.First_bb(); tmp; tmp = tmp->Next())
        {
            if(tmp->Kind() == BB_LOGIF)
                {
                    DevAssert(!IsEmpty<BB_NODE>(*tmp),
                              ("LOGIF block are normally never optimized, so"
                               " they cannot be empty!"));
                    OPCODE opc = WN_opcode(GetLastOp<BB_NODE, WN>(*tmp));
                    if(opc != OPC_FALSEBR && opc != OPC_TRUEBR)
                        {
                            DbgPrintTailmerge((debugOutput,
                                               "BB%d must be split\n",
                                               BasicBlockId<CFG, BB_NODE>(a_cfg,
                                                                          *tmp))
                                              );
                            DevAssert(opc == OPC_GOTO,
                                      ("Only a goto could be inserted by the "
                                       "algorithm!!"));
                            SplitLastOpInNewFallThroughBB(a_cfg, *tmp);
                        }
                }
        }
}

/**
 * Split last operation of given basic block in a new basic block.
 * The idea is to create a fallthrough basic block for the last operation
 *
 * @param  a_cfg Control flow graph
 * @param  a_bb Basic block to be spilt
 *
 * @pre    true
 * @post   a_bb.Next() is new and a_bb.Next().Operations =
 *         a_bb@pre.Operations->last() and a_bb.Operations = a_bb.Operations@pre
 *         ->excluding(a_bb.Operations@pre->last())
 */
static void
SplitLastOpInNewFallThroughBB(CFG& a_cfg, BB_NODE& a_bb)
{
    WN* op = TAILMERGE_NAMESPACE::GetLastOp<BB_NODE, WN>(a_bb);
    TAILMERGE_NAMESPACE::RemoveOp<CFG, BB_NODE, WN>(a_cfg, a_bb, op);
    BB_NODE* newBb = CreateBb(a_cfg, a_bb, false, " for split");
    TAILMERGE_NAMESPACE::AppendOp<CFG, BB_NODE, WN>(a_cfg, *newBb, op);
    newBb->Append_pred(&a_bb, a_cfg.Mem_pool());
}

/**
 * Create and insert a new basic block in cfg.
 *
 * @param  a_cfg Control flow graph
 * @param  a_fixedPoint Basic block used as fixed point
 * @param  bBefore Specify whether insertion of new basic block must be done
 *         before (bBefore = true) or after (bBefore = false) a_fixedPoint
 * @param  msg Optionnal message, used for verbose
 *
 * @pre    a_cfg contains a_fixedPoint
 * @post   result <> NULL and if bBefore then result->Next() = a_fixedPoint else
 *         a_fixedPoint.Next() = result endif
 *
 * @return The created basic block
 */
static BB_NODE*
CreateBb(CFG& a_cfg, BB_NODE& a_fixedPoint, bool bBefore, const char* msg)
{
    using namespace TAILMERGE_NAMESPACE;
    BB_NODE* res = a_cfg.Create_and_allocate_bb(BB_GOTO);
    FmtAssert(res, ("Basic block creation failed%s", msg? msg: ""));
    DbgPrintTailmerge((debugOutput, "Created bb%s: BB%d\n",
                       msg? msg: "", BasicBlockId<CFG, BB_NODE>(a_cfg, *res)));
    if(bBefore)
        {
            a_fixedPoint.Insert_Before(res);
        }
    else
        {
            a_fixedPoint.Insert_After(res);
        }
    return res;
}

//------------------------------------------------------------------------------
// Tailmerge functions to be targeted definition
//------------------------------------------------------------------------------
namespace TAILMERGE_NAMESPACE
{
template<>
static bool
IsEmpty<BB_NODE>(const BB_NODE& a_bb)
{
    return a_bb.Firststmt() == NULL;
}

template<>
static BB_NODE*
InvalidBasicBlock<BB_NODE>()
{
    return (BB_NODE*)NULL;
}

template<>
static bool
AreEquivalent<WN>(WN* wn1, WN* wn2)
{
    bool result = wn1 == wn2;
    if(!result && wn1 && wn2 && WN_Equiv(wn1, wn2))
        {
            result = true;
            // now examine the kids
            if(WN_opcode(wn1) == OPC_BLOCK)
                {
                    WN *kid1 = WN_first(wn1);
                    WN *kid2 = WN_first(wn2);
                    while(result)
                        {
                            result &= AreEquivalent(kid1, kid2);
                            if(kid1 == NULL)
                                {
                                    result &= kid2 == NULL;
                                    break;
                                }
                            kid1 = WN_next(kid1);
                            kid2 = WN_next(kid2);
                        }
                }
            else
                {
                    // since the two nodes are equiv, they have the same # of
                    // children
                    int i;
                    for(i=0; result && i < WN_kid_count(wn1); ++i)
                        {
                            result &= AreEquivalent(WN_kid(wn1, i),
                                                    WN_kid(wn2, i));
                        }
                }
            if(result && (WN_opcode(wn1) == OPC_TRUEBR ||
                          WN_opcode(wn1) == OPC_FALSEBR))
                {
                    // Truebr and falsebr are never equivalent because in
                    // current whirl representation the next instruction is
                    // always the end of a basic block, thus an implicit jump.
                    // But this jump can never be equal since next basic block
                    // cannot be the same.
                    result = false;
                }
        }
    return result;
}

template<>
static WN*
GetLastOp<BB_NODE, WN>(BB_NODE& bb)
{
    return bb.Laststmt();
}

template<>
static void
DumpOperation<WN>(FILE* a_file, WN* op)
{
    fdump_tree_no_st(a_file, op);
    fprintf(a_file, "\n");
}

template<>
static int
BasicBlockId<CFG, BB_NODE>(const CFG& cfg, const BB_NODE& bb)
{
    return (int)bb.Id();
}

template<>
static void
ReplaceJump<CFG, BB_NODE>(CFG& cfg, BB_NODE& src, BB_NODE& tgt, BB_NODE& origBb,
                       bool jumpHere)
{
    // Avoid duplicat in succ/pred list.
    // Is it needed?
    if(src.Succ()->Contains(&tgt))
        {
            src.Remove_succ(&origBb, cfg.Mem_pool());
        }
    else
        {
            src.Replace_succ(&origBb, &tgt);
        }
    if(origBb.Pred()->Contains(&tgt))
        {
            origBb.Remove_pred(&src, cfg.Mem_pool());
        }
    else
        {
            origBb.Replace_pred(&src, &tgt);
        }
    InternalAddGoto(cfg, src, tgt, jumpHere);
}

template<>
static void
AddGoto<CFG, BB_NODE>(CFG& a_cfg, BB_NODE& src, BB_NODE& tgt, bool forExplicit)
{
    InternalAddGoto(a_cfg, src, tgt, false);
}

template<>
static bool
IsJump<CFG, BB_NODE, WN>(const WN* op, const CFG* a_cfg, const BB_NODE* tgt)
{
    OPCODE opc = WN_opcode(op);
    return (opc == OPC_GOTO || opc == OPC_TRUEBR ||opc == OPC_FALSEBR)
        // This part is not needed because insertion point of the tailmerge
        // algorithm has been set to 'after' (see construction of the CTailmerge
        // object) and falsebr and truebr are defined as never equivalent.
/*        && (!tgt || (a_cfg &&
                     a_cfg->Get_bb_from_label(WN_label_number(op)) == tgt))*/;
}

template<>
static void
RemoveBBs<CFG, CNode<BB_NODE, WN>::BasicBlocks>(CFG& a_cfg, CNode<BB_NODE, WN>::BasicBlocks& a_toRemove)
{
    // It is complicate to remove empty basic blocks at this points (basic
    // blocks statements are not all set) and a little use.
    // Deadcode while do the proper job
}

template<>
static void
RemoveOp<CFG, BB_NODE, WN>(CFG& a_cfg, BB_NODE& a_bb, WN* op)
{
    WN* it;
    for(it = a_bb.Laststmt(); it; it = WN_prev(it))
        {
            if(it == op)
                {
                    if(WN_prev(it))
                        {
                            WN_next(WN_prev(it)) = WN_next(it);
                        }
                    if(WN_next(it))
                        {
                            WN_prev(WN_next(it)) = WN_prev(it);
                        }
                    if(it ==  a_bb.Laststmt())
                        {
                            a_bb.Set_laststmt(WN_prev(it));
                        }
                    if(it == a_bb.Firststmt())
                        {
                            a_bb.Set_firststmt(WN_next(it));
                        }
                    break;
                }
        }
}

template<>
static void
AppendOp<CFG, BB_NODE, WN>(CFG& a_cfg, BB_NODE& a_bb, WN* op)
{
    a_cfg.Append_wn_in(&a_bb, op);
    if(IsJump<CFG, BB_NODE, WN>(op))
        {
            LABEL_IDX label = WN_label_number(op);
            BB_NODE* tgt = a_cfg.Get_bb_from_label(label);
            DevAssert(tgt, ("Cannot get label from a jump operation"));

            if(g_createdLabels.find(label) != g_createdLabels.end())
                {
                    g_createdLabels[label].push_back(&a_bb);
                }
            ConnectPredSucc(a_cfg, a_bb, *tgt);
        }
}

template<>
static void
GetBasicBlocksList<CFG, BB_NODE>(std::list<BB_NODE*>& listOfBBs, CFG& a_cfg)
{
    for(BB_NODE* tmp = a_cfg.First_bb(); tmp; tmp = tmp->Next())
        {
            if(!IsFake(a_cfg, *tmp))
                {
                    listOfBBs.push_back(tmp);
                }
        }
}

template<>
static void
GetPredecessorsList<CFG, BB_NODE>(std::list<BB_NODE*>& listOfPreds, CFG& a_cfg,
                               const BB_NODE& a_bb)
{
    for(BB_LIST* tmp = a_bb.Pred(); tmp; tmp = tmp->Next())
        {
            if(!IsFake(a_cfg, *tmp->Node()))
                {
                    listOfPreds.push_back(tmp->Node());
                }
        }
}

template<>
static BB_NODE*
GenAndInsertBB<CFG, BB_NODE>(CFG& a_cfg, BB_NODE& a_bb, bool bBefore)
{
    // Do not know the kind!!!
    BB_NODE* res = CreateBb(a_cfg, a_bb, bBefore, " for gen and insert");
    SetLabel(a_cfg, *res, false);
    return res;
}

} // End TAILMERGE_NAMESPACE
