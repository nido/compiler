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
 * $Revision$
 * $Date$
 * $Author$
 * $Source$
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
#include <vector.h>
#include "defs.h"
#include "errors.h"
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
REGISTER_SUBCLASS_INFO REGISTER_SUBCLASS_info[ISA_REGISTER_SUBCLASS_MAX + 1];
ISA_REGISTER_CLASS  REGISTER_CLASS_vec[ISA_REGISTER_CLASS_MAX + 1];
REGISTER_CLASS_INFO REGISTER_CLASS_info[ISA_REGISTER_CLASS_MAX + 1];

CLASS_REG_PAIR      CLASS_REG_PAIR_zero;
CLASS_REG_PAIR      CLASS_REG_PAIR_ep;
CLASS_REG_PAIR      CLASS_REG_PAIR_gp;
CLASS_REG_PAIR      CLASS_REG_PAIR_sp;
CLASS_REG_PAIR      CLASS_REG_PAIR_fp;
CLASS_REG_PAIR      CLASS_REG_PAIR_ra;
CLASS_REG_PAIR      CLASS_REG_PAIR_v0;
CLASS_REG_PAIR      CLASS_REG_PAIR_static_link;
CLASS_REG_PAIR      CLASS_REG_PAIR_pfs;
CLASS_REG_PAIR      CLASS_REG_PAIR_lc;
CLASS_REG_PAIR      CLASS_REG_PAIR_ec;
CLASS_REG_PAIR      CLASS_REG_PAIR_true;
CLASS_REG_PAIR      CLASS_REG_PAIR_fzero;
CLASS_REG_PAIR      CLASS_REG_PAIR_fone;
CLASS_REG_PAIR      CLASS_REG_PAIR_link;

const CLASS_REG_PAIR CLASS_REG_PAIR_undef =
  {CREATE_CLASS_N_REG(ISA_REGISTER_CLASS_UNDEFINED,REGISTER_UNDEFINED)};

#if ISA_REGISTER_MAX >= 64
const REGISTER_SET REGISTER_SET_EMPTY_SET = { 0 };
#endif /* ISA_REGISTER_MAX >= 64 */


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
  char regname[8];
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
  char regname[8];
  char regname2[8];

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
 *   Initialize_Register_Classes
 * ====================================================================
 */
static void
Initialize_Register_Classes ()
{
  ISA_REGISTER_CLASS rclass;

  /*  Create the register classes for all the target registers.
   */
  FOR_ALL_ISA_REGISTER_CLASS( rclass ) {
    Initialize_Register_Class(rclass);
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
  /*  Create the register classes for all the target registers.
   */
  Initialize_Register_Classes ();
  Initialize_Register_Subclasses();
  Init_Mtype_RegClass_Map();
  return;
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
    if (REGISTER_set_not_allocatable (rclass, reg))
      Initialize_Register_Class(rclass);
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
    if (REGISTER_Check_Alloc_Status(rclass)) 
      Initialize_Register_Class(rclass);
  }

  Set_Register_Not_Allocatable ();

  // also check for user register variables in PU (local dreg list).
  if ( ST_ATTR_Table_Size (CURRENT_SYMTAB)) {
    For_all (St_Attr_Table, CURRENT_SYMTAB, 
	Dont_Allocate_Dreg());
  }

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
  BOOL status;

  if (FRAME_POINTER_REQUIRED_FOR_PU && FP_TN != NULL) {
    rclass = TN_register_class(FP_TN);
    status = REGISTER_set_not_allocatable (rclass, TN_register(FP_TN));
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

