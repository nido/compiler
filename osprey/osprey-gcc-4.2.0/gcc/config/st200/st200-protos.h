/* Prototypes for st200.c functions used in the md file & elsewhere.
   Copyright (C) 1999, 2000 Free Software Foundation, Inc.

This file is part of GNU CC.

GNU CC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GNU CC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU CC; see the file COPYING.  If not, write to
the Free Software Foundation, 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#ifndef GCC_ST200_PROTOS_H
#define GCC_ST200_PROTOS_H

extern int  lx_long_imm_operand (rtx, machine_mode_t);
extern int  lx_long_add_operand (rtx, machine_mode_t);
extern void lx_print_operand (FILE *, rtx, int);
extern enum reg_class lx_secondary_reload_class (enum reg_class, machine_mode_t, rtx);

extern rtx  lx_function_arg (CUMULATIVE_ARGS *, machine_mode_t, tree, int);
extern void lx_va_start (tree, rtx);

extern void lx_function_arg_advance (CUMULATIVE_ARGS *, machine_mode_t, tree, int named);
extern rtx  lx_function_value (tree, tree);

extern int  lx_direct_return (void);
extern int  lx_elimination_offset (unsigned int, unsigned int);
extern void lx_expand_prologue (void);
extern void lx_expand_epilogue (void);
extern void st200_optimization_options (int, int);
extern void st200_override_options (void);

#endif /* GCC_ST200_PROTOS_H */
