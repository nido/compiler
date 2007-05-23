#include <map>
#include <list>

#include <assert.h>
#include <limits.h>

#ifdef TARG_ST

#ifdef _KEEP_RCS_ID
static const char source_file[] = __FILE__;
static const char rcs_id[] = "";
#endif /* _KEEP_RCS_ID */

#include "mempool.h"
#include "defs.h"
#include "bb.h"
#include "bbregs.h" 		/* to get the definition of BBREGS */
#include "gtn_universe.h"
#include "bitset.h"	/* our clients do not need to know about it */
#include "gtn_set.h"
#include "dominate.h"
#include "findloops.h"
#include "cxx_memory.h"
#include "tracing.h"
#include "gra_live.h"
#include "reg_live.h"
#include "cg_flags.h"

#include "cg_dud.h"

extern void Print_BB_Dominators(BB* bb);
extern void Print_Dominators(void);

static BOOL BaseOffset_Combine(OP *, OP *, OPS *);
static BOOL BaseOffset_CheckCombine(OP *, OP *);
static int baseOffset_Cost(DUD_REGION *, OP *, OP*);
static BOOL check_OffsetIncrOffset(OP *, OP *);

static std::map <OP *, OP * > M_Mem2Incr;

static std::map <OP *, OP * > M_PostIncr2Mem;
static std::map <OP *, OP * > M_PreIncr2Mem;
static std::map <OP *, OP * > M_OffsetIncr2Mem;

/*
 * Check that increment operation is a diadic add/sub
 * Allowed increment operations:
 *  Rn=Rn+Rp / Rn=Rn-Rp
 *  Rn=Rn+imm / Rn=Rn-imm
 */
static BOOL
check_Incrop(OP *incrop) {

  if (incrop == NULL) return FALSE;

  if(!OP_iadd(incrop) && !OP_isub(incrop)) return FALSE;

  INT op1_idx = TOP_Find_Operand_Use(OP_code(incrop), OU_opnd1);
  INT op2_idx = TOP_Find_Operand_Use(OP_code(incrop), OU_opnd2);

  // Allowed increment operations:
  // Rn=Rn+Rp / Rn=Rn-Rp
  // Rn=Rn+imm / Rn=Rn-imm

  if (OP_iadd(incrop)){
	if(!OP_Defs_TN(incrop, OP_opnd(incrop, op1_idx)) && !OP_Defs_TN(incrop, OP_opnd(incrop, op2_idx)))
	  return FALSE;
  }
  else if(OP_isub(incrop)){
	if(!OP_Defs_TN(incrop, OP_opnd(incrop, op1_idx)))
	  return FALSE;
  }

  return TRUE;
}


/*
 * Check offset compatibility for post increment
 * betwen two operations
 * - offset of memory operation and post increment operation are compatible
 * - increment is compatible with scaling factor
 * - increment fit in immediate range
 *
 * Possible patterns:
 *  @0(base) ; base += offset ;
 *  @offset(base) ; base += offset ;
 *
 *  @0(base_tn) ; base_tn += reg ;
 *  @reg(base_tn) ; base_tn += reg ;
 */
static BOOL
check_PostIncrOffset(OP *incrop, OP *memop){

  BOOL is_incr;
  INT64 incr_val = 0;

  if (incrop == NULL || memop == NULL) return FALSE;

  // Look for offset on memop
  INT offset_idx = OP_find_opnd_use(memop, OU_offset);
  INT base_idx = OP_find_opnd_use(memop, OU_base);

  if(offset_idx < 0) return FALSE;

  // Look for offset on incrop
  INT opnd1_idx = OP_find_opnd_use(incrop, OU_opnd1);
  INT opnd2_idx = OP_find_opnd_use(incrop, OU_opnd2);

  INT incropnd;
  if (OP_Defs_TN(incrop, OP_opnd(incrop, opnd1_idx)))
	incropnd = opnd2_idx;
  else
	incropnd = opnd1_idx;

  if (TN_has_value(OP_opnd(incrop, incropnd))) {  // Rn+imm
    is_incr = (OP_iadd(incrop) == (TN_value(OP_opnd(incrop, incropnd)) >= 0));
    incr_val = OP_iadd(incrop) ? TN_value(OP_opnd(incrop, incropnd)) : -TN_value(OP_opnd(incrop, incropnd));
  }
  else {  // Rn+Rp
    is_incr = OP_iadd(incrop); // Assume memop is inc
  }

  if(TN_has_value(OP_opnd(incrop, incropnd))) {  // Rn+imm

	TOP top_automod = TOP_UNDEFINED;
	TN *automod_tn = NULL;

	// Check the possible post increment/decrement sequences
	// 1) @0(base_tn) ; base_tn += cst ;    --> @0(base_tn=+cst) ;
	// 2) cst(base_tn) ; base_tn += cst ;  --> @0(base_tn+=cst) ;

	if(TN_has_value(OP_opnd(memop, offset_idx))){

	  if (TN_value(OP_opnd(memop, offset_idx)) == 0) {
		// Case 1)
		top_automod = TOP_AM_automod_variant(OP_code(memop), TRUE, is_incr, ISA_REGISTER_CLASS_UNDEFINED);
	  }
	  else if (TN_value(OP_opnd(memop, offset_idx)) == incr_val) {
		// Case 2)
		top_automod = TOP_AM_automod_variant(OP_code(memop), FALSE, is_incr, ISA_REGISTER_CLASS_UNDEFINED);
	  }
	  automod_tn = Gen_Literal_TN((is_incr) ? incr_val : -incr_val, 4);
	}

	if (top_automod == TOP_UNDEFINED) {
	  return FALSE;
	}

	// Check if immediate range is enough
    INT automod_idx = TOP_Find_Operand_Use(top_automod, OU_offset);
    if (!TOP_opnd_value_in_range(top_automod, automod_idx, TN_value(automod_tn))) {
	  return FALSE;
	}

    // Check the scaling factor
    if ((TN_value(automod_tn) % TOP_Mem_Bytes(top_automod)) != 0) {
      return FALSE;
	}

	return TRUE;

  }
  else{  // Rn+Rp
	TOP top_automod = TOP_UNDEFINED;

	// Check the possible post increment/decrement sequences
	// 1) @0(base_tn) ; base_tn += reg ;    --> @0(base_tn=+reg) ;
	// 2) @reg(base_tn) ; base_tn += reg ;  --> @0(base_tn+=reg) ;

    if (TN_has_value(OP_opnd(memop, offset_idx)) && (TN_value(OP_opnd(memop, offset_idx)) == 0)) {
	  // Case 1)
      top_automod = TOP_AM_automod_variant(OP_code(memop), TRUE, is_incr, TN_register_class(OP_opnd(incrop, incropnd)));
	}
    else if ((OP_opnd(incrop, incropnd) == OP_opnd(memop, offset_idx)) && (is_incr == TRUE)) {
      // Case 2)
      top_automod = TOP_AM_automod_variant(OP_code(memop), FALSE, is_incr, TN_register_class(OP_opnd(incrop, incropnd)));
    }

	if (top_automod == TOP_UNDEFINED) {
	  return FALSE;
	}

	return TRUE;
  }

  return FALSE;
}


/*
 * Check offset compatibility for pre increment
 * betwen two operations
 * - offset of memory operation and post increment operation are compatible
 * - increment is compatible with scaling factor
 * - increment fit in immediate range
 *
 * Possible patterns:
 *  base_tn += cst ; @0(base_tn) ;
 *  base_tn += cst ; @-cst(base_tn) ;
 *
 *  base_tn += reg ; @0(base_tn) ;
 *  base_tn -= reg ; @reg(base_tn) ;
 */
