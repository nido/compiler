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


/*
 * This defines the ABI subprogram interface,
 * and is used to determine how parameters and results are passed.
 * We have an array of tables, where each table describes the info
 * for one abi.  The array is indexed by the TARGET_ABI enumeration.
 * The register values are the PREG offsets, so these values can be
 * used in WHIRL.
 */

#define TRACE_ENTRY(x)
#define TRACE_EXIT(x)
#define TRACE_EXIT_i(x,i)

#include <limits.h>
#include "defs.h"
#include "mtypes.h"
#include "errors.h"
#include "erglob.h"
#include "stab.h"
#include "config_targ.h"
#include "targ_sim.h"
#include "targ_sim_body.h"

/* some definitions for the dedicated hardware pregs: */

UINT Int_Preg_Min_Offset              =  1;
UINT Int_Preg_Max_Offset           =  64;
UINT Ptr_Preg_Min_Offset              =  1;
UINT Ptr_Preg_Max_Offset           =  0;
UINT Float_Preg_Min_Offset         =  1;
UINT Float_Preg_Max_Offset         =  0;  
UINT Branch_Preg_Min_Offset        =  65;
UINT Branch_Preg_Max_Offset        =  72;
UINT Fcc_Preg_Min_Offset           =  1;
UINT Fcc_Preg_Max_Offset           =  0;
UINT Last_Dedicated_Preg_Offset    =  Branch_Preg_Max_Offset;

/* The offsets for return registers are fixed: */
UINT First_Int_Preg_Return_Offset	= 17;	/* register v0 */
UINT Last_Int_Preg_Return_Offset	= 18;	/* register v1 */
UINT First_Ptr_Preg_Return_Offset       = 0;
UINT Last_Ptr_Preg_Return_Offset        = 0;
UINT First_Float_Preg_Return_Offset	= 0;	/* register f0 */
UINT Last_Float_Preg_Return_Offset	= 0;	/* register f2 */

/* Parameter placement */
UINT First_Int_Preg_Param_Offset	= 17;	/* register a0 */
UINT First_Ptr_Preg_Param_Offset        = 0;
UINT First_Float_Preg_Param_Offset	= 0;	/* register fa0 */

UINT Stack_Pointer_Preg_Offset	= 13;	/* register sp */
UINT Frame_Pointer_Preg_Offset	= 62;	/* register fp */
UINT Static_Link_Preg_Offset	= 63;
UINT Struct_Return_Preg_Offset  = 16;   /* returning structs */

static mDED_PREG_NUM Input_Base_Preg = 0;
static mDED_PREG_NUM Output_Base_Preg = 0;

#define I0 Int_Preg_Min_Offset
#define P0 Ptr_Preg_Min_Offset
#define F0 Float_Preg_Min_Offset

static
SIM SIM_Info[] = {
	/* flags */
	/* int args, flt args, dbl args */
	/* int res , flt res, dbl res */
	/* int type, flt type, dbl type */
	/* save area, formal-area, var ofst */
	/* struct arg, struct res, slink, pic */
  {/* ABI_UNDEF */
	0,
	{0,0,0}, {0,0,0}, {0,0,0}, {0,0,0},
	{0,0,0}, {0,0,0}, {0,0,0}, {0,0,0},
	0, 0, 0, 0,
	0, 0, 0, 
	0, 0, 0, 0
  },
  { /* ABI_ST200 */
	SIM_COORD_MEM_REG | SIM_REG_STRUCTS | SIM_RET_ADDR_VIA_INT_RET,
	{I0+16,I0+23,1}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
	{I0+16,I0+17,1}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
	MTYPE_I4, MTYPE_U4, MTYPE_U4, MTYPE_U8,
	0, 64, -64, 
	-1, 32, I0+62, I0+25
  }
};

/* ====================================================================
 *   Is_Return_Preg
 *
 *   return whether preg is a return preg
 * ====================================================================
 */
BOOL 
Is_Return_Preg (
  PREG_NUM preg
)
{
  return (preg >= First_Int_Preg_Return_Offset
	                 && preg <= Last_Int_Preg_Return_Offset);
}

/* ====================================================================
 *   Is_Int_Output_Preg
 *
 *   return whether preg is an output preg 
 * ====================================================================
 */
BOOL 
Is_Int_Output_Preg (
  PREG_NUM preg
)
{
  // Seems like rotating register stuff ...
#if 0
  return (preg <= Output_Base_Preg 
      && preg > (Output_Base_Preg - MAX_NUMBER_OF_REGISTER_PARAMETERS));
#endif
  return FALSE;
}

