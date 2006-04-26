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
 * Module: dst.c
 *
 * Revision history:
 *  01-May-93 - Original Version
 *
 * Description:
 *  Implements generation of dwarf debugging information be means of
 *  ../common/com/dwarf_DST_producer.h.  The information is generated
 *  by starting with the outermost scope (il_header.primary_scope) and
 *  handling nested scopes as they are encountered. 
 *
 *  Note how we mark nodes that have no DST_INFO_IDX associated with
 *  them as visited by means of a static file-scope variable 
 *  (e.g. void_is_visited).  This is necessary when we want to mark
 *  a type of edg node as visited, without a ASSOC_DST field.
 * ====================================================================
 * ====================================================================
 */


static char *source_file = __FILE__;
#ifdef _KEEP_RCS_ID
#endif /* _KEEP_RCS_ID */

#include "W_values.h"

#include "defs.h"
#include "glob.h"
#include "config.h"
#include "dwarf_DST_producer.h"
#include "dwarf_DST_dump.h"
#include "file_util.h"  /* From common/util */
#include "srcpos.h"
#include "symtab.h"
#include "gnu_config.h"
#include "gnu/flags.h"
  // [HK]
#if __GNUC__ >=3
#include <string>
#endif // __GNUC__ >=3
extern "C" {
#include "gnu/system.h"
#include "gnu/tree.h"
#ifdef TARG_ST
  /* (cbr) fix includes */
#include "gnu/cp/cp-tree.h"
#else
#include "cp-tree.h"
#endif
}
#include "wfe_misc.h"
#include "wfe_dst.h"
#include "wfe_expr.h"
#include "tree_symtab.h"

#include <sys/types.h>
#include <sys/stat.h>   /* For accessing file statistics (stat()) */
#ifdef sun
#include <netdb.h>      /* define MAXHOSTNAMELEN on Solaris */
#else
#ifndef __MINGW32__
#include <sys/param.h>  /* define MAXHOSTNAMELEN elsewhere  */
#else
#include <winsock.h>
#define MAXHOSTNAMELEN 256
#endif
#endif
#include <unistd.h>     /* for gethostname() and getcwd() */
  // [HK]
#if __GNUC__ <3
#include <string>
#endif // __GNUC__ <3
#include <vector>
#include <map>


extern FILE *tree_dump_file; //for debugging only

static BOOL dst_initialized = FALSE;

#if defined(sun)
#include <sys/systeminfo.h>
#define getdomainname(x,y) sysinfo(SI_SRPC_DOMAIN,x,y)
#endif
#if defined(__CYGWIN__) || defined(__MINGW32__)
#include <sys/unistd.h>
#define getdomainname(x,y) gethostname(x,y)
#endif

#define MAX_CWD_CHARS (256 - (MAXHOSTNAMELEN+1))
static char  cwd_buffer[MAX_CWD_CHARS+MAXHOSTNAMELEN+1];
static char *current_working_dir = &cwd_buffer[0];
static char *current_host_dir = &cwd_buffer[0];

// A file-global current scope is not useful, as with gcc we see
// declarations out of context and must derive right context.
//static DST_INFO_IDX current_scope_idx = DST_INVALID_INIT;

static DST_INFO_IDX comp_unit_idx = DST_INVALID_INIT;	// Compilation unit

static void DST_enter_file (char *, UINT);

static UINT last_dir_num = 0;
static UINT current_dir = 1;
static UINT last_file_num = 0;
UINT current_file = 1;

static DST_INFO_IDX DST_Create_var(ST *var_st, tree decl);
static DST_INFO_IDX DST_Create_Parmvar(ST *var_st, tree decl);
static DST_INFO_IDX DST_Create_type(ST *typ_decl, tree decl);
static void DST_enter_param_vars(tree fndecl,DST_INFO_IDX parent,tree parameter_list,int is_abstract_root, int is_declaration_only);




static std::vector< std::pair< char *, UINT > > dir_dst_list;
typedef std::map< std::string, DST_INFO_IDX > DST_Type_Map;
static DST_Type_Map basetypes;

// Use DECL_CONTEXT or TYPE_CONTEXT to get
// the  index of the current applicable scope
// for the thing indicated.
// We already got to TYPE_CONTEXT/DECL_CONTEXT on the input
//   The TYPE_CONTEXT for any sort of type which could have a name or
//    which could have named members (e.g. tagged types in C/C++) will
//    point to the node which represents the scope of the given type, or
//    will be NULL_TREE if the type has "file scope".  For most types, this
//    will point to a BLOCK node or a FUNCTION_DECL node, but it could also
//    point to a FUNCTION_TYPE node (for types whose scope is limited to the
//    formal parameter list of some function type specification) or it
//    could point to a RECORD_TYPE, UNION_TYPE or QUAL_UNION_TYPE node
//    (for C++ "member" types).
//   DECL_CONTEXT points to the node representing the context in which
//    this declaration has its scope.  For FIELD_DECLs, this is the
//    RECORD_TYPE, UNION_TYPE, or QUAL_UNION_TYPE node that the field
//    is a member of.  For VAR_DECL, PARM_DECL, FUNCTION_DECL, LABEL_DECL,
//    and CONST_DECL nodes, this points to either the FUNCTION_DECL for the
//    containing function, the RECORD_TYPE or UNION_TYPE for the containing
//    type, or NULL_TREE if the given decl has "file scope".


static DST_INFO_IDX 
DST_get_context(tree intree)
{
    tree ltree = intree;
    DST_INFO_IDX l_dst_idx = DST_INVALID_INIT;

    while(ltree) {
	switch(TREE_CODE(ltree)) {
	case BLOCK:
#ifndef TARG_ST // [CL] handle block tree
	  ltree =  BLOCK_SUPERCONTEXT(ltree);
#else
	    // unclear when this will happen, as yet
	    // FIX
	    ltree = TYPE_CONTEXT(ltree);
            DevWarn("Unhandled BLOCK scope of decl/var");
#endif
	    break;
	case FUNCTION_DECL: {
		// This is a normal case!
		l_dst_idx = DECL_DST_IDX(ltree);
		if(DST_IS_NULL(l_dst_idx)) {
			DevWarn("forward reference to subprogram! assuming global context\n");
			return comp_unit_idx;
		}
		return l_dst_idx;

        }
	    break;
	case RECORD_TYPE:
	    ltree = TYPE_CONTEXT(ltree);
	    break;
	case UNION_TYPE:
	    ltree = TYPE_CONTEXT(ltree);
	    break;
	case QUAL_UNION_TYPE:
	    ltree = TYPE_CONTEXT(ltree);
	    break;
	case FUNCTION_TYPE:
            DevWarn("Unhandled FUNCTION_TYPE scope of decl/var/type");
	    return comp_unit_idx;
        case REFERENCE_TYPE:
	    // cannot find our context from here
	    return comp_unit_idx;
	default:
#ifndef TARG_ST
	    DevWarn("Unhandled scope of tree code %d",
			TREE_CODE(ltree));
#endif

	   // *is any of this right?
           if(TREE_CODE_CLASS(TREE_CODE(ltree)) == 'd') {
		ltree =  DECL_CONTEXT(ltree);
		continue;
	   } else if (TREE_CODE_CLASS(TREE_CODE(ltree)) == 't') {
		ltree =  TYPE_CONTEXT(ltree);
		continue;
           } else {
	      // cannot find our context from here
		// ??
           }
	   return comp_unit_idx;
	}

    }
    // This is  the normal case for most things.
    return comp_unit_idx;
}


// get the directory path dst info.
// if already exists, return existing info, else append to list.
static UINT
Get_Dir_Dst_Info (char *name)
{
        std::vector< std::pair < char*, UINT > >::iterator found;
	// assume linear search is okay cause list will be small?
        for (found = dir_dst_list.begin(); 
		found != dir_dst_list.end(); 
		++found)
        {
		if (strcmp ((*found).first, name) == 0) {
			return (*found).second;
		}
	}
	// not found, so append path to dst list
#ifdef TARG_ST
	// [CG] Merge fix from gccfe:
	// //[CM] (MBTst16964, MBTst16965) Extend name livetime
	dir_dst_list.push_back (std::make_pair (xstrdup(name), ++last_dir_num));
#else
	dir_dst_list.push_back (std::make_pair (name, ++last_dir_num));
#endif
	DST_mk_include_dir (name);
	return last_dir_num;
}

static std::vector< std::pair< char *, UINT > > file_dst_list;

// get the file dst info.
// if already exists, return existing info, else append to list.
static UINT
Get_File_Dst_Info (char *name, UINT dir)
{
        std::vector< std::pair < char*, UINT > >::iterator found;
	// assume linear search is okay cause list will be small?
        for (found = file_dst_list.begin(); 
		found != file_dst_list.end(); 
		++found)
        {
		if (strcmp ((*found).first, name) == 0) {
			return (*found).second;
		}
	}
	// not found, so append file to dst list
#ifdef TARG_ST
	// [CG] Merge fix from gccfe:
	// //[CM] (MBTst16964, MBTst16965) Extend name livetime
	file_dst_list.push_back (std::make_pair (xstrdup(name), ++last_file_num));
#else
	file_dst_list.push_back (std::make_pair (name, ++last_file_num));
#endif
	DST_enter_file (name, dir);
	return last_file_num;
}


/* drops path prefix in string */
static char *
drop_path (char *s)
{
        char *tail;
        tail = strrchr (s, '/');
        if (tail == NULL) {
                return s;       /* no path prefix */
        } else {
                tail++;         /* skip the slash */
                return tail;    /* points inside s, not new string! */
        }
}

static void
DST_enter_file (char *file_name, UINT dir)
{
        UINT64 file_size = 0;
        UINT64 fmod_time = 0;
        struct stat fstat;
        if (stat(file_name, &fstat) == 0) {
                /* File was found, so set to non-zero values */
                file_size = (UINT64)fstat.st_size;
                fmod_time = (UINT64)fstat.st_mtime;
        }
        DST_mk_file_name(
                file_name,
                dir,
                file_size,
                fmod_time);
}

/* Given the set of options passed into the front-end, string
 * together the ones of interest for debugging and return
 * the resultant string.  The options of interest depends on 
 * the level of debugging.  The caller should free the malloced
 * string once it is no longer needed.
 */
