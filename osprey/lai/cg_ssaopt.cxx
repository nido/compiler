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

// Do we have to ensure compatibility with old gcc version?
#include <set>

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

#include "cg_spill.h"

static BOOL
Retrieve_Base_Offset(OP* op, TN*& tn_base, TN*& tn_offset);

static BOOL Trace_SSA_CBPO = 0;

#if 0
#define SIGNED_BITS 9
#define MIN_OFFSET ((-1)<<(SIGNED_BITS-1))
#define MAX_OFFSET (( 1 <<(SIGNED_BITS-1))-1)
#define MAX_WIDTH  (( 1 << SIGNED_BITS  ) -1)
#else
/* Replace by target dependent info.*/
static INT MIN_OFFSET = 0;           /* In case of signed 9 bits -> -256    */
static INT MAX_OFFSET = 0;           /* In case of signed 9 bits -> +255    */
static INT MAX_WIDTH = 0;            /* MAX_WIDTH = MAX_OFFSET - MIN_OFFSET */
static INT min_offset_alignment = 1; /* 1 -> char (byte), 2 -> short (2 bytes), 4 -> word (4 bytes), ... */
#endif

static MEM_POOL extended_offset_pool;
static MEM_POOL base_offset_pool;
static TN_MAP base_offset_map;
static TN_MAP constant_map;

static BOOL Optimized_Extended_Immediate;
static float min_frequency_ratio = 1.0;

/* ====================================================================
 * Data structure for base/offset propagation
 *
 * ====================================================================
 */

class Addr_Exp_Info {
public:
  Addr_Exp_Info(TN *_base, TN *_offset) {
    base_reg = _base;
    offset_tn = _offset;
  }
  Addr_Exp_Info() {
    base_reg = NULL;
    offset_tn = NULL;
  }

  TN    *base_reg;
  TN    *offset_tn;
};

#define Addr_base_reg(addr)  ((addr).base_reg)
#define Addr_offset_tn(addr) ((addr).offset_tn)

class Base_Offset_Info {
public:
  Base_Offset_Info(OP *_op, TN *_base, TN *_offset, INT64 _base_dist, INT64 _addr_dist) {
    op        = _op;
    base_tn   = _base;
    offset_tn = _offset;
    base_dist = _base_dist;
    addr_dist = _addr_dist;
  }

  OP	*op;
  TN	*base_tn;
  TN    *offset_tn;
  INT64  base_dist;
  INT64  addr_dist;
};
  
#define BO_op(b)	((b).op)
#define BO_base_tn(b)   ((b).base_tn)
#define BO_offset_tn(b) ((b).offset_tn)
#define BO_base_dist(b) ((b).base_dist)
#define BO_addr_dist(b) ((b).addr_dist)

typedef std::vector<Base_Offset_Info> vec_bo_info;

typedef std::pair<TN *, vec_bo_info *> base_ops;
typedef std::vector<base_ops> vec_base_ops;

#define VEC_BASE_base(bv) (bv).first
#define VEC_BASE_ops(bv)  (bv).second

static void
Print_base_ops(FILE *file, base_ops &base_list) {
  int i;
  vec_bo_info& op_offset = *VEC_BASE_ops(base_list);

  fPrint_TN(file, "***** Print_base_ops for %s\n", VEC_BASE_base(base_list));  
  for (i = 0; i < op_offset.size(); i++) {
    fprintf(file, "\t(op %d", OP_map_idx(BO_op(op_offset[i])));
    fPrint_TN(file, ", base %s", BO_base_tn(op_offset[i]));
    fPrint_TN(file, ", offset %s", BO_offset_tn(op_offset[i]));
    fprintf(file, ", base distance 0x%llx", BO_base_dist(op_offset[i]));
    fprintf(file, ", addr distance 0x%llx", BO_addr_dist(op_offset[i]));
    fprintf(file, ")\n");
  }
}

/* ====================================================================
 * Comparison function to sort List(OP* op, INT64 offset), in
 * increasing offset values.
 * ====================================================================
 */

inline bool offset_lt (Base_Offset_Info off1, Base_Offset_Info off2) 
{ 
  return (BO_addr_dist(off1) < BO_addr_dist(off2));
}

/* ====================================================================
 * Return the common domminator of two basic blocks.
 * ====================================================================
 */

static BB*
BB_get_common_dom(BB *dom, BB* bb) {

  if (dom == NULL)
    return bb;

  while (dom && !BB_SET_MemberP (BB_dom_set((bb)), dom)) {
    dom = BB_dominator(dom);
  }

  return dom;
}

/* ====================================================================
 * Returns the list of operations in the form 'offset[base]', where
 * base == tn_base. Returns NULL if there is no such list.
 * ====================================================================
 */

static vec_bo_info*
Lookup_Ops_List(vec_base_ops& Base_Ops_List, TN *tn_base) {
  for (int i = 0; i < Base_Ops_List.size(); i++) {
    base_ops& base_list = Base_Ops_List[i];
    if (VEC_BASE_base(base_list) == tn_base)
      return VEC_BASE_ops(base_list);
  }
  return NULL;
}

