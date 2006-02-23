/*
  Copyright (C) 2002, STMicroelectronics, All Rights Reserved.

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
 * Module: cg_ivs.cxx
 *
 * Description:
 *
 *
 * ====================================================================
 * ====================================================================
 */

#ifdef _KEEP_RCS_ID
static const char source_file[] = __FILE__;
static const char rcs_id[] = "";
#endif /* _KEEP_RCS_ID */

#include <alloca.h>
#include "defs.h"
#include "resource.h"
#include "config.h"
#include "errors.h"
#include "mempool.h"
#include "cg.h"
#include "cgir.h"
#include "tracing.h"
#include "cgtarget.h"
#include "cgprep.h"
#include "op.h"
#include "op_list.h"
#include "op_map.h"
#include "bb.h"
#include "register.h"
#include "bbregs.h"
#include "dominate.h"
#include "cg_loop.h"
#include "cgexp.h"
#include "annotations.h"
#include "gtn_universe.h"
#include "gtn_set.h"
#include "cg_cflow.h"
#ifdef TARG_ST
#include "wn_util.h"
#endif
#include "whirl2ops.h"
#include "cg_db_op.h"
#include "cio.h"
#include "gra_live.h"
#include "data_layout.h"
#include "cxx_memory.h"
#include "cg_sched_est.h"
#include "cg_ivs.h"

#ifdef TARG_ST
#include "stblock.h" // for ST_alignment
#endif

// ======================================================================
//
//  Implementation of the interface for class LOOP_IVS, defined in cg_ivs.h
//
// ======================================================================

// TBD: Traiter les multi BB en inserant les operations qui
// definissent des GTN et qui ne sont pas sur des dominateurs de la
// queue de boucle sous forme de KILL.

// For a TN used in an operation, returns the Induction Variable from
// which it is derived. Returns 0 if not derived from an IV.
DefID_t
LOOP_IVS::OPND_IV_cycle(INT op_idx, INT opnd_idx) {
  Is_True(op_idx > 0, ("Calling OPND_IV_cycle with an invalid op."));
  return Find_IV(op_idx, opnd_idx);
}

// For a TN used in an operation, returns theoffset from the value of
// the IV at loop entry to this use.
INT
LOOP_IVS::OPND_IV_offset(INT op_idx, INT opnd_idx) {
  Is_True(op_idx > 0, ("Calling OPND_IV_offset with an invalid op."));
  if (OPND_omega(op_idx, opnd_idx) > 0)
    return 0;
  else
    return IV_offset(OPND_defid(op_idx, opnd_idx));
}

// For an induction variable, returns its step
INT
LOOP_IVS::IV_step(DefID_t iv_cycle) {
  Is_True(IV_cycle(iv_cycle) == iv_cycle, ("Calling IV_step on a non IV-representative op"));
  return IV_offset(iv_cycle);
}

// For an induction variable, returns its defining operation, if
// found, outside of the loop.
OP *
LOOP_IVS::IV_init(DefID_t iv_cycle) {
  Is_True(iv_cycle > 0, ("Calling IV_init with an invalid IV."));
  // Look for the definition of the IV outside the function.
  DefID_t iv_idx = iv_cycle;
  // Assume an ADD or a SUB, with IV being used on the first arg
  INT opnd_idx = TOP_Find_Operand_Use(OP_code(DEFID_op(iv_idx)), OU_opnd1);
  
  while (OPND_defid(DEFID_idx(iv_idx), opnd_idx) != iv_cycle) {
    iv_idx = OPND_defid(DEFID_idx(iv_idx), opnd_idx);
    opnd_idx = TOP_Find_Operand_Use(OP_code(DEFID_op(iv_idx)), OU_opnd1);
  }

  DEF_KIND kind;
  return TN_Reaching_Value_At_Op(OP_opnd(DEFID_op(iv_idx), opnd_idx), DEFID_op(iv_idx), &kind, TRUE);
}

void
LOOP_IVS::Trace_IVs_Entries( const char *message )
{
  Is_True( TFile, ( "LOOP_IVS::Trace_IVs_Entries -- TFile is NULL\n" ) );
  fprintf( TFile, "<ivs> LOOP_IVS::Trace_IVs_Entries(count %d) -- %s",
	   ivs_count, message );

  // Assumes table[0] is not used
  for ( INT index = 1; index < ivs_count; ++index ) {
    IVs_entry *entry = &ivs_table[index];
    fprintf( TFile, "\n%2d: ", index );
    Print_OP_No_SrcLine( entry->op );

    fprintf( TFile, "    op 0x%p", entry->op);
    if ( OP_results( entry->op ) > OP_MAX_FIXED_RESULTS )
      fprintf( TFile, " OP_results(entry->op) %d > OP_MAX_FIXED_RESULTS %d",
	       OP_results( entry->op ), OP_MAX_FIXED_RESULTS );
    else
      for (int i = 0; i < OP_results(entry->op); i++) 
	fprintf( TFile, ", IV_cycle (%3d,%1d), IV_offset %d",
		 DEFID_idx(entry->IV_cycle[i]), DEFID_res(entry->IV_cycle[i]), entry->IV_offset[i] );
    fprintf(TFile, "\n    opnds:");
    if ( OP_opnds( entry->op ) > OP_MAX_FIXED_OPNDS )
      fprintf( TFile, " OP_opnds(entry->op) %d > OP_MAX_FIXED_OPNDS %d",
	       OP_opnds( entry->op ), OP_MAX_FIXED_OPNDS );
    else
      for ( INT opnd = 0; opnd < OP_opnds( entry->op ); ++opnd ) {
	fprintf( TFile, "  opnd %d: src (%3d,%1d), om %u;",
		 opnd, DEFID_idx(entry->opnd_source[opnd]),
		 DEFID_res(entry->opnd_source[opnd]), OPND_omega(entry, opnd) );
      }
  }
  fprintf( TFile, "\n\n" );
}

/* Follow the use-def chains from def_index, until a cycle is
   found. If the cycle only contains ADD and SUB operations with an
   immediate value, return the index of the operation in the IVs table
   that is last executed in the loop. Return 0 otherwise. */

DefID_t
LOOP_IVS::Find_IV( INT op_idx, INT opnd_idx ) {

  DefID_t defid_src = OPND_defid(op_idx, opnd_idx);
  IVs_entry *entry = &ivs_table[op_idx];
  INT iv_idx = opnd_idx;

  DefID_t IV_index = 0;
  INT64 IV_step = 0;

  while (1) {

    Is_True(iv_idx >= 0, ("TOP_Find_Operand_Use did not find operand index."));

    /* Follow the use-def chain. Check the variable is defined in the
       loop and on the first result of the defining operation. */
    int defid = entry->opnd_source[iv_idx];
    if ( defid == 0 )
      return 0;

    // Check if this operation has already been visited. If yes, then
    // use the information already collected on the IVs.
    // TBD: discriminate between "not visited", and "not an IV"
    if (IV_cycle(defid)) {
      Is_True(IV_index == 0, ("Inconsistent state in Find_IV"));
      Set_IV_cycle(defid_src, IV_cycle(defid));
      Set_IV_offset(defid_src, IV_offset(defid));
      return IV_cycle(defid);
    }

    /* Check if a cycle is found. */
    if ( OPND_omega(entry, iv_idx) == 1 )
      if (IV_index == 0) {
	IV_index = defid;
	// IV_step is the offset from the use of the variable
	// ivs_table[op_idx]->opnd_source[opnd_idx], to the
	// value at the loop entry of the main IV on which it is based.
	Set_IV_offset(defid_src, IV_step);
	// Now reset IV_step to accumulate the step of the main
	// induction variable.
	IV_step = 0;
      }
      else {
	Is_True(IV_index == defid, ("Inconsistent IVs table."));
	Set_IV_cycle(defid_src, IV_index);
	break;
      }
    entry = &ivs_table[DEFID_idx(defid)];

    /* Only ADD and SUB operations with an immediate operand are
       recognized. */
    if (!OP_iadd(entry->op) && !OP_isub(entry->op))
      return 0;

    // FdF 20060116: TBD: Consider cst+i also.
    TN *opnd_incr = OP_opnd(entry->op, TOP_Find_Operand_Use(OP_code(entry->op), OU_opnd2));
    if (!TN_has_value(opnd_incr))
      return 0;

    // FdF 20060116: TBD: Also consider auto-increment operations.
    IV_step += OP_iadd(entry->op) ? TN_value(opnd_incr) : -TN_value(opnd_incr);

    iv_idx = TOP_Find_Operand_Use(OP_code(entry->op), OU_opnd1);
  }

  // For each operation in the cycle, entry.IV_cycle is initialized
  // to IV_index. entry.IV_offset is initialized to the increment of
  // the IV from the begining of an iteration of the loop to this
  // instruction included.

  Is_True(IV_index != 0, ("Inconsistency in Induction Variable"));
  Is_True(IV_step != 0, ("Inconsistency in Induction Variable"));

  DefID_t iv_index = IV_index;
  int iv_step = IV_step;

  do {
    entry = &ivs_table[DEFID_idx(iv_index)];
    Set_IV_cycle(iv_index, IV_index);
    Set_IV_offset(iv_index, iv_step);

    OP *op_iv = entry->op;
    Is_True(OP_iadd(op_iv) || OP_isub(op_iv), ("Unexpected operation"));
    int step_idx = TOP_Find_Operand_Use(OP_code(op_iv), OU_opnd2);
    INT64 incr = TN_value(OP_opnd(op_iv, step_idx));
    if (OP_isub(op_iv)) incr = -incr;
    iv_step -= incr;

    iv_idx = TOP_Find_Operand_Use(OP_code(op_iv), OU_opnd1);
    iv_index = entry->opnd_source[iv_idx];

  } while (OPND_omega(entry, iv_idx) == 0);

  Is_True(iv_step == 0, ("Inconsistency in Induction Variable"));

  return IV_index;
}

