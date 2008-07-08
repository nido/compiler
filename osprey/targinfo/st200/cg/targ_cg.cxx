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


/* ====================================================================
 * ====================================================================
 *
 * Module: cgtarget.cxx
 *
 * Description:
 *
 * Support routines for target-specific code generator functionality.
 *
 * ====================================================================
 * ====================================================================
 */

#include <ctype.h>
#include "W_alloca.h"

#include "defs.h"
#include "util.h"
#include "config.h"
#include "config_TARG.h"
#include "config_asm.h"
#include "erglob.h"
#include "ercg.h"
#include "tracing.h"
#include "data_layout.h"
#include "const.h"
#include "wn.h"
#include "import.h"
#include "opt_alias_interface.h"
#include "opt_alias_mgr.h"
#include "cgir.h"
#include "cg.h"
#include "void_list.h"
#include "whirl2ops.h"
#include "w2op.h"
#include "cgtarget.h"
#include "calls.h"
#include "cgexp.h"
#include "config_asm.h"
#include "hb_sched.h"
#include "targ_isa_variants.h"
#include "cgemit.h"
#include "targ_cg_private.h"

/* for asm stmt */
#include "register_preg.h"

#ifdef TARG_ST
#include "../../../gccfe/gnu/st200/insn-config.h" /* for MAX_RECOG_OPERANDS */
#endif

/* Import from targ_cgemit.cxx. */
extern void CGEMIT_Qualified_Name(ST *st, vstring *buf);

UINT32 CGTARG_branch_taken_penalty;
BOOL CGTARG_branch_taken_penalty_overridden = FALSE;

#ifdef TARG_ST
UINT32 CGTARG_max_issue_width;
BOOL CGTARG_max_issue_width_overriden = FALSE;
#endif

UINT32 CGTARG_NUM_SPILL_TYPES = 0;
TY_IDX CGTARG_Spill_Type[MTYPE_MAX_LIMIT+1];
CLASS_INDEX CGTARG_Spill_Mtype[MTYPE_MAX_LIMIT+1];
static TOP *CGTARG_Invert_Table;

static ISA_EXEC_UNIT_PROPERTY template_props[ISA_MAX_BUNDLES][ISA_BUNDLE_MAX_SLOTS];

static BOOL earliest_regclass_use_initialized = FALSE;
#ifdef TARG_ST
static INT earliest_regclass_use[ISA_REGISTER_CLASS_MAX_LIMIT+1];
static INT earliest_regsubclass_use[ISA_REGISTER_SUBCLASS_MAX_LIMIT+1];
static INT earliest_dedicated_reg_use[ISA_REGISTER_CLASS_MAX_LIMIT+1][ISA_REGISTER_MAX+1];
static INT earliest_assigned_reg_use[ISA_REGISTER_CLASS_MAX_LIMIT+1][ISA_REGISTER_MAX+1];
#else
static INT earliest_regclass_use[ISA_REGISTER_CLASS_MAX+1];
static INT earliest_regsubclass_use[ISA_REGISTER_SUBCLASS_MAX+1];
static INT earliest_dedicated_reg_use[ISA_REGISTER_CLASS_MAX+1][ISA_REGISTER_MAX+1];
static INT earliest_assigned_reg_use[ISA_REGISTER_CLASS_MAX+1][ISA_REGISTER_MAX+1];
#endif

/* ====================================================================
 *   CGTARG_Preg_Register_And_Class
 * ====================================================================
 */
BOOL
CGTARG_Preg_Register_And_Class (
  WN_OFFSET preg,
  ISA_REGISTER_CLASS *p_rclass,
  REGISTER *p_reg
)
{
  ISA_REGISTER_CLASS rclass = 0, rclassi;
  INT regnum;
  REGISTER reg;
  
  /* Get the target register number and class associated with the
   * preg, if there is one that is.
   */
  FOR_ALL_ISA_REGISTER_CLASS( rclassi ) {
    int preg_min = CGTARG_Regclass_Preg_Min(rclassi);
    int preg_max = CGTARG_Regclass_Preg_Max(rclassi);
    if (preg >= preg_min && preg <= preg_max) {
      regnum = preg - preg_min;
      rclass = rclassi;
      break;
    }
  }
  if (rclass == 0)
    return FALSE;

  /* Find the CG register for the target register and class.
   */
  for (reg = REGISTER_MIN; 
       reg <= REGISTER_CLASS_last_register(rclass); 
       reg++ ) {
    if (REGISTER_machine_id(rclass,reg) == regnum) {
      *p_reg = reg;
      *p_rclass = rclass;
      return TRUE;
    }
  }

  FmtAssert(FALSE, ("failed to map preg %d", preg));
  /*NOTREACHED*/
}

/* ====================================================================
 *   OP_br_variant
 *
 *   TODO: isa_properties ...
 * ====================================================================
 */
VARIANT
OP_br_variant (
  const OP *op
)
{
 TOP opcode=OP_code(op);

#define CASE_TOPB(top) \
          case TOP_##top##_r_r_b: case TOP_##top##_i_r_b: case TOP_##top##_ii_r_b: \
          case TOP_##top##_r_r_r: case TOP_##top##_i_r_r: case TOP_##top##_ii_r_r

#define CASE_TOPF(top) \
          case TOP_##top##_r_r_r: case TOP_##top##_r_r_b

  switch (opcode) {
  CASE_TOPB(cmpeq):
    return V_BR_I4EQ;

  CASE_TOPB(cmpge):
    return V_BR_I4GE;

  CASE_TOPB(cmpgeu):
    return V_BR_U4GE;

  CASE_TOPB(cmpgt):
    return V_BR_I4GT;

  CASE_TOPB(cmpgtu):
    return V_BR_U4GT;

  CASE_TOPB(cmple):
    return V_BR_I4LE;

  CASE_TOPB(cmpleu):
    return V_BR_U4LE;

  CASE_TOPB(cmplt):
    return V_BR_I4LT;

  CASE_TOPB(cmpltu):
    return V_BR_U4LT;

  CASE_TOPB(cmpne):
    return V_BR_I4NE;
    
  CASE_TOPF(cmpeqf_n):
    return V_BR_FEQ;

  CASE_TOPF(cmpgtf_n):
    return V_BR_FGT;

  CASE_TOPF(cmpgef_n):
    return V_BR_FGE;

  CASE_TOPF(cmplef_n):
    return V_BR_FLE;

  CASE_TOPF(cmpltf_n):
    return V_BR_FLT;

  case TOP_mtb_r_b:  // [SC] Checked mtb
  case TOP_convib_r_b:
    return V_BR_I4NE0;

#undef CASE_TOPF
#undef CASE_TOPB
  }

  return V_BR_NONE;
}


/* ====================================================================
 *   OP_cmp_variant
 *
 *   Returns the variant for the interpretation of the semantic of
 *   opcodes having the TOP_is_cmp property.
 *   Must be interpreted as:
 *   result is true if OU_opnd1 <variant> OU_opnd2 is true
 * ====================================================================
 */

VARIANT
OP_cmp_variant(const OP* op)
{
  TOP top = OP_code(op);
#define CASE_TOPB(top) \
          case TOP_##top##_r_r_b: case TOP_##top##_i_r_b: case TOP_##top##_ii_r_b: \
          case TOP_##top##_r_r_r: case TOP_##top##_i_r_r: case TOP_##top##_ii_r_r

  switch (top) {
  CASE_TOPB(cmpeq):
    return V_CMP_EQ;

  CASE_TOPB(cmpne):
    return V_CMP_NE;

  CASE_TOPB(cmpge):
    return V_CMP_GE;

  CASE_TOPB(cmpgeu):
    return V_CMP_GEU;

  CASE_TOPB(cmpgt):
    return V_CMP_GT;

  CASE_TOPB(cmpgtu):
    return V_CMP_GTU;

  CASE_TOPB(cmple):
    return V_CMP_LE;

  CASE_TOPB(cmpleu):
    return V_CMP_LEU;

  CASE_TOPB(cmplt):
    return V_CMP_LT;

  CASE_TOPB(cmpltu):
    return V_CMP_LTU;

  CASE_TOPB(andl):
    return V_CMP_ANDL;

  CASE_TOPB(nandl):
    return V_CMP_NANDL;

  CASE_TOPB(orl):
    return V_CMP_ORL;

  CASE_TOPB(norl):
    return V_CMP_NORL;
  }

  FmtAssert(0, ("OP_cmp_variant undefined for TOP %s", TOP_Name(top)));
  return V_CMP_NONE;
#undef CASE_TOPB
}

/* ====================================================================
 *   TOP_cond_variant
 *
 *   Returns the variant for the interpretation of the OU_condition operand
 *   for an opcode having the TOP_is_select or TOP_is_cond semantic.
 *
 *   Possible values of variants are V_COND_TRUE, V_COND_FALSE or V_COND_UNKNOWN.
 *   See lai/variants.h for the actual semantic.
 *
 * ====================================================================
 */

VARIANT
TOP_cond_variant(TOP top)
{
#define CASE_TOP(top) \
          case TOP_##top##_r_r_b_r: case TOP_##top##_i_r_b_r: case TOP_##top##_ii_r_b_r
#define CASE_BR(top) \
          case TOP_##top##_i_b

  switch (top) {
  CASE_TOP(slct):
  case TOP_st240_slct_r_r_b_r:
  CASE_BR(br):
    return V_COND_TRUE;

  CASE_TOP(slctf):  
  CASE_BR(brf):
    return V_COND_FALSE;
  }
  FmtAssert(0, ("TOP_cond_variant undefined for TOP %s", TOP_Name(top)));
  return V_COND_NONE;
#undef CASE_BR
#undef CASE_TOP
}


/* ====================================================================
 *   Make_Branch_Conditional
 *
 *   If a BB ends in an unconditional branch, turn it into a 
 *   conditional branch with TRUE predicate, so we can predicate with 
 *   something else later.
 *   If we can't find an unconditional branch, just give up and do 
 *   nothing
 * ====================================================================
 */
void
Make_Branch_Conditional (
  BB *bb,
  TN *pred_tn,     // cond TN for branching
  BOOL cond        // whether branches on cond TN TRUE or FALSE
)
{
  OP *new_br;
  TOP new_top;
  OP* br_op = BB_branch_op(bb);
  OPS ops = OPS_EMPTY;
  
  if (!br_op) return;

  if (TN_register_class(pred_tn) != ISA_REGISTER_CLASS_branch) {
    TN *br_tn = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch);
    Exp_COPY(br_tn, pred_tn, &ops);
    pred_tn = br_tn;
  }

  switch (OP_code(br_op)) {
  case TOP_goto_i:
    new_top = cond ? TOP_br_i_b : TOP_brf_i_b;
    break;
  case TOP_igoto:
    FmtAssert(FALSE,("Can't handle igoto -> contitional"));
    new_top = TOP_UNDEFINED;
    break;
  default:
    FmtAssert(FALSE,("Can't handle %s", TOP_Name(OP_code(br_op))));
    return;
  }

  // Make a tmp cond register so we can replace it later
  new_br = Mk_OP(new_top, pred_tn, OP_opnd(br_op,0));
  OP_srcpos(new_br) = OP_srcpos(br_op);
  OPS_Append_Op(&ops, new_br);
  BB_Insert_Ops_After(bb, br_op, &ops);
  BB_Remove_Op(bb, br_op);
}

/* ====================================================================
 *   CGTARG_Parallel_Compare
 * ====================================================================
 */
TOP
CGTARG_Parallel_Compare (
  OP* cmp_op, 
  COMPARE_TYPE ctype
)
{
  //
  // None for floats.  For gp registers, only support "eq" and "ne"
  // for arbitrary register compares.  Others must have one operand
  // be the zero register.
  //
  if (OP_flop(cmp_op)) {
    return TOP_UNDEFINED;
  }

  FmtAssert(FALSE,("CGTARG_Parallel_Compare: not implemented"));

  return TOP_UNDEFINED;
}

/* ====================================================================
 *   CGTARG_Unconditional_Compare
 * ====================================================================
 */
static BOOL 
CGTARG_Unconditional_Compare_Helper (
  TOP top, 
  TOP* uncond_ver
)
{
  // There is no unconditional compares on the ST100 for the
  // moment:
  switch (top) {

  default:
    *uncond_ver = TOP_UNDEFINED;
  }
  return FALSE;
}

BOOL CGTARG_Unconditional_Compare(OP *op, TOP* uncond_ver)
{
  return (CGTARG_Unconditional_Compare_Helper(OP_code(op),uncond_ver));
}

/* ====================================================================
 *   CGTARG_Compute_Branch_Parameters
 * ====================================================================
 */
void 
CGTARG_Compute_Branch_Parameters (
  INT32 *mispredict, 
  INT32 *fixed, 
  INT32 *brtaken, 
  double *factor
)
{
  *mispredict = 0;
  *fixed = 0;
  *brtaken = 0;
  *factor = 0.0;

  // For ST220/ST231 targets we have:
  // - no hardware prediction
  // - fixed branch cost is null
  // - taken branch cost is 1
  *mispredict= 0; *fixed= 0; *brtaken= 1; *factor = 1.0;

 /*
  * override for command line options
  *	-CG:mispredicted_branch=N
  *	-CG:mispredicted_factor=N
  */
  if (CG_branch_mispredict_penalty >= 0)
    *mispredict= CG_branch_mispredict_penalty ;

  if (CG_branch_mispredict_factor >= 0)
    *factor= CG_branch_mispredict_factor * (.01);
}

/* ====================================================================
 *   CGTARG_Branch_Info
 * ====================================================================
 */
void
CGTARG_Branch_Info ( 
  const OP  *op,
  INT *tfirst,            /* Which operand is the first target? */
  INT *tcount             /* How many target operands are there? */
)
{
  INT i;
  TN *tn;

  /* Initialize results: */
  *tfirst = -1;
  *tcount = 0;

  /* Find the first target: */
  for ( i = 0; ; i++ ) {
    if ( i >= OP_opnds(op) ) return;
    tn = OP_opnd(op,i);
    if ( tn != NULL && TN_is_label(tn) ) break;
  }
  *tfirst = i;

  /* Count the targets: */
  *tcount = 1;
  for ( i++; i < OP_opnds(op); i++ ) {
    tn = OP_opnd(op,i);
    if ( tn == NULL || ! TN_is_label(tn) ) return;
    (*tcount)++;
  }
  return;
}

/* ====================================================================
 *   CGTARG_Analyze_Branch
 *
 *   'tn1' is TN that is set to TRUE if the branch is taken;
 *   'tn2' is TN that is set to TRUE is the branch falls through.
 *   TODO: but CFLOW crushes, so for now always return the cond. TN
 *         in 'tn1'.
 * ====================================================================
 */