static BOOL
check_PreIncrOffset(OP *incrop, OP *memop){

  BOOL is_incr;
  INT64 incr_val = 0;

  if (incrop == NULL || memop == NULL) return FALSE;

  // Look for offset on memop
  INT offset_idx = OP_find_opnd_use(memop, OU_offset);
  INT base_idx = OP_find_opnd_use(memop, OU_base);

  if(offset_idx < 0) return FALSE;

  // Look for offset on incrop
  INT opnd1_idx = OP_find_opnd_use(incrop, OU_opnd1);
  INT opnd2_idx = OP_find_opnd_use(incrop, OU_opnd2);

  INT incropnd;
  if (OP_Defs_TN(incrop, OP_opnd(incrop, opnd1_idx)))
	incropnd = opnd2_idx;
  else
	incropnd = opnd1_idx;

  if (TN_has_value(OP_opnd(incrop, incropnd))) {  // Rn+imm
    is_incr = (OP_iadd(incrop) == (TN_value(OP_opnd(incrop, incropnd)) >= 0));
    incr_val = OP_iadd(incrop) ? TN_value(OP_opnd(incrop, incropnd)) : -TN_value(OP_opnd(incrop, incropnd));
  }
  else {  // Rn+Rp
    is_incr = OP_iadd(incrop); // Assume memop is inc
  }

  if(TN_has_value(OP_opnd(incrop, incropnd))) {  // Rn+imm

	TOP top_automod = TOP_UNDEFINED;
	TN *automod_tn = NULL;

	// Check the possible pre increment/decrement sequences
	//  1) base_tn += cst ; @0(base_tn) ;    --> @0(base_tn+=cst) ;
	//  2) base_tn += cst ; @-cst(base_tn) ; --> @0(base_tn=+cst) ;

	if(TN_has_value(OP_opnd(memop, offset_idx))){

	  if (TN_value(OP_opnd(memop, offset_idx)) == 0) {
		// Cases 1)
		top_automod = TOP_AM_automod_variant(OP_code(memop), FALSE, is_incr, ISA_REGISTER_CLASS_UNDEFINED);
	  }
	  else if (TN_value(OP_opnd(memop, offset_idx)) == -incr_val) {
		// Case 2)
		top_automod = TOP_AM_automod_variant(OP_code(memop), TRUE, is_incr, ISA_REGISTER_CLASS_UNDEFINED);
	  }
	  automod_tn = Gen_Literal_TN((is_incr) ? incr_val : -incr_val, 4);
	}

	if (top_automod == TOP_UNDEFINED) {
	  return FALSE;
	}

	// Check if immediate range is enough
    INT automod_idx = TOP_Find_Operand_Use(top_automod, OU_offset);
    if (!TOP_opnd_value_in_range(top_automod, automod_idx, TN_value(automod_tn))) {
	  return FALSE;
	}

    // Check the scaling factor
    if ((TN_value(automod_tn) % TOP_Mem_Bytes(top_automod)) != 0) {
      return FALSE;
	}

	return TRUE;

  }
  else{  // Rn+Rp
	TOP top_automod = TOP_UNDEFINED;

	// Check the possible post increment/decrement sequences
	// 1) base_tn += reg ; @0(base_tn) ;    --> @0(base_tn+=reg) ;
	// 2) base_tn -= reg ; @reg(base_tn) ;  --> @0(base_tn=-reg) ;

    if (TN_has_value(OP_opnd(memop, offset_idx)) && (TN_value(OP_opnd(memop, offset_idx)) == 0)) {
	  // Case 1)
      top_automod = TOP_AM_automod_variant(OP_code(memop), FALSE, is_incr, TN_register_class(OP_opnd(incrop, incropnd)));
	}
    else if ((OP_opnd(incrop, incropnd) == OP_opnd(memop, offset_idx)) && (is_incr == FALSE)) {
      // Case 2)
      top_automod = TOP_AM_automod_variant(OP_code(memop), TRUE, is_incr, TN_register_class(OP_opnd(incrop, incropnd)));
    }

	if (top_automod == TOP_UNDEFINED) {
	  return FALSE;
	}

	return TRUE;
  }

  return FALSE;
}


/*
 * Check that base TN is not used between memop and incrop
 */
static BOOL
is_NotUsed_PostIncr(DUD_REGION *dud, OP *memop, OP *incrop){

  if(memop == NULL || incrop == NULL) return TRUE;

  // Get base TN
  INT base_idx = OP_find_opnd_use(memop, OU_base);
  assert(base_idx >= 0);

  // Get increment TN
  INT opnd1_idx = OP_find_opnd_use(incrop, OU_opnd1);
  INT opnd2_idx = OP_find_opnd_use(incrop, OU_opnd2);

  INT incropnd;
  if (OP_Defs_TN(incrop, OP_opnd(incrop, opnd1_idx)))
	incropnd = opnd2_idx;
  else
	incropnd = opnd1_idx;

  //DU
  DUD_LIST ud_link, du_link;

  dud->Get_Use_Def(memop, base_idx, ud_link);

  INT ud_idx = 0;
  if (ud_link.size() == 2) {
    ud_idx = 1;
  }

  OP *defop = ud_link.op(ud_idx);
  INT defidx = ud_link.idx(ud_idx);

  dud->Get_Def_Use(defop, defidx, du_link);

  INT i;
  for (i = 0; i < du_link.size(); i++) {
    OP *useop = du_link.op(i);

    if(useop != NULL && useop != memop && useop != incrop){
      BB *usebb = OP_bb(useop);

      BS *DOM = BB_dom_set(usebb);
      BS *PDOM = BB_pdom_set(usebb);

      // Operation must be on a path from BBB(memory operation) to BB(increment operation)
      if( ((OP_bb(incrop) == OP_bb(memop) && OP_bb(incrop) == usebb && OP_Precedes(memop, useop) && OP_Precedes(useop, incrop))
		   || (OP_bb(incrop) != OP_bb(memop) && OP_bb(incrop) == usebb && OP_Precedes(useop, incrop))
		   || (OP_bb(incrop) != OP_bb(memop) && OP_bb(memop) == usebb && OP_Precedes(memop, useop))
	  || (OP_bb(incrop) != OP_bb(memop) && OP_bb(memop) != usebb && OP_bb(incrop) != usebb))
	  && BS_MemberP(DOM, BB_id(OP_bb(memop))) && BS_MemberP(PDOM, BB_id(OP_bb(incrop)))){


		// offset if operation is a load/store
		INT offset_idx = OP_find_opnd_use(useop, OU_offset);

		// immediate operand if operation has an immediate
		INT useopnd_1 = OP_find_opnd_use(useop, OU_opnd1);
		INT useopnd_2 = OP_find_opnd_use(useop, OU_opnd2);

		int useopnd = -1;
		if(useopnd_1 > 0 && TN_has_value(OP_opnd(useop, useopnd_1)))
		  useopnd = useopnd_1;
		else if(useopnd_2 > 0 && TN_has_value(OP_opnd(useop, useopnd_2)))
		  useopnd = useopnd_2;

		// Check that operation can be repaired at no cost:
		// -load/store with immediate deplacemnt when increment is a constant
		// -immediate operation when increment is a constant
		if (TN_has_value(OP_opnd(incrop, incropnd))){  // Rn+imm

		  // load/store or immediate operation
		  if(offset_idx > 0 && TN_has_value(OP_opnd(useop, offset_idx))){

		  INT64 incr_val = OP_iadd(incrop) ? TN_value(OP_opnd(incrop, incropnd)) : -TN_value(OP_opnd(incrop, incropnd));

		  // Check immediate range & scaling factor
		  if (!TOP_opnd_value_in_range(OP_code(useop), offset_idx, TN_value(OP_opnd(useop, offset_idx))-incr_val))
			return FALSE;

		  if ((TN_value(OP_opnd(useop, offset_idx)) % TOP_Mem_Bytes(OP_code(useop))) != 0)
			return FALSE;

		  }
		  else if( (OP_iadd(useop) || OP_isub(useop))  // immediate add/sub
				   && (useopnd > 0 && TN_has_value(OP_opnd(useop, useopnd)))){

			INT64 incr_val = OP_iadd(incrop) ? TN_value(OP_opnd(incrop, incropnd)) : -TN_value(OP_opnd(incrop, incropnd));

			// Check immediate range
			if (!TOP_opnd_value_in_range(OP_code(useop), useopnd, TN_value(OP_opnd(useop, useopnd))-incr_val))
			  return FALSE;
		  }
		  else // Other operations
			return FALSE;

		}
		else {  // Rn+Rp
		  // Repair code insertion needed
		  return FALSE;
		}

      }
    }
  }

  return TRUE;

}


/*
 * Check that offset TN is not defined on a path between memop and incrop
 */
static BOOL
is_NotDefined_PostIncr(DUD_REGION *dud, OP *memop, OP *incrop){

  if (incrop == NULL) return FALSE;

  // Look for offset on incrop
  INT op1_idx = TOP_Find_Operand_Use(OP_code(incrop),OU_opnd1);
  INT op2_idx = TOP_Find_Operand_Use(OP_code(incrop),OU_opnd2);

  // If offset TN is constant return TRUE
  if (TN_has_value(OP_opnd(incrop, op2_idx)) || TN_has_value(OP_opnd(incrop, op1_idx)))
	return TRUE;

  // Get increment TN
  INT incropnd;
  if (OP_Defs_TN(incrop, OP_opnd(incrop, op1_idx)))
	incropnd = op2_idx;
  else
	incropnd = op1_idx;

  // Get UD link
  DUD_LIST ud_link;
  dud->Get_Use_Def(incrop, incropnd, ud_link);

  // Check that all defs are not on a path from memory operation to increment operation
  int j;
  for(j = 0; j < ud_link.size(); j++){

	OP *op = ud_link.op(j);

    if(op != NULL && op != memop && op != incrop){

      BB *bb = OP_bb(op);

      BS *DOM = BB_dom_set(bb);
      BS *PDOM = BB_pdom_set(bb);

	  if( ((OP_bb(incrop) == OP_bb(memop) && OP_bb(incrop) == bb && OP_Precedes(memop, op) && OP_Precedes(op, incrop))
		   || (OP_bb(incrop) != OP_bb(memop) && OP_bb(incrop) == bb && OP_Precedes(op, incrop))
		   || (OP_bb(incrop) != OP_bb(memop) && OP_bb(memop) == bb && OP_Precedes(memop, op))
		   || (OP_bb(incrop) != OP_bb(memop) && OP_bb(memop) != bb && OP_bb(incrop) != bb))
		  && BS_MemberP(DOM, BB_id(OP_bb(memop))) && BS_MemberP(PDOM, BB_id(OP_bb(incrop)))){
		return FALSE;
	  }
	}

  }

  return TRUE;
}


