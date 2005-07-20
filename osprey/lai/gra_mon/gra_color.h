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

//  GRA register assignment via graph coloring
/////////////////////////////////////
//  
//  Description:
//
//      Interface for actual register assignment.
//
//  Exported functions:
//
//      void GRA_Color(void)
//          Choose registers for all the global LRANGEs in the current PU or
//          region or whatever.
//
#ifdef TARG_ST
//      BOOL Can_Allocate_From (INT nregs,
//                              REGISTER_SET subclass_allowed,
//                              REGISTER_SET allowed)
//          
//          Return TRUE if it is possible to find a sequence of NREGS
//          registers from ALLOWED, whose first register is in
//          SUBCLASS_ALLOWED.
//
#endif
/////////////////////////////////////




#ifndef GRA_COLOR_INCLUDED
#define GRA_COLOR_INCLUDED
#ifndef GRA_COLOR_RCS_ID
#define GRA_COLOR_RCS_ID
#ifdef _KEEP_RCS_ID
#endif
#endif

extern void
GRA_Color(void);

#ifdef TARG_ST
extern BOOL
Can_Allocate_From (INT nregs,
		   REGISTER_SET subclass_allowed,
		   REGISTER_SET allowed);
#endif

#endif