/* ====================================================================
 *   Is_Formal_Preg
 *
 *   return whether preg is an input preg 
 * ====================================================================
 */
extern BOOL
Is_Formal_Preg (
  PREG_NUM preg
)
{
  return ((preg >= PR_first_reg(SIM_INFO.int_args) &&
	                   preg <= PR_last_reg(SIM_INFO.int_args)));
}

static BOOL Struct_Is_HFA (const TY_IDX, Mtype_Return_Level, TYPE_ID&);

/* ====================================================================
 *   Array_Is_HFA
 * ====================================================================
 */
static BOOL
Array_Is_HFA (
  const TY_IDX       ty,
  Mtype_Return_Level level,
  TYPE_ID&           hfa_mtype
             )
{
  const TY_IDX ety = TY_etype (ty);

  if (TY_kind (ety) == KIND_SCALAR) {

    TYPE_ID mtype = TY_mtype (ety);

    switch (mtype) {

      case MTYPE_F4:
      case MTYPE_F8:
      case MTYPE_FQ:
      case MTYPE_F10:
      case MTYPE_F16:
      case MTYPE_C4:
      case MTYPE_C8:
      case MTYPE_CQ:
      case MTYPE_C10:
      case MTYPE_C16:

        if (hfa_mtype == MTYPE_V)
          hfa_mtype = mtype;

        else
        if (hfa_mtype != mtype)
          return FALSE;

        break;

      default:

        return FALSE;
    }
  }

  else
  if (TY_kind (ety) == KIND_ARRAY) {

    if (!Array_Is_HFA (ety, level, hfa_mtype))
      return FALSE;
  }

  else
  if (TY_kind (ety) == KIND_STRUCT) {

    if (!Struct_Is_HFA (ety, level, hfa_mtype))
      return FALSE;
  }

  else
    return FALSE;

  return TRUE;
} /* Array_Is_HFA */


/* ====================================================================
 *   Struct_Is_HFA
 * ====================================================================
 */
static BOOL
Struct_Is_HFA (
  const TY_IDX       ty,
  Mtype_Return_Level level,
  TYPE_ID&           hfa_mtype
)
{
  mUINT64 offset = 0;

  if (TY_is_union (ty))
    return FALSE;

  if (TY_fld (ty).Is_Null ())
    return FALSE;

  FLD_HANDLE fld = TY_fld (ty);

  do {

    if (FLD_ofst (fld) != offset)
      return FALSE;

    TY_IDX fty = FLD_type (fld);

    if (TY_kind (fty) == KIND_SCALAR) {

      TYPE_ID mtype = TY_mtype (fty);

      switch (mtype) {

        case MTYPE_F4:
        case MTYPE_F8:
        case MTYPE_FQ:
        case MTYPE_F10:
        case MTYPE_F16:
        case MTYPE_C4:
        case MTYPE_C8:
        case MTYPE_CQ:
        case MTYPE_C10:
        case MTYPE_C16:

          if (hfa_mtype == MTYPE_V)
            hfa_mtype = mtype;

          else
          if (hfa_mtype != mtype)
            return FALSE;

          break;

        default:

          return FALSE;
      }
    }

    else
    if (TY_kind (fty) == KIND_ARRAY) {

      if (!Array_Is_HFA (fty, level, hfa_mtype))
        return FALSE;
    }

    else
    if (TY_kind (fty) == KIND_STRUCT) {

      if (!Struct_Is_HFA (fty, level, hfa_mtype))
        return FALSE;
    }

    else
      return FALSE;

    offset += TY_size (fty);
    fld = FLD_next (fld);
  } while (!fld.Is_Null ());

  return TRUE;
} /* Struct_Is_HFA */

/* ====================================================================
 *   Struct_Has_One_Float
 * ====================================================================
 */
static BOOL
Struct_Has_One_Float (
  const TY_IDX ty, 
  TYPE_ID &ftype
)
{
    if (TY_is_union (ty))
	return FALSE;

    if (TY_fld (ty).Is_Null ())
	return FALSE;

    FLD_HANDLE fld = TY_fld (ty);

    if (FLD_last_field (fld) && FLD_type (fld) != 0 &&
	TY_kind (FLD_type (fld)) == KIND_SCALAR) {
	ftype = TY_mtype (FLD_type (fld));
	if (MTYPE_float (ftype))
	    return TRUE;
    }
    ftype = MTYPE_V;
    return FALSE;
} // Struct_Has_One_Float

/* ====================================================================
 *   no_overlap
 *
 *   check if two fields in a struct overlaps.
 * ====================================================================
 */
