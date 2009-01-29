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
#include <strings.h>
#include <stamp.h>
#include <cmplrs/rcodes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "W_errno.h"
#include "W_times.h"
#include "string_utils.h"
#include "options.h"
#include "option_seen.h"
#include "option_names.h"
#include "opt_actions.h"
#include "get_options.h"
#include "lang_defs.h"
#include "errors.h"
#include "phases.h"
#include "file_utils.h"
#include "file_names.h"
#include "run.h"
#include "objects.h"
#include "libiberty/libiberty.h"

string help_pattern = NULL;
boolean debug = FALSE;
boolean nostdinc = FALSE;
#ifdef TARG_ST
/* (cbr) don't exclude non c++ standard paths */
boolean nostdincc = FALSE;
#endif
boolean show_version = FALSE;
boolean dump_version = FALSE;
boolean dump_machine = FALSE;

extern void check_for_combos(void);
extern boolean is_replacement_combo(int);
extern void toggle_implicits(void);
extern void set_defaults(void);
extern void add_special_options (void);

static void check_old_CC_options (string name);
static void check_makedepend_flags (void);
static void mark_used (void);
static void dump_args (string msg, FILE *file);
static void print_help_msg (void);

static string_list_t *files;
static string_list_t *file_suffixes;
string_list_t *feedback_files;

static char compiler_version[] = INCLUDE_STAMP;

#ifdef TARG_ST
#if defined(TARGET_LINUX)
#define THREAD_MODEL "posix"
#elif defined(TARGET_OS21)
#define THREAD_MODEL "generic"
#else
#define THREAD_MODEL "single"
#endif
static const char thread_model[] = THREAD_MODEL ;
#endif

static struct tms tm0, tm1;
static clock_t time0, time1;


#ifdef TARG_STxP70
extern void command_line_copy(int argc, char **argv);
#endif

/* treat_one_arg: Treat one command line argument 
 * Extracted in order to be able to call it for an
 * argument that is not really on command line but has
 * been build during extension command line parsing for
 * STxP70 target.
 */
extern void treat_one_arg ( int * argc, char *argv[] ) {
   int flag;
	int base_flag;

   option_name = argv[*argc];
   flag = get_option(argc, argv);
	if (flag == O_Unrecognized) { 
		if (print_warnings) {
		    /* print as error or not at all? */
		    parse_error(option_name, "unknown flag");
		}
	}
	else {
		/* reset option name to possibly include 
		 * 2nd part, e.g. -G 8 */
		option_name = get_option_name(flag);
	}
	/* sometimes is simple alias to another flag */
   flag = get_real_option_if_aliased (flag);

	/* sometimes need to look at parent flag */
	if (is_derived_option (flag)) {
		base_flag = get_derived_parent(flag);
		/* sometimes base is simple alias */
		base_flag = get_real_option_if_aliased (base_flag);
	}
	else {
		base_flag = flag;
	}

	if (is_object_option(base_flag)) {
		/* put in separate object list */
		add_object (base_flag, optargs);
		source_kind = S_o;
	} else {
		/* add unique real flag to list */
		add_option_seen (flag);
	}
	if (base_flag == O_generate_instantiation_info) {
     /* This is a def_list_file option that is being
		passed by the prelinker to the frontend.
		It should not be recorded in the command
		line. Therefore cancel the saved arg. */
    cancel_saved_arg(1);
	}

	opt_action(base_flag);

}

static char relocatable_str1[1024];
static char relocatable_str2[1024];
static char *script_argv[] = { relocatable_str1, relocatable_str2 };

