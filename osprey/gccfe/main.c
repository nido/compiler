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


/* main driver for front end */
#include <stdio.h>
#include <stdlib.h>
#include <cmplrs/rcodes.h>
#include "wfe_misc.h"
#ifdef TARG_ST
/* (cbr) for idents_strs */
#include "wfe_dst.h"
#endif
#include "glob.h"
// #include "cmd_line.h"

// gnu_init returns file to compile, like t.i (not t.c!)
#if defined (TARG_ST) && (GNU_FRONT_END==33)
extern int toplev_main (INT argc, char **argv);
#else
extern char * gnu_init (INT argc, char **argv, char **envp);
#endif
extern void check_gnu_errors (int *, int *);

/* (cbr) wfe hooks */
void (*WFE_Hook_Ptr)(void) = NULL;

int
main ( 
  INT argc,	/* Number of command line arguments */
  char **argv,	/* Array of command line arguments */
  char **envp)	/* Array of environment pointers */
{
	INT error_count, sorry_count;
	BOOL need_inliner;

#if defined (TARG_ST) && (GNU_FRONT_END==33)
	extern int flag_preprocess_only;
	/* (cbr) gcc 3.3 upgrade */
	{
	  if (toplev_main (argc, argv))
	    exit (RC_USER_ERROR);
	}
	if (flag_preprocess_only) {
	  exit (RC_OKAY);
	}
#else
	Orig_Src_File_Name = gnu_init (argc, argv, envp);

	WFE_Init (argc, argv, envp);	/* sgi initialization */
	WFE_File_Init (argc, argv);	/* inits per source file */

	compile_file (Orig_Src_File_Name);
#endif

	check_gnu_errors (&error_count, &sorry_count);
	if (error_count) {
	  exit (RC_USER_ERROR);
	}
	if (sorry_count) {
	  exit (RC_USER_ERROR);
	}

	WFE_Check_Errors (&error_count, &sorry_count, &need_inliner);

	if (error_count)
    		Terminate (RC_INTERNAL_ERROR) ;

#ifdef TARG_ST
    // (cbr) emit ident directives if any
    if (idents_strs) {
      WFE_Idents();
    }
#endif

#ifdef TARG_ST
    /* (cbr) wfe hooks */
    weak_finish ();
    WFE_Alias_Finish();
#endif

	WFE_File_Finish ();
        WFE_Finish ();

  	if (need_inliner)
		exit ( RC_NEED_INLINER );

	exit (RC_OKAY);
}
