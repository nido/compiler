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


/* -*-Mode: c++;-*- (Tell emacs to use c++ mode) */

#include <dwarf.h>
#include <vector>
#include <algorithm>
// [HK]
#if __GNUC__ >=3
#include <ext/hash_map>
#else
#include <hash_map>
#endif //  __GNUC__ >=3
#ifdef TARG_ST
#include <list>
#endif

#include "namespace_trans.h"
#include "mempool.h"
#include "mempool_allocator.h"
#define USE_STANDARD_TYPES
#define USE_DST_INTERNALS
#include "dwarf_DST_mem.h"      // Basic definition of DST data structure.
#include "dwarf_DST.h"
#include "pu_info.h"
#include "ipc_dst_utils.h"
#include "ipc_file.h"           // For IP_FILE_HDR
#include "ipa_cg.h"             // For Get_Node_From_PU

#include "ipc_dst_merge.h"

// Merge DSTs.  The steps are
// ** Determine which DSTs, and which subprograms within the DSTs,
//    we're going to merge.  Within each DST, we need to merge the
//    subprograms that are explicitly mentioned in the pu list, and
//    the transitive closure of subprograms that they refer to.
//    The data structure we will use is a map from a DST_TYPE to
//    vector of DST_IDXs within that DST.  The vector is sorted by
//    DST_IDX.
// ** Merge all of the include directories, and merge the filenames.
//    To merge the filenames from a particular input DST, we need to
//    know the number of include dirs merged in all previous input DSTs.
//    (As an offset to the include directories' ordinal numbers.)
// ** Merge the compile unit nodes and the subprogram nodes.
//    For each input DST:
//      -- Copy its compile unit node (assumed to be only one) to the
//         merged DST.  Copy all of its subprogram nodes to the merged
//         DST, as children of the newly created subprogram nodes.  
//         The compile unit nodes in the merged DST form a singly
//         linked list, and the subprogram nodes underneath any particular
//         compile unit also form a singly linked list.
//      -- After entering all of the DST's subprogram nodes in the merged
//         DST, loop through the newly entered nodes to update cross-
//         references.
// ** Update the references to DST indices in the pu list.

// Typedefs for the data structures used in the merge process:
// a vector of pairs (old index, new index), and a map from DST_TYPE
// to such a vector.

typedef std::pair<DST_IDX, DST_IDX> idx_pair;
struct idx_pair_less {
  bool operator()(const idx_pair& x, const idx_pair& y) const {
    return x.first < y.first;
  }
};
typedef mempool_allocator<idx_pair> idx_pair_allocator;
typedef std::vector<idx_pair, idx_pair_allocator> idx_pair_vector;

typedef std::pair<const DST_TYPE, idx_pair_vector> dst_hash_value_type;
typedef mempool_allocator<dst_hash_value_type> dst_hash_allocator;
struct DST_TYPE_hash {
  size_t operator()(DST_TYPE p) const {
    return reinterpret_cast<size_t>(p);
  }
};

typedef __GNUEXT::hash_map<DST_TYPE, idx_pair_vector,
                      DST_TYPE_hash, std::equal_to<DST_TYPE>,
                      dst_hash_allocator>
        dst_hash_map;


#ifdef TARG_ST
// [CL] we also need to update the file references inside the DSTs and
// the WNs

// The data structures are similar to the above ones, except for the
// types of indexes, which are mUINT16 as in srcpos.h

typedef std::pair<mUINT16, mUINT16> idx_file_pair;
struct idx_file_pair_less {
  bool operator()(const idx_file_pair& x, const idx_file_pair& y) const {
    return x.first < y.first;
  }
};

typedef mempool_allocator<idx_file_pair> idx_file_pair_allocator;
typedef std::vector<idx_file_pair, idx_file_pair_allocator> idx_file_pair_vector;

typedef std::pair<const DST_TYPE, idx_file_pair_vector> dst_hash_file_type;
typedef mempool_allocator<dst_hash_file_type> dst_hash_file_allocator;

typedef __GNUEXT::hash_map<DST_TYPE, idx_file_pair_vector,
                      DST_TYPE_hash, std::equal_to<DST_TYPE>,
                      dst_hash_file_allocator>
        dst_hash_file_map;

dst_hash_file_map* M_files = NULL;
#endif