static char *
DST_get_command_line_options(INT32 num_copts, 
			     char *copts[])
{
  INT32	    i, 
            strlength = 0;
  INT32     num_opts = 0;
  char    **selected_opt;
  INT32    *opt_size;
  char     *rtrn, *cp;
  char      ch;
  INT32     record_option;

      // [HK]
#if __GNUC__ >= 3
  selected_opt = (char **)xmalloc(sizeof(char*) * num_copts);
  opt_size = (INT32 *)xmalloc(sizeof(INT32) * num_copts);
#else
  selected_opt = (char **)malloc(sizeof(char*) * num_copts);
  opt_size = (INT32 *)malloc(sizeof(INT32) * num_copts);
#endif // __GNUC__ >= 3
  
  for (i = 1; i <= num_copts; i++)
  {
     if (copts[i] != NULL && copts[i][0] == '-')
     {
	ch = copts[i][1];  /* Next flag character */
	if (Debug_Level <= 0)
	   /* No debugging */
	   record_option = (ch == 'g' || /* Debugging option */
			    ch == 'O');  /* Optimization level */
	else
	   /* Full debugging */
	   record_option = (ch == 'D' || /* Macro symbol definition */
			    ch == 'g' || /* Debugging option */
			    ch == 'I' || /* Search path for #include files */
			    ch == 'O' || /* Optimization level */
			    ch == 'U');  /* Macro symbol undefined */
	if (record_option)
	{
	   opt_size[num_opts] = strlen(copts[i]) + 1; /* Arg + space/null */
	   selected_opt[num_opts] = copts[i];
	   strlength += opt_size[num_opts];
	   num_opts += 1;
	}
     }
  }
  
  if (strlength == 0)
  {
      // [HK]
#if __GNUC__ >= 3
     rtrn = (char *)xcalloc(1, 1); /* An empty string */
#else
     rtrn = (char *)calloc(1, 1); /* An empty string */
#endif // __GNUC__ >= 3
  }
  else
  {
      // [HK]
#if __GNUC__ >= 3
     rtrn = (char *)xmalloc(strlength);
#else
     rtrn = (char *)malloc(strlength);
#endif // __GNUC__ >= 3
     cp = rtrn;

     /* Append the selected options to the string (rtrn) */
     for (i = 0; i < num_opts; i++)
	if (opt_size[i] > 0)
	{
	   cp = strcpy(cp, selected_opt[i]) + opt_size[i];
	   cp[-1] = ' '; /* Space character */
	}
     cp[-1] = '\0'; /* Terminating null character */
  }
  
  free(selected_opt);
  free(opt_size);
  return rtrn;
} /* DST_get_command_line_options */

static char *
Get_Name (tree node)
{
  static char buf[64];


  if (node == NULL) {
		buf[0] = 0;
                return buf;
  }
  char *name = buf;
  buf[0] = 0;

#define DANAME(d) ((TREE_CODE_CLASS(TREE_CODE(d)) =='d')? \
((DECL_NAME(d))?IDENTIFIER_POINTER(DECL_NAME(d)):"?"):\
 "?2")


  int tc_class = (int)TREE_CODE_CLASS(TREE_CODE(node));

  if (tc_class == 'd')
  {
      if (DECL_NAME (node)) {
        name = IDENTIFIER_POINTER (DECL_NAME (node));
      }
  }
  else if (tc_class == 't')
  {
      if (TYPE_NAME (node))
        {
          if (TREE_CODE (TYPE_NAME (node)) == IDENTIFIER_NODE)
            name =  IDENTIFIER_POINTER (TYPE_NAME (node));
#ifdef TARG_ST // [CL] Handle anonymous unions
          else if (TREE_CODE (TYPE_NAME (node)) == TYPE_DECL
		   && ! DECL_IGNORED_P( TYPE_NAME(node))
                   && DECL_NAME (TYPE_NAME (node)))
            name = IDENTIFIER_POINTER (DECL_NAME (TYPE_NAME (node)));
#else
          else if (TREE_CODE (TYPE_NAME (node)) == TYPE_DECL
                   && DECL_NAME (TYPE_NAME (node)))
            name = IDENTIFIER_POINTER (DECL_NAME (TYPE_NAME (node)));
#endif
	  
        } 
  } else {
  }
  return name;
}

// static data member of class
// Since there will be a global somewhere with this,
// we need to avoid creating ST information. 
// ST information
// the DECL dst record.
//
static void
DST_enter_static_data_mem(tree  parent_tree,
                DST_INFO_IDX parent_idx,
                TY_IDX parent_ty_idx,
                tree field)
{
	/* cannot be a bit field */
    int isbit = 0;
    DST_INFO_IDX field_idx = DST_INVALID_INIT;

    char * linkage_name = 
  		IDENTIFIER_POINTER(DECL_ASSEMBLER_NAME (field));
    char * mem_name = Get_Name(field);


    tree ftype = TREE_TYPE(field);


    // We don't want ST entries created for this class decl.
    ST * st = 0; // Do not do Get here Get_ST(field);
    ST_IDX base =  ST_IDX_ZERO; //  Nothere ST_st_idx(st);


    DST_INFO_IDX fidx = Create_DST_type_For_Tree(ftype,base,parent_ty_idx);

    USRCPOS src;
    // For now, the source location appears bogus
    // (or at least odd) for files other than the base
    // file, so lets leave it out. Temporarily.
    //USRCPOS_srcpos(src) = Get_Srcpos();
    USRCPOS_clear(src);


#ifdef TARG_ST
    // [CL]
    DST_accessibility access = DW_ACCESS_public;

    if (TREE_PROTECTED(field))
      access = DW_ACCESS_protected;
    else if (TREE_PRIVATE(field))
      access = DW_ACCESS_private;
#endif

    field_idx = DST_mk_variable(
        src,         // srcpos
        mem_name,  // user typed name, not mangled
        fidx,        // user typed type name here (typedef type perhaps).
        0,           // offset (fortran uses non zero )
        (void*) base, // underlying type here, not typedef.
        DST_INVALID_IDX,  // abstract origin
        TRUE,          // is_declaration=  decl only
        FALSE,         // is_automatic ?
#ifdef TARG_ST
	// [CL]
	TREE_PUBLIC(field) ? TRUE : FALSE, // is_external ?
        DECL_ARTIFICIAL(field),      // is_artificial ?
	access);
#else
        FALSE,         // is_external ?
        FALSE  );      // is_artificial ?
#endif

    DECL_DST_FIELD_IDX(field) = field_idx;
    DST_append_child(parent_idx,field_idx);

    DST_INFO_IDX varidx = DECL_DST_IDX(field);
    DECL_DST_SPECIFICATION_IDX(field) = field_idx;

    // FIXME: need a data version of this.
    //if(mem_name && linkage_name && strcmp(mem_name, linkage_name)) {
    //   DST_add_linkage_name_to_subprogram(field_idx, linkage_name);
    //}
#ifdef TARG_ST
// [CL] dont output invisible names
    if(mem_name && linkage_name && strcmp(mem_name, linkage_name) &&
       TREE_PUBLIC(field) && !DECL_ABSTRACT(field) ) {
      DST_add_linkage_name_to_variable(field_idx, linkage_name);
    }
#endif
    DECL_DST_SPECIFICATION_IDX(field) = field_idx;

    return ;
}


// Called for member functions, whether static member funcs
// or non-static member funcs.
// Here we add the member func to the class decl.
// If the function has arguments and the first argument's name
// is "this" it is a non-static member function. Otherwise it
// is a static member function.
// We don't want types added here to be remembered:
// not DST and not ST information. As this is not a definition point.
// (for some functions it can be, but we don't yet handle that)
// 
static void
DST_enter_member_function( tree parent_tree,
		DST_INFO_IDX parent_idx,
                TY_IDX parent_ty_idx,
		tree fndecl)
{
    USRCPOS src;
    USRCPOS_srcpos(src) = Get_Srcpos();
    DST_INFO_IDX dst = DST_INVALID_INIT;
    DST_INFO_IDX ret_dst = DST_INVALID_IDX;
                                     
    DST_INFO_IDX current_scope_idx = parent_idx;

    

#ifdef TARG_ST
// [CL] this is the actual type of the result
    tree resdecl = TREE_TYPE(fndecl);
#else
    tree resdecl = DECL_RESULT(fndecl);
#endif
    tree restype = 0;
    if( resdecl) {
	   restype = TREE_TYPE(resdecl);
    }
    if(restype) {
	 TY_IDX itx = Get_TY(restype);
	 ret_dst = TYPE_DST_IDX(restype);
    }

    BOOL is_prototyped = TRUE;

#ifdef TARG_ST
    char * basename = xstrdup((char*)lang_decl_name(fndecl, 0));
 #else
    char * basename = 
	IDENTIFIER_OPNAME_P(DECL_NAME(fndecl))? 0 :
	IDENTIFIER_POINTER (DECL_NAME (fndecl));
#endif
    char * linkage_name = 
	IDENTIFIER_POINTER(DECL_ASSEMBLER_NAME (fndecl));

    tree ftype = TREE_TYPE(fndecl);

    int is_abstract_root = 0; // Simply a decl here, not abs. root.

    // is_declaration TRUE  as this is function declared in class,
    // not a definition or abstract root.
    TY_IDX base =  Get_TY(ftype);


    BOOL is_external = TRUE;
    DST_virtuality  virtuality = 
		 DECL_PURE_VIRTUAL_P(fndecl)?
			 DW_VIRTUALITY_pure_virtual
		 : DECL_VIRTUAL_P(fndecl)?
			DW_VIRTUALITY_virtual : DW_VIRTUALITY_none;
    // FIX virtuality.

    DST_inline inlin = 0; 
		//DECL_PENDING_INLINE_P(fndecl)?
		//	DW_INL_inlined: DW_INL_not_inlined;

    
#ifndef TARG_ST
    //FIX vtable elem loc
    DST_vtable_elem_location vtable_elem_location =  0;
#else
    DST_vtable_elem_location vtable_elem_location;
    /* (cbr) elem loc must be an integer */
    if (DECL_VINDEX(fndecl) && TREE_CODE (DECL_VINDEX(fndecl)) == INTEGER_CST) {
        vtable_elem_location = Get_Integer_Value(DECL_VINDEX(fndecl));
    } else {
        vtable_elem_location = 0;
    }
#endif

    dst = DST_mk_subprogram(
        src,			// srcpos
        basename,
        ret_dst,        	// return type
        DST_INVALID_IDX,        // Index to alias for weak is set later
        (void*) 0,              // index to fe routine for st_idx
        inlin,                  // dwarf inline code.
        virtuality,     	// applies to C++, dwarf virt code
        vtable_elem_location,   // vtable_elem_location (vtable slot #
				// as emitted in MIPS, something else
				// by gcc for ia64 )
        TRUE,         // is_declaration
        is_prototyped,           // always true for C++
#ifdef TARG_ST
	// [CL]
        is_external,  // is_external? (has external linkage)
	DECL_ARTIFICIAL(fndecl) );
#else
        is_external );  // is_external? (has external linkage)
#endif

    // producer routines thinks we will set pc to fe ptr initially
    DST_RESET_assoc_fe (DST_INFO_flag(DST_INFO_IDX_TO_PTR(dst)));

    DST_append_child (current_scope_idx, dst);



    DECL_DST_FIELD_IDX(fndecl) = dst;

    // Now we create the argument info itself, relying
    // on the is_prototyped flag above to let us know if
    // we really should do this.
    if(is_prototyped) {
       tree parms = DECL_ARGUMENTS(fndecl);
       if(!parms) {
          // no arguments: int y(); for example in C++.
       } else {
	   // This kills be with mem func. FIX
	   DST_enter_param_vars(fndecl, 
		dst,
		parms,
		is_abstract_root,
		/* is_declaration_only */ 1);
       }

    }
