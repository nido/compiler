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


#ifndef targ_sim_core_INCLUDED
#define targ_sim_core_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

/*
 * This defines the ABI subprogram interface,
 * and is used to determine how parameters and results are passed.
 * The global variable Target_ABI is used to determine which
 * calling convention to use.
 * The register values are the PREG offsets, so these values can be
 * used in WHIRL.
 */

#define Preg_Offset_Is_Int(n) \
        ((n) >= Int_Preg_Min_Offset && (n) <= Int_Preg_Max_Offset)
#define Preg_Offset_Is_Ptr(n) \
        ((n) >= Ptr_Preg_Min_Offset && (n) <= Ptr_Preg_Max_Offset)
#define Preg_Offset_Is_Float(n) \
        ((n) >= Float_Preg_Min_Offset && (n) <= Float_Preg_Max_Offset)
#define Preg_Offset_Is_Branch(n) \
        ((n) >= Branch_Preg_Min_Offset && (n) <= Branch_Preg_Max_Offset)
#define Preg_Offset_Is_Fcc(n) \
        ((n) >= Fcc_Preg_Min_Offset && (n) <= Fcc_Preg_Max_Offset)
#define Preg_Is_Dedicated(n) (n <= Last_Dedicated_Preg_Offset)

/* return whether preg is a return preg */
BE_EXPORTED extern BOOL Is_Return_Preg (PREG_NUM preg);

/* return whether preg is an output preg */
BE_EXPORTED extern BOOL Is_Int_Output_Preg (PREG_NUM preg);

/* return whether preg is an input parameter preg */
BE_EXPORTED extern BOOL Is_Formal_Preg (PREG_NUM preg);

/* define an enumeration of the different levels of mtypes/whirl
 * that we can use for returns */
typedef enum {
	Use_Simulated,		/* use simulated types */
	Complex_Not_Simulated, 	/* lower complex but not quad */
	No_Simulated		/* all lowered to machine-level */
} Mtype_Return_Level;

class RETURN_INFO {
private:
  mINT8    count;
  BOOL     return_via_first_arg;
  TYPE_ID  mtype [MAX_NUMBER_OF_REGISTERS_FOR_RETURN];
  PREG_NUM preg  [MAX_NUMBER_OF_REGISTERS_FOR_RETURN];
public:
  friend inline mINT8 RETURN_INFO_count (const RETURN_INFO&);
  friend inline BOOL  RETURN_INFO_return_via_first_arg (const RETURN_INFO&);
  friend inline TYPE_ID  RETURN_INFO_mtype (const RETURN_INFO&, INT32);
  friend inline PREG_NUM RETURN_INFO_preg (const RETURN_INFO&, INT32);
  friend BE_EXPORTED RETURN_INFO Get_Return_Info (TY_IDX rtype, Mtype_Return_Level level);
};

#if __GNUC__ >= 4
    // [HK] name-injection of friend declarations is no longer allowed in gcc-4.1.x.
    // a declaration of Get_Return_Info needs to be present outside of the scope 
    // of class RETURN_INFO
BE_EXPORTED RETURN_INFO Get_Return_Info (TY_IDX, Mtype_Return_Level);
#endif

inline mINT8
RETURN_INFO_count (const RETURN_INFO& info) { return info.count; }

inline BOOL 
RETURN_INFO_return_via_first_arg (const RETURN_INFO& info)
{
  return info.return_via_first_arg;
}

inline TYPE_ID
RETURN_INFO_mtype (const RETURN_INFO& info, INT32 i) { return info.mtype [i]; }

inline PREG_NUM
RETURN_INFO_preg (const RETURN_INFO& info, INT32 i) { return info.preg [i]; }

/* This routine figures out the mtypes of the return registers that are
 * used for returning an object of the given type.
 * This returns the mtypes to use for the CALL opcode in high-level whirl.
 * This means that returns of simulated objects, like FQ, are just shown
 * as returning FQ, which will later be split into F8F8.
 * However, structures that return in registers are specified explicitly.
 * If a register is unused, MTYPE_V is returned.
 */
