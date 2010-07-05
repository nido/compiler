/*

  Copyright (C) 2000 Silicon Graphics, Inc.  All Rights Reserved.

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


// isa_operands_gen.h
/////////////////////////////////////
//
//  Interface for selecting TOPs depending on activated ISA subset.
//
//  void ISA_Selector_Begin( const char* archname )
//      Initialize to generate selectors for the architecture 
//      with the given <archname>.  The information will be written to the 
//      files targ_isa_selector.h.  
//
//  void ISA_Selector_Operator(	const char* operator_name, ... )
//      Used to create a new selector operator. Variadic arguments are couple
//      of ISA_SUBET and corresponding TOP. This variadic argument list
//      must be terminated by ISA_SUBSET_UNDEFINED.
//
//  void ISA_Selector_End(void)
//      Complete processing of TOP selector generation.
//
//
/////////////////////////////////////


#ifndef isa_selector_gen_INCLUDED
#define isa_selector_gen_INCLUDED

#ifdef _KEEP_RCS_ID
#endif /* _KEEP_RCS_ID */

#ifdef __cplusplus
extern "C" {
#endif

extern void ISA_Selector_Begin ( const char* archname );
extern void ISA_Selector_Operator( const char* operator_name, ... );
extern void ISA_Selector_End(void);

#ifdef __cplusplus
}
#endif
#endif /* isa_selector_gen_INCLUDED */