/*
 * Check that base TN is not used on a path between incrop and memop
 */
static BOOL 
is_NotUsed_PreIncr(DUD_REGION *dud, OP *memop, OP *incrop){

  // look for base & increment TN on incrop
  INT op1_idx = TOP_Find_Operand_Use(OP_code(incrop), OU_opnd1);
  INT op2_idx = TOP_Find_Operand_Use(OP_code(incrop), OU_opnd2);

  INT useopnd;
  INT incropnd;
  if (OP_Defs_TN(incrop, OP_opnd(incrop, op1_idx))){
	useopnd = op1_idx;
	incropnd = op2_idx;
  }
  else {
	useopnd = op2_idx;
	incropnd = op1_idx;
  }

  // Check that all use of base TN are not on a path from memory operation to increment operation
  DUD_LIST du_link;
  dud->Get_Def_Use(incrop, useopnd, du_link);

  INT i;
  for (i = 0; i < du_link.size(); i++) {
    OP *useop = du_link.op(i); 

    if(useop != NULL && useop != memop && useop != incrop){
      BB *usebb = OP_bb(useop);

      // Do not count instructions before the memory operation in the basicblock

      BS *DOM = BB_dom_set(usebb);
      BS *PDOM = BB_pdom_set(usebb);

      // Operation must be on a path from BB(increment operation) to BB(memory operation)
      if( ((OP_bb(incrop) == OP_bb(memop) && OP_bb(incrop) == usebb && OP_Precedes(useop, memop) && OP_Precedes(incrop, useop))
		   || (OP_bb(incrop) != OP_bb(memop) && OP_bb(incrop) == usebb && OP_Precedes(incrop, useop))
		   || (OP_bb(incrop) != OP_bb(memop) && OP_bb(memop) == usebb && OP_Precedes(useop, memop))
		   || (OP_bb(incrop) != OP_bb(memop) && OP_bb(memop) != usebb && OP_bb(incrop) != usebb))
		  && BS_MemberP(DOM, BB_id(OP_bb(incrop))) && BS_MemberP(PDOM, BB_id(OP_bb(memop)))
		  ){

		// offset if operation is a load/store
		INT offset_idx = OP_find_opnd_use(useop, OU_offset);

		// immediate operand if operation has an immediate
		INT useopnd_1 = OP_find_opnd_use(useop, OU_opnd1);
		INT useopnd_2 = OP_find_opnd_use(useop, OU_opnd2);

		int useopnd = -1;
		if(useopnd_1 > 0 && TN_has_value(OP_opnd(useop, useopnd_1)))
		  useopnd = useopnd_1;
		else if(useopnd_2 > 0 && TN_has_value(OP_opnd(useop, useopnd_2)))
		  useopnd = useopnd_2;

		// Check that operation can be repaired at no cost:
		// -load/store with immediate deplacemnt when increment is a constant
		// -immediate operation when increment is a constant
		if (TN_has_value(OP_opnd(incrop, incropnd))){  //Rn+imm

		  // load/store or immediate operation
		  if((offset_idx > 0 && TN_has_value(OP_opnd(useop, offset_idx)))){

			INT64 incr_val = OP_iadd(incrop) ? TN_value(OP_opnd(incrop, incropnd)) : -TN_value(OP_opnd(incrop, incropnd));

			// Check immediate range & scaling factor
			if (!TOP_opnd_value_in_range(OP_code(useop), offset_idx, TN_value(OP_opnd(useop, offset_idx))-incr_val))
			  return FALSE;

			if ((TN_value(OP_opnd(useop, offset_idx)) % TOP_Mem_Bytes(OP_code(useop))) != 0)
			  return FALSE;

		  }
		  else if( (OP_iadd(useop) || OP_isub(useop))  // immediate add/sub
				   && (useopnd > 0 && TN_has_value(OP_opnd(useop, useopnd)))){

			INT64 incr_val = OP_iadd(incrop) ? TN_value(OP_opnd(incrop, incropnd)) : -TN_value(OP_opnd(incrop, incropnd));

			// Check immediate range
			if (!TOP_opnd_value_in_range(OP_code(useop), useopnd, TN_value(OP_opnd(useop, useopnd))-incr_val))
			  return FALSE;
		  }
		  else // Other operations
			return FALSE;

		}
		else {  // Rn+Rp
		  // Repair code insertion needed
		  return FALSE;
		}

	  }
	}
  }

  return TRUE;
}



/*
 * Check that operand is not defined on a path between memop and incrop
 */
static BOOL
is_NotDefined_PreIncr(DUD_REGION *dud, OP *memop, OP *incrop, INT opnd){

  if (incrop == NULL) return FALSE;

  //check that operand is not defined between incrop an memop
  if(TN_is_register(OP_opnd(incrop, opnd))){

	TN *tn = OP_opnd(incrop, opnd);

	OP *op;
	for (op = dud->Begin_op(); op != dud->End_op(); op = dud->Next_op(op)) {

	  if(op != NULL && op != memop && op != incrop){
		BB *usebb = OP_bb(op);

		// Do not count instructions before the memory operation in the basicblock

		BS *DOM = BB_dom_set(usebb);
		BS *PDOM = BB_pdom_set(usebb);

		// Operation must be on a path from BB(increment operation) to BB(memory operation)
		if( ((OP_bb(incrop) == OP_bb(memop) && OP_bb(incrop) == usebb && OP_Precedes(op, memop) && OP_Precedes(incrop, op))
			 || (OP_bb(incrop) != OP_bb(memop) && OP_bb(incrop) == usebb && OP_Precedes(incrop, op))
			 || (OP_bb(incrop) != OP_bb(memop) && OP_bb(memop) == usebb && OP_Precedes(op, memop))
			 || (OP_bb(incrop) != OP_bb(memop) && OP_bb(memop) != usebb && OP_bb(incrop) != usebb))
			&& BS_MemberP(DOM, BB_id(OP_bb(incrop))) && BS_MemberP(PDOM, BB_id(OP_bb(memop)))
			){

		  for (INT idx = 0; idx < OP_results(op); idx++) {
			if (OP_result(op, idx) == tn)
			  // tn is redefined
			  return FALSE;
		  }
		}
	  }
	}
  }

  return TRUE;
}

/*
 * Get increment value of increment operation
 */
static int
get_IncrValue(OP *incrop){

  if (incrop == NULL) return 0;

  //look for offset on defop
  INT op1_idx = TOP_Find_Operand_Use(OP_code(incrop),OU_opnd1);
  INT op2_idx = TOP_Find_Operand_Use(OP_code(incrop),OU_opnd2);

  if(!TN_has_value(OP_opnd(incrop, op2_idx)) && !TN_has_value(OP_opnd(incrop, op1_idx)))
	return INT_MIN;

  INT useopnd;
  if (TN_has_value(OP_opnd(incrop, op2_idx)))
	useopnd = op2_idx;
  else if (TN_has_value(OP_opnd(incrop, op1_idx)))
	useopnd = op1_idx;

  return TN_value(OP_opnd(incrop, useopnd));

}


/*
 * Check that tn is not live-out
 * on any out-edge leaving the region
 */
static BOOL
check_LiveOut(BB_SET *bbRegion_set, OP *memop, OP* incrop, TN *tn){

  if(memop == NULL || incrop == NULL) return FALSE;

  BB *dom;
  FOR_ALL_BB_SET_members(bbRegion_set, dom) { //forall dominated

	BS *DOM = BB_dom_set(dom);
	BS *PDOM = BB_pdom_set(dom);

	if(BS_MemberP(DOM, BB_id(OP_bb(memop))) && !BS_MemberP(PDOM, BB_id(OP_bb(incrop))) && !BS_MemberP(DOM, BB_id(OP_bb(incrop)))){

	  BBLIST *bb_l;
	  FOR_ALL_BB_SUCCS(dom, bb_l){

		BB *succ = BBLIST_item(bb_l);

		if(!BB_SET_MemberP(bbRegion_set, succ)){  //outgoing edge from the region
		  //check live out
		  if(GRA_LIVE_TN_Live_Outof_BB(tn, dom) && GRA_LIVE_TN_Live_Into_BB(tn, succ))
			return FALSE;
		}
	  }
	}
  }

  return TRUE;

}


/*
 * Find DU chain of incrop operation
 * Check that useop is not inside
 */
static BOOL
is_Incrop_DU(DUD_REGION *dud, OP *useop, OP* incrop){

  //only one result allowed
  if(OP_results(incrop) > 1) return FALSE;

  //du links 
  DUD_LIST du_link;
  dud->Get_Def_Use(incrop, 0, du_link);

  int j;
  for(j = 0; j < du_link.size(); j++){

	OP *op = du_link.op(j);

	if(op == useop){

	  BB *usebb = OP_bb(useop);
	  BB *incrbb = OP_bb(incrop);

	  BS *DOM = BB_dom_set(usebb);

	  return ((usebb == incrbb && OP_Precedes(incrop, useop) )
		  || (usebb != incrbb && BS_MemberP(DOM, BB_id(incrbb))));

	}
  }
  return FALSE;
}