#ifdef TARG_ST
// [CL] dont output invisible names
    if(basename && linkage_name && strcmp(basename, linkage_name) &&
       TREE_PUBLIC(fndecl) && !DECL_ABSTRACT(fndecl) ) {
#else
    if(basename && linkage_name && strcmp(basename, linkage_name)) {
#endif
       DST_add_linkage_name_to_subprogram(dst, linkage_name);
    }
    DECL_DST_SPECIFICATION_IDX(fndecl) = dst;
}

        
#ifdef TARG_ST
/* Given a value, round it up to the lowest multiple of `boundary'
   which is not less than the value itself.  */

static inline INT
ceiling (INT value, unsigned int boundary)
{
  return (((value + boundary - 1) / boundary) * boundary);
}
#endif

static void
DST_enter_normal_field(tree  parent_tree,
		DST_INFO_IDX parent_idx,
		TY_IDX parent_ty_idx,
		tree field)
{
    char isbit = 0; 

#ifdef TARG_ST
    // [CL] Ignore the nameless fields that are used to skip bits but
    // handle C++ anonymous unions.
    if (DECL_NAME(field) == NULL_TREE &&
	TREE_CODE(TREE_TYPE(field)) != UNION_TYPE) {
      return;
    }
#endif
    if ( ! DECL_BIT_FIELD(field)
#ifdef TARG_ST
         /* (cbr) if size is not set it's not going to be used as a bitfield */
           && DECL_SIZE (field) 
#endif
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
	   // already warned
           //DevWarn(field size %d does not match type size %d,
           //                            DECL_SIZE(field),
           ////                           TY_size(Get_TY(TREE_TYPE(field)))
           //                                   * BITSPERBYTE );

	   isbit = 1;
    }

    if (DECL_BIT_FIELD(field)) {
	   isbit = 1;
    }
    DST_INFO_IDX field_idx = DST_INVALID_INIT;
    char *field_name = Get_Name((field));

    tree ftype = TREE_TYPE(field);


    TY_IDX base = Get_TY(ftype);

    DST_INFO_IDX fidx = Create_DST_type_For_Tree(ftype,base,parent_ty_idx);

    USRCPOS src;
    // For now, the source location appears bogus
    // (or at least odd) for files other than the base
    // file, so lets leave it out. Temporarily.
    //USRCPOS_srcpos(src) = Get_Srcpos();
    USRCPOS_clear(src);

#ifdef TARG_ST
	/* The following code is inspired from dwarf2out.c:
	   field_byte_offset, add_bit_offset_attribute */

        INT bitoff = int_bit_position(field);

	unsigned int type_align_in_bytes;
	unsigned int type_align_in_bits;
	UINT type_size_in_bits;

 	tree type_size = TYPE_SIZE(ftype);

	type_size_in_bits = Get_Integer_Value(type_size);
	type_align_in_bits = TYPE_ALIGN(ftype);
	UINT align = TYPE_ALIGN(ftype)/BITSPERBYTE;
	type_align_in_bytes = type_align_in_bits / BITS_PER_UNIT;

	INT object_offset_in_align_units;
	INT object_offset_in_bits;
	INT object_offset_in_bytes;
	INT deepest_bitpos;
	UINT field_size_in_bits;

#ifdef TARG_ST
        /* (cbr) */
        if (!DECL_SIZE(field))
          field_size_in_bits = Get_Integer_Value (type_size);
        else
#endif
          field_size_in_bits = Get_Integer_Value(DECL_SIZE(field));

	/* Figure out the bit-distance from the start of the structure to the
	   "deepest" bit of the bit-field.  */
	deepest_bitpos = bitoff + field_size_in_bits;

	/* This is the tricky part.  Use some fancy footwork to deduce where the
	   lowest addressed bit of the containing object must be.  */
	object_offset_in_bits
	  = ceiling (deepest_bitpos, type_align_in_bits) - type_size_in_bits;

	/* Compute the offset of the containing object in "alignment units".  */
	object_offset_in_align_units = object_offset_in_bits / type_align_in_bits;

	/* Compute the offset of the containing object in bytes.  */
	object_offset_in_bytes = object_offset_in_align_units * type_align_in_bytes;

	INT fld_offset_bytes = object_offset_in_bytes;
#else
    INT bitoff = Get_Integer_Value(DECL_FIELD_BIT_OFFSET(field));
INT fld_offset_bytes = bitoff / BITSPERBYTE;
 tree type_size = TYPE_SIZE(ftype);
UINT align = TYPE_ALIGN(ftype)/BITSPERBYTE;
#endif
    INT tsize;
    if (type_size == NULL) {
          // incomplete structs have 0 size
          Fail_FmtAssertion("DST_enter_normal_field: type_size NULL ");
          tsize = 0;
    }
    else {
          if (TREE_CODE(type_size) != INTEGER_CST) {
            if (TREE_CODE(type_size) == ARRAY_TYPE)
              Fail_FmtAssertion ("Encountered VLA at line %d", lineno);
            else
              Fail_FmtAssertion ("VLA at line %d not currently implemented", 
		lineno);
            tsize = 0;
          }
          else
            tsize = Get_Integer_Value(type_size) / BITSPERBYTE;
    }

#ifdef TARG_ST
    // [CL]
    DST_accessibility access = DW_ACCESS_public;

    if (TREE_PROTECTED(field))
      access = DW_ACCESS_protected;
    else if (TREE_PRIVATE(field))
      access = DW_ACCESS_private;
#endif

    if(isbit == 0) {
	  field_idx = DST_mk_member(
		src,
		field_name,
		fidx, // field type	
		fld_offset_bytes, // field offset in bytes
			0, // no container (size zero)
			0, // bit_offset= no offset into container
		        0, // bit_size= no bitfield size
	       FALSE, // is_bitfield= not a bitfield
	       FALSE, // is_static= not a static member
	       FALSE, // is_declaration= 
#ifdef TARG_ST
  // [CL]
	       DECL_ARTIFICIAL(field), // is_artificial = no
	       access);
#else
	       FALSE); // is_artificial = no
#endif
			
    } else {
	  if(tsize == 0) {
	   Fail_FmtAssertion("bit field type size 0!");
	   return;
	  }
#ifdef TARG_ST
	  INT highest_order_object_bit_offset;
	  INT highest_order_field_bit_offset;

	  highest_order_object_bit_offset = object_offset_in_bytes * BITS_PER_UNIT;
	  highest_order_field_bit_offset = bitoff;

	  if (! BYTES_BIG_ENDIAN)
	    {
	      highest_order_field_bit_offset += field_size_in_bits;
	      highest_order_object_bit_offset += type_size_in_bits;
	    }

	  UINT into_cont_off
	    = (! BYTES_BIG_ENDIAN
	       ? highest_order_object_bit_offset - highest_order_field_bit_offset
	       : highest_order_field_bit_offset - highest_order_object_bit_offset);

#else
	  UINT container_off = fld_offset_bytes - (fld_offset_bytes%align);
	  UINT into_cont_off = bitoff - (container_off*BITSPERBYTE);
#endif

	  field_idx = DST_mk_member(
			src,
			field_name,
			fidx	,      // field type	
			fld_offset_bytes,    // container offset in bytes
			tsize,         // container size, bytes
                        into_cont_off, // offset into 
					// container, bits

                        Get_Integer_Value(DECL_SIZE(field)), // bitfield size
                        TRUE, // a bitfield
                        FALSE, // not a static member
                        FALSE, // Only TRUE for C++?
#ifdef TARG_ST
  // [CL]
			DECL_ARTIFICIAL(field), // is_artificial = no
			access);
#else
                        FALSE); // artificial (no)
#endif
    }
    DST_append_child(parent_idx,field_idx);


	
    return ;
}
static void
DST_enter_struct_union_members(tree parent_tree, 
	DST_INFO_IDX parent_idx  )
{ 
    DST_INFO_IDX dst_idx = DST_INVALID_INIT;

    TY_IDX parent_ty_idx = Get_TY(parent_tree);
    
    //if(TREE_CODE_CLASS(TREE_CODE(parent_tree)) != 'd') {
     //   DevWarn("DST_enter_struct_union_members input not 't' but %c",
      //          TREE_CODE_CLASS(TREE_CODE(parent_tree)));
   // }

    tree field = TREE_PURPOSE(parent_tree);
    for( ; field ; field = TREE_CHAIN(field) )
    { 
	if(TREE_CODE(field) == FIELD_DECL) {
	   DST_enter_normal_field( parent_tree,parent_idx,
		parent_ty_idx,field);
	} else if(TREE_CODE(field) == VAR_DECL) {
		// Here create static class data mem decls
		// These cannot be definitions.
	   DST_enter_static_data_mem( parent_tree,parent_idx,
		parent_ty_idx,field);
        } else if (TREE_CODE(field) == FUNCTION_DECL) {
		// Cannot happen.
		DevWarn("Impossible FUNCTION DECL member of class!\n");
	}  else {
	  // RECORD_TYPE is class itself, apparently,
	  // and can appear here.
        }
    }

    // member functions
    tree methods = TYPE_METHODS(parent_tree);
    for  ( ; methods != NULL_TREE; methods = TREE_CHAIN(methods)) {
	if(TREE_CODE(methods) == FUNCTION_DECL ) {
#ifdef TARG_ST
// [CL] Don't include clones in the member list
      if (DECL_MAYBE_IN_CHARGE_CONSTRUCTOR_P(methods)
       || DECL_MAYBE_IN_CHARGE_DESTRUCTOR_P(methods))
	continue;
#endif
#ifndef TARG_ST
// [CL]
	   if ( DECL_ARTIFICIAL(methods)) {
	     // compiler generated methods are not interesting.
	     // We want only ones user coded.
	     continue;	   
	   } else {
#endif
	     DST_enter_member_function( parent_tree,parent_idx,
		parent_ty_idx,methods);
#ifndef TARG_ST
	   }
#endif
	}
    }

    return;
}