static BOOL
no_overlap (const FLD_HANDLE fld1, const FLD_HANDLE fld2)
{
    if (FLD_ofst (fld1) <= FLD_ofst (fld2))
	return FLD_ofst(fld1) + TY_size (FLD_type (fld1)) <= FLD_ofst (fld2);
    else
	return FLD_ofst(fld2) + TY_size (FLD_type (fld2)) <= FLD_ofst (fld1);
} // no_overlap

/* ====================================================================
 *   Struct_Has_Two_Floats
 * ====================================================================
 */
static BOOL
Struct_Has_Two_Floats (
  const TY_IDX ty, 
  TYPE_ID& ftype1, 
  TYPE_ID& ftype2
)
{
    if (TY_is_union (ty))
	return FALSE;

    if (TY_fld (ty).Is_Null ())
	return FALSE;

    FLD_HANDLE fld1 = TY_fld (ty);

    if (FLD_last_field (fld1))
	return FALSE;

    FLD_HANDLE fld2 (FLD_next (fld1));

    if (FLD_last_field (fld2) &&
	FLD_type (fld1) != 0 && TY_kind (FLD_type (fld1)) == KIND_SCALAR &&
	FLD_type (fld2) != 0 && TY_kind (FLD_type (fld2)) == KIND_SCALAR &&
	no_overlap (fld1, fld2)) {
	ftype1 = TY_mtype (FLD_type(fld1));
	ftype2 = TY_mtype (FLD_type(fld2));
	if (MTYPE_float (ftype1) && MTYPE_float (ftype2))
	    return TRUE;
    }
    ftype1 = ftype2 = MTYPE_V;
    return FALSE;

} // Struct_Has_Two_Floats

/* ====================================================================
 *   Is_Simulated_Type
 * ====================================================================
 */
static BOOL
Is_Simulated_Type (
  TYPE_ID mtype
)
{
  switch (mtype) {
    case MTYPE_FQ: return TRUE;
    case MTYPE_C4: return TRUE;
    case MTYPE_C8: return TRUE;
    case MTYPE_CQ: return TRUE;
    default:	return FALSE;
  }
}

/* This routine figures out the mtypes of the return registers that are 
 * used for returning an object of the given type.
 * This returns the mtypes to use for the CALL opcode in high-level whirl.
 * This means that returns of simulated objects, like FQ, are just shown
 * as returning FQ, which will later be split into F8F8.
 * However, structures that return in registers are specified explicitly.
 */
/*ARGSUSED*/
extern void
Get_Return_Mtypes (
  TY_IDX rtype,		/* The result type */
  Mtype_Return_Level level,	/* whether to lower the mtypes */
  TYPE_ID *mreg1,	/* out: mtype for result register 1 */
  TYPE_ID *mreg2)	/* out: mtype for result register 2 */
{
  Fail_FmtAssertion (
    ("Get_Return_Mtypes should not be invoked; invoke Get_Return_Info instead"));
}

/* This routine figures out which return registers are to be used
 * for returning an object with the given mtypes.
 * It is assumed that the mtypes will be determined by calling
 * Get_Return_Mtypes.
 */
/*ARGSUSED*/
extern void
Get_Return_Pregs (
  TYPE_ID mreg1,	/* in:  mtype for result register 1 */
  TYPE_ID mreg2,	/* in:  mtype for result register 2 */
  PREG_NUM *rreg1,	/* out: result register 1 */
  PREG_NUM *rreg2)	/* out: result register 2 */
{
  Fail_FmtAssertion (
    ("Get_Return_Pregs should not be invoked; invoke Get_Return_Info instead"));
}

/* ====================================================================
 *   Get_Return_Info
 *
 *   This routine figures out the mtypes of the return registers that are 
 *   used for returning an object of the given type.
 *   This returns the mtypes to use for the CALL opcode in high-level whirl.
 *   This means that returns of simulated objects, like FQ, are just shown
 *   as returning FQ, which will later be split into F8F8.
 *   However, structures that return in registers are specified explicitly.
 *
 *   This routine figures out which return registers are to be used
 *   for returning an object with the given mtypes.
 *   It is assumed that the mtypes will be determined by calling
 *   Get_Return_Mtypes.
 * ====================================================================
 */
