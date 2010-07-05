/*

  Copyright (C) 2008, STMicroelectronics Inc.  All Rights Reserved.


*/

#include "wn.h"
#include "wintrinsic.h"
#include "extension_intrinsic.h"

/** 
 * Initialize pattern_rec
 * 
 */
void
BETARG_Init_Pattern_Rec(void) {
}


/**
 * Function that generates an intrinsic OP from extension code generation.
 * Return the corresponding whirl node in case of success, return NULL otherwise.
 *
 * @param intrnidx  id of the intrinsic
 * @param nbkids    arguments count of the intrinsics
 * @param kids      arguments of the intrinsics
 * @param dsttype   return type
 * @param new_stmts statement containing the initial code, if any
 * @param modified  will be set in case of successful generation
 */
WN *
BETARG_Create_Intrinsic_from_OP(INTRINSIC intrnidx, int nbkids, WN *kids[],
				TYPE_ID dsttype, WN** new_stmts, bool* modified)
{
  return Create_Intrinsic_from_OP(intrnidx, nbkids, kids, 0, NULL, dsttype, new_stmts, modified);
}
