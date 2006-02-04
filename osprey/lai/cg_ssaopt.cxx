/*
  ,Copyright (C) 2002, STMicroelectronics, All Rights Reserved.

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
*/

/* ====================================================================
 * ====================================================================
 *
 * Module: cg_ssaopt.cxx
 *
 * Description:
 *
 * General Flags are:
 *
 * ====================================================================
 * ====================================================================
 */


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
#include "cg_ssaopt.h"

#define SIGNED_BITS 9
#define MIN_OFFSET ((-1)<<(SIGNED_BITS-1))
#define MAX_OFFSET (( 1 <<(SIGNED_BITS-1))-1)
#define MAX_WIDTH  (( 1 << SIGNED_BITS  ) -1)

static MEM_POOL extended_offset_pool;

/* ====================================================================
 * Create the following data structure:
 * List(TN *base_tn, List(OP* op, INT64 offset))
 * ====================================================================
 */

typedef std::pair<OP*, INT64> op_offset;
typedef std::vector<op_offset> vec_op_offset;

typedef std::pair<TN *, vec_op_offset *> base_ops;
typedef std::vector<base_ops> vec_base_ops;

#define VEC_BASE_base(bv) (bv).first
#define VEC_BASE_ops(bv)  (bv).second

#define VEC_OP_op(ov)     (ov).first
#define VEC_OP_offset(ov) (ov).second

/* ====================================================================
 * Comparison function to sort List(OP* op, INT64 offset), in
 * increasing offset values.
 * ====================================================================
 */

inline bool offset_lt (op_offset off1, op_offset off2) 
{ 
  return (VEC_OP_offset(off1) < VEC_OP_offset(off2));
}

/* ====================================================================
 * Return the common domminator of two basic blocks.
 * ====================================================================
 */

static BB*
BB_get_common_dom(BB *dom, BB* bb) {

  if (dom == NULL)
    return bb;

  while (!BB_SET_MemberP (BB_dom_set((bb)), dom)) {
    dom = BB_dominator(dom);
  }

  return dom;
}

/* ====================================================================
 * Returns the list of operations in the form 'offset[base]', where
 * base == tn_base. Returns NULL if there is no such list.
 * ====================================================================
 */

static vec_op_offset*
Loopkup_Ops_List(vec_base_ops& Base_Ops_List, TN *tn_base) {
  for (int i = 0; i < Base_Ops_List.size(); i++) {
    base_ops& base_list = Base_Ops_List[i];
    if (VEC_BASE_base(base_list) == tn_base)
      return VEC_BASE_ops(base_list);
  }
  return NULL;
}

/* ====================================================================
 * For a sublist of operations in the form 'offset[base_tn]', replace
 * the base_tn by a new base and update the offsets, such that they
 * are now within [MIN_OFFSET, MAX_OFFSET].
 * ====================================================================
 */

static void
Update_Ops_With_New_Base(TN *base_tn, vec_op_offset& op_offset, int first, int last) {
  // Compute a new base_offset, such that positive offsets are
  // preferred, and alignment is kept if possible.
  int width = VEC_OP_offset(op_offset[last]) - VEC_OP_offset(op_offset[first]);
  INT64 base_offset = VEC_OP_offset(op_offset[first]) + (width <= MAX_OFFSET ? 0 : width - MAX_OFFSET);
  if ((width+3)&~3 <= MAX_WIDTH) base_offset = (base_offset+3) & ~3;
  else if ((width+1)&~1 <= (MAX_WIDTH & ~1)) base_offset = (base_offset+1) & ~1;

  // In case the original base_tn is rematerializable, new_base_tn can
  // also be rematerializable in simple cases.

  TN *new_base_tn = Dup_TN (base_tn);
  if (TN_is_rematerializable(base_tn)) {
    Is_True(TN_is_rematerializable(new_base_tn), ("Dup_TN did not propagate rematerializable attribute."));
    WN *wn_home = TN_home(base_tn);
    if (wn_home && WN_operator(wn_home) == OPR_LDA) {
      WN *new_wn_home = WN_CreateLda(WN_opcode(wn_home),  WN_lda_offset(wn_home) + base_offset,
				     WN_ty(wn_home), WN_st_idx(wn_home), WN_field_id(wn_home));
      Set_TN_home(new_base_tn, new_wn_home);
    }
    else {
      Reset_TN_is_rematerializable(new_base_tn);
      Set_TN_home(new_base_tn, NULL);
    }
  }

  BB *common_dom = NULL;

  for (int i = first; i <= last; i++) {
    OP *op = VEC_OP_op(op_offset[i]);
    INT base_idx = OP_find_opnd_use(op, OU_base);
    INT offset_idx = OP_find_opnd_use(op, OU_offset);
    
    TN *offset_tn = OP_opnd(op, offset_idx);
    TN *new_offset_tn = Gen_Adjusted_TN(offset_tn, -base_offset);

    Set_OP_opnd(op, base_idx, new_base_tn);
    Set_OP_opnd(op, offset_idx, new_offset_tn);

    common_dom = BB_get_common_dom(common_dom, OP_bb(op));
  }

  // Insert the initialization of the new base in the common
  // domminator of all the operations.
  OPS ops = OPS_EMPTY;
  if (base_offset < 0)
    Exp_SUB(MTYPE_I4, new_base_tn, base_tn, Gen_Literal_TN(-base_offset, 4), &ops);
  else
    Exp_ADD(MTYPE_I4, new_base_tn, base_tn, Gen_Literal_TN(base_offset, 4), &ops);

  OP *op_base = TN_ssa_def(base_tn);
  OP *op_new_base = OPS_last(&ops);

  if (common_dom == OP_bb(op_base)) {
    OP *point = op_base;
    while (OP_next(point) && OP_code(OP_next(point)) == TOP_phi) point = OP_next(point);
    BB_Insert_Ops(common_dom, point, &ops, FALSE);
  }
  else {
    OP *point = BB_first_op(common_dom);
    if (point == NULL || (OP_code(point) != TOP_phi))
      BB_Insert_Ops(common_dom, NULL, &ops, TRUE);
    else {
      while (OP_next(point) && OP_code(OP_next(point)) == TOP_phi) point = OP_next(point);
      BB_Insert_Ops(common_dom, point, &ops, FALSE);
    }
  }
}