VARIANT
CGTARG_Analyze_Branch (
  OP *br,
  TN **tn1,
  TN **tn2
)
{
  VARIANT variant;
  TOP top = OP_code(br);

  *tn1 = NULL;
  *tn2 = NULL;

  switch (top) {
  case TOP_br_i_b:
    variant = V_BR_P_TRUE;
    Set_V_true_br(variant);
    *tn1 = OP_opnd(br, 0);
    break;
  case TOP_brf_i_b:
    variant = V_BR_P_TRUE;
    Set_V_false_br(variant);
    // Should really return 'tn2' but until CFLOW is fixed ...
    //*tn2 = OP_opnd(br, 0);
    *tn1 = OP_opnd(br, 0);
    break;
  case TOP_goto_i:
  case TOP_igoto:
  case TOP_return:
    variant = V_BR_ALWAYS;
    *tn1 = NULL;
    break;

  default:
    FmtAssert(FALSE, ("not sure what to do"));
    variant = V_BR_NONE;
    break;
  }

  return variant;
}

/* ====================================================================
 *   CGTARG_Analyze_Compare
 * ====================================================================
 */
VARIANT
CGTARG_Analyze_Compare (
  OP *br,
  TN **tn1,
  TN **tn2,
  OP **compare_op
)
{
  VARIANT variant;
  TN *cond_tn1;
  TN *cond_tn2;
  BOOL is_double;
  OP *def_op;
  DEF_KIND kind;
  BOOL false_br;
  
  /* Classify the condition based on the branch instruction. */
  /* [CG]: on ST200 branch variant may have V_BR_FALSE set. */
  variant = CGTARG_Analyze_Branch(br, &cond_tn1, &cond_tn2);
  false_br = V_false_br(variant);

#if 0
  /*
   * Check for double precision float compare.
   */
  if (cond_tn1 && TN_is_float(cond_tn1) && TN_size(cond_tn1) == 8) {
    is_double = TRUE;
  } else {
    is_double = FALSE;
  }
#endif

  *compare_op = NULL;
  *tn1 = NULL;
  *tn2 = NULL;
  variant = V_BR_NONE;

  /* Attempt to find the defining OP for the tested value.
   */
  def_op = TN_Reaching_Value_At_Op(cond_tn1, br, &kind, TRUE);
  
  if (def_op != NULL) {
    variant = OP_br_variant(def_op);
    if (variant != V_BR_NONE) {
      cond_tn1 = OP_opnd(def_op, 0);
      cond_tn2 = OP_opnds(def_op) >= 2 ? OP_opnd(def_op, 1): NULL;

      /* [CG] : Set false branch variant if needed. */
      if (false_br) Set_V_false_br(variant);
      *compare_op = def_op;
      *tn1 = cond_tn1;
      *tn2 = cond_tn2;
    }
  }
  return variant;
}

/* ====================================================================
 *   CGTARG_Get_unc_Variant
 * ====================================================================
 */
TOP 
CGTARG_Get_unc_Variant (
  TOP top
)
{
  TOP result;

  CGTARG_Unconditional_Compare_Helper(top, &result);
  
  if (result == TOP_UNDEFINED) {
    result = top;
  }
  
  return (result);
}

/* ====================================================================
 *   st200_encode_extract_mask
 * ====================================================================
 */
UINT
st200_encode_extract_mask (UINT bit_size, UINT bit_offset,
			   TOP *extract_op)
{
  if (bit_size > 16) {
    switch (*extract_op) {
    case TOP_extract_i_r_r: *extract_op = TOP_extractl_i_r_r; break;
    case TOP_extractu_i_r_r: *extract_op = TOP_extractlu_i_r_r; break;
    default:
      FmtAssert (FALSE,
		 ("Unexpected extract op (%d) in st200_encode_extract_mask",
		  *extract_op));
    }
    bit_size -= 16;
  }
  --bit_size;
  if (bit_size > 7) {
    // Force sign extension, otherwise we would have to use a long immediate.
    bit_size |= 0xfffffff8;
  }
  return (bit_size << 5) | bit_offset;
}

/* ====================================================================
 *   st200_decode_extract_mask
 * ====================================================================
 */
void
st200_decode_extract_mask (TOP extract_op, UINT extract_mask,
			   UINT *bit_size, UINT *bit_offset)
{
  *bit_offset = extract_mask & 0x1f;
  *bit_size = ((extract_mask >> 5) & 0xf) + 1;
  if (extract_op == TOP_extractl_i_r_r
      || extract_op == TOP_extractlu_i_r_r) {
    *bit_size += 16;
  }
}

/* ====================================================================
 *                           Properties:
 * ====================================================================
 */

TOP 
CGTARG_Invert(TOP opr)
{
  TOP top = CGTARG_Invert_Table[(INT)opr];
  if (! ISA_SUBSET_LIST_Member(ISA_SUBSET_List, top)) {
    top = TOP_UNDEFINED;
  }

  return top;
}

BOOL
CGTARG_Is_OP_Inter_RegClass_Bool_Copy(OP *op)
{
  return OP_code(op) == TOP_convbi_b_r || OP_code (op) == TOP_convib_r_b;
}

// [CL] try to invert an operation, able to deal with cases where it
// is not sufficient to invert the TOP, but also change operands
OP*
CGTARG_Invert_OP(OP* op)
{
  OP* new_op = NULL;

  TOP new_top = CGTARG_Invert(OP_code(op));
  TN *result = OP_result(op,0);
  TN* new_result = Build_TN_Like(result);

  if (new_top != TOP_UNDEFINED) {
    DevAssert(!OP_iand (op) && !OP_ior (op), ("try to invert a logif"));
    new_op = Dup_OP(op);
    OP_Change_Opcode(new_op, new_top);
    Set_OP_result(new_op, 0, new_result);
  } else if (OP_code(op) == TOP_convib_r_b) {
    TN *opnd = OP_opnd(op, 0);
    new_op = Mk_OP(TOP_cmpeq_r_r_b, new_result, opnd, Zero_TN);
  }
  return new_op;
}

OP*
CGTARG_Negate_OP(OP* op) {
  return CGTARG_Invert_OP(op);
}

/* ====================================================================
 *   CGTARG_Dependence_Required
 * ====================================================================
 */
BOOL
CGTARG_Dependence_Required (
  OP *pred_op, 
  OP *succ_op,
  INT *latency
)
{

  // Force asm to stay at end of block
  if (OP_code(succ_op) == TOP_asm) {
    *latency = 1;
    return TRUE;
  }

  if (FORCE_NOOPS
      && OP_code(succ_op) == TOP_syncins
      && OP_code(pred_op) == TOP_prgins) {
    *latency = 4;
    return TRUE;
  }

  // Arcs between an OP that defines a dedicated call register and the
  // call OP. I do it here because the usual REGIN/OUT mechanism
  // want me to tell it to which opnd the dependence is attached.
  // There is no opnd corresponding to dedicated call registers, so ...
  // I just hope that associating all of these to opnd 0 using MISC
  // arcs will work.

  if (!OP_call(succ_op)) return FALSE;

  INT i;
  BOOL need_dependence = FALSE;
  INT max_latency = 0;

  // First compute the max of result latencies for pred_op.
  for (i = 0; i < OP_results(pred_op); i++) {
    TN *result = OP_result(pred_op,i);
    if (TN_is_register(result) && TN_is_dedicated(result)) {
      REGISTER reg = TN_register(result);
      ISA_REGISTER_CLASS rclass = TN_register_class (result);
      INT cur_latency;
      
      // regs which have implicit meaning.
      if(REGISTER_SET_MemberP(REGISTER_CLASS_function_argument(rclass), reg) ||
	 REGISTER_SET_MemberP(REGISTER_CLASS_caller_saves(rclass), reg)) {
	
	need_dependence = TRUE;
	cur_latency = CGTARG_Max_RES_Latency(pred_op, i);
	// FdF 20041117: If not st220, set latency to 0 to allow
	// bundling with the call instruction (ddts 19668).
	if (!FORCE_NOOPS)
	  cur_latency = 0;
	if (cur_latency > max_latency) max_latency = cur_latency;
      }
    }
  }

  // Remove cyles lost by branch taken.
  max_latency -= 1 + CGTARG_Branch_Taken_Penalty();
  if (max_latency < 0) max_latency = 0;
  *latency = max_latency;

  return need_dependence;
}

/* ====================================================================
 *                          Local ASMs stuff:
 * ====================================================================
 */
#ifdef TARG_ST
// [TTh] Use gccfe limit for max ASM operands
static TN* asm_constraint_tn[MAX_RECOG_OPERANDS];
static ISA_REGISTER_SUBCLASS asm_constraint_sc[MAX_RECOG_OPERANDS];
static char asm_constraint_name[MAX_RECOG_OPERANDS][8];
#else
static TN* asm_constraint_tn[10];
static ISA_REGISTER_SUBCLASS asm_constraint_sc[10];
static char asm_constraint_name[10][8];
#endif
static INT asm_constraint_index;

/* ====================================================================
 *   CGTARG_Init_Asm_Constraints ()
 * ====================================================================
 */
void
CGTARG_Init_Asm_Constraints (void)
{
  // can use any type; it will be ignored
  Setup_Output_Parameter_Locations (MTYPE_To_TY(MTYPE_I8));
#ifdef TARG_ST
  for (INT i = 0; i < MAX_RECOG_OPERANDS; ++i)
#else
  for (INT i = 0; i < 10; ++i)
#endif
  {
    asm_constraint_tn[i] = NULL;
    asm_constraint_sc[i] = ISA_REGISTER_SUBCLASS_UNDEFINED;
    asm_constraint_name[i][0] = '\0';
  }
  asm_constraint_index = 0;
}

/* ====================================================================
 *   CGTARG_Modified_Asm_Opnd_Name (modifier, tn, tn_name)
 * ====================================================================
 */
char CGTARG_Asm_Opnd_Modifiers[] = { 'r' };
INT  CGTARG_Num_Asm_Opnd_Modifiers = 1;

const char* 
CGTARG_Modified_Asm_Opnd_Name(
  char modifier, 
  TN* tn, 
  char *tn_name
)
{
  if (modifier == 'r') {
    return tn_name;
  }
  else {
    FmtAssert(FALSE, ("Unknown ASM operand modifier '%c'", modifier));
  }
  /*NOTREACHED*/
}

// -----------------------------------------------------------------------
// Given a constraint for an ASM parameter, and the load of the matching
// argument passed to ASM (possibly NULL), choose an appropriate TN for it
// This function may return NULL, in this case the TN was not matched
// and the caller must emit a fatal error message.
// -----------------------------------------------------------------------
TN* 
CGTARG_TN_For_Asm_Operand (
  const char* constraint,            
  const WN* load,
  TN* pref_tn,
  ISA_REGISTER_SUBCLASS* subclass
)
{
  // skip constraint modifiers:
  // = input and output parameters are separated in the WHIRL for ASM
  // & early_clobber flag is set in Handle_ASM
  // % commutativity of operands is ignored for now
  static const char* modifiers = "=&%";
  while (strchr(modifiers, *constraint)) {
    constraint++;
  }
  
  // TODO: we should really look at multiple specified constraints
  // and the load in order to select the best TN, but for now we
  // assume that when we get here we can safely pick a TN

  // if 'm' is one of the choices, always prefer that one
  // TODO: we decide this in the front end, but it's not optimal
  if (*constraint != 'm') {
    const char* m = constraint;
    while (*++m) {
      if (*m == 'm') {
        constraint = m;
        break;
      }
    }
  }
  
  // prefer register/memory over immediates; this isn't optimal, 
  // but we may not always be able to generate an immediate
  static const char* immediates = "in";
  while (strchr(immediates, *constraint) && *(constraint+1)) {
    constraint++;
  }

  TN* ret_tn;
  
  // The 'n' constraints only accepts constant literal values.
  if (strchr("n", *constraint)) {
    if (load && WN_operator(load)==OPR_LDID && WN_class(load)==CLASS_PREG &&
	(WN_rtype(load) == MTYPE_I4 || WN_rtype(load) == MTYPE_U4)) {
      // immediate could have been put in preg by wopt
      load = Preg_Is_Rematerializable(WN_load_offset(load), NULL);
    }
    if (load && WN_operator(load) == OPR_INTCONST &&
	(WN_rtype(load) == MTYPE_I4 || WN_rtype(load) == MTYPE_U4)) {
      ret_tn = Gen_Literal_TN(WN_const_val(load), 
			      MTYPE_bit_size(WN_rtype(load))/8);
    } else {
      ErrMsg(EC_CG_Generic_Error, "input operand to 'asm' does not match constraint 'n'");
      return NULL;
    }
  }
  
  // The 'i' constraints accepts literals and symbolic values as the
  // address of a symbol.
  else if (strchr("i", *constraint)) {
    if (load && WN_operator(load)==OPR_LDID && WN_class(load)==CLASS_PREG &&
	(WN_rtype(load) == MTYPE_I4 || WN_rtype(load) == MTYPE_U4)) {
      // immediate could have been put in preg by wopt
      load = Preg_Is_Rematerializable(WN_load_offset(load), NULL);
    }
    if (load && WN_operator(load) == OPR_INTCONST &&
	(WN_rtype(load) == MTYPE_I4 || WN_rtype(load) == MTYPE_U4)) {
      ret_tn = Gen_Literal_TN(WN_const_val(load), 
                            MTYPE_bit_size(WN_rtype(load))/8);
    } else if (load && WN_operator(load) == OPR_LDA &&
	       (WN_rtype(load) == MTYPE_I4 || WN_rtype(load) == MTYPE_U4)) {
      ST *sym = WN_st(load);
      ret_tn = Gen_Symbol_TN (sym, 0, TN_RELOC_NONE);
    } else {
      ErrMsg(EC_CG_Generic_Error, "input operand to 'asm' does not match constraint 'i'");
      return NULL;
    }
  }

  // digit constraint means that we should reuse a previous operand
  else if (isdigit(*constraint)) {
    INT prev_index = *constraint - '0';
    FmtAssert(asm_constraint_tn[prev_index], 
              ("numeric matching constraint refers to NULL value"));
    ret_tn = asm_constraint_tn[prev_index];
  }
    
  else if (strchr("gmr", *constraint)) {
    TYPE_ID rtype = (load != NULL ? WN_rtype(load) : MTYPE_I4);
    if (rtype == MTYPE_I4 || rtype == MTYPE_U4) {
      ret_tn = (pref_tn ? pref_tn : Build_TN_Of_Mtype(rtype));
    } else {
      ErrMsg(EC_CG_Generic_Error, "operand type to 'asm' does not match constraint");
      return NULL;
    }
  }
  
  else if (strchr("b", *constraint)) {
    TYPE_ID rtype = MTYPE_B;
    TYPE_ID load_rtype = (load != NULL ?  WN_rtype(load) : MTYPE_I4);
    if (load_rtype == MTYPE_I4 || load_rtype == MTYPE_U4 ||
	load_rtype == MTYPE_B) {
      // whirl2ops.cxx is responsible of the [IU]4 -> B conversion if needed.
      ret_tn = (pref_tn ? pref_tn : Build_TN_Of_Mtype(rtype));
    } else {
      ErrMsg(EC_CG_Generic_Error, "operand type to 'asm' does not match constraint");
      return NULL;
    }      
  }

  else {
    FmtAssert(FALSE, ("ASM constraint <%s> not supported", constraint));
  }

  asm_constraint_tn[asm_constraint_index] = ret_tn;
  asm_constraint_index++;
  
  return ret_tn;
}

