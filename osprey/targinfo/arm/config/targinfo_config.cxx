/*
Copyright (C) 2010 ST Microelectronics, Inc.  All Rights Reserved.

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

*/
#include <stdio.h>

#include "targinfo_config.h"
#include "dyn_isa_api_versions.h"

extension_unit_property_pairing* CGTARG_get_Extension_Unit_Map(INT32 api_revision, INT32* count) {
  
  // default behavior
  *count = 0;
  return NULL;
}
