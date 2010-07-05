/* Type information for ada.
   Copyright (C) 2002 Free Software Foundation, Inc.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2, or (at your option) any later
version.

GCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA
02111-1307, USA.  */

/* This file is machine generated.  Do not edit.  */

void
gt_ggc_mx_lang_tree_node (x_p)
      void *x_p;
{
  union lang_tree_node * const x = (union lang_tree_node *)x_p;
  if (ggc_test_and_set_mark (x))
    {
      switch (TREE_CODE (&((*x)).generic) == GNAT_LOOP_ID)
        {
        case 0:
          switch (tree_node_structure (&((*x).generic)))
            {
            case TS_COMMON:
              gt_ggc_m_9tree_node ((*x).generic.common.chain);
              gt_ggc_m_9tree_node ((*x).generic.common.type);
              break;
            case TS_INT_CST:
              gt_ggc_m_9tree_node ((*x).generic.int_cst.common.chain);
              gt_ggc_m_9tree_node ((*x).generic.int_cst.common.type);
              gt_ggc_m_7rtx_def ((*x).generic.int_cst.rtl);
              break;
            case TS_REAL_CST:
              gt_ggc_m_9tree_node ((*x).generic.real_cst.common.chain);
              gt_ggc_m_9tree_node ((*x).generic.real_cst.common.type);
              gt_ggc_m_7rtx_def ((*x).generic.real_cst.rtl);
              gt_ggc_m_10real_value ((*x).generic.real_cst.real_cst_ptr);
              break;
            case TS_VECTOR:
              gt_ggc_m_9tree_node ((*x).generic.vector.common.chain);
              gt_ggc_m_9tree_node ((*x).generic.vector.common.type);
              gt_ggc_m_7rtx_def ((*x).generic.vector.rtl);
              gt_ggc_m_9tree_node ((*x).generic.vector.elements);
              break;
            case TS_STRING:
              gt_ggc_m_9tree_node ((*x).generic.string.common.chain);
              gt_ggc_m_9tree_node ((*x).generic.string.common.type);
              gt_ggc_m_7rtx_def ((*x).generic.string.rtl);
              break;
            case TS_COMPLEX:
              gt_ggc_m_9tree_node ((*x).generic.complex.common.chain);
              gt_ggc_m_9tree_node ((*x).generic.complex.common.type);
              gt_ggc_m_7rtx_def ((*x).generic.complex.rtl);
              gt_ggc_m_9tree_node ((*x).generic.complex.real);
              gt_ggc_m_9tree_node ((*x).generic.complex.imag);
              break;
            case TS_IDENTIFIER:
              gt_ggc_m_9tree_node ((*x).generic.identifier.common.chain);
              gt_ggc_m_9tree_node ((*x).generic.identifier.common.type);
              break;
            case TS_DECL:
              gt_ggc_m_9tree_node ((*x).generic.decl.common.chain);
              gt_ggc_m_9tree_node ((*x).generic.decl.common.type);
              gt_ggc_m_9tree_node ((*x).generic.decl.size);
              gt_ggc_m_9tree_node ((*x).generic.decl.size_unit);
              gt_ggc_m_9tree_node ((*x).generic.decl.name);
              gt_ggc_m_9tree_node ((*x).generic.decl.context);
              gt_ggc_m_9tree_node ((*x).generic.decl.arguments);
              gt_ggc_m_9tree_node ((*x).generic.decl.result);
              gt_ggc_m_9tree_node ((*x).generic.decl.initial);
              gt_ggc_m_9tree_node ((*x).generic.decl.abstract_origin);
              gt_ggc_m_9tree_node ((*x).generic.decl.assembler_name);
              gt_ggc_m_9tree_node ((*x).generic.decl.section_name);
              gt_ggc_m_9tree_node ((*x).generic.decl.attributes);
              gt_ggc_m_7rtx_def ((*x).generic.decl.rtl);
              gt_ggc_m_7rtx_def ((*x).generic.decl.live_range_rtl);
              switch (TREE_CODE((tree) &((*x))))
                {
                case FUNCTION_DECL:
                  gt_ggc_m_8function ((*x).generic.decl.u2.f);
                  break;
                case PARM_DECL:
                  gt_ggc_m_7rtx_def ((*x).generic.decl.u2.r);
                  break;
                case FIELD_DECL:
                  gt_ggc_m_9tree_node ((*x).generic.decl.u2.t);
                  break;
                default:
                  break;
                }
              gt_ggc_m_9tree_node ((*x).generic.decl.saved_tree);
              gt_ggc_m_9tree_node ((*x).generic.decl.inlined_fns);
              gt_ggc_m_9tree_node ((*x).generic.decl.vindex);
              gt_ggc_m_9lang_decl ((*x).generic.decl.lang_specific);
              break;
            case TS_TYPE:
              gt_ggc_m_9tree_node ((*x).generic.type.common.chain);
              gt_ggc_m_9tree_node ((*x).generic.type.common.type);
              gt_ggc_m_9tree_node ((*x).generic.type.values);
              gt_ggc_m_9tree_node ((*x).generic.type.size);
              gt_ggc_m_9tree_node ((*x).generic.type.size_unit);
              gt_ggc_m_9tree_node ((*x).generic.type.attributes);
              gt_ggc_m_9tree_node ((*x).generic.type.pointer_to);
              gt_ggc_m_9tree_node ((*x).generic.type.reference_to);
              switch (debug_hooks == &sdb_debug_hooks ? 1 : debug_hooks == &dwarf2_debug_hooks ? 2 : 0)
                {
                case 1:
                  break;
                default:
                  break;
                }
              gt_ggc_m_9tree_node ((*x).generic.type.name);
              gt_ggc_m_9tree_node ((*x).generic.type.minval);
              gt_ggc_m_9tree_node ((*x).generic.type.maxval);
              gt_ggc_m_9tree_node ((*x).generic.type.next_variant);
              gt_ggc_m_9tree_node ((*x).generic.type.main_variant);
              gt_ggc_m_9tree_node ((*x).generic.type.binfo);
              gt_ggc_m_9tree_node ((*x).generic.type.context);
              gt_ggc_m_9lang_type ((*x).generic.type.lang_specific);
              break;
            case TS_LIST:
              gt_ggc_m_9tree_node ((*x).generic.list.common.chain);
              gt_ggc_m_9tree_node ((*x).generic.list.common.type);
              gt_ggc_m_9tree_node ((*x).generic.list.purpose);
              gt_ggc_m_9tree_node ((*x).generic.list.value);
              break;
            case TS_VEC:
              gt_ggc_m_9tree_node ((*x).generic.vec.common.chain);
              gt_ggc_m_9tree_node ((*x).generic.vec.common.type);
              {
                size_t i1_0;
                const size_t ilimit1_0 = (TREE_VEC_LENGTH ((tree)&((*x).generic.vec)));
                for (i1_0 = 0; i1_0 < ilimit1_0; i1_0++) {
                  gt_ggc_m_9tree_node ((*x).generic.vec.a[i1_0]);
                }
              }
              break;
            case TS_EXP:
              gt_ggc_m_9tree_node ((*x).generic.exp.common.chain);
              gt_ggc_m_9tree_node ((*x).generic.exp.common.type);
              switch (TREE_CODE ((tree) &(*x)))
                {
                case METHOD_CALL_EXPR:
                  gt_ggc_m_7rtx_def ((*x).generic.exp.operands[3]);
                  gt_ggc_m_9tree_node ((*x).generic.exp.operands[2]);
                  gt_ggc_m_9tree_node ((*x).generic.exp.operands[1]);
                  gt_ggc_m_9tree_node ((*x).generic.exp.operands[0]);
                  break;
                case WITH_CLEANUP_EXPR:
                  gt_ggc_m_7rtx_def ((*x).generic.exp.operands[2]);
                  gt_ggc_m_9tree_node ((*x).generic.exp.operands[1]);
                  gt_ggc_m_9tree_node ((*x).generic.exp.operands[0]);
                  break;
                case RTL_EXPR:
                  gt_ggc_m_7rtx_def ((*x).generic.exp.operands[1]);
                  gt_ggc_m_7rtx_def ((*x).generic.exp.operands[0]);
                  break;
                case GOTO_SUBROUTINE_EXPR:
                  gt_ggc_m_7rtx_def ((*x).generic.exp.operands[1]);
                  gt_ggc_m_7rtx_def ((*x).generic.exp.operands[0]);
                  break;
                case SAVE_EXPR:
                  gt_ggc_m_7rtx_def ((*x).generic.exp.operands[2]);
                  gt_ggc_m_9tree_node ((*x).generic.exp.operands[1]);
                  gt_ggc_m_9tree_node ((*x).generic.exp.operands[0]);
                  break;
                default:
                  {
                    size_t i2_0;
                    const size_t ilimit2_0 = (TREE_CODE_LENGTH (TREE_CODE ((tree) &(*x))));
                    for (i2_0 = 0; i2_0 < ilimit2_0; i2_0++) {
                      gt_ggc_m_9tree_node ((*x).generic.exp.operands[i2_0]);
                    }
                  }
                  break;
                }
              break;
            case TS_BLOCK:
              gt_ggc_m_9tree_node ((*x).generic.block.common.chain);
              gt_ggc_m_9tree_node ((*x).generic.block.common.type);
              gt_ggc_m_9tree_node ((*x).generic.block.vars);
              gt_ggc_m_9tree_node ((*x).generic.block.subblocks);
              gt_ggc_m_9tree_node ((*x).generic.block.supercontext);
              gt_ggc_m_9tree_node ((*x).generic.block.abstract_origin);
              gt_ggc_m_9tree_node ((*x).generic.block.fragment_origin);
              gt_ggc_m_9tree_node ((*x).generic.block.fragment_chain);
              break;
            default:
              break;
            }
          break;
        case 1:
          gt_ggc_m_9tree_node ((*x).loop_id.common.chain);
          gt_ggc_m_9tree_node ((*x).loop_id.common.type);
          gt_ggc_m_7nesting ((*x).loop_id.loop_id);
          break;
        default:
          break;
        }
  }
}

