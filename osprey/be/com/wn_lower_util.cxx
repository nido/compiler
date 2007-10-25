/*

  Copyright (C) 2005-2006 ST Microelectronics, Inc.  All Rights Reserved.

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

  Contact information:  ST Microelectronics, Inc., 
  address, or:

  http://www.st.com

*/

#include "defs.h"
#include "config.h"
#include "errors.h"
#include "wn.h"
#include "wn_simp.h"
#include "wn_util.h"
#include "const.h"
#include "be_symtab.h"
#include "wn_lower.h"
#include "wn_lower_private.h"
#include "wn_lower_util.h"
#ifdef TARG_ST
#include "w2op.h"		/* For can_do_fast_divide. */
#include "betarget.h"           /* For BETARG_... function. */
#include "config_opt.h"		/* For Fused_FP, Fused_Madd. */
#include "config_TARG.h"	/* For Madd_Allowed. */
#endif
/* ====================================================================
 * Lowering State Support.
 * ====================================================================
 */

/*
 * current_state
 *
 * Maintains current state information.
 */
CURRENT_STATE current_state;

/*
 * setCurrentState()
 *
 * See Interface Description.
 */
void
setCurrentState(WN *tree, LOWER_ACTIONS actions)
{
  if (tree) {
    current_stmt =	tree;
    current_srcpos =	WN_Get_Linenum(tree);
    current_actions =	actions;

    if (WN_opcode(tree) == OPC_FUNC_ENTRY)
      current_function = tree;
  }
}

/*
 * setCurrentStateBlockFirst()
 *
 * See Interface Description.
 */
void
setCurrentStateBlockFirst(WN *tree, LOWER_ACTIONS actions)
{
  Is_True(WN_opcode(tree) == OPC_BLOCK, ("expected BLOCK node"));

  setCurrentState(WN_first(tree), actions);
}

/*
 * setCurrentStateBlockLast()
 *
 * See Interface Description.
 */
void
setCurrentStateBlockLast(WN *tree, LOWER_ACTIONS actions)
{
  Is_True(WN_opcode(tree) == OPC_BLOCK, ("expected BLOCK node"));

  setCurrentState(WN_last(tree), actions);
}


/*
 * pushCurrentState()
 *
 * See Interface Description.
 */
CURRENT_STATE
pushCurrentState(WN *tree, LOWER_ACTIONS actions)
{
  CURRENT_STATE saveState = current_state;

  setCurrentState(tree, actions);

  return saveState;
}

/*
 * popCurrentState()
 *
 * See Interface Description.
 */
void 
popCurrentState(CURRENT_STATE state)
{
  current_state = state;
}


/* ====================================================================
 * LEAF WHIRL Tree Support
 * ====================================================================
 */

/*
 * LEAF Make_Leaf(WN *block, WN *tree, TYPE_ID type)
 *
 * See Interface Description.
 */
LEAF
Make_Leaf(WN *block, WN *tree, TYPE_ID type)
{
  LEAF leaf;
  leaf.type = type;
  switch (WN_operator(tree)) {
  case OPR_CONST:
    leaf.kind = LEAF_IS_CONST;
    leaf.u.tc = Const_Val(tree);
    WN_Delete(tree);
    break;
  case OPR_INTCONST:
    leaf.kind = LEAF_IS_INTCONST;
    leaf.u.intval = WN_const_val(tree);
    WN_Delete(tree);
    break;
  default:
    leaf.kind = LEAF_IS_PREG;
    leaf.u.n = AssignExpr(block, tree, type);
    break;
  }
  return leaf;
}

/*
 * WN *Load_Leaf(const LEAF &leaf)
 *
 * See Interface Description.
 */
WN *
Load_Leaf(const LEAF &leaf)
{
  switch (leaf.kind) {
  case LEAF_IS_CONST:
    return Make_Const(leaf.u.tc);
  case LEAF_IS_INTCONST:
    return WN_CreateIntconst(OPR_INTCONST, leaf.type, MTYPE_V, leaf.u.intval);
  case LEAF_IS_PREG:
    return WN_LdidPreg(leaf.type, leaf.u.n);
  }
  FmtAssert(FALSE, ("unhandled leaf kind in Load_Leaf"));
  /*NOTREACHED*/
}


/* ====================================================================
 * Assignment Support
 * ====================================================================
 */

/* Current name used for PREG allocation. */
static char *current_preg_name;

/*
 * AssignPregName()
 *
 * See interface description.
 */
