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
 * Module: register.c
 *
 * Revision history:
 *  17-May-93 - Original Version
 *
 * Description:
 *
 *      register implementation.
 *
 * ====================================================================
 * ====================================================================
 */

#define INCLUDING_IN_REGISTER

#include <ctype.h>                /* for isdigit() */
// [HK]
#if __GNUC__ >= 3
#include <vector>
#else
#include <vector.h>
#endif //  __GNUC__ >= 3
#include "defs.h"
#include "errors.h"
#include "erglob.h"
#include "tracing.h"
#include "mempool.h"
#include "config.h"
#include "glob.h"
#include "util.h"
#include "tn.h"
#include "op.h"
#include "calls.h"
#include "data_layout.h"

#include "targ_sim.h"

#include "targ_isa_registers.h"
#include "targ_abi_properties.h"
#include "targ_isa_operands.h"

#include "cgtarget.h"

/* ====================================================================
 * Is the frame-pointer register required for the current function
 * ====================================================================
 */

#define FRAME_POINTER_REQUIRED_FOR_PU	\
  	(Current_PU_Stack_Model != SMODEL_SMALL || PUSH_FRAME_POINTER_ON_STACK)

/* ====================================================================
 * Shared data structures
 * ====================================================================
 */

/* Exported data:
 */
#ifdef TARG_ST
REGISTER_SUBCLASS_INFO REGISTER_SUBCLASS_info[ISA_REGISTER_SUBCLASS_MAX_LIMIT + 1];
ISA_REGISTER_CLASS  REGISTER_CLASS_vec[ISA_REGISTER_CLASS_MAX_LIMIT + 1];
REGISTER_CLASS_INFO REGISTER_CLASS_info[ISA_REGISTER_CLASS_MAX_LIMIT + 1];
#else
REGISTER_SUBCLASS_INFO REGISTER_SUBCLASS_info[ISA_REGISTER_SUBCLASS_MAX + 1];
ISA_REGISTER_CLASS  REGISTER_CLASS_vec[ISA_REGISTER_CLASS_MAX + 1];
REGISTER_CLASS_INFO REGISTER_CLASS_info[ISA_REGISTER_CLASS_MAX + 1];
#endif

const CLASS_REG_PAIR CLASS_REG_PAIR_undef =
  {CREATE_CLASS_N_REG(ISA_REGISTER_CLASS_UNDEFINED,REGISTER_UNDEFINED)};

CLASS_REG_PAIR      CLASS_REG_PAIR_zero = CLASS_REG_PAIR_undef;
CLASS_REG_PAIR      CLASS_REG_PAIR_ep = CLASS_REG_PAIR_undef;
CLASS_REG_PAIR      CLASS_REG_PAIR_gp = CLASS_REG_PAIR_undef;
CLASS_REG_PAIR      CLASS_REG_PAIR_sp = CLASS_REG_PAIR_undef;
CLASS_REG_PAIR      CLASS_REG_PAIR_fp = CLASS_REG_PAIR_undef;
CLASS_REG_PAIR      CLASS_REG_PAIR_ra = CLASS_REG_PAIR_undef;
CLASS_REG_PAIR      CLASS_REG_PAIR_rs = CLASS_REG_PAIR_undef;
CLASS_REG_PAIR      CLASS_REG_PAIR_v0 = CLASS_REG_PAIR_undef;
CLASS_REG_PAIR      CLASS_REG_PAIR_static_link = CLASS_REG_PAIR_undef;
CLASS_REG_PAIR      CLASS_REG_PAIR_pfs = CLASS_REG_PAIR_undef;
CLASS_REG_PAIR      CLASS_REG_PAIR_lc = CLASS_REG_PAIR_undef;
CLASS_REG_PAIR      CLASS_REG_PAIR_ec = CLASS_REG_PAIR_undef;
CLASS_REG_PAIR      CLASS_REG_PAIR_true = CLASS_REG_PAIR_undef;
CLASS_REG_PAIR      CLASS_REG_PAIR_fzero = CLASS_REG_PAIR_undef;
CLASS_REG_PAIR      CLASS_REG_PAIR_fone = CLASS_REG_PAIR_undef;
CLASS_REG_PAIR      CLASS_REG_PAIR_link = CLASS_REG_PAIR_undef;
#ifdef TARG_ST // [SC] TLS support
CLASS_REG_PAIR      CLASS_REG_PAIR_tp = CLASS_REG_PAIR_undef;
#endif

#if ISA_REGISTER_MAX >= 64
const REGISTER_SET REGISTER_SET_EMPTY_SET = { 0 };
#endif /* ISA_REGISTER_MAX >= 64 */

/* Track the "allocatable" state of each register.
 */
enum {
  AS_default = 0,	/* the default is what targ_info says */
  AS_allocatable = 1,
  AS_not_allocatable = 2
};

#ifdef TARG_ST
static mUINT8 reg_alloc_status[ISA_REGISTER_CLASS_MAX_LIMIT + 1][REGISTER_MAX + 1];
#else
static mUINT8 reg_alloc_status[ISA_REGISTER_CLASS_MAX + 1][REGISTER_MAX + 1];
#endif

// list of registers that should not be allocated, both globally and locally.
static std::vector< std::pair< ISA_REGISTER_CLASS, REGISTER> > dont_allocate_these_registers;
static std::vector< std::pair< ISA_REGISTER_CLASS, REGISTER> > dont_allocate_these_registers_in_pu;


/* ====================================================================
 *
 *  Set_CLASS_REG_PAIR
 *
 *  Set the rclass and register.x
 *
 * ====================================================================
 */
void Set_CLASS_REG_PAIR(CLASS_REG_PAIR& rp, ISA_REGISTER_CLASS rclass, REGISTER reg)
{
  rp.class_reg.rclass = rclass;
  rp.class_reg.reg = reg;
}


