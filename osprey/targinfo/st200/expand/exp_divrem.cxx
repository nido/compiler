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


/* EXP routines for expanding divide and rem */

#include <signal.h>
#include "defs.h"
#include "errors.h"
#include "erglob.h"
#include "ercg.h"
#include "tracing.h"
#include "config.h"
#include "config_TARG.h"
#include "config_debug.h"
#include "mtypes.h"
#include "topcode.h"
#include "tn.h"
#include "op.h"
#include "targ_isa_lits.h"
#include "targ_trap.h"
#include "cg_flags.h"
#include "cgexp.h"


#define RESET_COND_DEF_LAST(ops) Set_OP_cond_def_kind(OPS_last(ops),OP_ALWAYS_UNC_DEF)

/* Define the exponent parameters for the various float types.
 */
enum { E32min = -126,   E32max = 127,   E32bias = 127   }; // single
enum { E64min = -1022,  E64max = 1023,  E64bias = 1023  }; // double
enum { E80min = -16382, E80max = 16383, E80bias = 16383 }; // long double
enum { E82min = -65534, E82max = 65535, E82bias = 65535 }; // register-format

/* ====================================================================
 *   Exp_Float_Divide
 * ====================================================================
 */
void
Expand_Float_Divide(
  TN *result, 
  TN *src1, 
  TN *src2, 
  TYPE_ID mtype, 
  OPS *ops
)
{
  FmtAssert(FALSE, ("Not implemented"));
}

/* ====================================================================
 *   Exp_Float_Recip
 * ====================================================================
 */ 
void
Expand_Float_Recip (
  TN *result, 
  TN *src, 
  TYPE_ID mtype, 
  OPS *ops
)
{
  FmtAssert(FALSE,("Not Implemented"));

  return;
}

/*****************************************************************************
 *
 * Integer division internal support routines
 *
 *****************************************************************************/

/* Return values for Check_Divide:
 */
typedef enum {
  DIVCHK_RUNTIME,	// unknown except at run-time
  DIVCHK_BYZERO,	// unconditional div-by-zero
  DIVCHK_OVERFLOW	// unconditional overflow
} DIVCHK_STATUS;

static TOP
Pick_Compare_For_Mtype (
  TYPE_ID mtype,
  BOOL imm
)
{
  switch (mtype) {
    case MTYPE_I4:
    case MTYPE_U4:
      return (imm) ? TOP_cmpeq_r_r_r : TOP_cmpeq_i_r_r;
      break;
    default:
      FmtAssert(FALSE,("Pick_Compare_For_Mtype: unsupported mtype"));
  }
}

/* ====================================================================
 *   Check_Divide
 * 
 *   Check a divide for undefined operations. The operands are examined
 *   to determine if it is known at compile-time that a fault will
 *   occur or if it cannot be known until run-time. The return value
 *   indicates the situation. In addition, if divide checking is
 *   enabled, code is generated to cause an exception.
 * ====================================================================
 */ 
static DIVCHK_STATUS
Check_Divide (
  TN *numerator, 
  TN *divisor, 
  TYPE_ID mtype, 
  OPS *ops
)
{
  // TODO: don't want to generate checks while using simulator, so reset
  // div-by-zero checking which is on by default.
  BOOL div_zero_check = FALSE; // DEBUG_Div_Zero_Check

  INT64	divisor_val;
  BOOL const_divisor = TN_Value_At_Op (divisor, NULL, &divisor_val);

  /* Check for divide-by-zero.
   */
  if (const_divisor) {
    if (divisor_val == 0) {
      if (div_zero_check) {
	TOP top = TOP_UNDEFINED;
	if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_syscall_i)) {
	  top = TOP_syscall_i;
	} else if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_st240_syscall_i)) {
	  top = TOP_st240_syscall_i;
	}
	Build_OP (top, Gen_Literal_TN(FPE_INTDIV_trap, 1), ops);
      }
      return DIVCHK_BYZERO;
    }
  } 
  else if (div_zero_check) {
    // Generate a compare and a trap:
    TN *p1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch);
    TOP opc = Pick_Compare_For_Mtype (mtype, FALSE);
