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
#include <stdlib.h>
#include <libiberty.h>
#include <hashtab.h>
#include "SYS.h"
#include "opt_actions.h"
#include "options.h"
#include "option_names.h"
#include "option_seen.h"
#include "lang_defs.h"
#include "errors.h"
#include "file_utils.h"
#include "file_names.h"
#include "string_utils.h"
#include "get_options.h"
#include "objects.h"
#include "phases.h"
#include "run.h"

#include "W_alloca.h"

/* keep list of previous toggled option names, to give better messages */
typedef struct toggle_name_struct {
	int *address;
	string name;
} toggle_name;
#define MAX_TOGGLES	50
static toggle_name toggled_names[MAX_TOGGLES];
static int last_toggle_index = 0;
static int inline_on_seen = FALSE;
int inline_t = UNDEFINED;
boolean dashdash_flag = FALSE;
boolean read_stdin = FALSE;
boolean xpg_flag = FALSE;
int default_olevel = UNDEFINED;
static int default_isa = UNDEFINED;
static int default_proc = UNDEFINED;
int instrumentation_invoked = UNDEFINED;
boolean ftz_crt = FALSE;

#ifdef TARG_ST
 extern int ExtensionSeen;
int c_std;
/* TB: to warm if olevel is not enough if uninitialized var warning is
   aked*/
boolean Wuninitialized_is_asked = FALSE; 
#endif

#ifdef TARG_ST
char *print_name;
int print_kind;
#endif

#ifdef TARG_STxP70
boolean farcall = FALSE;
#endif


/* ====================================================================
 *
 * -Ofast targets
 *
 * Given an -Ofast option, tables which map the IP numbers to
 * processors for use in Ofast_Target below.
 *
 * See common/com/MIPS/config_platform.h.
 *
 * PV 378171:  Change this and config.c to use an external table.
 *
 * ====================================================================
 */

/* Mapping from processors to best ISA: */
static struct {
  PROCESSOR p;
  ISA	isa;
  int	opt;
} P_to_I_Map[] =
{
#ifdef TARG_MIPS
  { PROC_R4K,	ISA_MIPS3,  O_mips3 },
  { PROC_R5K,	ISA_MIPS4,  O_mips4 },
  { PROC_R8K,	ISA_MIPS4,  O_mips4 },
  { PROC_R10K,	ISA_MIPS4,  O_mips4 },
#endif
  { PROC_ITANIUM,	ISA_IA641,  O_Unrecognized },
  { PROC_ST100, ISA_ST100, O_Unrecognized },
  { PROC_ST210, ISA_ST210, O_Unrecognized },
  { PROC_ST220, ISA_ST220, O_Unrecognized },
  { PROC_ST221, ISA_ST220, O_Unrecognized },
  { PROC_ST231, ISA_ST220, O_Unrecognized },
  { PROC_armv5, ISA_armv5, O_Unrecognized },
  { PROC_armv6, ISA_armv6, O_Unrecognized },
  { PROC_NONE,	ISA_NONE,  O_Unrecognized }
};

static struct {
  string pname;
  PROCESSOR pid;
} Proc_Map[] =
{
  { "r4000",	PROC_R4K },
  { "r4k",	PROC_R4K },
  { "r5000",	PROC_R5K },
  { "r5k",	PROC_R5K },
  { "r8000",	PROC_R8K },
  { "r8k",	PROC_R8K },
  { "r10000",	PROC_R10K },
  { "r10k",	PROC_R10K },
  { "r12000",	PROC_R10K },
  { "r12k",	PROC_R10K },
  { "r14000",	PROC_R10K },
  { "r14k",	PROC_R10K },
  { "r16000",	PROC_R10K },
  { "r16k",	PROC_R10K },
  { "itanium",	PROC_ITANIUM },
  { "st100",    PROC_ST100 },
  { "st210",    PROC_ST210 },
  { "st220",    PROC_ST220 },
  { "st221",    PROC_ST221 },
  { "st231",    PROC_ST231 },
  { "st240",    PROC_ST240 },
  { "arm9",     PROC_armv5 },
  { "arm11",    PROC_armv6 },
  { "stxp70_v3",PROC_stxp70_v3 },
  { "stxp70_v4",PROC_stxp70_v4_single},
  { "stxp70v3", PROC_stxp70_v3 },
  { "stxp70v4", PROC_stxp70_v4_single },
  { "stxp70v4single", PROC_stxp70_v4_single },
  { "stxp70v4dual",PROC_stxp70_v4_dual },
  { "stxp70v4dualarith",PROC_stxp70_v4_dual_arith},
  { NULL,	PROC_NONE }
};

#ifdef MUMBLE_ST200_BSP
static struct {
  string pname;
  RUNTIME pid;
} Runtime_Map[] =
{
  { "bare",	RUNTIME_BARE },
  { "os21",	RUNTIME_OS21 },
  { "os21_d",	RUNTIME_OS21_DEBUG },
  { NULL,	RUNTIME_NONE }
};
static struct {
  string pname;
  SYSCALL pid;
} Syscall_Map[] =
{
  { "libgloss",	SYSCALL_LIBGLOSS },
  { "libdtf",	SYSCALL_LIBDTF },
  { NULL,      	SYSCALL_NONE }
};
#endif

#ifdef MUMBLE_STxP70_BSP
static struct {
  string pname;
  RUNTIME pid;
} Runtime_Map[] =
{
  { NULL,	RUNTIME_NONE }
};
#endif


#ifdef MUMBLE_ARM_BSP
static struct {
  string pname;
  RUNTIME pid;
} Runtime_Map[] =
{
  { NULL,	RUNTIME_NONE }
};
#endif


int ofast = UNDEFINED;	/* -Ofast toggle -- implicit in Process_Ofast */
char *Ofast_Name = NULL;/* -Ofast= name */


static void
add_toggle_name (int *obj, string name)
{
	int i;
	for (i = 0; i < last_toggle_index; i++) {
		if (obj == toggled_names[i].address) {
			break;
		}
	}
	if (i == last_toggle_index) {
		if (last_toggle_index >= MAX_TOGGLES) {
			internal_error("too many toggle names\n");
		} else {
			last_toggle_index++;
		}
	}
	toggled_names[i].address = obj;
	toggled_names[i].name = string_copy(option_name);
}

static string
get_toggle_name (int *obj)
{
	int i;
	for (i = 0; i < last_toggle_index; i++) {
		if (obj == toggled_names[i].address) {
			return toggled_names[i].name;
		}
	}
	internal_error("no previously toggled name?");
	return "<unknown>";
}

/* return whether has been toggled yet */
extern boolean
is_toggled (int obj)
{
	return (obj != UNDEFINED);
}

/* set obj to value; allow many toggles; last toggle is final value */
extern void
toggle (int *obj, int value)
{
	if (*obj != UNDEFINED && *obj != value) {
		warning ("%s conflicts with %s; using latter value (%s)", 
			get_toggle_name(obj), option_name, option_name);
	}
	*obj = value;
	add_toggle_name(obj, option_name);
}

/* ====================================================================
 *
 * Get_Group_Option_Value
 *
 * Given a group option string, search for the option with the given
 * name.  Return NULL if not found, the option value if found ("" if
 * value is empty).
 *
 * ====================================================================
 */

static char *
Get_Group_Option_Value (
  char *arg,	/* Raw option string */
  char *name,	/* Suboption full name */
  char *abbrev)	/* Suboption abbreviation */
{
  char *endc = arg;
  int n;

  while ( TRUE ) {
    n = strcspn ( arg, ":=" );
    if ( strncasecmp ( arg, abbrev, strlen(abbrev) ) == 0
      && strncasecmp ( arg, name, n ) == 0 )
    {
      endc += n;
      if ( *endc == '=' ) {
	/* Duplicate value lazily: */
	char *result = strdup ( endc+1 );

	* ( result + strcspn ( result, ":=" ) ) = 0;
	return result;
      } else {
	/* No value: */
	return "";
      }
    }
    if ( ( endc = strchr ( arg, ':' ) ) == NULL ) return NULL;
    arg = ++endc;
  }

  /* Shouldn't get here, but ... */
  /* return NULL;  compiler gets better */
}

/* ====================================================================
 *
 * Bool_Group_Value
 *
 * Given a group option value string for a Boolean group value,
 * determine whether it is TRUE or FALSE.
 *
 * ====================================================================
 */

static boolean
Bool_Group_Value ( char *val )
{
  if ( *val == 0 ) {
    /* Empty string is TRUE for group options */
    return TRUE;
  }

  if ( strcasecmp ( val, "OFF" ) == 0
    || strcasecmp ( val, "NO" ) == 0
    || strcasecmp ( val, "FALSE" ) == 0
    || strcasecmp ( val, "0" ) == 0 )
  {
    return FALSE;
  } else {
    return TRUE;
  }
}

/* ====================================================================
 *
 * Routine to manage the implications of -Ofast.
 *
 * Turn on -O3 and -IPA.  Check_Target below will deal with the ABI and
 * ISA implications later.
 *
 * ====================================================================
 */

void
Process_Ofast ( char *ipname )
{
  int flag;
  char *suboption;

  /* Remember the name for later defaulting of ISA/processor: */
  Ofast_Name = string_copy (ipname);

  /* -O3: */
  if (!Gen_feedback) {
     O3_flag = TRUE;
     toggle ( &olevel, 3 );
     add_option_seen ( O_O3 );

#ifdef TARG_IA64
     ftz_crt = TRUE;	// flush to zero
#endif

     /* -IPA: */
     toggle ( &ipa, TRUE );
     add_option_seen ( O_IPA );

     /* -OPT:Ofast=ipname
      * We will call add_string_option using O_OPT_; if the descriptor
      * for it in OPTIONS changes, this code might require change...
      * Build the "Ofast=ipname" string, then call add_string_option:
      */
     toggle ( &ofast, TRUE );
     suboption = concat_strings ( "Ofast=", ipname );
     flag = add_string_option ( O_OPT_, suboption );
     add_option_seen ( flag );
   } else {
     suboption = concat_strings ( "platform=", ipname );
     flag = add_string_option ( O_TARG_, suboption );
     add_option_seen ( flag );
   }
}

/* ====================================================================
 *
 * Process_Opt_Group
 *
 * We've found a -OPT option group.  Inspect it for -OPT:reorg_common
 * options, and set -split_common and -ivpad accordingly.
 *
 * NOTE: We ignore anything that doesn't match what's expected --
 * the compiler will produce reasonable error messages for junk.
 *
 * ====================================================================
 */

void
Process_Opt_Group ( string opt_args )
{
  char *optval = NULL;

  if ( debug ) {
    fprintf ( stderr, "Process_Opt_Group: %s\n", opt_args );
  }

#ifdef TARG_ST
  if (strncmp("enable_instrument", opt_args, strlen("enable_instrument")) == 0)
     instrumentation_invoked = TRUE;
#endif  
  /* Go look for -OPT:instrument */
  optval = Get_Group_Option_Value ( opt_args, "instrumentation", "instr");
  if (optval != NULL) {
     instrumentation_invoked = TRUE;
  }

  /* Go look for -OPT:reorg_common: */
  optval = Get_Group_Option_Value ( opt_args, "reorg_common", "reorg");
  if ( optval != NULL && Bool_Group_Value(optval)) {
    /* If we found it, set -Wl,-split_common,-ivpad: */
    add_option_seen ( O_split_common );
    add_option_seen ( O_ivpad );
  }
}