extern int 
main (int argc, char *argv[])
{
	int i;		/* index to argv */
	string_item_t *p, *q;
	int num_files = 0;

	time0 = times(&tm0);
	save_command_line(argc, argv);		/* for prelinker    */	
#ifdef TARG_STxP70
	command_line_copy(argc,argv);       /* in case V[<ver>] is invoked */
#endif
	program_name = drop_path(argv[0]);	/* don't print path */
	orig_program_name = string_copy(argv[0]);
	files = init_string_list();
	file_suffixes = init_string_list();
	feedback_files = init_string_list ();	/* for cord feedback files */
	init_options();
	init_temp_files();
	init_count_files();
	init_option_seen();
	init_objects();

	invoked_lang = get_named_language(program_name);
	check_for_driver_controls (argc, argv);

	if (abi == ABI_I64 || abi == ABI_I32) {
                /* prepend i64 compiler and include path */
#if !defined(linux) && !defined(sun) && !defined(__CYGWIN__) && !defined(__MINGW32__)
                prefix_all_phase_dirs(PHASE_MASK, "/m2i");
		prefix_all_phase_dirs(LIB_MASK, "/ia64");
#endif
	}
#if 0
	if (abi == ABI_IA32) {
                /* prepend i32 compiler and include path */
                prefix_all_phase_dirs(PHASE_MASK, "/m2i");
		prefix_all_phase_dirs(LIB_MASK, "/ia32");
	}
#endif

	/* can't add toolroot until other prefixes */
	init_phase_info(argv[0]); 

        /* Hack for F90 ftpp; For pre processing F90 calls ftpp;
         * Unlike cpp, ftpp does not like the -Amachine(mips) and -Asystem(unix)
         * that are passed to cpp; we need to remove these if ftpp is used for
         * preprocessing. Also remove all the woff options for cpp and ftpp.
         */

	remove_phase_for_option(O_A,P_f90_cpp);
	remove_phase_for_option(O_E,P_f90_cpp);

	i = 1;
	while (i < argc) {
		option_name = argv[i];
		set_current_arg_pos(i);
		if (argv[i][0] == '-' && !dashdash_flag) {
			treat_one_arg(&i,argv);
         
		} else if (argv[i][0] == '+') {
			check_old_CC_options(argv[i]);
			i++;
		} else {
#ifdef TARG_STxP70
         // [HC]: Try to take .ld files as link script files and automatically
         //       inserting -T ahead.
         // [YJ]: We used to call directly the following routines (with the
         //       appropriate arguments):
         //         add_script_file and add_option_seen
         //
         //       However, this method doesn't work anymore for IPA.
         //       Hereafter, we modify the current argument so as to emulate
         //       what happens with argument -Txxx.ld       
         char  *suffix_ptr;
         int    org_index;
         char  *sav;
         char  *tmp;

         if ((NULL!=(suffix_ptr=get_suffix(argv[i]))) && 
             !strcmp(suffix_ptr,"ld")) {

            org_index=i;
            sav = argv[i];

            tmp = malloc(strlen("-T")+strlen(argv[i])+1);
            if(NULL==tmp) { 
             internal_error("memory allocation error.");
             exit(1);
            }
            strcpy(tmp,"-T");
            strcat(tmp,argv[i]);
            argv[i] = tmp;

            treat_one_arg(&i,argv);

            argv[org_index] = sav; // Restore initial state in argv table.
            i = org_index ;        // In particular, it is very important
                                   // to restore i (otherwise next argument
                                   // won't be parsed).
            free(tmp);
			} else {
#endif
			source_kind = get_source_kind(argv[i]);
			/* if -E used, then preprocess anything, even .o's */
			if (last_phase == P_any_cpp && source_kind == S_o) 
			{
				source_kind = S_c;
			}
			if (source_kind == S_o) {
				/* object file or library */
				add_object (O_object, argv[i]);
				/* Save away objects should it be necessary
				   to invoke the archive phase (-ar option). */
				    add_ar_objects(argv[i]);
			} else {
				/*
				 * Reserve place in object list for .o,
				 * so multiple .o's have same position
				 * relative to other parameters.
				 * If only one source file, then will
				 * treat this as a temp file.
				 */
            /* fix #14600: reserve tempfile name for .o if 
             *             necessary, and associate it with 
             *             original source file 
             */
				char *obj_name = make_temp_file(".o");

            associate_src_object(argv[i],obj_name);
				add_object (O_object, obj_name);
				add_ar_objects(obj_name);
				add_string(files, argv[i]);
				/* Because of -x <lang> option,
				 * we need position as well as name to
				 * determine a file's source kind.
				 * So we want to record the source_kind now,
				 * before we lose the position info.
				 * Thus have parallel list of file suffixes,
				 * which will be the known suffix that we
				 * want to treat this file as, e.g.
				 * -x assembler t.asm will give suffix "s".
				 * Use string_list just cause it is readily
				 * available. */
				add_string(file_suffixes, 
					get_suffix_string(source_kind));
				num_files++;
			}
#ifdef TARG_STxP70
		} // Associated to trial to take .ld files as link script ones 
		  // and automatically inserting -T ahead.
#endif
			cancel_saved_arg(1);
			i++;
		}
	}

	/* Check target specifications for consistency: */
	Check_Target ();

#if defined( TARG_STxP70 )

   /* Temporary: deactivate IPA for STxP70.
    */
    if(TRUE==ipa) {
       error("ipa is not supported yet for the STxP70 target");
       exit(RC_USER_ERROR);
    }

   /* At this point, add TENV for extension if it exists as if it was
    * on standard command line...
    */
   {
      extern int UserDefinedExtensionSeen;
      extern char TENVExtension_str[];
      if (UserDefinedExtensionSeen) {
         char *newargv[] = { TENVExtension_str };
         int  i = 0;
         
   		treat_one_arg(&i,newargv);
      }
   }

   /* At this point, add -Wy,-T,<link_scripts>.reloc in case of
    * binopt activation
    */
   if (!option_was_seen(O_S) && !option_was_seen(O_c)) {
      extern string_list_t *script_files;
      string_item_t *p;
      int i;
      
      p = script_files->head;
      if (NULL == p) {
         string spath;
         string ofile;
	 
         spath = get_phase_dir(P_library);
         spath = concat_path (spath, "ldscript");
	 if (shared == RELOCATABLE) {
            ofile = concat_path (spath, "sx_valid.ld.reloc");
	 } else {
            ofile = concat_path (spath, "sx_valid.ld");
	 }
	 strcpy(relocatable_str1,"-T");
	 snprintf(relocatable_str2,1024,"%s",ofile);
	 i = 0;
	 treat_one_arg(&i,script_argv);
      }
      p = script_files->head;
      if ((NULL != p ) && 
          ((deadcode==TRUE) || 
           ((olevel>=2) && (deadcode==UNDEFINED))
	  )
	 ) {
	 if (shared == RELOCATABLE) {
            snprintf(relocatable_str1,1024,"%s",p->name);
	 } else {
            snprintf(relocatable_str1,1024,"%s.reloc",p->name);
	 }
	 if (!file_exists(p->name)) {
	    error("Link script file required: %s",p->name);
	 }
         if (!file_exists(relocatable_str1)) {
            warning("Relocatable link script file required: %s\nPlease read \"HowTo write associated relocatable link script for BinOpt\"",relocatable_str1);
	 } else {
  	    if (shared == RELOCATABLE) {
               snprintf(relocatable_str1,1024,"-Wy,-T,%s",p->name);
	    } else {
               snprintf(relocatable_str1,1024,"-Wy,-T,%s.reloc",p->name);
	    }
            i = 0;
            treat_one_arg(&i,script_argv);
            treat_one_arg(&i,script_argv); // Required two times!
	 }
      }
   }
#endif

#ifdef TARG_ST
	if (dump_machine) {
#ifdef TARG_ST200
	    fprintf(stdout, "st200\n") ;
#elif defined( TARG_STxP70 )
	    fprintf(stdout, "stxp70\n") ;
#else
	    fprint(stdout, "unknown\n") ;
#endif
	    /* gcc behavior for multiple -dump options is to dump the first and stop */
	    exit(RC_OKAY) ;
	}
#endif

        if (show_version) {
            /* Echo information about the compiler version */
#ifdef TARG_ST
	  FILE *output=stdout;
	  if (execute_flag) output=stderr;
	  if (!dump_version) {
	      fprintf(output, "STMicroelectronics Compilers: Version %s\n", compiler_version);
	      fprintf(output, "Pro64 OpenSource 0.01.0-13\n");
	      fprintf(output, "Thread model: %s\n", thread_model) ;
	      if (invoked_lang == L_CC)
		  fprintf(output, "using g++ version 3.3.3 front end\n");
	      else
		  fprintf(output, "using gcc version 3.3.3 front end\n");
	  } else {
	      if (invoked_lang == L_CC) 
		  fprintf(output, "3.3.3\n");
	      else
		  fprintf(output, "3.3.3\n");
	  }
#else
#ifndef mips
            fprintf(stderr, "SGIcc Compilers: Version %s\n", compiler_version);
#else
            fprintf(stderr, "MIPSpro Compilers: Version %s\n", compiler_version);
#endif
#endif
        }
	if (option_was_seen(O_show_defaults)) {
		/* TODO: print default values */
		exit(RC_OKAY);
	}

	if (argc == 1 || option_was_seen(O_help) || option_was_seen(O__help) 
#ifdef TARG_STxP70
      || option_was_seen(O_full_help)
#endif
		|| help_pattern != NULL) 
	{
		print_help_msg();
	}

#ifndef TARG_ST
	if ( ! execute_flag && ! show_flag) {
		exit(RC_OKAY);	/* just exit */
	}
	if (source_kind == S_NONE) {
	        if (show_version) {	/* just exit */
	            exit(RC_OKAY);
		}
		if (read_stdin) {
			source_file = "-";
			if (option_was_seen(O_E)) {
				source_lang = L_cpp;
			}
			else {
				source_kind = get_source_kind(source_file);
				source_lang = invoked_lang;
			}

		}
		else {
			error("no source or object file given");
		}
	}
#endif
	/* if toggle flags have superceded previous flags,
	 * or if option has been repeated,
	 * unmark the previous flags:
	 */
	FOREACH_OPTION_SEEN(i) {
		if (current_option_seen_later(i)) {
			set_current_option_unseen();
		} else if (flag_is_superceded(i)) {
			set_option_unseen(i);
		}
	}

	/* check for certain combinations of options */
	check_for_combos();

	if (debug) {
		dump_args("user flags", stdout);
	}
	if (ipa == TRUE)
	    save_ipl_commands ();

	/* if user has specified feedback files, turn on cord */
	if (feedback_files->head) cordflag=TRUE;

	/* if cord is not on, set last phase to ld */
	if (cordflag!=TRUE) {
	     last_phase=earliest_phase(P_any_ld, last_phase);
         }

	/* mark the used options */
	mark_used();

	/* call toggle routine for implicitly-used options */
	toggle_implicits();

	/* add defaults if not already set */
	set_defaults();

	if (num_files > 1) {
		multiple_source_files = TRUE;
	}
	/* handle anything else */
	check_makedepend_flags ();
	add_library_options();
	add_special_options();

	if (debug) {
		dump_args("with defaults", stdout);
		dump_objects();
	}
	if (has_errors()) return error_status;

#ifdef TARG_ST
	/* Treat -print options. */
	if (print_kind != PRINT_NONE && print_name != NULL) {
	  if (print_kind == PRINT_FILE) 
	    print_file_path(print_name);
	  else if (print_kind == PRINT_PROG) 
	    print_file_path(print_name);
	}
	
	/* Even if we don't execute, we process options. */

	if ( ! execute_flag && ! show_flag) {
	  exit(RC_OKAY);	/* just exit */
	}

	if (source_kind == S_NONE) {
	  if (read_stdin) {
	    source_file = "-";
	    ignore_suffix = TRUE;
#ifdef TARG_ST
	    /* (cbr) don't force lang. DDTSst21455, st21438 */
	    if ( ! option_was_seen(O_E))
	      source_kind = default_source_kind;
	    else
#endif
	    source_kind = get_source_kind(source_file);
	    source_lang = get_source_lang(source_kind);
	  } else if (show_version) {	/* just exit */
	    exit(RC_OKAY);
	  } else {
	    error("no source or object file given");
	  }
	}
#endif
	catch_signals();
	remember_last_phase = last_phase;

/* for DRA (Distributed Reshape Array) templitization, we want to
 * run prelinker and ld later
 * ??? why not just have ar and dsm prelink be set in determine_phase_order?
 */
	if ((multiple_source_files || 
	     option_was_seen(O_ar) ||
             option_was_seen(O_dsm)) && 
#ifndef TARG_ST
	     ((last_phase == P_any_ld) && (shared != RELOCATABLE)) || 
#else /* TARG_ST */
	    /* TB: fix -r option*/
	     ((last_phase == P_any_ld) /* && (shared != RELOCATABLE) */) || 
#endif /* TARG_ST */
	     (last_phase == P_pixie)) {
		/* compile all files to object files, do ld later */
		last_phase = P_any_as;
		add_minus_c_option();	/* for .ii file */
	}

	if (Use_feedback) {
	   struct stat stat_buf;
	   time_t fb_file_mod_time;
	   time_t count_file_mod_time;
	   boolean fb_file_exists = TRUE;

	   if (fb_cdir != NULL) 
	      warning ("-fb_cdir cannot be used with -fbuse; -fb_cdir ignored");
	   save_name(&fb_file, concat_strings(drop_path(prof_file), ".x.cfb"));
	   if (!(stat(fb_file, &stat_buf) != 0 && errno == ENOENT))
#ifndef sgi
		fb_file_mod_time = stat_buf.st_mtime;
#else
		fb_file_mod_time = stat_buf.st_mtim.tv_sec;
#endif
           else
		fb_file_exists = FALSE;
           if (!(stat(count_files->head->name, &stat_buf) != 0 && errno == ENOENT))
#ifndef sgi
		count_file_mod_time = stat_buf.st_mtime;
#else
		count_file_mod_time = stat_buf.st_mtim.tv_sec;
#endif
           else {
		internal_error("%s doesn't exist", count_files->head->name);
		perror(program_name);
           }

	   if (!fb_file_exists || (fb_file_mod_time <= count_file_mod_time))
	       run_prof();
        }

	if (read_stdin) {
		if ( option_was_seen(O_E) 
#if defined (TARG_ST) && (GNU_FRONT_END==33)
		     /* (cbr) gcc 3.3.3 compatibility */
		     || (source_lang != L_NONE))
#else
			|| (source_lang != L_NONE && source_kind != S_o)) 
#endif
		{
			run_compiler();
		}
		else {
#if defined (TARG_ST) && (GNU_FRONT_END==33)
		     /* (cbr) gcc 3.3.3 compatibility */
		  error ("-E required when input is from standard input");
#else
		  error("-E or specified language required when input is from standard input");
#endif
		}
		cleanup();
		return error_status;
	}

	for (p = files->head, q=file_suffixes->head; p != NULL; p = p->next, q=q->next) 
	{
		source_file = p->name;
		if (multiple_source_files) {
			fprintf(stderr, "%s:\n", source_file);
		}
		if (execute_flag && !file_exists(source_file)) {
			error("file does not exist:  %s", source_file);
			continue;
		}
		source_kind = get_source_kind_from_suffix(q->name);
		source_lang = get_source_lang(source_kind);
#ifdef TARG_STxP70
		if (source_lang ==  L_CC) {
		  error("Compiler does not support language of source file: C++");
		  exit(RC_OKAY);
		}
#endif
		if (source_lang != invoked_lang
			&& source_lang != L_as
			&& (fullwarn || (source_lang == L_f90)) )
		{
			warning("compiler not invoked with language of source file; will compile with %s but link with %s", get_lang_name(source_lang), get_lang_name(invoked_lang));
		}
		run_compiler();
		if (multiple_source_files) cleanup();
	}
	if (has_errors()) {
		cleanup();
		return error_status;
	}

	if (num_files == 0 || remember_last_phase != last_phase) {
#ifdef BCO_ENABLED /* Thierry */
	  if (deadcode == TRUE || (icache_opt == TRUE  && (icache_static == TRUE|| icache_profile || icache_mapping)))
	    need_icache_run_ld = TRUE;
#endif /* BCO_Enabled Thierry */
		/* run ld */
		last_phase = remember_last_phase;
		source_file = NULL;
		source_kind = S_o;
		source_lang = get_source_lang(source_kind);

		if (option_was_seen(O_dsm_clone)) {
          	    run_dsm_prelink();
          	    if (has_errors()) {
                      cleanup();
                      return error_status;
                    }
                }
		if (option_was_seen(O_ar)) {
		   run_ar();
		}
		else {
		    run_ld ();
		}
		if (Gen_feedback)
		  run_pixie();
	}

	cleanup();
   cleanup_src_objects();
	time1 = times(&tm1);
        if (time_flag) {
	  double utime, stime, wtime;
	  utime = (double)(tm1.tms_utime + tm1.tms_cutime -
			   tm0.tms_utime - tm0.tms_cutime) / (double)CLOCKS_PER_SEC;
	  stime = (double)(tm1.tms_stime + tm1.tms_cstime -
			   tm0.tms_stime - tm0.tms_cstime) / (double)CLOCKS_PER_SEC;
	  wtime =  (double)(time1 - time0)/ (double)CLOCKS_PER_SEC;
	  fprintf (stderr, "%s total time:  %.3fu %.3fs %.3fe %.2f%%\n",
		   program_name, utime, stime, wtime,
		   ((utime + stime) / wtime) * 100.0);
	}
	return error_status;
}