#if 0
    Build_OP (opc, p1, True_TN, divisor, 
                     Gen_Literal_TN (0, MTYPE_byte_size(mtype)), ops);
    Build_OP (TOP_GP32_TRAP_GT_U4, p1, 
                             Gen_Literal_TN(FPE_INTDIV_trap, 1), ops);
#else
    FmtAssert(FALSE,("Check_Divide: no can do now"));
#endif
  }

  /* Check for overflow.
   */
  if (MTYPE_is_signed(mtype)) {
    INT64 numer_val;
    BOOL const_numer = TN_Value_At_Op (numerator, NULL, &numer_val);
    const INT64 minint_val = MTYPE_size_min (mtype);
    const INT min_tn_size = MTYPE_byte_size (mtype);
    if (const_divisor && const_numer) {
      if (numer_val == minint_val && divisor_val == -1) {
	if (DEBUG_Div_Oflow_Check) {
	  TOP top = TOP_UNDEFINED;
	  if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_syscall_i)) {
	    top = TOP_syscall_i;
	  } else if (ISA_SUBSET_LIST_Member(ISA_SUBSET_List, TOP_st240_syscall_i)) {
	    top = TOP_st240_syscall_i;
	  }
	  Build_OP (top, Gen_Literal_TN(FPE_INTOVF_trap, 1), ops);
	}
	return DIVCHK_OVERFLOW;
      }
    } 
    else if (DEBUG_Div_Oflow_Check) {

      /* Generate code to test for most-negative-integer divided by -1
       */
      TOP opc;
      TN *p1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch);
#if 0
      if (const_numer) {
	opc = Pick_Compare_For_Mtype (mtype, FALSE);
	Build_OP (opc, p1, True_TN, Gen_Literal_TN(-1, 4), divisor, ops);
      } 
      else if (const_divisor) {
	TN *minint = Build_TN_Of_Mtype (mtype);
	Expand_Immediate (minint, Gen_Literal_TN(minint_val, min_tn_size), 
			  TRUE, ops);
	opc = Pick_Compare_For_Mtype (mtype, TRUE);
	Build_OP (opc, p1, True_TN, numerator, minint, ops);
      } else {
	FmtAssert(FALSE,("Not Implemented"));
	//	opc = is_double ? TOP_noop : TOP_noop;
	Build_OP (opc, p1, True_TN, Gen_Literal_TN(-1, 4), divisor, ops);

	TN *minint = Build_TN_Of_Mtype (mtype);
	Expand_Immediate (minint, Gen_Literal_TN(minint_val, min_tn_size), 
			  TRUE, ops);
	//	opc = is_double ? TOP_noop : TOP_noop;
	Build_OP (opc, p1, True_TN, numerator, minint, ops);
      }
      Build_OP (TOP_GP32_TRAP_GT_U4, p1, 
                               Gen_Literal_TN(FPE_INTOVF_trap, 1), ops);
#else
      FmtAssert(FALSE,("Check_Divide: no can do I said"));
#endif
    }
  }

  return DIVCHK_RUNTIME;
}

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

/* ====================================================================
 * Expand the sequence for division and/or remainder by a variable.
 * ====================================================================
 */
static void
Expand_NonConst_DivRem (TN *quot, TN *rem, TN *x, TN *y, TYPE_ID mtype, OPS *ops)
{
    BOOL is_signed = MTYPE_is_signed(mtype);
    if (quot) {
      TOP divop = is_signed ? TOP_div_r_r_r : TOP_divu_r_r_r ;
      Build_OP(divop, quot, x, y, ops) ;
    } else if (rem) {
      TOP remop = is_signed ? TOP_rem_r_r_r : TOP_remu_r_r_r ;
      Build_OP(remop, rem, x, y, ops) ;
    } else {
	FmtAssert(FALSE, ("Expand_NonConst_DivRem unexpected expansion"));
    }
}

/* ====================================================================
 *   Expand_Power_Of_2_Divide
 * ====================================================================
 */