// Create use-def links over the instructions in the loop.

void
LOOP_IVS::Init_IVs_Table(OP *first_op, hTN_MAP32 tn_last_op) {

  INT index;
  OP *op = first_op;
  for ( index = 1; index < ivs_count; ++index ) {

    // Initialize the table entry for this OP
    IVs_entry& entry = ivs_table[index];
    entry.op = op;

    // Initialize the map tn_last_op
    for ( INT res = OP_results( op ) - 1; res >= 0; --res ) {
      entry.IV_cycle[res] = 0;
      entry.IV_offset[res] = 0;
      hTN_MAP32_Set( tn_last_op, OP_result( op, res ), DEFID_make(index, res) );
    }
    op = OP_next( op );
  }

  // Initialize the operands of ivs_table entries.
  for ( index = 1; index < ivs_count; ++index ) {
    IVs_entry& entry = ivs_table[index];
    OP *op = entry.op;

    // Initialize the ivs_table entry for the operands of this OP op
    // to point to the operands' most recent definitions
    for ( INT opnd = OP_opnds( op ) - 1; opnd >= 0; --opnd ) {
      INT defid = 0;
      TN *opnd_op = OP_opnd( op, opnd );
      if ( TN_is_register( opnd_op ) )
	defid = hTN_MAP32_Get( tn_last_op, opnd_op );

      entry.opnd_source[opnd] = defid;
    }

    // Update the map tn_last_op
    for ( INT res = OP_results( op ) - 1; res >= 0; --res ) {
      hTN_MAP32_Get_And_Set( tn_last_op, OP_result( op, res ), DEFID_make(index,res) );
    }
  }

  // Finally, initialize the entry 0
  ivs_table[0].op = NULL;

  for ( INT res = 0; res < OP_MAX_FIXED_RESULTS; res++ ) {
    ivs_table[0].IV_cycle[res] = 0;
    ivs_table[0].IV_offset[res] = 0;
  }

  for ( INT opnd = 0; opnd < OP_MAX_FIXED_OPNDS; opnd++ ) {
    ivs_table[0].opnd_source[opnd] = 0;
  }
}

void LOOP_IVS::Init( BB *body )
{
  // First, deallocate the ivs_table if not NULL.
  if (ivs_table) {
    CXX_DELETE_ARRAY( ivs_table, _loc_mem_pool );
  }
  
  // Count the number of OPs in the loop body.
  ivs_count = 1;
  OP *op;
  FOR_ALL_BB_OPs_FWD( body, op ) {
    ++ivs_count;
  }

  // Allocate a table of IVs_entry, with one entry for each loop OP.
  // ivs_table[0] is not used, because index 0 is used to indicate
  // constant and global operand TNs.
  ivs_table = (IVs_entry *) CXX_NEW_ARRAY( IVs_entry, ivs_count,
					   _loc_mem_pool );

  // The map tn_last_op remembers, for each TN, the index (in
  // ivs_table) of the most recent OP defining that TN
  hTN_MAP32 tn_last_op = hTN_MAP32_Create( _loc_mem_pool );

  // Initialize the ivs_table entries and the map tn_last_op.
  Init_IVs_Table(BB_first_op( body ), tn_last_op);
}

// ======================================================================
//
//  End of the implementation of the interface for class LOOP_IVS
//
// ======================================================================

#if 0
// ======================================================================
//
//  Implementation example for optimizations based on loop induction
//  variables
//
// ======================================================================

#if 0
INT
LOOP_IVS::Lookup_Op(OP *op)
{

  // TBD: Does not work when some operations have been removed from
  // the BB since the creation of ivs_table
  // Assumes table[0] is not used
  INT left  = 1;
  INT right = ivs_count-1;
  while ( left <= right ) {
    INT middle = ( left + right ) / 2;
    OP *middle_op = ivs_table[middle].op;
    if ( op == middle_op )
      return middle;
    if ( OP_Precedes( op, middle_op ) )
      right = middle - 1;
    else
      left = middle + 1;
  }
  return 0;
}
#endif

static void
Optimize_Loop_Induction_Variables( LOOP_DESCR *loop ) {

  if (BB_SET_Size(LOOP_DESCR_bbset(loop)) != 1)
    return;

  // Initialize memory pool for LOOP_IVS
  MEM_POOL local_mem_pool;
  MEM_POOL_Initialize( &local_mem_pool, "LOOP_IVS local pool", FALSE );
  MEM_POOL_Push( &local_mem_pool );

  LOOP_IVS loop_ivs( loop, &local_mem_pool );
  BB *body = LOOP_DESCR_loophead( loop );
  loop_ivs.Init( body );

  //  loop_ivs.Trace_IVs_Entries("Before optimizations");

  // Write here the code to optimize the IVs

  OP *op;
  int idx;
  FOR_ALL_LOOP_IVS_OPs_FWD( &loop_ivs, idx, op ) {
    INT op_idx = loop_ivs.Lookup_Op(op);
    Is_True(op_idx == idx, ("LOOP_IVS_Verify failed"));
    Print_OP(op);
    INT opndx;
    for (opndx = 0; opndx < OP_opnds(op); opndx++) {
      DefID_t iv_cycle = loop_ivs.OPND_IV_cycle(idx, opndx);
      fprintf(TFile, "\t");
      Print_TN(OP_opnd(op, opndx), 0);
      if (iv_cycle == 0)
	fprintf(TFile, ": not based on an induction variable.\n");
      else {
	INT offset = loop_ivs.OPND_IV_offset(idx, opndx);
	fprintf(TFile, ": IV = (%d,%d), offset = %d\n", DEFID_idx(iv_cycle), DEFID_res(iv_cycle), offset);
	if (DEFID_idx(iv_cycle) == op_idx) {
	  INT64 step = loop_ivs.IV_step(iv_cycle);
	  OP *op_init =  loop_ivs.IV_init(iv_cycle);
	  fprintf(TFile, "\tIV: step %lld, init ", step);
	  if (op_init)
	    Print_OP(op_init);
	  else
	    fprintf(TFile, "(null)\n");
	}
      }
    }
  }

  // Dispose memory pool for LOOP_IVS
  MEM_POOL_Pop( &local_mem_pool );
  MEM_POOL_Delete( &local_mem_pool );

  return;
}

void Perform_Induction_Variables_Optimizations() {
  MEM_POOL loop_descr_pool;
  MEM_POOL_Initialize(&loop_descr_pool, "loop_descriptors", TRUE);
  MEM_POOL_Push (&loop_descr_pool);
  BOOL trace_general = Get_Trace(TP_CGLOOP, 1);

  Calculate_Dominators();		/* needed for loop recognition */

  for (LOOP_DESCR *loop = LOOP_DESCR_Detect_Loops(&loop_descr_pool);
       loop;
       loop = LOOP_DESCR_next(loop)) {
    BB *head = LOOP_DESCR_loophead(loop);
	  
    Optimize_Loop_Induction_Variables(loop);
  }

  MEM_POOL_Pop (&loop_descr_pool);
  MEM_POOL_Delete(&loop_descr_pool);

  Free_Dominators_Memory ();
}
#endif

#ifdef TARG_ST200

// ======================================================================
//
//  An implementation of load-store packing based on the loop
//  induction variable module.
//
//  Perform_Load_Store_Packing looks for ldw and stw instructions that
//  access consecutive words in memory and pack them respectively into
//  ldp and stp instructions.  It will generate code to handle dynamic
//  alignment, or will produce more efficient code when the alignment
//  is known at compile time.
//
// ======================================================================

static
LOOPDEP Get_Loopdep_Kind(BB *bb) {
  BB *head;
  LOOPDEP kind = (LOOPDEP)0;

  // Get a tentative header of the loop containing bb
  head = bb;

  ANNOTATION *annot;
  WN *pragma = NULL;
  for (annot = ANNOT_Get(BB_annotations(head), ANNOT_PRAGMA);
       annot != NULL;
       annot = ANNOT_Get(ANNOT_next(annot), ANNOT_PRAGMA)) {
    if (WN_pragma(ANNOT_pragma(annot)) == WN_PRAGMA_IVDEP ||
	WN_pragma(ANNOT_pragma(annot)) == WN_PRAGMA_LOOPDEP) {
      pragma = ANNOT_pragma(annot);
    }
  }

  if (pragma) {
    if (WN_pragma(pragma) == WN_PRAGMA_LOOPDEP) {
      kind = (LOOPDEP)WN_pragma_arg1(pragma);
    } else if (WN_pragma(pragma) == WN_PRAGMA_IVDEP) {
      if (Liberal_Ivdep) kind = LOOPDEP_LIBERAL;
      else if (Cray_Ivdep) kind = LOOPDEP_VECTOR;
      else kind = LOOPDEP_PARALLEL;
    }
  }
  return kind;
}

static BOOL
LoadStore_Packing_Candidate_Op( OP *op )
{
  if ( OP_has_implicit_interactions( op ) ||
      (OP_opnds( op ) > OP_MAX_FIXED_OPNDS) ||
       (OP_results( op ) > OP_MAX_FIXED_RESULTS) )
    return FALSE;

  if ( !OP_load( op ) && !OP_store( op ) )
    return FALSE;

  if (OP_load(op) && ((CG_LOOP_load_store_packing&0x1) == 0))
    return FALSE;

  if (OP_store(op) && ((CG_LOOP_load_store_packing&0x2) == 0))
    return FALSE;

  if ( OP_unalign_store( op ) ||
       OP_unalign_ld( op )    ||

       // Skip predicated operations
       OP_cond_def( op )      ||

       OP_Mem_Ref_Bytes(op) != 4 )
    return FALSE;

  return TRUE;
}

