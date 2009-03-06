/* Type information for ada/utils.c.
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
gt_ggc_mx_binding_level (x_p)
      void *x_p;
{
  struct binding_level * const x = (struct binding_level *)x_p;
  if (ggc_test_and_set_mark (x))
    {
      gt_ggc_m_9tree_node ((*x).names);
      gt_ggc_m_9tree_node ((*x).blocks);
      gt_ggc_m_9tree_node ((*x).this_block);
      gt_ggc_m_13binding_level ((*x).level_chain);
  }
}

void
gt_ggc_mx_e_stack (x_p)
      void *x_p;
{
  struct e_stack * const x = (struct e_stack *)x_p;
  if (ggc_test_and_set_mark (x))
    {
      gt_ggc_m_7e_stack ((*x).next);
      gt_ggc_m_9tree_node ((*x).elab_list);
  }
}

void
gt_ggc_mx_language_function (x_p)
      void *x_p;
{
  struct language_function * const x = (struct language_function *)x_p;
  if (ggc_test_and_set_mark (x))
    {
  }
}

/* GC roots.  */

static void gt_ggc_ma_associate_gnat_to_gnu PARAMS ((void *));
static void
gt_ggc_ma_associate_gnat_to_gnu (x_p)
      void *x_p;
{
  size_t i;
  union tree_node ** const x = (union tree_node **)x_p;
  if (ggc_test_and_set_mark (x))
    for (i = 0; i < (max_gnat_nodes); i++)
      gt_ggc_m_9tree_node (x[i]);
}

const struct ggc_root_tab gt_ggc_r_gt_ada_utils_h[] = {
  {
    &current_binding_level,
    1,
    sizeof (current_binding_level),
    &gt_ggc_mx_binding_level

  },
  {
    &float_types[0],
    1 * (NUM_MACHINE_MODES),
    sizeof (float_types[0]),
    &gt_ggc_mx_tree_node

  },
  {
    &signed_and_unsigned_types[0][0],
    1 * (2 * MAX_BITS_PER_WORD + 1) * (2),
    sizeof (signed_and_unsigned_types[0][0]),
    &gt_ggc_mx_tree_node

  },
  {
    &elist_stack,
    1,
    sizeof (elist_stack),
    &gt_ggc_mx_e_stack

  },
  {
    &pending_elaborations,
    1,
    sizeof (pending_elaborations),
    &gt_ggc_mx_tree_node

  },
  {
    &associate_gnat_to_gnu,
    1,
    sizeof (associate_gnat_to_gnu),
    &gt_ggc_ma_associate_gnat_to_gnu
  },
  LAST_GGC_ROOT_TAB
};

const struct ggc_root_tab gt_ggc_rd_gt_ada_utils_h[] = {
  { &free_binding_level, 1, sizeof (free_binding_level), NULL },
  LAST_GGC_ROOT_TAB
};

