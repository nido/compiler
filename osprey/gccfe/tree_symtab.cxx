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


/* translate gnu decl trees to symtab references */

#include "W_values.h" // for BITSPERBYTE
#include "gnu_config.h"
extern "C" {
#include "gnu/flags.h"
#include "gnu/system.h"
#include "gnu/tree.h"
#include "gnu/toplev.h"
#include "gnu/c-tree.h"
}
#ifdef TARG_IA32
// the definition in gnu/config/i386/i386.h causes problem
// with the enumeration in common/com/ia32/config_targ.h
#undef TARGET_PENTIUM
#endif /* TARG_IA32 */

#include "defs.h"
#include "errors.h"
#include "symtab.h"
#include "strtab.h"
#include "tree_symtab.h"
#include "wn.h"
#include "wfe_expr.h"
#include "wfe_misc.h"
#include "wfe_dst.h"
#include "ir_reader.h"
#include <cmplrs/rcodes.h>
#ifdef TARG_ST
//TB: extension loader
extern TYPE_ID MachineMode_To_Mtype(machine_mode_t mode);
#endif
//#define WFE_DEBUG

#ifdef TARG_ST
// [CG]: Helper functions for volatility attributes propagation.
// See comments in the implementation.
static int check_and_update_volatility(TY_IDX &ty_idx);
static void fixup_volatility(TY_IDX &ty_idx);
#ifdef WFE_DEBUG
static void print_volatility(TY_IDX &ty_idx);
#endif
#endif

extern FILE *tree_dump_file; // For debugging only

#ifdef TARG_ST
INT pstatic_as_global = 0;
#else
extern INT pstatic_as_global;
#endif

#ifdef TARG_ST
/* KEY use these for C++ but not for C, so provide stubs only here. */
void add_duplicates (tree newdecl, tree olddecl) {}
void erase_duplicates (tree decl) {}
#endif

static char*
Get_Name (tree node)
{
	static UINT anon_num = 0;
	static char buf[64];

	if (node == NULL) {
		++anon_num;
		sprintf(buf, ".anonymous.%d", anon_num);
		return buf;
	}
	else if (TREE_CODE (node) == IDENTIFIER_NODE)
		return IDENTIFIER_POINTER (node);
	else if (TREE_CODE (node) == TYPE_DECL)
		// If type has a typedef-name, the TYPE_NAME is a TYPE_DECL.
		return IDENTIFIER_POINTER (DECL_NAME (node));
	else
		FmtAssert(FALSE, ("Get_Name unexpected tree"));
		return NULL;
}

#ifdef TARG_ST
/* ====================================================================
 *   initialize_arb ()
 *
 *   Setup ARB HANDLE's stride, lower and upper bounds.
 * ====================================================================
 */
