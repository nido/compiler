/*

  Copyright (C) 2007, STMicroelectronics Inc.  All Rights Reserved.

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

#include "defs.h"
#include "pqs_cg.h"
#include "pqs.h"
#include "cgir.h"
#include "pqs_target.h"

void
PQSTARG_classify_instruction (PQS_MANAGER *pqsm, OP *op)
{
  TOP topcode = OP_code (op);
  TN *qual;

  if (OP_has_predicate(op)) {
    qual = OP_opnd(op, OP_find_opnd_use(op, OU_predicate));
  } else {
    qual = NULL;
  }

  if (OP_icmp (op)) {
    VARIANT v = OP_cmp_variant (op);

    pqsm->Add_Predicate_Cmp_Def (OP_result (op, 0), qual, v,
				 OP_Opnd1 (op), OP_Opnd2 (op));
  } else if (topcode == TOP_orl_b_b_b
	     || topcode == TOP_norl_b_b_b
	     || topcode == TOP_andl_b_b_b
	     || topcode == TOP_nandl_b_b_b) {
    VARIANT v = ((topcode == TOP_orl_b_b_b)  ? V_CMP_ORL :
		 (topcode == TOP_norl_b_b_b) ? V_CMP_NORL :
		 (topcode == TOP_andl_b_b_b) ? V_CMP_ANDL :
		 V_CMP_NANDL);

    pqsm->Add_Predicate_Cmp_Def (OP_result (op, 0), qual, v,
				 OP_opnd (op, 0), OP_opnd (op, 1));
  } else if (OP_copy (op)) {
    pqsm->Add_Copy (OP_Copy_Result_TN (op), qual, OP_Copy_Operand_TN (op));
  } else if (topcode == TOP_convbi_b_r || topcode == TOP_convib_r_b) {
    pqsm->Add_Copy (OP_result (op, 0), qual, OP_opnd (op, 0));
  }
}

BOOL
PQSTARG_sets_results_if_qual_true(OP *op)
{
  return TRUE;
}

void
PQSTARG_target_synthesized_info (PQS_MANAGER *pqsm)
{
}
  
