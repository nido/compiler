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
 * Module: const.c
 *
 * Revision history:
 *  12-Jun-91 - Integrated from Josie
 *  30-Apr-93 - Added constant table manipulation from stab.c
 *
 * Description:
 *
 * Routines for manipulating constants.
 *
 * ====================================================================
 * ====================================================================
 */
#ifdef USE_PCH
#include "common_com_pch.h"
#endif /* USE_PCH */
#pragma hdrstop

//#ifdef TARG_ST
//#define USE_STANDARD_TYPES 1
//#endif

#include "defs.h"

#ifdef TARG_ST
#include "defs.h"
#include "config.h"
#include "config_target.h"
#include "config_asm.h"
#include "erglob.h"
#include "opcode.h"
#include "tracing.h"
#include "mempool.h"
#include "mtypes.h"
#include "symtab.h"
#include "wn.h"
#include "strtab.h"
#include "wintrinsic.h"
#include "wn_core.h"
#include "stab.h"
#include "const.h"
#include "quad.h"
#include "quadsim.h"

#include "W_limits.h"
// [HK]
#if __GNUC__ >= 3
#include <ext/hash_map>
#else
#include <hash_map>
#endif // __GNUC__ >= 3
#if __GNUC__ >= 3
#include <cmath>
#endif // __GNUC__ >= 3
#include "namespace_trans.h"
#include <fp_class.h>
#include "W_math.h"

/* the only file that should include this */
#include "targ_const_private.h"

#include "config_debug.h" // TB: for -DEBUG:trapuv_float_value option
#include "config_TARG.h"
#else

#include "defs.h"
#include "symtab.h"
#include "wn.h"
#include "config_targ.h"
#include <hash_map>
#include "W_math.h"

#endif /* TARG_ST */

namespace {
    // unnamed namespace for function objects used for removing duplicated
    // constant STs
    struct eq_tcon {
	bool operator () (TCON_IDX t1_idx, TCON_IDX t2_idx) const {
	    if (t1_idx == t2_idx)
		return TRUE;
	    const TCON& t1 = Tcon_Table[t1_idx];
	    const TCON& t2 = Tcon_Table[t2_idx];

	    if (TCON_ty (t1) != TCON_ty (t2))
		return FALSE;
	    if (t1.flags != t2.flags)
		return FALSE;

	    switch (TCON_ty (t1)) {

	    case MTYPE_I1:
	    case MTYPE_I2:
	    case MTYPE_I4:
	    case MTYPE_I8:
	    case MTYPE_U1:
	    case MTYPE_U2:
	    case MTYPE_U4:
	    case MTYPE_U8:
		return TCON_i0 (t1) == TCON_i0 (t2);

	    case MTYPE_F4:
		// Need to use integer inequality because need to match exact bit pattern
		return TCON_ival (t1) == TCON_ival (t2);

	    case MTYPE_F8:
		// Need to use integer inequality because need to match exact bit pattern
		return TCON_k0 (t1) == TCON_k0 (t2);
	    
	    case MTYPE_STR:
		return (TCON_str_idx (t1) == TCON_str_idx (t2) &&
			TCON_str_len (t1) == TCON_str_len (t2));

	    default:
		return memcmp (&t1, &t2, sizeof(TCON)) == 0;
	    }
	}
    };

    struct tcon_hash {
	size_t operator() (TCON_IDX tcon_idx) const {
	    const TCON& tcon = Tcon_Table[tcon_idx];
	    size_t val = TCON_ty (tcon);
	    val ^= TCON_ival (tcon);
	    return val;
	}
    };
} // unnamed namespace
		
// [HK]
// typedef std::hash_map<TCON_IDX, ST *, tcon_hash, eq_tcon> TCON_MERGE;
typedef __GNUEXT::hash_map<TCON_IDX, ST *, tcon_hash, eq_tcon> TCON_MERGE;

BE_EXPORTED ST *
New_Const_Sym (TCON_IDX tcon, TY_IDX ty)
{
    static TCON_MERGE merge;

    TCON_MERGE::iterator iter = merge.find (tcon);

    ST* st;
    if (iter == merge.end ()) {
	// create new constant
	st = New_ST (GLOBAL_SYMTAB);

	ST_Init (st, 0, CLASS_CONST, SCLASS_FSTATIC, EXPORT_LOCAL, ty);
	Set_ST_tcon (st, tcon);
	Set_ST_is_initialized (st);
	merge[tcon] = st;
    } else {
	st = (*iter).second;
	Is_True (ST_class (st) == CLASS_CONST &&
		 ST_sclass (st) == SCLASS_FSTATIC &&
		 ST_export (st) == EXPORT_LOCAL &&
		 ST_is_initialized (st), ("Mismatched const ST"));
    }
    return st;
	
} // New_Const_Sym


ST *
Gen_String_Sym (TCON *val, TY_IDX ty, BOOL)
{
    return New_Const_Sym (Enter_tcon (*val), ty);
}


/* ====================================================================
 *
 * Const_Val
 *
 * Return the value of a constant node.
 *
 * WARNING:  The node passed must be know to be one of those with a
 * ND_dec field pointing to a CLASS_CONST symbol.
 *
 * ====================================================================
 */

TCON
Const_Val ( WN *n )
{
    return WN_val (n);
}


/* ====================================================================
 *
 * Make_Const
 *
 * Make a constant node of the given value.
 *
 * ====================================================================
 */

WN *
Make_Const ( TCON c )
{
   WN *n;
   OPCODE opc;
   ST *csym;
  

#ifdef Is_True_On
  Check_TCON(&c);
#endif /* Is_True_On */

  switch(TCON_ty(c)) {
#ifdef TARG_STxP70
    case MTYPE_I4:
      opc = OPC_I4CONST;
      break;
#endif
    case MTYPE_F4:
      opc = OPC_F4CONST;
      break;
    case MTYPE_F8:
      opc = OPC_F8CONST;
      break;
     case MTYPE_FQ:
      opc = OPC_FQCONST;
      break;
    case MTYPE_C4:
      opc = OPC_C4CONST;
      break;
    case MTYPE_C8:
      opc = OPC_C8CONST;
      break;
    case MTYPE_CQ:
      opc = OPC_CQCONST;
      break;
    default:
      Is_True ( FALSE, ( "Make_Const can not handle %s",
			 Mtype_Name(TCON_ty(c)) ) );
      return NULL;
  }
   
   csym = New_Const_Sym (Enter_tcon (c), Be_Type_Tbl(TCON_ty(c)));
   n = WN_CreateConst(opc, csym );
   return (n);
   
}

#ifndef MONGOOSE_BE
/* ====================================================================
 *
 * Make_Zerocon
 *
 * Make a zero node of the given type.
 *
 * ====================================================================
 */

WN *
Make_Zerocon ( TYPE_ID ty )
{
  return Make_Const ( Targ_Conv ( ty, Host_To_Targ ( MTYPE_I4, 0 ) ));
}


/* ====================================================================
 *
 * Make_Comparison_Result_Const
 *
 * Make a TRUE or FALSE node of the default comparison result type.
 *
 * ====================================================================
 */

WN *
Make_Comparison_Result_Const ( INT16 val )
{
  return Make_Const ( Host_To_Targ ( Comparison_Result_Mtype, val ) );
}


/* ====================================================================
 *
 * Make_Integer_Const
 *
 * Make an integer constant node of the given type and value.
 *
 * ====================================================================
 */

WN *
Make_Integer_Const ( INT16 mtype, TARG_INT val )
{
   /* Actually, in WHIRL there are no integer constants of the symbol table type */
   /*
    *  return Make_Const ( Host_To_Targ ( mtype, val ) );
    */

   /* NOTE: TARG_INT should be INT64 for this to work! */
   return (WN_CreateIntconst(OPCODE_make_op(OPR_INTCONST,mtype,MTYPE_V), val));

}

#endif /* MONGOOSE_BE */

/******************************************************************************
 This routine makes a WHIRL node representing the reduction
 identity for a given WHIRL op. For example, the reduction identity 
 for + is 0, for * is 1.  The only tricky part is getting the value
 and constant types correct for each mtype (particularly partial-word
 integers.
*******************************************************************************/

WN * Make_Reduction_Identity ( INT32 opr, TYPE_ID mtype )
{
   WN * r;
   TYPE_ID ntype;

   if ((mtype == MTYPE_I1) || (mtype == MTYPE_I2))
     ntype = MTYPE_I4;
   else if ((mtype == MTYPE_U1) || (mtype == MTYPE_U2))
     ntype = MTYPE_U4;
   else
     ntype = mtype;

   switch (opr) {

    case OPR_ADD:
    case OPR_SUB:
      switch (mtype) {
       case MTYPE_I1:
       case MTYPE_I2:
       case MTYPE_I4:
       case MTYPE_I8:
       case MTYPE_U1:
       case MTYPE_U2:
       case MTYPE_U4:
       case MTYPE_U8:
	 r = WN_CreateIntconst ( OPCODE_make_op ( OPR_INTCONST, ntype,
						 MTYPE_V), 0 );
	 break;

       case MTYPE_F4:
       case MTYPE_F8:
       case MTYPE_FQ:
       case MTYPE_C4:
       case MTYPE_C8:
       case MTYPE_CQ:
	 r = Make_Const ( Host_To_Targ_Float ( ntype, 0.0 ) );
	 break;

       default:
	 Fail_FmtAssertion ( "No reduction identity for operator %d, type %d",
			    opr, mtype);
      }
      break;

    case OPR_MPY:
    case OPR_DIV:
      switch (mtype) {
       case MTYPE_I1:
       case MTYPE_I2:
       case MTYPE_I4:
       case MTYPE_I8:
       case MTYPE_U1:
       case MTYPE_U2:
       case MTYPE_U4:
       case MTYPE_U8:
	 r = WN_CreateIntconst ( OPCODE_make_op ( OPR_INTCONST, ntype,
						 MTYPE_V), 1 );
	 break;

       case MTYPE_F4:
       case MTYPE_F8:
       case MTYPE_FQ:
       case MTYPE_C4:
       case MTYPE_C8:
       case MTYPE_CQ:
	 r = Make_Const ( Host_To_Targ_Float ( ntype, 1.0 ) );
	 break;

       default:
	 Fail_FmtAssertion ( "No reduction identity for operator %d, type %d",
			    opr, mtype);
      }
      break;

    case OPR_MAX:
      switch (mtype) {
       case MTYPE_I1:
	 r = WN_CreateIntconst ( OPCODE_make_op ( OPR_INTCONST, ntype,
						 MTYPE_V), -128LL );
	 break;

       case MTYPE_I2:
	 r = WN_CreateIntconst ( OPCODE_make_op ( OPR_INTCONST, ntype,
						 MTYPE_V), -32768LL );
	 break;

       case MTYPE_I4:
	 r = WN_CreateIntconst ( OPCODE_make_op ( OPR_INTCONST, ntype,
						 MTYPE_V), -2147483648LL);
	 break;

       case MTYPE_I8:
	 r = WN_CreateIntconst ( OPCODE_make_op ( OPR_INTCONST, ntype,
						 MTYPE_V),
				0x8000000000000000LL );
	 break;

       case MTYPE_U1:
       case MTYPE_U2:
       case MTYPE_U4:
       case MTYPE_U8:
	 r = WN_CreateIntconst ( OPCODE_make_op ( OPR_INTCONST, ntype,
						 MTYPE_V), 0 );
	 break;

       case MTYPE_F4:
       case MTYPE_F8:
       case MTYPE_FQ:
	 r = Make_Const ( Host_To_Targ_Float ( ntype, -HUGE_VAL ) );
	 break;

       default:
	 Fail_FmtAssertion ( "No reduction identity for operator %d, type %d",
			    opr, mtype);
      }
      break;

    case OPR_MIN:
      switch (mtype) {
       case MTYPE_I1:
	 r = WN_CreateIntconst ( OPCODE_make_op ( OPR_INTCONST, ntype,
						 MTYPE_V), 0x7f );
	 break;

       case MTYPE_I2:
	 r = WN_CreateIntconst ( OPCODE_make_op ( OPR_INTCONST, ntype,
						 MTYPE_V), 0x7fff );
	 break;

       case MTYPE_I4:
	 r = WN_CreateIntconst ( OPCODE_make_op ( OPR_INTCONST, ntype,
						 MTYPE_V), 0x7fffffff );
	 break;

       case MTYPE_I8:
	 r = WN_CreateIntconst ( OPCODE_make_op ( OPR_INTCONST, ntype,
						 MTYPE_V),
				0x7fffffffffffffffLL );
	 break;

       case MTYPE_U1:
	 r = WN_CreateIntconst ( OPCODE_make_op ( OPR_INTCONST, ntype,
						 MTYPE_V), 0xff );
	 break;

       case MTYPE_U2:
	 r = WN_CreateIntconst ( OPCODE_make_op ( OPR_INTCONST, ntype,
						 MTYPE_V), 0xffff );
	 break;

       case MTYPE_U4:
	 r = WN_CreateIntconst ( OPCODE_make_op ( OPR_INTCONST, ntype,
						 MTYPE_V), 0xffffffff );
	 break;

       case MTYPE_U8:
	 r = WN_CreateIntconst ( OPCODE_make_op ( OPR_INTCONST, ntype,
						 MTYPE_V),
				0xffffffffffffffffLL );
	 break;

       case MTYPE_F4:
       case MTYPE_F8:
       case MTYPE_FQ:
	 r = Make_Const ( Host_To_Targ_Float ( ntype, HUGE_VAL ) );
	 break;

       default:
	 Fail_FmtAssertion ( "No reduction identity for operator %d, type %d",
			    opr, mtype);
      }
      break;

    case OPR_BIOR:
    case OPR_BXOR:
      switch (mtype) {
       case MTYPE_I1:
       case MTYPE_I2:
       case MTYPE_I4:
       case MTYPE_I8:
       case MTYPE_U1:
       case MTYPE_U2:
       case MTYPE_U4:
       case MTYPE_U8:
	 r = WN_CreateIntconst ( OPCODE_make_op ( OPR_INTCONST, ntype,
						 MTYPE_V), 0 );
	 break;

       default:
	 Fail_FmtAssertion ( "No reduction identity for operator %d, type %d",
			    opr, mtype);
      }
      break;

    case OPR_BAND:
      switch (mtype) {
       case MTYPE_I1:
       case MTYPE_U1:
	 r = WN_CreateIntconst ( OPCODE_make_op ( OPR_INTCONST, ntype,
						 MTYPE_V), 0xff );
	 break;

       case MTYPE_I2:
       case MTYPE_U2:
	 r = WN_CreateIntconst ( OPCODE_make_op ( OPR_INTCONST, ntype,
						 MTYPE_V), 0xffff );
	 break;

       case MTYPE_I4:
       case MTYPE_U4:
	 r = WN_CreateIntconst ( OPCODE_make_op ( OPR_INTCONST, ntype,
						 MTYPE_V), 0xffffffff );
	 break;

       case MTYPE_I8:
       case MTYPE_U8:
	 r = WN_CreateIntconst ( OPCODE_make_op ( OPR_INTCONST, ntype,
						 MTYPE_V),
				0xffffffffffffffffLL );
	 break;

       default:
	 Fail_FmtAssertion ( "No reduction identity for operator %d, type %d",
			    opr, mtype);
      }
      break;

    case OPR_LIOR:
      r = WN_CreateIntconst ( OPCODE_make_op ( OPR_INTCONST, ntype, MTYPE_V ),
			     0 );
      break;
	
    case OPR_LAND:
      r = WN_CreateIntconst ( OPCODE_make_op ( OPR_INTCONST, ntype, MTYPE_V ),
			     1 );
      break;
	
    case OPR_EQ:    /* Fortran .EQV. operator on type LOGICAL */
    case OPR_NE:    /* Fortran .NEQV. operator on type LOGICAL */
    case OPR_CAND:  /* C/C++ short-circuit "&&" on integral or floating-
                       point type */
    case OPR_CIOR:  /* C/C++ short-circuit "||" on int/float */
      switch (mtype) {
       case MTYPE_I1:
       case MTYPE_I2:
       case MTYPE_I4:
       case MTYPE_I8:
       case MTYPE_U1:
       case MTYPE_U2:
       case MTYPE_U4:
       case MTYPE_U8:
	 r = WN_CreateIntconst ( OPCODE_make_op ( OPR_INTCONST, ntype,
						 MTYPE_V),
                                 (opr == OPR_EQ || opr == OPR_CAND) ? 1
                                                                    : 0 );
	 break;

       case MTYPE_F4:
       case MTYPE_F8:
       case MTYPE_FQ:
        Is_True(opr == OPR_CAND || opr == OPR_CIOR,
	        ("bad opr %d", (INT) opr));
        r = Make_Const(Host_To_Targ_Float(ntype,
                                          (opr == OPR_CAND) ? 1.0 : 0.0));
        break;

       default:
	 Fail_FmtAssertion ( "No reduction identity for operator %d, type %d",
			    opr, mtype);
      }
      break;

    default:
      Fail_FmtAssertion ( "No reduction identity for operator %d, type %d", opr,
			 mtype);
      break;
   }
   return (r);
}

#ifdef TARG_ST

/* 
 * Arthur: moved all this code here from targ_const.cxx -- seems
 *         quite target-independent to me.
 */

/* ====================================================================
 * ====================================================================
 *
 * Target constant handling.
 *
 * Routines in this file are supposed to do operations of compile time
 * constants in a host independent way. Using these routines, the
 * compiler should be able to guarantee that the same source will give
 * same object, no matter which machine the compiler is hosted on.
 *
 * TCON is a data type used to represent all scalar and string
 * compile-time constants.  Its internals should be visible only inside
 * this file.  To facilitate constant operations, we probably want to
 * keep constants in something resembling the host representation, but
 * that is not necessary for the rest of the compiler.  The routines
 * below must be consistent among  themselves in interpreting TCON and
 * doing operations on them.
 *
 * Theoretically rehosting the compiler to machines with different
 * byte-order or different floating point format should require changes
 * only to internal structure of TCON and to targ_const.c. It should
 * even be possible to keep constants in some kind of symbolic way and
 * do symbolic constant folding if the target representation is not
 * known to some phases of the  compiler.
 *
 * The rest of the compiler must not assume anything about internals of
 * TCON, except that it keeps the type of the constant within itself.
 * In particular, the size of TCON must not be assumed to related to
 * the size of the represented constant in any way.
 *
 * ====================================================================
 * ====================================================================
 */

/* For fp_class */

#define DMANTWIDTH      52
#define DEXPWIDTH       11
#define DSIGNMASK       0x7fffffffffffffffll
#define DEXPMASK        0x800fffffffffffffll
#define DQNANBITMASK    0xfff7ffffffffffffll

#define MANTWIDTH       23
#define EXPWIDTH        8
#define SIGNMASK        0x7fffffff
#define EXPMASK         0x807fffff
#define QNANBITMASK     0xffbfffff

/* Just to make sure we have this, it should come from math.h */
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define MIN_INT_I8	(((mINT64)1)<<(sizeof(mINT64)*8 - 1))
#define MIN_INT_I4	(1<<(sizeof(mINT32)*8 - 1))
#define MIN_INT_I2	(1<<(sizeof(mINT16)*8 - 1))

/* results of comparisons */
#define LOGICAL_MTYPE   MTYPE_U4

//#include "targ_const_private.h"

/* This initailization must be static because it may be used
 * before first call to Targ_WhirlOp.  Also WARNING: It requires
 * that TCON_ty field be first one in TCON
 */
TCON Zero_I4_Tcon = { MTYPE_I4 };
TCON Zero_I8_Tcon = { MTYPE_I8 };
TCON Quad_Zero_Tcon = { MTYPE_FQ };

static TCON Targ_Ipower(TCON base, UINT64 exp, BOOL neg_exp, BOOL *folded, TYPE_ID btype);
static TCON Targ_Power(TCON base, TCON exp, BOOL *folded, TYPE_ID btype);


TCON Targ_Conv ( TYPE_ID ty_to, TCON c ); /* Defined later, used in Targ_WhirlOp */

/* ====================================================================
 * This is the safe way of converting a QUAD_TYPE to a quad and vice
 * versa, and replaces:
 *
 *     #define TCON_RQ(c)	(*((quad *)&(c).vals.dval))
 *
 * This #define may be fatal should a TCON have a smaller alignment 
 * than a quad. 
 * ====================================================================
*/
typedef union QUAD_REPRESENTATION
{
   QUAD_TYPE a_quadtype;
   QUAD      a_quad;
#ifdef QUAD_PRECISION_SUPPORTED
   long double a_longdouble;
#endif
} Quad_Representation;

static QUAD
R16_To_RQ(QUAD_TYPE qt)
{
   Quad_Representation repr;
   
   repr.a_quadtype = qt;
   return repr.a_quad;
}

static QUAD_TYPE
RQ_To_R16(QUAD q)
{
   Quad_Representation repr;
   
   repr.a_quad = q;
   return repr.a_quadtype;
}

#ifdef QUAD_PRECISION_SUPPORTED
static long double
R16_To_RLD(QUAD_TYPE qt)
{
   Quad_Representation repr;
   
   repr.a_quadtype = qt;
   return repr.a_longdouble;
}

static QUAD_TYPE
RLD_To_R16(long double q)
{
   Quad_Representation repr;
   
   repr.a_longdouble = q;
   return repr.a_quadtype;
}
#endif

TCON
Extract_Quad_Hi(TCON v)
{
  TCON c;
  TCON_clear(c);

  switch (TCON_ty(v)) {
  case MTYPE_FQ:
    {
      QUAD	quadTemp = R16_To_RQ(TCON_R16(v));

      TCON_ty(c) = MTYPE_F8;
      Set_TCON_R8(c, quadTemp.hi);
      return c;
    }
  /***************************************************
	TO BE DETERMINED
  case MTYPE_CQ:
    {
       quad	quadTemp = R16_To_RQ(TCON_IR16(v));

       TCON_ty(c) = MTYPE_C8;
       Set_TCON_R16(c, TCON_IR16(complex));
       return c;
    }
  ***************************************************/
  default:
     ErrMsg ( EC_Inv_Mtype, Mtype_Name(TCON_ty(v)), "Extract_Quad_Hi" );
     TCON_ty(c) = MTYPE_F4;
     return c;
  }
}

TCON
Extract_Quad_Lo(TCON v)
{
  TCON c;
  TCON_clear(c);

  switch (TCON_ty(v)) {
  case MTYPE_FQ:
    {
      QUAD	quadTemp = R16_To_RQ(TCON_R16(v));

      TCON_ty(c) = MTYPE_F8;
      Set_TCON_R8(c, quadTemp.lo);
      return c;
    }
  /***************************************************
	TO BE DETERMINED
  case MTYPE_CQ:
    {
       quad	quadTemp = R16_To_RQ(TCON_IR16(v));

       TCON_ty(c) = MTYPE_C8;
       Set_TCON_R16(c, TCON_IR16(complex));
       return c;
    }
  ***************************************************/
  default:
     ErrMsg ( EC_Inv_Mtype, Mtype_Name(TCON_ty(v)), "Extract_Quad_Lo" );
     TCON_ty(c) = MTYPE_F4;
     return c;
  }
}

/* ====================================================================
 *   Representation for long doubles
 * ====================================================================
 */

typedef	struct {
#if HOST_IS_LITTLE_ENDIAN
  float lo;
  float hi;
#else
  float hi;
  float lo;
#endif
} DOUBLE;

typedef union DOUBLE_REPRESENTATION {
   double  a_doubletype;
   DOUBLE  a_double;
} Double_Representation;

static DOUBLE
R8_To_RD(double dt)
{
   Double_Representation repr;
   
   repr.a_doubletype = dt;
   return repr.a_double;
}

static double
RD_To_R8(DOUBLE d)
{
   Double_Representation repr;
   
   repr.a_double = d;
   return repr.a_doubletype;
}

/* ====================================================================
 *   Extract_Double_Hi/Extract_Double_Lo
 * ====================================================================
 */
TCON
Extract_Double_Hi(TCON v)
{
  TCON c;
  TCON_clear(c);

  DOUBLE doubleTemp = R8_To_RD(TCON_R8(v));

#ifdef TARG_STxP70
  if(Enable_Fpx) {
    TCON_ty(c) = MTYPE_I4;
  }
  else {
    TCON_ty(c) = MTYPE_F4;
  }
#else
  TCON_ty(c) = MTYPE_F4;
#endif

#if __GNUC__ < 3
  // [CL] copying floats on x86 with gcc-2.95 implies conversions
  // which may change the copied value (especially in our case where
  // the lo/hi parts of a double are not generally "plain" floats
  // They are likely to be NaNs and as such candidate to conversions
  // which should apply. It would be much cleaner to split a double
  // into 2 parts which are *not* float and have special types for
  /// the compiler
  memcpy(&TCON_R4(c), &doubleTemp.hi, sizeof(doubleTemp.hi));
#else
  Set_TCON_R4(c, doubleTemp.hi);
#endif
  return c;
}


TCON
Extract_Double_Lo(TCON v)
{
  TCON c;
  TCON_clear(c);

  DOUBLE doubleTemp = R8_To_RD(TCON_R8(v));

#ifdef TARG_STxP70
  if(Enable_Fpx) {
    TCON_ty(c) = MTYPE_I4;
  }
  else {
    TCON_ty(c) = MTYPE_F4;
  }
#else
  TCON_ty(c) = MTYPE_F4;
#endif

#if __GNUC__ < 3
  // [CL] copying floats on x86 with gcc-2.95 implies conversions
  // which may change the copied value (especially in our case where
  // the lo/hi parts of a double are not generally "plain" floats
  // They are likely to be NaNs and as such candidate to conversions
  // which should apply. It would be much cleaner to split a double
  // into 2 parts which are *not* float and have special types for
  /// the compiler
  memcpy(&TCON_R4(c), &doubleTemp.lo, sizeof(doubleTemp.lo));
#else
  Set_TCON_R4(c, doubleTemp.lo);
#endif
  return c;
}

/* ====================================================================
 *   Representation for long long
 * ====================================================================
 */

typedef	struct {
#if HOST_IS_LITTLE_ENDIAN
  INT32 lo;
  INT32 hi;
#else
  INT32 hi;
  INT32 lo;
#endif
} SLONGLONG;

typedef	struct {
#if HOST_IS_LITTLE_ENDIAN
  UINT32 lo;
  UINT32 hi;
#else
  UINT32 hi;
  UINT32 lo;
#endif
} ULONGLONG;

typedef union LONGLONG_REPRESENTATION {
   INT64 a_doubletype;
   SLONGLONG  a_double;
} LongLong_Representation;

typedef union ULONGLONG_REPRESENTATION {
   UINT64 a_doubletype;
   ULONGLONG  a_double;
} ULongLong_Representation;

static SLONGLONG
I8_To_RL(long long dt)
{
   LongLong_Representation repr;
   
   repr.a_doubletype = dt;
   return repr.a_double;
}

static INT64
RL_To_I8(SLONGLONG d)
{
   LongLong_Representation repr;
   
   repr.a_double = d;
   return repr.a_doubletype;
}

static ULONGLONG
U8_To_RL(long long dt)
{
   ULongLong_Representation repr;
   
   repr.a_doubletype = dt;
   return repr.a_double;
}

static INT64
RL_To_U8(ULONGLONG d)
{
   ULongLong_Representation repr;
   
   repr.a_double = d;
   return repr.a_doubletype;
}

/* ====================================================================
 *   Extract_LongLong_Hi/Extract_LongLong_Lo
 * ====================================================================
 */
TCON
Extract_LongLong_Hi(TCON v)
{
  TCON c;
  TCON_clear(c);

  switch (TCON_ty(v)) {

  case MTYPE_I8: {
    SLONGLONG longlongTemp = I8_To_RL(TCON_I8(v));

    TCON_ty(c) = MTYPE_I4;
    TCON_I4(c) = longlongTemp.hi;
    break;
  }
  case MTYPE_U8: {
    ULONGLONG ulonglongTemp = U8_To_RL(TCON_U8(v));

    TCON_ty(c) = MTYPE_U4;
    TCON_U4(c) = ulonglongTemp.hi;
    break;
  }
  default:
    break;
  }

  return c;
}


TCON
Extract_LongLong_Lo(TCON v)
{
  TCON c;
  TCON_clear(c);

  switch (TCON_ty(v)) {

  case MTYPE_I8: {
    SLONGLONG longlongTemp = I8_To_RL(TCON_I8(v));

    TCON_ty(c) = MTYPE_I4;
    TCON_I4(c) = longlongTemp.lo;
    break;
  }
  case MTYPE_U8: {
    ULONGLONG ulonglongTemp = U8_To_RL(TCON_U8(v));

    TCON_ty(c) = MTYPE_U4;
    TCON_U4(c) = ulonglongTemp.lo;
    break;
  }
  default:
    break;
  }

  return c;
}

/* ====================================================================
 *   Check_TCON
 * ====================================================================
 */
#ifdef Is_True_On
void
Check_TCON ( TCON *tc )
{
  switch (TCON_ty(*tc)) {
    case MTYPE_I1:
    case MTYPE_I2:
    case MTYPE_I4:
    case MTYPE_U1:
    case MTYPE_U2:
    case MTYPE_U4:
    case MTYPE_A4:
    case MTYPE_F4:
     Is_True ( TCON_v1(*tc)|TCON_v2(*tc)|TCON_v3(*tc) == 0,
	       ("High order word of %s TCON non zero %x",
		Mtype_Name(TCON_ty(*tc)), TCON_v1(*tc)) );
     break;
    case MTYPE_I8:
    case MTYPE_U8:
    case MTYPE_A8:
    case MTYPE_F8:
     Is_True ( TCON_v2(*tc)|TCON_v3(*tc) == 0,
	       ("High order word of %s TCON non zero %x",
		Mtype_Name(TCON_ty(*tc)), TCON_v1(*tc)) );
     break;
#ifdef TARG_NEEDS_QUAD_OPS
    case MTYPE_FQ:
    {
     QUAD q = R16_To_RQ(TCON_R16(*tc));
     /*
       In Fortran, a user can specify an illegal quad constant
       using VMS-style bit constants, so we should just give
       a warning.
     */
     if ( q.hi == 0.0 && q.lo != 0.0)
      ErrMsg( EC_Ill_Quad_Const, TCON_u0(*tc), TCON_u1(*tc), TCON_u2(*tc), TCON_u3(*tc));
    }
     break;
#endif
    default:
     break;
  }
} /* Check_TCON */
#endif /* Is_True_On */