#ifdef TARG_ST
/* ====================================================================
 *   strstr_No_Digit_After
 *
 *   Find the first occurrence of the substring needle in the string
 *   haystack, that is not directly followed by a digit [0-9].
 * ====================================================================
 */
static char *
strstr_No_Digit_After(char *haystack, const char *needle) {
  char *p = haystack;
  while ((p = strstr(p, needle)) != NULL) {
    char* leftover = p + strlen(needle);
    if (!(*leftover >= '0' && *leftover <= '9')) {
      return (p);
    }
    p = leftover;
  }
  return (NULL);
}
#endif

/* ====================================================================
 *   Replace_Substring
 * ====================================================================
 */
static char* 
Replace_Substring (
  char* in, 
  char* from, 
  char* to
)
{
  UINT  buflen = strlen(in) + 64;
  char* buf = (char*) alloca(buflen);
  char* p;
#ifdef TARG_ST
  // [TTh] To handle more than 10 parameters in asm stmt, we must
  //       ignore, for instance, %10, %11, .. when searching for %1
  while ((p = strstr_No_Digit_After(in, from)) != NULL) {
#else
  while ((p = strstr(in, from)) != NULL) {
#endif
    char* leftover = p + strlen(from);
    *p = '\0';
    while (strlen(in) + strlen(to) + strlen(leftover) >= buflen) {
      buflen = buflen * 2;
      buf = (char*) alloca(buflen);
    }
    sprintf(buf, "%s%s%s", in, to, leftover);
    in = strdup(buf);
  }
  return in;
}

/* ====================================================================
 *   CGTARG_Modify_Asm_String
 * ====================================================================
 */
//TB add subclass parameter to CGTARG_Modify_Asm_String to handle
//registers that have different name depending on thier subclass
char* 
CGTARG_Modify_Asm_String (
  char* asm_string, 
  UINT32 position, 
  bool memory, 
  TN* tn,
  ISA_REGISTER_SUBCLASS sc)
{
  char* name;
  if (TN_is_register(tn)) {
    ISA_REGISTER_CLASS cl = TN_register_class(tn);
    REGISTER reg = TN_register(tn);
    name = (char*) REGISTER_name(cl, reg);
    if (memory) {
      char* buf = (char*) alloca(strlen(name)+3);
      sprintf(buf, "[%s]", name);
      name = buf;
    }
  }
  else {
    FmtAssert(!memory && (TN_is_symbol(tn) || TN_has_value(tn)),
              ("ASM operand must be a register, a literal constant or a symbolic constant"));
    if (TN_has_value(tn)) {
      char* buf = (char*) alloca(32);
      sprintf(buf, "%lld", TN_value(tn));
      name = buf;
    } else if (TN_is_symbol(tn) && 
	       ST_name(TN_var(tn)) &&
	       *(ST_name(TN_var(tn))) != '\0' &&
	       TN_offset(tn) == 0 && 
	       TN_relocs(tn) == TN_RELOC_NONE) {
      ST *st = TN_var(tn);
#ifdef TARG_ST
      // +128 is enough for all separators and suffixes added to the name
      vstring buf = vstr_begin(strlen(ST_name(st))+128+1);
      CGEMIT_Qualified_Name(st, &buf);
      name = (char*) alloca(strlen(vstr_str(buf))+1);
      strcpy(name, vstr_str(buf));
      vstr_end(buf);
#else
      const char *st_name = ST_name(st);
      // +128 is enough for all separators and suffixes added to the name
      char* buf = (char*) alloca(strlen(st_name)+128+1);
      // Code extracted from r_qualified_name() function in cgemit.cxx.
      if (ST_is_export_local(st) && ST_class(st) == CLASS_VAR) {
	if (ST_level(st) == GLOBAL_SYMTAB)
	  sprintf (buf, "%s%s%d", st_name,
		   Label_Name_Separator, ST_index(st));
	else
	  sprintf (buf, "%s%s%d%s%d", st_name,
		   Label_Name_Separator, ST_pu(Get_Current_PU_ST()),
		   Label_Name_Separator, ST_index(st));
      } else {
	sprintf (buf, "%s%s", st_name, Symbol_Name_Suffix);
      }
      name = buf;
#endif
    } else {
      FmtAssert(!memory && (TN_is_symbol(tn) || TN_has_value(tn)),
		("ASM operand is not a valid symbolic constant"));
    }
  }
  
  char pattern[5];
#ifdef TARG_ST
  // [TTh] Handle more than 10 parameters
  sprintf(pattern, "%%%d", position);
#else
  sprintf(pattern, "%%%c", '0'+position);
#endif
  
  asm_string =  Replace_Substring(asm_string, pattern, name);

  if (TN_is_register(tn)) {
    for (INT i = 0; i < CGTARG_Num_Asm_Opnd_Modifiers; i++) {
      char modifier = CGTARG_Asm_Opnd_Modifiers[i];
#ifdef TARG_ST
      // [TTh] Handle more than 10 parameters
      sprintf(pattern, "%%%c%d", modifier, position);
#else
      sprintf(pattern, "%%%c%c", modifier, '0'+position);
#endif
      name = (char*) CGTARG_Modified_Asm_Opnd_Name(modifier, tn, name);
      asm_string  = Replace_Substring(asm_string, pattern, name);
    }
  }
  
    
  return asm_string;
}

/* ====================================================================
 *
 * CGTARG_Postprocess_Asm_String
 *
 * ====================================================================
 */
void
CGTARG_Postprocess_Asm_String (
  char *s
)
{
}

/* ====================================================================
 * ====================================================================
 *                            ISA Helpers:
 * ====================================================================
 * ====================================================================
 */

/* ====================================================================
 *   CGTARG_Load_From_Memory
 * ====================================================================
 */
void CGTARG_Load_From_Memory (
  TN *tn, 
  ST *mem_loc, 
  OPS *ops,
  VARIANT variant
)
{
  TYPE_ID mtype = TY_mtype(ST_type(mem_loc));

  if (TN_register_class(tn) == ISA_REGISTER_CLASS_branch) {
    /* 
     * Since we can't directly load a predicate TN, first load into
     * an integer register and then set the predicate by checking for
     * a non-zero value.
     */
    TN *temp_tn = Build_TN_Of_Mtype (mtype);
    Exp_Load (MTYPE_I4, mtype, temp_tn, mem_loc, 0, ops, V_NONE);
    Build_OP(TOP_targ_mov_r_b, tn, temp_tn, ops);
    DevWarn("Spill of branch register: reload\n");
  }
  else {
    /* 
     * Non-predicate TNs are just a simple load.
     */
    Exp_Load (mtype, mtype, tn, mem_loc, 0, ops, variant);
  }

  return;
}

/* ====================================================================
 *   CGTARG_Store_To_Memory
 * ====================================================================
 */
void CGTARG_Store_To_Memory(
  TN *tn,
  ST *mem_loc,
  OPS *ops,
  VARIANT variant
)
{
  TYPE_ID mtype = TY_mtype(ST_type(mem_loc));

  if (TN_register_class(tn) == ISA_REGISTER_CLASS_branch) {
    /* 
     * Since we can't directly store a predicate TN, first copy to
     * an integer register and then store.
     */
    TN *temp_tn = Build_TN_Of_Mtype (mtype);
    Build_OP(TOP_targ_mov_b_r, temp_tn, tn, ops);
    Exp_Store (mtype, temp_tn, mem_loc, 0, ops, V_NONE);
    DevWarn("Spill of branch register: store\n");
  }
  else {
    /* 
     * Non-predicate TNs are just a simple store.
     */
    Exp_Store (mtype, tn, mem_loc, 0, ops, variant);
  }

  return;
}

/* ====================================================================
 *   CGTARG_Speculative_Load
 * ====================================================================
 */
TOP
CGTARG_Speculative_Load (OP *op)
{
  TOP opcode = OP_code(op);
  TOP ld = TOP_UNDEFINED;

  switch (opcode) {
  case TOP_ldw_r_i_r: 
    ld = TOP_ldw_d_r_i_r;
    break;
  case TOP_ldw_r_ii_r:
    ld = TOP_ldw_d_r_ii_r;
    break;
  case TOP_ldh_r_i_r:
    ld = TOP_ldh_d_r_i_r;
    break;
  case TOP_ldh_r_ii_r:
    ld = TOP_ldh_d_r_ii_r;
    break;
  case TOP_ldhu_r_i_r:
    ld = TOP_ldhu_d_r_i_r;
    break;
  case TOP_ldhu_r_ii_r:
    ld = TOP_ldhu_d_r_ii_r;
    break;
  case TOP_ldb_r_i_r:
    ld = TOP_ldb_d_r_i_r;
    break;
  case TOP_ldb_r_ii_r:
    ld = TOP_ldb_d_r_ii_r;
    break;
  case TOP_ldbu_r_i_r:
    ld = TOP_ldbu_d_r_i_r;
    break;
  case TOP_ldbu_r_ii_r:
    ld = TOP_ldbu_d_r_ii_r;
    break;
  }

  return ld;
}

/* ====================================================================
 *   CGTARG_Predicated_Store_Op
 * ====================================================================
 */
TOP
st200_Predicated_Store_Op (TOP opcode)
{
  TOP stw = TOP_UNDEFINED;

  switch (opcode) {
  case TOP_stl_p_r_i: 
    stw = TOP_stlc_p_b_r_i;
    break;
  case TOP_stl_p_r_ii:
    stw = TOP_stlc_p_b_r_ii;
    break;
  case TOP_stw_r_r_i: 
    stw = TOP_stwc_r_b_r_i;
    break;
  case TOP_stw_r_r_ii:
    stw = TOP_stwc_r_b_r_ii;
    break;
  case TOP_sth_r_r_i:
    stw = TOP_sthc_r_b_r_i;
    break;
  case TOP_sth_r_r_ii:
    stw = TOP_sthc_r_b_r_ii;
    break;
  case TOP_stb_r_r_i:
    stw = TOP_stbc_r_b_r_i;
    break;
  case TOP_stb_r_r_ii:
    stw = TOP_stbc_r_b_r_ii;
    break;
  }

  return stw;
}

/* ====================================================================
 *   CGTARG_Predicated_Store
 * ====================================================================
 */
TOP
CGTARG_Predicated_Store (OP *op)
{
  return st200_Predicated_Store_Op (OP_code(op));
}

/* ====================================================================
 *   CGTARG_Predicated_Load_Op
 * ====================================================================
 */
TOP
st200_Predicated_Load_Op (TOP opcode)
{
  TOP ld = TOP_UNDEFINED;

  switch (opcode) {
  case TOP_ldl_r_i_p: 
    ld = TOP_ldlc_r_i_b_p;
    break;
  case TOP_ldl_r_ii_p:
    ld = TOP_ldlc_r_ii_b_p;
    break;
  case TOP_ldw_r_i_r: 
    ld = TOP_ldwc_r_i_b_r;
    break;
  case TOP_ldw_r_ii_r:
    ld = TOP_ldwc_r_ii_b_r;
    break;
  case TOP_ldh_r_i_r:
    ld = TOP_ldhc_r_i_b_r;
    break;
  case TOP_ldh_r_ii_r:
    ld = TOP_ldhc_r_ii_b_r;
    break;
  case TOP_ldhu_r_i_r:
    ld = TOP_ldhuc_r_i_b_r;
    break;
  case TOP_ldhu_r_ii_r:
    ld = TOP_ldhuc_r_ii_b_r;
    break;
  case TOP_ldb_r_i_r:
    ld = TOP_ldbc_r_i_b_r;
    break;
  case TOP_ldb_r_ii_r:
    ld = TOP_ldbc_r_ii_b_r;
    break;
  case TOP_ldbu_r_i_r:
    ld = TOP_ldbuc_r_i_b_r;
    break;
  case TOP_ldbu_r_ii_r:
    ld = TOP_ldbuc_r_ii_b_r;
    break;
  case TOP_pft_r_i:
    ld = TOP_pftc_r_i_b;
    break;
  case TOP_pft_r_ii:
    ld = TOP_pftc_r_ii_b;
    break;
  }

  return ld;
}

/* ====================================================================
 *   CGTARG_Predicated_Load
 * ====================================================================
 */
TOP
CGTARG_Predicated_Load (OP *op)
{
  return st200_Predicated_Load_Op (OP_code (op));
}

/* ====================================================================
 *   CGTARG_Can_Fit_Immediate_In_Add_Instruction
 * ====================================================================
 */
BOOL
CGTARG_Can_Fit_Immediate_In_Add_Instruction (
  INT64 immed
)
{
  /* [CG]: Here we don't make assumption on how
     the code selector will treat extended immediate.
     So we assume that add does not have extended immediate
     form.
  */
  return ISA_LC_Value_In_Class (immed, LC_isrc2);
}

/* ====================================================================
 *   CGTARG_Can_Load_Immediate_In_Single_Instruction
 * ====================================================================
 */
BOOL
CGTARG_Can_Load_Immediate_In_Single_Instruction (
  INT64 immed
)
{
  return ISA_LC_Value_In_Class (immed, LC_xsrc2);
}

/* ====================================================================
 * CGTARG_Can_Change_To_Brlikely
 * ====================================================================
 */
BOOL CGTARG_Can_Change_To_Brlikely(OP *xfer_op, TOP *new_opcode)
{
  return FALSE;
}

/* ====================================================================
 *   CGTARG_Branches_On_True
 * ====================================================================
 */
BOOL
CGTARG_Branches_On_True (
  OP* br_op, 
  OP* cmp_op
)
{
  FmtAssert(FALSE,("CGTARG_Branches_On_True: not implemented"));

  //  if (OP_opnd(br_op, OP_PREDICATE_OPND) == OP_result(cmp_op, 0)) {
  //    return TRUE;
  //  }
  return FALSE;
}

/* ====================================================================
 *   CGTARG_Generate_Remainder_Branch
 * ====================================================================
 */
void
CGTARG_Generate_Remainder_Branch (
  TN *trip_count, 
  TN *label_tn,
  OPS *prolog_ops, 
  OPS *body_ops
)
{
  INT32 trip_size = TN_size(trip_count);

  if (TN_is_constant(trip_count)) {
    TN *lit_trip_count = trip_count;
    trip_count = CGTARG_gen_trip_count_TN(trip_size);
    Exp_Immediate(trip_count, lit_trip_count, TRUE, prolog_ops);
  }

  Exp_OP2(trip_size == 4 ? OPC_I4ADD : OPC_I8ADD,
	  trip_count,
	  trip_count,
	  Gen_Literal_TN(-1, trip_size),
	  body_ops);

  Exp_OP3v(OPC_TRUEBR,
	   NULL,
	   label_tn,
	   trip_count,
	   Zero_TN,
	   trip_size == 4 ? V_BR_I4NE : V_BR_I8NE,
	   body_ops);
}

/* ====================================================================
 *   CGTARG_Generate_Branch_Cloop
 * ====================================================================
 */
#ifdef TARG_ST
BOOL
CGTARG_Generate_Branch_Cloop(LOOP_DESCR *loop,
                             OP *br_op,
			     TN *trip_count_tn,
			     TN *label_tn,
			     OPS *prolog_ops,
			     OPS *body_ops)
#else
void
CGTARG_Generate_Branch_Cloop(OP *br_op,
			     TN *unrolled_trip_count,
			     TN *trip_count_tn,
			     INT32 ntimes,
			     TN *label_tn,
			     OPS *prolog_ops,
			     OPS *body_ops)
#endif
{ 
  // Related option: -TARG:activate_hwloop (Activate_Hwloop variable in config_TARG.cxx).
  return FALSE;
}


/* ====================================================================
 * ====================================================================
 *                 Scheduling related stuff:
 * ====================================================================
 * ====================================================================
 */

/* ====================================================================
 *   CGTARG_Max_OP_Latency
 * ====================================================================
 */
INT32
CGTARG_Max_OP_Latency (
  OP *op
)
{
  INT i;
  INT max_latency;

  max_latency = 0;

  for (i = 0; i < OP_results(op); i++) {
    INT latency = CGTARG_Max_RES_Latency(op, i); 
    if (latency > max_latency) max_latency = latency;
  }
  return max_latency;
}

static BOOL
is_constant_register (mUINT16 class_n_reg)
{
  return class_n_reg == CLASS_AND_REG_zero
    || class_n_reg == CLASS_AND_REG_fzero
    || class_n_reg == CLASS_AND_REG_fone;
}

static void
init_register_use (void)
{
  INT i;
  ISA_REGISTER_CLASS c;
  ISA_REGISTER_SUBCLASS sc;

  // Initialize earliest_regclass_use/earliest_regsubclass_use,
  // arrays containing the earliest cycle on which a register of
  // each class/subclass may be accessed as an operand.
  for (i = 0; i <= ISA_REGISTER_CLASS_MAX; i++) {
    earliest_regclass_use[i] = INT_MAX;
  }
  for (i = 0; i <= ISA_REGISTER_SUBCLASS_MAX; i++) {
    earliest_regsubclass_use[i] = INT_MAX;
  }

  // Iterate over all operations, setting
  // earliest_regclass_use and earliest_regsubclass use
  // to the minimum operand access times found.
  for (i = 0; i < TOP_count; i++) {
    if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, (TOP)i)
	&& !TOP_is_dummy ((TOP)i)) {
      const ISA_OPERAND_INFO *oinfo = ISA_OPERAND_Info ((TOP)i);
      const INT n_opnds = ISA_OPERAND_INFO_Operands (oinfo);
      for (INT opnd = 0; opnd < n_opnds; opnd++) {
	const ISA_OPERAND_VALTYP *otype = ISA_OPERAND_INFO_Operand (oinfo, opnd);
	if (ISA_OPERAND_VALTYP_Is_Register (otype)) {
	  ISA_REGISTER_CLASS c = ISA_OPERAND_VALTYP_Register_Class (otype);
	  ISA_REGISTER_SUBCLASS sc =
	    ISA_OPERAND_VALTYP_Register_Subclass (otype);
	  INT usetime = TSI_Operand_Access_Time ((TOP)i, opnd);
	  if (sc == ISA_REGISTER_SUBCLASS_UNDEFINED) {
	    if (usetime < earliest_regclass_use[c]) {
	      earliest_regclass_use[c] = usetime;
	    }
	  } else {
	    if (usetime < earliest_regsubclass_use[sc]) {
	      earliest_regsubclass_use[sc] = usetime;
	    }
	  }
	}
      }
    }
  }

  // Now set earliest_dedicated_reg_use and earliest_assigned_reg_use,
  // based on the class information.
  // For a register that lives in multiple subclasses, we have
  // to assume the earliest-use of all those subclasses.
  // For earliest_assigned_reg_use, singleton subclasses are filtered out:
  // dedicated registers must always be used for singleton subclass operands.
  FOR_ALL_ISA_REGISTER_CLASS (c) {
    REGISTER_SET rset = REGISTER_CLASS_universe (c);
    REGISTER reg;
    FOR_ALL_REGISTER_SET_members (rset, reg) {
      INT i = reg - REGISTER_MIN;
      earliest_dedicated_reg_use[c][i] = earliest_regclass_use[c];
      earliest_assigned_reg_use[c][i] = earliest_regclass_use[c];
    }
  }
  FOR_ALL_ISA_REGISTER_SUBCLASS(sc) {
    const ISA_REGISTER_SUBCLASS_INFO *info =
      ISA_REGISTER_SUBCLASS_Info (sc);
    ISA_REGISTER_CLASS c = ISA_REGISTER_SUBCLASS_INFO_Class (info);
    BOOL singleton = (ISA_REGISTER_SUBCLASS_INFO_Count (info) == 1);
    REGISTER_SET rset = REGISTER_SUBCLASS_members (sc);
    REGISTER reg;
    FOR_ALL_REGISTER_SET_members (rset, reg) {
      INT i = reg - REGISTER_MIN;
      if (earliest_dedicated_reg_use[c][i] > earliest_regsubclass_use[sc]) {
	earliest_dedicated_reg_use[c][i] = earliest_regsubclass_use[sc];
      }
      if (! singleton
	  && earliest_assigned_reg_use[c][i] > earliest_regsubclass_use[sc]) {
	earliest_assigned_reg_use[c][i] = earliest_regsubclass_use[sc];
      }
    }      
  }

  // Now recalculate earliest_regclass_use and
  // earliest_regsubclass_use as the minimum
  // earliest_assigned_reg_use for each of the members.
  FOR_ALL_ISA_REGISTER_CLASS (c) {
    REGISTER_SET rset = REGISTER_CLASS_universe (c);
    REGISTER reg;
    FOR_ALL_REGISTER_SET_members (rset, reg) {
      INT i = reg - REGISTER_MIN;
      if (earliest_assigned_reg_use[c][i] < earliest_regclass_use[c]) {
	earliest_regclass_use[c] = earliest_assigned_reg_use[c][i];
      }
    }
  }
  FOR_ALL_ISA_REGISTER_SUBCLASS(sc) {
    const ISA_REGISTER_SUBCLASS_INFO *info =
      ISA_REGISTER_SUBCLASS_Info (sc);
    ISA_REGISTER_CLASS c = ISA_REGISTER_SUBCLASS_INFO_Class (info);
    REGISTER_SET rset = REGISTER_SUBCLASS_members (sc);
    REGISTER reg;
    FOR_ALL_REGISTER_SET_members (rset, reg) {
      INT i = reg - REGISTER_MIN;
      if (earliest_assigned_reg_use[c][i] < earliest_regsubclass_use[sc]) {
	earliest_regsubclass_use[sc] = earliest_assigned_reg_use[c][i];
      }
    }
  }

