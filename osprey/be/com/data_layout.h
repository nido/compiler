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
 * Module: data_layout.h
 *
 * Revision history:
 *  11-Nov-94 - Original Version
 *
 * Description:
 *
 * General support for data layout functionality in the back end.
 *
 * Also see:  stblock.h for base block handling.
 *
 * ====================================================================
 * ====================================================================
 */
#ifndef	data_layout_INCLUDED
#define	data_layout_INCLUDED

#include "stab.h"
#include "wn.h"
#ifdef TARG_ST
/* Arthur: I moved some of the target-specific definitions there */
#include "config_target.h"
#endif
#include "targ_sim.h"

#ifdef __cplusplus
extern "C" {
#endif

BE_EXPORTED extern ST *SP_Sym;		/* stack pointer */
BE_EXPORTED extern ST *FP_Sym;		/* frame pointer */
BE_EXPORTED extern ST *Local_Spill_Sym;	/* reserved close location for spill */

/* direction for the stack frame to grow */
typedef enum { INCREMENT, DECREMENT } STACK_DIR;

/* Choice of stack model to use: */
typedef enum {
  SMODEL_UNDEF = 0,	/* Not yet defined */
  SMODEL_SMALL = 1,	/* Small frame, no frame pointer */
  SMODEL_LARGE = 2,	/* Large frame, use frame pointer */
  SMODEL_DYNAMIC = 3	/* Dynamic allocation, use frame pointer */
} STACK_MODEL;

#ifdef TARG_ST
#define Is_root_base(st) (ST_class(st) == CLASS_BLOCK && STB_root_base(st))
#define Has_No_Base_Block(st) (ST_base(st) == st)
#define Is_Allocatable(st)	(!Has_Base_Block(st) && !Is_root_base(st))
#define Is_Allocatable_Root_Block(st) \
 (ST_class(st) == CLASS_BLOCK && !Is_root_base(st) && Has_No_Base_Block(st))

#define ST_force_gprel(s)	ST_gprel(s)
#endif /* TARG_ST */

BE_EXPORTED extern STACK_MODEL Current_PU_Stack_Model;    /* stack model of current pu */

BE_EXPORTED extern STACK_DIR Stack_Direction(void);

/*
 * Initialize_Stack_Frame must be called once per PU,
 * before it is split into regions.
 * This will determine the stack model,
 * set up internal structures, and allocate all formals.
 */
BE_EXPORTED extern void Initialize_Stack_Frame (WN *PU_tree);

// This is called after lowering, when have more accurate view of code.
BE_EXPORTED extern void Calculate_Stack_Frame_Sizes (WN *PU_tree);

/*
 * Finalize_Stack_Frame should be called once per PU before cgemit;
 * it returns the stack frame size.
 */
BE_EXPORTED extern INT64 Finalize_Stack_Frame(void);

/* 
 * Verify that stack space needed for actuals doesn't overflow
 * what we initially reserved. 
 */
BE_EXPORTED extern void Check_Actual_Stack_Size (WN *call_tree);

/* allocate a temporary for the vararg formal */
BE_EXPORTED extern ST *Allocate_Vararg_Formal (void);

BE_EXPORTED extern BOOL Is_Allocated (ST *st);	/* has ST been allocated? */

/* Allocate the memory location for this object 'st' */
BE_EXPORTED extern void Allocate_Object ( ST *st );

/* This function allows the code generator to allocate space for */
/* register spilled temp variable */
BE_EXPORTED extern void Allocate_Temp_To_Memory ( ST *st );

/* Get upformal symbol for altentries that will be copied to temp space */
BE_EXPORTED extern ST* Get_Altentry_UpFormal_Symbol (ST *formal, PLOC ploc);

/* reset the upformal segment when start an alternate entry */
BE_EXPORTED extern void Reset_UPFORMAL_Segment(void );

/* get vararg symbol that corresponds to ploc value */
BE_EXPORTED extern ST* Get_Vararg_Symbol (PLOC);

/* 
 * Allocate the memory location for all global/static symbols.
 */
BE_EXPORTED extern void Allocate_File_Statics (void);

/*
 * Pad global, C arrays
 */
BE_EXPORTED extern void Pad_Global_Arrays();

/* return GUESS of whether ST will use an offset < 16 bits */
BE_EXPORTED extern BOOL Uses_Small_Offset (ST *st, WN_OFFSET wn_ofst);

BE_EXPORTED extern INT32 Stack_Alignment (void);

/* Return true if the ST is defined on the stack */
BE_EXPORTED extern BOOL ST_on_stack(ST *);

/* Return true if the ST is defined in the pu */
BE_EXPORTED extern BOOL ST_pu_defined(ST *);

/* Look for a formal-ref-base corresponding to the sym. */
BE_EXPORTED extern ST * Get_ST_formal_ref_base (const ST *sym);
BE_EXPORTED extern void Set_ST_formal_ref_base (const ST *sym, ST *base);

/* return 0 if no formal-preg-num */
BE_EXPORTED extern PREG_NUM Get_ST_formal_preg_num (const ST *base);
BE_EXPORTED extern void Set_ST_formal_preg_num (const ST *base, PREG_NUM p);

inline BOOL ST_has_formal_preg_num (const ST *const st)
  {
    return (ST_sclass(st) == SCLASS_FORMAL &&
	    Get_ST_formal_preg_num(st) != 0);
  }

/* Map arg-area-size to the corresponding PU */
BE_EXPORTED extern UINT32 Get_PU_arg_area_size(PU_IDX);
BE_EXPORTED extern void Set_PU_arg_area_size(PU_IDX, UINT32 size);

/* size of actual build area for current PU */
BE_EXPORTED extern INT32 Current_PU_Actual_Size;	

/* return ST for __return_address if one exists */
BE_EXPORTED extern ST * Find_Special_Return_Address_Symbol (void);

#ifdef TARG_ST
/* get ST for __return_address, creates it if it does not exist. */
BE_EXPORTED extern ST *Get_Special_Return_Address_Symbol (void);
#endif

/* return stack offset adjustment, which may depend on size of pu frame */
BE_EXPORTED extern INT Stack_Offset_Adjustment_For_PU (void);

#ifdef TARG_ST
  // [CL] return the name of the section for ST. Exported for IPA use
BE_EXPORTED extern STR_IDX Find_Section_Name_For_ST (const ST *st);
#endif

#ifdef TARG_ST
/* Allocate space for the REGSAVE located area after FTEMP. */
BE_EXPORTED extern void Allocate_Reg_Save_Area(INT64 size);

/* Returns the block symbol corresponding to the REGSAVE area. */
BE_EXPORTED extern ST *Get_Reg_Save_Area(void);
#endif

#ifdef TARG_ST
/*
 * SF_VARARG_INFO type
 *
 * This type collects information on the vararg area status.
 * It is used to communicate to the va_arg/va_start emulation
 * the status of the vararg araes.
 * Once the lowering of entry/exit formals have been done.
 * the function Get_Vararg_Save_Area_Info() can be called
 * to fill a SF_VARARG_INFO type.
 * From the SF_VARARG_INFO the emulation can access the following
 * information:
 *
 *   INT SF_VARARG_INFO_flags(x)
 *   Retuns the flags for interpreting this information.
 *   if SF_VARARG_INFO_flags(x) & SF_VARARG_INFO_HAS_INT
 *	then the vararg area as a in register save area base
 *   if SF_VARARG_INFO_flags(x) & SF_VARARG_INFO_HAS_FLOAT
 *	then the vararg area as a float register save area base
 *
 *   if SF_VARARG_INFO_flags(x) & SF_VARARG_INFO_HAS_STACK
 *	then the vararg area as a variable stack area base
 *
 *   INT SF_VARARG_INFO_fixed_int_params(x)
 *   Returns the number of fixed int slots used by the function.
 *
 *   INT SF_VARARG_INFO_int_slot_size(x)
 *   Returns the size of an int slot.
 *
 *   INT SF_VARARG_INFO_fixed_float_params(x)
 *   Returns the number of fixed float slots used by the function.
 *
 *   INT SF_VARARG_INFO_float_slot_size(x)
 *   Returns the size of a float slot.
 *
 *   ST *SF_VARARG_INFO_var_int_save_base(x)
 *   INT SF_VARARG_INFO_var_int_save_offset(x)
 *   Returns the base+offset address location to the
 *   start (low memory) of the variable int registers save area.
 *   The first variable int register should be homed at that location.
 *
 *   ST *SF_VARARG_INFO_var_float_save_base(x)
 *   INT SF_VARARG_INFO_var_float_save_offset(x)
 *   Returns the base+offset address location to the
 *   start (low memory) of the variable float registers save area.
 *   The first variable float register should be homed at that location.
 *
 *   ST *SF_VARARG_INFO_var_float_save_base(x)
 *   INT SF_VARARG_INFO_var_float_save_offset(x)
 *   Returns the base+offset address location to the
 *   start (low memory) of the variable float registers save area.
 *   The first variable float register should be homed at that location.
 */

#define SF_VARARG_INFO_HAS_INT 1
#define SF_VARARG_INFO_HAS_FLOAT 2
#define SF_VARARG_INFO_HAS_STACK 4

typedef struct {
  INT flags;
  INT fixed_int_slots;
  INT int_slot_size;
  INT fixed_float_slots;
  INT float_slot_size;
  ST *var_int_save_base;
  INT var_int_save_offset;
  ST *var_float_save_base;
  INT var_float_save_offset;
  ST *var_stack_base;
  INT var_stack_offset;
} SF_VARARG_INFO;
#define SF_VARARG_INFO_flags(x) ((x)->flags)
#define SF_VARARG_INFO_fixed_int_slots(x) ((x)->fixed_int_slots)
#define SF_VARARG_INFO_int_slot_size(x) ((x)->int_slot_size)
#define SF_VARARG_INFO_fixed_float_slots(x) ((x)->fixed_float_slots)
#define SF_VARARG_INFO_float_slot_size(x) ((x)->float_slot_size)
#define SF_VARARG_INFO_var_int_save_base(x) ((x)->var_int_save_base)
#define SF_VARARG_INFO_var_int_save_offset(x) ((x)->var_int_save_offset)
#define SF_VARARG_INFO_var_float_save_base(x) ((x)->var_float_save_base)
#define SF_VARARG_INFO_var_float_save_offset(x) ((x)->var_float_save_offset)
#define SF_VARARG_INFO_var_stack_base(x) ((x)->var_stack_base)
#define SF_VARARG_INFO_var_stack_offset(x) ((x)->var_stack_offset)

BE_EXPORTED extern void Get_Vararg_Save_Area_Info(SF_VARARG_INFO *vararg_info);

BE_EXPORTED extern void Get_Vararg_Ploc_Symbol (PLOC, ST **symbol, INT *offset);

/* get preg symbols corresponding to basic types Int, Float, Ptr. */
BE_EXPORTED extern ST * Get_Vararg_Preg_Symbol (PLOC);

#endif

#ifdef __cplusplus
}
#endif
#endif /* data_layout_INCLUDED */

