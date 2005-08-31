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


//-*-c++-*-
/* ====================================================================
 * ====================================================================
 *
 * Module: tn.h
 *
 * Description:
 *
 * Define the TN (temporary name) data structure and relevant utility
 * operations.
 *
 * Exported types:
 *
 *   TN
 *
 *     The Temporary Name (TN) structure describes the operands and result
 *     of OPs. A TN can be used to describe either a register or a constant.
 *     The TN_flags(tn) field is applicable to all TNs and is used to determine
 *     the correct variant of the TN. The TN_size(tn) field is also applicable
 *     to all TNs and gives the size of the operand/result in bytes.
 *     All the other fields are applicable in specific contexts as described 
 *     below.
 *
 *     Register TNs:
 *
 *       The flag TN_is_register(tn) indicates if a TN is a register TN.
 *
 *       The following fields are valid only for register TNs:
 *
 *	   TN_number(tn) 
 *	     numeric id associated with a register TN.
 *
 *	   TN_register_class(tn)
 *	     The TN_register_class(tn) indicates the register class of 
 * 	     the register TN.
 *	
 *	   TN_register(tn)
 *           The TN_register(tn) indicates the register assigned to the
 *	     register TN.
 *
#ifdef TARG_ST
 *         TN_nhardregs(tn)
 *           The number of consecutive machine registers that must be
 *           allocated to hold TN.  If TN_register is not REGISTER_UNDEFINED,
 *           then the tn occupies registers
 *            [ TN_register : TN_register + TN_nhardregs - 1 ]
 *
 *         TN_registers(tn)
 *           If TN_register is not REGISTER_UNDEFINED,
 *           the set of machine registers that are assigned to TN.
 *           IF TN_register is REGISTER_UNDEFINED, returns an empty set.
 *
#endif
 *	   TN_register_and_class(tn)
 *	     TN_register_class and TN_register combined into a single
 *	     scalar type for efficiency in comparing registers and classes.
 *
 *	   TN_save_reg(tn)
 *	     If the tn is a save-tn, this field gives the register 
 * 	     that the save-tn is saving/restoring. For all other tns,
 *	     this is set to REGISTER_UNDEFINED.
 *
 *	   TN_spill(tn)
 *	     These fields point to spill information for register TNs.
 *
 *	   TN_is_rematerializable(tn)
 *	     If this attribute is true, the <tn> contains a constant
 *	     value. It can be rematerialized using the TN_home field.
 *
 *	   TN_home(tn)
 *	     This field is valid only if TN_is_rematerializable is true.
 *	     It points to the WHIRL node to use in rematerializing the
 *	     constant value of the <tn>.
 *
 *	   TN_is_dedicated(tn)
 *	     This macro indicates that the register TN corresponds to a 
 *           physical register.
 *
 *	   TN_is_float(tn)
 *	     TN is for a floating point register.
 *
 *	   TN_is_fpu_int(tn)
 *	     TN is for an integer value in an FP register.
 *
 *	   TN_is_fcc_register(tn)
 *	     TN is an fcc (FP condition code) register.
 *
 *	   TN_is_save_reg(tn)
 *	     TN is a save-tn for a callee-saved register.
 *
 *
 *    Constant TNs:
 *
 *      The flag TN_is_constant(tn) indicates if a TN is a constant TN. A 
 *      constant TN can be one of the following sub-types:
 * 
 *      1. Integer constant:
 *	  TN_has_value(tn) indicates if a constant TN is an integer
 *	  constant. The value of the constant is given by TN_value(tn).
 *
 *      2. Symbol TN:
 *	  TN_is_symbol(tn) indicates if a constant TN representing a 
 *	  "symbol+offset" expression. The symbol is given by TN_var(tn)
 *	  and the offset by TN_offset(tn). The TN_relocs(tn) field
 *	  indicates additional relocations/operations to be applied 
 *	  to the "symbol+offset".
 *
 *      3. Label TN:
 *	  TN_is_label(tn) indicates if a constant TN represents a label.
 *	  The label is indicated by TN_label(tn). The TN_offset(tn) is
 *	  used to indicate a byte offset from the label.
 *
 *      4. Enum TN:
 *	  TN_is_enum(tn) indicates if a constant TN represents an enum.
 *	  The enum value is indicated by TN_enum(tn).
 *	   
 * Utility functions:
 *
 *   void TN_Allocate_Register( TN *tn, REGISTER reg )
 *	Register TNs may be allocated to a register. This function
 *	sets <tn>'s _register to be <reg>.  This is the act of register
 *	assignment.
 *
 *   BOOL TN_Is_Allocatable( TN *tn )
 *	TRUE for just those TNs that can be assigned a register.  This
 *	excludes all kind of things that must NOT be register allocated,
 *	such as constants, zeros, dedicated TNs. 
 *
 *   OP *TN_Reaching_Value_At_Op(TN *tn, OP *op, DEF_KIND *kind, 
 *				 BOOL reaching_def)
 *      The routine can be used to find (1) the reaching definition of
 *      operand <tn> of <op>, or (2) the following use of the result <tn>
 *	of <op>. The knob is controlled by the flag <reaching_def>.
 *      If <reaching_def> is TRUE, find the reaching definition of the <tn>, 
 *	if <reaching_def> is FALSE, find the reaching use of the <tn>. 
 *	If none is found, return NULL. <kind> determines the definition kind 
 *	property returned by the function, i.e if the value definition 
 *      of <tn> is VAL_KNOWN, VAL_UNKNOWN, VAL_COND_DEF, VAL_COND_USE, .. etc.
 *
 *	Note that not all reaching definitions are found, dominator
 *	information is necessary to handle more cases and it's not
 *	proven to be worth it.
 *
 *   BOOL TN_Value_At_Op( TN *tn, OP *use_op, INT64 *val )
 *	If it can be determined that <tn> has a known integer value, as 
 *	referenced by <use_op>, return the value via the  out parameter <val>. 
 *	<use_op> may be NULL, in which case if <tn> is not constant,
 *	no attempt is made to find a reaching definition. The return 
 *	value indicates if we were able to determine the value.
 *
 *   TN *Build_Dedicated_TN( REGISTER_CLASS rclass, REGISTER reg, INT size )
 *	Create a dedicated TN for register 'reg' in class 'rclass'.
 *	NOTE: Currently this returns the same TN when called multiple
 *	times with the same 'rclass', 'reg' pair. This will eventually
 *	change to build a new TN on each call.
 *	If the 'size' param is 0, then use default dedicated tn for regclass.
 *	Otherwise, for float rclass create separate TN for each float size.
 *	The 'size' is specified in bytes.
 *
 *   void Init_Dedicated_TNs( void )
 *	This routine should be called once per compilation before we
 *	process any PUs. It initializes the dedicated TNs.
 *	NOTE: This will eventually go away, see Build_Dedicated_TN.
 *
 *   BOOL TN_is_dedicated_class_and_reg( TN *tn, UINT16 class_n_reg )
 *	Returns a boolean to indicate if <tn> is a dedicated TN
 *	for the specified class and register.
 *
 * ====================================================================
 * ====================================================================
 */