#if 0
  // Debug
  FOR_ALL_ISA_REGISTER_CLASS (c) {
    printf ("Class %d: earliest use = %d\n", c, earliest_regclass_use[c]);
    REGISTER_SET rset = REGISTER_CLASS_universe (c);
    REGISTER reg;
    FOR_ALL_REGISTER_SET_members (rset, reg) {
      INT i = reg - REGISTER_MIN;
      printf ("  %d: earliest dedicated use = %d, assigned use = %d\n",
	      i, earliest_dedicated_reg_use[c][i],
	      earliest_assigned_reg_use[c][i]);
    }
  }
  FOR_ALL_ISA_REGISTER_SUBCLASS (sc) {
    printf ("Subclass %d: earliest use = %d\n", sc, earliest_regsubclass_use[sc]);
    const ISA_REGISTER_SUBCLASS_INFO *info =
      ISA_REGISTER_SUBCLASS_Info (sc);
    ISA_REGISTER_CLASS c = ISA_REGISTER_SUBCLASS_INFO_Class (info);
    REGISTER_SET rset = REGISTER_SUBCLASS_members (sc);
    REGISTER reg;
    FOR_ALL_REGISTER_SET_members (rset, reg) {
      INT i = reg - REGISTER_MIN;
      printf ("  %d: earliest dedicated use = %d, assigned use = %d\n",
	      i, earliest_dedicated_reg_use[c][i],
	      earliest_assigned_reg_use[c][i]);
    }
  }
  // End of debug
#endif

  earliest_regclass_use_initialized = TRUE;
}


static INT
get_earliest_regclass_use (ISA_REGISTER_CLASS c,
			   ISA_REGISTER_SUBCLASS sc)
{
  INT usetime;
  if (!earliest_regclass_use_initialized) {
    init_register_use();
  }
  if (sc == ISA_REGISTER_SUBCLASS_UNDEFINED) {
    usetime = earliest_regclass_use[c];
  } else {
    usetime = earliest_regsubclass_use[sc];
  }
  FmtAssert (usetime != INT_MAX, ("earliest_regclass_use not set for class = %d, subclass = %d\n", (int)c, (int)sc));
  return usetime;
}

static INT
get_earliest_dedicated_reg_use (ISA_REGISTER_CLASS rclass, REGISTER reg) {
  if (!earliest_regclass_use_initialized) {
    init_register_use();
  }
  INT usetime = earliest_dedicated_reg_use[rclass][reg - REGISTER_MIN];
  FmtAssert (usetime != INT_MAX, ("earliest_reg_use not set for class = %d, register = %d\n", (int)rclass, (int)reg));
  return usetime;
}

static INT
get_earliest_assigned_reg_use (ISA_REGISTER_CLASS rclass, REGISTER reg) {
  if (!earliest_regclass_use_initialized) {
    init_register_use();
  }
  INT usetime = earliest_assigned_reg_use[rclass][reg - REGISTER_MIN];
  FmtAssert (usetime != INT_MAX, ("earliest_reg_use not set for class = %d, register = %d\n", (int)rclass, (int)reg));
  return usetime;
}

/* ====================================================================
 *   CGTARG_Max_RES_Latency
 * ====================================================================
 */
INT32
CGTARG_Max_RES_Latency (
			OP *op,
			INT idx
			)
{
  //	asm latency always forced to 1.
  if (OP_code(op) == TOP_asm) {
    return 1;
  }
  //    Instructions writing into regiser r0.0 have a 0 latency.
  if (is_constant_register (TN_register_and_class (OP_result (op, idx)))) {
    return 0;
  }
  INT result_avail = TSI_Result_Available_Time (OP_code (op), idx);
  INT opnd_used;
  if (TN_register(OP_result(op,idx)) != REGISTER_UNDEFINED) {
    if (TN_is_dedicated (OP_result(op, idx))) {
      opnd_used = get_earliest_dedicated_reg_use(TN_register_class(OP_result(op,idx)),
						 TN_register(OP_result(op,idx)));
    } else {
      opnd_used = get_earliest_assigned_reg_use(TN_register_class(OP_result(op,idx)),
						TN_register(OP_result(op,idx)));
    }
  } else if (idx < OP_fixed_results(op))  {
    ISA_REGISTER_CLASS c = OP_result_reg_class (op, idx);
    ISA_REGISTER_SUBCLASS sc = OP_result_reg_subclass (op, idx);
    opnd_used = get_earliest_regclass_use (c, sc);
  } else {
    return 1;
  }
  
  INT latency = result_avail - opnd_used;
  // ALU ops reading r63 still need an extra cycle of latency because
  // there is no forwarding of the alu result to the SLR used by a branch op.
  // This is not handled in the machine description, so catch it here.
  if (TN_register_and_class(OP_result(op,idx)) == CLASS_AND_REG_ra
      && ! OP_load (op)) {
    latency++;
  }
  
  return latency;
}

/* ====================================================================
 *   CGTARG_Adjust_Latency
 * ====================================================================
 */
void
CGTARG_Adjust_Latency (
  OP *pred_op, 
  OP *succ_op, 
  CG_DEP_KIND kind, 
  UINT8 opnd, 
  INT16 *latency
)
{
  INT i;
  const TOP pred_code = OP_code(pred_op);
  const TOP succ_code = OP_code(succ_op);

  // 1. Instructions writing into a branch register must be followed
  //    by 2 cycle (bundle) before 
  //         TOP_br_i_b 
  //         TOP_brf_i_b 
  //    can be issued that uses this register.
  //	Treated by ti_si.


  // 2. Instructions writing into a PC must be followed
  //    by 1 cycle (bundle) before a TOP_br_i_b can be issued.
  //	Treated by ti_si.

  // 3. Load instructions (resp arith instruction) writing LR register
  //    must be followed by 3 cycles delay (resp 2 cycles delay) 
  //    before one of the following may be issued:
  //         TOP_icall
  //         TOP_igoto
  //         TOP_return
  //    (i.e. an indirect branch op).
  //    Treated by ti_si for loads.  Just need to handle arith
  //    instructions here.
  if (kind == CG_DEP_REGIN
      && OP_xfer (succ_op)
      && (TN_register_and_class (OP_opnd (succ_op, opnd))
	  == CLASS_AND_REG_ra)) {
    if (! OP_load (pred_op)) {
      *latency = MAX (*latency,
		      TSI_Result_Available_Time (pred_code, 0)
		      - TSI_Operand_Access_Time (succ_code, opnd) + 1);
    }
  }

  // 4. TOP_prgins must be followed by 3 cycles (bundles) before
  //    issueing a TOP_syncins instruction
  //	TODO ?

  // 5. Instructions writing SAVED_PC must be followed by 4 cycles
  //    (bundles) before issueing the TOP_rfi instruction.
  //	TODO ?

  // 6 ... more about TOP_rfi
  //	TODO ?

  // 7. Special asm latencies
  //    For input treated by the standard READ/WRITE latencies
  //	For output, force to 1.
  if (pred_code == TOP_asm) *latency = 1;

  // 8. Any REGOUT latency must be at least 1 on this target.
  if (kind == CG_DEP_REGOUT && *latency < 1) *latency = 1;
  
  // 9. Special latency on constant registers, even on output dependence.
  /* FdF: MBTst15896 Fixed operand for REGOUT. */
  if ((kind == CG_DEP_REGIN &&
       is_constant_register (TN_register_and_class (OP_opnd(succ_op, opnd))))
      || (kind == CG_DEP_REGANTI
	  && is_constant_register (TN_register_and_class (OP_opnd(pred_op, opnd))))
      || (kind == CG_DEP_REGOUT
	  && is_constant_register (TN_register_and_class (OP_result(succ_op, opnd)))))
    *latency = 0;

  return;
}