/* ====================================================================
 *
 *  REGISTER_SET_Range
 *
 *  Return the a set of the registers: low .. high
 *
 * ====================================================================
 */
REGISTER_SET
REGISTER_SET_Range(UINT low, UINT high)
{
#if ISA_REGISTER_MAX < 64
  Is_True(low >= REGISTER_MIN && low <= high && high <= REGISTER_MAX,
	  ("REGISTER_SET_Range: bad range specification"));

  UINT leading_zeros = (sizeof(REGISTER_SET_WORD) * 8) - high;
  UINT trailing_zeros = low - REGISTER_MIN;
  return   ((REGISTER_SET_WORD)-1 << (leading_zeros + trailing_zeros)) 
	>> leading_zeros;
#else /* ISA_REGISTER_MAX < 64 */
  INT i;
  REGISTER_SET set;
  for (i = 0; i <= MAX_REGISTER_SET_IDX; ++i) {
    UINT this_low = (i * 64) + REGISTER_MIN;
    UINT this_high = this_low + 63;
    if (low > this_high || high < this_low) {
      set.v[i] = 0;
    } else {
      UINT leading_zeros = high > this_high ? 0 : this_high - high;
      UINT trailing_zeros = low < this_low ? 0 : low - this_low;
      set.v[i] =   ((REGISTER_SET_WORD)-1 << (leading_zeros + trailing_zeros)) 
		>> leading_zeros;
    }
  }
  return set;
#endif /* ISA_REGISTER_MAX < 64 */
}

/* ====================================================================
 *   Mark_Specified_Registers_As_Not_Allocatable
 *
 *   some variables can be pre-allocated to registers, in which case 
 *   the symtab will be marked, or the command-line may list registers 
 *   not to be used.
 * ====================================================================
 */
static void
Increment_Register_Name (
  char **name
)
{
  INT i = atoi(*name);
  ++i;
  sprintf(*name, "%d", i);
}

static void
Set_Register_Range_Not_Allocatable (
  char *regname1, 
  char *regname2
)
{
  char regname[16];
  char *p;	// points to first digit in regname 
  INT count = 0;
  strcpy(regname,regname1);
  // find where digits start
  for (p = regname; *p && !isdigit(*p); ++p) ;
  FmtAssert( strncmp(regname1, regname2, p - regname) == 0,
	("register range %s-%s doesn't have matching prefixes", 
	regname1, regname2));

  // create each regname in range
  while (strcmp(regname, regname2) != 0) {
	Set_Register_Never_Allocatable (regname);
	Increment_Register_Name (&p);
	++count; if (count > 200) break;	// avoid infinite loop
  }
  Set_Register_Never_Allocatable (regname);
}

struct Set_DREG_Not_Allocatable 
{
  inline void operator() (UINT32, ST_ATTR *st_attr) const {
    if (ST_ATTR_kind (*st_attr) != ST_ATTR_DEDICATED_REGISTER)
      return;
    PREG_NUM p = ST_ATTR_reg_id(*st_attr);
    Set_Register_Never_Allocatable(p);
  }
};

void
Mark_Specified_Registers_As_Not_Allocatable ()
{
  OPTION_LIST *ol = Registers_Not_Allocatable;
  char *start;
  char *p;
  char regname[16];
  char regname2[16];

  // go through global dreg list
  if (ST_ATTR_Table_Size (GLOBAL_SYMTAB)) {
    For_all (St_Attr_Table, GLOBAL_SYMTAB, Set_DREG_Not_Allocatable());
  }

  // now go through command-line list
  if (ol == NULL) return;
  for ( ; ol != NULL; ol = OLIST_next(ol)) {

    /* Check for commas and ranges: */
    p = OLIST_val(ol);
    start = p;
    while ( *p != ':' && *p != 0 ) {
      if ( *p == ',') {
	strncpy (regname, start, p-start+1);
	regname[p-start] = '\0';
	Set_Register_Never_Allocatable (regname);
	++p;
	start = p;
      }
      else if (*p == '-' ) {
	strncpy (regname, start, p-start+1);
	regname[p-start] = '\0';
	++p;
	start = p;
	while (*p != ',' && *p != '\0') {
	  ++p;
	}
	strncpy (regname2, start, p-start+1);
	regname2[p-start] = '\0';
	Set_Register_Range_Not_Allocatable (regname, regname2);
	if (*p == 0) return;
	++p;
	start = p;
      }
      else {
	++p;
      }
    }
    strncpy (regname, start, p-start+1);
    Set_Register_Never_Allocatable (regname);
  }
}

/* ====================================================================
 * ====================================================================
 *                 Initialization and termination
 * ====================================================================
 * ====================================================================
 */

/* ====================================================================
 *  Initialize_Register_Class
 *
 *  Initialize the register class 'rclass'. A register class may be
 *  intialized multiple times.
 * ====================================================================
 */