namespace {                     // Unnamed namespace: internal functions
                                // used in the DST merge.

// Initialize the map from DST_TYPE to vector.  For each pu,
// enter its DST in the map (if it's not there already) and
// enter the pu's DST_IDX in the appropriate vector.  This 
// function does not compute the transitive closure.

void initialize_dst_map(dst_hash_map& M, pu_info* pu) 
{
  Is_True(pu != 0, ("No pu"));

  while (pu) {
    IPA_NODE* cg_node = Get_Node_From_PU(pu);
    DST_TYPE dst = IP_FILE_HDR_dst(cg_node->File_Header());
    Is_True(dst != 0, ("No dst"));

    // This is necessary to prevent a memory leak: make sure that the
    // new vector is created with the correct mempool allocator.
    if (M.find(dst) == M.end())
      M.insert(std::make_pair(dst, idx_pair_vector(M.get_allocator())));
    
    if (PU_Info_pu_dst(pu) != DST_INVALID_IDX)
      M[dst].push_back(idx_pair(PU_Info_pu_dst(pu), DST_INVALID_IDX));

    if (PU_Info_child(pu))
      initialize_dst_map(M, PU_Info_child(pu));

    pu = PU_Info_next(pu);
  }
}

// Each vector in the map M contains a list of DST_IDX's that point to 
// subprogram nodes.  Those subprogram nodes may refer to other subprogram
// nodes that aren't yet on the list.  Add all such nodes (and the nodes
// that they in turn refer to, etc.) to the list.  At the end, every 
// vector in M will be sorted. 
void construct_transitive_closure(dst_hash_map& M) 
{
  for (dst_hash_map::iterator map_iter = M.begin();
       map_iter != M.end();
       ++map_iter) {

    DST_TYPE dst = map_iter->first;
    idx_pair_vector& V = map_iter->second;
    idx_pair_vector tmp(V.get_allocator());

    std::sort(V.begin(), V.end(), idx_pair_less());
    do {
      tmp.clear();              // Vector of indices that aren't yet in V
                                // but that should be.
      for (idx_pair_vector::const_iterator i = V.begin();
           i != V.end();
           ++i) {
        Is_True(i->first != DST_INVALID_IDX,
                ("DST index vector contains an invalid index"));
        DST_INFO* info = DST_get_info(dst, i->first);
        DST_SUBPROGRAM* subpr = DST_get_subprogram_attr(dst, info);

        if (DST_SUBPROGRAM_has_spec(info)) {
          DST_IDX spec = DST_SUBPROGRAM_spec(info, subpr);
          if (spec != DST_INVALID_IDX) {
            idx_pair spec_pair(spec, DST_INVALID_IDX);
            if (!std::binary_search(V.begin(), V.end(), spec_pair,
                                    idx_pair_less()))
              tmp.push_back(spec_pair);
          }
        }
        if (DST_SUBPROGRAM_has_origin(info)) {
          DST_IDX origin = DST_SUBPROGRAM_origin(info, subpr);
          if (origin != DST_INVALID_IDX) {
            idx_pair origin_pair(origin, DST_INVALID_IDX);
            if (!std::binary_search(V.begin(), V.end(), origin_pair,
                                    idx_pair_less()))
              tmp.push_back(origin_pair);
          }
        }
      }
      std::sort(tmp.begin(), tmp.end(), idx_pair_less());
      idx_pair_vector::size_type N = V.size();
      V.insert(V.end(), tmp.begin(), tmp.end());
      std::inplace_merge(V.begin(), V.begin() + N, V.end(),
                         idx_pair_less());
    } while (!tmp.empty());
  }
}

// Merge the include dirs and file names in all of M's DSTs into
// the output DST output.  This is slightly tricky because file name
// nodes refer to include directories by ordinal number, and the ordinal
// number changes in the merge.  We take care of that by copying the
// directories in the same order as in the original DSTs.  All we need,
// then, is a per-DST ordinal number offset.

void merge_directories_and_files(dst_hash_map& M, MEM_POOL* p,
#ifdef TARG_ST
// [CL] construct M_files map
                                 DST_TYPE output, dst_hash_file_map& M_files)
#else
                                 DST_TYPE output)