#if 0
/* ====================================================================
 *   CGTARG_ARC_Sched_Latency
 *
 *    	  Wrapper function for ARC_latency to let us fix up some cases 
 *	  where it returns a result that just doesn't make sence.  In
 *	  particular a latency of -1 for the pre-branch latency makes no
 *	  scheduling sense for CPUs which have same-cycle branch shadows.
 *	  Should be 0.
 *
 *   TODO: see if this functionality is redundant ?
 * ====================================================================
 */
INT CGTARG_ARC_Sched_Latency(
  ARC *arc
)
{
  if ( ARC_kind(arc) == CG_DEP_PREBR && PROC_has_same_cycle_branch_shadow() )
    return 0;
  else
    return ARC_latency(arc);
}
#endif

/* ====================================================================
 *   CGTARG_Bundle_Stop_Bit_Available
 * ====================================================================
 */
BOOL 
CGTARG_Bundle_Stop_Bit_Available(TI_BUNDLE *bundle, INT slot)
{
  // Return TRUE the stop-bit is already set.
  if (TI_BUNDLE_stop_bit(bundle, slot)) return TRUE;

  return TI_BUNDLE_Stop_Bit_Available(bundle, slot);
}

/* ====================================================================
 *   Get_Extended_Opcode
 * ====================================================================
 */
static TOP
Get_Extended_Opcode (
  TOP opcode
)
{
  return TOP_get_variant (opcode, VARATT_next_immediate);
}

/* =========================================================================
 *   CGTARG_Get_Info_For_Common_Base_Opt
 *   Get information on memory op used in common base pointer optimisation.
 * =========================================================================
 */
void
CGTARG_Get_Info_For_Common_Base_Opt( INT *min_offset_alignment, INT *min_offset, INT *max_offset) {
  *min_offset_alignment = 1; /* char offset alignment. No scaling constraint on offset. */
  *min_offset = ISA_LC_Min (LC_isrc2); /* LC_imm_isrc2 for word memory accesses. */
  *max_offset = ISA_LC_Max (LC_isrc2);
}

/* ====================================================================
 *   CGTARG_offset_is_extended
 *   Does this offset TN is an extended immediate operand ?
 * ====================================================================
 */
BOOL
CGTARG_offset_is_extended(TN *offset, INT64 *val) {

  if (TN_has_value(offset)) {
    *val = TN_value(offset);
    if (!ISA_LC_Value_In_Class(*val, LC_isrc2)) {
      return TRUE;
    }
  }
  else if (TN_is_symbol(offset)) {
    ST *st, *base_st;
    INT64 base_ofst;

    st = TN_var(offset);
    base_st = Base_Symbol (st);
    // SP/FP relative may actually fit into 9-bits
    if (base_st == SP_Sym || base_st == FP_Sym) {
      base_ofst = Base_Offset (st);
      *val = CGTARG_TN_Value (offset, base_ofst);
      if (!ISA_LC_Value_In_Class(*val, LC_isrc2)) {
	return TRUE;
      }
    }
  }

  return FALSE;
}
  
/* ====================================================================
 *   CGTARG_need_extended_Opcode
 *   Does this OP have extended immediate operand ?
 * ====================================================================
 */
BOOL
CGTARG_need_extended_Opcode(OP *op, TOP *etop) {
  INT i;

  BOOL extra_slot_reqd = FALSE;
  
  // [CG] 2004/11/15. Filter out simulated opcodes such as asm
  if (OP_simulated(op)) return FALSE;

  for (i = 0; i < OP_opnds(op); i++) {
    TN *opnd = OP_opnd(op, i);
    INT64 val;

    if (TN_is_constant(opnd)) {

      if (TN_has_value(opnd)) {
	val = TN_value(opnd);
	if (!ISA_LC_Value_In_Class(val, LC_isrc2)) {
	  extra_slot_reqd = TRUE;
	}
      }
      else if (TN_is_symbol(opnd)) {
	ST *st, *base_st;
	INT64 base_ofst;

	st = TN_var(opnd);
	base_st = Base_Symbol (st);
	// SP/FP relative may actually fit into 9-bits
	if (base_st == SP_Sym || base_st == FP_Sym) {
	  base_ofst = Base_Offset (st);
	  val = CGTARG_TN_Value (opnd, base_ofst);
	  if (!ISA_LC_Value_In_Class(val, LC_isrc2)) {
	    extra_slot_reqd = TRUE;
	  }
	}
	else if (TN_relocs(opnd)) {
	  // [SC] Anything with a non-null relocation needs
	  // an extra slot.
	  extra_slot_reqd = TRUE;
	}
	else if (ST_class(st) == CLASS_CONST) {
	  // Handle floating-point constants
	  if (MTYPE_is_float(TCON_ty(ST_tcon_val(st)))) {
	    switch(TCON_ty(ST_tcon_val(st))) {
	    case MTYPE_F4:  // Take the value as a 32bit bit pattern
	      val = TCON_v0(ST_tcon_val(st));
	      break;
	    default:
	      FmtAssert(FALSE,("only 32 bits floating point values are supported"));
	      break;
	    }
	    if (!ISA_LC_Value_In_Class(val, LC_isrc2)) {
	      extra_slot_reqd = TRUE;
	    }
	  } else {
	    extra_slot_reqd = TRUE;
	  }
	}
	else {
	  //
	  // must be a assembly resolved symbolic address (label) ? 
	  //
	  // If it is a PC-relative call, br, etc. we assume that 
	  // 23-bits allocated for it in the opcode is enough.
	  // TODO: investigate -- shouldn't we have generated a
	  // label TN ??
	  // Not xfer -- reserve the following slot
	  // clarkes: 030910: getpc expands to a call instruction,
	  // but is not really a transfer op.
	  //
	  if (!OP_xfer(op) && OP_code(op) != TOP_getpc) {
	    extra_slot_reqd = TRUE;
	  }
	}
      }

      // assume that labels fit into 23-bit, enums are not emitted ?

      break;
    }
  }

  *etop = TOP_UNDEFINED;
  if (extra_slot_reqd)
    *etop = Get_Extended_Opcode(OP_code(op));
  
  return extra_slot_reqd;
}

static int compare_prop (const void *p1, const void *p2)
{
  ISA_EXEC_UNIT_PROPERTY prop1 = *(ISA_EXEC_UNIT_PROPERTY *)p1;
  ISA_EXEC_UNIT_PROPERTY prop2 = *(ISA_EXEC_UNIT_PROPERTY *)p2;
  return (prop2 > prop1) - (prop1 > prop2);
}

static void
init_templates (void)
{
  for (INT t = 0; t < ISA_MAX_BUNDLES; ++t) {
    INT n_props = ISA_EXEC_Slot_Count(t);
    for (INT i = 0; i < n_props; ++i) {
      template_props[t][i] = ISA_EXEC_Slot_Prop(t, i);
    }
    qsort(template_props[t], n_props, sizeof(ISA_EXEC_UNIT_PROPERTY),
	  compare_prop);
  }
}

static INT
find_matching_template (ISA_EXEC_UNIT_PROPERTY bundle_props[ISA_BUNDLE_MAX_SLOTS],
			INT n_bundle_props,
			BOOL match_alignment,
			UINT32 pc,
			BOOL allow_subset)
{
  for (INT t = 0; t < ISA_MAX_BUNDLES; ++t) {
    if (ISA_EXEC_Base(t)/8 > DEFAULT_TEXT_ALIGNMENT)
      continue;
    if (match_alignment
	&& ((pc % (ISA_EXEC_Base(t)/8)) != (ISA_EXEC_Bias(t)/8))) {
      continue;
    }
    const ISA_EXEC_UNIT_PROPERTY *t_props = template_props[t];
    INT n_t_props = ISA_EXEC_Slot_Count(t);
    if (n_t_props > CGTARG_Max_Issue_Width()) {
      continue;
    }
    if (n_bundle_props == n_t_props
	&& memcmp(bundle_props, t_props,
		  sizeof(ISA_EXEC_UNIT_PROPERTY) * n_bundle_props) == 0) {
      // Perfect match.
      return t;
    } else if (allow_subset) {
      if (n_t_props < n_bundle_props) {
	// This template is too small to match.
	continue;
      }
      INT bundle_i = 0;
      INT template_i = 0;
      while (bundle_i < n_bundle_props) {
	while (bundle_props[bundle_i] < t_props[template_i]) {
	  template_i++;
	  if (template_i == n_t_props) {
	    // Failed to find a match.
	    break;
	  }
	}
	if (template_i == n_t_props) {
	  break;
	}
	if (bundle_props[bundle_i] > t_props[template_i]) {
	  // Failed to find a match.
	  break;
	}
	bundle_i++;
	template_i++;
      }
      if (bundle_i == n_bundle_props) {
	// Match found.
	return t;
      }
    }
  }
  return -1;
}

/* ====================================================================
 *   CGTARG_Bundle_Slot_Available
 * ====================================================================
 */
BOOL 
CGTARG_Bundle_Slot_Available(TI_BUNDLE              *bundle,
			     OP                     *op,
			     INT                     slot,
			     ISA_EXEC_UNIT_PROPERTY *prop, 
			     BOOL                    stop_bit_reqd,
			     const CG_GROUPING      *grouping
			     )
{
  INT i;

  if (TI_BUNDLE_slot_filled(bundle, slot)) {
    return FALSE;
  }
  if (slot > 0 && !TI_BUNDLE_slot_filled(bundle, slot - 1)) {
    return FALSE;
  }

  *prop = ISA_EXEC_Unit_Prop(OP_code(op));

  ISA_EXEC_UNIT_PROPERTY bundle_props[ISA_MAX_SLOTS];
  INT n_bundle_props = 0;
  
  FOR_ALL_SLOT_MEMBERS(bundle, i) {
    if (TI_BUNDLE_slot_filled(bundle, i)) {
      bundle_props[n_bundle_props++] = TI_BUNDLE_exec_property(bundle, i);
    } else if (i == slot) {
      bundle_props[n_bundle_props++] = *prop;
    }
  }
  qsort (bundle_props, n_bundle_props, sizeof(ISA_EXEC_UNIT_PROPERTY),
	 compare_prop);
  INT t = find_matching_template(bundle_props, n_bundle_props,
			     FALSE, 0, TRUE);
  if (t != -1) {
    Set_TI_BUNDLE_pack_code(bundle, t);
    return TRUE;
  }

  return FALSE;
}

/* ====================================================================
 *   CGTARG_Handle_Bundle_Hazard
 *
 *   Handle all target-specific bundle hazards in this routine.
 * ====================================================================
 */
void
CGTARG_Handle_Bundle_Hazard (OP                          *op, 
			     TI_BUNDLE                   *bundle, 
			     VECTOR                      *bundle_vector,
			     BOOL                        slot_avail, 
			     INT                         slot_pos, 
			     INT                         max_pos,
			     BOOL                        stop_bit_reqd,
			     ISA_EXEC_UNIT_PROPERTY      prop)
{
  INT ti_err = TI_RC_OKAY;
  INT template_bit = TI_BUNDLE_Return_Template(bundle);
  FmtAssert (template_bit != -1, ("Illegal template encoding"));

  //
  // fill with nops
  //
  OP *prev_op = NULL;
  BOOL bundle_full = TI_BUNDLE_Is_Full(bundle, &ti_err);
  FmtAssert(ti_err != TI_RC_ERROR, ("%s", TI_errmsg));

  // if the <bundle> is full, set the <end_group> marker appropriately.
  if (bundle_full) {

    // set <end_group> marker if the bundle is full
    if (prev_op) {
      // if we added a noop after:
      Set_OP_end_group(prev_op);
      VECTOR_Reset (*bundle_vector);
    }
    else {
      Set_OP_end_group(op);
      VECTOR_Reset (*bundle_vector);
    }
    //    *clock = *clock + 1;
  }

  return;
}

/* ====================================================================
 *   CGTARG_Finish_Bundle: fill empty slots with nops
 * ====================================================================
 */
void CGTARG_Finish_Bundle(OP                          *op, 
			  TI_BUNDLE                   *bundle)
{
  INT template_bit = TI_BUNDLE_Return_Template(bundle);
  FmtAssert (template_bit != -1, ("Illegal template encoding"));
  // [SC] No work to do here.  We perform a post-pass to
  // insert padding nops.
}

// Create a label '.'
static LABEL_IDX
Gen_Label_PC() {
  LABEL_IDX lab;
  LABEL *label;

  label = &New_LABEL(CURRENT_SYMTAB, lab);
  // FdF: Need the extra space, because cgemit checks if the first
  // character is a digit
  LABEL_Init (*label, Save_Str(" 1"), LKIND_DEFAULT);

  return lab;
}

// Replace the sequence ;; NOP ;; NOP ;; by ;; GOTO 1 ;;
static OP*
NOPs2Goto (
  OP *op,
  LABEL_IDX lab_PC
)
{
  OP *goto_op;

  goto_op = Mk_OP(TOP_goto_i, Gen_Label_TN(lab_PC, 0));
  BB_Insert_Op(OP_bb(op), op, goto_op, FALSE);
  
  // It is guarantee that OP_scyle(op) is correct in case it is the
  // last OP of the basic block, while OP_scycle(OP_prev(op)) may be
  // -1.
  OP_scycle(goto_op) = OP_scycle(op)-1;
  Set_OP_bundled (goto_op);
  Set_OP_end_group(goto_op);
  Set_OP_nop2goto(goto_op);

  BB_Remove_Op(OP_bb(op), OP_prev(op));
  BB_Remove_Op(OP_bb(op), op);

  return goto_op;
}

#define Bundle_is_NOP(op, start) (start && OP_end_group(op) && OP_noop(op))

