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
// LOOP_IVS::LOOP_IVS(MEM_POOL *mem_pool)
//    Constructor for a new LOOP_IVS instance
//
//  LOOP_IVS::~LOOP_IVS(void)
//    Destructor for a LOOP_IVS instance
//
//  void LOOP_IVS::Init( LOOP_DESCR *loop )
//    Initializes a LOOP_IVS instance for a Single_BB loop
//
//  DefID_t OPND_IV_cycle(INT op_idx, INT opnd_idx);
//    Returns an ID for an Induction Variable. 
//
//  INT OPND_IV_offset(INT op_idx, INT opnd_idx);
//    Returns the constant value from this use to the value of the IV
//    at the beginnning of a loop iteration.
//
//  INT IV_step(DefID_t iv_cycle);
//    Get the step of an IV
//
//  OP *IV_init(DefID_t iv_cycle);
//    Get the init operation of an IV
//
//  void Trace_IVs_Entries(const char *message);
//    Dump some traces for an LOOP_IVS instance.
//
// =======================================================================
// ======================================================================= */

#ifdef TARG_ST
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

    void initIV(INT res_idx) {
      IV_cycle[res_idx] = 0; IV_offset[res_idx] = 0;
    }
    void setNotIV(INT res_idx)  {
      IV_cycle[res_idx] = 0; IV_offset[res_idx] = 1;
    }
    void setIV(INT res_idx, DefID_t iv_cycle, INT iv_offset)  {
      IV_cycle[res_idx] = iv_cycle; IV_offset[res_idx] = iv_offset;
    }
    BOOL isIV(INT res_idx) {
      return (IV_cycle[res_idx] > 0);
    }
    BOOL notIV(INT res_idx) {
      return (IV_cycle[res_idx] == 0) && (IV_offset[res_idx] != 0);
    }
  } IVs_entry_t;

  MEM_POOL *_loc_mem_pool;

  IVs_entry_t *ivs_table;
  INT ivs_count;

  IVs_entry_t& DEFID_entry(DefID_t defid) { return ivs_table[DEFID_idx(defid)]; }
  OP *DEFID_op(DefID_t defid) { return ivs_table[DEFID_idx(defid)].op; }

  // Return whether the use-def link crosses an iteration or not.
  BOOL OPND_hasOmega(INT op_idx, INT opnd_idx) { return DEFID_idx(OPND_defid(op_idx, opnd_idx)) >= op_idx; }
  BOOL OPND_hasOmega(IVs_entry_t *ivs_entry, INT opnd_idx) { return OPND_hasOmega(ivs_entry-ivs_table, opnd_idx); }

  void DEFID_setIV(DefID_t defid, DefID_t iv_cycle, INT iv_offset) {
    ivs_table[DEFID_idx(defid)].setIV(DEFID_res(defid), iv_cycle, iv_offset);
  }
  void DEFID_setNotIV(DefID_t defid) { ivs_table[DEFID_idx(defid)].setNotIV(DEFID_res(defid)); }
  BOOL DEFID_isIV(DefID_t defid) { return ivs_table[DEFID_idx(defid)].isIV(DEFID_res(defid)); }
  BOOL DEFID_notIV(DefID_t defid) { return ivs_table[DEFID_idx(defid)].notIV(DEFID_res(defid)); }

  DefID_t IV_cycle(DefID_t defid) { return ivs_table[DEFID_idx(defid)].IV_cycle[DEFID_res(defid)]; }
  INT IV_offset(DefID_t defid) { return ivs_table[DEFID_idx(defid)].IV_offset[DEFID_res(defid)]; }

  void Init_IVs_Table(OP *first_op, hTN_MAP32 tn_last_op);
  DefID_t Find_IV( DefID_t defid, DefID_t in_iv_cycle );

 public:

  LOOP_IVS(MEM_POOL *mem_pool)
    : _loc_mem_pool(mem_pool) ,
    ivs_table( NULL ),
    ivs_count( 0 ) {}

  ~LOOP_IVS(void) {}

  MEM_POOL *Mem_pool() { return _loc_mem_pool; }

  void Init( LOOP_DESCR *loop );
  void Replace_Op(INT index, OP *new_op);
  INT First_opidx() { return 1; }
  INT Last_opidx() { return ivs_count-1; }
  INT Size() { return ivs_count; }

  OP *Op(INT op_idx) {
    return ((op_idx > 0) && (op_idx < ivs_count)) ? ivs_table[op_idx].op : NULL;
  }
  DefID_t OPND_defid(INT op_idx, INT opnd_idx) { return ivs_table[op_idx].opnd_source[opnd_idx]; }
  DefID_t OPND_IV_cycle(INT op_idx, INT opnd_idx);
  INT OPND_IV_offset(INT op_idx, INT opndx_idx);
  INT IV_step(DefID_t iv_cycle);
  OP *IV_init(DefID_t iv_cycle);

  void Trace_IVs_Entries(const char *message);
};

#define FOR_ALL_LOOP_IVS_OPs_FWD(loop_ivs, opidx, op) \
  for (op = (loop_ivs)->Op(opidx=((loop_ivs)->First_opidx())); opidx <= (loop_ivs)->Last_opidx(); op = (loop_ivs)->Op(++opidx))

#define FOR_ALL_LOOP_IVS_OPs_REV(loop_ivs, opidx, op) \
  for (op = (loop_ivs)->Op(opidx=((loop_ivs)->Last_opidx())); opidx >= (loop_ivs)->First_opidx(); op = (loop_ivs)->Op(--opidx))

void Perform_Induction_Variables_Optimizations();

/* CG_LOOP load_store_packing:
   0x1 : Enable load packing
   0x2 : Enable store packing
   0x4 : Enable load packing also if alignment is unknown
   0x8 : Enable loop peeling
   0x10: Enable loop specialization
   0x20: Enable loop peeling also when loop trip count is a compile time literal value
   0x40: Force unroll factor to 2 if required for packing and the
         loop would not be unrolled otherwise.
   0x80: Enable packing only on memory bounded loops, but keep other
         loop transformations related to packing.
   0x100:Perform packing analysis and loop transformation, but do not
         actually pack
   0x200:Perform load packing in EBO
*/
enum LOAD_STORE_PACKING {
  PACKING_LOAD				= 0x1,
  PACKING_STORE				= 0x2,
  PACKING_LOAD_DYN_ALIGN 		= 0x4,
  PACKING_LOOP_PEELING			= 0x8,
  PACKING_LOOP_SPECIALIZATION		= 0x10,
  PACKING_LOOP_PEELING_CONST_TRIP_COUNT = 0x20,
  PACKING_FORCE_LOOP_UNROLL		= 0x40,
  PACKING_ONLY_MEMORY_BOUNDED		= 0x80,
  PACKING_NO_ACTUAL_PACKING		= 0x100,
  PACKING_EBO_LOAD			= 0x200
};
extern INT32 Loop_packing_flags;
extern INT32 CG_LOOP_stream_align;

extern BOOL IVS_Analyze_Load_Store_Packing( CG_LOOP &cg_loop);
extern BOOL IVS_Perform_Load_Store_Packing( CG_LOOP &cg_loop);
extern INT  Loop_Packing_Options(CG_LOOP &cg_loop);
#endif /* CG_IVS_INCLUDED */

#endif /* TARG_ST */