void
AssignPregName(const char *part1, const char *part2)
{
  static char name[128];
  if (part2 == NULL)
    snprintf(name, sizeof(name), "%s", part1);
  else
    snprintf(name, sizeof(name), "%s%s", part1, part2);
  current_preg_name = name;
}

/*
 * ResetPregName()
 *
 * See interface description.
 */
void
ResetPregName(void)
{
  current_preg_name = NULL;
}

/*
 * CurrentPregName()
 *
 * See interface description.
 */
char *
CurrentPregName(void)
{
  return current_preg_name;
}

/*
 * AssignPregExprPos()
 *
 * See Interface Description.
 */
PREG_NUM
AssignPregExprPos(WN *block, WN *tree, TY_IDX ty,
		  SRCPOS srcpos, LOWER_ACTIONS actions)
{
  PREG_NUM	pregNo;
  TYPE_ID	type;

  Is_True((WN_operator_is(tree, OPR_PARM)==FALSE),("bad parm"));

  type = TY_mtype(Ty_Table[ty]);
  pregNo = Create_Preg(type, current_preg_name);
  
  AssignToPregExprPos(pregNo, block, tree, ty, srcpos, actions);

  return pregNo;
}


/*
 * AssignExprTY()
 *
 * See Interface Description.
 */
PREG_NUM
AssignExprTY(WN *block, WN *tree, TY_IDX type)
{
  return AssignPregExprPos(block, tree, type, current_srcpos,
			   current_actions);
}

/*
 * AssignExpr()
 *
 * See Interface Description.
 */
PREG_NUM
AssignExpr(WN *block, WN *tree, TYPE_ID type)
{
  return AssignPregExprPos(block, tree, MTYPE_To_TY(type), current_srcpos,
			   current_actions);
}

/*
 * AssignToPregExprPos()
 *
 * See Interface Description.
 */
void
AssignToPregExprPos(PREG_NUM pregNo, 
		    WN *block, WN *tree, TY_IDX ty,
		    SRCPOS srcpos, LOWER_ACTIONS actions)
{
  TYPE_ID	type;
  ST		*preg = MTYPE_To_PREG(TY_mtype(Ty_Table[ty]));

  Is_True((WN_operator_is(tree, OPR_PARM)==FALSE),("bad parm"));

  type = TY_mtype(Ty_Table[ty]);

  {
    WN	*stBlock, *stid;

    stid = WN_Stid(type, pregNo, preg, ty, tree);

    if (srcpos) {
      WN_Set_Linenum (stid, srcpos);
    }

    stBlock = WN_CreateBlock();

   /*
    *	This lowering may leed to infinite regress if the
    * 	children cannot be lowered (and are allocated a temp, for example) 
    */
    if (actions)
      stid = lower_stmt(stBlock, stid, actions);

    WN_INSERT_BlockLast(stBlock, stid);

    WN_INSERT_BlockLast(block, stBlock);
  }
}

/*
 * AssignToPregExprTY()
 *
 * See Interface Description.
 */
void
AssignToPregExprTY(PREG_NUM pregNo, WN *block, WN *tree, TY_IDX type)
{
  AssignToPregExprPos(pregNo, block, tree, type, current_srcpos,
		      current_actions);
}

/*
 * AssignToPregExpr()
 *
 * See Interface Description.
 */
void
AssignToPregExpr(PREG_NUM pregNo, WN *block, WN *tree, TYPE_ID type)
{
  AssignToPregExprPos(pregNo, block, tree, MTYPE_To_TY(type), current_srcpos,
			   current_actions);
}


/* ====================================================================
 * WHIRL Tree Queries
 * ====================================================================
 */

/*
 * Get_Intconst_Val()
 *
 * See interface description.
 */
INT64 
Get_Intconst_Val(WN *wn)
{
  WN *intconst = NULL;
  if (WN_operator(wn) == OPR_LDID &&
      WN_class(wn) == CLASS_PREG &&
      Preg_Home(WN_offset(wn)) != NULL &&
      WN_operator_is(Preg_Home(WN_offset(wn)), OPR_INTCONST))
    intconst = Preg_Home(WN_offset(wn));
  else if (WN_operator(wn) == OPR_INTCONST)
    intconst = wn;
  DevAssert(intconst != NULL, ("unexpected call to Get_Intconst_Val"));
  return WN_const_val(intconst);
}

/*
 * Is_Intconst_Val()
 *
 * See interface description.
 */
