/*

  Copyright (C) 2000 Silicon Graphics, Inc.  All Rights Reserved.

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

/* ====================================================================
 * ====================================================================
 *
 * Module: whirl2ops.cxx
 *
 * Description:
 *
 * This	file contains the routines to convert WHIRL which is input to cg
 * into OPs. This includes creating basic blocks, TNs.
 *
 * ====================================================================
 * ====================================================================
 */

#include "W_alloca.h"
#include <ctype.h>
#include <vector>
#ifdef TARG_ST
#include <stdarg.h>
#endif
#include "defs.h"
#include "mempool.h"
#include "wn.h"
#include "symtab.h"
#include "const.h"
#include "erbe.h"
#include "ercg.h"
#include "erglob.h"
#include "tracing.h"
#include "glob.h"
#include "config.h"
#include "config_TARG.h"
#include "topcode.h"
#include "targ_isa_lits.h"
#include "intrn_info.h"
#include "opcode.h"
#include "w2op.h"
#include "wn_util.h"
#include "ir_reader.h"
#include "cgir.h"
#include "region_util.h"
#include "cg_region.h"
#include "cg.h"
#include "calls.h"
#include "cgtarget.h"
#include "cgexp.h"
#include "stblock.h"
#include "targ_sim.h"
#include "irbdata.h"
#include "ttype.h"
#include "pf_cg.h"
#include "wn_map.h"
#include "whirl2ops.h"
#include "gtn_universe.h"
#include "variants.h"
#include "targ_sim.h"
#include "eh_region.h"
#include "fb_whirl.h"
#include "xstats.h"
#include "data_layout.h"
#include "label_util.h"
#include "comment.h"
#include "be_symtab.h"
#include "be_util.h"
#include "config_asm.h"

#ifdef TARG_ST
#include "betarget.h"    /* for Target_Has_Immediate_Operand */
#include "bb_map.h"
#include "register_preg.h" /* For CGTARG_Regclass_Preg_Min() */
#include "config_opt.h" /* For OPT_Enable_Warn_Assume */
#include "cg_affirm.h"
#endif

#if   defined ( TARG_ST200  )
#include "../gccfe/gnu/st200/insn-config.h" /* for MAX_RECOG_OPERANDS */
#elif defined ( TARG_STxP70 )
#include "../gccfe/gnu/stxp70/insn-config.h" /* for MAX_RECOG_OPERANDS */
#endif

#ifdef TARG_ST200
#define ENABLE_64_BITS  /* Activate 64 bits support for st200 family. */
#endif

#if defined(CGG_ENABLED)
#include "targ_cgg_exp.h"
static TN * Expand_Expr_local (WN *expr, WN *parent, TN *result);
static void Expand_Statement_local (WN *stmt);
static TN * Expand_Expr (WN *expr, WN *parent, TN *result);
static void Expand_Statement (WN *stmt);
#define CGG_STATIC 
#else
#define CGG_STATIC static
#endif


#ifdef EMULATE_LONGLONG
extern void Add_TN_Pair (TN*, TN*);
extern TN *If_Get_TN_Pair(TN*);
extern TN *Gen_Literal_TN_Pair(UINT64);
#endif

// #define BB_SEARCH_LIMIT 1000	 some ridiculously high number

BOOL Compiling_Proper_REGION;
static BOOL Trace_WhirlToOp = FALSE;

/* reference to a dedicated TN in Cur_BB */
static BOOL dedicated_seen;

#ifdef CGG_ENABLED
CGG_STATIC BOOL In_Glue_Region = FALSE;	/* in glue-code region */
#else
static BOOL In_Glue_Region = FALSE;	/* in glue-code region */
#endif

/* Forward declarations. */
static TN * Expand_Expr (WN *expr, WN *parent, TN *result);
static void initialize_region_stack(WN *);
static RID *region_stack_pop(void);
static void region_stack_push(RID *value);
static void region_stack_eh_set_has_call(void);

#ifdef TARG_ST
/* Declaration of a structure used to
 * managed result of intrinsics.
 */
typedef struct{
   UINT   numres;
   TN    *res[ISA_OPERAND_max_results];
} INTRINSIC_RESULT;

static INTRINSIC_RESULT *Intrinsic_Result_Sav = NULL;
static UINT              Intrinsic_Count      = 0;

/* Forward declaration */
static void               push_intrinsic_result(WN* intrincall, TN **res, INT numres);
static INTRINSIC_RESULT*  get_last_intrinsic_result(void);
#endif                       /* TARG_ST */

/*
static INT16 WHIRL_Compare_To_OP_variant (OPCODE opcode, BOOL invert);
*/

/* The cgexp routines now take as input an OPS to which the
 * expanded OPs are added.
 */
static OPS New_OPs;

static OP *Last_Processed_OP;
#ifdef CGG_ENABLED
CGG_STATIC SRCPOS current_srcpos;
#else
static SRCPOS current_srcpos;
#endif
static INT total_bb_insts;

/* The current basic block being generated. */
#ifdef CGG_ENABLED
CGG_STATIC BB *Cur_BB;
#else
static BB *Cur_BB;
#endif

static RID **region_stack_base;
static RID **region_stack_ptr;
static INT   region_stack_size;
#define current_region (*(region_stack_ptr - 1))
#define region_depth   (region_stack_ptr - region_stack_base)
static BB_NUM min_bb_id;

#ifdef TARG_ST
static BB_MAP loop_pragma_map;
#else
static WN *last_loop_pragma;
#endif

#define return_max 3

/*
 * Last_Mem_OP is used to keep track of the last
 * op processed for the memory op to wn mapping.
 */

#ifdef CGG_ENABLED
CGG_STATIC OP *Last_Mem_OP;
#else
static OP *Last_Mem_OP;
#endif
OP_MAP OP_to_WN_map;
static OP_MAP predicate_map = NULL;
static WN_MAP WN_to_OP_map;

OP_MAP OP_Asm_Map;

#ifdef TARG_ST
OP_MAP OP_to_callinfo_map;
#endif

TN *
Get_Complement_TN(TN *tn)
{
  TN *c_tn;
  PREG_NUM preg = TN_To_PREG(tn);
  if (preg != 0) {
    PREG_NUM c_preg = preg + 1;
    c_tn = PREG_To_TN_Array[c_preg];
  } else {
    c_tn = Build_TN_Like(tn);
  }
  return c_tn;
}

#ifdef TARG_ST
// FdF 20070510: For an operation created by load/store packing,
// associate the list of original operations.

static OP_MAP OP_packed_to_ops_map = NULL;

static OP *
Link_Packed_Op(OP *opi, OP *opj) {
  // If opj is already a packed operation, be sure to get down to the
  // chain
  int dummy;
  OP *op;

  OP *opfirst = opj;
  if (OP_packed(opfirst))
    opfirst = Get_First_Packed_Op(opfirst, &dummy);

  OP_MAP_Set(OP_packed_to_ops_map, opi, opfirst);

  // Then, find the end of the chain for linking with a next op
  OP *oplast = opfirst;
  while ((op = Get_Next_Packed_Op(opfirst, oplast, &dummy)) != NULL)
    oplast = op;

  return oplast;
}

void
Set_Packed_Ops(OP *packed_op, INT n, OP *op1, ...) {

  if (OP_packed_to_ops_map == NULL)
    OP_packed_to_ops_map = OP_MAP_Create();

  OP *dup_op;

  // FdF 20070515: Call Dup_OP since op1 may later be converted into a
  // noop when removed from the code.
  dup_op = Dup_OP(op1);
  dup_op->bb = OP_bb(op1);
  Copy_WN_For_Memory_OP (dup_op, op1);

  // If op1 is already a packed operation, be sure to get down to the
  // chain
  op1 = Link_Packed_Op(packed_op, dup_op);
  
  INT i;
  va_list ops;
  OP* opi = op1, *opj;

  va_start(ops, op1);
  for (i = 1; i < n; i++) {
    opj = va_arg(ops, OP *);

    // FdF 20070515: Call Dup_OP since opj may later be converted into
    // a noop when removed from the code.
    dup_op = Dup_OP(opj);
    dup_op->bb = OP_bb(opj);
    Copy_WN_For_Memory_OP (dup_op, opj);
    opj = dup_op;
    opi = Link_Packed_Op(opi, opj);
  }
  va_end(ops);

  Set_OP_packed(packed_op);
}

void
Copy_Packed_Ops(OP *dest_packed, OP *src_packed) {

  Is_True(OP_packed(src_packed) && (OP_packed_to_ops_map != NULL),
	  ("Copy_Packed_Ops_N: Must be called with a packed op.\n"));

  OP *opi = dest_packed, *opj;
  int dummy;

  // OP_MAP_Set(OP_packed_to_ops_map, dest_packed, Get_First_Packed_Op(src_packed, &dummy));

  // FdF 20071022: The entire chain is now duplicated, so that each
  // packed operation points to its own copies of the original
  // operations. This way, it is possible in cg_dep_graph to update
  // these copies of original operations with information on the
  // packed operation (unoll_bb, unrollings, orig_idx).
  for (opj = Get_First_Packed_Op(src_packed, &dummy); opj; opj = Get_Next_Packed_Op(src_packed, opj, &dummy)) {
    OP *dup_opj;
    dup_opj = Dup_OP(opj);
    dup_opj->bb = OP_bb(opj);
    Copy_WN_For_Memory_OP (dup_opj, opj);
    opi = Link_Packed_Op(opi, dup_opj);
  }

  Set_OP_packed(dest_packed);
}

// FdF 20070510: For a packed operation, return the first operation
// associated to packed_op. For a non packed operation, return the
// original operation.

OP *
Get_First_Packed_Op(OP *packed_op, int *offset) {

  // *offset must be set to zero in any case.
  *offset = 0;

  // If not a packed op, return NULL
  if ((OP_packed_to_ops_map == NULL) || !OP_packed(packed_op))
    return NULL;

  return (OP *)OP_MAP_Get(OP_packed_to_ops_map, packed_op);
}

// FdF 20070510: For a packed operation, return the next operation
// after opi associated to packed_op. For a non packed operation,
// return NULL;

OP *
Get_Next_Packed_Op(OP *packed_op, OP *opi, int *offset) {

  // If not a packed op, or the last unit op of a packed op, return
  // NULL
  if ((OP_packed_to_ops_map == NULL) || !OP_packed(packed_op))
    return NULL;

  // For example, 4 16 bit load (h1-h4) can be packed into one 64 bit load (l1) and
  // also in 2 32 bit load (w1,w2). This gives the following chain:
  // l1->h1->h2->h3->h4
  //   /       /
  // w1      w2
  // When looking at packed ops for w1, we must stop after h2.
  
  *offset += OP_Mem_Ref_Bytes(opi);
  if (*offset >= OP_Mem_Ref_Bytes(packed_op))
    return NULL;
  return (OP *)OP_MAP_Get(OP_packed_to_ops_map, opi);
}
#endif

void Copy_WN_For_Memory_OP(OP *dest, OP *src)
{
  WN *wn = Get_WN_From_Memory_OP(src);
  UINT64 predicate = predicate_map ? OP_MAP64_Get(predicate_map, src) : 0;
  if (wn)
    OP_MAP_Set(OP_to_WN_map, dest, wn);
  if (predicate)
    OP_MAP64_Set(predicate_map, dest, predicate);

#ifdef TARG_ST
  // FdF 20070627: Need also to copy on a packed operation the map to
  // the unit elements of the original packed operation.
  if (OP_packed(src)) {
    Is_True(OP_Mem_Ref_Bytes(dest) == OP_Mem_Ref_Bytes(src),
	    ("Copy_WN_For_Memory_OP: src and dst do not have the same size."));
    Copy_Packed_Ops(dest, src);
  }
#endif
}


#ifdef TARG_ST
/* [TTh] Test if the memory OP is associated to a WN and
 * if its memory access is smaller than the WN memory access
 * (case of splitted memory accesses)
 */
BOOL Memory_OP_Is_Partial_WN_Access(OP *op) {
  if (OP_memory(op) && !OP_hoisted(op)) {
    WN *wn = Get_WN_From_Memory_OP(op);
    if (wn && (TOP_Mem_Bytes(OP_code(op)) < MTYPE_byte_size(WN_desc(wn)))) {
      return TRUE;
    }
  }
  return FALSE;
}
#endif


OP *Get_OP_From_WN(WN *wn ) 
{
  return (OP*) WN_MAP_Get(WN_to_OP_map, wn);
}


void Set_Memory_OP_Predicate_Info(OP *memop, TN *pred_tn, UINT8 omega,
				  BOOL inverted)
{
  UINT64 predicate = pred_tn ?
    (TN_number(pred_tn) | (UINT64)omega << 32 | (UINT64)inverted << 40) : 0;
  if (predicate_map == NULL && predicate) predicate_map = OP_MAP64_Create();
  if (predicate || predicate_map && OP_MAP64_Get(predicate_map, memop))
    OP_MAP64_Set(predicate_map, memop, predicate);
}
  
void Get_Memory_OP_Predicate_Info(OP *memop, TN **pred_tn, UINT8 *omega,
				  BOOL *inverted)
{
  UINT64 predicate = predicate_map ? OP_MAP64_Get(predicate_map, memop) : 0;
  *pred_tn = predicate ? TNvec(predicate & 0xffffffff) : NULL;
  *omega = (predicate >> 32) & 0xff;
  *inverted = (predicate >> 40) & 1;
}


/* =======================================================================
 *
 *  initialize_region_stack
 *
 *  initialize the stack giving the current region number.
 *  Make sure that the POOL we use is pushed and popped somewhere.
 *
 * =======================================================================
 */
static void initialize_region_stack(WN *wn)
{
  RID *rid = REGION_get_rid(wn);
  Is_True(rid != NULL, ("initialize_region_stack, NULL RID"));

  region_stack_size = 8;
  region_stack_base = TYPE_MEM_POOL_ALLOC_N( RID *, &MEM_local_pool,
					    region_stack_size );
  region_stack_ptr = region_stack_base;

  region_stack_push( rid );
}

/* =======================================================================
 *
 *  region_stack_pop
 *
 *  pop an element from the region stack
 *
 * =======================================================================
 */
static RID *region_stack_pop(void)
{
  if ( region_stack_ptr == region_stack_base )
    return NULL;
  else
    return *(--region_stack_ptr);
}

/* =======================================================================
 *
 *  region_stack_push
 *
 *  push an element on the region stack; grow the stack if necessary
 *
 * =======================================================================
 */
static void region_stack_push(RID *value)
{
  *(region_stack_ptr++) = value;
  if ( ( region_stack_ptr - region_stack_base ) == region_stack_size ) {
    region_stack_base = TYPE_MEM_POOL_REALLOC_N( RID *, &MEM_local_pool,
						 region_stack_base, region_stack_size, 
						 2 * region_stack_size );
    region_stack_ptr = region_stack_base + region_stack_size;
    region_stack_size = 2 * region_stack_size;
  }
}

/* =======================================================================
 *
 *  region_stack_eh_set_has_call
 *
 *  call EH_Set_Has_Call for every EH region on the region stack
 *
 * =======================================================================
 */

static void 
region_stack_eh_set_has_call (void)
{
  RID ** p;
  for (p = region_stack_ptr - 1; p >= region_stack_base; --p)
#ifdef KEY
    if (RID_TYPE_eh(*p) && RID_eh_range_ptr(*p))
#else
    if (RID_TYPE_eh(*p))
#endif
      EH_Set_Has_Call(RID_eh_range_ptr(*p));
}

/* =======================================================================
 *   Process_New_OPs
 *
 * Process the new OPs that have been created since the last call to 
 * this routine. We set their srcpos field and increment the count
 * of number of OPs in the BB. We check if any of the new OPs have
 * a reference to GP and remember the fact.
 * =======================================================================
 */
static void
Process_New_OPs (void)
{
  OP *op;
  INT i;

  op = Last_Processed_OP ? OP_next(Last_Processed_OP) : OPS_first(&New_OPs);

  for (; op != NULL; op = OP_next(op)) {
    for (i = OP_opnds(op)-1; i >= 0; i--) {
      if (OP_opnd(op,i) == GP_TN) PU_References_GP = TRUE;
    }
    OP_srcpos(op) = current_srcpos;
    total_bb_insts++;
  }
  Last_Processed_OP = OPS_last(&New_OPs);
}

#if defined TARG_ST || defined EMULATE_LONGLONG
/***********************************************************************
 *
 * Assume that orig_bb was split into orig_bb followed by new_bb,
 * with the ops being split between the two.
 * 
 * Update Annotations, flags etc appropriately.
 *
 ***********************************************************************/
static void 
Update_BB_Properties (BB *orig_bb, BB *new_bb) {

  BB_rid(new_bb) = BB_rid(orig_bb);
  BB_branch_wn(new_bb) = BB_branch_wn(orig_bb);
  BB_branch_wn(orig_bb) = NULL;

  ANNOTATION *ant = BB_annotations(orig_bb);

  while (ant) {
    switch (ANNOT_kind(ant)) {
    case ANNOT_LABEL:
      // this stays with orig_bb
      break;
    case ANNOT_PRAGMA:
      #pragma mips_frequency_hint NEVER
      FmtAssert (FALSE,
                 ("Update_BB_Properties: foxed by pragma annotation\n"));
      
      /*NOTREACHED*/
    case ANNOT_ENTRYINFO:
      // this stays with orig_bb;
      Reset_BB_entry(new_bb);
      break;
    case ANNOT_EXITINFO:
    case ANNOT_CALLINFO:
    case ANNOT_ASMINFO:
      // exitinfo, callinfo and asminfo move to new_bb
      BB_annotations(orig_bb) = ANNOT_Unlink(BB_annotations(orig_bb),
                                             ant);
      ANNOT_next(ant) = BB_annotations(new_bb);
      BB_annotations(new_bb) = ant;

      if (ANNOT_kind(ant) == ANNOT_CALLINFO) {
        Reset_BB_call(orig_bb);
        Set_BB_call(new_bb);
      } else if (ANNOT_kind(ant) == ANNOT_ASMINFO) {
	Reset_BB_asm(orig_bb);
	Set_BB_asm(new_bb);
      } else {
        Reset_BB_exit(orig_bb);
        Set_BB_exit(new_bb);
      }
      break;
    case ANNOT_NOTE: // we should tolerate comment
      break;
    case ANNOT_LOOPINFO:
    case ANNOT_SWITCH:
    case ANNOT_ROTATING_KERNEL:
    default:
      #pragma mips_frequency_hint NEVER
      FmtAssert (FALSE,
                 ("Update_BB_Properties: Unknown annotation %d\n", ANNOT_kind(ant)));
      /*NOTREACHED*/
    }
    ant = ANNOT_next(ant);
  }
  
}

/***********************************************************************
 *
 * Given a BB, we may need to split it if it contains a
 * jump or a label (or both) in its midst. If so, split it into one or more
 * BBs, so that each of the resulting BBs is guaranteed to not have
 * labels in the middle (through TOP_label) or jumps in the middle.
 * 
 * Return a pointer to the last bb created.
 *
 ***********************************************************************/
static BB *
Split_Jumpy_BB (BB *bb) {

  BOOL need_to_split = FALSE;
  OP *op;
  for (op = BB_first_op(bb); op; op = OP_next(op)) {
    if (OP_xfer(op) && op != BB_last_op(bb)) {
      need_to_split = TRUE;
      break;
    }
    if (OP_code(op) == TOP_label) {
      need_to_split = TRUE;
      break;
    }
    if (op == BB_last_op(bb)) break;
  }

  if (!need_to_split) return bb;

  // do the split
  for (op = BB_first_op(bb); op; ) {

    // Split if it is a jump
    if (OP_xfer(op) && op != BB_last_op(bb)) {
      BB *new_bb = Gen_And_Append_BB (bb);
      op = OP_next(op);

      while (op) {
        OP *tmp_op = OP_next(op);
        BB_Move_Op_To_End (new_bb, bb, op);
        op = tmp_op;
      }

      Update_BB_Properties (bb, new_bb);

#ifdef TARG_ST
      if (OP_call(BB_last_op(bb))) {
	// [SC] if bb now ends in a call, there must have been
	// a call in the middle of it.
	// Callinfo for calls that are not at the end of a block
	// should have been added to OP_to_callinfo_map.
	// Extract that information here and add it to bb.
	CALLINFO *call_info = (CALLINFO *) OP_MAP_Get (OP_to_callinfo_map, BB_last_op(bb));
	BB_Add_Annotation (bb, ANNOT_CALLINFO, call_info);
        Set_BB_call(bb);
	region_stack_eh_set_has_call ();
      }
#endif
      bb = new_bb;
      op = BB_first_op(new_bb);
      continue;
    }

    // split if it is a label
    if (OP_code(op) == TOP_label) {
      BB *new_bb = Gen_And_Append_BB (bb);
      while (op) {
        OP *tmp_op = OP_next(op);
        BB_Move_Op_To_End (new_bb, bb, op);
        op = tmp_op;
      }

      // setup the label, and then 
      // reset values so we can continue outer for
      Update_BB_Properties (bb, new_bb);
      bb = new_bb;
      op = BB_first_op(new_bb);
      Is_True (OP_code(op) == TOP_label,
               ("Split_Jumpy_BB: where did the dummy label go?"));
      TN *label_tn = OP_opnd(op,0);
      LABEL_IDX label = TN_label(label_tn);
      BB_Add_Annotation(bb, ANNOT_LABEL, (void*) label);
      FmtAssert (Get_Label_BB(label) == NULL,
                 ("Split_Jumpy_BB: Label %s defined more than once",
                  LABEL_name(label)));
      Set_Label_BB (label,bb);
      Set_BB_has_label(bb);
      BB_Remove_Op (bb, op);
      op = BB_first_op(bb);
      continue;
    }

    if (op == BB_last_op(bb)) break;

    op = OP_next(op);
  }
  
  return bb;
}
#endif /* TARG_ST || EMULATE_LONGLONG */


/* Start a new basic block. Any OPs that have not been put into a BB
 * are added to the current basic block before we create a new one.
 */
static BB *
Start_New_Basic_Block (void)
{
  BB *bb = Cur_BB;

  if (bb == NULL) {
    // Cur_BB is NULL if we don't have any basic blocks in the PU yet.
    // Mark this BB as REGION_First_BB.
    bb = Gen_And_Append_BB (bb);
    REGION_First_BB = bb;
  }
  // check if we already have instructions in the basic block
  FmtAssert (BB_first_op(bb) == NULL, ("Start_New_Basic_Block: error"));

  if (OPS_first(&New_OPs) != NULL) {
    Process_New_OPs ();
    BB_Append_Ops(bb, &New_OPs);
    OPS_Init(&New_OPs);
#if defined TARG_ST || defined EMULATE_LONGLONG
    bb = Split_Jumpy_BB (bb);
#endif
    if ( dedicated_seen ) {
      Set_BB_has_globals( bb );
    }
    else {
      Reset_BB_has_globals( bb );
    }
    bb = Gen_And_Append_BB (bb);
  } else if (BB_entry(bb) || BB_exit(bb)) {
    // If current basic block is a entry point or an exit, create a new
    // basic block, even if the current one is empty. This is because code 
    // will be added later to the entry/exit bb.
    bb = Gen_And_Append_BB (bb);
  }
#ifdef TARG_ST
  // FdF 23/04/2004: Because loop pragmas must now be associated with
  // the pre-header of the loop, before being moved to the loop
  // header.
  else if (BB_MAP_Get(loop_pragma_map, bb)) {
    bb = Gen_And_Append_BB (bb);
  }
  // FdF 12/05/2004: Frequency_Hint pragmas are attached to a specific
  // basic block. Even if empty, a basic block must be created to
  // attach this pragma.
  else if (BB_has_pragma(bb)) {
    ANNOTATION *ant;

    for ( ant = ANNOT_First(BB_annotations(bb), ANNOT_PRAGMA);
	  ant != NULL;
	  ant = ANNOT_Next(ant, ANNOT_PRAGMA)) {
      WN *wn = ANNOT_pragma(ant);
      WN_PRAGMA_ID pragma = (WN_PRAGMA_ID) WN_pragma(wn);
      if (pragma == WN_PRAGMA_MIPS_FREQUENCY_HINT) {
	bb = Gen_And_Append_BB (bb);
	break;
      }
    }
  }
#endif

  if (PU_has_region(Get_Current_PU())) {
    BB_rid(bb) = Non_Transparent_RID(current_region);
  }
  else {
    BB_rid(bb) = NULL;
  }

  total_bb_insts = 0;
  Last_Processed_OP = NULL;
  Last_Mem_OP = NULL;
  Cur_BB = bb;
  dedicated_seen = FALSE;

  return bb;
}

/* Check if we are over the threshold of how many instructions
 * we will allow in a single basic block. If we are, then call 
 * 'Start_New_Basic_Block' to start a new basic block.
 */
static void
Process_OPs_For_Stmt (void)
{
  Process_New_OPs ();

#if 0	// now done later in Split_BBs()
  if (Enable_BB_Splitting && (total_bb_insts > Split_BB_Length)) {
    /* We assume in LRA that the number of instructions in a BB fits
     * in 16 bits.
     */
    FmtAssert (total_bb_insts < 32768,
  	  ("Convert_WHIRL_To_OPs: Too many instructions for 1 statment (%d)\n", 
	   total_bb_insts));
    if (Trace_WhirlToOp) {
          fprintf (TFile, "Convert_WHIRL_To_OPs: splitting a large BB (%d)\n", 
		    total_bb_insts);
    }
    Start_New_Basic_Block ();
  }
#endif
}

/* ====================================================================
 *   Allocate_Result_TN
 *
 * Create a new result TN for the WHIRL node wn. We pass in the opnd_tn
 * array that contains the TNs allocated for each operand. This could
 * used to do value numbering if desired.
 *
 * Currently we allocate a new TN for every invocation of this procedure.
 * ====================================================================
 */
static TN *
Allocate_Result_TN (
  WN *wn, 
  TN **opnd_tn
)
{
#ifdef EMULATE_LONGLONG

  TYPE_ID mtype = WN_rtype(wn);
  if (mtype == MTYPE_I8 || mtype == MTYPE_U8) {
    TYPE_ID new_mtype = (mtype == MTYPE_I8 ? MTYPE_I4 : MTYPE_U4);
    TN *tn  = Build_TN_Of_Mtype (new_mtype);
    TN *tn2 = Build_TN_Of_Mtype (new_mtype);
    Add_TN_Pair (tn, tn2);
    return tn;
  }
  else return Build_TN_Of_Mtype (WN_rtype(wn));

#else 
  FmtAssert (WN_rtype(wn) != MTYPE_V, ("Unexpected allocation of result void type result"));
  return Build_TN_Of_Mtype (WN_rtype(wn));
#endif
}

/*
 * Set the op2wn mappings for memory ops
 */
#ifdef CGG_ENABLED
CGG_STATIC void
#else
static void
#endif
Set_OP_To_WN_Map(WN *wn)
{
  OP *op;

  op = Last_Mem_OP ? OP_next(Last_Mem_OP) : OPS_first(&New_OPs);
  for ( ; op != NULL; op = OP_next(op)) {
    if ( (!OP_memory(op) || OP_no_alias(op)) && !OP_call(op) 
	&& !OP_Is_Barrier(op) && OP_code(op) != TOP_spadjust)
	continue;

    // Mark volatile operations
    if (OP_memory(op) && WN_Is_Volatile_Mem(wn))
      Set_OP_volatile(op);

    // We don't have aliasing information at -O0 and -O1.
    if (CG_opt_level < 2) continue;

    if (OP_prefetch(op)) {

      /* These maps will have already been created if Alias_Manager
       * is TRUE. However, if we have prefetches and no alias manager
       * (compiled with -O3 -PHASE:w=0 for example) then we create it
       * the first time we need it since we can't otherwise predict
       * it and we don't want to create it for -O0, etc.
       */
      if (OP_to_WN_map == NULL) {
	OP_to_WN_map = OP_MAP_Create();
	Is_True(WN_to_OP_map == WN_MAP_UNDEFINED,
		("WN_to_OP_map has already been created"));
	WN_to_OP_map = WN_MAP_Create(&MEM_phase_pool);
      }
      OP_MAP_Set(OP_to_WN_map, op, wn);
      WN_MAP_Set(WN_to_OP_map, wn, op);
    } else if (Alias_Manager) {
      OP_MAP_Set(OP_to_WN_map, op, wn);
    }
  }

  Last_Mem_OP = OPS_last(&New_OPs);
}

/* don't home structs or arrays of arrays for now */
static BOOL Disallowed_Homeable(ST *sym)
{
  if (Is_Structure_Type(ST_type(sym))) {
    return TRUE;
  } else if (TY_kind(ST_type(sym)) == KIND_ARRAY &&
	     (TY_kind(TY_AR_etype(ST_type(sym))) == KIND_STRUCT || 
	      TY_kind(TY_AR_etype(ST_type(sym))) == KIND_ARRAY)) {
    return TRUE;
  } 
  return FALSE;
}

/* ====================================================================
 *   Preg_Is_Rematerializable
 *
 * check if TN is rematerializable. Can't handle IDNAME's right now 
 * filter out things like IDNAME, FQCONST and uplevel references
 * ====================================================================
 */
extern WN* 
Preg_Is_Rematerializable (
  PREG_NUM preg, 
  BOOL *gra_homeable
)
{
  WN	*home= Preg_Home(preg);
  OPCODE opc;

  if (home == NULL) 
	return NULL;
  opc = WN_opcode(home);

  /* allow homing on symbols with simple addressing */
  if (WN_operator(home) == OPR_LDID) {
    ST *sym = WN_st(home);
    ST *basesym = Base_Symbol(sym);
    TYPE_ID rtype = OPCODE_rtype(opc);

    /* can't handle quad's without lowerer support.  defer for now.
     * shouldn't see complex as they're expanded when the optimizer
     * sees them, but what the heck.
     */
    if (MTYPE_is_quad(rtype)) {
      return NULL;
    }
    if (MTYPE_is_complex(rtype)) {
      return NULL;
    }

    /* can't do homing without the alias manager.  we shouldn't get
     * here anyway as we won't home without running wopt and thus
     * should have an alias manager, but let's be paranoid.
     */
    if ( !Alias_Manager ) {
      return NULL;
    }

    /* disallow complex data types, but allow common blocks (which sorta
     * look like a complex data type).
     */
    if (GRA_home == TRUE && !Disallowed_Homeable(sym) &&
	ST_class(sym) == CLASS_VAR) {
      if (((ST_gprel(basesym) ||
	   (ST_is_split_common(basesym) && ST_gprel(ST_full(basesym))))
	   && ! ST_is_const_var (sym)
	   && CGTARG_GP_Expressions_Are_Rematerializable_p)
	  || ST_on_stack(sym)) {
	*gra_homeable = TRUE;
	return home;
      }
    }
    return NULL;
  }
  if (MTYPE_is_complex(OPCODE_rtype(opc)))
	return NULL;
  if (MTYPE_is_quad(OPCODE_rtype(opc)))
	return NULL;
  if (OPCODE_has_sym(opc) && ST_is_uplevelTemp(WN_st(home)))
	return NULL;
  if (WN_operator(home) == OPR_LDA
      && ! CGTARG_Address_Constants_Are_Rematerializable_p)
        return NULL;

  return home;
}

