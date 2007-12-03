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

#include "wfe_dst.h"

extern TY_IDX Create_TY_For_Tree (tree, TY_IDX idx = TY_IDX_ZERO);
extern "C" ST* Create_ST_For_Tree (tree);


#ifdef EXTRA_WORD_IN_TREE_NODES

/*
 * Hacked macro defintions to get things to compile.
 * DECL_NAME was chosen completely arbitrarily.
 */

#define TYPE_TY_IDX(tree) ((TY_IDX) DECL_NAME(tree))
#define DECL_ST(tree)     ((ST *)   DECL_NAME(tree))
#define DECL_SYMTAB_IDX(tree) ((SYMTAB_IDX) DECL_NAME(tree))
#define DECL_LABEL_IDX(tree) ((LABEL_IDX) DECL_NAME(tree))
#define TREE_STRING_ST(tree) ((ST *) DECL_NAME(tree))
#define DECL_LABEL_DEFINED(tree) ((BOOL) DECL_NAME(tree))
#define DECL_FIELD_ID(tree) ((INT32) DECL_NAME(tree))
#define DECL_DST_IDX(tree) ((INT32) DECL_NAME(tree))
#define DECL_DST_FIELD_IDX(tree) ((INT32) DECL_NAME(tree))
#define DECL_DST_ABSTRACT_ROOT_IDX(tree) ((INT32) DECL_NAME(tree))
#define DECL_DST_SPECIFICATION_IDX(tree) ((INT32) DECL_NAME(tree))
#define TYPE_DST_IDX(tree) ((INT32) DECL_NAME(tree))

#else /* EXTRA_WORD_IN_TREE_NODES */

//TB: Add undef of these dumy declaration to get think to compile with
//common tree.h (remove all defined(FRONT_END_C) in tree.h)
#undef TYPE_TY_IDX
TY_IDX& TYPE_TY_IDX(tree);
#undef DECL_ST
ST*& DECL_ST(tree);
#undef DECL_SYMTAB_IDX
SYMTAB_IDX& DECL_SYMTAB_IDX(tree);
#undef DECL_LABEL_IDX
LABEL_IDX& DECL_LABEL_IDX(tree);
#undef TREE_STRING_ST
ST*& TREE_STRING_ST(tree);
#undef DECL_LABEL_DEFINED
BOOL& DECL_LABEL_DEFINED(tree);
#undef DECL_FIELD_ID
INT32& DECL_FIELD_ID(tree);
#undef TYPE_FIELD_IDS_USED
INT32& TYPE_FIELD_IDS_USED(tree);
#undef SCOPE_NUMBER
INT32& SCOPE_NUMBER(tree);
#undef LABEL_SCOPE
tree& LABEL_SCOPE(tree);
#undef DECL_DST_IDX
DST_INFO_IDX& DECL_DST_IDX(tree);
#undef DECL_DST_FIELD_IDX
DST_INFO_IDX& DECL_DST_FIELD_IDX(tree);
#undef TYPE_DST_IDX
DST_INFO_IDX& TYPE_DST_IDX(tree);
#undef DECL_DST_SPECIFICATION_IDX
DST_INFO_IDX& DECL_DST_SPECIFICATION_IDX(tree);
#undef DECL_DST_ABSTRACT_ROOT_IDX
DST_INFO_IDX& DECL_DST_ABSTRACT_ROOT_IDX(tree);
#undef HANDLER_LABEL
LABEL_IDX& HANDLER_LABEL(tree);
#ifdef KEY
#undef PARENT_SCOPE
tree& PARENT_SCOPE(tree);
#endif

#endif /* EXTRA_WORD_IN_TREE_NODES */

/* 
 * either return a previously created TY_IDX associated with a type,
 * or create a new one.
 */
inline TY_IDX
Get_TY (tree type_tree)
{
	TY_IDX idx = TYPE_TY_IDX(type_tree);
        if (idx > 1) {
	    if (TREE_CODE(type_tree) == RECORD_TYPE ||
	        TREE_CODE(type_tree) == UNION_TYPE) {
	      FLD_HANDLE elt_fld = TY_fld(idx);
#ifdef TARG_ST
	      /* (cbr) fields are not handled even if type is beeing processed
		 as a base_type or member */
	      if (elt_fld.Is_Null() && !TREE_ASM_WRITTEN(type_tree))
#else
	      if (elt_fld.Is_Null() && TYPE_METHODS(type_tree) == NULL) 
#endif
		return Create_TY_For_Tree (type_tree, idx); // forward declared
	      else return idx;
	    }
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
		    !DECL_EXTERNAL(decl_tree)        &&
		    !DECL_INITIAL(decl_tree))
			Set_ST_sclass (st, SCLASS_UGLOBAL);
#ifdef TARG_ST
		/* (cbr) adjust attribute in case the symbol was forward declared */
		if (DECL_SECTION_NAME (decl_tree) &&
		    !ST_has_named_section (st)) { 
		  SYMTAB_IDX level = ST_level(st);
		  DevWarn("Change section for symbol %s",
			  TREE_STRING_POINTER (DECL_SECTION_NAME (decl_tree)));
		  ST_ATTR_IDX st_attr_idx;
		  ST_ATTR&    st_attr = New_ST_ATTR (level, st_attr_idx);
		  ST_ATTR_Init (st_attr, ST_st_idx (st), ST_ATTR_SECTION_NAME,
				Save_Str (TREE_STRING_POINTER (DECL_SECTION_NAME (decl_tree))));
		  Set_ST_has_named_section (st);
		}
#endif
                return st;
        }
	return Create_ST_For_Tree (decl_tree);
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


bool is_empty_base_class (tree type_tree);
#ifdef TARG_ST
/* (cbr) real_field can't be static */
tree next_real_or_virtual_field (tree type_tree, tree field, bool &real_field);
#else
tree next_real_or_virtual_field (tree type_tree, tree field);
#endif
size_t Roundup (size_t offset, int alignment);
size_t Type_Size_Without_Vbases (tree type_tree);

#endif