static void
Initialize_Register_Class(
  ISA_REGISTER_CLASS rclass
)
{
  INT32              i;
  const ISA_REGISTER_CLASS_INFO *icinfo = ISA_REGISTER_CLASS_Info(rclass);
  const char        *rcname         = ISA_REGISTER_CLASS_INFO_Name(icinfo);
  INT		     bit_size       = ISA_REGISTER_CLASS_INFO_Bit_Size(icinfo);
  INT                first_isa_reg  = ISA_REGISTER_CLASS_INFO_First_Reg(icinfo);
  INT                last_isa_reg   = ISA_REGISTER_CLASS_INFO_Last_Reg(icinfo);
  INT                register_count = last_isa_reg - first_isa_reg + 1;

  REGISTER_SET       allocatable    = REGISTER_SET_EMPTY_SET;
  REGISTER_SET       caller         = REGISTER_SET_EMPTY_SET;
  REGISTER_SET       callee         = REGISTER_SET_EMPTY_SET;
  REGISTER_SET       func_argument  = REGISTER_SET_EMPTY_SET;
  REGISTER_SET       func_value     = REGISTER_SET_EMPTY_SET;
  REGISTER_SET       shrink_wrap    = REGISTER_SET_EMPTY_SET;
  REGISTER_SET	     stacked        = REGISTER_SET_EMPTY_SET;
  REGISTER_SET	     rotating       = REGISTER_SET_EMPTY_SET;


  /* Verify we have a valid rclass and that the type used to implement 
   * a register set is large enough.
   */
  FmtAssert(rclass >= ISA_REGISTER_CLASS_MIN && 
	    rclass <= ISA_REGISTER_CLASS_MAX,
	                       ("invalide register class %d", (INT)rclass));
  FmtAssert((sizeof(REGISTER_SET) * 8) >= register_count,
	    ("REGISTER_SET type cannot represent all registers in "
	     "the class %s", rcname));

  REGISTER_CLASS_name(rclass) = rcname;

  /* Now make sets of various register properties:
   */
  for (i = 0; i < register_count; ++i) {
    INT      isa_reg        = i + first_isa_reg;
    REGISTER reg            = i + REGISTER_MIN;
    BOOL     is_allocatable = ABI_PROPERTY_Is_allocatable(rclass, isa_reg);
    INT      alloc_status   = reg_alloc_status[rclass][reg];

    /* CG likes to pretend that a class with only one register can't
     * be allocated, so perpetuate that illusion.
     */
    if (register_count <= 1) is_allocatable = FALSE;

    switch ( alloc_status ) {
      case AS_allocatable:
	is_allocatable = TRUE;
	break;
      case AS_not_allocatable:
	is_allocatable = FALSE;
	break;
      case AS_default:
	break;
      default:
	Is_True(FALSE, ("unhandled allocations status: %d", alloc_status));
    }

    if (is_allocatable) {
      allocatable = REGISTER_SET_Union1(allocatable, reg);

#ifdef TARG_ST
      if (ABI_PROPERTY_Is_global_ptr(rclass, isa_reg)) {
	Set_CLASS_REG_PAIR_reg(CLASS_REG_PAIR_gp, reg);
	Set_CLASS_REG_PAIR_rclass(CLASS_REG_PAIR_gp, rclass);
	if (Constant_GP) {
	  //        if (GP_Is_Preserved) {
	  /* neither caller nor callee saved (always preserved). */
	} else if (Is_Caller_Save_GP) {
	  /* caller-saved. */
	  caller = REGISTER_SET_Union1(caller, reg);
	} else {
	  /* callee-saved. */
	  callee = REGISTER_SET_Union1(callee, reg);
	}
      }
#else
      if ( ABI_PROPERTY_Is_global_ptr(rclass, isa_reg) ) {
        if ( GP_Is_Preserved ) {
          /* neither caller nor callee saved (always preserved). */
        } else if ( Is_Caller_Save_GP ) {
          /* caller-saved. */
          caller = REGISTER_SET_Union1(caller, reg);
        } else {
          /* callee-saved. */
          callee = REGISTER_SET_Union1(callee, reg);
        }
      }
#endif
      else {
        if (ABI_PROPERTY_Is_callee(rclass, isa_reg)) {
          callee = REGISTER_SET_Union1(callee, reg);
          shrink_wrap = REGISTER_SET_Union1(shrink_wrap, reg);
        }
        if (ABI_PROPERTY_Is_caller(rclass, isa_reg))
          caller = REGISTER_SET_Union1(caller, reg);
        if (ABI_PROPERTY_Is_func_arg(rclass, isa_reg))
          func_argument = REGISTER_SET_Union1(func_argument, reg);
        if (ABI_PROPERTY_Is_func_val(rclass, isa_reg))
          func_value = REGISTER_SET_Union1(func_value, reg);
        if (ABI_PROPERTY_Is_ret_addr(rclass, isa_reg))
          shrink_wrap = REGISTER_SET_Union1(shrink_wrap, reg);
        if ( ABI_PROPERTY_Is_stacked(rclass, isa_reg) )
          stacked = REGISTER_SET_Union1(stacked, reg);
      }
    }

    REGISTER_bit_size(rclass, reg) = bit_size;
    REGISTER_machine_id(rclass, reg) = isa_reg;
    REGISTER_allocatable(rclass, reg) = is_allocatable;
    REGISTER_name(rclass, reg) = 
                   ISA_REGISTER_CLASS_INFO_Reg_Name(icinfo, isa_reg);

    if ( ABI_PROPERTY_Is_frame_ptr(rclass, isa_reg) ) {
      Set_CLASS_REG_PAIR_reg(CLASS_REG_PAIR_fp, reg);
      Set_CLASS_REG_PAIR_rclass(CLASS_REG_PAIR_fp, rclass);
    }
    else if ( ABI_PROPERTY_Is_static_link(rclass, isa_reg) ) {
      Set_CLASS_REG_PAIR_reg(CLASS_REG_PAIR_static_link, reg);
      Set_CLASS_REG_PAIR_rclass(CLASS_REG_PAIR_static_link, rclass);
    }
    else if ( ABI_PROPERTY_Is_global_ptr(rclass, isa_reg) ) {
#ifdef TARG_ST
      if (Gen_GP_Relative) {
	Set_CLASS_REG_PAIR_reg(CLASS_REG_PAIR_gp, reg);
	Set_CLASS_REG_PAIR_rclass(CLASS_REG_PAIR_gp, rclass);
      }
#else
      Set_CLASS_REG_PAIR_reg(CLASS_REG_PAIR_gp, reg);
      Set_CLASS_REG_PAIR_rclass(CLASS_REG_PAIR_gp, rclass);
#endif
    }
    else if ( ABI_PROPERTY_Is_ret_addr(rclass, isa_reg) ) {
      Set_CLASS_REG_PAIR_reg(CLASS_REG_PAIR_ra, reg);
      Set_CLASS_REG_PAIR_rclass(CLASS_REG_PAIR_ra, rclass);
    }
    else if ( ABI_PROPERTY_Is_stack_ptr(rclass, isa_reg) ) {
      Set_CLASS_REG_PAIR_reg(CLASS_REG_PAIR_sp, reg);
      Set_CLASS_REG_PAIR_rclass(CLASS_REG_PAIR_sp, rclass);
    }
    else if ( ABI_PROPERTY_Is_entry_ptr(rclass, isa_reg) ) {
      Set_CLASS_REG_PAIR_reg(CLASS_REG_PAIR_ep, reg);
      Set_CLASS_REG_PAIR_rclass(CLASS_REG_PAIR_ep, rclass);
    }
    else if ( ABI_PROPERTY_Is_zero(rclass, isa_reg) ) {
      Set_CLASS_REG_PAIR_reg(CLASS_REG_PAIR_zero, reg);
      Set_CLASS_REG_PAIR_rclass(CLASS_REG_PAIR_zero, rclass);
    }
    else if ( ABI_PROPERTY_Is_prev_funcstate(rclass, isa_reg) ) {
      Set_CLASS_REG_PAIR_reg(CLASS_REG_PAIR_pfs, reg);
      Set_CLASS_REG_PAIR_rclass(CLASS_REG_PAIR_pfs, rclass);
    }
    //
    // Arthur: More to possible ABI:
    //
    else if ( ABI_PROPERTY_Is_link(rclass, isa_reg) ) {
      Set_CLASS_REG_PAIR_reg(CLASS_REG_PAIR_link, reg);
      Set_CLASS_REG_PAIR_rclass(CLASS_REG_PAIR_link, rclass);
    }
    else if ( ABI_PROPERTY_Is_ret_struct(rclass, isa_reg) ) {
      Set_CLASS_REG_PAIR_reg(CLASS_REG_PAIR_rs, reg);
      Set_CLASS_REG_PAIR_rclass(CLASS_REG_PAIR_rs, rclass);
    }
    else if ( ABI_PROPERTY_Is_loop_count(rclass, isa_reg) ) {
      Set_CLASS_REG_PAIR_reg(CLASS_REG_PAIR_lc, reg);
      Set_CLASS_REG_PAIR_rclass(CLASS_REG_PAIR_lc, rclass);
    }
    else if ( ABI_PROPERTY_Is_epilog_count(rclass, isa_reg) ) {
      Set_CLASS_REG_PAIR_reg(CLASS_REG_PAIR_ec, reg);
      Set_CLASS_REG_PAIR_rclass(CLASS_REG_PAIR_ec, rclass);
    }
    else if ( ABI_PROPERTY_Is_true_predicate(rclass, isa_reg) ) {
      Set_CLASS_REG_PAIR_reg(CLASS_REG_PAIR_true, reg);
      Set_CLASS_REG_PAIR_rclass(CLASS_REG_PAIR_true, rclass);
    }
    else if ( ABI_PROPERTY_Is_fzero(rclass, isa_reg) ) {
      Set_CLASS_REG_PAIR_reg(CLASS_REG_PAIR_fzero, reg);
      Set_CLASS_REG_PAIR_rclass(CLASS_REG_PAIR_fzero, rclass);
    }
    else if ( ABI_PROPERTY_Is_fone(rclass, isa_reg) ) {
      Set_CLASS_REG_PAIR_reg(CLASS_REG_PAIR_fone, reg);
      Set_CLASS_REG_PAIR_rclass(CLASS_REG_PAIR_fone, rclass);
    }
#ifdef TARG_ST // [SC] TLS support
    else if (ABI_PROPERTY_Is_thread_ptr(rclass, isa_reg) ) {
      Set_CLASS_REG_PAIR_reg(CLASS_REG_PAIR_tp, reg);
      Set_CLASS_REG_PAIR_rclass(CLASS_REG_PAIR_tp, rclass);
    }
#endif
  }

  REGISTER_CLASS_universe(rclass)          =
	REGISTER_SET_Range(REGISTER_MIN, REGISTER_MIN + register_count - 1);
  REGISTER_CLASS_allocatable(rclass)       = allocatable;
  REGISTER_CLASS_callee_saves(rclass)      = callee;
  REGISTER_CLASS_caller_saves(rclass)      = caller;
  REGISTER_CLASS_function_argument(rclass) = func_argument;
  REGISTER_CLASS_function_value(rclass)    = func_value;
  REGISTER_CLASS_shrink_wrap(rclass)       = shrink_wrap;
  REGISTER_CLASS_register_count(rclass)    = register_count;
  REGISTER_CLASS_stacked(rclass)           = stacked;
  REGISTER_CLASS_rotating(rclass)          = rotating;
#ifdef TARG_ST
    REGISTER_CLASS_is_ptr(rclass)
	= ISA_REGISTER_CLASS_INFO_Is_Ptr(icinfo);
#endif
  REGISTER_CLASS_can_store(rclass)
	= ISA_REGISTER_CLASS_INFO_Can_Store(icinfo);
  REGISTER_CLASS_multiple_save(rclass)
	= ISA_REGISTER_CLASS_INFO_Multiple_Save(icinfo);

#ifdef TARG_ST
  // [CG] Implemented in targ_register.cxx.
  CGTARG_Initialize_Register_Class (rclass);
#else
  /* There are multiple integer return regs -- v0 is the lowest
   * of the set.
   */
  if (rclass == ISA_REGISTER_CLASS_integer) {
    Set_CLASS_REG_PAIR_reg(CLASS_REG_PAIR_v0, REGISTER_SET_Choose(func_value));
    Set_CLASS_REG_PAIR_rclass(CLASS_REG_PAIR_v0, rclass);
  }
#endif

  return;
}