#endif
{
  Is_True(DST_get_include_dirs(output) == DST_INVALID_IDX,
          ("Output DST already has include dirs"));
  Is_True(DST_get_file_names(output) == DST_INVALID_IDX,
          ("Output DST already has file names"));
  
  UINT16 prev_include_dirs = 0;
  DST_IDX cur_include_dir = DST_INVALID_IDX;
  DST_IDX cur_filename = DST_INVALID_IDX;

#ifdef TARG_ST
  // [CL]
  UINT32 new_file_idx = 0;
#endif

  for (dst_hash_map::iterator i = M.begin(); i != M.end(); ++i) {
    DST_TYPE src = i->first;
    UINT16 include_dirs_this_dst = 0;

    // Merge in this DST's include dirs.
    DST_IDX old_dir = DST_get_include_dirs(src);
    while (old_dir != DST_INVALID_IDX) {
      ++include_dirs_this_dst;
      cur_include_dir =
        DST_copy_include_dir(src, output, p, cur_include_dir, old_dir);
      old_dir = DST_INCLUDE_DIR_next(DST_get_include_dir(src, old_dir));
    }

#ifdef TARG_ST
    // [CL] reset old_file_idx when changing compilation unit
    UINT32 old_file_idx = 0;
#endif
    // Merge in this DST's filenames.
    DST_IDX old_file = DST_get_file_names(src);
    while (old_file != DST_INVALID_IDX) {
      cur_filename = DST_copy_filename(src, output, p, cur_filename, old_file,
                                       prev_include_dirs);

#ifdef TARG_ST
      // [CL] construct the M_files map
      if (M_files.find(src) == M_files.end())
	M_files.insert(std::make_pair(src,
				      idx_file_pair_vector(M_files.get_allocator())));

      old_file_idx++;
      new_file_idx++;
      M_files[src].push_back(idx_file_pair(old_file_idx, new_file_idx));
#endif

      old_file = DST_INCLUDE_DIR_next(DST_get_file_name(src, old_file));
    }

    // Add the include dirs we've merged in to the offset
    prev_include_dirs += include_dirs_this_dst;
  }
}

#ifdef TARG_ST
// [CL] update file number DST info in all the relevant nodes
static idx_file_pair_vector* V_files;

typedef mempool_allocator<idx_file_pair_vector*> idx_file_pair_vector_allocator;
static std::list < idx_file_pair_vector*, idx_file_pair_vector_allocator > V_files_stack;

