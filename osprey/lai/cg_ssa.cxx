
#include <list.h>

#include "defs.h"
#include "tracing.h"
#include "errors.h"
#include "mempool.h"
#include "cxx_memory.h"
#include "glob.h"
#include "bitset.h"
#include "config_targ.h"
#include "config.h"

#include "symtab.h"
#include "strtab.h"
#include "be_symtab.h"
#include "targ_isa_lits.h"
#include "topcode.h"
#include "cgir.h"
#include "cg.h"
#include "region_util.h"
#include "cg_region.h"
#include "tn_set.h"
#include "tn_list.h"
#include "tn_map.h"
#include "op_map.h"
#include "bb_set.h"
#include "bb_list.h"
#include "gtn_universe.h"
#include "gtn_set.h"
#include "gtn_tn_set.h"

#include "dominate.h"
#include "gra_live.h"
#include "cgexp.h"
#include "cg_ssa.h"

//
// Memory pool for allocating things during SSA construction.
// Memory allocated from this pool is initialized to zero.
//
static MEM_POOL ssa_pool;

static BOOL Trace_SSA_Build;                  /* -Wb,-tt60:0x002 */
static BOOL Trace_SSA_Out;                    /* -Wb,-tt60:0x004 */
static BOOL Trace_phi_removal;                /* -Wb,-tt60:0x008 */
static BOOL Trace_dom_frontier;               /* -Wb,-tt60:0x010 */

/* ================================================================
 *
 *   Mapping TNs -> BBs where they are defined
 *
 * ================================================================
 */

static TN_MAP tn_def_map = NULL;

#define TN_is_def_in(t)        ((BB_LIST *)TN_MAP_Get(tn_def_map,t))

/* ================================================================
 *
 *   Renaming stack management
 *
 * ================================================================
 */
//
// TN stack is implemented as list
//
typedef struct _stack_item {
  TN *tn;
  struct _stack_item *next;
} STACK_ITEM;

static STACK_ITEM **tn_stack; // each TN has a corresponding list

#define TN_STACK_empty(t)     (tn_stack[TN_number(t)] == NULL)
#define TN_STACK_ptr(t)       (tn_stack[TN_number(t)])
#define TN_STACK_tn(t)        ((tn_stack[TN_number(t)])->tn)
#define TN_STACK_next(t)      ((tn_stack[TN_number(t)])->next)

/* ================================================================
 *  initialize_tn_stack
 * ================================================================
 */
static void 
initialize_tn_stack (void)
{
  //
  // Initialize this for all TNs to NULL
  //
  tn_stack = (STACK_ITEM**)TYPE_MEM_POOL_ALLOC_N(STACK_ITEM*, 
						&MEM_local_pool,
						Last_TN+1);
  return;
}

/* ================================================================
 *   tn_stack_pop
 *
 *   pop an element from the tn stack.
 * ================================================================
 */
static TN *
tn_stack_pop (
  TN *base
)
{
  TN *tn;

  if (TN_STACK_empty(base))
    tn = base;
  else {
    // get the top of list
    tn = TN_STACK_tn(base);
    // remove the top of list
    TN_STACK_ptr(base) = TN_STACK_next(base);
  }

  return tn;
}

/* ================================================================
 *  tn_stack_push
 *
 *  push an element on the region stack; 
 *  grow the stack if necessary
 * ================================================================
 */
static void 
tn_stack_push (
  TN *base,
  TN *tn
)
{
  // add an item to the top of the list for 'base'
  STACK_ITEM *st = TYPE_MEM_POOL_ALLOC(STACK_ITEM, &MEM_local_pool);
  st->tn = tn;
  st->next = TN_STACK_ptr(base);
  TN_STACK_ptr(base) = st;
}

/* ================================================================
 *   tn_stack_top
 *
 *  returns the top element from the tn stack of given 'base' name
 * ================================================================
 */
static TN*
tn_stack_top (
  TN *base
)
{
  if (TN_STACK_empty(base))
    return base;
  else
    return TN_STACK_tn(base);
}



/* ================================================================
 *
 *   Dominator routines
 *
 *   In addition to BB_dom_set(), BB_pdom_set() provided by the
 *   dominate.h interface, we need things such as BB_children(),
 *   BB_idom(), and the bottom-up traversal of dominator tree.
 *
 *   TODO: we should really generalize this and merge into
 *         the dominate.[h,cxx] interface.
 * ================================================================
 */

typedef struct _Dom_Tree_node {
  BB *_M_data;
  BB *_M_parent;
  BB_SET *_M_kids;
} DOM_TREE;

#define DOM_TREE_node(t)   (t->_M_data)
#define DOM_TREE_kids(t)   (t->_M_kids)
#define DOM_TREE_parent(t) (t->_M_parent)

//
// This table is indexed with BB_id(bb). Each entry contains
// the DOM_TREE info for this bb
//
static DOM_TREE *dom_map;

#define BB_dominator(bb)         (dom_map[BB_id(bb)]._M_parent)
#define Set_BB_dominator(bb,dom) (dom_map[BB_id(bb)]._M_parent = dom); 
#define BB_children(bb)          (dom_map[BB_id(bb)]._M_kids)

inline void Add_BB_child(BB *bb, BB *child) {
  dom_map[BB_id(bb)]._M_kids = BB_SET_Union1D(dom_map[BB_id(bb)]._M_kids,
					      child,
					      &ssa_pool);
}

/* ================================================================
 *   DOM_TREE_Print
 * ================================================================
 */
static void
DOM_TREE_Print (
  FILE *file
)
{
  BB *bb;

  fprintf(file, "<ssa> DOMINATOR_INFO: \n\n");
  for (bb = REGION_First_BB; bb; bb = BB_next(bb)) {
    fprintf(file, "%s DOM_INFO for BB%d \n%s", 
	    DBar, BB_id(bb), DBar);

    // BB dominators set
    fprintf(file, "   doms: ");
    BB_SET_Print(BB_dom_set(bb), file);
    fprintf(file, "\n");

    // immediate dominator
    if (BB_dominator(bb))
      fprintf(file, "   idom: BB%d\n", BB_id(BB_dominator(bb)));
    else
      fprintf(file, "   idom: NONE\n");

    // BBs dominated by BB
    fprintf(file, "   kids: ");
    BB_SET_Print(BB_children(bb), file);
    fprintf(file, "\n");

    fprintf(file, "%s\n", DBar);
  }

  return;
}

/* ================================================================
 *   DOM_TREE_Initialize
 *
 *   NOTE:  BBs are numbered from 1, so their indices in the bit 
 *          vectors start from 1. The immediate dominators of 
 *          entry BBs is set to NULL.
 *
 * ================================================================
 */
static void
DOM_TREE_Initialize ()
{
  INT i;
  BB *bb;

  // First, calculate the dominator/postdominator information:
  Calculate_Dominators();

  // initialize dominator tree
  dom_map = (DOM_TREE *)TYPE_MEM_POOL_ALLOC_N(DOM_TREE, 
					      &ssa_pool, 
					      PU_BB_Count+2);

  for (i = 1; i < PU_BB_Count+2; i++) {
    dom_map[i]._M_kids = BB_SET_Create_Empty(PU_BB_Count+2,
					     &ssa_pool);
  }

  //  RID *rid = BB_rid(REGION_First_BB);
  //  CGRIN *cgrin = rid ? RID_Find_Cginfo(REGION_First_BB) : NULL;

  // 
  // The immediate dominator of BBi is the BB with the same set of
  // dominators except for BBi itself:
  //
  for (bb = REGION_First_BB; bb; bb = BB_next(bb)) {
    BB_SET *match, *check;
    BB *cur;

    // 
    // match is the BS (set of dominators) to be matched:
    //
    match = BB_SET_Copy(BB_dom_set(bb), &MEM_local_pool);  // bb's dom set ...
    match = BB_SET_Difference1D(match, bb);	       // ... except bb.

    // 
    // match is also the set of dominators to check:
    //
    check = BB_SET_Copy(match, &MEM_local_pool);  // Check all bb's dominators ...

    //
    // Now go check them.  We start with the last (highest numbered)
    // dominator, because that will be the immediate dominator if we
    // are dealing with a topologically-sorted BB list:
    //
    //Set_BB_dominator(bb, NULL); -- alredy zeroed
    //
    FOR_ALL_BB_SET_members(check, cur) {
      if (BB_SET_EqualP(match, BB_dom_set(cur))) {

	// parent/child:
	Set_BB_dominator(bb, cur);

	// at the same time, bb is a child of cur's
	Add_BB_child(cur, bb);

	break;
      }
    }

  }

  if (Trace_dom_frontier) 
    DOM_TREE_Print(TFile);

  return;
}

/* ================================================================
 *   DOM_TREE_Finalize
 * ================================================================
 */
