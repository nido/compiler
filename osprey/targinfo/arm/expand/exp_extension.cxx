/*
 
  Copyright (C) 2006 ST Microelectronics, Inc.  All Rights Reserved.
 
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
 
  Contact information:  ST Microelectronics, Inc.,
  , or:
 
  http://www.st.com
 
  For further information regarding this notice, see:
 
  http:
*/

/*
 * Date: 2006/08/01
 * Desc: These functions are used to expand extension specific operations,
 *       such as compose/extract/widemove/load/store.
 */

#include "cgexp.h"
#include "erglob.h"

/*
 * Macro used to simplify search of available TOP in extension, by trying getting
 * a specific TOP for different register size, starting with the biggest one.
 * Parameters:
 * IN_function      : Function called to try retrieving a TOP
 *                    Expected prototype:  TOP IN_function(INT size)
 * IN_biggest_rsize : Preferred register width
 * IN_biggest_rsize : Last possible register width
 * OUT_top          : Resulting TOP, TOP_UNDEFINED if not found
 * OUT_rsize        : Register size used by returned TOP
 */
#define GET_TOP_FOR_BIGGEST_AVAIL_REG_SIZE(IN_function,			\
					   IN_biggest_rsize,		\
					   IN_smallest_rsize,		\
					   OUT_top,			\
					   OUT_rsize)			\
  {									\
    OUT_rsize = IN_biggest_rsize;					\
    OUT_top   = IN_function (OUT_rsize);				\
    while ((OUT_top == TOP_UNDEFINED) &&				\
	   (OUT_rsize > IN_smallest_rsize)) {				\
      OUT_rsize = OUT_rsize/2;						\
      OUT_top   = IN_function (OUT_rsize);				\
    }									\
  }

// [TTh] 2006/08/01: Predicated compose/extract are not yet fully supported by
//       the compiler, explaining why they are currently disabled.
//
// #define PREDICATED_COMPOSE_EXTRACT

/*
 * Build a simulated extract TOP, that extract 2 sub-TNs from a composite one.
 */
void Build_OP_simulated_extract(TN *tgt0, TN *tgt1, TN *guard, TN *src, OPS *ops) {
  const extension_regclass_t *rc_info;
  TOP top;
  TN *results[2];
  TN *operands[2];

  // Retrieve TOP
  rc_info = EXTENSION_get_REGISTER_CLASS_info(TN_register_class(src));
  top = rc_info->get_simulated_extract_TOP(TN_size(src), TN_size(tgt0));
  FmtAssert((top != TOP_UNDEFINED),
	    ("Cannot find simulated extract TOP(src size:%d, tgt size:%d)\n",
	     TN_size(src), TN_size(tgt0)));

  // Build result array
  results[0]  = tgt0;
  results[1]  = tgt1;

  // Build operand array
#ifdef PREDICATED_COMPOSE_EXTRACT
  operands[0] = guard;
  operands[1] = src;
#else
  operands[0] = src;
#endif

  // Create op
#ifdef PREDICATED_COMPOSE_EXTRACT
  OPS_Append_Op(ops, Mk_VarOP(top, 2, 2, results, operands));
#else
  OPS_Append_Op(ops, Mk_VarOP(top, 2, 1, results, operands));
#endif
}


/*
 * Build a simulated extract TOP, that extract 4 sub-TNs from a composite one.
 */
void Build_OP_simulated_extract(TN *tgt0, TN *tgt1, TN *tgt2, TN *tgt3, TN *guard, TN *src, OPS *ops) {
  const extension_regclass_t *rc_info;
  TOP top;
  TN *results[4];
  TN *operands[2];

  // Retrieve TOP
  rc_info = EXTENSION_get_REGISTER_CLASS_info(TN_register_class(src));
  top = rc_info->get_simulated_extract_TOP(TN_size(src), TN_size(tgt0));
  FmtAssert((top != TOP_UNDEFINED),
	    ("Cannot find simulated extract TOP(src size:%d, tgt size:%d)\n",
	     TN_size(src), TN_size(tgt0)));

  // Build result array
  results[0]  = tgt0;
  results[1]  = tgt1;
  results[2]  = tgt2;
  results[3]  = tgt3;

  // Build operand array
#ifdef PREDICATED_COMPOSE_EXTRACT
  operands[0] = guard;
  operands[1] = src;
#else
  operands[0] = src;
#endif

  // Create op
#ifdef PREDICATED_COMPOSE_EXTRACT
  OPS_Append_Op(ops, Mk_VarOP(top, 4, 2, results, operands));
#else
  OPS_Append_Op(ops, Mk_VarOP(top, 4, 1, results, operands));
#endif
}


/*
 * Build a simulated compose TOP, that produce a composite TN from 2 sub-TNs.
 */
void Build_OP_simulated_compose(TN *tgt, TN *guard, TN *src0, TN *src1, OPS *ops) {
  const extension_regclass_t *rc_info;
  TOP top;
  TN *results[1];
  TN *operands[3];

  // Retrieve TOP
  rc_info = EXTENSION_get_REGISTER_CLASS_info(TN_register_class(tgt));
  top = rc_info->get_simulated_compose_TOP(TN_size(src0), TN_size(tgt));
  FmtAssert((top != TOP_UNDEFINED),
	    ("Cannot find simulated compose TOP(src size:%d, tgt size:%d)\n",
	     TN_size(src0), TN_size(tgt)));

  // Build result array
  results[0]  = tgt;

  // Build operand array
#ifdef PREDICATED_COMPOSE_EXTRACT
  operands[0] = guard;
  operands[1] = src0;
  operands[2] = src1;
#else
  operands[0] = src0;
  operands[1] = src1;
#endif

  // Create op
#ifdef PREDICATED_COMPOSE_EXTRACT
  OPS_Append_Op(ops, Mk_VarOP(top, 1, 3, results, operands));
#else
  OPS_Append_Op(ops, Mk_VarOP(top, 1, 2, results, operands));
#endif
}


/*
 * Build a simulated compose TOP, that produce a composite TN from 4 sub-TNs.
 */
void Build_OP_simulated_compose(TN *tgt, TN *guard, TN *src0, TN *src1, TN *src2, TN *src3, OPS *ops) {
  const extension_regclass_t *rc_info;
  TOP top;
  TN *results[1];
  TN *operands[5];

  // Retrieve TOP
  rc_info = EXTENSION_get_REGISTER_CLASS_info(TN_register_class(tgt));
  top = rc_info->get_simulated_compose_TOP(TN_size(src0), TN_size(tgt));
  FmtAssert((top != TOP_UNDEFINED),
	    ("Cannot find simulated compose TOP(src size:%d, tgt size:%d)\n",
	     TN_size(src0), TN_size(tgt)));

  // Build result array
  results[0]  = tgt;

  // Build operand array
#ifdef PREDICATED_COMPOSE_EXTRACT
  operands[0] = guard;
  operands[1] = src0;
  operands[2] = src1;
  operands[3] = src2;
  operands[4] = src3;
#else
  operands[0] = src0;
  operands[1] = src1;
  operands[2] = src2;
  operands[3] = src3;
#endif

  // Create op
#ifdef PREDICATED_COMPOSE_EXTRACT
  OPS_Append_Op(ops, Mk_VarOP(top, 1, 5, results, operands));
#else
  OPS_Append_Op(ops, Mk_VarOP(top, 1, 4, results, operands));
#endif
}


/*
 * Return true if the alignment constraint of <top> operation is smaller
 * or equal to data_align.
 * Return false otherwise.
 */
static inline BOOL IsCompatibleAlignment(TOP top, UINT32 data_align) {
  UINT32 top_align;
  top_align = TOP_Mem_Alignment (top);
  return ((data_align == 0) || (top_align <= data_align));
}


/*
 * Expand a copy operation between 2 TNs, more likely to be extension ones.
 * - It can produce one or more move operations, depending on the TOP 
 *   availables for the corresponding register class.
 * - If the TNs are not yet allocated and there is no single move TOP 
 *   available for this register size, a simulated widemove TOP is
 *   generated, to help the register allocator.
 * pre-condition: at least one of the tn register belongs to an extension
 */
