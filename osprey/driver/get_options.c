
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

#include <string.h>
#include <ctype.h>
#include <malloc.h>
#include <stdlib.h>
#include <assert.h>

#ifndef sun
#include <getopt.h>
#endif

#include "string_utils.h"
#include "get_options.h"
#include "options.h"
#include "option_names.h"
#include "errors.h"
#include "file_utils.h"
#include "lang_defs.h"
#include "phases.h"
#include "opt_actions.h"
#include "option_seen.h" // TB: To add O_T option seen when -Wl,-T is invoked
// [HK] added missing prototype for getsubopt
extern int getsubopt(char **, char *const *, char **);

string option_name;

/*
 * In following code, we are using "arg" to refer to the
 * argv[argi] string, and "option" to refer to a user option.
 * "optarg" is used to refer to the argument of an option,
 * e.g. for "-Dfoo" foo is the argument to -D.
 */

string optargs = NULL;		/* argument to option, visible externally */
int optargd = 0;		/* argument to option, visible externally */
static int optindex = 1;	/* current index into option */

#ifdef TARG_ST
boolean linker_W_option_was_seen = FALSE;
#endif

/* are we at last character of arg string? */
#define is_last_char(argv,argi)	(argv[*argi][optindex+1] == '\0')

/* get new "-" arg */
#define get_next_arg(argi)	(*argi)++; optindex = 1;

/* are we at beginning of user arg? */
#define is_new_arg	(optindex == 1)

/* get next character in arg; 
 * for handling single-letter options that combine into one arg */
static void
get_next_char (char **argv, int *argi)
{
	if (is_last_char(argv,argi)) {
		get_next_arg(argi);
	} else {
		optindex++;
	}
}

/* move to end of option with given length */
static void
end_option (char **argv, int *argi, int length)
/*ARGSUSED*/
{
	optindex = length-1;	/* -1 cause we start at 0 */
}

#define current_string(argv,argi)	&argv[*argi][optindex]

/* get argument that follows option */
static string
get_optarg (char **argv, int *argi)
{
	if (is_last_char(argv,argi)) {
		(*argi)++;
		optindex = 0;
	} else {
		optindex++;
	}
	return current_string(argv,argi);
}

static string null_string = "";

/* get next string in option */
static string
next_string (char **argv, int *argi)
{
	if (is_last_char(argv,argi)) {
		if (argv[*argi+1] == NULL)
			return null_string;
		else
			return argv[*argi+1];
	} else {
		return &argv[*argi][optindex+1];
	}
}

/* get next string in option after prefix */
static string
next_string_after (string prefix, char **argv, int *argi)
{
	/* use new_optindex to temporarily move to end of prefix */
	size_t new_optindex = strlen(prefix);
	if (argv[*argi][new_optindex] == '\0') {
		if (argv[*argi+1] == NULL)
			return null_string;
		else
			return argv[*argi+1];
	} else {
		return &argv[*argi][new_optindex];
	}
}

static boolean
is_decimal (string s)
{
	if (isdigit(*s)) return TRUE;
	else return FALSE;
}

/* add arg to prefix for indirect option */
extern int 
add_string_option (int flag, string arg)
{
	/* check that there is an argument after the option,
	 * which should not begin with a dash (except a single dash)
	 * unless option ends in a comma (e.g. -pfa,%s ) */
	string s = get_option_name(flag);
	if (arg == NULL || (*arg == '-' && arg[1] != '\0' && s[strlen(s)-1] != ',')) {
		parse_error(get_option_name(flag), "no argument given for option");
		return flag;
	}
	return add_derived_option(flag, arg);
}