static void
DOM_TREE_Finalize()
{ 
  // Clean up the dominator/posdominator information:
  Free_Dominators_Memory();
}

/* ================================================================
 *      Auxilliary REGION handling stuff
 * ================================================================
 */

static BB_SET *region_entry_set;
static BB_SET *region_exit_set;

//
// The RID associated with the entire REGION being analyzed
//
static RID *ssa_scope_rid;

/* ================================================================
 *   Set_Entries_Exits
 *
 *   Copy of the GRA_LIVE_xxx function.
 * ================================================================
 */
static void
Set_Entries_Exits( 
  RID *rid 
)
{
  CGRIN *cgrin;
  BB *bb;
  INT i,n;

  if ( rid == NULL ) {
    /*
     * whole PU
     */
    for (bb = REGION_First_BB; bb; bb = BB_next(bb)) {
      if (BB_succs(bb) == NULL)
	region_exit_set = BB_SET_Union1D(region_exit_set,
					 bb,
					 &ssa_pool);
      if (BB_preds(bb) == NULL)
	region_entry_set = BB_SET_Union1D(region_entry_set,
					  bb,
					  &ssa_pool);
    }
    return;
  }

  /*
   * use entry and exit info from the CGRIN
   */
  cgrin = RID_cginfo( rid );
  bb = CGRIN_entry( cgrin );
  region_entry_set = BB_SET_Union1D(region_entry_set,
				    bb,
				    &ssa_pool);
  n = RID_num_exits( rid );
  for (i = 0; i < n; i++) {
    bb = CGRIN_exit_i( cgrin, i );
    region_exit_set = BB_SET_Union1D(region_exit_set,
				     bb,
				     &ssa_pool);
  }
  ssa_scope_rid = rid;
  return;
}


/* ================================================================
 *
 *                         SSA algorithm
 *
 * ================================================================
 */

//
// Only rename TNs that are not dedicated or a save for callee
// saves
//
#define TN_can_be_renamed(tn)  (!TN_is_dedicated(tn) && !TN_is_save_reg(tn))

//
// 1. We need some mapping between PHI-function opnds
//    and BB predecessors of the BB where the PHI-function lives.
//
typedef struct phi_map_entry {
  BB **opnd_src;    /* table of pred BBs for each operand */
} PHI_MAP_ENTRY;

static OP_MAP phi_op_map = NULL;

//
// 2. We need mapping between each TN and its definition.
//    This is exported, so everybody can have access.
//
TN_MAP tn_ssa_map = NULL;

//
// Mapping between phi-resource TNs and their representative name
// TNs
static MEM_POOL tn_map_pool;
//static TN_MAP tn_to_new_name_map = NULL;

static hTN_MAP tn_to_new_name_map = NULL;

#define TN_new_name(t)       ((TN *)hTN_MAP_Get(tn_to_new_name_map,t))
#define Set_TN_new_name(t,n) (hTN_MAP_Set(tn_to_new_name_map,t,n))

/* ================================================================
 *   Set_PHI_Operands
 * ================================================================
 */
static void
Set_PHI_Operands(
  OP   *phi
)
{
  INT i = 0;
  BBLIST *preds;

  Is_True(OP_code(phi) == TOP_phi,("not a PHI function"));
  PHI_MAP_ENTRY *entry = TYPE_MEM_POOL_ALLOC(PHI_MAP_ENTRY, 
					     &ssa_pool);

  BB *bb = OP_bb(phi);

  entry->opnd_src = TYPE_MEM_POOL_ALLOC_N(BB *, 
					  &ssa_pool, 
					  OP_opnds(phi));

  FOR_ALL_BB_PREDS(bb,preds) {
    BB *pred = BBLIST_item(preds);
    entry->opnd_src[i] = pred;
    i++;
  }

  OP_MAP_Set(phi_op_map, phi, entry);
  
  return;
}

/* ================================================================
 *   Get_PHI_Predecessor
 * ================================================================
 */
BB*
Get_PHI_Predecessor (
  OP *phi,
  INT8 i
)
{
  Is_True(OP_code(phi) == TOP_phi,("not a PHI function"));
  PHI_MAP_ENTRY *entry = (PHI_MAP_ENTRY *)OP_MAP_Get(phi_op_map, phi);
  Is_True(entry != NULL,("unmapped op ?"));

  return entry->opnd_src[i];
}

/* ================================================================
 *   SSA_Prepend_Phi_To_BB
 * ================================================================
 */
void SSA_Prepend_Phi_To_BB (
  OP *phi_op, 
  BB *bb
) 
{
  //
  // add 'phi_op' to 'bb' before all other insts
  //
  BB_Prepend_Op(bb, phi_op);

  //
  // Some additional bookkeeping that is not done by Mk_OP
  //
  Set_PHI_Operands(phi_op);
}

//
// dominance frontier blocks for each BB in the region
//
static BB_SET **DF;

/* ================================================================
 *   compute_dominance_frontier
 *
 *   recursively traverse the dominator tree in bottom-up order.
 * ================================================================
 */
static void
compute_dominance_frontier (
  BB *bb,
  BOOL *visited
)
{
  BB *kid;

  // if we've visited this BB while processing another
  // region_entry, return with peace
  if (visited[BB_id(bb)]) return;

  // allocate the DF set
  DF[BB_id(bb)] = BB_SET_Create_Empty(PU_BB_Count+2, &ssa_pool);

  FOR_ALL_BB_SET_members(BB_children(bb), kid) {
    compute_dominance_frontier (kid, visited);
  }

  // children are done, do this one
  BB *x = bb;
  //
  // local
  //
  BBLIST *succs;
  FOR_ALL_BB_SUCCS(x, succs) {
    BB* y = BBLIST_item(succs);
    if (BB_dominator(y) != x) {
      //
      // bb does not dominate it's successor => successor
      // is in dominance frontier
      //
      DF[BB_id(x)] = BB_SET_Union1D(DF[BB_id(x)], y, &ssa_pool);
    }
  }

  //
  // up
  //
  FOR_ALL_BB_SET_members(BB_children(bb),kid) {
    BB *y, *z = kid;
    FOR_ALL_BB_SET_members(DF[BB_id(z)],y) {
      if (BB_dominator(y) != x) {
	DF[BB_id(x)] = BB_SET_Union1D(DF[BB_id(x)], y, &ssa_pool);
      }
    }
  }

  // mark as visited
  visited[BB_id(bb)] = TRUE;

  return;
}


/* ================================================================
 *   SSA_Compute_Dominance_Frontier
 *
 *   Algorithm page 466, Fig.10 of Cytron et al. paper.
 *
 *   NOTE: lst - is in the reverse order of the dominator tree.
 * ================================================================
 */
static void
SSA_Compute_Dominance_Frontier ()
{
  BB *bb;
  BOOL *visited;  //  whether DF has been computed for this BB

  if (Trace_dom_frontier) {
    fprintf(TFile, "<ssa> Build Dominance Frontier\n");
    fflush(TFile);
  }

  //
  // Initialize the dominance frontier structure
  //
  DF = (BB_SET **)TYPE_MEM_POOL_ALLOC_N(BB_SET *, 
					&ssa_pool, 
					PU_BB_Count+2 );

  visited = (BOOL *)alloca(sizeof(BOOL)*(PU_BB_Count+2));
  bzero(visited, sizeof(BOOL)*(PU_BB_Count+2));

  FOR_ALL_BB_SET_members(region_entry_set,bb) {
    compute_dominance_frontier (bb, visited);
  }

  //
  // debugging
  //
  if (Trace_dom_frontier) {
    fprintf(TFile, "<ssa> DOMINANCE FRONTIERS: \n");
    for (bb = REGION_First_BB; bb; bb = BB_next(bb)) {
      fprintf(TFile, "  BB%d : ", BB_id(bb));
      BB_SET_Print(DF[BB_id(bb)], TFile);
      fprintf(TFile, "\n--------------------\n");
    }
  }

  return;
}

/* ================================================================
 *   SSA_Place_Phi_In_BB ()
 *
 *   Make a PHI-function of the type 
 *
 *               tn = PHI (tn, tn, ... , tn)
 *
 *   where the number of 'tn's equals the number of 'bb's predecessors in
 *   the CFG, and each 'tn' refers to the corresponding BB predecessor.
 *
 * ================================================================
 */