static void
check_old_CC_options (string name)
{
	if (same_string(name, "+I")) {
		warn_no_longer_supported2(name, "-keep");
	} else if (same_string(name, "+L")) {
		warn_no_longer_supported(name);
	} else if (same_string(name, "+d")) {
		warn_no_longer_supported2(name, "-INLINE:none");
	} else if (same_string(name, "+p")  ||
	           same_string(name, "+pc") ||
	           same_string(name, "+pa")) {
		warn_ignored(name);
		warning("the effect of +p is now the default (see -anach and -cfront)");
	} else if (same_string(name, "+v")) {
		warn_no_longer_supported2(name, "-show");
	} else if (same_string(name, "+w")) {
		warn_no_longer_supported2(name, "-fullwarn");
	} else if (same_string(name, "+a0")) {
		warn_no_longer_supported(name);
	} else if (same_string(name, "+a1")) {
		warn_no_longer_supported(name);
	} else {
		parse_error(name, "bad syntax");
	}
}

/*
 * Kludges for MDupdate/target/ignore support;
 * we put this here rather than special_options
 * cause needs to refer to files list.
 */
static void
check_makedepend_flags (void)
{
	int flag;
	if (option_was_seen(O_MDupdate)) {
	    if (outfile != NULL) {
		/* if compiling to something other than .o,
		 * then add MDtarget info */
		/* ??? should we add even if user gives -MDtarget?
		 * ??? sherwood does this, so I guess is okay. */
		flag = add_string_option(O_MDtarget, outfile);
		add_option_seen (flag);
	    } 
	    if (!multiple_source_files && files->head != NULL 
		&& last_phase == P_any_ld)
	    {
		/* if compiling .c to a.out, 
		 * don't put .o in Makedepend list */
		if (outfile == NULL) {
			string s = change_suffix(files->head->name, NULL);
			s[strlen(s)-1] = NIL;	/* drop . of suffix */
			flag = add_string_option(O_MDtarget, s);
			add_option_seen (flag);
		}
		flag = add_string_option(O_MDignore, 
			get_object_file(files->head->name) );
		add_option_seen (flag);
	    }
	} 
}

