# 1 "NBG_burs_template.c"

/*
 * DO NOT MODIFY THIS FILE.
 * This file is the result of a code generator generator.
 * Input model: ./models/pro_st220.md
 */
# 3 "NBG_burs_template.c"

#include <stdio.h>
#include "NBG_defs.h"
#include "NBG_State.h"
#include "NBG_Link.h"
#include "NBG_burs.h"

# 2 "./models/pro_st220.md"
  
#ifdef TARG_ST /* PRO */
#ifndef __cplusplus
#error "should be compiled in cxx mode"
#endif

#include "defs.h"
#include "config.h"
#include "wn.h"

#include <stdarg.h>
static int assert_line = 0;
static const char *assert_file = "";
static const char *assert_cond = "";
static int
assert_set(const char *file, int line, const char *cond)
{
  assert_line = line;
  assert_file = file;
  assert_cond = cond;
  return 1;
}

static int
assert_fmt(const char *msg, ...)
{
  va_list p;
  fprintf(stderr, "CGG ASSERTION FAILED:\n");
  va_start(p, msg);
  vfprintf(stderr, msg, p);
  va_end(p);
  fprintf(stderr, "\nassertion failed: %s: %d: %s\n", assert_file, assert_line, assert_cond);	
  fflush(stderr);
  abort();
  return 0;
}

/* Defines errors exit. Must be of type int (assert is void). */
#define ASSERT_FMT(s, args) ((s) ? 1: \
	(assert_set(__FILE__, __LINE__, #s) && assert_fmt args))
#define ABORT_FMT(args) (assert_set(__FILE__, __LINE__, "") && assert_fmt args)

#else
#include "../pro/tn.h"
#include "../pro/tiny_op.h"
#include "../pro/tiny_wn.h"
#include "../common/debug.h"
/* from symtab_defs.h. */
enum ST_CLASS
{
    CLASS_UNK	= 0,
    CLASS_VAR	= 1,			// data variable
    CLASS_FUNC	= 2,			// addrress of a function.
    CLASS_CONST	= 3,			// constant value
    CLASS_PREG	= 4,			// pseudo register
    CLASS_BLOCK	= 5,			// base to a block of data
    CLASS_NAME  = 6,			// just hold an ST name
    CLASS_COUNT = 7			// total number of classes
}; // ST_CLASS

#define WN_class(x) (WN_st_idx(x) > 512 ? CLASS_UNK: CLASS_PREG)
#endif

/* 
 * Abstract Expression Tree definition.
 * We map this the WHIRL nodes (WN *).
 * The operators in the description are WHIRL operators OPR_...
 * The parameters are rtype and desc MTYPE_...
 */
#define NBG_TREE WN *
#define NBG_TREE_OP(tree) WN_operator(tree)
#ifdef TARG_ST
#define NBG_TREE_OP_STRING(op) OPERATOR_name((OPERATOR)(op))
#else
#define NBG_TREE_OP_STRING(op) ""
#endif
#define TERM_OP(op) OPR_##op	/* Builds ID of operators. */
#define NBG_TREE_ARITY(tree) WN_kid_count(tree)	
#define NBG_TREE_KID_AT(tree, i) WN_kid(tree, i)
#define NBG_TREE_PARAM_AT(tree, i) (i == 0 ? WN_rtype(tree) : WN_desc(tree))
#define TERM_PARAM(param) MTYPE_##param	/* Builds ID of param. */
#define NBG_TREE_VALUE_AT(tree, i) (i == 0 ? WN_const_val(tree) : i == 1 ? WN_cvtl_bits(tree) : WN_class(tree))
#define TERM_VALUE(value) (value)

/* Needed for uncse. */
#define NBG_LABEL_UNCSE
#define REUSE_COST CHECK_COST(NBG_reuse, 0)
#define NBG_REUSE_NTERM NTERM(reuse_reg)
#define USE_DEF(p) wn_use_def(p)
#define IS_USE(p) wn_is_use(p)
#define IS_DEF(p) wn_is_def(p)
#define IS_CLOBBER(p) wn_is_clobber(p)
#define LAST_DEF(p) wn_last_def(p)
#define SET_USE(p) wn_set_use(p)
#define SET_DEF(p) wn_set_def(p)
#define SET_CLOBBER(p) wn_set_clobber(p)
#define DEF_EXPR(p) wn_def_expr(p)
static int wn_is_use(WN *tree);
static int wn_is_def(WN *tree);
static int wn_is_clobber(WN *tree);
static int wn_set_clobber(WN *tree);
static int wn_set_use(WN *tree);
static int wn_set_def(WN *tree);
static WN * wn_def_expr(WN *tree);
static WN * wn_use_def(WN *tree);
static WN * wn_last_def(WN *tree);


#define STATE_WN ((NBG_TREE)NBG_State_utree(state))
#define STATE_OPC (WN_opcode(STATE_WN))
#define STATE_OPR (WN_operator(STATE_WN))

#define CHECK_COST(cond,cost) ((cond) ? (cost): NBG_COST_UNDEF)
#define KID0(t) (WN_kid(t,0))
#define KID1(t) (WN_kid(t,1))
#define KID2(t) (WN_kid(t,2))

#define FAIL_COST ASSERT_FMT(0,("CGG CODE SELECTION: unexpected WN node %s\n", OPCODE_name(STATE_OPC)))

/*
 * Trace level
 */
int CGG_trace_level;

/*
 * Debug level
 */
int CGG_debug_level;
#define DEBUG_LEVEL(level) (CGG_debug_level >= level) &&

/*
 * Parametrisation of cost depending on level.
 */
int CGG_opt_level;
#define CGG_LEVEL CGG_opt_level 

/*
 * Helpers for special cost calculation
 */
static int
is_power_of_2_u32(UINT32 val) 
{
  return (((val) != 0) && (((val) & ((val)-1)) == 0));
}

static int
get_power_of_2_u32 (UINT32 val)
{
  int i = -1;
  ASSERT_FMT(val != 0, ("get_power_of_2: val is zero"));
  while(val != 0) {
    val >>=1;
    i++;
  }
  return i;
}

#define GET_PW2(tree) (IS_CNSTNEG(tree) ? \
		      get_power_of_2_u32((UINT32)(-WN_const_val(tree))) : \
		      get_power_of_2_u32((UINT32)(WN_const_val(tree))))

#define IS_CNSTPW2S(tree) (IS_CNSTNEG(tree) && \
		           is_power_of_2_u32((UINT32)(-WN_const_val(tree))))
#define IS_CNSTPW2U(tree) (!IS_CNSTNEG(tree) && \
		           is_power_of_2_u32((UINT32)WN_const_val(tree)))
#define IS_CNSTNEG(tree) (MTYPE_is_signed(WN_rtype(tree)) && WN_const_val(tree) < 0)

/*
 * Some shortcuts for cost definition.
 */
#define ST_COST		1
#define LD_COST		3
#define ARI_COST	1
#define MUL32_COST	4
#define MUL_COST	3
#define DIV_COST	32
#define DIV_CNST_COST	6
#define ABS_COST	2
#define MMAX_COST	2
#define MADD_COST	4
#define FLT_COST	10
#define LOGIC_COST	1
#define SELECT_COST	1
#define CVT_COST	1
#define CVTL_COST	1
#define INTCONST_COST	1
#define CONST_COST	1
# 205 "pro_st220.gen.c"
# 11 "NBG_burs_template.c"

#ifdef __cplusplus
extern "C" {
#endif

/* 
 * Declaration of generated code .
 */

/* State labelling computation. */
static void NBG_compute_state(NBG_State state, void *user_state);

/* Chain rules closure computation. */
static void NBG_compute_closure(NBG_State state, int nterm);

/* Number of non terminals for a rule. */
#define NBG_Rule_nkids(r) (rule_nterms[r][0])

/* Access to the non terminal list for a rule. */
#define NBG_Rule_nterms(r) (&rule_nterms[r][1])

/* Acces to rule string. */
#define NBG_Rule_string(r) (rule_strings[r])

/* Acces to the kid path for a given rule and kid num. */
#define NBG_Rule_kid_path(r,k) (rule_kids[r][k])

/* Acces to emit actions. The function defs are external. */
#define NBG_emit_action(r) (NBG_emit_actions_[r])
typedef void (*NBG_emit_action_t) (NBG_Link, void *state);
typedef void (NBG_emit_action_f_t) (NBG_Link, void *state);

#ifdef NBG_LABEL_UNCSE
/* Set when the labelling should match NBG_REUSE_NTERM. */
static int NBG_reuse;
#endif

# 244 "pro_st220.gen.c"

/* Definition of non terminals. */
#define NTERM(nt) NTERM_##nt

enum {
  NTERM(NUL) = 0,
  NTERM(root),
  NTERM(rcon),
  NTERM(cnsti4),
  NTERM(stmt),
  NTERM(reg),
  NTERM(cond),
  NTERM(param),
  NTERM(inv),
  NTERM(cnsti4_ge8),
  NTERM(cnsti4_ge16),
  NTERM(cnsti4_ge24),
  NTERM(cnsti4_sx8),
  NTERM(cnsti4_sx16),
  NTERM(cnsti4_zx8),
  NTERM(cnsti4_zx16),
  NTERM(cnsti4_1),
  NTERM(cnsti4_2),
  NTERM(cnsti4_3),
  NTERM(cnsti4_4),
  NTERM(cnsti4_8),
  NTERM(cnsti4_16),
  NTERM(cnsti4_0xff),
  NTERM(cnsti4_0xffff),
  NTERM(cnstpw2u),
  NTERM(cnstpw2s),
  NTERM(m_reg_shl_1),
  NTERM(m_reg_shl_2),
  NTERM(m_reg_shl_3),
  NTERM(m_reg_shl_4),
  NTERM(m_reg_sx16h),
  NTERM(m_reg_zx16h),
  NTERM(m_reg_sx8),
  NTERM(reg_or_m_reg_sx8),
  NTERM(m_reg_zx8),
  NTERM(reg_or_m_reg_zx8),
  NTERM(m_reg_sx16),
  NTERM(reg_or_m_reg_sx16),
  NTERM(m_reg_zx16),
  NTERM(reg_or_m_reg_zx16),
  NTERM(reg_sx8),
  NTERM(reg_zx8),
  NTERM(reg_sx16),
  NTERM(reg_zx16),
  NTERM(reuse_reg),
  NTERM(COUNT)
  
};

/* Starting non terminal. */
static int NBG_Rule_start = NTERM(root);


/* Total number of rules. */
static int NBG_Rules_num = 313;

static const NBG_NTerm rule_nterms_1 [] = { 1,NTERM(stmt), }; 
static const NBG_NTerm rule_nterms_2 [] = { 1,NTERM(reg), }; 
static const NBG_NTerm rule_nterms_3 [] = { 1,NTERM(reg), }; 
static const NBG_NTerm rule_nterms_4 [] = { 0,}; 
static const NBG_NTerm rule_nterms_5 [] = { 1,NTERM(reg), }; 
static const NBG_NTerm rule_nterms_6 [] = { 1,NTERM(reg), }; 
static const NBG_NTerm rule_nterms_7 [] = { 1,NTERM(reg), }; 
static const NBG_NTerm rule_nterms_8 [] = { 2,NTERM(reg), NTERM(reg), }; 
static const NBG_NTerm rule_nterms_9 [] = { 3,NTERM(reg), NTERM(reg), NTERM(reg), }; 
static const NBG_NTerm rule_nterms_10 [] = { 1,NTERM(reg), }; 
static const NBG_NTerm rule_nterms_11 [] = { 1,NTERM(reg), }; 
static const NBG_NTerm rule_nterms_12 [] = { 1,NTERM(reg), }; 
static const NBG_NTerm rule_nterms_13 [] = { 2,NTERM(reg), NTERM(reg), }; 
static const NBG_NTerm rule_nterms_14 [] = { 0,}; 
static const NBG_NTerm rule_nterms_15 [] = { 0,}; 
static const NBG_NTerm rule_nterms_16 [] = { 0,}; 
static const NBG_NTerm rule_nterms_17 [] = { 1,NTERM(reg), }; 
static const NBG_NTerm rule_nterms_18 [] = { 0,}; 
static const NBG_NTerm rule_nterms_19 [] = { 2,NTERM(reg), NTERM(reg), }; 
static const NBG_NTerm rule_nterms_20 [] = { 0,}; 
static const NBG_NTerm rule_nterms_21 [] = { 0,}; 
static const NBG_NTerm rule_nterms_22 [] = { 0,}; 
static const NBG_NTerm rule_nterms_23 [] = { 1,NTERM(reg), }; 
static const NBG_NTerm rule_nterms_24 [] = { 0,}; 
static const NBG_NTerm rule_nterms_25 [] = { 0,}; 
static const NBG_NTerm rule_nterms_26 [] = { 0,}; 
static const NBG_NTerm rule_nterms_27 [] = { 2,NTERM(reg), NTERM(rcon), }; 
static const NBG_NTerm rule_nterms_28 [] = { 2,NTERM(rcon), NTERM(reg), }; 
static const NBG_NTerm rule_nterms_29 [] = { 2,NTERM(reg), NTERM(rcon), }; 
static const NBG_NTerm rule_nterms_30 [] = { 2,NTERM(rcon), NTERM(reg), }; 
static const NBG_NTerm rule_nterms_31 [] = { 1,NTERM(reg), }; 
static const NBG_NTerm rule_nterms_32 [] = { 2,NTERM(reg), NTERM(rcon), }; 
static const NBG_NTerm rule_nterms_33 [] = { 2,NTERM(reg), NTERM(rcon), }; 
static const NBG_NTerm rule_nterms_34 [] = { 2,NTERM(reg), NTERM(rcon), }; 
static const NBG_NTerm rule_nterms_35 [] = { 2,NTERM(reg), NTERM(rcon), }; 
static const NBG_NTerm rule_nterms_36 [] = { 2,NTERM(rcon), NTERM(reg), }; 
static const NBG_NTerm rule_nterms_37 [] = { 2,NTERM(reg), NTERM(cnsti4), }; 
static const NBG_NTerm rule_nterms_38 [] = { 2,NTERM(reg), NTERM(cnsti4), }; 
static const NBG_NTerm rule_nterms_39 [] = { 2,NTERM(reg), NTERM(reg), }; 
static const NBG_NTerm rule_nterms_40 [] = { 2,NTERM(reg), NTERM(reg), }; 
static const NBG_NTerm rule_nterms_41 [] = { 2,NTERM(reg), NTERM(rcon), }; 
static const NBG_NTerm rule_nterms_42 [] = { 1,NTERM(reg), }; 
static const NBG_NTerm rule_nterms_43 [] = { 2,NTERM(reg), NTERM(rcon), }; 
static const NBG_NTerm rule_nterms_44 [] = { 2,NTERM(rcon), NTERM(reg), }; 
static const NBG_NTerm rule_nterms_45 [] = { 2,NTERM(reg), NTERM(rcon), }; 
static const NBG_NTerm rule_nterms_46 [] = { 2,NTERM(rcon), NTERM(reg), }; 
static const NBG_NTerm rule_nterms_47 [] = { 3,NTERM(reg), NTERM(reg), NTERM(reg), }; 
static const NBG_NTerm rule_nterms_48 [] = { 3,NTERM(reg), NTERM(reg), NTERM(reg), }; 
static const NBG_NTerm rule_nterms_49 [] = { 3,NTERM(reg), NTERM(reg), NTERM(reg), }; 
static const NBG_NTerm rule_nterms_50 [] = { 3,NTERM(reg), NTERM(reg), NTERM(reg), }; 
static const NBG_NTerm rule_nterms_51 [] = { 1,NTERM(reg), }; 
static const NBG_NTerm rule_nterms_52 [] = { 2,NTERM(reg), NTERM(rcon), }; 
static const NBG_NTerm rule_nterms_53 [] = { 2,NTERM(rcon), NTERM(reg), }; 
static const NBG_NTerm rule_nterms_54 [] = { 2,NTERM(reg), NTERM(rcon), }; 
static const NBG_NTerm rule_nterms_55 [] = { 2,NTERM(rcon), NTERM(reg), }; 
static const NBG_NTerm rule_nterms_56 [] = { 2,NTERM(reg), NTERM(rcon), }; 
static const NBG_NTerm rule_nterms_57 [] = { 2,NTERM(rcon), NTERM(reg), }; 
static const NBG_NTerm rule_nterms_58 [] = { 2,NTERM(reg), NTERM(rcon), }; 
static const NBG_NTerm rule_nterms_59 [] = { 2,NTERM(rcon), NTERM(reg), }; 
static const NBG_NTerm rule_nterms_60 [] = { 1,NTERM(rcon), }; 
static const NBG_NTerm rule_nterms_61 [] = { 2,NTERM(reg), NTERM(rcon), }; 
static const NBG_NTerm rule_nterms_62 [] = { 2,NTERM(rcon), NTERM(reg), }; 
static const NBG_NTerm rule_nterms_63 [] = { 2,NTERM(reg), NTERM(rcon), }; 
static const NBG_NTerm rule_nterms_64 [] = { 2,NTERM(rcon), NTERM(reg), }; 
static const NBG_NTerm rule_nterms_65 [] = { 3,NTERM(cond), NTERM(reg), NTERM(rcon), }; 
static const NBG_NTerm rule_nterms_66 [] = { 3,NTERM(cond), NTERM(rcon), NTERM(reg), }; 
static const NBG_NTerm rule_nterms_67 [] = { 3,NTERM(reg), NTERM(reg), NTERM(reg), }; 
static const NBG_NTerm rule_nterms_68 [] = { 2,NTERM(reg), NTERM(rcon), }; 
static const NBG_NTerm rule_nterms_69 [] = { 2,NTERM(rcon), NTERM(rcon), }; 
static const NBG_NTerm rule_nterms_70 [] = { 2,NTERM(reg), NTERM(rcon), }; 
static const NBG_NTerm rule_nterms_71 [] = { 2,NTERM(rcon), NTERM(reg), }; 
static const NBG_NTerm rule_nterms_72 [] = { 2,NTERM(reg), NTERM(rcon), }; 
static const NBG_NTerm rule_nterms_73 [] = { 2,NTERM(rcon), NTERM(rcon), }; 
static const NBG_NTerm rule_nterms_74 [] = { 2,NTERM(reg), NTERM(rcon), }; 
static const NBG_NTerm rule_nterms_75 [] = { 2,NTERM(rcon), NTERM(reg), }; 
static const NBG_NTerm rule_nterms_76 [] = { 2,NTERM(reg), NTERM(rcon), }; 
static const NBG_NTerm rule_nterms_77 [] = { 2,NTERM(rcon), NTERM(reg), }; 
static const NBG_NTerm rule_nterms_78 [] = { 2,NTERM(reg), NTERM(rcon), }; 
static const NBG_NTerm rule_nterms_79 [] = { 2,NTERM(rcon), NTERM(reg), }; 
static const NBG_NTerm rule_nterms_80 [] = { 2,NTERM(reg), NTERM(rcon), }; 
static const NBG_NTerm rule_nterms_81 [] = { 2,NTERM(rcon), NTERM(rcon), }; 
static const NBG_NTerm rule_nterms_82 [] = { 2,NTERM(reg), NTERM(rcon), }; 
static const NBG_NTerm rule_nterms_83 [] = { 2,NTERM(rcon), NTERM(reg), }; 
static const NBG_NTerm rule_nterms_84 [] = { 2,NTERM(reg), NTERM(rcon), }; 
static const NBG_NTerm rule_nterms_85 [] = { 2,NTERM(rcon), NTERM(rcon), }; 
static const NBG_NTerm rule_nterms_86 [] = { 2,NTERM(reg), NTERM(rcon), }; 
static const NBG_NTerm rule_nterms_87 [] = { 2,NTERM(rcon), NTERM(reg), }; 
static const NBG_NTerm rule_nterms_88 [] = { 2,NTERM(reg), NTERM(rcon), }; 
static const NBG_NTerm rule_nterms_89 [] = { 2,NTERM(rcon), NTERM(reg), }; 
static const NBG_NTerm rule_nterms_90 [] = { 2,NTERM(reg), NTERM(rcon), }; 
static const NBG_NTerm rule_nterms_91 [] = { 2,NTERM(rcon), NTERM(reg), }; 
static const NBG_NTerm rule_nterms_92 [] = { 1,NTERM(reg), }; 
static const NBG_NTerm rule_nterms_93 [] = { 1,NTERM(reg), }; 
static const NBG_NTerm rule_nterms_94 [] = { 1,NTERM(reg), }; 
static const NBG_NTerm rule_nterms_95 [] = { 1,NTERM(reg), }; 
static const NBG_NTerm rule_nterms_96 [] = { 1,NTERM(reg), }; 
static const NBG_NTerm rule_nterms_97 [] = { 1,NTERM(reg), }; 
static const NBG_NTerm rule_nterms_98 [] = { 1,NTERM(reg), }; 
static const NBG_NTerm rule_nterms_99 [] = { 1,NTERM(reg), }; 
static const NBG_NTerm rule_nterms_100 [] = { 1,NTERM(reg), }; 
static const NBG_NTerm rule_nterms_101 [] = { 1,NTERM(reg), }; 
static const NBG_NTerm rule_nterms_102 [] = { 1,NTERM(reg), }; 
static const NBG_NTerm rule_nterms_103 [] = { 1,NTERM(reg), }; 
static const NBG_NTerm rule_nterms_104 [] = { 1,NTERM(reg), }; 
static const NBG_NTerm rule_nterms_105 [] = { 1,NTERM(reg), }; 
static const NBG_NTerm rule_nterms_106 [] = { 1,NTERM(reg), }; 
static const NBG_NTerm rule_nterms_107 [] = { 1,NTERM(reg), }; 
static const NBG_NTerm rule_nterms_108 [] = { 0,}; 
static const NBG_NTerm rule_nterms_109 [] = { 0,}; 
static const NBG_NTerm rule_nterms_110 [] = { 1,NTERM(reg), }; 
static const NBG_NTerm rule_nterms_111 [] = { 1,NTERM(reg), }; 
static const NBG_NTerm rule_nterms_112 [] = { 1,NTERM(reg), }; 
static const NBG_NTerm rule_nterms_113 [] = { 1,NTERM(reg), }; 
static const NBG_NTerm rule_nterms_114 [] = { 1,NTERM(cond), }; 
static const NBG_NTerm rule_nterms_115 [] = { 1,NTERM(cond), }; 
static const NBG_NTerm rule_nterms_116 [] = { 0,}; 
static const NBG_NTerm rule_nterms_117 [] = { 0,}; 
static const NBG_NTerm rule_nterms_118 [] = { 1,NTERM(reg), }; 
static const NBG_NTerm rule_nterms_119 [] = { 0,}; 
static const NBG_NTerm rule_nterms_120 [] = { 0,}; 
static const NBG_NTerm rule_nterms_121 [] = { 0,}; 
static const NBG_NTerm rule_nterms_122 [] = { 0,}; 
static const NBG_NTerm rule_nterms_123 [] = { 0,}; 
static const NBG_NTerm rule_nterms_124 [] = { 0,}; 
static const NBG_NTerm rule_nterms_125 [] = { 0,}; 
static const NBG_NTerm rule_nterms_126 [] = { 1,NTERM(reg), }; 
static const NBG_NTerm rule_nterms_127 [] = { 1,NTERM(reg), }; 
static const NBG_NTerm rule_nterms_128 [] = { 1,NTERM(reg), }; 
static const NBG_NTerm rule_nterms_129 [] = { 1,NTERM(reg), }; 
static const NBG_NTerm rule_nterms_130 [] = { 2,NTERM(reg), NTERM(reg), }; 
static const NBG_NTerm rule_nterms_131 [] = { 2,NTERM(reg), NTERM(reg), }; 
static const NBG_NTerm rule_nterms_132 [] = { 2,NTERM(reg), NTERM(reg), }; 
static const NBG_NTerm rule_nterms_133 [] = { 2,NTERM(reg), NTERM(reg), }; 
static const NBG_NTerm rule_nterms_134 [] = { 2,NTERM(reg), NTERM(reg), }; 
static const NBG_NTerm rule_nterms_135 [] = { 0,}; 
static const NBG_NTerm rule_nterms_136 [] = { 0,}; 
static const NBG_NTerm rule_nterms_137 [] = { 0,}; 
static const NBG_NTerm rule_nterms_138 [] = { 3,NTERM(reg), NTERM(reg), NTERM(reg), }; 
static const NBG_NTerm rule_nterms_139 [] = { 2,NTERM(reg), NTERM(reg), }; 
static const NBG_NTerm rule_nterms_140 [] = { 1,NTERM(inv), }; 
static const NBG_NTerm rule_nterms_141 [] = { 1,NTERM(inv), }; 
static const NBG_NTerm rule_nterms_142 [] = { 2,NTERM(inv), NTERM(inv), }; 
static const NBG_NTerm rule_nterms_143 [] = { 2,NTERM(inv), NTERM(inv), }; 
static const NBG_NTerm rule_nterms_144 [] = { 2,NTERM(inv), NTERM(inv), }; 
static const NBG_NTerm rule_nterms_145 [] = { 1,NTERM(inv), }; 
static const NBG_NTerm rule_nterms_146 [] = { 2,NTERM(inv), NTERM(inv), }; 
static const NBG_NTerm rule_nterms_147 [] = { 2,NTERM(inv), NTERM(inv), }; 
static const NBG_NTerm rule_nterms_148 [] = { 2,NTERM(inv), NTERM(inv), }; 
static const NBG_NTerm rule_nterms_149 [] = { 2,NTERM(inv), NTERM(inv), }; 
static const NBG_NTerm rule_nterms_150 [] = { 1,NTERM(inv), }; 
static const NBG_NTerm rule_nterms_151 [] = { 1,NTERM(inv), }; 
static const NBG_NTerm rule_nterms_152 [] = { 1,NTERM(inv), }; 
static const NBG_NTerm rule_nterms_153 [] = { 1,NTERM(inv), }; 
static const NBG_NTerm rule_nterms_154 [] = { 1,NTERM(inv), }; 
static const NBG_NTerm rule_nterms_155 [] = { 1,NTERM(inv), }; 
static const NBG_NTerm rule_nterms_156 [] = { 1,NTERM(inv), }; 
static const NBG_NTerm rule_nterms_157 [] = { 0,}; 
static const NBG_NTerm rule_nterms_158 [] = { 0,}; 
static const NBG_NTerm rule_nterms_159 [] = { 0,}; 
static const NBG_NTerm rule_nterms_160 [] = { 0,}; 
static const NBG_NTerm rule_nterms_161 [] = { 1,NTERM(cnsti4), }; 
static const NBG_NTerm rule_nterms_162 [] = { 0,}; 
static const NBG_NTerm rule_nterms_163 [] = { 0,}; 
static const NBG_NTerm rule_nterms_164 [] = { 0,}; 
static const NBG_NTerm rule_nterms_165 [] = { 0,}; 
static const NBG_NTerm rule_nterms_166 [] = { 0,}; 
static const NBG_NTerm rule_nterms_167 [] = { 1,NTERM(cnsti4_sx8), }; 
static const NBG_NTerm rule_nterms_168 [] = { 0,}; 
static const NBG_NTerm rule_nterms_169 [] = { 0,}; 
static const NBG_NTerm rule_nterms_170 [] = { 1,NTERM(cnsti4_zx8), }; 
static const NBG_NTerm rule_nterms_171 [] = { 0,}; 
static const NBG_NTerm rule_nterms_172 [] = { 0,}; 
static const NBG_NTerm rule_nterms_173 [] = { 0,}; 
static const NBG_NTerm rule_nterms_174 [] = { 0,}; 
static const NBG_NTerm rule_nterms_175 [] = { 0,}; 
static const NBG_NTerm rule_nterms_176 [] = { 0,}; 
static const NBG_NTerm rule_nterms_177 [] = { 0,}; 
static const NBG_NTerm rule_nterms_178 [] = { 0,}; 
static const NBG_NTerm rule_nterms_179 [] = { 0,}; 
static const NBG_NTerm rule_nterms_180 [] = { 0,}; 
static const NBG_NTerm rule_nterms_181 [] = { 2,NTERM(reg), NTERM(cnstpw2u), }; 
static const NBG_NTerm rule_nterms_182 [] = { 2,NTERM(cnstpw2u), NTERM(reg), }; 
static const NBG_NTerm rule_nterms_183 [] = { 2,NTERM(reg), NTERM(cnstpw2s), }; 
static const NBG_NTerm rule_nterms_184 [] = { 2,NTERM(cnstpw2s), NTERM(reg), }; 
static const NBG_NTerm rule_nterms_185 [] = { 2,NTERM(reg), NTERM(cnsti4_1), }; 
static const NBG_NTerm rule_nterms_186 [] = { 2,NTERM(reg), NTERM(cnsti4_2), }; 
static const NBG_NTerm rule_nterms_187 [] = { 2,NTERM(reg), NTERM(cnsti4_2), }; 
static const NBG_NTerm rule_nterms_188 [] = { 2,NTERM(reg), NTERM(cnsti4_4), }; 
static const NBG_NTerm rule_nterms_189 [] = { 2,NTERM(reg), NTERM(cnsti4_3), }; 
static const NBG_NTerm rule_nterms_190 [] = { 2,NTERM(reg), NTERM(cnsti4_8), }; 
static const NBG_NTerm rule_nterms_191 [] = { 2,NTERM(reg), NTERM(cnsti4_4), }; 
static const NBG_NTerm rule_nterms_192 [] = { 2,NTERM(reg), NTERM(cnsti4_16), }; 
static const NBG_NTerm rule_nterms_193 [] = { 2,NTERM(m_reg_shl_1), NTERM(rcon), }; 
static const NBG_NTerm rule_nterms_194 [] = { 2,NTERM(m_reg_shl_2), NTERM(rcon), }; 
static const NBG_NTerm rule_nterms_195 [] = { 2,NTERM(m_reg_shl_3), NTERM(rcon), }; 
static const NBG_NTerm rule_nterms_196 [] = { 2,NTERM(m_reg_shl_4), NTERM(rcon), }; 
static const NBG_NTerm rule_nterms_197 [] = { 3,NTERM(reg), NTERM(m_reg_zx16h), NTERM(cnsti4_16), }; 
static const NBG_NTerm rule_nterms_198 [] = { 3,NTERM(m_reg_zx16h), NTERM(reg), NTERM(cnsti4_16), }; 
static const NBG_NTerm rule_nterms_199 [] = { 2,NTERM(m_reg_sx16h), NTERM(m_reg_sx16h), }; 
static const NBG_NTerm rule_nterms_200 [] = { 2,NTERM(m_reg_zx16h), NTERM(m_reg_zx16h), }; 
static const NBG_NTerm rule_nterms_201 [] = { 2,NTERM(m_reg_sx16), NTERM(m_reg_sx16h), }; 
static const NBG_NTerm rule_nterms_202 [] = { 2,NTERM(m_reg_sx16h), NTERM(m_reg_sx16), }; 
static const NBG_NTerm rule_nterms_203 [] = { 2,NTERM(m_reg_zx16), NTERM(m_reg_zx16h), }; 
static const NBG_NTerm rule_nterms_204 [] = { 2,NTERM(m_reg_zx16h), NTERM(m_reg_zx16), }; 
static const NBG_NTerm rule_nterms_205 [] = { 2,NTERM(reg), NTERM(m_reg_sx16h), }; 
static const NBG_NTerm rule_nterms_206 [] = { 2,NTERM(m_reg_sx16h), NTERM(reg), }; 
static const NBG_NTerm rule_nterms_207 [] = { 2,NTERM(reg), NTERM(m_reg_zx16h), }; 
static const NBG_NTerm rule_nterms_208 [] = { 2,NTERM(m_reg_zx16h), NTERM(reg), }; 
static const NBG_NTerm rule_nterms_209 [] = { 2,NTERM(m_reg_sx16), NTERM(m_reg_sx16), }; 
static const NBG_NTerm rule_nterms_210 [] = { 2,NTERM(m_reg_sx16), NTERM(cnsti4_sx16), }; 
static const NBG_NTerm rule_nterms_211 [] = { 2,NTERM(cnsti4_sx16), NTERM(m_reg_sx16), }; 
static const NBG_NTerm rule_nterms_212 [] = { 2,NTERM(m_reg_zx16), NTERM(m_reg_zx16), }; 
static const NBG_NTerm rule_nterms_213 [] = { 2,NTERM(m_reg_zx16), NTERM(cnsti4_zx16), }; 
static const NBG_NTerm rule_nterms_214 [] = { 2,NTERM(cnsti4_zx16), NTERM(m_reg_zx16), }; 
static const NBG_NTerm rule_nterms_215 [] = { 2,NTERM(reg), NTERM(m_reg_sx16), }; 
static const NBG_NTerm rule_nterms_216 [] = { 2,NTERM(reg), NTERM(cnsti4_sx16), }; 
static const NBG_NTerm rule_nterms_217 [] = { 2,NTERM(m_reg_sx16), NTERM(reg), }; 
static const NBG_NTerm rule_nterms_218 [] = { 2,NTERM(cnsti4_sx16), NTERM(reg), }; 
static const NBG_NTerm rule_nterms_219 [] = { 2,NTERM(reg), NTERM(m_reg_zx16), }; 
static const NBG_NTerm rule_nterms_220 [] = { 2,NTERM(reg), NTERM(cnsti4_zx16), }; 
static const NBG_NTerm rule_nterms_221 [] = { 2,NTERM(m_reg_zx16), NTERM(reg), }; 
static const NBG_NTerm rule_nterms_222 [] = { 2,NTERM(cnsti4_zx16), NTERM(reg), }; 
static const NBG_NTerm rule_nterms_223 [] = { 2,NTERM(reg), NTERM(cnsti4_16), }; 
static const NBG_NTerm rule_nterms_224 [] = { 1,NTERM(m_reg_sx16h), }; 
static const NBG_NTerm rule_nterms_225 [] = { 3,NTERM(m_reg_sx16h), NTERM(cnsti4_16), NTERM(cnsti4_16), }; 
static const NBG_NTerm rule_nterms_226 [] = { 2,NTERM(reg), NTERM(cnsti4_16), }; 
static const NBG_NTerm rule_nterms_227 [] = { 1,NTERM(m_reg_zx16h), }; 
static const NBG_NTerm rule_nterms_228 [] = { 3,NTERM(m_reg_zx16h), NTERM(cnsti4_16), NTERM(cnsti4_16), }; 
static const NBG_NTerm rule_nterms_229 [] = { 2,NTERM(m_reg_zx16h), NTERM(cnsti4_0xffff), }; 
static const NBG_NTerm rule_nterms_230 [] = { 2,NTERM(m_reg_zx16h), NTERM(cnsti4_0xffff), }; 
static const NBG_NTerm rule_nterms_231 [] = { 1,NTERM(reg_sx8), }; 
static const NBG_NTerm rule_nterms_232 [] = { 1,NTERM(reg), }; 
static const NBG_NTerm rule_nterms_233 [] = { 1,NTERM(m_reg_sx8), }; 
static const NBG_NTerm rule_nterms_234 [] = { 1,NTERM(reg_or_m_reg_sx16), }; 
static const NBG_NTerm rule_nterms_235 [] = { 1,NTERM(reg_zx8), }; 
static const NBG_NTerm rule_nterms_236 [] = { 1,NTERM(reg), }; 
static const NBG_NTerm rule_nterms_237 [] = { 1,NTERM(m_reg_zx8), }; 
static const NBG_NTerm rule_nterms_238 [] = { 1,NTERM(reg_or_m_reg_zx8), }; 
static const NBG_NTerm rule_nterms_239 [] = { 2,NTERM(reg_or_m_reg_zx8), NTERM(cnsti4_0xff), }; 
static const NBG_NTerm rule_nterms_240 [] = { 1,NTERM(reg_sx16), }; 
static const NBG_NTerm rule_nterms_241 [] = { 1,NTERM(m_reg_sx8), }; 
static const NBG_NTerm rule_nterms_242 [] = { 1,NTERM(reg), }; 
static const NBG_NTerm rule_nterms_243 [] = { 1,NTERM(m_reg_sx16), }; 
static const NBG_NTerm rule_nterms_244 [] = { 1,NTERM(m_reg_sx16h), }; 
static const NBG_NTerm rule_nterms_245 [] = { 1,NTERM(reg_or_m_reg_sx16), }; 
static const NBG_NTerm rule_nterms_246 [] = { 3,NTERM(reg_or_m_reg_sx16), NTERM(cnsti4_16), NTERM(cnsti4_16), }; 
static const NBG_NTerm rule_nterms_247 [] = { 1,NTERM(reg_zx16), }; 
static const NBG_NTerm rule_nterms_248 [] = { 1,NTERM(m_reg_zx8), }; 
static const NBG_NTerm rule_nterms_249 [] = { 1,NTERM(reg), }; 
static const NBG_NTerm rule_nterms_250 [] = { 1,NTERM(m_reg_zx16), }; 
static const NBG_NTerm rule_nterms_251 [] = { 1,NTERM(m_reg_zx16h), }; 
static const NBG_NTerm rule_nterms_252 [] = { 1,NTERM(reg_or_m_reg_zx16), }; 
static const NBG_NTerm rule_nterms_253 [] = { 3,NTERM(reg_or_m_reg_zx16), NTERM(cnsti4_16), NTERM(cnsti4_16), }; 
static const NBG_NTerm rule_nterms_254 [] = { 2,NTERM(reg_or_m_reg_zx16), NTERM(cnsti4_0xffff), }; 
static const NBG_NTerm rule_nterms_255 [] = { 1,NTERM(reg_sx8), }; 
static const NBG_NTerm rule_nterms_256 [] = { 1,NTERM(reg), }; 
static const NBG_NTerm rule_nterms_257 [] = { 2,NTERM(reg), NTERM(cnsti4_ge24), }; 
static const NBG_NTerm rule_nterms_258 [] = { 2,NTERM(reg_sx16), NTERM(cnsti4_ge8), }; 
static const NBG_NTerm rule_nterms_259 [] = { 1,NTERM(reg_sx8), }; 
static const NBG_NTerm rule_nterms_260 [] = { 1,NTERM(reg_sx8), }; 
static const NBG_NTerm rule_nterms_261 [] = { 1,NTERM(cnsti4_sx8), }; 
static const NBG_NTerm rule_nterms_262 [] = { 1,NTERM(reg_zx8), }; 
static const NBG_NTerm rule_nterms_263 [] = { 1,NTERM(reg), }; 
static const NBG_NTerm rule_nterms_264 [] = { 2,NTERM(reg), NTERM(cnsti4_0xff), }; 
static const NBG_NTerm rule_nterms_265 [] = { 2,NTERM(reg), NTERM(cnsti4_ge24), }; 
static const NBG_NTerm rule_nterms_266 [] = { 2,NTERM(reg_zx16), NTERM(cnsti4_ge8), }; 
static const NBG_NTerm rule_nterms_267 [] = { 2,NTERM(reg_zx16), NTERM(cnsti4_ge8), }; 
static const NBG_NTerm rule_nterms_268 [] = { 1,NTERM(reg_zx8), }; 
static const NBG_NTerm rule_nterms_269 [] = { 1,NTERM(reg_zx8), }; 
static const NBG_NTerm rule_nterms_270 [] = { 1,NTERM(reg_zx8), }; 
static const NBG_NTerm rule_nterms_271 [] = { 2,NTERM(reg_zx8), NTERM(cnsti4_0xff), }; 
static const NBG_NTerm rule_nterms_272 [] = { 1,NTERM(cnsti4_zx8), }; 
static const NBG_NTerm rule_nterms_273 [] = { 1,NTERM(reg_sx16), }; 
static const NBG_NTerm rule_nterms_274 [] = { 1,NTERM(reg_sx8), }; 
static const NBG_NTerm rule_nterms_275 [] = { 1,NTERM(reg_zx8), }; 
static const NBG_NTerm rule_nterms_276 [] = { 1,NTERM(reg), }; 
static const NBG_NTerm rule_nterms_277 [] = { 3,NTERM(reg), NTERM(cnsti4_16), NTERM(cnsti4_16), }; 
static const NBG_NTerm rule_nterms_278 [] = { 2,NTERM(reg), NTERM(cnsti4_ge16), }; 
static const NBG_NTerm rule_nterms_279 [] = { 1,NTERM(reg_sx16), }; 
static const NBG_NTerm rule_nterms_280 [] = { 3,NTERM(reg_sx16), NTERM(cnsti4_16), NTERM(cnsti4_16), }; 
static const NBG_NTerm rule_nterms_281 [] = { 1,NTERM(cnsti4_sx16), }; 
static const NBG_NTerm rule_nterms_282 [] = { 1,NTERM(reg_zx16), }; 
static const NBG_NTerm rule_nterms_283 [] = { 1,NTERM(reg_zx8), }; 
static const NBG_NTerm rule_nterms_284 [] = { 1,NTERM(reg), }; 
static const NBG_NTerm rule_nterms_285 [] = { 3,NTERM(reg), NTERM(cnsti4_16), NTERM(cnsti4_16), }; 
static const NBG_NTerm rule_nterms_286 [] = { 2,NTERM(reg), NTERM(cnsti4_0xffff), }; 
static const NBG_NTerm rule_nterms_287 [] = { 2,NTERM(reg), NTERM(cnsti4_ge16), }; 
static const NBG_NTerm rule_nterms_288 [] = { 1,NTERM(reg_zx16), }; 
static const NBG_NTerm rule_nterms_289 [] = { 3,NTERM(reg_zx16), NTERM(cnsti4_16), NTERM(cnsti4_16), }; 
static const NBG_NTerm rule_nterms_290 [] = { 2,NTERM(reg_zx16), NTERM(cnsti4_0xffff), }; 
static const NBG_NTerm rule_nterms_291 [] = { 1,NTERM(cnsti4_zx16), }; 
static const NBG_NTerm rule_nterms_292 [] = { 0,}; 
static const NBG_NTerm rule_nterms_293 [] = { 0,}; 
static const NBG_NTerm rule_nterms_294 [] = { 0,}; 
static const NBG_NTerm rule_nterms_295 [] = { 1,NTERM(reg), }; 
static const NBG_NTerm rule_nterms_296 [] = { 0,}; 
static const NBG_NTerm rule_nterms_297 [] = { 0,}; 
static const NBG_NTerm rule_nterms_298 [] = { 0,}; 
static const NBG_NTerm rule_nterms_299 [] = { 1,NTERM(reg), }; 
static const NBG_NTerm rule_nterms_300 [] = { 0,}; 
static const NBG_NTerm rule_nterms_301 [] = { 0,}; 
static const NBG_NTerm rule_nterms_302 [] = { 0,}; 
static const NBG_NTerm rule_nterms_303 [] = { 1,NTERM(reg), }; 
static const NBG_NTerm rule_nterms_304 [] = { 0,}; 
static const NBG_NTerm rule_nterms_305 [] = { 0,}; 
static const NBG_NTerm rule_nterms_306 [] = { 0,}; 
static const NBG_NTerm rule_nterms_307 [] = { 1,NTERM(reg), }; 
static const NBG_NTerm rule_nterms_308 [] = { 1,NTERM(reuse_reg), }; 
static const NBG_NTerm rule_nterms_309 [] = { 1,NTERM(reuse_reg), }; 
static const NBG_NTerm rule_nterms_310 [] = { 1,NTERM(reuse_reg), }; 
static const NBG_NTerm rule_nterms_311 [] = { 1,NTERM(reuse_reg), }; 
static const NBG_NTerm rule_nterms_312 [] = { 1,NTERM(reuse_reg), }; 
static const NBG_NTerm rule_nterms_313 [] = { 0,}; 

/* Rule ids. */
enum {
  NBG_RULE_UNKNOWN = 0,
  NBG_RULE_root_stmt,
  NBG_RULE_root_reg,
  NBG_RULE_rcon_reg,
  NBG_RULE_cnsti4_INTCONST,
  NBG_RULE_stmt_STID___CLASS__PREG_reg_,
  NBG_RULE_stmt_STID_reg_,
  NBG_RULE_stmt_ISTORE_reg_LDA_,
  NBG_RULE_stmt_ISTORE_reg_reg_,
  NBG_RULE_stmt_ISTOREX_reg_reg_reg_,
  NBG_RULE_stmt_STBITS___CLASS__PREG_reg_,
  NBG_RULE_stmt_STBITS_reg_,
  NBG_RULE_stmt_ISTBITS_reg_LDA_,
  NBG_RULE_stmt_ISTBITS_reg_reg_,
  NBG_RULE_reg_LDID___CLASS__PREG,
  NBG_RULE_reg_LDID,
  NBG_RULE_reg_ILOAD_LDA_,
  NBG_RULE_reg_ILOAD_reg_,
  NBG_RULE_reg_LDA,
  NBG_RULE_reg_ILOADX_reg_reg_,
  NBG_RULE_reg_LDBITS___CLASS__PREG,
  NBG_RULE_reg_LDBITS,
  NBG_RULE_reg_ILDBITS_LDA_,
  NBG_RULE_reg_ILDBITS_reg_,
  NBG_RULE_reg_INTCONST_0,
  NBG_RULE_reg_INTCONST,
  NBG_RULE_reg_CONST,
  NBG_RULE_reg_ADD_reg_rcon_,
  NBG_RULE_reg_ADD_rcon_reg_,
  NBG_RULE_reg_SUB_reg_rcon_,
  NBG_RULE_reg_SUB_rcon_reg_,
  NBG_RULE_reg_NEG_reg_,
  NBG_RULE_reg_ASHR_reg_rcon_,
  NBG_RULE_reg_LSHR_reg_rcon_,
  NBG_RULE_reg_SHL_reg_rcon_,
  NBG_RULE_reg_MPY_reg_rcon_,
  NBG_RULE_reg_MPY_rcon_reg_,
  NBG_RULE_reg_DIV_reg_cnsti4_,
  NBG_RULE_reg_REM_reg_cnsti4_,
  NBG_RULE_reg_DIV_reg_reg_,
  NBG_RULE_reg_REM_reg_reg_,
  NBG_RULE_reg_MOD_reg_rcon_,
  NBG_RULE_reg_ABS_reg_,
  NBG_RULE_reg_MAX_reg_rcon_,
  NBG_RULE_reg_MAX_rcon_reg_,
  NBG_RULE_reg_MIN_reg_rcon_,
  NBG_RULE_reg_MIN_rcon_reg_,
  NBG_RULE_reg_MADD_reg_reg_reg_,
  NBG_RULE_reg_MSUB_reg_reg_reg_,
  NBG_RULE_reg_NMADD_reg_reg_reg_,
  NBG_RULE_reg_NMSUB_reg_reg_reg_,
  NBG_RULE_reg_BNOT_reg_,
  NBG_RULE_reg_BAND_reg_rcon_,
  NBG_RULE_reg_BAND_rcon_reg_,
  NBG_RULE_reg_BIOR_reg_rcon_,
  NBG_RULE_reg_BIOR_rcon_reg_,
  NBG_RULE_reg_BXOR_reg_rcon_,
  NBG_RULE_reg_BXOR_rcon_reg_,
  NBG_RULE_reg_BNOR_reg_rcon_,
  NBG_RULE_reg_BNOR_rcon_reg_,
  NBG_RULE_reg_LNOT_rcon_,
  NBG_RULE_reg_LAND_reg_rcon_,
  NBG_RULE_reg_LAND_rcon_reg_,
  NBG_RULE_reg_LIOR_reg_rcon_,
  NBG_RULE_reg_LIOR_rcon_reg_,
  NBG_RULE_reg_SELECT_cond_reg_rcon_,
  NBG_RULE_reg_SELECT_cond_rcon_reg_,
  NBG_RULE_reg_SELECT_reg_reg_reg_,
  NBG_RULE_cond_EQ_reg_rcon_,
  NBG_RULE_cond_EQ_rcon_rcon_,
  NBG_RULE_cond_NE_reg_rcon_,
  NBG_RULE_cond_NE_rcon_reg_,
  NBG_RULE_cond_GT_reg_rcon_,
  NBG_RULE_cond_GT_rcon_rcon_,
  NBG_RULE_cond_GE_reg_rcon_,
  NBG_RULE_cond_GE_rcon_reg_,
  NBG_RULE_cond_LT_reg_rcon_,
  NBG_RULE_cond_LT_rcon_reg_,
  NBG_RULE_cond_LE_reg_rcon_,
  NBG_RULE_cond_LE_rcon_reg_,
  NBG_RULE_reg_EQ_reg_rcon_,
  NBG_RULE_reg_EQ_rcon_rcon_,
  NBG_RULE_reg_NE_reg_rcon_,
  NBG_RULE_reg_NE_rcon_reg_,
  NBG_RULE_reg_GT_reg_rcon_,
  NBG_RULE_reg_GT_rcon_rcon_,
  NBG_RULE_reg_GE_reg_rcon_,
  NBG_RULE_reg_GE_rcon_reg_,
  NBG_RULE_reg_LT_reg_rcon_,
  NBG_RULE_reg_LT_rcon_reg_,
  NBG_RULE_reg_LE_reg_rcon_,
  NBG_RULE_reg_LE_rcon_reg_,
  NBG_RULE_reg_SQRT_reg_,
  NBG_RULE_reg_RSQRT_reg_,
  NBG_RULE_reg_RECIP_reg_,
  NBG_RULE_reg_RND_reg_,
  NBG_RULE_reg_TRUNC_reg_,
  NBG_RULE_reg_CEIL_reg_,
  NBG_RULE_reg_FLOOR_reg_,
  NBG_RULE_reg_CVTL_I4__16_reg_,
  NBG_RULE_reg_CVTL_U4__16_reg_,
  NBG_RULE_reg_CVTL_I4__8_reg_,
  NBG_RULE_reg_CVTL_U4__8_reg_,
  NBG_RULE_reg_CVTL_reg_,
  NBG_RULE_reg_CVT_reg_,
  NBG_RULE_reg_TAS_reg_,
  NBG_RULE_reg_PAREN_reg_,
  NBG_RULE_reg_EXTRACT__BITS_reg_,
  NBG_RULE_stmt_GOTO,
  NBG_RULE_stmt_REGION__EXIT,
  NBG_RULE_stmt_AGOTO_reg_,
  NBG_RULE_stmt_XGOTO_reg_,
  NBG_RULE_stmt_TRUEBR_reg_,
  NBG_RULE_stmt_FALSEBR_reg_,
  NBG_RULE_stmt_TRUEBR_cond_,
  NBG_RULE_stmt_FALSEBR_cond_,
  NBG_RULE_stmt_TRUEBR_INTCONST_,
  NBG_RULE_stmt_FALSEBR_INTCONST_,
  NBG_RULE_param_PARM_reg_,
  NBG_RULE_stmt_CALL_V,
  NBG_RULE_stmt_CALL,
  NBG_RULE_stmt_ICALL_V,
  NBG_RULE_stmt_ICALL,
  NBG_RULE_stmt_RETURN,
  NBG_RULE_stmt_PICCALL_V,
  NBG_RULE_stmt_PICCALL,
  NBG_RULE_reg_LOWPART_reg_,
  NBG_RULE_reg_HIGHPART_reg_,
  NBG_RULE_reg_MINPART_reg_,
  NBG_RULE_reg_MAXPART_reg_,
  NBG_RULE_reg_COMPOSE__BITS_reg_reg_,
  NBG_RULE_reg_HIGHMPY_reg_reg_,
  NBG_RULE_reg_XMPY_reg_reg_,
  NBG_RULE_reg_DIVREM_reg_reg_,
  NBG_RULE_reg_MINMAX_reg_reg_,
  NBG_RULE_reg_LDMA,
  NBG_RULE_reg_LDA__LABEL,
  NBG_RULE_stmt_IDNAME,
  NBG_RULE_stmt_MSTORE_reg_reg_reg_,
  NBG_RULE_reg_MLOAD_reg_reg_,
  NBG_RULE_inv_FIRSTPART_inv_,
  NBG_RULE_inv_SECONDPART_inv_,
  NBG_RULE_inv_CAND_inv_inv_,
  NBG_RULE_inv_CIOR_inv_inv_,
  NBG_RULE_inv_CSELECT_inv_inv_,
  NBG_RULE_inv_ILDA_inv_,
  NBG_RULE_inv_PAIR_inv_inv_,
  NBG_RULE_inv_RROTATE_inv_inv_,
  NBG_RULE_inv_COMMA_inv_inv_,
  NBG_RULE_inv_RCOMMA_inv_inv_,
  NBG_RULE_inv_ARRAY_inv_,
  NBG_RULE_inv_INTRINSIC__OP_inv_,
  NBG_RULE_inv_IO__ITEM_inv_,
  NBG_RULE_inv_TRIPLET_inv_,
  NBG_RULE_inv_ARRAYEXP_inv_,
  NBG_RULE_inv_ARRSECTION_inv_,
  NBG_RULE_inv_WHERE_inv_,
  NBG_RULE_stmt_FORWARD__BARRIER,
  NBG_RULE_stmt_BACKWARD__BARRIER,
  NBG_RULE_stmt_INTRINSIC__CALL_V,
  NBG_RULE_reg_INTRINSIC__CALL,
  NBG_RULE_rcon_cnsti4,
  NBG_RULE_cnsti4__ge8_INTCONST_I4,
  NBG_RULE_cnsti4__ge16_INTCONST_I4,
  NBG_RULE_cnsti4__ge24_INTCONST_I4,
  NBG_RULE_cnsti4__sx8_INTCONST_I4,
  NBG_RULE_cnsti4__sx16_INTCONST_I4,
  NBG_RULE_cnsti4__sx16_cnsti4__sx8,
  NBG_RULE_cnsti4__zx8_INTCONST_I4,
  NBG_RULE_cnsti4__zx16_INTCONST_I4,
  NBG_RULE_cnsti4__zx16_cnsti4__zx8,
  NBG_RULE_cnsti4__1_INTCONST_I4_1,
  NBG_RULE_cnsti4__2_INTCONST_I4_2,
  NBG_RULE_cnsti4__3_INTCONST_I4_3,
  NBG_RULE_cnsti4__4_INTCONST_I4_4,
  NBG_RULE_cnsti4__8_INTCONST_I4_8,
  NBG_RULE_cnsti4__16_INTCONST_I4_16,
  NBG_RULE_cnsti4__0xff_INTCONST_I4_0xff,
  NBG_RULE_cnsti4__0xffff_INTCONST_I4_0xffff,
  NBG_RULE_cnstpw2u_INTCONST,
  NBG_RULE_cnstpw2s_INTCONST,
  NBG_RULE_reg_MPY_reg_cnstpw2u_,
  NBG_RULE_reg_MPY_cnstpw2u_reg_,
  NBG_RULE_reg_MPY_reg_cnstpw2s_,
  NBG_RULE_reg_MPY_cnstpw2s_reg_,
  NBG_RULE_m__reg__shl__1_SHL_reg_cnsti4__1_,
  NBG_RULE_m__reg__shl__1_MPY_reg_cnsti4__2_,
  NBG_RULE_m__reg__shl__2_SHL_reg_cnsti4__2_,
  NBG_RULE_m__reg__shl__2_MPY_reg_cnsti4__4_,
  NBG_RULE_m__reg__shl__3_SHL_reg_cnsti4__3_,
  NBG_RULE_m__reg__shl__3_MPY_reg_cnsti4__8_,
  NBG_RULE_m__reg__shl__4_SHL_reg_cnsti4__4_,
  NBG_RULE_m__reg__shl__3_MPY_reg_cnsti4__16_,
  NBG_RULE_reg_ADD_m__reg__shl__1_rcon_,
  NBG_RULE_reg_ADD_m__reg__shl__2_rcon_,
  NBG_RULE_reg_ADD_m__reg__shl__3_rcon_,
  NBG_RULE_reg_ADD_m__reg__shl__4_rcon_,
  NBG_RULE_reg_SHL_MPY_reg_m__reg__zx16h__cnsti4__16_,
  NBG_RULE_reg_SHL_MPY_m__reg__zx16h_reg__cnsti4__16_,
  NBG_RULE_reg_MPY_m__reg__sx16h_m__reg__sx16h_,
  NBG_RULE_reg_MPY_m__reg__zx16h_m__reg__zx16h_,
  NBG_RULE_reg_MPY_m__reg__sx16_m__reg__sx16h_,
  NBG_RULE_reg_MPY_m__reg__sx16h_m__reg__sx16_,
  NBG_RULE_reg_MPY_m__reg__zx16_m__reg__zx16h_,
  NBG_RULE_reg_MPY_m__reg__zx16h_m__reg__zx16_,
  NBG_RULE_reg_MPY_reg_m__reg__sx16h_,
  NBG_RULE_reg_MPY_m__reg__sx16h_reg_,
  NBG_RULE_reg_MPY_reg_m__reg__zx16h_,
  NBG_RULE_reg_MPY_m__reg__zx16h_reg_,
  NBG_RULE_reg_MPY_m__reg__sx16_m__reg__sx16_,
  NBG_RULE_reg_MPY_m__reg__sx16_cnsti4__sx16_,
  NBG_RULE_reg_MPY_cnsti4__sx16_m__reg__sx16_,
  NBG_RULE_reg_MPY_m__reg__zx16_m__reg__zx16_,
  NBG_RULE_reg_MPY_m__reg__zx16_cnsti4__zx16_,
  NBG_RULE_reg_MPY_cnsti4__zx16_m__reg__zx16_,
  NBG_RULE_reg_MPY_reg_m__reg__sx16_,
  NBG_RULE_reg_MPY_reg_cnsti4__sx16_,
  NBG_RULE_reg_MPY_m__reg__sx16_reg_,
  NBG_RULE_reg_MPY_cnsti4__sx16_reg_,
  NBG_RULE_reg_MPY_reg_m__reg__zx16_,
  NBG_RULE_reg_MPY_reg_cnsti4__zx16_,
  NBG_RULE_reg_MPY_m__reg__zx16_reg_,
  NBG_RULE_reg_MPY_cnsti4__zx16_reg_,
  NBG_RULE_m__reg__sx16h_ASHR_I4_reg_cnsti4__16_,
  NBG_RULE_m__reg__sx16h_CVTL_I4__16_m__reg__sx16h_,
  NBG_RULE_m__reg__sx16h_ASHR_I4_SHL_m__reg__sx16h_cnsti4__16__cnsti4__16_,
  NBG_RULE_m__reg__zx16h_LSHR_U4_reg_cnsti4__16_,
  NBG_RULE_m__reg__zx16h_CVTL_U4__16_m__reg__zx16h_,
  NBG_RULE_m__reg__zx16h_LSHR_U4_SHL_m__reg__zx16h_cnsti4__16__cnsti4__16_,
  NBG_RULE_m__reg__zx16h_BAND_I4_m__reg__zx16h_cnsti4__0xffff_,
  NBG_RULE_m__reg__zx16h_BAND_U4_m__reg__zx16h_cnsti4__0xffff_,
  NBG_RULE_m__reg__sx8_reg__sx8,
  NBG_RULE_reg__or__m__reg__sx8_reg,
  NBG_RULE_reg__or__m__reg__sx8_m__reg__sx8,
  NBG_RULE_m__reg__sx8_CVTL_I4__8_reg__or__m__reg__sx16_,
  NBG_RULE_m__reg__zx8_reg__zx8,
  NBG_RULE_reg__or__m__reg__zx8_reg,
  NBG_RULE_reg__or__m__reg__zx8_m__reg__zx8,
  NBG_RULE_m__reg__zx8_CVTL_U4__8_reg__or__m__reg__zx8_,
  NBG_RULE_m__reg__zx8_BAND__reg__or__m__reg__zx8_cnsti4__0xff_,
  NBG_RULE_m__reg__sx16_reg__sx16,
  NBG_RULE_m__reg__sx16_m__reg__sx8,
  NBG_RULE_reg__or__m__reg__sx16_reg,
  NBG_RULE_reg__or__m__reg__sx16_m__reg__sx16,
  NBG_RULE_m__reg__sx16_m__reg__sx16h,
  NBG_RULE_m__reg__sx16_CVTL_I4__16_reg__or__m__reg__sx16_,
  NBG_RULE_m__reg__sx16_ASHR_I4_SHL_reg__or__m__reg__sx16_cnsti4__16__cnsti4__16_,
  NBG_RULE_m__reg__zx16_reg__zx16,
  NBG_RULE_m__reg__zx16_m__reg__zx8,
  NBG_RULE_reg__or__m__reg__zx16_reg,
  NBG_RULE_reg__or__m__reg__zx16_m__reg__zx16,
  NBG_RULE_m__reg__zx16_m__reg__zx16h,
  NBG_RULE_m__reg__zx16_CVTL_U4__16_reg__or__m__reg__zx16_,
  NBG_RULE_m__reg__zx16_LSHR_U4_SHL_reg__or__m__reg__zx16_cnsti4__16__cnsti4__16_,
  NBG_RULE_m__reg__zx16_BAND__reg__or__m__reg__zx16_cnsti4__0xffff_,
  NBG_RULE_reg_reg__sx8,
  NBG_RULE_reg__sx8_CVTL_I4__8_reg_,
  NBG_RULE_reg__sx8_ASHR_I4_reg_cnsti4__ge24_,
  NBG_RULE_reg__sx8_ASHR_I4_reg__sx16_cnsti4__ge8_,
  NBG_RULE_reg__sx8_CVTL_I4__8_reg__sx8_,
  NBG_RULE_reg__sx8_CVTL_I4__16_reg__sx8_,
  NBG_RULE_reg__sx8_cnsti4__sx8,
  NBG_RULE_reg_reg__zx8,
  NBG_RULE_reg__zx8_CVTL_U4__8_reg_,
  NBG_RULE_reg__zx8_BAND_reg_cnsti4__0xff_,
  NBG_RULE_reg__zx8_LSHR_U4_reg_cnsti4__ge24_,
  NBG_RULE_reg__zx8_LSHR_U4_reg__zx16_cnsti4__ge8_,
  NBG_RULE_reg__zx8_ASHR_I4_reg__zx16_cnsti4__ge8_,
  NBG_RULE_reg__zx8_CVTL_U4__8_reg__zx8_,
  NBG_RULE_reg__zx8_CVTL_U4__16_reg__zx8_,
  NBG_RULE_reg__zx8_CVTL_I4__16_reg__zx8_,
  NBG_RULE_reg__zx8_BAND_reg__zx8_cnsti4__0xff_,
  NBG_RULE_reg__zx8_cnsti4__zx8,
  NBG_RULE_reg_reg__sx16,
  NBG_RULE_reg__sx16_reg__sx8,
  NBG_RULE_reg__sx16_reg__zx8,
  NBG_RULE_reg__sx16_CVTL_I4__16_reg_,
  NBG_RULE_reg__sx16_ASHR_I4_SHL_reg_cnsti4__16__cnsti4__16_,
  NBG_RULE_reg__sx16_ASHR_I4_reg_cnsti4__ge16_,
  NBG_RULE_reg__sx16_CVTL_I4__16_reg__sx16_,
  NBG_RULE_reg__sx16_ASHR_I4_SHL_reg__sx16_cnsti4__16__cnsti4__16_,
  NBG_RULE_reg__sx16_cnsti4__sx16,
  NBG_RULE_reg_reg__zx16,
  NBG_RULE_reg__zx16_reg__zx8,
  NBG_RULE_reg__zx16_CVTL_U4__16_reg_,
  NBG_RULE_reg__zx16_LSHR_U4_SHL_reg_cnsti4__16__cnsti4__16_,
  NBG_RULE_reg__zx16_BAND_reg_cnsti4__0xffff_,
  NBG_RULE_reg__zx16_LSHR_U4_reg_cnsti4__ge16_,
  NBG_RULE_reg__zx16_CVTL_U4__16_reg__zx16_,
  NBG_RULE_reg__zx16_LSHR_U4_SHL_reg__zx16_cnsti4__16__cnsti4__16_,
  NBG_RULE_reg__zx16_BAND_reg__zx16_cnsti4__0xffff_,
  NBG_RULE_reg__zx16_cnsti4__zx16,
  NBG_RULE_reg__zx16_LDID_U4_U2___CLASS__PREG,
  NBG_RULE_reg__zx16_LDID_U4_U2,
  NBG_RULE_reg__zx16_ILOAD_U4_U2_LDA_,
  NBG_RULE_reg__zx16_ILOAD_U4_U2_reg_,
  NBG_RULE_reg__sx16_LDID_I4_I1___CLASS__PREG,
  NBG_RULE_reg__sx16_LDID_I4_I1,
  NBG_RULE_reg__sx16_ILOAD_I4_I1_LDA_,
  NBG_RULE_reg__sx16_ILOAD_I4_I1_reg_,
  NBG_RULE_reg__zx8_LDID_U4_U1___CLASS__PREG,
  NBG_RULE_reg__zx8_LDID_U4_U1,
  NBG_RULE_reg__zx8_ILOAD_U4_U1_LDA_,
  NBG_RULE_reg__zx8_ILOAD_U4_U1_reg_,
  NBG_RULE_reg__sx8_LDID_I4_I1___CLASS__PREG,
  NBG_RULE_reg__sx8_LDID_I4_I1,
  NBG_RULE_reg__sx8_ILOAD_I4_I1_LDA_,
  NBG_RULE_reg__sx8_ILOAD_I4_I1_reg_,
  NBG_RULE_reg_reuse__reg,
  NBG_RULE_reg__sx16_reuse__reg,
  NBG_RULE_reg__zx16_reuse__reg,
  NBG_RULE_reg__sx8_reuse__reg,
  NBG_RULE_reg__zx8_reuse__reg,
  NBG_RULE_reuse__reg_LDID___CLASS__PREG,
  
};

static const NBG_NTerm * const rule_nterms[] = {
  (const NBG_NTerm *)0,
  rule_nterms_1, 
  rule_nterms_2, 
  rule_nterms_3, 
  rule_nterms_4, 
  rule_nterms_5, 
  rule_nterms_6, 
  rule_nterms_7, 
  rule_nterms_8, 
  rule_nterms_9, 
  rule_nterms_10, 
  rule_nterms_11, 
  rule_nterms_12, 
  rule_nterms_13, 
  rule_nterms_14, 
  rule_nterms_15, 
  rule_nterms_16, 
  rule_nterms_17, 
  rule_nterms_18, 
  rule_nterms_19, 
  rule_nterms_20, 
  rule_nterms_21, 
  rule_nterms_22, 
  rule_nterms_23, 
  rule_nterms_24, 
  rule_nterms_25, 
  rule_nterms_26, 
  rule_nterms_27, 
  rule_nterms_28, 
  rule_nterms_29, 
  rule_nterms_30, 
  rule_nterms_31, 
  rule_nterms_32, 
  rule_nterms_33, 
  rule_nterms_34, 
  rule_nterms_35, 
  rule_nterms_36, 
  rule_nterms_37, 
  rule_nterms_38, 
  rule_nterms_39, 
  rule_nterms_40, 
  rule_nterms_41, 
  rule_nterms_42, 
  rule_nterms_43, 
  rule_nterms_44, 
  rule_nterms_45, 
  rule_nterms_46, 
  rule_nterms_47, 
  rule_nterms_48, 
  rule_nterms_49, 
  rule_nterms_50, 
  rule_nterms_51, 
  rule_nterms_52, 
  rule_nterms_53, 
  rule_nterms_54, 
  rule_nterms_55, 
  rule_nterms_56, 
  rule_nterms_57, 
  rule_nterms_58, 
  rule_nterms_59, 
  rule_nterms_60, 
  rule_nterms_61, 
  rule_nterms_62, 
  rule_nterms_63, 
  rule_nterms_64, 
  rule_nterms_65, 
  rule_nterms_66, 
  rule_nterms_67, 
  rule_nterms_68, 
  rule_nterms_69, 
  rule_nterms_70, 
  rule_nterms_71, 
  rule_nterms_72, 
  rule_nterms_73, 
  rule_nterms_74, 
  rule_nterms_75, 
  rule_nterms_76, 
  rule_nterms_77, 
  rule_nterms_78, 
  rule_nterms_79, 
  rule_nterms_80, 
  rule_nterms_81, 
  rule_nterms_82, 
  rule_nterms_83, 
  rule_nterms_84, 
  rule_nterms_85, 
  rule_nterms_86, 
  rule_nterms_87, 
  rule_nterms_88, 
  rule_nterms_89, 
  rule_nterms_90, 
  rule_nterms_91, 
  rule_nterms_92, 
  rule_nterms_93, 
  rule_nterms_94, 
  rule_nterms_95, 
  rule_nterms_96, 
  rule_nterms_97, 
  rule_nterms_98, 
  rule_nterms_99, 
  rule_nterms_100, 
  rule_nterms_101, 
  rule_nterms_102, 
  rule_nterms_103, 
  rule_nterms_104, 
  rule_nterms_105, 
  rule_nterms_106, 
  rule_nterms_107, 
  rule_nterms_108, 
  rule_nterms_109, 
  rule_nterms_110, 
  rule_nterms_111, 
  rule_nterms_112, 
  rule_nterms_113, 
  rule_nterms_114, 
  rule_nterms_115, 
  rule_nterms_116, 
  rule_nterms_117, 
  rule_nterms_118, 
  rule_nterms_119, 
  rule_nterms_120, 
  rule_nterms_121, 
  rule_nterms_122, 
  rule_nterms_123, 
  rule_nterms_124, 
  rule_nterms_125, 
  rule_nterms_126, 
  rule_nterms_127, 
  rule_nterms_128, 
  rule_nterms_129, 
  rule_nterms_130, 
  rule_nterms_131, 
  rule_nterms_132, 
  rule_nterms_133, 
  rule_nterms_134, 
  rule_nterms_135, 
  rule_nterms_136, 
  rule_nterms_137, 
  rule_nterms_138, 
  rule_nterms_139, 
  rule_nterms_140, 
  rule_nterms_141, 
  rule_nterms_142, 
  rule_nterms_143, 
  rule_nterms_144, 
  rule_nterms_145, 
  rule_nterms_146, 
  rule_nterms_147, 
  rule_nterms_148, 
  rule_nterms_149, 
  rule_nterms_150, 
  rule_nterms_151, 
  rule_nterms_152, 
  rule_nterms_153, 
  rule_nterms_154, 
  rule_nterms_155, 
  rule_nterms_156, 
  rule_nterms_157, 
  rule_nterms_158, 
  rule_nterms_159, 
  rule_nterms_160, 
  rule_nterms_161, 
  rule_nterms_162, 
  rule_nterms_163, 
  rule_nterms_164, 
  rule_nterms_165, 
  rule_nterms_166, 
  rule_nterms_167, 
  rule_nterms_168, 
  rule_nterms_169, 
  rule_nterms_170, 
  rule_nterms_171, 
  rule_nterms_172, 
  rule_nterms_173, 
  rule_nterms_174, 
  rule_nterms_175, 
  rule_nterms_176, 
  rule_nterms_177, 
  rule_nterms_178, 
  rule_nterms_179, 
  rule_nterms_180, 
  rule_nterms_181, 
  rule_nterms_182, 
  rule_nterms_183, 
  rule_nterms_184, 
  rule_nterms_185, 
  rule_nterms_186, 
  rule_nterms_187, 
  rule_nterms_188, 
  rule_nterms_189, 
  rule_nterms_190, 
  rule_nterms_191, 
  rule_nterms_192, 
  rule_nterms_193, 
  rule_nterms_194, 
  rule_nterms_195, 
  rule_nterms_196, 
  rule_nterms_197, 
  rule_nterms_198, 
  rule_nterms_199, 
  rule_nterms_200, 
  rule_nterms_201, 
  rule_nterms_202, 
  rule_nterms_203, 
  rule_nterms_204, 
  rule_nterms_205, 
  rule_nterms_206, 
  rule_nterms_207, 
  rule_nterms_208, 
  rule_nterms_209, 
  rule_nterms_210, 
  rule_nterms_211, 
  rule_nterms_212, 
  rule_nterms_213, 
  rule_nterms_214, 
  rule_nterms_215, 
  rule_nterms_216, 
  rule_nterms_217, 
  rule_nterms_218, 
  rule_nterms_219, 
  rule_nterms_220, 
  rule_nterms_221, 
  rule_nterms_222, 
  rule_nterms_223, 
  rule_nterms_224, 
  rule_nterms_225, 
  rule_nterms_226, 
  rule_nterms_227, 
  rule_nterms_228, 
  rule_nterms_229, 
  rule_nterms_230, 
  rule_nterms_231, 
  rule_nterms_232, 
  rule_nterms_233, 
  rule_nterms_234, 
  rule_nterms_235, 
  rule_nterms_236, 
  rule_nterms_237, 
  rule_nterms_238, 
  rule_nterms_239, 
  rule_nterms_240, 
  rule_nterms_241, 
  rule_nterms_242, 
  rule_nterms_243, 
  rule_nterms_244, 
  rule_nterms_245, 
  rule_nterms_246, 
  rule_nterms_247, 
  rule_nterms_248, 
  rule_nterms_249, 
  rule_nterms_250, 
  rule_nterms_251, 
  rule_nterms_252, 
  rule_nterms_253, 
  rule_nterms_254, 
  rule_nterms_255, 
  rule_nterms_256, 
  rule_nterms_257, 
  rule_nterms_258, 
  rule_nterms_259, 
  rule_nterms_260, 
  rule_nterms_261, 
  rule_nterms_262, 
  rule_nterms_263, 
  rule_nterms_264, 
  rule_nterms_265, 
  rule_nterms_266, 
  rule_nterms_267, 
  rule_nterms_268, 
  rule_nterms_269, 
  rule_nterms_270, 
  rule_nterms_271, 
  rule_nterms_272, 
  rule_nterms_273, 
  rule_nterms_274, 
  rule_nterms_275, 
  rule_nterms_276, 
  rule_nterms_277, 
  rule_nterms_278, 
  rule_nterms_279, 
  rule_nterms_280, 
  rule_nterms_281, 
  rule_nterms_282, 
  rule_nterms_283, 
  rule_nterms_284, 
  rule_nterms_285, 
  rule_nterms_286, 
  rule_nterms_287, 
  rule_nterms_288, 
  rule_nterms_289, 
  rule_nterms_290, 
  rule_nterms_291, 
  rule_nterms_292, 
  rule_nterms_293, 
  rule_nterms_294, 
  rule_nterms_295, 
  rule_nterms_296, 
  rule_nterms_297, 
  rule_nterms_298, 
  rule_nterms_299, 
  rule_nterms_300, 
  rule_nterms_301, 
  rule_nterms_302, 
  rule_nterms_303, 
  rule_nterms_304, 
  rule_nterms_305, 
  rule_nterms_306, 
  rule_nterms_307, 
  rule_nterms_308, 
  rule_nterms_309, 
  rule_nterms_310, 
  rule_nterms_311, 
  rule_nterms_312, 
  rule_nterms_313, 
  
};

static const char * const rule_strings[] = {
  (const char *)0,
  "root: stmt = 1 : (0)",
  "root: reg = 2 : (0)",
  "rcon: reg = 3 : (0)",
  "cnsti4: INTCONST = 4 : (0)",
  "stmt: STID ###CLASS_PREG(reg) = 5 : (0)",
  "stmt: STID(reg) = 6 : (ST_COST)",
  "stmt: ISTORE(reg,LDA) = 7 : (ST_COST)",
  "stmt: ISTORE(reg,reg) = 8 : (ST_COST)",
  "stmt: ISTOREX(reg,reg,reg) = 9 : (FAIL_COST)",
  "stmt: STBITS ###CLASS_PREG(reg) = 10 : (1+1)",
  "stmt: STBITS(reg) = 11 : (1+ST_COST)",
  "stmt: ISTBITS(reg,LDA) = 12 : (1+ST_COST)",
  "stmt: ISTBITS(reg,reg) = 13 : (1+ST_COST)",
  "reg: LDID ###CLASS_PREG = 14 : (0)",
  "reg: LDID = 15 : (LD_COST)",
  "reg: ILOAD(LDA) = 16 : (LD_COST)",
  "reg: ILOAD(reg) = 17 : (LD_COST)",
  "reg: LDA = 18 : (LD_COST)",
  "reg: ILOADX(reg,reg) = 19 : (FAIL_COST)",
  "reg: LDBITS ###CLASS_PREG = 20 : (1+0)",
  "reg: LDBITS = 21 : (1+LD_COST)",
  "reg: ILDBITS(LDA) = 22 : (1+LD_COST)",
  "reg: ILDBITS(reg) = 23 : (1+LD_COST)",
  "reg: INTCONST #0 = 24 : (INTCONST_COST)",
  "reg: INTCONST = 25 : (INTCONST_COST)",
  "reg: CONST = 26 : (CONST_COST)",
  "reg: ADD(reg,rcon) = 27 : (ARI_COST)",
  "reg: ADD(rcon,reg) = 28 : (ARI_COST)",
  "reg: SUB(reg,rcon) = 29 : (ARI_COST)",
  "reg: SUB(rcon,reg) = 30 : (ARI_COST)",
  "reg: NEG(reg) = 31 : (ARI_COST)",
  "reg: ASHR(reg,rcon) = 32 : (ARI_COST)",
  "reg: LSHR(reg,rcon) = 33 : (ARI_COST)",
  "reg: SHL(reg,rcon) = 34 : (ARI_COST)",
  "reg: MPY(reg,rcon) = 35 : (MUL32_COST)",
  "reg: MPY(rcon,reg) = 36 : (MUL32_COST)",
  "reg: DIV(reg,cnsti4) = 37 : (DIV_CNST_COST)",
  "reg: REM(reg,cnsti4) = 38 : (DIV_CNST_COST)",
  "reg: DIV(reg,reg) = 39 : (DIV_COST)",
  "reg: REM(reg,reg) = 40 : (DIV_COST)",
  "reg: MOD(reg,rcon) = 41 : (FAIL_COST&&DIV_COST)",
  "reg: ABS(reg) = 42 : (ABS_COST)",
  "reg: MAX(reg,rcon) = 43 : (MMAX_COST)",
  "reg: MAX(rcon,reg) = 44 : (MMAX_COST)",
  "reg: MIN(reg,rcon) = 45 : (MMAX_COST)",
  "reg: MIN(rcon,reg) = 46 : (MMAX_COST)",
  "reg: MADD(reg,reg,reg) = 47 : (FAIL_COST&&MADD_COST)",
  "reg: MSUB(reg,reg,reg) = 48 : (FAIL_COST&&MADD_COST)",
  "reg: NMADD(reg,reg,reg) = 49 : (FAIL_COST&&MADD_COST)",
  "reg: NMSUB(reg,reg,reg) = 50 : (FAIL_COST&&MADD_COST)",
  "reg: BNOT(reg) = 51 : (ARI_COST)",
  "reg: BAND(reg,rcon) = 52 : (ARI_COST)",
  "reg: BAND(rcon,reg) = 53 : (ARI_COST)",
  "reg: BIOR(reg,rcon) = 54 : (ARI_COST)",
  "reg: BIOR(rcon,reg) = 55 : (ARI_COST)",
  "reg: BXOR(reg,rcon) = 56 : (ARI_COST)",
  "reg: BXOR(rcon,reg) = 57 : (ARI_COST)",
  "reg: BNOR(reg,rcon) = 58 : (FAIL_COST&&ARI_COST)",
  "reg: BNOR(rcon,reg) = 59 : (FAIL_COST&&ARI_COST)",
  "reg: LNOT(rcon) = 60 : (LOGIC_COST)",
  "reg: LAND(reg,rcon) = 61 : (LOGIC_COST)",
  "reg: LAND(rcon,reg) = 62 : (LOGIC_COST)",
  "reg: LIOR(reg,rcon) = 63 : (LOGIC_COST)",
  "reg: LIOR(rcon,reg) = 64 : (LOGIC_COST)",
  "reg: SELECT(cond,reg,rcon) = 65 : (SELECT_COST)",
  "reg: SELECT(cond,rcon,reg) = 66 : (SELECT_COST)",
  "reg: SELECT(reg,reg,reg) = 67 : (SELECT_COST)",
  "cond: EQ(reg,rcon) = 68 : (0)",
  "cond: EQ(rcon,rcon) = 69 : (0)",
  "cond: NE(reg,rcon) = 70 : (0)",
  "cond: NE(rcon,reg) = 71 : (0)",
  "cond: GT(reg,rcon) = 72 : (0)",
  "cond: GT(rcon,rcon) = 73 : (0)",
  "cond: GE(reg,rcon) = 74 : (0)",
  "cond: GE(rcon,reg) = 75 : (0)",
  "cond: LT(reg,rcon) = 76 : (0)",
  "cond: LT(rcon,reg) = 77 : (0)",
  "cond: LE(reg,rcon) = 78 : (0)",
  "cond: LE(rcon,reg) = 79 : (0)",
  "reg: EQ(reg,rcon) = 80 : (1)",
  "reg: EQ(rcon,rcon) = 81 : (1)",
  "reg: NE(reg,rcon) = 82 : (1)",
  "reg: NE(rcon,reg) = 83 : (1)",
  "reg: GT(reg,rcon) = 84 : (1)",
  "reg: GT(rcon,rcon) = 85 : (1)",
  "reg: GE(reg,rcon) = 86 : (1)",
  "reg: GE(rcon,reg) = 87 : (1)",
  "reg: LT(reg,rcon) = 88 : (1)",
  "reg: LT(rcon,reg) = 89 : (1)",
  "reg: LE(reg,rcon) = 90 : (1)",
  "reg: LE(rcon,reg) = 91 : (1)",
  "reg: SQRT(reg) = 92 : (FAIL_COST&&FLT_COST)",
  "reg: RSQRT(reg) = 93 : (FAIL_COST&&FLT_COST)",
  "reg: RECIP(reg) = 94 : (FAIL_COST&&FLT_COST)",
  "reg: RND(reg) = 95 : (FAIL_COST&&FLT_COST)",
  "reg: TRUNC(reg) = 96 : (FAIL_COST&&FLT_COST)",
  "reg: CEIL(reg) = 97 : (FAIL_COST&&FLT_COST)",
  "reg: FLOOR(reg) = 98 : (FAIL_COST&&FLT_COST)",
  "reg: CVTL.I4 ##16(reg) = 99 : (CVTL_COST)",
  "reg: CVTL.U4 ##16(reg) = 100 : (CVTL_COST)",
  "reg: CVTL.I4 ##8(reg) = 101 : (CVTL_COST)",
  "reg: CVTL.U4 ##8(reg) = 102 : (CVTL_COST)",
  "reg: CVTL(reg) = 103 : (CVTL_COST)",
  "reg: CVT(reg) = 104 : (FAIL_COST&&CVT_COST)",
  "reg: TAS(reg) = 105 : (0)",
  "reg: PAREN(reg) = 106 : (0)",
  "reg: EXTRACT_BITS(reg) = 107 : (2)",
  "stmt: GOTO = 108 : (1)",
  "stmt: REGION_EXIT = 109 : (1)",
  "stmt: AGOTO(reg) = 110 : (1)",
  "stmt: XGOTO(reg) = 111 : (1)",
  "stmt: TRUEBR(reg) = 112 : (1)",
  "stmt: FALSEBR(reg) = 113 : (1)",
  "stmt: TRUEBR(cond) = 114 : (1)",
  "stmt: FALSEBR(cond) = 115 : (1)",
  "stmt: TRUEBR(INTCONST) = 116 : (1)",
  "stmt: FALSEBR(INTCONST) = 117 : (1)",
  "param: PARM(reg) = 118 : (FAIL_COST)",
  "stmt: CALL.V = 119 : (1)",
  "stmt: CALL = 120 : (1)",
  "stmt: ICALL.V = 121 : (1)",
  "stmt: ICALL = 122 : (1)",
  "stmt: RETURN = 123 : (1)",
  "stmt: PICCALL.V = 124 : (FAIL_COST)",
  "stmt: PICCALL = 125 : (FAIL_COST)",
  "reg: LOWPART(reg) = 126 : (FAIL_COST)",
  "reg: HIGHPART(reg) = 127 : (FAIL_COST)",
  "reg: MINPART(reg) = 128 : (FAIL_COST)",
  "reg: MAXPART(reg) = 129 : (FAIL_COST)",
  "reg: COMPOSE_BITS(reg,reg) = 130 : (FAIL_COST)",
  "reg: HIGHMPY(reg,reg) = 131 : (FAIL_COST)",
  "reg: XMPY(reg,reg) = 132 : (FAIL_COST)",
  "reg: DIVREM(reg,reg) = 133 : (FAIL_COST)",
  "reg: MINMAX(reg,reg) = 134 : (FAIL_COST)",
  "reg: LDMA = 135 : (FAIL_COST)",
  "reg: LDA_LABEL = 136 : (FAIL_COST)",
  "stmt: IDNAME = 137 : (FAIL_COST)",
  "stmt: MSTORE(reg,reg,reg) = 138 : (FAIL_COST)",
  "reg: MLOAD(reg,reg) = 139 : (FAIL_COST)",
  "inv: FIRSTPART(inv) = 140 : (FAIL_COST)",
  "inv: SECONDPART(inv) = 141 : (FAIL_COST)",
  "inv: CAND(inv,inv) = 142 : (FAIL_COST)",
  "inv: CIOR(inv,inv) = 143 : (FAIL_COST)",
  "inv: CSELECT(inv,inv) = 144 : (FAIL_COST)",
  "inv: ILDA(inv) = 145 : (FAIL_COST)",
  "inv: PAIR(inv,inv) = 146 : (FAIL_COST)",
  "inv: RROTATE(inv,inv) = 147 : (FAIL_COST)",
  "inv: COMMA(inv,inv) = 148 : (FAIL_COST)",
  "inv: RCOMMA(inv,inv) = 149 : (FAIL_COST)",
  "inv: ARRAY(inv) = 150 : (FAIL_COST)",
  "inv: INTRINSIC_OP(inv) = 151 : (FAIL_COST)",
  "inv: IO_ITEM(inv) = 152 : (FAIL_COST)",
  "inv: TRIPLET(inv) = 153 : (FAIL_COST)",
  "inv: ARRAYEXP(inv) = 154 : (FAIL_COST)",
  "inv: ARRSECTION(inv) = 155 : (FAIL_COST)",
  "inv: WHERE(inv) = 156 : (FAIL_COST)",
  "stmt: FORWARD_BARRIER = 157 : (1)",
  "stmt: BACKWARD_BARRIER = 158 : (1)",
  "stmt: INTRINSIC_CALL.V = 159 : (FAIL_COST)",
  "reg: INTRINSIC_CALL = 160 : (FAIL_COST)",
  "rcon: cnsti4 = 161 : (0)",
  "cnsti4_ge8: INTCONST.I4 = 162 : (CHECK_COST(WN_const_val(STATE_WN) >= 8, 0))",
  "cnsti4_ge16: INTCONST.I4 = 163 : (CHECK_COST(WN_const_val(STATE_WN) >= 16, 0))",
  "cnsti4_ge24: INTCONST.I4 = 164 : (CHECK_COST(WN_const_val(STATE_WN) >= 24, 0))",
  "cnsti4_sx8: INTCONST.I4 = 165 : (CHECK_COST(WN_const_val(STATE_WN) < (1<<7) && WN_const_val(STATE_WN) >= -(1<<7) , 0))",
  "cnsti4_sx16: INTCONST.I4 = 166 : (CHECK_COST(WN_const_val(STATE_WN) < (1<<15) && WN_const_val(STATE_WN) >= -(1<<15) , 0))",
  "cnsti4_sx16: cnsti4_sx8 = 167 : (0)",
  "cnsti4_zx8: INTCONST.I4 = 168 : (CHECK_COST(WN_const_val(STATE_WN) < (1<<8) && WN_const_val(STATE_WN) >= 0 , 0))",
  "cnsti4_zx16: INTCONST.I4 = 169 : (CHECK_COST(WN_const_val(STATE_WN) < (1<<16) && WN_const_val(STATE_WN) >= 0 , 0))",
  "cnsti4_zx16: cnsti4_zx8 = 170 : (0)",
  "cnsti4_1: INTCONST.I4 #1 = 171 : (0)",
  "cnsti4_2: INTCONST.I4 #2 = 172 : (0)",
  "cnsti4_3: INTCONST.I4 #3 = 173 : (0)",
  "cnsti4_4: INTCONST.I4 #4 = 174 : (0)",
  "cnsti4_8: INTCONST.I4 #8 = 175 : (0)",
  "cnsti4_16: INTCONST.I4 #16 = 176 : (0)",
  "cnsti4_0xff: INTCONST.I4 #0xff = 177 : (0)",
  "cnsti4_0xffff: INTCONST.I4 #0xffff = 178 : (0)",
  "cnstpw2u: INTCONST = 179 : (CHECK_COST(IS_CNSTPW2U(STATE_WN), 0))",
  "cnstpw2s: INTCONST = 180 : (CHECK_COST(IS_CNSTPW2S(STATE_WN), 0))",
  "reg: MPY(reg,cnstpw2u) = 181 : (1)",
  "reg: MPY(cnstpw2u,reg) = 182 : (1)",
  "reg: MPY(reg,cnstpw2s) = 183 : (2)",
  "reg: MPY(cnstpw2s,reg) = 184 : (2)",
  "m_reg_shl_1: SHL(reg,cnsti4_1) = 185 : (0)",
  "m_reg_shl_1: MPY(reg,cnsti4_2) = 186 : (0)",
  "m_reg_shl_2: SHL(reg,cnsti4_2) = 187 : (0)",
  "m_reg_shl_2: MPY(reg,cnsti4_4) = 188 : (0)",
  "m_reg_shl_3: SHL(reg,cnsti4_3) = 189 : (0)",
  "m_reg_shl_3: MPY(reg,cnsti4_8) = 190 : (0)",
  "m_reg_shl_4: SHL(reg,cnsti4_4) = 191 : (0)",
  "m_reg_shl_3: MPY(reg,cnsti4_16) = 192 : (0)",
  "reg: ADD(m_reg_shl_1,rcon) = 193 : (ARI_COST)",
  "reg: ADD(m_reg_shl_2,rcon) = 194 : (ARI_COST)",
  "reg: ADD(m_reg_shl_3,rcon) = 195 : (ARI_COST)",
  "reg: ADD(m_reg_shl_4,rcon) = 196 : (ARI_COST)",
  "reg: SHL(MPY(reg,m_reg_zx16h),cnsti4_16) = 197 : (MUL_COST)",
  "reg: SHL(MPY(m_reg_zx16h,reg),cnsti4_16) = 198 : (MUL_COST)",
  "reg: MPY(m_reg_sx16h,m_reg_sx16h) = 199 : (MUL_COST)",
  "reg: MPY(m_reg_zx16h,m_reg_zx16h) = 200 : (MUL_COST)",
  "reg: MPY(m_reg_sx16,m_reg_sx16h) = 201 : (MUL_COST)",
  "reg: MPY(m_reg_sx16h,m_reg_sx16) = 202 : (MUL_COST)",
  "reg: MPY(m_reg_zx16,m_reg_zx16h) = 203 : (MUL_COST)",
  "reg: MPY(m_reg_zx16h,m_reg_zx16) = 204 : (MUL_COST)",
  "reg: MPY(reg,m_reg_sx16h) = 205 : (MUL_COST)",
  "reg: MPY(m_reg_sx16h,reg) = 206 : (MUL_COST)",
  "reg: MPY(reg,m_reg_zx16h) = 207 : (MUL_COST)",
  "reg: MPY(m_reg_zx16h,reg) = 208 : (MUL_COST)",
  "reg: MPY(m_reg_sx16,m_reg_sx16) = 209 : (MUL_COST)",
  "reg: MPY(m_reg_sx16,cnsti4_sx16) = 210 : (MUL_COST)",
  "reg: MPY(cnsti4_sx16,m_reg_sx16) = 211 : (MUL_COST)",
  "reg: MPY(m_reg_zx16,m_reg_zx16) = 212 : (MUL_COST)",
  "reg: MPY(m_reg_zx16,cnsti4_zx16) = 213 : (MUL_COST)",
  "reg: MPY(cnsti4_zx16,m_reg_zx16) = 214 : (MUL_COST)",
  "reg: MPY(reg,m_reg_sx16) = 215 : (MUL_COST)",
  "reg: MPY(reg,cnsti4_sx16) = 216 : (MUL_COST)",
  "reg: MPY(m_reg_sx16,reg) = 217 : (MUL_COST)",
  "reg: MPY(cnsti4_sx16,reg) = 218 : (MUL_COST)",
  "reg: MPY(reg,m_reg_zx16) = 219 : (MUL_COST)",
  "reg: MPY(reg,cnsti4_zx16) = 220 : (MUL_COST)",
  "reg: MPY(m_reg_zx16,reg) = 221 : (MUL_COST)",
  "reg: MPY(cnsti4_zx16,reg) = 222 : (MUL_COST)",
  "m_reg_sx16h: ASHR.I4(reg,cnsti4_16) = 223 : (0)",
  "m_reg_sx16h: CVTL.I4 ##16(m_reg_sx16h) = 224 : (0)",
  "m_reg_sx16h: ASHR.I4(SHL(m_reg_sx16h,cnsti4_16),cnsti4_16) = 225 : (0)",
  "m_reg_zx16h: LSHR.U4(reg,cnsti4_16) = 226 : (0)",
  "m_reg_zx16h: CVTL.U4 ##16(m_reg_zx16h) = 227 : (0)",
  "m_reg_zx16h: LSHR.U4(SHL(m_reg_zx16h,cnsti4_16),cnsti4_16) = 228 : (0)",
  "m_reg_zx16h: BAND.I4(m_reg_zx16h,cnsti4_0xffff) = 229 : (0)",
  "m_reg_zx16h: BAND.U4(m_reg_zx16h,cnsti4_0xffff) = 230 : (0)",
  "m_reg_sx8: reg_sx8 = 231 : (0)",
  "reg_or_m_reg_sx8: reg = 232 : (0)",
  "reg_or_m_reg_sx8: m_reg_sx8 = 233 : (0)",
  "m_reg_sx8: CVTL.I4 ##8(reg_or_m_reg_sx16) = 234 : (0)",
  "m_reg_zx8: reg_zx8 = 235 : (0)",
  "reg_or_m_reg_zx8: reg = 236 : (0)",
  "reg_or_m_reg_zx8: m_reg_zx8 = 237 : (0)",
  "m_reg_zx8: CVTL.U4 ##8(reg_or_m_reg_zx8) = 238 : (0)",
  "m_reg_zx8: BAND.(reg_or_m_reg_zx8,cnsti4_0xff) = 239 : (0)",
  "m_reg_sx16: reg_sx16 = 240 : (0)",
  "m_reg_sx16: m_reg_sx8 = 241 : (0)",
  "reg_or_m_reg_sx16: reg = 242 : (0)",
  "reg_or_m_reg_sx16: m_reg_sx16 = 243 : (0)",
  "m_reg_sx16: m_reg_sx16h = 244 : (0)",
  "m_reg_sx16: CVTL.I4 ##16(reg_or_m_reg_sx16) = 245 : (0)",
  "m_reg_sx16: ASHR.I4(SHL(reg_or_m_reg_sx16,cnsti4_16),cnsti4_16) = 246 : (0)",
  "m_reg_zx16: reg_zx16 = 247 : (0)",
  "m_reg_zx16: m_reg_zx8 = 248 : (0)",
  "reg_or_m_reg_zx16: reg = 249 : (0)",
  "reg_or_m_reg_zx16: m_reg_zx16 = 250 : (0)",
  "m_reg_zx16: m_reg_zx16h = 251 : (0)",
  "m_reg_zx16: CVTL.U4 ##16(reg_or_m_reg_zx16) = 252 : (0)",
  "m_reg_zx16: LSHR.U4(SHL(reg_or_m_reg_zx16,cnsti4_16),cnsti4_16) = 253 : (0)",
  "m_reg_zx16: BAND.(reg_or_m_reg_zx16,cnsti4_0xffff) = 254 : (0)",
  "reg: reg_sx8 = 255 : (0)",
  "reg_sx8: CVTL.I4 ##8(reg) = 256 : (CVTL_COST)",
  "reg_sx8: ASHR.I4(reg,cnsti4_ge24) = 257 : (ARI_COST)",
  "reg_sx8: ASHR.I4(reg_sx16,cnsti4_ge8) = 258 : (ARI_COST)",
  "reg_sx8: CVTL.I4 ##8(reg_sx8) = 259 : (0)",
  "reg_sx8: CVTL.I4 ##16(reg_sx8) = 260 : (0)",
  "reg_sx8: cnsti4_sx8 = 261 : (INTCONST_COST)",
  "reg: reg_zx8 = 262 : (0)",
  "reg_zx8: CVTL.U4 ##8(reg) = 263 : (CVTL_COST)",
  "reg_zx8: BAND(reg,cnsti4_0xff) = 264 : (CVTL_COST)",
  "reg_zx8: LSHR.U4(reg,cnsti4_ge24) = 265 : (ARI_COST)",
  "reg_zx8: LSHR.U4(reg_zx16,cnsti4_ge8) = 266 : (ARI_COST)",
  "reg_zx8: ASHR.I4(reg_zx16,cnsti4_ge8) = 267 : (ARI_COST)",
  "reg_zx8: CVTL.U4 ##8(reg_zx8) = 268 : (0)",
  "reg_zx8: CVTL.U4 ##16(reg_zx8) = 269 : (0)",
  "reg_zx8: CVTL.I4 ##16(reg_zx8) = 270 : (0)",
  "reg_zx8: BAND(reg_zx8,cnsti4_0xff) = 271 : (0)",
  "reg_zx8: cnsti4_zx8 = 272 : (INTCONST_COST)",
  "reg: reg_sx16 = 273 : (0)",
  "reg_sx16: reg_sx8 = 274 : (0)",
  "reg_sx16: reg_zx8 = 275 : (0)",
  "reg_sx16: CVTL.I4 ##16(reg) = 276 : (CVTL_COST)",
  "reg_sx16: ASHR.I4(SHL(reg,cnsti4_16),cnsti4_16) = 277 : (CVTL_COST)",
  "reg_sx16: ASHR.I4(reg,cnsti4_ge16) = 278 : (ARI_COST)",
  "reg_sx16: CVTL.I4 ##16(reg_sx16) = 279 : (0)",
  "reg_sx16: ASHR.I4(SHL(reg_sx16,cnsti4_16),cnsti4_16) = 280 : (0)",
  "reg_sx16: cnsti4_sx16 = 281 : (INTCONST_COST)",
  "reg: reg_zx16 = 282 : (0)",
  "reg_zx16: reg_zx8 = 283 : (0)",
  "reg_zx16: CVTL.U4 ##16(reg) = 284 : (CVTL_COST)",
  "reg_zx16: LSHR.U4(SHL(reg,cnsti4_16),cnsti4_16) = 285 : (CVTL_COST)",
  "reg_zx16: BAND(reg,cnsti4_0xffff) = 286 : (CVTL_COST)",
  "reg_zx16: LSHR.U4(reg,cnsti4_ge16) = 287 : (ARI_COST)",
  "reg_zx16: CVTL.U4 ##16(reg_zx16) = 288 : (0)",
  "reg_zx16: LSHR.U4(SHL(reg_zx16,cnsti4_16),cnsti4_16) = 289 : (0)",
  "reg_zx16: BAND(reg_zx16,cnsti4_0xffff) = 290 : (0)",
  "reg_zx16: cnsti4_zx16 = 291 : (INTCONST_COST)",
  "reg_zx16: LDID.U4.U2 ###CLASS_PREG = 292 : (LD_COST)",
  "reg_zx16: LDID.U4.U2 = 293 : (LD_COST)",
  "reg_zx16: ILOAD.U4.U2(LDA) = 294 : (LD_COST)",
  "reg_zx16: ILOAD.U4.U2(reg) = 295 : (LD_COST)",
  "reg_sx16: LDID.I4.I1 ###CLASS_PREG = 296 : (LD_COST)",
  "reg_sx16: LDID.I4.I1 = 297 : (LD_COST)",
  "reg_sx16: ILOAD.I4.I1(LDA) = 298 : (LD_COST)",
  "reg_sx16: ILOAD.I4.I1(reg) = 299 : (LD_COST)",
  "reg_zx8: LDID.U4.U1 ###CLASS_PREG = 300 : (LD_COST)",
  "reg_zx8: LDID.U4.U1 = 301 : (LD_COST)",
  "reg_zx8: ILOAD.U4.U1(LDA) = 302 : (LD_COST)",
  "reg_zx8: ILOAD.U4.U1(reg) = 303 : (LD_COST)",
  "reg_sx8: LDID.I4.I1 ###CLASS_PREG = 304 : (LD_COST)",
  "reg_sx8: LDID.I4.I1 = 305 : (LD_COST)",
  "reg_sx8: ILOAD.I4.I1(LDA) = 306 : (LD_COST)",
  "reg_sx8: ILOAD.I4.I1(reg) = 307 : (LD_COST)",
  "reg: reuse_reg = 308 : (0)",
  "reg_sx16: reuse_reg = 309 : (0)",
  "reg_zx16: reuse_reg = 310 : (0)",
  "reg_sx8: reuse_reg = 311 : (0)",
  "reg_zx8: reuse_reg = 312 : (0)",
  "reuse_reg: LDID ###CLASS_PREG = 313 : (REUSE_COST)",
  
};

static const char rule_kid_1_0 [] = { (char)-1,  };

static const char * const rule_kids_1 [] = { rule_kid_1_0,  };

static const char rule_kid_2_0 [] = { (char)-1,  };

static const char * const rule_kids_2 [] = { rule_kid_2_0,  };

static const char rule_kid_3_0 [] = { (char)-1,  };

static const char * const rule_kids_3 [] = { rule_kid_3_0,  };


static const char * const rule_kids_4 [] = {  };

static const char rule_kid_5_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_5 [] = { rule_kid_5_0,  };

static const char rule_kid_6_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_6 [] = { rule_kid_6_0,  };

static const char rule_kid_7_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_7 [] = { rule_kid_7_0,  };

static const char rule_kid_8_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_8_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_8 [] = { rule_kid_8_0, rule_kid_8_1,  };

static const char rule_kid_9_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_9_1 [] = { (char)1, (char)-1,  };
static const char rule_kid_9_2 [] = { (char)2, (char)-1,  };

static const char * const rule_kids_9 [] = { rule_kid_9_0, rule_kid_9_1, rule_kid_9_2,  };

static const char rule_kid_10_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_10 [] = { rule_kid_10_0,  };

static const char rule_kid_11_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_11 [] = { rule_kid_11_0,  };

static const char rule_kid_12_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_12 [] = { rule_kid_12_0,  };

static const char rule_kid_13_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_13_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_13 [] = { rule_kid_13_0, rule_kid_13_1,  };


static const char * const rule_kids_14 [] = {  };


static const char * const rule_kids_15 [] = {  };


static const char * const rule_kids_16 [] = {  };

static const char rule_kid_17_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_17 [] = { rule_kid_17_0,  };


static const char * const rule_kids_18 [] = {  };

static const char rule_kid_19_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_19_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_19 [] = { rule_kid_19_0, rule_kid_19_1,  };


static const char * const rule_kids_20 [] = {  };


static const char * const rule_kids_21 [] = {  };


static const char * const rule_kids_22 [] = {  };

static const char rule_kid_23_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_23 [] = { rule_kid_23_0,  };


static const char * const rule_kids_24 [] = {  };


static const char * const rule_kids_25 [] = {  };


static const char * const rule_kids_26 [] = {  };

static const char rule_kid_27_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_27_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_27 [] = { rule_kid_27_0, rule_kid_27_1,  };

static const char rule_kid_28_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_28_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_28 [] = { rule_kid_28_0, rule_kid_28_1,  };

static const char rule_kid_29_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_29_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_29 [] = { rule_kid_29_0, rule_kid_29_1,  };

static const char rule_kid_30_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_30_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_30 [] = { rule_kid_30_0, rule_kid_30_1,  };

static const char rule_kid_31_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_31 [] = { rule_kid_31_0,  };

static const char rule_kid_32_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_32_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_32 [] = { rule_kid_32_0, rule_kid_32_1,  };

static const char rule_kid_33_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_33_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_33 [] = { rule_kid_33_0, rule_kid_33_1,  };

static const char rule_kid_34_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_34_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_34 [] = { rule_kid_34_0, rule_kid_34_1,  };

static const char rule_kid_35_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_35_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_35 [] = { rule_kid_35_0, rule_kid_35_1,  };

static const char rule_kid_36_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_36_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_36 [] = { rule_kid_36_0, rule_kid_36_1,  };

static const char rule_kid_37_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_37_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_37 [] = { rule_kid_37_0, rule_kid_37_1,  };

static const char rule_kid_38_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_38_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_38 [] = { rule_kid_38_0, rule_kid_38_1,  };

static const char rule_kid_39_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_39_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_39 [] = { rule_kid_39_0, rule_kid_39_1,  };

static const char rule_kid_40_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_40_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_40 [] = { rule_kid_40_0, rule_kid_40_1,  };

static const char rule_kid_41_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_41_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_41 [] = { rule_kid_41_0, rule_kid_41_1,  };

static const char rule_kid_42_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_42 [] = { rule_kid_42_0,  };

static const char rule_kid_43_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_43_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_43 [] = { rule_kid_43_0, rule_kid_43_1,  };

static const char rule_kid_44_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_44_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_44 [] = { rule_kid_44_0, rule_kid_44_1,  };

static const char rule_kid_45_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_45_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_45 [] = { rule_kid_45_0, rule_kid_45_1,  };

static const char rule_kid_46_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_46_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_46 [] = { rule_kid_46_0, rule_kid_46_1,  };

static const char rule_kid_47_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_47_1 [] = { (char)1, (char)-1,  };
static const char rule_kid_47_2 [] = { (char)2, (char)-1,  };

static const char * const rule_kids_47 [] = { rule_kid_47_0, rule_kid_47_1, rule_kid_47_2,  };

static const char rule_kid_48_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_48_1 [] = { (char)1, (char)-1,  };
static const char rule_kid_48_2 [] = { (char)2, (char)-1,  };

static const char * const rule_kids_48 [] = { rule_kid_48_0, rule_kid_48_1, rule_kid_48_2,  };

static const char rule_kid_49_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_49_1 [] = { (char)1, (char)-1,  };
static const char rule_kid_49_2 [] = { (char)2, (char)-1,  };

static const char * const rule_kids_49 [] = { rule_kid_49_0, rule_kid_49_1, rule_kid_49_2,  };

static const char rule_kid_50_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_50_1 [] = { (char)1, (char)-1,  };
static const char rule_kid_50_2 [] = { (char)2, (char)-1,  };

static const char * const rule_kids_50 [] = { rule_kid_50_0, rule_kid_50_1, rule_kid_50_2,  };

static const char rule_kid_51_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_51 [] = { rule_kid_51_0,  };

static const char rule_kid_52_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_52_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_52 [] = { rule_kid_52_0, rule_kid_52_1,  };

static const char rule_kid_53_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_53_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_53 [] = { rule_kid_53_0, rule_kid_53_1,  };

static const char rule_kid_54_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_54_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_54 [] = { rule_kid_54_0, rule_kid_54_1,  };

static const char rule_kid_55_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_55_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_55 [] = { rule_kid_55_0, rule_kid_55_1,  };

static const char rule_kid_56_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_56_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_56 [] = { rule_kid_56_0, rule_kid_56_1,  };

static const char rule_kid_57_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_57_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_57 [] = { rule_kid_57_0, rule_kid_57_1,  };

static const char rule_kid_58_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_58_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_58 [] = { rule_kid_58_0, rule_kid_58_1,  };

static const char rule_kid_59_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_59_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_59 [] = { rule_kid_59_0, rule_kid_59_1,  };

static const char rule_kid_60_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_60 [] = { rule_kid_60_0,  };

static const char rule_kid_61_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_61_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_61 [] = { rule_kid_61_0, rule_kid_61_1,  };

static const char rule_kid_62_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_62_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_62 [] = { rule_kid_62_0, rule_kid_62_1,  };

static const char rule_kid_63_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_63_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_63 [] = { rule_kid_63_0, rule_kid_63_1,  };

static const char rule_kid_64_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_64_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_64 [] = { rule_kid_64_0, rule_kid_64_1,  };

static const char rule_kid_65_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_65_1 [] = { (char)1, (char)-1,  };
static const char rule_kid_65_2 [] = { (char)2, (char)-1,  };

static const char * const rule_kids_65 [] = { rule_kid_65_0, rule_kid_65_1, rule_kid_65_2,  };

static const char rule_kid_66_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_66_1 [] = { (char)1, (char)-1,  };
static const char rule_kid_66_2 [] = { (char)2, (char)-1,  };

static const char * const rule_kids_66 [] = { rule_kid_66_0, rule_kid_66_1, rule_kid_66_2,  };

static const char rule_kid_67_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_67_1 [] = { (char)1, (char)-1,  };
static const char rule_kid_67_2 [] = { (char)2, (char)-1,  };

static const char * const rule_kids_67 [] = { rule_kid_67_0, rule_kid_67_1, rule_kid_67_2,  };

static const char rule_kid_68_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_68_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_68 [] = { rule_kid_68_0, rule_kid_68_1,  };

static const char rule_kid_69_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_69_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_69 [] = { rule_kid_69_0, rule_kid_69_1,  };

static const char rule_kid_70_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_70_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_70 [] = { rule_kid_70_0, rule_kid_70_1,  };

static const char rule_kid_71_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_71_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_71 [] = { rule_kid_71_0, rule_kid_71_1,  };

static const char rule_kid_72_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_72_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_72 [] = { rule_kid_72_0, rule_kid_72_1,  };

static const char rule_kid_73_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_73_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_73 [] = { rule_kid_73_0, rule_kid_73_1,  };

static const char rule_kid_74_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_74_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_74 [] = { rule_kid_74_0, rule_kid_74_1,  };

static const char rule_kid_75_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_75_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_75 [] = { rule_kid_75_0, rule_kid_75_1,  };

static const char rule_kid_76_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_76_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_76 [] = { rule_kid_76_0, rule_kid_76_1,  };

static const char rule_kid_77_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_77_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_77 [] = { rule_kid_77_0, rule_kid_77_1,  };

static const char rule_kid_78_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_78_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_78 [] = { rule_kid_78_0, rule_kid_78_1,  };

static const char rule_kid_79_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_79_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_79 [] = { rule_kid_79_0, rule_kid_79_1,  };

static const char rule_kid_80_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_80_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_80 [] = { rule_kid_80_0, rule_kid_80_1,  };

static const char rule_kid_81_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_81_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_81 [] = { rule_kid_81_0, rule_kid_81_1,  };

static const char rule_kid_82_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_82_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_82 [] = { rule_kid_82_0, rule_kid_82_1,  };

static const char rule_kid_83_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_83_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_83 [] = { rule_kid_83_0, rule_kid_83_1,  };

static const char rule_kid_84_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_84_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_84 [] = { rule_kid_84_0, rule_kid_84_1,  };

static const char rule_kid_85_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_85_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_85 [] = { rule_kid_85_0, rule_kid_85_1,  };

static const char rule_kid_86_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_86_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_86 [] = { rule_kid_86_0, rule_kid_86_1,  };

static const char rule_kid_87_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_87_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_87 [] = { rule_kid_87_0, rule_kid_87_1,  };

static const char rule_kid_88_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_88_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_88 [] = { rule_kid_88_0, rule_kid_88_1,  };

static const char rule_kid_89_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_89_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_89 [] = { rule_kid_89_0, rule_kid_89_1,  };

static const char rule_kid_90_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_90_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_90 [] = { rule_kid_90_0, rule_kid_90_1,  };

static const char rule_kid_91_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_91_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_91 [] = { rule_kid_91_0, rule_kid_91_1,  };

static const char rule_kid_92_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_92 [] = { rule_kid_92_0,  };

static const char rule_kid_93_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_93 [] = { rule_kid_93_0,  };

static const char rule_kid_94_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_94 [] = { rule_kid_94_0,  };

static const char rule_kid_95_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_95 [] = { rule_kid_95_0,  };

static const char rule_kid_96_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_96 [] = { rule_kid_96_0,  };

static const char rule_kid_97_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_97 [] = { rule_kid_97_0,  };

static const char rule_kid_98_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_98 [] = { rule_kid_98_0,  };

static const char rule_kid_99_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_99 [] = { rule_kid_99_0,  };

static const char rule_kid_100_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_100 [] = { rule_kid_100_0,  };

static const char rule_kid_101_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_101 [] = { rule_kid_101_0,  };

static const char rule_kid_102_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_102 [] = { rule_kid_102_0,  };

static const char rule_kid_103_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_103 [] = { rule_kid_103_0,  };

static const char rule_kid_104_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_104 [] = { rule_kid_104_0,  };

static const char rule_kid_105_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_105 [] = { rule_kid_105_0,  };

static const char rule_kid_106_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_106 [] = { rule_kid_106_0,  };

static const char rule_kid_107_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_107 [] = { rule_kid_107_0,  };


static const char * const rule_kids_108 [] = {  };


static const char * const rule_kids_109 [] = {  };

static const char rule_kid_110_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_110 [] = { rule_kid_110_0,  };

static const char rule_kid_111_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_111 [] = { rule_kid_111_0,  };

static const char rule_kid_112_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_112 [] = { rule_kid_112_0,  };

static const char rule_kid_113_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_113 [] = { rule_kid_113_0,  };

static const char rule_kid_114_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_114 [] = { rule_kid_114_0,  };

static const char rule_kid_115_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_115 [] = { rule_kid_115_0,  };


static const char * const rule_kids_116 [] = {  };


static const char * const rule_kids_117 [] = {  };

static const char rule_kid_118_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_118 [] = { rule_kid_118_0,  };


static const char * const rule_kids_119 [] = {  };


static const char * const rule_kids_120 [] = {  };


static const char * const rule_kids_121 [] = {  };


static const char * const rule_kids_122 [] = {  };


static const char * const rule_kids_123 [] = {  };


static const char * const rule_kids_124 [] = {  };


static const char * const rule_kids_125 [] = {  };

static const char rule_kid_126_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_126 [] = { rule_kid_126_0,  };

static const char rule_kid_127_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_127 [] = { rule_kid_127_0,  };

static const char rule_kid_128_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_128 [] = { rule_kid_128_0,  };

static const char rule_kid_129_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_129 [] = { rule_kid_129_0,  };

static const char rule_kid_130_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_130_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_130 [] = { rule_kid_130_0, rule_kid_130_1,  };

static const char rule_kid_131_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_131_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_131 [] = { rule_kid_131_0, rule_kid_131_1,  };

static const char rule_kid_132_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_132_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_132 [] = { rule_kid_132_0, rule_kid_132_1,  };

static const char rule_kid_133_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_133_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_133 [] = { rule_kid_133_0, rule_kid_133_1,  };

static const char rule_kid_134_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_134_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_134 [] = { rule_kid_134_0, rule_kid_134_1,  };


static const char * const rule_kids_135 [] = {  };


static const char * const rule_kids_136 [] = {  };


static const char * const rule_kids_137 [] = {  };

static const char rule_kid_138_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_138_1 [] = { (char)1, (char)-1,  };
static const char rule_kid_138_2 [] = { (char)2, (char)-1,  };

static const char * const rule_kids_138 [] = { rule_kid_138_0, rule_kid_138_1, rule_kid_138_2,  };

static const char rule_kid_139_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_139_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_139 [] = { rule_kid_139_0, rule_kid_139_1,  };

static const char rule_kid_140_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_140 [] = { rule_kid_140_0,  };

static const char rule_kid_141_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_141 [] = { rule_kid_141_0,  };

static const char rule_kid_142_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_142_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_142 [] = { rule_kid_142_0, rule_kid_142_1,  };

static const char rule_kid_143_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_143_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_143 [] = { rule_kid_143_0, rule_kid_143_1,  };

static const char rule_kid_144_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_144_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_144 [] = { rule_kid_144_0, rule_kid_144_1,  };

static const char rule_kid_145_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_145 [] = { rule_kid_145_0,  };

static const char rule_kid_146_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_146_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_146 [] = { rule_kid_146_0, rule_kid_146_1,  };

static const char rule_kid_147_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_147_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_147 [] = { rule_kid_147_0, rule_kid_147_1,  };

static const char rule_kid_148_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_148_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_148 [] = { rule_kid_148_0, rule_kid_148_1,  };

static const char rule_kid_149_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_149_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_149 [] = { rule_kid_149_0, rule_kid_149_1,  };

static const char rule_kid_150_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_150 [] = { rule_kid_150_0,  };

static const char rule_kid_151_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_151 [] = { rule_kid_151_0,  };

static const char rule_kid_152_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_152 [] = { rule_kid_152_0,  };

static const char rule_kid_153_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_153 [] = { rule_kid_153_0,  };

static const char rule_kid_154_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_154 [] = { rule_kid_154_0,  };

static const char rule_kid_155_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_155 [] = { rule_kid_155_0,  };

static const char rule_kid_156_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_156 [] = { rule_kid_156_0,  };


static const char * const rule_kids_157 [] = {  };


static const char * const rule_kids_158 [] = {  };


static const char * const rule_kids_159 [] = {  };


static const char * const rule_kids_160 [] = {  };

static const char rule_kid_161_0 [] = { (char)-1,  };

static const char * const rule_kids_161 [] = { rule_kid_161_0,  };


static const char * const rule_kids_162 [] = {  };


static const char * const rule_kids_163 [] = {  };


static const char * const rule_kids_164 [] = {  };


static const char * const rule_kids_165 [] = {  };


static const char * const rule_kids_166 [] = {  };

static const char rule_kid_167_0 [] = { (char)-1,  };

static const char * const rule_kids_167 [] = { rule_kid_167_0,  };


static const char * const rule_kids_168 [] = {  };


static const char * const rule_kids_169 [] = {  };

static const char rule_kid_170_0 [] = { (char)-1,  };

static const char * const rule_kids_170 [] = { rule_kid_170_0,  };


static const char * const rule_kids_171 [] = {  };


static const char * const rule_kids_172 [] = {  };


static const char * const rule_kids_173 [] = {  };


static const char * const rule_kids_174 [] = {  };


static const char * const rule_kids_175 [] = {  };


static const char * const rule_kids_176 [] = {  };


static const char * const rule_kids_177 [] = {  };


static const char * const rule_kids_178 [] = {  };


static const char * const rule_kids_179 [] = {  };


static const char * const rule_kids_180 [] = {  };

static const char rule_kid_181_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_181_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_181 [] = { rule_kid_181_0, rule_kid_181_1,  };

static const char rule_kid_182_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_182_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_182 [] = { rule_kid_182_0, rule_kid_182_1,  };

static const char rule_kid_183_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_183_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_183 [] = { rule_kid_183_0, rule_kid_183_1,  };

static const char rule_kid_184_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_184_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_184 [] = { rule_kid_184_0, rule_kid_184_1,  };

static const char rule_kid_185_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_185_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_185 [] = { rule_kid_185_0, rule_kid_185_1,  };

static const char rule_kid_186_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_186_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_186 [] = { rule_kid_186_0, rule_kid_186_1,  };

static const char rule_kid_187_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_187_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_187 [] = { rule_kid_187_0, rule_kid_187_1,  };

static const char rule_kid_188_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_188_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_188 [] = { rule_kid_188_0, rule_kid_188_1,  };

static const char rule_kid_189_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_189_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_189 [] = { rule_kid_189_0, rule_kid_189_1,  };

static const char rule_kid_190_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_190_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_190 [] = { rule_kid_190_0, rule_kid_190_1,  };

static const char rule_kid_191_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_191_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_191 [] = { rule_kid_191_0, rule_kid_191_1,  };

static const char rule_kid_192_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_192_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_192 [] = { rule_kid_192_0, rule_kid_192_1,  };

static const char rule_kid_193_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_193_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_193 [] = { rule_kid_193_0, rule_kid_193_1,  };

static const char rule_kid_194_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_194_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_194 [] = { rule_kid_194_0, rule_kid_194_1,  };

static const char rule_kid_195_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_195_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_195 [] = { rule_kid_195_0, rule_kid_195_1,  };

static const char rule_kid_196_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_196_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_196 [] = { rule_kid_196_0, rule_kid_196_1,  };

static const char rule_kid_197_0 [] = { (char)0, (char)0, (char)-1,  };
static const char rule_kid_197_1 [] = { (char)0, (char)1, (char)-1,  };
static const char rule_kid_197_2 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_197 [] = { rule_kid_197_0, rule_kid_197_1, rule_kid_197_2,  };

static const char rule_kid_198_0 [] = { (char)0, (char)0, (char)-1,  };
static const char rule_kid_198_1 [] = { (char)0, (char)1, (char)-1,  };
static const char rule_kid_198_2 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_198 [] = { rule_kid_198_0, rule_kid_198_1, rule_kid_198_2,  };

static const char rule_kid_199_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_199_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_199 [] = { rule_kid_199_0, rule_kid_199_1,  };

static const char rule_kid_200_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_200_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_200 [] = { rule_kid_200_0, rule_kid_200_1,  };

static const char rule_kid_201_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_201_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_201 [] = { rule_kid_201_0, rule_kid_201_1,  };

static const char rule_kid_202_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_202_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_202 [] = { rule_kid_202_0, rule_kid_202_1,  };

static const char rule_kid_203_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_203_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_203 [] = { rule_kid_203_0, rule_kid_203_1,  };

static const char rule_kid_204_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_204_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_204 [] = { rule_kid_204_0, rule_kid_204_1,  };

static const char rule_kid_205_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_205_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_205 [] = { rule_kid_205_0, rule_kid_205_1,  };

static const char rule_kid_206_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_206_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_206 [] = { rule_kid_206_0, rule_kid_206_1,  };

static const char rule_kid_207_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_207_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_207 [] = { rule_kid_207_0, rule_kid_207_1,  };

static const char rule_kid_208_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_208_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_208 [] = { rule_kid_208_0, rule_kid_208_1,  };

static const char rule_kid_209_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_209_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_209 [] = { rule_kid_209_0, rule_kid_209_1,  };

static const char rule_kid_210_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_210_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_210 [] = { rule_kid_210_0, rule_kid_210_1,  };

static const char rule_kid_211_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_211_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_211 [] = { rule_kid_211_0, rule_kid_211_1,  };

static const char rule_kid_212_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_212_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_212 [] = { rule_kid_212_0, rule_kid_212_1,  };

static const char rule_kid_213_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_213_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_213 [] = { rule_kid_213_0, rule_kid_213_1,  };

static const char rule_kid_214_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_214_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_214 [] = { rule_kid_214_0, rule_kid_214_1,  };

static const char rule_kid_215_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_215_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_215 [] = { rule_kid_215_0, rule_kid_215_1,  };

static const char rule_kid_216_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_216_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_216 [] = { rule_kid_216_0, rule_kid_216_1,  };

static const char rule_kid_217_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_217_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_217 [] = { rule_kid_217_0, rule_kid_217_1,  };

static const char rule_kid_218_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_218_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_218 [] = { rule_kid_218_0, rule_kid_218_1,  };

static const char rule_kid_219_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_219_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_219 [] = { rule_kid_219_0, rule_kid_219_1,  };

static const char rule_kid_220_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_220_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_220 [] = { rule_kid_220_0, rule_kid_220_1,  };

static const char rule_kid_221_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_221_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_221 [] = { rule_kid_221_0, rule_kid_221_1,  };

static const char rule_kid_222_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_222_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_222 [] = { rule_kid_222_0, rule_kid_222_1,  };

static const char rule_kid_223_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_223_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_223 [] = { rule_kid_223_0, rule_kid_223_1,  };

static const char rule_kid_224_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_224 [] = { rule_kid_224_0,  };

static const char rule_kid_225_0 [] = { (char)0, (char)0, (char)-1,  };
static const char rule_kid_225_1 [] = { (char)0, (char)1, (char)-1,  };
static const char rule_kid_225_2 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_225 [] = { rule_kid_225_0, rule_kid_225_1, rule_kid_225_2,  };

static const char rule_kid_226_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_226_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_226 [] = { rule_kid_226_0, rule_kid_226_1,  };

static const char rule_kid_227_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_227 [] = { rule_kid_227_0,  };

static const char rule_kid_228_0 [] = { (char)0, (char)0, (char)-1,  };
static const char rule_kid_228_1 [] = { (char)0, (char)1, (char)-1,  };
static const char rule_kid_228_2 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_228 [] = { rule_kid_228_0, rule_kid_228_1, rule_kid_228_2,  };

static const char rule_kid_229_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_229_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_229 [] = { rule_kid_229_0, rule_kid_229_1,  };

static const char rule_kid_230_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_230_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_230 [] = { rule_kid_230_0, rule_kid_230_1,  };

static const char rule_kid_231_0 [] = { (char)-1,  };

static const char * const rule_kids_231 [] = { rule_kid_231_0,  };

static const char rule_kid_232_0 [] = { (char)-1,  };

static const char * const rule_kids_232 [] = { rule_kid_232_0,  };

static const char rule_kid_233_0 [] = { (char)-1,  };

static const char * const rule_kids_233 [] = { rule_kid_233_0,  };

static const char rule_kid_234_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_234 [] = { rule_kid_234_0,  };

static const char rule_kid_235_0 [] = { (char)-1,  };

static const char * const rule_kids_235 [] = { rule_kid_235_0,  };

static const char rule_kid_236_0 [] = { (char)-1,  };

static const char * const rule_kids_236 [] = { rule_kid_236_0,  };

static const char rule_kid_237_0 [] = { (char)-1,  };

static const char * const rule_kids_237 [] = { rule_kid_237_0,  };

static const char rule_kid_238_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_238 [] = { rule_kid_238_0,  };

static const char rule_kid_239_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_239_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_239 [] = { rule_kid_239_0, rule_kid_239_1,  };

static const char rule_kid_240_0 [] = { (char)-1,  };

static const char * const rule_kids_240 [] = { rule_kid_240_0,  };

static const char rule_kid_241_0 [] = { (char)-1,  };

static const char * const rule_kids_241 [] = { rule_kid_241_0,  };

static const char rule_kid_242_0 [] = { (char)-1,  };

static const char * const rule_kids_242 [] = { rule_kid_242_0,  };

static const char rule_kid_243_0 [] = { (char)-1,  };

static const char * const rule_kids_243 [] = { rule_kid_243_0,  };

static const char rule_kid_244_0 [] = { (char)-1,  };

static const char * const rule_kids_244 [] = { rule_kid_244_0,  };

static const char rule_kid_245_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_245 [] = { rule_kid_245_0,  };

static const char rule_kid_246_0 [] = { (char)0, (char)0, (char)-1,  };
static const char rule_kid_246_1 [] = { (char)0, (char)1, (char)-1,  };
static const char rule_kid_246_2 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_246 [] = { rule_kid_246_0, rule_kid_246_1, rule_kid_246_2,  };

static const char rule_kid_247_0 [] = { (char)-1,  };

static const char * const rule_kids_247 [] = { rule_kid_247_0,  };

static const char rule_kid_248_0 [] = { (char)-1,  };

static const char * const rule_kids_248 [] = { rule_kid_248_0,  };

static const char rule_kid_249_0 [] = { (char)-1,  };

static const char * const rule_kids_249 [] = { rule_kid_249_0,  };

static const char rule_kid_250_0 [] = { (char)-1,  };

static const char * const rule_kids_250 [] = { rule_kid_250_0,  };

static const char rule_kid_251_0 [] = { (char)-1,  };

static const char * const rule_kids_251 [] = { rule_kid_251_0,  };

static const char rule_kid_252_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_252 [] = { rule_kid_252_0,  };

static const char rule_kid_253_0 [] = { (char)0, (char)0, (char)-1,  };
static const char rule_kid_253_1 [] = { (char)0, (char)1, (char)-1,  };
static const char rule_kid_253_2 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_253 [] = { rule_kid_253_0, rule_kid_253_1, rule_kid_253_2,  };

static const char rule_kid_254_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_254_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_254 [] = { rule_kid_254_0, rule_kid_254_1,  };

static const char rule_kid_255_0 [] = { (char)-1,  };

static const char * const rule_kids_255 [] = { rule_kid_255_0,  };

static const char rule_kid_256_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_256 [] = { rule_kid_256_0,  };

static const char rule_kid_257_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_257_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_257 [] = { rule_kid_257_0, rule_kid_257_1,  };

static const char rule_kid_258_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_258_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_258 [] = { rule_kid_258_0, rule_kid_258_1,  };

static const char rule_kid_259_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_259 [] = { rule_kid_259_0,  };

static const char rule_kid_260_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_260 [] = { rule_kid_260_0,  };

static const char rule_kid_261_0 [] = { (char)-1,  };

static const char * const rule_kids_261 [] = { rule_kid_261_0,  };

static const char rule_kid_262_0 [] = { (char)-1,  };

static const char * const rule_kids_262 [] = { rule_kid_262_0,  };

static const char rule_kid_263_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_263 [] = { rule_kid_263_0,  };

static const char rule_kid_264_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_264_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_264 [] = { rule_kid_264_0, rule_kid_264_1,  };

static const char rule_kid_265_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_265_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_265 [] = { rule_kid_265_0, rule_kid_265_1,  };

static const char rule_kid_266_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_266_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_266 [] = { rule_kid_266_0, rule_kid_266_1,  };

static const char rule_kid_267_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_267_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_267 [] = { rule_kid_267_0, rule_kid_267_1,  };

static const char rule_kid_268_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_268 [] = { rule_kid_268_0,  };

static const char rule_kid_269_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_269 [] = { rule_kid_269_0,  };

static const char rule_kid_270_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_270 [] = { rule_kid_270_0,  };

static const char rule_kid_271_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_271_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_271 [] = { rule_kid_271_0, rule_kid_271_1,  };

static const char rule_kid_272_0 [] = { (char)-1,  };

static const char * const rule_kids_272 [] = { rule_kid_272_0,  };

static const char rule_kid_273_0 [] = { (char)-1,  };

static const char * const rule_kids_273 [] = { rule_kid_273_0,  };

static const char rule_kid_274_0 [] = { (char)-1,  };

static const char * const rule_kids_274 [] = { rule_kid_274_0,  };

static const char rule_kid_275_0 [] = { (char)-1,  };

static const char * const rule_kids_275 [] = { rule_kid_275_0,  };

static const char rule_kid_276_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_276 [] = { rule_kid_276_0,  };

static const char rule_kid_277_0 [] = { (char)0, (char)0, (char)-1,  };
static const char rule_kid_277_1 [] = { (char)0, (char)1, (char)-1,  };
static const char rule_kid_277_2 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_277 [] = { rule_kid_277_0, rule_kid_277_1, rule_kid_277_2,  };

static const char rule_kid_278_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_278_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_278 [] = { rule_kid_278_0, rule_kid_278_1,  };

static const char rule_kid_279_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_279 [] = { rule_kid_279_0,  };

static const char rule_kid_280_0 [] = { (char)0, (char)0, (char)-1,  };
static const char rule_kid_280_1 [] = { (char)0, (char)1, (char)-1,  };
static const char rule_kid_280_2 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_280 [] = { rule_kid_280_0, rule_kid_280_1, rule_kid_280_2,  };

static const char rule_kid_281_0 [] = { (char)-1,  };

static const char * const rule_kids_281 [] = { rule_kid_281_0,  };

static const char rule_kid_282_0 [] = { (char)-1,  };

static const char * const rule_kids_282 [] = { rule_kid_282_0,  };

static const char rule_kid_283_0 [] = { (char)-1,  };

static const char * const rule_kids_283 [] = { rule_kid_283_0,  };

static const char rule_kid_284_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_284 [] = { rule_kid_284_0,  };

static const char rule_kid_285_0 [] = { (char)0, (char)0, (char)-1,  };
static const char rule_kid_285_1 [] = { (char)0, (char)1, (char)-1,  };
static const char rule_kid_285_2 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_285 [] = { rule_kid_285_0, rule_kid_285_1, rule_kid_285_2,  };

static const char rule_kid_286_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_286_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_286 [] = { rule_kid_286_0, rule_kid_286_1,  };

static const char rule_kid_287_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_287_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_287 [] = { rule_kid_287_0, rule_kid_287_1,  };

static const char rule_kid_288_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_288 [] = { rule_kid_288_0,  };

static const char rule_kid_289_0 [] = { (char)0, (char)0, (char)-1,  };
static const char rule_kid_289_1 [] = { (char)0, (char)1, (char)-1,  };
static const char rule_kid_289_2 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_289 [] = { rule_kid_289_0, rule_kid_289_1, rule_kid_289_2,  };

static const char rule_kid_290_0 [] = { (char)0, (char)-1,  };
static const char rule_kid_290_1 [] = { (char)1, (char)-1,  };

static const char * const rule_kids_290 [] = { rule_kid_290_0, rule_kid_290_1,  };

static const char rule_kid_291_0 [] = { (char)-1,  };

static const char * const rule_kids_291 [] = { rule_kid_291_0,  };


static const char * const rule_kids_292 [] = {  };


static const char * const rule_kids_293 [] = {  };


static const char * const rule_kids_294 [] = {  };

static const char rule_kid_295_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_295 [] = { rule_kid_295_0,  };


static const char * const rule_kids_296 [] = {  };


static const char * const rule_kids_297 [] = {  };


static const char * const rule_kids_298 [] = {  };

static const char rule_kid_299_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_299 [] = { rule_kid_299_0,  };


static const char * const rule_kids_300 [] = {  };


static const char * const rule_kids_301 [] = {  };


static const char * const rule_kids_302 [] = {  };

static const char rule_kid_303_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_303 [] = { rule_kid_303_0,  };


static const char * const rule_kids_304 [] = {  };


static const char * const rule_kids_305 [] = {  };


static const char * const rule_kids_306 [] = {  };

static const char rule_kid_307_0 [] = { (char)0, (char)-1,  };

static const char * const rule_kids_307 [] = { rule_kid_307_0,  };

static const char rule_kid_308_0 [] = { (char)-1,  };

static const char * const rule_kids_308 [] = { rule_kid_308_0,  };

static const char rule_kid_309_0 [] = { (char)-1,  };

static const char * const rule_kids_309 [] = { rule_kid_309_0,  };

static const char rule_kid_310_0 [] = { (char)-1,  };

static const char * const rule_kids_310 [] = { rule_kid_310_0,  };

static const char rule_kid_311_0 [] = { (char)-1,  };

static const char * const rule_kids_311 [] = { rule_kid_311_0,  };

static const char rule_kid_312_0 [] = { (char)-1,  };

static const char * const rule_kids_312 [] = { rule_kid_312_0,  };


static const char * const rule_kids_313 [] = {  };

static const char * const * const rule_kids[] = {
  (const char * const *)0,
  rule_kids_1, 
  rule_kids_2, 
  rule_kids_3, 
  rule_kids_4, 
  rule_kids_5, 
  rule_kids_6, 
  rule_kids_7, 
  rule_kids_8, 
  rule_kids_9, 
  rule_kids_10, 
  rule_kids_11, 
  rule_kids_12, 
  rule_kids_13, 
  rule_kids_14, 
  rule_kids_15, 
  rule_kids_16, 
  rule_kids_17, 
  rule_kids_18, 
  rule_kids_19, 
  rule_kids_20, 
  rule_kids_21, 
  rule_kids_22, 
  rule_kids_23, 
  rule_kids_24, 
  rule_kids_25, 
  rule_kids_26, 
  rule_kids_27, 
  rule_kids_28, 
  rule_kids_29, 
  rule_kids_30, 
  rule_kids_31, 
  rule_kids_32, 
  rule_kids_33, 
  rule_kids_34, 
  rule_kids_35, 
  rule_kids_36, 
  rule_kids_37, 
  rule_kids_38, 
  rule_kids_39, 
  rule_kids_40, 
  rule_kids_41, 
  rule_kids_42, 
  rule_kids_43, 
  rule_kids_44, 
  rule_kids_45, 
  rule_kids_46, 
  rule_kids_47, 
  rule_kids_48, 
  rule_kids_49, 
  rule_kids_50, 
  rule_kids_51, 
  rule_kids_52, 
  rule_kids_53, 
  rule_kids_54, 
  rule_kids_55, 
  rule_kids_56, 
  rule_kids_57, 
  rule_kids_58, 
  rule_kids_59, 
  rule_kids_60, 
  rule_kids_61, 
  rule_kids_62, 
  rule_kids_63, 
  rule_kids_64, 
  rule_kids_65, 
  rule_kids_66, 
  rule_kids_67, 
  rule_kids_68, 
  rule_kids_69, 
  rule_kids_70, 
  rule_kids_71, 
  rule_kids_72, 
  rule_kids_73, 
  rule_kids_74, 
  rule_kids_75, 
  rule_kids_76, 
  rule_kids_77, 
  rule_kids_78, 
  rule_kids_79, 
  rule_kids_80, 
  rule_kids_81, 
  rule_kids_82, 
  rule_kids_83, 
  rule_kids_84, 
  rule_kids_85, 
  rule_kids_86, 
  rule_kids_87, 
  rule_kids_88, 
  rule_kids_89, 
  rule_kids_90, 
  rule_kids_91, 
  rule_kids_92, 
  rule_kids_93, 
  rule_kids_94, 
  rule_kids_95, 
  rule_kids_96, 
  rule_kids_97, 
  rule_kids_98, 
  rule_kids_99, 
  rule_kids_100, 
  rule_kids_101, 
  rule_kids_102, 
  rule_kids_103, 
  rule_kids_104, 
  rule_kids_105, 
  rule_kids_106, 
  rule_kids_107, 
  rule_kids_108, 
  rule_kids_109, 
  rule_kids_110, 
  rule_kids_111, 
  rule_kids_112, 
  rule_kids_113, 
  rule_kids_114, 
  rule_kids_115, 
  rule_kids_116, 
  rule_kids_117, 
  rule_kids_118, 
  rule_kids_119, 
  rule_kids_120, 
  rule_kids_121, 
  rule_kids_122, 
  rule_kids_123, 
  rule_kids_124, 
  rule_kids_125, 
  rule_kids_126, 
  rule_kids_127, 
  rule_kids_128, 
  rule_kids_129, 
  rule_kids_130, 
  rule_kids_131, 
  rule_kids_132, 
  rule_kids_133, 
  rule_kids_134, 
  rule_kids_135, 
  rule_kids_136, 
  rule_kids_137, 
  rule_kids_138, 
  rule_kids_139, 
  rule_kids_140, 
  rule_kids_141, 
  rule_kids_142, 
  rule_kids_143, 
  rule_kids_144, 
  rule_kids_145, 
  rule_kids_146, 
  rule_kids_147, 
  rule_kids_148, 
  rule_kids_149, 
  rule_kids_150, 
  rule_kids_151, 
  rule_kids_152, 
  rule_kids_153, 
  rule_kids_154, 
  rule_kids_155, 
  rule_kids_156, 
  rule_kids_157, 
  rule_kids_158, 
  rule_kids_159, 
  rule_kids_160, 
  rule_kids_161, 
  rule_kids_162, 
  rule_kids_163, 
  rule_kids_164, 
  rule_kids_165, 
  rule_kids_166, 
  rule_kids_167, 
  rule_kids_168, 
  rule_kids_169, 
  rule_kids_170, 
  rule_kids_171, 
  rule_kids_172, 
  rule_kids_173, 
  rule_kids_174, 
  rule_kids_175, 
  rule_kids_176, 
  rule_kids_177, 
  rule_kids_178, 
  rule_kids_179, 
  rule_kids_180, 
  rule_kids_181, 
  rule_kids_182, 
  rule_kids_183, 
  rule_kids_184, 
  rule_kids_185, 
  rule_kids_186, 
  rule_kids_187, 
  rule_kids_188, 
  rule_kids_189, 
  rule_kids_190, 
  rule_kids_191, 
  rule_kids_192, 
  rule_kids_193, 
  rule_kids_194, 
  rule_kids_195, 
  rule_kids_196, 
  rule_kids_197, 
  rule_kids_198, 
  rule_kids_199, 
  rule_kids_200, 
  rule_kids_201, 
  rule_kids_202, 
  rule_kids_203, 
  rule_kids_204, 
  rule_kids_205, 
  rule_kids_206, 
  rule_kids_207, 
  rule_kids_208, 
  rule_kids_209, 
  rule_kids_210, 
  rule_kids_211, 
  rule_kids_212, 
  rule_kids_213, 
  rule_kids_214, 
  rule_kids_215, 
  rule_kids_216, 
  rule_kids_217, 
  rule_kids_218, 
  rule_kids_219, 
  rule_kids_220, 
  rule_kids_221, 
  rule_kids_222, 
  rule_kids_223, 
  rule_kids_224, 
  rule_kids_225, 
  rule_kids_226, 
  rule_kids_227, 
  rule_kids_228, 
  rule_kids_229, 
  rule_kids_230, 
  rule_kids_231, 
  rule_kids_232, 
  rule_kids_233, 
  rule_kids_234, 
  rule_kids_235, 
  rule_kids_236, 
  rule_kids_237, 
  rule_kids_238, 
  rule_kids_239, 
  rule_kids_240, 
  rule_kids_241, 
  rule_kids_242, 
  rule_kids_243, 
  rule_kids_244, 
  rule_kids_245, 
  rule_kids_246, 
  rule_kids_247, 
  rule_kids_248, 
  rule_kids_249, 
  rule_kids_250, 
  rule_kids_251, 
  rule_kids_252, 
  rule_kids_253, 
  rule_kids_254, 
  rule_kids_255, 
  rule_kids_256, 
  rule_kids_257, 
  rule_kids_258, 
  rule_kids_259, 
  rule_kids_260, 
  rule_kids_261, 
  rule_kids_262, 
  rule_kids_263, 
  rule_kids_264, 
  rule_kids_265, 
  rule_kids_266, 
  rule_kids_267, 
  rule_kids_268, 
  rule_kids_269, 
  rule_kids_270, 
  rule_kids_271, 
  rule_kids_272, 
  rule_kids_273, 
  rule_kids_274, 
  rule_kids_275, 
  rule_kids_276, 
  rule_kids_277, 
  rule_kids_278, 
  rule_kids_279, 
  rule_kids_280, 
  rule_kids_281, 
  rule_kids_282, 
  rule_kids_283, 
  rule_kids_284, 
  rule_kids_285, 
  rule_kids_286, 
  rule_kids_287, 
  rule_kids_288, 
  rule_kids_289, 
  rule_kids_290, 
  rule_kids_291, 
  rule_kids_292, 
  rule_kids_293, 
  rule_kids_294, 
  rule_kids_295, 
  rule_kids_296, 
  rule_kids_297, 
  rule_kids_298, 
  rule_kids_299, 
  rule_kids_300, 
  rule_kids_301, 
  rule_kids_302, 
  rule_kids_303, 
  rule_kids_304, 
  rule_kids_305, 
  rule_kids_306, 
  rule_kids_307, 
  rule_kids_308, 
  rule_kids_309, 
  rule_kids_310, 
  rule_kids_311, 
  rule_kids_312, 
  rule_kids_313, 
  
};

static void
NBG_compute_state(NBG_State state, void *user_state)
{
  NBG_Cost cost;
  switch(NBG_State_op(state)) {
  case TERM_OP(CEIL):
    /* reg:CEIL(reg) */
    if (NBG_State_op(state) == TERM_OP(CEIL) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      1) {
      cost = (FAIL_COST&&FLT_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 97);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    break;
  
  case TERM_OP(MINPART):
    /* reg:MINPART(reg) */
    if (NBG_State_op(state) == TERM_OP(MINPART) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      1) {
      cost = (FAIL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 128);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    break;
  
  case TERM_OP(STBITS):
    /* stmt:STBITS###CLASS_PREG(reg) */
    if (NBG_State_op(state) == TERM_OP(STBITS) &&
      NBG_TREE_VALUE_AT((NBG_TREE)NBG_State_utree(state), 2) == TERM_VALUE(CLASS_PREG) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      1) {
      cost = (1+1);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(stmt))) {
          NBG_State__cost(state, NTERM(stmt), cost);
          NBG_State__rule(state, NTERM(stmt), 10);
          NBG_compute_closure(state, NTERM(stmt));
        }
        }
      }
    
    /* stmt:STBITS(reg) */
    if (NBG_State_op(state) == TERM_OP(STBITS) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      1) {
      cost = (1+ST_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(stmt))) {
          NBG_State__cost(state, NTERM(stmt), cost);
          NBG_State__rule(state, NTERM(stmt), 11);
          NBG_compute_closure(state, NTERM(stmt));
        }
        }
      }
    
    break;
  
  case TERM_OP(MAXPART):
    /* reg:MAXPART(reg) */
    if (NBG_State_op(state) == TERM_OP(MAXPART) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      1) {
      cost = (FAIL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 129);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    break;
  
  case TERM_OP(RCOMMA):
    /* inv:RCOMMA(inv,inv) */
    if (NBG_State_op(state) == TERM_OP(RCOMMA) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(inv)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(inv)) &&
      1) {
      cost = (FAIL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(inv));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(inv));
        if (cost < NBG_State_cost(state, NTERM(inv))) {
          NBG_State__cost(state, NTERM(inv), cost);
          NBG_State__rule(state, NTERM(inv), 149);
          
        }
        }
      }
    
    break;
  
  case TERM_OP(STID):
    /* stmt:STID###CLASS_PREG(reg) */
    if (NBG_State_op(state) == TERM_OP(STID) &&
      NBG_TREE_VALUE_AT((NBG_TREE)NBG_State_utree(state), 2) == TERM_VALUE(CLASS_PREG) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(stmt))) {
          NBG_State__cost(state, NTERM(stmt), cost);
          NBG_State__rule(state, NTERM(stmt), 5);
          NBG_compute_closure(state, NTERM(stmt));
        }
        }
      }
    
    /* stmt:STID(reg) */
    if (NBG_State_op(state) == TERM_OP(STID) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      1) {
      cost = (ST_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(stmt))) {
          NBG_State__cost(state, NTERM(stmt), cost);
          NBG_State__rule(state, NTERM(stmt), 6);
          NBG_compute_closure(state, NTERM(stmt));
        }
        }
      }
    
    break;
  
  case TERM_OP(ARRAY):
    /* inv:ARRAY(inv) */
    if (NBG_State_op(state) == TERM_OP(ARRAY) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(inv)) &&
      1) {
      cost = (FAIL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(inv));
        if (cost < NBG_State_cost(state, NTERM(inv))) {
          NBG_State__cost(state, NTERM(inv), cost);
          NBG_State__rule(state, NTERM(inv), 150);
          
        }
        }
      }
    
    break;
  
  case TERM_OP(CSELECT):
    /* inv:CSELECT(inv,inv) */
    if (NBG_State_op(state) == TERM_OP(CSELECT) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(inv)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(inv)) &&
      1) {
      cost = (FAIL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(inv));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(inv));
        if (cost < NBG_State_cost(state, NTERM(inv))) {
          NBG_State__cost(state, NTERM(inv), cost);
          NBG_State__rule(state, NTERM(inv), 144);
          
        }
        }
      }
    
    break;
  
  case TERM_OP(ILDBITS):
    /* reg:ILDBITS(LDA) */
    if (NBG_State_op(state) == TERM_OP(ILDBITS) &&
      NBG_State_op(NBG_State_get_child(state, 0)) == TERM_OP(LDA) &&
      1) {
      cost = (1+LD_COST);
      if (cost != NBG_COST_UNDEF) {
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 22);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg:ILDBITS(reg) */
    if (NBG_State_op(state) == TERM_OP(ILDBITS) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      1) {
      cost = (1+LD_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 23);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    break;
  
  case TERM_OP(ISTORE):
    /* stmt:ISTORE(reg,LDA) */
    if (NBG_State_op(state) == TERM_OP(ISTORE) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_op(NBG_State_get_child(state, 1)) == TERM_OP(LDA) &&
      1) {
      cost = (ST_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(stmt))) {
          NBG_State__cost(state, NTERM(stmt), cost);
          NBG_State__rule(state, NTERM(stmt), 7);
          NBG_compute_closure(state, NTERM(stmt));
        }
        }
      }
    
    /* stmt:ISTORE(reg,reg) */
    if (NBG_State_op(state) == TERM_OP(ISTORE) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(reg)) &&
      1) {
      cost = (ST_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(stmt))) {
          NBG_State__cost(state, NTERM(stmt), cost);
          NBG_State__rule(state, NTERM(stmt), 8);
          NBG_compute_closure(state, NTERM(stmt));
        }
        }
      }
    
    break;
  
  case TERM_OP(ICALL):
    /* stmt:ICALL.V */
    if (NBG_State_op(state) == TERM_OP(ICALL) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(V) &&
      1) {
      cost = (1);
      if (cost != NBG_COST_UNDEF) {
        if (cost < NBG_State_cost(state, NTERM(stmt))) {
          NBG_State__cost(state, NTERM(stmt), cost);
          NBG_State__rule(state, NTERM(stmt), 121);
          NBG_compute_closure(state, NTERM(stmt));
        }
        }
      }
    
    /* stmt:ICALL */
    if (NBG_State_op(state) == TERM_OP(ICALL) &&
      1) {
      cost = (1);
      if (cost != NBG_COST_UNDEF) {
        if (cost < NBG_State_cost(state, NTERM(stmt))) {
          NBG_State__cost(state, NTERM(stmt), cost);
          NBG_State__rule(state, NTERM(stmt), 122);
          NBG_compute_closure(state, NTERM(stmt));
        }
        }
      }
    
    break;
  
  case TERM_OP(CVTL):
    /* reg:CVTL.I4##16(reg) */
    if (NBG_State_op(state) == TERM_OP(CVTL) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(I4) &&
      NBG_TREE_VALUE_AT((NBG_TREE)NBG_State_utree(state), 1) == TERM_VALUE(16) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      1) {
      cost = (CVTL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 99);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg:CVTL.U4##16(reg) */
    if (NBG_State_op(state) == TERM_OP(CVTL) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(U4) &&
      NBG_TREE_VALUE_AT((NBG_TREE)NBG_State_utree(state), 1) == TERM_VALUE(16) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      1) {
      cost = (CVTL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 100);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg:CVTL.I4##8(reg) */
    if (NBG_State_op(state) == TERM_OP(CVTL) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(I4) &&
      NBG_TREE_VALUE_AT((NBG_TREE)NBG_State_utree(state), 1) == TERM_VALUE(8) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      1) {
      cost = (CVTL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 101);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg:CVTL.U4##8(reg) */
    if (NBG_State_op(state) == TERM_OP(CVTL) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(U4) &&
      NBG_TREE_VALUE_AT((NBG_TREE)NBG_State_utree(state), 1) == TERM_VALUE(8) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      1) {
      cost = (CVTL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 102);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg:CVTL(reg) */
    if (NBG_State_op(state) == TERM_OP(CVTL) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      1) {
      cost = (CVTL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 103);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* m_reg_sx16h:CVTL.I4##16(m_reg_sx16h) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(CVTL) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(I4) &&
      NBG_TREE_VALUE_AT((NBG_TREE)NBG_State_utree(state), 1) == TERM_VALUE(16) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(m_reg_sx16h)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(m_reg_sx16h));
        if (cost < NBG_State_cost(state, NTERM(m_reg_sx16h))) {
          NBG_State__cost(state, NTERM(m_reg_sx16h), cost);
          NBG_State__rule(state, NTERM(m_reg_sx16h), 224);
          NBG_compute_closure(state, NTERM(m_reg_sx16h));
        }
        }
      }
    
    /* m_reg_zx16h:CVTL.U4##16(m_reg_zx16h) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(CVTL) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(U4) &&
      NBG_TREE_VALUE_AT((NBG_TREE)NBG_State_utree(state), 1) == TERM_VALUE(16) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(m_reg_zx16h)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(m_reg_zx16h));
        if (cost < NBG_State_cost(state, NTERM(m_reg_zx16h))) {
          NBG_State__cost(state, NTERM(m_reg_zx16h), cost);
          NBG_State__rule(state, NTERM(m_reg_zx16h), 227);
          NBG_compute_closure(state, NTERM(m_reg_zx16h));
        }
        }
      }
    
    /* m_reg_sx8:CVTL.I4##8(reg_or_m_reg_sx16) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(CVTL) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(I4) &&
      NBG_TREE_VALUE_AT((NBG_TREE)NBG_State_utree(state), 1) == TERM_VALUE(8) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg_or_m_reg_sx16)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg_or_m_reg_sx16));
        if (cost < NBG_State_cost(state, NTERM(m_reg_sx8))) {
          NBG_State__cost(state, NTERM(m_reg_sx8), cost);
          NBG_State__rule(state, NTERM(m_reg_sx8), 234);
          NBG_compute_closure(state, NTERM(m_reg_sx8));
        }
        }
      }
    
    /* m_reg_zx8:CVTL.U4##8(reg_or_m_reg_zx8) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(CVTL) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(U4) &&
      NBG_TREE_VALUE_AT((NBG_TREE)NBG_State_utree(state), 1) == TERM_VALUE(8) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg_or_m_reg_zx8)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg_or_m_reg_zx8));
        if (cost < NBG_State_cost(state, NTERM(m_reg_zx8))) {
          NBG_State__cost(state, NTERM(m_reg_zx8), cost);
          NBG_State__rule(state, NTERM(m_reg_zx8), 238);
          NBG_compute_closure(state, NTERM(m_reg_zx8));
        }
        }
      }
    
    /* m_reg_sx16:CVTL.I4##16(reg_or_m_reg_sx16) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(CVTL) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(I4) &&
      NBG_TREE_VALUE_AT((NBG_TREE)NBG_State_utree(state), 1) == TERM_VALUE(16) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg_or_m_reg_sx16)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg_or_m_reg_sx16));
        if (cost < NBG_State_cost(state, NTERM(m_reg_sx16))) {
          NBG_State__cost(state, NTERM(m_reg_sx16), cost);
          NBG_State__rule(state, NTERM(m_reg_sx16), 245);
          NBG_compute_closure(state, NTERM(m_reg_sx16));
        }
        }
      }
    
    /* m_reg_zx16:CVTL.U4##16(reg_or_m_reg_zx16) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(CVTL) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(U4) &&
      NBG_TREE_VALUE_AT((NBG_TREE)NBG_State_utree(state), 1) == TERM_VALUE(16) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg_or_m_reg_zx16)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg_or_m_reg_zx16));
        if (cost < NBG_State_cost(state, NTERM(m_reg_zx16))) {
          NBG_State__cost(state, NTERM(m_reg_zx16), cost);
          NBG_State__rule(state, NTERM(m_reg_zx16), 252);
          NBG_compute_closure(state, NTERM(m_reg_zx16));
        }
        }
      }
    
    /* reg_sx8:CVTL.I4##8(reg) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(CVTL) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(I4) &&
      NBG_TREE_VALUE_AT((NBG_TREE)NBG_State_utree(state), 1) == TERM_VALUE(8) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      1) {
      cost = (CVTL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg_sx8))) {
          NBG_State__cost(state, NTERM(reg_sx8), cost);
          NBG_State__rule(state, NTERM(reg_sx8), 256);
          NBG_compute_closure(state, NTERM(reg_sx8));
        }
        }
      }
    
    /* reg_sx8:CVTL.I4##8(reg_sx8) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(CVTL) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(I4) &&
      NBG_TREE_VALUE_AT((NBG_TREE)NBG_State_utree(state), 1) == TERM_VALUE(8) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg_sx8)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg_sx8));
        if (cost < NBG_State_cost(state, NTERM(reg_sx8))) {
          NBG_State__cost(state, NTERM(reg_sx8), cost);
          NBG_State__rule(state, NTERM(reg_sx8), 259);
          NBG_compute_closure(state, NTERM(reg_sx8));
        }
        }
      }
    
    /* reg_sx8:CVTL.I4##16(reg_sx8) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(CVTL) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(I4) &&
      NBG_TREE_VALUE_AT((NBG_TREE)NBG_State_utree(state), 1) == TERM_VALUE(16) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg_sx8)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg_sx8));
        if (cost < NBG_State_cost(state, NTERM(reg_sx8))) {
          NBG_State__cost(state, NTERM(reg_sx8), cost);
          NBG_State__rule(state, NTERM(reg_sx8), 260);
          NBG_compute_closure(state, NTERM(reg_sx8));
        }
        }
      }
    
    /* reg_zx8:CVTL.U4##8(reg) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(CVTL) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(U4) &&
      NBG_TREE_VALUE_AT((NBG_TREE)NBG_State_utree(state), 1) == TERM_VALUE(8) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      1) {
      cost = (CVTL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg_zx8))) {
          NBG_State__cost(state, NTERM(reg_zx8), cost);
          NBG_State__rule(state, NTERM(reg_zx8), 263);
          NBG_compute_closure(state, NTERM(reg_zx8));
        }
        }
      }
    
    /* reg_zx8:CVTL.U4##8(reg_zx8) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(CVTL) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(U4) &&
      NBG_TREE_VALUE_AT((NBG_TREE)NBG_State_utree(state), 1) == TERM_VALUE(8) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg_zx8)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg_zx8));
        if (cost < NBG_State_cost(state, NTERM(reg_zx8))) {
          NBG_State__cost(state, NTERM(reg_zx8), cost);
          NBG_State__rule(state, NTERM(reg_zx8), 268);
          NBG_compute_closure(state, NTERM(reg_zx8));
        }
        }
      }
    
    /* reg_zx8:CVTL.U4##16(reg_zx8) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(CVTL) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(U4) &&
      NBG_TREE_VALUE_AT((NBG_TREE)NBG_State_utree(state), 1) == TERM_VALUE(16) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg_zx8)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg_zx8));
        if (cost < NBG_State_cost(state, NTERM(reg_zx8))) {
          NBG_State__cost(state, NTERM(reg_zx8), cost);
          NBG_State__rule(state, NTERM(reg_zx8), 269);
          NBG_compute_closure(state, NTERM(reg_zx8));
        }
        }
      }
    
    /* reg_zx8:CVTL.I4##16(reg_zx8) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(CVTL) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(I4) &&
      NBG_TREE_VALUE_AT((NBG_TREE)NBG_State_utree(state), 1) == TERM_VALUE(16) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg_zx8)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg_zx8));
        if (cost < NBG_State_cost(state, NTERM(reg_zx8))) {
          NBG_State__cost(state, NTERM(reg_zx8), cost);
          NBG_State__rule(state, NTERM(reg_zx8), 270);
          NBG_compute_closure(state, NTERM(reg_zx8));
        }
        }
      }
    
    /* reg_sx16:CVTL.I4##16(reg) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(CVTL) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(I4) &&
      NBG_TREE_VALUE_AT((NBG_TREE)NBG_State_utree(state), 1) == TERM_VALUE(16) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      1) {
      cost = (CVTL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg_sx16))) {
          NBG_State__cost(state, NTERM(reg_sx16), cost);
          NBG_State__rule(state, NTERM(reg_sx16), 276);
          NBG_compute_closure(state, NTERM(reg_sx16));
        }
        }
      }
    
    /* reg_sx16:CVTL.I4##16(reg_sx16) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(CVTL) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(I4) &&
      NBG_TREE_VALUE_AT((NBG_TREE)NBG_State_utree(state), 1) == TERM_VALUE(16) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg_sx16)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg_sx16));
        if (cost < NBG_State_cost(state, NTERM(reg_sx16))) {
          NBG_State__cost(state, NTERM(reg_sx16), cost);
          NBG_State__rule(state, NTERM(reg_sx16), 279);
          NBG_compute_closure(state, NTERM(reg_sx16));
        }
        }
      }
    
    /* reg_zx16:CVTL.U4##16(reg) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(CVTL) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(U4) &&
      NBG_TREE_VALUE_AT((NBG_TREE)NBG_State_utree(state), 1) == TERM_VALUE(16) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      1) {
      cost = (CVTL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg_zx16))) {
          NBG_State__cost(state, NTERM(reg_zx16), cost);
          NBG_State__rule(state, NTERM(reg_zx16), 284);
          NBG_compute_closure(state, NTERM(reg_zx16));
        }
        }
      }
    
    /* reg_zx16:CVTL.U4##16(reg_zx16) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(CVTL) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(U4) &&
      NBG_TREE_VALUE_AT((NBG_TREE)NBG_State_utree(state), 1) == TERM_VALUE(16) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg_zx16)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg_zx16));
        if (cost < NBG_State_cost(state, NTERM(reg_zx16))) {
          NBG_State__cost(state, NTERM(reg_zx16), cost);
          NBG_State__rule(state, NTERM(reg_zx16), 288);
          NBG_compute_closure(state, NTERM(reg_zx16));
        }
        }
      }
    
    break;
  
  case TERM_OP(BNOR):
    /* reg:BNOR(reg,rcon) */
    if (NBG_State_op(state) == TERM_OP(BNOR) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(rcon)) &&
      1) {
      cost = (FAIL_COST&&ARI_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(rcon));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 58);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg:BNOR(rcon,reg) */
    if (NBG_State_op(state) == TERM_OP(BNOR) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(rcon)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(reg)) &&
      1) {
      cost = (FAIL_COST&&ARI_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(rcon));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 59);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    break;
  
  case TERM_OP(PAREN):
    /* reg:PAREN(reg) */
    if (NBG_State_op(state) == TERM_OP(PAREN) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 106);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    break;
  
  case TERM_OP(BNOT):
    /* reg:BNOT(reg) */
    if (NBG_State_op(state) == TERM_OP(BNOT) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      1) {
      cost = (ARI_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 51);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    break;
  
  case TERM_OP(MOD):
    /* reg:MOD(reg,rcon) */
    if (NBG_State_op(state) == TERM_OP(MOD) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(rcon)) &&
      1) {
      cost = (FAIL_COST&&DIV_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(rcon));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 41);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    break;
  
  case TERM_OP(REGION_EXIT):
    /* stmt:REGION_EXIT */
    if (NBG_State_op(state) == TERM_OP(REGION_EXIT) &&
      1) {
      cost = (1);
      if (cost != NBG_COST_UNDEF) {
        if (cost < NBG_State_cost(state, NTERM(stmt))) {
          NBG_State__cost(state, NTERM(stmt), cost);
          NBG_State__rule(state, NTERM(stmt), 109);
          NBG_compute_closure(state, NTERM(stmt));
        }
        }
      }
    
    break;
  
  case TERM_OP(CAND):
    /* inv:CAND(inv,inv) */
    if (NBG_State_op(state) == TERM_OP(CAND) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(inv)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(inv)) &&
      1) {
      cost = (FAIL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(inv));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(inv));
        if (cost < NBG_State_cost(state, NTERM(inv))) {
          NBG_State__cost(state, NTERM(inv), cost);
          NBG_State__rule(state, NTERM(inv), 142);
          
        }
        }
      }
    
    break;
  
  case TERM_OP(ILDA):
    /* inv:ILDA(inv) */
    if (NBG_State_op(state) == TERM_OP(ILDA) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(inv)) &&
      1) {
      cost = (FAIL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(inv));
        if (cost < NBG_State_cost(state, NTERM(inv))) {
          NBG_State__cost(state, NTERM(inv), cost);
          NBG_State__rule(state, NTERM(inv), 145);
          
        }
        }
      }
    
    break;
  
  case TERM_OP(EXTRACT_BITS):
    /* reg:EXTRACT_BITS(reg) */
    if (NBG_State_op(state) == TERM_OP(EXTRACT_BITS) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      1) {
      cost = (2);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 107);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    break;
  
  case TERM_OP(ISTOREX):
    /* stmt:ISTOREX(reg,reg,reg) */
    if (NBG_State_op(state) == TERM_OP(ISTOREX) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 2), NTERM(reg)) &&
      1) {
      cost = (FAIL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 2), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(stmt))) {
          NBG_State__cost(state, NTERM(stmt), cost);
          NBG_State__rule(state, NTERM(stmt), 9);
          NBG_compute_closure(state, NTERM(stmt));
        }
        }
      }
    
    break;
  
  case TERM_OP(CONST):
    /* reg:CONST */
    if (NBG_State_op(state) == TERM_OP(CONST) &&
      1) {
      cost = (CONST_COST);
      if (cost != NBG_COST_UNDEF) {
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 26);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    break;
  
  case TERM_OP(IO_ITEM):
    /* inv:IO_ITEM(inv) */
    if (NBG_State_op(state) == TERM_OP(IO_ITEM) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(inv)) &&
      1) {
      cost = (FAIL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(inv));
        if (cost < NBG_State_cost(state, NTERM(inv))) {
          NBG_State__cost(state, NTERM(inv), cost);
          NBG_State__rule(state, NTERM(inv), 152);
          
        }
        }
      }
    
    break;
  
  case TERM_OP(BIOR):
    /* reg:BIOR(reg,rcon) */
    if (NBG_State_op(state) == TERM_OP(BIOR) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(rcon)) &&
      1) {
      cost = (ARI_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(rcon));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 54);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg:BIOR(rcon,reg) */
    if (NBG_State_op(state) == TERM_OP(BIOR) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(rcon)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(reg)) &&
      1) {
      cost = (ARI_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(rcon));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 55);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    break;
  
  case TERM_OP(ARRAYEXP):
    /* inv:ARRAYEXP(inv) */
    if (NBG_State_op(state) == TERM_OP(ARRAYEXP) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(inv)) &&
      1) {
      cost = (FAIL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(inv));
        if (cost < NBG_State_cost(state, NTERM(inv))) {
          NBG_State__cost(state, NTERM(inv), cost);
          NBG_State__rule(state, NTERM(inv), 154);
          
        }
        }
      }
    
    break;
  
  case TERM_OP(PARM):
    /* param:PARM(reg) */
    if (NBG_State_op(state) == TERM_OP(PARM) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      1) {
      cost = (FAIL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(param))) {
          NBG_State__cost(state, NTERM(param), cost);
          NBG_State__rule(state, NTERM(param), 118);
          
        }
        }
      }
    
    break;
  
  case TERM_OP(ILOADX):
    /* reg:ILOADX(reg,reg) */
    if (NBG_State_op(state) == TERM_OP(ILOADX) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(reg)) &&
      1) {
      cost = (FAIL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 19);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    break;
  
  case TERM_OP(MSTORE):
    /* stmt:MSTORE(reg,reg,reg) */
    if (NBG_State_op(state) == TERM_OP(MSTORE) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 2), NTERM(reg)) &&
      1) {
      cost = (FAIL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 2), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(stmt))) {
          NBG_State__cost(state, NTERM(stmt), cost);
          NBG_State__rule(state, NTERM(stmt), 138);
          NBG_compute_closure(state, NTERM(stmt));
        }
        }
      }
    
    break;
  
  case TERM_OP(HIGHMPY):
    /* reg:HIGHMPY(reg,reg) */
    if (NBG_State_op(state) == TERM_OP(HIGHMPY) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(reg)) &&
      1) {
      cost = (FAIL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 131);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    break;
  
  case TERM_OP(SQRT):
    /* reg:SQRT(reg) */
    if (NBG_State_op(state) == TERM_OP(SQRT) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      1) {
      cost = (FAIL_COST&&FLT_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 92);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    break;
  
  case TERM_OP(NEG):
    /* reg:NEG(reg) */
    if (NBG_State_op(state) == TERM_OP(NEG) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      1) {
      cost = (ARI_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 31);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    break;
  
  case TERM_OP(FIRSTPART):
    /* inv:FIRSTPART(inv) */
    if (NBG_State_op(state) == TERM_OP(FIRSTPART) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(inv)) &&
      1) {
      cost = (FAIL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(inv));
        if (cost < NBG_State_cost(state, NTERM(inv))) {
          NBG_State__cost(state, NTERM(inv), cost);
          NBG_State__rule(state, NTERM(inv), 140);
          
        }
        }
      }
    
    break;
  
  case TERM_OP(MSUB):
    /* reg:MSUB(reg,reg,reg) */
    if (NBG_State_op(state) == TERM_OP(MSUB) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 2), NTERM(reg)) &&
      1) {
      cost = (FAIL_COST&&MADD_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 2), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 48);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    break;
  
  case TERM_OP(XGOTO):
    /* stmt:XGOTO(reg) */
    if (NBG_State_op(state) == TERM_OP(XGOTO) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      1) {
      cost = (1);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(stmt))) {
          NBG_State__cost(state, NTERM(stmt), cost);
          NBG_State__rule(state, NTERM(stmt), 111);
          NBG_compute_closure(state, NTERM(stmt));
        }
        }
      }
    
    break;
  
  case TERM_OP(ISTBITS):
    /* stmt:ISTBITS(reg,LDA) */
    if (NBG_State_op(state) == TERM_OP(ISTBITS) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_op(NBG_State_get_child(state, 1)) == TERM_OP(LDA) &&
      1) {
      cost = (1+ST_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(stmt))) {
          NBG_State__cost(state, NTERM(stmt), cost);
          NBG_State__rule(state, NTERM(stmt), 12);
          NBG_compute_closure(state, NTERM(stmt));
        }
        }
      }
    
    /* stmt:ISTBITS(reg,reg) */
    if (NBG_State_op(state) == TERM_OP(ISTBITS) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(reg)) &&
      1) {
      cost = (1+ST_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(stmt))) {
          NBG_State__cost(state, NTERM(stmt), cost);
          NBG_State__rule(state, NTERM(stmt), 13);
          NBG_compute_closure(state, NTERM(stmt));
        }
        }
      }
    
    break;
  
  case TERM_OP(NE):
    /* cond:NE(reg,rcon) */
    if (NBG_State_op(state) == TERM_OP(NE) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(rcon)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(rcon));
        if (cost < NBG_State_cost(state, NTERM(cond))) {
          NBG_State__cost(state, NTERM(cond), cost);
          NBG_State__rule(state, NTERM(cond), 70);
          
        }
        }
      }
    
    /* cond:NE(rcon,reg) */
    if (NBG_State_op(state) == TERM_OP(NE) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(rcon)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(reg)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(rcon));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(cond))) {
          NBG_State__cost(state, NTERM(cond), cost);
          NBG_State__rule(state, NTERM(cond), 71);
          
        }
        }
      }
    
    /* reg:NE(reg,rcon) */
    if (NBG_State_op(state) == TERM_OP(NE) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(rcon)) &&
      1) {
      cost = (1);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(rcon));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 82);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg:NE(rcon,reg) */
    if (NBG_State_op(state) == TERM_OP(NE) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(rcon)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(reg)) &&
      1) {
      cost = (1);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(rcon));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 83);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    break;
  
  case TERM_OP(XMPY):
    /* reg:XMPY(reg,reg) */
    if (NBG_State_op(state) == TERM_OP(XMPY) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(reg)) &&
      1) {
      cost = (FAIL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 132);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    break;
  
  case TERM_OP(NMSUB):
    /* reg:NMSUB(reg,reg,reg) */
    if (NBG_State_op(state) == TERM_OP(NMSUB) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 2), NTERM(reg)) &&
      1) {
      cost = (FAIL_COST&&MADD_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 2), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 50);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    break;
  
  case TERM_OP(SUB):
    /* reg:SUB(reg,rcon) */
    if (NBG_State_op(state) == TERM_OP(SUB) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(rcon)) &&
      1) {
      cost = (ARI_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(rcon));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 29);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg:SUB(rcon,reg) */
    if (NBG_State_op(state) == TERM_OP(SUB) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(rcon)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(reg)) &&
      1) {
      cost = (ARI_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(rcon));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 30);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    break;
  
  case TERM_OP(LOWPART):
    /* reg:LOWPART(reg) */
    if (NBG_State_op(state) == TERM_OP(LOWPART) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      1) {
      cost = (FAIL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 126);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    break;
  
  case TERM_OP(CVT):
    /* reg:CVT(reg) */
    if (NBG_State_op(state) == TERM_OP(CVT) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      1) {
      cost = (FAIL_COST&&CVT_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 104);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    break;
  
  case TERM_OP(LDA):
    /* reg:LDA */
    if (NBG_State_op(state) == TERM_OP(LDA) &&
      1) {
      cost = (LD_COST);
      if (cost != NBG_COST_UNDEF) {
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 18);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    break;
  
  case TERM_OP(LSHR):
    /* reg:LSHR(reg,rcon) */
    if (NBG_State_op(state) == TERM_OP(LSHR) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(rcon)) &&
      1) {
      cost = (ARI_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(rcon));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 33);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* m_reg_zx16h:LSHR.U4(reg,cnsti4_16) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(LSHR) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(U4) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(cnsti4_16)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(cnsti4_16));
        if (cost < NBG_State_cost(state, NTERM(m_reg_zx16h))) {
          NBG_State__cost(state, NTERM(m_reg_zx16h), cost);
          NBG_State__rule(state, NTERM(m_reg_zx16h), 226);
          NBG_compute_closure(state, NTERM(m_reg_zx16h));
        }
        }
      }
    
    /* m_reg_zx16h:LSHR.U4(SHL(m_reg_zx16h,cnsti4_16),cnsti4_16) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(LSHR) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(U4) &&
      NBG_State_op(NBG_State_get_child(state, 0)) == TERM_OP(SHL) &&
      NBG_State_rule(NBG_State_get_child(NBG_State_get_child(state, 0), 0), NTERM(m_reg_zx16h)) &&
      NBG_State_rule(NBG_State_get_child(NBG_State_get_child(state, 0), 1), NTERM(cnsti4_16)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(cnsti4_16)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(NBG_State_get_child(state, 0), 0), NTERM(m_reg_zx16h));
        cost += NBG_State_cost(NBG_State_get_child(NBG_State_get_child(state, 0), 1), NTERM(cnsti4_16));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(cnsti4_16));
        if (cost < NBG_State_cost(state, NTERM(m_reg_zx16h))) {
          NBG_State__cost(state, NTERM(m_reg_zx16h), cost);
          NBG_State__rule(state, NTERM(m_reg_zx16h), 228);
          NBG_compute_closure(state, NTERM(m_reg_zx16h));
        }
        }
      }
    
    /* m_reg_zx16:LSHR.U4(SHL(reg_or_m_reg_zx16,cnsti4_16),cnsti4_16) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(LSHR) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(U4) &&
      NBG_State_op(NBG_State_get_child(state, 0)) == TERM_OP(SHL) &&
      NBG_State_rule(NBG_State_get_child(NBG_State_get_child(state, 0), 0), NTERM(reg_or_m_reg_zx16)) &&
      NBG_State_rule(NBG_State_get_child(NBG_State_get_child(state, 0), 1), NTERM(cnsti4_16)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(cnsti4_16)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(NBG_State_get_child(state, 0), 0), NTERM(reg_or_m_reg_zx16));
        cost += NBG_State_cost(NBG_State_get_child(NBG_State_get_child(state, 0), 1), NTERM(cnsti4_16));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(cnsti4_16));
        if (cost < NBG_State_cost(state, NTERM(m_reg_zx16))) {
          NBG_State__cost(state, NTERM(m_reg_zx16), cost);
          NBG_State__rule(state, NTERM(m_reg_zx16), 253);
          NBG_compute_closure(state, NTERM(m_reg_zx16));
        }
        }
      }
    
    /* reg_zx8:LSHR.U4(reg,cnsti4_ge24) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(LSHR) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(U4) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(cnsti4_ge24)) &&
      1) {
      cost = (ARI_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(cnsti4_ge24));
        if (cost < NBG_State_cost(state, NTERM(reg_zx8))) {
          NBG_State__cost(state, NTERM(reg_zx8), cost);
          NBG_State__rule(state, NTERM(reg_zx8), 265);
          NBG_compute_closure(state, NTERM(reg_zx8));
        }
        }
      }
    
    /* reg_zx8:LSHR.U4(reg_zx16,cnsti4_ge8) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(LSHR) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(U4) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg_zx16)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(cnsti4_ge8)) &&
      1) {
      cost = (ARI_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg_zx16));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(cnsti4_ge8));
        if (cost < NBG_State_cost(state, NTERM(reg_zx8))) {
          NBG_State__cost(state, NTERM(reg_zx8), cost);
          NBG_State__rule(state, NTERM(reg_zx8), 266);
          NBG_compute_closure(state, NTERM(reg_zx8));
        }
        }
      }
    
    /* reg_zx16:LSHR.U4(SHL(reg,cnsti4_16),cnsti4_16) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(LSHR) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(U4) &&
      NBG_State_op(NBG_State_get_child(state, 0)) == TERM_OP(SHL) &&
      NBG_State_rule(NBG_State_get_child(NBG_State_get_child(state, 0), 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(NBG_State_get_child(state, 0), 1), NTERM(cnsti4_16)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(cnsti4_16)) &&
      1) {
      cost = (CVTL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(NBG_State_get_child(state, 0), 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(NBG_State_get_child(state, 0), 1), NTERM(cnsti4_16));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(cnsti4_16));
        if (cost < NBG_State_cost(state, NTERM(reg_zx16))) {
          NBG_State__cost(state, NTERM(reg_zx16), cost);
          NBG_State__rule(state, NTERM(reg_zx16), 285);
          NBG_compute_closure(state, NTERM(reg_zx16));
        }
        }
      }
    
    /* reg_zx16:LSHR.U4(reg,cnsti4_ge16) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(LSHR) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(U4) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(cnsti4_ge16)) &&
      1) {
      cost = (ARI_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(cnsti4_ge16));
        if (cost < NBG_State_cost(state, NTERM(reg_zx16))) {
          NBG_State__cost(state, NTERM(reg_zx16), cost);
          NBG_State__rule(state, NTERM(reg_zx16), 287);
          NBG_compute_closure(state, NTERM(reg_zx16));
        }
        }
      }
    
    /* reg_zx16:LSHR.U4(SHL(reg_zx16,cnsti4_16),cnsti4_16) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(LSHR) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(U4) &&
      NBG_State_op(NBG_State_get_child(state, 0)) == TERM_OP(SHL) &&
      NBG_State_rule(NBG_State_get_child(NBG_State_get_child(state, 0), 0), NTERM(reg_zx16)) &&
      NBG_State_rule(NBG_State_get_child(NBG_State_get_child(state, 0), 1), NTERM(cnsti4_16)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(cnsti4_16)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(NBG_State_get_child(state, 0), 0), NTERM(reg_zx16));
        cost += NBG_State_cost(NBG_State_get_child(NBG_State_get_child(state, 0), 1), NTERM(cnsti4_16));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(cnsti4_16));
        if (cost < NBG_State_cost(state, NTERM(reg_zx16))) {
          NBG_State__cost(state, NTERM(reg_zx16), cost);
          NBG_State__rule(state, NTERM(reg_zx16), 289);
          NBG_compute_closure(state, NTERM(reg_zx16));
        }
        }
      }
    
    break;
  
  case TERM_OP(REM):
    /* reg:REM(reg,cnsti4) */
    if (NBG_State_op(state) == TERM_OP(REM) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(cnsti4)) &&
      1) {
      cost = (DIV_CNST_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(cnsti4));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 38);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg:REM(reg,reg) */
    if (NBG_State_op(state) == TERM_OP(REM) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(reg)) &&
      1) {
      cost = (DIV_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 40);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    break;
  
  case TERM_OP(RETURN):
    /* stmt:RETURN */
    if (NBG_State_op(state) == TERM_OP(RETURN) &&
      1) {
      cost = (1);
      if (cost != NBG_COST_UNDEF) {
        if (cost < NBG_State_cost(state, NTERM(stmt))) {
          NBG_State__cost(state, NTERM(stmt), cost);
          NBG_State__rule(state, NTERM(stmt), 123);
          NBG_compute_closure(state, NTERM(stmt));
        }
        }
      }
    
    break;
  
  case TERM_OP(LNOT):
    /* reg:LNOT(rcon) */
    if (NBG_State_op(state) == TERM_OP(LNOT) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(rcon)) &&
      1) {
      cost = (LOGIC_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(rcon));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 60);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    break;
  
  case TERM_OP(PICCALL):
    /* stmt:PICCALL.V */
    if (NBG_State_op(state) == TERM_OP(PICCALL) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(V) &&
      1) {
      cost = (FAIL_COST);
      if (cost != NBG_COST_UNDEF) {
        if (cost < NBG_State_cost(state, NTERM(stmt))) {
          NBG_State__cost(state, NTERM(stmt), cost);
          NBG_State__rule(state, NTERM(stmt), 124);
          NBG_compute_closure(state, NTERM(stmt));
        }
        }
      }
    
    /* stmt:PICCALL */
    if (NBG_State_op(state) == TERM_OP(PICCALL) &&
      1) {
      cost = (FAIL_COST);
      if (cost != NBG_COST_UNDEF) {
        if (cost < NBG_State_cost(state, NTERM(stmt))) {
          NBG_State__cost(state, NTERM(stmt), cost);
          NBG_State__rule(state, NTERM(stmt), 125);
          NBG_compute_closure(state, NTERM(stmt));
        }
        }
      }
    
    break;
  
  case TERM_OP(GE):
    /* cond:GE(reg,rcon) */
    if (NBG_State_op(state) == TERM_OP(GE) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(rcon)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(rcon));
        if (cost < NBG_State_cost(state, NTERM(cond))) {
          NBG_State__cost(state, NTERM(cond), cost);
          NBG_State__rule(state, NTERM(cond), 74);
          
        }
        }
      }
    
    /* cond:GE(rcon,reg) */
    if (NBG_State_op(state) == TERM_OP(GE) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(rcon)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(reg)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(rcon));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(cond))) {
          NBG_State__cost(state, NTERM(cond), cost);
          NBG_State__rule(state, NTERM(cond), 75);
          
        }
        }
      }
    
    /* reg:GE(reg,rcon) */
    if (NBG_State_op(state) == TERM_OP(GE) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(rcon)) &&
      1) {
      cost = (1);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(rcon));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 86);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg:GE(rcon,reg) */
    if (NBG_State_op(state) == TERM_OP(GE) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(rcon)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(reg)) &&
      1) {
      cost = (1);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(rcon));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 87);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    break;
  
  case TERM_OP(INTRINSIC_CALL):
    /* stmt:INTRINSIC_CALL.V */
    if (NBG_State_op(state) == TERM_OP(INTRINSIC_CALL) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(V) &&
      1) {
      cost = (FAIL_COST);
      if (cost != NBG_COST_UNDEF) {
        if (cost < NBG_State_cost(state, NTERM(stmt))) {
          NBG_State__cost(state, NTERM(stmt), cost);
          NBG_State__rule(state, NTERM(stmt), 159);
          NBG_compute_closure(state, NTERM(stmt));
        }
        }
      }
    
    /* reg:INTRINSIC_CALL */
    if (NBG_State_op(state) == TERM_OP(INTRINSIC_CALL) &&
      1) {
      cost = (FAIL_COST);
      if (cost != NBG_COST_UNDEF) {
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 160);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    break;
  
  case TERM_OP(ARRSECTION):
    /* inv:ARRSECTION(inv) */
    if (NBG_State_op(state) == TERM_OP(ARRSECTION) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(inv)) &&
      1) {
      cost = (FAIL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(inv));
        if (cost < NBG_State_cost(state, NTERM(inv))) {
          NBG_State__cost(state, NTERM(inv), cost);
          NBG_State__rule(state, NTERM(inv), 155);
          
        }
        }
      }
    
    break;
  
  case TERM_OP(RSQRT):
    /* reg:RSQRT(reg) */
    if (NBG_State_op(state) == TERM_OP(RSQRT) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      1) {
      cost = (FAIL_COST&&FLT_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 93);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    break;
  
  case TERM_OP(HIGHPART):
    /* reg:HIGHPART(reg) */
    if (NBG_State_op(state) == TERM_OP(HIGHPART) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      1) {
      cost = (FAIL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 127);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    break;
  
  case TERM_OP(ABS):
    /* reg:ABS(reg) */
    if (NBG_State_op(state) == TERM_OP(ABS) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      1) {
      cost = (ABS_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 42);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    break;
  
  case TERM_OP(INTCONST):
    /* cnsti4:INTCONST */
    if (NBG_State_op(state) == TERM_OP(INTCONST) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        if (cost < NBG_State_cost(state, NTERM(cnsti4))) {
          NBG_State__cost(state, NTERM(cnsti4), cost);
          NBG_State__rule(state, NTERM(cnsti4), 4);
          NBG_compute_closure(state, NTERM(cnsti4));
        }
        }
      }
    
    /* reg:INTCONST#0 */
    if (NBG_State_op(state) == TERM_OP(INTCONST) &&
      NBG_TREE_VALUE_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_VALUE(0) &&
      1) {
      cost = (INTCONST_COST);
      if (cost != NBG_COST_UNDEF) {
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 24);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg:INTCONST */
    if (NBG_State_op(state) == TERM_OP(INTCONST) &&
      1) {
      cost = (INTCONST_COST);
      if (cost != NBG_COST_UNDEF) {
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 25);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* cnsti4_ge8:INTCONST.I4 */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(INTCONST) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(I4) &&
      1) {
      cost = (CHECK_COST(WN_const_val(STATE_WN) >= 8, 0));
      if (cost != NBG_COST_UNDEF) {
        if (cost < NBG_State_cost(state, NTERM(cnsti4_ge8))) {
          NBG_State__cost(state, NTERM(cnsti4_ge8), cost);
          NBG_State__rule(state, NTERM(cnsti4_ge8), 162);
          
        }
        }
      }
    
    /* cnsti4_ge16:INTCONST.I4 */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(INTCONST) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(I4) &&
      1) {
      cost = (CHECK_COST(WN_const_val(STATE_WN) >= 16, 0));
      if (cost != NBG_COST_UNDEF) {
        if (cost < NBG_State_cost(state, NTERM(cnsti4_ge16))) {
          NBG_State__cost(state, NTERM(cnsti4_ge16), cost);
          NBG_State__rule(state, NTERM(cnsti4_ge16), 163);
          
        }
        }
      }
    
    /* cnsti4_ge24:INTCONST.I4 */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(INTCONST) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(I4) &&
      1) {
      cost = (CHECK_COST(WN_const_val(STATE_WN) >= 24, 0));
      if (cost != NBG_COST_UNDEF) {
        if (cost < NBG_State_cost(state, NTERM(cnsti4_ge24))) {
          NBG_State__cost(state, NTERM(cnsti4_ge24), cost);
          NBG_State__rule(state, NTERM(cnsti4_ge24), 164);
          
        }
        }
      }
    
    /* cnsti4_sx8:INTCONST.I4 */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(INTCONST) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(I4) &&
      1) {
      cost = (CHECK_COST(WN_const_val(STATE_WN) < (1<<7) && WN_const_val(STATE_WN) >= -(1<<7) , 0));
      if (cost != NBG_COST_UNDEF) {
        if (cost < NBG_State_cost(state, NTERM(cnsti4_sx8))) {
          NBG_State__cost(state, NTERM(cnsti4_sx8), cost);
          NBG_State__rule(state, NTERM(cnsti4_sx8), 165);
          NBG_compute_closure(state, NTERM(cnsti4_sx8));
        }
        }
      }
    
    /* cnsti4_sx16:INTCONST.I4 */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(INTCONST) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(I4) &&
      1) {
      cost = (CHECK_COST(WN_const_val(STATE_WN) < (1<<15) && WN_const_val(STATE_WN) >= -(1<<15) , 0));
      if (cost != NBG_COST_UNDEF) {
        if (cost < NBG_State_cost(state, NTERM(cnsti4_sx16))) {
          NBG_State__cost(state, NTERM(cnsti4_sx16), cost);
          NBG_State__rule(state, NTERM(cnsti4_sx16), 166);
          NBG_compute_closure(state, NTERM(cnsti4_sx16));
        }
        }
      }
    
    /* cnsti4_zx8:INTCONST.I4 */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(INTCONST) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(I4) &&
      1) {
      cost = (CHECK_COST(WN_const_val(STATE_WN) < (1<<8) && WN_const_val(STATE_WN) >= 0 , 0));
      if (cost != NBG_COST_UNDEF) {
        if (cost < NBG_State_cost(state, NTERM(cnsti4_zx8))) {
          NBG_State__cost(state, NTERM(cnsti4_zx8), cost);
          NBG_State__rule(state, NTERM(cnsti4_zx8), 168);
          NBG_compute_closure(state, NTERM(cnsti4_zx8));
        }
        }
      }
    
    /* cnsti4_zx16:INTCONST.I4 */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(INTCONST) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(I4) &&
      1) {
      cost = (CHECK_COST(WN_const_val(STATE_WN) < (1<<16) && WN_const_val(STATE_WN) >= 0 , 0));
      if (cost != NBG_COST_UNDEF) {
        if (cost < NBG_State_cost(state, NTERM(cnsti4_zx16))) {
          NBG_State__cost(state, NTERM(cnsti4_zx16), cost);
          NBG_State__rule(state, NTERM(cnsti4_zx16), 169);
          NBG_compute_closure(state, NTERM(cnsti4_zx16));
        }
        }
      }
    
    /* cnsti4_1:INTCONST.I4#1 */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(INTCONST) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(I4) &&
      NBG_TREE_VALUE_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_VALUE(1) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        if (cost < NBG_State_cost(state, NTERM(cnsti4_1))) {
          NBG_State__cost(state, NTERM(cnsti4_1), cost);
          NBG_State__rule(state, NTERM(cnsti4_1), 171);
          
        }
        }
      }
    
    /* cnsti4_2:INTCONST.I4#2 */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(INTCONST) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(I4) &&
      NBG_TREE_VALUE_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_VALUE(2) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        if (cost < NBG_State_cost(state, NTERM(cnsti4_2))) {
          NBG_State__cost(state, NTERM(cnsti4_2), cost);
          NBG_State__rule(state, NTERM(cnsti4_2), 172);
          
        }
        }
      }
    
    /* cnsti4_3:INTCONST.I4#3 */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(INTCONST) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(I4) &&
      NBG_TREE_VALUE_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_VALUE(3) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        if (cost < NBG_State_cost(state, NTERM(cnsti4_3))) {
          NBG_State__cost(state, NTERM(cnsti4_3), cost);
          NBG_State__rule(state, NTERM(cnsti4_3), 173);
          
        }
        }
      }
    
    /* cnsti4_4:INTCONST.I4#4 */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(INTCONST) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(I4) &&
      NBG_TREE_VALUE_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_VALUE(4) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        if (cost < NBG_State_cost(state, NTERM(cnsti4_4))) {
          NBG_State__cost(state, NTERM(cnsti4_4), cost);
          NBG_State__rule(state, NTERM(cnsti4_4), 174);
          
        }
        }
      }
    
    /* cnsti4_8:INTCONST.I4#8 */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(INTCONST) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(I4) &&
      NBG_TREE_VALUE_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_VALUE(8) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        if (cost < NBG_State_cost(state, NTERM(cnsti4_8))) {
          NBG_State__cost(state, NTERM(cnsti4_8), cost);
          NBG_State__rule(state, NTERM(cnsti4_8), 175);
          
        }
        }
      }
    
    /* cnsti4_16:INTCONST.I4#16 */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(INTCONST) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(I4) &&
      NBG_TREE_VALUE_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_VALUE(16) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        if (cost < NBG_State_cost(state, NTERM(cnsti4_16))) {
          NBG_State__cost(state, NTERM(cnsti4_16), cost);
          NBG_State__rule(state, NTERM(cnsti4_16), 176);
          
        }
        }
      }
    
    /* cnsti4_0xff:INTCONST.I4#0xff */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(INTCONST) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(I4) &&
      NBG_TREE_VALUE_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_VALUE(0xff) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        if (cost < NBG_State_cost(state, NTERM(cnsti4_0xff))) {
          NBG_State__cost(state, NTERM(cnsti4_0xff), cost);
          NBG_State__rule(state, NTERM(cnsti4_0xff), 177);
          
        }
        }
      }
    
    /* cnsti4_0xffff:INTCONST.I4#0xffff */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(INTCONST) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(I4) &&
      NBG_TREE_VALUE_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_VALUE(0xffff) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        if (cost < NBG_State_cost(state, NTERM(cnsti4_0xffff))) {
          NBG_State__cost(state, NTERM(cnsti4_0xffff), cost);
          NBG_State__rule(state, NTERM(cnsti4_0xffff), 178);
          
        }
        }
      }
    
    /* cnstpw2u:INTCONST */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(INTCONST) &&
      1) {
      cost = (CHECK_COST(IS_CNSTPW2U(STATE_WN), 0));
      if (cost != NBG_COST_UNDEF) {
        if (cost < NBG_State_cost(state, NTERM(cnstpw2u))) {
          NBG_State__cost(state, NTERM(cnstpw2u), cost);
          NBG_State__rule(state, NTERM(cnstpw2u), 179);
          
        }
        }
      }
    
    /* cnstpw2s:INTCONST */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(INTCONST) &&
      1) {
      cost = (CHECK_COST(IS_CNSTPW2S(STATE_WN), 0));
      if (cost != NBG_COST_UNDEF) {
        if (cost < NBG_State_cost(state, NTERM(cnstpw2s))) {
          NBG_State__cost(state, NTERM(cnstpw2s), cost);
          NBG_State__rule(state, NTERM(cnstpw2s), 180);
          
        }
        }
      }
    
    break;
  
  case TERM_OP(LIOR):
    /* reg:LIOR(reg,rcon) */
    if (NBG_State_op(state) == TERM_OP(LIOR) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(rcon)) &&
      1) {
      cost = (LOGIC_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(rcon));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 63);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg:LIOR(rcon,reg) */
    if (NBG_State_op(state) == TERM_OP(LIOR) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(rcon)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(reg)) &&
      1) {
      cost = (LOGIC_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(rcon));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 64);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    break;
  
  case TERM_OP(SECONDPART):
    /* inv:SECONDPART(inv) */
    if (NBG_State_op(state) == TERM_OP(SECONDPART) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(inv)) &&
      1) {
      cost = (FAIL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(inv));
        if (cost < NBG_State_cost(state, NTERM(inv))) {
          NBG_State__cost(state, NTERM(inv), cost);
          NBG_State__rule(state, NTERM(inv), 141);
          
        }
        }
      }
    
    break;
  
  case TERM_OP(GT):
    /* cond:GT(reg,rcon) */
    if (NBG_State_op(state) == TERM_OP(GT) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(rcon)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(rcon));
        if (cost < NBG_State_cost(state, NTERM(cond))) {
          NBG_State__cost(state, NTERM(cond), cost);
          NBG_State__rule(state, NTERM(cond), 72);
          
        }
        }
      }
    
    /* cond:GT(rcon,rcon) */
    if (NBG_State_op(state) == TERM_OP(GT) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(rcon)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(rcon)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(rcon));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(rcon));
        if (cost < NBG_State_cost(state, NTERM(cond))) {
          NBG_State__cost(state, NTERM(cond), cost);
          NBG_State__rule(state, NTERM(cond), 73);
          
        }
        }
      }
    
    /* reg:GT(reg,rcon) */
    if (NBG_State_op(state) == TERM_OP(GT) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(rcon)) &&
      1) {
      cost = (1);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(rcon));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 84);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg:GT(rcon,rcon) */
    if (NBG_State_op(state) == TERM_OP(GT) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(rcon)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(rcon)) &&
      1) {
      cost = (1);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(rcon));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(rcon));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 85);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    break;
  
  case TERM_OP(MPY):
    /* reg:MPY(reg,rcon) */
    if (NBG_State_op(state) == TERM_OP(MPY) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(rcon)) &&
      1) {
      cost = (MUL32_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(rcon));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 35);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg:MPY(rcon,reg) */
    if (NBG_State_op(state) == TERM_OP(MPY) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(rcon)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(reg)) &&
      1) {
      cost = (MUL32_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(rcon));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 36);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg:MPY(reg,cnstpw2u) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(MPY) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(cnstpw2u)) &&
      1) {
      cost = (1);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(cnstpw2u));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 181);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg:MPY(cnstpw2u,reg) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(MPY) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(cnstpw2u)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(reg)) &&
      1) {
      cost = (1);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(cnstpw2u));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 182);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg:MPY(reg,cnstpw2s) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(MPY) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(cnstpw2s)) &&
      1) {
      cost = (2);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(cnstpw2s));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 183);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg:MPY(cnstpw2s,reg) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(MPY) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(cnstpw2s)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(reg)) &&
      1) {
      cost = (2);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(cnstpw2s));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 184);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* m_reg_shl_1:MPY(reg,cnsti4_2) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(MPY) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(cnsti4_2)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(cnsti4_2));
        if (cost < NBG_State_cost(state, NTERM(m_reg_shl_1))) {
          NBG_State__cost(state, NTERM(m_reg_shl_1), cost);
          NBG_State__rule(state, NTERM(m_reg_shl_1), 186);
          
        }
        }
      }
    
    /* m_reg_shl_2:MPY(reg,cnsti4_4) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(MPY) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(cnsti4_4)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(cnsti4_4));
        if (cost < NBG_State_cost(state, NTERM(m_reg_shl_2))) {
          NBG_State__cost(state, NTERM(m_reg_shl_2), cost);
          NBG_State__rule(state, NTERM(m_reg_shl_2), 188);
          
        }
        }
      }
    
    /* m_reg_shl_3:MPY(reg,cnsti4_8) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(MPY) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(cnsti4_8)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(cnsti4_8));
        if (cost < NBG_State_cost(state, NTERM(m_reg_shl_3))) {
          NBG_State__cost(state, NTERM(m_reg_shl_3), cost);
          NBG_State__rule(state, NTERM(m_reg_shl_3), 190);
          
        }
        }
      }
    
    /* m_reg_shl_3:MPY(reg,cnsti4_16) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(MPY) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(cnsti4_16)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(cnsti4_16));
        if (cost < NBG_State_cost(state, NTERM(m_reg_shl_3))) {
          NBG_State__cost(state, NTERM(m_reg_shl_3), cost);
          NBG_State__rule(state, NTERM(m_reg_shl_3), 192);
          
        }
        }
      }
    
    /* reg:MPY(m_reg_sx16h,m_reg_sx16h) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(MPY) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(m_reg_sx16h)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(m_reg_sx16h)) &&
      1) {
      cost = (MUL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(m_reg_sx16h));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(m_reg_sx16h));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 199);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg:MPY(m_reg_zx16h,m_reg_zx16h) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(MPY) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(m_reg_zx16h)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(m_reg_zx16h)) &&
      1) {
      cost = (MUL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(m_reg_zx16h));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(m_reg_zx16h));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 200);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg:MPY(m_reg_sx16,m_reg_sx16h) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(MPY) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(m_reg_sx16)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(m_reg_sx16h)) &&
      1) {
      cost = (MUL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(m_reg_sx16));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(m_reg_sx16h));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 201);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg:MPY(m_reg_sx16h,m_reg_sx16) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(MPY) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(m_reg_sx16h)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(m_reg_sx16)) &&
      1) {
      cost = (MUL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(m_reg_sx16h));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(m_reg_sx16));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 202);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg:MPY(m_reg_zx16,m_reg_zx16h) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(MPY) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(m_reg_zx16)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(m_reg_zx16h)) &&
      1) {
      cost = (MUL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(m_reg_zx16));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(m_reg_zx16h));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 203);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg:MPY(m_reg_zx16h,m_reg_zx16) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(MPY) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(m_reg_zx16h)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(m_reg_zx16)) &&
      1) {
      cost = (MUL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(m_reg_zx16h));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(m_reg_zx16));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 204);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg:MPY(reg,m_reg_sx16h) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(MPY) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(m_reg_sx16h)) &&
      1) {
      cost = (MUL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(m_reg_sx16h));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 205);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg:MPY(m_reg_sx16h,reg) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(MPY) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(m_reg_sx16h)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(reg)) &&
      1) {
      cost = (MUL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(m_reg_sx16h));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 206);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg:MPY(reg,m_reg_zx16h) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(MPY) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(m_reg_zx16h)) &&
      1) {
      cost = (MUL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(m_reg_zx16h));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 207);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg:MPY(m_reg_zx16h,reg) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(MPY) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(m_reg_zx16h)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(reg)) &&
      1) {
      cost = (MUL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(m_reg_zx16h));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 208);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg:MPY(m_reg_sx16,m_reg_sx16) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(MPY) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(m_reg_sx16)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(m_reg_sx16)) &&
      1) {
      cost = (MUL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(m_reg_sx16));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(m_reg_sx16));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 209);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg:MPY(m_reg_sx16,cnsti4_sx16) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(MPY) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(m_reg_sx16)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(cnsti4_sx16)) &&
      1) {
      cost = (MUL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(m_reg_sx16));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(cnsti4_sx16));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 210);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg:MPY(cnsti4_sx16,m_reg_sx16) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(MPY) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(cnsti4_sx16)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(m_reg_sx16)) &&
      1) {
      cost = (MUL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(cnsti4_sx16));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(m_reg_sx16));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 211);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg:MPY(m_reg_zx16,m_reg_zx16) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(MPY) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(m_reg_zx16)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(m_reg_zx16)) &&
      1) {
      cost = (MUL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(m_reg_zx16));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(m_reg_zx16));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 212);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg:MPY(m_reg_zx16,cnsti4_zx16) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(MPY) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(m_reg_zx16)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(cnsti4_zx16)) &&
      1) {
      cost = (MUL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(m_reg_zx16));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(cnsti4_zx16));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 213);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg:MPY(cnsti4_zx16,m_reg_zx16) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(MPY) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(cnsti4_zx16)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(m_reg_zx16)) &&
      1) {
      cost = (MUL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(cnsti4_zx16));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(m_reg_zx16));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 214);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg:MPY(reg,m_reg_sx16) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(MPY) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(m_reg_sx16)) &&
      1) {
      cost = (MUL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(m_reg_sx16));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 215);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg:MPY(reg,cnsti4_sx16) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(MPY) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(cnsti4_sx16)) &&
      1) {
      cost = (MUL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(cnsti4_sx16));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 216);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg:MPY(m_reg_sx16,reg) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(MPY) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(m_reg_sx16)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(reg)) &&
      1) {
      cost = (MUL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(m_reg_sx16));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 217);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg:MPY(cnsti4_sx16,reg) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(MPY) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(cnsti4_sx16)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(reg)) &&
      1) {
      cost = (MUL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(cnsti4_sx16));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 218);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg:MPY(reg,m_reg_zx16) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(MPY) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(m_reg_zx16)) &&
      1) {
      cost = (MUL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(m_reg_zx16));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 219);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg:MPY(reg,cnsti4_zx16) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(MPY) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(cnsti4_zx16)) &&
      1) {
      cost = (MUL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(cnsti4_zx16));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 220);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg:MPY(m_reg_zx16,reg) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(MPY) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(m_reg_zx16)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(reg)) &&
      1) {
      cost = (MUL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(m_reg_zx16));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 221);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg:MPY(cnsti4_zx16,reg) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(MPY) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(cnsti4_zx16)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(reg)) &&
      1) {
      cost = (MUL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(cnsti4_zx16));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 222);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    break;
  
  case TERM_OP(ASHR):
    /* reg:ASHR(reg,rcon) */
    if (NBG_State_op(state) == TERM_OP(ASHR) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(rcon)) &&
      1) {
      cost = (ARI_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(rcon));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 32);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* m_reg_sx16h:ASHR.I4(reg,cnsti4_16) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(ASHR) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(I4) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(cnsti4_16)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(cnsti4_16));
        if (cost < NBG_State_cost(state, NTERM(m_reg_sx16h))) {
          NBG_State__cost(state, NTERM(m_reg_sx16h), cost);
          NBG_State__rule(state, NTERM(m_reg_sx16h), 223);
          NBG_compute_closure(state, NTERM(m_reg_sx16h));
        }
        }
      }
    
    /* m_reg_sx16h:ASHR.I4(SHL(m_reg_sx16h,cnsti4_16),cnsti4_16) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(ASHR) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(I4) &&
      NBG_State_op(NBG_State_get_child(state, 0)) == TERM_OP(SHL) &&
      NBG_State_rule(NBG_State_get_child(NBG_State_get_child(state, 0), 0), NTERM(m_reg_sx16h)) &&
      NBG_State_rule(NBG_State_get_child(NBG_State_get_child(state, 0), 1), NTERM(cnsti4_16)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(cnsti4_16)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(NBG_State_get_child(state, 0), 0), NTERM(m_reg_sx16h));
        cost += NBG_State_cost(NBG_State_get_child(NBG_State_get_child(state, 0), 1), NTERM(cnsti4_16));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(cnsti4_16));
        if (cost < NBG_State_cost(state, NTERM(m_reg_sx16h))) {
          NBG_State__cost(state, NTERM(m_reg_sx16h), cost);
          NBG_State__rule(state, NTERM(m_reg_sx16h), 225);
          NBG_compute_closure(state, NTERM(m_reg_sx16h));
        }
        }
      }
    
    /* m_reg_sx16:ASHR.I4(SHL(reg_or_m_reg_sx16,cnsti4_16),cnsti4_16) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(ASHR) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(I4) &&
      NBG_State_op(NBG_State_get_child(state, 0)) == TERM_OP(SHL) &&
      NBG_State_rule(NBG_State_get_child(NBG_State_get_child(state, 0), 0), NTERM(reg_or_m_reg_sx16)) &&
      NBG_State_rule(NBG_State_get_child(NBG_State_get_child(state, 0), 1), NTERM(cnsti4_16)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(cnsti4_16)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(NBG_State_get_child(state, 0), 0), NTERM(reg_or_m_reg_sx16));
        cost += NBG_State_cost(NBG_State_get_child(NBG_State_get_child(state, 0), 1), NTERM(cnsti4_16));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(cnsti4_16));
        if (cost < NBG_State_cost(state, NTERM(m_reg_sx16))) {
          NBG_State__cost(state, NTERM(m_reg_sx16), cost);
          NBG_State__rule(state, NTERM(m_reg_sx16), 246);
          NBG_compute_closure(state, NTERM(m_reg_sx16));
        }
        }
      }
    
    /* reg_sx8:ASHR.I4(reg,cnsti4_ge24) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(ASHR) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(I4) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(cnsti4_ge24)) &&
      1) {
      cost = (ARI_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(cnsti4_ge24));
        if (cost < NBG_State_cost(state, NTERM(reg_sx8))) {
          NBG_State__cost(state, NTERM(reg_sx8), cost);
          NBG_State__rule(state, NTERM(reg_sx8), 257);
          NBG_compute_closure(state, NTERM(reg_sx8));
        }
        }
      }
    
    /* reg_sx8:ASHR.I4(reg_sx16,cnsti4_ge8) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(ASHR) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(I4) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg_sx16)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(cnsti4_ge8)) &&
      1) {
      cost = (ARI_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg_sx16));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(cnsti4_ge8));
        if (cost < NBG_State_cost(state, NTERM(reg_sx8))) {
          NBG_State__cost(state, NTERM(reg_sx8), cost);
          NBG_State__rule(state, NTERM(reg_sx8), 258);
          NBG_compute_closure(state, NTERM(reg_sx8));
        }
        }
      }
    
    /* reg_zx8:ASHR.I4(reg_zx16,cnsti4_ge8) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(ASHR) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(I4) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg_zx16)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(cnsti4_ge8)) &&
      1) {
      cost = (ARI_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg_zx16));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(cnsti4_ge8));
        if (cost < NBG_State_cost(state, NTERM(reg_zx8))) {
          NBG_State__cost(state, NTERM(reg_zx8), cost);
          NBG_State__rule(state, NTERM(reg_zx8), 267);
          NBG_compute_closure(state, NTERM(reg_zx8));
        }
        }
      }
    
    /* reg_sx16:ASHR.I4(SHL(reg,cnsti4_16),cnsti4_16) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(ASHR) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(I4) &&
      NBG_State_op(NBG_State_get_child(state, 0)) == TERM_OP(SHL) &&
      NBG_State_rule(NBG_State_get_child(NBG_State_get_child(state, 0), 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(NBG_State_get_child(state, 0), 1), NTERM(cnsti4_16)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(cnsti4_16)) &&
      1) {
      cost = (CVTL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(NBG_State_get_child(state, 0), 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(NBG_State_get_child(state, 0), 1), NTERM(cnsti4_16));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(cnsti4_16));
        if (cost < NBG_State_cost(state, NTERM(reg_sx16))) {
          NBG_State__cost(state, NTERM(reg_sx16), cost);
          NBG_State__rule(state, NTERM(reg_sx16), 277);
          NBG_compute_closure(state, NTERM(reg_sx16));
        }
        }
      }
    
    /* reg_sx16:ASHR.I4(reg,cnsti4_ge16) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(ASHR) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(I4) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(cnsti4_ge16)) &&
      1) {
      cost = (ARI_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(cnsti4_ge16));
        if (cost < NBG_State_cost(state, NTERM(reg_sx16))) {
          NBG_State__cost(state, NTERM(reg_sx16), cost);
          NBG_State__rule(state, NTERM(reg_sx16), 278);
          NBG_compute_closure(state, NTERM(reg_sx16));
        }
        }
      }
    
    /* reg_sx16:ASHR.I4(SHL(reg_sx16,cnsti4_16),cnsti4_16) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(ASHR) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(I4) &&
      NBG_State_op(NBG_State_get_child(state, 0)) == TERM_OP(SHL) &&
      NBG_State_rule(NBG_State_get_child(NBG_State_get_child(state, 0), 0), NTERM(reg_sx16)) &&
      NBG_State_rule(NBG_State_get_child(NBG_State_get_child(state, 0), 1), NTERM(cnsti4_16)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(cnsti4_16)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(NBG_State_get_child(state, 0), 0), NTERM(reg_sx16));
        cost += NBG_State_cost(NBG_State_get_child(NBG_State_get_child(state, 0), 1), NTERM(cnsti4_16));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(cnsti4_16));
        if (cost < NBG_State_cost(state, NTERM(reg_sx16))) {
          NBG_State__cost(state, NTERM(reg_sx16), cost);
          NBG_State__rule(state, NTERM(reg_sx16), 280);
          NBG_compute_closure(state, NTERM(reg_sx16));
        }
        }
      }
    
    break;
  
  case TERM_OP(MAX):
    /* reg:MAX(reg,rcon) */
    if (NBG_State_op(state) == TERM_OP(MAX) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(rcon)) &&
      1) {
      cost = (MMAX_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(rcon));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 43);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg:MAX(rcon,reg) */
    if (NBG_State_op(state) == TERM_OP(MAX) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(rcon)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(reg)) &&
      1) {
      cost = (MMAX_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(rcon));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 44);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    break;
  
  case TERM_OP(INTRINSIC_OP):
    /* inv:INTRINSIC_OP(inv) */
    if (NBG_State_op(state) == TERM_OP(INTRINSIC_OP) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(inv)) &&
      1) {
      cost = (FAIL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(inv));
        if (cost < NBG_State_cost(state, NTERM(inv))) {
          NBG_State__cost(state, NTERM(inv), cost);
          NBG_State__rule(state, NTERM(inv), 151);
          
        }
        }
      }
    
    break;
  
  case TERM_OP(MIN):
    /* reg:MIN(reg,rcon) */
    if (NBG_State_op(state) == TERM_OP(MIN) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(rcon)) &&
      1) {
      cost = (MMAX_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(rcon));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 45);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg:MIN(rcon,reg) */
    if (NBG_State_op(state) == TERM_OP(MIN) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(rcon)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(reg)) &&
      1) {
      cost = (MMAX_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(rcon));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 46);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    break;
  
  case TERM_OP(IDNAME):
    /* stmt:IDNAME */
    if (NBG_State_op(state) == TERM_OP(IDNAME) &&
      1) {
      cost = (FAIL_COST);
      if (cost != NBG_COST_UNDEF) {
        if (cost < NBG_State_cost(state, NTERM(stmt))) {
          NBG_State__cost(state, NTERM(stmt), cost);
          NBG_State__rule(state, NTERM(stmt), 137);
          NBG_compute_closure(state, NTERM(stmt));
        }
        }
      }
    
    break;
  
  case TERM_OP(SELECT):
    /* reg:SELECT(cond,reg,rcon) */
    if (NBG_State_op(state) == TERM_OP(SELECT) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(cond)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 2), NTERM(rcon)) &&
      1) {
      cost = (SELECT_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(cond));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 2), NTERM(rcon));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 65);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg:SELECT(cond,rcon,reg) */
    if (NBG_State_op(state) == TERM_OP(SELECT) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(cond)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(rcon)) &&
      NBG_State_rule(NBG_State_get_child(state, 2), NTERM(reg)) &&
      1) {
      cost = (SELECT_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(cond));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(rcon));
        cost += NBG_State_cost(NBG_State_get_child(state, 2), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 66);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg:SELECT(reg,reg,reg) */
    if (NBG_State_op(state) == TERM_OP(SELECT) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 2), NTERM(reg)) &&
      1) {
      cost = (SELECT_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 2), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 67);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    break;
  
  case TERM_OP(BAND):
    /* reg:BAND(reg,rcon) */
    if (NBG_State_op(state) == TERM_OP(BAND) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(rcon)) &&
      1) {
      cost = (ARI_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(rcon));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 52);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg:BAND(rcon,reg) */
    if (NBG_State_op(state) == TERM_OP(BAND) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(rcon)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(reg)) &&
      1) {
      cost = (ARI_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(rcon));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 53);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* m_reg_zx16h:BAND.I4(m_reg_zx16h,cnsti4_0xffff) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(BAND) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(I4) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(m_reg_zx16h)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(cnsti4_0xffff)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(m_reg_zx16h));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(cnsti4_0xffff));
        if (cost < NBG_State_cost(state, NTERM(m_reg_zx16h))) {
          NBG_State__cost(state, NTERM(m_reg_zx16h), cost);
          NBG_State__rule(state, NTERM(m_reg_zx16h), 229);
          NBG_compute_closure(state, NTERM(m_reg_zx16h));
        }
        }
      }
    
    /* m_reg_zx16h:BAND.U4(m_reg_zx16h,cnsti4_0xffff) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(BAND) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(U4) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(m_reg_zx16h)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(cnsti4_0xffff)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(m_reg_zx16h));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(cnsti4_0xffff));
        if (cost < NBG_State_cost(state, NTERM(m_reg_zx16h))) {
          NBG_State__cost(state, NTERM(m_reg_zx16h), cost);
          NBG_State__rule(state, NTERM(m_reg_zx16h), 230);
          NBG_compute_closure(state, NTERM(m_reg_zx16h));
        }
        }
      }
    
    /* m_reg_zx8:BAND.(reg_or_m_reg_zx8,cnsti4_0xff) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(BAND) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg_or_m_reg_zx8)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(cnsti4_0xff)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg_or_m_reg_zx8));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(cnsti4_0xff));
        if (cost < NBG_State_cost(state, NTERM(m_reg_zx8))) {
          NBG_State__cost(state, NTERM(m_reg_zx8), cost);
          NBG_State__rule(state, NTERM(m_reg_zx8), 239);
          NBG_compute_closure(state, NTERM(m_reg_zx8));
        }
        }
      }
    
    /* m_reg_zx16:BAND.(reg_or_m_reg_zx16,cnsti4_0xffff) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(BAND) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg_or_m_reg_zx16)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(cnsti4_0xffff)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg_or_m_reg_zx16));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(cnsti4_0xffff));
        if (cost < NBG_State_cost(state, NTERM(m_reg_zx16))) {
          NBG_State__cost(state, NTERM(m_reg_zx16), cost);
          NBG_State__rule(state, NTERM(m_reg_zx16), 254);
          NBG_compute_closure(state, NTERM(m_reg_zx16));
        }
        }
      }
    
    /* reg_zx8:BAND(reg,cnsti4_0xff) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(BAND) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(cnsti4_0xff)) &&
      1) {
      cost = (CVTL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(cnsti4_0xff));
        if (cost < NBG_State_cost(state, NTERM(reg_zx8))) {
          NBG_State__cost(state, NTERM(reg_zx8), cost);
          NBG_State__rule(state, NTERM(reg_zx8), 264);
          NBG_compute_closure(state, NTERM(reg_zx8));
        }
        }
      }
    
    /* reg_zx8:BAND(reg_zx8,cnsti4_0xff) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(BAND) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg_zx8)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(cnsti4_0xff)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg_zx8));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(cnsti4_0xff));
        if (cost < NBG_State_cost(state, NTERM(reg_zx8))) {
          NBG_State__cost(state, NTERM(reg_zx8), cost);
          NBG_State__rule(state, NTERM(reg_zx8), 271);
          NBG_compute_closure(state, NTERM(reg_zx8));
        }
        }
      }
    
    /* reg_zx16:BAND(reg,cnsti4_0xffff) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(BAND) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(cnsti4_0xffff)) &&
      1) {
      cost = (CVTL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(cnsti4_0xffff));
        if (cost < NBG_State_cost(state, NTERM(reg_zx16))) {
          NBG_State__cost(state, NTERM(reg_zx16), cost);
          NBG_State__rule(state, NTERM(reg_zx16), 286);
          NBG_compute_closure(state, NTERM(reg_zx16));
        }
        }
      }
    
    /* reg_zx16:BAND(reg_zx16,cnsti4_0xffff) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(BAND) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg_zx16)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(cnsti4_0xffff)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg_zx16));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(cnsti4_0xffff));
        if (cost < NBG_State_cost(state, NTERM(reg_zx16))) {
          NBG_State__cost(state, NTERM(reg_zx16), cost);
          NBG_State__rule(state, NTERM(reg_zx16), 290);
          NBG_compute_closure(state, NTERM(reg_zx16));
        }
        }
      }
    
    break;
  
  case TERM_OP(TRIPLET):
    /* inv:TRIPLET(inv) */
    if (NBG_State_op(state) == TERM_OP(TRIPLET) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(inv)) &&
      1) {
      cost = (FAIL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(inv));
        if (cost < NBG_State_cost(state, NTERM(inv))) {
          NBG_State__cost(state, NTERM(inv), cost);
          NBG_State__rule(state, NTERM(inv), 153);
          
        }
        }
      }
    
    break;
  
  case TERM_OP(COMMA):
    /* inv:COMMA(inv,inv) */
    if (NBG_State_op(state) == TERM_OP(COMMA) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(inv)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(inv)) &&
      1) {
      cost = (FAIL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(inv));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(inv));
        if (cost < NBG_State_cost(state, NTERM(inv))) {
          NBG_State__cost(state, NTERM(inv), cost);
          NBG_State__rule(state, NTERM(inv), 148);
          
        }
        }
      }
    
    break;
  
  case TERM_OP(DIVREM):
    /* reg:DIVREM(reg,reg) */
    if (NBG_State_op(state) == TERM_OP(DIVREM) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(reg)) &&
      1) {
      cost = (FAIL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 133);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    break;
  
  case TERM_OP(ILOAD):
    /* reg:ILOAD(LDA) */
    if (NBG_State_op(state) == TERM_OP(ILOAD) &&
      NBG_State_op(NBG_State_get_child(state, 0)) == TERM_OP(LDA) &&
      1) {
      cost = (LD_COST);
      if (cost != NBG_COST_UNDEF) {
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 16);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg:ILOAD(reg) */
    if (NBG_State_op(state) == TERM_OP(ILOAD) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      1) {
      cost = (LD_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 17);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg_zx16:ILOAD.U4.U2(LDA) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(ILOAD) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(U4) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 1) == TERM_PARAM(U2) &&
      NBG_State_op(NBG_State_get_child(state, 0)) == TERM_OP(LDA) &&
      1) {
      cost = (LD_COST);
      if (cost != NBG_COST_UNDEF) {
        if (cost < NBG_State_cost(state, NTERM(reg_zx16))) {
          NBG_State__cost(state, NTERM(reg_zx16), cost);
          NBG_State__rule(state, NTERM(reg_zx16), 294);
          NBG_compute_closure(state, NTERM(reg_zx16));
        }
        }
      }
    
    /* reg_zx16:ILOAD.U4.U2(reg) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(ILOAD) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(U4) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 1) == TERM_PARAM(U2) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      1) {
      cost = (LD_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg_zx16))) {
          NBG_State__cost(state, NTERM(reg_zx16), cost);
          NBG_State__rule(state, NTERM(reg_zx16), 295);
          NBG_compute_closure(state, NTERM(reg_zx16));
        }
        }
      }
    
    /* reg_sx16:ILOAD.I4.I1(LDA) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(ILOAD) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(I4) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 1) == TERM_PARAM(I1) &&
      NBG_State_op(NBG_State_get_child(state, 0)) == TERM_OP(LDA) &&
      1) {
      cost = (LD_COST);
      if (cost != NBG_COST_UNDEF) {
        if (cost < NBG_State_cost(state, NTERM(reg_sx16))) {
          NBG_State__cost(state, NTERM(reg_sx16), cost);
          NBG_State__rule(state, NTERM(reg_sx16), 298);
          NBG_compute_closure(state, NTERM(reg_sx16));
        }
        }
      }
    
    /* reg_sx16:ILOAD.I4.I1(reg) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(ILOAD) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(I4) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 1) == TERM_PARAM(I1) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      1) {
      cost = (LD_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg_sx16))) {
          NBG_State__cost(state, NTERM(reg_sx16), cost);
          NBG_State__rule(state, NTERM(reg_sx16), 299);
          NBG_compute_closure(state, NTERM(reg_sx16));
        }
        }
      }
    
    /* reg_zx8:ILOAD.U4.U1(LDA) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(ILOAD) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(U4) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 1) == TERM_PARAM(U1) &&
      NBG_State_op(NBG_State_get_child(state, 0)) == TERM_OP(LDA) &&
      1) {
      cost = (LD_COST);
      if (cost != NBG_COST_UNDEF) {
        if (cost < NBG_State_cost(state, NTERM(reg_zx8))) {
          NBG_State__cost(state, NTERM(reg_zx8), cost);
          NBG_State__rule(state, NTERM(reg_zx8), 302);
          NBG_compute_closure(state, NTERM(reg_zx8));
        }
        }
      }
    
    /* reg_zx8:ILOAD.U4.U1(reg) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(ILOAD) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(U4) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 1) == TERM_PARAM(U1) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      1) {
      cost = (LD_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg_zx8))) {
          NBG_State__cost(state, NTERM(reg_zx8), cost);
          NBG_State__rule(state, NTERM(reg_zx8), 303);
          NBG_compute_closure(state, NTERM(reg_zx8));
        }
        }
      }
    
    /* reg_sx8:ILOAD.I4.I1(LDA) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(ILOAD) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(I4) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 1) == TERM_PARAM(I1) &&
      NBG_State_op(NBG_State_get_child(state, 0)) == TERM_OP(LDA) &&
      1) {
      cost = (LD_COST);
      if (cost != NBG_COST_UNDEF) {
        if (cost < NBG_State_cost(state, NTERM(reg_sx8))) {
          NBG_State__cost(state, NTERM(reg_sx8), cost);
          NBG_State__rule(state, NTERM(reg_sx8), 306);
          NBG_compute_closure(state, NTERM(reg_sx8));
        }
        }
      }
    
    /* reg_sx8:ILOAD.I4.I1(reg) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(ILOAD) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(I4) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 1) == TERM_PARAM(I1) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      1) {
      cost = (LD_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg_sx8))) {
          NBG_State__cost(state, NTERM(reg_sx8), cost);
          NBG_State__rule(state, NTERM(reg_sx8), 307);
          NBG_compute_closure(state, NTERM(reg_sx8));
        }
        }
      }
    
    break;
  
  case TERM_OP(DIV):
    /* reg:DIV(reg,cnsti4) */
    if (NBG_State_op(state) == TERM_OP(DIV) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(cnsti4)) &&
      1) {
      cost = (DIV_CNST_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(cnsti4));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 37);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg:DIV(reg,reg) */
    if (NBG_State_op(state) == TERM_OP(DIV) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(reg)) &&
      1) {
      cost = (DIV_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 39);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    break;
  
  case TERM_OP(CALL):
    /* stmt:CALL.V */
    if (NBG_State_op(state) == TERM_OP(CALL) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(V) &&
      1) {
      cost = (1);
      if (cost != NBG_COST_UNDEF) {
        if (cost < NBG_State_cost(state, NTERM(stmt))) {
          NBG_State__cost(state, NTERM(stmt), cost);
          NBG_State__rule(state, NTERM(stmt), 119);
          NBG_compute_closure(state, NTERM(stmt));
        }
        }
      }
    
    /* stmt:CALL */
    if (NBG_State_op(state) == TERM_OP(CALL) &&
      1) {
      cost = (1);
      if (cost != NBG_COST_UNDEF) {
        if (cost < NBG_State_cost(state, NTERM(stmt))) {
          NBG_State__cost(state, NTERM(stmt), cost);
          NBG_State__rule(state, NTERM(stmt), 120);
          NBG_compute_closure(state, NTERM(stmt));
        }
        }
      }
    
    break;
  
  case TERM_OP(LDA_LABEL):
    /* reg:LDA_LABEL */
    if (NBG_State_op(state) == TERM_OP(LDA_LABEL) &&
      1) {
      cost = (FAIL_COST);
      if (cost != NBG_COST_UNDEF) {
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 136);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    break;
  
  case TERM_OP(RND):
    /* reg:RND(reg) */
    if (NBG_State_op(state) == TERM_OP(RND) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      1) {
      cost = (FAIL_COST&&FLT_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 95);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    break;
  
  case TERM_OP(CIOR):
    /* inv:CIOR(inv,inv) */
    if (NBG_State_op(state) == TERM_OP(CIOR) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(inv)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(inv)) &&
      1) {
      cost = (FAIL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(inv));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(inv));
        if (cost < NBG_State_cost(state, NTERM(inv))) {
          NBG_State__cost(state, NTERM(inv), cost);
          NBG_State__rule(state, NTERM(inv), 143);
          
        }
        }
      }
    
    break;
  
  case TERM_OP(LE):
    /* cond:LE(reg,rcon) */
    if (NBG_State_op(state) == TERM_OP(LE) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(rcon)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(rcon));
        if (cost < NBG_State_cost(state, NTERM(cond))) {
          NBG_State__cost(state, NTERM(cond), cost);
          NBG_State__rule(state, NTERM(cond), 78);
          
        }
        }
      }
    
    /* cond:LE(rcon,reg) */
    if (NBG_State_op(state) == TERM_OP(LE) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(rcon)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(reg)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(rcon));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(cond))) {
          NBG_State__cost(state, NTERM(cond), cost);
          NBG_State__rule(state, NTERM(cond), 79);
          
        }
        }
      }
    
    /* reg:LE(reg,rcon) */
    if (NBG_State_op(state) == TERM_OP(LE) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(rcon)) &&
      1) {
      cost = (1);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(rcon));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 90);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg:LE(rcon,reg) */
    if (NBG_State_op(state) == TERM_OP(LE) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(rcon)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(reg)) &&
      1) {
      cost = (1);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(rcon));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 91);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    break;
  
  case TERM_OP(MINMAX):
    /* reg:MINMAX(reg,reg) */
    if (NBG_State_op(state) == TERM_OP(MINMAX) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(reg)) &&
      1) {
      cost = (FAIL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 134);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    break;
  
  case TERM_OP(RECIP):
    /* reg:RECIP(reg) */
    if (NBG_State_op(state) == TERM_OP(RECIP) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      1) {
      cost = (FAIL_COST&&FLT_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 94);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    break;
  
  case TERM_OP(RROTATE):
    /* inv:RROTATE(inv,inv) */
    if (NBG_State_op(state) == TERM_OP(RROTATE) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(inv)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(inv)) &&
      1) {
      cost = (FAIL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(inv));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(inv));
        if (cost < NBG_State_cost(state, NTERM(inv))) {
          NBG_State__cost(state, NTERM(inv), cost);
          NBG_State__rule(state, NTERM(inv), 147);
          
        }
        }
      }
    
    break;
  
  case TERM_OP(LDBITS):
    /* reg:LDBITS###CLASS_PREG */
    if (NBG_State_op(state) == TERM_OP(LDBITS) &&
      NBG_TREE_VALUE_AT((NBG_TREE)NBG_State_utree(state), 2) == TERM_VALUE(CLASS_PREG) &&
      1) {
      cost = (1+0);
      if (cost != NBG_COST_UNDEF) {
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 20);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg:LDBITS */
    if (NBG_State_op(state) == TERM_OP(LDBITS) &&
      1) {
      cost = (1+LD_COST);
      if (cost != NBG_COST_UNDEF) {
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 21);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    break;
  
  case TERM_OP(BACKWARD_BARRIER):
    /* stmt:BACKWARD_BARRIER */
    if (NBG_State_op(state) == TERM_OP(BACKWARD_BARRIER) &&
      1) {
      cost = (1);
      if (cost != NBG_COST_UNDEF) {
        if (cost < NBG_State_cost(state, NTERM(stmt))) {
          NBG_State__cost(state, NTERM(stmt), cost);
          NBG_State__rule(state, NTERM(stmt), 158);
          NBG_compute_closure(state, NTERM(stmt));
        }
        }
      }
    
    break;
  
  case TERM_OP(BXOR):
    /* reg:BXOR(reg,rcon) */
    if (NBG_State_op(state) == TERM_OP(BXOR) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(rcon)) &&
      1) {
      cost = (ARI_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(rcon));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 56);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg:BXOR(rcon,reg) */
    if (NBG_State_op(state) == TERM_OP(BXOR) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(rcon)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(reg)) &&
      1) {
      cost = (ARI_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(rcon));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 57);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    break;
  
  case TERM_OP(GOTO):
    /* stmt:GOTO */
    if (NBG_State_op(state) == TERM_OP(GOTO) &&
      1) {
      cost = (1);
      if (cost != NBG_COST_UNDEF) {
        if (cost < NBG_State_cost(state, NTERM(stmt))) {
          NBG_State__cost(state, NTERM(stmt), cost);
          NBG_State__rule(state, NTERM(stmt), 108);
          NBG_compute_closure(state, NTERM(stmt));
        }
        }
      }
    
    break;
  
  case TERM_OP(MADD):
    /* reg:MADD(reg,reg,reg) */
    if (NBG_State_op(state) == TERM_OP(MADD) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 2), NTERM(reg)) &&
      1) {
      cost = (FAIL_COST&&MADD_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 2), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 47);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    break;
  
  case TERM_OP(COMPOSE_BITS):
    /* reg:COMPOSE_BITS(reg,reg) */
    if (NBG_State_op(state) == TERM_OP(COMPOSE_BITS) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(reg)) &&
      1) {
      cost = (FAIL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 130);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    break;
  
  case TERM_OP(FALSEBR):
    /* stmt:FALSEBR(reg) */
    if (NBG_State_op(state) == TERM_OP(FALSEBR) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      1) {
      cost = (1);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(stmt))) {
          NBG_State__cost(state, NTERM(stmt), cost);
          NBG_State__rule(state, NTERM(stmt), 113);
          NBG_compute_closure(state, NTERM(stmt));
        }
        }
      }
    
    /* stmt:FALSEBR(cond) */
    if (NBG_State_op(state) == TERM_OP(FALSEBR) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(cond)) &&
      1) {
      cost = (1);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(cond));
        if (cost < NBG_State_cost(state, NTERM(stmt))) {
          NBG_State__cost(state, NTERM(stmt), cost);
          NBG_State__rule(state, NTERM(stmt), 115);
          NBG_compute_closure(state, NTERM(stmt));
        }
        }
      }
    
    /* stmt:FALSEBR(INTCONST) */
    if (NBG_State_op(state) == TERM_OP(FALSEBR) &&
      NBG_State_op(NBG_State_get_child(state, 0)) == TERM_OP(INTCONST) &&
      1) {
      cost = (1);
      if (cost != NBG_COST_UNDEF) {
        if (cost < NBG_State_cost(state, NTERM(stmt))) {
          NBG_State__cost(state, NTERM(stmt), cost);
          NBG_State__rule(state, NTERM(stmt), 117);
          NBG_compute_closure(state, NTERM(stmt));
        }
        }
      }
    
    break;
  
  case TERM_OP(MLOAD):
    /* reg:MLOAD(reg,reg) */
    if (NBG_State_op(state) == TERM_OP(MLOAD) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(reg)) &&
      1) {
      cost = (FAIL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 139);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    break;
  
  case TERM_OP(PAIR):
    /* inv:PAIR(inv,inv) */
    if (NBG_State_op(state) == TERM_OP(PAIR) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(inv)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(inv)) &&
      1) {
      cost = (FAIL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(inv));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(inv));
        if (cost < NBG_State_cost(state, NTERM(inv))) {
          NBG_State__cost(state, NTERM(inv), cost);
          NBG_State__rule(state, NTERM(inv), 146);
          
        }
        }
      }
    
    break;
  
  case TERM_OP(AGOTO):
    /* stmt:AGOTO(reg) */
    if (NBG_State_op(state) == TERM_OP(AGOTO) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      1) {
      cost = (1);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(stmt))) {
          NBG_State__cost(state, NTERM(stmt), cost);
          NBG_State__rule(state, NTERM(stmt), 110);
          NBG_compute_closure(state, NTERM(stmt));
        }
        }
      }
    
    break;
  
  case TERM_OP(LT):
    /* cond:LT(reg,rcon) */
    if (NBG_State_op(state) == TERM_OP(LT) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(rcon)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(rcon));
        if (cost < NBG_State_cost(state, NTERM(cond))) {
          NBG_State__cost(state, NTERM(cond), cost);
          NBG_State__rule(state, NTERM(cond), 76);
          
        }
        }
      }
    
    /* cond:LT(rcon,reg) */
    if (NBG_State_op(state) == TERM_OP(LT) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(rcon)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(reg)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(rcon));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(cond))) {
          NBG_State__cost(state, NTERM(cond), cost);
          NBG_State__rule(state, NTERM(cond), 77);
          
        }
        }
      }
    
    /* reg:LT(reg,rcon) */
    if (NBG_State_op(state) == TERM_OP(LT) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(rcon)) &&
      1) {
      cost = (1);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(rcon));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 88);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg:LT(rcon,reg) */
    if (NBG_State_op(state) == TERM_OP(LT) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(rcon)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(reg)) &&
      1) {
      cost = (1);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(rcon));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 89);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    break;
  
  case TERM_OP(NMADD):
    /* reg:NMADD(reg,reg,reg) */
    if (NBG_State_op(state) == TERM_OP(NMADD) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 2), NTERM(reg)) &&
      1) {
      cost = (FAIL_COST&&MADD_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 2), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 49);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    break;
  
  case TERM_OP(FORWARD_BARRIER):
    /* stmt:FORWARD_BARRIER */
    if (NBG_State_op(state) == TERM_OP(FORWARD_BARRIER) &&
      1) {
      cost = (1);
      if (cost != NBG_COST_UNDEF) {
        if (cost < NBG_State_cost(state, NTERM(stmt))) {
          NBG_State__cost(state, NTERM(stmt), cost);
          NBG_State__rule(state, NTERM(stmt), 157);
          NBG_compute_closure(state, NTERM(stmt));
        }
        }
      }
    
    break;
  
  case TERM_OP(ADD):
    /* reg:ADD(reg,rcon) */
    if (NBG_State_op(state) == TERM_OP(ADD) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(rcon)) &&
      1) {
      cost = (ARI_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(rcon));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 27);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg:ADD(rcon,reg) */
    if (NBG_State_op(state) == TERM_OP(ADD) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(rcon)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(reg)) &&
      1) {
      cost = (ARI_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(rcon));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 28);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg:ADD(m_reg_shl_1,rcon) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(ADD) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(m_reg_shl_1)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(rcon)) &&
      1) {
      cost = (ARI_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(m_reg_shl_1));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(rcon));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 193);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg:ADD(m_reg_shl_2,rcon) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(ADD) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(m_reg_shl_2)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(rcon)) &&
      1) {
      cost = (ARI_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(m_reg_shl_2));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(rcon));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 194);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg:ADD(m_reg_shl_3,rcon) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(ADD) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(m_reg_shl_3)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(rcon)) &&
      1) {
      cost = (ARI_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(m_reg_shl_3));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(rcon));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 195);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg:ADD(m_reg_shl_4,rcon) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(ADD) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(m_reg_shl_4)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(rcon)) &&
      1) {
      cost = (ARI_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(m_reg_shl_4));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(rcon));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 196);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    break;
  
  case TERM_OP(WHERE):
    /* inv:WHERE(inv) */
    if (NBG_State_op(state) == TERM_OP(WHERE) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(inv)) &&
      1) {
      cost = (FAIL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(inv));
        if (cost < NBG_State_cost(state, NTERM(inv))) {
          NBG_State__cost(state, NTERM(inv), cost);
          NBG_State__rule(state, NTERM(inv), 156);
          
        }
        }
      }
    
    break;
  
  case TERM_OP(TRUNC):
    /* reg:TRUNC(reg) */
    if (NBG_State_op(state) == TERM_OP(TRUNC) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      1) {
      cost = (FAIL_COST&&FLT_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 96);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    break;
  
  case TERM_OP(FLOOR):
    /* reg:FLOOR(reg) */
    if (NBG_State_op(state) == TERM_OP(FLOOR) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      1) {
      cost = (FAIL_COST&&FLT_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 98);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    break;
  
  case TERM_OP(TAS):
    /* reg:TAS(reg) */
    if (NBG_State_op(state) == TERM_OP(TAS) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 105);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    break;
  
  case TERM_OP(TRUEBR):
    /* stmt:TRUEBR(reg) */
    if (NBG_State_op(state) == TERM_OP(TRUEBR) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      1) {
      cost = (1);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(stmt))) {
          NBG_State__cost(state, NTERM(stmt), cost);
          NBG_State__rule(state, NTERM(stmt), 112);
          NBG_compute_closure(state, NTERM(stmt));
        }
        }
      }
    
    /* stmt:TRUEBR(cond) */
    if (NBG_State_op(state) == TERM_OP(TRUEBR) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(cond)) &&
      1) {
      cost = (1);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(cond));
        if (cost < NBG_State_cost(state, NTERM(stmt))) {
          NBG_State__cost(state, NTERM(stmt), cost);
          NBG_State__rule(state, NTERM(stmt), 114);
          NBG_compute_closure(state, NTERM(stmt));
        }
        }
      }
    
    /* stmt:TRUEBR(INTCONST) */
    if (NBG_State_op(state) == TERM_OP(TRUEBR) &&
      NBG_State_op(NBG_State_get_child(state, 0)) == TERM_OP(INTCONST) &&
      1) {
      cost = (1);
      if (cost != NBG_COST_UNDEF) {
        if (cost < NBG_State_cost(state, NTERM(stmt))) {
          NBG_State__cost(state, NTERM(stmt), cost);
          NBG_State__rule(state, NTERM(stmt), 116);
          NBG_compute_closure(state, NTERM(stmt));
        }
        }
      }
    
    break;
  
  case TERM_OP(LDID):
    /* reg:LDID###CLASS_PREG */
    if (NBG_State_op(state) == TERM_OP(LDID) &&
      NBG_TREE_VALUE_AT((NBG_TREE)NBG_State_utree(state), 2) == TERM_VALUE(CLASS_PREG) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 14);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg:LDID */
    if (NBG_State_op(state) == TERM_OP(LDID) &&
      1) {
      cost = (LD_COST);
      if (cost != NBG_COST_UNDEF) {
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 15);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg_zx16:LDID.U4.U2###CLASS_PREG */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(LDID) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(U4) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 1) == TERM_PARAM(U2) &&
      NBG_TREE_VALUE_AT((NBG_TREE)NBG_State_utree(state), 2) == TERM_VALUE(CLASS_PREG) &&
      1) {
      cost = (LD_COST);
      if (cost != NBG_COST_UNDEF) {
        if (cost < NBG_State_cost(state, NTERM(reg_zx16))) {
          NBG_State__cost(state, NTERM(reg_zx16), cost);
          NBG_State__rule(state, NTERM(reg_zx16), 292);
          NBG_compute_closure(state, NTERM(reg_zx16));
        }
        }
      }
    
    /* reg_zx16:LDID.U4.U2 */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(LDID) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(U4) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 1) == TERM_PARAM(U2) &&
      1) {
      cost = (LD_COST);
      if (cost != NBG_COST_UNDEF) {
        if (cost < NBG_State_cost(state, NTERM(reg_zx16))) {
          NBG_State__cost(state, NTERM(reg_zx16), cost);
          NBG_State__rule(state, NTERM(reg_zx16), 293);
          NBG_compute_closure(state, NTERM(reg_zx16));
        }
        }
      }
    
    /* reg_sx16:LDID.I4.I1###CLASS_PREG */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(LDID) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(I4) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 1) == TERM_PARAM(I1) &&
      NBG_TREE_VALUE_AT((NBG_TREE)NBG_State_utree(state), 2) == TERM_VALUE(CLASS_PREG) &&
      1) {
      cost = (LD_COST);
      if (cost != NBG_COST_UNDEF) {
        if (cost < NBG_State_cost(state, NTERM(reg_sx16))) {
          NBG_State__cost(state, NTERM(reg_sx16), cost);
          NBG_State__rule(state, NTERM(reg_sx16), 296);
          NBG_compute_closure(state, NTERM(reg_sx16));
        }
        }
      }
    
    /* reg_sx16:LDID.I4.I1 */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(LDID) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(I4) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 1) == TERM_PARAM(I1) &&
      1) {
      cost = (LD_COST);
      if (cost != NBG_COST_UNDEF) {
        if (cost < NBG_State_cost(state, NTERM(reg_sx16))) {
          NBG_State__cost(state, NTERM(reg_sx16), cost);
          NBG_State__rule(state, NTERM(reg_sx16), 297);
          NBG_compute_closure(state, NTERM(reg_sx16));
        }
        }
      }
    
    /* reg_zx8:LDID.U4.U1###CLASS_PREG */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(LDID) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(U4) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 1) == TERM_PARAM(U1) &&
      NBG_TREE_VALUE_AT((NBG_TREE)NBG_State_utree(state), 2) == TERM_VALUE(CLASS_PREG) &&
      1) {
      cost = (LD_COST);
      if (cost != NBG_COST_UNDEF) {
        if (cost < NBG_State_cost(state, NTERM(reg_zx8))) {
          NBG_State__cost(state, NTERM(reg_zx8), cost);
          NBG_State__rule(state, NTERM(reg_zx8), 300);
          NBG_compute_closure(state, NTERM(reg_zx8));
        }
        }
      }
    
    /* reg_zx8:LDID.U4.U1 */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(LDID) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(U4) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 1) == TERM_PARAM(U1) &&
      1) {
      cost = (LD_COST);
      if (cost != NBG_COST_UNDEF) {
        if (cost < NBG_State_cost(state, NTERM(reg_zx8))) {
          NBG_State__cost(state, NTERM(reg_zx8), cost);
          NBG_State__rule(state, NTERM(reg_zx8), 301);
          NBG_compute_closure(state, NTERM(reg_zx8));
        }
        }
      }
    
    /* reg_sx8:LDID.I4.I1###CLASS_PREG */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(LDID) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(I4) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 1) == TERM_PARAM(I1) &&
      NBG_TREE_VALUE_AT((NBG_TREE)NBG_State_utree(state), 2) == TERM_VALUE(CLASS_PREG) &&
      1) {
      cost = (LD_COST);
      if (cost != NBG_COST_UNDEF) {
        if (cost < NBG_State_cost(state, NTERM(reg_sx8))) {
          NBG_State__cost(state, NTERM(reg_sx8), cost);
          NBG_State__rule(state, NTERM(reg_sx8), 304);
          NBG_compute_closure(state, NTERM(reg_sx8));
        }
        }
      }
    
    /* reg_sx8:LDID.I4.I1 */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(LDID) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 0) == TERM_PARAM(I4) &&
      NBG_TREE_PARAM_AT((NBG_TREE)NBG_State_utree(state), 1) == TERM_PARAM(I1) &&
      1) {
      cost = (LD_COST);
      if (cost != NBG_COST_UNDEF) {
        if (cost < NBG_State_cost(state, NTERM(reg_sx8))) {
          NBG_State__cost(state, NTERM(reg_sx8), cost);
          NBG_State__rule(state, NTERM(reg_sx8), 305);
          NBG_compute_closure(state, NTERM(reg_sx8));
        }
        }
      }
    
    /* reuse_reg:LDID###CLASS_PREG */
    if ((CGG_LEVEL >= 2) &&
      NBG_State_op(state) == TERM_OP(LDID) &&
      NBG_TREE_VALUE_AT((NBG_TREE)NBG_State_utree(state), 2) == TERM_VALUE(CLASS_PREG) &&
      1) {
      cost = (REUSE_COST);
      if (cost != NBG_COST_UNDEF) {
        if (cost < NBG_State_cost(state, NTERM(reuse_reg))) {
          NBG_State__cost(state, NTERM(reuse_reg), cost);
          NBG_State__rule(state, NTERM(reuse_reg), 313);
          NBG_compute_closure(state, NTERM(reuse_reg));
        }
        }
      }
    
    break;
  
  case TERM_OP(LAND):
    /* reg:LAND(reg,rcon) */
    if (NBG_State_op(state) == TERM_OP(LAND) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(rcon)) &&
      1) {
      cost = (LOGIC_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(rcon));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 61);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg:LAND(rcon,reg) */
    if (NBG_State_op(state) == TERM_OP(LAND) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(rcon)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(reg)) &&
      1) {
      cost = (LOGIC_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(rcon));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 62);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    break;
  
  case TERM_OP(SHL):
    /* reg:SHL(reg,rcon) */
    if (NBG_State_op(state) == TERM_OP(SHL) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(rcon)) &&
      1) {
      cost = (ARI_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(rcon));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 34);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* m_reg_shl_1:SHL(reg,cnsti4_1) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(SHL) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(cnsti4_1)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(cnsti4_1));
        if (cost < NBG_State_cost(state, NTERM(m_reg_shl_1))) {
          NBG_State__cost(state, NTERM(m_reg_shl_1), cost);
          NBG_State__rule(state, NTERM(m_reg_shl_1), 185);
          
        }
        }
      }
    
    /* m_reg_shl_2:SHL(reg,cnsti4_2) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(SHL) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(cnsti4_2)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(cnsti4_2));
        if (cost < NBG_State_cost(state, NTERM(m_reg_shl_2))) {
          NBG_State__cost(state, NTERM(m_reg_shl_2), cost);
          NBG_State__rule(state, NTERM(m_reg_shl_2), 187);
          
        }
        }
      }
    
    /* m_reg_shl_3:SHL(reg,cnsti4_3) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(SHL) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(cnsti4_3)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(cnsti4_3));
        if (cost < NBG_State_cost(state, NTERM(m_reg_shl_3))) {
          NBG_State__cost(state, NTERM(m_reg_shl_3), cost);
          NBG_State__rule(state, NTERM(m_reg_shl_3), 189);
          
        }
        }
      }
    
    /* m_reg_shl_4:SHL(reg,cnsti4_4) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(SHL) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(cnsti4_4)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(cnsti4_4));
        if (cost < NBG_State_cost(state, NTERM(m_reg_shl_4))) {
          NBG_State__cost(state, NTERM(m_reg_shl_4), cost);
          NBG_State__rule(state, NTERM(m_reg_shl_4), 191);
          
        }
        }
      }
    
    /* reg:SHL(MPY(reg,m_reg_zx16h),cnsti4_16) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(SHL) &&
      NBG_State_op(NBG_State_get_child(state, 0)) == TERM_OP(MPY) &&
      NBG_State_rule(NBG_State_get_child(NBG_State_get_child(state, 0), 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(NBG_State_get_child(state, 0), 1), NTERM(m_reg_zx16h)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(cnsti4_16)) &&
      1) {
      cost = (MUL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(NBG_State_get_child(state, 0), 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(NBG_State_get_child(state, 0), 1), NTERM(m_reg_zx16h));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(cnsti4_16));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 197);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg:SHL(MPY(m_reg_zx16h,reg),cnsti4_16) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_op(state) == TERM_OP(SHL) &&
      NBG_State_op(NBG_State_get_child(state, 0)) == TERM_OP(MPY) &&
      NBG_State_rule(NBG_State_get_child(NBG_State_get_child(state, 0), 0), NTERM(m_reg_zx16h)) &&
      NBG_State_rule(NBG_State_get_child(NBG_State_get_child(state, 0), 1), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(cnsti4_16)) &&
      1) {
      cost = (MUL_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(NBG_State_get_child(state, 0), 0), NTERM(m_reg_zx16h));
        cost += NBG_State_cost(NBG_State_get_child(NBG_State_get_child(state, 0), 1), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(cnsti4_16));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 198);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    break;
  
  case TERM_OP(LDMA):
    /* reg:LDMA */
    if (NBG_State_op(state) == TERM_OP(LDMA) &&
      1) {
      cost = (FAIL_COST);
      if (cost != NBG_COST_UNDEF) {
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 135);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    break;
  
  case TERM_OP(EQ):
    /* cond:EQ(reg,rcon) */
    if (NBG_State_op(state) == TERM_OP(EQ) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(rcon)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(rcon));
        if (cost < NBG_State_cost(state, NTERM(cond))) {
          NBG_State__cost(state, NTERM(cond), cost);
          NBG_State__rule(state, NTERM(cond), 68);
          
        }
        }
      }
    
    /* cond:EQ(rcon,rcon) */
    if (NBG_State_op(state) == TERM_OP(EQ) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(rcon)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(rcon)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(rcon));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(rcon));
        if (cost < NBG_State_cost(state, NTERM(cond))) {
          NBG_State__cost(state, NTERM(cond), cost);
          NBG_State__rule(state, NTERM(cond), 69);
          
        }
        }
      }
    
    /* reg:EQ(reg,rcon) */
    if (NBG_State_op(state) == TERM_OP(EQ) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(reg)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(rcon)) &&
      1) {
      cost = (1);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(reg));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(rcon));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 80);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg:EQ(rcon,rcon) */
    if (NBG_State_op(state) == TERM_OP(EQ) &&
      NBG_State_rule(NBG_State_get_child(state, 0), NTERM(rcon)) &&
      NBG_State_rule(NBG_State_get_child(state, 1), NTERM(rcon)) &&
      1) {
      cost = (1);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(NBG_State_get_child(state, 0), NTERM(rcon));
        cost += NBG_State_cost(NBG_State_get_child(state, 1), NTERM(rcon));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 81);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    break;
  
  }
  
}


static void
NBG_compute_closure(NBG_State state, int nterm)
{
  NBG_Cost cost;
  switch(nterm) {
  case NTERM(m_reg_sx16h):
    /* m_reg_sx16: m_reg_sx16h = 244 : (0) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_rule(state, NTERM(m_reg_sx16h)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(state, NTERM(m_reg_sx16h));
        if (cost < NBG_State_cost(state, NTERM(m_reg_sx16))) {
          NBG_State__cost(state, NTERM(m_reg_sx16), cost);
          NBG_State__rule(state, NTERM(m_reg_sx16), 244);
          NBG_compute_closure(state, NTERM(m_reg_sx16));
        }
        }
      }
    
    break;
  
  case NTERM(m_reg_zx16h):
    /* m_reg_zx16: m_reg_zx16h = 251 : (0) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_rule(state, NTERM(m_reg_zx16h)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(state, NTERM(m_reg_zx16h));
        if (cost < NBG_State_cost(state, NTERM(m_reg_zx16))) {
          NBG_State__cost(state, NTERM(m_reg_zx16), cost);
          NBG_State__rule(state, NTERM(m_reg_zx16), 251);
          NBG_compute_closure(state, NTERM(m_reg_zx16));
        }
        }
      }
    
    break;
  
  case NTERM(stmt):
    /* root: stmt = 1 : (0) */
    if (NBG_State_rule(state, NTERM(stmt)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(state, NTERM(stmt));
        if (cost < NBG_State_cost(state, NTERM(root))) {
          NBG_State__cost(state, NTERM(root), cost);
          NBG_State__rule(state, NTERM(root), 1);
          
        }
        }
      }
    
    break;
  
  case NTERM(m_reg_sx8):
    /* reg_or_m_reg_sx8: m_reg_sx8 = 233 : (0) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_rule(state, NTERM(m_reg_sx8)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(state, NTERM(m_reg_sx8));
        if (cost < NBG_State_cost(state, NTERM(reg_or_m_reg_sx8))) {
          NBG_State__cost(state, NTERM(reg_or_m_reg_sx8), cost);
          NBG_State__rule(state, NTERM(reg_or_m_reg_sx8), 233);
          
        }
        }
      }
    
    /* m_reg_sx16: m_reg_sx8 = 241 : (0) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_rule(state, NTERM(m_reg_sx8)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(state, NTERM(m_reg_sx8));
        if (cost < NBG_State_cost(state, NTERM(m_reg_sx16))) {
          NBG_State__cost(state, NTERM(m_reg_sx16), cost);
          NBG_State__rule(state, NTERM(m_reg_sx16), 241);
          NBG_compute_closure(state, NTERM(m_reg_sx16));
        }
        }
      }
    
    break;
  
  case NTERM(reg_zx8):
    /* m_reg_zx8: reg_zx8 = 235 : (0) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_rule(state, NTERM(reg_zx8)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(state, NTERM(reg_zx8));
        if (cost < NBG_State_cost(state, NTERM(m_reg_zx8))) {
          NBG_State__cost(state, NTERM(m_reg_zx8), cost);
          NBG_State__rule(state, NTERM(m_reg_zx8), 235);
          NBG_compute_closure(state, NTERM(m_reg_zx8));
        }
        }
      }
    
    /* reg: reg_zx8 = 262 : (0) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_rule(state, NTERM(reg_zx8)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(state, NTERM(reg_zx8));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 262);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg_sx16: reg_zx8 = 275 : (0) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_rule(state, NTERM(reg_zx8)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(state, NTERM(reg_zx8));
        if (cost < NBG_State_cost(state, NTERM(reg_sx16))) {
          NBG_State__cost(state, NTERM(reg_sx16), cost);
          NBG_State__rule(state, NTERM(reg_sx16), 275);
          NBG_compute_closure(state, NTERM(reg_sx16));
        }
        }
      }
    
    /* reg_zx16: reg_zx8 = 283 : (0) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_rule(state, NTERM(reg_zx8)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(state, NTERM(reg_zx8));
        if (cost < NBG_State_cost(state, NTERM(reg_zx16))) {
          NBG_State__cost(state, NTERM(reg_zx16), cost);
          NBG_State__rule(state, NTERM(reg_zx16), 283);
          NBG_compute_closure(state, NTERM(reg_zx16));
        }
        }
      }
    
    break;
  
  case NTERM(reuse_reg):
    /* reg: reuse_reg = 308 : (0) */
    if ((CGG_LEVEL >= 2) &&
      NBG_State_rule(state, NTERM(reuse_reg)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(state, NTERM(reuse_reg));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 308);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg_sx16: reuse_reg = 309 : (0) */
    if ((CGG_LEVEL >= 2) &&
      NBG_State_rule(state, NTERM(reuse_reg)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(state, NTERM(reuse_reg));
        if (cost < NBG_State_cost(state, NTERM(reg_sx16))) {
          NBG_State__cost(state, NTERM(reg_sx16), cost);
          NBG_State__rule(state, NTERM(reg_sx16), 309);
          NBG_compute_closure(state, NTERM(reg_sx16));
        }
        }
      }
    
    /* reg_zx16: reuse_reg = 310 : (0) */
    if ((CGG_LEVEL >= 2) &&
      NBG_State_rule(state, NTERM(reuse_reg)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(state, NTERM(reuse_reg));
        if (cost < NBG_State_cost(state, NTERM(reg_zx16))) {
          NBG_State__cost(state, NTERM(reg_zx16), cost);
          NBG_State__rule(state, NTERM(reg_zx16), 310);
          NBG_compute_closure(state, NTERM(reg_zx16));
        }
        }
      }
    
    /* reg_sx8: reuse_reg = 311 : (0) */
    if ((CGG_LEVEL >= 2) &&
      NBG_State_rule(state, NTERM(reuse_reg)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(state, NTERM(reuse_reg));
        if (cost < NBG_State_cost(state, NTERM(reg_sx8))) {
          NBG_State__cost(state, NTERM(reg_sx8), cost);
          NBG_State__rule(state, NTERM(reg_sx8), 311);
          NBG_compute_closure(state, NTERM(reg_sx8));
        }
        }
      }
    
    /* reg_zx8: reuse_reg = 312 : (0) */
    if ((CGG_LEVEL >= 2) &&
      NBG_State_rule(state, NTERM(reuse_reg)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(state, NTERM(reuse_reg));
        if (cost < NBG_State_cost(state, NTERM(reg_zx8))) {
          NBG_State__cost(state, NTERM(reg_zx8), cost);
          NBG_State__rule(state, NTERM(reg_zx8), 312);
          NBG_compute_closure(state, NTERM(reg_zx8));
        }
        }
      }
    
    break;
  
  case NTERM(cnsti4_zx8):
    /* cnsti4_zx16: cnsti4_zx8 = 170 : (0) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_rule(state, NTERM(cnsti4_zx8)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(state, NTERM(cnsti4_zx8));
        if (cost < NBG_State_cost(state, NTERM(cnsti4_zx16))) {
          NBG_State__cost(state, NTERM(cnsti4_zx16), cost);
          NBG_State__rule(state, NTERM(cnsti4_zx16), 170);
          NBG_compute_closure(state, NTERM(cnsti4_zx16));
        }
        }
      }
    
    /* reg_zx8: cnsti4_zx8 = 272 : (INTCONST_COST) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_rule(state, NTERM(cnsti4_zx8)) &&
      1) {
      cost = (INTCONST_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(state, NTERM(cnsti4_zx8));
        if (cost < NBG_State_cost(state, NTERM(reg_zx8))) {
          NBG_State__cost(state, NTERM(reg_zx8), cost);
          NBG_State__rule(state, NTERM(reg_zx8), 272);
          NBG_compute_closure(state, NTERM(reg_zx8));
        }
        }
      }
    
    break;
  
  case NTERM(reg):
    /* root: reg = 2 : (0) */
    if (NBG_State_rule(state, NTERM(reg)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(state, NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(root))) {
          NBG_State__cost(state, NTERM(root), cost);
          NBG_State__rule(state, NTERM(root), 2);
          
        }
        }
      }
    
    /* rcon: reg = 3 : (0) */
    if (NBG_State_rule(state, NTERM(reg)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(state, NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(rcon))) {
          NBG_State__cost(state, NTERM(rcon), cost);
          NBG_State__rule(state, NTERM(rcon), 3);
          
        }
        }
      }
    
    /* reg_or_m_reg_sx8: reg = 232 : (0) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_rule(state, NTERM(reg)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(state, NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg_or_m_reg_sx8))) {
          NBG_State__cost(state, NTERM(reg_or_m_reg_sx8), cost);
          NBG_State__rule(state, NTERM(reg_or_m_reg_sx8), 232);
          
        }
        }
      }
    
    /* reg_or_m_reg_zx8: reg = 236 : (0) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_rule(state, NTERM(reg)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(state, NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg_or_m_reg_zx8))) {
          NBG_State__cost(state, NTERM(reg_or_m_reg_zx8), cost);
          NBG_State__rule(state, NTERM(reg_or_m_reg_zx8), 236);
          
        }
        }
      }
    
    /* reg_or_m_reg_sx16: reg = 242 : (0) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_rule(state, NTERM(reg)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(state, NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg_or_m_reg_sx16))) {
          NBG_State__cost(state, NTERM(reg_or_m_reg_sx16), cost);
          NBG_State__rule(state, NTERM(reg_or_m_reg_sx16), 242);
          
        }
        }
      }
    
    /* reg_or_m_reg_zx16: reg = 249 : (0) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_rule(state, NTERM(reg)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(state, NTERM(reg));
        if (cost < NBG_State_cost(state, NTERM(reg_or_m_reg_zx16))) {
          NBG_State__cost(state, NTERM(reg_or_m_reg_zx16), cost);
          NBG_State__rule(state, NTERM(reg_or_m_reg_zx16), 249);
          
        }
        }
      }
    
    break;
  
  case NTERM(cnsti4_zx16):
    /* reg_zx16: cnsti4_zx16 = 291 : (INTCONST_COST) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_rule(state, NTERM(cnsti4_zx16)) &&
      1) {
      cost = (INTCONST_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(state, NTERM(cnsti4_zx16));
        if (cost < NBG_State_cost(state, NTERM(reg_zx16))) {
          NBG_State__cost(state, NTERM(reg_zx16), cost);
          NBG_State__rule(state, NTERM(reg_zx16), 291);
          NBG_compute_closure(state, NTERM(reg_zx16));
        }
        }
      }
    
    break;
  
  case NTERM(reg_sx8):
    /* m_reg_sx8: reg_sx8 = 231 : (0) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_rule(state, NTERM(reg_sx8)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(state, NTERM(reg_sx8));
        if (cost < NBG_State_cost(state, NTERM(m_reg_sx8))) {
          NBG_State__cost(state, NTERM(m_reg_sx8), cost);
          NBG_State__rule(state, NTERM(m_reg_sx8), 231);
          NBG_compute_closure(state, NTERM(m_reg_sx8));
        }
        }
      }
    
    /* reg: reg_sx8 = 255 : (0) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_rule(state, NTERM(reg_sx8)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(state, NTERM(reg_sx8));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 255);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    /* reg_sx16: reg_sx8 = 274 : (0) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_rule(state, NTERM(reg_sx8)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(state, NTERM(reg_sx8));
        if (cost < NBG_State_cost(state, NTERM(reg_sx16))) {
          NBG_State__cost(state, NTERM(reg_sx16), cost);
          NBG_State__rule(state, NTERM(reg_sx16), 274);
          NBG_compute_closure(state, NTERM(reg_sx16));
        }
        }
      }
    
    break;
  
  case NTERM(cnsti4_sx8):
    /* cnsti4_sx16: cnsti4_sx8 = 167 : (0) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_rule(state, NTERM(cnsti4_sx8)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(state, NTERM(cnsti4_sx8));
        if (cost < NBG_State_cost(state, NTERM(cnsti4_sx16))) {
          NBG_State__cost(state, NTERM(cnsti4_sx16), cost);
          NBG_State__rule(state, NTERM(cnsti4_sx16), 167);
          NBG_compute_closure(state, NTERM(cnsti4_sx16));
        }
        }
      }
    
    /* reg_sx8: cnsti4_sx8 = 261 : (INTCONST_COST) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_rule(state, NTERM(cnsti4_sx8)) &&
      1) {
      cost = (INTCONST_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(state, NTERM(cnsti4_sx8));
        if (cost < NBG_State_cost(state, NTERM(reg_sx8))) {
          NBG_State__cost(state, NTERM(reg_sx8), cost);
          NBG_State__rule(state, NTERM(reg_sx8), 261);
          NBG_compute_closure(state, NTERM(reg_sx8));
        }
        }
      }
    
    break;
  
  case NTERM(cnsti4_sx16):
    /* reg_sx16: cnsti4_sx16 = 281 : (INTCONST_COST) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_rule(state, NTERM(cnsti4_sx16)) &&
      1) {
      cost = (INTCONST_COST);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(state, NTERM(cnsti4_sx16));
        if (cost < NBG_State_cost(state, NTERM(reg_sx16))) {
          NBG_State__cost(state, NTERM(reg_sx16), cost);
          NBG_State__rule(state, NTERM(reg_sx16), 281);
          NBG_compute_closure(state, NTERM(reg_sx16));
        }
        }
      }
    
    break;
  
  case NTERM(reg_zx16):
    /* m_reg_zx16: reg_zx16 = 247 : (0) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_rule(state, NTERM(reg_zx16)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(state, NTERM(reg_zx16));
        if (cost < NBG_State_cost(state, NTERM(m_reg_zx16))) {
          NBG_State__cost(state, NTERM(m_reg_zx16), cost);
          NBG_State__rule(state, NTERM(m_reg_zx16), 247);
          NBG_compute_closure(state, NTERM(m_reg_zx16));
        }
        }
      }
    
    /* reg: reg_zx16 = 282 : (0) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_rule(state, NTERM(reg_zx16)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(state, NTERM(reg_zx16));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 282);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    break;
  
  case NTERM(m_reg_zx16):
    /* reg_or_m_reg_zx16: m_reg_zx16 = 250 : (0) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_rule(state, NTERM(m_reg_zx16)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(state, NTERM(m_reg_zx16));
        if (cost < NBG_State_cost(state, NTERM(reg_or_m_reg_zx16))) {
          NBG_State__cost(state, NTERM(reg_or_m_reg_zx16), cost);
          NBG_State__rule(state, NTERM(reg_or_m_reg_zx16), 250);
          
        }
        }
      }
    
    break;
  
  case NTERM(cnsti4):
    /* rcon: cnsti4 = 161 : (0) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_rule(state, NTERM(cnsti4)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(state, NTERM(cnsti4));
        if (cost < NBG_State_cost(state, NTERM(rcon))) {
          NBG_State__cost(state, NTERM(rcon), cost);
          NBG_State__rule(state, NTERM(rcon), 161);
          
        }
        }
      }
    
    break;
  
  case NTERM(m_reg_zx8):
    /* reg_or_m_reg_zx8: m_reg_zx8 = 237 : (0) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_rule(state, NTERM(m_reg_zx8)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(state, NTERM(m_reg_zx8));
        if (cost < NBG_State_cost(state, NTERM(reg_or_m_reg_zx8))) {
          NBG_State__cost(state, NTERM(reg_or_m_reg_zx8), cost);
          NBG_State__rule(state, NTERM(reg_or_m_reg_zx8), 237);
          
        }
        }
      }
    
    /* m_reg_zx16: m_reg_zx8 = 248 : (0) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_rule(state, NTERM(m_reg_zx8)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(state, NTERM(m_reg_zx8));
        if (cost < NBG_State_cost(state, NTERM(m_reg_zx16))) {
          NBG_State__cost(state, NTERM(m_reg_zx16), cost);
          NBG_State__rule(state, NTERM(m_reg_zx16), 248);
          NBG_compute_closure(state, NTERM(m_reg_zx16));
        }
        }
      }
    
    break;
  
  case NTERM(m_reg_sx16):
    /* reg_or_m_reg_sx16: m_reg_sx16 = 243 : (0) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_rule(state, NTERM(m_reg_sx16)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(state, NTERM(m_reg_sx16));
        if (cost < NBG_State_cost(state, NTERM(reg_or_m_reg_sx16))) {
          NBG_State__cost(state, NTERM(reg_or_m_reg_sx16), cost);
          NBG_State__rule(state, NTERM(reg_or_m_reg_sx16), 243);
          
        }
        }
      }
    
    break;
  
  case NTERM(reg_sx16):
    /* m_reg_sx16: reg_sx16 = 240 : (0) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_rule(state, NTERM(reg_sx16)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(state, NTERM(reg_sx16));
        if (cost < NBG_State_cost(state, NTERM(m_reg_sx16))) {
          NBG_State__cost(state, NTERM(m_reg_sx16), cost);
          NBG_State__rule(state, NTERM(m_reg_sx16), 240);
          NBG_compute_closure(state, NTERM(m_reg_sx16));
        }
        }
      }
    
    /* reg: reg_sx16 = 273 : (0) */
    if ((CGG_LEVEL >= 1) &&
      NBG_State_rule(state, NTERM(reg_sx16)) &&
      1) {
      cost = (0);
      if (cost != NBG_COST_UNDEF) {
        cost += NBG_State_cost(state, NTERM(reg_sx16));
        if (cost < NBG_State_cost(state, NTERM(reg))) {
          NBG_State__cost(state, NTERM(reg), cost);
          NBG_State__rule(state, NTERM(reg), 273);
          NBG_compute_closure(state, NTERM(reg));
        }
        }
      }
    
    break;
  
  }
}


static NBG_emit_action_f_t emit_rule_action_noaction;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_cnsti4;
static NBG_emit_action_f_t emit_rule_action_STID_PREG;
static NBG_emit_action_f_t emit_rule_action_STID;
static NBG_emit_action_f_t emit_rule_action_stmt_ISTORE_reg_LDA_;
static NBG_emit_action_f_t emit_rule_action_stmt_ISTORE_reg_reg_;
static NBG_emit_action_f_t emit_rule_action_stmt_ISTOREX_reg_reg_reg_;
static NBG_emit_action_f_t emit_rule_action_stmt_STBITS___CLASS__PREG_reg_;
static NBG_emit_action_f_t emit_rule_action_stmt_STBITS_reg_;
static NBG_emit_action_f_t emit_rule_action_stmt_ISTBITS_reg_LDA_;
static NBG_emit_action_f_t emit_rule_action_stmt_ISTBITS_reg_reg_;
static NBG_emit_action_f_t emit_rule_action_LDID_PREG;
static NBG_emit_action_f_t emit_rule_action_LDID;
static NBG_emit_action_f_t emit_rule_action_ILOAD_LDA;
static NBG_emit_action_f_t emit_rule_action_ILOAD;
static NBG_emit_action_f_t emit_rule_action_reg_LDA;
static NBG_emit_action_f_t emit_rule_action_reg_ILOADX_reg_reg_;
static NBG_emit_action_f_t emit_rule_action_reg_LDBITS___CLASS__PREG;
static NBG_emit_action_f_t emit_rule_action_reg_LDBITS;
static NBG_emit_action_f_t emit_rule_action_reg_ILDBITS_LDA_;
static NBG_emit_action_f_t emit_rule_action_reg_ILDBITS_reg_;
static NBG_emit_action_f_t emit_rule_action_INTCONST_0;
static NBG_emit_action_f_t emit_rule_action_INTCONST;
static NBG_emit_action_f_t emit_rule_action_CONST;
static NBG_emit_action_f_t emit_rule_action_ADD;
static NBG_emit_action_f_t emit_rule_action_ADD;
static NBG_emit_action_f_t emit_rule_action_SUB;
static NBG_emit_action_f_t emit_rule_action_SUB;
static NBG_emit_action_f_t emit_rule_action_NEG;
static NBG_emit_action_f_t emit_rule_action_ASHR;
static NBG_emit_action_f_t emit_rule_action_LSHR;
static NBG_emit_action_f_t emit_rule_action_SHL;
static NBG_emit_action_f_t emit_rule_action_MPY32;
static NBG_emit_action_f_t emit_rule_action_MPY32;
static NBG_emit_action_f_t emit_rule_action_DIV_CNST;
static NBG_emit_action_f_t emit_rule_action_REM_CNST;
static NBG_emit_action_f_t emit_rule_action_DIV;
static NBG_emit_action_f_t emit_rule_action_REM;
static NBG_emit_action_f_t emit_rule_action_MOD;
static NBG_emit_action_f_t emit_rule_action_ABS;
static NBG_emit_action_f_t emit_rule_action_MAX;
static NBG_emit_action_f_t emit_rule_action_MAX;
static NBG_emit_action_f_t emit_rule_action_MIN;
static NBG_emit_action_f_t emit_rule_action_MIN;
static NBG_emit_action_f_t emit_rule_action_reg_MADD_reg_reg_reg_;
static NBG_emit_action_f_t emit_rule_action_reg_MSUB_reg_reg_reg_;
static NBG_emit_action_f_t emit_rule_action_reg_NMADD_reg_reg_reg_;
static NBG_emit_action_f_t emit_rule_action_reg_NMSUB_reg_reg_reg_;
static NBG_emit_action_f_t emit_rule_action_BNOT;
static NBG_emit_action_f_t emit_rule_action_BAND;
static NBG_emit_action_f_t emit_rule_action_BAND;
static NBG_emit_action_f_t emit_rule_action_BIOR;
static NBG_emit_action_f_t emit_rule_action_BIOR;
static NBG_emit_action_f_t emit_rule_action_BXOR;
static NBG_emit_action_f_t emit_rule_action_BXOR;
static NBG_emit_action_f_t emit_rule_action_BNOR;
static NBG_emit_action_f_t emit_rule_action_BNOR;
static NBG_emit_action_f_t emit_rule_action_LNOT;
static NBG_emit_action_f_t emit_rule_action_LAND;
static NBG_emit_action_f_t emit_rule_action_LAND;
static NBG_emit_action_f_t emit_rule_action_LIOR;
static NBG_emit_action_f_t emit_rule_action_LIOR;
static NBG_emit_action_f_t emit_rule_action_SELECT_cond;
static NBG_emit_action_f_t emit_rule_action_SELECT_cond;
static NBG_emit_action_f_t emit_rule_action_SELECT_reg;
static NBG_emit_action_f_t emit_rule_action_noaction;
static NBG_emit_action_f_t emit_rule_action_noaction;
static NBG_emit_action_f_t emit_rule_action_noaction;
static NBG_emit_action_f_t emit_rule_action_noaction;
static NBG_emit_action_f_t emit_rule_action_noaction;
static NBG_emit_action_f_t emit_rule_action_noaction;
static NBG_emit_action_f_t emit_rule_action_noaction;
static NBG_emit_action_f_t emit_rule_action_noaction;
static NBG_emit_action_f_t emit_rule_action_noaction;
static NBG_emit_action_f_t emit_rule_action_noaction;
static NBG_emit_action_f_t emit_rule_action_noaction;
static NBG_emit_action_f_t emit_rule_action_noaction;
static NBG_emit_action_f_t emit_rule_action_COMPARE_INT;
static NBG_emit_action_f_t emit_rule_action_COMPARE_INT;
static NBG_emit_action_f_t emit_rule_action_COMPARE_INT;
static NBG_emit_action_f_t emit_rule_action_COMPARE_INT;
static NBG_emit_action_f_t emit_rule_action_COMPARE_INT;
static NBG_emit_action_f_t emit_rule_action_COMPARE_INT;
static NBG_emit_action_f_t emit_rule_action_COMPARE_INT;
static NBG_emit_action_f_t emit_rule_action_COMPARE_INT;
static NBG_emit_action_f_t emit_rule_action_COMPARE_INT;
static NBG_emit_action_f_t emit_rule_action_COMPARE_INT;
static NBG_emit_action_f_t emit_rule_action_COMPARE_INT;
static NBG_emit_action_f_t emit_rule_action_COMPARE_INT;
static NBG_emit_action_f_t emit_rule_action_reg_SQRT_reg_;
static NBG_emit_action_f_t emit_rule_action_reg_RSQRT_reg_;
static NBG_emit_action_f_t emit_rule_action_reg_RECIP_reg_;
static NBG_emit_action_f_t emit_rule_action_reg_RND_reg_;
static NBG_emit_action_f_t emit_rule_action_reg_TRUNC_reg_;
static NBG_emit_action_f_t emit_rule_action_reg_CEIL_reg_;
static NBG_emit_action_f_t emit_rule_action_reg_FLOOR_reg_;
static NBG_emit_action_f_t emit_rule_action_CVTLI4_16;
static NBG_emit_action_f_t emit_rule_action_CVTLU4_16;
static NBG_emit_action_f_t emit_rule_action_CVTLI4_8;
static NBG_emit_action_f_t emit_rule_action_CVTLU4_8;
static NBG_emit_action_f_t emit_rule_action_reg_CVTL_reg_;
static NBG_emit_action_f_t emit_rule_action_reg_CVT_reg_;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_reg_EXTRACT__BITS_reg_;
static NBG_emit_action_f_t emit_rule_action_GOTO;
static NBG_emit_action_f_t emit_rule_action_GOTO;
static NBG_emit_action_f_t emit_rule_action_AGOTO;
static NBG_emit_action_f_t emit_rule_action_XGOTO;
static NBG_emit_action_f_t emit_rule_action_CONDBR_reg;
static NBG_emit_action_f_t emit_rule_action_CONDBR_reg;
static NBG_emit_action_f_t emit_rule_action_CONDBR_cond;
static NBG_emit_action_f_t emit_rule_action_CONDBR_cond;
static NBG_emit_action_f_t emit_rule_action_CONDBR_const;
static NBG_emit_action_f_t emit_rule_action_CONDBR_const;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_CALL;
static NBG_emit_action_f_t emit_rule_action_CALL;
static NBG_emit_action_f_t emit_rule_action_CALL;
static NBG_emit_action_f_t emit_rule_action_CALL;
static NBG_emit_action_f_t emit_rule_action_RETURN;
static NBG_emit_action_f_t emit_rule_action_CALL;
static NBG_emit_action_f_t emit_rule_action_CALL;
static NBG_emit_action_f_t emit_rule_action_reg_LOWPART_reg_;
static NBG_emit_action_f_t emit_rule_action_reg_HIGHPART_reg_;
static NBG_emit_action_f_t emit_rule_action_reg_MINPART_reg_;
static NBG_emit_action_f_t emit_rule_action_reg_MAXPART_reg_;
static NBG_emit_action_f_t emit_rule_action_reg_COMPOSE__BITS_reg_reg_;
static NBG_emit_action_f_t emit_rule_action_reg_HIGHMPY_reg_reg_;
static NBG_emit_action_f_t emit_rule_action_reg_XMPY_reg_reg_;
static NBG_emit_action_f_t emit_rule_action_reg_DIVREM_reg_reg_;
static NBG_emit_action_f_t emit_rule_action_reg_MINMAX_reg_reg_;
static NBG_emit_action_f_t emit_rule_action_reg_LDMA;
static NBG_emit_action_f_t emit_rule_action_reg_LDA__LABEL;
static NBG_emit_action_f_t emit_rule_action_stmt_IDNAME;
static NBG_emit_action_f_t emit_rule_action_stmt_MSTORE_reg_reg_reg_;
static NBG_emit_action_f_t emit_rule_action_reg_MLOAD_reg_reg_;
static NBG_emit_action_f_t emit_rule_action_inv_FIRSTPART_inv_;
static NBG_emit_action_f_t emit_rule_action_inv_SECONDPART_inv_;
static NBG_emit_action_f_t emit_rule_action_inv_CAND_inv_inv_;
static NBG_emit_action_f_t emit_rule_action_inv_CIOR_inv_inv_;
static NBG_emit_action_f_t emit_rule_action_inv_CSELECT_inv_inv_;
static NBG_emit_action_f_t emit_rule_action_inv_ILDA_inv_;
static NBG_emit_action_f_t emit_rule_action_inv_PAIR_inv_inv_;
static NBG_emit_action_f_t emit_rule_action_inv_RROTATE_inv_inv_;
static NBG_emit_action_f_t emit_rule_action_inv_COMMA_inv_inv_;
static NBG_emit_action_f_t emit_rule_action_inv_RCOMMA_inv_inv_;
static NBG_emit_action_f_t emit_rule_action_inv_ARRAY_inv_;
static NBG_emit_action_f_t emit_rule_action_inv_INTRINSIC__OP_inv_;
static NBG_emit_action_f_t emit_rule_action_inv_IO__ITEM_inv_;
static NBG_emit_action_f_t emit_rule_action_inv_TRIPLET_inv_;
static NBG_emit_action_f_t emit_rule_action_inv_ARRAYEXP_inv_;
static NBG_emit_action_f_t emit_rule_action_inv_ARRSECTION_inv_;
static NBG_emit_action_f_t emit_rule_action_inv_WHERE_inv_;
static NBG_emit_action_f_t emit_rule_action_stmt_FORWARD__BARRIER;
static NBG_emit_action_f_t emit_rule_action_stmt_BACKWARD__BARRIER;
static NBG_emit_action_f_t emit_rule_action_stmt_INTRINSIC__CALL_V;
static NBG_emit_action_f_t emit_rule_action_reg_INTRINSIC__CALL;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_cnsti4;
static NBG_emit_action_f_t emit_rule_action_cnsti4;
static NBG_emit_action_f_t emit_rule_action_cnsti4;
static NBG_emit_action_f_t emit_rule_action_cnsti4;
static NBG_emit_action_f_t emit_rule_action_cnsti4;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_cnsti4;
static NBG_emit_action_f_t emit_rule_action_cnsti4;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_cnsti4;
static NBG_emit_action_f_t emit_rule_action_cnsti4;
static NBG_emit_action_f_t emit_rule_action_cnsti4;
static NBG_emit_action_f_t emit_rule_action_cnsti4;
static NBG_emit_action_f_t emit_rule_action_cnsti4;
static NBG_emit_action_f_t emit_rule_action_cnsti4;
static NBG_emit_action_f_t emit_rule_action_cnsti4;
static NBG_emit_action_f_t emit_rule_action_cnsti4;
static NBG_emit_action_f_t emit_rule_action_cnsti4;
static NBG_emit_action_f_t emit_rule_action_cnsti4;
static NBG_emit_action_f_t emit_rule_action_MPY_PW2;
static NBG_emit_action_f_t emit_rule_action_MPY_PW2;
static NBG_emit_action_f_t emit_rule_action_MPY_PW2;
static NBG_emit_action_f_t emit_rule_action_MPY_PW2;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_SH1ADD;
static NBG_emit_action_f_t emit_rule_action_SH2ADD;
static NBG_emit_action_f_t emit_rule_action_SH3ADD;
static NBG_emit_action_f_t emit_rule_action_SH4ADD;
static NBG_emit_action_f_t emit_rule_action_MULHS;
static NBG_emit_action_f_t emit_rule_action_MULHS_2;
static NBG_emit_action_f_t emit_rule_action_MULHH;
static NBG_emit_action_f_t emit_rule_action_MULHHU;
static NBG_emit_action_f_t emit_rule_action_MULLH;
static NBG_emit_action_f_t emit_rule_action_MULLH_2;
static NBG_emit_action_f_t emit_rule_action_MULLHU;
static NBG_emit_action_f_t emit_rule_action_MULLHU_2;
static NBG_emit_action_f_t emit_rule_action_MULH;
static NBG_emit_action_f_t emit_rule_action_MULH_2;
static NBG_emit_action_f_t emit_rule_action_MULHU;
static NBG_emit_action_f_t emit_rule_action_MULHU_2;
static NBG_emit_action_f_t emit_rule_action_MULLL;
static NBG_emit_action_f_t emit_rule_action_MULLL;
static NBG_emit_action_f_t emit_rule_action_MULLL_2;
static NBG_emit_action_f_t emit_rule_action_MULLLU;
static NBG_emit_action_f_t emit_rule_action_MULLL;
static NBG_emit_action_f_t emit_rule_action_MULLLU_2;
static NBG_emit_action_f_t emit_rule_action_MULL;
static NBG_emit_action_f_t emit_rule_action_MULL;
static NBG_emit_action_f_t emit_rule_action_MULL_2;
static NBG_emit_action_f_t emit_rule_action_MULL_2;
static NBG_emit_action_f_t emit_rule_action_MULLU;
static NBG_emit_action_f_t emit_rule_action_MULLU;
static NBG_emit_action_f_t emit_rule_action_MULLU_2;
static NBG_emit_action_f_t emit_rule_action_MULLU_2;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_CVTLI4_8;
static NBG_emit_action_f_t emit_rule_action_SHRI4_cnst;
static NBG_emit_action_f_t emit_rule_action_SHRI4_cnst;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_INTCONST;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_CVTLU4_8;
static NBG_emit_action_f_t emit_rule_action_CVTLU4_8;
static NBG_emit_action_f_t emit_rule_action_SHRU4_cnst;
static NBG_emit_action_f_t emit_rule_action_SHRU4_cnst;
static NBG_emit_action_f_t emit_rule_action_SHRI4_cnst;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_INTCONST;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_CVTLI4_16;
static NBG_emit_action_f_t emit_rule_action_CVTLI4_16;
static NBG_emit_action_f_t emit_rule_action_SHRI4_cnst;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_INTCONST;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_CVTLU4_16;
static NBG_emit_action_f_t emit_rule_action_CVTLU4_16;
static NBG_emit_action_f_t emit_rule_action_CVTLU4_16;
static NBG_emit_action_f_t emit_rule_action_SHRU4_cnst;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_INTCONST;
static NBG_emit_action_f_t emit_rule_action_LDID_PREG;
static NBG_emit_action_f_t emit_rule_action_LDID;
static NBG_emit_action_f_t emit_rule_action_ILOAD_LDA;
static NBG_emit_action_f_t emit_rule_action_ILOAD;
static NBG_emit_action_f_t emit_rule_action_LDID_PREG;
static NBG_emit_action_f_t emit_rule_action_LDID;
static NBG_emit_action_f_t emit_rule_action_ILOAD_LDA;
static NBG_emit_action_f_t emit_rule_action_ILOAD;
static NBG_emit_action_f_t emit_rule_action_LDID_PREG;
static NBG_emit_action_f_t emit_rule_action_LDID;
static NBG_emit_action_f_t emit_rule_action_ILOAD_LDA;
static NBG_emit_action_f_t emit_rule_action_ILOAD;
static NBG_emit_action_f_t emit_rule_action_LDID_PREG;
static NBG_emit_action_f_t emit_rule_action_LDID;
static NBG_emit_action_f_t emit_rule_action_ILOAD_LDA;
static NBG_emit_action_f_t emit_rule_action_ILOAD;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_chain;
static NBG_emit_action_f_t emit_rule_action_LDID_PREG;

static NBG_emit_action_f_t * const NBG_emit_actions_ [] = {
  (NBG_emit_action_f_t *)0,
  &emit_rule_action_noaction,
  &emit_rule_action_chain,
  &emit_rule_action_chain,
  &emit_rule_action_cnsti4,
  &emit_rule_action_STID_PREG,
  &emit_rule_action_STID,
  &emit_rule_action_stmt_ISTORE_reg_LDA_,
  &emit_rule_action_stmt_ISTORE_reg_reg_,
  &emit_rule_action_stmt_ISTOREX_reg_reg_reg_,
  &emit_rule_action_stmt_STBITS___CLASS__PREG_reg_,
  &emit_rule_action_stmt_STBITS_reg_,
  &emit_rule_action_stmt_ISTBITS_reg_LDA_,
  &emit_rule_action_stmt_ISTBITS_reg_reg_,
  &emit_rule_action_LDID_PREG,
  &emit_rule_action_LDID,
  &emit_rule_action_ILOAD_LDA,
  &emit_rule_action_ILOAD,
  &emit_rule_action_reg_LDA,
  &emit_rule_action_reg_ILOADX_reg_reg_,
  &emit_rule_action_reg_LDBITS___CLASS__PREG,
  &emit_rule_action_reg_LDBITS,
  &emit_rule_action_reg_ILDBITS_LDA_,
  &emit_rule_action_reg_ILDBITS_reg_,
  &emit_rule_action_INTCONST_0,
  &emit_rule_action_INTCONST,
  &emit_rule_action_CONST,
  &emit_rule_action_ADD,
  &emit_rule_action_ADD,
  &emit_rule_action_SUB,
  &emit_rule_action_SUB,
  &emit_rule_action_NEG,
  &emit_rule_action_ASHR,
  &emit_rule_action_LSHR,
  &emit_rule_action_SHL,
  &emit_rule_action_MPY32,
  &emit_rule_action_MPY32,
  &emit_rule_action_DIV_CNST,
  &emit_rule_action_REM_CNST,
  &emit_rule_action_DIV,
  &emit_rule_action_REM,
  &emit_rule_action_MOD,
  &emit_rule_action_ABS,
  &emit_rule_action_MAX,
  &emit_rule_action_MAX,
  &emit_rule_action_MIN,
  &emit_rule_action_MIN,
  &emit_rule_action_reg_MADD_reg_reg_reg_,
  &emit_rule_action_reg_MSUB_reg_reg_reg_,
  &emit_rule_action_reg_NMADD_reg_reg_reg_,
  &emit_rule_action_reg_NMSUB_reg_reg_reg_,
  &emit_rule_action_BNOT,
  &emit_rule_action_BAND,
  &emit_rule_action_BAND,
  &emit_rule_action_BIOR,
  &emit_rule_action_BIOR,
  &emit_rule_action_BXOR,
  &emit_rule_action_BXOR,
  &emit_rule_action_BNOR,
  &emit_rule_action_BNOR,
  &emit_rule_action_LNOT,
  &emit_rule_action_LAND,
  &emit_rule_action_LAND,
  &emit_rule_action_LIOR,
  &emit_rule_action_LIOR,
  &emit_rule_action_SELECT_cond,
  &emit_rule_action_SELECT_cond,
  &emit_rule_action_SELECT_reg,
  &emit_rule_action_noaction,
  &emit_rule_action_noaction,
  &emit_rule_action_noaction,
  &emit_rule_action_noaction,
  &emit_rule_action_noaction,
  &emit_rule_action_noaction,
  &emit_rule_action_noaction,
  &emit_rule_action_noaction,
  &emit_rule_action_noaction,
  &emit_rule_action_noaction,
  &emit_rule_action_noaction,
  &emit_rule_action_noaction,
  &emit_rule_action_COMPARE_INT,
  &emit_rule_action_COMPARE_INT,
  &emit_rule_action_COMPARE_INT,
  &emit_rule_action_COMPARE_INT,
  &emit_rule_action_COMPARE_INT,
  &emit_rule_action_COMPARE_INT,
  &emit_rule_action_COMPARE_INT,
  &emit_rule_action_COMPARE_INT,
  &emit_rule_action_COMPARE_INT,
  &emit_rule_action_COMPARE_INT,
  &emit_rule_action_COMPARE_INT,
  &emit_rule_action_COMPARE_INT,
  &emit_rule_action_reg_SQRT_reg_,
  &emit_rule_action_reg_RSQRT_reg_,
  &emit_rule_action_reg_RECIP_reg_,
  &emit_rule_action_reg_RND_reg_,
  &emit_rule_action_reg_TRUNC_reg_,
  &emit_rule_action_reg_CEIL_reg_,
  &emit_rule_action_reg_FLOOR_reg_,
  &emit_rule_action_CVTLI4_16,
  &emit_rule_action_CVTLU4_16,
  &emit_rule_action_CVTLI4_8,
  &emit_rule_action_CVTLU4_8,
  &emit_rule_action_reg_CVTL_reg_,
  &emit_rule_action_reg_CVT_reg_,
  &emit_rule_action_chain,
  &emit_rule_action_chain,
  &emit_rule_action_reg_EXTRACT__BITS_reg_,
  &emit_rule_action_GOTO,
  &emit_rule_action_GOTO,
  &emit_rule_action_AGOTO,
  &emit_rule_action_XGOTO,
  &emit_rule_action_CONDBR_reg,
  &emit_rule_action_CONDBR_reg,
  &emit_rule_action_CONDBR_cond,
  &emit_rule_action_CONDBR_cond,
  &emit_rule_action_CONDBR_const,
  &emit_rule_action_CONDBR_const,
  &emit_rule_action_chain,
  &emit_rule_action_CALL,
  &emit_rule_action_CALL,
  &emit_rule_action_CALL,
  &emit_rule_action_CALL,
  &emit_rule_action_RETURN,
  &emit_rule_action_CALL,
  &emit_rule_action_CALL,
  &emit_rule_action_reg_LOWPART_reg_,
  &emit_rule_action_reg_HIGHPART_reg_,
  &emit_rule_action_reg_MINPART_reg_,
  &emit_rule_action_reg_MAXPART_reg_,
  &emit_rule_action_reg_COMPOSE__BITS_reg_reg_,
  &emit_rule_action_reg_HIGHMPY_reg_reg_,
  &emit_rule_action_reg_XMPY_reg_reg_,
  &emit_rule_action_reg_DIVREM_reg_reg_,
  &emit_rule_action_reg_MINMAX_reg_reg_,
  &emit_rule_action_reg_LDMA,
  &emit_rule_action_reg_LDA__LABEL,
  &emit_rule_action_stmt_IDNAME,
  &emit_rule_action_stmt_MSTORE_reg_reg_reg_,
  &emit_rule_action_reg_MLOAD_reg_reg_,
  &emit_rule_action_inv_FIRSTPART_inv_,
  &emit_rule_action_inv_SECONDPART_inv_,
  &emit_rule_action_inv_CAND_inv_inv_,
  &emit_rule_action_inv_CIOR_inv_inv_,
  &emit_rule_action_inv_CSELECT_inv_inv_,
  &emit_rule_action_inv_ILDA_inv_,
  &emit_rule_action_inv_PAIR_inv_inv_,
  &emit_rule_action_inv_RROTATE_inv_inv_,
  &emit_rule_action_inv_COMMA_inv_inv_,
  &emit_rule_action_inv_RCOMMA_inv_inv_,
  &emit_rule_action_inv_ARRAY_inv_,
  &emit_rule_action_inv_INTRINSIC__OP_inv_,
  &emit_rule_action_inv_IO__ITEM_inv_,
  &emit_rule_action_inv_TRIPLET_inv_,
  &emit_rule_action_inv_ARRAYEXP_inv_,
  &emit_rule_action_inv_ARRSECTION_inv_,
  &emit_rule_action_inv_WHERE_inv_,
  &emit_rule_action_stmt_FORWARD__BARRIER,
  &emit_rule_action_stmt_BACKWARD__BARRIER,
  &emit_rule_action_stmt_INTRINSIC__CALL_V,
  &emit_rule_action_reg_INTRINSIC__CALL,
  &emit_rule_action_chain,
  &emit_rule_action_cnsti4,
  &emit_rule_action_cnsti4,
  &emit_rule_action_cnsti4,
  &emit_rule_action_cnsti4,
  &emit_rule_action_cnsti4,
  &emit_rule_action_chain,
  &emit_rule_action_cnsti4,
  &emit_rule_action_cnsti4,
  &emit_rule_action_chain,
  &emit_rule_action_cnsti4,
  &emit_rule_action_cnsti4,
  &emit_rule_action_cnsti4,
  &emit_rule_action_cnsti4,
  &emit_rule_action_cnsti4,
  &emit_rule_action_cnsti4,
  &emit_rule_action_cnsti4,
  &emit_rule_action_cnsti4,
  &emit_rule_action_cnsti4,
  &emit_rule_action_cnsti4,
  &emit_rule_action_MPY_PW2,
  &emit_rule_action_MPY_PW2,
  &emit_rule_action_MPY_PW2,
  &emit_rule_action_MPY_PW2,
  &emit_rule_action_chain,
  &emit_rule_action_chain,
  &emit_rule_action_chain,
  &emit_rule_action_chain,
  &emit_rule_action_chain,
  &emit_rule_action_chain,
  &emit_rule_action_chain,
  &emit_rule_action_chain,
  &emit_rule_action_SH1ADD,
  &emit_rule_action_SH2ADD,
  &emit_rule_action_SH3ADD,
  &emit_rule_action_SH4ADD,
  &emit_rule_action_MULHS,
  &emit_rule_action_MULHS_2,
  &emit_rule_action_MULHH,
  &emit_rule_action_MULHHU,
  &emit_rule_action_MULLH,
  &emit_rule_action_MULLH_2,
  &emit_rule_action_MULLHU,
  &emit_rule_action_MULLHU_2,
  &emit_rule_action_MULH,
  &emit_rule_action_MULH_2,
  &emit_rule_action_MULHU,
  &emit_rule_action_MULHU_2,
  &emit_rule_action_MULLL,
  &emit_rule_action_MULLL,
  &emit_rule_action_MULLL_2,
  &emit_rule_action_MULLLU,
  &emit_rule_action_MULLL,
  &emit_rule_action_MULLLU_2,
  &emit_rule_action_MULL,
  &emit_rule_action_MULL,
  &emit_rule_action_MULL_2,
  &emit_rule_action_MULL_2,
  &emit_rule_action_MULLU,
  &emit_rule_action_MULLU,
  &emit_rule_action_MULLU_2,
  &emit_rule_action_MULLU_2,
  &emit_rule_action_chain,
  &emit_rule_action_chain,
  &emit_rule_action_chain,
  &emit_rule_action_chain,
  &emit_rule_action_chain,
  &emit_rule_action_chain,
  &emit_rule_action_chain,
  &emit_rule_action_chain,
  &emit_rule_action_chain,
  &emit_rule_action_chain,
  &emit_rule_action_chain,
  &emit_rule_action_chain,
  &emit_rule_action_chain,
  &emit_rule_action_chain,
  &emit_rule_action_chain,
  &emit_rule_action_chain,
  &emit_rule_action_chain,
  &emit_rule_action_chain,
  &emit_rule_action_chain,
  &emit_rule_action_chain,
  &emit_rule_action_chain,
  &emit_rule_action_chain,
  &emit_rule_action_chain,
  &emit_rule_action_chain,
  &emit_rule_action_chain,
  &emit_rule_action_chain,
  &emit_rule_action_chain,
  &emit_rule_action_chain,
  &emit_rule_action_chain,
  &emit_rule_action_chain,
  &emit_rule_action_chain,
  &emit_rule_action_chain,
  &emit_rule_action_chain,
  &emit_rule_action_CVTLI4_8,
  &emit_rule_action_SHRI4_cnst,
  &emit_rule_action_SHRI4_cnst,
  &emit_rule_action_chain,
  &emit_rule_action_chain,
  &emit_rule_action_INTCONST,
  &emit_rule_action_chain,
  &emit_rule_action_CVTLU4_8,
  &emit_rule_action_CVTLU4_8,
  &emit_rule_action_SHRU4_cnst,
  &emit_rule_action_SHRU4_cnst,
  &emit_rule_action_SHRI4_cnst,
  &emit_rule_action_chain,
  &emit_rule_action_chain,
  &emit_rule_action_chain,
  &emit_rule_action_chain,
  &emit_rule_action_INTCONST,
  &emit_rule_action_chain,
  &emit_rule_action_chain,
  &emit_rule_action_chain,
  &emit_rule_action_CVTLI4_16,
  &emit_rule_action_CVTLI4_16,
  &emit_rule_action_SHRI4_cnst,
  &emit_rule_action_chain,
  &emit_rule_action_chain,
  &emit_rule_action_INTCONST,
  &emit_rule_action_chain,
  &emit_rule_action_chain,
  &emit_rule_action_CVTLU4_16,
  &emit_rule_action_CVTLU4_16,
  &emit_rule_action_CVTLU4_16,
  &emit_rule_action_SHRU4_cnst,
  &emit_rule_action_chain,
  &emit_rule_action_chain,
  &emit_rule_action_chain,
  &emit_rule_action_INTCONST,
  &emit_rule_action_LDID_PREG,
  &emit_rule_action_LDID,
  &emit_rule_action_ILOAD_LDA,
  &emit_rule_action_ILOAD,
  &emit_rule_action_LDID_PREG,
  &emit_rule_action_LDID,
  &emit_rule_action_ILOAD_LDA,
  &emit_rule_action_ILOAD,
  &emit_rule_action_LDID_PREG,
  &emit_rule_action_LDID,
  &emit_rule_action_ILOAD_LDA,
  &emit_rule_action_ILOAD,
  &emit_rule_action_LDID_PREG,
  &emit_rule_action_LDID,
  &emit_rule_action_ILOAD_LDA,
  &emit_rule_action_ILOAD,
  &emit_rule_action_chain,
  &emit_rule_action_chain,
  &emit_rule_action_chain,
  &emit_rule_action_chain,
  &emit_rule_action_chain,
  &emit_rule_action_LDID_PREG,
  
};
# 49 "NBG_burs_template.c"


static void
findent(int indent, FILE *file)
{
  int i;
  for (i = 0; i < indent; i++) {
    fprintf(file, " ");
  }
}

static void
NBG_labelled_self_print_indented(NBG_State state, int indent, FILE *file)
{
  int i;
  NBG_ASSERT(state != NULL);
  findent(indent, file);
  fprintf(file, "%s(%d)\n", NBG_TREE_OP_STRING(NBG_State_op(state)), NBG_State_op(state));
  for (i = 0; i < NBG_MAX_NTERMS; i++) {
    NBG_Rule r;
    if ((r = state->rule[i]) != NBG_RULE_NUL) {
      findent(indent+1, file);
      fprintf(file, "(rule %d, cost %d): %s\n", r, state->cost[i], NBG_Rule_string(r));
    }
  }
}

static void
NBG_labelled_print_indented(NBG_State state, int indent, FILE *file)
{
  int i;
  NBG_ASSERT(state != NULL);
  NBG_labelled_self_print_indented(state, indent, file);
  if (state->arity > 0) {
    findent(indent, file);
    fprintf(file, "[\n");
    for (i = 0; i < state->arity; i++) {
      NBG_labelled_print_indented(state->children[i], indent+2, file);
    }
    findent(indent, file);
    fprintf(file, "]\n");
  }
  if (state->reuse != NULL) {
    findent(indent+1, file);
    fprintf(file, " REUSE \n");
    NBG_labelled_print_indented(state->reuse, indent+1, file);
  }
}

void
NBG_labelled_print(NBG_State state, FILE *file)
{
  NBG_labelled_print_indented(state, 0, file);
}

static void
NBG_linked_self_print_indented(NBG_Link link, int indent, FILE *file)
{
  NBG_ASSERT(link != NULL);
  findent(indent, file);
  fprintf(file, "%s(%d): rule %d: %s", NBG_TREE_OP_STRING(NBG_State_op(link->state)), NBG_State_op(link->state),
	  link->rule, NBG_Rule_string(link->rule));
}

static void
NBG_linked_print_indented(NBG_Link link, int indent, FILE *file)
{
  int i;
  NBG_ASSERT(link != NULL);
  NBG_linked_self_print_indented(link, indent, file);
  if (NBG_Link_nkids(link) > 0) {
    findent(indent, file);
    fprintf(file, "[\n");
    for (i = 0; i < NBG_Link_nkids(link); i++) {
      NBG_linked_print_indented(NBG_Link_kid(link, i), indent+2, file);
    }
    findent(indent, file);
    fprintf(file, "]\n");
  }
}

void
NBG_linked_print(NBG_Link link, FILE *file)
{
  NBG_linked_print_indented(link, 0, file);
}


static NBG_State NBG_state_list = NULL;
static NBG_State NBG_state_last = NULL;

void
NBG_del_state_list(void)
{
  if (NBG_state_list != NULL) {
    NBG_State_delnext(NBG_state_list);
    NBG_state_list = NULL;
    NBG_state_last = NULL;
  }
}

static void
NBG_add_state_list(NBG_State state)
{
  NBG_ASSERT(state->next == NULL);
  if (NBG_state_list == NULL) {
    NBG_ASSERT(NBG_state_last == NULL);
    NBG_state_list = state;
    NBG_state_last = state;
  } else {
    NBG_ASSERT(NBG_state_last != NULL);
    NBG_state_last->next = state;
    NBG_state_last = state;
  }
}

NBG_State
NBG_label(void * p, void *user_state)
{
  NBG_ASSERT(p != NULL);
  {
    NBG_TREE tree = (NBG_TREE)p;
    NBG_State return_state = NULL;
    int arity = NBG_TREE_ARITY(tree);
    NBG_Op op = NBG_TREE_OP(tree);
    int i;
    
    return_state = NBG_State_new(op, arity);
    NBG_add_state_list(return_state);
    NBG_State__utree(return_state, tree);
    for (i = 0; i < arity; i++) {
      NBG_State child = NBG_label(NBG_TREE_KID_AT(tree, i), user_state);
      NBG_State__parent(child, return_state);
      NBG_State__child(return_state, i, child);
    }
    NBG_compute_state(return_state, user_state);
    return return_state;
  }
}

#ifdef NBG_LABEL_UNCSE

static NBG_State
NBG_merge_load_cse(NBG_State load_state, NBG_State cse_state)
{
  NBG_State merge_state;
  int arity;
  int i;

  arity = NBG_State_arity(cse_state);
  merge_state = NBG_State_new(NBG_State_op(cse_state), arity);
  NBG_add_state_list(merge_state);
  NBG_State__utree(merge_state, NBG_State_utree(cse_state));
  for (i = 0; i < arity; i++) {
    NBG_State__child(merge_state, i,
		     NBG_State_child(cse_state, i));
  }
  NBG_ASSERT(NBG_State_rule(load_state, NBG_REUSE_NTERM) != 0);
  NBG_State__rule(merge_state, NBG_REUSE_NTERM, 
		  NBG_State_rule(load_state, NBG_REUSE_NTERM));
  NBG_State__cost(merge_state, NBG_REUSE_NTERM, 
		  NBG_State_cost(load_state, NBG_REUSE_NTERM));
  NBG_compute_closure(merge_state, NBG_REUSE_NTERM);
  
  /* We copy nterms from reuse state to merge state, unless
     cost is greater than without reuse. This as the effect
     at the same cost of favorizing reuse of expressions. */
  for (i = 1; i < NTERM(COUNT); i++) {
    int rule;
    int rule_2;
    if (i == NBG_REUSE_NTERM) continue;
    if ((rule = NBG_State_rule(cse_state, i))) {
      if ((rule_2 = NBG_State_rule(merge_state, i)) == 0 ||
	  NBG_State_cost(merge_state, i) >= NBG_State_cost(cse_state, i)) {
	NBG_State__rule(merge_state, i, rule);
	NBG_State__cost(merge_state, i, NBG_State_cost(cse_state, i));
      }
    } else {
      NBG_State__rule(merge_state, i, 0);
      NBG_State__cost(merge_state, i, 0);
    }
  }
  NBG_State__reuse(merge_state, load_state);
  NBG_State__reuse(load_state, cse_state);
  return merge_state;
}

static NBG_State
NBG_label_uncse_(void * p, int level, void *user_state)
{
  NBG_ASSERT(p != NULL);
  {
    NBG_TREE tree = (NBG_TREE)p;
    NBG_State return_state = NULL;
    int arity = NBG_TREE_ARITY(tree);
    NBG_Op op = NBG_TREE_OP(tree);
    int i;

    if (IS_USE((NBG_TREE)p)) {
      NBG_TREE def;
      NBG_State cse_state = NULL, load_state = NULL;
      NBG_ASSERT(arity == 0);
      if (level == 0) SET_USE((NBG_TREE)p);
      def = USE_DEF((NBG_TREE)p);
      if (def != NULL && !IS_CLOBBER(def)) {
	NBG_TREE cse = DEF_EXPR(def);
	cse_state = NBG_label_uncse_(cse, level+1, user_state);
      }
      if (LAST_DEF((NBG_TREE)p) == def) {
	NBG_reuse = (cse_state != NULL);
	load_state = NBG_label(tree, NULL);
	NBG_reuse = 0;
      }
      if (cse_state == NULL) return load_state;
      if (load_state == NULL) return cse_state;
      return NBG_merge_load_cse(load_state, cse_state);
    } else {
      return_state = NBG_State_new(op, arity);
      NBG_add_state_list(return_state);
      NBG_State__utree(return_state, tree);
      for (i = 0; i < arity; i++) {
	NBG_State child = NBG_label_uncse_(NBG_TREE_KID_AT(tree, i), level, user_state);
	if (child == NULL) return NULL;
	NBG_State__parent(child, return_state);
	NBG_State__child(return_state, i, child);
      }
      NBG_compute_state(return_state, user_state);

      if (level == 0) {
	if (IS_DEF((NBG_TREE)p)) {
	  SET_DEF((NBG_TREE)p);
	}
	if (IS_CLOBBER((NBG_TREE)p)) {
	  SET_CLOBBER((NBG_TREE)p);
	}
      }
      return return_state;
    }
  }
}

NBG_State
NBG_label_uncse(void * p, void *user_state)
{
  NBG_State return_state;
  return_state = NBG_label_uncse_(p, 0, user_state);
  NBG_ASSERT(return_state != NULL);
  return return_state;
}
#endif

static NBG_State
NBG_State_get_kid(NBG_State s, NBG_Rule rule, int i)
{
  const char *path = NBG_Rule_kid_path(rule, i);
  const char *p;
  NBG_State kid = s;
  for (p = path; *p != (char)-1; p++) {
    kid = NBG_State_child(kid, (int)*p);
  }
  return kid;

}

static NBG_Link
NBG_link_nterm(NBG_State s, int nterm)
{
  NBG_Rule rule;
  int nkids;
  const NBG_NTerm *nterms;
  int i;
  NBG_Link link;

  rule = NBG_State_rule(s, nterm);
  NBG_ASSERT(rule != NBG_RULE_NUL);
  nkids = NBG_Rule_nkids(rule);
  nterms = NBG_Rule_nterms(rule);
  link = NBG_Link_new(rule, nkids, NULL);
  link->parent = NULL;
  link->state = s;
  link->xptr = NBG_State_utree(s);
  for (i = 0; i < nkids; i++) {
    NBG_State kid;
    nterm = nterms[i];
    kid = NBG_State_get_kid(s, rule, i);
#ifdef NBG_LABEL_UNCSE
    if (nterm == NBG_REUSE_NTERM) {
      NBG_ASSERT(kid->reuse != NULL);
      link->kids[i] = NBG_link_nterm(kid->reuse, nterm);
    } else
#endif
      {
      link->kids[i] = NBG_link_nterm(kid, nterm);
      }
    link->kids[i]->parent = link;
  }
  return link;
}

int
NBG_matched(NBG_State s)
{
  NBG_Rule rule = NBG_State_rule(s, NBG_Rule_start);
  return rule != NBG_RULE_NUL;
}

NBG_Link
NBG_link(NBG_State s)
{
  NBG_ASSERT(s != NULL);
  {
    return NBG_link_nterm(s, NBG_Rule_start);
  }
}

void
NBG_emit(NBG_Link l, void *state)
{
  int i;
  for (i = 0; i < l->nkids; i++) {
    NBG_emit(l->kids[i], state);
  }
  NBG_emit_action(l->rule)(l, state);
}

void
NBG_visit_bu(NBG_Link l, NBG_visit_f visit, void *state)
{
  int i;
  for (i = 0; i < l->nkids; i++) {
    NBG_visit_bu(l->kids[i], visit, state);
  }
  visit(l, state);
}

void
NBG_visit_td(NBG_Link l, NBG_visit_f visit, void *state)
{
  int i;
  visit(l, state);
  for (i = 0; i < l->nkids; i++) {
    NBG_visit_td(l->kids[i], visit, state);
  }
}

void
NBG_rules_print(FILE *file)
{
  int i;
  for(i = 1; i <= NBG_Rules_num; i++) {
    fprintf(file, "%s\n", NBG_Rule_string(i));
  }
}

#ifdef __cplusplus
}
#endif

# 1 "./models/pro_st220.act"

/*
 * Template functions for implementation of the emit phase.
 * Each of the template must be completed according
 * to the matched rule.
 * The emit phase processes the link tree bottom-up.
 * Children in the link tree correspond to
 * already processed non terminals in the rule.
 * The argument passed is the current link tree.
 * Available methods are:
 * NBG_Link_nkids(link): number of kids.
 * NBG_Link_kid(link, i): get kid i of link tree.
 * NBG_Link_parent(link): get parent (or NULL for root).
 * NBG_Link_utree(link): get pointer to IR tree.
 */

#ifdef __cplusplus
extern "C" {
#endif
/*
 * INT_MAP declaration
 */
typedef struct INT_MAP INT_MAP;
static void INT_MAP_clear(INT_MAP *map);
static void *INT_MAP_unset(INT_MAP *map, int idx);
static void *INT_MAP_set(INT_MAP *map, int idx);
static void *INT_MAP_get(INT_MAP *map, int idx);
static void *INT_MAP_access(INT_MAP *map, int idx);
static void *INT_MAP_append(INT_MAP *map);
static void INT_MAP_ralloc(INT_MAP *map, int new_size);
static int INT_MAP_size(INT_MAP *map);
static int INT_MAP_count(INT_MAP *map);
static INT_MAP *INT_MAP_del(INT_MAP *map);
static INT_MAP *INT_MAP_new(size_t elt_size, int hint);

/*
 * INT_INT_MAP
 */
#define INT_INT_MAP_new(hint) INT_MAP_new(sizeof(int), (hint))
#define INT_INT_MAP_del(map) INT_MAP_del(map)
#define INT_INT_MAP_clear(map) INT_MAP_clear(map)
#define INT_INT_MAP_set(map, num, val) (*((int *)INT_MAP_set((map), (num))) = (val))
#define INT_INT_MAP_get(map, num) (*((int *)INT_MAP_get((map), (num))))
#define INT_INT_MAP_append(map, val) (*((int *)INT_MAP_append(map)) = (val))

/*
 * INT_PTR_MAP
 */
#define INT_PTR_MAP_new(hint) INT_MAP_new(sizeof(void *), (hint))
#define INT_PTR_MAP_del(map) INT_MAP_del(map)
#define INT_PTR_MAP_clear(map) INT_MAP_clear(map)
#define INT_PTR_MAP_set(map, num, val) (*((void **)INT_MAP_set((map), (num))) = (val))
#define INT_PTR_MAP_get(map, num) (*((void **)INT_MAP_get((map), (num))))
#define INT_PTR_MAP_append(map, val) (*((void **)INT_MAP_append(map)) = (val))

#ifdef __cplusplus
}
#endif

#ifdef TARG_ST
#include "defs.h"
#include "mempool.h"
#include "wn.h"
#include "symtab.h"
#include "const.h"
#include "erbe.h"
#include "erglob.h"
#include "tracing.h"
#include "glob.h"
#include "config.h"
#include "config_TARG.h"
#include "topcode.h"
#include "targ_isa_lits.h"
#include "intrn_info.h"
#include "opcode.h"
#include "w2op.h"
#include "wn_util.h"
#include "ir_reader.h"
#include "cgir.h"
#include "region_util.h"
#include "cg_region.h"
#include "cg.h"
#include "calls.h"
#include "cgtarget.h"
#include "stblock.h"
#include "targ_sim.h"
#include "irbdata.h"
#include "ttype.h"
#include "pf_cg.h"
#include "wn_map.h"
#include "gtn_universe.h"
#include "variants.h"
#include "targ_sim.h"
#include "eh_region.h"
#include "fb_whirl.h"
#include "xstats.h"
#include "data_layout.h"
#include "label_util.h"
#include "comment.h"
#include "be_symtab.h"
#include "be_util.h"
#include "config_asm.h"
#include "exp_targ.h"

/* exported from exp_targ. */
extern void Exp_Lda (TYPE_ID mtype, TN *tgt_tn, ST *sym, INT64 ofst, OPERATOR call_opr, OPS *ops);
extern void Exp_COPY (TN *dst, TN *src, OPS *ops);
extern void Exp_Load (TYPE_ID rtype, TYPE_ID desc, TN *tgt_tn, ST *sym, INT64 ofst, OPS *ops, VARIANT variant);
extern void Exp_Store (TYPE_ID mtype, TN *src_tn, ST *sym, INT64 ofst, OPS *ops, VARIANT variant);
extern void Expand_Select (TN *dest_tn, TN *cond_tn, TN *true_tn, TN *false_tn, 
			   TYPE_ID mtype, BOOL float_cond, OPS *ops);
extern void Exp_Select_And_Condition (OPCODE select, TN *result, TN *true_tn, TN *false_tn, OPCODE compare,
				     TN *cmp_kid1, TN *cmp_kid2, VARIANT variant, OPS *ops);
extern void Exp_Extract_Bits (TYPE_ID rtype, TYPE_ID desc,
			      UINT bit_offset, UINT bit_size, TN *tgt_tn, TN *src_tn, OPS *ops);
extern void Exp_Deposit_Bits (TYPE_ID rtype, TYPE_ID desc, UINT bit_offset, UINT bit_size, 
			      TN *tgt_tn, TN *src1_tn, TN *src2_tn, OPS *ops);
extern void Exp_Indirect_Branch (TN *targ_reg, OPS *ops);

/* exported vars from whirl2ops.cxx. */
extern SRCPOS current_srcpos;
extern OP *Last_Mem_OP;
extern BOOL In_Glue_Region;
extern BB *Cur_BB;

/* exported fron whirl2ops.cxx. */
extern void Set_OP_To_WN_Map(WN *wn);
extern void TN_CORRESPOND_Free(void);
extern TN *TN_CORRESPOND_Lookup(TN *result);
extern TN *TN_CORRESPOND_Get (TN *result, WN *expr);

/* from whirl2ops.h. */
extern TN **PREG_To_TN_Array;
extern TYPE_ID *PREG_To_TN_Mtype;
extern TN *Get_Complement_TN(TN *tn);
extern TN *PREG_To_TN (ST *preg_st, PREG_NUM preg_num, WN *wn);
extern void PREG_To_TN_Clear (void);
extern PREG_NUM TN_To_PREG (TN *tn);
extern PREG_NUM TN_To_Assigned_PREG (TN *tn);
extern LABEL_IDX Get_WN_Label (WN *wn);

extern PREG_NUM Find_PREG_For_Symbol (const ST *st);

extern WN * Preg_Is_Rematerializable(PREG_NUM preg, BOOL *gra_homeable);

static void Handle_STID_ (WN *stid, OPCODE opcode, TN *op0, OPS *ops);
static void Handle_STID_PREG_ (WN *stid, OPCODE opcode, TN *op0, OPS *ops);
static void Handle_ISTORE_LDA_ (WN *istore, WN *kid1, OPCODE opcode, TN *op0, OPS *ops);
static void Handle_ISTORE_ (WN *istore, OPCODE opcode, TN *op0, TN *op1, OPS *ops);
static void Handle_STBITS_PREG_ (WN *stbits, WN *kid0, OPCODE opcode, TN *op0, OPS *ops);
static void Handle_STBITS_ (WN *stbits, WN *kid0, OPCODE opcode, TN *op0, OPS *ops);
static void Handle_ISTBITS_LDA_(WN *istbits, WN *kid0, WN *kid1, OPCODE opcode, TN *op0, OPS *ops);
static void Handle_ISTBITS_(WN *istbits, WN *kid0, OPCODE opcode, TN *op0, TN *op1, OPS *ops);

static TN *Handle_LDID_ (WN *ldid, OPCODE opcode, TN *result, OPS *ops);
static TN *Handle_LDID_PREG_ (WN *ldid, OPCODE opcode, TN *result, OPS *ops);
static TN *Handle_ILOAD_LDA_ (WN *iload, WN *lda, OPCODE opcode, TN *result, OPS *ops);
static TN *Handle_ILOAD_ (WN *iload, OPCODE opcode, TN *result, TN *op0, OPS *ops);
static TN *Handle_LDBITS_PREG_(WN *ldbits, OPCODE opcode, TN *result, OPS *ops);
static TN *Handle_LDBITS_(WN *ldbits, OPCODE opcode, TN *result, OPS *ops);
static TN *Handle_ILDBITS_(WN *ildbits, OPCODE opcode, TN *result, TN *op0, OPS *ops);
static TN *Handle_ILDBITS_LDA_(WN *ildbits, WN *lda, OPCODE opcode, TN *result, OPS *ops);

static TN *Allocate_TN_Of_Mtype(TYPE_ID type);

static void Emit_Noop(NBG_Link LINK, void *STATE);
static void Emit_SELECT_(WN *select, WN *compare, TN *result, TN *cond, TN *trueop, TN *falseop, OPS *ops);
static void Emit_SELECT_AND_COND_(WN *select, WN *compare, TN *result, TN *cond_l, TN *cond_r, TN *trueop, TN *falseop, OPS *ops);
static INT16 WHIRL_Compare_To_OP_variant (OPCODE opcode);

#else
#include "../pro/tn.h"
#include "../pro/tiny_op.h"
#include "../pro/tiny_wn.h"
#include "../pro/st200/exp_targ.h"
extern FILE *TFile;
#endif

static TOP TOP_Imm(TOP regform, INT64 imm);
#define TOP_IMM(top, tn) (TN_is_constant(tn) ? TOP_Imm(top, TN_value(tn)) : top)

#if 0
/*
 * PREG_MAP definition
 */
static INT_MAP *preg_state_map;
#define PREG_STATE_MAP_new(hint) INT_MAP_new(sizeof(NBG_State), (hint))
#define PREG_STATE_MAP_del(map) INT_MAP_del(map)
#define PREG_STATE_MAP_clear(map) INT_MAP_clear(map)
#define PREG_STATE_MAP_set(map, num, state) (*((NBG_State *)INT_MAP_set((map), (int)(preg))) = (state))
#define PREG_STATE_MAP_get(map, num) (*((NBG_State *)INT_MAP_get(map, (int)(preg))))
#endif

/*
 * REG REUSE.
 */
static TN *preg_to_tn(WN *tree, PREG_NUM preg);
static PREG_NUM get_preg(WN *tree);
#if 0
static NBG_State get_reuse_state(WN* tree);
static void set_reuse_state(WN* tree, NBG_State state);
static int has_reuse_state(WN* tree);
#endif

/*
 * DU chains.
 */
static void du_init_bb(void);
static void du_init_stmt(void);
static void du_add_use(int preg_use);
static void du_add_def(int preg_def);
static void du_dump(FILE *file);
static void du_stmt_dump(int stmt, FILE *file);
static int du_use_preg(int use);
static int du_use_stmt(int use);
static int du_stmt_last_use(int stmt);
static int du_stmt_first_use(int stmt);
static int du_stmt_def_preg(int stmt);
static int du_stmt_current(void);
static int du_stmt_last(void);
static int du_stmt_first(void);
static int du_preg_last_def(int preg);


/* Information about rule. */
#define RULE_L(link) (NBG_Link_rule(link))
#define RULE	(NBG_Link_rule(LINK))
#define RULE_STR	NBG_Rule_string(RULE)

/* The STATE must contain a pointer to the OP list. */
/* Get pointer to op list. */
#define OPS_PTR	((OPS *)STATE)

/* Get link kids. */
#define LK_L(link,i) NBG_Link_kid(link,i)
#define LK0 NBG_Link_kid(LINK,0)
#define LK1 NBG_Link_kid(LINK,1)
#define LK2 NBG_Link_kid(LINK,2)
#define LK3 NBG_Link_kid(LINK,3)

/* Get WHIRL nodes. */
#define WN_L(link) ((WN *)NBG_Link_utree(link))
#define WN_LK	WN_L(LINK)		/* current */
#define WN_LK0	WN_L(LK0)		/* wn ok link kid. */
#define WN_LK1	WN_L(LK1)
#define WN_LK2	WN_L(LK2)
#define WN_LK3	WN_L(LK3)

/* Get WHIRL node KIDS/PARENTS. */
/* Must use this instead of WN_kid after label pass. */
#define REUSE_L(link) (NBG_State_reuse(STATE_L(link)) == NULL ? NULL : NBG_State_reuse(STATE_L(link)))
#define WN_REUSE_L(link) ((WN *)NBG_State_utree(REUSE_L(link)))
#define STATE_L(link) NBG_Link_state(link)
#define WN_KID0 ((WN *)NBG_State_utree(NBG_State_child(STATE_L(LINK), 0)))
#define WN_KID1 ((WN *)NBG_State_utree(NBG_State_child(STATE_L(LINK), 1)))
#define WN_KID2 ((WN *)NBG_State_utree(NBG_State_child(STATE_L(LINK), 2)))

/* Helpers for pro. */
#define WN_PARENT	(NBG_State_parent(NBG_Link_state(LINK)) == NULL ? NULL: \
			 (WN *)NBG_State_utree(NBG_State_parent(NBG_Link_state(LINK))))
#define WN_RESULT	TN_LK

/* Get infos from whirl node. */
#define OFFSET(wn) WN_offset(wn)
#define ST_IDX(wn) WN_st_idx(wn)
#define OPC(wn) WN_opcode(wn)
#define OPR(wn) WN_operator(wn)
#define DESC(wn) WN_desc(wn)
#define RTYPE(wn) WN_rtype(wn)
#define CONST(wn) WN_const_val(wn)

/* Get TN from link (allocated tn for the node). */
#define TN_L(link)	((TN *)NBG_Link_otree(link))
#define TN_LK	TN_L(LINK)		/* current reg */
#define TN_LK0	TN_L(LK0)		/* tn of link kids */
#define TN_LK1	TN_L(LK1)
#define TN_LK2	TN_L(LK2)
#define TN_LK3	TN_L(LK3)

/* Generate tn. */
#ifdef TARG_ST //PRO
#define GEN_RESULT(type) { if (TN_LK == NULL) TN_LK = ALLOC_REG(type);}
#define ALLOC_REG(type) Allocate_TN_Of_Mtype(type)
#define GEN_CONST(val) Gen_Literal_TN(val, 4)
#else
#define GEN_RESULT(type) { if (TN_LK == NULL)  TN_LK = ALLOC_REG(type); }
#define ALLOC_REG(type) build_tn_of_mtype(type)
#define GEN_CONST(val) gen_constant_tn(val)
#endif

/* 
 * Shortcut for common operators.
 * - Allocate result tn from whirl rtype.
 * - generate operator from result tn and operand tns.
 * - add to op list.
 */
#define MAKE_STMT_0(top) { BUILD_OP_0_0(top, OPS_PTR); }
#define MAKE_STMT_1(top) { BUILD_OP_0_1(top, TN_LK0, OPS_PTR); }
#define MAKE_STMT_2(top) { BUILD_OP_0_2(top, TN_LK0, TN_LK1, OPS_PTR); }
#define MAKE_EXPR_0(top) { GEN_RESULT(RTYPE(WN_LK)); BUILD_OP_1_0(top, TN_LK, OPS_PTR); }
#define MAKE_EXPR_1(top) { GEN_RESULT(RTYPE(WN_LK)); BUILD_OP_1_1(top, TN_LK, TN_LK0, OPS_PTR); }
#define MAKE_EXPR_2(top) { GEN_RESULT(RTYPE(WN_LK)); BUILD_OP_1_2(top, TN_LK, TN_LK0, TN_LK1, OPS_PTR); }
#define MAKE_EXPR_3(top) { GEN_RESULT(RTYPE(WN_LK)); BUILD_OP_1_3(top, TN_LK, TN_LK0, TN_LK1, TN_LK2, OPS_PTR); }

/* This one implements chain like rules by propagating register from kid. */
#ifdef TARG_ST
#define EMIT_CHAIN Emit_Noop(LINK,STATE)
#else
#define EMIT_CHAIN { TN_LK = TN_LK0; }
#endif

#define EMIT_NO_ALLOC  ASSERT_FMT(TN_LK == NULL, ("no allocation, no action"))

/* This one is for unimplemented rule. */
#ifdef TARG_ST //PRO
#define UNIMPLEMENTED ABORT_FMT(("unimplemented rule in emit: %s", RULE_STR))
#else
#define UNIMPLEMENTED PRO_ABORT_FMT(("unimplemented rule in emit: %s", RULE_STR))
#endif

#ifdef TARG_ST //PRO
#define ADD_OP(ops_ptr, op) OPS_Append_Op(ops_ptr, op)
#else
#define ADD_OP(ops_ptr, op) OPS_Append_Op(ops_ptr, op)
#endif

/* Mk_OP like. Allocates an initialize top and operands for a new CG OP. */
#ifdef TARG_ST //PRO
#define MK_OP_1_0(top,res)			Mk_OP(top,res)
#define MK_OP_1_1(top,res,op0)			Mk_OP(top,res,op0)
#define MK_OP_1_2(top,res,op0,op1)		Mk_OP(top,res,op0,op1)
#define MK_OP_1_3(top,res,op0,op1,op2)		Mk_OP(top,res,op0,op1,op2)
#define MK_OP_0_0(top)				Mk_OP(top)
#define MK_OP_0_1(top, op0)			Mk_OP(top, op0)
#define MK_OP_0_2(top, op0, op1)		Mk_OP(top, op0, op1)
#define MK_OP_0_3(top, op0, op1, op2)		Mk_OP(top, op0, op1, op2)
#else
#define OP_NEW(a,b,c,d,e,f,g) OP_new(a,b,c,d,e,f,g)
#define MK_OP_1_0(top,res)			OP_NEW(top,1, 0, res, 0, 0, 0)
#define MK_OP_1_1(top,res,op0)			OP_NEW(top,1, 1, res, op0, 0, 0)
#define MK_OP_1_2(top,res,op0,op1)		OP_NEW(top,1, 2, res, op0, op1, 0)
#define MK_OP_1_3(top,res,op0,op1,op2)		OP_NEW(top,1, 3, res, op0, op1, op2)
#define MK_OP_0_0(top)				OP_NEW(top,0, 0, 0, 0, 0, 0)
#define MK_OP_0_1(top, op0)			OP_NEW(top,0, 1, op0, 0, 0, 0)
#define MK_OP_0_2(top, op0, op1)		OP_NEW(top,0, 2, op0, op1, 0, 0)
#define MK_OP_0_3(top, op0, op1, op2)		OP_NEW(top,0, 3, op0, op1, op2, 0)
#endif

/* Build_OP like. Allocates an initialize top and operands for a new CG OP and add to list. */
#define BUILD_OP_1_0(top,res,ops)		ADD_OP(ops, MK_OP_1_0(top,res))
#define BUILD_OP_1_1(top,res,op0,ops)		ADD_OP(ops, MK_OP_1_1(top,res,op0))
#define BUILD_OP_1_2(top,res,op0,op1,ops)	ADD_OP(ops, MK_OP_1_2(top,res,op0,op1))
#define BUILD_OP_1_3(top,res,op0,op1,op2,ops)	ADD_OP(ops, MK_OP_1_3(top,res,op0,op1,op2))
#define BUILD_OP_0_0(top,ops)			ADD_OP(ops, MK_OP_0_0(top))
#define BUILD_OP_0_1(top,op0,ops)		ADD_OP(ops, MK_OP_0_1(top, op0))
#define BUILD_OP_0_2(top,op0,op1,ops)		ADD_OP(ops, MK_OP_0_2(top, op0, op1))
#define BUILD_OP_0_3(top,op0,op1,op2,ops)	ADD_OP(ops, MK_OP_0_3(top, op0, op1, op2))


static void emit_rule_action_noaction (NBG_Link LINK, void *STATE)
{
  /* no action. Force TN_LK to 0. */
  TN_LK = 0;
}

static void emit_rule_action_chain (NBG_Link LINK, void *STATE)
{
  /* chain like rule. propagate kid register or copy is TN_LK is forced. */
  EMIT_CHAIN;
}

/*RULE: stmt:STID(reg) */
static void emit_rule_action_STID (NBG_Link LINK, void *STATE)
{
#ifdef TARG_ST
  Handle_STID_(WN_LK, OPC(WN_LK), TN_LK0, OPS_PTR);
#else
  Expand_Store(DESC(WN_LK), TN_LK0, SP_TN, GEN_CONST(ST_IDX(WN_LK)), OPS_PTR);
#endif
}

/*RULE: stmt:STID###CLASS_PREG(reg) */
static void emit_rule_action_STID_PREG (NBG_Link LINK, void *STATE)
{
#ifdef TARG_ST
  ASSERT_FMT(TN_LK0 != NULL, ("CGG: PREG of kid of STID is not preallocated"));
  Handle_STID_PREG_(WN_LK, OPC(WN_LK), TN_LK0, OPS_PTR);

#else
  PREG_NUM preg = get_preg(WN_LK);
  TN *tn = preg_to_tn(WN_LK, preg);
  ASSERT_FMT(TN_LK0 != NULL, ("CGG: PREG of kid of STID is not preallocated"));
  ASSERT_FMT(TN_LK0 == tn, ("CGG: PREG of kid of STID is not correct, %d expected %d", TN_idx(TN_LK0), TN_idx(tn)));
#endif
}

/*RULE: stmt:ISTORE(reg,LDA) */
static void emit_rule_action_stmt_ISTORE_reg_LDA_ (NBG_Link LINK, void *STATE)
{
#ifdef TARG_ST
  Handle_ISTORE_LDA_(WN_LK, WN_KID1, OPC(WN_LK), TN_LK0, OPS_PTR);
#else
  UNIMPLEMENTED;
#endif
}


/*RULE: stmt:ISTORE(reg,reg) */
static void emit_rule_action_stmt_ISTORE_reg_reg_ (NBG_Link LINK, void *STATE)
{
#ifdef TARG_ST
  Handle_ISTORE_(WN_LK, OPC(WN_LK), TN_LK0, TN_LK1, OPS_PTR);
#else
  UNIMPLEMENTED;
#endif
}


/*RULE: stmt:ISTOREX(reg,reg,reg) */
static void emit_rule_action_stmt_ISTOREX_reg_reg_reg_ (NBG_Link LINK, void *STATE)
{
  UNIMPLEMENTED;
}


/*RULE: stmt:STBITS###CLASS_PREG(reg) */
static void emit_rule_action_stmt_STBITS___CLASS__PREG_reg_ (NBG_Link LINK, void *STATE)
{
#ifdef TARG_ST
  Handle_STBITS_PREG_(WN_LK, WN_KID0, OPC(WN_LK), TN_LK0, OPS_PTR);
#else
  UNIMPLEMENTED;
#endif
}


/*RULE: stmt:STBITS(reg) */
static void emit_rule_action_stmt_STBITS_reg_ (NBG_Link LINK, void *STATE)
{
#ifdef TARG_ST
  Handle_STBITS_(WN_LK, WN_KID0, OPC(WN_LK), TN_LK0, OPS_PTR);
#else
  UNIMPLEMENTED;
#endif
}


/*RULE: stmt:ISTBITS(reg,LDA) */
static void emit_rule_action_stmt_ISTBITS_reg_LDA_ (NBG_Link LINK, void *STATE)
{
#ifdef TARG_ST
  Handle_ISTBITS_LDA_(WN_LK, WN_KID0, WN_KID1, OPC(WN_LK), TN_LK0, OPS_PTR);
#else
  UNIMPLEMENTED;
#endif
}

/*RULE: stmt:ISTBITS(reg,reg) */
static void emit_rule_action_stmt_ISTBITS_reg_reg_ (NBG_Link LINK, void *STATE)
{
#ifdef TARG_ST
  Handle_ISTBITS_(WN_LK, WN_KID0, OPC(WN_LK), TN_LK0, TN_LK1, OPS_PTR);
#else
  UNIMPLEMENTED;
#endif
}


/*RULE: reg:LDID###CLASS_PREG */
static void emit_rule_action_LDID_PREG (NBG_Link LINK, void *STATE)
{
#ifdef TARG_ST
  ASSERT_FMT(WN_class(WN_LK) == CLASS_PREG, ("preg class was expected in LDID in rule: %s", RULE_STR));
  TN_LK = Handle_LDID_PREG_(WN_LK, OPC(WN_LK), TN_LK, OPS_PTR);
#else
  int preg = get_preg(WN_LK);
  TN *src = preg_to_tn(WN_LK, preg);
  if (TN_LK == NULL) {
    TN_LK = src;
  } else {
    BUILD_OP_1_1(TOP_mov_r, TN_LK, src, OPS_PTR);
  }
  /*if (CGG_LEVEL >=2) {
    du_add_use(preg);
    }*/
#endif
}



/*RULE: reg:LDID */
static void emit_rule_action_LDID (NBG_Link LINK, void *STATE)
{
#ifdef TARG_ST
  ASSERT_FMT(WN_class(WN_LK) != CLASS_PREG, ("unexpected preg class in LDID in rule: %s", RULE_STR));

  GEN_RESULT(RTYPE(WN_LK));
  TN_LK = Handle_LDID_(WN_LK, OPC(WN_LK), TN_LK, OPS_PTR);
#else
  GEN_RESULT(RTYPE(WN_LK));
  Expand_Load(OPC(WN_LK), TN_LK, SP_TN, GEN_CONST(ST_IDX(WN_LK)), OPS_PTR);
#endif
}


/*RULE: reg:ILOAD(LDA) */
static void emit_rule_action_ILOAD_LDA (NBG_Link LINK, void *STATE)
{
#ifdef TARG_ST
  GEN_RESULT(RTYPE(WN_LK));
  TN_LK = Handle_ILOAD_LDA_(WN_LK, WN_KID0, OPC(WN_LK), TN_LK, OPS_PTR);
#else
  UNIMPLEMENTED;
#endif
}


/*RULE: reg:ILOAD(reg) */
static void emit_rule_action_ILOAD (NBG_Link LINK, void *STATE)
{
#ifdef TARG_ST
  GEN_RESULT(RTYPE(WN_LK));
  TN_LK = Handle_ILOAD_(WN_LK, OPC(WN_LK), TN_LK, TN_LK0, OPS_PTR);
#else
  GEN_RESULT(RTYPE(WN_LK));
  Expand_Load(OPC(WN_LK), TN_LK, TN_LK0, GEN_CONST(OFFSET(WN_LK)), OPS_PTR);
#endif
}

/*RULE: reg:ILOADX(reg,reg) */
static void emit_rule_action_reg_ILOADX_reg_reg_ (NBG_Link LINK, void *STATE)
{
  UNIMPLEMENTED;
}


/*RULE: reg:LDBITS###CLASS_PREG */
static void emit_rule_action_reg_LDBITS___CLASS__PREG (NBG_Link LINK, void *STATE)
{
#ifdef TARG_ST
  GEN_RESULT(RTYPE(WN_LK));
  Handle_LDBITS_PREG_(WN_LK, OPC(WN_LK), TN_LK, OPS_PTR);
#else
  UNIMPLEMENTED;
#endif
}


/*RULE: reg:LDBITS */
static void emit_rule_action_reg_LDBITS (NBG_Link LINK, void *STATE)
{
#ifdef TARG_ST
  GEN_RESULT(RTYPE(WN_LK));
  TN_LK = Handle_LDBITS_(WN_LK, OPC(WN_LK), TN_LK, OPS_PTR);
#else
  UNIMPLEMENTED;
#endif
}

/*RULE: reg:ILDBITS(LDA) */
static void emit_rule_action_reg_ILDBITS_LDA_ (NBG_Link LINK, void *STATE)
{
#ifdef TARG_ST
  GEN_RESULT(RTYPE(WN_LK));
  TN_LK = Handle_ILDBITS_LDA_(WN_LK, WN_KID0, OPC(WN_LK), TN_LK, OPS_PTR);
#else
  UNIMPLEMENTED;
#endif
}


/*RULE: reg:ILDBITS(reg) */
static void emit_rule_action_reg_ILDBITS_reg_ (NBG_Link LINK, void *STATE)
{
#ifdef TARG_ST
  GEN_RESULT(RTYPE(WN_LK));
  TN_LK = Handle_ILDBITS_(WN_LK, OPC(WN_LK), TN_LK, TN_LK0, OPS_PTR);
#else
  UNIMPLEMENTED;
#endif
}


/*RULE: cnsti4:INTCONST... */
static void emit_rule_action_cnsti4 (NBG_Link LINK, void *STATE)
{
#ifdef TARG_ST
  /* ref: whirl2ops.cxx: Expand_Expr/case INTCONST. */
  TYPE_ID rtype = RTYPE(WN_LK);
  TN *op0;
  ASSERT_FMT(rtype == MTYPE_I4 || rtype == MTYPE_U4 || rtype == MTYPE_A4, 
	     ("unexpected rtype MTYPE %s in rule: %s", MTYPE_name(rtype), RULE_STR));
  ASSERT_FMT(TN_LK == NULL, ("unexpected preallocation in rule: %s", RULE_STR));
  
  if (Zero_TN && TN_is_const_reg(Zero_TN) && WN_const_val(WN_LK) == 0) {
    TN_LK = Zero_TN;
  } else {
    TN_LK = GEN_CONST(WN_const_val(WN_LK));
  }

#else
  TN_LK = GEN_CONST(WN_const_val(WN_LK));
#endif
}

/*RULE: reg:INTCONST#0 */
static void emit_rule_action_INTCONST_0 (NBG_Link LINK, void *STATE)
{
#ifdef TARG_ST
  /* ref: whirl2ops.cxx: Expand_Expr/case INTCONST. */
  TYPE_ID rtype = RTYPE(WN_LK);
  ASSERT_FMT(rtype == MTYPE_I4 || rtype == MTYPE_U4 || rtype == MTYPE_A4, 
	     ("unexpected rtype MTYPE %s in rule: %s", MTYPE_name(rtype), RULE_STR));
  if (TN_LK != NULL && TN_LK != Zero_TN) { 
    Exp_COPY(TN_LK, Zero_TN, OPS_PTR);
  } else {
    TN_LK = Zero_TN;
  }
#else
  TN *cnst;
  GEN_RESULT(RTYPE(WN_LK));
  cnst = GEN_CONST(CONST(WN_LK));
  BUILD_OP_1_1(TOP_IMM(TOP_mov_r,cnst), TN_LK, cnst, OPS_PTR);
#endif
}

/*RULE: reg:INTCONST */
static void emit_rule_action_INTCONST (NBG_Link LINK, void *STATE)
{
#ifdef TARG_ST
  /* ref: whirl2ops.cxx: Expand_Expr/case INTCONST. */
  TYPE_ID rtype = RTYPE(WN_LK);
  TN *op0;
  ASSERT_FMT(rtype == MTYPE_I4 || rtype == MTYPE_U4 || rtype == MTYPE_A4, 
	     ("unexpected rtype MTYPE %s in rule: %s", MTYPE_name(rtype), RULE_STR));
  
  if (TN_LK == NULL) {
    /* Allocates result. */
    GEN_RESULT(RTYPE(WN_LK));
    if (CGSPILL_Rematerialize_Constants) {
      Set_TN_is_rematerializable(TN_LK);
      Set_TN_home (TN_LK, WN_LK);
    }
  }
  op0 = Gen_Literal_TN ((INT32) WN_const_val(WN_LK), 4);
    
  /* ref: cgexp.cxx. */
  Expand_Immediate (TN_LK, op0, RTYPE(WN_LK), OPS_PTR);

#else
  /* ref: TBC */
  TYPE_ID rtype = RTYPE(WN_LK);
  TN *cnst;
  ASSERT_FMT(rtype == MTYPE_I4 || rtype == MTYPE_U4, ("unexpected rtype MTYPE %s in rule: %s", MTYPE_name(rtype), RULE_STR));

  GEN_RESULT(RTYPE(WN_LK));
  cnst = GEN_CONST(CONST(WN_LK));
  BUILD_OP_1_1(TOP_IMM(TOP_mov_r, cnst), TN_LK, cnst, OPS_PTR);
#endif
}


/*RULE: reg:CONST */
static void emit_rule_action_CONST (NBG_Link LINK, void *STATE)
{
#ifdef TARG_ST
  /* ref: whirl2ops.cxx: Expand_Expr/case CONST. */
  TN *op0;
  if (TN_LK == NULL) {
    GEN_RESULT(RTYPE(WN_LK));
    if (CGSPILL_Rematerialize_Constants) {
      Set_TN_is_rematerializable(TN_LK);
      Set_TN_home (TN_LK, WN_LK);
    }
  }
  op0 = Gen_Symbol_TN (WN_st(WN_LK), 0, 0);
  
  /* ref: cgexp.cxx. */
  Expand_Const (TN_LK, op0, RTYPE(WN_LK), OPS_PTR);
  
#else
  UNIMPLEMENTED;
#endif
}


/*RULE: reg:ADD(reg,reg) */
static void emit_rule_action_ADD (NBG_Link LINK, void *STATE)
{
#ifdef TARG_ST
  TYPE_ID rtype = RTYPE(WN_LK);
  GEN_RESULT(rtype);
  Expand_Add(TN_LK, TN_LK0, TN_LK1, rtype, OPS_PTR);
#else
  /* ref: exp_targ.cxx/Expand_Add. */
  TYPE_ID rtype = RTYPE(WN_LK);
  ASSERT_FMT(rtype == MTYPE_I4 || rtype == MTYPE_U4, ("unexpected rtype MTYPE %s in rule: %s", MTYPE_name(rtype), RULE_STR));

  MAKE_EXPR_2(TOP_add_r);
#endif
}


/*RULE: reg:SUB(reg,reg) */
static void emit_rule_action_SUB (NBG_Link LINK, void *STATE)
{
#ifdef TARG_ST
  TYPE_ID rtype = RTYPE(WN_LK);
  GEN_RESULT(rtype);
  Expand_Sub(TN_LK, TN_LK0, TN_LK1, rtype, OPS_PTR);
#else
  /* ref: exp_targ.cxx/Expand_Sub. */
  TYPE_ID rtype = RTYPE(WN_LK);
  ASSERT_FMT(rtype == MTYPE_I4 || rtype == MTYPE_U4, ("unexpected rtype MTYPE %s in rule: %s", MTYPE_name(rtype), RULE_STR));

  MAKE_EXPR_2(TOP_sub_r);
#endif
}


/*RULE: reg:NEG(reg) */
static void emit_rule_action_NEG (NBG_Link LINK, void *STATE)
{
#ifdef TARG_ST
  TYPE_ID rtype = RTYPE(WN_LK);
  GEN_RESULT(rtype);
  Expand_Neg(TN_LK, TN_LK0, rtype, OPS_PTR);
#else
  /* ref: exp_targ.cxx/Expand_Neg. */
  TYPE_ID rtype = RTYPE(WN_LK);
  ASSERT_FMT(rtype == MTYPE_I4 || rtype == MTYPE_U4, ("unexpected rtype MTYPE %s in rule: %s", MTYPE_name(rtype), RULE_STR));

  GEN_RESULT(RTYPE(WN_LK));
  BUILD_OP_1_2(TOP_sub_r, TN_LK, Zero_TN, TN_LK0, OPS_PTR);
#endif
}


/*RULE: reg:ASHR(reg,reg) */
static void emit_rule_action_ASHR (NBG_Link LINK, void *STATE)
{
#ifdef TARG_ST
  TYPE_ID rtype = RTYPE(WN_LK);
  GEN_RESULT(rtype);
  Expand_Shift(TN_LK, TN_LK0, TN_LK1, rtype, shift_aright, OPS_PTR);

#else
  /* ref: exp_targ.cxx/Expand_Shift. */
  TYPE_ID rtype = RTYPE(WN_LK);
  ASSERT_FMT(rtype == MTYPE_I4 || rtype == MTYPE_U4, ("unexpected rtype MTYPE %s in rule: %s", MTYPE_name(rtype), RULE_STR));

  MAKE_EXPR_2(TOP_shr_r);
#endif
}


/*RULE: reg:LSHR(reg,reg) */
static void emit_rule_action_LSHR (NBG_Link LINK, void *STATE)
{
#ifdef TARG_ST
  TYPE_ID rtype = RTYPE(WN_LK);
  GEN_RESULT(rtype);
  Expand_Shift(TN_LK, TN_LK0, TN_LK1, rtype, shift_lright, OPS_PTR);

#else
  /* ref: exp_targ.cxx/Expand_Shift. */
  TYPE_ID rtype = RTYPE(WN_LK);
  ASSERT_FMT(rtype == MTYPE_I4 || rtype == MTYPE_U4, ("unexpected rtype MTYPE %s in rule: %s", MTYPE_name(rtype), RULE_STR));

  MAKE_EXPR_2(TOP_shru_r);
#endif
}

static void emit_rule_action_SHRI4_cnst (NBG_Link LINK, void *STATE)
{
#ifdef TARG_ST
  TYPE_ID rtype = MTYPE_I4;
  GEN_RESULT(rtype);
  Expand_Shift(TN_LK, TN_LK0, TN_LK1, rtype, shift_aright, OPS_PTR);
#else
  TYPE_ID rtype = MTYPE_I4;
  GEN_RESULT(rtype);
  BUILD_OP_1_2(TOP_IMM(TOP_shr_r,TN_LK1), TN_LK, TN_LK0, TN_LK1, OPS_PTR);
#endif
}

static void emit_rule_action_SHRU4_cnst (NBG_Link LINK, void *STATE)
{
#ifdef TARG_ST
  TYPE_ID rtype = MTYPE_U4;
  GEN_RESULT(rtype);
  Expand_Shift(TN_LK, TN_LK0, TN_LK1, rtype, shift_lright, OPS_PTR);
#else
  TYPE_ID rtype = MTYPE_U4;
  GEN_RESULT(rtype);
  BUILD_OP_1_2(TOP_IMM(TOP_shru_r, TN_LK1), TN_LK, TN_LK0, TN_LK1, OPS_PTR);
#endif
}

/*RULE: reg:SHL(reg,reg) */
static void emit_rule_action_SHL (NBG_Link LINK, void *STATE)
{
#ifdef TARG_ST
  TYPE_ID rtype = RTYPE(WN_LK);
  GEN_RESULT(rtype);
  Expand_Shift(TN_LK, TN_LK0, TN_LK1, rtype, shift_left, OPS_PTR);

#else
  /* ref: exp_targ.cxx/Expand_Shift. */
  TYPE_ID rtype = RTYPE(WN_LK);
  ASSERT_FMT(rtype == MTYPE_I4 || rtype == MTYPE_U4, ("unexpected rtype MTYPE %s in rule: %s", MTYPE_name(rtype), RULE_STR));

  MAKE_EXPR_2(TOP_shl_r);
#endif
}


/*RULE: reg:MPY(reg,reg) */
static void emit_rule_action_MPY32 (NBG_Link LINK, void *STATE)
{
#ifdef TARG_ST
  TYPE_ID rtype = RTYPE(WN_LK);
  GEN_RESULT(RTYPE(WN_LK));
  Expand_Multiply(TN_LK, rtype, TN_LK0, rtype, TN_LK1, rtype, OPS_PTR);
#else
  /* ref: exp_targ.cxx/Expand_Multiply. */
  TYPE_ID rtype = RTYPE(WN_LK);
  TN *op0 = TN_LK0;
  TN *op1 = TN_LK1;

  if (TN_is_constant(op0)) {
    op0 = TN_LK1;
    op1 = TN_LK0;
  }
  switch(rtype) {
  case MTYPE_I4:
  case MTYPE_U4:
  case MTYPE_A4:
    {
      
      TN *tmp1, *tmp2;
      
      GEN_RESULT(RTYPE(WN_LK));
      tmp1 = ALLOC_REG(RTYPE(WN_LK));
      tmp2 = ALLOC_REG(RTYPE(WN_LK));
      BUILD_OP_1_2(TOP_IMM(TOP_mullu_r, op1), tmp1, op0, op1, OPS_PTR);
      BUILD_OP_1_2(TOP_IMM(TOP_mulhs_r, op1), tmp2, op0, op1, OPS_PTR);
      BUILD_OP_1_2(TOP_add_r, TN_LK, tmp1, tmp2, OPS_PTR);
    } break;
  default:
    ABORT_FMT(("unexpected rtype MTYPE %s in rule: %s", MTYPE_name(rtype), RULE_STR));
  }
#endif
}


/*RULE: reg:DIV_CNST(reg,con) */
static void emit_rule_action_DIV_CNST (NBG_Link LINK, void *STATE)
{
#ifdef TARG_ST
  TYPE_ID rtype = RTYPE(WN_LK);
  GEN_RESULT(rtype);
  Expand_Divide(TN_LK, TN_LK0, TN_LK1, rtype, OPS_PTR);
#else
  UNIMPLEMENTED;
#endif
}

/*RULE: reg:DIV(reg,reg) */
static void emit_rule_action_DIV (NBG_Link LINK, void *STATE)
{
  UNIMPLEMENTED;
}


/*RULE: reg:REM_CNST(reg,con) */
static void emit_rule_action_REM_CNST (NBG_Link LINK, void *STATE)
{
#ifdef TARG_ST
  TYPE_ID rtype = RTYPE(WN_LK);
  GEN_RESULT(rtype);
  Expand_Rem(TN_LK, TN_LK0, TN_LK1, rtype, OPS_PTR);
#else
  UNIMPLEMENTED;
#endif
}

/*RULE: reg:REM(reg,reg) */
static void emit_rule_action_REM (NBG_Link LINK, void *STATE)
{
  UNIMPLEMENTED;
}


/*RULE: reg:MOD(reg,reg) */
static void emit_rule_action_MOD (NBG_Link LINK, void *STATE)
{
  UNIMPLEMENTED;
}


/*RULE: reg:ABS(reg) */
static void emit_rule_action_ABS (NBG_Link LINK, void *STATE)
{
  /* ref: exp_targ.cxx/Expand_Abs. */
  TYPE_ID rtype = RTYPE(WN_LK);
  switch (rtype) {
  case MTYPE_I4: {
    /* generate max(neg(op0),op0). */
    TN *tmp1;
    GEN_RESULT(RTYPE(WN_LK));
    tmp1 = ALLOC_REG(RTYPE(WN_LK));
    BUILD_OP_1_2(TOP_sub_r, tmp1, Zero_TN, TN_LK0, OPS_PTR);
    BUILD_OP_1_2(TOP_max_r, TN_LK, TN_LK0, tmp1, OPS_PTR);
  } break;
  case MTYPE_F4: {
    ABORT_FMT(("unexpected rtype MTYPE %s in rule: %s", MTYPE_name(rtype), RULE_STR));
  } break;
  default:
    ABORT_FMT(("unexpected rtype MTYPE %s in rule: %s", MTYPE_name(rtype), RULE_STR));
  }
}


/*RULE: reg:MAX(reg,reg) */
static void emit_rule_action_MAX (NBG_Link LINK, void *STATE)
{
#ifdef TARG_ST
  TYPE_ID rtype = RTYPE(WN_LK);
  GEN_RESULT(rtype);
  Expand_Max(TN_LK, TN_LK0, TN_LK1, rtype, OPS_PTR);
#else
  UNIMPLEMENTED;
#endif
}


/*RULE: reg:MIN(reg,reg) */
static void emit_rule_action_MIN (NBG_Link LINK, void *STATE)
{
#ifdef TARG_ST
  TYPE_ID rtype = RTYPE(WN_LK);
  GEN_RESULT(rtype);
  Expand_Min(TN_LK, TN_LK0, TN_LK1, rtype, OPS_PTR);
#else
  UNIMPLEMENTED;
#endif
}


/*RULE: reg:MADD(reg,reg,reg) */
static void emit_rule_action_reg_MADD_reg_reg_reg_ (NBG_Link LINK, void *STATE)
{
  UNIMPLEMENTED;
}


/*RULE: reg:MSUB(reg,reg,reg) */
static void emit_rule_action_reg_MSUB_reg_reg_reg_ (NBG_Link LINK, void *STATE)
{
  UNIMPLEMENTED;
}


/*RULE: reg:NMADD(reg,reg,reg) */
static void emit_rule_action_reg_NMADD_reg_reg_reg_ (NBG_Link LINK, void *STATE)
{
  UNIMPLEMENTED;
}


/*RULE: reg:NMSUB(reg,reg,reg) */
static void emit_rule_action_reg_NMSUB_reg_reg_reg_ (NBG_Link LINK, void *STATE)
{
  UNIMPLEMENTED;
}


/*RULE: reg:BNOT(reg) */
static void emit_rule_action_BNOT (NBG_Link LINK, void *STATE)
{
#ifdef TARG_ST
  TYPE_ID rtype = RTYPE(WN_LK);
  GEN_RESULT(rtype);
  Expand_Binary_Complement(TN_LK, TN_LK0, rtype, OPS_PTR);
#else
  /* ref: exp_targ.cxx/Expand_Binary_Compliment. */
  TYPE_ID rtype = RTYPE(WN_LK);
  TN *cnst;
  ASSERT_FMT(rtype == MTYPE_I4 || rtype == MTYPE_U4, ("unexpected rtype MTYPE %s in rule: %s", MTYPE_name(rtype), RULE_STR));

  GEN_RESULT(rtype);
  cnst = GEN_CONST(-1);
  BUILD_OP_1_2(TOP_IMM(TOP_xor_r, cnst), TN_LK, TN_LK0, cnst, OPS_PTR);
#endif
}


/*RULE: reg:BAND(reg,reg) */
static void emit_rule_action_BAND (NBG_Link LINK, void *STATE)
{
#ifdef TARG_ST
  TYPE_ID rtype = RTYPE(WN_LK);
  ASSERT_FMT(rtype == MTYPE_I4 || rtype == MTYPE_U4, ("unexpected rtype MTYPE %s in rule: %s", MTYPE_name(rtype), RULE_STR));
  GEN_RESULT(rtype);
  Expand_Binary_And(TN_LK, TN_LK0, TN_LK1, rtype, OPS_PTR);
#else
  /* ref: exp_targ.cxx/Expand_Binary_And. */
  TYPE_ID rtype = RTYPE(WN_LK);
  ASSERT_FMT(rtype == MTYPE_I4 || rtype == MTYPE_U4, ("unexpected rtype MTYPE %s in rule: %s", MTYPE_name(rtype), RULE_STR));

  MAKE_EXPR_2(TOP_and_r);
#endif
}


/*RULE: reg:BIOR(reg,reg) */
static void emit_rule_action_BIOR (NBG_Link LINK, void *STATE)
{
#ifdef TARG_ST
  TYPE_ID rtype = RTYPE(WN_LK);
  ASSERT_FMT(rtype == MTYPE_I4 || rtype == MTYPE_U4, ("unexpected rtype MTYPE %s in rule: %s", MTYPE_name(rtype), RULE_STR));
  GEN_RESULT(rtype);
  Expand_Binary_Or(TN_LK, TN_LK0, TN_LK1, rtype, OPS_PTR);
#else
  /* ref: exp_targ.cxx/Expand_Binary_Or. */
  TYPE_ID rtype = RTYPE(WN_LK);
  ASSERT_FMT(rtype == MTYPE_I4 || rtype == MTYPE_U4, ("unexpected rtype MTYPE %s in rule: %s", MTYPE_name(rtype), RULE_STR));

  MAKE_EXPR_2(TOP_or_r);
#endif
}


/*RULE: reg:BXOR(reg,reg) */
static void emit_rule_action_BXOR (NBG_Link LINK, void *STATE)
{
#ifdef TARG_ST
  TYPE_ID rtype = RTYPE(WN_LK);
  ASSERT_FMT(rtype == MTYPE_I4 || rtype == MTYPE_U4, ("unexpected rtype MTYPE %s in rule: %s", MTYPE_name(rtype), RULE_STR));
  GEN_RESULT(rtype);
  Expand_Binary_Xor(TN_LK, TN_LK0, TN_LK1, rtype, OPS_PTR);
#else
  /* ref: exp_targ.cxx/Expand_Binary_Xor. */
  TYPE_ID rtype = RTYPE(WN_LK);
  ASSERT_FMT(rtype == MTYPE_I4 || rtype == MTYPE_U4, ("unexpected rtype MTYPE %s in rule: %s", MTYPE_name(rtype), RULE_STR));

  MAKE_EXPR_2(TOP_xor_r);
#endif
}


/*RULE: reg:BNOR(reg,reg) */
static void emit_rule_action_BNOR (NBG_Link LINK, void *STATE)
{
  UNIMPLEMENTED;
}


/*RULE: reg:LNOT(reg) */
static void emit_rule_action_LNOT (NBG_Link LINK, void *STATE)
{
  /* ref: exp_targ.cxx/Expand_Logical_Not. */
  TYPE_ID rtype = RTYPE(WN_LK);

  ASSERT_FMT(rtype == MTYPE_I4 || rtype == MTYPE_U4, ("unexpected rtype MTYPE %s in rule: %s", MTYPE_name(rtype), RULE_STR));
  GEN_RESULT(rtype);
  BUILD_OP_1_2(TOP_IMM(TOP_cmpeq_r_r, TN_LK0), TN_LK, Zero_TN, TN_LK0, OPS_PTR);
}


/*RULE: reg:LAND(reg,reg) */
static void emit_rule_action_LAND (NBG_Link LINK, void *STATE)
{
  /* ref: exp_targ.cxx/Expand_Logical_And. */
  TYPE_ID rtype = RTYPE(WN_LK);
  TN *op0 = TN_LK0;
  TN *op1 = TN_LK1;

  ASSERT_FMT(rtype == MTYPE_I4 || rtype == MTYPE_U4, ("unexpected rtype MTYPE %s in rule: %s", MTYPE_name(rtype), RULE_STR));

  if (TN_is_constant(op0)) {
    op0 = TN_LK1;
    op1 = TN_LK0;
  }
  
  GEN_RESULT(rtype);
  BUILD_OP_1_2(TOP_IMM(TOP_andl_r_r, op1), TN_LK, op0, op1, OPS_PTR);
}


/*RULE: reg:LIOR(reg,reg) */
static void emit_rule_action_LIOR (NBG_Link LINK, void *STATE)
{
  /* ref: exp_targ.cxx/Expand_Logical_Or. */
  TYPE_ID rtype = RTYPE(WN_LK);
  TN *op0 = TN_LK0;
  TN *op1 = TN_LK1;

  ASSERT_FMT(rtype == MTYPE_I4 || rtype == MTYPE_U4, ("unexpected rtype MTYPE %s in rule: %s", MTYPE_name(rtype), RULE_STR));

  if (TN_is_constant(op0)) {
    op0 = TN_LK1;
    op1 = TN_LK0;
  }
  
  GEN_RESULT(rtype);
  BUILD_OP_1_2(TOP_IMM(TOP_orl_r_r, op1), TN_LK, op0, op1, OPS_PTR);
}


/*RULE: reg:SELECT(cond,reg,reg) */
static void emit_rule_action_SELECT_cond (NBG_Link LINK, void *STATE)
{
#ifdef TARG_ST
  TN *lhs, *rhs;
  GEN_RESULT(RTYPE(WN_LK));
  /* Get registers for lhs of cond and rhs of cond. */
  lhs = TN_L(LK_L(LK0,0));
  rhs = TN_L(LK_L(LK0,1));
  Emit_SELECT_AND_COND_(WN_LK, WN_KID0, TN_LK, lhs, rhs, TN_LK1, TN_LK2, OPS_PTR);
#else
  UNIMPLEMENTED;
#endif
}

/*RULE: reg:SELECT(reg,reg,reg) */
static void emit_rule_action_SELECT_reg (NBG_Link LINK, void *STATE)
{
#ifdef TARG_ST
  TYPE_ID rtype = RTYPE(WN_LK);
  TN *tmp = Build_RCLASS_TN(ISA_REGISTER_CLASS_branch);

  GEN_RESULT(rtype);
  Emit_SELECT_(WN_LK, WN_KID0, TN_LK, TN_LK0, TN_LK1, TN_LK2, OPS_PTR);
#else
  UNIMPLEMENTED;
#endif
}

static void emit_rule_action_COMPARE_INT (NBG_Link LINK, void *STATE)
{
#ifdef TARG_ST
  TYPE_ID rtype = RTYPE(WN_LK);
  TYPE_ID desc = RTYPE(WN_LK);
  ASSERT_FMT(rtype == MTYPE_I4 || rtype == MTYPE_U4 || rtype == MTYPE_A4, 
	     ("unexpected rtype %s for compare in rule: %s", MTYPE_name(rtype), RULE_STR));
  ASSERT_FMT(desc == MTYPE_I4 || desc == MTYPE_U4 || desc == MTYPE_A4, 
	     ("unexpected desc %s for compare in rule: %s", MTYPE_name(desc), RULE_STR));
  GEN_RESULT(RTYPE(WN_LK));
  switch(WN_operator(WN_LK)) {
  case OPR_LT:
    Expand_Int_Less (TN_LK, TN_LK0, TN_LK1, desc, OPS_PTR);
    break;
  case OPR_LE:
    Expand_Int_Less_Equal (TN_LK, TN_LK0, TN_LK1, desc, OPS_PTR);
    break;
  case OPR_EQ:
    Expand_Int_Equal (TN_LK, TN_LK0, TN_LK1, desc, OPS_PTR);
    break;
  case OPR_NE:
    Expand_Int_Not_Equal (TN_LK, TN_LK0, TN_LK1, desc, OPS_PTR);
    break;
  case OPR_GE:
    Expand_Int_Greater_Equal (TN_LK, TN_LK0, TN_LK1, desc, OPS_PTR);
    break;
  case OPR_GT:
    Expand_Int_Greater (TN_LK, TN_LK0, TN_LK1, desc, OPS_PTR);
    break;
  default:
    ASSERT_FMT(0, ("unexpected opcode for compare in rule: %s", OPCODE_name(WN_opcode(WN_LK)), RULE_STR));
  }
#else
  UNIMPLEMENTED;
#endif
}

/*RULE: reg:SQRT(reg) */
static void emit_rule_action_reg_SQRT_reg_ (NBG_Link LINK, void *STATE)
{
  UNIMPLEMENTED;
}


/*RULE: reg:RSQRT(reg) */
static void emit_rule_action_reg_RSQRT_reg_ (NBG_Link LINK, void *STATE)
{
  UNIMPLEMENTED;
}


/*RULE: reg:RECIP(reg) */
static void emit_rule_action_reg_RECIP_reg_ (NBG_Link LINK, void *STATE)
{
  UNIMPLEMENTED;
}


/*RULE: reg:RND(reg) */
static void emit_rule_action_reg_RND_reg_ (NBG_Link LINK, void *STATE)
{
  UNIMPLEMENTED;
}


/*RULE: reg:TRUNC(reg) */
static void emit_rule_action_reg_TRUNC_reg_ (NBG_Link LINK, void *STATE)
{
  UNIMPLEMENTED;
}


/*RULE: reg:CEIL(reg) */
static void emit_rule_action_reg_CEIL_reg_ (NBG_Link LINK, void *STATE)
{
  UNIMPLEMENTED;
}


/*RULE: reg:FLOOR(reg) */
static void emit_rule_action_reg_FLOOR_reg_ (NBG_Link LINK, void *STATE)
{
  UNIMPLEMENTED;
}


/*RULE: reg:CVTL(reg) */
static void emit_rule_action_reg_CVTL_reg_ (NBG_Link LINK, void *STATE)
{
  UNIMPLEMENTED;
}

/*RULE: reg:CVTLI4(reg) */
static void emit_rule_action_CVTLI4_8 (NBG_Link LINK, void *STATE)
{
  MAKE_EXPR_1(TOP_sxtb);
}

static void emit_rule_action_CVTLU4_8 (NBG_Link LINK, void *STATE)
{
  TN *cnst;
  GEN_RESULT(RTYPE(WN_LK));
  cnst = GEN_CONST(0xff);
  BUILD_OP_1_2(TOP_IMM(TOP_and_r, cnst), TN_LK, TN_LK0, cnst, OPS_PTR);
}

static void emit_rule_action_CVTLI4_16 (NBG_Link LINK, void *STATE)
{
  MAKE_EXPR_1(TOP_sxth);
}

static void emit_rule_action_CVTLU4_16 (NBG_Link LINK, void *STATE)
{
  MAKE_EXPR_1(TOP_zxth);
}


/*RULE: reg:CVT(reg) */
static void emit_rule_action_reg_CVT_reg_ (NBG_Link LINK, void *STATE)
{
  UNIMPLEMENTED;
}


static void emit_rule_action_GOTO(NBG_Link LINK, void *STATE)
{
#ifdef TARG_ST
  TN *target_tn =  Gen_Label_TN (Get_WN_Label (WN_LK), 0);
  Expand_Branch (target_tn, NULL, NULL, V_BR_ALWAYS, OPS_PTR);
#else
  UNIMPLEMENTED;
#endif
}

static void emit_rule_action_AGOTO(NBG_Link LINK, void *STATE)
{
#ifdef TARG_ST
  Exp_Indirect_Branch (TN_LK0, OPS_PTR);
#else
  UNIMPLEMENTED;
#endif
}

static void emit_rule_action_XGOTO(NBG_Link LINK, void *STATE)
{
#ifdef TARG_ST
  Exp_Indirect_Branch (TN_LK0, OPS_PTR);
#else
  UNIMPLEMENTED;
#endif
}

static void emit_rule_action_CONDBR_reg(NBG_Link LINK, void *STATE)
{
#ifdef TARG_ST
  TN *target_tn;
  TN *op0;
  TN *op1;
  VARIANT variant;
  WN *condition = WN_LK0;
  TYPE_ID rtype_cond = WN_rtype(condition);

  ASSERT_FMT(rtype_cond == MTYPE_I4 || rtype_cond == MTYPE_U4 || rtype_cond == MTYPE_A4, 
	     ("unexpected rtype for cond of branch MTYPE %s in rule: %s", MTYPE_name(rtype_cond), RULE_STR));

  op0 = TN_LK0;
  variant = V_BR_I4NE;
  if (Zero_TN && TN_is_const_reg(Zero_TN)) { 
    op1 = Zero_TN;
  } else {
    // I will have to do the comparison to the literal TN 0:
    op1 = Gen_Literal_TN (0, MTYPE_byte_size(rtype_cond));
  }

  // Set the branch is on false:
  if (WN_opcode(WN_LK) == OPC_FALSEBR) Set_V_false_br(variant);

  target_tn = Gen_Label_TN (Get_WN_Label (WN_LK), 0);
  Expand_Branch (target_tn, op0, op1, variant, OPS_PTR);

#else
  UNIMPLEMENTED;
#endif
}

static void emit_rule_action_CONDBR_cond(NBG_Link LINK, void *STATE)
{
#ifdef TARG_ST
  TN *target_tn;
  TN *lhs, *rhs;
  VARIANT variant;
  WN *condition = WN_LK0;

  variant = WHIRL_Compare_To_OP_variant (WN_opcode(condition));
  ASSERT_FMT(variant != V_BR_NONE, ("unexpected condition variant V_BR_NONE in rule: %s", RULE_STR));
  
  /* Get registers for lhs of cond and rhs of cond. */
  lhs = TN_L(LK_L(LK0,0));
  rhs = TN_L(LK_L(LK0,1));

  // Set the branch is on false:
  if (WN_opcode(WN_LK) == OPC_FALSEBR) Set_V_false_br(variant);

  target_tn = Gen_Label_TN (Get_WN_Label (WN_LK), 0);
  Expand_Branch (target_tn, lhs, rhs, variant, OPS_PTR);

#else
  UNIMPLEMENTED;
#endif
}

static void emit_rule_action_CONDBR_const(NBG_Link LINK, void *STATE)
{
#ifdef TARG_ST
  WN *intconst = WN_KID0;
  VARIANT variant;
  TN *target_tn;

  ASSERT_FMT(WN_operator(intconst) == OPR_INTCONST,
	     ("expected INTCONST operator intead of %s in rule: %s", OPERATOR_name(WN_operator(intconst)), RULE_STR));

  variant = WN_const_val(intconst) != 0 ? V_BR_ALWAYS : V_BR_NEVER;

  // Set the branch is on false:
  if (WN_opcode(WN_LK) == OPC_FALSEBR) Set_V_false_br(variant);

  target_tn = Gen_Label_TN (Get_WN_Label (WN_LK), 0);
  Expand_Branch (target_tn, NULL, NULL, variant, OPS_PTR);
#else
  UNIMPLEMENTED;
#endif
}


/*RULE: stmt:CALL.V */
static void emit_rule_action_CALL (NBG_Link LINK, void *STATE)
{
#ifdef TARG_ST
  UNIMPLEMENTED;
#else
  MAKE_STMT_0(TOP_call);
#endif
}

/*RULE: stmt:CALL.V */
static void emit_rule_action_RETURN (NBG_Link LINK, void *STATE)
{
#ifdef TARG_ST
  UNIMPLEMENTED;
#else
  MAKE_STMT_0(TOP_return);
#endif
}



/*RULE: reg:LOWPART(reg) */
static void emit_rule_action_reg_LOWPART_reg_ (NBG_Link LINK, void *STATE)
{
  UNIMPLEMENTED;
}


/*RULE: reg:HIGHPART(reg) */
static void emit_rule_action_reg_HIGHPART_reg_ (NBG_Link LINK, void *STATE)
{
  UNIMPLEMENTED;
}


/*RULE: reg:MINPART(reg) */
static void emit_rule_action_reg_MINPART_reg_ (NBG_Link LINK, void *STATE)
{
  UNIMPLEMENTED;
}


/*RULE: reg:MAXPART(reg) */
static void emit_rule_action_reg_MAXPART_reg_ (NBG_Link LINK, void *STATE)
{
  UNIMPLEMENTED;
}


/*RULE: reg:EXTRACT_BITS(reg) */
static void emit_rule_action_reg_EXTRACT__BITS_reg_ (NBG_Link LINK, void *STATE)
{
#ifdef TARG_ST
  /* from Handle_EXTRACT_BITS. */
  GEN_RESULT(RTYPE(WN_LK));
  Exp_Extract_Bits(RTYPE(WN_LK), RTYPE(WN_LK), 
		   WN_bit_offset(WN_LK), WN_bit_size(WN_LK), 
		   TN_LK, TN_LK0, OPS_PTR);
#else
  UNIMPLEMENTED;
#endif
}


/*RULE: reg:COMPOSE_BITS(reg,reg) */
static void emit_rule_action_reg_COMPOSE__BITS_reg_reg_ (NBG_Link LINK, void *STATE)
{
  UNIMPLEMENTED;
}


/*RULE: reg:HIGHMPY(reg,reg) */
static void emit_rule_action_reg_HIGHMPY_reg_reg_ (NBG_Link LINK, void *STATE)
{
  UNIMPLEMENTED;
}


/*RULE: reg:XMPY(reg,reg) */
static void emit_rule_action_reg_XMPY_reg_reg_ (NBG_Link LINK, void *STATE)
{
  UNIMPLEMENTED;
}


/*RULE: reg:DIVREM(reg,reg) */
static void emit_rule_action_reg_DIVREM_reg_reg_ (NBG_Link LINK, void *STATE)
{
  UNIMPLEMENTED;
}


/*RULE: reg:MINMAX(reg,reg) */
static void emit_rule_action_reg_MINMAX_reg_reg_ (NBG_Link LINK, void *STATE)
{
  UNIMPLEMENTED;
}


/*RULE: reg:LDA */
static void emit_rule_action_reg_LDA (NBG_Link LINK, void *STATE)
{
#ifdef TARG_ST
  /* from whirl2ops/Handle_LDA. */
  TN *result = TN_LK;
  
  ASSERT_FMT(WN_operator(WN_PARENT) != OPR_ICALL, ("unexpected ICALL parent to LDA, unimplemented"));
	      
  if (result == NULL) {
    GEN_RESULT(RTYPE(WN_LK));
    result = TN_LK;
    if (CGSPILL_Rematerialize_Constants) {
      Set_TN_is_rematerializable(result);
      Set_TN_home (result, WN_LK);
    }
  }
  TN_LK = result;

  Last_Mem_OP = OPS_last(OPS_PTR);
  Exp_Lda (RTYPE(WN_LK),
	   result,
	   WN_st(WN_LK), 
	   WN_lda_offset(WN_LK),
	   OPERATOR_UNKNOWN,	// the tree is indirect_call: unimplemented
	   OPS_PTR);

  Set_OP_To_WN_Map(WN_LK);

#else
  UNIMPLEMENTED;
#endif
}


/*RULE: reg:LDMA */
static void emit_rule_action_reg_LDMA (NBG_Link LINK, void *STATE)
{
  UNIMPLEMENTED;
}


/*RULE: reg:LDA_LABEL */
static void emit_rule_action_reg_LDA__LABEL (NBG_Link LINK, void *STATE)
{
  UNIMPLEMENTED;
}


/*RULE: stmt:IDNAME */
static void emit_rule_action_stmt_IDNAME (NBG_Link LINK, void *STATE)
{
  UNIMPLEMENTED;
}


/*RULE: stmt:MSTORE(reg,reg,reg) */
static void emit_rule_action_stmt_MSTORE_reg_reg_reg_ (NBG_Link LINK, void *STATE)
{
  UNIMPLEMENTED;
}


/*RULE: reg:MLOAD(reg,reg) */
static void emit_rule_action_reg_MLOAD_reg_reg_ (NBG_Link LINK, void *STATE)
{
  UNIMPLEMENTED;
}


/*RULE: inv:FIRSTPART(inv) */
static void emit_rule_action_inv_FIRSTPART_inv_ (NBG_Link LINK, void *STATE)
{
  UNIMPLEMENTED;
}


/*RULE: inv:SECONDPART(inv) */
static void emit_rule_action_inv_SECONDPART_inv_ (NBG_Link LINK, void *STATE)
{
  UNIMPLEMENTED;
}


/*RULE: inv:CAND(inv,inv) */
static void emit_rule_action_inv_CAND_inv_inv_ (NBG_Link LINK, void *STATE)
{
  UNIMPLEMENTED;
}


/*RULE: inv:CIOR(inv,inv) */
static void emit_rule_action_inv_CIOR_inv_inv_ (NBG_Link LINK, void *STATE)
{
  UNIMPLEMENTED;
}


/*RULE: inv:CSELECT(inv,inv) */
static void emit_rule_action_inv_CSELECT_inv_inv_ (NBG_Link LINK, void *STATE)
{
  UNIMPLEMENTED;
}


/*RULE: inv:ILDA(inv) */
static void emit_rule_action_inv_ILDA_inv_ (NBG_Link LINK, void *STATE)
{
  UNIMPLEMENTED;
}


/*RULE: inv:PAIR(inv,inv) */
static void emit_rule_action_inv_PAIR_inv_inv_ (NBG_Link LINK, void *STATE)
{
  UNIMPLEMENTED;
}


/*RULE: inv:RROTATE(inv,inv) */
static void emit_rule_action_inv_RROTATE_inv_inv_ (NBG_Link LINK, void *STATE)
{
  UNIMPLEMENTED;
}


/*RULE: inv:COMMA(inv,inv) */
static void emit_rule_action_inv_COMMA_inv_inv_ (NBG_Link LINK, void *STATE)
{
  UNIMPLEMENTED;
}


/*RULE: inv:RCOMMA(inv,inv) */
static void emit_rule_action_inv_RCOMMA_inv_inv_ (NBG_Link LINK, void *STATE)
{
  UNIMPLEMENTED;
}


/*RULE: inv:ARRAY(inv) */
static void emit_rule_action_inv_ARRAY_inv_ (NBG_Link LINK, void *STATE)
{
  UNIMPLEMENTED;
}


/*RULE: inv:INTRINSIC_OP(inv) */
static void emit_rule_action_inv_INTRINSIC__OP_inv_ (NBG_Link LINK, void *STATE)
{
  UNIMPLEMENTED;
}


/*RULE: inv:IO_ITEM(inv) */
static void emit_rule_action_inv_IO__ITEM_inv_ (NBG_Link LINK, void *STATE)
{
  UNIMPLEMENTED;
}


/*RULE: inv:TRIPLET(inv) */
static void emit_rule_action_inv_TRIPLET_inv_ (NBG_Link LINK, void *STATE)
{
  UNIMPLEMENTED;
}


/*RULE: inv:ARRAYEXP(inv) */
static void emit_rule_action_inv_ARRAYEXP_inv_ (NBG_Link LINK, void *STATE)
{
  UNIMPLEMENTED;
}


/*RULE: inv:ARRSECTION(inv) */
static void emit_rule_action_inv_ARRSECTION_inv_ (NBG_Link LINK, void *STATE)
{
  UNIMPLEMENTED;
}


/*RULE: inv:WHERE(inv) */
static void emit_rule_action_inv_WHERE_inv_ (NBG_Link LINK, void *STATE)
{
  UNIMPLEMENTED;
}


/*RULE: stmt:FORWARD_BARRIER */
static void emit_rule_action_stmt_FORWARD__BARRIER (NBG_Link LINK, void *STATE)
{
  /* ref: cgexp.cxx. */
  MAKE_STMT_0(TOP_fwd_bar);
}


/*RULE: stmt:BACKWARD_BARRIER */
static void emit_rule_action_stmt_BACKWARD__BARRIER (NBG_Link LINK, void *STATE)
{
  /* ref: cgexp.cxx. */
  MAKE_STMT_0(TOP_bwd_bar);
}


/*RULE: stmt:INTRINSIC_CALL.V */
static void emit_rule_action_stmt_INTRINSIC__CALL_V (NBG_Link LINK, void *STATE)
{
  UNIMPLEMENTED;
}


/*RULE: reg:INTRINSIC_CALL */
static void emit_rule_action_reg_INTRINSIC__CALL (NBG_Link LINK, void *STATE)
{
  UNIMPLEMENTED;
}


/*RULE: reg:MPY_PW2(reg,reg) */
static void emit_rule_action_MPY_PW2 (NBG_Link LINK, void *STATE)
{
#ifdef TARG_ST
  TYPE_ID rtype = RTYPE(WN_LK);
  TN *op0 = TN_LK0;
  TN *op1 = TN_LK1;
  int pw2;

  if (TN_is_constant(op0)) {
    op0 = TN_LK1;
    op1 = TN_LK0;
  }
  ASSERT_FMT(TN_is_constant(op1), ("unexpected tn in rule: %s", RULE_STR));

  GEN_RESULT(rtype);
  pw2 = TN_value(op1) < 0 ? 
    get_power_of_2_u32((UINT32)(-TN_value(op1))):
    get_power_of_2_u32((UINT32)(TN_value(op1)));
  ASSERT_FMT(pw2 > 0 && pw2 <= 31, ("unexpected shift value %d for power of 2 mpy", pw2));
  if (TN_value(op1) < 0 && pw2 < 31) {
    TN * tmp1 = ALLOC_REG(rtype);
    Expand_Shift (tmp1, op0, GEN_CONST(pw2), rtype, shift_left, OPS_PTR);
    Expand_Neg (TN_LK, tmp1, rtype, OPS_PTR);
  } else {
    Expand_Shift (TN_LK, op0, GEN_CONST(pw2), rtype, shift_left, OPS_PTR);
  }

#else
  UNIMPLEMENTED;
#endif
}

static void emit_rule_action_SH1ADD (NBG_Link LINK, void *STATE)
{
  GEN_RESULT(RTYPE(WN_LK));
  BUILD_OP_1_2(TOP_IMM(TOP_sh1add_r, TN_LK1), TN_LK, TN_LK0, TN_LK1, OPS_PTR);
}

static void emit_rule_action_SH2ADD (NBG_Link LINK, void *STATE)
{
  GEN_RESULT(RTYPE(WN_LK));
  BUILD_OP_1_2(TOP_IMM(TOP_sh2add_r, TN_LK1), TN_LK, TN_LK0, TN_LK1, OPS_PTR);
}

static void emit_rule_action_SH3ADD (NBG_Link LINK, void *STATE)
{
  GEN_RESULT(RTYPE(WN_LK));
  BUILD_OP_1_2(TOP_IMM(TOP_sh3add_r, TN_LK1), TN_LK, TN_LK0, TN_LK1, OPS_PTR);
}

static void emit_rule_action_SH4ADD (NBG_Link LINK, void *STATE)
{
  GEN_RESULT(RTYPE(WN_LK));
  BUILD_OP_1_2(TOP_IMM(TOP_sh4add_r, TN_LK1), TN_LK, TN_LK0, TN_LK1, OPS_PTR);
}

/*RULE: regi4:	SHL(MPY(regi4, m_regi4_zx16h), m_cnsti4_16) */
static void emit_rule_action_MULHS (NBG_Link LINK, void *STATE)
{
  GEN_RESULT(RTYPE(WN_LK));
  BUILD_OP_1_2(TOP_IMM(TOP_mulhs_r, TN_LK1), TN_LK, TN_LK0, TN_LK1, OPS_PTR);
}


/*RULE: regi4:	SHL(MPY(m_regi4_zx16h, regi4), m_cnsti4_16) */
static void emit_rule_action_MULHS_2 (NBG_Link LINK, void *STATE)
{
  GEN_RESULT(RTYPE(WN_LK)); 
  BUILD_OP_1_2(TOP_IMM(TOP_mulhs_r, TN_LK0), TN_LK, TN_LK1, TN_LK0, OPS_PTR);
}


/*RULE: regi4:MPY(m_regi4_sx16h, m_regi4_sx16h) */
static void emit_rule_action_MULHH (NBG_Link LINK, void *STATE)
{
  GEN_RESULT(RTYPE(WN_LK)); 
  BUILD_OP_1_2(TOP_IMM(TOP_mulhh_r, TN_LK1), TN_LK, TN_LK0, TN_LK1,OPS_PTR);
}


/*RULE: regi4:MPY(m_regi4_zx16h, m_regi4_zx16h) */
static void emit_rule_action_MULHHU (NBG_Link LINK, void *STATE)
{
  GEN_RESULT(RTYPE(WN_LK)); 
  BUILD_OP_1_2(TOP_IMM(TOP_mulhhu_r, TN_LK1), TN_LK, TN_LK0, TN_LK1,OPS_PTR);
}


/*RULE: regi4: MPY(m_regi4_sx16, m_regi4_sx16h) */
static void emit_rule_action_MULLH (NBG_Link LINK, void *STATE)
{
  GEN_RESULT(RTYPE(WN_LK)); 
  BUILD_OP_1_2(TOP_IMM(TOP_mullh_r, TN_LK1), TN_LK, TN_LK0, TN_LK1, OPS_PTR);
}

/*RULE: regi4: MPY(m_regi4_sx16h, m_regi4_sx16) */
static void emit_rule_action_MULLH_2 (NBG_Link LINK, void *STATE)
{
  GEN_RESULT(RTYPE(WN_LK)); 
  BUILD_OP_1_2(TOP_IMM(TOP_mullh_r, TN_LK0), TN_LK, TN_LK1, TN_LK0, OPS_PTR);
}

/*RULE: regi4:MPY(m_regi4_zx16, m_regi4_zx16h) */
static void emit_rule_action_MULLHU (NBG_Link LINK, void *STATE)
{
  GEN_RESULT(RTYPE(WN_LK)); 
  BUILD_OP_1_2(TOP_IMM(TOP_mullhu_r, TN_LK1), TN_LK, TN_LK0, TN_LK1, OPS_PTR);
}


/*RULE: regi4:MPY(m_regi4_zx16h, m_regi4_zx16) */
static void emit_rule_action_MULLHU_2 (NBG_Link LINK, void *STATE)
{
  GEN_RESULT(RTYPE(WN_LK)); 
  BUILD_OP_1_2(TOP_IMM(TOP_mullhu_r, TN_LK0), TN_LK, TN_LK1, TN_LK0, OPS_PTR);
  
}


/*RULE: regi4:MPY(regi4, m_regi4_sx16h) */
static void emit_rule_action_MULH (NBG_Link LINK, void *STATE)
{
  GEN_RESULT(RTYPE(WN_LK)); 
  BUILD_OP_1_2(TOP_IMM(TOP_mulh_r, TN_LK1), TN_LK, TN_LK0, TN_LK1, OPS_PTR);
}


/*RULE: regi4:MPY(m_regi4_sx16h, regi4) */
static void emit_rule_action_MULH_2 (NBG_Link LINK, void *STATE)
{
  GEN_RESULT(RTYPE(WN_LK)); 
  BUILD_OP_1_2(TOP_IMM(TOP_mulh_r, TN_LK0), TN_LK, TN_LK1, TN_LK0, OPS_PTR);
}


/*RULE: regi4:MPY(regi4, m_regi4_zx16h) */
static void emit_rule_action_MULHU (NBG_Link LINK, void *STATE)
{
  GEN_RESULT(RTYPE(WN_LK)); 
  BUILD_OP_1_2(TOP_IMM(TOP_mulhu_r, TN_LK1), TN_LK, TN_LK0, TN_LK1, OPS_PTR);
}


/*RULE: regi4:MPY(m_regi4_zx16h, regi4) */
static void emit_rule_action_MULHU_2 (NBG_Link LINK, void *STATE)
{
  GEN_RESULT(RTYPE(WN_LK)); 
  BUILD_OP_1_2(TOP_IMM(TOP_mulhu_r, TN_LK0), TN_LK, TN_LK1, TN_LK0, OPS_PTR);
}


/*RULE: regi4:MPY(m_regi4_sx16,m_regi4_sx16) */
static void emit_rule_action_MULLL (NBG_Link LINK, void *STATE)
{
  GEN_RESULT(RTYPE(WN_LK)); 
  BUILD_OP_1_2(TOP_IMM(TOP_mulll_r, TN_LK1), TN_LK, TN_LK0, TN_LK1, OPS_PTR);
}


static void emit_rule_action_MULLL_2 (NBG_Link LINK, void *STATE)
{
  GEN_RESULT(RTYPE(WN_LK)); 
  BUILD_OP_1_2(TOP_IMM(TOP_mulll_r, TN_LK0), TN_LK, TN_LK1, TN_LK0, OPS_PTR);
}


/*RULE: regi4:MPY(m_regi4_zx16,m_regi4_zx16) */
static void emit_rule_action_MULLLU (NBG_Link LINK, void *STATE)
{
  GEN_RESULT(RTYPE(WN_LK)); 
  BUILD_OP_1_2(TOP_IMM(TOP_mulllu_r, TN_LK1), TN_LK, TN_LK0, TN_LK1, OPS_PTR);
}

static void emit_rule_action_MULLLU_2 (NBG_Link LINK, void *STATE)
{
  GEN_RESULT(RTYPE(WN_LK)); 
  BUILD_OP_1_2(TOP_IMM(TOP_mulllu_r, TN_LK0), TN_LK, TN_LK1, TN_LK0, OPS_PTR);
}


/*RULE: regi4:MPY(regi4,m_regi4_sx16) */
static void emit_rule_action_MULL (NBG_Link LINK, void *STATE)
{
  GEN_RESULT(RTYPE(WN_LK)); 
  BUILD_OP_1_2(TOP_IMM(TOP_mull_r, TN_LK1), TN_LK, TN_LK0, TN_LK1, OPS_PTR);
}


/*RULE: regi4:MPY(m_regi4_sx16,regi4) */
static void emit_rule_action_MULL_2 (NBG_Link LINK, void *STATE)
{
  GEN_RESULT(RTYPE(WN_LK)); 
  BUILD_OP_1_2(TOP_IMM(TOP_mull_r, TN_LK0), TN_LK, TN_LK1, TN_LK0, OPS_PTR);
}


/*RULE: regi4:MPY(regi4,m_regi4_zx16) */
static void emit_rule_action_MULLU (NBG_Link LINK, void *STATE)
{
  GEN_RESULT(RTYPE(WN_LK)); 
  BUILD_OP_1_2(TOP_IMM(TOP_mullu_r, TN_LK1), TN_LK, TN_LK0, TN_LK1, OPS_PTR);
}


/*RULE: regi4:MPY(m_regi4_zx16, regi4) */
static void emit_rule_action_MULLU_2 (NBG_Link LINK, void *STATE)
{
  GEN_RESULT(RTYPE(WN_LK)); 
  BUILD_OP_1_2(TOP_IMM(TOP_mullu_r, TN_LK0), TN_LK, TN_LK1, TN_LK0, OPS_PTR);
}


/*
 * TOP_Immediate_Variant
 */
static TOP
TOP_Imm(TOP regform, INT64 imm)
{
  int s9 = 0;
  if (imm >= -(1<<8) && imm < (1<<8)) s9 = 1;

#define CASE_TOP(top) case TOP_##top##_r: return s9 ? TOP_##top##_i : TOP_##top##_ii;
#define CASE_TOP2(top) case TOP_##top##_r_r: return s9 ? TOP_##top##_i_r : TOP_##top##_ii_r; \
			case TOP_##top##_r_b: return s9 ? TOP_##top##_i_b : TOP_##top##_ii_b;

  switch(regform) {
    CASE_TOP(mov);
    CASE_TOP(add);
    CASE_TOP(sub);
    CASE_TOP(shl);
    CASE_TOP(shr);
    CASE_TOP(shru);
    CASE_TOP(sh1add);
    CASE_TOP(sh2add);
    CASE_TOP(sh3add);
    CASE_TOP(sh4add);
    CASE_TOP(and);
    CASE_TOP(andc);
    CASE_TOP(or);
    CASE_TOP(orc);
    CASE_TOP(xor);
    CASE_TOP(mullhus);
    CASE_TOP(max);
    CASE_TOP(maxu);
    CASE_TOP(min);
    CASE_TOP(minu);
    CASE_TOP(mulhhs);
    CASE_TOP(mull);
    CASE_TOP(mullu);
    CASE_TOP(mulh);
    CASE_TOP(mulhu);
    CASE_TOP(mulll);
    CASE_TOP(mulllu);
    CASE_TOP(mullh);
    CASE_TOP(mullhu);
    CASE_TOP(mulhh);
    CASE_TOP(mulhhu);
    CASE_TOP(mulhs);
    CASE_TOP2(cmpeq);
    CASE_TOP2(cmpne);
    CASE_TOP2(cmpge);
    CASE_TOP2(cmpgeu);
    CASE_TOP2(cmpgt);
    CASE_TOP2(cmpgtu);
    CASE_TOP2(cmple);
    CASE_TOP2(cmpleu);
    CASE_TOP2(cmplt);
    CASE_TOP2(cmpltu);
    CASE_TOP2(andl);
    CASE_TOP2(nandl);
    CASE_TOP2(orl);
    CASE_TOP2(norl);
    CASE_TOP(slct);
    CASE_TOP(slctf);
  default:
    ASSERT_FMT(0, ("unexpected top: %s", TOP_Name(regform)));
  }
  return TOP_UNDEFINED;
#undef TOP_CASE
}


#ifdef __cplusplus
extern "C" {
#endif

/*
 * CGG_visit_td_prealloc
 * This is called from targ_cgg_exp in a top down traversal
 * pass used for preallocation to preg.
 * All forced destination must be set here.
 * STATE contains nothing currently.
 * The preallocation is propagated down to chain rules.
 * This implies that:
 * IN THE MODEL, A CHAIN RULE MUST NOT CHANGE THE RESULT TYPE.
 * INT THE ACTIONS A CHAIN RULE MUST ONLY PROPAGATE UP RESULTS AND
 * NOT GENERATE ANY ACTION.
 *
 * summary:
 * - preallocates preg of STID
 * - allocate symbol objects of nodes with symbols
 * - counts WN number
 */
void
CGG_visit_td_prealloc(NBG_Link LINK, void *STATE)
{
  static NBG_Link last_link;
  static WN * last_wn;
  WN *wn = WN_LK;

  if (NBG_Link_parent(LINK) == NULL) {
    last_link = NULL;
    last_wn = NULL;
  }
  
  /* Actions performed for each unique wn encountered. */
  if (last_wn != wn) {
#ifdef TARG_ST
    /* From whirl2ops.cxx (Expand_Expr). */
    /* Counts WN number. */
    PU_WN_Cnt++;
    
    /* From whirl2ops.cxx (Expand_Expr). */
    /* Allocate object for symbols. */
    if (OPCODE_has_sym(WN_opcode(wn)) && WN_st(wn) != NULL) {
      if (WN_operator(wn) != OPR_CONST || 
	  !MTYPE_is_float(WN_rtype(wn)) || 
	  CG_floating_const_in_memory)
	/* make sure st is allocated */
	Allocate_Object (WN_st(wn));
    }
#endif
  }

  /* Handle chain rules. */
  if (NBG_emit_action(NBG_Link_rule(LINK)) == emit_rule_action_chain) {
    /* simply propagate allocation. */
    TN_LK0 = TN_LK;
  } else {
    switch (NBG_Link_rule(LINK)) {
    case NBG_RULE_stmt_STID___CLASS__PREG_reg_: {
      TN *result;
      PREG_NUM preg;
      WN *stid = wn;

      preg = get_preg(stid);
      result = preg_to_tn(stid, preg);

      /* Force destination of kid. */
      TN_LK0 = result;
    } break;
    default:
      break;
    }
  }
  last_wn = wn;
  last_link = LINK;
}

#ifdef __cplusplus
}
#endif

#ifdef TARG_ST

/*
 * Emit NOOP.
 * Due to the bottom up reg alloc, we may have to produce copy when
 * a preg was forced, upper in the tree.
 * THIS MAY INTRODUCE DIFFERENCES WITH STANDARD CODE SELECTOR.
 */
static void 
Emit_Noop(NBG_Link LINK, void *STATE)
{
  if (TN_LK != NULL) { 
    if (TN_LK != TN_LK0) Exp_COPY(TN_LK, TN_LK0, OPS_PTR);
  } else {
    TN_LK = TN_LK0;
  }
}

/*
 * Currently we allocate a new TN for every invocation of this procedure.
 */
static TN *
Allocate_TN_Of_Mtype(TYPE_ID mtype)
{
  ASSERT_FMT(mtype != MTYPE_I8 && mtype != MTYPE_U8, ("unexpected mtype for allocate TN: %s", MTYPE_name(mtype)));
  return Build_TN_Of_Mtype (mtype);
}


/* ====================================================================
 *   Memop_Variant
 *
 *   Determine the Exp_OP variant for a memory operation.
 * ====================================================================
 */
static VARIANT 
Memop_Variant (
  WN *memop
)
{
  VARIANT variant = V_NONE;
#ifdef TARG_ST
  INT     required_alignment = MTYPE_alignment(WN_desc(memop));
#else
  INT     required_alignment = MTYPE_RegisterSize(WN_desc(memop));
#endif

  /* If volatile, set the flag.
   */
  if (WN_Is_Volatile_Mem(memop)) Set_V_volatile(variant);

  /* Determine the alignment related variants. We have to check both 
   * ty alignment and the offset alignment.
   */
  Is_True ((required_alignment != 0), ("bad value 0 for required_alignment"));
 
  if (required_alignment > 1) {
    WN_OFFSET offset;
    INT ty_align;
    INT align;

    switch (WN_operator(memop)) {
    case OPR_LDID:
    case OPR_LDBITS:
    case OPR_STID:
    case OPR_STBITS:
      /* store_offset and load_offset are the same, so we share the code
       */
      offset = WN_load_offset(memop);
      ty_align = ST_alignment(WN_st(memop));
      break;
    case OPR_ILOAD:
    case OPR_ILDBITS:
      {
	TY_IDX ty = WN_load_addr_ty(memop);
	if (TY_kind(ty) == KIND_POINTER) ty = TY_pointed(ty);
	ty_align = TY_align(ty);
	offset = WN_load_offset(memop);
      }
      break;
    case OPR_ISTORE:
    case OPR_ISTBITS:
      {
	TY_IDX ty = WN_ty(memop);
	if (TY_kind(ty) == KIND_POINTER) ty = TY_pointed(ty);
	ty_align = TY_align(ty);
	offset = WN_store_offset(memop);
      }
      break;
    default:
      FmtAssert(FALSE, ("unhandled memop in Memop_Variant"));
      /*NOTREACHED*/
    }

    align = ty_align;
    if (offset) {
      INT offset_align = offset % required_alignment;
      if (offset_align) align = MIN(ty_align, offset_align);
    }

    if (align < required_alignment) {
      Set_V_alignment(variant, align);
     /*
      *	TODO
      *	When we have ST information we may be able to compute an
      *	offset(say mod 16) that will give us further information
      */
      Set_V_align_offset_unknown(variant);

      /* We have an unaligned volatile. What to do??
       * Suneel/bean want the object to ignore the atomicity of volatile
       * and generate the unaligned references.
       * This will at least get the users code working
       */
      if (V_volatile(variant)) {
	ErrMsgLine(EC_Ill_Align,
		   SRCPOS_linenum(current_srcpos),
		   ty_align,
		   "reference to unaligned volatile:  volatile atomicity is ignored");
      }
    }
  }

  /* Now get prefetch flags if any
   */
  WN *pf_wn = NULL;
  PF_POINTER *pf_ptr = (PF_POINTER *) WN_MAP_Get(WN_MAP_PREFETCH, memop);
  if (pf_ptr) {
    pf_wn = PF_PTR_wn_pref_2L(pf_ptr);
    if (pf_wn == NULL) pf_wn = PF_PTR_wn_pref_1L(pf_ptr);
  }
  if (pf_wn) Set_V_pf_flags(variant, WN_prefetch_flag(pf_wn));

  return variant;
}

/* ======================================================================
 *   from Handle_STID
 * ======================================================================
 */
static void
Handle_STID_ (
  WN *stid, 
  OPCODE opcode,
  TN *op0,
  OPS *ops
)
{
  // generate a store:
  VARIANT variant = Memop_Variant(stid);

  ASSERT_FMT(WN_class(stid) != CLASS_PREG, ("unexpected class preg"));
  ASSERT_FMT(!ST_assigned_to_dedicated_preg(WN_st(stid)), ("unexpected dedicated preg, class is not PREG"));

  Last_Mem_OP = OPS_last(ops);
  Exp_Store (OPCODE_desc(opcode), 
	     op0,
	     WN_st(stid), 
	     WN_store_offset(stid),
	     ops,
	     variant);
  Set_OP_To_WN_Map(stid);
  return;
}

/* ======================================================================
 *   from Handle_STID
 * ======================================================================
 */
static void
Handle_STID_PREG_ (
  WN *stid, 
  OPCODE opcode,
  TN *op0,
  OPS *ops
)
{
  TN *result;

  ASSERT_FMT(WN_class(stid) == CLASS_PREG, ("expected class preg"));
  result = PREG_To_TN (WN_st(stid), WN_store_offset(stid), stid);
  ASSERT_FMT(result == op0, ("unexpected STID_PREG operand"));

  /* Nothing to generate. The kid was forced to the preg during CGG_td_visit_prealloc. */
  
  if (In_Glue_Region) {
    if (Trace_REGION_Interface) {
      fprintf(TFile,"set op_glue on preg store in bb %d\n", BB_id(Cur_BB));
    }
    Set_OP_glue(OPS_last(ops));
  }
}


/* ====================================================================
 *   from Handle_LDID
 * ====================================================================
 */
static TN *
Handle_LDID_ (
  WN *ldid, 
  OPCODE opcode,
  TN *result, 
  OPS *ops
)
{
  // generate a load:
  VARIANT variant;

  ASSERT_FMT(WN_class(ldid) != CLASS_PREG, ("unexpected class preg"));
  ASSERT_FMT(!ST_assigned_to_dedicated_preg(WN_st(ldid)), ("unexpected dedicated preg, class is not PREG"));


  if (opcode == OPC_U4U8LDID) {
    opcode =	OPC_U4U4LDID;
    WN_set_opcode(ldid, opcode);
    if (Target_Byte_Sex == BIG_ENDIAN) {
      WN_offset(ldid) += 4;	// get low-order word
    }
  }
  else if (opcode == OPC_I4I8LDID) {
    opcode = OPC_I4I4LDID;
    WN_set_opcode(ldid, opcode);
    if (Target_Byte_Sex == BIG_ENDIAN) {
      WN_offset(ldid) += 4;	// get low-order word
    }
  }
  variant = Memop_Variant(ldid);

  Last_Mem_OP = OPS_last(ops);
  Exp_Load (OPCODE_rtype(opcode), OPCODE_desc(opcode),
	     result, 
	     WN_st(ldid), 
	     WN_load_offset(ldid), 
	     ops, 
	     variant);

  Set_OP_To_WN_Map(ldid);
  return result;
}

/* ====================================================================
 *   from Handle_ISTORE
 * ====================================================================
 */
static void
Handle_ISTORE_LDA_ (
  WN *istore, 
  WN *kid1,
  OPCODE opcode,
  TN *op0,
  OPS *ops
)
{
  VARIANT variant;
  ST *st;

  ASSERT_FMT(WN_operator_is(kid1, OPR_LDA), ("expected LDA as kid1 of ISTORE"));

  variant = Memop_Variant(istore);

  st = WN_st(kid1);
  /* make sure st is allocated */
  Allocate_Object (st);
  
  Last_Mem_OP = OPS_last(ops);
  Exp_Store (OPCODE_desc(opcode),
	     op0,
	     st, 
	     WN_offset(istore) + WN_lda_offset(kid1),
	     ops,
	     variant);

  Set_OP_To_WN_Map(istore);
}

static void
Handle_ISTORE_ (
  WN *istore, 
  OPCODE opcode,
  TN *op0,
  TN *op1,
  OPS *ops
)
{
  VARIANT variant;
  TN *offset_tn;

  variant = Memop_Variant(istore);

  offset_tn = Gen_Literal_TN (WN_offset(istore), 4);
  
  Last_Mem_OP = OPS_last(ops);

  if (V_alignment(variant) != V_NONE) {
    Expand_Misaligned_Store (OPCODE_desc(opcode), op0, op1, offset_tn, variant, ops);
  } else {
    Expand_Store (OPCODE_desc(opcode), op0, op1, offset_tn, ops);
  }

  Set_OP_To_WN_Map(istore);
}


/* ====================================================================
 * from Handle_STBITS
 * ====================================================================
 */
static void
Handle_STBITS_PREG_ (
  WN *stbits, 
  WN *kid,
  OPCODE opcode,
  TN *op0, 
  OPS *ops)
{
  TN *field_tn;
  TN *result;
    
  const TYPE_ID desc = Mtype_TransferSign(MTYPE_U4, WN_desc(stbits));
  const TYPE_ID rtype = Mtype_TransferSize(WN_rtype(kid), desc);

  ASSERT_FMT(WN_class(stbits) == CLASS_PREG, ("expected class preg for STBITS"));
  
  field_tn =  PREG_To_TN (WN_st(stbits), WN_store_offset(stbits), stbits);
  result = field_tn;

  // deposit bits_tn into field_tn returning result in result
  Exp_Deposit_Bits(rtype, desc, 
		   WN_bit_offset(stbits), WN_bit_size(stbits), 
		   result, field_tn, op0, ops);
}

static void
Handle_STBITS_ (
  WN *stbits, 
  WN *kid,
  OPCODE opcode,
  TN *op0, 
  OPS *ops)
{
  VARIANT variant;
  TN *field_tn;
  TN *result;
    
  const TYPE_ID desc = Mtype_TransferSign(MTYPE_U4, WN_desc(stbits));
  const TYPE_ID rtype = Mtype_TransferSize(WN_rtype(kid), desc);

  ASSERT_FMT(WN_class(stbits) != CLASS_PREG, ("unexpected class preg for STBITS"));
  
  variant = Memop_Variant(stbits);
  field_tn = ALLOC_REG(WN_rtype(kid));
  
  Last_Mem_OP = OPS_last(ops);
  Exp_Load(rtype, desc, field_tn, WN_st(stbits), WN_load_offset(stbits),
	   ops, variant); // must do an unsigned load
  Set_OP_To_WN_Map(stbits);
  
  result = ALLOC_REG(WN_rtype(kid));

  // deposit bits_tn into field_tn returning result in result
  Exp_Deposit_Bits(rtype, desc, 
		   WN_bit_offset(stbits), WN_bit_size(stbits), 
		   result, field_tn, op0, ops);
  
  Last_Mem_OP = OPS_last(ops);
  Exp_Store(desc, result, WN_st(stbits), WN_store_offset(stbits), ops,
	    variant);
  Set_OP_To_WN_Map(stbits);
}


/* ====================================================================
 * from Handle_ISTBITS
 * ====================================================================
 */
static void
Handle_ISTBITS_(WN *istbits, WN *kid, OPCODE opcode, TN *op0, TN *op1, OPS *ops)
{
  VARIANT variant;
  TN *offset_tn;
  TN *field_tn;
  TN *result;
  TYPE_ID desc = Mtype_TransferSign(MTYPE_U4, WN_desc(istbits));
  TYPE_ID rtype = desc;
  
  // TBC: ????
  // guard against U1MPY or U2MPY
  if (MTYPE_byte_size(rtype) < 4)
    rtype = Mtype_TransferSize(MTYPE_U4, rtype);
  if (MTYPE_byte_size(WN_rtype(kid)) > MTYPE_byte_size(rtype)) 
    rtype = Mtype_TransferSize(WN_rtype(kid), rtype);

  variant = Memop_Variant(istbits);

  offset_tn = Gen_Literal_TN (WN_store_offset(istbits), 4);
  
  field_tn = ALLOC_REG(WN_rtype(kid));

  Last_Mem_OP = OPS_last(ops);
  if (V_alignment(variant) != V_NONE) {
    Expand_Misaligned_Load (OPCODE_make_op(OPR_ILOAD, rtype, desc),
			    field_tn, op1, offset_tn, variant, ops);
  } else {
    Expand_Load(OPCODE_make_op(OPR_ILOAD, rtype, desc),
		field_tn, op1, offset_tn, ops);
  }
  Set_OP_To_WN_Map(istbits);
  
  result = ALLOC_REG(WN_rtype(kid));
  // deposit bits_tn into field_tn returning result in result
  Exp_Deposit_Bits(rtype, desc, 
		   WN_bit_offset(istbits), WN_bit_size(istbits),
		   result, field_tn, op0, ops);
  
  Last_Mem_OP = OPS_last(ops);
  if (V_alignment(variant) != V_NONE) {
    Expand_Misaligned_Store(OPCODE_make_op(OPR_ISTORE, MTYPE_V, desc), 
			    result, op1, offset_tn, variant, ops);
  } else { 
    Expand_Store(OPCODE_make_op(OPR_ISTORE, MTYPE_V, desc), 
		 result, op1, offset_tn, ops);
  }

  Set_OP_To_WN_Map(istbits);
}

static void
Handle_ISTBITS_LDA_(WN *istbits, WN *kid0, WN *kid1, OPCODE opcode, TN *op0, OPS *ops)
{
  VARIANT variant;
  TN *offset_tn;
  TN *field_tn;
  TN *result;
  ST *st;
  TYPE_ID desc = Mtype_TransferSign(MTYPE_U4, WN_desc(istbits));
  TYPE_ID rtype = desc;

  ASSERT_FMT(WN_operator_is(kid1, OPR_LDA), ("expected LDA as kid1 of ISTBITS"));


  // TBC: ????
  // guard against U1MPY or U2MPY
  if (MTYPE_byte_size(rtype) < 4)
    rtype = Mtype_TransferSize(MTYPE_U4, rtype);
  if (MTYPE_byte_size(WN_rtype(kid0)) > MTYPE_byte_size(rtype)) 
    rtype = Mtype_TransferSize(WN_rtype(kid0), rtype);
  
  st = WN_st(kid1);
  /* make sure st is allocated */
  Allocate_Object (st);
  
  field_tn = ALLOC_REG(WN_rtype(kid0));

  Last_Mem_OP = OPS_last(ops);
  Exp_Load(rtype, desc, field_tn, st, 
	   WN_store_offset(istbits) + WN_lda_offset(kid1), ops,
	   variant);
  Set_OP_To_WN_Map(istbits);

  result = ALLOC_REG(WN_rtype(kid0));
  // deposit bits_tn into field_tn returning result in result
  Exp_Deposit_Bits(rtype, desc, WN_bit_offset(istbits), 
		   WN_bit_size(istbits), result, field_tn, op0, ops);
  
  Last_Mem_OP = OPS_last(ops);
  Exp_Store(desc, result, st, WN_store_offset(istbits) + WN_lda_offset(kid1),
	    ops, variant);
  Set_OP_To_WN_Map(istbits);
}

/* ====================================================================
 *   from Handle_ILOAD_
 * ====================================================================
 */
static TN *
Handle_ILOAD_LDA_ (
  WN *iload, 
  WN *kid0,
  OPCODE opcode,
  TN *result, 
  OPS *ops
)
{
  // generate a load:
  VARIANT variant;
  ST *st;
  
  ASSERT_FMT(WN_operator_is(kid0, OPR_LDA), ("expected LDA as kid0 of ILOAD/ILDBITS"));
  
  if (opcode == OPC_U4U8ILOAD) {
    opcode = OPC_U4U4ILOAD;
    WN_set_opcode(iload, opcode);
    if (Target_Byte_Sex == BIG_ENDIAN) {
      WN_offset(iload) += 4;	// get low-order word
    }
  }
  else if (opcode == OPC_I4I8ILOAD) {
    opcode =	OPC_I4I4ILOAD;
    WN_set_opcode(iload, opcode);
    if (Target_Byte_Sex == BIG_ENDIAN) {
      WN_offset(iload) += 4;	// get low-order word
    }
  }

  variant = Memop_Variant(iload);
  
  st = WN_st(kid0);
  /* make sure st is allocated */
  Allocate_Object (st);
  
  Last_Mem_OP = OPS_last(ops);
  Exp_Load (OPCODE_rtype(opcode), OPCODE_desc(opcode),
	    result, 
	    st, 
	    WN_offset(iload) + WN_lda_offset(kid0),
	    ops,
	    variant);

  Set_OP_To_WN_Map(iload);
  
  return result;
}

/* ====================================================================
 *   from Handle_ILOAD_
 * ====================================================================
 */
static TN *
Handle_ILOAD_ (
  WN *iload, 
  OPCODE opcode,
  TN *result, 
  TN *op0,
  OPS *ops
)
{
  // generate a load:
  VARIANT variant;
  TN *offset_tn;

  if (opcode == OPC_U4U8ILOAD) {
    opcode = OPC_U4U4ILOAD;
    WN_set_opcode(iload, opcode);
    if (Target_Byte_Sex == BIG_ENDIAN) {
	WN_offset(iload) += 4;	// get low-order word
    }
  }
  else if (opcode == OPC_I4I8ILOAD) {
    opcode =	OPC_I4I4ILOAD;
    WN_set_opcode(iload, opcode);
    if (Target_Byte_Sex == BIG_ENDIAN) {
	WN_offset(iload) += 4;	// get low-order word
    }
  }

  variant = Memop_Variant(iload);
  
  offset_tn = Gen_Literal_TN (WN_offset(iload), 4);
  Last_Mem_OP = OPS_last(ops);
  if (V_alignment(variant) != V_NONE) {
    Expand_Misaligned_Load (opcode, result, op0, offset_tn,
			    variant, ops);
  } else {
    Expand_Load (opcode, result, op0, offset_tn, ops);
  }
  Set_OP_To_WN_Map(iload);
  return result;
}

/* ====================================================================
 *   from Handle_LDID
 * ====================================================================
 */
static TN *
Handle_LDID_PREG_ (
  WN *ldid, 
  OPCODE opcode,
  TN *result, 
  OPS *ops
)
{
  TN *ldid_result;

  ASSERT_FMT(WN_class(ldid) == CLASS_PREG, ("expected class preg"));

  if (ST_assigned_to_dedicated_preg(WN_st(ldid))) {
    // replace st with dedicated preg
    WN_offset(ldid) = Find_PREG_For_Symbol(WN_st(ldid));
    WN_st_idx(ldid) = ST_st_idx(MTYPE_To_PREG(ST_mtype(WN_st(ldid))));
  }

  ldid_result = PREG_To_TN (WN_st(ldid), WN_load_offset(ldid), ldid);
  
  if (result == NULL) {
    result = ldid_result;
  } else {
    Exp_COPY (result, ldid_result, ops);
  }
  return result;
}

/* ====================================================================
 * from HANDLE_LDBITS
 * ====================================================================
 */
static TN *
Handle_LDBITS_PREG_(WN *ldbits, OPCODE opcode, TN *result, OPS *ops)
{
  TN *ld_tn;
  
  ASSERT_FMT(WN_class(ldbits) == CLASS_PREG, ("expected class preg for LDBITS"));
  
  ld_tn = PREG_To_TN (WN_st(ldbits), WN_load_offset(ldbits), ldbits);
  Exp_Extract_Bits(WN_rtype(ldbits), WN_desc(ldbits), 
		   WN_bit_offset(ldbits), WN_bit_size(ldbits),
		   result, ld_tn, ops);
  return result;
}

static TN *
Handle_LDBITS_(WN *ldbits, OPCODE opcode, TN *result, OPS *ops)
{
  /* From handle ldbits. */
  TN *ld_tn;
  VARIANT variant;
  
  variant = Memop_Variant(ldbits);
  ld_tn = ALLOC_REG(WN_rtype(ldbits));

  Last_Mem_OP = OPS_last(ops);
  Exp_Load (WN_rtype(ldbits), WN_desc(ldbits),
	    ld_tn, 
	    WN_st(ldbits), 
	    WN_load_offset(ldbits), 
	    ops, 
	    variant);
  Set_OP_To_WN_Map(ldbits);
  
  Exp_Extract_Bits(WN_rtype(ldbits), WN_desc(ldbits), 
		   WN_bit_offset(ldbits), WN_bit_size(ldbits),
		   result, ld_tn, ops);
  return result;
}

static TN *
Handle_ILDBITS_LDA_(WN *ildbits, WN *lda, OPCODE opcode, TN *result, OPS *ops)
{
  TN *ld_tn;
  OPCODE ld_opc;

  ld_opc = OPCODE_make_op(OPR_ILOAD, WN_rtype(ildbits), WN_desc(ildbits));
  ld_tn = ALLOC_REG(WN_rtype(ildbits));
  
  ld_tn = Handle_ILOAD_LDA_(ildbits, lda, ld_opc, ld_tn, ops);

  Exp_Extract_Bits(WN_rtype(ildbits), WN_desc(ildbits), 
		   WN_bit_offset(ildbits), WN_bit_size(ildbits),
		   result, ld_tn, ops);
  return result;
}

static TN *
Handle_ILDBITS_(WN *ildbits, OPCODE opcode, TN *result, TN *op0, OPS *ops)
{
  TN *ld_tn;
  OPCODE ld_opc;

  ld_opc = OPCODE_make_op(OPR_ILOAD, WN_rtype(ildbits), WN_desc(ildbits));
  ld_tn = ALLOC_REG(WN_rtype(ildbits));
  
  ld_tn = Handle_ILOAD_(ildbits, ld_opc, ld_tn, op0, ops);

  Exp_Extract_Bits(WN_rtype(ildbits), WN_desc(ildbits), 
		   WN_bit_offset(ildbits), WN_bit_size(ildbits),
		   result, ld_tn, ops);
  return result;
}

/* ====================================================================
 *   WHIRL_Compare_To_OP_variant
 *
 *   Given a WHIRL comparison opcode, this routine returns
 *   the corresponding variant for the BCOND. 
 *   For float compares, keep the order of the comparison and
 *   set the false_br flag if invert, because NaN comparisons
 *   cannot be inverted.  But integer compares can be inverted.
 * ====================================================================
 */
static INT16
WHIRL_Compare_To_OP_variant (
  OPCODE opcode
)
{
  INT16 variant;

  variant = V_BR_NONE;

  switch (opcode) {
// >> WHIRL 0.30: replaced OPC_T1{EQ,NE,GT,GE,LT,LE} by OPC_BT1, OPC_I4T1 variants
// TODO WHIRL 0.30: get rid of OPC_I4T1 variants
// CG: Added OPC_U4... variants
  case OPC_BI8EQ: case OPC_I4I8EQ: case OPC_U4I8EQ: variant = V_BR_I8EQ; break;
  case OPC_BI4EQ: case OPC_I4I4EQ: case OPC_U4I4EQ: variant = V_BR_I4EQ; break;
  case OPC_BU8EQ: case OPC_I4U8EQ: case OPC_U4U8EQ: variant = V_BR_U8EQ; break;
  case OPC_BU4EQ: case OPC_I4U4EQ: case OPC_U4U4EQ: variant = V_BR_U4EQ; break;
  case OPC_BA4EQ:                  variant = V_BR_A4EQ; break;
  case OPC_BA8EQ:                  variant = V_BR_A8EQ; break;
  case OPC_BFQEQ: case OPC_I4FQEQ: case OPC_U4FQEQ: variant = V_BR_QEQ; break;
  case OPC_BF8EQ: case OPC_I4F8EQ: case OPC_U4F8EQ: variant = V_BR_DEQ; break;
  case OPC_BF4EQ: case OPC_I4F4EQ: case OPC_U4F4EQ: variant = V_BR_FEQ; break;
  case OPC_BI8NE: case OPC_I4I8NE: case OPC_U4I8NE: variant = V_BR_I8NE; break;
  case OPC_BI4NE: case OPC_I4I4NE: case OPC_U4I4NE: variant = V_BR_I4NE; break;
  case OPC_BU8NE: case OPC_I4U8NE: case OPC_U4U8NE: variant = V_BR_U8NE; break;
  case OPC_BU4NE: case OPC_I4U4NE: case OPC_U4U4NE: variant = V_BR_U4NE; break;
  case OPC_BA4NE:                  variant = V_BR_A4NE; break;
  case OPC_BA8NE:                  variant = V_BR_A8NE; break;
  case OPC_BFQNE: case OPC_I4FQNE: case OPC_U4FQNE: variant = V_BR_QNE; break;
  case OPC_BF8NE: case OPC_I4F8NE: case OPC_U4F8NE: variant = V_BR_DNE; break;
  case OPC_BF4NE: case OPC_I4F4NE: case OPC_U4F4NE: variant = V_BR_FNE; break;
  case OPC_BI8GT: case OPC_I4I8GT: case OPC_U4I8GT: variant = V_BR_I8GT; break;
  case OPC_BI4GT: case OPC_I4I4GT: case OPC_U4I4GT: variant = V_BR_I4GT; break;
  case OPC_BU8GT: case OPC_I4U8GT: case OPC_U4U8GT: variant = V_BR_U8GT; break;
  case OPC_BU4GT: case OPC_I4U4GT: case OPC_U4U4GT: variant = V_BR_U4GT; break;
  case OPC_BA4GT:                  variant = V_BR_A4GT; break;
  case OPC_BA8GT:                  variant = V_BR_A8GT; break;
  case OPC_BFQGT: case OPC_I4FQGT: case OPC_U4FQGT: variant = V_BR_QGT; break;
  case OPC_BF8GT: case OPC_I4F8GT: case OPC_U4F8GT: variant = V_BR_DGT; break;
  case OPC_BF4GT: case OPC_I4F4GT: case OPC_U4F4GT: variant = V_BR_FGT; break;
  case OPC_BI8GE: case OPC_I4I8GE: case OPC_U4I8GE: variant = V_BR_I8GE; break;
  case OPC_BI4GE: case OPC_I4I4GE: case OPC_U4I4GE: variant = V_BR_I4GE; break;
  case OPC_BU8GE: case OPC_I4U8GE: case OPC_U4U8GE: variant = V_BR_U8GE; break;
  case OPC_BU4GE: case OPC_I4U4GE: case OPC_U4U4GE: variant = V_BR_U4GE; break;
  case OPC_BA4GE:                  variant = V_BR_A4GE; break;
  case OPC_BA8GE:                  variant = V_BR_A8GE; break;
  case OPC_BFQGE: case OPC_I4FQGE: case OPC_U4FQGE: variant = V_BR_QGE; break;
  case OPC_BF8GE: case OPC_I4F8GE: case OPC_U4F8GE: variant = V_BR_DGE; break;
  case OPC_BF4GE: case OPC_I4F4GE: case OPC_U4F4GE: variant = V_BR_FGE; break;
  case OPC_BI8LT: case OPC_I4I8LT: case OPC_U4I8LT: variant = V_BR_I8LT; break;
  case OPC_BI4LT: case OPC_I4I4LT: case OPC_U4I4LT: variant = V_BR_I4LT; break;
  case OPC_BU8LT: case OPC_I4U8LT: case OPC_U4U8LT: variant = V_BR_U8LT; break;
  case OPC_BU4LT: case OPC_I4U4LT: case OPC_U4U4LT: variant = V_BR_U4LT; break;
  case OPC_BA4LT:                  variant = V_BR_A4LT; break;
  case OPC_BA8LT:                  variant = V_BR_A8LT; break;
  case OPC_BFQLT: case OPC_I4FQLT: case OPC_U4FQLT: variant = V_BR_QLT; break;
  case OPC_BF8LT: case OPC_I4F8LT: case OPC_U4F8LT: variant = V_BR_DLT; break;
  case OPC_BF4LT: case OPC_I4F4LT: case OPC_U4F4LT: variant = V_BR_FLT; break;
  case OPC_BI8LE: case OPC_I4I8LE: case OPC_U4I8LE: variant = V_BR_I8LE; break;
  case OPC_BI4LE: case OPC_I4I4LE: case OPC_U4I4LE: variant = V_BR_I4LE; break;
  case OPC_BU8LE: case OPC_I4U8LE: case OPC_U4U8LE: variant = V_BR_U8LE; break;
  case OPC_BU4LE: case OPC_I4U4LE: case OPC_U4U4LE: variant = V_BR_U4LE; break;
  case OPC_BA4LE:                  variant = V_BR_A4LE; break;
  case OPC_BA8LE:                  variant = V_BR_A8LE; break;
  case OPC_BFQLE: case OPC_I4FQLE: case OPC_U4FQLE: variant = V_BR_QLE; break;
  case OPC_BF8LE: case OPC_I4F8LE: case OPC_U4F8LE: variant = V_BR_DLE; break;
  case OPC_BF4LE: case OPC_I4F4LE: case OPC_U4F4LE: variant = V_BR_FLE; break;
  case OPC_BBNE: variant = V_BR_PNE; break;
  case OPC_BBEQ: variant = V_BR_PEQ; break;
// << WHIRL 0.30: replaced OPC_T1{EQ,NE,GT,GE,LT,LE} by OPC_BT1, OPC_I4T1 variants
  }

  /* do invert in the target dependent part of the compiler
  if (invert) variant = Negate_BR_Variant(variant);
  */

  return variant;
}

/* ====================================================================
 *   from  Handle_SELECT
 * ====================================================================
 */
static void
Emit_SELECT_ (
  WN *select,
  WN *compare,
  TN *result, 
  TN *cond,
  TN *trueop,
  TN *falseop,
  OPS *ops
)
{
  OPCODE opcode = WN_opcode(select);
  VARIANT variant;

  variant = WHIRL_Compare_To_OP_variant (WN_opcode(compare));
  
  Expand_Select (result, cond, trueop, falseop, WN_rtype(select), V_select_uses_fcc(variant), ops);
}

static void
Emit_SELECT_AND_COND_ (
  WN *select, 
  WN *compare, 
  TN *result, 
  TN *cond_l,
  TN *cond_r,
  TN *trueop,
  TN *falseop,
  OPS *ops
)
{
  OPCODE opcode = WN_opcode(select);
  VARIANT variant;

  variant = WHIRL_Compare_To_OP_variant (WN_opcode(compare));
  Exp_Select_And_Condition (opcode, result, trueop, falseop, 
			    WN_opcode(compare), cond_l, cond_r, variant, 
			    ops);
}


static TN *
preg_to_tn(WN *tree, PREG_NUM preg)
{
  return PREG_To_TN (WN_st(tree), preg, tree);
}

static PREG_NUM
get_preg(WN *tree)
{
  int preg;
  ASSERT_FMT(WN_class(tree) == CLASS_PREG, ("expected class preg"));
  if (ST_assigned_to_dedicated_preg(WN_st(tree))) {
    // replace st with dedicated preg
    WN_offset(tree) = Find_PREG_For_Symbol(WN_st(tree));
    WN_st_idx(tree) = ST_st_idx(MTYPE_To_PREG(ST_mtype(WN_st(tree))));
  }
  preg =  WN_offset(tree);
  ASSERT_FMT(preg != 0, ("unexpected preg 0"));
  return preg;
}

static int
preg_last(void)
{
  return Get_Preg_Num(PREG_Table_Size(CURRENT_SYMTAB));
}

static int
preg_is_dedicated(int preg)
{
  ISA_REGISTER_CLASS rclass;
  REGISTER reg;
  return CGTARG_Preg_Register_And_Class(preg, &rclass, &reg);
}

#if 0
static int
has_reuse_state(WN* tree)
{
  NBG_State state;
  PREG_NUM preg;

  preg = get_preg(tree);
  state = PREG_STATE_MAP_get(preg_state_map,preg);
  return state != NULL;
}

static NBG_State 
get_reuse_state(WN* tree)
{
  NBG_State state;
  PREG_NUM preg;

  preg = get_preg(tree);

  state = PREG_STATE_MAP_get(preg_state_map, preg);
  ASSERT_FMT(state != NULL, ("unexpected NULL preg state for preg %d", preg));
  ASSERT_FMT((OPERATOR)NBG_State_op(state) == OPR_STID, ("expected STID"));
  state = NBG_State_child(state, 0);
  if (CGG_trace_level) {
    fprintf (TFile,
	     "----- CGG:get_reuse_state -------------------\n");
    fprintf (TFile, "%d:", preg);
    NBG_labelled_print(state, TFile);
  }
  return state;
}

static void 
set_reuse_state(WN *tree, NBG_State state)
{
  PREG_NUM preg;
  TN *tn;

  preg = get_preg(tree);
  tn = PREG_To_TN (WN_st(tree), preg, tree);

  if (TN_is_dedicated(tn)) return;

  ASSERT_FMT(PREG_STATE_MAP_get(preg_state_map, preg) == NULL, ("CGG:set_reuse_state: unexpected redefinition of PREG %d", preg));
  PREG_STATE_MAP_set(preg_state_map, preg, state);

  if (CGG_trace_level) {
    fprintf (TFile,
	     "----- CGG:set_reuse_state -------------------\n");
    fprintf (TFile, "%d:", preg);
    NBG_labelled_print(state, TFile);
  }
}
#endif

#else

static INT_MAP *preg_tn_map;
static int preg_last_idx;

static TN *
preg_to_tn(WN *tree, PREG_NUM preg)
{
  int tn_idx;
  TN *tn;
  if (preg_tn_map == NULL) preg_tn_map = INT_INT_MAP_new(0);
  if (preg <= tn_dedicated_number()) {
    tn = get_tn_from_idx(preg-1);
  } else {
    tn_idx = INT_INT_MAP_get(preg_tn_map, preg);
    if (tn_idx == 0) {
      tn = ALLOC_REG(WN_rtype(tree));
      tn_idx = TN_idx(tn);
      INT_INT_MAP_set(preg_tn_map, preg, tn_idx);
    }
    tn = get_tn_from_idx(tn_idx);
    preg_last_idx = tn_idx;
  }

  return tn;
}

static int
preg_last(void)
{
  return MAX(preg_last_idx, tn_dedicated_number());
}

static int
preg_is_dedicated(int preg)
{
  TN *tn;
  tn = preg_to_tn(NULL, preg);
  return TN_idx(tn) < tn_dedicated_number();
}


static PREG_NUM
get_preg(WN *tree)
{
  int preg;
  ASSERT_FMT(WN_operator(tree) == OPR_STID ||
	     WN_operator(tree) == OPR_LDID, ("expected class preg"));
  preg = WN_st_idx(tree);
  ASSERT_FMT(preg != 0, ("unexpected preg 0"));
  return preg;
}

#if 0
static int
has_reuse_state(WN* tree)
{
  NBG_State state;
  PREG_NUM preg;

  preg = get_preg(tree);
  state = PREG_STATE_MAP_get(preg_state_map,preg);
  return state != NULL;
}

static NBG_State 
get_reuse_state(WN* tree)
{
  NBG_State state;
  PREG_NUM preg;

  preg = get_preg(tree);

  state = PREG_STATE_MAP_get(preg_state_map, preg);
  ASSERT_FMT(state != NULL, ("unexpected NULL preg state for preg %d", preg));
  ASSERT_FMT((OPERATOR)NBG_State_op(state) == OPR_STID, ("expected STID"));
  state = NBG_State_child(state, 0);
  if (CGG_trace_level) {
    fprintf (TFile,
	     "----- CGG:get_reuse_state -------------------\n");
    fprintf (TFile, "%d:", preg);
    NBG_labelled_print(state, TFile);
  }
  return state;
}

static void
kill_reuse_state(preg)
{
  int preg_last_def;
  int i, j;
  preg_last_def = du_preg_last_def(preg);
  for (i = preg_last_def + 1; i <= du_stmt_current(); i++) {
    for (j = du_stmt_first_use(i); j <= du_stmt_last_use(i); j++) {
      int preg_use = du_use_preg(j);
      if (preg_use != preg) continue;
      if (CGG_trace_level) fprintf(TFile, "stmt %d kill use of %d in stmt %d\n", 
				   du_stmt_current(), preg, i);
    }
  }
}

static void 
set_reuse_state(WN *tree, NBG_State state)
{
  PREG_NUM preg;

  preg = get_preg(tree);
  kill_reuse_state(preg);
  if (preg <= 63) return;

  ASSERT_FMT(PREG_STATE_MAP_get(preg_state_map, preg) == NULL, ("CGG:set_reuse_state: unexpected redefinition of PREG %d", preg));
  PREG_STATE_MAP_set(preg_state_map, preg, state);

  if (CGG_trace_level) {
    fprintf (TFile,
	     "----- CGG:set_reuse_state -------------------\n");
    fprintf (TFile, "%d:", preg);
    NBG_labelled_print(state, TFile);
  }
}
#endif

#endif

/*
 * DU chains
 */
static INT_MAP *stmt_def_map; /* map stmt def id to stmt uses ids. */
static INT_MAP *stmt_uses_map; /* stack of stmt uses. */
static INT_MAP *stmt_preg_uses_map; /* stack of preg uses. */
static INT_MAP *stmt_preg_def_map; /* map stmt to preg def. */
static INT_MAP *stmt_tree_def_map; /* map stmt to tree def. */

static INT_MAP *preg_last_def_map; /* map last preg definition. */
static INT_MAP *preg_last_use_map; /* map last preg use. */

static int du_current;
static void
du_init_bb(void)
{
  if (stmt_def_map == NULL) stmt_def_map = INT_INT_MAP_new(0);
  if (stmt_uses_map == NULL) stmt_uses_map = INT_INT_MAP_new(0);
  if (stmt_preg_uses_map == NULL) stmt_preg_uses_map = INT_INT_MAP_new(0);
  if (stmt_preg_def_map == NULL) stmt_preg_def_map = INT_INT_MAP_new(0);
  if (stmt_tree_def_map == NULL) stmt_tree_def_map = INT_PTR_MAP_new(0);
  if (preg_last_def_map == NULL) preg_last_def_map = INT_INT_MAP_new(0);

  INT_MAP_clear(stmt_def_map);
  INT_MAP_clear(stmt_uses_map);
  INT_MAP_clear(stmt_preg_uses_map);
  INT_MAP_clear(stmt_preg_def_map);
  INT_MAP_clear(stmt_tree_def_map);
  INT_MAP_clear(preg_last_def_map);
  INT_INT_MAP_append(stmt_def_map, INT_MAP_count(stmt_uses_map));
  INT_INT_MAP_append(stmt_def_map, INT_MAP_count(stmt_uses_map));
}

static void
du_init_stmt(void)
{
  INT_INT_MAP_set(stmt_def_map, 
		  INT_MAP_count(stmt_def_map)-1, 
		  INT_MAP_count(stmt_uses_map));
  du_current = INT_MAP_count(stmt_def_map)-1;
  INT_INT_MAP_append(stmt_def_map, 
		     INT_MAP_count(stmt_uses_map));
}

static void
du_add_use(int preg_use)
{
  int stmt_last_def;
  if (CGG_trace_level) fprintf(TFile, "-- CGG_DU: add_use %d\n", preg_use);

  INT_INT_MAP_append(stmt_preg_uses_map, preg_use);
  stmt_last_def = INT_INT_MAP_get(preg_last_def_map, preg_use);
  INT_INT_MAP_append(stmt_uses_map, stmt_last_def);
  INT_INT_MAP_set(stmt_def_map, 
		  du_current+1, INT_MAP_count(stmt_uses_map));
}

static void
du_add_def(int preg_def)
{
  if (CGG_trace_level) fprintf(TFile, "-- CGG_DU: add_def %d\n", preg_def);
  INT_INT_MAP_set(stmt_preg_def_map, du_current, preg_def);
  INT_INT_MAP_set(preg_last_def_map, preg_def, du_current);
}

static void
du_add_clobber(int preg_def)
{
  if (CGG_trace_level) fprintf(TFile, "-- CGG_DU: add_clobber\n");
  INT_INT_MAP_set(stmt_preg_def_map, du_current, 0);
  INT_INT_MAP_set(preg_last_def_map, preg_def, du_current);
}

static void
du_stmt_dump(int stmt, FILE *file)
{
  int i;
  const char *sep = "";
  int stmt_use, reg_use, reg_def;
  reg_def = du_stmt_def_preg(stmt);
  fprintf(file, "stmt %d: def : %d, uses:", stmt, reg_def);
  for (i = du_stmt_first_use(stmt); 
       i <= du_stmt_last_use(stmt); i++) {
    stmt_use = du_use_stmt(i);
    reg_use = du_use_preg(i);
    fprintf(file, "%s %d[%d]", sep, reg_use, stmt_use);
    sep = ",";
  }
  fprintf(file, "\n");
}

static void
du_dump(FILE *file)
{
  int i;
  for (i = du_stmt_first(); i <= du_stmt_last(); i++) {
    du_stmt_dump(i, file);
  }
}

static int
du_preg_last_def(int preg)
{
  return INT_INT_MAP_get(preg_last_def_map, preg);
}

static int
du_stmt_first(void)
{
  return 1;
}

static int
du_stmt_last(void)
{
  return INT_MAP_count(stmt_def_map)-2;
}

static int
du_stmt_current(void)
{
  return du_current;
}

static int
du_stmt_def_preg(int stmt)
{
  return INT_INT_MAP_get(stmt_preg_def_map, stmt);
}

static int
du_stmt_first_use(int stmt)
{
  return INT_INT_MAP_get(stmt_def_map, stmt);
}

static int
du_stmt_last_use(int stmt)
{
  return INT_INT_MAP_get(stmt_def_map, stmt+1)-1;
}

static int
du_use_stmt(int use)
{
  return INT_INT_MAP_get(stmt_uses_map, use);
}

static int
du_use_preg(int use)
{
  return INT_INT_MAP_get(stmt_preg_uses_map, use);
}

static int
du_use_def_stmt(int stmt, int preg)
{
  int i;
  for (i = du_stmt_first_use(stmt); 
       i <= du_stmt_last_use(stmt); i++) {
    int reg_use = du_use_preg(i);
    if (reg_use == preg) {
      int stmt_use = du_use_stmt(i);
      return stmt_use;
    }
  }
  NBG_ASSERT(0);
  return 0;
}

#ifdef TARG_ST
static WN_MAP WN_stmt_map = WN_MAP_UNDEFINED;

static void
init_wn_stmt_map(void)
{
  if (WN_stmt_map == WN_MAP_UNDEFINED) {
    WN_stmt_map = WN_MAP_Create(&MEM_phase_pool);
  }
}

static void
fini_wn_stmt_map(void)
{
  if (WN_stmt_map != WN_MAP_UNDEFINED) {
    WN_MAP_Delete(WN_stmt_map);
    WN_stmt_map = WN_MAP_UNDEFINED;
  }
}
static int
wn_get_stmt(WN *wn)
{
  int stmt;
  stmt = (int) WN_MAP_Get(WN_stmt_map, wn);
  return stmt;
}

static void
wn_set_stmt(WN *wn, int stmt)
{
  WN_MAP_Set(WN_stmt_map, wn, (void *)stmt);
}

#else
static int
wn_get_stmt(WN *wn)
{
  return wn->stmt;
}

static void
wn_set_stmt(WN *wn, int stmt)
{
  wn->stmt = stmt;
}

#endif


static WN *
du_stmt_def_tree(int stmt)
{
  return (WN *)INT_PTR_MAP_get(stmt_tree_def_map, stmt);
}

static void
du_add_use_tree(WN *ptr, int preg_use)
{
  if (CGG_trace_level) fprintf(TFile, "-- CGG_DU: add_use_tree stmt %d, reg %d\n", du_current, preg_use);
  du_add_use(preg_use);

  wn_set_stmt(ptr, du_current);
}

static void
du_add_def_tree(WN *ptr, int preg_def)
{
  if (CGG_trace_level) fprintf(TFile, "-- CGG_DU: add_def_tree stmt %d, reg %d\n", du_current, preg_def);
  du_add_def(preg_def);
  wn_set_stmt(ptr, du_current);
  INT_PTR_MAP_set(stmt_tree_def_map, du_current, ptr);
}

static void
du_add_clobber_tree(WN *ptr)
{
  int i;
  if (CGG_trace_level) fprintf(TFile, "-- CGG_DU: add_clobber stmt %d\n", du_current);
  for (i = 1; i <= preg_last(); i++) {
    if (preg_is_dedicated(i)) du_add_clobber(i);
  }
  wn_set_stmt(ptr, du_current);
  INT_PTR_MAP_set(stmt_tree_def_map, du_current, ptr);
}

static WN *
du_use_def_tree(WN *ptr, int preg_use)
{
  WN *def;
  int use_stmt;
  int def_stmt;
  use_stmt = wn_get_stmt(ptr);
  def_stmt = du_use_def_stmt(use_stmt, preg_use);
  def = du_stmt_def_tree(def_stmt);

  if (CGG_trace_level) fprintf(TFile, "-- CGG_DU: du_use_def (%d, %d) -> %d\n", use_stmt, preg_use, def_stmt);
  return def;
}


/*
 * Used for uncse.
 */
static int
wn_is_use(WN *tree)
{
  return WN_operator(tree) == OPR_LDID && WN_class(tree) == CLASS_PREG;
}

static int
wn_is_def(WN *tree)
{
  return WN_operator(tree) == OPR_STID && WN_class(tree) == CLASS_PREG;
}

static int
wn_is_clobber(WN *tree)
{
  switch (WN_operator(tree)) {
  case OPR_CALL:
  case OPR_ICALL:
  case OPR_PICCALL:
  case OPR_INTRINSIC_CALL:
    return 1;
  default:
    return 0;
  }
}

static int
wn_set_clobber(WN *tree)
{
  du_add_clobber_tree(tree);
  return 0;
}

static int
wn_set_use(WN *tree)
{
  int preg = get_preg(tree);
  du_add_use_tree(tree, preg);
  return 0;
}

static int
wn_set_def(WN *tree)
{
  int preg = get_preg(tree);
  du_add_def_tree(tree, preg);
  return 0;
}

static WN *
wn_def_expr(WN *tree)
{
  return WN_kid0(tree);
}

static WN *
wn_use_def(WN *tree)
{
  int preg = get_preg(tree);
  return du_use_def_tree(tree, preg);
}

static WN *
wn_last_def(WN *tree)
{
  int preg = get_preg(tree);
  int last = du_preg_last_def(preg);
  return du_stmt_def_tree(last);
}


/*
 * Containers
 */
#ifdef __cplusplus
extern "C" {
#endif

/* Handling INT_MAP memory. */
#define INT_MAP_MALLOC(s) malloc(s)
#define INT_MAP_REALLOC(p,s) realloc(p,s)
#define INT_MAP_FREE(p) free(p)

#define INT_MAP_HINT 256
struct INT_MAP {
  int size;
  int count;
  size_t elt_size;
  void *elts;
  int hint;
};

static void
INT_MAP_ctor(INT_MAP *map, size_t elt_size, int hint)
{
  map->size = 0;
  map->count = 0;
  map->elts = NULL;
  map->elt_size = elt_size;
  map->hint = hint == 0 ? INT_MAP_HINT: hint;
}

static void
INT_MAP_dtor(INT_MAP *map)
{
  ASSERT_FMT(map->size == 0 || map->elts != NULL, ("corrupted INT_MAP at destruction"));
  ASSERT_FMT(map->elts == NULL || map->size != 0, ("corrupted INT_MAP at destruction"));
  map->size = 0;
  if (map->elts != NULL) INT_MAP_FREE(map->elts);
}

static INT_MAP *
INT_MAP_new(size_t elt_size, int hint)
{
  INT_MAP *map;
  map = (INT_MAP *)INT_MAP_MALLOC(sizeof(INT_MAP));
  INT_MAP_ctor(map, elt_size, hint);
  return map;
}

static INT_MAP *
INT_MAP_del(INT_MAP *map)
{
  INT_MAP_dtor(map);
  INT_MAP_FREE(map);
  return NULL;
}

static int
INT_MAP_size(INT_MAP *map)
{
  return map->size;
}

static int
INT_MAP_count(INT_MAP *map)
{
  return map->count;
}

static void
INT_MAP_ralloc(INT_MAP *map, int new_size)
{
  int new_map_size;
  if (map->size == 0) new_map_size = map->hint;
  else new_map_size = map->size;
  while (new_map_size < new_size) new_map_size <<= 1;

  if (new_map_size > map->size) {
    if (map->size == 0) {
      map->elts = INT_MAP_MALLOC(new_map_size*map->elt_size);
    } else  {
      map->elts = INT_MAP_REALLOC(map->elts, new_map_size*map->elt_size);
    }
    memset((char *)map->elts + map->size * map->elt_size, 0, (new_map_size - map->size) * map->elt_size);
    map->size = new_map_size;
  }
}

static void *
INT_MAP_access(INT_MAP *map, int idx)
{
  ASSERT_FMT(idx < map->size, ("invalid INT_MAP access: %d, size %d", idx, map->size));
  return (void *)((char *)map->elts + idx * map->elt_size);
}

static void *
INT_MAP_get(INT_MAP *map, int idx)
{
  INT_MAP_ralloc(map, idx+1);
  return INT_MAP_access(map, idx);
}

static void *
INT_MAP_set(INT_MAP *map, int idx)
{
  INT_MAP_ralloc(map, idx+1);
  return INT_MAP_access(map, idx);
}

static void *
INT_MAP_unset(INT_MAP *map, int idx)
{
  return INT_MAP_access(map, idx);
}

static void *
INT_MAP_append(INT_MAP *map)
{
  int idx = map->count;
  INT_MAP_ralloc(map, idx+1);
  map->count++;
  return INT_MAP_access(map, idx);
}

static void
INT_MAP_clear(INT_MAP *map)
{
  map->count = 0;
  if (map->elts != NULL) memset(map->elts, 0, map->size * map->elt_size);
}

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 * CGG Interface.
 */
static int initialized;
static int in_function, in_region, in_bb;

void
CGG_Initialize(void)
{
  if (CGG_trace_level) fprintf (TFile, "----- CGG_Initialize -------------------\n");
  ASSERT_FMT(!initialized, ("CGG_Initialize: CGG_Finalize was not called"));
  initialized = 1;

#if 0
  preg_state_map = PREG_STATE_MAP_new(0);
#endif
}

void
CGG_Finalize(void)
{
  if (CGG_trace_level) fprintf (TFile, "----- CGG_Finalize -------------------\n");

  ASSERT_FMT(initialized, ("CGG_Finalize: CGG_Initialize was not called"));
  initialized = 0;

  NBG_del_state_list();
#if 0
  PREG_STATE_MAP_del(preg_state_map);
  preg_state_map = NULL;
#endif
}

void
CGG_Start_function(WN *stmt)
{
  if (CGG_trace_level) fprintf (TFile, "----- CGG_Start_function -------------------\n");
  ASSERT_FMT(initialized && !in_function, ("CGG_Start_function: CGG_Initialize was not called"));
  in_function = 1;

#ifdef TARG_ST
  if (CGG_opt_level >= 2) init_wn_stmt_map();
#endif
}

void
CGG_End_function(WN *stmt)
{
  if (CGG_trace_level) fprintf (TFile, "----- CGG_End_function -------------------\n");
  ASSERT_FMT(initialized && in_function, ("CGG_End_function:  CGG_Initialize was not called"));
  in_function = 0;

  NBG_del_state_list();
#ifdef TARG_ST
  if (CGG_opt_level >= 2) fini_wn_stmt_map();
#endif
}

void
CGG_Start_region(WN *stmt)
{
  if (CGG_trace_level) fprintf (TFile, "----- CGG_Start_region -------------------\n");
  ASSERT_FMT(initialized && in_function && !in_region, ("CGG_Start_region: CGG_Start_bb: CGG_Initialize was not called"));
  in_region = 1;
}

void
CGG_End_region(WN *stmt)
{
  if (CGG_trace_level) fprintf (TFile, "----- CGG_End_region -------------------\n");
  ASSERT_FMT(initialized && in_function && in_region, ("CGG_End_region:  CGG_Initialize was not called"));
  in_region = 0;
}

void
CGG_Start_bb(WN *stmt)
{
  ASSERT_FMT(initialized && in_function, ("CGG_Start_bb: CGG_Initialize was not called"));
  in_bb = 1;

  if (CGG_trace_level) {
    fprintf (TFile, "----- CGG_Start_bb -------------------\n");
  }

  NBG_del_state_list();
#if 0
  PREG_STATE_MAP_clear(preg_state_map);
#endif
  if (CGG_opt_level >= 2) du_init_bb();
}

void
CGG_Start_stmt(WN *stmt)
{
  ASSERT_FMT(initialized && in_function && in_bb, ("CGG_Start_stmt: CGG_Initialize was not called"));

  if (CGG_trace_level) {
    fprintf (TFile,
	     "----- CGG_Start_stmt at %s...-------------------\n", OPCODE_name(WN_opcode(stmt)));
  }
  if (CGG_opt_level >= 2) {
    du_init_stmt();
#ifdef TARG_ST
    if (wn_is_clobber(stmt)) wn_set_clobber(stmt);
#endif
  }

}


#ifdef __cplusplus
}
#endif
# 408 "NBG_burs_template.c"

# 736 "./models/pro_st220.md"
# 607 "pro_st220.gen.c"
# 410 "NBG_burs_template.c"