BE_EXPORTED extern void Get_Return_Mtypes (
  TY_IDX rtype,		/* The result type */
  Mtype_Return_Level level,	/* whether to lower the mtypes */
  TYPE_ID *mreg1,	/* out: mtype for result register 1 */
  TYPE_ID *mreg2);	/* out: mtype for result register 2 */

/* This routine figures out which return registers are to be used
 * for returning an object with the given mtypes.
 * Preg 0 is returned for unused registers.
 * It is assumed that the mtypes will be determined by calling
 * Get_Return_Mtypes.
 */
BE_EXPORTED extern void Get_Return_Pregs (
  TYPE_ID mreg1,	/* in:  mtype for result register 1 */
  TYPE_ID mreg2,	/* in:  mtype for result register 2 */
  PREG_NUM *rreg1,	/* out: result register 1 */
  PREG_NUM *rreg2);	/* out: result register 2 */

#ifdef TARG_ST
/* [SC] Return the type of a nested function trampoline for the target. */
BE_EXPORTED extern TYPE_ID Get_Nested_Fn_Trampoline_Type (void);
#endif

/* PLOC contains information about the location of a parameter.
 * If reg == 0, then the parameter is stored on the stack. 
 * For -DEBUG:varargs_prototypes=off a floating point parameter is
 * passed in both the floating point register and integer register.
 */
typedef struct {
	PREG_NUM reg;
	INT32 formal_offset;
	INT32 upformal_offset;
	INT32 size;
	PREG_NUM vararg_reg;
#ifdef TARG_ST
  // Arthur: we want to calculate padding in one place, the ABI
  //         routines.
  INT32 lpad;
  INT32 rpad;
  INT32 lpad_overlap;	// Amount of lpad allocated to the previous segment when the object overlaps

  // [JV] To say if the parameter is passed by reference.
  BOOL  by_reference;
#endif
} PLOC;

#define PLOC_reg(p)		    ((p).reg)
#define PLOC_vararg_reg(p)	    ((p).vararg_reg)
#define PLOC_on_stack(p)	    ((p).reg == 0)
#define PLOC_offset(p)              ((PLOC_on_stack(p) ? (p).upformal_offset : (p).formal_offset) + 0)
#define PLOC_set_offset(p,o)        if(PLOC_on_stack(p)) { (p).upformal_offset = (o); } else { (p).formal_offset = (o); }
#define PLOC_formal_offset(p)       ((p).formal_offset)
#define PLOC_upformal_offset(p)     ((p).upformal_offset)
#define PLOC_size(p)		    ((p).size)
#define PLOC_is_empty(p)	    (PLOC_size(p) == 0)
#define PLOC_is_nonempty(p)	    (PLOC_size(p) != 0)
#ifdef TARG_ST
#define PLOC_lpad(p)                ((p).lpad)
#define PLOC_rpad(p)                ((p).rpad)
#define PLOC_by_reference(p)        ((p).by_reference)
#define PLOC_clear(p)		    { memset(&(p), 0, sizeof(PLOC)); }
#define PLOC_lpad_overlap(p)		    ((p).lpad_overlap)
#endif

#ifdef TARG_ST
BE_EXPORTED extern PLOC Setup_Input_Parameter_Locations (TY_IDX pu_type, BOOL first_hidden_param_is_lowered = TRUE);
#else
BE_EXPORTED extern PLOC Setup_Input_Parameter_Locations (TY_IDX pu_type);
#endif
BE_EXPORTED extern PLOC Get_Input_Parameter_Location (TY_IDX ptype);

BE_EXPORTED extern PLOC First_Input_PLOC_Reg (PLOC ploc, TY_IDX parm_ty);
BE_EXPORTED extern PLOC Next_Input_PLOC_Reg (void);