/* ====================================================================
 * Returns true if the value does not fit in range
 * 
 * ====================================================================
 */

/* ====================================================================
 * For a sublist of operations in the form 'offset[base_tn]', replace
 * the base_tn by a new base and update the offsets, such that they
 * are now within [MIN_OFFSET, MAX_OFFSET].
 * ====================================================================
 */

static TN *
Dup_Rematerializable_TN(TN *tn, INT64 adjust) {

  TN *new_tn;
  if (TN_is_dedicated(tn))
    new_tn = Dup_TN_Even_If_Dedicated(tn);
  else
    new_tn = Dup_TN(tn);

  if (TN_is_rematerializable(tn)) {
    Is_True(TN_is_rematerializable(new_tn), ("Dup_TN did not propagate rematerializable attribute."));
    WN *wn_home = TN_home(tn);
    if (wn_home && WN_operator(wn_home) == OPR_LDA) {
      WN *new_wn_home = WN_CreateLda(WN_opcode(wn_home), WN_lda_offset(wn_home) + adjust,
				     WN_ty(wn_home), WN_st_idx(wn_home), WN_field_id(wn_home));
      Set_TN_home(new_tn, new_wn_home);
    }
    else {
      Reset_TN_is_rematerializable(new_tn);
      Set_TN_home(new_tn, NULL);
    }
  }

  // FdF 20061212: In any case, reset the GRA_homeable property,
  // otherwise new_tn would share the same spill location as the
  // original one.
  if (TN_is_gra_homeable(tn)) {
    Reset_TN_is_gra_homeable(new_tn);
    Set_TN_home(new_tn, NULL);
  }

  return new_tn;
}


static BOOL
Use_TN(const OPS& ops, TN *tn)
{
    OP* op;
    BOOL result = FALSE;
    for(op = OPS_first(&ops); op && !result; op = OP_next(op))
        {
            for(INT i = 0; i < OP_opnds(op); ++i)
                {
                    result |= tn == OP_opnd(op, i);
                }
        }
    return result;
}

static BOOL
Set_Remat(TN* result_tn, TN* oldBase)
{
    BOOL result = !TN_is_dedicated(result_tn);
    if(result && !TN_is_rematerializable(result_tn))
        {
            if(TN_is_symbol(oldBase))
                {
                    // Pointer are seen as unsigned int
                    CGSPILL_Attach_Lda_Remat(result_tn, MTYPE_U4,
                                             TN_offset(oldBase),
                                             TN_var(oldBase));
                }
            else if(TN_has_value(oldBase))
                {
                    CGSPILL_Attach_Intconst_Remat(result_tn, MTYPE_I4,
                                                  TN_value(oldBase));
                }
            else
                {
                    result = FALSE;
                }
        }
    return result;
}

static BOOL
Update_Expensive_Load_Im(OP *&op, INT index, INT64 offset_val, TN *new_base_tn,
                         TN *new_offset_tn)
{

    OPS ops = OPS_EMPTY;
    TN *result_tn = OP_result(op, 0);
    OP *point = OP_next(op);
    BB *bb = OP_bb(op);
    TN *oldBase = OP_opnd(op, index);
    BOOL result;
    BOOL tnIsRemat = TN_is_rematerializable(result_tn) != 0;
    if((result = Set_Remat(result_tn, oldBase)))
        {
            BB_Remove_Op(OP_bb(op), op);
            if(offset_val < 0)
                {
                    Exp_SUB(MTYPE_I4, result_tn, new_base_tn,
                            Gen_Literal_TN(-offset_val, 4), &ops);
                }
            else if(offset_val > 0)
                {
                    Exp_ADD(MTYPE_I4, result_tn, new_base_tn,
                            new_offset_tn, &ops);
                }
            else
                {
                    Expand_Copy(result_tn, True_TN, new_base_tn, &ops);
                }
            if(!CGTARG_sequence_is_cheaper_than_load_imm(&ops, op))
                {
                    result = FALSE;
                    OPS_Remove_All(&ops);
                    OPS_Append_Op(&ops, op);
                    if(!tnIsRemat)
                        {
                            Reset_TN_is_rematerializable(result_tn);
                        }
                }
            if(point)
                {
                    BB_Insert_Ops(bb, point, &ops, TRUE);
                }
            else
                {
                    // else op was the last instruction of the basic block
                    BB_Append_Ops(bb, &ops);
                }
            op = ops.last;
        }
    return result;
}