BOOL EXTENSION_Expand_Copy(TN *tgt_tn, TN *guard, TN *src_tn, OPS *ops) {

  FmtAssert(FALSE,("Not yet implemented"));
#if 0

  const extension_regclass_t *rc_info;
  TOP top_move = TOP_UNDEFINED;
  INT nb_access;
  INT i;

  ISA_REGISTER_CLASS tgt_rc = TN_register_class(tgt_tn);
  ISA_REGISTER_CLASS src_rc = TN_register_class(src_tn);

  INT tgt_rc_size = ISA_REGISTER_CLASS_INFO_Bit_Size(ISA_REGISTER_CLASS_Info(tgt_rc)) / 8;

  INT tgt_size = TN_size(tgt_tn);
  INT src_size = TN_size(src_tn);
  
  if (tgt_rc == src_rc) {
    FmtAssert((tgt_size == src_size), ("Cannot expand extension copy, because of incompatible size"));

    rc_info = EXTENSION_get_REGISTER_CLASS_info(src_rc);

    // Retrieve the widest move TOP for current TN and
    // define the number of access required (1, 2 or 4 parts)
    nb_access = 1;
    top_move = rc_info->get_move_X2X_TOP(tgt_size);
    if (top_move == TOP_UNDEFINED &&
	(tgt_size == tgt_rc_size*2 || tgt_size == tgt_rc_size*4)) {
      // Move not defined for level 1
      nb_access = 2;
      top_move = rc_info->get_move_X2X_TOP(tgt_size/2);
      if (top_move == TOP_UNDEFINED &&
	  (tgt_size == tgt_rc_size*4)) {
	// Move not defined for level 2
	nb_access = 4;
	top_move = rc_info->get_move_X2X_TOP(tgt_size/4);
      }
    }
    if (top_move == TOP_UNDEFINED) {
      FmtAssert(0, ("Cannot generate Copy, no TOP found"));
    }

    // ###############################
    // ## Effective code generation ##
    // ###############################
    switch (nb_access) {

    case 1:
      // Found a TOP to perform the move in a single operation: do it.
      Build_OP(top_move, tgt_tn, guard, src_tn, ops);
      if (TN_is_true_pred(guard)) Set_OP_copy (OPS_last(ops)); // Mark true copy
      return TRUE;
      break;
      
    case 2:
    case 4:
    default:
      // No TOP found to perform the move in a single operation.
      // Different process depending on TN status:
      // Non-allocated TNs: 
      // --> Generate a single simulated TOP widemove
      //     (Facilitate register allocation work)
      // Allocated TNs:
      // --> Generate a set of move operating on subparts of the TNs
      if (TN_register(src_tn) == REGISTER_UNDEFINED ||
	  TN_register(tgt_tn) == REGISTER_UNDEFINED) {
	// Non-allocated TNs
	// -----------------
	// Retrieve simulated TOP
	top_move = rc_info->get_simulated_widemove_TOP(src_size);
	FmtAssert((top_move != TOP_UNDEFINED), ("Cannot get simulated widemove for current size(%d)\n", src_size));
	Build_OP(top_move, tgt_tn, src_tn, ops);
	return TRUE;
      }
      else {
	// Allocated TNs
	// -------------
	TN *src_sub_tn, *tgt_sub_tn;
	int sub_access_size = src_size/nb_access;
	REGISTER src_reg = TN_register(src_tn);
	REGISTER tgt_reg = TN_register(tgt_tn);

	for (i=0; i<nb_access; i++) {
	  src_sub_tn = Gen_Register_TN(src_rc, sub_access_size);
	  Set_TN_register(src_sub_tn, src_reg+i);

	  tgt_sub_tn = Gen_Register_TN(tgt_rc, sub_access_size);
	  Set_TN_register(tgt_sub_tn, tgt_reg+i);

	  Build_OP(top_move, tgt_sub_tn, guard, src_sub_tn, ops);
	}
	return TRUE;
      }
      break;
    }
  }
  else {  // != reg classes
    // Note: only 32bits moves are handled here.
    // Indeed, 64 bits move needs 2 input TNs. These moves should be handled at higher level
    // of the code generation, if required.
    if (src_rc == ISA_REGISTER_CLASS_integer) {
      rc_info = EXTENSION_get_REGISTER_CLASS_info(tgt_rc);
      if (tgt_size <= 4) {
	top_move = rc_info->get_move_R2X_TOP(tgt_size);
      }
      else {
	top_move = rc_info->get_insert_and_zeroext_R2X_TOP(tgt_size);
      }
      if (top_move == TOP_UNDEFINED) {
	FmtAssert(0, ("Cannot generate Copy from gpr, no TOP found"));
      }
      Build_OP(top_move, tgt_tn, guard, src_tn, ops);
      return TRUE;
    }
    else if (tgt_rc == ISA_REGISTER_CLASS_integer) {
      rc_info = EXTENSION_get_REGISTER_CLASS_info(src_rc);
      if (src_size <= 4) {
	top_move = rc_info->get_move_X2R_TOP(src_size);
      }
      if (top_move == TOP_UNDEFINED) {
	FmtAssert(0, ("Cannot generate Copy to gpr, no TOP found"));
      }
      Build_OP(top_move, tgt_tn, guard, src_tn, ops);
      return TRUE;
    }
    else {  // Move between extension register classes
      FmtAssert(0, ("Copy between extension register classes not implemented"));
    }
  }
#endif
  return FALSE;
}


/*
 * Perform sign extension of source TN to dest TN, both belonging to same
 * register class. Source size should be smaller or equal to Dest size.
 * If from_32bits is true, only the lowest 32 bits of Source register
 * are correctly sign extended, otherwise the source register is already
 * sign extended on its full size.
 */
static BOOL EXTENSION_Expand_SignExt(TN *dest, TN *src, BOOL from_32bits, OPS *ops) {

  FmtAssert(FALSE,("Not yet implemented"));
#if 0
  const extension_regclass_t *rc_info;
  ISA_REGISTER_CLASS rc;
  int src_rsize;
  int dest_rsize;
  int rc_size;
  TN *sub_TNs[4];

  rc         = TN_register_class(src);
  rc_size    = ISA_REGISTER_CLASS_INFO_Bit_Size(ISA_REGISTER_CLASS_Info(rc)) / 8;
  rc_info    = EXTENSION_get_REGISTER_CLASS_info(rc);
  src_rsize  = TN_size(src);
  dest_rsize = TN_size(dest);

  FmtAssert((src_rsize <= dest_rsize), ("Unexpected register size for sign extension"));

  // Note: if SIGNX is available, it implicitly means that Dest is greater than 32 bits
  TOP top_SIGNX = rc_info->get_signext_X_TOP(dest_rsize);
  TOP top_Q2SP  = rc_info->get_signext_P2X_TOP(dest_rsize/2);
  TOP top_P2SX  = rc_info->get_signext_P2X_TOP(dest_rsize);
  TOP top_CLRX  = rc_info->get_clear_TOP(dest_rsize);

  // -----------------------------------------------------------------
  // Various code selection will be tried, order by preference.
  // Choice will be made based on available predefined instructions
  // and Source and Dest sizes.
  // -----------------------------------------------------------------
  if (src_rsize == dest_rsize) {
    if (!from_32bits) {
      //
      // No sign extension required, just copy TNs if needed
      //
      if (src != dest) {
	return (EXTENSION_Expand_Copy(dest, True_TN, src, ops));
      }
      return TRUE;
    }
    else if (top_SIGNX != TOP_UNDEFINED) {
      //
      // Source and dest TNs have same size but only lowest 32 bits are 
      // correctly sign extended in source
      // ->   RFnSIGNX   dest, src, #1
      //
      Build_OP(top_SIGNX, dest, True_TN, src, Gen_Literal_TN(1, 4), ops);
      return TRUE;
    }
  }

  if ((from_32bits) && (src_rsize > 4) && (src_rsize > rc_size) && (top_P2SX != TOP_UNDEFINED)) {
    if (((src_rsize/2 == 4) || ((top_SIGNX == TOP_UNDEFINED || top_CLRX == TOP_UNDEFINED) &&
				(rc_info->get_signext_X_TOP(src_rsize/2) != TOP_UNDEFINED))) &&
	(( src_rsize == dest_rsize) ||
	 ((src_rsize == dest_rsize/2) && (top_Q2SP != TOP_UNDEFINED)))) {
      //
      // Source is bigger than 32 bits and only lowest 32 bits are sign extended:
      // Extract the lowest half of Source and insure it is correclty sign extended
      // ->   Extract   sub_0, sub_1, src
      // -> ( RFnSIGNx  sub_0, sub_0, #1 )  (only if Source is > 64 bits)
      //    ( Sign Extension from sub_0 to Dest done afterwards)
      //
      src_rsize  = src_rsize/2;
      sub_TNs[0] = Gen_Register_TN(rc, src_rsize);
      sub_TNs[1] = Gen_Register_TN(rc, src_rsize);
      Build_OP_simulated_extract(sub_TNs[0], sub_TNs[1], True_TN, src, ops);
      src        = sub_TNs[0];
      if (src_rsize > 4) {
	TOP top_SIGNP = rc_info->get_signext_X_TOP(src_rsize);
	Build_OP(top_SIGNP, src, True_TN, src, Gen_Literal_TN(1, 4), ops);
	from_32bits = FALSE;
      }
    }
    else if (((src_rsize/4 == 4) || ((top_SIGNX == TOP_UNDEFINED || top_CLRX == TOP_UNDEFINED) &&
				     (rc_info->get_signext_X_TOP(src_rsize/4) != TOP_UNDEFINED))) &&
	     (( src_rsize == dest_rsize) && (top_Q2SP != TOP_UNDEFINED))) {
      //
      // Source is bigger than 32 bits and only lowest 32 bits are sign extended:
      // Extract the lowest quarter of Source and insure it is correclty sign extended
      // ->   Extract   sub_0, sub_1, sub_2, sub_3, src
      // -> ( RFnSIGNQ  sub_0, sub_0, #1 )    (only if Source is > 64 bits)
      //    ( Sign Extension from sub_0 to Dest done afterwards)
      //
      src_rsize = src_rsize/4;
      for (int i=0; i<4; i++) {
	sub_TNs[i] = Gen_Register_TN(rc, src_rsize);
      }
      Build_OP_simulated_extract(sub_TNs[0], sub_TNs[1], sub_TNs[2], sub_TNs[3], True_TN, src, ops);
      src = sub_TNs[0];
      if (src_rsize > 4) {
	TOP top_SIGNQ = rc_info->get_signext_X_TOP(src_rsize);
	Build_OP(top_SIGNQ, src, True_TN, src, Gen_Literal_TN(1, 4), ops);
	from_32bits = FALSE;
      }
    }
  }
  
  if ((!from_32bits) || (src_rsize == 4)) {
    if ((dest_rsize == src_rsize*2) && (top_P2SX != TOP_UNDEFINED)) {
      //
      // Source is correctly sign extended but is smaller than dest
      // ->   RFnP2SX   dest, src
      //
      Build_OP(top_P2SX, dest, True_TN, src, ops);
      return TRUE;
    }
    
    if ((dest_rsize == src_rsize*4) && (top_Q2SP != TOP_UNDEFINED) && (top_P2SX != TOP_UNDEFINED)) {
      //
      // Source is correctly sign extended but is smaller than dest
      // ->   RFnQ2SP   tmp64, src
      // ->   RFnP2SX   dest, tmp64
      //
      TN *tmp_tn = Gen_Register_TN(rc, dest_rsize/2);
      Build_OP(top_Q2SP, tmp_tn, True_TN, src, ops);
      Build_OP(top_P2SX, dest, True_TN, tmp_tn, ops);
      return TRUE;
    }
  }

  if ((src_rsize >= 4) && (top_SIGNX != TOP_UNDEFINED) && (top_CLRX != TOP_UNDEFINED)) {
    //
    // Source is smaller than dest but at least 32 bits wide.
    // Note: this is the less efficient code, as CLR is required for liveness analysis,
    //       (to initialize tmpx_sub1 (+_sub2 and _sub3 if defined) but is finally useless
    //       in the final code as the SIGNX will overwrite those register parts.
    // ->   RFnCLRx    tmpx
    // ->   Extract    tmpx_sub0, tmpx_sub1, tmpx
    // ->   Compose    tmpx, src, tmpx_sub1
    // ->   RFnSIGNX   dest, tmpx, #n
    //
    TN *tmp_tn = Gen_Register_TN(rc, dest_rsize);
    Build_OP(top_CLRX, tmp_tn, True_TN, ops);

    int nb_part = dest_rsize / src_rsize;

    // Create temporary sub-registers
    for (int i=0; i<nb_part; i++) {
      sub_TNs[i] = Gen_Register_TN(rc, src_rsize);
    }
    if (nb_part == 2) {
      Build_OP_simulated_extract(sub_TNs[0], sub_TNs[1], True_TN, tmp_tn, ops);
      Build_OP_simulated_compose(tmp_tn, True_TN, src, sub_TNs[1], ops);
    } else {  // nb_access == 4
      Build_OP_simulated_extract(sub_TNs[0], sub_TNs[1], sub_TNs[2], sub_TNs[3], True_TN, tmp_tn, ops);
      Build_OP_simulated_compose(tmp_tn, True_TN, src, sub_TNs[1], sub_TNs[2], sub_TNs[3], ops);
    }
    TN *start = Gen_Literal_TN(from_32bits?1:(src_rsize/4), 4);
    Build_OP(top_SIGNX, dest, True_TN, tmp_tn, start, ops);
    return TRUE;
  }
 
  char error_msg[128];
  sprintf(error_msg, "[%s] Failed to sign extend data on extension register "
	  "class, lack of predefined instructions.",
	  EXTENSION_Get_Extension_Name_From_REGISTER_CLASS(rc));
  ErrMsg(EC_Ext_Expand, error_msg);

#endif
  return FALSE;
}


