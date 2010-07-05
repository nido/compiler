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
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include "W_wait.h"
#include "sys_process.h"
#include <fcntl.h>
#include "W_unistd.h"
#include "W_errno.h"
#include "W_signal.h"
#include <malloc.h>
#include <sys/param.h>
#include <time.h>
#include "W_times.h"		// For times()
#include "W_limits.h"
#include "W_alloca.h"
#include <cmplrs/rcodes.h>
#include "run.h"
#include "string_utils.h"
#include "errors.h"
#include "file_names.h"
#include "phases.h"
#include "opt_actions.h"
#include "file_utils.h"
#include "libiberty/libiberty.h"
#ifdef TARG_ST
#include "option_names.h" // CL: for olevel
#include "SYS.h"	// For SYS_setenv()
#endif

/* Debug only. Uncomment to use clock() in addition for timing
   (for checking that clock() returns the same elapsed time as times(). */
#if 0
#define USE_CLOCK
#endif


boolean show_flag = FALSE;
boolean execute_flag = TRUE;
#ifdef TARG_ST
boolean show_cmd_line = FALSE;
#endif
boolean time_flag = FALSE;
boolean prelink_flag = TRUE;
boolean quiet_flag = TRUE;
int memory_flag = 0;
boolean run_m4 = FALSE;
static boolean ran_twice = FALSE;
static int f90_fe_status = RC_OKAY;
static string f90_fe_name = NULL; 

static void init_time (void);
static void print_time (string phase);

#ifdef sgi
static prmap_sgi_t mapbuf[100];
static prmap_sgi_arg_t mapbuf_desc = { (char *) mapbuf, sizeof (mapbuf)} ;
#define DATA_ADDRESS ((char *)0x10000000)
#define TEXT_ADDRESS ((char *)0x400000)
#endif

#ifndef sgi
#define LOGFILE "/var/log/messages"
#else
#define LOGFILE "/usr/adm/SYSLOG"
#endif


#ifdef COMPILER_LICENSING

#include <lmsgi.h>

/*
#include "lmclient.h"
#include "lm_attr.h"
#include "lm_code.h"
*/

#include <invent.h>
#include <sys/sysmp.h>


LM_CODE(code, ENCRYPTION_CODE_1, ENCRYPTION_CODE_2, VENDOR_KEY1,
                     VENDOR_KEY2, VENDOR_KEY3, VENDOR_KEY4, VENDOR_KEY5);

#define LINGER_PERIOD  10
#define ALL (-1)
#define ALL_8BIT (ALL & 0xFF)

/* extern  char* license_errstr(); */

boolean licensed_component = FALSE;
char feature_name[10];
string compiler_licensing;


void get_license( string feature_name, boolean soft, boolean tiers )

{

 boolean err = FALSE;
 string hard_licensing;
 string tier_licensing;
 int numCPU;
 int archCPU;
 char *p;
 inventory_t *inv;
 int systemNumCPU = 0;
 int systemArchCPU;

 hard_licensing = getenv("HARD_LICENSING"); 
 if (hard_licensing != NULL) {
     soft = FALSE;
 }

 tier_licensing = getenv("TIER_LICENSING"); 
 if (tier_licensing != NULL) {
     tiers = FALSE;
 }

 if (license_init(&code,"sgifd",B_TRUE) < 0) {
    err = TRUE;
 }

 /* Set Up Soft Licensing */
 license_set_attr(LMSGI_NO_SUCH_FEATURE, NULL);

 /* Set the linger period to 10 seconds */
 license_set_attr(LM_A_LINGER, (LM_A_VAL_TYPE) LINGER_PERIOD);


 /*
    Attempt to check out a license
    leave the first argument as is.  you only need to fill in
    the feature name (program name) and version number.
 */

 if (license_chk_out(&code,     /* leave as is */
     feature_name,              /* replace with your feature name */
     "7.000"                    /* replace with your version number */
 )) {
      err = TRUE;
      fprintf(stderr,"%s\n", license_errstr());
      fprintf(stderr,"\n");
 }

 if ((tiers) && (err == FALSE)) {
     CONFIG *conf = lc_auth_data(get_job(), feature_name);
     if (conf == NULL) {
         err = TRUE;
         fprintf(stderr,"%s\n", license_errstr());
         fprintf(stderr,"\n");
     } else {
         if ((conf->users == 0) && (conf->lc_vendor_def != NULL) &&
             (strcmp(conf->lc_vendor_def, "") != 0)) {

           /* Nodelocked license with non empty vendor string */
           /* sscanf(conf->lc_vendor_def, "%d:%d", &numCPU, &archCPU); */

           p = conf->lc_vendor_def;
           while ( ((*p) != '\0')  && (((*p) < '0') || ((*p) > '9')) )
              p++;
           numCPU = atoi(p);

           while ((inv = getinvent()) != NULL) {
               if ((inv->inv_class == INV_PROCESSOR) &&
                   (inv->inv_type == INV_CPUBOARD)) {
                   if ((inv->inv_unit & ALL_8BIT) == ALL_8BIT) {
                       systemNumCPU = sysmp(MP_NPROCS);
                       systemArchCPU = inv->inv_state;
                   } else {
                       systemNumCPU = inv->inv_unit;
                       systemArchCPU = inv->inv_state;
                   }
               }
           }

           if ((numCPU >= systemNumCPU)) {
               err = FALSE;
           } else if ( numCPU == 0 ) {  /* A zero in the vendor string implies         */
               err = FALSE;             /* the largest system configuration tier       */
           } else {                     /* In other words, it will work on any system. */
               err = TRUE;
               fprintf(stderr, " This product's license is for %d CPU's; Your system has %d CPU's\n", numCPU, systemNumCPU ); 
               fprintf(stderr,"\n");
           }
        }
     }
 }

 if (err) {
   fprintf(stderr," This product (%s) requires a license password. \n", feature_name);
   fprintf(stderr," For license installation and trouble shooting \n");
   fprintf(stderr," information visit the web page: \n");
   fprintf(stderr,"\n");
   fprintf(stderr,"         http://www.sgi.com/Support/Licensing/install_docs.html \n");
   fprintf(stderr,"\n");
   fprintf(stderr," To obtain a Permanent license (proof of purchase\n");
   fprintf(stderr," required) or an Evaluation license please\n");
   fprintf(stderr," visit our license request web page: \n");
   fprintf(stderr,"\n");
#if 0
   fprintf(stderr,"    Internal SGI Users refer to:\n"); 
   fprintf(stderr,"         http://wwclass.csd.sgi.com/swl-internal/beta_lice.html\n");
   fprintf(stderr,"    Otherwise:\n");
#endif
   fprintf(stderr,"         http://www.sgi.com/Products/license.html \n");
   fprintf(stderr,"\n");
   fprintf(stderr,"         or send a blank email message to: \n");
   fprintf(stderr,"\n");
   fprintf(stderr,"         license@sgi.com \n");
   fprintf(stderr,"\n");
   fprintf(stderr," In North America, Silicon Graphics' customers may request \n");
   fprintf(stderr," Permanent licenses by sending a facsimile to: \n");
   fprintf(stderr,"\n");
   fprintf(stderr,"         (650) 390-0537 \n");
   fprintf(stderr,"\n");
   fprintf(stderr,"         or by calling our technical support hotline \n");
   fprintf(stderr,"\n");
   fprintf(stderr,"         1-800-800-4SGI \n");
   fprintf(stderr,"\n");
   fprintf(stderr," If you are Outside of North America or you are not a Silicon \n");
   fprintf(stderr," Graphics support customer then contact your local support provider. \n");
   fprintf(stderr,"\n");
#if 0
   fprintf(stderr," Note: Permanent Licenses require verification of entitlement \n");
   fprintf(stderr," (i.e., Proof-of-Purchase). \n");
   fprintf(stderr,"  \n");
#endif
   
   if (!soft)
      exit(-1);
 }
}