RETURN_INFO
Get_Return_Info (
  TY_IDX rtype, 
  Mtype_Return_Level level
)
{
  TYPE_ID mtype = TY_mtype (rtype);
  PREG_NUM reg;
  RETURN_INFO info;
  INT32 i; 

  info.return_via_first_arg = FALSE;
  info.return_via_return_reg = FALSE;

  switch (mtype) {

    case MTYPE_UNKNOWN:

      // FORTRAN character array
      info.count = 0;
      // f90 already has made visible the arg for arrays
      // info.return_via_first_arg = TRUE;
      break;

    case MTYPE_V:

      info.count = 0;
      break;

    // Floating point types are mapped to integer regs
    case MTYPE_F4:

      info.count = 1;
      info.mtype [0] = MTYPE_I4;
      info.preg  [0] = PR_first_reg(SIM_INFO.int_results);
      break;

    case MTYPE_I1:
    case MTYPE_I2:
    case MTYPE_I4:
    case MTYPE_U1:
    case MTYPE_U2:
    case MTYPE_U4:
    case MTYPE_A4:

      info.count = 1;
      info.mtype [0] = mtype;
      info.preg  [0] = PR_first_reg(SIM_INFO.int_results);
      break;

    // Floating-point types are mapped to integer regs
    case MTYPE_F8:

      reg = PR_first_reg(SIM_INFO.int_results);
      if (level == Use_Simulated) {
	info.count = 1;
	info.mtype [0] = MTYPE_I8;
	info.preg  [0] = reg;
      }
      else {
	info.count = 2;
	info.mtype [0] = SIM_INFO.int_type;
	info.preg  [0] = reg++;
	info.mtype [1] = SIM_INFO.int_type;
	info.preg  [1] = reg++;
      }
      break;

    case MTYPE_I8:
    case MTYPE_U8:

      reg = PR_first_reg(SIM_INFO.int_results);
      if (level == Use_Simulated) {
	info.count = 1;
	info.mtype [0] = mtype;
	info.preg  [0] = reg;
      }
      else {
	info.count = 2;
	info.mtype [0] = SIM_INFO.int_type;
	info.preg  [0] = reg++;
	info.mtype [1] = SIM_INFO.int_type;
	info.preg  [1] = reg++;
      }
      break;

    case MTYPE_M:

      //
      // Return Structure or Array.
      // Aggregates with # elements <= 8 and under 256 bits are
      // returned in registers r0.16 - r0.23.
      // Otherwise, return in a callee buffer pointed at by r0.15.
      //

      info.count = 0;
      info.return_via_return_reg = TRUE;

#if 0
      if (SIM_INFO.max_struct_result != 0) {

        UINT64 size = TY_size(Ty_Table[rtype]);

        if (size > 0 && 8 * size <= 2 * SIM_INFO.max_struct_result) {

          TYPE_ID hfa_mtype = MTYPE_V;

          if (Struct_Is_HFA (rtype, level, hfa_mtype) &&
              hfa_mtype != MTYPE_V &&
              ((hfa_mtype != MTYPE_F4 && hfa_mtype != MTYPE_C4) ||
               ((hfa_mtype == MTYPE_F4 || hfa_mtype == MTYPE_C4) &&
                8 * size <= SIM_INFO.max_struct_result))) {

            PREG_NUM reg = PR_first_reg(SIM_INFO.flt_results);
            INT32 n;
            INT32 i;
            INT32 step;

            info.return_via_first_arg = FALSE;

            switch (hfa_mtype) {

              case MTYPE_F4:
              case MTYPE_F8:
              case MTYPE_F10:

                break;

              case MTYPE_C4:
              case MTYPE_C8:
              case MTYPE_C10:

                if (level != Use_Simulated)
                  hfa_mtype = Mtype_complex_to_real(hfa_mtype);
                break;
            }
                  
            switch (hfa_mtype) {

              case MTYPE_F4:
              case MTYPE_C4:

                n = TY_size (rtype) / TY_size (Be_Type_Tbl (MTYPE_F4));
                info.count = n;

                for (i = 0; i < n; i++) {

                  info.mtype [i] = hfa_mtype; 
                  info.preg  [i] = reg; 
                  reg += PR_skip_value(SIM_INFO.flt_results);
                }
                break;

              case MTYPE_F8:
              case MTYPE_F10:
              case MTYPE_C8:
              case MTYPE_C10:

                n = TY_size (rtype) / TY_size (Be_Type_Tbl (hfa_mtype));
                step = TY_size (Be_Type_Tbl (hfa_mtype)) /
                       TY_size (Be_Type_Tbl (MTYPE_F8));
                info.count = n;

                for (i = 0; i < n; i++) {

                  info.mtype [i] = hfa_mtype; 
                  info.preg  [i] = reg; 
                  reg += step * PR_skip_value(SIM_INFO.flt_results);
                }
                break;
            }
          }

          else
          if (8 * size <= SIM_INFO.max_struct_result) {

            int n =   (size + MTYPE_RegisterSize(SIM_INFO.int_type) - 1)
                    / MTYPE_RegisterSize(SIM_INFO.int_type);
            reg = PR_first_reg(SIM_INFO.int_results);

            info.return_via_first_arg = FALSE;
            info.count = n;
            for (int i = 0; i < n; i++) {

              info.mtype [i] = SIM_INFO.int_type;
              info.preg  [i] = reg++;
            }

            break;
          }
        }
      }
#endif
      break;

    case MTYPE_FQ:
    case MTYPE_C4:
    case MTYPE_C8:

    default:

      info.count = 0;
      FmtAssert (FALSE, ("Invalid return mtype %s encountered",
                                                (MTYPE_name(mtype))));
      break;
  } /* switch (mtype) */

  for (i = info.count; i < MAX_NUMBER_OF_REGISTERS_FOR_RETURN; i++) {
    info.mtype [i] = MTYPE_V;
    info.preg  [i] = 0;
  }

  return info;
} /* Get_Return_Info */