static TN *
Update_Op_With_New_Base(OP *&op, TN *new_base_tn, INT64 base_offset, INT64 addr_dist, INT& nbDone) {
    INT base_idx = OP_find_opnd_use(op, OU_base);
    INT offset_idx = OP_find_opnd_use(op, OU_offset);
    
    TN *base_tn = NULL;
    TN *offset_tn = NULL;
    BOOL result = Retrieve_Base_Offset(op, base_tn, offset_tn);
    DevAssert(result, ("We should not be here if Retrieve_Base_Offset returns "
                       "false!!!"));
    
    TN *new_offset_tn;
    
    INT64 offset_val = addr_dist - base_offset;
    new_offset_tn = Gen_Literal_TN(offset_val, 4);
    if (TN_is_register(offset_tn) && offset_idx >= 0) {
        TOP new_top;
        new_top = TOP_opnd_immediate_variant(OP_code(op), offset_idx, offset_val);
        FmtAssert(new_top != TOP_UNDEFINED,("Unable to get TOP immediate variant"));
        OP_Change_Opcode(op, new_top);
    }
    
#ifdef Is_True_On
    if(offset_idx >= 0) {
        FmtAssert(TOP_opnd_value_in_range (OP_code(op), offset_idx, offset_val),("new offset does not fit in immediate"));
    }
#endif
    
    if (Trace_SSA_CBPO) {
        fprintf(TFile, ">>> Replacing OLD op by NEW op:\n    OLD ");
        Print_OP_No_SrcLine (op);
    }
    
    if(base_idx < 0 || offset_idx < 0) {
        INT index = CGTARG_expensive_load_imm_immediate_index(op);
        if(index >= 0) {
            if(!Update_Expensive_Load_Im(op, index, offset_val, new_base_tn,
                                         new_offset_tn)) {
                if(Trace_SSA_CBPO) {
                    fprintf(TFile, "Do not update expensive load\n");
                }
            } else {
                ++nbDone;
            }
        }
    } else {
        ++nbDone;
        Set_OP_opnd(op, base_idx, new_base_tn);
        Set_OP_opnd(op, offset_idx, new_offset_tn);
    }
    if (Trace_SSA_CBPO) {
        fprintf(TFile, "    NEW ");
        Print_OP_No_SrcLine (op);
    }

    return new_offset_tn;
}


static TN *
Update_Op_With_New_Base(OP *&op, TN *new_base_tn, INT64 base_offset, INT64 addr_dist, BB* common_dom, float sum, INT& nbDone, const std::set<int>& alreadyDone) {
    TN *result = NULL;
    BB *bb = OP_bb(op);
    BB *common_dom_new = BB_get_common_dom(common_dom, bb);
    if(Trace_SSA_CBPO) {
        fprintf(TFile, "BB%d frequency: %f\nDominator (BB%d) frequency: %f\n",
                BB_id(bb), BB_freq(bb), BB_id(common_dom_new),
                BB_freq(common_dom_new));
    }

    if(alreadyDone.find(BB_id(bb)) == alreadyDone.end()) {
        sum += BB_freq(bb);
    }
    // Here we check that the frequency of the basic block, that will receive
    // the initialization of the new base, does not have a too big difference
    // with the source one. Moreover, if frequencies are too different but we
    // do not change the dominator and we have already had to emit the new base
    // initialization or we do not change the destination basic block, then
    // performs the optimization.
    // FdF 20061027: In case of operations in exception handlers,
    // there will be no common dominator. Some operations will not be
    // optimized in this case. (common_dom_new == NULL)
    if(common_dom_new &&
       (!(CG_cbpo_block_method & CBPO_BLOCK_LOCAL) ||
        ((sum / BB_freq(common_dom_new)) > min_frequency_ratio) ||
        (common_dom_new == common_dom && nbDone) || common_dom_new == bb)) {
        result = Update_Op_With_New_Base(op, new_base_tn, base_offset,
                                         addr_dist, nbDone);
    } else {
        if(Trace_SSA_CBPO) {
            fprintf(TFile, "Do not update load: Frequencies are too "
                    "different: Sum %f\tDominator %f\n", sum,
                    BB_freq(common_dom_new));
        }
    }
    return result;
}

