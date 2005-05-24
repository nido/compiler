
// [HK]
#if __GNUC__ >= 3
#include <list>
// using std::list;
#else
#include <list.h>
#endif // __GNUC__ >= 3

#include "defs.h"
#include "tracing.h"
#include "errors.h"
#include "mempool.h"
#include "cxx_memory.h"
#include "glob.h"
#include "bitset.h"
#ifndef TARG_ST 
#include "config_targ.h"
#else
#include "config_target.h"
#endif
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

INT32 CG_ssa_algorithm = 2;

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
  if (TN_STACK_empty(base)) {
    if (!TN_is_dedicated(base) && !TN_is_save_reg(base))
      DevWarn("Uninitialized PHI arg");
    return base;
  }
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
  BB_LIST *_M_kids;
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
  dom_map[BB_id(bb)]._M_kids = BB_LIST_Push(child, dom_map[BB_id(bb)]._M_kids, &ssa_pool);
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
    for (BB_LIST *elist = BB_children(bb); elist; elist = BB_LIST_rest(elist)) {
      BB *kid = BB_LIST_first(elist);
      fprintf(file, "%d ", BB_id(kid));
    }
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
    dom_map[i]._M_kids = NULL;
  }

  //  RID *rid = BB_rid(REGION_First_BB);
  //  CGRIN *cgrin = rid ? RID_Find_Cginfo(REGION_First_BB) : NULL;

  // 
  // The immediate dominator of BBi is the BB with the same set of
  // dominators except for BBi itself:
  //
  BB_SET *match, *check;
  match = BB_SET_Create_Empty(PU_BB_Count+2, &ssa_pool);
  check = BB_SET_Create_Empty(PU_BB_Count+2, &ssa_pool);
  for (bb = REGION_First_BB; bb; bb = BB_next(bb)) {
    BB *cur;

    // 
    // match is the BS (set of dominators) to be matched:
    //
    match = BB_SET_CopyD(match, BB_dom_set(bb), &ssa_pool);
    match = BB_SET_Difference1D(match, bb);	       // ... except bb.

    // 
    // match is also the set of dominators to check:
    //
    check = BB_SET_CopyD(check, match, &ssa_pool);  // Check all bb's dominators ...

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
  const OP *phi,
  UINT8 i
)
{
  Is_True(OP_code(phi) == TOP_phi,("not a PHI function"));
  PHI_MAP_ENTRY *entry = (PHI_MAP_ENTRY *)OP_MAP_Get(phi_op_map, phi);
  Is_True(entry != NULL,("unmapped op ?"));

  return entry->opnd_src[i];
}

/* ================================================================
 *   Get_PHI_Predecessor_Idx
 * ================================================================
 */
UINT8
Get_PHI_Predecessor_Idx (
  const OP *phi,
  BB *bb
)
{
  Is_True(OP_code(phi) == TOP_phi,("not a PHI function"));
  PHI_MAP_ENTRY *entry = (PHI_MAP_ENTRY *)OP_MAP_Get(phi_op_map, phi);
  for (INT i = 0; i < OP_opnds(phi); i++) {
    if (entry->opnd_src[i] == bb)
      return i;
  }
  FmtAssert(FALSE, ("Get_PHI_Predecessor_Idx: BB %d not a predecessor of BB %d", BB_id(bb), BB_id(OP_bb(phi))));

}

/* ================================================================
 *   Set_PHI_Predecessor
 * ================================================================
 */