static INT32
DST_update_info(INT32        indentation, 
		DST_DW_tag   tag,
		DST_flag     flag,
		DST_ATTR_IDX iattr,
		DST_INFO_IDX iinfo)
{
  USRCPOS decl;

  switch (tag) {

  case DW_TAG_inlined_subroutine:
    decl = DST_INLINED_SUBROUTINE_decl(DST_ATTR_IDX_TO_PTR(iattr,
							   DST_INLINED_SUBROUTINE));
    break;

  case DW_TAG_subprogram:
    if (DST_IS_memdef(flag)) {
      decl = DST_SUBPROGRAM_memdef_decl(DST_ATTR_IDX_TO_PTR(iattr,
							    DST_SUBPROGRAM));
    } else if (DST_IS_declaration(flag)) {
      decl = DST_SUBPROGRAM_decl_decl(DST_ATTR_IDX_TO_PTR(iattr,
							  DST_SUBPROGRAM));
    } else {
      decl = DST_SUBPROGRAM_def_decl(DST_ATTR_IDX_TO_PTR(iattr,
							 DST_SUBPROGRAM));
    }
    break;

  case DW_TAG_variable:
    if (DST_IS_const(flag)) {
      decl = DST_VARIABLE_constant_decl(DST_ATTR_IDX_TO_PTR(iattr,
							    DST_VARIABLE));
    } else if (DST_IS_comm(flag)) {
      decl = DST_VARIABLE_comm_decl(DST_ATTR_IDX_TO_PTR(iattr,
							DST_VARIABLE));
    } else if (DST_IS_memdef(flag)) {
      decl = DST_VARIABLE_memdef_decl(DST_ATTR_IDX_TO_PTR(iattr,
							  DST_VARIABLE));
    } else if (DST_IS_declaration(flag)) {
      decl = DST_VARIABLE_decl_decl(DST_ATTR_IDX_TO_PTR(iattr,
							DST_VARIABLE));
    } else {
      decl = DST_VARIABLE_def_decl(DST_ATTR_IDX_TO_PTR(iattr,
						       DST_VARIABLE));
    }
    break;

  case DW_TAG_entry_point:
    decl = DST_ENTRY_POINT_decl(DST_ATTR_IDX_TO_PTR(iattr,
						    DST_ENTRY_POINT));
    break;

  case DW_TAG_common_inclusion:
    decl = DST_COMMON_INCL_decl(DST_ATTR_IDX_TO_PTR(iattr,
						    DST_COMMON_INCL));
    break;

  case DW_TAG_formal_parameter:
    decl = DST_FORMAL_PARAMETER_decl(DST_ATTR_IDX_TO_PTR(iattr,
						 DST_FORMAL_PARAMETER));
    break;

  case DW_TAG_unspecified_parameters:
    decl = DST_UNSPECIFIED_PARAMETERS_decl(DST_ATTR_IDX_TO_PTR(iattr,
						 DST_UNSPECIFIED_PARAMETERS));
    break;

  case DW_TAG_constant:
    if (DST_IS_declaration(flag)) {
      decl = DST_CONSTANT_decl_decl(DST_ATTR_IDX_TO_PTR(iattr,
						       DST_CONSTANT));
    } else {
      decl = DST_CONSTANT_def_decl(DST_ATTR_IDX_TO_PTR(iattr,
						       DST_CONSTANT));
    }
    break;

  case DW_TAG_typedef:
    decl = DST_TYPEDEF_decl(DST_ATTR_IDX_TO_PTR(iattr,
						DST_TYPEDEF));
    break;

  case DW_TAG_array_type:
    decl = DST_ARRAY_TYPE_decl(DST_ATTR_IDX_TO_PTR(iattr,
						   DST_ARRAY_TYPE));
    break;

  case DW_TAG_string_type:
    decl = DST_STRING_TYPE_decl(DST_ATTR_IDX_TO_PTR(iattr,
						    DST_STRING_TYPE));
    break;

  case DW_TAG_structure_type:
    decl = DST_STRUCTURE_TYPE_decl(DST_ATTR_IDX_TO_PTR(iattr,
						       DST_STRUCTURE_TYPE));
    break;

  case DW_TAG_union_type:
    decl = DST_UNION_TYPE_decl(DST_ATTR_IDX_TO_PTR(iattr,
						   DST_UNION_TYPE));
    break;

  case DW_TAG_class_type:
    decl = DST_CLASS_TYPE_decl(DST_ATTR_IDX_TO_PTR(iattr,
						   DST_CLASS_TYPE));
    break;

  case DW_TAG_member:
    decl = DST_MEMBER_decl(DST_ATTR_IDX_TO_PTR(iattr,
					       DST_MEMBER));
    break;

  case DW_TAG_inheritance:
    decl = DST_INHERITANCE_decl(DST_ATTR_IDX_TO_PTR(iattr,
						    DST_INHERITANCE));
    break;

  case DW_TAG_template_type_param:
    decl = DST_TEMPLATE_TYPE_PARAMETER_decl(DST_ATTR_IDX_TO_PTR(iattr,
						DST_TEMPLATE_TYPE_PARAMETER));
    break;

  case DW_TAG_template_value_param:
    decl = DST_TEMPLATE_VALUE_PARAMETER_decl(DST_ATTR_IDX_TO_PTR(iattr,
						DST_TEMPLATE_VALUE_PARAMETER));
    break;

  case DW_TAG_enumeration_type:
    decl = DST_ENUMERATION_TYPE_decl(DST_ATTR_IDX_TO_PTR(iattr,
						 DST_ENUMERATION_TYPE));
    break;

  case DW_TAG_enumerator:
    decl = DST_ENUMERATOR_decl(DST_ATTR_IDX_TO_PTR(iattr,
						   DST_ENUMERATOR));
    break;

  case DW_TAG_subroutine_type:
    decl = DST_SUBROUTINE_TYPE_decl(DST_ATTR_IDX_TO_PTR(iattr,
							DST_SUBROUTINE_TYPE));
    break;
  }

  // Compute new index
  UINT32 old_file_idx = USRCPOS_filenum(decl);
  idx_file_pair p(old_file_idx, 0);
  UINT32 new_file_idx  = std::lower_bound(V_files->begin(),
					  V_files->end(), p,
					  idx_file_pair_less())->second;

  USRCPOS_filenum(decl) = new_file_idx;

  // Write back new index
  switch(tag) {
  case DW_TAG_inlined_subroutine:
    DST_INLINED_SUBROUTINE_decl(DST_ATTR_IDX_TO_PTR(iattr,
						    DST_INLINED_SUBROUTINE)) =
      decl;
    break;

  case DW_TAG_subprogram:
    if (DST_IS_memdef(flag)) {
      DST_SUBPROGRAM_memdef_decl(DST_ATTR_IDX_TO_PTR(iattr, DST_SUBPROGRAM))
	= decl;
    } else if (DST_IS_declaration(flag)) {
      DST_SUBPROGRAM_decl_decl(DST_ATTR_IDX_TO_PTR(iattr, DST_SUBPROGRAM))
	= decl;
    } else {
      DST_SUBPROGRAM_def_decl(DST_ATTR_IDX_TO_PTR(iattr, DST_SUBPROGRAM))
	= decl;
    }
    break;

  case DW_TAG_variable:
    if (DST_IS_const(flag)) {
      DST_VARIABLE_constant_decl(DST_ATTR_IDX_TO_PTR(iattr, DST_VARIABLE))
	= decl;
    } else if (DST_IS_comm(flag)) {
      DST_VARIABLE_comm_decl(DST_ATTR_IDX_TO_PTR(iattr, DST_VARIABLE))
	= decl;
    } else if (DST_IS_memdef(flag)) {
      DST_VARIABLE_memdef_decl(DST_ATTR_IDX_TO_PTR(iattr, DST_VARIABLE))
	= decl;
    } else if (DST_IS_declaration(flag)) {
      DST_VARIABLE_decl_decl(DST_ATTR_IDX_TO_PTR(iattr, DST_VARIABLE))
	= decl;
    } else {
      DST_VARIABLE_def_decl(DST_ATTR_IDX_TO_PTR(iattr, DST_VARIABLE))
	= decl;
    }
    break;

  case DW_TAG_entry_point:
    DST_ENTRY_POINT_decl(DST_ATTR_IDX_TO_PTR(iattr, DST_ENTRY_POINT)) = decl;
    break;

  case DW_TAG_common_inclusion:
    DST_COMMON_INCL_decl(DST_ATTR_IDX_TO_PTR(iattr, DST_COMMON_INCL)) = decl;
    break;

  case DW_TAG_formal_parameter:
    DST_FORMAL_PARAMETER_decl(DST_ATTR_IDX_TO_PTR(iattr,
						  DST_FORMAL_PARAMETER))
      = decl;
    break;

  case DW_TAG_unspecified_parameters:
    DST_UNSPECIFIED_PARAMETERS_decl(DST_ATTR_IDX_TO_PTR(iattr,
						 DST_UNSPECIFIED_PARAMETERS))
      = decl;
    break;

  case DW_TAG_constant:
    if (DST_IS_declaration(flag)) {
      DST_CONSTANT_decl_decl(DST_ATTR_IDX_TO_PTR(iattr, DST_CONSTANT))
	= decl;
    } else {
      DST_CONSTANT_def_decl(DST_ATTR_IDX_TO_PTR(iattr, DST_CONSTANT))
	= decl;
    }
    break;

  case DW_TAG_typedef:
    DST_TYPEDEF_decl(DST_ATTR_IDX_TO_PTR(iattr, DST_TYPEDEF)) = decl;
    break;

  case DW_TAG_array_type:
    DST_ARRAY_TYPE_decl(DST_ATTR_IDX_TO_PTR(iattr, DST_ARRAY_TYPE)) = decl;
    break;

  case DW_TAG_string_type:
    DST_STRING_TYPE_decl(DST_ATTR_IDX_TO_PTR(iattr, DST_STRING_TYPE)) = decl;
    break;

  case DW_TAG_structure_type:
    DST_STRUCTURE_TYPE_decl(DST_ATTR_IDX_TO_PTR(iattr, DST_STRUCTURE_TYPE))
      = decl;
    break;

  case DW_TAG_union_type:
    DST_UNION_TYPE_decl(DST_ATTR_IDX_TO_PTR(iattr, DST_UNION_TYPE)) = decl;
    break;

  case DW_TAG_class_type:
    DST_CLASS_TYPE_decl(DST_ATTR_IDX_TO_PTR(iattr, DST_CLASS_TYPE)) = decl;
    break;

  case DW_TAG_member:
    DST_MEMBER_decl(DST_ATTR_IDX_TO_PTR(iattr, DST_MEMBER)) = decl;
    break;

  case DW_TAG_inheritance:
    DST_INHERITANCE_decl(DST_ATTR_IDX_TO_PTR(iattr, DST_INHERITANCE)) = decl;
    break;

  case DW_TAG_template_type_param:
    DST_TEMPLATE_TYPE_PARAMETER_decl(DST_ATTR_IDX_TO_PTR(iattr,
						DST_TEMPLATE_TYPE_PARAMETER))
      = decl;
    break;

  case DW_TAG_template_value_param:
    DST_TEMPLATE_VALUE_PARAMETER_decl(DST_ATTR_IDX_TO_PTR(iattr,
						DST_TEMPLATE_VALUE_PARAMETER))
      = decl;
    break;

  case DW_TAG_enumeration_type:
    DST_ENUMERATION_TYPE_decl(DST_ATTR_IDX_TO_PTR(iattr,
						  DST_ENUMERATION_TYPE))
      = decl;
    break;

  case DW_TAG_enumerator:
    DST_ENUMERATOR_decl(DST_ATTR_IDX_TO_PTR(iattr, DST_ENUMERATOR)) = decl;
    break;

  case DW_TAG_subroutine_type:
    DST_SUBROUTINE_TYPE_decl(DST_ATTR_IDX_TO_PTR(iattr, DST_SUBROUTINE_TYPE))
      = decl;
    break;
  }
  return 0;
}