// We have a struct/union. Create a DST record
// and enter it.
static DST_INFO_IDX
DST_enter_struct_union(tree type_tree, TY_IDX ttidx  , TY_IDX idx, 
		INT tsize)
{ 
    DST_INFO_IDX dst_idx  = TYPE_DST_IDX(type_tree);

    DST_INFO_IDX current_scope_idx =
         DST_get_context(TYPE_CONTEXT(type_tree));

    if(DST_IS_NULL(dst_idx)) {

	// not yet created, so create it


	// Deal with scope here (FIX)
        // in case scope of decl is different from scope of ref
	//
        USRCPOS src;
        // For now, the source location appears bogus
        // (or at least odd) for files other than the base
        // file, so lets leave it out. Temporarily.
        //USRCPOS_srcpos(src) = Get_Srcpos();
        USRCPOS_clear(src);

	char *name = Get_Name(type_tree);

	if(TREE_CODE(type_tree) == RECORD_TYPE) {
	   dst_idx = DST_mk_structure_type(src,
		  name , // struct tag name
		  tsize,
		  DST_INVALID_IDX, // not inlined
		   TREE_PURPOSE(type_tree)== 0   // 1 if incomplete
		   );
	} else if (TREE_CODE(type_tree) == UNION_TYPE) {
	   dst_idx = DST_mk_union_type(src,
		  name  , // union tag name
		  tsize,
		  DST_INVALID_IDX, // not inlined
		   TREE_PURPOSE(type_tree)== 0   // arg 1 if incomplete
		   );
	} else {
	  // no DST_enter_struct_union_members(type_tree,dst_idx);
          // leave as DST_IS_NULL
          return dst_idx;
	}
	DST_append_child(current_scope_idx,dst_idx);

	// set this now so we will not infinite loop
	// if this has ptr to itself inside.
        TYPE_DST_IDX(type_tree) = dst_idx;


#ifdef TARG_ST
	// [CL] record what type our vtable lives in
	DST_INFO_IDX vtidx = DST_INVALID_IDX;

	if (TYPE_VFIELD (type_tree)) {
	  tree vtype = DECL_FCONTEXT (TYPE_VFIELD (type_tree));

	  if (vtype != type_tree) {
	    // Generate type if it is not the one we are currently
	    // generating
	    TY_IDX itx =  TYPE_TY_IDX(vtype);
	    vtidx = Create_DST_type_For_Tree (vtype,itx, idx);
	  } else {
	    vtidx = dst_idx;
	  }
	}
	if(TREE_CODE(type_tree) == RECORD_TYPE) {
	  DST_add_structure_containing_type(dst_idx, vtidx);
	} else if (TREE_CODE(type_tree) == UNION_TYPE) {
	  DST_add_union_containing_type(dst_idx, vtidx);
	}
#endif

  	// Do the base classes
        INT32 offset = 0;
        INT32 anonymous_fields = 0;
        if (TYPE_BINFO(type_tree) &&
                    BINFO_BASETYPES(TYPE_BINFO(type_tree))) {
          tree basetypes = BINFO_BASETYPES(TYPE_BINFO(type_tree));
          INT32 i;
          for (i = 0; i < TREE_VEC_LENGTH(basetypes); ++i) {
#ifdef TARG_ST // [CL] update for 3.x frontend version
    	            INT32 virtual_offset;
#endif
                    tree binfo = TREE_VEC_ELT(basetypes, i);
                    tree basetype = BINFO_TYPE(binfo);

		    int virtuality = DW_VIRTUALITY_none;
		    if(TREE_VIA_VIRTUAL(binfo)) {
			// Don't know how to tell if
			// it is pure_virtual.  FIX
			virtuality = DW_VIRTUALITY_virtual;
#ifdef TARG_ST // [CL]
			virtual_offset = -Get_Integer_Value(BINFO_VPTR_FIELD(binfo));
#endif
		    }
#ifdef TARG_ST // [CL]
		    else {
		      virtual_offset = 0;
		    }
#endif
#ifdef TARG_ST // [CL]
		    offset = Get_Integer_Value(BINFO_OFFSET(binfo));
#else
                    offset = Roundup (offset,
                                    TYPE_ALIGN(basetype) / BITSPERBYTE);
#endif

		    TY_IDX itx =  TYPE_TY_IDX(basetype);
		    DST_INFO_IDX bcidx =
                       Create_DST_type_For_Tree (basetype,itx, idx);

		    // There is no way to pass in DW_ACCESS_* here.
		    // That is am omission.  FIX
#ifdef TARG_ST
		    // [CL]
		    DST_accessibility access = DW_ACCESS_private;

		    if (TREE_VIA_PROTECTED(binfo))
		      access = DW_ACCESS_protected;
		    else if (TREE_VIA_PUBLIC(binfo))
		      access = DW_ACCESS_public;

		    DST_INFO_IDX inhx = 
		      DST_mk_inheritance(src,
					 bcidx,
					 virtuality,
					 offset,
					 access,
					 virtual_offset);
#else
		    DST_INFO_IDX inhx = 
		      DST_mk_inheritance(src,
				bcidx,
			        virtuality,
				offset);
#endif

		    DST_append_child(dst_idx,inhx);

                    if (!is_empty_base_class(basetype) ||
                        !TREE_VIA_VIRTUAL(binfo)) {
                      //FLD_Init (fld, Save_Str(Get_Name(0)),
                       //         Get_TY(basetype) , offset);
                      offset += Type_Size_Without_Vbases (basetype);
                    }

          }
        }


	// now can do the members of our type.
	DST_enter_struct_union_members(type_tree,dst_idx);

    }

    return  dst_idx;
}


// We have a enum. 
// and enter it.
static DST_INFO_IDX
DST_enter_enum(tree type_tree, TY_IDX ttidx  , TY_IDX idx, 
		INT tsize)
{ 
   DST_INFO_IDX dst_idx = 
       TYPE_DST_IDX(type_tree);

   if(TREE_CODE_CLASS(TREE_CODE(type_tree)) != 't') {
        DevWarn("DST_enter_enum input not 't' but %c",
                TREE_CODE_CLASS(TREE_CODE(type_tree)));
   }
   DST_INFO_IDX current_scope_idx =
        DST_get_context(TYPE_CONTEXT(type_tree));


   if(DST_IS_NULL(dst_idx)) {
      DST_INFO_IDX t_dst_idx = DST_INVALID_INIT;
      USRCPOS src;
      // For now, the source location appears bogus
      // (or at least odd) for files other than the base
      // file, so lets leave it out. Temporarily.
      //USRCPOS_srcpos(src) = Get_Srcpos();
      USRCPOS_clear(src);
      char *name1 = Get_Name(type_tree);
      tree enum_entry = TYPE_VALUES(type_tree);
      DST_size_t e_tsize =  tsize;
      t_dst_idx = DST_mk_enumeration_type( src,
                           name1,
                           e_tsize, // Type size.
                           DST_INVALID_IDX, // Not inlined.
                           (enum_entry==NULL_TREE)); // Pass non-zero 
						// if incomplete.
      DST_append_child(current_scope_idx,t_dst_idx);

      TYPE_DST_IDX(type_tree) = t_dst_idx;

#ifdef TARG_ST
      // [CL] do not forget to update the return value
      dst_idx = t_dst_idx;
#endif

      DST_CONST_VALUE enumerator;
      if(tsize == 8) {
	   DST_CONST_VALUE_form(enumerator) =  DST_FORM_DATA8;
      } else if (tsize == 4) {
	   DST_CONST_VALUE_form(enumerator) =  DST_FORM_DATA4;
      } else {
	   // ???
	   DST_CONST_VALUE_form(enumerator) =  DST_FORM_DATA4;
	   DevWarn("Unexpected type size  %d in enumerator",
		(int)tsize);
      }

      for( ; enum_entry; enum_entry = TREE_CHAIN(enum_entry) ) {
         USRCPOS src;
         // For now, the source location appears bogus
         // (or at least odd) for files other than the base
         // file, so lets leave it out. Temporarily.
         //USRCPOS_srcpos(src) = Get_Srcpos();
         USRCPOS_clear(src);
         char *name2 = 
		  IDENTIFIER_POINTER(TREE_PURPOSE(enum_entry));

         if (tsize == 8) {
	   DST_CONST_VALUE_form_data8(enumerator) = 
	      Get_Integer_Value(TREE_VALUE(enum_entry));
         } else {
	   DST_CONST_VALUE_form_data4(enumerator) = 
		Get_Integer_Value(TREE_VALUE(enum_entry));
         }

	 DST_INFO_IDX ed = DST_mk_enumerator(src,
				name2,
				enumerator);
	 DST_append_child(t_dst_idx,ed);
	
      }


    }
    return dst_idx;
}



#ifdef TARG_ST
// [CL] Try to mimic gcc's behavior with the hope of supporting
// VLA [but so far, I don't know what info to pass to be]
static void Create_bound_info(tree bound, DST_cval_ref *dbound, BOOL *is_cval)
{
  switch (TREE_CODE(bound)) {
  case INTEGER_CST:
    dbound->cval = Get_Integer_Value(bound);
    *is_cval=TRUE;
    break;
  case CONVERT_EXPR:
  case NOP_EXPR:
  case NON_LVALUE_EXPR:
    Create_bound_info(TREE_OPERAND(bound, 0), dbound, is_cval);
    break;
  default:
    dbound->cval = 0;
    DevWarn ("Encountered VLA at line %d: debug info for bound ignored", lineno);
    {
      USRCPOS src;
      USRCPOS_clear(src);
      DST_INFO_IDX bound_var = DST_INVALID_IDX; /* Artificial variable containing the bound */

#if 0
      // [CL] I don't know how to generate valid info in such a case,
      // particularly the 'var' field of DST_mk_variable causes assertion
      // failure in be
      std::string names("int");
      DST_INFO_IDX bound_type = basetypes[names];
      bound_var = DST_mk_variable(
				  src,               // srcpos
				  NULL,              // no name
				  bound_type,
				  0,                 // offset
				  (void*) ST_st_idx(DECL_ST(bound)), // var
				  DST_INVALID_IDX,   // abstract origin
				  FALSE,             // is_declaration
				  FALSE,             // is_automatic
				  FALSE,             // is_external
				  TRUE  );           // is_artificial

      //		DST_INFO_IDX current_scope_idx =
      //		  DST_get_context(DECL_CONTEXT(type_tree));
      //		DST_append_child (current_scope_idx, bound_var);
      DST_append_child (comp_unit_idx, bound_var);
#endif

      dbound->ref = bound_var;
    }
  }
}
#endif

