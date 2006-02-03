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


/* This is a set of stubs for the WHIRL simplifier, for those
places where we don't want it. */
#ifdef USE_PCH
#include "common_com_pch.h"
#endif /* USE_PCH */
#pragma hdrstop

#include "defs.h"
#include "errors.h"
#include "erglob.h"
#include "tracing.h"
#include "stab.h"
#include "wn.h"
#include "wn_util.h"
#include "ir_reader.h"

#include "config.h"
#ifdef TARG_ST 
#include "config_target.h"
#else
#include "config_targ.h"
#endif

#include "const.h"
#include "targ_const.h"
#include "wn_simp.h"


BOOL WN_Simplifier_Enable(BOOL enable) 
{
   BOOL r = Enable_WN_Simp;
   Enable_WN_Simp = enable;
   return (r);
}

#if 0	/* mpm:  now defined in wn_simp? */
WN * WN_SimplifyExp1(OPCODE opc, MEM_POOL * pool, WN * k0)
{
   return (WN *) NULL;
}

WN * WN_SimplifyExp2(OPCODE opc, MEM_POOL * pool, WN * k0, WN * k1)
{
   return (WN *) NULL;
}

WN *WN_SimplifyCvtl(OPCODE opc, INT16 cvtl_bits, WN *k0)
{
   return (WN *) NULL;
}

#endif
