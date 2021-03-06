/*
 *  Copyright (C) 2008 PathScale, LLC.  All Rights Reserved.
 */

/*
 * Copyright 2002, 2003, 2004, 2005, 2006 PathScale, Inc.  All Rights Reserved.
 */

/*

  Copyright (C) 2000, 2001 Silicon Graphics, Inc.  All Rights Reserved.

   Path64 is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   Path64 is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with Path64; see the file COPYING.  If not, write to the Free
   Software Foundation, 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.

   Special thanks goes to SGI for their continued support to open source

*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <errno.h>
#include "errors.h"
#include "string_utils.h"
#include "file_utils.h"

extern int errno;
static char *saved_orig_program_name;

#ifdef KEY /* Mac port */
int compat_gcc;
#endif /* KEY Mac port */

/* drops path prefix in string */
char *
drop_path (char *s)
{
        char *tail;
        tail = strrchr (s, '/');
	if (tail == NULL) {
		return s;	/* no path prefix */
	} else {
		tail++;		/* skip the slash */
		return tail;	/* points inside s, not new string! */
	}
}

/* drops the last component of the path, leaving only the directory */
char *
directory_path (char *s)
{
        char *t, *tail = NULL;
	char path[MAXPATHLEN];
	for (t = s; *t; t++) {
		if (*t == '/')
			tail = t;
	}
	if (tail == NULL) {
		return NULL;
	} else {
		return substring_copy (s, 0, tail-s);
	}
}

char *
concat_path (char *d, char *f)
{
	if ((d == NULL) || (strlen (d) == 0) || strcmp(d, "/") == 0) {
		/* Directory is root, don't return //f */
		return concat_strings ("/", f);
	} else if ((f == NULL) || (strlen (f) == 0)) {
		/* file is null, return directory portion only */
		return d;
	} else {
		char *path = (char *) malloc(strlen(d) + strlen(f) + 2);
		strcpy (path, d);
		strcat (path, "/");
		strcat (path, f);
		return path;
	}
}

boolean
file_exists (char *path)
{
	int st;
	struct stat sbuf;
	st = stat(path, &sbuf);
	if (st == -1 && (errno == ENOENT || errno == ENOTDIR))
		return FALSE;
	else
		return TRUE;
}

boolean
is_executable (char *path)
{
	int st;
	struct stat sbuf;
	st = stat(path, &sbuf);
	if (st == -1 && (errno == ENOENT || errno == ENOTDIR))
		return FALSE;
	else
                /* this check is not completely accurate */
	        return (sbuf.st_mode & (S_IXUSR|S_IXGRP|S_IXOTH)) != 0;
}

boolean
is_directory (char *path)
{
        /* check if . file exists */
        buffer_t buf;
	if (*path == NIL) return FALSE;		/* empty path */
        strcpy(buf, path);
        strcat(buf, "/.");
        if (file_exists(buf)) 
		return TRUE;
        else 
		return FALSE;
}

boolean want_directory (char *path)
{
	if (is_directory(path))
		return TRUE;

#ifdef KEY
	// Warn about missing directory argument to -I/-L.  Bug 2794.
	if (fullwarn ||
	    (path[0] == '-' && compat_gcc != TRUE))
		warning("%s is not a directory", path);
	// Accept anything as a directory after (possibly) warning about the
	// invalid ones.
	return TRUE;
#else
	if (fullwarn)
		warning("%s is not a directory", path);
	return TRUE;
#endif
}

/* check if directory is writable */
boolean
directory_is_writable (char *path)
{
	FILE *f;
	char *s;
	int fd;
	s = concat_strings(path, "/ctm.XXXXXX");
	fd = mkstemp(s);
	if (fd == -1)
		return FALSE;
	f = fdopen(fd, "w");
	if (f == NULL) {
		return FALSE;
	} else {
		fclose (f);
		unlink(s);
		return TRUE;
	}
}

char *
get_cwd (void)
{
	char *cwd = getcwd((char *) NULL, MAXPATHLEN);
	if (cwd == NULL) {
		cwd = getenv("PWD");
		if (cwd == NULL) {
			/* can't get path */
			cwd = ".";
		}
	}
	return string_copy(cwd);
}

void file_utils_set_program_name(char *name)
{
        saved_orig_program_name = name;
}

#ifdef KEY
// Get the saved program name.
char *
file_utils_get_program_name()
{
        return saved_orig_program_name;
}
#endif

// Get program path from PATH variable.
static char *
get_executable_dir_from_path(char *name)
{
  if (name[0] != '/') {
    char *path = getenv("PATH");
    if (path != NULL) {
      char *p = string_copy(path);
      char *tmp;
      char *dir;
      while ((dir = strtok_r(p, ":", &tmp)) != NULL) {
	if (is_directory(dir)) {
	  char filename[MAXPATHLEN];
	  snprintf(filename, MAXPATHLEN, "%s/%s", dir, name);
	  if (is_executable(filename)) {
	    return string_copy(filename);
	  }
	}
        p = NULL;
      }
    }
  }
  return name;
}

char *
get_executable_dir (void)
{
	char path[MAXPATHLEN];
	int rval;
	int i;

	/* Look in this special place for a link to the executable. This
	   only works on Linux, but it is benign if we try it elsewhere. */
	rval = readlink ("/proc/self/exe", path, sizeof(path));
	if (rval <= 0) {
		// If can't read /proc/self/exe, get program path from PATH
		// variable.
		char *p = get_executable_dir_from_path(saved_orig_program_name);
		strncpy(path, p, sizeof(path));
		rval = strlen(path);
	} else {
		path[rval] = '\0';	// readlink doesn't append NULL
	}
	if (rval > 0) {
		for (i=rval-1; i >= 0; i--) {
			if (path[i] == '/') break;
		}
		if (i > 0) {
			/* Overwrite the trailing slash, giving the directory
			   portion of the path. */
			path[i] = '\0';      
		} else if (i == 0) {
			/* Directory is the root */
		        strcpy (path, "/");
		}
		if (is_directory (path)) {
			/* Verify that it is a directory */
			return string_copy (path);
		}
	}

	/* TBD: try to extract the name from argv0 */

	/* Can't get anything reasonable. */
	return NULL;
}

void
dump_file_to_stdout(char *filename)
{
  const int buf_size = 1024;
  char buf[buf_size];
  FILE *f;
  int n;

  if (filename == NULL || !file_exists(filename))
    internal_error("file does not exist");
  
  f = fopen(filename, "r");
  if (f == NULL)
    internal_error("cannot open file for read");

  // Copy the content of file to stdout.
  while ((n = fread(buf, 1, buf_size, f)) > 0) {
    write(1, buf, n);
  }
  fclose(f);
}