// We have an array
// enter it.
static DST_INFO_IDX
DST_enter_array_type(tree type_tree, TY_IDX ttidx  , TY_IDX idx,INT tsize)
{ 
   DST_INFO_IDX dst_idx = TYPE_DST_IDX(type_tree);

   if(TREE_CODE_CLASS(TREE_CODE(type_tree)) != 't') {
        DevWarn("DST_enter_array_type input not 't' but %c",
                TREE_CODE_CLASS(TREE_CODE(type_tree)));
   }

   if(DST_IS_NULL(dst_idx)) {

      USRCPOS src;
      // For now, the source location appears bogus
      // (or at least odd) for files other than the base
      // file, so lets leave it out. Temporarily.
      //USRCPOS_srcpos(src) = Get_Srcpos();
      USRCPOS_clear(src);

      //if tsize == 0, is incomplete array

      tree elt_tree = TREE_TYPE(type_tree);
      TY_IDX itx = TYPE_TY_IDX(elt_tree);

      // not created yet, so create
      DST_INFO_IDX inner_dst  =
                    Create_DST_type_For_Tree (elt_tree,itx, idx);

      dst_idx = DST_mk_array_type( src,
                            0, // name ?. Nope array types not named: no tag
				// in  C/C++
                           inner_dst, // element type DST_INFO_IDX
                           tsize, // type size
                           DST_INVALID_IDX, // not inlined
                           (tsize == 0)); // pass non-zero if incomplete.
      DST_append_child(comp_unit_idx,dst_idx);

      TYPE_DST_IDX(type_tree) = dst_idx;
#ifdef TARG_ST
      /* CL: Add subrange info */
      {
	tree index_type, min, max;
      
	index_type = TYPE_DOMAIN (type_tree);
	if (index_type) {
	  min = TYPE_MIN_VALUE (index_type);
	  max = TYPE_MAX_VALUE (index_type);

	  if (TREE_TYPE(index_type)) {
	    BOOL is_lb_cval=FALSE;
	    BOOL is_ub_cval=FALSE;

	    DST_cval_ref dmin, dmax;

	    Create_bound_info(min, &dmin, &is_lb_cval);
	    Create_bound_info(max, &dmax, &is_ub_cval);

	    DST_INFO_IDX range_idx =
#ifdef TARG_ST
              /* (cbr) */
	      DST_mk_subrange_type(is_lb_cval, dmin, is_ub_cval, dmax, FALSE, 0LL);
#else
	      DST_mk_subrange_type(is_lb_cval, dmin, is_ub_cval, dmax);
#endif

	    DST_append_child(dst_idx, range_idx);
	  }
	}
      }
#endif
    }
    return dst_idx;
}

// Given a tree with
// POINTER_TYPE, type is OFFSET_TYPE
// we know this is a pointer_to_member tree.
// And we know debug_level >= 2 
// And we know we've not constructed it yet
// (see the caller code)
// What we want to construct here is
// DW_TAG_ptr_to_member_type
// DW_AT_type (of the type of the offset_type)
// DW_AT_containing_type ( of the type of the
//    ttree base-type record-type
static  DST_INFO_IDX
DST_construct_pointer_to_member(tree type_tree)
{
    tree ttree = TREE_TYPE(type_tree);
    FmtAssert(TREE_CODE(type_tree) == POINTER_TYPE
                        && TREE_CODE(ttree) == OFFSET_TYPE,
                          ("DST_construct_pointer_to_member:"
			   "invalid incoming arguments "));
    USRCPOS src;
    // For now, the source location appears bogus
    // (or at least odd) for files other than the base
    // file, so lets leave it out. Temporarily.
    //USRCPOS_srcpos(src) = Get_Srcpos();
    DST_INFO_IDX error_idx = DST_INVALID_INIT;


    USRCPOS_clear(src);

    char *name1 = 0;
    if(DECL_ORIGINAL_TYPE(type_tree)== 0) {
      // not a typedef type.
      name1 =  Get_Name(TREE_TYPE(type_tree));
    } else {
      // is a typedef type
      name1 = Get_Name(DECL_ORIGINAL_TYPE(type_tree));
    }


    tree member_type = TREE_TYPE(ttree);
    if( TREE_CODE(member_type) == ERROR_MARK) {
	return error_idx;
    }
    if(TREE_CODE_CLASS(TREE_CODE(ttree)) != 't') {
	DevWarn("Unexpected tree shape1: pointer_to_member %c\n",
		TREE_CODE_CLASS(TREE_CODE(ttree)));
    }
    TY_IDX midx = Get_TY(member_type);
    TYPE_TY_IDX(member_type) = midx;
    TY_IDX orig_idx = 0;

    DST_INFO_IDX mdst =
                    Create_DST_type_For_Tree(member_type,
                        midx,
			orig_idx);
    TYPE_DST_IDX(type_tree) = mdst;


    tree base_type = TYPE_OFFSET_BASETYPE(ttree);
    if( TREE_CODE(base_type) == ERROR_MARK) {
	return error_idx;
    }
    if(TREE_CODE_CLASS(TREE_CODE(base_type)) != 't') {
	DevWarn("Unexpected tree shape2: pointer_to_member %c\n",
		TREE_CODE_CLASS(TREE_CODE(base_type)));
    }
    TY_IDX container_idx = Get_TY(base_type);


    DST_INFO_IDX container_dst = Create_DST_type_For_Tree(
			base_type,
                        container_idx,
			orig_idx);
    TYPE_DST_IDX(type_tree) = container_dst;
			

    DST_INFO_IDX lidx =
	DST_mk_ptr_to_member_type(src,
		  name1, //name, I expect it to be empty 
		  mdst	, //type of member
		  container_dst); //type of class
    return lidx;

}

      /*--------------------------------------------------
       * Visible routines for creating the DST information
       *--------------------------------------------------*/

// This is in parallel to Create_TY_For_Tree() in 
// tree_symtab.cxx  and must be kept so.
// We cannot use the TY tree as too much information
// is lost that debuggers depend on.
// type_tree_in is void * so 
//
// idx is non-zero only for RECORD and UNION, 
// when there is forward declaration.
//
// ttidx is the type tree Create_TY_For_Tree just created
// for type_tree