static void
Expand_Power_Of_2_Divide (TN *result, TN *numer, INT64 dvsr, 
			  TYPE_ID mtype, OPS *ops)
{
  FmtAssert(mtype == MTYPE_I4 || mtype == MTYPE_U4,
	    ("Expand_Power_Of_2_Divide: mtype %s not handled",
	      MTYPE_name(mtype)));

  INT pow2 = Get_Power_Of_2 (dvsr, mtype);

  if (MTYPE_is_unsigned(mtype)) {
    Build_OP (TOP_shru_i_r_r, result,
	      numer, Gen_Literal_TN (pow2, MTYPE_byte_size(mtype)), ops);
  } else {
    // signed is more complicated
    TN *t0 = Build_TN_Of_Mtype(mtype);
    TN *t1 = Build_TN_Of_Mtype(mtype);
    TN *t2 = dvsr < 0 ? Build_TN_Of_Mtype(mtype) : result;
    INT64 absdvsr = dvsr < 0 ? -dvsr : dvsr;

    if (absdvsr == 2) {
      /* Optimize for abs(divisor) == 2:
       *      extr.u tmp0=numer,signbit,1  -- dvsr-1 if numer negative else 0
       *      add tmp1=tmp0,numer
       *      shr result=tmp1,1
       * if (dvsr<0) sub result=0,result
       */
      // extract the sign bit
      Build_OP(TOP_shru_i_r_r,t0, numer, Gen_Literal_TN(31,4), ops);
      // add sign to numer
      Build_OP(TOP_add_r_r_r, t1, t0, numer, ops);
      // shr by 1
      Build_OP(TOP_shr_i_r_r, t2, t1, Gen_Literal_TN(1, 4), ops);
    } else {
      /* General case:
       *      cmp.lt p1,p2=numer,zero         -- numerator negative?
       *      add tmp1=abs(dvsr)-1,numer      -- speculatively add dvsr-1 to numer
       * (p1) shr result=tmp1,pow2(abs(dvsr))
       * (p2) shr result=numer,pow2(abs(dvsr))
       * if (dvsr<0) sub result=0,result
       */
#if 1 /* CGuillon: better sequence for ST200 */
      TN *p1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch);

      Build_OP(TOP_cmplt_r_r_b, p1, numer, Zero_TN, ops);
      Expand_Add (t0, Gen_Literal_TN(absdvsr-1,4), numer, mtype, ops);

      Build_OP(TOP_targ_slct_r_r_b_r, t1, p1, t0, numer, ops);
      Build_OP(TOP_shr_i_r_r, t2, t1, Gen_Literal_TN(pow2, 4), ops);
#else
      TN *p1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch);
      TN *t3 = Build_TN_Of_Mtype(mtype);

      Build_OP(TOP_cmplt_r_r_b, p1, numer, Zero_TN, ops);
      Expand_Add (t0, Gen_Literal_TN(absdvsr-1,4), numer, mtype, ops);
      
      Build_OP(TOP_shr_i_r_r, t1, t0, Gen_Literal_TN(pow2, 4), ops);
      Build_OP(TOP_shr_i_r_r, t3, numer, Gen_Literal_TN(pow2, 4), ops);
      Build_OP(TOP_targ_slct_r_r_b_r, t2, p1, t1, t3, ops);
#endif
    }
    if (dvsr < 0) {
      // must negate the result
	Build_OP (TOP_sub_r_r_r, result, Zero_TN, t2, ops);
    }
  }
}

/* ====================================================================
 *   Expand_Non_Power_Of_2_Signed_Divide
 *   Uses the method described in Hacker's Delight, 1st edition
 *   The magic method is described Figure-10.1 p. 174
 * ====================================================================
 */
struct ms {
  INT32 M; /* Magic variable */
  INT32 s;  /* Shift amount */
} ;

