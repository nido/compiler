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
#include "lang_defs.h"

/*
 * Routines for handling the list of predefined options,
 * plus any user-defined options.
 */

extern void init_options (void);	/* init the options list */

extern string get_option_name (int flag);	/* return name */
extern string get_option_help (int flag);	/* return help msg */

/* whether option should have a blank space in it, e.g. -o foo */
extern boolean option_has_blank (int flag);

/* whether the option is valid for the language */
extern boolean option_matches_language (int flag, languages_t l);
/* whether the option is valid for the phase */
extern boolean option_matches_phase (int flag, phases_t p);
/* whether the option is internal (based on language mask) */
extern boolean is_internal_option (int flag);
extern void set_internal_option (int flag);

/* add phase to list of valid phases for option */
extern void add_phase_for_option(int flag, phases_t p);
/* remove phase from list of valid phases for option */
extern void remove_phase_for_option(int flag, phases_t p);
/* set language for option */
extern void set_language_for_option (int flag, languages_t l);

#ifdef TARG_ST
// [CL]
extern mask_t option_phases (int flag);
extern boolean has_implied_option(int flag);
#endif

/* add new user-defined option */
extern int add_new_option (string arg);

/*
 * For options with arbitrary argument, we have the option prefix in 
 * our predefined table/list of options.
 * Now we want to create a new derived entry for the exact option string,
 * which will point back to the prefix entry.  
 */
extern int add_derived_option (int parent, string arg);
extern boolean is_derived_option (int flag);	/* is option derived? */
extern int get_derived_parent (int flag);	/* return base parent */

/*
 * iterator routines 
 * (should use FOREACH macros rather than individual routines)
 */
/* iterate through all known options */
#define FOREACH_OPTION(i)	\
	for (i = first_option(); !no_more_options(); i = next_option())
extern int first_option (void);
extern int next_option (void);
extern boolean no_more_options (void);

/* iterate through options in option-combination-list */
#define FOREACH_OPTION_IN_COMBO(i,c)	\
	for (i = first_combo_item(c); !no_more_combo_items(c); i = next_combo_item(c))
extern int first_combo_item (int combo_flag);
extern int next_combo_item (int combo_flag);
extern boolean no_more_combo_items (int combo_flag);

/* iterate through implied options for a particular option */
#define FOREACH_IMPLIED_OPTION(i,o)	\
	for (i = first_implied_option(o); !no_more_implied_options(o); i = next_implied_option(o))
extern int first_implied_option (int flag);
extern int next_implied_option (int flag);
extern boolean no_more_implied_options (int flag);

/* return name of current implied string;
 * MUST BE INSIDE IMPLIED ITERATOR when calling this routine! */
extern string get_current_implied_name (void);

#ifdef TARG_STxP70
typedef struct extension_implies_list_rec {
   int info_index;
	string name;
	struct extension_implies_list_rec *next;
} extension_implies_list_T;

typedef struct {
   string name;
   string architecture;
   string exthwtype;
   string help;
   string libpath;
   string libname;
   extension_implies_list_T * implies;
} extension_T;

typedef enum {
  STXP70_ARCH_V3 = 0,
  STXP70_ARCH_V4 = 1,
  STXP70_ARCH_SIZE = 2
} stxp70_architecture_dim;

typedef enum {
  STXP70_EXTHWTYPE_SINGLE = 0,
  STXP70_EXTHWTYPE_SINGLE_DUAL = 1,
  STXP70_EXTHWTYPE_DUAL_DUAL = 2,
  STXP70_EXTHWTYPE_SIZE = 3
} stxp70_exthwtype_dim;

#define STXP70_MAX_EXTENSION 50
extern extension_T static_ext_opt[STXP70_ARCH_SIZE][STXP70_EXTHWTYPE_SIZE][STXP70_MAX_EXTENSION];
extern int static_ext_nr[STXP70_ARCH_SIZE][STXP70_EXTHWTYPE_SIZE];
extern void extract_from_sxextensionrc ( char * driver_path );
extern void connect_extensions ( void ) ;
#endif