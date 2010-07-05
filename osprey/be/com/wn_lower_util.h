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

/* ====================================================================
 *
 * Module: wn_lower_util.h
 *
 * Description:
 *
 * This module provides common support functions to the lowering phases
 * invoqued by the wn_lower.h module.
 *
 * Interface:
 *
 * This interface is private to be.so it should not be exported to
 * other compiler components.
 * 
 *
 * Lowering State Support
 * ======================
 *
 * typedef CURRENT_STATE
 * CURRENT_STATE current_state
 * 
 * This type maintains information to locate a point in the lowered tree.
 * The current_state variable holds the current location while traversing the
 * whirl tree.
 * At any time during the lowering the current state can be accessed,
 * modified, saved or restored (for nesting traversals).
 * The information available from the state is:
 *   srcpos: the source position
 *   stmt: the statement that defines the location in the whirl tree
 *   function: the processed function when lowering a function
 *   actions: the applied lowering actions
 *  
 *	#define	current_srcpos		current_state.srcpos
 *	#define	current_stmt		current_state.stmt
 *	#define	current_actions		current_state.actions
 *	#define current_function	current_state.function
 *
 * Short names for accesing the current state informations.
 *
 *
 * void setCurrentState(WN *tree, LOWER_ACTIONS actions);
 * void setCurrentStateBlockFirst(WN *tree, LOWER_ACTIONS actions);
 * void setCurrentStateBlockLast(WN *tree, LOWER_ACTIONS actions);
 *
 *   these functions modify the current state to set the new actions or
 *   statement location. The two last functions set the location to
 *   the first or last statement of a block respectively. the passed tree
 *   must be a block tree.
 *
 * CURRENT_STATE pushCurrentState(WN *tree, LOWER_ACTIONS actions);
 *
 *   This function set a new state for the current state and returns the old
 *   one. It is used for nested traversals.
 *
 * void popCurrentState(CURRENT_STATE state);
 *
 *   This function reset the current state to the given state. It is used
 *   for nested traversals.
 *
 *
 * Assignment Support
 * ==================
 *
 * PREG_NUM AssignPregExprPos(WN *block, WN *tree, TY_IDX ty, SRCPOS srcpos,
 *                            LOWER_ACTIONS actions)
 *
 * PREG_NUM AssignExpr(WN *block, WN *tree, TYPE_ID type)
 *
 * PREG_NUM AssignExprTY(WN *block, WN *tree, TY_IDX ty)
 *
 *   All the above functions allocate a preg of the given type and assign
 *   expression tree to it, and attach the generated statement to the block. 
 *   For the functions where the actions are not specified, the current
 *   lowering state actions are used (see current_actions above).
 *   If specified and not null, attach srcpos to the statement.
 *   The created preg will have the name specified by the
 *   value of CurrentPregName(), see below.
 *
 *   void AssignPregName(const char *part1, const char *part2 = NULL)
 *   char *CurrentPregName()
 *   void ResetPregName(void)
 *
 *   AssignPregName() construct a name by concatenating the two arguments
 *   and set this name for the PREGs that will be allocated by the Assign...()
 *   functions above. CurrentPregName() returns the currently defined
 *   name or NULL if no name has been assigned to the preg allocation.
 *   The name can be used to allocate explicitly a PREG with Create_Preg()
 *   (defined in symtab.h).
 *   ResetPregName() resets the allocated PREGs name to NULL.
 *   If the current PREG name is NULL, the PREGs will have a canonical name
 *   as defined by Create_Preg().
 *
 * void AssignToPregExprPos(PREG_NUM pregNo, WN *block, WN *tree, TY_IDX ty,
 *                          SRCPOS srcpos, LOWER_ACTIONS actions)
 *
 * void AssignToPregExprTY(PREG_NUM pregNo, WN *block, WN *tree, TY_IDX type)
 *
 * void AssignToPregExpr(PREG_NUM pregNo, WN *block, WN *tree, TYPE_ID type)
 *
 *   All these functions assign the given expression to the preg specified.
 *   These function do the same as the Equivelent AssignPreg...() functions
 *   but for an available preg.
 *
 * 
 * LEAF WHIRL Tree Support
 * =======================
 *
 * LEAF Make_Leaf(WN *block, WN *tree, TYPE_ID type)
 *
 *   Make an arbitrary expression tree into a leaf.
 *   If the expression is an integer or floating point constant
 *   no transformation is made. However, other expressions are stored
 *   into a PREG.
 *
 *   Make_Leaf is used in place of AssignExpr when performing a sort
 *   of "poor man's" CSE, and you want to avoid creating unnecessary
 *   PREGs for constants (which can also thwart the simplifier).
 *
 * WN *Load_Leaf(const LEAF &leaf)
 *
 *   Generate whirl to load the value of a leaf expression created by
 *   Make_Leaf().
 *
 *
 * WHIRL Tree Queries
 * ==================
 *
 * BOOL Is_Intconst_Val(WN *tree)
 * INT64 Get_Intconst_Val(WN *tree)
 *
 *   The Is_Intconst_Val() function returns whether the given tree is an 
 *   INTCONST tree. It may use rematerialization information to  to retrieve
 *   the information more globally.
 *   The Get_Intconst_Val() function returns the effective literal value
 *   of the tree. It must be used only if Is_Intconst_Val() returned true.
 *
 * BOOL WN_Is_Emulated_Operator (OPERATOR opr, TYPE_ID rtype, TYPE_ID desc);
 *
 *  Returns true if the (operator, rtype, desc) pair requires emulation.
 *  If false is returned, the WHIRL operator tree will be kept and
 *  must be handle by the code selector.
 *  rtype and desc may be different for conversion operators.
 *
 * BOOL WN_Is_Emulated_Type (TYPE_ID type);
 *
 *  Returns true when the emulation is forced for the given type.
 *  When true, the operators using this type should be emulated.
 *
 * BOOL WN_Is_Emulated (WN *tree);
 *
 *  Returns true when the emulation is required for the given tree node.
 *  Should be used in place of WN_Is_Emulated_Operator() when possible.
 *
 * BOOL WN_Madd_Allowed (TYPE_ID type);
 *
 *  Returns true if generation of MADD and alike WHIRL operators is
 *  activated for this type.
 *
 * BOOL WN_STBITS_Allowed (TYPE_ID type);
 *
 *  Returns true if generation of [I]STBITS is
 *  activated for this type. Generally this means that the code 
 *  selector can handle STBITS.
 *
 * BOOL WN_LDBITS_Allowed (TYPE_ID type);
 *
 *  Returns true if generation of [I]LDBITS is
 *  activated for this type. Generally this means that the code 
 *  selector can handle LDBITS.
 *
 * ===================================================================
 */