// Browse all DSTs in M, and fix file references thanks to M_files
void merge_file_refs(dst_hash_map& M, MEM_POOL* p, DST_TYPE output,
		     dst_hash_file_map& M_files)
{
  DST_IDX cu_idx = DST_INVALID_IDX;

  DST_TYPE my_current_dst = Current_DST;
  Current_DST = output;

  for (dst_hash_map::iterator dst_iter = M.begin();
       dst_iter != M.end();
       ++dst_iter) {
    DST_TYPE src = dst_iter->first;
    idx_pair_vector& V = dst_iter->second;

    V_files = &M_files[src];
    std::sort(V_files->begin(), V_files->end(), idx_file_pair_less());

    idx_pair_vector::iterator i;

    for (i = V.begin(); i < V.end(); ++i) {
      if (!DST_IS_NULL(i->second)) {
	DST_preorder_visit(i->second, 0, &DST_update_info);
      }
    }
  }
  Current_DST = my_current_dst;
}

// Update filenum of wn and all its kids, thanks to the M_files map.
void update_wn(WN* wn, dst_hash_file_map& M_files)
{
  DST_TYPE dst;

  if (wn) {

    // [CL] When reaching an INLINE_BODY_END pragma, switch back to
    // the previous DST
    if ( (WN_opcode(wn) == OPC_PRAGMA) &&
	 (WN_pragma(wn) == WN_PRAGMA_INLINE_BODY_END) ) {
      V_files = V_files_stack.front();
      V_files_stack.pop_front();
    }

    // [CL] it seems that labels are inserted at some time after
    // inlining, and they sometimes belong to the caller, sometimes to
    // the callee. I don't know how to know which case we are
    // facing. It is safer to clear source line info on all labels
    if (WN_opcode(wn) == OPC_LABEL) {
      USRCPOS srcpos;
      USRCPOS_srcpos (srcpos) = 0;
      WN_Set_Linenum(wn, USRCPOS_srcpos(srcpos));
    }

    USRCPOS srcpos;
    USRCPOS_srcpos (srcpos) = WN_Get_Linenum(wn);
    if (USRCPOS_srcpos(srcpos) != 0) {
      if (V_files->empty()) {
	// [CL] If we have no info about this DST, it means that all
	// the functions from that comp.unit have been inlined at all
	// the call sites and the corresponding Whirl has been
	// deleted: there is no PU left in the original compilation
	// unit. In this case, clear the source information as we have
	// no means of recovering it
	USRCPOS_srcpos(srcpos) = 0;
      } else {
	UINT32 old_file_idx = USRCPOS_filenum(srcpos);
	idx_file_pair p(old_file_idx, 0);

	idx_file_pair_vector::iterator new_pair
	  = std::lower_bound(V_files->begin(),
			     V_files->end(), p,
			     idx_file_pair_less());

	if (new_pair == V_files->end()) {
	  FmtAssert(FALSE, ("Could not merge DST"));
	}
	UINT32 new_file_idx  = new_pair->second;

	USRCPOS_filenum(srcpos) = new_file_idx;
      }
      WN_Set_Linenum(wn, USRCPOS_srcpos(srcpos));
    }

    // [CL] When reaching an INLINE_BODY_START pragma, switch to the
    // new DST
    if ( (WN_opcode(wn) == OPC_PRAGMA) &&
	 (WN_pragma(wn) == WN_PRAGMA_INLINE_BODY_START) ) {
	dst = (DST_TYPE)WN_pragma_arg2(wn);
	V_files_stack.push_front(V_files);
	V_files = &M_files[dst];
    }

    switch(WN_opcode(wn)) {
    case OPC_BLOCK:
      // (cbr) fix crosses initialization build bug
      {
	WN* kid = WN_first(wn);
	while(kid) {
	  update_wn(kid, M_files);
	  kid = WN_next(kid);
	}
      }
      break;
    default:
      for (int i=0; i < WN_kid_count(wn); i++) {
	WN* kid = WN_kid(wn, i);
	update_wn(kid, M_files);
      }
    }
  }
}