static INT
Check_And_Update_Ops_With_New_Base(BB *&common_dom, TN* new_base_tn,
                                   INT64 base_offset, vec_bo_info& op_offset,
                                   int first, int last) {
  common_dom = NULL;
  INT nbDone = 0;
  int redoSize;
  int nbPhase = 1;

  std::list<int> shouldRedo;
  std::set<int> alreadyDone;
  float sum = 0.0;
  BB *tmp_dom = common_dom;
  for (int i = first; i <= last; i++) {
      shouldRedo.push_back(i);
      BB* bb = OP_bb(BO_op(op_offset[i]));
      if(alreadyDone.find(BB_id(bb)) == alreadyDone.end()) {
          alreadyDone.insert(BB_id(bb));
          sum += BB_freq(bb);
          tmp_dom = BB_get_common_dom(tmp_dom, bb);
      }
  }

  // Save block method value
  INT CG_cbpo_block_method_save = CG_cbpo_block_method;

  if(CG_cbpo_block_method & CBPO_BLOCK_GLOBAL) {
      if((sum / BB_freq(tmp_dom)) < min_frequency_ratio) {
          if(Trace_SSA_CBPO) {
              fprintf(TFile, "Do not globaly optimize: Sum %f\tDominator "
                      "(BB%d) %f\n",
                      sum, BB_id(tmp_dom), BB_freq(tmp_dom));
          }
          if(CG_cbpo_block_method == CBPO_BLOCK_GLOBAL) {
              return 0;
          }
          // Else: Global has failed, but user set method global then local, so
          // continue
      } else {
          // Global will work, so do not apply local block method
          CG_cbpo_block_method = CBPO_BLOCK_GLOBAL;
      }
  }

  alreadyDone.clear();
  sum = 0.0;

  // We perform the update of the operation while we have rejected
  // transformation and the number of rejected transformation is lower than the
  // previous iteration. The purpose is to miss not optimization opportunity,
  // since dominator may change and the evaluation of the frequency condition
  // can become true (because of nbDone for instance, which may force an
  // optimization).
  do {
    redoSize = shouldRedo.size();
    std::list<int> tmp;
    std::list<int>::const_iterator it;
    for(it = shouldRedo.begin(); it != shouldRedo.end(); ++it) {
      OP *op = BO_op(op_offset[*it]);
      INT nbDoneOld = nbDone;
      Update_Op_With_New_Base(op, new_base_tn, base_offset,
                              BO_addr_dist(op_offset[*it]), common_dom, sum,
                              nbDone, alreadyDone);
      if(nbDone != nbDoneOld) {
          if(Trace_SSA_CBPO) {
              fprintf(TFile, "Update dominator (%d)\n", nbPhase);
          }
          // Increase the sum for this group of operations unless it has been
          // done
          if(alreadyDone.find(BB_id(OP_bb(op))) == alreadyDone.end()) {
              alreadyDone.insert(BB_id(OP_bb(op)));
              sum += BB_freq(OP_bb(op));
          }
          common_dom = BB_get_common_dom(common_dom, OP_bb(op));
      } else {
          tmp.push_back(*it);
      }
    }
    ++nbPhase;
    shouldRedo = tmp;
  } while(!shouldRedo.empty() && shouldRedo.size() < redoSize);

  // Restore value defined by user
  CG_cbpo_block_method = CG_cbpo_block_method_save;
  return nbDone;
}

static void
Update_Ops_With_New_Base(TN *base_tn, INT64 base_adjust, TN *remat_base, INT64 remat_adjust, vec_bo_info& op_offset, int first, int last) {
  // Compute a new base_offset, such that positive offsets are
  // preferred, and alignment is kept if possible.
  int width = BO_addr_dist(op_offset[last]) - BO_addr_dist(op_offset[first]);
  INT64 base_offset;
  INT min_offset;
  if (width <= MAX_OFFSET)
    min_offset = 0;
  else
    min_offset = MAX_OFFSET - width;
  base_offset = BO_addr_dist(op_offset[first]) - min_offset;

  /* Search biggest admissible alignment. */
  /* Start with min target alignment constraint */
  // We will increment base_offset so as to maximize its alignment,
  // such that the offset for the first and last operation is still
  // representable.
  int biggest_offset_alignment = min_offset_alignment;
  int dist_to_next_align = (-base_offset)&(biggest_offset_alignment-1);
  FmtAssert(MIN_OFFSET<0,("This algorithm works only with MIN_OFFSET < 0"));
  while ((min_offset - dist_to_next_align) >= MIN_OFFSET && biggest_offset_alignment < MAX_OFFSET) {
    min_offset -= dist_to_next_align;
    base_offset += dist_to_next_align;
    biggest_offset_alignment *= 2;
    dist_to_next_align = (-base_offset)&(biggest_offset_alignment-1);
  }

  // In case the original base_tn is rematerializable, new_base_tn can
  // also be rematerializable in simple cases.

  TN *new_base_tn;
  if (base_tn && TN_is_register(base_tn))
    new_base_tn = Dup_Rematerializable_TN(base_tn, base_offset-base_adjust);
  else if (remat_base != NULL)
    new_base_tn = Dup_Rematerializable_TN(remat_base, base_offset-remat_adjust);
  else
    new_base_tn = Build_TN_Of_Mtype(MTYPE_I4);

  BB *common_dom;
  INT nbDone = Check_And_Update_Ops_With_New_Base(common_dom, new_base_tn,
                                                  base_offset, op_offset,
                                                  first, last);

  // Insert the initialization of the new base in the common
  // domminator of all the operations.
  OPS ops = OPS_EMPTY;
  INT64 init_adjust = base_offset - base_adjust;

  if(Trace_SSA_CBPO) {
      fprintf(TFile,"*** nbDone %d\n", nbDone);
  }

  if(nbDone) {
      if (base_tn == NULL)
          Expand_Immediate(new_base_tn, Gen_Literal_TN(init_adjust, 4), MTYPE_I4, &ops);
      else if (!TN_is_register(base_tn))
          Expand_Immediate(new_base_tn, Gen_Adjusted_TN(base_tn, init_adjust), MTYPE_I4, &ops);
      else if (init_adjust < 0)
          Exp_SUB(MTYPE_I4, new_base_tn, base_tn, Gen_Literal_TN(-init_adjust, 4), &ops);
      else if (init_adjust > 0)
          Exp_ADD(MTYPE_I4, new_base_tn, base_tn, Gen_Literal_TN(init_adjust, 4), &ops);
      else {
          Expand_Copy(new_base_tn, True_TN, base_tn, &ops);
      }
      
      if (Trace_SSA_CBPO) {
          fprintf(TFile, ">>> Inserting new OP in BB %d:\n", BB_id(common_dom));
          Print_OPS_No_SrcLines(&ops);
      }
      
      // If operations to be inserted do not use base_tn, we must not insert them
      // after the definition of that tn. E.g.
      // base_tn :- load_im cst
      // Is replace by
      // base_tn :- move new_base_tn
      // So operations to be inserted define new_base_tn, so we have to insert
      // them before first usage of new_base_tn (i.e. operation which define
      // base_tn) or in dominator bb.
      // This is not true with other load operations, since new_base_tn is
      // defined from base_tn, that is why we check base_tn usage.
      OP *op_base = (base_tn && TN_is_register(base_tn) &&
                     Use_TN(ops, base_tn))? TN_ssa_def(base_tn): NULL;
      OP *point = (op_base && (common_dom == OP_bb(op_base)))? OP_next(op_base):
          BB_first_op(common_dom);

      // FdF 20061124: Be careful not to insert code before spadjust
      while (point && (OP_code(point) == TOP_phi ||
                       point == BB_entry_sp_adj_op(OP_bb(point)))) {
          point = OP_next(point);
      }

      OP* sp_adjust;
      if ((point != NULL) &&
          !(op_base && common_dom == OP_bb(op_base)) && // In this case, insertion point is after op_base. So necessary after sp_adjust.
          ( ( sp_adjust = BB_entry_sp_adj_op(OP_bb(point)) ) != NULL))  {
          // [JV] In case of double sp_adjust, the last one is marked
          // sp_adjust.
          point = OP_next(sp_adjust);
      }
      BOOL before = (point != NULL);
      BB_Insert_Ops(common_dom, point, &ops, before);
      
      Optimized_Extended_Immediate = TRUE;
  }
}