void
gt_ggc_mx_lang_decl (x_p)
      void *x_p;
{
  struct lang_decl * const x = (struct lang_decl *)x_p;
  if (ggc_test_and_set_mark (x))
    {
      switch (TREE_CODE (&((*x).t).generic) == GNAT_LOOP_ID)
        {
        case 0:
          switch (tree_node_structure (&((*x).t.generic)))
            {
            case TS_COMMON:
              gt_ggc_m_9tree_node ((*x).t.generic.common.chain);
              gt_ggc_m_9tree_node ((*x).t.generic.common.type);
              break;
            case TS_INT_CST:
              gt_ggc_m_9tree_node ((*x).t.generic.int_cst.common.chain);
              gt_ggc_m_9tree_node ((*x).t.generic.int_cst.common.type);
              gt_ggc_m_7rtx_def ((*x).t.generic.int_cst.rtl);
              break;
            case TS_REAL_CST:
              gt_ggc_m_9tree_node ((*x).t.generic.real_cst.common.chain);
              gt_ggc_m_9tree_node ((*x).t.generic.real_cst.common.type);
              gt_ggc_m_7rtx_def ((*x).t.generic.real_cst.rtl);
              gt_ggc_m_10real_value ((*x).t.generic.real_cst.real_cst_ptr);
              break;
            case TS_VECTOR:
              gt_ggc_m_9tree_node ((*x).t.generic.vector.common.chain);
              gt_ggc_m_9tree_node ((*x).t.generic.vector.common.type);
              gt_ggc_m_7rtx_def ((*x).t.generic.vector.rtl);
              gt_ggc_m_9tree_node ((*x).t.generic.vector.elements);
              break;
            case TS_STRING:
              gt_ggc_m_9tree_node ((*x).t.generic.string.common.chain);
              gt_ggc_m_9tree_node ((*x).t.generic.string.common.type);
              gt_ggc_m_7rtx_def ((*x).t.generic.string.rtl);
              break;
            case TS_COMPLEX:
              gt_ggc_m_9tree_node ((*x).t.generic.complex.common.chain);
              gt_ggc_m_9tree_node ((*x).t.generic.complex.common.type);
              gt_ggc_m_7rtx_def ((*x).t.generic.complex.rtl);
              gt_ggc_m_9tree_node ((*x).t.generic.complex.real);
              gt_ggc_m_9tree_node ((*x).t.generic.complex.imag);
              break;
            case TS_IDENTIFIER:
              gt_ggc_m_9tree_node ((*x).t.generic.identifier.common.chain);
              gt_ggc_m_9tree_node ((*x).t.generic.identifier.common.type);
              break;
            case TS_DECL:
              gt_ggc_m_9tree_node ((*x).t.generic.decl.common.chain);
              gt_ggc_m_9tree_node ((*x).t.generic.decl.common.type);
              gt_ggc_m_9tree_node ((*x).t.generic.decl.size);
              gt_ggc_m_9tree_node ((*x).t.generic.decl.size_unit);
              gt_ggc_m_9tree_node ((*x).t.generic.decl.name);
              gt_ggc_m_9tree_node ((*x).t.generic.decl.context);
              gt_ggc_m_9tree_node ((*x).t.generic.decl.arguments);
              gt_ggc_m_9tree_node ((*x).t.generic.decl.result);
              gt_ggc_m_9tree_node ((*x).t.generic.decl.initial);
              gt_ggc_m_9tree_node ((*x).t.generic.decl.abstract_origin);
              gt_ggc_m_9tree_node ((*x).t.generic.decl.assembler_name);
              gt_ggc_m_9tree_node ((*x).t.generic.decl.section_name);
              gt_ggc_m_9tree_node ((*x).t.generic.decl.attributes);
              gt_ggc_m_7rtx_def ((*x).t.generic.decl.rtl);
              gt_ggc_m_7rtx_def ((*x).t.generic.decl.live_range_rtl);
              switch (TREE_CODE((tree) &((*x))))
                {
                case FUNCTION_DECL:
                  gt_ggc_m_8function ((*x).t.generic.decl.u2.f);
                  break;
                case PARM_DECL:
                  gt_ggc_m_7rtx_def ((*x).t.generic.decl.u2.r);
                  break;
                case FIELD_DECL:
                  gt_ggc_m_9tree_node ((*x).t.generic.decl.u2.t);
                  break;
                default:
                  break;
                }
              gt_ggc_m_9tree_node ((*x).t.generic.decl.saved_tree);
              gt_ggc_m_9tree_node ((*x).t.generic.decl.inlined_fns);
              gt_ggc_m_9tree_node ((*x).t.generic.decl.vindex);
              gt_ggc_m_9lang_decl ((*x).t.generic.decl.lang_specific);
              break;
            case TS_TYPE:
              gt_ggc_m_9tree_node ((*x).t.generic.type.common.chain);
              gt_ggc_m_9tree_node ((*x).t.generic.type.common.type);
              gt_ggc_m_9tree_node ((*x).t.generic.type.values);
              gt_ggc_m_9tree_node ((*x).t.generic.type.size);
              gt_ggc_m_9tree_node ((*x).t.generic.type.size_unit);
              gt_ggc_m_9tree_node ((*x).t.generic.type.attributes);
              gt_ggc_m_9tree_node ((*x).t.generic.type.pointer_to);
              gt_ggc_m_9tree_node ((*x).t.generic.type.reference_to);
              switch (debug_hooks == &sdb_debug_hooks ? 1 : debug_hooks == &dwarf2_debug_hooks ? 2 : 0)
                {
                case 1:
                  break;
                default:
                  break;
                }
              gt_ggc_m_9tree_node ((*x).t.generic.type.name);
              gt_ggc_m_9tree_node ((*x).t.generic.type.minval);
              gt_ggc_m_9tree_node ((*x).t.generic.type.maxval);
              gt_ggc_m_9tree_node ((*x).t.generic.type.next_variant);
              gt_ggc_m_9tree_node ((*x).t.generic.type.main_variant);
              gt_ggc_m_9tree_node ((*x).t.generic.type.binfo);
              gt_ggc_m_9tree_node ((*x).t.generic.type.context);
              gt_ggc_m_9lang_type ((*x).t.generic.type.lang_specific);
              break;
            case TS_LIST:
              gt_ggc_m_9tree_node ((*x).t.generic.list.common.chain);
              gt_ggc_m_9tree_node ((*x).t.generic.list.common.type);
              gt_ggc_m_9tree_node ((*x).t.generic.list.purpose);
              gt_ggc_m_9tree_node ((*x).t.generic.list.value);
              break;
            case TS_VEC:
              gt_ggc_m_9tree_node ((*x).t.generic.vec.common.chain);
              gt_ggc_m_9tree_node ((*x).t.generic.vec.common.type);
              {
                size_t i1_0;
                const size_t ilimit1_0 = (TREE_VEC_LENGTH ((tree)&((*x).t.generic.vec)));
                for (i1_0 = 0; i1_0 < ilimit1_0; i1_0++) {
                  gt_ggc_m_9tree_node ((*x).t.generic.vec.a[i1_0]);
                }
              }
              break;
            case TS_EXP:
              gt_ggc_m_9tree_node ((*x).t.generic.exp.common.chain);
              gt_ggc_m_9tree_node ((*x).t.generic.exp.common.type);
              switch (TREE_CODE ((tree) &(*x)))
                {
                case METHOD_CALL_EXPR:
                  gt_ggc_m_7rtx_def ((*x).t.generic.exp.operands[3]);
                  gt_ggc_m_9tree_node ((*x).t.generic.exp.operands[2]);
                  gt_ggc_m_9tree_node ((*x).t.generic.exp.operands[1]);
                  gt_ggc_m_9tree_node ((*x).t.generic.exp.operands[0]);
                  break;
                case WITH_CLEANUP_EXPR:
                  gt_ggc_m_7rtx_def ((*x).t.generic.exp.operands[2]);
                  gt_ggc_m_9tree_node ((*x).t.generic.exp.operands[1]);
                  gt_ggc_m_9tree_node ((*x).t.generic.exp.operands[0]);
                  break;
                case RTL_EXPR:
                  gt_ggc_m_7rtx_def ((*x).t.generic.exp.operands[1]);
                  gt_ggc_m_7rtx_def ((*x).t.generic.exp.operands[0]);
                  break;
                case GOTO_SUBROUTINE_EXPR:
                  gt_ggc_m_7rtx_def ((*x).t.generic.exp.operands[1]);
                  gt_ggc_m_7rtx_def ((*x).t.generic.exp.operands[0]);
                  break;
                case SAVE_EXPR:
                  gt_ggc_m_7rtx_def ((*x).t.generic.exp.operands[2]);
                  gt_ggc_m_9tree_node ((*x).t.generic.exp.operands[1]);
                  gt_ggc_m_9tree_node ((*x).t.generic.exp.operands[0]);
                  break;
                default:
                  {
                    size_t i2_0;
                    const size_t ilimit2_0 = (TREE_CODE_LENGTH (TREE_CODE ((tree) &(*x))));
                    for (i2_0 = 0; i2_0 < ilimit2_0; i2_0++) {
                      gt_ggc_m_9tree_node ((*x).t.generic.exp.operands[i2_0]);
                    }
                  }
                  break;
                }
              break;
            case TS_BLOCK:
              gt_ggc_m_9tree_node ((*x).t.generic.block.common.chain);
              gt_ggc_m_9tree_node ((*x).t.generic.block.common.type);
              gt_ggc_m_9tree_node ((*x).t.generic.block.vars);
              gt_ggc_m_9tree_node ((*x).t.generic.block.subblocks);
              gt_ggc_m_9tree_node ((*x).t.generic.block.supercontext);
              gt_ggc_m_9tree_node ((*x).t.generic.block.abstract_origin);
              gt_ggc_m_9tree_node ((*x).t.generic.block.fragment_origin);
              gt_ggc_m_9tree_node ((*x).t.generic.block.fragment_chain);
              break;
            default:
              break;
            }
          break;
        case 1:
          gt_ggc_m_9tree_node ((*x).t.loop_id.common.chain);
          gt_ggc_m_9tree_node ((*x).t.loop_id.common.type);
          gt_ggc_m_7nesting ((*x).t.loop_id.loop_id);
          break;
        default:
          break;
        }
  }
}

