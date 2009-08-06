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


#include "namespace_trans.h"
#include "W_values.h"
#include "defs.h"
#include "errors.h"
#include "gnu_config.h"
extern "C" {
#include "gnu/flags.h"
}
#include "symtab.h"
extern "C" {
#include "gnu/system.h"
#include "gnu/tree.h"
#ifdef TARG_ST
  /* (cbr) fix includes paths */
#include "gnu/toplev.h"
#include "gnu/cp/cp-tree.h"
#include "gnu/real.h"
  /* (cbr) linkonce support */
#include "gnu/output.h"
#include "gnu/langhooks.h"
#include "tm_p.h"
#else
#include "cp-tree.h"
#endif
}
#undef TARGET_PENTIUM // hack around macro definition in gnu
#include "strtab.h"
#include "wn.h"
#include "wfe_expr.h"
#include "wfe_decl.h"
#include "wfe_misc.h"
#include "wfe_dst.h"
#include "ir_reader.h"
#include "tree_symtab.h"
#ifdef KEY
#include "wfe_stmt.h"
#include <map>
#endif

#include <ext/hash_map>
using __gnu_cxx::hash_map;
typedef struct {
    size_t operator()(void* p) const { return reinterpret_cast<size_t>(p); }
} void_ptr_hash;
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
// [CM] : Helper functions for `reloc' computation
static int categorize_addressed_constants (tree);
#endif

#ifdef TARG_ST
/* (cbr) for C++ const initializers */
extern "C" { 
extern tree ssdf_decl;
}
#endif

#ifdef TARG_ST
INT pstatic_as_global = 1;
#else
extern INT pstatic_as_global;
#endif

extern FILE *tree_dump_file; /* for debugging */
extern void Push_Deferred_Function(tree);

#ifdef KEY
// Map duplicate gcc nodes that refer to the same function.
std::multimap<tree, tree> duplicate_of;
void
add_duplicates (tree newdecl, tree olddecl)
{
  duplicate_of.insert (std::pair<tree, tree>(newdecl, olddecl));
  duplicate_of.insert (std::pair<tree, tree>(olddecl, newdecl));
}

// Remove all references to DECL from the map.
void
erase_duplicates (tree decl)
{
  int i, j;
  int count = duplicate_of.count (decl);

  for (i=0; i<count; i++) {
    std::multimap<tree, tree>::iterator iter = duplicate_of.find(decl);
    tree t = (*iter).second;

    // Erase entries with DECL as the data, i.e., <..., DECL>.
    int count2 = duplicate_of.count(t); 
    for (j=0; j<count2; j++) {
      std::multimap<tree, tree>::iterator iter2 = duplicate_of.find(t);
      tree t2 = (*iter2).second;
      if (t2 == decl) {
	duplicate_of.erase (iter2);
      }
    }

    // Erase entry with DECL as the key, i.e., <DECL, ...>.
    duplicate_of.erase (iter);
  }
}

static ST*
get_duplicate_st (tree decl)
{
  int count = duplicate_of.count (decl);

  for (int i=0; i<count; ++i) {
    std::multimap<tree, tree>::iterator iter = duplicate_of.find(decl);
    tree t = (*iter).second;
    // The node t could have been garbage-collected by gcc.  This is a crude
    // test to see if t is still valid.
    if (TREE_CODE(t) == FUNCTION_DECL &&
	DECL_NAME(t) == DECL_NAME(decl) &&
	DECL_ASSEMBLER_NAME_SET_P(t) == DECL_ASSEMBLER_NAME_SET_P(decl) &&
	(!DECL_ASSEMBLER_NAME_SET_P(t) ||
	 DECL_ASSEMBLER_NAME(t) == DECL_ASSEMBLER_NAME(decl))) {
      // Return the ST previously allocated, if any.
      ST *st = DECL_ST(t);
      if (st != NULL)
        return st;
    }
    duplicate_of.erase (iter);
  }
  return NULL;
}
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
		return ((char *) IDENTIFIER_POINTER (node));
	else if (TREE_CODE (node) == TYPE_DECL)
		// If type has a typedef-name, the TYPE_NAME is a TYPE_DECL.
		return ((char *) IDENTIFIER_POINTER (DECL_NAME (node)));
	else
		FmtAssert(FALSE, ("Get_Name unexpected tree"));
		return NULL;
}

static void
dump_field(tree field)
{
  fprintf(stderr, "%s:  ", Get_Name(DECL_NAME(field)));
  fprintf(stderr, "%d\n", DECL_FIELD_ID(field));
}

tree
#ifdef TARG_ST
/* (cbr) real_field can't be static */
next_real_or_virtual_field (tree type_tree, tree field, bool &real_field)
#else
next_real_or_virtual_field (tree type_tree, tree field)
#endif
{
#ifndef TARG_ST
  static bool real_field = true;
#endif
#if defined KEY && ! defined TARG_ST
  // [SC] The KEY version is non-reentrant, and there are nested calls to
  // this function, and so it can fail.  The ST version is reentrant.

  static tree prev_field = NULL_TREE;

  // If FIELD is not the same as the previously returned field, then we are
  // being called to traverse a new list or to traverse the same list over
  // again.  In either case, begin with the real fields.
  if (field != prev_field)
    real_field = true;

  if (field == TYPE_VFIELD(type_tree))
    real_field = false;

  if (TREE_CHAIN(field))
    return (prev_field = TREE_CHAIN(field));

  if (real_field && TYPE_VFIELD(type_tree)) {
    real_field = false;
    return (prev_field = TYPE_VFIELD(type_tree));
  }

  real_field = true;
  return (prev_field = NULL_TREE);

#else

  if (field == TYPE_VFIELD(type_tree))
    real_field = false;

  if (TREE_CHAIN(field))
    return TREE_CHAIN(field);
  
  if (real_field && TYPE_VFIELD(type_tree)) {
    real_field = false;
    return TYPE_VFIELD(type_tree);
  }

  real_field = true;
  return NULL_TREE;
#endif	// KEY
}

static void
Do_Base_Types (tree type_tree)
{
  tree binfo = TYPE_BINFO(type_tree);
  tree basetypes = binfo ? BINFO_BASETYPES(binfo) : 0;
  INT32 i;
  if (basetypes)
    for (i = 0; i < TREE_VEC_LENGTH(basetypes); ++i)
      (void) Get_TY (BINFO_TYPE(TREE_VEC_ELT(basetypes, i)));
}

size_t 
Roundup (size_t offset, int alignment)
{
  return (offset % alignment) ? offset + alignment - offset % alignment
			      : offset;
}

size_t
Type_Size_Without_Vbases (tree type_tree)
{
  tree field;
  tree last_field_decl = 0;

#ifdef TARG_ST
/* (cbr) real_field can't be static */
  bool real_field = true;
  for (field = TYPE_FIELDS(type_tree);
       field;
       field = next_real_or_virtual_field (type_tree, field, real_field))
#else
  for (field = TYPE_FIELDS(type_tree);
       field;
       field = next_real_or_virtual_field (type_tree, field))
#endif
    if (TREE_CODE(field) == FIELD_DECL)
      last_field_decl = field;

  if (last_field_decl == 0)
    return 0;

  return
    Get_Integer_Value (DECL_FIELD_OFFSET(last_field_decl))  		     +
    Get_Integer_Value (DECL_FIELD_BIT_OFFSET(last_field_decl)) / BITSPERBYTE +
    Get_Integer_Value (DECL_SIZE(last_field_decl)) / BITSPERBYTE;
} 

