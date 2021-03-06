/*

  Copyright (C) 2000, 2001 Silicon Graphics, Inc.  All Rights Reserved.

   Path64 is free software; you can redistribute it and/or modify it
   under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation version 2.1

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


#pragma ident "@(#) libfi/mathlb/rtoi.c	92.2	09/27/99 14:56:21"


#include <fortran.h>
#include <fp.h>
#include <math.h>

extern _f_real8 _RTOI( _f_real8 r, _f_int8 i);

/*
 * RTOI - Real(kind=8) raised to an integer(kind=64) power
 */
_f_real8
_RTOI( _f_real8 r,
	   _f_int8 i)
{
	_f_real8 base, result;
	if (i == 0) {
		if (r != 0.0) {
			result	= 1.0;
			/* retain isnan since _SGL_NaN
			 * is just one form of many
			 * possible nans.
			 */
			if (isnan(r))
				result	= r;
		}
		else {
#if	defined(__mips) || defined(_LITTLE_ENDIAN)
			result	= 1.0;
#else
			result	= _SGL_NaN;
#endif
		}
		return(result);
	}
	result	= 1.0;
	base	= r;
	if (i < 0) {
		if (r != 0.0) {
			base	= 1.0/r;
			i	= -i;
		}
	}
	while (i != 0) {
		if ((i & 1) == 1)
			result *= base;
		i =	(unsigned) i >> 1;
		if (i != 0)
			base *= base;
	}
	return (result);
}

#if	defined(__mips) || defined(_LITTLE_ENDIAN)
_f_real8
__powdl( _f_real8 x,
	_f_int8 y )
{
	return(_RTOI(x, y));
}

/* if needed for compatibility with f77 and pass by address */
#if 0
_f_real8
pow_dl( _f_real8 *x,
	_f_int8 *y )
{
	return(_RTOI(*x, *y));
}
#endif	/* end of if 0 */

#endif	/* end of mips or little endian */
