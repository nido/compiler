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


// ====================================================================
// ====================================================================
//
// Module: profile_error.c
//
// Revision history:
//  24-Jul-98 - Original Version
//
// Description:
//
// ====================================================================
// ====================================================================

#include <stdio.h>
#include <stdlib.h>

#include "profile_error.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void profile_error(char *fmt, char *fname)
{
    fprintf (stderr, "%s: ", "libinstrC: ");
    fprintf(stderr, fmt, fname);
    fputc('\n', stderr);
    fflush(stderr);
    return ;
} 

void profile_warn(char *fmt, char *fname)
{
    fprintf (stderr, "%s: ", "libinstrC: ");
    fprintf(stderr, fmt, fname);
    fputc('\n', stderr);
    fflush(stderr);
    return;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