/*
 * Compute cost of moving incrop close to memop upward
 */
static int
postincr_Cost(DUD_REGION *dud, OP *memop, OP* incrop){

  if(memop == NULL || incrop == NULL) return 0;

  int cost = 0;

  //look for increment TN
  INT op1_idx = TOP_Find_Operand_Use(OP_code(incrop), OU_opnd1);
  INT op2_idx = TOP_Find_Operand_Use(OP_code(incrop), OU_opnd2);

  INT incropnd;
  if (OP_Defs_TN(incrop, OP_opnd(incrop, op2_idx)))
	incropnd = op1_idx;
  else
	incropnd = op2_idx;

  // Look for Base TN
  INT base_idx = OP_find_opnd_use(memop, OU_base);
  assert(base_idx >= 0);

  DUD_LIST ud_link, du_link;

  dud->Get_Use_Def(memop, base_idx, ud_link);

  INT ud_idx = 0;
  if (ud_link.size() == 2) {
    // First def is outside the region
    ud_idx = 1;
  }

  OP *defop = ud_link.op(ud_idx);
  INT defidx = ud_link.idx(ud_idx);

  dud->Get_Def_Use(defop, defidx, du_link);

  INT i;
  for (i = 0; i < du_link.size(); i++) {
    OP *useop = du_link.op(i);

    if(useop != NULL && useop != memop && useop != incrop && !is_Incrop_DU(dud, useop, incrop)){
      BB *usebb = OP_bb(useop);

      /*
       * Cases that need repair
       * BB(memop) in DOM(useop) and BB(incrop) in PDOM(useop)
       * ( = useop in a path between memop and incrop)
       *
       * Cases that need repair on edge
       * BB(memop) in DOM(useop) and BB(incrop) not in DOM(useop) and BB(incrop) not in PDOM(useop)
       * ( = useop in a path from memop that doesn't include incrop)
       *
       * Cases that need no repair
       * BB(memop) in DOM(useop) and BB(incrop) in DOM(useop)
       * BB(memop) in PDOM(useop) and BB(incrop) in PDOM(useop)
       * ( = useop before or after a path from memop to incrop
       */

      BS *DOM = BB_dom_set(usebb);
      BS *PDOM = BB_pdom_set(usebb);

      // Operation must be on a path from BBB(memory operation) to BB(increment operation)
      if( ((OP_bb(incrop) == OP_bb(memop) && OP_bb(incrop) == usebb && OP_Precedes(memop, useop) && OP_Precedes(useop, incrop))
		   || (OP_bb(incrop) != OP_bb(memop) && OP_bb(incrop) == usebb && OP_Precedes(useop, incrop))
		   || (OP_bb(incrop) != OP_bb(memop) && OP_bb(memop) == usebb && OP_Precedes(memop, useop))
		   || (OP_bb(incrop) != OP_bb(memop) && OP_bb(memop) != usebb && OP_bb(incrop) != usebb))
		  && BS_MemberP(DOM, BB_id(OP_bb(memop))) && BS_MemberP(PDOM, BB_id(OP_bb(incrop)))){

		// offset if operation is a load/store
		INT offset_idx = OP_find_opnd_use(useop, OU_offset);

		// immediate operand if operation has an immediate
		INT useopnd_1 = OP_find_opnd_use(useop, OU_opnd1);
		INT useopnd_2 = OP_find_opnd_use(useop, OU_opnd2);

		int useopnd = -1;
		if(useopnd_1 > 0 && TN_has_value(OP_opnd(useop, useopnd_1)))
		  useopnd = useopnd_1;
		else if(useopnd_2 > 0 && TN_has_value(OP_opnd(useop, useopnd_2)))
		  useopnd = useopnd_2;

		// Check that operation can be repaired at no cost:
		// -load/store with immediate deplacemnt when increment is a constant
		// -immediate operation when increment is a constant
		if (TN_has_value(OP_opnd(incrop, incropnd))){  //Rn+imm

		  // load/store
		  if(offset_idx > 0 && TN_has_value(OP_opnd(useop, offset_idx))){

			INT64 incr_val = OP_iadd(incrop) ? TN_value(OP_opnd(incrop, incropnd)) : -TN_value(OP_opnd(incrop, incropnd));

			// Check immediate range & scaling factor
			if (!TOP_opnd_value_in_range(OP_code(useop), offset_idx, TN_value(OP_opnd(useop, offset_idx))-incr_val))
			  return INT_MAX;

			if ((TN_value(OP_opnd(useop, offset_idx)) % TOP_Mem_Bytes(OP_code(useop))) != 0)
			  return INT_MAX;

			// Constant offset cost nothing to repair
			cost++;
		  }
		  else if( (OP_iadd(useop) || OP_isub(useop))  // immediate add/sub
				   && (useopnd > 0 && TN_has_value(OP_opnd(useop, useopnd)))){
			cost++;
		  }
		  else // Other operations
			// Repair code insertion needed
			return INT_MAX;

		}
		else {  // Rn+Rp
		  // Repair code insertion needed
		  return INT_MAX;
		}


      }
      else if(OP_bb(defop) != OP_bb(memop)
			  && BS_MemberP(DOM, BB_id(OP_bb(memop))) && !BS_MemberP(PDOM, BB_id(OP_bb(incrop))) && !BS_MemberP(DOM, BB_id(OP_bb(incrop)))){
		// Repair code insertion needed in a basicblock or on an out edge of the region

		// Currently not supported
		return INT_MAX;
      }
      else{
		// Operation not on a path from BB(memory operation) to BB(increment operation)
		;
      }
    }
  }

  return cost;
}

/*
 * Compute cost of moving incrop close to memop downward
 */
static int
preincr_Cost(DUD_REGION *dud, OP *memop, OP* incrop){

  if(memop == NULL) return 0;

  int cost = 0;

  //look for increment TN
  INT op1_idx = TOP_Find_Operand_Use(OP_code(incrop), OU_opnd1);
  INT op2_idx = TOP_Find_Operand_Use(OP_code(incrop), OU_opnd2);

  INT incropnd;
  if (OP_Defs_TN(incrop, OP_opnd(incrop, op2_idx)))
	incropnd = op1_idx;
  else
	incropnd = op2_idx;

  DUD_LIST du_link;

  dud->Get_Def_Use(incrop, 0, du_link);

  INT i;
  for (i = 0; i < du_link.size(); i++) {

    OP *useop = du_link.op(i);

    if(useop != NULL && useop != memop && useop != incrop){
      BB *usebb = OP_bb(useop);

      // Do not count instructions before the memory operation in the basicblock

      BS *DOM = BB_dom_set(usebb);
      BS *PDOM = BB_pdom_set(usebb);

      // Operation must be on a path from BB(increment operation) to BB(memory operation)
      if( ((OP_bb(incrop) == OP_bb(memop) && OP_bb(incrop) == usebb && OP_Precedes(useop, memop) && OP_Precedes(incrop, useop))
		   || (OP_bb(incrop) != OP_bb(memop) && OP_bb(incrop) == usebb && OP_Precedes(incrop, useop))
		   || (OP_bb(incrop) != OP_bb(memop) && OP_bb(memop) == usebb && OP_Precedes(useop, memop))
		   || (OP_bb(incrop) != OP_bb(memop) && OP_bb(memop) != usebb && OP_bb(incrop) != usebb))
		  && BS_MemberP(DOM, BB_id(OP_bb(incrop))) && BS_MemberP(PDOM, BB_id(OP_bb(memop)))
		  ){

		// offset if operation is a load/store
		INT offset_idx = OP_find_opnd_use(useop, OU_offset);

		// immediate operand if operation has an immediate
		INT useopnd_1 = OP_find_opnd_use(useop, OU_opnd1);
		INT useopnd_2 = OP_find_opnd_use(useop, OU_opnd2);

		int useopnd = -1;
		if(useopnd_1 > 0 && TN_has_value(OP_opnd(useop, useopnd_1)))
		  useopnd = useopnd_1;
		else if(useopnd_2 > 0 && TN_has_value(OP_opnd(useop, useopnd_2)))
		  useopnd = useopnd_2;

		// Check that operation can be repaired at no cost:
		// -load/store with immediate deplacemnt when increment is a constant
		// -immediate operation when increment is a constant
		if (TN_has_value(OP_opnd(incrop, incropnd))){  //Rn+imm

		  // load/store
		  if(offset_idx > 0 && TN_has_value(OP_opnd(useop, offset_idx))){

			INT64 incr_val = OP_iadd(incrop) ? TN_value(OP_opnd(incrop, incropnd)) : -TN_value(OP_opnd(incrop, incropnd));

			// Check immediate range & scaling factor
			if (!TOP_opnd_value_in_range(OP_code(useop), offset_idx, TN_value(OP_opnd(useop, offset_idx))-incr_val))
			  return INT_MAX;

			if ((TN_value(OP_opnd(useop, offset_idx)) % TOP_Mem_Bytes(OP_code(useop))) != 0)
			  return INT_MAX;

			// Constant offset cost nothing to repair
			cost++;
		  }
		  else if( (OP_iadd(useop) || OP_isub(useop))  // immediate add/sub
				   && (useopnd > 0 && TN_has_value(OP_opnd(useop, useopnd)))){
			cost++;
		  }
		  else // Other operations
			return INT_MAX;

		}
		else {  // Rn+Rp
		  // Repair code insertion needed
		  return INT_MAX;
		}


      }
      else if(OP_bb(incrop) != OP_bb(memop)
			  && BS_MemberP(DOM, BB_id(OP_bb(incrop))) && !BS_MemberP(PDOM, BB_id(OP_bb(memop))) && !BS_MemberP(DOM, BB_id(OP_bb(memop)))){

		// Repair code insertion needed in a basicblock or on an out edge of the region

		// Currently not supported
		return INT_MAX;
      }
      else{
		// Operation not on a path from BB(increment operation) to BB(memory operation)
		;
      }
    }
  }

  return cost;
}