#ifndef tn_INCLUDED
#define tn_INCLUDED

/* Define the type before any includes so they can use it. */
typedef struct tn TN;

struct TN_LIST;

/* #include "tn_list.h" */
#include  "defs.h"
#include  "errors.h"
#include  "mempool.h"
#include "register.h"
#include "symtab.h"

#include "targ_isa_enums.h"

/* Target-specific TN info */
#include "targ_tn.h"

class WN;

/* Define the TN number type: */
typedef INT32 TN_NUM;	/* Individual objects */
typedef mINT32 mTN_NUM;	/* Table components */
#define TN_NUM_MAX	INT32_MAX


/* Define the TN structure: (NOTE: keep the size <= 16 bytes) */

struct tn {
  /* offset 0 */
  union {
    INT64	value;		/* TN_has_value: Integer constant */
    INT64	offset;		/* Offset from symbol (constant) */
    struct {
      mTN_NUM	number;		/* The ID of the register TN */
      CLASS_REG_PAIR save_creg;	/* if save_tn, the corresponding save_reg */
      CLASS_REG_PAIR class_reg; /* Dedicated/allocated register ID
				   and register class (see register.h) */
    } reg_tn;
  } u1;
  /* offset 8 */
  mUINT16	flags;		/* Attribute flags */
  mUINT8	relocs;		/* Relocation flags (for symbol TNs) */
  mUINT8	size;		/* Size of the TN in bytes (must be <= 16) */
  /* offset 12 */
  union {
    LABEL_IDX	label;		/* Label constant */
    ISA_ENUM_CLASS_VALUE ecv;	/* Enum constant */
    ST		*var;		/* Symbolic constant (user variable) */
    union {			/* Spill location */
      ST	*spill;		/* ...for register TN */
      WN        *home;		/* Whirl home if rematerializable */
    } u3;
  } u2;
};