// ======================================================================
//  Post-pass to replace a sequence of ;; nop ;; nop ;; by ;; goto 1 ;;
//  This is activated under option -CG:nop2goto=1
// ======================================================================
static void NOPs_to_GOTO (
  BB *bb
)
{
  OP *op = BB_first_op(bb);
  bool bundle_NOP; // Did we find a NOP bundle
  bool bundle_start; // Is the current OP the first one in a bundle
  LABEL_IDX label_PC;
  /* The number of bundles which must follow prgins, before a syncins
     may be issued.  The instructions introduced by the nop2goto
     transformation are syncins instructions, so we must take care not
     to issue them in the shadow of a prgins. */
#define PRGINS_SHADOW 3
  INT bundles_since_prgins = PRGINS_SHADOW;

  // Create a new label for each basic block. It would be possible to
  // create only one by PU, but the bundler works currently at the
  // basic block level.
  label_PC = Gen_Label_PC();

  // Initial state
  bundle_NOP = FALSE;
  bundle_start = TRUE;
  
  while (op != NULL) {

    if (OP_code(op) == TOP_prgins) {
      bundles_since_prgins = 0;
    } else if (bundle_start) {
      bundles_since_prgins++;
    }

    // Previous bundle is a NOP, and the current one also
    if (bundle_NOP && Bundle_is_NOP(op, bundle_start)
	&& bundles_since_prgins > PRGINS_SHADOW)
      op = NOPs2Goto(op, label_PC);

    // Set the state for the next operation.
    bundle_NOP = (Bundle_is_NOP(op, bundle_start));
    bundle_start = OP_end_group(op);
    op = OP_next(op);
  }
}

static UINT32
update_bundle_pc (
		  OP *op,
		  UINT32 pc
		  )
{
  if (Trace_HB) {
    fprintf (TFile, "update_bundle_pc: %lx: ", (long)pc);
    Print_OP_No_SrcLine (op);
    fprintf (TFile, "\n");
  }
	     
  if (OP_code(op) == TOP_asm) {
    pc = (pc + DEFAULT_FUNCTION_ALIGNMENT - 1) & ~(DEFAULT_FUNCTION_ALIGNMENT - 1);
  } else {
    const ISA_PACK_INFO *pinfo = ISA_PACK_Info(OP_code(op));
    pc += ISA_PACK_Inst_Words(OP_code(op)) * (ISA_PACK_INST_WORD_SIZE / 8);
  }
  return pc;
}

static bool trailing_nop_needed(BB *bb) {
  OP *op_last;

  if (!bb)
    return false;

  if (BB_succs(bb))
    return false;

  if ((op_last = BB_xfer_op (bb)) && OP_call(op_last)) {
    BB *bb_next = BB_next(bb);
    if (!bb_next) return true;

    if (BB_has_label(bb_next)) {  
      ANNOTATION *ant;
      for (ant = ANNOT_First(BB_annotations(bb_next), ANNOT_LABEL);
           ant != NULL;
           ant = ANNOT_Next(ant, ANNOT_LABEL)) {
        LABEL_IDX lab = ANNOT_label(ant);
        if (LABEL_kind(Label_Table[lab]) == LKIND_END_EH_RANGE) {
          if (WN_Call_Never_Return(CALLINFO_call_wn(ANNOT_callinfo(ANNOT_Get (BB_annotations(bb), ANNOT_CALLINFO))))) {
            if (!BB_next(bb_next) || BB_entry(BB_next(bb_next)))
                return true;
          }
        }
      }
    }
  }

  return false;
}

static OP *
Pad_Bundle (
	    UINT32 *pc,
	    OP *bundle_start,
            INT *free_slots_in_previous_bundle,
	    INT *scycle_delta
	    )
{
  ISA_EXEC_UNIT_PROPERTY bundle_props[ISA_MAX_SLOTS];
  INT n_bundle_props = 0;
  UINT32 bundle_start_pc = *pc;

  OP *last_in_bundle = bundle_start;
  while (!OP_end_group(last_in_bundle)) {
    last_in_bundle = OP_next(last_in_bundle);
    // FdF 20060327: The basic block may end with dummy ops, which are
    // never marked end_group
    if (last_in_bundle == NULL)
      return NULL;
  }
  OP *bundle_end = OP_next(last_in_bundle);
  OP *op;
  for (op = bundle_start; op != bundle_end; op = OP_next(op)) {
    if (!OP_dummy(op)) {
      ISA_EXEC_UNIT_PROPERTY prop = ISA_EXEC_Unit_Prop(OP_code(op));
      for (INT w = 0; w < ISA_PACK_Inst_Words(OP_code(op)); w++) {
	bundle_props[n_bundle_props++] = prop;
      }
      *pc = update_bundle_pc(op, *pc);
      if (OP_scycle(op) != -1) { OP_scycle(op) += *scycle_delta; }
    }
  }
  qsort(bundle_props, n_bundle_props, sizeof(ISA_EXEC_UNIT_PROPERTY),
	compare_prop);
  
  // Find a template that matches bundle_props.
  INT t;
  while ((t = find_matching_template(bundle_props, n_bundle_props,
				     TRUE, bundle_start_pc, FALSE)) == -1) {
    // No match, so insert a nop.
    OP *noop = Mk_OP(TOP_nop);
    Set_OP_end_group(noop);
    Set_OP_bundled(noop);
    if (n_bundle_props == CGTARG_Max_Issue_Width()) {
      // No room for nops in current bundle, so insert them in
      // previous bundle.
      if (*free_slots_in_previous_bundle == 0) {
	// No room for nops in previous bundle either, so create
	// a new bundle between current and previous.
	DevWarn("Creating nop bundle to satisfy alignment\n");
	*free_slots_in_previous_bundle = CGTARG_Max_Issue_Width() - 1;
	BB_Insert_Op_Before(OP_bb(bundle_start), bundle_start, noop);
	OP_scycle(noop) = OP_scycle(bundle_start);
	// Inserting a nop bundle bumps all the start cycles by 1.
	(*scycle_delta)++;
	for (op = bundle_start; op != bundle_end; op = OP_next(op)) {
	  if (OP_scycle(op) != -1) { OP_scycle(op)++; }
	}
      } else {
	OP *last_in_prev_bundle = OP_prev(bundle_start);
	OP_scycle(noop) = OP_scycle(last_in_prev_bundle);
	Reset_OP_end_group(last_in_prev_bundle);
	DevWarn("Inserting nop in previous bundle to satisfy alignment\n");
	BB_Insert_Op_After(OP_bb(last_in_prev_bundle), last_in_prev_bundle, noop);
      }
      bundle_start_pc = update_bundle_pc(noop, bundle_start_pc);
    } else {
      OP_scycle(noop) = OP_scycle(last_in_bundle);
      Reset_OP_end_group(last_in_bundle);
      BB_Insert_Op_After(OP_bb(last_in_bundle), last_in_bundle, noop);
      last_in_bundle = noop;
      for (INT w = 0; w < ISA_PACK_Inst_Words(OP_code(noop)); w++) {
	FmtAssert(n_bundle_props < CGTARG_Max_Issue_Width(),
		  ("Pad_Bundle: too many nops inserted\n"));
	bundle_props[n_bundle_props++] = ISA_EXEC_Unit_Prop(OP_code(noop));
	qsort(bundle_props, n_bundle_props, sizeof(ISA_EXEC_UNIT_PROPERTY),
	      compare_prop);
      }
    }
    *pc = update_bundle_pc(noop, *pc);
  }
  *free_slots_in_previous_bundle = (CGTARG_Max_Issue_Width() - n_bundle_props);
  return bundle_end;
}

static void Pad_Bundles_With_NOPs (
				   BB *bb,
				   INT *scycle_delta
)
{
  static UINT32 hot_addr = 0;
  static UINT32 cold_addr = 0;
  static PU_IDX pad_bundles_current_pu = 0;
  UINT32 *pc;

  // [SC] Moved here from cgemit, so that the nop is inserted before
  // bundle formation is complete.
  // (cbr) generalize for all regions
  // 20051010: Fix for ddts 23277: Insert a word at the end of a
  // function that ends with a procedure call (must be noreturn), so
  // that $r63 is set with an address in the current function (needed
  // for unwinding).
  if (trailing_nop_needed (bb)) {
    // Append a nop bundle to the bb.
    OP *noop = Mk_OP(TOP_nop);
    Set_OP_end_group(noop);
    Set_OP_bundled(noop);
    OP_scycle(noop) = OP_scycle(BB_last_op(bb));
    if (OP_scycle(noop) != -1) OP_scycle(noop)++;
    BB_Insert_Op_After(bb, BB_last_op(bb), noop);
  }
  if (pad_bundles_current_pu != ST_pu(Get_Current_PU_ST())) {
    cold_addr = 0;
    hot_addr = 0;
    pad_bundles_current_pu = ST_pu(Get_Current_PU_ST());
  }
  if (BB_Is_Cold(bb)) {
    pc = &cold_addr;
  } else {
    pc = &hot_addr;
  }

  INT align_bb = Check_If_Should_Align_BB(bb);
  if (align_bb > 1)
    *pc = ((*pc + align_bb - 1) / align_bb) * align_bb;
  OP *bundle_start = BB_first_op(bb);
  INT free_slots_in_previous_bundle = 0;
  while (bundle_start) {
    bundle_start = Pad_Bundle (pc, bundle_start, &free_slots_in_previous_bundle,
			       scycle_delta);
  }
}

/* ====================================================================
 *   CGTARG_Make_Bundles_Postpass
 * ====================================================================
 */
void CGTARG_Make_Bundles_Postpass(BB *bb)
{
  if (FORCE_NOOPS && CG_NOPs_to_GOTO) {
    NOPs_to_GOTO(bb);
  }
  INT scycle_delta = 0;
  Pad_Bundles_With_NOPs (bb, &scycle_delta);
}  

/* ====================================================================
 *   CGTARG_Pseudo_Make_Expand
 *
 *   Generate sequence that correspond to pseudo MAKE.
 * ====================================================================
 */
void
CGTARG_Pseudo_Make_Expand() {
  /* No pseudo MAKE (TOP_MAKE) selected on ST200. */
}

/* ====================================================================
 *   CGTARG_Resize_Instructions
 * ====================================================================
 */
void CGTARG_Resize_Instructions ()
{
  // Iterate over all the operations to change them to _ii when needed.
  for (BB *bb = REGION_First_BB; bb != NULL; bb = BB_next(bb)) {
    for (OP *op = BB_first_op(bb); op != NULL; op = OP_next(op)) {
      TOP etop;
      INT old_size = OP_inst_words(op), new_size = 1;
      if (old_size == 2)
	OP_Change_Opcode(op, TOP_get_variant(OP_code(op), VARATT_prev_immediate));
      if (CGTARG_need_extended_Opcode(op, &etop)) {
	Is_True(OP_inst_words(op) == 1, ("CGTARG_Resize_Instructions: Internal Error"));
	OP_Change_Opcode(op, etop);
	new_size = 2;
      }
      if (new_size > old_size)
	Reset_BB_scheduled(OP_bb(op));
    }
  }
}

/* ====================================================================
 *   CGTARG_Handle_Errata_Hazard
 * ====================================================================
 */
void
CGTARG_Handle_Errata_Hazard (OP *op, INT erratnum, INT ops_to_check)
{
}

/* ====================================================================
 *   CGTARG_Insert_Stop_Bits
 * ====================================================================
 */
void
CGTARG_Insert_Stop_Bits(BB *bb)
{
  return;
}

#if 0
/* ====================================================================
 *   CGTARG_Special_Min_II
 *
 *   Check for target specific (tail stepping, and other?) special
 *	  cases that might force a higher Min II. If a case applies, the
 *	  target specific MII is returned, otherwise 0 is returned.
 *
 * ====================================================================
 */
INT32 CGTARG_Special_Min_II(BB* loop_body, BOOL trace)
{
  return 0;
}
#endif

/* ====================================================================
 *   CGTARG_Perform_THR_Code_Generation (OP load_op, THR_TYPE type)
 *
 *   Perform THR (and target-specific) code generation tasks per
 *   THR_TYPE and OP. Depending on <type>, <op> can now be 
 *   substituted (or replaced with) new ops.
 * ====================================================================
 */
void 
CGTARG_Perform_THR_Code_Generation (
  OP *load_op, 
  THR_TYPE type
)
{
  if (type & THR_DATA_SPECULATION_NO_RB) {
    FmtAssert(OP_load(load_op), ("CGTARG_Perform_THR_Code_Generation : not a load OP"));
    INT enum_pos = -1;
#if 0
    BOOL float_op = OP_Is_Float_Mem(load_op) ? TRUE : FALSE;
    CGTARG_Return_Enum_TN(load_op, (float_op) ? ECV_fldtype : ECV_ldtype, &enum_pos);

    FmtAssert(enum_pos != -1, ("OP doesn't have enum operand"));
    
    TN *enum1_tn = (float_op) ?  Gen_Enum_TN(ECV_fldtype_a) :
                                 Gen_Enum_TN(ECV_ldtype_a);
    Set_OP_opnd(load_op, enum_pos, enum1_tn);
    Set_OP_speculative(load_op);               // set the OP_speculative flag.
    
    OP *check_load = Dup_OP(load_op);
    TN *enum2_tn;
    enum2_tn = (float_op) ? Gen_Enum_TN(ECV_fldtype_c_nc) : 
                            Gen_Enum_TN(ECV_ldtype_c_nc);
    Set_OP_opnd(check_load, enum_pos, enum2_tn); // set the enum TN, need to 
                                                 // add generic enum accessor.
    Set_OP_cond_def_kind(check_load, OP_ALWAYS_COND_DEF); 

    BB_Insert_Op_After(OP_bb(load_op), load_op, check_load);
#endif
  } else if (type & THR_DATA_SPECULATION_NO_RB_CLEANUP) {
#if 0
    INT check_base_idx = TOP_Find_Operand_Use(OP_code(load_op), OU_base);
    TN *check_base_tn = OP_opnd(load_op, check_base_idx);
    TN *adv_load_base_tn = OP_opnd(chk_load, check_base_idx);
    
    TN *check_result_tn = OP_result(load_op, 0);
    TN *adv_load_result_tn = OP_result(chk_load, 0);

    if (TNs_Are_Equivalent(check_result_tn, adv_load_result_tn) &&
	TNs_Are_Equivalent(check_base_tn, adv_load_base_tn)) {
      BB_Remove_Op(OP_bb(load_op), load_op);
      
      INT enum_pos = -1;
      BOOL float_op = OP_Is_Float_Mem(chk_load) ? TRUE : FALSE;
      TN *enum_tn = 
	CGTARG_Return_Enum_TN(chk_load,  
			      (float_op) ? ECV_fldtype_c_nc : ECV_ldtype_c_nc, 
			      &enum_pos); 

      FmtAssert(enum_tn, ("OP doesn't have enum operand"));
      Set_TN_enum(enum_tn, (float_op) ? ECV_fldtype : ECV_ldtype);   
    }
#endif
  }
}



/**********************************************************************
 *
 *             CGTARG_Interference implementation:
 *
 **********************************************************************
 */

static MEM_POOL interference_pool;
static VOID_LIST** writing;     /* writing[i] is a list of live ranges being
                                   written into registers in cycle i */
static BOOL is_loop;            /* Are we working on a loop? */
static INT32 assumed_longest_latency = 40;
                                /* We need to allocate <writing> to be somewhat
                                   longer than the number of cycles in the
                                   schedule in order to accommodate writes
                                   initiated near the end of the schedule.
                                   We'll check and grow this number as
                                   necessary. */