/*
 * Repair offset when a post increment operation
 * is moved up close to memory operation
 */
static void
postincr_RepairOffset(DUD_REGION *dud, OP *memop, OP* incrop){

  if(memop == NULL || incrop == NULL) return;

  // Look for Base TN
  INT base_idx = OP_find_opnd_use(memop, OU_base);
  assert(base_idx >= 0);

  TN *tn_base = OP_opnd(memop, base_idx);

  // Repair all operations on path from memop to incrop
  DUD_LIST ud_link, du_link;

  dud->Get_Use_Def(memop, base_idx, ud_link);

  INT ud_idx = 0;
  if (ud_link.size() == 2) {
    // First def is outside the region
    ud_idx = 1;
  }

  OP *defop = ud_link.op(ud_idx);
  INT defidx = ud_link.idx(ud_idx);

  dud->Get_Def_Use(defop, defidx, du_link);

  int offset = get_IncrValue(incrop);

  INT i;
  for (i = 0; i < du_link.size(); i++) {
    OP *useop = du_link.op(i);

    // Do not repair the memory or increment operation
    // Do not repair operations defined by the increment operation
    if(useop != NULL && useop != memop && useop != incrop && !is_Incrop_DU(dud, useop, incrop)){

      BB *usebb = OP_bb(useop);

      BS *DOM = BB_dom_set(usebb);
      BS *PDOM = BB_pdom_set(usebb);

      //do not repair operations which are not on a path from BB(memory operation) to BB(increment operation)
      if(((OP_bb(incrop) == OP_bb(memop) && OP_bb(incrop) == usebb && OP_Precedes(memop, useop) && OP_Precedes(useop, incrop))
		  || (OP_bb(incrop) != OP_bb(memop) && OP_bb(incrop) == usebb && OP_Precedes(useop, incrop))
		  || (OP_bb(incrop) != OP_bb(memop) && OP_bb(memop) == usebb && OP_Precedes(memop, useop))
		  || (OP_bb(incrop) != OP_bb(memop) && OP_bb(memop) != usebb && OP_bb(incrop) != usebb))
		 && BS_MemberP(DOM, BB_id(OP_bb(memop))) && BS_MemberP(PDOM, BB_id(OP_bb(incrop)))){

		// Offset TN
		INT useopnd = OP_find_opnd_use(useop, OU_offset);
		INT baseopnd = OP_find_opnd_use(useop, OU_base);

		// Constant repair
		if(TNs_Are_Equivalent(tn_base, OP_opnd(useop, baseopnd)) && OP_Refs_TN(useop, OP_opnd(useop, baseopnd))){

		  if(useopnd >= 0 && TN_has_value(OP_opnd(useop, useopnd))){

			TN *useop_tn = Dup_TN(OP_opnd(useop, useopnd));

			// Set offset to new value
			if (OP_iadd(incrop))
			  Set_TN_value(useop_tn, TN_value(useop_tn) - offset);
			else if(OP_isub(incrop))
			  Set_TN_value(useop_tn, TN_value(useop_tn) + offset);

			Set_OP_opnd(useop, useopnd, useop_tn);

		  }
		  else{
			// Non constant code repair
			// Curently not supported
			assert(0 && "code repair insertion not supported in automod");
		  }
		}
      }
      else if(OP_bb(defop) != OP_bb(memop)
			  && BS_MemberP(DOM, BB_id(OP_bb(memop))) && !BS_MemberP(PDOM, BB_id(OP_bb(incrop))) && !BS_MemberP(DOM, BB_id(OP_bb(incrop)))){
		// Repair code insertion needed in a basicblock or on an out edge of the region

		// Curently not supported
		assert(0 && "edge repair not supported in automod");
      }	
    }
  }
}


/*
 * Repair offset when a pre increment operation
 * is moved up close to memory operation
 */
static void
preincr_RepairOffset(DUD_REGION *dud, OP *memop, OP* incrop){

  if(memop == NULL || incrop == NULL) return;

  // Offset to repair
  int offset = get_IncrValue(incrop);

  // Look for Base TN
  INT base_idx = OP_find_opnd_use(memop, OU_base);
  assert(base_idx >= 0);

  TN *tn_base = OP_opnd(memop, base_idx);

  DUD_LIST du_link;

  dud->Get_Def_Use(incrop, 0, du_link);

  INT i;
  for (i = 0; i < du_link.size(); i++) {

    OP *useop = du_link.op(i);

    if(useop != NULL && useop != memop && useop != incrop){
      BB *usebb = OP_bb(useop);

      BS *DOM = BB_dom_set(usebb);
      BS *PDOM = BB_pdom_set(usebb);
  
      // Operation must be on a path from BB(increment operation) to BB(memory operation)
      if( ((OP_bb(incrop) == OP_bb(memop) && OP_bb(incrop) == usebb && OP_Precedes(useop, memop) && OP_Precedes(incrop, useop))
		   || (OP_bb(incrop) != OP_bb(memop) && OP_bb(incrop) == usebb && OP_Precedes(incrop, useop))
		   || (OP_bb(incrop) != OP_bb(memop) && OP_bb(memop) == usebb && OP_Precedes(useop, memop))
		   || (OP_bb(incrop) != OP_bb(memop) && OP_bb(memop) != usebb && OP_bb(incrop) != usebb))
		  && BS_MemberP(DOM, BB_id(OP_bb(incrop))) && BS_MemberP(PDOM, BB_id(OP_bb(memop)))
	  ){

		// Offset TN
		INT useopnd = OP_find_opnd_use(useop, OU_offset);
		INT baseopnd = OP_find_opnd_use(useop, OU_base);

		if(TNs_Are_Equivalent(tn_base,OP_opnd(useop, baseopnd)) && OP_Refs_TN(useop, OP_opnd(useop, baseopnd))){

		  // Constant repair
		  if(useopnd >= 0 && TN_has_value(OP_opnd(useop, useopnd))){

			TN *useop_tn = Dup_TN(OP_opnd(useop, useopnd));

			// Set offset to new value
			if (OP_iadd(incrop))
			  Set_TN_value(useop_tn, TN_value(useop_tn) + offset);
			else if(OP_isub(incrop))
			  Set_TN_value(useop_tn, TN_value(useop_tn) - offset);

			Set_OP_opnd(useop, useopnd, useop_tn);

		  }
		  else{
			// Non constant code repair

			// Curently not supported
			assert(0 && "code repair insertion not supported in automod");
		  }
		}
      }
      else if(OP_bb(incrop) != OP_bb(memop)
	      && BS_MemberP(DOM, BB_id(OP_bb(incrop))) && !BS_MemberP(PDOM, BB_id(OP_bb(memop))) && !BS_MemberP(DOM, BB_id(OP_bb(memop)))){
		// Repair code insertion needed in a basicblock or on an out edge of the region

		// Currently not supported
		assert(0 && "edge repair not supported in automod");
      }
    }
  }
}