/* ====================================================================
 *   Initialize_Register_Subclasses
 *
 *   Initialize the register subclass information cache.
 * ====================================================================
 */
static void
Initialize_Register_Subclasses(void)
{
  ISA_REGISTER_SUBCLASS sc;

  for (sc = (ISA_REGISTER_SUBCLASS)0; // standard iterator skips _UNDEFINED (0)
       sc <= ISA_REGISTER_SUBCLASS_MAX; 
       sc = (ISA_REGISTER_SUBCLASS)(sc + 1)) {
    INT i;
    ISA_REGISTER_CLASS rc;
    REGISTER_SET members = REGISTER_SET_EMPTY_SET;
    const ISA_REGISTER_SUBCLASS_INFO *scinfo = ISA_REGISTER_SUBCLASS_Info(sc);
    INT count = ISA_REGISTER_SUBCLASS_INFO_Count(scinfo);

    for (i = 0; i < count; ++i) {
      INT isa_reg = ISA_REGISTER_SUBCLASS_INFO_Member(scinfo, i);
      const char *reg_name = ISA_REGISTER_SUBCLASS_INFO_Reg_Name(scinfo, i);
      REGISTER reg = (REGISTER)(isa_reg + REGISTER_MIN);
      members = REGISTER_SET_Union1(members, reg);
      REGISTER_SUBCLASS_reg_name(sc, reg) = reg_name;
    }
    rc = ISA_REGISTER_SUBCLASS_INFO_Class(scinfo);
    members = REGISTER_SET_Intersection(members, REGISTER_CLASS_universe(rc));

    REGISTER_SUBCLASS_members(sc) = members;
    REGISTER_SUBCLASS_name(sc) = ISA_REGISTER_SUBCLASS_INFO_Name(scinfo);
    REGISTER_SUBCLASS_register_class(sc) = rc;
  }

  return;
}