static void
SSA_Place_Phi_In_BB (
  TN *tn, 
  BB *bb
)
{
  INT i;
  INT num_results = 1;
  INT num_opnds = BBlist_Len(BB_preds(bb));

  TN *result[1];

  // Arthur: amazingly, we can have a large number of predecessors !!
  //TN *opnd[5];
  TN **opnd = (TN**)alloca(sizeof(TN*)*num_opnds);
  bzero(opnd, sizeof(TN*)*num_opnds);
  // only 1 result possible
  result[0] = tn;

  for (i = 0; i < num_opnds; i++) {
    opnd[i] = tn;
  }

  OP *phi_op = Mk_VarOP (TOP_phi, 
			 num_results,   /* single result TN */
			 num_opnds, 
			 result, 
			 opnd);

  if (Trace_SSA_Build) {
    fprintf(TFile, "    adding phi for ");
    Print_TN(tn, FALSE);
    fprintf(TFile, " in BB%d : ", BB_id(bb));
    Print_OP(phi_op);
    //fprintf(TFile, "\n");
  }

  SSA_Prepend_Phi_To_BB (phi_op, bb);

  /*
  fprintf(TFile, "BB%d:\n", BB_id(OP_bb(phi_op)));
  for (i = 0; i < num_opnds; i++) {
    fprintf(TFile, "  pred[%d] = BB%d\n", i, BB_id(Get_PHI_Predecessor(phi_op,i)));
  }
  */

  return;
}

/* ================================================================
 *  initialize_tn_def_map
 * ================================================================
 */
static void
initialize_tn_def_map ()
{
  INT i;
  BB *bb;
  OP *op;

  tn_def_map = TN_MAP_Create();

  for (bb = REGION_First_BB; bb; bb = BB_next(bb)) {
    FOR_ALL_BB_OPs_FWD(bb, op) {

      //fprintf(TFile,"BB%d : ", BB_id(bb));
      //Print_OP(op);

      for (i = 0; i < OP_results(op); i++) {
	TN *tn = OP_result(op,i);
	BB_LIST *p = TN_is_def_in(tn);
	TN_MAP_Set(tn_def_map, tn, BB_LIST_Push(bb, p, &MEM_local_pool));
      }
    }
  }

  return;
}

/* ================================================================
 *  finalize_tn_def_map
 * ================================================================
 */
static void
finalize_tn_def_map ()
{
  TN_MAP_Delete(tn_def_map);
  return;
}

#if 0
/* ================================================================
 *   TN_is_def_in
 *
 *   Given a TN, return a BB_SET of BBs in which it is defined
 *   TODO: may be computationally expensive, perhaps do it another
 *         way.
 * ================================================================
 */
static BB_SET*
TN_is_def_in (
  TN *tn
)
{
  INT i;
  BB *bb;
  OP *op;
  BB_SET *set = BB_SET_Create_Empty(PU_BB_Count+2, &MEM_local_pool);

  for (bb = REGION_First_BB; bb; bb = BB_next(bb)) {
    FOR_ALL_BB_OPs_FWD(bb, op) {

      //fprintf(TFile,"BB%d : ", BB_id(bb));
      //Print_OP(op);

      for (i = 0; i < OP_results(op); i++) {
	if (OP_result(op,i) == tn) {
	  set = BB_SET_Union1(set, bb, &MEM_local_pool);
	}
      }
    }
  }

  return set;
}
#endif

/* ================================================================
 *   SSA_Place_Phi_Functions ()
 *
 *   Cytron et al. Fig.11, p. 470.
 *
 *   Modifications fro the article:
 *
 *      1. a phi-function is added to the BB only if the TN is live-in
 *         for that BB.
 * ================================================================
 */
static void
SSA_Place_Phi_Functions (
  TN *tn
)
{
  BB *bb;
  list<BB*> work_lst;
  list<BB*>::iterator bbi;

  // push the pool so that work and has already is not
  // accumulated in memory for all TNs
  MEM_POOL_Push(&MEM_local_pool);

  BB_SET *work = BB_SET_Create_Empty(PU_BB_Count+2, &MEM_local_pool);
  BB_LIST *p;
  for (p = TN_is_def_in(tn); p != NULL; p = BB_LIST_rest(p)) {
    work = BB_SET_Union1(work, BB_LIST_first(p), &MEM_local_pool);
  }
  //BB_SET *work = BB_SET_Copy(TN_is_def_in(tn), &MEM_local_pool);
  BB_SET *has_already = BB_SET_Create_Empty(PU_BB_Count+2, 
					    &MEM_local_pool);
  FOR_ALL_BB_SET_members(work, bb) {
    work_lst.push_back(bb);
  }

  if (Trace_SSA_Build) {
    fprintf(TFile, "\n  --> ");
    Print_TN(tn, FALSE);
    fprintf(TFile, " ");
    BB_SET_Print(work, TFile);
    fprintf(TFile, "\n");
  }

#if 0
  fprintf(TFile, "  work_lst: ");
  for (bbi = work_lst.begin(); bbi != work_lst.end(); bbi++) {
    fprintf(TFile, "    BB%d ", BB_id(*bbi));
  }
  fprintf(TFile, "\n");
#endif

  for (bbi = work_lst.begin(); bbi != work_lst.end(); bbi++) {
    BB *sc;
    FOR_ALL_BB_SET_members(DF[BB_id(*bbi)], sc) {
      //
      // if a phi-function for 'tn' has not yet been added to 'sc'
      // and 'tn' is actually live in 'sc', add the phi-function
      //

      if (Trace_SSA_Build) {
	fprintf(TFile, "    BB%d: df node BB%d: ", BB_id(*bbi), BB_id(sc));
	fprintf(TFile, " %s, %s\n", BB_SET_MemberP(has_already, sc) ? "has already" : "no phi yet", GTN_SET_MemberP(BB_live_in(sc), tn) ? "live in" : "not live in");
      }

      if (!BB_SET_MemberP(has_already, sc) &&
	                     GTN_SET_MemberP(BB_live_in(sc), tn)) {

	//fprintf(TFile, "    placing a PHI in BB%d\n", BB_id(sc));

        SSA_Place_Phi_In_BB (tn, sc);
        has_already = BB_SET_Union1(has_already, sc, &MEM_local_pool);
        if (!BB_SET_MemberP(work, sc)) {
          work = BB_SET_Union1(work, sc, &MEM_local_pool);
	  work_lst.push_back(sc);
        }
      }
    }
  }

  MEM_POOL_Pop(&MEM_local_pool);

  return;
}

/* =======================================================================
 *   Copy_TN
 *
 *   a little wrapper here to make things simpler ...
 *
 *   make a copy of 'tn' and some bookkeeping here :
 * =======================================================================
 */
static TN *
Copy_TN (
  TN *tn
)
{
  Is_True(TN_is_register(tn),("not a register tn"));

  //
  // Is supposed to return a different TN_number but else
  // identical
  //
  // TODO: TN_GLOBAL_REG flag and any spill location associated 
  //       with this TN is cleared in the new TN. Does it matter ?
  //
  TN *new_tn = Dup_TN(tn);

  return new_tn;
}

/* ================================================================
 *   Rename_Phi_Operands
 * ================================================================
 */
static void
Rename_Phi_Operands (
  BB *bb,
  BB *sc
)
{
  INT i;
  OP *op;

  FOR_ALL_BB_PHI_OPs(sc,op) {
    for (i = 0; i < OP_opnds(op); i++) {
      TN *tn = OP_opnd(op,i);
      if (Get_PHI_Predecessor(op,i) == bb) {
	TN *new_tn = tn_stack_top(tn);
	Set_OP_opnd(op,i,new_tn);
	break;
      }
    }
  }

  return;
}

/* ================================================================
 *   SSA_Rename_BB
 *
 *   Recursively visit BBs in the order of dominator tree. Every new 
 *   definition is renamed and the new name goes on top of the 
 *   tn_stack.
 *   Every use is renamed from the top of the tn_stack.
 *
 *   When we're done with the dominator tree subtree, the tn_stack 
 *   is poped.
 *
 *   At every BB, visit its CFG successors and rename operands of the
 *   PHI-functions.
 * ================================================================
 */
