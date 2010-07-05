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


#include "basic.h"
#include "config_platform.h"

/* values of options */

#define UNDEFINED	-1	/* for any undefined option */

#ifdef TARG_STxP70
#define LIB_FP_SOFT_FLOAT      0
#define LIB_FP_ACE             1
/* Control for lib_kind. */
#define LIB_STXP70_16	       0
#define LIB_STXP70_32	       1
#endif

#define ENDIAN_BIG	0
#define ENDIAN_LITTLE	1

#define EXTENDED_ANSI	0
#define KR_ANSI		1
#define STRICT_ANSI	2
#define POSIX		3

#define CALL_SHARED	0
#define NON_SHARED	1
#define DSO_SHARED	2
#define RELOCATABLE	4

#define NORMAL_MP	0
#define CRAY_MP		1

#ifdef TARG_ST
// [CG]: Define C standards (-std=... option)
#define C_STD_UNDEF	0
#define C_STD_C89	1
#define C_STD_C94	2
#define C_STD_C99	3
#define C_STD_GNU89	4
#define C_STD_GNU99	5
#define C_STD_CXX98	6   /* (cbr) for cplusplus */
#define C_STD_GNU98	7   /* (cbr) for cplusplus */
extern int c_std;

#define STRICT_ALIAS    1
#define NOSTRICT_ALIAS  2
#endif

extern boolean debug;		/* debugging turned on */

extern boolean nostdinc;	/* no standard include directory */

#ifdef TARG_ST
/* (cbr) don't exclude non c++ standard paths */
extern boolean nostdincc;	/* no C++ include directory */
#endif

extern string help_pattern;	/* pattern string for help file */

extern int inline_t;            /* toggle for inline options */

extern boolean dashdash_flag;   /* when you see -- set this flag to
				   indicate the end of the options */

extern boolean read_stdin;	/* read stdin for input */

extern boolean  xpg_flag;

extern int default_olevel;	/* default optimization level */

extern int ofast;		/* Ofast has been set */

extern int instrumentation_invoked;	/* Instrument whirl */

extern boolean ftz_crt;		/* add flush-to-zero crt */

#ifdef TARG_ST
extern char *print_name;
extern int print_kind;
#define PRINT_NONE 0
#define PRINT_FILE 1
#define PRINT_PROG 2

/* TB: to warm if olevel is not enough if uninitialized var warning is
   aked*/
extern boolean Wuninitialized_is_asked; 

#endif

/* return whether has been toggled yet */
extern boolean is_toggled (int obj);

/* set obj to value; allow many toggles; last toggle is final value */
extern void toggle (int *obj, int value);

/* Options for configuring the target: */

#ifdef TARG_STxP70
/* Handle multiply flag processed in stxp70_options.i file and pass it to back-end */
extern void Handle_Options ( void );
#endif

/* Verify that the target selection is consistent and set defaults: */
extern void Check_Target ( void );

/* process -F option (different for Fortran and C++) */
extern void dash_F_option(void);

/* untoggle the object, so it can be re-toggled later */
extern void untoggle (int *obj, int value);

/* save value in string */
extern void save_name (string *obj, string value);

/* do action associated with flag */
extern void opt_action (int optflag);

/* return whether to mark this flag as unseen */
extern boolean flag_is_superceded (int optflag);


/* check if there is a -- and _XPG is set */
extern void check_dashdash ( void );

/* set options for DSM options */
extern void set_dsm_options (void);

extern void Process_Mp (void);
extern void Process_Cray_Mp (void);

extern void print_file_path (string);	/* print path to named file */

#ifdef TARG_ST
/* print multi lib paths a la gcc */
extern void print_multi_lib ();	
extern void print_multi_os_directory ();	
#endif

#ifdef TARG_STxP70
typedef enum { got_none, got_small, got_standard, got_large} TYPE_GOT_MODEL;
extern TYPE_GOT_MODEL got_model_opt;
#endif

#ifdef TARG_ST200
typedef enum { OS21_TRACE_WRAP, OS21_TRACE_UNDEFINED } OS21_TRACE ;
extern int os21_trace_options_set() ;
extern int get_os21_trace_options_nelements(OS21_TRACE kind) ;
extern int get_os21_trace_options_elements(OS21_TRACE kind, char *elts[], size_t maxnelts) ;
extern int os21_profiler_options_set() ;
extern int get_os21_profiler_options_emit_undefined() ;
extern int get_os21_profiler_options_warn() ;
#endif
