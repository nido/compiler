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


/* ====================================================================
 * ====================================================================
 *
 * Module: cg_select.h
 * $Revision$
 * $Date$
 * $Author$
 * $Source$
 *
 * Description:
 *
 *   External interface to the select transformations.
 *
 * Utilities:
 *
 *   void HB_Form_Select(RID *rid, const BB_REGION& bb_region)
 *	Perform select optimizations
 *
 * ====================================================================
 * ====================================================================
 */

#ifndef SELECT_H_INCLUDED
#define SELECT_H_INCLUDED

extern void Convert_Select(RID *, const BB_REGION&);
extern void Select_Init(void);
extern void draw_CFG(void);

// Traces
// -Wb,-tt61:flags
#define Select_Trace_Candidates 1 
#define Select_Trace_Gen        2 
#define Select_Trace_Spec       4 
#define Select_Trace_Merge      8 
#define Select_Trace_Dup        16 
#define Select_Trace_Stats      32
#define Select_Trace_daVinci    64
#define Select_Trace_stdout  	0x80000000

// op list management
typedef slist<OP*> op_list;

static inline void
BB_Remove_Ops (BB* bb, op_list ops) 
{
  op_list::iterator op_iter = ops.begin();
  op_list::iterator op_end  = ops.end();
  for (; op_iter != op_end; ++op_iter) {
    BB_Remove_Op(bb, *op_iter);
  }
  ops.clear();
}

typedef struct {
  BB *fall_thru;
  BB *target;
} logif_info;

//
// targ_select.cxx
//
extern TN * Expand_CMP_Reg (OP *, TN *, OPS *);

#endif /* SELECT_H_INCLUDED */