static void
SSA_Rename_BB (
  BB   *bb,
  BOOL *visited    // has been remaned processing another entry BB
                   // in this case only need to renema ephi-operands
                   // of its CFG successors
)
{
  INT  i;
  OP  *op;
  TN  *new_tn;

  if (visited[BB_id(bb)]) return;

#if 0
  if (Trace_SSA_Build) {
    fprintf(TFile, "  BB%d: \n", BB_id(bb));
  }
#endif

  FOR_ALL_BB_OPs_FWD(bb, op) {

    //
    // if this happens to be a phi-function, operands are renamed
    // when processing corresponding CFG predecessors
    //
    if (OP_code(op) != TOP_phi) {
      //
      // rename operands
      //
      for (i = 0; i < OP_opnds(op); i++) {
	TN *tn = OP_opnd(op,i);
	//
	// don't rename not allocatable TNs
	//
	if (!TN_is_register(tn) || TN_is_dedicated(tn)) continue;

	new_tn = tn_stack_top(tn);
	Set_OP_opnd(op, i, new_tn);
      }  /* while opnds */
    }

    //
    // push results on stack
    //
    for (i = 0; i < OP_results(op); i++) {
      TN *tn = OP_result(op,i);
      //
      // result TN must be a register ??
      // but do not touch dedicated TNs
      if (TN_can_be_renamed(tn)) {
	//
	new_tn = Copy_TN (tn);
	tn_stack_push(tn, new_tn);
#if 0
	fprintf(TFile, "  top of stack for ");
	Print_TN(tn, FALSE);
	fprintf(TFile, " is ");
	Print_TN(new_tn, FALSE);
	fprintf(TFile, "\n");
#endif
      }
    }  /* while rslts */
  } /* for all BB ops */

  //
  // rename phi-functions in this 'bb's CFG successors
  //
  BBLIST *succs;
  FOR_ALL_BB_SUCCS(bb,succs) {
    BB *succ = BBLIST_item(succs);
    Rename_Phi_Operands (bb, succ);
  }

  //
  // rename the dominated subtree of this BB
  //
  BB *kid;
  FOR_ALL_BB_SET_members(BB_children(bb),kid) {
    SSA_Rename_BB (kid, visited);
  }

  //
  // pop the tn_stack for those TNs that have been pushed
  // as many times as it's been pushed ... Must do it in the reverse
  // order of pushing since it's here where I really rename all the
  // destination TNs.
  //

  FOR_ALL_BB_OPs_REV(bb,op) {
    //
    // rename result TNs
    //
    for (i = OP_results(op)-1; i >= 0; i--) {
      TN *tn = OP_result(op,i);
      //
      // must be a register TN, but do not rename dedicated or
      // save TNs
      //
      if (TN_can_be_renamed(tn)) {
	new_tn = tn_stack_pop(tn);
	Set_OP_result(op,i,new_tn);
	Set_TN_ssa_def(new_tn, op);  // this should also include PHIs
      }
    }
  }

  // mark as visited
  visited[BB_id(bb)] = TRUE;

  return;
}

/* ================================================================
 *   SSA_Rename
 * ================================================================
 */
static void
SSA_Rename ()
{
  INT  i;
  BB  *bb;

  //
  // First, calculate the dominance frontiers
  //
  SSA_Compute_Dominance_Frontier ();

  initialize_tn_def_map();

  //
  // has given TN been processed by the renaming algorithm ?
  //
  TN_SET  *tn_seen = TN_SET_Create_Empty (Last_TN + 1,&MEM_local_pool);

  if (Trace_SSA_Build) {
    fprintf(TFile, "<ssa> Placing the PHI-nodes: \n");
  }

  for (bb = REGION_First_BB; bb; bb = BB_next(bb)) {
    OP *op;
    FOR_ALL_BB_OPs_FWD (bb, op) {
      for (i = 0; i < OP_results(op); i++) {
	TN *tn = OP_result(op, i);
	// if the TN is not renamable or has been processed, move on
	if (!TN_is_ssa_reg(tn) || TN_SET_MemberP(tn_seen, tn)) continue;

	SSA_Place_Phi_Functions (tn);
	tn_seen = TN_SET_Union1(tn_seen, tn, &MEM_local_pool);
      }
    }
  }

  if (Trace_SSA_Build) {
    Trace_IR(TP_SSA, "AFTER PHI_NODES INSERTION", NULL);
  }

  finalize_tn_def_map();

  //
  // visit nodes in the dominator tree order renaming TNs
  //

  //  if (Trace_SSA_Build) {
  //    fprintf(TFile, "<ssa> Renaming TNs: \n");
  //  }

  initialize_tn_stack();

  BOOL *visited = (BOOL *)alloca(sizeof(BOOL)*(PU_BB_Count+2));
  bzero(visited, sizeof(BOOL)*(PU_BB_Count+2));

  FOR_ALL_BB_SET_members(region_entry_set,bb) {
    SSA_Rename_BB (bb, visited);
  }

  return;
}

/* ================================================================
 *   ssa_init
 * ================================================================
 */
static void
ssa_init () {
  // Initialize ssa_pool once per CG compilation
  static BOOL initialized;

  if (!initialized) {
    MEM_POOL_Initialize (&ssa_pool, "ssa pool", TRUE);
    MEM_POOL_Initialize (&tn_map_pool, "tn map pool", TRUE);
    // So that SSA_Make_Consistent() can Pop it in order to
    // clean up memory
    MEM_POOL_Push(&tn_map_pool);
    initialized = TRUE;
  }

  return;
}

/* ================================================================
 *   SSA_Enter
 *
 *   Can be called two ways:
 *   1) on a region (pu_dst is NULL, returns code)
 *   2) on a PU (pu_dst is no NULL, returns NULL)
 *
 *   Must be called before any other SSA routine. It will prepare the
 *   data structures and build the SSA form.
 * ================================================================
 */
void
SSA_Enter (
  RID *rid, 
  BOOL region 
)
{
  Trace_SSA_Build = Get_Trace(TP_SSA, SSA_BUILD);
  Trace_dom_frontier = Get_Trace(TP_SSA, SSA_DOM_FRONT);

  if (Trace_SSA_Build) {
    fprintf(TFile, "%s CFG Before SSA_Enter\n%s\n", DBar, DBar);
    Print_All_BBs ();
  }

  ssa_init();

  //
  // Corresponding Pop is done by SSA_Remove_Phi_Nodes()
  //
  MEM_POOL_Push (&ssa_pool);

  //
  // initialize phi_op_map, deleted by the SSA_Remove_Phi_Nodes()
  //
  phi_op_map = OP_MAP_Create();

  //
  // initialize tn_ssa_map, deleted by the SSA_Remove_Phi_Nodes()
  //
  tn_ssa_map = TN_MAP_Create();

  /* Why + 2?  Nobody seems to know.
   */
  region_exit_set  = BB_SET_Create(PU_BB_Count + 2,&ssa_pool);
  region_entry_set = BB_SET_Create(PU_BB_Count + 2,&ssa_pool);

  // Initialize some auxilliary data structures
  Set_Entries_Exits(rid);

  MEM_POOL_Push (&MEM_local_pool);

  //initialize_tn_stack();
  DOM_TREE_Initialize();
  SSA_Rename();
  DOM_TREE_Finalize();

  //Trace_IR(TP_SSA, "After the SSA Conversion", NULL);

  MEM_POOL_Pop (&MEM_local_pool);

  return;
}

/* ================================================================
 *   Out-Of-SSA-Translation
 *
 *   Based on V.Sreedhar et Al.
 * ================================================================
 */

//
// Interference graph.
// Only edge information is necessary unlike the real interference
// graph with node degrees, costs, etc.
// 
// We only keep interferences for global TNs.
// Localized TN interference is found by scanning the BBs.
// It seems a reasonable tradeoff given that we are looking
// for PHI-functions interference information.
//

//
// Global TNs interference map, IGRAPH
// For each global TN gives a set of other global TNs which
// interfere with it.
//
static GTN_SET **tn_imap;

// Interference Graph Tracing
static BOOL Trace_Igraph;

/* ================================================================
 *   IGRAPH_Print
 * ================================================================
 */
static void
IGRAPH_Print (
  FILE *file
)
{
  INT i;

  fprintf (file, "  --- Interference Graph --- \n");
  for (i = 1; i < GTN_UNIVERSE_size; i++) {
    TN *tn = GTN_UNIVERSE_INT_tn(i);

    //
    // Do not include dedicated or save TNs:
    //
    if (!TN_is_ssa_reg(tn)) continue;

    fprintf(file, "  TN%d: ", TN_number(tn));
    GTN_TN_SET_Print(tn_imap[i], file);
    fprintf(file, "\n");
  }

  return;
}

/* ================================================================
 *   IGRAPH_Initialize
 * ================================================================
 */
static void
IGRAPH_Initialize ()
{
  INT i;
  BB *bb;

  //
  // GTN universe is much smaller than all TNs
  //
  INT32    size = GTN_UNIVERSE_size;

  // 
  // Initialize interference map
  //
  tn_imap = (GTN_SET **)TYPE_MEM_POOL_ALLOC_N(GTN_SET *, 
					      &ssa_pool, 
					      size+1);
  for (i = 1; i <= size; i++) {
    tn_imap[i] = GTN_SET_Create(size,&ssa_pool);
  }

  return;
}

/* ================================================================
 *   IGRAPH_Add_Interference
 * ================================================================
 */