void Set_PHI_Predecessor (
 const OP *phi,
 UINT8 pos,
 BB *pred
)
{
  PHI_MAP_ENTRY *entry = (PHI_MAP_ENTRY *)OP_MAP_Get(phi_op_map, phi);
  Is_True(entry != NULL,("unmapped op ?"));

  entry->opnd_src[pos] = pred;
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

/* ================================================================
 * ================================================================
 * 		Interface for PSI operations
 * ================================================================
 * ================================================================
 */


static TN *
OP_guard(OP *op) {

  if (!OP_has_predicate(op))
    return NULL;

  TN *guard_tn = OP_opnd(op, OP_PREDICATE_OPND);
  if (TN_is_true_pred(guard_tn))
    return NULL;

  return guard_tn;
}

//
//  An information is missing in the PSI operation, that is whether
//  the guard associated with an operand is active on the TRUE or
//  FALSE value. The mapping is done here.
//
typedef struct psi_map_entry {
  BOOL *true_guards;
} PSI_MAP_ENTRY;

static OP_MAP psi_op_map = NULL;

/* ================================================================
 *   Allocate_PSI_Guards
 * ================================================================
 */
static void
Allocate_PSI_Guards (
  OP   *psi
)
{
  Is_True(OP_code(psi) == TOP_psi,("not a PSI function"));
  PSI_MAP_ENTRY *entry = TYPE_MEM_POOL_ALLOC(PSI_MAP_ENTRY, 
					     &ssa_pool);

  entry->true_guards = TYPE_MEM_POOL_ALLOC_N(BOOL, 
					     &ssa_pool, 
					     PSI_opnds(psi));

  OP_MAP_Set(psi_op_map, psi, entry);
  
  return;
}

/* ================================================================
 *   PSI_guard
 * ================================================================
 */
TN*
PSI_guard (
  const OP *psi,
  UINT8 pos,
  BOOL *true_guard
)
{
  Is_True(OP_code(psi) == TOP_psi,("not a PSI function"));
  PSI_MAP_ENTRY *entry = (PSI_MAP_ENTRY *)OP_MAP_Get(psi_op_map, psi);
  Is_True(entry != NULL,("unmapped PSI op ?"));

  *true_guard = entry->true_guards[pos];
  return OP_opnd(psi, pos<<1);
}

/* ================================================================
 *   Set_PSI_guard
 * ================================================================
 */
void Set_PSI_guard (
 OP *psi,
 UINT8 pos,
 TN *guard,
 BOOL true_guard
)
{
  PSI_MAP_ENTRY *entry = (PSI_MAP_ENTRY *)OP_MAP_Get(psi_op_map, psi);
  Is_True(entry != NULL,("unmapped PSI op ?"));

  Set_OP_opnd(psi, pos<<1, guard);
  entry->true_guards[pos] = true_guard;
}

/* ================================================================
 *   Set_PSI_opnd
 * ================================================================
 */
void Set_PSI_opnd (
 OP *psi,
 UINT8 pos,
 TN *opnd
)
{
  Set_OP_opnd(psi, pos<<1+1, opnd);
}

/* ================================================================
 *   Sort_PHI_opnds
 *
 *   Sort operands of a PHI operations such that no definition of an
 *   element dominates the definition of an element on its left. We
 *   must be in PHI-conform SSA to do this, so this is equivalent to
 *   say that the associated predecessor of an operand must not
 *   dominate the associated predecessor of an operand on its left.
 *
 * ================================================================
 */
void
Sort_PHI_opnds (
 OP *phi
)
{
  // Really needed, or follow an order given in input (breadth-first
  // for example) ?

  // Use a simple insertion sort algorithm to sort the operands and
  // associated predecessors mapping.

  PHI_MAP_ENTRY *entry = (PHI_MAP_ENTRY *)OP_MAP_Get(phi_op_map, phi);
  Is_True(entry != NULL,("unmapped op ?"));

  for (int i = 1; i < OP_opnds(phi); i++) {
    TN *TN_i = OP_opnd(phi, i);
    BB *BB_i = entry->opnd_src[i];

    int j;
    // We need this loop because the dominance relation is a partial order.
    for (j = 0; !BB_SET_MemberP(BB_dom_set(entry->opnd_src[j]), BB_i); j++);
    if (i != j) {
      //      fprintf(stderr, "Reordering PHI operands (%d/%d)\n", j, OP_opnds(phi));
      for (int k = i; k > j; k--) {
	Set_OP_opnd(phi, k, OP_opnd(phi, k-1));
	entry->opnd_src[k] = entry->opnd_src[k-1];
      }
      Set_OP_opnd(phi, j, TN_i);
      entry->opnd_src[j] = BB_i;
    }
  }
}

/* ================================================================
 *   PSI_inline
 * ================================================================
 */
OP *
PSI_inline (
 OP *psi_op
)
{
  int extra_opnds = 0;
  int opndx;

  // Look if some arguments are defined by PSI operations
  for (opndx = 0; opndx < PSI_opnds(psi_op); opndx++) {
    TN *opnd_tn = PSI_opnd(psi_op, opndx);
    OP *def_op = TN_ssa_def(opnd_tn);
    if (def_op && OP_code(def_op) == TOP_psi)
      // Count the additional arguments after inlining
      extra_opnds += PSI_opnds(def_op) - 1;
  }

  // Inline all PSI operations that are arguments of this PSI
  // operation
  if (extra_opnds) {
    INT num_results = 1;
    INT num_opnds = PSI_opnds(psi_op) + extra_opnds;
    int inlined_opndx;

    TN *result[1];

    TN **opnd = (TN**)alloca(sizeof(TN*)*num_opnds*2);
    BOOL *true_guards = (BOOL *)alloca(sizeof(BOOL)*num_opnds);

    result[0] = OP_result(psi_op, 0);

    // Inline arguments of PSI operations
    for (opndx = 0, inlined_opndx = 0; opndx < PSI_opnds(psi_op); opndx++) {
      TN *opnd_tn = PSI_opnd(psi_op, opndx);
      OP *def_op = TN_ssa_def(opnd_tn);
      BOOL true_guard;
      if (def_op && OP_code(def_op) == TOP_psi) {
	for (int psi_opndx = 0; psi_opndx < PSI_opnds(def_op); psi_opndx++) {
	  opnd[inlined_opndx*2+1] = PSI_opnd(def_op, psi_opndx);
	  opnd[inlined_opndx*2] = PSI_guard(def_op, psi_opndx, &true_guard);
	  true_guards[inlined_opndx] = true_guard;
	  inlined_opndx ++;
	}
      }
      else {
	opnd[inlined_opndx*2+1] = opnd_tn;
	opnd[inlined_opndx*2] = PSI_guard(psi_op, opndx, &true_guard);
	true_guards[inlined_opndx] = true_guard;
	inlined_opndx ++;
      }
    }

    Is_True(inlined_opndx == num_opnds, ("PSI_inline: internal error"));
    
    psi_op = Mk_VarOP (TOP_psi,
		       num_results,
		       num_opnds*2,
		       result,
		       opnd);

    Allocate_PSI_Guards(psi_op);

    PSI_MAP_ENTRY *entry = (PSI_MAP_ENTRY *)OP_MAP_Get(psi_op_map, psi_op);
    for (opndx = 0; opndx < PSI_opnds(psi_op); opndx++) {
      entry->true_guards[opndx] = true_guards[opndx];
    }

  }

  return psi_op;
}

/* ================================================================
 *   PSI_reduce
 * ================================================================
 */
OP *
PSI_reduce (
 OP *psi_op
)
{
  // Remove duplicated arguments
  // Discard all arguments on the left of an unconditional definition

  int removed_opnds = 0;
  int opndx;

  for (opndx = PSI_opnds(psi_op)-1; opndx >= 0; opndx--) {
    TN *opnd_tn = PSI_opnd(psi_op, opndx);

    // Discard all arguments on the left of an unconditional
    // definition
    OP *def_op = TN_ssa_def(opnd_tn);
    if (!def_op || OP_guard(def_op) == NULL) {
      for (int i = 0; i < opndx; i++)
	Set_PSI_opnd(psi_op, i, NULL);
      removed_opnds += opndx;
      break;
    }

    // Otherwise, look for duplicate of an argument on the right
    for (int i = opndx+1; i < PSI_opnds(psi_op); i++)
      if (PSI_opnd(psi_op, i) == opnd_tn) {
	removed_opnds ++;
	Set_PSI_opnd(psi_op, i, NULL);
	break;
      }
  }

  if (removed_opnds) {
    INT num_results = 1;
    INT num_opnds = PSI_opnds(psi_op) - removed_opnds;
    int new_opndx;

    TN *result[1];

    TN **opnd = (TN**)alloca(sizeof(TN*)*num_opnds*2);
    BOOL *true_guards = (BOOL *)alloca(sizeof(BOOL)*num_opnds);

    result[0] = OP_result(psi_op, 0);

    // Remove arguments of PSI operations
    for (opndx = 0, new_opndx = 0; opndx < PSI_opnds(psi_op); opndx++) {
      TN *opnd_tn = PSI_opnd(psi_op, opndx);
      BOOL true_guard;
      if (opnd_tn == NULL) continue;
      opnd[new_opndx*2] = PSI_guard(psi_op, opndx, &true_guard);
      true_guards[new_opndx] = true_guard;
      opnd[new_opndx*2+1] = opnd_tn;
      new_opndx++;
    }

    Is_True(new_opndx == num_opnds, ("PSI_reduce: internal error"));
    
    psi_op = Mk_VarOP (TOP_psi,
		       num_results,
		       num_opnds*2,
		       result,
		       opnd);

    Allocate_PSI_Guards(psi_op);
    PSI_MAP_ENTRY *entry = (PSI_MAP_ENTRY *)OP_MAP_Get(psi_op_map, psi_op);
    for (opndx = 0; opndx < PSI_opnds(psi_op); opndx++) {
      entry->true_guards[opndx] = true_guards[opndx];
    }
  }

  return psi_op;
}

/* ================================================================
 *   Convert_PHI_to_PSI
 * ================================================================
 */
OP *
Convert_PHI_to_PSI (
 OP *phi
)
{

#ifdef Is_True_On
  /* A prerequisite is that Sort_PHI_opnds has been called on that phi
     operation. */
  for (int i = 1; i < OP_opnds(phi); i++) {
    BB *BB_i = Get_PHI_Predecessor(phi, i);
    for (int j = 0; j < i; j++) {
      BB *BB_j = Get_PHI_Predecessor(phi, j);
      Is_True(!BB_SET_MemberP(BB_dom_set(BB_j), BB_i), 
	      ("Convert_PHI_to_PSI: Incorrect PHI operands dominance relation for PSI conversion."));
    }
  }
#endif

  TN **opnd = (TN**)alloca(sizeof(TN*)*OP_opnds(phi)*2);

  for (int i = 0; i < OP_opnds(phi); i++) {
    opnd[2*i] = NULL;
    opnd[2*i+1] = OP_opnd(phi, i);
  }

  INT num_results = 1;
  TN *result[1];
  result[0] = OP_result(phi, 0);

  OP *psi_op = Mk_VarOP (TOP_psi,
			 num_results,
			 OP_opnds(phi)*2,
			 result,
			 opnd);

  // Allocate an array to associate a guard with each operand of the
  // PSI
  Allocate_PSI_Guards(psi_op);

  // Arguments that are defined on a PSI are replaced by the arguments
  // of this PSI
  // FdF 20050518: Will be called explicitely if needed
  //  psi_op = PSI_inline(psi_op);

  // Remove duplicated arguments or unreachable definitions.
  // FdF 20050518: Will be called explicitely if needed
  //  psi_op = PSI_reduce(psi_op);

  return psi_op;
}

OP *
OP_Make_movc (
  TN *guard,
  TN *dst,
  TN *src
)
{
  OPS cmov_ops = OPS_EMPTY;
  Expand_Select (dst, guard, src, dst, MTYPE_I4,
		 FALSE, &cmov_ops);
  Is_True(OPS_length(&cmov_ops) == 1, ("Make_movc: Expand_Select produced more than a single operation"));
  return OPS_first(&cmov_ops);
}

OP *
OP_Make_movcf (
  TN *guard,
  TN *dst,
  TN *src
)
{
  OPS cmov_ops = OPS_EMPTY;
  Expand_Select (dst, guard, dst, src, MTYPE_I4,
 		 FALSE, &cmov_ops);
  Is_True(OPS_length(&cmov_ops) == 1, ("Make_movcf: Expand_Select produced more than a single operation"));
  return OPS_first(&cmov_ops);
}
#if 0
OP *
SSA_Expand_Movc (
  OP *op
)
{
  TN *dst = OP_result(op, 0);
  TN *guard = OP_opnd(op, 0);
  TN *src = OP_opnd(op, 1);

  OP *expand_op;

  OPS cmov_ops = OPS_EMPTY;

  if (OP_code(op) == TOP_movc)
    Expand_Select (dst, guard, src, dst, MTYPE_I4, FALSE, &cmov_ops);
  else
    Expand_Select (dst, guard, dst, src, MTYPE_I4, FALSE, &cmov_ops);

  Is_True(OPS_length(&cmov_ops) == 1, ("Make_movcf: Expand_Select produced more than a single operation"));

  expand_op == OPS_first(&cmov_ops);

  BB_Insert_Ops_Before(OP_bb(op), op, &cmov_ops);
  BB_Remove_Op(OP_bb(op), op);

  return expand_op;
}
#endif
//
// dominance frontier blocks for each BB in the region
//
static BB_LIST **DF;
static BB_SET *DF_bb;

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

  // if we've visited this BB while processing another
  // region_entry, return with peace
  if (visited[BB_id(bb)]) return;

  // allocate the DF set
  DF[BB_id(bb)] = NULL;

  for (BB_LIST *elist = BB_children(bb); elist; elist = BB_LIST_rest(elist)) {
    BB *kid = BB_LIST_first(elist);
    compute_dominance_frontier (kid, visited);
  }

  // children are done, do this one
  BB *x = bb;
  //
  // local
  //
  BB_SET_ClearD(DF_bb);
  BBLIST *succs;
  FOR_ALL_BB_SUCCS(x, succs) {
    BB* y = BBLIST_item(succs);
    if (BB_dominator(y) != x) {
      //
      // bb does not dominate it's successor => successor
      // is in dominance frontier
      //
      DF_bb = BB_SET_Union1D(DF_bb, y, &ssa_pool);
    }
  }

  //
  // up
  //
  for (BB_LIST *elist = BB_children(bb); elist; elist = BB_LIST_rest(elist)) {
    BB *z = BB_LIST_first(elist);
    Is_True(z != x, ("Inconsistent dominator tree"));
    for (BB_LIST *dflist = DF[BB_id(z)]; dflist; dflist = BB_LIST_rest(dflist)) {
      BB *y = BB_LIST_first(dflist);
      if (BB_dominator(y) != x) {
	DF_bb = BB_SET_Union1D(DF_bb, y, &ssa_pool);
      }
    }
  }

  BB *y;
  FOR_ALL_BB_SET_members(DF_bb,y) {
    DF[BB_id(x)] = BB_LIST_Push(y, DF[BB_id(x)], &ssa_pool);
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
  }

  //
  // Initialize the dominance frontier structure
  //
  DF = (BB_LIST **)TYPE_MEM_POOL_ALLOC_N(BB_LIST *, 
					&ssa_pool, 
					PU_BB_Count+2 );

  visited = (BOOL *)alloca(sizeof(BOOL)*(PU_BB_Count+2));
  BZERO(visited, sizeof(BOOL)*(PU_BB_Count+2));

  DF_bb = BB_SET_Create_Empty(PU_BB_Count+2, &ssa_pool);

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
      for (BB_LIST *elist = DF[BB_id(bb)]; elist; elist = BB_LIST_rest(elist)) {
	BB *df = BB_LIST_first(elist);
	fprintf(TFile, "%d ", BB_id(df));
      }
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
  TN **opnd = (TN**)alloca(sizeof(TN*)*num_opnds);
  BZERO(opnd, sizeof(TN*)*num_opnds);
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
  }

  SSA_Prepend_Phi_To_BB (phi_op, bb);