// update file references of all WNs of all PUs starting for pu
void update_wns(dst_hash_map& M, dst_hash_file_map& M_files, pu_info* pu) 
{
  Is_True(pu != 0, ("No pu"));
  WN *func_entry;

  while (pu) {
    if (PU_Info_pu_dst(pu) != DST_INVALID_IDX) {
      IPA_NODE* cg_node = Get_Node_From_PU(pu);
      DST_TYPE dst = IP_FILE_HDR_dst(cg_node->File_Header());
      Is_True(dst != 0, ("No dst"));

      Is_True(M.find(dst) != M.end() && M.find(dst)->first == dst,
              ("DST not present in map"));

      V_files = &M_files[dst];

      V_files_stack.clear();

      func_entry = PU_Info_tree_ptr(pu);
      update_wn(func_entry, M_files);
    }

    if (PU_Info_child(pu))
      update_wns(M, M_files, PU_Info_child(pu));

    pu = PU_Info_next(pu);
  }
}
#endif

// Merge all of the compile units, and all of the mentioned subprogram nodes.
void merge_subprograms(dst_hash_map& M, MEM_POOL* p, DST_TYPE output)
{
    // [HK]
//   using std::binary_search;
//   using std::lower_bound;

  DST_IDX cu_idx = DST_INVALID_IDX;

  for (dst_hash_map::iterator dst_iter = M.begin();
       dst_iter != M.end();
       ++dst_iter) {
    DST_TYPE src = dst_iter->first;
    idx_pair_vector& V = dst_iter->second;

    DST_IDX src_cu_idx = DST_get_compile_unit(src);
    Is_True(DST_INFO_sibling(DST_get_info(src, src_cu_idx)) == DST_INVALID_IDX,
            ("Input dst should have only one compile unit"));

    // Copy the compile unit.
    cu_idx = DST_copy_compile_unit(src, output, p, cu_idx, src_cu_idx);
                             
    // Copy all of the mentioned subprogram units.
    DST_IDX subpr_idx = DST_INVALID_IDX;
    idx_pair_vector::iterator i;

    for (i = V.begin(); i < V.end(); ++i) {
      i->second = subpr_idx =
        DST_copy_subprogram(src, output, p, cu_idx, subpr_idx, i->first);
      Is_True(subpr_idx != DST_INVALID_IDX,
              ("Index of new subprogram unit is invalid"));
    }

    // Fix up cross-references.
    for (i = V.begin(); i < V.end(); ++i) {
      DST_INFO* src_sub_info = DST_get_info(src, i->first);
      DST_INFO* sub_info = DST_get_info(output, i->second);
      DST_SUBPROGRAM* src_sub = DST_get_subprogram_attr(src, src_sub_info);
      DST_SUBPROGRAM* sub = DST_get_subprogram_attr(output, sub_info);

      if (DST_SUBPROGRAM_has_spec(src_sub_info)) {
        DST_IDX src_spec = DST_SUBPROGRAM_spec(src_sub_info, src_sub);
        if (src_spec != DST_INVALID_IDX) {
          idx_pair p(src_spec, DST_INVALID_IDX);
          Is_True(std::binary_search(V.begin(), V.end(), p, idx_pair_less()),
                  ("Input spec index not found in index vector"));
          Is_True(std::lower_bound(V.begin(), V.end(), p,
                              idx_pair_less())->first == src_spec,
                  ("Inconsistent index vector"));
          Is_True(std::lower_bound(V.begin(), V.end(), p,
                              idx_pair_less())->second != DST_INVALID_IDX,
                  ("Input spec index maps to invalid index"));
          DST_SUBPROGRAM_spec(sub_info, sub) = 
            std::lower_bound(V.begin(), V.end(), p, idx_pair_less())->second;
        }
      }

      if (DST_SUBPROGRAM_has_origin(src_sub_info)) {
        DST_IDX src_origin = DST_SUBPROGRAM_origin(src_sub_info, src_sub);
        if (src_origin != DST_INVALID_IDX) {
          idx_pair p(src_origin, DST_INVALID_IDX);
          Is_True(std::binary_search(V.begin(), V.end(), p, idx_pair_less()),
                  ("Input origin index not found in index vector"));
          Is_True(std::lower_bound(V.begin(), V.end(), p,
                              idx_pair_less())->first == src_origin,
                  ("Inconsistent index vector"));
          Is_True(std::lower_bound(V.begin(), V.end(), p,
                              idx_pair_less())->second != DST_INVALID_IDX,
                  ("Input origin index maps to invalid index"));
          DST_SUBPROGRAM_origin(sub_info, sub) = 
            std::lower_bound(V.begin(), V.end(), p, idx_pair_less())->second;
        }
      }
    }
  }
}

