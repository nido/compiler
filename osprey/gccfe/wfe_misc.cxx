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


#include "W_values.h"
#include <sys/types.h>
#include <elf.h>

#include <stdarg.h>
#include "gnu_config.h"
extern "C" {
#include "gnu/system.h"
#include "gnu/tree.h"
#ifdef TARG_ST
  /* (cbr) 3.3 upgrade fix includes */
#include "gnu/real.h"
#endif
}

#include "defs.h"
#include "config.h"
#include "config_debug.h"
#include "config_list.h"
#include "config_TARG.h"
#include "controls.h"
#include "erglob.h"
#include "erlib.h"
#include "file_util.h"
#include "flags.h"
#include "glob.h"
#include "mempool.h"
#include "tracing.h"
#include "util.h"
#include "errors.h"
// #include "cmd_line.h"
#include "err_host.tab"
#if 0
// get rid of warning: MAX/MIN redefined
#include <stdarg.h>
#include "gnu_config.h"
extern "C" {
#include "gnu/system.h"
#include "gnu/tree.h"
}
#endif
#include "wn.h"
#include "wn_util.h"
#include "wn_simp.h"
#include "symtab.h"
#include "pu_info.h"
#include "ir_reader.h"
#include "ir_bwrite.h"
#include "wfe_decl.h"
#include "wfe_expr.h"
#include "wfe_dst.h"
#include "wfe_misc.h"
#include "wfe_stmt.h"
#ifdef TARG_ST
//TB : for WFE_Loader_Initialize_Register
#include "wfe_loader.h"
#endif
int WFE_Keep_Zero_Length_Structs = FALSE;

extern int optimize;

PU_Info *PU_Tree_Root = NULL;
int      wfe_invoke_inliner = FALSE;

extern void Initialize_IRB (void);	/* In lieu of irbutil.h */
extern char *asm_file_name;		/* from toplev.c */

int trace_verbose = FALSE;
// an_error_severity error_threshold = es_warning;

static BOOL Prepare_Source (void);
static void WFE_Stmt_Stack_Init (void);
static void WFE_Stmt_Stack_Free (void);

#ifdef TARG_ST
// When region optimization is enabled using -foptimize-regions, we try not
// to close a region as soon as a call stmt finishes. We try to keep it open
// and include as many calls as possible.
// If we got an opportunity but did not close a region in WFE_Stmt_Append,
// we set the following flag.
bool Did_Not_Terminate_Region = FALSE;

#include "gnu/flags.h"
#else
/* (cbr) include gnu/flags.h
// The following taken from gnu/flags.h
// our #include of flags.h gets common/util/flags.h instead
enum debug_info_level
{
  DINFO_LEVEL_NONE,     /* Write no debugging info.  */
  DINFO_LEVEL_TERSE,    /* Write minimal info to support tracebacks only.  */
  DINFO_LEVEL_NORMAL,   /* Write info for all declarations (and line table). */
  DINFO_LEVEL_VERBOSE   /* Write normal info plus #define/#undef info.  */
};
#endif

/* Specify how much debugging info to generate.  */
extern enum debug_info_level debug_info_level;
// End gnu/flags.h data decl



/* ====================================================================
 *
 * Local data.
 *
 * ====================================================================
 */

/*       MAX_DEBUG_LEVEL	2  :: Defined in flags.h */
# define DEF_DEBUG_LEVEL	0
INT8 Debug_Level = DEF_DEBUG_LEVEL;	/* -gn:	debug level */
# define MAX_MSG_LEVEL 2
# define DEF_MSG_LEVEL 2

#ifdef MONGOOSE_CIF
mUINT32 Cif_Level = 0;       	/* CIF level */
#define MAX_CIF_LEVEL 3 
#define DEF_CIF_LEVEL 2 
#endif /* MONGOOSE_CIF */

/* Default file	extensions: */
#define	IRB_FILE_EXTENSION ".B"	/* ACIR file */
#define	IRD_FILE_EXTENSION ".D"	/* Intermediate data file */
#define	ERR_FILE_EXTENSION ".e"	/* Error file */
#define	LST_FILE_EXTENSION ".l"	/* Listing file */
#define	TRC_FILE_EXTENSION ".t"	/* Trace file */
#define DSTDUMP_FILE_EXTENSION ".fe.dst" /* DST dump-file extension */

