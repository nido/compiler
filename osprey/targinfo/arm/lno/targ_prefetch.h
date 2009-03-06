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

#ifndef targ_PREFETCH_INCLUDED
#define targ_PREFETCH_INCLUDED

#include "targ_proc_properties.h"

/* From be/lno/prefetch.cxx. */

static inline BOOL
Target_ISA_Has_Prefetch()
{
  /* Return TRUE if target ISA has a prefetch instruction. */
  return PROC_has_prefetch();
}

static inline UINT32
Target_Proc_Run_Prefetch()
{
  /* Return 0 for no automatic prefetching,
     1 for conservative prefetching,
     2 for aggressive prefetching.
  */
  /* clarkes: 030403: Automatic prefetching is
     disabled by default, until we are confident
     that it is useful.
     When we are confident, should return 2 for
     ST220.
  */
  return 0;
}

#endif /* targ_PREFETCH_INCLUDED */