/*
 * Perform zero extension of source TN to dest TN, both belonging to same
 * register class. Source size should be smaller to Dest size.
 */
static BOOL EXTENSION_Expand_ZeroExt(TN *dest, TN *src, OPS *ops) {

  FmtAssert(FALSE,("Not yet implemented"));
#if 0

  const extension_regclass_t *rc_info;
  ISA_REGISTER_CLASS rc;
  int src_rsize;
  int dest_rsize;
  TN *sub_TNs[4];

  rc         = TN_register_class(src);
  src_rsize  = TN_size(src);
  dest_rsize = TN_size(dest);
  rc_info    = EXTENSION_get_REGISTER_CLASS_info(rc);

  FmtAssert((src_rsize <= dest_rsize), ("Unexpected register size for zero extension"));

  // -----------------------------------------------------------------
  // Various code selection will be tried, order by preference.
  // Choice will be made based on available predefined instructions
  // and Source and Dest sizes.
  // -----------------------------------------------------------------
  if (src_rsize == dest_rsize) {
    // No zero extension required, just copy TNs if needed
    if (src != dest) {
      return (EXTENSION_Expand_Copy(dest, True_TN, src, ops));
    }
    return TRUE;
  }

  if (dest_rsize == src_rsize*2) {
    // Source is half the size of the Dest, 2 solutions:
    TOP top_P2UX = rc_info->get_zeroext_P2X_TOP(dest_rsize);
    if (top_P2UX != TOP_UNDEFINED) {
      // ->   RFnP2UX   dest, src
      Build_OP(top_P2UX, dest, True_TN, src, ops);
      return TRUE;
    }

    TOP top_CLR = rc_info->get_clear_TOP(src_rsize);
    if (top_CLR != TOP_UNDEFINED) {
      // ->   RFnCLRp   tmp
      // ->   Compose   dest, src, tmp
      TN *tmp_tn = Gen_Register_TN(rc, src_rsize);
      Build_OP(top_CLR, tmp_tn, True_TN, ops);
      Build_OP_simulated_compose(dest, True_TN, src, tmp_tn, ops);
      return TRUE;
    }
  }
  else if (dest_rsize == src_rsize*4) {
    // Source is the quarter of the size of the Dest
    TOP top_Q2UP = rc_info->get_zeroext_P2X_TOP(dest_rsize/2);
    TOP top_P2UX = rc_info->get_zeroext_P2X_TOP(dest_rsize);
    if ((top_Q2UP != TOP_UNDEFINED) && (top_P2UX != TOP_UNDEFINED)) {
      // ->   RFnQ2UP   tmp, src
      // ->   RFnP2UX   dest, tmp
      TN *tmp_tn = Gen_Register_TN(rc, dest_rsize/2);
      Build_OP(top_Q2UP, tmp_tn, True_TN, src, ops);
      Build_OP(top_P2UX, dest, True_TN, tmp_tn, ops);
      return TRUE;
    }

    TOP top = rc_info->get_clear_TOP(dest_rsize);
    if (top != TOP_UNDEFINED) {
      // ->   RFnCLRx    tmpx
      // ->   Extract    sub0, sub1, sub2, sub3, tmpx
      // ->   Compose    tmpx, src, sub1, sub2, sub3
      TN *tmp_tn = Gen_Register_TN(rc, dest_rsize);
      Build_OP(top, tmp_tn, True_TN, ops);
      Build_OP_simulated_extract(sub_TNs[0], sub_TNs[1], sub_TNs[2], sub_TNs[3], True_TN, tmp_tn, ops);
      Build_OP_simulated_compose(dest, True_TN, src, sub_TNs[1], sub_TNs[2], sub_TNs[3], ops);
      return TRUE;
    }
  }

  char error_msg[128];
  sprintf(error_msg, "[%s] Failed to zero extend data on extension register "
	  "class, lack of predefined instructions.",
	  EXTENSION_Get_Extension_Name_From_REGISTER_CLASS(rc));
  ErrMsg(EC_Ext_Expand, error_msg);
#endif
  return FALSE;
}


/*
 * Expand a 32 bits immediate value within an extension register.
 * Various sequences of code might be generated, depending on the
 * immediate value and available predefined instructions.
 * If immediate is 0 and CLR instruction available, use it
 * Otherwise, generate the following instruction sequence:
 *   MAKE       Rn, #const    (copy value in gpr)
 *   XRFnR2X    Xn, Rn        (move and zero extend to ext register)
 *   XRFnSIGNX  Xn, Xn        (sign extend)
 *   Note: the later is not required if immediate value is known and positive,
 *         or if extension register is smaller or equal to 32 bits.
 */