/* ====================================================================
 *   REGISTER_Begin
 * ====================================================================
 */
void
REGISTER_Begin (void)
{
  ISA_REGISTER_CLASS rclass;

  /*  Create the register classes for all the target registers.
   */
  FOR_ALL_ISA_REGISTER_CLASS( rclass ) {
	Initialize_Register_Class(rclass);
#ifdef HAS_STACKED_REGISTERS
    	REGISTER_Init_Stacked(rclass);
#endif
  }
  Initialize_Register_Subclasses();

#ifdef TARG_ST
  // [CG] Implemented in targ_register.cxx.
  CGTARG_REGISTER_Begin();
  CGTARG_DW_DEBUG_Begin();
#endif
}

struct Dont_Allocate_Dreg
{
  inline void operator() (UINT32, ST_ATTR *st_attr) const {
    if (ST_ATTR_kind (*st_attr) != ST_ATTR_DEDICATED_REGISTER)
      return;
    PREG_NUM preg = ST_ATTR_reg_id(*st_attr);
    ISA_REGISTER_CLASS rclass;
    REGISTER reg;
    CGTARG_Preg_Register_And_Class(preg, &rclass, &reg);
    REGISTER_Set_Allocatable (rclass, reg, FALSE /* is_allocatable */);
  }
};
 
/* ====================================================================
 *   REGISTER_Pu_Begin
 *
 *   See interface description
 * ====================================================================
 */
extern void
REGISTER_Pu_Begin(void)
{
  ISA_REGISTER_CLASS rclass;

  /* Scan all the registers to find if the initial allocation status
   * will be different from the current state. The initial status
   * is all registers are set to their "default".
   */
  FOR_ALL_ISA_REGISTER_CLASS( rclass ) {
    REGISTER reg;
    BOOL re_init = FALSE;

    for ( reg = REGISTER_MIN;
	  reg <= REGISTER_CLASS_last_register(rclass);
	  reg++
    ) {
      if ( reg_alloc_status[rclass][reg] != AS_default) {
	reg_alloc_status[rclass][reg] = AS_default;
	re_init = TRUE;
      }
    }

    if ( re_init ) Initialize_Register_Class(rclass);

    // always reset rotating register set
    REGISTER_CLASS_rotating(rclass) = REGISTER_SET_EMPTY_SET;

#ifdef HAS_STACKED_REGISTERS
    REGISTER_Init_Stacked(rclass);
#endif
  }

  // now check for any registers that user doesn't want allocated
  std::vector< std::pair< ISA_REGISTER_CLASS, REGISTER > >::iterator r;
  for (r = dont_allocate_these_registers.begin(); 
	r != dont_allocate_these_registers.end(); 
	++r)
  {
	REGISTER_Set_Allocatable ((*r).first, (*r).second, FALSE /* is_allocatable */);
  }
  // also check for user register variables in PU (local dreg list).
  if ( ST_ATTR_Table_Size (CURRENT_SYMTAB)) {
    For_all (St_Attr_Table, CURRENT_SYMTAB, 
	Dont_Allocate_Dreg());
  }

#ifdef TARG_ST
  // [CG] Implemented in targ_register.cxx.
  CGTARG_REGISTER_Pu_Begin();
#endif

  if ( Get_Trace(TP_MISC, 0x100) ) REGISTER_CLASS_Trace_All();
}

/* ====================================================================
 *   REGISTER_Reset_FP ()
 *
 *   possibly reset fp to non-allocatable if need a frame pointer
 * ====================================================================
 */
void
REGISTER_Reset_FP (void)
{
  ISA_REGISTER_CLASS rclass;
  if (FRAME_POINTER_REQUIRED_FOR_PU && FP_TN != NULL) {
    rclass = TN_register_class(FP_TN);
    reg_alloc_status[rclass][TN_register(FP_TN)] = AS_not_allocatable;
    Initialize_Register_Class(rclass);
  }
}

/* ====================================================================
 * ====================================================================
 *
 * REGISTER_SET functions
 *
 * ====================================================================
 * ====================================================================
 */


/* ====================================================================
 *
 *  REGISTER_SET_Difference_Range
 *
 *  See interface description
 *
 * ====================================================================
 */