static void
Memop_to_Incrop(BB_REGION *bbRegion, BB_SET *bbRegion_set, DUD_REGION *dud, OP* op) {

  // Look for base/offset TN of memory operation
  INT base_idx = OP_find_opnd_use(op, OU_base);
  INT offset_idx = OP_find_opnd_use(op, OU_offset);

  if (base_idx < 0 || offset_idx < 0) return;

  TN *tn_base = OP_opnd(op, base_idx);
  TN *tn_offset = OP_opnd(op, offset_idx);

  // Look for use-def link. If unique and a ADD/SUB, this is a candidate
  DUD_LIST ud_link, du_link;

  if (dud->Get_Use_Def(op, base_idx, ud_link) > 2) return;
  if (ud_link.size() == 0) return;

  INT ud_idx = 0;
  if (ud_link.size() == 2) {
    // Check that first def is outside the region
    if (ud_link.op(0) != NULL) return;
    ud_idx = 1;
  }

  OP *defop = ud_link.op(ud_idx);
  INT defidx = ud_link.idx(ud_idx);

  if(dud->Get_Def_Use(defop, defidx, du_link) < 0) return;

  // post increment
  // No support for predicated code yet
//   if (du_link.has_partial_def()) return;
  if (!du_link.has_partial_def()) {

    int best_cost = INT_MAX;
    OP *best_PostIncr;

    INT i;
    for (i = 0; i < du_link.size(); i++) {
      OP *useop = du_link.op(i);

	  if(check_Incrop(useop)
		 && check_PostIncrOffset(useop, op)
 		 && is_NotDefined_PostIncr(dud, op, useop)
		 && is_NotUsed_PostIncr(dud, op, useop)
		 && check_LiveOut(bbRegion_set, op, useop, tn_base)
		 && LOOP_DESCR_Find_Loop(OP_bb(useop)) == LOOP_DESCR_Find_Loop(OP_bb(op))
		 ) {

		BS *DOM = BB_dom_set(OP_bb(useop));
		BS *PDOM = BB_pdom_set(OP_bb(op));

		if((OP_bb(useop) != OP_bb(op) && BS_MemberP(DOM, BB_id(OP_bb(op))) && BS_MemberP(PDOM, BB_id(OP_bb(useop)))) 
		   || (OP_bb(useop) == OP_bb(op) && OP_Precedes(op, useop))){

		  // Compute cost
		  int cost = postincr_Cost(dud, op, useop);

		  // Add to candidates
		  std::map <OP *, OP *> :: const_iterator it_post = M_PostIncr2Mem.find(useop);
		  std::map <OP *, OP *> :: const_iterator it_pre = M_PreIncr2Mem.find(useop);

		  if(cost < best_cost && it_post == M_PostIncr2Mem.end() && it_pre == M_PreIncr2Mem.end()){
			best_cost = cost;
			best_PostIncr = useop;
		  }
		}
	  }
    }

    if(best_cost < INT_MAX){
      assert(best_PostIncr != NULL);

      M_PostIncr2Mem[best_PostIncr] = op;
      M_Mem2Incr[op] = best_PostIncr;
    }

  }

  // pre-increment
  if (ud_link.size() == 1) {

    if (check_Incrop(defop)
		&& check_PreIncrOffset(defop, op)
		&& is_NotUsed_PreIncr(dud, op, defop)
		&& check_LiveOut(bbRegion_set, defop, op, tn_base)
		&& LOOP_DESCR_Find_Loop(OP_bb(defop)) == LOOP_DESCR_Find_Loop(OP_bb(op))){

      BS *DOM = BB_dom_set(OP_bb(op));
      BS *PDOM = BB_pdom_set(OP_bb(defop));

      if( (OP_bb(defop) != OP_bb(op) && BS_MemberP(DOM, BB_id(OP_bb(defop))) && BS_MemberP(PDOM, BB_id(OP_bb(op))))
		  || (OP_bb(defop) == OP_bb(op) && OP_Precedes(defop, op)) ){

		// compute cost
		int cost = preincr_Cost(dud, op, defop);

		//add to candidates
		std::map <OP *, OP *> :: const_iterator it_post = M_PostIncr2Mem.find(defop);
		std::map <OP *, OP *> :: const_iterator it_pre = M_PreIncr2Mem.find(defop);
		std::map <OP *, OP *> :: const_iterator it_mem = M_Mem2Incr.find(op);

		if(cost < INT_MAX && it_post == M_PostIncr2Mem.end() && it_pre == M_PreIncr2Mem.end() && it_mem == M_Mem2Incr.end()){

		  M_PreIncr2Mem[defop] = op;
		  M_Mem2Incr[op] = defop;
		}

      }
    }
  }

  // base + offset
  if (ud_link.size() == 2 || ud_link.size() == 1) {  // memory operation has only one def

	if(du_link.size() == 1) {  // increment operation has only one use

	  /*
	   * check for valid increment operation
	   * check for valid increment offset
	   * check that incremented TN is not used between increment and memory operation
	   * check that increment TN is not live-out between increment and memory operation
	   */
	  if (OP_load(op)  // store not allowed
		  && (OP_iadd(defop) || OP_isub(defop))
 		  && check_OffsetIncrOffset(defop, op)
		  && is_NotUsed_PreIncr(dud, op, defop) 
		  && check_LiveOut(bbRegion_set, defop, op, tn_base)
		  && LOOP_DESCR_Find_Loop(OP_bb(defop)) == LOOP_DESCR_Find_Loop(OP_bb(op))){

		BS *DOM = BB_dom_set(OP_bb(op));
		BS *PDOM = BB_pdom_set(OP_bb(defop));

		if( (OP_bb(defop) != OP_bb(op) && BS_MemberP(DOM, BB_id(OP_bb(defop))) && BS_MemberP(PDOM, BB_id(OP_bb(op))))
			|| (OP_bb(defop) == OP_bb(op) && OP_Precedes(defop, op)) ){

		  // compute cost
		  int cost = baseOffset_Cost(dud, op, defop);		  

		  if(cost < INT_MAX){

			//valid base + offset

			//add to candidates
			std::map <OP *, OP *> :: const_iterator it_post = M_PostIncr2Mem.find(defop);
			std::map <OP *, OP *> :: const_iterator it_pre = M_PreIncr2Mem.find(defop);
			std::map <OP *, OP *> :: const_iterator it_mem = M_Mem2Incr.find(op);

			if(it_post == M_PostIncr2Mem.end() && it_pre == M_PreIncr2Mem.end() && it_mem == M_Mem2Incr.end()){
			  M_OffsetIncr2Mem[defop] = op;
			  M_Mem2Incr[op] = defop;
			}

		  }

		}

	  }

	}

  }

  return;
}


/*
 * Perform code reparation and motion
 * on selected memory operation/increment operation
 */
static void
code_Repair(DUD_REGION *dud){

  std::map <OP *, OP *> :: const_iterator it;

  // Pre-increment
  for(it = M_PreIncr2Mem.begin(); it != M_PreIncr2Mem.end(); ++it){

    OP *incrop = (*it).first;
    OP *memop = M_PreIncr2Mem[incrop];

    // Repair operations on path from increment operation to memory operation 
    preincr_RepairOffset(dud, memop, incrop);
    // Move increment operation before memory operation
    BB_Move_Op_Before(OP_bb(memop), memop, OP_bb(incrop), incrop);
  }

  // Post-increment 
  for(it = M_PostIncr2Mem.begin(); it != M_PostIncr2Mem.end(); ++it){

    OP *incrop = (*it).first;
    OP *memop = M_PostIncr2Mem[incrop];

   // Repair operations on path from memory operation to increment operation
    postincr_RepairOffset(dud, memop, incrop);
    // Move increment operation after memory operation
    BB_Move_Op_After(OP_bb(memop), memop, OP_bb(incrop), incrop);
  }

  // base + offset
  for(it = M_OffsetIncr2Mem.begin(); it != M_OffsetIncr2Mem.end(); ++it){

    OP *incrop = (*it).first;
    OP *memop = M_OffsetIncr2Mem[incrop];

	// Replace increment/memory operation by base + offset memory operation
    OPS ops = OPS_EMPTY;

	if(BaseOffset_Combine(memop, incrop, &ops)) {

	  // Repair operations on path from increment operation to memory operation 
	  preincr_RepairOffset(dud, memop, incrop);

      BB_Insert_Ops(OP_bb(memop), memop, &ops, FALSE);

// 	  fprintf(stderr, ">>------------------------------------------------<<\n");
// 	  Print_OP(incrop);
// 	  Print_OP(memop);
// 	  fprintf(stderr, "------------------------------------------------\n");
// 	  Print_OPS(&ops);
// 	  fprintf(stderr, "<<------------------------------------------------>>\n");

	  // move increment operation after memory operation
// 	  BB_Move_Op_After(OP_bb(memop), memop, OP_bb(incrop), incrop);
	  BB_Remove_Op(OP_bb(incrop), incrop);
	  BB_Remove_Op(OP_bb(memop), memop);
	}
  }

}


void Perform_AutoMod_Optimization() {

#ifdef TARG_STxP70

  MEM_POOL loop_descr_pool;
  MEM_POOL_Initialize(&loop_descr_pool, "loop_descriptors", TRUE);
  MEM_POOL_Push (&loop_descr_pool);

  MEM_POOL bbregion_set_pool;
  MEM_POOL_Initialize(&bbregion_set_pool, "bbregion_set", TRUE);
  MEM_POOL_Push (&bbregion_set_pool);

  BOOL trace_general = Get_Trace(TP_CGLOOP, 1);

  Calculate_Dominators();	/* needed for loop recognition */

  for (LOOP_DESCR *loop = LOOP_DESCR_Detect_Loops(&loop_descr_pool); loop; loop = LOOP_DESCR_next(loop)) {

    BB *head = LOOP_DESCR_loophead(loop);

    // Restrict to innermost loop to avoid processing large amount of code
//     if (!BB_innermost(head)) continue;

    BB_REGION bbRegion(LOOP_DESCR_bbset(loop), &loop_descr_pool);

    BB_SET *bbRegion_set = BB_SET_Create (PU_BB_Count + 2, &bbregion_set_pool);
    bbRegion_set = BB_REGION_to_BB_SET(bbRegion_set, bbRegion, &bbregion_set_pool);

    DUD_REGION *dudRegion = Build_DUD_info(&bbRegion, &loop_descr_pool);

    if (dudRegion != NULL) {

      M_PostIncr2Mem.clear();
      M_PreIncr2Mem.clear();
      M_OffsetIncr2Mem.clear();

	  // Automod
	  for (OP *op = dudRegion->Begin_op(); op != dudRegion->End_op(); op = dudRegion->Next_op(op)) {
		Memop_to_Incrop(&bbRegion, bbRegion_set, dudRegion, op);
      }

      // Code repair and motion
      code_Repair(dudRegion);

    }

  }

  MEM_POOL_Pop (&loop_descr_pool);
  MEM_POOL_Delete(&loop_descr_pool);

  MEM_POOL_Pop (&bbregion_set_pool);
  MEM_POOL_Delete(&bbregion_set_pool);

  Free_Dominators_Memory ();

#endif

}