/* Define the TN_flags masks: */
#define TN_CONSTANT	0x0001	/* Constant value, numeric or label */
#define TN_HAS_VALUE	0x0002	/* Constant numeric value */
#define TN_LABEL	0x0004	/* Constant label value */
#define TN_TAG		0x0008	/* Constant tag value */
#define TN_SYMBOL	0x0010	/* References symbol table element */
#define TN_FLOAT	0x0020	/* => tn for a Floating-Point value */
#define TN_DEDICATED	0x0040	/* Dedicated, register if reg!=NULL */
#define TN_FPU_INT	0x0080	/* Int value in FPU (also TN_FLOAT) */
#define TN_GLOBAL_REG	0x0100	/* TN is a GTN (global register) */
#define TN_IF_CONV_COND 0x0200      /* TN is an if conversion conditinal */
#define TN_REMATERIALIZABLE 0x0400	/* TN is rematerializable from whirl */
#define TN_GRA_HOMEABLE 0x0800      /* TN can be homed by gra */
#define TN_ENUM		0x1000      /* Constant enum value */
#define TN_GRA_CANNOT_SPLIT 0x2000  /* its live range not to be split by GRA */

#define CAN_USE_TN(x)	(x)

/* Define the access functions: */
#define     TN_flags(t)		(CAN_USE_TN(t)->flags)
#define Set_TN_flags(t,x)	(CAN_USE_TN(t)->flags = (x))
/* define TN_is_{constant,register} ahead of time */
#define       TN_is_constant(r)	(TN_flags(r) &   TN_CONSTANT)
#define   Set_TN_is_constant(r)	(TN_flags(r) |=  TN_CONSTANT)
#define       TN_is_register(r)	(!TN_is_constant(r))

inline TN * CAN_USE_REG_TN (const TN *t)
{
	Is_True(TN_is_register(t), ("not a register tn"));
	return (TN*)t;
}

#define     TN_relocs(t)	(CAN_USE_TN(t)->relocs)
#define Set_TN_relocs(t,x)	(CAN_USE_TN(t)->relocs = (x))
#define     TN_size(t)		(CAN_USE_TN(t)->size+0)
#define Set_TN_size(t,x)	(CAN_USE_TN(t)->size = (x))
#define     TN_number(t)	(CAN_USE_REG_TN(t)->u1.reg_tn.number+0)
#define Set_TN_number(t,x)	(CAN_USE_REG_TN(t)->u1.reg_tn.number = (x))
#define	    TN_class_reg(t)	(CAN_USE_REG_TN(t)->u1.reg_tn.class_reg)
#define	Set_TN_class_reg(t,x)	(CAN_USE_REG_TN(t)->u1.reg_tn.class_reg = (x))
#define     TN_register(t)	\
		(CLASS_REG_PAIR_reg(CAN_USE_REG_TN(t)->u1.reg_tn.class_reg)+0)
#define Set_TN_register(t,x)	\
		(Set_CLASS_REG_PAIR_reg(CAN_USE_REG_TN(t)->u1.reg_tn.class_reg,(x)))
#define     TN_register_class(t) \
		(CLASS_REG_PAIR_rclass(CAN_USE_REG_TN(t)->u1.reg_tn.class_reg))
#define Set_TN_register_class(t,x) \
		(Set_CLASS_REG_PAIR_rclass(CAN_USE_REG_TN(t)->u1.reg_tn.class_reg,(x)))
#define     TN_register_and_class(t) \
		(CLASS_REG_PAIR_class_n_reg(CAN_USE_REG_TN(t)->u1.reg_tn.class_reg)+0)
#define Set_TN_register_and_class(t,x) \
		(Set_CLASS_REG_PAIR_class_n_reg(CAN_USE_REG_TN(t)->u1.reg_tn.class_reg,(x)))