/* ====================================================================
 *                       PREG Interface
 * ====================================================================
 */
/* array to map PREGs into TNs. */
TN **PREG_To_TN_Array;
TYPE_ID *PREG_To_TN_Mtype;
static PREG_NUM max_preg_to_tn_index = 0;

// if we create new pregs, then need to realloc space.
static void Realloc_Preg_To_TN_Arrays (PREG_NUM preg_num)
{
  if (preg_num < max_preg_to_tn_index) return;
  	
  max_preg_to_tn_index = Get_Preg_Num (PREG_Table_Size(CURRENT_SYMTAB));
  PREG_To_TN_Array = TYPE_MEM_POOL_REALLOC_N( TN *, &MEM_pu_pool,
	 PREG_To_TN_Array, max_preg_to_tn_index, max_preg_to_tn_index + 10);
  PREG_To_TN_Mtype = TYPE_MEM_POOL_REALLOC_N( TYPE_ID, &MEM_pu_pool,
	 PREG_To_TN_Mtype, max_preg_to_tn_index, max_preg_to_tn_index + 10);
  max_preg_to_tn_index += 10;
}

/* ====================================================================
 *   Set_TN_For_PREG
 *
 *   create a TN for this PREG.
 * ====================================================================
 */
TN*
Set_TN_For_PREG (
  PREG_NUM preg_num,
  TYPE_ID mtype
)
{
  TN *tn;
  ISA_REGISTER_CLASS rclass;
  REGISTER reg;

#ifdef EMULATE_LONGLONG
  if (mtype == MTYPE_I8 || mtype == MTYPE_U8) {
    mtype = (mtype == MTYPE_I8 ? MTYPE_I4 : MTYPE_U4);
    tn = Build_TN_Of_Mtype(mtype);
    Add_TN_Pair (tn, Build_TN_Of_Mtype(mtype));
  } else {
    tn = Build_TN_Of_Mtype (mtype);
  }
#else
  tn = Build_TN_Of_Mtype (mtype);
#endif

  TN_MAP_Set(TN_To_PREG_Map, tn, (void *)preg_num);

  if (Is_Predicate_REGISTER_CLASS(TN_register_class(tn))) {

    // When we create a predicate TN, we actually need to create
    // a pair. The "true" TN corresponds to preg_num; the "false"
    // TN corresponds to preg_num+1.
    Is_True(!TN_is_gra_homeable(tn) && !TN_is_rematerializable(tn),
	  ("don't support homeable or rematerializable predicate preg"));
    PREG_NUM preg2_num = preg_num + 1;
    TN *tn2 = Build_TN_Of_Mtype (mtype);
    TN_MAP_Set(TN_To_PREG_Map, tn2, (void *)preg2_num);
    PREG_To_TN_Array[preg2_num] = tn2;
    PREG_To_TN_Mtype[preg2_num] = mtype;
  }

  PREG_To_TN_Array[preg_num] = tn;
  PREG_To_TN_Mtype[preg_num] = mtype;

  return tn;
}

#ifdef TARG_ST
struct wn_home_hilo {
  WN *hi;
  WN *lo;
};

static WN_MAP WN_To_Hilo_map = WN_MAP_UNDEFINED;

static WN *
Get_hilo_home(WN *home, WN *wn)
{
  struct wn_home_hilo *hilo;

  PARITY par = WN_parity(wn);
  if (WN_To_Hilo_map == WN_MAP_UNDEFINED)
    WN_To_Hilo_map = WN_MAP_Create(&MEM_phase_pool);

  hilo = (struct wn_home_hilo *)WN_MAP_Get(WN_To_Hilo_map, home);
  if (! hilo) {
    hilo = (struct wn_home_hilo *)malloc (sizeof (struct wn_home_hilo));

    HILO_lower_wn (home, &hilo->hi, &hilo->lo);
    WN_MAP_Set(WN_To_Hilo_map, home, (void*)hilo);
  }

  if (par == PARITY_DOUBLE_HI || par == PARITY_LONGLONG_HI)
    return hilo->hi;
  if (par == PARITY_DOUBLE_LO || par == PARITY_LONGLONG_LO)
    return hilo->lo;

  FmtAssert (FALSE, ("Get_hilo_home: unknown or no parity"));
}
#endif

/* ====================================================================
 *   PREG_To_TN 
 *
 *   function exported externally for use in LRA. 
 * ====================================================================
 */
TN *
PREG_To_TN (
  ST *preg_st, 
  PREG_NUM preg_num
#ifdef TARG_ST
  , WN *wn
#endif
)
{
  TN *tn;

  Is_True((preg_num <= Get_Preg_Num(PREG_Table_Size(CURRENT_SYMTAB))),
	    ("PREG_To_TN(): preg %d > SYMTAB_last_preg(%d)",
	     preg_num, Get_Preg_Num(PREG_Table_Size(CURRENT_SYMTAB))));

  tn = PREG_To_TN_Array[preg_num];

  if (tn == NULL) {
    ISA_REGISTER_CLASS rclass;
    REGISTER reg;

    if (CGTARG_Preg_Register_And_Class(preg_num, &rclass, &reg)) {
#ifdef TARG_ST
      //TB: Add a check that the rlass is compatible with preg_st
#define DEFAULT_RCLASS_SIZE(rc)	\
	((REGISTER_bit_size(rc, REGISTER_CLASS_last_register(rc))+7)/8)
      if (ST_size(preg_st)!=0 && ST_size(preg_st) != DEFAULT_RCLASS_SIZE(rclass))
	{
#ifdef TARG_ST
	  // [TTh] Check size compatibility with register class
	  if (!ISA_OPERAND_Exist_With_Register_Class_Bitsize(rclass, ST_size(preg_st)*8)) {
#endif
	    unsigned line;
	    USRCPOS usrcpos;
	    USRCPOS_srcpos(usrcpos) = WN_Get_Linenum(wn);
	    line = USRCPOS_linenum(usrcpos);
	    ErrMsgSrcpos(EC_CG_Generic_Fatal, USRCPOS_srcpos(usrcpos), "type mismatch");
#ifdef TARG_ST
	  }
#endif
	}
#endif
#ifndef TARG_ST
      //[TB] This restriction seems to be unadapted to our cores.
      Is_True(!Is_Predicate_REGISTER_CLASS(rclass),
		           ("don't support dedicate predicate pregs"));
#endif
#ifdef HAS_STACKED_REGISTERS
	if (ABI_PROPERTY_Is_stacked(
		rclass,
		REGISTER_machine_id(rclass, reg) )) 
	{
		reg = REGISTER_Allocate_Stacked_Register(
			(Is_Int_Output_Preg(preg_num) ? ABI_PROPERTY_caller 
						      : ABI_PROPERTY_callee),
			rclass, reg);

		if (PU_has_syscall_linkage(Get_Current_PU())
			&& ! Is_Int_Output_Preg(preg_num)) 
		{
			// syscall linkage means the input parameters
			// are preserved in the PU, so can restart.  
			// So mark the stacked register such that it
			// won't be available for future allocation
			// (i.e. it can be used if in whirl, but LRA
			// won't ever allocate it).
			// Note that we are assuming here that the only
			// stacked pregs whirl2ops will see will be either
			// input or output parameters.  If this is not true,
			// then instead will need to iterate thru params.
			REGISTER_Unallocate_Stacked_Register (rclass, reg);
		}
	}
#endif
#ifdef ENABLE_64_BITS
      // [CG]: Currently we don't want to handle dedicated TNs of 
      // different sizes, thus we don't specify the size
      tn = Build_Dedicated_TN(rclass, reg, 0);
#else
      tn = Build_Dedicated_TN(rclass, reg, ST_size(preg_st));
#endif
#ifdef EMULATE_LONGLONG
      // only on IA-32
      if (reg == First_Int_Preg_Return_Offset) {
	// dedicated and eax
	// map another TN for edx, its longlong pair,
	// for when this tn is used in longlong situations
	// such as longlong return values from functions
	if (CGTARG_Preg_Register_And_Class(Last_Int_Preg_Return_Offset,
                                             &rclass, &reg)) {
	  TN *pair = Build_Dedicated_TN(rclass, reg, ST_size(preg_st));
            
	  Add_TN_Pair (tn, pair);
	} else {
          #pragma mips_frequency_hint NEVER
	  FmtAssert (FALSE,
               ("Could not find reg for Last_Int_Preg_Return_Offset"));
	  /*NOTREACHED*/
	}
      }
#endif
      PREG_To_TN_Array[preg_num] = tn;
      PREG_To_TN_Mtype[preg_num] = TY_mtype(ST_type(preg_st));
    }
    else {
      /* create a TN for this PREG. */
      TYPE_ID mtype = TY_mtype(ST_type(preg_st));
      tn = Set_TN_For_PREG (preg_num, mtype);
    }

    if (CGSPILL_Rematerialize_Constants) {
      BOOL gra_homeable = FALSE;
      WN *home= Preg_Is_Rematerializable(preg_num, &gra_homeable);

      if (home) {
#ifdef TARG_ST
        //
        // Christian: 'home' is set by WOPT.
        //            it may happen that 'home' is a 64-bit
        //            thing, which we can't handle on a 32-bit
        //            machine.
#ifdef ENABLE_64_BITS
        if (!Enable_64_Bits_Ops &&
            (MTYPE_is_double(WN_rtype(home)) || 
             MTYPE_is_longlong(WN_rtype(home)))) {
          home = Get_hilo_home (home, wn);
        }
#else
        if (Only_32_Bit_Ops &&
            (MTYPE_is_double(WN_rtype(home)) || 
             MTYPE_is_longlong(WN_rtype(home)))) {
          home = Get_hilo_home (home, wn);
        }
#endif
#endif

	if (gra_homeable) {
	  if (TN_number(tn) < GRA_non_home_lo ||
	      TN_number(tn) > GRA_non_home_hi) {
	    Set_TN_is_gra_homeable(tn);
	    Set_TN_home (tn, home);
	  }
	} else {
	  Set_TN_is_rematerializable(tn);
	  Set_TN_home (tn, home);
	}
      }
    }

    if (Get_Trace (TP_CGEXP, 16)) {
      fprintf(TFile, "preg %d maps to tn %d", preg_num, TN_number(tn));
      if (TN_is_gra_homeable(tn)) {
	fprintf(TFile, "(gra_homeable)\n");
      } else if (TN_is_rematerializable(tn)) {
	fprintf(TFile, "(rematerializable)\n");
      } else {
	fprintf(TFile, "\n");
      }
    }

  }

  if (TN_is_dedicated(tn)) {
    dedicated_seen = TRUE;
    // For dedicated FP registers, it is important that we use
    // a TN of the right size. So we create a new one if the
    // size of tn does not match the size of preg_st.
    if (TN_is_float(tn) && TN_size(tn) != ST_size(preg_st)) {
      tn = Build_Dedicated_TN (TN_register_class(tn),
                               TN_register(tn),
                               ST_size(preg_st));
    }
  }

  return tn;
}

/* ====================================================================
 *   TN_To_Assigned_PREG
 *
 *   Return the physical PREG assigned to the <tn>. 
 * ====================================================================
 */
PREG_NUM
TN_To_Assigned_PREG (
  TN *tn
)
{
  PREG_NUM i;

  FmtAssert (TN_register(tn) != REGISTER_UNDEFINED, 
    ("TN_To_Assigned_PREG: no assigned register for TN%d", TN_number(tn)));
#ifdef TARG_ST
  // TB: New way of mapping machine id to preg num.
  // work for core and extension register classes.
  i = REGISTER_machine_id(TN_register_class(tn), TN_register(tn));
  i += CGTARG_Regclass_Preg_Min(TN_register_class(tn));
#else
  // TB: New way of mapping machine id to preg num 
  i = REGISTER_machine_id(TN_register_class(tn), TN_register(tn));
  if (TN_is_float(tn)) {
    i += Float_Preg_Min_Offset;
  }
#if defined(TARG_IA32) || defined(TARG_ST)
  // There's no ZERO register: eax has machine_id 0, but preg_id 1
  // TARG_ST200: have zero reg
  else {
    i += Int_Preg_Min_Offset;
  }
#endif  
#endif
  return i;
}


/* See if there is a PREG corresponding to a TN. If there is, return the 
 * preg number, otherwise return 0. 
 * NOTE: This is currently very slow for non-dedicated TNs, use with care.
 */
PREG_NUM
TN_To_PREG (TN *tn)
{
  PREG_NUM i;

  if (TN_is_dedicated(tn)) {
    return TN_To_Assigned_PREG (tn);
  }

  for (i = Last_Dedicated_Preg_Offset; 
       i < Get_Preg_Num(PREG_Table_Size(CURRENT_SYMTAB));
       i++) 
  {
    if (PREG_To_TN_Array[i] == tn) return i;
  }
  return 0;
}

/* =======================================================================
 *
 *  PREG_To_TN_Clear 
 *
 *  Call this when starting a new REGION, so that 
 *  the TNs in the new REGION will be distinct from the
 *  TNs in all previously compiled REGIONs.
 *
 * =======================================================================
 */
void
PREG_To_TN_Clear (void)
{
  PREG_NUM i;

  for (i = Last_Dedicated_Preg_Offset + 1; 
       i <= Get_Preg_Num(PREG_Table_Size(CURRENT_SYMTAB));
       i++) 
  {
    PREG_To_TN_Array[i] = NULL;
    PREG_To_TN_Mtype[i] = (TYPE_ID)0;
  }
  max_preg_to_tn_index = 0;
}

/* =======================================================================
 *
 *  TN_LIST_From_PREG_LIST
 *
 *  Use PREG_To_TN_Array to convert a PREG_LIST to a TN_LIST.
 *
 * =======================================================================
 */
static TN_LIST *
TN_LIST_From_PREG_LIST( PREG_LIST *prl0, MEM_POOL *pool )
{
  PREG_LIST *prl;
  PREG_NUM pr;
  TN_LIST *tnl = NULL;
  TN *tn;

  for ( prl = prl0; prl; prl = PREG_LIST_rest( prl ) ) {
    pr = PREG_LIST_first( prl );
    if (pr > Last_Dedicated_Preg_Offset) {
      tn = PREG_To_TN_Array[pr];
      Is_True(tn != NULL,
	    ("TN_LIST_From_PREG_LIST, NULL TN in PREG to TN map, PREG%d",pr));
      tnl = TN_LIST_Push( tn, tnl, pool );
    }
  }

  return tnl;
}

/* =======================================================================
 *   Add_PregTNs_To_BB
 *
 * add list of TOP_pregtn to bb, so tn renaming and splitting keeps
 * track of the preg associated with the tn 
 * =======================================================================
 */
static void
Add_PregTNs_To_BB (PREG_LIST *prl0, BB *bb, BOOL prepend)
{
  PREG_LIST *prl;
  PREG_NUM pr;
  TN *tn;
  TOP topcode = (prepend ? TOP_begin_pregtn : TOP_end_pregtn);
  OPS ops;
  OPS_Init(&ops);

  for ( prl = prl0; prl; prl = PREG_LIST_rest( prl ) ) {
    pr = PREG_LIST_first( prl );
    if (pr > Last_Dedicated_Preg_Offset) {
      tn = PREG_To_TN_Array[pr];
      if (tn == NULL) {
  	if (Trace_REGION_Interface)
	  fprintf(TFile, "<region> remove preg %d from rid list\n", pr);
	REGION_remove_preg(BB_rid(bb), pr, !prepend);
      } else {
      	Build_OP (topcode, tn, Gen_Literal_TN(pr, 4), &ops);
      	Set_OP_glue(OPS_last(&ops));
      }
    }
  }
  if ( Trace_REGION_Interface ) {
    fprintf( TFile, "<region> add pregtns to bb %d:\n", BB_id(bb)  );
    Print_OPS(&ops);
  }
  /*
  if (prepend)
    CGSPILL_Prepend_Ops (bb, &ops);
  else
    CGSPILL_Append_Ops (bb, &ops);
  */
}


/* Check if the parent WHIRL node can take an immediate operand
 * of the value const_val. If parent is NULL, return FALSE.
 */
static BOOL
Has_Immediate_Operand (WN *parent, WN *expr)
{
  Is_True( WN_operator_is(expr, OPR_INTCONST),
    ("Has_Immediate_Operand: Not a constant") );

  /* If parent WHIRL node is not given, the constant 
   * cannot be an immediate operand. 
   */
  if (parent == NULL) return FALSE;

  // check for target-specific cases where immediate operand is okay.
  if (Target_Has_Immediate_Operand (parent, expr)) {
	return TRUE;
  }

  /* Check to make sure that the immediate operand is the second
   * operand of the parent
   */
  if (WN_kid_count(parent) < 2 || WN_kid1(parent) != expr) return FALSE;
  /* 
   * ISTORE has 2 kids, but 2nd kid is address not an offset.
   * TODO:  check for any other such cases?
   * This kind of info should probably be somehow kept as an opcode property.
   */
  if (OPCODE_is_store(WN_opcode(parent))) return FALSE;

  /* Can_Be_Immediate on TOP is not what we want here,
   * because some opcodes (e.g. MPY) do not take any immediate,
   * yet we want to pass immediate for later optimized expansion.
   * So instead we need a separate list of those opcodes for which
   * we are prepared to handle immediates.
   */
  if (MTYPE_is_float(WN_rtype(parent)))
	return FALSE;

  switch (WN_operator(parent)) {
  case OPR_ADD:
  case OPR_SUB:
  case OPR_MPY:
  case OPR_DIV:
  case OPR_REM:
  case OPR_DIVREM:
  case OPR_SHL:
  case OPR_LSHR:
  case OPR_ASHR:
  case OPR_BAND:
  case OPR_BIOR:
  case OPR_BXOR:
  case OPR_EQ:
  case OPR_NE:
  case OPR_LE:
  case OPR_LT:
  case OPR_GE:
  case OPR_GT:
	return TRUE;
  }
  return FALSE;
}

/* ====================================================================
 *   Handle_Call_Site
 *
 *   Handle the misc. stuff that needs to be done for OPR_CALL, OPR_PICCALL,
 *   and OPR_ICALL nodes. This includes:
 *
 *    1. generate code for the actual arguments
 *    2. mark the current PU and bb as having a call.
 *    3. mark the current EH range as having a call.
 * ====================================================================
 */
static void
Handle_Call_Site (
  WN *call, 
  OPERATOR call_opr
)
{
  TN          *tgt_tn;
  ST          *call_st = (call_opr != OPR_ICALL) ? WN_st(call) : NULL;
  CALLINFO    *call_info;

  /* 
   * wn_lower_call has already expanded the parameters, so don't need
   * to do anything more for parameters at this point.
   */

  /* Note the presence of a call in the current PU and bb. */
  PU_Has_Calls = TRUE;

  /* Generate the call instruction */
  if (call_opr == OPR_CALL) {
    tgt_tn = Gen_Symbol_TN (call_st, 0, 0);
  }
  else {
    /* For PIC calls, force t9 to be the result register. */
    tgt_tn = Gen_PIC_Calls ? Ep_TN : NULL;
    tgt_tn = Expand_Expr (WN_kid(call,WN_kid_count(call)-1), call, tgt_tn);

    /* If call-shared and the call is to a non PREEMPTIBLE symbol,
     * generate a jal instead of a jalr. 
     */
    if (Gen_PIC_Call_Shared && 
	call_st != NULL &&
	!ST_is_preemptible(call_st) )
    {
      tgt_tn = Gen_Symbol_TN (call_st, 0, 0);
      call_opr = OPR_CALL;
    }
  }
  Last_Mem_OP = OPS_last(&New_OPs);
  Exp_Call (call_opr, RA_TN, tgt_tn, &New_OPs);
  Set_OP_To_WN_Map(call);

  call_info = TYPE_PU_ALLOC (CALLINFO);
  CALLINFO_call_st(call_info) = call_st;
  CALLINFO_call_wn(call_info) = call;
  BB_Add_Annotation (Cur_BB, ANNOT_CALLINFO, call_info);

  region_stack_eh_set_has_call();

  /* For now, terminate the basic block. This makes GRA work better since 
   * it has finer granularity. It is also easier for LRA to make the
   * assumption that a procedure call breaks a basic block. 
   */

#ifdef TARG_ST
  // FdF 20041105: A "noreturn" call is also an exit block, so as to
  // enable tail call optimization and optimization of the call
  // sequence.
  if (WN_Call_Never_Return( call ) && !PU_has_region(Get_Current_PU())) {
    EXITINFO *exit_info = TYPE_PU_ALLOC (EXITINFO);
    EXITINFO_srcpos(exit_info) = current_srcpos;
    BB_Add_Annotation (Cur_BB, ANNOT_EXITINFO, exit_info);
    Set_BB_exit(Cur_BB);
  }
#endif

  Start_New_Basic_Block ();

  // if caller-save-gp and not defined in own dso, then restore gp.
  // if call_st == null, then indirect call, and assume external.
  if (Gen_GP_Relative && Is_Caller_Save_GP && !Constant_GP
	&& (call_st == NULL || ST_export(call_st) == EXPORT_PREEMPTIBLE))
  {
    // restore old gp
    // assume that okay to restore gp before return val of call
    TN *caller_gp_tn = PREG_To_TN_Array[Caller_GP_Preg];
    if (caller_gp_tn == NULL) {
      caller_gp_tn = CGTARG_Gen_GP_TN ();
      TN_MAP_Set(TN_To_PREG_Map, caller_gp_tn, (void *)Caller_GP_Preg);
      PREG_To_TN_Array[Caller_GP_Preg] = caller_gp_tn;
      PREG_To_TN_Mtype[Caller_GP_Preg] = TY_mtype(Spill_Ptr_Type);
    }
    Exp_COPY (GP_TN, caller_gp_tn, &New_OPs);
  }

  return;
}

/* ====================================================================
 *   Memop_Variant
 *
 *   Determine the Exp_OP variant for a memory operation.
 * ====================================================================
 */
static VARIANT 
Memop_Variant (
  WN *memop
)
{
  VARIANT variant = V_NONE;
#ifdef TARG_ST
  INT     required_alignment = MTYPE_alignment(WN_desc(memop));
#else
  INT     required_alignment = MTYPE_RegisterSize(WN_desc(memop));
#endif

  /* If volatile, set the flag.
   */
  if (WN_Is_Volatile_Mem(memop)) Set_V_volatile(variant);

  /* Determine the alignment related variants. We have to check both 
   * ty alignment and the offset alignment.
   */
  Is_True ((required_alignment != 0), ("bad value 0 for required_alignment"));
 
  if (required_alignment > 1) {
    WN_OFFSET offset;
    INT ty_align;
    INT align;

    switch (WN_operator(memop)) {
    case OPR_LDID:
    case OPR_LDBITS:
    case OPR_STID:
    case OPR_STBITS:
      /* store_offset and load_offset are the same, so we share the code
       */
      offset = WN_load_offset(memop);
      ty_align = ST_alignment(WN_st(memop));
      break;
    case OPR_ILOAD:
    case OPR_ILDBITS:
      {
	TY_IDX ty = WN_load_addr_ty(memop);
	if (TY_kind(ty) == KIND_POINTER) ty = TY_pointed(ty);
	ty_align = TY_align(ty);
	offset = WN_load_offset(memop);
      }
      break;
    case OPR_ISTORE:
    case OPR_ISTBITS:
      {
	TY_IDX ty = WN_ty(memop);
	if (TY_kind(ty) == KIND_POINTER) ty = TY_pointed(ty);
	ty_align = TY_align(ty);
	offset = WN_store_offset(memop);
      }
      break;
    default:
      FmtAssert(FALSE, ("unhandled memop in Memop_Variant"));
      /*NOTREACHED*/
    }

    align = ty_align;
    if (offset) {
#ifdef TARG_ST
      // [TTh] Do not use directly the offset as an alignment,
      //       as it is not always a power of 2.
      INT offset_align = 0;
      // Fix for bug #23667. Offset can be negative, hence offset_abs too.
      // But negative values are meaningless for alignment.
      INT offset_abs = (offset < 0? -offset: offset) % required_alignment;
      if (offset_abs) {
	offset_align = 1;
	while (!(offset_abs & 0x1)) {
	  offset_align <<= 1;
	  offset_abs   >>= 1;
	}
      }
#else
      INT offset_align = offset % required_alignment;
#endif
      if (offset_align) align = MIN(ty_align, offset_align);
    }

    if (align < required_alignment) {
      Set_V_alignment(variant, align);
     /*
      *	TODO
      *	When we have ST information we may be able to compute an
      *	offset(say mod 16) that will give us further information
      */
      Set_V_align_offset_unknown(variant);

      /* We have an unaligned volatile. What to do??
       * Suneel/bean want the object to ignore the atomicity of volatile
       * and generate the unaligned references.
       * This will at least get the users code working
       */
      if (V_volatile(variant)) {
	ErrMsgSrcpos(EC_Ill_Align,
		     current_srcpos,
		     align,
		     "reference to unaligned volatile:  volatile atomicity is ignored");
      } else if (Warn_Misaligned_Access >= 1) {
	ErrMsgSrcpos(EC_Ill_Align,
		     current_srcpos,
		     align,
		     "unaligned memory access");
      }
      if (Warn_Misaligned_Access >= 2) {
	ErrMsgSrcpos(EC_CG_Generic_Fatal, current_srcpos, 
		     "fatal error due to misaligned access");
      }
    }
#ifdef TARG_ST
    else if (align > required_alignment) {
      // [TTh] Create an alignment variant in case of 'overaligned'
      // access, to benefit from this information at code selection.
      Set_V_overalign(variant);
      Set_V_alignment(variant, align);
      Set_V_align_offset_unknown(variant); 
    }
#endif
  }

  /* Now get prefetch flags if any
   */
  WN *pf_wn = NULL;
  PF_POINTER *pf_ptr = (PF_POINTER *) WN_MAP_Get(WN_MAP_PREFETCH, memop);
  if (pf_ptr) {
    pf_wn = PF_PTR_wn_pref_2L(pf_ptr);
    if (pf_wn == NULL) pf_wn = PF_PTR_wn_pref_1L(pf_ptr);
  }
  if (pf_wn) Set_V_pf_flags(variant, WN_prefetch_flag(pf_wn));

  return variant;
}

/* ====================================================================
 *   Handle_LDA
 * ====================================================================
 */
static TN *
Handle_LDA (
  WN *lda, 
  WN *parent, 
  TN *result, 
  OPCODE opcode
)
{
  OPERATOR call_op = OPERATOR_UNKNOWN;
  /* check if the LDA is for a procedure call. */
  if (parent != NULL) call_op = WN_operator(parent);

  if (result == NULL) {
    result = Allocate_Result_TN (lda, NULL);
    if (CGSPILL_Rematerialize_Constants
	&& CGTARG_Address_Constants_Are_Rematerializable_p) {
      Set_TN_is_rematerializable(result);
      Set_TN_remat (result, lda);
    }
  }

#ifdef TARG_ST
  TN *tmp_tn=NULL;
  if (TN_is_dedicated(result)) {
    tmp_tn=result;
    result=Build_TN_Like(tmp_tn);
  }
#endif

  Last_Mem_OP = OPS_last(&New_OPs);
  Exp_Lda (
      OPCODE_rtype(opcode),
      result,
      WN_st(lda), 
      WN_lda_offset(lda),
      call_op,
      &New_OPs);
  Set_OP_To_WN_Map(lda);
  
#ifdef TARG_ST
  if(tmp_tn != NULL)
    Exp_COPY (tmp_tn,result, &New_OPs);
#endif


  return result;
}

/* ====================================================================
 *   find_dreg_preg
 *
 *   return preg for corresponding ST via dreg table
 * ====================================================================
 */
struct find_dreg_preg {
	ST_IDX st;
	find_dreg_preg (const ST *s) : st (ST_st_idx (s)) {}

	BOOL operator () (UINT, const ST_ATTR *st_attr) const {
	    return (ST_ATTR_kind (*st_attr) == ST_ATTR_DEDICATED_REGISTER &&
		    ST_ATTR_st_idx (*st_attr) == st);
    	}
};

/* ====================================================================
 *   Find_PREG_For_Symbol
 * ====================================================================
 */
PREG_NUM
Find_PREG_For_Symbol (
  const ST *st
)
{
    ST_IDX idx = ST_st_idx (st);
    ST_ATTR_IDX d;

    d = For_all_until (St_Attr_Table, ST_IDX_level (idx),
                          find_dreg_preg(st));
    return ST_ATTR_reg_id(St_Attr_Table(ST_IDX_level (idx), d));
} 

#ifdef ENABLE_64_BITS
#define MTYPE_bsize(type) ((MTYPE_bit_size(type)+7)/8)
extern void Expand_Multi(TN *tgt_tn, TN *src_tn, OPS *ops);
extern void Expand_Extract(TN *low_tn, TN *high_tn, TN *src_tn, OPS *ops);
extern void Expand_Compose(TN *tgt_tn, TN *low_tn, TN *high_tn, OPS *ops);
#endif

/* ====================================================================
 *   Handle_LDID
 * ====================================================================
 */