REGISTER_SET
REGISTER_SET_Difference_Range(
  REGISTER_SET   set,
  REGISTER       low,
  REGISTER       high
)
{
#ifdef TARG_ST
  FmtAssert ((INT)high >= (INT)low,
	     ("REGISTER_SET_Difference_Range: high < low"));
#endif
  return REGISTER_SET_Difference(set, REGISTER_SET_Range(low, high));
}


/* ====================================================================
 *
 *  REGISTER_SET_CHOOSE_ENGINE
 *
 *  The guts of the REGISTER_SET_Choose... functions. Return the index
 *  (1-based) of the first set bit in 'set'.
 *
 * ====================================================================
 */
inline REGISTER REGISTER_SET_Choose_Engine(
  REGISTER_SET set
)
{
  INT i = 0;
  do {
    REGISTER_SET_WORD w = REGISTER_SET_ELEM(set, i);
    INT regbase = REGISTER_MIN + (i * sizeof(REGISTER_SET_WORD) * 8);
    do {
      REGISTER_SET_WORD lowb = w & 0xff;
      if ( lowb ) return regbase + UINT8_least_sig_one[lowb];
    } while (regbase += 8, w >>= 8);
  } while (++i <= MAX_REGISTER_SET_IDX);
  return REGISTER_UNDEFINED;
}


/* ====================================================================
 *
 *  REGISTER_SET_Choose
 *
 *  See interface description
 *
 * ====================================================================
 */
extern REGISTER
REGISTER_SET_Choose(
  REGISTER_SET set
)
{
  return REGISTER_SET_Choose_Engine(set);
}


/* ====================================================================
 *
 *  REGISTER_SET_Choose_Range
 *
 *  See interface description
 *
 * ====================================================================
 */

extern REGISTER
REGISTER_SET_Choose_Range(
  REGISTER_SET set,
  REGISTER     low,
  REGISTER     high
)
{
  if (low > REGISTER_MAX) {
    return REGISTER_UNDEFINED;
  } else {
    REGISTER_SET temp;
    temp = REGISTER_SET_Intersection(set, REGISTER_SET_Range(low, high));
    return REGISTER_SET_Choose_Engine(temp);
  }
}


/* ====================================================================
 *
 *  REGISTER_SET_Choose_Next
 *
 *  See interface description
 *
 * ====================================================================
 */
extern REGISTER
REGISTER_SET_Choose_Next(
  REGISTER_SET set,
  REGISTER     reg
)
{
  if ( reg >= REGISTER_MAX ) {
    return REGISTER_UNDEFINED;
  } else {
    REGISTER_SET temp;
    temp = REGISTER_SET_Difference(set, REGISTER_SET_Range(REGISTER_MIN, reg));
    return REGISTER_SET_Choose_Engine(temp);
  }
}


/* ====================================================================
 *
 *  REGISTER_SET_Choose_Intersection
 *
 *  See interface description
 *
 * ====================================================================
 */
extern REGISTER
REGISTER_SET_Choose_Intersection(
  REGISTER_SET set1,
  REGISTER_SET set2
)
{
  REGISTER_SET set = REGISTER_SET_Intersection(set1, set2);
  return REGISTER_SET_Choose(set);
}


/* ====================================================================
 *
 *  REGISTER_SET_Size
 *
 *  See interface description
 *
 * ====================================================================
 */
extern INT32
REGISTER_SET_Size(
  REGISTER_SET set
)
{
  INT32 size = 0;
  INT i = 0;
  do {
    REGISTER_SET_WORD w = REGISTER_SET_ELEM(set, i);
    do {
      size += UINT8_pop_count[w & 0xff];
    } while (w >>= 8);
  } while (++i <= MAX_REGISTER_SET_IDX);
  return size;
}


/* ====================================================================
 *
 *  REGISTER_SET_Print
 *
 *  Prints out a register set
 *
 * ====================================================================
 */
extern void
REGISTER_SET_Print(
  REGISTER_SET regset,
  FILE *f
)
{
  REGISTER    i;
  const char *sep = "";

  fprintf(f, "[");
  for ( i = REGISTER_SET_Choose(regset);
        i != REGISTER_UNDEFINED;
        i = REGISTER_SET_Choose_Next(regset,i)
  ) {
    fprintf(f, "%s%d", sep, i);
    sep = ",";
  }

  fprintf(f, "]");
}

/* ====================================================================
 * ====================================================================
 *
 * REGISTER_CLASS functions
 *
 * ====================================================================
 * ====================================================================
 */

/* ====================================================================
 *
 *  REGISTER_CLASS_OP_Update_Mapping
 *
 *  See interface description
 *
 * ====================================================================
 */
extern void
REGISTER_CLASS_OP_Update_Mapping(
    OP *op
)
{
  INT32 i;
  const ISA_OPERAND_INFO *oinfo = ISA_OPERAND_Info(OP_code(op));

  for (i = OP_results(op) - 1; i >= 0; --i) {
    TN *tn = OP_result(op,i);

    if (    TN_is_register(tn)
         && TN_register_class(tn) == ISA_REGISTER_CLASS_UNDEFINED
    ) {
      const ISA_OPERAND_VALTYP *otype = ISA_OPERAND_INFO_Result(oinfo, i);
      ISA_REGISTER_CLASS rclass = ISA_OPERAND_VALTYP_Register_Class(otype);
      Set_TN_register_class(tn, rclass);
    }
  }

  for ( i = OP_opnds(op) - 1; i >= 0; --i ) {
    TN *tn = OP_opnd(op,i);

    if (    TN_is_register(tn)
         && TN_register_class(tn) == ISA_REGISTER_CLASS_UNDEFINED
    ) {
      const ISA_OPERAND_VALTYP *otype = ISA_OPERAND_INFO_Operand(oinfo, i);
      ISA_REGISTER_CLASS rclass = ISA_OPERAND_VALTYP_Register_Class(otype);
      Set_TN_register_class(tn, rclass);
    }
  }
}