#define     TN_save_creg(t)	(CAN_USE_REG_TN(t)->u1.reg_tn.save_creg)
#define     TN_save_reg(t)	(CLASS_REG_PAIR_reg(TN_save_creg(t))+0)
#define     TN_save_rclass(t)	(CLASS_REG_PAIR_rclass(TN_save_creg(t)))
#define Set_TN_save_creg(t,x)	(CAN_USE_REG_TN(t)->u1.reg_tn.save_creg = (x))
#define     TN_is_save_reg(t)	(!CLASS_REG_PAIR_EqualP(TN_save_creg(t),CLASS_REG_PAIR_undef))
#define     TN_spill(t)		(CAN_USE_TN(t)->u2.u3.spill)
#define Set_TN_spill(t,x)	(CAN_USE_TN(t)->u2.u3.spill = (x))
#define     TN_spill_is_valid(t)(TN_is_register(t) && !(TN_is_rematerializable(t) || TN_is_gra_homeable(t)))
#define     TN_has_spill(t)	(TN_spill_is_valid(t) && (TN_spill(t) != NULL))
#define     TN_value(t)		(CAN_USE_TN(t)->u1.value)
#define Set_TN_value(t,x)	(CAN_USE_TN(t)->u1.value = (x))
#define     TN_offset(t)	(CAN_USE_TN(t)->u1.offset)
#define Set_TN_offset(t,x)	(CAN_USE_TN(t)->u1.offset = (x))
#define     TN_label(t)		(CAN_USE_TN(t)->u2.label)
#define Set_TN_label(t,x)	(CAN_USE_TN(t)->u2.label = (x))
#define     TN_enum(t)		(CAN_USE_TN(t)->u2.ecv)
#define Set_TN_enum(t,x)	(CAN_USE_TN(t)->u2.ecv = (x))
#define     TN_var(t)		(CAN_USE_TN(t)->u2.var)
#define Set_TN_var(t,x)		(CAN_USE_TN(t)->u2.var = (x))
#define     TN_home(t)		(CAN_USE_TN(t)->u2.u3.home)
#define Set_TN_home(t,x)        (CAN_USE_TN(t)->u2.u3.home = (x))

/* Define the TN_flags access functions: */
#define       TN_has_value(r)	(TN_flags(r) &   TN_HAS_VALUE)
#define   Set_TN_has_value(r)	(TN_flags(r) |=  TN_HAS_VALUE)
#define       TN_is_label(r)	(TN_flags(r) &   TN_LABEL)
#define   Set_TN_is_label(r)	(TN_flags(r) |=  TN_LABEL)
#define       TN_is_tag(r)	(TN_flags(r) &   TN_TAG)
#define   Set_TN_is_tag(r)	(TN_flags(r) |=  TN_TAG)
#define       TN_is_symbol(r)	(TN_flags(r) &   TN_SYMBOL)
#define   Set_TN_is_symbol(r)	(TN_flags(r) |=  TN_SYMBOL)
#define       TN_is_enum(r)	(TN_flags(r) &   TN_ENUM)
#define   Set_TN_is_enum(r)	(TN_flags(r) |=  TN_ENUM)

#define       TN_is_float(x)    (TN_flags(x) &   TN_FLOAT)
#define   Set_TN_is_float(x)    (TN_flags(x) |=  TN_FLOAT)
#define       TN_is_fpu_int(x)  (TN_flags(x) &   TN_FPU_INT)
#define   Set_TN_is_fpu_int(x)  (TN_flags(x) |=  TN_FPU_INT)
#define Reset_TN_is_fpu_int(x)  (TN_flags(x) &=  ~TN_FPU_INT)
#define       TN_is_global_reg(x) (TN_flags(x) &   TN_GLOBAL_REG)
#define   Set_TN_is_global_reg(x) (TN_flags(x) |=  TN_GLOBAL_REG)
#define Reset_TN_is_global_reg(x) (TN_flags(x) &= ~TN_GLOBAL_REG)
#define      TN_is_dedicated(r)	(TN_flags(r) &   TN_DEDICATED)
#define  Set_TN_is_dedicated(r)	(TN_flags(r) |=  TN_DEDICATED)
#define Reset_TN_is_dedicated(r) (TN_flags(r)&= ~TN_DEDICATED)

#define      TN_is_if_conv_cond(r)  (TN_flags(r) &   TN_IF_CONV_COND)
#define  Set_TN_is_if_conv_cond(r)  (TN_flags(r) |=  TN_IF_CONV_COND)
#define Reset_TN_is_if_conv_cond(r) (TN_flags(r) &= ~TN_IF_CONV_COND)