static INT32 cycle_count;       /* Number of cycles in the schedule under
                                   consideration. */
static void (*make_interference)(void*,void*);
                                /* Client's interference call back. */

/* ====================================================================
 *   Increase_Assumed_Longest_Latency
 *
 *   We need to increase our assumptions about the longest latency 
 *   operation in our target.  Also reallocate <writing>.
 *
 * ====================================================================
 */
static void
Increase_Assumed_Longest_Latency(INT32 new_longest_latency )
{
  DevWarn("Assumed longest latency should be at least %d",
          new_longest_latency);
  writing = TYPE_MEM_POOL_REALLOC_N(VOID_LIST*,&interference_pool,writing,
                                    cycle_count + assumed_longest_latency,
                                    cycle_count + new_longest_latency);
  assumed_longest_latency = new_longest_latency;
}

/* ====================================================================
 *   CGTARG_Interference_Required
 * ====================================================================
 */
BOOL CGTARG_Interference_Required(void)
{
  return FALSE;
}

/* ====================================================================
 *   CGTARG_Interference_Initialize
 * ====================================================================
 */
void
CGTARG_Interference_Initialize ( 
  INT32 cycle_count_local, 
  BOOL is_loop_local,
  void (*make_interference_local)(void*,void*) 
)
{
  static BOOL interference_pool_initialized = FALSE;

  if ( ! interference_pool_initialized ) {
    MEM_POOL_Initialize(&interference_pool,"Target_specific_interference",
                        TRUE);
    interference_pool_initialized = TRUE;
  }

  MEM_POOL_Push(&interference_pool);

  is_loop = is_loop_local;
  make_interference = make_interference_local;
  cycle_count = cycle_count_local;

  if ( is_loop )
    writing = TYPE_MEM_POOL_ALLOC_N(VOID_LIST*,&interference_pool,cycle_count);
  else
    writing = TYPE_MEM_POOL_ALLOC_N(VOID_LIST*,&interference_pool,
                                    cycle_count + assumed_longest_latency);
}

/* ====================================================================
 *   CGTARG_Result_Live_Range
 * ====================================================================
 */
void
CGTARG_Result_Live_Range ( 
  void* lrange, 
  OP* op, 
  INT32 offset 
)
{
  VOID_LIST* l;
  INT32 cycle = OP_scycle(op);

  FmtAssert(FALSE, ("CGTARG_Result_Live_Range: not implemented"));

  return;
}

/* ====================================================================
 *   CGTARG_Operand_Live_Range
 * ====================================================================
 */
void
CGTARG_Operand_Live_Range ( 
  void* lrange, 
  INT   opnd, 
  OP*   op, 
  INT32 offset 
)
{
  VOID_LIST* l;
  INT32 cycle = OP_scycle(op);
  INT32 reg_acc_cyc = TI_LATENCY_Operand_Access_Cycle(OP_code(op), opnd);

  if ( is_loop )
    reg_acc_cyc = Mod(reg_acc_cyc + cycle + offset,cycle_count);
  else {
    reg_acc_cyc += cycle + offset;

    if ( reg_acc_cyc >= cycle_count + assumed_longest_latency )
      return;   /* Nothing writing in this cycle anyway */
  }

  for ( l = writing[reg_acc_cyc]; l != NULL; l = VOID_LIST_rest(l) )
    make_interference(lrange,VOID_LIST_first(l));
}

/* ====================================================================
 *   CGTARG_Interference_Finalize
 * ====================================================================
 */
void
CGTARG_Interference_Finalize(void)
{
  MEM_POOL_Pop(&interference_pool);
  writing = (VOID_LIST **) -1;
}

/**********************************************************************
 *
 *                 Peak Rate Class (PRC):
 *
 **********************************************************************
 */

/* ====================================================================
 *
 * Reduce_Fraction
 *
 * Half hearted attempt to reduce a fraction. If we don't succeed
 * the only problem will be that we might round incorrectly on a
 * instruction rate.
 *
 * The algorithm is to first try the denominator as a factor and
 * then a few small primes.
 *
 * ====================================================================
 */
static void
Reduce_Fraction(INT frac[2])
{
  INT i;
  static const INT primes[] = {2, 3, 5, 7, 11, 13};
  INT n = frac[0];
  INT d = frac[1];
  INT p = d;

  if (d < -1 || d > 1) {
    for (i = sizeof(primes) / sizeof(primes[0]); ; p = primes[--i]) {
      while (n % p == 0 && d % p == 0) {
	n = n / p;
	d = d / p;
      }
      if (i == 0) break;
    }
  }

  frac[0] = n;
  frac[1] = d;
}

/* ====================================================================
 *
 * Harmonic_Mean
 *
 * Compute the harmonic weighted mean of two rates as follows:
 *
 *	  1        a                    b
 *	---- = ( ----- * a_rate ) + ( ----- * b_rate )
 *	mean     a + b                a + b
 *
 * Where:
 *
 *	"a" is the number of operations of class "a"
 *	"b" is the number of operations of class "b"
 *
 * ====================================================================
 */
static void
Harmonic_Mean(
  INT mean[2],
  INT a,
  const INT a_rate[2],
  INT b,
  const INT b_rate[2]
) {
  if (a == 0) {
    mean[0] = b_rate[0];
    mean[1] = b_rate[1];
  } else if (b == 0) {
    mean[0] = a_rate[0];
    mean[1] = a_rate[1];
  } else {
    mean[1] =   (a * a_rate[1] * b_rate[0]) 
	      + (b * b_rate[1] * a_rate[0]);
    mean[0] = (a + b) * a_rate[0] * b_rate[0];
    Reduce_Fraction(mean);
  }
}

/* ====================================================================
 *   CGTARG_Peak_Rate
 * ====================================================================
 */
void CGTARG_Peak_Rate ( 
  PEAK_RATE_CLASS prc, 
  PRC_INFO *info, 
  INT ratio[2] 
)
{
  ratio[0] = 1;
  ratio[1] = 1;
  
  switch (prc) {
  case PRC_INST:
    ratio[0] = 4;
    break;
  case PRC_MADD:
    ratio[0] = 2;
    break;
  case PRC_MEMREF:
    ratio[0] = 1;
    break;
  case PRC_FLOP:
  case PRC_FADD:
  case PRC_FMUL:
    ratio[0] = 4;
    break;
  case PRC_IOP:
    ratio[0] = 4;
    break;
  default:
    ratio[0] = 4;
    break;
  }
}

/* =======================================================================
 *   Plural
 *
 *   Return "s" if i != 1, "" otherwise.  Used to get the number of nouns
 *   right when printing.
 * =======================================================================
 */
#define Plural(i) ((i) != 1 ? "s" : "")

/* =======================================================================
 *   Percent_Of_Peak
 *
 *   Compute the percentage of peak instructions executed. Both the
 *   actual number of instructions executed and the peak attainable
 *   are expressed as a fraction of insts/cycle.
 * =======================================================================
 */
static INT
Percent_Of_Peak(INT numer, INT denom, INT peak[2])
{
  if (numer == 0) return 0;
  return (numer * peak[1] * 100) / ((denom * peak[0]) + peak[1] - 1);
}

/* =======================================================================
 *   CGTARG_Print_PRC_INFO
 *
 *   Print statistics for the PRC_INFO to a 'file'.
 * =======================================================================
 */
void
CGTARG_Print_PRC_INFO(
  FILE       *file,
  PRC_INFO   *info,
  INT32      ii,
  const char *prefix,
  const char *suffix
)
{
  char *s;
  INT madds_per_cycle[2];
  INT memrefs_per_cycle[2];
  INT flops_per_cycle[2];
  INT fadds_per_cycle[2];
  INT fmuls_per_cycle[2];
  INT iops_per_cycle[2];
  INT insts_per_cycle[2];
  INT insts = info->refs[PRC_INST];
  INT memrefs = info->refs[PRC_MEMREF];
  INT flops = info->refs[PRC_FLOP];
  INT madds = info->refs[PRC_MADD];
  INT fadds = info->refs[PRC_FADD];
  INT fmuls = info->refs[PRC_FMUL];
  INT iops = info->refs[PRC_IOP];

  CGTARG_Peak_Rate(PRC_INST, info, insts_per_cycle);
  CGTARG_Peak_Rate(PRC_MEMREF, info, memrefs_per_cycle);
  CGTARG_Peak_Rate(PRC_FLOP, info, flops_per_cycle);
  CGTARG_Peak_Rate(PRC_MADD, info, madds_per_cycle);
  CGTARG_Peak_Rate(PRC_FADD, info, fadds_per_cycle);
  CGTARG_Peak_Rate(PRC_FMUL, info, fmuls_per_cycle);
  CGTARG_Peak_Rate(PRC_IOP, info, iops_per_cycle);

  if (flops != 0) {
    BOOL unbalanced_fpu = FALSE;

    if ( madds_per_cycle[0] != 0 ) {
      fprintf(file,"%s%5d flop%1s        (%3d%% of peak) (madds count as 2)%s"
                   "%s%5d flop%1s        (%3d%% of peak) (madds count as 1)%s"
                   "%s%5d madd%1s        (%3d%% of peak)%s",
		 prefix,
		 flops + madds,
		 Plural(flops + madds),
		 Percent_Of_Peak(flops + madds, ii * 2, madds_per_cycle),
		 suffix,
		 prefix,
		 flops,
		 Plural(flops),
		 Percent_Of_Peak(flops, ii, flops_per_cycle),
		 suffix,
		 prefix,
		 madds,
		 Plural(madds),
		 Percent_Of_Peak(madds, ii, madds_per_cycle),
		 suffix);
    }
    else {
      fprintf(file,"%s%5d flop%1s        (%3d%% of peak)%s",
		 prefix,
		 flops,
		 Plural(flops),
		 Percent_Of_Peak(flops, ii, flops_per_cycle),
		 suffix);
    }

    if ( unbalanced_fpu ) {
      INT fmuls2_per_cycle[2]; /* combined fmul/madd peak rate */
      INT fadds2_per_cycle[2]; /* combined fadd/madd peak rate */
      INT fadds2 = fadds + madds;
      INT fmuls2 = fmuls + madds;

      Harmonic_Mean(fmuls2_per_cycle,
		    fmuls, fmuls_per_cycle,
		    madds, madds_per_cycle);
      Harmonic_Mean(fadds2_per_cycle,
		    fadds, fadds_per_cycle,
		    madds, madds_per_cycle);

      fprintf(file,"%s%5d fmul%1s        (%3d%% of peak)%s%s",
		 prefix,
		 fmuls2,
		 Plural(fmuls2),
		 Percent_Of_Peak(fmuls2, ii, fmuls2_per_cycle),
		 madds_per_cycle[0] ? " (madds count as 1)" : "",
		 suffix);
      fprintf(file,"%s%5d fadd%1s        (%3d%% of peak)%s%s",
		 prefix,
		 fadds2,
		 Plural(fadds2),
		 Percent_Of_Peak(fadds2, ii, fadds2_per_cycle),
		 madds_per_cycle[0] ? " (madds count as 1)" : "",
		 suffix);
    }
  }

  s = "";
  if (FALSE) {
    iops += memrefs;
    s = " (mem refs included)";
  }

  fprintf(file,"%s%5d mem ref%1s     (%3d%% of peak)%s"
               "%s%5d integer op%1s  (%3d%% of peak)%s%s"
               "%s%5d instruction%1s (%3d%% of peak)%s",
               prefix,
               memrefs,
               Plural(memrefs),
               Percent_Of_Peak(memrefs, ii, memrefs_per_cycle),
	       suffix,
               prefix,
               iops,
               Plural(iops),
               Percent_Of_Peak(iops, ii, iops_per_cycle),
	       s,
	       suffix,
               prefix,
               insts,
               Plural(insts),
               Percent_Of_Peak(insts, ii, insts_per_cycle),
	       suffix);
}



/* =======================================================================
 *   CGTARG_Compute_PRC_INFO
 *
 *   Compute some basic information about the given 'bb'. 
 * =======================================================================
 */
void
CGTARG_Compute_PRC_INFO(
  BB *bb,
  PRC_INFO *info
)
{
  OP *op;

  BZERO (info, sizeof (PRC_INFO));

  for ( op = BB_first_op(bb); op != NULL; op = OP_next(op) ) {
    INT num_insts = OP_Real_Ops (op);

    if (num_insts == 0) continue;

    // FdF: Because for LAO we add the annotation after the bundling.
    if (OP_noop(op)) continue;      

    info->refs[PRC_INST] += num_insts;

    if ( OP_flop(op) ) {
      BOOL is_single = (OP_result_size(op,0) == 32);

      ++info->refs[PRC_FLOP];
      info->refs[PRC_FLOP_S] += is_single;
      if (OP_madd(op)) {
        ++info->refs[PRC_MADD];
	info->refs[PRC_MADD_S] += is_single;
      }
      else if (OP_fadd(op) || OP_fsub(op)) {
	++info->refs[PRC_FADD];
	info->refs[PRC_FADD_S] += is_single;
      }
      else if (OP_fmul(op)) {
	++info->refs[PRC_FMUL];
	info->refs[PRC_FMUL_S] += is_single;
      }
    }
    else if (OP_memory(op))
      ++info->refs[PRC_MEMREF];
    else {
      INT k;

      /* Conditional moves and m[tf]c1 are not tagged as flops.
       * We certainly don't want to call them integer ops, so assume
       * anything that uses FP regs isn't an integer instruction.
       */
      if (OP_has_result(op) && TN_is_float(OP_result(op,0))) goto not_iop;

      for (k = 0; k < OP_opnds(op); k++) {
	if (TN_is_float(OP_opnd(op,k))) goto not_iop;
      }

      info->refs[PRC_IOP] += num_insts;

    not_iop:
      ;
    }
  }
}

/**********************************************************************
 *
 *                  Load immediate information
 *
 **********************************************************************
 */
BOOL CGTARG_is_expensive_load_imm(OP* op)
{
    return FALSE;
}

INT CGTARG_expensive_load_imm_immediate_index(OP* op)
{
    return -1;
}

INT CGTARG_expensive_load_imm_base_index(OP* op)
{
    return -1;
}


BOOL CGTARG_sequence_is_cheaper_than_load_imm(OPS* ops, OP* op)
{
    return ops && OPS_length(ops) <= 1;
}

BOOL CGTARG_should_factorize(OP* first, OP* last)
{
    return FALSE;
}

/**********************************************************************
 *
 *                  TOP to/from Multi conversion
 *
 **********************************************************************
 */
static TOP *TOP_To_Multi_Table = NULL;
static TOP *TOP_From_Multi_Table = NULL;
static TOP *TOP_Register_Variant_Table = NULL;