static INT Current_Int_Param_Num = 0;

#if 0
static INT Current_Float_Param_Num = -1;
#endif

static INT Current_Offset;
static BOOL First_Param_In_Return_Reg = FALSE;

/* ====================================================================
 *   Setup_Parameter_Locations
 * ====================================================================
 */
static PLOC
Setup_Parameter_Locations (
  TY_IDX pu_type
)
{
  static PLOC plocNULL;

  TY_IDX ret_type = (TY_kind(pu_type) == KIND_FUNCTION ? TY_ret_type(pu_type)
			: pu_type);
  RETURN_INFO info = Get_Return_Info (ret_type, No_Simulated);
  First_Param_In_Return_Reg = (RETURN_INFO_return_via_first_arg(info) 
			       & SIM_return_addr_via_int_return_reg);
  if (TY_is_varargs (pu_type)) {
    // find last fixed parameter
    TYLIST_IDX idx = TY_tylist (pu_type);
    Last_Fixed_Param = -1;
    for (++idx; Tylist_Table[idx] != 0; ++idx)
      ++Last_Fixed_Param;
    // old style varargs is counting va_alist and should not
    if ( ! TY_has_prototype(pu_type))
      --Last_Fixed_Param;
    // account for functions returning to first parameter
    if (TY_return_to_param (pu_type))
      ++Last_Fixed_Param;
  } else
    Last_Fixed_Param = INT_MAX;

  Current_Int_Param_Num = 0;
  Current_Offset = 0;

  return plocNULL;
} 

/* ====================================================================
 *   Get_Current_Int_Preg_Num
 * ====================================================================
 */
static inline PREG_NUM
Get_Current_Int_Preg_Num (
  Preg_Range pr
)
{
  PREG_NUM i;

  i = PR_first_reg(pr) + (Current_Int_Param_Num * PR_skip_value(pr));
  if (i > PR_last_reg(pr))
    return 0;
  else
    return i;
}

/* ====================================================================
 *   Get_Preg_Alignment
 *
 *   Pregs list begins on a double word alignment.
 *   Depending on preg it is either 4 or 8 bytes aligned.
 *   If preg is 0 (parm on stack), return alignment 0.
 * ====================================================================
 */
static inline INT
Get_Preg_Alignment (
  PREG_NUM preg
)
{
  if (preg == 0) return 0;
  
  if ((preg % 2) == 0) return 4;

  else return 8;
}

/* ====================================================================
 *   Get_Parameter_Location
 * ====================================================================
 */
