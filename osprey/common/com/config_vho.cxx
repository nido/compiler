/*

  Copyright (C) 2000, 2001 Silicon Graphics, Inc.  All Rights Reserved.

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


/* ====================================================================
 * ====================================================================
 *
 * Module: config_vho.c
 *
 * Revision history:
 *
 * Description:
 *
 * Configure the -VHO group (included in config.c).
 *
 * ====================================================================
 * ====================================================================
 */

/* This file is included in config.c, so it doesn't need its own set of
 * standard includes -- only the following:
 */
#include "config_vho.h"

/* ====================================================================
 *
 * Global flag variables which reflect the -VHO group options.
 *
 * ====================================================================
 */

BOOL    VHO_Force_Lowering              = FALSE;
BOOL    VHO_Struct_Opt                  = TRUE;
BOOL	VHO_Struct_Limit		= 4;
BOOL    VHO_Combine_Loads               = FALSE;
BOOL    VHO_Recycle_Pregs               = FALSE;
INT32   VHO_Switch_Density              = 12;
INT32   VHO_Switch_If_Else_Limit        = 6;
INT32   VHO_Switch_Compgoto_Limit       = 3;
BOOL    VHO_Switch_Opt                  = FALSE;
#ifdef TARG_ST
INT32   VHO_Switch_If_Else_Limit_Space  = 7;
INT32   VHO_Switch_Density_Limit_Space  = 98; 
INT32   VHO_Switch_Old_Control_Space    = FALSE;
#endif
BOOL    VHO_Cselect_Opt                 = FALSE;
BOOL    VHO_Iload_Opt                   = FALSE;
BOOL    VHO_Istore_Opt                  = FALSE;
BOOL    VHO_Call_Opt                    = FALSE;
#ifdef TARG_ST
/* Turn icall to call conversion inside vho_lower to FALSE by default,
   because (1) the original implementation is buggy; and (2) we can now
   do icall to call conversion inside ipa.
*/
BOOL    VHO_Icall_Devir			= FALSE;
#endif  // KEY
BOOL    VHO_Check_Tree                  = FALSE;
BOOL    VHO_Single_Loop_Test            = FALSE;
BOOL    VHO_Use_Do_While                = FALSE;
#ifdef KEY
/* enable misc. loop distribution and interchange at VHO lower time */
BOOL    VHO_Enable_Misc_Loop_Transformation = FALSE;
/* enable misc. loop fusion at VHO lower time */
BOOL    VHO_Enable_Misc_Loop_Fusion = FALSE;
#endif

/* List of global variables to turn on and off various optimizations */

/* ====================================================================
 *
 * Descriptor for the -VHO option group.
 *
 * ====================================================================
 */

/* scalar optimizer options: */
static OPTION_DESC Options_VHO[] = {
  { OVK_BOOL,	OV_INTERNAL,	FALSE, "lower",              "lower",
    FALSE, 0, 0, &VHO_Force_Lowering,  NULL },
  { OVK_BOOL,	OV_INTERNAL,	FALSE, "struct_opt",         "struct_opt",
    TRUE, 0, 0,  &VHO_Struct_Opt,      NULL },
  { OVK_INT32,   OV_INTERNAL,    FALSE, "struct_limit",      "struct_limit",
    4, 0, INT32_MAX,  &VHO_Struct_Limit,    NULL },
  { OVK_BOOL,	OV_INTERNAL,	FALSE, "recycle_pregs",      "recycle_pregs",
    FALSE, 0, 0, &VHO_Recycle_Pregs,   NULL },
  { OVK_BOOL,	OV_INTERNAL,	FALSE, "combine_loads",      "combine_loads",
    FALSE, 0, 0, &VHO_Combine_Loads,   NULL },
  { OVK_INT32,	OV_INTERNAL,	FALSE, "switch_if_else",      "switch_if_else",
    6, 1, INT32_MAX,    &VHO_Switch_If_Else_Limit,   NULL },
  { OVK_INT32,	OV_INTERNAL,	FALSE, "switch_density",      "switch_density",
    12, 1, INT32_MAX,    &VHO_Switch_Density,   NULL },
  { OVK_INT32,	OV_INTERNAL,	FALSE, "switch_compgoto",     "switch_compgoto",
    3, 1, INT32_MAX,    &VHO_Switch_Compgoto_Limit,   NULL },
#ifdef TARG_ST
  { OVK_INT32,  OV_INTERNAL,    FALSE, "switch_if_else_space", "switch_if_else_space",
    7, 1,  INT32_MAX,   &VHO_Switch_If_Else_Limit_Space,   NULL },
  { OVK_INT32,  OV_INTERNAL,    FALSE, "switch_density_space", "switch_density_space",
    98, 1, INT32_MAX,   &VHO_Switch_Density_Limit_Space,   NULL },
  { OVK_BOOL,   OV_INTERNAL,    FALSE, "switch_old_control_space", "switch_old_control_space",
    FALSE, 0, 0,  &VHO_Switch_Old_Control_Space, NULL },
#endif
  { OVK_BOOL,	OV_INTERNAL,	FALSE, "switch_opt",         "switch",
    TRUE, 0, 0,  &VHO_Switch_Opt,      NULL },
  { OVK_BOOL,	OV_INTERNAL,	FALSE, "cselect_opt",        "cselect",
    TRUE, 0, 0,  &VHO_Cselect_Opt,      NULL },
  { OVK_BOOL,	OV_INTERNAL,	FALSE, "iload_opt",          "iload",
    TRUE, 0, 0,  &VHO_Iload_Opt,      NULL },
  { OVK_BOOL,	OV_INTERNAL,	FALSE, "istore_opt",         "istore",
    TRUE, 0, 0,  &VHO_Istore_Opt,      NULL },
  { OVK_BOOL,	OV_INTERNAL,	FALSE, "call_opt",           "call",
    TRUE, 0, 0,  &VHO_Call_Opt,      NULL },
#ifdef KEY
  { OVK_BOOL,	OV_INTERNAL,	TRUE, "icall_devir",          "icall",
    TRUE, 0, 0,  &VHO_Icall_Devir,      NULL },
#endif
  { OVK_BOOL,	OV_INTERNAL,	FALSE, "check_tree",         "check_tree",
    TRUE, 0, 0,  &VHO_Check_Tree,      NULL },
  { OVK_BOOL,	OV_INTERNAL,	FALSE, "single_loop_test",   "single_loop_test",
    TRUE, 0, 0,  &VHO_Single_Loop_Test,      NULL },
  { OVK_BOOL,	OV_INTERNAL,	FALSE, "use_do_while",       "use_do_while",
    TRUE, 0, 0,  &VHO_Use_Do_While,      NULL },
#ifdef KEY
  { OVK_BOOL,   OV_INTERNAL,    TRUE, "misc_loop_transformation", "",
    0, 0, 0,    &VHO_Enable_Misc_Loop_Transformation, NULL },
  { OVK_BOOL,   OV_INTERNAL,    TRUE, "misc_loop_fusion", "",
    0, 0, 0,    &VHO_Enable_Misc_Loop_Fusion, NULL },
#endif
  { OVK_COUNT }		/* List terminator -- must be last */
};