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
 * $Revision$
 * $Date$
 * $Author$
 * $Source$
 *
 * Description:
 *
 * Support routines for target-specific code generator functionality.
 *
 * ====================================================================
 * ====================================================================
 */

#include <ctype.h>

#include "defs.h"
#include "util.h"
#include "config.h"
#include "config_TARG.h"
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


UINT32 CGTARG_branch_taken_penalty;
BOOL CGTARG_branch_taken_penalty_overridden = FALSE;

static TOP CGTARG_Invert_Table[TOP_count+1];

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
  ISA_REGISTER_CLASS rclass;
  INT regnum;
  REGISTER reg;

  /* Get the target register number and class associated with the
   * preg, if there is one that is.
   */
  if (Preg_Offset_Is_Int(preg)) {
    regnum = preg - Int_Preg_Min_Offset;
    rclass = ISA_REGISTER_CLASS_integer;
  }
  else if (preg >= Branch_Preg_Min_Offset && 
		            preg <= Branch_Preg_Max_Offset) {
    regnum = preg - Branch_Preg_Min_Offset;
    rclass = ISA_REGISTER_CLASS_branch;
  }
  else if (preg == 0) {
    FmtAssert(FALSE, ("preg = 0"));
    /* 0 not considered part of normal int group for error purposes,
     * but in our case it can be zero_tn. */
    regnum = 0;
    rclass = ISA_REGISTER_CLASS_integer;
  }
  else {
    return FALSE;
  }

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
 *   TOP_br_variant
 *
 *   TODO: isa_properties ...
 * ====================================================================
 */
VARIANT
TOP_br_variant (
  TOP opcode
)
{
  switch (opcode) {
  case TOP_cmpeq_r_b:
  case TOP_cmpeq_i_b:
  case TOP_cmpeq_ii_b:
  case TOP_cmpeq_r_r:
  case TOP_cmpeq_i_r:
  case TOP_cmpeq_ii_r:
    return V_BR_I4EQ;

  case TOP_cmpge_r_b:
  case TOP_cmpge_i_b:
  case TOP_cmpge_ii_b:
  case TOP_cmpge_r_r:
  case TOP_cmpge_i_r:
  case TOP_cmpge_ii_r:
    return V_BR_I4GE;

  case TOP_cmpgeu_r_b:
  case TOP_cmpgeu_i_b:
  case TOP_cmpgeu_ii_b:
  case TOP_cmpgeu_r_r:
  case TOP_cmpgeu_i_r:
  case TOP_cmpgeu_ii_r:
    return V_BR_U4GE;

  case TOP_cmpgt_r_b:
  case TOP_cmpgt_i_b:
  case TOP_cmpgt_ii_b:
  case TOP_cmpgt_r_r:
  case TOP_cmpgt_i_r:
  case TOP_cmpgt_ii_r:
    return V_BR_I4GT;

  case TOP_cmpgtu_r_b:
  case TOP_cmpgtu_i_b:
  case TOP_cmpgtu_ii_b:
  case TOP_cmpgtu_r_r:
  case TOP_cmpgtu_i_r:
  case TOP_cmpgtu_ii_r:
    return V_BR_U4GT;

  case TOP_cmple_r_b:
  case TOP_cmple_i_b:
  case TOP_cmple_ii_b:
  case TOP_cmple_r_r:
  case TOP_cmple_i_r:
  case TOP_cmple_ii_r:
    return V_BR_I4LE;

  case TOP_cmpleu_r_b:
  case TOP_cmpleu_i_b:
  case TOP_cmpleu_ii_b:
  case TOP_cmpleu_r_r:
  case TOP_cmpleu_i_r:
  case TOP_cmpleu_ii_r:
    return V_BR_U4LE;

  case TOP_cmplt_r_b:
  case TOP_cmplt_i_b:
  case TOP_cmplt_ii_b:
  case TOP_cmplt_r_r:
  case TOP_cmplt_i_r:
  case TOP_cmplt_ii_r:
    return V_BR_I4LT;

  case TOP_cmpltu_r_b:
  case TOP_cmpltu_i_b:
  case TOP_cmpltu_ii_b:
  case TOP_cmpltu_r_r:
  case TOP_cmpltu_i_r:
  case TOP_cmpltu_ii_r:
    return V_BR_U4LT;

  case TOP_cmpne_r_b:
  case TOP_cmpne_i_b:
  case TOP_cmpne_ii_b:
  case TOP_cmpne_r_r:
  case TOP_cmpne_i_r:
  case TOP_cmpne_ii_r:
    return V_BR_I4NE;
    
  case TOP_mtb:
    return V_BR_I4NE0;
  }

  return V_BR_NONE;
}


/* ====================================================================
 *   TOP_cmp_variant
 *
 *   Returns the variant for the interpretation of the semantic of
 *   opcodes having the TOP_is_cmp property.
 *   Must be interpreted as:
 *   result is true if OU_opnd1 <variant> OU_opnd2 is true
 * ====================================================================
 */