BOOL EXTENSION_Expand_Immediate(TN   *dest,
				TN   *src,
				BOOL  is_signed,
				OPS  *ops) {
  FmtAssert(FALSE,("Not yet implemented"));
#if 0

  char error_msg[128];
  const extension_regclass_t *rc_info;
  ISA_REGISTER_CLASS dest_rc;
  INT dest_rc_size;
  INT dest_rsize;
  int nb_access;
  int i;
  TN *sub_TNs[4];

  dest_rc      = TN_register_class(dest);
  dest_rc_size = ISA_REGISTER_CLASS_INFO_Bit_Size(ISA_REGISTER_CLASS_Info(dest_rc)) / 8;
  dest_rsize   = TN_size(dest);
  rc_info      = EXTENSION_get_REGISTER_CLASS_info(dest_rc);

  // ---------------------------------------
  // --  Generate CLR if appropriate
  // ---------------------------------------
  if (TN_has_value(src) && TN_value(src)==0) {
    TOP top_clr;
    int top_clr_rsize; // reg size used by clr top
    GET_TOP_FOR_BIGGEST_AVAIL_REG_SIZE(/* IN_function       */ rc_info->get_clear_TOP,
				       /* IN_biggest_rsize  */ dest_rsize,
				       /* IN_smallest_rsize */ dest_rc_size,
				       /* OUT_top           */ top_clr,
				       /* OUT_rsize         */ top_clr_rsize);
    if (top_clr != TOP_UNDEFINED) {
      nb_access = dest_rsize / top_clr_rsize;
      if (nb_access == 1) {
	Build_OP(top_clr, dest, True_TN, ops);
      }
      else {
	int sub_size = dest_rsize/nb_access;
	for (i=0; i<nb_access; i++) {
	  // Create tempo sub reg
	  sub_TNs[i] = Gen_Register_TN(dest_rc, sub_size);
	  // Clear it
	  Build_OP(top_clr, sub_TNs[i], True_TN, ops);
	}
	// Compose the dest register
	if (nb_access == 2) {
	  Build_OP_simulated_compose(dest, True_TN, sub_TNs[0], sub_TNs[1], ops);
	} else {  // nb_access == 4
	  Build_OP_simulated_compose(dest, True_TN, sub_TNs[0], sub_TNs[1],
				     sub_TNs[2], sub_TNs[3], ops);
	}
      }
      return TRUE;
    }
  }

  /* After this point, we know we cannot use clear instructions */

  // ---------------------------------------
  // --  Create constant in GPR register
  // ---------------------------------------
  TN *gpr;
  gpr = Gen_Register_TN(ISA_REGISTER_CLASS_integer, 4);
  Expand_Immediate(gpr, src, is_signed, ops);


  // --------------------------------------
  // --  Move GPR to Extension register
  // --------------------------------------
  if (dest_rsize <= 4) {
    TOP top_r2x;
    top_r2x = rc_info->get_insert_and_zeroext_R2X_TOP(dest_rsize);
    if (top_r2x != TOP_UNDEFINED) {
      Build_OP(top_r2x, dest, True_TN, gpr, ops);
      return TRUE;
    }
  }
  else {
    TOP top_r2x;
    int top_r2x_rsize;
    GET_TOP_FOR_BIGGEST_AVAIL_REG_SIZE(/* IN_function       */ rc_info->get_insert_and_zeroext_R2X_TOP,
				       /* IN_biggest_rsize  */ dest_rsize,
				       /* IN_smallest_rsize */ dest_rc_size,
				       /* OUT_top           */ top_r2x,
				       /* OUT_rsize         */ top_r2x_rsize);
    if (top_r2x != TOP_UNDEFINED) {
      nb_access = dest_rsize / top_r2x_rsize;
      if (nb_access == 1) {
	Build_OP(top_r2x, dest, True_TN, gpr, ops);
	if (is_signed && (!TN_has_value(src) || TN_value(src)<0)) {
	  EXTENSION_Expand_SignExt(dest, dest, TRUE, ops);
	}
      }
      else {
	// Verify value can fit in move
	if (top_r2x_rsize < 4) {
	  if (TN_is_symbol(src)) {
	    DevWarn("Cannot insure that symbol value will be correctly copied into extension register");
	  }
	  else if ((!is_signed &&  TN_value(src) >= ( 1<< top_r2x_rsize)) ||
		   ( is_signed && (TN_value(src) >= ( 1<<(top_r2x_rsize-1)) ||
				   TN_value(src) <  (-1<<(top_r2x_rsize-1))))) {
	    sprintf(error_msg, "No way to generate value %d in extension register of class %s",
		    (int)TN_value(src),
		    ISA_REGISTER_CLASS_INFO_Name(ISA_REGISTER_CLASS_Info(dest_rc)));
	    ErrMsg(EC_Ext_Expand, error_msg);
	  }
	}

	// Create tempo sub reg
	sub_TNs[0] = Gen_Register_TN(dest_rc, dest_rsize/nb_access);
	Build_OP(top_r2x, sub_TNs[0], True_TN, gpr, ops);

	if (is_signed) {
	  EXTENSION_Expand_SignExt(dest, sub_TNs[0], TRUE, ops);
	} else {
	  EXTENSION_Expand_ZeroExt(dest, sub_TNs[0], ops);
	}
      }
      return TRUE;
    }
  }

  sprintf(error_msg, "[%s] Failed to expand immediate in extension register class",
	  EXTENSION_Get_Extension_Name_From_REGISTER_CLASS(dest_rc));
  ErrMsg(EC_Ext_Expand, error_msg);

#endif
  return FALSE;
}


/*
 * Real load expansion function, used to produce both (base + immediate)
 * and (base + reg).
 * Note: Auto-modified addressing mode are not generated here.
 *       This is handle later in the compilation flow (in EBO).
 */