static PLOC
Get_Parameter_Location (
  TY_IDX ty, 
  BOOL is_output
)
{
  PLOC ploc;				// return location

  ploc.reg = 0;
  ploc.start_offset = Current_Offset;
  ploc.size = 0;
  ploc.vararg_reg = 0;               // to silence purify

#if 0
  // Arthur: NEVER REACHED ??
  if (TY_kind (ty) == KIND_VOID) {
    if (is_output && IS_INT_PREG(PLOC_reg(ploc)))
      PLOC_reg(ploc) = Output_Base_Preg - PLOC_reg(ploc) + 32;
    else if ( ! is_output && IS_INT_PREG(PLOC_reg(ploc)))
      PLOC_reg(ploc) = Input_Base_Preg + PLOC_reg(ploc) - 32;
    return ploc;
  }
#endif

  /* check for array case where fe doesn't fill in right btype */
  TYPE_ID pmtype = Fix_TY_mtype (ty);	/* Target type */

  ploc.size = MTYPE_RegisterSize(pmtype);

  if (First_Param_In_Return_Reg) {
    First_Param_In_Return_Reg = FALSE;
    ploc.reg = PR_first_reg(SIM_INFO.int_results);
    if (is_output && IS_INT_PREG(PLOC_reg(ploc)))
      PLOC_reg(ploc) = Output_Base_Preg - PLOC_reg(ploc) + 32;
    else if ( ! is_output && IS_INT_PREG(PLOC_reg(ploc)))
      PLOC_reg(ploc) = Input_Base_Preg + PLOC_reg(ploc) - 32;
    return ploc;
  }

#if 0
  // Arthur:  I may nedd to check for alignment later ??
  ++Current_Param_Num;
  if (TY_align_exp (ty) == 4 && (Current_Param_Num % 2) == 1) {

    FmtAssert(FALSE,("Get_Parameter_Location: type quad aligned"));

    /* skip a parameter slot so quad-aligned */

    ++Current_Param_Num;

    /* adjust Last_Fixed_Param in varargs case */

    if (Last_Fixed_Param < INT_MAX)
      ++Last_Fixed_Param;
    ploc.start_offset += MTYPE_RegisterSize(SIM_INFO.flt_type);
  }
#endif

  INT rpad = 0;			/* padding to right of object */

  switch (pmtype) {
	
    case MTYPE_I1:
    case MTYPE_U1:
    case MTYPE_I2:
    case MTYPE_U2:
    case MTYPE_I4:
    case MTYPE_U4:

      if (Target_Byte_Sex == BIG_ENDIAN) {
	/* want to right-justify the object */
	ploc.start_offset += (MTYPE_RegisterSize(SIM_INFO.int_type) -
			      ploc.size);
      }
      else {
	/* Pad to word; leave address alone */
          rpad = (MTYPE_RegisterSize(SIM_INFO.int_type) - ploc.size);
      }
      ploc.reg = Get_Current_Int_Preg_Num (SIM_INFO.int_args);
      Current_Int_Param_Num++;
      break;

      // Floating-point types are mapped to integer regs
    case MTYPE_F4:

    case MTYPE_A4:

      ploc.reg = Get_Current_Int_Preg_Num (SIM_INFO.int_args);
      Current_Int_Param_Num++;
      break;

    case MTYPE_I8:
    case MTYPE_U8:

      // Floating-point types are mapped to integer regs
    case MTYPE_F8:

      //
      // These occupy two registers. This must be aligned on an
      // 8-byte boundary, so may skip a slot
      //
      ploc.reg = Get_Current_Int_Preg_Num (SIM_INFO.int_args);
      if (Get_Preg_Alignment(ploc.reg) == 4) {
	//
	// skip one slot if the next still fits the register list
	//
	Current_Int_Param_Num++;
	ploc.start_offset += MTYPE_RegisterSize(SIM_INFO.int_type);
	ploc.reg = Get_Current_Int_Preg_Num (SIM_INFO.int_args);
      }

      // takes two registers:
      Current_Int_Param_Num = Current_Int_Param_Num + 2;

      //
      // TODO: depending on endianness, the First_PLOC_reg() and
      //       Next_PLOC_reg(), see targ_sim_body.h, should take
      //       either first or second of these two.
      //

      /* adjust Last_Fixed_Param in varargs case */
      if (Last_Fixed_Param < INT_MAX)
	++Last_Fixed_Param;

      break;
	
    case MTYPE_FQ:

      FmtAssert(FALSE,("FQ are passed"));
#if 0
        ++Current_Float_Param_Num;
	if (Current_Param_Num > Last_Fixed_Param && !SIM_varargs_floats) {
	    /* varargs causes float args to be int regs */
	    ploc.reg = Get_Current_Preg_Num (SIM_INFO.int_args);
	} else {
	    ploc.reg = Get_Current_Float_Preg_Num (SIM_INFO.flt_args);
	    ploc.vararg_reg = Get_Current_Preg_Num (SIM_INFO.int_args);
	}
	Current_Param_Num++;
	/* adjust Last_Fixed_Param in varargs case */
	if (Last_Fixed_Param < INT_MAX)
	    ++Last_Fixed_Param;
	Current_Float_Param_Num++;
#endif
	break;
	
    case MTYPE_C4:
    case MTYPE_C8:
    case MTYPE_CQ:

      FmtAssert(FALSE,("Complex are passed"));
#if 0
        ++Current_Float_Param_Num;
	ploc.reg = Get_Current_Float_Preg_Num (SIM_INFO.flt_args);
	Current_Param_Num++;
	/* adjust Last_Fixed_Param in varargs case */
	if (Last_Fixed_Param < INT_MAX)
	    ++Last_Fixed_Param;
	Current_Float_Param_Num++;
#endif
	break;
	
    case MTYPE_M:
        {
	  //
	  // When passed by value, parameters will be put in the
	  // register slots as many as can fit and the rest copied
	  // on stack.
	  //
	  ploc.size = TY_size (ty);

	  // Calculate the number of parameter slots needed
	  // A parameter slot is 4 bytes in size
	  //
	  INT psize = TY_size (ty) / 4;
	  // round up
	  if ((TY_size (ty) % MTYPE_RegisterSize(SIM_INFO.int_type)) != 0)
	    psize++;

	  ploc.reg = Get_Current_Int_Preg_Num (SIM_INFO.int_args);

	  // Structures are padded to a multiple of 32 bit.
	  rpad = (psize * 4) - ploc.size;

	  // Structures over 4 bytes are aligned on a 8-byte boundary
	  if (Get_Preg_Alignment(ploc.reg) == 4) {
	    //
	    // skip one slot if the next still fits the register list
	    //
	    Current_Int_Param_Num++;
	    ploc.start_offset += 4;
	    ploc.reg = Get_Current_Int_Preg_Num (SIM_INFO.int_args);
	  }

	  // takes psize registers:
	  Current_Int_Param_Num += psize;

	  /* adjust Last_Fixed_Param in varargs case */
	  if (Last_Fixed_Param < INT_MAX)
	    Last_Fixed_Param += psize - 1;

	}
	break;
	
    default:
	FmtAssert (FALSE, ("Get_Parameter_Location:  unknown mtype %s",
			   MTYPE_name(pmtype)));
  }

  Current_Offset = ploc.start_offset + ploc.size + rpad;

#if 0
  // Arthur: What is this ?? must be rotating registers ...
  if (is_output && IS_INT_PREG(PLOC_reg(ploc)))
    PLOC_reg(ploc) = Output_Base_Preg - PLOC_reg(ploc) + 32;
  else if ( ! is_output && IS_INT_PREG(PLOC_reg(ploc)))
    PLOC_reg(ploc) = Input_Base_Preg + PLOC_reg(ploc) - 32;
#endif

  return ploc;
} 


