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


#include <stdio.h>
#include <stdlib.h>
#include "W_alloca.h"
#include "basic.h"
#include "string_utils.h"
#include "objects.h"
#include "option_names.h"
#include "options.h"
#include "option_seen.h"
#include "opt_actions.h"
#include "get_options.h"
#include "errors.h"
#include "lang_defs.h"
#include "file_names.h"
#include "file_utils.h"
#include "run.h"

#ifdef TARG_ST
/*
 * TARG_ST Specific comments:
 * [CG] add_script_file(), append_script_files_to_list(): 
 *      handling of -T<file> options must use these interface.
 *      The script_files list maintains the user specified script files
 *      added by add_script_file(). In run_ld() processing, the default
 *	script file or the user list is appended at the end of the ld
 *	arguments list.
 */
#endif

string_list_t *objects;
string_list_t *lib_objects;
static string_list_t *cxx_prelinker_objects;
static string_list_t *ar_objects; 
static string_list_t *library_dirs;
#ifdef TARG_ST
/* [CG]: See comment above. */
string_list_t *script_files;
/* [TB]: Add objects list for ld -r and final ld phase for -Wy,XXX and
   -Wz,XXX options */
string_list_t *objects_ld_r;
string_list_t *objects_ld_final;
#endif

extern void
init_objects (void)
{
 	objects = init_string_list();
#ifdef TARG_ST
	/* [TB]: Add objects list for ld -r and final ld phase for -Wy,XXX and
	   -Wz,XXX options */
	objects_ld_r = init_string_list();
	objects_ld_final = init_string_list();
#endif
 	lib_objects = init_string_list();
 	cxx_prelinker_objects = init_string_list();
 	ar_objects = init_string_list();
	library_dirs = init_string_list();
#ifdef TARG_ST
	/* [CG]: See comment above. */
 	script_files = init_string_list();
#endif
}



/* whether option is an object or not */
extern boolean
is_object_option (int flag)
{
	switch (flag) {
	case O_object:
	case O_objectlist:
	case O_l:
	case O_all:
	case O_notall:
        case O__whole_archive:
        case O__no_whole_archive:
	case O_none:
	case O_exports:
	case O_hides:
	case O_ignore_minor:
	case O_require_minor:
	case O_exact_version:
	case O_ignore_version:
	case O_exclude:
	case O_delay_load:
	case O_force_load:
		return TRUE;
	default:
		return FALSE;
	}
}

/* library list options get put in object list,
 * so order w.r.t. libraries is preserved. */
extern void
add_object (int flag, string arg)
{
    /* cxx_prelinker_object_list contains real objects, -objectlist flags. */
	switch (flag) {
	case O_l:
		/* xpg fort77 has weird rule about putting all libs after objects */
		if (xpg_flag && invoked_lang == L_f77) {
			add_string(lib_objects, concat_strings("-l",arg));
		} else {
			add_string(objects, concat_strings("-l",arg));
		}
		if (invoked_lang == L_CC) {
		    add_string(cxx_prelinker_objects,concat_strings("-l",arg));
		}

		/* when -lm, implicitly add extra math libraries */
		if (same_string(arg, "m")) {
			/* add -lmv -lmsgi */
			if (xpg_flag && invoked_lang == L_f77) {
				add_string(lib_objects, "-lmv");
				add_string(lib_objects, "-lmsgi");
			} else {
#ifndef TARG_ST
				add_string(objects, "-lmv");
				add_string(objects, "-lmsgi");
#endif
			}
			if (invoked_lang == L_CC) {
			    add_string(cxx_prelinker_objects, "-lmv");
			    add_string(cxx_prelinker_objects, "-lmsgi");
			}
		}
		break;
	case O_objectlist:
		add_multi_strings(objects, concat_strings("-objectlist ",arg));
		if (invoked_lang == L_CC) {
		    add_string(cxx_prelinker_objects,
				concat_strings("-YO=",arg));
		}
		break;
	case O_object:
		if (dashdash_flag && arg[0] == '-') {
		  add_string(objects,"--");
		  dashdash_flag = 1;
		}
		add_string(objects, arg);
		if (invoked_lang == L_CC) {
		    add_string(cxx_prelinker_objects, arg);
		}

		break;
	case O_all:
          /* O_all and O_notall are special cases.  They're object
             options, but (at least for the gnu linker) we don't 
             pass them as-is to the linker.  For normal non-object
             options this would be handled automatically. */
#if defined(linux) || defined (TARG_ST)
          add_string(objects, get_option_name(O__whole_archive));
          break;
#endif
	case O_notall:
#if defined(linux) || defined (TARG_ST)
          add_string(objects, get_option_name(O__no_whole_archive));
          break;
#endif
	case O_none:
	case O_exports:
	case O_hides:
	case O_ignore_minor:
	case O_require_minor:
	case O_exact_version:
	case O_ignore_version:
		add_string(objects, get_option_name(flag));
		break;
	case O_delay_load:
	case O_force_load:
		add_string(objects, get_option_name(flag));
		break;
	case O_exclude:
		add_string(objects, get_option_name(flag));
		add_string(objects, arg);
		break;
	default:
		internal_error("add_object called with not-an-object");
	}
}