void
gt_ggc_mx_lang_type (x_p)
      void *x_p;
{
  struct lang_type * const x = (struct lang_type *)x_p;
  if (ggc_test_and_set_mark (x))
    {
      switch (TREE_CODE (&((*x).t).generic) == GNAT_LOOP_ID)
        {
        case 0:
          switch (tree_node_structure (&((*x).t.generic)))
            {
            case TS_COMMON:
              gt_ggc_m_9tree_node ((*x).t.generic.common.chain);
              gt_ggc_m_9tree_node ((*x).t.generic.common.type);
              break;
            case TS_INT_CST:
              gt_ggc_m_9tree_node ((*x).t.generic.int_cst.common.chain);
              gt_ggc_m_9tree_node ((*x).t.generic.int_cst.common.type);
              gt_ggc_m_7rtx_def ((*x).t.generic.int_cst.rtl);
              break;
            case TS_REAL_CST:
              gt_ggc_m_9tree_node ((*x).t.generic.real_cst.common.chain);
              gt_ggc_m_9tree_node ((*x).t.generic.real_cst.common.type);
              gt_ggc_m_7rtx_def ((*x).t.generic.real_cst.rtl);
              gt_ggc_m_10real_value ((*x).t.generic.real_cst.real_cst_ptr);
              break;
            case TS_VECTOR:
              gt_ggc_m_9tree_node ((*x).t.generic.vector.common.chain);
              gt_ggc_m_9tree_node ((*x).t.generic.vector.common.type);
              gt_ggc_m_7rtx_def ((*x).t.generic.vector.rtl);
              gt_ggc_m_9tree_node ((*x).t.generic.vector.elements);
              break;
            case TS_STRING:
              gt_ggc_m_9tree_node ((*x).t.generic.string.common.chain);
              gt_ggc_m_9tree_node ((*x).t.generic.string.common.type);
              gt_ggc_m_7rtx_def ((*x).t.generic.string.rtl);
              break;
            case TS_COMPLEX:
              gt_ggc_m_9tree_node ((*x).t.generic.complex.common.chain);
              gt_ggc_m_9tree_node ((*x).t.generic.complex.common.type);
              gt_ggc_m_7rtx_def ((*x).t.generic.complex.rtl);
              gt_ggc_m_9tree_node ((*x).t.generic.complex.real);
              gt_ggc_m_9tree_node ((*x).t.generic.complex.imag);
              break;
            case TS_IDENTIFIER:
              gt_ggc_m_9tree_node ((*x).t.generic.identifier.common.chain);
              gt_ggc_m_9tree_node ((*x).t.generic.identifier.common.type);
              break;
            case TS_DECL:
              gt_ggc_m_9tree_node ((*x).t.generic.decl.common.chain);
              gt_ggc_m_9tree_node ((*x).t.generic.decl.common.type);
              gt_ggc_m_9tree_node ((*x).t.generic.decl.size);
              gt_ggc_m_9tree_node ((*x).t.generic.decl.size_unit);
              gt_ggc_m_9tree_node ((*x).t.generic.decl.name);
              gt_ggc_m_9tree_node ((*x).t.generic.decl.context);
              gt_ggc_m_9tree_node ((*x).t.generic.decl.arguments);
              gt_ggc_m_9tree_node ((*x).t.generic.decl.result);
              gt_ggc_m_9tree_node ((*x).t.generic.decl.initial);
              gt_ggc_m_9tree_node ((*x).t.generic.decl.abstract_origin);
              gt_ggc_m_9tree_node ((*x).t.generic.decl.assembler_name);
              gt_ggc_m_9tree_node ((*x).t.generic.decl.section_name);
              gt_ggc_m_9tree_node ((*x).t.generic.decl.attributes);
              gt_ggc_m_7rtx_def ((*x).t.generic.decl.rtl);
              gt_ggc_m_7rtx_def ((*x).t.generic.decl.live_range_rtl);
              switch (TREE_CODE((tree) &((*x))))
                {
                case FUNCTION_DECL:
                  gt_ggc_m_8function ((*x).t.generic.decl.u2.f);
                  break;
                case PARM_DECL:
                  gt_ggc_m_7rtx_def ((*x).t.generic.decl.u2.r);
                  break;
                case FIELD_DECL:
                  gt_ggc_m_9tree_node ((*x).t.generic.decl.u2.t);
                  break;
                default:
                  break;
                }
              gt_ggc_m_9tree_node ((*x).t.generic.decl.saved_tree);
              gt_ggc_m_9tree_node ((*x).t.generic.decl.inlined_fns);
              gt_ggc_m_9tree_node ((*x).t.generic.decl.vindex);
              gt_ggc_m_9lang_decl ((*x).t.generic.decl.lang_specific);
              break;
            case TS_TYPE:
              gt_ggc_m_9tree_node ((*x).t.generic.type.common.chain);
              gt_ggc_m_9tree_node ((*x).t.generic.type.common.type);
              gt_ggc_m_9tree_node ((*x).t.generic.type.values);
              gt_ggc_m_9tree_node ((*x).t.generic.type.size);
              gt_ggc_m_9tree_node ((*x).t.generic.type.size_unit);
              gt_ggc_m_9tree_node ((*x).t.generic.type.attributes);
              gt_ggc_m_9tree_node ((*x).t.generic.type.pointer_to);
              gt_ggc_m_9tree_node ((*x).t.generic.type.reference_to);
              switch (debug_hooks == &sdb_debug_hooks ? 1 : debug_hooks == &dwarf2_debug_hooks ? 2 : 0)
                {
                case 1:
                  break;
                default:
                  break;
                }
              gt_ggc_m_9tree_node ((*x).t.generic.type.name);
              gt_ggc_m_9tree_node ((*x).t.generic.type.minval);
              gt_ggc_m_9tree_node ((*x).t.generic.type.maxval);
              gt_ggc_m_9tree_node ((*x).t.generic.type.next_variant);
              gt_ggc_m_9tree_node ((*x).t.generic.type.main_variant);
              gt_ggc_m_9tree_node ((*x).t.generic.type.binfo);
              gt_ggc_m_9tree_node ((*x).t.generic.type.context);
              gt_ggc_m_9lang_type ((*x).t.generic.type.lang_specific);
              break;
            case TS_LIST:
              gt_ggc_m_9tree_node ((*x).t.generic.list.common.chain);
              gt_ggc_m_9tree_node ((*x).t.generic.list.common.type);
              gt_ggc_m_9tree_node ((*x).t.generic.list.purpose);
              gt_ggc_m_9tree_node ((*x).t.generic.list.value);
              break;
            case TS_VEC:
              gt_ggc_m_9tree_node ((*x).t.generic.vec.common.chain);
              gt_ggc_m_9tree_node ((*x).t.generic.vec.common.type);
              {
                size_t i1_0;
                const size_t ilimit1_0 = (TREE_VEC_LENGTH ((tree)&((*x).t.generic.vec)));
                for (i1_0 = 0; i1_0 < ilimit1_0; i1_0++) {
                  gt_ggc_m_9tree_node ((*x).t.generic.vec.a[i1_0]);
                }
              }
              break;
            case TS_EXP:
              gt_ggc_m_9tree_node ((*x).t.generic.exp.common.chain);
              gt_ggc_m_9tree_node ((*x).t.generic.exp.common.type);
              switch (TREE_CODE ((tree) &(*x)))
                {
                case METHOD_CALL_EXPR:
                  gt_ggc_m_7rtx_def ((*x).t.generic.exp.operands[3]);
                  gt_ggc_m_9tree_node ((*x).t.generic.exp.operands[2]);
                  gt_ggc_m_9tree_node ((*x).t.generic.exp.operands[1]);
                  gt_ggc_m_9tree_node ((*x).t.generic.exp.operands[0]);
                  break;
                case WITH_CLEANUP_EXPR:
                  gt_ggc_m_7rtx_def ((*x).t.generic.exp.operands[2]);
                  gt_ggc_m_9tree_node ((*x).t.generic.exp.operands[1]);
                  gt_ggc_m_9tree_node ((*x).t.generic.exp.operands[0]);
                  break;
                case RTL_EXPR:
                  gt_ggc_m_7rtx_def ((*x).t.generic.exp.operands[1]);
                  gt_ggc_m_7rtx_def ((*x).t.generic.exp.operands[0]);
                  break;
                case GOTO_SUBROUTINE_EXPR:
                  gt_ggc_m_7rtx_def ((*x).t.generic.exp.operands[1]);
                  gt_ggc_m_7rtx_def ((*x).t.generic.exp.operands[0]);
                  break;
                case SAVE_EXPR:
                  gt_ggc_m_7rtx_def ((*x).t.generic.exp.operands[2]);
                  gt_ggc_m_9tree_node ((*x).t.generic.exp.operands[1]);
                  gt_ggc_m_9tree_node ((*x).t.generic.exp.operands[0]);
                  break;
                default:
                  {
                    size_t i2_0;
                    const size_t ilimit2_0 = (TREE_CODE_LENGTH (TREE_CODE ((tree) &(*x))));
                    for (i2_0 = 0; i2_0 < ilimit2_0; i2_0++) {
                      gt_ggc_m_9tree_node ((*x).t.generic.exp.operands[i2_0]);
                    }
                  }
                  break;
                }
              break;
            case TS_BLOCK:
              gt_ggc_m_9tree_node ((*x).t.generic.block.common.chain);
              gt_ggc_m_9tree_node ((*x).t.generic.block.common.type);
              gt_ggc_m_9tree_node ((*x).t.generic.block.vars);
              gt_ggc_m_9tree_node ((*x).t.generic.block.subblocks);
              gt_ggc_m_9tree_node ((*x).t.generic.block.supercontext);
              gt_ggc_m_9tree_node ((*x).t.generic.block.abstract_origin);
              gt_ggc_m_9tree_node ((*x).t.generic.block.fragment_origin);
              gt_ggc_m_9tree_node ((*x).t.generic.block.fragment_chain);
              break;
            default:
              break;
            }
          break;
        case 1:
          gt_ggc_m_9tree_node ((*x).t.loop_id.common.chain);
          gt_ggc_m_9tree_node ((*x).t.loop_id.common.type);
          gt_ggc_m_7nesting ((*x).t.loop_id.loop_id);
          break;
        default:
          break;
        }
  }
}

