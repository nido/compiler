
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
inline void Set_TN_ssa_def(TN *t, OP *o) {
  if (tn_ssa_map != NULL) TN_MAP_Set(tn_ssa_map, t, o);
}

//
// Iterate over PHI-nodes in BB
//
#define FOR_ALL_BB_PHI_OPs(bb,op)    \
  for (op = BB_first_op(bb); op != NULL && OP_code(op) == TOP_phi; op = OP_next(op))

extern BOOL SSA_Check (RID *rid, BOOL region);
extern void SSA_Enter (RID *rid, BOOL region);
extern void SSA_Make_Consistent (RID *rid, BOOL region);
extern void SSA_Remove_Phi_Nodes (RID *rid, BOOL region);

extern void SSA_Collect_Info (RID *rid, BOOL region, INT phase);

// which BB corresponds to PHI-node operand 'opnd_idx' ?
extern BB*  Get_PHI_Predecessor (OP *phi, UINT8 opnd_idx);
extern void Set_PHI_Predecessor (OP *phi, UINT8 pos, BB *pred);

//
// Prepend the 'phi_op' to the 'bb' and do bookkeeping
//
extern void SSA_Prepend_Phi_To_BB (OP *phi_op, BB *bb);

// Tracing flags
#define SSA_BUILD        0x00000001  /* trace SSA build */
#define SSA_MAKE_CONST   0x00000002  /* trace make consistent */
#define SSA_IGRAPH       0x00000004  /* trace interference graph */
#define SSA_REMOVE_PHI   0x00000008  /* trace PHI-removal */
#define SSA_DOM_FRONT    0x00000010  /* trace dominance frontier */
#define SSA_COLLECT_INFO 0x00000020  /* collect statistics for SSA */

#endif /* cg_ssa_INCLUDED */