static BOOL
Packing_Load_Candidate_Arc( ARC *arc )
{
  return ( ARC_kind( arc ) != CG_DEP_MEMIN &&
	   //	   ARC_kind( arc ) != CG_DEP_MEMOUT &&
	   (ARC_kind( arc ) != CG_DEP_MEMANTI || ARC_is_definite(arc)) &&
	   ARC_kind( arc ) != CG_DEP_MEMVOL);
}

static BOOL
Packing_Store_Candidate_Arc( ARC *arc )
{
  return ( ARC_kind( arc ) != CG_DEP_MEMIN );
}

// Initialize the base symbol, the associated relocation, and the
// offset of this TN.

static void
Analyse_Offset_TN(TN *offset_tn, ST **st, INT *relocs, INT64 *val) {

  if (TN_is_symbol(offset_tn)) {
    ST *symbol = TN_var(offset_tn);
    Base_Symbol_And_Offset (symbol, st, val);
    *relocs = TN_relocs(offset_tn);
    *val += TN_offset(offset_tn);
    Is_True(0, ("Analyse_Offset_TN with TN_is_symbol: To be checked."));
  }
  else {
    *st = NULL;
    *relocs = 0;
    *val = TN_value(offset_tn);
  }
}

static TN *
Adjust_Offset_TN(TN *offset_tn, INT64 adjust) {

  TN *new_offset_tn = NULL;

  if (TN_is_symbol(offset_tn)) {
    ST *st = TN_var(offset_tn), *symbol;
    INT64 offset_val;
    Base_Symbol_And_Offset (st, &symbol, &offset_val);
    INT32 relocs = TN_relocs(offset_tn);
    offset_val += TN_offset(offset_tn);
    offset_val += adjust;
    new_offset_tn = Gen_Symbol_TN(symbol, offset_val, relocs);
    Is_True(0, ("Adjust_Offset_TN: To Be Checked."));
  }
  else {
    INT64 offset_val;
    offset_val = TN_value(offset_tn);
    new_offset_tn = Gen_Literal_TN(offset_val + adjust, 4);
  }

  return new_offset_tn;
}

static INT
Get_Pragma_Alignment(BB *loophead) {

  INT pragma_alignment = -1;

  ANNOTATION *align_ant = ANNOT_Get(BB_annotations(loophead), ANNOT_PRAGMA);
  while (align_ant && WN_pragma(ANNOT_pragma(align_ant)) != WN_PRAGMA_STREAM_ALIGNMENT)
    align_ant = ANNOT_Get(ANNOT_next(align_ant), ANNOT_PRAGMA);
  if (align_ant) {
    WN *wn = ANNOT_pragma(align_ant);
    pragma_alignment = WN_pragma_arg1(wn);
  }
  return pragma_alignment;
}

static INT
Pack32_Get_Alignment(OP *memop, INT64 offset) {

  // FdF 20050919: Try to collect the alignment for this memory
  // operation of the loop.

  BB *loophead = BB_loop_head_bb(OP_bb(memop));

  OP *use_base = memop;
  OP *def_base;

  TN *base_tn = OP_opnd(memop, TOP_Find_Operand_Use(OP_code(use_base), OU_base));
  INT64 base_offset = 0;
  ST *base_sym = NULL;
  DEF_KIND kind;

  // Look for definitions of the base_tn used on the load/store
  // operation. The following cases can occur:
  //   def_base == NULL : No reaching definition found
  //   def_base = TN + cst : Look for definition of TN
  //   def_base = $r12 + cst : base = $r12, offset += cst, break;
  //   def_base = $r12 + $sym : base = $r12, sym = $sym, break;
  //   def_base = $sym : break; base = NULL, sym = $sym, break;
  //   def_base = <other op> : def_base = NULL, break;

  // base_offset accumulates the offset from the start of the loop to
  // the variable on which the alignment is known. Use loophead to
  // ignore offsets from operations in the current loop.

  while ((def_base = TN_Reaching_Value_At_Op(base_tn, use_base, &kind, TRUE)) != NULL) {

    TN *opnd_tn = NULL;
    INT64 val;

    // Check if it is defined in another loop.
    if (BB_loop_head_bb(OP_bb(def_base)) && (BB_loop_head_bb(OP_bb(def_base)) != loophead)) {
      def_base = NULL;
      break;
    }

    if (OP_copy(def_base)) {
      base_tn = OP_opnd(def_base, OP_Copy_Operand(def_base));
      opnd_tn = Zero_TN;
    }
    else if (OP_iadd(def_base) || OP_isub(def_base)) {
      base_tn = OP_opnd(def_base, TOP_Find_Operand_Use(OP_code(def_base), OU_opnd1));
      opnd_tn = OP_opnd(def_base, TOP_Find_Operand_Use(OP_code(def_base), OU_opnd2));
    }
    else {
      def_base = NULL;
      break;
    }

    if (base_tn == SP_TN) {
      if (TN_is_symbol(opnd_tn)) {
	base_sym = TN_var(opnd_tn);
	base_offset += TN_offset(opnd_tn);
      }
      else if (TN_Value_At_Op(opnd_tn, def_base, &val)) {
	if (OP_isub(def_base)) val = -val;
	base_offset += val;
      }
      else
	def_base = NULL;
      break;
    }

    if (TN_is_symbol(base_tn)) {
      if (TN_Value_At_Op(opnd_tn, def_base, &val)) {
	if (OP_isub(def_base)) val = -val;
	base_offset += val;
	base_sym = TN_var(base_tn);
	base_tn = NULL;
      }
      else
	def_base = NULL;
      break;
    }

    if (TN_Value_At_Op(opnd_tn, def_base, &val)) {
      if (OP_isub(def_base)) val = -val;
      // Ignore offset from operations inside the loop, this offset
      // has already been taken into account
      if (BB_loop_head_bb(OP_bb(def_base)) != loophead)
	base_offset += val;
      use_base = def_base;
      continue;
    }

    def_base = NULL;
    break;
  }

  // Get exact alignment

  INT base_alignment = -1;
  if (def_base != NULL) {
    Is_True(BB_loop_head_bb(OP_bb(def_base)) != loophead, ("Pack32_Get_Alignment: Incorrect base for stream"));
    if (base_sym == NULL) {
      Is_True(base_tn == SP_TN, ("Pack32_Get_Alignment: only SP_TN+Symbol is recognized"));
      base_alignment = Stack_Alignment();
    }
    else {
      base_alignment = ST_alignment(base_sym);
    }
    //    fprintf(TFile, "alignment(=%d), ", base_alignment);
  }

  /* aligned_memop:
     -1 means no compile time alignment.
     0 means memop address % 8 is 0
     4 means memop address % 8 is 4
  */
  INT aligned_memop = -1;
  if ((base_alignment != -1) && ((base_alignment&7) == 0) && ((base_offset&3) == 0)) {
    Is_True((offset&3) == 0, ("Pack32_Get_Alignment: Incorrect offset for stream"));
    aligned_memop = (base_offset+offset)&4;
  }

  return aligned_memop;
}

static void
Generate_Select_Or_Copy (
  TN *dest_tn,
  TN *cond_tn,
  TN *true_tn,
  TN *false_tn,
  OPS *ops
)
{
  if (TN_is_constant(cond_tn))
    if (TN_value(cond_tn) == 0)
      Exp_COPY(dest_tn, false_tn, ops);
    else
      Exp_COPY(dest_tn, true_tn, ops);
  else
    Expand_Select(dest_tn, cond_tn, true_tn, false_tn, MTYPE_I4, FALSE, ops); 
}

/*
  Candidate_Memory_t

  - index is the index of this memory op in the Pack32_entry table.
  - offset is the offset from this memory op to the value of the
     induction variable it is based on at the beginning of an
     iteration.

  Additional information is attached to the first memory access of a
  stream:

  - count gives the number of load/store in the stream

  - alignment gives the alignment of the memory access for this
    load/store. -1 for dynamic alignment, 0 for static alignment 0%8,
    4 for static alignment 4%8

  - aligned gives whether the stream is aligned, misaligned, or
    unknown at each iteration of the loop. -1 for dynamic alignment, 0
    if statically unaligned, 1 if statically aligned.
*/


typedef struct {
  INT index;
  INT offset;
  INT count;
  INT alignment;
  INT aligned;
} Candidate_Memory_t;

static BOOL
Compare_Offsets(const void *p1, const void *p2) {
  const Candidate_Memory_t *cand1 = (Candidate_Memory_t *)p1;
  const Candidate_Memory_t *cand2 = (Candidate_Memory_t *)p2;
  return (cand1->offset < cand2->offset) ? -1 : (cand1->offset == cand2->offset) ? 0 : 1;
}

static BOOL
Compare_Offsets_neg(const void *p1, const void *p2) {
  const Candidate_Memory_t *cand1 = (Candidate_Memory_t *)p1;
  const Candidate_Memory_t *cand2 = (Candidate_Memory_t *)p2;
  return (cand1->offset > cand2->offset) ? -1 : (cand1->offset == cand2->offset) ? 0 : 1;
}

