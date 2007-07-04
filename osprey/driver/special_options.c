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


/*
 * OPTIONS that are not simple enough to handle in the table
 * are done by hand in these routines.
 */
#include <strings.h>
#include <stdlib.h>
#include <stamp.h>
#include "string_utils.h"
#include "file_utils.h"
#include "options.h"
#include "option_seen.h"
#include "option_names.h"
#include "lang_defs.h"
#include "errors.h"
#include "opt_actions.h"
#include "file_names.h"
#include "get_options.h"
#include "phases.h"
#ifdef BCO_ENABLED /* Thierry */
#include "file_utils.h"
#endif /* BCO_Enabled Thierry */
static char compiler_version[] = INCLUDE_STAMP;

extern void
set_defaults (void)
{
	int flag;
	/* handle SGI_CC environment variable */
	string sgi_cc = getenv("SGI_CC");
	if (sgi_cc != NULL && !is_toggled(ansi)) {
		/* value not set yet */
		if (same_string(sgi_cc, "-cckr")) {
			toggle(&ansi,KR_ANSI);
			prepend_option_seen (O_cckr);
		} else if (same_string(sgi_cc, "-xansi")) {
			toggle(&ansi,EXTENDED_ANSI);
			prepend_option_seen (O_xansi);
		} else if (same_string(sgi_cc, "-ansi")) {
			toggle(&ansi,STRICT_ANSI);
			prepend_option_seen (O_ansi);
		}
	}

#ifdef TARG_ST
	// [CG] Set C standard if strict ansi defined
	if (c_std == C_STD_UNDEF && ansi == STRICT_ANSI) {
	  if (invoked_lang == L_cc) {
	    c_std = C_STD_C89;
	  }
	  else if (invoked_lang == L_CC) {
	    // (cbr) for cplusplus
	    c_std = C_STD_CXX98;
	  }
	}
#endif

	/* XPG fort77 doesn't allow -O with no explicit level */
	if (xpg_flag && invoked_lang == L_f77 && option_was_seen(O_O)) {
		error("XPG compiles must specify explicit optlevel rather than -O");
	}
	{
	  /* QA wants way to turn off this check via environment var */
	  string ir_version_check = getenv("COMPILER_IR_VERSION_CHECK");
	  if (ir_version_check != NULL) {
		if (same_string(ir_version_check, "off")) {
			flag = add_string_option(O_DEBUG_, "ir_version_check=off");
			/* prepend so comes before user option */
			prepend_option_seen(flag);
		}
	  }
	}
#ifdef IRIX62
	flag = add_string_option(O_LNO_, "plower=off");
	prepend_option_seen(flag);
	flag = add_string_option(O_CG_, "pjump_all=off");
	prepend_option_seen(flag);
#endif
#ifdef IRIX64
	flag = add_string_option(O_CG_, "pjump_all=off");
	prepend_option_seen(flag);
#endif

#ifdef TARG_ST
	// (cbr) -fstrict-aliasing default on 3.3.3 
	if (aliasing == UNDEFINED) {
	  flag = add_string_option(O_OPT_, "alias=typed");
	  prepend_option_seen(flag);
	}

	// Set exception support options.
	if (invoked_lang == L_CC && !is_toggled(noexceptions)) {
	  flag = add_string_option(O_LANG_, "exc=ON");
	  prepend_option_seen(flag);
	  flag = add_string_option(O_CG_, "emit_unwind_info=ON");
	  prepend_option_seen(flag);
	}
#endif

#ifdef TARG_ST
	/* Disable LNO and LAO when C++ and exceptions are enabled.  */
	/* This disabling should be removed in the long term after LNO (resp. LAO) support are validated (resp. fixed).  */
	if (invoked_lang == L_CC  && (!is_toggled(noexceptions) || (noexceptions==FALSE)) && (olevel >= 3)) {
	    flag = add_string_option(O_PHASE_, "LNO=off");
	    prepend_option_seen(flag);
#ifdef LAO_ENABLED
	    // Only when LAO is enabled.
	    flag = add_string_option(O_CG_, "LAO_optimization=0");
	    prepend_option_seen(flag);
#endif
	}
#endif

#ifdef TARG_ST200
	if (endian == UNDEFINED) 
	  toggle(&endian, ENDIAN_LITTLE);
#ifdef MUMBLE_ST200_BSP
	if (st200_runtime == UNDEFINED) 
	  toggle(&st200_runtime, RUNTIME_BARE);
	/* At this point we let st200_syscall undefined, since it is 'autodetected' later on*/
#endif
#endif /* TARG_ST200 */

#ifdef TARG_STxP70
	if (endian == UNDEFINED) 
	  toggle(&endian, ENDIAN_LITTLE);

	if(fp_lib == UNDEFINED)
	  toggle(&fp_lib, LIB_FP_SOFT_FLOAT);

	if(lib_kind == UNDEFINED)
	  toggle(&lib_kind, LIB_STXP70_32);

	if(lib_short_double == UNDEFINED)
	  toggle(&lib_short_double, FALSE);

	if(lib_nofloat == UNDEFINED)
	  toggle(&lib_nofloat, FALSE);

#ifdef MUMBLE_STxP70_BSP
	if (stxp70_runtime == UNDEFINED) 
	  toggle((int*)&stxp70_runtime, RUNTIME_BARE);
#endif
#endif /* TARG_STxP70 */

#if defined (TARG_ST) && (GNU_FRONT_END==29)
	/* Default gcc behavior. */
	if (signed_char == UNDEFINED)
	  toggle(&signed_char, TRUE);
#endif

	prepend_option_seen(O_usegfe);
	prepend_option_seen(O_usef90);

	if (ansi == UNDEFINED) {
		toggle(&ansi,EXTENDED_ANSI);
		if (!is_toggled(cppundef)) {
		  prepend_option_seen(O_xansi);
		}
	}

	// Definition of non standard cpp options
	// are controlled by the cppundef flag (-undef)
	if (!is_toggled(cppundef)) {
#ifdef sgi
	  prepend_option_seen (O_cpp_irix);
	  prepend_option_seen(O_cpp_svr4);
	  prepend_option_seen(O__SYSTYPE_SVR4);
	  prepend_option_seen(O_cpp_cfe);
	  if (ansi != STRICT_ANSI) {
	    prepend_option_seen (O_cpp_nonansi_svr4);
	    prepend_option_seen (O_cpp_extensions);
	    prepend_option_seen (O_cpp_krdefs);
	  }
#endif
#ifdef HOST_IA32
	  flag = add_string_option(O_D, "__host_ia32");
	  prepend_option_seen (flag);
#endif
#ifdef HOST_IA64
	  flag = add_string_option(O_D, "__host_ia64");
	  prepend_option_seen (flag);
#endif
#ifdef HOST_MIPS
	  flag = add_string_option(O_D, "__host_mips");
	  prepend_option_seen (flag);
	  if (ansi != STRICT_ANSI) {
	    flag = add_string_option(O_D, "host_mips");
	    prepend_option_seen (flag);
	  }
#endif
	  
#ifdef MIPS
	  prepend_option_seen(O_cpp_mips);
#endif
	  prepend_option_seen(O_m1);
#ifdef TARG_ST
	  /* Arthur: if we ever need to define something like this,
	   *         we'll fix it
	   */
#else
	  prepend_option_seen(O_cpp_version);
#endif
#ifdef sgi
	  /* old compiler-version define */
	  flag = add_string_option(O_D, "_COMPILER_VERSION=740");
	  prepend_option_seen (flag);
#endif
	  prepend_option_seen(O_cpp_nonansi_f77);
	  prepend_option_seen(O_cpp_fortran90);
	  prepend_option_seen(O_cpp_fortran77);
	  prepend_option_seen(O_cpp_fortran);
	  prepend_option_seen(O_cpp_cplus);
	  prepend_option_seen(O_cpp_assembly);
	  prepend_option_seen(O_D_LANGUAGE_C);
	} /* !is_toggled(cppundef) */
	
	prepend_option_seen(O_ptnone);
	prepend_option_seen(O_prelink);
#ifndef TARG_ST
	/* to be enabled with 2.13 version of the linker. */
	prepend_option_seen(O_demangle);
#endif

	/* whether to generate shared objects */
	if (shared == UNDEFINED) {
	  if (abi == ABI_IA32) {
	    toggle(&shared,NON_SHARED);
	    prepend_option_seen(O_non_shared);
	  }
	  else if (abi != ABI_ST100 && abi != ABI_ST200_embedded && 
		   abi != ABI_STxP70_embedded && abi != ABI_STxP70_fpx) {
	    if (gprel == UNDEFINED) {
	      toggle(&shared,CALL_SHARED);
	      prepend_option_seen(O_call_shared);
	      // clarkes: By default, call_shared implies gprel.
	    }
	  }
	}
#ifndef sgi
#ifndef TARG_ST
	if (shared != NON_SHARED) {
		prepend_option_seen(O_cpp_pic);
	}
#endif
#endif

	/* whether to generate GP-relative addressing */
	if (gprel == UNDEFINED) {
	  if (abi == ABI_ST100) {
	    toggle(&gprel, FALSE);
	  }
	  else if (abi == ABI_ST200_embedded || abi == ABI_ST200_PIC) {
	    if (shared == CALL_SHARED || shared == DSO_SHARED) {
	      toggle(&gprel,TRUE);
	      // clarkes: But do not prepend -gprel, since that
	      // implies Constant_GP.
	    }
	    else {
	      toggle(&gprel,FALSE);
	    }
	  }
	  else if (abi == ABI_STxP70_embedded || abi == ABI_STxP70_fpx) {
	    toggle(&gprel,FALSE);
	  }
	  else {
	    toggle(&gprel,TRUE);
	    // clarkes: Seems unlikely that prepending -gprel is the
	    // correct thing to do, since it also implies Constant_GP.
	    prepend_option_seen(O_gprel);
	  }
	}

	if (!is_toggled(cppundef)) {
#ifndef sgi
	  if (abi == ABI_64 || abi == ABI_I64)
	    add_option_seen(O_cpp_lp64);
	  else
	    add_option_seen(O_cpp_lp32);
	  if (abi == ABI_I32)
	    add_option_seen(O_cpp_i32);
	  else if (abi == ABI_I64)
	    add_option_seen(O_cpp_i64);
	  else if (abi == ABI_IA32)
	    add_option_seen(O_cpp_ia32);
#endif
	}
	if (!is_toggled(isstatic)) {
		toggle(&isstatic,1);
		prepend_option_seen(O_automatic);
	}
	prepend_option_seen(O_auto_include);

}


