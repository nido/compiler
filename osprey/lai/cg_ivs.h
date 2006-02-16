/*
  Copyright (C) 2001, STMicroelectronics, All Rights Reserved.

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


//-*-c++-*-
// =======================================================================
// =======================================================================
//
//  Module: cg_ivs.h
//
//  Description:
//  ============

//  This module consists of Loop Induction Variables analysis and
//  optimization
//
// 
//  Exported Functions:
//  ===================
//
//  void Perform_Induction_Variables_Optimizations();
//    Performs Loop Induction Variables optmization on all loops
//
// LOOP_IVS::LOOP_IVS(LOOP_DESCR *loop, MEM_POOL *mem_pool)
//    Constructor for a new LOOP_IVS instance
//
//  LOOP_IVS::~LOOP_IVS(void)
//    Destructor for a LOOP_IVS instance
//
//  void LOOP_IVS::Init( BB *body )
//    Initializes a LOOP_IVS instance for a Single_BB loop
//
//  INT Lookup_Op(OP *op);
//    Returns an ID for an OP in the LOOP_IVS instance.
//
//  DefID_t Get_IV_cycle(OP *op, INT opnd_idx, INT *offset);
//    Returns an ID for an Induction Variable. Initializes offset to
//    be the constant value from this use to the value of the IV at
//    the beginnning of a loop iteration.
//
//  void Get_IV_desc(DefID_t iv_cycle, TN **step, OP **init);
//    Get the step and the init operation for an IV
//
//  void Trace_IVs_Entries(const char *message);
//    Dump some traces for an LOOP_IVS instance.
//
// =======================================================================
// ======================================================================= */

#include "bb.h"
#include "cg_loop.h"
#include "findloops.h"

#ifndef CG_IVS_INCLUDED
#define CG_IVS_INCLUDED

// A use-def link is represented as an integer composed with:
//  28 bits for the index of the op in the IVS entries
//   4 bits for the index of the def in the op
typedef unsigned int DefID_t;
static inline DefID_t DEFID_make(UINT op_idx, UINT res_idx) { return (op_idx << 4 | res_idx); };
static inline UINT DEFID_idx(DefID_t defid) { return (defid >> 4); }
static inline UINT DEFID_res(DefID_t defid) { return (defid & 0xf); }

class LOOP_IVS {
 private:

  // For an operation op, attach use-def links on operands, and IV
  // information on results.
  typedef struct IVs_entry {
    OP    *op;
    /* For each result */
    DefID_t IV_cycle[OP_MAX_FIXED_RESULTS];
    INT     IV_offset[OP_MAX_FIXED_RESULTS];
    /* For each argument */
    DefID_t opnd_source[OP_MAX_FIXED_OPNDS];
  } IVs_entry_t;

  MEM_POOL *_loc_mem_pool;

  IVs_entry_t *ivs_table;
  INT ivs_count;

  IVs_entry_t& DEFID_entry(DefID_t defid) { return ivs_table[DEFID_idx(defid)]; }
  OP *DEFID_op(DefID_t defid) { return ivs_table[DEFID_idx(defid)].op; }
  DefID_t OPND_defid(INT op_idx, INT opnd_idx) { return ivs_table[op_idx].opnd_source[opnd_idx]; }
  INT OPND_omega(INT op_idx, INT opnd_idx) { return DEFID_idx(OPND_defid(op_idx, opnd_idx)) >= op_idx; }
  INT OPND_omega(IVs_entry_t *ivs_entry, INT opnd_idx) { return OPND_omega(ivs_entry-ivs_table, opnd_idx); }

  // TBD: Must return 0 when IV_offset is used to represent the step
  // of the induction variable
  DefID_t IV_cycle(DefID_t defid) { return ivs_table[DEFID_idx(defid)].IV_cycle[DEFID_res(defid)]; }
  INT IV_offset(DefID_t defid) { return ivs_table[DEFID_idx(defid)].IV_offset[DEFID_res(defid)]; }

  void Set_IV_cycle(DefID_t defid_src, DefID_t defid_cycle) {
    ivs_table[DEFID_idx(defid_src)].IV_cycle[DEFID_res(defid_src)] = defid_cycle;
  }
  void Set_IV_offset(DefID_t defid_src, INT offset) {
    ivs_table[DEFID_idx(defid_src)].IV_offset[DEFID_res(defid_src)] = offset;
  }

  void Init_IVs_Table(OP *first_op, hTN_MAP32 tn_last_op);
  DefID_t Find_IV( INT use_index, INT opnd_idx );

 public:

  LOOP_IVS(LOOP_DESCR *loop, MEM_POOL *mem_pool)
    : _loc_mem_pool(mem_pool)  {}

  ~LOOP_IVS(void) {}

  void Init( BB *body );
  INT Count() { return ivs_count; }

  OP *Op(INT op_idx) {
    return ((op_idx > 0) && (op_idx < ivs_count)) ? ivs_table[op_idx].op : NULL;
  }

  INT Lookup_Op(OP *op);

  DefID_t OPND_IV_cycle(INT op_idx, INT opnd_idx);
  INT OPND_IV_offset(INT op_idx, INT opndx_idx);
  INT IV_step(DefID_t iv_cycle);
  OP *IV_init(DefID_t iv_cycle);

  void Trace_IVs_Entries(const char *message);
};

#define FOR_ALL_LOOP_IVS_OPs_FWD(loop_ivs, opidx, op) \
  for (op = (loop_ivs)->Op(opidx=1); opidx < (loop_ivs)->Count(); op = (loop_ivs)->Op(++opidx))

void Perform_Induction_Variables_Optimizations();

extern BOOL IVS_Analyze_Load_Store_Packing( CG_LOOP &cg_loop);
extern BOOL IVS_Perform_Load_Store_Packing( CG_LOOP &cg_loop );

#endif /* CG_IVS_INCLUDED */