/* append object files to the ar_objects list. */
extern void
add_ar_objects (string arg)
{
    add_string(ar_objects, arg);
}

/* append objects to end of list */
extern void
append_objects_to_list (string_list_t *list)
{
	append_string_lists (list, objects);
	if (xpg_flag && invoked_lang == L_f77) {
		append_string_lists (list, lib_objects);
	}
}

/* append cxx_prelinker_objects to end of list */
extern void
append_cxx_prelinker_objects_to_list (string_list_t *list)
{
	append_string_lists (list, cxx_prelinker_objects);
}

extern void
append_ar_objects_to_list(string_list_t *list)
{
    append_string_lists (list, ar_objects);
}

extern void
append_libraries_to_list (string_list_t *list)
{
#ifdef TARG_STxP70
#ifndef COSY_LIB /* [HC] Newlib tree support. Keep former code for CoSy lib support */
        string lib_path;
        extern int STxP70mult;
        string_item_t *p;

        for (p = library_dirs->head; p != NULL; p = p->next) {
		add_string(list, concat_strings("-L", p->name));
        }
        /*
         * get_phase_dir(P_library) is not in library_dirs because
         * library_dirs is also used as the search path for the crt file
         */
	lib_path = string_copy(get_phase_dir(P_library));
	if (lib_short_double == TRUE) {
	  lib_path = concat_strings(lib_path,"/spieee754");
	}
	if (fpx == TRUE) {
	  lib_path = concat_strings(lib_path,"/fpx");
	} else if (STxP70mult == TRUE) {
	  lib_path = concat_strings(lib_path,"/mult");
	} else {
	  lib_path = concat_strings(lib_path,"/nomult");
	}
	if (lib_kind == LIB_STXP70_16) {
	  lib_path = concat_strings(lib_path,"/reg16");
        } else {
	  lib_path = concat_strings(lib_path,"/reg32");
	}
        if (!option_was_seen(O_L)) {
                add_string(list,
                           concat_strings("-L", lib_path));
        }
	/* Add path for link scripts */
        add_string(list,
                   concat_strings("-L", concat_strings(get_phase_dir(P_library),"/ldscript")));
#else
        string_item_t *p;
        for (p = library_dirs->head; p != NULL; p = p->next) {
		add_string(list, concat_strings("-L", p->name));
        }
        /*
         * get_phase_dir(P_library) is not in library_dirs because
         * library_dirs is also used as the search path for the crt file
         */
        if (!option_was_seen(O_L)) {
                add_string(list,
                           concat_strings("-L", get_phase_dir(P_library)));
        }
#endif
#else
        string_item_t *p;
        for (p = library_dirs->head; p != NULL; p = p->next) {
		add_string(list, concat_strings("-L", p->name));
        }
        /*
         * get_phase_dir(P_library) is not in library_dirs because
         * library_dirs is also used as the search path for the crt file
         */
        if (!option_was_seen(O_L)) {
                add_string(list,
                           concat_strings("-L", get_phase_dir(P_library)));
        }
#endif
}

extern void
dump_objects (void)
{
	printf("objects:  ");
	print_string_list (stdout, objects);
	printf("objects for ld-r:  ");
	print_string_list (stdout, objects_ld_r);
	printf("objects for final ld:  ");
	print_string_list (stdout, objects_ld_final);
}