static void
IGRAPH_Add_Interference (
  TN *tn1, 
  TN *tn2
) 
{
  //
  // Only add interference if both are GTNs
  //
  UINT32 idx1 = GTN_UNIVERSE_TN_int(tn1);
  UINT32 idx2 = GTN_UNIVERSE_TN_int(tn2);

  if (idx1 == 0 || idx2 == 0) return;

  tn_imap[idx1] = GTN_SET_Union1D (tn_imap[idx1], 
				   tn2,
				   &ssa_pool);
  tn_imap[idx2] = GTN_SET_Union1D (tn_imap[idx2], 
				   tn1,
				   &ssa_pool);
  return;
}

/* ================================================================
 *   Propagate_Interference
 * ================================================================
 */
static void
Propagate_Interference (
  BB *bb,
  BOOL *visited
)
{
  INT i;
  OP *op;

  visited[BB_id(bb)] = TRUE;

  //
  // It is a bottom-up traversal, so check the successors first
  //
  if (!BB_SET_MemberP(region_exit_set,bb) ) {
    BBLIST *succs;
    FOR_ALL_BB_SUCCS(bb, succs) {
      BB *succ = BBLIST_item(succs);
      if (visited[BB_id(succ)] == FALSE)
	Propagate_Interference (succ, visited);
    }
  }

  if (Trace_Igraph) {
    fprintf(TFile, "====== BB%d: \n", BB_id(bb));
    GRA_LIVE_Print_Liveness(bb);
    //    fprintf(TFile, "====== BB%d -- live_out: ", BB_id(bb));
    //    GTN_TN_SET_Print(BB_live_out(bb), TFile);
    //    fprintf(TFile, "\n");
  }

  GTN_SET *current = GTN_SET_Copy(BB_live_out(bb), &MEM_local_pool);

  //
  // At this point, the 'current' holds all TNs exposed
  // down from this 'bb' (still live)
  //
  FOR_ALL_BB_OPs_REV(bb,op) {

    if (Trace_Igraph) {
      fprintf(TFile, "  ");
      Print_OP_No_SrcLine(op);
    }

    // results
    for (i = 0; i < OP_results(op); i++) {
      TN *res = OP_result(op,i);

      if (!TN_is_ssa_reg(res)) continue;

      if (Trace_Igraph) {
	fprintf(TFile, "  === interferences ");
	Print_TN(res, FALSE);
	fprintf(TFile, ":");
      }

      // interference with 'current' TNs
      for (TN *tn = GTN_SET_Choose(current);
	   tn != GTN_SET_CHOOSE_FAILURE;
	   tn = GTN_SET_Choose_Next(current, tn)) {

	// do not count interference with itself
	if (!TN_is_ssa_reg(tn) || tn == res) continue;

	if (Trace_Igraph) {
	  fprintf(TFile, " ");
	  Print_TN(tn, FALSE);
	  fprintf(TFile, ",");
	}

	IGRAPH_Add_Interference(res, tn);
      }

      if (Trace_Igraph) {
	fprintf(TFile, "\n");
      }

      // remove res from 'current'
      current = GTN_SET_Difference1(current,
				    res,
				    &MEM_local_pool);

    }

    // operands

    // We add the operands to the 'current' of this BB.

    if (OP_code(op) == TOP_phi) {
      break; // finished traversal
    }
    else {
      for (i = 0; i < OP_opnds(op); i++) {
	TN *opnd = OP_opnd(op,i);
	// add to live_out set if it's a global, non dedicated
	// register TN
	if (TN_is_ssa_reg(opnd)) {
	  current = GTN_SET_Union1(current, 
				   opnd, 
				   &MEM_local_pool);
	}
      }
    }

    if (Trace_Igraph) {
      fprintf(TFile, "  current: ");
      GTN_TN_SET_Print(current, TFile);
      fprintf(TFile, "\n");
    }

  }

  if (BB_SET_MemberP(region_entry_set, bb)) {
    TN *tn1, *tn2;
    //
    // interference of defreach with liveout
    //
    for (tn1 = GTN_SET_Choose(current);
	 tn1 != GTN_SET_CHOOSE_FAILURE;
	 tn1 = GTN_SET_Choose_Next(current, tn1)) {
      for (tn2 = GTN_SET_Choose(BB_defreach_in(bb));
	   tn2 != GTN_SET_CHOOSE_FAILURE;
	   tn2 = GTN_SET_Choose_Next(BB_defreach_in(bb), tn2)) {
	IGRAPH_Add_Interference(tn1, tn2);
      }
    }
  }

  return;
}

/* ================================================================
 *   IGRAPH_Build
 * ================================================================
 */
static void
IGRAPH_Build ()
{
  BB *bb;

  //
  // Initialize IGRAPH data structures
  //
  IGRAPH_Initialize ();

  // calculate interference based on the liveness
  BOOL *visited;  // when we arrive at a BB, we set this to TRUE
                  // and go see its successors. When all successors
                  // have been seen and the BB processed, we unset
                  // to FALSE, so we know that only those BBs with
                  // visited set to TRUE are left.
  visited = (BOOL *)alloca(sizeof(BOOL)*(PU_BB_Count+2));
  bzero(visited, sizeof(BOOL)*(PU_BB_Count+2));

  FOR_ALL_BB_SET_members(region_entry_set,bb) {
    if (visited[BB_id(bb)] == FALSE)
      Propagate_Interference(bb, visited);
  }

  if (Trace_SSA_Out) {
    IGRAPH_Print(TFile);
  }

  return;
}

/* ================================================================
 *   IGRAPH_Clean
 * ================================================================
 */
static void
IGRAPH_Clean ()
{

  return;
}

/* ================================================================
 *   IGRAPH_TNs_Interfere
 * ================================================================
 */
static BOOL
IGRAPH_TNs_Interfere (
  TN *tn1,
  TN *tn2
)
{
  FmtAssert(FALSE,("not implemented"));
  return FALSE;
}

/* ================================================================
 * ================================================================
 *                  Translating Out-Of-SSA
 * ================================================================
 * ================================================================
 */

//
// Congruence Class:
//
typedef struct _phiCongruenceClass {
  TN *name;               // this class's representative TN
  ISA_REGISTER_CLASS rc;  // register class of its members
  TN_LIST *gtns;          // TNs in this class
} PHI_CONGRUENCE_CLASS;

#define PHI_CONGRUENCE_CLASS_name(cc)  (cc->name)
#define PHI_CONGRUENCE_CLASS_rc(cc)    (cc->rc)
#define PHI_CONGRUENCE_CLASS_gtns(cc)  (cc->gtns)

//
// Arthur: I associate a congruence class to each GTN.
//         Each GTN will be pointing to a pointer to a
//         CONGRUENCE_CLASS. Thus, when phiCongruenceClasses
//         are merged, it is sufficient to change the
//         phiCongruenceClass pointer so that all GTNs
//         pointing at this phiCongruenceClass are pointing
//         to the new phiCongruenceClass.
//
//         In my implementation, if a phiCongruenceClass
//         of a TN is NULL -- that really means that it only
//         contains that TN itself and nothing else.
//         Once some PHI-node resources are merged into a
//         phiCongruenceClass, that PHI-node's TNs are made 
//         to point at this phiCongruenceClass.
//

hTN_MAP phiCongruenceClass_map;
#define phiCongruenceClass(tn)  ((PHI_CONGRUENCE_CLASS *)hTN_MAP_Get(phiCongruenceClass_map,tn))
#define Set_phiCongruenceClass(tn,cc) (hTN_MAP_Set(phiCongruenceClass_map,tn,cc))

/* ================================================================
 *   PHI_CONGRUENCE_CLASS_make
 * ================================================================
 */
static PHI_CONGRUENCE_CLASS *
PHI_CONGRUENCE_CLASS_make(
  ISA_REGISTER_CLASS rclass
)
{
  PHI_CONGRUENCE_CLASS *new_cc = 
                         TYPE_MEM_POOL_ALLOC(PHI_CONGRUENCE_CLASS, 
					         &MEM_local_pool);
  PHI_CONGRUENCE_CLASS_name(new_cc) = NULL;
  PHI_CONGRUENCE_CLASS_rc(new_cc) = rclass;
  PHI_CONGRUENCE_CLASS_gtns(new_cc) = NULL;

  return new_cc;
}

/* ================================================================
 *   PHI_CONGRUENCE_CLASS_Add_GTN
 * ================================================================
 */
static void
PHI_CONGRUENCE_CLASS_Add_GTN (
  PHI_CONGRUENCE_CLASS *cc,
  TN *tn
)
{
  PHI_CONGRUENCE_CLASS_gtns(cc) = 
    TN_LIST_Push(tn,
		 PHI_CONGRUENCE_CLASS_gtns(cc),
		 &MEM_local_pool);
  return;
}

/* ================================================================
 *   PHI_CONGRUENCE_CLASS_Member
 * ================================================================
 */