BOOL 
Is_Intconst_Val(WN *wn)
{
  WN *intconst = NULL;
  if (WN_operator(wn) == OPR_LDID &&
      WN_class(wn) == CLASS_PREG &&
      Preg_Home(WN_offset(wn)) != NULL &&
      WN_operator_is(Preg_Home(WN_offset(wn)), OPR_INTCONST))
    intconst = Preg_Home(WN_offset(wn));
  else if (WN_operator(wn) == OPR_INTCONST)
    intconst = wn;
  return intconst != NULL ? TRUE: FALSE;
}

static BOOL
Should_Call_Divide (WN *tree) 
{
  TYPE_ID rtype = WN_rtype(tree);

  if (WN_operator_is(WN_kid1(tree), OPR_INTCONST)) {
    INT64 constval = WN_const_val(WN_kid1(tree));
    
    if (Can_Do_Fast_Divide(rtype, constval)) return FALSE;
  }
  
  /* In all other cases, we fall back to per operator check. */
  return WN_Is_Emulated_Operator(WN_operator(tree), rtype, WN_desc(tree));
}

static BOOL
Should_Call_Remainder (WN *tree) 
{
  TYPE_ID rtype = WN_rtype(tree);

  if (WN_operator_is(WN_kid1(tree), OPR_INTCONST)) {
    INT64 constval = WN_const_val(WN_kid1(tree));
    
    if (Can_Do_Fast_Remainder(rtype, constval)) return FALSE;
  }

  /* In all other cases, we fall back to per operator check. */
  return WN_Is_Emulated_Operator(WN_operator(tree), rtype, WN_desc(tree));
}


BOOL
WN_Is_Emulated(WN *tree)
{
  OPERATOR opr = WN_operator(tree);

  /* MOD/REM/DIV are treated specifically. */
  switch(opr) {
  case OPR_MOD: 
  case OPR_REM:
    return Should_Call_Remainder(tree);
  case OPR_DIV:
    return Should_Call_Divide(tree);
  default:
    break;
  }

  TYPE_ID res = WN_rtype(tree);
  TYPE_ID desc = WN_desc(tree);
  return WN_Is_Emulated_Operator(opr, res, desc);
}

BOOL
WN_Is_Emulated_Type (TYPE_ID type)
{
  /* This generic conditions are sufficient to decide
     when the type is emulated. */
  if (Only_32_Bit_Ops && MTYPE_is_longlong(type)) return TRUE;
  if (Emulate_Single_Float_Type && type == MTYPE_F4) return TRUE;
  if (Emulate_Double_Float_Type && MTYPE_is_double(type)) return TRUE;

  /* Otherwise call a target dependent function to find more emulated types. */
  return BETARG_is_emulated_type(type);
}

BOOL
WN_Is_Emulated_Operator (OPERATOR opr, TYPE_ID rtype, TYPE_ID desc)
{
  // If floating point emulation is requested explicitly.
  if (Emulate_FloatingPoint_Ops && 
      (MTYPE_is_float(rtype) || MTYPE_is_float(desc)))
    return TRUE;
  
  // Tests to know if operator must be emulated is moved to betarget.cxx
  // in targinfo.
  return BETARG_is_emulated_operator(opr,rtype,desc);
}

BOOL
WN_Madd_Allowed (TYPE_ID type)
{
  if (!Madd_Allowed) return FALSE;
  if (MTYPE_is_float(type) && (!Fused_FP || !Fused_Madd)) return FALSE;
						
  /* We check availability of all MADD operators there.
     It means that all of them must be enabled to activate
     MADD selection.
  */
  return BETARG_is_enabled_operator(OPR_MADD, type, type) &&
    BETARG_is_enabled_operator(OPR_MSUB, type, type) &&
    BETARG_is_enabled_operator(OPR_NMADD, type, type) &&
    BETARG_is_enabled_operator(OPR_NMSUB, type, type);
}

BOOL
WN_STBITS_Allowed (TYPE_ID type)
{
  /* For now we never allow STBITS to go through the middle end
     as it is not supported by WOPT. 
     At low level of optimization we could let it go through as
     it was done before, but I don't see benefit for this.
     The benefit of always returning false here is that the code
     selector does not have to implement it.
  */
  return FALSE;
  /* return BETARG_is_enabled_operator(OPR_STBITS, MTYPE_V, type); */
}

BOOL
WN_LDBITS_Allowed (TYPE_ID type)
{
  /* For the same reason as WN_STBITS_Allowed(), this function return always
     FALSE.
  */
  return FALSE;
  /* return BETARG_is_enabled_operator(OPR_LDBITS, type, type); */
}