#endif

#ifdef TARG_ST
static int
has_space(const char *str) 
{
  int quote= 0;
  while(*str != '\0') {
    if (*str == '\\') {
      str++;
      if (*str != '\0') str++;
      continue;
    }
    if (*str == '"' || *str == '\'') {
      if (quote == 0) quote = *str;
      else if (quote == *str) quote = 0;
    } else if (isspace(*str) && quote == 0) 
      return 1;
    str++;
  }
  return 0;
}

extern int
run_redirected_program (string program, char **argv, string infile, string outfile, string errfile)
{
  int status = -1;
  int dry_run = 0;
  int ver_run = 0;
  int i;
  int fdin, fdout, fderr, org_stderr, org_stdout, org_stdin;
  int pid, waitstatus, termsig;
  char *errmsg_fmt, *errmsg_arg;

  pid = sys_pexecute (program, (const char * const *)argv, 
		      (const char **)&errmsg_fmt, (const char **)&errmsg_arg, 
		      SYS_P_STOP_ON_EXIT, infile, outfile, errfile);
  
  
  /* Check return of pexecute. */
  if (pid < 0) {
    error(errmsg_fmt, errmsg_arg);
    cleanup ();
    exit (RC_SYSTEM_ERROR);
    /* NOTREACHED */
  }
  
  pid = sys_pwait(pid, &waitstatus, 0);
  if (pid < 0) {
    error("unexpected error waiting for %s", program);
    cleanup();
    exit(RC_SYSTEM_ERROR);
    /* NOTREACHED */
  }
  if (time_flag) {
    fprintf(stderr, "%s phase time: ", drop_path(program));
    sys_print_ptime(pid, stderr);
  }
  if (memory_flag) {
    fprintf(stderr, "%s phase memory: ", drop_path(program));
    sys_print_pmem(pid, stderr);
  }
  
  pid = sys_pwait(pid, &waitstatus, SYS_P_TERM);
  if (pid < 0) {
    error("unexpected error waiting for %s", program);
    cleanup();
    exit(RC_SYSTEM_ERROR);
    /* NOTREACHED */
  }
#ifdef WIFSTOPPED
  if (WIFSTOPPED(waitstatus)) {
    termsig = WSTOPSIG(waitstatus);
    error("STOPPED signal %d received by %s", program, termsig);
    cleanup();
    exit(RC_SYSTEM_ERROR);
    /* NOTREACHED */
  } 
#endif
  if (WIFEXITED(waitstatus)) {
    status = WEXITSTATUS(waitstatus);
    /* Will return at end of function with status set. */
  } else if(WIFSIGNALED(waitstatus)){
    termsig = WTERMSIG(waitstatus);
    error("%s died due to signal %d", program, termsig);
#ifdef WCOREFLAG
    if(waitstatus & WCOREFLAG) {
      error("core dumped");
    }
#endif
#ifdef SIGKILL
    if (termsig == SIGKILL) {
      error("Probably caused by running out of swap space -- check %s", LOGFILE);
    }
#endif
    cleanup();
    exit(RC_SYSTEM_ERROR);
    /* NOTREACHED */
  } else {
    /* cannot happen, I think! */
    internal_error("unexpected status returned by %s", program);
    cleanup();
    exit(RC_SYSTEM_ERROR);
    /* NOTREACHED */
  }
  return status;
}

