/*

  Copyright (C) 2000, 2001 Silicon Graphics, Inc.  All Rights Reserved.

  This program is free software; you can redistribute it and/or modify it
  under the terms of version 2.1 of the GNU Lesser General Public License 
  as published by the Free Software Foundation.

  This program is distributed in the hope that it would be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

  Further, this software is distributed without any warranty that it is
  free of the rightful claim of any third person regarding infringement 
  or the like.  Any license provided herein, whether implied or 
  otherwise, applies only to this software file.  Patent licenses, if
  any, provided herein do not apply to combinations of this program with 
  other software, or any other product whatsoever.  

  You should have received a copy of the GNU Lesser General Public 
  License along with this program; if not, write the Free Software 
  Foundation, Inc., 59 Temple Place - Suite 330, Boston MA 02111-1307, 
  USA.

  Contact information:  Silicon Graphics, Inc., 1600 Amphitheatre Pky,
  Mountain View, CA 94043, or:

  http://www.sgi.com

  For further information regarding this notice, see:

  http://oss.sgi.com/projects/GenInfo/NoticeExplan

*/


#pragma ident "@(#) libfi/mpp_shift/cshift_p_wd2_1.c	92.1	07/13/99 10:44:06"


#include <stdlib.h>
#include <liberrno.h>
#include <cray/dopevec.h>
#include "f90_macros.h"

#define RANK		1
#define	CSHIFT_COMPLEX	1

/*
 *	Compiler generated call:
 *		CALL _CSHIFT_CP1(RESULT, SOURCE, SHIFT, DIM)
 *
 *	Purpose: Perform an end-off shift of the SOURCE array along the DIM
 *		 dimension.  The magnitude of the shift is found in SHIFT.
 *
 *	Arguments:
 *		RESULT   - Dope vector for result temporary array
 *		SOURCE   - Dope vector for user source array
 *		SHIFT    - Dope vector for shift count
 *		DIM	 - Dimension along which to shift (optional)
 *
 *	Description:
 *		This is the MPP version of CSHIFT.  This particular file
 *		contains the intermediate routines.  These routines parse
 *		and update the dope vectors, allocate either shared or
 *		private space for the result temporary, and possibly update
 *		the shared data descriptor (sdd) for the result temporary.
 *		Once this set-up work is complete, a Fortran subroutine is
 *		called which uses features from the Fortran Programming
 *		Model to perform the actual shift.
 *
 *		Include file cshift_p.h contains the rank independent
 *		source code for this routine.
 */

void
_CSHIFT_CP1 (
		DopeVectorType  *result,
		DopeVectorType  *source,
		DopeVectorType  *shift,
		int		*dim)

{
#include "cshift_p.h"

/*
 *      Call the Fortran work routine
 */

	special = 0;
	if (shflag == _btol(1)) {
	    if (shftval >= -1 && shftval <= 1) {
		if (_blkct (source_sdd_ptr, 1, 0) == 1) {
		    special = 1;
		}
	    }
	}

	if (special) {
	    CSHIFT_SPEC_WD2_P1@ ( result_sdd_ptr, source_sdd_ptr,
		&dim_val, src_extents, &shftval);
	} else {
	    CSHIFT_WD2_P1@ ( result_sdd_ptr, source_sdd_ptr, src_extents,
	    &shftval);
	}
}