/* mark all implied options as implicitly seen */
static void
mark_used (void)
{
  int i;
  int iflag;

  FOREACH_OPTION(i) {
    if (option_was_seen(i) && !option_was_implicitly_seen(i)) {
      FOREACH_IMPLIED_OPTION(iflag, i) {
	if (option_was_seen(iflag)) {
		continue;	/* already on list */
	}
	add_option_implicitly_seen (iflag);
	if (is_object_option(iflag)
	  && option_matches_language(i, invoked_lang) )
	{
	  /* put in object list. */
	  /* can assume it is ld option. */
	  /* name is full name, so cheat
	   * by saying it is an object,
	   * even if is really -lname. */
	  add_object (O_object, get_current_implied_name());
	}
      }
    }
  }
}

static void
print_help_msg (void)
{
	int i;
	string msg;
	string name;
	fprintf(stdout, "usage:  %s <options> <files>\n", program_name);

#ifdef TARG_STxP70
   fprintf(stdout, "\n");
#endif

	if (help_pattern != NULL)
	  fprintf(stdout, "available options that contain %s:\n", help_pattern);
	else
	  fprintf(stdout, "available options:\n");

	FOREACH_OPTION(i) {
		msg = get_option_help(i);
		if (msg != NULL) {
		    if (option_matches_language (i, invoked_lang)) {
			name = get_option_name(i);
			/* if pattern specified, only print if pattern found */
			if (help_pattern != NULL
				&& !contains_substring(name, help_pattern)
				&& !contains_substring(msg, help_pattern))
			{
				continue;	/* to next option */
			}
#if defined(TARG_ST)
#ifdef TARG_STxP70
			if ((option_was_seen(O_full_help) || !contains_substring(msg, "[*NASTxP70*]"))
             && !contains_substring(msg, "[*NA*]")) {
            if (strstr(msg,name)==msg) {
               fprintf(stdout,"%s\n",msg);
            } else {
               string msg_ptr;
               string msg_ptr_end;
               #define COLUMN_WIDTH 57
               #define OPTION_WIDTH 20
               
               if (strlen(name)>=OPTION_WIDTH) {
                  fprintf(stdout,"%s\n                    ",name);
               } else {
                  fprintf(stdout,"%s ",name);
                  for (i=strlen(name)+1;i<OPTION_WIDTH;i++) {
                     fprintf(stdout," ");
                  }
               }
               msg_ptr = msg;
               msg_ptr_end = msg + strlen(msg);
               while (msg_ptr < msg_ptr_end) {
                  if (strlen(msg_ptr)>COLUMN_WIDTH+1) {
                     char * ptr1;
                     char char_svg;
                     
                     char_svg = *(msg_ptr+COLUMN_WIDTH);
                     *(msg_ptr+COLUMN_WIDTH) = 0;
                     ptr1 = strrchr(msg_ptr,' ');
                     *(msg_ptr+COLUMN_WIDTH) = char_svg;
                     if (ptr1) {
                        *ptr1++ = 0;
                        fprintf(stdout,"%s\n                    ",msg_ptr);
                     } else {
                        ptr1 = msg_ptr+COLUMN_WIDTH;
                        char_svg = *ptr1;
                        *ptr1 = 0;
                        fprintf(stdout,"%s\n                    ",msg_ptr);
                        *ptr1 = char_svg;
                     }
                     msg_ptr = ptr1;
                  } else {
                     fprintf(stdout,"%s\n",msg_ptr);
                     msg_ptr = msg_ptr_end;
                  }
               }

               #undef COLUMN_WIDTH
               #undef OPTION_WIDTH
            }
         }
#else
			if (!contains_substring(msg, "[*NA*]")) 
			    fprintf(stdout, "\t%s:  %s\n", name, msg);
#endif
#else
			fprintf(stderr, "\t%s:  %s\n", name, msg);
#endif
		    }
		}
	}
#if !defined(TARG_ST) || !defined(RELEASE)
	if (help_pattern == NULL && invoked_lang == L_cc) {
	  fprintf(stderr, "The environment variable SGI_CC is also checked\n");
	}
#endif

	exit(RC_OKAY);
}

static void
dump_args (string msg, FILE *file)
{
	int i;
	fprintf(file, "dump args %s: ", msg);
	FOREACH_OPTION_SEEN(i) {
		if (i == O_Unrecognized) continue;
		/* there are some combos that result in a warning 
		 * and replacing one of the args; in that case the
		 * option name looks like "arg1 arg2" but the implied
		 * list is just "arg1". */
		if (is_replacement_combo(i)) {
			int iflag;
			FOREACH_IMPLIED_OPTION(iflag, i) {
			    fprintf(file, " %s", get_current_implied_name());
			}
		} else {
			fprintf(file, " %s", get_option_name(i));
		}
	}
	fprintf(file, "\n");
}