static BOOL
Combine_Adjacent_Loads( LOOP_IVS *loop_ivs, Candidate_Memory_t *memop_table)
{
  /* In the loop prolog, generate the following instructions,
   * depending on the alignment of base and offset:
   *
   *  (step > 0)
   *  offset&4=0, base&4=0 => aligned = 1 ;;                    ;; addr = addr  ;;
   *  offset&4=0, base&4=1 => aligned = 0 ;; t0 = ldw oft(@addr);; addr = addr+4;;
   *  offset&4=1, base&4=0 => aligned = 0 ;; t0 = ldw oft(@addr);; addr = addr+4;;
   *  offset&4=1, base&4=1 => aligned = 1 ;;                    ;; addr = addr  ;;
   *
   *  (step < 0)
   *  offset&4=0, base&4=0 => aligned = 0 ;; t0 = ldw oft(@addr);; addr = addr-8;;
   *  offset&4=0, base&4=1 => aligned = 1 ;;                    ;; addr = addr-4;;
   *  offset&4=1, base&4=0 => aligned = 1 ;;                    ;; addr = addr-4;;
   *  offset&4=1, base&4=1 => aligned = 0 ;; t0 = ldw oft(@addr);; addr = addr-8;;
   */

  INT memop_count = memop_table[0].count;

  if (Get_Trace(TP_CGLOOP, 0x10))
      fprintf(TFile, "<ivs packing> Combine adjacent %d Loads\n", memop_count);

  int load_index = memop_table[0].index;
  OP *load_op = loop_ivs->Op(load_index);

  DefID_t IV_index = loop_ivs->OPND_IV_cycle(load_index, TOP_Find_Operand_Use(OP_code(load_op), OU_base));
  int IV_offset = loop_ivs->OPND_IV_offset(load_index, TOP_Find_Operand_Use(OP_code(load_op), OU_base));;
  OP *IV_op = loop_ivs->Op(DEFID_idx(IV_index));
  TN *IV_addr_tn = OP_result(IV_op, DEFID_res(IV_index));

  /* Compute the aligned boolean */
  INT64 IV_step = loop_ivs->IV_step(IV_index);

  /* load_alignment:
     -1 means no compile time alignment.
     0 means load address % 8 is 0
     4 means load address % 8 is 4
  */
  INT load_alignment =  memop_table[0].alignment;

  /* span_iteration:
     -1 means that a ldp may span an iteration
      0 means that no ldp will span an iteration
      1 means that a ldp will span an iteration
  */
  INT span_iteration;
  switch (memop_table[0].aligned) {
  case -1:
    span_iteration = -1;
    break;
  case 0:
    span_iteration = 1;
    break;
  case 1:
    span_iteration = 0;
    break;
  }

  // Find loop prolog, check it is unique.
  Is_True(CG_LOOP_prolog != NULL, ("Incorrect loop: Missing loop prolog"));
  BB *body = OP_bb(load_op);

  /* We must change the base TN to be IV_addr_tn, since the TN
     used on the load operation may not be available in the
     prolog. This requires that the offset of the ldw is adjusted. */

  int base_idx = TOP_Find_Operand_Use(OP_code(load_op), OU_base);
  int offset_idx = TOP_Find_Operand_Use(OP_code(load_op), OU_offset);

  TN *offset_tn = OP_opnd(load_op, offset_idx);
  TN *ldw_offset_tn = Adjust_Offset_TN(offset_tn, IV_offset);


  /* Generate a load operation in the loop prolog if the ldp may or
     will span an iteration. */

  OP *point = BB_last_op(CG_LOOP_prolog);
  BOOL before = (point != NULL && OP_xfer(point));
  TN *t0 = NULL;

  if (span_iteration != 0) {

    OP *ldw_op = Dup_OP(load_op);
    t0 = Dup_TN(OP_result(load_op, 0));

    Set_OP_opnd(ldw_op, base_idx, IV_addr_tn);
    Set_OP_opnd(ldw_op, offset_idx, ldw_offset_tn);
    Set_OP_result(ldw_op, 0, t0);
    Copy_WN_For_Memory_OP (ldw_op, load_op);
    OP_srcpos(ldw_op) = OP_srcpos(load_op);

    BB_Insert_Op(CG_LOOP_prolog, point, ldw_op, before);
    point = ldw_op;
    before = FALSE;
  }

  /* Get the offset from the value of IV_addr_tn at the entry of the
     loop to the effective address of the first load operation. */
  TN *aligned;
  OPS ops = OPS_EMPTY;
  switch (span_iteration) {
  case -1: {
    INT64 ldw_offset = memop_table[0].offset;
    Is_True((ldw_offset & 3) == 0, ("Misaligned ldw"));
    TN *breg_tn = Gen_Register_TN (ISA_REGISTER_CLASS_integer, 4);
    Expand_Binary_And (breg_tn, IV_addr_tn, Gen_Literal_TN(4, 4), MTYPE_I4, &ops);
    aligned = Gen_Register_TN (ISA_REGISTER_CLASS_branch, 1);
    if ((IV_step > 0) == ((ldw_offset&4) != 0))
      Expand_Copy(aligned, NULL, breg_tn, &ops); // aligned = (breg_tn != 0)
    else
      Expand_Logical_Not(aligned, breg_tn, V_NONE, &ops); // aligned = (breg_tn == 0)
    break;
  }
  case 0:
    aligned = Gen_Literal_TN(1, 4);
    break;
  case 1:
    aligned = Gen_Literal_TN(0, 4);
    break;
  }

  /* Compute the addr expression at the loop entry. ldp_offset_tn is
     the expression of the address at loop entry, to be defined
     according to the aligned boolean. */

  TN *ldp_addr_tn = Dup_TN(IV_addr_tn);
  TN *ldp_offset_tn;

  TN *adjust_addr_tn = Dup_TN(IV_addr_tn);
  if (IV_step > 0) {
    Expand_Add(adjust_addr_tn, IV_addr_tn, Gen_Literal_TN(4, 4), MTYPE_I4, &ops);
    ldp_offset_tn = ldw_offset_tn;
  }
  else {
    Expand_Add(adjust_addr_tn, IV_addr_tn, Gen_Literal_TN(-4, 4), MTYPE_I4, &ops);
    ldp_offset_tn = Adjust_Offset_TN(ldw_offset_tn, -4);
  }

  Generate_Select_Or_Copy(ldp_addr_tn, aligned, IV_addr_tn, adjust_addr_tn, &ops);

  BB_Insert_Ops(CG_LOOP_prolog, point, &ops, before);

  // Now, update load operations by pairs
  TN *prev_tn = t0;

  for (int candidate_index = 0; candidate_index < memop_count; candidate_index += 2) {

    OP *load1_op = loop_ivs->Op(memop_table[candidate_index].index);
    OP *load2_op = loop_ivs->Op(memop_table[candidate_index+1].index);

    // TBD: Also update the dep graph

    // Replace the load operations in the loop.

    /* Replace the first load instruction:
     * tn1 = ldw ofst1($addr_tn)
     * by
     * (IV_step > 0)
     *   t1,t2 = ldp ofst1($addr_tn)
     * else
     *   t2,t1 = ldp ofst1($addr_tn)
     * tn1 = slct aligned, t1, prev_tn
     */

    OPS ops_ldp = OPS_EMPTY;

    TN *t64 = Gen_Register_TN (ISA_REGISTER_CLASS_integer, 8);
    TN *t1 = Dup_TN(OP_result(load_op, 0));
    TN *t2 = Dup_TN(OP_result(load_op, 0));

    Expand_Load(OPC_I8I8LDID, t64,
		ldp_addr_tn,
		ldp_offset_tn,
		&ops_ldp);
    OP_srcpos(OPS_first(&ops_ldp)) = OP_srcpos(load1_op);

    extern void Expand_Extract(TN *low_tn, TN *high_tn, TN *src_tn, OPS *ops);

    if (IV_step > 0)
      Expand_Extract(t1, t2, t64, &ops_ldp);
    else
      Expand_Extract(t2, t1, t64, &ops_ldp);

    /* Initialize the OP_omega for new operations. */
    CG_LOOP_Init_OPS(&ops_ldp);
    {
      OP *new_op;
      FOR_ALL_OPS_OPs_FWD(&ops_ldp, new_op) {
	// FdF 20051003: Do not copy unroll information on LDP, to
	// avoid dependencies to be optimized.
	if (OP_load(new_op)) {
	  Copy_WN_For_Memory_OP (new_op, load1_op);
	  for (int i = 0; i < OP_opnds(new_op); i++) {
	    if (OP_opnd(new_op, i) == ldp_addr_tn)
	      Set_OP_omega(new_op, i, 1);
	  }
	  // FdF 20051108: Copy unroll information on LDP when LOOPDEP
	  // is used. This is unsafe though.
	  if (Get_Loopdep_Kind(body) != 0) {
	    if (Get_Trace(TP_CGLOOP, 0x10) && candidate_index == 0)
	      fprintf(TFile, "<ivs packing> Optimistics dependences with LOOPDEP.\n");
	    Set_OP_unroll_bb(new_op, OP_unroll_bb(load1_op));
	    Set_OP_unrolling(new_op, OP_unrolling(load1_op));
	    Set_OP_orig_idx(new_op, OP_map_idx(load1_op));
	  }
	}
	else {
	  Set_OP_unroll_bb(new_op, OP_unroll_bb(load1_op));
	  Set_OP_unrolling(new_op, OP_unrolling(load1_op));
	  Set_OP_orig_idx(new_op, OP_map_idx(load1_op));
	}
      }
    }

    // Insert ops_ldp before load1 or load2, depending on which one
    // dominates the other.
    if (memop_table[candidate_index].index < memop_table[candidate_index+1].index)
      BB_Insert_Ops(body, load1_op, &ops_ldp, TRUE);
    else
      BB_Insert_Ops(body, load2_op, &ops_ldp, TRUE);

    OPS ops_ldw1 = OPS_EMPTY;
    Generate_Select_Or_Copy(OP_result(load1_op, 0), aligned, t1, prev_tn, &ops_ldw1);

    /* Initialize the OP_omega for new operations generated for the
       first ldp op. */
    CG_LOOP_Init_OPS(&ops_ldw1);
    if ((span_iteration != 0) && (candidate_index == 0)) {
      OP *new_op;
      FOR_ALL_OPS_OPs_FWD(&ops_ldw1, new_op) {
	for (int i = 0; i < OP_opnds(new_op); i++) {
	  if (OP_opnd(new_op, i) == prev_tn)
	    Set_OP_omega(new_op, i, 1);
	}
      }
    }

    BB_Insert_Ops(body, load1_op, &ops_ldw1, TRUE);

    /* Replace the second load instruction:
     * tn2 = ldw ofst2($addr_tn)
     * by
     * tn2 = slct aligned, t2, t1
     * t0 = t2
     */

    OPS ops_ldw2 = OPS_EMPTY;
    Generate_Select_Or_Copy(OP_result(load2_op, 0), aligned, t2, t1, &ops_ldw2);
    /* Initialize the OP_omega for new operations. */
    CG_LOOP_Init_OPS(&ops_ldw2);

    BB_Insert_Ops(body, load2_op, &ops_ldw2, TRUE);

    if ((candidate_index+2) < memop_count) {
      if (span_iteration != 0)
	prev_tn = t2;
    }
    else {
      OPS ops_last = OPS_EMPTY;
      if (span_iteration != 0)
	Expand_Copy(t0, NULL, t2, &ops_last);

      // Also, increment the ldp_base_addr
      Expand_Add(ldp_addr_tn, ldp_addr_tn, Gen_Literal_TN(IV_step, 4), MTYPE_I4, &ops_last);

      CG_LOOP_Init_OPS(&ops_last);
      {
	OP *new_op;
	FOR_ALL_OPS_OPs_FWD(&ops_last, new_op) {
	  for (int i = 0; i < OP_opnds(new_op); i++) {
	    if (OP_opnd(new_op, i) == ldp_addr_tn)
	      Set_OP_omega(new_op, i, 1);
	  }
	}
      }

      // Insert ops_last before load1 or load2, depending on which one
      // is dominated by the other.
      if (memop_table[candidate_index].index < memop_table[candidate_index+1].index)
	BB_Insert_Ops(body, load2_op, &ops_last, TRUE);
      else
	BB_Insert_Ops(body, load1_op, &ops_last, TRUE);
    }

    ldp_offset_tn = Adjust_Offset_TN(ldp_offset_tn, (IV_step > 0 ? 8 : -8));

    BB_Remove_Op(body, load1_op);
    BB_Remove_Op(body, load2_op);

  }

  return TRUE;
}

