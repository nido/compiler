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


#pragma ident "@(#) libu/ffio/ffread.c	92.2	10/11/99 15:30:43"

#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ffio.h>
/*
 * This module contains a driver that starts the chain of calls
 * through the foreign dataset control layers.
 *
 */

#if	defined(__mips) || defined(_LITTLE_ENDIAN)
ssize_t
ffread(int fd, void *buf, size_t nbytes)
#elif _USE_OLD_PROTOTYPE == 0
ssize_t
ffread(int fd, void *buf, size_t nbytes, ...)
#else
int
ffread(int fd, char *buf, int nbytes, ...)
#endif
	{
	struct fdinfo *fio;
	ssize_t ret;
	int  locfulp;
	bitptr bufptr;
	int locubc, *pubc, na;	/* need a place to put result */
	struct ffsw locstat, *pstat;	/* need a place to put result */
#if	!defined(__mips) && !defined(_LITTLE_ENDIAN)
	va_list ap;
#endif

	fio = GETIOB(fd);
	SET_BPTR(bufptr, CPTR2BP(buf));
	/* adjust number of bits requested if ubc passed in */
#ifdef _CRAY
	NUMARG(na);
#elif	defined(__mips) || defined(_LITTLE_ENDIAN)
	na = 3;
#endif
	locubc = 0;
	pubc = &locubc;
	locfulp = FULL;
	pstat = &locstat;
#if	!defined(__mips) && !defined(_LITTLE_ENDIAN)
	va_start(ap, nbytes);
	if (na < 3 || na > 6)
		{
		errno = FDC_ERR_NOPARM;
		return(ERR);
		}
	if (na > 3)
		pstat = va_arg(ap, struct ffsw *);
	if (na > 4)
		locfulp = va_arg(ap, int);
	if (na > 5)
		pubc = va_arg(ap, int *);
#endif
	CHECK_FIOPTR(fio, pstat);
	ret = XRCALL(fio, readrtn) fio, bufptr, nbytes,
						pstat, locfulp, pubc);
	if (na < 4)
		errno = locstat.sw_error;
	return (ret);
	}
/*
 * This routine is like ffread, except it expects all parameters .
 * If ubc == NULL, then do not return ubc information to user.
 */
ssize_t
ffreadf(int fd, void *buf, size_t nbytes, struct ffsw *pstat, int locfulp,
 int *ubc)
	{
	struct fdinfo *fio;
	ssize_t ret;
	bitptr bufptr;
	int locubc, *pubc;

	
	if (ubc == NULL) {
		pubc = &locubc;
		locubc = 0;
	}
	else {
		pubc = ubc;
	}
	fio = GETIOB(fd);
	SET_BPTR(bufptr, CPTR2BP(buf));
	CHECK_FIOPTR(fio, pstat);
	ret = XRCALL(fio, readrtn) fio, bufptr, nbytes,
						pstat, locfulp, pubc);
	return (ret);
	}