#endif

/* exec another program, putting result in output.
 * This is simple version of full run_phase. */
extern void
run_simple_program (string name, char **argv, string output)
{
#ifdef TARG_ST
  int status;
  status = run_redirected_program (name, argv, NULL, output, NULL);
  if (status != RC_OKAY) {
    /* internal error */
    /* most internal errors use exit code of 1 */
    internal_error("%s returned non-zero status %d", name, status);
  } 
#else
  /* fork a process */
  forkpid = fork();
  if (forkpid == -1) {
    error("no more processes");
    cleanup ();
    exit (RC_SYSTEM_ERROR);
    /* NOTREACHED */
  }
  
  if (forkpid == 0) {
    /* child */
    if ((fdout = creat (output, 0666)) == -1) {
      error ("cannot create output file %s", output);
      cleanup ();
      exit (RC_SYSTEM_ERROR);
      /* NOTREACHED */
    }
    dup2 (fdout, fileno(stdout));
    
    execv(name, argv);
    error("cannot exec %s", name);
    cleanup ();
    exit (RC_SYSTEM_ERROR);
    /* NOTREACHED */
  } else {
    /* parent */
    int procid;	/* id of the /proc file */
    while ((waitpid = wait (&waitstatus)) != forkpid) {
      if (waitpid == -1) {
	error("bad return from wait");
	cleanup();
	exit(RC_SYSTEM_ERROR);
	/* NOTREACHED */
      }
    }
    if (WIFSTOPPED(waitstatus)) {
      termsig = WSTOPSIG(waitstatus);
      error("STOPPED signal received from %s", name);
      cleanup();
      exit(RC_SYSTEM_ERROR);
      /* NOTREACHED */
    } else if (WIFEXITED(waitstatus)) {
      int status = WEXITSTATUS(waitstatus);
      if (status != RC_OKAY) {
	/* internal error */
	/* most internal errors use exit code of 1 */
	internal_error("%s returned non-zero status %d",
		       name, status);
      } 
      return;
    } else if(WIFSIGNALED(waitstatus)){
      termsig = WTERMSIG(waitstatus);
      error("%s died due to signal %d", name, termsig);
#ifdef WCOREFLAG
      if(waitstatus & WCOREFLAG) {
	error("core dumped");
      }
#endif
      if (termsig == SIGKILL) {
	error("Probably caused by running out of swap space -- check %s", LOGFILE);
      }
      cleanup();
      exit(RC_SYSTEM_ERROR);
    } else {
      /* cannot happen, I think! */
      internal_error("driver exec'ing is confused");
      return;
    }
  }
#endif
}