static ms
magics(INT32 d)
{
  INT32 p ;
  UINT32 ad, anc, delta, q1, r1, q2, r2, t;
  const UINT32 two31 = 0x80000000;
  struct ms mags ;
  FmtAssert( ((2 <= d ) && (d <= 0x7FFFFFFF)) || ((-0x80000000 <= d) && (d <= -2)),
	     ("Expand_Non_Power_Of_2_Signed_Divide::magic: value : %d not handled",d));

  ad = d < 0 ? -d : d ;
  t = two31 + ((UINT32)d >> 31);
  anc = t - 1 - t % ad;
  p = 31;
  q1 = two31 / anc;
  r1 = two31 - q1 * anc;
  q2 = two31 / ad;
  r2 = two31 - q2 * ad; 
  do {
    p = p + 1;
    q1 = 2 * q1;
    r1 = 2 * r1;
    if (r1 >= anc) {
      q1 = q1 + 1;
      r1 = r1 - anc;
    }
    q2 = 2 * q2;
    r2 = 2 * r2;
    if (r2 >= ad) {
      q2 = q2 + 1;
      r2 = r2 - ad;
    }
    delta = ad - r2;
  } while(q1 < delta || (q1 == delta && r1 == 0)) ;

  mags.M = q2 + 1;
  if (d < 0) mags.M = -mags.M;
  mags.s = p - 32;
  return mags ;
}

static BOOL
Expand_Non_Power_Of_2_Signed_Divide (TN *result, TN *numer_tn, INT64 dvsr, 
				     TYPE_ID mtype, OPS *ops)
{
  FmtAssert(mtype == MTYPE_I4,
	    ("Expand_Non_Power_Of_2_Signed_Divide: mtype %s not handled",
	     MTYPE_name(mtype)));

  /* Choose to handle code generation regardless of the sign of the
   divisor instead of normalizing it and then negating the result */
  struct ms mags = magics((INT32)dvsr);

  TN *M_tn = Build_TN_Of_Mtype (mtype);
  TN *q_tn = Build_TN_Of_Mtype (mtype);
  TN *r_tn = Build_TN_Of_Mtype (mtype);

  // Create the magic constant, then expand the instruction sequence
  Expand_Immediate (M_tn, Gen_Literal_TN (mags.M, 4), MTYPE_is_signed(mtype), ops);
  Expand_High_Multiply (q_tn, M_tn, numer_tn, mtype, ops);

  if (((INT32)dvsr>0) && (mags.M<0)) 
    Expand_Add(q_tn, q_tn, numer_tn, mtype, ops);
  else if (((INT32)dvsr<0) && (mags.M>0)) 
    Expand_Sub(q_tn, q_tn, numer_tn, mtype, ops);

  if (((INT32)dvsr>0))
    Expand_Shift(r_tn, numer_tn, Gen_Literal_TN(31, 4), mtype, shift_lright, ops);
  else
    Expand_Shift(r_tn, q_tn, Gen_Literal_TN(31, 4), mtype, shift_lright, ops);

  if (mags.s > 0)
    Expand_Shift(q_tn, q_tn, Gen_Literal_TN(mags.s, 4), mtype, shift_aright, ops);

  Expand_Add(result, q_tn, r_tn, mtype, ops);

  return TRUE;
}

/* ====================================================================
 *   Expand_Non_Power_Of_2_Unsigned_Divide
 *   Uses the method described in Hacker's Delight, 1st edition
 *   The magicu method is described Figure-10.2 p. 185
 * ====================================================================
 */
struct mu {
  UINT32 M; /* Magic variable */
  INT32 a;  /* Add indicator */
  INT32 s;  /* Shift amount */
} ;

static mu 
magicu(UINT32 d)
{
  INT32 p ;
  UINT32 nc, delta, q1, r1, q2, r2 ;
  struct mu magu ;
  FmtAssert( d>= 1,
	     ("Expand_Non_Power_Of_2_Unsigned_Divide::magicu: value : %d not handled",d));

  magu.a = 0;		// Clear 'add' indicator
  nc = -1 - (-d) % d;	// Unsigned arithmetic here
  p = 31;
  q1 = 0x80000000 / nc;
  r1 = 0x80000000 - q1 * nc;
  q2 = 0x7fffffff / d;
  r2 = 0x7fffffff - q2 * d;
  do {
    p = p + 1;
    if (r1 >= nc - r1) {
      q1 = 2 * q1 + 1;
      r1 = 2 * r1 - nc; 
    } else {
      q1 = 2 * q1;
      r1 = 2 * r1;
    }
    if (r2 + 1 >= d - r2) {
      if (q2 >= 0x7fffffff) 
	magu.a = 1;
      q2 = 2 * q2 + 1;
      r2 = 2 * r2 + 1 - d;
    } else {
      if (q2 >= 0x80000000)
	magu.a = 1;
      q2 = 2 * q2;
      r2 = 2*r2 + 1;
    }
    delta = d - 1 - r2;
  } while (p < 64 &&
	   (q1 < delta || (q1 == delta && r1 == 0)));

  magu.M = q2 + 1;
  magu.s = p - 32;
  return magu ;
}