/* ====================================================================
 * Collect all operations in the form offset[base], where offset is a
 * constant or a symbol. Then, for each base, sort the operations in
 * increasing offset order.
 * ====================================================================
 */

static void
Collect_Base_Offset(vec_base_ops& Base_Ops_List) {

  for (BB *bb = REGION_First_BB; bb != NULL; bb = BB_next(bb)) {

    OP *op;
    FOR_ALL_BB_OPs(bb, op) {

      INT base_idx = OP_find_opnd_use(op, OU_base);
      INT offset_idx = OP_find_opnd_use(op, OU_offset);

      if (base_idx < 0 || offset_idx < 0) continue;

      TN *tn_base = OP_opnd(op, base_idx);
      TN *tn_offset = OP_opnd(op, offset_idx);

      Is_True(TN_is_constant(tn_offset), ("Illegal offset in operation."));

      INT64 val;
      if (CGTARG_offset_is_extended(tn_offset, &val)) {
	vec_op_offset *oplist;
	if ((oplist = Loopkup_Ops_List(Base_Ops_List, tn_base)) == NULL) {
	  oplist = CXX_NEW (vec_op_offset, &extended_offset_pool);
	  Base_Ops_List.push_back(std::make_pair(tn_base, oplist));
	}
	oplist->push_back(op_offset(op, val));
      }
    }
  }

  for (int i = 0; i < Base_Ops_List.size(); i++) {
    base_ops& base_list = Base_Ops_List[i];
    vec_op_offset& op_offset = *VEC_BASE_ops(base_list);
    stable_sort(op_offset.begin(), op_offset.end(), offset_lt);
  }
}

/* ====================================================================
 * Look for contiguous store in memory of (almost) the same
 * value. Replace them with a call to memcpy. There must be no
 * operations in alias in between. Remove in the function only the
 * stores of the value used in the memset.
 * ====================================================================
 */

static void
Generate_Common_Base(vec_base_ops Base_Ops_List) {

  for (int i = 0; i < Base_Ops_List.size(); i++) {
    base_ops& base_list = Base_Ops_List[i];
    vec_op_offset& op_offset = *VEC_BASE_ops(base_list);

    // Compare (op_offset.size() * 2) with (op_offset.size()+2)
    if (op_offset.size() <= 2) continue;

    TN *base_tn = VEC_BASE_base(base_list);
    if (! TN_ssa_def(base_tn) ) {
#ifdef Is_True_On
      fPrint_TN(TFile, "Generate_Common_Base: %s is not optimized", base_tn);
      fprintf(TFile, " (gain would be at most %d bytes)\n", (op_offset.size()-2)*4);
#endif
      continue;
    }

    // Now, use a greedy algorithm
    int j, j_first;
    for (j = j_first = 0; j < op_offset.size(); j++) {
      if ((VEC_OP_offset(op_offset[j]) - VEC_OP_offset(op_offset[j_first])) > MAX_WIDTH) {
	if (j - j_first > 2)
	  Update_Ops_With_New_Base(base_tn, op_offset, j_first, j-1);
	j_first = j;
      }
    }
    if (j - j_first > 2)
      Update_Ops_With_New_Base(base_tn, op_offset, j_first, j-1);
  }
}

/* ====================================================================
 * Optimize operations in the form 'offset[base]'. 
 * ====================================================================
 */

static void
Optimize_Extended_Offset()
{
  MEM_POOL_Initialize (&extended_offset_pool, "op_offset map pool", TRUE);
  MEM_POOL_Push(&extended_offset_pool);
  vec_base_ops Base_Ops_List;

  Collect_Base_Offset(Base_Ops_List);

  Generate_Common_Base(Base_Ops_List);

  MEM_POOL_Pop(&extended_offset_pool);
  MEM_POOL_Delete(&extended_offset_pool);
}


/* ================================================================
 *
 *   Entry
 *
 * ================================================================
 */
void
SSA_Optimize()
{
  Optimize_Extended_Offset();
}
