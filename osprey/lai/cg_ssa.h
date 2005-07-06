
/* ========================================================================
 *
 *   Module that deals with the SSA form.
 *
 *   The entire program or a region of a program can be converted to the
 *   SSA form.
 *
 *   In the case of a region, the region -- a single entry, multiple
 *   exits entity, possesses the following properties:
 *
 *     1. every node in the region is dominated by the region entry;
 *     2. since we add post-exits to the nodes through which the region
 *        may be exited, such post-exits constitute a natural border
 *        of the region with the property that every post-exit is
 *        immediately dominated by a node from the region.
 *
 *   As a result of these properties the following holds:
 *
 *   Traversing the dominator tree starting at the entry node for a region
 *   will visit all nodes within this region's border without going out of
 *   this region's limits.
 *
 *   It basically means that :
 *
 *      (1) I can start traversing the dominator tree at the entry, 
 *      (2) I will necessary encounter one of the border (post-exit) nodes
 *          before I left the region -- therefore I can stop exploring the
 *          given tree branch as soon as I see a border node,
 *      (3) No node in the region will be missed by such traversal.
 *          Indeed, if a node X is not visited then its dominator tree
 *          predecessor is not visited or is not in the region. If it is
 *          not in the region, X may not be in the region because the
 *          only way to leave the region is through the border nodes
 *          that are immediately dominated by some node in the region.
 *          If X's predecessor is in the region and is not visited, then
 *          we can repeat the explanation. But eventually nodes in the
 *          region will be exhausted, which would mean that the entry
 *          node has not been visited -- nonsence, I started with it.
 *
 *   




 *   For building the SSA form, I use the algorithm by Cytron et al.
 *   ACM Transactions on Programming Languages and Systems, vol. 13,
 *   N. 4, October 1991, pp. 451 - 490.
 *
 *   The algorithm does:
 *
 *     1. compute the dominance frontier of the nodes in the program
 *        (region),
 *     2. place the phi-functions,
 *     3. rename variables.
 *
 *   Dominance frontier computation requires the bottom-up traversal of
 *   the dominator tree. The bottom-up list of nodes is constructed as
 *   a reversal of the DFS traversal of the dominator tree of the program
 *   (region).
 *   
 * ========================================================================
 */

#ifndef cg_ssa_INCLUDED
#define cg_ssa_INCLUDED

/* ========================================================================
 *   SSA flags:
 * ========================================================================
 */
extern INT32 CG_ssa_algorithm;

/* ========================================================================
 *   Mapping of TNs to their SSA definitions
 * ========================================================================
 */
extern TN_MAP tn_ssa_map;
#define TN_ssa_def(t)        ((OP *)TN_MAP_Get(tn_ssa_map, t))
/* The following function creates the SSA use-def link. It is
   automatically called when an operation is inserted in a
   block. However, this function MUST BE EXPLICITELY CALLED when the
   result of an operation is changed, in case of renaming for
   example. */
inline void Set_TN_ssa_def(TN *t, OP *o) {
  if (tn_ssa_map != NULL && !TN_is_dedicated(t) && !TN_is_save_reg(t)) {
#ifdef Is_True_On
    if (o && TN_ssa_def(t))
      Is_True(!o || !TN_ssa_def(t), ("Set_TN_ssa_def cannot be called on a TN with an SSA def."));
#endif
    TN_MAP_Set(tn_ssa_map, t, o);
  }
}

/* The two following functions update the SSA use-def link when an
   operation is inserted into or removed from a basicblock. */

inline void SSA_setup(OP *o) {
  if (tn_ssa_map)
    for (int i = 0; i < OP_results(o); i++)
      Set_TN_ssa_def(OP_result(o, i), o);
}

inline void SSA_unset(OP *o) {
  if (tn_ssa_map)
    for (int i = 0; i < OP_results(o); i++)
      Set_TN_ssa_def(OP_result(o, i), NULL);
}

//
// Iterate over PHI-nodes in BB
//
#define FOR_ALL_BB_PHI_OPs(bb,op)    \
  for (op = BB_first_op(bb); op != NULL && OP_code(op) == TOP_phi; op = OP_next(op))

extern BOOL SSA_Check (RID *rid, BOOL region);
extern BOOL SSA_Verify (RID *rid, BOOL region);
extern void SSA_Enter (RID *rid, BOOL region);
extern void SSA_Make_Conventional (RID *rid, BOOL region);
extern void SSA_Remove_Pseudo_OPs (RID *rid, BOOL region);

extern void SSA_Collect_Info (RID *rid, BOOL region, INT phase);

// which BB corresponds to PHI-node operand 'opnd_idx' ?
extern BB*  Get_PHI_Predecessor (const OP *phi, UINT8 opnd_idx);
extern void Set_PHI_Predecessor (const OP *phi, UINT8 pos, BB *pred);

// which opnd_idx corresponds to PHI-node predecessor BB  ?
extern UINT8 Get_PHI_Predecessor_Idx (const OP *phi, BB *);

// which guard TN is associated to PSI-node operand 'opnd_idx' ?
#define PSI_opnds(psi)    (OP_opnds(psi)>>1)
#define PSI_opnd(psi, i)  (OP_opnd(psi, ((i)<<1)+1))
extern void Set_PSI_opnd(OP *, UINT8, TN *);
extern TN *PSI_guard(const OP *, UINT8);
extern void Set_PSI_guard(OP *, UINT8, TN *);

// Sort PHI operands according to the dominance relation of the
// argument's definition
extern void Sort_PHI_opnds (OP *phi);

// Create a PSI operation from a PHI
extern OP * Convert_PHI_to_PSI (OP *phi);

// Simplify a PSI operation
extern OP * PSI_inline (OP *psi);
extern OP * PSI_reduce (OP *psi);

// Conditional move operations
extern OP *OP_Make_movc (TN *guard, TN *dst, TN *src);
extern OP *OP_Make_movcf (TN *guard, TN *dst, TN *src);

//
// Initialise the 'phi_op' mapping
//
extern void Initialize_PHI_map(OP   *phi);

// Tracing flags
#define SSA_BUILD        0x00000001  /* trace SSA build */
#define SSA_MAKE_CONST   0x00000002  /* trace make conventional */
#define SSA_IGRAPH       0x00000004  /* trace interference graph */
#define SSA_REMOVE_PHI   0x00000008  /* trace PHI-removal */
#define SSA_DOM_FRONT    0x00000010  /* trace dominance frontier */
#define SSA_COLLECT_INFO 0x00000020  /* collect statistics for SSA */

/* FdF 20050309: Externalized for use in SSA_Optimize(). */

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
extern DOM_TREE *dom_map;

#define BB_dominator(bb)         (dom_map[BB_id(bb)]._M_parent)
#define BB_children(bb)          (dom_map[BB_id(bb)]._M_kids)

#endif /* cg_ssa_INCLUDED */