static BOOL
Expand_Non_Power_Of_2_Unsigned_Divide (TN *result, TN *numer_tn, INT64 dvsr, 
				       TYPE_ID mtype, OPS *ops)
{
  FmtAssert(mtype == MTYPE_U4,
	    ("Expand_Non_Power_Of_2_Unsigned_Divide: mtype %s not handled",
	     MTYPE_name(mtype)));
  
  /* Code generation depends on the 'add' indicator */
  struct mu magu = magicu((UINT32)dvsr);

  TN *M_tn = Build_TN_Of_Mtype (mtype);
  TN *q_tn = Build_TN_Of_Mtype (mtype);

  // Create the magic constant, then expand the instruction sequence
  Expand_Immediate (M_tn, Gen_Literal_TN (magu.M, 4), MTYPE_is_signed(mtype), ops);
  Expand_High_Multiply (q_tn, M_tn, numer_tn, mtype, ops);

  if (magu.a) {
    TN *t_tn = Build_TN_Of_Mtype (mtype);
    Expand_Sub(t_tn, numer_tn, q_tn, mtype, ops);
    Expand_Shift(t_tn, t_tn, Gen_Literal_TN(1, 4), mtype, shift_lright, ops);
    Expand_Add(t_tn, q_tn, t_tn, mtype, ops);
    Expand_Shift(result, t_tn, Gen_Literal_TN(magu.s - 1, 4), mtype, shift_lright, ops);
  } else {
    Expand_Shift(result, q_tn, Gen_Literal_TN(magu.s, 4), mtype, shift_lright, ops);
  }

  return TRUE;
}

/* ====================================================================
 *   Expand_Divide_By_Constant
 * ====================================================================
 */
static BOOL
Expand_Divide_By_Constant (
  TN *result, 
  TN *src1,
  TN *src2,
  INT64 src2_val,
  TYPE_ID mtype, 
  OPS *ops
)
{
  BOOL is_signed = MTYPE_is_signed(mtype);

  FmtAssert(mtype == MTYPE_I4 || mtype == MTYPE_U4,
	    ("Expand_Divide_By_Constant: mtype %s not handled",
	     MTYPE_name(mtype)));

  /* Handle the trivial ones:
   */
  if (src2_val == 1) {
    Exp_COPY(result, src1, ops);
    return TRUE;
  } else if (is_signed && src2_val == -1) {
    Expand_Neg(result, src1, mtype, ops);
    return TRUE;
  }

  /* Look for simple shift optimizations:
   */
  if (Is_Power_Of_2 (src2_val, mtype)) {
    Expand_Power_Of_2_Divide(result, src1, src2_val, mtype, ops);
    return TRUE;
  }

  /* Conversions to multiplications may be disabled */
  if (!CGEXP_cvrt_int_div_to_mult) return FALSE;

  if (is_signed) {
    return Expand_Non_Power_Of_2_Signed_Divide(result, src1, src2_val, mtype, ops);
  } else {
    return Expand_Non_Power_Of_2_Unsigned_Divide(result, src1, src2_val, mtype, ops);
  }

  return FALSE;
}

/* ====================================================================
 *   Expand_Divide
 *
 *   Should be called only if Lai_Code = TRUE or 
 *   OPT_inline_divide = TRUE.
 * ====================================================================
 */