void
Process_Default_Group (string default_args)
{
  string s;
  int i;

  if ( debug ) {
    fprintf ( stderr, "Process_Default_Group: %s\n", default_args );
  }

  /* Go look for -DEFAULT:isa=mipsN: */
  s = Get_Group_Option_Value ( default_args, "isa", "isa");
  if (s != NULL && same_string_prefix (s, "mips")) {
	default_isa = atoi(s + strlen("mips"));
  }
  /* Go look for -DEFAULT:proc=rN000: */
  s = Get_Group_Option_Value ( default_args, "proc", "proc");
  if (s != NULL) {
	for (i = 0; Proc_Map[i].pname != NULL; i++) {
		if (same_string(s, Proc_Map[i].pname)) {
			default_proc = Proc_Map[i].pid;
		}
	}
  }
  /* Go look for -DEFAULT:opt=[0-3]: */
  s = Get_Group_Option_Value ( default_args, "opt", "opt");
  if (s != NULL) {
	default_olevel = atoi(s);
  }
  /* Go look for -DEFAULT:arith=[0-3]: */
  s = Get_Group_Option_Value ( default_args, "arith", "arith");
  if (s != NULL) {
	i = add_string_option (O_OPT_, concat_strings("IEEE_arith=", s));
	add_option_seen (i);
  }
}

/* ====================================================================
 *
 * Routines to manage the target selection (ABI, ISA, and processor).
 *
 * Make sure that the driver picks up a consistent view of the target
 * selected, based either on user options or on defaults.
 *
 * ====================================================================
 */

/* ====================================================================
 *
 * Process_Targ_Group
 *
 * We've found a -TARG option group.  Inspect it for ABI, ISA, and/or
 * processor specification, and toggle the state appropriately.
 *
 * NOTE: We ignore anything that doesn't match what's expected --
 * the compiler will produce reasonable error messages for junk.
 *
 * ====================================================================
 */

void
Process_Targ_Group ( string targ_args )
{
  char *cp = targ_args;	/* Skip -TARG: */
  char *cpeq;
  string ftz;

  if ( debug ) {
    fprintf ( stderr, "Process_Targ_Group: %s\n", targ_args );
  }

  ftz = Get_Group_Option_Value ( targ_args, "flush_to_zero", "flush_to_zero");
  if ( ftz != NULL && Bool_Group_Value(ftz)) {
    /* link in ftz.o */
    ftz_crt = TRUE;
  }

  while ( *cp != 0 ) {
    switch ( *cp ) {
      case 'a':
	if ( strncasecmp ( cp, "abi", 3 ) == 0 && *(cp+3) == '=' ) {
#ifdef TARG_MIPS
	  if ( strncasecmp ( cp+4, "n32", 3 ) == 0 ) {
	    add_option_seen ( O_n32 );
	    toggle ( &abi, ABI_N32 );
	  } else if ( strncasecmp ( cp+4, "64", 2 ) == 0 ) {
	    add_option_seen ( O_64 );
	    toggle ( &abi, ABI_64 );
	  }
#endif
	}
	break;

      case 'e':
	if ( strncasecmp ( cp, "exc_enable", 10 ) == 0 && *(cp+10) == '=' ) {
  	  int flag;
  	  buffer_t buf;
	  int mask = 0;
	  cp += 11;
    	  while ( *cp != 0 && *cp != ':' ) {
	    switch (*cp) {
	    case 'I': mask |= (1 << 5); break;
	    case 'U': mask |= (1 << 4); break;
	    case 'O': mask |= (1 << 3); break;
	    case 'Z': mask |= (1 << 2); break;
	    case 'D': mask |= (1 << 1); break;
	    case 'V': mask |= (1 << 0); break;
	    }
	    ++cp;
	  }
#ifndef TARG_ST
	  flag = add_string_option(O_WlC, "-defsym,_IEEE_ENABLE_DEFINED=1");
	  add_option_seen (flag);
	  sprintf(buf, "-defsym,_IEEE_ENABLE=%#x", mask);
	  flag = add_string_option(O_WlC, buf);
	  add_option_seen (flag);
#else
	  /* The following code is inspired from parse_W_option
	     in get_options.c
	     It is here to keep compatibility */
	  flag = add_new_option("-defsym _IEEE_ENABLE_DEFINED=1");
	  add_phase_for_option(flag, get_phase('l'));
	  set_internal_option ( flag );
	  add_option_seen (flag);
	  sprintf(buf, "-defsym _IEEE_ENABLE=%#x", mask);
	  flag = add_new_option(buf);
	  add_phase_for_option(flag, get_phase('l'));
	  set_internal_option ( flag );
	  add_option_seen (flag);
#endif
	}
	break;

      case 'i':
	/* We support both isa=mipsn and plain mipsn in group.
	 * Simply move cp to point to value, and fall through to
	 * 'm' case:
	 */
	if ( strncasecmp ( cp, "isa", 3 ) != 0 || *(cp+3) != '=' ) {
	  break;
	} else {
	  cp += 4;
	}
	/* Fall through */

      case 'm':
#ifdef TARG_MIPS
	if ( strncasecmp ( cp, "mips", 4 ) == 0 ) {
	  if ( '1' <= *(cp+4) && *(cp+4) <= '6' ) {
	    toggle ( &isa, *(cp+4) - '0' );
	    switch ( isa ) {
	      case 1:	add_option_seen ( O_mips1 );
			break;
	      case 2:	add_option_seen ( O_mips2 );
			break;
	      case 3:	add_option_seen ( O_mips3 );
			break;
	      case 4:	add_option_seen ( O_mips4 );
			break;
	      default:	error ( "invalid ISA: %s", cp );
			break;
	    }
	  }
	}
#endif
	break;


      case 'p':
	/* Allow abbreviation of "processor" to "pr" or longer: */
	cpeq = strchr ( cp, '=' );
	if ( cpeq != NULL
	  && strncasecmp ( cp, "processor", cpeq-cp ) == 0 )
	{
	  /* We don't actually add options here, because they don't
	   * have implications (e.g. associated -D options), and the
	   * phases will do just fine based on the -TARG: option:
	   */
	  int i;
	  cp = cpeq+1;
	  for (i = 0; Proc_Map[i].pname != NULL; i++) {
		if (same_string(cp, Proc_Map[i].pname)) {
			toggle (&proc, Proc_Map[i].pid);
		}
	  }
	}
	break;
    }

    /* Skip to the next group option: */
    while ( *cp != 0 && *cp != ':' ) ++cp;
    if ( *cp == ':' ) ++cp;
  }
}

/* ====================================================================
 *
 * Ofast_Target
 *
 * There was a -Ofast option, which affects the target defaults.
 * The ABI always defaults to -n32 (today).  The processor then
 * defaults to that used in the platform indicated by -Ofast, or the
 * r10000 (today).  Finally, the ISA is defaulted to the highest
 * supported by the platform, usually -mips4 today.
 *
 * ====================================================================
 */

static void
Ofast_Target ( void )
{
  int ix;
  PLATFORM_OPTIONS *popts;

  /* Driverwrap should always insert an ABI, but just in case: */
  if ( abi == UNDEFINED ) {
    add_option_seen ( O_i64 );
    option_name = get_option_name ( O_Ofast );
    toggle ( &abi, ABI_I64 );
  }

  /* Now fetch the IP descriptor by name: */
  popts = Get_Platform_Options ( Ofast_Name );

  /* Get the processor -- we won't bother to toggle it since the
   * compiler will figure it out the same way and only this routine
   * in the driver needs it:
   */
  if ( proc == UNDEFINED ) {
    if ( Ofast_Name != NULL
      && *Ofast_Name != 0
      && popts->id == IP0 )
    {
      warning ( "Unrecognized -Ofast value '%s': defaulting to '%s' (%s)", 
		Ofast_Name, popts->name, popts->nickname );
    }
    proc = popts->processor;
  }

  /* Finally, get the ISA (the purpose of all this): */
  if ( isa == UNDEFINED ) {
    for ( ix = 0;
	  P_to_I_Map[ix].p != proc && P_to_I_Map[ix].p != PROC_NONE;
	  ++ix )
    { }
    add_option_seen ( P_to_I_Map[ix].opt );
    option_name = get_option_name ( P_to_I_Map[ix].opt );
    toggle ( &isa, P_to_I_Map[ix].isa );
  }

  if ( debug ) {
    fprintf ( stderr,
	      "Ofast_Target -Ofast=%s: '%s' (%s) r%dk mips%d\n",
	      Ofast_Name, popts->name, popts->nickname, proc, isa );
  }
}

#ifdef TARG_ST200
/* ====================================================================
 *
 * add_st200_phase_for_option
 *
 *   Add flag to all needed phase for st200 target option
 *
 * ====================================================================
 */
static void
add_st200_phase_for_option( int flag )
{
  add_phase_for_option(flag, P_be);
  add_phase_for_option(flag, P_any_ipl);
  add_phase_for_option(flag, P_any_fe);
#if (GNU_FRONT_END==33)
  /* (cbr) -TARG now passed to cpp */
  add_phase_for_option(flag, P_gcpp);
  add_phase_for_option(flag, P_gcpp_plus);
#endif
  if (!already_provided(flag)) {
    /* [CL] Only prepend this option if
       not already provided by the user */
    prepend_option_seen (flag);
  }
}
#endif /* TARG_ST200 */

#ifdef TARG_STxP70
/* ====================================================================
 *
 * add_stxp70_phase_for_option
 *
 *   Add flag to all needed phase for stxp70 target option
 *
 * ====================================================================
 */
static void
add_stxp70_phase_for_option( int flag )
{
  add_phase_for_option(flag, P_be);
  add_phase_for_option(flag, P_any_ipl);
  add_phase_for_option(flag, P_any_fe);
#if (GNU_FRONT_END==33)
  /* (cbr) -TARG now passed to cpp */
  add_phase_for_option(flag, P_gcpp);
  add_phase_for_option(flag, P_gcpp_plus);
#endif
  if (!already_provided(flag)) {
    /* [CL] Only prepend this option if
       not already provided by the user */
    prepend_option_seen (flag);
  }
}


static void
add_stxp70_int_option(char* option_name, int imm, phases_t phase)
{
  char *str = alloca(strlen(option_name)+16);
  int flag;
  sprintf(str, option_name, imm);
  flag = add_new_option(str);
  add_phase_for_option(flag, phase);
  if (!already_provided(flag)) {
    prepend_option_seen (flag);
  }
}

static void
check_range(char* m1, char* m2, int min1, int max1, int min2, int max2)
{
  if ((max1>0) && (max2>0) &&
       ((min1<=min2 && min2<=max1) ||
        (min2<=min1 && min1<=max2)))
   warning("Conflict between size ranges %s [%d:%d] and "
           "%s [%d:%d]\n",
           m1, min1, max1, m2, min2, max2);
}

#endif /* TARG_STxP70 */

#ifdef TARG_ARM
/* ====================================================================
 *
 * add_arm_phase_for_option
 *
 *   Add flag to all needed phase for ARM target option
 *
 * ====================================================================
 */
static void
add_arm_phase_for_option( int flag )
{
  add_phase_for_option(flag, P_be);
  add_phase_for_option(flag, P_any_ipl);
  add_phase_for_option(flag, P_any_fe);
#if (GNU_FRONT_END==33)
  /* (cbr) -TARG now passed to cpp */
  add_phase_for_option(flag, P_gcpp);
  add_phase_for_option(flag, P_gcpp_plus);
#endif
  if (!already_provided(flag)) {
    /* [CL] Only prepend this option if
       not already provided by the user */
    prepend_option_seen (flag);
  }
}