static
BOOL EXTENSION_Expand_Load(AM_Base_Reg_Type base_type,
			   BOOL             offs_is_imm,
			   BOOL             offs_is_incr,
			   TN              *result,
			   TN              *base,
			   TN              *offset,
			   OPS             *ops,
			   UINT32           data_align) {
  FmtAssert(FALSE,("Not yet implemented"));
#if 0
  const extension_regclass_t *rc_info;
  TN *result_sub_tns[4];
  TN *tmp_offset;
  TN *tmp_base;
  INT64 tmp_val;
  TOP top_load, top_tmp;
  TOP top_misalign;
  INT nb_access;
  INT current_access_size;
  INT i;

  ISA_REGISTER_CLASS result_rc;
  INT result_rc_size;
  INT result_size;

  result_rc   = TN_register_class(result);
  result_size = TN_size(result);
  result_rc_size = ISA_REGISTER_CLASS_INFO_Bit_Size(ISA_REGISTER_CLASS_Info(result_rc)) / 8;

  rc_info = EXTENSION_get_REGISTER_CLASS_info(result_rc);

  // ##########################################
  // ## Retrieve the widest load TOP         ##
  // ## for current TN and define the number ##
  // ## of access required (1, 2 or 4 parts) ##
  // ##########################################
  // ...Search Full size load (1x)
  //    --------------------------
  top_misalign = TOP_UNDEFINED;
  top_load = rc_info->get_load_TOP(result_size, base_type, offs_is_imm, true/*offs_is_incr*/);

  if (top_load != TOP_UNDEFINED && !IsCompatibleAlignment(top_load, data_align)) {
    // ...Found it but alignment constraint too strong.
    //    Keep it in case nothing better is found
    top_misalign = top_load;
    top_load = TOP_UNDEFINED;
  }

  if (top_load == TOP_UNDEFINED &&
      (result_size == result_rc_size*2 || result_size == result_rc_size*4)) {
    // ...Search Half size load (2x)
    //    --------------------------
    top_load = rc_info->get_load_TOP(result_size/2, base_type, offs_is_imm, true/*offs_is_incr*/);

    if (top_load != TOP_UNDEFINED && !IsCompatibleAlignment(top_load, data_align)) {
      // ...Found it but alignment constraint too strong.
      if (top_misalign == TOP_UNDEFINED ||
	  (TOP_Mem_Alignment(top_load) < TOP_Mem_Alignment(top_misalign))) {
	// ...Keep it in case nothing better is found
	top_misalign = top_load;
      }
      top_load = TOP_UNDEFINED;
    }
    
    if (top_load == TOP_UNDEFINED &&
	(result_size == result_rc_size*4)) {
      // ...Search Quarter size load (4x)
      //    -----------------------------
      top_load = rc_info->get_load_TOP(result_size/4, base_type, offs_is_imm, true/*offs_is_incr*/);
      
      if (top_load != TOP_UNDEFINED && !IsCompatibleAlignment(top_load, data_align)) {
	// ...Found it but alignment constraint too strong.
	if (top_misalign == TOP_UNDEFINED ||
	    (TOP_Mem_Alignment(top_load) < TOP_Mem_Alignment(top_misalign))) {
	  // ...Keep it in case nothing better is found
	  top_misalign = top_load;
	}
	top_load = TOP_UNDEFINED;
      }
    }
  }

  if (top_load == TOP_UNDEFINED) {
    // No load found for current extension register!!
    // (except a possibly misaligned one, handled later)
    // Looking for an alternative
    if (result_size <= 4) {
      TOP top_insert = rc_info->get_insert_and_zeroext_R2X_TOP(result_size);
      if (top_insert != TOP_UNDEFINED) {
	// ----------------------------------------------------------------------------
	// RESCUE CODE 1:                           (not optimal but nearly never used)
	// LOAD to Core reg a 8, 16 or 32 bits data chunks and PUSH it to extension reg
	// ----------------------------------------------------------------------------
	int mtype;
	if (result_size == 4) {
	  mtype = MTYPE_U4;
	} else if (result_size == 2) {
	  mtype = MTYPE_U2;
	} else {
	  mtype = MTYPE_U1;
	}
	OPCODE opc = OPCODE_make_op(OPR_LDID, MTYPE_U4, mtype);
	TN *gpr = Gen_Register_TN(ISA_REGISTER_CLASS_integer, 4);
	Expand_Load(opc, gpr, base, offset, ops);
	Build_OP(top_insert, result, True_TN, gpr, ops);
	return TRUE;
      }
    }
    if (result_size >= 8) {
      TOP top_push_first = rc_info->get_insert_and_zeroext_RR2X_TOP(result_size);
      TOP top_push_next  = rc_info->get_rotate_and_insert_RR2X_TOP(result_size);
      if ((top_push_first != TOP_UNDEFINED) && (top_push_next != TOP_UNDEFINED)) {
	// --------------------------------------------------------------------------
	// RESCUE CODE 2:                         (not optimal but nearly never used)
	// LOAD to Core reg 64 bits data chunks (2x32) and PUSH them to extension reg
	// --------------------------------------------------------------------------
	OPCODE opc = OPCODE_make_op(OPR_LDID, MTYPE_U4, MTYPE_U4);
	
	nb_access = result_size/8; // count of 64 bits chunks
	
	// Set offset to the last address to access (reverse access)
	if (offs_is_imm) {
	  if (TN_is_symbol(offset)) {
	    offset = Gen_Symbol_TN(TN_var(offset),
				   TN_offset(offset) + (result_size - 4),
				   TN_relocs(offset));
	  }
	  else if (TN_has_value(offset)) {
	    offset = Gen_Literal_TN(TN_value(offset) + (result_size - 4), 4);
	  }
	  else {
	    FmtAssert(0, ("EXTENSION_Expand_Load(): unexpected constant"));
	  }
	}
	else {
	  tmp_base = Gen_Register_TN(TN_register_class(base), TN_size(base));
	  Build_OP(TOP_add_r, tmp_base, True_TN, base, offset, ops);
	  base = tmp_base;
	  offset = Gen_Literal_TN((result_size - 4), 4);
	}
	  
        TN *tmp_res      = NULL;
        TN *tmp_res_prev = NULL;

	for (i=0; i<nb_access; i++) {
	  TN *low  = Gen_Register_TN(ISA_REGISTER_CLASS_integer, 4);
	  TN *high = Gen_Register_TN(ISA_REGISTER_CLASS_integer, 4);
	  // ... load high 32 bits chunk
	  Expand_Load(opc, high, base, offset, ops);
	  if (TN_is_symbol(offset)) {
	    offset = Gen_Symbol_TN(TN_var(offset), TN_offset(offset) - 4, TN_relocs(offset));
	  } else {
	    offset = Gen_Literal_TN(TN_value(offset) - 4, 4);
	  }
	  // ... load load 32 bits chunk
	  Expand_Load(opc, low, base, offset, ops);
	  // ... move chunks to extension reg
          // ... we build progressively the final extension
          // ... register. The final TN is "result".
          // ... Other ones are built by duplicating "result" TN.

          tmp_res_prev = tmp_res;
          tmp_res = (i!=nb_access-1) ? Dup_TN(result) : result;

	  if (i == 0) { 
	    Build_OP(top_push_first, tmp_res, True_TN, high, low, ops);
	  } else {
	    Build_OP(top_push_next , tmp_res, True_TN, tmp_res_prev, high, low, ops);
	  }
         
	  // ... increment offset
	  if (TN_is_symbol(offset)) {
	    offset = Gen_Symbol_TN(TN_var(offset), TN_offset(offset) - 4, TN_relocs(offset));
	  } else {
	    offset = Gen_Literal_TN(TN_value(offset) - 4, 4);
	  }
	}
	return TRUE;
      }
    }
    if (result_size >= 4) {
      TOP top_push_first = rc_info->get_insert_and_zeroext_R2X_TOP(result_size);
      TOP top_push_next  = rc_info->get_rotate_and_insert_R2X_TOP(result_size);
      if ((top_push_first != TOP_UNDEFINED) && (top_push_next != TOP_UNDEFINED)) {
	// -------------------------------------------------------------------
	// RESCUE CODE 3:                  (not optimal but nearly never used)
	// LOAD to Core reg 32 bits data chunks and PUSH them to extension reg
	// -------------------------------------------------------------------
	OPCODE opc = OPCODE_make_op(OPR_LDID, MTYPE_U4, MTYPE_U4);
	  
	nb_access = result_size/4; // count of 32 bits chunks
	  
	// Set offset to the last address to access (reverse access)
	if (offs_is_imm) {
	  if (TN_is_symbol(offset)) {
	    offset = Gen_Symbol_TN(TN_var(offset), TN_offset(offset) + (result_size - 4), TN_relocs(offset));
	  }
	  else if (TN_has_value(offset)) {
	    offset = Gen_Literal_TN(TN_value(offset) + (result_size - 4), 4); // backward mem accesses
	  }
	  else {
	    FmtAssert(0, ("EXTENSION_Expand_Load(): unexpected constant"));
	  }
	}
	else {
	  tmp_base = Gen_Register_TN(TN_register_class(base), TN_size(base));
	  Build_OP(TOP_add_r, tmp_base, True_TN, base, offset, ops);
	  base = tmp_base;
	  offset = Gen_Literal_TN((result_size - 4), 4);
	}
	  
	for (i=0; i<nb_access; i++) {
	  TN *gpr = Gen_Register_TN(ISA_REGISTER_CLASS_integer, 4);
	  // ... load 32 bits chunk
	  Expand_Load(opc, gpr, base, offset, ops);
	  // ... push chunk to extension reg
	  if (i == 0) {
	    Build_OP(top_push_first, result, True_TN, gpr, ops);
	  } else {
	    Build_OP(top_push_next, result, True_TN, result, gpr, ops);
	  }
	  // ... increment offset
	  if (TN_is_symbol(offset)) {
	    offset = Gen_Symbol_TN(TN_var(offset), TN_offset(offset) - 4, TN_relocs(offset));
	  } else {
	    offset = Gen_Literal_TN(TN_value(offset) - 4, 4);
	  }
	}
	return TRUE;
      }
    }

    if (top_misalign != TOP_UNDEFINED) {
      // ------------------------------------
      // Available load is too constrained...
      // ------------------------------------
      char error_msg[256];
      sprintf(error_msg, "[%s] Failed to generate load on extension registers for "
	      "data aligned on %d bytes. Available one is aligned on %d bytes,"
	      " and lack of predefined instructions does not allow usage of core load.",
	      EXTENSION_Get_Extension_Name_From_REGISTER_CLASS(result_rc),
	      data_align, TOP_Mem_Alignment(top_misalign));
      ErrMsg(EC_Ext_Expand, error_msg);
      top_load = top_misalign;
    }
    else {
      char error_msg[256];
      sprintf(error_msg, "[%s] No load found in extension for register size %d."
	      "Lack of predefined instructions does not allow usage of core load.",
	      EXTENSION_Get_Extension_Name_From_REGISTER_CLASS(result_rc),
	      result_size);
      ErrMsg(EC_Ext_Expand, error_msg);
      return FALSE;
    }
  }

  nb_access = result_size/TOP_Mem_Bytes(top_load);

  // ###############################
  // ## Effective code generation ##
  // ###############################
  switch (nb_access) {
  case 1:
    // ===============================================================
    // Composite register size:  1
    // ===============================================================
    Build_OP(top_load, result, True_TN, base, offset, ops);
    return TRUE;
    
  case 2:
  case 4:
    // ===============================================================
    // Composite register size:  2, 4
    // ===============================================================    
    current_access_size = result_size/nb_access;

    // Create temporary sub-registers
    for (i=0; i<nb_access; i++) {
      result_sub_tns[i] = Gen_Register_TN(result_rc, current_access_size);
    }
    
    switch (base_type) {
    case AM_BASE_DEFAULT:
      if (offs_is_imm) {
	if (offs_is_incr) {
	  // -----------------------------------------------
	  // AddrMode:    (Rn + #imm_u8)
	  // -----------------------------------------------
	  //
	  // if (immediate value of all load addressing mode fits in u8)
	  //     load  low,  @( Rn + #u8 )
	  //     ...
	  //     load  high, @( Rn + #(u8 + (nb_access-1)*current_access_size) )
	  // else
	  //     add   Rt, Rn, #(nb_access*current_access_size)
	  //     load  low,  @( Rt + (#u8 - #(nb_access*current_access_size) ) )
	  //     ...
	  //     load  high, @( Rt + (#u8 - #current_access_size) )
	  //
	  FmtAssert(TN_is_constant(offset) &&
		    (TN_has_value(offset) || TN_is_symbol(offset)),
		    ("Unexpected offset type for current addressing mode"));

	  // Check offset range (only for value TN with positive offset)
	  if (TN_has_value(offset) && TN_value(offset)>0) {
	    tmp_val = TN_value(offset) + (INT64)(current_access_size*(nb_access-1));
	    if (!ISA_LC_Value_In_Class (tmp_val, LC_imm_u8)) {
	      // Not all offsets will fit in current addressing mode
	      // -> Increment base and decrement offset
	      INT64 dec = (INT64)(current_access_size*nb_access);

	      TN *tmp_offset = Dup_TN(offset);
	      Set_TN_value(tmp_offset, dec);

	      tmp_base = Gen_Register_TN(TN_register_class(base), TN_size(base));
	      Build_OP(TOP_add_i8, tmp_base, True_TN, base, tmp_offset, ops);
	      base = tmp_base;
	      
	      offset = Dup_TN(offset);
	      Set_TN_value(offset, TN_value(offset) - dec);
	    }
	  }
	  
	  // Generate the move sequence
	  for (i=0; i<nb_access; i++) {
	    if (i!=0) {
	      offset = Dup_TN(offset);
	      if (TN_has_value(offset)) {
		Set_TN_value(offset, TN_value(offset)+current_access_size);
	      }
	      else { // TN_is_symbol
		Set_TN_offset(offset, TN_offset(offset)+current_access_size);
	      }
	    }
	    Build_OP(top_load, result_sub_tns[i], True_TN, base, offset, ops);
	  }
	}
	else {  // !offs_is_incr
	  // -----------------------------------------------
	  // AddrMode:    (Rn - #imm_u8)
	  // -----------------------------------------------
	  //
	  // if (immediate offset remains positive for all loads)
	  //     load  low,  @( Rn - #u8 )
	  //     ...
	  //     load  high, @( Rn - #(u8 - (nb_access-1)*current_access_size) )
	  // else // it is needed to switch from negative displacement to positive
	  //      // one at some point
	  //     load  low,  @( Rn - #u8 )
	  //     ...
	  //     load  high, @( Rn + #(-(u8 - (nb_access-1)*current_access_size)) )
	  //
	  FmtAssert(TN_is_constant(offset) &&
		    (TN_has_value(offset) || TN_is_symbol(offset)),
		    ("Unexpected offset type for current addressing mode"));

	  if (TN_is_symbol(offset)) {
	    for (i=0; i<nb_access; i++) {
	      if (i!=0) {
		offset = Dup_TN(offset);
		Set_TN_offset(offset, TN_offset(offset)-current_access_size);
	      }
	      Build_OP(top_load, result_sub_tns[i], True_TN, base, offset, ops);
	    }
	  }
	  else { // TN_has_value(offset)
	    // Here we will check that offset remain positive
	    FmtAssert(TN_value(offset)>=0, ("Unexpected negative value"));
	    for (i=0; i<nb_access; i++) {
	      if (i!=0) {
		if (TN_value(offset)-current_access_size >= 0) {
		  offset = Dup_TN(offset);
		  Set_TN_value(offset, TN_value(offset)-current_access_size);
		}
		else {
		  // The offset is becoming negative, need to switch from AM_DISPL_NEG_U8
		  // to AM_DISPL_POS_U8
		  break;
		}
	      }
	      Build_OP(top_load, result_sub_tns[i], True_TN, base, offset, ops);
	    }
	    if (i<nb_access) {
	      // Still some access to generate with the other addressing mode
	      // Switch from AM_DISPL_NEG_U8 to AM_DISPL_POS_U8
	      top_tmp = rc_info->get_load_TOP(current_access_size, base_type, offs_is_imm, !offs_is_incr);
	      FmtAssert((top_tmp!=TOP_UNDEFINED), ("Cannot find expected load instruction!"));
	    
	      offset = Dup_TN(offset);
	      Set_TN_value(offset, -(TN_value(offset)-current_access_size));
	      
	      for (; i<nb_access; i++) {
		
		Build_OP(top_tmp, result_sub_tns[i], True_TN, base, offset, ops);
		
		if (i!=nb_access-1) {
		  offset = Dup_TN(offset);
		  Set_TN_value(offset, TN_value(offset)+current_access_size);
		}
	      }
	    }
	  }
	}
      }
      else {  // !offs_is_imm
	// -----------------------------------------------
	// AddrMode:    (Rn + Rp)
	// -----------------------------------------------
	//
	// add   Rt, Rn, Rp
	// load  low,  @( Rt + #0 )
	// ...
	// load  high, @( Rt + #(nb_access-1)*current_access_size )
	//
	tmp_base = Gen_Register_TN(TN_register_class(base), TN_size(base));
	Build_OP(TOP_add_r, tmp_base, True_TN, base, offset, ops);
	base = tmp_base;
	
	top_tmp = rc_info->get_load_TOP(current_access_size, AM_BASE_DEFAULT, 
					/*offs_is_imm*/true, /*offs_is_incr*/true);
	FmtAssert((top_tmp!=TOP_UNDEFINED), ("Cannot find expected load instruction!"));
	
	for (i=0; i<nb_access; i++) {
	  tmp_offset = Gen_Literal_TN(i*current_access_size, 4, 1);
	  Build_OP(top_tmp, result_sub_tns[i], True_TN, base, tmp_offset, ops);
	}
      }
      break;

    case AM_BASE_GP:
    case AM_BASE_SP:
      // -----------------------------------------------
      // AddrMode:    (GP + #imm_u12)
      //              (SP + #imm_u12)
      // -----------------------------------------------
      // (GP + #imm_u12)
      // ---------------
      // if (all immediate offset remains in U12 range)
      //     load  low,  @( R13 + #u12 )
      //     ...
      //     load  high, @( R13 + #(u12 + (nb_access-1)*current_access_size)) )
      // else
      //     make  Rt, #u12
      //     add  Rt, R13, Rt
      //     load  low,  @( Rt + #0 )
      //     ...
      //     load  high, @( Rt + (nb_access-1)*current_access_size)
      //
      // (SP + #imm_u12)
      // ---------------
      // (Same handling except that R13 is replace by R15)
      //
      FmtAssert(TN_is_constant(offset) && 
		(TN_has_value(offset) || TN_is_symbol(offset)),
		("Unexpected offset type for current addressing mode"));

      if (TN_is_symbol(offset)) {
	for (i=0; i<nb_access; i++) {
	  if (i!=0) {
	    offset = Dup_TN(offset);
	    Set_TN_offset(offset, TN_offset(offset)+current_access_size);
	  }
	  if ((base_type == AM_BASE_GP) && !TN_is_reloc_none(offset)) {
	    Set_GP_rel_reloc(offset, EXTENSION_REGISTER_CLASS_to_MTYPE(result_rc, current_access_size));
	  }
	  Build_OP(top_load, result_sub_tns[i], True_TN, base, offset, ops);
	}
      }
      else {  // TN_has_value(offset)
	tmp_val = TN_value(offset) + (INT64)((nb_access-1)*current_access_size);
	if ((tmp_val >= 0) && ISA_LC_Value_In_Class(tmp_val, LC_imm_u12)) {
	  for (i=0; i<nb_access; i++) {
	    if (i!=0) {
	      offset = Dup_TN(offset);
	      Set_TN_value(offset, TN_value(offset)+current_access_size);
	    }
	    if ((base_type == AM_BASE_GP) && !TN_is_reloc_none(offset)) {
	      Set_GP_rel_reloc(offset, EXTENSION_REGISTER_CLASS_to_MTYPE(result_rc, current_access_size));
	    }
	    Build_OP(top_load, result_sub_tns[i], True_TN, base, offset, ops);
	  }
	}
	else {
	  // Create a new base and update offset (to fit addressing mode offset range)
	  tmp_base   = Gen_Register_TN(TN_register_class(base), TN_size(base));
	  tmp_offset = Gen_Literal_TN(TN_value(offset), 4);
	  Expand_Immediate(tmp_base, tmp_offset, MTYPE_I4, ops);
	  Build_OP(TOP_add_r, tmp_base, True_TN, tmp_base, base, ops);
	  base = tmp_base;
	
	  // AM_DISPL_POS_U8
	  top_tmp = rc_info->get_load_TOP(current_access_size, AM_BASE_DEFAULT,
					  /*offs_is_imm*/true, /*offs_is_incr*/true);
	  FmtAssert((top_tmp!=TOP_UNDEFINED), ("Cannot find expected load instruction!"));
	
	  for (i=0; i<nb_access; i++) {
	    tmp_offset = Gen_Literal_TN(i*current_access_size, 4, 1);
	    Build_OP(top_tmp, result_sub_tns[i], True_TN, base, tmp_offset, ops);
	  }
	}
      }
      break;

    default:
      // -----------------------------------------------
      // AddrMode:    Unexpected base!!
      // -----------------------------------------------
      FmtAssert(0, ("Unexpected base register type"));
    }

    // Compose the result register
    if (nb_access == 2) {
      Build_OP_simulated_compose(result, True_TN, result_sub_tns[0], result_sub_tns[1], ops);
    }
    else {  // nb_access == 4
      Build_OP_simulated_compose(result, True_TN, result_sub_tns[0], result_sub_tns[1],
				 result_sub_tns[2], result_sub_tns[3], ops);
    }

    return TRUE;
    break;

  default:
    // ===============================================================
    // Composite register size:  Unexpected value!!
    // ===============================================================
    FmtAssert(0, ("EXTENSION_Expand_Load(): Unexpected composite register size: %d", nb_access));
  }
#endif
  return FALSE;
}