static BOOL
Combine_Adjacent_Stores( LOOP_IVS *loop_ivs, Candidate_Memory_t *memop_table )
{
  /* Stores must be statically correctly aligned. This means that for
     a step > 0, the first store must be aligned on 0%8, and aligned on
     4%8 for a step < 0. */

  INT memop_count = memop_table[0].count;

  int store_index = memop_table[0].index;
  OP *store_op = loop_ivs->Op(store_index);

  DefID_t IV_index = loop_ivs->OPND_IV_cycle(store_index, TOP_Find_Operand_Use(OP_code(store_op), OU_base));
  int IV_offset = loop_ivs->OPND_IV_offset(store_index, TOP_Find_Operand_Use(OP_code(store_op), OU_base));
  OP *IV_op = loop_ivs->Op(DEFID_idx(IV_index));
  TN *IV_addr_tn = OP_result(IV_op, DEFID_res(IV_index));

  Is_True(CG_LOOP_prolog != NULL, ("Incorrect loop: Missing loop prolog"));
  BB *body = OP_bb(store_op);

  int offset_idx = TOP_Find_Operand_Use(OP_code(store_op), OU_offset);
  TN *offset_tn = OP_opnd(store_op, offset_idx);

  TN *stw_offset_tn = Adjust_Offset_TN(offset_tn, IV_offset);

  INT64 IV_step = loop_ivs->IV_step(IV_index);

  /* store_alignment:
     -1 means no compile time alignment.
     0 means store address % 8 is 0
     4 means store address % 8 is 4
  */
  INT store_alignment = memop_table[0].alignment;
  Is_True( (IV_step > 0) == (store_alignment == 0),
	       ( "Combine_Adjacent_Stores: Illegal alignment for store operation" ) );

  if (Get_Trace(TP_CGLOOP, 0x10))
    fprintf(TFile, "<ivs packing> Combine adjacent %d Stores\n", memop_count);

  // Get the offset of the value of IV_addr_tn at the entry of the
  // loop to the effective address of the first store operation.
  TN *aligned = Gen_Literal_TN(1, 4);
  OPS ops = OPS_EMPTY;

  /* Compute the addr expression at the loop entry. stp_offset_tn is
     the expression of the address at loop entry, to be defined
     according to the aligned boolean. */

  TN *stp_addr_tn = Dup_TN(IV_addr_tn);
  Exp_COPY(stp_addr_tn, IV_addr_tn, &ops);
  OP *point = BB_last_op(CG_LOOP_prolog);
  BOOL before = (point != NULL && OP_xfer(point));
  BB_Insert_Ops(CG_LOOP_prolog, point, &ops, before);

  TN *stp_offset_tn;
  if (IV_step > 0)
    stp_offset_tn = stw_offset_tn;
  else
    stp_offset_tn = Adjust_Offset_TN(stw_offset_tn, -4);

  // Now, update store operations by pairs
  for (int candidate_index = 0; candidate_index < memop_count; candidate_index += 2) {

    OP *store1_op = loop_ivs->Op(memop_table[candidate_index].index);
    OP *store2_op = loop_ivs->Op(memop_table[candidate_index+1].index);

    // TBD: Also update the dep graph

    // Replace the store operations in the loop.

    /* Replace the last store instruction:
     * stw ofst2($addr_tn), t2
     * by
     * (IV_step > 0)
     *   stp ofst2($addr_tn), t1,t2
     * else
     *   stp ofst2($addr_tn), t2,t1
     */

    OPS ops_stp = OPS_EMPTY;

    TN *t64 = Gen_Register_TN (ISA_REGISTER_CLASS_integer, 8);
    TN *t1 = OP_opnd(store1_op, TOP_Find_Operand_Use(OP_code(store1_op), OU_storeval));
    TN *t2 = OP_opnd(store2_op, TOP_Find_Operand_Use(OP_code(store2_op), OU_storeval));

    extern void Expand_Compose(TN *src_tn, TN *low_tn, TN *high_tn, OPS *ops);

    if (IV_step > 0)
      Expand_Compose(t64, t1, t2, &ops_stp);
    else
      Expand_Compose(t64, t2, t1, &ops_stp);

    Expand_Store(MTYPE_I8,
		t64,		 
		stp_addr_tn,
		stp_offset_tn,
		&ops_stp);
    OP_srcpos(OPS_last(&ops_stp)) = OP_srcpos(store2_op);

    /* Initialize the OP_omega for new operations. */
    CG_LOOP_Init_OPS(&ops_stp);
    {
      OP *new_op;
      FOR_ALL_OPS_OPs_FWD(&ops_stp, new_op) {
	// FdF 20051003: Do not copy unroll information on STP, to
	// avoid dependencies to be optimized.
	if (OP_store(new_op)) {
	  Copy_WN_For_Memory_OP (new_op, store2_op);
	  for (int i = 0; i < OP_opnds(new_op); i++) {
	    if (OP_opnd(new_op, i) == stp_addr_tn)
	      Set_OP_omega(new_op, i, 1);
	  }
	  // FdF 20051108: Copy unroll information on STP when LOOPDEP
	  // is used. This is unsafe though.
	  if (Get_Loopdep_Kind(body) != 0) {
	    if (Get_Trace(TP_CGLOOP, 0x10) && candidate_index == 0)
	      fprintf(TFile, "<ivs packing> Optimistics dependences with LOOPDEP.\n");
	    Set_OP_unroll_bb(new_op, OP_unroll_bb(store1_op));
	    Set_OP_unrolling(new_op, OP_unrolling(store1_op));
	    Set_OP_orig_idx(new_op, OP_map_idx(store1_op));
	  }
	}
	else {
	  Set_OP_unroll_bb(new_op, OP_unroll_bb(store2_op));
	  Set_OP_unrolling(new_op, OP_unrolling(store2_op));
	  Set_OP_orig_idx(new_op, OP_map_idx(store2_op));
	}
      }
    }

    // Insert ops_stp before store1 or store2, depending on which one
    // dominates the other.
    if (memop_table[candidate_index].index < memop_table[candidate_index+1].index)
      BB_Insert_Ops(body, store2_op, &ops_stp, TRUE);
    else
      BB_Insert_Ops(body, store1_op, &ops_stp, TRUE);

    if ((candidate_index+2) == memop_count) {
      OPS ops_last = OPS_EMPTY;

      // Increment the stp_base_addr
      Expand_Add(stp_addr_tn, stp_addr_tn, Gen_Literal_TN(IV_step, 4), MTYPE_I4, &ops_last);

      CG_LOOP_Init_OPS(&ops_last);
      {
	OP *new_op;
	FOR_ALL_OPS_OPs_FWD(&ops_last, new_op) {
	  for (int i = 0; i < OP_opnds(new_op); i++) {
	    if (OP_opnd(new_op, i) == stp_addr_tn)
	      Set_OP_omega(new_op, i, 1);
	  }
	}
      }

      // Insert ops_last after load1 or load2, depending on which one
      // dominates the other.
      if (memop_table[candidate_index].index < memop_table[candidate_index+1].index)
	BB_Insert_Ops(body, store2_op, &ops_last, FALSE);
      else
	BB_Insert_Ops(body, store1_op, &ops_last, FALSE);
    }

    stp_offset_tn = Adjust_Offset_TN(stp_offset_tn, (IV_step > 0 ? 8 : -8));

    BB_Remove_Op(body, store1_op);
    BB_Remove_Op(body, store2_op);
  }

  return TRUE;
}