static void
add_arm_int_option(char* option_name, int imm, phases_t phase)
{
  char *str = alloca(strlen(option_name)+16);
  int flag;
  sprintf(str, option_name, imm);
  flag = add_new_option(str);
  add_phase_for_option(flag, phase);
  if (!already_provided(flag)) {
    prepend_option_seen (flag);
  }
}

static void
check_range(char* m1, char* m2, int min1, int max1, int min2, int max2)
{
  if ((max1>0) && (max2>0) &&
       ((min1<=min2 && min2<=max1) ||
        (min2<=min1 && min1<=max2)))
   warning("Conflict between size ranges %s [%d:%d] and "
           "%s [%d:%d]\n",
           m1, min1, max1, m2, min2, max2);
}

#endif /* TARG_ARM */

/* ====================================================================
 *
 * Check_Target
 *
 * Verify that the target selection is consistent and set defaults.
 *
 * ====================================================================
 */
void
Check_Target ( void )
{
  int opt_id;
  int opt_val;
  int flag;
  string old_dir;
  string new_P_library_dir;
  string new_P_startup_dir;
  char * ptr;

  if ( debug ) {
    fprintf ( stderr, "Check_Target ABI=%d ISA=%d Processor=%d\n",
	      abi, isa, proc );
  }

  /* If -Ofast is given, default to -n32, specified platform's
   * processor, and best ISA:
   */
  if ( (ofast == TRUE) || (Gen_feedback == TRUE)) {
    Ofast_Target ();
  }

#ifdef TARG_ST200
  switch (proc) {
  case UNDEFINED:
    toggle(&proc, PROC_ST220);
    /* fallthru ST220 default. */
  case PROC_ST220:
    flag = add_new_option("-TARG:proc=st220");
    break;
  case PROC_ST221:
    flag = add_new_option("-TARG:proc=st221");
    break;
  case PROC_ST231:
    flag = add_new_option("-TARG:proc=st231");
    break;
  case PROC_ST240:
    flag = add_new_option("-TARG:proc=st240");
    break;
  case PROC_ST210:
    flag = add_new_option("-TARG:proc=st210");
    break;
  }

  if (proc != PROC_NONE) {
    add_st200_phase_for_option(flag);
  }

  extern int packing_level;

  if (packing_level != -1) {
    buffer_t buf;
    sprintf(buf, "-TARG:packing_level=%d", packing_level);
    flag = add_new_option(buf);
    add_st200_phase_for_option(flag);
  }
#endif


#ifdef TARG_STxP70
  /* [VCdV] Handle multiply flag processed in stxp70_options.i file
   * and pass it to back-end
   */
  extern boolean STxP70mult;
  if (STxP70mult) {
    flag = add_new_option("-TARG:enable_mx=on");
    add_phase_for_option(flag, P_be);
    if (!already_provided(flag)) {
      prepend_option_seen (flag);
    }
    ExtensionSeen=1;
  }

  //[dt] Best way of detecting default arch from environment
  char *proc_env_name=NULL;
  if (proc == UNDEFINED && (proc_env_name = getenv("SXARCHITECTURE")) != NULL) {
    int i;
    if (same_string(proc_env_name,"stxp70v4")) 	toggle (&proc, PROC_stxp70_v4_single);
    else if (same_string(proc_env_name,"stxp70v3")) 	toggle (&proc, PROC_stxp70_v3);
    else {
      for (i = 0; Proc_Map[i].pname != NULL; i++) {
        if (same_string(proc_env_name, Proc_Map[i].pname)) {
          toggle (&proc, Proc_Map[i].pid);
        }
      }
    }
  }
  extern
  unsigned int corecfg1;

  switch (proc) {
    case PROC_stxp70_v4_dual_arith:
    case PROC_stxp70_v4_single:
    case PROC_stxp70_v4_dual:
      proc = UNDEFINED; /* [HC] to prevent warning emission in toggle if proc==PROC_stxp70_v3 */
      switch ((corecfg1 & (3<<10))>>10) {
	case 0x1: /* v4 dual issue, single core ALU */
	  if (bundlingas!=FALSE) { /* in combination with --no-bundle ? */
             toggle (&proc, PROC_stxp70_v4_dual); 
	  } else {
	     toggle (&proc, PROC_stxp70_v4_single);
	  }
          break;
	case 0x3: /* v4 dual issue, dual core ALU */
	  if (bundlingas!=FALSE) { /* in combination with --no-bundle ? */
             toggle (&proc, PROC_stxp70_v4_dual_arith); 
	  } else {
	     toggle (&proc, PROC_stxp70_v4_single);
          }
	  break;
        case 0x0: /* v4 Single issue */
	default:
          toggle (&proc, PROC_stxp70_v4_single); break;
      }
      change_phase_name(P_as,"stxp70v4-as");
      change_phase_name(P_gas,"stxp70v4-as");
      change_phase_name(P_ld,"stxp70v4-ld");
      change_phase_name(P_ipa_link,"stxp70v4-ipa_link");
#ifdef BCO_ENABLED
      change_phase_name(P_ldsimple,"stxp70v4-ld");
      change_phase_name(P_binopt,"stxp70v4-binopt");
#endif
      if (ipa == TRUE) {
	error("IPA is not supported on STxP70 v4 architecture");
      }
      break;
    case PROC_stxp70_v3:
      if (bundlingas==FALSE) {
        warning("--no-bundle is not supported by STxP70 v3 architecture");
      }
      switch ((corecfg1 & (3<<10))>>10) {
	case 0x1: /* v4 dual issue, single core ALU */
	case 0x3: /* v4 dual issue, dual core ALU */
          warning("Dual issue is not supported by STxP70 v3 architecture");
	  corecfg1 &= ~(3<<10);
          break;
      }
      change_phase_name(P_as,"stxp70v3-as");
      change_phase_name(P_gas,"stxp70v3-as");
      change_phase_name(P_ld,"stxp70v3-ld");
      change_phase_name(P_ipa_link,"stxp70v3-ipa_link");
#ifdef BCO_ENABLED
      change_phase_name(P_ldsimple,"stxp70v3-ld");
      change_phase_name(P_binopt,"stxp70v3-binopt");
#endif
      break;
  }
  
  /*
   * [VB] Force no tail recursion when -pg or -finstrument-functions
   */
  if (option_was_seen(O_pg) || option_was_seen(O_finstrument_functions)) {
    flag = add_new_option("-WOPT:tail_recursion=false");
    add_phase_for_option(flag, P_be);
    if (!already_provided(flag)) {
      add_option_seen (flag);
    }
  }

  old_dir = get_phase_dir(P_library);
  new_P_library_dir = string_copy(old_dir);
  old_dir = get_phase_dir(P_startup);
  new_P_startup_dir = string_copy(old_dir);
  switch (proc) {
  case UNDEFINED:
    toggle(&proc, PROC_stxp70_v3);
    /* fallthru stxp70 default. */
  case PROC_stxp70_v3:
    flag = add_new_option("-TARG:proc=stxp70_v3");
    if (NULL != (ptr = strstr(new_P_library_dir,"stxp70v4/lib"))) {
      ptr[7] = '3';
      set_phase_dir(get_phase_mask(P_library),new_P_library_dir);
    } else if (NULL != (ptr = strstr(new_P_library_dir,"stxp70v4\\lib"))) {
      ptr[7] = '3';
      set_phase_dir(get_phase_mask(P_library),new_P_library_dir);
    }
    if (NULL != (ptr = strstr(new_P_startup_dir,"stxp70v4/lib"))) {
       ptr[7] = '3';
       set_phase_dir(get_phase_mask(P_startup),new_P_startup_dir);
    } else if (NULL != (ptr = strstr(new_P_startup_dir,"stxp70v4\\lib"))) {
       ptr[7] = '3';
       set_phase_dir(get_phase_mask(P_startup),new_P_startup_dir);
    }
    break;
  case PROC_stxp70_v4_single:
  case PROC_stxp70_v4_dual:
  case PROC_stxp70_v4_dual_arith:
    if (proc == PROC_stxp70_v4_single) {
       flag = add_new_option("-TARG:proc=stxp70_v4_single");
    } else if (proc == PROC_stxp70_v4_dual) {
       flag = add_new_option("-TARG:proc=stxp70_v4_dual");
    } else if (proc == PROC_stxp70_v4_dual_arith) {
       flag = add_new_option("-TARG:proc=stxp70_v4_dual_arith");
    }
    if (NULL != (ptr = strstr(new_P_library_dir,"stxp70v3/lib"))) {
      ptr[7] = '4';
      set_phase_dir(get_phase_mask(P_library),new_P_library_dir);
    } else if (NULL != (ptr = strstr(new_P_library_dir,"stxp70v3\\lib"))) {
      ptr[7] = '4';
      set_phase_dir(get_phase_mask(P_library),new_P_library_dir);
    }
    if (NULL != (ptr = strstr(new_P_startup_dir,"stxp70v3/lib"))) {
       ptr[7] = '4';
       set_phase_dir(get_phase_mask(P_startup),new_P_startup_dir);
    } else if (NULL != (ptr = strstr(new_P_startup_dir,"stxp70v3\\lib"))) {
       ptr[7] = '4';
       set_phase_dir(get_phase_mask(P_startup),new_P_startup_dir);
    }
    break;
  }

  if (proc != PROC_NONE) {
    add_stxp70_phase_for_option(flag);
  }

  extern
    enum { hwloop_default = -1, hwloop_none, hwlooponly, jrgtudeconly, hwloop_all}
  hwloop_mapping;
  extern
    unsigned int corecfg;
    
  /* -1 = default ; 0 : none 1: hwlooponly 2; jrgtudeconly 3: all  */
  int core_has_hwloop = (corecfg & 0x300000) != 0;
  /* select hwloop_mapping depending on olevel, osize hwloop option */
  /* and core configuration                                         */

  if (!core_has_hwloop) { /* configuration without HW Loops */ 

    /* a warning is emmited if the user specified -Mhwloop=all */
    if (hwloop_mapping==hwloop_all) {
      warning("HW loop mapping deactivated due to core configuration - considers default instead");
      hwloop_mapping=hwloop_default; /* mapping as default instead */
    }

    /* if default chosen, mapping is set according to optimization level*/
    if (hwloop_mapping==hwloop_default) {
      if (olevel>0)
        hwloop_mapping = jrgtudeconly;
      else
        hwloop_mapping = hwloop_none;
    }
  } else if (hwloop_mapping==hwloop_default) { /* configuration with HW Loops & default */

    /* as default chosen, mapping is set according to optimization level*/
    if (olevel>0) {
        if (osize>0)
          hwloop_mapping= jrgtudeconly;
        else
          hwloop_mapping= hwloop_all;
    } else {
        hwloop_mapping = hwloop_none;
    }
  }

  add_stxp70_int_option("-TARG:activate_hwloop=%d", hwloop_mapping, P_be);
  add_stxp70_int_option("-TARG:core_has_hwloop=%d", core_has_hwloop?  1 : 0, P_be);

  /* pass memspace align options to back-end */
  extern int da_mem, sda_mem, tda_mem;
  if (da_mem>0) 
    add_stxp70_int_option("-TARG:da_mem=%d", da_mem, P_be);
  if (sda_mem>0) 
    add_stxp70_int_option("-TARG:sda_mem=%d", sda_mem, P_be);
  if (tda_mem>0) 
    add_stxp70_int_option("-TARG:tda_mem=%d", tda_mem, P_be);

  if ( (da_mem & sda_mem) || (da_mem & tda_mem) ||
       (sda_mem & tda_mem))
    {
      warning("-Mda/-Msda/-Mtda options are conflicting !\n"
              "Priority DA>SDA>TDA will be used.\n");
    }

  /* pass memspace size options to back-end */
  extern int da_minsize, da_maxsize;
  extern int sda_minsize, sda_maxsize;
  extern int tda_minsize, tda_maxsize;
  if (da_minsize>da_maxsize) {
    warning(" da range incorrect %d > %d !", da_minsize, da_maxsize);
    da_minsize=da_maxsize=0;
  }
  if (sda_minsize>sda_maxsize) {
    warning("sda range incorrect %d > %d !", sda_minsize, sda_maxsize);
    sda_minsize=sda_maxsize=0;
  }
  if (tda_minsize>tda_maxsize) {
    warning("tda range incorrect %d > %d !", tda_minsize, tda_maxsize);
    tda_minsize=tda_maxsize=0;
  }
  check_range("da", "sda", da_minsize, da_maxsize, sda_minsize, sda_maxsize);
  check_range("da", "tda", da_minsize, da_maxsize, tda_minsize, tda_maxsize);
  check_range("sda", "tda",sda_minsize, sda_maxsize, tda_minsize, tda_maxsize);
              
  if (da_minsize>0) 
    add_stxp70_int_option("-TARG:da_minsize=%d", da_minsize, P_be);
  if (da_maxsize>0)
     add_stxp70_int_option("-TARG:da_maxsize=%d", da_maxsize, P_be);
  if (sda_minsize>0) 
    add_stxp70_int_option("-TARG:sda_minsize=%d", sda_minsize, P_be);
  if (sda_maxsize>0)
    add_stxp70_int_option("-TARG:sda_maxsize=%d", sda_maxsize, P_be);
  if (tda_minsize>0) 
    add_stxp70_int_option("-TARG:tda_minsize=%d", tda_minsize, P_be);
  if (tda_maxsize>0)
    add_stxp70_int_option("-TARG:tda_maxsize=%d", tda_maxsize, P_be);

  if ((da_mem || sda_mem || tda_mem) &&
      (da_minsize || da_maxsize || sda_minsize || sda_maxsize ||
       tda_minsize || tda_maxsize))
    {
      warning(" use of memory placement options based on alignment\n"
             " simultaneously with memory placement options based on size\n"
             " is conflicting. Priority is on size then on alignment.\n");
    }

  connect_extensions();
#endif

#ifdef TARG_ARM
  switch (proc) {
  case UNDEFINED:
    toggle(&proc, PROC_armv5);
    /* fallthru arm default (armv5). */
  case PROC_armv5:
    flag = add_new_option("-TARG:proc=armv5");
    break;
  case PROC_armv6:
    flag = add_new_option("-TARG:proc=armv6");
    break;
  }

  if (proc != PROC_NONE) {
    add_arm_phase_for_option(flag);
  }
#endif

  if (abi == UNDEFINED) {
#ifdef IA64
	toggle(&abi, ABI_I64);
    	add_option_seen ( O_i64 );
#elif IA32
	toggle(&abi, ABI_IA32);
    	add_option_seen ( O_ia32 );
#elif ST100
	toggle(&abi, ABI_ST100);
    	add_option_seen ( O_st100 );
#elif ST200
	toggle(&abi, ABI_ST200_embedded);
#elif defined( TARG_STxP70 )
	toggle(&abi, ABI_STxP70_embedded);
#elif defined( TARG_ARM )
	toggle(&abi, ABI_ARM_ver1);
#else
	warning("abi should have been specified by driverwrap");
  	/* If nothing is defined, default to -n32 */
    	toggle ( &abi, ABI_N32 );
    	add_option_seen ( O_n32 );
#endif
  }

  /* Check ABI against ISA: */
  if ( isa != UNDEFINED ) {
    switch ( abi ) {
#ifdef TARG_MIPS
      case ABI_N32:
	if ( isa < ISA_MIPS3 ) {
	  add_option_seen ( O_mips3 );
	  warning ( "ABI specification %s conflicts with ISA "
		    "specification %s: defaulting ISA to mips3",
		    get_toggle_name (&abi),
		    get_toggle_name (&isa) );
	  option_name = get_option_name ( O_mips3 );
	  isa = UNDEFINED;	/* To avoid another message */
	  toggle ( &isa, ISA_MIPS3 );
	}
	break;

      case ABI_64:
	if ( isa < ISA_MIPS3 ) {
	  /* Default to -mips4 if processor supports it: */
	  if ( proc == UNDEFINED || proc >= PROC_R5K ) {
	    opt_id = O_mips4;
	    opt_val = ISA_MIPS4;
	    add_option_seen ( O_mips4 );
	  } else {
	    opt_id = O_mips3;
	    opt_val = ISA_MIPS3;
	    add_option_seen ( O_mips3 );
	  }
	  warning ( "ABI specification %s conflicts with ISA "
		    "specification %s: defaulting ISA to mips%d",
		    get_toggle_name (&abi),
		    get_toggle_name (&isa),
		    opt_val );
	  option_name = get_option_name ( opt_id );
	  isa = UNDEFINED;	/* To avoid another message */
	  toggle ( &isa, opt_val );
	}
	break;
#endif
    }

  } else {
    /* ISA is undefined, so derive it from ABI and possibly processor: */

    switch ( abi ) {
#ifdef TARG_MIPS
      case ABI_N32:
      case ABI_64:
        if (default_isa == ISA_MIPS3) {
	  opt_val = ISA_MIPS3;
	  opt_id = O_mips3;
	}
	else if (default_isa == ISA_MIPS4) {
	  opt_val = ISA_MIPS4;
	  opt_id = O_mips4;
	}
	else if (abi == ABI_64 && proc != PROC_R4K) {
	  opt_val = ISA_MIPS4;
	  opt_id = O_mips4;
	}
	else {
	  opt_val = ISA_MIPS3;
	  opt_id = O_mips3;
	}
	toggle ( &isa, opt_val );
	add_option_seen ( opt_id );
	option_name = get_option_name ( opt_id );
	break;
#endif
      case ABI_I32:
      case ABI_I64:
	opt_val = ISA_IA641;
	toggle ( &isa, opt_val );
	break;
      case ABI_ST100:
	opt_val = ISA_ST100;
	toggle ( &isa, opt_val );
	break;
      case ABI_ST200_embedded:
      case ABI_ST200_PIC:
	opt_val = ISA_ST220;
	toggle ( &isa, opt_val );
	break;
      case ABI_ARM_ver1:
      case ABI_ARM_ver2:
	opt_val = ISA_armv5;
	toggle ( &isa, opt_val );
	break;
      case ABI_STxP70_embedded:
	opt_val = ISA_stxp70_v3;
	toggle ( &isa, opt_val );
	break;
      case ABI_STxP70_fpx:
	opt_val = ISA_stxp70_v3;
	toggle ( &isa, opt_val );
	break;
      case ABI_IA32:
	opt_val = ISA_IA32;
	toggle ( &isa, opt_val );
	break;
    }
  }
  if (isa == UNDEFINED) {
	internal_error ("isa should have been defined by now");
  }

  /* Check ABI against processor: */
  if ( proc != UNDEFINED ) {
    switch ( abi ) {
#ifdef TARG_MIPS
      case ABI_N32:
      case ABI_64:
	if ( proc < PROC_R4K ) {
	  warning ( "ABI specification %s conflicts with processor "
		    "specification %s: defaulting processor to r10000",
		    get_toggle_name (&abi),
		    get_toggle_name (&proc) );
	  option_name = get_option_name ( O_r10000 );
	  proc = UNDEFINED;	/* To avoid another message */
	  add_option_seen ( O_r10000 );
	  toggle ( &proc, PROC_R10K );
	}
	break;
#endif
    }
  }

  /* Check ISA against processor: */
  if ( proc != UNDEFINED ) {
    switch ( isa ) {
#ifdef TARG_MIPS
      case ISA_MIPS1:
	/* Anything works: */
	break;

      case ISA_MIPS2:
      case ISA_MIPS3:
	if ( proc < PROC_R4K ) {
	  warning ( "ISA specification %s conflicts with processor "
		    "specification %s: defaulting processor to r10000",
		    get_toggle_name (&isa),
		    get_toggle_name (&proc) );
	  add_option_seen ( O_r10000 );
	  proc = UNDEFINED;	/* To avoid another message */
	  option_name = get_option_name ( O_r10000 );
	  toggle ( &proc, PROC_R10K );
	}
	break;

      case ISA_MIPS4:
	if ( proc < PROC_R5K ) {
	  warning ( "ISA specification %s conflicts with processor "
		    "specification %s: defaulting processor to r10000",
		    get_toggle_name (&isa),
		    get_toggle_name (&proc) );
	  add_option_seen ( O_r10000 );
	  proc = UNDEFINED;	/* To avoid another message */
	  option_name = get_option_name ( O_r10000 );
	  toggle ( &proc, PROC_R10K );
	}
	break;
#endif
    }
  }
  else if (default_proc != UNDEFINED) {
	/* set proc if compatible */
	opt_id = 0;
#ifdef TARG_MIPS
	switch (default_proc) {
	case PROC_R4K:
		if (isa <= ISA_MIPS3) {
			opt_id = O_r4000;
		}
		break;
	case PROC_R5K:
		opt_id = O_r5000;
		break;
	case PROC_R8K:
		opt_id = O_r8000;
		break;
	case PROC_R10K:
		opt_id = O_r10000;
		break;
	}
#endif
	if (abi == ABI_I64 || abi == ABI_IA32) {
		opt_id = 0;	/* no proc for i64, ia32 yet */
	}
	/* ST100 may have more than one implementation in the future */
	if (abi == ABI_ST100 || 
	    abi == ABI_ST200_embedded || abi == ABI_ST200_PIC ||
	    abi == ABI_STxP70_embedded || abi == ABI_STxP70_fpx ||
	    abi == ABI_ARM_ver1 || abi == ABI_ARM_ver2
	    ) {
	  opt_id = 0;
	}
	if (opt_id != 0) {
		add_option_seen ( opt_id );
		option_name = get_option_name ( opt_id );
		toggle ( &proc, default_proc);
	}
  }

  if ( debug ) {
    fprintf ( stderr, "Check_Target done; ABI=%d ISA=%d Processor=%d\n",
	      abi, isa, proc );
  }
}