#if 0
  fprintf(TFile, "BB%d:\n", BB_id(OP_bb(phi_op)));
  for (i = 0; i < num_opnds; i++) {
    fprintf(TFile, "  pred[%d] = BB%d\n", i, BB_id(Get_PHI_Predecessor(phi_op,i)));
  }
#endif

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
static BB_SET *Phi_Functions_work;
static BB_SET *Phi_Functions_has_already;
static void
SSA_Place_Phi_Functions (
  TN *tn
)
{
  BB *bb;
  std::list<BB*> work_lst;
  std::list<BB*>::iterator bbi;

  // push the pool so that work and has already is not
  // accumulated in memory for all TNs
  MEM_POOL_Push(&MEM_local_pool);

  Phi_Functions_work = BB_SET_ClearD(Phi_Functions_work);
  BB_LIST *p;
  for (p = TN_is_def_in(tn); p != NULL; p = BB_LIST_rest(p)) {
    Phi_Functions_work = BB_SET_Union1D(Phi_Functions_work, BB_LIST_first(p), &MEM_local_pool);
  }
  //BB_SET *work = BB_SET_Copy(TN_is_def_in(tn), &MEM_local_pool);
  FOR_ALL_BB_SET_members(Phi_Functions_work, bb) {
    work_lst.push_back(bb);
  }

  if (Trace_SSA_Build) {
    fprintf(TFile, "\n  --> ");
    Print_TN(tn, FALSE);
    fprintf(TFile, " ");
    BB_SET_Print(Phi_Functions_work, TFile);
    fprintf(TFile, "\n");
  }

#if 0
  fprintf(TFile, "  work_lst: ");
  for (bbi = work_lst.begin(); bbi != work_lst.end(); bbi++) {
    fprintf(TFile, "    BB%d ", BB_id(*bbi));
  }
  fprintf(TFile, "\n");
#endif

  Phi_Functions_has_already = BB_SET_ClearD(Phi_Functions_has_already);

  for (bbi = work_lst.begin(); bbi != work_lst.end(); bbi++) {
    BB *sc;
    for (BB_LIST *dflist = DF[BB_id(*bbi)]; dflist; dflist = BB_LIST_rest(dflist)) {
      sc = BB_LIST_first(dflist);
      //
      // if a phi-function for 'tn' has not yet been added to 'sc'
      // and 'tn' is actually live in 'sc', add the phi-function
      //

      if (Trace_SSA_Build) {
	fprintf(TFile, "    BB%d: df node BB%d: ", BB_id(*bbi), BB_id(sc));
	fprintf(TFile, " %s, %s\n", BB_SET_MemberP(Phi_Functions_has_already, sc) ? "has already" : "no phi yet", GTN_SET_MemberP(BB_live_in(sc), tn) ? "live in" : "not live in");
      }

      if (!BB_SET_MemberP(Phi_Functions_has_already, sc) &&
	                     GTN_SET_MemberP(BB_live_in(sc), tn)) {

	//fprintf(TFile, "    placing a PHI in BB%d\n", BB_id(sc));

        SSA_Place_Phi_In_BB (tn, sc);
        Phi_Functions_has_already = BB_SET_Union1D(Phi_Functions_has_already, sc, &MEM_local_pool);
        if (!BB_SET_MemberP(Phi_Functions_work, sc)) {
          Phi_Functions_work = BB_SET_Union1D(Phi_Functions_work, sc, &MEM_local_pool);
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

  // FdF: If tn was gra_homeable, new_tn must not be. Otherwise they
  // would share the same home location, and when code motion make the
  // two interfere, spilling one would break the other. (bug dec_amr
  // with LAO).
  //
  // TODO: Fixing it in SSA_Make_Conventional would be better. Global
  // registers marked gra_homeable to the same home location should
  // be renamed into the same global TN, or if there is an
  // interference, the gra_homeable property must be removed.

  if (TN_is_gra_homeable(tn)) {
    Reset_TN_is_gra_homeable(new_tn);
    Set_TN_spill(new_tn, NULL);
  }

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

  // We also need to rename LOOPINFO_trip_count_tn
  {
    ANNOTATION *annot = ANNOT_Get(BB_annotations(bb), ANNOT_LOOPINFO);
    if (annot) {
      LOOPINFO *info = ANNOT_loopinfo(annot);
      TN *trip_count_tn = LOOPINFO_trip_count_tn(info);
      if ((trip_count_tn != NULL) && TN_is_register(trip_count_tn)) {
	LOOPINFO_trip_count_tn(info) = tn_stack_top(trip_count_tn);
      }
    }
  }

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
  for (BB_LIST *elist = BB_children(bb); elist; elist = BB_LIST_rest(elist)) {
    BB *kid = BB_LIST_first(elist);
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
	Set_TN_ssa_def(tn, NULL);
#if 0
	fprintf(TFile, "  setting TN_ssa_def for ");
	Print_TN(new_tn, FALSE);
	fprintf(TFile, " to ");
	Print_OP(op);
#endif
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

  Phi_Functions_work = BB_SET_Create_Empty(PU_BB_Count+2, &MEM_local_pool);
  Phi_Functions_has_already = BB_SET_Create_Empty(PU_BB_Count+2, &MEM_local_pool);

  for (bb = REGION_First_BB; bb; bb = BB_next(bb)) {
    OP *op;
    FOR_ALL_BB_OPs_FWD (bb, op) {
      for (i = 0; i < OP_results(op); i++) {
	TN *tn = OP_result(op, i);
	// if the TN is not renamable or has been processed, move on
	if (!(TN_is_global_reg(tn) && TN_can_be_renamed(tn)) || 
	                         TN_SET_MemberP(tn_seen, tn)) continue;

	SSA_Place_Phi_Functions (tn);
	tn_seen = TN_SET_Union1D(tn_seen, tn, &MEM_local_pool);
      }
    }
  }

  if (Trace_SSA_Build) {
    Trace_IR(TP_SSA, "AFTER PHI_NODES INSERTION", NULL);
  }

  finalize_tn_def_map();
  initialize_tn_stack();

  BOOL *visited = (BOOL *)alloca(sizeof(BOOL)*(PU_BB_Count+2));
  BZERO(visited, sizeof(BOOL)*(PU_BB_Count+2));

  //
  // visit nodes in the dominator tree order renaming TNs
  //
  FOR_ALL_BB_SET_members(region_entry_set,bb) {
    SSA_Rename_BB (bb, visited);
  }

#if 0
  fprintf(TFile, "  TN SSA MAPPING: \n\n");
  for (INT i = First_Regular_TN; i < Last_TN; i++) {
    Print_TN(TNvec(i), FALSE);
    fprintf(TFile, "  --> ");
    if (((OP *)TN_MAP_Get(tn_ssa_map, TNvec(i)))) 
      Print_OP_No_SrcLine((OP *)TN_MAP_Get(tn_ssa_map, TNvec(i)));
    else 
      fprintf(TFile, "\n");
  }
#endif
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
  // Corresponding Pop is done by SSA_Remove_Pseudo_OPs()
  //
  MEM_POOL_Push (&ssa_pool);

  // [FdF, CM 20030926] Moved from ssa_init to this point
  // so that the pushes/popes are well-balanced
  // So that SSA_Make_Conventional() can Pop it in order to
  // clean up memory
  MEM_POOL_Push(&tn_map_pool);

  //
  // initialize phi_op_map, deleted by the SSA_Remove_Pseudo_OPs()
  //
  phi_op_map = OP_MAP_Create();

  //
  // initialize psi_op_map, deleted by the SSA_Remove_Pseudo_OPs()
  //
  psi_op_map = OP_MAP_Create();

  //
  // initialize tn_ssa_map, deleted by the SSA_Remove_Pseudo_OPs()
  //
  tn_ssa_map = TN_MAP_Create();

  /* Why + 2?  Nobody seems to know.
   */
  region_exit_set  = BB_SET_Create_Empty(PU_BB_Count + 2,&ssa_pool);
  region_entry_set = BB_SET_Create_Empty(PU_BB_Count + 2,&ssa_pool);

  // Initialize some auxilliary data structures
  Set_Entries_Exits(rid);

  MEM_POOL_Push (&MEM_local_pool);

  //initialize_tn_stack();
  DOM_TREE_Initialize();
  SSA_Rename();
  DOM_TREE_Finalize();

  if (Get_Trace(TP_TEMP, 0x8)) {
  BB *bb;
  OP *op;
  for (bb = REGION_First_BB; bb; bb = BB_next(bb)) {
    FOR_ALL_BB_OPs_FWD(bb,op) {
      if (OP_code(op) != TOP_phi)
	break;
      Sort_PHI_opnds(op); // PSI experimentation
    }
  }
  }
  //Trace_IR(TP_SSA, "After the SSA Conversion", NULL);

  MEM_POOL_Pop (&MEM_local_pool);

  return;
}

/* ================================================================
 *   SSA_Check
 *
 *   Checks if the current function can be put in SSA form. Currently,
 *   the only limitation is that no basic block have more that 255
 *   predecessors. The reason is that the number of opnds in an OP is
 *   encoded on an uINT8, and a PHI operation has as many operands as
 *   the number of predecessors of the basic block it is in.
 * ================================================================ */
BOOL
SSA_Check (
  RID *rid, 
  BOOL region 
)
{
  BB *bb;
  for (bb = REGION_First_BB; bb; bb = BB_next(bb)) {
    if (BBlist_Len(BB_preds(bb)) > 255)
      return FALSE;
  }
  return TRUE;
}

/* ================================================================
 *   SSA_Verify
 *
 *   Verify the invariant properties of the SSA representation
 *
 * ================================================================ */

void
SSA_Verify (
  RID *rid, 
  BOOL region 
)
{
  BB *bb;
  for (bb = REGION_First_BB; bb; bb = BB_next(bb)) {
    OP *phi;

    FOR_ALL_BB_PHI_OPs(bb, phi) {

      if (OP_opnds(phi) != BB_preds_len(bb)) {
        fprintf (TFile, "for bb BB%d\n", BB_id(bb));
        Print_All_BBs();
        DevAssert(FALSE, ("ssa: invalid phi\n"));
      }

      /* Check that each basic block predecessor is correctly
	 associated with a PHI operand. */

      BBLIST *edge;
      int opnd_idx;
      FOR_ALL_BB_PREDS(bb, edge) {
        BB *bp = BBLIST_item(edge);
	opnd_idx = Get_PHI_Predecessor_Idx(phi, bp);
	Is_True(Get_PHI_Predecessor(phi, opnd_idx) == bp,
		("SSA_Verify: BB %d opnd %d in PHI, inconsistency in PHI map.", BB_id(bb), opnd_idx));

	/* Check that the definition of a PHI operand dominates its
	   associated predecessor. */
	TN *opnd_tn = OP_opnd(phi, opnd_idx);
	OP *def_op = TN_ssa_def(opnd_tn);
	Is_True (!def_op || BB_SET_MemberP(BB_dom_set(bp), OP_bb(def_op)),
		 ("SSA_Verify: BB:%d opnd %d in PHI, definition does not dominate predecessor block.", BB_id(bb), opnd_idx));
      }

      /* Check that each PHI operand is correctly associated with a
	 basic block predecessor. */

      for (int opnd_idx = 0; opnd_idx < OP_opnds(phi); opnd_idx++) {
	BB *bp = Get_PHI_Predecessor(phi, opnd_idx);
	Is_True(Get_PHI_Predecessor_Idx(phi, bp) == opnd_idx,
		("SSA_Verify: BB %d opnd %d in PHI, inconsistency in PHI map.", BB_id(bb), opnd_idx));
      }
    }
  }
}

/* ================================================================
 *                Mapping TN -> SSA Universe
 *
 *    This is used to identify relevant SSA TNs during the 
 *    out-of-ssa translation process
 * ================================================================
 */

static INT32 SSA_UNIVERSE_size; // SSA universe size
                                // SSA_UNIVERSE size is calculated
                                // by initialize_ssa_translation()
                                // and it makes sure that there is
                                // enough room for all potential
                                // SSA TNs.
static INT last_tn_idx;         // temp used while building SSA_UNIV.

static TN **idx_tn_table;       // idx -> TN in SSA universe 
static hTN_MAP32 tn_idx_map;    // TN -> idx in SSA universe

#define SSA_UNIVERSE_tn_idx(t)  (hTN_MAP32_Get(tn_idx_map,t))
#define SSA_UNIVERSE_idx_tn(i)  (idx_tn_table[i])

#define TN_is_ssa_reg(t)        (TN_is_register(t) && SSA_UNIVERSE_tn_idx(t) != 0)

/* ================================================================
 *   SSA_UNIVERSE_Add_TN
 *
 *   Register TN with the SSA_UNIVERSE.
 *   Return its idx.
 * ================================================================
 */
inline INT
SSA_UNIVERSE_Add_TN (
  TN *tn
)
{
  // If already added, return it's index
  if (TN_is_ssa_reg(tn)) return SSA_UNIVERSE_tn_idx(tn);

  if (Trace_SSA_Build) {
    fprintf(TFile, "  add to universe: ");
    Print_TN(tn, FALSE);
    fprintf(TFile, "\n");
  }

  FmtAssert(TN_is_global_reg(tn),("SSA_UNIVERSE: adding a  non global operand"));

  last_tn_idx++;
  FmtAssert(last_tn_idx <= SSA_UNIVERSE_size,("SSA_UNIVERSE overflowed"));
  hTN_MAP32_Set(tn_idx_map,tn,last_tn_idx);
  idx_tn_table[last_tn_idx]=tn;
  return last_tn_idx;
}

/* ================================================================
 * ================================================================
 *                     Interference Graph
 * ================================================================
 * ================================================================
 */

//
// IGRAPH -- Interference Graph.
//
// Only edge information is necessary unlike the real interference
// graph with node degrees, costs, etc.
// 
// We only keep interferences for SSA TNs.
//

// We set a 'tn_imap' entry for each PHI-function resource.
// There may be at most as many copies made as there are operands +
// results of the PHI functions (one copy for each operand/result in
// the worst case for each PHI-function present at the beginnig of
// the SSA_Make_Conventional() process). 

//
// PHI resources interference map, IGRAPH
//
static BS **tn_imap;    // IGRAPH map, implemented as a bitset

/* ================================================================
 *   IGRAPH_init_tn
 *
 *   Initialize the tn_map -- bitvector indicating interference TNs
 * ================================================================
 */
inline void IGRAPH_init_tn (TN *tn) {
  INT idx = SSA_UNIVERSE_tn_idx(tn);
  tn_imap[idx] = BS_Create_Empty(SSA_UNIVERSE_size+1, &MEM_local_pool);
  return;
}

// Interference Graph Tracing
static BOOL Trace_Igraph;

// Whether IGRAPH is in use
static BOOL Igraph_Used = FALSE;

/* ================================================================
 *   IGRAPH_print_tn
 * ================================================================
 */
static void
IGRAPH_print_tn (
  INT i,
  FILE *file
)
{
  fprintf(file, "{");

  //
  // Interference info found ??
  //
  if (tn_imap[i] != NULL) {
    BS_ELT idx;
    for (idx = BS_Choose(tn_imap[i]);
	 idx != BS_CHOOSE_FAILURE;
	 idx = BS_Choose_Next(tn_imap[i], idx)) {

      fprintf(file, " TN%d", TN_number(SSA_UNIVERSE_idx_tn(idx)));
    }
  }

  fprintf(file, " }\n");

  return;
}

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

  fprintf (file, "   last_tn_idx = %d\n", last_tn_idx);

  for (i = 1; i <= last_tn_idx; i++) {
    TN *tn = SSA_UNIVERSE_idx_tn(i);

    if (tn != NULL) {
      fprintf(file, "  TN%d: ", TN_number(tn));
      IGRAPH_print_tn(i, file);
    }
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
  OP *op;
  BB *bb;

  // SSA_UNIVERSE_size is calculated by the initialize_ssa_translation

  tn_imap = (BS **)TYPE_MEM_POOL_ALLOC_N(BS *, 
					&MEM_local_pool, 
					2*SSA_UNIVERSE_size+1);
  // initialize interference to NULL
  BZERO(tn_imap, sizeof(BS*)*2*SSA_UNIVERSE_size+1);

  Igraph_Used = TRUE;

  return;
}

/* ================================================================
 *   IGRAPH_Clean
 * ================================================================
 */
static void
IGRAPH_Clean ()
{

  // normally everything will go away with the MEM_local_pool.

  Igraph_Used = FALSE;
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
  INT32 idx1 = SSA_UNIVERSE_tn_idx(tn1);
  INT32 idx2 = SSA_UNIVERSE_tn_idx(tn2);

  Is_True(idx1 != 0,("TN%d not a SSA TN", TN_number(tn1)));
  Is_True(idx2 != 0,("TN%d not a SSA TN", TN_number(tn2)));

  if (Trace_Igraph) {
    fprintf(TFile, "    IGRAPH_Add_Interference: ");
    Print_TN(tn1, FALSE);
    fprintf(TFile, " [%d] and ", idx1);
    Print_TN(tn2, FALSE);
    fprintf(TFile, " [%d] \n", idx2);
    fflush(TFile);
  }

  //
  // If no interferences have been found before, initialize
  // the structure
  //
  if (tn_imap[idx1] == NULL) IGRAPH_init_tn(tn1);
  if (tn_imap[idx2] == NULL) IGRAPH_init_tn(tn2);

  tn_imap[idx1] = BS_Union1D (tn_imap[idx1], idx2, &MEM_local_pool);
  tn_imap[idx2] = BS_Union1D (tn_imap[idx2], idx1, &MEM_local_pool);

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
    fprintf(TFile, "====== \n");
  }

  GTN_SET *current = GTN_SET_Copy(BB_live_out(bb), &MEM_local_pool);

  //
  // At this point, the 'current' holds all TNs exposed
  // down from this 'bb' (still live)
  //
  FOR_ALL_BB_OPs_REV(bb,op) {
    TN *res;

    if (Trace_Igraph) {
      fprintf(TFile, "  ");
      Print_OP_No_SrcLine(op);
      fprintf(TFile, "  current before op: ");
      GTN_TN_SET_Print(current, TFile);
      fprintf(TFile, "\n");
    }

    if (OP_code(op) == TOP_phi) {
      res = OP_result(op,0);

      for (TN *tn = GTN_SET_Choose(current);
	   tn != GTN_SET_CHOOSE_FAILURE;
	   tn = GTN_SET_Choose_Next(current, tn)) {

	// do not count interference with itself
	if (!TN_is_ssa_reg(tn) || tn == res) continue;
	IGRAPH_Add_Interference(res, tn);
      }

      //
      // A PHI adds its result to the 'current', not operands
      //
      if (Trace_Igraph) {
	fprintf(TFile, "  add res to current: ");
	Print_TN(res, FALSE);
	fprintf(TFile, "\n");
      }
      current = GTN_SET_Union1(current, res, &MEM_local_pool);
      if (Trace_Igraph) {
	fprintf(TFile, "  current after op: ");
	GTN_TN_SET_Print(current, TFile);
	fprintf(TFile, "\n");
      }
      continue;
    }

    // results
    for (i = 0; i < OP_results(op); i++) {
      res = OP_result(op,i);

      if (!TN_is_ssa_reg(res)) continue;

      // interference with 'current' TNs
      for (TN *tn = GTN_SET_Choose(current);
	   tn != GTN_SET_CHOOSE_FAILURE;
	   tn = GTN_SET_Choose_Next(current, tn)) {

	// do not count interference with itself
	if (!TN_is_ssa_reg(tn) || tn == res) continue;
	IGRAPH_Add_Interference(res, tn);
      }

      // remove res from 'current'
      if (Trace_Igraph) {
	fprintf(TFile, "  remove res from current: ");
	Print_TN(res, FALSE);
	fprintf(TFile, "\n");
      }
      current = GTN_SET_Difference1(current,
				    res,
				    &MEM_local_pool);

    }

    // operands

    // We add the operands to the 'current' of this BB.
    for (i = 0; i < OP_opnds(op); i++) {
      TN *opnd = OP_opnd(op,i);
      // add to live_out set if it's a global, non dedicated
      // register TN
      if (TN_is_ssa_reg(opnd)) {
	if (Trace_Igraph) {
	  fprintf(TFile, "  add opnd to current: ");
	  Print_TN(opnd, FALSE);
	  fprintf(TFile, "\n");
	}
	current = GTN_SET_Union1(current, 
				 opnd, 
				 &MEM_local_pool);
      }
    }

    if (Trace_Igraph) {
      fprintf(TFile, "  current after op: ");
      GTN_TN_SET_Print(current, TFile);
      fprintf(TFile, "\n");
    }

  } // FOR_ALL_BB_OPs_REV

#if 0
  if (BB_SET_MemberP(region_entry_set, bb)) {
    TN *tn1, *tn2;
    //
    // interference of defreach with liveout
    //
    for (tn1 = GTN_SET_Choose(current);
	 tn1 != GTN_SET_CHOOSE_FAILURE;
	 tn1 = GTN_SET_Choose_Next(current, tn1)) {

      if (!TN_is_ssa_reg(tn1)) continue;

      for (tn2 = GTN_SET_Choose(BB_defreach_in(bb));
	   tn2 != GTN_SET_CHOOSE_FAILURE;
	   tn2 = GTN_SET_Choose_Next(BB_defreach_in(bb), tn2)) {

	if (!TN_is_ssa_reg(tn2)) continue;

	IGRAPH_Add_Interference(tn1, tn2);
      }
    }
  }
#endif

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

  if (Trace_Igraph) {
    fprintf(TFile, "%s\t\t\t IGRAPH_Build \n%s\n", DBar, DBar);
  }

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
  BZERO(visited, sizeof(BOOL)*(PU_BB_Count+2));

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
 *   IGRAPH_TNs_Interfere
 * ================================================================
 */
static BOOL
IGRAPH_TNs_Interfere (
  TN *tn1,
  TN *tn2
)
{
  INT32 idx1 = SSA_UNIVERSE_tn_idx(tn1);
  INT32 idx2 = SSA_UNIVERSE_tn_idx(tn2);

  Is_True(idx1 != 0,("TN%d not a SSA TN", TN_number(tn1)));
  Is_True(idx2 != 0,("TN%d not a SSA TN", TN_number(tn2)));

  //
  // tn_imap[idx] == NULL means no interference found !
  //
  if (tn_imap[idx1] == NULL || tn_imap[idx2] == NULL)
    return FALSE;

  if (BS_MemberP(tn_imap[idx1],idx2)) {
    return TRUE;
  }

  return FALSE;
}

/* ================================================================
 * ================================================================
 *                     Congruence Class
 * ================================================================
 * ================================================================
 */

//
// Congruence Class:
//
typedef struct _phiCongruenceClass {
  ISA_REGISTER_CLASS rc;  // register class of its members
  TN_LIST *gtns;          // TNs in this class
} PHI_CONGRUENCE_CLASS;

#define PHI_CONGRUENCE_CLASS_rc(cc)    (cc->rc)
#define PHI_CONGRUENCE_CLASS_gtns(cc)  (cc->gtns)

//
// Arthur: 
//         SSA related TNs will be pointing to a pointer to a
//         CONGRUENCE_CLASS. Thus, when phiCongruenceClasses
//         are merged, it is sufficient to change the
//         phiCongruenceClass pointer so that all TNs
//         pointing at this phiCongruenceClass are pointing
//         to the new phiCongruenceClass.
//

static PHI_CONGRUENCE_CLASS** phiCongruenceClass_map;

#define phiCongruenceClass(t) \
                       (phiCongruenceClass_map[SSA_UNIVERSE_tn_idx(t)])
#define Set_phiCongruenceClass(t,cc) \
                  (phiCongruenceClass_map[SSA_UNIVERSE_tn_idx(t)] = cc)

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
  PHI_CONGRUENCE_CLASS_rc(new_cc) = rclass;
  PHI_CONGRUENCE_CLASS_gtns(new_cc) = NULL;

  return new_cc;
}

/* ================================================================
 *   PHI_CONGRUENCE_CLASS_Add_TN
 * ================================================================
 */
static void
PHI_CONGRUENCE_CLASS_Add_TN (
  PHI_CONGRUENCE_CLASS *cc,
  TN *tn
)
{
  PHI_CONGRUENCE_CLASS_gtns(cc) = 
    TN_LIST_Push(tn,
		 PHI_CONGRUENCE_CLASS_gtns(cc),
		 &MEM_local_pool);

  // Make this TN point to this congruence class
  Is_True(TN_is_ssa_reg(tn),("TN is not SSA register"));
  Set_phiCongruenceClass(tn,cc);

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
 *
 *   NOTE: For the user, the two congruence classes are destroyed.
 *   Implementation, however, reuses the cc1 to save space and
 *   time.
 * ================================================================
 */
static PHI_CONGRUENCE_CLASS*
PHI_CONGRUENCE_CLASS_Merge (
  PHI_CONGRUENCE_CLASS *cc1,
  PHI_CONGRUENCE_CLASS *cc2
)
{
  TN_LIST *p;
  for (p = PHI_CONGRUENCE_CLASS_gtns(cc2); p != NULL; p = TN_LIST_rest(p)) {
    TN *tn = TN_LIST_first(p);
    if (!PHI_CONGRUENCE_CLASS_Member(cc1,tn)) {
      PHI_CONGRUENCE_CLASS_Add_TN (cc1, tn);
    }
  }

  return cc1;
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
  // In order to reduce the number of TNs, use the first TN
  // in the congruence class as its representative name:
  //
  Is_True(cc != NULL,("empty congruence class"));
  return TN_LIST_first(PHI_CONGRUENCE_CLASS_gtns(cc));
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
 *   merge_psiCongruenceClasses
 *
 *   merge the psiCongruenceClass[] corresponding to all
 *   resources of a PSI-node.
 * ================================================================
 */
static void
merge_psiCongruenceClasses (
  OP *psi_op
)
{
  INT i;
  TN *tn;

  ISA_REGISTER_CLASS rclass = TN_register_class(OP_result(psi_op,0));
  PHI_CONGRUENCE_CLASS *current = PHI_CONGRUENCE_CLASS_make(rclass);

  if (Trace_SSA_Out) {
    fprintf(TFile, "=== Merge_PsiCongruenceClasses (BB%d)", BB_id(OP_bb(psi_op)));
    Print_OP_No_SrcLine(psi_op);
  }

  for (i = 0; i < PSI_opnds(psi_op); i++) {
    tn = PSI_opnd(psi_op,i);
    Is_True(phiCongruenceClass(tn) != NULL,("empty congruence class"));
    current = PHI_CONGRUENCE_CLASS_Merge(current, phiCongruenceClass(tn));
  }

  tn = OP_result(psi_op,0); 
  Is_True(phiCongruenceClass(tn) != NULL,("empty congruence class"));
  current = PHI_CONGRUENCE_CLASS_Merge(current, phiCongruenceClass(tn));

  if (Trace_SSA_Out) {
    TN_LIST *p;
    fprintf(TFile, "  Class: ");
    PHI_CONGRUENCE_CLASS_Print(current);
    fprintf(TFile,"\n");
  }

  return;
}

/* ================================================================
 *   SSA_Make_PSI_Conventional
 * ================================================================
 */
static void
SSA_Make_PSI_Conventional ()
{
  BB *bb;
  OP *op;

  /* Eliminate_Psi_Resource_Interference() */

  for (bb = REGION_First_BB; bb; bb = BB_next(bb)) {
    FOR_ALL_BB_OPs(bb, op) {
      if (OP_code(op) != TOP_psi)
	continue;

      /* In e first pass, just support speculated operands that must
	 be predicated. */

      for (int opndx = 1; opndx < PSI_opnds(op); opndx++) {
	TN *opnd_tn = PSI_opnd(op, opndx);
	BOOL true_guard;
	TN *psi_guard = PSI_guard(op, opndx, &true_guard);
	OP *def_op = TN_ssa_def(opnd_tn);

	if (!def_op || OP_code(def_op) == TOP_psi)
	  continue;

	if (OP_guard(def_op)) {
	  /* Currently, we do not support "partial" speculation, which
	     means speculation that do not break the semantics of the
	     PSI operation. */
	  TN *op_guard = OP_guard(def_op);

	  Is_True(psi_guard == op_guard && true_guard, ("Inconsistent predicate on psi argument and its definition."));
	  continue;
	}

	/* The definition cannot be guarded or has been
	   speculated. Introduce a predicated move instruction. */
	TN *mov_tn = Dup_TN(opnd_tn);
	OP *mov_op = true_guard ? OP_Make_movc(psi_guard, mov_tn, opnd_tn) : OP_Make_movcf(psi_guard, mov_tn, opnd_tn);
	BB_Insert_Op_Before(OP_bb(op), op, mov_op);
	Set_PSI_opnd(op, opndx, mov_tn);
      }

      merge_psiCongruenceClasses(op);
    }
  }
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
    Is_True(phiCongruenceClass(tn) != NULL,("empty congruence class"));
    current = PHI_CONGRUENCE_CLASS_Merge(current, phiCongruenceClass(tn));

#if 0
    if (Trace_SSA_Out) {
      fprintf(TFile, "setting class for ");
      Print_TN(tn, FALSE);
      fprintf(TFile, " to class: ");
      PHI_CONGRUENCE_CLASS_Print(current);
      fprintf(TFile,"\n");
    }
#endif
  }

  tn = OP_result(phi_op,0); 
  Is_True(phiCongruenceClass(tn) != NULL,("empty congruence class"));
  current = PHI_CONGRUENCE_CLASS_Merge(current, phiCongruenceClass(tn));

#if 0
  if (Trace_SSA_Out) {
    fprintf(TFile, "setting class for ");
    Print_TN(tn, FALSE);
    fprintf(TFile, " to class: ");
    PHI_CONGRUENCE_CLASS_Print(current);
    fprintf(TFile,"\n");
  }
#endif

  if (Trace_SSA_Out) {
    TN_LIST *p;
    fprintf(TFile, "  Class: ");
    PHI_CONGRUENCE_CLASS_Print(current);
    fprintf(TFile,"\n");
  }

  return;
}

/* ================================================================
 * ================================================================
 *                  Translating Out-Of-SSA
 *
 *   based on V.C. Shridhar's algorithm
 * ================================================================
 * ================================================================
 */

/* ================================================================
 *   SSA_UNIVERSE_Initialize
 *
 *   Initialize PhiCongruenceClasses, IGRAPH, if necessary, etc.
 * ================================================================
 */
static void
SSA_UNIVERSE_Initialize () 
{
  INT i;
  OP *op;
  BB *bb;

  SSA_UNIVERSE_size = 0;

  //
  // Traverse the CFG twice:
  //
  //   1. count the number of PHI-nodes operands+results. 
  //      Twice this+1 is the size of BS to allocate
  //      for each IGRAPH entry; and also how many entries there will
  //      be. This accounts for all potential copy TNs.
  //
  //   2. Initialize the SSA_UNIVERSE;
  //   3. initialize PhiCongruenceClasses for all PHI
  //      resources;
  //   4. Add appropriate TNs to the SSA_UNIVERSE.
  //

  // Calculate the SSA_UNIVERSE_size
  for (bb = REGION_First_BB; bb; bb = BB_next(bb)) {
    FOR_ALL_BB_OPs(bb, op) {
      if (OP_code(op) == TOP_phi || OP_code(op) == TOP_psi)
	SSA_UNIVERSE_size += OP_opnds(op) + OP_results(op);
    }
  }
  SSA_UNIVERSE_size *= 2;

  // Initialize the SSA_UNIVERSE stuff
  idx_tn_table = (TN **)TYPE_MEM_POOL_ALLOC_N(TN *, 
					&MEM_local_pool, 
					SSA_UNIVERSE_size+1);

  tn_idx_map = hTN_MAP32_Create(&MEM_local_pool);

  //
  // initialize last_tn_idx
  // 
  last_tn_idx = 0;

  // Initialize the PHI_CONGRUENCE_CLASS stuff:
  phiCongruenceClass_map = 
     (PHI_CONGRUENCE_CLASS **)TYPE_MEM_POOL_ALLOC_N(PHI_CONGRUENCE_CLASS *, 
					&MEM_local_pool, 
					SSA_UNIVERSE_size+1);

  // Do the second traversal:
  for (bb = REGION_First_BB; bb; bb = BB_next(bb)) {
    FOR_ALL_BB_OPs(bb, op) {
      if (OP_code(op) != TOP_phi && OP_code(op) != TOP_psi)
	continue;
      //
      // Initialize PhiCongruenceClasses for PHI resources;
      // Add PHI resources to the SSA_UNIVERSE
      //
      if (phiCongruenceClass(OP_result(op,0)) == NULL) {
	// first, add this TN to the SSA universe
	SSA_UNIVERSE_Add_TN(OP_result(op,0));
	// create a congruence class
	PHI_CONGRUENCE_CLASS *cc = 
	  PHI_CONGRUENCE_CLASS_make(TN_register_class(OP_result(op,0)));
	PHI_CONGRUENCE_CLASS_Add_TN(cc, OP_result(op,0));
      }

      for (i = 0; i < OP_opnds(op); i++) {
	TN *tn = OP_opnd(op,i);
	// first, add this TN to the SSA universe
	SSA_UNIVERSE_Add_TN(tn);
	if (phiCongruenceClass(tn) == NULL) {
	  PHI_CONGRUENCE_CLASS *cc = 
	       PHI_CONGRUENCE_CLASS_make(TN_register_class(tn));
	  PHI_CONGRUENCE_CLASS_Add_TN(cc, tn);
	}
      }
    }
  }

  //
  // Delete the tn_to_new_name map that may have been left
  // from a previous invocation of the routine. And initialize
  // a new one.
  //
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
 *   SSA_UNIVERSE_Finalize
 * ================================================================
 */
static void
SSA_UNIVERSE_Finalize () 
{
  //
  // Before the CongruenceClasses are destroyed, we need to map
  // all involved TNs onto CongruenceClass representative names.
  //
  map_phi_resources_to_new_names();

  // reset the SSA_UNIVERSE_size.
  // The SSA data structures will go away with MEM_local_pool Pop.
  SSA_UNIVERSE_size = 0;

  return;
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
 *   NOT:Must maintain the global/local attribute for TNs because this
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
  //FdF 20041014: Fix for DDTS MBTst19376. Register must be
  //global. Also, use Copy_TN instead of Dup_TN, to reset the
  //gra_homeable value.
  TN *new_tn = Copy_TN(tn);
  Set_TN_is_global_reg(new_tn);

  // Make it an SSA TN
  SSA_UNIVERSE_Add_TN(new_tn);

  // replace old tn in the phi OP
  Set_OP_opnd(phi_op, opnd_idx, new_tn);

#if 0
  // new_tn is global (because live-out of BB)
  GTN_UNIVERSE_Add_TN(new_tn);

  // update Liveness
  GRA_LIVE_Add_Live_Out_GTN(in_bb, new_tn);
#endif

#if 0
  // Reset TN SSA attribute if it is not LiveIn
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

#if 0
  // not liveout, we should update the LiveOut set
  GRA_LIVE_Remove_Live_Out_GTN(in_bb, tn);
#endif

#if 0
  // 'tn' may no longer be SSA TN.
  // There is no way to remove 'tn' from the GTN_UNIVERSE.
  // I may need to replace 'tn' with a new TN.
  // Let it remain for now and just reset is_global_reg 
  // attribute.
  if (!GTN_SET_MemberP(BB_live_in(Get_PHI_Predecessor(phi_op,opnd_idx)),
  	       tn))
  //Reset_TN_is_global_reg(tn);
#endif

liveout:
#endif

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

  // Create a congruence class for new_tn
  PHI_CONGRUENCE_CLASS *cc = 
    PHI_CONGRUENCE_CLASS_make(TN_register_class(new_tn));
  //Set_phiCongruenceClass(new_tn,cc);
  PHI_CONGRUENCE_CLASS_Add_TN(cc,new_tn);

  // Update interference graph
  if (Igraph_Used) {
    // Build interference edges between new_tn and live_out[in_bb]
    for (tn = GTN_SET_Choose(BB_live_out(in_bb));
	 tn != GTN_SET_CHOOSE_FAILURE;
	 tn = GTN_SET_Choose_Next(BB_live_out(in_bb), tn)) {
      if (TN_is_ssa_reg(tn)) IGRAPH_Add_Interference(tn, new_tn);
    }
  }

  return;
}

/* ================================================================
 *   insert_result_copy
 *
 *   Insert a copy of PHI result TN at the beginning of given BB
 *   (right after the PHI-nodes).
 *   Update interference graph.
 *
 *   NOTE: Must maintain the global/local attribute for TNs because 
 *         this is tested by TN_is_ssa_reg(tn).
 *
 *   Return new TN.
 * ================================================================
 */
static void
insert_result_copy (
  OP   *phi_op,
  BB   *in_bb
)
{
  TN *tn = OP_result(phi_op, 0);
  //FdF 20041014: Fix for DDTS MBTst19376. Register must be
  //global. Also, use Copy_TN instead of Dup_TN, to reset the
  //gra_homeable value.
  TN *new_tn = Copy_TN(tn);
  Set_TN_is_global_reg(new_tn);

  // Make it an SSA TN
  SSA_UNIVERSE_Add_TN(new_tn);

  // replace old tn in the phi OP
  Set_OP_result(phi_op, 0, new_tn);

#if 0
  // new_tn is global (because PHI result and => live_in)
  GTN_UNIVERSE_Add_TN(new_tn);

  // update Liveness
  GRA_LIVE_Remove_Live_In_GTN(in_bb, tn);
  GRA_LIVE_Add_Live_In_GTN(in_bb, new_tn);
#endif

#if 0
  // 'tn' may no longer be SSA.
  // There is no way to remove a TN from the GTN_UNIVERSE.
  // If 'tn' is not live out of the 'in_bb', it should become
  // not global. 
  //
  if (!GTN_SET_MemberP(BB_live_out(in_bb),tn))
    Reset_TN_is_ssa_reg(tn);
    //Reset_TN_is_global_reg(tn);
#endif

  // Finally, insert a copy
  OPS ops = OPS_EMPTY;
  Exp_COPY(tn, new_tn, &ops);
  Set_OP_ssa_move(OPS_last(&ops));

  // find first OP after the PHI-nodes
  OP *point = NULL;
  FOR_ALL_BB_PHI_OPs(in_bb,point);
  BB_Insert_Ops_Before(in_bb, point, &ops);

  // Create a congruence class for new_tn
  PHI_CONGRUENCE_CLASS *cc = 
    PHI_CONGRUENCE_CLASS_make(TN_register_class(new_tn));
  //Set_phiCongruenceClass(new_tn,cc);
  PHI_CONGRUENCE_CLASS_Add_TN(cc,new_tn);

  // Update interference graph
  if (Igraph_Used) {
    // Build interference edges between new_tn and live_in[in_bb]
    for (tn = GTN_SET_Choose(BB_live_in(in_bb));
	 tn != GTN_SET_CHOOSE_FAILURE;
	 tn = GTN_SET_Choose_Next(BB_live_in(in_bb), tn)) {
      if (TN_is_ssa_reg(tn)) IGRAPH_Add_Interference(tn, new_tn);
    }
  }

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

      // insert copies for result
      insert_result_copy(op, bb);

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

  /* No interference between variables in a congruence class. */
  if (cc1 == cc2)
    return FALSE;

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

  if (Trace_SSA_Out) {
    fprintf(TFile, "-----------------------------------------------\n");
    fprintf(TFile, "        Eliminate_Phi_Resource_Interference    \n");
    fprintf(TFile, "-----------------------------------------------\n");
  }

  for (bb = REGION_First_BB; bb; bb = BB_next(bb)) {
    FOR_ALL_BB_PHI_OPs(bb, op) {
      PHI_CONGRUENCE_CLASS *cc1, *cc2;

      /*
       * Set of candidate resources for inserting copies as described
       * in VC's paper
       */
      TN_LIST *candidateResourceSet = NULL;

      // does result interfere with any of the operands ?
      cc1 = phiCongruenceClass(OP_result(op,0));
      Is_True(cc1 != NULL,("empty congruence class for TN%d", TN_number(OP_result(op,0))));

#if 0
      if (Trace_SSA_Out) {
	fprintf(TFile, "<SSA>    ");
	Print_OP(op);
	fprintf(TFile, "          result CC: ");
	PHI_CONGRUENCE_CLASS_Print(cc1);
	fprintf(TFile,"\n");
      }
#endif

      for (i = 0; i < OP_opnds(op); i++) {
	cc2 = phiCongruenceClass(OP_opnd(op,i));
	Is_True(cc1 != NULL,("empty congruence class"));
#if 0
	if (Trace_SSA_Out) {
	  fprintf(TFile, "          opnd %d CC: ", i);
	  PHI_CONGRUENCE_CLASS_Print(cc2);
	  fprintf(TFile,"\n");
	}
#endif
	if (phi_resources_interfere(cc1, cc2)) {
	  candidateResourceSet = TN_LIST_Push(OP_opnd(op,i), candidateResourceSet, &MEM_local_pool);
	  candidateResourceSet = TN_LIST_Push(OP_result(op,0), candidateResourceSet, &MEM_local_pool);
#if 0
	  if (Trace_SSA_Out) {
	    fprintf(TFile, "          found interference\n");
	  }
#endif
	}
      }

      // do operands interfere
      for (i = 0; i < OP_opnds(op); i++) {
	cc1 = phiCongruenceClass(OP_opnd(op,i));
	Is_True(cc1 != NULL,("empty congruence class"));

	for (j = i+1; j < OP_opnds(op); j++) {
	  cc2 = phiCongruenceClass(OP_opnd(op,j));
	  Is_True(cc1 != NULL,("empty congruence class"));

	  // do not add to candidateResourceSet if the same
	  if (OP_opnd(op,i) == OP_opnd(op,j)) continue;

	  if (phi_resources_interfere(cc1, cc2)) {
	    candidateResourceSet = TN_LIST_Push(OP_opnd(op,i), candidateResourceSet, &MEM_local_pool);
	    candidateResourceSet = TN_LIST_Push(OP_opnd(op,j), candidateResourceSet, &MEM_local_pool);
	  }
	}
      } // operands

      //
      // Insert necessary copies
      //
      for (TN_LIST *p = candidateResourceSet; 
	   p != NULL; 
	   p = TN_LIST_rest(p)) {
	TN *tn = TN_LIST_first(p);

	if (TN_ssa_def(tn) == op) {
	  // this is a PHI result
#if 0
	  if (Trace_SSA_Out) {
	    fprintf(TFile, "<SSA> Inserting a copy for ");
	    Print_TN(tn, FALSE);
	    fprintf(TFile, " in BB%d\n", BB_id(bb));
	  }
#endif
	  insert_result_copy(op, bb);
	}
	else {
	  // must be one of the PHI operands
	  for (i = 0; i < OP_opnds(op); i++) {
	    if (OP_opnd(op,i) == tn) {
#if 0
	      if (Trace_SSA_Out) {
		fprintf(TFile, "<SSA> Inserting a copy for ");
		Print_TN(tn, FALSE);
		fprintf(TFile, " in BB%d\n", BB_id(Get_PHI_Predecessor(op,i)));
	      }
#endif
	      insert_operand_copy(op, i, Get_PHI_Predecessor(op,i));
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
 *   Eliminate_Phi_Resource_Interference
 * ================================================================
 */
static void
Eliminate_Psi_Resource_Interference()
{
  INT i,j;
  BB *bb;
  OP *op;

  if (Trace_SSA_Out) {
    fprintf(TFile, "-----------------------------------------------\n");
    fprintf(TFile, "        Eliminate_Psi_Resource_Interference    \n");
    fprintf(TFile, "-----------------------------------------------\n");
  }

  for (bb = REGION_First_BB; bb; bb = BB_next(bb)) {
    FOR_ALL_BB_PHI_OPs(bb, op) {
      PHI_CONGRUENCE_CLASS *cc1, *cc2;

      
    }
  }
}

/* ================================================================
 *   SSA_Make_Conventional
 * ================================================================
 */
void
SSA_Make_Conventional (
  RID *rid, 
  BOOL region 
)
{
  Trace_SSA_Out = Get_Trace(TP_SSA, SSA_MAKE_CONST);
  Trace_Igraph = Get_Trace(TP_SSA, SSA_IGRAPH);

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
  // Initialize PHI congruence classes, etc.
  //
  SSA_UNIVERSE_Initialize ();

  SSA_Make_PSI_Conventional();

  switch (CG_ssa_algorithm) {
  case 1: 
    insert_copies_blindly ();
    break;
  case 2:
  case 3:
    //
    // Build the interference graph
    //
    IGRAPH_Build();
    Eliminate_Phi_Resource_Interference();
    IGRAPH_Clean(); // do I need to clean anything ??
    break;
  default:
    Is_True(FALSE,("specify CG_ssa_algorithm"));
  }

  //
  // Map PHI resources to their congruence class names, clean up, etc.
  //
  SSA_UNIVERSE_Finalize();

  MEM_POOL_Pop(&MEM_local_pool);

  // Because, the above adds new TNs, we are not entirely able to
  // maintain the live-in out information incrementally.
  // Just recompute it here.
  GRA_LIVE_Recalc_Liveness(rid);

  if (Trace_SSA_Out) {
    Trace_IR(TP_SSA, "ELIMINATE PHI-RESOURCE INTRFERENCE", NULL);
  }

  return;
}

/* ================================================================
 *   SSA_Remove_Pseudo_OPs
 *
 *   Remove PHI-nodes renaming their resources into a representative
 *   name.
 *   IMPORTANT: - maintains the liveness info.
 *              - removes maps and pops ssa_pool, no more trace of
 *                the SSA in this region.
 * ================================================================
 */
void
SSA_Remove_Pseudo_OPs (
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
      fprintf(TFile, "============ remove_pseudo_ops BB%d ===========\n",
	      BB_id(bb));
    }

    // We also need to update LOOPINFO_trip_count_tn
    {
      ANNOTATION *annot = ANNOT_Get(BB_annotations(bb), ANNOT_LOOPINFO);
      if (annot) {
	LOOPINFO *info = ANNOT_loopinfo(annot);
	TN *trip_count_tn = LOOPINFO_trip_count_tn(info);
	if ((trip_count_tn != NULL) && TN_is_register(trip_count_tn)) {
	  if (TN_new_name(trip_count_tn) != NULL)
	    LOOPINFO_trip_count_tn(info) = TN_new_name(trip_count_tn);
	}
      }
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

      else if (OP_code(op) == TOP_psi) {
	if (Trace_phi_removal) {
	  fprintf(TFile, "  removing a psi \n\n");
	  //	  Print_OP_No_SrcLine(op);
	}

	BB_Remove_Op(bb, op);
      }

      else {

#if 0
	// First, replace PSEUDO ops by a real operation
	if (OP_code(op) == TOP_movc || OP_code(op) == TOP_movcf)
	  op = SSA_Expand_Movc(op);
#endif
	// Not a PHI-node, rename resources into representative name
	for (i = 0; i < OP_opnds(op); i++) {
	  tn = OP_opnd(op,i);

	  if (!TN_is_register(tn)) continue;

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

	  if (!TN_is_register(tn)) continue;

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
  tn_ssa_map = NULL;         /* so we knew we're out of the SSA */
  OP_MAP_Delete(phi_op_map);
  OP_MAP_Delete(psi_op_map);

  MEM_POOL_Pop (&ssa_pool);

#if 0
  //
  // SSA translation maintains liveness info (see Sreedhar's paper).
  // Normally, we shouldn't need to recompute it.
  // However, at the moment
  //   defreach_in
  //   defreach_out
  //   live_def
  //   live_use
  // are not being updated properly by the out of SSA
  // algorithm (I've been too lazy to look at it). So, for now
  // just recompute the liveness.
  // TODO: fix this.
  //
  GRA_LIVE_Recalc_Liveness(region ? rid : NULL);
#endif

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
