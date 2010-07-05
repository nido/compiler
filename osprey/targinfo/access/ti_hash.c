
/*

  Copyright (C) 2007 STMicroelectronics, Inc.  All Rights Reserved.

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

*/

#include <stdio.h>
#include "defs.h"
#include "ti_hash.h"

/* Hash routine popular in literature dedicated to the subject 
 * Hash table is assumed to have a size that is a power of 2
 * and mask is simply 2**n-1.
 */

mUINT32 TI_HASH_one_at_a_time( char *string, mUINT32 mask )
{
     mUINT32 length = strlen(string);
     mUINT32 res = 0;
     mUINT32 i;

     for(i=0;i<length;++i) {
        res += string[i];
        res += res << 10U;
        res ^= res >> 6U;
    }

    res += res << 3;
    res ^= res >> 11;
    res += res << 15;
    return res & mask;
}