static int
get_olevel_flag (int olevel)
{
	switch (olevel) {
	case 0: return O_O0;
	case 1: return O_O1;
	case 2: return O_O2;
	case 3: return O_O3;
	default: return O_Unrecognized;
	}
}

/* replace -O* with O0 */
static void
turn_down_opt_level (int new_olevel, string msg)
{
	int flag;
	int new_flag;
	if (fullwarn) warning(msg);
	flag = get_olevel_flag(olevel);
	new_flag = get_olevel_flag(new_olevel);
	if (option_was_seen(O_O))
		replace_option_seen (O_O, new_flag);
	else if (option_was_seen(flag))
		replace_option_seen (flag, new_flag);
	else
		internal_error("driver didn't find -O flag");
	olevel = new_olevel;
}

#ifdef TARG_ST

/*
set OPTION_CALL_SHARED: {
-shared
--shared
-dy
-Wl,-shared
-Wl,-Bdynamic
-Wl,-dy
-Wl,-call_shared
-Wl,--dynamic-linker
}

set OPTION_NON_SHARED:{
-static
--static
-Wl,-Bstatic
-Wl,-dn
-Wl,-non_shared
-Wl,-static
-Wl,-r
-r
-non_shared
} 
dynamic is true when one of OPTION_CALL_SHARED is passed on the cmd line
dynamic is false when one of OPTION_NON_SHARED is passed on the cmd line
dynamic is undef otherweise.
*/