/* ====================================================================
 *
 * Routines to manage inlining choices (the -INLINE group and friends).
 *
 * ====================================================================
 */

/* toggle inline for a normal option (not "=on" or "=off") */

static void
toggle_inline_normal(void)
{
  if (inline_t == UNDEFINED)
    inline_t = TRUE;
}

/* toggle inline for "=on" */

static void
toggle_inline_on(void)
{
  if (inline_t == FALSE) {
    warning ("-noinline or -INLINE:=off has been seen, %s ignored",
	     option_name);
  }
  else {

    inline_t = TRUE;
    inline_on_seen = TRUE;
  }
}

/* toggle inline for "=off" */

static void
toggle_inline_off(void)
{
  if (inline_on_seen == TRUE) {
    warning ("Earlier request for inline processing has been overridden by %s",
	     option_name);
  }
  inline_t = FALSE;
}

/* process -INLINE option */
void
Process_Inline ( void )
{
  int more_symbols = TRUE;
  char *args = option_name+7;

  if (strncmp (option_name, "-noinline", 9) == 0)
      toggle_inline_off();
  else if (*args == '\0')
    /* Treat "-INLINE" like "-INLINE:=on" for error messages */
    toggle_inline_on();
  else do {
    char *endc;
    *args = ':';
    if ((endc = strchr(++args, ':')) == NULL)
      more_symbols = FALSE;
    else
      *endc = '\0';
    if (strcasecmp(args, "=off") == 0)
      toggle_inline_off();
    else if (strcasecmp(args, "=on") == 0)
      toggle_inline_on();
    else
      toggle_inline_normal();
    args = endc;
  }
  while (more_symbols);
}