static void
Init_TOP_Variant_Tables() {
  TOP top, top_multi, top_immediate;
  TOP_To_Multi_Table   = TYPE_MEM_POOL_ALLOC_N(TOP, Malloc_Mem_Pool, (TOP_count + 1));
  TOP_From_Multi_Table = TYPE_MEM_POOL_ALLOC_N(TOP, Malloc_Mem_Pool, (TOP_count + 1));
  TOP_Register_Variant_Table = TYPE_MEM_POOL_ALLOC_N(TOP, Malloc_Mem_Pool, (TOP_count + 1));

  TOP_To_Multi_Table[TOP_UNDEFINED] = TOP_UNDEFINED;
  TOP_From_Multi_Table[TOP_UNDEFINED] = TOP_UNDEFINED;
  TOP_Register_Variant_Table[TOP_UNDEFINED] = TOP_UNDEFINED;

  for (top = 0; top < TOP_count; top++) {
	TOP_From_Multi_Table[top] = top;
	TOP_Register_Variant_Table[top] = TOP_UNDEFINED;
  }
  
  for (top = 0; top < TOP_count; top++) {
	top_multi = TOP_get_variant(top, VARATT_multi);
	
	if (top_multi != TOP_UNDEFINED) {
	  TOP_To_Multi_Table[top] = top_multi;
	  TOP_From_Multi_Table[top_multi] = top;
	}
	else {
	  TOP_To_Multi_Table[top] = top;
	}

#if 0
	// TODO_MERGE: merge variant model from stxp70 / must update MDS
	// Skip this for now for ST200 that uses ATT_register
	top_immediate = TOP_get_variant(top, VARATT_immediate);
	// In some description the immediate form reference itself. 
	// We must skip this.
	if (top != top_immediate) {
	  // Iterate over all the immediate forms.
	  while (top_immediate != TOP_UNDEFINED) {
	    DevAssert(TOP_Register_Variant_Table[top_immediate] == TOP_UNDEFINED, ("Unexpected duplicate register variant: immediate form: %s, register_form1: %s, register_form2: %s ", TOP_Name(top_immediate), TOP_Name(TOP_Register_Variant_Table[top_immediate]), TOP_Name(top)));
	    TOP_Register_Variant_Table[top_immediate] = top;
	    top_immediate = TOP_get_variant(top_immediate, VARATT_next_immediate);
	  }
	}
#endif
  }
}

/* =======================================================================
 *   CGTARG_TOP_To_Multi
 *
 *   Return the multi version of a given TOP
 * =======================================================================
 */
TOP CGTARG_TOP_To_Multi(TOP top) {
  return (TOP_To_Multi_Table[top]);
}

/* =======================================================================
 *   CGTARG_TOP_From_Multi
 *
 *   Return the standard version of a given multi TOP
 * =======================================================================
 */
TOP CGTARG_TOP_From_Multi(TOP multi_top) {
  return (TOP_From_Multi_Table[multi_top]);
}


/* =======================================================================
 *   CGTARG_TOP_Register_Variant
 *
 *   Return the register variant of the given immediate form top.
 *   This function must be kept private to targinfo.
 *   The user level function is TOP_opnd_register_variant().
 * =======================================================================
 */
TOP CGTARG_TOP_Register_Variant(TOP immform_top) {
  return TOP_Register_Variant_Table[immform_top];
}


/**********************************************************************
 *
 *                  Hardware Workarounds ??
 *
 **********************************************************************
 */
/* ====================================================================
 *   Hardware_Workarounds
 *
 *   Placeholder for all Hardware workarounds. 
 * ====================================================================
 */
void
Hardware_Workarounds(void)
{
}

/* ====================================================================
 * ====================================================================
 *                 Initialization / Termination
 * ====================================================================
 * ====================================================================
 */

/*
 * Initialize resources based on given issue width
 */
#ifdef TARG_ST
extern "C" void SI_RESOURCE_ID_Set_Max_Avail(UINT, INT);
#endif

static void
init_max_issue_width(UINT32 max_issue)
{
  INT max_resource = 0;
  INT id;
  for (id = 0; id < SI_resource_count; id++) {
    if (max_resource < SI_RESOURCE_ID_Avail_Per_Cycle(id)) {
      max_resource = SI_RESOURCE_ID_Avail_Per_Cycle(id);
    }
  }

  for (id = 0; id < SI_resource_count; id++) {
    INT new_avail = (SI_RESOURCE_ID_Avail_Per_Cycle(id) * max_issue) /
      max_resource;
    if (new_avail < 1) new_avail = 1;
    SI_RESOURCE_ID_Set_Max_Avail(id, new_avail);
  }
}


/* ====================================================================
 *   CGTARG_Initialize
 * ====================================================================
 */
void 
CGTARG_Initialize ()
{
  INT32 i;

  CGTARG_NUM_SPILL_TYPES = 0;

  // Spill type for integer registers
  CGTARG_Spill_Type[CGTARG_NUM_SPILL_TYPES] = Spill_Int_Type;
  CGTARG_Spill_Mtype[CGTARG_NUM_SPILL_TYPES] = Spill_Int_Mtype;
  CGTARG_NUM_SPILL_TYPES++;

  // Spill type for branch registers
  CGTARG_Spill_Type[CGTARG_NUM_SPILL_TYPES] = MTYPE_To_TY (MTYPE_I1);
  CGTARG_Spill_Mtype[CGTARG_NUM_SPILL_TYPES] = MTYPE_I1;
  CGTARG_NUM_SPILL_TYPES++;

  // Spill type for paired registers
  CGTARG_Spill_Type[CGTARG_NUM_SPILL_TYPES] = MTYPE_To_TY (MTYPE_I8);
  CGTARG_Spill_Mtype[CGTARG_NUM_SPILL_TYPES] = MTYPE_I8;
  CGTARG_NUM_SPILL_TYPES++;

  /* TODO: tabulate in the arch data base */

  /* Allocate table
   */
  CGTARG_Invert_Table = TYPE_MEM_POOL_ALLOC_N(TOP, Malloc_Mem_Pool, (TOP_count + 1));

  /* Init all table entries to TOP_UNDEFINED.
   */
  for(i = 0; i <= TOP_count; ++i) {
    CGTARG_Invert_Table[i]        = TOP_UNDEFINED;
  }

  /* Init table for CGTARG_Invert:
   */
#define CASE_TOP(top1,top2) \
  CGTARG_Invert_Table[TOP_##top1] = TOP_##top2; \
  CGTARG_Invert_Table[TOP_##top2] = TOP_##top1

#define CASE_TOP_BR(top1,top2) \
  CGTARG_Invert_Table[TOP_##top1##_i_b] = TOP_##top2##_i_b; \
  CGTARG_Invert_Table[TOP_##top2##_i_b] = TOP_##top1##_i_b

#define CASE_TOP_IR(top1,top2) \
  CGTARG_Invert_Table[TOP_##top1##_r] = TOP_##top2##_r; \
  CGTARG_Invert_Table[TOP_##top1##_i] = TOP_##top2##_i; \
  CGTARG_Invert_Table[TOP_##top1##_ii] = TOP_##top2##_ii; \
  CGTARG_Invert_Table[TOP_##top2##_r] = TOP_##top1##_r; \
  CGTARG_Invert_Table[TOP_##top2##_i] = TOP_##top1##_i; \
  CGTARG_Invert_Table[TOP_##top2##_ii] = TOP_##top1##_ii

#define CASE_TOP_SLCT(top1,top2) \
  CGTARG_Invert_Table[TOP_##top1##_r_r_b_r] = TOP_##top2##_r_r_b_r; \
  CGTARG_Invert_Table[TOP_##top1##_i_r_b_r] = TOP_##top2##_i_r_b_r; \
  CGTARG_Invert_Table[TOP_##top1##_ii_r_b_r] = TOP_##top2##_ii_r_b_r; \
  CGTARG_Invert_Table[TOP_##top2##_r_r_b_r] = TOP_##top1##_r_r_b_r; \
  CGTARG_Invert_Table[TOP_##top2##_i_r_b_r] = TOP_##top1##_i_r_b_r; \
  CGTARG_Invert_Table[TOP_##top2##_ii_r_b_r] = TOP_##top1##_ii_r_b_r

#define CASE_TOP_IRB(top1,top2) \
  CGTARG_Invert_Table[TOP_##top1##_r_r_b] = TOP_##top2##_r_r_b; \
  CGTARG_Invert_Table[TOP_##top2##_r_r_b] = TOP_##top1##_r_r_b; \
  CGTARG_Invert_Table[TOP_##top1##_i_r_b] = TOP_##top2##_i_r_b; \
  CGTARG_Invert_Table[TOP_##top2##_i_r_b] = TOP_##top1##_i_r_b; \
  CGTARG_Invert_Table[TOP_##top1##_ii_r_b] = TOP_##top2##_ii_r_b; \
  CGTARG_Invert_Table[TOP_##top2##_ii_r_b] = TOP_##top1##_ii_r_b; \
  CGTARG_Invert_Table[TOP_##top1##_r_r_r] = TOP_##top2##_r_r_r; \
  CGTARG_Invert_Table[TOP_##top2##_r_r_r] = TOP_##top1##_r_r_r; \
  CGTARG_Invert_Table[TOP_##top1##_i_r_r] = TOP_##top2##_i_r_r; \
  CGTARG_Invert_Table[TOP_##top2##_i_r_r] = TOP_##top1##_i_r_r; \
  CGTARG_Invert_Table[TOP_##top1##_ii_r_r] = TOP_##top2##_ii_r_r; \
  CGTARG_Invert_Table[TOP_##top2##_ii_r_r] = TOP_##top1##_ii_r_r

#define CASE_TOP_CMP(top1,top2) \
  CASE_TOP_IRB(top1,top2); \
  CGTARG_Invert_Table[TOP_st240_##top1##_r_b] = TOP_st240_##top2##_r_b; \
  CGTARG_Invert_Table[TOP_st240_##top2##_r_b] = TOP_st240_##top1##_r_b; \
  CGTARG_Invert_Table[TOP_st240_##top1##_i_b] = TOP_st240_##top2##_i_b; \
  CGTARG_Invert_Table[TOP_st240_##top2##_i_b] = TOP_st240_##top1##_i_b; \
  CGTARG_Invert_Table[TOP_st240_##top1##_ii_b] = TOP_st240_##top2##_ii_b; \
  CGTARG_Invert_Table[TOP_st240_##top2##_ii_b] = TOP_st240_##top1##_ii_b

#define CASE_TOP_CMPF(top1,top2) \
  CGTARG_Invert_Table[TOP_##top1##_r_r_r] = CGTARG_Invert_Table[TOP_##top2##_r_r_r]; \
  CGTARG_Invert_Table[TOP_##top2##_r_r_r] = CGTARG_Invert_Table[TOP_##top1##_r_r_r]; \
  CGTARG_Invert_Table[TOP_##top1##_r_r_b] = CGTARG_Invert_Table[TOP_##top2##_r_r_b]; \
  CGTARG_Invert_Table[TOP_##top2##_r_r_b] = CGTARG_Invert_Table[TOP_##top1##_r_r_b]

  CASE_TOP_BR(br,brf);
  CASE_TOP_SLCT(slct,slctf);
  if (Is_Target_st240 ()) {
    CASE_TOP(st240_slct_r_r_b_r,slctf_r_r_b_r);
  }
  CASE_TOP_IRB(cmpeq,cmpne);
  CASE_TOP_IRB(cmpge,cmplt);
  CASE_TOP_IRB(cmpgeu,cmpltu);
  CASE_TOP_IRB(cmpgt,cmple);
  CASE_TOP_IRB(cmpgtu,cmpleu);
  CASE_TOP_IRB(andl,nandl);
  CASE_TOP_IRB(orl,norl);
  CASE_TOP_CMPF(cmpgef_n,cmpltf_n);
  CASE_TOP_CMPF(cmpgtf_n,cmplef_n);

  targ_cg_init_targ_op ();

  init_templates();

  Init_TOP_Variant_Tables();

  if (CGTARG_max_issue_width_overriden) 
    init_max_issue_width(CGTARG_max_issue_width);

  return;
#undef CASE_TOP
#undef CASE_TOP_IR
#undef CASE_TOP_IRB
#undef CASE_TOP_CMP
#undef CASE_TOP_CMPF

}

BOOL 	 
CGTARG_Is_Simple_Jump(const OP* op)
{
    TOP top = OP_code(op);
    // not yet tested
    return top == TOP_br_i_b || top == TOP_brf_i_b ||top == TOP_goto_i;
}


/** 
 * Returns whether basic bloc contain loop init.
 * 
 * Note that this api is called from CFLOW 
 * @param bb 
 * 
 * @return Bool
 */

BOOL
CGTARG_bb_is_hwloop_init(const BB* bb)
{
   return FALSE;
}


/** 
 * Returns whether op may alias with a call.
 * 
 * Note that this api is called from LICM on non register allocated
 * code.
 * To avoid a strong degradation of performance this api should only
 * return false for instruction that do not have an impact on register
 * pressure and thus do not result in spill !
 *
 *
 * @param op 
 * 
 * @return 
 */
BOOL
CGTARG_op_may_alias_with_call(const OP* op)
{
  return TRUE;
}

BOOL
CGTARG_registerclass_may_be_copied(ISA_REGISTER_CLASS cl)
{
  return TRUE;
}

/** 
 * check target specific rules on whether op may be moved
 * into loophead
 * 
 * @param op 
 * @param loophead 
 * 
 * @return 
 */
BOOL
CGTARG_Code_Motion_To_LoopHead_Is_Legal(const OP* op, BB* loophead)
{
  return TRUE;
}

/** 
 *  Target specific function that does minor local changes to the asm
 * just before asm output.
 *
 * empty on st200.
 * 
 */
void
CGTARG_Fixup_ASM_Code ()
{
  
}

/** 
 * Verify that op has no loop sideeffects that avoids it to be moved
 * upward.
 * 
 * @param _loop 
 * @param bb
 * @param op 
 * 
 * @return true/false
 */
BOOL
CGTARG_OP_Has_Loop_Sideeffects(BB* bb, OP* op) {
  return false;
}

/** 
 * This function finds out the bundle template id corresponding to
 * the slot-mask. order_change variable specifies whether the order had
 * to be changed.
 * 
 * @param mask 
 * @param order_changed 
 * 
 * @return  bundle_id
 */
int CGTARG_Detect_Bundle_Id(ISA_EXEC_MASK slot_mask, BOOL* order_changed) {
  *order_changed = FALSE;
  return 0;
}

/** 
 * fix op in bundle slots before code emission if slot_mask does not
 * conform to bundle template.
 * 
 * @param slot_mask 
 * @param slot_op 
 * @param ibundle 
 */
void
CGTARG_FixBundle(ISA_EXEC_MASK slot_mask, OP *slot_op[], INT ibundle){ 
}
