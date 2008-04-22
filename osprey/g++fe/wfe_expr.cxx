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


#include "W_values.h"
#include "defs.h"
#include "glob.h"
#include "config.h"
#include "wn.h"
#include "wn_util.h"
#include "const.h"

#include "gnu_config.h"
#include "gnu/system.h"

#ifdef KEY
extern "C" {
#include "gnu/flags.h"
}
#endif

extern "C" {
#include "gnu/machmode.h"
#ifdef TARG_ST
#include "gnu/typeclass.h" // Needed to implement __builtin_classify_type
#include "gnu/errors.h"    // Needed to see warning and error
#include "gnu/output.h"    // (cbr) for current_function_has_cleanup
#endif
#include "gnu/system.h"
#include "gnu/tree.h"
#include "gnu/real.h"
#include "gnu/c-common.h"
#ifdef GPLUSPLUS_FE
#include "gnu/cp/cp-tree.h"
#endif /* GPLUSPLUS_FE */
}

#include "ir_reader.h"
#include "wfe_misc.h"
#include "wfe_dst.h"
#include "tree_symtab.h"
#include "wfe_decl.h"
#include "wfe_expr.h"
#include "wfe_stmt.h"
#ifdef TARG_ST
#include <list>
extern "C" {
#include "gnu/tree-inline.h"
}
#include "wfe_pragmas.h"
// [HK] needed for No_Math_Errno
#include "config_opt.h"
#endif

#ifdef TARG_ST
/* (cbr) import from gnu */
extern "C" {
#include <cmplrs/rcodes.h>
  extern int flag_exceptions;
}
#endif

// #define WFE_DEBUG

LABEL_IDX loop_expr_exit_label = 0; // exit label for LOOP_EXPRs


// [HK] add functions to treat expansion of EXACT_DIV_EXPR in case of division by power of 2 
/* ====================================================================
 *   Is_Power_OF_2
 *
 *   return TRUE if the val is a power of 2
 * ====================================================================
 */
#define IS_POWER_OF_2(val)	((val != 0) && ((val & (val-1)) == 0))

static BOOL 
Is_Power_Of_2 (
  INT64 val, 
  TYPE_ID mtype
)
{
  if (MTYPE_is_signed(mtype) && val < 0) val = -val;

  if (mtype == MTYPE_U4) val &= 0xffffffffull;

  return IS_POWER_OF_2(val);
}

/* ====================================================================
 *   Get_Power_OF_2
 * ====================================================================
 */
static INT
Get_Power_Of_2 (
  INT64 val, 
  TYPE_ID mtype
)
{
  INT i;
  INT64 pow2mask;

  if (MTYPE_is_signed(mtype) && val < 0) val = -val;

  if (mtype == MTYPE_U4) val &= 0xffffffffull;

  pow2mask = 1;
  for ( i = 0; i < MTYPE_size_reg(mtype); ++i ) {
    if (val == pow2mask) return i;
    pow2mask <<= 1;
  }

  FmtAssert(FALSE, ("Get_Power_Of_2 unexpected value"));
  /* NOTREACHED */
}
// [HK]

struct operator_from_tree_t {
  int      tree_code;
  char*    name;
  char     code;
  int      nkids;
  OPERATOR opr;
} Operator_From_Tree [] = {
  ERROR_MARK,              "error_mark",              'x', 0,  OPERATOR_UNKNOWN,
  IDENTIFIER_NODE,         "identifier_node",         'x', -1, OPERATOR_UNKNOWN,
#if defined (TARG_ST) && (GNU_FRONT_END==295)
  /* (cbr) gcc 3.3 upgrade */
  OP_IDENTIFIER,           "op_identifier",           'x', 2,  OPERATOR_UNKNOWN,
#endif
  TREE_LIST,               "tree_list",               'x', 2,  OPERATOR_UNKNOWN,
  TREE_VEC,                "tree_vec",                'x', 2,  OPERATOR_UNKNOWN,
  BLOCK,                   "block",                   'b', 0,  OPERATOR_UNKNOWN,
  VOID_TYPE,               "void_type",               't', 0,  OPERATOR_UNKNOWN,
  INTEGER_TYPE,            "integer_type",            't', 0,  OPERATOR_UNKNOWN,
  REAL_TYPE,               "real_type",               't', 0,  OPERATOR_UNKNOWN,
  COMPLEX_TYPE,            "complex_type",            't', 0,  OPERATOR_UNKNOWN,
#if defined (TARG_ST) && (GNU_FRONT_END==33)
  /* (cbr) gcc 3.3 upgrade */
  VECTOR_TYPE,             "vector_type",             't', 0,  OPERATOR_UNKNOWN,
#endif
  ENUMERAL_TYPE,           "enumeral_type",           't', 0,  OPERATOR_UNKNOWN,
  BOOLEAN_TYPE,            "boolean_type",            't', 0,  OPERATOR_UNKNOWN,
  CHAR_TYPE,               "char_type",               't', 0,  OPERATOR_UNKNOWN,
  POINTER_TYPE,            "pointer_type",            't', 0,  OPERATOR_UNKNOWN,
  OFFSET_TYPE,             "offset_type",             't', 0,  OPERATOR_UNKNOWN,
  REFERENCE_TYPE,          "reference_type",          't', 0,  OPERATOR_UNKNOWN,
  METHOD_TYPE,             "method_type",             't', 0,  OPERATOR_UNKNOWN,
  FILE_TYPE,               "file_type",               't', 0,  OPERATOR_UNKNOWN,
  ARRAY_TYPE,              "array_type",              't', 0,  OPERATOR_UNKNOWN,
  SET_TYPE,                "set_type",                't', 0,  OPERATOR_UNKNOWN,
  RECORD_TYPE,             "record_type",             't', 0,  OPERATOR_UNKNOWN,
  UNION_TYPE,              "union_type",              't', 0,  OPERATOR_UNKNOWN,
  QUAL_UNION_TYPE,         "qual_union_type",         't', 0,  OPERATOR_UNKNOWN,
  FUNCTION_TYPE,           "function_type",           't', 0,  OPERATOR_UNKNOWN,
  LANG_TYPE,               "lang_type",               't', 0,  OPERATOR_UNKNOWN,
  INTEGER_CST,             "integer_cst",             'c', 2,  OPERATOR_UNKNOWN,
  REAL_CST,                "real_cst",                'c', 3,  OPERATOR_UNKNOWN,
  COMPLEX_CST,             "complex_cst",             'c', 3,  OPERATOR_UNKNOWN,
#if defined (TARG_ST) && (GNU_FRONT_END==33)
  /* (cbr) gcc 3.3 upgrade */
  VECTOR_CST,              "vector_cst",             'c', 3,  OPERATOR_UNKNOWN,
#endif
  STRING_CST,              "string_cst",              'c', 3,  OPERATOR_UNKNOWN,
  FUNCTION_DECL,           "function_decl",           'd', 0,  OPERATOR_UNKNOWN,
  LABEL_DECL,              "label_decl",              'd', 0,  OPERATOR_UNKNOWN,
  CONST_DECL,              "const_decl",              'd', 0,  OPERATOR_UNKNOWN,
  TYPE_DECL,               "type_decl",               'd', 0,  OPERATOR_UNKNOWN,
  VAR_DECL,                "var_decl",                'd', 0,  OPERATOR_UNKNOWN,
  PARM_DECL,               "parm_decl",               'd', 0,  OPERATOR_UNKNOWN,
  RESULT_DECL,             "result_decl",             'd', 0,  OPERATOR_UNKNOWN,
  FIELD_DECL,              "field_decl",              'd', 0,  OPERATOR_UNKNOWN,
  NAMESPACE_DECL,          "namespace_decl",          'd', 0,  OPERATOR_UNKNOWN,
  COMPONENT_REF,           "component_ref",           'r', 2,  OPERATOR_UNKNOWN,
  BIT_FIELD_REF,           "bit_field_ref",           'r', 3,  OPERATOR_UNKNOWN,
  INDIRECT_REF,            "indirect_ref",            'r', 1,  OPERATOR_UNKNOWN,
  BUFFER_REF,              "buffer_ref",              'r', 1,  OPERATOR_UNKNOWN,
  ARRAY_REF,               "array_ref",               'r', 2,  OPERATOR_UNKNOWN,
#if defined (TARG_ST) && (GNU_FRONT_END==33)
  /* (cbr) gcc 3.3 upgrade */
  ARRAY_RANGE_REF,         "array_range_ref",         'r', 2,  OPERATOR_UNKNOWN,
  VTABLE_REF,              "vtable_ref",              'r', 3,  OPERATOR_UNKNOWN,
#endif
  CONSTRUCTOR,             "constructor",             'e', 2,  OPERATOR_UNKNOWN,
  COMPOUND_EXPR,           "compound_expr",           'e', 2,  OPERATOR_UNKNOWN,
  MODIFY_EXPR,             "modify_expr",             'e', 2,  OPERATOR_UNKNOWN,
  INIT_EXPR,               "init_expr",               'e', 2,  OPERATOR_UNKNOWN,
  TARGET_EXPR,             "target_expr",             'e', 4,  OPERATOR_UNKNOWN,
  COND_EXPR,               "cond_expr",               'e', 3,  OPERATOR_UNKNOWN,
  BIND_EXPR,               "bind_expr",               'e', 3,  OPERATOR_UNKNOWN,
  CALL_EXPR,               "call_expr",               'e', 3,  OPERATOR_UNKNOWN,
  METHOD_CALL_EXPR,        "method_call_expr",        'e', 4,  OPERATOR_UNKNOWN,
  WITH_CLEANUP_EXPR,       "with_cleanup_expr",       'e', 3,  OPERATOR_UNKNOWN,
  CLEANUP_POINT_EXPR,      "cleanup_point_expr",      'e', 1,  OPERATOR_UNKNOWN,
  PLACEHOLDER_EXPR,        "placeholder_expr",        'x', 0,  OPERATOR_UNKNOWN,
  WITH_RECORD_EXPR,        "with_record_expr",        'e', 2,  OPERATOR_UNKNOWN,
  PLUS_EXPR,               "plus_expr",               '2', 2,  OPR_ADD,
  MINUS_EXPR,              "minus_expr",              '2', 2,  OPR_SUB,
  MULT_EXPR,               "mult_expr",               '2', 2,  OPR_MPY,
  TRUNC_DIV_EXPR,          "trunc_div_expr",          '2', 2,  OPR_DIV,
  CEIL_DIV_EXPR,           "ceil_div_expr",           '2', 2,  OPR_DIV,
  FLOOR_DIV_EXPR,          "floor_div_expr",          '2', 2,  OPERATOR_UNKNOWN,
  ROUND_DIV_EXPR,          "round_div_expr",          '2', 2,  OPERATOR_UNKNOWN,
  TRUNC_MOD_EXPR,          "trunc_mod_expr",          '2', 2,  OPR_REM,
  CEIL_MOD_EXPR,           "ceil_mod_expr",           '2', 2,  OPERATOR_UNKNOWN,
  FLOOR_MOD_EXPR,          "floor_mod_expr",          '2', 2,  OPERATOR_UNKNOWN,
  ROUND_MOD_EXPR,          "round_mod_expr",          '2', 2,  OPERATOR_UNKNOWN,
  RDIV_EXPR,               "rdiv_expr",               '2', 2,  OPR_DIV,
  EXACT_DIV_EXPR,          "exact_div_expr",          '2', 2,  OPR_DIV,
  FIX_TRUNC_EXPR,          "fix_trunc_expr",          '1', 1,  OPERATOR_UNKNOWN,
  FIX_CEIL_EXPR,           "fix_ceil_expr",           '1', 1,  OPERATOR_UNKNOWN,
  FIX_FLOOR_EXPR,          "fix_floor_expr",          '1', 1,  OPERATOR_UNKNOWN,
  FIX_ROUND_EXPR,          "fix_round_expr",          '1', 1,  OPERATOR_UNKNOWN,
  FLOAT_EXPR,              "float_expr",              '1', 1,  OPERATOR_UNKNOWN,
#if defined (TARG_ST) && (GNU_FRONT_END==295)
  /* (cbr) gcc 3.3 upgrade */
  EXPON_EXPR,              "expon_expr",              '2', 2,  OPERATOR_UNKNOWN,
#endif
  NEGATE_EXPR,             "negate_expr",             '1', 1,  OPR_NEG,
  MIN_EXPR,                "min_expr",                '2', 2,  OPR_MIN,
  MAX_EXPR,                "max_expr",                '2', 2,  OPR_MAX,
  ABS_EXPR,                "abs_expr",                '1', 1,  OPR_ABS,
  FFS_EXPR,                "ffs_expr",                '1', 1,  OPERATOR_UNKNOWN,
  LSHIFT_EXPR,             "lshift_expr",             '2', 2,  OPR_SHL,
  RSHIFT_EXPR,             "rshift_expr",             '2', 2,  OPERATOR_UNKNOWN,
  LROTATE_EXPR,            "lrotate_expr",            '2', 2,  OPR_RROTATE,
  RROTATE_EXPR,            "rrotate_expr",            '2', 2,  OPR_RROTATE,
  BIT_IOR_EXPR,            "bit_ior_expr",            '2', 2,  OPR_BIOR,
  BIT_XOR_EXPR,            "bit_xor_expr",            '2', 2,  OPR_BXOR,
  BIT_AND_EXPR,            "bit_and_expr",            '2', 2,  OPR_BAND,
  BIT_ANDTC_EXPR,          "bit_andtc_expr",          '2', 2,  OPERATOR_UNKNOWN,
  BIT_NOT_EXPR,            "bit_not_expr",            '1', 1,  OPR_BNOT,
  TRUTH_ANDIF_EXPR,        "truth_andif_expr",        'e', 2,  OPR_CAND,
  TRUTH_ORIF_EXPR,         "truth_orif_expr",         'e', 2,  OPR_CIOR,
  TRUTH_AND_EXPR,          "truth_and_expr",          'e', 2,  OPR_BAND,
  TRUTH_OR_EXPR,           "truth_or_expr",           'e', 2,  OPR_BIOR,
  TRUTH_XOR_EXPR,          "truth_xor_expr",          'e', 2,  OPR_BXOR,
  TRUTH_NOT_EXPR,          "truth_not_expr",          'e', 1,  OPR_LNOT,
  LT_EXPR,                 "lt_expr",                 '<', 2,  OPR_LT,
  LE_EXPR,                 "le_expr",                 '<', 2,  OPR_LE,
  GT_EXPR,                 "gt_expr",                 '<', 2,  OPR_GT,
  GE_EXPR,                 "ge_expr",                 '<', 2,  OPR_GE,
  EQ_EXPR,                 "eq_expr",                 '<', 2,  OPR_EQ,
  NE_EXPR,                 "ne_expr",                 '<', 2,  OPR_NE,
  UNORDERED_EXPR,          "unordered_expr",          '<', 2,  OPERATOR_UNKNOWN,
  ORDERED_EXPR,            "ordered_expr",            '<', 2,  OPERATOR_UNKNOWN,
#if defined (TARG_ST) && (GNU_FRONT_END==33)
  /* (cbr) that should be safe to map those since we don't have exceptions anyway */
  UNLT_EXPR,               "unlt_expr",               '<', 2,  OPR_LT,
  UNLE_EXPR,               "unle_expr",               '<', 2,  OPR_LE,
  UNGT_EXPR,               "ungt_expr",               '<', 2,  OPR_GT,
  UNGE_EXPR,               "unge_expr",               '<', 2,  OPR_GE,
  UNEQ_EXPR,               "uneq_expr",               '<', 2,  OPR_EQ,
#else
  UNLT_EXPR,               "unlt_expr",               '<', 2,  OPERATOR_UNKNOWN,
  UNLE_EXPR,               "unle_expr",               '<', 2,  OPERATOR_UNKNOWN,
  UNGT_EXPR,               "ungt_expr",               '<', 2,  OPERATOR_UNKNOWN,
  UNGE_EXPR,               "unge_expr",               '<', 2,  OPERATOR_UNKNOWN,
  UNEQ_EXPR,               "uneq_expr",               '<', 2,  OPERATOR_UNKNOWN,
#endif
  IN_EXPR,                 "in_expr",                 '2', 2,  OPERATOR_UNKNOWN,
  SET_LE_EXPR,             "set_le_expr",             '<', 2,  OPERATOR_UNKNOWN,
  CARD_EXPR,               "card_expr",               '1', 1,  OPERATOR_UNKNOWN,
  RANGE_EXPR,              "range_expr",              '2', 2,  OPERATOR_UNKNOWN,
  CONVERT_EXPR,            "convert_expr",            '1', 1,  OPERATOR_UNKNOWN,
  NOP_EXPR,                "nop_expr",                '1', 1,  OPERATOR_UNKNOWN,
  NON_LVALUE_EXPR,         "non_lvalue_expr",         '1', 1,  OPERATOR_UNKNOWN,
#if defined (TARG_ST) && (GNU_FRONT_END==33)
  /* (cbr) gcc 3.3 upgrade */
  VIEW_CONVERT_EXPR,       "view_convert_expr",       '1', 1,  OPERATOR_UNKNOWN,
#endif
  SAVE_EXPR,               "save_expr",               'e', 3,  OPERATOR_UNKNOWN,
  UNSAVE_EXPR,             "unsave_expr",             'e', 1,  OPERATOR_UNKNOWN,
  RTL_EXPR,                "rtl_expr",                'e', 2,  OPERATOR_UNKNOWN,
  ADDR_EXPR,               "addr_expr",               'e', 1,  OPERATOR_UNKNOWN,
  REFERENCE_EXPR,          "reference_expr",          'e', 1,  OPERATOR_UNKNOWN,
  ENTRY_VALUE_EXPR,        "entry_value_expr",        'e', 1,  OPERATOR_UNKNOWN,
#if defined (TARG_ST) && (GNU_FRONT_END==33)
  /* (cbr) gcc 3.3 upgrade */
  FDESC_EXPR,              "fdesc_expr",              'e', 2,  OPERATOR_UNKNOWN,
#endif
  COMPLEX_EXPR,            "complex_expr",            '2', 2,  OPR_PAIR,
  CONJ_EXPR,               "conj_expr",               '1', 1,  OPERATOR_UNKNOWN,
  REALPART_EXPR,           "realpart_expr",           '1', 1,  OPR_FIRSTPART,
  IMAGPART_EXPR,           "imagpart_expr",           '1', 1,  OPR_SECONDPART,
  PREDECREMENT_EXPR,       "predecrement_expr",       'e', 2,  OPR_SUB,
  PREINCREMENT_EXPR,       "preincrement_expr",       'e', 2,  OPR_ADD,
  POSTDECREMENT_EXPR,      "postdecrement_expr",      'e', 2,  OPR_SUB,
  POSTINCREMENT_EXPR,      "postincrement_expr",      'e', 2,  OPR_ADD,
  VA_ARG_EXPR,             "va_arg_expr",             'e', 1,  OPERATOR_UNKNOWN,
  TRY_CATCH_EXPR,          "try_catch_expr",          'e', 2,  OPERATOR_UNKNOWN,
  TRY_FINALLY_EXPR,        "try_finally",             'e', 2,  OPERATOR_UNKNOWN,
  GOTO_SUBROUTINE_EXPR,    "goto_subroutine",         'e', 2,  OPERATOR_UNKNOWN,
#if defined (TARG_ST) && (GNU_FRONT_END==295)
  /* (cbr) gcc 3.3 upgrade */
  POPDHC_EXPR,             "popdhc_expr",             's', 0,  OPERATOR_UNKNOWN,
  POPDCC_EXPR,             "popdcc_expr",             's', 0,  OPERATOR_UNKNOWN,
#endif
  LABEL_EXPR,              "label_expr",              's', 1,  OPERATOR_UNKNOWN,
  GOTO_EXPR,               "goto_expr",               's', 1,  OPERATOR_UNKNOWN,
  RETURN_EXPR,             "return_expr",             's', 1,  OPERATOR_UNKNOWN,
  EXIT_EXPR,               "exit_expr",               's', 1,  OPERATOR_UNKNOWN,
  LOOP_EXPR,               "loop_expr",               's', 1,  OPERATOR_UNKNOWN,
  LABELED_BLOCK_EXPR,      "labeled_block_expr",      'e', 2,  OPERATOR_UNKNOWN,
  EXIT_BLOCK_EXPR,         "exit_block_expr",         'e', 2,  OPERATOR_UNKNOWN,
  EXPR_WITH_FILE_LOCATION, "expr_with_file_location", 'e', 3,  OPERATOR_UNKNOWN,
  SWITCH_EXPR,             "switch_expr",             'e', 2,  OPERATOR_UNKNOWN,
#if defined (TARG_ST) && (GNU_FRONT_END==33)
  /* (cbr) gcc 3.3 upgrade */
  EXC_PTR_EXPR,            "exc_ptr_expr",            'e', 0,  OPERATOR_UNKNOWN,
  LAST_AND_UNUSED_TREE_CODE,"last_and_unused_tree_code",0, 0,  OPERATOR_UNKNOWN,
  SRCLOC,                  "srcloc",                  'x', 2,  OPERATOR_UNKNOWN,
  SIZEOF_EXPR,             "sizeof_expr",             '1', 1,  OPERATOR_UNKNOWN,
  ARROW_EXPR,              "arrow_expr",              'e', 1,  OPERATOR_UNKNOWN,
  ALIGNOF_EXPR,            "alignof_expr",            '1', 1,  OPERATOR_UNKNOWN,
  EXPR_STMT,               "expr_stmt",               'e', 1,  OPERATOR_UNKNOWN,
  COMPOUND_STMT,           "compound_stmt",           'e', 1,  OPERATOR_UNKNOWN,
  DECL_STMT,               "decl_stmt",               'e', 1,  OPERATOR_UNKNOWN,
  IF_STMT,                 "if_stmt",                 'e', 3,  OPERATOR_UNKNOWN,
  FOR_STMT,                "for_stmt",                'e', 4,  OPERATOR_UNKNOWN,
  WHILE_STMT,              "while_stmt",              'e', 2,  OPERATOR_UNKNOWN,
  DO_STMT,                 "do_stmt",                 'e', 2,  OPERATOR_UNKNOWN,
  RETURN_STMT,             "return_stmt",             'e', 1,  OPERATOR_UNKNOWN,
  BREAK_STMT,              "break_stmt",              'e', 0,  OPERATOR_UNKNOWN,
  CONTINUE_STMT,           "continue_stmt",           'e', 0,  OPERATOR_UNKNOWN,
  SWITCH_STMT,             "switch_stmt",             'e', 2,  OPERATOR_UNKNOWN,
  GOTO_STMT,               "goto_stmt",               'e', 1,  OPERATOR_UNKNOWN,
  LABEL_STMT,              "label_stmt",              'e', 1,  OPERATOR_UNKNOWN,
  ASM_STMT,                "asm_stmt",                'e', 5,  OPERATOR_UNKNOWN,
  SCOPE_STMT,              "scope_stmt",              'e', 1,  OPERATOR_UNKNOWN,
  FILE_STMT,               "file_stmt",               'e', 1,  OPERATOR_UNKNOWN,
  CASE_LABEL,              "case_label",              'e', 2,  OPERATOR_UNKNOWN,
  STMT_EXPR,               "stmt_expr",               'e', 1,  OPERATOR_UNKNOWN,
  COMPOUND_LITERAL_EXPR,   "compound_literal_expr",   'e', 1,  OPERATOR_UNKNOWN,
  CLEANUP_STMT,            "cleanup_stmt",            'e', 2,  OPERATOR_UNKNOWN,
#ifdef HANDLE_WFE_PRAGMAS
  PRAGMA_STMT,             "pragma_stmt",           'e', 2,  OPERATOR_UNKNOWN,
#endif

  CP_DUMMY_TREE_CODE,      "cp_dummy_tree_code",        0, 0,  OPERATOR_UNKNOWN,
#else
  LAST_AND_UNUSED_TREE_CODE,"last_and_unused_tree_code",0, 0,  OPERATOR_UNKNOWN,
#endif
#ifdef GPLUSPLUS_FE
  OFFSET_REF,              "offset_ref",              'r', 2,  OPERATOR_UNKNOWN,
  PTRMEM_CST,              "ptrmem_cst",              'c', 2,  OPERATOR_UNKNOWN,
  NEW_EXPR,                "nw_expr",                 'e', 3,  OPERATOR_UNKNOWN,
  VEC_NEW_EXPR,            "vec_nw_expr",             'e', 3,  OPERATOR_UNKNOWN,
  DELETE_EXPR,             "dl_expr",                 'e', 2,  OPERATOR_UNKNOWN,
  VEC_DELETE_EXPR,         "vec_dl_expr",             'e', 2,  OPERATOR_UNKNOWN,
  SCOPE_REF,               "scope_ref",               'r', 2,  OPERATOR_UNKNOWN,
  MEMBER_REF,              "member_ref",              'r', 2,  OPERATOR_UNKNOWN,
  TYPE_EXPR,               "type_expr",               'e', 1,  OPERATOR_UNKNOWN,
  AGGR_INIT_EXPR,          "aggr_init_expr",          'e', 3,  OPERATOR_UNKNOWN,
  THROW_EXPR,              "throw_expr",              'e', 1,  OPERATOR_UNKNOWN,
  EMPTY_CLASS_EXPR,        "empty_class_expr",        'e', 0,  OPERATOR_UNKNOWN,
#if defined (TARG_ST) && (GNU_FRONT_END==33)
  /* (cbr) gcc 3.3 upgrade */
  BASELINK,                "baselink",                'e', 3,  OPERATOR_UNKNOWN,
#endif
  TEMPLATE_DECL,           "template_decl",           'd', 0,  OPERATOR_UNKNOWN,
  TEMPLATE_PARM_INDEX,     "template_parm_index",     'x', 0,  OPERATOR_UNKNOWN,
  TEMPLATE_TYPE_PARM,      "template_type_parm",      't', 0,  OPERATOR_UNKNOWN,
  TEMPLATE_TEMPLATE_PARM,  "template_template_parm",  't', 0,  OPERATOR_UNKNOWN,
#if defined (TARG_ST) && (GNU_FRONT_END==33)
  /* (cbr) gcc 3.3 upgrade */
  BOUND_TEMPLATE_TEMPLATE_PARM,  "bound_template_template_parm",  't', 0,  OPERATOR_UNKNOWN,
#endif
  TYPENAME_TYPE,           "typename_type",           't', 0,  OPERATOR_UNKNOWN,
#if defined (TARG_ST) && (GNU_FRONT_END==33)
  /* (cbr) gcc 3.3 upgrade */
  UNBOUND_CLASS_TEMPLATE,  "unbound_class_template",  't', 0,  OPERATOR_UNKNOWN,
#endif
  TYPEOF_TYPE,             "typeof_type",             't', 0,  OPERATOR_UNKNOWN,
  USING_DECL,              "using_decl",              'd', 0,  OPERATOR_UNKNOWN,
#if defined (TARG_ST) && (GNU_FRONT_END==33)
  /* (cbr) gcc 3.3 upgrade */
  USING_STMT,              "using_directive",         'e', 1,  OPERATOR_UNKNOWN,
#endif
  DEFAULT_ARG,             "default_arg",             'c', 2,  OPERATOR_UNKNOWN,
  TEMPLATE_ID_EXPR,        "template_id_expr",        'e', 2,  OPERATOR_UNKNOWN,
#if defined (TARG_ST) && (GNU_FRONT_END==295)
  /* (cbr) gcc 3.3 upgrade */
  CPLUS_BINDING,           "binding",                 'x', 2,  OPERATOR_UNKNOWN,
#endif
  OVERLOAD,                "overload",                'x', 1,  OPERATOR_UNKNOWN,
  WRAPPER,                 "wrapper",                 'x', 1,  OPERATOR_UNKNOWN,
#if defined (TARG_ST) && (GNU_FRONT_END==295)
  /* (cbr) gcc 3.3 upgrade */
  SRCLOC,                  "srcloc",                  'x', 2,  OPERATOR_UNKNOWN,
#endif
  LOOKUP_EXPR,             "lookup_expr",             'e', 1,  OPERATOR_UNKNOWN,
#if defined (TARG_ST) && (GNU_FRONT_END==295)
  /* (cbr) gcc 3.3 upgrade */
  FUNCTION_NAME,           "function_name",           'e', 0,  OPERATOR_UNKNOWN,
#endif
  MODOP_EXPR,              "modop_expr",              'e', 3,  OPERATOR_UNKNOWN,
  CAST_EXPR,               "cast_expr",               '1', 1,  OPERATOR_UNKNOWN,
  REINTERPRET_CAST_EXPR,   "reinterpret_cast_expr",   '1', 1,  OPERATOR_UNKNOWN,
  CONST_CAST_EXPR,         "const_cast_expr",         '1', 1,  OPERATOR_UNKNOWN,
  STATIC_CAST_EXPR,        "static_cast_expr",        '1', 1,  OPERATOR_UNKNOWN,
  DYNAMIC_CAST_EXPR,       "dynamic_cast_expr",       '1', 1,  OPERATOR_UNKNOWN,
#if defined (TARG_ST) && (GNU_FRONT_END==295)
  /* (cbr) gcc 3.3 upgrade */
  SIZEOF_EXPR,             "sizeof_expr",             '1', 1,  OPERATOR_UNKNOWN,
  ALIGNOF_EXPR,            "alignof_expr",            '1', 1,  OPERATOR_UNKNOWN,
  ARROW_EXPR,              "arrow_expr",              'e', 1,  OPERATOR_UNKNOWN,
#endif
  DOTSTAR_EXPR,            "dotstar_expr",            'e', 2,  OPERATOR_UNKNOWN,
  TYPEID_EXPR,             "typeid_expr",             'e', 1,  OPERATOR_UNKNOWN,
  PSEUDO_DTOR_EXPR,        "pseudo_dtor_expr",        'e', 3,  OPERATOR_UNKNOWN,
#if defined (TARG_ST) && (GNU_FRONT_END==295)
  /* (cbr) gcc 3.3 upgrade */
  EXPR_STMT,               "expr_stmt",               'e', 1,  OPERATOR_UNKNOWN,
  COMPOUND_STMT,           "compound_stmt",           'e', 1,  OPERATOR_UNKNOWN,
  DECL_STMT,               "decl_stmt",               'e', 1,  OPERATOR_UNKNOWN,
  IF_STMT,                 "if_stmt",                 'e', 3,  OPERATOR_UNKNOWN,
  FOR_STMT,                "for_stmt",                'e', 4,  OPERATOR_UNKNOWN,
  WHILE_STMT,              "while_stmt",              'e', 2,  OPERATOR_UNKNOWN,
  DO_STMT,                 "do_stmt",                 'e', 2,  OPERATOR_UNKNOWN,
  RETURN_STMT,             "return_stmt",             'e', 1,  OPERATOR_UNKNOWN,
  BREAK_STMT,              "break_stmt",              'e', 0,  OPERATOR_UNKNOWN,
  CONTINUE_STMT,           "continue_stmt",           'e', 0,  OPERATOR_UNKNOWN,
  SWITCH_STMT,             "switch_stmt",             'e', 2,  OPERATOR_UNKNOWN,
  GOTO_STMT,               "goto_stmt",               'e', 1,  OPERATOR_UNKNOWN,
  LABEL_STMT,              "label_stmt",              'e', 1,  OPERATOR_UNKNOWN,
  ASM_STMT,                "asm_stmt",                'e', 5,  OPERATOR_UNKNOWN,
  SUBOBJECT,               "subobject",               'e', 1,  OPERATOR_UNKNOWN,
  CTOR_STMT,               "ctor_stmt",               'e', 0,  OPERATOR_UNKNOWN,
  CLEANUP_STMT,            "cleanup_stmt",            'e', 2,  OPERATOR_UNKNOWN,
  START_CATCH_STMT,        "start_catch_stmt",        'e', 0,  OPERATOR_UNKNOWN,
  SCOPE_STMT,              "scope_stmt",              'e', 1,  OPERATOR_UNKNOWN,
#endif
  CTOR_INITIALIZER,        "ctor_initializer",        'e', 2,  OPERATOR_UNKNOWN,
#if defined (TARG_ST) && (GNU_FRONT_END==295)
  /* (cbr) gcc 3.3 upgrade */
  CASE_LABEL,              "case_label",              'e', 2,  OPERATOR_UNKNOWN,
#endif
  RETURN_INIT,             "return_init",             'e', 2,  OPERATOR_UNKNOWN,
  TRY_BLOCK,               "try_block",               'e', 2,  OPERATOR_UNKNOWN,
#if defined (TARG_ST) && (GNU_FRONT_END==33)
  /* (cbr) gcc 3.3 upgrade */
  EH_SPEC_BLOCK,           "eh_spec_block",           'e', 2,  OPERATOR_UNKNOWN,
#endif
  HANDLER,                 "handler",                 'e', 2,  OPERATOR_UNKNOWN,
#if defined (TARG_ST) && (GNU_FRONT_END==295)
  /* (cbr) gcc 3.3 upgrade */
  STMT_EXPR,               "stmt_expr",               'e', 1,  OPERATOR_UNKNOWN,
#endif
#if defined (TARG_ST) && (GNU_FRONT_END==33)
  /* (cbr) gcc 3.3 upgrade */
  MUST_NOT_THROW_EXPR,     "must_not_throw_expr",     'e', 1,  OPERATOR_UNKNOWN,
#endif
  TAG_DEFN,                "tag_defn",                'e', 0,  OPERATOR_UNKNOWN,
  IDENTITY_CONV,           "identity_conv",           'e', 1,  OPERATOR_UNKNOWN,
  LVALUE_CONV,             "lvalue_conv",             'e', 1,  OPERATOR_UNKNOWN,
  QUAL_CONV,               "qual_conv",               'e', 1,  OPERATOR_UNKNOWN,
  STD_CONV,                "std_conv",                'e', 1,  OPERATOR_UNKNOWN,
  PTR_CONV,                "ptr_conv",                'e', 1,  OPERATOR_UNKNOWN,
  PMEM_CONV,               "pmem_conv",               'e', 1,  OPERATOR_UNKNOWN,
  BASE_CONV,               "base_conv",               'e', 1,  OPERATOR_UNKNOWN,
  REF_BIND,                "ref_bind",                'e', 1,  OPERATOR_UNKNOWN,
  USER_CONV,               "user_conv",               'e', 2,  OPERATOR_UNKNOWN,
  AMBIG_CONV,              "ambig_conv",              'e', 1,  OPERATOR_UNKNOWN,
  RVALUE_CONV,             "rvalue_conv",             'e', 1,  OPERATOR_UNKNOWN,
  LAST_CPLUS_TREE_CODE,    "last_cplus_tree_code",     0,  0,  OPERATOR_UNKNOWN
#endif /* GPLUSPLUSFE */
};