/*
 * Processing -F option: ratfor-related stuff for Fortran, but
 * (obsolete) C code generation option in C++ and unknown for C.
 */
void dash_F_option(void)
{
    if (invoked_lang == L_f77) {
	last_phase=earliest_phase(P_ratfor,last_phase);
    } else if (invoked_lang == L_CC) {
	error("-F is not supported: cannot generate intermediate C code");
    } else {
	parse_error("-F", "unknown flag");
    }
}

/* untoggle the object, so it can be re-toggled later */
extern void
untoggle (int *obj, int value)
/*ARGSUSED*/
{
  *obj = UNDEFINED;
}

/* change path for particular phase(s), e.g. -Yb,/usr */
static void
change_phase_path (string arg)
{
	string dir;
	string s;
	for (s = arg; s != NULL && *s != NIL && *s != ','; s++)
		;
	if (s == NULL || *s == NIL) {
		parse_error(option_name, "bad syntax for -Y option");
		return;
	}
	dir = s+1;
#ifndef __MINGW32__
	if (dir[0] == '~' && (dir[1] == '/' || dir[1] == '\0')) {
	    char *home = getenv("HOME");
	    if (home)
		dir = concat_strings(home, dir+1);
	}
#endif
	if (!is_directory(dir))
		parse_error(option_name, "not a directory");
	for (s = arg; *s != ','; s++) {
		/* do separate check so can give better error message */
		if (get_phase(*s) == P_NONE) {
			parse_error(option_name, "bad phase for -Y option");
		} else {
			set_phase_dir(get_phase_mask(get_phase(*s)), dir);
		}
	}
}

/* halt after a particular phase, e.g. -Hb */
/* but also process -H and warn its ignored */
static void
change_last_phase (string s)
{
	phases_t phase;
	if (s == NULL || *s == NIL) {
		warn_ignored("-H");
	} else if ( *(s+1)!=NIL) {
		parse_error(option_name, "bad syntax for -H option");
	} else if ((phase=get_phase(*s)) == P_NONE) {
			parse_error(option_name, "bad phase for -H option");
	} else {
			last_phase=earliest_phase(phase, last_phase);
	}
}

extern void
save_name (string *obj, string value)
{
	*obj = string_copy(value);
}

static void
check_output_name (string name)
{
	if (name == NULL) return;
	if (get_source_kind_from_suffix (get_suffix(name)) != S_o && file_exists(name) && strcmp(option_name,name)) {
		warning("%s %s will overwrite a file that has a source-file suffix", option_name, name);
	}
}

void
check_dashdash (void)
{
	if(xpg_flag)
	   dashdash_flag = 1;
	else
	   error("%s not allowed in non XPG4 environment", option_name);
}

static string
Get_Binary_Name ( string name)
{
  string new;
  int len, i;
  new = string_copy(name);
  len = strlen(new);
  for ( i=0; i<len; i++ ) {
    if (strncmp(&new[i], ".x.Counts", 9) == 0) {
      new[i] = 0;
      break;
    }
  }
  return new;
}
 
void
Process_fbuse ( char *fname )
{
static boolean is_first_count_file = TRUE;
Use_feedback = TRUE;
add_string (count_files, fname);
if (is_first_count_file && (prof_file == NULL))
  prof_file = Get_Binary_Name(drop_path(fname));
is_first_count_file = FALSE;
}

static int instr_flag;
void
Process_fb_nocreate ( char *fname )
{
  toggle ( &instrumentation_invoked, UNDEFINED);
  fb_file = NULL;
  set_option_unseen(instr_flag);
}

void
Process_fb_create ( char *fname )
{
   fb_file = string_copy(fname);

   if (instrumentation_invoked == TRUE) {
	/* instrumentation already specified */
   	instr_flag = add_string_option (O_OPT_, "instr_unique_output=on");
   }
   else {
   	toggle ( &instrumentation_invoked, TRUE );
	  //          instr_flag = add_string_option (O_OPT_, "instr=on:instr_unique_output=on");
	  instr_flag = add_string_option (O_OPT_, "instr=0:instr_unique_output=on");
   }
   add_option_seen (instr_flag);
}

void
Process_fb_opt ( char *fname )
{
   fb_file = string_copy(fname);
   toggle ( &instrumentation_invoked, FALSE);
}

void
Process_fbexe ( char *fname )
{
  prof_file = string_copy(fname);
}

void
Process_fb_xdir ( char *fname )
{
  fb_xdir = string_copy(fname);
}

void
Process_fb_cdir ( char *fname )
{
  fb_cdir =  string_copy(fname);
}

typedef enum {
  DSM_UNDEFINED,
  DSM_OFF,
  DSM_ON
} DSM_OPTION;

static DSM_OPTION dsm_option=DSM_UNDEFINED;
static DSM_OPTION dsm_clone=DSM_UNDEFINED;
static DSM_OPTION dsm_check=DSM_UNDEFINED;

void
set_dsm_default_options (void)
{
  if (dsm_option==DSM_UNDEFINED) dsm_option=DSM_ON;
  if (dsm_clone==DSM_UNDEFINED && invoked_lang != L_CC) dsm_clone=DSM_ON;
  if (dsm_check==DSM_UNDEFINED) dsm_check=DSM_OFF;
}

void
reset_dsm_default_options (void)
{
  dsm_option=DSM_OFF;
  dsm_clone=DSM_OFF;
  dsm_check=DSM_OFF;
}

void
set_dsm_options (void)
{

  if (dsm_option==DSM_ON) {
    add_option_seen(O_dsm);
  } else {
    reset_dsm_default_options();
    if (option_was_seen(O_dsm))
      set_option_unseen(O_dsm); 
  }

  if (dsm_clone==DSM_ON) 
    add_option_seen(O_dsm_clone);
  else
    if (option_was_seen(O_dsm_clone))
      set_option_unseen(O_dsm_clone); 
  if (dsm_check==DSM_ON) 
    add_option_seen(O_dsm_check);
  else
    if (option_was_seen(O_dsm_check))
      set_option_unseen(O_dsm_check); 
}

/* ====================================================================
 *
 * Process_Mp_Group
 *
 * We've found a -MP option group.  Inspect it for dsm request
 * and toggle the state appropriately.
 *
 * NOTE: We ignore anything that doesn't match what's expected --
 * the compiler will produce reasonable error messages for junk.
 *
 * ====================================================================
 */

void
Process_Mp_Group ( string mp_args )
{
  char *cp = mp_args;	/* Skip -MP: */

  if ( debug ) {
    fprintf ( stderr, "Process_Mp_Group: %s\n", mp_args );
  }

  while ( *cp != 0 ) {
    switch ( *cp ) {
      case 'd':
	if ( strncasecmp ( cp, "dsm", 3 ) == 0 &&
             (*(cp+3)==':' || *(cp+3)=='\0'))
            set_dsm_default_options();
	else if ( strncasecmp ( cp, "dsm=on", 6 ) == 0 )
            set_dsm_default_options();
	else if ( strncasecmp ( cp, "dsm=off", 7 ) == 0 )
            reset_dsm_default_options();
	else if ( strncasecmp ( cp, "dsm=true", 8 ) == 0 )
            set_dsm_default_options();
	else if ( strncasecmp ( cp, "dsm=false", 9 ) == 0 )
            reset_dsm_default_options();
	else
          parse_error(option_name, "Unknown -MP: option");
	break;
      case 'c':
	if ( strncasecmp ( cp, "clone", 5 ) == 0) {
          if ( *(cp+5) == '=' ) {
	    if ( strncasecmp ( cp+6, "on", 2 ) == 0 )
              dsm_clone=DSM_ON;
	    else if ( strncasecmp ( cp+6, "off", 3 ) == 0 )
              dsm_clone=DSM_OFF;
          } else if ( *(cp+5) == ':' || *(cp+5) == '\0' ) {
              dsm_clone=DSM_ON;
          } else
            parse_error(option_name, "Unknown -MP: option");
	} else if ( strncasecmp ( cp, "check_reshape", 13 ) == 0) {
          if ( *(cp+13) == '=' ) {
	    if ( strncasecmp ( cp+14, "on", 2 ) == 0 ) {
              dsm_check=DSM_ON;
	    } else if ( strncasecmp ( cp+14, "off", 3 ) == 0 ) {
              dsm_check=DSM_OFF;
            }
          } else if ( *(cp+13) == ':' || *(cp+13) == '\0' ) {
              dsm_check=DSM_ON;
          } else
            parse_error(option_name, "Unknown -MP: option");
	}
	else
          parse_error(option_name, "Unknown -MP: option");
	break;
    case 'm':
      if (strncasecmp (cp, "manual=off", 10) == 0) {
        set_option_unseen (O_mp);
        reset_dsm_default_options ();
      }
      else
        parse_error(option_name, "Unknown -MP: option");
      break;
    case 'o':
      if (strncasecmp (cp, "open_mp=off", 11) == 0) {
	 Disable_open_mp = TRUE;
      } else if (strncasecmp (cp, "old_mp=off", 10) == 0) {
	 Disable_old_mp = TRUE;
      } else if ((strncasecmp (cp, "open_mp=on", 10) == 0) ||
		 (strncasecmp (cp, "old_mp=on", 9) == 0)) {
           /* No op; do nothing */
      } else {
	 parse_error(option_name, "Unknown -MP: option");
      }
      break;
    default:
          parse_error(option_name, "Unknown -MP: option");
    }

    /* Skip to the next group option: */
    while ( *cp != 0 && *cp != ':' ) ++cp;
    if ( *cp == ':' ) ++cp;
  }

  if ( debug ) {
    fprintf ( stderr, "Process_Dsm_Group done\n" );
  }
}

void
Process_Mp ( void )
{

  if ( debug ) {
    fprintf ( stderr, "Process_Mp\n" );
  }

  if (!option_was_seen (O_mp)) {
    /* avoid duplicates */
    add_option_seen (O_mp);
  }
  set_dsm_default_options();

  if ( debug ) {
    fprintf ( stderr, "Process_Mp done\n" );
  }
}

void Process_Cray_Mp (void) {

  if (invoked_lang == L_f90) {
    /* this part is now empty (we do the processing differently)
     * but left as a placeholder and error-checker.
     */
  }
  else error ("-cray_mp applicable only to f90");
}

void
Process_Promp ( void )
{

  if ( debug ) {
    fprintf ( stderr, "Process_Promp\n" );
  }

  /* Invoke -PROMP:=on for f77,f90 -mplist for C, and nothing for
   * other languages.
   */
  if (invoked_lang == L_f77 || invoked_lang == L_f90) {
    add_option_seen ( O_promp );
    add_option_seen(add_string_option(O_FE_, "endloop_marker=1"));
  } else if (invoked_lang == L_cc) {
    /* add_option_seen(O_mplist); */
    add_option_seen ( O_promp );
  }
  if ( debug ) {
    fprintf ( stderr, "Process_Promp done\n" );
  }
}