/* ====================================================================
 * Propagate constants and sequence of form base +/- offset
 *
 * ====================================================================
 */

static TN *
Get_Constant_Value(TN *tn) {
  if (!TN_is_constant(tn) && TN_MAP_Get(constant_map, tn))
    return (TN *)TN_MAP_Get(constant_map, tn);
  if (TN_is_zero(tn))
    return Gen_Literal_TN(0, 4);
  return tn;
}

// This function will return one of the following :
// (NULL, offset=Value), (Symbol, offset=Value), (Register,
// offset=Value), (tn, offset=0)

static TN *
Get_Addr_Exp(TN *tn, INT64 *offset_value) {

  TN *base = tn;
  *offset_value = 0;

  TN *base_def = Get_Constant_Value(tn);
  if (TN_is_constant(base_def)) {
    if (TN_is_symbol(base_def)) {
      base = Gen_Symbol_TN(TN_var(base_def), 0, TN_relocs(base_def));
      *offset_value = TN_offset(base_def);
    }
    else { // TN_has_value(base_def)
      base = NULL;
      *offset_value = TN_value(base_def);
    }
  }
  else if (TN_MAP_Get(base_offset_map,tn)) {
    Addr_Exp_Info *tn_info = (Addr_Exp_Info*) TN_MAP_Get(base_offset_map,tn);
    // Ignore the case Reg+Sym
    if (TN_has_value(Addr_offset_tn(*tn_info))) {
      base = Addr_base_reg(*tn_info);
      *offset_value = TN_value(Addr_offset_tn(*tn_info));
    }
  }

  return base;
}

// We support the following expressions, after copy propagation
// (reg+val), (reg-val), (val+reg), (sym+val), (sym-val), (val+sym),
// (val+val), (val-val), (reg+sym), (sym+reg), (reg-sym)

static BOOL
Compute_Addr_Exp(Addr_Exp_Info &addr_exp, TN *base, TN *offset, BOOL commutative, BOOL negate_offset) {

  // Propagate the constant on base and offset if available.
  INT64 base_adjust, offset_adjust;
  base = Get_Addr_Exp(base, &base_adjust);
  offset = Get_Addr_Exp(offset, &offset_adjust);

  // In case of a SUB, only an immediate value is accepted
  if (negate_offset)
    if (offset != NULL)
      return FALSE;
    else
      offset_adjust = -offset_adjust;

  if (commutative &&
      (((base == NULL) && (offset != NULL)) ||
       ((base != NULL) && (offset != NULL) && TN_is_symbol(base) && TN_is_register(offset)))) {
    TN *tmp = base;
    base = offset;
    offset = tmp;
  }

  if (base == NULL && offset == NULL) {
    addr_exp.base_reg = NULL;
    addr_exp.offset_tn = Gen_Literal_TN(base_adjust+offset_adjust, 4);
    return TRUE;
  }

  else if ((base != NULL) && (offset == NULL)) {
    if (TN_is_symbol(base)) {
      addr_exp.base_reg = NULL;
      addr_exp.offset_tn = Gen_Adjusted_TN(base, base_adjust+offset_adjust);
      return TRUE;
    }
    else if (TN_is_register(base)) {
      addr_exp.base_reg = base;
      addr_exp.offset_tn = Gen_Literal_TN(base_adjust+offset_adjust, 4);
      return TRUE;
    }
  }

  else if ((base != NULL) && (offset != NULL)) {
    if (TN_is_register(base) && (TN_is_symbol(offset))) {
      addr_exp.base_reg = base;
      addr_exp.offset_tn = Gen_Adjusted_TN(offset, base_adjust+offset_adjust);
      return TRUE;
    }
  }

  return FALSE;
}