/* ====================================================================
 * ====================================================================
 *
 * REGISTER functions
 *
 * ====================================================================
 * ====================================================================
 */


/* ====================================================================
 *
 *  REGISTER_Print
 *
 *  Prints out a register to a specified file.
 *
 * ====================================================================
 */
extern void
REGISTER_Print(
  ISA_REGISTER_CLASS rclass,
  REGISTER reg,
  FILE *f
)
{
  fprintf(f, REGISTER_name(rclass, reg));
}

/* ====================================================================
 *
 *  CLASS_REG_PAIR_Print
 *
 *  Prints out a register to a specified file.
 *
 * ====================================================================
 */
extern void
CLASS_REG_PAIR_Print(
  CLASS_REG_PAIR crp,
  FILE *f
)
{
  REGISTER_Print(CLASS_REG_PAIR_rclass(crp), CLASS_REG_PAIR_reg(crp),f);
}

/* ====================================================================
 *
 *  REGISTER_Set_Allocatable
 *
 *  See interface description
 *
 * ====================================================================
 */
void
REGISTER_Set_Allocatable(
  ISA_REGISTER_CLASS rclass,
  REGISTER           reg,
  BOOL               is_allocatable
)
{
  INT prev_status = reg_alloc_status[rclass][reg];
  INT new_status  = is_allocatable ? AS_allocatable : AS_not_allocatable;

  if ( prev_status != new_status ) {
    reg_alloc_status[rclass][reg] = new_status;

    Initialize_Register_Class(rclass);
  }
}

/* ====================================================================
 * ====================================================================
 *
 * Tracing
 *
 * ====================================================================
 * ====================================================================
 */


#define TRUE_FALSE(b) ((b) ? "true" : "false")


/* =======================================================================
 *
 *  REGISTER_SET_Print_Name
 *
 *  
 *
 * =======================================================================
 */
static void
REGISTER_SET_Print_Name(
  ISA_REGISTER_CLASS rclass,
  REGISTER_SET regset,
  FILE *f
)
{
  REGISTER i;
  char    *sep = "";

  fprintf(f, "[");
  for ( i = REGISTER_SET_Choose(regset);
        i != REGISTER_UNDEFINED;
        i = REGISTER_SET_Choose_Next(regset,i)
  ) {
    fprintf(f, "%s%s",sep,REGISTER_name(rclass,i));
    sep = ",";
  }

  fprintf(f, "]");
}


/* =======================================================================
 *
 *  REGISTER_Trace
 *
 *  See interface description
 *
 * =======================================================================
 */
extern void REGISTER_Trace(
  ISA_REGISTER_CLASS rclass,
  REGISTER reg
)
{
  if (    reg < REGISTER_MIN 
       || reg > REGISTER_CLASS_last_register(rclass) ) return;

  fprintf(TFile, "  reg %2d:"
		 " name=%-5s"
		 " bit-size=%-3d"
		 " mach-id=%-2d"
		 " allocatable=%-5s\n",
		 reg,
		 REGISTER_name(rclass, reg),
		 REGISTER_bit_size(rclass, reg),
		 REGISTER_machine_id(rclass, reg),
		 TRUE_FALSE(REGISTER_allocatable(rclass, reg)));
}


/* =======================================================================
 *
 *  REGISTER_CLASS_Trace
 *
 *  See interface description
 *
 * =======================================================================
 */
extern void REGISTER_CLASS_Trace(
  ISA_REGISTER_CLASS rclass
)
{
  REGISTER reg;
  REGISTER_SET set;

  fprintf(TFile, "register class %d (%s) register-count=%d can-store=%s\n",
		 rclass, REGISTER_CLASS_name(rclass),
		 REGISTER_CLASS_register_count(rclass),
		 TRUE_FALSE(REGISTER_CLASS_can_store(rclass)));

  for ( reg = REGISTER_MIN; reg <= REGISTER_MAX; reg++ ) {
    REGISTER_Trace(rclass, reg);
  }

  fprintf(TFile, "\n  universe: ");
  REGISTER_SET_Print(REGISTER_CLASS_universe(rclass), TFile);
  fprintf(TFile, "\n");

  set = REGISTER_CLASS_allocatable(rclass);
  if ( !REGISTER_SET_EmptyP(set) ) {
    fprintf(TFile, "  allocatable: ");
    REGISTER_SET_Print(set, TFile);
    fprintf(TFile, "\n");
  }

  set = REGISTER_CLASS_callee_saves(rclass);
  if ( !REGISTER_SET_EmptyP(set) ) {
    fprintf(TFile, "  callee_saves: ");
    REGISTER_SET_Print(set, TFile);
    fprintf(TFile, "\n");
  }

  set = REGISTER_CLASS_caller_saves(rclass);
  if ( !REGISTER_SET_EmptyP(set) ) {
    fprintf(TFile, "  caller_saves: ");
    REGISTER_SET_Print(set, TFile);
    fprintf(TFile, "\n");
  }

  set = REGISTER_CLASS_function_argument(rclass);
  if ( !REGISTER_SET_EmptyP(set) ) {
    fprintf(TFile, "  function_argument: ");
    REGISTER_SET_Print(set, TFile);
    fprintf(TFile, "\n");
  }

  set = REGISTER_CLASS_function_value(rclass);
  if ( !REGISTER_SET_EmptyP(set) ) {
    fprintf(TFile, "  function_value: ");
    REGISTER_SET_Print(set, TFile);
    fprintf(TFile, "\n");
  }

  set = REGISTER_CLASS_shrink_wrap(rclass);
  if ( !REGISTER_SET_EmptyP(set) ) {
    fprintf(TFile, "  shrink_wrap: ");
    REGISTER_SET_Print(set, TFile);
    fprintf(TFile, "\n");
  }

  fprintf(TFile, "\n  universe: ");
  REGISTER_SET_Print_Name(rclass, REGISTER_CLASS_universe(rclass), TFile);
  fprintf(TFile, "\n");

  set = REGISTER_CLASS_allocatable(rclass);
  if ( !REGISTER_SET_EmptyP(set) ) {
    fprintf(TFile, "  allocatable: ");
    REGISTER_SET_Print_Name(rclass, set, TFile);
    fprintf(TFile, "\n");
  }

  set = REGISTER_CLASS_callee_saves(rclass);
  if ( !REGISTER_SET_EmptyP(set) ) {
    fprintf(TFile, "  callee_saves: ");
    REGISTER_SET_Print_Name(rclass, set, TFile);
    fprintf(TFile, "\n");
  }

  set = REGISTER_CLASS_caller_saves(rclass);
  if ( !REGISTER_SET_EmptyP(set) ) {
    fprintf(TFile, "  caller_saves: ");
    REGISTER_SET_Print_Name(rclass, set, TFile);
    fprintf(TFile, "\n");
  }

  set = REGISTER_CLASS_function_argument(rclass);
  if ( !REGISTER_SET_EmptyP(set) ) {
    fprintf(TFile, "  function_argument: ");
    REGISTER_SET_Print_Name(rclass, set, TFile);
    fprintf(TFile, "\n");
  }

  set = REGISTER_CLASS_function_value(rclass);
  if ( !REGISTER_SET_EmptyP(set) ) {
    fprintf(TFile, "  function_value: ");
    REGISTER_SET_Print_Name(rclass, set, TFile);
    fprintf(TFile, "\n");
  }

  set = REGISTER_CLASS_shrink_wrap(rclass);
  if ( !REGISTER_SET_EmptyP(set) ) {
    fprintf(TFile, "  shrink_wrap: ");
    REGISTER_SET_Print_Name(rclass, set, TFile);
    fprintf(TFile, "\n");
  }
}