extern void
run_phase (phases_t phase, string name, string_list_t *args)
{
	char **argv;
	int argc;
	string_item_t *p;
	string output = NULL;
	string input = NULL;
	boolean save_stderr = FALSE;
	int fdin, fdout;
	int forkpid;
	int waitpid;
	int waitstatus;
	int status;
	int termsig;
	int	num_maps;
	string rld_path;
	struct stat stat_buf;
	char *env_name, *new_ld_library_path, *old_ld_library_path;
	const boolean uses_message_system = 
			(phase == P_f90_fe || phase == P_f90_cpp ||
			 phase == P_cppf90_fe);

#ifdef TARG_ST
	if (show_flag || show_cmd_line) {
#else
	if (show_flag) {
#endif
		/* echo the command */
		fprintf(stderr, "%s ", name);
		print_string_list(stderr, args);
	}
	if (!execute_flag) return;

	/* [CG] Now done in reun_redirected_program. *
	   /* if (time_flag) init_time();*/

	/* copy arg_list to argv format that exec wants */
	argc = 1;
	for (p = args->head; p != NULL; p=p->next) {
		argc++;
	}
	argv = (char **) malloc((argc+1)*sizeof(char*));
	argv[0] = name;
	for (argc = 1, p = args->head; p != NULL; argc++, p=p->next) {
		/* don't put redirection in arg list */
		if (same_string(p->name, "<")) {
			/* has input file */
			input = p->next->name;
			break;
		} else if (same_string(p->name, ">")) {
			/* has output file */
			output = p->next->name;
			break;
		} else if (same_string(p->name, ">&")) {
			/* has error output file */
			output = p->next->name;
			save_stderr = TRUE;
			break;
		}
		argv[argc] = p->name;
	}
	argv[argc] = NULL;


#ifdef COMPILER_LICENSING
/***********************/
/* PUT Licensing Here  */
/***********************/
        compiler_licensing = getenv("COMPILER_LICENSING"); 
        if (compiler_licensing == NULL) {
           if  (strstr(name, "fecc")) {
                licensed_component = TRUE;
                /* printf("COMPILER LICENSING ON\n"); */
                feature_name[0] = 'c';
                feature_name[1] = 'p';
                feature_name[2] = 'p';
                feature_name[3] = '\0';
          
           } else if (strstr(name, "fec")) {
                licensed_component = TRUE;
                /* printf("COMPILER LICENSING ON\n"); */
                feature_name[0] = 'c';
                feature_name[1] = 'c';
                feature_name[2] = '\0';
      
           } else if (strstr(name, "mfef77")) {
                licensed_component = TRUE;
                feature_name[0] = 'f';
                feature_name[1] = '7';
                feature_name[2] = '7';
                feature_name[3] = '\0';

           } else if (strstr(name, "mfef90")) {
                licensed_component = TRUE;
                feature_name[0] = 'f';
                feature_name[1] = '9';
                feature_name[2] = '0';
                feature_name[3] = '\0';

           } else
                licensed_component = FALSE;
       
           if (licensed_component) {
               get_license(feature_name, TRUE, TRUE); /* soft licensing == TRUE; tier licensing == TRUE */
           }
         }
#endif

#ifdef TARG_ST
	new_ld_library_path = NULL;
	rld_path = get_phase_ld_library_path (phase);
	if (ld_library_path != NULL || rld_path != NULL) {
	  env_name = "LD_LIBRARY_PATH";
	  old_ld_library_path = SYS_getenv(env_name);
	  if (old_ld_library_path != NULL) 
	    old_ld_library_path = SYS_strdup(old_ld_library_path);
	  
	  /* The library path is contructed as:
	     [rld_path][:ld_library_path]
	     Note that ld_library_path (ref phases.c) should contains the
	     the path to lib/cmplrs and the initial user ld library path.
	  */
	  if (rld_path != NULL && *rld_path != '\0')
	    new_ld_library_path = rld_path;
	  if (ld_library_path != NULL && *ld_library_path != '\0') {
	    if (new_ld_library_path != NULL) new_ld_library_path = concat_strings(new_ld_library_path, concat_strings(":", ld_library_path));
	    else new_ld_library_path = ld_library_path;
	  }
	}
	if (new_ld_library_path != NULL) 
	  SYS_setenv(env_name, new_ld_library_path);

	// [CG] Use common interface with run_simple_program
	if (save_stderr) {
	  status = run_redirected_program (name, argv, NULL, NULL, output);
	} else {
	  status = run_redirected_program (name, argv, NULL, output, NULL);
	}
	
	/* Reset old LD_LIBRARY_PATH. */
	if (new_ld_library_path != NULL &&
	    old_ld_library_path != NULL) 
	  SYS_setenv(env_name, old_ld_library_path);
	
	/* [CG] Now done in reun_redirected_program. *
	/*	if (time_flag) print_time(name);*/
	if (status != -1) {
	  extern int inline_t;
	  boolean internal_err = FALSE;
	  boolean user_err = FALSE;
	  boolean exec_err = FALSE;
	  
	  if (phase == P_prof) {
	    /* Make sure the .cfb files were created before
	       changing the STATUS to OKAY */
	    if (prof_file != NULL) {
	      if (!(stat(fb_file, &stat_buf) != 0 && errno == ENOENT))
		status = RC_OKAY;
	    } else {
	      internal_error("No count file was specified for a prof run");
	      perror(program_name);
	    }
	  }
	  
	  if (phase == P_f90_fe && keep_listing) {
	    string cif_file;
	    cif_file = construct_given_name(
					    drop_path(source_file), "T", TRUE);
	    if (!(stat(cif_file, &stat_buf) != 0 && errno == ENOENT))
	      f90_fe_status = status;
	    f90_fe_name = string_copy(name);
	    
	    /* Change the status to OKAY so that we can 
	     * execute the lister on the cif_file; we will
	     * take appropriate action on this status once 
	     * the lister has finished executing. See below.
	     */
	    
	    status = RC_OKAY;
	  }
	  
	  if (phase == P_lister) {
	    if (status == RC_OKAY && f90_fe_status != RC_OKAY) {
	      
	      /* We had encountered an error in the F90_fe phase
	       * but we ignored it so that we could execute the
	       * lister on the cif file; we need to switch the
	       * status to the status we received from F90_fe
	       * and use the name of the F90_fe_phase, so that
	       * we can issue a correct error message.
	       */
	      
	      status = f90_fe_status;
	      name = string_copy(f90_fe_name);
	      
	      /* Reset f90_fe_status to OKAY for any further
	       * compilations on other source files.
	       */
	      
	      f90_fe_status = RC_OKAY;
	    }
	  }
	  
	  switch (status) {
	  case RC_OKAY:
	    if (inline_t == UNDEFINED
#ifdef TARG_ST
		&& (is_matching_phase(get_phase_mask(phase), P_wgen) ||
		    (gnu_major_version == 3
		     && is_matching_phase(get_phase_mask(phase), P_any_fe)) || 
#else
		&& (is_matching_phase(get_phase_mask(phase), P_any_fe) || 
#endif
#if defined (TARG_ST) && (GNU_FRONT_END==33)
		    /* TODO: check inline. */
		    /* (cbr) if !keep fe is called at P_gcpp */
		    (!keep_flag && gnu_major_version == 3
		     && is_matching_phase(get_phase_mask(phase), P_gcpp)))
#endif
		)
	      
	      {
#ifdef TARG_ST
		// CL: always invoke inliner at -O2
		/* (cbr) inline c++ compiler generated
		   functions if fe requested it */
		if (olevel > 1 ||
		    is_matching_phase(get_phase_mask(phase),
				      P_cplus_gfe) ||
		    is_matching_phase(get_phase_mask(phase),
				      P_wgen))
		  inline_t = TRUE;
		else 
#endif
		  inline_t = FALSE;
	      }
	    break;
	  case RC_NEED_INLINER:
	    if (inline_t == UNDEFINED
#ifdef TARG_ST
		&& ((gnu_major_version == 3
		     && is_matching_phase(get_phase_mask(phase), P_any_fe))
		    || is_matching_phase(get_phase_mask(phase), P_wgen)))
#else
		&& is_matching_phase(
				     get_phase_mask(phase), P_any_fe) )
#endif
	      {
		inline_t = TRUE;
	      }
	    /* completed successfully */
	    break;
	    
	  case RC_USER_ERROR:
	  case RC_NORECOVER_USER_ERROR:
	  case RC_SYSTEM_ERROR:
	  case RC_GCC_ERROR:
	    user_err = TRUE;
	    break;

	  case SYS_RC_EXEC_ERROR:
	    /* sys_pexecute failed to exec sub process. */
	    exec_err = TRUE;
	    break;

	  case RC_OVERFLOW_ERROR:
	    if (!ran_twice && phase == P_be) {
	      /* try recompiling with larger limits */
	      ran_twice = TRUE;
	      add_string (args, "-TENV:long_eh_offsets");
	      add_string (args, "-TENV:large_stack");
	      run_phase (phase, name, args);
	      return;
	    }
	    internal_err = TRUE;
	    break;
	  case RC_INTERNAL_ERROR:
#ifdef TARG_ST
	    /* as does not differenciate internal/user error, so we return
	       user error by default. */
#ifdef BCO_ENABLED
	    if (phase = P_any_as || phase == P_ld || phase == P_ldplus || phase == P_ldsimple)
#else
	      if (phase = P_any_as || phase == P_ld || phase == P_ldplus)
#endif
#else
		if (phase == P_ld || phase == P_ldplus)
#endif
		  {
		    /* gcc/ld returns 1 for undefined */
		    user_err = TRUE;
		  }
		else
		  internal_err = TRUE;
	    break;
#ifdef TARG_ST
	  case RC_UNIMPLEMENTED_ERROR:
	    /* (cm) The C++ fe reports this kind of return code when dealing with unsupported exceptions*/
	    if(is_matching_phase(get_phase_mask(phase), P_cplus_gfe)) {
	      user_err = TRUE ;
	    } else {
	      internal_err = TRUE ;
	    }
	    break ;
#endif
	  default:
	    internal_err = TRUE;
	    break;
	  } 
	  if (exec_err) {
	    error("could not execute %s", name);
	  } 
	  else if (internal_err) {
	    internal_error("%s returned non-zero status %d",
			   name, status);
	  }
	  else if (user_err) {
	    /* assume phase will print diagnostics */
	    if (!show_flag || save_stderr) {
	      nomsg_error();
	    } else {
	      error("%s returned non-zero status %d",
		    name, status);
	    }
	  }
	  ran_twice = FALSE;
	}
	return;
#else
/* #ifdef MEMORY_FLAG */
/* 	/\* if we want memory stats, we need to open a pipe as a semaphore *\/ */
/* 	if (memory_flag) */
/* 		{ */
/* 		if (pipe(Pipe) < 0) */
/* 			{ */
/* 			error("pipe failed for -showm"); */
/* 			cleanup (); */
/* 			exit (RC_SYSTEM_ERROR); */
/* 			/\* NOTREACHED *\/ */
/* 			} */
/* 		} */
/* #endif */

/* 	/\* fork a process *\/ */
/* 	forkpid = fork(); */
/* 	if (forkpid == -1) { */
/* 		error("no more processes"); */
/* 		cleanup (); */
/* 		exit (RC_SYSTEM_ERROR); */
/* 		/\* NOTREACHED *\/ */
/* 	} */

/* 	if (forkpid == 0) { */
/* 		/\* child *\/ */
/* 		/\* if we want memory stats, we have to wait for */
/* 		   parent to connect to our /proc *\/ */
/* #ifdef MEMORY_FLAG */
/* 		if (memory_flag) my_psema(); */
/* #endif */

/* 		if (input != NULL) { */
/* 			if ((fdin = open (input, O_RDONLY)) == -1) { */
/* 				error ("cannot open input file %s", input); */
/* 				cleanup (); */
/* 				exit (RC_SYSTEM_ERROR); */
/* 				/\* NOTREACHED *\/ */
/* 			} */
/* 	    		dup2 (fdin, fileno(stdin)); */
/* 		} */
/* 		if (output != NULL) { */
/* 			if ((fdout = creat (output, 0666)) == -1) { */
/* 				error ("cannot create output file %s", output); */
/* 				cleanup (); */
/* 				exit (RC_SYSTEM_ERROR); */
/* 				/\* NOTREACHED *\/ */
/* 			} */
/* 			if (save_stderr) { */
/* 	    			dup2 (fdout, fileno(stderr)); */
/* 			} else { */
/* 	    			dup2 (fdout, fileno(stdout)); */
/* 			} */
/* 		}  */

/* 		rld_path = get_phase_ld_library_path (phase); */

/* 		if (rld_path != 0) { */
/* 		    string env_name = "LD_LIBRARYN32_PATH"; */
/* 		    int len; */
/* 		    string new_env; */
    
/* 		    len = strlen (env_name) + strlen(rld_path) + 2; */
		    
/* 		    if (ld_libraryn32_path) { */
/* 			len += strlen (ld_libraryn32_path) + 1; */
/* 			new_env = alloca (len); */
/* 			sprintf (new_env, "%s=%s:%s", env_name, rld_path, */
/* 				 ld_libraryn32_path);  */
/* 		    } else { */
/* 			new_env = alloca (len); */
/* 			sprintf (new_env, "%s=%s", env_name, rld_path); */
/* 		    } */

/* 		    putenv (new_env); */

/* 		    /\* repeat the same thing in case the component is built */
/* 		       O32 *\/ */

/* 		    env_name = "LD_LIBRARY_PATH"; */
/* 		    len = strlen (env_name) + strlen(rld_path) + 2; */
		    
/* 		    if (ld_library_path) { */
/* 			len += strlen (ld_library_path) + 1; */
/* 			new_env = alloca (len); */
/* 			sprintf (new_env, "%s=%s:%s", env_name, rld_path, */
/* 				 ld_library_path);  */
/* 		    } else { */
/* 			new_env = alloca (len); */
/* 			sprintf (new_env, "%s=%s", env_name, rld_path); */
/* 		    } */

/* 		    putenv (new_env); */

/* 		} */
/* 		if (uses_message_system) { */
/* 		   string toolroot; */
/* 		   string nlspath; */
/* 		   toolroot = getenv("TOOLROOT"); */
/* 		   nlspath = getenv("NLSPATH"); */
/* 		   if (nlspath==NULL) { */
/* 		   	int len; */
/* 		   	string new_env; */
/* 		   	string env_name = "NLSPATH="; */
/* 			string env_file = "/%N.cat"; */
/* #if defined(linux) || defined(sun) || defined(__CYGWIN__) || defined(__MINGW32__) */
/* 			string env_path = get_phase_dir(P_f90_fe); */
/* 			/\* The phase_dir already has the prepended toolroot *\/ */
/* 			toolroot = "\0"; */
/* #else */
/* 			string env_path = "/usr/lib/locale/C/LC_MESSAGES"; */
/* 		      	if (toolroot == NULL) { */
/* 				toolroot = "\0"; */
/* 		      	} */
/* #endif */
/* 		      	/\* add toolroot as prefix to phase dirs *\/ */
/* 		      	len = strlen(env_name) + strlen(toolroot) + strlen(env_path) +  */
/* 			  strlen(env_file) + 1; */
/* 		      	new_env = alloca(len); */
/* 		      	sprintf(new_env,"%s%s%s%s",env_name,toolroot,env_path,env_file); */
/* 		      	putenv(new_env); */
/* 		   } */
/* 		} */

/* 		if (uses_message_system && getenv("ORIG_CMD_NAME") == NULL) { */
/* 		   const string env_name = "ORIG_CMD_NAME="; */
/* 		   const int len = strlen(env_name) + strlen(program_name) + 1; */
/* 		   const string new_env = alloca(len); */

/* 		   strcpy(new_env, env_name); */
/* 		   strcat(new_env, program_name); */
/* 		   putenv(new_env); */
/* 		} */

/* 		if (phase == P_f90_fe) { */
/* 		   string root; */
/* 		   string modulepath; */
/* 		   int len; */
/* 		   string new_env; */
/* 		   string env_name = "FORTRAN_SYSTEM_MODULES="; */
/* 		   string env_val = "/usr/lib/f90modules"; */
/* 		   root = getenv("TOOLROOT"); */
/* 		   if (root != NULL) { */
/* 		      len = strlen(env_val) + strlen(root) +3 + strlen(env_val); */
/* 		      new_env = alloca(len); */
/* 		      sprintf(new_env,"%s/%s:%s",root,env_val,env_val); */
/* 		      env_val = new_env; */
/* 		   } */
/* 		   modulepath = getenv("FORTRAN_SYSTEM_MODULES"); */
/* 		   if (modulepath != NULL) { */
/* 		      /\* Append env_val to FORTRAN_SYSTEM_MODULES *\/ */
/* 		      if (modulepath[strlen(modulepath)-1] == ':') { */
/* 			 /\* Just append env_val *\/ */
/* 			 len = strlen(modulepath) + strlen(env_val) + 1; */
/* 			 new_env = alloca(len); */
/* 			 sprintf(new_env,"%s%s",modulepath,env_val); */
/* 		      } else { */
/* 			 /\* append :env_val *\/ */
/* 			 len = strlen(modulepath) + strlen(env_val) + 2; */
/* 			 new_env = alloca(len); */
/* 			 sprintf(new_env,"%s:%s",modulepath,env_val); */
/* 		      } */
/* 		      env_val = new_env; */
/* 		   } */
		   
/* 		   /\* add root as prefix to phase dirs *\/ */
/* 		   len = strlen(env_name) + strlen(env_val) + 1; */
/* 		   new_env = alloca(len); */
/* 		   sprintf(new_env,"%s%s",env_name,env_val); */
/* 		   putenv(new_env); */
/* 		} */
/* #if defined(linux) || defined(sun) || defined(__CYGWIN__) || defined(__MINGW32__) */
/* 		{ */
/* 		    /\* need to setenv COMPILER_PATH for collect to find ld *\/ */
/* 		    string env_name = "COMPILER_PATH"; */
/* 		    string collect_path = get_phase_dir(P_collect); */
/* 		    int len = strlen (env_name) + strlen(collect_path) + 2; */
/* 		    string new_env = alloca (len); */

/* 		    sprintf (new_env, "%s=%s", env_name, collect_path);  */
/* 		    putenv (new_env); */
/* 		} */
/* #endif */

/* 		execv(name, argv); */
/* 		error("cannot exec %s", name); */
/* 		cleanup (); */
/* 		exit (RC_SYSTEM_ERROR); */
/* 		/\* NOTREACHED *\/ */
/* 	} else { */
/* 		/\* parent *\/ */
/* 		int procid;	/\* id of the /proc file *\/ */
/* #ifdef MEMORY_FLAG */
/* 		/\* if we are interested in memory statistics, we need to */
/* 		   set a stop-on-exit for the child *\/ */
/* 		if (memory_flag && (procid = stop_on_exit(forkpid))) { */
/*    		  /\* now go and get the memory maps *\/ */
/* 		  while (1) { */
/*    			if ((num_maps=ioctl(procid, PIOCMAP_SGI, &mapbuf_desc)) < 0) { */
/* 				perror("PIOCMAP_SGI"); */
/* 				close(procid); */
/* 				memory_flag = 0; */
/* 				break; */
/*    			} */
			
/*    			premptyset(&syscallSet); */
/*    			if (ioctl(procid, PIOCSEXIT, &syscallSet) < 0) { */
/*       				perror("PIOCSEXIT"); */
/* 				close(procid); */
/* 				memory_flag = 0; */
/* 				break; */
/* 				} */
/*    			/\* continue the process *\/ */
/*    			ioctl(procid, PIOCRUN, NULL); */
/*    			close(procid); */
/* 			break; */
/* 		  } /\* while *\/ */
/* 		} else { */
/* 			/\* if we cant set flags on child, dont use -showm *\/ */
/* 			memory_flag = 0; */
/* 		} */
/* #endif */

/* 		while ((waitpid = wait (&waitstatus)) != forkpid) { */
/* 			if (waitpid == -1) { */
/* 				error("bad return from wait"); */
/* 				cleanup(); */
/* 				exit(RC_SYSTEM_ERROR); */
/* 				/\* NOTREACHED *\/ */
/* 			} */
/* 		} */
/* 		if (time_flag) print_time(name); */
/* #ifdef MEMORY_FLAG */
/* 		if (memory_flag) print_mem(name, num_maps); */
/* #endif */

/* 		if (WIFSTOPPED(waitstatus)) { */
/* 			termsig = WSTOPSIG(waitstatus); */
/* 			error("STOPPED signal received from %s", name); */
/* 			cleanup(); */
/* 			exit(RC_SYSTEM_ERROR); */
/* 			/\* NOTREACHED *\/ */
/* 		} else if (WIFEXITED(waitstatus)) { */
/* 		        int status = WEXITSTATUS(waitstatus); */
/* 			extern int inline_t; */
/* 			boolean internal_err = FALSE; */
/* 			boolean user_err = FALSE; */
		
/* 			if (phase == P_prof) { */
/*                            /\* Make sure the .cfb files were created before */
/*                               changing the STATUS to OKAY *\/ */
/*                            if (prof_file != NULL) { */
/*                               if (!(stat(fb_file, &stat_buf) != 0 && errno == ENOENT)) */
/*                                   status = RC_OKAY; */
/*                            } else { */
/* 			      internal_error("No count file was specified for a prof run"); */
/* 			      perror(program_name); */
/*                            } */
/*                         } */

/* 			if (phase == P_f90_fe && keep_listing) { */
/* 			    string cif_file; */
/* 			    cif_file = construct_given_name( */
/* 					  drop_path(source_file), "T", TRUE); */
/*                             if (!(stat(cif_file, &stat_buf) != 0 && errno == ENOENT)) */
/* 			       f90_fe_status = status; */
/* 			       f90_fe_name = string_copy(name); */

/* 			       /\* Change the status to OKAY so that we can  */
/* 				* execute the lister on the cif_file; we will */
/* 				* take appropriate action on this status once  */
/* 				* the lister has finished executing. See below. */
/* 				*\/ */

/* 			       status = RC_OKAY; */
/*                         } */

/* 			if (phase == P_lister) { */
/* 			   if (status == RC_OKAY && f90_fe_status != RC_OKAY) { */

/* 			      /\* We had encountered an error in the F90_fe phase */
/* 			       * but we ignored it so that we could execute the */
/* 			       * lister on the cif file; we need to switch the */
/* 			       * status to the status we received from F90_fe */
/* 			       * and use the name of the F90_fe_phase, so that */
/* 			       * we can issue a correct error message. */
/* 			       *\/ */

/* 			       status = f90_fe_status; */
/* 			       name = string_copy(f90_fe_name); */

/* 			       /\* Reset f90_fe_status to OKAY for any further */
/* 				* compilations on other source files. */
/* 				*\/ */

/* 			       f90_fe_status = RC_OKAY; */
/*                            } */
/*                         } */

/* 			switch (status) { */
/* 			case RC_OKAY: */
/* 			  if (inline_t == UNDEFINED */
/* 			      && (is_matching_phase(get_phase_mask(phase), P_any_fe)) */
			      
/* 			      { */
/* 				    inline_t = FALSE; */
/* 				} */
/* 				break; */
/* 			case RC_NEED_INLINER: */
/* 				if (inline_t == UNDEFINED */
/* 				    && is_matching_phase( */
/* 					get_phase_mask(phase), P_any_fe) ) */
/* 				{ */
/* 					inline_t = TRUE; */
/* 				} */
/* 				/\* completed successfully *\/ */
/* 				break; */
				
/* 			case RC_USER_ERROR: */
/* 			case RC_NORECOVER_USER_ERROR: */
/* 			case RC_SYSTEM_ERROR: */
/* 			case RC_GCC_ERROR: */
/* 				user_err = TRUE; */
/* 				break; */

/* 			case RC_OVERFLOW_ERROR: */
/* 				if (!ran_twice && phase == P_be) { */
/* 					/\* try recompiling with larger limits *\/ */
/* 					ran_twice = TRUE; */
/* 					add_string (args, "-TENV:long_eh_offsets"); */
/* 					add_string (args, "-TENV:large_stack"); */
/* 					run_phase (phase, name, args); */
/* 					return; */
/* 				} */
/* 				internal_err = TRUE; */
/* 				break; */
/* 			case RC_INTERNAL_ERROR: */
/* 			      if (phase == P_ld || phase == P_ldplus) */
/* 				  { */
/* 					/\* gcc/ld returns 1 for undefined *\/ */
/* 					user_err = TRUE; */
/* 				} */
/* 				else */
/* 					internal_err = TRUE; */
/* 				break; */
/* 			default: */
/* 				internal_err = TRUE; */
/* 				break; */
/* 			}  */
/* 			if (internal_err) { */
/* 				internal_error("%s returned non-zero status %d", */
/* 					name, status); */
/* 			} */
/* 			else if (user_err) { */
/* 				/\* assume phase will print diagnostics *\/ */
/* 				if (!show_flag || save_stderr) { */
/* 					nomsg_error(); */
/* 				} else { */
/* 					error("%s returned non-zero status %d", */
/* 						name, status); */
/* 				} */
/* 			} */
/* 			ran_twice = FALSE; */
/* 			return; */
/* 		} else if(WIFSIGNALED(waitstatus)){ */
/* 			termsig = WTERMSIG(waitstatus); */
/* 			error("%s died due to signal %d", name, termsig); */
/* 			if(waitstatus & WCOREFLAG) { */
/* 				error("core dumped"); */
/* 			} */
/* 			if (termsig == SIGKILL) { */
/* 				error("Probably caused by running out of swap space -- check %s", LOGFILE); */
/* 			} */
/* 			cleanup(); */
/* 			exit(RC_SYSTEM_ERROR); */
/* 		} else { */
/* 			/\* cannot happen, I think! *\/ */
/* 			internal_error("driver exec'ing is confused"); */
/* 			return; */
/* 		} */
/* 	} */
#endif // TARG_ST
}

/*
 * Handler () is used for catching signals.
 */
extern void
handler (int sig)
{
#ifdef SIGINT
  if (sig == SIGINT) {
    fprintf(stderr, "%s INTERRUPTED:  stop processing", program_name);
    nomsg_error();
    goto cleanup;
  }
#endif
#if defined(SIGTERM)
  if (sig == SIGTERM) {
    fprintf(stderr, "%s TERMINATED:  stop processing", program_name);
    nomsg_error();
    goto cleanup;
  }
#endif
#if defined(__CYGWIN__) || defined(__MINGW32__)
  error("signal %d caught, stop processing", sig);
  error_status = RC_SYSTEM_ERROR;
#else
  error("signal %d (%s) caught, stop processing", sig, _sys_siglist[sig]);
  error_status = RC_SYSTEM_ERROR;
#endif
 cleanup:
  cleanup ();
  exit (error_status);
}

/* set signal handler */
extern void
catch_signals (void)
{
    /* modelled after Handle_Signals in common/util/errors.c */
#ifdef SIGHUP
    if (signal (SIGHUP, SIG_IGN) != SIG_IGN)
        signal (SIGHUP,  handler);
#endif
#ifdef SIGINT
    if (signal (SIGINT, SIG_IGN) != SIG_IGN)
        signal (SIGINT,  handler);
#endif
#ifdef SIGQUIT
    if (signal (SIGQUIT, SIG_IGN) != SIG_IGN)
        signal (SIGQUIT,  handler);
#endif
#ifdef SIGILL
    if (signal (SIGILL, SIG_IGN) != SIG_IGN)
        signal (SIGILL,  handler);
#endif
#ifdef SIGTRAP
    if (signal (SIGTRAP, SIG_IGN) != SIG_IGN)
        signal (SIGTRAP,  handler);
#endif
#ifdef SIGIOT
    if (signal (SIGIOT, SIG_IGN) != SIG_IGN)
        signal (SIGIOT,  handler);
#endif
#ifdef SIGEMT
    if (signal (SIGEMT, SIG_IGN) != SIG_IGN)
        signal (SIGEMT,  handler);
#endif
#ifdef SIGFPE
    if (signal (SIGFPE, SIG_IGN) != SIG_IGN)
        signal (SIGFPE,  handler);
#endif
#ifdef SIGBUS
    if (signal (SIGBUS, SIG_IGN) != SIG_IGN)
        signal (SIGBUS,  handler);
#endif
#ifdef SIGSEGV
    if (signal (SIGSEGV, SIG_IGN) != SIG_IGN)
        signal (SIGSEGV,  handler);
#endif
#ifdef SIGTERM
    if (signal (SIGTERM, SIG_IGN) != SIG_IGN)
        signal (SIGTERM,  handler);
#endif
#ifdef SIGPIPE
    if (signal (SIGPIPE, SIG_IGN) != SIG_IGN)
        signal (SIGPIPE,  handler);
#endif
}

/* this code is copied from csh, for printing times */

clock_t time0;
struct tms tm0;
#ifdef USE_CLOCK
clock_t clock0;
#endif

static void
init_time (void)
{
#ifdef USE_CLOCK
    clock0 = clock();
#endif
    time0 = times (&tm0);

}


static void
print_time (string phase)
{
  clock_t clock1;
  clock_t time1;
  double wtime;
  double utime, stime;
  struct tms tm1;
  
#ifdef USE_CLOCK
  clock1 = clock();
  wtime = (double)(clock1 - clock0)/CLOCKS_PER_SEC;
  fprintf (stderr, "%s phase elapsed time: %.2f sec.\n",
	   drop_path(phase), wtime);
#endif
  time1 = times(&tm1);
  utime = (double)(tm1.tms_utime + tm1.tms_cutime -
		   tm0.tms_utime - tm0.tms_cutime) / (double)HZ;
  stime = (double)(tm1.tms_stime + tm1.tms_cstime -
		   tm0.tms_stime - tm0.tms_cstime) / (double)HZ;
  wtime =  (double)(time1 - time0)/ (double)HZ;
  fprintf (stderr, "%s phase time:  %.3fu %.3fs %.3fe %.2f%%\n",
	   drop_path(phase), utime, stime, wtime,
	   ((utime + stime) / wtime) * 100.0);
}