bool
is_empty_base_class (tree type_tree)
{
  tree field = TYPE_FIELDS(type_tree);
#if defined (TARG_ST) && (GNU_FRONT_END==33)
  /* (cbr) a class with a vptr will not be empty */
  return CLASSTYPE_EMPTY_P(type_tree);
#else
  return TREE_CODE(field) == TYPE_DECL && TREE_CHAIN(field) == 0;
#endif
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
#ifdef TARG_ST
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

  if (TYPE_SIZE(dim_tree)) {
    if (TREE_CODE(TYPE_MAX_VALUE(TYPE_DOMAIN(dim_tree))) == INTEGER_CST) {
      Set_ARB_const_ubnd (arb);
      Set_ARB_ubnd_val (arb, 
          Get_Integer_Value(TYPE_MAX_VALUE(TYPE_DOMAIN(dim_tree))));
    }
    else {
      WN *uwn = WFE_Expand_Expr(TYPE_MAX_VALUE(TYPE_DOMAIN(dim_tree)));
      if (WN_opcode (uwn) == OPC_U4I4CVT ||
	  WN_opcode (uwn) == OPC_U5I5CVT ||
	  WN_opcode (uwn) == OPC_U8I8CVT) {
	uwn = WN_kid0 (uwn);
      }
#ifdef TARG_ST
      /* (cbr) support for vla */
      ST *st;
      TY_IDX ty_idx;
      if (WN_operator (uwn) != OPR_LDID) {
	ty_idx = Get_TY(TREE_TYPE(TYPE_MAX_VALUE(TYPE_DOMAIN(dim_tree))));
        st = Gen_Temp_Symbol (ty_idx, "__save_vla");
        TYPE_ID mtype = TY_mtype (ty_idx);
        WFE_Set_ST_Addr_Saved (uwn);     
        uwn = WN_Stid (mtype, 0, st, ty_idx, uwn);
        WFE_Stmt_Append (uwn, Get_Srcpos());    
        uwn = WN_Ldid (mtype, 0, st, ty_idx);
      }
      else {
        st = WN_st (uwn);
        ty_idx = ST_type (st);
      }
#else
      FmtAssert (WN_operator (uwn) == OPR_LDID,
				("bounds operator for VLA not LDID"));
      ST *st = WN_st (uwn);
      TY_IDX ty_idx = ST_type (st);
#endif
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

// idx is non-zero only for RECORD and UNION, when there is forward declaration
extern TY_IDX
Create_TY_For_Tree (tree type_tree, TY_IDX idx)
{

	if(TREE_CODE(type_tree) == ERROR_MARK)
	   return idx;

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


	// for typedefs get the information from the base type
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
#ifdef KEY
		if (TYPE_RESTRICT(type_tree))
			Set_TY_is_restrict (idx);
#endif
#ifdef TARG_ST
                // (cbr) handle may_alias attribute
                if (lookup_attribute ("may_alias", TYPE_ATTRIBUTES (type_tree))) {
		  // [CL] don't apply the attribute to the predefined type
		  idx = Copy_TY(idx);
                  Set_TY_no_ansi_alias(idx);
                }
#endif
		TYPE_TY_IDX(type_tree) = idx;
		if(Debug_Level >= 2) {
#ifdef TARG_ST
		  defer_DST_type(type_tree, idx, 0);
#else		 
		  DST_INFO_IDX dst = Create_DST_type_For_Tree(type_tree,
			idx,orig_idx);
		  TYPE_DST_IDX(type_tree) = dst;
#endif
	        }
#ifdef TARG_ST
		if (TYPE_FIELD_IDS_USED_KNOWN(TYPE_MAIN_VARIANT(type_tree))) {
		  SET_TYPE_FIELD_IDS_USED(type_tree,
					  GET_TYPE_FIELD_IDS_USED(TYPE_MAIN_VARIANT(type_tree)));
		}
#else
		TYPE_FIELD_IDS_USED(type_tree) =
			TYPE_FIELD_IDS_USED(TYPE_MAIN_VARIANT(type_tree));
#endif
#ifdef TARG_ST
                /* (cbr) in case of forward decl don't emit a second time
                   member functions (bug pro-release-1-6-0-B/53) */
                TREE_ASM_WRITTEN(type_tree) = 1;
#endif
		return idx;
	}

	TYPE_ID mtype;
	INT tsize;
	BOOL variable_size = FALSE;
	tree type_size = TYPE_SIZE(type_tree);
	UINT align = TYPE_ALIGN(type_tree) / BITSPERBYTE;
	if (type_size == NULL) {
		// incomplete structs have 0 size.  Similarly, 'void' is
                // an incomplete type that can never be completed.
#ifdef TARG_ST
		FmtAssert(TREE_CODE(type_tree) == ARRAY_TYPE 
			|| TREE_CODE(type_tree) == ENUMERAL_TYPE
			|| TREE_CODE(type_tree) == UNION_TYPE
			|| TREE_CODE(type_tree) == RECORD_TYPE
			|| TREE_CODE(type_tree) == LANG_TYPE
			|| TREE_CODE(type_tree) == VOID_TYPE
			|| TREE_CODE(type_tree) == (enum tree_code)TEMPLATE_TYPE_PARM,
			  ("Create_TY_For_Tree: type_size NULL for non ARRAY/RECORD/VOID, type is %d",
                           (int) TREE_CODE(type_tree)));
#else
		FmtAssert(TREE_CODE(type_tree) == ARRAY_TYPE 
			|| TREE_CODE(type_tree) == ENUMERAL_TYPE
			|| TREE_CODE(type_tree) == UNION_TYPE
			|| TREE_CODE(type_tree) == RECORD_TYPE
			|| TREE_CODE(type_tree) == LANG_TYPE
			|| TREE_CODE(type_tree) == VOID_TYPE,
			  ("Create_TY_For_Tree: type_size NULL for non ARRAY/RECORD/VOID, type is %d",
                           (int) TREE_CODE(type_tree)));
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
                  /* (cbr) use TYPE_PRECISION for integers */
                  if (TREE_CODE(type_tree) == INTEGER_TYPE)
                    tsize = TYPE_PRECISION(type_tree) / BITSPERBYTE;
                  else
		    // FdF 20081105: Round the size up. Imported from
		    // Open64-4.2. Fix for codex #55746
                    tsize = (Get_Integer_Value(type_size) + BITSPERBYTE - 1) / BITSPERBYTE;
#else
                tsize = Get_Integer_Value(type_size) / BITSPERBYTE;
#endif
	}
	switch (TREE_CODE(type_tree)) {
	case VOID_TYPE:
	case LANG_TYPE: // unknown type
		idx = MTYPE_To_TY (MTYPE_V);	// use predefined type
		break;
	case BOOLEAN_TYPE:
	case INTEGER_TYPE:
		switch (tsize) {
		case 1:  mtype = MTYPE_I1;  break;
		case 2:  mtype = MTYPE_I2;  break;
		case 4:  mtype = MTYPE_I4;  break;
#ifdef TARG_ST
                  /* (cbr) */
                case 5:  mtype = MTYPE_I5; break;
#endif
		case 8:  mtype = MTYPE_I8;  break;
                case 16: mtype = MTYPE_I16; break;
		default:  FmtAssert(FALSE,
                                    ("Get_TY unexpected size %d", tsize));
		}
#if defined (TARG_ST) && (GNU_FRONT_END==33)
                /* (cbr) make sure boolean is signed as Boolean_type in whirl */
                if (TREE_CODE (type_tree) != BOOLEAN_TYPE)
#endif
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
		mtype = (TREE_UNSIGNED(type_tree) ? MTYPE_U4 : MTYPE_I4);
		idx = MTYPE_To_TY (mtype);	// use predefined type
		break;
	case REAL_TYPE:
		switch (tsize) {
		case 4:  mtype = MTYPE_F4; break;
		case 8:  mtype = MTYPE_F8; break;
		default:  FmtAssert(FALSE, ("Get_TY unexpected size"));
		}
		idx = MTYPE_To_TY (mtype);	// use predefined type
		break;
	case COMPLEX_TYPE:
		switch (tsize) {
		case  8:  mtype = MTYPE_C4; break;
		case 16:  mtype = MTYPE_C8; break;
		default:  FmtAssert(FALSE, ("Get_TY unexpected size"));
		}
		idx = MTYPE_To_TY (mtype);	// use predefined type
		break;
	case POINTER_TYPE:
		if (TYPE_PTRMEM_P(type_tree)) {
			// pointer to member
			idx = Be_Type_Tbl(Pointer_Size == 8 ? MTYPE_I8 : MTYPE_I4);
			break;
		}
		/* FALLTHRU */
	case REFERENCE_TYPE:
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
		if (TYPE_SIZE(TREE_TYPE(type_tree)) == 0)
			break; // anomaly:  type will never be needed
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
			    WN_opcode (swn) == OPC_U8I8CVT) {
				swn = WN_kid0 (swn);
			}
#if 0
#ifdef TARG_ST
                        TY_IDX    ty_idx  = Get_TY (TREE_TYPE(type_size));
                        ST *st = Gen_Temp_Symbol (ty_idx, "__stride_vla");
                        TYPE_ID mtype = TY_mtype (ty_idx);

                        swn = WN_Stid (mtype, 0, st, ty_idx, swn);
                        WFE_Stmt_Append (swn, Get_Srcpos());

                        swn = WN_Ldid (mtype, 0, st, ty_idx);
#else
			FmtAssert (WN_operator (swn) == OPR_LDID,
				("stride operator for VLA not LDID"));
			ST *st = WN_st (swn);
			TY_IDX ty_idx = ST_type (st);
#endif

			WN *wn = WN_CreateXpragma (WN_PRAGMA_COPYIN_BOUND,
						   (ST_IDX) NULL, 1);
			WN_kid0 (wn) = WN_Ldid (TY_mtype (ty_idx), 0, st, ty_idx);
			WFE_Stmt_Append (wn, Get_Srcpos());
			Clear_ARB_const_stride (arb);
			Set_ARB_stride_var (arb, (ST_IDX) ST_st_idx (st));
#endif
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
#if 0
			WN *uwn = WFE_Expand_Expr (TYPE_MAX_VALUE (TYPE_DOMAIN (type_tree)) );
			if (WN_opcode (uwn) == OPC_U4I4CVT ||
			    WN_opcode (uwn) == OPC_U8I8CVT) {
				uwn = WN_kid0 (uwn);
			}
			ST *st;
			TY_IDX ty_idx;
			WN *wn;
			if (WN_operator (uwn) != OPR_LDID) {
				ty_idx = Get_TY (TREE_TYPE (TYPE_MAX_VALUE (TYPE_DOMAIN (type_tree)) ) );
				st = Gen_Temp_Symbol (ty_idx, "__vla_bound");
				wn = WN_Stid (TY_mtype (ty_idx), 0, st, ty_idx, uwn);
				WFE_Stmt_Append (wn, Get_Srcpos());
			}
			else {
				st = WN_st (uwn);
				ty_idx = ST_type (st);
			}
			wn = WN_CreateXpragma (WN_PRAGMA_COPYIN_BOUND, (ST_IDX) NULL, 1);
			WN_kid0 (wn) = WN_Ldid (TY_mtype (ty_idx), 0, st, ty_idx);
			WFE_Stmt_Append (wn, Get_Srcpos());
			Clear_ARB_const_ubnd (arb);
			Set_ARB_ubnd_var (arb, ST_st_idx (st));
#endif
		    }
		}
		else {
			Clear_ARB_const_ubnd (arb);
			Set_ARB_ubnd_val (arb, 0);
		}
#if 0
		if (variable_size) {
			WN *swn, *wn;
			swn = WFE_Expand_Expr (type_size);
			if (TY_size(TY_etype(ty))) {
				if (WN_opcode (swn) == OPC_U4I4CVT ||
				    WN_opcode (swn) == OPC_U8I8CVT) {
					swn = WN_kid0 (swn);
				}
#ifdef TARG_ST
                                TY_IDX    ty_idx  = Get_TY (TREE_TYPE(type_size));
                                ST *st = Gen_Temp_Symbol (ty_idx, "__size_vla");
				TYPE_ID mtype = TY_mtype (ty_idx);

                                swn = WN_Stid (mtype, 0, st, ty_idx, swn);
				WFE_Stmt_Append (swn, Get_Srcpos());

                                swn = WN_Ldid (mtype, 0, st, ty_idx);
				swn = WN_Div (mtype, swn, WN_Intconst (mtype, BITSPERBYTE));

#else
				FmtAssert (WN_operator (swn) == OPR_LDID,
					("size operator for VLA not LDID"));
				ST *st = WN_st (swn);
				TY_IDX ty_idx = ST_type (st);
				TYPE_ID mtype = TY_mtype (ty_idx);
				swn = WN_Div (mtype, swn, WN_Intconst (mtype, BITSPERBYTE));
#endif
				wn = WN_Stid (mtype, 0, st, ty_idx, swn);
				WFE_Stmt_Append (wn, Get_Srcpos());
			}
		}
#endif
		} // end array scope