/* do initial pass through args to check for options that affect driver */
extern void
check_for_driver_controls (int argc, char *argv[])
{
        int i;
	string s;
	for (i = 1; i < argc; i++) {
                if (strncmp(argv[i], "-woff", 5) == 0) {
			s = next_string_after("-woff",argv,&i);
			if (same_string(s, "options")) {
                                print_warnings = FALSE;
			} else if (same_string(s, "all")) {
                                print_warnings = FALSE;
                        }
		} 
		else if (strcmp(argv[i], "-fullwarn") == 0) {
			fullwarn = TRUE;
		} 
		else if (strcmp(argv[i], "-v") == 0) {
			fullwarn = TRUE;
                } 
		else if (strcmp(argv[i], "-fbgen") == 0) {
			Gen_feedback = TRUE;
                } 
		else if (strcmp(argv[i], "-fbuse") == 0) {
			Use_feedback = TRUE;
                } 
		else if (strcmp(argv[i], "-E") == 0) {
			last_phase = P_any_cpp;
                } 
		else if (strcmp(argv[i], "-ignore_suffix") == 0) {
			ignore_suffix = TRUE;
                } 
		else if (strcmp(argv[i], "-i32") == 0) {
			abi = ABI_I32;
                }
		else if (strcmp(argv[i], "-i64") == 0) {
			abi = ABI_I64;
                }
		else if (strcmp(argv[i], "-ia32") == 0) {
			abi = ABI_IA32;
                }
		else if (strcmp(argv[i], "-st100") == 0) {
			abi = ABI_ST100;
                }
		/* clarkes: Do we want the -st200 option to force the
		   embedded ABI?
		*/
		else if (strcmp(argv[i], "-st200") == 0) {
			abi = ABI_ST200_embedded;
                }
		else if (strcmp(argv[i], "-stxp70") == 0) {
			abi = ABI_STxP70_embedded;
                }
		else if (strcmp(argv[i], "-arm") == 0) {
			abi = ABI_ARM_ver1;
                }
        }
}

/* for -u option: 
 * UGLY KLUDGE:  f77 has -u option with no args, ld has -u option with one arg.
 * Ucode compiler handled this by having -u when invoked by non-fortran be
 * ld option, and Sun seems to also do this.  So we have to do it too, even
 * though it breaks all our assumptions about options having only one meaning.*/
static int
parse_u_option (char **argv, int *argi)
{
	if (invoked_lang == L_f77 || invoked_lang == L_f90) {
		int flag;
       		get_next_arg(argi);
		flag = add_new_option("-u");
		add_phase_for_option(flag, P_f_fe);
		return flag;
	} else {
                optargs = get_optarg(argv, argi);
                get_next_arg(argi);
                return add_string_option(O_u,optargs);
	}
}

/* for -U option:
 * UGLY KLUDGE:  f77 has -U with no args and -U<arg>.
 * In C we allow -U <arg> but for f77 we will return -U if a space after the U.
 */
static int
parse_U_option (char **argv, int *argi)
{
	if ((invoked_lang == L_f77 || invoked_lang == L_f90)
		&& is_last_char(argv,argi)) 
	{
		/* -U */
		int flag;
       		get_next_arg(argi);
		flag = add_new_option("-U");
		add_phase_for_option(flag, P_f_fe);
		return flag;
	} else {
		/* -U<arg> */
                optargs = get_optarg(argv, argi);
                get_next_arg(argi);
                return add_string_option(O_U,optargs);
	}
}

/* for -C option:
 * UGLY KLUDGE:  f77 has -C means "runtime subscript checking"
 * In C we allow -C to mean "keep C comments after cpp"
 */
static int
parse_C_option (char **argv, int *argi)
{
   if (is_last_char(argv,argi)) {
      if (invoked_lang == L_f77 || invoked_lang == L_f90)
	{
	   /* -C : for fortran 77/90 */
	   int flag;
	   get_next_arg(argi);
	   flag = add_new_option("-DEBUG:subscript_check");
	   add_phase_for_option(flag, P_f_fe);
	   add_phase_for_option(flag, P_f90_fe);
	   return flag;
	} else {
	   /* -C : for C/C++ */
	   int flag;
	   get_next_arg(argi);
	   flag = add_new_option("-C");
	   add_phase_for_option(flag, P_any_cpp);
	   return flag;
	}
   } else {
      get_next_arg(argi);
      return O_Unrecognized;
   }
}

/* need to hand code -R option since the generic code doeesnt like
   the arg string to begin with - */
static int
parse_R_option (char **argv, int *argi)
{
	if ((invoked_lang == L_f77 || invoked_lang == L_f90)
		&& !is_last_char(argv,argi)) 
	{
		/* -R */
		int flag;
		optargs = get_optarg(argv, argi);
       		get_next_arg(argi);
		flag = add_new_option(optargs);
		add_phase_for_option(flag, P_ratfor);
		return flag;
	} else {
		parse_error(option_name, "no argument given for option");
       		get_next_arg(argi);
		return O_Unrecognized;
	}
}

static boolean middle_of_multi_option = FALSE;

/* common routine to end -W by adjusting to correct place */
static void
end_multi_option (char **argv, int *argi, string p)
{
	if (*p == NIL) {
		middle_of_multi_option = FALSE;
        	(void) get_optarg(argv, argi);
        	get_next_arg(argi);
	} else {
		middle_of_multi_option = TRUE;
		optargs = p+1;
	}
}