void
print_file_path (string fname)
{
  /* Search for fname in usual places, and print path when found. */
  /* gcc does separate searches for libraries and programs,
   * but that seems redundant as the paths are nearly identical,
   * so try combining into one search. */
  string path;
  path = concat_path(get_phase_dir(P_be), fname);
  if (file_exists(path)) {
	printf("%s\n", path);
	return;
  }
  path = concat_path(get_phase_dir(P_library), fname);
  if (file_exists(path)) {
	printf("%s\n", path);
	return;
  }
#if defined(linux) || defined(sun) || defined(__CYGWIN__) || defined(__MINGW32__)
  path = concat_path(get_phase_dir(P_gcpp), fname);
  if (file_exists(path)) {
	printf("%s\n", path);
	return;
  }
  path = concat_path(get_phase_dir(P_gas), fname);
  if (file_exists(path)) {
	printf("%s\n", path);
	return;
  }
  path = concat_path(get_phase_dir(P_alt_library), fname);
  if (file_exists(path)) {
	printf("%s\n", path);
	return;
  }
#endif
  /* not found, so just print fname */
  printf("%s\n", fname);
}

#ifdef BCO_ENABLED /* Thierry */

/* ====================================================================
 *
 * Process_ICache_Group
 *
 * We've found a ---icache-opt option group.  Inspect it and toggle
 * the state appropriately.
 *
 * NOTE: We ignore anything that doesn't match what's expected -- the
 * compiler will produce reasonable error messages for junk.
 *
 * ====================================================================
 */
void
Process_ICache_Group (string cache_args)
{

  char *cp = cache_args;

  if ( debug ) {
    fprintf ( stderr, "Process_ICache_Group: %s\n", cache_args );
  }

  icache_opt = Bool_Group_Value(cp);
}

/* ====================================================================
 *
 * Process_ICachestatic_Group
 *
 * We've found a --icache-static=%s option group.  Inspect it and toggle
 * the state appropriately.
 *
 * NOTE: We ignore anything that doesn't match what's expected -- the
 * compiler will produce reasonable error messages for junk.
 *
 * ====================================================================
 */
void
Process_ICachestatic_Group (string cache_args)
{

  char *cp = cache_args;

  if ( debug ) {
    fprintf ( stderr, "Process_ICachestatic_Group: %s\n", cache_args );
  }

  icache_static = Bool_Group_Value(cp);
}


/* ====================================================================
 *
 * Process_ICacheprofile_Group
 *
 * We've found a ---icache-profile=file option.  Inspect it and toggle
 * the state appropriately.
 *
 * ====================================================================
 */
void
Process_ICacheprofile_Group (string cache_args)
{
  char *cp = cache_args;

  if ( debug ) {
    fprintf ( stderr, "Process_ICacheprofile_Group: %s\n", cache_args );
  }

  if (file_exists(cp))
    icache_profile = cp;
  else
    warning ("--icache-profile=%s does not exist, option ignored ", cp);
}

/* ====================================================================
 *
 * Process_ICacheprofileExe_Group
 *
 * We've found a --icache-profile-exe=file option.  Inspect it and toggle
 * the state appropriately.
 *
 * ====================================================================
 */
void
Process_ICacheprofileExe_Group (string cache_args)
{
  char *cp = cache_args;

  if ( debug ) {
    fprintf ( stderr, "Process_ICacheprofileExe_Group: %s\n", cache_args );
  }
  if (file_exists(cp))
    icache_profile_exe = cp;
  else
    warning ("--icache-profile-exe=%s does not exist, option ignored ", cp);
}
/* ====================================================================
 *
 * Process_ICachemapping_Group
 *
 * We've found a ---icache-mapping=file option. Inspect it and toggle
 * the state appropriately.
 *
 * ====================================================================
 */
void
Process_ICachemapping_Group (string cache_args)
{
  char *cp = cache_args;

  if ( debug ) {
    fprintf ( stderr, "Process_ICachemapping_Group: %s\n", cache_args );
  }

  if (file_exists(cp))
    icache_mapping = cp;
  else
    warning ("--icache-mapping=%s does not exist, option ignored ", cp);
}

/* ====================================================================
 *
 * Process_ICachealgo_Group
 *
 * We've found a ---icache-algo=name option group.  Inspect it and toggle
 * the state appropriately.
 *
 * NOTE: We ignore anything that doesn't match what's expected -- the
 * compiler will produce reasonable error messages for junk.
 *
 * ====================================================================
 */
void
Process_ICachealgo_Group (string cache_args)
{
  char *cp = cache_args;

  if ( debug ) {
    fprintf ( stderr, "Process_ICachealgo_Group: %s\n", cache_args );
  }

  if ( strncasecmp ( cp, "ph", 2 ) == 0) 
    icache_algo = algo_PH;
  else if ( strncasecmp ( cp, "col", 3 ) == 0) 
    icache_algo = algo_COL;
  else if ( strncasecmp ( cp, "ph_col", 6 ) == 0) 
    icache_algo = algo_PH_COL;
  else if ( strncasecmp ( cp, "trg", 3 ) == 0) 
    icache_algo = algo_TRG;
  else if ( strncasecmp ( cp, "ltrg", 4 ) == 0) 
    icache_algo = algo_LTRG;
  else
    warning("Unknown --icache-algo=%s option", cp);
}

#endif /* BCO_Enabled Thierry */

#ifdef TARG_ST
void
Process_Std(string option_args) {
  int flag = LAST_PREDEFINED_OPTION ;

  if ( debug ) {
    fprintf ( stderr, "Process_Std: %s\n", option_args);
  }

  /* Select the appropriate language standard.  We currently
     recognize:
     -std=iso9899:1990		same as -ansi
     -std=iso9899:199409	ISO C as modified in amend. 1
     -std=iso9899:1999		ISO C 99
     -std=c89			same as -std=iso9899:1990
     -std=c99			same as -std=iso9899:1999
     -std=gnu89			default, iso9899:1990 + gnu extensions
     -std=gnu99			iso9899:1999 + gnu extensions
     (cbr) recognize c++ standards
     -std=c++98                 iso14882
     -std=gnu++98               iso14882 + gnu extensions
  */
  if (!strcmp (option_args, "iso9899:1990")
      || !strcmp (option_args, "c89")) {
      flag = add_new_option("-std=c89") ;
      c_std = C_STD_C89;
      toggle(&ansi,STRICT_ANSI);
  } else if (!strcmp (option_args, "iso9899:199409")) {
      flag = add_new_option("-std=iso9899:199409") ;    
      c_std = C_STD_C94;
      toggle(&ansi,STRICT_ANSI);
  }
  else if (!strcmp (option_args, "iso9899:199x")
	   || !strcmp (option_args, "iso9899:1999")
	   || !strcmp (option_args, "c9x")
	   || !strcmp (option_args, "c99")) {
      flag = add_new_option("-std=c99") ;    
      c_std = C_STD_C99;
      toggle(&ansi,STRICT_ANSI);
  } else if (!strcmp (option_args, "gnu89")) {
      flag = add_new_option("-std=gnu89") ;    
      c_std = C_STD_GNU89;
  }
#ifdef TARG_ST
  /* (cbr) handle C++ */
  else if (!strcmp (option_args, "c++98")) {
      flag = add_new_option("-std=c++98") ;    
      c_std = C_STD_CXX98;
      toggle(&ansi,STRICT_ANSI);
  }
  else if (!strcmp (option_args, "gnu++98")) {
      flag = add_new_option("-std=gnu++98") ;    
      c_std = C_STD_GNU98;
      /* (cm) gnu++98 (default mode) does not trigger strict ansi toggle(&ansi,STRICT_ANSI); */
  }
#endif
  else if (!strcmp (option_args, "gnu9x") || !strcmp (option_args, "gnu99")) {   
      flag = add_new_option("-std=gnu99") ;    
      c_std = C_STD_GNU99;
  } else {
      warning("unknown C standard `%s'", option_args) ;
  }
  if (flag != LAST_PREDEFINED_OPTION) {
      /* Need to pass to tools recognizing this option*/
      add_phase_for_option(flag, P_gcpp);
      add_phase_for_option(flag, P_c_gfe);
      add_option_seen (flag);
  }

}

#endif

#ifdef TARG_ST
/* TB: add -Wuninitialized option with -Wall */
void Add_Wuninitialized() {
  int flag = add_new_option("-WOPT:warn_uninit=on") ;    
  add_phase_for_option(flag, P_be);
  if (!already_provided(flag)) {
    prepend_option_seen (flag);
  }
}
/* TB: add -Wuninitialized option with -Wall */
void Add_Wreturn_type() {
  int flag = add_new_option("-OPT:warn_return_void") ;    
  add_phase_for_option(flag, P_be);
  if (!already_provided(flag)) {
    prepend_option_seen (flag);
  }
}
#endif


#ifdef TARG_ST200

#ifdef MUMBLE_ST200_BSP
string st200_core, st200_soc, st200_board;
string st200_core_name, st200_soc_name, st200_board_name;
RUNTIME st200_runtime = UNDEFINED;
SYSCALL st200_syscall = UNDEFINED ;
string st200_targetdir ; /* Set iff targetdir is command-line overriden */
string st200_libdir;
#endif

void
Process_ST200_Targ (string option,  string targ_args )
{
  char *targ;
  int i;
  int flag;
  buffer_t buf;
#ifdef MUMBLE_ST200_BSP
  string spath;
#endif

  if (debug)
    fprintf ( stderr, "Process_ST200_Targ %s%s\n", option,targ_args);

#ifdef MUMBLE_ST200_BSP
  if (strncasecmp (option, "-mlibdir", 8) == 0) {
    if (is_directory(targ_args)) 
      st200_libdir = string_copy (targ_args);
    else
      warning("libdir %s undefined. ", targ_args);
  }

  if (strncasecmp (option, "-mtargetname", 12) == 0) {
      /* 
	 This option overrides the 'target' default target tree name
	 We accept to do this only if we find it as a sibling of the
	 default 'target' name
      */    
      string defaulttargbase = string_copy(get_phase_dir(P_alt_library)) ;
      string droptargetdefault = strrchr(defaulttargbase, SYS_getDirSeparator()) ;
      if (droptargetdefault) {
	  string alttargetdir ;
	  *droptargetdefault = '\0' ;
	  alttargetdir = concat_path(defaulttargbase, targ_args) ;
	  if (is_directory(alttargetdir))
	      set_phase_dir(get_phase_mask(P_alt_library), alttargetdir) ;
	  else
	    warning("targetname %s not found. setting to default", targ_args);	
      } else 
	  warning("targetname %s not found. setting to default", targ_args);	
  }

  if (strncasecmp (option, "-mtargetdir", 11) == 0) {
      /* is_directory successes if it is a file that is existing locally */
      if (is_directory(targ_args)) {
	/* Substitution should happen only if core/soc/board hiearchy exists */
	/* So we cannot use the obvious set_phase_dir (get_phase_mask(P_alt_library), targ_args) ; */
	st200_targetdir = string_copy (targ_args);
      } else {	  
	    warning("targetdir %s not found. setting to default", targ_args);
      }
  }

  if (st200_targetdir) 
    spath = st200_targetdir;
  else
    spath = get_phase_dir(P_alt_library);
#endif

  if (strncasecmp (option, "-mcore", 6) == 0) {
    for (i = 0; Proc_Map[i].pname != NULL; i++) {
      if (same_string(targ_args, Proc_Map[i].pname)) {
	toggle (&proc, Proc_Map[i].pid);
      }
    }
    if ( proc == UNDEFINED ) {
      warning("unsupported processor %s\n", targ_args);
      proc = PROC_NONE;
    }
#ifdef MUMBLE_ST200_BSP
    st200_core = concat_path(spath, concat_path("core", targ_args));
    st200_core_name = string_copy (targ_args);
#endif
  }

#ifdef MUMBLE_ST200_BSP
  else if (strncasecmp (option, "-msoc", 5) == 0) {
    st200_soc = concat_path(spath, concat_path("soc", targ_args));
    st200_soc_name = string_copy (targ_args);
  }

  else if (strncasecmp (option, "-mboard", 7) == 0) {
    st200_board = concat_path(spath, concat_path("board", targ_args));
    st200_board_name = string_copy (targ_args);
  }

  else if (strncasecmp (option, "-mruntime", 9) == 0) {
    for (i = 0; Runtime_Map[i].pname != NULL; i++) {
      if (same_string(targ_args, Runtime_Map[i].pname)) {
	toggle (&st200_runtime, Runtime_Map[i].pid);
      }
    }
    if (st200_runtime == UNDEFINED ) {
      st200_runtime = RUNTIME_BARE;
      warning("runtime %s undefined. setting to bare", targ_args);
    }
  }

  else if (strncasecmp (option, "-msyscall", 9) == 0) {
    for (i = 0; Syscall_Map[i].pname != NULL; i++) {
      if (same_string(targ_args, Syscall_Map[i].pname)) {
	toggle (&st200_syscall, Syscall_Map[i].pid);
      }
    }
    if (st200_syscall == UNDEFINED ) {
      warning("system call model %s unknown. switching to default", targ_args);
    }
  }

#endif
}