TN *
Expand_Divide (
  TN *result, 
  TN *src1, 
  TN *src2, 
  TYPE_ID mtype, 
  OPS *ops
)
{
  FmtAssert (MTYPE_is_integral(mtype) && MTYPE_byte_size(mtype) == 4,
	     ("Unexpected MTYPE: %s", MTYPE_name(mtype)));

  /* Check for undefined operations we can detect at compile-time
   * and when enabled, generate run-time checks.
   */
  switch (Check_Divide(src1, src2, mtype, ops)) {
  case DIVCHK_BYZERO:
  case DIVCHK_OVERFLOW:
//    Build_OP(TOP_ifixup, result, ops);
    OPS_Append_Op(ops, Mk_VarOP(TOP_ifixup,1,0,&result,NULL));
    return NULL;
  }

  /* Look for simple shift optimizations and multiply_hi optimizations:
   */
  INT64 src2_val, src1_val;
  BOOL const_src2 = TN_Value_At_Op (src2, NULL, &src2_val);
  BOOL const_src1 = TN_Value_At_Op (src1, NULL, &src1_val);

  

  if (const_src1) {
    if (!TN_is_constant(src1))
      src1 = Gen_Literal_TN(src1_val, MTYPE_byte_size(mtype));
    TN* tmp1;
    tmp1 = Expand_Immediate_Into_Register(mtype, src1, ops); 
    src1 = tmp1;
  }

  if (const_src2) {
    if (Expand_Divide_By_Constant (result, src1, src2, src2_val, mtype, ops)) {
      return NULL;
    }
    if (!TN_is_constant(src2))
      src2 = Gen_Literal_TN(src2_val, MTYPE_byte_size(mtype));
    TN* tmp2;
    tmp2 = Expand_Immediate_Into_Register(mtype, src2, ops); 
    src2 = tmp2;
  }

  Expand_NonConst_DivRem(result, NULL, src1, src2, mtype, ops);
  return NULL;
}

/* ====================================================================
 *   Expand_Power_Of_2_Rem
 *
 *   Expand the sequence for remainder of a power of two. It is the
 *   caller's job to verify that divisor is a non-zero power of two.
 *
 *   Expand rem(x, [+-]2^n) as follows:
 *
 *	Using the identities
 *		rem(x,y) =  rem( x,-y)
 *		rem(x,y) = -rem(-x, y)
 *
 *	unsigned
 *	f=	x & MASK(n)
 *
 *	signed
 *	f=	x & MASK(n)		x>=0
 *	f=	-(-x & MASK(n))		x<0
 * ====================================================================
 */
static void 
Expand_Power_Of_2_Rem (
  TN *result, 
  TN *src1, 
  INT64 src2_val, 
  TYPE_ID mtype, 
  OPS *ops
)
{
  Is_True(MTYPE_is_class_integer(mtype),("wrong mtype"));
  Is_True(MTYPE_byte_size(mtype) <= 4,("longlong ?"));

  INT n = Get_Power_Of_2(src2_val, mtype);
  INT64 nMask = (1LL << n) - 1;
  TN *con = Gen_Literal_TN(nMask, 4);

  if (MTYPE_signed(mtype)) {
    TN *tmp1, *tmp2, *tmp3, *tmp4;
    TN *p1;

    //
    // Test sign of src1
    //
    p1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_branch);
    Build_OP(TOP_cmplt_r_r_b, p1, src1, Zero_TN, ops);

    //
    // Get absolute value of src1
    //
    tmp1 = Build_TN_Of_Mtype(mtype);
    tmp2 = Build_TN_Of_Mtype(mtype);
    Build_OP(TOP_sub_r_r_r, tmp1, Zero_TN, src1, ops);
    Build_OP(TOP_targ_slct_r_r_b_r, tmp2, p1, tmp1, src1, ops);

    //
    // Perform the AND
    //
    tmp3 = Build_TN_Of_Mtype(mtype);
    Expand_Binary_And(tmp3, tmp2, con, mtype, ops);

    //
    // Negate the result if src1 was negative
    //
    tmp4 = Build_TN_Of_Mtype(mtype);
    Build_OP(TOP_sub_r_r_r, tmp4, Zero_TN, tmp3, ops);
    Build_OP(TOP_targ_slct_r_r_b_r, result, p1, tmp4, tmp3, ops);
  } else {
    Expand_Binary_And(result, src1, con, mtype, ops);
  }
}

