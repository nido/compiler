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
#ifdef BCO_ENABLED /* Thierry */
#include "sys/types.h"
#endif /* BCO_Enabled Thierry */
/* drops path prefix in string; result points inside old string */
extern string drop_path (string s);

/* check whether file exists */
extern boolean file_exists (string path);

/* check whether is a directory */
extern boolean is_directory (string path);

/* check whether directory is writable */
extern boolean directory_is_writable (string path);

#ifdef TARG_ST
/* Make a path from two path fragments. */
extern string concat_path (string path, string path2);
#endif

#ifndef OLD
/* Obsolete. Use getpwd from libiberty. */
/* get current working directory */
extern string get_cwd (void);
#endif

#ifdef BCO_ENABLED /* Thierry */
/* Return time of last data modification of a file */
extern time_t
get_date_of_file (string path);
#endif /* BCO_Enabled Thierry */

#ifndef OLD
#ifdef TARG_ST
extern string get_driver_directory(char *program) ;
extern string get_driver_absolute_directory(char *program) ;
extern string get_root_directory(string driver_directory) ;
#endif
#endif