static INT64
Get_TN_Distance(TN *common_tn, TN *base_tn) {

  INT64 base_adjust;
  base_tn = Get_Addr_Exp(base_tn, &base_adjust);

  if ((common_tn == NULL) && (base_tn == NULL))
    return 0;

  else if (common_tn == base_tn)
    return base_adjust;

  /* FdF 20061117: For symbols, pointers may be different, but symbols
     may actually be the same. (codex bug #20430) */

  else if ((common_tn != NULL) && (base_tn != NULL) &&
	   TN_is_symbol(common_tn) && TN_is_symbol(base_tn) &&
	   (TN_var(common_tn) == TN_var(base_tn)) &&
	   (TN_offset(common_tn) == TN_offset(base_tn)) &&
	   (TN_relocs(common_tn) == TN_relocs(base_tn)))
    return base_adjust;

  FmtAssert (FALSE, ("Get_TN_Distance: No distance between TNs"));
}

// TBD: Perform propagate and collect at the same time. Also,
// propagate should be performed in a depth first search of the
// dominator tree.

static void
Propagate_Base_Offset( void ) {

  for (BB *bb = REGION_First_BB; bb != NULL; bb = BB_next(bb)) {
    OP *op;
    FOR_ALL_BB_OPs(bb, op) {

      /* Constant propagation */
      INT copy_operand_idx;
      if( (copy_operand_idx = OP_Copy_Operand(op)) != -1 ) {
	TN *constant = OP_opnd(op,copy_operand_idx);
	TN *result = OP_result(op,0);

	if (TN_is_dedicated(result)) continue;

	// FdF 20060918: Consider that a symbol requires an extended immediate
	// FdF 20061205: At this time, propagate all values, the
	// extended offset values will be checked later.
	if (TN_has_value(constant) || TN_is_symbol(constant)) {

	  if (Trace_SSA_CBPO) {
	    fprintf(TFile, "%s: Propagate constant from OP:\n", __FUNCTION__);
	    Print_OP_No_SrcLine(op);
	  }
	    
	  TN_MAP_Set(constant_map, result, constant);
	}
      }

      if(OP_iadd(op) || OP_isub(op)) {
	/* Suppose that base is in opnd1 and offset in opnd2 */
	TN *base = OP_opnd(op,OP_find_opnd_use(op, OU_opnd1));
	TN *offset = OP_opnd(op,OP_find_opnd_use(op, OU_opnd2));
	TN *result = OP_result(op,0);

	if(TN_is_dedicated(result)) { continue; }

	Addr_Exp_Info addr_exp;
	if (!Compute_Addr_Exp(addr_exp, base, offset, TRUE, OP_isub(op)))
	  continue;

	if (addr_exp.base_reg == NULL) {

	  if (Trace_SSA_CBPO) {
	    fprintf(TFile, "%s: Propagate constant from OP:\n", __FUNCTION__);
	    Print_OP_No_SrcLine(op);
	  }

	  TN_MAP_Set(constant_map, result, addr_exp.offset_tn);
	}
	else {

	  if (Trace_SSA_CBPO) {
	    fprintf(TFile, "%s: Propagate base/offset from OP:\n", __FUNCTION__);
	    Print_OP_No_SrcLine(op);
	  }

	  Addr_Exp_Info *tn_info = CXX_NEW (Addr_Exp_Info, &base_offset_pool);
	  memset(tn_info, 0, sizeof(Addr_Exp_Info));
	  Addr_base_reg(*tn_info) = addr_exp.base_reg;
	  Addr_offset_tn(*tn_info) = addr_exp.offset_tn;
	  TN_MAP_Set(base_offset_map, result, tn_info);
	}
      }
    }
  }
}


static BOOL
Retrieve_Base_Offset(OP* op, TN*& tn_base, TN*& tn_offset)
{
    INT base_idx = OP_find_opnd_use(op, OU_base);
    INT offset_idx = OP_find_opnd_use(op, OU_offset);
    BOOL result = FALSE;

    if (base_idx < 0 || offset_idx < 0)
        {
            INT index;
            if((index = CGTARG_expensive_load_imm_immediate_index(op)) >= 0 &&
               CG_cbpo_optimize_load_imm)
                {
                    TN* tn = OP_opnd(op, index);
                    result = TRUE;
                    if(TN_is_symbol(tn))
                        {
                            tn_base = Gen_Symbol_TN(TN_var(tn), 0,
                                                    TN_relocs(tn));
                            tn_offset = Gen_Literal_TN(TN_offset(tn), 4);
                        }
                    else
                        {
                            tn_base = tn;
                            tn_offset = Gen_Literal_TN(0, 4);
                            // Currently, we do not optimize known constant
                            result = FALSE;
                        }
                }
        }
    else
        {
            tn_base = OP_opnd(op, base_idx);
            tn_offset = OP_opnd(op, offset_idx);
            result = TRUE;
        }
    return result;
}

