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
//  Module: cg_dud.h
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
//  INT Lookup_Op(OP *op);
//    Returns an ID for an OP in the LOOP_IVS instance.
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

#ifndef CG_DUD_INCLUDED
#define CG_DUD_INCLUDED

#define IDX_WIDTH 3
#define IDX_MASK ((1<<IDX_WIDTH)-1)

typedef unsigned int DUDsite_t;
static inline DUDsite_t DUDsite_makeDef(UINT op_idx, UINT res_idx) {
  Is_True(res_idx <= IDX_MASK, ("DUDsite_makeDef: Internal Error"));
  return ((op_idx << IDX_WIDTH)|res_idx);
}
static inline DUDsite_t DUDsite_makeUse(UINT op_idx, UINT opnd_idx) {
  Is_True(opnd_idx <= IDX_MASK, ("DUDsite_make: Internal Error"));
  return ((op_idx << IDX_WIDTH)|opnd_idx);
}
static inline UINT DUDsite_opid(DUDsite_t dudlink) { return (dudlink >> IDX_WIDTH); }
static inline UINT DUDsite_opnd(DUDsite_t dudlink) { return (dudlink & IDX_MASK); }

typedef std::list<DUDsite_t> DUDsite_list;

class DUD_REGION {
  // For each operation, define a struct that olds def-use and use-def
  // links.
  public:

  typedef struct DUDinfo {
    OP *op;
    DUDsite_list def_use[OP_MAX_FIXED_RESULTS];
    DUDsite_list use_def[OP_MAX_FIXED_OPNDS];
  } DUDinfo_t;

  MEM_POOL *_loc_mem_pool;

  // Define a map from an OP * to a unique ID in the region.
  OP_MAP DUD_opid_map;

  // Define an array to store all the DUDinfo elements for all
  // operations in a region.
  INT DUDinfo_size;
  DUDinfo_t *DUDinfo_table;

  INT DUD_opidx(DUDinfo_t *dud_item) { return dud_item-DUDinfo_table; };

  DUD_REGION(MEM_POOL *mem_pool)
    : _loc_mem_pool(mem_pool) ,
      DUDinfo_table ( NULL),
      DUDinfo_size ( 0) {
    Is_True((OP_MAX_FIXED_RESULTS <= (1<<IDX_WIDTH)) &&
	    (OP_MAX_FIXED_OPNDS <= (1<<IDX_WIDTH)), ("Internal Error in DUD_REGION"));
  };

  BOOL Init( BB_REGION *bb_region, MEM_POOL *region_pool );
  void Trace_DUD();

  MEM_POOL *dud_pool() { return _loc_mem_pool; };

  INT DUD_size() { return DUDinfo_size; };
  void Set_DUD_size(INT size) { DUDinfo_size = size; };
  void Set_DUD_opid(OP *op, INT ID) { OP_MAP32_Set(DUD_opid_map, op, ID); };
  INT DUD_opid(OP *op) { return OP_MAP32_Get(DUD_opid_map, op); };
};

BOOL TN_is_DUDreg(TN *tn);

DUD_REGION *Build_DUD_info(BB_REGION *bb_region, MEM_POOL *region_pool);

#endif /* CG_DUD_INCLUDED */

#endif /* TARG_ST */
