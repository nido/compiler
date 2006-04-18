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
//
//  This module buils the def-use and use-def links on a single-entry
//  BB_REGION
//
//  Exported Types:
//  ==============
//
//  typedef std::vector< std::pair<OP *, INT> > dud_link_t;
// 
//  Exported Functions:
//  ===================
//
//  DUD_REGION *DUD_REGION::DUD_REGION(MEM_POOL *mem_pool)
//    Constructor for the DUD_REGION object.
//
//  DUD_REGION::Init(BB_REGION *bb_region, MEM_POOL *region_pool);
//    Computes the Def-Use/Use-Def links between TNs in a BB_REGION.
//
//  DUD_REGION *Build_DUD_info(BB_REGION *bb_region, MEM_POOL
//  *region_pool); Creates a DUD_REGION object, build the
//  def-use/use-def links in the region, and return a DUD_REGION
//  object if successful.
//
//  DUD_REGION::Trace_DUD()
//    Trace the Def-Use/Use-Def links
//
//  INT DUD_REGION::Get_Use_Def(OP *op, INT opnd, dud_link_t &dud_link);
//    Returns in dud_link a vector of def-sites for a use
//    site. Returns the number of elements in dud-link
//
//  INT DUD_REGION::Get_Def_Use(OP *op, INT opnd, dud_link_t &dud_link);
//    Returns in dud_link a vector of use sites for a def
//    site. Returns the number of elements in dud-link
//
// =======================================================================
// ======================================================================= */

#ifdef TARG_ST
#include "bb.h"
#include "cg_loop.h"
#include "findloops.h"

#ifndef CG_DUD_INCLUDED
#define CG_DUD_INCLUDED

typedef std::vector< std::pair<OP *, INT> > dud_link_t;

class DUD_REGION {
  // For each operation, define a struct that olds def-use and use-def
  // links.
  private:

  /* ******************** *
   *      DUDsite         *
   * ******************** */

  static const int IDX_WIDTH = 3;
  static const int IDX_MASK = ((1<<IDX_WIDTH)-1);

  typedef unsigned int DUDsite_t;
  typedef std::list<DUDsite_t> DUDsite_list;

  DUDsite_t DUDsite_makeDef(UINT op_idx, UINT res_idx) {
    Is_True(res_idx <= IDX_MASK, ("DUDsite_makeDef: Internal Error"));
    return ((op_idx << IDX_WIDTH)|res_idx);
  }

  DUDsite_t DUDsite_makeUse(UINT op_idx, UINT opnd_idx) {
    Is_True(opnd_idx <= IDX_MASK, ("DUDsite_make: Internal Error"));
    return ((op_idx << IDX_WIDTH)|(OP_MAX_FIXED_RESULTS+opnd_idx));
  }

  UINT DUDsite_opid(DUDsite_t dudlink) { return (dudlink >> IDX_WIDTH); };
  UINT DUDsite_opnd(DUDsite_t dudlink, BOOL *result) { 
    INT opnd = dudlink & IDX_MASK;
    *result = TRUE;
    if (opnd >= OP_MAX_FIXED_RESULTS) {
      *result = FALSE;
      opnd -= OP_MAX_FIXED_RESULTS;
    }
    return opnd;
  }

  /* ******************** *
   *      DUDinfo         *
   * ******************** */

  typedef struct DUDinfo {
    OP *op;
    DUDsite_list dud_list[OP_MAX_FIXED_RESULTS+OP_MAX_FIXED_OPNDS];
  } DUDinfo_t;

  MEM_POOL *_loc_mem_pool;

  // Define a map from an OP * to a unique ID in the region.
  OP_MAP DUD_opid_map;

  // Define an array to store all the DUDinfo elements for all
  // operations in a region.
  INT DUDinfo_size;
  DUDinfo_t *DUDinfo_table;

  MEM_POOL *dud_pool() { return _loc_mem_pool; };

  void Set_DUD_size(INT size) { DUDinfo_size = size; };
  INT Get_DUD_size() { return DUDinfo_size; };

  BOOL DUD_check_size(INT op_count) {
    return (op_count < (UINT_MAX >> IDX_WIDTH));
  }

  void DUD_Allocate(INT op_count, MEM_POOL *pool) {
    Set_DUD_size(op_count);
    DUDinfo_table = (DUDinfo_t *) CXX_NEW_ARRAY( DUDinfo_t, Get_DUD_size(), pool );
  }

  void Set_DUD_op(INT opid, OP *op) { DUDinfo_table[opid].op = op; };
  OP *Get_DUD_op(INT opid) { return DUDinfo_table[opid].op; };

  void Set_DUD_opid(OP *op, INT opid) { OP_MAP32_Set(DUD_opid_map, op, opid); };
  INT Get_DUD_opid(OP *op) { return OP_MAP32_Get(DUD_opid_map, op); };

  void TNuse_Push_DUDsite(INT opid, INT opnd, DUDsite_t site) {
    DUDinfo_table[opid].dud_list[opnd+OP_MAX_FIXED_RESULTS].push_back(site);
  }

  void TNdef_Push_DUDsite(INT opid, INT res, DUDsite_t site) {
    DUDinfo_table[opid].dud_list[res].push_back(site);
  }

  public:

  DUD_REGION(MEM_POOL *mem_pool)
    : _loc_mem_pool(mem_pool) ,
      DUDinfo_table ( NULL),
      DUDinfo_size ( 0) {
    Is_True(((OP_MAX_FIXED_RESULTS+OP_MAX_FIXED_OPNDS) <= (1<<IDX_WIDTH)), ("Internal Error in DUD_REGION"));
    DUD_opid_map = OP_MAP32_Create();
  };

  ~DUD_REGION() {
    OP_MAP_Delete(DUD_opid_map);
  }

  BOOL Init( BB_REGION *bb_region, MEM_POOL *region_pool );

  INT DUD_REGION::Get_Use_Def(OP *op, INT opnd, dud_link_t &dud_link);
  INT DUD_REGION::Get_Def_Use(OP *op, INT res, dud_link_t &dud_link);

  void Trace_DUD();  
};

DUD_REGION *Build_DUD_info(BB_REGION *bb_region, MEM_POOL *region_pool);

#endif /* CG_DUD_INCLUDED */

#endif /* TARG_ST */