/*******************************************************************************
 * This is a set of routines for complex arithmetic on 
 * complex numbers. Much of this was lifted from libF77.a
 */

static TCON complex_sqrt(TCON c0)
{
   float fr,fi,fmag;
   double dr,di,dmag;
   TCON r;

   TCON_clear(r);
   TCON_ty(r) = TCON_ty(c0);

   switch (TCON_ty(c0)) {
    case MTYPE_C4:
      fr = TCON_R4(c0);
      fi = TCON_IR4(c0);
      if( (fmag = hypotf(fr, fi)) == 0.)
	TCON_R4(r) = TCON_IR4(r) = 0.;
      else if (fr > 0) {
	 TCON_R4(r) = sqrtf(0.5 * (fmag + fr) );
	 TCON_IR4(r) = fi / TCON_R4(r) / 2;
      } else {
	 TCON_IR4(r) = sqrtf(0.5 * (fmag - fr) );
	 if (fi < 0) TCON_IR4(r) = -TCON_IR4(r);
	 TCON_R4(r) = fi / TCON_IR4(r) /2;
      }
      break;
      
    case MTYPE_C8:
      dr = TCON_R8(c0);
      di = TCON_IR8(c0);
      if( (dmag = hypot(dr, di)) == 0.)
	TCON_R8(r) = TCON_IR8(r) = 0.;
      else if (dr > 0) {
	 TCON_R8(r) = sqrt(0.5 * (dmag + dr) );
	 TCON_IR8(r) = di / TCON_R8(r) / 2;
      } else {
	 TCON_IR8(r) = sqrt(0.5 * (dmag - dr) );
	 if (di < 0) TCON_IR8(r) = -TCON_IR8(r);
	 TCON_R8(r) = di / TCON_IR8(r) /2;
      }
      break;
      
#ifdef TARG_NEEDS_QUAD_OPS
    case MTYPE_CQ:
      QUAD qr,qi,qmag,rqr,rqi,q0,q05;
      INT err;
      qr = R16_To_RQ(TCON_R16(c0));
      qi = R16_To_RQ(TCON_IR16(c0));
      q0 = __c_q_ext(0.0,&err);
      q05 = __c_q_ext(0.5,&err);

#ifdef TODO_MONGOOSE
      /* This set of stuff needs to be re-written to make use of the
	 qhypot function, when we figure out how to make it available*/
      /* qmag = qhypot(qr,qi); */
#endif
      qmag = __c_q_sqrt(__c_q_add(__c_q_mul(qi,qi,&err),
				  __c_q_mul(qr,qr,&err),
				  &err),&err);
      if (__c_q_eq(qmag,q0,&err)) {
	 rqr = q0;
	 rqi = q0;
      }
      else if (__c_q_gt(qr,q0,&err)) {
	 rqr = __c_q_sqrt(__c_q_mul(q05,__c_q_add(qmag,qr,&err),&err),&err);
	 rqi = __c_q_mul(q05,__c_q_div(qi,rqr,&err),&err);
      } else {
	 rqi = __c_q_sqrt(__c_q_mul(q05,__c_q_sub(qmag,qr,&err),&err),&err);
	 if (__c_q_lt(qi,q0,&err)) rqi = __c_q_neg(rqi,&err);
	 rqr = __c_q_mul(q05,__c_q_div(qi,rqi,&err),&err);
      }
      TCON_R16(r) = RQ_To_R16(rqr);
      TCON_IR16(r) = RQ_To_R16(rqi);
      break;
#endif

    default:
      ErrMsg ( EC_Inv_Mtype, Mtype_Name(TCON_ty(c0)), "complex_sqrt" );
   }

   return (r);
}