#define      TN_is_rematerializable(r)  (TN_flags(r) &   TN_REMATERIALIZABLE)
#define  Set_TN_is_rematerializable(r)  (TN_flags(r) |=  TN_REMATERIALIZABLE)
#define Reset_TN_is_rematerializable(r) (TN_flags(r) &= ~TN_REMATERIALIZABLE)

#define      TN_is_gra_homeable(r)  (TN_flags(r) &   TN_GRA_HOMEABLE)
#define  Set_TN_is_gra_homeable(r)  (TN_flags(r) |=  TN_GRA_HOMEABLE)
#define Reset_TN_is_gra_homeable(r) (TN_flags(r) &= ~TN_GRA_HOMEABLE)

#define      TN_is_gra_cannot_split(r)  (TN_flags(r) &   TN_GRA_CANNOT_SPLIT)
#define  Set_TN_is_gra_cannot_split(r)  (TN_flags(r) |=  TN_GRA_CANNOT_SPLIT)

/* ====================================================================
 *
 * External variables.
 *
 * ====================================================================
 */

/* The register TNs are in a table named TNvec, indexed by their TN 
 * numbers in the range 1..Last_TN.  The first part of the table, the 
 * range 1..Last_Dedicated_TN, consists of TNs for various dedicated 
 * purposes (e.g. stack pointer, zero, physical registers).  It is 
 * followed by TNs for user variables and compiler temporaries, in the 
 * range First_Regular_TN..Last_TN.
 */
extern TN_NUM Last_Dedicated_TN;/* The last dedicated TN number */
extern TN_NUM First_Regular_TN;	/* after all the preallocated TNs */
extern TN_NUM Last_TN;		/* The last allocated TN number */
extern TN_NUM First_REGION_TN;	/* The first non-dedicated TN in the current REGION */
extern TN **TN_Vec;		/* Mapping from number to TN */
#define TNvec(i) TN_Vec[i]

// The following are special-purpose TNs required in the compiler for
// specific purposes.  
// NOTE: Don't use these TNs directly in comparisons with other TNs.
//       Instead use the TN_is_xx_reg macros. This allows the 
//       comparisons to work with renaming of dedicated registers.
//
extern  TN *Zero_TN;		// Zero register TN
extern 	TN *FP_TN;		// Frame Pointer
extern  TN *SP_TN;		// Stack Pointer
extern	TN *RA_TN;		// Return address register
extern  TN *Ep_TN;		// Entry point TN
extern	TN *GP_TN;		// Global pointer register
extern	TN *Pfs_TN;		// Previous Function State TN
extern	TN *LC_TN;		// Loop Counter TN
extern	TN *EC_TN;		// Epilog Counter TN
extern	TN *True_TN;		// TN for true condition (predicate)
extern  TN *FZero_TN;		// Floating zero (0.0) register TN
extern  TN *FOne_TN;		// Floating one (1.0) register TN
extern  TN *Link_TN;            // Link TN for indirect branching
extern  TN *RS_TN;              // TN for returning structs by value
#ifdef TARG_ST                  // [SC] TLS supprt 
extern TN *TP_TN;               // Thread Pointer
#endif

/* ====================================================================
 * Prototypes of external routines.
 * ====================================================================
 */

/* Intialize the dedicated TNs at the start of the compilation. */
extern  void Init_Dedicated_TNs (void);

/* Initialize the TN data structure at the start of each PU. */ 
extern	void Init_TNs_For_PU (void);
/* Initialize the TN data structure at the start of each REGION. */ 
extern	void Init_TNs_For_REGION (void);

/* TN generation: */

/* The following set of routines can be used only for register TNs */

CG_EXPORTED extern TN* Gen_Register_TN (ISA_REGISTER_CLASS rclass, INT size);

CG_EXPORTED extern  TN *Build_Dedicated_TN ( ISA_REGISTER_CLASS rclass, REGISTER reg, INT size);