static void
initialize_arb (
  ARB_HANDLE arb,              // ARB HANDLE to initialize
  tree dim_tree,               // corresponding dimention tree
  TY_IDX *ty_dim0,             // element TY_IDX
  INT64 *bitsize               // element size
)
{

  // TODO: determine how to set hosted and flow dependent ??
  //       anything else ??
  BOOL hosted = FALSE;
  BOOL flow_dependent = FALSE;
  BOOL variable_size = FALSE;

  // determine if we have variable size array dimention.
  // if it is a constant size, calculate the bitsize
  if (TYPE_SIZE(dim_tree) == NULL) {
    // incomplete structs have 0 size
    FmtAssert(TREE_CODE(dim_tree) == ARRAY_TYPE ||
	      TREE_CODE(dim_tree) == UNION_TYPE ||
	      TREE_CODE(dim_tree) == RECORD_TYPE,
	      ("type_size NULL for non ARRAY/RECORD"));
    *bitsize = 0;
  }
  else {
    if (TREE_CODE(TYPE_SIZE(dim_tree)) == INTEGER_CST) {
      // constant size, update the bitsize
      if (TREE_CODE(dim_tree) == INTEGER_TYPE)
	*bitsize = TYPE_PRECISION(dim_tree);
      else
	*bitsize = Get_Integer_Value(TYPE_SIZE(dim_tree));
    }
    else {
      // variable size:
      if (TREE_CODE(dim_tree) == ARRAY_TYPE)
	DevWarn ("Encountered VLA at line %d", lineno);
      else
	Fail_FmtAssertion ("VLA at line %d not currently implemented", lineno);
      variable_size = TRUE;
    }
  }

  ARB_Init (arb, 0, 0, 0);

  // update stride - the argument is the size in bytes of the 
  // current dim. If stride isn't constant size becomes 0.

  if (TYPE_SIZE(TREE_TYPE(dim_tree))) {
    if (TREE_CODE(TYPE_SIZE(TREE_TYPE(dim_tree))) == INTEGER_CST) {
      Set_ARB_const_stride (arb);
      Set_ARB_stride_val (arb, 
	Get_Integer_Value (TYPE_SIZE(TREE_TYPE(dim_tree)))/BITSPERBYTE);
    }
    else {
      WN *swn;
      swn = WFE_Expand_Expr (TYPE_SIZE(TREE_TYPE(dim_tree)));
#if defined (TARG_ST) && (GNU_FRONT_END==33)
      /* (cbr) save vla index */
      if (WN_operator (swn) != OPR_LDID) {
        TY_IDX    ty_idx  = Get_TY (TREE_TYPE(TYPE_SIZE(TREE_TYPE (dim_tree))));
        ST *st = Gen_Temp_Symbol (ty_idx, "__save_expr");
        WFE_Set_ST_Addr_Saved (swn);
        swn = WN_Stid (TY_mtype (ty_idx), 0, st, ty_idx, swn);
        WFE_Stmt_Append (swn, Get_Srcpos());
        swn = WN_Ldid (TY_mtype (ty_idx), 0, st, ty_idx);
      }
#endif
      if (WN_opcode (swn) == OPC_U4I4CVT ||
	  WN_opcode (swn) == OPC_U5I5CVT ||
	  WN_opcode (swn) == OPC_U8I8CVT) {
	swn = WN_kid0 (swn);
      }
      FmtAssert (WN_operator (swn) == OPR_LDID,
			       ("stride operator for VLA not LDID"));
      ST *st = WN_st (swn);
      TY_IDX ty_idx = ST_type (st);
      WN *wn = WN_CreateXpragma (WN_PRAGMA_COPYIN_BOUND,
						   (ST_IDX) NULL, 1);
      WN_kid0 (wn) = WN_Ldid (TY_mtype (ty_idx), 0, st, ty_idx);
      WFE_Stmt_Append (wn, Get_Srcpos());
      Clear_ARB_const_stride (arb);
      Set_ARB_stride_var (arb, (ST_IDX) ST_st_idx (st));
    }
  }
  else {
    // incomplete type
    Set_ARB_stride_var (arb, 0);
  }

  // update the lower bound: it's always 0 for C arrays

  Set_ARB_const_lbnd (arb);
  Set_ARB_lbnd_val (arb, 0);

  // update the upper bound:
  //
  // If the bound isn't a constant, the FE puts it
  // into a temp, so the temp just has to be addressed.

#ifdef TARG_ST
  /* (cbr) check 0 value */
  if (TYPE_SIZE(dim_tree) && !integer_zerop (TYPE_SIZE(dim_tree))) {
#else
  if (TYPE_SIZE(dim_tree)) {
#endif
    if (TREE_CODE(TYPE_MAX_VALUE(TYPE_DOMAIN(dim_tree))) == INTEGER_CST) {
      Set_ARB_const_ubnd (arb);
      Set_ARB_ubnd_val (arb, 
          Get_Integer_Value(TYPE_MAX_VALUE(TYPE_DOMAIN(dim_tree))));
    }
    else {
      WN *uwn = WFE_Expand_Expr(TYPE_MAX_VALUE(TYPE_DOMAIN(dim_tree)));
#if defined (TARG_ST) && (GNU_FRONT_END==33)
      /* (cbr) save vla index */
      if (WN_operator (uwn) != OPR_LDID) {
        TY_IDX    ty_idx  = Get_TY (TREE_TYPE(TYPE_SIZE(TREE_TYPE (dim_tree))));
        ST *st = Gen_Temp_Symbol (ty_idx, "__save_expr");
        WFE_Set_ST_Addr_Saved (uwn);
        uwn = WN_Stid (TY_mtype (ty_idx), 0, st, ty_idx, uwn);
        WFE_Stmt_Append (uwn, Get_Srcpos());
        uwn = WN_Ldid (TY_mtype (ty_idx), 0, st, ty_idx);
      }
#endif
      if (WN_opcode (uwn) == OPC_U4I4CVT ||
	  WN_opcode (uwn) == OPC_U5I5CVT ||
	  WN_opcode (uwn) == OPC_U8I8CVT) {
	uwn = WN_kid0 (uwn);
      }

      FmtAssert (WN_operator (uwn) == OPR_LDID,
				("bounds operator for VLA not LDID"));
      ST *st = WN_st (uwn);
      TY_IDX ty_idx = ST_type (st);
      WN *wn = WN_CreateXpragma (WN_PRAGMA_COPYIN_BOUND,
						   (ST_IDX) NULL, 1);
      WN_kid0 (wn) = WN_Ldid (TY_mtype (ty_idx), 0, st, ty_idx);
      WFE_Stmt_Append (wn, Get_Srcpos());
      Clear_ARB_const_ubnd (arb);
      Set_ARB_ubnd_var (arb, ST_st_idx (st));
    }
  }
  else {
    Clear_ARB_const_ubnd (arb);
    Set_ARB_ubnd_val (arb, 0);
  }

#if 0 // [SC] Does not work for multiple dimensions.
  // This code first initializes the variable that holds the
  // array size to be the size in bits, because that is what
  // gcc expects.
  // It then overwrites the value of the variable with the
  // size in bytes, because that is what the open64 alloca code
  // expects (wfe_decl.cxx/WFE_Alloca_ST).
  // Unfortunately, the variable may be referenced again in a tree
  // generated by gcc for the size of an outer dimension, when
  // there are multiple dimensions.
  // In that tree it is expected to be the size in bits, but
  // because it has been overwritten with the byte size, it contains
  // the wrong value.
  // I have removed this code, and fixed WFE_Alloca_ST to
  // convert the bit size to bytes.
  
  if (variable_size) {
    WN *swn, *wn;
    swn = WFE_Expand_Expr(TYPE_SIZE(dim_tree));
    if (TY_size(*ty_dim0)) {
      if (WN_opcode (swn) == OPC_U4I4CVT ||
	  WN_opcode (swn) == OPC_U5I5CVT ||
	  WN_opcode (swn) == OPC_U8I8CVT) {
	swn = WN_kid0 (swn);
      }
      FmtAssert (WN_operator (swn) == OPR_LDID,
					("size operator for VLA not LDID"));
      ST *st = WN_st (swn);
      TY_IDX ty_idx = ST_type (st);
      TYPE_ID mtype = TY_mtype (ty_idx);
      swn = WN_Div (mtype, swn, WN_Intconst (mtype, BITSPERBYTE));
      wn = WN_Stid (mtype, 0, st, ty_idx, swn);
      WFE_Stmt_Append (wn, Get_Srcpos());
    }
  }
#endif

  return;
}

/* ====================================================================
 *   mk_array_dimention
 *
 *   Walk the array_tree and collect data.
 * ====================================================================
 */
static BOOL
mk_array_dimention (
  tree array_tree,             // current GNU tree
  INT32 *dim,                  // current dimension
  ARB_HANDLE *array_dims,      // fill this out
  TY_IDX *ty_dim0,             // return TY_IDX of element
  INT32 *array_rank,           // return dimensions
  INT64 *bitsize               // return size of current dimension
)
{
  // Save the TY associated with dim=0, as it's needed for
  // TY_etype of the TY_ARI.

  switch (TREE_CODE(array_tree)) {
      
    case ARRAY_TYPE:
      // If the array is of type array, walk through it 
      // If we encountered something else than what we accept as
      // basic array type, we will need to make a la gcc, i.e.
      // an array of array of array ...
      // Do it after unwinding the recursion stack.
      (*array_rank)++;
      if (!mk_array_dimention (TREE_TYPE(array_tree),
			       dim, array_dims, ty_dim0,
			       array_rank, bitsize)) {
	if (*dim == *array_rank-1) {
	  // we haven't succeded and we'll just do as usual
	  //
	  // assumes 1 dimension
	  // nested arrays are treated as arrays of arrays
	  *ty_dim0 = Get_TY(TREE_TYPE(array_tree));
	  *array_rank = 1;
	  *dim = 0;
	}
	else {
	  // continue unwinding the recursion stack
	  (*dim)++;
	  return FALSE;
	}
      }
      break;

    default:

      // got to the bottom of it, initialize the basic induction case:
      *ty_dim0 = Get_TY(array_tree);

      return TRUE;
  }

  // Make a new array dimension. 
  // The dimensions are processed in order 1->rank of array.
  //
  // NOTE: from crayf90/sgi/cwh_types.cxx fei_array_dimen().

  ARB_HANDLE p;

  if (*dim == 0) {
     *array_dims = New_ARB();
     p = *array_dims;
  } else {
     p = New_ARB();
  }

  initialize_arb (p, array_tree, ty_dim0, bitsize);

  // increment the dimention:
  (*dim)++;

  return TRUE;
}

/* ====================================================================
 *   GNU_TO_WHIRL_mk_array_type
 *
 *   Gets a gcc tree, returns the TY_IDX or NULL_IDX if the array is
 *   not "good".
 *
 *   idx is non-zero only for RECORD and UNION, when there is 
 *   forward declaration ??. so it is 0 here ?
 * ====================================================================
 */
TY_IDX
GNU_TO_WHIRL_mk_array_type (
  tree type_tree, 
  TY_IDX idx
)
{
  // For now only ARRAY_TYPE is treated:
  Is_True((TREE_CODE(type_tree) == ARRAY_TYPE),
	              ("GNU_TO_WHIRL_mk_array_type: expected ARRAY_TYPE"));

  // initialize:
  ARB_HANDLE array_dims;
  INT32 array_rank = 0;
  TY_IDX ty_dim0 = TY_IDX_ZERO;
  INT64 bitsize = 0;
  INT32 dim = 0;

  // We prefer first walk the tree and make sure that what's under
  // there is OK. There may be cases when it is advantegeous to
  // leave the thing as array of array of array etc.
  mk_array_dimention (type_tree, 
		      &dim,        // current dimension
		      &array_dims, // fill this out
		      &ty_dim0,    // return TY_IDX of element
		      &array_rank, // return dimensions
		      &bitsize);   // return size of current dimension

  FmtAssert(ty_dim0 != TY_IDX_ZERO, ("something went wrong ??"));

  Set_ARB_first_dimen (array_dims[0]);
  Set_ARB_last_dimen (array_dims[array_rank-1]);
  for (UINT i = 0; i < array_rank; ++i) {
    Set_ARB_dimension (array_dims[i], array_rank-i);
  }

  TY &ty = New_TY (idx);
  TY_Init (ty, bitsize/BITSPERBYTE, KIND_ARRAY, MTYPE_M, 
			Save_Str(Get_Name(TYPE_NAME(type_tree))));
  Set_TY_etype (ty, ty_dim0);
  Set_TY_align (idx, TY_align(ty_dim0));
  Set_TY_arb (ty, array_dims[0]);

  return idx;
}
#endif /* TARG_ST */

/* ====================================================================
 *   Create_TY_For_Tree
 *
 *   idx is non-zero only for RECORD and UNION, when there is 
 *   forward declaration.
 * ====================================================================
 */
extern TY_IDX
Create_TY_For_Tree (tree type_tree, TY_IDX idx)
{
  if (TREE_CODE(type_tree) == ERROR_MARK)
    exit (RC_USER_ERROR);

#ifdef WFE_DEBUG
  print_node_brief (stdout, "  Create_TY_For_Tree: ", type_tree, 0);
  fprintf(stdout,", %d\n", idx);
  if (TYPE_MODE(type_tree)) {
    fprintf (stdout, "     TYPE_MODE %s (UNIT_SIZE %d)\n",
	                       GET_MODE_NAME(TYPE_MODE(type_tree)), 
                          GET_MODE_UNIT_SIZE(TYPE_MODE(type_tree)));

    //    fprintf (stdout, "     size_int_wide (%d, SIZETYPE) = ",
    //	     GET_MODE_UNIT_SIZE (TYPE_MODE (type_tree)));
    //    print_node_brief (stdout, "", 
    //     size_int_wide (GET_MODE_UNIT_SIZE (TYPE_MODE (type_tree)), SIZETYPE), 0);
    //    fprintf(stdout,", \n");
  }

  if (TYPE_NAME (type_tree)) {
    if (TREE_CODE (TYPE_NAME (type_tree)) == IDENTIFIER_NODE)
      fprintf (stdout, "     IDENTIFIER NODE %s", 
                              IDENTIFIER_POINTER (TYPE_NAME (type_tree)));
    else if (TREE_CODE (TYPE_NAME (type_tree)) == TYPE_DECL) {
      fprintf(stdout, "     TYPE DECL ");
      if (DECL_NAME (TYPE_NAME (type_tree))) {
	fprintf (stdout, "%s",
		  IDENTIFIER_POINTER (DECL_NAME (TYPE_NAME (type_tree))));
      }
      fprintf(stdout,"\n");
    }
  }

  if (TYPE_MAIN_VARIANT(type_tree) == type_tree)
    fprintf(stdout, "     TYPE MAIN VARIANT\n");

  print_node_brief (stdout, "     TYPE SIZE TREE: ", 
		                                TYPE_SIZE(type_tree), 0);
  fprintf(stdout,"\n");
  fprintf(stdout, "     TYPE_PRECISION %d\n", TYPE_PRECISION(type_tree));
  fprintf(stdout, "     TYPE_ALIGN %d\n", TYPE_ALIGN(type_tree) / BITSPERBYTE);
  fprintf(stdout, "     TYPE_QUALIFIERS: %c%c%c\n", 
	  TYPE_VOLATILE(type_tree) ? 'V':'-',
	  TYPE_READONLY(type_tree) ? 'C':'-',
	  TYPE_RESTRICT(type_tree) ? 'R':'-');
#endif

  TY_IDX orig_idx = idx;
  if(TREE_CODE_CLASS(TREE_CODE(type_tree)) != 't') {
    DevWarn("Bad tree class passed to Create_TY_For_Tree %c",
		TREE_CODE_CLASS(TREE_CODE(type_tree)));
    return idx;
  }

#ifndef TARG_ST
  // [CG] We don't want to share typedefs as this is bogus
  // in case of forward declaration.
  if (TYPE_NAME(type_tree) &&
      idx == 0 &&
      (TREE_CODE(type_tree) == RECORD_TYPE ||
       TREE_CODE(type_tree) == UNION_TYPE) &&
      TREE_CODE(TYPE_NAME(type_tree)) == TYPE_DECL &&
      TYPE_MAIN_VARIANT(type_tree) != type_tree) {
    idx = Get_TY (TYPE_MAIN_VARIANT(type_tree));
    if (TYPE_READONLY(type_tree))
      Set_TY_is_const (idx);
    if (TYPE_VOLATILE(type_tree))
      Set_TY_is_volatile (idx);
    // restrict qualifier supported by gcc under -std=c99
#ifdef TARG_ST
    if (TYPE_RESTRICT(type_tree))
      Set_TY_is_restrict (idx) ;
#endif /* TARG_ST */
    TYPE_TY_IDX(type_tree) = idx;
    if(Debug_Level >= 2) {
      struct mongoose_gcc_DST_IDX dst = 
	Create_DST_type_For_Tree(type_tree,idx,orig_idx);
      TYPE_DST_IDX(type_tree) = dst;
    }
#ifdef WFE_DEBUG
    print_node_brief (stdout, "  END Create_TY_For_Tree: ", type_tree, 0);
    fprintf(stdout,"\n    new_idx %d, orig_idx: %d, new_ty: %p, orig_ty: %p\n", 
	    idx, orig_idx, &(Ty_Table[idx]), &(Ty_Table[orig_idx]));
#endif
    return idx;
  }
#endif

  TYPE_ID mtype;
  INT tsize;
  BOOL variable_size = FALSE;
  tree type_size = TYPE_SIZE(type_tree);
  UINT align = TYPE_ALIGN(type_tree) / BITSPERBYTE;

  if (TREE_CODE(type_tree) == VOID_TYPE)
    tsize = 0;
  else
    if (type_size == NULL) {
      // incomplete structs have 0 size
#ifdef TARG_ST
      FmtAssert(TREE_CODE(type_tree) == ARRAY_TYPE 
		|| TREE_CODE(type_tree) == UNION_TYPE
		|| TREE_CODE(type_tree) == ENUMERAL_TYPE
		|| TREE_CODE(type_tree) == RECORD_TYPE,
		("Create_TY_For_Tree: type_size NULL for non ARRAY/RECORD"));
#else
      FmtAssert(TREE_CODE(type_tree) == ARRAY_TYPE 
		|| TREE_CODE(type_tree) == UNION_TYPE
		|| TREE_CODE(type_tree) == RECORD_TYPE,
		("Create_TY_For_Tree: type_size NULL for non ARRAY/RECORD"));
#endif
      tsize = 0;
    }
    else {
      if (TREE_CODE(type_size) != INTEGER_CST) {
	if (TREE_CODE(type_tree) == ARRAY_TYPE)
	  DevWarn ("Encountered VLA at line %d", lineno);
	else
	  Fail_FmtAssertion ("VLA at line %d not currently implemented", lineno);
	variable_size = TRUE;
	tsize = 0;
      }
      else
#ifdef TARG_ST
	// Must base it on TYPE_PRECISION unfortunately ...
	// or maybe fortunately, so I do not mess with rat's code
	if (TREE_CODE(type_tree) == INTEGER_TYPE)
	  tsize = TYPE_PRECISION(type_tree) / BITSPERBYTE;
	else
	  tsize = Get_Integer_Value(type_size) / BITSPERBYTE;
#else
      tsize = Get_Integer_Value(type_size) / BITSPERBYTE;
#endif
    }

#ifdef WFE_DEBUG
  fprintf(stdout, "     TYPE_SIZE %d\n", tsize);
#endif

  switch (TREE_CODE(type_tree)) {

  case VOID_TYPE:
    idx = MTYPE_To_TY (MTYPE_V);	// use predefined type
    break;

  case BOOLEAN_TYPE:
  case INTEGER_TYPE:
    switch (tsize) {
    case 1:  mtype = MTYPE_I1; break;
    case 2:  mtype = MTYPE_I2; break;
    case 4:  mtype = MTYPE_I4; break;
    case 5:  mtype = MTYPE_I5; break;
    case 8:  mtype = MTYPE_I8; break;
#ifdef _LP64
    case 16:  mtype = MTYPE_I8; break;
#endif /* _LP64 */
    default:  FmtAssert(FALSE, ("Get_TY unexpected size"));
    }
    if (TREE_UNSIGNED(type_tree)) {
      mtype = MTYPE_complement(mtype);
    }
    idx = MTYPE_To_TY (mtype);	// use predefined type
    Set_TY_align (idx, align);
    break;

  case CHAR_TYPE:
    mtype = (TREE_UNSIGNED(type_tree) ? MTYPE_U1 : MTYPE_I1);
    idx = MTYPE_To_TY (mtype);	// use predefined type
    break;

  case ENUMERAL_TYPE:
    switch (tsize) {
    case 0:  mtype = MTYPE_I4; break;  // for incomplete enumeral type
                                       // (forward references to enumeral type)
    case 1:  mtype = MTYPE_I1; break;
    case 2:  mtype = MTYPE_I2; break;
    case 4:  mtype = MTYPE_I4; break;
    case 5:  mtype = MTYPE_I5; break;
    case 8:  mtype = MTYPE_I8; break;
    default:  FmtAssert(FALSE, ("Get_TY unexpected size"));
    }
    if (TREE_UNSIGNED(type_tree)) {
      mtype = MTYPE_complement(mtype);
    }
    idx = MTYPE_To_TY (mtype);	// use predefined type
    break;

  case REAL_TYPE:
    switch (tsize) {
    case 4:  mtype = MTYPE_F4; break;
    case 8:  mtype = MTYPE_F8; break;
#ifdef TARG_MIPS
    case 16: mtype = MTYPE_FQ; break;
#endif /* TARG_MIPS */
#ifdef TARG_IA64
    case 12: mtype = MTYPE_F10; break;
#endif /* TARG_IA64 */
#ifdef TARG_IA32
    case 12: mtype = MTYPE_F10; break;
#endif /* TARG_IA32 */
    default: FmtAssert(FALSE, ("Get_TY unexpected REAL_TYPE size %d", tsize));
    }
    idx = MTYPE_To_TY (mtype);	// use predefined type
    break;

  case COMPLEX_TYPE:
    switch (tsize) {
    case  8: mtype = MTYPE_C4; break;
    case 16: mtype = MTYPE_C8; break;
#ifdef TARG_MIPS
    case 32: mtype = MTYPE_CQ; break;
#endif /* TARG_MIPS */
#ifdef TARG_IA64
    case 24: mtype = MTYPE_C10; break;
#endif /* TARG_IA64 */
#ifdef TARG_IA32
    case 24: mtype = MTYPE_C10; break;
#endif /* TARG_IA32 */
    default:  FmtAssert(FALSE, ("Get_TY unexpected size"));
    }
    idx = MTYPE_To_TY (mtype);	// use predefined type
    break;

  case REFERENCE_TYPE:
  case POINTER_TYPE:
    idx = Make_Pointer_Type (Get_TY (TREE_TYPE(type_tree)));
    Set_TY_align (idx, align);
    break;

  case ARRAY_TYPE:

#ifdef TARG_ST
    // This makes multi-dimentional arrays.
    idx = GNU_TO_WHIRL_mk_array_type (type_tree, idx);
    /* (cbr) set type align */
    if (align)
      Set_TY_align (idx, align);
#else
    {	// new scope for local vars
      TY &ty = New_TY (idx);
      TY_Init (ty, tsize, KIND_ARRAY, MTYPE_M, 
			Save_Str(Get_Name(TYPE_NAME(type_tree))) );
      Set_TY_etype (ty, Get_TY (TREE_TYPE(type_tree)));
      Set_TY_align (idx, TY_align(TY_etype(ty)));
      // assumes 1 dimension
      // nested arrays are treated as arrays of arrays
      ARB_HANDLE arb = New_ARB ();
      ARB_Init (arb, 0, 0, 0);
      Set_TY_arb (ty, arb);
      Set_ARB_first_dimen (arb);
      Set_ARB_last_dimen (arb);
      Set_ARB_dimension (arb, 1);

      if (TREE_CODE(TYPE_SIZE(TREE_TYPE(type_tree))) == INTEGER_CST) {
	Set_ARB_const_stride (arb);
	Set_ARB_stride_val (arb, 
			    Get_Integer_Value (TYPE_SIZE(TREE_TYPE(type_tree))) 
				/ BITSPERBYTE);
      }
      else {
	WN *swn;
	swn = WFE_Expand_Expr (TYPE_SIZE(TREE_TYPE(type_tree)));
	if (WN_opcode (swn) == OPC_U4I4CVT ||
	    WN_opcode (swn) == OPC_U5I5CVT ||
	    WN_opcode (swn) == OPC_U8I8CVT) {
	  swn = WN_kid0 (swn);
	}
	FmtAssert (WN_operator (swn) == OPR_LDID,
				("stride operator for VLA not LDID"));
	ST *st = WN_st (swn);
	TY_IDX ty_idx = ST_type (st);
	WN *wn = WN_CreateXpragma (WN_PRAGMA_COPYIN_BOUND,
						   (ST_IDX) NULL, 1);
	WN_kid0 (wn) = WN_Ldid (TY_mtype (ty_idx), 0, st, ty_idx);
	WFE_Stmt_Append (wn, Get_Srcpos());
	Clear_ARB_const_stride (arb);
	Set_ARB_stride_var (arb, (ST_IDX) ST_st_idx (st));
      }

      Set_ARB_const_lbnd (arb);
      Set_ARB_lbnd_val (arb, 0);

      if (type_size) {
	if (TREE_CODE(TYPE_MAX_VALUE (TYPE_DOMAIN (type_tree))) ==
	    INTEGER_CST) {
	  Set_ARB_const_ubnd (arb);
	  Set_ARB_ubnd_val (arb, Get_Integer_Value (
				TYPE_MAX_VALUE (TYPE_DOMAIN (type_tree)) ));
	}
	else {
	  WN *uwn = WFE_Expand_Expr (TYPE_MAX_VALUE (TYPE_DOMAIN (type_tree)) );
	  if (WN_opcode (uwn) == OPC_U4I4CVT ||
	      WN_opcode (uwn) == OPC_U5I5CVT ||
	      WN_opcode (uwn) == OPC_U8I8CVT) {
	    uwn = WN_kid0 (uwn);
	  }
	  FmtAssert (WN_operator (uwn) == OPR_LDID,
				("bounds operator for VLA not LDID"));
	  ST *st = WN_st (uwn);
	  TY_IDX ty_idx = ST_type (st);
	  WN *wn = WN_CreateXpragma (WN_PRAGMA_COPYIN_BOUND,
						   (ST_IDX) NULL, 1);
	  WN_kid0 (wn) = WN_Ldid (TY_mtype (ty_idx), 0, st, ty_idx);
	  WFE_Stmt_Append (wn, Get_Srcpos());
	  Clear_ARB_const_ubnd (arb);
	  Set_ARB_ubnd_var (arb, ST_st_idx (st));
	}
      }
      else {
	Clear_ARB_const_ubnd (arb);
	Set_ARB_ubnd_val (arb, 0);
      }

      if (variable_size) {
	WN *swn, *wn;
	swn = WFE_Expand_Expr (type_size);
	if (TY_size(TY_etype(ty))) {
	  if (WN_opcode (swn) == OPC_U4I4CVT ||
	      WN_opcode (swn) == OPC_U5I5CVT ||
	      WN_opcode (swn) == OPC_U8I8CVT) {
	    swn = WN_kid0 (swn);
	  }
	  FmtAssert (WN_operator (swn) == OPR_LDID,
					("size operator for VLA not LDID"));
	  ST *st = WN_st (swn);
	  TY_IDX ty_idx = ST_type (st);
	  TYPE_ID mtype = TY_mtype (ty_idx);
	  swn = WN_Div (mtype, swn, WN_Intconst (mtype, BITSPERBYTE));
	  wn = WN_Stid (mtype, 0, st, ty_idx, swn);
	  WFE_Stmt_Append (wn, Get_Srcpos());
	}
      }
    } // end array scope
#endif /* TARG_ST */

    break;

  case RECORD_TYPE:
  case UNION_TYPE:
    {	// new scope for local vars
      TY &ty = (idx == TY_IDX_ZERO) ? New_TY(idx) : Ty_Table[idx];
      TY_Init (ty, tsize, KIND_STRUCT, MTYPE_M, 
			Save_Str(Get_Name(TYPE_NAME(type_tree))) );
      if (TREE_CODE(type_tree) == UNION_TYPE) {
	Set_TY_is_union(idx);
      }
      if (align == 0) align = 1;	// in case incomplete type
      Set_TY_align (idx, align);
      // set idx now in case recurse thru fields
      TYPE_TY_IDX(type_tree) = idx;	

      // to handle nested structs and avoid entering flds
      // into wrong struct, make two passes over the fields.
      // first create the list of flds for the current struct,
      // but don't follow the nested types.  Then go back thru
      // the fields and set the fld_type, recursing down into
      // nested structs.
      Set_TY_fld (ty, FLD_HANDLE());
      FLD_IDX first_field_idx = Fld_Table.Size ();
      tree field;
      FLD_HANDLE fld;
#ifdef TARG_ST
      /* clarkes: Fix field accessor */
      for (field = TYPE_FIELDS(type_tree); 
#else
      for (field = TREE_PURPOSE(type_tree); 
#endif
	   field;
	   field = TREE_CHAIN(field) )
	{
	  if (TREE_CODE(field) == TYPE_DECL) {
	    DevWarn ("got TYPE_DECL in field list");
	    continue;
	  }
	  if (TREE_CODE(field) == CONST_DECL) {
	    DevWarn ("got CONST_DECL in field list");
	    continue;
	  }
	  fld = New_FLD ();
	  FLD_Init (fld, Save_Str(Get_Name(DECL_NAME(field))), 
		    0, // type
		    Get_Integer_Value(DECL_FIELD_OFFSET(field)) +
		    Get_Integer_Value(DECL_FIELD_BIT_OFFSET(field))
					/ BITSPERBYTE );
	  if (DECL_NAME(field) == NULL)
	    Set_FLD_is_anonymous(fld);
#ifdef OLDCODE
	  if ( ! DECL_BIT_FIELD(field)
	       && Get_Integer_Value(DECL_SIZE(field)) > 0
	       && Get_Integer_Value(DECL_SIZE(field))
	       != (TY_size(Get_TY(TREE_TYPE(field))) 
		   * BITSPERBYTE) )
	    {
				// for some reason gnu doesn't set bit field
				// when have bit-field of standard size
				// (e.g. int f: 16;).  But we need it set
				// so we know how to pack it, because 
				// otherwise the field type is wrong.
	      DevWarn("field size %d doesn't match type size %d", 
		      Get_Integer_Value(DECL_SIZE(field)),
		      TY_size(Get_TY(TREE_TYPE(field)))
						* BITSPERBYTE );
	      DECL_BIT_FIELD(field) = 1;
	    }
	  if (DECL_BIT_FIELD(field)) {
	    Set_FLD_is_bit_field (fld);
				// bofst is remaining bits from byte offset
	    Set_FLD_bofst (fld, 
			   // Get_Integer_Value(DECL_FIELD_OFFSET(field)) +
			   Get_Integer_Value(DECL_FIELD_BIT_OFFSET(field))
			                                    % BITSPERBYTE );
	    Set_FLD_bsize (fld, Get_Integer_Value(DECL_SIZE(field)));
			}
#endif /* OLDCODE */
	}
      FLD_IDX last_field_idx = Fld_Table.Size () - 1;
      if (last_field_idx >= first_field_idx) {
	Set_TY_fld (ty, FLD_HANDLE (first_field_idx));
	Set_FLD_last_field (FLD_HANDLE (last_field_idx));
      }
      // now set the fld types.
      fld = TY_fld(ty);
#ifdef TARG_ST
      /* clarkes: Fix field accessor */
      for (field = TYPE_FIELDS(type_tree); 
#else
      for (field = TREE_PURPOSE(type_tree);
#endif
	   field;
	   field = TREE_CHAIN(field))
	{
	  if (TREE_CODE(field) == TYPE_DECL)
	    continue;
	  if (TREE_CODE(field) == CONST_DECL)
	    continue;
#ifdef TARG_ST
	  // [CL] Handle zero-sized arrays
	  if (DECL_SIZE(field) != NULL) {
#endif
	  if ( ! DECL_BIT_FIELD(field)
	       && Get_Integer_Value(DECL_SIZE(field)) > 0
	       && Get_Integer_Value(DECL_SIZE(field))
	       != (TY_size(Get_TY(TREE_TYPE(field))) 
					* BITSPERBYTE) )
	    {
				// for some reason gnu doesn't set bit field
				// when have bit-field of standard size
				// (e.g. int f: 16;).  But we need it set
				// so we know how to pack it, because 
				// otherwise the field type is wrong.
#ifndef TARG_ST
	      // [CG]: Does not need a devwarn
	      DevWarn("field size %lld doesn't match type size %lld", 
		      Get_Integer_Value(DECL_SIZE(field)),
		      TY_size(Get_TY(TREE_TYPE(field)))
						* BITSPERBYTE );
#endif
	      DECL_BIT_FIELD(field) = 1;
	    }
#ifdef TARG_ST
	  }
#endif
	  if (DECL_BIT_FIELD(field)) {
	    Set_FLD_is_bit_field (fld);
				// bofst is remaining bits from byte offset
	    Set_FLD_bofst (fld, 
			   // Get_Integer_Value(DECL_FIELD_OFFSET(field)) +
			   Get_Integer_Value(DECL_FIELD_BIT_OFFSET(field))
					% BITSPERBYTE );
	    Set_FLD_bsize (fld, Get_Integer_Value(DECL_SIZE(field)));
	  }
	  TY_IDX fty_idx = Get_TY(TREE_TYPE(field));
#ifdef TARG_ST
	  // [CG]: For structure fields, the qualifiers are
	  // on the field nodes (not on the field node type).
	  // Thus we get them explicitly.
	  if (TREE_THIS_VOLATILE(field)) Set_TY_is_volatile (fty_idx);
	  if (TREE_READONLY(field)) Set_TY_is_const (fty_idx);
	  if (TYPE_RESTRICT(field)) Set_TY_is_restrict (fty_idx);
#endif
	  if ((TY_align (fty_idx) > align) || (TY_is_packed (fty_idx)))
	    Set_TY_is_packed (ty);
	  Set_FLD_type(fld, fty_idx);
	  fld = FLD_next(fld);
	}
    } // end record scope
    break;

  case METHOD_TYPE:
    DevWarn ("Encountered METHOD_TYPE at line %d", lineno);

  case FUNCTION_TYPE:
    {	// new scope for local vars
      tree arg;
      INT32 num_args;
      TY &ty = New_TY (idx);
      TY_Init (ty, 0, KIND_FUNCTION, MTYPE_UNKNOWN, 0); 
      Set_TY_align (idx, 1);
      TY_IDX ret_ty_idx;
      TY_IDX arg_ty_idx;
      TYLIST tylist_idx;

      // allocate TYs for return as well as parameters
      // this is needed to avoid mixing TYLISTs if one
      // of the parameters is a pointer to a function

#ifdef WFE_DEBUG
      print_node_brief (stdout,"  FUNCTION RETURN TYPE: ", TREE_TYPE(type_tree), 0);
      fprintf(stdout, "\n");
#endif

      ret_ty_idx = Get_TY(TREE_TYPE(type_tree));

      //fprintf(stdout, "TY name %s\n", TY_name(ret_ty_idx));

      for (arg = TYPE_ARG_TYPES(type_tree);
	   arg;
	   arg = TREE_CHAIN(arg)) {

#ifdef WFE_DEBUG
	print_node_brief (stdout,"  ARG TYPE: ", TREE_VALUE(arg), 0);
	fprintf(stdout, "\n");
#endif
	arg_ty_idx = Get_TY(TREE_VALUE(arg));
      }

      // if return type is pointer to a zero length struct
      // convert it to void
      if (!WFE_Keep_Zero_Length_Structs    &&
	  TY_mtype (ret_ty_idx) == MTYPE_M &&
	  TY_size (ret_ty_idx) == 0) {
	// zero length struct being returned
	DevWarn ("function returning zero length struct at line %d", lineno);
	ret_ty_idx = Be_Type_Tbl (MTYPE_V);
      }

      Set_TYLIST_type (New_TYLIST (tylist_idx), ret_ty_idx);
      Set_TY_tylist (ty, tylist_idx);
      for (num_args = 0, arg = TYPE_ARG_TYPES(type_tree);
	   arg;
	   num_args++, arg = TREE_CHAIN(arg))
	{
	  arg_ty_idx = Get_TY(TREE_VALUE(arg));
	  if (!WFE_Keep_Zero_Length_Structs    &&
	      TY_mtype (arg_ty_idx) == MTYPE_M &&
	      TY_size (arg_ty_idx) == 0) {
				// zero length struct passed as parameter
	    DevWarn ("zero length struct encountered in function prototype at line %d", lineno);
	  }
	  else
	    Set_TYLIST_type (New_TYLIST (tylist_idx), arg_ty_idx);
	}
      if (num_args)
	{
	  Set_TY_has_prototype(idx);
	  if (arg_ty_idx != Be_Type_Tbl(MTYPE_V))
	    {
	      Set_TYLIST_type (New_TYLIST (tylist_idx), 0);
	      Set_TY_is_varargs(idx);
	    }
	  else
	    Set_TYLIST_type (Tylist_Table [tylist_idx], 0);
	}
      else
	Set_TYLIST_type (New_TYLIST (tylist_idx), 0);
    } // end FUNCTION_TYPE scope
    break;
#ifdef TARG_ST
	   // TB: extension vector types
	   case VECTOR_TYPE:
      {
	FmtAssert((TYPE_MODE (type_tree) - STATIC_COUNT_MACHINE_MODE) >= 0,
			  ("Get_TY: not a dynamic vector type %s", TYPE_NAME (type_tree)));
	// Find MTYPE associated to TYPE_MODE (type_tree)
	mtype = MachineMode_To_Mtype(TYPE_MODE (type_tree));
	if (TREE_UNSIGNED(type_tree)) 
	  mtype = MTYPE_complement(mtype);
	idx = MTYPE_To_TY (mtype);
	Set_TY_align (idx, align);
      }
	     break;
#endif // TARG_ST
      default:
	FmtAssert(FALSE, ("Get_TY unexpected tree_type"));
      }

  if (TYPE_READONLY(type_tree))
    Set_TY_is_const (idx);
  if (TYPE_VOLATILE(type_tree))
    Set_TY_is_volatile (idx);
#ifdef TARG_ST
  // [CG]: Fixup volatility for structure fields.
  fixup_volatility(idx);
#ifdef WFE_DEBUG
  print_volatility(idx);
#endif
#endif
#ifdef TARG_ST
  // (cbr) handle may_alias attribute
  if (lookup_attribute ("may_alias", TYPE_ATTRIBUTES (type_tree))) {
    // [CL] don't apply the attribute to the predefined type
    idx = Copy_TY(idx);
    Set_TY_no_ansi_alias(idx);
  }
#endif
  // restrict qualifier supported by gcc under -std=c99
#ifdef TARG_ST
  if (TYPE_RESTRICT(type_tree))
    Set_TY_is_restrict (idx);
#endif /* TARG_ST */
  TYPE_TY_IDX(type_tree) = idx;
  if(Debug_Level >= 2) {
    struct mongoose_gcc_DST_IDX dst = 
	    Create_DST_type_For_Tree(type_tree,idx,orig_idx);
    TYPE_DST_IDX(type_tree) = dst;
  }
#ifdef WFE_DEBUG
  print_node_brief (stdout, "  END Create_TY_For_Tree: ", type_tree, 0);
  fprintf(stdout,"\n    new_idx %d, orig_idx: %d, new_ty: %p, orig_ty: %p\n", 
	  idx, orig_idx, &(Ty_Table[idx]), &(Ty_Table[orig_idx]));
#endif
  return idx;
}


#ifdef TARG_ST
/** 
  *  This function resets attributes based "properties" on variable.
  * it is used by Get_ST(). It is run at each call of get_st to
  * guarantee proper properties in case of forward declarations.
  * 
  * @param decl_node 
  */
void
set_variable_attributes(tree decl_node)
  {
    ST *st = DECL_ST(decl_node);

    FmtAssert (st!=NULL, ("st cannot be NULL in set_variable_attributes()"));

    TY_IDX ty_idx = Get_TY(TREE_TYPE(decl_node));
    if (DECL_USER_ALIGN (TREE_TYPE(decl_node))) {
      UINT align = DECL_ALIGN(decl_node) / BITSPERBYTE;
      if (align) {
        Set_TY_align (ty_idx, align);
        Set_ST_type (st, ty_idx);
      }
    }
    
    if (DECL_USER_ALIGN (decl_node)) {
      UINT align = DECL_ALIGN(decl_node) / BITSPERBYTE;
      if (align) {
        Set_TY_align (ty_idx, align);
        Set_ST_type (st, ty_idx);
      }
    }
#ifdef TARG_STxP70
    // (cbr) memory_space support
    tree attr;
    ST_MEMORY_SPACE kind=ST_MEMORY_DEFAULT;
    attr = lookup_attribute ("memory", DECL_ATTRIBUTES (decl_node));
    if (attr) {
      attr = TREE_VALUE (TREE_VALUE (attr));
      FmtAssert (TREE_CODE (attr) == STRING_CST, ("Malformed memory attribute"));
      if (!strcmp (TREE_STRING_POINTER (attr), "da")) {
        kind = ST_MEMORY_DA;
      } else if (!strcmp (TREE_STRING_POINTER (attr), "sda")) {
        kind = ST_MEMORY_SDA;
      } else if (!strcmp (TREE_STRING_POINTER (attr), "tda")) {
        kind = ST_MEMORY_TDA;
      } else if (!strcmp (TREE_STRING_POINTER (attr), "none")) {
        kind = ST_MEMORY_NONE;
      } else {
        FmtAssert (FALSE, ("Malformed tls_model attribute"));
      }
    } 
    Set_ST_memory_space (*st, kind);
#endif        
    if (DECL_SECTION_NAME (decl_node) &&
	!ST_has_named_section (st)) { // [TTh] Insure that attributes are updated
                                      // only once to avoid multiple memory
                                      // allocation for same variable
      SYMTAB_IDX level = ST_level(st);
      if (TREE_CODE (decl_node) == FUNCTION_DECL)
        level = GLOBAL_SYMTAB;
      ST_ATTR_IDX st_attr_idx;
      ST_ATTR&    st_attr = New_ST_ATTR (level, st_attr_idx);
      ST_ATTR_Init (st_attr, ST_st_idx (st), ST_ATTR_SECTION_NAME,
                    Save_Str (TREE_STRING_POINTER (DECL_SECTION_NAME (decl_node))));
      Set_ST_has_named_section (st);
    }
  }
#endif  // TARG_ST


ST*
Create_ST_For_Tree (tree decl_node)
{
  TY_IDX     ty_idx;
  ST*        st;
  char      *name;
  ST_SCLASS  sclass;
  ST_EXPORT  eclass;
  SYMTAB_IDX level;

  if (TREE_CODE(decl_node) == ERROR_MARK)
    exit (RC_USER_ERROR);

#ifdef WFE_DEBUG
  print_node_brief (stdout,"  Create_ST_For_Tree: ", decl_node, 0);
  fprintf(stdout, "\n");
#endif


#ifdef TARG_ST
  if (DECL_ASSEMBLER_NAME_SET_P (decl_node))
    name = IDENTIFIER_POINTER (DECL_ASSEMBLER_NAME (decl_node));
  else
#endif
  if (DECL_NAME (decl_node))
    name = IDENTIFIER_POINTER (DECL_NAME (decl_node));
  else {
    DevWarn ("no name for DECL_NODE");
    name = "__unknown__";
  }

  switch (TREE_CODE(decl_node)) {

    case FUNCTION_DECL:
      {
        TY_IDX func_ty_idx = Get_TY(TREE_TYPE(decl_node));

        if (DECL_WIDEN_RETVAL (decl_node)) {
/*
          extern tree long_long_integer_type_node;
          extern tree long_long_unsigned_type_node;
*/
          tree type_tree = TREE_TYPE(decl_node);
          tree ret_type_tree = TREE_TYPE (type_tree);
          TY_IDX ret_ty_idx = Get_TY(ret_type_tree);
	  if (MTYPE_signed (TY_mtype (ret_ty_idx)))
            TREE_TYPE (type_tree) = long_long_integer_type_node;
          else
            TREE_TYPE (type_tree) = long_long_unsigned_type_node;
          TY_IDX old_func_ty_idx = func_ty_idx;
          func_ty_idx = Create_TY_For_Tree (type_tree, TY_IDX_ZERO);
          TREE_TYPE (type_tree) = ret_type_tree;
          TYPE_TY_IDX(type_tree) = old_func_ty_idx;
        }

        sclass = SCLASS_EXTERN;
#ifdef TARG_ST
	eclass = Get_Export_Class_For_Tree(decl_node, CLASS_FUNC, sclass);

	if (DECL_CONTEXT (decl_node) == 0) {
	  level = GLOBAL_SYMTAB + 1;
	} else {
	  level = PU_lexical_level (Get_ST (DECL_CONTEXT (decl_node))) +1;
	}
#else
        eclass = TREE_PUBLIC(decl_node) ? EXPORT_PREEMPTIBLE : EXPORT_LOCAL;
        level  = GLOBAL_SYMTAB+1;
#endif

        PU_IDX pu_idx;
        PU&    pu = New_PU (pu_idx);

        PU_Init (pu, func_ty_idx, level);

        st = New_ST (GLOBAL_SYMTAB);

        ST_Init (st,
                 Save_Str ( IDENTIFIER_POINTER (DECL_ASSEMBLER_NAME (decl_node))),
                 CLASS_FUNC, sclass, eclass, TY_IDX (pu_idx));
/*
       if (TREE_CODE(TREE_TYPE(decl_node)) == METHOD_TYPE)
         fprintf (stderr, "Create_ST_For_Tree: METHOD_TYPE\n");
*/
#ifdef TARG_ST
          /* (cbr) handle used attribute */
        if (DECL_IS_USED (decl_node)) {
          Set_PU_no_delete (Pu_Table [ST_pu (st)]);            
	  Set_ST_is_used(st);
	}
	/* [TB] handle optimization function level attributes */
	Set_PU_size_opt (Pu_Table [ST_pu (st)], (enum PU_OPTLEVEL) DECL_OPTSIZE(decl_node));
// 	Set_PU_perf_opt (Pu_Table [ST_pu (st)], DECL_OPTPERF(decl_node));
#endif
      }
      break;

    case PARM_DECL:
    case VAR_DECL:
      {
        if (TREE_CODE(decl_node) == PARM_DECL) {
          sclass = SCLASS_FORMAL;
          eclass = EXPORT_LOCAL;
          level = CURRENT_SYMTAB;
        }
        else {
          if (DECL_CONTEXT (decl_node) == 0) {
            if (TREE_PUBLIC (decl_node)) {
	      if (DECL_INITIAL(decl_node))
		sclass = SCLASS_DGLOBAL;
	      else if (TREE_STATIC(decl_node)) {
#ifdef TARG_ST
                /* (cbr) unitialized placed variables should be allocated */
		// [CG]: Added support for common and nocommon attributes
		// in conjunction with -fnocommon.
		// The gnu front-end has set DECL_COMMON accordingly, so we
		// simply check this flag.
		if (DECL_SECTION_NAME(decl_node) ||
		    !DECL_COMMON(decl_node))
              	  sclass = SCLASS_UGLOBAL;
		else
              	  sclass = SCLASS_COMMON;
#else
		if (flag_no_common)
              	  sclass = SCLASS_UGLOBAL;
		else
              	  sclass = SCLASS_COMMON;
#endif
	      }
	      else
              	sclass = SCLASS_EXTERN;
#ifdef TARG_ST
	      eclass = Get_Export_Class_For_Tree(decl_node, CLASS_VAR, sclass);
#else
              eclass = EXPORT_PREEMPTIBLE;
#endif
            }
            else {
              	sclass = SCLASS_FSTATIC;
#ifdef TARG_ST
		eclass = Get_Export_Class_For_Tree(decl_node, CLASS_VAR, sclass);
#else
		eclass = EXPORT_LOCAL;
#endif
            }
            level = GLOBAL_SYMTAB;
          }
          else {
            if (DECL_EXTERNAL(decl_node)) {
	      sclass = SCLASS_EXTERN;
	      level  = GLOBAL_SYMTAB;
#ifdef TARG_ST
	      eclass = Get_Export_Class_For_Tree(decl_node, CLASS_VAR, sclass);
#else
              eclass = EXPORT_PREEMPTIBLE;
#endif
            }
            else {
	      if (TREE_STATIC (decl_node)) {
		sclass = SCLASS_PSTATIC;
		if (pstatic_as_global)
			level = GLOBAL_SYMTAB;
		else
			level = CURRENT_SYMTAB;
              }
              else {
		sclass = SCLASS_AUTO;
#ifdef TARG_ST
		level = PU_lexical_level (Get_ST (DECL_CONTEXT (decl_node)));
#else
		level = decl_node->decl.symtab_idx ?
                        decl_node->decl.symtab_idx : CURRENT_SYMTAB;
#endif
              }
              eclass = EXPORT_LOCAL;
            }
          }
        }

        st = New_ST (level);
        ty_idx = Get_TY (TREE_TYPE(decl_node));
        if (TY_kind (ty_idx) == KIND_ARRAY &&
            TREE_STATIC (decl_node) &&
            DECL_INITIAL (decl_node) == FALSE &&
            TY_size (ty_idx) == 0) {
          Set_TY_size (ty_idx, TY_size (Get_TY (TREE_TYPE (TREE_TYPE (decl_node)))));
        }
	if (TY_mtype (ty_idx) == MTYPE_M &&
#ifdef TARG_ST
            // [SC] Alignment of parameters is fixed by the ABI, and cannot
            // be changed by Aggregate_Alignment.
            TREE_CODE(decl_node) != PARM_DECL &&
#endif
	    Aggregate_Alignment > 0 &&
	    Aggregate_Alignment > TY_align (ty_idx))
	  Set_TY_align (ty_idx, Aggregate_Alignment);
	// qualifiers are set on decl nodes
	if (TREE_READONLY(decl_node))
		Set_TY_is_const (ty_idx);
	if (TREE_THIS_VOLATILE(decl_node))
		Set_TY_is_volatile (ty_idx);

#ifdef KEY
	// Handle aligned attribute (bug 7331)
	if (DECL_USER_ALIGN (decl_node))
	  Set_TY_align (ty_idx, DECL_ALIGN_UNIT (decl_node));
	// NOTE: we do not update the ty_idx value in the TYPE_TREE. So
	// if any of the above properties are set, the next time we get into
	// Get_ST, the ty_idx in the TYPE_TREE != ty_idx in st. The solution
	// is either to update TYPE_TREE now, or compare the ty_idx_index
	// in Get_ST (instead of ty_idx). Currently we do the latter.
#endif // KEY
#ifdef TARG_ST
	if (TYPE_RESTRICT(decl_node))
	    Set_TY_is_restrict (ty_idx) ;
#endif /* TARG_ST */
        ST_Init (st, Save_Str(name), CLASS_VAR, sclass, eclass, ty_idx);
        if (TREE_CODE(decl_node) == PARM_DECL) {
		Set_ST_is_value_parm(st);
        }
#ifdef TARG_ST
        /* (cbr) mark readonly vars const */
        if (sclass != SCLASS_AUTO && 
            TREE_CODE(decl_node) == VAR_DECL &&
            TYPE_READONLY(decl_node))
          Set_ST_is_const_var (st);

	// [CL] handle used attribute
	if (sclass != SCLASS_AUTO && 
            TREE_CODE(decl_node) == VAR_DECL &&
	    DECL_IS_USED(decl_node)) {
	  Set_ST_is_used(st);
	}
#endif 
#ifndef TARG_ST 
        /* (cbr) catch declarations with 0 size */
        if (TREE_CODE (decl_node) == VAR_DECL && 
            !DECL_EXTERNAL (decl_node)) {

          if (TREE_CODE(TREE_TYPE(decl_node)) == RECORD_TYPE) {
            if (DECL_SIZE (decl_node) == 0) {
              error_with_file_and_line (DECL_SOURCE_FILE (decl_node),
                                        DECL_SOURCE_LINE (decl_node),
                                        "storage size of `%s' isn't known",
                                        IDENTIFIER_POINTER (DECL_NAME (decl_node)));
            }
            else if (!host_integerp (DECL_SIZE_UNIT (decl_node), 1)) {
              error_with_decl (decl_node, "size of variable `%s' is too large");
            }
          }
        }
#endif
	if (TREE_CODE(decl_node) == VAR_DECL && DECL_THREAD_LOCAL(decl_node)) {
		Set_ST_is_thread_local (st);
	}
#ifdef TARG_ST
	// [SC] TLS support
	if (DECL_THREAD_LOCAL (decl_node)) {
	  INT i;
	  ST_TLS_MODEL kind;
	  ST_TLS_MODEL default_kind;
	  tree attr;
	  Set_ST_is_thread_private (st);
	  attr = lookup_attribute ("tls_model", DECL_ATTRIBUTES (decl_node));
	  if (attr) {
	    attr = TREE_VALUE (TREE_VALUE (attr));
	    FmtAssert (TREE_CODE (attr) == STRING_CST, ("Malformed tls_model attribute"));
	    if (!strcmp (TREE_STRING_POINTER (attr), "local-exec"))
	      kind = ST_TLS_MODEL_LOCAL_EXEC;
	    else if (!strcmp (TREE_STRING_POINTER (attr), "initial-exec"))
	      kind = ST_TLS_MODEL_INITIAL_EXEC;
	    else if (!strcmp (TREE_STRING_POINTER (attr), "local-dynamic"))
	      kind = ST_TLS_MODEL_LOCAL_DYNAMIC;
	    else if (!strcmp (TREE_STRING_POINTER (attr), "global-dynamic"))
	      kind = ST_TLS_MODEL_GLOBAL_DYNAMIC;
	    else
	      FmtAssert (FALSE, ("Malformed tls_model attribute"));
	  } else {
	    BOOL local_p = FALSE;
	    if (!DECL_P (decl_node)) {
	      local_p = TRUE;
	    } else if (! TREE_PUBLIC (decl_node)) {
	      local_p = TRUE;
	    } else if (decl_visibility (decl_node) != VISIBILITY_DEFAULT) {
	      local_p = TRUE;
	    } else if (DECL_EXTERNAL (decl_node)) {
	      local_p = FALSE;
	    } else if (DECL_ONE_ONLY (decl_node) || DECL_WEAK (decl_node)) {
	      local_p = FALSE;
	    } else if (Gen_PIC_Shared) {
	      local_p = FALSE;
	    } else {
	      local_p = TRUE;
	    }
	    if (!Gen_PIC_Shared) {
	      kind = local_p ? ST_TLS_MODEL_LOCAL_EXEC : ST_TLS_MODEL_INITIAL_EXEC;
	    } else {
	      kind = local_p ? ST_TLS_MODEL_LOCAL_DYNAMIC : ST_TLS_MODEL_GLOBAL_DYNAMIC;
	    }
	    switch (flag_tls_default) {
	    case TLS_MODEL_LOCAL_EXEC:
	      default_kind = ST_TLS_MODEL_LOCAL_EXEC;
	      break;
	    case TLS_MODEL_LOCAL_DYNAMIC:
	      default_kind = ST_TLS_MODEL_LOCAL_DYNAMIC;
	      break;
	    case TLS_MODEL_INITIAL_EXEC:
	      default_kind = ST_TLS_MODEL_INITIAL_EXEC;
	      break;
	    case TLS_MODEL_GLOBAL_DYNAMIC:
	    default:
	      default_kind = ST_TLS_MODEL_GLOBAL_DYNAMIC;
	      break;
	    }
	    if (kind < default_kind) {
	      kind = default_kind;
	    }
	  }
	  Set_ST_tls_model (*st, kind);
	}
#endif
      }
      break;

    default:
      {
        Fail_FmtAssertion ("Create_ST_For_Tree: unexpected tree type");
      }
      break;
  }

  DECL_ST(decl_node) = st;

  if ((DECL_WEAK (decl_node)) && (TREE_CODE (decl_node) != PARM_DECL)) {
    Set_ST_is_weak_symbol (st);
/*
    if (TREE_CODE (decl_node) == FUNCTION_DECL)
      Set_ST_sclass (st, SCLASS_TEXT);
*/
  }

//   if (DECL_SECTION_NAME (decl_node)) {
//     fprintf(stderr, "specify attribute in CreateST\n");
//     if (TREE_CODE (decl_node) == FUNCTION_DECL)
//       level = GLOBAL_SYMTAB;
//     ST_ATTR_IDX st_attr_idx;
//     ST_ATTR&    st_attr = New_ST_ATTR (level, st_attr_idx);
//     ST_ATTR_Init (st_attr, ST_st_idx (st), ST_ATTR_SECTION_NAME,
//                   Save_Str (TREE_STRING_POINTER (DECL_SECTION_NAME (decl_node))));
//     Set_ST_has_named_section (st);
//   }
//  set_variable_attributes(decl_node);


  if (DECL_SYSCALL_LINKAGE (decl_node)) {
	Set_PU_has_syscall_linkage (Pu_Table [ST_pu(st)]);
  }

#ifdef TARG_ST
  /* (cbr) const/pure attributes.
     Warning: 
     gcc pure maps to open64 no_side_effect
     gcc const maps to open64 pure
  */
  if (TREE_CODE (decl_node) == FUNCTION_DECL) {
    if (TREE_READONLY (decl_node)) {
      Set_PU_is_pure (Pu_Table [ST_pu (st)]);
    }
    if (DECL_IS_PURE (decl_node)) {
      Set_PU_no_side_effects (Pu_Table [ST_pu (st)]);
    }
  }
#endif

  if(Debug_Level >= 2) {
     struct mongoose_gcc_DST_IDX dst =
       Create_DST_decl_For_Tree(decl_node,st);
     DECL_DST_IDX(decl_node) = dst;
  }

  return st;
}

#ifdef TARG_ST
// [CG]
// fixup_volatility()
//
// Function that propagate volatile attributes
// to all possibly overlapping fileds in a struct/union
// where some fields are volatile.
// This function must be called at the end of the
// struct/union type creation, when all the fields
// are set up.
//
// The rational for this comes from a bug referenced
// as 1-5-0-B/ddts/18793.
// The problem is that in WOPT SSA form, the 
// volatile accesses are not considered at all
// in memory dependency chains.
// The consequence of this is:
//   If two memory access overlaps and one and only
//   one of them is marked volatile, the DSE/SPRE/DCE
//   in WOPT will generate unexpected code.
// This can appear when an union has some fields marked
// volatile and the other not.
// The solutions to this are:
// 1. Reengineer the WOPT SSA form to handle this case.
// 2. Fix DSE/SPRE/DCE optimizations to not remove stores
// that access aggregates where a member is volatile. It
// seems sufficient for the original problem, but not 
// terribly scalable.
// 3. Arrange the WHIRL such that overlapping fields in
// unions are all marked volatile and have the front-end
// generate volatile access for all accesses to this 
// volatile fileds.
//
// I choose to implement solution 3 as it is the most
// straightforward and the most localized changed.
//
// The solution is implemented in two steps:
// 1. The fixup_volatility() in tree_symtab.cxx sets the
// volatile flag for the fields of a structure that may
// overlap some other volatile field.
// 2. The function_update_volatile_accesses() in 
// wfe_decl.cxx arrange for having all memory accesses
// to a volatile field to be marked volatile. The reason
// for this step is that the translator inspects the GCC
// tree for marking volatility, not the field type. Thus
// we have some additional accesses (due to the 1st step)
// to mark as volatile.
//
static void
set_aggregate_fields_volatile(TY_IDX &ty_idx)
{
  if (TY_kind(ty_idx) == KIND_STRUCT) {
    if (!TY_fld (ty_idx).Is_Null ()) {
      FLD_ITER fld_iter = Make_fld_iter (TY_fld (ty_idx));
      do {
	TY_IDX fld_ty = FLD_type (fld_iter);
	set_aggregate_fields_volatile(fld_ty);
	Set_FLD_type(fld_iter, fld_ty);
      } while (!FLD_last_field (fld_iter++));
    }
  } else {
    Set_TY_is_volatile (ty_idx);
  }
}

static int
check_and_update_volatility(TY_IDX &ty_idx)
{
  int volatility = FALSE;
  if (TY_is_volatile(ty_idx)) volatility = TRUE;
  else if (TY_kind(ty_idx) == KIND_STRUCT) {
    if (!TY_fld (ty_idx).Is_Null ()) {
      FLD_ITER fld_iter = Make_fld_iter (TY_fld (ty_idx));
      do {
	TY_IDX fld_ty = FLD_type (fld_iter);
	if (check_and_update_volatility(fld_ty)) {
	  Set_FLD_type(fld_iter, fld_ty);
	  volatility = TRUE;
	  break;
	}
      } while (!FLD_last_field (fld_iter++));
    }
    if (TY_is_union(ty_idx) &&
	volatility) {
      set_aggregate_fields_volatile (ty_idx);
    }
  }
  return volatility;
}

static void
fixup_volatility(TY_IDX &ty_idx)
{
  check_and_update_volatility(ty_idx);
}

#ifdef WFE_DEBUG
static void
print_volatility(TY_IDX &ty_idx)
{
  char info[4] = {0,0,0,0};
  char *p = info;
  printf("TY: %s: ", TY_name(ty_idx));
  if (TY_is_volatile(ty_idx)) printf("V");
  printf("\n");
  if (TY_kind(ty_idx) == KIND_STRUCT) {
    if (TY_is_union(ty_idx)) printf("UNION");
    else printf("STRUCT");
    printf(" FIELDS {\n");
    
    if (!TY_fld (ty_idx).Is_Null ()) {
      FLD_ITER fld_iter = Make_fld_iter (TY_fld (ty_idx));
      do {
	TY_IDX fld_ty = FLD_type (fld_iter);
	printf(" %s {\n", FLD_name(fld_iter));
	print_volatility(fld_ty);
	printf(" }\n");
      } while (!FLD_last_field (fld_iter++));
    }
    printf(" }\n");
  }
}
#endif
#endif

#ifdef TARG_ST
/*
 * The following functions are for symbols' visibility management.
 * The visibility is an attribute of a symbol declaration/definition.
 * The visibility apply to functions and to variables.
 * In this interface visibility is an int type defined with the 
 * following values (ref. elf.h):
 * -1:	undefined
 *  0:	STV_DEFAULT
 *  1:	STV_INTERNAL
 *  2:	STV_HIDDEN
 *  3:	STV_PROTECTED
 *
 * The export class is defined in symtab_defs.h and contains the visibility
 * in addition to the storeage class.
 *
 * Function list:
 *  int Get_Default_Visibility(tree decl_node, ST_class st_class , ST_SCLASS sclass)
 *	Gets the default visibility for the node. It may be specified
 *	by the user command line switch -fvisibility=...
 *	This command line switch do not apply to external definitions.
 *	The default value is STV_DEFAULT.
 *
 *  void Set_Default_Visibility(int visibility)
 *	Sets the default visibility.
 *
 *  int	Get_Visibility_From_Attribute (tree decl_node, ST_class st_class , ST_SCLASS sclass)
 *	Gets the visibility from a specified visibility attribute on the
 *	declaration tree.
 *	Returns -1 if no visibility attribute is specified.
 *
 *  int	Get_Visibility_From_Specification_File (tree decl_node, ST_class st_class , ST_SCLASS sclass)
 *	Gets the visibility for the symbol corresponding to the declaration
 *	node from the visibility specification file.
 *	Returns -1 if no specifcation file was given or if the name of the
 *	symbol is not matched by the specification file.
 *
 *  int Get_Visibility_For_Tree (tree decl_node, ST_class st_class , ST_SCLASS sclass)
 *	Gets the symbol visibility for a particular tree decl./def.
 *	(tree code must be VAR_DECL or FUNCTION_DECL).
 *	The symbol visibility is determined in the following order:
 *	1. node's visibility as returned by 
 *	   Get_Visibility_From_Attribute(),
 *	2. otherwise, symbol name visibility as returned by 
 *	   Get_Visibility_from_Specification_File(),
 *	3. otherwise, default visibility as returned by 
 *	   Get_Default_Visibility().
 *	
 *  ST_EXPORT Adjust_Export_Class_Visibility(ST_EXPORT default_eclass, int sv)
 *	Adjusts the export class to the given visibility.
 *
 *  ST_EXPORT Get_Export_Class_For_Tree (tree decl_node, ST_class st_class , ST_SCLASS sclass)
 *	Gets the export class from a declaration tree as follow:
 *	1. gets scope for tree (global or local symbol),
 *	2. gets visibility as returned by Get_Visibility_For_Tree(),
 *	3. returns export class as given by 
 *	   Get_Export_Class_From_Scope_And_Visibility().
 *
 */
#include "vspec.h"
#include "vspec_parse.h"
#include "erglob.h"

#ifndef STV_DEFAULT
#define	STV_DEFAULT	0
#define	STV_INTERNAL	1
#define	STV_HIDDEN	2
#define	STV_PROTECTED	3
#endif

static int default_visibility = STV_DEFAULT;

static int
Get_Default_Visibility(tree decl_node, ST_CLASS st_class, ST_SCLASS sclass)
{
  if (ENV_Symbol_Visibility != STV_DEFAULT) 
    default_visibility = ENV_Symbol_Visibility;
  /* Default visibility only apply to definitions, not
     external declarations. */
  if ((st_class == CLASS_FUNC ||
       st_class == CLASS_VAR) &&
      sclass != SCLASS_EXTERN) {
    return default_visibility;
  }
  return STV_DEFAULT;
}

static void
Set_Default_Visibility(int visibility)
{
  default_visibility = visibility;
}

static int
Get_Visibility_From_Attribute (tree decl_node, ST_CLASS st_class, ST_SCLASS sclass)
{
  int sv = -1;
  tree visibility;
  
#if defined (TARG_ST) && (GNU_FRONT_END==33)
  /* (cbr) switch to new gcc frontend */
  visibility = lookup_attribute ("visibility", DECL_ATTRIBUTES (decl_node));
  if (visibility)
    visibility = TREE_VALUE(TREE_VALUE(visibility));
#else
  visibility = DECL_VISIBILITY(decl_node);
#endif
  if (visibility != NULL) {
    const char *which = TREE_STRING_POINTER (visibility);
    if (strcmp (which, "default") == 0)
      sv = STV_DEFAULT;
    else if (strcmp (which, "internal") == 0)
      sv = STV_INTERNAL;
    else if (strcmp (which, "hidden") == 0)
      sv = STV_HIDDEN;
    else if (strcmp (which, "protected") == 0)
      sv = STV_PROTECTED;
    else {
      FmtAssert (FALSE, ("Get_Visibility_For_Tree unexpected visibility %s",
			 which));
    }
  }
  return sv;
}

static int
Get_Visibility_From_Specification_File(tree decl_node, ST_CLASS st_class, ST_SCLASS sclass)
{
  int sv = -1;
  const char *name;
  static vspec_pattern_list_t *vspec;

  FmtAssert(DECL_ASSEMBLER_NAME(decl_node) != NULL, 
	    ("Get_Visibility_From_Specification_File(): Unnamed symbol declaration"));

  if (ENV_Symbol_Visibility_Spec_Filename == NULL) return sv;

  /* We take the assembler name for matching. */
  name = IDENTIFIER_POINTER (DECL_ASSEMBLER_NAME (decl_node));
  /* Parse specification file if not done. */
  if (vspec == NULL) {
    vspec = vspec_parse(ENV_Symbol_Visibility_Spec_Filename);
    if (vspec == NULL) {
      ErrMsg(EC_Misc_String, "visibility specification file", vspec_parse_strerr());
      ENV_Symbol_Visibility_Spec_Filename = NULL;
      return sv;
    }
  }
  
  sv = (int)vspec_pattern_list_match(vspec, name);
  return sv;
}

static int
Get_Visibility_For_Tree (tree decl_node, ST_CLASS st_class , ST_SCLASS sclass)
{
  int sv = STV_DEFAULT;
  if (st_class == CLASS_FUNC || st_class == CLASS_VAR) {
    sv = Get_Visibility_From_Attribute(decl_node, st_class, sclass);
    if (sv != -1) return sv;
    sv = Get_Visibility_From_Specification_File(decl_node, st_class, sclass);
    if (sv != -1) return sv;
    sv = Get_Default_Visibility(decl_node, st_class, sclass);
  }
  return sv;
}

static ST_EXPORT
Adjust_Export_Class_Visibility(ST_EXPORT default_eclass, int sv)
{
  ST_EXPORT eclass;
  switch(default_eclass) {
  case EXPORT_LOCAL:
  case EXPORT_LOCAL_INTERNAL:
    /* Local symbol. */
    if (sv == STV_INTERNAL) eclass = EXPORT_LOCAL_INTERNAL;
    else eclass = EXPORT_LOCAL;
    break;
  default:
    /* Global symbol. */
    switch (sv) {
    case STV_INTERNAL: eclass = EXPORT_INTERNAL; break;
    case STV_HIDDEN: eclass = EXPORT_HIDDEN; break;
    case STV_PROTECTED: eclass = EXPORT_PROTECTED; break;
    default: eclass = EXPORT_PREEMPTIBLE; break;
    }
  } 
  return eclass;
}

ST_EXPORT
Get_Export_Class_For_Tree (tree decl_node, ST_CLASS st_class, ST_SCLASS sclass)
{
  ST_EXPORT eclass;
  int sv;
  FmtAssert(TREE_CODE(decl_node) == VAR_DECL ||
	    TREE_CODE(decl_node) == FUNCTION_DECL, 
	    ("Get_Export_Scope_For_Tree unexpected tree"));
  FmtAssert(st_class == CLASS_FUNC ||
	    st_class == CLASS_VAR, 
	    ("Get_Export_Scope_For_Tree unexpected symbol class"));
  
  sv = Get_Visibility_For_Tree(decl_node, st_class, sclass);

  if (TREE_CODE(decl_node) == FUNCTION_DECL) {
    if (TREE_PUBLIC(decl_node)) eclass = EXPORT_PREEMPTIBLE;
    else eclass = EXPORT_LOCAL;
  } else if (TREE_CODE(decl_node) == VAR_DECL) {
    if (TREE_PUBLIC(decl_node) ||
	(DECL_CONTEXT (decl_node) != 0 && DECL_EXTERNAL(decl_node)))
      eclass = EXPORT_PREEMPTIBLE;
    else 
      eclass = EXPORT_LOCAL;
  }
  eclass = Adjust_Export_Class_Visibility(eclass, sv);
  return eclass;
}
#endif

#ifdef TARG_ST

#if __GNUC__ >=3
#include <ext/hash_map>
#else
#include <hash_map>
#endif

namespace {

  struct ptrhash {
    size_t operator()(void* p) const { return reinterpret_cast<size_t>(p); }
  };

  __GNUEXT::hash_map<tree, ST*,        ptrhash>     st_map;
}

ST*& DECL_ST(tree t) {
  if (
#ifndef TARG_ST
      /* (cbr) also for functions */
      TREE_CODE(t) == VAR_DECL			     &&
#endif
      (DECL_CONTEXT(t) == 0 || 
       TREE_CODE(DECL_CONTEXT(t)) == NAMESPACE_DECL) &&
#if defined (TARG_ST) && (GNU_FRONT_END==33)
      /* (cbr) 3.3 updade. be anon */
      DECL_NAME(t) &&
#endif
     DECL_ASSEMBLER_NAME(t))
    return st_map[DECL_ASSEMBLER_NAME(t)];
  else
    return st_map[t];
  }
#endif