#ifndef TARG_ST
// [CG]: This function should not be used. Simplification on cvtl are
// already handled in WN_Simplify...() functions

// check whether the WHIRL operator has subsumed cvtl in its semantics
// (intended only for integer operations)
bool
Has_Subsumed_Cvtl(OPERATOR opr)
{
  if (OPERATOR_is_load(opr) || OPERATOR_is_leaf(opr))
    return TRUE;
  if (opr == OPR_CVT || opr == OPR_CVTL || opr == OPR_TAS)
    return TRUE;
  if (opr == OPR_EQ || opr == OPR_NE ||
      opr == OPR_GE || opr == OPR_GT ||
      opr == OPR_LE || opr == OPR_LT ||
      opr == OPR_LNOT || opr == OPR_LAND || opr == OPR_LIOR ||
      opr == OPR_CAND || opr == OPR_CIOR)
    return TRUE;
  return FALSE;
}
#endif

#ifdef TARG_ST
// [CG] Make standard integral ctype promotions/conversions
//#define STANDARD_C_CONVERSION
extern WN *WFE_Cast(TYPE_ID mtype, TYPE_ID kid_mtype, WN *kid);
extern TYPE_ID WFE_Promoted_Type(TYPE_ID mtype);
extern TYPE_ID WFE_Promoted_Binary_Type(TYPE_ID mtype1, TYPE_ID mtype2);
#endif

#if defined (TARG_ST) && (GNU_FRONT_END==33)
/* (cbr) gcc 3.3 upgrade */
extern double real_value_to_double (REAL_VALUE_TYPE);
extern "C" {
extern tree c_strlen (tree);
           }
#endif

#ifdef TARG_ST
// [CG]: Helper for appending an  expression statement.
// Generate EVAL(wn) or directly the wn when
// it is a statement node.
// Return the node effectively generated
static WN *WFE_Append_Expr_Stmt(WN *wn)
{
  if (WN_operator (wn) != OPR_PREFETCH)
    wn = WN_CreateEval (wn);
  WFE_Stmt_Append (wn, Get_Srcpos ());
  return wn;
}
#endif

// Round up an object size to the size it would require in the parameter
// area on the stack.  This is defined to be the difference between its
// start address and the lowest possible starting address of the next parameter.
inline UINT64 Parameter_Size(UINT64 sz)
{
#ifdef TARG_ST
  if (Target_Byte_Sex == BIG_ENDIAN) {
    return sz;
  } else {
    return (sz + UNITS_PER_WORD - 1) & ~(UNITS_PER_WORD - 1);
  }
#else
#   if WORDS_BIG_ENDIAN
	return sz;
#   else
	return (sz + UNITS_PER_WORD - 1) & ~(UNITS_PER_WORD - 1);
#   endif
#endif
}

inline TYPE_ID
Widen_Mtype (TYPE_ID t)
{
  if (MTYPE_is_m(t))
    return t;
  if (MTYPE_is_void(t) || t == MTYPE_BS) {
    Fail_FmtAssertion ("Widen_Mtype: for MTYPE_V or MTYPE_BS");
    return t;
  }
  if (MTYPE_byte_size(t) >= 4)
    return t;
  return Mtype_TransferSize(MTYPE_I4, t);
}

// Traverse the tree to see if the address of a variable is being taken

void
WFE_Set_ST_Addr_Saved (WN *wn)
{
  OPERATOR  Operator;
  ST       *st;

  Operator = WN_operator (wn);

  switch ( Operator ) {

    case OPR_LDA:
    case OPR_LDMA:

      st = WN_st (wn);

      if (ST_class(st) == CLASS_VAR || ST_class(st) == CLASS_FUNC)
        Set_ST_addr_saved (st);
      break;

    case OPR_ARRAY:

      WFE_Set_ST_Addr_Saved (WN_kid0 (wn));
      break;

    case OPR_LDID:

      st = WN_st (wn);
      if (ST_pt_to_unique_mem (st))
        Clear_ST_pt_to_unique_mem (st);
      break;

    case OPR_CONST:
    case OPR_ILOAD:
    case OPR_MLOAD:
    case OPR_INTCONST:
    case OPR_INTRINSIC_OP:
    case OPR_CALL:
    case OPR_EQ:
    case OPR_NE:
    case OPR_GT:
    case OPR_GE:
    case OPR_LT:
    case OPR_LE:
    case OPR_ALLOCA:
      break;

    case OPR_EVAL:
    case OPR_TAS:
    case OPR_CVT:
    case OPR_CVTL:
    case OPR_NEG:
    case OPR_ABS:
    case OPR_SQRT:
    case OPR_REALPART:
    case OPR_IMAGPART:
    case OPR_PAREN:
    case OPR_RND:
    case OPR_TRUNC:
    case OPR_CEIL:
    case OPR_FLOOR:
    case OPR_BNOT:
    case OPR_LNOT:
    case OPR_LOWPART:
    case OPR_HIGHPART:
    case OPR_MINPART:
    case OPR_MAXPART:
    case OPR_RECIP:
    case OPR_RSQRT:
    case OPR_PARM:
    case OPR_EXTRACT_BITS:

      WFE_Set_ST_Addr_Saved (WN_kid0(wn));
      break;

    case OPR_CSELECT:

      WFE_Set_ST_Addr_Saved (WN_kid1(wn));
      WFE_Set_ST_Addr_Saved (WN_kid2(wn));
      break;

    case OPR_SELECT:
    case OPR_ADD:
    case OPR_SUB:
    case OPR_MPY:
    case OPR_DIV:
    case OPR_MOD:
    case OPR_REM:
    case OPR_DIVREM:
    case OPR_MAX:
    case OPR_MIN:
    case OPR_MINMAX:
    case OPR_BAND:
    case OPR_BIOR:
    case OPR_BXOR:
    case OPR_BNOR:
    case OPR_LAND:
    case OPR_LIOR:
    case OPR_SHL:
    case OPR_ASHR:
    case OPR_LSHR:
    case OPR_COMPLEX:
    case OPR_HIGHMPY:
    case OPR_RROTATE:
    case OPR_COMPOSE_BITS:

      WFE_Set_ST_Addr_Saved (WN_kid0(wn));
      WFE_Set_ST_Addr_Saved (WN_kid1(wn));
      break;

    case OPR_CAND:
    case OPR_CIOR:

      break;

    case OPR_COMMA:

      WFE_Set_ST_Addr_Saved (WN_kid1(wn));
      break;

    case OPR_RCOMMA:

      WFE_Set_ST_Addr_Saved (WN_kid0(wn));
      break;

#ifdef TARG_ST
      // [CG]: set flag for LDA of label
  case OPR_LDA_LABEL:
    Set_LABEL_addr_saved (WN_label_number(wn));
    break;
#endif

    default:

      DevWarn ("WFE_Set_ST_Addr_Saved not implemented");
  }
} /* WFE_Set_ST_Addr_Saved */

#ifndef GPLUSPLUS_FE
typedef struct wfe_bind_expr_t {
  tree  rtl_expr;
  WN   *block;
} WFE_BIND_EXPR;

WFE_BIND_EXPR *wfe_bind_expr_stack       = NULL;
INT32          wfe_bind_expr_stack_last  = -1;
INT32          wfe_bind_expr_stack_max   = 0;

void
WFE_Expand_Start_Stmt_Expr (tree t)
{
  WN *block = WN_CreateBlock ();
  WFE_Stmt_Push (block, wfe_stmk_comma, Get_Srcpos ());
} /* WFE_Start_Stmt_Expr */