DST_INFO_IDX
Create_DST_type_For_Tree (tree type_tree, TY_IDX ttidx  , TY_IDX idx)
{
    
    DST_INFO_IDX dst_idx = DST_INVALID_INIT;

    if(TREE_CODE_CLASS(TREE_CODE(type_tree)) != 't') {
	DevWarn("Create_DST_type_For_Tree input not 't' but %c",
		TREE_CODE_CLASS(TREE_CODE(type_tree)));
	return dst_idx;

    }
	 


    // for typedefs get the information from the base type
    if (TYPE_NAME(type_tree)) {
	    
#ifndef TARG_ST // [CL] do not restrain to union & struct
	if(  idx == 0  &&
	    (TREE_CODE(type_tree) == RECORD_TYPE ||
	     TREE_CODE(type_tree) == UNION_TYPE) &&
	    TREE_CODE(TYPE_NAME(type_tree)) == TYPE_DECL &&
	    TYPE_MAIN_VARIANT(type_tree) != type_tree) {
#else
	  if ( idx == 0 &&
	    TREE_CODE(TYPE_NAME(type_tree)) == TYPE_DECL &&
	    TYPE_MAIN_VARIANT(type_tree) != type_tree) {

	        dst_idx = TYPE_DST_IDX(type_tree);
#endif
		idx = Get_TY (TYPE_MAIN_VARIANT(type_tree));

#ifdef TARG_ST // [CL] create typedef info
		USRCPOS src;
		// For now, the source location appears bogus
		USRCPOS_clear(src);
		DST_INFO_IDX type_idx;

		type_idx = TYPE_DST_IDX(TYPE_MAIN_VARIANT(type_tree));

		if (TYPE_READONLY(type_tree)) {
		  type_idx = DST_mk_const_type(type_idx);
		  DST_append_child(comp_unit_idx,type_idx);
		}
		if (TYPE_VOLATILE(type_tree)) {
		  type_idx = DST_mk_volatile_type(type_idx);
		  DST_append_child(comp_unit_idx,type_idx);
		}

		if (DECL_ORIGINAL_TYPE(TYPE_NAME(type_tree))) {
		  dst_idx = DST_mk_typedef( src,
					    Get_Name(type_tree),
					    type_idx,
					    DST_INVALID_IDX);

		  DST_append_child(comp_unit_idx,dst_idx);
		} else {
		  dst_idx = type_idx;
		}
#endif
		//if (TYPE_READONLY(type_tree))
		//	Set_TY_is_const (idx);
		//if (TYPE_VOLATILE(type_tree))
		//	Set_TY_is_volatile (idx);
		// restrict qualifier not supported by gcc
		//TYPE_TY_IDX(type_tree) = idx;
		//return idx;
		// FIX	      
		//hack so rest of gnu need know nothing of DST 

		return dst_idx;
       } else {
//
       }
    }
    DST_INFO_IDX current_scope_idx = comp_unit_idx;
    if(TYPE_CONTEXT(type_tree)) {
	current_scope_idx = DST_get_context(TYPE_CONTEXT(type_tree));
    }

    char *name1 = Get_Name(type_tree);

    TYPE_ID mtype;
    INT tsize;
    BOOL variable_size = FALSE;
    tree type_size = TYPE_SIZE(type_tree);
    UINT align = TYPE_ALIGN(type_tree) / BITSPERBYTE;
    if (type_size == NULL) {
		// In a typedef'd type 
		// incomplete structs have 0 size
		FmtAssert( TREE_CODE(type_tree) == ARRAY_TYPE 
			|| TREE_CODE(type_tree) == UNION_TYPE
			|| TREE_CODE(type_tree) == RECORD_TYPE
			|| TREE_CODE(type_tree) == ENUMERAL_TYPE
			|| TREE_CODE(type_tree) == VOID_TYPE
			|| TREE_CODE(type_tree) == LANG_TYPE,
			  ("Create_DST_type_For_Tree: type_size NULL for non ARRAY/RECORD"));
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
			tsize = Get_Integer_Value(type_size) / BITSPERBYTE;
   }
   int encoding = 0;
   switch (TREE_CODE(type_tree)) {
   case VOID_TYPE:
   case LANG_TYPE:
		//idx = MTYPE_To_TY (MTYPE_V);	// use predefined type
		break;
   case BOOLEAN_TYPE:
		{
		encoding = DW_ATE_boolean;
		goto common_basetypes;
		}
   case INTEGER_TYPE:
		{
		// enter base type
		if (TREE_UNSIGNED(type_tree)) {
		 encoding = DW_ATE_unsigned;
		} else {
		 encoding = DW_ATE_signed;
		}
		goto common_basetypes;
		}
     case CHAR_TYPE:
		{
		// enter base type
		if (TREE_UNSIGNED(type_tree)) {
		 encoding = DW_ATE_unsigned_char;
		} else {
		 encoding = DW_ATE_signed_char;
		}
		goto common_basetypes;
		}
     case ENUMERAL_TYPE:
		{
		dst_idx = DST_enter_enum(type_tree,ttidx,idx,
                        tsize);

		}
		break;
     case REAL_TYPE:
		{
		// enter base type
		encoding = DW_ATE_float;
		goto common_basetypes;
		}
    case COMPLEX_TYPE:
                // enter base type
                encoding = DW_ATE_complex_float;
    //============
    common_basetypes:
		{
		FmtAssert(name1 != 0,
		   ("name of base type empty, cannot make DST entry!"));

                std::string names(name1);
                DST_Type_Map::iterator p =
                        basetypes.find(names);
                if(p != basetypes.end()) {
                        DST_INFO_IDX t = (*p).second;
                        return t;
                } else {
                       dst_idx = DST_mk_basetype(
                                name1,encoding,tsize);
                       basetypes[names] = dst_idx;
		       DST_append_child(comp_unit_idx,dst_idx);
                }

                }

		break;
    case REFERENCE_TYPE:
	       {
                dst_idx = TYPE_DST_IDX(type_tree);
		if(DST_IS_NULL(dst_idx)) {

		  tree ttree = TREE_TYPE(type_tree);
	          TY_IDX itx = TYPE_TY_IDX(ttree);
	          DST_INFO_IDX inner_dst =
		    Create_DST_type_For_Tree (ttree,itx, idx);


		  // not created yet, so create
		  dst_idx = DST_mk_reference_type(
		       	inner_dst,    // type ptd to
			DW_ADDR_none, // no address class
			tsize);
                                
                  DST_append_child(current_scope_idx,dst_idx);

		  TYPE_DST_IDX(type_tree) = dst_idx;
		}
               }
		break;
    case POINTER_TYPE:
	       {
                dst_idx =  TYPE_DST_IDX(type_tree);
                if(DST_IS_NULL(dst_idx)) {

		  tree ttree = TREE_TYPE(type_tree);
		  if(TREE_CODE(ttree) == OFFSET_TYPE) {
		     dst_idx = DST_construct_pointer_to_member(type_tree);
		  } else {

	              TY_IDX itx = TYPE_TY_IDX(ttree);
	              DST_INFO_IDX inner_dst =
		         Create_DST_type_For_Tree (ttree,itx, idx);

                      // not created yet, so create


		      dst_idx = DST_mk_pointer_type(
		       	inner_dst,    // type ptd to
			DW_ADDR_none, // no address class
			tsize);
		  }
                                
                  DST_append_child(current_scope_idx,dst_idx);

		  TYPE_DST_IDX(type_tree) = dst_idx;
                 }
               }
	       break;
    case OFFSET_TYPE:

		//  Do nothing. We should not get here.
		break;

    case ARRAY_TYPE:
		
	       {
                dst_idx = DST_enter_array_type(type_tree, 
			ttidx, idx, tsize);
	       }
	       break;
    case RECORD_TYPE:
    case UNION_TYPE:
		{
		dst_idx = DST_enter_struct_union(type_tree,ttidx,idx,
			tsize);
		}
		break;
    case METHOD_TYPE:
		{
		//DevWarn ("Encountered METHOD_TYPE at line %d", lineno);
		}
		break;

    case FUNCTION_TYPE:
		{	// new scope for local vars
#if 0
		tree arg;
		INT32 num_args;
		TY &ty = New_TY (idx);
		TY_Init (ty, 0, KIND_FUNCTION, MTYPE_UNKNOWN, NULL); 
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
#endif
		} // end FUNCTION_TYPE scope
#ifdef TARG_ST  // [CL] generate type info for function prototype
		// useful for record/union fields that are pointers
		// to function. Generate corresponding param types
		{
		  tree ttree = TREE_TYPE(type_tree);
	          TY_IDX itx = TYPE_TY_IDX(ttree);
		  DST_INFO_IDX ret_idx = Create_DST_type_For_Tree(ttree, idx, idx);

		  USRCPOS src;
		  // For now, the source location appears bogus
		  USRCPOS_clear(src);

		  BOOL isprototyped = FALSE;

		  if (TREE_TYPE(type_tree) &&
		      TYPE_ARG_TYPES(type_tree)) {
		    isprototyped = TRUE;
		  }
	    
		  dst_idx = DST_mk_subroutine_type(src,
						   NULL,
						   ret_idx,
						   DST_INVALID_IDX,
						   isprototyped);

		  DST_append_child(current_scope_idx,dst_idx);

		  if (isprototyped) {
		    tree arg;
		    DST_INFO_IDX param_idx;

		    for (arg = TYPE_ARG_TYPES(type_tree);
			 arg;
			 arg = TREE_CHAIN(arg)) {

		      // [CL] parameter list finished by a 'void' node
		      // means this is not a vararg function.
		      // should not generate a parameter
		      if ( (TREE_CHAIN(arg) == NULL) &&
			   (TREE_VALUE(arg) == void_type_node) ) {
			break;
		      }

		      DST_INFO_IDX arg_idx = TYPE_DST_IDX(TREE_VALUE(arg));

		      param_idx = DST_mk_formal_parameter(src,
							  NULL,
							  arg_idx,
							  NULL,
							  DST_INVALID_IDX,
							  DST_INVALID_IDX,
							  FALSE,
							  FALSE,
							  FALSE,
							  TRUE);

		      DST_append_child(dst_idx,param_idx);
		    }

		    // Generate info for elipsis
		    if (!arg) {
		      param_idx = DST_mk_unspecified_parameters(src,
								DST_INVALID_IDX);
		      DST_append_child(dst_idx,param_idx);

		    }
		  }
		}

		TYPE_DST_IDX(type_tree) = dst_idx;
#endif
		break;
    default:

		FmtAssert(FALSE, ("Get_TY unexpected tree_type"));
    }
    //if (TYPE_READONLY(type_tree))
//		Set_TY_is_const (idx);
 //   if (TYPE_VOLATILE(type_tree))
//		Set_TY_is_volatile (idx);
    // restrict qualifier not supported by gcc
 //   TYPE_TY_IDX(type_tree) = idx;
	
    
    return dst_idx;
}

extern DST_INFO_IDX
Create_DST_decl_For_Tree(
        tree decl, ST* var_st)
{



  DST_INFO_IDX cur_idx = DECL_DST_IDX(decl);
  if(!DST_IS_NULL(cur_idx)) {

	// Already processed. Do not redo!
	return cur_idx;
  }

  DST_INFO_IDX dst_idx = DST_INVALID_INIT;

  //if (TREE_CODE(decl) != VAR_DECL) return var_idx;

  if (DECL_IGNORED_P(decl))    {
  	return cur_idx;
  }


  // if tests  for locals and returns if local 
  // if (DECL_CONTEXT(decl) != 0)  return var_idx;


  // If this is simply an extern decl (not an instance)
  // (and the context is not an inner lexical block,
  // where we could be hiding an outer decl with the same name so
  // need a decl here) 
  // then we can just do nothing. 
  // externs get debug info at the point of def.
  if (TREE_CODE(decl) == VAR_DECL && 
		 DECL_EXTERNAL(decl) && !DECL_COMMON(decl)) {
      return cur_idx;
  }

#ifdef TARG_ST
  // [CL] preliminary support for anonymous unions
  if (TREE_CODE(decl) == VAR_DECL && DECL_EXTERNAL(decl)) {
	return cur_idx ;
  }
#else
  // For now ignore plain declarations?
  // till we get more working
  if (TREE_CODE(decl) == VAR_DECL && (!TREE_STATIC(decl)
		 && !DECL_COMMON(decl))) {
	return cur_idx ;
  }
#endif

  // is something that we want to put in DST
  // (a var defined in this file, or a type).
  // The following eliminates locals 
  //if(!TREE_PERMANENT(decl)) {
  //	// !in permanent obstack 
  //	return cur_idx ;
  // }

  int tcode = TREE_CODE(decl);
  switch(tcode) {
  case VAR_DECL: {
      //Get_ST(decl);
      dst_idx = DST_Create_var(var_st,decl);
#ifdef TARG_ST // [CL] handle anonymous union
    if (ANON_AGGR_TYPE_P (TREE_TYPE(decl))) {
      for (tree mydecl = DECL_ANON_UNION_ELEMS(decl); mydecl != NULL; mydecl = TREE_CHAIN(mydecl)) {
	tree field_decl = TREE_VALUE(mydecl);
	(void)Create_ST_For_Tree (field_decl);
	// After creation of each field, make the location point to
	// the main ST by fixing what is setup in
	// dwarf_DST_producer.cxx:mk_variable()
	DST_INFO *info = DST_INFO_IDX_TO_PTR(DECL_DST_IDX(field_decl));
	DST_ASSOC_INFO_fe_ptr(
		DST_VARIABLE_def_st(
		    DST_ATTR_IDX_TO_PTR(
		        DST_INFO_attributes(info), DST_VARIABLE)))
	                    = (void*) ST_st_idx(var_st);
	DST_SET_assoc_fe(DST_INFO_flag(info));
      }
    }
#endif
      }
      break;
  case TYPE_DECL: {
      //Get_ST(decl);
      dst_idx = DST_Create_type(var_st,decl);
      }
      break;
  case PARM_DECL: {
      //Get_ST(decl);
      dst_idx = DST_Create_Parmvar(var_st,decl);
      }
      break;
  default: {
      }
      break;
  }
  return dst_idx;
}



static DST_INFO_IDX
DST_Create_type(ST *typ_decl, tree decl)
{
    USRCPOS src;
    // For now, the source location appears bogus
    // (or at least odd) for files other than the base
    // file, so lets leave it out. Temporarily.
    //USRCPOS_srcpos(src) = Get_Srcpos();


    USRCPOS_clear(src);
    DST_INFO_IDX dst_idx = DST_INVALID_INIT;
    



    char *name1 = 0; 
    if(DECL_ORIGINAL_TYPE(decl)== 0) {
      // not a typedef type.
      name1 =  Get_Name(TREE_TYPE(decl));
    } else {
      // is a typedef type
      name1 = Get_Name(DECL_ORIGINAL_TYPE(decl));
    }
  
    // FIX look in various contexts to find known types ?
    // It is not true base types that are the problem, it
    // is typedefs creating 'new types'.
    std::string names(name1);
    DST_Type_Map::iterator p =
                        basetypes.find(names);
    if(p != basetypes.end()) {
                        //Yep, already known.
        DST_INFO_IDX t = (*p).second;
        // hack so rest of gnu need know nothing of DST
        return t;
    } 

    DST_INFO_IDX current_scope_idx =
         DST_get_context(DECL_CONTEXT(decl));

    // Nope, something new. make a typedef entry.
    // First, ensure underlying type is set up.
    tree undt = DECL_RESULT(decl);
    TY_IDX base;

    if(!undt) {
	DevWarn ("DST no result type for typedef decl: impossible");
	return DST_INVALID_IDX;
    } 
    // Do for side effect of creating DST for base type.
    // ie, in typedef int a, ensure int is there.
    base = Get_TY(undt);
    DST_INFO_IDX dst = 
	Create_DST_type_For_Tree(undt,base,
		/* struct/union fwd decl TY_IDX=*/ 0);

    dst_idx = DST_mk_typedef( src,
                          name1, // new type name we are defining
                          dst, // type of typedef
                          DST_INVALID_IDX);
    DST_append_child(current_scope_idx,dst_idx);
    // and add the new type to our base types map
    basetypes[names] = dst_idx;
    TYPE_DST_IDX(undt) = dst_idx;

    return dst_idx;
}

static DST_INFO_IDX
DST_Create_Parmvar(ST *var_st, tree param)
{
    USRCPOS src;
    // For now, the source location appears bogus
    // (or at least odd) for files other than the base
    // file, so lets leave it out. Temporarily.
    //USRCPOS_srcpos(src) = Get_Srcpos();

    USRCPOS_clear(src);


    DST_INFO_IDX type_idx = DST_INVALID_INIT;

    ST * st = Get_ST(param); // As a side effect,
                //parameters and the types are set in ST.
		// and in the DST
    tree type = TREE_TYPE(param);

    TY_IDX ty_idx = Get_TY(type); 

    DST_INFO_IDX dtype = DECL_DST_IDX(param);
    return dtype;
}

// Look thru the members of myrecord, looking for
// static vars. If we find one with that
// linkage name, return the DST_INFO_IDX.
// Return the DST_INVALID_INIT value if such a member
// not found.
//
static DST_INFO_IDX
DST_find_class_member(char * linkage_name_in, tree myrecord)
{
    DST_INFO_IDX return_member_dst = DST_INVALID_INIT;
    if(!linkage_name_in)
		return return_member_dst;
    tree field = TREE_PURPOSE(myrecord);
    for( ; field ; field = TREE_CHAIN(field) )
    {
        if(TREE_CODE(field) == VAR_DECL) {

             char * linkage_name =
                        IDENTIFIER_POINTER(DECL_ASSEMBLER_NAME (field));
	     if(linkage_name && 
			(strcmp(linkage_name,  linkage_name_in) == 0)) {
		return_member_dst =  DECL_DST_FIELD_IDX(field);
                return return_member_dst;
	     }
        }
    }
    tree methods = TYPE_METHODS(myrecord);
    for  ( ; methods != NULL_TREE; methods = TREE_CHAIN(methods)) {
        if(TREE_CODE(methods) == FUNCTION_DECL ) {
#ifndef TARG_ST
	  // [CL]
           if ( DECL_ARTIFICIAL(methods)) {
             // compiler generated methods are not interesting.
             // We want only ones user coded.
             continue;
           } else {
#endif
	     char * linkage_name = 
		   IDENTIFIER_POINTER(DECL_ASSEMBLER_NAME (methods));
	     if(linkage_name &&
			(strcmp(linkage_name,  linkage_name_in) == 0)) {
		return_member_dst =  DECL_DST_FIELD_IDX(methods);
                return return_member_dst;
#ifndef TARG_ST
	     }
#endif
           }
        }
    }


    return return_member_dst;
}

static DST_INFO_IDX
DST_Create_var(ST *var_st, tree decl)
{
    USRCPOS src;
    // For now, the source location appears bogus
    // (or at least odd) for files other than the base
    // file, so lets leave it out. Temporarily.
    //USRCPOS_srcpos(src) = Get_Srcpos();



    int is_external = TREE_PUBLIC(decl); // true if var has external linkage
    int external_decl = DECL_EXTERNAL(decl); // true if var undefined
    tree context = DECL_CONTEXT(decl);

    // If it is external, 
    // then unless it hides some local it need not
    // be in the dwarf at all.
    if(external_decl) {
     if( context && TREE_CODE(context) == RECORD_TYPE) {
	// Is C++ class function mention, not a def.
	DST_INFO_IDX no_info = DST_INVALID_INIT;
	return no_info;
     }
     // FIXME: does not remove other externals...

    }
    char *field_name = Get_Name(decl);
#ifdef TARG_ST
    // [CL] preliminary support for anonymous unions
    char *linkage_name = NULL;
    if (strlen(field_name) > 0) {
      linkage_name = IDENTIFIER_POINTER(DECL_ASSEMBLER_NAME (decl));
    }
#else
    char *linkage_name = 	
		IDENTIFIER_POINTER(DECL_ASSEMBLER_NAME (decl));
#endif

    int class_var_found_member = 0;
    DST_INFO_IDX class_var_idx = DST_INVALID_INIT;
    if(context && TREE_CODE(context) == RECORD_TYPE) {
	/*look for  static data member decl*/
	class_var_idx =
		DST_find_class_member(linkage_name, context);
	if(!DST_IS_NULL(class_var_idx)) {
		class_var_found_member = 1;
		field_name = 0; // we will use DW_AT_specification
		   //to get name in debugger
		   //So do not assign name here.
	}
	// Field name should now have the class name and ::
	// prepended, per dwarf2
	// Save_Str2 can glob 2 pieces together.
	char *classname = Get_Name(context);
	if(classname && !class_var_found_member) {
	   int len = strlen(classname);
#ifdef TARG_ST
           /* (cbr) dummy_new_mempool not yet set */
	   char newname [len+5 
			/* 5 makes room for :: and null char */];
#else
	   char* newname = new char[len+5 
			/* 5 makes room for :: and null char */];
#endif


	   // no 0 check: let it crash to get signal handler msg.

	   strcpy(newname,classname);
	   strcpy(newname+len,"::");
	   if(field_name) {
	    // We do not need the string in a stable storage
	    // area, but this is a convenient way to
	    // concatenate strings and avoid a memory leak.
	    field_name = Index_To_Str(Save_Str2(newname,field_name));
	   }

#ifndef TARG_ST
           /* (cbr) dummy_new_mempool not yet set */
	   delete [] newname;
#endif
	}


    }

    USRCPOS_clear(src);
    DST_INFO_IDX dst = DST_INVALID_INIT;

    DST_INFO_IDX type = TYPE_DST_IDX(TREE_TYPE(decl));


#ifdef TARG_ST
	// [CL] Handle const qualifier, which is attached to the
	// decl tree, not to the type tree
	if (TREE_READONLY(decl)) {
	  type = DST_mk_const_type(type);
	  DST_append_child(comp_unit_idx,type);
	}
	// [CL] Handle volatile qualifier
	if (TREE_THIS_VOLATILE(decl)) {
	  type = DST_mk_volatile_type(type);
	  DST_append_child(comp_unit_idx,type);
	}
#endif

    dst = DST_mk_variable(
        src,                    // srcpos
        field_name,
        type,    // user typed type name here (typedef type perhaps).
	0,  // offset (fortran uses non zero )
        (void*) ST_st_idx(var_st), // underlying type here, not typedef.
        DST_INVALID_IDX,        // abstract origin
        external_decl,          // is_declaration
        FALSE,                  // is_automatic
        is_external,  // is_external
#ifdef TARG_ST
	// [CL]
	DECL_ARTIFICIAL(decl)  ); // is_artificial
#else
	FALSE  ); // is_artificial
#endif

    DST_INFO_IDX current_scope_idx =
         DST_get_context(DECL_CONTEXT(decl));


    // At present, DST producer in common/com does not
    // allow setting linkage name

    //if(linkage_name && basename && strcmp(linkage_name,basename)) {
      // we have a genuine linkage name.
     // DST_add_linkage_name_to_subprogram(dst, linkage_name);
    //}
#ifdef TARG_ST
// [CL] dont output invisible names
    if(field_name && linkage_name && strcmp(field_name, linkage_name) &&
       TREE_PUBLIC(decl) && !DECL_ABSTRACT(decl) ) {
      DST_add_linkage_name_to_variable(dst, linkage_name);
    }
#endif




    // If this is a def of a static var member, want DW_AT_specification
    // added to point to class mem
    if(class_var_found_member) {
       DST_add_specification_to_variable(dst, class_var_idx);
    }


    DST_append_child (current_scope_idx, dst);
    return dst;

}