/*
 * Real store expansion function, used to produce both (base + immediate)
 * and (base + reg).
 * Note: Auto-modified addressing mode are not generated here.
 *       This is handle later in the compilation flow (in EBO).
 */
static
BOOL EXTENSION_Expand_Store(AM_Base_Reg_Type base_type,
			    BOOL             offs_is_imm,
			    BOOL             offs_is_incr,
			    TN              *src,
			    TN              *base,
			    TN              *offset,
			    OPS             *ops,
			   UINT32           data_align) {

  FmtAssert(FALSE,("Not yet implemented"));
#if 0

  const extension_regclass_t *rc_info;
  TN *src_sub_tns[4];
  TN *tmp_offset;
  TN *tmp_base;
  INT64 tmp_val;
  TOP top_store, top_tmp;
  TOP top_misalign;
  INT nb_access;
  INT current_access_size;
  INT i;

  ISA_REGISTER_CLASS src_rc;
  INT src_rc_size;
  INT src_size;

  src_rc   = TN_register_class(src);
  src_size = TN_size(src);
  src_rc_size = ISA_REGISTER_CLASS_INFO_Bit_Size(ISA_REGISTER_CLASS_Info(src_rc)) / 8;

  rc_info = EXTENSION_get_REGISTER_CLASS_info(src_rc);

  // ##########################################
  // ## Retrieve the widest store TOP        ##
  // ## for current TN and define the number ##
  // ## of access required (1, 2 or 4 parts) ##
  // ##########################################
  // ...Search Full size store (1x)
  //    ---------------------------
  top_misalign = TOP_UNDEFINED;
  top_store = rc_info->get_store_TOP(src_size, base_type, offs_is_imm, true/*offs_is_incr*/);

  if (top_store != TOP_UNDEFINED && !IsCompatibleAlignment(top_store, data_align)) {
    // ...Found it but alignment constraint too strong.
    //    Keep it in case nothing better is found
    top_misalign = top_store;
    top_store = TOP_UNDEFINED;
  }

  if (top_store == TOP_UNDEFINED &&
      (src_size == src_rc_size*2 || src_size == src_rc_size*4)) {
    // ...Search Half size store (2x)
    //    ---------------------------
    top_store = rc_info->get_store_TOP(src_size/2, base_type, offs_is_imm, true/*offs_is_incr*/);

    if (top_store != TOP_UNDEFINED && !IsCompatibleAlignment(top_store, data_align)) {
      // ...Found it but alignment constraint too strong.
      if (top_misalign == TOP_UNDEFINED ||
	  (TOP_Mem_Alignment(top_store) < TOP_Mem_Alignment(top_misalign))) {
	// ...Keep it in case nothing better is found
	top_misalign = top_store;

      }
      top_store = TOP_UNDEFINED;
    }

    if (top_store == TOP_UNDEFINED &&
	(src_size == src_rc_size*4)) {
      // ...Search Quarter size store (4x)
      //    ------------------------------
      top_store = rc_info->get_store_TOP(src_size/4, base_type, offs_is_imm, true/*offs_is_incr*/);

      if (top_store != TOP_UNDEFINED && !IsCompatibleAlignment(top_store, data_align)) {
	// ...Found it but alignment constraint too strong.
	if (top_misalign == TOP_UNDEFINED ||
	    (TOP_Mem_Alignment(top_store) < TOP_Mem_Alignment(top_misalign))) {
	  // ...Keep it in case nothing better is found
	  top_misalign = top_store;
	}
	top_store = TOP_UNDEFINED;
      }
    }
  }

  if (top_store == TOP_UNDEFINED) {
    // No store found for current extension register!!
    // (except a possibly misaligned one, handled later)
    // Looking for an alternative
    if (src_size <= 4) {
      TOP top_extract = rc_info->get_extract_X2R_TOP(src_size);
      if (top_extract != TOP_UNDEFINED) {
	// ---------------------------------------------------------------------
	// RESCUE CODE 1:                    (not optimal but nearly never used)
	// MOVE 16 or 32 bits from extension reg to Core reg and STORE it in mem
	// ---------------------------------------------------------------------
	TN *gpr = Gen_Register_TN(ISA_REGISTER_CLASS_integer, 4);
	Build_OP(top_extract, gpr, True_TN, src, ops);
	// ... store chunk in mem
	Expand_Store((src_size==1)?MTYPE_U1:((src_size==2)?MTYPE_U2:MTYPE_U4),
		     gpr, base, offset, ops);
	return TRUE;
      }
    }
    if (src_size >= 4) {
      TOP top_extract_rotate = rc_info->get_extract_and_rotate_X2R_TOP(src_size);
      if (top_extract_rotate != TOP_UNDEFINED) {
	// -----------------------------------------------------------------------
	// RESCUE CODE 2:                      (not optimal but nearly never used)
	// POP 32 bits chunks from extension reg to Core reg and STORE them in mem
	// -----------------------------------------------------------------------
	nb_access = src_size/4; // count of 32 bits chunks

	if (!offs_is_imm) {
	  // Create a new base = base + offset = Rn + Rp
	  tmp_base = Gen_Register_TN(TN_register_class(base), TN_size(base));
	  Build_OP(TOP_add_r, tmp_base, True_TN, base, offset, ops);
	  base = tmp_base;
	  offset = Gen_Literal_TN(0, 4);
	}
	
	// Copy the extension register in a tempo one
	// (the pop mechanism kills the initial reg value)
	TN *tmp_x = Gen_Register_TN(src_rc, src_size);
	
	for (i=0; i<nb_access; i++) {
	  TN *gpr = Gen_Register_TN(ISA_REGISTER_CLASS_integer, 4);
	  // ... move the 32 bits to core reg
	  if (i == 0) {
	    Build_OP(top_extract_rotate, gpr, tmp_x, True_TN, src, ops);
	  } else {
	    Build_OP(top_extract_rotate, gpr, tmp_x, True_TN, tmp_x, ops);
	  }
	  // ... store chunk in mem
	  Expand_Store(MTYPE_U4, gpr, base, offset, ops);
	  // ... increment offset
	  if (TN_is_symbol(offset)) {
	    offset = Gen_Symbol_TN(TN_var(offset), TN_offset(offset) + 4, TN_relocs(offset));
	  } else {
	    offset = Gen_Literal_TN(TN_value(offset) + 4, 4);
	  }
	}
	return TRUE;
      }
    }

    if (top_misalign != TOP_UNDEFINED) {
      // -------------------------------------
      // Available store is too constrained...
      // -------------------------------------
      char error_msg[256];
      sprintf(error_msg, "[%s] Failed to generate store from extension registers to "
	      "memory aligned on %d bytes. Available one is aligned on %d bytes,"
	      " and lack of predefined instructions does not allow usage of core store.",
	      EXTENSION_Get_Extension_Name_From_REGISTER_CLASS(src_rc),
	      data_align, TOP_Mem_Alignment(top_misalign));
      ErrMsg(EC_Ext_Expand, error_msg);
      top_store = top_misalign;
    }
    else {
      char error_msg[256];
      sprintf(error_msg, "[%s] No store found in extension for register size %d."
	      "Lack of predefined instructions does not allow usage of core store.",
	      EXTENSION_Get_Extension_Name_From_REGISTER_CLASS(src_rc),
	      src_size);
      ErrMsg(EC_Ext_Expand, error_msg);
      return FALSE;
    }
  }

  nb_access = src_size/TOP_Mem_Bytes(top_store);

  // ###############################
  // ## Effective code generation ##
  // ###############################
  switch (nb_access) {
  case 1:
    // ===============================================================
    // Composite register size:  1
    // ===============================================================
    Build_OP(top_store, True_TN, base, offset, src, ops);
    return TRUE;

  case 2:
  case 4:
    // ===============================================================
    // Composite register size:  2, 4
    // ===============================================================    
    current_access_size = src_size/nb_access;
    
    // Create temporary sub-registers
    for (i=0; i<nb_access; i++) {
      src_sub_tns[i] = Gen_Register_TN(src_rc, current_access_size);
    }
    if (nb_access == 2) {
      Build_OP_simulated_extract(src_sub_tns[0], src_sub_tns[1], True_TN, src, ops);
    }
    else {  // nb_access == 4
      Build_OP_simulated_extract(src_sub_tns[0], src_sub_tns[1],
				 src_sub_tns[2], src_sub_tns[3], True_TN, src, ops);
    }

    switch (base_type) {
    case AM_BASE_DEFAULT:
      if (offs_is_imm) {
	if (offs_is_incr) {
	  // -----------------------------------------------
	  // AddrMode:    (Rn + #imm_u8)
	  // -----------------------------------------------
	  //
	  // if (immediate value of all store addressing mode fits in u8)
	  //     store  low,  @( Rn + #u8 )
	  //     ...
	  //     store  high, @( Rn + #(u8 + (nb_access-1)*current_access_size) )
	  // else
	  //     add   Rt, Rn, #(nb_access*current_access_size)
	  //     store  low,  @( Rt + (#u8 - #(nb_access*current_access_size) )
	  //     ...
	  //     store  high, @( Rt + (#u8 - #current_access_size) )
	  FmtAssert(TN_is_constant(offset) && 
		    (TN_has_value(offset) || TN_is_symbol(offset)),
		    ("Unexpected offset type for current addressing mode"));

	  // Check offset range (only for value TN with positive offset)
	  if (TN_has_value(offset) && TN_value(offset)>0) {
	    tmp_val = TN_value(offset) + (INT64)(current_access_size*(nb_access-1));
	    if (!ISA_LC_Value_In_Class (tmp_val, LC_imm_u8)) {
	      // Not all offsets will fit in current addressing mode
	      // -> Increment base and decrement offset
	      INT64 dec = (INT64)(current_access_size*nb_access);

	      TN *tmp_offset = Dup_TN(offset);
	      Set_TN_value(tmp_offset, dec);

	      tmp_base = Gen_Register_TN(TN_register_class(base), TN_size(base));
	      Build_OP(TOP_add_i8, tmp_base, True_TN, base, tmp_offset, ops);
	      base = tmp_base;
	      
	      offset = Dup_TN(offset);
	      Set_TN_value(offset, TN_value(offset) - dec);
	    }
	  }

	  // Generate the move sequence
	  for (i=0; i<nb_access; i++) {
	    if (i!=0) {
	      offset = Dup_TN(offset);
	      if (TN_has_value(offset)) {
		Set_TN_value(offset, TN_value(offset)+current_access_size);
	      }
	      else { // TN_is_symbol
		Set_TN_offset(offset, TN_offset(offset)+current_access_size);
	      }
	    }
	    Build_OP(top_store, True_TN, base, offset, src_sub_tns[i], ops);
	  }
	}
	else {  // !offs_is_incr
	  // -----------------------------------------------
	  // AddrMode:    (Rn - #imm_u8)
	  // -----------------------------------------------
	  //
	  // if (immediate offset remains positive for all stores)
	  //     store  low,  @( Rn - #u8 )
	  //     ...
	  //     store  high, @( Rn - #(u8 - (nb_access-1)*current_access_size) )
	  // else // it is needed to switch from negative displacement to positive
	  //      // one at some point
	  //     store  low,  @( Rn - #u8 )
	  //     ...
	  //     store  high, @( Rn + #(-(u8 - (nb_access-1)*current_access_size)) )
	  FmtAssert(TN_is_constant(offset) &&
		    (TN_has_value(offset) || TN_is_symbol(offset)),
		    ("Unexpected offset type for current addressing mode"));

	  if (TN_is_symbol(offset)) {
	    // Consider that symbol offset remains positive
	    for (i=0; i<nb_access; i++) {
	      if (i!=0) {
		offset = Dup_TN(offset);
		Set_TN_value(offset, TN_value(offset)-current_access_size);
	      }
	      Build_OP(top_store, True_TN, base, offset, src_sub_tns[i], ops);
	    }
	  }
	  else { // TN_has_value(offset)
	    // Here we will check that offset remain positive
	    FmtAssert(TN_value(offset)>=0, ("Unexpected negative value"));
	    for (i=0; i<nb_access; i++) {
	      if (i!=0) {
		if (TN_value(offset)-current_access_size >= 0) {
		  offset = Dup_TN(offset);
		  Set_TN_value(offset, TN_value(offset)-current_access_size);
		}
		else {
		  // The offset is becoming negative, need to switch from AM_DISPL_NEG_U8
		  // to AM_DISPL_POS_U8
		  break;
		}
	      }
	      Build_OP(top_store, True_TN, base, offset, src_sub_tns[i], ops);
	    }
	    if (i<nb_access) {
	      // Still some access to generate with the other addressing mode
	      // Switch from AM_DISPL_NEG_U8 to AM_DISPL_POS_U8
	      top_tmp = rc_info->get_store_TOP(current_access_size, base_type, offs_is_imm, !offs_is_incr);
	      FmtAssert((top_tmp!=TOP_UNDEFINED), ("Cannot find expected store instruction!"));
	      
	      offset = Dup_TN(offset);
	      Set_TN_value(offset, -(TN_value(offset)-current_access_size));
	      
	      for (; i<nb_access; i++) {
		
		Build_OP(top_tmp, True_TN, base, offset, src_sub_tns[i], ops);
		
		if (i!=nb_access-1) {
		  offset = Dup_TN(offset);
		  Set_TN_value(offset, TN_value(offset)+current_access_size);
		}
	      }
	    }
	  }
	}
      }
      else {  // !offs_is_imm
	// -----------------------------------------------
	// AddrMode:    (Rn + Rp)
	// -----------------------------------------------
	//
	// add   Rt, Rn, Rp
	// store  low,  @( Rt + #0 )
	// ...
	// store  high, @( Rt + #(nb_access-1)*current_access_size )
	//
	tmp_base = Gen_Register_TN(TN_register_class(base), TN_size(base));
	Build_OP(TOP_add_r, tmp_base, True_TN, base, offset, ops);
	base = tmp_base;
	
	top_tmp = rc_info->get_store_TOP(current_access_size, AM_BASE_DEFAULT, 
					/*offs_is_imm*/true, /*offs_is_incr*/true);
	FmtAssert((top_tmp!=TOP_UNDEFINED), ("Cannot find expected store instruction!"));
	
	for (i=0; i<nb_access; i++) {
	  tmp_offset = Gen_Literal_TN(i*current_access_size, 4, 1);
	  Build_OP(top_tmp, True_TN, base, tmp_offset, src_sub_tns[i], ops);
	}
      }
      break;

    case AM_BASE_GP:
    case AM_BASE_SP:
      // -----------------------------------------------
      // AddrMode:    (GP + #imm_u12)
      //              (SP + #imm_u12)
      // -----------------------------------------------
      // (GP + #imm_u12)
      // ---------------
      // if (all immediate offset remains in U12 range)
      //     store  low,  @( R13 + #u12 )
      //     ...
      //     store  high, @( R13 + #(u12 + (nb_access-1)*current_access_size)) )
      // else
      //     make  Rt, #u12
      //     add  Rt, R13, Rt
      //     store  low,  @( Rt + #0 )
      //     ...
      //     store  high, @( Rt + (nb_access-1)*current_access_size)
      //
      // (SP + #imm_u12)
      // ---------------
      // (Same handling except that R13 is replace by R15)
      //
      FmtAssert(TN_is_constant(offset) && 
		(TN_has_value(offset) || TN_is_symbol(offset)),
		("Unexpected offset type for current addressing mode"));

      if (TN_is_symbol(offset)) {
	for (i=0; i<nb_access; i++) {
	  if (i!=0) {
	    offset = Dup_TN(offset);
	    Set_TN_offset(offset, TN_offset(offset)+current_access_size);
	  }
	  if ((base_type == AM_BASE_GP) && !TN_is_reloc_none(offset)) {
	    Set_GP_rel_reloc(offset, EXTENSION_REGISTER_CLASS_to_MTYPE(src_rc, current_access_size));
	  }
	  Build_OP(top_store, True_TN, base, offset, src_sub_tns[i], ops);
	}
      }
      else {  // TN_has_value(offset)
	tmp_val = TN_value(offset) + (INT64)((nb_access-1)*current_access_size);
	if ((tmp_val >= 0) && ISA_LC_Value_In_Class(tmp_val, LC_imm_u12)) {
	  for (i=0; i<nb_access; i++) {
	    if (i!=0) {
	      offset = Dup_TN(offset);
	      Set_TN_value(offset, TN_value(offset)+current_access_size);
	    }
	    if ((base_type == AM_BASE_GP) && !TN_is_reloc_none(offset)) {
	      Set_GP_rel_reloc(offset, EXTENSION_REGISTER_CLASS_to_MTYPE(src_rc, current_access_size));
	    }
	    Build_OP(top_store, True_TN, base, offset, src_sub_tns[i], ops);
	  }
	}
	else {
	  // Create a new base and update offset (to fit addressing mode offset range)
	  tmp_base   = Gen_Register_TN(TN_register_class(base), TN_size(base));
	  tmp_offset = Gen_Literal_TN(TN_value(offset), 4);
	  Expand_Immediate(tmp_base, tmp_offset, MTYPE_I4, ops);
	  Build_OP(TOP_add_r, tmp_base, True_TN, tmp_base, base, ops);
	  base = tmp_base;
	  
	  // AM_DISPL_POS_U8
	  top_tmp = rc_info->get_store_TOP(current_access_size, AM_BASE_DEFAULT,
					   /*offs_is_imm*/true, /*offs_is_incr*/true);
	  FmtAssert((top_tmp!=TOP_UNDEFINED), ("Cannot find expected load instruction!"));

	  for (i=0; i<nb_access; i++) {
	    tmp_offset = Gen_Literal_TN(i*current_access_size, 4, 1);
	    Build_OP(top_tmp, True_TN, base, tmp_offset, src_sub_tns[i], ops);
	  }
	}
      }
      break;

    default:
      // -----------------------------------------------
      // AddrMode:    Unexpected base!!
      // -----------------------------------------------
      FmtAssert(0, ("Unexpected base register type"));
    }
	
    return TRUE;
    break;
	
  default:
    // ===============================================================
    // Composite register size:  Unexpected value!!
    // ===============================================================
    FmtAssert(0, ("EXTENSION_Expand_Store(): Unexpected composite register size: %d", nb_access));
  }

#endif
  return FALSE;
}