void
WFE_Expand_End_Stmt_Expr (tree t)
{
  WN *block = WFE_Stmt_Pop (wfe_stmk_comma);
  ++wfe_bind_expr_stack_last;
  if (wfe_bind_expr_stack_last == wfe_bind_expr_stack_max) {
    if (wfe_bind_expr_stack == NULL) {
      wfe_bind_expr_stack_max = 32;
      // [HK]
#if __GNUC__ >= 3
      wfe_bind_expr_stack     =        (WFE_BIND_EXPR *) xmalloc (wfe_bind_expr_stack_max *
#else
      wfe_bind_expr_stack     =        (WFE_BIND_EXPR *) malloc (wfe_bind_expr_stack_max *
#endif // __GNUC__ >= 3
                                  sizeof (WFE_BIND_EXPR));
    }
    else {
      wfe_bind_expr_stack_max = wfe_bind_expr_stack_max +
                                (wfe_bind_expr_stack_max >> 1);
      wfe_bind_expr_stack     =
        (WFE_BIND_EXPR *) realloc (wfe_bind_expr_stack,
                                   wfe_bind_expr_stack_max *
                                   sizeof (WFE_BIND_EXPR));
    }
  }
  wfe_bind_expr_stack [wfe_bind_expr_stack_last].rtl_expr = t;
  wfe_bind_expr_stack [wfe_bind_expr_stack_last].block    = block;
} /* WFE_End_Stmt_Expr */
#else
#if defined TARG_ST && defined (_NO_WEAK_SUPPORT_)
void
WFE_Expand_Start_Stmt_Expr (tree t)
{
  abort();
}

void
WFE_Expand_End_Stmt_Expr (tree t)
{
  abort();
}
#endif
#endif /* GPLUSPLUS_FE */

typedef struct wfe_save_expr_t {
  tree  exp;
  ST   *st;
#ifdef KEY
  INT32  level;	// to identify which cleanup the save expr belongs to
#endif
} WFE_SAVE_EXPR;

WFE_SAVE_EXPR *wfe_save_expr_stack      = NULL;
INT32          wfe_save_expr_stack_last = -1;
INT32          wfe_save_expr_stack_max  = 0;

#ifdef KEY
INT32 wfe_save_expr_level;	// identify the current cleanup
INT32 wfe_last_save_expr_level;	// the last cleanup level used
#endif

static WN*
WFE_Save_Expr (tree save_exp,
               bool need_result,
               TY_IDX nop_ty_idx,
               TY_IDX component_ty_idx,
               INT64 component_offset,
               UINT16 field_id)
{
  INT32     i;
  tree      exp     = TREE_OPERAND (save_exp, 0);
  TY_IDX    ty_idx  = Get_TY (TREE_TYPE (exp));
  TYPE_ID   mtype   = TY_mtype (ty_idx);
  ST       *st;
  WN       *wn;
  bool     found = false;  

  for (i = wfe_save_expr_stack_last; i >= 0; i--) {
#ifndef KEY
    if (wfe_save_expr_stack [i].exp == exp) {
#else
    if (wfe_save_expr_stack [i].exp == save_exp &&
    	wfe_save_expr_stack [i].level == wfe_save_expr_level) {
#endif
      st = wfe_save_expr_stack [i].st;
      FmtAssert (st != 0,
                 ("WFE_Save_Expr: st not yet assigned"));
      found = true;
      break;
    }
  }
  
  if (!found) {
    i = ++wfe_save_expr_stack_last;
    if (i == wfe_save_expr_stack_max) {
      if (wfe_save_expr_stack == NULL) {
        wfe_save_expr_stack_max = 32;
        wfe_save_expr_stack     =
      // [HK]
#if __GNUC__ >= 3
          (WFE_SAVE_EXPR *) xmalloc (wfe_save_expr_stack_max *
#else
          (WFE_SAVE_EXPR *) malloc (wfe_save_expr_stack_max *
#endif // __GNUC__ >= 3
                                    sizeof (WFE_SAVE_EXPR));
      }
      else {
        wfe_save_expr_stack_max = wfe_save_expr_stack_max +
                                  (wfe_save_expr_stack_max >> 1);
        wfe_save_expr_stack     =
      // [HK]
#if __GNUC__ >= 3
          (WFE_SAVE_EXPR *) xrealloc (wfe_save_expr_stack,
#else
          (WFE_SAVE_EXPR *) realloc (wfe_save_expr_stack,
#endif // __GNUC__ >= 3
                                     wfe_save_expr_stack_max *
                                     sizeof (WFE_SAVE_EXPR));
      }
    }
#ifndef KEY
    wfe_save_expr_stack [i].exp = exp;
#else
    wfe_save_expr_stack [i].exp = save_exp;
    wfe_save_expr_stack [i].level = wfe_save_expr_level;
#endif
    wfe_save_expr_stack [i].st  = 0;
    wn = WFE_Expand_Expr (exp);
    st = Gen_Temp_Symbol (ty_idx, "__save_expr");
    WFE_Set_ST_Addr_Saved (wn);
    wn = WN_Stid (mtype, 0, st, ty_idx, wn);
    WFE_Stmt_Append (wn, Get_Srcpos());
    wfe_save_expr_stack [i].st = st;
  }

  if (component_ty_idx == 0)
    wn = WN_Ldid (mtype, 0, st, ty_idx);
  else {
    TYPE_ID desc  = TY_mtype(component_ty_idx);
    TYPE_ID rtype = Widen_Mtype(desc);
    wn = WN_CreateLdid(OPR_LDID, rtype, desc, component_offset, st,
		     field_id? ty_idx : component_ty_idx, field_id);  
  }
  return wn;
} /* WFE_Save_Expr */

/* process the tree doing array indicing and return the WN that performs
 * the address computation; ty_idx returns the high-level array type if it
 * is a DECL node, and the element type if it is an ARRAY_REF node.
 */
static WN *
WFE_Array_Expr(tree exp, 
	       TY_IDX *ty_idx, 
	       TY_IDX component_ty_idx,
	       INT64 component_offset,
	       UINT32 field_id)
{
  WN *wn;
  enum tree_code code = TREE_CODE (exp);

#ifdef WFE_DEBUG
  //  fprintf(stdout, "==================================================\n");
  //  fprintf(stdout, "  Array_Expr:\n");
  //  fprintf(stdout, "==================================================\n");
  //  print_tree (stdout, exp);
#endif

  if (code == COMPONENT_REF) {
    TY_IDX ty_idx0;
    tree arg0 = TREE_OPERAND(exp, 0); 
    tree arg1 = TREE_OPERAND(exp, 1); 
    if (component_ty_idx == 0)
      ty_idx0 = Get_TY(TREE_TYPE(exp));
    else ty_idx0 = component_ty_idx;
    Is_True(! DECL_BIT_FIELD(arg1),
	    ("WFE_Array_Expr: address arithmetic cannot use bitfield addresses"));
    INT64 ofst = (BITSPERBYTE * Get_Integer_Value(DECL_FIELD_OFFSET(arg1)) +
				Get_Integer_Value(DECL_FIELD_BIT_OFFSET(arg1)))
			      / BITSPERBYTE;
    wn = WFE_Array_Expr(arg0, ty_idx, ty_idx0, ofst + component_offset,
			  field_id + DECL_FIELD_ID(arg1));

#ifdef WFE_DEBUG
    //    fprintf(stdout, "\n>> COMPONENT_REF wn:\n");
    //    fdump_tree(stdout, wn);
    //    fprintf(stdout, "==================================================\n");
#endif

    return wn;
  }
  else if (code == VAR_DECL || code == PARM_DECL) {
    ST *st = Get_ST (exp);
    ST *base_st = ST_base (st);
    // for VLAs the instead of using the ST use its base st
    // also for the time being do not support VLAs within structs
#ifdef TARG_ST
    /* (cbr) VLAs are auto */
    if (st != base_st && ST_sclass (st) == SCLASS_AUTO) {
#else
    if (st != base_st) {
#endif
      FmtAssert (component_ty_idx == 0,
                 ("VLAs within struct not currently implemented"));
      wn = WN_Ldid (Pointer_Mtype, 0, base_st, ST_type (base_st));
    }
#if defined (TARG_ST) && (GNU_FRONT_END==33)
    /* (cbr) gcc 3.3 upgrade: non_pods are reference parameters.
       Marked with TREE_ADDRESSABLE */
    else if (code == PARM_DECL && TREE_ADDRESSABLE(TREE_TYPE(exp))) {
      wn = WN_Ldid (Pointer_Mtype, 0, st, ST_type(st));
    }
#endif
    else {
      wn = WN_Lda (Pointer_Mtype, ST_ofst(st)+component_offset, st, field_id);
    }
    if (component_ty_idx == 0)
      *ty_idx = ST_type(st);
    else {
      *ty_idx = component_ty_idx;
      if (TY_align(ST_type(st)) < TY_align(component_ty_idx))
	Set_TY_align(*ty_idx, TY_align(ST_type(st)));//pick more stringent align
    }

#ifdef TARG_ST
    /* (cbr) for anon unions */
    if (TY_kind(*ty_idx) == KIND_STRUCT) {
      FLD_ITER fld_iter = Make_fld_iter (TY_fld (*ty_idx));
      do {
	TY_IDX fld_ty = FLD_type (fld_iter);
        if (TY_kind(fld_ty) == KIND_ARRAY) {
          *ty_idx = fld_ty;
        }
      } while (!FLD_last_field (fld_iter++));
    }
#endif

    Is_True(TY_kind(*ty_idx) == KIND_ARRAY,
	    ("WFE_Array_Expr: ARRAY_REF base not of type KIND_ARRAY"));

#ifdef WFE_DEBUG
    //    fprintf(stdout, "\n>> component_ty_idx = %d\n", component_ty_idx);
    //    fprintf(stdout, "\n>> VAR_DECL/PARM_DECL wn:\n");
    //    fdump_tree(stdout, wn);
    //    fprintf(stdout, "==================================================\n");
#endif

    return wn;
  }
  else if (code == CONSTRUCTOR) {
    ST *st = WFE_Generate_Temp_For_Initialized_Aggregate (exp, "");
    wn = WN_Lda (Pointer_Mtype, ST_ofst(st)+component_offset, st, field_id);
    if (component_ty_idx == 0)
      *ty_idx = ST_type(st);
    else *ty_idx = component_ty_idx;
    Is_True(TY_kind(*ty_idx) == KIND_ARRAY,
	    ("WFE_Array_Expr: ARRAY_REF base not of type KIND_ARRAY"));
    return wn;
  }
  else if (code == STRING_CST) {
    wn = WFE_Expand_Expr(exp);
    *ty_idx = ST_type (TREE_STRING_ST (exp));
    return wn;
  }
  else if (code == INDIRECT_REF) {
    wn = WFE_Expand_Expr(TREE_OPERAND (exp, 0));
    if (component_ty_idx == 0)
      *ty_idx = Get_TY (TREE_TYPE(exp));
    else {
      *ty_idx = component_ty_idx;
      INT node_align = TYPE_ALIGN(TREE_TYPE(exp)) / BITSPERBYTE;
      if (node_align < TY_align(component_ty_idx))
	Set_TY_align(*ty_idx, node_align);//pick more stringent align
    }
    if (component_offset != 0) { // TODO: use ILDA instead
      WN *wn0 = WN_Intconst(MTYPE_I4, component_offset);
      wn = WN_Binary(OPR_ADD, Pointer_Mtype, wn, wn0);
    }
    return wn;
  }
  else if (code == CALL_EXPR) {
    wn = WFE_Expand_Expr(exp);
    FmtAssert (WN_opcode (wn) == OPC_MCOMMA,
               ("opcode other than OPC_MCOMMA for call underneath ARRAY_REF"));
    WFE_Stmt_Append (WN_kid0 (wn), Get_Srcpos ());
    ST *st = WN_st (WN_kid1 (wn));
    WN_Delete (WN_kid1 (wn));
    WN_Delete (wn);
    wn = WN_Lda (Pointer_Mtype, ST_ofst(st)+component_offset, st, field_id);
    if (component_ty_idx == 0)
      *ty_idx = ST_type(st);
    else {
      *ty_idx = component_ty_idx;
      if (TY_align(ST_type(st)) < TY_align(component_ty_idx))
	Set_TY_align(*ty_idx, TY_align(ST_type(st)));//pick more stringent align
    }
    Is_True(TY_kind(*ty_idx) == KIND_ARRAY,
	    ("WFE_Array_Expr: ARRAY_REF base not of type KIND_ARRAY"));
    return wn;
  }
  else if (code == ARRAY_REF) { // recursive call
    WN *wn0, *wn1, *wn2;
    TY_IDX ty_idx0;

#ifdef TARG_ST
    // Since we're now capable of making multi-dimentional arrays
    //
    // Get to the bottom of the sequence of array_refs.
    // For a reference to int x[i][j], the gcc makes something 
    // like so:
    //
    //       array_ref  --> integer_type
    //        /     \                 ^
    //       |       |                |
    //       V       V                |
    //  var_decl j  array_ref  --> array_type
    //               /     \               ^
    //              |       |              |
    //              V       V              |
    //         var_decl i  var_decl x --> array_type
    //
    // We start at the topmost of array_ref and recurse to
    // the var_decl x. At this point the TREE_CODE(exp) is
    // VAR_DECL, so it calls Get_ST and eventually Get_TY.
    // Get_TY () (see tree_symtab.h) will recurse down the
    // chain of types towards the integer_type.
    // In present implementation, only certain types are
    // converted into multidimentional arrays (see the
    // switch in mk_array_dimention () in tree_symtab.cxx).
    // The types here must correspond to that switch.
    //
    // TODO: eventually, perhaps everything will be converted
    //       to multi-dimentional arrays and the following
    //       test will be suppressed.
    {
      tree base_expr;

      base_expr = TREE_OPERAND (exp, 0);
      while (TREE_CODE(base_expr) == ARRAY_REF) {
	base_expr = TREE_OPERAND (base_expr, 0);
      }

      //
      // Arthur: seems that we should not pass the component_ty_idx to
      //         the next level of ARRAY, but need to pass offset and
      //         the field_id. The field_id and the component_offset
      //         keep track of the fields in nested structs. They are
      //         necessary for calculating the right offset for LDA,
      //         and ILOAD operators. The component_ty_idx should
      //         keep the type of the first non array_ref tree node
      //         only until next array_ref tree node (used to live
      //         through the entire chain).
      //
      wn0 = WFE_Array_Expr(base_expr, &ty_idx0, 0,
			                 component_offset, field_id);

      ARB_HANDLE arb = TY_arb(ty_idx0);
      OPCODE op_array = OPCODE_make_op(OPR_ARRAY, Pointer_type, MTYPE_V);
      wn = WN_Create (op_array, ARB_dimension(arb)*2+1);
      // single array elemet size
      WN_element_size(wn) = TY_size(TY_etype(ty_idx0));
      // array base
      WN_array_base(wn) = wn0;

#ifdef WFE_DEBUG
      //      fprintf(stdout, "  array dimention %d\n", ARB_dimension(arb));
      //      fprintf(stdout, "  array elt size %lld\n", WN_element_size(wn));
      //      fprintf(stdout, "  array base WN: \n");
      //      fdump_tree(stdout, wn0);
#endif

      // Process dimentions:
      // GNU's index tree for the current dimention
      base_expr = exp;
      for (UINT i = 0; i < ARB_dimension(arb); i++) {

#ifdef WFE_DEBUG
	//	fprintf(stdout, "  base expr [%d]: \n", i);
	//	print_tree (stdout, base_expr);
#endif

	// I do not even know why ??
	Is_True(ARB_const_lbnd(arb[i]),
		 ("WFE_Array_Expr: non-const lower bound dim %d", i));

	// kid (i+1) is the size of dimention i:
	if (ARB_const_ubnd(arb[i]))
	  wn1 = WN_Intconst(MTYPE_I4, 
                     ARB_ubnd_val(arb[i]) - ARB_lbnd_val(arb[i]) + 1);
	else {
	  // [SC]: Set size for VLA
	  ST_IDX ubnd_st_idx = ARB_ubnd_var (arb[i]);
	  if (ubnd_st_idx != ST_IDX_ZERO) {
	    ST *ubnd_var = ST_ptr (ARB_ubnd_var (arb[i]));
	    TY_IDX ubnd_ty_idx = ST_type(ubnd_var);
	    TYPE_ID ubnd_mtype = TY_mtype (ubnd_ty_idx);
	    wn1 = WN_Add (Widen_Mtype(ubnd_mtype),
			  WN_Ldid (ubnd_mtype, 0, ubnd_var, ubnd_ty_idx),
			  WN_Intconst (Widen_Mtype(ubnd_mtype), 1));
	  }
	  else {
	    FmtAssert (ARB_last_dimen (arb[i]),
		      ("Upper bounds may be omitted only on last dimension"));
	    wn1 = WN_Intconst (MTYPE_I4, 0);
	  }
	}
	WN_array_dim(wn, ARB_dimension(arb)-i-1) = wn1;

#ifdef WFE_DEBUG
	//	fprintf(stdout, "  WN_array_dim[%d]:\n", i);
	//	fdump_tree(stdout, wn1);
	//	fflush(stdout);
#endif

	// kid (n+i+1) is the index expression for the dimention i
	wn2 = WFE_Expand_Expr (TREE_OPERAND(base_expr,1));
	WN_array_index(wn, ARB_dimension(arb)-i-1) = wn2;

#ifdef WFE_DEBUG
	//	fprintf(stdout, "  WN_array_index[%d]:\n", i);
	//	fdump_tree(stdout, wn2);
#endif

	base_expr = TREE_OPERAND (base_expr, 0);
      }
    } 

#else /* !TARG_ST */

    wn0 = WFE_Array_Expr(TREE_OPERAND (exp, 0), &ty_idx0, component_ty_idx,
			 component_offset, field_id);
    Is_True(TY_kind(ty_idx0) == KIND_ARRAY,
	    ("WFE_Array_Expr: arg 0 of ARRAY_REF not of type KIND_ARRAY"));
    ARB_HANDLE arb = TY_arb(ty_idx0);
    if (ARB_dimension(arb) == 1 && 
	ARB_first_dimen(arb) && ARB_last_dimen(arb) &&
	ARB_const_lbnd(arb)) {
      if (ARB_const_ubnd(arb))
        wn1 = WN_Intconst(MTYPE_I4, ARB_ubnd_val(arb) - ARB_lbnd_val(arb) + 1);
      else
        wn1 = WN_Intconst(MTYPE_I4, 0);
      wn2 = WFE_Expand_Expr (TREE_OPERAND (exp, 1));
      wn = WN_Ternary(OPR_ARRAY, Pointer_Mtype, wn0, wn1, wn2);
      WN_element_size(wn) = TY_size(Get_TY (TREE_TYPE(exp)));
    }
    else Is_True(FALSE,
		 ("WFE_Array_Expr: only const-bounds 1-dimension arrays handled now"));
#endif /* TARG_ST */

    if (component_ty_idx == 0) {
      *ty_idx = TY_etype(ty_idx0);
      if (TY_align(ty_idx0) < TY_align(*ty_idx))
	Set_TY_align(*ty_idx, TY_align(ty_idx0));// pick more stringent align
    }
    else *ty_idx = component_ty_idx;
    return wn;
  }


#ifdef TARG_ST
  else if (code == COMPOUND_EXPR) {
    // Contains two expressions to compute, one followed by the other.
    // the first value is ignored.  The second one's value is used.  The
    // type of the first expression need not agree with the other types.
    tree arg0 = TREE_OPERAND(exp, 0); 
    tree arg1 = TREE_OPERAND(exp, 1); 

    // Arthur: so we do not care for the first kid ...
    wn = WFE_Expand_Expr (arg0, FALSE);
    if (wn) {
      wn = WFE_Append_Expr_Stmt (wn);
#ifdef WFE_DEBUG
      fdump_tree(stdout, wn);
#endif
    }

    // Get the result of the second kid
    wn = WFE_Array_Expr(arg1, ty_idx, component_ty_idx, component_offset, field_id);
#ifdef WFE_DEBUG
    fdump_tree(stdout, wn);
    fprintf(stdout, "==================================================\n");
#endif

    return wn;
  }

  // (cbr) assignment expression
  else if (code == MODIFY_EXPR) {
    tree assign = TREE_OPERAND(exp, 0); 
    tree val = TREE_OPERAND(exp, 1); 

    WN *wn1 = WFE_Expand_Expr (val);
    (void) WFE_Lhs_Of_Modify_Expr(code, TREE_OPERAND (exp, 0), FALSE,
                                  0, 0, 0, FALSE, wn1, 0, FALSE, FALSE);

    wn = WFE_Array_Expr(assign, ty_idx, component_ty_idx, component_offset, field_id);

#ifdef WFE_DEBUG
    fdump_tree(stdout, wn1);
    fdump_tree(stdout, wn);
#endif

    return wn;
  }

    /* (cbr) target_expr */
  else if (code == TARGET_EXPR) {
    wn = WFE_Expand_Expr(TREE_OPERAND(exp, 1));
    wn = WFE_Append_Expr_Stmt (wn);
#ifdef WFE_DEBUG
      fdump_tree(stdout, wn);
#endif
    wn = WFE_Array_Expr(TREE_OPERAND(exp, 0), ty_idx, component_ty_idx,
                        component_offset, field_id);
    return wn;
  }

    /* (cbr) cond_expr */
  else if (code == COND_EXPR) {
    WN *wn0, *wn1, *wn2;

    tree op0 = TREE_OPERAND (exp, 0);
    tree op1 = TREE_OPERAND (exp, 1);
    tree op2 = TREE_OPERAND (exp, 2);

    enum tree_code code1 = TREE_CODE (op1);
    enum tree_code code2 = TREE_CODE (op2);

    wn0 = WFE_Expand_Expr (TREE_OPERAND(op0, 0)); 

    if (code1 == COMPONENT_REF && code2 == COMPONENT_REF) {
      extern WN*  WFE_Address_Of(tree);

      wn1 = WFE_Address_Of(TREE_OPERAND(op1, 0));
      wn2 = WFE_Address_Of(TREE_OPERAND(op2, 0));

      wn  = WN_Cselect (Pointer_Mtype, wn0, wn1, wn2);

      if (component_ty_idx == 0)
        *ty_idx = Get_TY (TREE_TYPE(exp));
      else 
        *ty_idx = component_ty_idx;
    return wn;
    }

    Is_True(FALSE,
	    ("WFE_Array_Expr: unsupported node for base of COND_EXPR"));
    return NULL;
  } else if (code == VA_ARG_EXPR) {
    // [SC] va_arg.
    // Assign to temporary, then act on temporary.
    // TREE_TYPE (exp) is the type of the va_arg;
    // TREE_OPERAND (exp, 0) is the va_list.
    tree type = TREE_TYPE (exp);
    TY_IDX va_ty_idx = Get_TY (type);
    TYPE_ID va_mtype = TY_mtype (va_ty_idx);
    ST *temp_st = Gen_Temp_Symbol (va_ty_idx, ".tmp");
    Set_ST_addr_saved (temp_st);
    wn = WFE_Expand_Expr (exp);
    wn = WN_Stid (va_mtype, 0, temp_st, va_ty_idx, wn);
    WFE_Stmt_Append (wn, Get_Srcpos ());
    wn = WN_Lda (Pointer_Mtype, ST_ofst(temp_st)+component_offset,
		 temp_st, field_id);
    if (component_ty_idx == 0)
      *ty_idx = ST_type(temp_st);
    else {
      *ty_idx = component_ty_idx;
      if (TY_align(ST_type(temp_st)) < TY_align(component_ty_idx))
	Set_TY_align(*ty_idx, TY_align(ST_type(temp_st)));//pick more stringent align
    }
    Is_True(TY_kind(*ty_idx) == KIND_ARRAY,
	    ("WFE_Array_Expr: VA_ARG_EXPR not of type KIND_ARRAY"));
    return wn;
  }

#endif /* TARG_ST */

  else {
    Is_True(FALSE,
	    ("WFE_Array_Expr: unsupported node for base of ARRAY_REF"));
    return NULL;
  }
}


/* rhs_wn is the WN representing the rhs of a MODIFY_EXPR node; this
 * routine processes the lhs of the node and generate the appropriate
 * form of store.
 *
 * In special cases where the RHS of the store is unknown but the
 * statement being expanded is nonetheless semantically a store,
 * rhs_wn can be NULL. This happens, for example, for each output
 * operand of an asm statement. When rhs_wn is NULL, we manufacture an
 * RHS that is an LDID of a PREG specified by rhs_preg_num (generally
 * a negative-numbered PREG). If rhs_st is non-NULL, rhs_preg_num is
 * ignored.  assign_code tells if it is {PRE,POST}{IN,DE}CREMENT_EXPR.
 * Ordinarily, it is MODIFY_EXPR.
 */
WN *
WFE_Lhs_Of_Modify_Expr(tree_code assign_code,
		       tree lhs, 
		       bool need_result,
		       TY_IDX component_ty_idx, 
		       INT64 component_offset,
		       UINT32 field_id,
		       bool is_bit_field,
		       WN *rhs_wn,
		       PREG_NUM rhs_preg_num,
		       bool is_realpart,
		       bool is_imagpart)
{
  WN *wn;
  ST *st;
  bool result_in_temp = FALSE;
  ST *result_preg_st;
  PREG_NUM result_preg;
  PREG_NUM lhs_preg_num = 0;
  enum tree_code code = TREE_CODE (lhs);
  BOOL volt = FALSE;
  if (rhs_wn != NULL) {
    WFE_Set_ST_Addr_Saved (rhs_wn);
  }
  if (code == COMPONENT_REF) {
    INT64 ofst;
    TY_IDX ty_idx0;

    tree arg0 = TREE_OPERAND(lhs, 0);
    tree arg1 = TREE_OPERAND(lhs, 1);
#ifdef GPLUSPLUS_FE
    // for g++ ensure that the WHIRL type for the enclosing structure has been
    // created in order to set the field id to a non zero value
    (void) Get_TY (TREE_TYPE (arg0));
#endif /* GPLUSPLUS_FE */
    if (component_ty_idx == 0)
      ty_idx0 = Get_TY(TREE_TYPE(lhs));
    else ty_idx0 = component_ty_idx;
    if (DECL_BIT_FIELD(arg1)) 
      is_bit_field = TRUE;
#ifdef TARG_ST
    /* (cbr) propagate to struct fields */
    if (TREE_THIS_VOLATILE(lhs))
      Set_TY_is_volatile(ty_idx0);
#endif
    if (! is_bit_field)
      ofst = (BITSPERBYTE * Get_Integer_Value(DECL_FIELD_OFFSET(arg1)) +
			    Get_Integer_Value(DECL_FIELD_BIT_OFFSET(arg1)))
			  / BITSPERBYTE;
    else ofst = 0;
    wn = WFE_Lhs_Of_Modify_Expr(assign_code, arg0, need_result, ty_idx0, 
				ofst+component_offset,
			        field_id + DECL_FIELD_ID(arg1), is_bit_field, 
				rhs_wn, rhs_preg_num, is_realpart, is_imagpart);
    return wn;
  }

  if (code == REALPART_EXPR) {
    tree arg0 = TREE_OPERAND(lhs, 0);
    TY_IDX ty_idx0 = Get_TY(TREE_TYPE(arg0));
    wn = WFE_Lhs_Of_Modify_Expr(assign_code, arg0, need_result, ty_idx0,
				component_offset, field_id, is_bit_field,
				rhs_wn, rhs_preg_num, TRUE, FALSE);
    return wn;
  }

  if (code == IMAGPART_EXPR) {
    tree arg0 = TREE_OPERAND(lhs, 0);
    TY_IDX ty_idx0 = Get_TY(TREE_TYPE(arg0));
    wn = WFE_Lhs_Of_Modify_Expr(assign_code, arg0, need_result, ty_idx0,
				component_offset, field_id, is_bit_field,
				rhs_wn, rhs_preg_num, FALSE, TRUE);
    return wn;
  }

  if (code == PARM_DECL || code == VAR_DECL || code == RESULT_DECL) {
    TY_IDX hi_ty_idx = Get_TY(TREE_TYPE(lhs)); // type associated with field_id
#ifdef TARG_ST
    /* (cbr) propagate to struct fields */
    if (TREE_THIS_VOLATILE(lhs) || TY_is_volatile(component_ty_idx)) {
#else
    if (TREE_THIS_VOLATILE(lhs)) {
#endif
      Set_TY_is_volatile(hi_ty_idx);
      volt = TRUE;
    }
    TY_IDX desc_ty_idx = component_ty_idx;
    if (desc_ty_idx == 0)
      desc_ty_idx = hi_ty_idx;
    if (TY_is_volatile(desc_ty_idx)) {
      Clear_TY_is_volatile(desc_ty_idx);
      volt = TRUE;
    }
    st = Get_ST (lhs);
    if (ST_assigned_to_dedicated_preg (st)) {
      Set_TY_is_volatile(hi_ty_idx);
      volt = TRUE;
    }
    Is_True(! is_bit_field || field_id <= MAX_FIELD_ID,
	    ("WFE_Lhs_Of_Modify_Expr: field id for bit-field exceeds limit"));

    TYPE_ID rtype = Widen_Mtype(TY_mtype(desc_ty_idx));
    TYPE_ID desc = is_bit_field ? MTYPE_BS : TY_mtype(desc_ty_idx);

    if (rhs_wn == NULL) {
      // Manufacture a negative-PREG RHS for the STID we are about to
      // generate. This feature is used in preparing WHIRL ASM
      // statements.
      // TODO: How to support a bit-field output of non-integral
      // number of bytes?
      if (rtype == MTYPE_M && desc == MTYPE_M) {
        FmtAssert(TY_size(desc_ty_idx) == MTYPE_byte_size(Def_Int_Mtype),
                  ("Size of ASM struct opnd must be equal to register size"));
        desc = rtype = Def_Int_Mtype;
        desc_ty_idx = hi_ty_idx = MTYPE_To_TY(Def_Int_Mtype);
      }
      ST *rhs_st = MTYPE_To_PREG(desc);
      rhs_wn = WN_CreateLdid (OPR_LDID, rtype,
			      desc, rhs_preg_num, rhs_st,
			      desc_ty_idx, 0);
    }
    else {
      WN *result_wn;	// the result wn to be returned
      if (assign_code == MODIFY_EXPR) {
	if (is_realpart)
	  rhs_wn = WN_Binary(OPR_COMPLEX, rtype,
			     rhs_wn,
			     WN_Unary(OPR_IMAGPART,
				      Mtype_complex_to_real (rtype),
				      WN_CreateLdid(OPR_LDID, rtype, desc,
						    ST_ofst(st) + component_offset,
						    st, hi_ty_idx, field_id)));
	else
	if (is_imagpart)
	  rhs_wn = WN_Binary(OPR_COMPLEX, rtype,
			     WN_Unary(OPR_REALPART,
				      Mtype_complex_to_real (rtype),
				      WN_CreateLdid(OPR_LDID, rtype, desc,
						    ST_ofst(st) + component_offset,
						    st, hi_ty_idx, field_id)),
			     rhs_wn);
      }
      else {
	if (is_realpart)
	  rhs_wn = WN_Binary(OPR_COMPLEX, rtype,
			     rhs_wn,
			     WN_Floatconst (Mtype_complex_to_real (rtype), 0.0));
	else
	if (is_imagpart)
	  rhs_wn = WN_Binary(OPR_COMPLEX, rtype,
			     WN_Floatconst (Mtype_complex_to_real (rtype), 0.0),
			     rhs_wn);
      }

      if (assign_code == PREINCREMENT_EXPR ||
	  assign_code == PREDECREMENT_EXPR) {
        wn = WN_CreateLdid (OPR_LDID, rtype, desc, 
			    ST_ofst(st) + component_offset,
			    st, hi_ty_idx, field_id);
        rhs_wn = WN_Binary(Operator_From_Tree [assign_code].opr,
		           rtype, wn, rhs_wn);
	result_wn = rhs_wn;
      }
      else if (assign_code == POSTINCREMENT_EXPR ||
	       assign_code == POSTDECREMENT_EXPR) {
        result_wn = WN_CreateLdid (OPR_LDID, rtype, desc, 
				   ST_ofst(st) + component_offset,
				   st, hi_ty_idx, field_id);
      }
      else result_wn = rhs_wn;

      if (need_result && 
	  (volt ||
	   assign_code == POSTINCREMENT_EXPR ||
	   assign_code == POSTDECREMENT_EXPR)) { // save result in a preg
        result_in_temp = TRUE;
#ifdef TARG_ST
	/* [FdF] Create a Symbol instead of a PREG, so that we can propagate the
	   restrict or volatile property. */
	/* [CG]: I don't understand why we need to propagate this, Ask FdF. */
	/* [CG]: Another mmotivation for this is that in the case of a volatile assignment, 
	   there may be assignment of structures that can't be put into a preg. Thus we
	   must use a temporary with the corresponding type. Ref to bug 1-5-0-B/ddts/18793. */
	TY_IDX tmp_type = rtype == MTYPE_M ? desc_ty_idx: MTYPE_To_TY(rtype);
	result_preg_st = Gen_Temp_Symbol(tmp_type, ".tmp");
	result_preg = 0;
#else
        result_preg_st = MTYPE_To_PREG(rtype);
        result_preg = Create_Preg(rtype, NULL);
#endif
        wn = WN_Stid(rtype, result_preg, result_preg_st, desc_ty_idx, result_wn,
		     0);
        WFE_Stmt_Append (wn, Get_Srcpos());
        result_wn = WN_Ldid(rtype, result_preg, result_preg_st, desc_ty_idx, 0);
      }

      if (assign_code == POSTINCREMENT_EXPR ||
	  assign_code == POSTDECREMENT_EXPR) {
        rhs_wn = WN_Binary(Operator_From_Tree [assign_code].opr,
		           rtype, result_wn, rhs_wn);
      }
      else rhs_wn = result_wn;

      // rhs_wn is now always the right-hand-side of the assignment
    }

    // the assignment
    if (!WFE_Keep_Zero_Length_Structs &&
        desc == MTYPE_M               &&
        TY_size (hi_ty_idx) == 0) {
      // ignore zero length structs
    }
    else {
#if defined (TARG_ST) && (GNU_FRONT_END==33)
      /* (cbr) gcc 3.3 upgrade: non_pods are reference parameters.
         Marked with TREE_ADDRESSABLE */
      if (TREE_CODE (lhs) == PARM_DECL && TREE_ADDRESSABLE(TREE_TYPE(lhs))) {
        WN *w0 = WN_Ldid (Pointer_Mtype, 0, st, ST_type(st));
        wn = WN_CreateIstore(OPR_ISTORE, MTYPE_V, desc, component_offset, 
                             ST_type(st), rhs_wn, w0, field_id);
      }
      else
#endif
      wn = WN_Stid (desc, ST_ofst(st) + component_offset + lhs_preg_num, st,
		    hi_ty_idx, rhs_wn, field_id);
      WFE_Stmt_Append(wn, Get_Srcpos());
    }
    if (need_result) {
      if (! result_in_temp)
        wn = WN_CreateLdid(OPR_LDID, rtype, desc, 
			   ST_ofst(st) + component_offset, st, hi_ty_idx,
			   field_id);
      else wn = WN_Ldid(rtype, result_preg, result_preg_st, desc_ty_idx, 0);
      if (is_realpart)
	wn = WN_Unary (OPR_REALPART, Mtype_complex_to_real (rtype), wn);
      else
      if (is_imagpart)
	wn = WN_Unary (OPR_IMAGPART, Mtype_complex_to_real (rtype), wn);
    }
    else wn = NULL;
  }
  else if (code == INDIRECT_REF) {
    TY_IDX hi_ty_idx = Get_TY(TREE_TYPE(lhs));
    if (TREE_THIS_VOLATILE(lhs)) {
      Set_TY_is_volatile(hi_ty_idx);
      volt = TRUE;
    }
    tree op = TREE_OPERAND(lhs, 0);
    WN *addr_wn = WFE_Expand_Expr (TREE_OPERAND (lhs, 0));
    TY_IDX desc_ty_idx = component_ty_idx;
    if (desc_ty_idx == 0)
      desc_ty_idx = hi_ty_idx;
    if (TY_is_volatile(desc_ty_idx)) {
      Clear_TY_is_volatile(desc_ty_idx);
      volt = TRUE;
    }
    Is_True(! is_bit_field || field_id <= MAX_FIELD_ID,
	    ("WFE_Lhs_Of_Modify_Expr: field id for bit-field exceeds limit"));
    if (WN_has_side_effects(addr_wn) &&
	(need_result || 
	 assign_code == PREINCREMENT_EXPR ||
	 assign_code == PREDECREMENT_EXPR ||
	 assign_code == POSTINCREMENT_EXPR ||
	 assign_code == POSTDECREMENT_EXPR)) {
      ST       *preg_st;
      PREG_NUM  preg;
      TY_IDX    address_ty_idx = Get_TY (TREE_TYPE (TREE_OPERAND (lhs, 0)));
      preg_st = MTYPE_To_PREG(Pointer_Mtype);
      preg    = Create_Preg (Pointer_Mtype, NULL);
      wn      = WN_Stid (Pointer_Mtype, preg, preg_st, address_ty_idx, addr_wn);
      WFE_Set_ST_Addr_Saved (addr_wn);
      WFE_Stmt_Append (wn, Get_Srcpos());
      addr_wn = WN_Ldid (Pointer_Mtype, preg, preg_st, address_ty_idx);
    }

    TYPE_ID rtype = Widen_Mtype(TY_mtype(desc_ty_idx));
    TYPE_ID desc = is_bit_field ? MTYPE_BS : TY_mtype(desc_ty_idx);

    if (rhs_wn == NULL) {
      // Manufacture a negative-PREG RHS for the ISTORE we are about to
      // generate. This feature is used in preparing WHIRL ASM
      // statements.
      ST *rhs_st;
      // TODO: How to support a bit-field output of non-integral
      // number of bytes?
      rhs_st = MTYPE_To_PREG(desc);
      // Types are likely to be wrong in the following
      rhs_wn = WN_CreateLdid (OPR_LDID, rtype, desc, rhs_preg_num, rhs_st,
			      desc_ty_idx, 0);
    }
    else {
      WN *result_wn;	// the result wn to be returned

      if (assign_code == MODIFY_EXPR) {
	if (is_realpart)
	  rhs_wn = WN_Binary(OPR_COMPLEX, rtype,
			     rhs_wn,
			     WN_Unary(OPR_IMAGPART,
				      Mtype_complex_to_real (rtype),
				      WN_CreateIload(OPR_ILOAD, rtype, desc,
						     component_offset,
						     field_id != 0 ? hi_ty_idx : desc_ty_idx,
						     Make_Pointer_Type(hi_ty_idx, FALSE),
						     WN_COPY_Tree (addr_wn),
						     field_id)));
	else
	if (is_imagpart)
	  rhs_wn = WN_Binary(OPR_COMPLEX, rtype,
			     WN_Unary(OPR_REALPART,
				      Mtype_complex_to_real (rtype),
				      WN_CreateIload(OPR_ILOAD, rtype, desc,
						     component_offset,
						     field_id != 0 ? hi_ty_idx : desc_ty_idx,
						     Make_Pointer_Type(hi_ty_idx, FALSE),
						     WN_COPY_Tree (addr_wn),
						     field_id)),
			     rhs_wn);
      }
      else {
	if (is_realpart)
	  rhs_wn = WN_Binary(OPR_COMPLEX, rtype,
			     rhs_wn,
			     WN_Floatconst (Mtype_complex_to_real (rtype), 0.0));
	else
	if (is_imagpart)
	  rhs_wn = WN_Binary(OPR_COMPLEX, rtype,
			     WN_Floatconst (Mtype_complex_to_real (rtype), 0.0),
			     rhs_wn);
      }

      if (assign_code == PREINCREMENT_EXPR ||
	  assign_code == PREDECREMENT_EXPR) {
        wn = WN_CreateIload (OPR_ILOAD, rtype, desc, component_offset,
			     field_id != 0 ? hi_ty_idx : desc_ty_idx,
			     Make_Pointer_Type(hi_ty_idx, FALSE),
			     WN_COPY_Tree (addr_wn),
			     field_id);
        rhs_wn = WN_Binary(Operator_From_Tree [assign_code].opr,
                           rtype, wn, rhs_wn);
        result_wn = rhs_wn;
      }
      else if (assign_code == POSTINCREMENT_EXPR ||
	       assign_code == POSTDECREMENT_EXPR) {
	result_wn = WN_CreateIload (OPR_ILOAD, rtype, desc, component_offset,
				    field_id != 0 ? hi_ty_idx : desc_ty_idx,
				    Make_Pointer_Type(hi_ty_idx, FALSE),
				    WN_COPY_Tree (addr_wn),
				    field_id);
      }
      else result_wn = rhs_wn;

      if (need_result && 
	  (volt ||
           assign_code == POSTINCREMENT_EXPR ||
           assign_code == POSTDECREMENT_EXPR)) { // save result in a preg
	result_in_temp = TRUE;
#ifdef TARG_ST
	// [CG]: Fix 1-5-0-B/ddts/18793. See above.
	TY_IDX tmp_type = rtype == MTYPE_M ? desc_ty_idx: MTYPE_To_TY(rtype);
	result_preg_st = Gen_Temp_Symbol(tmp_type, ".tmp");
	result_preg = 0;
#else
        result_preg_st = MTYPE_To_PREG(rtype);
        result_preg = Create_Preg(rtype, NULL);
#endif
        wn = WN_Stid(rtype, result_preg, result_preg_st, desc_ty_idx, result_wn,
		     0);
        WFE_Stmt_Append (wn, Get_Srcpos());;
        result_wn = WN_Ldid(rtype, result_preg, result_preg_st, desc_ty_idx, 0);
      }

      if (assign_code == POSTINCREMENT_EXPR ||
	  assign_code == POSTDECREMENT_EXPR) {
        rhs_wn = WN_Binary(Operator_From_Tree [assign_code].opr,
                           rtype, result_wn, rhs_wn);
      }
      else rhs_wn = result_wn;

      // rhs_wn is now always the right-hand-side of the assignment
    }

    // the assignment
    if (!WFE_Keep_Zero_Length_Structs &&
        desc == MTYPE_M               &&
        TY_size (hi_ty_idx) == 0) {
      // ignore zero length structs
      if (WN_has_side_effects (addr_wn)) {
#ifdef TARG_ST
	wn = WFE_Append_Expr_Stmt (addr_wn);
#else
        wn = WN_CreateEval (addr_wn);
        WFE_Stmt_Append (wn, Get_Srcpos());
#endif
      }
      wn = NULL;
    }
    else {
      wn = WN_CreateIstore(OPR_ISTORE, MTYPE_V, desc, component_offset, 
			   Make_Pointer_Type (hi_ty_idx, FALSE),
			   rhs_wn, addr_wn, field_id);
      WFE_Stmt_Append(wn, Get_Srcpos());
      if (need_result) {
	if (! result_in_temp)
          wn = WN_CreateIload(OPR_ILOAD, rtype, desc, component_offset,
			      field_id != 0 ? hi_ty_idx : desc_ty_idx,
			      Make_Pointer_Type (hi_ty_idx, FALSE),
			      WN_COPY_Tree (addr_wn),
			      field_id);
	else wn = WN_Ldid(rtype, result_preg, result_preg_st, desc_ty_idx, 0);
	if (is_realpart)
	  wn = WN_Unary (OPR_REALPART, Mtype_complex_to_real (rtype), wn);
	else
	if (is_imagpart)
	  wn = WN_Unary (OPR_IMAGPART, Mtype_complex_to_real (rtype), wn);
      }
      else wn = NULL;
    }
  }
  else if (code == ARRAY_REF) {
    TY_IDX elem_ty_idx;
    // generate the WHIRL array node
    WN *addr_wn = WFE_Array_Expr(lhs, &elem_ty_idx, 0, 0, 0);
    if (TY_is_volatile(elem_ty_idx))
      volt = TRUE;
    TY_IDX desc_ty_idx = component_ty_idx;
    if (desc_ty_idx == 0)
      desc_ty_idx = Get_TY (TREE_TYPE(lhs));
    if (TY_is_volatile(desc_ty_idx)) {
      Clear_TY_is_volatile(desc_ty_idx);
      volt = TRUE;
    }
    Is_True(! is_bit_field || field_id <= MAX_FIELD_ID,
	    ("WFE_Lhs_Of_Modify_Expr: field id for bit-field exceeds limit"));
    if (WN_has_side_effects(addr_wn) &&
        (need_result ||
         assign_code == PREINCREMENT_EXPR ||
         assign_code == PREDECREMENT_EXPR ||
         assign_code == POSTINCREMENT_EXPR ||
	 assign_code == POSTDECREMENT_EXPR)) {
      ST       *preg_st;
      PREG_NUM  preg;
      TY_IDX    address_ty_idx = Make_Pointer_Type(elem_ty_idx, FALSE);
      preg_st = MTYPE_To_PREG(Pointer_Mtype);
      preg    = Create_Preg (Pointer_Mtype, NULL);
      wn      = WN_Stid (Pointer_Mtype, preg, preg_st, address_ty_idx, addr_wn);
      WFE_Set_ST_Addr_Saved (addr_wn);
      WFE_Stmt_Append (wn, Get_Srcpos());
      addr_wn = WN_Ldid (Pointer_Mtype, preg, preg_st, address_ty_idx);
    }

    TYPE_ID rtype = Widen_Mtype(TY_mtype(desc_ty_idx));
    TYPE_ID desc = is_bit_field ? MTYPE_BS : TY_mtype(desc_ty_idx);

    if (rhs_wn == NULL) {
      // Manufacture a negative-PREG RHS for the ISTORE we are about to
      // generate. This feature is used in preparing WHIRL ASM
      // statements.
      ST *rhs_st;
      // TODO: How to support a bit-field output of non-integral
      // number of bytes?
      rhs_st = MTYPE_To_PREG(desc);
      rhs_wn = WN_CreateLdid (OPR_LDID, rtype, desc, rhs_preg_num, rhs_st,
			      desc_ty_idx, 0);
    }
    else {
      WN *result_wn;    // the result wn to be returned

      if (assign_code == MODIFY_EXPR) {
	if (is_realpart)
	  rhs_wn = WN_Binary(OPR_COMPLEX, rtype,
			     rhs_wn,
			     WN_Unary(OPR_IMAGPART,
				      Mtype_complex_to_real (rtype),
				      WN_CreateIload(OPR_ILOAD, rtype, desc,
						     component_offset,
						     field_id != 0 ? elem_ty_idx : desc_ty_idx,
						     Make_Pointer_Type(elem_ty_idx, FALSE),
						     WN_COPY_Tree (addr_wn),
						     field_id)));
	else
	if (is_imagpart)
	  rhs_wn = WN_Binary(OPR_COMPLEX, rtype,
			     WN_Unary(OPR_REALPART,
				      Mtype_complex_to_real (rtype),
				      WN_CreateIload(OPR_ILOAD, rtype, desc,
						     component_offset,
						     field_id != 0 ? elem_ty_idx : desc_ty_idx,
						     Make_Pointer_Type(elem_ty_idx, FALSE),
						     WN_COPY_Tree (addr_wn),
						     field_id)),
			     rhs_wn);
      }
      else {
	if (is_realpart)
	  rhs_wn = WN_Binary(OPR_COMPLEX, rtype,
			     rhs_wn,
			     WN_Floatconst (Mtype_complex_to_real (rtype), 0.0));
	else
	if (is_imagpart)
	  rhs_wn = WN_Binary(OPR_COMPLEX, rtype,
			     WN_Floatconst (Mtype_complex_to_real (rtype), 0.0),
			     rhs_wn);
      }

      if (assign_code == PREINCREMENT_EXPR ||
          assign_code == PREDECREMENT_EXPR) {
        wn = WN_CreateIload (OPR_ILOAD, rtype, desc, component_offset,
                             field_id != 0 ? elem_ty_idx : desc_ty_idx,
                             Make_Pointer_Type(elem_ty_idx, FALSE),
                             WN_COPY_Tree (addr_wn),
                             field_id);
        rhs_wn = WN_Binary(Operator_From_Tree [assign_code].opr,
                           rtype, wn, rhs_wn);
	result_wn = rhs_wn;
      }
      else if (assign_code == POSTINCREMENT_EXPR ||
	       assign_code == POSTDECREMENT_EXPR) {
        result_wn = WN_CreateIload (OPR_ILOAD, rtype, desc, component_offset,
				    field_id != 0 ? elem_ty_idx : desc_ty_idx,
				    Make_Pointer_Type(elem_ty_idx, FALSE),
				    WN_COPY_Tree (addr_wn),
				    field_id);
      }
      else result_wn = rhs_wn;

      if (need_result && 
	  (volt ||
           assign_code == POSTINCREMENT_EXPR ||
	   assign_code == POSTDECREMENT_EXPR)) { // save result in a preg
        result_in_temp = TRUE;
#ifdef TARG_ST
	// [CG]: Fix 1-5-0-B/ddts/18793. See above.
	TY_IDX tmp_type = rtype == MTYPE_M ? desc_ty_idx: MTYPE_To_TY(rtype);
	result_preg_st = Gen_Temp_Symbol(tmp_type, ".tmp");
	result_preg = 0;
#else
        result_preg_st = MTYPE_To_PREG(rtype);
        result_preg = Create_Preg(rtype, NULL);
#endif
        wn = WN_Stid(rtype, result_preg, result_preg_st, desc_ty_idx, result_wn,
		     0);
        WFE_Stmt_Append (wn, Get_Srcpos());;
        result_wn = WN_Ldid(rtype, result_preg, result_preg_st, desc_ty_idx, 0);
      }

      if (assign_code == POSTINCREMENT_EXPR ||
          assign_code == POSTDECREMENT_EXPR) {
        rhs_wn = WN_Binary(Operator_From_Tree [assign_code].opr,
                           rtype, result_wn, rhs_wn);
      }
      else rhs_wn = result_wn;

      // rhs_wn is now always the right-hand-side of the assignment
    }

    // the assignment
    if (!WFE_Keep_Zero_Length_Structs &&
        desc == MTYPE_M               &&
        TY_size (elem_ty_idx) == 0) {
      // ignore zero length structs
      if (WN_has_side_effects (addr_wn)) {
#ifdef TARG_ST
	wn = WFE_Append_Expr_Stmt (addr_wn);
#else
        wn = WN_CreateEval (addr_wn);
        WFE_Stmt_Append (wn, Get_Srcpos());
#endif
      }
      wn = NULL;
    }
    else {
      wn = WN_CreateIstore(OPR_ISTORE, MTYPE_V, desc, component_offset, 
			   Make_Pointer_Type(elem_ty_idx, FALSE), rhs_wn,
			   addr_wn, field_id);
      WFE_Stmt_Append(wn, Get_Srcpos());
      if (need_result) {
        if (! result_in_temp)
	  wn = WN_CreateIload (OPR_ILOAD, rtype, desc, component_offset,
			       field_id != 0 ? elem_ty_idx : desc_ty_idx,
                               Make_Pointer_Type (elem_ty_idx, FALSE),
			       WN_COPY_Tree (addr_wn),
			       field_id);
	else wn = WN_Ldid(rtype, result_preg, result_preg_st, desc_ty_idx, 0);
	if (is_realpart)
	  wn = WN_Unary (OPR_REALPART, Mtype_complex_to_real (rtype), wn);
	else
	if (is_imagpart)
	  wn = WN_Unary (OPR_IMAGPART, Mtype_complex_to_real (rtype), wn);
      }
      else wn = NULL;
    }
  }
  else Fail_FmtAssertion ("WFE_Lhs_Of_Modify_Expr: unhandled tree node in LHS of MODIFY_EXPR");
  return wn;
}

/* ============================================================================
 *
 * WFE_Expand_Expr_With_Sequence_Point
 *
 * This routine is invoked instead of WN_Expand_Expr to handle the
 * following expression nodes
 *
 *   both operands of && and ||
 *   all three operands of conditional ?
 *   controlling expression of if
 *   controlling expression of switch
 *   controlling expression of while
 *   statement expression
 *
 * In order to generate WHIRL for an expression with side effects,
 * we would like to move operations such as calls, pre increment/decrement
 * into a comma operator, and operations such as post increment/decrement
 * into a rcomma operator.
 *
 * Sequence points related to function call and return are not handled
 * here as we cannot generate RCOMMA nodes in these cases.
 *
 * ============================================================================
 */

WN*
WFE_Expand_Expr_With_Sequence_Point (tree exp, TYPE_ID mtype)
{
  WN *wn;

  if (mtype == MTYPE_V)
    wn = WFE_Expand_Expr (exp, FALSE);

  else {

    WN *comma_block      = WN_CreateBlock ();

    WFE_Stmt_Push (comma_block, wfe_stmk_comma, Get_Srcpos ());
    wn = WFE_Expand_Expr (exp);

    // normalize bool expressions
    if (TREE_TYPE (exp) == boolean_type_node) {
      if (WN_operator (wn) == OPR_LDID ||
          WN_operator (wn) == OPR_ILOAD) {
        WN *zero = WN_Intconst (WN_rtype (wn), 0);
        wn = WN_Relational (OPR_NE, MTYPE_I4, wn, zero);
      }
    }

    WFE_Stmt_Pop (wfe_stmk_comma);
    if (WN_first (comma_block)) {
      if (wn)
	wn = WN_CreateComma (OPR_COMMA, Mtype_comparison (mtype), MTYPE_V,
			     comma_block, wn);
      else
	WFE_Stmt_Append (comma_block, Get_Srcpos());
    }
    else
      WN_Delete (comma_block);
  }

  return wn;
} /* WFE_Expand_Expr_With_Sequence_Point */

static void
emit_barrier (bool type, tree list, INT32 k)
{
  INT32  i;
  WN    *wn = WN_CreateBarrier (type, k);

  for (i = 0; i < k; i++) {
    tree exp = TREE_VALUE (list);
    ST *st   = Get_ST (exp);
    WN_kid (wn, i) = WN_Lda (Pointer_Mtype, 0, st,
                             Make_Pointer_Type (ST_type (st), FALSE));
    list = TREE_CHAIN (list);
  }

  WFE_Stmt_Append (wn, Get_Srcpos());
} /* emit_barrier */

static WN *
emit_builtin_lock_test_and_set (tree exp, INT32 k)
{
  WN        *wn;
  WN        *arg_wn;
  WN        *ikids [2];
  TYPE_ID    obj_mtype;
  TY_IDX     arg_ty_idx;
  TYPE_ID    arg_mtype;
  tree       list = TREE_OPERAND (exp, 1);
  OPCODE     opc;
  INTRINSIC  iopc;

  obj_mtype  = TY_mtype (TY_pointed (Get_TY(TREE_TYPE(TREE_VALUE(list)))));
  arg_ty_idx = Get_TY(TREE_TYPE(TREE_VALUE(list)));
  arg_mtype  = TY_mtype (arg_ty_idx);
  arg_wn     = WFE_Expand_Expr (TREE_VALUE (list));
  arg_wn     = WN_CreateParm (arg_mtype, arg_wn, arg_ty_idx, WN_PARM_BY_VALUE);
  ikids [0]  = arg_wn;
  list       = TREE_CHAIN (list);
  arg_ty_idx = Get_TY(TREE_TYPE(TREE_VALUE(list)));
  arg_mtype  = TY_mtype (arg_ty_idx);
  arg_wn     = WFE_Expand_Expr (TREE_VALUE (list));
  arg_wn     = WN_CreateParm (arg_mtype, arg_wn, arg_ty_idx, WN_PARM_BY_VALUE);
  ikids [1]  = arg_wn;
  list       = TREE_CHAIN (list);

  if (obj_mtype == MTYPE_I4) {
    opc  = OPC_I4INTRINSIC_CALL;
    iopc = INTRN_LOCK_TEST_AND_SET_I4;
  }
  else
  if (obj_mtype == MTYPE_U4) {
    opc  = OPC_U4INTRINSIC_CALL;
    iopc = INTRN_LOCK_TEST_AND_SET_I4;
  }
  else
  if (obj_mtype == MTYPE_I8) {
    opc  = OPC_I8INTRINSIC_CALL;
    iopc = INTRN_LOCK_TEST_AND_SET_I8;
  }
  else
  if (obj_mtype == MTYPE_U8) {
    opc  = OPC_U8INTRINSIC_CALL;
    iopc = INTRN_LOCK_TEST_AND_SET_I8;
  }
  else {
    Fail_FmtAssertion ("unknown object type in __builtin_lock_test_and_set");
    opc  = OPCODE_UNKNOWN;
    iopc = INTRINSIC_NONE;
  }

  wn = WN_Create_Intrinsic (opc, iopc, 2, ikids);
  WFE_Stmt_Append (wn, Get_Srcpos());

  ST       *preg_st = MTYPE_To_PREG(obj_mtype);
  TY_IDX    preg_ty_idx = Be_Type_Tbl(obj_mtype);
  PREG_NUM  preg = Create_Preg (obj_mtype, NULL);

  wn = WN_Ldid (obj_mtype, -1, Return_Val_Preg, preg_ty_idx);
  wn = WN_Stid (obj_mtype, preg, preg_st, preg_ty_idx, wn),
  WFE_Stmt_Append (wn, Get_Srcpos());

  emit_barrier (FALSE, list, k);

  wn = WN_Ldid (obj_mtype, preg, preg_st, preg_ty_idx);

  return wn;
} /* emit_builtin_lock_test_and_set */

static void
emit_builtin_lock_release (tree exp, INT32 k)
{
  WN        *wn;
  WN        *arg_wn;
  WN        *ikids [1];
  TYPE_ID    obj_mtype;
  TY_IDX     arg_ty_idx;
  TYPE_ID    arg_mtype;
  tree       list = TREE_OPERAND (exp, 1);
  OPCODE     opc;
  INTRINSIC  iopc;

  obj_mtype  = TY_mtype (TY_pointed (Get_TY(TREE_TYPE(TREE_VALUE(list)))));
  arg_ty_idx = Get_TY(TREE_TYPE(TREE_VALUE(list)));
  arg_mtype  = TY_mtype (arg_ty_idx);
  arg_wn     = WFE_Expand_Expr (TREE_VALUE (list));
  arg_wn     = WN_CreateParm (arg_mtype, arg_wn, arg_ty_idx, WN_PARM_BY_VALUE);
  ikids [0]  = arg_wn;
  list       = TREE_CHAIN (list);

  emit_barrier (TRUE, list, k);

  opc = OPC_VINTRINSIC_CALL;
  if (obj_mtype == MTYPE_I4)
    iopc = INTRN_LOCK_RELEASE_I4;
  else
  if (obj_mtype == MTYPE_U4)
    iopc = INTRN_LOCK_RELEASE_I4;
  else
  if (obj_mtype == MTYPE_I8)
    iopc = INTRN_LOCK_RELEASE_I8;
  else
  if (obj_mtype == MTYPE_U8)
    iopc = INTRN_LOCK_RELEASE_I8;
  else {
    Fail_FmtAssertion ("unknown object type in __builtin_lock_test_and_set");
    opc  = OPCODE_UNKNOWN;
    iopc = INTRINSIC_NONE;
  }

  wn = WN_Create_Intrinsic (opc, iopc, 1, ikids);
  WFE_Stmt_Append (wn, Get_Srcpos());
} /* emit_builtin_lock_release */

static WN *
emit_builtin_compare_and_swap (tree exp, INT32 k)
{
  WN        *wn;
  WN        *arg_wn;
  WN        *ikids [3];
  TYPE_ID    obj_mtype;
  TY_IDX     arg_ty_idx;
  TYPE_ID    arg_mtype;
  tree       list = TREE_OPERAND (exp, 1);
  OPCODE     opc;
  INTRINSIC  iopc;

  obj_mtype  = TY_mtype (TY_pointed (Get_TY(TREE_TYPE(TREE_VALUE(list)))));
  arg_ty_idx = Get_TY(TREE_TYPE(TREE_VALUE(list)));
  arg_mtype  = TY_mtype (arg_ty_idx);
  arg_wn     = WFE_Expand_Expr (TREE_VALUE (list));
  arg_wn     = WN_CreateParm (arg_mtype, arg_wn, arg_ty_idx, WN_PARM_BY_VALUE);
  ikids [0]  = arg_wn;
  list       = TREE_CHAIN (list);
  arg_ty_idx = Get_TY(TREE_TYPE(TREE_VALUE(list)));
  arg_mtype  = TY_mtype (arg_ty_idx);
  arg_wn     = WFE_Expand_Expr (TREE_VALUE (list));
  arg_wn     = WN_CreateParm (arg_mtype, arg_wn, arg_ty_idx, WN_PARM_BY_VALUE);
  ikids [1]  = arg_wn;
  list       = TREE_CHAIN (list);
  arg_ty_idx = Get_TY(TREE_TYPE(TREE_VALUE(list)));
  arg_mtype  = TY_mtype (arg_ty_idx);
  arg_wn     = WFE_Expand_Expr (TREE_VALUE (list));
  arg_wn     = WN_CreateParm (arg_mtype, arg_wn, arg_ty_idx, WN_PARM_BY_VALUE);
  ikids [2]  = arg_wn;
  list       = TREE_CHAIN (list);

  emit_barrier (TRUE, list, k);

  opc = OPC_I4INTRINSIC_CALL;
  if (obj_mtype == MTYPE_I4)
    iopc = INTRN_COMPARE_AND_SWAP_I4;
  else
  if (obj_mtype == MTYPE_U4)
    iopc = INTRN_COMPARE_AND_SWAP_I4;
  else
  if (obj_mtype == MTYPE_I8)
    iopc = INTRN_COMPARE_AND_SWAP_I8;
  else
  if (obj_mtype == MTYPE_U8)
    iopc = INTRN_COMPARE_AND_SWAP_I8;
  else {
    Fail_FmtAssertion ("unknown object type in __builtin_lock_test_and_set");
    opc  = OPCODE_UNKNOWN;
    iopc = INTRINSIC_NONE;
  }

  wn = WN_Create_Intrinsic (opc, iopc, 3, ikids);
  WFE_Stmt_Append (wn, Get_Srcpos());

  ST       *preg_st = MTYPE_To_PREG(MTYPE_I4);
  TY_IDX    preg_ty_idx = Be_Type_Tbl(MTYPE_I4);
  PREG_NUM  preg = Create_Preg (MTYPE_I4, NULL);

  wn = WN_Ldid (MTYPE_I4, -1, Return_Val_Preg, preg_ty_idx);
  wn = WN_Stid (MTYPE_I4, preg, preg_st, preg_ty_idx, wn),
  WFE_Stmt_Append (wn, Get_Srcpos());

  emit_barrier (FALSE, list, k);

  wn = WN_Ldid (MTYPE_I4, preg, preg_st, preg_ty_idx);

  return wn;
} /* emit_builtin_compare_and_swap */

static void
emit_builtin_synchronize (tree exp, INT32 k)
{
  WN *wn;
  tree list = TREE_OPERAND (exp, 1);
  emit_barrier (TRUE,  list, k);
  wn = WN_Create_Intrinsic (OPC_VINTRINSIC_CALL, INTRN_SYNCHRONIZE, 0, NULL);
  WFE_Stmt_Append (wn, Get_Srcpos());
  emit_barrier (FALSE, list, k);
} /* emit_builtin_synchronize */

#ifdef TARG_ST
static WN *
emit_builtin_classify_type (tree expr) 
{
  WN *wn ;
  int icode = no_type_class ;
  /* Refer to gccfe builtins.c/expand_builtin_classify_type*/
  if (expr) {
    tree arglist = TREE_OPERAND (expr, 1) ;
    tree type = TREE_TYPE (TREE_VALUE (arglist));
    enum tree_code code = TREE_CODE (type);
    if (code == VOID_TYPE)
	icode = void_type_class ;
    else if (code == INTEGER_TYPE)
	icode = integer_type_class ;
    else if (code == CHAR_TYPE)
	icode = char_type_class ;
    else if (code == ENUMERAL_TYPE)
	icode = enumeral_type_class ;
    else if (code == BOOLEAN_TYPE)
	icode = boolean_type_class ;
    else if (code == POINTER_TYPE)
	icode = pointer_type_class ;
    else if (code == REFERENCE_TYPE)
	icode = reference_type_class ;
    else if (code == OFFSET_TYPE)
	icode = offset_type_class ;
    else if (code == REAL_TYPE)
	icode = real_type_class ;
    else if (code == COMPLEX_TYPE)
	icode = complex_type_class ;
    else if (code == FUNCTION_TYPE)
	icode = function_type_class ;
    else if (code == METHOD_TYPE)
	icode = method_type_class ;
    else if (code == RECORD_TYPE)
	icode = record_type_class ;
    else if (code == UNION_TYPE || code == QUAL_UNION_TYPE)
	icode = union_type_class ;
    else if (code == ARRAY_TYPE)
    {
      if (TYPE_STRING_FLAG (type))
          icode = string_type_class ;
        else
          icode = array_type_class ;
    }
    else if (code == SET_TYPE)
	icode = set_type_class ;
    else if (code == FILE_TYPE)
	icode = file_type_class ;
    else if (code == LANG_TYPE)
	icode = lang_type_class ;
    else 
	icode = no_type_class ;
  }
  wn = WN_Intconst(MTYPE_I4, icode) ;
  return wn ;
}

static WN *
emit_builtin_trap ()
{
  WN *wn = WN_Create_Intrinsic (OPC_VINTRINSIC_CALL, INTRN_TRAP, 0, NULL);
  WFE_Stmt_Append (wn, Get_Srcpos()); 
  return wn;
} /* emit_builtin_trap */

static void
emit_builtin_init_dwarf_reg_sizes (WN *address)
{
  TYPE_ID desc = MTYPE_U1;
  INT64 desc_sz = MTYPE_byte_size (desc);
  TY_IDX ty_idx = Make_Pointer_Type (MTYPE_To_TY (desc));
  SRCPOS srcpos = Get_Srcpos ();
  for (INT i = 0; i < FIRST_PSEUDO_REGISTER_USED; i++) {
    int dwarf_id = GCCTARG_Dwarf_Get_Reg_Id_From_Gcc_Reg (i);
    if (dwarf_id < DWARF_FRAME_REGISTERS) {
      INT64 offset = dwarf_id * desc_sz;
      INT64 sz = GCCTARG_Gcc_Reg_Bit_Size (i) / BITS_PER_UNIT;

      if (offset < 0)
	continue;
      WFE_Stmt_Append (WN_Istore (desc, offset, ty_idx,
				  WN_COPY_Tree (address),
				  WN_Intconst (MTYPE_U4, sz), 0),
		       srcpos);
    }
  }
      
#ifdef DWARF_ALT_FRAME_RETURN_COLUMN
  {
    INT64 offset = DWARF_ALT_FRAME_RETURN_COLUMN * desc_sz;
    INT64 sz = GET_MODE_SIZE (Pmode);
    WFE_Stmt_Append (WN_Istore (desc, offset, ty_idx,
				WN_COPY_Tree (address),
				WN_IntConst (MTYPE_U4, sz), 0),
		     srcpos);
  }
#endif
}
#endif

static char *
get_string_pointer (WN *wn)
{
  char *ptr = NULL;

  if (WN_operator (wn) == OPR_LDA) {
    ST *st = WN_st (wn);
    if (ST_class (st) == CLASS_CONST) {
      TCON tcon = Tcon_Table [ST_tcon (st)];
      if (TCON_ty (tcon) == MTYPE_STRING)
        ptr = ((char *) Targ_String_Address (tcon)) + WN_offset (wn);
    }
  }

  return ptr;
} /* get_string_pointer */

// Auxiliary function for WFE_Expand_Expr, return the address of
// a tree operand.  (Used for ADDR_EXPR.)
WN*
WFE_Address_Of(tree arg0)
{
  enum tree_code code0 = TREE_CODE (arg0);
  ST *st = 0;
  WN* wn = 0;
  WN* wn0;
  WN* wn1;
  TY_IDX ty_idx;

  switch (code0) {
  case VAR_DECL:
  case PARM_DECL:
  case FUNCTION_DECL:

      st = Get_ST (arg0);
      ty_idx = ST_type (st);
      // for VLAs, use the base_st instead of st
#ifdef TARG_ST
      /* (cbr) VLAs are auto */
      if (code0 == VAR_DECL &&
          ST_sclass (st) == SCLASS_AUTO &&
          st != ST_base(st)) {
#else
        if (code0 == VAR_DECL &&
            st != ST_base(st)) {
#endif
        FmtAssert (ST_ofst (st) == 0,
                   ("VLA within struct not currently implemented"));
        wn = WN_Ldid (Pointer_Mtype, 0, ST_base(st), ST_type(ST_base(st)));
        }
#if defined (TARG_ST) && (GNU_FRONT_END==33)
      /* (cbr) gcc 3.3 upgrade: non_pods are reference parameters.
         Marked with TREE_ADDRESSABLE */
        else if (code0 == PARM_DECL && TREE_ADDRESSABLE(TREE_TYPE(arg0))) {
          wn = WN_Ldid (Pointer_Mtype, 0, st, ST_type(st));
        }
#endif
        else if (!WFE_Keep_Zero_Length_Structs &&
                 code0 == PARM_DECL            &&
                 TY_mtype (ty_idx) == MTYPE_M  &&
                 TY_size (ty_idx) == 0) {
          // taking address of zero length struct passed as parameter
          DevWarn ("taking address of zero length struct %s at line %d",
                   ST_name (st), lineno);
          wn = WN_Intconst (Pointer_Mtype, 0);
        }
        else
          wn = WN_Lda (Pointer_Mtype, ST_ofst(st), st);
    break;

  case INDIRECT_REF:
    wn = WFE_Expand_Expr (TREE_OPERAND(arg0, 0));
    break;

  case STRING_CST:
    {
      TCON tcon;
      tcon = Host_To_Targ_String (MTYPE_STRING,
                                  TREE_STRING_POINTER(arg0),
                                  TREE_STRING_LENGTH(arg0));
      ty_idx = Get_TY(TREE_TYPE(arg0));
      st = New_Const_Sym (Enter_tcon (tcon), ty_idx);
      wn = WN_Lda (Pointer_Mtype, ST_ofst(st), st);
      TREE_STRING_ST (arg0) = st;
    }
    break;

  case CONSTRUCTOR:
    {
      st = WFE_Generate_Temp_For_Initialized_Aggregate (arg0, "");
      wn = WN_Lda (Pointer_Mtype, ST_ofst(st), st);
    }
    break;

  case LABEL_DECL:
    {
#ifndef TARG_ST
      DevWarn ("taking address of a label at line %d", lineno);
#endif
      LABEL_IDX label_idx = WFE_Get_LABEL (arg0, FALSE);
#if 0
      FmtAssert (arg0->decl.symtab_idx == CURRENT_SYMTAB,
                 ("line %d: taking address of a label not defined in current function currently not implemented", lineno));
#endif
      wn = WN_LdaLabel (Pointer_Mtype, label_idx);
      Set_LABEL_addr_saved (label_idx);
#ifdef TARG_ST
      /* (cbr) label is taken, can't inline */
      Set_PU_no_inline (Get_Current_PU ());
#endif
    }
    break;

  case TARGET_EXPR:
    {
      WFE_Expand_Expr (arg0);
      st = Get_ST (TREE_OPERAND(arg0, 0));
#if defined (TARG_ST) && (GNU_FRONT_END==33)
      /* (cbr) gcc 3.3 upgrade: non_pods are reference parameters.
         Marked with TREE_ADDRESSABLE */
      if (TREE_CODE(TREE_OPERAND(arg0, 0)) == PARM_DECL &&
          TREE_ADDRESSABLE(TREE_TYPE(TREE_OPERAND(arg0, 0)))) {
        wn = WN_Ldid (Pointer_Mtype, 0, st, ST_type(st));        
      }
      else
#endif
        wn = WN_Lda (Pointer_Mtype, ST_ofst(st), st);
    }
    break;

  case COMPOUND_EXPR:
    {
      st = WN_st (WFE_Expand_Expr (arg0));
      wn = WN_Lda (Pointer_Mtype,  ST_ofst (st), st);
    }
    break;

  case NOP_EXPR:
    {
      wn = WFE_Address_Of(TREE_OPERAND(arg0, 0));
    }
    break;

  case MIN_EXPR:
  case MAX_EXPR:
    {
      // &(a <? b) or &(a >? b)
      tree op0 = TREE_OPERAND(arg0, 0);
      tree op1 = TREE_OPERAND(arg0, 1);
      WN* a = WFE_Expand_Expr(op0);
      WN* b = WFE_Expand_Expr(op1);
      FmtAssert(!WN_has_side_effects(a) && !WN_has_side_effects(b),
                ("Addr of MIN/MAX_EXPR with side effects not yet supported"));

      FmtAssert(same_type_p(TREE_TYPE(op0), TREE_TYPE(op1)),
                ("Types of MIN/MAX_EXPR operands differ"));
      TY_IDX  ptr_ty    = Make_Pointer_Type (Get_TY(TREE_TYPE(op0)), FALSE);
      TYPE_ID ptr_mtype = TY_mtype(ptr_ty);
      TY_IDX  arg_ty    = Get_TY(TREE_TYPE(TREE_OPERAND(arg0, 0)));
      TYPE_ID arg_mtype = TY_mtype(arg_ty);

      WN* aptr = WFE_Address_Of(op0);
      WN* bptr = WFE_Address_Of(op1);
      wn = WN_Select(Widen_Mtype(ptr_mtype),
                     WN_Relational(code0 == MIN_EXPR ? OPR_LT : OPR_GT,
                                   Widen_Mtype(arg_mtype),
                                   a, b),
                     aptr, bptr);
      Set_PU_has_very_high_whirl (Get_Current_PU ());
    }
    break;

  case COMPONENT_REF:
    {
      wn = WFE_Expand_Expr (arg0);
      ty_idx = Get_TY(TREE_TYPE(arg0));
      if (WN_operator (wn) == OPR_LDID) {
        WN_set_operator (wn, OPR_LDA);
        WN_set_desc (wn, MTYPE_V);
      }
      else
      if (WN_operator (wn) == OPR_ILOAD) {
        wn0 = WN_kid0 (wn);
        wn1 = WN_Intconst (Pointer_Mtype, WN_offset (wn));
        wn  = WN_Binary (OPR_ADD, Pointer_Mtype, wn0, wn1);
      }
      else
        Fail_FmtAssertion ("WFE_Address_Of has unhandled %s",
                           Operator_From_Tree [code0].name);
    }
    break;

#ifdef TARG_ST
    /* (cbr) compute, save expression and return address */
    case SAVE_EXPR:
      {
        wn = WFE_Expand_Expr (arg0);
        st = WN_st (wn);
        wn = WN_Lda (Pointer_Mtype, ST_ofst(st), st);
      }
      break;
    case VA_ARG_EXPR:
      wn = WFE_Expand_Expr (arg0);
      if (WN_operator (wn) == OPR_ILOAD) {
	/* [SC] Remove the indirection to get the address. */
	wn = WN_Binary (OPR_ADD, Pointer_Mtype,
			WN_kid0 (wn),
			WN_Intconst (Pointer_Mtype, WN_offset (wn)));
      } else {
	/* [SC] Save to temp, and return address of temp. */
	tree type = TREE_TYPE (arg0);
	TY_IDX va_ty_idx = Get_TY (type);
	TYPE_ID va_mtype = TY_mtype (va_ty_idx);
	ST *temp_st = Gen_Temp_Symbol (va_ty_idx, ".tmp");
	Set_ST_addr_saved (temp_st);
	wn = WN_Stid (va_mtype, 0, temp_st, va_ty_idx, wn);
	WFE_Stmt_Append (wn, Get_Srcpos ());
	wn = WN_Lda (Pointer_Mtype, ST_ofst(temp_st), temp_st);
      }
      break;
#endif

  default:
    {
      Fail_FmtAssertion ("WFE_Address_Of: Unexpected operand %s",
                         Operator_From_Tree [code0].name);
    }
    break;
  }

  FmtAssert(wn != 0, ("WFE_Address_Of: null WHIRL tree for %s",
                      Operator_From_Tree [code0].name));


  return wn;
}

#ifdef KEY
static bool inside_eh_region = false;
// Setup an EH region, typically across a function call.
void
Setup_EH_Region (bool for_unwinding)
{
    WN * region_body;

    if (for_unwinding)
	region_body = WFE_Stmt_Pop (wfe_stmk_region_body);
    else
    	{
	    region_body = WFE_Stmt_Pop (wfe_stmk_call_region_body);
	    inside_eh_region = false;
	}

    INITV_IDX iv;
    LABEL_IDX pad = 0;

    if (!for_unwinding) pad = lookup_cleanups (iv);
    else
    {
	iv = New_INITV();
        INITV_Set_ZERO (Initv_Table[iv], MTYPE_U4, 1);
    }

    INITV_IDX initv_label = New_INITV();
    if (pad)
    	INITV_Init_Label (initv_label, pad, 1);
    else
	INITV_Set_ZERO (Initv_Table[initv_label], MTYPE_U4, 1);
    INITV_IDX blk = New_INITV();
    INITV_Init_Block (blk, initv_label);

    Set_INITV_next (initv_label, iv);

    TY_IDX ty = MTYPE_TO_TY_array[MTYPE_U4];
    ST * ereg = Gen_Temp_Named_Symbol (ty, "dummy1", CLASS_VAR,
				SCLASS_EH_REGION_SUPP);
    Set_ST_is_initialized (*ereg);
    Set_ST_is_not_used (*ereg);
    INITO_IDX ereg_supp = New_INITO (ST_st_idx(ereg), blk);

    WFE_Stmt_Append (WN_CreateRegion (REGION_KIND_EH, region_body,
      WN_CreateBlock(), WN_CreateBlock(), New_Region_Id(), ereg_supp), Get_Srcpos());

    Set_PU_has_region (Get_Current_PU());
    Set_PU_has_exc_scopes (Get_Current_PU());
}
#endif // KEY

#ifdef TARG_ST
struct WFE_Common_Save_Expr_Data {
  INT first_seen;
  INT last_seen;
  tree exp;
};

class WFE_Eval_Common_Save_Exprs_Data {
public:
  INT current_operand;
  std::list<WFE_Common_Save_Expr_Data *> save_exprs;
  MEM_POOL pool;
  
  WFE_Eval_Common_Save_Exprs_Data () : current_operand (0),
				       save_exprs (std::list<WFE_Common_Save_Expr_Data *>())
  {
    MEM_POOL_Initialize (&pool, "WFE_Eval_Common_Save_Exprs_Data pool", FALSE);
  }
  ~WFE_Eval_Common_Save_Exprs_Data () {
    MEM_POOL_Delete (&pool);
  }
  void Visit_TreeNode (tree exp);
};

void
WFE_Eval_Common_Save_Exprs_Data::Visit_TreeNode (tree save_exp)
  // EXP is a SAVE_EXPR tree: note that we have seen it in the
  // current operand.
{
  std::list<WFE_Common_Save_Expr_Data *>::iterator it;
  WFE_Common_Save_Expr_Data *entry = NULL;
  
  tree exp = TREE_OPERAND (save_exp, 0);
  // Look up the entry for this tree.
  for (it = save_exprs.begin ();
       it != save_exprs.end ();
       it++) {
    if (TREE_OPERAND ((*it)->exp, 0) == exp) {
      entry = *it;
      break;
    }
  }
  if (! entry) {
    // Not found, create a new entry.
    entry = TYPE_MEM_POOL_ALLOC (WFE_Common_Save_Expr_Data, &pool);
    entry->exp = save_exp;
    entry->first_seen = current_operand;
    save_exprs.push_back (entry);
  }
  // Note that entry has been seen in current operand.
  entry->last_seen = current_operand;
}

tree WFE_Eval_Common_Save_Exprs_Visit_Node (tree *tp,
					    int *walk_subtrees,
					    void *data)
{
  WFE_Eval_Common_Save_Exprs_Data *Data =
    (WFE_Eval_Common_Save_Exprs_Data *)data;
  
  if (TREE_CODE (*tp) == SAVE_EXPR) {
    Data->Visit_TreeNode (*tp);
  }
  
  return NULL;
}

void
WFE_Eval_Common_Save_Exprs (tree exp, INT noperands)
{
  INT opd;
  WFE_Eval_Common_Save_Exprs_Data Data;
  // First find the common save_exprs.
  for (opd = 0; opd < noperands; opd++) {
    Data.current_operand = opd;
    walk_tree_without_duplicates (& TREE_OPERAND (exp, opd),
				  WFE_Eval_Common_Save_Exprs_Visit_Node,
				  & Data);
  }
  
  // Now evaluate the save_exprs that appeared in multiple
  // operands.
  std::list<WFE_Common_Save_Expr_Data *>::iterator it;
  for (it = Data.save_exprs.begin ();
       it != Data.save_exprs.end ();
       it++) {
    if ((*it)->first_seen != (*it)->last_seen) {
      // This one seen in multiple operands, so evaluate it.
      // Ignore the returned whirl tree - we do not need it.
#ifdef WFE_DEBUG
      printf("\nEvaluating common save_expr of ");
      print_tree (stdout, exp);
#endif
      (void)WFE_Save_Expr ((*it)->exp, FALSE, 0, 0, 0, 0);
    }
  }
}
#endif

/* expand gnu expr tree into symtab & whirl */
WN *
WFE_Expand_Expr (tree exp, 
		 bool need_result,
		 TY_IDX nop_ty_idx, 
		 TY_IDX component_ty_idx, 
		 INT64 component_offset,
		 UINT16 field_id,
		 bool is_bit_field,
		 bool is_aggr_init_via_ctor)
{
  FmtAssert(exp != NULL_TREE, ("WFE_Expand_Expr: null argument"));
  enum tree_code code = TREE_CODE (exp);
  WN *wn, *wn0, *wn1, *wn2;
  ST *st;
  TY_IDX ty_idx;
  TY_IDX desc_ty_idx;
  tree arg0, arg1, arg2;

  wn = NULL;

#ifdef WFE_DEBUG
  fprintf (stderr,
           "{( WFE_Expand_Expr: %s\n", Operator_From_Tree [code].name); // ")}"
#endif /* WFE_DEBUG */

#ifdef TARG_ST
  if (TREE_CODE_CLASS(code) == '2') {
    WFE_Eval_Common_Save_Exprs (exp, 2);
  }
#endif
  
  switch (code)
    {
    // leaves
    case ADDR_EXPR:
      wn = WFE_Address_Of(TREE_OPERAND(exp, 0));
      break;

    case FUNCTION_DECL:
      {
	 st = Get_ST (exp);
	 ty_idx = ST_type (st);
	 wn = WN_Lda (Pointer_Mtype, ST_ofst(st), st);
      }
      break;

    case TREE_LIST: 
      {
	tree stmt;
	for (stmt = TREE_PURPOSE(exp); stmt; stmt = TREE_CHAIN(stmt))
	  WFE_Expand_Stmt (stmt);
	wn = WFE_Expand_Expr (TREE_VALUE(exp));
      }
      break;

    case DECL_STMT:
      {
        tree decl = DECL_STMT_DECL(exp);
	WFE_Expand_Decl (decl);
	wn = WFE_Expand_Expr (decl);
      }
      break;

    case BIND_EXPR:
#ifdef GPLUSPLUS_FE
#ifndef TARG_ST
      DevWarn ("Encountered BIND_EXPR at line %d", lineno);
#endif
      // ignore the first operand as it ia a list of temporary variables
      wn = WFE_Expand_Expr (TREE_OPERAND (exp, 1));
      break;
#else
      {
        INT32    i;
        WN      *block;
        TYPE_ID  mtype;
        tree     t;

#ifndef TARG_ST
	DevWarn ("Encountered BIND_EXPR at line %d", lineno);
#endif
        for (i = wfe_bind_expr_stack_last; i >= 0; --i) {

          if (wfe_bind_expr_stack [i].rtl_expr == TREE_OPERAND (exp, 1)) {

            block = wfe_bind_expr_stack [i].block;
            t     = wfe_bind_expr_stack [i].rtl_expr;
            wfe_bind_expr_stack [i] = wfe_bind_expr_stack [wfe_bind_expr_stack_last];
            --wfe_bind_expr_stack_last;
            break;
          }
        }

        FmtAssert (i >= 0,
                   ("BIND_EXPR: did not find tree"));
	ty_idx = Get_TY (TREE_TYPE(t));
        mtype  = TY_mtype (ty_idx);
	if (mtype == MTYPE_V) {
	  WFE_Stmt_Append (block, Get_Srcpos ());
          break;
	}
	else {
	  wn0 = block;
	  wn1 = WN_COPY_Tree (WN_last (wn0));
	  WN_DELETE_FromBlock (wn0, WN_last (wn0));
	  WFE_Stmt_Append (wn0, Get_Srcpos ());
	  if (nop_ty_idx == 0 && component_ty_idx == 0) {
	    wn = WN_kid0 (wn1);
            break;
	  }
          if (WN_operator (WN_kid0 (wn1)) == OPR_LDID)
            st = WN_st (WN_kid0 (wn1));
          else {
            st = Gen_Temp_Symbol (ty_idx, "__bind_expr");
            WFE_Set_ST_Addr_Saved (WN_kid0 (wn1));
            wn0 = WN_Stid (mtype, 0, st, ty_idx, WN_kid0 (wn1));
            WFE_Stmt_Append (wn0, Get_Srcpos ());
          }
	}
      }
      /*FALLTHRU*/
#endif /* GPLUSPLUS_FE */

    case TARGET_EXPR:
      {
	st            = Get_ST (TREE_OPERAND(exp, 0));
	TY_IDX ty     = ST_type(st);
	TYPE_ID mtype = TY_mtype (ty);
	/*
	 * Usually operand 1 of the target_expr will be an aggr_init_expr
	 * for which AGGR_INIT_VIA_CTOR_P holds.  Such a node has the
	 * annoying property that its first argument is not the expected
	 * argument to the constructor call.  Instead the argument whose
	 * address should be passed to the constructor appears as 
	 * operand 2.  The value in operand 2, however, is not always
 	 * right:  it is the original temporary var_decl from the
	 * target_expr.  What we really want is the current operand 0
	 * of the target_expr, which may have been changed (see INIT_EXPR).
	 * This is really ugly, but we can't help it because at the
	 * expression level we need to stay compatible with the current
	 * rtl generation.
	 * So we're going to replace the first argument of the aggr_init_expr
	 * with the var_decl from operand 0 of the target_expr, and pass
	 * is_aggr_init_via_ctor to WFE_Expand_Expr, so it can be dealt
	 * with by the AGGR_INIT_EXPR/CALL_EXPR code.
	 *
	 * If a target expression is initialized by a target expression,
	 * it ought not to have an associated cleanup, so we clear the
	 * cleanup in this case.
	 */
	tree t = TREE_OPERAND(exp, 1);
 	if (TREE_CODE(t) == TARGET_EXPR)
	  TREE_OPERAND(t, 2) = 0;

#if defined (TARG_ST) && (GNU_FRONT_END==33)
        if (TREE_CODE (t) == COND_EXPR) {
          tree targ = TREE_OPERAND(t, 1);
          if (TREE_CODE (targ) == TARGET_EXPR)
            TREE_OPERAND(targ, 2) = NULL_TREE;
          targ = TREE_OPERAND(t, 2);
          if (TREE_CODE (targ) == TARGET_EXPR)
            TREE_OPERAND(targ, 2) = NULL_TREE;
        }
#endif

#if defined (TARG_ST) && (GNU_FRONT_END==33)
        /* (cbr) avoid creating a temporary object for ctors */
        if (TREE_CODE (t) == COMPOUND_EXPR && 
            !TREE_OPERAND(exp,2) &&
            TREE_CODE (TREE_OPERAND (t, 0)) == CALL_EXPR &&
            TREE_CODE (TREE_OPERAND (TREE_OPERAND (t, 0), 0)) == ADDR_EXPR &&
            DECL_CONSTRUCTOR_P (TREE_OPERAND (TREE_OPERAND (TREE_OPERAND (t, 0), 0) ,0))) {
          tree args = TREE_VALUE(TREE_OPERAND (TREE_OPERAND (t, 0), 1));

	  TREE_OPERAND(args, 0) = TREE_OPERAND(exp, 0);

	  WFE_Expand_Expr (TREE_OPERAND (t, 0));
        }
        else
          if (TREE_CODE (t) == CALL_EXPR &&
              (RETURN_IN_MEMORY (TREE_TYPE(TREE_OPERAND(exp,0))) ||
               (TYPE_LANG_SPECIFIC(TREE_TYPE(TREE_OPERAND(exp,0))) &&
                TREE_ADDRESSABLE (TREE_TYPE(TREE_OPERAND(exp,0))) && 
                CLASSTYPE_NON_POD_P (TREE_TYPE(TREE_OPERAND(exp,0)))))) {  
            tree args = tree_cons (NULL_TREE,
                                   build1 (ADDR_EXPR,
                                           build_pointer_type(TREE_TYPE(TREE_OPERAND(exp,0))),
                                           TREE_OPERAND(exp,0)),
                                   TREE_OPERAND(t, 1));


            if (st) {
              Set_ST_addr_passed (st);
            }
            
            TREE_OPERAND(t, 1) = args;
            WFE_Expand_Expr (t, false);
          }
        else
#endif

	if (TREE_CODE(t) == (enum tree_code)AGGR_INIT_EXPR && AGGR_INIT_VIA_CTOR_P(t)) {
	  tree args = TREE_OPERAND(t, 1);
	  TREE_VALUE(args) = TREE_OPERAND(exp, 0);
	  WFE_Expand_Expr (t, false, 0, 0, 0, 0, false, true);
	}
	else {
	  WFE_Stmt_Append(WN_Stid (mtype, ST_ofst(st), st, ty,
		        	   WFE_Expand_Expr (t)),
			  Get_Srcpos());
#ifdef TARG_ST
          /* (cbr) mark symbol as addr_saved if its address is taken */
          if (TREE_CODE(t) == ADDR_EXPR &&
	      (
              TREE_CODE (TREE_OPERAND (t, 0)) == VAR_DECL
	      ||
              TREE_CODE (TREE_OPERAND (t, 0)) == FUNCTION_DECL
	       )
	      ) {
            Set_ST_addr_saved(Get_ST (TREE_OPERAND (t, 0)));
          }
#endif
	}

        if (TREE_OPERAND(exp, 2)) {
#ifdef TARG_ST
          /* (cbr) support for deferred cleanups */
          ST *guard_st=0;

          if ((TREE_CODE (t) == COMPOUND_EXPR && TREE_SIDE_EFFECTS(t) ||
              (TREE_CODE (t) == CALL_EXPR && TYPE_HAS_NONTRIVIAL_DESTRUCTOR (TREE_TYPE (t))))) {
            guard_st = Get_Deferred_Cleanup(); 
        }
#endif

          tree cleanup = TREE_OPERAND(exp, 2);

#ifdef TARG_ST
            /* (cbr) conditionalize the cleanup.  */
          if (guard_st) {
            tree cond = build_decl (VAR_DECL, get_identifier(ST_name(ST_st_idx(guard_st))), unsigned_type_node);
            DECL_ST(cond) = guard_st;

            cleanup = build (COND_EXPR, void_type_node,
                             c_common_truthvalue_conversion(cond),
                             cleanup, integer_zero_node);
            TREE_OPERAND(exp, 2) = cleanup;
            }
          
          Push_Temp_Cleanup(cleanup, true, CLEANUP_EH_ONLY (exp));
#else
#ifdef KEY
          Push_Temp_Cleanup(TREE_OPERAND(exp, 2), true, CLEANUP_EH_ONLY (exp));
#else
          Push_Temp_Cleanup(TREE_OPERAND(exp, 2), true);
#endif
#endif
        }
      }


    case PARM_DECL: // for formal parms
    case CONSTRUCTOR:
    case VAR_DECL:
      {
	UINT xtra_BE_ofst = 0; 	// only needed for big-endian target
        PREG_NUM preg_num = 0;
	desc_ty_idx = component_ty_idx;
	TY_IDX hi_ty_idx = Get_TY (TREE_TYPE(exp));

#ifdef TARG_ST
	BOOL is_volatile = TREE_THIS_VOLATILE(exp);
#endif

	if (desc_ty_idx == 0)
	  desc_ty_idx = hi_ty_idx;

#ifdef TARG_ST
	if (! MTYPE_is_class_integer(TY_mtype(desc_ty_idx)))
#else
	if (! MTYPE_is_integral(TY_mtype(desc_ty_idx)))
#endif
	  ty_idx = desc_ty_idx;
	else {
	  ty_idx = nop_ty_idx;
	  if (ty_idx == 0) 
	    ty_idx = desc_ty_idx;
	}

#if 0
	fprintf(stdout, "  hi_ty_idx = %s\n", TY_name(hi_ty_idx));
	fprintf(stdout, "  ty_idx = %s\n", TY_name(ty_idx));
	fprintf(stdout, "  desc_ty_idx = %s\n", TY_name(desc_ty_idx));
#endif
	UINT cvtl_size = 0; // if non-zero, need to generate CVTL with this size
#ifdef TARG_ST
	//
	// Arthur: if only 32 bit operations are allowed on target,
	//         need a CVT
	//
	BOOL need_cvt = FALSE;
	BOOL need_cvtl = FALSE;
	TY_IDX cvt_ty_idx;

	// 
	// First, determine whether a CVT is necessary
	//
	if (Only_32_Bit_Ops) {
	  if (TY_size(ty_idx) > TY_size(desc_ty_idx)) {
	    //
	    // widening a value in memory 
	    //
	    // Want to generate:
	    //
	    //    I4I2LDID
	    //  I8I4CVT         if desc_ty_idx <= 4; ty_idx > 4
	    //
	    //  I8I5LDID        if desc_ty_idx > 4; ty_idx > 4
	    //
	    //  I4I2LDID        if ty_idx <= 4
	    //
	    if (TY_size(ty_idx) > 4 && TY_size(desc_ty_idx) <= 4) {
	      need_cvt = TRUE;
	      cvt_ty_idx = ty_idx;
	      ty_idx = MTYPE_To_TY(Mtype_TransferSign(TY_mtype(desc_ty_idx), MTYPE_I4));
	    }
	  }
	  else if (TY_size(ty_idx) < TY_size(desc_ty_idx)) {

	    //
	    // truncating the value in memory to a smaller value
	    // in register. Want to generate depending on sizes:
	    //
	    //
	    //   1. ty_idx:I2 desc_idx: I4
	    //
	    //      I4I4LDID
	    //    I4CVTL 16 
	    //
	    //   2. ty_idx:I2 desc_idx: I8
	    //
	    //        I8I8LDID
	    //      I4I8CVT
	    //    I4CVTL 16
	    //
	    //   3. ty_idx: I5 desc_idx: I8
	    //
	    //      I8I8LDID
	    //    I5I8CVT
	    //

	    if (TY_size(desc_ty_idx) > 4 && TY_size(ty_idx) >= 4) {
	      need_cvt = TRUE;
	      cvt_ty_idx = ty_idx;
	    }
	    else if (TY_size(desc_ty_idx) <= 4) {
	      if (! is_bit_field) {
		need_cvtl = TRUE;
		cvt_ty_idx = MTYPE_signed(TY_mtype(ty_idx)) ? MTYPE_To_TY(MTYPE_I4) : MTYPE_To_TY(MTYPE_U4);
		cvtl_size = TY_size(ty_idx) * 8;
	      }
	    }
	    else {
	      if (! is_bit_field) {
		need_cvt = TRUE;
		need_cvtl = TRUE;
		cvt_ty_idx = MTYPE_signed(TY_mtype(ty_idx)) ? MTYPE_To_TY(MTYPE_I4) : MTYPE_To_TY(MTYPE_U4);
		cvtl_size = TY_size(ty_idx) * 8;
	      }
	    }

	    ty_idx = desc_ty_idx;

	  } // truncation
	}

#if 0
	fprintf(stdout, "ty_idx = %s\n", MTYPE_name(TY_mtype(ty_idx)));
	fprintf(stdout, "desc_ty_idx = %s\n", MTYPE_name(TY_mtype(desc_ty_idx)));
#endif

#else  /* !TARG_ST */

	if (! is_bit_field) {
	  if (TY_size(desc_ty_idx) > TY_size(ty_idx)) {
	    if (Target_Byte_Sex == BIG_ENDIAN)
	      xtra_BE_ofst = TY_size(desc_ty_idx) - TY_size(ty_idx);
	    cvtl_size = TY_size(ty_idx) * 8;
	    ty_idx = desc_ty_idx;
	  }
	}
	else {
	  if (TY_size(desc_ty_idx) > TY_size(ty_idx)) 
	    ty_idx = desc_ty_idx;
	}
#endif /* TARG_ST */

        TYPE_ID rtype = Widen_Mtype(TY_mtype(ty_idx));
        TYPE_ID desc = TY_mtype(desc_ty_idx);
#ifdef TARG_ST
        if (MTYPE_is_class_integer(desc)) {
#else
        if (MTYPE_is_integral(desc)) {
#endif
          if (MTYPE_signed(rtype) != MTYPE_signed(desc)) {
            if (MTYPE_size_min(rtype) > MTYPE_size_min(desc) ||
		is_bit_field)
              rtype = Mtype_TransferSign(desc, rtype);
            else desc = Mtype_TransferSign(rtype, desc);
          }
        }

#ifndef TARG_ST
	if (TREE_THIS_VOLATILE(exp))
	  Set_TY_is_volatile(ty_idx);
#endif

	if (code == PARM_DECL || code == VAR_DECL) {
            st = Get_ST (exp);

          if (ST_assigned_to_dedicated_preg (st))
#ifdef TARG_ST
	    is_volatile = TRUE;
#else
	    Set_TY_is_volatile(ty_idx);
#endif
        }
	else
	if (code == CONSTRUCTOR) {
#ifndef TARG_ST
	  DevWarn ("Encountered CONSTRUCTOR at line %d", lineno);
#endif
	  st = WFE_Generate_Temp_For_Initialized_Aggregate (exp, "");
	}

	Is_True(! is_bit_field || field_id <= MAX_FIELD_ID,
		("WFE_Expand_Expr: field id for bit-field exceeds limit"));
#ifdef TARG_ST
        /* (cbr) gcc 3.3 upgrade: non_pods are reference parameters.
           Marked with TREE_ADDRESSABLE */
	TY_IDX ldid_ty_idx = field_id != 0 ? hi_ty_idx : ty_idx;
	if (is_volatile) Set_TY_is_volatile(ldid_ty_idx);

        if (code == PARM_DECL && TREE_ADDRESSABLE(TREE_TYPE(exp))) {
          WN *w0 = WN_Ldid (Pointer_Mtype, 0, st, ST_type(st));

          wn = WN_CreateIload(OPR_ILOAD, rtype,
                              is_bit_field ? MTYPE_BS : desc, 
                              ST_ofst(st)+component_offset,
                              ldid_ty_idx, ST_type(st), w0, field_id);
        }
        else {
          wn = WN_CreateLdid (OPR_LDID, rtype,
                              is_bit_field ? MTYPE_BS : desc,
                              ST_ofst(st)+component_offset+preg_num, st,
                              ldid_ty_idx, field_id);
        }

	if (is_volatile)
	  DevAssert (WN_Is_Volatile_Mem(wn), 
		     ("Non volatile generated for opcode %s", OPCODE_name(WN_opcode(wn))));
#else
	wn = WN_CreateLdid (OPR_LDID, rtype,
			    is_bit_field ? MTYPE_BS : desc,
			    ST_ofst(st)+component_offset+xtra_BE_ofst+preg_num, st,
			    field_id != 0 ? hi_ty_idx : ty_idx, field_id);
#endif
#ifdef TARG_ST
	// May need both !
	if (need_cvt) {
	  wn = WN_Cvt(rtype, TY_mtype(cvt_ty_idx), wn);
	}
	if (need_cvtl) {
	  wn = WN_CreateCvtl(OPR_CVTL, TY_mtype(cvt_ty_idx), MTYPE_V, cvtl_size, wn);
	}
#else
	if (cvtl_size != 0)
	  wn = WN_CreateCvtl(OPR_CVTL, rtype, MTYPE_V, cvtl_size, wn);
#endif
      }
      break;

    case CONST_DECL:
        wn = WFE_Expand_Expr(DECL_INITIAL(exp), need_result);
	break;

    case INTEGER_CST:
      {
	ty_idx = Get_TY (TREE_TYPE(exp));
	TYPE_ID mtyp = TY_mtype(ty_idx);
	mtyp = (mtyp == MTYPE_V) ? MTYPE_I4 : Widen_Mtype(mtyp);
	wn = WN_Intconst(mtyp, Get_Integer_Value(exp));
      }
      break;

    case PTRMEM_CST:
      {
	wn = WFE_Expand_Expr(cplus_expand_constant (exp),
			     need_result, nop_ty_idx, component_ty_idx,
			     component_offset, field_id);
      }
      break;

    case REAL_CST:
#if defined (TARG_ST) && (GNU_FRONT_END==33)
      /* (cbr) gcc 3.3 upgrade */
      {
	TCON tcon;
        double val = real_value_to_double (TREE_REAL_CST(exp));
	ty_idx = Get_TY (TREE_TYPE(exp));
	tcon = Host_To_Targ_Float (TY_mtype (ty_idx), val);
	st = New_Const_Sym (Enter_tcon (tcon), ty_idx);
	wn = WN_CreateConst (OPR_CONST, TY_mtype (ty_idx), MTYPE_V, st);
      }
#else
      {
	TCON tcon;
	ty_idx = Get_TY (TREE_TYPE(exp));
	tcon = Host_To_Targ_Float (TY_mtype (ty_idx), TREE_REAL_CST(exp));
	st = New_Const_Sym (Enter_tcon (tcon), ty_idx);
	wn = WN_CreateConst (OPR_CONST, TY_mtype (ty_idx), MTYPE_V, st);
      }
#endif
      break;

    case COMPLEX_CST:
#if defined (TARG_ST) && (GNU_FRONT_END==33)
      /* (cbr) gcc 3.3 upgrade */
      {
	TCON tcon;
        long l1, l2;
        REAL_VALUE_TO_TARGET_SINGLE(TREE_REAL_CST(TREE_REALPART(exp)), l1);
        REAL_VALUE_TO_TARGET_SINGLE(TREE_REAL_CST(TREE_IMAGPART(exp)), l2);
	ty_idx = Get_TY (TREE_TYPE(exp));
	tcon = Host_To_Targ_Complex (TY_mtype (ty_idx),
                                     *(float*)&l1, *(float*)&l2);
	st = New_Const_Sym (Enter_tcon (tcon), ty_idx);
	wn = WN_CreateConst (OPR_CONST, TY_mtype (ty_idx), MTYPE_V, st);
      }
#else
      {
	TCON tcon;
	ty_idx = Get_TY (TREE_TYPE(exp));
	tcon = Host_To_Targ_Complex (TY_mtype (ty_idx),
				     TREE_REAL_CST(TREE_REALPART(exp)),
				     TREE_REAL_CST(TREE_IMAGPART(exp)));
	st = New_Const_Sym (Enter_tcon (tcon), ty_idx);
	wn = WN_CreateConst (OPR_CONST, TY_mtype (ty_idx), MTYPE_V, st);
      }
#endif
      break;

    // this should occur only if string is a statement expression
    case STRING_CST:
      {
	TCON tcon;
	tcon = Host_To_Targ_String (MTYPE_STRING,
				    TREE_STRING_POINTER(exp),
				    TREE_STRING_LENGTH(exp));
	ty_idx = Get_TY(TREE_TYPE(exp));
	st = New_Const_Sym (Enter_tcon (tcon), ty_idx);
	wn = WN_Lda (Pointer_Mtype, ST_ofst(st), st);
	TREE_STRING_ST (exp) = st;
      }
      break;

    // unary ops
    case BIT_NOT_EXPR:
    case ABS_EXPR:
    case NEGATE_EXPR:
    case REALPART_EXPR:
    case IMAGPART_EXPR:
      {
        wn0 = WFE_Expand_Expr (TREE_OPERAND (exp, 0));
        wn  = WN_Unary (Operator_From_Tree [code].opr,
                        Widen_Mtype(TY_mtype(Get_TY(TREE_TYPE(exp)))), wn0);
      }
      break;

    case TRUTH_NOT_EXPR:
      wn0 = WFE_Expand_Expr (TREE_OPERAND (exp, 0));
      wn1 = WN_Intconst (MTYPE_I4, 0);
      wn  = WN_Relational (OPR_EQ, MTYPE_I4, wn0, wn1);
      break;

    case CONJ_EXPR:
      {
	ty_idx = Get_TY (TREE_TYPE(exp));
        TYPE_ID complex_mtype = TY_mtype(ty_idx);
        TYPE_ID float_mtype   = Mtype_complex_to_real (complex_mtype);
        wn0 = WFE_Expand_Expr (TREE_OPERAND (exp, 0));
	if (WN_has_side_effects (wn0)) {
	  ST       *preg_st;
	  PREG_NUM  preg;
	  preg_st = MTYPE_To_PREG(complex_mtype);
	  preg    = Create_Preg (complex_mtype, NULL);
	  wn0     = WN_Stid (complex_mtype, preg, preg_st, ty_idx, wn0);
	  WFE_Stmt_Append (wn0, Get_Srcpos());
	  wn0 = WN_Ldid (complex_mtype, preg, preg_st, ty_idx);
	}
        wn = WN_Binary (OPR_COMPLEX, complex_mtype,
			WN_Unary (OPR_REALPART, float_mtype, wn0),
			WN_Unary (OPR_NEG, float_mtype,
				  WN_Unary (OPR_REALPART, float_mtype, wn0)));
      }
      break;

    case NOP_EXPR:
      {
	ty_idx = Get_TY (TREE_TYPE(exp));
        TYPE_ID mtyp = TY_mtype(ty_idx);
#ifdef TARG_ST //[CG] do not pass nop_ty_idx
	// do not pass struct type down because will cause rtype of MTYPE_M
        wn = WFE_Expand_Expr (TREE_OPERAND (exp, 0), TRUE,
			      0,
                              component_ty_idx, component_offset,
                              field_id, is_bit_field);
#else
	// do not pass struct type down because will cause rtype of MTYPE_M
        wn = WFE_Expand_Expr (TREE_OPERAND (exp, 0), TRUE, 
			      (mtyp == MTYPE_M) ? 0 : ty_idx,
			       component_ty_idx, component_offset,
			       field_id, is_bit_field);
#endif
	if (mtyp == MTYPE_V) 
	  break;
	if (mtyp == MTYPE_M) 
	  break;

#ifdef TARG_ST
#ifdef STANDARD_C_CONVERSION
	{
	  if (mtyp == MTYPE_M || mtyp == MTYPE_V) break;
	  TY_IDX kid_ty = Get_TY(TREE_TYPE(TREE_OPERAND (exp, 0)));
	  TYPE_ID kid_mtype = TY_mtype(kid_ty);
	  wn = WFE_Cast(mtyp, kid_mtype, wn);
	  break;
	}
#endif
#endif

#ifdef TARG_ST
	//
	// See if we need type conversions:
	//

#if 0
	fprintf(stdout, "  NOP_EXPR: Convert WN MTYPE_%s to MTYPE_%s\n",
		MTYPE_name(WN_rtype(wn)), MTYPE_name(mtyp));
#endif

	if (MTYPE_is_class_integer(mtyp) && 
	    MTYPE_is_class_integer(WN_rtype(wn))) {
	  if (MTYPE_size_min(mtyp) < MTYPE_size_min(WN_rtype(wn))) {
	    // Arthur: we can not do CVTL U8 -> U2 on 32-bit
	    //         machines. Must do CVT U8 -> U4; CVTL U4 -> 16
	    if (Only_32_Bit_Ops && MTYPE_size_min(WN_rtype(wn)) > 32) {
	      wn = WN_Cvt(WN_rtype(wn), Widen_Mtype(mtyp), wn);
	    }
	    // If it is still smaller, add CVTL
	    if (MTYPE_size_min(mtyp) < 32) {
	      wn = WN_CreateCvtl(OPR_CVTL, Widen_Mtype(mtyp), MTYPE_V,
			         MTYPE_size_min(mtyp), wn);
	    }
	  }
	  else {
	    TY_IDX ty_idx0 = Get_TY(TREE_TYPE(TREE_OPERAND (exp, 0)));
	    TYPE_ID mtyp0 = TY_mtype(ty_idx0);

	    // Arthur: following does not work for 32_Bit_Ops
	    if (MTYPE_size_min(mtyp) > MTYPE_size_min(mtyp0) /* [CG] &&
		! Has_Subsumed_Cvtl(WN_operator(wn))*/) {
	      // make a CVTL for subinteger types and CVT for
	      // others

              /* (cbr) don't need to sign extend when orig is call < 32 bit */
              if (TREE_CODE (TREE_OPERAND (exp, 0)) != CALL_EXPR) {
                if (MTYPE_size_min(mtyp) <= 32) {
                  wn = WN_CreateCvtl(OPR_CVTL, Widen_Mtype(mtyp0), MTYPE_V,
                                     MTYPE_size_min(mtyp0), wn);
                }
                else {
                  // if mtyp > 32 bits, and mtyp0 is < 32 and
                  // Only_32_bit_Ops, must travel through a I4
                  if (Only_32_Bit_Ops && (MTYPE_size_min(mtyp0) < 32)) {
                    wn = WN_CreateCvtl(OPR_CVTL, Widen_Mtype(mtyp0), MTYPE_V,
                                       MTYPE_size_min(mtyp0), wn);
                  }
                  // finally, make a CVT
                  wn = WN_Cvt(Widen_Mtype(mtyp0), mtyp, wn);
                }
              }
              else if (MTYPE_size_min(mtyp) > 32) {
                wn = WN_Cvt(Widen_Mtype(mtyp0), mtyp, wn);
              }
            }
#ifdef TARG_ST
            /* (cbr) don't forget the conversion ! */
	    else { // same size
	      if (mtyp != WN_rtype(wn)) 
	        wn = WN_Cvt(WN_rtype(wn), mtyp, wn);
	    }            
#endif
          }
	}
#else /* !TARG_ST */

	if (MTYPE_is_integral(mtyp) && MTYPE_is_integral(WN_rtype(wn))) {
	  if (MTYPE_size_min(mtyp) < MTYPE_size_min(WN_rtype(wn))) {
	    if (MTYPE_size_min(mtyp) != 32)
	      wn = WN_CreateCvtl(OPR_CVTL, Widen_Mtype(mtyp), MTYPE_V,
			         MTYPE_size_min(mtyp), wn);
	    else wn = WN_Cvt(WN_rtype(wn), mtyp, wn);
	  }
	  else {
	    TY_IDX ty_idx0 = Get_TY(TREE_TYPE(TREE_OPERAND (exp, 0)));
	    TYPE_ID mtyp0 = TY_mtype(ty_idx0);

	    if (MTYPE_size_min(mtyp) > MTYPE_size_min(mtyp0) &&
		! Has_Subsumed_Cvtl(WN_operator(wn)))
	      wn = WN_CreateCvtl(OPR_CVTL, Widen_Mtype(mtyp0), MTYPE_V,
				 MTYPE_size_min(mtyp0), wn);

	    if (MTYPE_size_min(mtyp) > MTYPE_size_min(WN_rtype(wn)))
	      wn = WN_Cvt(WN_rtype(wn), mtyp, wn);
	    else { // same size
	      if (mtyp != WN_rtype(wn)) 
	        wn = WN_Cvt(WN_rtype(wn), mtyp, wn);
	    }
	  }
	}
#endif /* TARG_ST */
	else {
	  if (mtyp != WN_rtype(wn)) 
	    wn = WN_Cvt(WN_rtype(wn), mtyp, wn);
	}
      }
      break;

    case COMPONENT_REF:
      {
	INT64 ofst;
	arg0 = TREE_OPERAND (exp, 0);
	arg1 = TREE_OPERAND (exp, 1);

#ifdef TARG_ST
        /* (cbr) handle non_lvalue_expr used for ptrtotmem funcs */
      if (TREE_CODE (arg0) ==  NON_LVALUE_EXPR)
        arg0 = TREE_OPERAND (arg0, 0);
#endif

	// If this is an indirect of a nop_expr, we may need to fix the
	// type of the nop_expr:
	(void) Get_TY(TREE_TYPE(arg0));

	if (component_ty_idx == 0)
	  ty_idx = Get_TY (TREE_TYPE(exp));
	else ty_idx = component_ty_idx;
	if (DECL_BIT_FIELD(arg1)) 
	  is_bit_field = TRUE;

	if (! is_bit_field && 
	    component_ty_idx == 0) {  // only for top-level COMPONENT_REF
          // if size does not agree with ty_idx, fix ty_idx
          tree sizenode = DECL_SIZE(arg1);
          if (TREE_CODE(sizenode) == INTEGER_CST) {
	    TYPE_ID c_mtyp = TY_mtype(ty_idx);
	    INT32 bsize = Get_Integer_Value(sizenode);
	    if (MTYPE_size_min(c_mtyp) > bsize) {
	      FmtAssert(MTYPE_is_integral(c_mtyp), 
	        ("COMPONENT_REF: integer type expected at inconsistent field size"));
	      c_mtyp = Mtype_AlignmentClass(bsize >> 3, MTYPE_type_class(c_mtyp));
	      ty_idx = MTYPE_To_TY(c_mtyp);
	    }
	  }
        }

	if (! is_bit_field)
	  ofst = (BITSPERBYTE * Get_Integer_Value(DECL_FIELD_OFFSET(arg1)) +
			        Get_Integer_Value(DECL_FIELD_BIT_OFFSET(arg1)))
			      / BITSPERBYTE;
	else ofst = 0;
#ifdef TARG_ST
	// [CG]: Propagate down volatile on component ty
	BOOL is_volatile = TREE_THIS_VOLATILE(exp);
	if (is_volatile) Set_TY_is_volatile(ty_idx);
#endif

        wn = WFE_Expand_Expr (arg0, TRUE, nop_ty_idx, ty_idx, ofst+component_offset,
			      field_id + DECL_FIELD_ID(arg1), is_bit_field);
      }
      break;

    case INDIRECT_REF:
      {
	UINT xtra_BE_ofst = 0; 	// only needed for big-endian target
        wn0 = WFE_Expand_Expr (TREE_OPERAND (exp, 0));

#ifdef TARG_ST
	/* [CG]: volatile can be on expression itself or on 
	   type of component. */
	BOOL is_volatile = (TREE_THIS_VOLATILE(exp) ||
			    component_ty_idx != 0 && 
			    TY_is_volatile(component_ty_idx));
#endif

	TY_IDX hi_ty_idx = Get_TY(TREE_TYPE(exp));

	desc_ty_idx = component_ty_idx;
	if (desc_ty_idx == 0)
	  desc_ty_idx = hi_ty_idx;

        if (! MTYPE_is_integral(TY_mtype(desc_ty_idx)))
	  ty_idx = desc_ty_idx;
	else {
	  ty_idx = nop_ty_idx;
	  if (ty_idx == 0) 
	    ty_idx = desc_ty_idx;
	}

	if (! is_bit_field) {
	  if (TY_size(desc_ty_idx) > TY_size(ty_idx)) {
	    if (Target_Byte_Sex == BIG_ENDIAN)
	      xtra_BE_ofst = TY_size(desc_ty_idx) - TY_size(ty_idx);
	    desc_ty_idx = ty_idx;
	  }
	}
	else {
	  if (TY_size(desc_ty_idx) > TY_size(ty_idx)) 
	    ty_idx = desc_ty_idx;
	}

	TYPE_ID rtype = Widen_Mtype(TY_mtype(ty_idx));
	TYPE_ID desc = TY_mtype(desc_ty_idx);
	if (MTYPE_is_integral(desc)) {
	  if (MTYPE_signed(rtype) != MTYPE_signed(desc)) {
	    if (MTYPE_size_min(rtype) > MTYPE_size_min(desc) ||
		is_bit_field)
	      rtype = Mtype_TransferSign(desc, rtype);
	    else desc = Mtype_TransferSign(rtype, desc);
	  }
	}

#ifndef TARG_ST
	if (TREE_THIS_VOLATILE(exp))
	  Set_TY_is_volatile(hi_ty_idx);
#endif

	Is_True(! is_bit_field || field_id <= MAX_FIELD_ID,
		("WFE_Expand_Expr: field id for bit-field exceeds limit"));

        if (!WFE_Keep_Zero_Length_Structs &&
            rtype == MTYPE_M              &&
            TY_size (hi_ty_idx) == 0) {
	  if (WN_has_side_effects (wn0)) {
#ifdef TARG_ST
	    wn = WFE_Append_Expr_Stmt (wn0);
#else
	    wn = WN_CreateEval (wn0);
	    WFE_Stmt_Append (wn, Get_Srcpos());
#endif
	  }
	  wn = NULL;
        }
        else {
	  // special case indexing into a constant string
	  if (WN_operator (wn0) == OPR_LDA          &&
	      ST_class (WN_st (wn0)) == CLASS_CONST &&
	      is_bit_field == FALSE                 &&
	      field_id == 0) {
            st = WN_st (wn0);
	    TCON tcon = Tcon_Table [ST_tcon (st)];
	    if (TCON_ty (tcon) == MTYPE_STRING &&
                TY_size (Be_Type_Tbl (desc)) == 1) {
	      mUINT32 len = Targ_String_Length (tcon);
	      mUINT64 offset = component_offset + xtra_BE_ofst + WN_offset (wn0);
	      if (offset <= len    &&
		  desc == MTYPE_U1 &&
		  (rtype == MTYPE_U4 || rtype == MTYPE_U8)) {
		unsigned char *cp = (unsigned char *) Targ_String_Address (tcon);
		unsigned long long val = cp [offset];
		wn = WN_Intconst (rtype, val);
		break;
	      }
	      else
	      if (offset <= len    &&
		  desc == MTYPE_I1 &&
		  (rtype == MTYPE_I4 || rtype == MTYPE_I8)) {
		signed char *cp = (signed char *) Targ_String_Address (tcon);
		signed long long val = cp [offset];
		wn = WN_Intconst (rtype, val);
		break;
	      }
	    }
	  }
	  if (need_result) {
#ifdef TARG_ST
	  // Arthur: if rtype happens to be a 64-bit type but
	  //         desc is less, and 
	  //         we are in Only_32_Bit_Ops mode, need
	  //         a CVT
	  //
	  TYPE_ID cvt_ty = MTYPE_UNKNOWN;
	  if (Only_32_Bit_Ops &&
	      MTYPE_is_class_integer(rtype) &&
	      MTYPE_is_class_integer(desc)) {

	    if (MTYPE_byte_size(rtype) > 4 && MTYPE_byte_size(desc) <= 4) {
	      cvt_ty = rtype;
	      rtype = MTYPE_signed_type(rtype) ? MTYPE_I4 : MTYPE_U4;
	    }
	  }

	  TY_IDX iload_ty_idx = field_id != 0 ? hi_ty_idx : ty_idx;
	  if (is_volatile) Set_TY_is_volatile(hi_ty_idx);
	  TY_IDX iload_addr_ty_idx = Make_Pointer_Type (hi_ty_idx, FALSE);
	  wn = WN_CreateIload(OPR_ILOAD, rtype,
			      is_bit_field ? MTYPE_BS : desc, 
			      component_offset+xtra_BE_ofst,
			      iload_ty_idx, 
			      iload_addr_ty_idx,
			      wn0, field_id);

	if (TREE_THIS_VOLATILE(exp))
	  DevAssert (WN_Is_Volatile_Mem(wn), 
		     ("Non volatile generated for opcode %s", OPCODE_name(WN_opcode(wn))));

	  // If need a CVT
	  if (cvt_ty != MTYPE_UNKNOWN) {
	    wn = WN_Cvt(rtype, cvt_ty, wn);
	  }
#else
	    wn = WN_CreateIload(OPR_ILOAD, rtype,
				is_bit_field ? MTYPE_BS : desc, 
				component_offset+xtra_BE_ofst,
				field_id != 0 ? hi_ty_idx : ty_idx, 
				Make_Pointer_Type (hi_ty_idx, FALSE),
				wn0, field_id);
#endif /* TARG_ST */
          }
	  else
	  if (WN_has_side_effects (wn0))
	    wn = wn0;
	}
      }
      break;

    case CONVERT_EXPR:
    case FLOAT_EXPR:
      {
        wn0 = WFE_Expand_Expr (TREE_OPERAND (exp, 0));
	ty_idx = Get_TY (TREE_TYPE(exp));
	TYPE_ID mtyp = TY_mtype(ty_idx);
#ifdef TARG_ST
#ifdef STANDARD_C_CONVERSION
	{
	  wn = wn0;
	  if (mtyp == MTYPE_M || mtyp == MTYPE_V) break;
	  TY_IDX kid_ty = Get_TY(TREE_TYPE(TREE_OPERAND (exp, 0)));
	  TYPE_ID kid_mtype = TY_mtype(kid_ty);
	  wn = WFE_Cast(mtyp, kid_mtype, wn0);
	  break;
	}
#endif
#endif

	if (mtyp == MTYPE_V)
	  wn = wn0;
	else {
	  mtyp = Widen_Mtype(TY_mtype(ty_idx));
	  if (mtyp == WN_rtype(wn0) || mtyp == MTYPE_V)
	    wn = wn0;
	  else {
	    wn = WN_Cvt(WN_rtype(wn0), mtyp, wn0);
	    // The following opcodes are not valid for MIPS
	    if (WN_opcode(wn) == OPC_I4U4CVT ||
	        WN_opcode(wn) == OPC_U4I4CVT ||
	        WN_opcode(wn) == OPC_I8U8CVT ||
	        WN_opcode(wn) == OPC_U8I8CVT) {
	      wn = WN_kid0 (wn);
	    }
	  }
	}
      }
      break;

    case FIX_TRUNC_EXPR:
      {
        wn0 = WFE_Expand_Expr (TREE_OPERAND (exp, 0));
	ty_idx = Get_TY (TREE_TYPE(exp));
#ifdef TARG_ST
#ifdef STANDARD_C_CONVERSION
	{
	  TYPE_ID mtyp = TY_mtype(ty_idx);
	  wn = wn0;
	  if (mtyp == MTYPE_M || mtyp == MTYPE_V) break;
	  TY_IDX kid_ty = Get_TY(TREE_TYPE(TREE_OPERAND (exp, 0)));
	  TYPE_ID kid_mtype = TY_mtype(kid_ty);
	  wn = WFE_Cast(mtyp, kid_mtype, wn0);
	  break;
	}
#endif
#endif
	TYPE_ID mtyp = Widen_Mtype(TY_mtype(ty_idx));
	wn = WN_Trunc(WN_rtype(wn0), mtyp, wn0);
      }
      break;

#ifdef GPLUSPLUS_FE
    case EXPR_STMT:
      {
#ifdef TARG_ST
        /* (cbr) need result */
	wn = WFE_Expand_Expr (EXPR_STMT_EXPR(exp), need_result);
#else
	wn = WFE_Expand_Expr (EXPR_STMT_EXPR(exp), false);
#endif
      }
      break;

    case STMT_EXPR:
      {
        wn = WFE_Expand_Expr (TREE_OPERAND (exp, 0), need_result);
      }
      break;

#if defined (TARG_ST) && (GNU_FRONT_END==295)
      /* (cbr) gcc 3.3 upgrade */
      case SUBOBJECT:
      break;
#endif

    case CLEANUP_POINT_EXPR: 
      {
        Push_Temp_Cleanup(exp, false);
        wn = WFE_Expand_Expr (TREE_OPERAND (exp, 0));
        WN * cleanup_block = WN_CreateBlock ();
        WFE_Stmt_Push (cleanup_block, wfe_stmk_temp_cleanup, Get_Srcpos ());
        Do_Temp_Cleanups(exp);
        WFE_Stmt_Pop (wfe_stmk_temp_cleanup);
	if (wn && WN_has_side_effects (wn) && WN_first (cleanup_block)) {
	  DevWarn("CLEANUP_POINT_EXPR: expressson has side effects");
	  ty_idx = Get_TY (TREE_TYPE(exp));
	  st = Gen_Temp_Symbol (ty_idx, "__cleanup_point_expr");
	  TYPE_ID mtype = TY_mtype (ty_idx);
	  WFE_Set_ST_Addr_Saved (wn);
	  wn = WN_Stid (mtype, 0, st, ty_idx, wn);
	  WFE_Stmt_Append (wn, Get_Srcpos ());
	  wn = WN_Ldid (mtype, 0, st, ty_idx);
	}
	WFE_Stmt_Append (cleanup_block, Get_Srcpos ());
      }
      break;

    case THROW_EXPR:
      WFE_One_Stmt (TREE_OPERAND (exp, 0));
      Call_Throw();
      break;

    case TRY_CATCH_EXPR:
      DevWarn ("Encountered TRY_CATCH_EXPR at line %d:  ignored", lineno);
      break;

    case COMPOUND_STMT:
      {
	tree t = COMPOUND_BODY(exp);
	tree last_expr_stmt = 0;
	wn = NULL;
	while (t) {
	  if (TREE_CODE(t) == (enum tree_code)EXPR_STMT)
	    last_expr_stmt = t;
	  t = TREE_CHAIN(t);
	}

	t = COMPOUND_BODY(exp);

	while (t != last_expr_stmt) {
	  WFE_Expand_Stmt (t);
	  t = TREE_CHAIN(t);
	}

	if (t) {
	  wn =  WFE_Expand_Expr(t, need_result);
	  t = TREE_CHAIN(t);
	}

	while (t) {
	  WFE_Expand_Stmt(t);
	  t = TREE_CHAIN(t);
	}

      }
      break;

    case EMPTY_CLASS_EXPR:
      DevWarn ("Encountered EMPTY_CLASS_EXPR at line %d\n", lineno);
      ty_idx = Get_TY (TREE_TYPE(exp));
      st = Gen_Temp_Symbol (ty_idx, "__empty_class_expr");
      wn = WN_Ldid (TY_mtype (ty_idx), 0, st, ty_idx);
      break;
#endif /* GLPUSPLUFE */

    // binary ops
    case PLUS_EXPR:
    case MINUS_EXPR:
    case MULT_EXPR:
    case MAX_EXPR:
    case MIN_EXPR:
    case LSHIFT_EXPR:
    case BIT_AND_EXPR:
    case BIT_IOR_EXPR:
    case BIT_XOR_EXPR:
    case TRUNC_DIV_EXPR:
    case TRUNC_MOD_EXPR:
    case RDIV_EXPR:
    case TRUTH_AND_EXPR:
    case TRUTH_OR_EXPR:
    case TRUTH_XOR_EXPR:
    case COMPLEX_EXPR:
    case CEIL_DIV_EXPR:
      {
#ifdef TARG_ST
        /* (cbr) easier for debug */
        TYPE_ID etype = TY_mtype(Get_TY(TREE_TYPE(exp)));
#endif
        wn0 = WFE_Expand_Expr (TREE_OPERAND (exp, 0));
        wn1 = WFE_Expand_Expr (TREE_OPERAND (exp, 1));
#ifdef TARG_ST
        /* (cbr)  */
        wn  = WN_Binary (Operator_From_Tree [code].opr,
                         Widen_Mtype(etype), wn0, wn1);
#else
        wn  = WN_Binary (Operator_From_Tree [code].opr,
                         Widen_Mtype(TY_mtype(Get_TY(TREE_TYPE(exp)))), wn0, wn1);
#endif
      }
      break;
// [HK] add special treatment for EXACT_DIV_EXPR case

    case EXACT_DIV_EXPR:
    {
    	TYPE_ID etype = TY_mtype(Get_TY(TREE_TYPE(exp)));
	INT64 val = Get_Integer_Value(TREE_OPERAND (exp, 1));
	wn0 = WFE_Expand_Expr (TREE_OPERAND (exp, 0));
#ifdef TARG_ST
	if ((MTYPE_is_class_integer(etype)) &&
#else
	if ((MTYPE_is_integral(etype)) &&
#endif
	    (Is_Power_Of_2(val, etype)))
	{ 
	   TYPE_ID mtyp = Widen_Mtype(etype);
	   wn1 = WN_Intconst (mtyp, Get_Power_Of_2(val, mtyp));
	   wn  = WN_Binary (MTYPE_signed(mtyp) ? OPR_ASHR : OPR_LSHR,
                         mtyp, wn0, wn1);
	}
	else
	{     
	   wn1 = WFE_Expand_Expr (TREE_OPERAND (exp, 1));
	   wn  = WN_Binary (Operator_From_Tree [code].opr, Widen_Mtype(etype), wn0, wn1);

#ifdef TARG_ST
	   if ((MTYPE_is_class_integer(etype)) &&
#else
	   if ((MTYPE_is_integral(etype)) &&
#endif
	    (Widen_Mtype(etype) != etype) &&
	    (TY_size (Get_TY(TREE_TYPE(exp))) < 32) &&
	    (code == PLUS_EXPR || code == MINUS_EXPR || code == MULT_EXPR))
	       wn = WN_CreateCvtl(OPR_CVTL, Widen_Mtype(etype), MTYPE_V,
			     TY_size (Get_TY(TREE_TYPE(exp))) * 8, wn);
	}			     
      }
      break;


    case LROTATE_EXPR:
      {
        wn0 = WFE_Expand_Expr (TREE_OPERAND (exp, 0));
        wn1 = WFE_Expand_Expr (TREE_OPERAND (exp, 1));
	wn  = WN_Lrotate (TY_mtype(Get_TY(TREE_TYPE(exp))), wn0, wn1);
      }
      break;

    case RROTATE_EXPR:
      {
        wn0 = WFE_Expand_Expr (TREE_OPERAND (exp, 0));
        wn1 = WFE_Expand_Expr (TREE_OPERAND (exp, 1));
	wn  = WN_Rrotate (TY_mtype(Get_TY(TREE_TYPE(exp))), wn0, wn1);
      }
      break;

    case RSHIFT_EXPR:
      {
	ty_idx = Get_TY(TREE_TYPE(exp));
	TYPE_ID mtyp = Widen_Mtype(TY_mtype(Get_TY(TREE_TYPE(exp))));
        wn0 = WFE_Expand_Expr (TREE_OPERAND (exp, 0));
        wn1 = WFE_Expand_Expr (TREE_OPERAND (exp, 1));
        wn  = WN_Binary (MTYPE_signed(mtyp) ? OPR_ASHR : OPR_LSHR,
                         mtyp, wn0, wn1);
      }
      break;

    case TRUTH_ANDIF_EXPR:
    case TRUTH_ORIF_EXPR:
      {
#ifdef TARG_ST
        /* (cbr) support for deferred cleanups */
        Push_Cleanup_Deferral ();
#endif

        wn0 = WFE_Expand_Expr_With_Sequence_Point (TREE_OPERAND (exp, 0),
						   Boolean_type);

#ifdef TARG_ST
        /* (cbr) support for deferred cleanups */
        Push_Cleanup_Deferral ();
#endif

        wn1 = WFE_Expand_Expr_With_Sequence_Point (TREE_OPERAND (exp, 1),
						   Boolean_type);

        wn  = WN_Binary (Operator_From_Tree [code].opr,
                         Boolean_type, wn0, wn1);
        if (Boolean_type != MTYPE_B &&
	    Widen_Mtype(TY_mtype(Get_TY(TREE_TYPE(exp)))) != Boolean_type)
	  wn = WN_Cvt (Boolean_type, Widen_Mtype(TY_mtype(Get_TY(TREE_TYPE(exp)))), wn);

      }
      break;

#if defined (TARG_ST) && (GNU_FRONT_END==33)
  /* (cbr) that should be safe to map those since we don't have exceptions anyway */
    case UNLT_EXPR:
    case UNLE_EXPR:
    case UNGT_EXPR:
    case UNGE_EXPR:
    case UNEQ_EXPR:
#endif
    case LT_EXPR:
    case LE_EXPR:
    case GT_EXPR:
    case GE_EXPR:
    case EQ_EXPR:
    case NE_EXPR:
      {
        wn0 = WFE_Expand_Expr (TREE_OPERAND (exp, 0));
        wn1 = WFE_Expand_Expr (TREE_OPERAND (exp, 1));

	// check if conversion is needed
	ty_idx = Get_TY (TREE_TYPE(exp));
        TYPE_ID mtyp = TY_mtype(ty_idx);
	TY_IDX ty_idx0 = Get_TY(TREE_TYPE(TREE_OPERAND (exp, 0)));
	TYPE_ID mtyp0 = TY_mtype(ty_idx0);
	TY_IDX ty_idx1 = Get_TY(TREE_TYPE(TREE_OPERAND (exp, 1)));
	TYPE_ID mtyp1 = TY_mtype(ty_idx1);
#ifdef TARG_ST
	TYPE_ID op_mtype = mtyp0;
#endif
#ifdef STANDARD_C_CONVERSION
	{
	  TYPE_ID promoted_mtype = WFE_Promoted_Type(mtyp);
	  TYPE_ID promoted_binary_mtype = WFE_Promoted_Binary_Type(mtyp0, mtyp1);
	  wn0 = WFE_Cast(promoted_binary_mtype, mtyp0, wn0);
	  wn1 = WFE_Cast(promoted_binary_mtype, mtyp1, wn1);
	  
	  wn = WN_CreateExp2(Operator_From_Tree [code].opr, 
			     promoted_mtype,
			     promoted_binary_mtype, wn0, wn1);
	  break;
	}
#endif

	if (MTYPE_size_min(mtyp1) > MTYPE_size_min(mtyp0)
#ifndef TARG_ST 
	    && ! Has_Subsumed_Cvtl(WN_operator(wn0))
#endif
            )
#ifdef TARG_ST
	if (Only_32_Bit_Ops) {
	  op_mtype = mtyp1;
	  wn0 = WN_CreateCvtl(OPR_CVTL, Widen_Mtype(mtyp0), MTYPE_V,
			      MTYPE_size_min(mtyp0), wn0);
	  // [CG] May need a CVT
	  if (MTYPE_size_min(mtyp1) > 32 && MTYPE_size_min(mtyp0) <= 32) {
	    wn0 = WN_Cvt(Widen_Mtype(mtyp0), mtyp1, wn0);
	  }
	} else {
	  op_mtype = mtyp1;
	  wn0 = WN_CreateCvtl(OPR_CVTL, Widen_Mtype(mtyp0), MTYPE_V,
			      MTYPE_size_min(mtyp0), wn0);
	}
#else
	  wn0 = WN_CreateCvtl(OPR_CVTL, Widen_Mtype(mtyp0), MTYPE_V,
			      MTYPE_size_min(mtyp0), wn0);
#endif
	if (MTYPE_size_min(mtyp0) > MTYPE_size_min(mtyp1)
#ifndef TARG_ST 
	    && ! Has_Subsumed_Cvtl(WN_operator(wn1))
#endif
	    )
#ifdef TARG_ST
	if (Only_32_Bit_Ops) {
	  op_mtype = mtyp0;
	  wn1 = WN_CreateCvtl(OPR_CVTL, Widen_Mtype(mtyp1), MTYPE_V,
			      MTYPE_size_min(mtyp1), wn1);
	  // [CG] May need a CVT
	  if (MTYPE_size_min(mtyp0) > 32 && MTYPE_size_min(mtyp1) <= 32) {
	    wn1 = WN_Cvt(Widen_Mtype(mtyp1), mtyp0, wn1);
	  }
	} else {
	  op_mtype = mtyp0;
	  wn1 = WN_CreateCvtl(OPR_CVTL, Widen_Mtype(mtyp1), MTYPE_V,
			      MTYPE_size_min(mtyp1), wn0);
	}
#else
	  wn1 = WN_CreateCvtl(OPR_CVTL, Widen_Mtype(mtyp1), MTYPE_V,
			      MTYPE_size_min(mtyp1), wn1);
#endif


#ifdef TARG_ST
	// Arthur: if Only_32_Bit_Ops, need a compare I4I4XXX and
	//         check if if result mtyp > 32 bits, add a CVT.
	//
	// We allow I4I4EQ and I4I8EQ, but not I8I4EQ not I8I8EQ
	if (Only_32_Bit_Ops) {

	  // I assume mtyp is an inetger or boolean
	  Is_True(MTYPE_is_class_integer(mtyp) ||
		  MTYPE_is_class_boolean(mtyp),("mtype"));

	  wn = WN_CreateExp2(Operator_From_Tree [code].opr, 
			     MTYPE_TransferSize(4, mtyp),
			     Widen_Mtype(op_mtype), 
			     wn0, 
			     wn1);
	  // need a CVT ?
	  if (MTYPE_size_min(mtyp) > 32) {
	    wn = WN_Cvt(WN_rtype(wn), mtyp, wn);
	  }
	}
	else {
        wn  = WN_Relational (Operator_From_Tree [code].opr,
			     Widen_Mtype(TY_mtype(Get_TY(TREE_TYPE(TREE_OPERAND(exp, 0))))),
			     wn0, wn1);
	}
#else
        wn  = WN_Relational (Operator_From_Tree [code].opr,
			     Widen_Mtype(TY_mtype(Get_TY(TREE_TYPE(TREE_OPERAND(exp, 0))))),
			     wn0, wn1);
#endif
      }
      break;

    case COND_EXPR:
      {
        TY_IDX ty_idx1 = Get_TY (TREE_TYPE(TREE_OPERAND (exp, 1)));
        TY_IDX ty_idx2 = Get_TY (TREE_TYPE(TREE_OPERAND (exp, 2)));
	ty_idx = Get_TY (TREE_TYPE(exp));
	wn0 = WFE_Expand_Expr_With_Sequence_Point (TREE_OPERAND (exp, 0),
						   Boolean_type);

#ifdef TARG_ST
        /* (cbr) pro-release-1-9-0-B/26 select test is always a boolean */
        TY_IDX test_idx = TY_mtype(Get_TY(TREE_TYPE(TREE_OPERAND(exp,0))));
        if (MTYPE_is_longlong(test_idx))
          wn0 = WN_Cvt(test_idx, Boolean_type, wn0);
#endif

	if (TY_mtype (ty_idx)  == MTYPE_V ||
            TY_mtype (ty_idx1) == MTYPE_V ||
            TY_mtype (ty_idx2) == MTYPE_V) {
	  WN *then_block = WN_CreateBlock ();
	  WN *else_block = WN_CreateBlock ();
	  WN *if_stmt    = WN_CreateIf (wn0, then_block, else_block);
	  WFE_Stmt_Append (if_stmt, Get_Srcpos());
	  WFE_Stmt_Push (then_block, wfe_stmk_if_then, Get_Srcpos());
#ifdef TARG_ST
          /* (cbr) pro-release-1-9-0-B/6 need if throw_expr part of the conditiol assignment */
	  wn1 = WFE_Expand_Expr (TREE_OPERAND (exp, 1), need_result);
#else
	  wn1 = WFE_Expand_Expr (TREE_OPERAND (exp, 1), FALSE);
#endif
	  if (wn1) {
#ifdef TARG_ST
	    wn1 = WFE_Append_Expr_Stmt (wn1);
#else
	    wn1 = WN_CreateEval (wn1);
	    WFE_Stmt_Append (wn1, Get_Srcpos());
#endif
	  }
	  WFE_Stmt_Pop (wfe_stmk_if_then);
	  WFE_Stmt_Push (else_block, wfe_stmk_if_else, Get_Srcpos());
#ifdef TARG_ST
          /* (cbr) need if throw_expr part of the conditiol assignment */
	  wn2 = WFE_Expand_Expr (TREE_OPERAND (exp, 2), need_result);
#else
	  wn2 = WFE_Expand_Expr (TREE_OPERAND (exp, 2), FALSE);
#endif
	  if (wn2) {
#ifdef TARG_ST
	    wn2 = WFE_Append_Expr_Stmt (wn2);
#else
	    wn2 = WN_CreateEval (wn2);
	    WFE_Stmt_Append (wn2, Get_Srcpos());
#endif
	  }
	  WFE_Stmt_Pop (wfe_stmk_if_else);
        }
	else {
#ifdef TARG_ST
        /* (cbr) support for deferred cleanups */
          Push_Cleanup_Deferral ();
#endif
	  wn1 = WFE_Expand_Expr_With_Sequence_Point (TREE_OPERAND (exp, 1),
						     TY_mtype (ty_idx));

#ifdef TARG_ST
        /* (cbr) support for deferred cleanups */
          Push_Cleanup_Deferral ();
#endif
	  wn2 = WFE_Expand_Expr_With_Sequence_Point (TREE_OPERAND (exp, 2),
						     TY_mtype (ty_idx));

	  wn  = WN_CreateExp3 (OPR_CSELECT, Mtype_comparison (TY_mtype (ty_idx)),
			   MTYPE_V, wn0, wn1, wn2);

	  Set_PU_has_very_high_whirl (Get_Current_PU ());
        }
      }
      break;

    case INIT_EXPR:
    case MODIFY_EXPR:
      /*
       * When operand 1 of an init_expr or modify_expr is a target_expr,
       * then the temporary in the target_expr needs to be replaced by
       * operand 1 of the init_expr or modify_expr and the cleanup
       * (operand 2) of the target_expr needs to be zeroed out, since
       * no temporary will be generated so none should be destroyed.
       */
      {
      tree te = TREE_OPERAND(exp, 1);

#if defined (TARG_ST) && (GNU_FRONT_END==33)
      if (TREE_CODE(te) == (enum tree_code)MUST_NOT_THROW_EXPR) {
	tree t = TREE_OPERAND(te, 0);
	TREE_OPERAND(t, 2) = NULL_TREE;
      }
#endif

      if (TREE_CODE(te) == TARGET_EXPR) {
	tree t = te;
	TREE_OPERAND(t, 2) = NULL_TREE;

#if defined (TARG_ST) && (GNU_FRONT_END==33)
        /* (cbr) avoid creating a temporary object for ctors */
        if (TREE_CODE (TREE_OPERAND(exp, 0)) == INDIRECT_REF && 
            (TREE_CODE (TREE_OPERAND ( TREE_OPERAND(exp, 0),0)) == VAR_DECL ||
             TREE_CODE (TREE_OPERAND ( TREE_OPERAND(exp, 0),0)) == PARM_DECL)) {
          tree t1 = TREE_OPERAND(t, 1);
          tree arg = TREE_OPERAND (TREE_OPERAND(exp, 0), 0);
          if (TREE_CODE (t1) == COMPOUND_EXPR) {
            tree exp2 = TREE_OPERAND(t1, 0);
            if (TREE_CODE (exp2) == CALL_EXPR) {
              tree callop = TREE_VALUE(TREE_OPERAND(exp2,1));
              if (TREE_CODE (callop) == ADDR_EXPR) {
                TREE_VALUE(TREE_OPERAND(exp2,1)) = arg;                
                wn = WFE_Expand_Expr(t);
                break;
              }
            }
          }
        }
        else 
#endif
	if (TREE_CODE(TREE_OPERAND(exp, 0)) == VAR_DECL    ||
	    TREE_CODE(te) == RESULT_DECL ||
	    TREE_CODE(TREE_OPERAND(exp, 0)) == PARM_DECL) {
 	  TREE_OPERAND(t, 0) = TREE_OPERAND(exp, 0);
	  wn = WFE_Expand_Expr(t);
	  break;
	}
      }

	DevWarn ("INIT_EXPR/MODIFY_EXPR kid1 is TARGET_EXPR, kid0 is %s\n",
		 Operator_From_Tree [TREE_CODE(TREE_OPERAND(exp, 0))].name);
      }
		
    case PREDECREMENT_EXPR:
    case PREINCREMENT_EXPR:
    case POSTDECREMENT_EXPR:
    case POSTINCREMENT_EXPR:
      {
	if (TREE_CODE(TREE_OPERAND(exp, 1)) == ERROR_MARK)
	    break;
        wn1 = WFE_Expand_Expr (TREE_OPERAND (exp, 1)); // r.h.s.
#ifdef TARG_ST
        /* (cbr) it's possible that the return type is not of the type of
           modified expression. Treat it separatly
        */
        if (component_ty_idx && need_result) {
          (void) WFE_Lhs_Of_Modify_Expr(code, TREE_OPERAND (exp, 0), FALSE, 
                                        0, 0, 0, FALSE, wn1, 0, FALSE, FALSE);

          wn = WFE_Expand_Expr (TREE_OPERAND (exp, 0), TRUE, 0,
                                component_ty_idx, component_offset, field_id,
                                is_bit_field); 
        }
        else 
#endif
	wn  = WFE_Lhs_Of_Modify_Expr(code, TREE_OPERAND (exp, 0), need_result, 
				     0, 0, 0, FALSE, wn1, 0, FALSE, FALSE);
      }
      break;

    // ternary ops

    case BIT_FIELD_REF:
      {
        wn = WFE_Expand_Expr (TREE_OPERAND (exp, 0), TRUE, nop_ty_idx, 
			      component_ty_idx, component_offset,
			      field_id, FALSE);
	ty_idx = Get_TY (TREE_TYPE(exp));
	TYPE_ID rtype = TY_mtype(ty_idx);
	UINT siz = TY_size(ty_idx);
	TYPE_ID desc;
	if (siz <= 8) {
	  if (MTYPE_signed(rtype))
	    desc = Mtype_AlignmentClass(siz, MTYPE_CLASS_INTEGER);
	  else desc = Mtype_AlignmentClass(siz, MTYPE_CLASS_UNSIGNED_INTEGER);
	  rtype = Widen_Mtype(desc);
	}
	else desc = rtype;
	WN_set_rtype(wn, rtype);
	WN_set_desc(wn, desc);
	INT bofst = Get_Integer_Value(TREE_OPERAND(exp, 2));
	INT bsiz =Get_Integer_Value(TREE_OPERAND(exp, 1));
	if ((bsiz & 7) == 0 &&	// field size multiple of bytes
	    MTYPE_size_min(desc) % bsiz == 0 && // accessed loc multiple of bsiz
	    bofst % bsiz == 0) {		// bofst multiple of bsiz
	  // not really a bit-field extraction!
	  if (MTYPE_signed(rtype))
	    WN_set_desc(wn, Mtype_AlignmentClass(bsiz >> 3, MTYPE_CLASS_INTEGER));
	  else WN_set_desc(wn, Mtype_AlignmentClass(bsiz >> 3, MTYPE_CLASS_UNSIGNED_INTEGER));
	  WN_load_offset(wn) = WN_load_offset(wn) + (bofst >> 3);
	} else {
	  if (WN_operator(wn) == OPR_LDID)
	    WN_set_operator(wn, OPR_LDBITS);
	  else WN_set_operator(wn, OPR_ILDBITS);
	  WN_set_bit_offset_size(wn, bofst, bsiz);
	}
	if (MTYPE_byte_size (WN_desc(wn)) != TY_size(WN_ty(wn)))
	  // the container is smaller than the entire struct
	  WN_set_ty (wn, MTYPE_To_TY (WN_desc(wn)));
      }
      break;

    // n-ary ops

    case ARRAY_REF:
      {
	UINT xtra_BE_ofst = 0; 	// only needed for big-endian target
	TY_IDX elem_ty_idx;
	// generate the WHIRL array node
        wn0 = WFE_Array_Expr(exp, &elem_ty_idx, 0, 0, 0);

	// generate the iload node
	TY_IDX hi_ty_idx = Get_TY (TREE_TYPE(exp));
	desc_ty_idx = component_ty_idx;
	if (desc_ty_idx == 0)
          desc_ty_idx = hi_ty_idx;

        if (! MTYPE_is_integral(TY_mtype(desc_ty_idx)))
	  ty_idx = desc_ty_idx;
	else {
	  ty_idx = nop_ty_idx;
	  if (ty_idx == 0) 
	    ty_idx = desc_ty_idx;
	}

	if (! is_bit_field) {
	  if (TY_size(desc_ty_idx) > TY_size(ty_idx)) {
	    if (Target_Byte_Sex == BIG_ENDIAN)
	      xtra_BE_ofst = TY_size(desc_ty_idx) - TY_size(ty_idx);
	    desc_ty_idx = ty_idx;
	  }
	}
        else {
          if (TY_size(desc_ty_idx) > TY_size(ty_idx))
            ty_idx = desc_ty_idx;
        }

        TYPE_ID rtype = Widen_Mtype(TY_mtype(ty_idx));
        TYPE_ID desc = TY_mtype(desc_ty_idx);
        if (MTYPE_is_integral(desc)) {
          if (MTYPE_signed(rtype) != MTYPE_signed(desc)) {
            if (MTYPE_size_min(rtype) > MTYPE_size_min(desc) ||
		is_bit_field)
              rtype = Mtype_TransferSign(desc, rtype);
            else desc = Mtype_TransferSign(rtype, desc);
          }
        }

	Is_True(! is_bit_field || field_id <= MAX_FIELD_ID,
		("WFE_Expand_Expr: field id for bit-field exceeds limit"));
	wn = WN_CreateIload(OPR_ILOAD, rtype,
			    is_bit_field ? MTYPE_BS : desc, 
			    component_offset+xtra_BE_ofst,
			    field_id != 0 ? hi_ty_idx : ty_idx,
			    Make_Pointer_Type(elem_ty_idx, FALSE),
			    wn0, field_id);
      }
      break;

    case AGGR_INIT_EXPR:
    case CALL_EXPR:
      {
	tree arglist = TREE_OPERAND (exp, 1);
        TYPE_ID ret_mtype;
        WN *call_wn;
        WN *arg_wn;
	TY_IDX  arg_ty_idx;
        TYPE_ID arg_mtype;
        INT num_args = 0;
	INT num_handlers = 0;
        INT i;
	tree list;
	arg0 = TREE_OPERAND (exp, 0);
	enum tree_code code0 = TREE_CODE (arg0);

	for (list = TREE_OPERAND (exp, 1); list; list = TREE_CHAIN (list)) {
          arg_ty_idx = Get_TY(TREE_TYPE(TREE_VALUE(list)));
          if (!WFE_Keep_Zero_Length_Structs    &&
              TY_mtype (arg_ty_idx) == MTYPE_M &&
              TY_size (arg_ty_idx) == 0) {
            // zero length struct parameter
          }
          else
            num_args++;
        }
        ty_idx = Get_TY(TREE_TYPE(exp));
        if (need_result) {
          if (!WFE_Keep_Zero_Length_Structs  &&
              TY_mtype (ty_idx) == MTYPE_M   &&
              TY_size (ty_idx) == 0) {
            // zero length struct return
            ret_mtype = MTYPE_V;
          }
          else
            ret_mtype = TY_mtype (ty_idx);
        }
        else
          ret_mtype = MTYPE_V;
        st = NULL;
        if (code0 == ADDR_EXPR                  &&
            TREE_CODE (TREE_OPERAND (arg0, 0))) {
	  tree func = TREE_OPERAND (arg0, 0);
	  BOOL intrinsic_op = FALSE;
          BOOL whirl_generated = FALSE;
	  INTRINSIC iopc = INTRINSIC_NONE;
          
#if defined (TARG_ST) && (GNU_FRONT_END==33)
          /* (cbr) gcc 3.3 upgrade */
          if (DECL_BUILT_IN_CLASS (func)) {
#else
            if (DECL_BUILT_IN (func)) {
#endif

            switch (DECL_FUNCTION_CODE (func)) {

	      case END_BUILTINS:
		break;

	      case BUILT_IN_STDARG_START:
	      {
		arg1 = TREE_VALUE (arglist);
		arg2 = TREE_VALUE (TREE_CHAIN (arglist));
		WN *arg_wn = WFE_Expand_Expr (arg1);
		ST *st1 = WN_st (arg_wn);
		while (TREE_CODE (arg2) == NOP_EXPR
		       || TREE_CODE (arg2) == CONVERT_EXPR
		       || TREE_CODE (arg2) == NON_LVALUE_EXPR
		       || TREE_CODE (arg2) == INDIRECT_REF)
		  arg2 = TREE_OPERAND (arg2, 0);
		ST *st2 = Get_ST (arg2);
		wn = WN_Lda (Pointer_Mtype, 
                             ((TY_size (ST_type (st2)) + 7) & (-8)),
                             st2);
		wn = WN_Stid (Pointer_Mtype, 0, st1, ST_type (st1), wn);
		WFE_Stmt_Append (wn, Get_Srcpos());
		whirl_generated = TRUE;
		wn = NULL;
		break;
	      }

	      case BUILT_IN_VA_END:
	      {
		whirl_generated = TRUE;
		break;
	      }

	      case BUILT_IN_NEXT_ARG:
	      {
                tree last_parm = tree_last 
				   (DECL_ARGUMENTS (Current_Function_Decl()));
		while (TREE_CODE (last_parm) == NOP_EXPR
		       || TREE_CODE (last_parm) == CONVERT_EXPR
		       || TREE_CODE (last_parm) == NON_LVALUE_EXPR
		       || TREE_CODE (last_parm) == INDIRECT_REF)
		  last_parm = TREE_OPERAND (last_parm, 0);
		ST *st = Get_ST (last_parm);
		arg_wn = WN_Lda (Pointer_Mtype, ST_ofst(st), st);
		wn = WN_Binary (OPR_ADD, Pointer_Mtype, arg_wn,
				WN_Intconst (Pointer_Mtype,
					     Parameter_Size(ST_size(st))));
                whirl_generated = TRUE;
		break;
	      }

              case BUILT_IN_ALLOCA:
		Set_PU_has_alloca (Get_Current_PU ());
		Set_PU_has_user_alloca (Get_Current_PU ());
                arg_wn = WFE_Expand_Expr (TREE_VALUE (TREE_OPERAND (exp, 1)));
	        wn = WN_CreateAlloca (arg_wn);
                whirl_generated = TRUE;
                break;

              case BUILT_IN_MEMCPY:
		iopc = INTRN_MEMCPY;
                break;

              case BUILT_IN_MEMCMP:
		iopc = INTRN_MEMCMP;
                break;

              case BUILT_IN_MEMSET:
		iopc = INTRN_MEMSET;
                break;

              case BUILT_IN_STRCPY:
		iopc = INTRN_STRCPY;
                break;

              case BUILT_IN_STRCMP:
#ifdef GPLUSPLUS_FE
		iopc = INTRN_STRCMP;
#else
		if (arglist == 0
		    /* Arg could be non-pointer if user redeclared this fcn wrong.  */
		    || TREE_CODE (TREE_TYPE (TREE_VALUE (arglist))) != POINTER_TYPE
		    || TREE_CHAIN (arglist) == 0
		    || TREE_CODE (TREE_TYPE (TREE_VALUE (TREE_CHAIN (arglist)))) != POINTER_TYPE)
		  break;
		else {
		  arg1 = TREE_VALUE (arglist);
		  arg2 = TREE_VALUE (TREE_CHAIN (arglist));
		  tree len1 = c_strlen (arg1);
		  if (Enable_Expand_Builtin && len1) {
		    tree len2 = c_strlen (arg2);
		    if (len2) {
		      char *ptr1 = get_string_pointer (WFE_Expand_Expr (arg1));
		      char *ptr2 = get_string_pointer (WFE_Expand_Expr (arg2));
		      if (ptr1 && ptr2) {
			wn = WN_Intconst (MTYPE_I4,
					  strcmp (ptr1, ptr2));
			whirl_generated = TRUE;
			break;
		      }
		    }
		  }
		  iopc = INTRN_STRCMP;
//		  intrinsic_op = TRUE;
		}
#endif /* GPLUSPLUS_FE */
                break;

              case BUILT_IN_STRLEN:
#ifdef GPLUSPLUS_FE
		iopc = INTRN_STRLEN;
#else
		if (arglist == 0
		/* Arg could be non-pointer if user redeclared this fcn wrong.  */
		   || TREE_CODE (TREE_TYPE (TREE_VALUE (arglist))) != POINTER_TYPE)
		  break;
		else {
		  tree src = TREE_VALUE (arglist);
		  tree len = c_strlen (src);
		  if (Enable_Expand_Builtin && len) {
		    wn = WFE_Expand_Expr (len);
		    whirl_generated = TRUE;
		  }
		  else {
		    iopc = INTRN_STRLEN;
//		    intrinsic_op = TRUE;
		  }
		}
#endif /* GPLUSPLUS_FE */
                break;

#if defined (TARG_ST) 
#if GNU_FRONT_END==295
              case BUILT_IN_FSQRT:
#else
                /* (cbr) gcc 3.3 upgrade */
              case BUILT_IN_SQRTF:
              case BUILT_IN_SQRT:
#endif
#ifdef TARG_ST
		// sometimes it is called sqrtf(x), no result_needed
		// generate a normal call
		if (ret_mtype == MTYPE_V) break;
#endif
		  /* [HK] only treated as built-ins if -fno-math-errno is true */ 
		if ( No_Math_Errno ) {
		    arg_wn = WFE_Expand_Expr (TREE_VALUE (TREE_OPERAND (exp, 1)));
		    wn = WN_CreateExp1 (OPR_SQRT, ret_mtype, MTYPE_V, arg_wn);
		    whirl_generated = TRUE;
		}
                break;
#endif

              case BUILT_IN_SIN:
		     if (ret_mtype == MTYPE_F4) iopc = INTRN_F4SIN;
                else if (ret_mtype == MTYPE_F8) iopc = INTRN_F8SIN;
                else Fail_FmtAssertion ("unexpected mtype for intrinsic 'sin'");
		intrinsic_op = TRUE;
                break;

              case BUILT_IN_COS:
		     if (ret_mtype == MTYPE_F4) iopc = INTRN_F4COS;
                else if (ret_mtype == MTYPE_F8) iopc = INTRN_F8COS;
                else Fail_FmtAssertion ("unexpected mtype for intrinsic 'cos'");
		intrinsic_op = TRUE;
                break;

              case BUILT_IN_CONSTANT_P:
              {
#ifndef TARG_ST
		DevWarn ("Encountered BUILT_IN_CONSTANT_P: at line %d\n",
                         lineno);
#endif
                tree arg = TREE_VALUE (TREE_OPERAND (exp, 1));
                STRIP_NOPS (arg);
                if (really_constant_p (arg)
                    || (TREE_CODE (arg) == ADDR_EXPR
                        && TREE_CODE (TREE_OPERAND (arg, 0)) == STRING_CST))
                  wn = WN_Intconst (MTYPE_I4, 1);

                else
                  wn = WN_Intconst (MTYPE_I4, 0);
//                wn = WFE_Expand_Expr (TREE_VALUE (TREE_OPERAND (exp, 1)));
                whirl_generated = TRUE;
                break;
              }
#if 0
              case BUILT_IN_LOCK_TEST_AND_SET:
                wn = emit_builtin_lock_test_and_set (exp, num_args-2);
                whirl_generated = TRUE;
                break;

              case BUILT_IN_LOCK_RELEASE:
                emit_builtin_lock_release (exp, num_args-1);
                whirl_generated = TRUE;
                break;

              case BUILT_IN_COMPARE_AND_SWAP:
                wn = emit_builtin_compare_and_swap (exp, num_args-3);
                whirl_generated = TRUE;
                break;

              case BUILT_IN_SYNCHRONIZE:
                emit_builtin_synchronize (exp, num_args);
                whirl_generated = TRUE;
                break;

              case BUILT_IN_RETURN_ADDRESS:
                i = Get_Integer_Value (TREE_VALUE (TREE_OPERAND (exp, 1)));
		if (i > 0) {
			// currently don't handle levels > 0,
			// which requires iterating thru call-stack
			// and finding $ra in fixed place.
			warning("non-zero levels not supported for builtin_return_address");
			wn = WN_Intconst(Pointer_Mtype, 0);
		}
		else {
			st = WFE_Get_Return_Address_ST (i);
			wn = WN_Ldid (Pointer_Mtype, 0, st, ST_type (st));
		}
                whirl_generated = TRUE;
		break;
#endif

#ifdef TARG_ST
                // (cbr) Builtin for glibc compatibility, Returning the
                // builtin_return_address value.
                // I think that the libc should be fixed or the builtin
                // deprecated but. keep the compatibility here 
            case BUILT_IN_EXTRACT_RETURN_ADDR:
	      // [SC] Add builtin_frob_return_address, it is also an identity op
	      // on our supported targets.
	    case BUILT_IN_FROB_RETURN_ADDR:
              wn = WFE_Expand_Expr (TREE_VALUE (TREE_OPERAND (exp, 1)));
              whirl_generated = TRUE;
              break;
#endif

#if defined (TARG_ST) && (GNU_FRONT_END==33)
              /* (cbr) needed to implement eh_return */
            case BUILT_IN_EH_RETURN_DATA_REGNO: {
              int v = Get_Integer_Value(TREE_VALUE (TREE_OPERAND (exp, 1)));
              wn = WN_Intconst(MTYPE_I4, EH_RETURN_DATA_REGNO(v));
              whirl_generated = TRUE;
              break;
            }
            case BUILT_IN_DWARF_SP_COLUMN: {
              int v = DWARF_FRAME_REGNUM (STACK_POINTER_REGNUM);
              wn = WN_Intconst(MTYPE_I4, v);
              whirl_generated = TRUE;
              break;
            }
	    case BUILT_IN_EH_RETURN:
              iopc = INTRN_BUILTIN_EH_RETURN;
	      break;
	    case BUILT_IN_UNWIND_INIT:
              iopc = INTRN_BUILTIN_UNWIND_INIT;
	      break;
	    case BUILT_IN_DWARF_CFA:
              iopc = INTRN_BUILTIN_DWARF_CFA;
	      break;
	    case BUILT_IN_INIT_DWARF_REG_SIZES:
	      emit_builtin_init_dwarf_reg_sizes
		(WFE_Expand_Expr (TREE_VALUE (TREE_OPERAND (exp, 1))));
	      whirl_generated = TRUE;
	      break;
#endif

#ifdef TARG_ST
              case BUILT_IN_CLASSIFY_TYPE:
		// builtin_function ("__builtin_classify_type", default_function_type, BUILT_IN_CLASSIFY_TYPE, BUILT_IN_NORMAL, NULL_PTR);
		// default_function_type is integer
		// This is a CALL_EXPR where tree arglist = TREE_OPERAND (exp, 1);
		wn = emit_builtin_classify_type(exp) ;
		whirl_generated = TRUE;
		break;
#endif

#ifdef TARG_ST
                /* (cbr) add */
	      case BUILT_IN_EXPECT:
	      {
		// __builtin_expect has two arguments
		// the first argument is the value to be returned
		// the second value is the expected value for branch prediction
/*
		WN *arg1, *arg2;
		list = TREE_OPERAND (exp, 1);
		arg1 = WFE_Expand_Expr (TREE_VALUE (list));
		list = TREE_CHAIN (list);
		arg2 = WFE_Expand_Expr (TREE_VALUE (list));
		wn   = WN_Relational (OPR_EQ, WN_rtype (arg1), arg1, arg2);
*/
		list = TREE_OPERAND (exp, 1);
		wn   = WFE_Expand_Expr (TREE_VALUE (list));
#ifdef TARG_ST
                /* (cbr) need to remember for branch prediction hint. 0 = no hint */
                extern int if_else_hint;
                if_else_hint =
                  Get_Integer_Value (TREE_VALUE (TREE_CHAIN(list))) ? 
                  FREQUENCY_HINT_FREQUENT : FREQUENCY_HINT_NEVER;
#endif
		whirl_generated = TRUE;
		break;
	      }
#endif


#ifdef TARG_ST
		// [CG}: Added support for __builtin_prefech. Ref gcc exensions
	    case BUILT_IN_PREFETCH:
	      if (arglist == 0 ||
		  /* Arg could be non-pointer if user redeclared this fcn wrong.  */
		  TREE_CODE (TREE_TYPE (TREE_VALUE (arglist))) != POINTER_TYPE)
		break;
	      if (Ignore_Builtin_Prefetch) {
		whirl_generated = TRUE;
		break;
	      }
	      arg_wn = WFE_Expand_Expr (TREE_VALUE (arglist));
	      wn = WN_CreatePrefetch(0, 0, arg_wn);
	      WN_pf_set_manual(wn);
	      // clarkes:Should use the optional rw argument to set read/write.
	      WN_pf_set_read(wn);
	      // clarkes:
	      // GNU builtin does not have an argument to specify confidence
	      // (unless it can be derived from locality).
	      // So set high confidence level since this is a user assertion
	      // that a prefetch is required.
	      WN_pf_set_confidence(wn, 2);
	      // clarkes:
	      // GNU builtin also does not have a way to specify stride.
	      // Default it to 1 in first level cache.
	      WN_pf_set_stride_1L(wn, 1);
	      whirl_generated = TRUE;
	      break;

#endif

#ifdef TARG_ST
	    case BUILT_IN_TRAP: 
              wn = emit_builtin_trap ();
              whirl_generated = TRUE;
	      break;
#endif

#ifdef TARG_ST100
	    case BUILT_IN_MPSSE: 
	      iopc = INTRN_MPSSE;
	      intrinsic_op = TRUE;
	      break;

	    case BUILT_IN_MASSE: 
#ifdef WFE_DEBUG
  fprintf (stdout, " BUILT_IN_MASSE:\n");
#endif /* WFE_DEBUG */
	      iopc = INTRN_MASSE;
	      intrinsic_op = TRUE;
	      break;

	    case BUILT_IN_DIVW: 
	      iopc = INTRN_DIVW;
	      intrinsic_op = TRUE;
	      break;

	    case BUILT_IN_DIVUW: 
	      iopc = INTRN_DIVUW;
	      intrinsic_op = TRUE;
	      break;
#endif
#ifdef TARG_ST
#include "gfec_wfe_expr.h"	/* Will come from targinfo/<arch>/...*/
#endif /* defined(TARG_ST200) */
	      default:
		DevWarn ("Encountered BUILT_IN: %d (%s) at line %d\n",
			 DECL_FUNCTION_CODE (func), built_in_names[DECL_FUNCTION_CODE (func)], lineno);
		break;
            }
	  }

          if (whirl_generated) {
            break;
          }

	  if (intrinsic_op) {
	    WN *ikids [5];
	    for (i = 0, list = TREE_OPERAND (exp, 1);
		 list;
		 i++, list = TREE_CHAIN (list)) {
              arg_wn     = WFE_Expand_Expr (TREE_VALUE (list));
	      arg_ty_idx = Get_TY(TREE_TYPE(TREE_VALUE(list)));
	      arg_mtype  = TY_mtype(arg_ty_idx);
              arg_wn     = WN_CreateParm (Mtype_comparison (arg_mtype), arg_wn,
					  arg_ty_idx, WN_PARM_BY_VALUE);
	      ikids [i]  = arg_wn;
	    }

#ifdef TARG_ST
	    //
	    // Arthur: there is a bug here !
	    //         We can't generate an INTRINSIC_OP of rtype
	    //         < I4/U4. We really need to generate
	    //           I4INTRINSIC_OP
	    //         I4CVTL 16
	    //         for example.
	    //
	    // [CG] We get the real return type of the call
	    // because the rtype of INTRINSIC_OP must be conform
	    // the the intrinsic prototype. The ret_mtype
	    // may have been set to void if need_result is true.
	    // We don't want this
	    TYPE_ID iop_mtype = TY_mtype (ty_idx);
	    ret_mtype = iop_mtype;

	    BOOL need_cvtl = FALSE;
	    UINT cvtl_size;
	    if (MTYPE_byte_size(ret_mtype) < MTYPE_byte_size(Max_Int_Mtype)) {
	      cvtl_size = MTYPE_byte_size(ret_mtype) * 8;
	      ret_mtype = MTYPE_signed(ret_mtype) ? Max_Int_Mtype : Max_Uint_Mtype;
	      need_cvtl = TRUE;
	    }
#endif

	    wn = WN_Create_Intrinsic (OPR_INTRINSIC_OP, ret_mtype, MTYPE_V,
				      iopc, num_args, ikids);

#if 0
            /* (cbr) don't sign extend intrinsic return val if < 32 bit */
#ifdef TARG_ST
	    if (need_cvtl) {
	      wn = WN_CreateCvtl(OPR_CVTL, ret_mtype, MTYPE_V, cvtl_size, wn);
	    }
#endif
#endif

	    break;
	  }

	  if (iopc) {
            call_wn = WN_Create (OPR_INTRINSIC_CALL, ret_mtype, MTYPE_V, num_args);
	    WN_intrinsic (call_wn) = iopc;
	  }
	  else {
	    num_handlers = Current_Handler_Count();
            call_wn = WN_Create (OPR_CALL, ret_mtype, MTYPE_V,
                                 num_args + num_handlers);

            st = Get_ST (TREE_OPERAND (arg0, 0));
            WN_st_idx (call_wn) = ST_st_idx (st);
	  }
        }

        else {
	  num_args++;
	  num_handlers = Current_Handler_Count();
          call_wn = WN_Create (OPR_ICALL, ret_mtype, MTYPE_V,
			       num_args + num_handlers);
	  WN_kid(call_wn, num_args-1) = WFE_Expand_Expr (TREE_OPERAND (exp, 0));
	  WN_set_ty (call_wn, TY_pointed(Get_TY(TREE_TYPE (TREE_OPERAND (exp, 0)))));
	}

	WN_Set_Linenum (call_wn, Get_Srcpos());
	WN_Set_Call_Default_Flags (call_wn);

#ifdef TARG_ST
        /* (cbr) GNU C interprets a `volatile void' return type to indicate
           that the function does not return. */
        if (st) {
          tree func = TREE_OPERAND (arg0, 0);
          if (TREE_THIS_VOLATILE (func) &&
              TREE_TYPE (TREE_TYPE (func))) {
            WN_Set_Call_Never_Return(call_wn);
          }

          /* (cbr) for 'malloc' attribute */
          if (DECL_IS_MALLOC (func)) {
            WN_Set_Call_Does_Mem_Alloc(call_wn);
          }
        }
#endif

#ifdef TARG_ST
	// [CL] Apply inline/noinline pragma in order:
	// check callsite pragmas, then function scope pragmas,
	// and finally file scope pragmas
	if (st) {
	  WN* pragma_wn;
	  bool has_callsite_pragma = FALSE;
	  bool has_function_pragma = FALSE;
	  bool has_file_pragma = FALSE;

	  if (pragma_wn = Has_Callsite_Pragma_Inline(call_wn)) {
	    WN_Set_Call_Inline(call_wn);
	    has_callsite_pragma = TRUE;
	    WN_pragma_arg1(pragma_wn) = 1;
	  }
	  else if (pragma_wn = Has_Callsite_Pragma_NoInline(call_wn)) {
	    WN_Set_Call_Dont_Inline(call_wn);
	    has_callsite_pragma = TRUE;
	    WN_pragma_arg1(pragma_wn) = 1;
	  }

	  if (!has_callsite_pragma) {
	    if (pragma_wn = Has_Function_Pragma_Inline(call_wn)) {
	      WN_Set_Call_Inline(call_wn);
	      has_function_pragma = TRUE;
	      WN_pragma_arg1(pragma_wn) = 1;
	    }
	    else if (pragma_wn = Has_Function_Pragma_NoInline(call_wn)) {
	      WN_Set_Call_Dont_Inline(call_wn);
	      has_function_pragma = TRUE;
	      WN_pragma_arg1(pragma_wn) = 1;
	    }
	  }

	  if (!has_callsite_pragma && !has_function_pragma) {
	    if (pragma_wn = Has_File_Pragma_Inline(call_wn)) {
	      WN_Set_Call_Inline(call_wn);
	      has_file_pragma = TRUE;
	      WN_pragma_arg1(pragma_wn) = 1;
	    }
	    else if (pragma_wn = Has_File_Pragma_NoInline(call_wn)) {
	      WN_Set_Call_Dont_Inline(call_wn);
	      has_file_pragma = TRUE;
	      WN_pragma_arg1(pragma_wn) = 1;
	    }
	  }
	}
#endif

        if (st) {
          tree func = TREE_OPERAND (arg0, 0);
          if (DECL_INLINE (func)) {
            wfe_invoke_inliner = TRUE;
          }
        }

        i = 0;
	for (list = TREE_OPERAND (exp, 1);
	     list;
	     list = TREE_CHAIN (list)) {
	  if (i == 0 && is_aggr_init_via_ctor) {
            ST * st = Get_ST(TREE_VALUE(list));
	    arg_wn = WN_Lda(Pointer_Mtype, ST_ofst(st), st);
	    arg_ty_idx = Get_TY(
			   build_pointer_type(TREE_TYPE(TREE_VALUE(list))));
          }
#if defined (TARG_ST) && (GNU_FRONT_END==33)
          /* (cbr) C++ uses a TARGET_EXPR to indicate that we want to make a
             new object from the argument.  It is safe in the only case where this
             is a useful optimization; namely, when the argument is a plain object.
          */
          else if (TREE_CODE (TREE_VALUE(list)) == TARGET_EXPR 
              && (DECL_P (TREE_OPERAND (TREE_VALUE(list), 1)))) {
            arg_wn     = WFE_Expand_Expr (TREE_OPERAND (TREE_VALUE(list), 1));
	    arg_ty_idx = Get_TY(TREE_TYPE(TREE_OPERAND (TREE_VALUE(list), 1)));
          }
#endif
	  else {
            arg_wn     = WFE_Expand_Expr (TREE_VALUE (list));
	    arg_ty_idx = Get_TY(TREE_TYPE(TREE_VALUE(list)));
	  }

	  arg_mtype  = TY_mtype(arg_ty_idx);
#if 0
	  // gcc allows non-struct actual to correspond to a struct formal;
	  // fix mtype of parm node so as not to confuse back-end
	  if (arg_mtype == MTYPE_M) {
	    arg_mtype = WN_rtype(arg_wn);
	  }
#endif
          arg_wn = WN_CreateParm (Mtype_comparison (arg_mtype), arg_wn,
		    		  arg_ty_idx, WN_PARM_BY_VALUE);
          WN_kid (call_wn, i++) = arg_wn;
        }

#ifdef ADD_HANDLER_INFO
	if (num_handlers) 
	  Add_Handler_Info (call_wn, i, num_handlers);
#endif

#ifdef KEY
          //	if (key_exceptions
          if (flag_exceptions
            && !TREE_NOTHROW(exp) && 
// Call terminate() "when the destruction of an object during stack 
// unwinding (except.ctor) exits using an exception" [except.terminate]
// So we don't want to form a region in such cases.
//
// NOTE: It need not be a destructor call, e.g. if we inline the destructor,
// all functions inside cannot throw. We assume that it is cleanup code means
// it has to be a destructor, be its call or its body.
	    !(in_cleanup))
	{
#ifdef TARG_ST
            /* (cbr) also if eh handler need (e.g cleanup) without try seen */
          if (!inside_eh_region)
#else
            if (!inside_eh_region && try_block_seen)
#endif
	    { // check that we are not already in a region
            	WN * region_body = WN_CreateBlock();
		inside_eh_region = true;
            	WFE_Stmt_Push (region_body, wfe_stmk_call_region_body, Get_Srcpos());
	    }

            }

#endif // KEY

        if (ret_mtype == MTYPE_V) {
	  WFE_Stmt_Append (call_wn, Get_Srcpos());
        }

	else {
          wn0 = WN_CreateBlock ();
          WN_INSERT_BlockLast (wn0, call_wn);

	  wn1 = WN_Ldid (ret_mtype, -1, Return_Val_Preg, ty_idx);

	  if (ret_mtype == MTYPE_M) { // copy the -1 preg to a temp area
	    TY_IDX ret_ty_idx = ty_idx;
	    if (Aggregate_Alignment > 0 &&
		Aggregate_Alignment > TY_align (ret_ty_idx))
	      Set_TY_align (ret_ty_idx, Aggregate_Alignment);
            if (TY_align (ret_ty_idx) < MTYPE_align_best(Spill_Int_Mtype))
              Set_TY_align (ret_ty_idx, MTYPE_align_best(Spill_Int_Mtype));
	    ST *ret_st = Gen_Temp_Symbol(ret_ty_idx, 
		  st ? Index_To_Str(Save_Str2(".Mreturn.",
					      ST_name(ST_st_idx(st))))
		     : ".Mreturn.");
	    wn1 = WN_Stid (ret_mtype, 0, ret_st, ty_idx, wn1);
            WN_INSERT_BlockLast (wn0, wn1);

	    // ritual for determining the right mtypes to be used in the LDID
            UINT xtra_BE_ofst = 0;  // only needed for big-endian target
            desc_ty_idx = component_ty_idx;
            if (desc_ty_idx == 0)
              desc_ty_idx = Get_TY (TREE_TYPE(exp));
              
            if (! MTYPE_is_integral(TY_mtype(desc_ty_idx)))
              ty_idx = desc_ty_idx;
            else { 
              ty_idx = nop_ty_idx;
              if (ty_idx == 0)
                ty_idx = desc_ty_idx;
            }

	    if (! is_bit_field) {
              if (TY_size(desc_ty_idx) > TY_size(ty_idx)) {
                if (Target_Byte_Sex == BIG_ENDIAN)
                  xtra_BE_ofst = TY_size(desc_ty_idx) - TY_size(ty_idx);
                desc_ty_idx = ty_idx;
	      }
            }
	    else {
	      if (TY_size(desc_ty_idx) > TY_size(ty_idx))
		ty_idx = desc_ty_idx;
	    }

	    TYPE_ID rtype = Widen_Mtype(TY_mtype(ty_idx));
	    TYPE_ID desc = TY_mtype(desc_ty_idx);
	    if (MTYPE_is_integral(desc)) {
	      if (MTYPE_signed(rtype) != MTYPE_signed(desc)) {
		if (MTYPE_size_min(rtype) > MTYPE_size_min(desc) ||
		    is_bit_field)
		  rtype = Mtype_TransferSign(desc, rtype);
		else desc = Mtype_TransferSign(rtype, desc);
	      }
	    }

            Is_True(! is_bit_field || field_id <= MAX_FIELD_ID,
                    ("WFE_Expand_Expr: field id for bit-field exceeds limit"));
  
	    wn1 = WN_CreateLdid(OPR_LDID, rtype,
			        is_bit_field ? MTYPE_BS : desc,
			        ST_ofst(ret_st)+component_offset+xtra_BE_ofst, 
				ret_st,
				(field_id != 0 && component_ty_idx != 0) ?
				Get_TY (TREE_TYPE(exp)) : ty_idx,
				field_id);
	  }

          wn  = WN_CreateComma (OPR_COMMA, WN_rtype (wn1), MTYPE_V,
                                wn0, wn1);
        }
      }
      break;

    case COMPOUND_EXPR:
      {
        wn = WFE_Expand_Expr (TREE_OPERAND (exp, 0), FALSE);

        if (wn && WN_has_side_effects(wn)) {
#ifdef TARG_ST
	  wn = WFE_Append_Expr_Stmt (wn);
#else
          wn = WN_CreateEval (wn);
          WFE_Stmt_Append (wn, Get_Srcpos ());
#endif
        }
        wn = WFE_Expand_Expr (TREE_OPERAND (exp, 1), need_result);
      }
      break;

    case NON_LVALUE_EXPR:
      {
        wn = WFE_Expand_Expr (TREE_OPERAND (exp, 0));
      }
      break;

    case SAVE_EXPR:
      {
#ifndef TARG_ST
	DevWarn ("Encountered SAVE_EXPR at line %d", lineno);
#endif
        wn = WFE_Save_Expr (exp, need_result, nop_ty_idx,
			    component_ty_idx, component_offset, field_id);
      }
      break;

    case ERROR_MARK:
      // This is not necessarily an error:  return a constant 0.
      wn = WN_Intconst(MTYPE_I4, 0);
      break;

    case LOOP_EXPR:
      {
#ifndef TARG_ST
        DevWarn ("Encountered LOOP_EXPR at line %d\n", lineno);
#endif
        LABEL_IDX saved_loop_expr_exit_label = loop_expr_exit_label;
        loop_expr_exit_label = 0;
        tree body = LOOP_EXPR_BODY(exp);
        WN *loop_test = WN_Intconst (Boolean_type, 1);
        WN *loop_body = WN_CreateBlock ();
        if (body) {
          WFE_Stmt_Push (loop_body, wfe_stmk_while_body, Get_Srcpos());
          wn = WFE_Expand_Expr (body);
          WFE_Stmt_Pop (wfe_stmk_while_body);
        }
        WN *loop_stmt = WN_CreateWhileDo (loop_test, loop_body);
        WFE_Stmt_Append (loop_stmt, Get_Srcpos());
        if (loop_expr_exit_label)
          WFE_Stmt_Append (WN_CreateLabel ((ST_IDX) 0, loop_expr_exit_label, 0, NULL),
                           Get_Srcpos ());
        loop_expr_exit_label = saved_loop_expr_exit_label;
      }
      break;

    case EXIT_EXPR:
      {
#ifndef TARG_ST
        DevWarn ("Encountered EXIT_EXPR at line %d\n", lineno);
#endif
	WN *test = WFE_Expand_Expr (TREE_OPERAND(exp, 0));
        New_LABEL (CURRENT_SYMTAB, loop_expr_exit_label);
        WN *stmt = WN_CreateTruebr (loop_expr_exit_label, test);
        WFE_Stmt_Append (stmt, Get_Srcpos ());
      }
      break;

    case VA_ARG_EXPR:
#ifdef TARG_ST200
      {
	INT64 align;
	INT64 rounded_size;
	INT64 adjustment;
	tree type = TREE_TYPE (exp);
	TY_IDX hi_ty_idx = Get_TY (type);
	TY_IDX ty_idx = component_ty_idx ? component_ty_idx : hi_ty_idx;
	TYPE_ID mtype = TY_mtype (ty_idx);
	INT64 type_size = TY_size (hi_ty_idx);
	TY_IDX va_list_ty_idx = Get_TY (va_list_type_node);
	TYPE_ID va_list_mtype = TY_mtype (va_list_ty_idx);
	WN *ap_addr, *ap_load, *ap_store;
	WN *arg_addr;
	UINT va_list_field_id = (Target_Byte_Sex == BIG_ENDIAN) ? 1 : 0;
	tree operand = TREE_OPERAND (exp, 0);

	// [SC] I would like to use WFE_Address_Of(operand) here,
	// but WFE_Address_Of is available in C++ translator but not the
	// C translator.
	if (TREE_CODE (operand) == INDIRECT_REF) {
	  ap_addr = WFE_Expand_Expr (TREE_OPERAND(operand, 0));
	} else {
	  ap_addr = WFE_Expand_Expr (build1 
				     (ADDR_EXPR,
				      build_pointer_type (TREE_TYPE (operand)),
				      operand));
	}
	if (WN_operator (ap_addr) == OPR_LDA) {
	  ap_load = WN_Ldid (Pointer_Mtype, 0, WN_st (ap_addr),
			     va_list_ty_idx, va_list_field_id);
	} else {
	  ap_load = WN_Iload (Pointer_Mtype, 0, va_list_ty_idx,
			      ap_addr, va_list_field_id);
	}
	
	// Any parameter larger than a word is double-word aligned.
	align = ((type_size > UNITS_PER_WORD)
		 ? (2 * UNITS_PER_WORD)
		 : UNITS_PER_WORD);
	// All parameters are passd in a multiple of word-sized slots.
	rounded_size = (((int_size_in_bytes (type) + (UNITS_PER_WORD - 1))
			 / UNITS_PER_WORD) * UNITS_PER_WORD);

	// Set wn = start address of arg.
	if (align > 4) {
	  // ap is guaranteed to be 4-byte aligned, but for larger
	  // alignments we must adjust it.
	  wn = WN_Binary (OPR_BAND,
			  Pointer_Mtype,
			  WN_Binary (OPR_ADD, Pointer_Mtype, ap_load,
				     WN_Intconst (Pointer_Mtype, align - 1)),
			  WN_Intconst (Pointer_Mtype, ~(align - 1)));
	} else {
	  wn = ap_load;
	}

	// add to wn the rounded size of the arg
	wn = WN_Binary (OPR_ADD, Pointer_Mtype, wn,
			WN_Intconst (Pointer_Mtype, rounded_size));
	// store back in ap
	if (WN_operator (ap_addr) == OPR_LDA) {
	  ap_store = WN_Stid (Pointer_Mtype, 0, WN_st (ap_addr),
			      va_list_ty_idx, wn, va_list_field_id);
	} else {
	  ap_store = WN_Istore (Pointer_Mtype, 0,
				Make_Pointer_Type (va_list_ty_idx),
				WN_COPY_Tree (ap_addr),
				wn, va_list_field_id);
	}
        WFE_Stmt_Append (ap_store, Get_Srcpos ());
	
	if (Target_Byte_Sex == BIG_ENDIAN
	    && type_size > UNITS_PER_WORD
	    && (INTEGRAL_TYPE_P(type)
		|| FLOAT_TYPE_P(type))) {
	  // Handle multi-word scalar/complex, passed in registers.
	  UINT64 n_words = type_size / UNITS_PER_WORD;
	  // Need to exchange alternate words.
	  // Create a temporary of the appropriate type.
	  // The type is union { ty; struct { int; int; ... } };
	  TY_IDX struct_ty_idx;
	  TY &struct_ty = New_TY (struct_ty_idx);
	  TY_Init (struct_ty, type_size, KIND_STRUCT, MTYPE_M, 0);
	  Set_TY_align (struct_ty_idx, TY_align (MTYPE_To_TY (Def_Int_Mtype)));
	  FLD_HANDLE struct_fld;
	  for (UINT64 offs = 0; offs < type_size; offs += UNITS_PER_WORD) {
	    struct_fld = New_FLD ();
	    FLD_Init (struct_fld, 0 /* anonymous */,
		      MTYPE_To_TY(Def_Int_Mtype), offs);
	    if (offs == 0) Set_TY_fld (struct_ty, struct_fld);
	  }
	  Set_FLD_last_field (struct_fld);
	  TY_IDX union_ty_idx;
	  TY &union_ty = New_TY (union_ty_idx);
	  TY_Init (union_ty, type_size, KIND_STRUCT, MTYPE_M, 0);
	  Set_TY_is_union (union_ty_idx);
	  Set_TY_align (union_ty_idx, TY_align (ty_idx));
	  FLD_HANDLE union_fld;
	  union_fld = New_FLD ();
	  FLD_Init (union_fld, 0 /* anonymous */, ty_idx, 0);
	  Set_TY_fld (union_ty, union_fld);
	  union_fld = New_FLD ();
	  FLD_Init (union_fld, 0 /* anonymous */, struct_ty_idx, 0);
	  Set_FLD_last_field (union_fld);
	  // Finally created the type, now create the temporary.
	  ST *temp_st = Gen_Temp_Symbol (union_ty_idx, "_va_arg_temp");
	  Set_ST_addr_saved (temp_st);
	  // Copy words from stack to symbol, swapping
	  // alternate words.
	  INT64 load_offset = -type_size;
	  INT64 store_offset = 0;
	  // field_id 3 is the first field of the struct within the
	  // union.
	  UINT store_field_id = 3;
	  for (UINT64 w = 0; w < n_words; w+= 2) {
	    wn = WN_CreateIload(OPR_ILOAD, Def_Int_Mtype, Def_Int_Mtype,
				load_offset,
				MTYPE_TO_TY_array[Def_Int_Mtype],
				Make_Pointer_Type(ty_idx),
				WN_COPY_Tree (ap_load));
	    wn = WN_Stid (Def_Int_Mtype, 
			  store_offset + UNITS_PER_WORD,
			  temp_st, union_ty_idx,
			  wn, store_field_id++);
	    WFE_Stmt_Append (wn, Get_Srcpos ());
	    wn = WN_CreateIload(OPR_ILOAD, Def_Int_Mtype, Def_Int_Mtype,
				load_offset + UNITS_PER_WORD,
				MTYPE_TO_TY_array[Def_Int_Mtype],
				Make_Pointer_Type(ty_idx),
				WN_COPY_Tree (ap_load));
	    wn = WN_Stid (Def_Int_Mtype, store_offset,
			  temp_st, union_ty_idx,
			  wn, store_field_id++);
	    WFE_Stmt_Append (wn, Get_Srcpos ());
	    load_offset += 2 * UNITS_PER_WORD;
	    store_offset += 2 * UNITS_PER_WORD;
	  }
	  // load the ap limit value.
	  if (WN_operator (ap_addr) == OPR_LDA) {
	    wn = WN_Ldid (Pointer_Mtype, 4, WN_st (ap_addr),
			  va_list_ty_idx, 2);
	  } else {
	    wn = WN_Iload (Pointer_Mtype, 4, va_list_ty_idx,
			   WN_COPY_Tree (ap_addr), 2);
	  }
	  // compare ap with the limit.
	  wn = WN_Relational (OPR_LT,
			      Pointer_Mtype,
			      WN_Binary (OPR_SUB,
					 Pointer_Mtype,
					 WN_COPY_Tree (ap_load),
					 WN_Intconst (Pointer_Mtype,
						      rounded_size)),
			      wn);
	  // if ap is below limit, load the temporary,
	  // otherwise load through ap.
	  wn = WN_Cselect (mtype,
			   wn,
			   WN_Ldid (mtype, 0, temp_st, union_ty_idx, 1),
			   WN_CreateIload (OPR_ILOAD, mtype, mtype,
					   -type_size + component_offset,
					   field_id != 0 ? hi_ty_idx : ty_idx,
					   Make_Pointer_Type (hi_ty_idx, FALSE),
					   WN_COPY_Tree (ap_load), field_id));
	  Set_PU_has_very_high_whirl (Get_Current_PU ());
	} else {
	  if (Target_Byte_Sex == BIG_ENDIAN
	      && type_size < UNITS_PER_WORD
	      && ! AGGREGATE_TYPE_P (type)) {
	    adjustment = type_size;
	  } else {
	    adjustment = rounded_size;
	  }
	  arg_addr = WN_COPY_Tree (ap_load);
	  // Now ap points to the word just after the arg.
	  wn = WN_CreateIload (OPR_ILOAD, Widen_Mtype (mtype), mtype,
			       -adjustment + component_offset,
			       field_id != 0 ? hi_ty_idx : ty_idx,
			       Make_Pointer_Type (hi_ty_idx, FALSE),
			       arg_addr,
			       field_id);
	}
      }
#else
      {
        // code swiped from builtins.c (std_expand_builtin_va_arg)
	INT64 align;
	INT64 rounded_size;
	tree type = TREE_TYPE (exp);
	TY_IDX ty_idx = Get_TY (type);
	TYPE_ID mtype = TY_mtype (ty_idx);

        /* Compute the rounded size of the type.  */
	align = PARM_BOUNDARY / BITS_PER_UNIT;
	rounded_size = (((int_size_in_bytes (type) + align - 1) / align) * align);

	/* Get AP.  */
	WN *ap = WFE_Expand_Expr (TREE_OPERAND (exp, 0));
	st = WN_st (ap);

	if (Target_Byte_Sex == BIG_ENDIAN) {
	  Fail_FmtAssertion ("VA_ARG_EXPR not implemented for BIG_ENDIAN");
	  INT64 adj;
	  adj = TREE_INT_CST_LOW (TYPE_SIZE (type)) / BITS_PER_UNIT;
	  if (rounded_size > align)
	    adj = rounded_size;

	  wn = WN_Binary (OPR_ADD, Pointer_Mtype, wn,
			  WN_Intconst (Pointer_Mtype, rounded_size - adj));
	}

	/* Compute new value for AP.  */
	wn = WN_Binary (OPR_ADD, Pointer_Mtype, ap,
			WN_Intconst (Pointer_Mtype, rounded_size));
	wn = WN_Stid (Pointer_Mtype, 0, st, ST_type (st), wn);
        WFE_Stmt_Append (wn, Get_Srcpos ());
        wn = WN_CreateIload (OPR_ILOAD, Widen_Mtype (mtype), mtype, -rounded_size,
			     ty_idx, Make_Pointer_Type (ty_idx, FALSE),
			     WN_Ldid (Pointer_Mtype, 0, st, ST_type (st)));
      }
#endif
      break;


#ifdef KEY
    case EXPR_WITH_FILE_LOCATION:
      wn = WFE_Expand_Expr (EXPR_WFL_NODE (exp), need_result, nop_ty_idx,
			    component_ty_idx, component_offset, field_id,
			    is_bit_field, is_aggr_init_via_ctor);
      break;

      case EXC_PTR_EXPR: {
        DevWarn ("Check implementation of EXC_PTR_EXPR: at line %d\n", lineno);
        if (flag_exceptions)
        //        if (key_exceptions)
          {
            ST_IDX exc_ptr_st = TCON_uval (INITV_tc_val (INITO_val (Get_Current_PU().unused)));
            wn = WN_Ldid (Pointer_Mtype, 0, exc_ptr_st, Get_TY(TREE_TYPE(exp)));
          }
        else
          {
            ST * preg_st = MTYPE_To_PREG(Pointer_Mtype);
            wn = WN_Ldid (Pointer_Mtype, 0, preg_st, Get_TY(TREE_TYPE(exp)));
          }
      }
        break;

    case CLEANUP_STMT:
      DevWarn ("CLEANUP_STMT not implemented: at line %d\n", lineno);
      // TODO:  Return a dummy constant 0 for now.
      wn = WN_Intconst(MTYPE_I4, 0);
      break;

      case MUST_NOT_THROW_EXPR:
      // Call terminate if this expr throws
        wn = WFE_Expand_Expr (TREE_OPERAND (exp,0));
        LABEL_IDX terminate_label, around_label;

        New_LABEL (CURRENT_SYMTAB, terminate_label);
        New_LABEL (CURRENT_SYMTAB, around_label);
        WFE_Stmt_Append (WN_CreateGoto (around_label), Get_Srcpos());

        WFE_Stmt_Append (WN_CreateLabel ((ST_IDX) 0, terminate_label, 0, NULL),
                         Get_Srcpos());
        Call_Terminate ();
        WFE_Stmt_Append (WN_CreateLabel ((ST_IDX) 0, around_label, 0, NULL),
                         Get_Srcpos());
        break;
#endif /* KEY */

    default:
      {
        Fail_FmtAssertion ("WFE_Expand_Expr: not implemented %s",
                           Operator_From_Tree [code].name);
      }
      break;
    }

#ifdef WFE_DEBUG
  if (wn)
    fdump_tree (stderr, wn);

  fprintf (stderr, // "{("
           ")} WFE_Expand_Expr: %s\n", Operator_From_Tree [code].name);
#endif /* WFE_DEBUG */


#if defined (TARG_ST) && (GNU_FRONT_END==33)
    /* (cbr) gcc 3.3 upgrade */
  if (need_result) {
    FmtAssert (wn != 0 || 
	       code == (enum tree_code)CALL_EXPR || 
	       code == (enum tree_code)BIND_EXPR ||
               code == (enum tree_code)COMPOUND_EXPR ||
               code == (enum tree_code)INDIRECT_REF ||
               code == (enum tree_code)COMPONENT_REF ||
	       code == (enum tree_code)FOR_STMT || 
	       code == (enum tree_code)RETURN_STMT || 
	       code == (enum tree_code)ASM_STMT || 
	       code == (enum tree_code)COMPOUND_STMT || 
	       code == (enum tree_code)STMT_EXPR ||
               code == (enum tree_code)LOOP_EXPR ||
               code == (enum tree_code)NOP_EXPR ||
               code == (enum tree_code)THROW_EXPR ||
               ((code == (enum tree_code)COND_EXPR) && (TY_mtype (ty_idx) == MTYPE_V)),
	       ("WFE_Expand_Expr: NULL WHIRL tree for %s at line %d",
		Operator_From_Tree [code].name, lineno));
   }
#else
  if (need_result) {
    FmtAssert (wn != 0 || code == CALL_EXPR || code == BIND_EXPR ||
               code == COMPOUND_STMT ||
               code == STMT_EXPR     ||
 	       code == SUBOBJECT     ||
               code == COMPOUND_EXPR ||
               code == INDIRECT_REF  ||
               code == COMPONENT_REF ||
               code == LOOP_EXPR     ||
               code == NOP_EXPR      ||
               code == THROW_EXPR    ||
               ((code == COND_EXPR) && (TY_mtype (ty_idx) == MTYPE_V)),
	       ("WFE_Expand_Expr: NULL WHIRL tree for %s",
		Operator_From_Tree [code].name));
  }
#endif
  return wn;
}

#ifdef KEY
// Like WFE_One_Stmt but don't reuse label indexes already allocated so far.
// This is necessary because the cleanup represented by the EXP tree can be
// expanded multiple times, and each expansion needs its own set of labels.
void
WFE_One_Stmt_Cleanup (tree exp)
{
  LABEL_IDX idx = WFE_unusable_label_idx;
  INT32 save_expr_level = wfe_save_expr_level;

  // Don't reuse label indexes that are allocated up to this point.
  WFE_unusable_label_idx = WFE_last_label_idx;

  // Make the saved expr's, if any, unique to this cleanup.
  wfe_save_expr_level = ++wfe_last_save_expr_level;
  
#ifdef TARG_ST
  /* (cbr) pro-fe3.3-c++/50 make sure to catch cleanup code that can throw
     instead of unwinding it and recalling cleanup again */
  extern bool can_cleanup;
  can_cleanup = false;
#endif

  WFE_One_Stmt(exp);

#ifdef TARG_ST
  can_cleanup = true;
#endif

  WFE_unusable_label_idx = idx;
  wfe_save_expr_level = save_expr_level;
}
#endif

void WFE_One_Stmt (tree exp)
{
  WN *wn;

#ifndef KEY
  wfe_save_expr_stack_last = -1; // to minimize searches
#endif

#ifdef TARG_ST
  // (cbr) returning zero length struct. will be converted to void.
  tree exp_type = TREE_TYPE (exp);
  if (exp_type && AGGREGATE_TYPE_P(exp_type) && !Get_Integer_Value (TYPE_SIZE (exp_type)))
    wn = NULL;
  else
#endif
    wn = WFE_Expand_Expr_With_Sequence_Point (exp, MTYPE_V);

  if (wn) {
    for (;;) {
      if (WN_operator (wn) == OPR_COMMA) {
	WN *crwn = wn;
	if (WN_operator (WN_kid1 (wn)) == OPR_LDID                 &&
	    WN_st (WN_kid1 (wn)) == Return_Val_Preg                &&
	    (WN_operator (WN_last (WN_kid0 (wn))) == OPR_CALL   ||
	     WN_operator (WN_last (WN_kid0 (wn))) == OPR_ICALL)) {
	  WN_set_rtype (WN_last (WN_kid0 (wn)), MTYPE_V);
	  WFE_Stmt_Append (WN_kid0 (wn), Get_Srcpos ());
	  WN_Delete (crwn);
	  break;
	}
	else {
	  WFE_Stmt_Append (WN_kid0 (wn), Get_Srcpos ());
	  wn = WN_kid1 (wn);
	  WN_Delete (crwn);
	}
      }
      else {
	if (WN_has_side_effects (wn)) {
#ifdef TARG_ST
	  wn = WFE_Append_Expr_Stmt (wn);
#else
	  wn = WN_CreateEval (wn);
	  WFE_Stmt_Append (wn, Get_Srcpos ());
#endif
	}
	break;
      }
    }
  }
}

void WFE_Null_Return (void)
{
  WN *wn = WN_CreateReturn ();
  WFE_Stmt_Append (wn, Get_Srcpos());
}

UINT64
Get_Integer_Value (tree exp)
{
	FmtAssert (TREE_CODE(exp) == INTEGER_CST, 
		("Get_Integer_Value unexpected tree"));
#ifdef _LP64
	return TREE_INT_CST_LOW (exp);
#else
	UINT64 h = TREE_INT_CST_HIGH (exp);
	UINT64 l = TREE_INT_CST_LOW (exp);
	l = l << 32 >> 32;	// zero-out high 32 bits
	h = h << 32;
	return (h | l);
#endif /* _LP64 */
}

void
WFE_Expr_Init (void)
{
  INT i;
  for (i = 0; i < LAST_CPLUS_TREE_CODE; i++)
    FmtAssert (Operator_From_Tree [i].tree_code == i,
               ("Operator_From_Tree[%d] incorrect, value = %d (last is %d)",
                i, Operator_From_Tree [i].tree_code, LAST_CPLUS_TREE_CODE));
}

char *
WFE_Tree_Node_Name (tree op)
{
  return Operator_From_Tree [TREE_CODE (op)].name;
}

#ifdef TARG_ST
TYPE_ID
WFE_Promoted_Type(TYPE_ID mtype)
{
  switch (mtype) {
  case MTYPE_I1:
  case MTYPE_I2:
  case MTYPE_U1:
  case MTYPE_U2:
    return MTYPE_I4;
  default:
    return mtype;
  }
}

static WN *
WFE_Integral_Cast (TYPE_ID mtype, TYPE_ID kid_mtype, WN *kid)
{
  WN *cvt;
  WN *widen;
  TYPE_ID widen_mtype = WFE_Promoted_Type(mtype);
  TYPE_ID widen_kid_mtype = WFE_Promoted_Type(kid_mtype);
  
  if (kid_mtype != widen_kid_mtype) {
    TYPE_ID cvtl_mtype = Mtype_TransferSign(kid_mtype, widen_kid_mtype);
    WN *widen = WN_CreateCvtl(OPR_CVTL, cvtl_mtype, MTYPE_V,
			      MTYPE_size_min(kid_mtype), kid);
    return WFE_Integral_Cast(mtype, widen_kid_mtype, widen);
  }
  if (widen_mtype != mtype) {
    WN *cvt = WFE_Integral_Cast(widen_mtype, kid_mtype, kid);
    TYPE_ID cvtl_mtype = Mtype_TransferSign(mtype, widen_mtype);
    return WN_CreateCvtl(OPR_CVTL, cvtl_mtype, MTYPE_V,
			 MTYPE_size_min(mtype), cvt);
  }
  if (mtype != kid_mtype) {
    return WN_Cvt(kid_mtype, mtype, kid);
  } 
  return kid;
}

static WN *
WFE_Float_Trunc (TYPE_ID mtype, TYPE_ID kid_mtype, WN *kid)
{
  WN *cvt;
  WN *widen;
  TYPE_ID widen_mtype = WFE_Promoted_Type(mtype);

  if (widen_mtype != mtype) {
    WN *cvt = WFE_Float_Trunc(widen_mtype, kid_mtype, kid);
    TYPE_ID cvtl_mtype = Mtype_TransferSign(mtype, widen_mtype);
    return WN_CreateCvtl(OPR_CVTL, widen_mtype, MTYPE_V,
			 MTYPE_size_min(mtype), cvt);
  }
  return WN_Trunc(kid_mtype, mtype, kid);
}

static WN *
WFE_Integral_To_Float (TYPE_ID mtype, TYPE_ID kid_mtype, WN *kid)
{
  WN *cvt;
  WN *widen;
  TYPE_ID widen_kid_mtype = WFE_Promoted_Type(kid_mtype);
  
  if (kid_mtype != widen_kid_mtype) {
    TYPE_ID cvtl_mtype = Mtype_TransferSign(kid_mtype, widen_kid_mtype);
    WN *widen = WN_CreateCvtl(OPR_CVTL, cvtl_mtype, MTYPE_V,
			      MTYPE_size_min(kid_mtype), kid);
    return WFE_Integral_To_Float(mtype, widen_kid_mtype, widen);
  }
  return WN_Cvt(kid_mtype, mtype, kid);
}

static WN *
WFE_Float_Cast (TYPE_ID mtype, TYPE_ID kid_mtype, WN *kid)
{
  if (mtype != kid_mtype) {
    return WN_Cvt(kid_mtype, mtype, kid);
  } else {
    return kid;
  }
}

WN *
WFE_Cast(TYPE_ID mtype, TYPE_ID kid_mtype, WN *kid)
{
  if (MTYPE_is_integral(mtype) &&
      MTYPE_is_integral(kid_mtype)) {
    return WFE_Integral_Cast(mtype, kid_mtype, kid);
  } else if (MTYPE_is_integral(mtype) &&
	     MTYPE_is_float(kid_mtype)) {
    return WFE_Float_Trunc(mtype, kid_mtype, kid);
  } else if (MTYPE_is_float(mtype) &&
	     MTYPE_is_integral(kid_mtype)) {
    return WFE_Integral_To_Float(mtype, kid_mtype, kid);
  } else if (MTYPE_is_float(mtype) &&
	     MTYPE_is_float(kid_mtype)) {
    return WFE_Float_Cast(mtype, kid_mtype, kid);
  }
  FmtAssert(0, ("Unexpected mtypes for cast: %s -> %s\n", MTYPE_name(kid_mtype), MTYPE_name(mtype)));
  return 0;
}

TYPE_ID
WFE_Promoted_Binary_Type(TYPE_ID mtype1, TYPE_ID mtype2)
{
  TYPE_ID widen_mtype1 = WFE_Promoted_Type(mtype1);
  TYPE_ID widen_mtype2 = WFE_Promoted_Type(mtype2);
  if (widen_mtype1 == widen_mtype2) {
    return widen_mtype1;
  } else if (MTYPE_size_min(widen_mtype1) < MTYPE_size_min(widen_mtype2)) {
    return widen_mtype2;
  } else if (MTYPE_size_min(widen_mtype1) > MTYPE_size_min(widen_mtype2)) {
    return widen_mtype1;
  } else if (!MTYPE_signed(widen_mtype1)) {
    return widen_mtype1;
  } else if (!MTYPE_signed(widen_mtype1)) {
    return widen_mtype2;
  }
  FmtAssert(0, ("binary type promotion failed for mtypes: %s %s\n", MTYPE_name(mtype1), MTYPE_name(mtype2)));
}

#endif