static void
Init_Packing_Candidates(LOOP_IVS *loop_ivs) {

  OP *op;
  INT opidx;
  FOR_ALL_LOOP_IVS_OPs_FWD(loop_ivs, opidx, op ) {
    if ( LoadStore_Packing_Candidate_Op( op ) )
      Set_OP_flag1( op );
    else
      Reset_OP_flag1( op );
  }

  // Search the CG dependence graph, there must be no alias between
  // operations candidates for load/store packing.
  FOR_ALL_LOOP_IVS_OPs_FWD( loop_ivs, opidx, op ) {
    if ( OP_flag1( op ) ) {
      if (OP_load(op)) {
	ARC_LIST *arcs = OP_preds( op );
	while ( arcs ) {
	  ARC *arc = ARC_LIST_first( arcs );
	  arcs = ARC_LIST_rest( arcs );

	  OP *op_pred = ARC_pred( arc );
	  if ( (BB_loop_head_bb(OP_bb(op_pred)) == BB_loop_head_bb(OP_bb(op))) &&
	       !Packing_Load_Candidate_Arc( arc ) ) {
	    Reset_OP_flag1(op);
	    Reset_OP_flag1(op_pred);
	  }
	}
      }
      else {
	ARC_LIST *arcs = OP_succs( op );
	while ( arcs ) {
	  ARC *arc = ARC_LIST_first( arcs );
	  arcs = ARC_LIST_rest( arcs );

	  OP *op_succ = ARC_succ( arc );
	  if ( (BB_loop_head_bb(OP_bb(op_succ)) == BB_loop_head_bb(OP_bb(op))) &&
	       !Packing_Store_Candidate_Arc( arc ) ) {
	    Reset_OP_flag1(op);
	    Reset_OP_flag1(op_succ);
	  }
	}
      }
    }
  }
}

static INT
Compute_Packing_IVs(LOOP_IVS *loop_ivs) {

  INT Candidate_count = 0;
  OP *op;
  INT opidx;
  FOR_ALL_LOOP_IVS_OPs_FWD( loop_ivs, opidx, op ) {
    if ( !OP_flag1( op ) )
      continue;
    int base_idx = TOP_Find_Operand_Use(OP_code(op), OU_base);
    DefID_t IV_index = loop_ivs->OPND_IV_cycle(opidx, base_idx);
    if (IV_index == 0) {
      Reset_OP_flag1(op);
      continue;
    }
    // Get the step of the induction variable
    INT64 step = loop_ivs->IV_step(IV_index);
    // It must be a multiple of a word
    if (step & 3) {
      Reset_OP_flag1(op);
      continue;
    }
    Candidate_count ++;
  }

  return Candidate_count;
}

static INT
Find_Consecutive_Memops(LOOP_IVS *loop_ivs, Candidate_Memory_t *memop_table) {

  OP *op;
  INT opidx;
  FOR_ALL_LOOP_IVS_OPs_FWD( loop_ivs, opidx, op ) {
    if ( !OP_flag1( op ) )  continue;

    int base_idx = TOP_Find_Operand_Use(OP_code(op), OU_base);
    DefID_t IV_index = loop_ivs->OPND_IV_cycle(opidx, base_idx);
    int IV_offset = loop_ivs->OPND_IV_offset(opidx, base_idx);

    INT64 step = loop_ivs->IV_step(IV_index);

    ST *symbol = NULL;
    INT32 relocs = 0;
    INT64 offset_val;
    
    int offset_idx = TOP_Find_Operand_Use(OP_code(op), OU_offset);
    TN *offset_tn = OP_opnd(op, offset_idx);

    Analyse_Offset_TN(offset_tn, &symbol, &relocs, &offset_val);

    // Check that the offset from loop entry to the memory access is a
    // multiple of 4
    if (((IV_offset+offset_val)&3) != 0)
      continue;

    Is_True(IV_index != 0, ("Inconsistency in Induction Variable"));

    int memop_count = (step > 0) ? step/4 : -step/4;

    INT memop_idx = 0;
    memop_table[memop_idx].index = opidx;

    // Offset is initialized to the offset of the base of the memory
    // from the value of the primary induction variable at loop entry
    // and the offset on this memory operation.
    memop_table[memop_idx].offset = IV_offset + offset_val;
    memop_idx ++;
    Reset_OP_flag1(op);

    // Then, look for step/4 32 bit operations, that can be packed
    // into step/8 64 bit operations

    OP *op2;
    INT opidx2;
    for (opidx2 = opidx+1; opidx2 < loop_ivs->Count(); opidx2 ++) {
      op2 = loop_ivs->Op(opidx2);
      // Look for a sequence of identical operations
      if ( !OP_flag1( op2 ) || (OP_code(op) != OP_code(op2)))
	continue;
      // Get the step of the induction variable.
      DefID_t IV_index2 = loop_ivs->OPND_IV_cycle(opidx2, base_idx);
      int IV_offset2 = loop_ivs->OPND_IV_offset(opidx2, base_idx);;

      // This operation must be based on the same IV as the reference one.
      if (IV_index != IV_index2)
	continue;

      TN *offset2_tn = OP_opnd(op2, offset_idx);

      INT64 offset2_val;
      ST *symbol2 = NULL;
      INT32 relocs2 = 0;

      Analyse_Offset_TN(offset2_tn, &symbol2, &relocs2, &offset2_val);

      if (symbol != symbol2 || relocs != relocs2)
	continue;

      // Check that the offset from loop entry to the memory access is a
      // multiple of 4
      if (((IV_offset2+offset2_val)&3) != 0)
	continue;

      if (memop_idx < memop_count) {
	memop_table[memop_idx].index = opidx2;
	memop_table[memop_idx].offset = IV_offset2 + offset2_val;
      }
      memop_idx++;
      Reset_OP_flag1(op2);
    }

    // Not the exact number of memory accesses
    if (memop_idx != memop_count)
      continue;

    // Sort the operations in increasing/decreasing order of the
    // memory accesses.

    if (step > 0)
      qsort(memop_table, memop_count, sizeof(Candidate_Memory_t), Compare_Offsets);
    else
      qsort(memop_table, memop_count, sizeof(Candidate_Memory_t), Compare_Offsets_neg);

    // Check that they are all contiguous.
    int incr = step > 0 ? 4 : -4;
    int i;
    for (i = 1; i < memop_count; i++) {
      if (memop_table[i].offset != (memop_table[i-1].offset+incr))
	break;
    }
    if (i != memop_count)
      continue;

    int alignment = Pack32_Get_Alignment(loop_ivs->Op(memop_table[0].index), memop_table[0].offset);
    // Look for an option or a #pragma stream_alignment
    if (alignment == -1) {
      BB *loophead = BB_loop_head_bb(OP_bb(loop_ivs->Op(memop_table[0].index)));
      int pragma_alignment = Get_Pragma_Alignment(loophead);
      if (((pragma_alignment&0x7) == 0) ||
	  ((pragma_alignment == -1) && ((CG_LOOP_stream_align&0x7) == 0)))
	alignment = (step > 0) ? 0 : 4;
    }
    memop_table[0].count = memop_count;
    memop_table[0].alignment = alignment;
    memop_table[0].aligned = (alignment == -1) ? -1 : (step > 0) == (alignment == 0) ? 1 : 0;

    return memop_count;
  }

  memop_table[0].count = 0;
  return 0;
}