// For each parameter decl,
// create a record and attach to the function
// it belongs to.
// is_declaration_only stuff does NOT work: it was
// for member functions, which we don't deal with. 
// see DST_enter_member_function() comments
// 
static void
DST_enter_param_vars(tree fndecl,
  DST_INFO_IDX parent_idx,
  tree parameter_list,
  int is_abstract_root,
  int is_declaration_only)
{
    USRCPOS src;
    USRCPOS_srcpos(src) = Get_Srcpos();

   
    tree pdecl = parameter_list;
#ifdef TARG_ST // [CL] add support for elipsis
    tree ptype = TYPE_ARG_TYPES(TREE_TYPE(fndecl));
#endif

    for(  ; pdecl; pdecl = TREE_CHAIN(pdecl)) {
      if(TREE_CODE_CLASS(TREE_CODE(pdecl)) != 'd') {
	DevWarn("parameter node not decl! tree node code %d ",
		TREE_CODE(pdecl));
      } else {
        DST_INFO_IDX param_idx = DST_INVALID_INIT;
        DST_INFO_IDX type_idx = DST_INVALID_INIT;
        BOOL is_artificial = DECL_ARTIFICIAL(pdecl);
	int decl_to_be_restored = 0;
	int type_to_be_restored = 0;
	tree type = TREE_TYPE(pdecl);

	
	DST_INFO_IDX save_type_idx = TYPE_DST_IDX(type);
	DST_INFO_IDX save_decl_idx = DECL_DST_IDX(pdecl);

	ST *st = 0;
	if(!is_declaration_only) {
	  st = Get_ST(pdecl); // As a side effect,
		// parameters and the types are set in ST
		// and dst
		// and we do not want ST set up in this case.

	}
	


        TY_IDX ty_idx = Get_TY(type);


	type_idx = TYPE_DST_IDX(type);

#ifdef TARG_ST
	// [CL] Handle const qualifier, which is attached to the
	// pdecl tree, not to the type tree
	if (TREE_READONLY(pdecl)) {
	  type_idx = DST_mk_const_type(type_idx);
	  DST_append_child(comp_unit_idx,type_idx);
	}
	// [CL] Handle volatile qualifier
	if (TREE_THIS_VOLATILE(pdecl)) {
	  type_idx = DST_mk_volatile_type(type_idx);
	  DST_append_child(comp_unit_idx,type_idx);
	}
#endif

	char *name = Get_Name(pdecl);

	DST_INFO_IDX initinfo = DST_INVALID_IDX;
        //tree initl = DECL_INITIAL(pdecl);
	//if(initl) {  FIXME: optional params
		//  Get_TY(initl); // As a side effect,
			// set types, etc
	  //initinfo = DECL_DST_IDX(initl);
	//}

	ST_IDX loc = (is_abstract_root || is_declaration_only)? 
		ST_IDX_ZERO: ST_st_idx(st);
	DST_INFO_IDX aroot = DST_INVALID_IDX;
	if(!is_abstract_root && !is_declaration_only) {
	  //. get the abstract root idx if it exists
	  aroot = DECL_DST_ABSTRACT_ROOT_IDX(pdecl);
	}


	param_idx = DST_mk_formal_parameter(
		src,
		name,
		type_idx,
		(void* )loc, // So backend can get location.
			// For a formal in abstract root
			// or a plain declaration (no def)
			// there is no location.
		aroot, // If inlined, and this
			// is concrete instance,pass idx of formal
			// in the abstract root
		initinfo, // C++ default value (of optional param)
		0?TRUE:FALSE, // true if C++ optional param // FIXME
	        FALSE, // DW_AT_variable_parameter not set in C++.
		is_artificial,
		is_declaration_only);

       // producer routines thinks we will set pc to fe ptr initially
       DST_RESET_assoc_fe (DST_INFO_flag(DST_INFO_IDX_TO_PTR(param_idx)));

	// The abstract root and
	// each concrete root have
	// distinct values.
	// The concrete root values need not be recorded, and
	// they will differ (no one unique value anyway) based
        // off the same pdecl node (????). FIXME 
	if(is_abstract_root) {
	   DECL_DST_ABSTRACT_ROOT_IDX(pdecl) = param_idx;
	} 
	if (!is_declaration_only && !is_abstract_root) {
	    DECL_DST_IDX(pdecl) = param_idx;
	}

	DST_append_child(parent_idx,param_idx);
      }
#ifdef TARG_ST
      // [CL] add support for elipsis
      ptype = TREE_CHAIN(ptype);
#endif
    }
#ifdef TARG_ST
    // Generate info for elipsis
    if (!ptype) {
      DST_INFO_IDX param_idx;
      param_idx = DST_mk_unspecified_parameters(src,
						DST_INVALID_IDX);
      DST_append_child(parent_idx,param_idx);
      
    }
#endif
}