static int
parse_multi_option (char **argv, int *argi)
{
	int flag;
	string p, q;
	buffer_t buf;
	if (! middle_of_multi_option) {
        	optargs = next_string(argv,argi);
		for (p = optargs; *p != NIL && *p != ','; p++)
			;
		if (*p == NIL) {
			parse_error(option_name, "bad syntax for option");
			end_multi_option(argv, argi, p);
			return O_Unrecognized;
		}
		p++;	/* skip past comma */
	} else {
		p = optargs;
	}
	if (*p == NIL) {
		/* no args */
		parse_error(option_name, "bad syntax for option");
		end_multi_option(argv, argi, p);
		return O_Unrecognized;
	}
	/* args separated by comma are separate args, unless \, */
	q = buf;
	while (*p != NIL && *p != ',') {
		if (*p == '\\' && *(p+1) == ',') {
			*q = ',';
			p++;
		} else {
			*q = *p;
		}
		p++;
		q++;
	}
	*q = NIL;
	flag = add_new_option(buf);
	end_multi_option(argv, argi, p);
	return flag;
}


/* for -W option: */
/* add option for particular phase(s), e.g. -Wb,-foo becomes -foo option */
static int
parse_W_option (char **argv, int *argi)
{
	int flag;
	phases_t phase;
	string start = next_string(argv,argi);
	if ( ! contains_substring(start, ",") || *start == '-') {
		get_next_char(argv,argi);
		return O_W;
	}
	flag = parse_multi_option (argv, argi);
	/* set phases for whatever user said */
	for (; *start != NIL && *start != ','; start++) {
	  phase = get_phase(*start);
	  if (phase == P_NONE) {
	    parse_error(option_name, "bad phase for -W option");
	  }
#ifdef TARG_ST
	  /* [TB] Add parsing of -Wy,XXX and Wz,XXX option to
	     toggle shared and dynamic */
	  if (phase == P_any_ld && 
	      (*start == phasekey_for_ld || *start == phasekey_for_ld_r || *start == phasekey_for_simple_ld)) {
	    /* TB case of -Wy,XXX option : add XXX to objects_ld_r */
	    /* TB case of -Wz,XXX option : add XXX to objects_ld_final */
	    string_list_t *objects_to_change = (*start == phasekey_for_ld_r) ? 
	      objects_ld_r : (*start == phasekey_for_ld) ? objects : objects_ld_final;
	    /* [CG]: For -Wl options we pass directly the flags to
	       the objects list. Indeed the options must be passed in
	       order (with other standard objects) to ld. */
	    /* TB case of -Wl,XXX option : add XXX to objects */
	    static boolean previous_was_map = FALSE;
	    static boolean previous_was_T = FALSE;
	    string optionname = get_option_name(flag);
	    linker_W_option_was_seen = TRUE;
	    /* TB: Parse different -Wl,* options*/
	    if ( (strcmp(optionname,"-shared") == 0) ||
		 (strcmp(optionname,"--shared") == 0))
	      {
		toggle(&shared,DSO_SHARED);
		toggle(&dynamic,TRUE);
	      } else if ((strcmp(optionname,"-dy") == 0) ||
			 (strcmp(optionname,"-Bdynamic") == 0) ||
			 (strcmp(optionname,"-call_shared") == 0) ||
			 (strcmp(optionname,"-export-dynamic") == 0) ||
			 (strcmp(optionname,"-E") == 0) ||
			 (strncmp(optionname,"-dynamic-linker", 15) == 0) || 
			 (strcmp(optionname,"--dy") == 0) ||
			 (strcmp(optionname,"--Bdynamic") == 0) ||
			 (strcmp(optionname,"--call_shared") == 0) ||
			 (strcmp(optionname,"--export-dynamic") == 0) ||
			 (strncmp(optionname,"--dynamic-linker", 16) == 0) ) {
		
		toggle(&dynamic,TRUE);
	      } else if ( (strcmp(optionname,"-r") == 0) ||
			  (strcmp(optionname,"--relocateable") == 0) ||
			  (strcmp(optionname,"-relocateable") == 0) ) {
		toggle(&shared,RELOCATABLE);
		last_phase=P_any_ld;
		toggle(&dynamic,FALSE);
	      } else if ((strcmp(optionname,"--relax") == 0) ||
			 (strcmp(optionname,"-relax") == 0) ) {
		toggle(&relax,TRUE);
		// TB: add O_T option seen when -Wl,-T is invoked
	      }
	    if (*start == phasekey_for_ld) {
	      if (strcmp(optionname,"-T") == 0) {
		add_option_seen(O_T);
		/* Next arg is linker script file */
		previous_was_T = TRUE;
	      } else if (strncmp(optionname,"-T",2) == 0) {
		/* -Tscript */
		add_option_seen(O_T);
		optionname +=2;
		add_script_file(optionname);
	      }  else if (previous_was_T == TRUE) {
		add_script_file(optionname);
		previous_was_T = FALSE;
	      }
#ifdef BCO_ENABLED
	      /* [TB] Disable -Map / -M for all but the final linkphase. */
	      else if (strcmp(optionname,"-M") == 0) {
		flag_M=flag;
	      } else if ((strcmp(optionname,"--Map") == 0) ||
			 (strcmp(optionname,"-Map") == 0) ) {
		/* Next arg is mapfile */
		previous_was_map = TRUE;
		/* [TB] Disable -Map for all but the final linkphase. */
		flag_MAP=flag;
	      } else if (previous_was_map == TRUE) {
		flag_MAPFILE=flag;
		previous_was_map = FALSE;
	      }
#endif /* BCO_ENABLED */
	      else {
		add_string(objects_to_change,optionname);
	      }
	    } else 
	      add_string(objects_to_change,optionname);
	  }/* (*start == phasekey_for_ld || *start == phasekey_for_ld_r || *start == phasekey_for_simple_ld)*/ 
	  else /* Other -W?,xxxx option*/
	    add_phase_for_option(flag, phase);
#else
	  add_phase_for_option(flag, phase);
#endif
	}/*(; *start != NIL && *start != ','; start++ */
	
	/* warn about -WK being obsolete */
	if (auto_parallelize && phase == P_any_optfe) {
		warning("-WK,<options> is ignored with new -pfa/-pca");
	}

	/* Force treatment as internal so that -Wb, gets prefixed
	 * to IPL -cmds list entry (PV 375260):
	 */
	set_internal_option ( flag );

	return flag;
}