#endif /* TARG_ST */

		break;
	case RECORD_TYPE:
	case UNION_TYPE:
		{	// new scope for local vars
                TY &ty = (idx == TY_IDX_ZERO) ? New_TY(idx) : Ty_Table[idx];
#ifdef KEY
		// Must create DSTs in the order that the records are declared,
		// in order to preserve their scope.  Bug 4168.
		if (Debug_Level >= 2)
		  defer_DST_type(type_tree, idx, orig_idx);
#endif	// KEY
		TY_Init (ty, tsize, KIND_STRUCT, MTYPE_M, 
			Save_Str(Get_Name(TYPE_NAME(type_tree))) );
		if (TREE_CODE(type_tree) == UNION_TYPE) {
			Set_TY_is_union(idx);
		}
		if (align == 0) align = 1;	// in case incomplete type
		Set_TY_align (idx, align);
		// set idx now in case recurse thru fields
		TYPE_TY_IDX(type_tree) = idx;

#ifdef TARG_ST
                /* (cbr) mark non-pods */
                if (TYPE_LANG_SPECIFIC (type_tree) &&
                    TREE_ADDRESSABLE (type_tree) && 
                    CLASSTYPE_NON_POD_P (type_tree)) 

                  Set_TY_is_non_pod(idx);
#endif

                Do_Base_Types (type_tree);

#ifdef TARG_ST
                  /* (cbr) from now fields are handled */
                  TREE_ASM_WRITTEN(type_tree) = 1;
#endif

		// Process nested structs and static data members first

#ifdef TARG_ST
/* (cbr) real_field can't be static */
                bool real_field = true;
                for (tree field =  TYPE_FIELDS (type_tree);
                          field;
                     field = next_real_or_virtual_field(type_tree, field,
                                                        real_field))
#else
                for (tree field =  TYPE_FIELDS (type_tree);
                          field;
                     field = next_real_or_virtual_field(type_tree, field))
#endif
                  {
                    if (TREE_CODE(field) == TYPE_DECL ||
                        TREE_CODE(field) == FIELD_DECL) {
                      tree field_type = TREE_TYPE(field);
                      if ((TREE_CODE(field_type) == RECORD_TYPE ||
                           TREE_CODE(field_type) == UNION_TYPE) &&
                          field_type != type_tree)
#ifdef KEY
			// Defer typedefs within class
			// declarations to avoid circular
			// declaration dependences.  See
			// example in bug 5134.
			if (TREE_CODE(field) == TYPE_DECL)
			  defer_decl(field_type);
			else
#endif
                        Get_TY(field_type);
                    }
#ifdef KEY	// Defer expansion of static vars until all the fields in
		// _every_ struct are laid out.  Consider this code (see
		// bug 3044):
		//  struct A
		//    struct B *p
		//  struct B
		//    static struct A *q = ...	// static data member with
		//                              // initializer
		// We cannot expand static member vars while expanding the
		// enclosing stuct, for the following reason:  Expansion of
		// struct A leads to expansion of p, which leads to the
		// expansion of struct B, which leads to the expansion of q and
		// q's initializer.  The code that expands the initializer goes
		// through the fields of struct A, but these fields are not yet
		// completely defined, and this will cause kg++fe to die.
		//
		// The solution is the delay all static var expansions until
		// the very end.
			else if (TREE_CODE(field) == VAR_DECL)
				defer_decl(field);
#else
                    else if (TREE_CODE(field) == VAR_DECL)
                        WFE_Expand_Decl(field);
#endif
		    else if (TREE_CODE(field) == (enum tree_code)TEMPLATE_DECL)
		      WFE_Expand_Decl(field);
                  }

  		Set_TY_fld (ty, FLD_HANDLE());
		FLD_IDX first_field_idx = Fld_Table.Size ();
		tree field;
		tree method = TYPE_METHODS(type_tree);
		FLD_HANDLE fld;
		INT32 next_field_id = 1;
		hash_map <tree, tree, void_ptr_hash> anonymous_base;

		// Generate an anonymous field for every direct, nonempty,
		// nonvirtual base class.  

		INT32 offset = 0;
		INT32 anonymous_fields = 0;
#ifndef KEY     // g++'s class.c already laid out the base types.  Bug 11622.
		if (TYPE_BINFO(type_tree) &&
		    BINFO_BASETYPES(TYPE_BINFO(type_tree))) {
		  tree basetypes = BINFO_BASETYPES(TYPE_BINFO(type_tree));
		  INT32 i;
		  for (i = 0; i < TREE_VEC_LENGTH(basetypes); ++i) {
		    tree binfo = TREE_VEC_ELT(basetypes, i);
		    tree basetype = BINFO_TYPE(binfo);
		    offset = Roundup (offset,
				    TYPE_ALIGN(basetype) / BITSPERBYTE);
		    if (!is_empty_base_class(basetype) || 
			!TREE_VIA_VIRTUAL(binfo)) {
		      ++next_field_id;
		      ++anonymous_fields;
		      next_field_id += TYPE_FIELD_IDS_USED(basetype);
		      fld = New_FLD();
		      FLD_Init (fld, Save_Str(Get_Name(0)), 
				Get_TY(basetype) , offset);
		      offset += Type_Size_Without_Vbases (basetype);
                      // For the field with base class type,
                      // set it to anonymous and base class,
                      // and add it into anonymous_base set
                      // to avoid create the anonymous field twice in the TY
                      Set_FLD_is_anonymous(fld);
                      Set_FLD_is_base_class(fld);
                      anonymous_base.insert(CLASSTYPE_AS_BASE(basetype));
#ifdef KEY
// temporary hack for a bug in gcc
// Details: From layout_class_type(), it turns out that for this
// type, gcc is apparently sending wrong type info, they have 2 fields
// each 8 bytes in a 'record', with the type size == 8 bytes also!
// So we take care of it here...
		      if (offset > tsize)
			{
			    tsize = offset;
			    Set_TY_size (ty, tsize);
			}
#endif // KEY
		    }
		  }
		}
#endif
                // find all base classes
                if (TYPE_BINFO(type_tree) && BINFO_BASETYPES(TYPE_BINFO(type_tree))) {
                  tree basetypes = BINFO_BASETYPES(TYPE_BINFO(type_tree));
                  INT32 i;
                  for (i = 0; i < TREE_VEC_LENGTH(basetypes); ++i) {
                    tree basetype = BINFO_TYPE(TREE_VEC_ELT(basetypes, i));
                    anonymous_base[CLASSTYPE_AS_BASE(basetype)] = basetype;
                  }
                }

#ifdef TARG_ST
/* (cbr) real_field can't be static */
                bool real_field_2 = true;
		for (field = TYPE_FIELDS(type_tree); 
                     field;
                     field = next_real_or_virtual_field(type_tree, field,
                                                        real_field_2))
#else
		for (field = TYPE_FIELDS(type_tree); 
                       field;
                       field = next_real_or_virtual_field(type_tree, field) )
#endif
		{
			if (TREE_CODE(field) == TYPE_DECL) {
				continue;
			}
			if (TREE_CODE(field) == CONST_DECL) {
                // Just for the time being, we need to investigate
                // whether this criteria is reasonable.
                static BOOL once_is_enough=FALSE;
                if (!once_is_enough) {
				DevWarn ("got CONST_DECL in field list");
                  once_is_enough=TRUE;
                }
				continue;
			}
			if (TREE_CODE(field) == VAR_DECL) {
				continue;	
			}
			if (TREE_CODE(field) == (enum tree_code)TEMPLATE_DECL) {
				continue;
			}
#ifdef TARG_ST
                        /* (cbr) it's possible not to have any layout info, e.g 
                         we belong to a template */
                        if (!DECL_FIELD_OFFSET(field)) {
                          continue;
                        }
#endif
			DECL_FIELD_ID(field) = next_field_id;
			next_field_id += 
#ifdef TARG_ST
			  GET_TYPE_FIELD_IDS_USED(TREE_TYPE(field)) + 1;
#else
			  TYPE_FIELD_IDS_USED(TREE_TYPE(field)) + 1;
#endif
			fld = New_FLD ();
			FLD_Init (fld, Save_Str(Get_Name(DECL_NAME(field))), 
				0, // type
				Get_Integer_Value(DECL_FIELD_OFFSET(field)) +
				Get_Integer_Value(DECL_FIELD_BIT_OFFSET(field))
					/ BITSPERBYTE);
                        if (DECL_NAME(field) == NULL)
                          Set_FLD_is_anonymous(fld);
                        if (anonymous_base.find(TREE_TYPE(field)) != anonymous_base.end()) {
                          Set_FLD_is_base_class(fld);
                          // set the base class type;
                          tree base_class = anonymous_base[TREE_TYPE(field)];
                          Set_FLD_type(fld, Get_TY(base_class));
#ifdef TARG_ST
			  // [SC] We just carefully calculated next_field_id for
			  // the anonymous_base_type, and now we are instead 
			  // using the base class type.  The number of fields
			  // differs between these (anonymous base does not
			  // contain the virtual base classes), so to avoid
			  // inconsistencies later, we must adjust here.
			  next_field_id += (GET_TYPE_FIELD_IDS_USED (base_class)
					    - GET_TYPE_FIELD_IDS_USED (TREE_TYPE(field)));
#endif
                        }
		}

#ifdef TARG_ST
		SET_TYPE_FIELD_IDS_USED(type_tree, next_field_id - 1);
#else
		TYPE_FIELD_IDS_USED(type_tree) = next_field_id - 1;
#endif
  		FLD_IDX last_field_idx = Fld_Table.Size () - 1;
		if (last_field_idx >= first_field_idx) {
			Set_TY_fld (ty, FLD_HANDLE (first_field_idx));
			Set_FLD_last_field (FLD_HANDLE (last_field_idx));
		}
		// now set the fld types.
		// first skip the anonymous fields, whose types are already
		// set.
		fld = TY_fld(ty);
		while (anonymous_fields--)
		  fld = FLD_next(fld);
#ifdef TARG_ST
/* (cbr) real_field can't be static */
                real_field_2 = true;
		for (field = TYPE_FIELDS(type_tree);
			field;
			field = next_real_or_virtual_field(type_tree, field, real_field_2))
#else
		for (field = TYPE_FIELDS(type_tree);
		     /* ugly hack follows; traversing the fields isn't
                        the same from run-to-run. fwa? */
			field && fld.Entry();
			field = next_real_or_virtual_field(type_tree, field))
#endif
		{
#ifdef KEY
		        // FdF 20081105: Fix for codex #55746
			const  int FLD_BIT_FIELD_SIZE   = 64;
#endif
			if (TREE_CODE(field) == TYPE_DECL)
				continue;
			if (TREE_CODE(field) == CONST_DECL)
				continue;
			if (TREE_CODE(field) == VAR_DECL)
				continue;
			if (TREE_CODE(field) == (enum tree_code)TEMPLATE_DECL)
				continue;
#ifdef TARG_ST
                        /* (cbr) it's possible not to have any layout info, e.g 
                         we belong to a template */
                        if (!DECL_FIELD_OFFSET(field)) {
                          continue;
                        }
#endif
#ifdef KEY
			// Don't expand the field's type if it's a pointer
			// type, in order to avoid circular dependences
			// involving member object types and base types.  See
			// example in bug 4954.  
			if (TREE_CODE(TREE_TYPE(field)) == POINTER_TYPE) {
				// Defer expanding the field's type.  Put in a
				// generic pointer type for now.
				TY_IDX p_idx =
				  Make_Pointer_Type(MTYPE_To_TY(MTYPE_U8),
						    FALSE);
				Set_FLD_type(fld, p_idx);
				defer_field(field, fld);
				fld = FLD_next(fld);
				continue;
			}
#endif
                        if (anonymous_base.find(TREE_TYPE(field)) == anonymous_base.end()) {
			  TY_IDX fty_idx = Get_TY(TREE_TYPE(field));

#ifdef TARG_ST
			  // [CG]: For structure fields, the qualifiers are
			  // on the field nodes (not on the field node type).
			  // Thus we get them explicitly.
			  if (TREE_THIS_VOLATILE(field)) Set_TY_is_volatile (fty_idx);
			  if (TREE_READONLY(field)) Set_TY_is_const (fty_idx);
			  if (TREE_RESTRICT(field)) Set_TY_is_restrict (fty_idx);
#endif
			  if ((TY_align (fty_idx) > align) || (TY_is_packed (fty_idx)))
			    Set_TY_is_packed (ty);
			  Set_FLD_type(fld, fty_idx);
			}
			if ( ! DECL_BIT_FIELD(field)
#ifdef TARG_ST
                             /* (cbr) might not have a size */
                              && DECL_SIZE (field)
#endif
				&& Get_Integer_Value(DECL_SIZE(field)) > 0
#ifdef KEY
			     // FdF 20081105: Fix for codex #55746
// We don't handle bit-fields > 64 bits. For an INT field of 128 bits, we
// make it 64 bits. But then don't set it as FLD_IS_BIT_FIELD.
				&& Get_Integer_Value(DECL_SIZE(field)) <= 
				   FLD_BIT_FIELD_SIZE
				// bug 2401
				&& TY_size(Get_TY(TREE_TYPE(field))) != 0
#endif
				&& Get_Integer_Value(DECL_SIZE(field))
				  != (TY_size(Get_TY(TREE_TYPE(field))) 
					* BITSPERBYTE) )
			{
#ifdef KEY
			        FmtAssert( Get_Integer_Value(DECL_SIZE(field)) <=
					   FLD_BIT_FIELD_SIZE,
					   ("field size too big") );
#endif
				// for some reason gnu doesn't set bit field
				// when have bit-field of standard size
				// (e.g. int f: 16;).  But we need it set
				// so we know how to pack it, because 
				// otherwise the field type is wrong.
				DevWarn("field size %d doesn't match type size %d", 
					(int)Get_Integer_Value(DECL_SIZE(field)),
					(int)TY_size(Get_TY(TREE_TYPE(field)))
						* BITSPERBYTE );
				DECL_BIT_FIELD(field) = 1;
			}
			if (DECL_BIT_FIELD(field)) {
				Set_FLD_is_bit_field (fld);
				// bofst is remaining bits from byte offset
				Set_FLD_bofst (fld, 
					Get_Integer_Value(
						DECL_FIELD_BIT_OFFSET(field))
						% BITSPERBYTE);
				Set_FLD_bsize (fld, Get_Integer_Value(
                                                           DECL_SIZE(field)));
			}
			fld = FLD_next(fld);
		}
#ifndef KEY	// Don't expand methods by going through TYPE_METHODS,
		// because:
		//   1) It is incorrect to translate all methods in
		//      TYPE_METHODS to WHIRL because some of the methods are
		//      never used, and generating the assembly code for them
		//      might lead to undefined symbol references.  Instead,
		//      consult the gxx_emitted_decls list, which has all the
		//      functions (including methods) that g++ has ever emitted
		//      to assembly.
		//   2) Expanding the methods here will cause error when the
		//      methods are for a class B that appears as a field in an
		//      enclosing class A.  When Get_TY is run for A, it will
		//      call Get_TY for B in order to calculate A's field ID's.
		//      (Need Get_TY to find B's TYPE_FIELD_IDS_USED.)  If
		//      Get_TY uses the code below to expand B's methods, it
		//      will lead to error because the expansion requires the
		//      field ID's of the enclosing record (A), and these field
		//      ID's are not yet defined.

		// process methods
                if (!Enable_WFE_DFE) {
#if defined (TARG_ST) && (GNU_FRONT_END==33)
                  /* (cbr) gcc 3.3 upgrade */
                  if (!cp_type_quals (type_tree)) {           
#else
                    if (CP_TYPE_QUALS(type_tree) == TYPE_UNQUALIFIED) {
#endif
			while (method != NULL_TREE) {
                          WFE_Expand_Decl (method);
                          method = TREE_CHAIN(method);
			}
		}
		}
#endif	// KEY
		} //end record scope
		break;
	case METHOD_TYPE:
		//DevWarn ("Encountered METHOD_TYPE at line %d", lineno);
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

		ret_ty_idx = Get_TY(TREE_TYPE(type_tree));

		for (arg = TYPE_ARG_TYPES(type_tree);
		     arg;
		     arg = TREE_CHAIN(arg))
			arg_ty_idx = Get_TY(TREE_VALUE(arg));

		// if return type is pointer to a zero length struct
		// convert it to void
		if (!WFE_Keep_Zero_Length_Structs    &&
		    TY_mtype (ret_ty_idx) == MTYPE_M &&
		    TY_size (ret_ty_idx) == 0) {

			// zero length struct being returned
		  	DevWarn ("function returning zero length struct at line %d", lineno);
			ret_ty_idx = Be_Type_Tbl (MTYPE_V);
		}

#ifdef TARG_ST
                /* (cbr) returned in hidden param */
                if (RETURN_IN_MEMORY (TREE_TYPE(type_tree)) ||
                 (TYPE_LANG_SPECIFIC(TREE_TYPE(type_tree)) &&
                  TREE_ADDRESSABLE (TREE_TYPE(type_tree)) && 
                  CLASSTYPE_NON_POD_P (TREE_TYPE(type_tree)))) {
                  Set_TY_return_to_param(idx);
                }
#endif

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
		  
#if defined (TARG_ST) && (GNU_FRONT_END==33)
        case TEMPLATE_TYPE_PARM:
          break;
#endif
	default:
		FmtAssert(FALSE, ("Get_TY unexpected tree_type"));
	}

