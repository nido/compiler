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

#ifndef tree_symtab_INCLUDED
#define tree_symtab_INCLUDED

#if defined(TARG_ST) && defined(__cplusplus)
// (cbr) only used in translator.
#include <cmplrs/rcodes.h>
#endif
#include "W_values.h" // for BITSPERBYTE

#if defined(TARG_ST) && !defined(__cplusplus)
// [CL] Create_ST_For_Tree() is used from gnu/stmt.c, so this
// prototype must be available in C language. (DDTS MBTst23502)
// Don't forget to update both C and C++ prototypes if needed
extern ST* Create_ST_For_Tree (tree);
#else

extern TY_IDX Create_TY_For_Tree (tree, TY_IDX idx = TY_IDX_ZERO);
extern "C" ST* Create_ST_For_Tree (tree);

#ifdef TARG_ST
// (cbr) 
extern void set_variable_attributes(tree decl_node);
ST*& DECL_ST(tree);
extern "C" void Update_TY (tree);
#endif

/* 
 * either return a previously created TY_IDX associated with a type,
 * or create a new one.
 */
inline TY_IDX
Get_TY (tree type_tree)
{
	if (TREE_CODE(type_tree) == ERROR_MARK)
	    exit (RC_USER_ERROR);
	TY_IDX idx = TYPE_TY_IDX(type_tree);
        if (idx != 0) {
	    if (TREE_CODE(type_tree) == RECORD_TYPE ||
	        TREE_CODE(type_tree) == UNION_TYPE) {
	      FLD_HANDLE elt_fld = TY_fld(idx);
	      if (elt_fld.Is_Null()) 
		return Create_TY_For_Tree (type_tree, idx); // forward declared
	      else return idx;
	    }
#ifdef TARG_ST
	    /* (cbr) couldn't guess its size the first time.(pro-release-1-8-0-B/8) */
	    else if (TREE_CODE(type_tree) == ARRAY_TYPE) {
	      if (!TY_size(idx))
		return Create_TY_For_Tree (type_tree, idx); // forward declared
	    }
#endif
	    else return idx;
        }
	return Create_TY_For_Tree (type_tree, TY_IDX_ZERO);
}

/*
 * either return a previously created ST associated with a
 * var-decl/parm-decl/function_decl, or create a new one.
 */
inline ST *
Get_ST (tree decl_tree)
{
	ST *st = DECL_ST(decl_tree);
        if (st != NULL) {
		if (TREE_CODE(decl_tree) == VAR_DECL &&
		    ST_sclass(st) == SCLASS_EXTERN   &&
		    !ST_is_weak_symbol(st)           &&
#ifdef TARG_ST
		    // [CL] handle aliases: keep SCLASS_EXTERN
		    !Has_Base_Block(st) &&
		    !ST_emit_symbol(st) &&
#endif
                    !DECL_EXTERNAL(decl_tree)       
#ifndef TARG_ST
		    /* (cbr) (DDTSst22530) must adjust type size */
		    && !DECL_INITIAL(decl_tree)
#endif
		    ) {
#if defined (TARG_ST) && (GNU_FRONT_END==33)
		  /* (cbr) adjust type in case of forward declaration */

		  if (DECL_SECTION_NAME(decl_tree) ||
		      !DECL_COMMON(decl_tree))
		    Set_ST_sclass (st, SCLASS_UGLOBAL);
		  else
		    Set_ST_sclass (st, SCLASS_COMMON);

		  TY_IDX ty_idx = Get_TY(TREE_TYPE(decl_tree));
		  Set_ST_type (st, ty_idx);
#else
		  Set_ST_sclass (st, SCLASS_UGLOBAL);
#endif
		}
#ifdef TARG_ST
		/* (cbr) adjust attribute in case the symbol was
                   forward declared */
                set_variable_attributes(decl_tree);
#endif
        }
	else {
	  st = Create_ST_For_Tree (decl_tree);
#ifdef TARG_ST
          set_variable_attributes(decl_tree);
#endif
	}
	if ((CURRENT_SYMTAB > GLOBAL_SYMTAB + 1) &&
	    ((TREE_CODE(decl_tree) == VAR_DECL) ||
	     (TREE_CODE(decl_tree) == PARM_DECL)) &&
	    (ST_level(st) < CURRENT_SYMTAB) &&
	    (ST_level(st) > GLOBAL_SYMTAB)) {
		Set_ST_has_nested_ref (st);
		ST *base_st = st;
		while (base_st != ST_base (base_st)) {
			base_st = ST_base (base_st);
			Set_ST_has_nested_ref (base_st);
		}
	}
	return st;
}

#ifdef TARG_ST
/*
 * Get_Export_Class_For_Tree (tree decl_node, ST_CLASS storage, ST_SCLASS sclass)
 * 
 * Returns the export class for a tree accounting for
 * visibility attribute, visibility specification file and
 * default visibility setting.
 * This function should be called each time a new symbol is
 * created for setting its export class.
 */
extern ST_EXPORT Get_Export_Class_For_Tree (tree decl_node, ST_CLASS storage, ST_SCLASS sclass);
#endif

#endif /* __cplusplus */

#endif
