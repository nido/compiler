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
#include <string.h>
#include <unistd.h>
#include "W_errno.h"
#include "libiberty/libiberty.h"
#include "phases.h"
#include "lang_defs.h"
#include "string_utils.h"
#include "file_names.h"
#include "file_utils.h"
#include "errors.h"
#include "opt_actions.h"
#include "option_seen.h"
#include "option_names.h"
#include "run.h"
#include "SYS.h"

extern int errno;

boolean keep_flag = FALSE;
string keep_dir = NULL;

string_list_t *count_files = NULL;
static string_list_t *temp_files = NULL;

#ifndef TARG_ST
static string tmpdir;
#define DEFAULT_TMPDIR	"/tmp"
#endif

static string saved_object = NULL;

/* get object file corresponding to src file */
extern string
get_object_file (string src)
{
	return change_suffix(drop_path(src), "o");
}

/*
 * Need temp file names to be same if use same suffix
 * (because this can be called for both producer and consumer
 * of temp file), but also need names that won't conflict.
 * Put suffix in standard place so have easy way to check 
 * if file already created. 
 * [CG] for TARG_ST, use make_temp_file from libiberty instead of tempnam.
 * Use tempnam to generate unique file name;
 * tempnam verifies that file is writable.
 */
string
create_temp_file_name (string suffix)
{
#ifdef TARG_ST
  string temp;
  string_item_t *p;
  string dotted_suffix = concat_strings(".", suffix);
  int dotted_suffix_len = strlen(dotted_suffix);
  for (p = temp_files->head; p != NULL; p = p->next) {
    string name = p->name;
    int name_len = strlen(name);
    /* Check if tmp name ends with ".<suffix>". In this case returns this
       temp file. */
    if (name_len-dotted_suffix_len >= 0 &&
	strncmp(name+name_len-dotted_suffix_len, dotted_suffix, dotted_suffix_len) == 0) {
      return name;
    }
  }
  temp = make_temp_file(dotted_suffix);
  add_string (temp_files, temp);
  return temp;
#else
	buffer_t buf;
	buffer_t pathbuf;
	size_t pathbuf_len;
	string s;
 	string_item_t *p;
	
	/* use same prefix as gcc compilers;
	 * tempnam limits us to 5 chars, and may have 2-letter suffix. */
	sprintf(buf, "cc%s.", suffix);
	sprintf(pathbuf, "%s/%s", tmpdir, buf); /* full path of tmp files */
	pathbuf_len = strlen(pathbuf);

	for (p = temp_files->head; p != NULL; p = p->next) {
	  /* Can't use get_suffix here because we don't actually
		 * want the suffix. tempnam may return a value with a period
		 * in it. This will confuse our duplicates check below.
		 * We can't change get_suffix, because in other cases we
		 * actually want the right-most period. foo.bar.c
		 * We are guaranteed here that the first period after the last
		 * directory divider is the position we want because we chose
		 * its contents above.
		 */
		string file_name = SYS_baseptr(p->name);
		s = strchr(file_name, '.');
		/* we know that s won't be null because we created a string
		 * with a period in it. */
		s++;
		/* assume that s points inside p->name,
		 * e.g. /tmp/ccB.abc, s points to a */
#ifdef TARG_ST
		// [SC] When we have a two character suffix, e.g. "IL",
		// s - pathbuf_len may point to before the start of p->name,
		// triggering valgrind memcheck errors.
		if ((s - pathbuf_len) >= p->name
		    && strncmp(s-pathbuf_len, pathbuf, pathbuf_len) == 0) {
#else
		if (strncmp(s-pathbuf_len, pathbuf, pathbuf_len) == 0) {
#endif
			/* matches the prefix and suffix character */
			return p->name;
		}
	}
	/* need new file name */
	s = tempnam (tmpdir, buf);
	add_string (temp_files, s);
	return s;
#endif /* !TARG_ST */
}

extern string
construct_name (string src, string suffix)
{
	if (keep_flag || current_phase == remember_last_phase) {
		string srcname;
		/* 
		 * if -c -o <name>, then use name.suffix
		 * (this helps when use same source to create different .o's)
		 * if outfile doesn't have .o suffix, don't do this.
		 */
		if (outfile && option_was_seen(O_c) && get_suffix(outfile))
			srcname = outfile;
		else
			srcname = src;
/* fix #14600 */
      srcname = change_suffix(drop_path(srcname), suffix);
      if (keep_flag && (NULL!=keep_dir)) {
         return concat_path(keep_dir,srcname);
      } else {
         return srcname;
      }
/* end of fix */
	} else {
		return create_temp_file_name (suffix);
	}
}

/* use given src name, but check if treated as a temp file or not */
extern string
construct_given_name (string src, string suffix, boolean keep)
{
	string s;
	s = change_suffix(drop_path(src), suffix);
/* fix #14600 */
   if (keep_flag && (NULL!=keep_dir)) {
      s = concat_path(keep_dir,s);
   }
/* end of fix */
	if (keep || current_phase == remember_last_phase) {
		return s;
	} else {
		s = string_copy(s);
		add_string_if_new (temp_files, s);
		return s;
	}
}

extern void
mark_saved_object_for_cleanup ( void )
{
	if (saved_object != NULL)
	add_string_if_new (temp_files, saved_object);
}

/* Create filename with the given extension; eg. foo.anl from foo.f */
extern string
construct_file_with_extension (string src, string ext)
{
	return change_suffix(drop_path(src),ext);
}

extern void
init_temp_files (void)
{
#ifdef TARG_ST
#else
  tmpdir = getenv("TMPDIR");
  if (tmpdir == NULL) {
    tmpdir = DEFAULT_TMPDIR;
  } 
  else if (!is_directory(tmpdir)) {
    error("$TMPDIR does not exist: %s", tmpdir);
  } 
  else if (!directory_is_writable(tmpdir)) {
    error("$TMPDIR not writable: %s", tmpdir);
  } 
  else if (tmpdir[strlen(tmpdir)-1] == '/') {
    /* drop / at end so strcmp matches */
    tmpdir[strlen(tmpdir)-1] = '\0';
  }
#endif /* !TARG_ST */
  temp_files = init_string_list();
}

extern void
init_count_files (void)
{
        count_files = init_string_list();
}

#ifdef TARG_ST
static void
cleanup_list(string_list_t *list)
{
  /* cleanup temp-files */
  string_item_t *p;
  int status;
  if (list == NULL) return;
  for (p = list->head; p != NULL; p = p->next) {
    if (debug) printf("unlink %s\n", p->name);
    if (execute_flag) {
      status = unlink(p->name);
      if (status != 0 && errno != ENOENT) {
	internal_error("cannot unlink temp file %s", p->name);
	perror(program_name);			
      }
    }
  }
  list->head = list->tail = NULL; 
}
#endif

extern void
cleanup (void)
{
#ifdef TARG_ST
  cleanup_list(temp_files);
  if (has_errors()) {
     cleanup_src_objects();
  }
#else
	/* cleanup temp-files */
	string_item_t *p;
	int status;
	if (temp_files == NULL) return;
	for (p = temp_files->head; p != NULL; p = p->next) {
		if (debug) printf("unlink %s\n", p->name);
		if (execute_flag) {
		    status = unlink(p->name);
		    if (status != 0 && errno != ENOENT) {
			internal_error("cannot unlink temp file %s", p->name);
			perror(program_name);			
		    }
		}
	}
	temp_files->head = temp_files->tail = NULL; 
#endif
}


/* fix #14600 */

extern string_list_t *objects;

/* linked list of couples src/obj strings */
typedef struct src_obj_string_item_rec {
	string src;
	string obj;
   int    read;
   int    keep;
	struct src_obj_string_item_rec *next;
} src_obj_string_item_t;
typedef struct src_obj_string_list_rec {
	src_obj_string_item_t *head;
	src_obj_string_item_t *tail;
} src_obj_string_list_t;

/* iterator */
#define FOREACH_SRCOBJ(p,list)	\
	for (p = list->head; p != NULL; p = p->next)
#define SRCOBJ_SRC(p)	(p->src)
#define SRCOBJ_OBJ(p)	(p->obj)
#define SRCOBJ_READ(p)	(p->read)
#define SRCOBJ_KEEP(p)	(p->keep)

static src_obj_string_list_t src_obj_list = { NULL, NULL };

/* associates a source file name to its temporary object file 
   in a specific list */
extern void 
associate_src_object ( string src, string obj ) {
	src_obj_string_item_t *p;

	p = (src_obj_string_item_t *) malloc(sizeof(src_obj_string_item_t));
	SRCOBJ_SRC(p) = src;
   SRCOBJ_OBJ(p) = obj;
   SRCOBJ_READ(p)= 0;
   SRCOBJ_KEEP(p)= 0;
	p->next = NULL;
	if ((&src_obj_list)->head == NULL) {
		(&src_obj_list)->head = (&src_obj_list)->tail = p;
	} else {
		(&src_obj_list)->tail->next = p;
		(&src_obj_list)->tail = p;
	}
}

static src_obj_string_item_t *
find_associated_src_object ( string src ) {
	src_obj_string_item_t *p;
   
   FOREACH_SRCOBJ(p,(&src_obj_list)) {
      if (!strcmp(SRCOBJ_SRC(p),src)) {
         return p;
      }
   }
   return NULL;
}

extern string
get_associated_src_object ( string src ) {
   src_obj_string_item_t *result;
   
   result = find_associated_src_object(src);
   
   if (NULL==result) {
      associate_src_object(src,construct_name(src,"o"));
      result = find_associated_src_object(src);
   } else {
      extern boolean multiple_source_files;
      
      if (!SRCOBJ_READ(result)) {
         string_item_t * pobj, *svgpobj=NULL;
         string tmp;
         int status;
         
         if (keep_flag || option_was_seen(O_c)) {
            if (NULL!=keep_dir) {
               tmp = concat_path(keep_dir,change_suffix(drop_path(src), "o"));
            } else {
               tmp = change_suffix(drop_path(src),"o");
            }
            SRCOBJ_KEEP(result) = 1;
         } else {
              tmp = change_suffix(drop_path(src),"o");
         }
         FOREACH_STRING(pobj,objects) {
            if (!strcmp(STRING_NAME(pobj),SRCOBJ_OBJ(result))) {
               STRING_NAME(pobj) = tmp;
               svgpobj = pobj;
            }
         }
         /* must unlink preceeding tmp file */
         status = unlink(SRCOBJ_OBJ(result));
         if (status != 0 && errno != ENOENT) {
            internal_error("cannot unlink temp file %s", SRCOBJ_OBJ(result));
            perror(program_name);			
         }
         SRCOBJ_OBJ(result) = tmp;
      }
   }
   SRCOBJ_READ(result) = 1;
   return SRCOBJ_OBJ(result);
}

extern void cleanup_src_objects ( void ) {
   src_obj_string_item_t * pobj;
   int status;

   FOREACH_SRCOBJ(pobj,(&src_obj_list)) {
      if (has_errors() || !SRCOBJ_KEEP(pobj)) {
         status = unlink(SRCOBJ_OBJ(pobj));
         if (status != 0 && errno != ENOENT) {
            internal_error("cannot unlink temp file %s", SRCOBJ_OBJ(pobj));
            perror(program_name);			
         }
      }
   }
}
/* end of fix #14600 */