#ifdef TARG_ST
/* [CG]: See comment above. */
extern void
add_script_file (string file)
{
  add_string(script_files, file);
}
extern void
append_script_files_to_list (string_list_t *list)
{
  string_item_t *p;
  for (p = script_files->head; p != NULL; p = p->next) {
    add_string(list, "-T");
    add_string(list, p->name);
  }
}
#endif

extern void
add_library_dir (string path)
{
	add_string(library_dirs, path);
}

extern void
add_library_options (void)
{
	int flag;
	buffer_t mbuf;
	buffer_t rbuf;
	string suffix = NULL;
	string mips_lib = NULL;
	string proc_lib = NULL;
	string lib = NULL;
	/*
	 * 32-bit libraries go in /usr/lib32. 
	 * 64-bit libraries go in /usr/lib64.
	 * isa-specific libraries append /mips{2,3,4}.
	 * non_shared libraries append /nonshared.
	 */
#ifdef TARG_ST200
#ifdef MUMBLE_ST200_BSP
	extern string st200_core, st200_soc, st200_board;
	extern string st200_core_name, st200_soc_name, st200_board_name ;
	extern string st200_targetdir ;
	string ofile;
#endif
	extern string st200_libdir;

	switch (proc) {
	case PROC_ST210:
	  append_phase_dir(P_library, "st210");
	  append_phase_dir(P_startup, "st210");
	  break;
	case PROC_ST221:
	  append_phase_dir(P_library, "st221");
	  append_phase_dir(P_startup, "st221");
	  break;
	case PROC_ST220:
	  append_phase_dir(P_library, "st220");
	  append_phase_dir(P_startup, "st220");
	  break;
	case PROC_ST231:
	  append_phase_dir(P_library, "st231");
	  append_phase_dir(P_startup, "st231");
	  break;
	case PROC_ST240:
	  append_phase_dir(P_library, "st240");
	  append_phase_dir(P_startup, "st240");
	  break;
	}

	if (endian == ENDIAN_LITTLE) {
	  append_phase_dir(P_library, "le");
	  append_phase_dir(P_startup, "le");
	}
	else {
	  append_phase_dir(P_library, "be");
	  append_phase_dir(P_startup, "be");
	}

#ifdef MUMBLE_ST200_BSP
	switch (st200_runtime) {	  
	case RUNTIME_BARE:
	  append_phase_dir(P_library, "bare");
	  append_phase_dir(P_startup, "bare");
	  break;
	case RUNTIME_OS21:
	case RUNTIME_OS21_DEBUG:
	  append_phase_dir(P_library, "os21");
	  append_phase_dir(P_startup, "os21");
	  break;	  
	default:
	  internal_error("no runtime set? (%d)", st200_runtime);
	}
#endif
#elif defined( TARG_STxP70 )
#ifdef MUMBLE_STxP70_BSP
	extern string stxp70_core, stxp70_soc, stxp70_board;
	extern string stxp70_core_name, stxp70_soc_name, stxp70_board_name ;
	extern string stxp70_targetdir ;
	string ofile;
#endif
	extern string stxp70_libdir;

#ifdef COSY_LIB /* [HC] Architecture is managed at toolset level. Keep for CoSy Lib compat. */
	switch (proc) {
	case PROC_stxp70_v3:
	  append_phase_dir(P_library, "/stxp70");
	  append_phase_dir(P_startup, "/stxp70");
	  break;
	}
#endif

#ifdef COSY_LIB /* [HC] Only one endianness available. Keep for CoSy lib support. */
	if (endian == ENDIAN_LITTLE) {
	  append_phase_dir(P_library, "/le");
	  append_phase_dir(P_startup, "/le");
	}
	else {
	  append_phase_dir(P_library, "/be");
	  append_phase_dir(P_startup, "/be");
	}
#endif

#ifdef COSY_LIB /* [HC] Only bare machine as of now. Keep for CoSy lib support. */
	switch (stxp70_runtime) {
	case RUNTIME_NONE:
	  break;
	case RUNTIME_BARE:
	  append_phase_dir(P_library, "/bare");
	  append_phase_dir(P_startup, "/bare");
	  break;	  
	default:
	  internal_error("no runtime set? (%d)", stxp70_runtime);
	}
#endif

#elif defined( TARG_ARM )
#ifdef MUMBLE_ARM_BSP
	extern string arm_core, arm_soc, arm_board;
	extern string arm_core_name, arm_soc_name, arm_board_name ;
	extern string arm_targetdir ;
	string ofile;
#endif
	extern string arm_libdir;

	switch (proc) {
	case PROC_armv5:
	  append_phase_dir(P_library, "/armv5");
	  append_phase_dir(P_startup, "/armv5");
	  break;
	case PROC_armv6:
	  append_phase_dir(P_library, "/armv6");
	  append_phase_dir(P_startup, "/armv6");
	  break;
	}

	if (endian == ENDIAN_LITTLE) {
	  append_phase_dir(P_library, "/le");
	  append_phase_dir(P_startup, "/le");
	}
	else {
	  append_phase_dir(P_library, "/be");
	  append_phase_dir(P_startup, "/be");
	}

	switch (arm_runtime) {
	case RUNTIME_NONE:
	  break;
	case RUNTIME_BARE:
	  append_phase_dir(P_library, "/bare");
	  append_phase_dir(P_startup, "/bare");
	  break;	  
	default:
	  internal_error("no runtime set? (%d)", arm_runtime);
	}
#endif

	switch (abi) {
#ifdef TARG_MIPS
	case ABI_N32:
	case ABI_I32:
		append_phase_dir(P_library, "32");
		append_phase_dir(P_startup, "32");
		break;
	case ABI_64:
		append_phase_dir(P_library, "64");
		append_phase_dir(P_startup, "64");
		break;
#endif
	case ABI_I64:
		break;
	case ABI_IA32:
 		break;
        case ABI_ST100:
	case ABI_ST200_embedded:
	case ABI_ST200_PIC:
	case ABI_STxP70_embedded:
	case ABI_STxP70_fpx:
	case ABI_ARM_ver1:
	case ABI_ARM_ver2:
	  break;
	default:
		internal_error("no abi set? (%d)", abi);
	}
#ifdef TARG_MIPS
	if (isa > ISA_MIPS1 && isa <= ISA_MIPS6) {
		sprintf(mbuf, "%s/mips%d", get_phase_dir(P_library), isa);
		mips_lib = mbuf;
	}
	if (proc > 4 || (proc == 4 && isa == 3)) {
		/* add processor-specific r* lib-path */
		sprintf(rbuf, "%s/r%d000", mips_lib, proc);
		proc_lib = rbuf;
	}
#endif

#ifndef TARG_ST
#if !defined(linux)
	flag = add_string_option(O_L__, get_phase_dir(P_library));
        add_option_seen (flag);
#endif
#endif

#ifdef TARG_ST200
#ifdef MUMBLE_ST200_BSP
	/* set core path */
	if (!st200_core) {
	  st200_core = concat_path(get_phase_dir(P_alt_library), concat_path("core", "st220"));
	  st200_core_name = string_copy ("st220");
	}
	if (st200_targetdir) {
	  st200_core = concat_path(st200_targetdir, concat_path("core", st200_core_name));
	  if (!is_directory (st200_core))
	    st200_core = concat_path(get_phase_dir(P_alt_library), concat_path("core", st200_core_name));
	}

	/* set soc path */
	if (!st200_soc) {
	  st200_soc = concat_path(get_phase_dir(P_alt_library), concat_path("soc", "default"));
	  st200_soc_name = string_copy ("default");
	}
	if (st200_targetdir) {
	  st200_soc = concat_path(st200_targetdir, concat_path("soc", st200_soc_name));
	  if (!is_directory (st200_soc))
	    st200_soc = concat_path(get_phase_dir(P_alt_library), concat_path("soc", st200_soc_name));
	}

	/* set board path */
	if (!st200_board) {
	  st200_board = concat_path(get_phase_dir(P_alt_library), concat_path("board", "default"));
	  st200_board_name = string_copy ("default");
	}
	if (st200_targetdir) {
	  st200_board = concat_path(st200_targetdir, concat_path("board", st200_board_name));
	  if (!is_directory (st200_board))
	    st200_board = concat_path(get_phase_dir(P_alt_library), concat_path("board", st200_board_name));
	}

	if (!nostdinc) {
	  /* set core include path */
	  if (st200_core && is_directory (st200_core)) {
	    flag = add_string_option(O_isystem__, st200_core);
	    add_option_seen (flag);
	  }

	  /* set soc include path */
	  if (st200_soc && is_directory (st200_soc)) {
	    flag = add_string_option(O_isystem__, st200_soc);
	    add_option_seen (flag);
	  }

	  /* set board include path */
	  if (st200_board && is_directory (st200_board)) {
	    flag = add_string_option(O_isystem__, st200_board);
	    add_option_seen (flag);
	  }
	}

	if (!option_was_seen(O_nostdlib)) {
	  if (st200_core && is_directory (st200_core)) {
	    st200_core = concat_path (st200_core, 
				      concat_path(endian == ENDIAN_LITTLE ? "le" : "be", 
						  (st200_runtime == RUNTIME_OS21 || 
						   st200_runtime == RUNTIME_OS21_DEBUG) ? 
						  "os21" : "bare"));
	    flag = add_string_option(O_L__, st200_core);
	    add_option_seen (flag);
	  }

	  if (st200_soc && is_directory (st200_soc)) {
	    st200_soc = concat_path (st200_soc, 
				     concat_path(proc == PROC_ST220 ? "st220" :  
						 proc == PROC_ST231 ? "st231" : 
						 proc == PROC_ST240 ? "st240" : "st220" ,
						 concat_path(endian == ENDIAN_LITTLE ? "le" : "be", 
							     (st200_runtime == RUNTIME_OS21 || 
							      st200_runtime == RUNTIME_OS21_DEBUG) ? 
							     "os21" : "bare")));
	    flag = add_string_option(O_L__, st200_soc);
	    add_option_seen (flag);
	  }

	  if (st200_board && is_directory (st200_board)) {
	    st200_board = concat_path (st200_board, 
				       concat_path(proc == PROC_ST220 ? "st220" :  
						   proc == PROC_ST231 ? "st231" : 
						   proc == PROC_ST240 ? "st240" : "st220" ,
						   
						   concat_path(endian == ENDIAN_LITTLE ? "le" : "be", 
							     (st200_runtime == RUNTIME_OS21 || 
							      st200_runtime == RUNTIME_OS21_DEBUG) ? 
							     "os21" : "bare")));
	    flag = add_string_option(O_L__, st200_board);
	    add_option_seen (flag);
	  }
	}

	if (st200_libdir) {
	  st200_libdir = concat_path (st200_libdir, 
				      concat_path(proc == PROC_ST220 ? "st220" :  
						  proc == PROC_ST231 ? "st231" : 
						  proc == PROC_ST240 ? "st240" : "st220" ,
						  
						  concat_path(endian == ENDIAN_LITTLE ? "le" : "be", 
							      (st200_runtime == RUNTIME_OS21 || 
							       st200_runtime == RUNTIME_OS21_DEBUG) ? 
							      "os21" : "bare")));
	  add_library_dir (st200_libdir);
	}
#endif /* MUMBLE_ST200_BSP */

#endif /* TARG_ST200 */

#ifdef TARG_STxP70
#ifdef MUMBLE_STxP70_BSP
#define DEF_CORE_NAME "stxp70"
#define DEF_BOARD_NAME "default"
#define DEF_SOC_NAME "default"
	/* set core path */
	if (!stxp70_core) {
	  stxp70_core = concat_path(get_phase_dir(P_alt_library), concat_path("core", DEF_CORE_NAME));
	  stxp70_core_name = string_copy (DEF_CORE_NAME);
	}
	if (stxp70_targetdir) {
	  stxp70_core = concat_path(stxp70_targetdir, concat_path("core", stxp70_core_name));
	  if (!is_directory (stxp70_core))
	    stxp70_core = concat_path(get_phase_dir(P_alt_library), concat_path("core", stxp70_core_name));
	}

	/* set soc path */
	if (!stxp70_soc) {
	  stxp70_soc = concat_path(get_phase_dir(P_alt_library), concat_path("soc", DEF_SOC_NAME));
	  stxp70_soc_name = string_copy (DEF_SOC_NAME);
	}
	if (stxp70_targetdir) {
	  stxp70_soc = concat_path(stxp70_targetdir, concat_path("soc", stxp70_soc_name));
	  if (!is_directory (stxp70_soc))
	    stxp70_soc = concat_path(get_phase_dir(P_alt_library), concat_path("soc", stxp70_soc_name));
	}

	/* set board path */
	if (!stxp70_board) {
	  stxp70_board = concat_path(get_phase_dir(P_alt_library), concat_path("board", DEF_BOARD_NAME));
	  stxp70_board_name = string_copy (DEF_BOARD_NAME);
	}
	if (stxp70_targetdir) {
	  stxp70_board = concat_path(stxp70_targetdir, concat_path("board", stxp70_board_name));
	  if (!is_directory (stxp70_board))
	    stxp70_board = concat_path(get_phase_dir(P_alt_library), concat_path("board", stxp70_board_name));
	}

	if (!nostdinc) {
	  /* set core include path */
	  if (stxp70_core && is_directory (stxp70_core)) {
	    flag = add_string_option(O_isystem__, stxp70_core);
	    add_option_seen (flag);
	  }

	  /* set soc include path */
	  if (stxp70_soc && is_directory (stxp70_soc)) {
	    flag = add_string_option(O_isystem__, stxp70_soc);
	    add_option_seen (flag);
	  }

	  /* set board include path */
	  if (stxp70_board && is_directory (stxp70_board)) {
	    flag = add_string_option(O_isystem__, stxp70_board);
	    add_option_seen (flag);
	  }
	}

	if (!option_was_seen(O_nostdlib)) {
	  if (stxp70_core && is_directory (stxp70_core)) {
	    stxp70_core = concat_path (stxp70_core, 
				      concat_path(endian == ENDIAN_LITTLE ? "le" : "be", 
						  stxp70_runtime == RUNTIME_NONE ? "none" : "unknown"));
	    flag = add_string_option(O_L__, stxp70_core);
	    add_option_seen (flag);
	  }

	  if (stxp70_soc && is_directory (stxp70_soc)) {
	    stxp70_soc = concat_path (stxp70_soc, 
				      concat_path((proc == PROC_stxp70_v3) ? "stxp70v3" :
						  (proc == PROC_stxp70_v4_single ||proc == PROC_stxp70_v4_dual ||proc == PROC_stxp70_v4_dual_arith) ? "stxp70v4" : "unknown",
						  concat_path(endian == ENDIAN_LITTLE ? "le" : "be", 
							      stxp70_runtime == RUNTIME_NONE ? "none" : "unknown")));
	    flag = add_string_option(O_L__, stxp70_soc);
	    add_option_seen (flag);
	  }

	  if (stxp70_board && is_directory (stxp70_board)) {
	    stxp70_board = concat_path (stxp70_board, 
					concat_path((proc == PROC_stxp70_v3) ? "stxp70v3" :
						    (proc == PROC_stxp70_v4_single ||proc == PROC_stxp70_v4_dual ||proc == PROC_stxp70_v4_dual_arith) ? "stxp70v4" : "unknown",
						    concat_path(endian == ENDIAN_LITTLE ? "le" : "be", 
								stxp70_runtime == RUNTIME_NONE ? "none" : "unknown")));
	    flag = add_string_option(O_L__, stxp70_board);
	    add_option_seen (flag);
	  }
	}
#endif /* MUMBLE_STxP70_BSP */
	
	if (stxp70_libdir) {
	  stxp70_libdir = concat_path (stxp70_libdir, 
				       concat_path((proc == PROC_stxp70_v3) ? "stxp70v3" :
						    (proc == PROC_stxp70_v4_single ||proc == PROC_stxp70_v4_dual ||proc == PROC_stxp70_v4_dual_arith) ? "stxp70v4" : "unknown",
						   concat_path(endian == ENDIAN_LITTLE ? "le" : "be", 
							       stxp70_runtime == RUNTIME_NONE ? "none" : "unknown")));
	  add_library_dir (stxp70_libdir);
	}
#endif /* TARG_STxP70 */


#ifdef TARG_ARM
#ifdef MUMBLE_ARM_BSP
#define DEF_CORE_NAME "armv5"
#define DEF_BOARD_NAME "default"
#define DEF_SOC_NAME "default"
	/* set core path */
	if (!arm_core) {
	  arm_core = concat_path(get_phase_dir(P_alt_library), concat_path("core", DEF_CORE_NAME));
	  arm_core_name = string_copy (DEF_CORE_NAME);
	}
	if (arm_targetdir) {
	  arm_core = concat_path(arm_targetdir, concat_path("core", arm_core_name));
	  if (!is_directory (arm_core))
	    arm_core = concat_path(get_phase_dir(P_alt_library), concat_path("core", arm_core_name));
	}

	/* set soc path */
	if (!arm_soc) {
	  arm_soc = concat_path(get_phase_dir(P_alt_library), concat_path("soc", DEF_SOC_NAME));
	  arm_soc_name = string_copy (DEF_SOC_NAME);
	}
	if (arm_targetdir) {
	  arm_soc = concat_path(arm_targetdir, concat_path("soc", arm_soc_name));
	  if (!is_directory (arm_soc))
	    arm_soc = concat_path(get_phase_dir(P_alt_library), concat_path("soc", arm_soc_name));
	}

	/* set board path */
	if (!arm_board) {
	  arm_board = concat_path(get_phase_dir(P_alt_library), concat_path("board", DEF_BOARD_NAME));
	  arm_board_name = string_copy (DEF_BOARD_NAME);
	}
	if (arm_targetdir) {
	  arm_board = concat_path(arm_targetdir, concat_path("board", arm_board_name));
	  if (!is_directory (arm_board))
	    arm_board = concat_path(get_phase_dir(P_alt_library), concat_path("board", arm_board_name));
	}

	if (!nostdinc) {
	  /* set core include path */
	  if (arm_core && is_directory (arm_core)) {
	    flag = add_string_option(O_isystem__, arm_core);
	    add_option_seen (flag);
	  }

	  /* set soc include path */
	  if (arm_soc && is_directory (arm_soc)) {
	    flag = add_string_option(O_isystem__, arm_soc);
	    add_option_seen (flag);
	  }

	  /* set board include path */
	  if (arm_board && is_directory (arm_board)) {
	    flag = add_string_option(O_isystem__, arm_board);
	    add_option_seen (flag);
	  }
	}

	if (!option_was_seen(O_nostdlib)) {
	  if (arm_core && is_directory (arm_core)) {
	    arm_core = concat_path (arm_core, 
				      concat_path(endian == ENDIAN_LITTLE ? "le" : "be", 
						  arm_runtime == RUNTIME_NONE ? "none" : "unknown"));
	    flag = add_string_option(O_L__, arm_core);
	    add_option_seen (flag);
	  }

	  if (arm_soc && is_directory (arm_soc)) {
	    arm_soc = concat_path (arm_soc, 
				   concat_path(proc == PROC_armv5 ? "armv5" : 
					       proc == PROC_armv6 ? "armv6" :  "unknown",
					       concat_path(endian == ENDIAN_LITTLE ? "le" : "be", 
							   arm_runtime == RUNTIME_NONE ? "none" : "unknown")));
	    flag = add_string_option(O_L__, arm_soc);
	    add_option_seen (flag);
	  }

	  if (arm_board && is_directory (arm_board)) {
	    arm_board = concat_path (arm_board, 
				   concat_path(proc == PROC_armv5 ? "armv5" : 
					       proc == PROC_armv6 ? "armv6" :  "unknown",
					       concat_path(endian == ENDIAN_LITTLE ? "le" : "be", 
							   arm_runtime == RUNTIME_NONE ? "none" : "unknown")));
	    flag = add_string_option(O_L__, arm_board);
	    add_option_seen (flag);
	  }
	}
#endif /* MUMBLE_ARM_BSP */
	
	if (arm_libdir) {
	  arm_libdir = concat_path (arm_libdir, 
				   concat_path(proc == PROC_armv5 ? "armv5" : 
					       proc == PROC_armv6 ? "armv6" :  "unknown",
					       concat_path(endian == ENDIAN_LITTLE ? "le" : "be", 
							   arm_runtime == RUNTIME_NONE ? "none" : "unknown")));
	  add_library_dir (arm_libdir);
 	}
#endif /* TARG_ARM */
}

/* search library_dirs for the crt file */
extern string
find_crt_path (string crtname)
{
  string_item_t *p;
  char *ptr;
  
  for (p = library_dirs->head; p != NULL; p = p->next) {
    ptr = concat_path(p->name, crtname);
    if (file_exists(ptr)) {
      return ptr;
    }
  }
  /* not found */
  if (option_was_seen(O_nostdlib) || option_was_seen(O_L)) {
    error("crt files not found in any -L directories:");
    for (p = library_dirs->head; p != NULL; p = p->next) {
      fprintf(stderr, "\t%s\n", concat_path(p->name, crtname));
    }
    return crtname;
  } else {
    /* use default */
    ptr = concat_path(get_phase_dir(P_startup), crtname);
    return ptr;
  }
}