/* Macros to check if a TN is a particular dedicated register. */
#define TN_is_sp_reg(r)	   (TN_register_and_class(r) == CLASS_AND_REG_sp)
#define TN_is_gp_reg(r)	   (TN_register_and_class(r) == CLASS_AND_REG_gp)
#define TN_is_ep_reg(r)	   (TN_register_and_class(r) == CLASS_AND_REG_ep)
#define TN_is_fp_reg(r)	   (TN_register_and_class(r) == CLASS_AND_REG_fp)
#define TN_is_ra_reg(r)	   (TN_register_and_class(r) == CLASS_AND_REG_ra)
#define TN_is_rs_reg(r)	   (TN_register_and_class(r) == CLASS_AND_REG_rs)
#define TN_is_zero_reg(r)  (TN_register_and_class(r) == CLASS_AND_REG_zero)
#define TN_is_static_link_reg(r) (TN_register_and_class(r) == CLASS_AND_REG_static_link)
#define TN_is_link_reg(r) (TN_register_and_class(r) == CLASS_AND_REG_link)
#define TN_is_pfs_reg(r)   (TN_register_and_class(r) == CLASS_AND_REG_pfs)
#define TN_is_lc_reg(r)   (TN_register_and_class(r) == CLASS_AND_REG_lc)
#define TN_is_ec_reg(r)   (TN_register_and_class(r) == CLASS_AND_REG_ec)
#define TN_is_true_pred(r) (TN_register_and_class(r) == CLASS_AND_REG_true)
#define TN_is_fzero_reg(r) (TN_register_and_class(r) == CLASS_AND_REG_fzero)
#define TN_is_fone_reg(r)  (TN_register_and_class(r) == CLASS_AND_REG_fone)

// Check if the TN is either a constant zero or the zero register TN.
// If you know it is a register TN, use TN_is_zero_reg directly.
inline BOOL TN_is_zero (const TN *r) 
{
  return ((TN_has_value(r) && TN_value(r) == 0) || (TN_is_register(r) && TN_is_zero_reg(r)));
}

#ifdef TARG_ST
inline INT TN_nhardregs (const TN *tn)
{
  REGISTER reg = TN_register(tn);
  ISA_REGISTER_CLASS rclass = TN_register_class(tn);
  if (reg == REGISTER_UNDEFINED) {
    reg = REGISTER_CLASS_last_register(rclass);
  }
  INT sz = (REGISTER_bit_size(rclass, reg) + 7) / 8;
  return (TN_size(tn) + sz - 1)/ sz;
}

inline REGISTER_SET TN_registers (const TN *tn)
{
  REGISTER reg = TN_register(tn);
  if (reg == REGISTER_UNDEFINED) {
    return REGISTER_SET_EMPTY_SET;
  } else {
    return REGISTER_SET_Range (reg, reg + TN_nhardregs (tn) - 1);
  }
}
#endif

// Returns TRUE if the TN represents a hardwired registers.
inline BOOL TN_is_const_reg(const TN *r)
{
  return (TN_is_register(r) && 
	  TN_is_dedicated(r) &&
	  (TN_is_zero_reg(r) || 
	   TN_is_true_pred(r) ||
	   TN_is_fzero_reg(r) ||
	   TN_is_fone_reg(r)));
}

/*
   TNs_Are_Equivalent
   Returns TRUE if TNs have the same base TN number or that they are
   assigned the same register.
*/
inline BOOL TNs_Are_Equivalent(TN *tn1, TN *tn2) 
{
  if ( (tn1 == tn2 ||
	(TN_register(tn1) != REGISTER_UNDEFINED &&
	 TN_register(tn2) != REGISTER_UNDEFINED &&
	 TN_register_and_class(tn1) == TN_register_and_class(tn2))))
    return TRUE;

  return FALSE;
}

/* Build a TN that matches the register class */
inline TN* Build_RCLASS_TN (ISA_REGISTER_CLASS rclass)
{
#ifdef TARG_ST
  // Ensure size is > 0 !
  return Gen_Register_TN (rclass, 
	(REGISTER_bit_size(rclass, REGISTER_CLASS_last_register(rclass))+7)/8);
#else
	return Gen_Register_TN (rclass, 
		(REGISTER_bit_size(rclass, 
		 REGISTER_CLASS_last_register(rclass))/8) );
#endif
}

// ---------------------------------------------------------------------
inline TN *Build_TN_Like(TN *tn)
{
  FmtAssert(TN_is_register(tn),("Build_TN_Like called on non-register tn"));

  return Gen_Register_TN (TN_register_class(tn), TN_size(tn) );
}

// ---------------------------------------------------------------------
inline TN *Build_TN_Of_Mtype(TYPE_ID mtype)
{
  ISA_REGISTER_CLASS rc = Register_Class_For_Mtype(mtype);
#ifdef TARG_ST
  // FdF 20050830: MTYPE_RegisterSize(mtype) returns 0 for MTYPE_B,
  // while size is expected to be > 0.
  INT size = MTYPE_RegisterSize(mtype);
  if (size == 0) size = 1;
  return Gen_Register_TN (rc, size);
#else
  return Gen_Register_TN (rc, MTYPE_RegisterSize(mtype) );
#endif
}

