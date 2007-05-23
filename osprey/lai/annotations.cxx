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



/* =======================================================================
 * =======================================================================
 *
 *  Module: annotations.c
 *
 *  Description:
 *  ============
 *
 *  Module to create and access annotations that can be attached to 
 *  any arbitrary data structure.
 *
 *  See annotations.h for description.
 *
 * =======================================================================
 * =======================================================================
 */

#include "defs.h"
#include "errors.h"
#include "mempool.h"
#include "cgir.h"
#include "annotations.h"


extern ANNOTATION *
ANNOT_Add (
  ANNOTATION *annot_list, 
  ANNOTATION_KIND kind, 
  void *information,
  MEM_POOL *pool)
{
  ANNOTATION *new_a = TYPE_MEM_POOL_ALLOC (ANNOTATION, pool);
  ANNOTATION *list, *next;

  ANNOT_next(new_a) = NULL;
  ANNOT_info(new_a) = information;
  ANNOT_kind(new_a) = kind;

  /* The new annotation is added at the end of the annotations list.
   * This maintains the order in which annotations are added. Some 
   * clients care about this.
   */
  for (list = annot_list; list != NULL; list = next) {
    next = ANNOT_next(list);
    if (next == NULL) {
      ANNOT_next(list) = new_a;
      break;
    }
  }
  if (annot_list == NULL) annot_list = new_a;
  return annot_list;

}

extern ANNOTATION *
ANNOT_Unlink (
  ANNOTATION *annot_list,
  ANNOTATION *this1)
{
  ANNOTATION *list, *next;

  if ( annot_list == this1 ) {
    return ANNOT_next(annot_list);
  }

  for (list = annot_list; list != NULL; list = next) {
    next = ANNOT_next(list);
    if ( next == this1 ) {
      ANNOT_next(list) = ANNOT_next(next);
      break;
    }
  }

  return annot_list;
}

extern ANNOTATION *
ANNOT_Get (ANNOTATION *list, ANNOTATION_KIND kind)
{
  ANNOTATION_KIND cur_kind;

  while (list != NULL) {
    cur_kind = ANNOT_kind(list);
    if (cur_kind == kind) {
      break;
    }
    list = ANNOT_next(list);
  }
  return list;
}

#ifdef TARG_ST
// [TTh] Constructor for ASM_OP_ANNOT, that allocate ASM_OP_ANNOT
//       object, as well as its set of internal arrays.
//       All fields are initialized to 0.
extern ASM_OP_ANNOT *
Create_Empty_ASM_OP_ANNOT(INT num_results, INT num_opnds)
{
  const char **p_const_char_p;;
  ISA_REGISTER_SUBCLASS *p_SUBCLASS;
  mUINT32 *p_mUINT32;
  bool *p_bool;

  ASM_OP_ANNOT* asm_info = TYPE_PU_ALLOC(ASM_OP_ANNOT);
  BZERO(asm_info, sizeof(ASM_OP_ANNOT));

  /* Allocate tables 'shared' by results and operands */
  if (num_results + num_opnds > 0) {
    p_const_char_p = TYPE_PU_ALLOC_N(const char *, num_results + num_opnds);
    BZERO(p_const_char_p, sizeof(const char *) *  (num_results + num_opnds));

    p_SUBCLASS = TYPE_PU_ALLOC_N(ISA_REGISTER_SUBCLASS,  num_results + num_opnds);
    BZERO(p_SUBCLASS, sizeof(ISA_REGISTER_SUBCLASS) * (num_results + num_opnds));

    p_mUINT32 = TYPE_PU_ALLOC_N(mUINT32, num_results + num_opnds);
    BZERO(p_mUINT32, sizeof(mUINT32) *  (num_results + num_opnds));

    p_bool =  TYPE_PU_ALLOC_N(bool, (2 * num_results) + num_opnds);
    BZERO(p_bool, sizeof(bool) *   ((2 * num_results) + num_opnds));
  }

  if (num_results > 0) {
    asm_info->result_constraint = p_const_char_p;  p_const_char_p += num_results;
    asm_info->result_subclass   = p_SUBCLASS;      p_SUBCLASS     += num_results;
    asm_info->result_position   = p_mUINT32;       p_mUINT32      += num_results;
    asm_info->result_clobber    = p_bool;          p_bool         += num_results;
    asm_info->result_memory     = p_bool;          p_bool         += num_results;

    asm_info->result_same_opnd  = TYPE_PU_ALLOC_N(mINT8, num_results);
    BZERO(asm_info->result_same_opnd, sizeof(mINT8) * num_results);
  }

  if (num_opnds > 0) {
    asm_info->opnd_constraint = p_const_char_p;
    asm_info->opnd_subclass   = p_SUBCLASS;
    asm_info->opnd_position   = p_mUINT32;
    asm_info->opnd_memory     = p_bool;
  }

  return asm_info;
}
#endif