VARIANT
TOP_cmp_variant(TOP top)
{
  switch (top) {
  case TOP_cmpeq_r_b:
  case TOP_cmpeq_i_b:
  case TOP_cmpeq_ii_b:
  case TOP_cmpeq_r_r:
  case TOP_cmpeq_i_r:
  case TOP_cmpeq_ii_r:
    return V_CMP_EQ;

  case TOP_cmpne_r_b:
  case TOP_cmpne_i_b:
  case TOP_cmpne_ii_b:
  case TOP_cmpne_r_r:
  case TOP_cmpne_i_r:
  case TOP_cmpne_ii_r:
    return V_CMP_NE;

  case TOP_cmpge_r_b:
  case TOP_cmpge_i_b:
  case TOP_cmpge_ii_b:
  case TOP_cmpge_r_r:
  case TOP_cmpge_i_r:
  case TOP_cmpge_ii_r:
    return V_CMP_GE;

  case TOP_cmpgeu_r_b:
  case TOP_cmpgeu_i_b:
  case TOP_cmpgeu_ii_b:
  case TOP_cmpgeu_r_r:
  case TOP_cmpgeu_i_r:
  case TOP_cmpgeu_ii_r:
    return V_CMP_GEU;

  case TOP_cmpgt_r_b:
  case TOP_cmpgt_i_b:
  case TOP_cmpgt_ii_b:
  case TOP_cmpgt_r_r:
  case TOP_cmpgt_i_r:
  case TOP_cmpgt_ii_r:
    return V_CMP_GT;

  case TOP_cmpgtu_r_b:
  case TOP_cmpgtu_i_b:
  case TOP_cmpgtu_ii_b:
  case TOP_cmpgtu_r_r:
  case TOP_cmpgtu_i_r:
  case TOP_cmpgtu_ii_r:
    return V_CMP_GTU;

  case TOP_cmple_r_b:
  case TOP_cmple_i_b:
  case TOP_cmple_ii_b:
  case TOP_cmple_r_r:
  case TOP_cmple_i_r:
  case TOP_cmple_ii_r:
    return V_CMP_LE;

  case TOP_cmpleu_r_b:
  case TOP_cmpleu_i_b:
  case TOP_cmpleu_ii_b:
  case TOP_cmpleu_r_r:
  case TOP_cmpleu_i_r:
  case TOP_cmpleu_ii_r:
    return V_CMP_LEU;

  case TOP_cmplt_r_b:
  case TOP_cmplt_i_b:
  case TOP_cmplt_ii_b:
  case TOP_cmplt_r_r:
  case TOP_cmplt_i_r:
  case TOP_cmplt_ii_r:
    return V_CMP_LT;

  case TOP_cmpltu_r_b:
  case TOP_cmpltu_i_b:
  case TOP_cmpltu_ii_b:
  case TOP_cmpltu_r_r:
  case TOP_cmpltu_i_r:
  case TOP_cmpltu_ii_r:
    return V_CMP_LTU;

  case TOP_andl_r_b:
  case TOP_andl_i_b:
  case TOP_andl_ii_b:
  case TOP_andl_r_r:
  case TOP_andl_i_r:
  case TOP_andl_ii_r:
    return V_CMP_ANDL;

  case TOP_nandl_r_b:
  case TOP_nandl_i_b:
  case TOP_nandl_ii_b:
  case TOP_nandl_r_r:
  case TOP_nandl_i_r:
  case TOP_nandl_ii_r:
    return V_CMP_NANDL;

  case TOP_orl_r_b:
  case TOP_orl_i_b:
  case TOP_orl_ii_b:
  case TOP_orl_r_r:
  case TOP_orl_i_r:
  case TOP_orl_ii_r:
    return V_CMP_ORL;

  case TOP_norl_r_b:
  case TOP_norl_i_b:
  case TOP_norl_ii_b:
  case TOP_norl_r_r:
  case TOP_norl_i_r:
  case TOP_norl_ii_r:
    return V_CMP_NORL;
  }

  FmtAssert(0, ("TOP_cmp_variant undefined for TOP %s", TOP_Name(top)));
  return V_CMP_NONE;
}

/* ====================================================================
 *   TOP_cond_variant
 *
 *   Returns the variant for the interpretation of the OU_condition operand
 *   for an opcode having the TOP_is_select or TOP_is_cond semantic.
 *
 *   Possible values of variants are V_COND_TRUE or V_COND_FALSE.
 *   The select semantic is:
 *   (OU_condition == 0 ? OU_opnd1: OU_opnd2) if variant = V_COND_FALSE
 *   else OU_condition != 0 ? OU_opnd1: OU_opnd2) if variant != V_COND_FALSE
 *   The cond branch semantic is:
 *   branch taken if OU_condition == 0 if variant == V_COND_FALSE
 *   branch taken if OU_condition != 0 if variant != V_COND_FALSE
 *
 * ====================================================================
 */