static TN *
Handle_LDID (
  WN *ldid, 
  TN *result, 
  OPCODE opcode
)
{
  if (ST_assigned_to_dedicated_preg(WN_st(ldid))) {
    // replace st with dedicated preg
    WN_offset(ldid) = Find_PREG_For_Symbol(WN_st(ldid));
    WN_st_idx(ldid) = ST_st_idx(MTYPE_To_PREG(ST_mtype(WN_st(ldid))));
  }

  /* Check if we have an LDID of a PREG. Just return the TN corresponding
   * to the PREG. If there is a result TN, generate a copy.
   */
  if (WN_class(ldid) == CLASS_PREG) {

#ifdef TARG_ST
    TN *ldid_result;
    INTRINSIC_RESULT *res;

    /*
     * [YJ]: For intrinsic call with dynamic mtype, we don't have lowered yet 
     * negative PREG_NUM (see what happens in wn_lower.cxx).
     * It is time now to deal with this special case.
     * Notice that this mechanism could be further extended to
     * "normal intrinsic".
     */
    if(WN_load_offset(ldid)<0) { 
        FmtAssert(MTYPE_is_dynamic(WN_rtype(ldid)),("Not a dynamic type"));

     /*
      * For multiple result intrinsics, Handle_SUBPART
      * routine should have caught the following case. 
      * Hence the assertion.
      *             LDID
      *           SUBPART
      *         STID
      *             LDID
      *           SUBPART
      *         STID
      */
        res = get_last_intrinsic_result();
        FmtAssert(res->numres==1,("Unexpected multiple results builtin"));

        ldid_result = res->res[0];
      } else {
      /* Managing the normal case */
        ldid_result = PREG_To_TN (WN_st(ldid), WN_load_offset(ldid), ldid);
      }
#else
    TN *ldid_result = PREG_To_TN (WN_st(ldid), WN_load_offset(ldid));
#endif
    // TB 02 2006: FIX bug pro-release-1-9-0-B/39: DO not create ops for node such as
    // U4U4LDID 76 <1,4,.preg_U4> T<8,.predef_U4,4> # <preg>
    // U4STID 76 <1,4,.preg_U4> T<8,.predef_U4,4> # <preg> {freq: 0, ln: 110, col: 0}
    // Otherwise freq.cxx module will not finished (function Is_Pointer)
    if (ldid_result == result)
      return result;

#ifdef ENABLE_64_BITS
    FmtAssert(result == NULL || TN_size(result) == MTYPE_bsize(OPCODE_rtype(opcode)), ("Unexpected result size for LDID"));

    if (MTYPE_bsize(OPCODE_rtype(opcode)) > TN_size(ldid_result)) {
      if (result == NULL) result = Allocate_Result_TN (ldid, NULL);
      Expand_Multi(result, ldid_result, &New_OPs);
    } else 
#endif


    if (result == NULL) {
      result = ldid_result;
    }
    else {
#ifdef EMULATE_LONGLONG
      {
#ifdef TARG_ST
	// Arthur:
	// Expand_Copy() should not be seen here !!
	// should be able to handle everything in Exp_COPY !!
	// But I feel something should be done about the
	// EMULATE_LONGLONG ??
	//
        TYPE_ID mtype =  ST_mtype(WN_st(ldid));
	Exp_COPY (result, ldid_result, &New_OPs);
#else
        extern void
          Expand_Copy (TN *result, TN *src, TYPE_ID mtype, OPS *ops);
        TYPE_ID mtype =  ST_mtype(WN_st(ldid));
        if (mtype == MTYPE_I8 || mtype == MTYPE_U8) {
	  Expand_Copy (result, ldid_result, mtype, &New_OPs);
        } else {
          Exp_COPY (result, ldid_result, &New_OPs);
        }
#endif /* TARG_ST */
      }
#else
      Exp_COPY (result, ldid_result, &New_OPs);
#endif
    }

    return result;
  } 

  // generate a load:
  VARIANT variant;

  if (opcode == OPC_U4U8LDID) {
    opcode =	OPC_U4U4LDID;
    WN_set_opcode(ldid, opcode);
    if (Target_Byte_Sex == BIG_ENDIAN) {
      WN_offset(ldid) += 4;	// get low-order word
    }
  }
  else if (opcode == OPC_I4I8LDID) {
    opcode = OPC_I4I4LDID;
    WN_set_opcode(ldid, opcode);
    if (Target_Byte_Sex == BIG_ENDIAN) {
      WN_offset(ldid) += 4;	// get low-order word
    }
  }
  variant = Memop_Variant(ldid);

  if (result == NULL) 
    result = Allocate_Result_TN (ldid, NULL);

  Last_Mem_OP = OPS_last(&New_OPs);
  Exp_Load (OPCODE_rtype(opcode), OPCODE_desc(opcode),
	     result, 
	     WN_st(ldid), 
	     WN_load_offset(ldid), 
	     &New_OPs, 
	     variant);

  Set_OP_To_WN_Map(ldid);
  return result;
}

#ifdef TARG_ST
/* ====================================================================
 *   Handle_SUBPART
 *
 *   SUBPART operator has been created so as to get back
 *   results of a multiple result intrinsic call.
 * ====================================================================
 */
static TN *
Handle_SUBPART (
  WN *subpart, 
  TN *result, 
  OPCODE opcode
)
{
   WN               *kid0; 
   WN_OFFSET         subpart_idx;
   INTRINSIC_RESULT *res;

   kid0        = WN_kid0(subpart);
   subpart_idx = WN_subpart_index(subpart);

   // Get result TNs of last intrinsic call.
   res         = get_last_intrinsic_result();

   // Check that index of result is OK.
   // Check that kid is indeed an LDID with a negative PREG
   FmtAssert(subpart_idx>=0 && subpart_idx<res->numres,
             ("Internal error in intrinsic representation"));
   FmtAssert(WN_operator_is(kid0,OPR_LDID) && WN_load_offset(kid0)<0,
             ("Internal error in intrinsic representation"));

   if(NULL==result)
     result=res->res[subpart_idx];

   // Similarly to what has been done in Handle_LDID, we
   // avoid to create a copy when both result and 
   // operand are identical. See TB's comments on that point.
   if(result!=res->res[subpart_idx])
      Exp_COPY (result, res->res[subpart_idx], &New_OPs);

   return result;
}
#endif     /* TARG_ST */

/* ====================================================================
 *   Handle_LDBITS
 * ====================================================================
 */
static TN *
Handle_LDBITS (
  WN *ldbits, 
  TN *result, 
  OPCODE opcode
)
{
  TN *src_tn;
  UINT bit_ofst;

  if (result == NULL) result = Allocate_Result_TN (ldbits, NULL);

  if (WN_class(ldbits) == CLASS_PREG)
  { /* LDBITS of a PREG */
#ifdef TARG_ST
    src_tn = PREG_To_TN (WN_st(ldbits), WN_load_offset(ldbits), ldbits);
#else
    src_tn = PREG_To_TN (WN_st(ldbits), WN_load_offset(ldbits));
#endif
  } 
  else
  {
    VARIANT variant = Memop_Variant(ldbits);

    src_tn = Allocate_Result_TN (ldbits, NULL);
    Last_Mem_OP = OPS_last(&New_OPs);
    Exp_Load (OPCODE_rtype(opcode), OPCODE_desc(opcode),
	src_tn, 
	WN_st(ldbits), 
	WN_load_offset(ldbits), 
	&New_OPs, 
	variant);
    Set_OP_To_WN_Map(ldbits);
  }

  if (Target_Byte_Sex == LITTLE_ENDIAN) {
      // for LX as LITTLE_ENDIAN:
      bit_ofst = WN_bit_offset(ldbits);
  } else {
      // for LX as BIG_ENDIAN:
      bit_ofst = MTYPE_bit_size(WN_desc(ldbits)) - WN_bit_size(ldbits) - WN_bit_offset(ldbits);
  }
  Exp_Extract_Bits(WN_rtype(ldbits), WN_desc(ldbits), bit_ofst, 
		   WN_bit_size(ldbits), result, src_tn, &New_OPs);
  return result;
}

/* ====================================================================
 *   Handle_EXTRACT_BITS
 * ====================================================================
 */
static TN *
Handle_EXTRACT_BITS (
  WN *extrbits, 
  TN *result, 
  OPCODE opcode
)
{
  TN *kid0_tn = Expand_Expr (WN_kid0(extrbits), extrbits, NULL);
  if (result == NULL) result = Allocate_Result_TN (extrbits, NULL);

  Exp_Extract_Bits(OPCODE_rtype(opcode), OPCODE_rtype(opcode), 
		   WN_bit_offset(extrbits), WN_bit_size(extrbits), 
		   result, kid0_tn, &New_OPs);
  return result;
}

typedef struct TN_CORRESPOND
{
  TN			*result;
  TN			*result2;
  struct TN_CORRESPOND	*next;
} TN_CORRESPOND, *TN_CORRESPONDp;

static TN_CORRESPONDp tn_correspond_list= NULL;


TN *TN_CORRESPOND_Lookup(TN *result)
{
  TN_CORRESPONDp p;

  for(p= tn_correspond_list; p; p= p->next)
  {
    if (p->result == result)
    {
      return p->result2;
    }
  }
  return NULL;
}

void TN_CORRESPOND_Free(void)
{
  TN_CORRESPONDp next;
  TN_CORRESPONDp p = tn_correspond_list;

  while(p)
  {
    next =	p->next;
    MEM_POOL_FREE(Malloc_Mem_Pool, p);
    p =		next;
  }
  tn_correspond_list= NULL;
}

static TN *
TN_CORRESPOND_Set(TN *result, WN *expr)
{
  TN_CORRESPONDp p;

  Is_True((TN_CORRESPOND_Lookup(result)==NULL),("TN_CORRESPOND_Set(): expected null"));

  p = TYPE_MEM_POOL_ALLOC(TN_CORRESPOND, Malloc_Mem_Pool);
  p->result =		result;
  p->result2= 		Allocate_Result_TN(expr, NULL);
  p->next = 		tn_correspond_list;

  tn_correspond_list=	p;

  return p->result2;
}

/* ====================================================================
 *   TN_CORRESPOND_Get
 * ====================================================================
 */
TN *
TN_CORRESPOND_Get (
  TN *result, 
  WN *expr
)
{
  TN *tn = TN_CORRESPOND_Lookup(result);

  if (tn)
    return tn;

  return TN_CORRESPOND_Set(result, expr);
}

/* ====================================================================
 *   U4ExprHasUpperBitZero
 *
 *   Determine if the upper bit:31 is zero
 * ====================================================================
 */
static BOOL U4ExprHasUpperBitZero(WN *wn)
{
  switch(WN_opcode(wn)) {

  case OPC_U4LSHR:
   /*
    *  if we shift by a non zero amount, the expression sign bit will be zero
    */
    if (WN_operator_is(WN_kid1(wn), OPR_INTCONST))
    {
      if (0 < WN_const_val(WN_kid1(wn)))
	return TRUE;
    }
    break;

  case OPC_U4BAND:
   /*
    *  if the constant sign bit is zero, the expression sign bit will be zero
    */
    if (WN_operator_is(WN_kid1(wn), OPR_INTCONST))
    {
      if ((WN_const_val(WN_kid1(wn)) & 0x80000000) == 0)
	return TRUE;
    }
    break;
  }

  return FALSE;
}

/* ======================================================================
 *   Is_CVT_Noop
 *
 *   Some CVTs are noops on a given architecture. This should really
 *   have two parts: target-independent and target-dependent. 
 *   WHIRL processing being target-independent can't entirely avoid 
 *   generating these.
 * ======================================================================
 */
static BOOL
Is_CVT_Noop (
  WN *cvt, 
  WN *parent
)
{
  /*
   * All int to int sign conversions are noops in C.
   */
  if (WN_opcode(cvt) == OPC_I4U4CVT || WN_opcode(cvt) == OPC_U4I4CVT ||
      WN_opcode(cvt) == OPC_I5U5CVT || WN_opcode(cvt) == OPC_U5I5CVT ||
      WN_opcode(cvt) == OPC_I8U8CVT || WN_opcode(cvt) == OPC_U8I8CVT) {
	// normally this is removed before cg, but sometimes not
	return TRUE;
  }

  if (Enable_CVT_Opt) {

    switch(WN_opcode(cvt)) {

#ifdef TARG_IA64
    case OPC_F8F4CVT:
    case OPC_F4F8CVT:
	if (WN_operator(parent) == OPR_TAS) {
		/* for IA-64, the tas (getf) does the size conversion too,
		 * so don't need the cvt. */
		return TRUE;
	}
	break;
#endif
    case OPC_U8I4CVT:
    case OPC_I8I4CVT:
     /*
      *  if 32-bit ints are sign-extended to 64-bit, then is a nop.
      */
#ifdef TARG_ST
      // [CG] Not a noop in 64 bits mode
      if (!Enable_64_Bits_Ops)
#endif
      if (!Split_64_Bit_Int_Ops && !Only_Unsigned_64_Bit_Ops)
      {
	return TRUE;
      }
      break;

    case OPC_U5I4CVT:
    case OPC_I5I4CVT:
      /*
       *  Arthur:
       *
       *  On ST100 the Split_64_Bit_Int_Ops MUST be TRUE, so that 32-bit 
       *  values are properly sign-extended in 40-bit registers.
       *  This may seem as an ugly sharing of the functionality -- it
       *  is not. Split_64_Bit_Int_Ops seems to mean that int ops will be
       *  operated upon as 32-bit entities, not 64-bit entities, and the
       *  compiler manipulates WHIRL accordingly. It means that 32-bit
       *  values are operated upon as 32-bit values, not 40-bit values on
       *  the ST100. Now, whether I5I4CVT is a noop is architecture
       *  dependent: on ST100 32-bit values are kept properly sign-extended
       *  in 40-bit registers, so it's a noop; on TI C55x, 40-bit takes 2
       *  registers and it wouldn't be a noop.
       */
      if (Split_64_Bit_Int_Ops || Only_Unsigned_64_Bit_Ops) {
	return TRUE;
      }
      break;

    case OPC_U8U4CVT:
    case OPC_I8U4CVT:
#ifndef EMULATE_LONGLONG
     /*
      *  if we can determine the upper bit:31 is zero, the cast is a nop
      */
#ifdef TARG_ST
      // [CG] Not a noop in 64 bits mode
      if (!Enable_64_Bits_Ops)
#endif
      if (U4ExprHasUpperBitZero(WN_kid0(cvt)))
      {
	return TRUE;
      }
#endif
      break;

    case OPC_I4U8CVT:
    case OPC_I4I5CVT:
    case OPC_I4I8CVT:
    case OPC_U4U8CVT:
    case OPC_U4I5CVT:
    case OPC_U4I8CVT:
    case OPC_I5U8CVT:
    case OPC_I5I8CVT:
    case OPC_U5U8CVT:
    case OPC_U5I8CVT:
     /*
      *  For truncation converts, the memory operation will
      *  perform the necessary truncation.
      */
#ifdef TARG_ST
      // [CG] Not a noop in 64 bits mode
      if (!Enable_64_Bits_Ops)
#endif
      if (parent)
      {
	switch(WN_opcode(parent))
	{
	case OPC_I4STID:
	case OPC_U4STID:
	case OPC_I5STID:
	case OPC_U5STID:
	  if (WN_class(parent) != CLASS_PREG)
	  {
	    return TRUE;
	  }
	  break;

	case OPC_I4ISTORE:
	case OPC_U4ISTORE:
	case OPC_I5ISTORE:
	case OPC_U5ISTORE:
	  return TRUE;
	}
      }
      break;

    case OPC_I4CVTL:
    case OPC_U4CVTL:
    case OPC_I5CVTL:
    case OPC_U5CVTL:
    case OPC_I8CVTL:
    case OPC_U8CVTL:
     /*
      *  For truncation converts, the memory operation will
      *  perform the necessary truncation.
      */
      if (parent)
      {
	switch(WN_operator(parent))
	{
	case OPR_STID:
	  if (MTYPE_size_reg(WN_desc(parent)) ==  WN_cvtl_bits(cvt)
		&& WN_class(parent) != CLASS_PREG)
	  {
	    return TRUE;
	  }
	  break;

	case OPR_ISTORE:
	  if (MTYPE_size_reg(WN_desc(parent)) ==  WN_cvtl_bits(cvt))
	  {
	    return TRUE;
	  }
	}
      }
      break;
    }
  }
  
  return FALSE;
}

/* ======================================================================
 *   Is_CVTL_Opcode
 *
 *   CVTL opcodes are either CVTLs or CVT that change the integer size.
 * ======================================================================
 */
static BOOL
Is_CVTL_Opcode (
  OPCODE opc
)
{
  switch (opc) {
    case OPC_U8I4CVT:
    case OPC_U8U4CVT:
    case OPC_U8I5CVT:
    case OPC_U8U5CVT:

    case OPC_I8I4CVT:
    case OPC_I8U4CVT:
    case OPC_I8I5CVT:
    case OPC_I8U5CVT:

    case OPC_U5I4CVT:
    case OPC_U5U4CVT:
    case OPC_U5I8CVT:
    case OPC_U5U8CVT:

    case OPC_I5I4CVT:
    case OPC_I5U4CVT:
    case OPC_I5I8CVT:
    case OPC_I5U8CVT:
  
    case OPC_I4I8CVT:
    case OPC_I4U8CVT:
    case OPC_I4I5CVT:
    case OPC_I4U5CVT:

    case OPC_U4U8CVT:
    case OPC_U4I8CVT:
    case OPC_U4U5CVT:
    case OPC_U4I5CVT:

    case OPC_I8CVTL:
    case OPC_I5CVTL:
    case OPC_I4CVTL:
    case OPC_U8CVTL:
    case OPC_U5CVTL:
    case OPC_U4CVTL:
      return TRUE;
    default:
      return FALSE;
  }
}

/* ====================================================================
 *   Get_mtype_for_mult
 *
 *   This determines which mtype is really needed for a multiply.
 *   This mimics the Expand_Expr () handling of some OPERATORs. If
 *   you make a change here - look there too.
 * ====================================================================
 */
static TYPE_ID
Get_mtype_for_mult (
  WN *expr,
  WN *parent
)
{
  TYPE_ID mtype;

  switch (WN_operator(expr)) {
    /*
     * MTYPEs of LDID. ILOAD should be derived from WN_desc() rather
     * than WN_rtype 
     * TODO: perhaps it's just a OPERATOR_is_load()
     *       which would include ILDBITS, LDBITS, MLOAD as well ??)
     */
    case OPR_LDID:
    case OPR_ILOAD:
      mtype = WN_desc(expr);
      break;

    case OPR_CVTL:
      if (Is_CVT_Noop(expr, parent)) {
	mtype = Get_mtype_for_mult (WN_kid0(expr), parent);
      }
      else {
	mtype = MTYPE_TransferSize(WN_cvtl_bits(expr)>>3, WN_rtype(expr));
      }
      break;

    case OPR_CVT:
      if (Is_CVT_Noop(expr, parent)) {
	mtype = Get_mtype_for_mult (WN_kid0(expr), parent);
      }
      else {
	mtype = WN_rtype(expr);
      }

    default:
      mtype = WN_rtype(expr);
  }

  return mtype;
}

/* ====================================================================
 *   Expand_Expr_Mult
 *
 *   This determines which mtype is really needed for a multiply.
 *   This mimics the Expand_Expr () handling of some OPERATORs. 
 * ====================================================================
 */
static TN*
Expand_Expr_Mult (
  WN      *expr,
  TYPE_ID *mtype,
  WN *parent
)
{
  TN *kid_tn;

  switch (WN_operator(expr)) {
    /*
     * MTYPEs of LDID. ILOAD should be derived from WN_desc() rather
     * than WN_rtype 
     * TODO: perhaps it's just a OPERATOR_is_load()
     *       which would include ILDBITS, LDBITS, MLOAD as well ??)
     */
    case OPR_LDID:
    case OPR_ILOAD:
      kid_tn = Expand_Expr (expr, parent, NULL);
      *mtype = WN_desc(expr);
      break;

    case OPR_CVTL:
      if (Is_CVT_Noop(expr, parent)) {
	kid_tn = Expand_Expr_Mult (WN_kid0(expr), mtype, parent);
      }
      else {
	kid_tn = Expand_Expr (expr, parent, NULL);
	*mtype = MTYPE_TransferSize(WN_cvtl_bits(expr)>>3, WN_rtype(expr));
      }
      break;

    case OPR_CVT:
      if (Is_CVT_Noop(expr, parent)) {
	kid_tn = Expand_Expr_Mult (WN_kid0(expr), mtype, parent);
      }
      else {
	kid_tn = Expand_Expr (expr, parent, NULL);
	*mtype = WN_rtype(expr);
      }
      break;

    default:
      kid_tn = Expand_Expr (expr, parent, NULL);
      *mtype = WN_rtype(expr);
  }

  return kid_tn;
}

#ifdef TARG_ST

/* ====================================================================
 *   Handle_MPY
 * ====================================================================
 */
static TN *
Handle_MPY (
  WN *expr, 
  TN *result,
  OPCODE opcode
)
{
  OP *Last_OP;
  TN *kid0_tn, *kid1_tn;
  TYPE_ID mtype0, mtype1;

  if (result == NULL) {
    result = Allocate_Result_TN (expr, NULL);
  }

  if (MTYPE_is_float(OPCODE_rtype(opcode))) {
    kid0_tn = Expand_Expr (WN_kid0(expr), expr, NULL);
    kid1_tn = Expand_Expr (WN_kid1(expr), expr, NULL);
    Exp_OP2 (opcode, result, kid0_tn, kid1_tn, &New_OPs);
    return result;
  }

  /* call make MULT with mtypes of arguments: */
  kid0_tn = Expand_Expr_Mult (WN_kid0(expr), &mtype0, expr);
  kid1_tn = Expand_Expr_Mult (WN_kid1(expr), &mtype1, expr);
    
  if (Trace_Exp) {
    #pragma mips_frequency_hint NEVER
    fprintf(TFile, "exp_mul %s: ", OPCODE_name(opcode));
    Print_TN(result, FALSE);
    fprintf(TFile, " (%s) :- ", MTYPE_name(WN_rtype(expr)));
    Print_TN(kid0_tn, FALSE);
    fprintf(TFile, " (%s) ", MTYPE_name(mtype0));
    Print_TN(kid1_tn, FALSE);
    fprintf(TFile, " (%s) \n", MTYPE_name(mtype1));
  }

  /* for debuggging */
  Last_OP = OPS_last(&New_OPs);
  Expand_Multiply (result, WN_rtype(expr), 
		   kid0_tn, mtype0, 
		   kid1_tn, mtype1, 
		   &New_OPs);

  if (Trace_Exp) {
    OP *op;
    if (Last_OP) op = OP_next(Last_OP);
    else op = OPS_first(&New_OPs);

    while (op != NULL) {
      fprintf(TFile, " into "); Print_OP (op);
      op = OP_next(op);
    }
  }

  return result;
}

/* ====================================================================
 *   Handle_MADD
 * ====================================================================
 */
static TN *
Handle_MADD (
  WN *expr, 
  TN *result,
  OPCODE opcode
)
{
  OP *Last_OP;
  TN *kid0_tn, *kid1_tn, *kid2_tn;

  if (result == NULL) {
    result = Allocate_Result_TN (expr, NULL);
  }

  if (MTYPE_is_float(OPCODE_rtype(opcode))) {
    kid0_tn = Expand_Expr (WN_kid0(expr), expr, NULL);
    kid1_tn = Expand_Expr (WN_kid1(expr), expr, NULL);
    kid2_tn = Expand_Expr (WN_kid2(expr), expr, NULL);
    Exp_OP3 (opcode, result, kid0_tn, kid1_tn, kid2_tn, &New_OPs);
    return result;
  }

  /* call make MADD with mtypes of arguments: */
  kid0_tn = Expand_Expr (WN_kid0(expr), expr, NULL);
  kid1_tn = Expand_Expr (WN_kid1(expr), expr, NULL);
  kid2_tn = Expand_Expr (WN_kid2(expr), expr, NULL);

  if (Trace_Exp) {
    #pragma mips_frequency_hint NEVER
    fprintf(TFile, "exp_madd %s: ", OPCODE_name(opcode));
    Print_TN(result, FALSE);
    fprintf(TFile, " :- ");
    Print_TN(kid0_tn, FALSE);
    fprintf(TFile, " ");
    Print_TN(kid1_tn, FALSE);
    fprintf(TFile, " ");
    Print_TN(kid2_tn, FALSE);
    fprintf(TFile, " \n");
  }

  /* for debuggging */
  Last_OP = OPS_last(&New_OPs);
  Expand_Madd (result, WN_rtype(expr), 
	       kid0_tn, WN_rtype(WN_kid0(expr)), 
	       kid1_tn, Get_mtype_for_mult(WN_kid1(expr), expr),
	       kid2_tn, Get_mtype_for_mult(WN_kid2(expr), expr),
	       &New_OPs);

  if (Trace_Exp) {
    OP *op = OP_next(Last_OP);
    while (op != NULL) {
      fprintf(TFile, " into "); Print_OP (op);
      op = OP_next(op);
    }
  }

  return result;
}

#endif /* TARG_ST */

/* ====================================================================
 *   Handle_DIVREM
 * ====================================================================
 */
static TN *
Handle_DIVREM (
  WN *expr, 
  WN *parent, 
  TN *result, 
  OPCODE opcode
)
{
  TN	*result2, *kid0_tn, *kid1_tn;

  Is_True ((parent && WN_class(parent) == CLASS_PREG), 
	   ("DIVREM: expected store of preg"));
#ifdef TARG_ST
  Is_True ((result == PREG_To_TN(WN_st(parent), WN_store_offset(parent),
                                 parent)), 
	   ("DIVREM: bad result tn"));
#else
  Is_True ((result == PREG_To_TN(WN_st(parent), WN_store_offset(parent))), 
	   ("DIVREM: bad result tn"));
#endif

  kid0_tn =	Expand_Expr (WN_kid0(expr), expr, NULL);
  kid1_tn =	Expand_Expr (WN_kid1(expr), expr, NULL);

  result2 = TN_CORRESPOND_Get(result, expr);
 
  Exp_OP(opcode,
	 result,
	 result2, 
	 kid0_tn,
	 kid1_tn,
	 V_NONE,
	&New_OPs);

  return result;
}

/* ====================================================================
 *   Handle_DIVPART
 * ====================================================================
 */
static TN *
Handle_DIVPART(WN *expr, WN *parent, TN *result)
{
  TN *pregTN;
  WN *kid = WN_kid0(expr);

  Is_True ((WN_class(kid) == CLASS_PREG), ("DIVPART: expected preg"));

#ifdef TARG_ST
  pregTN =  PREG_To_TN(WN_st(kid), WN_store_offset(kid), kid);
#else
  pregTN =  PREG_To_TN(WN_st(kid), WN_store_offset(kid));
#endif

  if (result==NULL)
  {
    return pregTN;
  }

  Exp_COPY (result, pregTN, &New_OPs);
  return result;
}

/* ====================================================================
 *   Handle_REMPART
 * ====================================================================
 */
static TN *
Handle_REMPART(WN *expr, WN *parent, TN *result)
{
  TN *pregTN;
  WN *kid = WN_kid0(expr);

  Is_True ((WN_class(kid) == CLASS_PREG), ("REMPART: expected preg"));

#ifdef TARG_ST
  pregTN =	PREG_To_TN(WN_st(kid), WN_store_offset(kid), kid);
#else
  pregTN =	PREG_To_TN(WN_st(kid), WN_store_offset(kid));
#endif
  pregTN =	TN_CORRESPOND_Get(pregTN, expr);

  Is_True ((pregTN),("expected tn correspondence"));

  if (result==NULL)
  {
    return pregTN;
  }

  Exp_COPY (result, pregTN, &New_OPs);
  return result;
}

/* ====================================================================
 *   Handle_MINMAX
 * ====================================================================
 */
static TN *
Handle_MINMAX(WN *expr, WN *parent, TN *result, OPCODE opcode)
{
  TN	*result2, *kid0_tn, *kid1_tn;

  Is_True ((parent && WN_class(parent) == CLASS_PREG), 
	   ("MINMAX: expected store of preg"));
#ifdef TARG_ST
  Is_True ((result == PREG_To_TN(WN_st(parent), WN_store_offset(parent),
                                 parent)), 
	   ("MINMAX: bad result tn"));
#else
  Is_True ((result == PREG_To_TN(WN_st(parent), WN_store_offset(parent))), 
	   ("MINMAX: bad result tn"));
#endif

  kid0_tn =	Expand_Expr (WN_kid0(expr), expr, NULL);
  kid1_tn =	Expand_Expr (WN_kid1(expr), expr, NULL);

  result2 = TN_CORRESPOND_Get(result, expr);
 
  Exp_OP(opcode,
	 result,
	 result2, 
	 kid0_tn,
	 kid1_tn,
	 V_NONE,
	&New_OPs);

  return result;
}

/* ====================================================================
 *   Handle_MINPART
 * ====================================================================
 */
static TN *
Handle_MINPART(WN *expr, WN *parent, TN *result)
{
  TN *pregTN;
  WN *kid = WN_kid0(expr);

  Is_True ((WN_class(kid) == CLASS_PREG), ("MINPART: expected preg"));

#ifdef TARG_ST
  pregTN =  PREG_To_TN(WN_st(kid), WN_store_offset(kid), kid);
#else
  pregTN =  PREG_To_TN(WN_st(kid), WN_store_offset(kid));
#endif

  if (result==NULL)
  {
    return pregTN;
  }

  Exp_COPY (result, pregTN, &New_OPs);
  return result;
}

/* ====================================================================
 *   Handle_MAXPART
 * ====================================================================
 */
static TN *
Handle_MAXPART(WN *expr, WN *parent, TN *result)
{
  TN *pregTN;
  WN *kid = WN_kid0(expr);

  Is_True ((WN_class(kid) == CLASS_PREG), ("MAXPART: expected preg"));

#ifdef TARG_ST
  pregTN =	PREG_To_TN(WN_st(kid), WN_store_offset(kid), kid);
#else
  pregTN =	PREG_To_TN(WN_st(kid), WN_store_offset(kid));
#endif
  pregTN =	TN_CORRESPOND_Get(pregTN, expr);

  Is_True ((pregTN),("expected tn correspondende"));

  if (result==NULL)
  {
    return pregTN;
  }

  Exp_COPY (result, pregTN, &New_OPs);
  return result;
}

/* ====================================================================
 *   Handle_ILOAD
 * ====================================================================
 */
static TN *
Handle_ILOAD (
  WN *iload, 
  TN *result, 
  OPCODE opcode
)
{
  VARIANT variant;
  WN *kid0 = WN_kid0(iload);
  ST *st;

  if (opcode == OPC_U4U8ILOAD) {
    opcode = OPC_U4U4ILOAD;
    WN_set_opcode(iload, opcode);
    if (Target_Byte_Sex == BIG_ENDIAN) {
	WN_offset(iload) += 4;	// get low-order word
    }
  }
  else if (opcode == OPC_I4I8ILOAD) {
    opcode =	OPC_I4I4ILOAD;
    WN_set_opcode(iload, opcode);
    if (Target_Byte_Sex == BIG_ENDIAN) {
	WN_offset(iload) += 4;	// get low-order word
    }
  }

  variant = Memop_Variant(iload);
  if (result == NULL) 
    result = Allocate_Result_TN (iload, NULL);

  /* If the kid of the ILOAD is an LDA, handle the ILOAD like an LDID */
  if (WN_operator_is(kid0, OPR_LDA)) {
    Last_Mem_OP = OPS_last(&New_OPs);
    st = WN_st(kid0);
    /* make sure st is allocated */
    Allocate_Object (st);

    Exp_Load (OPCODE_rtype(opcode), OPCODE_desc(opcode),
	result, 
	st, 
	WN_offset(iload) + WN_lda_offset(kid0),
	&New_OPs,
	variant);
  }
  else {
    TN *kid0_tn = Expand_Expr (kid0, iload, NULL); 
    TN *offset_tn = Gen_Literal_TN (WN_offset(iload), 4);
    Last_Mem_OP = OPS_last(&New_OPs);
    Exp_OP2v (opcode, result, kid0_tn, offset_tn, variant, &New_OPs);
  }
  Set_OP_To_WN_Map(iload);
  return result;
}