/* Static data:	command	line information: */
static INT32 Argc;		/* Copy of argc */
static char **Argv;		/* Copy of argv */
static INT32 Source_Arg;	/* Number of current source arg */
static INT32 Src_Count;		/* Number of source files seen */
static char Dash [] = "-";

/* Internal flags: */
static BOOL Echo_Flag =	FALSE;	/* Echo command	lines */
static BOOL Delete_IR_File = FALSE;	/* Delete SGIR file when done */


/* ====================================================================
 *
 * Cleanup_Files
 *
 * Close all per-source	files involved in a compilation	and prepare the
 * global variables for	the next source.  This routine is externalized
 * for signal cleanup; the report parameter allows suppressing of error
 * reporting during such cleanup.
 *
 * ====================================================================
 */

void
Cleanup_Files (	BOOL report, BOOL delete_dotofile )
{
  /* No	current	line number for	errors:	*/
  Set_Error_Line (ERROR_LINE_UNKNOWN);

  /* Close source file:	*/
  if ( Src_File	!= NULL	&& Src_File != stdin &&	fclose (Src_File) ) {
    if ( report	) ErrMsg ( EC_Src_Close, Src_File_Name,	errno );
  }
  Src_File = NULL;

  /* Close and delete SGIR file: */
  if ( IR_File != NULL && fclose (IR_File) ) {
    if ( report	) ErrMsg ( EC_IR_Close,	IR_File_Name, errno );
  }
  IR_File = NULL;
  if ( Delete_IR_File && unlink	(IR_File_Name) ) {
    if ( report	) ErrMsg ( EC_IR_Delete, IR_File_Name, errno );
  }

  /* Close listing file: */
  if ( Lst_File	!= NULL	&& Lst_File != stdout && fclose	(Lst_File) ) {
    if ( report	) ErrMsg ( EC_Lst_Close, Lst_File_Name,	errno );
  }
  Lst_File = NULL;

  /* Close trace file: */
  Set_Trace_File ( NULL	);

  /* Disable timing file: */
  Tim_File = NULL;

  /* Finally close error file: */
  Set_Error_File ( NULL	);
  Set_Error_Source ( NULL );
}

/* ====================================================================
 *
 * Terminate
 *
 * Do any necessary cleanup and	terminate the program with the given
 * status.
 *
 * ====================================================================
 */

void
Terminate ( INT status )
{
  /* Close and delete files as necessary: */
  Cleanup_Files	( FALSE, FALSE);

  exit (status);
}

#ifdef TARG_ST
/* ====================================================================
 *
 * WFE_Prepare_Gcc_Options
 *
 * Filter out open64 options and return options to be passed
 * to gnu init.
 * ===================================================================
 */
void WFE_Prepare_Gcc_Options(int argc, char **argv, int *gnu_argc, char ***gnu_argv)
{
  // [HK] malloc is poisoned, use xmalloc instead
  char **new_argv = (char **)xmalloc(sizeof(char *)*(argc+1));
  int new_argc = 1;

  int i;
  new_argv[0] = argv[0];
  for (i = 1; i < argc; i++) {
    if (strncmp(argv[i], "-OPT:", 4) == 0 ||
	strncmp(argv[i], "-TARG:", 5) == 0 ||
	strncmp(argv[i], "-TENV:", 5) == 0) {
#ifdef TARG_ST
      // [CL] don't forget to update target description
      // before calling Configure()
      if (strncmp(argv[i]+1, "TARG:", 5) == 0) {
	Process_Command_Line_Group (argv[i]+1, Common_Option_Groups);
      }
#ifdef TARG_ST
      if (strncmp(argv[i]+1, "TENV:", 5) == 0) {
	// [TB]: need to know if an extension is present very early
	if (strncmp(argv[i]+6, ":extension", strlen("extension")) == 0) {
	  Process_Command_Line_Group (argv[i]+1, Common_Option_Groups);
	}
      }
#endif

#endif
      continue;
    } else {
      new_argv[new_argc] = argv[i];
      new_argc++;
    }
  }
  new_argv[new_argc] = NULL;

  *gnu_argc = new_argc;
  *gnu_argv = new_argv;
}

#endif

/*
/* ====================================================================
 *
 * Prepare_Source
 *
 * Process the next source argument and	associated file	control	flags
 * from	the command line.  Pre-process the source file unless
 * suppressed, and initialize output files as required.	 Return	TRUE
 * iff we have a successfully pre-processed source file	left to
 * compile.
 *
 * ====================================================================
 */

