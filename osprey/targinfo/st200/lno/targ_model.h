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


//-*-c++-*-
//                     Machine Modeling
//                     ----------------
//

#ifndef targ_MODEL_INCLUDED
#define targ_MODEL_INCLUDED

/* ST220 integer registers */
#define	Target_INTRs	64

/* ST220 ptr registers */
#define	Target_PTRs	0

/* ST220 Branch registers */
#define	Target_BRs	8

/* Target_FPRRs
 * The number of Floating-Point registers is specified by default
 * in config/config_target.cxx (declared in config/config_TARG.h).
 * It can be specified in the command line, see config_TARG.cxx.
 */

/* from be/lno/model.cxx:
 * How many different types of register do we need to have the
 * pipelines going + some architectural registers
 */
#define Reserved_Int_Regs	4	// [(load-use-store latency) + 1]*2DUs
                                        // so the pipeline is kept busy
#define Reserved_Ptr_Regs	5       // $sp, $fp, $gp, $pr, $lr
#define Reserved_Bool_Regs	1       // g0 ?

#endif /* targ_MODEL_INCLUDED */