/* ====================================================================
 * Collect all operations in the form offset[base], where offset is a
 * constant or a symbol.
 * ====================================================================
 */

static void
Collect_Base_Offset(vec_base_ops& Base_Ops_List) {

  for (BB *bb = REGION_First_BB; bb != NULL; bb = BB_next(bb)) {

    OP *op;
    FOR_ALL_BB_OPs(bb, op) {

        TN *tn_base = NULL;
        TN *tn_offset = NULL;
        if(!Retrieve_Base_Offset(op, tn_base, tn_offset)) continue;

      Addr_Exp_Info addr_exp;
      if (!Compute_Addr_Exp(addr_exp, tn_base, tn_offset, FALSE, FALSE))
	continue;

      INT64 base_dist, addr_dist;
      TN *common_tn = NULL;
      INT64 offset_init;

      if ((addr_exp.base_reg == NULL) && (TN_is_symbol(addr_exp.offset_tn))) {
	common_tn = Gen_Symbol_TN(TN_var(addr_exp.offset_tn), 0, TN_relocs(addr_exp.offset_tn));
	addr_dist = TN_offset(addr_exp.offset_tn);
      }

      else if ((addr_exp.base_reg == NULL) && TN_has_value(addr_exp.offset_tn)) {
	common_tn = NULL;
	addr_dist = TN_value(addr_exp.offset_tn);
      }

      else if ((addr_exp.base_reg != NULL) && TN_has_value(addr_exp.offset_tn)) {
	common_tn = addr_exp.base_reg;
	addr_dist = TN_value(addr_exp.offset_tn);
      }

      else
	continue;

      base_dist = Get_TN_Distance(common_tn, tn_base);

      /* Then, add the operation in the list associated with
	 common_tn. Each element contains (op, tn_base, val) */
      vec_bo_info *oplist;
      if ((oplist = Lookup_Ops_List(Base_Ops_List, common_tn)) == NULL) {
	oplist = CXX_NEW (vec_bo_info, &extended_offset_pool);
	Base_Ops_List.push_back(std::make_pair(common_tn, oplist));
      }
      oplist->push_back(Base_Offset_Info(op, tn_base, tn_offset, base_dist, addr_dist));
    }
  }
}

/* ====================================================================
 * Process the list of operations created by Collect_Base_Offset and
 * transformed by Generate_Common_Base.
 *
 * All operations are on the same base TN. Use one or several
 * auxiliary base pointer to replace extended offsets by a reference
 * to an auxiliary base pointer and a small offset.
 * ====================================================================
 */