#ifdef TARG_ST
        /* (cbr) */
        if (TYPE_NOTHROW_P (type_tree))
          Set_TY_is_nothrow (idx);
#endif
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
#ifdef KEY
	if (TYPE_RESTRICT(type_tree))
		Set_TY_is_restrict (idx);
#endif
	TYPE_TY_IDX(type_tree) = idx;
        if(Debug_Level >= 2) {
#ifdef KEY
	  // DSTs for records were entered into the defer list in the order
	  // that the records are declared, in order to preserve their scope.
	  // Bug 4168.
	  if (TREE_CODE(type_tree) != RECORD_TYPE &&
	      TREE_CODE(type_tree) != UNION_TYPE) {
	    // Defer creating DST info until there are no partially constructed
	    // types, in order to prevent Create_DST_type_For_Tree from calling
	    // Get_TY, which in turn may use field IDs from partially created
	    // structs.  Such fields IDs are wrong.  Bug 5658.
	    defer_DST_type(type_tree, idx, orig_idx);
	  }
#else
          DST_INFO_IDX dst =
            Create_DST_type_For_Tree(type_tree,
              idx,orig_idx);
          TYPE_DST_IDX(type_tree) = dst;
#endif
        }

#ifdef WFE_DEBUG
  print_node_brief (stdout, "  END Create_TY_For_Tree: ", type_tree, 0);
  fprintf(stdout,"\n    new_idx %d, orig_idx: %d, new_ty: %p, orig_ty: %p\n", 
	  idx, orig_idx, &(Ty_Table[idx]), &(Ty_Table[orig_idx]));