/* ====================================================================
 *   Handle_ILDBITS
 * ====================================================================
 */
static TN *
Handle_ILDBITS (WN *ildbits, TN *result, OPCODE opcode)
{
  VARIANT variant = Memop_Variant(ildbits);
  WN *kid0 = WN_kid0(ildbits);
  ST *st;
  UINT bit_ofst;
  TN *src_tn = Allocate_Result_TN (ildbits, NULL);
  if (result == NULL) result = Allocate_Result_TN (ildbits, NULL);

  /* If the kid of the ILDBITSs is an LDA, handle the ILDBITS like an LDBITS */
  if (WN_operator_is(kid0, OPR_LDA)) {
    Last_Mem_OP = OPS_last(&New_OPs);
    st = WN_st(kid0);
    /* make sure st is allocated */
    Allocate_Object (st);

    Exp_Load (OPCODE_rtype(opcode), OPCODE_desc(opcode),
	src_tn, 
	st, 
	WN_offset(ildbits) + WN_lda_offset(kid0),
	&New_OPs,
	variant);
  }
  else {
    TN *kid0_tn = Expand_Expr (kid0, ildbits, NULL); 
    TN *offset_tn = Gen_Literal_TN (WN_offset(ildbits), 4);
    Last_Mem_OP = OPS_last(&New_OPs);
    Exp_OP2v (OPCODE_make_op(OPR_ILOAD, OPCODE_rtype(opcode), OPCODE_desc(opcode)), 
	      src_tn, kid0_tn, offset_tn, variant, &New_OPs);
  }
  Set_OP_To_WN_Map(ildbits);


  if (Target_Byte_Sex == LITTLE_ENDIAN) {
      // for LX as LITTLE_ENDIAN:
      bit_ofst = WN_bit_offset(ildbits);
  } else {
      // for LX as BIG_ENDIAN:
      bit_ofst = MTYPE_bit_size(WN_desc(ildbits)) - WN_bit_size(ildbits) - WN_bit_offset(ildbits);
  }
  Exp_Extract_Bits(WN_rtype(ildbits), WN_desc(ildbits), bit_ofst,
		   WN_bit_size(ildbits), result, src_tn, &New_OPs);
  return result;
}

static BOOL Operator_Is_Bool (OPERATOR opr) {

  if (opr == OPR_LNOT ||
      opr == OPR_EQ ||
      opr == OPR_NE ||
      opr == OPR_GE ||
      opr == OPR_GT ||
      opr == OPR_LE ||
      opr == OPR_LT ||
      opr == OPR_LAND ||
      opr == OPR_LIOR) {
    return TRUE;
  } else {
    return FALSE;
  }
}

/* ======================================================================
 *   Handle_STID
 * ======================================================================
 */
static void
Handle_STID (
  WN *stid, 
  OPCODE opcode
)
{
  TN *result;

#ifdef EMULATE_LONGLONG
  {
    // long long check: if LHS is I/U8 and RHS is I4, then RHS must be
    // a boolean operator
    TYPE_ID rtype = OPCODE_rtype(opcode);
    TYPE_ID dtype = WN_rtype(WN_kid0(stid));
    if ((rtype == MTYPE_I8 || rtype == MTYPE_U8) && dtype == MTYPE_I4) {
      FmtAssert (Operator_Is_Bool (WN_operator(WN_kid0(stid))),
                 ("Handle_STID: LHS is I/U8, RHS is I4 and not BOOL"));
    }
  }
#endif

  if (ST_assigned_to_dedicated_preg(WN_st(stid))) {
    // replace st with dedicated preg
    WN_offset(stid) = Find_PREG_For_Symbol(WN_st(stid));
    WN_st_idx(stid) = ST_st_idx(MTYPE_To_PREG(ST_mtype(WN_st(stid))));
  }

  /* Check if we have an STID of a PREG. Get the TN corresponding to
   * the PREG and use it as the result TN for the kid0 of the STID.
   */
  if (WN_class(stid) == CLASS_PREG) {
    WN *kid = WN_kid0(stid);

#ifdef TARG_ST
    result = PREG_To_TN (WN_st(stid), WN_store_offset(stid), stid);
#else
    result = PREG_To_TN (WN_st(stid), WN_store_offset(stid));
#endif

#ifdef ENABLE_64_BITS
    if (MTYPE_bsize(OPCODE_desc(opcode)) > TN_size(result)) {
      TN *tmp = result;
      result = Expand_Expr (kid, stid, NULL);
      Expand_Multi(tmp, result, &New_OPs);
    } else 
#endif
    Expand_Expr (kid, stid, result);

    if (In_Glue_Region) {
      if (Trace_REGION_Interface) {
	fprintf(TFile,"set op_glue on preg store in bb %d\n", BB_id(Cur_BB));
      }
      Set_OP_glue(OPS_last(&New_OPs));
    }

#ifdef TARG_ST
   /* Reconfigurability
    * case 1: we have an intrinsic call with multiple
    *         resuls. Pattern look typically:
    *
    *     INTRINSIC_CALL xxxxx
    *         LDID -1<xxxxx>
    *       SUBPART1 <xxxxx>
    *     STID <xxxxx>
    *         LDID -1<xxxxx>
    *       SUBPART2 <xxxxx>
    *     STID <xxxxx>
    *
    * The negative PREG comes from the fact that we
    * don't have lowered the WHIRL for this specific
    * construction.
    *
    * Following copy expansion is skipped.
    */
   if (WN_operator_is(kid,OPR_SUBPART))
    { WN *kidkid = WN_kid0(kid);

      if(WN_operator_is(kidkid,OPR_LDID) && 
         WN_class(kidkid)==CLASS_PREG    &&
         WN_load_offset(kidkid)<0)
           return;

      FmtAssert(0,("SUBPART operator uncorrectly handled"));
    }
#endif

    /* If the child is a PREG and has a corresponding TN:
     *   1. it was part of a DIVREM or MTYPE_B pair,
     *   2. it is SP or FP realtive PREG.
     * We need to create a correspondence for the STID, and do an assignment
     */
    if (WN_operator_is(kid, OPR_LDID) && WN_class(kid) == CLASS_PREG) {

#ifdef TARG_ST
      /* Reconfigurability.
       * case 2: we have an intrinsic call with a single result.
       *         Therefore SUBPART operator is useless.
       *
       * As previously, we skip copy expansion.
       */
      if(WN_load_offset(kid)<0)
         return;

      TN *ldidTN = PREG_To_TN (WN_st(kid), WN_load_offset(kid), kid);
#else
      TN *ldidTN = PREG_To_TN (WN_st(kid), WN_load_offset(kid));
#endif

      TN *ldidTN2 = TN_CORRESPOND_Lookup(ldidTN);
      if (ldidTN2 != NULL) {
	TN *stidTN = TN_CORRESPOND_Get(result, kid);
	Exp_COPY(stidTN, ldidTN2, &New_OPs);
      } else if (Is_Predicate_REGISTER_CLASS(TN_register_class(ldidTN))) {
#if !defined(TARG_ST) ||  defined(TARG_IA64)
	/* [CG]: This code handle the double predicate definition for IA64
	   conditionalized it under TARG_IA64. */
	Is_True(Is_Predicate_REGISTER_CLASS(TN_register_class(result)),
		  ("result should be predicate register class"));
	PREG_NUM cpreg_num = WN_load_offset(kid) + 1;
	TN *ctn = PREG_To_TN_Array[cpreg_num];
	PREG_NUM cresult_num = WN_store_offset(stid) + 1;
	TN *cresult = PREG_To_TN_Array[cresult_num];
	Exp_COPY (cresult, ctn, &New_OPs);
#endif
      }
    }

    return;
  }

  // generate a store:
  VARIANT variant = Memop_Variant(stid);

  Last_Mem_OP = OPS_last(&New_OPs);
  Exp_Store (OPCODE_desc(opcode), 
	     Expand_Expr (WN_kid0(stid), stid, NULL),
	     WN_st(stid), 
	     WN_store_offset(stid),
	     &New_OPs,
	     variant);
  Set_OP_To_WN_Map(stid);
  return;
}

/* ======================================================================
 *   Handle_STBITS
 *
 *   Writing into a bitfield: 
 * ======================================================================
 */
static void
Handle_STBITS (
  WN *stbits
)
{
  VARIANT variant;
  WN *kid = WN_kid0(stbits);
  TN *result;
  TN *field_tn;
  TN *bits_tn = Allocate_Result_TN (kid, NULL);
  UINT bit_ofst;
  const TYPE_ID desc = Mtype_TransferSign(MTYPE_U4, WN_desc(stbits));
  const TYPE_ID rtype = Mtype_TransferSize(WN_rtype(kid), desc);

  Expand_Expr (kid, stbits, bits_tn);

  /* Check if we have an STBITS of a PREG. Get the TN corresponding to
   * the PREG */
  if (WN_class(stbits) == CLASS_PREG) {
#ifdef TARG_ST
    field_tn = PREG_To_TN (WN_st(stbits), WN_store_offset(stbits), stbits);
#else
    field_tn = PREG_To_TN (WN_st(stbits), WN_store_offset(stbits));
#endif
    result = field_tn;
  } else {
    variant = Memop_Variant(stbits);
    field_tn = Allocate_Result_TN (kid, NULL);
    Last_Mem_OP = OPS_last(&New_OPs);
    Exp_Load(rtype, desc, field_tn, WN_st(stbits), WN_load_offset(stbits),
	      &New_OPs, variant); // must do an unsigned load
    Set_OP_To_WN_Map(stbits);
    result = Allocate_Result_TN (kid, NULL);
  }

  if (Target_Byte_Sex == LITTLE_ENDIAN) {
      // for LX as LITTLE_ENDIAN:
      bit_ofst = WN_bit_offset(stbits);
  } else {
      // for LX as BIG_ENDIAN:
      bit_ofst = MTYPE_bit_size(WN_desc(stbits)) - WN_bit_size(stbits) - WN_bit_offset(stbits);
  }

  // deposit bits_tn into field_tn returning result in result
  Exp_Deposit_Bits(rtype, desc, bit_ofst,
		   WN_bit_size(stbits), result, field_tn, bits_tn, &New_OPs);

  if (WN_class(stbits) != CLASS_PREG) 
    {
    Last_Mem_OP = OPS_last(&New_OPs);
    Exp_Store(desc, result, WN_st(stbits), WN_store_offset(stbits), &New_OPs,
	      variant);
    Set_OP_To_WN_Map(stbits);
  }
}

/* ====================================================================
 *   Handle_COMPOSE_BITS
 * ====================================================================
 */
static TN *
Handle_COMPOSE_BITS (WN *compbits, TN *result, OPCODE opcode)
{
  TN *kid0_tn = Expand_Expr (WN_kid0(compbits), compbits, NULL);
  TN *kid1_tn = Expand_Expr (WN_kid1(compbits), compbits, NULL);
  if (result == NULL) result = Allocate_Result_TN (compbits, NULL);

  Exp_Deposit_Bits(OPCODE_rtype(opcode), OPCODE_rtype(opcode), 
		   WN_bit_offset(compbits), WN_bit_size(compbits), 
		   result, kid0_tn, kid1_tn, &New_OPs);

  return result;
}

/* ====================================================================
 *   Handle_LROTATE
 * ====================================================================
 */
static TN *
Handle_LROTATE (WN *lrotate, TN *result, OPCODE opcode)
{
  TN *kid0_tn = Expand_Expr (WN_kid0(lrotate), lrotate, NULL);
  TN *kid1_tn = Expand_Expr (WN_kid1(lrotate), lrotate, NULL);
  if (result == NULL) result = Allocate_Result_TN (lrotate, NULL);

  Exp_LRotate(OPCODE_rtype(opcode), result, kid0_tn, kid1_tn, &New_OPs);

  return result;
}

/* ====================================================================
 *   Handle_RROTATE
 * ====================================================================
 */
static TN *
Handle_RROTATE (WN *rrotate, TN *result, OPCODE opcode)
{
  TN *kid0_tn = Expand_Expr (WN_kid0(rrotate), rrotate, NULL);
  TN *kid1_tn = Expand_Expr (WN_kid1(rrotate), rrotate, NULL);
  if (result == NULL) result = Allocate_Result_TN (rrotate, NULL);

  Exp_RRotate(OPCODE_rtype(opcode), result, kid0_tn, kid1_tn, &New_OPs);

  return result;
}

/* ====================================================================
 *   Handle_ISTORE
 * ====================================================================
 */
static void
Handle_ISTORE (WN *istore, OPCODE opcode)
{
  VARIANT variant  = Memop_Variant(istore);
  WN *kid1 = WN_kid1(istore);
  TN *kid0_tn = Expand_Expr (WN_kid0(istore), istore, NULL);
  ST *st;

#ifdef EMULATE_LONGLONG
  {
    // long long check: If the LHS is an I/U8, and the RHS is I4,
    // then assert that RHS must be a boolean.
    TYPE_ID rtype = WN_rtype(WN_kid0(istore));
    TYPE_ID dtype = OPCODE_desc(opcode);
    if ((dtype == MTYPE_I8 || dtype == MTYPE_U8) && rtype == MTYPE_I4) {
      FmtAssert (Operator_Is_Bool (WN_operator(WN_kid0(istore))),
                 ("Handle_ISTORE: LHS is I/U8, RHS is I4 and not BOOL"));
    }
  }
#endif

  /* if the kid1 is an LDA, treat the ISTORE as an STID */
  if (WN_operator_is(kid1, OPR_LDA)) {
    Last_Mem_OP = OPS_last(&New_OPs);
    st = WN_st(kid1);
    /* make sure st is allocated */
    Allocate_Object (st);

    Exp_Store (
	OPCODE_desc(opcode),
	kid0_tn,
	st,
	WN_offset(istore) + WN_lda_offset(kid1),
	&New_OPs,
	variant);
  }
  else {
    TN *kid1_tn = Expand_Expr (WN_kid1(istore), istore, NULL);
    TN *offset_tn = Gen_Literal_TN (WN_offset(istore), 4);
    Last_Mem_OP = OPS_last(&New_OPs);
    Exp_OP3v (
	opcode, 
	NULL, 
	kid0_tn,
	kid1_tn,
	offset_tn,
	variant,
	&New_OPs);
  }
  Set_OP_To_WN_Map(istore);
}

/* ====================================================================
 *   Handle_ISTBITS
 *
 *   A store of value computed by kid0 is performed to address in
 *   bytes given by adding the WN_store_offset(istbits) field to the 
 *   the address computed by kid1.
 *
 *   WN_desc(istbits) gives the unit of memory being accessed to
 *   perform the bitfield deposition.
 * ====================================================================
 */
static void
Handle_ISTBITS (
  WN *istbits
)
{
  VARIANT variant  = Memop_Variant(istbits);
  WN *kid0 = WN_kid0(istbits);
  WN *kid1 = WN_kid1(istbits);
  TN *bits_tn = Expand_Expr (kid0, istbits, NULL);
  TN *field_tn = Allocate_Result_TN (kid0, NULL);
  TN *result = Allocate_Result_TN (kid0, NULL);
  TYPE_ID desc = Mtype_TransferSign(MTYPE_U4, WN_desc(istbits));
  TYPE_ID rtype = desc;
  UINT bit_ofst;
  /*
  TN *field_tn = Build_TN_Of_Mtype (desc);
  TN *result = Build_TN_Of_Mtype (desc);
  */

  // guard against U1MPY or U2MPY
  if (MTYPE_byte_size(rtype) < 4)
    rtype = Mtype_TransferSize(MTYPE_U4, rtype);
  if (MTYPE_byte_size(WN_rtype(kid0)) > MTYPE_byte_size(rtype)) 
    rtype = Mtype_TransferSize(WN_rtype(kid0), rtype);

  if (Target_Byte_Sex == LITTLE_ENDIAN) {
      // for LX as LITTLE_ENDIAN:
      bit_ofst = WN_bit_offset(istbits);
  } else {
      // for LX as BIG_ENDIAN:
      bit_ofst = MTYPE_bit_size(WN_desc(istbits)) - WN_bit_size(istbits) - WN_bit_offset(istbits);
  }

  /* if the kid1 is an LDA, treat the ISTBITS as an STBITS */
  if (WN_operator_is(kid1, OPR_LDA)) {
    Last_Mem_OP = OPS_last(&New_OPs);
    ST *st = WN_st(kid1);
    /* make sure st is allocated */
    Allocate_Object (st);

    Last_Mem_OP = OPS_last(&New_OPs);
    Exp_Load(rtype, desc, field_tn, st, 
	      WN_store_offset(istbits) + WN_lda_offset(kid1), &New_OPs,
	      variant);
    Set_OP_To_WN_Map(istbits);

    // deposit bits_tn into field_tn returning result in result
    Exp_Deposit_Bits(rtype, desc, bit_ofst, 
		     WN_bit_size(istbits), result, field_tn, bits_tn, &New_OPs);

    Last_Mem_OP = OPS_last(&New_OPs);
    Exp_Store(desc, result, st, WN_store_offset(istbits) + WN_lda_offset(kid1),
	      &New_OPs, variant);
    Set_OP_To_WN_Map(istbits);
  }
  else {
    TN *kid1_tn = Expand_Expr (WN_kid1(istbits), istbits, NULL);
    TN *offset_tn = Gen_Literal_TN (WN_store_offset(istbits), 4);

    Last_Mem_OP = OPS_last(&New_OPs);
    Exp_OP2v(OPCODE_make_op(OPR_ILOAD, rtype, desc),
	     field_tn, kid1_tn, offset_tn, variant, &New_OPs);
    Set_OP_To_WN_Map(istbits);

    // deposit bits_tn into field_tn returning result in result
    Exp_Deposit_Bits(rtype, desc, bit_ofst,
         WN_bit_size(istbits), result, field_tn, bits_tn, &New_OPs);

    Last_Mem_OP = OPS_last(&New_OPs);
    Exp_OP3v(OPCODE_make_op(OPR_ISTORE, MTYPE_V, desc), 
	       NULL, result, kid1_tn, offset_tn, variant, &New_OPs);
    Set_OP_To_WN_Map(istbits);
  }

  return;
}

/* ====================================================================
 *   WHIRL_Compare_To_OP_variant
 *
 *   Given a WHIRL comparison opcode, this routine returns
 *   the corresponding variant for the BCOND. 
 *   For float compares, keep the order of the comparison and
 *   set the false_br flag if invert, because NaN comparisons
 *   cannot be inverted.  But integer compares can be inverted.
 * ====================================================================
 */
static INT16
WHIRL_Compare_To_OP_variant (
  OPCODE opcode
)
{
  INT16 variant;

  variant = V_BR_NONE;

  switch (opcode) {
// >> WHIRL 0.30: replaced OPC_T1{EQ,NE,GT,GE,LT,LE} by OPC_BT1, OPC_I4T1 variants
// TODO WHIRL 0.30: get rid of OPC_I4T1 variants
  case OPC_BI8EQ: case OPC_I4I8EQ: variant = V_BR_I8EQ; break;
  case OPC_BI4EQ: case OPC_I4I4EQ: variant = V_BR_I4EQ; break;
  case OPC_BU8EQ: case OPC_I4U8EQ: variant = V_BR_U8EQ; break;
  case OPC_BU4EQ: case OPC_I4U4EQ: variant = V_BR_U4EQ; break;
  case OPC_BA4EQ:                  variant = V_BR_A4EQ; break;
  case OPC_BA8EQ:                  variant = V_BR_A8EQ; break;
  case OPC_BFQEQ: case OPC_I4FQEQ: variant = V_BR_QEQ; break;
  case OPC_BF8EQ: case OPC_I4F8EQ: variant = V_BR_DEQ; break;
  case OPC_BF4EQ: case OPC_I4F4EQ: variant = V_BR_FEQ; break;
  case OPC_BI8NE: case OPC_I4I8NE: variant = V_BR_I8NE; break;
  case OPC_BI4NE: case OPC_I4I4NE: variant = V_BR_I4NE; break;
  case OPC_BU8NE: case OPC_I4U8NE: variant = V_BR_U8NE; break;
  case OPC_BU4NE: case OPC_I4U4NE: variant = V_BR_U4NE; break;
  case OPC_BA4NE:                  variant = V_BR_A4NE; break;
  case OPC_BA8NE:                  variant = V_BR_A8NE; break;
  case OPC_BFQNE: case OPC_I4FQNE: variant = V_BR_QNE; break;
  case OPC_BF8NE: case OPC_I4F8NE: variant = V_BR_DNE; break;
  case OPC_BF4NE: case OPC_I4F4NE: variant = V_BR_FNE; break;
  case OPC_BI8GT: case OPC_I4I8GT: variant = V_BR_I8GT; break;
  case OPC_BI4GT: case OPC_I4I4GT: variant = V_BR_I4GT; break;
  case OPC_BU8GT: case OPC_I4U8GT: variant = V_BR_U8GT; break;
  case OPC_BU4GT: case OPC_I4U4GT: variant = V_BR_U4GT; break;
  case OPC_BA4GT:                  variant = V_BR_A4GT; break;
  case OPC_BA8GT:                  variant = V_BR_A8GT; break;
  case OPC_BFQGT: case OPC_I4FQGT: variant = V_BR_QGT; break;
  case OPC_BF8GT: case OPC_I4F8GT: variant = V_BR_DGT; break;
  case OPC_BF4GT: case OPC_I4F4GT: variant = V_BR_FGT; break;
  case OPC_BI8GE: case OPC_I4I8GE: variant = V_BR_I8GE; break;
  case OPC_BI4GE: case OPC_I4I4GE: variant = V_BR_I4GE; break;
  case OPC_BU8GE: case OPC_I4U8GE: variant = V_BR_U8GE; break;
  case OPC_BU4GE: case OPC_I4U4GE: variant = V_BR_U4GE; break;
  case OPC_BA4GE:                  variant = V_BR_A4GE; break;
  case OPC_BA8GE:                  variant = V_BR_A8GE; break;
  case OPC_BFQGE: case OPC_I4FQGE: variant = V_BR_QGE; break;
  case OPC_BF8GE: case OPC_I4F8GE: variant = V_BR_DGE; break;
  case OPC_BF4GE: case OPC_I4F4GE: variant = V_BR_FGE; break;
  case OPC_BI8LT: case OPC_I4I8LT: variant = V_BR_I8LT; break;
  case OPC_BI4LT: case OPC_I4I4LT: variant = V_BR_I4LT; break;
  case OPC_BU8LT: case OPC_I4U8LT: variant = V_BR_U8LT; break;
  case OPC_BU4LT: case OPC_I4U4LT: variant = V_BR_U4LT; break;
  case OPC_BA4LT:                  variant = V_BR_A4LT; break;
  case OPC_BA8LT:                  variant = V_BR_A8LT; break;
  case OPC_BFQLT: case OPC_I4FQLT: variant = V_BR_QLT; break;
  case OPC_BF8LT: case OPC_I4F8LT: variant = V_BR_DLT; break;
  case OPC_BF4LT: case OPC_I4F4LT: variant = V_BR_FLT; break;
  case OPC_BI8LE: case OPC_I4I8LE: variant = V_BR_I8LE; break;
  case OPC_BI4LE: case OPC_I4I4LE: variant = V_BR_I4LE; break;
  case OPC_BU8LE: case OPC_I4U8LE: variant = V_BR_U8LE; break;
  case OPC_BU4LE: case OPC_I4U4LE: variant = V_BR_U4LE; break;
  case OPC_BA4LE:                  variant = V_BR_A4LE; break;
  case OPC_BA8LE:                  variant = V_BR_A8LE; break;
  case OPC_BFQLE: case OPC_I4FQLE: variant = V_BR_QLE; break;
  case OPC_BF8LE: case OPC_I4F8LE: variant = V_BR_DLE; break;
  case OPC_BF4LE: case OPC_I4F4LE: variant = V_BR_FLE; break;
  case OPC_BBNE: variant = V_BR_PNE; break;
  case OPC_BBEQ: variant = V_BR_PEQ; break;
// << WHIRL 0.30: replaced OPC_T1{EQ,NE,GT,GE,LT,LE} by OPC_BT1, OPC_I4T1 variants
  }

  /* do invert in the target dependent part of the compiler
  if (invert) variant = Negate_BR_Variant(variant);
  */

  return variant;
}

/* ====================================================================
 *   Handle_SELECT
 * ====================================================================
 */
static TN *
Handle_SELECT (
  WN *select, 
  TN *result, 
  OPCODE opcode
)
{
  TN	*trueop, *falseop, *cond;
  TN	*op1, *op2;
  WN	*compare;
  VARIANT variant;

 /*
  *  Expand the true/false before the condition
  */
  trueop = Expand_Expr (WN_kid1(select), select, NULL);
  falseop = Expand_Expr (WN_kid2(select), select, NULL);
  compare = WN_kid0(select);

  if (result == NULL) result = Allocate_Result_TN (select, NULL);

  variant = WHIRL_Compare_To_OP_variant (WN_opcode(compare));
  if (Check_Select_Expansion (WN_opcode(compare)) || (variant == V_BR_NONE)) {
	Is_True(   WN_desc(select) != MTYPE_B
		|| (   WN_operator_is(compare, OPR_LDID) 
		    && WN_class(compare) == CLASS_PREG),
		("MTYPE_B SELECT condition must be preg or relop"));
    	cond = Expand_Expr (compare, select, NULL);
	Exp_OP3 (opcode, result, cond, trueop, falseop, &New_OPs);
  } 
  else {
    	op1 = Expand_Expr (WN_kid0(compare), compare, NULL);
    	op2 = Expand_Expr (WN_kid1(compare), compare, NULL);
  	Exp_Select_And_Condition (opcode, result, trueop, falseop, 
				  WN_opcode(compare), op1, op2, variant, 
				  &New_OPs);
  }

  return result;
}

/* ====================================================================
 *   Handle_LAND_LIOR
 *
 *	Try to change logical operations into binary operations
 *
 *	If the operands are boolean, then we can use the binary version
 * ====================================================================
 */
static TN*
Handle_LAND_LIOR(WN *expr, WN *parent, TN *result)
{
  VARIANT  variant;
  OPCODE opcode = WN_opcode(expr);
  TN *op1 = Expand_Expr(WN_kid0(expr), expr, NULL);
  TN *op2 = Expand_Expr(WN_kid1(expr), expr, NULL);

  variant = V_NONE;
  if (OPCODE_is_boolean(WN_opcode(WN_kid0(expr)))) Set_V_normalized_op1(variant);
  if (OPCODE_is_boolean(WN_opcode(WN_kid1(expr)))) Set_V_normalized_op2(variant);

  if (result == NULL) result = Allocate_Result_TN (expr, NULL);

  Exp_OP2v(opcode, result, op1, op2, variant, &New_OPs);

  return result;
}

/* ====================================================================
 *   Handle_LNOT
 * ====================================================================
 */
static TN*
Handle_LNOT(WN *expr, WN *parent, TN *result)
{
  VARIANT  variant;
  OPCODE opcode = WN_opcode(expr);
  WN *kid0 = WN_kid0(expr);
  TN *op1 = Expand_Expr(kid0, expr, NULL);

  variant = V_NONE;
  if (OPCODE_is_boolean(WN_opcode(kid0))) Set_V_normalized_op1(variant);

  if (result == NULL) result = Allocate_Result_TN (expr, NULL);

  if (   WN_rtype(kid0) == MTYPE_B
      && WN_operator_is(kid0, OPR_LDID)
      && WN_class(kid0) == CLASS_PREG)
  {
    PREG_NUM cpreg_num = WN_load_offset(kid0) + 1;
    TN *ctn = PREG_To_TN_Array[cpreg_num];
    PREG_NUM result_num = TN_To_PREG(result);
    if (result_num) {
      PREG_NUM cresult_num = result_num + 1;
      TN *cresult = PREG_To_TN_Array[cresult_num];
      Exp_COPY (cresult, op1, &New_OPs);
    }
    Exp_COPY (result, ctn, &New_OPs);
  } else {
    Exp_OP1v(opcode, result, op1, variant, &New_OPs);
  }

  return result;
}

/* ======================================================================
 *   Handle_ALLOCA
 * ======================================================================
 */
static TN* 
Handle_ALLOCA (
  WN *tree, 
  TN *result
)
{
  TN *tsize = Expand_Expr (WN_kid0(tree), tree, NULL);
  // align the size
  if (TN_has_value(tsize)) {
  	INT64 size; 
	size = TN_value(tsize);
	size += Stack_Alignment() - 1;
	size &= -Stack_Alignment();
	tsize = Gen_Literal_TN (size, Pointer_Size);
  }
  else if ( ! TN_is_zero_reg (tsize)) {
	TN *tmp1 = Build_TN_Like ( tsize );
	TN *tmp2 = Build_TN_Like ( tsize );
	Exp_ADD (Pointer_Mtype, tmp1, tsize, 
		Gen_Literal_TN (Stack_Alignment()-1, Pointer_Size), 
		&New_OPs);
  	Exp_OP2 ((MTYPE_is_size_double(WN_rtype(tree)) ? OPC_I8BAND : OPC_I4BAND), 
		tmp2, tmp1, 
		Gen_Literal_TN (-Stack_Alignment(), Pointer_Size), 
		&New_OPs);
	tsize = tmp2;
  }

  INT64 offset;
  INT stack_adjustment = Stack_Offset_Adjustment_For_PU();
  if ( ! TN_is_zero(tsize)) {
	Exp_Spadjust (SP_TN, tsize, V_SPADJUST_MINUS, &New_OPs);

  	// return sp + arg area
	offset = Current_PU_Actual_Size + stack_adjustment;
  }
  else {
	// tsize == 0
	// return original $sp + stack_adjustment
	offset = stack_adjustment;
  }
  if (offset == 0) {
	if (result == NULL)
		result = SP_TN;
	else
		Exp_COPY(result, SP_TN, &New_OPs);
  }
  else {
  	if (result == NULL) 
		result = Build_TN_Of_Mtype (Pointer_Mtype);
	Exp_ADD (Pointer_Mtype, result, SP_TN,
		Gen_Literal_TN (offset, Pointer_Size), 
		&New_OPs);
  }
  return result;
}