static BOOL
LoadStore_Packing( LOOP_IVS *loop_ivs, CG_LOOP &cg_loop )
{
  BOOL packing_done = FALSE;

  BB *body = LOOP_DESCR_loophead(cg_loop.Loop());
  loop_ivs->Init( body );

  // Identify (set flag1 for) all OPs that are suitable candidates for
  // 64 bit load/store packing.

  Init_Packing_Candidates(loop_ivs);

  // Make a pass to initialize the IV variables for each candidate
  // load/store, and see if there are opportunities for packing.

  INT Candidate_count = Compute_Packing_IVs(loop_ivs);

  //  loop_ivs->Trace_IVs_Entries("LoadStore_Packing: after Compute_Packing_IVs");

  // Now, look for a sequence of load or store operations that can be
  // packed.
  if (Candidate_count <= 1) return FALSE;

  //  Trace_Pack32_Entries(pack32_table, pack32_count, "Before main loop");

  // Statistics
  int load_cnt = 0;
  int mem_cnt = 0;
  OP *op;
  FOR_ALL_BB_OPs(body, op) {
    if (OP_memory(op))
      mem_cnt ++;
    if (OP_load(op))
      load_cnt ++;
  }

  BB_MAP sch_est;
  CG_SCHED_EST *se;
  int latency_II = 0;
  if (Get_Trace(TP_CGLOOP, 0x10)) {
    sch_est = BB_MAP_Create();
    se = CG_SCHED_EST_Create(body, &MEM_local_nz_pool, SCHED_EST_FOR_UNROLL);
    BB_MAP_Set(sch_est, body, se); 
    latency_II = CG_SCHED_EST_Critical_Length(se);
  }

  se = CG_SCHED_EST_Create_Empty(&MEM_local_nz_pool, SCHED_EST_FOR_UNROLL);
  FOR_ALL_BB_OPs(body, op) {
    if (!OP_copy(op) && (OP_code(op) != TOP_extractp) && (OP_code(op) != TOP_composep))
      CG_SCHED_EST_Add_Op_Resources(se, OP_code(op));
  }
  int resource_II = CG_SCHED_EST_Resource_Cycles(se);

  if ((CG_LOOP_load_store_packing & 0x40) && (mem_cnt < resource_II))
    return FALSE;

  INT load_stream_count = 0, store_stream_count = 0;

  // Allocate an array to store all memory operations that have a same
  // base IV and symbolic offset
  Candidate_Memory_t *memop_table
    = (Candidate_Memory_t *)alloca( (Candidate_count+1) * sizeof( Candidate_Memory_t ) );

  // Initialize the table with the streams
  INT memop_count;
  Candidate_Memory_t *candidate_stream = memop_table;
  while ((memop_count = Find_Consecutive_Memops(loop_ivs, candidate_stream)) != 0) {

    // Only paired of ldw/stw operations can be packed.
    if ((memop_count&1) == 0)
      candidate_stream += memop_count;
  }

  //  fprintf(TFile, "32 bit Packing = {");
  for ( candidate_stream = memop_table;
	candidate_stream->count > 0;
	candidate_stream += candidate_stream->count ) {

    //    fprintf(TFile, "%d, ", memop_count);

    if ((candidate_stream->alignment == -1) &&
	cg_loop.Peel_stream()) {
      OP *base_op = loop_ivs->Op(candidate_stream->index);

      // Check this is the stream on which the loop peeling applies
      if (OP_map_idx(cg_loop.Peel_op()) == OP_orig_idx(base_op) &&
	  OP_unrolling(base_op) == 0) {

	// Peeling has been applied. The alignment is now forced to
	// peel_align
	candidate_stream->alignment = (cg_loop.Peel_align()+candidate_stream->offset)&4;
	candidate_stream->aligned = 1; /* Peeling always aligns the stream. */
	if (Get_Trace(TP_CGLOOP, 0x10)) {
	  fprintf(TFile, "<ivs packing> Using static alignment %d after loop peeling.\n", candidate_stream->alignment);
	}
      }
    }
    else if ((candidate_stream->alignment == -1) &&
	     (cg_loop.Special_streams() > 0)) {
      // Find the stream
      OP *base_op = loop_ivs->Op(candidate_stream->index);
      INT orig_idx = OP_unroll_bb(base_op) ? OP_orig_idx(base_op) : OP_map_idx(base_op);
      for (int idx = 0; idx < cg_loop.Special_streams(); idx++) {
	if ((OP_map_idx(cg_loop.Special_stream_op(idx)) == orig_idx) &&
	    (OP_unrolling(base_op) == 0)) {
	  candidate_stream->alignment = (cg_loop.Special_stream_align(idx)+candidate_stream->offset)&4;
	  candidate_stream->aligned = 1; /* Specialization always aligns the stream. */
	  if (Get_Trace(TP_CGLOOP, 0x10)) {
	    fprintf(TFile, "<ivs packing> Using static alignment %d after loop specialization.\n", candidate_stream->alignment);
	  }
	  break;
	}
      }
    }

    if (OP_load(loop_ivs->Op(candidate_stream->index)) && ((CG_LOOP_load_store_packing&0x4) != 0)) {
      // Check also that alignment is statically known
      if (candidate_stream->alignment == -1)
	continue;
    }
    else if (OP_store(loop_ivs->Op(candidate_stream->index))) {
      if (candidate_stream->aligned != 1)
	continue;
    }

    // Generate the code to pair load/store together.
    if (OP_load(loop_ivs->Op(candidate_stream->index))) {
      if (Get_Trace(TP_CGLOOP, 0x10)) {
	const char *align_kind = (candidate_stream->alignment == -1) ? "select" : "static";
	fprintf(TFile, "<ivs packing> Load stream, %d memops, alignment %s(%d)\n", candidate_stream->count, align_kind, candidate_stream->alignment);
      }
      if (Combine_Adjacent_Loads(loop_ivs, candidate_stream)) {
	load_stream_count ++;
	packing_done = TRUE;
      }
    }
    else {
      if (Get_Trace(TP_CGLOOP, 0x10)) {
	fprintf(TFile, "<ivs packing> Store stream, %d memops, alignment static(%d)\n", candidate_stream->count, candidate_stream->alignment);
      }
      if (Combine_Adjacent_Stores(loop_ivs, candidate_stream)) {
	store_stream_count ++;
	packing_done = TRUE;
      }
    }
  }

  //  fprintf(TFile, "}\n");
  if (Get_Trace(TP_CGLOOP, 0x10)) {
    fprintf(TFile, "<ivs packing> after unrolling Load stream %d, Store stream %d, Peeling %d, Special %d\n", load_stream_count, store_stream_count, cg_loop.Peel_stream(), (cg_loop.Special_streams() > 0));
    fprintf(TFile, "<ivs packing> latency_II %d, resource_II %d %s(memop=%d)\n", latency_II, resource_II, (mem_cnt == resource_II) ? "(memory bounded)" : "", mem_cnt);

    mem_cnt = 0;
    load_cnt = 0;
    FOR_ALL_BB_OPs(body, op) {
      if (OP_memory(op))
	mem_cnt ++;
      if (OP_load(op))
	load_cnt ++;
    }

    se = CG_SCHED_EST_Create(body, &MEM_local_nz_pool, SCHED_EST_FOR_UNROLL);
    BB_MAP_Set(sch_est, body, se);
    latency_II = CG_SCHED_EST_Critical_Length(se);

    se = CG_SCHED_EST_Create_Empty(&MEM_local_nz_pool, SCHED_EST_FOR_UNROLL);
    FOR_ALL_BB_OPs(body, op) {
      if (!OP_copy(op) && (OP_code(op) != TOP_extractp) && (OP_code(op) != TOP_composep))
	CG_SCHED_EST_Add_Op_Resources(se, OP_code(op));
    }
    resource_II = CG_SCHED_EST_Resource_Cycles(se);

    fprintf(TFile, "<ivs packing> new_latency_II %d, new_resource_II %d %s(memop=%d)\n", latency_II, resource_II, (mem_cnt == resource_II) ? "(memory bounded)" : "", mem_cnt);
  }

  return packing_done;
}