static BOOL
Prepare_Source ( void )
{
  INT16	i;
  char *cp;
  char *fname;
  INT16 len;
  BOOL  dashdash_flag = FALSE;

  /* Initialize error handler: */
  Init_Error_Handler ( 100 );
  Set_Error_Line ( ERROR_LINE_UNKNOWN );
  Set_Error_File ( NULL );
  Set_Error_Phase ( "Front End Driver" );

  /* Clear file names: */
  Src_File_Name = NULL;	/* Source file */
  IR_File_Name = NULL;	/* SGIR file */
  Irb_File_Name = NULL;	/* ACIR file */
  Err_File_Name = Dash;	/* Error file */
  Lst_File_Name = NULL;	/* Listing file */
  Trc_File_Name = NULL;	/* Trace file */
  DSTdump_File_Name = NULL; /* DST dump */

  Delete_IR_File = FALSE;
  
#ifdef TARG_ST
  // [SC] The scan of Argv to find the source file name (in the non-ST code
  // below) is flawed because it does not take into account that
  // options may take option arguments in the next Argv element.
  // This occurs often: e.g. -isystem has an argument in the next Argv
  // element, the code below interprets this argument as a source file
  // name.
  // For gfec/gfecc, we can take advantage of two observations:
  // 1. Prepare_Source is called only once per invocation of gfec/gfecc,
  // so there is no need to handle multiple source file names.
  // 2. The driver ensures that the source file name will
  // be the last argument, i.e. it will be in Argv[Argc-1], so we do
  // not need to scan Argv to find it.
  // So instead, do a simple scan through the options looking for
  // the ones we need to process, then fall into the original SGI code
  // for the last argument (which must be the source file).
  //
  // (Another observation: if we are in gfec/gfecc preprocess-only mode, then
  // Prepare_Source is never called.)
  while ( ++Source_Arg < Argc) {
    i = Source_Arg;
    if (Argv[i] != NULL
	&& (strncmp (Argv[i], "-OPT:", 5 ) == 0
	    || strncmp (Argv[i], "-TARG:", 6) == 0
	    || strncmp (Argv[i], "-TENV:", 6) == 0)) {
      Process_Command_Line_Group (Argv[i] + 1, Common_Option_Groups);
    }
    if (Argv[i] != NULL) {
      // [SC] Some extra assertions to detect command lines on
      // which the source file has not been placed last.

      // Output redirection not allowed, since the driver places
      // it after the source file name.
      FmtAssert (*Argv[i] != '>', ("Malformed source file argument (%c)",
				   *Argv[i]));
      // The last command line argument before the source file
      // must not be -o.
      if (i == (Argc - 1))
	FmtAssert (strcmp (Argv[i], "-o") != 0,
		  ("Malformed command line, source file name preceded by -o"));
    }
  }
  FmtAssert (Argv[i] != 0, ("Missing source file argument"));
  // [SC] I make the following assertion to check that the driver has
  // placed the source file name last, but am uneasy about it, since
  // it prevents source file names that begin with '-'.
  FmtAssert (*Argv[i] != '-' || *(Argv[i]+1) == 0,
	     ("Malformed source file argument"));
  {
    {
#else
  /* Check the command line flags for -f? and source file names: */
  while ( ++Source_Arg <= Argc ) {
    i = Source_Arg;

    /* Null argument => end of list: */
    if ( Argv[i] == NULL ) return FALSE;

    if ( !dashdash_flag && (*(Argv[i]) == '-' ) && *(Argv[i]+1) != '\0') {
      cp = Argv[i]+1;	/* Pointer to next flag character */

      /* -oname or -o name are passed to the linker: */
      if ( *cp == 'o' ) {
	++cp;
	if ( *cp == 0 ) {
	  /* Link file name is next command line argument: */
	  ++Source_Arg;
	}
	continue;
      }

      /* process as command-line option group */
      if (strncmp(cp, "OPT:", 4) == 0)
	{ 
	  Process_Command_Line_Group (cp, Common_Option_Groups);
	  continue;
	}

#ifdef TARG_ST
      /* process -O options */
      if (*cp == 'O') {
	++cp;
	OPT_Space = !strcmp( cp, "s" );
	Opt_Level = Get_Numeric_Flag (&cp, 0, MAX_OPT_LEVEL,
				      DEF_O_LEVEL, Argv[i]); 
      }
#endif
    } 
    else {
#endif
      Src_Count++;
      dashdash_flag = FALSE;

      /* Copy the given source name: */
      len = strlen ( Argv[i] );
  // [HK] malloc is poisoned, use xmalloc instead
      Src_File_Name = (char *) xmalloc (len+5);
      strcpy ( Src_File_Name, Argv[i] );

      /* We've got a source file name -- open other files.
       * We want them to be created in the current directory, so we
       * strip off the filename only from Src_File_Name for use:
       */
      fname = Last_Pathname_Component ( Src_File_Name );

      /* Error file first to get error reports: */
      if ( Err_File_Name == NULL ) {
	/* Replace source file extension to get error file: */
	Err_File_Name = New_Extension
			    ( fname, ERR_FILE_EXTENSION	);
      } else if ( *Err_File_Name == '-' ) {
	/* Disable separate error file: */
	Err_File_Name = NULL;
      }
      Set_Error_File ( Err_File_Name );

      /* Trace file next: */
      if ( Trc_File_Name == NULL ) {
	if ( Tracing_Enabled ) {
	  /* Replace source file extension to get trace file: */
	  Trc_File_Name = New_Extension
			    ( fname, TRC_FILE_EXTENSION	);
	}
      } else if ( *Trc_File_Name == '-' ) {
	/* Leave trace file on stdout: */
	Trc_File_Name = NULL;
      }
      Set_Trace_File ( Trc_File_Name );
      if ( Get_Trace (TKIND_INFO, TINFO_TIME) ) Tim_File = TFile;

      /* We're ready to pre-process: */
      IR_File_Name = Src_File_Name;

      /* Open the IR file for compilation: */
      if ( Irb_File_Name == NULL ) {
	if (asm_file_name == NULL) {
		/* Replace source file extension to get listing file: */
		Irb_File_Name = New_Extension (	fname, IRB_FILE_EXTENSION );
	}
	else {
		Irb_File_Name = asm_file_name;
	}
      }

	if ( (Irb_File = fopen ( Irb_File_Name, "w" )) == NULL ) {
	  ErrMsg ( EC_IR_Open, IR_File_Name, errno );
	  Cleanup_Files ( TRUE, FALSE );	/* close opened files */
	  return Prepare_Source ();
	} else {
	  if ( Get_Trace ( TP_MISC, 1) ) {
	    fprintf ( TFile, 
	      "\n%sControl Values: Open_Dot_B_File\n%s\n", DBar, DBar );
	    Print_Controls ( TFile, "", TRUE );
	  }
	}

      /* Configure internal options for this source file */
      Configure_Source ( Src_File_Name );

      return TRUE;
    }
  }

  return FALSE;
}


#ifdef TARG_ST
void
WFE_Init_Errors ()
  {
  Set_Error_Tables ( Phases, host_errlist );
  }
      
#endif


void
WFE_Init (INT argc, char **argv)
{
#ifndef TARG_ST
  // (cbr) done in toplev.c before WFE_Prepare_Gcc_Option
  Set_Error_Tables ( Phases, host_errlist );
#endif
  MEM_Initialize();
#ifndef TARG_ST
  // (cbr) for gcc errors compatibility use the one registered in gcc.
  Handle_Signals();
#endif

  /* Perform preliminary command line processing: */
  Set_Error_Line ( ERROR_LINE_UNKNOWN );
  Set_Error_Phase ( "Front End Driver" );
  Preconfigure ();
#ifdef TARG_MIPS
  ABI_Name = (char *) mips_abi_string;
#endif
#ifdef TARG_IA64
  ABI_Name = "i64";
#endif
#ifdef TARG_IA32
  ABI_Name = "ia32";
#endif
  Init_Controls_Tbl();
  Argc = argc;
  Argv = argv;
  Target_Byte_Sex = TARGET_BIG_ENDIAN ? BIG_ENDIAN : LITTLE_ENDIAN;
#ifndef TARG_ST
  // [TB] Moved to toplevel.c
  Configure ();
#endif
//Initialize_C_Int_Model();
  IR_reader_init();
  Initialize_Symbol_Tables (TRUE);
  WFE_Stmt_Stack_Init ();
  WFE_Stmt_Init ();
  WFE_Expr_Init ();
  WHIRL_Mldid_Mstid_On = TRUE;
  WN_Simp_Fold_LDA = TRUE;  // fold (LDA offset) + const to LDA (offset+const)
			    // since the static initialization code relies on it
  WHIRL_Keep_Cvt_On = TRUE; // so simplifier won't I8I4CVT

#ifndef TARG_ST
  /* [CG] optimize = 0 is now forced in gnu fe (toplevel.c). */
  Opt_Level = optimize;
  optimize = 0;
#endif

  // This is not right: we should match what gnu does
  // and this is only an approximation.
  Debug_Level = (debug_info_level >= DINFO_LEVEL_NORMAL)? 2:0;
} /* WFE_Init */

void
WFE_File_Init (INT argc, char **argv)
{
  /* Process each source file: */
  Prepare_Source();
  MEM_POOL_Push (&MEM_src_pool);

#ifdef TARG_ST
  /* [TTh] Mark disabled Gcc registers based on command line options
   * processed within Prepare_Source()
   */
  GCCTARG_Mark_Disabled_Gcc_Reg();
#endif

  Restore_Cmd_Line_Ctrls();

  /* If the user forgot to specify sources, complain: */
  if ( Src_Count == 0 ) {
    ErrMsg ( EC_No_Sources );
  }

  Open_Output_Info ( Irb_File_Name );
  DST_build(argc, argv);	// do initial setup of dst
}

void
WFE_File_Finish (void)
{
    Verify_SYMTAB (GLOBAL_SYMTAB);
    Write_Global_Info (PU_Tree_Root);
    Close_Output_Info ();
    IR_reader_finish ();
    MEM_POOL_Pop (&MEM_src_pool);
}

void
WFE_Finish ()
{
  WFE_Stmt_Stack_Free ();
}

void
WFE_Check_Errors (int *error_count, int *warning_count, BOOL *need_inliner)
{
  
  /* If we've seen errors, note them and terminate: */
  Get_Error_Count ( error_count, warning_count);
  *need_inliner = wfe_invoke_inliner;
}

#define ENLARGE(x) (x + (x >> 1))
#define WN_STMT_STACK_SIZE 32

static WN_STMT *wn_stmt_stack;
static WN_STMT *wn_stmt_sp;
static WN_STMT *wn_stmt_stack_last;
static INT      wn_stmt_stack_size;

char * WFE_Stmt_Kind_Name [wfe_stmk_last+1] = {
  "'unknown'",
  "'function entry'",
  "'function pragma'",
  "'function body'",
  "'region pragmas'",
  "'scope'",
  "'if condition'",
  "'if then clause'",
  "'if else clause'",
  "'while condition'",
  "'while body'",
  "'dowhile condition'",
  "'dowhile body'",
  "'for condition'",
  "'for body'",
  "'switch'",
  "'comma'",
  "'rcomma'",
  "'last'"
};

WN_STMT *
WFE_Get_Stmt()
{
return wn_stmt_sp;
}

static void
WFE_Stmt_Stack_Init (void)
{
  wn_stmt_stack_size = WN_STMT_STACK_SIZE;
  // [HK] malloc is poisoned, use xmalloc instead
  wn_stmt_stack      = (WN_STMT *) xmalloc (sizeof (WN_STMT) *
                                           wn_stmt_stack_size );
  wn_stmt_sp         = wn_stmt_stack - 1;
  wn_stmt_stack_last = wn_stmt_stack + wn_stmt_stack_size - 1;
} /* WFE_Stmt_Stack_Init */

static void
WFE_Stmt_Stack_Free (void)
{
  free (wn_stmt_stack);
  wn_stmt_stack = NULL;
} /* WFE_Stmt_stack_free */

void
WFE_Stmt_Push (WN* wn, WFE_STMT_KIND kind, SRCPOS srcpos)
{
  INT new_stack_size;

  if (wn_stmt_sp == wn_stmt_stack_last) {
    new_stack_size = ENLARGE(wn_stmt_stack_size);
    wn_stmt_stack =
  // [HK] realloc is poisoned, use xrealloc instead
      (WN_STMT *) xrealloc (wn_stmt_stack, new_stack_size * sizeof (WN_STMT));
    wn_stmt_sp = wn_stmt_stack + wn_stmt_stack_size - 1;
    wn_stmt_stack_size = new_stack_size;
    wn_stmt_stack_last = wn_stmt_stack + wn_stmt_stack_size - 1;
  }
  ++wn_stmt_sp;
  wn_stmt_sp->wn   = wn;
  wn_stmt_sp->kind = kind;

  if (srcpos)
    WN_Set_Linenum ( wn, srcpos );
} /* WFE_Stmt_Push */

WN*
WFE_Stmt_Top (void)
{
  FmtAssert (wn_stmt_sp >= wn_stmt_stack,
             ("no more entries on stack in function WFE_Stmt_Top"));

  return (wn_stmt_sp->wn);
} /* WFE_Stmt_Top */

#ifdef TARG_ST
// A region has started before a call stmt, and it seems its difficult 
// to close the region cleanly. For the time being, we have this function 
// that closes the call region.
// Return 1 if we did close a region.
bool
Check_For_Call_Region ()
{
  //  if (key_exceptions) {
  if (flag_exceptions) {
    if (wn_stmt_sp->kind == wfe_stmk_call_region_body) {
      Setup_EH_Region();
      return TRUE;
    }
  }
  return FALSE;
}
#endif

void
WFE_Stmt_Append (WN* wn, SRCPOS srcpos)
{
  WN * body;
  WN * last;

  if (srcpos) {
    WN_Set_Linenum ( wn, srcpos );
    if (WN_operator(wn) == OPR_BLOCK && WN_first(wn) != NULL)
    	WN_Set_Linenum ( WN_first(wn), srcpos );
  }

  body = WFE_Stmt_Top ();

  if (body) {

    last = WN_last(body);
    WN_INSERT_BlockAfter (body, last, wn);
  }

#ifdef TARG_ST
  Check_For_Call_Region();
#endif
} /* WFE_Stmt_Append */


WN*
WFE_Stmt_Last (void)
{
  WN * body;

  body = WFE_Stmt_Top ();
  return (WN_last(body));
} /* WFE_Stmt_Last */


WN *
WFE_Stmt_Delete ()
{
  WN * body;
  WN * last;
  WN * prev;

  body = WFE_Stmt_Top ();
  last = WN_last(body);
  prev = WN_prev(last);
  if (prev)
    WN_next(prev)  = NULL;
  else
    WN_first(body) = NULL;
  WN_last(body) = prev;
  WN_prev(last) = NULL;

  return last;
} /* WFE_Stmt_Delete */

void
WFE_Stmt_Move_To_End (WN *first_wn, WN *last_wn)
{
  WN * body;
  WN * last;
  WN * prev;
  WN * next;

  body = WFE_Stmt_Top ();
  last = WN_last(body);

  if (last == last_wn) return;

  prev = WN_prev(first_wn);
  next = WN_next(last_wn);

  if (prev)
    WN_next(prev)  = next;
  else
    WN_first(body) = next;

  WN_prev(next) = prev;

  WN_next(last) = first_wn;
  WN_prev(first_wn) = last;
  WN_next(last_wn) = NULL;
  WN_last(body) = last_wn;

} /* WFE_Stmt_Move_To_End */


WN*
WFE_Stmt_Pop (WFE_STMT_KIND kind)
{
  WN * wn;

  FmtAssert (wn_stmt_sp >= wn_stmt_stack,
             ("no more entries on stack in function WFE_Stmt_Pop"));

#ifdef KEY
  // another hack.
  WN * to_be_pushed = 0;
  //  if (key_exceptions && wn_stmt_sp->kind != kind)
  if (flag_exceptions && wn_stmt_sp->kind != kind)
  {
    if (!Did_Not_Terminate_Region)
    {
  	FmtAssert (wn_stmt_sp->kind == wfe_stmk_call_region_body,
             ("mismatch in statements: expected %s, got %s\n",
              WFE_Stmt_Kind_Name [kind],
              WFE_Stmt_Kind_Name [wn_stmt_sp->kind]));

	to_be_pushed = WFE_Stmt_Pop (wfe_stmk_call_region_body);
    }
  }
#endif

  FmtAssert (wn_stmt_sp->kind == kind,
             ("mismatch in statements: expected %s, got %s\n",
              WFE_Stmt_Kind_Name [kind],
              WFE_Stmt_Kind_Name [wn_stmt_sp->kind]));

  wn = wn_stmt_sp->wn;
  wn_stmt_sp--;

#ifdef TARG_ST
  if (to_be_pushed) 
  	WFE_Stmt_Push (to_be_pushed, wfe_stmk_call_region_body, Get_Srcpos()); 
#endif

  return (wn);
} /* WFE_Stmt_Pop */

/*
void process_diag_override_option(an_option_kind kind,
                                  char          *opt_arg)
{
}
*/