static void
Handle_DEALLOCA (WN *tree)
{
  INT stack_adjustment = Stack_Offset_Adjustment_For_PU();
  TN *val = Expand_Expr (WN_kid0(tree), tree, NULL);
  Last_Mem_OP = OPS_last(&New_OPs);
  if (stack_adjustment != 0) {
	Exp_ADD (Pointer_Mtype, val, val, 
		Gen_Literal_TN (-stack_adjustment, Pointer_Size),
		&New_OPs);
  }
  Exp_Spadjust (SP_TN, val, V_SPADJUST_PLUS, &New_OPs);
  Set_OP_To_WN_Map (tree);
}


#ifdef TARG_ST
/* ======================================================================
 *   Get_Intrinsic_Call_Dedicated_Tn
 *
 *   Implement a small forward pass in order to determine
 *   if some INTRINSIC_CALL results of are in fact dedicated TNs.
 *   Resemble routine Find_Asm_Out_Parameter_Load.
 * ======================================================================
 */
static TN* 
Get_Intrinsic_Call_Dedicated_Tn( WN *intr, INT numout )
{
    TN  *result = NULL;   // Default value.
    bool found = false;   // Whether nth result has been found.
    WN  *stmt;
    WN  *kid0;
    INT  index_res = -1;  // Return current index of results.
    ST  *ded_st;

    /* Looking for nth output of an intrinsic call */
    for(stmt  = WN_next(intr);
        found == false && stmt && !OPERATOR_is_call(WN_operator(stmt));
        stmt  = WN_next(stmt))
     {
       if(!WN_operator_is(stmt,OPR_STID))
           continue;

       kid0 = WN_kid0(stmt);
       index_res = 0;
       if(WN_operator_is(kid0,OPR_SUBPART)) {
	 index_res = WN_subpart_index(kid0);
	 kid0 = WN_kid0(kid0);
       }

       if(WN_operator_is(kid0,OPR_LDID) &&
          WN_load_offset(kid0)<0 &&
	  index_res == numout) {
           found = true;
           ded_st = WN_st(stmt);
           if (ST_assigned_to_dedicated_preg(ded_st))
                 result = PREG_To_TN(MTYPE_To_PREG(ST_mtype(ded_st)),
                                     Find_PREG_For_Symbol(ded_st), stmt);
          }
      }

    // It is possible to not find any use of the INSTRINSIC output
    // if the result is dead but the INTRINSIC is side effect.
    // Return NULL in this case. Note that this must happen only
    // for allocatable register classes of the corresponding output.
    // I.e. an intrinsic result store to a dedicated ST should never be
    // removed by deadcode.

   return result;
}


/* ======================================================================
 *   Is_Intrinsic_InOut_Param_Single_TN
 *
 *   Return TRUE if the specified in/out parameter of the multi-result 
 *   intrinsic call <intrn_expr> uses the same TN as input and output.
 *   The input TN is specified <source_tn>, as well as the index
 *   in the output parameter list (<result_idx>).
 * ======================================================================
 */
static BOOL
Is_Intrinsic_InOut_Param_Single_TN(WN *intrn_expr, INT result_idx, TN *source_tn) {
  // For multi-result intrinsic call, the effective call is 
  // followed by a list of load/store statements that walk
  // through all result subparts.
  WN *next_expr = WN_next(intrn_expr);
  while (next_expr) {
    OPERATOR opr = WN_operator(next_expr);
    if (opr == OPR_INTRINSIC_CALL) {
      return (FALSE); // Found another intrinsic call, break the walk.
    }
    else if (opr == OPR_STID) {
      WN *stid = next_expr;
      if (WN_operator(WN_kid0(stid)) == OPR_SUBPART) {
	WN       *subpart     = WN_kid0(stid);
	WN       *kid0        = WN_kid0(subpart);
	WN_OFFSET subpart_idx = WN_subpart_index(subpart);
	
	if (subpart_idx == result_idx) {
	  // OK, found access to expected result subpart
	  if (WN_class(stid) == CLASS_PREG) {
	    TN *result_tn = PREG_To_TN (WN_st(stid), WN_store_offset(stid), stid);
	    if (result_tn != source_tn) {
	      return (FALSE);
	    }
	  }
	  return (TRUE);
	}
      }
    }
    next_expr = WN_next(next_expr);
  }
  return (FALSE);
}
#endif                    /* TARG_ST */

/* ======================================================================
 *   Get_Intrinsic_Op_Parameters
 * ======================================================================
 */

static void
Get_Intrinsic_Op_Parameters( WN *expr, TN **result, TN ***opnds, INT *numopnds, TN ***res, INT *numrests ) {

  INT i;
  INTRINSIC id = (INTRINSIC) WN_intrinsic (expr);
  INTRN_RETKIND rkind = INTRN_return_kind(id);
  INT numkids = WN_kid_count(expr);

  INT allocated_results_nb = 2;
  INT allocated_opnds_nb = 10;
  *numrests = 0;
  *numopnds = 0;

  *res = TYPE_MEM_POOL_ALLOC_N(TN*, Malloc_Mem_Pool,allocated_results_nb);  
  *opnds = TYPE_MEM_POOL_ALLOC_N(TN*, Malloc_Mem_Pool,allocated_opnds_nb);  

#define CHECK_RESULTS(results_nb)					\
  if((results_nb) > allocated_results_nb) {				\
    INT new_size = allocated_results_nb*2;				\
    while ((results_nb) > new_size) {					\
      new_size *= 2;							\
    }									\
    *res = TYPE_MEM_POOL_REALLOC_N(TN*, Malloc_Mem_Pool,*res,allocated_results_nb,new_size); \
    allocated_results_nb = new_size;					\
  }

#define CHECK_OPNDS(opnds_nb)						\
  if((opnds_nb) > allocated_opnds_nb) {					\
    INT new_size = allocated_opnds_nb*2;				\
    while ((opnds_nb) > new_size) {					\
      new_size *= 2;							\
    }									\
    *opnds = TYPE_MEM_POOL_REALLOC_N(TN*, Malloc_Mem_Pool,*opnds,allocated_opnds_nb,new_size); \
    allocated_opnds_nb = new_size;					\
  }

  TYPE_ID result_mtype = WN_rtype(expr);
  
#ifdef TARG_ST  
  // This function is now common to intrinsic op and intrinsic call.
  // For intrinsic call, the actual result may be void in the following cases:
  // 1. the intrinsic call is a void result
  // 2. the intrinsic call is a non-void result but the actual tree as
  // been forced to MTYPE_V because the result is not used.
  // For the second case we must anyway create an artificial result
  // because the intrinsic expaansion function must have all results
  // available as TNs even if they are dead.
  if (WN_operator_is(expr,OPR_INTRINSIC_CALL) &&
      rkind != IRETURN_UNKNOWN && rkind != IRETURN_V && 
      WN_rtype(expr) == MTYPE_V) {
    // The node was forced to a VOID rtype.
    // Get the result mtype from the intrinsic description
    result_mtype = INTRN_mtype_for_return_kind(rkind);
  }
#endif

#ifdef TARG_ST
  //
  // Handling dynamic mtype (whether composed or not)
  // for INTRINSIC_CALL. 
  //
  // For INTRINSIC_OP, see below.
  //
  if (MTYPE_is_dynamic(WN_rtype(expr)) &&
      WN_operator_is(expr,OPR_INTRINSIC_CALL)) {
   
     proto_intrn_info_t *intr_call_info;
     mUINT32             i;
     mUINT32             in = 0;
     mUINT32             out= 0;

     intr_call_info = INTRN_proto_info((const INTRINSIC) WN_intrinsic(expr));

     // In/out parameters are counted both in
     // *numopnds and in *numrests.
    *numopnds = INTRN_number_of_in_param(intr_call_info);
    *numrests = INTRN_number_of_out_param(intr_call_info);

     CHECK_RESULTS(*numrests);
     CHECK_OPNDS(*numopnds);

     // If an in/out *real* parameter is described as consisting
     // of two parameters (one IN and one OUT), we either use the
     // same TN (if both parameter correspond to the same data)
     // or 2 differents TNs.
     // In a following step, the Expand_Intrinsic will be able
     // to detect the "same_res" constraint and generate a copy
     // if necessary (so if 2 TNs were generated).
     for(i=0;i<intr_call_info->argument_count;++i) {

        if(INTRN_is_in_param(i,intr_call_info)) {
           (*opnds)[in] = Expand_Expr(WN_kid(expr,in), expr, NULL);

           if(INTRN_is_inout_param(i,intr_call_info)) {
             (*res)[out] = Get_Intrinsic_Call_Dedicated_Tn(expr,out);
	     if(NULL==(*res)[out]) {
	       if (Is_Intrinsic_InOut_Param_Single_TN(expr, out, (*opnds)[in])) {
		 // Use same TN for both input and output
		 (*res)[out] = (*opnds)[in];
	       } else {
		 // Insure that source TN will not be modified
		 (*res)[out] = Build_TN_Of_Mtype(intr_call_info->arg_type[i]);
	       }
	     }
            ++out;
            }
           ++in;

         } else if (!INTRN_is_inout_param(i,intr_call_info)) {
           // Looking for a dedicated tn (forward pass). If we haven't
           // found a dedicated one, we create a "normal" one.
           (*res)[out] = Get_Intrinsic_Call_Dedicated_Tn(expr,out);
           if(NULL==(*res)[out])
             (*res)[out] = Build_TN_Of_Mtype(intr_call_info->arg_type[i]);
           ++out;
         }                                  // End if else if...
     }                                      // End for

     // Special case: we have to manage the functional form
     // Not very clean: functional and procedural forms for intrinsic
     // call are stuff for front-end.
     if(INTRN_return_type(intr_call_info) != MTYPE_V &&
        !MTYPE_is_composed(INTRN_return_type(intr_call_info))) {
           (*numrests)++;
           CHECK_RESULTS(*numrests);
           FmtAssert(*numrests==1,
              ("cannot mix functional and procedural form for intrinsic call"));

           (*res)[out] = Get_Intrinsic_Call_Dedicated_Tn(expr,out);
           if(NULL==(*res)[out])
             (*res)[out] = Build_TN_Of_Mtype(INTRN_return_type(intr_call_info));
           ++out;
         }


   } else
#endif
#ifdef ENABLE_64_BITS
  if (Enable_64_Bits_Ops) {
    if (rkind != IRETURN_UNKNOWN) {
      if (*result == NULL && rkind != IRETURN_V) {
	*result = Build_TN_Of_Mtype (result_mtype);
      }

      if(rkind != IRETURN_V) {
	if (TN_size(*result) == 8) {
	  *numrests = 2;
	  CHECK_RESULTS(*numrests);
	  (*res)[0] = Build_TN_Of_Mtype (MTYPE_U4);
	  (*res)[1] = Build_TN_Of_Mtype (MTYPE_U4);
	} else {
	  *numrests = 1;
	  CHECK_RESULTS(*numrests);
	  (*res)[0] = *result;
	}
      }

      *numopnds = 0;
      for (i = 0; i < numkids; i++) {
	TN *kid = Expand_Expr(WN_kid(expr,i), expr, NULL);
	if (TN_size(kid) == 8) {
	  CHECK_OPNDS((*numopnds)+2);
	  (*opnds)[*numopnds] = Build_TN_Of_Mtype (MTYPE_U4);
	  (*opnds)[(*numopnds)+1] = Build_TN_Of_Mtype (MTYPE_U4);
	  Expand_Extract((*opnds)[*numopnds], (*opnds)[(*numopnds)+1], kid, &New_OPs);
	  (*numopnds) += 2;
	} else {
	  CHECK_OPNDS((*numopnds)+1);
	  (*opnds)[*numopnds] = kid;
	  (*numopnds) += 1;
	}
      }
    }
  } else
#endif
  if (Only_32_Bit_Ops && 
      (MTYPE_is_longlong(WN_rtype(expr)) || MTYPE_is_double(WN_rtype(expr))))
  {

    // This intrinsic op should has been preprocessed so that a pair
    // of result values that it returns are parameters.
    Is_True(*result == NULL, ("result set for 64 bit intrinsic"));
    //
    // There are 2 results normally:
    //
    *numrests = 2;
    CHECK_RESULTS(*numrests);
    for (i = 0; i < *numrests; i++) {
      (*res)[i] = Expand_Expr(WN_kid(expr,i), expr, NULL);
    }
    //
    // The rest of kids are operands
    //
    *numopnds = numkids - *numrests;
    CHECK_OPNDS(*numopnds);
    for (i = 0; i < *numopnds; i++) {
      (*opnds)[i] = Expand_Expr(WN_kid(expr,i+*numrests), expr, NULL);
    }
  }
  else if (rkind != IRETURN_UNKNOWN) {
    if (*result == NULL &&  rkind != IRETURN_V) {
      *result = Build_TN_Of_Mtype (result_mtype);
    }

    if (rkind != IRETURN_V) {
      *numrests = 1;
    }

    CHECK_RESULTS(*numrests);
    (*res)[0] = *result;
    //
    // All kids are operands
    //
    *numopnds = numkids;
    CHECK_OPNDS(*numopnds);
    for (i = 0; i < *numopnds; i++) {
      (*opnds)[i] = Expand_Expr(WN_kid(expr,i), expr, NULL);
    }
  }

#undef CHECK_RESULTS
#undef CHECK_OPNDS
}


/* ======================================================================
 *   Handle_INTRINSIC_OP
 * ======================================================================
 */
static TN*
Handle_INTRINSIC_OP (WN *expr, TN *result)
{
  INT i;
  INTRINSIC id = (INTRINSIC) WN_intrinsic (expr);
  INTRN_RETKIND rkind = INTRN_return_kind(id);
  INT numkids = WN_kid_count(expr);
  TN  **kids;
  TN  **res;
  INT numopnds = 0;
  INT numrests = 0;

  // It can only be a cg_intrinsic and there always is a corresponding
  // OPs sequence for it or else we're generating Lai_Code !
  FmtAssert(Lai_Code || INTRN_cg_intrinsic(id), 
                          ("Handle_INTRINSIC_OP: not a cg INTRINSIC_OP"));
  if (Trace_Exp) {
    fprintf(TFile, "Handle_INTRINSIC_OP %s\n", INTRN_c_name(id));
  }


  Get_Intrinsic_Op_Parameters( expr, &result, &kids, &numopnds, &res, &numrests );

#ifdef TARG_ST

  FmtAssert(Inline_Intrinsics_Allowed,("inlining intrinsics not allowed"));

  if (Trace_Exp) {
    fprintf(TFile, "exp_intrinsic_op %s: ", INTRN_c_name(id));
    for (i = 0; i < numrests-1; i++) {
      Print_TN(res[i], FALSE);
      fprintf(TFile, ", ");
    }
    if(numrests > 0) {
      Print_TN(res[numrests-1], TRUE);
    }
    fprintf(TFile, " :- ");
    for (i = 0; i < numopnds-1; i++) {
      Print_TN(kids[i], FALSE);
      fprintf(TFile, ", ");
    }
    if(numopnds > 0) {
      Print_TN(kids[numopnds-1], FALSE);
    }
    fprintf(TFile, "\n");
  }

  /* for debuggging */
  OP *Last_OP = OPS_last(&New_OPs);
  BB *Last_BB = Cur_BB;

  Exp_Intrinsic_Op (id, numrests, numopnds, res, kids, &New_OPs, current_srcpos);

#ifdef ENABLE_64_BITS
  if (Enable_64_Bits_Ops) {
    if (TN_size(result) == 8) {
      Expand_Compose(result, res[0], res[1], &New_OPs);
    }
  }
#endif

#else // !TARG_ST
  kids[0] = Expand_Expr(WN_kid0(expr), expr, NULL);
  kids[1] = (numkids > 1) ? Expand_Expr(WN_kid1(expr), expr, NULL) : NULL;
  kids[2] = (numkids > 2) ? Expand_Expr(WN_kid2(expr), expr, NULL) : NULL;
  FmtAssert(numkids <= 3, ("unexpected number of kids in intrinsic_op"));

  if (rkind != IRETURN_UNKNOWN && result == NULL) {
    result = Allocate_Result_TN(expr, NULL);
  }

  if (Trace_Exp) {
    fprintf(TFile, "exp_intrinsic_op %s: ", INTRN_c_name(id));
    Print_TN(result, FALSE);
    fprintf(TFile, " :- ");
    for (i = 0; i < numkids-1; i++) {
      Print_TN(kids[i], FALSE);
      fprintf(TFile, ", ");
    }
    Print_TN(kids[numkids-1], FALSE);
    fprintf(TFile, "\n");
  }

  /* for debuggging */
  OP *Last_OP = OPS_last(&New_OPs);

  Exp_Intrinsic_Op (id, 1, numkids, &result, kids, &New_OPs);
#endif

  if (Trace_Exp) {
    OP *op;
    if (Last_OP) op = OP_next(Last_OP);
    else op = OPS_first(&New_OPs);

    while (op != NULL) {
      fprintf(TFile, " into "); Print_OP (op);
      op = OP_next(op);
    }
  }

  MEM_POOL_FREE(Malloc_Mem_Pool, res);
  MEM_POOL_FREE(Malloc_Mem_Pool, kids);

  return result;
}

/* ======================================================================
 *   Get_Non_Local_Label_Name 
 *
 *   Return name for nonlocal label, given level and label number.
 *   Must be fixed name across PU's; the goto won't find the real
 *   label_idx in the parent pu, it will just have the level and index.
 * ======================================================================
 */
static STR_IDX
Get_Non_Local_Label_Name (SYMTAB_IDX level, LABEL_IDX index)
{
#ifdef TARG_ST
  // [CG] We have BB_Label_Name_Scope_Index in config_asm.h for ST targets
  char name[128];
  sprintf(name, BB_Label_Name_Scope_Index, "nonlocal", level, index);
#else
	// create special label name .Lnonlocal.<level>.<index>
  	char *name = (char *) alloca (11 + 1 + 8 + 1 + 8 + 1);
	sprintf(name, ".Lnonlocal%s%d%s%d", Label_Name_Separator, 
		level, Label_Name_Separator, index);
#endif
	return Save_Str(name);
}

// Get LABEL from whirl node.
// It is much simpler since now we don't need to create STs for
// label numbers.  The only thing we might need to do is create a
// LABEL_name.
#ifdef CGG_ENABLED
CGG_STATIC LABEL_IDX
#else
static LABEL_IDX
#endif
Get_WN_Label (WN *wn)
{
  LABEL_IDX label = WN_label_number(wn);
  char *name;
  char *label_prefix = NULL;

  FmtAssert (OPCODE_has_label(WN_opcode(wn)),
	("Get_WN_Label: invalid opcode %d ", WN_opcode(wn)));
  FmtAssert (label > 0 && label <= LABEL_Table_Size(CURRENT_SYMTAB),
	("Get_WN_Label: label %d greater than last label %d", 
		label, LABEL_Table_Size(CURRENT_SYMTAB)));
#ifdef TARG_ST
  // [CG] Simplified label generation. 
  if (LABEL_name_idx(label) == 0) {
    STR_IDX label_name_idx;
    if (LABEL_target_of_goto_outer_block(label)) {
      label_name_idx = Get_Non_Local_Label_Name (CURRENT_SYMTAB, label);
    } else {
      char buffer[128];
      // Same name as  Gen_Temp_Label() in label_util.cxx
      sprintf(buffer, BB_Label_Name_Scope_Index, "", 
	      Current_PU_Count(), label);
      label_name_idx = Save_Str(buffer);
    }
    Set_LABEL_name_idx (Label_Table[label], label_name_idx);
  }
#else
  if (LABEL_name_idx(label) == 0) {
	if (LABEL_target_of_goto_outer_block(label)) {
		Set_LABEL_name_idx (Label_Table[label], 
			Get_Non_Local_Label_Name (CURRENT_SYMTAB, label) );
	}
	else {
	  label_prefix = ".Lt";
	}
  }
  else if (isdigit(LABEL_name(label)[0])) {
	// prefix with .L so .s file will be legal,
	  label_prefix = ".L";
  }
  if (label_prefix != NULL) {
	// create label name:  prefix<name>.<pu-number>.<label-index>
	// pu and index number assure is unique across file
	char *oldname = (LABEL_name_idx(label) == 0 ? "" : LABEL_name(label));
  	name = (char *) alloca (strlen(label_prefix) + strlen(oldname) 
		+ 1 + 8 + 1 + 8 + 1);
	sprintf(name, "%s%s%s%d%s%d", label_prefix, oldname,
		Label_Name_Separator, Current_PU_Count(), 
		Label_Name_Separator, label);
	Set_LABEL_name_idx (Label_Table[label], Save_Str(name));
  }
#endif
  return label;
}

/* ====================================================================
 *   Expand_Expr
 *
 * Expand a WHIRL expression into a sequence of OPs. Return the result
 * TN. The caller can specify a 'result' TN if already known.
 *
 * The 'parent' WHIRL node for the expression is passed for cases where
 * the context is needed for expansion. Currently, this is used for
 * the following:
 *    - to determine if an INTCONST can be used as an immediate field 
 *      in the parent OP. 
 *    - to determine if an LDA is used directly in a CALL node.
 *    - to determine if a truncation CVT can be eliminated
 *
 * The 'parent' node maybe set to NULL to disable the above checks. It
 * is also NULL for cases where there is no parent (i.e. statement
 * level nodes).
 * ====================================================================
 */