/* ====================================================================
 *           Structure stuff:
 * ====================================================================
 */
struct PSTRUCT {
    BOOL	is_struct;
    BOOL        first_call;
    BOOL        is_hfa;
    TYPE_ID     hfa_mtype;
    INT64	offset;			// offset from beginning of struct
    INT64	size;

    PSTRUCT () : is_struct (FALSE), first_call (TRUE),
		 is_hfa (FALSE), hfa_mtype (MTYPE_V),
		 offset (0), size (0) {}
};

static PSTRUCT pstruct;

#define	PSTRUCT_struct		pstruct.is_struct
#define	PSTRUCT_first_call	pstruct.first_call
#define	PSTRUCT_hfa		pstruct.is_hfa
#define	PSTRUCT_hfa_mtype	pstruct.hfa_mtype
#define	PSTRUCT_offset		pstruct.offset
#define	PSTRUCT_size		pstruct.size

/* ====================================================================
 *   Setup_Struct_Parameter_Locations
 * ====================================================================
 */
static void
Setup_Struct_Parameter_Locations (TY_IDX struct_ty)
{
    PSTRUCT_struct = ! TY_is_union (struct_ty);
    PSTRUCT_first_call = TRUE;
    PSTRUCT_hfa = Struct_Is_HFA (struct_ty, No_Simulated, PSTRUCT_hfa_mtype);
    PSTRUCT_offset = 0;
    PSTRUCT_size = TY_size (struct_ty);
}

/* ====================================================================
 *   Get_Struct_Parameter_Location
 * ====================================================================
 */