static TCON complex_divide(TCON c0, TCON c1)
{
   TCON r;

   TCON_clear(r);
   TCON_ty(r) = TCON_ty(c0);

   switch (TCON_ty(c0)) {
    case MTYPE_C4:
      {
	 float c0r,c0i,c1r,c1i,t1,t2,t3;
	 c0r = TCON_R4(c0);
	 c0i = TCON_IR4(c0);
	 c1r = TCON_R4(c1);
	 c1i = TCON_IR4(c1);
#if __GNUC__ >= 3
	 if (std::fabs(c1r) < std::fabs(c1i)) {
#else
	 if (fabsf(c1r) < fabsf(c1i)) {
#endif
	    t2 = c1r*(c1r/c1i) + c1i;
	    t1 = (c0i*(c1r/c1i) - c0r)/t2;
	    t3 = (c0r*(c1r/c1i) + c0i)/t2;
	 } else {
	    t2 = c1i*(c1i/c1r) + c1r;
	    t1 = (c0i - c0r*(c1i/c1r))/t2;
	    t3 = (c0i*(c1i/c1r) + c0r)/t2;
	 }	    
	 TCON_R4(r) = t3;
	 TCON_IR4(r) = t1;
      }
      break;
      
    case MTYPE_C8:
      {
	 double c0r,c0i,c1r,c1i,t1,t2,t3;
	 c0r = TCON_R8(c0);
	 c0i = TCON_IR8(c0);
	 c1r = TCON_R8(c1);
	 c1i = TCON_IR8(c1);
#if __GNUC__ >= 3
	 if (std::abs(c1r) < std::abs(c1i)) {
#else
	 if (fabs(c1r) < fabs(c1i)) {
#endif
	    t2 = c1r*(c1r/c1i) + c1i;
	    t1 = (c0i*(c1r/c1i) - c0r)/t2;
	    t3 = (c0r*(c1r/c1i) + c0i)/t2;
	 } else {
	    t2 = c1i*(c1i/c1r) + c1r;
	    t1 = (c0i - c0r*(c1i/c1r))/t2;
	    t3 = (c0i*(c1i/c1r) + c0r)/t2;
	 }	    
	 TCON_R8(r) = t3;
	 TCON_IR8(r) = t1;
      }
      break;
      
#ifdef TARG_NEEDS_QUAD_OPS
    case MTYPE_CQ:
      {
	 QUAD c0r,c0i,c1r,c1i,t1,t2,t3,t4,ar,ai,q0;
	 INT err;
	 q0 = __c_q_ext(0.0,&err);
	 c0r = R16_To_RQ(TCON_R16(c0));
	 c0i = R16_To_RQ(TCON_IR16(c0));
	 c1r = R16_To_RQ(TCON_R16(c1));
	 c1i = R16_To_RQ(TCON_IR16(c1));
	 ar = c1r;
	 ai = c1i;
	 if (__c_q_lt(ar,q0,&err)) ar = __c_q_neg(ar,&err);
	 if (__c_q_lt(ai,q0,&err)) ai = __c_q_neg(ai,&err);
	 if (__c_q_lt(ar,ai,&err)) {
	    t4 = __c_q_div(c1r,c1i,&err);
	    t2 = __c_q_add(c1i,__c_q_mul(c1r,t4,&err),&err);
	    t1 = __c_q_div(__c_q_sub(__c_q_mul(c0i,t4,&err),c0r,&err),
			   t2,&err);
	    t3 = __c_q_div(__c_q_add(__c_q_mul(c0r,t4,&err),c0i,&err),
			   t2,&err);
	 } else {
	    t4 = __c_q_div(c1i,c1r,&err);
	    t2 = __c_q_add(c1r,__c_q_mul(c1i,t4,&err),&err);
	    t1 = __c_q_div(__c_q_sub(c0i,__c_q_mul(c0r,t4,&err),&err),
			   t2,&err);
	    t3 = __c_q_div(__c_q_add(__c_q_mul(c0i,t4,&err),c0r,&err),
			   t2,&err);
	 }	    
	 TCON_R16(r) = RQ_To_R16(t3);
	 TCON_IR16(r) = RQ_To_R16(t1);
      }
      break;
#endif
      
    default:
      ErrMsg ( EC_Inv_Mtype, Mtype_Name(TCON_ty(c0)), "complex_divide" );
   }
   
   return (r);
}

/* ====================================================================
 *
 * Targ_Convert_Length
 *
 * Convert the length of constants
 *
 * ====================================================================
 */
static TCON
Targ_Convert_Length (
  TCON	c0,		/* The constant to be converted */
  TCON	c1,		/* The source length for conversion */
  TYPE_ID mtype,	/* The result's MTYPE */
  INT16	len,		/* The result bit length */
  BOOL	sign )		/* Is the conversion signed? */
{
  static const INT64 one = 1;
  INT64 sval = Targ_To_Host ( c0 );
  INT64 slen = Targ_To_Host ( c1 );

  /* If the source length is greater than the desired length, this is
   * a simple truncation:
   */
  if ( slen >= len ) {
    if ( len == 32 ) {
      mINT32 tval = sval & 0x0ffffffff;
      return Host_To_Targ ( mtype, tval );
    } else if ( len == 64 ) {
      return Host_To_Targ ( mtype, sval );
    } else {
      ErrMsg ( EC_Unimplemented, "Targ_Convert_Length: bad length 1" );
    }
  }

  if ( len == 32 ) {
    mINT32 highmask = (-1) << slen;
    mINT32 signmask = 1 << (slen-1);
    mINT32 tval = ( sval & ~highmask );

    if ( sign && ((tval & signmask) != 0) ) {
      tval |= highmask;
    }
    return Host_To_Targ ( mtype, tval );

  } else if ( len == 64 ) {
    INT64 highmask = (-one) << slen;
    INT64 signmask = one << (slen-1);
    INT64 tval = ( sval & ~highmask );

    if ( sign && ((tval & signmask) != 0) ) {
      tval |= highmask;
    }
    return Host_To_Targ ( mtype, tval );

  } else {
    ErrMsg ( EC_Unimplemented, "Targ_Convert_Length: bad length 2" );
    return c0;
  }

} /* Targ_Convert_Length */

/* ====================================================================
 *
 * Targ_WhirlOp
 *
 * Apply an WHIRL operator to TCON operands, yielding a TCON for the
 * result value.  The folded parameter, if non-NULL, is used to return
 * whether anything was done.
 *
 * It is an error to call this function with folded == NULL if in fact
 * nothing can be done (perhaps because the operator cannot be handed
 * yet).
 *
 * There are many call sites of Targ_WhirlOp with folded == NULL and
 * only a few with that check folded.  Is this right?
 *
 * TODO: Check the call sites of Targ_WhirlOp that have NULL folded
 * arguments to make sure they are doing the right thing.
 *
 * TODO Josie/92:  Install changes.
 *
 * ====================================================================
 */

/************************************
The latest word on BOTH_OPNDS: instead of complaining about the 
operands being of different types, it should now coerce both operands
to the type of the third argument. It should only do this for converts which
are essentially NOPs however.

**************************************/

#define BOTH_OPNDS(op0,op1,type) \
if (type != MTYPE_U8) { op0=Targ_Conv(type,op0);op1=Targ_Conv(type,op1); } \
TCON_ty(op0)=type

/* ====================================================================
 *
 * Targ_WhirlOp
 *
 * Given an SGIR operator (arithmetic) and one or two constant
 * operands, produce a constant expression result.  Some operators,
 * which can't always fold (e.g. division where the divisor might be
 * zero), will return an indicator in "folded" of whether they
 * succeeded; they generally take assertion failures if folded is NULL
 * and they can't fold.
 *
 * TODO:  Should these operators observe IEEE rules more completely,
 * e.g. returning NaN for min/max when one operand is NaN?
 *
 * ====================================================================
 */
TCON
Targ_WhirlOp ( 
  OPCODE op, 
  TCON c0, 
  TCON c1, 
  BOOL *folded 
)
{
#ifdef TARG_NEEDS_QUAD_OPS
   QUAD q0, q1;
   INT  err;
#endif
   TCON t1, t2;
   TYPE_ID optype;
   BOOL dummy_folded;
  
#ifdef Is_True_On
   Check_TCON (&c0);
   Check_TCON (&c1);
#endif

#undef DEBUG_FOLD
#ifdef DEBUG_FOLD
   printf("Folding %s on 0x%llx (%s), 0x%llx (%s), result",OPCODE_name(op),
	  TCON_I8(c0),Mtype_Name(TCON_ty(c0)),TCON_I8(c1),Mtype_Name(TCON_ty(c1)));
#endif

   if (!folded) {
     folded = &dummy_folded;
   }

   *folded = TRUE;
   
   optype = OPCODE_rtype(op);
   TYPE_ID desc = OPCODE_desc(op);
   OPERATOR opr = OPCODE_operator(op);

   if (OPERATOR_is_compare(opr)) {

     // Convert both operands to the type of result ??
     if (MTYPE_is_class_integer(desc)) BOTH_OPNDS(c0,c1,desc);

     switch (desc) {
     case MTYPE_I4: 
       switch (opr) {
         case OPR_EQ: TCON_I4(c0) = TCON_I4(c0) == TCON_I4(c1); break;
         case OPR_NE: TCON_I4(c0) = TCON_I4(c0) != TCON_I4(c1); break;
         case OPR_LT: TCON_I4(c0) = TCON_I4(c0) <  TCON_I4(c1); break;
         case OPR_LE: TCON_I4(c0) = TCON_I4(c0) <= TCON_I4(c1); break;
         case OPR_GT: TCON_I4(c0) = TCON_I4(c0) >  TCON_I4(c1); break;
         case OPR_GE: TCON_I4(c0) = TCON_I4(c0) >= TCON_I4(c1); break;
       } 
       break;
     case MTYPE_U4: 
       switch (opr) {
         case OPR_EQ: TCON_I4(c0) = TCON_U4(c0) == TCON_U4(c1); break;
         case OPR_NE: TCON_I4(c0) = TCON_U4(c0) != TCON_U4(c1); break;
         case OPR_LT: TCON_I4(c0) = TCON_U4(c0) <  TCON_U4(c1); break;
         case OPR_LE: TCON_I4(c0) = TCON_U4(c0) <= TCON_U4(c1); break;
         case OPR_GT: TCON_I4(c0) = TCON_U4(c0) >  TCON_U4(c1); break;
         case OPR_GE: TCON_I4(c0) = TCON_U4(c0) >= TCON_U4(c1); break;
       } 
       break;
     case MTYPE_I8:
       switch (opr) {
	 case OPR_EQ: TCON_v0(c0) = TCON_I8(c0) == TCON_I8(c1); break;
	 case OPR_NE: TCON_v0(c0) = TCON_I8(c0) != TCON_I8(c1); break;
	 case OPR_LT: TCON_v0(c0) = TCON_I8(c0) <  TCON_I8(c1); break;
	 case OPR_LE: TCON_v0(c0) = TCON_I8(c0) <= TCON_I8(c1); break;
	 case OPR_GT: TCON_v0(c0) = TCON_I8(c0) >  TCON_I8(c1); break;
	 case OPR_GE: TCON_v0(c0) = TCON_I8(c0) >= TCON_I8(c1); break;
       }
       TCON_v1(c0) = 0;
       break;
     case MTYPE_U8:
       switch (opr) {
	 case OPR_EQ: TCON_v0(c0) = TCON_U8(c0) == TCON_U8(c1); break;
	 case OPR_NE: TCON_v0(c0) = TCON_U8(c0) != TCON_U8(c1); break;
	 case OPR_LT: TCON_v0(c0) = TCON_U8(c0) <  TCON_U8(c1); break;
	 case OPR_LE: TCON_v0(c0) = TCON_U8(c0) <= TCON_U8(c1); break;
	 case OPR_GT: TCON_v0(c0) = TCON_U8(c0) >  TCON_U8(c1); break;
	 case OPR_GE: TCON_v0(c0) = TCON_U8(c0) >= TCON_U8(c1); break;
       }
       TCON_v1(c0) = 0;
       break;
     case MTYPE_F4:
       switch (opr) {
	 case OPR_EQ: TCON_v0(c0) = TCON_R4(c0) == TCON_R4(c1); break;
	 case OPR_NE: TCON_v0(c0) = TCON_R4(c0) != TCON_R4(c1); break;
	 case OPR_LT: TCON_v0(c0) = TCON_R4(c0) <  TCON_R4(c1); break;
	 case OPR_LE: TCON_v0(c0) = TCON_R4(c0) <= TCON_R4(c1); break;
	 case OPR_GT: TCON_v0(c0) = TCON_R4(c0) >  TCON_R4(c1); break;
	 case OPR_GE: TCON_v0(c0) = TCON_R4(c0) >= TCON_R4(c1); break;
       }
       TCON_v1(c0) = 0;
       break;
     case MTYPE_F8:
       switch (opr) {
	 case OPR_EQ: TCON_v0(c0) = TCON_R8(c0) == TCON_R8(c1); break;
	 case OPR_NE: TCON_v0(c0) = TCON_R8(c0) != TCON_R8(c1); break;
	 case OPR_LT: TCON_v0(c0) = TCON_R8(c0) <  TCON_R8(c1); break;
	 case OPR_LE: TCON_v0(c0) = TCON_R8(c0) <= TCON_R8(c1); break;
	 case OPR_GT: TCON_v0(c0) = TCON_R8(c0) >  TCON_R8(c1); break;
	 case OPR_GE: TCON_v0(c0) = TCON_R8(c0) >= TCON_R8(c1); break;
       }
       TCON_v1(c0) = 0;
       break;
#ifdef TARG_NEEDS_QUAD_OPS
     case MTYPE_FQ:
       q0 = R16_To_RQ(TCON_R16(c0));
       q1 = R16_To_RQ(TCON_R16(c1));
       switch (opr) {
       case OPR_EQ: TCON_v0(c0) = __c_q_eq ( q0, q1, &err ); break;
       case OPR_NE: TCON_v0(c0) = __c_q_ne ( q0, q1, &err ); break;
       case OPR_LT: TCON_v0(c0) = __c_q_lt ( q0, q1, &err ); break;
       case OPR_LE: TCON_v0(c0) = __c_q_le ( q0, q1, &err ); break;
       case OPR_GT: TCON_v0(c0) = __c_q_gt ( q0, q1, &err ); break;
       case OPR_GE: TCON_v0(c0) = __c_q_ge ( q0, q1, &err ); break;
       }
       TCON_v1(c0) = 0;
       TCON_v2(c0) = 0;
       TCON_v3(c0) = 0;
       break;
#endif
     case MTYPE_C4:
       switch (opr) {
       case OPR_EQ: TCON_v0(c0) = (TCON_R4(c0) == TCON_R4(c1)) && 
				  (TCON_IR4(c0) == TCON_IR4(c1)); break; 
       case OPR_NE: TCON_v0(c0) = (TCON_R4(c0) != TCON_R4(c1)) || 
				  (TCON_IR4(c0) != TCON_IR4(c1)); break; 
       }
       TCON_v1(c0) = 0;
       break;
     case MTYPE_C8:
       switch (opr) {
       case OPR_EQ: TCON_v0(c0) = (TCON_R8(c0) == TCON_R8(c1)) && 
				  (TCON_IR8(c0) == TCON_IR8(c1)); break; 
       case OPR_NE: TCON_v0(c0) = (TCON_R8(c0) != TCON_R8(c1)) || 
				  (TCON_IR8(c0) != TCON_IR8(c1)); break; 
       }
       TCON_v1(c0) = 0;
       break;
#ifdef TARG_NEEDS_QUAD_OPS
     case MTYPE_CQ:
       q0 = R16_To_RQ(TCON_R16(c0));
       q1 = R16_To_RQ(TCON_R16(c1));
       switch (opr) {
       case OPR_EQ: TCON_v0(c0) = __c_q_eq ( q0, q1, &err ); break;
       case OPR_NE: TCON_v0(c0) = __c_q_ne ( q0, q1, &err ); break;
       }
       q0 = R16_To_RQ(TCON_IR16(c0));
       q1 = R16_To_RQ(TCON_IR16(c1));
       switch (opr) {
       case OPR_EQ: TCON_v0(c0) &= __c_q_eq ( q0, q1, &err ); break;
       case OPR_NE: TCON_v0(c0) |= __c_q_ne ( q0, q1, &err ); break;
       }
       TCON_v1(c0) = 0;
       TCON_v2(c0) = 0;
       TCON_v3(c0) = 0;
       break;
#endif
     }
     TCON_ty(c0) = optype;
   } /* OPERATOR_is_compare */

   else {
     switch (op) {

       case OPC_F4PAREN:
       case OPC_F8PAREN:
       case OPC_FQPAREN:
       case OPC_C4PAREN:
       case OPC_C8PAREN:
       case OPC_CQPAREN:
         /* just c0 */
         break;

       case OPC_I8CVTL:
         c0 = Targ_Convert_Length(c0,c1,MTYPE_I8,64,TRUE);
         break;
       case OPC_I4CVTL:
         c0 = Targ_Convert_Length(c0,c1,MTYPE_I4,32,TRUE);
         break;
       case OPC_U8CVTL:
         c0 = Targ_Convert_Length(c0,c1,MTYPE_U8,64,FALSE);
        break;
       case OPC_U4CVTL:
         c0 = Targ_Convert_Length(c0,c1,MTYPE_U4,32,FALSE);
         break;

       /* 
	* For shift cases read ANSI/ISO 9899-1990 standard, Section 6.3.7.
	* The type of the shift is that of the left operand. The right 
	* operand type can actually be larger than the left. The 
	* TCON_I8U8I4U4 macro handles this (any other combination will 
	* have a CVT inserted).
	*
	* Arthur: Bellow is compiler-specific rather than target-specific,
	* so make it so for all targets.
	*
	* For shift amounts that are greater than the width of the bits in
	* the left operand, the behavior on IA64 is defined as: 
        *  1) The shift count is interperted as an unsigned number
        *  2) If the value of the shift count is greater than the word size,
	*     the result is all zero (left shifts and logical right shift) or
	*     a word filled with the sign bit (arithmetic right shift).
	*/
       case OPC_I8SHL:
       case OPC_U8SHL:
         c0 = Targ_Conv(optype, c0);
         if ((TCON_I8U8I4U4(c1) >= 0) && (TCON_I8U8I4U4(c1) <= 63)) {
	   TCON_I8(c0) <<= TCON_I8U8I4U4(c1);
	 } else {
	   TCON_I8(c0) = 0;
	 }
         break;

       case OPC_I4SHL:
       case OPC_U4SHL:
         c0 = Targ_Conv(optype, c0);
         if ((TCON_I8U8I4U4(c1) >= 0) && (TCON_I8U8I4U4(c1) <= 31)) {
	   TCON_I4(c0) <<= TCON_I8U8I4U4(c1);
	 } else {
	   TCON_I4(c0) = 0;
	 }
         break;

       case OPC_I8LSHR:
       case OPC_U8LSHR:
         c0 = Targ_Conv(optype, c0);
         if ((TCON_I8U8I4U4(c1) >= 0) && (TCON_I8U8I4U4(c1) <= 63)) {
	   TCON_U8(c0) >>= TCON_I8U8I4U4(c1);
	 } else {
	   TCON_U8(c0) = 0;
	 }
         break;

       case OPC_I4LSHR:
       case OPC_U4LSHR:
         c0 = Targ_Conv(optype, c0);
         if ((TCON_I8U8I4U4(c1) >= 0) && (TCON_I8U8I4U4(c1) <= 31)) {
	   TCON_U4(c0) >>= TCON_I8U8I4U4(c1);
	 } else {
	   TCON_U4(c0) = 0;
	 }
       break;

       case OPC_I8ASHR:
       case OPC_U8ASHR:
         c0 = Targ_Conv(optype, c0);
         TCON_I8(c0) >>= ((TCON_I8U8I4U4(c1) >= 0) && (TCON_I8U8I4U4(c1) <= 63)) ? TCON_I8U8I4U4(c1) : 63;
         break;

    case OPC_I4ASHR:
    case OPC_U4ASHR:
      c0 = Targ_Conv(optype, c0);
      TCON_I4(c0) >>= ((TCON_I8U8I4U4(c1) >= 0) && (TCON_I8U8I4U4(c1) <= 31)) ? TCON_I8U8I4U4(c1) : 31;
      break;

    case OPC_F4F8CVT:
      Is_True(TCON_ty(c0) == MTYPE_F8, ("Illegal operand to %s", OPCODE_name(op)));
      c0 = Targ_Conv(MTYPE_F4, c0);
      break;
    case OPC_F8F4CVT:
      Is_True(TCON_ty(c0) == MTYPE_F4, ("Illegal operand to %s", OPCODE_name(op)));
      c0 = Targ_Conv(MTYPE_F8, c0);
      break;
#ifdef TARG_NEEDS_QUAD_OPS
    case OPC_FQF4CVT:
      Is_True(TCON_ty(c0) == MTYPE_F4, ("Illegal operand to %s", OPCODE_name(op)));
      c0 = Targ_Conv(MTYPE_FQ, c0);
      break;
    case OPC_F4FQCVT:
      Is_True(TCON_ty(c0) == MTYPE_FQ, ("Illegal operand to %s", OPCODE_name(op)));
      c0 = Targ_Conv(MTYPE_F4, c0);
      break;
    case OPC_FQF8CVT:
      Is_True(TCON_ty(c0) == MTYPE_F8, ("Illegal operand to %s", OPCODE_name(op)));
      c0 = Targ_Conv(MTYPE_FQ, c0);
      break;
    case OPC_F8FQCVT:
      Is_True(TCON_ty(c0) == MTYPE_FQ, ("Illegal operand to %s", OPCODE_name(op)));
      c0 = Targ_Conv(MTYPE_F8, c0);
      break;
#endif
    case OPC_I4F4CVT:
      Is_True(TCON_ty(c0) == MTYPE_F4, ("Illegal operand to %s", OPCODE_name(op)));
      c0 = Targ_Conv(MTYPE_I4, c0);
      break;
    case OPC_I4F8CVT:
      Is_True(TCON_ty(c0) == MTYPE_F8, ("Illegal operand to %s", OPCODE_name(op)));
      c0 = Targ_Conv(MTYPE_I4, c0);
      break;
#ifdef TARG_NEEDS_QUAD_OPS
    case OPC_I4FQCVT:
      Is_True(TCON_ty(c0) == MTYPE_FQ, ("Illegal operand to %s", OPCODE_name(op)));
      c0 = Targ_Conv(MTYPE_I4, c0);
      break;
#endif     
    case OPC_I8F4CVT:
      Is_True(TCON_ty(c0) == MTYPE_F4, ("Illegal operand to %s", OPCODE_name(op)));
      c0 = Targ_Conv(MTYPE_I8, c0);
      break;
    case OPC_I8F8CVT:
      Is_True(TCON_ty(c0) == MTYPE_F8, ("Illegal operand to %s", OPCODE_name(op)));
      c0 = Targ_Conv(MTYPE_I8, c0);
      break;
#ifdef TARG_NEEDS_QUAD_OPS
    case OPC_I8FQCVT:
      Is_True(TCON_ty(c0) == MTYPE_FQ, ("Illegal operand to %s", OPCODE_name(op)));
      c0 = Targ_Conv(MTYPE_I8, c0);
      break;
#endif

    case OPC_U4F4CVT:
      Is_True(TCON_ty(c0) == MTYPE_F4, ("Illegal operand to %s", OPCODE_name(op)));
      c0 = Targ_Conv(MTYPE_U4, c0);
      break;
    case OPC_U4F8CVT:
      Is_True(TCON_ty(c0) == MTYPE_F8, ("Illegal operand to %s", OPCODE_name(op)));
      c0 = Targ_Conv(MTYPE_U4, c0);
      break;
#ifdef TARG_NEEDS_QUAD_OPS
    case OPC_U4FQCVT:
      Is_True(TCON_ty(c0) == MTYPE_FQ, ("Illegal operand to %s", OPCODE_name(op)));
      c0 = Targ_Conv(MTYPE_U4, c0);
      break;
#endif     
    case OPC_U8F4CVT:
      Is_True(TCON_ty(c0) == MTYPE_F4, ("Illegal operand to %s", OPCODE_name(op)));
      c0 = Targ_Conv(MTYPE_U8, c0);
      break;
    case OPC_U8F8CVT:
      Is_True(TCON_ty(c0) == MTYPE_F8, ("Illegal operand to %s", OPCODE_name(op)));
      c0 = Targ_Conv(MTYPE_U8, c0);
      break;
#ifdef TARG_NEEDS_QUAD_OPS
    case OPC_U8FQCVT:
      Is_True(TCON_ty(c0) == MTYPE_FQ, ("Illegal operand to %s", OPCODE_name(op)));
      c0 = Targ_Conv(MTYPE_U8, c0);
      break;
#endif     
    case OPC_I8I4CVT:
    case OPC_I8U4CVT:
    case OPC_I8A4CVT:
    case OPC_I8BCVT:

    case OPC_U8I4CVT:
    case OPC_U8U4CVT:
    case OPC_U8A4CVT:
    case OPC_U8BCVT:

    case OPC_I4I8CVT:
    case OPC_I4U8CVT:
    case OPC_I4A4CVT:
    case OPC_I4A8CVT:
    case OPC_I4BCVT:

    case OPC_U4I8CVT:
    case OPC_U4U8CVT:
    case OPC_U4A4CVT:
    case OPC_U4A8CVT:
    case OPC_U4BCVT:

    case OPC_A4I8CVT:
    case OPC_A4U8CVT:
    case OPC_A4I4CVT:
    case OPC_A4U4CVT:

      c0 = Targ_Conv(OPCODE_desc(op),c0);
      c0 = Targ_Conv(optype, c0);
      break;

    case OPC_I4F8RND:
      if (TCON_R8(c0) >= 0.0) {
	TCON_I4(c0) = (INT32)(TCON_R8(c0) + 0.5);
      } else {
	TCON_I4(c0) = (INT32)(TCON_R8(c0) - 0.5);
      }
      TCON_v1(c0) = 0;
      TCON_ty(c0) = MTYPE_I4;
      break;
    case OPC_I4F4RND:
      if (TCON_R4(c0) >= 0.0) {
	TCON_I4(c0) = (INT32)(TCON_R4(c0) + 0.5);
      } else {
	TCON_I4(c0) = (INT32)(TCON_R4(c0) - 0.5);
      }
      TCON_v1(c0) = 0;
      TCON_ty(c0) = MTYPE_I4;
      break;
#ifdef TARG_NEEDS_QUAD_OPS
    case OPC_I4FQRND:
      if (__c_q_ge(R16_To_RQ(TCON_R16(c0)), 
		   __c_q_flotj(0, &err), 
		   &err))
	TCON_R16(c0) = RQ_To_R16(__c_q_add(R16_To_RQ(TCON_R16(c0)), 
					   __c_q_ext(.5, &err),
					   &err));
      else
	TCON_R16(c0) = RQ_To_R16(__c_q_sub(R16_To_RQ(TCON_R16(c0)), 
					   __c_q_ext(.5, &err),
					   &err));
     
      c0 = Targ_Conv(MTYPE_I4,c0);
      break;
#endif
     
    case OPC_I8F8RND:
      if (TCON_R8(c0) >= 0.0) {
	TCON_I8(c0) = (INT64)(TCON_R8(c0) + 0.5);
      } else {
	TCON_I8(c0) = (INT64)(TCON_R8(c0) - 0.5);
      }
      TCON_ty(c0) = MTYPE_I8;
      break;
    case OPC_I8F4RND:
      if (TCON_R4(c0) >= 0.0) {
	TCON_I8(c0) = (INT64)(TCON_R4(c0) + 0.5);
      } else {
	TCON_I8(c0) = (INT64)(TCON_R4(c0) - 0.5);
      }
      TCON_ty(c0) = MTYPE_I8;
      break;
#ifdef TARG_NEEDS_QUAD_OPS
    case OPC_I8FQRND:
      if (__c_q_ge(R16_To_RQ(TCON_R16(c0)), 
		   __c_q_flotj(0, &err), 
		   &err))
	TCON_R16(c0) = RQ_To_R16(__c_q_add(R16_To_RQ(TCON_R16(c0)), 
					   __c_q_ext(.5, &err),
					   &err));
      else
	TCON_R16(c0) = RQ_To_R16(__c_q_sub(R16_To_RQ(TCON_R16(c0)), 
					   __c_q_ext(.5, &err),
					   &err));
     
      c0 = Targ_Conv(MTYPE_I8,c0);
      break;
#endif
     
      /* Handle truncation like C style CVT ops, i.e. with Targ_Conv() */
    case OPC_I4F4TRUNC:
      Is_True(TCON_ty(c0) == MTYPE_F4, ("Illegal operand to %s", OPCODE_name(op)));
      c0 = Targ_Conv(MTYPE_I4, c0);
      break;
    case OPC_I4F8TRUNC:
      Is_True(TCON_ty(c0) == MTYPE_F8, ("Illegal operand to %s", OPCODE_name(op)));
      c0 = Targ_Conv(MTYPE_I4, c0);
      break;
#ifdef TARG_NEEDS_QUAD_OPS
    case OPC_I4FQTRUNC:
      Is_True(TCON_ty(c0) == MTYPE_FQ, ("Illegal operand to %s", OPCODE_name(op)));
      c0 = Targ_Conv(MTYPE_I4, c0);
      break;
#endif
     
    case OPC_I8F4TRUNC:
      Is_True(TCON_ty(c0) == MTYPE_F4, ("Illegal operand to %s", OPCODE_name(op)));
      c0 = Targ_Conv(MTYPE_I8, c0);
      break;
    case OPC_I8F8TRUNC:
      Is_True(TCON_ty(c0) == MTYPE_F8, ("Illegal operand to %s", OPCODE_name(op)));
      c0 = Targ_Conv(MTYPE_I8, c0);
      break;
#ifdef TARG_NEEDS_QUAD_OPS
    case OPC_I8FQTRUNC:
      Is_True(TCON_ty(c0) == MTYPE_FQ, ("Illegal operand to %s", OPCODE_name(op)));
      c0 = Targ_Conv(MTYPE_I8, c0);
      break;
#endif


    case OPC_I4F4FLOOR:
      t1 = Targ_Conv(MTYPE_I4, c0);
      t2 = Targ_Conv(MTYPE_F4,t1);
      if (TCON_R4(t2) > TCON_R4(c0)) {
	 TCON_I4(t1) -= 1;
      }
      c0 = t1;
      break;
    case OPC_I4F8FLOOR:
      t1 = Targ_Conv(MTYPE_I4, c0);
      t2 = Targ_Conv(MTYPE_F8,t1);
      if (TCON_R8(t2) > TCON_R8(c0)) {
	 TCON_I4(t1) -= 1;
      }
      c0 = t1;
      break;
#ifdef TARG_NEEDS_QUAD_OPS
    case OPC_I4FQFLOOR:
      t1 = Targ_Conv(MTYPE_I4, c0);
      t2 = Targ_Conv(MTYPE_FQ,t1);
      if (__c_q_gt(R16_To_RQ(TCON_R16(t2)),R16_To_RQ(TCON_R16(c0)),&err)) {
	 TCON_I4(t1) -= 1;
      }
      c0 = t1;
      break;
#endif

    case OPC_I8F4FLOOR:
      t1 = Targ_Conv(MTYPE_I8, c0);
      t2 = Targ_Conv(MTYPE_F4,t1);
      if (TCON_R4(t2) > TCON_R4(c0)) {
	 TCON_I8(t1) -= 1;
      }
      c0 = t1;
      break;
    case OPC_I8F8FLOOR:
      t1 = Targ_Conv(MTYPE_I8, c0);
      t2 = Targ_Conv(MTYPE_F8,t1);
      if (TCON_R8(t2) > TCON_R8(c0)) {
	 TCON_I8(t1) -= 1;
      }
      c0 = t1;
      break;
#ifdef TARG_NEEDS_QUAD_OPS
    case OPC_I8FQFLOOR:
      t1 = Targ_Conv(MTYPE_I8, c0);
      t2 = Targ_Conv(MTYPE_FQ,t1);
      if (__c_q_gt(R16_To_RQ(TCON_R16(t2)),R16_To_RQ(TCON_R16(c0)),&err)) {
	 TCON_I8(t1) -= 1;
      }
      c0 = t1;
      break;
#endif
     

    case OPC_I4F4CEIL:
      t1 = Targ_Conv(MTYPE_I4, c0);
      t2 = Targ_Conv(MTYPE_F4,t1);
      if (TCON_R4(t2) < TCON_R4(c0)) {
	 TCON_I4(t1) += 1;
      }
      c0 = t1;
      break;
    case OPC_I4F8CEIL:
      t1 = Targ_Conv(MTYPE_I4, c0);
      t2 = Targ_Conv(MTYPE_F8,t1);
      if (TCON_R8(t2) < TCON_R8(c0)) {
	 TCON_I4(t1) += 1;
      }
      c0 = t1;
      break;
#ifdef TARG_NEEDS_QUAD_OPS
    case OPC_I4FQCEIL:
      t1 = Targ_Conv(MTYPE_I4, c0);
      t2 = Targ_Conv(MTYPE_FQ,t1);
      if (__c_q_lt(R16_To_RQ(TCON_R16(t2)),R16_To_RQ(TCON_R16(c0)),&err)) {
	 TCON_I4(t1) += 1;
      }
      c0 = t1;
      break;
#endif

    case OPC_I8F4CEIL:
      t1 = Targ_Conv(MTYPE_I8, c0);
      t2 = Targ_Conv(MTYPE_F4,t1);
      if (TCON_R4(t2) < TCON_R4(c0)) {
	 TCON_I8(t1) += 1;
      }
      c0 = t1;
      break;
    case OPC_I8F8CEIL:
      t1 = Targ_Conv(MTYPE_I8, c0);
      t2 = Targ_Conv(MTYPE_F8,t1);
      if (TCON_R8(t2) < TCON_R8(c0)) {
	 TCON_I8(t1) += 1;
      }
      c0 = t1;
      break;
#ifdef TARG_NEEDS_QUAD_OPS
    case OPC_I8FQCEIL:
      t1 = Targ_Conv(MTYPE_I8, c0);
      t2 = Targ_Conv(MTYPE_FQ,t1);
      if (__c_q_lt(R16_To_RQ(TCON_R16(t2)),R16_To_RQ(TCON_R16(c0)),&err)) {
	 TCON_I8(t1) += 1;
      }
      c0 = t1;
      break;
#endif
      
    case OPC_U4F8RND:
      TCON_U4(c0) = (UINT32)(TCON_R8(c0) + 0.5);
      TCON_v1(c0) = 0;
      TCON_ty(c0) = MTYPE_U4;
      break;

    case OPC_U4F4RND:
      TCON_U4(c0) = (UINT32)(TCON_R4(c0) + 0.5);
      TCON_v1(c0) = 0;
      TCON_ty(c0) = MTYPE_U4;
      break;

#ifdef TARG_NEEDS_QUAD_OPS
    case OPC_U4FQRND:
      TCON_R16(c0) = RQ_To_R16(__c_q_add(R16_To_RQ(TCON_R16(c0)), 
					 __c_q_ext(.5, &err),
					 &err));
      c0 = Targ_Conv(MTYPE_U4,c0);
      break;
#endif
     
    case OPC_U8F8RND:
      TCON_U8(c0) = (UINT64)(TCON_R8(c0) + 0.5);
      TCON_v1(c0) = 0;
      TCON_ty(c0) = MTYPE_U8;
      break;

    case OPC_U8F4RND:
      TCON_U8(c0) = (UINT64)(TCON_R4(c0) + 0.5);
      TCON_ty(c0) = MTYPE_U8;
      break;

#ifdef TARG_NEEDS_QUAD_OPS
    case OPC_U8FQRND:
      TCON_R16(c0) = RQ_To_R16(__c_q_add(R16_To_RQ(TCON_R16(c0)), 
					   __c_q_ext(.5, &err),
					   &err));
      c0 = Targ_Conv(MTYPE_U8,c0);
      break;
#endif
     
    case OPC_U4F4FLOOR:
    case OPC_U4F4TRUNC:
      Is_True(TCON_ty(c0) == MTYPE_F4, ("Illegal operand to %s", OPCODE_name(op)));
      c0 = Targ_Conv(MTYPE_U4, c0);
      break;

    case OPC_U4F8FLOOR:
    case OPC_U4F8TRUNC:
      Is_True(TCON_ty(c0) == MTYPE_F8, ("Illegal operand to %s", OPCODE_name(op)));
      c0 = Targ_Conv(MTYPE_U4, c0);
      break;

#ifdef TARG_NEEDS_QUAD_OPS
    case OPC_U4FQFLOOR:
    case OPC_U4FQTRUNC:
      Is_True(TCON_ty(c0) == MTYPE_FQ, ("Illegal operand to %s", OPCODE_name(op)));
      c0 = Targ_Conv(MTYPE_U4, c0);
      break;
#endif
     
    case OPC_U8F4FLOOR:
    case OPC_U8F4TRUNC:
      Is_True(TCON_ty(c0) == MTYPE_F4, ("Illegal operand to %s", OPCODE_name(op)));
      c0 = Targ_Conv(MTYPE_U8, c0);
      break;

    case OPC_U8F8FLOOR:
    case OPC_U8F8TRUNC:
      Is_True(TCON_ty(c0) == MTYPE_F8, ("Illegal operand to %s", OPCODE_name(op)));
      c0 = Targ_Conv(MTYPE_U8, c0);
      break;

#ifdef TARG_NEEDS_QUAD_OPS
    case OPC_U8FQFLOOR:
    case OPC_U8FQTRUNC:
      Is_True(TCON_ty(c0) == MTYPE_FQ, ("Illegal operand to %s", OPCODE_name(op)));
      c0 = Targ_Conv(MTYPE_U8, c0);
      break;
#endif

    case OPC_U4F4CEIL:
      t1 = Targ_Conv(MTYPE_U4, c0);
      t2 = Targ_Conv(MTYPE_F4,t1);
      if (TCON_R4(t2) < TCON_R4(c0)) {
	 TCON_U4(t1) += 1;
      }
      c0 = t1;
      break;

    case OPC_U4F8CEIL:
      t1 = Targ_Conv(MTYPE_U4, c0);
      t2 = Targ_Conv(MTYPE_F8,t1);
      if (TCON_R8(t2) < TCON_R8(c0)) {
	 TCON_U4(t1) += 1;
      }
      c0 = t1;
      break;

#ifdef TARG_NEEDS_QUAD_OPS
    case OPC_U4FQCEIL:
      t1 = Targ_Conv(MTYPE_U4, c0);
      t2 = Targ_Conv(MTYPE_FQ,t1);
      if (__c_q_lt(R16_To_RQ(TCON_R16(t2)),R16_To_RQ(TCON_R16(c0)),&err)) {
	 TCON_U4(t1) += 1;
      }
      c0 = t1;
      break;
#endif

    case OPC_U8F4CEIL:
      t1 = Targ_Conv(MTYPE_U8, c0);
      t2 = Targ_Conv(MTYPE_F4,t1);
      if (TCON_R4(t2) < TCON_R4(c0)) {
	 TCON_U8(t1) += 1;
      }
      c0 = t1;
      break;

    case OPC_U8F8CEIL:
      t1 = Targ_Conv(MTYPE_U8, c0);
      t2 = Targ_Conv(MTYPE_F8,t1);
      if (TCON_R8(t2) < TCON_R8(c0)) {
	 TCON_U8(t1) += 1;
      }
      c0 = t1;
      break;

#ifdef TARG_NEEDS_QUAD_OPS
    case OPC_U8FQCEIL:
      t1 = Targ_Conv(MTYPE_U8, c0);
      t2 = Targ_Conv(MTYPE_FQ,t1);
      if (__c_q_lt(R16_To_RQ(TCON_R16(t2)),R16_To_RQ(TCON_R16(c0)),&err)) {
	 TCON_U8(t1) += 1;
      }
      c0 = t1;
      break;
#endif
     
#ifdef TARG_NEEDS_QUAD_OPS
    case OPC_FQI4CVT:
    case OPC_FQU4CVT:
    case OPC_FQI8CVT:
    case OPC_FQU8CVT:
      c0 = Targ_Conv(OPCODE_rtype(op),Targ_Conv(OPCODE_desc(op),c0));
      break;
#endif
    case OPC_F4I4CVT:
    case OPC_F8I4CVT:
    case OPC_F4U4CVT:
    case OPC_F8U4CVT:
    case OPC_F4I8CVT:
    case OPC_F8I8CVT:
    case OPC_F4U8CVT:
    case OPC_F8U8CVT:
      c0 = Targ_Conv(OPCODE_rtype(op),Targ_Conv(OPCODE_desc(op),c0));
      break;
      

      /* The 1 and 2 byte types can safely be typecast like this,
	 since no additional bit motion is necessary. */
    case OPC_I1TAS:
    case OPC_U1TAS:
    case OPC_I2TAS:
    case OPC_U2TAS:
      TCON_ty(c0) = OPCODE_rtype(op);
      break;


      /* If TAS can't be applied to complex types, these are safe
	 since the bits are in the same position in the TCON structure */
    case OPC_I8TAS:
    case OPC_U8TAS:
    case OPC_F8TAS:
    case OPC_FQTAS:
      TCON_ty(c0) = OPCODE_rtype(op);
      break;


    case OPC_U4TAS:
    case OPC_I4TAS:
      TCON_ty(c0) = OPCODE_rtype(op);
      break;

    case OPC_F4TAS:
#if 0
      /* Need to move the bits if the source is not an F4 */
      if (TCON_ty(c0) != MTYPE_F4) {
	 TCON_v1(c0) = TCON_v0(c0);
	 TCON_v0(c0) = 0;
      }
#endif
      TCON_ty(c0) = MTYPE_F4;
      break;


      /* Should these even exist? */
    case OPC_C4TAS:
      TCON_ty(c0) = MTYPE_C4;
      break;
    case OPC_C8TAS:
      TCON_ty(c0) = MTYPE_C8;
      break;
    case OPC_CQTAS:
      TCON_ty(c0) = MTYPE_CQ;
      break;
     
    case OPC_I8ABS:
      c0 = Targ_Conv(MTYPE_I8,c0);
      if (TCON_I8(c0) < 0) TCON_I8(c0)  = -TCON_I8(c0);
      break;
    case OPC_I4ABS:
      c0 = Targ_Conv(MTYPE_I4,c0);
      if (TCON_I4(c0) < 0) TCON_I4(c0)  = -TCON_I4(c0);
      break;
    case OPC_F4ABS:
      if (TCON_R4(c0) < 0) TCON_R4(c0)  = -TCON_R4(c0);
      break;
    case OPC_F8ABS:
      if (TCON_R8(c0) < 0) TCON_R8(c0)  = -TCON_R8(c0);
      break;
#ifdef TARG_NEEDS_QUAD_OPS
    case OPC_FQABS:
      if (__c_q_lt(R16_To_RQ(TCON_R16(c0)),
		   __c_q_flotj(0, &err), 
		   &err))
	TCON_R16(c0) = RQ_To_R16(__c_q_neg(R16_To_RQ(TCON_R16(c0)), 
					   &err));
      break;
#endif
     
    case OPC_I8NEG:
      c0 = Targ_Conv(MTYPE_I8,c0);
      TCON_I8(c0)  = -TCON_I8(c0);
      break;
    case OPC_I4NEG:
      c0 = Targ_Conv(MTYPE_I4,c0);
      TCON_I4(c0)  = -TCON_I4(c0);
      break;
    case OPC_U8NEG:
      c0 = Targ_Conv(MTYPE_U8,c0);
      TCON_U8(c0)  = -TCON_U8(c0);
      break;
    case OPC_U4NEG:
      c0 = Targ_Conv(MTYPE_U4,c0);
      TCON_U4(c0)  = -TCON_U4(c0);
      break;
    case OPC_F4NEG:
      TCON_R4(c0)  = -TCON_R4(c0);
      break;
    case OPC_F8NEG:
      TCON_R8(c0)  = -TCON_R8(c0);
      break;
#ifdef TARG_NEEDS_QUAD_OPS
    case OPC_FQNEG:
      TCON_R16(c0) = RQ_To_R16(__c_q_neg(R16_To_RQ(TCON_R16(c0)), 
					 &err));
      
      break;
#endif
    case OPC_C4NEG:
      TCON_R4(c0)  = -TCON_R4(c0);
      TCON_IR4(c0)  = -TCON_IR4(c0);
      break;
    case OPC_C8NEG:
      TCON_R8(c0)  = -TCON_R8(c0);
      TCON_IR8(c0)  = -TCON_IR8(c0);
      break;
#ifdef TARG_NEEDS_QUAD_OPS
    case OPC_CQNEG:
      TCON_R16(c0) = RQ_To_R16(__c_q_neg(R16_To_RQ(TCON_R16(c0)), 
					 &err));
      TCON_IR16(c0) = RQ_To_R16(__c_q_neg(R16_To_RQ(TCON_IR16(c0)), 
					  &err));
      
      break;
#endif
     
    case OPC_BLAND:
    case OPC_I4LAND:
    case OPC_BCAND:
    case OPC_I4CAND:
      TCON_v0(c0) = TCON_U4(c0) && TCON_U4(c1); 
      TCON_ty(c0) = LOGICAL_MTYPE;
      break;
     
    case OPC_BLIOR:
    case OPC_I4LIOR:
    case OPC_BCIOR:
    case OPC_I4CIOR:
      TCON_v0(c0) = TCON_U4(c0) || TCON_U4(c1);
      TCON_ty(c0) = LOGICAL_MTYPE;
      break;
     
    case OPC_BLNOT:
    case OPC_I4LNOT:
      switch (TCON_ty(c0)) {
        case MTYPE_I4:
	  TCON_v0(c0) = (TCON_I4(c0) == 0);
	  break;
        case MTYPE_U4:
	  TCON_v0(c0) = (TCON_U4(c0) == 0);
	  break;
        case MTYPE_I8:
	  TCON_v0(c0) = (TCON_I8(c0) == 0);
	  break;
        case MTYPE_U8:
	  TCON_v0(c0) = (TCON_U8(c0) == 0);
	  break;
	default:
	  FmtAssert(0,("Targ_WhirlOp, illegal operand type for LNOT"));
	  break;
      }
      TCON_ty(c0) = OPCODE_rtype(op);
      break;
     
    case OPC_I8BXOR:		/* type of result is already correct */
      BOTH_OPNDS(c0,c1,MTYPE_I8);
      TCON_I8(c0) ^= TCON_I8(c1);
      break;
    case OPC_I4BXOR:
      BOTH_OPNDS(c0,c1,MTYPE_I4);
      TCON_I4(c0) ^= TCON_I4(c1);
      break;
    case OPC_U8BXOR:		/* type of result is already correct */
      BOTH_OPNDS(c0,c1,MTYPE_U8);
      TCON_U8(c0) ^= TCON_U8(c1);
      break;
    case OPC_U4BXOR:
      BOTH_OPNDS(c0,c1,MTYPE_U4);
      TCON_U4(c0) ^= TCON_U4(c1);
      break;
     
    case OPC_I8BAND:		/* type of result is already correct */
      BOTH_OPNDS(c0,c1,MTYPE_I8);
      TCON_I8(c0) &= TCON_I8(c1);
      break;
    case OPC_I4BAND:
      BOTH_OPNDS(c0,c1,MTYPE_I4);
      TCON_I4(c0) &= TCON_I4(c1);
      break;
    case OPC_U8BAND:		/* type of result is already correct */
      BOTH_OPNDS(c0,c1,MTYPE_U8);
      TCON_U8(c0) &= TCON_U8(c1);
      break;
    case OPC_U4BAND:
      BOTH_OPNDS(c0,c1,MTYPE_U4);
      TCON_U4(c0) &= TCON_U4(c1);
      break;
     
    case OPC_I8BIOR:		/* type of result is already correct */
      BOTH_OPNDS(c0,c1,MTYPE_I8);
      TCON_I8(c0) |= TCON_I8(c1);
      break;
    case OPC_I4BIOR:
      BOTH_OPNDS(c0,c1,MTYPE_I4);
      TCON_I4(c0) |= TCON_I4(c1);
      break;
    case OPC_U8BIOR:		/* type of result is already correct */
      BOTH_OPNDS(c0,c1,MTYPE_U8);
      TCON_U8(c0) |= TCON_U8(c1);
      break;
    case OPC_U4BIOR:
      BOTH_OPNDS(c0,c1,MTYPE_U4);
      TCON_U4(c0) |= TCON_U4(c1);
      break;

    case OPC_I8BNOR:		/* type of result is already correct */
      BOTH_OPNDS(c0,c1,MTYPE_I8);
      TCON_I8(c0) |= TCON_I8(c1);
      TCON_I8(c0) = ~TCON_I8(c0);
      break;
    case OPC_I4BNOR:
      BOTH_OPNDS(c0,c1,MTYPE_I4);
      TCON_I4(c0) |= TCON_I4(c1);
      TCON_I4(c0) = ~TCON_I4(c0);
      break;
    case OPC_U8BNOR:		/* type of result is already correct */
      BOTH_OPNDS(c0,c1,MTYPE_U8);
      TCON_U8(c0) |= TCON_U8(c1);
      TCON_U8(c0) = ~TCON_U8(c0);
      break;
    case OPC_U4BNOR:
      BOTH_OPNDS(c0,c1,MTYPE_U4);
      TCON_U4(c0) |= TCON_U4(c1);
      TCON_U4(c0) = ~TCON_U4(c0);
      break;
     
    case OPC_I8BNOT:		/* type of result is already correct */
      c0 = Targ_Conv(MTYPE_I8,c0);
      TCON_I8(c0) = ~TCON_I8(c0);
      break;
    case OPC_I4BNOT:
      c0 = Targ_Conv(MTYPE_I4,c0);
      TCON_I4(c0) = ~TCON_I4(c0);
      break;
    case OPC_U8BNOT:		/* type of result is already correct */
      c0 = Targ_Conv(MTYPE_U8,c0);
      TCON_U8(c0) = ~TCON_U8(c0);
      break;
    case OPC_U4BNOT:
      c0 = Targ_Conv(MTYPE_U4,c0);
      TCON_U4(c0) = ~TCON_U4(c0);
      break;
     
      
    case OPC_I8ADD:		/* type of result is already correct */
      BOTH_OPNDS(c0,c1,MTYPE_I8);
      TCON_I8(c0) += TCON_I8(c1);
      break;
    case OPC_I4ADD:
      BOTH_OPNDS(c0,c1,MTYPE_I4);
      TCON_I4(c0) += TCON_I4(c1);
      break;
    case OPC_U8ADD:
      BOTH_OPNDS(c0,c1,MTYPE_U8);
      TCON_U8(c0) += TCON_U8(c1);
      break;
    case OPC_U4ADD:
      BOTH_OPNDS(c0,c1,MTYPE_U4);
      TCON_U4(c0) += TCON_U4(c1);
      break;
    case OPC_A4ADD:
      BOTH_OPNDS(c0,c1,MTYPE_A4);
      TCON_U4(c0) += TCON_U4(c1);
      break;
    case OPC_F4ADD:
      TCON_R4(c0) += TCON_R4(c1);
      break;
    case OPC_F8ADD:
      TCON_R8(c0) += TCON_R8(c1);
      break;
#ifdef TARG_NEEDS_QUAD_OPS
    case OPC_FQADD:
      
      
      TCON_R16(c0) = RQ_To_R16(__c_q_add(R16_To_RQ(TCON_R16(c0)), 
					 R16_To_RQ(TCON_R16(c1)), 
					 &err));
      
      break;
#endif
    case OPC_C4ADD:
      TCON_R4(c0) += TCON_R4(c1);
      TCON_IR4(c0) += TCON_IR4(c1);
      break;
    case OPC_C8ADD:
      TCON_R8(c0) += TCON_R8(c1);
      TCON_IR8(c0) += TCON_IR8(c1);
      break;
#ifdef TARG_NEEDS_QUAD_OPS
    case OPC_CQADD:
      TCON_R16(c0) = RQ_To_R16(__c_q_add(R16_To_RQ(TCON_R16(c0)), 
					 R16_To_RQ(TCON_R16(c1)), 
					 &err));
      TCON_IR16(c0) = RQ_To_R16(__c_q_add(R16_To_RQ(TCON_IR16(c0)), 
					  R16_To_RQ(TCON_IR16(c1)), 
					  &err));
      
      break;
#endif

    case OPC_I8SUB:		/* type of result is already correct */
      BOTH_OPNDS(c0,c1,MTYPE_I8);
      TCON_I8(c0) -= TCON_I8(c1);
      break;
    case OPC_I4SUB:
      BOTH_OPNDS(c0,c1,MTYPE_I4);
      TCON_I4(c0) -= TCON_I4(c1);
      break;
    case OPC_U8SUB:
      BOTH_OPNDS(c0,c1,MTYPE_U8);
      TCON_U8(c0) -= TCON_U8(c1);
      break;
    case OPC_U4SUB:
      BOTH_OPNDS(c0,c1,MTYPE_U4);
      TCON_U4(c0) -= TCON_U4(c1);
      break;
    case OPC_F4SUB:
      TCON_R4(c0) -= TCON_R4(c1);
      break;
    case OPC_F8SUB:
      TCON_R8(c0) -= TCON_R8(c1);
      break;
#ifdef TARG_NEEDS_QUAD_OPS
    case OPC_FQSUB:
      
      
      TCON_R16(c0) = RQ_To_R16(__c_q_sub(R16_To_RQ(TCON_R16(c0)), 
					 R16_To_RQ(TCON_R16(c1)), 
					 &err));
      
      break;
#endif
    case OPC_C4SUB:
      TCON_R4(c0) -= TCON_R4(c1);
      TCON_IR4(c0) -= TCON_IR4(c1);
      break;
    case OPC_C8SUB:
      TCON_R8(c0) -= TCON_R8(c1);
      TCON_IR8(c0) -= TCON_IR8(c1);
      break;
#ifdef TARG_NEEDS_QUAD_OPS
    case OPC_CQSUB:
      
      
      TCON_R16(c0) = RQ_To_R16(__c_q_sub(R16_To_RQ(TCON_R16(c0)), 
					 R16_To_RQ(TCON_R16(c1)), 
					 &err));
      TCON_IR16(c0) = RQ_To_R16(__c_q_sub(R16_To_RQ(TCON_IR16(c0)), 
					  R16_To_RQ(TCON_IR16(c1)), 
					  &err));
      
      break;
#endif

    case OPC_F8SQRT:
      if (TCON_R8(c0) >= 0) {
	TCON_R8(c0) = sqrt(TCON_R8(c0));
      } else {
	*folded = FALSE;
      }
      break;
    case OPC_F4SQRT:
      if (TCON_R4(c0) >= 0) {
	TCON_R4(c0) = sqrtf(TCON_R4(c0));
      } else {
	*folded = FALSE;
      }
      break;
#ifdef TARG_NEEDS_QUAD_OPS
    case OPC_FQSQRT:
      if (__c_q_ge(R16_To_RQ(TCON_R16(c0)), 
		   __c_q_flotj(0, &err), 
		   &err)) {
	TCON_R16(c0) = RQ_To_R16(__c_q_sqrt(R16_To_RQ(TCON_R16(c0)), 
					    &err));
      } else {
	*folded = FALSE;
      }
      break;
#endif
    case OPC_C4SQRT:
    case OPC_C8SQRT:
    case OPC_CQSQRT:
      c0 = complex_sqrt(c0);
      break;

    case OPC_I8MPY:		/* type of result is already correct */
      BOTH_OPNDS(c0,c1,MTYPE_I8);
      TCON_I8(c0) *= TCON_I8(c1);
      break;
    case OPC_I4MPY:
      BOTH_OPNDS(c0,c1,MTYPE_I4);
      TCON_I4(c0) *= TCON_I4(c1);
      break;
    case OPC_U8MPY:
      BOTH_OPNDS(c0,c1,MTYPE_U8);
      TCON_U8(c0) *= TCON_U8(c1);
      break;
    case OPC_U4MPY:
      BOTH_OPNDS(c0,c1,MTYPE_U4);
      TCON_U4(c0) *= TCON_U4(c1);
      break;
    case OPC_F4MPY:
      TCON_R4(c0) *= TCON_R4(c1);
      break;
    case OPC_F8MPY:
      TCON_R8(c0) *= TCON_R8(c1);
      break;

#ifdef TARG_NEEDS_QUAD_OPS
    case OPC_FQMPY:
      
      
      TCON_R16(c0) = RQ_To_R16(__c_q_mul(R16_To_RQ(TCON_R16(c0)), 
					 R16_To_RQ(TCON_R16(c1)), 
					 &err));
      
      break;
#endif

    case OPC_C4MPY:
      TCON_R4(t1) = TCON_R4(c0)*TCON_R4(c1) - TCON_IR4(c0)*TCON_IR4(c1);
      TCON_IR4(c0) = TCON_R4(c0)*TCON_IR4(c1) + TCON_IR4(c0)*TCON_R4(c1);
      TCON_R4(c0) = TCON_R4(t1);
      break;
    case OPC_C8MPY:
      TCON_R8(t1) = TCON_R8(c0)*TCON_R8(c1) - TCON_IR8(c0)*TCON_IR8(c1);
      TCON_IR8(c0) = TCON_R8(c0)*TCON_IR8(c1) + TCON_IR8(c0)*TCON_R8(c1);
      TCON_R8(c0) = TCON_R8(t1);
      break;
     
#ifdef TARG_NEEDS_QUAD_OPS
    case OPC_CQMPY:
      {
	 QUAD a,b,c,d,r1,r2,r3;
	 a = R16_To_RQ(TCON_R16(c0));
	 b = R16_To_RQ(TCON_IR16(c0));
	 c = R16_To_RQ(TCON_R16(c1));
	 d = R16_To_RQ(TCON_IR16(c1));
	 r1 = __c_q_mul(a,c,&err);
	 r2 = __c_q_mul(b,d,&err);
	 r3 = __c_q_sub(r1,r2,&err);
	 r1 = __c_q_mul(a,d,&err);
	 r2 = __c_q_mul(b,c,&err);
	 r1 = __c_q_add(r1,r2,&err);
	 TCON_R16(c0) = RQ_To_R16(r3);  
	 TCON_IR16(c0) = RQ_To_R16(r1);  
      }
      break;
#endif

    case OPC_I8DIV:		/* type of result is already correct */
      BOTH_OPNDS(c0,c1,MTYPE_I8);
      if (TCON_I8(c1) != 0) {
	 /* check for possible integer overflow */
	 if ( TCON_I8(c1) != -1 || TCON_I8(c0) != MIN_INT_I8 )
	   TCON_I8(c0) /= TCON_I8(c1); 
	 else {
/*	    ErrMsg( EC_Ill_Divide );*/
            *folded = FALSE;
	    /* leave c0 as MIN_INT_I8 */
	 }
      }
      else {
	 /* divide by zero; so don't fold */
/*          ErrMsg( EC_Ill_Int_Oflow, TCON_I8(c0), "/", TCON_I8(c1) );*/
	 *folded = FALSE;
      }
      break;
    case OPC_I4DIV:
      BOTH_OPNDS(c0,c1,MTYPE_I4);
      if (TCON_I4(c1) != 0) {
	 /* check for possible integer overflow */
	 if ( TCON_I4(c1) != -1 || TCON_I4(c0) != MIN_INT_I4 )
	   TCON_I4(c0) /= TCON_I4(c1);
	 else {
	   *folded = FALSE;
/*	   ErrMsg( EC_Ill_Int_Oflow, TCON_I4(c0), "/", TCON_I4(c1) );*/
	}
      }
      else {
	 /* divide by zero; so don't fold */
/*	 ErrMsg( EC_Ill_Divide );*/
	 *folded = FALSE;
      }
      break;
    case OPC_U8DIV:
      BOTH_OPNDS(c0,c1,MTYPE_U8);
      if (TCON_U8(c1) != 0)
	TCON_U8(c0) /= TCON_U8(c1);
      else {
	 /* divide by zero; so don't fold */
/*	 ErrMsg( EC_Ill_UDivide );*/
	 *folded = FALSE;
      }
      break;
    case OPC_U4DIV:
      BOTH_OPNDS(c0,c1,MTYPE_U4);
      if (TCON_U4(c1) != 0)
	TCON_U4(c0) /= TCON_U4(c1);
      else {
	 /* divide by zero; so don't fold */
/*	 ErrMsg( EC_Ill_UDivide );*/
	 *folded = FALSE;
      }
      break;
    case OPC_F4DIV:
      if (TCON_R4(c1) == 0.0) {
	 /* divide by zero; so don't fold */
/*	 ErrMsg( EC_Ill_Divide );*/
	 *folded = FALSE;
      }
      TCON_R4(c0) /= TCON_R4(c1);
      break;
    case OPC_F8DIV:
      if (TCON_R8(c1) == 0.0) {
	 /* divide by zero; so don't fold */
/*	 ErrMsg( EC_Ill_Divide );*/
	 *folded = FALSE;
      }
      TCON_R8(c0) /= TCON_R8(c1);
      break;
#ifdef TARG_NEEDS_QUAD_OPS
    case OPC_FQDIV:
      
      
      if (__c_q_eq(R16_To_RQ(TCON_R16(c1)), 
		   __c_q_flotj(0, &err), 
		   &err)) {
	 /* divide by zero; so don't fold */
/*	 ErrMsg( EC_Ill_Divide );*/
	 *folded = FALSE;
	 /* break; */
      }
      TCON_R16(c0) = RQ_To_R16(__c_q_div(R16_To_RQ(TCON_R16(c0)), 
					 R16_To_RQ(TCON_R16(c1)), 
					 &err));
      
      break;
#endif

    case OPC_C8DIV:
    case OPC_CQDIV:
    case OPC_C4DIV:
      c0 = complex_divide(c0,c1);
      break;
      
    case OPC_I8REM:		/* type of result is already correct */
      BOTH_OPNDS(c0,c1,MTYPE_I8);
      if (TCON_I8(c1) != 0) {
	 /* check for possible integer overflow */
	 if ( TCON_I8(c1) != -1 || TCON_I8(c0) != MIN_INT_I8 )
	   TCON_I8(c0) %= TCON_I8(c1);
	 else {
/*	   ErrMsg( EC_Ill_Int_Oflow, TCON_I8(c0), "%", TCON_I8(c1) );*/
	   *folded = FALSE;
	}
      }
      else {
	/* rem by zero; so don't fold */
/*	 ErrMsg( EC_Ill_Modulus );*/
	*folded = FALSE;
      }
      break;
    case OPC_I4REM:
      BOTH_OPNDS(c0,c1,MTYPE_I4);
      if (TCON_I4(c1) != 0) {
	 /* check for possible integer overflow */
	 if ( TCON_I4(c1) != -1 || TCON_I4(c0) != MIN_INT_I4 )
	   TCON_I4(c0) %= TCON_I4(c1);
	 else {
/*	   ErrMsg( EC_Ill_Int_Oflow, TCON_I4(c0), "%", TCON_I4(c1) );*/
	   *folded = FALSE;
	}
      }
      else {
	/* rem by zero; so don't fold */
/*	 ErrMsg( EC_Ill_Modulus );*/
	*folded = FALSE;
      }
      break;
    case OPC_U8REM:
      BOTH_OPNDS(c0,c1,MTYPE_U8);
      if (TCON_U8(c1) != 0)
	TCON_U8(c0) %= TCON_U8(c1); 
      else {
	/* rem by zero; so don't fold */
/*	 ErrMsg( EC_Ill_UModulus );*/
	*folded = FALSE;
      }
      break;
    case OPC_U4REM:
      BOTH_OPNDS(c0,c1,MTYPE_U4);
      if (TCON_U4(c1) != 0)
	TCON_U4(c0) %= TCON_U4(c1);
      else {
	/* rem by zero; so don't fold */
/*	 ErrMsg( EC_Ill_UModulus );*/
	*folded = FALSE;
      }
      break;

    case OPC_I8MOD:		/* type of result is already correct */
      BOTH_OPNDS(c0,c1,MTYPE_I8);
      if (TCON_I8(c1) != 0) {
	 /* check for possible integer overflow */
	 if ( TCON_I8(c1) != -1 || TCON_I8(c0) != MIN_INT_I8 ) {
	    INT64 rem = TCON_I8(c0) % TCON_I8(c1);
	    if ( rem != 0 && ( (TCON_I8(c0)>0) ^ (TCON_I8(c1)>0) ) ) 
	      rem += TCON_I8(c1);
	    TCON_I8(c0) = rem;
	 }
	 else {
/*	   ErrMsg( EC_Ill_Int_Oflow, TCON_I8(c0), "%", TCON_I8(c1) );*/
	   *folded = FALSE;
	}
      } else {
	/* mod by 0; so don't fold */
/*	 ErrMsg( EC_Ill_Modulus );*/
	*folded = FALSE;
      }
      break;
    case OPC_I4MOD:
      BOTH_OPNDS(c0,c1,MTYPE_I4);
      if (TCON_I4(c1) != 0) {
	 /* check for possible integer overflow */
	 if ( TCON_I4(c1) != -1 || TCON_I4(c0) != MIN_INT_I4 ) {
	    INT32 rem = TCON_I4(c0) % TCON_I4(c1);
	    if ( rem != 0 && ( (TCON_I4(c0)>0) ^ (TCON_I4(c1)>0) ) ) 
	      rem += TCON_I4(c1);
	    TCON_I4(c0) = rem;
	 }
	 else {
/*	   ErrMsg( EC_Ill_Int_Oflow, TCON_I4(c0), "%", TCON_I4(c1) );*/
	   *folded = FALSE;
	}
      } else {
	/* mod by 0; so don't fold */
/*	 ErrMsg( EC_Ill_Modulus );*/
	*folded = FALSE;
      }
      break;
    case OPC_U8MOD:
      BOTH_OPNDS(c0,c1,MTYPE_U8);
      if (TCON_U8(c1) != 0)
	TCON_U8(c0) %= TCON_U8(c1); 
      else {
	/* mod by 0; so don't fold */
/*	 ErrMsg( EC_Ill_UModulus );*/
	*folded = FALSE;
      }
      break;
    case OPC_U4MOD:
      BOTH_OPNDS(c0,c1,MTYPE_U4);
      if (TCON_U4(c1) != 0)
	TCON_U4(c0) %= TCON_U4(c1);
      else {
	/* mod by 0; so don't fold */
/*	 ErrMsg( EC_Ill_UModulus );*/
	*folded = FALSE;
      }
      break;


    case OPC_F4RECIP:
      c0 = Targ_WhirlOp(OPC_F4DIV,Host_To_Targ_Float(MTYPE_F4,1.0),c0,folded);
      break;
    case OPC_F8RECIP:
      c0 = Targ_WhirlOp(OPC_F8DIV,Host_To_Targ_Float(MTYPE_F8,1.0),c0,folded);
      break;
    case OPC_FQRECIP:
      c0 = Targ_WhirlOp(OPC_FQDIV,Host_To_Targ_Float(MTYPE_FQ,1.0),c0,folded);
      break;
    case OPC_C4RECIP:
      c0 = Targ_WhirlOp(OPC_C4DIV,Host_To_Targ_Float(MTYPE_C4,1.0),c0,folded);
      break;
    case OPC_C8RECIP:
      c0 = Targ_WhirlOp(OPC_C8DIV,Host_To_Targ_Float(MTYPE_C8,1.0),c0,folded);
      break;
    case OPC_CQRECIP:
      c0 = Targ_WhirlOp(OPC_CQDIV,Host_To_Targ_Float(MTYPE_CQ,1.0),c0,folded);
      break;

    case OPC_F4RSQRT:
      c0 = Targ_WhirlOp(OPC_F4SQRT,c0,c0,folded);
      if (*folded) c0 = Targ_WhirlOp(OPC_F4DIV,Host_To_Targ_Float(MTYPE_F4,1.0),c0,folded);
      break;
    case OPC_F8RSQRT:
      c0 = Targ_WhirlOp(OPC_F8SQRT,c0,c0,folded);
      if (*folded) c0 = Targ_WhirlOp(OPC_F8DIV,Host_To_Targ_Float(MTYPE_F8,1.0),c0,folded);
      break;
    case OPC_FQRSQRT:
      c0 = Targ_WhirlOp(OPC_FQSQRT,c0,c0,folded);
      if (*folded) c0 = Targ_WhirlOp(OPC_FQDIV,Host_To_Targ_Float(MTYPE_FQ,1.0),c0,folded);
      break;
    case OPC_C4RSQRT:
      c0 = Targ_WhirlOp(OPC_C4SQRT,c0,c0,folded);
      if (*folded) c0 = Targ_WhirlOp(OPC_C4DIV,Host_To_Targ_Float(MTYPE_C4,1.0),c0,folded);
      break;
    case OPC_C8RSQRT:
      c0 = Targ_WhirlOp(OPC_C8SQRT,c0,c0,folded);
      if (*folded) c0 = Targ_WhirlOp(OPC_C8DIV,Host_To_Targ_Float(MTYPE_C8,1.0),c0,folded);
      break;
    case OPC_CQRSQRT:
      c0 = Targ_WhirlOp(OPC_CQSQRT,c0,c0,folded);
      if (*folded) c0 = Targ_WhirlOp(OPC_CQDIV,Host_To_Targ_Float(MTYPE_CQ,1.0),c0,folded);
      break;

    case OPC_I8MIN:
      BOTH_OPNDS(c0,c1,MTYPE_I8);
      TCON_I8(c0) = TCON_I8(c0)<=TCON_I8(c1)? TCON_I8(c0) : TCON_I8(c1);
      break;
    case OPC_I4MIN:
      BOTH_OPNDS(c0,c1,MTYPE_I4);
      TCON_I4(c0) = TCON_I4(c0)<=TCON_I4(c1)? TCON_I4(c0) : TCON_I4(c1);
      break;
    case OPC_U8MIN:
      BOTH_OPNDS(c0,c1,MTYPE_U8);
      TCON_U8(c0) = TCON_U8(c0)<=TCON_U8(c1)? TCON_U8(c0) : TCON_U8(c1);
      break;
    case OPC_U4MIN:
      BOTH_OPNDS(c0,c1,MTYPE_U4);
      TCON_U4(c0) = TCON_U4(c0)<=TCON_U4(c1)? TCON_U4(c0) : TCON_U4(c1);
      break;

    case OPC_I8MAX:
      BOTH_OPNDS(c0,c1,MTYPE_I8);
      TCON_I8(c0) = TCON_I8(c0)>=TCON_I8(c1)? TCON_I8(c0) : TCON_I8(c1);
      break;
    case OPC_I4MAX:
      BOTH_OPNDS(c0,c1,MTYPE_I4);
      TCON_I4(c0) = TCON_I4(c0)>=TCON_I4(c1)? TCON_I4(c0) : TCON_I4(c1);
      break;
    case OPC_U8MAX:
      BOTH_OPNDS(c0,c1,MTYPE_U8);
      TCON_U8(c0) = TCON_U8(c0)>=TCON_U8(c1)? TCON_U8(c0) : TCON_U8(c1);
      break;
    case OPC_U4MAX:
      BOTH_OPNDS(c0,c1,MTYPE_U4);
      TCON_U4(c0) = TCON_U4(c0)>=TCON_U4(c1)? TCON_U4(c0) : TCON_U4(c1);
      break;

    case OPC_F4MIN:
      TCON_R4(c0) = TCON_R4(c0) < TCON_R4(c1) ? 
	TCON_R4(c0) : TCON_R4(c1);
      break;
    case OPC_F4MAX:
      TCON_R4(c0) = TCON_R4(c0) < TCON_R4(c1) ? 
	TCON_R4(c1) : TCON_R4(c0);
      break;
    case OPC_F8MIN:
      TCON_R8(c0) = TCON_R8(c0) < TCON_R8(c1) ? 
	TCON_R8(c0) : TCON_R8(c1);
      break;
    case OPC_F8MAX:
      TCON_R8(c0) = TCON_R8(c0) < TCON_R8(c1) ? 
	TCON_R8(c1) : TCON_R8(c0);
      break;

#ifdef TARG_NEEDS_QUAD_OPS
    case OPC_FQMIN:
      
      
      TCON_R16(c0) = __c_q_lt(R16_To_RQ(TCON_R16(c0)), 
			      R16_To_RQ(TCON_R16(c1)), &err) ?
				TCON_R16(c0) : TCON_R16(c1);
      
      break;

    case OPC_FQMAX:
      
      
      TCON_R16(c0) = __c_q_lt(R16_To_RQ(TCON_R16(c0)), 
			      R16_To_RQ(TCON_R16(c1)), &err) ?
				TCON_R16(c1) : TCON_R16(c0);
      
      break;
#endif


    case OPC_C4COMPLEX:
      TCON_IR4(c0) = TCON_R4(c1);
      TCON_ty(c0) = MTYPE_C4;
      break;
    case OPC_C8COMPLEX:
      TCON_IR8(c0) = TCON_R8(c1);
      TCON_ty(c0) = MTYPE_C8;
      break;
    case OPC_CQCOMPLEX:
      TCON_IR16(c0) = TCON_R16(c1);
      TCON_ty(c0) = MTYPE_CQ;
      break;


    case OPC_F4REALPART:
      TCON_ty(c0) = MTYPE_F4;
      break;
    case OPC_F8REALPART:
      TCON_ty(c0) = MTYPE_F8;
      break;
    case OPC_FQREALPART:
      TCON_ty(c0) = MTYPE_FQ;
      break;


    case OPC_F4IMAGPART:
      TCON_R4(c0) = TCON_IR4(c0);
      TCON_ty(c0) = MTYPE_F4;
      break;
    case OPC_F8IMAGPART:
      TCON_R8(c0) = TCON_IR8(c0);
      TCON_ty(c0) = MTYPE_F8;
      break;
    case OPC_FQIMAGPART:
      TCON_R16(c0) = TCON_IR16(c0);
      TCON_ty(c0) = MTYPE_FQ;
      break;

    default:
    //      DevWarn ("Targ_WhirlOp can not handle %s", OPCODE_name(op));
      FmtAssert ( folded, ("Targ_WhirlOp can not handle %s", OPCODE_name(op)) );
      *folded = FALSE;
      break;      
     }
   } /* !OPERATOR_is_compare */

#ifdef DEBUG_FOLD
   printf(" 0x%llx (%s)\n",TCON_I8(c0),Mtype_Name(TCON_ty(c0)));
#endif

   return c0;
} /* Targ_WhirlOp */

/* ====================================================================
 *   Targ_Conv
 *
 *   Convert a TCON's value to a new type.  We currently assume C's
 *   conversion rules.
 * ====================================================================
 */
TCON
Targ_Conv ( 
  TYPE_ID ty_to, 
  TCON c 
)
{
  TYPE_ID ty_from;
  TCON r;
#ifdef TARG_NEEDS_QUAD_OPS
  INT32 err;
#endif

#ifdef TARG_ST
// [TB] Extension support
#define FROM_TO(type_from, type_to) (type_from)*(MTYPE_MAX_LIMIT+1)+(type_to)
#else
#define FROM_TO(type_from, type_to) (type_from)*(MTYPE_LAST+1)+(type_to)
#endif
  r = MTYPE_size_min(ty_to) <= 32 ? Zero_I4_Tcon : Zero_I8_Tcon;

  TCON_v0(r) = 0;
  TCON_v1(r) = 0;
  TCON_v2(r) = 0;
  TCON_v3(r) = 0;
  TCON_iv0(r) = 0;
  TCON_iv1(r) = 0;
  TCON_iv2(r) = 0;
  TCON_iv3(r) = 0;
  ty_from = TCON_ty(c);

  Is_True ( ty_to > MTYPE_UNKNOWN && ty_to <= MTYPE_LAST,	
	    ("Bad dest type in Targ_Conv: %s", Mtype_Name(ty_to)) );
  Is_True ( ty_from > MTYPE_UNKNOWN && ty_from <= MTYPE_LAST,	
	    ("Bad dest type in Targ_Conv: %s", Mtype_Name(ty_from)) );

  if (ty_from == ty_to) 
    return c;

  /* TODO: sign or zero extend when converting from small to large.
	   make sure it matches convert.c
  */
  switch ( FROM_TO(ty_from, ty_to) ) {
#ifdef TARG_NEEDS_QUAD_OPS
    case FROM_TO(MTYPE_C8, MTYPE_CQ):
      TCON_R16(r) = RQ_To_R16(__c_q_extd(TCON_R8(c), &err));
      TCON_IR16(r) = RQ_To_R16(__c_q_extd(TCON_IR8(c), &err));
      break;
    case FROM_TO(MTYPE_C4, MTYPE_CQ):
      TCON_R16(r) = RQ_To_R16(__c_q_ext(TCON_R4(c), &err));
      TCON_IR16(r) = RQ_To_R16(__c_q_ext(TCON_IR4(c), &err));
      break;
    case FROM_TO(MTYPE_FQ, MTYPE_CQ):
      TCON_R16(r) = TCON_R16(c);
      TCON_IR16(r) = RQ_To_R16(__c_q_ext(0.0, &err));
      break;
    case FROM_TO(MTYPE_F8, MTYPE_CQ):
      TCON_R16(r) = RQ_To_R16(__c_q_extd(TCON_R8(c), &err));
      TCON_IR16(r) = RQ_To_R16(__c_q_ext(0.0, &err));
      break;
    case FROM_TO(MTYPE_F4, MTYPE_CQ):
      TCON_R16(r) = RQ_To_R16(__c_q_ext(TCON_R4(c), &err));
      TCON_IR16(r) = RQ_To_R16(__c_q_ext(0.0, &err));
      break;
    case FROM_TO(MTYPE_I8, MTYPE_CQ): 
      TCON_R16(r) = RQ_To_R16(__c_q_flotk(TCON_I8(c), &err));
      TCON_IR16(r) = RQ_To_R16(__c_q_ext(0.0, &err));
      break;
    case FROM_TO(MTYPE_I4, MTYPE_CQ): 
    case FROM_TO(MTYPE_I2, MTYPE_CQ): 
    case FROM_TO(MTYPE_I1, MTYPE_CQ): 
      TCON_R16(r) = RQ_To_R16(__c_q_flotj(TCON_v0(c), &err));
      TCON_IR16(r) = RQ_To_R16(__c_q_ext(0.0, &err));
      break;
    case FROM_TO(MTYPE_U8, MTYPE_CQ):
      TCON_R16(r) = RQ_To_R16(__c_q_flotku(TCON_U8(c), &err));
      TCON_IR16(r) = RQ_To_R16(__c_q_ext(0.0, &err));
      break;
    case FROM_TO(MTYPE_U4, MTYPE_CQ):
    case FROM_TO(MTYPE_U2, MTYPE_CQ):
    case FROM_TO(MTYPE_U1, MTYPE_CQ):
      TCON_R16(r) = RQ_To_R16(__c_q_flotju(TCON_u0(c), &err));
      TCON_IR16(r) = RQ_To_R16(__c_q_ext(0.0, &err));
      break;
#endif /* TARG_NEEDS_QUAD_OPS */

#ifdef TARG_NEEDS_QUAD_OPS
    case FROM_TO(MTYPE_CQ, MTYPE_C8):
      TCON_R8(r) = __c_dble_q(R16_To_RQ(TCON_R16(c)), &err);
      TCON_IR8(r) = __c_dble_q(R16_To_RQ(TCON_IR16(c)), &err);
      break;
#endif /* TARG_NEEDS_QUAD_OPS */
    case FROM_TO(MTYPE_C4, MTYPE_C8):
      TCON_R8(r) = TCON_R4(c);
      TCON_IR8(r) = TCON_IR4(c);
      break;
#ifdef TARG_NEEDS_QUAD_OPS
    case FROM_TO(MTYPE_FQ, MTYPE_C8):
      TCON_R8(r) = __c_dble_q(R16_To_RQ(TCON_R16(c)), &err);
      TCON_IR8(r) = 0.0;
      break;
#endif /* TARG_NEEDS_QUAD_OPS */
    case FROM_TO(MTYPE_F8, MTYPE_C8):
      TCON_R8(r) = TCON_R8(c);
      TCON_IR8(r) = 0.0;
      break;
    case FROM_TO(MTYPE_F4, MTYPE_C8):
      TCON_R8(r) = TCON_R4(c);
      TCON_IR8(r) = 0.0;
      break;
    case FROM_TO(MTYPE_I8, MTYPE_C8):
      TCON_R8(r) = TCON_I8(c);
      TCON_IR8(r) = 0.0;
      break;
    case FROM_TO(MTYPE_I4, MTYPE_C8):
    case FROM_TO(MTYPE_I2, MTYPE_C8):
    case FROM_TO(MTYPE_I1, MTYPE_C8):
      TCON_R8(r) = TCON_v0(c);
      TCON_IR8(r) = 0.0;
      break;
    case FROM_TO(MTYPE_U8, MTYPE_C8):
      TCON_R8(r) = TCON_U8(c);
      TCON_IR8(r) = 0.0;
      break;
    case FROM_TO(MTYPE_U4, MTYPE_C8):
    case FROM_TO(MTYPE_U2, MTYPE_C8):
    case FROM_TO(MTYPE_U1, MTYPE_C8):
      TCON_R8(r) = TCON_u0(c);
      TCON_IR8(r) = 0.0;
      break;

#ifdef TARG_NEEDS_QUAD_OPS
    case FROM_TO(MTYPE_CQ, MTYPE_C4):
      Set_TCON_R4(r, __c_sngl_q(R16_To_RQ(TCON_R16(c)), &err));
      Set_TCON_IR4(r, __c_sngl_q(R16_To_RQ(TCON_IR16(c)), &err));
      break;
#endif /* TARG_NEEDS_QUAD_OPS */
    case FROM_TO(MTYPE_C8, MTYPE_C4):
      TCON_R4(r) = TCON_R8(c);
      TCON_IR4(r) = TCON_IR8(c);
      break;
#ifdef TARG_NEEDS_QUAD_OPS
    case FROM_TO(MTYPE_FQ, MTYPE_C4):
      Set_TCON_R4(r, __c_sngl_q(R16_To_RQ(TCON_R16(c)), &err));
      TCON_IR4(r) = 0.0;
      break;
#endif /* TARG_NEEDS_QUAD_OPS */
    case FROM_TO(MTYPE_F8, MTYPE_C4):
      TCON_R4(r) = TCON_R8(c);
      TCON_IR4(r) = 0.0;
      break;
    case FROM_TO(MTYPE_F4, MTYPE_C4):
      TCON_R4(r) = TCON_R4(c);
      TCON_IR4(r) = 0.0;
      break;
    case FROM_TO(MTYPE_I8, MTYPE_C4):
      TCON_R4(r) = TCON_I8(c);
      TCON_IR4(r) = 0.0;
      break;
    case FROM_TO(MTYPE_I4, MTYPE_C4):
    case FROM_TO(MTYPE_I2, MTYPE_C4):
    case FROM_TO(MTYPE_I1, MTYPE_C4):
      TCON_R4(r) = TCON_v0(c);
      TCON_IR4(r) = 0.0;
      break;
    case FROM_TO(MTYPE_U8, MTYPE_C4):
      TCON_R4(r) = TCON_U8(c);
      TCON_IR4(r) = 0.0;
      break;
    case FROM_TO(MTYPE_U4, MTYPE_C4):
    case FROM_TO(MTYPE_U2, MTYPE_C4):
    case FROM_TO(MTYPE_U1, MTYPE_C4):
      TCON_R4(r) = TCON_u0(c);
      TCON_IR4(r) = 0.0;
      break;

#ifdef TARG_NEEDS_QUAD_OPS
    case FROM_TO(MTYPE_CQ, MTYPE_FQ):
      TCON_R16(r) = TCON_R16(c);
      break;
    case FROM_TO(MTYPE_C8, MTYPE_FQ):
      TCON_R16(r) = RQ_To_R16(__c_q_extd(TCON_R8(c), &err));
      break;
    case FROM_TO(MTYPE_C4, MTYPE_FQ):
      TCON_R16(r) = RQ_To_R16(__c_q_ext(TCON_R4(c), &err));
      break;
    case FROM_TO(MTYPE_F8, MTYPE_FQ):
      TCON_R16(r) = RQ_To_R16(__c_q_extd(TCON_R8(c), &err));
      break;
    case FROM_TO(MTYPE_F4, MTYPE_FQ):
      TCON_R16(r) = RQ_To_R16(__c_q_ext(TCON_R4(c), &err));
      break;
    case FROM_TO(MTYPE_I8, MTYPE_FQ): 
      TCON_R16(r) = RQ_To_R16(__c_q_flotk(TCON_I8(c), &err));
      break;
    case FROM_TO(MTYPE_I4, MTYPE_FQ): 
    case FROM_TO(MTYPE_I2, MTYPE_FQ): 
    case FROM_TO(MTYPE_I1, MTYPE_FQ): 
      TCON_R16(r) = RQ_To_R16(__c_q_flotj(TCON_v0(c), &err));
      break;
    case FROM_TO(MTYPE_U8, MTYPE_FQ):
      TCON_R16(r) = RQ_To_R16(__c_q_flotku(TCON_U8(c), &err));
      break;
    case FROM_TO(MTYPE_U4, MTYPE_FQ):
    case FROM_TO(MTYPE_U2, MTYPE_FQ):
    case FROM_TO(MTYPE_U1, MTYPE_FQ):
      TCON_R16(r) = RQ_To_R16(__c_q_flotju(TCON_u0(c), &err));
      break;
#endif /* TARG_NEEDS_QUAD_OPS */

#ifdef TARG_NEEDS_QUAD_OPS
    case FROM_TO(MTYPE_CQ, MTYPE_F8):
      TCON_R8(r) = __c_dble_q(R16_To_RQ(TCON_R16(c)), &err);
      break;
#endif /* TARG_NEEDS_QUAD_OPS */
    case FROM_TO(MTYPE_C8, MTYPE_F8):
      TCON_R8(r) = TCON_R8(c);
      break;
    case FROM_TO(MTYPE_C4, MTYPE_F8):
      TCON_R8(r) = TCON_R4(c);
      break;
#ifdef TARG_NEEDS_QUAD_OPS
    case FROM_TO(MTYPE_FQ, MTYPE_F8):
      TCON_R8(r) = __c_dble_q(R16_To_RQ(TCON_R16(c)), &err);
      break;
#endif /* TARG_NEEDS_QUAD_OPS */
    case FROM_TO(MTYPE_F4, MTYPE_F8):
      TCON_R8(r) = TCON_R4(c);
      break;
    case FROM_TO(MTYPE_I8, MTYPE_F8):
      TCON_R8(r) = TCON_I8(c);
      break;
    case FROM_TO(MTYPE_I4, MTYPE_F8):
    case FROM_TO(MTYPE_I2, MTYPE_F8):
    case FROM_TO(MTYPE_I1, MTYPE_F8):
      TCON_R8(r) = TCON_v0(c);
      break;
    case FROM_TO(MTYPE_U8, MTYPE_F8):
      TCON_R8(r) = TCON_U8(c);
      break;
    case FROM_TO(MTYPE_U4, MTYPE_F8):
    case FROM_TO(MTYPE_U2, MTYPE_F8):
    case FROM_TO(MTYPE_U1, MTYPE_F8):
      TCON_R8(r) = TCON_u0(c);
      break;

#ifdef TARG_NEEDS_QUAD_OPS
    case FROM_TO(MTYPE_CQ, MTYPE_F4):
      Set_TCON_R4(r, __c_sngl_q(R16_To_RQ(TCON_R16(c)), &err));
      break;
#endif /* TARG_NEEDS_QUAD_OPS */
    case FROM_TO(MTYPE_C8, MTYPE_F4):
      TCON_R4(r) = TCON_R8(c);
      break;
    case FROM_TO(MTYPE_C4, MTYPE_F4):
      TCON_R4(r) = TCON_R4(c);
      break;
#ifdef TARG_NEEDS_QUAD_OPS
    case FROM_TO(MTYPE_FQ, MTYPE_F4):
      Set_TCON_R4(r, __c_sngl_q(R16_To_RQ(TCON_R16(c)), &err));
      break;
#endif /* TARG_NEEDS_QUAD_OPS */

    case FROM_TO(MTYPE_F8, MTYPE_F4):
      TCON_R4(r) = TCON_R8(c);
      break;

    case FROM_TO(MTYPE_I8, MTYPE_F4):
      TCON_R4(r) = TCON_I8(c);
      break;
    case FROM_TO(MTYPE_I4, MTYPE_F4):
    case FROM_TO(MTYPE_I2, MTYPE_F4):
    case FROM_TO(MTYPE_I1, MTYPE_F4):
      TCON_R4(r) = TCON_v0(c);
      break;
    case FROM_TO(MTYPE_U8, MTYPE_F4):
      TCON_R4(r) = TCON_U8(c);
      break;
    case FROM_TO(MTYPE_U4, MTYPE_F4):
    case FROM_TO(MTYPE_U2, MTYPE_F4):
    case FROM_TO(MTYPE_U1, MTYPE_F4):
      TCON_R4(r) = TCON_u0(c);
      break;

#ifdef TARG_NEEDS_QUAD_OPS
    case FROM_TO(MTYPE_CQ, MTYPE_I8):
      TCON_I8(r) = __c_ki_qint(R16_To_RQ(TCON_R16(c)), &err);
      break;
#endif /* TARG_NEEDS_QUAD_OPS */
    case FROM_TO(MTYPE_C8, MTYPE_I8):
      TCON_I8(r) = (INT64)TCON_R8(c);
      break;
    case FROM_TO(MTYPE_C4, MTYPE_I8):
      TCON_I8(r) = (INT64)TCON_R4(c);
      break;
#ifdef TARG_NEEDS_QUAD_OPS
    case FROM_TO(MTYPE_FQ, MTYPE_I8):
      TCON_I8(r) = __c_ki_qint(R16_To_RQ(TCON_R16(c)), &err);
      break;
#endif /* TARG_NEEDS_QUAD_OPS */
    case FROM_TO(MTYPE_F8, MTYPE_I8):
      TCON_I8(r) = (INT64)TCON_R8(c);
      break;
    case FROM_TO(MTYPE_F4, MTYPE_I8):
      TCON_I8(r) = (INT64)TCON_R4(c);
      break;

    case FROM_TO(MTYPE_I4, MTYPE_I8):
    case FROM_TO(MTYPE_I2, MTYPE_I8):
    case FROM_TO(MTYPE_I1, MTYPE_I8):
    case FROM_TO(MTYPE_B, MTYPE_I8):

       TCON_I8(r) = (INT64)TCON_I4(c);
       break;

    case FROM_TO(MTYPE_I4, MTYPE_I5):
    case FROM_TO(MTYPE_I2, MTYPE_I5):
    case FROM_TO(MTYPE_I1, MTYPE_I5):
    case FROM_TO(MTYPE_B, MTYPE_I5):

       TCON_I8(r) = (INT64)TCON_v0(c);
       break;

    case FROM_TO(MTYPE_U8, MTYPE_I8):
    case FROM_TO(MTYPE_U5, MTYPE_I8):

    case FROM_TO(MTYPE_U8, MTYPE_I5):
    case FROM_TO(MTYPE_U5, MTYPE_I5):

       TCON_I8(r) = TCON_U8(c);
       break;

    case FROM_TO(MTYPE_A4, MTYPE_I8):
    case FROM_TO(MTYPE_U4, MTYPE_I8):
    case FROM_TO(MTYPE_U2, MTYPE_I8):
    case FROM_TO(MTYPE_U1, MTYPE_I8):

    case FROM_TO(MTYPE_A4, MTYPE_I5):
    case FROM_TO(MTYPE_U4, MTYPE_I5):
    case FROM_TO(MTYPE_U2, MTYPE_I5):
    case FROM_TO(MTYPE_U1, MTYPE_I5):
       TCON_I8(r) = TCON_u0(c);
       break;

#ifdef TARG_NEEDS_QUAD_OPS
    case FROM_TO(MTYPE_CQ, MTYPE_I4):
    case FROM_TO(MTYPE_CQ, MTYPE_I2):
    case FROM_TO(MTYPE_CQ, MTYPE_I1):
      TCON_v0(r) = __c_ji_qint(R16_To_RQ(TCON_R16(c)), &err);
      break;
#endif
    case FROM_TO(MTYPE_C8, MTYPE_I4):
    case FROM_TO(MTYPE_C8, MTYPE_I2):
    case FROM_TO(MTYPE_C8, MTYPE_I1):
      TCON_v0(r) = (INT32)TCON_R8(c);
      break;
    case FROM_TO(MTYPE_C4, MTYPE_I4):
    case FROM_TO(MTYPE_C4, MTYPE_I2):
    case FROM_TO(MTYPE_C4, MTYPE_I1):
      TCON_v0(r) = (INT32)TCON_R4(c);
      break;
#ifdef TARG_NEEDS_QUAD_OPS
    case FROM_TO(MTYPE_FQ, MTYPE_I4):
    case FROM_TO(MTYPE_FQ, MTYPE_I2):
    case FROM_TO(MTYPE_FQ, MTYPE_I1):
      TCON_v0(r) = __c_ji_qint(R16_To_RQ(TCON_R16(c)), &err);
      break;
#endif
    case FROM_TO(MTYPE_F8, MTYPE_I4):
    case FROM_TO(MTYPE_F8, MTYPE_I2):
    case FROM_TO(MTYPE_F8, MTYPE_I1):
      TCON_v0(r) = (INT32)TCON_R8(c);
      break;
    case FROM_TO(MTYPE_F4, MTYPE_I4):
    case FROM_TO(MTYPE_F4, MTYPE_I2):
    case FROM_TO(MTYPE_F4, MTYPE_I1):
      TCON_v0(r) = (INT32)TCON_R4(c);
      break;
    case FROM_TO(MTYPE_I8, MTYPE_I4):
    case FROM_TO(MTYPE_I8, MTYPE_I2):
    case FROM_TO(MTYPE_I8, MTYPE_I1):
       TCON_v0(r) = TCON_I8(c);
       break;
    case FROM_TO(MTYPE_I5, MTYPE_I4):
    case FROM_TO(MTYPE_I5, MTYPE_I2):
    case FROM_TO(MTYPE_I5, MTYPE_I1):
       TCON_v0(r) = TCON_I8(c);
       break;
    case FROM_TO(MTYPE_I4, MTYPE_I2):
    case FROM_TO(MTYPE_I4, MTYPE_I1):
    case FROM_TO(MTYPE_I2, MTYPE_I4):
    case FROM_TO(MTYPE_I2, MTYPE_I1):
    case FROM_TO(MTYPE_I1, MTYPE_I4):
    case FROM_TO(MTYPE_I1, MTYPE_I2):
    case FROM_TO(MTYPE_B, MTYPE_I4):
       TCON_v0(r) = TCON_v0(c);
       break;
    case FROM_TO(MTYPE_U8, MTYPE_I4):
    case FROM_TO(MTYPE_U8, MTYPE_I2):
    case FROM_TO(MTYPE_U8, MTYPE_I1):
       TCON_v0(r) = TCON_U8(c);
       break;
    case FROM_TO(MTYPE_U5, MTYPE_I4):
    case FROM_TO(MTYPE_U5, MTYPE_I2):
    case FROM_TO(MTYPE_U5, MTYPE_I1):
       TCON_v0(r) = TCON_U8(c);
       break;
    case FROM_TO(MTYPE_A4, MTYPE_I4):
    case FROM_TO(MTYPE_U4, MTYPE_I4):
    case FROM_TO(MTYPE_U4, MTYPE_I2):
    case FROM_TO(MTYPE_U4, MTYPE_I1):
    case FROM_TO(MTYPE_U2, MTYPE_I4):
    case FROM_TO(MTYPE_U2, MTYPE_I2):
    case FROM_TO(MTYPE_U2, MTYPE_I1):
    case FROM_TO(MTYPE_U1, MTYPE_I4):
    case FROM_TO(MTYPE_U1, MTYPE_I2):
    case FROM_TO(MTYPE_U1, MTYPE_I1):
       TCON_v0(r) = TCON_v0(c);
       break;

#ifdef TARG_NEEDS_QUAD_OPS
    case FROM_TO(MTYPE_CQ, MTYPE_U8):
      TCON_U8(r) = __c_ki_quint(R16_To_RQ(TCON_R16(c)), &err);
      break;
#endif
    case FROM_TO(MTYPE_C8, MTYPE_U8):
      TCON_U8(r) = (UINT64)TCON_R8(c);
      break;
    case FROM_TO(MTYPE_C4, MTYPE_U8):
      TCON_U8(r) = (UINT64)TCON_R4(c);
      break;
#ifdef TARG_NEEDS_QUAD_OPS
    case FROM_TO(MTYPE_FQ, MTYPE_U8):
      TCON_U8(r) = __c_ki_quint(R16_To_RQ(TCON_R16(c)), &err);
      break;
#endif
    case FROM_TO(MTYPE_F8, MTYPE_U8):
      TCON_U8(r) = (UINT64)TCON_R8(c);
      break;
    case FROM_TO(MTYPE_F4, MTYPE_U8):
      TCON_U8(r) = (UINT64)TCON_R4(c);
      break;
    case FROM_TO(MTYPE_I8, MTYPE_U8):
    case FROM_TO(MTYPE_I8, MTYPE_U5):
       TCON_U8(r) = TCON_I8(c);
       break;
    case FROM_TO(MTYPE_I5, MTYPE_U8):
    case FROM_TO(MTYPE_I5, MTYPE_U5):
       TCON_U8(r) = TCON_I8(c);
       break;
    case FROM_TO(MTYPE_I4, MTYPE_U8):
    case FROM_TO(MTYPE_I2, MTYPE_U8):
    case FROM_TO(MTYPE_I1, MTYPE_U8):
    case FROM_TO(MTYPE_B, MTYPE_U8):
       TCON_U8(r) = TCON_v0(c);
       break;

    case FROM_TO(MTYPE_A4, MTYPE_U8):
    case FROM_TO(MTYPE_U4, MTYPE_U8):
    case FROM_TO(MTYPE_U2, MTYPE_U8):
    case FROM_TO(MTYPE_U1, MTYPE_U8):
       TCON_U8(r) = TCON_u0(c);
       break;

#ifdef TARG_NEEDS_QUAD_OPS
    case FROM_TO(MTYPE_CQ, MTYPE_U4):
    case FROM_TO(MTYPE_CQ, MTYPE_U2):
    case FROM_TO(MTYPE_CQ, MTYPE_U1):
      TCON_u0(r) = __c_ji_quint(R16_To_RQ(TCON_R16(c)), &err);
      break;
#endif
    case FROM_TO(MTYPE_C8, MTYPE_U4):
    case FROM_TO(MTYPE_C8, MTYPE_U2):
    case FROM_TO(MTYPE_C8, MTYPE_U1):
      TCON_u0(r) = (UINT32)TCON_R8(c);
      break;
    case FROM_TO(MTYPE_C4, MTYPE_U4):
    case FROM_TO(MTYPE_C4, MTYPE_U2):
    case FROM_TO(MTYPE_C4, MTYPE_U1):
      TCON_u0(r) = (UINT32)TCON_R4(c);
      break;
#ifdef TARG_NEEDS_QUAD_OPS
    case FROM_TO(MTYPE_FQ, MTYPE_U4):
    case FROM_TO(MTYPE_FQ, MTYPE_U2):
    case FROM_TO(MTYPE_FQ, MTYPE_U1):
      TCON_u0(r) = __c_ji_quint(R16_To_RQ(TCON_R16(c)), &err);
      break;
#endif
    case FROM_TO(MTYPE_F8, MTYPE_U4):
    case FROM_TO(MTYPE_F8, MTYPE_U2):
    case FROM_TO(MTYPE_F8, MTYPE_U1):
      TCON_u0(r) = (UINT32)TCON_R8(c);
      break;
    case FROM_TO(MTYPE_F4, MTYPE_U4):
    case FROM_TO(MTYPE_F4, MTYPE_U2):
    case FROM_TO(MTYPE_F4, MTYPE_U1):
      TCON_u0(r) = (UINT32)TCON_R4(c);
      break;

    case FROM_TO(MTYPE_I8, MTYPE_U4):
    case FROM_TO(MTYPE_I8, MTYPE_A4):
    case FROM_TO(MTYPE_I8, MTYPE_U2):
    case FROM_TO(MTYPE_I8, MTYPE_U1):
    case FROM_TO(MTYPE_I8, MTYPE_B):

    case FROM_TO(MTYPE_I5, MTYPE_U4):
    case FROM_TO(MTYPE_I5, MTYPE_A4):
    case FROM_TO(MTYPE_I5, MTYPE_U2):
    case FROM_TO(MTYPE_I5, MTYPE_U1):
    case FROM_TO(MTYPE_I5, MTYPE_B):

       TCON_u0(r) = TCON_I8(c);
       break;

    case FROM_TO(MTYPE_I4, MTYPE_A4):
    case FROM_TO(MTYPE_I4, MTYPE_U4):
    case FROM_TO(MTYPE_I4, MTYPE_U2):
    case FROM_TO(MTYPE_I4, MTYPE_U1):
    case FROM_TO(MTYPE_I4, MTYPE_B):
    case FROM_TO(MTYPE_I2, MTYPE_U4):
    case FROM_TO(MTYPE_I2, MTYPE_U2):
    case FROM_TO(MTYPE_I2, MTYPE_U1):
    case FROM_TO(MTYPE_I2, MTYPE_B):
    case FROM_TO(MTYPE_I1, MTYPE_U4):
    case FROM_TO(MTYPE_I1, MTYPE_U2):
    case FROM_TO(MTYPE_I1, MTYPE_U1):
    case FROM_TO(MTYPE_I1, MTYPE_B):
    case FROM_TO(MTYPE_B, MTYPE_U4):
       TCON_v0(r) = TCON_v0(c);
       break;

    case FROM_TO(MTYPE_U8, MTYPE_A4):
    case FROM_TO(MTYPE_U8, MTYPE_U4):
    case FROM_TO(MTYPE_U8, MTYPE_U2):
    case FROM_TO(MTYPE_U8, MTYPE_U1):
    case FROM_TO(MTYPE_U8, MTYPE_B):

    case FROM_TO(MTYPE_U5, MTYPE_A4):
    case FROM_TO(MTYPE_U5, MTYPE_U4):
    case FROM_TO(MTYPE_U5, MTYPE_U2):
    case FROM_TO(MTYPE_U5, MTYPE_U1):
    case FROM_TO(MTYPE_U5, MTYPE_B):

       TCON_u0(r) = TCON_U8(c);
       break;

    case FROM_TO(MTYPE_A4, MTYPE_U4):
    case FROM_TO(MTYPE_U4, MTYPE_A4):
    case FROM_TO(MTYPE_U4, MTYPE_U2):
    case FROM_TO(MTYPE_U4, MTYPE_U1):
    case FROM_TO(MTYPE_U4, MTYPE_B):
    case FROM_TO(MTYPE_U2, MTYPE_U4):
    case FROM_TO(MTYPE_U2, MTYPE_U1):
    case FROM_TO(MTYPE_U2, MTYPE_B):
    case FROM_TO(MTYPE_U1, MTYPE_U4):
    case FROM_TO(MTYPE_U1, MTYPE_U2):
    case FROM_TO(MTYPE_U1, MTYPE_B):
       TCON_v0(r) = TCON_v0(c);
       break;

    default:
      Is_True ( FALSE, ( "Targ-Conv can not convert from %s to %s",
			  Mtype_Name(ty_from), Mtype_Name(ty_to) ) );
  }
  TCON_ty(r) = ty_to;
  return r;
} /* Targ_Conv */

/* ====================================================================
 *   Targ_Reinterpret_Cast
 *
 *   Cast a TCON's value to a new type.  
 * ====================================================================
 */
TCON
Targ_Reinterpret_Cast ( 
  TYPE_ID ty_to, 
  TCON c 
)
{
  TYPE_ID ty_from;
  TCON r;

  FmtAssert(offsetof(TCON, vals.f.fval) == offsetof(TCON, vals.uval.u0),
	    ("Targ_Reinterpret_Cast: Overlay mismatch in TCON between MTYPE_F4 and MTYPE_U4"));

  FmtAssert(offsetof(TCON, vals.f.fval) == offsetof(TCON, vals.ival.v0),
	    ("Targ_Reinterpret_Cast: Overlay mismatch in TCON between MTYPE_F4 and MTYPE_I4"));

  FmtAssert(offsetof(TCON, vals.uval.u0) == offsetof(TCON, vals.ival.v0),
	    ("Targ_Reinterpret_Cast: Overlay mismatch in TCON between MTYPE_I4 and MTYPE_U4"));

#ifdef TARG_ST
// [TB] Extension support
#define FROM_TO(type_from, type_to) (type_from)*(MTYPE_MAX_LIMIT+1)+(type_to)
#else
#define FROM_TO(type_from, type_to) (type_from)*(MTYPE_LAST+1)+(type_to)
#endif
  r = MTYPE_size_min(ty_to) <= 32 ? Zero_I4_Tcon : Zero_I8_Tcon;

  TCON_v0(r) = 0;
  TCON_v1(r) = 0;
  TCON_v2(r) = 0;
  TCON_v3(r) = 0;
  TCON_iv0(r) = 0;
  TCON_iv1(r) = 0;
  TCON_iv2(r) = 0;
  TCON_iv3(r) = 0;
  ty_from = TCON_ty(c);

  Is_True ( ty_to > MTYPE_UNKNOWN && ty_to <= MTYPE_LAST,	
	    ("Bad dest type in Targ_Reinterpret_Cast: %s", Mtype_Name(ty_to)) );
  Is_True ( ty_from > MTYPE_UNKNOWN && ty_from <= MTYPE_LAST,	
	    ("Bad dest type in Targ_Reinterpret_Cast: %s", Mtype_Name(ty_from)) );

  if (ty_from == ty_to) 
    return c;

  switch ( FROM_TO(ty_from, ty_to) ) {
    case FROM_TO(MTYPE_F4, MTYPE_U4):
    case FROM_TO(MTYPE_F4, MTYPE_I4):
    case FROM_TO(MTYPE_F8, MTYPE_U8):
    case FROM_TO(MTYPE_F8, MTYPE_I8):
    case FROM_TO(MTYPE_U4, MTYPE_F4):
    case FROM_TO(MTYPE_I4, MTYPE_F4):
    case FROM_TO(MTYPE_U8, MTYPE_F8):
    case FROM_TO(MTYPE_I8, MTYPE_F8):
    case FROM_TO(MTYPE_I4, MTYPE_U4):
    case FROM_TO(MTYPE_U4, MTYPE_I4):
    case FROM_TO(MTYPE_I8, MTYPE_U8):
    case FROM_TO(MTYPE_U8, MTYPE_I8):
      r = c;
      break;

    default:
      Is_True ( FALSE, ( "Targ_Reinterpret_Cast can not type-cast from %s to %s",
			  Mtype_Name(ty_from), Mtype_Name(ty_to) ) );
  }
  TCON_ty(r) = ty_to;
  return r;
} /* Targ_Reinterpret_Cast */

#ifndef MONGOOSE_BE
/* ====================================================================
 *
 * Targ_Atoll
 *
 * Convert an ASCII number representation to an INT64.  If it fits,
 * put it in i64 and return TRUE.  If not, truncate it into i64 and
 * return FALSE.
 *
 * WARNING:  sgned must be 0 or 1.
 *
 * ====================================================================
 */
static BOOL
Targ_Atoll ( char *str, INT64 *i64, BOOL sgned )
{
  UINT64 m, d;
  UINT64 mhi, mlo;
  INT16 digits;
  BOOL neg = FALSE;
  BOOL valid = TRUE;

  if ( *str == '-' ) {
    neg = TRUE;
    str++;
  }

  m = 0;
  digits = 0;
  while ( *str ) {
    d = *str - '0';
    if ( valid && ++digits >= 19 ) {
      /* Check for overflow before proceeding: */
      mhi = (UINT32) (m >> 32);
      mlo = (UINT32) m;
      mlo = 10 * mlo + d;
      mhi *= 10;
      if ( mlo > UINT32_MAX ) {
	mhi += mlo >> 32;
      }
      if ( ( sgned && mhi > INT32_MAX+(neg) ) ||
	   ( ! sgned && mhi > UINT32_MAX ) )
      {
	valid = FALSE;
      }
    }
    m = 10 * m + d;
    str++;
  }

  *i64 = neg ? -(INT64)m : (INT64)m;
  return valid;
} /* Targ_Atoll */

/* ====================================================================
 *
 * Targ_Atoc
 *
 * Convert an ASCII number representation to a TCON of the given mtype.
 *
 * ====================================================================
 */
TCON
Targ_Atoc ( TYPE_ID ty, char *str )
{
  static TCON c;
  INT64 m;
#if !(defined(FRONT_END_C) || defined(FRONT_END_CPLUSPLUS))
  char *sstr;
#endif /* !(defined(FRONT_END_C) || defined(FRONT_END_CPLUSPLUS)) */
  INT err;

  TCON_ty(c) = ty;
  TCON_v0(c) = 0;
  TCON_v1(c) = 0;
  TCON_v2(c) = 0;
  TCON_v3(c) = 0;

  switch (ty) {

    case MTYPE_I1:
      if ( ! Targ_Atoll ( str, &m, TRUE ) ||
	   m > SCHAR_MAX ||
	   m < SCHAR_MIN )
      {
	ErrMsg ( EC_Large_Const, str );
      }
      TCON_v0(c) = m;
      TCON_v1(c) = 0;
      break;

    case MTYPE_I2:
      if ( ! Targ_Atoll ( str, &m, TRUE ) ||
	   m > INT16_MAX ||
	   m < INT16_MIN )
      {
	ErrMsg ( EC_Large_Const, str );
      }
      TCON_v0(c) = m;
      TCON_v1(c) = 0;
      break;

    case MTYPE_I4:
      if ( ! Targ_Atoll ( str, &m, TRUE ) ||
	   m > INT32_MAX ||
	   m < INT32_MIN )
      {
	ErrMsg ( EC_Large_Const, str );
      }
      TCON_v0(c) = m;
      TCON_v1(c) = 0;
      break;

    case MTYPE_I8:
      if ( ! Targ_Atoll ( str, &m, TRUE ) ) {
	ErrMsg ( EC_Large_Const, str );
      }
      TCON_I8(c) = m;
      break;

    case MTYPE_U1:
      if ( ! Targ_Atoll ( str, &m, FALSE ) ||
	   (UINT64) m > UCHAR_MAX )
      {
	ErrMsg ( EC_Large_Const, str );
      }
      TCON_u0(c) = (UINT8) m;
      TCON_u1(c) = 0;
      break;

    case MTYPE_U2:
      if ( ! Targ_Atoll ( str, &m, FALSE ) ||
	   (UINT64) m > UINT16_MAX )
      {
	ErrMsg ( EC_Large_Const, str );
      }
      TCON_u0(c) = (UINT16) m;
      TCON_u1(c) = 0;
      break;

    case MTYPE_U4:
      if ( ! Targ_Atoll ( str, &m, FALSE ) ||
	   (UINT64) m > UINT32_MAX )
      {
	ErrMsg ( EC_Large_Const, str );
      }
      TCON_u0(c) = (UINT32) m;
      TCON_u1(c) = 0;
      break;

    case MTYPE_U8:
      if ( ! Targ_Atoll ( str, &m, FALSE ) ) {
	ErrMsg ( EC_Large_Const, str );
      }
      TCON_U8(c) = (UINT64) m;
      break;

    case MTYPE_F4:
      /* TODO: do these carefully, taking care of overflow */
/* Do this the right way      * ((float *) &TCON_v0(c)) = atof(str); */
      TCON_R4(c) = atof(str);
      TCON_v0(c) = 0;
      break;

    case MTYPE_F8:
#if !(defined(FRONT_END_C) || defined(FRONT_END_CPLUSPLUS))
      for ( sstr = str; *sstr; sstr++ ) {
	if (*sstr == 'd' || *sstr == 'D') {
	  *sstr='E';
	  break;
	}
      }
#endif /* !(defined(FRONT_END_C) || defined(FRONT_END_CPLUSPLUS)) */
 /* Do this the right way     * ((double *) &TCON_v0(c)) = atof(str); */
      TCON_R8(c) = atof(str);
      break;

    case MTYPE_FQ:
#if !(defined(FRONT_END_C) || defined(FRONT_END_CPLUSPLUS))
      for ( sstr = str; *sstr; sstr++ ) {
	if (*sstr == 'q' || *sstr == 'Q') {
	  *sstr='E';
	  break;
	}
      }
#endif /* !(defined(FRONT_END_C) || defined(FRONT_END_CPLUSPLUS)) */
      /* TODO: add errror checking */
      TCON_R16(c) = RQ_To_R16(__c_a_to_q(str, &err));
      
      break;

    default:
      ErrMsg ( EC_Inv_Mtype, Mtype_Name(ty), "Targ_Atoc" );
  }
  return c;
} /* Targ_Atoc */
#endif /* MONGOOSE_BE */

/* ====================================================================
 *
 * Targ_Hexfptoc
 *
 * Convert an ASCII number representation of a floating-point hexadecimal
 * constant to a TCON of the given mtype.  The input string must be a
 * valid hexadecimal number.  Its length must be 32 digits for MTYPE_FQ,
 * 16 digits for MTYPE_F8, or 8 digits for MTYPE_F4.  These restrictions
 * are assumed correct and are not checked by this function!
 *
 * ====================================================================
 */
TCON
Targ_Hexfptoc(const TYPE_ID ty, const char * const str)
{
  static TCON c;
  char ac[9];

  TCON_ty(c) = ty;
  TCON_u1(c) = 0;
  TCON_u0(c) = 0;
  TCON_u3(c) = 0;
  TCON_u2(c) = 0;
  /* Set the null character in the string argument for strtoul.  All
     strings will have 8 characters. */
  ac[8] = '\0';
  switch (ty) {
    case MTYPE_FQ:
      strncpy(ac, str+24, 8);
      TCON_u2(c) = strtoul(ac, (char **)NULL, 16);
      strncpy(ac, str+16, 8);
      TCON_u3(c) = strtoul(ac, (char **)NULL, 16);
      /* FALL THROUGH */
    case MTYPE_F8:
      strncpy(ac, str+8, 8);
      TCON_u0(c) = strtoul(ac, (char **)NULL, 16);
      /* FALL THROUGH */
    case MTYPE_F4:
      strncpy(ac, str, 8);
      TCON_u1(c) = strtoul(ac, (char **)NULL, 16);
      break;
    default:
      ErrMsg(EC_Inv_Mtype, Mtype_Name(ty), "Targ_Hexfptoc");
  }
  return c;
} /* Targ_Hexfptoc */

/* ====================================================================
 *
 * Targ_Print
 *
 * Convert constant c to a character string. if fmt is given, it is 
 * used as the printf fmt, else a default fmt is provided. The returning
 * ptr points to a static array which gets recycled after TPB_SIZE calls.
 *
 * WARNING: TO BE USED ONLY FOR DIAGNOSTICS.
 *
 * ====================================================================
 */
char *
Targ_Print ( char *fmt, TCON c )
{
   INT slen,i;
   char *bytes;
   INT spos;
   BOOL string_as_bytes;

   struct tpb {
      char buf[1024];
   };
#define TPB_SIZE 8
   static struct tpb tpbuf[8];
   static INT tpidx = 0;
#if !(defined(FRONT_END_C) || defined(FRONT_END_CPLUSPLUS))
   char *re;
#endif /* !(defined(FRONT_END_C) || defined(FRONT_END_CPLUSPLUS)) */
   char *r;

   r = tpbuf[tpidx].buf;
   tpidx = (tpidx + 1) & 7;
   switch (TCON_ty(c)) {
    case MTYPE_STRING:
      /* Since in F90 strings may be used to represent byte strings,
       * we will use two slightly different methods for displaying them.
       */
      slen = TCON_len(c);
      bytes = Index_to_char_array (TCON_cp(c));
      string_as_bytes = FALSE;
      for (i = 0; i < slen-1; i++) {
	 if (bytes[i] == '\0') {
	    string_as_bytes = TRUE;
	    break;
	 }
      }
      if (string_as_bytes) {
	 spos = 0;
	 spos = sprintf(r,"(%d hex bytes) ",slen);
	 for (i=0; i < slen && spos < 1000; i++) {
	    if (i == slen-1) {
	       spos += sprintf(r+spos,"%x",bytes[i]);
	    } else {
	       spos += sprintf(r+spos,"%x, ",bytes[i]);
	    }
	 }
	 if (i < slen) {
	    sprintf(r+spos,"...");
	 }
      } else {
	 sprintf(r, "(%d bytes) \"", slen);
	 char *t = r + strlen (r);
         char *s = r + 1000;
	 for (i = 0; i < slen && t < s; i++) {
		t = Targ_Append_To_Dbuf (t, bytes[i]);
         }
	 if (i < slen) {
	    sprintf(t,"...\"");
	 } else {
	    sprintf(t, "\"");
	 }
      }
      break;

    case MTYPE_B:
    case MTYPE_I1:
    case MTYPE_I2:
    case MTYPE_I4:
    case MTYPE_U1:
    case MTYPE_U2:
    case MTYPE_U4:
      if (fmt == NULL) fmt = "%1d";
      sprintf(r, fmt, TCON_v0(c));
      break;

    case MTYPE_I5:
    case MTYPE_U5:
    case MTYPE_I8:
    case MTYPE_U8:
      if (fmt == NULL) fmt = "%1lld";
      sprintf(r, fmt, TCON_I8(c));
      break;

    case MTYPE_F4:
      if (fmt == NULL) fmt = "%#12.7g";
        sprintf(r, fmt, TCON_R4(c));
#if !(defined(FRONT_END_C) || defined(FRONT_END_CPLUSPLUS))
      if (re = strchr(r, 'e'))
	*re = 'd';
#endif /* !(defined(FRONT_END_C) || defined(FRONT_END_CPLUSPLUS)) */
      break;

    case MTYPE_F8:
      if (fmt == NULL) fmt = "%#21.16g";
        sprintf(r, fmt, TCON_R8(c));
#if !(defined(FRONT_END_C) || defined(FRONT_END_CPLUSPLUS))
      if (re = strchr(r, 'e'))
	*re = 'd';
#endif /* !(defined(FRONT_END_C) || defined(FRONT_END_CPLUSPLUS)) */
      break;

#ifdef TARG_NEEDS_QUAD_OPS
    case MTYPE_FQ:
      {
	 INT dummy_err;
	 QUAD q = R16_To_RQ(TCON_R16(c));
	 /*
	    In Fortran, a user can specify an illegal quad constant
	    using VMS-style bit constants, so we should just give
	    a warning.
	    */
	 if ( q.hi == 0.0 && q.lo != 0.0)
	   ErrMsg( EC_Ill_Quad_Const, TCON_u0(c), TCON_u1(c), TCON_u2(c), TCON_u3(c));
	 __c_q_to_a(r, q, &dummy_err);
#if !(defined(FRONT_END_C) || defined(FRONT_END_CPLUSPLUS))
	 if (re = strchr(r, 'e'))
	   *re = 'd';
#endif /* !(defined(FRONT_END_C) || defined(FRONT_END_CPLUSPLUS)) */
      }
      break;
#endif

    case MTYPE_C4:
      if (fmt == NULL) fmt = "%#12.7g, %#12.7g";
      sprintf(r, fmt, TCON_R4(c), TCON_IR4(c));
#if !(defined(FRONT_END_C) || defined(FRONT_END_CPLUSPLUS))
      if (re = strchr(r, 'e'))
	*re = 'd';
      if (re = strrchr(r, 'e'))
	*re = 'd';
#endif /* !(defined(FRONT_END_C) || defined(FRONT_END_CPLUSPLUS)) */
      break;

    case MTYPE_C8:
      if (fmt == NULL) fmt = "%#21.16g, %#21.16g";
      sprintf(r, fmt, TCON_R8(c), TCON_IR8(c));
#if !(defined(FRONT_END_C) || defined(FRONT_END_CPLUSPLUS))
      if (re = strchr(r, 'e'))
	*re = 'd';
      if (re = strrchr(r, 'e'))
	*re = 'd';
#endif /* !(defined(FRONT_END_C) || defined(FRONT_END_CPLUSPLUS)) */
      break;

#ifdef TARG_NEEDS_QUAD_OPS
    case MTYPE_CQ:
      {
	 INT dummy_err;
	 INT l;
	 QUAD q = R16_To_RQ(TCON_R16(c));
	 /*
	    In Fortran, a user can specify an illegal quad constant
	    using VMS-style bit constants, so we should just give
	    a warning.
	    */
	 if ( q.hi == 0.0 && q.lo != 0.0)
	   ErrMsg( EC_Ill_Quad_Const, TCON_u0(c), TCON_u1(c), TCON_u2(c), TCON_u3(c));
	 __c_q_to_a(r, q, &dummy_err);
#if !(defined(FRONT_END_C) || defined(FRONT_END_CPLUSPLUS))
	 if (re = strchr(r, 'e'))
	   *re = 'd';
#endif /* !(defined(FRONT_END_C) || defined(FRONT_END_CPLUSPLUS)) */

	 l = strlen(r);
	 r [l++] = ',';
	 r [l++] = ' ';

	 q = R16_To_RQ(TCON_IR16(c));
	 /*
	    In Fortran, a user can specify an illegal quad constant
	    using VMS-style bit constants, so we should just give
	    a warning.
	    */
	 if ( q.hi == 0.0 && q.lo != 0.0)
	   ErrMsg( EC_Ill_Quad_Const, TCON_u0(c), TCON_u1(c), TCON_u2(c), TCON_u3(c));
	 __c_q_to_a(&r [l], q, &dummy_err);
#if !(defined(FRONT_END_C) || defined(FRONT_END_CPLUSPLUS))
	 if (re = strchr(r, 'e'))
	   *re = 'd';
#endif /* !(defined(FRONT_END_C) || defined(FRONT_END_CPLUSPLUS)) */
      }
      break;
#endif

    default:
      ErrMsg ( EC_Inv_Mtype, Mtype_Name(TCON_ty(c)), "Targ_Print" );
   }
   return r;
} /* Targ_Print */

/* ====================================================================
 *   Targ_To_Host
 *
 *   Convert target constant represented as a TCON into a host 
 *   constant value
 * ====================================================================
 */
INT64
Targ_To_Host ( 
  TCON c 
)
{
  mINT32 i32;
  mINT64 i64;

  switch (TCON_ty(c)) {
    case MTYPE_B:
      return  (TCON_U4(c)&0x1);
    case MTYPE_I1:
      i32 = TCON_I4(c);
      return ((i32&0x80) ? 0xffffffffffffff00ll : 0ll) | (i32&0xff);
    case MTYPE_I2:
      i32 = TCON_I4(c);
      return ((i32&0x8000) ? 0xffffffffffff0000ll : 0ll) | (i32&0xffff);
    case MTYPE_I4:
      return TCON_I4(c);
    case MTYPE_I5:
      i64 = TCON_I8(c);
      return ((i64&0x800000000ll) ? 
                        0xffffffff00000000ll : 0ll) | (i64&0xfffffffffll);
    case MTYPE_U1:
      return  (TCON_U4(c)&0xff);
    case MTYPE_U2:
      return  (TCON_U4(c)&0xffff);
    case MTYPE_U4:
      return (TCON_U4(c)&0x00000000ffffffffll);
    case MTYPE_U5:
      return (TCON_U8(c)&0x0000000fffffffffll);
    case MTYPE_I8:
      return TCON_I8(c);
    case MTYPE_U8:
      return TCON_U8(c);
    case MTYPE_A4:
      return (TCON_U4(c)&0x00000000ffffffffll);

    default:
      ErrMsg ( EC_Inv_Mtype, Mtype_Name(TCON_ty(c)), "Targ_To_Host" );
  }

  return 0;
} /* Targ_To_Host */

#ifndef MONGOOSE_BE
/* ====================================================================
 *   Targ_To_Signed_Host
 * ====================================================================
 */
INT64
Targ_To_Signed_Host ( TCON c )
{
  mINT32 i32;

  switch (TCON_ty(c)) {
    case MTYPE_B:
      return  (TCON_U4(c)&0x1);
    case MTYPE_I1:
      i32 = TCON_I4(c);
      return ((i32&0x80) ? 0xffffffffffffff00ll : 0ll) | (i32&0xff);
    case MTYPE_I2:
      i32 = TCON_I4(c);
      return ((i32&0x8000) ? 0xffffffffffff0000ll : 0ll) | (i32&0xffff);
    case MTYPE_U1:
      return  (TCON_U4(c)&0xff);
    case MTYPE_U2:
      return  (TCON_U4(c)&0xffff);
    case MTYPE_U4:
      return TCON_I4(c); /* force it to sign extend */
    case MTYPE_I4:
      return TCON_I4(c);
    case MTYPE_I8:
      return TCON_I8(c);
    case MTYPE_U8:
      return TCON_U8(c);

    default:
      ErrMsg ( EC_Inv_Mtype, Mtype_Name(TCON_ty(c)), "Targ_To_Signed_Host" );
  }

  return 0;
} /* Targ_To_Signed_Host */
#endif /* MONGOOSE_BE */

/* ====================================================================
 *   Host_To_Targ
 *
 *   Convert host value 'v' to the TCON of type 'ty'.
 * ====================================================================
 */
TCON
Host_To_Targ (
  TYPE_ID ty, 
  INT64 v
)
{
  static TCON c;

  TCON_clear(c);

  switch (ty) {
    case MTYPE_B:
    case MTYPE_I1:
    case MTYPE_I2:
    case MTYPE_I4:
    case MTYPE_U1:
    case MTYPE_U2:
    case MTYPE_U4:
    case MTYPE_A4:
      TCON_ty(c) = ty;
      TCON_I8(c) = v; /* Don't change the upper bits */
      return c;
    case MTYPE_I5:
    case MTYPE_U5:
      // On this host, IA32, the I5/U5 constants are kept in 64-bit
      // quantities, so do not change the upper bits
    case MTYPE_I8:
    case MTYPE_U8:
      TCON_ty(c) = ty;
      TCON_I8(c) = v;
      return c;

    default:
      FmtAssert(FALSE,("Host_To_Targ: unknown Mtype %s", MTYPE_name(ty)));
      /*      ErrMsg (EC_Inv_Mtype, Mtype_Name(ty), "Host_To_Targ"); */

  }
} /* Host_To_Targ */

/* ====================================================================
 *   Host_To_Targ_Float
 * ====================================================================
 */
TCON
Host_To_Targ_Float ( TYPE_ID ty, double v )
{
  TCON c;

  switch (ty) {
   case MTYPE_C4:
   case MTYPE_F4:
     TCON_clear(c);
     TCON_ty(c) = ty;
     Set_TCON_R4(c,v);
     return c;

   case MTYPE_C8:
   case MTYPE_F8:
     TCON_clear(c);
     TCON_ty(c) = ty;
     TCON_R8(c) = v;
     return c;
     
   case MTYPE_CQ:
   case MTYPE_FQ:
     TCON_clear(c);
     c = Targ_Conv (MTYPE_FQ,
		    Host_To_Targ_Float ( MTYPE_F8, v ) );
     TCON_ty(c) = ty;
     return c;
     
   default:
     ErrMsg ( EC_Inv_Mtype, Mtype_Name(ty), "Host_To_Targ_Float" );
     TCON_clear(c);
     TCON_ty(c) = MTYPE_F4;
     return c;
  }
} /* Host_To_Targ_Float */

/* ====================================================================
 *   Host_To_Targ_Float_4
 *
 *   like Host_To_Targ_Float but avoids conversion from float to 
 *   double.
 * ====================================================================
 */
TCON
Host_To_Targ_Float_4 ( TYPE_ID ty, float v )
{
  TCON c;

  switch (ty) {
   case MTYPE_C4:
   case MTYPE_F4:
     TCON_clear(c);
     TCON_ty(c) = ty;
     Set_TCON_R4(c,v);
     return c;

   case MTYPE_C8:
   case MTYPE_F8:
     TCON_clear(c);
     TCON_ty(c) = ty;
     TCON_R8(c) = v;
     return c;
     
   case MTYPE_CQ:
   case MTYPE_FQ:
     TCON_clear(c);
     c = Targ_Conv (MTYPE_FQ,
		    Host_To_Targ_Float ( MTYPE_F8, v ) );
     TCON_ty(c) = ty;
     return c;
     
   default:
     ErrMsg ( EC_Inv_Mtype, Mtype_Name(ty), "Host_To_Targ_Float" );
     TCON_clear(c);
     TCON_ty(c) = MTYPE_F4;
     return c;
  }
} /* Host_To_Targ_Float_4 */

/* ====================================================================
 *   Host_To_Targ_UV
 * ====================================================================
 */
TCON
Host_To_Targ_UV( TYPE_ID ty)
{
  TCON c;

  TCON_clear(c);
  TCON_ty(c) = ty;

#ifdef TARG_ST
    //TB Add -trapuv_float_value option   
  switch (ty) {
   case MTYPE_F4:
     TCON_v0(c)= DEBUG_Trapuv_Float_Value;
     break;
   case MTYPE_F8:
     TCON_v0(c)= DEBUG_Trapuv_Float_Value;
     TCON_v1(c)= DEBUG_Trapuv_Float_Value;
     break;
   case MTYPE_FQ:
     TCON_v0(c)= DEBUG_Trapuv_Float_Value;
     TCON_v1(c)= DEBUG_Trapuv_Float_Value;
     TCON_v2(c)= DEBUG_Trapuv_Float_Value;
     TCON_v3(c)= DEBUG_Trapuv_Float_Value;
     break;
   case MTYPE_C4:
     TCON_v1(c)= DEBUG_Trapuv_Float_Value;
     TCON_iv1(c)= DEBUG_Trapuv_Float_Value;
     break;
   case MTYPE_C8:
     TCON_v0(c)= DEBUG_Trapuv_Float_Value;
     TCON_v1(c)= DEBUG_Trapuv_Float_Value;
     TCON_iv0(c)= DEBUG_Trapuv_Float_Value;
     TCON_iv1(c)= DEBUG_Trapuv_Float_Value;
     break;
   case MTYPE_CQ:
     TCON_v0(c)= DEBUG_Trapuv_Float_Value;
     TCON_v1(c)= DEBUG_Trapuv_Float_Value;
     TCON_v2(c)= DEBUG_Trapuv_Float_Value;
     TCON_v3(c)= DEBUG_Trapuv_Float_Value;
     TCON_iv0(c)= DEBUG_Trapuv_Float_Value;
     TCON_iv1(c)= DEBUG_Trapuv_Float_Value;
     TCON_iv2(c)= DEBUG_Trapuv_Float_Value;
     TCON_iv3(c)= DEBUG_Trapuv_Float_Value;
     break;
#else  // TARG_ST
  switch (ty) {
   case MTYPE_F4:
     TCON_v0(c)= 0xfffa5a5a;
     break;
   case MTYPE_F8:
     TCON_v0(c)= 0xfffa5a5a;
     TCON_v1(c)= 0xfffa5a5a;
     break;
   case MTYPE_FQ:
     TCON_v0(c)= 0xfffa5a5a;
     TCON_v1(c)= 0xfffa5a5a;
     TCON_v2(c)= 0xfffa5a5a;
     TCON_v3(c)= 0xfffa5a5a;
     break;
   case MTYPE_C4:
     TCON_v1(c)= 0xfffa5a5a;
     TCON_iv1(c)= 0xfffa5a5a;
     break;
   case MTYPE_C8:
     TCON_v0(c)= 0xfffa5a5a;
     TCON_v1(c)= 0xfffa5a5a;
     TCON_iv0(c)= 0xfffa5a5a;
     TCON_iv1(c)= 0xfffa5a5a;
     break;
   case MTYPE_CQ:
     TCON_v0(c)= 0xfffa5a5a;
     TCON_v1(c)= 0xfffa5a5a;
     TCON_v2(c)= 0xfffa5a5a;
     TCON_v3(c)= 0xfffa5a5a;
     TCON_iv0(c)= 0xfffa5a5a;
     TCON_iv1(c)= 0xfffa5a5a;
     TCON_iv2(c)= 0xfffa5a5a;
     TCON_iv3(c)= 0xfffa5a5a;
     break;
#endif //TARG_ST
   default:
     ErrMsg ( EC_Inv_Mtype, Mtype_Name(ty), "Host_To_Targ_UV" );
     TCON_clear(c);
     TCON_ty(c) = MTYPE_F4;
     return c;
  }
  return c;
} /* Host_To_Targ_UV */

/* ====================================================================
 *   Make_Complex
 *
 *   Make complex TCON from two TCONs representing real and imaginary 
 *   parts.
 * ====================================================================
 */
TCON
Make_Complex ( TYPE_ID ctype, TCON real, TCON imag )
{
  TCON c;

  TCON_clear(c);
  TCON_ty(c) = ctype;
  switch (ctype) {
  case MTYPE_C4:
     Set_TCON_R4(c, TCON_R4(real));
     Set_TCON_IR4(c, TCON_R4(imag));
     break;

  case MTYPE_C8:
     Set_TCON_R8(c, TCON_R8(real));
     Set_TCON_IR8(c, TCON_R8(imag));
     break;
     
  case MTYPE_CQ:
     Set_TCON_R16(c, TCON_R16(real));
     Set_TCON_IR16(c, TCON_R16(imag));
     break;
     
  default:
     ErrMsg ( EC_Inv_Mtype, Mtype_Name(ctype), "Make_Complex" );
     break;
  }
  return c;
} /* Make_Complex */

/* ====================================================================
 *   Extract_Complex_Real
 * ====================================================================
 */
TCON
Extract_Complex_Real(TCON complex)
{
  TCON c;
  TCON_clear(c);

  switch (TCON_ty(complex)) {
  case MTYPE_C4:
     TCON_ty(c) = MTYPE_F4;
     Set_TCON_R4(c, TCON_R4(complex));
     return c;

  case MTYPE_C8:
     TCON_ty(c) = MTYPE_F8;
     Set_TCON_R8(c, TCON_R8(complex));
     return c;
     
  case MTYPE_CQ:
     TCON_ty(c) = MTYPE_FQ;
     Set_TCON_R16(c, TCON_R16(complex));
     return c;
     
  default:
     ErrMsg ( EC_Inv_Mtype, Mtype_Name(TCON_ty(complex)), "Extract_Complex_Real" );
     TCON_ty(c) = MTYPE_F4;
     return c;
  }
}

/* ====================================================================
 *   Extract_Complex_Imag
 * ====================================================================
 */
TCON
Extract_Complex_Imag(TCON complex)
{
  TCON c;
  TCON_clear(c);

  switch (TCON_ty(complex)) {
  case MTYPE_C4:
     TCON_ty(c) = MTYPE_F4;
     Set_TCON_R4(c, TCON_IR4(complex));
     return c;

  case MTYPE_C8:
     TCON_ty(c) = MTYPE_F8;
     Set_TCON_R8(c, TCON_IR8(complex));
     return c;
     
  case MTYPE_CQ:
     TCON_ty(c) = MTYPE_FQ;
     Set_TCON_R16(c, TCON_IR16(complex));
     return c;
     
  default:
     ErrMsg ( EC_Inv_Mtype, Mtype_Name(TCON_ty(complex)), "Extract_Complex_Real" );
     TCON_ty(c) = MTYPE_F4;
     return c;
  }
}

/* ====================================================================
 *   Host_To_Targ_Quad
 * ====================================================================
 */
TCON
Host_To_Targ_Quad(QUAD_TYPE v)
{
  static TCON c;

  FmtAssert(FALSE,("Host_To_Targ_Quad: not implemented"));
#if 0
  TCON_ty(c) = MTYPE_FQ;
  TCON_R16(c) = v;
#endif
  return c;
}

/* ====================================================================
 *   Host_To_Targ_Complex_Quad
 * ====================================================================
 */
TCON
Host_To_Targ_Complex_Quad(QUAD_TYPE real, QUAD_TYPE imag)
{
  static TCON c;
 
  FmtAssert(FALSE,("Host_To_Targ_Complex_Quad: not implemented"));
#if 0
  TCON_ty(c) = MTYPE_CQ;
  TCON_R16(c) = real;
  TCON_IR16(c) = imag;
#endif
  return c;
}

double 
Targ_To_Host_Float(TCON fvalue)
{ 
  INT16 ty = TCON_ty(fvalue);

  switch (ty) {
    case MTYPE_F4:
      return (double)TCON_R4(fvalue);

    case MTYPE_F8:
      return TCON_R8(fvalue);

    case MTYPE_FQ:
      return Targ_To_Host_Float(Targ_Conv(MTYPE_F8, fvalue));

    default:
      ErrMsg ( EC_Inv_Mtype, Mtype_Name(ty), "Targ_To_Host_Float" );
      return Targ_To_Host_Float(Targ_Conv(MTYPE_F8, fvalue));
  }
} /* Targ_To_Host_Float */

double 
Targ_To_Host_ComplexReal(TCON fvalue)
{ 
  INT16 ty = TCON_ty(fvalue);

  switch (ty) {
    case MTYPE_C4:
      return (double)TCON_R4(fvalue);

    case MTYPE_C8:
      return TCON_R8(fvalue);

    case MTYPE_CQ:
      return Targ_To_Host_Float(Targ_Conv(MTYPE_CQ, fvalue));

    default:
      ErrMsg ( EC_Inv_Mtype, Mtype_Name(ty), "Targ_To_Host_ComplexReal" );
      return Targ_To_Host_Float(Targ_Conv(MTYPE_C8, fvalue));
  }
}

double 
Targ_To_Host_ComplexImag(TCON fvalue)
{ 
  INT16 ty = TCON_ty(fvalue);

  switch (ty) {
    case MTYPE_C4:
      return (double)TCON_IR4(fvalue);

    case MTYPE_C8:
      return TCON_IR8(fvalue);

    case MTYPE_CQ:
      return Targ_To_Host_Float(Targ_Conv(MTYPE_C8, fvalue));

    default:
      ErrMsg ( EC_Inv_Mtype, Mtype_Name(ty), "Targ_To_Host_ComplexImag" );
      return Targ_To_Host_Float(Targ_Conv(MTYPE_C8, fvalue));
  }
}

QUAD_TYPE 
Targ_To_Host_Quad(TCON fvalue)
{
  INT16 ty = TCON_ty(fvalue);

  switch (ty) {
    case MTYPE_F4:
      return Targ_To_Host_Quad(Targ_Conv(MTYPE_FQ, fvalue));

    case MTYPE_F8:
      return Targ_To_Host_Quad(Targ_Conv(MTYPE_FQ, fvalue));

    case MTYPE_FQ:
      return TCON_R16(fvalue);

    default:
      ErrMsg ( EC_Inv_Mtype, Mtype_Name(ty), "Targ_To_Host_Quad" );
      return Targ_To_Host_Quad(Targ_Conv(MTYPE_FQ, fvalue));
  }
} /* Targ_To_Host_Quad */
   
INT
fp_class_d( double x )
{
  UINT64 ll, exp, mantissa;
  INT32 sign;

  ll = *(UINT64*)&x;
  exp = (ll >> DMANTWIDTH);
  sign = (exp >> DEXPWIDTH);
  exp &= 0x7ff;
  mantissa = (ll & (DSIGNMASK & DEXPMASK));
  if ( exp == 0x7ff ) {
    /* result is an infinity, or a NaN */
    if ( mantissa == 0 )
      return ( (sign == 0) ? FP_POS_INF : FP_NEG_INF );
    else if ( mantissa & ~DQNANBITMASK )
      return ( FP_QNAN );
    else
      return ( FP_SNAN );
  }

  if ( exp == 0 ) {
    if ( mantissa == 0 )
      return ( (sign == 0) ? FP_POS_ZERO : FP_NEG_ZERO );
    else
      return ( (sign == 0) ? FP_POS_DENORM : FP_NEG_DENORM );
  }
  else
    return ( (sign == 0) ? FP_POS_NORM : FP_NEG_NORM );
}

INT
fp_class_f( float x )
{
  UINT32 n, exp, mantissa;
  INT32 sign;

  n = *(UINT32 *)&x;
  exp = (n >> MANTWIDTH);
  sign = (exp >> EXPWIDTH);
  exp &= 0xff;
  mantissa = (n & (SIGNMASK & EXPMASK));

  if ( exp == 0xff ) {
    /* result is an infinity, or a NaN */
  if ( mantissa == 0 )
    return ( (sign == 0) ? FP_POS_INF : FP_NEG_INF );
  else if ( mantissa & ~QNANBITMASK )
    return ( FP_QNAN );
  else
    return ( FP_SNAN );
  }

  if ( exp == 0 ) {
    if ( mantissa == 0 )
      return ( (sign == 0) ? FP_POS_ZERO : FP_NEG_ZERO );
    else
      return ( (sign == 0) ? FP_POS_DENORM : FP_NEG_DENORM );
  }
  else
    return ( (sign == 0) ? FP_POS_NORM : FP_NEG_NORM );
}

INT32 
Targ_fp_class(TCON fvalue)
{
   INT16 ty = TCON_ty(fvalue);

   switch (ty) {
   case MTYPE_F4:
      return fp_class_f(TCON_R4(fvalue));

   case MTYPE_F8:
      return fp_class_d(TCON_R8(fvalue));
     
#ifdef TARG_NEEDS_QUAD_OPS
   case MTYPE_FQ:
      return __c_fp_class_q(R16_To_RQ(TCON_R16(fvalue)));
#endif

   default:
      ErrMsg ( EC_Inv_Mtype, Mtype_Name(ty), "Targ_fp_class" );
      return FP_QNAN;
   }
} /* Targ_fp_class */

TCON
Host_To_Targ_Complex ( TYPE_ID ty, double real, double imag )
{
  TCON c;

  switch (ty) {

   case MTYPE_C4:

     TCON_clear(c);
     TCON_ty(c) = ty;
     TCON_R4(c) = real;
     TCON_IR4(c) = imag;
     return c;

   case MTYPE_C8:

     TCON_clear(c);
     TCON_ty(c) = ty;
     TCON_R8(c) = real;
     TCON_IR8(c) = imag;
     return c;
     
   default:

     ErrMsg ( EC_Inv_Mtype, Mtype_Name(ty), "Host_To_Targ_Complex" );
     TCON_clear(c);
     TCON_ty(c) = MTYPE_C4;
     return c;
  }
} /* Host_To_Targ_Complex */

TCON
Host_To_Targ_Complex_4 ( TYPE_ID ty, float real, float imag )
{
  TCON c;

  switch (ty) {

   case MTYPE_C4:

     TCON_clear(c);
     TCON_ty(c) = ty;
     TCON_R4(c) = real;
     TCON_IR4(c) = imag;
     return c;

   case MTYPE_C8:

     TCON_clear(c);
     TCON_ty(c) = ty;
     TCON_R8(c) = real;
     TCON_IR8(c) = imag;
     return c;
     
   default:

     ErrMsg ( EC_Inv_Mtype, Mtype_Name(ty), "Host_To_Targ_Complex_4" );
     TCON_clear(c);
     TCON_ty(c) = MTYPE_C4;
     return c;
  }
} /* Host_To_Targ_Complex_4 */

TCON
Host_To_Targ_String ( TYPE_ID ty, char *v, UINT32 l )
{
  static TCON c;
  BOOL add_null = FALSE;	/* whether to add a NULL in strtab */
  Is_True(ty==MTYPE_STRING,
          ("Bad type of const to Host_To_Targ_String: %s", Mtype_Name(ty)));
  TCON_clear(c);
  TCON_ty(c) = ty;
  /* for debugging purposes, we ensure that the tcon string is 
   * null-terminated; however, we keep the len as is, without the null. */
  if (l == 0 || v[l-1] != '\0')
	add_null = TRUE;
  /* use StrN in case a wide-string with non-terminating NULLs */
  TCON_cp(c) = Save_StrN(v, (add_null ? l+1 : l));
  if (add_null)
      Index_to_char_array (TCON_cp(c))[l] = '\0';
  TCON_len(c) = l;
  return (c);
}

char *
Targ_String_Address ( TCON c )
{
  Is_True(TCON_ty(c)==MTYPE_STRING,
          ("Bad type of const to Host_To_Targ_String: %s",
            Mtype_Name(TCON_ty(c))));
  return Index_to_char_array (TCON_cp(c));
}
 
mUINT32
Targ_String_Length ( TCON c )
{
  Is_True(TCON_ty(c)==MTYPE_STRING,
          ("Bad type of const to Host_To_Targ_String: %s",
            Mtype_Name(TCON_ty(c))));
  return ( TCON_len(c) );
}

static TCON Targ_Ipower(TCON base, UINT64 exp, BOOL neg_exp, BOOL *folded, TYPE_ID btype)
{
   OPCODE mpy_op,div_op;
   TCON r;
#ifdef TARG_NEEDS_QUAD_OPS
   INT err;
#endif
   
   *folded = TRUE;
   TCON_clear(r);
   TCON_ty(r) = btype;

   switch (btype) {
    case MTYPE_I4:
      mpy_op = OPC_I4MPY;
      div_op = OPCODE_UNKNOWN;
      TCON_I4(r) = 1;
      break;
    case MTYPE_U4:
      mpy_op = OPC_U4MPY;
      div_op = OPCODE_UNKNOWN;
      TCON_U4(r) = 1;
      break;
    case MTYPE_I8:
      mpy_op = OPC_I8MPY;
      div_op = OPCODE_UNKNOWN;
      TCON_I8(r) = 1;
      break;
    case MTYPE_U8:
      mpy_op = OPC_U8MPY;
      div_op = OPCODE_UNKNOWN;
      TCON_U8(r) = 1;
      break;
    case MTYPE_F4:
      mpy_op = OPC_F4MPY;
      div_op = OPC_F4RECIP;
      TCON_R4(r) = 1.0;
      break;
    case MTYPE_F8:
      mpy_op = OPC_F8MPY;
      div_op = OPC_F8RECIP;
      TCON_R8(r) = 1.0;
      break;
#ifdef TARG_NEEDS_QUAD_OPS
    case MTYPE_FQ:
      mpy_op = OPC_FQMPY;
      div_op = OPC_FQRECIP;
      TCON_R16(r) = RQ_To_R16(__c_q_ext(1.0,&err));
      break;
#endif
    case MTYPE_C4:
      mpy_op = OPC_C4MPY;
      div_op = OPC_C4RECIP;
      TCON_R4(r) = 1.0;
      break;
    case MTYPE_C8:
      mpy_op = OPC_C8MPY;
      div_op = OPC_C8RECIP;
      TCON_R8(r) = 1.0;
      break;
#ifdef TARG_NEEDS_QUAD_OPS
    case MTYPE_CQ:
      mpy_op = OPC_CQMPY;
      div_op = OPC_CQRECIP;
      TCON_R16(r) = RQ_To_R16(__c_q_ext(1.0,&err));
      break;
#endif
   }

   /* At this point r contains 1, do the square and multiply loop */
   while (exp != 0) {
      if (exp & 1) {
	 r = Targ_WhirlOp(mpy_op,r,base,NULL);
      }
      base = Targ_WhirlOp(mpy_op,base,base,NULL);
      exp >>= 1;
   }

   /* take reciprocal if negative denominator */
   if (neg_exp) {
      if (div_op) {
	 r = Targ_WhirlOp(div_op,r,r,folded);
      } else {
	 /* Integer reciprocal is easy. If r is not 0, 1, or -1,
	    return 0. If it's 0, give up */
	 switch (TCON_ty(base)) {
	  case MTYPE_I4:
	    if (TCON_I4(r) == 0) {
	       *folded = FALSE;
	    } else if (TCON_I4(r) != 1 && TCON_I4(r) != -1) {
	       TCON_I4(r) = 0;
	    }
	    break;
	  case MTYPE_U4:
	    if (TCON_U4(r) == 0) {
	       *folded = FALSE;
	    } else if (TCON_U4(r) != 1) {
	       TCON_U4(r) = 0;
	    }
	    break;
	  case MTYPE_I8:
	    if (TCON_I8(r) == 0) {
	       *folded = FALSE;
	    } else if (TCON_I8(r) != 1 && TCON_I8(r) != -1) {
	       TCON_I8(r) = 0;
	    }
	    break;
	  case MTYPE_U8:
	    if (TCON_U8(r) == 0) {
	       *folded = FALSE;
	    } else if (TCON_U8(r) != 1) {
	       TCON_U8(r) = 0;
	    }
	    break;
	 }
      }
   }
   return (r);
}

/* General exponentiation routine. If nothing is done, return
   FALSE in folded */

static TCON Targ_Power(TCON base, TCON exp, BOOL *folded, TYPE_ID btype) 
{
   UINT64 int_exp;
   BOOL   neg_exp;
   TCON r;
   

   TCON_clear (r);

   /* Check for integer exponent */
   if (TCON_ty(exp) == MTYPE_U8) {
      return (Targ_Ipower(base, TCON_U8(exp), FALSE, folded, btype));
   } else if (TCON_ty(exp) == MTYPE_U4) {
      return (Targ_Ipower(base, (UINT64) TCON_U4(exp), FALSE, folded, btype));
   } else if (TCON_ty(exp) == MTYPE_I4) {
      int_exp = TCON_I4(exp);
      neg_exp = FALSE;
      if ((INT64) int_exp < 0) {
	 neg_exp = TRUE;
	 int_exp = -((INT64) int_exp);
      }
      return (Targ_Ipower(base,int_exp,neg_exp,folded,btype));

   } else if (TCON_ty(exp) == MTYPE_I8) {
      int_exp = TCON_I8(exp);
      neg_exp = FALSE;

      if (int_exp == 0x8000000000000000LL) {
	 neg_exp = TRUE;
      } else if ((INT64) int_exp < 0) {
	 int_exp = -((INT64)int_exp);
	 neg_exp = TRUE;
      }
      return (Targ_Ipower(base,int_exp,neg_exp,folded,btype));
   }

   /* Check for the same type */
   if (TCON_ty(base) != TCON_ty(exp)) {
      if (folded) *folded = FALSE;
      return (r);
   }

   /* Do the floating-point types */

   switch (TCON_ty(base)) {
    case MTYPE_F4:
      TCON_ty(r) = MTYPE_F4;
      TCON_R4(r) = pow ((double) TCON_R4(base), (double) TCON_R4(exp));
      break;

    case MTYPE_F8:
      TCON_ty(r) = MTYPE_F8;
      TCON_R8(r) = pow (TCON_R8(base),TCON_R8(exp));
      break;

    default:
      /* Not done yet */
      if (folded) *folded = FALSE;
      break;
   }

   return (r);
}

#ifndef MONGOOSE_BE
TCON
Targ_Pow ( TCON a, TCON b )
/* NOTE: No one invokes this function !! */

{
  INT32 va, vb, r;

  va = TCON_v0(a);
  vb = TCON_v0(b);
  if (va == 0)
    r = 0;
  else if (va == 1)
    r = 1;
  else if (va == -1) {
    if (vb < 0) vb = -vb;
    r = (vb % 2) ? -1 : 1;
  } else if (vb == 0)
    r = 1;
  else if (vb < 0) /* va > 1 or va < -1. vb < 0. => 1/va**vb = 0 */
    r = 0;
  else {
    if (vb >= 63) {
/*      ErrMsg ( EC_Exp_Oflow, va, vb );*/
      r = 0;
    } else {
      r = 1;
      while (vb--) r *= va;
    }
  }
  TCON_v0(a) = r;
  return a;
} /* Targ_Pow */
#endif /* MONGOOSE_BE */

/* ====================================================================
 *
 * Targ_Append_To_Dbuf
 *
 * Append the given character to the given string, returning an updated
 * pointer to the string.  If the character is "special", use the C
 * format for it, e.g. "\n" for newline, "\003" for ^C, etc.
 *
 * ====================================================================
 */

char *
Targ_Append_To_Dbuf (char *str, char ch)
{
  char ch1;
  if ( ch >= ' ' && ch <= '~' && ch != '\\' ) {
    *str++ = ch;
  } else {
    ch1 = 0;
    switch ( ch ) {
	case '\n':	ch1 = 'n'; break;
	case '\t':	ch1 = 't'; break;
	case '\b':	ch1 = 'b'; break;
	case '\r':	ch1 = 'r'; break;
	case '\f':	ch1 = 'f'; break;
	case '\v':	ch1 = 'v'; break;
	case '\?':	ch1 = '?'; break;
	case '\\':	ch1 = '\\'; break;
    }
    *str++ = '\\';
    if (ch1) 
        *str++ = ch1;
    else {
        sprintf(str, "%03o", ch & 0xff);
        str += 3;
    }
  }
  return str;
} /* Targ_Append_To_Dbuf */

#define DUMP_STR 1
#ifdef DUMP_STR
#define APPEND_TO_DBUF(a,b) a = Targ_Append_To_Dbuf(a,b)
#else /* DUMP_STR */
#define APPEND_TO_DBUF(a,b) 
#endif /* DUMP_STR */

#ifndef MONGOOSE_BE
/* ====================================================================
 *
 * Targ_Format_String
 *
 * Format the given string as a printable string, by replacing special
 * characters by the C source codes, e.g. "\n" for newline, "\003" for
 * '^C', etc.  The caller passes the string to be formatted (s), its
 * length (slen, zero implying NULL termination), the string to format
 * it into (buf), a maximum length (lmax), a maximum line length (line,
 * may be 0 for no maximum), and a string to insert in the buffer
 * between lines (divider).
 *
 * If lmax may be exceeded, the source string is truncated, and the
 * buffer is terminated with an * ellipsis (...).  If line is non-zero,
 * the formatted string has "divider" inserted between segments of at
 * most line characters; "divider" will typically consist of a
 * terminating double quote, a newline, possibly a tab, and an opening
 * double quote.
 *
 * Inserted dividers do count against the maximum length, but not
 * against the line length.  The returned string is NULL-terminated,
 * and the terminating NULL counts against the maximum length.
 *
 * The return value indicates whether the full string was formatted.
 *
 * NOTE:  No attempt has been made to be efficient.
 *
 * ====================================================================
 */

BOOL
Targ_Format_String (
  char	*s,	/* String to format, */
  INT32	slen,	/* ... of this length, */
  char	*buf,	/* ... into this buffer, */
  INT32	blen,	/* ... with at most this many characters, */
  INT32	line,	/* ... with lines at most this long (0 = no limit), */
  char	*divider )	/* ... divided by this string. */
{
  INT32 len=0;	/* Actual formatted length */
  INT32 llen=0;	/* Actual formatted line length */
  INT32 dlen = divider ? strlen(divider) : 0;
  char cbuf[5];	/* Buffer for a character */
  INT16 clen;	/* Length of character in cbuf */
  INT32 i;

  /* Adjust input parameters: */
  if ( slen == 0 ) slen = strlen(s);
  if ( line == 0 || divider == NULL ) line = blen;

  for ( i=0; i<slen; i++ ) {

    /* Format the character.  Note that the maximum value of clen, for
     * a special character formatted \xxx, is 4:
     */
    clen = Targ_Append_To_Dbuf ( cbuf, s[i] ) - cbuf;
    cbuf[clen] = 0;

    /* Make sure there's room in this line for the string --
     * we are conservative:
     */
    if ( llen > line-clen ) {
      /* If there's no room in buffer, ignore line length limit: */
      if ( len < blen-dlen-clen-1 ) {
	(void) strcpy ( buf, divider );
	buf += dlen;
	len += dlen;
	llen = 0;
      }
    }

    /* Is there room in the buffer for it? */
    if ( len+clen < blen ) {
      /* Yes: */
      (void) strcpy ( buf, cbuf );
      buf += clen;
      len += clen;
      llen += clen;
    } else {
      /* No:  insert ellipsis and quit: */
      if ( blen - len < 4 ) buf -= (len+4)-blen;
      (void) strcpy ( buf, "..." );
      return FALSE;
    }
  }

  /* We managed to format everything.  Note that copying the last
   * character always set terminating NULL.
   */
  return TRUE;
} /* Targ_Format_String */

#ifdef HAS_TCON_TO_STR
/*----------------------------------------------------------------------
 * Put value of v in *buf in a host independent way.  buf will only
 * be accessed as char*.  The byte ordering within buf may be
 * 'implementation defined', but it must give the same results for the
 * same compiler on different hosts.
 *
 * TODO Josie/92: Integrate changes
 *--------------------------------------------------------------------*/
char *
Tcon_To_Str(buf, v)
  TCON v;
  char *buf;
{
  INT *ip = (INT *) buf; /*TODO: or should this be INT64 ? */
  float *fp = (float *) buf;
  double *dp = (double *) buf;
  switch (TCON_ty(v)) {
    case MTYPE_B:
    case MTYPE_I1: /* may want to change for [IL][12] */
    case MTYPE_I2:
    case MTYPE_I4:
    case MTYPE_U1:
    case MTYPE_U2:
    case MTYPE_U4:
      /* if host and target byte order are different, we might want to do 
	 something here */
      *ip = TCON_v0(v);
      break;
    case MTYPE_I8:
    case MTYPE_U8:
      /* if host and target byte order are different, we might want to do 
	 something here */
      *ip = TCON_I8(v);
      break;
    case MTYPE_F4:
      /* user is attempting to look into individual bytes of floating constant 
	 He better know the floating point format. Compilers responsibility
	 is only to do the conversion in a host independent way, i.e., same
	 source must give same object on whatever machine the compiler 
	 is hosted */
      *fp = TCON_R4(v);
      break;
    case MTYPE_F8:
      *dp = TCON_R8(v);
      break;
    default:
      ErrMsg ( EC_Inv_Mtype, Mtype_Name(TCON_ty(v)), "Tcon_To_Str" );
  }
  return buf;
} /* Tcon_To_Str */

/*----------------------------------------------------------------------
 *	reverse of above. Used where constants have been specified in
 *  hex, binary, holleriths etc.
 *--------------------------------------------------------------------*/

/*
 * Throughout this routine, if Same_Byte_Sex, then perform no
 * transformation on data string, otherwise perform byte-sex-change on
 * data.  We always have to copy to a local buffer in case the argument
 * buffer is not aligned properly.
 */
TCON
Str_To_Tcon(TYPE_ID ty, char *buf)
{
  union {
    double d_align;		/* for alignment and size restrictions */
    char   buf[2*sizeof(double)];
  } local_buf;
  char *tbuf = (char *)&local_buf;
  TCON c;
  
  TCON_ty(c) = ty;

  switch (ty) {
    case MTYPE_I1:
    case MTYPE_U1: /* We want to sign-extend here; we'll truncate later */
      TCON_v0(c) = *buf;
      TCON_v1(c) = 0;
      break;
    case MTYPE_I2:
    case MTYPE_U2: /* We want to sign-extend here; we'll truncate later */
      if (Same_Byte_Sex) {
	tbuf[0] = buf[0];
	tbuf[1] = buf[1];
      } else {
	tbuf[0] = buf[1];
	tbuf[1] = buf[0];
      }
      TCON_v0(c) = *((mINT16 *)tbuf);
      TCON_v1(c) = 0;
      break;
    case MTYPE_I4:
    case MTYPE_U4:
      /* if host and target byte order are different, we might want to do 
	 something here */
      if (Same_Byte_Sex) {
	tbuf[0] = buf[0];
	tbuf[1] = buf[1];
	tbuf[2] = buf[2];
	tbuf[3] = buf[3];
      } else {
	tbuf[0] = buf[3];
	tbuf[1] = buf[2];
	tbuf[2] = buf[1];
	tbuf[3] = buf[0];
      }
      TCON_v0(c) = *((INT32 *)tbuf);
      TCON_v1(c) = 0;
      break;
    case MTYPE_I8:
    case MTYPE_U8:
      /* if host and target byte order are different, we might want to do 
	 something here */
      if (Same_Byte_Sex) {
	tbuf[0] = buf[0];
	tbuf[1] = buf[1];
	tbuf[2] = buf[2];
	tbuf[3] = buf[3];
	tbuf[4] = buf[4];
	tbuf[5] = buf[5];
	tbuf[6] = buf[6];
	tbuf[7] = buf[7];
	TCON_I8(c) = *((INT64 *)tbuf);
      } else {
	/* 
	 * We must be very careful about which word gets which set of 4 
	 * bytes here.  To be totally correct, buf[0] must go into the 
	 * byte referenced by (char *)(TCON_I8(c)).  That would be the 
	 * first word here, normally (ie, TCON_v0(c)).  However, since 
	 * the current code is written to use the host's representation 
	 * as the internal representation, there is a kludge in 
	 * Emit_Const above which always swaps words when dumping to the 
	 * output file.  As such, we swap the words here, so that the 
	 * later swap will do the right thing.  Blech, kludge upon kludge.
	 */
	tbuf[0] = buf[7];
	tbuf[1] = buf[6];
	tbuf[2] = buf[5];
	tbuf[3] = buf[4];
	TCON_v0(c) = *((INT *)tbuf);
	tbuf[0] = buf[3];
	tbuf[1] = buf[2];
	tbuf[2] = buf[1];
	tbuf[3] = buf[0];
	TCON_v1(c) = *((INT *)tbuf);
      }
      break;
    case MTYPE_F4:
      /* User is trying to give floating constant in hollerith, etc.
	 He needs to know the floating point format.  On the other hand,
	 he may be doing hollerith/character init of a variable which will
	 be printed out as character data, in which case the user doesn't
	 care what the format is.  Either way, the compiler's responsibility
	 is only to do the conversion in a host independent way, i.e., same
	 source must give same object on whatever machine the compiler 
	 is hosted.
       */
      if (Same_Byte_Sex) {
	tbuf[0] = buf[0];
	tbuf[1] = buf[1];
	tbuf[2] = buf[2];
	tbuf[3] = buf[3];
      } else {
	tbuf[0] = buf[3];
	tbuf[1] = buf[2];
	tbuf[2] = buf[1];
	tbuf[3] = buf[0];
      }
      Set_TCON_R4 ( c, *((float *)tbuf) );
      TCON_v1(c) = 0;
      break;

    case MTYPE_F8:
      if (Same_Byte_Sex) {
	tbuf[0] = buf[0];
	tbuf[1] = buf[1];
	tbuf[2] = buf[2];
	tbuf[3] = buf[3];
	tbuf[4] = buf[4];
	tbuf[5] = buf[5];
	tbuf[6] = buf[6];
	tbuf[7] = buf[7];
	TCON_R8(c) = *((double *)tbuf);
      } else {
	/* 
	 * We must be very careful about which word gets which set of 4 
	 * bytes here.  To be totally correct, buf[0] must go into the 
	 * byte referenced by (char *)(TCON_R8(c)).  That would be the 
	 * first word here, normally (ie, TCON_v0(c)).  However, since 
	 * the current code is written to use the host's representation 
	 * as the internal representation, there is a kludge in 
	 * Emit_Const above which always swaps words when dumping to the 
	 * output file.  As such, we swap the words here, so that the 
	 * later swap will do the right thing.  Blech, kludge upon kludge.
	 */
	tbuf[0] = buf[7];
	tbuf[1] = buf[6];
	tbuf[2] = buf[5];
	tbuf[3] = buf[4];
	TCON_v0(c) = *((INT *)tbuf);
	tbuf[0] = buf[3];
	tbuf[1] = buf[2];
	tbuf[2] = buf[1];
	tbuf[3] = buf[0];
	TCON_v1(c) = *((INT *)tbuf);
      }
      break;

    default:
      ErrMsg ( EC_Inv_Mtype, Mtype_Name(ty), "Str_To_Tcon" );
  }
  return c;
} /* Str_To_Tcon */

#endif /* HAS_TCON_TO_STR */

#endif /* MONGOOSE_BE */

/* ====================================================================
 *
 * Targ_Is_Integral
 *
 * Determine whether a TCON represents an integral value, and if so
 * return its value.
 *
 * ====================================================================
 */

BOOL
Targ_Is_Integral ( TCON tc, INT64 *iv )
{
  switch (TCON_ty(tc)) {
    case MTYPE_B:
    case MTYPE_I1:
    case MTYPE_I2:
    case MTYPE_I4:
      *iv = TCON_v0(tc);
      return TRUE;

    case MTYPE_U1:
    case MTYPE_U2:
    case MTYPE_U4:
      *iv = TCON_u0(tc);
      return TRUE;

    case MTYPE_I5:
    case MTYPE_U5:
      // I5/U5 are represented as 64-bit quantities
    case MTYPE_I8:
    case MTYPE_U8:
      *iv = TCON_I8(tc);
      return TRUE;

    case MTYPE_F4:
      {
	INT32 k = (INT32)TCON_R4(tc);
	float s = k;
	if (s == TCON_R4(tc)) {
	  *iv = k;
	  return TRUE;
	}
      }
      return FALSE;

    case MTYPE_F8:
      {
	INT64 k = (INT64)TCON_R8(tc);
	double d = k;
	if (d == TCON_R8(tc)) {
	  *iv = k;
	  return TRUE;
	}
      }
      return FALSE;

#ifdef TARG_NEEDS_QUAD_OPS
    case MTYPE_FQ:
      {
	QUAD  q;
	INT32 err;
	INT32 k = __c_ji_qint(R16_To_RQ(TCON_R16(tc)), &err);
	if (err) return FALSE;
	q = __c_q_flotj(k, &err);
	if (err) return FALSE;
	if (__c_q_eq(q, R16_To_RQ(TCON_R16(tc)), &err)) {
          if (err) return FALSE;
	  *iv = k;
	  return TRUE;
	}
      }
      return FALSE;
#endif

    /* TODO : fix for mongoose */
    case MTYPE_C4:
    case MTYPE_C8:
    case MTYPE_CQ:
      return FALSE;

    case MTYPE_STR:
      return FALSE;

    default:
      ErrMsg ( EC_Inv_Mtype, Mtype_Name(TCON_ty(tc)),
	       "Targ_Is_Integral" );
  }
  return FALSE;
} /* Targ_Is_Integral */

/*----------------------------------------------------------------------------
 * return TRUE if the target representation of this TCON is has all zeros
 *--------------------------------------------------------------------------*/

BOOL Targ_Is_Zero ( TCON t )
{
  switch (TCON_ty(t)) {
    case MTYPE_B:
    case MTYPE_I1:
    case MTYPE_I2:
    case MTYPE_I4:
    case MTYPE_U1:
    case MTYPE_U2:
    case MTYPE_U4:
    case MTYPE_A4:
       return TCON_v0(t) == 0;
    case MTYPE_F4:
      /* make sure is not -0.0 (sign-bit set);
       * -0.0 will == 0.0, so add check for sign bit. */
      return (TCON_R4(t) == 0.0 && TCON_v0(t) == 0);
      //       return TCON_v1(t) == 0;
    case MTYPE_I5:
    case MTYPE_U5:
      // I5 and U5 are represented as 64-bit
    case MTYPE_I8:
    case MTYPE_U8:
      return (TCON_v0(t)|TCON_v1(t)) == 0;
    case MTYPE_F8:
      /* make sure is not -0.0 (sign-bit set);
       * -0.0 will == 0.0, so add check for sign bit. */
      return (TCON_R8(t) == 0.0
	&& (TCON_v0(t)|TCON_v1(t)) == 0);

#ifdef TARG_NEEDS_QUAD_OPS
    case MTYPE_FQ:
      {
	INT32 dummy_err;
	/* return (TCON_v0(t)|TCON_v1(t)|TCON_v2(t)|TCON_v3(t)) == 0; */
	return (__c_q_eq(R16_To_RQ(TCON_R16(t)), 
			 __c_q_extd(0.0, &dummy_err), 
			 &dummy_err));
      }
#endif

    /* TODO : fix for mongoose */
    case MTYPE_C4:
    case MTYPE_C8:
    case MTYPE_CQ:
    case MTYPE_STR:
      //      return FALSE;

    default:
      ErrMsg ( EC_Inv_Mtype, Mtype_Name(TCON_ty(t)), "Targ_Is_Zero" );
  }
  return FALSE;
} /* Targ_Is_Zero */

/* ====================================================================
 *
 * Targ_Is_Power_Of_Two
 *
 * Determine whether the TCON represents a power of two.
 *
 * ====================================================================
 */

BOOL
Targ_Is_Power_Of_Two ( TCON t )
{
   INT64 cval;
   INT32 exponent, mant;

   if ( Targ_Is_Integral ( t, &cval ) ) {
      if ( cval == 0 ) return FALSE;
      if ( cval == ( cval & ~(cval-1) ) ) return TRUE;
   }

   /* Check for other floating point powers */
   switch (TCON_ty(t)) {
    case MTYPE_F4:
      exponent = (TCON_v0(t) & 0x7f800000) >> 23;
      mant = TCON_v0(t) & 0x007fffff;
      return mant == 0 && exponent != 0 && exponent != 255;

    case MTYPE_F8:
      exponent = (TCON_v1(t) & 0x7ff00000) >> 20;
      mant = (TCON_v1(t) & 0x000fffff) | TCON_v0(t);
      return mant==0 && exponent != 0 && exponent != 2047;
     
#ifdef TARG_NEEDS_QUAD_OPS
    case MTYPE_FQ:
      /* assumes quads are implemented as
       * struct quad {
       *  double hi,lo
       * }
       */
      exponent = (TCON_v1(t) & 0x7ff00000) >> 20;
      mant = (TCON_v1(t) & 0x000fffff) | TCON_v0(t) | TCON_v2(t) | TCON_v3(t);
      return mant==0 && exponent != 0 && exponent != 2047;
#endif
   }

   return FALSE;
}

#ifndef MONGOOSE_BE
/* ====================================================================
 *
 * Targ_Contains_One_Bit_On
 *
 * Determine whether the TCON contains an integral value with only
 * one bit on.
 *
 * If onebit is not NULL then return the bit number of the bit
 * that is on.
 *
 *
 * ====================================================================
 */

BOOL
Targ_Contains_One_Bit_On ( TCON t, INT32 *onebit )
{
  INT64 cval;
  INT32 i;

  if ( Targ_Is_Integral ( t, &cval ) ) {
    if ( cval == 0 ) return FALSE;
    if ( cval != ( cval & ~(cval-1) ) ) return FALSE;
    for (i=0; i< 64; i++) {
      if (cval & 0x1 ) { 
	if (onebit != NULL) *onebit = i;
	return TRUE;
      }
      cval = cval >> 1;
    } /* for i */
  }
  return FALSE;
}

/* ====================================================================
 *
 * Targ_Determine_High_Bit
 *
 * Determine the most significant bit that is on, with 0 being the
 * least-sig bit, and type's_size - 1 being the most-sig bit.
 * If no bits are on, return FALSE.
 *
 * ====================================================================
 */

BOOL
Targ_Determine_High_Bit ( TCON t, INT32 *highbit )
{
  INT64 cval;
  INT32 i;

  if ( Targ_Is_Integral ( t, &cval ) ) {
    UINT64 testbit;
    if ( cval == 0 ) return FALSE;

    testbit = ((INT64)1) << MTYPE_size_min(TCON_ty(t))-1;
    for ( i = MTYPE_size_min(TCON_ty(t))-1; i >= 0; i-- ) {
      if ( (cval & testbit) != 0 ) {
	if ( highbit != NULL ) *highbit = i;
	return TRUE;
      }

      testbit = testbit >> 1;
    }
  }
  return FALSE;
}
#endif /* MONGOOSE_BE */

/* ====================================================================
 *
 * Hash_TCON
 *
 * Hash a TCON into a 32-bit integer modulo another integer.
 *
 * ====================================================================
 */

UINT32
Hash_TCON ( TCON * t, UINT32 modulus )
{
  UINT32 hash = TCON_ty(*t);
  UINT32 rhash;
  INT32 i;
  char *s;

  switch (TCON_ty(*t)) {
    case MTYPE_B:
    case MTYPE_I1:
    case MTYPE_I2:
    case MTYPE_I4:
    case MTYPE_U1:
    case MTYPE_U2:
    case MTYPE_U4:
    case MTYPE_F4:
      hash += TCON_v0(*t);
      break;
    case MTYPE_I8:
    case MTYPE_U8:
    case MTYPE_F8:
      hash += TCON_v0(*t) + TCON_v1(*t);
      break;
    case MTYPE_FQ:
      hash += TCON_v0(*t) + TCON_v1(*t) + TCON_v2(*t) + TCON_v3(*t);
      break;
    case MTYPE_C4:
      hash += TCON_v0(*t);
      hash += TCON_iv0(*t);
      break;
    case MTYPE_C8:
      hash += TCON_v0(*t) + TCON_v1(*t);
      hash += TCON_iv0(*t) + TCON_iv1(*t);
      break;
    case MTYPE_CQ:
      hash += TCON_v0(*t) + TCON_v1(*t) + TCON_v2(*t) + TCON_v3(*t);
      hash += TCON_iv0(*t) + TCON_iv1(*t) + TCON_iv2(*t) + TCON_iv3(*t);
      break;
    case MTYPE_STRING:
      s = Index_to_char_array (TCON_cp (*t));
      for ( i = 0; i < TCON_len(*t); i++, s++ ) {
	hash += (*s) << ((i % 4) * 8);
      }
      break;
    default:
      ErrMsg ( EC_Inv_Mtype, Mtype_Name(TCON_ty(*t)), "Hash_TCON" );
      return 0;
  }

  if ( hash == 0 ) {
    /* Avoid later recalculation: */
    hash = MTYPE_LAST;
  }
  rhash = hash % modulus;

#ifdef Is_True_On
  if ( Get_Trace ( TP_MISC, 16 ) ) {
    fprintf ( TFile, "<tc> Hashing TCON ty=%d, %u mod %u = %u\n",
	      TCON_ty(*t), hash, modulus, rhash );
  }
#endif

  return rhash;
} /* Hash_TCON */

/* ====================================================================
 *
 * Handle_Fortran_Constants 
 * 
 * Process context dependent floating point constants (FORTRAN ONLY)
 *
 * ====================================================================
 */

#ifdef FRONT_END_FORTRAN
BOOL Handle_Fortran_Constants(ND *t, TY *to_ty) 
{
    return FALSE;
} /* Handle_Fortran_Constants */
#endif /* FRONT_END_FORTRAN */


/*
 *   Targ_IntrinsicOp
 *
 *  fold intrinsics. Arguments are essentially the same as Targ_Op and Targ_WhirlOp
 *
 */


TCON Targ_IntrinsicOp ( UINT32 intrinsic, TCON c[], BOOL *folded)
{
   TCON c0,t;
   *folded = TRUE;

   c0 = c[0]; /* to get the type information by default */


#define DEG_TO_RAD (M_PI/180.0)
#define RAD_TO_DEG (180.0/M_PI)

#ifdef QUAD_PRECISION_SUPPORTED
#define M_PIL  3.141592653589793238462643383279531l
#define DEG_TO_RADQ (M_PIL/180.0l)
#define RAD_TO_DEGQ (180.0l/M_PIL)
#endif

   switch ((INTRINSIC) intrinsic) {

    case INTRN_I4EXPEXPR:
      c0 = Targ_Power(c[0],c[1],folded,MTYPE_I4);
      break;
    case INTRN_I8EXPEXPR:
      c0 = Targ_Power(c[0],c[1],folded,MTYPE_I8);
      break;
    case INTRN_F4EXPEXPR:
    case INTRN_F4I4EXPEXPR:
    case INTRN_F4I8EXPEXPR:
      c0 = Targ_Power(c[0],c[1],folded,MTYPE_F4);
      break;
    case INTRN_F8EXPEXPR:
    case INTRN_F8I4EXPEXPR:
    case INTRN_F8I8EXPEXPR:
      c0 = Targ_Power(c[0],c[1],folded,MTYPE_F8);
      break;
    case INTRN_FQEXPEXPR:
    case INTRN_FQI4EXPEXPR:
    case INTRN_FQI8EXPEXPR:
      c0 = Targ_Power(c[0],c[1],folded,MTYPE_FQ);
      break;
    case INTRN_C4EXPEXPR:
    case INTRN_C4I4EXPEXPR:
    case INTRN_C4I8EXPEXPR:
      c0 = Targ_Power(c[0],c[1],folded,MTYPE_C4);
      break;
    case INTRN_C8EXPEXPR:
    case INTRN_C8I4EXPEXPR:
    case INTRN_C8I8EXPEXPR:
      c0 = Targ_Power(c[0],c[1],folded,MTYPE_C8);
      break;
    case INTRN_CQEXPEXPR:
    case INTRN_CQI4EXPEXPR:
    case INTRN_CQI8EXPEXPR:
      c0 = Targ_Power(c[0],c[1],folded,MTYPE_CQ);
      break;


    case INTRN_I1DIM:
    case INTRN_I2DIM:
    case INTRN_I4DIM:
      if (TCON_I4(c[0]) > TCON_I4(c[1])) {
	 TCON_I4(c0) = TCON_I4(c[0]) - TCON_I4(c[1]);
      } else {
	 TCON_I4(c0) = 0;
      }
      break;
    case INTRN_I8DIM:
      if (TCON_I8(c[0]) > TCON_I8(c[1])) {
	 TCON_I8(c0) = TCON_I8(c[0]) - TCON_I8(c[1]);
      } else {
	 TCON_I8(c0) = 0;
      }
      break;

    case INTRN_I1SIGN:
    case INTRN_I2SIGN:
    case INTRN_I4SIGN:
      if (TCON_I4(c0) < 0) TCON_I4(c0) = -TCON_I4(c0);
      if (TCON_I4(c[1]) < 0) TCON_I4(c0) = -TCON_I4(c0);
      break;

    case INTRN_I8SIGN:
      if (TCON_I8(c0) < 0) TCON_I8(c0) = -TCON_I8(c0);
      if (TCON_I8(c[1]) < 0) TCON_I8(c0) = -TCON_I8(c0);
      break;

    case INTRN_F4SIGN:
      c0 = Targ_WhirlOp(OPC_F4ABS,c0,c0,folded);
      if (TCON_R4(c[1]) < 0) TCON_R4(c0) = -TCON_R4(c0);
      break;

    case INTRN_F8SIGN:
      c0 = Targ_WhirlOp(OPC_F8ABS,c0,c0,folded);
      if (TCON_R8(c[1]) < 0) TCON_R8(c0) = -TCON_R8(c0);
      break;

    case INTRN_FQSIGN:
      c0 = Targ_WhirlOp(OPC_FQABS,c0,c0,folded);
      t = Targ_WhirlOp(OPC_I4FQLT,c[1],Quad_Zero_Tcon,folded);
      if (TCON_I4(t)) c0 = Targ_WhirlOp(OPC_FQNEG,c0,c0,folded);
      break;


    case INTRN_I2F4NINT:
      c0 = Targ_Conv(MTYPE_I2,Targ_WhirlOp(OPC_I4F4RND,c[0],c[0],folded));
      break;

    case INTRN_I4F4NINT:
      c0 = Targ_WhirlOp(OPC_I4F4RND,c[0],c[0],folded);
      break;

    case INTRN_I8F4NINT:
      c0 = Targ_WhirlOp(OPC_I8F4RND,c[0],c[0],folded);
      break;

    case INTRN_I2F8IDNINT:
      c0 = Targ_Conv(MTYPE_I2,Targ_WhirlOp(OPC_I4F8RND,c[0],c[0],folded));
      break;

    case INTRN_I4F8IDNINT:
      c0 = Targ_WhirlOp(OPC_I4F8RND,c[0],c[0],folded);
      break;

    case INTRN_I8F8IDNINT:
      c0 = Targ_WhirlOp(OPC_I8F8RND,c[0],c[0],folded);
      break;

    case INTRN_I2FQIQNINT:
      c0 = Targ_Conv(MTYPE_I2,Targ_WhirlOp(OPC_I4FQRND,c[0],c[0],folded));
      break;

    case INTRN_I4FQIQNINT:
      c0 = Targ_WhirlOp(OPC_I4FQRND,c[0],c[0],folded);
      break;

    case INTRN_I8FQIQNINT:
      c0 = Targ_WhirlOp(OPC_I8FQRND,c[0],c[0],folded);
      break;

    case INTRN_I1BITS:
    case INTRN_I2BITS:
    case INTRN_I4BITS:
      {
	 INT64 mask,bits,one=1;
	 mask = (one << TCON_U4(c[2])) - 1;
	 bits = TCON_U4(c0) >> TCON_U4(c[1]);
	 TCON_U4(c0) = mask & bits;
      }
      break;

    case INTRN_I8BITS:
      {
	 INT64 mask,bits,one=1;
	 mask = (one << TCON_U8(c[2])) - 1;
	 bits = TCON_U8(c0) >> TCON_U8(c[1]);
	 TCON_U8(c0) = mask & bits;
      }
      break;

    case INTRN_I1BSET:
    case INTRN_I2BSET:
    case INTRN_I4BSET:
      TCON_I4(c0) |= (1 << TCON_I4(c[1]));
      break;

    case INTRN_I8BSET:
      {
	 INT64 one=1;
	 TCON_I8(c0) |= (one << TCON_I8(c[1]));
      }
      break;

    case INTRN_I1BCLR:
    case INTRN_I2BCLR:
    case INTRN_I4BCLR:
      TCON_I4(c0) &= ~(1 << TCON_I4(c[1]));
      break;

    case INTRN_I8BCLR:
      {
	 INT64 one=1;
	 TCON_I8(c0) &= ~(one << TCON_I8(c[1]));
      }
      break;

    case INTRN_I1BTEST:
    case INTRN_I2BTEST:
    case INTRN_I4BTEST:
      {
	 INT32 test;
	 test = TCON_I4(c0) & (1<<TCON_I4(c[1]));
	 TCON_I4(c0) = (test != 0); TCON_v1(c0) = 0;
	 TCON_ty(c0) = LOGICAL_MTYPE;
      }
      break;

    case INTRN_I8BTEST:
      {
	 INT64 test,one=1;
	 test = TCON_I8(c0) & (one<<TCON_I8(c[1]));
	 TCON_I4(c0) = (test != 0); TCON_v1(c0) = 0;
	 TCON_ty(c0) = LOGICAL_MTYPE;
      }
      break;
      
    case INTRN_I1SHL:
      TCON_I4(c0) = (TCON_I4(c0) << TCON_I4(c[1]))&0xff;
      break;

    case INTRN_I2SHL:
      TCON_I4(c0) = (TCON_I4(c0) << TCON_I4(c[1]))&0xffff;
      break;

    case INTRN_I1SHFT:
      if (TCON_I4(c[1]) >= 0) {
	 TCON_I4(c0) <<= TCON_I4(c[1]);
      } else {
	 TCON_U4(c0) >>= (-TCON_I4(c[1]));
      }
      TCON_U4(c0) &= 0xff;
      break;

    case INTRN_I2SHFT:
      if (TCON_I4(c[1]) >= 0) {
	 TCON_I4(c0) <<= TCON_I4(c[1]);
      } else {
	 TCON_U4(c0) >>= (-TCON_I4(c[1]));
      }
      TCON_U4(c0) &= 0xffff;
      break;

    case INTRN_I4SHFT:
      if (TCON_I4(c[1]) >= 0) {
	 TCON_I4(c0) <<= TCON_I4(c[1]);
      } else {
	 TCON_U4(c0) >>= (-TCON_I4(c[1]));
      }
      break;

    case INTRN_I8SHFT:
      if (TCON_I8(c[1]) >= 0) {
	 TCON_I8(c0) <<= TCON_I8(c[1]);
      } else {
	 TCON_U8(c0) >>= (-TCON_I8(c[1]));
      }
      break;

    case INTRN_I1SHFTC:
    case INTRN_I2SHFTC:
    case INTRN_I4SHFTC:
      {
	 UINT32 mask,size,t,sl,sr;
	 size = TCON_I4(c[2]);
	 if (size == 32) {
	    mask = (UINT32)-1;
	 } else {
	    mask = (1U << size) - 1;
	 }
	 if (TCON_I4(c[1]) >= 0) {
	    sl = TCON_I4(c[1]);
	 } else {
	    sl = size + TCON_I4(c[1]);
	 }
	 sr = size - sl;
	 t = TCON_I4(c0) & mask;
	 t = ((t >> sl) | (t << sr)) & mask;
	 TCON_I4(c0) = t | (TCON_I4(c0) & (~mask));
      }
      break;


    case INTRN_I8SHFTC:
      {
	 UINT64 mask,size,t,sl,sr;
	 size = TCON_I8(c[2]);
	 if (size==64) {
	    mask = (UINT64)-1;
	 } else {
	    mask = (1ULL << size) - 1;
	 }
	 if (TCON_I8(c[1]) >= 0) {
	    sl = TCON_I8(c[1]);
	 } else {
	    sl = size + TCON_I8(c[1]);
	 }
	 sr = size - sl;
	 t = TCON_I8(c0) & mask;
	 t = ((t >> sl) | (t << sr)) & mask;
	 TCON_I8(c0) = t | (TCON_I8(c0) & (~mask));
      }
      break;

    case INTRN_I8DIVFLOOR:
      if (TCON_I8(c[1]) == 0) {
	 *folded = FALSE;
      } else {
	 INT64 q,sign;
	 q = TCON_I8(c[0]) / TCON_I8(c[1]);
	 sign = TCON_I8(c[0]) ^ TCON_I8(c[1]);
	 if (sign < 0 && q*TCON_I8(c[1]) != TCON_I8(c[0])) {
	    q -= 1;
	 }
	 TCON_I8(c0) = q;
      }
      break;

    case INTRN_I4DIVFLOOR:
      if (TCON_I4(c[1]) == 0) {
	 *folded = FALSE;
      } else {
	 INT32 q,sign;
	 q = TCON_I4(c[0]) / TCON_I4(c[1]);
	 sign = TCON_I4(c[0]) ^ TCON_I4(c[1]);
	 if (sign < 0 && q*TCON_I4(c[1]) != TCON_I4(c[0])) {
	    q -= 1;
	 }
	 TCON_I4(c0) = q;
      }
      break;

    case INTRN_U8DIVFLOOR:
      if (TCON_U8(c[1]) == 0) {
	 *folded = FALSE;
      } else {
	 TCON_U8(c0)  = TCON_U8(c[0]) / TCON_U8(c[1]);
      }
      break;

    case INTRN_U4DIVFLOOR:
      if (TCON_U4(c[1]) == 0) {
	 *folded = FALSE;
      } else {
	 TCON_U4(c0) = TCON_U4(c[0]) / TCON_U4(c[1]);
      }
      break;
     
    case INTRN_I8DIVCEIL:
      if (TCON_I8(c[1]) == 0) {
	 *folded = FALSE;
      } else {
	 INT64 q,sign;
	 q = TCON_I8(c[0]) / TCON_I8(c[1]);
	 sign = TCON_I8(c[0]) ^ TCON_I8(c[1]);
	 if (sign >= 0 && q*TCON_I8(c[1]) != TCON_I8(c[0])) {
	    q += 1;
	 }
	 TCON_I8(c0) = q;
      }
      break;

    case INTRN_I4DIVCEIL:
      if (TCON_I4(c[1]) == 0) {
	 *folded = FALSE;
      } else {
	 INT32 q,sign;
	 q = TCON_I4(c[0]) / TCON_I4(c[1]);
	 sign = TCON_I4(c[0]) ^ TCON_I4(c[1]);
	 if (sign >= 0 && q*TCON_I4(c[1]) != TCON_I4(c[0])) {
	    q += 1;
	 }
	 TCON_I4(c0) = q;
      }
      break;

    case INTRN_U8DIVCEIL:
      if (TCON_U8(c[1]) == 0) {
	 *folded = FALSE;
      } else {
	 UINT64 q;
	 q = TCON_U8(c[0]) / TCON_U8(c[1]);
	 if (q*TCON_U8(c[1]) != TCON_U8(c[0])) {
	    q += 1;
	 }
	 TCON_U8(c0) = q;
      }
      break;

    case INTRN_U4DIVCEIL:
      if (TCON_U4(c[1]) == 0) {
	 *folded = FALSE;
      } else {
	 UINT32 q;
	 q = TCON_U4(c[0]) / TCON_U4(c[1]);
	 if (q*TCON_U4(c[1]) != TCON_U4(c[0])) {
	    q += 1;
	 }
	 TCON_U4(c0) = q;
      }
      break;


    case INTRN_F4DIM:
      if (TCON_R4(c[0]) > TCON_R4(c[1])) {
	 TCON_R4(c0) = TCON_R4(c[0]) - TCON_R4(c[1]);
      } else {
	 TCON_R4(c0) = 0.0;
      }
      break;

    case INTRN_F8DIM:
      if (TCON_R8(c[0]) > TCON_R8(c[1])) {
	 TCON_R8(c0) = TCON_R8(c[0]) - TCON_R8(c[1]);
      } else {
	 TCON_R8(c0) = 0.0;
      }
      break;

    case INTRN_FQDIM:
      c0 = Targ_WhirlOp(OPC_FQSUB,c0,c[1],folded);
      t = Targ_WhirlOp(OPC_I4FQLT,c0,Quad_Zero_Tcon,folded);
      if (TCON_I4(t)) c0 = Quad_Zero_Tcon;
      break;
      

    case INTRN_F4AINT:
      t = Targ_WhirlOp(OPC_F4ABS,c[0],c[0],folded);
      if ((INT32) TCON_R4(t) < (1<<30)) {
	 TCON_R4(c0) = (INT32) TCON_R4(c0);
      }
      break;

    case INTRN_F8AINT:
      t = Targ_WhirlOp(OPC_F8ABS,c[0],c[0],folded);
      if ((INT64) TCON_R8(t) < (1LL << 62)) {
	 TCON_R8(c0) = (INT64) TCON_R8(c0);
      }
      break;

    case INTRN_F4ANINT:
      if (TCON_R4(c0) < 0) {
	 TCON_R4(c0) -= 0.5;
      } else {
	 TCON_R4(c0) += 0.5;
      }
      c0 = Targ_IntrinsicOp(INTRN_F4AINT,&c0,folded);
      break;
    case INTRN_F8ANINT:
      if (TCON_R8(c0) < 0) {
	 TCON_R8(c0) -= 0.5;
      } else {
	 TCON_R8(c0) += 0.5;
      }
      c0 = Targ_IntrinsicOp(INTRN_F8AINT,&c0,folded);
      break;

    case INTRN_F4EXP:
      TCON_R4(c0) = expf(TCON_R4(c0));
      break;
    case INTRN_F8EXP:
      TCON_R8(c0) = exp(TCON_R8(c0));
      break;
      
    case INTRN_F4LOG:
      TCON_R4(c0) = logf(TCON_R4(c0));
      break;
    case INTRN_F8LOG:
      TCON_R8(c0) = log(TCON_R8(c0));
      break;
      
    case INTRN_F4LOG10:
      TCON_R4(c0) = log10f(TCON_R4(c0));
      break;
    case INTRN_F8LOG10:
      TCON_R8(c0) = log10(TCON_R8(c0));
      break;

    case INTRN_F4COS:
      TCON_R4(c0) = cosf(TCON_R4(c0));
      break;
    case INTRN_F8COS:
      TCON_R8(c0) = cos(TCON_R8(c0));
      break;

    case INTRN_F4SIN:
      TCON_R4(c0) = sinf(TCON_R4(c0));
      break;
    case INTRN_F8SIN:
      TCON_R8(c0) = sin(TCON_R8(c0));
      break;

    case INTRN_F4TAN:
      TCON_R4(c0) = tanf(TCON_R4(c0));
      break;
    case INTRN_F8TAN:
      TCON_R8(c0) = tan(TCON_R8(c0));
      break;

    case INTRN_F4COSD:
      TCON_R4(c0) = cosf(DEG_TO_RAD*TCON_R4(c0));
      break;
    case INTRN_F8COSD:
      TCON_R8(c0) = cos(DEG_TO_RAD*TCON_R8(c0));
      break;

    case INTRN_F4SIND:
      TCON_R4(c0) = sinf(DEG_TO_RAD*TCON_R4(c0));
      break;
    case INTRN_F8SIND:
      TCON_R8(c0) = sin(DEG_TO_RAD*TCON_R8(c0));
      break;

    case INTRN_F4TAND:
      TCON_R4(c0) = tanf(DEG_TO_RAD*TCON_R4(c0));
      break;
    case INTRN_F8TAND:
      TCON_R8(c0) = tan(DEG_TO_RAD*TCON_R8(c0));
      break;


    case INTRN_F4ACOS:
      TCON_R4(c0) = acosf(TCON_R4(c0));
      break;
    case INTRN_F8ACOS:
      TCON_R8(c0) = acos(TCON_R8(c0));
      break;

    case INTRN_F4ASIN:
      TCON_R4(c0) = asinf(TCON_R4(c0));
      break;
    case INTRN_F8ASIN:
      TCON_R8(c0) = asin(TCON_R8(c0));
      break;

    case INTRN_F4ATAN:
      TCON_R4(c0) = atanf(TCON_R4(c0));
      break;
    case INTRN_F8ATAN:
      TCON_R8(c0) = atan(TCON_R8(c0));
      break;

    case INTRN_F4ACOSD:
      TCON_R4(c0) = RAD_TO_DEG*acosf(TCON_R4(c0));
      break;
    case INTRN_F8ACOSD:
      TCON_R8(c0) = RAD_TO_DEG*acos(TCON_R8(c0));
      break;

    case INTRN_F4ASIND:
      TCON_R4(c0) = RAD_TO_DEG*asinf(TCON_R4(c0));
      break;
    case INTRN_F8ASIND:
      TCON_R8(c0) = RAD_TO_DEG*asin(TCON_R8(c0));
      break;

    case INTRN_F4ATAND:
      TCON_R4(c0) = RAD_TO_DEG*atanf(TCON_R4(c0));
      break;
    case INTRN_F8ATAND:
      TCON_R8(c0) = RAD_TO_DEG*atan(TCON_R8(c0));
      break;

    case INTRN_F4COSH:
      TCON_R4(c0) = coshf(TCON_R4(c0));
      break;
    case INTRN_F8COSH:
      TCON_R8(c0) = cosh(TCON_R8(c0));
      break;

    case INTRN_F4SINH:
      TCON_R4(c0) = sinhf(TCON_R4(c0));
      break;
    case INTRN_F8SINH:
      TCON_R8(c0) = sinh(TCON_R8(c0));
      break;

    case INTRN_F4TANH:
      TCON_R4(c0) = tanhf(TCON_R4(c0));
      break;
    case INTRN_F8TANH:
      TCON_R8(c0) = tanh(TCON_R8(c0));
      break;

    case INTRN_F4ATAN2:
      TCON_R4(c0) = atan2f(TCON_R4(c0),TCON_R4(c[1]));
      break;
    case INTRN_F8ATAN2:
      TCON_R8(c0) = atan2(TCON_R8(c0),TCON_R8(c[1]));
      break;

    case INTRN_F4ATAN2D:
      TCON_R4(c0) = RAD_TO_DEG*atan2f(TCON_R4(c0),TCON_R4(c[1]));
      break;
    case INTRN_F8ATAN2D:
      TCON_R8(c0) = RAD_TO_DEG*atan2(TCON_R8(c0),TCON_R8(c[1]));
      break;

    case INTRN_F4CIS:
      TCON_ty(c0) = MTYPE_C4;
      TCON_R4(c0) = cosf(TCON_R4(c[0]));
      TCON_IR4(c0) = sinf(TCON_R4(c[0]));
      break;

    case INTRN_F8CIS:
      TCON_ty(c0) = MTYPE_C8;
      TCON_R8(c0) = cos(TCON_R8(c[0]));
      TCON_IR8(c0) = sin(TCON_R8(c[0]));
      break;

#ifdef QUAD_PRECISION_SUPPORTED
      /* Warning: we use native long doubles in here */

    case INTRN_FQEXP:
      TCON_R16(c0) = RLD_To_R16(expl(R16_To_RLD(TCON_R16(c0))));
      break;
    case INTRN_FQLOG:
      TCON_R16(c0) = RLD_To_R16(logl(R16_To_RLD(TCON_R16(c0))));
      break;
    case INTRN_FQLOG10:
      TCON_R16(c0) = RLD_To_R16(log10l(R16_To_RLD(TCON_R16(c0))));
      break;
    case INTRN_FQCOS:
      TCON_R16(c0) = RLD_To_R16(cosl(R16_To_RLD(TCON_R16(c0))));
      break;
    case INTRN_FQSIN:
      TCON_R16(c0) = RLD_To_R16(sinl(R16_To_RLD(TCON_R16(c0))));
      break;
    case INTRN_FQTAN:
      TCON_R16(c0) = RLD_To_R16(tanl(R16_To_RLD(TCON_R16(c0))));
      break;
    case INTRN_FQCOSD:
      TCON_R16(c0) = RLD_To_R16(cosl(DEG_TO_RADQ*R16_To_RLD(TCON_R16(c0))));
      break;
    case INTRN_FQSIND:
      TCON_R16(c0) = RLD_To_R16(sinl(DEG_TO_RADQ*R16_To_RLD(TCON_R16(c0))));
      break;
    case INTRN_FQTAND:
      TCON_R16(c0) = RLD_To_R16(tanl(DEG_TO_RADQ*R16_To_RLD(TCON_R16(c0))));
      break;
    case INTRN_FQCOSH:
      TCON_R16(c0) = RLD_To_R16(coshl(R16_To_RLD(TCON_R16(c0))));
      break;
    case INTRN_FQSINH:
      TCON_R16(c0) = RLD_To_R16(sinhl(R16_To_RLD(TCON_R16(c0))));
      break;
    case INTRN_FQTANH:
      TCON_R16(c0) = RLD_To_R16(tanhl(R16_To_RLD(TCON_R16(c0))));
      break;
    case INTRN_FQACOS:
      TCON_R16(c0) = RLD_To_R16(acosl(R16_To_RLD(TCON_R16(c0))));
      break;
    case INTRN_FQASIN:
      TCON_R16(c0) = RLD_To_R16(asinl(R16_To_RLD(TCON_R16(c0))));
      break;
    case INTRN_FQATAN:
      TCON_R16(c0) = RLD_To_R16(atanl(R16_To_RLD(TCON_R16(c0))));
      break;
    case INTRN_FQACOSD:
      TCON_R16(c0) = RLD_To_R16(RAD_TO_DEGQ*acosl(R16_To_RLD(TCON_R16(c0))));
      break;
    case INTRN_FQASIND:
      TCON_R16(c0) = RLD_To_R16(RAD_TO_DEGQ*asinl(R16_To_RLD(TCON_R16(c0))));
      break;
    case INTRN_FQATAND:
      TCON_R16(c0) = RLD_To_R16(RAD_TO_DEGQ*atanl(R16_To_RLD(TCON_R16(c0))));
      break;
    case INTRN_FQATAN2:
      TCON_R16(c0) = RLD_To_R16(atan2l(R16_To_RLD(TCON_R16(c0)),R16_To_RLD(TCON_R16(c[1]))));
      break;
    case INTRN_FQATAN2D:
      TCON_R16(c0) = RLD_To_R16(RAD_TO_DEGQ*atan2l(R16_To_RLD(TCON_R16(c0)),R16_To_RLD(TCON_R16(c[1]))));
      break;

    case INTRN_FQCIS:
      TCON_ty(c0) = MTYPE_CQ;
      TCON_R16(c0) = RLD_To_R16(cosl(R16_To_RLD(TCON_R16(c[0]))));
      TCON_IR16(c0) = RLD_To_R16(sinl(R16_To_RLD(TCON_R16(c[0]))));
      break;

    case INTRN_FQAINT:
      TCON_R16(c0) = RLD_To_R16(truncl(R16_To_RLD(TCON_R16(c0))));
      break;

    case INTRN_FQANINT:
      if (R16_To_RLD(TCON_R16(c0)) < 0) {
	 TCON_R16(c0) = RLD_To_R16(truncl(R16_To_RLD(TCON_R16(c0))-0.5l));
      } else {
	 TCON_R16(c0) = RLD_To_R16(truncl(R16_To_RLD(TCON_R16(c0))+0.5l));
      }
      break;

      /* These aren't done because we will catch them when lowering */
    case INTRN_C4COS:
    case INTRN_C8COS:
    case INTRN_C4SIN:
    case INTRN_C8SIN:
    case INTRN_C4EXP:
    case INTRN_C8EXP:
    case INTRN_C4LOG:
    case INTRN_C8LOG:
    case INTRN_CQEXP:
    case INTRN_CQLOG:
    case INTRN_CQCOS:
    case INTRN_CQSIN:
    case INTRN_F8F4PROD:
    case INTRN_FQF8PROD:
      *folded = FALSE;
      break;

#else /* No quad support in library */
      /* These aren't done because we don't have a convenient library yet */
    case INTRN_FQAINT:
    case INTRN_FQANINT:
    case INTRN_FQEXP:
    case INTRN_FQLOG:
    case INTRN_FQLOG10:
    case INTRN_FQCOS:
    case INTRN_FQSIN:
    case INTRN_FQCIS:
    case INTRN_FQTAN:
    case INTRN_FQCOSD:
    case INTRN_FQSIND:
    case INTRN_FQTAND:
    case INTRN_FQCOSH:
    case INTRN_FQSINH:
    case INTRN_FQTANH:
    case INTRN_FQACOS:
    case INTRN_FQASIN:
    case INTRN_FQATAN:
    case INTRN_FQACOSD:
    case INTRN_FQASIND:
    case INTRN_FQATAND:
    case INTRN_FQATAN2:
    case INTRN_FQATAN2D:

      /* These aren't done because we will catch them when lowering */
    case INTRN_C4COS:
    case INTRN_C8COS:
    case INTRN_C4SIN:
    case INTRN_C8SIN:
    case INTRN_C4EXP:
    case INTRN_C8EXP:
    case INTRN_C4LOG:
    case INTRN_C8LOG:
    case INTRN_CQEXP:
    case INTRN_CQLOG:
    case INTRN_CQCOS:
    case INTRN_CQSIN:
    case INTRN_F8F4PROD:
    case INTRN_FQF8PROD:
      *folded = FALSE;
      break;
#endif

    case INTRN_I1POPCNT:
    case INTRN_I2POPCNT:
    case INTRN_I4POPCNT:
    case INTRN_I8POPCNT:
       {
	  INT64 count;
	  INT i,numbits;
	  INT64 val;
	  switch (intrinsic) {
	   case INTRN_I1POPCNT:
	      numbits = 8;
	      val = TCON_I4(c0);
	      break;
	   case INTRN_I2POPCNT:
	      numbits = 16;
	      val = TCON_I4(c0);
	      break;
	   case INTRN_I4POPCNT:
	      numbits = 32;
	      val = TCON_I4(c0);
	      break;
	   case INTRN_I8POPCNT:
	      numbits = 64;
	      val = TCON_I8(c0);
	      break;
	  }
	  count = 0;
	  for (i = 0; i < numbits; i++) {
	     count += (val & 1);
	     val >>= 1;
	  }
	  TCON_I4(c0) = count;
	  TCON_ty(c0) = MTYPE_I4;
       }
       break;

    case INTRN_I1LEADZ:
    case INTRN_I2LEADZ:
    case INTRN_I4LEADZ:
    case INTRN_I8LEADZ:
       {
	  INT64 count;
	  INT i,numbits;
	  INT64 val;
	  switch (intrinsic) {
	   case INTRN_I1LEADZ:
	      numbits = 8;
	      val = TCON_I4(c0);
	      val <<= 56;
	      break;
	   case INTRN_I2LEADZ:
	      numbits = 16;
	      val = TCON_I4(c0);
	      val <<= 48;
	      break;
	   case INTRN_I4LEADZ:
	      numbits = 32;
	      val = TCON_I4(c0);
	      val <<= 32;
	      break;
	   case INTRN_I8LEADZ:
	      numbits = 64;
	      val = TCON_I8(c0);
	      break;
	  }
	  count = 0;
	  for (i = 0; i < numbits; i++) {
	     if (val >= 0) {
		++count;
		val <<= 1;
	     } else {
		break;
	     }
	  }
	  TCON_I4(c0) = count;
	  TCON_ty(c0) = MTYPE_I4;
       }
       break;

    default:
      *folded = FALSE;
      break;
   }
   return (c0);
}

#endif /* TARG_ST */
