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


/* $Header$ */
/*	3.0 SID #	1.2	*/
/* EFL support routine to copy string b to string a */

#include <cmplrs/host.h>
#include "s_copy.h"

#define M	( (int32_t) (sizeof(int32_t) - 1) )
#define EVEN(x)	( ( (x)+ M) & (~M) )

void
ef1asc_ (char *a, int32_t *la, char *b, int32_t *lb)
{
    s_copy( a, b, EVEN(*la), *lb );
}