void update_pu_dst_indices(dst_hash_map& M, pu_info* pu) 
{
  Is_True(pu != 0, ("No pu"));

  while (pu) {
    if (PU_Info_pu_dst(pu) != DST_INVALID_IDX) {
      IPA_NODE* cg_node = Get_Node_From_PU(pu);
      DST_TYPE dst = IP_FILE_HDR_dst(cg_node->File_Header());
      Is_True(dst != 0, ("No dst"));

      Is_True(M.find(dst) != M.end() && M.find(dst)->first == dst,
              ("DST not present in map"));

      idx_pair_vector& V = M.find(dst)->second;
      idx_pair p(PU_Info_pu_dst(pu), DST_INVALID_IDX);
      Is_True(std::binary_search(V.begin(), V.end(), p, idx_pair_less()),
              ("DST index not present in map"));
      Is_True(std::lower_bound(V.begin(), V.end(), p,
                               idx_pair_less())->first == PU_Info_pu_dst(pu),
              ("DST index not present in map"));
      Is_True(std::lower_bound(V.begin(), V.end(), p,
                               idx_pair_less())->second != DST_INVALID_IDX,
              ("DST index maps to incorrect value"));
      PU_Info_pu_dst(pu) = std::lower_bound(V.begin(), V.end(), p,
                                         idx_pair_less())->second;
    }

    if (PU_Info_child(pu))
      update_pu_dst_indices(M, PU_Info_child(pu));

    pu = PU_Info_next(pu);
  }
}

} // Close the unnamed namespace.