/* ====================================================================
 *
 * Process_ST200_OS21_Trace and helpers
 *
 * Tables of options to later on to linker are maintained
 * These tables are updated from options and then parsing the files that they imply
 *
 * ====================================================================
 */

/* The following functions deal with the management of the tables that
record linker options derived from the pseudo linker script parsing */
enum { HTAB_DEFAULT_SIZE = 256 /* We get ~160 on our .ld examples */} ;
static htab_t htab_undefined_names;
static htab_t htab_wrap_names;

static hashval_t
pseudo_ld_names_hash (const void *p) {
  return htab_hash_string ((const char *)p);
}

static int
pseudo_ld_names_eq (const void *p1, const void *p2) {
  return strcmp ((const char *)p1, (const char *)p2) == 0;
}

/* Strings are duplicated by xstrdup, so they can be freed upon deletion */
static void 
pseudo_ld_names_elm_del (void *p) {
  free(p); 
}

/* Internal helper to add a symbol in a table */
static void
add_specific_symbol (htab_t htab, const char *name) {
  void **slot ;
  slot = htab_find_slot (htab, name, INSERT) ;
  if (slot) *slot = xstrdup(name) ;
}

/* Internal helper to remove a symbol in a table*/
static void
del_specific_symbol (htab_t htab, const char *name) {
  void **slot ;
  slot = htab_find_slot (htab, name, NO_INSERT) ;
  if (slot && *slot)
    htab_clear_slot (htab, slot);
}

/* The following functions deal with the pseudo linker script
   parsing */

typedef void (*on_pseudo_ld_file_record_t)(const char *key, const char *val, int cbdata) ;

/* Called upon sucessfull line entry parsing, records in 'undefined' table */
static void 
on_pseudo_ld_file_record_undefined_name(const char *key, const char *val, int cbdata)
{
    if (!htab_undefined_names) 
	 htab_undefined_names = htab_create(HTAB_DEFAULT_SIZE, pseudo_ld_names_hash,
			       pseudo_ld_names_eq, pseudo_ld_names_elm_del ) ;

    if (htab_undefined_names) {
	if (cbdata) add_specific_symbol(htab_undefined_names, val) ; 
	else del_specific_symbol(htab_undefined_names, val) ;			
    } else {
	internal_error("Unable to create hash table on_pseudo_ld_file_record_undefined_name");
    }
}

/* Called upon sucessfull line entry parsing, records in 'wrap' table */
static void 
on_pseudo_ld_file_record_wrap_name(const char *key, const char *val, int cbdata)
{
    if (!htab_wrap_names) 
	 htab_wrap_names = htab_create(HTAB_DEFAULT_SIZE, pseudo_ld_names_hash,
			       pseudo_ld_names_eq, pseudo_ld_names_elm_del ) ;

    if (htab_wrap_names) {
	if (cbdata) add_specific_symbol(htab_wrap_names, val) ; 
	else del_specific_symbol(htab_wrap_names, val) ;			
    } else {
	internal_error("Unable to create hash table on_pseudo_ld_file_record_wrap_name");
    }
}

/* Hash tables can only be traversed, the following helps recording what we encounter */
typedef struct {
    int index ; 
    int maxindex ;
    char *strings[0] ;
} traverse_function_records_t ; 

static int
htab_traverse_function (void **slot, void *data)
{
  const char *slot_name = *slot;
  traverse_function_records_t *function_record = (traverse_function_records_t *)data ;

  /* Traversal stops when all the elements have been gathered */
  if (function_record->index < function_record->maxindex ) {
      function_record->strings[function_record->index] = *slot;
      function_record->index++ ;
      return 1 ;
  } else {
      /* Stop traversal */
      return 0 ;
  }
}

/* Parse a line, scans it for key value pair, calls callback for record */
static void 
read_pseudo_ld_file_record(const char *line, on_pseudo_ld_file_record_t cback, int cbackdata)
{
  char *eol = strrchr(line,'\n');
  if(eol) *eol = 0;
  eol = strrchr(line,'\r');
  if(eol) *eol = 0;
  char *sep = strchr(line, ' ') ;
  if(sep) {
    const char *key = line ; 
    const char *value = sep + 1 ;
    *sep = 0 ;
    if (cback) 
	(*cback)(key, value, cbackdata) ;
  }
}

/* Reads pseudo ld file line-by-line, sends to parsing */
static void 
read_pseudo_ld_file(const char *filename, on_pseudo_ld_file_record_t cback, int cbackdata)
{
  FILE *file = fopen(filename,"r") ;
  if(file) {
    buffer_t line ;
    while(fgets(line,sizeof(line),file)!=0)
	read_pseudo_ld_file_record(line, cback, cbackdata) ;
    fclose(file) ;
  } else {
    warning("Cannot open OS21 trace description in %s", filename);
  }
}

/* Helper function to sort option names */
static int 
sort_names_function(const void *arg1, const void *arg2)
{
    return strcmp(*(const char **)arg1, *(const char **)arg2);
}

/* 
   The following functions expose the gathered options to the driver
   *
   * os21_trace_options_set determines if trace options must be emitted
   *
   * get_os21_trace_options_nelements gets the number of trace strings
   * for a given kind
   *
   * get_os21_trace_options_elements returns a filled array of
   * pointers to strings for a given kind, limited to max, with actual
   * return
*/
int os21_trace_options_set() {
    return (get_os21_trace_options_nelements(OS21_TRACE_WRAP) !=0) ||
	(get_os21_trace_options_nelements(OS21_TRACE_UNDEFINED) !=0) ;
}

int 
get_os21_trace_options_nelements(OS21_TRACE kind)
{
    if (kind == OS21_TRACE_WRAP && htab_wrap_names) 
	return htab_elements(htab_wrap_names) ;
    else if (kind == OS21_TRACE_UNDEFINED && htab_undefined_names) 
	return htab_elements(htab_undefined_names) ;
    else return 0 ;
}

int 
get_os21_trace_options_elements(OS21_TRACE kind, char *elts[], size_t maxnelts)
{
    traverse_function_records_t *traverse_records_names ;
    int options_nelements = 0 ;
    htab_t htab = NULL ;

    /* Select the proper table */
    if (kind == OS21_TRACE_WRAP) htab = htab_wrap_names ;
    else if (kind == OS21_TRACE_UNDEFINED) htab = htab_undefined_names ;
    
    /* If there is any record of tracing options, traverse and gather the table info */
    if (htab) {
	size_t actualelts = maxnelts < htab_elements(htab) ? maxnelts : htab_elements(htab) ; 
	/* There is a dragon here : oversize by one element 
	   Otherwise the allocation is missing a slot and this is seen by valgrind
	   Otherwise this dumps core very later on
	 */
	traverse_records_names = xmalloc(sizeof(traverse_records_names) + (actualelts + 1) * sizeof(char*));
	traverse_records_names->index=0 ;
	traverse_records_names->maxindex= actualelts ;

	htab_traverse (htab, htab_traverse_function, traverse_records_names);

	/* The actual copied number is kept by index */
	options_nelements = traverse_records_names->index  ; 

	/* Add some platform determinism here by sorting the gathered names */
	qsort(traverse_records_names->strings, 
	      options_nelements , sizeof(char *),
	      sort_names_function);

	memcpy(elts, traverse_records_names->strings, options_nelements * sizeof(char *)) ; 

	free(traverse_records_names) ;
    }

    return options_nelements ;
}

/* This is the central function for the OS21 trace support 
   Analyze the options and parameters, calls for action and record...
*/
void
Process_ST200_OS21_Trace (string option,  string targ_args )
{
  static const char *prefix = "os21" ;
  char *subdir = "os21" ;
  string os21traceoptdir ;

  if (debug)
    fprintf ( stderr, "Process_ST200_OS21_Trace option=[%s] args=?[%s]\n", option,targ_args);

  /* The pseudo .ld decription files are indeed independant from the
     target They are placed in a subdirectoy of the target tree. */
  os21traceoptdir = concat_path(st200_targetdir ? st200_targetdir : get_phase_dir(P_alt_library), subdir);

  if (debug)
    fprintf ( stderr, "Process_ST200_OS21_Trace Looking in %s \n", os21traceoptdir);

#ifdef MUMBLE_ST200_BSP
  if (strncasecmp (option, "-trace-no-constructor", 21) == 0) {
    /* Parses the contents of $(path)/$(prefix)trace.ld and
       for each line found removes the line as possible a linker
       option, if present */
      buffer_t os21traceoptfilename  ;
      string os21traceoptfile ;
      sprintf(os21traceoptfilename, "%strace.ld", prefix) ;
      os21traceoptfile = concat_path(os21traceoptdir, os21traceoptfilename) ;
      if (file_exists(os21traceoptfile)) {
	  read_pseudo_ld_file(os21traceoptfile, on_pseudo_ld_file_record_undefined_name, 0) ;
      } else {
	  warning("-trace-no-constructor used but cannot find description in %s", os21traceoptfile);
      }
  } else if (strncasecmp (option, "-trace-api-no-", 14) == 0) {
    /* Parses the contents of $(path)/$(prefix)wrap-$(api).ld and
       for each line found removes the line as a possible linker
       option, if present */
      buffer_t os21traceoptfilename  ;
      string os21traceoptfile ;
      sprintf(os21traceoptfilename, "%swrap-%s.ld", prefix, targ_args) ;
      os21traceoptfile = concat_path(os21traceoptdir, os21traceoptfilename) ;

      if (file_exists(os21traceoptfile)) {
	  read_pseudo_ld_file(os21traceoptfile, on_pseudo_ld_file_record_wrap_name, 0) ;
      } else {
	  warning("-trace-api-no-%s used but cannot find description in %s", targ_args, os21traceoptfile);
      }
  } else if (strncasecmp (option, "-trace-api-", 11) == 0) {
    /* Parses the contents of $(path)/$(prefix)wrap-$(api).ld and
       for each line found adds the line as a linker option (these
       entrie can be removed by other means (see
       -trace-api-no-$(class)) */
      buffer_t os21traceoptfilename  ;
      string os21traceoptfile ;
      sprintf(os21traceoptfilename, "%swrap-%s.ld", prefix, targ_args) ;
      os21traceoptfile = concat_path(os21traceoptdir, os21traceoptfilename) ;

      if (file_exists(os21traceoptfile)) {
	  read_pseudo_ld_file(os21traceoptfile, on_pseudo_ld_file_record_wrap_name, 1) ;
      } else {
	  warning("-trace-api-%s used but cannot find description in %s", targ_args, os21traceoptfile);
      }
  } else if (strncasecmp (option, "-trace-api", 10) == 0) {
      /* Parses the contents of $(path)/$(prefix)wrap.ld and
	 or each line found adds the line as a possible linker option
	 these entries can be removed by other means see
	 -trace-api-no-$(class))
     */
      buffer_t os21traceoptfilename  ;
      string os21traceoptfile ;
      sprintf(os21traceoptfilename, "%swrap.ld", prefix) ;
      os21traceoptfile = concat_path(os21traceoptdir, os21traceoptfilename) ;
      if (file_exists(os21traceoptfile)) {
	  read_pseudo_ld_file(os21traceoptfile, on_pseudo_ld_file_record_wrap_name, 1) ;
      } else {
	  warning("-trace-api used but cannot find description in %s", os21traceoptfile);
      }
  } else if (strncasecmp (option, "-trace-script-prefix", 20) == 0) {
      /* Creating a leak here. Note that prefixes are accounted for only after the option is set*/
      prefix = string_copy(targ_args) ;
  } else if (strncasecmp (option, "-trace", 6) == 0) {
    /* Parses the contents of $(path)/$(prefix)trace.ld and
       or each line found adds the line as a possible linker option
       these entries can be removed by other means see
       -trace-api-no-constructor)
     */
      buffer_t os21traceoptfilename  ;
      string os21traceoptfile ;
      sprintf(os21traceoptfilename, "%strace.ld", prefix) ;
      os21traceoptfile = concat_path(os21traceoptdir, os21traceoptfilename) ;
      if (file_exists(os21traceoptfile)) {
	  read_pseudo_ld_file(os21traceoptfile, on_pseudo_ld_file_record_undefined_name, 1) ;
      } else {
	  warning("-trace used but cannot find description in %s", os21traceoptfile);
      }
  } else {
      warning("OS21TRACE : Unexpected argument [%s] passed to option [%s].", targ_args, option);
  }
#endif
}
#endif /* TARG_ST200 */


