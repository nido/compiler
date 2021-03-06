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


static char USMID[] = "@(#) libcif/cifunitdir.c	30.2	07/26/96 07:19:13";


/*
 *	Cif_Getunitdir returns the unit directory for the requested unit from
 *	an open sorted CIF.
 */

#define CIF_VERSION 3

#ifdef _ABSOFT
#include "cif.h"
#else
#include <cif.h>
#endif

#include <memory.h>
#include <stdio.h>

#include "cif_int.h"

int
Cif_Getunitdir
#ifdef __STDC__
(int cifd, struct Cif_unittbl *utp, struct Cif_unitdir **unitdir)
#else
(cifd, utp, unitdir)
int cifd;
struct Cif_unittbl *utp;
struct Cif_unitdir **unitdir;
#endif
{

	FILE *fd;
	int i, n;
	struct Cif_unitdir *udp;
	struct Cif_urectbl *urp;

	char buf[64];
	struct Cif_urectbl ur[CIF_MAXRECORD];
	struct Cif_unit unit;

	int status = 0;				/* status value */
	
	if (cifd < 0 || cifd > CIF_FT_SIZE || _Cif_filetbl[cifd].form == NOT_A_CIF)
		return (CIF_NOTOPEN);
	else if (_Cif_filetbl[cifd].optype == 'w' || _Cif_filetbl[cifd].form ==
			ASCII_CIF || _Cif_filetbl[cifd].seek == NO)
		return (CIF_BADREQ);
	_Cif_filetbl[cifd].ifull = NO;

	/* Position the file to the start of the unit, read the unit record and
	 * discard it.
	 */

	fd = _Cif_filetbl[cifd].fd;
	if (fseek (fd, utp->unitpos, 0))
		return (CIF_SYSERR);
	if (fread (&unit, _Cif_shortsize[CIF_UNIT][_Cif_filetbl[cifd].version], 1, fd) != 1) IO_ERROR;
	if ((n = unit.nlen) > 0)
		if (fread (buf, sizeof(char), n, fd) != n) IO_ERROR;

	/* Read in the unit directory structure, then the individual record
	 * entries.  Expand to the full directory structure.
	 */

	i = _Cif_filetbl[cifd].mode;
	/*
	 * If memory management mode isn't set, then set to FIXED.  If the
	 * mode is FIXED, reset the amount of buffer used.
	 */

	if (i == CIF_MEM_DEFAULT) {
	  if ((status = Cif_Memmode (cifd, CIF_MEM_FIXED)) != 0)
	    return (status);
	  i = _Cif_filetbl[cifd].mode;
	}
	if (i == CIF_MEM_FIXED)
	  _Cif_memarea[_Cif_filetbl[cifd].fme].mused = 0;

	udp = *unitdir = (struct Cif_unitdir *) _Cif_space[i]
	                                   (_Cif_structsize[CIF_UNITDIR][_Cif_filetbl[cifd].return_version], cifd);
	if (udp == NULL)
		return (CIF_NOMEM);
	if (fread ((char *)udp, _Cif_shortsize[CIF_UNITDIR][_Cif_filetbl[cifd].version], 1, fd) != 1) IO_ERROR;

/* should map this */

	n = sizeof(struct Cif_urectbl) * CIF_MAXRECORD;
	urp = udp->ur = (struct Cif_urectbl *) _Cif_space[i] (n, cifd);
	if (urp == NULL)
		return (CIF_NOMEM);
	(void) memset ((char *)urp, '\0', n);
	n = udp->nsections;
	if (fread ((char *)ur, sizeof(struct Cif_urectbl), n, fd) != n) IO_ERROR;
	for (i = 0; i < n; i++) {
		urp[ur[i].rectype] = ur[i];

		  /* mask out those records that this cif version (eg v1) doesn't know about */

		  if (_Cif_shortsize[ur[i].rectype][_Cif_filetbl[cifd].return_version] == 0)
		    urp[ur[i].rectype].nrecords = 0;
		}

	if (_Cif_filetbl[cifd].return_version == 1)
	  	udp->nsections = CIF_MAXRECORD_1;
	else
	  	udp->nsections = CIF_MAXRECORD;

	return (0);
}