static void
Generate_Common_Base(vec_base_ops Base_Ops_List) {
  int i, j;

  for (i = 0; i < Base_Ops_List.size(); i++) {
    base_ops& base_list = Base_Ops_List[i];
    vec_bo_info& op_offset = *VEC_BASE_ops(base_list);

    if (Trace_SSA_CBPO)
      Print_base_ops(TFile, base_list);

    if (op_offset.size() <= 1) continue;

    // See if all bases are the same: If true, consider only extended
    // offets from the list. Otherwise, consider all operations.

    tn *base_tn = BO_base_tn(op_offset[0]);
    INT register_base_idx = -1;
    BOOL all_same_base = TRUE;
    INT extended_count = 0;

    for (j = 0; j < op_offset.size(); j++) {
      if (BO_base_tn(op_offset[j]) != base_tn)
	all_same_base = FALSE;
      if (register_base_idx == -1 && TN_is_register(BO_base_tn(op_offset[j])))
	register_base_idx = j;
      TN *tn_offset = BO_offset_tn(op_offset[j]);
      INT64 val;
      if (!TN_is_constant(tn_offset) || CGTARG_offset_is_extended(tn_offset, &val))
	extended_count ++;
    }

    if (all_same_base && (extended_count <= 2))
      continue;

    TN *common_base_tn;
    INT64 common_adjust;
    // In cases common_base_tn is not a register, we may be able to
    // provide information for rematerialization
    TN *remat_base = NULL;
    INT64 remat_adjust = 0;

    // All bases are the same register, use this one to create
    // auxilliary bases.
    if (all_same_base && register_base_idx != -1) {
      common_base_tn = BO_base_tn(op_offset[register_base_idx]);
      common_adjust = BO_base_dist(op_offset[register_base_idx]);
    }

    else {
      // Address expressions are derived from a common register or
      // a common symbol, or are immediate values.
      common_base_tn = VEC_BASE_base(base_list);
      common_adjust = 0;
      // In case common_base_tn is a symbol or an immediate value, if
      // a register contains this symbol or value, use it to get
      // information for rematerialization.
      if (((common_base_tn == NULL) || (!TN_is_register(common_base_tn))) &&
	  (register_base_idx != -1)) {
	remat_base = BO_base_tn(op_offset[register_base_idx]);
	remat_adjust = BO_base_dist(op_offset[register_base_idx]);
      }
    }

    if ((common_base_tn != NULL) && TN_is_dedicated(common_base_tn) && !TN_is_const_reg(common_base_tn)) {
      if (Trace_SSA_CBPO) {
	fPrint_TN(TFile, "Generate_Common_Base: %s is not optimized", VEC_BASE_base(base_list));
	fprintf(TFile, " (gain would be at most %d bytes)\n", (op_offset.size()-2)*4);
      }
      continue;
    }

    if (Trace_SSA_CBPO) {
      fPrint_TN(TFile, "Generate_Common_Base: %s is optimized", VEC_BASE_base(base_list));
      fprintf(TFile, " (gain will be at most %d bytes)\n", (op_offset.size()-2)*4);
    }

    // Sort according to the offsets
    stable_sort(op_offset.begin(), op_offset.end(), offset_lt);

    // Now, use a greedy algorithm
    int j_first;
    all_same_base = TRUE;
    base_tn = BO_base_tn(op_offset[0]);
    extended_count = 0;
    INT64 val;

    for (j = j_first = 0; j < op_offset.size(); j++) {
      if ((BO_addr_dist(op_offset[j]) - BO_addr_dist(op_offset[j_first])) >= (MAX_WIDTH-(min_offset_alignment-1))) {
	if ((j - j_first > 2) &&
	    (!all_same_base || (extended_count > 2)))
	  Update_Ops_With_New_Base(common_base_tn, common_adjust, remat_base, remat_adjust, op_offset, j_first, j-1);
	j_first = j;
	base_tn = BO_base_tn(op_offset[j_first]);
	all_same_base = TRUE;
	extended_count = 0;
      }
      if (BO_base_tn(op_offset[j]) != base_tn)
	all_same_base = FALSE;
      if (!TN_is_constant(BO_offset_tn(op_offset[j])) || CGTARG_offset_is_extended(BO_offset_tn(op_offset[j]), &val))
	extended_count ++;
    }
    if ((j - j_first > 2) &&
	(!all_same_base || (extended_count > 2)))
      Update_Ops_With_New_Base(common_base_tn, common_adjust, remat_base, remat_adjust, op_offset, j_first, j-1);
  }
}

/* ====================================================================
 * Optimize operations in the form 'offset[base]'. 
 * ====================================================================
 */

static void
Optimize_Extended_Offset()
{
#ifdef Is_True_On
  if(!getenv("NO_CBPO")) {
#endif

  Trace_SSA_CBPO = Get_Trace(TP_SSA, SSA_CBPO);
  min_frequency_ratio = CG_cbpo_ratio / 100.0;

  MEM_POOL_Initialize (&base_offset_pool, "base_offset map pool", TRUE);
  MEM_POOL_Push(&base_offset_pool);
  MEM_POOL_Initialize (&extended_offset_pool, "op_offset map pool", TRUE);
  MEM_POOL_Push(&extended_offset_pool);

  vec_base_ops Base_Ops_List;

  if (Trace_SSA_CBPO) {
    fprintf(TFile, "%s CFG Before Optimize_Extended_Offset %s\n%s\n", DBar, __FUNCTION__, DBar);
    Print_All_BBs ();
  }

  base_offset_map = TN_MAP_Create();
  constant_map = TN_MAP_Create();
  
  CGTARG_Get_Info_For_Common_Base_Opt(&min_offset_alignment, &MIN_OFFSET, &MAX_OFFSET);
  MAX_WIDTH = MAX_OFFSET - MIN_OFFSET;

  Optimized_Extended_Immediate = FALSE;

  Propagate_Base_Offset();

  Collect_Base_Offset(Base_Ops_List);
  
  Generate_Common_Base(Base_Ops_List);

#ifdef TARG_ST
  // Resize instructions, otherwise EBO sees no penalty at inlining
  // extended immediates.
  if (Optimized_Extended_Immediate)
    CGTARG_Resize_Instructions ();
#endif

  if (Trace_SSA_CBPO) {
    fprintf(TFile, "%s CFG After Optimize_Extended_Offset %s\n%s\n", DBar, __FUNCTION__, DBar);
    Print_All_BBs ();
  }

  TN_MAP_Delete(base_offset_map);
  TN_MAP_Delete(constant_map);

  MEM_POOL_Pop(&extended_offset_pool);
  MEM_POOL_Delete(&extended_offset_pool);
  MEM_POOL_Pop(&base_offset_pool);
  MEM_POOL_Delete(&base_offset_pool);

#ifdef Is_True_On
  } /* !getenv("NO_CBPO") */
#endif
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