#ifdef TARG_STxP70

#ifdef MUMBLE_STxP70_BSP
string stxp70_core, stxp70_soc, stxp70_board;
string stxp70_core_name, stxp70_soc_name, stxp70_board_name;
RUNTIME stxp70_runtime = UNDEFINED;
string stxp70_targetdir ; /* Set iff targetdir is command-line overriden */
#endif
string stxp70_libdir = NULL;

void
Process_STxP70_Targ (string option,  string targ_args )
{
  char *targ;
  int i;
  int flag;
  buffer_t buf;
  string spath;
  string old_dir;
  string new_P_library_dir;
  string new_P_startup_dir;
  char * ptr;

  if (debug)
    fprintf ( stderr, "Process_STxP70_Targ %s%s\n", option,targ_args);


  if (strncasecmp (option, "-mcore", 6) == 0) {
    for (i = 0; Proc_Map[i].pname != NULL; i++) {
      if (same_string(targ_args, Proc_Map[i].pname)) {
	toggle (&proc, Proc_Map[i].pid);
      }
    }
    if ( proc == UNDEFINED ) {
      warning("unsupported processor %s\n", targ_args);
      proc = PROC_NONE;
    }
  }

  if (strncasecmp (option, "-mlibdir", 8) == 0) {
    if (is_directory(targ_args)) 
      stxp70_libdir = string_copy (targ_args);
    else
      warning("libdir %s undefined. ", targ_args);
  }

  if (strncasecmp (option, "-mtargetdir", 11) == 0) {
      if (is_directory(targ_args)) {
	/* Substitution should happen only if core/soc/board hiearchy exists */
	/* So we cannot use the obvious set_phase_dir (get_phase_mask(P_alt_library), targ_args) ; */
	stxp70_targetdir = string_copy (targ_args);
      } else {
	warning("targetdir %s undefined. setting to default", targ_args);
      }
  }

  if (stxp70_targetdir) 
    spath = stxp70_targetdir;
  else
    spath = get_phase_dir(P_alt_library);

  old_dir = get_phase_dir(P_library);
  new_P_library_dir = string_copy(old_dir);
  old_dir = get_phase_dir(P_startup);
  new_P_startup_dir = string_copy(old_dir);
  switch (proc) {
    case PROC_stxp70_v4_dual:
    case PROC_stxp70_v4_single:
    case PROC_stxp70_v4_dual_arith:
      change_phase_name(P_as,"stxp70v4-as");
      change_phase_name(P_gas,"stxp70v4-as");
      change_phase_name(P_ld,"stxp70v4-ld");
      change_phase_name(P_ipa_link,"stxp70v4-ipa_link");
#ifdef BCO_ENABLED
      change_phase_name(P_ldsimple,"stxp70v4-ld");
      change_phase_name(P_binopt,"stxp70v4-binopt");
#endif
      if (NULL != (ptr = strstr(new_P_library_dir,"/stxp70v"))) {
         ptr[8] = '4';
         set_phase_dir(get_phase_mask(P_library),new_P_library_dir);
      }
      if (NULL != (ptr = strstr(new_P_startup_dir,"/stxp70v"))) {
         ptr[8] = '4';
         set_phase_dir(get_phase_mask(P_startup),new_P_startup_dir);
      }
      break;
    case PROC_stxp70_v3:
      change_phase_name(P_as,"stxp70v3-as");
      change_phase_name(P_gas,"stxp70v3-as");
      change_phase_name(P_ld,"stxp70v3-ld");
      change_phase_name(P_ipa_link,"stxp70v3-ipa_link");
#ifdef BCO_ENABLED
      change_phase_name(P_ldsimple,"stxp70v3-ld");
      change_phase_name(P_binopt,"stxp70v3-binopt");
#endif
      if (NULL != (ptr = strstr(new_P_library_dir,"/stxp70v"))) {
         ptr[8] = '3';
         set_phase_dir(get_phase_mask(P_library),new_P_library_dir);
      }
      if (NULL != (ptr = strstr(new_P_startup_dir,"/stxp70v"))) {
         ptr[8] = '3';
         set_phase_dir(get_phase_mask(P_startup),new_P_startup_dir);
      }
      break;
  }
  
  if (strncasecmp (option, "-mcore", 6) == 0) {
    for (i = 0; Proc_Map[i].pname != NULL; i++) {
      if (same_string(targ_args, Proc_Map[i].pname)) {
	toggle (&proc, Proc_Map[i].pid);
      }
    }
    if ( proc == UNDEFINED ) {
      warning("unsupported processor %s\n", targ_args);
      proc = PROC_NONE;
    }
#ifdef MUMBLE_STxP70_BSP
    sprintf(buf, "%s/core/%s", spath, targ_args);
    stxp70_core = string_copy (buf);
    stxp70_core_name = string_copy (targ_args);
#endif
  }

#ifdef MUMBLE_STxP70_BSP
  else if (strncasecmp (option, "-msoc", 5) == 0) {
    sprintf(buf, "%s/soc/%s", spath, targ_args);
    stxp70_soc = string_copy (buf);
    stxp70_soc_name = string_copy (targ_args);
  }

  else if (strncasecmp (option, "-mboard", 7) == 0) {
    sprintf(buf, "%s/board/%s", spath, targ_args);
    stxp70_board = string_copy (buf);
    stxp70_board_name = string_copy (targ_args);
  }

  else if (strncasecmp (option, "-mruntime", 9) == 0) {
    for (i = 0; Runtime_Map[i].pname != NULL; i++) {
      if (same_string(targ_args, Runtime_Map[i].pname)) {
	toggle ((int*)&stxp70_runtime, Runtime_Map[i].pid);
      }
    }
    if (stxp70_runtime == UNDEFINED ) {
      warning("runtime %s undefined.", targ_args);
    }
  }

#endif
}


#endif // STXP70

#ifdef TARG_ARM

#ifdef MUMBLE_ARM_BSP
string arm_core, arm_soc, arm_board;
string arm_core_name, arm_soc_name, arm_board_name;
RUNTIME arm_runtime = UNDEFINED;
string arm_targetdir ; /* Set iff targetdir is command-line overriden */
string arm_libdir;
#endif

void
Process_ARM_Targ (string option,  string targ_args )
{
  char *targ;
  int i;
  int flag;
  buffer_t buf;
#ifdef MUMBLE_ARM_BSP
  string spath;
#endif

  if (debug)
    fprintf ( stderr, "Process_ARM_Targ %s%s\n", option,targ_args);

#ifdef MUMBLE_ARM_BSP
  if (strncasecmp (option, "-mlibdir", 8) == 0) {
    if (is_directory(targ_args)) 
      arm_libdir = string_copy (targ_args);
    else
      warning("libdir %s undefined. ", targ_args);
  }

  if (strncasecmp (option, "-mtargetdir", 11) == 0) {
      if (is_directory(targ_args)) {
	/* Substitution should happen only if core/soc/board hiearchy exists */
	/* So we cannot use the obvious set_phase_dir (get_phase_mask(P_alt_library), targ_args) ; */
	arm_targetdir = string_copy (targ_args);
      } else {
	warning("targetdir %s undefined. setting to default", targ_args);
      }
  }

  if (arm_targetdir) 
    spath = arm_targetdir;
  else
    spath = get_phase_dir(P_alt_library);
#endif

  if (strncasecmp (option, "-mcore", 6) == 0) {
    for (i = 0; Proc_Map[i].pname != NULL; i++) {
      if (same_string(targ_args, Proc_Map[i].pname)) {
	toggle (&proc, Proc_Map[i].pid);
      }
    }
    if ( proc == UNDEFINED ) {
      warning("unsupported processor %s\n", targ_args);
      proc = PROC_NONE;
    }
#ifdef MUMBLE_ARM_BSP
    arm_core = concat_path(spath, concat_path("core", targ_args));
    arm_core_name = string_copy (targ_args);
#endif
  }

#ifdef MUMBLE_ARM_BSP
  else if (strncasecmp (option, "-msoc", 5) == 0) {
    arm_soc = concat_path(spath, concat_path("soc", targ_args));
    arm_soc_name = string_copy (targ_args);
  }

  else if (strncasecmp (option, "-mboard", 7) == 0) {
    arm_board = concat_path(spath, concat_path("board", targ_args));
    arm_board_name = string_copy (targ_args);
  }

  else if (strncasecmp (option, "-mruntime", 9) == 0) {
    for (i = 0; Runtime_Map[i].pname != NULL; i++) {
      if (same_string(targ_args, Runtime_Map[i].pname)) {
	toggle (&arm_runtime, Runtime_Map[i].pid);
      }
    }
    if (arm_runtime == UNDEFINED ) {
      arm_runtime = RUNTIME_BARE;
      warning("runtime %s undefined. setting to bare", targ_args);
    }
  }

#endif
}
#endif /* TARG_ARM */

void
Process_keep_dir ( char * dir ) {
  keep_dir = string_copy(dir);
}

extern void Process_extrcdir ( string optargs );

#include "opt_action.i"