//
// fndecl can be 0 if this is an asm stmt treated as a function, 
// rather than being a true function.
DST_INFO_IDX
DST_Create_Subprogram (ST *func_st,tree fndecl)
{
    USRCPOS src;
#ifdef TARG_ST // [CL] so that Get_Srcpos returns something coherent
    input_filename = DECL_SOURCE_FILE(fndecl);
#endif

    USRCPOS_srcpos(src) = Get_Srcpos();
    DST_INFO_IDX dst = DST_INVALID_INIT;
    DST_INFO_IDX ret_dst = DST_INVALID_IDX;
                                     
    DST_INFO_IDX current_scope_idx = fndecl ?
#ifndef TARG_ST // [CL]
      	(DST_get_context(TYPE_CONTEXT(fndecl))): 
#else
      	(DST_get_context(DECL_CONTEXT(fndecl))): 
#endif
	comp_unit_idx;

    BOOL is_prototyped = FALSE;

    if(Debug_Level >= 2 && fndecl) {

#ifndef TARG_ST
	tree resdecl = DECL_RESULT(fndecl);
#else // [CL] this is the actual type of the result
        tree resdecl = TREE_TYPE(fndecl);
#endif
	tree restype = 0;
	if( resdecl) {
	   restype = TREE_TYPE(resdecl);
	}
	if(restype) {
	 TY_IDX itx = Get_TY(restype);
	 ret_dst = TYPE_DST_IDX(restype);
	}

        tree type = TREE_TYPE(fndecl);
	if(type) {
	  tree arg_types = TYPE_ARG_TYPES(type);
	  if(arg_types) {
		is_prototyped = TRUE;
	  }
	}
    }


#ifdef TARG_ST
    // [CL] get the 'true' function name
    char * basename = xstrdup((char*)lang_decl_name(fndecl, 0));
 #else
    char * basename = 
	IDENTIFIER_POINTER (DECL_NAME (fndecl));
#endif
    char * linkage_name = ST_name(func_st);

    char * funcname = basename;
    int is_abstract_root = 0;
    DST_INFO_IDX class_func_idx = DST_INVALID_INIT;
    int class_func_found_member = 0;
    tree context = DECL_CONTEXT(fndecl);
    if(context && TREE_CODE(context) == RECORD_TYPE) {
        /*look for  static data member decl*/
        class_func_idx =
                DST_find_class_member(linkage_name, context);
#if 0
	// [CL] keep the function name, otherwise GDB complains.
	// Actually, it seems that such info should be linked to the
	// proper scope, but this is not done in cgdwarf.cxx
        if(!DST_IS_NULL(class_func_idx)) {
                class_func_found_member = 1;
                funcname = 0; // we will use DW_AT_specification
                   //to get name in debugger
                   //So do not assign name here.
 		   // and no src position: leave that to member
		 USRCPOS_clear(src);
        }
#endif
        // Field name should now have the class name and ::
        // prepended, per dwarf2
        // Save_Str2 can glob 2 pieces together.
        char *classname = Get_Name(context);
        if(classname && !class_func_found_member) {
           int len = strlen(classname);
#ifdef TARG_ST
           /* (cbr) dummy_new_mempool not yet set */
           char newname [len+5
                        /* 5 makes room for :: and null char */];
#else
           char* newname = new char[len+5
                        /* 5 makes room for :: and null char */];
#endif

           // no 0 check: let it crash to get signal handler msg.

           strcpy(newname,classname);
           strcpy(newname+len,"::");
           if(funcname) {
            // We do not need the string in a stable storage
            // area, but this is a convenient way to
            // concatenate strings and avoid a memory leak.
            funcname = Index_To_Str(Save_Str2(newname,funcname));
           }

#ifndef TARG_ST
           /* (cbr) dummy_new_mempool not yet set */
           delete [] newname;
#endif
        }
    }

    linkage_name = ST_name(func_st);
    ST_IDX fstidx = ST_st_idx(func_st);

#ifdef TARG_ST
    // [CL]
    DST_virtuality  virtuality = 
		 DECL_PURE_VIRTUAL_P(fndecl)?
			 DW_VIRTUALITY_pure_virtual
		 : DECL_VIRTUAL_P(fndecl)?
			DW_VIRTUALITY_virtual : DW_VIRTUALITY_none;

    DST_vtable_elem_location vtable_elem_location;
    /* (cbr) elem loc must be an integer */
    if (DECL_VINDEX(fndecl) && TREE_CODE (DECL_VINDEX(fndecl)) == INTEGER_CST) {
        vtable_elem_location = Get_Integer_Value(DECL_VINDEX(fndecl));
    } else {
        vtable_elem_location = 0;
    }
#endif

    dst = DST_mk_subprogram(
        src,			// srcpos
        funcname,
        ret_dst,        	// return type
        DST_INVALID_IDX,        // Index to alias for weak is set later
        (void*) fstidx,         // index to fe routine for st_idx
        DW_INL_not_inlined,     // applies to C++
#ifdef TARG_ST
	// [CL]
	virtuality,
	vtable_elem_location,
#else
        0,                      // vtable_elem_location
        DW_VIRTUALITY_none,     // applies to C++
#endif
        FALSE,                  // is_declaration
        is_prototyped,           // 
#ifdef TARG_ST
	// [CL]
        ! ST_is_export_local(func_st),  // is_external
	DECL_ARTIFICIAL(fndecl) );
#else
        ! ST_is_export_local(func_st) );  // is_external
#endif
    // producer routines think we will set pc to fe ptr initially
    DST_RESET_assoc_fe (DST_INFO_flag(DST_INFO_IDX_TO_PTR(dst)));
    DST_append_child (current_scope_idx, dst);

    if(class_func_found_member) {
       DST_add_specification_to_subprogram(dst, class_func_idx);
    }

    if(linkage_name && funcname && strcmp(linkage_name,funcname)) {
      // we have a genuine linkage name.
      DST_add_linkage_name_to_subprogram(dst, linkage_name);
    }
    if( !DST_IS_NULL(DECL_DST_ABSTRACT_ROOT_IDX(fndecl))) {
	// FIXME need to record abstract root!
	DevWarn("Concrete function instance has abstract root! %s\n",
		basename);
    }


    if(fndecl) {
	DECL_DST_IDX(fndecl) =  dst;
    }

    // Now we create the argument info itself, relying
    // on the is_prototyped flag above to let us know if
    // we really should do this.
    if(is_prototyped) {
       tree parms = DECL_ARGUMENTS(fndecl);
       if(!parms) {
	  // Normal function, no paramaters: in C++ this means
          // it has no arguments at all.
	  // And no dwarf should be emitted.
          // DevWarn("impossible arg decls -- is %s empty?",basename?basename:"");
       } else {
	   DST_enter_param_vars(fndecl, 
			dst,
			parms, 
			/* abstract_root = */ 0,
			/* declaration only = */ 0  );
       }

    }

    return dst;
}

DST_INFO_IDX
DST_Get_Comp_Unit (void)
{
	return comp_unit_idx;
}

/* Look in "lexical.h" at push_input_stack() to find out about directories
 * and search paths.
*/
void
DST_build(int num_copts, /* Number of options passed to fec(c) */
	  char *copts[]) /* The array of option strings passed to fec(c) */
{
   char         *src_path, *comp_info;

   dst_initialized = TRUE;

   /* Initiate the memory system */
   DST_Init (NULL, 0);

   /* Enter the file-name as the first one in the file_list 
    * (DW_AT_name => src_path).  In the case that src_path should not
    * be an absolute path, we only need to eliminate the call to 
    * Make_Absolute_Path and we will get a path relative to the cwd.
    */

   if (Orig_Src_File_Name != NULL)
   {
     src_path = Orig_Src_File_Name;
   }
/*
   else {
     (void)DST_FILE_NUMBER(il_header.primary_source_file);
     src_path = il_header.primary_source_file->file_name;
   }
*/

   /* Get the DW_AT_comp_dir attribute (current_host_dir) */
   if (Debug_Level > 0)
   {
      int host_name_length = 0;
      
      current_host_dir = &cwd_buffer[0];
      if (gethostname(current_host_dir, MAXHOSTNAMELEN) == 0)
      {
	 /* Host name is ok */
	 host_name_length = strlen(current_host_dir);
	 if(strchr(current_host_dir,'.')) {
	    // If hostname is already a FQDN (fully qualified
	    // domain name) don't add the domain again...
	    // Somehow.
	 } else {
	   current_host_dir[host_name_length] = '.';
	   if (getdomainname(&current_host_dir[host_name_length+1], 
			   MAXHOSTNAMELEN-host_name_length) == 0)
	   {
	    /* Domain name is ok */
	    host_name_length += strlen(&current_host_dir[host_name_length]);
	   }
         }

      }
      current_host_dir[host_name_length++] = ':';  /* Prefix cwd with ':' */
      current_working_dir = &cwd_buffer[host_name_length];
   }
   else /* No debugging */
   {
      current_host_dir = NULL;
      current_working_dir = &cwd_buffer[0];
   }
   strcpy(current_working_dir, Get_Current_Working_Directory());
   if (current_working_dir == NULL) {
      perror("getcwd");
      exit(2);
   }

   /* Get the AT_producer attribute! */
   comp_info = DST_get_command_line_options(num_copts, copts);

   {
      comp_unit_idx = DST_mk_compile_unit(src_path,
					  current_host_dir,
					  comp_info, 
					  DW_LANG_C_plus_plus,
					  DW_ID_case_sensitive);
   }

   free(comp_info);

   WFE_Set_Line_And_File (0, Orig_Src_File_Name);
}

void
WFE_Set_Line_And_File (UINT line, const char* f)
{
	if (!dst_initialized) return;

        // We aren't really modifying f, this is just so we can
        // call legacy code.
        char* file = const_cast<char*>(f);

	// split file into directory path and file name
	char *dir;
	char *file_name = drop_path(file);;
	char buf[256];
	if (file_name == file) {
		// no path
		dir = current_working_dir;
	}
	else if (strncmp(file, "./", 2) == 0) {
		// current dir
		dir = current_working_dir;
	}
	else {
		// copy specified path
		strcpy (buf, file);
		dir = drop_path(buf);	// points inside string, after slash
		--dir;			// points before slash
		*dir = '\0';		// terminate path string
		dir = buf;
	}
	current_dir = Get_Dir_Dst_Info (dir);
	current_file = Get_File_Dst_Info (file_name, current_dir);
}