/* ====================================================================
 *   Expand_Rem
 * ====================================================================
 */
void
Expand_Rem (
  TN *result, 
  TN *src1, 
  TN *src2, 
  TYPE_ID mtype, 
  OPS *ops
)
{
  FmtAssert (MTYPE_is_integral(mtype) && MTYPE_byte_size(mtype) == 4,
	     ("Unexpected MTYPE: %s", MTYPE_name(mtype)));

  //
  // Check for undefined operations we can detect at compile-time
  // and when enabled, generate run-time checks.
  //
  switch (Check_Divide(src1, src2, mtype, ops)) {
  case DIVCHK_BYZERO:
  case DIVCHK_OVERFLOW:
//    Build_OP(TOP_ifixup, result, ops);
    OPS_Append_Op(ops, Mk_VarOP(TOP_ifixup,1,0,&result,NULL));
   return;
  }

  //
  // Try to optimize when constant divisor.
  //
  INT64 src2_val, src1_val;
  BOOL const_src2 = TN_Value_At_Op (src2, NULL, &src2_val);
  BOOL const_src1 = TN_Value_At_Op (src1, NULL, &src1_val);
  BOOL is_signed = MTYPE_is_signed(mtype);


  if (const_src1) {
    if (!TN_is_constant(src1))
      src1 = Gen_Literal_TN(src1_val, MTYPE_byte_size(mtype));
    TN* tmp1;
    tmp1 = Expand_Immediate_Into_Register(mtype, src1, ops); 
    src1 = tmp1;
  }

  if (const_src2) {

    // Handle trivial cases
    if (src2_val == 1 ||
	(is_signed && src2_val == -1)) {
      Expand_Immediate (result, Gen_Literal_TN(0, MTYPE_byte_size(mtype)),
			mtype, ops);
      return;
    }


    //
    // Handle powers of 2 specially.
    //
    if (Is_Power_Of_2(src2_val, mtype)) {
      Expand_Power_Of_2_Rem(result, src1, src2_val, mtype, ops);
      return;
    }

    // Handle remainders when divisions are generated in terms of multiplications
    if (CGEXP_cvrt_int_div_to_mult) {
      TN *div_tn = Build_TN_Like (result);

      if (Expand_Divide_By_Constant(div_tn, src1, src2, src2_val, mtype, ops)) {
	TN *mult_tn;

	/* Generate a multiply:
	 */
	mult_tn = Build_TN_Like (result);
	Expand_Multiply(mult_tn, mtype, div_tn, mtype, src2, mtype, ops);

	/* Subtract the result of the multiply from the original value.
	 */
	Expand_Sub(result, src1, mult_tn, mtype, ops);
	return;
      }
    }

    if (!TN_is_constant(src2))
      src2 = Gen_Literal_TN(src2_val, MTYPE_byte_size(mtype));
    TN* tmp2;
    tmp2 = Expand_Immediate_Into_Register(mtype, src2, ops); 
    src2 = tmp2;
  }

  Expand_NonConst_DivRem(NULL, result, src1, src2, mtype, ops);
}

/* ====================================================================
 *   Expand_Mod
 *
 *	Expand mod(x,y) as follows:
 *		t1=	rem(x,y)
 *		t2=	xor(t1,y)
 *		t3,t4=	cmp.lt(t2,0)
 *	  if t3 r=	t1+y
 *	  if t4 r=	t1
 *
 * ====================================================================
 */