static BOOL
LoadStore_Check_Packing( LOOP_IVS *loop_ivs, CG_LOOP &cg_loop )
{
  BB *body = LOOP_DESCR_loophead(cg_loop.Loop());
  loop_ivs->Init( body );

  // Identify (set flag1 for) all OPs that are suitable candidates for
  // 64 bit load/store packing.

  Init_Packing_Candidates(loop_ivs);

  // Make a pass to initialize the IV variables for each candidate
  // load/store, and see if there are opportunities for packing.

  INT Candidate_count = Compute_Packing_IVs(loop_ivs);

  // Now, look for a sequence of load or store operations that can be
  // packed.
  if (Candidate_count < 1) return FALSE;

  //  Trace_Pack32_Entries(pack32_table, pack32_count, "Before main loop");

  // Allocate an array to store all memory operations that have a same
  // base IV and symbolic offset
  Candidate_Memory_t *memop_table
    = (Candidate_Memory_t *)alloca( (Candidate_count+1) * sizeof( Candidate_Memory_t ) );

  INT memop_count;
  Candidate_Memory_t *candidate_stream;

  INT stream_count = 0;
  INT load_stream_count = 0, store_stream_count = 0;
  INT static_aligned_load_count = 0, static_aligned_store_count = 0;
  INT misaligned_load_count = 0, misaligned_store_count = 0;

  // Initialize the table with the streams
  for (candidate_stream = memop_table;
       (memop_count = Find_Consecutive_Memops(loop_ivs, candidate_stream)) != 0;
       candidate_stream += memop_count) {

    stream_count ++;

    if (OP_load(loop_ivs->Op(candidate_stream->index))) {
      load_stream_count ++;
      if (candidate_stream->alignment != -1)
	static_aligned_load_count ++;
      if (candidate_stream->aligned == 0)
	misaligned_load_count ++;

      if (Get_Trace(TP_CGLOOP, 0x10)) {
	const char *align_kind = (candidate_stream->alignment == -1) ? "select" : "static";
	fprintf(TFile, "<ivs packing> Load stream, %d memops, alignment %s(%d)\n", candidate_stream->count, align_kind, candidate_stream->alignment);
      }

    }
    else {
      store_stream_count ++;
      if (candidate_stream->alignment != -1)
	static_aligned_store_count ++;
      if (candidate_stream->aligned == 0)
	misaligned_store_count ++;

      if (Get_Trace(TP_CGLOOP, 0x10)) {
	const char *align_kind = (candidate_stream->aligned == 1) ? "static" : "discard";
	fprintf(TFile, "<ivs packing> Store stream, %d memops, alignment %s(%d)\n", candidate_stream->count, align_kind, candidate_stream->alignment);
      }

    }
  }

  // Check there is at least one stream candidate for packing
  if (stream_count == 0)
    return FALSE;

  BOOL doPeeling = FALSE;
  BOOL doSpecialize = FALSE;

  // Peeling can be applied if there is only one load stream with
  // unknown alignment, and other streams are store streams.

  if ((load_stream_count == 1) &&
      (static_aligned_load_count == 0)) {
    // Look for this load stream
    for (candidate_stream = memop_table;
	 candidate_stream->count > 0;
	 candidate_stream += candidate_stream->count)
      if (OP_load(loop_ivs->Op(candidate_stream->index)))
	break;
    // Peeling can be applied only on a stream with one memory operation
    if (candidate_stream->count == 1)
      doPeeling = TRUE;
  }

  // Peeling can also be applied if there is only one store stream,
  // with unknown or misaligned alignment.

  else if ((load_stream_count == 0) && (store_stream_count == 1) &&
	   ((static_aligned_store_count == 0) || (misaligned_store_count == 1))) {
    candidate_stream = memop_table;
    // Peeling can be applied only on a stream with one memory operation
    if (candidate_stream->count == 1)
      doPeeling = TRUE;
  }

  // In case the loop is not candidate for peeling, we can do loop
  // specialization if statically aligned store streams are well
  // aligned, and there are streams with unknown alignment.

  if ((CG_LOOP_load_store_packing&0x80) && !doPeeling &&
      (misaligned_store_count == 0) &&
      ((static_aligned_load_count + static_aligned_store_count) != stream_count)) {
    // Currently, only up to 4 streams can be managed for loop specialization
    INT dynamic_streams = stream_count - static_aligned_load_count - static_aligned_store_count;

    if (dynamic_streams > 4) {
      if (Get_Trace(TP_CGLOOP, 0x10))
	fprintf(TFile, "<ivs packing> Specialization cannot be applied on loops with more than 4(%d) dynamic streams.\n", dynamic_streams);
    }
    // Otherwise, mark the loop for specialization
    else
      doSpecialize = TRUE;
  }

  // Chek if the loop iterates enough for transformations such as
  // peeling or specialization to be profitable.
  INT loop_iter = 100;

  ANNOTATION *annot = ANNOT_Get(BB_annotations(body), ANNOT_LOOPINFO);
  LOOPINFO *info = ANNOT_loopinfo(annot);
  WN *wn = WN_COPY_Tree(LOOPINFO_wn(info));
  TN *trip_count_tn = LOOPINFO_trip_count_tn(info);

  if (trip_count_tn != NULL && TN_has_value(trip_count_tn))
    loop_iter = TN_value(trip_count_tn);
  else if (LOOPINFO_wn(info))
    loop_iter = WN_loop_trip_est(LOOPINFO_wn(info));

  if ((doPeeling || doSpecialize) && (loop_iter <= 16)) {
    if (Get_Trace(TP_CGLOOP, 0x10)) {
      const char *text = doPeeling ? "Peeling" : "Specialization";
      fprintf(TFile, "<ivs packing> %s cannot be applied on loops with less or equal to 16 iterations.\n", text);
    }
  }
  else if (doPeeling) {
    // Mark the loop for peeling
    int memop_index = candidate_stream->index;
    OP *base_op = loop_ivs->Op(memop_index);
    DefID_t IV_index = loop_ivs->OPND_IV_cycle(memop_index, TOP_Find_Operand_Use(OP_code(base_op), OU_base));
    INT64 IV_step = loop_ivs->IV_step(IV_index);

    cg_loop.Set_peel_op(base_op);
    // TBD: check peel_tn is available at loop entry
    Is_True(OP_opnd(base_op, TOP_Find_Operand_Use(OP_code(base_op), OU_base)) == OP_result(loop_ivs->Op(DEFID_idx(IV_index)), DEFID_res(IV_index)),
	    ("Loop peeling requires that the base adress is available at loop entry."));
    cg_loop.Set_peel_tn(OP_opnd(base_op, TOP_Find_Operand_Use(OP_code(base_op), OU_base)));
    cg_loop.Set_even_factor();
    cg_loop.Set_remainder_after();

    int aligned = (IV_step > 0) ? 0 : 4;
    int alignment = candidate_stream->alignment;

    if ((candidate_stream->offset&4) == 0) {
      cg_loop.Set_peel_stream(alignment == -1);
      cg_loop.Set_peel_align(aligned);
    }
    else if ((candidate_stream->offset&4) == 4) {
      cg_loop.Set_peel_stream(alignment == -1);
      cg_loop.Set_peel_align(4-aligned);
    }
  }
  else if (doSpecialize) {

    if (Get_Trace(TP_CGLOOP, 0x10)) {
      fprintf(TFile, "<ivs packing> Loop can be specialized\n");

    for (candidate_stream = memop_table;
	 candidate_stream->count > 0;
	 candidate_stream += candidate_stream->count) {

      if (candidate_stream->count&1)
	cg_loop.Set_even_factor();

      if (candidate_stream->alignment == -1) {

	int memop_index = candidate_stream->index;
	OP *base_op = loop_ivs->Op(memop_index);
        DefID_t IV_index = loop_ivs->OPND_IV_cycle(memop_index, TOP_Find_Operand_Use(OP_code(base_op), OU_base));
	INT64 IV_step = loop_ivs->IV_step(IV_index);
	int aligned = (IV_step > 0) == ((candidate_stream->offset&4) == 0) ? 0 : 4;

	cg_loop.Push_special_stream(base_op, OP_opnd(base_op, TOP_Find_Operand_Use(OP_code(base_op), OU_base)), aligned);
      }
    }
    cg_loop.Set_remainder_after();
  }

  // In case loop peeling or loop specialization is not performed,
  // count the number of streams that could eventually be packed, and
  // set Even_factor and/or Remainder_after if needed.

  if (cg_loop.Special_streams() == 0) {
    stream_count = 0;
    for (candidate_stream = memop_table;
	 candidate_stream->count > 0;
	 candidate_stream += candidate_stream->count) {

      // For load streams, all or only static aligned ones will be packed.
      if (OP_load(loop_ivs->Op(candidate_stream->index)) &&
	  (candidate_stream->alignment == -1) &&
	  (CG_LOOP_load_store_packing&0x4) != 0)
	continue;

      // For store streams, only statically well aligned streams will
      // be packed.
      if (OP_store(loop_ivs->Op(candidate_stream->index)) &&
	  (candidate_stream->aligned != 1))
	continue;

      stream_count ++;

      // If one of the stream contains an odd number of memory access,
      // set the even_factor.
	
      if (candidate_stream->count&1)
	cg_loop.Set_even_factor();

      // If one of the stream is statically aligned, set the
      // remainder_after flag.
      if (candidate_stream->alignment != -1)
	cg_loop.Set_remainder_after();
    }
  }
  // Check if there is at least one stream candidate for packing
  if (stream_count == 0)
    return FALSE;

  if (Get_Trace(TP_CGLOOP, 0x10)) {
    if (cg_loop.Even_factor()) {
      fprintf(TFile, "<ivs packing> Loop requires even unroll factor\n");
    }
    fprintf(TFile, "<ivs packing> before unrolling: Load stream %d, Store stream %d, Peeling %d, Special %d\n", load_stream_count, store_stream_count, cg_loop.Peel_stream(), (cg_loop.Special_streams() > 0));
  }

  return TRUE;
}

BOOL IVS_Perform_Load_Store_Packing( CG_LOOP &cg_loop )
{
  extern BOOL Enable_64_Bits_Ops;
  if (!Enable_64_Bits_Ops)
    return FALSE;

  LOOP_DESCR *loop = cg_loop.Loop();

  // Initialize memory pool for LOOP_IVS
  MEM_POOL local_mem_pool;
  MEM_POOL_Initialize( &local_mem_pool, "LOOP_IVS local pool", FALSE );
  MEM_POOL_Push( &local_mem_pool );

  BOOL changed_loop = FALSE;
  {
    // Invoke Read/CSE/Write Removal
    BB *head = LOOP_DESCR_loophead( loop );

    BOOL save_CG_DEP_Addr_Analysis = CG_DEP_Addr_Analysis;
    CG_DEP_Addr_Analysis = FALSE;

    // Derive dependence graph for loop body
    CG_DEP_Compute_Graph( head, NO_ASSIGNED_REG_DEPS, CYCLIC,
			  INCLUDE_MEMREAD_ARCS, INCLUDE_MEMIN_ARCS,
			  NO_CONTROL_ARCS, NULL );

    LOOP_IVS loop_ivs( loop, &local_mem_pool );
    changed_loop = LoadStore_Packing( &loop_ivs, cg_loop );

    CG_DEP_Addr_Analysis = save_CG_DEP_Addr_Analysis;
    CG_DEP_Delete_Graph( head );
  }

  // Dispose memory pool for LOOP_IVS
  MEM_POOL_Pop( &local_mem_pool );
  MEM_POOL_Delete( &local_mem_pool );

  if ( changed_loop )
    CG_LOOP_Recompute_Liveness( loop );

  return changed_loop;
}

BOOL IVS_Analyze_Load_Store_Packing( CG_LOOP &cg_loop )
{
  extern BOOL Enable_64_Bits_Ops;
  if (!Enable_64_Bits_Ops)
    return FALSE;

  BOOL can_be_packed = FALSE;
  LOOP_DESCR *loop = cg_loop.Loop();

  // Initialize memory pool for LOOP_IVS
  MEM_POOL local_mem_pool;
  MEM_POOL_Initialize( &local_mem_pool, "LOOP_IVS local pool", FALSE );
  MEM_POOL_Push( &local_mem_pool );

  {
    BB *head = LOOP_DESCR_loophead( loop );

    BOOL save_CG_DEP_Addr_Analysis = CG_DEP_Addr_Analysis;
    CG_DEP_Addr_Analysis = FALSE;

    // Derive dependence graph for loop body
    CG_DEP_Compute_Graph( head, NO_ASSIGNED_REG_DEPS, CYCLIC,
			  INCLUDE_MEMREAD_ARCS, INCLUDE_MEMIN_ARCS,
			  NO_CONTROL_ARCS, NULL );

    LOOP_IVS loop_ivs( loop, &local_mem_pool );
    can_be_packed = LoadStore_Check_Packing( &loop_ivs, cg_loop );

    CG_DEP_Addr_Analysis = save_CG_DEP_Addr_Analysis;
    CG_DEP_Delete_Graph( head );
  }

  // Dispose memory pool for LOOP_IVS
  MEM_POOL_Pop( &local_mem_pool );
  MEM_POOL_Delete( &local_mem_pool );

  return can_be_packed;
}
#endif