static PLOC 
Get_Struct_Parameter_Location (PLOC prev)
{
    TYPE_ID pmtype;
    PLOC next;
    INT ireg_size = MTYPE_RegisterSize(SIM_INFO.int_type);
    BOOL	onStack = (prev.reg == 0);

    if (PSTRUCT_first_call)
	PLOC_offset(next) = PLOC_offset(prev);
    else
	PLOC_offset(next) = PLOC_offset(prev) + PLOC_size(prev);

    if (PSTRUCT_offset >= PSTRUCT_size) {
      PLOC_size(next) = 0;
      return next;
    }

    if (PSTRUCT_struct && PSTRUCT_hfa &&
	!(Current_Int_Param_Num > Last_Fixed_Param && !SIM_varargs_floats)) {

      FmtAssert(FALSE,("struct is HFA ?"));

#if 0

      if (PSTRUCT_hfa_mtype == MTYPE_F4 || PSTRUCT_hfa_mtype == MTYPE_C4) {
        PLOC_size(next) = TY_size (Be_Type_Tbl (MTYPE_F4));
        PSTRUCT_offset += TY_size (Be_Type_Tbl (MTYPE_F4));
      } else {
        PLOC_size(next) = TY_size (Be_Type_Tbl (MTYPE_F8));
        PSTRUCT_offset += TY_size (Be_Type_Tbl (MTYPE_F8));
      }
      if (onStack) {
        PLOC_reg(next) = 0;
        PSTRUCT_first_call = FALSE;
      } else if (PSTRUCT_first_call) {
        PSTRUCT_first_call = FALSE;
        PLOC_reg(next) = PLOC_reg(prev);
        if (!IS_FLT_PREG(PLOC_reg(next)))
          PLOC_reg(next) = 0;
      } else if (IS_FLT_PREG(PLOC_reg(prev))) {
        PLOC_reg(next) =  PLOC_reg(prev) + PR_skip_value(SIM_INFO.flt_args);
        if (PLOC_reg(next) > PR_last_reg(SIM_INFO.flt_args)) {
          if (PSTRUCT_hfa_mtype == MTYPE_F4 || PSTRUCT_hfa_mtype == MTYPE_C4)
            PLOC_reg(next) = Get_Current_Preg_Num (SIM_INFO.int_args);
          else
            PLOC_reg(next) = 0;
        }
      } else if (Is_Int_Output_Preg(PLOC_reg(prev))) {
        PLOC_reg(next) =  PLOC_reg(prev) - PR_skip_value(SIM_INFO.int_args);
        if (!Is_Int_Output_Preg(PLOC_reg(next)))
          PLOC_reg(next) = 0;
      } else if (IS_INT_PREG(PLOC_reg(prev))) {
        PLOC_reg(next) =  PLOC_reg(prev) + PR_skip_value(SIM_INFO.int_args);
        if (!IS_INT_PREG(PLOC_reg(next)))
          PLOC_reg(next) = 0;
      }

      return next;
#endif
    }

    PLOC_size(next) = ireg_size;
    PSTRUCT_offset += ireg_size;

    if (onStack) {
      PLOC_reg(next) = 0;
      PSTRUCT_first_call = FALSE;
    } else if (PSTRUCT_first_call) {
      PSTRUCT_first_call = FALSE;
      PLOC_reg(next) = PLOC_reg(prev);
      if (!IS_INT_PREG(PLOC_reg(next)))
        PLOC_reg(next) = 0;
    } else if (IS_INT_PREG(PLOC_reg(prev))) {
      PLOC_reg(next) =  PLOC_reg(prev) + PR_skip_value(SIM_INFO.int_args);
      if (!IS_INT_PREG(PLOC_reg(next)))
        PLOC_reg(next) = 0;
    }

    return next;
} // Get_Struct_Parameter_Location

/* ====================================================================
 *   Get_Vararg_Parameter_Location
 *
 *   Iterate over vararg non-fixed parameters 
 * ====================================================================
 */
static PLOC
Get_Vararg_Parameter_Location (
  PLOC prev
)
{
  PLOC next;

  next.reg = Get_Current_Int_Preg_Num (SIM_INFO.int_args);
  Current_Int_Param_Num++;

  if (next.reg > PR_last_reg(SIM_INFO.int_args))
  {
    next.reg = 0;
    next.size = 0;
  }
  else
  {
    next.size = MTYPE_RegisterSize(SIM_INFO.int_type);
  }
  /* use Last_Param_Offset in case last fixed arg had padding */
  /*
  next.start_offset = Last_Param_Offset;
  Last_Param_Offset = next.start_offset + next.size;
  */

  next.start_offset = Current_Offset;
  Current_Offset = next.start_offset + next.size;

  return next;
}

/* ====================================================================
 *   Init_Targ_Sim
 * ====================================================================
 */

BOOL Is_Caller_Save_GP;  /* whether GP is caller-save */

//INT Formal_Save_Area_Size = DEFAULT_FORMAL_SAVE_AREA_SIZE;

// Used to account for push/pop area that may be written before the
// stack pointer is set:
// INT Stack_Offset_Adjustment;

extern void 
Init_Targ_Sim (void)
{
  Is_Caller_Save_GP = SIM_caller_save_gp;
}