/*
 * Expand load with immediate. Return FALSE if fail.
 */
BOOL EXTENSION_Expand_Load_Imm(AM_Base_Reg_Type base_type, TN *result, TN *base,
			       TN *ofst, OPS *ops, UINT32 data_align) {
  BOOL done;
  // Note: offs_is_incr (3rd argument) always set to true
  //       Hack to handle addressing modes as signed 9 bits.
  //       The target description defines the ld/st_i8 as signed 9 bits.
  //       This allows us to always use the inc form even for negative
  //       immediates.
  //       The rewriting to _inc or _dec form is done later in the code generator.
  //       This is a temporary scheme until we have a property for specifying
  //       that a load/store offset is substracted to the base and the code 
  //       generator is fixed to handle this.
  done = EXTENSION_Expand_Load(base_type, true, true, result, base, ofst, ops, data_align);
  return (done);
}


/*
 * Expand load with register. Return FALSE if fail.
 */
BOOL EXTENSION_Expand_Load_Reg(AM_Base_Reg_Type base_type, TN *result, TN *base,
			       TN *ofst, OPS *ops, UINT32 data_align) {
  BOOL done;
  done = EXTENSION_Expand_Load(base_type, false, true, result, base, ofst, ops, data_align);
  return (done);
}


/*
 * Expand store with immediate. Return FALSE if fail.
 */
BOOL EXTENSION_Expand_Store_Imm(AM_Base_Reg_Type base_type, TN *src, TN *base,
				TN *ofst, OPS *ops, UINT32 data_align) {
  BOOL done;
  // Note: offs_is_incr (3rd argument) always set to true
  //       Hack to handle addressing modes as signed 9 bits.
  //       The target description defines the ld/st_i8 as signed 9 bits.
  //       This allows us to always use the inc form even for negative
  //       immediates.
  //       The rewriting to _inc or _dec form is done later in the code generator.
  //       This is a temporary scheme until we have a property for specifying
  //       that a load/store offset is substracted to the base and the code 
  //       generator is fixed to handle this.
  done = EXTENSION_Expand_Store(base_type, true, true, src, base, ofst, ops, data_align);
  return (done);
}


/*
 * Expand store with register. Return FALSE if fail.
 */
BOOL EXTENSION_Expand_Store_Reg(AM_Base_Reg_Type base_type, TN *src, TN *base,
				TN *ofst, OPS *ops, UINT32 data_align) {
  BOOL done;
  done = EXTENSION_Expand_Store(base_type, false, true, src, base, ofst, ops, data_align);
  return (done);
}

