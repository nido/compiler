#include "defs.h"
#include "config.h"
#include "erglob.h"
#include "ercg.h"
#include "glob.h"
#include "tracing.h"
#include "util.h"

#include "symtab.h"
#include "opcode.h"
#include "intrn_info.h"
#include "wutil.h"      /* for WN_intrinsic_return_ty */
#include "const.h"      /* needed to manipulate target/host consts */
#include "cgir.h"

#include "topcode.h"
#include "targ_isa_lits.h"
#include "targ_isa_properties.h"

/* ======================================================================
 *   Get_Intrinsic_Size_Mtype
 * ======================================================================
 */
static TYPE_ID
Get_Intrinsic_Size_Mtype (
  INTRINSIC id
)
{
  FmtAssert(FALSE,("Not Implemented"));

  switch (id) {
  default:
	#pragma mips_frequency_hint NEVER
  	FmtAssert(FALSE, ("Unexpected intrinsic %d", id));
	/*NOTREACHED*/
  }
}

/* ======================================================================
 *   Intrinsic_Returns_New_Value
 * ======================================================================
 */
static BOOL
Intrinsic_Returns_New_Value (
  INTRINSIC id
)
{
  FmtAssert(FALSE,("Not Implemented"));

  switch (id) {
  default:
	return FALSE;
  }
}

/* ====================================================================
 *   Exp_Intrinsic_Op
 * ====================================================================
 */
void
Exp_Intrinsic_Op (
  INTRINSIC id, 
  INT num_results,
  INT num_opnds,
  TN *result[], 
  TN *opnd[],
  OPS *ops
)
{
  switch (id) {

    case INTRN_MPSSE:
      Build_OP (TOP_GP32_MPSSLL_GT_DR_DR_DR, result[0], True_TN, 
                                                   opnd[0], opnd[1], ops);
      break;

    case INTRN_MASSE:
      Build_OP (TOP_GP32_MASSLL_GT_DR_DR_DR_DR, result[0], True_TN, 
                                            opnd[0], opnd[1], opnd[2], ops);
      break;

    // Following only appear in Lai_Code:
    // TODO: make this a default ?
    case INTRN_DIVW:
    case INTRN_DIVUW:
      {
	INT i;
	TN *args[10];

	// Some intrinsics map to the ISA opcodes. If we're generating the
	// Lai_Code some intrinsics map to function calls. 
	// Exp_Intrinsic_Op () returns TOP_intrncall for such intrinsics.
	// The first operand TN of a TOP_intrncall is a symbolic TN
	// indicating the function name for a function to be called.
	// We need to pass a ST representing this function.
	// We do not have the ST for it because it's in the library
	// So create a dummy func ST for this function.
	TY_IDX  ty = Make_Function_Type(WN_intrinsic_return_ty(id));
	ST     *st = Gen_Intrinsic_Function(ty, INTRN_c_name(id));

	// operand 0 is the predicate
	args[0] = True_TN;

	// operand 1 is the intrinsic name
	args[1] = Gen_Symbol_TN (st, 0, TN_RELOC_NONE);

	// operands 2 .. num_opnds+2 are operands
	for (i = 0; i < num_opnds; i++) {
	  args[i+2] = opnd[i];
	}

	// create intrcall op
	OP* intrncall_op = Mk_VarOP(TOP_intrncall, num_results, num_opnds+2, result, args);
	OPS_Append_Op(ops, intrncall_op);
      }
      break;

    default:
      FmtAssert (FALSE, ("Exp_Intrinsic_Op: unknown intrinsic op"));
  }

  return;
}

/* ======================================================================
 *   Exp_Intrinsic_Call
 *
 *   initial expansion of intrinsic call (may not be complete lowering).
 *   return result TN (if set).
 *   If the intrinsic requires a label and loop (2 bb's)
 *   then ops is for first bb and ops2 is for bb after the label.
 *   Otherwise only ops is filled in.
 * ======================================================================
 */
TN *
Exp_Intrinsic_Call (
  INTRINSIC id, 
  TN *op0, 
  TN *op1, 
  TN *op2, 
  OPS *ops, 
  LABEL_IDX *label, 
  OPS *loop_ops
)
{
  FmtAssert(FALSE,("Not Implemented"));

  return NULL;
}