static BOOL BaseOffset_Combine(OP *mem_op, OP *inc_op, OPS *ops) {

  INT base_idx = OP_find_opnd_use(mem_op, OU_base);
  INT offset_idx = OP_find_opnd_use(mem_op, OU_offset);
  TN *base_tn   = OP_opnd(mem_op, base_idx);
  TN *offset_tn = OP_opnd(mem_op, offset_idx);

  INT opnd1_idx = OP_find_opnd_use(inc_op, OU_opnd1);
  INT opnd2_idx = OP_find_opnd_use(inc_op, OU_opnd2);

  if(TN_has_value(OP_opnd(inc_op, opnd1_idx))){
	int swap = opnd2_idx;
	opnd2_idx = opnd1_idx;  // imm
	opnd1_idx = swap;  // tn
  } 
  else if (OP_opnd(inc_op, opnd1_idx) != base_tn && !TN_has_value(OP_opnd(inc_op, opnd2_idx))) {
    int swap = opnd2_idx;
    opnd2_idx = opnd1_idx;  // tn
    opnd1_idx = swap;  // tn or base_tn
  }

  assert(!TN_has_value(OP_opnd(inc_op, opnd1_idx)));

  TN * tn1 = OP_opnd(inc_op, opnd1_idx);

  TN *adjust_tn = OP_opnd(inc_op, opnd2_idx);
  BOOL is_inc;
  INT64 adjust_val = 0;

  if (TN_has_value(adjust_tn)) {
    is_inc = (OP_iadd(inc_op) == (TN_value(adjust_tn) >= 0));
    adjust_val = OP_iadd(inc_op) ? TN_value(adjust_tn) : -TN_value(adjust_tn);
  }
  else {
    is_inc = OP_iadd(inc_op); // Assume mem_op is inc
  }

  TOP top_baseoffset = TOP_UNDEFINED;
  BOOL mem_first = FALSE;

  TN *baseoffset_tn = NULL;
  if (TN_has_value(adjust_tn)) {
    if (TN_has_value(offset_tn)) {
      INT64 offset_val = TN_value(offset_tn);

	  // Rn + imm => Rn + Rp variant
	  top_baseoffset = TOP_opnd_immediate_variant(OP_code(mem_op), OP_find_opnd_use(mem_op, OU_offset), adjust_val + offset_val);
	  baseoffset_tn = Gen_Literal_TN(adjust_val+offset_val, 4);

	  // Check if immediate range is enough
	  assert(TN_has_value(baseoffset_tn));
	  INT baseoffset_idx = TOP_Find_Operand_Use(top_baseoffset, OU_offset);
	  if (!TOP_opnd_value_in_range(top_baseoffset, baseoffset_idx, TN_value(baseoffset_tn))) {
		  return FALSE;
	  }

	  // Take into account the scaling factor
	  if ((TN_value(baseoffset_tn) % TOP_Mem_Bytes(top_baseoffset)) != 0) {
		return FALSE;
	  }
	}
  }
  else {
    if (TN_has_value(offset_tn) && (TN_value(offset_tn) == 0)) {
      // Rn + 0 => Rn + Rp variant if is_inc
      // Rn + 0 => Rn - Rp variant if !s_inc: not implemented
      if (is_inc) top_baseoffset = TOP_opnd_register_variant(OP_code(mem_op), offset_idx, TN_register_class(tn1));
      baseoffset_tn = adjust_tn;
    }
  }


  if(top_baseoffset == TOP_UNDEFINED){
	return FALSE;
  }

  if (OP_store(mem_op)) {
    /* Store not supported in Rn+Rp mode */
    return FALSE;
  }

  OP *new_mem_op = Dup_OP(mem_op);
  OP_Change_Opcode(new_mem_op, top_baseoffset);
  Set_OP_opnd(new_mem_op, OP_find_opnd_use(mem_op, OU_offset), baseoffset_tn);
  Set_OP_opnd(new_mem_op, OP_find_opnd_use(mem_op, OU_base), tn1);
  OPS_Append_Op(ops, new_mem_op);

  return TRUE;
}


/*
 * Check base + offset compatibility betwen two operations
 *
 * - offset of memory operation and post increment operation are compatible
 * - increment is compatible with scaling factor
 * - increment fit in immediate range
 *
 * Possible patterns:
 * base_tn = base_tn + cst1 ; @cst(base_tn)  --> @cst+cst1(base_tn) ;
 * base_tn = base_tn + offset_tn ; @0(base_tn)  --> @offset(base_tn) ;
 * base_tn = base1_tn + cst ; @0(base_tn)  --> @cst(base1_tn) ;
 * base_tn = base1_tn + offset_tn ; @0(base_tn)  --> @offset(base1_tn) ;
 */
static BOOL
check_OffsetIncrOffset(OP *incrop, OP *memop){

  BOOL is_incr;
  INT64 incr_val = 0;

  if (incrop == NULL || memop == NULL) return FALSE;

  // Look for offset on memop
  INT offset_idx = OP_find_opnd_use(memop, OU_offset);
  INT base_idx = OP_find_opnd_use(memop, OU_base);

  if(offset_idx < 0) return FALSE;  // Rp@(Rn) not allowed

  // base/offset tn
  TN *base_tn   = OP_opnd(memop, base_idx);
  TN *offset_tn = OP_opnd(memop, offset_idx);

  // Look for offset on incrop
  INT opnd1_idx = OP_find_opnd_use(incrop, OU_opnd1);
  INT opnd2_idx = OP_find_opnd_use(incrop, OU_opnd2);


  if(TN_has_value(OP_opnd(incrop, opnd1_idx))){
	int swap = opnd2_idx;
	opnd2_idx = opnd1_idx;  // imm
	opnd1_idx = swap;  // tn
  } 
  else if (OP_opnd(incrop, opnd1_idx) != base_tn && !TN_has_value(OP_opnd(incrop, opnd2_idx))) {
    int swap = opnd2_idx;
    opnd2_idx = opnd1_idx;  // tn
    opnd1_idx = swap;  // tn or base_tn
  }

  assert(!TN_has_value(OP_opnd(incrop, opnd1_idx)));

  if (TN_has_value(OP_opnd(incrop, opnd2_idx))) {  // Rn+imm
    is_incr = (OP_iadd(incrop) == (TN_value(OP_opnd(incrop, opnd2_idx)) >= 0));
    incr_val = OP_iadd(incrop) ? TN_value(OP_opnd(incrop, opnd2_idx)) : -TN_value(OP_opnd(incrop, opnd2_idx));
  }
  else {  // Rn+Rp
    is_incr = OP_iadd(incrop); // Assume memop is inc
  }

  // Check the possible base+offset sequences
  //  1) base_tn = base_tn + cst1 ; @cst(base_tn)  --> @cst+cst1(base_tn) ;
  //  2) base_tn = base_tn + offset_tn ; @0(base_tn)  --> @offset(base_tn) ;
  //  3) base_tn = base1_tn + cst ; @0(base_tn)  --> @cst(base1_tn) ;
  //  4) base_tn = base1_tn + offset_tn ; @0(base_tn)  --> @offset(base1_tn) ;

  if(TN_has_value(OP_opnd(incrop, opnd2_idx))) {  // Rn+imm

	TOP top_automod = TOP_UNDEFINED;
	TN *automod_tn = NULL;

	if(TN_has_value(OP_opnd(memop, offset_idx))){

	  INT offset_val = TN_value(OP_opnd(memop, offset_idx));

	  // cases 1&3
	  automod_tn = Gen_Literal_TN(incr_val+offset_val, 4);
	  top_automod = TOP_UNDEFINED;
	  if (is_incr) {
	    top_automod = TOP_opnd_immediate_variant(OP_code(memop), offset_idx, TN_value(automod_tn));
	  }
	  if (top_automod == TOP_UNDEFINED) {
		return FALSE;
	  }

	  // Check if immediate range is enough
	  INT automod_idx = TOP_Find_Operand_Use(top_automod, OU_offset);
	  if (!TOP_opnd_value_in_range(top_automod, automod_idx, TN_value(automod_tn))) {
		return FALSE;
	  }

	  // Check the scaling factor
	  if ((TN_value(automod_tn) % TOP_Mem_Bytes(top_automod)) != 0) {
		return FALSE;
	  }

	  return TRUE;
	}
  }
  else{  // Rn+Rp

    TOP top_automod = TOP_UNDEFINED;
    
    if (TN_has_value(OP_opnd(memop, offset_idx)) && (TN_value(OP_opnd(memop, offset_idx)) == 0)) {
      // cases 2&4
      // Rn + 0 => Rn + Rp variant if is_incr
      // Rn + 0 => Rn - Rp variant if !is_incr : not implemented
      if (is_incr) top_automod = TOP_opnd_register_variant(OP_code(memop), offset_idx, TN_register_class(OP_opnd(incrop, opnd2_idx)));
      if (top_automod == TOP_UNDEFINED) {
	return FALSE;
      }

      return TRUE;
    }
  }

  return FALSE;
}