static BOOL
PHI_CONGRUENCE_CLASS_Member (
  PHI_CONGRUENCE_CLASS *cc,
  TN *tn
)
{
  TN_LIST *p;
  for (p = PHI_CONGRUENCE_CLASS_gtns(cc); p != NULL; p = TN_LIST_rest(p)) {
    TN *ctn = TN_LIST_first(p);
    if (ctn == tn) return TRUE;
  }
  return FALSE;
}

/* ================================================================
 *   PHI_CONGRUENCE_CLASS_Merge
 * ================================================================
 */
static void
PHI_CONGRUENCE_CLASS_Merge (
  PHI_CONGRUENCE_CLASS *cc1,
  PHI_CONGRUENCE_CLASS *cc2
)
{
  TN_LIST *p;
  for (p = PHI_CONGRUENCE_CLASS_gtns(cc2); p != NULL; p = TN_LIST_rest(p)) {
    TN *tn = TN_LIST_first(p);
    if (!PHI_CONGRUENCE_CLASS_Member(cc1,tn)) {
      PHI_CONGRUENCE_CLASS_Add_GTN (cc1, tn);
    }
  }

  return;
}

/* ================================================================
 *   PHI_CONGRUENCE_CLASS_TN
 * ================================================================
 */
static TN *
PHI_CONGRUENCE_CLASS_TN (
  PHI_CONGRUENCE_CLASS *cc
)
{
  if (PHI_CONGRUENCE_CLASS_name(cc)) 
    return PHI_CONGRUENCE_CLASS_name(cc);

  TN *tn = Build_RCLASS_TN(PHI_CONGRUENCE_CLASS_rc(cc));
  PHI_CONGRUENCE_CLASS_name(cc) = tn;
  // This is a global TN
  //GTN_UNIVERSE_Add_TN(tn);
  //Set_TN_is_global_reg(tn);
  return tn;
}

/* ================================================================
 *   PHI_CONGRUENCE_CLASS_Print
 * ================================================================
 */
static void
PHI_CONGRUENCE_CLASS_Print (
  PHI_CONGRUENCE_CLASS *cc
)
{
  TN_LIST *p;
  fprintf(TFile, "{");
  for (p = PHI_CONGRUENCE_CLASS_gtns(cc); 
       p != NULL;
       p = TN_LIST_rest(p)) {
    Print_TN(TN_LIST_first(p), FALSE);
    fprintf(TFile," ");
  }
  fprintf(TFile, "}");
  return;
}

/* ================================================================
 *   merge_phiCongruenceClasses
 *
 *   merge the phiCongruenceClass[] corresponding to all
 *   resources of a PHI-node.
 * ================================================================
 */
static void
merge_phiCongruenceClasses (
  OP *phi_op
)
{
  INT i;
  TN *tn;

  ISA_REGISTER_CLASS rclass = TN_register_class(OP_result(phi_op,0));
  PHI_CONGRUENCE_CLASS *current = PHI_CONGRUENCE_CLASS_make(rclass);

  if (Trace_SSA_Out) {
    fprintf(TFile, "=== Merge_PhiCongruenceClasses (BB%d)", BB_id(OP_bb(phi_op)));
    Print_OP_No_SrcLine(phi_op);
  }

  for (i = 0; i < OP_opnds(phi_op); i++) {
    tn = OP_opnd(phi_op,i);
    if (phiCongruenceClass(tn) != NULL) {
      PHI_CONGRUENCE_CLASS_Merge(current, phiCongruenceClass(tn));
    }
    else {
      PHI_CONGRUENCE_CLASS_Add_GTN(current, tn);
    }

    Set_phiCongruenceClass(tn,current);
#if 0
    if (Trace_SSA_Out) {
      fprintf(TFile, "setting class for ");
      Print_TN(tn, FALSE);
      fprintf(TFile, " to class: ");
      PHI_CONGRUENCE_CLASS_Print(current);
      //GTN_TN_SET_Print(PHI_CONGRUENCE_CLASS_gtns(current), TFile);
      fprintf(TFile,"\n");
    }
#endif
  }

  for (i = 0; i < OP_results(phi_op); i++) {
    tn = OP_result(phi_op,i); 
    if (phiCongruenceClass(tn) != NULL) {
      PHI_CONGRUENCE_CLASS_Merge(current, phiCongruenceClass(tn));
    }
    else {
      PHI_CONGRUENCE_CLASS_Add_GTN(current, tn);
    }

    Set_phiCongruenceClass(tn,current);

#if 0
    if (Trace_SSA_Out) {
      fprintf(TFile, "setting class for ");
      Print_TN(tn, FALSE);
      fprintf(TFile, " to class: ");
      PHI_CONGRUENCE_CLASS_Print(current);
      //GTN_TN_SET_Print(PHI_CONGRUENCE_CLASS_gtns(current), TFile);
      fprintf(TFile,"\n");
    }
#endif
  }

  if (Trace_SSA_Out) {
    TN_LIST *p;
    //fprintf(TFile, "=== Merge_PhiCongruenceClasses ");
    //Print_OP(phi_op);
    fprintf(TFile, "  Class: ");
    for (p = PHI_CONGRUENCE_CLASS_gtns(current); 
	   p != NULL;
	   p = TN_LIST_rest(p)) {
	Print_TN(TN_LIST_first(p), FALSE);
	fprintf(TFile," ");
      }
    //GTN_TN_SET_Print(PHI_CONGRUENCE_CLASS_gtns(current), TFile);
    fprintf(TFile,"\n");
  }

  return;
}

/* ================================================================
 *   initialize_phiCongruenceClasses
 *
 *   Make a congruence class for every GTN in GTN_UNIVERSE.
 *   But allocate double of that size because there may be
 *   a potential copy OP for every PHI-node resource.
 * ================================================================
 */
static void
initialize_phiCongruenceClasses ()
{
  INT i;
  OP *op;
  BB *bb;

  phiCongruenceClass_map = hTN_MAP_Create(&MEM_local_pool);

  return;
}

/* ================================================================
 *   finalize_phiCongruenceClasses
 * ================================================================
 */
static void
finalize_phiCongruenceClasses ()
{
#if 0
  phiCongruenceClass_map_size = 0;
#endif
}

/* ================================================================
 *   repair_machine_ssa
 * ================================================================
 */
static void
repair_machine_ssa ()
{
  // not implemented
  return;
}

/* ================================================================
 *   insert_operand_copy
 *
 *   Insert a copy of given TN at the end of given BB.
 *   Maintain the up-to-date Liveness information.
 *   Must maintain the global/local attribute for TNs because this
 *   is tested by TN_is_ssa_reg(tn).
 *   Return new TN.
 * ================================================================
 */
static void
insert_operand_copy (
  OP   *phi_op,
  INT8  opnd_idx,
  BB   *in_bb
)
{
  TN *tn = OP_opnd(phi_op, opnd_idx);
  TN *new_tn = Dup_TN(tn);

  // replace old tn in the phi OP
  Set_OP_opnd(phi_op, opnd_idx, new_tn);

  // new_tn is global (because live-out of BB)
  GTN_UNIVERSE_Add_TN(new_tn);

  // update Liveness
  //GRA_LIVE_Add_Live_Out_GTN(in_bb, new_tn);

  // Remove tn from bb's LiveOut set if it is not LiveIn
  // or used in a PHI-node of any other bb's successor
  BBLIST *succs;
  FOR_ALL_BB_SUCCS(in_bb, succs) {
    BB *succ = BBLIST_item(succs);
  
    if (succ == OP_bb(phi_op)) continue;
  
    if (GTN_SET_MemberP(BB_live_in(succ), tn)) goto liveout;
  
    INT i;
    OP *op;
    FOR_ALL_BB_PHI_OPs(succ,op) {
      for (i = 0; i < OP_opnds(op); i++) {
  	if (OP_opnd(op,i) == tn && Get_PHI_Predecessor(op,i) == in_bb)
  	  goto liveout;
      }
    }
  }

  // not liveout, we should update the LiveOut set
  //GRA_LIVE_Remove_Live_Out_GTN(in_bb, tn);

  // 'tn' may no longer be global TN

  // There is no way to remove 'tn' from the GTN_UNIVERSE.
  // I may need to replace 'tn' with a new TN.
  // Let it remain for now and just reset is_global_reg 
  // attribute.
  if (!GTN_SET_MemberP(BB_live_in(Get_PHI_Predecessor(phi_op,opnd_idx)),
  	       tn))
    Reset_TN_is_global_reg(tn);

liveout:

  // Finally, append the copy op
  OPS ops = OPS_EMPTY;
  Exp_COPY(new_tn, tn, &ops);
  Set_OP_ssa_move(OPS_last(&ops));

  // Insert before the BB's last OP
  OP *point = NULL;
  if (BB_call(in_bb)) {
    // insert before the call - last OP in the 'in_bb'
    point = BB_last_op(in_bb);
  }
  else {
    point = BB_branch_op(in_bb);
  }

  if (point != NULL) {
    BB_Insert_Ops_Before(in_bb, point, &ops);
  }
  else {
    BB_Append_Ops(in_bb, &ops);
  }

  return;
}

