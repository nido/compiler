/*

  Copyright (C) 2002, ST Microelectronics, Inc.  All Rights Reserved.

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


#include "defs.h"

#include "pqs_defs.h"
#include "pqs_cg.h"
#include "pqs.h"
#include "tracing.h"
/* #include "targ_pqs.h" */

/* ====================================================================
 *   PQSTARG_get_top_info
 *
 *   TODO: generate automatically.
 * ====================================================================
 */
void
PQSTARG_get_top_info(
  TOP x, 
  PQS_ITYPE &itype, 
  PQS_RELOPTYPE &relop
)
{
   relop = PQS_RELOPTYPE_OTHER;
   itype = PQS_ITYPE_INVALID;

   switch (x) {
     case TOP_GP32_EQE_GT_BR_DR_DR:
     case TOP_GP32_EQE_GT_BR_DR_U8:
     case TOP_GP32_EQW_GT_BR_DR_DR:
     case TOP_GP32_EQW_GT_BR_DR_U8:
     case TOP_GP32_GEE_GT_BR_DR_DR:
     case TOP_GP32_GEE_GT_BR_DR_U8:
     case TOP_GP32_GEW_GT_BR_DR_DR:
     case TOP_GP32_GEW_GT_BR_DR_U8:
     case TOP_GP32_GTE_GT_BR_DR_DR:
     case TOP_GP32_GTE_GT_BR_DR_U8:
     case TOP_GP32_GTW_GT_BR_DR_DR:
     case TOP_GP32_GTW_GT_BR_DR_U8:
     case TOP_GP32_LEE_GT_BR_DR_DR:
     case TOP_GP32_LEE_GT_BR_DR_U8:
     case TOP_GP32_LEW_GT_BR_DR_DR:
     case TOP_GP32_LEW_GT_BR_DR_U8:
     case TOP_GP32_LTE_GT_BR_DR_DR:
     case TOP_GP32_LTE_GT_BR_DR_U8:
     case TOP_GP32_LTW_GT_BR_DR_DR:
     case TOP_GP32_LTW_GT_BR_DR_U8:
     case TOP_GP32_NEE_GT_BR_DR_DR:
     case TOP_GP32_NEE_GT_BR_DR_U8:
     case TOP_GP32_NEW_GT_BR_DR_DR:
     case TOP_GP32_NEW_GT_BR_DR_U8:
       itype = PQS_ITYPE_NORM;
       break;

    default: 
       itype = PQS_ITYPE_NOPREDICATES;
       break;
   }

   
   switch (x) {
     case TOP_GP32_NEE_GT_BR_DR_DR:
     case TOP_GP32_NEE_GT_BR_DR_U8:
     case TOP_GP32_NEW_GT_BR_DR_DR:
     case TOP_GP32_NEW_GT_BR_DR_U8:
       relop = PQS_RELOPTYPE_NE;
       break;

     case TOP_GP32_EQE_GT_BR_DR_DR:
     case TOP_GP32_EQE_GT_BR_DR_U8:
     case TOP_GP32_EQW_GT_BR_DR_DR:
     case TOP_GP32_EQW_GT_BR_DR_U8:
       relop = PQS_RELOPTYPE_EQ;
       break;
   }

   return;
}