#ifndef wn_lower_util_INCLUDED
#define wn_lower_util_INCLUDED

#include "wn_lower.h"

/* ====================================================================
 * Lowering State Support.
 * ====================================================================
 */
typedef struct CURRENT_STATE
{
  SRCPOS	srcpos;
  WN		*stmt;
  WN		*function;
  LOWER_ACTIONS	actions;
} CURRENT_STATE;

extern CURRENT_STATE current_state;

#define	current_srcpos		current_state.srcpos
#define	current_stmt		current_state.stmt
#define	current_actions		current_state.actions
#define current_function	current_state.function

extern void setCurrentState(WN *tree, LOWER_ACTIONS actions);
extern void setCurrentStateBlockFirst(WN *tree, LOWER_ACTIONS actions);
extern void setCurrentStateBlockLast(WN *tree, LOWER_ACTIONS actions);
extern CURRENT_STATE pushCurrentState(WN *tree, LOWER_ACTIONS actions);
extern void popCurrentState(CURRENT_STATE state);

/* ==================================================================== */


/* ====================================================================
 * Assignment Support
 * ====================================================================
 */
extern void AssignPregName(const char *part1, const char *part2 = NULL);
extern char *CurrentPregName();
extern void ResetPregName(void);

extern PREG_NUM AssignPregExprPos(WN *block, WN *tree, TY_IDX ty,
				  SRCPOS srcpos, LOWER_ACTIONS actions);

extern PREG_NUM AssignExprTY(WN *block, WN *tree, TY_IDX type);
extern PREG_NUM AssignExpr(WN *block, WN *tree, TYPE_ID type);

extern void AssignToPregExprPos(PREG_NUM pregNo, 
				WN *block, WN *tree, TY_IDX ty,
				SRCPOS srcpos, LOWER_ACTIONS actions);
extern void
AssignToPregExprTY(PREG_NUM pregNo, WN *block, WN *tree, TY_IDX type);
extern void
AssignToPregExpr(PREG_NUM pregNo, WN *block, WN *tree, TYPE_ID type);
     

/* ==================================================================== */


/* ====================================================================
 * LEAF WHIRL Tree Support
 * ====================================================================
 */
typedef enum { LEAF_IS_CONST, LEAF_IS_INTCONST, LEAF_IS_PREG} LEAF_KIND;
typedef struct {
  LEAF_KIND kind;
  TYPE_ID type;
  union {
    PREG_NUM n;
    INT64 intval;
    TCON tc;
  } u;
} LEAF;

extern LEAF Make_Leaf(WN *block, WN *tree, TYPE_ID type);
extern WN *Load_Leaf(const LEAF &leaf);

/* ==================================================================== */

/* ====================================================================
 * WHIRL Tree Queries
 * ====================================================================
 */

extern BOOL Is_Intconst_Val(WN *tree);
extern INT64 Get_Intconst_Val(WN *tree);

extern BOOL WN_Is_Emulated(WN *tree);
extern BOOL WN_Is_Emulated_Type (TYPE_ID type);
extern BOOL WN_Is_Emulated_Operator (OPERATOR opr, TYPE_ID rtype, TYPE_ID desc);
extern BOOL WN_Madd_Allowed (TYPE_ID type);
extern BOOL WN_LDBITS_Allowed (TYPE_ID type);
extern BOOL WN_STBITS_Allowed (TYPE_ID type);


/* ==================================================================== */

#endif /* wn_lower_util_INCLUDED */