/*
 * Compute cost of moving incrop close to memop downward
 */
static int
baseOffset_Cost(DUD_REGION *dud, OP *memop, OP* incrop){

  if(memop == NULL) return 0;

  int cost = 0;

  // base/offset tn
  TN *base_tn   = OP_opnd(memop, OP_find_opnd_use(memop, OU_base));
  TN *offset_tn = OP_opnd(memop, OP_find_opnd_use(memop, OU_offset));

  // look for increment TN
  INT opnd1_idx = OP_find_opnd_use(incrop, OU_opnd1);
  INT opnd2_idx = OP_find_opnd_use(incrop, OU_opnd2);

  if (OP_opnd(incrop, opnd1_idx) != base_tn) {
    int swap = opnd2_idx;
    opnd2_idx = opnd1_idx;  //imm or tn
    opnd1_idx = swap;  //imm or tn
  }

  // get imm tn
  INT incropnd = opnd2_idx;
  if(TN_has_value(OP_opnd(incrop, opnd1_idx)))
	incropnd = opnd1_idx;

  //check that increment tn are not defined between incrop an memop
  if(OP_opnd(incrop, opnd1_idx) != base_tn && TN_is_register(OP_opnd(incrop, opnd1_idx))){

	TN *adjust_tn = OP_opnd(incrop, opnd1_idx);

	OP *useop;
	for (useop = dud->Begin_op(); useop != dud->End_op(); useop = dud->Next_op(useop)) {

	  if(useop != NULL && useop != memop && useop != incrop){
		BB *usebb = OP_bb(useop);

		// Do not count instructions before the memory operation in the basicblock

		BS *DOM = BB_dom_set(usebb);
		BS *PDOM = BB_pdom_set(usebb);

		// Operation must be on a path from BB(increment operation) to BB(memory operation)
		if( ((OP_bb(incrop) == OP_bb(memop) && OP_bb(incrop) == usebb && OP_Precedes(useop, memop) && OP_Precedes(incrop, useop))
			 || (OP_bb(incrop) != OP_bb(memop) && OP_bb(incrop) == usebb && OP_Precedes(incrop, useop))
			 || (OP_bb(incrop) != OP_bb(memop) && OP_bb(memop) == usebb && OP_Precedes(useop, memop))
			 || (OP_bb(incrop) != OP_bb(memop) && OP_bb(memop) != usebb && OP_bb(incrop) != usebb))
			&& BS_MemberP(DOM, BB_id(OP_bb(incrop))) && BS_MemberP(PDOM, BB_id(OP_bb(memop)))
			){

		  for (INT idx = 0; idx < OP_results(useop); idx++) {
			if (OP_result(useop, idx) == adjust_tn)
			  // adjust_tn is redefined
			  return INT_MAX;
		  }
		}
	  }
	}
  }

  if(OP_opnd(incrop, opnd2_idx) != base_tn && TN_is_register(OP_opnd(incrop, opnd2_idx))){
	TN *adjust_tn = OP_opnd(incrop, opnd2_idx);

	OP *useop;
	for (useop = dud->Begin_op(); useop != dud->End_op(); useop = dud->Next_op(useop)) {

	  if(useop != NULL && useop != memop && useop != incrop){
		BB *usebb = OP_bb(useop);

		// Do not count instructions before the memory operation in the basicblock

		BS *DOM = BB_dom_set(usebb);
		BS *PDOM = BB_pdom_set(usebb);

		// Operation must be on a path from BB(increment operation) to BB(memory operation)
		if( ((OP_bb(incrop) == OP_bb(memop) && OP_bb(incrop) == usebb && OP_Precedes(useop, memop) && OP_Precedes(incrop, useop))
			 || (OP_bb(incrop) != OP_bb(memop) && OP_bb(incrop) == usebb && OP_Precedes(incrop, useop))
			 || (OP_bb(incrop) != OP_bb(memop) && OP_bb(memop) == usebb && OP_Precedes(useop, memop))
			 || (OP_bb(incrop) != OP_bb(memop) && OP_bb(memop) != usebb && OP_bb(incrop) != usebb))
			&& BS_MemberP(DOM, BB_id(OP_bb(incrop))) && BS_MemberP(PDOM, BB_id(OP_bb(memop)))
			){

		  for (INT idx = 0; idx < OP_results(useop); idx++) {
			if (OP_result(useop, idx) == adjust_tn)
			  // adjust_tn is redefined
			  return INT_MAX;
		  }
		}
	  }
	}
  }


  DUD_LIST du_link;
  dud->Get_Def_Use(incrop, 0, du_link);

  INT i;
  for (i = 0; i < du_link.size(); i++) {

    OP *useop = du_link.op(i);

    if(useop != NULL && useop != memop && useop != incrop){
      BB *usebb = OP_bb(useop);

      // Do not count instructions before the memory operation in the basicblock

      BS *DOM = BB_dom_set(usebb);
      BS *PDOM = BB_pdom_set(usebb);

      // Operation must be on a path from BB(increment operation) to BB(memory operation)
      if( ((OP_bb(incrop) == OP_bb(memop) && OP_bb(incrop) == usebb && OP_Precedes(useop, memop) && OP_Precedes(incrop, useop))
		   || (OP_bb(incrop) != OP_bb(memop) && OP_bb(incrop) == usebb && OP_Precedes(incrop, useop))
		   || (OP_bb(incrop) != OP_bb(memop) && OP_bb(memop) == usebb && OP_Precedes(useop, memop))
		   || (OP_bb(incrop) != OP_bb(memop) && OP_bb(memop) != usebb && OP_bb(incrop) != usebb))
		  && BS_MemberP(DOM, BB_id(OP_bb(incrop))) && BS_MemberP(PDOM, BB_id(OP_bb(memop)))
		  ){

		// offset if operation is a load/store
		INT offset_idx = OP_find_opnd_use(useop, OU_offset);

		// immediate operand if operation has an immediate
		INT useopnd_1 = OP_find_opnd_use(useop, OU_opnd1);
		INT useopnd_2 = OP_find_opnd_use(useop, OU_opnd2);

		int useopnd = -1;
		if(useopnd_1 > 0 && TN_has_value(OP_opnd(useop, useopnd_1)))
		  useopnd = useopnd_1;
		else if(useopnd_2 > 0 && TN_has_value(OP_opnd(useop, useopnd_2)))
		  useopnd = useopnd_2;

		// Check that operation can be repaired at no cost:
		// -load/store with immediate deplacemnt when increment is a constant
		// -immediate operation when increment is a constant
		if (TN_has_value(OP_opnd(incrop, opnd1_idx)) || TN_has_value(OP_opnd(incrop, opnd2_idx))){  //Rn+imm

		  // load/store
		  if(offset_idx > 0 && TN_has_value(OP_opnd(useop, offset_idx))){

			INT64 incr_val = OP_iadd(incrop) ? TN_value(OP_opnd(incrop, incropnd)) : -TN_value(OP_opnd(incrop, incropnd));

			// Check immediate range & scaling factor
			if (!TOP_opnd_value_in_range(OP_code(useop), offset_idx, TN_value(OP_opnd(useop, offset_idx))-incr_val))
			  return INT_MAX;

			if ((TN_value(OP_opnd(useop, offset_idx)) % TOP_Mem_Bytes(OP_code(useop))) != 0)
			  return INT_MAX;

			// Constant offset cost nothing to repair
			cost++;
		  }
		  else if( (OP_iadd(useop) || OP_isub(useop))  // immediate add/sub
				   && (useopnd > 0 && TN_has_value(OP_opnd(useop, useopnd)))){
			cost++;
		  }
		  else // Other operations
			return INT_MAX;

		}
		else {  // Rn+Rp
		  // Repair code insertion needed
		  return INT_MAX;
		}

      }
      else if(OP_bb(incrop) != OP_bb(memop)
			  && BS_MemberP(DOM, BB_id(OP_bb(incrop))) && !BS_MemberP(PDOM, BB_id(OP_bb(memop))) && !BS_MemberP(DOM, BB_id(OP_bb(memop)))){

		// Repair code insertion needed in a basicblock or on an out edge of the region

		// Currently not supported
		return INT_MAX;
      }
      else{
		// Operation not on a path from BB(increment operation) to BB(memory operation)
		;
      }
    }
  }

  return cost;
}

#endif