#if defined(CGG_ENABLED)
static TN *
Expand_Expr_local (
#else
static TN *
Expand_Expr (
#endif

  WN *expr, 
  WN *parent, 
  TN *result
)
{
  OPCODE opcode;
  OPERATOR opr;
  INT num_opnds;
  INT i;
  TN *opnd_tn[OP_MAX_FIXED_OPNDS]; // we shouldn't be handling variable OPs
  TN *const_tn;
  TOP top;
  INT const_operands = 0;

  opcode = WN_opcode (expr);
  opr = WN_operator(expr);

  PU_WN_Cnt++;
#if Is_True_On
  if (WN_rtype(expr) == MTYPE_C4 ||
      WN_rtype(expr) == MTYPE_C8 ||
      WN_rtype(expr) == MTYPE_CQ) 
  {
    ErrMsg (EC_Unimplemented, "Expand_Expr: COMPLEX");
  }
  if (opcode == OPC_MLOAD || opcode == OPC_MSTORE) {
    ErrMsg (EC_Unimplemented, "Expand_Expr: MLOAD/MSTORE");
  }
#endif

  top = WHIRL_To_TOP (expr);

  if (Trace_Exp) {
    fprintf(TFile, "<whirl>: %s [%s]\n", OPCODE_name(opcode),
               TOP_Name(top));
  }

  if (TOP_is_noop(top)
	&& (opr == OPR_PAREN || opr == OPR_TAS || opr == OPR_PARM)) 
  {
    /* For TAS nodes, if the new opcode is noop, we can ignore the TAS.
     * For PAREN and PARM nodes, ignore it for now.
     */
    return Expand_Expr (WN_kid0(expr), parent, result);
  }
  /* get #opnds from topcode or from #kids of whirl
   * (special cases like store handled directly). */
  if (top != TOP_UNDEFINED) {
    num_opnds =   ISA_OPERAND_INFO_Operands(ISA_OPERAND_Info(top))
		- (TOP_is_predicated(top) != 0);
  } else {
    num_opnds = OPCODE_nkids(opcode);
  }

  FmtAssert(num_opnds <= OP_MAX_FIXED_OPNDS, 
                           ("too many operands (%d)", num_opnds));

#ifdef TARG_ST
  //
  // Arthur: for floating-point constants, it is not always
  //         necessary to keep them in memory. On ST targets,
  //         for example, a floating-point constant can be
  //         loaded into a integer register in the same way
  //         as an integer constant.
  //
  //         In WHIRL, however, only INTCONST opcode allows not
  //         to have an associated symbol. There are two 
  //         possibilities:
  //
  //          1. generate CONST for floating-point constants
  //             as usual but not allocate memory here and
  //             handle expand into a proper sequence of bits;
  //          2. make front-end generate F4TAS(I4INTCONST)
  //             something like this
  //
  //         Here we try the first possibility, we don't want
  //         to loose that high-level information on the
  //         constant too early. Is this good ?
  //

  if (OPCODE_has_sym(opcode) && WN_st(expr) != NULL) {
    if (opr == OPR_CONST && 
	(!MTYPE_is_float(WN_rtype(expr)) || CG_floating_const_in_memory))
      /* make sure st is allocated */
      Allocate_Object (WN_st(expr));
  }
#else
  if (OPCODE_has_sym(opcode) && WN_st(expr) != NULL) {
    /* make sure st is allocated */
    Allocate_Object (WN_st(expr));
  }
#endif

  /* Setup the operands */
  switch (opr) {

  case OPR_LDID:
    return Handle_LDID (expr, result, opcode);

#ifdef TARG_ST
  case OPR_SUBPART:
    return Handle_SUBPART(expr, result, opcode);
#endif

  case OPR_LDBITS:
    return Handle_LDBITS (expr, result, opcode);

  case OPR_STID:
    Handle_STID (expr, opcode);
    return NULL;

  case OPR_STBITS:
    Handle_STBITS (expr);
    return NULL;

  case OPR_LDA:
    return Handle_LDA (expr, parent, result, opcode);

  case OPR_LDA_LABEL:
    // create a st that matches the label name
    // that we can then use with a relocation in the lda.
    {
        ST *st = New_ST (CURRENT_SYMTAB);
        ST_Init (st, Save_Str (LABEL_name(Get_WN_Label(expr))),
                 CLASS_NAME, SCLASS_UNKNOWN, EXPORT_LOCAL, WN_ty(expr));
    	opnd_tn[0] = Gen_Symbol_TN (st, 0, 0);
    }
    num_opnds = 1;
    break;

  case OPR_GOTO_OUTER_BLOCK:
    // create a st that matches the label name
    // that we can then use with a relocation in the lda.
    {
        ST *st = New_ST (CURRENT_SYMTAB);
        ST_Init (st, Get_Non_Local_Label_Name (
			WN_label_level(expr), WN_label_number(expr) ),
		CLASS_NAME, SCLASS_UNKNOWN, EXPORT_LOCAL, WN_ty(expr));
    	opnd_tn[0] = Gen_Symbol_TN (st, 0, 0);
    }
    num_opnds = 1;
    break;

  case OPR_ILOAD:
    return Handle_ILOAD (expr, result, opcode);

  case OPR_ILDBITS:
    return Handle_ILDBITS (expr, result, opcode);

  case OPR_EXTRACT_BITS:
    return Handle_EXTRACT_BITS (expr, result, opcode);

  case OPR_ISTORE:
    Handle_ISTORE (expr, opcode);
    return NULL;

  case OPR_ISTBITS:
    Handle_ISTBITS (expr);
    return NULL;

  case OPR_COMPOSE_BITS:
    return Handle_COMPOSE_BITS (expr, result, opcode);

  case OPR_LROTATE:
    return Handle_LROTATE (expr, result, opcode);

  case OPR_RROTATE:
    return Handle_RROTATE (expr, result, opcode);

  case OPR_SELECT:
    return Handle_SELECT(expr, result, opcode);

  case OPR_CALL:
  case OPR_ICALL:
  case OPR_PICCALL:
    Handle_Call_Site (expr, opr);
    return NULL;

  case OPR_CONST:
    if (result == NULL) {
      result = Allocate_Result_TN (expr, NULL);
      if (CGSPILL_Rematerialize_Constants) {
	Set_TN_is_rematerializable(result);
	Set_TN_remat (result, expr);
      }
    }
    opnd_tn[0] = Gen_Symbol_TN (WN_st(expr), 0, 0);
    num_opnds = 1;
    break;

  case OPR_INTCONST:
    /* Operand is a constant. Extract the information from the whirl
     * node and create a constant TN.
     */
    switch (opcode) {

      case OPC_I8INTCONST:
      case OPC_U8INTCONST:

#ifdef EMULATE_LONGLONG
	const_tn = Gen_Literal_TN_Pair((UINT64) WN_const_val(expr));
#else
	const_tn = Gen_Literal_TN (WN_const_val(expr), 8);
#endif
	break;

      case OPC_I4INTCONST:
      case OPC_U4INTCONST:
      case OPC_A4INTCONST:
	/* 
	 * even for U4 we sign-extend the value 
	 * so it matches what we want register to look like 
	 */
	const_tn = Gen_Literal_TN ((INT32) WN_const_val(expr), 4);
	break;

      case OPC_I5INTCONST:
      case OPC_U5INTCONST:
	const_tn = Gen_Literal_TN ((INT32) WN_const_val(expr), 5);
	break;

      case OPC_BINTCONST:
	if (result == NULL) 
	  result = Allocate_Result_TN (expr, NULL);
	Exp_Pred_Set(result, Get_Complement_TN(result), 
                                       WN_const_val(expr), &New_OPs);
	return result;

    default:
      #pragma mips_frequency_hint NEVER
      FmtAssert(FALSE, ("Expand_Expr: %s unhandled", OPCODE_name(opcode)));
      /*NOTREACHED*/
    }

    /* Check if the parent node can take an immediate operand of
     * the given value. If yes, return the const_tn. Otherwise, 
     * load the constant into a register and then use it from there.
     */
    if (Has_Immediate_Operand (parent, expr)) {
      if (Trace_Exp) {
	fprintf(TFile, "has_immed: %s\n", OPCODE_name(WN_opcode(parent)));
	fprintf(TFile, "const_tn: "); 
	Print_TN(const_tn,FALSE);
	fprintf(TFile, "\n");
      }
      return const_tn;
    }

    /* If the constant is in a hardwired register, return the register.
     * No need to generate a LDIMM in that case.
     */
    if (result == NULL) {
      switch (opcode) {
      case OPC_BINTCONST:
	/* Is a boolean 1 available ? */
        if (True_TN && TN_is_const_reg(True_TN) && WN_const_val(expr) == 1) 
          return True_TN;

      case OPC_I8INTCONST:
      case OPC_U8INTCONST:
      case OPC_I5INTCONST:
      case OPC_U5INTCONST:
      case OPC_I4INTCONST:
      case OPC_U4INTCONST:
	/* Is integer zero available ? Ensure that it is of correct 
	 register size for 64 bits. */
	if (Zero_TN && TN_is_const_reg(Zero_TN) && WN_const_val(expr) == 0
	    && TN_size(Zero_TN) == MTYPE_byte_size(OPCODE_rtype(opcode)))
	  return Zero_TN;
      }
    }

    if (CGSPILL_Rematerialize_Constants && result == NULL) {
      result = Allocate_Result_TN (expr, NULL);
      Set_TN_is_rematerializable(result);
      Set_TN_remat (result, expr);
    }

    opnd_tn[0] = const_tn;
    num_opnds = 1;
    break;

  case OPR_CVTL:
    if (Is_CVT_Noop(expr, parent)) {
      return Expand_Expr(WN_kid0(expr), parent, result);
    }
    else {
      opnd_tn[0] = Expand_Expr (WN_kid0(expr), expr, NULL);
      opnd_tn[1] = Gen_Literal_TN (WN_cvtl_bits(expr), 4);
      num_opnds = 2;
    }
    break;

  case OPR_CVT:

    if (Is_CVT_Noop(expr, parent)) {
      return Expand_Expr(WN_kid0(expr), parent, result);
    }
#ifndef TARG_ST
    // [CG] This code is not valid with 32 bits ops or
    // paired 64 bits ops
    else if (Is_CVTL_Opcode(opcode)) {
      opnd_tn[0] = Expand_Expr (WN_kid0(expr), expr, NULL);
      opnd_tn[1] = Gen_Literal_TN (32, 4);
      num_opnds = 2;
    }
#endif
    else
    {
      Is_True(WN_desc(expr) != MTYPE_B || WN_rtype(WN_kid0(expr)) == MTYPE_B,
	      ("rtype of xxBCVT kid is not MTYPE_B"));
      opnd_tn[0] = Expand_Expr (WN_kid0(expr), expr, NULL);
      num_opnds = 1;
    }
    break;	/* already set */

  case OPR_PREFETCH:
  case OPR_PREFETCHX:
    if (Prefetch_Kind_Enabled(expr)) {
      VARIANT variant = V_NONE;
      Set_V_pf_flags(variant, WN_prefetch_flag(expr));
      Last_Mem_OP = OPS_last(&New_OPs);
      Exp_Prefetch (top, 
    	Expand_Expr (WN_kid(expr,0), expr, NULL),
    	(opr == OPR_PREFETCH) ?
		Gen_Literal_TN (WN_offset(expr), 4) :
		Expand_Expr (WN_kid(expr,1), expr, NULL),
        variant,
	&New_OPs);
      Set_OP_To_WN_Map(expr);
    }
    return NULL;

#ifdef TARG_ST
  case OPR_MPY:
    return Handle_MPY (expr, result, opcode);
  case OPR_MADD:
    return Handle_MADD (expr, result, opcode);
#endif

  case OPR_DIVREM:
      return Handle_DIVREM(expr, parent, result, opcode);

  case OPR_DIVPART:
      return Handle_DIVPART(expr, parent, result);

  case OPR_REMPART:
      return Handle_REMPART(expr, parent, result);

  case OPR_MINMAX:
      return Handle_MINMAX(expr, parent, result, opcode);

  case OPR_MINPART:
      return Handle_MINPART(expr, parent, result);

  case OPR_MAXPART:
      return Handle_MAXPART(expr, parent, result);

  case OPR_LNOT:
      return Handle_LNOT(expr, parent, result);

  case OPR_LIOR:
  case OPR_LAND:
      return Handle_LAND_LIOR(expr, parent, result);

  case OPR_ALLOCA:
	return Handle_ALLOCA (expr, result);

  case OPR_DEALLOCA:
	Handle_DEALLOCA (expr);
	return NULL;

  case OPR_INTRINSIC_OP:
    return Handle_INTRINSIC_OP (expr, result);

  default:
    for (i = 0; i < num_opnds; i++) {
      opnd_tn[i] = Expand_Expr (WN_kid(expr, i), expr, NULL);
      /* TODO: verify that the opnd_tn is the right type. */
      if (TN_has_value(opnd_tn[i])) {
#if TODO_MONGOOSE
	/* Enable this check only when cfold is integrated. */
	Is_True (const_operands == 0, 
		("Expand_Expr: cannot have more than 1 constant operand"));
#else
        if (const_operands != 0) {
	  TN *ldimm_tn = Build_TN_Of_Mtype(WN_rtype(WN_kid(expr,i)));
	  Last_Mem_OP = OPS_last(&New_OPs);
	  Exp_OP1 (OPC_I4INTCONST, ldimm_tn, opnd_tn[i], &New_OPs); 
	  Set_OP_To_WN_Map(expr);
	  opnd_tn[i] = ldimm_tn;
	  printf ("Expand_Expr: cannot have more than 1 constant operand\n");
	}
#endif /* TODO_MONGOOSE */
	const_operands++;
      }
    }
    break;
  }

  /* if we need a result, make sure we have a valid result tn */
  if (OPCODE_is_expression(opcode) && (result == NULL)) {
    result = Allocate_Result_TN (expr, opnd_tn);
  }

  /* We now have the opcode, operands and the result of the OP. Call the
   * expander to add the expanded OP to New_OPs.
   */
  Last_Mem_OP = OPS_last(&New_OPs);
  if (num_opnds > 3) {
    ErrMsg (EC_Unimplemented, "Expand_Expr: cannot handle more than 3 opnds");
  }

  if (top != TOP_UNDEFINED) {
    // Build_OP uses OP_opnds to determine # operands, 
    // so doesn't matter if we pass extra unused ops.
    if (TOP_is_predicated(top)) {
      Build_OP (top, result, True_TN, opnd_tn[0], opnd_tn[1], opnd_tn[2], 
		 &New_OPs);
    } else {
      Build_OP (top, result, opnd_tn[0], opnd_tn[1], opnd_tn[2], &New_OPs);
    }
  } else {
    switch (num_opnds) {
    case 0:
      Exp_OP0 (opcode, result, &New_OPs);
      break;
    case 1:
      Exp_OP1 (opcode, result, opnd_tn[0], &New_OPs);
      break;
    case 2:
      Exp_OP2 (opcode, result, opnd_tn[0], opnd_tn[1], &New_OPs);
      break;
    case 3:
      Exp_OP3 (opcode, result, opnd_tn[0], opnd_tn[1], opnd_tn[2], &New_OPs);
      break;
    }
  }

  /* The TN_is_fpu_int is set on the result,when the OP is constructed,
   * according to the property of the opcode. For [d]mtc1, this is not
   * unconditionally true, the value in the integer register may
   * have in fact been a floating value. Such is the case when TAS is
   * involved. Detect the case and reset the flag (note that rather
   * than generating some complicated check, we'll reset the flag in
   * cases where it's already clear).
   */
  if (opr == OPR_TAS) Reset_TN_is_fpu_int(result);

  Set_OP_To_WN_Map(expr);
  return result;
}

/* =======================================================================
 *   Add_Label
 *
 *   Add a label to the current basic block.
 *   This routine returns the bb that the label is attached to.
 * =======================================================================
 */
BB *
Add_Label (
  LABEL_IDX label
)
{
  BB *bb = Start_New_Basic_Block ();
  BB_Add_Annotation (bb, ANNOT_LABEL, (void *)label);
  FmtAssert (Get_Label_BB(label) == NULL,
	("Add_Label: Label %s defined more than once", LABEL_name(label)));
  Set_Label_BB (label,bb);
  return bb;
}

static void
Link_BBs (BB *bb, LABEL_IDX label)
{
  BB      *dst_bb = Get_Label_BB(label);

  FmtAssert(dst_bb != (LABEL_IDX) 0, 
	    ("Build_CFG: Label %s not defined", ST_name(label)));

  Link_Pred_Succ(bb, dst_bb);
}

/* =======================================================================
 *
 *  label_is_external 
 *
 *  The second argument should be a branch with a label_number.
 *  Return TRUE iff the label points outside the REGION of the third argument.
 *  In this case, the first argument should point to the number
 *  of the corresponding exit from the REGION of the third argument.
 *
 * =======================================================================
 */
static 
BOOL label_is_external ( INT *num, WN *wn, BB *bb )
{
  INT32 label_number;
  LABEL_IDX label;
  BB *target_bb;
  RID *rid = BB_rid( bb );
  INT j;
  WN *goto_wn;
  BOOL match;

  label_number = WN_label_number( wn );
  label = label_number;
  target_bb = Get_Label_BB ( label );

  if ( ( target_bb == NULL ) || ( BB_id( target_bb ) < min_bb_id ) ) 
  {
    FmtAssert (rid, ("RID == NULL, label %d doesn't have a matching target",
		     label_number));
    if ( RID_num_exits( rid ) == 1 ) {
      *num = 0;
      return TRUE;
    }
    FmtAssert( RID_num_exits( rid ) > 1,
	      ("found branch to external label when num_exits <= 0") );
    // can't use REGION_search_block because need to return which exit it is
    goto_wn = WN_first( WN_region_exits ( RID_rwn( rid ) ) );
    match = FALSE;
    for ( j = 0; j < RID_num_exits( rid ); j++ ) {
      if ( label_number == WN_label_number( goto_wn ) ) {
	*num = j;
	match = TRUE;
	break;
      }
      goto_wn = WN_next( goto_wn );
    }
    FmtAssert( match, ("no matching label found in REGION exits for "
		       "external label L%d, RGN %d, BB%d",
		       label_number, RID_id(rid), BB_id(bb)));
    return TRUE;
  }

  return FALSE;
}

/* =======================================================================
 *
 *  Prefetch_Kind_Enabled
 *
 *  Test the kind of the prefetch  { {L1,L2}, {load,store} } against
 *  the four CG_enable_pf flags.  wn must be a prefetch whirl node.
 *
 * =======================================================================
 */
BOOL Prefetch_Kind_Enabled( WN *wn )
{
  BOOL is_read, is_write;
  BOOL is_L1, is_L2;
  BOOL z_conf;
  BOOL confidence_match;
  INT32 nz_conf;

  is_read  = WN_pf_read( wn );
  is_write = WN_pf_write( wn );
  is_L1 = ( WN_pf_stride_1L( wn ) != 0 );
  is_L2 = ( WN_pf_stride_2L( wn ) != 0 );

  z_conf  = ( WN_pf_confidence( wn ) == 0 );
  nz_conf =  WN_pf_confidence( wn );
  confidence_match = (    ( z_conf  && CG_enable_z_conf_prefetch )
		       || ( (nz_conf > 1) && CG_enable_nz_conf_prefetch ) );

  if ( confidence_match ) {
    
    if ( is_read && is_L1 )
      return CG_enable_pf_L1_ld;
    
    if ( is_write && is_L1 )
      return CG_enable_pf_L1_st;
    
    if ( is_read && is_L2 )
      return CG_enable_pf_L2_ld;
    
    if ( is_write && is_L2 )
      return CG_enable_pf_L2_st;
  }

  return FALSE;
}

/* =======================================================================
 *
 *  Has_External_Branch_Target
 *
 *  Return TRUE iff some successor block of bb is outside
 *  the current REGION.
 *
 * =======================================================================
 */
BOOL Has_External_Branch_Target( BB *bb )
{
  WN *branch_wn = BB_branch_wn( bb );
  WN *wn;
  INT i,j;

  if ( branch_wn == NULL )
    return FALSE;

  switch ( WN_opcode( branch_wn ) ) {
  case OPC_TRUEBR:
  case OPC_FALSEBR:
  case OPC_GOTO:
    return label_is_external( &j, branch_wn, bb );
  case OPC_REGION_EXIT:
    return TRUE;
  case OPC_COMPGOTO:
    wn = WN_first( WN_kid1( branch_wn ) );
    for ( i = 0; i < WN_num_entries( branch_wn ); i++ ) {
      if ( label_is_external( &j, wn, bb ) )
	return TRUE;
      wn = WN_next( wn );
    }
    if ( WN_kid_count( branch_wn ) == 3 ) {
      if ( label_is_external( &j, WN_kid( branch_wn, 2 ), bb ) )
	return TRUE;
    }
    return FALSE;
  default:
    #pragma mips_frequency_hint NEVER
    FmtAssert( FALSE, ("unexpected opcode in Has_External_Branch_Target") );
    /*NOTREACHED*/
  }
}

/* =======================================================================
 *
 *  Has_External_Fallthru
 *
 *  Return TRUE iff bb can fall thru to a block outside
 *  the current REGION.
 *
 * =======================================================================
 */
BOOL Has_External_Fallthru( BB *bb )
{
  WN *branch_wn = BB_branch_wn( bb );

  if ( BB_exit( bb ) )
    return FALSE;

  if ( ( branch_wn == NULL )
       || ( WN_opcode( branch_wn ) == OPC_TRUEBR ||
	    WN_opcode( branch_wn ) == OPC_FALSEBR ) ) {
    return ( BB_next( bb ) == NULL );
  }

  switch ( WN_opcode( branch_wn ) ) {
  case OPC_GOTO:
  case OPC_COMPGOTO:
    return FALSE;
  case OPC_REGION_EXIT:
    return TRUE;
  default:
    #pragma mips_frequency_hint NEVER
    FmtAssert( FALSE, ("unexpected opcode in Has_External_Fallthru") );
    /*NOTREACHED*/
  }
}

/* Build the control flow graph for the code generator. */
static void Build_CFG(void)
{
  BB *bb;
  WN *branch_wn, *wn;
  INT i, num;
  RID *rid;
  OP *br_op;
  TN *target_tn;
  LABEL_IDX label;

  // Recompute the list of exits, since with regions
  // some initial exit blocks may get later optimized away.
  while (Exit_BB_Head) {
    Exit_BB_Head = BB_LIST_Delete(BB_LIST_first(Exit_BB_Head), Exit_BB_Head);
  }

  for (bb = REGION_First_BB; bb != NULL; bb = BB_next(bb)) {

    if (BB_exit(bb)) {
	Exit_BB_Head = BB_LIST_Push (bb, Exit_BB_Head, &MEM_pu_pool);
    }

    rid = BB_rid(bb);
    if ( rid && RID_level(rid) >= RL_CGSCHED ) {
      /* This block has already been through this process
	 If it is a REGION exit, but not a PU exit, it may not
	 have been linked to all its successors. */
      if ( BB_REGION_Exit( bb, rid ) != NO_REGION_EXIT && !BB_exit( bb ) ) {
	br_op = BB_branch_op( bb );
	if ( br_op == NULL ) {
	  if ( BB_next( bb ) && !BB_exit( bb ) )
	    /* for some reason there is an empty block after a return block
	       but this should not really be a successor */
	    Link_Pred_Succ ( bb, BB_next( bb ) );
	} else {
	  if ( OP_cond( br_op ) ) {
	    if ( BB_next( bb ) )
	      Link_Pred_Succ ( bb, BB_next( bb ) );
	  }
	  /* Is the target tn always operand 0? */
	  target_tn = OP_opnd(br_op, OP_find_opnd_use(br_op, OU_target));
	  FmtAssert( TN_is_label( target_tn ),
		    ("target of branch is not a label") );
	  label = TN_label( target_tn );
	  if ( Get_Label_BB ( label ) != NULL )
	    Link_BBs( bb, label );
	}
      }
      continue;
    }

    if (BB_exit(bb)) {
      /* There are no successors for a procedure exit. */
      continue;
    }

    branch_wn = BB_branch_wn(bb);
    if (branch_wn != NULL) {
      switch (WN_opcode(branch_wn)) {
      case OPC_FALSEBR:
      case OPC_TRUEBR:
	/* The conditional branch may have expanded into an unconditional
	 * branch or a NOOP (see pv336306 and pv338171). Make sure we still
	 * have a conditional branch!
	 */
	{
	  OP *br_op = BB_branch_op( bb );
	  if ( br_op == NULL || OP_cond( br_op ) ) {
	    if ( ! Has_External_Fallthru( bb ) ) {
 	      Link_Pred_Succ(bb, BB_next(bb));
            }
#ifdef KEY
// Generally we have num_exits==0 while creating cgrin, so CGRIN_exits is
// NULL. With exceptions disabled, PU_has_region is generally not set for
// any PU, hence while creating the bb, we assign NULL to BB_rid. So, without
// exceptions we have rid==0 here. With exceptions we set BB_rid, but 
// num_exits is still 0, so we need the extra check.
	    else if ( rid && CGRIN_exits ( RID_cginfo( rid ) ) ) 
#else
	    else if ( rid ) 
#endif
            {
	      label_is_external( &num, branch_wn, bb );
	      CGRIN_exit_i( RID_cginfo( rid ), num) = bb;
	      CGRIN_exit_label_i( RID_cginfo( rid ), num) = 0;
	    }

	    if ( br_op == NULL ) break;
	  }
	}
	/* fall through and link the branch target. */
      case OPC_GOTO:
      case OPC_REGION_EXIT:
	if ( ! label_is_external( &num, branch_wn, bb ) ) { /*internal label*/
	  Link_BBs(bb, Get_WN_Label(branch_wn));
	} else if ( rid ) {
	  WN *new_exit;
	  LABEL_IDX new_label;
	  CGRIN *cgrin = RID_Find_Cginfo(bb);
	  Is_True(cgrin != NULL,("Build_CFG, null cginfo"));
	  CGRIN_exit_i( cgrin, num ) = bb;
	  label = Get_WN_Label( branch_wn );
	  CGRIN_exit_label_i( cgrin, num ) = label;
	  new_label = REGION_Exit_Whirl_Labels(
		      CGRIN_exit_glue_i(cgrin, num), bb, label, rid);
	  /* write this new label into an exit block that will
	     eventually become the exit block for a new inner region */
	  Is_True(new_label != (LABEL_IDX) 0,
		  ("Build_CFG, new region exit label is NULL"));
	  new_exit = WN_CreateRegionExit(new_label);
	  WN_INSERT_BlockLast(CGRIN_nested_exit(cgrin),new_exit);
	}
	break;
      case OPC_COMPGOTO:
	if (WN_kid_count(branch_wn) == 3) { /* default case */
	    Link_BBs (bb, Get_WN_Label(WN_kid(branch_wn,2)));
	} /* fall thru */
      case OPC_XGOTO:
	wn = WN_first(WN_kid1(branch_wn));	/* first goto */
	for (i = 0; i < WN_num_entries(branch_wn); i++) {
	  if ( ! label_is_external( &num, wn, bb ) ) {
	    Link_BBs (bb, Get_WN_Label(wn));
	  } else if ( rid ) {
	    CGRIN_exit_i( RID_cginfo( rid ), num ) = bb;
	    label = Get_WN_Label( wn );
	    CGRIN_exit_label_i( RID_cginfo( rid ), num ) = label;
	    REGION_Exit_Whirl_Labels(
		   CGRIN_exit_glue_i(RID_cginfo(rid), num), bb, label, rid);
	  }
	  wn = WN_next(wn);
	}
	if (WN_kid_count(branch_wn) == 3) {
	  if ( ! label_is_external( &num, WN_kid( branch_wn, 2 ), bb ) )
	    /* default case */
	    Link_BBs (bb, Get_WN_Label(WN_kid(branch_wn,2)));
	  else if ( rid ) {
	    CGRIN_exit_i( RID_cginfo( rid ), num ) = bb;
	    label = Get_WN_Label( WN_kid( branch_wn, 2 ) );
	    CGRIN_exit_label_i( RID_cginfo( rid ), num ) = label;
	    REGION_Exit_Whirl_Labels(
		   CGRIN_exit_glue_i(RID_cginfo(rid), num), bb, label, rid);
	  }
	}
	break;
      case OPC_AGOTO:
	{
	  BB *targ;
	  for (targ = REGION_First_BB; targ != NULL; targ = BB_next(targ)) {
	    if (BB_Has_Addr_Taken_Label(targ)) Link_Pred_Succ(bb, targ);
	  }
	}
      }
    } 
    else if (BB_next(bb) != NULL) {
      if (BB_rid(BB_next(bb)) != BB_rid(bb)
	  && BB_rid(BB_next(bb)) != NULL
	  && CGRIN_entry(RID_cginfo(BB_rid(BB_next(bb)))) != BB_next(bb)) {
	BB *region_entry = CGRIN_entry(RID_cginfo(BB_rid(BB_next(bb))));
	ANNOTATION *ant = ANNOT_Get (BB_annotations(region_entry), ANNOT_LABEL);
	OPS ops;
	OPS_Init(&ops);
	DevWarn("first bb in region %d is not the entry bb", 
		RID_id(BB_rid(BB_next(bb))));
	if (ant != NULL) {
	  label = ANNOT_label(ant);
	} else {
	  label = Gen_Temp_Label();
	  BB_Add_Annotation (region_entry, ANNOT_LABEL, (void *)label);
	  Set_Label_BB (label,region_entry);
	}
	target_tn = Gen_Label_TN (label, 0);
	Exp_OP1 (OPC_GOTO, NULL, target_tn, &ops);
	BB_Append_Ops(bb, &ops);
	Link_Pred_Succ (bb, region_entry);
      } 
      else if (BB_call(bb)
	&& WN_Call_Never_Return( CALLINFO_call_wn(ANNOT_callinfo(
		ANNOT_Get (BB_annotations(bb), ANNOT_CALLINFO) ))) )
      {
	continue;	// no successor
      } 
      else {
#ifdef TARG_ST
        // (cbr) call to Unwind_Resume is followed by a end_eh_range bblock.
        // will not be caught on preceding test. catch it here.
        if (BB_has_label(bb)) {
          ANNOTATION *ant;
          for (ant = ANNOT_First(BB_annotations(bb), ANNOT_LABEL);
               ant != NULL;
               ant = ANNOT_Next(ant, ANNOT_LABEL)) {
            LABEL_IDX lab = ANNOT_label(ant);
            if (LABEL_kind(Label_Table[lab]) == 
                LKIND_END_EH_RANGE) {
              BB *pred = BB_prev(bb);
              if (BB_call(pred) && WN_Call_Never_Return(CALLINFO_call_wn(ANNOT_callinfo(ANNOT_Get (BB_annotations(pred), ANNOT_CALLINFO)))))
                {
                  if (BB_Is_Unique_Predecessor (bb, pred)) {
                    EXITINFO *exit_info = TYPE_PU_ALLOC (EXITINFO);
                    EXITINFO_srcpos(exit_info) = current_srcpos;
                    BB_Add_Annotation (bb, ANNOT_EXITINFO, exit_info);
                    Set_BB_exit(bb);
                  } 
                }
            }
          }
        }
        if (!BB_exit(bb))
#endif
          Link_Pred_Succ (bb, BB_next(bb));
      }
    }
  }
}

/* ====================================================================
 *   WHIRL_Pick_Compare_Variant
 *
 *   Pick a EQ or NE variant of the given mtype.
 * ====================================================================
 */
static INT16
WHIRL_Pick_Compare_Variant (
  TY_IDX mtype
)
{
  if (mtype == MTYPE_I4 || mtype == MTYPE_U4)
    return V_BR_I4NE;
  //    return (invert) ? V_BR_I4EQ : V_BR_I4NE;
  else if (mtype == MTYPE_I8 || mtype == MTYPE_U8)
    return V_BR_I8NE;
  //    return (invert) ? V_BR_I8EQ : V_BR_I8NE;
  else
    FmtAssert(FALSE,("WHIRL_Pick_Compare_Variant: not handled"));
}

/* ====================================================================
 *   Handle_CONDBR
 *
 *   Handle expansion of a TRUEBR and FALSEBR.
 * ====================================================================
 */
static void
Handle_CONDBR (
  WN *branch
)
{
  WN *condition;
  VARIANT variant;
  TN *operand0, *operand1;
  TN *target_tn;

#ifdef TARG_ST
  condition = WN_kid0 (branch);

  // Inverting a FALSEBR depends on the target implementation.
  // For example, on the ST100 branches are active on FALSE condition
  // and should not be inverted. Thus, inverting should be delegated
  // to the machine dependent Expand_Branch.
  variant = WHIRL_Compare_To_OP_variant (WN_opcode(condition));
#else
  condition = WN_kid0 (branch);
  invert = (WN_opcode(branch) == OPC_FALSEBR);
  variant = WHIRL_Compare_To_OP_variant (WN_opcode(condition), invert);
#endif

  if (variant != V_BR_NONE) {
    operand0 = Expand_Expr (WN_kid0(condition), condition, NULL);
    operand1 = Expand_Expr (WN_kid1(condition), condition, NULL);
  }
  else if (WN_operator_is(condition, OPR_INTCONST)) {
    BOOL cond_is_true = WN_const_val(condition) != 0;
    variant = (cond_is_true) ? V_BR_ALWAYS : V_BR_NEVER;
    operand0 = NULL;
    operand1 = NULL;
  }
  else {
    // if V_BR_NONE and condition is not a constant, it's just
    // a branch targ, fcc:

    operand0 = Expand_Expr (condition, branch, NULL);
    if (WN_rtype(condition) == MTYPE_B) {
      Is_True(   WN_operator_is(condition, OPR_LDID) 
	      && WN_class(condition) == CLASS_PREG,
	      ("MTYPE_B TRUEBR/FALSEBR condition must be preg or relop"));

      operand1 = NULL;
      variant = V_BR_P_TRUE;

#ifndef TARG_ST
      if (invert) {
	PREG_NUM preg2_num = WN_load_offset(condition) + 1;
	operand0 = PREG_To_TN_Array[preg2_num];
      }
#endif
    }
    else if (Zero_TN && TN_is_const_reg(Zero_TN)) { 
      operand1 = Zero_TN;
      variant = WHIRL_Pick_Compare_Variant (WN_rtype(condition));
    }
    else {
      // I will have to do the comparison to the literal TN 0:
      operand1 = Gen_Literal_TN (0, MTYPE_byte_size(WN_rtype(condition)));
      variant = WHIRL_Pick_Compare_Variant (WN_rtype(condition));
    }
  } /* V_BR_NONE */

  // Set the branch is on false:
  if (WN_opcode(branch) == OPC_FALSEBR) Set_V_false_br(variant);

  target_tn = Gen_Label_TN (Get_WN_Label (branch), 0);
  Exp_OP3v (WN_opcode(branch), NULL, target_tn, operand0, operand1, 
	variant, &New_OPs);

  return;
}

/* ====================================================================
 *   Handle_XGOTO
 *
 *   Handle expansion of a XGOTO. 
 * ====================================================================
 */
static void
Handle_XGOTO (
  WN *branch
)
{
  TN *target_tn;
  WN *wn;
  INT i;
  ST *st;
  INITO_IDX ino;
  INITV_IDX inv, prev_inv;

  target_tn = Expand_Expr (WN_kid0(branch), NULL, NULL);
  Exp_Indirect_Branch (target_tn, &New_OPs);

  /* build jump table in init-data list */
  /* get table address */
  st = WN_st(branch);
  BB_Add_Annotation (Cur_BB, ANNOT_SWITCH, st);
  /* make sure st is allocated */
  Allocate_Object(st);
  ino = New_INITO(st);
  prev_inv = INITV_IDX_ZERO;

  wn = WN_first(WN_kid1(branch));	/* first goto */
  for (i = 0; i < WN_num_entries(branch); i++) {
    FmtAssert ((wn && WN_opcode(wn) == OPC_GOTO),
	       ("XGOTO block doesn't have goto's? (%d)", WN_opcode(wn)));
    LABEL_IDX lab = WN_label_number(wn);
    inv = New_INITV();
    INITV_Init_Label (inv, lab);
    prev_inv = Append_INITV (inv, ino, prev_inv);
    wn = WN_next(wn);
  }
}

/* ====================================================================
 *   Convert_Branch
 *
 *   Convert a whirl branch node into a list of expanded OPs. Terminate
 *   the current basic block.
 * ====================================================================
 */
static void
Convert_Branch (
  WN *branch
)
{
  OPCODE opcode;
  TN *target_tn;

  opcode = WN_opcode (branch);
  switch (opcode) {
  case OPC_GOTO:
  case OPC_REGION_EXIT:
    target_tn =  Gen_Label_TN (Get_WN_Label (branch), 0);
    Exp_OP1 (OPC_GOTO, NULL, target_tn, &New_OPs);
    break;
  case OPC_FALSEBR:
  case OPC_TRUEBR:
    Handle_CONDBR (branch);
    break;
  case OPC_XGOTO:
    Handle_XGOTO (branch);
    break;
  case OPC_AGOTO:
    target_tn = Expand_Expr (WN_kid0(branch), NULL, NULL);
    Exp_Indirect_Branch (target_tn, &New_OPs);
    break;
  default:
    #pragma mips_frequency_hint NEVER
    FmtAssert (FALSE, 
               ("Convert_Branch: unexpected opcode %s", OPCODE_name(opcode)));
    /*NOTREACHED*/
  }

  BB_branch_wn(Cur_BB) = branch;
  /* Terminate the basic block. */
  Start_New_Basic_Block ();
}

/* ====================================================================
 *   Handle_Entry
 *
 *   For each procedure entrypoint, add the bb to the list of entry 
 *   points.
 * ====================================================================
 */
static void 
Handle_Entry (WN *entry) 
{
  ST *entry_st;
  BB *entry_bb;
  ENTRYINFO *ent;

  if (WN_Label_Is_Handler_Begin(entry)) {
	// handlers are LABELS, but want function ST in entryinfo.
	// So create a dummy func ST for this pseudo-altentry.
	TY_IDX ty  = Make_Function_Type( MTYPE_To_TY(MTYPE_V));
        PU_IDX pu_idx;
        PU&    pu = New_PU (pu_idx);
        PU_Init (pu, ty, CURRENT_SYMTAB);
        entry_st = New_ST (GLOBAL_SYMTAB);
#ifdef KEY
	char *name = (char *) alloca (strlen("Handler")+1+8+1);
	sprintf (name, "Handler.%d", Current_PU_Count());
        ST_Init (entry_st, Save_Str2i (name, ".", Get_WN_Label(entry)),
                 CLASS_FUNC, SCLASS_TEXT, EXPORT_LOCAL, (TY_IDX) pu_idx);
	PU_Has_Exc_Handler = TRUE;
#else
        ST_Init (entry_st, Save_Str2i ("Handler", ".", Get_WN_Label(entry)),
                 CLASS_FUNC, SCLASS_TEXT, EXPORT_LOCAL, (TY_IDX) pu_idx);
#endif
	Allocate_Object(entry_st);
  }
  else {
    	entry_st = WN_st(entry);
	if (Read_Global_Data && ST_sclass(entry_st) == SCLASS_EXTERN) {
		// this can happen when IPA creates global list of all funcs.
		// reset sclass to be defined
		Set_ST_sclass(entry_st, SCLASS_TEXT);
		Allocate_Object(entry_st);
	}
  }

  /* start new bb, but don't put in list of labels */
  entry_bb = Start_New_Basic_Block ();
  Set_BB_entry (entry_bb);
  ent = TYPE_PU_ALLOC (ENTRYINFO);
  ENTRYINFO_name(ent) = entry_st;
  ENTRYINFO_entry_wn(ent) = entry;
  ENTRYINFO_srcpos(ent) = WN_Get_Linenum(entry);
  current_srcpos = ENTRYINFO_srcpos(ent);
  BB_Add_Annotation (entry_bb, ANNOT_ENTRYINFO, ent);
  Entry_BB_Head = BB_LIST_Push (entry_bb, Entry_BB_Head, &MEM_pu_pool);
}

static void Handle_Return (void)
{
  EXITINFO *exit_info;
  BB *exit_bb;

  exit_bb = Cur_BB;
  Set_BB_exit(exit_bb);
  exit_info = TYPE_PU_ALLOC (EXITINFO);
  EXITINFO_srcpos(exit_info) = current_srcpos;
  BB_Add_Annotation (exit_bb, ANNOT_EXITINFO, exit_info);
  /* Terminate the basic block */
  Start_New_Basic_Block ();
}

#ifdef TARG_ST
static void Handle_EH_Return (TN *stackadj, TN *handler)
{
  if (! PU_Has_EH_Return) {
    PU_Has_EH_Return = TRUE;
  }
  EH_Return_Stackadj_TN = CGTARG_EH_Gen_Return_Stackadj_TN ();
  if (EH_Return_Stackadj_TN) {
    Exp_COPY (EH_Return_Stackadj_TN, stackadj, &New_OPs);
  }
  Exp_COPY (RA_TN, handler, &New_OPs);
  BB *exit_bb = Cur_BB;
  Handle_Return ();
  EXITINFO *exit_info = ANNOT_exitinfo(ANNOT_Get(BB_annotations(exit_bb), ANNOT_EXITINFO));
  EXITINFO_is_eh_return(exit_info) = TRUE;
}
#endif

/* Handle traps (from OP_ASSERT or OP_TRAP) */
static void Handle_Trap(WN *trap) 
{
  Exp_OP1(OPC_TRAP, NULL, Gen_Literal_TN(WN_offset(trap), 4), &New_OPs);
}


// Try to find the load/store following the asm that matches
// the out parameter index.  Return NULL if not found.
// TODO:  search past first stmt for matching store.
// Can be okay to not find anything if out store was optimized away.
static WN*
Find_Asm_Out_Parameter_Load (const WN* stmt, PREG_NUM preg_num, ST** ded_st)
{
  WN* ret_load = NULL;
  for(; stmt != NULL; stmt = WN_next(stmt)) {
#ifdef TARG_ST
    // [CG]: If we encounter another ASM statement we stop the search.
    // Otherwise we will attach the load to the wrong asm statement.
    if (WN_opcode(stmt) == OPC_ASM_STMT) break;
#endif
    if (OPERATOR_is_store(WN_operator(stmt))) {
      WN* load = WN_kid0(stmt);
      OPERATOR opr = WN_operator(load);
      if (opr == OPR_CVT || opr == OPR_CVTL) {
        load = WN_kid0(load);
        opr = WN_operator(load);
      }
      if (OPERATOR_is_load(opr) || opr == OPR_LDA) {
        if (WN_has_sym(load) &&
            WN_class(load) == CLASS_PREG && 
            WN_offset(load) == preg_num) {
          ret_load = load;
          break;
        }
      }
    }
  }
  if (ret_load) {
    if (OPERATOR_has_sym(WN_operator(stmt)) &&
        ST_assigned_to_dedicated_preg(WN_st(stmt))) {
      *ded_st = WN_st(stmt);
    }
  }
  // [CG] It is possible to not find any use of the ASM output
  // if the result is dead but the asm is volatile or has
  // side effect. Thus do not need a warning, and return NULL.
#ifndef TARG_ST
  else {
    DevWarn("didn't find out store for asm preg %d", preg_num);
  }
#endif
  return ret_load;
}

// This function handles ASM statements using the newly built support
// for OPs with variable numbers of results and operands. Unlike
// Handle_Asm, which allocates registers for ASM operands very early,
// here we will only choose TNs for ASM operands, and record their 
// subclasses and early clobber properties as an annotation of the
// OP with TOP_asm opcode. The annotation will also contain the list
// of dedicated TNs that are clobbered by the ASM OP. LRA and GRA
// will use these annotations when assigning registers to ASM operand
// TNs. The replacement of TN names within the ASM string will happen
// much later during cgemit (this is required at least for IA-32 because
// of the need to do FP stack fixup).
//
static void
Handle_ASM (const WN* asm_wn)
{
  // 'result' and 'opnd' below have a fixed size as well as
  // the arrays in ASM_OP_ANNOT. Define here so we can sanity check.
#ifdef TARG_ST
  // [TTh] Max number of operands and results retrieved from gccfe constant
  // Note that arrays in ASM_OP_ANNOT are now dynamically allocated
  // when calling Create_Empty_ASM_OP_ANNOT(nb_res, nb_opnd)
  enum { MAX_OPNDS = MAX_RECOG_OPERANDS, MAX_RESULTS = MAX_RECOG_OPERANDS };
#else
  enum { MAX_OPNDS = 10, MAX_RESULTS = 10 };
#endif

  // these two arrays may have to be reallocatable
#ifdef TARG_ST
  TN* result[MAX_RESULTS];
  TN* opnd[MAX_OPNDS]; 
#else
  TN* result[10];
  TN* opnd[10]; 
#endif
  INT num_results = 0;
  INT num_opnds = 0;

  ISA_REGISTER_SUBCLASS opnd_sc[OP_MAX_FIXED_OPNDS];
  BZERO(opnd_sc, sizeof(opnd_sc));

  PU_Has_Asm = TRUE;

  CGTARG_Init_Asm_Constraints();

#ifdef TARG_ST
  // [TTh] Use new API to allocate ASM_OP_ANNOT. Need to know 
  //       result and operand counts.
  WN* asm_output_constraints = WN_asm_constraints(asm_wn);
  FmtAssert(WN_operator(asm_output_constraints) == OPR_BLOCK,
	    ("asm output constraints not a block?"));
  for (WN* out_pragma = WN_first(asm_output_constraints);
       out_pragma != NULL; 
       out_pragma = WN_next(out_pragma)) {
    num_results++;
  }
  ASM_OP_ANNOT* asm_info = Create_Empty_ASM_OP_ANNOT(num_results, WN_kid_count(asm_wn)-2);
  num_results = 0;
#else
  ASM_OP_ANNOT* asm_info = TYPE_PU_ALLOC(ASM_OP_ANNOT);
  BZERO(asm_info, sizeof(ASM_OP_ANNOT));
#endif

  ASM_OP_wn(asm_info) = asm_wn;

#ifdef TARG_IA32
#if 0  
  // Adding eflags register to the clobber set causes a problem
  // in LRA, because a live range that includes such an ASM OP
  // cannot use eflags register for allocation. Given that we
  // currently don't do any dependence-based transformations for
  // IA-32, it should be safe to ignore Asm_Clobbers_Cc flag.
  //
  if (WN_Asm_Clobbers_Cc(asm_wn)) {
    ASM_OP_clobber_set(asm_info)[ISA_REGISTER_CLASS_eflags] = 
      REGISTER_SET_Union1(REGISTER_SET_EMPTY_SET, REGISTER_MIN);
  }
#endif
#endif

  // process ASM clobber list
  for (const WN* clobber_pragma = WN_first(WN_asm_clobbers(asm_wn));
       clobber_pragma != NULL;
       clobber_pragma = WN_next(clobber_pragma)) {
    Is_True(WN_pragma(clobber_pragma) == WN_PRAGMA_ASM_CLOBBER,
            ("Wrong pragma type for ASM clobber"));
    if (WN_operator(clobber_pragma) == OPR_XPRAGMA) {
      WN* idname = WN_kid0(clobber_pragma);
      Is_True(WN_operator(idname) == OPR_IDNAME,
              ("Wrong kid operator for ASM clobber PREG"));
#ifdef TARG_ST
      TN* tn = PREG_To_TN(WN_st(idname), WN_offset(idname), idname);
#else
      TN* tn = PREG_To_TN(WN_st(idname), WN_offset(idname));
#endif
      FmtAssert(tn && TN_is_register(tn) && TN_is_dedicated(tn),
                ("Wrong TN for PREG from ASM clobber list"));
      ISA_REGISTER_CLASS rc = TN_register_class(tn);
      ASM_OP_clobber_set(asm_info)[rc] = 
        REGISTER_SET_Union1(ASM_OP_clobber_set(asm_info)[rc], TN_register(tn));
    }
  }
  
#ifdef TARG_ST
  OPS reload_ops = OPS_EMPTY;
#endif

  // process ASM output parameters:
  // the out stores must directly follow the ASM,
  // while the constraints are in kid1
#ifdef TARG_ST
  // [TTh] Now retrieved at start of function
#else
  WN* asm_output_constraints = WN_asm_constraints(asm_wn);
  FmtAssert(WN_operator(asm_output_constraints) == OPR_BLOCK,
            ("asm output constraints not a block?"));
#endif
  
  for (WN* out_pragma = WN_first(asm_output_constraints);
       out_pragma != NULL; 
       out_pragma = WN_next(out_pragma)) {

    FmtAssert(num_results < MAX_RESULTS,
	      ("too may asm results in Handle_ASM"));

    FmtAssert(WN_pragma(out_pragma) == WN_PRAGMA_ASM_CONSTRAINT,
              ("not an asm_constraint pragma"));

    const char* constraint = WN_pragma_asm_constraint(out_pragma);
    PREG_NUM preg = WN_pragma_asm_copyout_preg(out_pragma);
    ST* pref_st = NULL;
    WN* load = Find_Asm_Out_Parameter_Load(WN_next(asm_wn), preg, &pref_st);
    TN* pref_tn = NULL;
    if (pref_st) {
#ifdef TARG_ST
      pref_tn = PREG_To_TN(MTYPE_To_PREG(ST_mtype(pref_st)),
                           Find_PREG_For_Symbol(pref_st), load);
#else
      pref_tn = PREG_To_TN(MTYPE_To_PREG(ST_mtype(pref_st)),
                           Find_PREG_For_Symbol(pref_st));
#endif
    }
    ISA_REGISTER_SUBCLASS subclass = ISA_REGISTER_SUBCLASS_UNDEFINED;

    TN* tn = CGTARG_TN_For_Asm_Operand(constraint, load, pref_tn, &subclass);
    if (tn == NULL) {
      ErrMsgSrcpos(EC_CG_Generic_Fatal, current_srcpos, "operand to 'asm' statement does not match constraint");
    }
    ASM_OP_result_constraint(asm_info)[num_results] = constraint;
    ASM_OP_result_subclass(asm_info)[num_results] = subclass;
    ASM_OP_result_position(asm_info)[num_results] = 
      WN_pragma_asm_opnd_num(out_pragma);
    ASM_OP_result_clobber(asm_info)[num_results] = 
      (strchr(constraint, '&') != NULL);
    ASM_OP_result_memory(asm_info)[num_results] = 
      (strchr(constraint, 'm') != NULL);
#ifdef TARG_ST
    // Initialize same res info to -1
    ASM_OP_result_same_opnd(asm_info)[num_results] = -1;
#endif
    
    result[num_results] = tn;
    num_results++;
    
    // in WHIRL store that follows ASM stmt, replace negative
    // negative preg with new_preg mapped to ASM operand TN
    // it is possible that wopt optimized away the output store
    if (load) {

#ifdef TARG_ST
      if (TN_register_class(tn) != Register_Class_For_Mtype(WN_rtype(load))) {
	/* We may need to convert to the correct register class. */
	TN* tmp = Build_RCLASS_TN (Register_Class_For_Mtype(WN_rtype(load)));
	Exp_COPY(tmp, tn, &reload_ops);
	tn = tmp;
      }
#endif

      PREG_NUM new_preg = TN_To_PREG(tn);
      if (new_preg == 0) {
        char preg_name[16];
        sprintf(preg_name,"_asm_result_%d",WN_pragma_asm_opnd_num(out_pragma));
	new_preg = Create_Preg (TY_mtype(ST_type(WN_st(load))), preg_name);
	Realloc_Preg_To_TN_Arrays (new_preg);
        TN_MAP_Set(TN_To_PREG_Map, tn, (void*)new_preg);
        PREG_To_TN_Array[new_preg] = tn;
        PREG_To_TN_Mtype[new_preg] = TY_mtype(ST_type(WN_st(load)));
      }
      WN_offset(load) = new_preg;
    } 

  }

  // process asm input parameters, which are kids 2-n
  for (INT kid = 2; kid < WN_kid_count(asm_wn); ++kid) {
    FmtAssert(num_opnds < MAX_OPNDS,
	      ("too may asm operands in Handle_ASM"));

    WN* asm_input = WN_kid(asm_wn, kid);
    FmtAssert(WN_operator(asm_input) == OPR_ASM_INPUT,
              ("asm kid not an asm_input?"));

    const char* constraint = WN_asm_input_constraint(asm_input);
    WN* load = WN_kid0(asm_input);
    TN* pref_tn = NULL;
    if (OPERATOR_has_sym(WN_operator(load))) {
      ST* pref_st = WN_st(load);
      if (ST_assigned_to_dedicated_preg(pref_st)) {
#ifdef TARG_ST
        pref_tn = PREG_To_TN(MTYPE_To_PREG(ST_mtype(pref_st)),
                             Find_PREG_For_Symbol(pref_st), load);
#else
        pref_tn = PREG_To_TN(MTYPE_To_PREG(ST_mtype(pref_st)),
                             Find_PREG_For_Symbol(pref_st));
#endif
      }
    }
    ISA_REGISTER_SUBCLASS subclass = ISA_REGISTER_SUBCLASS_UNDEFINED;

    TN* tn = CGTARG_TN_For_Asm_Operand(constraint, load, pref_tn, &subclass);
    if (tn == NULL) {
      ErrMsgSrcpos(EC_CG_Generic_Fatal, current_srcpos, "operand to 'asm' statement does not match constraint");
    }

    ASM_OP_opnd_constraint(asm_info)[num_opnds] = constraint;
    ASM_OP_opnd_subclass(asm_info)[num_opnds] = subclass;
    ASM_OP_opnd_position(asm_info)[num_opnds] = WN_asm_opnd_num(asm_input);
    ASM_OP_opnd_memory(asm_info)[num_opnds] = 
      (strchr(constraint, 'm') != NULL);
#ifdef TARG_ST
    // Update result_same_opnd if the operand matches a result
    if (isdigit(*constraint)) {
      INT res_idx = *constraint-'0';
      ASM_OP_result_same_opnd(asm_info)[res_idx] = num_opnds;
    }
#endif

    opnd[num_opnds] = tn;
    num_opnds++;
    
    // we should create a TN even if it's an immediate
    // constraints on immediates are target-specific
    if (TN_is_register(tn)) {
#ifdef TARG_ST
      if (TN_register_class(tn) != Register_Class_For_Mtype(WN_rtype(load))) {
	/* We may need to convert to the corect register class. */
	TN* tmp = Build_RCLASS_TN (Register_Class_For_Mtype(WN_rtype(load)));
	Expand_Expr (load, NULL, tmp);
	Exp_COPY(tn, tmp, &New_OPs);
      }
      else
#endif
      Expand_Expr (load, NULL, tn);
    }
  }

  // now create ASM op
  OP* asm_op = Mk_VarOP(TOP_asm, num_results, num_opnds, result, opnd);
  if (WN_Asm_Volatile(asm_wn)) {
	Set_OP_volatile(asm_op);
  }
  OPS_Append_Op(&New_OPs, asm_op);
  OP_MAP_Set(OP_Asm_Map, asm_op, asm_info);
#ifdef TARG_ST
  // TODO: Determine what ASM_livein/out are ?
  ASMINFO* info = TYPE_PU_ALLOC (ASMINFO);
  ISA_REGISTER_CLASS rc;
  FOR_ALL_ISA_REGISTER_CLASS(rc) {
    ASMINFO_livein(info)[rc] = REGISTER_SET_EMPTY_SET;
    ASMINFO_liveout(info)[rc] = REGISTER_SET_EMPTY_SET;
    ASMINFO_kill(info)[rc] = ASM_OP_clobber_set(asm_info)[rc];
  }
  BB_Add_Annotation(Cur_BB, ANNOT_ASMINFO, info);
  Start_New_Basic_Block ();
  OPS_Append_Ops(&New_OPs, &reload_ops);
#endif
}

#ifdef TARG_ST
static BOOL
WN_Find_Unique_LDID(WN *expr, WN **ldid) {

  if (WN_operator(expr) == OPR_LDID) {
    if (*ldid == NULL) {
      // The first time a variable is seen
      *ldid = expr;
      return TRUE;
    }
    else if (WN_Equiv(*ldid, expr)) {
      // The same variable is used several times
      return TRUE;
    }
    else {
      return FALSE;
    }
  }
  else {
    int i;
    for (i = 0; i < WN_kid_count(expr); i++) {
      if (!WN_Find_Unique_LDID(WN_kid(expr,i), ldid))
	return FALSE;
    }
    return TRUE;
  }
}

// First, check that wn_affirm node references one and only one LDID
// variable. Then, look for the TN associated to this LDID variable
// and generate a copy tn = tn, on which the property 'affirm' is set.
static void
Handle_AFFIRM(WN *wn_affirm) {
  // First, look for the single PREG that should be used in the
  // expression.
  WN *ldid = NULL;
  OP *op_assume = NULL;

  if (Get_Trace(TP_CGEXP, 8)) {
    fprintf(TFile, "<%d> [ASSUME] WN_AFFIRM attached to CGIR OP:\n", Current_PU_Count());
    fdump_tree(TFile, wn_affirm);
  }

  if (!WN_Find_Unique_LDID(wn_affirm, &ldid)) {
    if (OPT_Enable_Warn_Assume)
      DevWarn("__builtin_assume uses more than one variable (line %d). Ignored.",
	      Srcpos_To_Line(WN_Get_Linenum(wn_affirm)));
  }
  else if (ldid == NULL) {
    if (OPT_Enable_Warn_Assume) {
      DevWarn("No variable found in __builtin_assume (line %d). Ignored.", Srcpos_To_Line(WN_Get_Linenum(wn_affirm)));
    }
    if (Get_Trace(TP_CGEXP, 8))
      fprintf(TFile, "*** Cannot analyze WHIRL node WN_AFFIRM ***\n", Current_PU_Count());
  }
  else {
    TN *result = Handle_LDID(ldid, NULL, WN_opcode (ldid));
    Exp_COPY(result, result, &New_OPs);
    op_assume = OPS_last(&New_OPs);
    OP_Set_Affirm(op_assume, wn_affirm);
    if (Get_Trace(TP_CGEXP, 8))
      Print_OP_No_SrcLine(op_assume);
  }
}
#endif

// replace all occurrences of match string with new string in s string.
static void
Replace_Substring (char *s, char *match, char *newstr)
{
  // need temp buffer since will modify s string
  char *buf = (char*) alloca(strlen(s)+16);
  // iterate until no matches
  while (TRUE) {
	char *p = strstr (s, match);
	if (p == NULL) {	// no match this time
        	return; 	
	}
	char *match_end = p + strlen(match);
	*p = '\0';
	sprintf(buf, "%s%s%s", s, newstr, match_end);
	strcpy(s,buf);
  }
}

static void
Modify_Asm_String (char *asm_string, INT pattern_index, TN *tn, char *tn_name)
{
  char pattern[4];
  sprintf(pattern, "%%%c", '0'+pattern_index);	// %N
  Replace_Substring (asm_string, pattern, tn_name);

  if (tn && TN_is_register(tn)) {
    for (INT i = 0; i < CGTARG_Num_Asm_Opnd_Modifiers; i++) {
      char modifier = CGTARG_Asm_Opnd_Modifiers[i];
      sprintf(pattern, "%%%c%c", modifier, '0'+pattern_index);
      char* mod_name = (char*) CGTARG_Modified_Asm_Opnd_Name(modifier, tn, tn_name);
      Replace_Substring (asm_string, pattern, mod_name);
    }
  }
}

/* Try to find and return a non-zero SRCPOS for the loop starting
 * with <body_label>.  (LABELs apparently don't have linenum info.) 
 * Since this is only for notes/debugging purposes, give up fairly
 * easily.
 */
static SRCPOS get_loop_srcpos(WN *body_label)
{
  if (current_srcpos) {
    return current_srcpos;
  } else if (WN_linenum(body_label)) {
    return WN_linenum(body_label);
  } else {
    WN *wn = WN_next(body_label);
    while (wn && WN_linenum(wn) == 0 && WN_opcode(wn) != OPC_LABEL)
      wn = WN_next(wn);
    return wn ? WN_linenum(wn) : 0;
  }
}

/* ====================================================================
 *   Expand_Statement
 *
 *   Expand a WHIRL statement into a list of OPs and add 
 *   them to the current basic block.
 * ====================================================================
 */
#if defined(CGG_ENABLED)
void
Expand_Statement_local (
#else
static void
Expand_Statement (
#endif

  WN *stmt
) 
{
  BB *bb;
  WN *loop_info;
  LOOPINFO *info = NULL;
  TN *trip_tn;
  OPCODE opc = WN_opcode(stmt);

  PU_WN_Cnt++;

  switch (opc) {
/*
  case OPC_EXC_SCOPE_BEGIN:
    new_label = Gen_Number_Label( ++SYMTAB_last_label( Current_Symtab ) );
    Allocate_Object(new_label);
    bb = Add_Label (new_label);
    EH_Begin_Range (stmt, new_label);
    break;
  case OPC_EXC_SCOPE_END:
    new_label = Gen_Number_Label( ++SYMTAB_last_label( Current_Symtab ) );
    Allocate_Object(new_label);
    bb = Add_Label (new_label);
    EH_End_Range(new_label);
    break;
*/
  case OPC_TRUEBR:
  case OPC_FALSEBR:
  case OPC_GOTO:
  case OPC_AGOTO:
  case OPC_COMPGOTO:
  case OPC_XGOTO:
  case OPC_REGION_EXIT:
    Convert_Branch (stmt);
    break;
  case OPC_RETURN:
    /* For regions with returns, could try to jump back through nested 
     * regions until reach outer PU, but that is inefficient, so instead
     * put the return code right in the region.  This means that once the
     * PU is finished we will insert exit code (epilog restore code) in
     * the exit_bb in the region.
     */
    Handle_Return ();
    break;
  case OPC_LABEL:
    loop_info = WN_label_loop_info(stmt);
    if (loop_info && CG_opt_level > 1) {
      WN *trip_wn = WN_loop_trip(loop_info);
      SRCPOS srcpos = get_loop_srcpos(stmt);
      if (trip_wn && WN_operator_is(trip_wn, OPR_INTCONST) &&
	  WN_const_val(trip_wn) < 1) {
	/*
	 * Usually, this indicates an error in the trip count computation,
	 * not an actual trip count < 1, since the optimizer removes such
	 * loops.  (Though we can get actual trip counts < 1 if the optimizer
	 * doesn't run, but CG optimization is done.)  So instead of removing
	 * the loop, simply warn and set the trip count to NULL so we won't
	 * believe it.
	 */
	DevWarn("removing loop trip count (line %d) "
		"(< 1, either useless or invalid)",
		Srcpos_To_Line(srcpos));
	WN_set_loop_trip(loop_info, NULL);
	trip_wn = NULL;
      }
#ifdef TARG_ST
      // [CG]: In the case where the machine is 32 bits we discard
      // 64 bits trip count information as we can't handle it in CG.
      if (Only_32_Bit_Ops && trip_wn && 
	  MTYPE_byte_size(WN_rtype(trip_wn)) > 4) {
	DevWarn("removing loop trip count (line %d) "
		"(trip count byte size > 4)",
		Srcpos_To_Line(srcpos));
	WN_set_loop_trip(loop_info, NULL);
	trip_wn = NULL;
      }
#endif
      if (trip_wn == NULL) {
	trip_tn = NULL;
      } else {
	if (WN_operator_is(trip_wn, OPR_INTCONST)) {
	  INT64 trip_val = WN_const_val(trip_wn);
	  UINT16 sz = MTYPE_RegisterSize(WN_rtype(trip_wn));
	  /* Correct trip estimate - not always right */
	  WN_loop_trip_est(loop_info) = MIN(trip_val, UINT16_MAX);
	  trip_tn = Gen_Literal_TN(WN_const_val(trip_wn), sz);
	} else {
	  /*
	   * Trip count TN must be defined in the BB immediately preceding
	   * the loop body so that CG_LOOP_Attach_Prolog_And_Epilog can
	   * include this BB in the prolog.  This is necessary so that
	   * the trip count TN is properly marked as live-out by
	   * CG_LOOP_Recompute_Liveness, which limits the liveness
	   * recomputation to the loop region.
	   */
	  if (OPS_first(&New_OPs)) {
	    Start_New_Basic_Block();
	  }
	  trip_tn = Expand_Expr(trip_wn, NULL, NULL);
	  if (OPS_first(&New_OPs) == NULL) {
	    /* Make sure trip TN is defined in this BB. */
	    TN *tmp = Dup_TN_Even_If_Dedicated(trip_tn);
	    Exp_COPY(tmp, trip_tn, &New_OPs);
	    trip_tn = tmp;
	  }
	}
      }
      info = TYPE_P_ALLOC(LOOPINFO);
      LOOPINFO_wn(info) = loop_info;
      LOOPINFO_srcpos(info) = srcpos;
      LOOPINFO_primary_trip_count_tn(info) = trip_tn;
#ifdef TARG_ST
      LOOPINFO_is_exact_trip_count(info) = TRUE;
      LOOPINFO_is_HWLoop(info) = FALSE;
#endif
#ifdef TARG_ST
      LOOPINFO_trip_min(info) = -1;
      LOOPINFO_kunroll(info) = 0;
#endif
      if (!CG_PU_Has_Feedback && WN_loop_trip_est(loop_info) == 0)
	WN_loop_trip_est(loop_info) = 100;
    }
    if (WN_Label_Is_Handler_Begin(stmt)) {
	LABEL_IDX label = Get_WN_Label(stmt);
	Handle_Entry(stmt);
	bb = BB_LIST_first(Entry_BB_Head);
        Set_BB_handler(bb);
	BB_Add_Annotation (bb, ANNOT_LABEL, (void *)label);
	FmtAssert (Get_Label_BB(label) == NULL,
       		("Label %s defined more than once", LABEL_name(label)));
	Set_Label_BB (label,bb);
    } else {
    	/* start of a new basic block */
    	bb = Add_Label(Get_WN_Label (stmt));
    }
#ifdef TARG_ST
    // (cbr) we don't necessary have LOOPINFO. put keep pragma
    if (info) {
      BB_Add_Annotation(bb, ANNOT_LOOPINFO, info);
    }
#else
    if (info) {
      BB_Add_Annotation(bb, ANNOT_LOOPINFO, info);
      if (last_loop_pragma) {
	BB_Add_Annotation(bb, ANNOT_PRAGMA, last_loop_pragma);
	last_loop_pragma = NULL;
      }
    }
#endif
    break;
  case OPC_ALTENTRY:
    Handle_Entry (stmt);
    break;
  case OPC_PRAGMA:
  case OPC_XPRAGMA:
#ifdef TARG_ST
    if (WN_Pragma_Users(WN_pragma(stmt)) & PUSER_CG) {
      if (WN_Pragma_Scope(WN_pragma(stmt)) == WN_PRAGMA_SCOPE_LOOP) {
	ANNOTATION *loop_pragmas = (ANNOTATION *)BB_MAP_Get(loop_pragma_map, Cur_BB);
	loop_pragmas = ANNOT_Add(loop_pragmas, ANNOT_PRAGMA, (void *)stmt, &MEM_pu_pool);
	BB_MAP_Set(loop_pragma_map, Cur_BB, loop_pragmas);

	// [CL] Mark prologue for debug output
      } else if (WN_pragma(stmt) == WN_PRAGMA_PREAMBLE_END) {
	OP* op;
	for (op=OPS_first(&New_OPs); op != NULL ; op=OP_next(op)) {
	  Set_OP_prologue(op);
	}
      }
#else
    if (WN_Pragma_Users(WN_pragma(stmt)) & PUSER_CG) {
      if (WN_pragma(stmt) == WN_PRAGMA_UNROLL)
	/*
	 * Will place loop pragmas on loop head BB.  Currently we
	 * need only one pragma per loop, so a single WN * is fine
	 * for tracking this.
	 */
	last_loop_pragma = stmt;
#endif
      else
	BB_Add_Annotation(Cur_BB, ANNOT_PRAGMA, stmt);
    }
    break;
  case OPC_COMMENT:
    COMMENT_Add(Cur_BB, WN_GetComment(stmt));
    break;
  case OPC_EVAL:
    /* For now, just evaluate the kid0. */
#ifdef TARG_IA32
    if (WN_has_side_effects(WN_kid0(stmt)))
#endif
    Expand_Expr (WN_kid0(stmt), NULL, NULL);
    break;
  case OPC_TRAP:
    Handle_Trap(stmt);
    break;
  case OPC_ASM_STMT:
    Handle_ASM (stmt);
    break;
#ifdef TARG_ST
  case OPC_AFFIRM:
    Handle_AFFIRM (stmt);
    break;
#endif
  default:
    PU_WN_Cnt--;	/* don't want to count node twice */
    Expand_Expr (stmt, NULL, NULL);
    break;
  }

  return;
}

#ifdef TARG_ST
/* ====================================================================
 * Push_Intrinsic_Result
 *
 * This function (and the following one) are auxiliary routines used
 * to save/get TN results of the last intrinsic.
 * ====================================================================
 */
static void push_intrinsic_result(
   WN *intrncall, 
   TN **res, 
   INT numres)
{
   INT i;


   FmtAssert(0<=numres && numres<ISA_OPERAND_max_results,
              ("too many results for intrinsic"));
   Intrinsic_Result_Sav = TYPE_PU_ALLOC(INTRINSIC_RESULT);

   ++Intrinsic_Count;

   Intrinsic_Result_Sav->numres = numres;
   for(i=0;i<numres;i++)
     Intrinsic_Result_Sav->res[i] = res[i];

   return;
}

static INTRINSIC_RESULT* get_last_intrinsic_result(void)
{
   INTRINSIC_RESULT *ret;
   INT               i;

   FmtAssert(NULL!=Intrinsic_Result_Sav && Intrinsic_Count>0,
             ("Internal error in intrinsic management"));

   ret = TYPE_PU_ALLOC(INTRINSIC_RESULT);

   ret->numres = Intrinsic_Result_Sav->numres;
   for(i=0;i<Intrinsic_Result_Sav->numres;i++)
      ret->res[i]=Intrinsic_Result_Sav->res[i];

   return ret;
}
#endif     /* TARG_ST */

/* ====================================================================
 *   Handle_INTRINSIC_CALL
 * ====================================================================
 */
static WN *
Handle_INTRINSIC_CALL (
  WN *intrncall
)
{
  TN *result = NULL;
  TN **opnd_tn;
  INT numopnds;
  TN **res;
  INT numrests;
  INT i;
  LABEL_IDX label = LABEL_IDX_ZERO;
  OPS loop_ops;

  WN *next_stmt = WN_next(intrncall);
  INTRINSIC id = (INTRINSIC) WN_intrinsic (intrncall);

  if (Trace_WhirlToOp) {
    fprintf(TFile, "Handle_INTRINSIC_CALL %s\n", INTRN_c_name(id));
  }

  Get_Intrinsic_Op_Parameters( intrncall, &result, &opnd_tn, &numopnds, &res, &numrests );

#ifdef TARG_ST
  // Store the result for the followings LDID or SUBPART
  push_intrinsic_result(intrncall,res,numrests);
#endif

  if (Trace_WhirlToOp) {
    fprintf(TFile, "exp_intrinsic_call %s: ", INTRN_c_name(id));
    for (i = 0; i < numrests-1; i++) {
      Print_TN(res[i], FALSE);
      fprintf(TFile, ", ");
    }
    if(numrests != 0) {
      Print_TN(res[numrests-1], TRUE);
    }
    fprintf(TFile, " :- ");
    for (i = 0; i < numopnds-1; i++) {
      Print_TN(opnd_tn[i], FALSE);
      fprintf(TFile, ", ");
    }
    if(numopnds != 0) {
      Print_TN(opnd_tn[numopnds-1], FALSE);
    }
    fprintf(TFile, "\n");
  }

  // if straight-line code, then label and loop_ops are unused,
  // but might create a loop in which case we need to create bb for it.
  // (other possible ways of doing this would have been to use split_bb
  // or multiple exp_ calls for the different parts).

  OPS_Init(&loop_ops);
#ifdef TARG_ST
  if (id == INTRN_BUILTIN_EH_RETURN) {
    Handle_EH_Return (opnd_tn[0], opnd_tn[1]);
  } else if (id == INTRN_BUILTIN_UNWIND_INIT) {
    PU_Has_EH_Return = TRUE;
  } else {
    Exp_Intrinsic_Call (id, numrests, numopnds, res, opnd_tn, &New_OPs, &label, &loop_ops, current_srcpos);
  }

  // [CG]:We keep the last generated op
  OP *last_intr_op =  OPS_last(&New_OPs);
#else
  Exp_Intrinsic_Call (id, numrests, numopnds, res, opnd_tn, &New_OPs, &label, &loop_ops, current_srcpos);
#endif

  if (OPS_first(&loop_ops) != NULL && label != LABEL_IDX_ZERO) {
	BB *bb = Start_New_Basic_Block ();
	BB_Add_Annotation (bb, ANNOT_LABEL, (void *)label);
	Set_Label_BB (label,bb);
  	BB_branch_wn(bb) = WN_Create(OPC_FALSEBR,1);
  	WN_label_number(BB_branch_wn(bb)) = label;
	New_OPs = loop_ops;
	Start_New_Basic_Block ();
  }

  /* Expand the next statement and check if it has a use of $2. If any
   * use of $2 is found, replace it by the 'result' TN of the intrncall.
   */
  if (next_stmt != NULL && 
      result != NULL &&
      !WN_operator_is (next_stmt, OPR_INTRINSIC_CALL))
  {
    OP *op;
    if (Trace_WhirlToOp) {
      fprintf (TFile, "---------------------------------\n");
      fdump_tree_with_freq (TFile, next_stmt, WN_MAP_UNDEFINED);
    }
    Expand_Statement (next_stmt);
    next_stmt = WN_next(next_stmt);
    FOR_ALL_OPS_OPs_REV (&New_OPs, op) {
#ifdef TARG_ST
      // [CG]:We stop at the  last generated op
      // An intrinsic call may not be expanded into a TOP_intrcall!
      if (op == last_intr_op) break;
#else
      if (OP_code(op) == TOP_intrncall) break;
#endif
      for (i = 0; i < OP_opnds(op); i++) {
	TN *otn = OP_opnd(op,i);

	// [JV] Use of CLASS_AND_REG_v0 should not be used any more
	// because we want to be more generic and able to managed also
	// reg class unknown by ABI.
	if (   TN_is_dedicated(otn)
	    && TN_register_and_class(otn) == CLASS_AND_REG_v0
        ) {
	  Set_OP_opnd (op, i, result);
	}
      }
    }
  }

  MEM_POOL_FREE(Malloc_Mem_Pool, res);
  MEM_POOL_FREE(Malloc_Mem_Pool, opnd_tn);

  return next_stmt;
}

/* ====================================================================
 *   Only_Has_Exc_Label
 *
 *   very similar to routine in cflow; someday should commonize
 * ====================================================================
 */
static BOOL
Only_Has_Exc_Label (
  BB *bb
)
{
  if (BB_has_label(bb)) {
    ANNOTATION *ant;

    for (ant = ANNOT_First(BB_annotations(bb), ANNOT_LABEL);
         ant != NULL;
         ant = ANNOT_Next(ant, ANNOT_LABEL)
    ) {
      LABEL_IDX lab = ANNOT_label(ant);
      if (!LABEL_begin_eh_range(lab) && !LABEL_end_eh_range(lab))
	return FALSE;
    }
    return TRUE;
  }
  return FALSE;
}

/* ====================================================================
 *   convert_stmt_list_to_OPs
 *
 *   Input must be either a REGION or a non-SCF statement
 * ====================================================================
 */
static void
convert_stmt_list_to_OPs (
  WN *stmt
)
{
  RID *rid;
  WN *first;
  BB *prev, *last;
  WN *next_stmt;

  for( ; stmt; stmt = next_stmt ) {
    next_stmt = WN_next(stmt);

    switch( WN_opcode( stmt ) ) {
    case OPC_REGION:
      rid = REGION_get_rid(stmt);
      Is_True(rid != NULL, ("convert_stmt_list_to_OPs NULL RID"));
      if ( RID_level( rid ) < RL_CG ) { /* the region is still WHIRL */
	region_stack_push( rid );
#ifdef KEY
	if (RID_TYPE_eh(rid) && RID_eh_range_ptr(rid)) {
#else
	if (RID_TYPE_eh(rid)) {
#endif
	  EH_Set_Start_Label(RID_eh_range_ptr(rid));
        }

	first = WN_first( WN_region_body( stmt ) );
	Start_New_Basic_Block();
	if (RID_is_glue_code(rid)) {
		In_Glue_Region = TRUE;
	}
	convert_stmt_list_to_OPs( first );
	if (RID_is_glue_code(rid)) {
		In_Glue_Region = FALSE;
	}
#ifdef KEY
	if (RID_TYPE_eh(rid) && RID_eh_range_ptr(rid)) {
#else
	if (RID_TYPE_eh(rid)) {
#endif
	  EH_Set_End_Label(RID_eh_range_ptr(rid));
#ifdef KEY
	  /* When a region is ended, always force to create a new bb, so
	     that the next region will not share any common bb with the
	     current region. (bug#3140)
	   */
	  {
	    BB* old_bb = Cur_BB;
	    Start_New_Basic_Block();
	    if( Cur_BB == old_bb ){
	      Cur_BB = Gen_And_Append_BB( Cur_BB );
	      BB_rid(Cur_BB) = Non_Transparent_RID(current_region);
	    }
	  }
#endif
        }
	rid = region_stack_pop();
      } else {			/* the region has been lowered to OPs */
	BB *old_bb = Cur_BB;
	Start_New_Basic_Block();
	if (old_bb == Cur_BB) {
		/* need empty bb before nested region,
		 * so successor-bb finds outer region and right tns_in list
		 * rather than inner region (i.e. otherwise at bb level it
		 * will look like we transfer directly from pu to inner region).
		 * Also, if BB_has_label want new bb for label, 
		 * but now we always have new bb.
		 */
		Set_BB_gra_spill(Cur_BB);	/* so cflow won't remove */
		Cur_BB = Gen_And_Append_BB(Cur_BB);
		BB_rid(Cur_BB) = Non_Transparent_RID(current_region);
	}

	prev = BB_prev( Cur_BB );
	last = Append_Region_BBs( prev, rid );
	BB_prev( Cur_BB ) = last;
	BB_next( last ) = Cur_BB;

	CGRIN *cginfo = RID_cginfo(rid);
	Is_True(cginfo != NULL, ("convert_stmt_list_to_OPs, null cginfo"));
	if ( CGRIN_min_bb_id( cginfo ) < min_bb_id )
	  min_bb_id = CGRIN_min_bb_id( cginfo );
	if ( CGRIN_first_regular_tn( cginfo ) < First_REGION_TN )
	  First_REGION_TN = CGRIN_first_regular_tn( cginfo );
	if ( CGRIN_first_gtn( cginfo ) < First_REGION_GTN )
	  First_REGION_GTN = CGRIN_first_gtn( cginfo );
      }
      break;
    default:
      Is_True((OPCODE_is_stmt(WN_opcode(stmt))), 
	      ("convert_stmt_list_to_OPs: %d not a stmt", WN_opcode(stmt)));
      
      if (Trace_WhirlToOp) {
	fprintf (TFile,
		 "----- convert_stmt_list_to_OPs, BB%d -------------------\n",
		 BB_id(Cur_BB));
	fdump_tree_with_freq (TFile, stmt, WN_MAP_UNDEFINED);
      }
      if (WN_Get_Linenum(stmt))
	current_srcpos = WN_Get_Linenum(stmt);
      if (WN_operator_is (stmt, OPR_INTRINSIC_CALL)) {
	next_stmt = Handle_INTRINSIC_CALL (stmt);
      }
      else {
        Expand_Statement( stmt );
      }
      Process_OPs_For_Stmt();
    }
  }
}

/* ====================================================================
 *   Convert_WHIRL_To_OPs
 *
 *   Top level call for lowering a WHIRL tree to OPs
 *   The only hierarchy allowed in the input tree is REGIONs
 * ====================================================================
 */
void 
Convert_WHIRL_To_OPs (
  WN *tree
)
{
  WN *stmt;
  CGRIN *cgrin;
  BB *last_bb;
  INT num_exits, i;
  RID *rid = REGION_get_rid(tree);
  Is_True(rid != NULL, ("Convert_WHIRL_To_OPs, NULL RID"));
  BOOL Trace_BBs = Get_Trace (TP_CGEXP, 512);
  Trace_WhirlToOp = Get_Trace (TP_CGEXP, 2);

  // Initialization; some of this should be PU level others are region level

  if (Trace_WhirlToOp) {
    fprintf(TFile, "%sWHIRL tree input to Convert_WHIRL_To_OPs:\n%s",
	    DBar, DBar);
    fdump_tree_with_freq(TFile, tree, WN_MAP_UNDEFINED);
  }

  initialize_region_stack(tree);
  Cur_BB = NULL;
  current_srcpos = 0;

  switch ( WN_opcode( tree ) ) {
  case OPC_FUNC_ENTRY:
#if defined(CGG_ENABLED) //CGG_DEV
    if (CG_enable_cgg) {
      CGG_Start_function(tree);
      CGG_Start_bb(tree);
    }
#endif
    Compiling_Proper_REGION = FALSE;
    if (RID_cginfo(rid) == NULL) {
      RID_cginfo(rid) = CGRIN_Create(RID_num_exits(rid));
    }
    Handle_Entry( tree );
    stmt = WN_entry_first( tree );
    break;
  case OPC_REGION:
#if defined(CGG_ENABLED) //CGG_DEV
    if (CG_enable_cgg) {
      CGG_Start_region(tree);
      CGG_Start_bb(tree);
    }
#endif
    Compiling_Proper_REGION = TRUE;
    if ( RID_level( rid ) < RL_CG ) {      /* it is WHIRL */
      num_exits = RID_num_exits( rid );
      cgrin = CGRIN_Create( num_exits ); /* creates entry & exit blocks also */
      RID_cginfo( rid ) = cgrin;
      RID_level( rid ) = RL_CG;
      region_stack_push( rid );
      Start_New_Basic_Block();
      CGRIN_entry( cgrin ) = Cur_BB;
      min_bb_id = BB_id( Cur_BB );
      CGRIN_min_bb_id( cgrin ) = min_bb_id;
      BB_rid( Cur_BB ) = rid;
      stmt = WN_first( WN_region_body( tree ) );
    }
    else { /* it is OPs */
      Cur_BB = Append_Region_BBs( Cur_BB, rid );
      if ( CGRIN_first_regular_tn( RID_cginfo( rid ) ) < First_REGION_TN )
	First_REGION_TN = CGRIN_first_regular_tn( RID_cginfo( rid ) );
      if ( CGRIN_first_gtn( RID_cginfo( rid ) ) < First_REGION_GTN )
	First_REGION_GTN = CGRIN_first_gtn( RID_cginfo( rid ) );
      CGRIN_entry( cgrin ) = REGION_First_BB;
      stmt = NULL;
    }
    break;
  default:
    Is_True( FALSE, ("unexpected opcode in Convert_WHIRL_To_OPs") );
    break;
  }

  if ( stmt ) {
    convert_stmt_list_to_OPs( stmt );
   }

  /* If we have any OPs that have not been entered into a basic block,
   * do so now. This can happen if there is no exit for a procedure.
   */
  if (OPS_first(&New_OPs) != NULL) Start_New_Basic_Block ();

  /* On rare occasions we end up with the final BB just falling off
   * the end of the PU. It turns out that through some optimization,
   * this block will never be executed. When this condition occurs,
   * we essentially end up with a broken CFG. Instead of making all
   * consumers have to cope with this case, we just add the missing
   * return. If it truely was correct we should end up removing it.
   */
  if (!Compiling_Proper_REGION) {
    BB *final_bb = Cur_BB;
    if (BB_length( final_bb) == 0
	&& !BB_exit( final_bb)
	&& !BB_entry( final_bb)
	/* if scope label then okay to back-up, else have to keep block */
	&& (!BB_has_label(final_bb) || Only_Has_Exc_Label (final_bb))
    ) {
      final_bb = BB_prev (final_bb);
    }

    if (!BB_exit(final_bb)) {
      OP *op = BB_last_op (final_bb);
      if (op == NULL || OP_cond( op )) {
	Handle_Return ();
      }
    }
  }

  /* Because of the way Start_New_Basic_Block works, we almost certainly
   * end up with one extra BB at the end of the chain. Verify that we
   * have an extra BB, and remove it if so. We also determine the
   * last BB for the region code that follows.
   */
  last_bb = Cur_BB;
  if (    ( BB_length( Cur_BB ) == 0 )
       && ( ! BB_has_label( Cur_BB ) )
       && ( ! BB_exit( Cur_BB ) )
  ) {
    last_bb = BB_prev( Cur_BB );
    Remove_BB( Cur_BB );
    if ( BB_id( Cur_BB ) == PU_BB_Count )
      --PU_BB_Count;
  }
  Is_True(last_bb != NULL,("Convert_WHIRL_To_OPs, last_bb is NULL"));

  /* Build the control flow graph */
  Build_CFG();

#ifdef TARG_ST
  // FdF 23/04/2004: Now, move pragma LOOP annotation to loop head.
  for (BB *bp = REGION_First_BB; bp; bp = BB_next(bp)) {
    ANNOTATION *loop_pragmas;
    if ((loop_pragmas = (ANNOTATION *)BB_MAP_Get(loop_pragma_map, bp)) != NULL) {
      // find the loop header in the successors of this node. This may
      // be the unique successor of the current node, or the successor
      // of this one if it has one single predecessor (and thus is not
      // a loop head).
      BB *loop_head = BB_Unique_Successor(bp);
      while (loop_head && BB_Unique_Predecessor(loop_head))
	loop_head = BB_Unique_Successor(loop_head);
      if (loop_head == NULL) {
	if (CG_opt_level > 1)
	  ErrMsgSrcpos(EC_LNO_Bad_Pragma_String, WN_Get_Linenum((WN *)ANNOT_info(ANNOT_First(loop_pragmas, ANNOT_PRAGMA))),
		       WN_Pragma_Name(WN_pragma((WN *)ANNOT_info(ANNOT_First(loop_pragmas, ANNOT_PRAGMA)))),
		       "not followed by a loop, ignored");
	continue;
      }
      ANNOTATION *ant, *next;
      for (ant = ANNOT_First(loop_pragmas, ANNOT_PRAGMA); ant; ant = next) {
	next = ANNOT_Next(ant, ANNOT_PRAGMA);
#ifdef TARG_ST
	if (WN_pragma(ANNOT_pragma(ant)) == WN_PRAGMA_IVDEP) {
	  // FdF: Canonicalize a pragma IVDEP into its LOOPDEP form
	  LOOPDEP loopdep;
	  if (Cray_Ivdep) loopdep = LOOPDEP_VECTOR;
	  else if (Liberal_Ivdep) loopdep = LOOPDEP_LIBERAL;
	  else loopdep = LOOPDEP_PARALLEL;
	  WN *pragma = WN_CreatePragma(WN_PRAGMA_LOOPDEP, (ST_IDX) NULL,
				       loopdep, 0);
	  ANNOT_info(ant) = pragma;
	}
#endif
	BB_Add_Annotation(loop_head, ANNOT_PRAGMA, ANNOT_info(ant));
	ANNOT_Unlink(loop_pragmas, ant);
      }

      // FdF 30/09/2004: Set WN_loop_trip_est with the value of
      // #pragma LOOPTRIP(n)
      ant = ANNOT_Get(BB_annotations(loop_head), ANNOT_PRAGMA);
      while (ant && WN_pragma(ANNOT_pragma(ant)) != WN_PRAGMA_LOOPTRIP)
	ant = ANNOT_Get(ANNOT_next(ant), ANNOT_PRAGMA);
      if (ant) {
	ANNOTATION *annot = ANNOT_Get(BB_annotations(loop_head), ANNOT_LOOPINFO);
	LOOPINFO *info = annot ? ANNOT_loopinfo(annot) : NULL;
	if (info) {
	  WN *wn = ANNOT_pragma(ant);
#ifdef TARG_ST
	  TN *trip_count = LOOPINFO_exact_trip_count_tn(info);
#else
	  TN *trip_count = LOOPINFO_trip_count_tn(info);
#endif
	  if (trip_count && TN_is_constant(trip_count)) {
	    if (TN_value(trip_count) !=  WN_pragma_arg1(wn))
	      ErrMsgSrcpos(EC_LNO_Bad_Pragma_String, WN_Get_Linenum(wn), WN_Pragma_Name(WN_pragma(wn)),
			   "inconsistent with computed value, ignored");
	  }
	  else 
	    WN_loop_trip_est(LOOPINFO_wn(info)) = WN_pragma_arg1(wn);
	}
      }

      // FdF 20060912: Set LOOPINFO_trip_min with the value of #pragma
      // LOOPMINITERCOUNT
      ant = ANNOT_Get(BB_annotations(loop_head), ANNOT_PRAGMA);
      int loopmin = 0;
      ANNOTATION *ant_loopmin = NULL;
      while (ant) {
	if (WN_pragma(ANNOT_pragma(ant)) == WN_PRAGMA_LOOPMINITERCOUNT) {
	  ant_loopmin = ant;
	  loopmin = MAX(loopmin, WN_pragma_arg1(ANNOT_pragma(ant)));
	}
	// FdF 20070914: LOOPMOD also defines a minimum iteration
	// count with its second argument
	else if (WN_pragma(ANNOT_pragma(ant)) == WN_PRAGMA_LOOPMOD)
	  loopmin = MAX(loopmin, WN_pragma_arg2(ANNOT_pragma(ant)));
	ant = ANNOT_Get(ANNOT_next(ant), ANNOT_PRAGMA);
      }
      if (loopmin > 0) {
	ANNOTATION *annot = ANNOT_Get(BB_annotations(loop_head), ANNOT_LOOPINFO);
	LOOPINFO *info = annot ? ANNOT_loopinfo(annot) : NULL;
	if (info)
	  LOOPINFO_trip_min(info) = loopmin;
	// FdF 20060913: Then remove this pragma
	if (ant_loopmin != NULL)
	  BB_annotations(loop_head) = ANNOT_Unlink(BB_annotations(loop_head), ant_loopmin);
      }
      
    }
  }
#endif

  switch ( WN_opcode( tree ) ) {
  case OPC_FUNC_ENTRY:
#if defined(CGG_ENABLED) //CGG_DEV
    if (CG_enable_cgg) CGG_End_function(tree);
#endif
    break;
  case OPC_REGION:
    region_stack_pop();
    rid = REGION_get_rid(tree);
    CGRIN_first_bb( cgrin ) = REGION_First_BB;
    BB_rid( last_bb ) = rid;
    CGRIN_last_bb( cgrin ) = last_bb;
    CGRIN_min_bb_id( cgrin ) = min_bb_id;
    CGRIN_first_regular_tn( RID_cginfo( rid ) ) = First_REGION_TN;
    CGRIN_first_gtn( RID_cginfo( rid ) ) = First_REGION_GTN;
    CGRIN_preg_to_tn_mapping( cgrin ) = NULL;
    CGRIN_tns_in( cgrin ) = NULL;
    
    if ( RID_pregs_in( rid ) ) {
      Add_PregTNs_To_BB (RID_pregs_in(rid), CGRIN_entry(cgrin), TRUE);
    }
    
    num_exits = RID_num_exits(rid);
    if (num_exits > 0) {
      TN_LIST **tno;
      tno = TYPE_MEM_POOL_ALLOC_N( TN_LIST *, &REGION_mem_pool, num_exits );
      CGRIN_tns_out( cgrin ) = tno;
    } else {
      CGRIN_tns_out( cgrin ) = NULL;
    }
    for ( i = 0; i < num_exits; i++ ) {
	FmtAssert(cgrin != NULL && CGRIN_exit_i(cgrin,i),
		  ("missing exit bb for RGN %d, exit %d, cgrin=0x%x",
					  RID_id(rid), i, cgrin));
	
        Add_PregTNs_To_BB (RID_pregs_out_i(rid,i), CGRIN_exit_i(cgrin,i),
		FALSE);
	
    }
#if defined(CGG_ENABLED) //CGG_DEV
    if (CG_enable_cgg) CGG_End_region(tree);
#endif 
   break;
  default:
    #pragma mips_frequency_hint NEVER
    FmtAssert( FALSE, ("unexpected opcode in Convert_WHIRL_To_OPs") );
    /*NOTREACHED*/
  }

  if (Trace_BBs)
    Print_All_BBs ();

  return;
}

/* ====================================================================
 *   Whirl2ops_Initialize ()
 * ====================================================================
 */
void 
Whirl2ops_Initialize (
  struct ALIAS_MANAGER *alias_mgr
)
{
  Alias_Manager = alias_mgr;

  if ( Alias_Manager) {
    OP_to_WN_map = OP_MAP_Create();
    WN_to_OP_map = WN_MAP_Create(&MEM_phase_pool);
  }
  else {
    OP_to_WN_map = NULL;
    WN_to_OP_map = WN_MAP_UNDEFINED;
  }
#ifdef TARG_ST
  loop_pragma_map = BB_MAP_Create();
#else
  last_loop_pragma = NULL;
#endif
  OP_Asm_Map = OP_MAP_Create();

#ifdef TARG_ST
  OP_to_callinfo_map = OP_MAP_Create();
#endif

#ifdef CGG_ENABLED
  if (CG_enable_cgg) {
    CGG_Set_Level(CG_cgg_level);
    if (Get_Trace (TP_CGEXP, 2)) CGG_Set_Trace(1);
    CGG_Initialize();
  }
#endif

  return;
}

/* ====================================================================
 *   Whirl2ops_Finalize ()
 * ====================================================================
 */
void 
Whirl2ops_Finalize (void)
{
  /* delete the maps */
  if (predicate_map) {
    OP_MAP_Delete(predicate_map);
    predicate_map = NULL;
  }
  if (OP_to_WN_map) {
    OP_MAP_Delete(OP_to_WN_map);
    OP_to_WN_map = NULL;
  }
  if (WN_to_OP_map != WN_MAP_UNDEFINED) {
    WN_MAP_Delete(WN_to_OP_map);
    WN_to_OP_map = WN_MAP_UNDEFINED;
  }
#ifndef TARG_ST
  if (last_loop_pragma && 
      !WN_pragma_compiler_generated(last_loop_pragma) &&
      CG_opt_level > 1) {
    ErrMsgSrcpos(EC_LNO_Bad_Pragma_String, WN_Get_Linenum(last_loop_pragma),
		 WN_Pragma_Name(WN_pragma(last_loop_pragma)),
		 "not followed by a loop, ignored");
  }
#endif

#ifdef TARG_ST
  OP_MAP_Delete(OP_to_callinfo_map);
  if (OP_packed_to_ops_map != NULL) {
    OP_MAP_Delete(OP_packed_to_ops_map);
    OP_packed_to_ops_map = NULL;
  }
  OP_Affirm_delete_map();
#endif

  OP_MAP_Delete(OP_Asm_Map);

#ifdef TARG_ST
  BB_MAP_Delete(loop_pragma_map);
  if (WN_To_Hilo_map != WN_MAP_UNDEFINED) {
    WN_MAP_Delete(WN_To_Hilo_map);
    WN_To_Hilo_map = WN_MAP_UNDEFINED;
  }
#endif

#ifdef CGG_ENABLED
  if (CG_enable_cgg) CGG_Finalize();
#endif

  return;
}



//CGG_DEV{
#ifdef CGG_ENABLED

/*
 * Do we have a BB frontier.
 * 0: no, 
 * 1: start bb before,
 * 2: start bb after,
 * 3: start bb before and after,
 */
static int 
cgg_test_start_bb (WN *stmt) 
{
  OPCODE opc = WN_opcode(stmt);
  int do_bb = 0;
  
  switch(opc) {
  case OPC_REGION:
  case OPC_FUNC_ENTRY:
  case OPC_ALTENTRY:
  case OPC_LABEL:
    do_bb = 1;
    break;
  case OPC_RETURN:
  case OPC_TRUEBR:
  case OPC_FALSEBR:
  case OPC_GOTO:
  case OPC_AGOTO:
  case OPC_COMPGOTO:
  case OPC_XGOTO:
  case OPC_REGION_EXIT:
  case OPC_GOTO_OUTER_BLOCK:
    do_bb = 2;
    break;
  case OPC_ASM_STMT:
    do_bb = 3;
    break;
  case OPC_TRAP:
  case OPC_EVAL:
  case OPC_COMMENT:
  case OPC_PRAGMA:
  case OPC_XPRAGMA:
  case OPC_EXC_SCOPE_END:
  case OPC_EXC_SCOPE_BEGIN:
  default:
    break;
  }
  return do_bb;
}

static void
cgg_may_start_bb(WN *stmt)
{
  static WN *last_stmt;
  static int after_last;
  if (stmt != last_stmt) {
    int do_bb = cgg_test_start_bb(stmt);
    
    if (after_last || (do_bb & 1)) {
      CGG_Start_bb(stmt);
      after_last = 0;
    }
    if (do_bb & 2) after_last = 1;
    last_stmt = stmt;
  }
}

/* From Handle_XGOTO. */
static void
cgg_handle_xgoto(WN *stmt)
{
  TN *target_tn;
  WN *wn;
  INT i;
  ST *st;
  INITO_IDX ino;
  INITV_IDX inv, prev_inv;


  /* build jump table in init-data list */
  /* get table address */
  st = WN_st(stmt);
  BB_Add_Annotation (Cur_BB, ANNOT_SWITCH, st);
  /* make sure st is allocated */
  Allocate_Object(st);
  ino = New_INITO(st);
  prev_inv = INITV_IDX_ZERO;

  wn = WN_first(WN_kid1(stmt));	/* first goto */
  for (i = 0; i < WN_num_entries(stmt); i++) {
    FmtAssert ((wn && WN_opcode(wn) == OPC_GOTO),
	       ("XGOTO block doesn't have goto's? (%d)", WN_opcode(wn)));
    LABEL_IDX lab = WN_label_number(wn);
    inv = New_INITV();
    INITV_Init_Label (inv, lab);
    prev_inv = Append_INITV (inv, ino, prev_inv);
    wn = WN_next(wn);
  }
}

/* From Convert_Branch. */
static void
cgg_convert_branch(WN *stmt)
{
  BB_branch_wn(Cur_BB) = stmt;
  /* Terminate the basic block. */
  Start_New_Basic_Block ();
}

static void
Expand_Statement(WN *stmt)
{
  int do_cgg = 0;
  int do_convert_branch = 0;
  int do_handle_xgoto = 0;
  if (CG_enable_cgg) {  
    if (Trace_WhirlToOp) fprintf(TFile, "-- Expand_Statement\n");
    cgg_may_start_bb(stmt);
    CGG_Start_stmt(stmt);

    switch(WN_opcode(stmt)) {
    case OPC_TRUEBR:
    case OPC_FALSEBR:
    case OPC_REGION_EXIT:
    case OPC_GOTO:
    case OPC_AGOTO:
      do_convert_branch = 1;
      do_cgg = 1;
      break;
    case OPC_XGOTO:
      do_handle_xgoto = 1;
      do_convert_branch = 1;
      do_cgg = 1;
      break;
    case OPC_COMPGOTO: /* not implemented. */
    default:
      do_cgg = 0;
      if (Trace_WhirlToOp) fprintf (TFile, "-! Skipped CGG_Expand_Statement\n");
    }
  }

  if (do_cgg) {
    if (Trace_WhirlToOp) fprintf(TFile, "-> CGG_Expand_Statement\n");
    CGG_Expand_Expr(stmt, NULL, NULL, &New_OPs);
    if (do_handle_xgoto) cgg_handle_xgoto(stmt);
    if (do_convert_branch) cgg_convert_branch(stmt);
  } else {
    Expand_Statement_local(stmt);
  }
}

static TN *
Expand_Expr(WN *expr, WN *parent, TN *result)
{
  int do_cgg = 0;

  if (CG_enable_cgg) {  
    if (Trace_WhirlToOp) fprintf(TFile, "-- Expand_Expr\n");
    switch(WN_operator(expr)) {
    case OPR_GOTO_OUTER_BLOCK:
    case OPR_LDA_LABEL:
    case OPR_CALL:
    case OPR_ICALL:
    case OPR_PICCALL:
    case OPR_PREFETCH:
    case OPR_PREFETCHX:
    case OPR_ALLOCA:
    case OPR_DEALLOCA:
    case OPR_INTRINSIC_OP:
    case OPR_INTRINSIC_CALL:
      do_cgg = 0;
      if (Trace_WhirlToOp) fprintf (TFile, "!- Skipped CGG_Expand_Expr\n");
      break;
    default:
      do_cgg = 1;
    }
  }
  if (do_cgg) {
    if (Trace_WhirlToOp) fprintf(TFile, "-> CGG_Expand_Expr\n");
    result = CGG_Expand_Expr(expr, parent, result, &New_OPs);
  } else {
    result = Expand_Expr_local(expr, parent, result);
  }
  return result;
}
#endif
