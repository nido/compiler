/* Type information for java/builtins.c.
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

/* GC roots.  */

const struct ggc_root_tab gt_ggc_r_gt_java_builtins_h[] = {
  {
    &java_builtins[0].class_name.t,
    1 * ARRAY_SIZE (java_builtins),
    sizeof (java_builtins[0]),
    &gt_ggc_mx_tree_node

  },
  {
    &java_builtins[0].method_name.t,
    1 * ARRAY_SIZE (java_builtins),
    sizeof (java_builtins[0]),
    &gt_ggc_mx_tree_node

  },
  LAST_GGC_ROOT_TAB
};

