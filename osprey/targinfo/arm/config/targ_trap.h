/*

  Copyright (C) 2001 ST Microelectronics, Inc.  All Rights Reserved.

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

  Contact information:

  http://www.st.com

  For further information regarding this notice, see:

  http://oss.sgi.com/projects/GenInfo/NoticeExplan

*/


/* ====================================================================
 * ====================================================================
 *
 *   Description:
 *
 *   TRAP description for the target processor.
 *   Replaces the OS description for the ST100.
 *
 * ====================================================================
 * ====================================================================
 */

#ifndef targ_trap_INCLUDED
#define targ_trap_INCLUDED

/* Trap values for the SIGFPE signal: */

#define FPE_INTDIV_trap    0    /* Integer divide by zero.  */
#define FPE_INTOVF_trap    1	/* Integer overflow.  */
#define FPE_FLTDIV_trap	   -1   /* Floating point divide by zero.  */
#define FPE_FLTOVF_trap	   -1   /* Floating point overflow.  */
#define FPE_FLTUND_trap	   -1   /* Floating point underflow.  */
#define FPE_FLTRES_trap	   -1   /* Floating point inexact result.  */
#define FPE_FLTINV_trap	   -1   /* Floating point invalid operation.  */
#define FPE_FLTSUB_trap	    2   /* Subscript out of range.  */

#endif /* targ_trap_INCLUDED */