void 
Expand_Mod (
  TN *result, 
  TN *src1, 
  TN *src2, 
  TYPE_ID mtype, 
  OPS *ops
)
{
  TN *tmp1;
  TN *tmp2;
  TN *p1;
  TN *p2;
  TOP opc;
  BOOL is_double = MTYPE_is_size_double(mtype);

  FmtAssert(FALSE,("Not Implemented"));

  /* Check for undefined operations we can detect at compile-time
   * and when enabled, generate run-time checks.
   */
  switch (Check_Divide(src1, src2, mtype, ops)) {
  case DIVCHK_BYZERO:
  case DIVCHK_OVERFLOW:
//    Build_OP(TOP_ifixup, result, ops);
    OPS_Append_Op(ops, Mk_VarOP(TOP_ifixup,1,0,&result,NULL));
    return;
  }

  /* Handle mod by power of 2 specially
   */
  INT64 src2_val;
  BOOL const_src2 = TN_Value_At_Op (src2, NULL, &src2_val);
  if (const_src2 && Is_Power_Of_2(src2_val, mtype)) {

    return;
  }
#if 0
  /* Calculate remainder 
   */
  tmp1 = Build_TN_Like(result);
  Expand_Rem(tmp1, src1, src2, mtype, ops);

  /* Are signs different? 
   */
  tmp2 = Build_TN_Like(result);
  Build_OP(TOP_noop, tmp2, True_TN, tmp1, src2, ops);

  p1 = Build_RCLASS_TN(ISA_REGISTER_CLASS_guard);
  p2 = Build_RCLASS_TN(ISA_REGISTER_CLASS_guard);
  opc = is_double ? TOP_noop : TOP_noop;
  Build_OP(opc, p1, p2, True_TN, tmp2, Zero_TN, ops);

  /* result = divisor + remainder if p1
   * result = remainder if p2
   */
  Build_OP(TOP_noop, result, p1, src2, tmp1, ops);
  Build_OP(TOP_noop, result, p2, tmp1, ops);
#endif

}

/* ====================================================================
 *   Expand_DivRem
 * ====================================================================
 */
void 
Expand_DivRem (
  TN *result, 
  TN *result2, 
  TN *src1, 
  TN *src2, 
  TYPE_ID mtype, 
  OPS *ops
)
{
  FmtAssert(FALSE,("Not Implemented"));

  /* Check for undefined operations we can detect at compile-time
   * and when enabled, generate run-time checks.
   */
  switch (Check_Divide(src1, src2, mtype, ops)) {
  case DIVCHK_BYZERO:
  case DIVCHK_OVERFLOW:
//    Build_OP(TOP_ifixup, result, ops);
//    Build_OP(TOP_ifixup, result2, ops);
    OPS_Append_Op(ops, Mk_VarOP(TOP_ifixup,1,0,&result,NULL));
    OPS_Append_Op(ops, Mk_VarOP(TOP_ifixup,1,0,&result2,NULL));
    return;
  }

  /* Usually we expect whirl operators to be folded where possible.
   * But divrem is an odd beast in that the result is a special
   * "handle" rather than a value. There is no way to produce constants.
   * Therefore in some odd instances we can get constant operands,
   * so fold them here, avoiding nasty trapping issues.
   */
  INT64 src1_val;
  BOOL const_src1 = TN_Value_At_Op (src1, NULL, &src1_val);
  INT64 src2_val;
  BOOL const_src2 = TN_Value_At_Op (src2, NULL, &src2_val);
  if (const_src1 && const_src2) {
    INT64 quot_val, rem_val;
    switch (mtype) {
    case MTYPE_I8:
      quot_val = (INT64)src1_val / (INT64)src2_val;
      rem_val = (INT64)src1_val % (INT64)src2_val;
      break;
    case MTYPE_U8:
      quot_val = (UINT64)src1_val / (UINT64)src2_val;
      rem_val = (UINT64)src1_val % (UINT64)src2_val;
      break;
    case MTYPE_U4:
      quot_val = (UINT32)src1_val / (UINT32)src2_val;
      rem_val = (UINT32)src1_val % (UINT32)src2_val;
      break;
    case MTYPE_I4:
      quot_val = (INT32)src1_val / (INT32)src2_val;
      rem_val = (INT32)src1_val % (INT32)src2_val;
      break;
    }
    BOOL is_signed = MTYPE_is_signed(mtype);
    INT tn_size = MTYPE_is_size_double(mtype) ? 8 : 4;
    Exp_Immediate(result, Gen_Literal_TN(quot_val, tn_size), is_signed, ops);
    Exp_Immediate(result2, Gen_Literal_TN(rem_val, tn_size), is_signed, ops);
    return;
  }

  /* Look for simple shift optimizations and multiply_hi optimizations:
   */
  if (const_src2) {
    ;
  }

  return;
}