/*
  Try to answer the question: Is the link a static (static exe or
  relocatable) or dynamic link (call_shared or dso_shared)
  
*/
static boolean 
maybe_dynamic() {
  if (dynamic == UNDEFINED)
    dynamic = FALSE;
  return dynamic;
}
#endif


static void
turn_off_ipa (string msg)
{
	int flag;
	warning (msg);
	ipa = FALSE;
	/* remove all ipa flags from option_seen list */
	FOREACH_OPTION_SEEN(flag) {
		if (flag == O_ipa)
			set_option_unseen(flag);
		else if (flag == O_IPA)
			set_option_unseen(flag);
		else if (is_derived_option (flag)
		    && get_derived_parent(flag) == O_IPA_)
			set_option_unseen(flag);
	}
}

static void
turn_off_lai (string msg)
{
  int flag;
  warning (msg);
  lai = FALSE;
  /* remove all lai flags from option_seen list */
  FOREACH_OPTION_SEEN(flag) {
    if (flag == O_lai)
      set_option_unseen(flag);
  }
}

extern void
add_special_options (void)
{
	int flag;
	buffer_t buf;
#ifndef BCO_ENABLED /* Thierry */
	string s;
#else /* BCO_Enabled Thierry */
/* 	string s; */
#endif /* BCO_Enabled Thierry */

	/* Hack for F90 -MDupdate. We need to pass the MDupdate to mfef90, because we don't
	 * have an integrated pre-processor. I can't figure out a better way to do this, given
	 * the architecture of the phase generator. 
	 * R. Shapiro, 2/26/97
	 */
	add_phase_for_option(O_MDupdate,P_f90_fe);
	add_phase_for_option(O_MDtarget,P_f90_fe);
	remove_phase_for_option(O_MDupdate,P_f90_cpp);
	remove_phase_for_option(O_MDtarget,P_f90_cpp);

        add_phase_for_option(O_D, P_cppf90_fe);
        add_phase_for_option(O_U, P_cppf90_fe);
        add_phase_for_option(O_E, P_cppf90_fe);
        add_phase_for_option(O_P, P_cppf90_fe);

	if (use_ftpp == TRUE) {
		/* ftpp means pass defines directly to mfef90,
		 * and since not using gcc we have to pass some options
		 * that are otherwise implicit. */
		if (endian == ENDIAN_LITTLE)
    			flag = add_string_option(O_D, "_LITTLE_ENDIAN");
		else
    			flag = add_string_option(O_D, "_BIG_ENDIAN");

		prepend_option_seen (flag);
    		flag = add_string_option(O_D, "__LONG_MAX__=9223372036854775807L");
		prepend_option_seen (flag);
		prepend_option_seen (O_elf);
		prepend_option_seen (O_cpp_unix);
#ifdef linux
		prepend_option_seen (O_cpp_linux);
#endif
#ifdef sun
                prepend_option_seen (O_cpp_sun);
#endif
#ifdef __CYGWIN__
                prepend_option_seen (O_cpp_cygwin);
#endif
#ifdef __MINGW32__
                prepend_option_seen (O_cpp_mingwin);
#endif
		prepend_option_seen (O_cpp_nonansi);
		if (keep_flag) {
			add_phase_for_option (O_keep, P_cppf90_fe);
		}
	}

#ifdef TARG_ST
	  /* (cbr) set -I seen for includes. check phases.c */
	if (!nostdinc) {
	  flag = add_string_option(O_isystem__, get_phase_dir(P_include));
	  add_option_seen (flag);
#ifdef TARG_STxP70
          flag = add_string_option(O_isystem__,
                                   concat_strings(get_phase_dir(P_include),
                                                  "/models"));
          add_option_seen (flag);
#endif
	}

	/* (cbr) specific path for c++ includes */
	if (!nostdinc && !nostdincc && invoked_lang == L_CC) {
	  flag = add_string_option(O_isystem__,  concat_path(get_phase_dir(P_include),"c++"));
	  add_option_seen (flag);
	  flag = add_string_option(O_isystem__,  concat_path(get_phase_dir(P_include), concat_path("c++","backward")));
	  add_option_seen (flag);
	}
#endif

#if defined (TARG_ST) && (GNU_FRONT_END==33)
	/* (cbr) use embedded cc1 cpp */
	if (option_was_seen(O_traditional)) {
	  if (option_was_seen (O_E)) {
	    add_phase_for_option(O_traditional_cpp, P_any_cpp);
	    replace_option_seen(O_traditional, O_traditional_cpp);
	  } else {
	    error("GNU C no longer supports -traditional without -E");
	  }
	}

	// (cbr) supports -CC and -C
	if (option_was_seen(O_CC)) {
	  if (option_was_seen (O_E)) {
	    add_phase_for_option(O_CC, P_any_cpp);
	    //	    prepend_option_seen(O_CC);
	  } else {
	    error("GNU C does not support -CC without using -E");
	  }
	}
	if (option_was_seen(O_C)) {
	  if (option_was_seen (O_E)) {
	    add_phase_for_option(O_C, P_any_cpp);
	    //	    prepend_option_seen(O_CC);
	  } else {
	    error("GNU C does not support -C without using -E");
	  }
	}
#else
	  /* (cbr) use embedded cc1 cpp */
	if (option_was_seen(O_traditional)
		&& !option_was_seen(O_traditional_cpp)) 
	{
		/* pass -traditional to both gfe and cpp */
		add_phase_for_option(O_traditional, P_c_gfe);
		add_phase_for_option(O_traditional, P_cplus_gfe);
	}
#endif

	if (abi == ABI_N32 || abi == ABI_64) {
		if (endian == ENDIAN_LITTLE)
			prepend_option_seen(O_mel);
		else
			prepend_option_seen(O_meb);
	}

#if defined(TARG_IA32)
	if (!is_toggled(cppundef)) {
	  flag = add_string_option(O_D, "__NO_MATH_INLINES");
	  prepend_option_seen (flag);
	}
#endif

	if (!is_toggled(cppundef)) {
	  if (ansi != STRICT_ANSI || invoked_lang == L_CC) {
	    /* C++:  Add the extensions that are valid in xansi mode. */
	    prepend_option_seen(O_xtendefs);
	  }
	}

	if (mpkind == CRAY_MP) {
		Process_Cray_Mp();
	}
	else if (mpkind == NORMAL_MP || auto_parallelize) {
		Process_Mp();
	}

        if (auto_parallelize && ipa) {
                flag = add_new_option("-IPA:array_summary");
                add_phase_for_option(flag, P_ipl);
                prepend_option_seen (flag);
        }
#ifdef TARG_ST
	/* Check that olevel is enought to find uninitialized var:
	   check is done by WOPT, so only in O2 and plus*/
	if (Wuninitialized_is_asked == TRUE && olevel < 2)
	  fprintf(stderr, "Warning: -Wuninitialized is not supported without -O2, -Os or -O3\n");
#endif /* TARG_ST*/

#ifdef BCO_ENABLED /* Thierry */

        // Thierry: turn off deadcode in RELOCATABLE (don't know how
	// to build a call graph without a main function). To activate
	// deadcode in RELOCATABLE mode, use -Wo,--preserved option.
	if (deadcode == UNDEFINED && shared == RELOCATABLE) {
	  deadcode = FALSE;
	}

	if (deadcode == UNDEFINED) {
	  maybe_dynamic();
	  // [TB]: Activate deadcode even in dynamic mode
	  deadcode = (olevel >= 2) ? TRUE : FALSE;
	  if (deadcode == TRUE) {
            prepend_option_seen (O__deadcode);
	  }
	}

	/* Thierry begin */
	
	/* If icache_static is UNDEFINED, set it to TRUE when olevel >= 2*/
        if (!icache_mapping  && icache_static == UNDEFINED) {
	  maybe_dynamic();
	  // [TB]: Activate icacheopt even in dynamic mode
	  icache_static = (olevel >= 2 ? TRUE: FALSE);
	} else if (icache_mapping)
	  icache_static = FALSE;

	/* If icache_opt is UNDEFINED, set it to TRUE when olevel >= 2*/
        if (icache_opt == UNDEFINED)
	  icache_opt = (olevel >= 2) ? TRUE: FALSE;

	if (icache_opt == UNDEFINED)
	  icache_opt = icache_static || icache_profile  || icache_mapping;

	if (last_phase <= P_any_as) 
	  icache_profile = NULL;

	/* Check that outfile exists with --icache_profile */
	if (icache_profile) {
	  time_t ip_file_mod_time;
	  time_t exe_file_mod_time;
	  string exe_for_icache_profile = icache_profile_exe;
	  exe_for_icache_profile = (exe_for_icache_profile == NULL) ? 
	    (outfile == NULL) ? "a.out" : outfile
	    : exe_for_icache_profile;

	  if (!file_exists(exe_for_icache_profile)) {
	    icache_profile = NULL;
	    warning ("--icache-profile: output file %s does not exist, option ignored ", exe_for_icache_profile);
	  } else {
	    /* Check that trace file is later than executable*/
	    ip_file_mod_time = get_date_of_file(icache_profile);
	    exe_file_mod_time = get_date_of_file(exe_for_icache_profile);
	    if (ip_file_mod_time !=-1 && exe_file_mod_time !=-1 
		&& (ip_file_mod_time < exe_file_mod_time)) {
	      error("executable %s is more recent than trace file %s", exe_for_icache_profile, icache_profile);
	      icache_profile = NULL;
	      perror(program_name);
	    }
	  }
	}
	if (icache_opt == TRUE && ( (icache_static == TRUE) || icache_profile)) {
	  if (icache_algo == algo_COL)
	    flag = add_new_option("--coloring");
	  else if (icache_algo == algo_PH_COL)
	    flag = add_new_option("--ph_col");
	  else if (icache_algo == algo_PH)
	    flag = add_new_option("--ph");
	  else if (icache_profile) {
	    if (icache_algo == algo_TRG)
	      flag = add_new_option("--trg");
	    else if (icache_algo == algo_LTRG)
	      flag = add_new_option("--ltrg");
	  } else {
	    flag = add_new_option("--ph");
	    warning("TRG or LTRG algorithms are only available with --icache-profile=<file>. Applying Pettis & Hansen algorithm.");
	  }

	  add_phase_for_option(flag, P_binopt);
	  prepend_option_seen (flag);
	  if (icache_profile)
	    {
	    /* Add --profile flag */
	      flag = add_new_option("--icacheopt");
	      add_phase_for_option(flag, P_binopt);
	      prepend_option_seen (flag);
	      flag = add_new_option("--profile");
	      add_phase_for_option(flag, P_binopt);
	      prepend_option_seen (flag);

	      flag = add_new_option(icache_profile);
	      add_phase_for_option(flag, P_binopt);
	      prepend_option_seen (flag);
	      flag = add_new_option("--trace");
	      add_phase_for_option(flag, P_binopt);
	      prepend_option_seen (flag);
	    }
	  else if (icache_static == TRUE)
	    {
	      /* Be option */
	      flag = add_new_option("-CG:emit_bb_freqs=on");
	      add_phase_for_option(flag, P_be);
	      prepend_option_seen (flag);
	      /* Add --static flag for binopt*/
	      flag = add_new_option("--icacheopt");
	      add_phase_for_option(flag, P_binopt);
	      prepend_option_seen (flag);
	      flag = add_new_option("--static");
	      add_phase_for_option(flag, P_binopt);
	      prepend_option_seen (flag);
	    } 
	}

        if (deadcode == TRUE || icache_opt == TRUE) {
	  /* AS option */
	  flag = add_new_option("--emit-all-relocs");
	  add_phase_for_option(flag, P_any_as);
	  prepend_option_seen (flag);
	  if (deadcode == TRUE|| icache_static == TRUE || icache_profile || icache_mapping) {
	    /* Add -r for link*/
	    flag = add_new_option("-r");
	    add_phase_for_option(flag, P_any_ld);
	    prepend_option_seen (flag);
	    /* Remove -r for the last linkphase*/
	    remove_phase_for_option(flag, P_ldsimple);
	    if (ipa == TRUE) {
	      remove_phase_for_option(flag, P_ipa_link);
	    }
	    /* [CL] add -produce-relocatable to ipa_link phase */
	    flag = add_new_option("-produce-relocatable");
	    add_phase_for_option(flag, P_ipa_link);
	    prepend_option_seen (flag);
	    /* [SC] Disable relaxation for all but the final linkphase. */
	    remove_phase_for_option(O__relax, P_any_ld);
	    add_phase_for_option(O__relax, P_ldsimple);
	    /* [TB] Disable --shared for all but the final linkphase. */
	    if (shared == DSO_SHARED) {
	      remove_phase_for_option(O_shared, P_any_ld);
	      add_phase_for_option(O_shared, P_ldsimple);
	      // [CL] ipa_link also needs to see -shared
	      if (ipa == TRUE) {
		add_phase_for_option(O_shared, P_ipa_link);
	      }
	    }
	  }
        }
	if (flag_MAP != UNDEFINED) {
	  if (deadcode == TRUE || icache_opt == TRUE) {
	    add_phase_for_option(flag_MAP, P_ldsimple);
	    if (flag_MAPFILE != UNDEFINED)
	      add_phase_for_option(flag_MAPFILE, P_ldsimple);
	  } else {
	    add_phase_for_option(flag_MAP, P_any_ld);
	    if (flag_MAPFILE != UNDEFINED)
	      add_phase_for_option(flag_MAPFILE, P_any_ld);
	  }
	}
	if (flag_M != UNDEFINED) {
	  if (deadcode == TRUE || icache_opt == TRUE) {
	    add_phase_for_option(flag_M, P_ldsimple);
	  } else {
	    add_phase_for_option(flag_M, P_any_ld);
	  }
	}
#endif /* BCO_Enabled Thierry */

#ifdef TARG_ST200
	if (relax == UNDEFINED) {
	  /* TB: in relocatable mode, desactivate --relax */
	  relax = (olevel >= 2 && (shared != RELOCATABLE)) ? TRUE : FALSE;
	  if (relax == TRUE) {
            prepend_option_seen (O__relax);
	  }
	}
#endif
#ifdef TARG_ST
	/* TB: with -fprofile-arcs option, tell be where to generate .gcno files */
	if (option_was_seen(O_fprofile_arcs) || option_was_seen(O_fprofile_arcs_cgir)) {
	  flag = add_new_option(concat_strings("-o,",outfile ? SYS_adirname(outfile) : SYS_getcwd()));
	  add_phase_for_option(flag, P_be);
	  prepend_option_seen (flag);
	}
#endif

	if ((mpkind == NORMAL_MP || auto_parallelize) && !Disable_open_mp) {
		flag = add_string_option(O_D, "_OPENMP=199810");
		prepend_option_seen (flag);
	}

	if (olevel == UNDEFINED) {
		olevel = default_olevel;
		if (olevel == UNDEFINED) {
			/* if no default, use -O0 */
			olevel = 0;
		}
		flag = get_olevel_flag(olevel);
		prepend_option_seen (flag);
	}

	switch (abi) {

	case ABI_N32:
	case ABI_64:
	  /* mips-based */
	  /* can't define _{BIG,LITTLE}_ENDIAN until 6.5.3 */
	  if (!is_toggled(cppundef)) {
	    if (endian == ENDIAN_LITTLE) {
	      flag = add_string_option(O_D, "_MIPSEL");
	      prepend_option_seen (flag);
	      if (ansi != STRICT_ANSI) {
		flag = add_string_option(O_D, "MIPSEL");
		prepend_option_seen (flag);
	      }
	    }
	    else {
	      flag = add_string_option(O_D, "_MIPSEB");
	      prepend_option_seen (flag);
	      if (ansi != STRICT_ANSI) {
		flag = add_string_option(O_D, "MIPSEB");
		prepend_option_seen (flag);
	      }
	    }
	    
	    /* add mips-specific flags */
	    if (ansi != STRICT_ANSI) {
	      flag = add_string_option(O_D, "mips");
	      prepend_option_seen (flag);
	    }
	  }

	  if (!nostdinc) {
	    /* mips only: add -I path for MIPS abi include directory */
	    if (option_was_seen(O_abi)) {
	      flag = add_string_option(O_I__, 
		         concat_path(get_phase_dir(P_include),"abi"));
	      add_option_seen (flag);
	    }
	    /* mips only: add -I path for CC */
	    flag = add_string_option(O_I__, 
			  concat_path(get_phase_dir(P_include),"CC"));
	    set_language_for_option (flag, L_CC);
	    add_option_seen (flag);
	  }

	  if (!is_toggled(gnum)) {
	    /* set gnum default */
	    if (option_was_seen(O_xgot))
	      toggle(&gnum,0);
	    else
	      toggle(&gnum,8);

	    sprintf(buf, "%d", gnum);
	    flag = add_string_option(O_G__, buf);
	    prepend_option_seen(flag);
	  }

	  break;

	case ABI_RAG32:

	  /* add mips-specific flags */
	  if (!is_toggled(cppundef)) {
	    if (ansi != STRICT_ANSI) {
	      flag = add_string_option(O_D, "mips");
	      prepend_option_seen (flag);
	    }
	  }
	  
	  if (!nostdinc) {
	    /* mips only: add -I path for MIPS abi include directory */
	    if (option_was_seen(O_abi)) {
	      flag = add_string_option(O_I__, 
                      concat_path(get_phase_dir(P_include),"abi"));
	      add_option_seen (flag);
	    }
	    /* mips only: add -I path for CC */
	    flag = add_string_option(O_I__, 
                       concat_path(get_phase_dir(P_include),"CC"));
	    set_language_for_option (flag, L_CC);
	    add_option_seen (flag);
	  }

	  if (!is_toggled(gnum)) {
	    /* set gnum default */
	    /* be compatible with ucode */
	    if (shared == NON_SHARED) {
	      toggle(&gnum,8);
	    } else {
	      toggle(&gnum,0);
	    }
	    sprintf(buf, "%d", gnum);
	    flag = add_string_option(O_G__, buf);
	    prepend_option_seen(flag);
	  }
	  break;

	case ABI_I64:
	case ABI_I32:
	case ABI_IA32:

	  if (!is_toggled(gnum)) {
	    /* set gnum default */
	    if (option_was_seen(O_xgot))
	      toggle(&gnum,0);
	    else
	      toggle(&gnum,8);

	    sprintf(buf, "%d", gnum);
	    flag = add_string_option(O_G__, buf);
	    prepend_option_seen(flag);
	  }
	  break;

	case ABI_ST100:

	  /* ST100 does not support GOT for now, set gnum to 0 */
	  if (is_toggled(gnum)) {
	    error ( "-G option is not supported on ST100 platform");
	  }
	  if (shared == NON_SHARED) {
	    error("GOT is not supported on ST100 platforms");
	  }
	  if (option_was_seen(O_KPIC) || 
	      shared == CALL_SHARED ||
	      shared == DSO_SHARED) {
	    /* for now ST100 does not support any shared options */
	    error("shared build is not supported on ST100 platforms");
	  }

	  /* for now ST100 does not support GP-relative addressing */
	  if (gprel == TRUE) {
	    error("GP-relative addressing is not supported on ST100 platforms");
	  }

	  /* ST100 does not support big endian code generation */
	  if (endian == ENDIAN_BIG) {
	    error("BIG endian mode is not supported on ST100 platforms");
	  }

	  if (!is_toggled(cppundef)) {
	    // add ST100 specific flags
	    flag = add_string_option(O_D, "__ST100");
	    prepend_option_seen (flag);
	  }
	  break;

	case ABI_ST200_embedded:
	case ABI_ST200_PIC:
	  /* ST200 does not have restrictions on GOT size so no need for -G. */
	  if (is_toggled(gnum)) {
	    error ( "-G option is not supported on ST200 platform");
	  }
#ifdef RELEASE
#if defined(__NO_BIGENDIAN)
	  /* 
	   * [CM] ST200 does support big endian code generation, so we authorize this flag,
	   * unless __NO_BIGENDIAN is defined. This is to be finally removed.
           */
	  if (endian == ENDIAN_BIG) {
	    error("BIG endian mode is not supported on ST200 platforms");
	  }
#endif /*__NO_BIGENDIAN */
	  /* ST200 currently supports -O0,-O1, -O2 optimization levels */
	  if (olevel > 3) {
	    error("Optimization level > 3 is not currently supported");
	  }
#endif

	  /* 
	   * On ST200 we need to pass the endianness down the
	   * toolchain
	   */
	  if (endian == ENDIAN_LITTLE) {
	    if (!is_toggled(cppundef)) {
	      flag = add_string_option(O_D, "__LITTLE_ENDIAN__");
	      prepend_option_seen (flag);
	    }
#ifdef TARG_ST
	    /* (cbr) done in OPTIONS */
	    if (!option_was_seen(O_EL))
#endif
	      prepend_option_seen(O_EL);

	    //prepend_option_seen(O_mlittle_endian);
	  }
	  else {
	    if (!is_toggled(cppundef)) {
	      flag = add_string_option(O_D, "__BIG_ENDIAN__");
	      prepend_option_seen (flag);
	    }
#ifdef TARG_ST
	    /* (cbr) done in OPTIONS */
	    if (!option_was_seen(O_EB))
#endif
	      prepend_option_seen(O_EB);

	      //prepend_option_seen(O_mbig_endian);
	  }

	  /*
	   * ST200 specific flags: 
	   * Note that the open64 defines are not really st200-specific
	   */
	  if (!is_toggled(cppundef)) {
	    if (proc == PROC_ST210) {
              flag = add_string_option(O_D, "__ST210__");
	      prepend_option_seen (flag);
 	      flag = add_string_option(O_D, "__st210__");
	      prepend_option_seen (flag);
	    }
	    if (proc == PROC_ST220) {
 	      flag = add_string_option(O_D, "__ST220__");
	      prepend_option_seen (flag);
 	      flag = add_string_option(O_D, "__st220__");
	      prepend_option_seen (flag);
	    }
	    else if (proc == PROC_ST221) {
 	      flag = add_string_option(O_D, "__ST221__");
	      prepend_option_seen (flag);
 	      flag = add_string_option(O_D, "__st221__");
	      prepend_option_seen (flag);
	    }
	    else if (proc == PROC_ST231) {
 	      flag = add_string_option(O_D, "__ST231__");
	      prepend_option_seen (flag);
 	      flag = add_string_option(O_D, "__st231__");
	      prepend_option_seen (flag);
	    } 	    
	    else if (proc == PROC_ST240) {
 	      flag = add_string_option(O_D, "__ST240__");
	      prepend_option_seen (flag);
 	      flag = add_string_option(O_D, "__st240__");
	      prepend_option_seen (flag);
	    } 	    
#ifdef MUMBLE_ST200_BSP
	    if (st200_runtime == RUNTIME_BARE) {
	      flag = add_string_option(O_D, "__BARE_BOARD__");
	      prepend_option_seen (flag);
	    }
	    else if (st200_runtime == RUNTIME_OS21 || st200_runtime == RUNTIME_OS21_DEBUG) {
	      flag = add_string_option(O_D, "__os21__");
	      prepend_option_seen (flag);
	      flag = add_string_option(O_D, "HAVE___REENT_FN");
	      prepend_option_seen (flag);
	      flag = add_string_option(O_D, "__OS21_BOARD__");
	      prepend_option_seen (flag);
	    }
#endif

	    flag = add_string_option(O_D, "__st200__");
	    prepend_option_seen (flag);
	    flag = add_string_option(O_D, "__ST200__");
	    prepend_option_seen (flag);
	    
	    /* [CM] Kept for compatibility */
	    flag = add_string_option(O_D, "__ST200");
	    prepend_option_seen (flag);
	    
	    flag = add_string_option(O_D, "__open64__");
	    prepend_option_seen (flag);

	    // (cbr) we are in a elf environment, say it
	    flag = add_string_option(O_D, "__ELF__");
	    prepend_option_seen (flag);

#ifdef TARG_ST200
	    /* This part depends on macro defined only for TARG_ST200
	       thus we conditionalize it.  */
	    flag = add_string_option(O_D, ST200CC_EXPAND__(__ST200CC__, ST200CC_MAJOR__));
	    prepend_option_seen (flag);

	    flag = add_string_option(O_D, ST200CC_EXPAND__(__ST200CC_MINOR__, ST200CC_MINOR__));
	    prepend_option_seen (flag);

	    flag = add_string_option(O_D, ST200CC_EXPAND__(__ST200CC_PATCHLEVEL__, ST200CC_PATCHLEVEL__));
	    prepend_option_seen (flag);

	    flag = add_string_option(O_D, ST200CC_EXPAND__(__ST200CC_DATE__, ST200CC_DATE__));
	    prepend_option_seen (flag);

	    flag = add_string_option(O_D, ST200CC_EXPAND__(__ST200CC_VERSION__, ST200CC_VERSION__));
	    prepend_option_seen (flag);
#endif

	    /* [CM] Must know the final optimization level to treat that expansion properly */
#if 0
	    // (cbr) this is now handled by the preprocessor 
	    if (olevel >= 1) {
		flag = add_string_option(O_D, "__OPTIMIZE__");
		prepend_option_seen (flag);
	    }
	    /* [CM] See OPTIONS : osize is only significant when olevel is 2*/
	    if ((olevel == 2) && (osize >= 1)) {
		flag = add_string_option(O_D, "__OPTIMIZE_SIZE__");
		prepend_option_seen (flag);
	    }
#endif

	  } /* !is_toggled(cppundef) */
	  break;

	case ABI_STxP70_embedded:
	case ABI_STxP70_fpx:
	  /*
	   * STxP70 specific flags: 
	   */
	  if(endian == ENDIAN_BIG) {
	    error("BIG endian mode is not supported on STxP70 platforms");
	  }

	  if (!option_was_seen(O_EL)) {
	    prepend_option_seen(O_EL);
	  }

	  if (!is_toggled(cppundef)) {
	    if (proc == PROC_stxp70 || proc == PROC_stxp70_ext) {
              flag = add_string_option(O_D, "__stxp70__");
	      prepend_option_seen (flag);
 	      flag = add_string_option(O_D, "__STXP70__");
	      prepend_option_seen (flag);
	      flag = add_string_option(O_D, "__STxP70__");
	      prepend_option_seen (flag);
	      flag = add_string_option(O_D, "__SX");
	      prepend_option_seen (flag);
	      flag = add_string_option(O_D, "__X3");
	      prepend_option_seen (flag);
	    }
	    
	    flag = add_string_option(O_D, "__open64__");
	    prepend_option_seen (flag);

	    // (cbr) we are in a elf environment, say it
	    flag = add_string_option(O_D, "__ELF__");
	    prepend_option_seen (flag);

	    flag = add_string_option(O_D, "__LITTLE_ENDIAN__");
	    prepend_option_seen (flag);

#ifdef TARG_STxP70
	    /* This part depends on macro defined only for TARG_STxP70
	       thus we conditionalize it.  */
	    flag = add_string_option(O_D, STXP70CC_EXPAND__(__STXP70CC__, STXP70CC_MAJOR__));
	    prepend_option_seen (flag);

	    flag = add_string_option(O_D, STXP70CC_EXPAND__(__STXP70CC_MINOR__, STXP70CC_MINOR__));
	    prepend_option_seen (flag);

	    flag = add_string_option(O_D, STXP70CC_EXPAND__(__STXP70CC_PATCHLEVEL__, STXP70CC_PATCHLEVEL__));
	    prepend_option_seen (flag);

	    flag = add_string_option(O_D, STXP70CC_EXPAND__(__STXP70CC_DATE__, STXP70CC_DATE__));
	    prepend_option_seen (flag);

	    flag = add_string_option(O_D, STXP70CC_EXPAND__(__STXP70CC_VERSION__, STXP70CC_VERSION__));
	    flag = add_string_option(O_g0, "");
	    prepend_option_seen (flag);
#endif
	  } /* !is_toggled(cppundef) */
	  break;

	default:
	  error ( "invalid ABI: %d", abi );
	  break;
	}
#ifndef TARG_ST
	/* fix COMPILER_VERSION: */
	FOREACH_IMPLIED_OPTION(flag, O_cpp_version) {
		s = get_current_implied_name();
		if (contains_substring (s, "XXX")) {
		   /* This handles the case of 6.1 vs. 6.02, etc.
		    * better. Richard Shapiro, 8/23/95
		    */
		   buf[0] = compiler_version[0];
		   buf[1] = compiler_version[2];
		   buf[2] = compiler_version[3] ?  compiler_version[3] : '0';
		   buf[3] = '\0';
		   replace_substring (s, "XXX", buf);
		}
	}
#endif
	/* some checks are easier to do by hand */
	if (olevel >= 2 && glevel == 2) {
#ifndef sgi
		glevel = 3;
		if (option_was_seen (O_g))
			replace_option_seen (O_g, O_g3);
		if (option_was_seen (O_g2))
			replace_option_seen (O_g2, O_g3);
#else
		turn_down_opt_level(0, "-O conflicts with -g; changing to -O0; if you want -O, use -g3");
#endif
	}

	if (option_was_seen(O_S) && ipa == TRUE) {
		turn_off_ipa ("-IPA -S combination not allowed, replaced with -S");
	}
	if (option_was_seen(O_S) && lai == TRUE) {
	  turn_off_lai ("-lai -S combination not allowed, replaced with -S");
	}
#ifdef IPA_PROFILING_O3_NOT_COEXIST // [CL] merge from Open64-0.15
	if (instrumentation_invoked == TRUE) {
	    if (ipa == TRUE) {
		inline_t = FALSE;
		turn_off_ipa ("-fb_create requires no -IPA");
	    }
	    if (olevel > 2)
		turn_down_opt_level (2, "-fb_create conflicts with -Ofast/-O3; changing to -O2");
	}
#endif /* IPA_PROFILING_O3_NOT_COEXIST */
	if (Gen_feedback && olevel > 0) {
		turn_down_opt_level(0, "-fbgen conflicts with -O; changing to -O0");
	}
	if (Gen_feedback && ipa == TRUE) {
		turn_off_ipa ("-IPA -fbgen combination not allowed, replaced with -fbgen");
	}
	if (glevel == 2 && ipa == TRUE) {
		turn_off_ipa ("-IPA -g combination not allowed, replaced with -g");
	}

	if (ipa == TRUE) {
	    if (olevel <= 1)
		flag = add_string_option (O_PHASE_, "i");
	    else
		flag = add_string_option (O_PHASE_, "p:i");
	} else {
	    /*
	     * Determine which back end phase(s) need to be run.
	     *
	     *			-O0/-O1	-O2		-O3
	     *			===========================
	     *		.B,.I:	cg	wopt/cg		lno/wopt/cg
	     *		.N:	cg	wopt/cg		wopt/cg
	     *		.O:	cg	cg		cg
	     */
	    if (source_kind == S_O)
		warning("compiles of WOPT-generated .O files will usually fail due to missing state information");
	    if (olevel <= 1 || source_kind == S_O)
		flag = add_string_option(O_PHASE_, "c");
	    else if (olevel == 2 || source_kind == S_N)
		flag = add_string_option(O_PHASE_, "w:c");
	    else 
		flag = add_string_option(O_PHASE_, "l:w:c");
	}
	prepend_option_seen (flag);

	if (abi == ABI_N32 || abi == ABI_64) {
        	set_dsm_options ();
	}

	if (option_was_seen(O_ar) && outfile == NULL) {
	   error("-ar option requires archive name to be specified with -o option");
	}
	
	if (lai == TRUE && isa != ISA_ST100) {
	  error("-lai is not supported on this platform");
	}

	if (skip_as == UNDEFINED) {
		/* for mips we generate object directly;
		 * for other targets we go thru as. */
		if (abi == ABI_N32 || abi == ABI_64)
			skip_as = TRUE;
		else
			skip_as = FALSE;
	}
	if (skip_as == FALSE && ! keep_flag && last_phase != P_be) {
		/* if going thru asm and not keeping .s file,
		 * then don't print extra notes and source */
		flag = add_string_option(O_LIST_, "source=off:notes=off");
		prepend_option_seen (flag);
	}

	if (option_was_seen(O_static) && invoked_lang == L_f90) {
		/* IRIX f90 -static was fe option,
		 * but gnu makes it a link option.
		 * Warn about possible misuse. */
		warning("Under Linux, -static is a linker option for using static libraries; if you wanted to put local data in static area, use -static-data instead");
	}
}

#ifdef MUMBLE_ST200_BSP
extern void
set_st200_bsp (string_list_t *args)
{
  extern string st200_core, st200_soc, st200_board;
  string ofile;

  // Need to make this conditional because -mcore is also
  // used for compiler's code generation.
  ofile = concat_path (st200_core, "bootcore.o");
  if (file_exists (ofile))
    add_string (args, ofile);

  ofile = concat_path (st200_soc, "bootsoc.o");
  if (file_exists (ofile))
    add_string (args, ofile);

  ofile = concat_path (st200_board, "bootboard.o");
  if (file_exists (ofile))
    add_string (args, ofile);
}
#endif

#ifdef MUMBLE_STxP70_BSP
extern void
set_stxp70_bsp (string_list_t *args)
{
  // No BSP specific .o files for STxP70.
}
#endif