/* ================================================================
 *   insert_result_copy
 *
 *   Insert a copy of given result TN at the beginning of given BB
 *   (right after the PHI-nodes).
 *   Maintain the up-to-date Liveness information.
 *   Must maintain the global/local attribute for TNs because this
 *   is tested by TN_is_ssa_reg(tn).
 *   Return new TN.
 * ================================================================
 */
static void
insert_result_copy (
  OP   *phi_op,
  INT8  res_idx,
  BB   *in_bb
)
{
  TN *tn = OP_result(phi_op, res_idx);
  TN *new_tn = Dup_TN(tn);

  // replace old tn in the phi OP
  Set_OP_result(phi_op, res_idx, new_tn);

  // new_tn is global (because PHI result and => live_in)
  GTN_UNIVERSE_Add_TN(new_tn);

  // update Liveness
  //GRA_LIVE_Remove_Live_In_GTN(in_bb, tn);
  //GRA_LIVE_Add_Live_In_GTN(in_bb, new_tn);

  // 'tn' may no longer be SSA

  // There is no way to remove a TN from the GTN_UNIVERSE.
  // If 'tn' is not live out of the 'in_bb', it should become
  // not global. 
  //
  if (!GTN_SET_MemberP(BB_live_out(in_bb),tn))
    // Let it remain for now and just reset is_global_reg
    // attribute (see what happens)
    Reset_TN_is_global_reg(tn);

  // Finally, insert a copy
  OPS ops = OPS_EMPTY;
  Exp_COPY(tn, new_tn, &ops);
  Set_OP_ssa_move(OPS_last(&ops));

  // find first OP after the PHI-nodes
  OP *point = NULL;
  FOR_ALL_BB_PHI_OPs(in_bb,point);
  BB_Insert_Ops_Before(in_bb, point, &ops);

  return;
}

/* ================================================================
 *   insert_copies_blindly
 * ================================================================
 */
static void
insert_copies_blindly ()
{
  INT i;
  OP *op;
  BB *bb;

  for (bb = REGION_First_BB; bb; bb = BB_next(bb)) {
    FOR_ALL_BB_PHI_OPs(bb, op) {
      // insert copies for operands
      for (i = 0; i < OP_opnds(op); i++) {
	insert_operand_copy(op, i, Get_PHI_Predecessor(op,i));
      }

      // insert copies for results
      for (i = 0; i < OP_results(op); i++) {
	insert_result_copy(op, i, bb);
      }

      // Merge phiCongruenceClass's for all PHI-node resources
      merge_phiCongruenceClasses(op);
    }
  }

  return;
}

/* ================================================================
 *   phi_resources_interfere
 * ================================================================
 */
static BOOL
phi_resources_interfere (
  PHI_CONGRUENCE_CLASS *cc1, 
  PHI_CONGRUENCE_CLASS *cc2
)
{
  TN_LIST *p1, *p2;
  TN *tn1, *tn2;

  for (p1 = PHI_CONGRUENCE_CLASS_gtns(cc1); 
       p1 != NULL;
       p1 = TN_LIST_rest(p1)) {
    tn1 = TN_LIST_first(p1);

    for (p2 = PHI_CONGRUENCE_CLASS_gtns(cc2); 
	 p2 != NULL;
	 p2 = TN_LIST_rest(p2)) {
      tn2 = TN_LIST_first(p2);

      if (IGRAPH_TNs_Interfere(tn1, tn2)) return TRUE;
    }
  }

  return FALSE;
}

/* ================================================================
 *   Eliminate_Phi_Resource_Interference
 * ================================================================
 */
static void
Eliminate_Phi_Resource_Interference()
{
  INT i,j;
  BB *bb;
  OP *op;

  /*
   * Set of candidate resources for insertign copies as described
   * in VC's paper
   */
  TN_LIST *candidateResourceSet;

  for (bb = REGION_First_BB; bb; bb = BB_next(bb)) {
    FOR_ALL_BB_PHI_OPs(bb, op) {
      PHI_CONGRUENCE_CLASS *cc1, *cc2;

      // does result interfere with any of the operands ?
      cc1 = phiCongruenceClass(OP_result(op,0));
      Is_True(cc1 != NULL, ("Empty congruence class for a SSA TN"));
	
      for (i = 0; i < OP_opnds(op); i++) {
	cc2 = phiCongruenceClass(OP_opnd(op,i));

	if (OP_result(op,0) == OP_opnd(op,i)) continue;

	if (phi_resources_interfere(cc1, cc2)) {
	  TN_LIST_Push(OP_opnd(op,i), candidateResourceSet, &MEM_local_pool);
	  TN_LIST_Push(OP_result(op,0), candidateResourceSet, &MEM_local_pool);
	}
      }

      // do operands interfere
      for (i = 0; i < OP_opnds(op); i++) {
	cc1 = phiCongruenceClass(OP_opnd(op,i));
	Is_True(cc1 != NULL, ("Empty congruence class for a SSA TN"));

	for (j = i+1; j < OP_opnds(op); j++) {
	  cc2 = phiCongruenceClass(OP_opnd(op,j));

	  if (OP_opnd(op,i) == OP_opnd(op,j)) continue;

	  if (phi_resources_interfere(cc1, cc2)) {
	    TN_LIST_Push(OP_opnd(op,i), candidateResourceSet, &MEM_local_pool);
	    TN_LIST_Push(OP_result(op,j), candidateResourceSet, &MEM_local_pool);
	  }
	}
      } // operands

      //
      // Insert necessary copies
      //
      TN_LIST *p;
      for (p = candidateResourceSet; p != NULL; p = TN_LIST_rest(p)) {
	TN *tn = TN_LIST_first(p);
	if (TN_ssa_def(tn) == op) {
	  // this PHI result
	  insert_result_copy(op, 0, bb);
	}
	else {
	  // must be one of the PHI operands
	  for (i = 0; i < OP_opnds(op); i++) {
	    if (OP_opnd(op,i) == tn) {
	      insert_operand_copy(op, i, bb);
	      break;
	    }
	  }
	}
      }

      // Merge phiCongruenceClass's for all PHI-node resources
      merge_phiCongruenceClasses(op);

    } // for all BB PHI OPs
  } // for all BBs

  return;
}

/* ================================================================
 *   map_phi_resources_to_new_names
 * ================================================================
 */
static void
map_phi_resources_to_new_names()
{
  INT i;
  TN *tn, *new_tn;
  OP *op;
  BB *bb;

  for (bb = REGION_First_BB; bb; bb = BB_next(bb)) {

    if (Trace_SSA_Out) {
      fprintf(TFile, "============ map_phi_resources_to_new_names BB%d ===========\n",
	      BB_id(bb));
    }

    FOR_ALL_BB_OPs_FWD(bb,op) {

      if (Trace_SSA_Out) {
	fprintf(TFile, " --> ");
	Print_OP_No_SrcLine(op);
	fprintf(TFile, "\n");
      }

      // if a PHI-node, do nothing - it will be removed
      if (OP_code(op) != TOP_phi) {

	for (i = 0; i < OP_opnds(op); i++) {
	  tn = OP_opnd(op,i);
	  if (TN_is_ssa_reg(tn) && (phiCongruenceClass(tn) != NULL)) {
	    if (TN_new_name(tn) == NULL) {
	      new_tn = PHI_CONGRUENCE_CLASS_TN(phiCongruenceClass(tn));
	      Set_TN_new_name(tn, new_tn);

	      if (Trace_SSA_Out) {
		Print_TN(tn, FALSE);
		fprintf(TFile," Class: ");
		PHI_CONGRUENCE_CLASS_Print(phiCongruenceClass(tn));
		//GTN_TN_SET_Print(PHI_CONGRUENCE_CLASS_gtns(phiCongruenceClass(tn)), TFile);
		fprintf(TFile,"\n");
		fprintf(TFile,"  new_name ");
		Print_TN(new_tn,FALSE);
		fprintf(TFile,"\n");
	      }
	    }
	  }
	}

	for (i = 0; i < OP_results(op); i++) {
	  tn = OP_result(op,i);
	  if (TN_is_ssa_reg(tn) && (phiCongruenceClass(tn) != NULL)) {
	    if (TN_new_name(tn) == NULL) {
	      new_tn = PHI_CONGRUENCE_CLASS_TN(phiCongruenceClass(tn));
	      Set_TN_new_name(tn, new_tn);

	      if (Trace_SSA_Out) {
		Print_TN(tn, FALSE);
		fprintf(TFile," Class: ");
		PHI_CONGRUENCE_CLASS_Print(phiCongruenceClass(tn));
		//GTN_TN_SET_Print(PHI_CONGRUENCE_CLASS_gtns(phiCongruenceClass(tn)), TFile);
		fprintf(TFile,"\n");
		fprintf(TFile,"  new_name ");
		Print_TN(new_tn,FALSE);
		fprintf(TFile,"\n");
	      }
	    }
	  }
	}
      } /* else not a TOP_phi */
    } /* FOR_ALL_BB_OPs_FWD */
  } /* for (bb = REGION_First_BB ... */

  return;
}

