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


#include "defs.h"
#include "erglob.h"
#include "err_host.h"
#include "tracing.h"
#ifdef TARG_ST 
#include "config_target.h"
#else
#include "config_targ.h"
#endif
#include "mempool.h"
#include "mempool_allocator.h"
#include "config.h"
#include "opcode.h"
#include "symtab.h"
#include "strtab.h"
#include "stab.h"


#include "targ_sim.h"

#include "config_host.h"
#include "irbdata.h"
#include "irbdata_defs.h"
#include "language.h"
#include "srcpos.h"
#include "const.h"
#include "targ_const.h"
#include "wintrinsic.h"
#include "wio.h"
#include "wn.h"
#include "wn_map.h"
#include "wn_pragmas.h"
#include "wn_simp.h"
#include "wutil.h"
#include "cxx_memory.h"
#include "errors.h"