VARIANT
TOP_cond_variant(TOP top)
{
  switch (top) {
  case TOP_slct_r:
  case TOP_slct_i:
  case TOP_slct_ii:
  case TOP_br:
    return V_COND_TRUE;

  case TOP_slctf_r:
  case TOP_slctf_i:
  case TOP_slctf_ii:
  case TOP_brf:
    return V_COND_FALSE;
  }
  FmtAssert(0, ("TOP_cond_variant undefined for TOP %s", TOP_Name(top)));
  return V_COND_NONE;
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
  case TOP_goto:
    new_top = cond ? TOP_br : TOP_brf;
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

  // For ST220/ST221/ST230 targets we have:
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
  case TOP_br:
    variant = V_BR_P_TRUE;
    Set_V_true_br(variant);
    *tn1 = OP_opnd(br, 0);
    break;
  case TOP_brf:
    variant = V_BR_P_TRUE;
    Set_V_false_br(variant);
    // Should really return 'tn2' but until CFLOW is fixed ...
    //*tn2 = OP_opnd(br, 0);
    *tn1 = OP_opnd(br, 0);
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
    variant = TOP_br_variant(OP_code(def_op));
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
 *                           Properties:
 * ====================================================================
 */

TOP 
CGTARG_Invert(TOP opr)
{
  return CGTARG_Invert_Table[(INT)opr];
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
static TN* asm_constraint_tn[10];
static ISA_REGISTER_SUBCLASS asm_constraint_sc[10];
static char asm_constraint_name[10][8];
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
  for (INT i = 0; i < 10; ++i) {
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
    if (load && WN_operator(load)==OPR_LDID && WN_class(load)==CLASS_PREG) {
      // immediate could have been put in preg by wopt
      load = Preg_Is_Rematerializable(WN_load_offset(load), NULL);
    }
    if (load && WN_operator(load) == OPR_INTCONST) {
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
    if (load && WN_operator(load)==OPR_LDID && WN_class(load)==CLASS_PREG) {
      // immediate could have been put in preg by wopt
      load = Preg_Is_Rematerializable(WN_load_offset(load), NULL);
    }
    if (load && WN_operator(load) == OPR_INTCONST) {
      ret_tn = Gen_Literal_TN(WN_const_val(load), 
                            MTYPE_bit_size(WN_rtype(load))/8);
    } else if (load && WN_operator(load) == OPR_LDA) {
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
  while ((p = strstr(in, from)) != NULL) {
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
char* 
CGTARG_Modify_Asm_String (
  char* asm_string, 
  UINT32 position, 
  bool memory, 
  TN* tn
)
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
    } else {
      FmtAssert(!memory && (TN_is_symbol(tn) || TN_has_value(tn)),
		("ASM operand is not a valid symbolic constant"));
    }
  }
  
  char pattern[4];
  sprintf(pattern, "%%%c", '0'+position);
  
  asm_string =  Replace_Substring(asm_string, pattern, name);

  if (TN_is_register(tn)) {
    for (INT i = 0; i < CGTARG_Num_Asm_Opnd_Modifiers; i++) {
      char modifier = CGTARG_Asm_Opnd_Modifiers[i];
      sprintf(pattern, "%%%c%c", modifier, '0'+position);
      name = (char*) CGTARG_Modified_Asm_Opnd_Name(modifier, tn, name);
      asm_string  = Replace_Substring(asm_string, pattern, name);
    }
  }
  
    
  return asm_string;
}

/* ====================================================================
 *
 * CGTARG_Postprocess_Asm_String: remove %Pn postincrement modifiers
 * %Pn says that operand n "may" be a postincrement, and if the
 * compiler wants to use a postincrement form, then put the postincrement
 * value here.  But we don't want to bother ever trying to do this,
 * so we'll just ignore it.
 *
 * ====================================================================
 */
void 
CGTARG_Postprocess_Asm_String (
  char *s
)
{
  char *p;
  while (TRUE) {
    p = strstr(s, "%P");
    if (p == NULL) break;
    // insert blanks cause easier than removing spaces
    *p = ' ';	// %
    ++p;
    *p = ' ';	// P
    ++p;
    *p = ' ';	// <number>
  }
}

/* ====================================================================
 * ====================================================================
 *                            ISA Helpers:
 * ====================================================================
 * ====================================================================
 */

/* ====================================================================
 *   CGTARG_Spill_Type
 *
 *   Given a TN to spill and the precision range of its contents,
 *   return the high-level type for the spill operation.
 * ====================================================================
 */
TY_IDX 
CGTARG_Spill_Type (
  TN *tn
) 
{
  switch (TN_register_class(tn)) {
    case ISA_REGISTER_CLASS_integer:
      return MTYPE_To_TY(MTYPE_I4);

    case ISA_REGISTER_CLASS_branch:
      return MTYPE_To_TY(MTYPE_I1);

    default:
      FmtAssert(FALSE,("CGTARG_Spill_Type: wrong TN register class"));
  }
}

/* ====================================================================
 *   CGTARG_Load_From_Memory
 * ====================================================================
 */
void CGTARG_Load_From_Memory (
  TN *tn, 
  ST *mem_loc, 
  OPS *ops
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
    Build_OP(TOP_mtb, tn, temp_tn, ops);
    DevWarn("Spill of branch register: reload\n");
  }
  else {
    /* 
     * Non-predicate TNs are just a simple load.
     */
    Exp_Load (mtype, mtype, tn, mem_loc, 0, ops, V_NONE);
  }

  return;
}

/* ====================================================================
 *   CGTARG_Store_To_Memory
 * ====================================================================
 */
void CGTARG_Store_To_Memory(TN *tn, ST *mem_loc, OPS *ops)
{
  TYPE_ID mtype = TY_mtype(ST_type(mem_loc));

  if (TN_register_class(tn) == ISA_REGISTER_CLASS_branch) {
    /* 
     * Since we can't directly store a predicate TN, first copy to
     * an integer register and then store.
     */
    TN *temp_tn = Build_TN_Of_Mtype (mtype);
    Build_OP(TOP_mfb, temp_tn, tn, ops);
    Exp_Store (mtype, temp_tn, mem_loc, 0, ops, V_NONE);
    DevWarn("Spill of branch register: store\n");
  }
  else {
    /* 
     * Non-predicate TNs are just a simple store.
     */
    Exp_Store (mtype, tn, mem_loc, 0, ops, V_NONE);
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
  case TOP_ldw_i: 
    ld = TOP_ldw_d_i;
    break;
  case TOP_ldw_ii:
    ld = TOP_ldw_d_ii;
    break;
  case TOP_ldh_i:
    ld = TOP_ldh_d_i;
    break;
  case TOP_ldh_ii:
    ld = TOP_ldh_d_ii;
    break;
  case TOP_ldhu_i:
    ld = TOP_ldhu_d_i;
    break;
  case TOP_ldhu_ii:
    ld = TOP_ldhu_d_ii;
    break;
  case TOP_ldb_i:
    ld = TOP_ldb_d_i;
    break;
  case TOP_ldb_ii:
    ld = TOP_ldb_d_ii;
    break;
  case TOP_ldbu_i:
    ld = TOP_ldbu_d_i;
    break;
  case TOP_ldbu_ii:
    ld = TOP_ldbu_d_ii;
    break;
  }

  return ld;
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
  return ISA_LC_Value_In_Class (immed, LC_s9);
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
  return ISA_LC_Value_In_Class (immed, LC_s32);
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
BOOL
CGTARG_Generate_Branch_Cloop(OP *br_op,
			     TN *unrolled_trip_count,
			     TN *trip_count_tn,
			     INT32 ntimes,
			     TN *label_tn,
			     OPS *prolog_ops,
			     OPS *body_ops)
{ 
  return FALSE;
  //FmtAssert(FALSE,("target does not support counted loop branches"));
}

/* ====================================================================
 * ====================================================================
 *                 Scheduling related stuff:
 * ====================================================================
 * ====================================================================
 */

/* ====================================================================
 *   CGTARG_Is_OP_Addr_Incr
 * ====================================================================
 */
BOOL
CGTARG_Is_OP_Addr_Incr (
  OP *op
)
{
  return OP_iadd(op) &&
         TN_has_value(OP_opnd(op, 1)) &&
         OP_result(op, 0) == OP_opnd(op, 0);
}

/* ====================================================================
 *   CGTARG_Is_OP_Cmp_Eq_Ne
 * ====================================================================
 */
BOOL
CGTARG_Is_OP_Cmp_Eq_Ne (
  OP *op
)
{
  TOP top = OP_code(op);

  return ((top == TOP_cmpeq_i_b) ||
	  (top == TOP_cmpeq_ii_b) ||
	  (top == TOP_cmpne_i_b) ||
	  (top == TOP_cmpne_ii_b));
}

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
  INT i;
  INT latency;

  //	asm latency always forced to 1.
  if (OP_code(op) == TOP_asm) {
    latency = 1;
  }
  else {
    INT result_avail = TSI_Result_Available_Time (OP_code (op), idx);
    INT first_opnd_read = INT_MAX;

    for (i = 0; i < OP_opnds(op); i++) {
      INT opnd_read = TSI_Operand_Access_Time (OP_code (op), i);
      if (opnd_read < first_opnd_read) {
	first_opnd_read = opnd_read;
      }
    }
    
    if (first_opnd_read == INT_MAX) {
      // An operation with results but no operands.
      first_opnd_read = 0;
    }
    latency = result_avail - first_opnd_read;
    
    //	Now handle special operand cases
    
    
    // Load instructions (resp arith instruction) writing LR register
    //    must be followed by 3 cycles delay (resp 2 cycles delay) 
    //    before one of the following may be issued:
    //         TOP_icall
    //         TOP_igoto
    //         TOP_return
    
    if (TN_register_and_class(OP_result(op,idx)) == CLASS_AND_REG_ra) {
      INT opnd_read = TSI_Operand_Access_Time (TOP_icall, 0);
      latency = MAX (latency, result_avail - opnd_read);
      // The previous calculation works for loads, but alu ops
      // writing R63 need an extra cycle, because there is no
      // forwarding of the alu result to the SLR used by a branch op.
      if (! OP_load (op)) {
	latency++;
      }
    }
    
    //    Instructions writing into a branch register must be followed
    //    by 2 cycle (bundle) before 
    //         TOP_br 
    //         TOP_brf 
    //    can be issued that uses this register.
    
    else if (TN_register_class(OP_result(op,idx)) == ISA_REGISTER_CLASS_branch) {
      latency = result_avail - TSI_Operand_Access_Time (TOP_br, 0);
    }

    //    Instructions writing into regiser r0.0 have a 0 latency.
    
    else if (is_constant_register (TN_register_and_class (OP_result (op, idx)))) {
      latency = 0;
    }
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
  //         TOP_br 
  //         TOP_brf 
  //    can be issued that uses this register.
  //	Treated by ti_si.


  // 2. Instructions writing into a PC must be followed
  //    by 1 cycle (bundle) before a TOP_br can be issued.
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

/* ======================================================================
 *   Delay_Scheduling_OP
 *
 *   Placeholder routine to check if placement of <op> at <slot_pos> in
 *   a <bundle> can be delayed.
 *
 * Arthur: I moved it from hb_hazards.cxx for two reasons:
 *         1. it is target dependent;
 *         2. I want to change the functionality: once function
 *            CGTARG_Bundle_Slot_Available() returns TRUE, I want
 *            the op to be guaranteed to be bundled. In this case,
 *            I can reserve stop bits (and eventually everything else
 *            in CGTARG_Bundle_Slot_Available().
 *
 * ======================================================================
 */
static BOOL
Delay_Scheduling_OP (
  OP *op, 
  INT slot_pos, 
  TI_BUNDLE *bundle
)
{

  // If <op> is a <xfer_op>, we would like to not greedily bundle it.
  // Rather, delay the same, so that nops (if necessary) can be 
  // inserted before (instead of after). As a result, any <xfer_op>
  // will be the last_op in a legal bundle.

  // If <op> needs to be the last member of the group, check for slot
  // positions and any prior stop bits present.
  if (OP_l_group(op) && (slot_pos != (ISA_MAX_SLOTS - 1)) &&
      !TI_BUNDLE_stop_bit(bundle, slot_pos))
    return TRUE;

  return FALSE;

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
  // Arthur: a hack -- I use the fact that all _ii opcodes follow
  //         immediately corresponding _i opcodes ...
  return (TOP) (opcode+1);
}

// [CL]
/* ====================================================================
 *   Twin_Slot: returns true if slot is used by an insns
 * that spans across two slots
 * ====================================================================
 */
static BOOL Twin_Slot(
	       TI_BUNDLE *bundle,
	       INT slot)
{
  switch(slot) {
  case 0:
    if (TI_BUNDLE_exec_property(bundle, slot) & ISA_EXEC_PROPERTY_EXT0_Unit)
      return TRUE;
    break;
  case 1:
    if (
	(TI_BUNDLE_exec_property(bundle, slot) & ISA_EXEC_PROPERTY_EXT0_Unit)
	||
	(TI_BUNDLE_exec_property(bundle, slot) & ISA_EXEC_PROPERTY_EXT1_Unit)
	)
      return TRUE;
    break;
  case 2:
    if (
	(TI_BUNDLE_exec_property(bundle, slot) & ISA_EXEC_PROPERTY_EXT1_Unit)
	||
	(TI_BUNDLE_exec_property(bundle, slot) & ISA_EXEC_PROPERTY_EXT2_Unit)
	)
      return TRUE;
    break;
  case 3:
    if (TI_BUNDLE_exec_property(bundle, slot) & ISA_EXEC_PROPERTY_EXT2_Unit)
      return TRUE;
    break;
  }
  return FALSE;
}

// [CL]
/* ====================================================================
 *   Is_Slot_Required: returns true if slot is used by an insn
 * that requires this particular slot number
 * ====================================================================
 */
static BOOL Is_This_Slot_Required(
	       TI_BUNDLE *bundle,
	       INT slot)
{
  switch(slot) {
  case 0:
    if (TI_BUNDLE_exec_property(bundle, slot) & ISA_EXEC_PROPERTY_ReqS0_Unit)
      return TRUE;
    break;
  }
  return FALSE;
}


/* ====================================================================
 *   Move_Slot: move property from 'from' slot to 'to' slot
 * ====================================================================
 */
static void Move_Slot(
	       TI_BUNDLE *bundle,
	       INT from,
	       INT to,
	       ISA_EXEC_UNIT_PROPERTY *to_prop
)
{
  // Get property of slot with which we swap
  ISA_EXEC_UNIT_PROPERTY new_prop = TI_BUNDLE_exec_property(bundle, from);

  // insn being move will have property of
  // destination, except for alignment constraints
  TI_BUNDLE_Reserve_Slot(bundle, to, 
			 *to_prop
			 & ~ISA_EXEC_PROPERTY_Odd_Unit
			 & ~ISA_EXEC_PROPERTY_Even_Unit);

  // Free 'from' slot
  TI_BUNDLE_slot_filled(bundle, from) =  FALSE;
  TI_BUNDLE_stop_bit(bundle, from) = FALSE; 

  // Recompute slot mask after free
  ISA_EXEC_MASK slot_mask = TI_BUNDLE_slot_mask(bundle);
  slot_mask = TI_BUNDLE_Set_Slot_Mask_Property(slot_mask, from, 0);
  Set_TI_BUNDLE_slot_mask (bundle, slot_mask);

  TI_BUNDLE_exec_property(bundle, from) = 0;

  // update new property with that of slot with which we swap
  *to_prop = new_prop
    | (*to_prop
       & (ISA_EXEC_PROPERTY_Odd_Unit | ISA_EXEC_PROPERTY_Even_Unit));
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
	if (!ISA_LC_Value_In_Class(val, LC_s9)) {
	  extra_slot_reqd = TRUE;
	}
      }
      else if (TN_is_symbol(opnd)) {
	ST *st, *base_st;
	INT64 base_ofst;

	st = TN_var(opnd);
	Base_Symbol_And_Offset (st, &base_st, &base_ofst);
	// SP/FP relative may actually fit into 9-bits
	if (base_st == SP_Sym || base_st == FP_Sym) {
	  val = CGTARG_TN_Value (opnd, base_ofst);
	  if (!ISA_LC_Value_In_Class(val, LC_s9)) {
	    extra_slot_reqd = TRUE;
	  }
	}
	else if (ST_gprel(base_st) && !OP_xfer(op)) {
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
	    if (!ISA_LC_Value_In_Class(val, LC_s9)) {
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
			     const CG_GROUPING      *grouping,
#if 1 // [CL] keep track of alignment,
      // and allow non-consecutive slot allocation
			     BOOL                    check_addr,
			     INT32                   addr,
			     INT*                    reserved_slot
#endif
			     )
{
  // If slot already filled, return FALSE.
  if (TI_BUNDLE_slot_filled(bundle, slot)) return FALSE;

  INT  inst_words = ISA_PACK_Inst_Words(OP_code(op));

  // Need to check if extra slots are required and available within the
  // bundle (eg. anything that uses imml,immr)
  //
  // TODO: add to ISA_PROPERTYs ??
  //
  BOOL extra_slot_reqd;

  TOP etop;

  extra_slot_reqd = (OP_inst_words(op) == 2);
  FmtAssert(extra_slot_reqd || !CGTARG_need_extended_Opcode(op, &etop),
	    ("CGTARG_Bundle_Slot_Available: opcode extension invalid: BB:%d TOP: %s", BB_id(OP_bb(op)), TOP_Name(OP_code(op))));

#if 1 // [CL] First check alignment constraints
  ISA_EXEC_UNIT_PROPERTY odd_even_prop = 0;
  INT slot_to_swap = 0;
  BOOL found_swap=FALSE;
  // Relax Odd/Even constraints if an extra slot is required:
  // is this case, the 'double' insn will always fit in
  // two adjacent slots
  // No need to change to extended opcode yet, as the normal
  // one has suitable properties for alignment constraints
  if (!extra_slot_reqd) {
    if (EXEC_PROPERTY_is_Odd_Unit(OP_code(op))) {
      odd_even_prop = ISA_EXEC_PROPERTY_Odd_Unit;
      if (check_addr  && (((addr+slot) & 1) == 0)) {
	// Trying to insert at an even word address.  If slot is > 0,
	// (ie if slot==2), we can insert here if there is a previous
	// slot where word address is odd, and that slot is not
	// already used by and insn constrained to be at an odd
	// address, and that slot is not used by an insn that uses 2
	// slots (swap would not be possible)
	for (int myslot=0; myslot<slot; myslot++) {
	  // If myslot is already required, skip it
	  if (Is_This_Slot_Required(bundle, myslot)) {
	    continue;
	  }
	  // If myslot is candidate, and not occupied by another 'Odd'
	  // or 'twin' op, asm will be able to swap
	  if ( (((addr+myslot) & 1) == 1)
	       && !(TI_BUNDLE_exec_property(bundle, myslot)
		    & ISA_EXEC_PROPERTY_Odd_Unit)
	       && !(Twin_Slot(bundle, myslot))
	       ) {
	    found_swap = TRUE;
	    slot_to_swap = myslot;
	    break;
	  }
	}
	if (!found_swap)
	  return FALSE;
      }
    } else if (EXEC_PROPERTY_is_Even_Unit(OP_code(op))) {
      odd_even_prop = ISA_EXEC_PROPERTY_Even_Unit;
      if (check_addr && (((addr+slot) & 1) == 1)) {
	// Same as above, with even
	for (int myslot=0; myslot<slot; myslot++) {
	  // If myslot is already required, skip it
	  if (Is_This_Slot_Required(bundle, myslot)) {
	    continue;
	  }
	  if ( (((addr+myslot) & 1) == 0)
	       && !(TI_BUNDLE_exec_property(bundle, myslot)
		    & ISA_EXEC_PROPERTY_Even_Unit)
	       && !(Twin_Slot(bundle, myslot))
	       ) {
	    found_swap = TRUE;
	    slot_to_swap = myslot;
	    break;
	  }
	}
	if (!found_swap)
	  return FALSE;
      }
    }
  }
#endif

#if 0 // [CL] Removed that stuff and changed machine
      // description so that control instruction also
      // required an ODD resource: impossible combinations
      // are excluded by resource exhaustion.

  // [CL] Handle the instructions that /require/ a particular slot,
  // ie branches
  if (EXEC_PROPERTY_is_ReqS0_Unit(OP_code(op))) {
    odd_even_prop = ISA_EXEC_PROPERTY_ReqS0_Unit;
    if (slot != 0) {
      // Trying to insert at a slot > 0
      // If a previous slot is used by an insn with
      // alignment constraints, or if a previous slot
      // is used by a 'twin' insn, give up for now
      for (int myslot=0; myslot<slot; myslot++) {
	if ( (TI_BUNDLE_exec_property(bundle, myslot)
		    & ISA_EXEC_PROPERTY_Even_Unit)
	     ||
	     (TI_BUNDLE_exec_property(bundle, myslot)
		    & ISA_EXEC_PROPERTY_Odd_Unit)
	     )
	  {
	    /* Consider impossible to fit */
	    return FALSE;
	  }
      }
    }
  }
#endif

  if (EXEC_PROPERTY_is_S0_Unit(OP_code(op)) &&
      EXEC_PROPERTY_is_S1_Unit(OP_code(op)) &&
      EXEC_PROPERTY_is_S2_Unit(OP_code(op)) &&
      EXEC_PROPERTY_is_S3_Unit(OP_code(op))) {

    //fprintf(TFile,"  S0,S1,S2,S3\n");

    if (slot == 0)
      *prop = ISA_EXEC_PROPERTY_S0_Unit;
    if (slot == 1)
      *prop = ISA_EXEC_PROPERTY_S1_Unit;
    if (slot == 2)
      *prop = ISA_EXEC_PROPERTY_S2_Unit;
    if (slot == 3)
      *prop = ISA_EXEC_PROPERTY_S3_Unit;
  }
  else if (EXEC_PROPERTY_is_S0_Unit(OP_code(op)) &&
	   EXEC_PROPERTY_is_S2_Unit(OP_code(op))) {

    //fprintf(TFile,"  S0,S2\n");

    if (slot == 0)
      *prop = ISA_EXEC_PROPERTY_S0_Unit;
    else if (slot == 2)
      *prop = ISA_EXEC_PROPERTY_S2_Unit;
    else
      return FALSE;
  }
  else if (EXEC_PROPERTY_is_S0_Unit(OP_code(op))) {

    //fprintf(TFile,"  S0\n");

    // for example branches, etc.
    if (slot == 0)
      *prop = ISA_EXEC_PROPERTY_S0_Unit;
    else
      return FALSE;
  }
  else if (EXEC_PROPERTY_is_S1_Unit(OP_code(op)) &&
	   EXEC_PROPERTY_is_S3_Unit(OP_code(op))) {

    //fprintf(TFile,"  S1,S3\n");

    if (slot == 1)
      *prop = ISA_EXEC_PROPERTY_S1_Unit;
    else if (slot == 3)
      *prop = ISA_EXEC_PROPERTY_S3_Unit;
    else
      return FALSE;
  }
  else if (EXEC_PROPERTY_is_EXT0_Unit(OP_code(op)) &&
	   EXEC_PROPERTY_is_EXT1_Unit(OP_code(op)) &&
	   EXEC_PROPERTY_is_EXT2_Unit(OP_code(op))) {
    if (slot == 0)
      *prop = ISA_EXEC_PROPERTY_EXT0_Unit;
    else if (slot == 1)
      *prop = ISA_EXEC_PROPERTY_EXT1_Unit;
    else if (slot == 2)
      *prop = ISA_EXEC_PROPERTY_EXT2_Unit;
    else
      return FALSE;
  }
  else {
    FmtAssert(FALSE, 
      ("CGTARG_Bundle_Slot_Available: unknown OP_code property 0x%x",
             ISA_EXEC_Unit_Prop(OP_code(op))));
  }

#if 1 // [CL]
  // Remember alignment constraints.
  // Might be overwritten later if we have a insn with
  // immediate extension
  *prop |= odd_even_prop;
#endif

  //fprintf(TFile," exec property 0x%x\n",*prop);

  // If there is a need to delay the scheduling of <op>...
  // It is done after determination of prop because apparently
  // prop is needed to be returned ?
  if (Delay_Scheduling_OP(op, slot, bundle)) return FALSE;

#if 1 // [CL]
  if (found_swap) {
    Move_Slot(bundle, slot_to_swap, slot, prop);
    slot = slot_to_swap;
  }
#endif

  //
  // All bundles have a stop at the end, reserve it (the routine
  // TI_BUNDLE_Slot_Available() needs it to match the stop bits in 
  // the template).
  //
  TI_BUNDLE_Reserve_Stop_Bit(bundle, 3);

  BOOL slot_avail = TRUE;
  if (extra_slot_reqd) {
    slot_avail = FALSE;
    // check whether slot and slot+1 are available for an extended OP
    if (slot == 0 &&
	TI_BUNDLE_Slot_Available(bundle, 
				 ISA_EXEC_PROPERTY_EXT0_Unit, 
				 slot) &&
	TI_BUNDLE_Slot_Available(bundle, 
				 ISA_EXEC_PROPERTY_EXT0_Unit, 
				 slot+1)) {
      slot_avail = TRUE;
    }
    else if (slot == 1 &&
	     TI_BUNDLE_Slot_Available(bundle, 
				      ISA_EXEC_PROPERTY_EXT1_Unit, 
				      slot) &&
	     TI_BUNDLE_Slot_Available(bundle, 
				      ISA_EXEC_PROPERTY_EXT1_Unit, 
				      slot+1)) {
      slot_avail = TRUE;
    }
    else if (slot == 2 &&
	     TI_BUNDLE_Slot_Available(bundle, 
				      ISA_EXEC_PROPERTY_EXT2_Unit, 
				      slot) &&
	     TI_BUNDLE_Slot_Available(bundle, 
				      ISA_EXEC_PROPERTY_EXT2_Unit, 
				      slot+1)) {
      slot_avail = TRUE;
    }
  }
  else {
    slot_avail = TI_BUNDLE_Slot_Available (bundle, *prop, slot);
  }

  //fprintf(TFile,"extra_slot_reqd = %s\n", extra_slot_reqd ? "true" : "false");
  //fprintf(TFile, "  extra slot avail = %s\n", (extra_slot_avail) ? "true" : "false");
  //fprintf(TFile,"slot_avail = %s\n", slot_avail ? "true" : "false");

  TI_BUNDLE_Unreserve_Stop_Bit(bundle, 3);

#if 1 // [CL]
  if (check_addr && slot_avail) {
    *reserved_slot = slot;
  }
#endif

  return slot_avail;
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

#if 1 // [CL]
/* ====================================================================
 *   CGTARG_Finish_Bundle: fill empty slots with nops
 * ====================================================================
 */
void CGTARG_Finish_Bundle(OP                          *op, 
			  TI_BUNDLE                   *bundle)
{
  INT template_bit = TI_BUNDLE_Return_Template(bundle);
  FmtAssert (template_bit != -1, ("Illegal template encoding"));
  INT i;
  BOOL found_filled = FALSE;
  //
  // fill with nops
  // Rely on assembler reordering: we can insert all necessary nops
  // before op: no problem if op order does not match allocated
  // slots
  // Do a reserve parsing because no stop bit is marked
  for (i = TI_BUNDLE_slot_count(bundle)-1; i >= 0 ; i--) {
    if (!found_filled && TI_BUNDLE_slot_filled(bundle, i)) {
      found_filled = TRUE;
      continue;
    }
    if (found_filled && !TI_BUNDLE_slot_filled(bundle, i)) {
      OP *noop = Mk_OP(CGTARG_Noop_Top(ISA_EXEC_Slot_Prop(template_bit, i)));

      BB_Insert_Op_Before(OP_bb(op), op, noop);
      OP_scycle(noop) = -1;

      Set_OP_bundled (noop);
      TI_BUNDLE_Reserve_Slot (bundle, i, ISA_EXEC_Slot_Prop(template_bit, i));
    }
  }
}
#endif

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

/* ====================================================================
 *   CGTARG_Initialize
 * ====================================================================
 */
void 
CGTARG_Initialize ()
{
  INT32 i;

  /* TODO: tabulate in the arch data base */

  /* Init all table entries to TOP_UNDEFINED.
   */
  for(i = 0; i <= TOP_count; ++i) {
    CGTARG_Invert_Table[i]        = TOP_UNDEFINED;
  }

  /* Init table for CGTARG_Invert:
   */
  CGTARG_Invert_Table[TOP_br]          = TOP_brf;
  CGTARG_Invert_Table[TOP_brf]         = TOP_br;
  CGTARG_Invert_Table[TOP_slct_r]      = TOP_slctf_r;
  CGTARG_Invert_Table[TOP_slct_i]      = TOP_slctf_i;
  CGTARG_Invert_Table[TOP_slct_ii]     = TOP_slctf_ii;
  CGTARG_Invert_Table[TOP_slctf_r]     = TOP_slct_r;
  CGTARG_Invert_Table[TOP_slctf_i]     = TOP_slct_i;
  CGTARG_Invert_Table[TOP_slctf_ii]    = TOP_slct_ii;
  CGTARG_Invert_Table[TOP_cmpeq_r_b]   = TOP_cmpne_r_b;
  CGTARG_Invert_Table[TOP_cmpeq_i_b]   = TOP_cmpne_i_b;
  CGTARG_Invert_Table[TOP_cmpeq_ii_b]  = TOP_cmpne_ii_b;
  CGTARG_Invert_Table[TOP_cmpge_r_b]   = TOP_cmplt_r_b;
  CGTARG_Invert_Table[TOP_cmpge_i_b]   = TOP_cmplt_i_b;
  CGTARG_Invert_Table[TOP_cmpge_ii_b]  = TOP_cmplt_ii_b;
  CGTARG_Invert_Table[TOP_cmpgeu_r_b]  = TOP_cmpltu_r_b;
  CGTARG_Invert_Table[TOP_cmpgeu_i_b]  = TOP_cmpltu_i_b;
  CGTARG_Invert_Table[TOP_cmpgeu_ii_b] = TOP_cmpltu_ii_b;
  CGTARG_Invert_Table[TOP_cmpgt_r_b]   = TOP_cmple_r_b;
  CGTARG_Invert_Table[TOP_cmpgt_i_b]   = TOP_cmple_i_b;
  CGTARG_Invert_Table[TOP_cmpgt_ii_b]  = TOP_cmple_ii_b;
  CGTARG_Invert_Table[TOP_cmpgtu_r_b]  = TOP_cmpleu_r_b;
  CGTARG_Invert_Table[TOP_cmpgtu_i_b]  = TOP_cmpleu_i_b;
  CGTARG_Invert_Table[TOP_cmpgtu_ii_b] = TOP_cmpleu_ii_b;
  CGTARG_Invert_Table[TOP_cmple_r_b]   = TOP_cmpgt_r_b;
  CGTARG_Invert_Table[TOP_cmple_i_b]   = TOP_cmpgt_i_b;
  CGTARG_Invert_Table[TOP_cmple_ii_b]  = TOP_cmpgt_ii_b;
  CGTARG_Invert_Table[TOP_cmpleu_r_b]  = TOP_cmpgtu_r_b;
  CGTARG_Invert_Table[TOP_cmpleu_i_b]  = TOP_cmpgtu_i_b;
  CGTARG_Invert_Table[TOP_cmpleu_ii_b] = TOP_cmpgtu_ii_b;
  CGTARG_Invert_Table[TOP_cmplt_r_b]   = TOP_cmpge_r_b;
  CGTARG_Invert_Table[TOP_cmplt_i_b]   = TOP_cmpge_i_b;
  CGTARG_Invert_Table[TOP_cmplt_ii_b]  = TOP_cmpge_ii_b;
  CGTARG_Invert_Table[TOP_cmpltu_r_b]  = TOP_cmpgeu_r_b;
  CGTARG_Invert_Table[TOP_cmpltu_i_b]  = TOP_cmpgeu_i_b;
  CGTARG_Invert_Table[TOP_cmpltu_ii_b] = TOP_cmpgeu_ii_b;
  CGTARG_Invert_Table[TOP_cmpne_r_b]   = TOP_cmpeq_r_b;
  CGTARG_Invert_Table[TOP_cmpne_i_b]   = TOP_cmpeq_i_b;
  CGTARG_Invert_Table[TOP_cmpne_ii_b]  = TOP_cmpeq_ii_b;
  CGTARG_Invert_Table[TOP_cmpeq_r_r]   = TOP_cmpne_r_r;
  CGTARG_Invert_Table[TOP_cmpeq_i_r]   = TOP_cmpne_i_r;
  CGTARG_Invert_Table[TOP_cmpeq_ii_r]  = TOP_cmpne_ii_r;
  CGTARG_Invert_Table[TOP_cmpge_r_r]   = TOP_cmplt_r_r;
  CGTARG_Invert_Table[TOP_cmpge_i_r]   = TOP_cmplt_i_r;
  CGTARG_Invert_Table[TOP_cmpge_ii_r]  = TOP_cmplt_ii_r;
  CGTARG_Invert_Table[TOP_cmpgeu_r_r]  = TOP_cmpltu_r_r;
  CGTARG_Invert_Table[TOP_cmpgeu_i_r]  = TOP_cmpltu_i_r;
  CGTARG_Invert_Table[TOP_cmpgeu_ii_r] = TOP_cmpltu_ii_r;
  CGTARG_Invert_Table[TOP_cmpgt_r_r]   = TOP_cmple_r_r;
  CGTARG_Invert_Table[TOP_cmpgt_i_r]   = TOP_cmple_i_r;
  CGTARG_Invert_Table[TOP_cmpgt_ii_r]  = TOP_cmple_ii_r;
  CGTARG_Invert_Table[TOP_cmpgtu_r_r]  = TOP_cmpleu_r_r;
  CGTARG_Invert_Table[TOP_cmpgtu_i_r]  = TOP_cmpleu_i_r;
  CGTARG_Invert_Table[TOP_cmpgtu_ii_r] = TOP_cmpleu_ii_r;
  CGTARG_Invert_Table[TOP_cmple_r_r]   = TOP_cmpgt_r_r;
  CGTARG_Invert_Table[TOP_cmple_i_r]   = TOP_cmpgt_i_r;
  CGTARG_Invert_Table[TOP_cmple_ii_r]  = TOP_cmpgt_ii_r;
  CGTARG_Invert_Table[TOP_cmpleu_r_r]  = TOP_cmpgtu_r_r;
  CGTARG_Invert_Table[TOP_cmpleu_i_r]  = TOP_cmpgtu_i_r;
  CGTARG_Invert_Table[TOP_cmpleu_ii_r] = TOP_cmpgtu_ii_r;
  CGTARG_Invert_Table[TOP_cmplt_r_r]   = TOP_cmpge_r_r;
  CGTARG_Invert_Table[TOP_cmplt_i_r]   = TOP_cmpge_i_r;
  CGTARG_Invert_Table[TOP_cmplt_ii_r]  = TOP_cmpge_ii_r;
  CGTARG_Invert_Table[TOP_cmpltu_r_r]  = TOP_cmpgeu_r_r;
  CGTARG_Invert_Table[TOP_cmpltu_i_r]  = TOP_cmpgeu_i_r;
  CGTARG_Invert_Table[TOP_cmpltu_ii_r] = TOP_cmpgeu_ii_r;
  CGTARG_Invert_Table[TOP_cmpne_r_r]   = TOP_cmpeq_r_r;
  CGTARG_Invert_Table[TOP_cmpne_i_r]   = TOP_cmpeq_i_r;
  CGTARG_Invert_Table[TOP_cmpne_ii_r]  = TOP_cmpeq_ii_r;
  CGTARG_Invert_Table[TOP_andl_r_r]  = TOP_nandl_r_r;
  CGTARG_Invert_Table[TOP_andl_i_r]  = TOP_nandl_i_r;
  CGTARG_Invert_Table[TOP_andl_ii_r]  = TOP_nandl_ii_r;
  CGTARG_Invert_Table[TOP_nandl_r_r]  = TOP_andl_r_r;
  CGTARG_Invert_Table[TOP_nandl_i_r]  = TOP_andl_i_r;
  CGTARG_Invert_Table[TOP_nandl_ii_r]  = TOP_andl_ii_r;
  CGTARG_Invert_Table[TOP_orl_r_r]  = TOP_norl_r_r;
  CGTARG_Invert_Table[TOP_orl_i_r]  = TOP_norl_i_r;
  CGTARG_Invert_Table[TOP_orl_ii_r]  = TOP_norl_ii_r;
  CGTARG_Invert_Table[TOP_norl_r_r]  = TOP_orl_r_r;
  CGTARG_Invert_Table[TOP_norl_i_r]  = TOP_orl_i_r;
  CGTARG_Invert_Table[TOP_norl_ii_r]  = TOP_orl_ii_r;
  CGTARG_Invert_Table[TOP_andl_r_b]  = TOP_nandl_r_b;
  CGTARG_Invert_Table[TOP_andl_i_b]  = TOP_nandl_i_b;
  CGTARG_Invert_Table[TOP_andl_ii_b]  = TOP_nandl_ii_b;
  CGTARG_Invert_Table[TOP_nandl_r_b]  = TOP_andl_r_b;
  CGTARG_Invert_Table[TOP_nandl_i_b]  = TOP_andl_i_b;
  CGTARG_Invert_Table[TOP_nandl_ii_b]  = TOP_andl_ii_b;
  CGTARG_Invert_Table[TOP_orl_r_b]  = TOP_norl_r_b;
  CGTARG_Invert_Table[TOP_orl_i_b]  = TOP_norl_i_b;
  CGTARG_Invert_Table[TOP_orl_ii_b]  = TOP_norl_ii_b;
  CGTARG_Invert_Table[TOP_norl_r_b]  = TOP_orl_r_b;
  CGTARG_Invert_Table[TOP_norl_i_b]  = TOP_orl_i_b;
  CGTARG_Invert_Table[TOP_norl_ii_b]  = TOP_orl_ii_b;


  return;
}