#endif


	return idx;
}

ST*
Create_ST_For_Tree (tree decl_node)
{
  TY_IDX     ty_idx;
  ST*        st;
  char      *name;
  char	    tempname[32];
  ST_SCLASS  sclass;
  ST_EXPORT  eclass;
  SYMTAB_IDX level;
  static INT anon_count = 0;

  if(TREE_CODE(decl_node) == ERROR_MARK) {
        Fail_FmtAssertion ("Unable to handle ERROR_MARK. internal error");
  }

#ifdef WFE_DEBUG
  print_node_brief (stdout,"  Create_ST_For_Tree: ", decl_node, 0);
  fprintf(stdout, "\n");
#endif

#ifdef KEY
  // If the decl is a function decl, and there are duplicate decls for the
  // function, then use a ST already allocated for the function, if such ST
  // exists.
  if (TREE_CODE (decl_node) == FUNCTION_DECL) {
    st = get_duplicate_st (decl_node);
    if (st) {
      set_DECL_ST(decl_node, st);
      return st;
    }
  }
#endif

#if defined (TARG_ST) && (GNU_FRONT_END==33)
  /* (cbr) gcc 3.3 upgrade */
  if (DECL_ASSEMBLER_NAME_SET_P (decl_node))
#else
  if (DECL_ASSEMBLER_NAME (decl_node))
#endif
    name = (char *) IDENTIFIER_POINTER (DECL_ASSEMBLER_NAME (decl_node));
#ifdef TARG_ST
    /* (cbr) get a name that makes sense */
  else if (DECL_NAME (decl_node)) {
    mangle_decl (decl_node);
    if (DECL_ASSEMBLER_NAME_SET_P (decl_node))
      name = (char *) IDENTIFIER_POINTER (DECL_ASSEMBLER_NAME (decl_node));
    else
      name = (char *) IDENTIFIER_POINTER (DECL_NAME (decl_node));
  }
#endif
  else {
#ifdef TARG_ST
    /* (cbr) avoid clash with user naming */
    sprintf(tempname, ".anon%d", ++anon_count);
#else
    sprintf(tempname, "anon%d", ++anon_count);
#endif
    name = tempname;
  }

  //   printf ("create %s 0x%x\n", name, decl_node);

#ifdef TARG_ST
   /* (cbr) in case for recursive self referencing type */
   ty_idx = Get_TY (TREE_TYPE(decl_node));
   if (st = DECL_ST(decl_node)) return st;
#endif

#ifdef KEY
  BOOL guard_var = FALSE;
  // See if variable is a guard variable.
  if (strncmp("_ZGV", name, 4) == 0) {
    guard_var = TRUE;
  }
#endif

  switch (TREE_CODE(decl_node)) {

    case FUNCTION_DECL:
      {
        if (Enable_WFE_DFE) {
          tree body = DECL_SAVED_TREE(decl_node);

          if (DECL_THUNK_P(decl_node) &&
              TREE_CODE(CP_DECL_CONTEXT(decl_node)) != NAMESPACE_DECL)
            Push_Deferred_Function (decl_node);
/*
          else
          if (DECL_TINFO_FN_P(decl_node))
            Push_Deferred_Function (decl_node);
*/
          else
          if (body != NULL_TREE && !DECL_EXTERNAL(decl_node) &&
#ifdef TARG_ST
              /* (cbr) */
              (TREE_PUBLIC(decl_node) ||
               (TREE_USED (decl_node) || TREE_ADDRESSABLE (decl_node))) &&
#endif
              (DECL_TEMPLATE_INFO(decl_node) == NULL              ||
               DECL_FRIEND_PSEUDO_TEMPLATE_INSTANTIATION(decl_node) ||
               DECL_TEMPLATE_INSTANTIATED(decl_node)              ||
               DECL_TEMPLATE_SPECIALIZATION(decl_node))) {
            Push_Deferred_Function (decl_node);
          }
        }

#ifndef TARG_ST
        /* (cbr) move above */
        TY_IDX func_ty_idx = Get_TY(TREE_TYPE(decl_node));
#endif

        sclass = SCLASS_EXTERN;
#ifdef TARG_ST
	eclass = Get_Export_Class_For_Tree(decl_node, CLASS_FUNC, sclass);
#else
        eclass = TREE_PUBLIC(decl_node) || DECL_WEAK(decl_node) ?
          EXPORT_PREEMPTIBLE :  EXPORT_LOCAL;
#endif

        level  = GLOBAL_SYMTAB+1;

        PU_IDX pu_idx;
        PU&    pu = New_PU (pu_idx);

#ifdef TARG_ST
        /* (cbr) move above */
        PU_Init (pu, ty_idx, level);
#else
        PU_Init (pu, func_ty_idx, level);
#endif

        st = New_ST (GLOBAL_SYMTAB);

        ST_Init (st,
                 Save_Str ( IDENTIFIER_POINTER (DECL_ASSEMBLER_NAME (decl_node))),
                 CLASS_FUNC, sclass, eclass, TY_IDX (pu_idx));

#ifndef TARG_ST
        /* (cbr) */
	if (DECL_THUNK_P(decl_node) &&
            TREE_CODE(CP_DECL_CONTEXT(decl_node)) != NAMESPACE_DECL)
          Set_ST_is_weak_symbol(st);
#endif

#ifdef TARG_ST
        /* (cbr) allow writable const in here */
        if (decl_node == ssdf_decl)
          Set_PU_static_initializer (pu);          
#endif

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
#ifdef KEY
    case RESULT_DECL:	// bug 3878
#endif
      {
        if (TREE_CODE(decl_node) == PARM_DECL) {
          sclass = SCLASS_FORMAL;
          eclass = EXPORT_LOCAL;
          level = CURRENT_SYMTAB;
        }
        else {
          if (DECL_CONTEXT (decl_node) == 0 			     ||
	      TREE_CODE (DECL_CONTEXT (decl_node)) == NAMESPACE_DECL ||
 	      TREE_CODE (DECL_CONTEXT (decl_node)) == RECORD_TYPE ) {
            if (TREE_PUBLIC (decl_node)) {
	      if (DECL_EXTERNAL(decl_node))
		sclass = SCLASS_EXTERN;
	      else
	      if (DECL_INITIAL(decl_node))
#ifdef TARG_ST
                /* (cbr) fix */
		sclass = SCLASS_DGLOBAL;
#else
		sclass = SCLASS_UGLOBAL;
#endif
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
#ifdef TARG_ST
                /* (cbr) static data member or static data in function member 
                   has external linkage */
            if (DECL_CONTEXT (decl_node) && TREE_STATIC (decl_node) &&
                TYPE_NEEDS_CONSTRUCTING (TREE_TYPE (decl_node)) &&
                DECL_COMDAT (DECL_CONTEXT (decl_node))) {
              tree init = DECL_INITIAL (decl_node);
              // (cbr) will be constructed at runtime. reserve space only
              if (!init)
                sclass = SCLASS_COMMON;
              else
                sclass = SCLASS_EXTERN;                

	      level  = GLOBAL_SYMTAB;
	      eclass = Get_Export_Class_For_Tree(decl_node, CLASS_VAR, sclass);
            }
            else
#endif
            if (DECL_EXTERNAL(decl_node) ||
                DECL_WEAK (decl_node)) {

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
#ifdef TARG_ST
                /* (cbr) for label_decl, take current_symtab */
                if (pstatic_as_global)
                  {
                    tree init = DECL_INITIAL (decl_node);
                    if (init) {
                      if (TREE_CODE (init) == ADDR_EXPR &&
                          TREE_CODE (TREE_OPERAND(init,0)) == LABEL_DECL)
                        level = CURRENT_SYMTAB;
                      else if (TREE_CODE (init) == CONSTRUCTOR) {
                        tree init2;
                        for (init2 = CONSTRUCTOR_ELTS(init);
                             init2;
                             init2 = TREE_CHAIN(init2)) {
                          if (TREE_CODE (TREE_VALUE(init2)) == ADDR_EXPR &&
                              TREE_CODE (TREE_OPERAND (TREE_VALUE(init2), 0)) ==  LABEL_DECL) {
                            level = CURRENT_SYMTAB;
                          }
                        }
                      }
                    }
                  }
#endif
              }
              else {
		sclass = SCLASS_AUTO;
	 	level = DECL_SYMTAB_IDX(decl_node) ?
			DECL_SYMTAB_IDX(decl_node) : CURRENT_SYMTAB;
              }
              eclass = EXPORT_LOCAL;
            }
          }
        }
#ifdef KEY
	// Make g++ guard variables global in order to make them weak.  Ideally
	// guard variables should be "common", but for some reason the back-end
	// currently can't handle C++ commons.  As a work around, make the
	// guard variables weak.  Since symtab_verify.cxx don't like weak
	// locals, make the guard variables global.
	if (guard_var) {
	  level = GLOBAL_SYMTAB;
	  sclass = SCLASS_UGLOBAL;
	  eclass = EXPORT_PREEMPTIBLE;
	}
#endif
        st = New_ST (level);
#ifndef TARG_ST
        /* (cbr) move above */
        ty_idx = Get_TY (TREE_TYPE(decl_node));
#endif
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

#if defined (TARG_ST) && (GNU_FRONT_END==33)
        /* (cbr) pass it through memory */
        if (TREE_CODE(decl_node) == PARM_DECL &&
            TREE_ADDRESSABLE(TREE_TYPE(decl_node))) {
          ty_idx = Make_Pointer_Type(ty_idx);
        }
#endif

	// qualifiers are set on decl nodes
	if (TREE_READONLY(decl_node))
          Set_TY_is_const (ty_idx);
	if (TREE_THIS_VOLATILE(decl_node))
          Set_TY_is_volatile (ty_idx);

#ifdef TARG_ST
        // (cbr) alignment can be changed by attribute
        if (DECL_USER_ALIGN (TREE_TYPE(decl_node))) {
          UINT align = DECL_ALIGN(decl_node) / BITSPERBYTE;
          if (align) Set_TY_align (ty_idx, align);
        }
	if (TREE_RESTRICT(decl_node))
	    Set_TY_is_restrict (ty_idx) ;
#endif /* TARG_ST */
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

        ST_Init (st, Save_Str(name), CLASS_VAR, sclass, eclass, ty_idx);
        if (TREE_CODE(decl_node) == PARM_DECL) {
            Set_ST_is_value_parm(st);
        }
#ifdef TARG_ST
        /* (cbr) mark readonly vars const */
        if (sclass != SCLASS_AUTO && 
            TREE_CODE(decl_node) == VAR_DECL &&
            (!TREE_CODE(TREE_TYPE(decl_node)) == REFERENCE_TYPE &&
             !TYPE_NEEDS_CONSTRUCTING (TREE_TYPE (decl_node))) &&
            TYPE_READONLY(decl_node))
          Set_ST_is_const_var (st);

	/* [CM] also mark readonly linkonce const (fix bug #28227) 
	   See also osprey/gccfe/gnu/varasm.c/categorize_decl_for_section()
	 */
	if (sclass == SCLASS_DGLOBAL && 
	    DECL_ONE_ONLY (decl_node) && 
	    TYPE_READONLY(decl_node) &&
	    !TREE_SIDE_EFFECTS (decl_node) &&
	    TREE_CONSTANT (DECL_INITIAL (decl_node)))
	  Set_ST_is_const_var (st);

	// [CL] handle used attribute
	if (sclass != SCLASS_AUTO && 
            TREE_CODE(decl_node) == VAR_DECL &&
	    DECL_IS_USED(decl_node)) {
	  Set_ST_is_used(st);
	}
#endif
#ifdef TARG_ST 
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

#ifdef KEY
  set_DECL_ST(decl_node, st);
#else
  DECL_ST(decl_node) = st;
#endif

#ifdef TARG_ST
  /* (cbr) allocate VLA */
  if (TREE_CODE(decl_node) == VAR_DECL) {
    tree type_tree = TREE_TYPE (decl_node);
    if (TYPE_SIZE(type_tree) &&
        TREE_CODE(TYPE_SIZE(type_tree)) != INTEGER_CST && TREE_CODE(type_tree) == ARRAY_TYPE) {
      st = WFE_Alloca_ST (decl_node);
    }
  }
#endif

  if (TREE_CODE(decl_node) == VAR_DECL &&
      DECL_CONTEXT(decl_node)	       &&
      TREE_CODE(DECL_CONTEXT(decl_node)) == RECORD_TYPE)
	Get_TY(DECL_CONTEXT(decl_node));

  if (Enable_WFE_DFE) {
#ifdef TARG_ST
    /* (cbr) ? */
    if (TREE_CODE(decl_node) == FUNCTION_DECL &&
#else
    if (TREE_CODE(decl_node) == VAR_DECL &&
#endif
        level == GLOBAL_SYMTAB &&
        !DECL_EXTERNAL (decl_node) &&
        DECL_INITIAL (decl_node)) {
      Push_Deferred_Function (decl_node);
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
      if (!strcmp (TREE_STRING_POINTER (attr), "da"))
	kind = ST_MEMORY_DA;
      else if (!strcmp (TREE_STRING_POINTER (attr), "sda"))
	kind = ST_MEMORY_SDA;
      else if (!strcmp (TREE_STRING_POINTER (attr), "tda"))
	kind = ST_MEMORY_TDA;
      else if (!strcmp (TREE_STRING_POINTER (attr), "none"))
	kind = ST_MEMORY_NONE;
      else
	FmtAssert (FALSE, ("Malformed tls_model attribute"));
    } 
    Set_ST_memory_space (*st, kind);
#endif

  if (DECL_WEAK      (decl_node) &&
      !DECL_EXTERNAL (decl_node)) {
    Set_ST_is_weak_symbol (st);
  }

#ifdef KEY
  // Make all symbols referenced in cleanup code and try handler code weak.
  // This is to work around an implementation issue where kg++fe always emit
  // the code in a cleanup or try handler, regardless of whether such code is
  // emitted by g++.  If the code calls a function foo that isn't emitted by
  // g++ into the RTL, then foo won't be tagged as needed, and the WHIRL for
  // foo won't be genterated.  This leads to undefined symbol at link-time.
  //
  // The correct solution is to mimick g++ and generate the cleanup/handler
  // code only if the region can generate an exception.  g++ does this in
  // except.c by checking for "(flag_non_call_exceptions ||
  // region->may_contain_throw)".  This checking isn't done in kg++fe because
  // the equivalent of "region->may_contain_throw" isn't (yet) implemented.
  // For now, work around the problem by making all symbols refereced in
  // cleanups and try handlers as weak.
  if (make_symbols_weak) {
    if (eclass != EXPORT_LOCAL &&
	eclass != EXPORT_LOCAL_INTERNAL &&
	// Don't make symbol weak if it is defined in current file.  Workaround
	// for SLES 8 linker.  Bug 3758.
        WEAK_WORKAROUND(st) != WEAK_WORKAROUND_dont_make_weak &&
        // Don't make builtin functions weak.  Bug 9534.
        !(TREE_CODE(decl_node) == FUNCTION_DECL &&
          DECL_BUILT_IN(decl_node))) {
      Set_ST_is_weak_symbol (st);
      WEAK_WORKAROUND(st) = WEAK_WORKAROUND_made_weak;
    }
  }
  // See comment above about guard variables.
  else if (guard_var) {
    Set_ST_is_weak_symbol (st);
    Set_ST_init_value_zero (st);
    Set_ST_is_initialized (st);
  }
#endif
#if defined (TARG_ST) && (GNU_FRONT_END==33)
    /* linkage status is not yet known
    if (DECL_DEFER_OUTPUT (decl_node) &&
        DECL_INLINE (decl_node) && TREE_PUBLIC (decl_node)) {
      DECL_COMDAT (decl_node) = 1;
      DECL_ONE_ONLY (decl_node) = 1;
    }
 */

    if (DECL_COMDAT (decl_node)) {
      Set_ST_is_comdat (st);
    }

    /* (cbr) merge into linkonce */
    if (DECL_ONE_ONLY (decl_node)) {
      /* [CM] The `reloc' computation from gccfe/gnu/varasm.c::assemble_variable */
      int reloc = 0 ;
      if (DECL_INITIAL (decl_node))
	reloc = categorize_addressed_constants (DECL_INITIAL (decl_node));
      default_unique_section (decl_node, reloc);
      if (sclass == SCLASS_COMMON)
        Set_ST_sclass (st, SCLASS_UGLOBAL);
    }
#endif

  if (DECL_SECTION_NAME (decl_node)) {
#if 0
    DevWarn ("section %s specified for %s",
	     TREE_STRING_POINTER (DECL_SECTION_NAME (decl_node)),
	     ST_name (st));
#endif
    if (TREE_CODE (decl_node) == FUNCTION_DECL)
      level = GLOBAL_SYMTAB;
    ST_ATTR_IDX st_attr_idx;
    ST_ATTR&    st_attr = New_ST_ATTR (level, st_attr_idx);
    ST_ATTR_Init (st_attr, ST_st_idx (st), ST_ATTR_SECTION_NAME,
		  Save_Str (TREE_STRING_POINTER (DECL_SECTION_NAME (decl_node))));
    Set_ST_has_named_section (st);
  }
/*
  if (DECL_SYSCALL_LINKAGE (decl_node)) {
	Set_PU_has_syscall_linkage (Pu_Table [ST_pu(st)]);
  }
*/

#ifdef TARG_ST
  /* (cbr) const/pure attributes */
  /* Warning: 
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
#ifdef KEY
    // Bug 559
    if (ST_sclass(st) != SCLASS_EXTERN) {
      // Add DSTs for all types seen so far.
      add_deferred_DST_types();

      DST_INFO_IDX dst = Create_DST_decl_For_Tree(decl_node,st);
      DECL_DST_IDX(decl_node) = dst;
    }
#else
    DST_INFO_IDX dst = Create_DST_decl_For_Tree(decl_node,st);
    DECL_DST_IDX(decl_node) = dst;
#endif
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
// From gccfe/gny/varasm.c, inspired by output_addressed_constants
// simplified to compute only `reloc'
static int
categorize_addressed_constants(tree exp)
{
  int reloc = 0, reloc2;
  tree tem;

  /* Give the front-end a chance to convert VALUE to something that
     looks more like a constant to the back-end.  */
  exp = (*lang_hooks.expand_constant) (exp);

  switch (TREE_CODE (exp))
    {
    case ADDR_EXPR:
    case FDESC_EXPR:
      /* Go inside any operations that get_inner_reference can handle and see
	 if what's inside is a constant: no need to do anything here for
	 addresses of variables or functions.  */
      for (tem = TREE_OPERAND (exp, 0); handled_component_p (tem);
	   tem = TREE_OPERAND (tem, 0))
	;

      if (TREE_PUBLIC (tem))
	reloc |= 2;
      else
	reloc |= 1;
      break;

    case PLUS_EXPR:
      reloc = categorize_addressed_constants (TREE_OPERAND (exp, 0));
      reloc |= categorize_addressed_constants (TREE_OPERAND (exp, 1));
      break;

    case MINUS_EXPR:
      reloc = categorize_addressed_constants (TREE_OPERAND (exp, 0));
      reloc2 = categorize_addressed_constants (TREE_OPERAND (exp, 1));
      /* The difference of two local labels is computable at link time.  */
      if (reloc == 1 && reloc2 == 1)
	reloc = 0;
      else
	reloc |= reloc2;
      break;

    case NOP_EXPR:
    case CONVERT_EXPR:
    case NON_LVALUE_EXPR:
      reloc = categorize_addressed_constants (TREE_OPERAND (exp, 0));
      break;

    case CONSTRUCTOR:
      for (tem = CONSTRUCTOR_ELTS (exp); tem; tem = TREE_CHAIN (tem))
	if (TREE_VALUE (tem) != 0)
	  reloc |= categorize_addressed_constants (TREE_VALUE (tem));

      break;

    default:
      break;
    }
  return reloc;
}
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
  
  tree attr = lookup_attribute ("visibility", DECL_ATTRIBUTES (decl_node));

  if (attr != NULL) {
    const char *which = TREE_STRING_POINTER (TREE_VALUE(TREE_VALUE(attr)));
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

  /* (cbr) temporary variables don't have visibility attribute */
  if (!DECL_ASSEMBLER_NAME_SET_P (decl_node))
    return sv;

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

#ifndef EXTRA_WORD_IN_TREE_NODES

#include <ext/hash_map>

namespace {

  using __gnu_cxx::hash_map;

  struct ptrhash {
    size_t operator()(void* p) const { return reinterpret_cast<size_t>(p); }
  };

  hash_map<tree, TY_IDX,     ptrhash>     ty_idx_map;
  hash_map<tree, ST*,        ptrhash>     st_map;
  hash_map<tree, SYMTAB_IDX, ptrhash>     symtab_idx_map;
  hash_map<tree, LABEL_IDX,  ptrhash>     label_idx_map;
  hash_map<tree, ST*,        ptrhash>     string_st_map;
  hash_map<tree, BOOL,       ptrhash>     bool_map;
  hash_map<tree, INT32,      ptrhash>     field_id_map;
  hash_map<tree, INT32,	  ptrhash>     type_field_ids_used_map;
  hash_map<tree, INT32,      ptrhash>     scope_number_map;
  hash_map<tree, tree,       ptrhash>     label_scope_map;
  hash_map<tree, DST_INFO_IDX,ptrhash>    decl_idx_map; 
  hash_map<tree, DST_INFO_IDX,ptrhash>    decl_field_idx_map; 
  hash_map<tree, DST_INFO_IDX,ptrhash>    decl_specification_idx_map; 
  hash_map<tree, DST_INFO_IDX,ptrhash>    type_idx_map;
  hash_map<tree, LABEL_IDX,  ptrhash>     handler_label_map;
  hash_map<tree, DST_INFO_IDX,ptrhash>    abstract_root_map;
#ifdef KEY
  // Map PU to the PU-specific st_map.
  hash_map<PU*, hash_map<tree, ST*, ptrhash>*, ptrhash>     pu_map;
  // TRUE if ST is a decl that is being/already been expanded.
  hash_map<tree, BOOL,        ptrhash>     expanded_decl_map;
  hash_map<tree, tree,  ptrhash>	   parent_scope_map;
  // Record whether a symbol referenced in a cleanup should be marked weak as a
  // workaround to the fact that kg++fe may emit cleanups that g++ won't emit
  // because g++ knows that are not needed.  The linker will complain if these
  // symbols are not defined.
  hash_map<ST*, INT32,        ptrhash>     weak_workaround_map;
#endif
}

TY_IDX& TYPE_TY_IDX(tree t)         { return ty_idx_map[t]; }

#ifdef KEY
BOOL& expanded_decl(tree t) {
  FmtAssert (DECL_CHECK(t), ("func_expanded: not a decl"));
  return expanded_decl_map[t];
}

// Put ST in a map based on the tree node T and the current PU.
void
set_DECL_ST(tree t, ST* st) {

  // Find the tree node to use as index into st_map.
  tree t_index;

  if (
#ifndef TARG_ST
      /* (cbr) also for functions */
      TREE_CODE(t) == VAR_DECL			     &&
#endif
      (DECL_CONTEXT(t) == 0 || 
       TREE_CODE(DECL_CONTEXT(t)) == NAMESPACE_DECL) &&
     DECL_NAME (t) && DECL_ASSEMBLER_NAME(t))
    t_index = DECL_ASSEMBLER_NAME(t);
  else
    t_index = t;

  // If ST is 1, then the caller only wants to pretend that there is a symbol
  // for T.  Later on, the caller will reset the ST to NULL and assign a real
  // symbol to T.
  if (st == (ST *) 1) {
    st_map[t_index] = st;
    return;
  }

  // If ST is a symbol that should be shared across functions, then put ST in
  // the st_map, which maps T directly to ST.  Otherwise, put ST in the
  // PU-specific st_map.
  //
  // It is observed that g++ uses the same tree for different functions, such
  // as inline functions.  As a result, we cannot attach PU-specific ST's 
  // directly to the tree nodes.
  //
  // If Current_scope is 0, then the symbol table has not been initialized, and
  // we are being called by WFE_Add_Weak to handle a weak symbol.  In that
  // case, use the non-PU-specific st_map.
  if (Current_scope != 0 &&
      (TREE_CODE(t) == PARM_DECL ||
       (TREE_CODE(t) == VAR_DECL &&
        (ST_sclass(st) == SCLASS_AUTO ||
         (! pstatic_as_global &&
	  ST_sclass(st) == SCLASS_PSTATIC))))) {
    // ST is PU-specific.  Use pu_map[pu] to get the PU-specific st_map, then
    // use st_map[t] to get the ST for the tree node t.
    //
    // We can access pu_map[pu] only if Scope_tab[Current_scope].st is valid
    // because we need to get the current PU, but Get_Current_PU requires a
    // valid Scope_tab[Current_scope].st.  If Scope_tab[Current_scope].st is
    // not set, then this means the caller is trying to create the ST for the
    // function symbol.
    if (Scope_tab[Current_scope].st != NULL) {
      // ok to call Get_Current_PU.
      PU *pu = &Get_Current_PU();
      hash_map<PU*, hash_map<tree, ST*, ptrhash>*, ptrhash>::iterator it =
	pu_map.find(pu);
      if (it == pu_map.end()) {
	// Create new PU-specific map.
	pu_map[pu] = new hash_map<tree, ST*, ptrhash>;
      }
      // Put the ST in the PU-specific st_map.
      (*(pu_map[pu]))[t_index] = st;
    }
  } else {
#ifdef Is_True_On
    if (st_map[t_index]) {
      // The st_map is already set.  This is ok only for weak ST.
      FmtAssert (ST_is_weak_symbol(st_map[t_index]),
		 ("set_DECL_ST: st_map already set"));
    }
#endif
    // Put the ST in the non-PU-specific st_map.
    st_map[t_index] = st;
  }
}

// Get ST associated with the tree node T.
ST*&
get_DECL_ST(tree t) {
  static ST *null_ST = (ST *) NULL;

  // Find the tree node to use as index into st_map.
  tree t_index;

  if (
#ifndef TARG_ST
      /* (cbr) also for functions */
      TREE_CODE(t) == VAR_DECL			     &&
#endif
      (DECL_CONTEXT(t) == 0 || 
       TREE_CODE(DECL_CONTEXT(t)) == NAMESPACE_DECL) &&
     DECL_NAME (t) && DECL_ASSEMBLER_NAME(t))
    t_index = DECL_ASSEMBLER_NAME(t);
  else
    t_index = t;

  // If Current_scope is 0, then the symbol table has not been initialized, and
  // we are being called by WFE_Add_Weak to handle a weak symbol.  Use the
  // non-PU-specific st_map.
  if (Current_scope == 0)
    return st_map[t_index];

  // See if the ST is in the non-PU-specific st_map.
  if (st_map[t_index]) {
    return st_map[t_index];
  }

  // The ST is not in the non-PU-specific map.  Look in the PU-specific map.

  // If Scope_tab[Current_scope].st is NULL, then the function ST has not
  // been set yet, and there is no PU-specific map.
  if (Scope_tab[Current_scope].st == NULL)
    return null_ST;

  // See if there is a PU-specific map.
  PU *pu = &Get_Current_PU();	// needs Scope_tab[Current_scope].st
  hash_map<PU*, hash_map<tree, ST*, ptrhash>*, ptrhash>::iterator pu_map_it =
    pu_map.find(pu);
  if (pu_map_it == pu_map.end())
    return null_ST;

  // There is a PU-specific map.  Get the ST from the map.
  hash_map<tree, ST*, ptrhash> *st_map = pu_map[pu];
  return (*st_map)[t_index];
}
INT32& WEAK_WORKAROUND(ST *st)         { return weak_workaround_map[st]; }
#else
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

SYMTAB_IDX& DECL_SYMTAB_IDX(tree t) { return symtab_idx_map[t]; }
LABEL_IDX& DECL_LABEL_IDX(tree t)   { return label_idx_map[t]; }
ST*& TREE_STRING_ST(tree t)         { return string_st_map[t]; }
BOOL& DECL_LABEL_DEFINED(tree t)    { return bool_map[t]; }
INT32& DECL_FIELD_ID(tree t)        { return field_id_map[t]; }
#ifdef TARG_ST
void SET_TYPE_FIELD_IDS_USED(tree t, INT32 v) {
  //  fprintf(stderr, "type_field_ids_used_map[%p] = %d\n", (void *)t, (int)v);
  type_field_ids_used_map[t] = v;
}
// INT32 & TYPE_FIELD_IDS_USED(tree t) { return type_field_ids_used_map[t]; }
BOOL TYPE_FIELD_IDS_USED_KNOWN(tree t) {
  hash_map<tree, INT32, ptrhash>::iterator it = type_field_ids_used_map.find(t);
  return it != type_field_ids_used_map.end();
}
INT32 GET_TYPE_FIELD_IDS_USED(tree t) {
  INT32 v = -1;
  hash_map<tree, INT32, ptrhash>::iterator it = type_field_ids_used_map.find(t);
  if (it == type_field_ids_used_map.end()) {
    FmtAssert (TREE_CODE(t) != RECORD_TYPE && TREE_CODE(t) != UNION_TYPE,
	       ("TYPE_FIELD_IDS_USED not set"));
    v = 0;
    type_field_ids_used_map[t] = v;
  } else {
    v = it->second;
  }
  return v;
}
#else
INT32 & TYPE_FIELD_IDS_USED(tree t) { return type_field_ids_used_map[t]; }
#endif
INT32 & SCOPE_NUMBER(tree t)        { return scope_number_map[t]; }
tree & LABEL_SCOPE(tree t)	    { return label_scope_map[t]; }
#ifdef KEY
tree & PARENT_SCOPE(tree t)	    { return parent_scope_map[t]; }
#endif

// This is for normal declarations.

// We do not know if the DST entry is filled in.
// So check and ensure a real entry exists.

DST_INFO_IDX & DECL_DST_IDX(tree t) 
{ 
	hash_map<tree, DST_INFO_IDX,ptrhash>::iterator it =
		decl_idx_map.find(t);
	if(it == decl_idx_map.end()) {
		// substitute for lack of default constructor
		DST_INFO_IDX dsti = DST_INVALID_IDX;
		decl_idx_map[t] = dsti;
	}
	return decl_idx_map[t]; 
}
// This is for static class members and member functions.
// We need a distinct DST record for a single ST.
// Note that only the main record actually need be linked
// to ST as only that one gets an address/location.

// We do not know if the DST entry is filled in.
// So check and ensure a real entry exists.
DST_INFO_IDX & DECL_DST_SPECIFICATION_IDX(tree t) 
{ 
	hash_map<tree, DST_INFO_IDX,ptrhash>::iterator it =
		decl_specification_idx_map.find(t);
	if(it == decl_specification_idx_map.end()) {
		// substitute for lack of default constructor
		DST_INFO_IDX dsti = DST_INVALID_IDX;
		decl_specification_idx_map[t] = dsti;
	}
	return decl_specification_idx_map[t]; 
}

// This is for static class members and member functions.
// We need a distinct DST record for a single ST.
// Note that only the main record actually need be linked
// to ST as only that one gets an address/location.

// We do not know if the DST entry is filled in.
// So check and ensure a real entry exists.
DST_INFO_IDX & DECL_DST_FIELD_IDX(tree t) 
{ 
	hash_map<tree, DST_INFO_IDX,ptrhash>::iterator it =
		decl_field_idx_map.find(t);
	if(it == decl_idx_map.end()) {
		// substitute for lack of default constructor
		DST_INFO_IDX dsti = DST_INVALID_IDX;
		decl_field_idx_map[t] = dsti;
	}
	return decl_field_idx_map[t]; 
}

// We do not know if the DST entry is filled in.
// So check and ensure a real entry exists.
DST_INFO_IDX & TYPE_DST_IDX(tree t) 
{
	hash_map<tree, DST_INFO_IDX,ptrhash>::iterator it =
		type_idx_map.find(t);
	if(it == type_idx_map.end()) {
		// substitute for lack of default constructor
		DST_INFO_IDX dsti = DST_INVALID_IDX;
		type_idx_map[t] = dsti;
	}
	return type_idx_map[t]; 
}

// We do not know if the DST entry is filled in.
// So check and ensure a real entry exists.
DST_INFO_IDX & DECL_DST_ABSTRACT_ROOT_IDX(tree t) 
{
	hash_map<tree, DST_INFO_IDX,ptrhash>::iterator it =
		abstract_root_map.find(t);
	if(it == abstract_root_map.end()) {
		// substitute for lack of default constructor
		DST_INFO_IDX dsti = DST_INVALID_IDX;
		abstract_root_map[t] = dsti;
	}
	return abstract_root_map[t]; 
}


LABEL_IDX& HANDLER_LABEL(tree t)    { return handler_label_map[t]; }


#endif