/* for -K option: */
static string optargp = NULL;	/* keep track of current -K arg */
static char *Kopts[] = {
#define K_PIC 0
	"PIC",
#define K_minabi 1
	"minabi",
#define K_sd 2
	"sd",
#define K_sz 3
	"sz",
#define K_fpe 4
	"fpe",
#define K_mau 5
	"mau",
	NULL};

static int
parse_K_option (char **argv, int *argi)
{
	int flag;
	string value;
	if (optargp == NULL) {
		optargp = string_copy(next_string(argv,argi));
	}
	switch (getsubopt(&optargp, Kopts, &value)) {
	case K_PIC:
		flag = O_KPIC; break;
	case K_minabi:
		flag = O_Kminabi; break;
	case K_sd:
		flag = O_Ksd; break;
	case K_sz:
		flag = O_Ksz; break;
	case K_fpe:
		flag = O_Kfpe; break;
	case K_mau:
		flag = O_Kmau; break;
	default:
		flag = O_K; break;
	};
	if (flag == O_K) {
		/* no more options */ 
		optargp = NULL;
		if (!is_last_char(argv,argi)) {
			/* Could be something like -Kv,
		 	 * but we won't handle this as multi-opt argument,
		 	 * and that is okay because no single -K in svr4 */
			optargs = current_string(argv,argi);
			flag = O_Unrecognized;
		}
		get_next_arg(argi);
	} else if (*optargp == '\0') {
		/* done with -K<values>; no more options */
		optargp = NULL;
		(void) get_optarg (argv,argi);	/* values arg */
		get_next_arg(argi);
	}
	return flag;
}

#ifdef TARG_ST200
int packing_level = -1;
static int
parse_mpacking_option (char **argv, int *argi)
{
  string ptr_subopt;
  string ptr_nextsubopt;
  int    res = O_mpacking__;
  
  /* Go to first sub-option i.e. after -mpacking */
  ptr_subopt = string_copy(next_string(argv,argi));
  ptr_subopt = strchr(ptr_subopt,'=');
  if (ptr_subopt == NULL) {
    // only -mpacking. default is 2
    packing_level = 2;
  }
  else {
    char c;
    packing_level = 0;
    res = O_Unrecognized;
    for (c = *++ptr_subopt; (c >= '0') && (c <= '9'); c = *++ptr_subopt) {
      packing_level = (packing_level * 10) + (c - '0');
      res = O_mpacking__;
    }
    if ((c != '\0') || (packing_level > 2))
      res = O_Unrecognized;
  }

  get_next_arg(argi);
  return res;
}
#endif

#ifdef TARG_STxP70
#include "stxp70_options.i"
#endif

#include "get_option.i"