#ifdef TARG_ST
BE_EXPORTED extern PLOC Setup_Output_Parameter_Locations (TY_IDX pu_type, BOOL first_hidden_param_is_lowered = TRUE);
#else
BE_EXPORTED extern PLOC Setup_Output_Parameter_Locations (TY_IDX pu_type);
#endif
BE_EXPORTED extern PLOC Get_Output_Parameter_Location (TY_IDX ptype);

BE_EXPORTED extern PLOC First_Output_PLOC_Reg (PLOC ploc, TY_IDX parm_ty);
BE_EXPORTED extern PLOC Next_Output_PLOC_Reg (void);

/* Iterate over implicit vararg non-fixed register parameters */
BE_EXPORTED extern PLOC Get_Vararg_Input_Parameter_Location (PLOC prev);
BE_EXPORTED extern PLOC Get_Vararg_Output_Parameter_Location (PLOC prev);

/* Return the size of Preg, based on the register and the abi. 
 */
BE_EXPORTED extern INT32 Get_Preg_Size (PREG_NUM p);

/*
 * When we have a structure or register-pair,
 * we need to break it into register-sized chunks.
 * So we first call Setup_Struct_Parameter_Locations with the structure TY,
 * then we iterate with Get_Struct_Parameter_Location until it
 * returns PLOC_is_empty.
 */
BE_EXPORTED extern BOOL Is_Struct_Input_Parameter (TY_IDX struct_ty);
BE_EXPORTED extern BOOL Is_Struct_Output_Parameter (TY_IDX struct_ty);
BE_EXPORTED extern void Setup_Struct_Input_Parameter_Locations (TY_IDX struct_ty);
BE_EXPORTED extern PLOC Get_Struct_Input_Parameter_Location ( PLOC prev );
BE_EXPORTED extern void Setup_Struct_Output_Parameter_Locations (TY_IDX struct_ty);
BE_EXPORTED extern PLOC Get_Struct_Output_Parameter_Location ( PLOC prev );

/*
 * TY_mtype() is not correct for structures/arrays.
 * This function will return a corrected TYPE_ID
 */
BE_EXPORTED extern TYPE_ID Fix_TY_mtype(TY_IDX);

/*
 * The following variables give info about the calling convention,
 * and are set by Init_Targ_Sim()
 */

BE_EXPORTED extern BOOL Is_Caller_Save_GP;	/* whether GP is caller-save */

/* Amount of space available in stack frame to save register formals. */
BE_EXPORTED extern INT Max_Formal_Save_Area_Size;

/* Amount of space used in stack frame to save register formals. */
BE_EXPORTED extern INT Formal_Save_Area_Size;

/* Adjust all stack offsets by this amount,
 * which represents the space available to subsequent frames. */
BE_EXPORTED extern INT Stack_Offset_Adjustment;

extern void Init_Targ_Sim (void);	/* initialize the info */


//TB Extend PREG registers to extension
BE_EXPORTED extern   INT Get_Static_Last_Dedicated_Preg_Offset(void);
#define Last_Dedicated_Preg_Offset Get_Last_Dedicated_Preg_Offset_Func()
BE_EXPORTED extern   INT Get_Last_Dedicated_Preg_Offset_Func(void);
BE_EXPORTED extern   void Set_Last_Dedicated_Preg_Offset(INT offset);

// PREG_IDX == PREG_NUM - Last_Dedicated_Preg_Offset

inline PREG_IDX
Get_Preg_Idx (PREG_NUM n)       { return n - Last_Dedicated_Preg_Offset; }
inline PREG_NUM
Get_Preg_Num (PREG_IDX i)       { return i + Last_Dedicated_Preg_Offset; }


#ifndef PUSH_RETURN_ADDRESS_ON_STACK
#define PUSH_RETURN_ADDRESS_ON_STACK FALSE
#endif

#ifndef PUSH_FRAME_POINTER_ON_STACK
#define PUSH_FRAME_POINTER_ON_STACK  FALSE
#endif

#ifdef __cplusplus
}
#endif
#endif /* targ_sim_core_INCLUDED */
