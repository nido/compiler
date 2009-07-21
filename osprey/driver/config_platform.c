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


/* ====================================================================
 * ====================================================================
 *
 * Module: config_platform.c
 *
 * Revision history:
 *  11-May-96 - Original Version
 *
 * Description:
 *
 * Configuration for the various platforms supported by the compiler.
 * This package is used to configure certain platform-specific options
 * like the -Ofast option set and cache configuration for LNO.
 *
 * This is separate from config_targ because it has a very small set
 * of clients, whereas config_targ.h is included in config.h and hence
 * everywhere.
 *
 * NOTE:  There is an outstanding bug, PV 378171, to base this
 * functionality on an external configuration file.
 *
 * ====================================================================
 * ====================================================================
 */

static char *source_file = __FILE__;

#ifdef _KEEP_RCS_ID
#endif /* _KEEP_RCS_ID */

#include <string.h>
#include "config_platform.h"

PLATFORM Platform = IP0;	/* Target platform */

/* Per-processor settings: */
static PLATFORM_OPTIONS PF_Opts[] =
{
  /* This is the default -- it must be first.  Nickname is used for
   * default warning messages:
   */
  { IP0, "ip??", "itanium", PROC_ITANIUM, "Itanium" },

  { IP19,    "ip19",	"r4k",	PROC_R4K,  "Challenge" },
  { IP20,    "ip20",	"r4k",	PROC_R4K,  "R4000 Indigo" },
  { IP21,    "ip21",	"r8k",	PROC_R8K,  "Power Challenge" },
  { IP22_4K, "ip22_4K",	"r4k",	PROC_R4K,  "R4x00 Indy/Indigo2" },
  { IP22_5K, "ip22_5K",	"r5k",	PROC_R5K,  "R5000 Indy/Indigo2" },
  { IP24,    "ip24",	"r5k",	PROC_R5K,  "R5000 Indy/Indigo2" },
  { IP25,    "ip25",	"r10k",	PROC_R10K, "R10000 Power Challenge" },
  { IP26,    "ip26",	"r8k",	PROC_R8K,  "R8000  Indigo2" },
  { IP27,    "ip27",	"r10k",	PROC_R10K, "Origin 2000" },
  { IP28,    "ip28",	"r10k",	PROC_R10K, "R10000 Indigo2" },
  { IP30,    "ip30",	"r10k",	PROC_R10K, "R10000 Octane" },
  { IP32_5K, "ip32_5K",	"r5k",	PROC_R5K,  "R5000  O2" },
  { IP32_10K,"ip32_10K","r10k",	PROC_R10K, "R10000 O2" },

  /* ST additions */
  { IP0, "ip??", "st100", PROC_ST100, "ST100" },
  { IP0, "ip??", "st210", PROC_ST210, "ST210" },
  { IP0, "ip??", "st220", PROC_ST220, "ST220" },
  { IP0, "ip??", "st231", PROC_ST231, "ST231" },
  { IP0, "ip??", "st240", PROC_ST240, "ST240" },
  { IP0, "ip??", "arm9",  PROC_armv5, "ARM9" },
  { IP0, "ip??", "arm11", PROC_armv6, "ARM11" },

  { IP_END,	"",	"",	PROC_NONE, "End of list" } /* Must be last */
};

PLATFORM_OPTIONS *
Get_Platform_Options ( char *name )
{
  int ix;

  /* Default for no name or empty name: */
  if ( name == NULL || *name == 0 ) {
    return &PF_Opts[0];
  }

  /* Find matching entry in table: */
  for ( ix=1; PF_Opts[ix].id != IP_END; ix++ ) {
    if ( strcasecmp ( name, PF_Opts[ix].name ) == 0 ) {
      return &PF_Opts[ix];
    }
  }

  /* No match -- default it: */
  return &PF_Opts[0];
}