/* GC roots.  */

const struct ggc_root_tab gt_ggc_r_gtype_ada_h[] = {
  {
    &gnat_raise_decls[0],
    1 * ((int) LAST_REASON_CODE + 1),
    sizeof (gnat_raise_decls[0]),
    &gt_ggc_mx_tree_node

  },
  {
    &gnat_std_decls[0],
    1 * ((int) ADT_LAST),
    sizeof (gnat_std_decls[0]),
    &gt_ggc_mx_tree_node

  },
  {
    &gnu_block_stack,
    1,
    sizeof (gnu_block_stack),
    &gt_ggc_mx_tree_node

  },
  LAST_GGC_ROOT_TAB
};

extern const struct ggc_root_tab gt_ggc_r_gtype_desc_c[];
extern const struct ggc_root_tab gt_ggc_r_gt_alias_h[];
extern const struct ggc_root_tab gt_ggc_r_gt_cselib_h[];
extern const struct ggc_root_tab gt_ggc_r_gt_dwarf2out_h[];
extern const struct ggc_root_tab gt_ggc_r_gt_emit_rtl_h[];
extern const struct ggc_root_tab gt_ggc_r_gt_except_h[];
extern const struct ggc_root_tab gt_ggc_r_gt_explow_h[];
extern const struct ggc_root_tab gt_ggc_r_gt_expr_h[];
extern const struct ggc_root_tab gt_ggc_r_gt_fold_const_h[];
extern const struct ggc_root_tab gt_ggc_r_gt_function_h[];
extern const struct ggc_root_tab gt_ggc_r_gt_gcse_h[];
extern const struct ggc_root_tab gt_ggc_r_gt_integrate_h[];
extern const struct ggc_root_tab gt_ggc_r_gt_optabs_h[];
extern const struct ggc_root_tab gt_ggc_r_gt_profile_h[];
extern const struct ggc_root_tab gt_ggc_r_gt_ra_build_h[];
extern const struct ggc_root_tab gt_ggc_r_gt_regclass_h[];
extern const struct ggc_root_tab gt_ggc_r_gt_reg_stack_h[];
extern const struct ggc_root_tab gt_ggc_r_gt_sdbout_h[];
extern const struct ggc_root_tab gt_ggc_r_gt_stor_layout_h[];
extern const struct ggc_root_tab gt_ggc_r_gt_varasm_h[];
extern const struct ggc_root_tab gt_ggc_r_gtype_ada_h[];
extern const struct ggc_root_tab gt_ggc_r_gt_ada_decl_h[];
extern const struct ggc_root_tab gt_ggc_r_gt_ada_trans_h[];
extern const struct ggc_root_tab gt_ggc_r_gt_ada_utils_h[];
const struct ggc_root_tab * const gt_ggc_rtab[] = {
  gt_ggc_r_gtype_desc_c,
  gt_ggc_r_gt_alias_h,
  gt_ggc_r_gt_cselib_h,
  gt_ggc_r_gt_dwarf2out_h,
  gt_ggc_r_gt_emit_rtl_h,
  gt_ggc_r_gt_except_h,
  gt_ggc_r_gt_explow_h,
  gt_ggc_r_gt_expr_h,
  gt_ggc_r_gt_fold_const_h,
  gt_ggc_r_gt_function_h,
  gt_ggc_r_gt_gcse_h,
  gt_ggc_r_gt_integrate_h,
  gt_ggc_r_gt_optabs_h,
  gt_ggc_r_gt_profile_h,
  gt_ggc_r_gt_ra_build_h,
  gt_ggc_r_gt_regclass_h,
  gt_ggc_r_gt_reg_stack_h,
  gt_ggc_r_gt_sdbout_h,
  gt_ggc_r_gt_stor_layout_h,
  gt_ggc_r_gt_varasm_h,
  gt_ggc_r_gtype_ada_h,
  gt_ggc_r_gt_ada_decl_h,
  gt_ggc_r_gt_ada_trans_h,
  gt_ggc_r_gt_ada_utils_h,
  NULL
};
extern const struct ggc_root_tab gt_ggc_rd_gt_bitmap_h[];
extern const struct ggc_root_tab gt_ggc_rd_gt_cselib_h[];
extern const struct ggc_root_tab gt_ggc_rd_gt_emit_rtl_h[];
extern const struct ggc_root_tab gt_ggc_rd_gt_lists_h[];
extern const struct ggc_root_tab gt_ggc_rd_gt_ada_utils_h[];
const struct ggc_root_tab * const gt_ggc_deletable_rtab[] = {
  gt_ggc_rd_gt_bitmap_h,
  gt_ggc_rd_gt_cselib_h,
  gt_ggc_rd_gt_emit_rtl_h,
  gt_ggc_rd_gt_lists_h,
  gt_ggc_rd_gt_ada_utils_h,
  NULL
};
extern const struct ggc_cache_tab gt_ggc_rc_gt_emit_rtl_h[];
extern const struct ggc_cache_tab gt_ggc_rc_gt_fold_const_h[];
extern const struct ggc_cache_tab gt_ggc_rc_gt_tree_h[];
const struct ggc_cache_tab * const gt_ggc_cache_rtab[] = {
  gt_ggc_rc_gt_emit_rtl_h,
  gt_ggc_rc_gt_fold_const_h,
  gt_ggc_rc_gt_tree_h,
  NULL
};