/* =======================================================================
 *   REGISTER_CLASS_Trace_All
 *
 *   See interface description
 * =======================================================================
 */
extern void
REGISTER_CLASS_Trace_All(void)
{
  ISA_REGISTER_CLASS rclass;

  fprintf(TFile, "\n%s"
                 " REGISTERs and ISA_REGISTER_CLASSes for PU \"%s\"\n"
                 "%s",
                 DBar, Cur_PU_Name, DBar);

  FOR_ALL_ISA_REGISTER_CLASS( rclass ) {
    fprintf(TFile, "\n");
    REGISTER_CLASS_Trace(rclass);
  }
}

// user wants given register to not be allocatable in file.
void
Set_Register_Never_Allocatable (char *regname) 
{
#ifdef TARG_ST
  //TB: Change the way register can be define with -ffixed-reg=%rclassname%reg_name
  int regnum;
  ISA_REGISTER_CLASS rclass = Register_Class_Num_From_Name(regname, &regnum);
#else
	ISA_REGISTER_CLASS rclass = CGTARG_Regname_Register_Class(regname);
#endif
	REGISTER reg;

	if (rclass == ISA_REGISTER_CLASS_UNDEFINED)
	  ErrMsg (EC_Inv_Register, regname);

#ifdef TARG_ST
	reg = REGISTER_MIN + regnum;
#else
	reg = REGISTER_MIN + atoi(regname+1);
#endif
	if (reg > REGISTER_CLASS_last_register(rclass))
	  ErrMsg (EC_Inv_Register, regname);

	dont_allocate_these_registers.push_back( std::make_pair( rclass, reg ));
}

// user wants given register to not be allocatable in file.
void
Set_Register_Never_Allocatable (PREG_NUM preg) 
{
  ISA_REGISTER_CLASS rclass;
  REGISTER reg;
  CGTARG_Preg_Register_And_Class(preg, &rclass, &reg);
  dont_allocate_these_registers.push_back( std::make_pair( rclass, reg ));
}


#ifdef TARG_ST
/* 
 * See interface description. 
 */
ISA_REGISTER_CLASS
Register_Class_For_Mtype(TYPE_ID mtype)
{
  return CGTARG_Register_Class_For_Mtype(mtype);
}

ISA_REGISTER_CLASS
Register_Subclass_For_Mtype(TYPE_ID mtype)
{
  return CGTARG_Register_Subclass_For_Mtype(mtype);
}

ISA_REGISTER_CLASS
Register_Class_Num_From_Name(char *regname, INT32 *regnum)
{
  return CGTARG_Register_Class_Num_From_Name(regname, regnum);
}

char *
ISA_REGISTER_CLASS_Symbol(ISA_REGISTER_CLASS rc)
{
  FmtAssert(0, ("Obsolete function for virtual register support. Not implemented"));
  return NULL;
}

char *
ISA_REGISTER_CLASS_ASM_Name(ISA_REGISTER_CLASS rc)
{
  FmtAssert(0, ("Obsolete function for virtual register support. Not implemented"));
  return NULL;
}


/*
 * [CG]: Comment valid up to "[CG]: End of Comment" below.
 * These functions are not useful for our targets. 
 * For now return a conservative answer for all target.
 * If needed these should be put above and implemented as a call to a target
 * dependent function CGTARG_REGISTER_...() in targ_register.cxx.
 */
BOOL 
REGISTER_Is_Rotating(ISA_REGISTER_CLASS rclass, REGISTER reg)
{
  return FALSE;
}

REGISTER_SET
REGISTER_Get_Requested_Rotating_Registers (ISA_REGISTER_CLASS rclass)
{
  return REGISTER_SET_EMPTY_SET;
}

BOOL
Is_Predicate_REGISTER_CLASS(ISA_REGISTER_CLASS rclass)
{
  return FALSE;
}

BOOL
REGISTER_Has_Stacked_Registers(ISA_REGISTER_CLASS rclass)
{
  return FALSE;
}

BOOL
REGISTER_Has_Rotating_Registers(ISA_REGISTER_CLASS rclass)
{
  return FALSE;
}
/* [CG]: End of Comment on non useful functions. */

#endif