extern	TN *Dup_TN ( TN *tn );	/* Duplicate an existing TN */
extern  TN *Dup_TN_Even_If_Dedicated ( TN *tn ) ; /* Ditto, but for
                                                   * dedicated */

/* Register assignment: */

inline BOOL TN_Is_Allocatable( const TN *tn )
{
  return ! ( TN_is_constant(tn) || TN_is_dedicated(tn));
}

inline void TN_Allocate_Register( TN *tn, REGISTER reg )
{
  Is_True (TN_Is_Allocatable(tn), ("Invalid TN for register allocation"));
  Set_TN_register(tn, reg);
}

inline BOOL TN_is_dedicated_class_and_reg( TN *tn, UINT16 class_n_reg )
{
  return    TN_is_dedicated(tn)
	 && TN_register_and_class(tn) == class_n_reg;
}


/* Only the following routines should be used to build constant TNs. */

CG_EXPORTED extern	TN *Gen_Literal_TN ( INT64 val, INT size );
// normally literals are hashed and reused; this creates unique TN
extern TN *Gen_Unique_Literal_TN (INT64 ivalue, INT size);

extern TN *Gen_Enum_TN (ISA_ENUM_CLASS_VALUE ecv);

extern  TN *Gen_Symbol_TN ( ST *s, INT64 offset, INT32 relocs);
CG_EXPORTED extern  TN *Gen_Label_TN ( LABEL_IDX lab, INT64 offset );
extern  TN *Gen_Tag_TN ( LABEL_IDX tag);
extern	TN *Gen_Adjusted_TN( TN *tn, INT64 adjust );


/* Trace support: */
/* Print TN to a file with given 'fmt'; assume fmt has a %s in it. */
extern	void  fPrint_TN ( FILE *f, char *fmt, TN *tn);
#pragma mips_frequency_hint NEVER fPrint_TN
/* Print TN to the trace file TFile */
extern	void   Print_TN ( TN *tn, BOOL verbose );
#pragma mips_frequency_hint NEVER Print_TN
/* Print a tn list to a file */
extern	void   Print_TN_List ( FILE *, struct TN_LIST * );
#pragma mips_frequency_hint NEVER Print_TN_List
/* Print all TNs */
extern	void   Print_TNs ( void );
#pragma mips_frequency_hint NEVER Print_TNs

/* Return the first tn in list which matches the register_class and register of tn0 */
/* If none is found, return NULL. */
extern TN *Find_TN_with_Matching_Register( TN *tn0, struct TN_LIST *list );

typedef enum {VAL_KNOWN, VAL_UNKNOWN, VAL_COND_DEF, VAL_COND_USE} DEF_KIND;

/* TN value support: */
extern struct op *TN_Reaching_Value_At_Op( TN *tn, struct op *op, DEF_KIND *kind, BOOL reaching_def );
extern BOOL TN_Value_At_Op( TN *tn, struct op *use_op, INT64 *val );

/* Determine whether a given expression involving a constant TN (which
 * may be a symbol TN) is a valid literal operand for the given opcode:
 */
extern BOOL Potential_Immediate_TN_Expr (
  TOP opcode,		/* The operation of interest */
  struct tn *tn1,	/* The primary TN (constant) */
  INT32	disp		/* Displacement from value */
);

/* Initialize machine specific dedicated TNs */
extern void Init_Dedicated_TNs (void);

/* Format const TN value depending on relocation*/
extern INT64 CGTARG_TN_Value (TN *t, INT64 base_ofst);

/*
 * Relocation info 
 */
typedef struct {
  char *name;
} TN_RELOCS_INFO;
CG_EXPORTED extern const TN_RELOCS_INFO TN_RELOCS_info[];


inline const char * TN_RELOCS_Name (mUINT8 rc)
{
  return TN_RELOCS_info[rc].name;
}

extern TN* Gen_Predicate_TN(void);
extern BOOL TN_Use_Base_ST_For_Reloc (INT reloc, ST *st);
extern INT  TN_Relocs_In_Asm (TN *t, ST *st, vstring *buf, INT64 *val);

#endif /* tn_INCLUDED */