/* ================================================================
 *   SSA_Make_Consistent
 * ================================================================
 */
void
SSA_Make_Consistent (
  RID *rid, 
  BOOL region 
)
{
  Trace_SSA_Out = Get_Trace(TP_SSA, SSA_MAKE_CONST);
  Trace_Igraph = Get_Trace(TP_SSA, SSA_IGRAPH);

  //
  // Delete the tn_to_new_name map that may have been left
  // from a previous invocation of the routine. And initialize
  // a new one.
  //
  //if (tn_to_new_name_map != NULL)
  //  TN_MAP_Delete(tn_to_new_name_map);
  MEM_POOL_Pop(&tn_map_pool);
  // Push it again and create the map
  MEM_POOL_Push(&tn_map_pool);
  //
  // Create a map of TNs to their representative names
  // TNs are mapped to their phiCongruenceClass'es that will
  // not survive this routine (they are in MEM_local_pool).
  // This is because we want to separate removing PHI resource
  // interference from actual removing of the PHI-nodes and 
  // renaming the PHI resources. Thus, phiCongruenceClass(tn)
  // map gets replaced by the phiResourceName(tn) map.
  //
  tn_to_new_name_map = hTN_MAP_Create(&tn_map_pool);

  //
  // This one is temporary working pool (zeroes memory)
  //
  MEM_POOL_Push(&MEM_local_pool);

  if (Trace_SSA_Out) {
    Trace_IR(TP_SSA, "Before Translating Out of SSA", NULL);
  }

  //
  // First, fix interferences due to ISA/ABI constraints
  //
  repair_machine_ssa();

  //
  // Build the interference graph
  //
  // NOTE: interference graph takes into account only GTNs that
  //       exist at this point. If the GTN is no longer a GTN, 
  //       and for any newly created GTN after this point, there
  //       is no interference info.
  //
  IGRAPH_Build();

  initialize_phiCongruenceClasses();

  switch (CG_ssa_algorithm) {
  case 1: 
    insert_copies_blindly ();
    break;
  case 2:
  case 3:
    Eliminate_Phi_Resource_Interference();
    break;
  default:
    Is_True(FALSE,("specify CG_ssa_algorithm"));
  }

  if (Trace_SSA_Out) {
    Trace_IR(TP_SSA, "AFTER ELIMINATE PHI-RESOURCE INTRFERENCE", NULL);
  }

  map_phi_resources_to_new_names();

  finalize_phiCongruenceClasses();

  IGRAPH_Clean(); // do I need to clean anything ??

  MEM_POOL_Pop(&MEM_local_pool);

  // Because, the above adds new TNs, we are not entirely able to
  // maintain the live-in out information incrementally.
  // Just recompute it here.
  GRA_LIVE_Recalc_Liveness(rid);

  return;
}

/* ================================================================
 *   SSA_Remove_Phi_Nodes
 *
 *   Remove PHI-nodes renaming their resources into a representative
 *   name.
 *   IMPORTANT: - maintains the liveness info.
 *              - removes maps and pops ssa_pool, no more trace of
 *                the SSA in this region.
 * ================================================================
 */
void
SSA_Remove_Phi_Nodes (
  RID *rid, 
  BOOL region 
)
{
  INT i;
  TN *tn, *new_tn;
  OP *op;
  BB *bb;

  Trace_phi_removal = Get_Trace(TP_SSA, SSA_REMOVE_PHI);

  for (bb = REGION_First_BB; bb; bb = BB_next(bb)) {

    if (Trace_phi_removal) {
      fprintf(TFile, "============ remove_phi_nodes BB%d ===========\n",
	      BB_id(bb));
    }

    // We can't use the macro because we're deleting OPs
    op = BB_first_op(bb);
    while (op != NULL) {
      OP *next_op = OP_next(op);

      if (Trace_phi_removal) {
	fprintf(TFile, " --> ");
	Print_OP_No_SrcLine(op);
	fprintf(TFile, "\n");
      }

      // if a PHI-node, remove
      if (OP_code(op) == TOP_phi) {

#if 0
	// First, we need to remove this OP's resources from
	// live_in/live_out sets
	for (i = 0; i < OP_results(op); i++) {
	  GRA_LIVE_Remove_Live_In_GTN(bb, OP_result(op,i));
	}
	for (i = 0; i < OP_opnds(op); i++) {
	  GRA_LIVE_Remove_Live_Out_GTN(Get_PHI_Predecessor(op,i),
				       OP_opnd(op,i));
	}
#endif
	if (Trace_phi_removal) {
	  fprintf(TFile, "  removing a phi \n\n");
	  //	  Print_OP_No_SrcLine(op);
	}

	BB_Remove_Op(bb, op);
      }
      else {
	// Not a PHI-node, rename resources into representative name
	for (i = 0; i < OP_opnds(op); i++) {
	  tn = OP_opnd(op,i);

	  if (!TN_is_ssa_reg(tn)) continue;

	  new_tn = TN_new_name(tn);
	  if (new_tn != NULL) {

	    //if (TN_is_ssa_reg(tn) && phiCongruenceClass(tn) != NULL) {
	    //  new_tn = PHI_CONGRUENCE_CLASS_TN(phiCongruenceClass(tn));

	    Set_OP_opnd(op, i, new_tn);
#if 0
	    // Add new_tn to live_in set of this BB
	    GRA_LIVE_Add_Live_In_GTN(bb, new_tn);
#endif
	  }
	}

	for (i = 0; i < OP_results(op); i++) {
	  tn = OP_result(op,i);

	  if (!TN_is_ssa_reg(tn)) continue;

	  new_tn = TN_new_name(tn);
	  if (new_tn != NULL) {
	    //if (TN_is_ssa_reg(tn) && phiCongruenceClass(tn) != NULL) {
	    //  new_tn = PHI_CONGRUENCE_CLASS_TN(phiCongruenceClass(tn));

	    Set_OP_result(op, i, new_tn);
#if 0
	    // Add new_tn to live_out of this BB
	    GRA_LIVE_Add_Live_Out_GTN(bb, new_tn);
#endif
	  }
	}
      } /* else not a TOP_phi */

      op = next_op;
    }
  }

  //
  // Delete maps
  //
  FmtAssert(tn_to_new_name_map != NULL,("tn_to_new_name_map not deleted"));
  MEM_POOL_Pop(&tn_map_pool);
  TN_MAP_Delete(tn_ssa_map);
  OP_MAP_Delete(phi_op_map);

  MEM_POOL_Pop (&ssa_pool);

  //
  // SSA translation maintains liveness info (see Sreedhar's paper).
  // Normally, we shouldn't need to recompute it.
  // However, at the moment
  //   defreach_in
  //   defreach_out
  //   live_def
  //   live_use
  // are not being updated properly by the out of SSA
  // algorithm (I've been toolazy to look at it). So, for now
  // just recompute the liveness.
  // TODO: fix this.
  //
  GRA_LIVE_Recalc_Liveness(region ? rid : NULL);

  return;
}

/* ================================================================
 *   SSA_Collect_Info
 *
 *   Collect some statistics related to the SSA:
 *
 *   1. number of SSA moves that remain in the code after
 *      the register allocation;
 *
 * ================================================================
 */
void
SSA_Collect_Info (
  RID *rid, 
  BOOL region,
  INT phase
)
{
  BB *bb;
  OP *op;

  INT ssa_move_count = 0;

  if (!Get_Trace(TP_SSA, SSA_COLLECT_INFO)) return;

  // statistics are dumped to TFile
  fprintf(TFile, "%s SSA Statistics for function \"%s\" (Out-of-SSA method: Shreedhar %d)\n%s\n", DBar, Cur_PU_Name, CG_ssa_algorithm, DBar);

  if (phase == TP_ALLOC) {
    for (bb = REGION_First_BB; bb; bb = BB_next(bb)) {
      FOR_ALL_BB_OPs_FWD (bb, op) {
	if (OP_ssa_move(op)) ssa_move_count++;
      }
    }

    fprintf(TFile, "  SSA moves after LRA: %d\n", ssa_move_count);
  }

  fprintf(TFile, "%s\t End SSA Statistics \n%s\n", DBar, DBar);
  
  return;
}