// Preconditions: pu_tree isn't null, output is empty.
DST_TYPE IPC_merge_DSTs(pu_info* pu_tree, MEM_POOL* p) 

{
  dst_hash_map M(100, DST_TYPE_hash(), std::equal_to<DST_TYPE>(),
                 dst_hash_allocator(p));

  // Get a list of the DSTs we are to merge, and the subprograms within
  // those DSTs.
  initialize_dst_map(M, pu_tree);
  construct_transitive_closure(M);

   // Create the output DST.
  DST_TYPE output = DST_create(p);

#ifdef TARG_ST
  // [CL]
  dst_hash_file_map M_files(100, DST_TYPE_hash(), std::equal_to<DST_TYPE>(),
		      dst_hash_file_allocator(p));
#endif

  // Merge the include directories and the file names.
#ifdef TARG_ST
  // [CL]
  merge_directories_and_files(M, p, output, M_files);
#else
  merge_directories_and_files(M, p, output);
#endif

  // Merge the compile unit and subprogram nodes, updating the
  // entries in the DST map.
  merge_subprograms(M, p, output);

#ifdef TARG_ST
  // [CL] update references to filenames before updating PUs
  // (otherwise we would have no longer access to the original refs)
  merge_file_refs(M, p, output, M_files);
  update_wns(M, M_files, pu_tree);
#endif

  // Update the DST indices in the pu tree.
  update_pu_dst_indices(M, pu_tree);

  return output;
}
