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

#if defined (TARG_ST) && (GNU_FRONT_END==33)
/* (cbr) gcc 3.3 upgrade */
extern char * toplev_main (INT argc, char **argv);
struct tree;
extern void WFE_Expand_Top_Level_Decl (struct tree *);
extern void WFE_Alias_Finish (void);
extern void weak_finish(void);
extern void check_gnu_errors (int *, int *);
#else
extern char * gnu_init (INT argc, char **argv, char **envp);
#endif

static INT argc_save;
static char **argv_save;
static char **envp_save;
extern INT32 Opt_Level;

extern BOOL  Enable_WFE_DFE;

#if defined (TARG_ST) && (GNU_FRONT_END==33)
/* (cbr) gcc 3.3 upgrade */
static
#endif
int WFE_Compile_File_Invoked = 0;

void WFE_Compile_File (struct tree *decl)
{
  WFE_Compile_File_Invoked = 1;
#ifndef TARG_ST
  // (cbr) done in toplev.c
  WFE_Init (argc_save, argv_save, envp_save);	/* sgi initialization */
  WFE_File_Init (argc_save, argv_save);	/* inits per source file */
#endif
  WFE_Expand_Top_Level_Decl (decl);
#ifdef TARG_ST
    // (cbr) emit ident directives if any
    if (idents_strs) {
      WFE_Idents();
    }
#endif
  weak_finish ();
#ifdef TARG_ST
  /* (cbr) create alias symbols and commit */
  WFE_Alias_Finish();
#endif
  WFE_File_Finish ();
  WFE_Finish ();
}

#if defined (TARG_ST) && (GNU_FRONT_END==295)
void (*back_end_hook) (struct tree *) = &WFE_Compile_File;
#endif

int
main ( 
  INT argc,	/* Number of command line arguments */
  char **argv,	/* Array of command line arguments */
  char **envp)	/* Array of environment pointers */
{
	INT error_count, sorry_count;
	BOOL need_inliner;
	extern int flag_preprocess_only;
	
	argc_save = argc;
	argv_save = argv;
	envp_save = envp;

#if defined (TARG_ST) && (GNU_FRONT_END==33)
	/* (cbr) gcc 3.3 upgrade */
	{
	  if (toplev_main (argc, argv))
	    exit (RC_USER_ERROR);
	}
	if (flag_preprocess_only) {
	  exit (RC_OKAY);
	}
#else
#ifdef TARG_ST
	{
	  int gnu_argc; char **gnu_argv;
	  WFE_Prepare_Gcc_Options(argc, argv, &gnu_argc, &gnu_argv);
	  Orig_Src_File_Name = gnu_init (gnu_argc, gnu_argv, envp);
	}
#else
	Orig_Src_File_Name = gnu_init (argc, argv, envp);
#endif

	compile_file (Orig_Src_File_Name);
#endif

	check_gnu_errors (&error_count, &sorry_count);
	if (error_count)
		exit (RC_USER_ERROR);
	if (sorry_count)
		exit (RC_USER_ERROR);

	WFE_Check_Errors (&error_count, &sorry_count, &need_inliner);
	if (error_count)
    		Terminate (RC_INTERNAL_ERROR) ;
  	if (need_inliner &&
            ((!Enable_WFE_DFE) || (Opt_Level > 1)))
		exit ( RC_NEED_INLINER );

	if (!WFE_Compile_File_Invoked) {
#ifndef TARG_ST
  // (cbr) done in toplev.c
		WFE_Init (argc_save, argv_save, envp_save);
		WFE_File_Init (argc_save, argv_save);
#endif
		WFE_File_Finish ();
		WFE_Finish ();
	}

	exit (RC_OKAY);
}
