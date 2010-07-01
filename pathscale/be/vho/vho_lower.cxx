/*
 *  Copyright (C) 2007, 2008, 2009 PathScale, LLC. All Rights Reserved.
 */

/*
 *  Copyright (C) 2006, 2007. QLogic Corporation. All Rights Reserved.
 */

/*
 * Copyright 2003, 2004, 2005, 2006 PathScale, Inc.  All Rights Reserved.
 */

/*

  Copyright (C) 2000, 2001 Silicon Graphics, Inc.  All Rights Reserved.

   Path64 is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   Path64 is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with Path64; see the file COPYING.  If not, write to the Free
   Software Foundation, 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.

   Special thanks goes to SGI for their continued support to open source

*/


//-*-c++-*-

#define __STDC_LIMIT_MACROS
#include <stdint.h>

#ifndef FRONT_END

#include "defs.h"
#include "config.h"
#include "config_opt.h"
#include "mempool.h"
#include "wn.h"
#include "wn_util.h"
#include "opcode.h"
#include <stdlib.h>
#include "flags.h"
#include "pu_info.h"
#include "vho_lower.h"
#include "f90_lower.h"
#include "fb_whirl.h"
#include "wn_simp.h"
#include "topcode.h"
#include "callutil.h"
#include "targ_sim.h"
#include "cxx_template.h"
#include "prompf.h" 
#include "wb_f90_lower.h"
#include "wn_lower.h"
#ifdef KEY
#include "w2op.h"		// For OPCODE_Can_Be_Spculative
#endif

typedef enum {
  ADDRESS_USED,
  ADDRESS_PASSED,
  ADDRESS_SAVED
} ADDRESS_INFO_TYPE;

#endif /* FRONT_END */

#include "tracing.h"
#include "ir_reader.h"
#include "config_vho.h"
#include "targ_const.h"

#define DO_VHO_LOWERING 1
#define IS_POWER_OF_2(x) (((x)!=0) && ((x) & ((x)-1))==0)

#define ST_addr_taken_passed(st)       ST_addr_passed(st)
#define Set_ST_addr_taken_passed(st)   Set_ST_addr_passed(st)
#define Reset_ST_addr_taken_passed(st) Reset_ST_addr_passed(st)
#define ST_addr_taken_saved(st)        ST_addr_saved(st)
#define Set_ST_addr_taken_saved(st)    Set_ST_addr_saved(st)
#define Reset_ST_addr_taken_saved(st)  Reset_ST_addr_saved(st)

#pragma set woff 1172
#ifdef KEY
static INT current_pu_id = -1;
static void Misc_Loop_Fusion ( WN * , WN * );
#endif

static BOOL
VHO_WN_is_zero ( WN * wn )
{
  return (    (    WN_operator(wn) == OPR_INTCONST
                && WN_const_val(wn) == 0 ) 
           || (    WN_operator(wn) == OPR_CONST
                && Targ_Is_Zero(STC_val(WN_st(wn))) ) );
} /* VHO_WN_is_zero */


static BOOL
VHO_WN_has_side_effects ( WN * wn )
{
  INT32    nkids;
  INT32    i;
  OPERATOR wn_operator;
  BOOL     has_side_effects;

  wn_operator = WN_operator(wn);
  nkids       = WN_kid_count(wn);

  switch ( wn_operator ) {

    case OPR_LDID:

      has_side_effects = TY_is_volatile(WN_ty(wn));
      break;

    case OPR_ILOAD:

      has_side_effects =    TY_is_volatile(WN_ty(wn))
                         || VHO_WN_has_side_effects ( WN_kid0(wn) );  
      break;

    default:

      has_side_effects = FALSE;

      for ( i = 0; i < nkids; i++ ) {

        if ( VHO_WN_has_side_effects ( WN_kid(wn,i) ) ) {

          has_side_effects = TRUE;
          break;
        }
      }
      break;
  }

  return has_side_effects;
} /* VHO_WN_has_side_effects */


typedef struct bool_expr_info_t {
  OPCODE opcode;
  BOOL   used_true_label;
  BOOL   used_false_label;
  LABEL_IDX true_label;
  LABEL_IDX false_label;
} BOOL_INFO;

static WN * vho_lower ( WN * wn, WN * block );
static WN * vho_lower_stmt ( WN * stmt, WN * block );
static WN * vho_lower_block ( WN * wn );
#if defined( KEY) && !defined(TARG_ST)
static WN * vho_lower_expr ( WN * expr, WN * block, BOOL_INFO * bool_info, OPCODE stmt_opc = OPCODE_UNKNOWN );
#else
static WN * vho_lower_expr ( WN * expr, WN * block, BOOL_INFO * bool_info );
#endif

/* promote integers less than 4 bytes into their 4 byte counterparts in order 
 * to get the right type for OPCODE_make_op
 */
static TYPE_ID 
Promoted_Mtype(TYPE_ID mtype) {
  switch (mtype) {
  case MTYPE_I1:
  case MTYPE_I2: 
    return MTYPE_I4;
  case MTYPE_U1:
  case MTYPE_U2: 
    return MTYPE_U4;

    case MTYPE_I8: 
    case MTYPE_U8: 
    case MTYPE_I5:
    case MTYPE_U5:
    case MTYPE_A4:
    case MTYPE_A8:
    case MTYPE_F4:
    case MTYPE_F8:
    case MTYPE_FQ:
    case MTYPE_M:
    case MTYPE_C4:
    case MTYPE_C8:
    case MTYPE_CQ:
    case MTYPE_V:
    return mtype;

    default:
#ifdef TARG_ST
    // Reconf: support of dynamic mtypes
    if (MTYPE_is_dynamic(mtype)) {
        return mtype;
    }
#endif
    return MTYPE_UNKNOWN;

  }
} /* Promoted_Mtype */

#ifdef VHO_DEBUG
static BOOL      VHO_Switch_Debug = TRUE;
static BOOL      VHO_Struct_Debug = TRUE;
static char    * VHO_Switch_Format;
#endif /* VHO_DEBUG */

static SRCPOS    VHO_Srcpos;

/* Variables related to handling of switch statements */

INT32   VHO_Switch_Distance = 128;         /* max distance between values    */
INT32   VHO_Switch_Default_Label_Count = 16;
BOOL    VHO_Give_Preg_Names = FALSE;

static const char * vho_lower_mstore_name    = "__lower_mstore";
static const char * vho_lower_cand_cior_name = "__cand_cior";
static const char * vho_lower_cselect_name   = "__cselect";
static const char * vho_lower_rcomma_name    = "__rcomma";
static const char * vho_lower_comma_name     = "__comma";

static TY_IDX vho_u4a1_ty_idx = (TY_IDX) 0;
static TY_IDX vho_u8a1_ty_idx = (TY_IDX) 0;

typedef struct switch_case_table_entry_t {
  WN    * wn;
  INT32   count;
  FB_FREQ freq;
} VHO_SWITCH_ITEM;

static VHO_SWITCH_ITEM * VHO_Switch_Case_Table; /* table for switch cases    */

#define VHO_SWITCH_wn(i)    (VHO_Switch_Case_Table[i].wn)
#define VHO_SWITCH_count(i) (VHO_Switch_Case_Table[i].count)
#define VHO_SWITCH_freq(i)  (VHO_Switch_Case_Table[i].freq)

static BOOL      VHO_Switch_Signed;        /* signedness of switch index     */
static OPCODE    VHO_Switch_Int_Opcode;    /* opcode for Create_Int          */
static OPCODE    VHO_Switch_Sub_Opcode;    /* opcode for subtract            */
static OPCODE    VHO_Switch_EQ_Opcode;     /* opcode for == comparison       */
static OPCODE    VHO_Switch_LT_Opcode;     /* opcode for <  comparison       */
static OPCODE    VHO_Switch_GT_Opcode;     /* opcode for >  comparison       */

static LABEL_IDX VHO_Switch_Default_Label; /* default label in switch        */
static INT32     VHO_Switch_Last_Label;	   /* last label in switch           */
static INT32   * VHO_Switch_Cluster_Table; /* table for switch clusters      */
static INT32     VHO_Switch_Ncases;        /* # of cases in switch           */
static INT32     VHO_Switch_Nclusters;     /* # of clusters in switch        */
static WN      * VHO_Switch_Index;         /* load of switch index           */
static WN      * VHO_Switch_Default_Goto;  /* goto switch default label      */
static WN      * VHO_Switch_Stmt;          /* switch statement               */
static FB_FREQ   VHO_Switch_Default_Freq;  /* goto switch default freq       */

#ifdef KEY
static BOOL VHO_In_MP_Region_Pragma = FALSE;
#endif // KEY

/* Variables related to struct lowering */

#ifdef TARG_X8664
INT32  VHO_Struct_Limit = 8;               /* max # of fields/statements     */
#else
INT32  VHO_Struct_Limit = 4;               /* max # of fields/statements     */
#endif

// static INT32  VHO_Struct_Alignment;      /* Alignment of struct           */
static BOOL   VHO_Struct_Can_Be_Lowered;    /* FALSE if cannot be lowered    */
static INT32  VHO_Struct_Nfields;           /* # of fields                   */
static TY_IDX VHO_Struct_Fld_Table [255]; /* table containing fields         */
static INT32 VHO_Struct_Offset_Table [255]; /* table containing fields offset */
static INT32  VHO_Struct_Last_Field_Offset; /* offset of last field          */
static INT32  VHO_Struct_Last_Field_Size;   /* size of last field            */

#define LESS    -1
#define EQUAL    0
#define GREATER  1

#define SWITCH_key(i)  (VHO_Switch_Signed ? WN_const_val(VHO_SWITCH_wn(i)) : (UINT64) WN_const_val(VHO_SWITCH_wn(i)))

INT32
VHO_Switch_Compare_Value ( const void *v_item1, const void *v_item2 )
{
  INT32   compare_code;
  WN    * case1;
  WN    * case2;

  case1 = (( VHO_SWITCH_ITEM * ) v_item1)->wn;
  case2 = (( VHO_SWITCH_ITEM * ) v_item2)->wn;

  if ( VHO_Switch_Signed ) {

    if ( WN_const_val(case1) < WN_const_val(case2) )
      compare_code = LESS;

    else
    if ( WN_const_val(case1) > WN_const_val(case2) )
      compare_code = GREATER;

    else
      compare_code = EQUAL;
  }
  else {

    if ( (UINT64) WN_const_val(case1) < (UINT64) WN_const_val(case2) )
      compare_code = LESS;

    else
    if ( (UINT64) WN_const_val(case1) > (UINT64) WN_const_val(case2) )
      compare_code = GREATER;

    else
      compare_code = EQUAL;
  }

  return ( compare_code );
} /* VHO_Switch_Compare_Value */


/* ============================================================================
 *
 * INT32
 * VHO_Switch_Compare_Frequency ( const void *v_item1, const void *v_item2 )
 *
 * Routine invoked by qsort to compare two case values frequencies specified
 * the switch statement.
 *
 * ============================================================================
 */

INT32
VHO_Switch_Compare_Frequency ( const void *v_item1, const void *v_item2 )
{
  INT32   compare_code;
  WN    * case1;
  WN    * case2;
  FB_FREQ freq1;
  FB_FREQ freq2;

  case1  = (( VHO_SWITCH_ITEM * ) v_item1)->wn;
  case2  = (( VHO_SWITCH_ITEM * ) v_item2)->wn;
  freq1  = (( VHO_SWITCH_ITEM * ) v_item1)->freq;
  freq2  = (( VHO_SWITCH_ITEM * ) v_item2)->freq;

  if ( freq1 != freq2 ) {

    // Order of _decreasing_ frequencies

    if ( freq1 > freq2 )
      compare_code = LESS;

    else // freq1 < freq2
      compare_code = GREATER;

  } else
  if ( VHO_Switch_Signed ) {

    if ( WN_const_val(case1) < WN_const_val(case2) )
      compare_code = LESS;

    else
    if ( WN_const_val(case1) > WN_const_val(case2) )
      compare_code = GREATER;

    else
      compare_code = EQUAL;
  }

  else {

    if ( (UINT64) WN_const_val(case1) < (UINT64) WN_const_val(case2) )
      compare_code = LESS;

    else
    if ( (UINT64) WN_const_val(case1) > (UINT64) WN_const_val(case2) )
      compare_code = GREATER;

    else
      compare_code = EQUAL;
  }

  return ( compare_code );
} /* VHO_Switch_Compare_Frequency */


#if defined(KEY) && !defined(TARG_ST)
/* ============================================================================
 *
 * static WN *
 * VHO_Switch_Generate_If_Else_Reduce_Branch ( SRCPOS srcpos )
 *
 * Generate if-else sequence for the switch statement by grouping consecutive
 * cases together and using a single branch for each group.
 *
 * ============================================================================
 */
static WN *
VHO_Switch_Generate_If_Else_Reduce_Branch (SRCPOS srcpos)
{
  WN *block, *case_goto, *wn;
  INT32 i, j;
  INT32 switch_first_label;
  INT32 highest_label_num, lowest_label_num;
  WN **labels_map;

  block = WN_CreateBlock();
  WN_Set_Linenum(block, srcpos);

  // Find the highest and lowest case label numbers.
  highest_label_num = 0;
  lowest_label_num = INT32_MAX;
  for (i = 0; i < VHO_Switch_Ncases; i++) {
    case_goto = VHO_SWITCH_wn(i);
    INT64 value = WN_label_number(case_goto);
    if (value > highest_label_num)
      highest_label_num = value;
    if (value < lowest_label_num)
      lowest_label_num = value;
  }

  // Create case labels map.
  INT32 size = (highest_label_num - lowest_label_num + 1) * sizeof(WN *);
  labels_map = (WN **) alloca(size);
  memset(labels_map, 0, size);

  // Map each case label to the first label in the case code.  A case code will
  // have multiple labels if it is shared by different case values.
  WN *first_label_in_group = (WN *) NULL;
  for (wn = WN_next(VHO_Switch_Stmt); ; wn = WN_next(wn)) {
    if (WN_operator(wn) == OPR_LABEL) {
      if (first_label_in_group == NULL)
	first_label_in_group = wn;
      // Map only the labels in the switch stmt.
      if (WN_label_number(wn) >= lowest_label_num &&
	  WN_label_number(wn) <= highest_label_num) {
	labels_map[WN_label_number(wn)] = first_label_in_group;
      }
      if (WN_label_number(wn) == VHO_Switch_Last_Label)
	break;
    } else {
      // Real code terminates the labels group.
      first_label_in_group = NULL;
    }
  }

  // Sort the case values.
  qsort(VHO_Switch_Case_Table, VHO_Switch_Ncases,
	sizeof(VHO_SWITCH_ITEM), VHO_Switch_Compare_Value);

  for (i = 0; i < VHO_Switch_Ncases; i++) {
    WN *test, *sub, *tas;
    case_goto = VHO_SWITCH_wn(i);
    INT64 first_value = WN_const_val(case_goto);
    INT64 last_value = first_value;

    // Detect consecutive case values sharing common case code.
    for (j = i + 1; j < VHO_Switch_Ncases; j++) {
      WN *next_case_goto = VHO_SWITCH_wn(j);
      INT64 next_value = WN_const_val(next_case_goto);
      if (next_value != last_value + 1 ||
	  (labels_map[WN_label_number(next_case_goto)] !=
	   labels_map[WN_label_number(case_goto)]))
	break;
      last_value = next_value;
    }

    if (first_value == last_value) {
      // No consecutive case values.
      test = WN_CreateExp2(VHO_Switch_EQ_Opcode,
			   WN_COPY_Tree(VHO_Switch_Index),
			   WN_CreateIntconst(VHO_Switch_Int_Opcode,
					     first_value));
      wn = WN_CreateTruebr(WN_label_number(case_goto), test);
      WN_Set_Linenum(wn, srcpos);
    } else {
      // Consecutive case values sharing common case code.  Translate:
      //
      //   case k:
      //   case k+1:
      //    ...
      //   case k+n:	goto label
      //
      // into:
      //
      //   if ((unsigned int)(index - k) <= n)
      //     goto label
      //
      // If index is less than k, then (unsigned int)(index - k) will be
      // a large unsigned int larger than n.

      WN *upper =
	WN_CreateIntconst(VHO_Switch_Int_Opcode, last_value - first_value);
      TYPE_ID mtype = WN_rtype(VHO_Switch_Index);
      sub = WN_CreateExp2(VHO_Switch_Sub_Opcode,
			  WN_COPY_Tree(VHO_Switch_Index),
			  WN_CreateIntconst(VHO_Switch_Int_Opcode,
					    first_value));
      tas = WN_Tas(MTYPE_U4, MTYPE_To_TY(MTYPE_U4), sub);
      test = WN_CreateExp2(OPCODE_make_op(OPR_LE, MTYPE_U4, MTYPE_U4),
			   tas, upper);
      wn = WN_CreateTruebr(WN_label_number(case_goto), test);
      WN_Set_Linenum(wn, srcpos);
      i = j - 1;		// Skip to the next case value.
    }
    WN_INSERT_BlockAfter(block, WN_last(block), wn);
  }

  WN_INSERT_BlockAfter(block, WN_last(block),
                       WN_COPY_Tree(VHO_Switch_Default_Goto));
  return block;
}
#endif

/* ============================================================================
 *
 * static WN *
 * VHO_Switch_Generate_If_Else ( SRCPOS srcpos )
 *
 * Generate if-else sequence for the switch statement.
 *
 * ============================================================================
 */

static WN *
VHO_Switch_Generate_If_Else ( SRCPOS srcpos )
{
  WN     * block;
  WN     * value;
  WN     * test;
  WN     * case_goto;
  WN     * wn;
  INT32    i;
  FB_FREQ  freq_rest;

  block = WN_CreateBlock();
  WN_Set_Linenum ( block, srcpos );
#ifdef TARG_ST
    /* TB: bug for switch without any cases */
  if ( Cur_PU_Feedback && (VHO_Switch_Ncases != 0)) {
#else
  if ( Cur_PU_Feedback ) {
#endif
    qsort ( VHO_Switch_Case_Table, VHO_Switch_Ncases,
            sizeof(VHO_SWITCH_ITEM), VHO_Switch_Compare_Frequency );

    freq_rest = VHO_Switch_Default_Freq;
    for ( i = 0; i < VHO_Switch_Ncases; i++ ) {
      freq_rest += VHO_SWITCH_freq(i);
    }

  }

  for ( i = 0; i < VHO_Switch_Ncases; i++ ) {

    case_goto = VHO_SWITCH_wn(i);
    value     = WN_CreateIntconst ( VHO_Switch_Int_Opcode,
                                    WN_const_val(case_goto) );
    test      = WN_CreateExp2 ( VHO_Switch_EQ_Opcode,
                                WN_COPY_Tree ( VHO_Switch_Index ), value );
    wn        = WN_CreateTruebr ( WN_label_number(case_goto), test );
    WN_Set_Linenum ( wn, srcpos );

    if ( Cur_PU_Feedback ) {
      freq_rest -= VHO_SWITCH_freq(i);
      Cur_PU_Feedback->Annot( wn, FB_EDGE_BRANCH_TAKEN, VHO_SWITCH_freq(i) );
      Cur_PU_Feedback->Annot( wn, FB_EDGE_BRANCH_NOT_TAKEN, freq_rest );
    }

    WN_INSERT_BlockAfter ( block, WN_last(block), wn );
  }
#ifdef TARG_ST
    /* TB: bug for switch without any cases */
  if ( Cur_PU_Feedback && (VHO_Switch_Ncases != 0))
#else
  if ( Cur_PU_Feedback && (VHO_Switch_Ncases > 0))
#endif
    Cur_PU_Feedback->Annot( wn, FB_EDGE_BRANCH_NOT_TAKEN,
			    VHO_Switch_Default_Freq );

  WN_INSERT_BlockAfter ( block, WN_last(block),
                         WN_COPY_Tree ( VHO_Switch_Default_Goto ) );

  return ( block );
} /* VHO_Switch_Generate_If_Else */
/* ============================================================================
 *
 * static WN *
 * VHO_Switch_Opt_Case_Hoist ( SRCPOS srcpos )
 *
 * Generate if-else sequence for the highly biased cases.
 * ============================================================================
 */
static WN *
VHO_Switch_Opt_Case_Hoist( SRCPOS srcpos )
{
  WN     * block;
  WN     * value;
  WN     * test;
  WN     * case_goto;
  WN     * wn;
  INT32    i;
  FB_FREQ  freq_total, freq_rest, freq_top;
  FB_FREQ  freq_tops; // record only top 30 cases of frequencies
  INT32    num_hoist=0;

  if (!Cur_PU_Feedback ) return NULL;  // no switch optimization if no feedback

#ifdef KEY
  freq_tops = freq_total = 0.0;
#else
  freq_tops = freq_total = 0;
#endif

  block = WN_CreateBlock();
  WN_Set_Linenum ( block, srcpos );

  if ( Cur_PU_Feedback ) {
    qsort ( VHO_Switch_Case_Table, VHO_Switch_Ncases,
            sizeof(VHO_SWITCH_ITEM), VHO_Switch_Compare_Frequency );

    freq_total = VHO_Switch_Default_Freq;
    for ( i = 0; i < VHO_Switch_Ncases; i++ ) {
      freq_total += VHO_SWITCH_freq(i);

      if (i < VHO_Switch_Ncases * 0.3) // top 30 % of cases
          freq_tops += VHO_SWITCH_freq(i);
    }
  }
  /* If lower than 70%, don't do switch optimization */
  if (freq_total.Value()<=0 || freq_tops.Value()/freq_total.Value() < 0.70)
      return NULL;


  freq_rest = freq_total;

  for ( i = 0; i < VHO_Switch_Ncases; i++ ) {

    if ( Cur_PU_Feedback &&  VHO_SWITCH_freq(i).Value()/freq_total.Value()
                              < (float) VHO_Switch_Opt_Threshold/100)
                break;
#ifdef VHO_DEBUG
    if ( VHO_Switch_Debug )
      fprintf ( TFile, "SWITCH_OPT_CASE_HOIST: %d (case %d of freq %f)\n", (INT32) srcpos, i, VHO_SWITCH_freq(i).Value() );
#endif /* VHO_DEBUG */

//    printf("WARNING: SWITCH_OPT_CASE_HOIST: %d (case %d of freq %f total:%f)\n",(INT32) srcpos, i, VHO_SWITCH_freq(i).Value(), freq_total.Value());

    num_hoist++;
    case_goto = VHO_SWITCH_wn(i);
    value     = WN_CreateIntconst ( VHO_Switch_Int_Opcode,
                                    WN_const_val(case_goto) );
    test      = WN_CreateExp2 ( VHO_Switch_EQ_Opcode,
                                WN_COPY_Tree ( VHO_Switch_Index ), value );
    wn = WN_CreateTruebr ( WN_label_number(case_goto), test );

    WN_Set_Linenum ( wn, srcpos );


    if ( Cur_PU_Feedback ) {
      freq_rest -= VHO_SWITCH_freq(i);
      Cur_PU_Feedback->Annot_branch( wn, FB_Info_Branch( VHO_SWITCH_freq(i),
                        freq_rest, WN_operator( wn ) ) );
      Cur_PU_Feedback->FB_hoist_case(VHO_Switch_Stmt, i);
#ifdef KEY
      VHO_SWITCH_freq(i) = 0.0;
#else
      VHO_SWITCH_freq(i) = 0;
#endif
     }

    WN_INSERT_BlockAfter ( block, WN_last(block), wn );
  }

  /* Remove all the cases that are hoisted */
  for ( i = 0; i < VHO_Switch_Ncases; i++ ) {
        VHO_SWITCH_wn(i) = VHO_SWITCH_wn(i + num_hoist);
        VHO_SWITCH_freq(i) = VHO_SWITCH_freq(i + num_hoist);
        VHO_SWITCH_count(i) = VHO_SWITCH_count(i+num_hoist);

  }
  VHO_Switch_Ncases = VHO_Switch_Ncases - num_hoist;
  VHO_SWITCH_wn(VHO_Switch_Ncases) = VHO_SWITCH_wn(VHO_Switch_Ncases - 1);

  return ( block );

} /* VHO_Switch_Opt_Case_Hoist */


/* ============================================================================
 *
 * static WN *
 * VHO_Switch_Generate_Compgoto ( SRCPOS srcpos )
 *
 * Generate a COMPGOTO for the switch statement.
 *
 * ============================================================================
 */

static WN *
VHO_Switch_Generate_Compgoto ( SRCPOS srcpos )
{
  WN      * block;
  WN      * test;
  WN      * value;
  WN      * wn;
  WN      * case_goto;
  INT32     i;
  INT32     j;
  INT32     n;
  INT64     case_value;
  INT64     curr_value;
  FB_FREQ * freq_new = NULL;

  block = WN_CreateBlock ();
  WN_Set_Linenum ( block, srcpos );

  case_goto  = VHO_SWITCH_wn(0);
  case_value = WN_const_val(case_goto);
  curr_value = 0;
  test       = WN_COPY_Tree ( VHO_Switch_Index );

  /* Check to see whether we can avoid generating a subtract. */

  if (     case_value
       && (UINT64)case_value > VHO_Switch_Compgoto_Limit ) {

    value      = WN_CreateIntconst ( VHO_Switch_Int_Opcode,
                                     WN_const_val(case_goto) );
    test       = WN_CreateExp2 ( VHO_Switch_Sub_Opcode, test, value );
    curr_value = case_value;
  }

  i = 0;
  n =   WN_const_val(VHO_SWITCH_wn(VHO_Switch_Ncases - 1))
      - curr_value + 1;

  if ( Cur_PU_Feedback )
    freq_new = TYPE_MEM_POOL_ALLOC_N( FB_FREQ, MEM_local_pool_ptr, n );

  for ( j = 0; j < n; j++ ) {

    if ( curr_value != case_value ) {
      wn = WN_COPY_Tree ( VHO_Switch_Default_Goto );
      if ( Cur_PU_Feedback )
//      freq_new[j] = FB_FREQ_UNKNOWN;
        freq_new[j] = FB_FREQ(FB_FREQ_TYPE_GUESS);  //INLINING_TUNINING
                             
    } else {

      wn = WN_CreateGoto ( (ST_IDX) NULL, WN_label_number(case_goto) );
      if ( Cur_PU_Feedback )
	freq_new[j] = VHO_SWITCH_freq(i);
      i++;
      case_goto   = VHO_SWITCH_wn(i);
      case_value  = WN_const_val(case_goto);
    }

    WN_Set_Linenum ( wn, srcpos );

    WN_INSERT_BlockAfter ( block, WN_last(block), wn );
    curr_value++;
  }

  wn = WN_CreateCompgoto ( n, test, block, VHO_Switch_Default_Goto,
                           VHO_Switch_Last_Label );

  if ( Cur_PU_Feedback ) {

    FB_Info_Switch info_switch( n );
    info_switch[ FB_EDGE_SWITCH_INDEX( FB_EDGE_SWITCH_DEFAULT ) ]
//      = FB_FREQ_UNKNOWN; // not VHO_Switch_Default_Freq
        = FB_FREQ(VHO_Switch_Default_Freq.Value(), FALSE);//INLINING_TUNINING  
    for ( j = 0; j < n; j++ )
      info_switch[ FB_EDGE_SWITCH_INDEX( FB_EDGE_SWITCH( j ) ) ] = freq_new[j];
    Cur_PU_Feedback->Annot_switch( wn, info_switch );

    MEM_POOL_FREE( MEM_local_pool_ptr, freq_new );
  }

  return wn;
} /* VHO_Switch_Generate_Compgoto */


/* ============================================================================
 *
 * static void
 * VHO_Switch_Generate_Binary_Search ( INT32 left, INT32 right, WN * block )
 *
 * Implement the switch statement as a binary search.
 * left and right give the indices in VHO_Switch_Case_Table which
 * contain the values to be compared with.
 * block contains the WHIRL block node to which statements are to be added.
 *
 * ============================================================================
 */

static void
VHO_Switch_Generate_Binary_Search ( INT32 left, INT32 right, WN * block )
{
  INT32    mid;
  INT64    value;
  WN     * test;
  WN     * wn;
  SRCPOS   srcpos;

  LABEL_IDX lt_label;
  LABEL_IDX gt_label;

  srcpos = WN_Get_Linenum(block);

  mid   = ( left + right + 1 ) >> 1;
  value = WN_const_val(VHO_SWITCH_wn(mid));


  if (mid > left)
    LABEL_Init (New_LABEL (CURRENT_SYMTAB, lt_label), 0, LKIND_DEFAULT);
  else
    lt_label = VHO_Switch_Default_Label;

  if (mid < right)
    LABEL_Init (New_LABEL (CURRENT_SYMTAB, gt_label), 0, LKIND_DEFAULT);
  else
    gt_label = VHO_Switch_Default_Label;

  if ( lt_label == gt_label ) {

    test = WN_CreateExp2 ( VHO_Switch_EQ_Opcode,
                           WN_COPY_Tree ( VHO_Switch_Index ),
                           WN_CreateIntconst ( VHO_Switch_Int_Opcode,
                                               value ) );
    wn = WN_CreateFalsebr ( lt_label, test );
    WN_Set_Linenum ( wn, srcpos );
    if ( Cur_PU_Feedback ) {
      Cur_PU_Feedback->Annot( wn, FB_EDGE_BRANCH_TAKEN, FB_FREQ_UNKNOWN );
      Cur_PU_Feedback->Annot( wn, FB_EDGE_BRANCH_NOT_TAKEN,
			      VHO_SWITCH_freq( mid ) );
    }

    WN_INSERT_BlockAfter ( block, WN_last(block), wn );
  }

  else {

    test = WN_CreateExp2 ( VHO_Switch_LT_Opcode,
                           WN_COPY_Tree ( VHO_Switch_Index ),
                           WN_CreateIntconst ( VHO_Switch_Int_Opcode,
                                               value ) );
    wn = WN_CreateTruebr ( lt_label, test );
    WN_Set_Linenum ( wn, srcpos );
    if ( Cur_PU_Feedback ) {
      Cur_PU_Feedback->Annot( wn, FB_EDGE_BRANCH_TAKEN, FB_FREQ_UNKNOWN );
      Cur_PU_Feedback->Annot( wn, FB_EDGE_BRANCH_NOT_TAKEN,
			      FB_FREQ_UNKNOWN );
    }

    WN_INSERT_BlockAfter ( block, WN_last(block), wn );

    test = WN_CreateExp2 ( VHO_Switch_GT_Opcode,
                           WN_COPY_Tree ( VHO_Switch_Index ),
                           WN_CreateIntconst ( VHO_Switch_Int_Opcode,
                                               value ) );
    wn = WN_CreateTruebr ( gt_label, test );
    WN_Set_Linenum ( wn, srcpos );
    if ( Cur_PU_Feedback ) {
      Cur_PU_Feedback->Annot( wn, FB_EDGE_BRANCH_TAKEN, FB_FREQ_UNKNOWN );
      Cur_PU_Feedback->Annot( wn, FB_EDGE_BRANCH_NOT_TAKEN,
			      VHO_SWITCH_freq( mid ) );
    }

    WN_INSERT_BlockAfter ( block, WN_last(block), wn );
  }

  wn = WN_CreateGoto ( (ST_IDX) NULL, WN_label_number(VHO_SWITCH_wn(mid)) );
  WN_Set_Linenum ( wn, srcpos );
  if ( Cur_PU_Feedback )
    Cur_PU_Feedback->Annot( wn, FB_EDGE_OUTGOING, VHO_SWITCH_freq( mid ) );

  WN_INSERT_BlockAfter ( block, WN_last(block), wn );
  
  if ( mid > left ) {

    wn = WN_CreateLabel ( (ST_IDX) 0, lt_label, 0, NULL );
    WN_Set_Linenum ( wn, srcpos );

    WN_INSERT_BlockAfter ( block, WN_last(block), wn );
    VHO_Switch_Generate_Binary_Search ( left, mid - 1, block );
  }

  if ( mid < right ) {

    wn = WN_CreateLabel ( (ST_IDX) 0, gt_label, 0, NULL );
    WN_Set_Linenum ( wn, srcpos );

    WN_INSERT_BlockAfter ( block, WN_last(block), wn );
    VHO_Switch_Generate_Binary_Search ( mid + 1, right, block );
  }
} /* VHO_Switch_Generate_Binary_Search */

/* ============================================================================
 *
 * static void
 * VHO_Switch_Find_Clusters ( void )
 *
 * Group the switch values into clusters.
 * For each cluster a COMPGOTO cna be generated.
 *
 * ============================================================================
 */

static void 
VHO_Switch_Find_Clusters ( void )
{
  INT32   i;
  INT32   j;
  INT32   k;
  INT32   n;

  qsort ( VHO_Switch_Case_Table, VHO_Switch_Ncases,
          sizeof(VHO_SWITCH_ITEM), VHO_Switch_Compare_Value );

#ifdef VHO_DEBUG
  if ( VHO_Switch_Debug ) {

    fprintf ( TFile, "SWITCH TABLE after sorting\n" );

    for ( i = 0; i < VHO_Switch_Ncases; i++ ) {

      case_goto = VHO_SWITCH_wn(i);
      fprintf ( TFile, VHO_Switch_Format, i,
                WN_const_val(case_goto), WN_label_number(case_goto) );
    }
  }
#endif /* VHO_DEBUG */

  j = 0;
  VHO_Switch_Cluster_Table [0] = 0;

  for ( i = 1; i < VHO_Switch_Ncases; i++ ) {

    if (    ( i - VHO_Switch_Cluster_Table [j] ) * VHO_Switch_Density
         >= SWITCH_key(i) - SWITCH_key(VHO_Switch_Cluster_Table[j]) ) {

      if (j > 0) {

        if (    ( i - VHO_Switch_Cluster_Table [j-1] ) * VHO_Switch_Density
             >= SWITCH_key(i) - SWITCH_key(VHO_Switch_Cluster_Table[j-1]) )
          j--;
      }
    }

    else
      VHO_Switch_Cluster_Table [++j] = i;
  }

  VHO_Switch_Cluster_Table [++j] = VHO_Switch_Ncases;

  VHO_Switch_Nclusters = 0;

  for ( i = 0; i < j; i++ ) {

    if (   VHO_Switch_Cluster_Table [i+1] - VHO_Switch_Cluster_Table [i]
         > VHO_Switch_If_Else_Limit )
      VHO_Switch_Nclusters++;

    else
      VHO_Switch_Nclusters +=
        VHO_Switch_Cluster_Table [i+1] - VHO_Switch_Cluster_Table [i];
  }

  if ( VHO_Switch_Nclusters > j ) {

    i = VHO_Switch_Nclusters;

    while ( j > 0 ) {

      n = VHO_Switch_Cluster_Table [j] - VHO_Switch_Cluster_Table [j-1];

      if ( n > VHO_Switch_If_Else_Limit )
        VHO_Switch_Cluster_Table [i--] = VHO_Switch_Cluster_Table [j];

      else {

        for ( k = 0; k < n; k++ )
          VHO_Switch_Cluster_Table [i--] = VHO_Switch_Cluster_Table [j] - k;

        if ( i < j )
          break;
      }

      --j;
    }
  }

#ifdef VHO_DEBUG
  if ( VHO_Switch_Debug ) {

    fprintf (TFile, "  VHO_Switch_Nclusters = %d\n", VHO_Switch_Nclusters);

    for ( i = 0; i <= VHO_Switch_Nclusters; i++ )
      fprintf (TFile, "  VHO_Switch_Cluster_Table [%2d] = %2d\n",
               i, VHO_Switch_Cluster_Table [i]);
  }
#endif /* VHO_DEBUG */
} /* VHO_Switch_Find_Clusters */

/* ============================================================================
 *
 * static WN *
 * VHO_Lower_Switch ( WN * wn )
 *
 * Lower a switch statement into a combination of
 *
 *   if-else
 *   COMPGOTO
 *   binary search.
 *
 * ============================================================================
 */

static WN *
VHO_Lower_Switch ( WN * wn )
{
  WN       * block;
  WN       * case_goto;
  TYPE_ID    mtype;
  INT32      i;
  INT32      j;
  SRCPOS     srcpos;
  INT32      count;
  #ifdef TARG_ST
  // VL: Needed to control switch expansion in Os
  INT64      first_case_value = INT64_MAX;
  INT64      last_case_value  = INT64_MIN;
  INT32      switch_density;
#endif
  WN       * conv_wn = NULL;

  LABEL_IDX  last_label;

  srcpos = WN_Get_Linenum(wn);
  block  = WN_kid1(wn);

  VHO_Switch_Stmt         = wn;
  VHO_Switch_Index        = WN_kid0(wn);
  VHO_Switch_Default_Goto = ( WN_kid_count(wn) == 3 ) ? WN_kid2(wn) : NULL;
  VHO_Switch_Last_Label   = WN_last_label(wn);

  mtype = WN_rtype (VHO_Switch_Index);

  VHO_Switch_Int_Opcode = OPCODE_make_op ( OPR_INTCONST,
                                           Promoted_Mtype (mtype),
                                           MTYPE_V );
  VHO_Switch_Sub_Opcode = OPCODE_make_op ( OPR_SUB,
                                           Promoted_Mtype (mtype),
                                           MTYPE_V );
  VHO_Switch_EQ_Opcode  = OPCODE_make_op ( OPR_EQ,
                                           MTYPE_I4,
                                           Promoted_Mtype (mtype) );
  VHO_Switch_LT_Opcode  = OPCODE_make_op ( OPR_LT,
                                           MTYPE_I4,
                                           Promoted_Mtype (mtype) );
  VHO_Switch_GT_Opcode  = OPCODE_make_op ( OPR_GT,
                                           MTYPE_I4,
                                           Promoted_Mtype (mtype) );

#ifdef VHO_DEBUG
  if ( VHO_Switch_Debug )
    fprintf ( TFile, "SWITCH TABLE before sorting\n" );
#endif /* VHO_DEBUG */

  switch ( mtype ) {

  case MTYPE_I1:
  case MTYPE_I2:
  case MTYPE_I4:
  case MTYPE_I8:

#ifdef VHO_DEBUG
    VHO_Switch_Format = "  [%2d] value = %" SCNd64 ", label = %d\n";
#endif /* VHO_DEBUG */

    VHO_Switch_Signed = TRUE;
    break;

  case MTYPE_U1:
  case MTYPE_U2:
  case MTYPE_U4:
  case MTYPE_U8:

#ifdef VHO_DEBUG
    VHO_Switch_Format = "  [%2d] value = %" SCNd64 ", label = %d\n";
#endif /* VHO_DEBUG */

    VHO_Switch_Signed = FALSE;
    break;
  }

  for ( VHO_Switch_Ncases = 0, case_goto = WN_first(block);
        case_goto;
        VHO_Switch_Ncases++, case_goto = WN_next(case_goto) ) {

#ifdef VHO_DEBUG
    if ( VHO_Switch_Debug )
      fprintf ( TFile, VHO_Switch_Format, VHO_Switch_Ncases,
		WN_const_val(case_goto), WN_label_number(case_goto) );
#endif /* VHO_DEBUG */

  }

  if ( Cur_PU_Feedback ) {
    VHO_Switch_Default_Freq
      = ( VHO_Switch_Default_Goto
	  ? Cur_PU_Feedback->Query( wn, FB_EDGE_SWITCH_DEFAULT )
	  : FB_FREQ_ZERO );
  }

  VHO_Switch_Case_Table =
    TYPE_MEM_POOL_ALLOC_N(VHO_SWITCH_ITEM,
                          MEM_local_pool_ptr, (VHO_Switch_Ncases+1));
  VHO_Switch_Cluster_Table =
    TYPE_MEM_POOL_ALLOC_N(INT32, MEM_local_pool_ptr, (VHO_Switch_Ncases+1));

  last_label = INT32_MIN;

  for ( i = 0, case_goto = WN_first(block);
        i < VHO_Switch_Ncases;
        i++, case_goto = WN_next(case_goto) ) {

    VHO_SWITCH_wn(i) = case_goto;
    if ( Cur_PU_Feedback )
      VHO_SWITCH_freq(i) = Cur_PU_Feedback->Query( wn, FB_EDGE_SWITCH( i ) );

    if ( WN_label_number(case_goto) == last_label )
      count++;

    else {

      for ( j = i - 1; j >= 0; --j ) {

        if ( last_label == WN_label_number(VHO_SWITCH_wn(j)) )
          VHO_SWITCH_count(j) = count;

        else
          break;
      }

      count = 1;
      last_label = WN_label_number(case_goto);
    }
  }

  for ( j = i - 1; j >= 0; --j ) {

    if ( last_label == WN_label_number(VHO_SWITCH_wn(j)) )
      VHO_SWITCH_count(j) = count;

    else
      break;
  }

  VHO_SWITCH_wn(VHO_Switch_Ncases) = VHO_SWITCH_wn(VHO_Switch_Ncases - 1);

  for ( i = 0; i < VHO_Switch_Ncases; i++ ) {

    case_goto = VHO_SWITCH_wn(i);
  }

#ifdef TARG_STxP70
  // VL 2007/09/12: 
  // To address art #26891 and limit size expansion in -Os we precompute 
  // actual switch density. Currently enabled on STxP70 only until a full
  // evaluation is acrried out on ST200.
  if(OPT_Expand_Switch_For_Space && VHO_Switch_Old_Control_Space==FALSE) {
    for ( i = 0,  case_goto = WN_first(block); 
          i < VHO_Switch_Ncases; 
          i++, case_goto = WN_next(case_goto) ) {

      if(WN_const_val(case_goto)<first_case_value) 
        first_case_value = WN_const_val(case_goto); 

      if(WN_const_val(case_goto)>last_case_value)  
        last_case_value = WN_const_val(case_goto); 

    }
    switch_density=(INT32)(100.0*VHO_Switch_Ncases/(abs(last_case_value-first_case_value)+1));

#ifdef VHO_DEBUG
    if ( VHO_Switch_Debug )
      fprintf ( TFile, "Case values [%lld->%lld], Ncase=%d (%d), density=%d (%d)\n",
                first_case_value, last_case_value, 
                VHO_Switch_Ncases, VHO_Switch_If_Else_Limit_Space,
                switch_density, VHO_Switch_Density_Limit_Space);
#endif // VHO_DEBUG

  } // End if OPT_Expand_Switch_For_Space

  // VL: If code is optimized for space, we prefer If-Else expansion, 
  // except if density and number of cases are high, in which case we 
  // expand the switch as CompGoto.
  if ( (OPT_Expand_Switch_For_Space && VHO_Switch_Old_Control_Space==FALSE &&
         (switch_density < VHO_Switch_Density_Limit_Space || 
          VHO_Switch_Ncases < VHO_Switch_If_Else_Limit_Space)) 
       ||
       (!(OPT_Expand_Switch_For_Space && VHO_Switch_Old_Control_Space==FALSE) &&
         (VHO_Switch_Ncases <= VHO_Switch_If_Else_Limit)) ) {
#else
  if ( VHO_Switch_Ncases <= VHO_Switch_If_Else_Limit ) {
#endif

#ifdef VHO_DEBUG
    if ( VHO_Switch_Debug )
      fprintf ( TFile, "SWITCH_If_Else %d\n", (INT32) srcpos );
#endif /* VHO_DEBUG */

#if defined( KEY) && !defined(TARG_ST)
    if (!Cur_PU_Feedback &&
	VHO_Switch_Reduce_Branch) {
      wn = VHO_Switch_Generate_If_Else_Reduce_Branch(srcpos);
    } else
#endif
    wn = VHO_Switch_Generate_If_Else ( srcpos );
  }

  else {
   if (VHO_Switch_Opt) 
       conv_wn = VHO_Switch_Opt_Case_Hoist ( srcpos );

#ifdef VHO_DEBUG
   if ( VHO_Switch_Debug )
       fprintf ( TFile, "SWITCH_OPT %d\n", (INT32) srcpos );
#endif /* VHO_DEBUG */
	    
    VHO_Switch_Find_Clusters ();

    if ( VHO_Switch_Nclusters == 1 ) {

#ifdef VHO_DEBUG
      if ( VHO_Switch_Debug )
        fprintf ( TFile, "SWITCH_Compgoto %d\n", (INT32) srcpos );
#endif /* VHO_DEBUG */

      wn = VHO_Switch_Generate_Compgoto ( srcpos );
    }

    else {

#ifdef VHO_DEBUG
      if ( VHO_Switch_Debug )
        fprintf ( TFile, "SWITCH_binary %d\n", (INT32) srcpos );
#endif /* VHO_DEBUG */

      wn = WN_CreateBlock ();
      WN_Set_Linenum ( wn, srcpos );

      if (VHO_Switch_Default_Goto)
        VHO_Switch_Default_Label = WN_label_number(VHO_Switch_Default_Goto);
      else
    	LABEL_Init (New_LABEL (CURRENT_SYMTAB, VHO_Switch_Default_Label), 
		0, LKIND_DEFAULT);

      VHO_Switch_Generate_Binary_Search ( 0, VHO_Switch_Ncases-1, wn );
    }
  }

  MEM_POOL_FREE ( MEM_local_pool_ptr, VHO_Switch_Case_Table );
  MEM_POOL_FREE ( MEM_local_pool_ptr, VHO_Switch_Cluster_Table );
  if (conv_wn == NULL)
        return wn;
  else {
     WN_INSERT_BlockAfter (conv_wn, WN_last(conv_wn),  wn);
     return conv_wn;
  }

  return wn;
} /* VHO_Lower_Switch */

/* ============================================================================
 *
 * static WN *
 * VHO_Lower_Compgoto ( WN * wn )
 *
 * ============================================================================
 */

static WN *
VHO_Lower_Compgoto ( WN * wn )
{
  return wn;
} /* VHO_Lower_Compgoto */


/* ============================================================================
 *
 * static void
 * VHO_Get_Field_List ( WN_OFFSET offset, TY_IDX sty_idx )
 *
 * Flatten a struct/union 'sty' starting as 'offset' into non overlapping
 * fields and place them in the array VHO_Struct_Fld_Table.
 * Update VHO_Struct_Nfields, VHO_Struct_Last_Field_Offset
 * VHO_Struct_Last_Field_Size.
 *
 * If the structure contains misaligned data, holes or fields which
 * are out of order, then lowering is not done.
 * This is done by setting VHO_Struct_Can_Be_Lowered to FALSE.
 *
 * ============================================================================
 */

static void
VHO_Get_Field_List ( WN_OFFSET offset, TY_IDX sty_idx )
{
  TY_IDX      fty_idx;
  WN_OFFSET   field_offset;

  if ( TY_fld(Ty_Table [sty_idx]).Is_Null () ) {

    VHO_Struct_Can_Be_Lowered = FALSE;
    return;
  }

  FLD_ITER fld_iter = Make_fld_iter (TY_fld (Ty_Table[sty_idx]));

  while (VHO_Struct_Can_Be_Lowered) {
    FLD_HANDLE fld (fld_iter);

    if ( FLD_is_bit_field(fld) ) {
      VHO_Struct_Can_Be_Lowered = FALSE;
      break;
    }

    fty_idx = FLD_type(fld);

    switch ( TY_kind(Ty_Table [fty_idx]) ) {

      case KIND_SCALAR:
      case KIND_POINTER:


        field_offset = offset + FLD_ofst(fld);

        if ( VHO_Struct_Nfields ) {
#ifdef TARG_ST
	  // FdF 20050315: From PathScale, to support padding withing
	  // a struct.
          if ( field_offset >=   VHO_Struct_Last_Field_Offset
                               + VHO_Struct_Last_Field_Size ) {
#else
          if ( field_offset >=   VHO_Struct_Last_Field_Offset
                               + VHO_Struct_Last_Field_Size ) {
#endif

            /* new field */
#ifdef TARG_ST
	    if (VHO_Struct_Nfields >= 255) {
	      VHO_Struct_Can_Be_Lowered = FALSE;
	      break;
	    }
#endif

            VHO_Struct_Last_Field_Offset = field_offset;
            VHO_Struct_Last_Field_Size   = TY_size (Ty_Table [fty_idx]);
            VHO_Struct_Offset_Table [VHO_Struct_Nfields] = field_offset;
            VHO_Struct_Fld_Table [VHO_Struct_Nfields++] = fty_idx;
          }

          else
          if ( field_offset == VHO_Struct_Last_Field_Offset ) {

            /* overlapping field with same start offset */

            if ( TY_size (Ty_Table [fty_idx]) > VHO_Struct_Last_Field_Size ) {

              /* overlapping field with larger size */

              VHO_Struct_Last_Field_Offset = field_offset;
              VHO_Struct_Last_Field_Size   = TY_size (Ty_Table [fty_idx]);
              VHO_Struct_Fld_Table [VHO_Struct_Nfields-1] = fty_idx;
              VHO_Struct_Offset_Table [VHO_Struct_Nfields-1] = field_offset; 
            }
          }

          else
          if (   field_offset + TY_size (Ty_Table [fty_idx]) 
               > VHO_Struct_Last_Field_Offset + VHO_Struct_Last_Field_Size ) {

            VHO_Struct_Can_Be_Lowered = FALSE;
          }
#ifdef TARG_ST
	  // FdF 20070913: Overlapping with current or previous field,
	  // but not included and does not include current field,
	  // cancel lowering
	  else if (field_offset < VHO_Struct_Last_Field_Offset)
	    VHO_Struct_Can_Be_Lowered = FALSE;
#endif
        }

        else {

          /* new field */

          VHO_Struct_Last_Field_Offset = field_offset;
          VHO_Struct_Last_Field_Size   = TY_size (Ty_Table [fty_idx]);
          VHO_Struct_Offset_Table [VHO_Struct_Nfields] = field_offset; 
          VHO_Struct_Fld_Table [VHO_Struct_Nfields++] = fty_idx;
        }
        break;

      case KIND_STRUCT:

        if ( TY_is_packed(Ty_Table[fty_idx] ))
          VHO_Struct_Can_Be_Lowered = FALSE;

        else
          VHO_Get_Field_List (offset + FLD_ofst (fld), fty_idx);
        break;

      case KIND_ARRAY:
        INT32   array_elem_num;
	TY_IDX  ety_idx;
		  
        ety_idx = Ty_Table [fty_idx].Etype();
#ifdef KEY // bug 5273: array element size can be zero
	if (TY_size(Ty_Table[ety_idx]) == 0)
	  array_elem_num = 0;
	else {
#endif
        Is_True (TY_size(Ty_Table [fty_idx])%TY_size(Ty_Table [ety_idx]) == 0,
                       ("unexpected array type"));
        array_elem_num = TY_size(Ty_Table [fty_idx])/TY_size(Ty_Table [ety_idx]);
#ifdef KEY // bug 5273
	}
#endif
        // Is_True (VHO_Struct_Nfields + array_elem_num < 255,
        //            ("number of flattened fields exceeds limit");
        if (VHO_Struct_Nfields + array_elem_num >= 255) {
           VHO_Struct_Can_Be_Lowered = FALSE;
           break;
        }  
#ifdef KEY // bug 15203: would be messy if array element is another array
    	if (TY_kind(Ty_Table[ety_idx]) == KIND_ARRAY) {
           VHO_Struct_Can_Be_Lowered = FALSE;
           break;
        }  
#endif
        field_offset = offset + FLD_ofst(fld);

        if(VHO_Struct_Nfields) {
          if ( field_offset >=   VHO_Struct_Last_Field_Offset
                                 + VHO_Struct_Last_Field_Size ) { 
            for(int i=0; i<array_elem_num; i++) {
              VHO_Struct_Offset_Table [VHO_Struct_Nfields] = field_offset + i*TY_size(Ty_Table [ety_idx]);
              VHO_Struct_Fld_Table [VHO_Struct_Nfields++] = ety_idx;
            }
            VHO_Struct_Last_Field_Offset = field_offset;
            VHO_Struct_Last_Field_Size   = TY_size(Ty_Table [fty_idx]); 
          } else if ( field_offset == VHO_Struct_Last_Field_Offset ) {
            if ( TY_size (Ty_Table [fty_idx]) > VHO_Struct_Last_Field_Size ) {
    #ifdef TARG_ST
	      if (array_elem_num == 1)
		// FdF 20070913: This is not really an array, just
		// analyze the unique element
		VHO_Get_Field_List (field_offset, ety_idx);
	      else if ((TY_kind(ety_idx) != KIND_SCALAR) && (TY_kind(ety_idx) != KIND_POINTER))
		// FdF 20070913: There may be padding inside array
		// elements, for example when ety_idx is a STRUCT (see
		// codex #29209), so cancel lowering.
		VHO_Struct_Can_Be_Lowered = FALSE;
	      else {
#endif
              VHO_Struct_Nfields--;
              for(int i=0; i<array_elem_num; i++) {
                VHO_Struct_Offset_Table [VHO_Struct_Nfields] = field_offset + i*TY_size(Ty_Table [ety_idx]);
                VHO_Struct_Fld_Table [VHO_Struct_Nfields++] = ety_idx;
              }
              VHO_Struct_Last_Field_Offset = field_offset;
              VHO_Struct_Last_Field_Size   = TY_size(Ty_Table [fty_idx]); 
             
#ifdef TARG_ST
	      }
#endif
            }
          } else if ( field_offset + TY_size (Ty_Table [fty_idx])
                      > VHO_Struct_Last_Field_Offset + VHO_Struct_Last_Field_Size ) {
            VHO_Struct_Can_Be_Lowered = FALSE;
          }      
#ifdef TARG_ST
	  // FdF 20070913: Overlapping with current or previous field,
	  // but not included and does not include current field,
	  // cancel lowering
	  else if (field_offset < VHO_Struct_Last_Field_Offset)
	    VHO_Struct_Can_Be_Lowered = FALSE;
#endif     
        }
        else {
          for(int i=0; i<array_elem_num; i++) {
            VHO_Struct_Offset_Table [VHO_Struct_Nfields] = field_offset + i*TY_size(Ty_Table [ety_idx]); 
            VHO_Struct_Fld_Table [VHO_Struct_Nfields++] = ety_idx;
          }
          VHO_Struct_Last_Field_Offset = field_offset;
          VHO_Struct_Last_Field_Size   = TY_size(Ty_Table [fty_idx]);
        }
        break;

      default:

        VHO_Struct_Can_Be_Lowered = FALSE;
        break;
    }

    if (FLD_last_field (fld))
      break;
    else
      ++fld_iter;
  }
   
} /* VHO_Get_Field_List */

static TY_IDX
get_field_type (TY_IDX struct_type, UINT field_id)
{
  Is_True (TY_kind (struct_type) == KIND_STRUCT, ("expecting KIND_STRUCT"));
  UINT cur_field_id = 0;
  FLD_HANDLE fld = FLD_get_to_field (struct_type, field_id, cur_field_id);
  Is_True (! fld.Is_Null(), ("Invalid field id %d for type 0x%x",
                          field_id, struct_type));
  return FLD_type (fld);
}

#ifdef KEY
// If there is a single field that spans across the entire
// struct (i.e. any other field has size zero), return the field-id,
// else return 0.
static INT
single_field_in_struct (TY_IDX struct_type)
{
  Is_True (TY_kind(struct_type) == KIND_STRUCT,
           ("single_field_in_struct: expected struct type"));
  
  Is_True (TY_size(struct_type) == 4 || TY_size(struct_type) == 8,
           ("single_field_in_struct: sizes 4/8 only supported"));
  FLD_ITER fld_iter = Make_fld_iter (TY_fld (struct_type));
  INT fld_count = 0;

  do {
    FLD_HANDLE fld(fld_iter);
    fld_count++;
    if (! FLD_is_bit_field(fld) &&
        TY_size (FLD_type (fld)) == TY_size (struct_type))
      return fld_count;
    // Else if there is a field with non-zero size, then there must be
    // multiple non-zero-size fields.
    else if (TY_size (FLD_type (fld)))
      return 0;
  } while (!FLD_last_field(fld_iter++));

  return 0;
}
#endif

/* ============================================================================
 *
 * WN *VHO_Lower_Mstore ( WN * wn )
 *
 * If the structure being copied is sufficiently small, then flatten out the
 * structure into a set of non overlapping fields and load/store the
 * individual fields, thereby replacing MSTORE/MLOAD by a sequence of
 * one or more (controlled by VHO_Struct_Limit) pairs of STID/ISTORE
 * over LDID/ILOAD.
 *
 * The following structures are not currently handled.
 *
 *   structures containing misaligned fields
 *   structures containing bit fields
 *   structures containing arrays
 *
 * ============================================================================
 */
#ifdef TARG_ST
/*
 * [CG]: We also tread MSTID in addition to MSTORE.
 */
WN *
VHO_Lower_Mstore ( WN * wn )
{
  WN_OFFSET   dst_offset;
  WN        * dst_address;
  TY_IDX      ptr_dst_ty_idx;
  TY_IDX      dst_ty_idx;
  ST        * dst_st;
  WN        * src_value;
  WN_OFFSET   src_offset;
  WN        * src_address;
  TY_IDX      ptr_src_ty_idx;
  TY_IDX      src_ty_idx;
  ST        * src_st;
  WN        * size;
  INT64       src_bytes, dst_bytes;
  WN        * block;
  // WN_OFFSET   field_offset;
  INT32       i;
  SRCPOS      srcpos;
  WN        * temp;
  PREG_NUM    preg;
  ST        * preg_st;
  TY_IDX      preg_ty_idx;
  OPCODE      opc;

  if (!VHO_Struct_Opt) return wn;

  srcpos     = WN_Get_Linenum(wn);    

#ifdef VHO_DEBUG
  if ( VHO_Struct_Debug )
    fprintf ( TFile, "VHO_Lower_Mstore : %s %s\n", OPCODE_name(WN_opcode(wn)),
	      OPCODE_name(WN_opcode(WN_kid0(wn))));
#endif /* VHO_DEBUG */

  if (WN_opcode(wn) == OPC_MSTID) {
    dst_ty_idx = WN_ty(wn);
    dst_bytes = TY_size(dst_ty_idx);
  } else if (WN_opcode(wn) == OPC_MSTORE &&
	     WN_operator(WN_kid(wn,2)) == OPR_INTCONST) {
    ptr_dst_ty_idx = WN_ty(wn);
    dst_ty_idx = TY_pointed(Ty_Table[ptr_dst_ty_idx]);
    dst_bytes = WN_const_val(WN_kid(wn,2));
  } else if (WN_opcode(wn) == OPC_MISTORE && WN_field_id (wn) == 0) {
    //    if (WN_ty(wn) != WN_ty(WN_kid1(wn)))
    //      return wn;
    ptr_dst_ty_idx = WN_ty(wn);
    dst_ty_idx = TY_pointed(Ty_Table[ptr_dst_ty_idx]);
    dst_bytes = TY_size(Ty_Table[dst_ty_idx]);
  } else return wn;
  
  src_value      = WN_kid0(wn);

  if (WN_opcode(src_value) == OPC_MMLDID &&
      // don't lower return values, handled in LOWER_RETURN_VAL
      WN_st(src_value) != Return_Val_Preg) {
    src_ty_idx  = WN_ty(src_value);
    src_bytes = TY_size(src_ty_idx);
  } else if (WN_opcode(src_value) == OPC_MLOAD &&
	     WN_operator(WN_kid1(src_value)) == OPR_INTCONST) {
    ptr_src_ty_idx = WN_ty(src_value);
    src_ty_idx  = TY_pointed(Ty_Table[ptr_src_ty_idx]);
    src_bytes = WN_const_val(WN_kid1(src_value));
    /* FdF: Support for MILOAD */
  } else if (WN_opcode(src_value) == OPC_MMILOAD && WN_field_id (src_value) == 0) {
    ptr_src_ty_idx = WN_load_addr_ty(src_value);
    src_ty_idx  = WN_ty(src_value);
    src_bytes = TY_size(Ty_Table[src_ty_idx]);
  } else return wn;

#ifdef VHO_DEBUG
  if ( VHO_Struct_Debug )
    fprintf ( TFile, "VHO_Lower_Mstore : size: %d %d, align: %d %d\n", 
	      (INT32)src_bytes, (INT32)dst_bytes, 
	      (INT32)TY_align(src_ty_idx), (INT32)TY_align(dst_ty_idx));
#endif /* VHO_DEBUG */
  
  if (TY_align(src_ty_idx) == TY_align(dst_ty_idx) &&
      src_bytes == dst_bytes &&
#ifdef TARG_ST
      /* (cbr) ddts st22453. emit code for all fields only if copying the whole struct */
      WN_field_id (wn) == 0 &&
#endif
      TY_kind(src_ty_idx) == KIND_STRUCT &&
      TY_is_packed(Ty_Table[src_ty_idx]) == FALSE &&
      TY_is_packed(Ty_Table[dst_ty_idx]) == FALSE &&
      src_bytes / TY_align(src_ty_idx) <= VHO_Struct_Limit ) {
    
      VHO_Struct_Nfields = 0;
      VHO_Struct_Can_Be_Lowered = TRUE;
      //    VHO_Struct_Alignment = TY_align(dst_ty_idx);

      /* Flatten out the structure into non overlapping fields */
      
      VHO_Get_Field_List ( 0, src_ty_idx );

#ifdef VHO_DEBUG
      if ( VHO_Struct_Debug )
        fprintf ( TFile, "VHO_Lower_Mstore : %s %d\n",
                  VHO_Struct_Can_Be_Lowered ? "TRUE" : "FALSE",
                  (INT32) srcpos );
#endif /* VHO_DEBUG */

      if ( VHO_Struct_Can_Be_Lowered && VHO_Struct_Nfields ) {

        block = WN_CreateBlock ();
        WN_Set_Linenum ( block, srcpos );
	
        src_offset = WN_offset(src_value);

	if (WN_opcode(src_value) == OPC_MMLDID) {
	  src_st = WN_st(src_value);
	} else if (WN_opcode(src_value) == OPC_MLOAD || WN_opcode(src_value) == OPC_MMILOAD) {
	  src_address = WN_kid0(src_value);
	  if (WN_operator(src_address) == OPR_LDA) {
	    src_st      = WN_st(src_address);
	    src_offset += WN_offset(src_address);
	  } else if (WN_operator(src_address) == OPR_ARRAY &&
		     WN_operator(WN_kid0(src_address)) == OPR_LDA ) {
	    src_st = NULL;
	  } else {
	    preg = Create_Preg (Pointer_Mtype, vho_lower_mstore_name);
	    
	    preg_st     = MTYPE_To_PREG ( Pointer_Mtype );
	    preg_ty_idx = ptr_src_ty_idx;
	    
	    if (src_offset == 0) {
	      opc  = OPCODE_make_op ( OPR_STID, MTYPE_V, Pointer_Mtype );
	      temp = WN_CreateStid ( opc, preg, preg_st, preg_ty_idx,
				     src_address );
	    } else {
	      opc  = OPCODE_make_op ( OPR_INTCONST, Pointer_Mtype, MTYPE_V );
	      temp = WN_CreateIntconst ( opc, src_offset );
	      opc  = OPCODE_make_op ( OPR_ADD, Pointer_Mtype, MTYPE_V );
	      temp = WN_CreateExp2 ( opc, src_address, temp );
	      opc  = OPCODE_make_op ( OPR_STID, MTYPE_V, Pointer_Mtype );
	      temp = WN_CreateStid ( opc, preg, preg_st, preg_ty_idx, temp );
	    }
	    
	    WN_Set_Linenum(temp, srcpos);
	    
	    WN_INSERT_BlockAfter ( block, WN_last(block), temp );
	    
	    opc = OPCODE_make_op ( OPR_LDID, Pointer_Mtype, Pointer_Mtype );
	    src_address = WN_CreateLdid ( opc, preg, preg_st, preg_ty_idx );
	    src_st      = NULL;
	    src_offset  = 0;
	  }
	}
	  
        dst_offset = WN_store_offset(wn);
	
	if (WN_opcode(wn) == OPC_MSTID) {
	  dst_st = WN_st(wn);
	} else if (WN_opcode(wn) == OPC_MSTORE || WN_opcode(wn) == OPC_MISTORE) {
	  dst_address = WN_kid1(wn);
	  if (WN_operator(dst_address) == OPR_LDA) {
	    dst_st      = WN_st(dst_address);
	    dst_offset += WN_offset(dst_address);
	  } else if (WN_operator(dst_address) == OPR_ARRAY &&
		     WN_operator(WN_kid0(dst_address)) == OPR_LDA ) {
	    dst_st = NULL;
	  } else {
	    preg = Create_Preg (Pointer_Mtype, vho_lower_mstore_name);
	    
	    preg_st     = MTYPE_To_PREG ( Pointer_Mtype );
	    preg_ty_idx = ptr_dst_ty_idx;
	    
	    if (dst_offset == 0) {
	      opc  = OPCODE_make_op ( OPR_STID, MTYPE_V, Pointer_Mtype );
	      temp = WN_CreateStid ( opc, preg, preg_st, preg_ty_idx,
				     dst_address );
	    } else {
	      opc  = OPCODE_make_op ( OPR_INTCONST, Pointer_Mtype, MTYPE_V );
	      temp = WN_CreateIntconst ( opc, dst_offset );
	      opc  = OPCODE_make_op ( OPR_ADD, Pointer_Mtype, MTYPE_V );
	      temp = WN_CreateExp2 ( opc, dst_address, temp );
	      opc  = OPCODE_make_op ( OPR_STID, MTYPE_V, Pointer_Mtype );
	      temp = WN_CreateStid ( opc, preg, preg_st, preg_ty_idx, temp );
	    }

	    WN_Set_Linenum(temp, srcpos);
	    
	    WN_INSERT_BlockAfter ( block, WN_last(block), temp );
	    
	    opc  = OPCODE_make_op ( OPR_LDID, Pointer_Mtype, Pointer_Mtype );
	    dst_address = WN_CreateLdid ( opc, preg, preg_st, preg_ty_idx );
	    dst_st      = NULL;
	    dst_offset  = 0;
	  }
	}

        // field_offset = 0;
	
        for ( i = 0; i < VHO_Struct_Nfields; i++ ) {
	  
          TY_IDX   fty_idx;
          WN     * src;
          WN     * dst;

          fty_idx = VHO_Struct_Fld_Table [i];

          if (src_st) {
            opc = OPCODE_make_op ( OPR_LDID,
                                   Promoted_Mtype (TY_mtype(fty_idx)),
                                   TY_mtype(fty_idx) );
            src = WN_CreateLdid ( opc, src_offset + VHO_Struct_Offset_Table[i],
                                  src_st, fty_idx );
          } else {
            opc = OPCODE_make_op ( OPR_ILOAD,
                                   Promoted_Mtype (TY_mtype(fty_idx)),
                                   TY_mtype(fty_idx) );
            src = WN_CreateIload ( opc, src_offset + VHO_Struct_Offset_Table[i], fty_idx,
                                   Make_Pointer_Type ( fty_idx, FALSE ),
                                   WN_COPY_Tree ( src_address ) );
          }

          if (dst_st) {
            opc = OPCODE_make_op ( OPR_STID, MTYPE_V, TY_mtype(fty_idx) ); 
            dst = WN_CreateStid ( opc, dst_offset + VHO_Struct_Offset_Table[i],
                                  dst_st, fty_idx, src );
          } else {
            opc = OPCODE_make_op ( OPR_ISTORE, MTYPE_V, TY_mtype(fty_idx) );
            dst = WN_CreateIstore ( opc, dst_offset + VHO_Struct_Offset_Table[i],
                                    Make_Pointer_Type ( fty_idx, FALSE ),
                                    src,
                                    WN_COPY_Tree ( dst_address ) );
          }
	  
          WN_Set_Linenum(dst, srcpos);
	  
          WN_INSERT_BlockAfter ( block, WN_last(block), dst );
          // field_offset += TY_size(fty_idx);
        }

        wn = block;
      }
  }

  return wn;
} /* VHO_Lower_Mstore */

#else

WN *
VHO_Lower_Mstore ( WN * wn )
{
  WN_OFFSET   dst_offset;
  WN        * dst_address;
  TY_IDX      ptr_dst_ty_idx;
  TY_IDX      dst_ty_idx;
  ST        * dst_st = NULL;
  WN        * src_value;
  WN_OFFSET   src_offset;
  WN        * src_address;
  TY_IDX      ptr_src_ty_idx;
  TY_IDX      src_ty_idx;
  ST        * src_st;
  WN        * size;
   INT64       src_bytes, dst_bytes;
  INT64       bytes;
  WN        * block;
  // WN_OFFSET   field_offset;
  INT32       i;
  SRCPOS      srcpos;
  WN        * temp;
  PREG_NUM    preg;
  ST        * preg_st;
  TY_IDX      preg_ty_idx;
  OPCODE      opc;

  src_value      = WN_kid0(wn);
  dst_address    = WN_kid1(wn);
  size           = WN_kid(wn,2);
  dst_offset     = WN_store_offset(wn);
  ptr_dst_ty_idx = WN_ty(wn);
  srcpos         = WN_Get_Linenum(wn);

  if (    VHO_Struct_Opt
       && WN_operator(size) == OPR_INTCONST 
       && WN_opcode(src_value) == OPC_MLOAD ) {
       /* need to handle WN_opcode(src_value) == OPC_MMLDID, 
          see VHO_Lower_Mstid function                        */


    bytes          = WN_const_val(size);
    src_address    = WN_kid0(src_value);
    ptr_src_ty_idx = WN_ty(src_value);
    src_ty_idx     = TY_pointed(Ty_Table[ptr_src_ty_idx]);
    if (WN_field_id(src_value) != 0) {
       src_ty_idx = get_field_type(src_ty_idx, WN_field_id(src_value));
    }

    dst_ty_idx     = TY_pointed(Ty_Table[ptr_dst_ty_idx]);
    if (WN_field_id(wn) != 0) {
       dst_ty_idx = get_field_type(dst_ty_idx, WN_field_id(wn));
    }

    if (    TY_align(src_ty_idx) == TY_align(dst_ty_idx)
         && TY_kind(src_ty_idx) == KIND_STRUCT
         && TY_is_packed(Ty_Table[src_ty_idx]) == FALSE
         && TY_is_packed(Ty_Table[dst_ty_idx]) == FALSE
         && bytes / TY_align(src_ty_idx) <= VHO_Struct_Limit ) {

      VHO_Struct_Nfields = 0;
      VHO_Struct_Can_Be_Lowered = TRUE;
//    VHO_Struct_Alignment = TY_align(dst_ty_idx);

      /* Flatten out the structure into non overlapping fields */

      VHO_Get_Field_List ( 0, src_ty_idx );

#ifdef VHO_DEBUG
      if ( VHO_Struct_Debug )
        fprintf ( TFile, "VHO_Lower_Mstore : %s %d\n",
                  VHO_Struct_Can_Be_Lowered ? "TRUE" : "FALSE",
                  (INT32) srcpos );
#endif /* VHO_DEBUG */

      if ( VHO_Struct_Can_Be_Lowered && VHO_Struct_Nfields ) {

        block = WN_CreateBlock ();
        WN_Set_Linenum ( block, srcpos );

        src_offset = WN_offset(src_value);

        if ( WN_operator(src_address) == OPR_LDA &&
        	WN_field_id(src_value) != 0 ) {

          src_st      = WN_st(src_address);
          src_offset += WN_offset(src_address);
        } else  src_st = NULL; 
/*
        if (    WN_operator(src_address) == OPR_ARRAY 
             && WN_operator(WN_kid0(src_address)) == OPR_LDA )
          src_st = NULL;

        else {

          preg = Create_Preg (Pointer_Mtype, vho_lower_mstore_name);

          preg_st     = MTYPE_To_PREG ( Pointer_Mtype );
          preg_ty_idx = ptr_src_ty_idx;

          if ( src_offset == 0 ) {

            opc  = OPCODE_make_op ( OPR_STID, MTYPE_V, Pointer_Mtype );
            temp = WN_CreateStid ( opc, preg, preg_st, preg_ty_idx,
				   src_address );
          } else {
            opc  = OPCODE_make_op ( OPR_INTCONST, Pointer_Mtype, MTYPE_V );
            temp = WN_CreateIntconst ( opc, src_offset );
            opc  = OPCODE_make_op ( OPR_ADD, Pointer_Mtype, MTYPE_V );
            temp = WN_CreateExp2 ( opc, src_address, temp );
            opc  = OPCODE_make_op ( OPR_STID, MTYPE_V, Pointer_Mtype );
            temp = WN_CreateStid ( opc, preg, preg_st, preg_ty_idx, temp );
          }

          WN_Set_Linenum(temp, srcpos);

          WN_INSERT_BlockAfter ( block, WN_last(block), temp );

          opc = OPCODE_make_op ( OPR_LDID, Pointer_Mtype, Pointer_Mtype );
          src_address = WN_CreateLdid ( opc, preg, preg_st, preg_ty_idx );
          src_st      = NULL;
          src_offset  = 0;
        }
*/

        dst_offset = WN_store_offset(wn);

        if ( WN_operator(dst_address) == OPR_LDA && WN_field_id(wn) == 0) {

          dst_st      = WN_st(dst_address);
          dst_offset += WN_offset(dst_address);
        }

        else  dst_st = NULL; 
/*
        if (    WN_operator(dst_address) == OPR_ARRAY
             && WN_operator(WN_kid0(dst_address)) == OPR_LDA )
          dst_st = NULL;

        else {

          preg = Create_Preg (Pointer_Mtype, vho_lower_mstore_name);

          preg_st     = MTYPE_To_PREG ( Pointer_Mtype );
          preg_ty_idx = ptr_src_ty_idx;

          if ( dst_offset == 0 ) {

            opc  = OPCODE_make_op ( OPR_STID, MTYPE_V, Pointer_Mtype );
            temp = WN_CreateStid ( opc, preg, preg_st, preg_ty_idx,
				   dst_address );
          }

          else {

            opc  = OPCODE_make_op ( OPR_INTCONST, Pointer_Mtype, MTYPE_V );
            temp = WN_CreateIntconst ( opc, dst_offset );
            opc  = OPCODE_make_op ( OPR_ADD, Pointer_Mtype, MTYPE_V );
            temp = WN_CreateExp2 ( opc, dst_address, temp );
            opc  = OPCODE_make_op ( OPR_STID, MTYPE_V, Pointer_Mtype );
            temp = WN_CreateStid ( opc, preg, preg_st, preg_ty_idx, temp );
          }

          WN_Set_Linenum(temp, srcpos);

          WN_INSERT_BlockAfter ( block, WN_last(block), temp );

          opc  = OPCODE_make_op ( OPR_LDID, Pointer_Mtype, Pointer_Mtype );
          dst_address = WN_CreateLdid ( opc, preg, preg_st, preg_ty_idx );
          dst_st      = NULL;
          dst_offset  = 0;
        }
*/
        // field_offset = 0;

        for ( i = 0; i < VHO_Struct_Nfields; i++ ) {

          TY_IDX   fty_idx;
          WN     * src;
          WN     * dst;

          fty_idx = VHO_Struct_Fld_Table [i];

          if ( src_st ) {

            opc = OPCODE_make_op ( OPR_LDID,
                                   Promoted_Mtype (TY_mtype(fty_idx)),
                                   TY_mtype(fty_idx) );
            src = WN_CreateLdid ( opc, src_offset + VHO_Struct_Offset_Table[i],
                                  src_st, fty_idx );
          }

          else {

            opc = OPCODE_make_op ( OPR_ILOAD,
                                   Promoted_Mtype (TY_mtype(fty_idx)),
                                   TY_mtype(fty_idx) );
            src = WN_CreateIload ( opc, src_offset + VHO_Struct_Offset_Table[i], fty_idx,
                                   Make_Pointer_Type ( fty_idx, FALSE ),
                                   WN_COPY_Tree ( src_address ) );
          }

          if ( dst_st ) {

            opc = OPCODE_make_op ( OPR_STID, MTYPE_V, TY_mtype(fty_idx) ); 
            dst = WN_CreateStid ( opc, dst_offset + VHO_Struct_Offset_Table[i],
                                  dst_st, fty_idx, src );
          }

          else {

            opc = OPCODE_make_op ( OPR_ISTORE, MTYPE_V, TY_mtype(fty_idx) );
            dst = WN_CreateIstore ( opc, dst_offset + VHO_Struct_Offset_Table[i],
                                    Make_Pointer_Type ( fty_idx, FALSE ),
                                    src,
                                    WN_COPY_Tree ( dst_address ) );
          }

          WN_Set_Linenum(dst, srcpos);

          WN_INSERT_BlockAfter ( block, WN_last(block), dst );
          // field_offset += TY_size(fty_idx);
        }

        wn = block;

      }
    }

    else {

#ifdef VHO_DEBUG
      if ( VHO_Struct_Debug )
        fprintf ( TFile, "VHO_Lower_Mstore : FALSE %d\n", (INT32) srcpos );
#endif /* VHO_DEBUG */

    }
  }

  return wn;
} /* VHO_Lower_Mstore */
#endif
/* ==============================================================================
 * 
 * WN *VHO_Lower_Mstid(WN * wn)
 *
 * If the structure being copied is sufficiently small, then flatten out the
 * structure into a set of non overlapping fields and load/store the 
 * individual fields, thereby replacing MMSTID/MLDID by a sequence of 
 * one or more (controlled by VHO_Struct_Limit) pairs of STID/ISTORE
 * over LDID/ILOAD.
 * 
 * The following structures are not currently handled.
 * 
 * structures containing misaligned fields
 * structures containing bit fields
 * structures containing arrays
 *
 * ==============================================================================
 */

WN *
VHO_Lower_Mstid (WN * wn)
{
  WN_OFFSET   dst_offset;
  TY_IDX      dst_ty_idx;
  ST        * dst_st;
  WN        * src_value;
  WN_OFFSET   src_offset;
  TY_IDX      src_ty_idx;
  ST        * src_st;
  WN        * size;
  INT64       bytes;
  WN        * block;
  // WN_OFFSET   field_offset;
  INT32       i;
  SRCPOS      srcpos;
  OPCODE      opc;

  src_value   = WN_kid0(wn);
  dst_ty_idx  = WN_ty(wn);
  srcpos      = WN_Get_Linenum(wn);

  if (WN_field_id(wn) != 0 || WN_field_id(src_value) != 0) {
    /* otherwise, we need to lower them into iload/istore, maybe later */
    return wn;
  }
  bytes = TY_size(dst_ty_idx);

  if (   VHO_Struct_Opt
      && bytes >0 
      && WN_opcode(src_value) == OPC_MMLDID &&
       ST_class(WN_st(wn)) != CLASS_PREG &&
       ST_class(WN_st(src_value)) != CLASS_PREG) {
       /* screen out PREG for now, need to change them into extract sometime */

    src_ty_idx = WN_ty(src_value);

#ifdef KEY
    VHO_Struct_Nfields = 0;
    VHO_Struct_Can_Be_Lowered = TRUE;
#endif
    
    if(   TY_align(src_ty_idx) == TY_align(dst_ty_idx)
       && TY_kind(src_ty_idx) == KIND_STRUCT
       && TY_is_packed(Ty_Table[src_ty_idx]) == FALSE
       && TY_is_packed(Ty_Table[dst_ty_idx]) == FALSE
       && bytes / TY_align(src_ty_idx) <= VHO_Struct_Limit) {

#ifndef KEY
      VHO_Struct_Nfields = 0;
      VHO_Struct_Can_Be_Lowered = TRUE;
#endif
      //  VHO_Struct_Alignment = TY_align(dst_ty_idx);

      /* Flatten out the structure into non overlapping fields */
    
      VHO_Get_Field_List (0, src_ty_idx);

#ifdef VHO_DEBUG
      if ( VHO_Struct_Debug)
        fprintf ( TFile, "VHO_Lower_Mstid : %s %d \n",
                  VHO_Struct_Can_Be_Lowered ? "TRUE" : "FALSE",
                  (INT32) srcpos);
#endif /* VHO_DEBUG */
 
      if ( VHO_Struct_Can_Be_Lowered && VHO_Struct_Nfields) {

        block = WN_CreateBlock();
        WN_Set_Linenum (block, srcpos);

        src_st = WN_st(src_value);
        src_offset = WN_offset(src_value);

        dst_st = WN_st(wn);
        dst_offset = WN_store_offset(wn);

        // field_offset = 0;
      
        for (i = 0; i < VHO_Struct_Nfields; i++) {
       
          TY_IDX    fty_idx;
          WN      * src;
          WN      * dst;

          fty_idx = VHO_Struct_Fld_Table[i];

          opc = OPCODE_make_op ( OPR_LDID,
                                 Promoted_Mtype (TY_mtype(fty_idx)),
                                 TY_mtype(fty_idx) );
          src = WN_CreateLdid ( opc, src_offset + VHO_Struct_Offset_Table[i],
                              src_st, fty_idx);
          opc = OPCODE_make_op ( OPR_STID, MTYPE_V, TY_mtype(fty_idx) );
          dst = WN_CreateStid ( opc, dst_offset + VHO_Struct_Offset_Table[i], 
                                dst_st, fty_idx, src);
          WN_Set_Linenum(dst, srcpos);
        
          WN_INSERT_BlockAfter (block, WN_last(block), dst);
          // field_offset += TY_size(fty_idx);
        } 
        wn = block;
      }
    }
#ifdef KEY
    if (VHO_Struct_Can_Be_Lowered && VHO_Struct_Nfields)
      ; // we already lowered the struct
    else if ((bytes == 4 || bytes == 8) && TY_size(src_ty_idx) == bytes) {
      // bug 7741: change MTYPE_M to MTYPE_U[48]
      TYPE_ID mtype = (bytes == 4) ? MTYPE_U4 : MTYPE_U8;
      WN_set_desc(wn, mtype);
      WN_set_rtype(src_value, mtype);
      WN_set_desc(src_value, mtype);

      // bugs 9989, 10139
      INT field_id;
      Is_True (WN_field_id(src_value) == 0,("Expected field-id zero"));
      if ((TY_kind(src_ty_idx) == KIND_STRUCT) &&
          (field_id /* assign */ = single_field_in_struct (src_ty_idx)))
        WN_set_field_id (src_value, field_id);

      Is_True (WN_field_id(wn) == 0,("Expected field-id zero"));
      if ((TY_kind(dst_ty_idx) == KIND_STRUCT) &&
          (field_id /* assign */ = single_field_in_struct (dst_ty_idx)))
        WN_set_field_id (wn, field_id);
    }
    else {
#endif

#ifdef VHO_DEBUG
    if ( VHO_Struct_Debug) 
     fprintf (TFile, "VHO_Lower_Mstid: FALSE %d\n", (INT32) scrpos);
#endif /* VHO_DEBUG */

    }
  }

  return wn;
} /* VHO_Lower_Mstid */

/* ============================================================================
 *
 * WN *VHO_Lower_Mistore ( WN * wn )
 *
 * If the structure being copied is sufficiently small, then flatten out the
 * structure into a set of non overlapping fields and load/istore the
 * individual fields, thereby replacing MISTORE/MLDID by a sequence of
 * one or more (controlled by VHO_Struct_Limit) pairs of STID/ISTORE
 * over LDID/ILOAD.
 *
 * The following structures are not currently handled.
 *
 *   structures containing misaligned fields
 *   structures containing bit fields
 *   structures containing arrays
 *
 * ============================================================================
 */
WN *
VHO_Lower_Mistore ( WN * wn )
{
  WN_OFFSET   dst_offset;
  TY_IDX      dst_ty_idx;
  ST        * dst_st;
  WN        * src_value;
  WN_OFFSET   src_offset;
  TY_IDX      src_ty_idx;
  ST        * src_st;
  WN        * size;
  INT64       bytes;
  WN        * block;
  // WN_OFFSET   field_offset;
  INT32       i;
  SRCPOS      srcpos;
  OPCODE      opc;

  src_value      = WN_kid0(wn);
  WN *dst_address= WN_kid1(wn);
  dst_offset     = WN_store_offset(wn);
  TY_IDX ptr_dst_ty_idx = WN_ty(wn);
  dst_ty_idx     = TY_pointed(Ty_Table[ptr_dst_ty_idx]);
  if (WN_field_id(wn) != 0) {
    dst_ty_idx = get_field_type(dst_ty_idx, WN_field_id(wn));
  }
  srcpos         = WN_Get_Linenum(wn);
  bytes          = TY_size(dst_ty_idx);

  if (WN_field_id(src_value) != 0) {
    /* otherwise, we need to get to the subfields, or generate ILOAD, later */
    return wn;
  }
  if (    VHO_Struct_Opt
	  && bytes > 0 
	  && WN_opcode(src_value) == OPC_MMLDID &&
	  ST_class(WN_st(src_value)) != CLASS_PREG ) {
    /* screen out PREG for now, need to change them into extract sometime */

    src_ty_idx     = WN_ty(src_value);

    if (    TY_align(src_ty_idx) == TY_align(dst_ty_idx)
         && TY_kind(src_ty_idx) == KIND_STRUCT
         && TY_is_packed(Ty_Table[src_ty_idx]) == FALSE
         && TY_is_packed(Ty_Table[dst_ty_idx]) == FALSE
         && bytes / TY_align(src_ty_idx) <= VHO_Struct_Limit ) {

      VHO_Struct_Nfields = 0;
      VHO_Struct_Can_Be_Lowered = TRUE;
      //    VHO_Struct_Alignment = TY_align(dst_ty_idx);

      /* Flatten out the structure into non overlapping fields */

      VHO_Get_Field_List ( 0, src_ty_idx );

#ifdef VHO_DEBUG
      if ( VHO_Struct_Debug )
        fprintf ( TFile, "VHO_Lower_Mistore : %s %d\n",
                  VHO_Struct_Can_Be_Lowered ? "TRUE" : "FALSE",
                  (INT32) srcpos );
#endif /* VHO_DEBUG */

      if ( VHO_Struct_Can_Be_Lowered && VHO_Struct_Nfields ) {

        block = WN_CreateBlock ();
        WN_Set_Linenum ( block, srcpos );

	src_st     = WN_st(src_value);
        src_offset = WN_offset(src_value);

        // field_offset = 0;

        for ( i = 0; i < VHO_Struct_Nfields; i++ ) {

          TY_IDX   fty_idx;
          WN     * src;
          WN     * dst;

          fty_idx = VHO_Struct_Fld_Table [i];

	  opc = OPCODE_make_op ( OPR_LDID,
				 Promoted_Mtype (TY_mtype(fty_idx)),
				 TY_mtype(fty_idx) );
	  src = WN_CreateLdid ( opc, src_offset + VHO_Struct_Offset_Table[i],
				src_st, fty_idx );
	  opc = OPCODE_make_op ( OPR_ISTORE, MTYPE_V, TY_mtype(fty_idx));
	  dst = WN_CreateIstore ( opc, dst_offset + VHO_Struct_Offset_Table[i],
				  Make_Pointer_Type( fty_idx, FALSE ),
				  src, WN_COPY_Tree( dst_address ));
          WN_Set_Linenum(dst, srcpos);

          WN_INSERT_BlockAfter ( block, WN_last(block), dst );
          // field_offset += TY_size(fty_idx);
        }
        wn = block;
      }
    }

    else {

#ifdef VHO_DEBUG
      if ( VHO_Struct_Debug )
        fprintf ( TFile, "VHO_Lower_Mstid : FALSE %d\n", (INT32) srcpos );
#endif /* VHO_DEBUG */
 
    }
  }

  return wn;
} /* VHO_Lower_Mstid */

static void
vho_lower_set_st_addr_info ( WN * wn, ADDRESS_INFO_TYPE code )
{
  OPCODE     opc;
  OPERATOR   wn_operator;
  ST       * st;

  opc = WN_opcode(wn);
  wn_operator = OPCODE_operator(opc);

  switch ( wn_operator ) {

    case OPR_LDA:

      st = WN_st(wn);
      if ( ST_class(st) == CLASS_VAR || ST_class(st) == CLASS_FUNC ) {

        if ( code == ADDRESS_PASSED ) {

          if ( WHIRL_Addr_Passed_On ) {
            Set_ST_addr_taken_passed(st);

            if ( WHIRL_Addr_Saved_For_Passed_On )
              Set_ST_addr_taken_saved(st);
          }
        }

        else
          Set_ST_addr_taken_saved(st);
      }
      break;

    case OPR_ARRAY:

      vho_lower_set_st_addr_info ( WN_kid0(wn), code );
      break;

    case OPR_LDID:
/*
      st = WN_st(wn);
      if ( ST_sclass(st) == SCLASS_FORMAL && (ST_class(st) == CLASS_VAR ||
					      ST_class(st) == CLASS_FUNC ))
        if ( code == ADDRESS_PASSED )
          Set_ST_addr_taken_passed(st);
        else
          Set_ST_addr_taken_saved(st);
      break;
*/
    case OPR_CONST:
    case OPR_ILOAD:
    case OPR_MLOAD:
    case OPR_INTCONST:
    case OPR_INTRINSIC_OP:
    case OPR_CALL:
    case OPR_EQ:
    case OPR_NE:
    case OPR_GT:
    case OPR_GE:
    case OPR_LT:
    case OPR_LE:
      break;

    case OPR_EVAL:
    case OPR_TAS:
    case OPR_CVT:
    case OPR_CVTL:
    case OPR_NEG:
    case OPR_ABS:
    case OPR_SQRT:
    case OPR_REALPART:
    case OPR_IMAGPART:
    case OPR_PAREN:
    case OPR_RND:
    case OPR_TRUNC:
    case OPR_CEIL:
    case OPR_FLOOR:
    case OPR_BNOT:
    case OPR_LNOT:
    case OPR_DIVPART:
    case OPR_REMPART:
    case OPR_MINPART:
    case OPR_MAXPART:
    case OPR_RECIP:
    case OPR_RSQRT:
    case OPR_PARM:
#ifdef TARG_ST
  case OPR_SUBPART:
  case OPR_AFFIRM:
#endif
#if defined( KEY) && !defined(TARG_ST)
    case OPR_EXTRACT_BITS:
    case OPR_ALLOCA:
#endif

      vho_lower_set_st_addr_info ( WN_kid0(wn), code );
      break;

    case OPR_SELECT:
    case OPR_CSELECT:
    case OPR_ADD:
    case OPR_SUB:
    case OPR_MPY:
    case OPR_DIV:
    case OPR_MOD:
    case OPR_REM:
    case OPR_DIVREM:
    case OPR_MAX:
    case OPR_MIN:
    case OPR_MINMAX:
    case OPR_BAND:
    case OPR_BIOR:
    case OPR_BXOR:
    case OPR_BNOR:
    case OPR_LAND:
    case OPR_LIOR:
    case OPR_SHL:
    case OPR_ASHR:
    case OPR_LSHR:
    case OPR_COMPLEX:
    case OPR_HIGHMPY:
#if defined( KEY) && !defined(TARG_ST)
    case OPR_COMPOSE_BITS:
#endif

      vho_lower_set_st_addr_info ( WN_kid0(wn), code );
      vho_lower_set_st_addr_info ( WN_kid1(wn), code );
      break;

    case OPR_CAND:
    case OPR_CIOR:

      break;

    case OPR_COMMA:

      vho_lower_set_st_addr_info ( WN_kid1(wn), code );
      break;

    case OPR_RCOMMA:

      vho_lower_set_st_addr_info ( WN_kid0(wn), code );
      break;

    default:

      FmtAssert(FALSE, ("op %s unexpected in vho_lower_set_st_addr_info",
      		      OPERATOR_name(wn_operator)));
  }
} /* vho_lower_set_st_addr_taken */


static void
vho_initialize_bool_info ( BOOL_INFO * bool_info )
{
  bool_info->opcode           = OPCODE_UNKNOWN;
  bool_info->used_true_label  = FALSE;
  bool_info->used_false_label = FALSE;
  bool_info->true_label       = 0;
  bool_info->false_label      = 0;
} /* vho_initialize_bool_info */


#ifdef TARG_ST
static BOOL
vho_is_ldid_of_return_val(WN *wn)
{
  return WN_operator(wn) == OPR_LDID
    && WN_st(wn) == Return_Val_Preg;
}
#endif
static WN *
#if defined( KEY) && !defined(TARG_ST)
vho_lower_comma ( WN * wn, WN *block, BOOL_INFO * bool_info, OPCODE stmt_opc )
#else
vho_lower_comma ( WN * wn, WN *block, BOOL_INFO * bool_info )
#endif
{
  WN       * comma_block;
  WN       * result_block;
  WN       * result;
  WN       * first;
  WN       * value;
  WN       * stmt;
  PREG_NUM   preg;
  BOOL       call;

  result = WN_kid1 (wn);
#ifdef TARG_ST
  call   = WHIRL_Return_Val_On && vho_is_ldid_of_return_val(result);
#else
  call   =   WHIRL_Return_Val_On
          && WN_operator(result) == OPR_LDID
          && WN_st(result) == Return_Val_Preg;
#endif

  comma_block = vho_lower_block (WN_kid0(wn));
  WN_Set_Linenum ( comma_block, VHO_Srcpos );
  first = WN_first(comma_block);

  result_block = WN_CreateBlock ();
  result = vho_lower_expr (WN_kid1(wn), result_block, bool_info);

  if (    VHO_Call_Opt
       && first
       && WN_first(result_block) == NULL
       && WN_operator(first) == OPR_PRAGMA
       && WN_pragma(first) == WN_PRAGMA_INLINE_BODY_START ) {
/*
    fprintf ( stderr, "INLINE_BODY_START\n" );
    fdump_tree ( stderr, comma_block );
*/
    value = NULL;
    preg  = 0;

    stmt = WN_next(first);

    for (;;) {
/*
      fprintf ( stderr, "INLINE stmt\n" );
      fdump_tree ( stderr, stmt );
*/
      if (    WN_operator(stmt) == OPR_STID
           && ST_class(WN_st(stmt)) == CLASS_PREG ) {

        if ( value == NULL )
          value = WN_kid0(stmt);

        else
        if (    WN_operator(WN_kid0(stmt)) == OPR_LDID
             && ST_class(WN_st(WN_kid0(stmt))) == CLASS_PREG
             && WN_load_offset(WN_kid0(stmt)) == preg ) {
        }

        else
          break;

        preg = WN_store_offset(stmt);
/*
        fprintf ( stderr, "preg = %d\n", preg );
*/
      }

      else
      if (    WN_operator(stmt) == OPR_PRAGMA
           && WN_pragma(stmt) == WN_PRAGMA_INLINE_BODY_END ) {

        stmt = WN_next(stmt);
/*
        fprintf ( stderr, "INLINE_BODY_END\n" );
        fprintf ( stderr, "result\n" );
        fdump_tree ( stderr, result );
        fprintf ( stderr, "stmt\n" );
        fdump_tree ( stderr, stmt );
*/
        if (    stmt
             && WN_next(stmt) == NULL
             && WN_operator(stmt) == OPR_STID
             && ST_class(WN_st(stmt)) == CLASS_PREG ) {

          if ( value == NULL )
            value = WN_kid0(stmt);

          else
          if (    WN_operator(WN_kid0(stmt)) == OPR_LDID
               && ST_class(WN_st(WN_kid0(stmt))) == CLASS_PREG
               && WN_load_offset(WN_kid0(stmt)) == preg ) {
          }

          else
            break;
        }

        else
          break;

        preg = WN_store_offset(stmt);
/*
        fprintf ( stderr, "preg = %d\n", preg );
        fprintf ( stderr, "value\n" );
        fdump_tree ( stderr, value );
        fprintf ( stderr, "preg = %d\n", preg );
*/
        if (    value
             && WN_operator(result) == OPR_LDID
             && ST_class(WN_st(result)) == CLASS_PREG
             && WN_load_offset(result) == preg ) {

          comma_block = NULL;
          result = WN_Simplify_Tree ( value );
/*
          fprintf ( stderr, "vho_lower_comma: inline old\n" );
          fdump_tree ( stderr, wn );
          fprintf ( stderr, "vho_lower_comma: inline new\n" );
          fdump_tree ( stderr, result );
*/
        }

        break;
      }

      else
        break;

      stmt = WN_next(stmt);
    }
  }

  if ( comma_block ) {

    if (call) {

      TYPE_ID rtype  = WN_rtype (result);
      TYPE_ID desc   = WN_desc  (result);
      TY_IDX  ty_idx = WN_ty (result);

      if (desc == MTYPE_M) {
        ST* st = Gen_Temp_Symbol (ty_idx, ".call");
        wn = WN_CreateStid (OPC_MSTID, 0, st, ty_idx, result);
        WN_Set_Linenum ( wn, VHO_Srcpos );
        WN_INSERT_BlockLast (comma_block, wn);
        result = WN_CreateLdid (OPC_MMLDID, 0, st, ty_idx);
      }
#ifdef TARG_X8664
      else if (MTYPE_is_complex(desc) ||
	       MTYPE_is_mmx_vector(desc)) {
        ST* st = Gen_Temp_Symbol (ty_idx, ".call");
        wn = WN_CreateStid (OPR_STID, MTYPE_V, desc, 0, st, ty_idx, result);
        WN_Set_Linenum ( wn, VHO_Srcpos );
        WN_INSERT_BlockLast (comma_block, wn);
        result = WN_CreateLdid (OPR_LDID, desc, desc, 0, st, ty_idx);
      }
#endif
#ifdef TARG_MIPS // Bug 12946: Will return complex quad via first arg
      else if (desc == MTYPE_CQ) {
        ST* st = Gen_Temp_Symbol (ty_idx, ".call");
        wn = WN_CreateStid (OPR_STID, MTYPE_V, desc, 0, st, ty_idx, result);
        WN_Set_Linenum ( wn, VHO_Srcpos );
        WN_INSERT_BlockLast (comma_block, wn);
        result = WN_CreateLdid (OPR_LDID, desc, desc, 0, st, ty_idx);
      }
#endif

      else {

#if defined(KEY) && !defined(TARG_ST)
        if (VHO_In_MP_Region_Pragma)
        { // Use a temporary symbol instead of a preg
          ST * comma_st = Gen_Temp_Symbol (MTYPE_TO_TY_array[rtype], "_comma");
          wn = WN_Stid (rtype, 0, comma_st, ty_idx, result);
          WN_Set_Linenum ( wn, VHO_Srcpos );
          WN_INSERT_BlockLast (comma_block, wn);
          result = WN_Ldid (rtype, 0, comma_st, ty_idx);
        }
        else if (VHO_Delete_Extra_Retval_Store &&
                 stmt_opc != OPCODE_UNKNOWN &&
                 OPCODE_operator(stmt_opc) == OPR_STID &&
                 OPCODE_desc(stmt_opc) == rtype /* bug 15095 */)
          ; // do nothing, just return result (sct 5409)
        else
#endif // KEY
        {
          PREG_NUM preg    = Create_Preg (rtype, vho_lower_comma_name);
          ST*      preg_st = MTYPE_To_PREG (rtype);
          OPCODE   opcode  = OPCODE_make_op (OPR_STID, MTYPE_V, rtype);
          wn = WN_CreateStid (opcode, preg, preg_st, ty_idx, result);
          WN_Set_Linenum ( wn, VHO_Srcpos );
          WN_INSERT_BlockLast (comma_block, wn);
          opcode = OPCODE_make_op (OPR_LDID, rtype, rtype);
          result = WN_CreateLdid (opcode, preg, preg_st, ty_idx);
        }
      }
    }

#if defined(KEY) && !defined(TARG_ST)
    if (VHO_In_MP_Region_Pragma && WN_operator (result) == OPR_LDID &&
        ST_class (WN_st (result)) == CLASS_PREG)
    {
      // This can be caused by inlining of a call inside an xpragma, where
      // we need to use a temporary instead of a preg.
      TYPE_ID rtype = WN_rtype (result);
      ST * comma_st = Gen_Temp_Symbol (MTYPE_TO_TY_array[rtype], "_comma");
      WN * stid = WN_Stid (rtype, 0, comma_st, WN_ty (result), result);
      WN_Set_Linenum (stid, VHO_Srcpos);
      WN_INSERT_BlockLast (comma_block, stid);
      result = WN_Ldid (rtype, 0, comma_st, WN_ty (result));
    }

    // If this comma node is inside an atomic pragma, insert the lowered
    // comma block before the pragma node, since omp-lowerer expects the
    // atomic store to be the next stmt after the pragma
    WN * last = WN_last (block);
    if (last && WN_operator (last) == OPR_PRAGMA &&
        WN_pragma (last) == WN_PRAGMA_ATOMIC)
    {
      WN_INSERT_BlockBefore ( block, WN_last (block), comma_block );
    }
    else
#endif // KEY
      WN_INSERT_BlockLast ( block, comma_block );
  }

  if ( WN_first(result_block) ) {

    WN_INSERT_BlockLast ( block, result_block );
  }

  wn = result;

  return wn;
} /* vho_lower_comma */


static WN *
vho_lower_rcomma ( WN * wn, WN *block, BOOL_INFO * bool_info )
{
  WN       * rcomma_block;
  WN       * test;
  TYPE_ID    preg_mtype;
  PREG_NUM   preg;
  ST       * preg_st;
  TY_IDX     preg_ty_idx;
  OPCODE     opcode;

  preg_mtype = WN_rtype(wn);
  rcomma_block = WN_CreateBlock ();
  WN_Set_Linenum ( rcomma_block, VHO_Srcpos );

  test         = vho_lower_expr (WN_kid0(wn), rcomma_block, bool_info);
  if ( WN_first(rcomma_block) )
    WN_INSERT_BlockLast ( block, rcomma_block );
  rcomma_block = vho_lower_block ( WN_kid1(wn));

  preg = Create_Preg (preg_mtype, vho_lower_rcomma_name);

  preg_st      = MTYPE_To_PREG ( preg_mtype );
  preg_ty_idx  = Be_Type_Tbl(preg_mtype);
  opcode       = OPCODE_make_op ( OPR_STID, MTYPE_V, preg_mtype );
  wn           = WN_CreateStid ( opcode, preg, preg_st, preg_ty_idx, test );
  WN_Set_Linenum ( wn, VHO_Srcpos );

  WN_INSERT_BlockLast ( block, wn  );
  WN_INSERT_BlockLast ( block, rcomma_block );
  opcode     = OPCODE_make_op ( OPR_LDID, Promoted_Mtype (preg_mtype),
                                preg_mtype );
  wn = WN_CreateLdid ( opcode, preg, preg_st, preg_ty_idx );

  return wn;
} /* vho_lower_rcomma */


static WN *
vho_simplify_cand ( WN * wn, WN * l_wn, WN * r_wn )
{
  BOOL       simplified;
  OPCODE     l_opc;
  OPCODE     r_opc;
  OPERATOR   l_oper;
  OPERATOR   r_oper;
  OPERATOR   ll_oper;
  OPERATOR   lr_oper;
  OPERATOR   rl_oper;
  OPERATOR   rr_oper;
  OPERATOR   llr_oper;
  OPERATOR   rlr_oper;

  WN       * ll_wn;
  WN       * lr_wn;
  WN       * rl_wn;
  WN       * rr_wn;
  WN       * lll_wn;
  WN       * llr_wn;
  WN       * rll_wn;
  WN       * rlr_wn;

  simplified = FALSE;

  l_opc  = WN_opcode(l_wn);
  r_opc  = WN_opcode(r_wn);
  l_oper = OPCODE_operator(l_opc);
  r_oper = OPCODE_operator(r_opc);

  /* simplify
   *          ( e && e )
   * to
   *          ( e )
   */

  if ( WN_Simp_Compare_Trees ( l_wn, r_wn ) == 0 ) {

    wn = l_wn;
    simplified = TRUE;
  }

  /* simplify 
   *          ( e && TRUE )
   * to
   *          ( e )
   */

  else
  if (    r_oper == OPR_INTCONST
       && WN_const_val(r_wn) == 1 ) {

    wn = l_wn;
    simplified = TRUE;
  }

  /* simplify 
   *          ( e && FALSE )
   * to
   *          ( FALSE )
   */

  else
  if (    r_oper == OPR_INTCONST
       && WN_const_val(r_wn) == 0 ) {

    wn = r_wn;
    simplified = TRUE;
  }

  /* simplify 
   *          ( TRUE && e )
   * to
   *          ( e )
   */

  else
  if (    l_oper == OPR_INTCONST
       && WN_const_val(l_wn) == 1 ) {

    wn = r_wn;
    simplified = TRUE;
  }

  /* simplify 
   *          ( FALSE && e )
   * to
   *          ( FALSE )
   */

  else
  if (    l_oper == OPR_INTCONST
       && WN_const_val(l_wn) == 0 ) {

    wn = l_wn;
    simplified = TRUE;
  }

  /* simplify
   *           ( ( e1 || e2 ) && ( e1 || e3 ) )
   * to
   *           ( e1 || ( e2 && e3 ) )
   */

  else
  if (    l_oper == OPR_CIOR
       && r_oper == OPR_CIOR ) {

    ll_wn   = WN_kid0(l_wn);
    lr_wn   = WN_kid1(l_wn);
    rl_wn   = WN_kid0(r_wn);
    rr_wn   = WN_kid1(r_wn);

    if ( WN_Simp_Compare_Trees ( ll_wn, rl_wn ) == 0 ) {

      WN *r_wn_new = WN_Create ( OPC_I4CAND, 2 );
      WN_kid0(r_wn_new) = lr_wn;
      WN_kid1(r_wn_new) = rr_wn;
      wn = WN_Create ( OPC_I4CIOR, 2 );
      WN_kid0(wn) = ll_wn;
      WN_kid1(wn) = r_wn_new;
      simplified = TRUE;

      if ( Cur_PU_Feedback )
	Cur_PU_Feedback->FB_factor_circuit( l_wn, r_wn, wn, r_wn_new );
    }
  }
 
  /* simplify
   *          ( ( ( e & c1 ) != 0 ) && ( ( e & c2 ) != 0 ) )
   * to
   *          ( ( e & ( c1 | c2 ) ) == ( c1 | c2 ) )
   */

  else
  if (    l_oper == OPR_NE
       && r_oper == OPR_NE
       && l_opc  == r_opc ) {

    ll_wn   = WN_kid0(l_wn);
    lr_wn   = WN_kid1(l_wn);
    rl_wn   = WN_kid0(r_wn);
    rr_wn   = WN_kid1(r_wn);

    ll_oper = WN_operator(ll_wn);
    lr_oper = WN_operator(lr_wn);
    rl_oper = WN_operator(rl_wn);
    rr_oper = WN_operator(rr_wn);

    if (    ll_oper == OPR_BAND
         && lr_oper == OPR_INTCONST
         && WN_const_val(lr_wn) == 0
         && rl_oper == OPR_BAND
         && rr_oper == OPR_INTCONST
         && WN_const_val(rr_wn) == 0 ) {

       lll_wn = WN_kid0(ll_wn);
       llr_wn = WN_kid1(ll_wn);
       rll_wn = WN_kid0(rl_wn);
       rlr_wn = WN_kid1(rl_wn);

       llr_oper = WN_operator(llr_wn);
       rlr_oper = WN_operator(rlr_wn);

       if (    llr_oper == OPR_INTCONST
            && rlr_oper == OPR_INTCONST
            && WN_Simp_Compare_Trees ( lll_wn, rll_wn ) == 0
            && IS_POWER_OF_2(WN_const_val(llr_wn))
            && IS_POWER_OF_2(WN_const_val(rlr_wn)) ) {

         r_wn = rlr_wn;
         WN_const_val(r_wn) |= WN_const_val(llr_wn);
         ll_wn = lll_wn;
         lr_wn = llr_wn;
         WN_const_val(lr_wn) = WN_const_val(r_wn);
         l_wn = WN_CreateExp2 ( WN_opcode(rl_wn), ll_wn, lr_wn );
#ifdef TARG_ST
	 if ((WN_rtype(l_wn) == MTYPE_I8) || (WN_rtype(l_wn) == MTYPE_U8)) {
	   FmtAssert ((WN_rtype(r_wn) == MTYPE_I8) || (WN_rtype(r_wn) == MTYPE_U8),
		      ("Inconsistency in WHIRL types in OPR_CAND"));
	   wn = WN_CreateExp2 ( OPC_I4I8EQ, l_wn, r_wn );
	 }
	 else
#else
#ifdef KEY
	 if( MTYPE_is_size_double(WN_rtype(l_wn)) )
	   wn = WN_CreateExp2 ( OPC_I4I8EQ, l_wn, r_wn );
	 else
#endif
#endif
	   wn = WN_CreateExp2 ( OPC_I4I4EQ, l_wn, r_wn );
         simplified = TRUE;
       }
    }
  }

  if ( ! simplified ) {

    if (    WN_kid0(wn) != l_wn
         || WN_kid1(wn) != r_wn ) {

      wn = WN_Create ( OPC_I4CAND, 2 );
      WN_kid0(wn) = l_wn;
      WN_kid1(wn) = r_wn;
    }
  }

  return wn;
} /* vho_simplify_cand */

static WN *
vho_simplify_cior ( WN * wn, WN * l_wn, WN * r_wn )
{
  BOOL       simplified;
  OPCODE     l_opc;
  OPCODE     r_opc;
  OPERATOR   l_oper;
  OPERATOR   r_oper;
  OPERATOR   ll_oper;
  OPERATOR   lr_oper;
  OPERATOR   rl_oper;
  OPERATOR   rr_oper;
  OPERATOR   llr_oper;
  OPERATOR   rlr_oper;

  WN       * ll_wn;
  WN       * lr_wn;
  WN       * rl_wn;
  WN       * rr_wn;
  WN       * lll_wn;
  WN       * llr_wn;
  WN       * rll_wn;
  WN       * rlr_wn;

  simplified = FALSE;

  l_opc  = WN_opcode(l_wn);
  r_opc  = WN_opcode(r_wn);
  l_oper = OPCODE_operator(l_opc);
  r_oper = OPCODE_operator(r_opc);

  if ( WN_Simp_Compare_Trees ( l_wn, r_wn ) == 0 ) {

    wn = l_wn;
    simplified = TRUE;
  }

  /* simplify 
   *          ( e || TRUE )
   * to
   *          ( TRUE )
   */

  else
  if (    r_oper == OPR_INTCONST
       && WN_const_val(r_wn) == 1 ) {

    wn = r_wn;
    simplified = TRUE;
  }

  /* simplify 
   *          ( e && FALSE )
   * to
   *          ( e )
   */

  else
  if (    r_oper == OPR_INTCONST
       && WN_const_val(r_wn) == 0 ) {

    wn = l_wn;
    simplified = TRUE;
  }

  /* simplify 
   *          ( TRUE || e )
   * to
   *          ( TRUE )
   */

  else
  if (    l_oper == OPR_INTCONST
       && WN_const_val(l_wn) == 1 ) {

    wn = l_wn;
    simplified = TRUE;
  }

  /* simplify 
   *          ( FALSE || e )
   * to
   *          ( e )
   */

  else
  if (    l_oper == OPR_INTCONST
       && WN_const_val(l_wn) == 0 ) {

    wn = r_wn;
    simplified = TRUE;
  }

  /* simplify
   *           ( ( e1 && e2 ) || ( e1 && e3 ) )
   * to
   *           ( e1 && ( e2 || e3 ) )
   */

  else
  if (    l_oper == OPR_CAND
       && r_oper == OPR_CAND ) {

    ll_wn   = WN_kid0(l_wn);
    lr_wn   = WN_kid1(l_wn);
    rl_wn   = WN_kid0(r_wn);
    rr_wn   = WN_kid1(r_wn);

    if ( WN_Simp_Compare_Trees ( ll_wn, rl_wn ) == 0 ) {

      WN *r_wn_new = WN_Create ( OPC_I4CIOR, 2 );
      WN_kid0(r_wn_new) = lr_wn;
      WN_kid1(r_wn_new) = rr_wn;
      wn = WN_Create ( OPC_I4CAND, 2 );
      WN_kid0(wn) = ll_wn;
      WN_kid1(wn) = r_wn_new;
      simplified = TRUE;

      if ( Cur_PU_Feedback )
	Cur_PU_Feedback->FB_factor_circuit( l_wn, r_wn, wn, r_wn_new );
    }
  }
 
  /* simplify
   *          ( ( ( e & c1 ) != 0 ) || ( ( e & c2 ) != 0 ) )
   * to
   *          ( e & ( c1 | c2 ) )
   */

  else
  if (    l_oper == OPR_NE
       && r_oper == OPR_NE
       && l_opc  == r_opc ) {

    ll_wn   = WN_kid0(l_wn);
    lr_wn   = WN_kid1(l_wn);
    rl_wn   = WN_kid0(r_wn);
    rr_wn   = WN_kid1(r_wn);

    ll_oper = WN_operator(ll_wn);
    lr_oper = WN_operator(lr_wn);
    rl_oper = WN_operator(rl_wn);
    rr_oper = WN_operator(rr_wn);

    if (    ll_oper == OPR_BAND
         && lr_oper == OPR_INTCONST
         && WN_const_val(lr_wn) == 0
         && rl_oper == OPR_BAND
         && rr_oper == OPR_INTCONST
         && WN_const_val(rr_wn) == 0 ) {

      lll_wn = WN_kid0(ll_wn);
      llr_wn = WN_kid1(ll_wn);
      rll_wn = WN_kid0(rl_wn);
      rlr_wn = WN_kid1(rl_wn);

      llr_oper = WN_operator(llr_wn);
      rlr_oper = WN_operator(rlr_wn);

      if (    llr_oper == OPR_INTCONST
           && rlr_oper == OPR_INTCONST
           && WN_Simp_Compare_Trees ( lll_wn, rll_wn ) == 0 ) {

        ll_wn = lll_wn;
        lr_wn = llr_wn;
        WN_const_val(lr_wn) |= WN_const_val(rlr_wn);
        l_wn = WN_CreateExp2 ( WN_opcode(rl_wn), ll_wn, lr_wn );
        r_wn = rr_wn;
        wn = WN_CreateExp2 ( l_opc, l_wn, r_wn );
        simplified = TRUE;
      }
    }
  }

  if ( ! simplified ) {

    if (    WN_kid0(wn) != l_wn
         || WN_kid1(wn) != r_wn ) {

      wn = WN_Create ( OPC_I4CIOR, 2 );
      WN_kid0(wn) = l_wn;
      WN_kid1(wn) = r_wn;
    }
  }

  return wn;
} /* vho_simplify_cior */


static WN *
vho_simplify_cand_cior ( WN * wn )
{
  WN * lwn;
  WN * rwn;

  switch ( WN_opcode(wn) ) {

// >> WHIRL 0.30: replaced OPC_CAND by OPC_B and OPC_I4 variants
// TODO WHIRL 0.30: get rid of OPC_I4 variant
    case OPC_BCAND:
    case OPC_I4CAND:
// << WHIRL 0.30: replaced OPC_CAND by OPC_B and OPC_I4 variants

      lwn = vho_simplify_cand_cior ( WN_kid0(wn) );
      rwn = vho_simplify_cand_cior ( WN_kid1(wn) );
      wn  = vho_simplify_cand ( wn, lwn, rwn );
      break;

// >> WHIRL 0.30: replaced OPC_CIOR by OPC_B and OPC_I4 variants
// TODO WHIRL 0.30: get rid of OPC_I4 variant
    case OPC_BCIOR:
    case OPC_I4CIOR:
// << WHIRL 0.30: replaced OPC_CIOR by OPC_B and OPC_I4 variants

      lwn = vho_simplify_cand_cior ( WN_kid0(wn) );
      rwn = vho_simplify_cand_cior ( WN_kid1(wn) );
      wn  = vho_simplify_cior ( wn, lwn, rwn );
      break;
  }

  return wn;
} /* vho_simplify_cand_cior */


static WN *
vho_lower_cand_cior ( WN *wn, WN *block, BOOL_INFO *bool_info )
{
  WN        * one;
  WN        * zero;
  WN        * cflow_block1;
  WN        * cflow_block2;
  BOOL_INFO   cflow_bool_info;
  BOOL_INFO   cflow_bool_info1;
  BOOL_INFO   cflow_bool_info2;
  OPCODE      opcode;
  LABEL_IDX   join_label;
  WN        * lwn;
  WN        * rwn;
  PREG_NUM    preg;
  ST        * preg_st;
  TY_IDX      preg_ty_idx;

  opcode = WN_opcode(wn);

  cflow_block1 = WN_CreateBlock ();
  cflow_block2 = WN_CreateBlock ();

  WN_Set_Linenum ( cflow_block1, VHO_Srcpos );
  WN_Set_Linenum ( cflow_block2, VHO_Srcpos );

  if ( ! bool_info ) {

    vho_initialize_bool_info (&cflow_bool_info);
    LABEL_Init (New_LABEL (CURRENT_SYMTAB, cflow_bool_info.true_label), 
	0, LKIND_DEFAULT);
    LABEL_Init (New_LABEL (CURRENT_SYMTAB, cflow_bool_info.false_label),
	0, LKIND_DEFAULT);
    bool_info = &cflow_bool_info;
  }

  cflow_bool_info1 = *bool_info;
  cflow_bool_info1.opcode = opcode;

  if ( cflow_bool_info1.opcode != OPCODE_UNKNOWN )
    cflow_bool_info1.opcode = opcode;

  cflow_bool_info2 = cflow_bool_info1;

/*
  if (    bool_info->opcode != OPCODE_UNKNOWN
       && bool_info->opcode != opcode ) {
*/

  {

    if ( opcode == (OPC_I4CAND) ) {

      LABEL_Init (New_LABEL (CURRENT_SYMTAB, cflow_bool_info1.true_label),
	0, LKIND_DEFAULT);
      cflow_bool_info1.used_true_label  = FALSE;
    }

    else {

      LABEL_Init (New_LABEL (CURRENT_SYMTAB, cflow_bool_info1.false_label),
	0, LKIND_DEFAULT);
      cflow_bool_info1.used_false_label = FALSE;
    }
  }

  lwn = vho_lower_expr (WN_kid0(wn), cflow_block1, &cflow_bool_info1);
  rwn = vho_lower_expr (WN_kid1(wn), cflow_block2, &cflow_bool_info2);
  WN_kid0(wn) = lwn;
  WN_kid1(wn) = rwn;

  WN *wn_label, *wn_left_branch = NULL, *wn_right_branch = NULL;

  if ( WN_first(cflow_block1) )
    WN_INSERT_BlockLast ( block, cflow_block1 );

  if (    WN_first(cflow_block2)
       || cflow_bool_info1.used_true_label
       || cflow_bool_info1.used_false_label
       || cflow_bool_info2.used_true_label
       || cflow_bool_info2.used_false_label ) {

    if ( opcode == (OPC_I4CAND) ) {

      if ( lwn ) {

        wn_left_branch
	  = WN_CreateFalsebr ( cflow_bool_info1.false_label, lwn );
        WN_Set_Linenum ( wn_left_branch, VHO_Srcpos );
        WN_INSERT_BlockLast ( block, wn_left_branch );
      }
/*
      else
      if ( cflow_bool_info1.used_true_label ) {

        WN *wn_goto = WN_CreateGoto ( (ST_IDX) NULL,
                                      cflow_bool_info1.false_label );
        WN_Set_Linenum ( wn_goto, VHO_Srcpos );
        WN_INSERT_BlockLast ( block, wn_goto );
      }
*/
      if ( cflow_bool_info1.used_true_label ) {

        wn_label = WN_CreateLabel ( (ST_IDX) 0, cflow_bool_info1.true_label,
				    0, NULL );
        WN_Set_Linenum ( wn_label, VHO_Srcpos );
        WN_INSERT_BlockLast ( block, wn_label );
      }

      if ( WN_first(cflow_block2) )
        WN_INSERT_BlockLast ( block, cflow_block2 );

      if ( rwn ) {

        wn_right_branch
	  = WN_CreateFalsebr ( cflow_bool_info2.false_label, rwn );
        WN_Set_Linenum ( wn_right_branch, VHO_Srcpos );
        WN_INSERT_BlockLast ( block, wn_right_branch );

        if ( bool_info->opcode ) {

          if ( opcode != bool_info->opcode ) {

            WN *wn_goto
	      = WN_CreateGoto ( (ST_IDX) NULL, cflow_bool_info2.true_label );
            WN_Set_Linenum ( wn_goto, VHO_Srcpos );
            WN_INSERT_BlockLast ( block, wn_goto );

            cflow_bool_info2.used_true_label = TRUE;
          }
        }
      }

      bool_info->used_false_label = TRUE;
      bool_info->used_true_label |= cflow_bool_info2.used_true_label;

      if (Cur_PU_Feedback) {
	Cur_PU_Feedback->FB_lower_circuit( wn,
					   wn_left_branch, wn_right_branch );
      }
    }

    else {

      if ( lwn ) {

        wn_left_branch = WN_CreateTruebr ( cflow_bool_info1.true_label, lwn );
        WN_Set_Linenum ( wn_left_branch, VHO_Srcpos );
        WN_INSERT_BlockLast ( block, wn_left_branch );
      }

      else
      if ( cflow_bool_info1.used_false_label ) {

        WN *wn_goto
	  = WN_CreateGoto ( (ST_IDX) NULL, cflow_bool_info1.true_label );
        WN_Set_Linenum ( wn_goto, VHO_Srcpos );
        WN_INSERT_BlockLast ( block, wn_goto );
      }

      if ( cflow_bool_info1.used_false_label ) {

        wn_label
	  = WN_CreateLabel ( (ST_IDX) 0, cflow_bool_info1.false_label, 0, NULL );
        WN_Set_Linenum ( wn_label, VHO_Srcpos );
        WN_INSERT_BlockLast ( block, wn_label );
      }

      if ( WN_first(cflow_block2) )
        WN_INSERT_BlockLast ( block, cflow_block2 );

      if ( rwn ) {

	wn_right_branch
	  = WN_CreateFalsebr ( cflow_bool_info2.false_label, rwn );
	WN_Set_Linenum ( wn_right_branch, VHO_Srcpos );
	WN_INSERT_BlockLast ( block, wn_right_branch );
	cflow_bool_info2.used_false_label = TRUE;

        if ( bool_info->opcode != opcode
	     && bool_info->opcode != OPCODE_UNKNOWN ) {

	  WN *wn_goto
	    = WN_CreateGoto ( (ST_IDX) NULL, cflow_bool_info2.true_label );
	  WN_Set_Linenum ( wn_goto, VHO_Srcpos );
	  WN_INSERT_BlockLast ( block, wn_goto );
        }
      }
/*
      else
      if (    bool_info->opcode != opcode
           && bool_info->opcode != OPCODE_UNKNOWN ) {

	WN *wn_goto
	  = WN_CreateGoto ( (ST_IDX) NULL, cflow_bool_info2.false_label );
        WN_Set_Linenum ( wn_goto, VHO_Srcpos );
        WN_INSERT_BlockLast ( block, wn_goto );
        cflow_bool_info2.used_false_label = TRUE;
      }
*/
      bool_info->used_true_label   = TRUE;
      bool_info->used_false_label |= cflow_bool_info2.used_false_label;

      if (Cur_PU_Feedback)
	Cur_PU_Feedback->FB_lower_circuit( wn,
					   wn_left_branch, wn_right_branch );
    }

    if ( bool_info == &cflow_bool_info ) {

      // TODO: Frequency data for the following

      LABEL_Init (New_LABEL (CURRENT_SYMTAB, join_label),
	0, LKIND_DEFAULT);

      preg = Create_Preg (MTYPE_I4, vho_lower_cand_cior_name);

      preg_st     = MTYPE_To_PREG ( MTYPE_I4 );
      preg_ty_idx = Be_Type_Tbl(MTYPE_I4);

      if ( bool_info->used_true_label ) {

        wn = WN_CreateLabel ( (ST_IDX) 0, bool_info->true_label, 0, NULL );
        WN_Set_Linenum ( wn, VHO_Srcpos );
        WN_INSERT_BlockLast ( block, wn );
      }

      one = WN_CreateIntconst ( OPC_I4INTCONST, 1 );
      wn  = WN_CreateStid ( OPC_I4STID, preg, preg_st, preg_ty_idx, one );
      WN_Set_Linenum ( wn, VHO_Srcpos );
      WN_INSERT_BlockLast ( block, wn );

      wn = WN_CreateGoto ( (ST_IDX) NULL, join_label );
      WN_Set_Linenum ( wn, VHO_Srcpos );
      WN_INSERT_BlockLast ( block, wn );

      wn = WN_CreateLabel ( (ST_IDX) 0, bool_info->false_label, 0, NULL );
      WN_Set_Linenum ( wn, VHO_Srcpos );
      WN_INSERT_BlockLast ( block, wn );

      zero    = WN_CreateIntconst ( OPC_I4INTCONST, 0 );
      wn      = WN_CreateStid ( OPC_I4STID, preg, preg_st, preg_ty_idx, zero );
      WN_Set_Linenum ( wn, VHO_Srcpos );
      WN_INSERT_BlockLast ( block, wn );

      wn = WN_CreateLabel ( (ST_IDX) 0, join_label, 0, NULL );
      WN_Set_Linenum ( wn, VHO_Srcpos );
      WN_INSERT_BlockLast ( block, wn );

      wn      = WN_CreateLdid ( OPC_I4I4LDID, preg, preg_st, preg_ty_idx );
    }

    else

      wn = NULL;
  }

  else {

    wn = vho_simplify_cand_cior ( wn );
  }

  return wn;
} /* vho_lower_cand_cior */


static WN *
vho_lower_cselect ( WN * wn_cselect, WN * block, BOOL_INFO * bool_info )
{
  WN        * wn = wn_cselect;
  WN        * cflow_block;
  WN        * cflow_block1;
  WN        * cflow_block2;
  BOOL_INFO   cflow_bool_info;
  WN        * test;
  WN        * lwn;
  WN        * rwn;
  TYPE_ID     preg_mtype;
  PREG_NUM    preg;
  ST        * preg_st;
  TY_IDX      preg_ty_idx;
  OPCODE      opcode;
  OPCODE      addopc;
  OPCODE      intopc;
  LABEL_IDX   join_label;
  BOOL        mcselect;
  BOOL        vcselect;
  TY_IDX      ty_idx;
  WN_OFFSET   offset;
  WN_OFFSET   lwn_offset;
  WN_OFFSET   rwn_offset;
  WN        * swn;
  BOOL        has_side_effects;

  vcselect     = FALSE;
  mcselect     = FALSE;
  cflow_block  = WN_CreateBlock ();
  cflow_block1 = WN_CreateBlock ();
  cflow_block2 = WN_CreateBlock ();

  WN_Set_Linenum ( cflow_block,  VHO_Srcpos );
  WN_Set_Linenum ( cflow_block1, VHO_Srcpos );
  WN_Set_Linenum ( cflow_block2, VHO_Srcpos );

  vho_initialize_bool_info (&cflow_bool_info);

  LABEL_Init (New_LABEL (CURRENT_SYMTAB, cflow_bool_info.true_label),
	0, LKIND_DEFAULT);
  LABEL_Init (New_LABEL (CURRENT_SYMTAB, cflow_bool_info.false_label),
	0, LKIND_DEFAULT);

  opcode = WN_opcode(wn_cselect);
  test   = vho_lower_expr ( WN_kid0(wn_cselect), cflow_block,
			    &cflow_bool_info );
  lwn    = vho_lower_expr ( WN_kid1(wn_cselect), cflow_block1, NULL );
  rwn    = vho_lower_expr ( WN_kid2(wn_cselect), cflow_block2, NULL );

  if (    WHIRL_Mldid_Mstid_On
       && opcode == OPC_MCSELECT ) {
#ifdef TARG_ST
    // [CG]: In the case where one of the labels was already used by the test
    // we cannot convert to an IF. We generate the correct control flow
    // directly (as for the non MCSELECT case).
    if (cflow_bool_info.used_true_label ||
	cflow_bool_info.used_false_label) {
      // We generate:
      // CSELECT                             <cflow_block>
      //  <test_expr>                        LABEL true_label
      //  <then_expr>                        <cflow_block1>
      //  <else_expr>            ===>        STID .mcselect
      //				     GOTO join_label
      //				     LABEL false_label
      //				     <cflow_block2>
      //				     STID .mcselect
      //				     LABEL join_label
      //				     LDID .mcselect

      ty_idx = WN_object_ty(lwn);
      ST* temp = Gen_Temp_Symbol (ty_idx, ".mcselect");

      // Insert the test block
      if ( WN_first(cflow_block) )
	WN_INSERT_BlockLast ( block, cflow_block );
      
      // Create the join label
      LABEL_Init (New_LABEL (CURRENT_SYMTAB, join_label),
		  0, LKIND_DEFAULT);
      // Create the true label if used
      if (cflow_bool_info.used_true_label) {
	wn = WN_CreateLabel ( (ST_IDX) 0, cflow_bool_info.true_label, 0, NULL );
	WN_Set_Linenum ( wn, VHO_Srcpos );
	WN_INSERT_BlockLast ( block, wn );
      }
      // Insert the code for the true block
      if ( WN_first(cflow_block1 ) )
	WN_INSERT_BlockLast ( block, cflow_block1 );
      // Insert the final Store
      lwn = WN_CreateStid (OPR_STID, MTYPE_V, MTYPE_M,
			   0, ST_st_idx (temp), ty_idx, lwn);
      WN_Set_Linenum ( lwn, VHO_Srcpos );
      WN_INSERT_BlockLast ( block, lwn );
      // Insert the goto
      wn = WN_CreateGoto ( (ST_IDX) NULL, join_label );
      WN_Set_Linenum ( wn, VHO_Srcpos );
      if (Cur_PU_Feedback)
	Cur_PU_Feedback->Annot( wn, FB_EDGE_OUTGOING, FB_FREQ_UNKNOWN );
      WN_INSERT_BlockLast ( block, wn );
      // Insert the false label
      wn = WN_CreateLabel ( (ST_IDX) 0, cflow_bool_info.false_label, 0, NULL );
      WN_Set_Linenum ( wn, VHO_Srcpos );
      WN_INSERT_BlockLast ( block, wn );
      // Insert the code for the false block
      if ( WN_first(cflow_block2 ) )
	WN_INSERT_BlockLast ( block, cflow_block2 );
      // Insert the final Store
      rwn = WN_CreateStid (OPR_STID, MTYPE_V, MTYPE_M,
			   0, ST_st_idx (temp), ty_idx, rwn);
      WN_Set_Linenum ( rwn, VHO_Srcpos );
      WN_INSERT_BlockLast ( block, rwn );
      // Insret the join label
      wn = WN_CreateLabel ( (ST_IDX) 0, join_label, 0, NULL );
      WN_Set_Linenum ( wn, VHO_Srcpos );
      WN_INSERT_BlockLast ( block, wn );
      // Insert the final Load
      wn = WN_CreateLdid (OPR_LDID, MTYPE_M, MTYPE_M, 
			  0, ST_st_idx (temp), ty_idx);
      return wn;
    }
#endif

    // CSELECT                             IF
    //  <test>                              <test>
    //  <then_expr>                        THEN
    //  <else_expr>            ===>          <then_expr>
    //                                      STID <preg>
    //                                     ELSE
    //                                       <else_expr>
    //                                      STID <preg>
    //                                     END_IF

#ifdef TARG_ST
    // [CG]: Must get the real object type (for structure fields)
    ty_idx = WN_object_ty(lwn);
#else
    ty_idx = WN_ty(lwn);
#endif
#if defined( KEY) && !defined(TARG_ST) // bug 5179
    if (WN_field_id(lwn) != 0)
      ty_idx = get_field_type(ty_idx, WN_field_id(lwn));
#endif
    ST* temp = Gen_Temp_Symbol (ty_idx, ".mcselect");

    wn = WN_CreateStid (OPR_STID, MTYPE_V, MTYPE_M,
                        0, ST_st_idx (temp), ty_idx, lwn);
    WN_Set_Linenum (wn, VHO_Srcpos);
    WN_INSERT_BlockLast (cflow_block1, wn);

    wn = WN_CreateStid (OPR_STID, MTYPE_V, MTYPE_M,
                        0, ST_st_idx (temp), ty_idx, rwn);
    WN_Set_Linenum (wn, VHO_Srcpos);
    WN_INSERT_BlockLast (cflow_block2, wn);

    // Bug Compiler-8761
    // Handong: For MCSELECT, test may return NULL, so it's bad idea to use
    // WN_CreateIf() without checking. I think creating the if.then.else
    // as the normal CSELECT is a better way.
    //
    if( test ){
      wn = WN_CreateIf (test, cflow_block1, cflow_block2);
      WN_Set_Linenum ( wn, VHO_Srcpos );
      if ( Cur_PU_Feedback ) {
        Cur_PU_Feedback->FB_lower_branch( wn_cselect, wn );
      }

      if (WN_first (cflow_block))
        WN_INSERT_BlockLast (block, cflow_block);

      WN_INSERT_BlockLast (block, wn);

    } else {

      if (WN_first (cflow_block))
        WN_INSERT_BlockLast (block, cflow_block);

      if( cflow_bool_info.used_true_label || cflow_bool_info.used_false_label ){
        LABEL_Init( New_LABEL (CURRENT_SYMTAB, join_label), 0, LKIND_DEFAULT );

        if( cflow_bool_info.used_true_label ){
          wn = WN_CreateLabel( (ST_IDX) 0, cflow_bool_info.true_label, 0, NULL );
          WN_Set_Linenum( wn, VHO_Srcpos );
          WN_INSERT_BlockLast( block, wn );
        }

        if( WN_first(cflow_block1 ) )
          WN_INSERT_BlockLast( block, cflow_block1 );

        wn = WN_CreateGoto ( (ST_IDX) NULL, join_label );
        WN_Set_Linenum ( wn, VHO_Srcpos );
        if (Cur_PU_Feedback)
          Cur_PU_Feedback->Annot( wn, FB_EDGE_OUTGOING, FB_FREQ_UNKNOWN );
        WN_INSERT_BlockLast ( block, wn );

        wn = WN_CreateLabel( (ST_IDX) 0, cflow_bool_info.false_label, 0, NULL );
        WN_Set_Linenum( wn, VHO_Srcpos );
        WN_INSERT_BlockLast( block, wn );

        if( WN_first(cflow_block2 ) )
          WN_INSERT_BlockLast( block, cflow_block2 );

        wn = WN_CreateLabel ( (ST_IDX) 0, join_label, 0, NULL );
        WN_Set_Linenum ( wn, VHO_Srcpos );

        WN_INSERT_BlockLast ( block, wn );
      }
    }

    wn = WN_CreateLdid (OPR_LDID, MTYPE_M, MTYPE_M, 
                        0, ST_st_idx (temp), ty_idx);
    return wn;
  }

  if (    VHO_Cselect_Opt
       && WN_first(cflow_block) == NULL
       && WN_first(cflow_block1) == NULL
       && WN_first(cflow_block2) == NULL
       && cflow_bool_info.used_true_label == FALSE
       && cflow_bool_info.used_false_label == FALSE ) {

    if ( opcode == OPC_VCSELECT ) {

    }

    else
    if ( opcode == OPC_MCSELECT ) {

    }

    else {

      has_side_effects =    VHO_WN_has_side_effects ( test )
                         || VHO_WN_has_side_effects ( lwn )
                         || VHO_WN_has_side_effects ( rwn );

      if ( has_side_effects == FALSE ) {

        /* kt != 0 ? kt : 0 => kt */
#ifdef KEY // bug 5363 and bug 5817
        if ( ( ! MTYPE_is_float(WN_desc(test)) || ! Force_IEEE_Comparisons) &&
	     WN_operator(test) == OPR_NE && 
	     VHO_WN_is_zero(WN_kid1(test)) && 
	     WN_Simp_Compare_Trees ( WN_kid0(test), lwn ) == 0 &&
	     VHO_WN_is_zero(rwn) ) 
          return lwn;
#endif

#ifdef KEY
	/* (a-b) >= 0 ? (a-b) : (b-a) => abs(a-b) */
        if( ( WN_operator(test) == OPR_GT ||
	      WN_operator(test) == OPR_GE ) &&
	    VHO_WN_is_zero(WN_kid1(test))   &&
	    WN_Simp_Compare_Trees( WN_kid0(test), lwn ) == 0 ){

	  if( WN_operator(rwn) == OPR_SUB          &&
	      WN_operator(lwn) == WN_operator(rwn) &&
	      WN_Simp_Compare_Trees( WN_kid0(rwn), WN_kid1(lwn) ) == 0 &&
	      WN_Simp_Compare_Trees( WN_kid1(rwn), WN_kid0(lwn) ) == 0 ){
	    
	    opcode = OPCODE_make_op ( OPR_ABS,
				      WN_rtype(wn),
				      MTYPE_V );
	    wn = WN_CreateExp1 ( opcode, lwn );
	    return wn;
	  }
	}

#ifdef TARG_X8664
	/* Handle saturation arithmetic SUB operator by converting it 
	 * to an intrinsic 
	 * x =  (y >= 0x8000) ? y - 0x8000 : 0; 
	 */
	
	if ( WN_operator(test) == OPR_GT &&
	     WN_rtype(test) == MTYPE_I4 &&
	     WN_desc(test) == MTYPE_U4 &&
	     WN_operator(WN_kid1(test)) == OPR_INTCONST &&
	     WN_const_val(WN_kid1(test)) == 0x7fff &&

	     WN_operator(WN_kid2(wn_cselect)) == OPR_INTCONST &&
	     WN_const_val(WN_kid2(wn_cselect)) == 0 &&

	     WN_operator(WN_kid1(wn_cselect)) == OPR_CVTL &&
	     WN_cvtl_bits(WN_kid1(wn_cselect)) == 16 &&
	     WN_operator(WN_kid0(WN_kid1(wn_cselect))) == OPR_SUB &&
	     WN_operator(WN_kid1(WN_kid0(WN_kid1(wn_cselect)))) == 
	     OPR_INTCONST &&
	     WN_const_val(WN_kid1(WN_kid0(WN_kid1(wn_cselect)))) == 0x8000 &&

	     WN_has_sym(WN_kid0(test)) && 
	     WN_has_sym(WN_kid0(WN_kid0(WN_kid1(wn_cselect)))) &&
	     WN_st(WN_kid0(test)) == 
	     WN_st(WN_kid0(WN_kid0(WN_kid1(wn_cselect)))) ) {
	  WN* kids[2];
	  kids[0] = WN_kid0(test);
	  kids[1] = WN_kid1(WN_kid0(WN_kid1(wn_cselect)));
	  // Create parm nodes for intrinsic op
	  kids[0] = WN_CreateParm (MTYPE_U4, kids[0], 
				   Be_Type_Tbl(MTYPE_U4),
	  			   WN_PARM_BY_VALUE);
	  kids[1] = WN_CreateParm (MTYPE_U4, kids[1], 
				   Be_Type_Tbl(MTYPE_U4),
	  			   WN_PARM_BY_VALUE);
	  wn = WN_Create_Intrinsic(OPCODE_make_op(OPR_INTRINSIC_OP,
						  MTYPE_U4, MTYPE_V),
				   INTRN_SUBSU2, 2, kids);
	  return wn;
	  
	}
#endif
#endif

        /* x > 0 ? x : -x => abs(x) */

        if (    (    WN_operator(test) == OPR_GT
                  || WN_operator(test) == OPR_GE )
             && VHO_WN_is_zero(WN_kid1(test))
             && WN_Simp_Compare_Trees ( WN_kid0(test), lwn ) == 0
             && WN_operator(rwn) == OPR_NEG
             && WN_Simp_Compare_Trees ( lwn, WN_kid0(rwn) ) == 0 ) {

          DevWarn ( "%s: %s %s\n",
                    OPCODE_name(WN_opcode(wn)),
                    MTYPE_name(WN_rtype(wn)),
                    MTYPE_name(WN_desc(wn)) );

          if ( MTYPE_signed(WN_rtype(wn)) ) {

            opcode = OPCODE_make_op ( OPR_ABS,
                                      WN_rtype(wn),
                                      MTYPE_V );
            wn = WN_CreateExp1 ( opcode, lwn );
/*
            fdump_tree ( stderr, wn );
*/
            return wn;
          }
        }

        /* x >= y ? x : y => max(x) */

        if (    (    WN_operator(test) == OPR_GT
                  || WN_operator(test) == OPR_GE )
             && WN_Simp_Compare_Trees ( WN_kid0(test), lwn ) == 0
             && WN_Simp_Compare_Trees ( WN_kid1(test), rwn ) == 0 ) {

          DevWarn ( "%s: %s %s\n",
                    OPCODE_name(WN_opcode(wn)),
                    MTYPE_name(WN_rtype(wn)),
                    MTYPE_name(WN_desc(wn)) );

          opcode = OPCODE_make_op ( OPR_MAX,
                                    WN_rtype(wn),
                                    MTYPE_V );
          wn = WN_CreateExp2 ( opcode, lwn, rwn );
/*
          fdump_tree ( stderr, wn );
*/
          return wn;
        }

        /* x <= y ? x : y => min(x) */

        if (    (    WN_operator(test) == OPR_LT
                  || WN_operator(test) == OPR_LE )
             && WN_Simp_Compare_Trees ( WN_kid0(test), lwn ) == 0
             && WN_Simp_Compare_Trees ( WN_kid1(test), rwn ) == 0 ) {

          DevWarn ( "%s: %s %s\n",
                    OPCODE_name(WN_opcode(wn)),
                    MTYPE_name(WN_rtype(wn)),
                    MTYPE_name(WN_desc(wn)) );

          opcode = OPCODE_make_op ( OPR_MIN,
                                    WN_rtype(wn),
                                    MTYPE_V );
          wn = WN_CreateExp2 ( opcode, lwn, rwn );
/*
          fdump_tree ( stderr, wn );
*/
          return wn;
        }
      }
    }
  }
 
  if ( WN_first(cflow_block) )
    WN_INSERT_BlockLast ( block, cflow_block );

  if ( opcode == OPC_VCSELECT )
    vcselect = TRUE;

  else {

    if ( opcode == OPC_MCSELECT ) {

      FmtAssert ( (    WN_opcode(lwn) == OPC_MLOAD
                    && WN_opcode(rwn) == OPC_MLOAD ),
                  ("unimplemented case encountered in OPC_MCSELECT") );

      mcselect   = TRUE;
      preg_mtype = Pointer_Mtype;
      lwn_offset = WN_offset(lwn);
      rwn_offset = WN_offset(rwn);
      swn        = WN_kid1(lwn);
      ty_idx     = WN_ty(lwn);
      lwn        = WN_kid0(lwn);
      rwn        = WN_kid0(rwn);

      if ( lwn_offset == rwn_offset )
        offset = lwn_offset;

      else {

        offset = 0;
        addopc = OPCODE_make_op ( OPR_ADD, Pointer_Mtype, MTYPE_V );
        intopc = OPCODE_make_op ( OPR_INTCONST, Pointer_Mtype, MTYPE_V );

        if ( lwn_offset )
          lwn = WN_CreateExp2 ( addopc,
                                lwn,
                                WN_CreateIntconst ( intopc, lwn_offset ) );

        if ( rwn_offset )
          rwn = WN_CreateExp2 ( addopc,
                                rwn,
                                WN_CreateIntconst ( intopc, rwn_offset ) );
      }
    }

    else {

      preg_mtype = WN_rtype(wn);
    }

    vho_lower_set_st_addr_info ( lwn, ADDRESS_SAVED );
    vho_lower_set_st_addr_info ( rwn, ADDRESS_SAVED );

    preg = Create_Preg (preg_mtype, vho_lower_cselect_name);

    preg_st     = MTYPE_To_PREG ( preg_mtype );
    preg_ty_idx = Be_Type_Tbl(preg_mtype);
    opcode      = OPCODE_make_op ( OPR_STID, MTYPE_V, preg_mtype );
  }

  if (    cflow_bool_info.used_true_label
       || cflow_bool_info.used_false_label ) {

    LABEL_Init (New_LABEL (CURRENT_SYMTAB, join_label),
	0, LKIND_DEFAULT);

    if ( cflow_bool_info.used_true_label ) {

      wn = WN_CreateLabel ( (ST_IDX) 0, cflow_bool_info.true_label, 0, NULL );
      WN_Set_Linenum ( wn, VHO_Srcpos );

      WN_INSERT_BlockLast ( block, wn );
    }

    if ( WN_first(cflow_block1 ) )
      WN_INSERT_BlockLast ( block, cflow_block1 );

    if ( ! vcselect ) {

      lwn = WN_CreateStid ( opcode, preg, preg_st, preg_ty_idx, lwn );
      WN_Set_Linenum ( lwn, VHO_Srcpos );

      WN_INSERT_BlockLast ( block, lwn );
    }

    wn = WN_CreateGoto ( (ST_IDX) NULL, join_label );
    WN_Set_Linenum ( wn, VHO_Srcpos );
    if (Cur_PU_Feedback)
      Cur_PU_Feedback->Annot( wn, FB_EDGE_OUTGOING, FB_FREQ_UNKNOWN );

    WN_INSERT_BlockLast ( block, wn );

    wn = WN_CreateLabel ( (ST_IDX) 0, cflow_bool_info.false_label, 0, NULL );
    WN_Set_Linenum ( wn, VHO_Srcpos );

    WN_INSERT_BlockLast ( block, wn );

    if ( WN_first(cflow_block2 ) )
      WN_INSERT_BlockLast ( block, cflow_block2 );

    if ( ! vcselect ) {

      rwn = WN_CreateStid ( opcode, preg, preg_st, preg_ty_idx, rwn );
      WN_Set_Linenum ( rwn, VHO_Srcpos );

      WN_INSERT_BlockLast ( block, rwn );
    }

    wn = WN_CreateLabel ( (ST_IDX) 0, join_label, 0, NULL );
    WN_Set_Linenum ( wn, VHO_Srcpos );

    WN_INSERT_BlockLast ( block, wn );
  }

  else {

    if ( ! vcselect ) {

      // CSELECT                             IF
      //  <test>                              <test>
      //  <then_expr>                        THEN
      //  <else_expr>            ===>          <then_expr>
      //                                      STID <preg>
      //                                     ELSE
      //                                       <else_expr>
      //                                      STID <preg>
      //                                     END_IF

      lwn = WN_CreateStid ( opcode, preg, preg_st, preg_ty_idx, lwn );
      WN_Set_Linenum ( lwn, VHO_Srcpos );

      WN_INSERT_BlockLast ( cflow_block1, lwn );

      rwn = WN_CreateStid ( opcode, preg, preg_st, preg_ty_idx, rwn );
      WN_Set_Linenum ( rwn, VHO_Srcpos );

      WN_INSERT_BlockLast ( cflow_block2, rwn );
    }

    wn = WN_CreateIf ( test, cflow_block1, cflow_block2 );
    WN_Set_Linenum ( wn, VHO_Srcpos );
    if ( Cur_PU_Feedback ) {
      Cur_PU_Feedback->FB_lower_branch( wn_cselect, wn );
    }

    WN_INSERT_BlockLast ( block, wn );
  }

  if ( !vcselect ) {

    opcode = OPCODE_make_op ( OPR_LDID, Promoted_Mtype (preg_mtype),
                              preg_mtype );
    wn = WN_CreateLdid ( opcode, preg, preg_st, preg_ty_idx );

    if ( mcselect )
      wn = WN_CreateMload ( offset, ty_idx, wn, swn );
  }

  else
    wn = NULL;

  return wn;
} /* vho_lower_cselect */


#if VHO_DEBUG
#define record_combine_loads_failure(arg1,arg2) \
  fprintf (stderr, "[%d] combine_load_failure at %d of vho_lower.c\n", \
	   arg1, arg2);
#else
#define record_combine_loads_failure(x,y)
#endif /* VHO_DEBUG */

WN *
vho_lower_combine_loads ( WN * wn )
{
  TYPE_ID    mtype;
  OPCODE     opcode;
  OPCODE     add_opc;
  OPCODE     or_opc;
  OPCODE     shift_opc;
  OPCODE     load_opcode;
  OPERATOR   wn_operator;
  OPERATOR   load_operator;
  INT32      bytes;
  INT32      size;
  INT32      item_size;
  INT32      last_item_size;
  INT32      shift_bits;
  INT32      shift_bytes;
  INT32      last_index;
  WN       * source;
  WN       * lwn;
  WN       * swn;
  WN       * twn;
  WN       * awn1;
  WN       * awn2;
  BOOL       combine;

  opcode = WN_opcode(wn);

  if ( opcode == OPC_U4ADD || opcode == OPC_U4BIOR ) {

    add_opc   = OPC_U4ADD;
    or_opc    = OPC_U4BIOR;
    shift_opc = OPC_U4SHL;
    size      = 4;
  }

  else
  if ( opcode == OPC_U8ADD || opcode == OPC_U8BIOR ) {

    add_opc   = OPC_U8ADD;
    or_opc    = OPC_U8BIOR;
    shift_opc = OPC_U8SHL;
    size      = 8;
  }

  else {

    record_combine_loads_failure ((INT32) VHO_Srcpos, __LINE__);
    return wn;
  }

  bytes   = 0;
  source  = NULL;
  combine = TRUE;
  lwn     = WN_kid0(wn);
  swn     = WN_kid1(wn);

  if ( WN_opcode(swn) != shift_opc ) {

    if ( WN_opcode(lwn) == shift_opc ) {

      swn = lwn;
      lwn = WN_kid1(wn);
    }

    else {

      combine = FALSE;
      record_combine_loads_failure ((INT32) VHO_Srcpos, __LINE__);
    }
  }

  mtype = WN_desc(lwn);

  if ( !MTYPE_is_unsigned(mtype) ) {

    combine = FALSE;
    record_combine_loads_failure ((INT32) VHO_Srcpos,  __LINE__);
  }

  load_opcode   = WN_opcode(lwn);
  load_operator = OPCODE_operator(load_opcode);

  if (    combine
       && WN_operator(WN_kid1(swn)) == OPR_INTCONST
       && ( load_operator == OPR_LDID || load_operator == OPR_ILOAD ) ) {

    bytes  = TY_size(WN_ty(lwn));
    source = lwn;
    last_item_size = bytes;

    if (    load_operator == OPR_ILOAD
         && WN_operator(WN_kid0(lwn)) == OPR_ARRAY )
      last_index = WN_kid_count(WN_kid0(lwn)) - 1;

    while ( swn ) {

      shift_bits = WN_const_val(WN_kid1(swn));

      if ( shift_bits == 0 || shift_bits % 8 ) {

        combine = FALSE;
        record_combine_loads_failure ((INT32) VHO_Srcpos, __LINE__);
        break;
      }

      shift_bytes = shift_bits >> 3;

      lwn         = WN_kid0(swn);
      opcode      = WN_opcode(lwn);
      wn_operator = OPCODE_operator(opcode);

      if ( wn_operator == OPR_LDID || wn_operator == OPR_ILOAD )
        swn = NULL;

      else
      if ( opcode == add_opc || opcode == or_opc ) {

        swn = WN_kid1(lwn);
        lwn = WN_kid0(lwn);

        if ( WN_opcode(swn) != shift_opc ) {

          if ( WN_opcode(lwn) == shift_opc ) {

            twn = lwn;
            lwn = swn;
            swn = twn;
          }

          else {

            combine = FALSE;
            record_combine_loads_failure ((INT32) VHO_Srcpos, __LINE__);
            break;
          }
        }

        if ( WN_operator(WN_kid1(swn)) != OPR_INTCONST ) {

          combine = FALSE;
          record_combine_loads_failure ((INT32) VHO_Srcpos, __LINE__);
          break;
        }

        wn_operator = WN_operator(lwn);

        if ( wn_operator != OPR_LDID && wn_operator != OPR_ILOAD ) {

          combine = FALSE;
          record_combine_loads_failure ((INT32) VHO_Srcpos, __LINE__);
          break;
        }
      }

      else {

        combine = FALSE;
        record_combine_loads_failure ((INT32) VHO_Srcpos, __LINE__);
        break;
      }

      item_size = TY_size(WN_ty(lwn));

      if ( item_size != shift_bytes ) {

        combine = FALSE;
        record_combine_loads_failure ((INT32) VHO_Srcpos, __LINE__);
        break;
      }

      bytes += item_size;

      if ( bytes > size ) {

        combine = FALSE;
        record_combine_loads_failure ((INT32) VHO_Srcpos, __LINE__);
        break;
      }

      opcode      = WN_opcode(lwn);
      wn_operator = OPCODE_operator(opcode);

      if ( wn_operator != load_operator ) {

        combine = FALSE;
        record_combine_loads_failure ((INT32) VHO_Srcpos,  __LINE__);
        break;
      }

      mtype = OPCODE_desc(opcode);

      if ( !MTYPE_is_unsigned(mtype) ) {

        combine = FALSE;
        record_combine_loads_failure ((INT32) VHO_Srcpos, __LINE__);
        break;
      }

      if ( load_operator == OPR_LDID ) {

        if (    WN_st(source) != WN_st(lwn)
             && WN_offset(source) != WN_offset(lwn) + last_item_size ) {

          combine = FALSE;
          record_combine_loads_failure ((INT32) VHO_Srcpos, __LINE__);
          break;
        }
      }

      else
      if ( load_operator == OPR_ILOAD ) {

        awn1 = WN_kid0(source);
        awn2 = WN_kid0(lwn);

        if ( WN_load_offset(source) == WN_load_offset(lwn) + last_item_size ) {

          if ( WN_Simp_Compare_Trees ( awn1, awn2 ) ) {

            combine = FALSE;
            record_combine_loads_failure ((INT32) VHO_Srcpos, __LINE__);
            break;
          }
        }

        else
        if ( WN_load_offset(source) == WN_load_offset(lwn) ) {

          if (    WN_operator(awn1) == OPR_ARRAY
               && WN_operator(WN_kid(awn1,last_index)) == OPR_INTCONST ) {

            WN_const_val(WN_kid(awn1,last_index)) -= 1;

            if ( WN_Simp_Compare_Trees ( awn1, awn2 ) ) {

              WN_const_val(WN_kid(awn1,last_index)) += 1;
              combine = FALSE;
              record_combine_loads_failure ((INT32) VHO_Srcpos, __LINE__);
              break;
            }

            else
              WN_const_val(WN_kid(awn1,last_index)) += 1;
          }

          else {

            combine = FALSE;
            record_combine_loads_failure ((INT32) VHO_Srcpos, __LINE__);
            break;
          }
        }

        else {

          combine = FALSE;
          record_combine_loads_failure ((INT32) VHO_Srcpos,  __LINE__);
          break;
        }
      }
      
      else {

        combine = FALSE;
        record_combine_loads_failure ((INT32) VHO_Srcpos, __LINE__);
        break;
      }

      source = lwn;
      last_item_size = item_size;
    }
  }

  if ( combine && bytes == size ) {

    TY_IDX aty_idx;
    TY_IDX uty_idx;
    TY_IDX pty_idx;

    if ( size == 4 ) {

      aty_idx = Be_Type_Tbl(MTYPE_U4);
      uty_idx = vho_u4a1_ty_idx;

      if ( uty_idx == (TY_IDX) 0 ) {

        uty_idx = aty_idx;
        Set_TY_align (uty_idx, 1);
        pty_idx = Make_Pointer_Type ( uty_idx, TRUE );

        vho_u4a1_ty_idx = uty_idx;
      }

      else
        pty_idx = TY_pointer(uty_idx);
    }

    else {

      aty_idx = Be_Type_Tbl(MTYPE_U8);
      uty_idx = vho_u8a1_ty_idx;

      if ( uty_idx == (TY_IDX) 0 ) {

        uty_idx = aty_idx;
        Set_TY_align (uty_idx, 1);
        pty_idx = Make_Pointer_Type ( uty_idx, TRUE );

        vho_u8a1_ty_idx = uty_idx;
      }

      else
        pty_idx = TY_pointer(uty_idx);
    }

    if ( load_operator == OPR_LDID ) {

      lwn = WN_COPY_Tree(lwn);
      WN_DELETE_Tree(wn);
      wn = lwn;
      opcode = size == 4 ? OPC_U4U4LDID : OPC_U8U8LDID;
      WN_set_opcode(wn,opcode);
      WN_set_ty(wn,uty_idx);
    }

    else {

      lwn = WN_COPY_Tree(lwn);
      WN_DELETE_Tree(wn);
      wn = lwn;
      opcode = size == 4 ? OPC_U4U4ILOAD : OPC_U8U8ILOAD;
      WN_set_opcode(wn,opcode);
      WN_set_ty(wn,aty_idx);
      WN_set_load_addr_ty(wn,pty_idx);
    }
  }

  return wn;
} /* vho_lower_combine_loads */

#ifdef KEY
#ifdef TARG_X8664
// Utility function to traverse through an aggregate type and determine
// if any field/element is floating-point: bug 7770
// Called by vho_lower_mparm(), if there is any FP field, the mparm cannot
// be transformed.
// Return 1 if OK to transform, 0 if cannot be transformed.
static inline BOOL
traverse_struct (const TY_IDX ty)
{
  Is_True (TY_size(ty) <= 8, ("Type size cannot exceed 8"));
  if (TY_kind (ty) == KIND_STRUCT)
  {
    FLD_ITER fld_iter = Make_fld_iter(TY_fld(ty));
    do
    {
      FLD_HANDLE fld(fld_iter);
      if (TY_size(FLD_type(fld)) == 0)
        continue;
      if (!traverse_struct (FLD_type(fld))) return 0;
    } while (!FLD_last_field(fld_iter++));
  }
  else if (TY_kind (ty) == KIND_ARRAY)
    return traverse_struct (TY_etype (ty));
  else
    switch (TY_mtype (ty))
    {
      case MTYPE_F4:
      case MTYPE_F8:
      case MTYPE_C4: return 0;
      default: return 1;
    }

  return 1;
}
#endif
#ifdef TARG_MIPS
// Utility function to traverse through an aggregate type and determine
// it contains either 1 or 2 floating-point fields, in which case the mparm
// cannot be transformed; called by vho_lower_mparm().
// Return 1 if OK to transform, 0 if cannot be transformed.
static inline BOOL
traverse_struct (const TY_IDX ty)
{
  Is_True (TY_size(ty) <= 8, ("Type size cannot exceed 8"));
  if (TY_kind (ty) == KIND_STRUCT)
  {
    FLD_ITER fld_iter = Make_fld_iter(TY_fld(ty));
    TYPE_ID mtype1 = TY_mtype(FLD_type(fld_iter));
    if ((mtype1 == MTYPE_C4 || mtype1 == MTYPE_C8 ||
         mtype1 == MTYPE_F4 || mtype1 == MTYPE_F8) && FLD_last_field(fld_iter))
      return 0;
    else if (mtype1 == MTYPE_F4 || mtype1 == MTYPE_F8) {
      if (FLD_last_field(++fld_iter)) {
	TYPE_ID mtype2 = TY_mtype(FLD_type(fld_iter));
	if (mtype2 == MTYPE_F4 || mtype2 == MTYPE_F8)
	  return 0;
      }
    }
  }
  else
    switch (TY_mtype (ty))
    {
      case MTYPE_F4:
      case MTYPE_F8:
      case MTYPE_C4: return 0;
      default: return 1;
    }

  return 1;
}
#endif

// If the struct under MPARM is of size 4 or 8, change LDID from M
// type to the appropriate type (U4/U8).
static inline WN *
vho_lower_mparm (WN * wn)
{
  WN * kid = WN_kid0 (wn);
  if (WN_opcode (kid) != OPC_MMLDID) return wn;

  TY_IDX ty = WN_ty (kid);
  INT64 bytes = TY_size (ty);

  if (VHO_Struct_Opt &&
      (bytes == 4 || bytes == 8) &&
      TY_size (WN_ty (wn)) == bytes &&
      ST_class (WN_st (kid)) != CLASS_PREG
#ifdef TARG_X8664
      && (Is_Target_32bit() ||
          traverse_struct (WN_ty (wn)))
#elif defined(TARG_MIPS) // bug 12809
      && traverse_struct (WN_ty(wn))
#endif
      )
  {
    TYPE_ID mtype = (bytes == 4) ? MTYPE_U4 : MTYPE_U8;
    WN_set_rtype (wn, mtype);
    WN_set_rtype (kid, mtype);
    WN_set_desc (kid, mtype);

    // bugs 9989, 10139
    INT field_id;
    Is_True (WN_field_id(kid) == 0,("vho_lower_mparm: Expected field-id zero"));
    if ((TY_kind(ty) == KIND_STRUCT) &&
        (field_id /* assign */ = single_field_in_struct (ty)))
      WN_set_field_id (kid, field_id);
  }

  return wn;
}
#endif

/* ============================================================================
 *
 * WN *vho_lower_expr ( WN * wn, WN * block, BOOL_INFO * bool_info )
 *
 * Given a tree, this routine traverses the tree performing lowering
 * actions for
 *
 *   SWITCH into a sequence of if-else, compgoto and/or binary search
 *   MSTORE into STID/ISTORE over LDID/ILOAD if the structure is
 *     sufficiently small.
 *
 * ============================================================================
 */

static WN *
#if defined( KEY) && !defined(TARG_ST)
vho_lower_expr ( WN * wn, WN * block, BOOL_INFO * bool_info, OPCODE stmt_opc )
#else
vho_lower_expr ( WN * wn, WN * block, BOOL_INFO * bool_info )
#endif
{
  OPCODE     opcode;
  OPERATOR   wn_operator;
  mINT16     nkids;
  mINT16     i;

  opcode      = WN_opcode(wn);
  wn_operator = OPCODE_operator(opcode);
  nkids       = WN_kid_count(wn);

  switch ( wn_operator ) {

    case OPR_ILOAD:

      WN_kid0(wn) = vho_lower_expr (WN_kid0(wn), block, NULL);

      if (    VHO_Iload_Opt
           && WN_operator(WN_kid0(wn)) == OPR_LDA ) {

        INT64 offset;

        offset = WN_load_offset(wn) + WN_lda_offset(WN_kid0(wn));

        if ( offset >= INT32_MIN && offset <= INT32_MAX ) {

          WN * wn1;
#ifdef TARG_ST
          wn1 = WN_CreateLdid ( OPCODE_make_op ( OPR_LDID,
                                                 OPCODE_rtype(opcode),
                                                 OPCODE_desc(opcode) ),
                                offset,
                                WN_st_idx(WN_kid0(wn)),
                                WN_ty(wn) );
#else
          wn1 = WN_CreateLdid ( OPCODE_make_op ( OPR_LDID,
                                                 OPCODE_rtype(opcode),
                                                 OPCODE_desc(opcode) ),
                                offset,
                                WN_st_idx(WN_kid0(wn)),
                                WN_ty(wn), 
                                WN_field_id(wn) );
#endif
/*
          fprintf ( stderr, "ILOAD->LDID old\n" );
          fdump_tree ( stderr, wn );
          fprintf ( stderr, "ILOAD->LDID new\n" );
          fdump_tree ( stderr, wn1 );
*/
          wn = wn1;
        }
      }

      break;

#if defined (KEY) && !defined(TARG_ST)
    case OPR_ILDBITS:  // 14561

      WN_kid0(wn) = vho_lower_expr (WN_kid0(wn), block, NULL);
      break;
#endif

    case OPR_ILOADX:

      FmtAssert(FALSE, ("op %s unexpected in vho_lower_expr",
      		      OPERATOR_name(wn_operator)));
      break;

    case OPR_MLOAD:

      WN_kid0(wn) = vho_lower_expr (WN_kid0(wn), block, NULL);
      WN_kid1(wn) = vho_lower_expr (WN_kid1(wn), block, NULL);
      break;

    case OPR_ARRAY:

      WN_kid0(wn) = vho_lower_expr (WN_kid0(wn), block, NULL);

      for ( i = (nkids + 1) >> 1; i < nkids; i++ )
        WN_kid(wn,i) = vho_lower_expr (WN_kid(wn,i), block, NULL);
      break;

    case OPR_INTRINSIC_OP:

#ifndef TARG_X8664
      for ( i = 0; i < nkids; i++ )
        WN_kid(wn,i) = vho_lower_expr (WN_kid(wn,i), block, NULL);
      break;
#else
      {
	// Fortran expression like a[i]**n can be vectorized if we expand 
	// inline before LNO. However, we do this only for small values of n.
	// The math library (powd) may do a better job for large values of n.
	if ( WN_intrinsic(wn) == INTRN_F8I4EXPEXPR || 
	     WN_intrinsic(wn) == INTRN_F4I4EXPEXPR ) {	  
	  WN* parm1 = WN_kid1(wn);
	  WN* exp = WN_kid0(parm1);
	  if (WN_operator(exp) == OPR_INTCONST &&
	      WN_const_val(exp) > 0 &&
	      WN_const_val(exp) <= 5) {
	    INT n = WN_const_val(exp);
	    WN_kid0(WN_kid0(wn)) = vho_lower_expr(WN_kid0(WN_kid0(wn)),
						  block, NULL); //bug 8576
	    WN* opnd = WN_COPY_Tree ( WN_kid0(WN_kid0(wn)) );
	    if ( n == 1 ) {
	      wn = opnd; break;
	    }
	    wn = WN_Mpy(WN_rtype(wn), opnd, WN_COPY_Tree ( opnd )); n-= 2;
	    for (;n > 0; n--)
	      wn = WN_Mpy(WN_rtype(wn), wn, WN_COPY_Tree ( opnd ));
	    break;
	  }
	} 

	for ( i = 0; i < nkids; i++ )
	  WN_kid(wn,i) = vho_lower_expr (WN_kid(wn,i), block, NULL);
	break;
      }
#endif

    case OPR_TAS:
#ifdef TARG_ST
  case OPR_SUBPART:
#endif

      WN_kid0(wn) = vho_lower_expr (WN_kid0(wn), block, NULL);
      break;

    case OPR_SELECT:

      WN_kid0(wn) = vho_lower_expr (WN_kid0(wn), block, NULL);
      WN_kid1(wn) = vho_lower_expr (WN_kid1(wn), block, NULL);
      WN_kid2(wn) = vho_lower_expr (WN_kid2(wn), block, NULL);
      break;

    case OPR_CSELECT:

      wn = vho_lower_cselect ( wn, block, bool_info);
      break;

    case OPR_CVT:
    case OPR_CVTL:

      WN_kid0(wn) = vho_lower_expr (WN_kid0(wn), block, NULL);
      break;

    case OPR_NEG:
    case OPR_ABS:
    case OPR_SQRT:
    case OPR_REALPART:
    case OPR_IMAGPART:
    case OPR_PAREN:
    case OPR_RND:
    case OPR_TRUNC:
    case OPR_CEIL:
    case OPR_FLOOR:
    case OPR_BNOT:
    case OPR_LNOT:
    case OPR_ALLOCA:
#if defined( KEY) && !defined(TARG_ST)
    case OPR_EXTRACT_BITS:
    case OPR_ATOMIC_RSQRT:
    case OPR_REPLICATE:
    case OPR_REDUCE_ADD:
    case OPR_REDUCE_MPY:
    case OPR_REDUCE_MAX:
    case OPR_REDUCE_MIN:
    case OPR_SHUFFLE:
#endif

      WN_kid0(wn) = vho_lower_expr (WN_kid0(wn), block, NULL);
      break;

    case OPR_ASM_INPUT:

      WN_kid0(wn) = vho_lower_expr (WN_kid0(wn), block, NULL);
      break;

    case OPR_ADD:
    case OPR_SUB:
    case OPR_MPY:
    case OPR_DIV:
    case OPR_MOD:
    case OPR_REM:
    case OPR_DIVREM:
#if defined( KEY) && !defined(TARG_ST)
    case OPR_COMPOSE_BITS:
#endif

      WN_kid0(wn) = vho_lower_expr (WN_kid0(wn), block, NULL);
      WN_kid1(wn) = vho_lower_expr (WN_kid1(wn), block, NULL);

      if ( VHO_Combine_Loads && wn_operator == OPR_ADD )
        wn = vho_lower_combine_loads ( wn );
      break;

    case OPR_LOWPART:
    case OPR_HIGHPART:

      WN_kid0(wn) = vho_lower_expr (WN_kid0(wn), block, NULL);
      break;

    case OPR_MAX:
    case OPR_MIN:
    case OPR_MINMAX:

      WN_kid0(wn) = vho_lower_expr (WN_kid0(wn), block, NULL);
      WN_kid1(wn) = vho_lower_expr (WN_kid1(wn), block, NULL);
      break;

    case OPR_MINPART:
    case OPR_MAXPART:

      WN_kid0(wn) = vho_lower_expr (WN_kid0(wn), block, NULL);
      break;

    case OPR_BAND:
    case OPR_BIOR:
    case OPR_BNOR:
    case OPR_BXOR:
    case OPR_LAND:
    case OPR_LIOR:

      WN_kid0(wn) = vho_lower_expr (WN_kid0(wn), block, NULL);
      WN_kid1(wn) = vho_lower_expr (WN_kid1(wn), block, NULL);

      if ( VHO_Combine_Loads && wn_operator == OPR_BIOR )
        wn = vho_lower_combine_loads ( wn );
      break;

    case OPR_CAND:
    case OPR_CIOR:

      wn = vho_lower_cand_cior ( wn, block, bool_info );
      break;

    case OPR_SHL:
    case OPR_ASHR:
    case OPR_LSHR:
    case OPR_COMPLEX:

      WN_kid0(wn) = vho_lower_expr (WN_kid0(wn), block, NULL);
      WN_kid1(wn) = vho_lower_expr (WN_kid1(wn), block, NULL);
      break;

    case OPR_RECIP:
    case OPR_RSQRT:

      WN_kid0(wn) = vho_lower_expr (WN_kid0(wn), block, NULL);
      break;

    case OPR_MADD:
    case OPR_MSUB:
    case OPR_NMADD:
    case OPR_NMSUB:

      WN_kid0(wn) = vho_lower_expr (WN_kid0(wn), block, NULL);
      WN_kid1(wn) = vho_lower_expr (WN_kid1(wn), block, NULL);
      WN_kid2(wn) = vho_lower_expr (WN_kid2(wn), block, NULL);
      break;

    case OPR_IO_ITEM:

      for ( i = 0; i < nkids; i++ )
        WN_kid(wn,i) = vho_lower_expr (WN_kid(wn,i), block, NULL);
      break;

    case OPR_EQ:
    case OPR_NE:
    case OPR_GT:
    case OPR_GE:
    case OPR_LT:
    case OPR_LE:

      WN_kid0(wn) = vho_lower_expr (WN_kid0(wn), block, NULL);
      WN_kid1(wn) = vho_lower_expr (WN_kid1(wn), block, NULL);
      break;

    case OPR_LDID:
#if defined( KEY) && !defined(TARG_ST)
    case OPR_LDBITS:

    // bugs 14589, 15169: the following will be handled in f90_lower
    case OPR_TRIPLET:
    case OPR_ARRAYEXP:
    case OPR_ARRSECTION:
#endif

      break;

    case OPR_LDA:
    case OPR_CONST:
    case OPR_INTCONST:
    case OPR_IDNAME:
#if defined( KEY) && !defined(TARG_ST)
    case OPR_LDA_LABEL:
#endif

      /* No need to lower */
      break;

    case OPR_LOOP_INFO:

      FmtAssert(FALSE, ("op %s unexpected in vho_lower_expr",
      		      OPERATOR_name(wn_operator)));
      break;

    case OPR_PARM:

#if defined( KEY) && !defined(TARG_ST)
// bug 7741
      wn = vho_lower_mparm (wn);
#endif
      WN_kid0(wn) = vho_lower_expr (WN_kid0(wn), block, NULL);
      break;

    case OPR_WHERE:
    case OPR_OPT_RESERVE2:

      FmtAssert(FALSE, ("op %s unexpected in vho_lower_expr",
      		      OPERATOR_name(wn_operator)));
      break;

    case OPR_COMMA:

#if defined( KEY) && !defined(TARG_ST)
      wn = vho_lower_comma ( wn, block, bool_info, stmt_opc );
#else
      wn = vho_lower_comma ( wn, block, bool_info );
#endif
/*
      comma_block = vho_lower_block (WN_kid0(wn));
      WN_INSERT_BlockLast ( block, comma_block );
      wn = vho_lower_expr (WN_kid1(wn), block, bool_info);
*/
      break;

    case OPR_RCOMMA:

      wn = vho_lower_rcomma ( wn, block, bool_info );
      break;

    case OPR_HIGHMPY:
    case OPR_XMPY:

      break;

    case OPR_RROTATE:
    {
#ifdef TARG_ST
        WN      *wn0   = vho_lower_expr (WN_kid0(wn), block, NULL);
        WN      *wn1   = vho_lower_expr (WN_kid1(wn), block, NULL);
        if (!Enable_Rotate || WN_rtype(wn) != MTYPE_U4 || WN_desc(wn) != MTYPE_U4){
            TYPE_ID  desc  = WN_desc(wn);
            TYPE_ID  rtype = WN_rtype(wn);
            TYPE_ID  shift_rtype = WN_rtype(wn1);
            INT32    size  = 8 * TY_size (Be_Type_Tbl (desc));
            WN      *rshift = WN_Lshr (rtype, WN_COPY_Tree (wn0), WN_COPY_Tree (wn1));
            WN      *lshift = WN_Shl  (rtype, wn0,
                                       WN_Binary (OPR_SUB, shift_rtype,
                                                  WN_Intconst (shift_rtype, size),
                                                  wn1));
            wn  = WN_Bior (rtype, lshift, rshift);
        }
#else
#ifdef KEY
      BOOL emulate_rrotate = ! VHO_Generate_Rrotate;
#ifdef TARG_X8664
      // By default, generate rotate instruction only if it is C++ (bug 7932)
      // Can be controlled by internal flag -VHO:rotate
      if (!VHO_Generate_Rrotate_Set && PU_cxx_lang (Get_Current_PU()))
	emulate_rrotate = FALSE;
      if (!Is_Target_64bit() && MTYPE_byte_size(WN_desc(wn)) > 4)
	emulate_rrotate = TRUE;
#endif
      if (emulate_rrotate)
#endif
      {
        TYPE_ID  desc  = WN_desc(wn);
        TYPE_ID  rtype = WN_rtype(wn);
        INT32    size  = 8 * TY_size (Be_Type_Tbl (desc));
        WN      *wn0   = vho_lower_expr (WN_kid0(wn), block, NULL);
        WN      *wn1   = vho_lower_expr (WN_kid1(wn), block, NULL);
        TYPE_ID  shift_rtype = WN_rtype(wn1);
        WN      *rshift = WN_Lshr (rtype, WN_COPY_Tree (wn0), WN_COPY_Tree (wn1));
        WN      *lshift = WN_Shl  (rtype, wn0,
                                   WN_Binary (OPR_SUB, shift_rtype,
                                              WN_Intconst (shift_rtype, size),
                                              wn1));
#ifdef TARG_X8664 // bug 4552
        // This is now unused code, but keep the fix for the record.
        if (size < MTYPE_size_min(rtype))
	  lshift = WN_CreateCvtl(OPR_CVTL, Mtype_TransferSign(MTYPE_U4, rtype), 
				  MTYPE_V, size, lshift);
#endif
        wn  = WN_Bior (rtype, lshift, rshift);
      }
#ifdef KEY
      else
      {
        WN_kid0 (wn) = vho_lower_expr (WN_kid0(wn), block, NULL);
        WN_kid1 (wn) = vho_lower_expr (WN_kid1(wn), block, NULL);
      }
#endif // KEY
#endif
      break;
    }

#ifdef TARG_ST
      case OPR_LROTATE:
    {
      WN      *wn0   = vho_lower_expr (WN_kid0(wn), block, NULL);
      WN      *wn1   = vho_lower_expr (WN_kid1(wn), block, NULL);
      if (!Enable_Rotate || WN_rtype(wn) != MTYPE_U4 || WN_desc(wn) != MTYPE_U4){
	TYPE_ID  desc  = WN_desc(wn);
	TYPE_ID  rtype = WN_rtype(wn);
	TYPE_ID  shift_rtype = WN_rtype(wn1);
	INT32    size  = 8 * TY_size (Be_Type_Tbl (desc));
	WN      *rshift = WN_Shl (rtype, WN_COPY_Tree (wn0), WN_COPY_Tree (wn1));
	WN      *lshift = WN_Lshr  (rtype, wn0,
				    WN_Binary (OPR_SUB, shift_rtype,
					       WN_Intconst (shift_rtype, size),
					       wn1));
	wn  = WN_Bior (rtype, lshift, rshift);
      }
      break;
    }

#endif
#if defined (KEY) && !defined(TARG_ST) // bug 14534
    case OPR_GOTO:
      break;
#endif

    default:

      FmtAssert(FALSE, ("op %s unexpected in vho_lower_expr",
      		      OPERATOR_name(wn_operator)));
      break;
  }

  return wn;
} /* vho_lower_expr */


static WN *
vho_lower_compgoto ( WN * wn, WN * block )
{
  WN_kid0(wn) = vho_lower_expr ( WN_kid0(wn), block, NULL );
  WN_kid1(wn) = vho_lower_block ( WN_kid1(wn) );
  return wn;
} /* vho_lower_compgoto */


static WN *
vho_lower_switch ( WN * wn, WN * block )
{
  WN_kid0(wn) = vho_lower_expr ( WN_kid0(wn), block, NULL );
  wn = VHO_Lower_Switch ( wn );
  return wn;
} /* vho_lower_switch */


static WN *
vho_lower_casegoto ( WN * wn, WN * block )
{
  return wn;
} /* vho_lower_casegoto */


static WN *
vho_lower_truebr ( WN * wn, WN * block )
{
  WN        * test;
  WN        * test_block;
  BOOL_INFO   bool_info;

  test_block = WN_CreateBlock ();
  WN_Set_Linenum ( test_block, VHO_Srcpos );

  vho_initialize_bool_info (&bool_info);
  bool_info.true_label  = WN_label_number(wn);

  LABEL_Init (New_LABEL (CURRENT_SYMTAB, bool_info.false_label),
	0, LKIND_DEFAULT);

  test = vho_lower_expr ( WN_kid0(wn), test_block, &bool_info );

  if ( WN_first(test_block) ) {
    WN_INSERT_BlockLast ( block, test_block );
  }

  if ( test ) {
    WN_kid0(wn) = test;
  }

  else {
    // TRUEBR <test> <true_label>    ===>    <test>
    //                                       GOTO <true_label>
    //                                       LABEL <false_label>

    WN *wn_goto = WN_CreateGoto ( (ST_IDX) NULL, bool_info.true_label );
    WN_Set_Linenum ( wn_goto, VHO_Srcpos );
    WN_INSERT_BlockLast ( block, wn_goto );
    
    WN *wn_label
      = WN_CreateLabel ( (ST_IDX) 0, bool_info.false_label, 0, NULL );
    WN_Set_Linenum ( wn_label, VHO_Srcpos );
    WN_INSERT_BlockLast ( block, wn_label );

    // Don't update feedback; let propagation handle

    wn = NULL;
  }

  return wn;
} /* vho_lower_truebr */


static WN *
vho_lower_return ( WN * wn, WN * block )
{
  return wn;
} /* vho_lower_return */

static BOOL
vho_singleton_field (TY_IDX ty_idx)
{
  return FALSE;
} /* vho_singleton_field */

static WN *
vho_lower_return_val (WN* wn, WN* block)
{
  WN_kid0(wn) = vho_lower_expr ( WN_kid0(wn), block, NULL );
  return wn;
} /* vho_lower_return_val */

static WN *
vho_lower_istore ( WN * wn, WN * block )
{
  WN_kid0(wn) = vho_lower_expr ( WN_kid0(wn), block, NULL );
  WN_kid1(wn) = vho_lower_expr ( WN_kid1(wn), block, NULL );
  
#ifdef TARG_ST
  /* [FdF]: Lower MISTORE */
  if (WN_opcode(wn) == OPC_MISTORE) {
    wn = VHO_Lower_Mstore ( wn );
  }
#endif

  return wn;
} /* vho_lower_istore */


static WN *
vho_lower_mstore ( WN * wn, WN * block )
{
  WN_kid0(wn) = vho_lower_expr ( WN_kid0(wn), block, NULL );
  WN_kid1(wn) = vho_lower_expr ( WN_kid1(wn), block, NULL );
  WN_kid(wn,2) = vho_lower_expr ( WN_kid(wn,2), block, NULL );
  wn = VHO_Lower_Mstore ( wn );

  return wn;
} /* vho_lower_mstore */


static WN *
vho_lower_stid ( WN * wn, WN * block )
{
    
#ifdef TARG_ST
  if (WHIRL_Return_Val_On && 
      ST_assigned_to_dedicated_preg(WN_st(wn)) &&
      WN_operator(WN_kid0(wn)) == OPR_COMMA &&
      vho_is_ldid_of_return_val(WN_kid1(WN_kid0(wn)))) {
    // [CG] In the case of an INTRINSIC_CALL returning into a dedicated register
    // it is mandatory to directly generate the STID "ded" (LDID "return_val_preg").
    WN *comma_tree = WN_kid0(wn);
    WN *comma_block = vho_lower_block (WN_kid0(comma_tree));
    WN_INSERT_BlockLast ( block, comma_block );
    WN_kid0(wn) = WN_kid1(comma_tree);
  }
#endif

#if defined( KEY) && !defined(TARG_ST)
  /* sct 5409 */

  WN_kid0(wn) = vho_lower_expr ( WN_kid0(wn), block, NULL, WN_opcode(wn) );
#else
  WN_kid0(wn) = vho_lower_expr ( WN_kid0(wn), block, NULL );
#endif

  #ifdef TARG_ST
  /* [CG]: Lower MSTID. */
  if (WN_opcode(wn) == OPC_MSTID) {
    wn = VHO_Lower_Mstore ( wn );
  }
#endif
#ifndef TARG_SR
  if (WN_desc(wn) == MTYPE_M) {
    /* lower MSTID */
    wn = VHO_Lower_Mstid (wn);
  }
#endif

  return wn;
} /* vho_lower_stid */


static WN *
vho_lower_call ( WN * wn, WN * block )
{
  INT32 i;

  for ( i = 0; i < WN_kid_count(wn); i++ )
    WN_actual(wn,i) = vho_lower_expr ( WN_actual(wn,i), block, NULL );

  return wn;
} /* vho_lower_call */

static char * extract_pu_name(char * src_pu_name )
{
	if (src_pu_name == NULL)
		return NULL;
	char * pu_name = strchr(src_pu_name,'/');
	if (pu_name == NULL)
		return NULL;
	else
	{
		pu_name ++;
		return pu_name;
	}
}

BOOL 
Is_Return_Store_Stmt( WN *wn )
{
  if ( wn && WN_operator( wn ) == OPR_STID ) {
    WN *val = WN_kid( wn, 0 );
    if ( WN_operator( val ) == OPR_LDID ) {
      ST *st = WN_st( val );
      if ( ST_sym_class( st ) == CLASS_PREG
	   && ( Is_Return_Preg( WN_offset( val ) )
		|| st == Return_Val_Preg ) )
	return TRUE;
    }
  }
  
  return FALSE;
}

int
GetPUSizeFromEnv(char * puname)
{
  if (puname == NULL)
	  return -1;

  char *p = getenv (puname);
  int num;
  if (p)
	  num = atoi(p);
  else num = 1;
  return num;
}

static WN *
vho_lower_if ( WN * wn, WN *block );


int Get_len_and_goaheadlen(char ** str)
{
	int len = atoi(*str);
	if (len>0 && len < strlen(*str) )
	{
		int ll = 0;
		if (len < 10)
			ll = 1;
		else if (len<100)
			ll = 2;
		else if (len<1000)
			ll = 3;
		else 
		{
			//I never see a class name longer than 999
			return 0;
		}
		*str  += ll;
		if (len < strlen(*str))
			*str += len;
			
	}
	return len;
}

//// NOTE: here similar means a manggled PU name in C++, 
//// the only difference between <name1> and <name2> is the class name. 
//// For example, "C13mrYZRectangle" and "C9mrPolygon" are class names:
////    boundingBox__C13mrYZRectangleddR6ggBox3  and boundingBox__C9mrPolygonddR6ggBox3
int Is_similar_cppname(char * name1, char * name2)
{
	char * class_name_start1, * class_name_start2;
	class_name_start1 = strstr(name1,"__C");
	class_name_start2 = strstr(name2,"__C");
	if (class_name_start1 == NULL || class_name_start2 == NULL)
		return 0;
	class_name_start1 += strlen("__C");
	class_name_start2 += strlen("__C");
	int pu_name_len1 = class_name_start1 - name1;
	int pu_name_len2 = class_name_start2 - name2;

	if (pu_name_len1 != pu_name_len2 || strncmp(name1,name2,pu_name_len1) != 0 )
		return 0;

	int class_name_len1, class_name_len2;
	class_name_len1 = 0;
	class_name_len2 = 0;
	class_name_len1 = Get_len_and_goaheadlen(&class_name_start1);
	class_name_len2 = Get_len_and_goaheadlen(&class_name_start2);
	if (class_name_len1 == 0 || class_name_len2 == 0)
		return 0;
	if (strcmp(class_name_start1,class_name_start2) != 0)
		return 0;
	return 1;
}

static WN *
vho_lower_icall ( WN * wn, WN * block )
{
	if ( ! VHO_Icall_Devir )
		return vho_lower_call(wn, block);
			
 //NOTE: Do NOT call this routine twice. Because devirtualization more than one time will absulutely degrade the performance.
  //First do devirtualization transform for indirect call. Then it will call vho_lower_xxx() for 
  //each of the new statements introduced by this transform. 
  //
  //The devirtualization transforms as below:
  //
  //The codes:
  // BLODK
  //     ... ...
  //       prepare_arguments
  //       U8ILOAD fun_ptr
  //     I4ICALL 
  //     ... ...
  // END_BLODK
  //Will be translated into:
  // BLOCK
  //   ... ...
  //   IF
  //       U8ILOAD fun_ptr
  //       U8LDA foo_1's address
  //     U8U8EQ
  //   THEN
  //     BLODK
  //         prepare_arguments
  //       I4CALL foo_1
  //     END_BLODK
  //   ELSE
  //     BLOCK
  //       IF
  //           U8ILOAD fun_ptr
  //           U8LDA foo_2's address
  //         U8U8EQ
  //       THEN
  //         BLOCK
  //             prepare_arguments
  //           I4CALL foo_2
  //         END_BLOCK
  //       ELSE
  //         BLOCK
  //             prepare_arguments
  //             I4ILOAD fun_ptr
  //           I4ICALL
  //         END_BLOCK
  //       ENDIF
  //     END_BLOCK
  //   ENDIF
  //   ... ...  // END_BLOCK

  // We get the name of funtion <foo1> from profile feedback,
  // Then we lookup symbol table for its <TY_IDX> and <ST*>.

  if (Cur_PU_Feedback == NULL)
  {
	  return vho_lower_call(wn,block);
  }

  const FB_Info_Icall & info_icall = Cur_PU_Feedback->Query_icall(wn);
  const FB_Info_Call & info_call = Cur_PU_Feedback->Query_call(wn);
  if ( info_icall.Is_uninit() )
  {
        return vho_lower_call(wn,block);
  }
 
  if ( info_icall.tnv._exec_counter == 0 )
  {
	  Is_True(info_icall.tnv._counters[0]==0,("_counters[0] must be 0 if _exec_counter is 0"));
	  return vho_lower_call(wn,block);
  }
  else
	  Is_True(info_icall.tnv._values[0] >0, ("function address must be positive!"));
  
  char * nameoffootmp = PU_Addr_Name_Map[info_icall.tnv._values[0]]; 
  int sizeoffoo = PU_Addr_Pusize_Map[info_icall.tnv._values[0]];
  if ( nameoffootmp == NULL)
  {
	  return vho_lower_call(wn,block);
  }

  char * nameoffoo1 = extract_pu_name(nameoffootmp);
  Is_True(nameoffoo1!=NULL,("Devirtualize: did not extract right pu_name in PU_Addr_Name_Map for address <%" SCNd64 ">",info_icall.tnv._values[0]));

  
  ST * st_foo1 = NULL;
  TY_IDX ty_foo1;
  
  ST_TAB * parray = Scope_tab[GLOBAL_SYMTAB].st_tab;
  UINT32 last = (*parray).Size();
  UINT32 first = 1;

  INT num_pu_found = 0;
  while (first < last){
	  ST * block  = &(*parray)[first];
	  UINT32 size = (*parray).Get_block_size(first);
	  for (UINT32 i = 0; i < size; ++i, ++block){
		if (ST_sym_class(block) != CLASS_FUNC)
			continue;
	    STR_IDX str_idx = ST_name_idx(block);
		if (strcmp(nameoffoo1,&Str_Table[str_idx]) == 0)
		{
			if (num_pu_found > 0)
			{
			}
			st_foo1 = block;
			ty_foo1 = ST_pu_type(block);
			num_pu_found ++;
			//TODO: add break to save tranverse time.
			// break;
		}
	  }
	  first += size;
  }

  if (num_pu_found == 0)
  {
  	  parray = Scope_tab[GLOBAL_SYMTAB].st_tab;
	  last = (*parray).Size();
	  first = 1;
	  while (first < last){
		  ST * block  = &(*parray)[first];
		  UINT32 size = (*parray).Get_block_size(first);
		  for (UINT32 i = 0; i < size; ++i, ++block){
			if (ST_sym_class(block) != CLASS_FUNC)
				continue;
		    STR_IDX str_idx = ST_name_idx(block);
			if (strcmp(nameoffoo1,&Str_Table[str_idx]) == 0)
			{
				st_foo1 = block;
				ty_foo1 = ST_pu_type(block);
				num_pu_found ++;
			}
			else if(Is_similar_cppname(nameoffoo1,&Str_Table[str_idx]))
			{
				if (num_pu_found == 0)
				{
					ST * newst = Copy_ST(block);
					STR_IDX newidx = Save_Str(nameoffoo1);
					Set_ST_name(newst,newidx);
					Set_ST_sclass(newst,SCLASS_EXTERN);
					st_foo1	= newst;
					ty_foo1 = ST_pu_type(newst);
				//	DevWarn("Found similar PU(%s) for foo1(%s)\n",&Str_Table[str_idx],nameoffoo1);
				}
				else 
				{
				//	DevWarn("Found(multiple so ignore) similar PU(%s) for foo1(%s)\n",&Str_Table[str_idx],nameoffoo1);
				}
				num_pu_found ++;
			}
		  }
		  first += size;
	  }
  }

  
  extern char * Src_File_Name; 

  char strii[20];
  char tmpfilename[50];

  char *p = getenv ("VHO_IGNORE_FILE_NUM");
  int ii, num_files;
  if (p)
	  num_files = atoi(p);
  else num_files = 0;
  for ( ii=1; ii<=num_files; ii++)
  {
      sprintf(strii,"%d",ii);
      strcpy(tmpfilename,"VHO_IGNORE_FILE_NAME");
      strcat(tmpfilename,strii);
      p =  getenv (tmpfilename);
      if ( !p )
	  continue;
      if ( strcmp(Src_File_Name, p) == 0)
      {
	return vho_lower_call(wn,block);
      }
  }
  
  if ( (float)info_icall.tnv._exec_counter != info_call.freq_entry._value )
  {
      return vho_lower_call(wn,block);
  }

  float ratio = info_icall.tnv._counters[0] * 1.0  / info_icall.tnv._exec_counter;
  if (ratio < 0.98 )
  {
      return vho_lower_call(wn,block);
  }
  else if ( info_icall.tnv._exec_counter < 50 )
  {
      return vho_lower_call(wn,block);
  }
  else if (sizeoffoo < 20 &&  info_icall.tnv._exec_counter / sizeoffoo < 80 )
  {
      return vho_lower_call(wn,block);
  }
  else if (sizeoffoo < 100 && info_icall.tnv._exec_counter / sizeoffoo < 160 )
  {
      return  vho_lower_call(wn,block);
  }
  else if (sizeoffoo < 200 && info_icall.tnv._exec_counter / sizeoffoo < 320 )
  {
      return  vho_lower_call(wn,block);
  }
  else if (sizeoffoo < 1000 && info_icall.tnv._exec_counter / sizeoffoo < 640 )
  {
      return  vho_lower_call(wn,block);
  }
  else if (sizeoffoo > 1000 && info_icall.tnv._exec_counter / sizeoffoo < 1280 )
  {
      return  vho_lower_call(wn,block);
  }
  else
  {
     if ( num_pu_found > 0 )
     {
         DevWarn("Devirtualize:<last decision : willuse>Found <%d> PUs in symbol tables match profiled names <%s>",num_pu_found,nameoffoo1);
     }
  }


  if (num_pu_found == 0)
  {
	  return vho_lower_call(wn,block);
  }
	  
  
  WN * wn_if, * test, * if_then, * if_else, *if_then_block, *if_else_block, *wn_ret_value;
  WN * tmpkid0, * tmpkid1;
  WN * stmt;
#ifdef TARG_ST
  // TB: for 32 bit target
  if (Pointer_Size == 4)
    tmpkid0 = WN_CreateLda(OPC_U4LDA,0, Make_Pointer_Type (ty_foo1),st_foo1);
  else
#endif

#if defined(KEY) && !defined(TARG_ST)
  const BOOL ptr_is_64bit = MTYPE_bit_size(Pointer_Mtype) == 64;
  tmpkid0 = WN_CreateLda( ptr_is_64bit ? OPC_U8LDA : OPC_U4LDA,
			  0, Make_Pointer_Type (ty_foo1),st_foo1 );
#else
  tmpkid0 = WN_CreateLda(OPC_U8LDA,0, Make_Pointer_Type (ty_foo1),st_foo1);
#endif // KEY
  tmpkid1 = WN_COPY_Tree(WN_kid(wn,WN_kid_count(wn)-1));
#ifdef TARG_ST
  // TB: for 32 bit target
  if (Pointer_Size == 4)
    test = WN_Create(OPC_U4U4EQ,2);  
  else
#endif

#if defined( KEY) && !defined(TARG_ST)
  test = WN_Create( ptr_is_64bit ? OPC_U8U8EQ : OPC_U4U4EQ, 2 );
#else  
  test = WN_Create(OPC_U8U8EQ,2);  
#endif // KEY

  WN_kid0(test) = tmpkid0;
  WN_kid1(test) = tmpkid1;

  if_then = WN_Create(WN_opcode(wn),WN_kid_count(wn)-1);
  WN_set_operator(if_then,OPR_CALL);
  for (int i=0;i<WN_kid_count(if_then);i++)
  {
    WN_kid(if_then,i) = WN_COPY_Tree(WN_kid(wn,i));
  }
  WN_st_idx(if_then) = ST_st_idx(st_foo1);

  if_then_block = WN_CreateBlock();
  WN_INSERT_BlockLast(if_then_block,if_then);
  stmt = WN_next(wn);
  while ( stmt && Is_Return_Store_Stmt( stmt ) ) {
    wn_ret_value = WN_COPY_Tree(stmt);
    WN_INSERT_BlockLast(if_then_block,wn_ret_value);
    stmt = WN_next(stmt);
  }
  
  if_else = WN_COPY_Tree(wn);
  if_else_block = WN_CreateBlock();
  WN_INSERT_BlockLast(if_else_block,if_else);
  stmt = WN_next(wn);
  while ( stmt && Is_Return_Store_Stmt( stmt ) ) {
    wn_ret_value = WN_COPY_Tree(stmt);
    WN_INSERT_BlockLast(if_else_block,wn_ret_value);
    stmt = WN_next(stmt);
  }

  //empty the stmt
  stmt = WN_next(wn);
  while ( stmt && Is_Return_Store_Stmt( stmt ) ) {
    wn_ret_value = stmt;
    stmt = WN_next(stmt);
    WN_set_kid_count(wn_ret_value,0);
	WN_set_operator(wn_ret_value,OPR_COMMENT);
	WN_set_rtype(wn_ret_value,MTYPE_V);
	WN_set_desc(wn_ret_value,MTYPE_V);
  }

  wn_if = WN_CreateIf(test, if_then_block, if_else_block);

  if (Cur_PU_Feedback)
	  Cur_PU_Feedback->FB_lower_icall( wn, if_else, if_then, wn_if );

  //Delete the map info. We delete it from <Cur_PU_Feedback>
  Cur_PU_Feedback->Delete(wn);

 // wn_if = vho_lower_if_ignore_else_for_devirtualize(wn_if,block);
  wn_if = vho_lower_if(wn_if,block);
  
  return wn_if;
} /* vho_lower_icall */


static WN *
vho_lower_intrinsic_call ( WN * wn, WN * block )
{
  INT32 i;

  for ( i = 0; i < WN_kid_count(wn); i++ )
    WN_actual(wn,i) = vho_lower_expr ( WN_actual(wn,i), block, NULL );

  return wn;
} /* vho_lower_intrinsic_call */


static WN *
vho_lower_eval ( WN * wn, WN * block )
{
  WN_kid0(wn) = vho_lower_expr ( WN_kid0(wn), block, NULL );

  if ( WN_kid0(wn) )
    return wn;

  else
    return NULL;
} /* vho_lower_eval */


static WN *
vho_lower_pragma ( WN * wn, WN * block )
{
  return wn;
} /* vho_lower_pragma */


static WN *
vho_lower_xpragma ( WN * wn, WN * block )
{
  INT32 i;

  for ( i = 0; i < WN_kid_count(wn); i++ )
    WN_kid(wn, i) = vho_lower_expr ( WN_kid(wn, i), block, NULL );

#if defined( KEY) && !defined(TARG_ST) // bug 14036
  if (VHO_In_MP_Region_Pragma && WN_kid_count(wn) == 1)
  {
    WN * kid = WN_kid0(wn);
    TY_IDX ty_idx = MTYPE_TO_TY_array[WN_rtype(kid)];
    ST * st = Gen_Temp_Symbol (ty_idx, "_mp_xpragma");
    WN * stid = WN_Stid(WN_rtype(kid), 0, st, ty_idx, kid);
    WN_kid0(wn) = WN_Ldid(WN_rtype(kid), 0, st, ty_idx);
    WN_INSERT_BlockLast (block, stid);
  }
#endif

  return wn;
} /* vho_lower_xpragma */


static WN *
vho_lower_prefetch ( WN * wn, WN * block )
{
#ifdef KEY // bug 9499
  // Kid0 may need to be lowered, for example, if it is a call.
  WN_kid0(wn) = vho_lower_expr ( WN_kid0(wn), block, NULL );
#endif
  return wn;
} /* vho_lower_prefetch */


static WN *
vho_lower_prefetchx ( WN * wn, WN * block )
{
  return wn;
} /* vho_lower_prefetchx */


static WN *
vho_lower_io ( WN * wn, WN * block )
{
  INT32 i;

  for ( i = 0; i < WN_kid_count(wn); i++ )
    WN_kid(wn, i) = vho_lower_expr ( WN_kid(wn, i), block, NULL );

  return wn;
} /* vho_lower_io */


static WN *
vho_lower_falsebr ( WN * wn, WN * block )
{
  WN        * test;
  WN        * test_block;
  BOOL_INFO   bool_info;

  test_block = WN_CreateBlock ();
  WN_Set_Linenum ( test_block, VHO_Srcpos );

  vho_initialize_bool_info (&bool_info);
  LABEL_Init (New_LABEL (CURRENT_SYMTAB, bool_info.true_label),
	0, LKIND_DEFAULT);
  bool_info.false_label = WN_label_number(wn);

  test = vho_lower_expr ( WN_kid0(wn), test_block, &bool_info );

  if ( WN_first(test_block) )
    WN_INSERT_BlockLast ( block, test_block );

  if ( test ) {

    WN_kid0(wn) = test;
  }

  else {

    wn = WN_CreateLabel ( (ST_IDX) 0, bool_info.true_label, 0, NULL );
    WN_Set_Linenum ( wn, VHO_Srcpos );
    WN_INSERT_BlockLast ( block, wn );

    wn = NULL;
  }

  return wn;
} /* vho_lower_falsebr */


static WN *
vho_lower_trap ( WN * wn, WN * block )
{
  return wn;
} /* vho_lower_trap */


static WN *
vho_lower_asm_stmt (WN* wn, WN* block)
{
  INT32 i;

  for ( i = 2; i < WN_kid_count(wn); i++ )
    WN_kid(wn, i) = vho_lower_expr ( WN_kid(wn, i), block, NULL );

  return wn;
} /* vho_lower_asm_stmt */

static WN *
vho_lower_stmt ( WN * wn, WN * block )
{
  PREG_NUM last_preg;

  last_preg  = PREG_Table_Size (CURRENT_SYMTAB);

  VHO_Srcpos = WN_Get_Linenum(wn);

  switch ( WN_operator(wn) ) {

    case OPR_DEALLOCA:

      break;

    case OPR_REGION_EXIT:

      break;

    case OPR_COMPGOTO:

      wn = vho_lower_compgoto ( wn, block );
      break;

    case OPR_SWITCH:

      wn = vho_lower_switch ( wn, block );
      break;

    case OPR_CASEGOTO:

      wn = vho_lower_casegoto ( wn, block );
      break;

    case OPR_XGOTO:
    case OPR_GOTO:
    case OPR_AGOTO:
    case OPR_ALTENTRY:

      break;

    case OPR_TRUEBR:

      wn = vho_lower_truebr ( wn, block );
      break;

    case OPR_RETURN:
#if defined( KEY) && !defined(TARG_ST)
    case OPR_GOTO_OUTER_BLOCK:
#endif

      wn = vho_lower_return ( wn, block );
      break;

    case OPR_RETURN_VAL:

      wn = vho_lower_return_val ( wn, block );
      break;

    case OPR_LABEL:

      break;

    case OPR_EXC_SCOPE_BEGIN:
    case OPR_EXC_SCOPE_END:

      break;

    case OPR_FORWARD_BARRIER:
    case OPR_BACKWARD_BARRIER:

      break;

    case OPR_ISTORE:

      wn = vho_lower_istore ( wn, block );

      if (    VHO_Istore_Opt
           && WN_operator(WN_kid1(wn)) == OPR_LDA ) {

        INT64  offset;
        OPCODE opcode;
         
        opcode = WN_opcode(wn);
        offset = WN_store_offset(wn) + WN_lda_offset(WN_kid1(wn));

        if ( offset >= INT32_MIN && offset <= INT32_MAX ) {

          WN * wn1;
#ifdef TARG_ST
          wn1 = WN_CreateStid ( OPCODE_make_op ( OPR_STID,
                                                 OPCODE_rtype(opcode),
                                                 OPCODE_desc(opcode) ),
                                offset,
                                WN_st_idx(WN_kid1(wn)),
                                WN_ty(wn),
                                WN_kid0(wn) );
#else
          wn1 = WN_CreateStid ( OPCODE_make_op ( OPR_STID,
                                                 OPCODE_rtype(opcode),
                                                 OPCODE_desc(opcode) ),
                                offset,
                                WN_st_idx(WN_kid1(wn)),
                                TY_pointed(WN_ty(wn)), 
                                WN_kid0(wn), 
                                WN_field_id(wn));
#endif
/*
          fprintf ( stderr, "ISTORE->STID old\n" );
          fdump_tree ( stderr, wn );
          fprintf ( stderr, "ISTORE->STID new\n" );
          fdump_tree ( stderr, wn1 );
*/
          wn = wn1;
        }
      }

      if (WN_desc(wn) == MTYPE_M) {
         if (WN_operator(wn) == OPR_STID) {
           wn = VHO_Lower_Mstid(wn);
         } else {
           wn = VHO_Lower_Mistore(wn);
         }
      }

      break;

    case OPR_ISTOREX:

      break;

    case OPR_MSTORE:

      wn = vho_lower_mstore ( wn, block );
      break;

    case OPR_STID:

      wn = vho_lower_stid ( wn, block );
      break;

    case OPR_CALL:

      wn = vho_lower_call ( wn, block );
      break;

    case OPR_INTRINSIC_CALL:

      wn = vho_lower_intrinsic_call ( wn, block );
      break;

    case OPR_ICALL:

      wn = vho_lower_icall ( wn, block );
      break;

    case OPR_PICCALL:

      break;
#ifdef TARG_ST
    case OPR_AFFIRM:
      // fall through
#endif
    case OPR_EVAL:

      wn = vho_lower_eval ( wn, block );
      break;

    case OPR_PRAGMA:

      wn = vho_lower_pragma ( wn, block );
      break;

    case OPR_XPRAGMA:

      wn = vho_lower_xpragma ( wn, block );
      break;

    case OPR_PREFETCH:

      wn = vho_lower_prefetch ( wn, block );
      break;

    case OPR_PREFETCHX:

      wn = vho_lower_prefetchx ( wn, block );
      break;

    case OPR_IO:

      wn = vho_lower_io ( wn, block );
      break;

    case OPR_COMMENT:

      break;

    case OPR_FALSEBR:

      wn = vho_lower_falsebr ( wn, block );
      break;
      
    case OPR_TRAP:

      wn = vho_lower_trap ( wn, block );
      break;

    case OPR_ASSERT:

      wn = vho_lower_trap ( wn, block );
      break;

    case OPR_ASM_STMT:

      wn = vho_lower_asm_stmt ( wn, block );
      break;

    default:

      break;
  }

  if ( VHO_Recycle_Pregs )
    Reset_PREG_Table_Size (CURRENT_SYMTAB, last_preg);

  return wn;
} /* vho_lower_stmt */


#if defined( KEY) && !defined(TARG_ST)
// Returns the last statement in the region (after recursively
// traversing any contained region). If EXTRACT is true, then
// extract this last statement if it is XPRAGMA.
static WN *
vho_last_stmt_in_region ( WN * wn, BOOL extract )
{
  Is_True (WN_operator(wn) == OPR_REGION,
           ("vho_last_stmt_in_region: expects REGION node"));

  WN * block = WN_region_body(wn);
  WN * last = WN_last(block);

  while (last)
  {
    if (WN_operator(last) == OPR_REGION)
    {
      block = WN_region_body(last);
      last = WN_last(block);
    }
    else
    {
      if (extract && WN_operator(last) == OPR_XPRAGMA)
        last = WN_EXTRACT_FromBlock (block, last);
      break;
    }
  }

  return last;
}

static WN *
vho_lower_region ( WN * wn, WN * block )
{
  WN_region_body(wn) = vho_lower_block ( WN_region_body(wn) );
  
  INT region_id = WN_region_id(wn);
  if ((WN_region_kind(wn) == REGION_KIND_MP) &&
      (current_pu_id < VHO_Disable_MP_PU_Before ||
       current_pu_id > VHO_Disable_MP_PU_After ||
       current_pu_id == VHO_Disable_MP_PU_Equal ||
       region_id < VHO_Disable_MP_Local_Before ||
       region_id > VHO_Disable_MP_Local_After ||
       region_id == VHO_Disable_MP_Local_Equal)) {
    WN* region_body = WN_region_body(wn);
    WN* stmt = WN_first(region_body);
    WN* next_stmt;
    while(stmt) {
      next_stmt = WN_next(stmt);
      WN_INSERT_BlockLast(block, stmt);
      stmt = next_stmt;
    }
    WN_next(wn) = WN_prev(wn) = NULL;
    return NULL;
  }

  // Lower pragmas for MP regions
  if (WN_region_kind(wn) == REGION_KIND_MP)
  {
#ifdef Is_True_On
    {
      // Verify the assumption that WN_region_pragmas does not contain any
      // non-pragma node before lowering
      // e.g. C++ EH region nodes can contain non-pragma nodes here.
      WN * pragmas = WN_first (WN_region_pragmas (wn));
      while (pragmas)
      {
        WN * stmt = NULL;
        if (WN_operator(pragmas) == OPR_REGION)
          stmt = vho_last_stmt_in_region (pragmas, FALSE);
	// use fmtassert if common/com is built without debug
	FmtAssert (WN_operator(pragmas) == OPR_PRAGMA ||
	           WN_operator(pragmas) == OPR_XPRAGMA ||
	           // Allow region nodes whose last stmt is xpragma
                   (stmt && WN_operator(stmt) == OPR_XPRAGMA),
	           ("Unexpected node in region pragmas"));
	pragmas = WN_next (pragmas);
      }
    }
#endif // Is_True_On

    VHO_In_MP_Region_Pragma = TRUE;
    WN_region_pragmas(wn) = vho_lower_block ( WN_region_pragmas(wn) );
    VHO_In_MP_Region_Pragma = FALSE;

    WN * iter = WN_first (WN_region_pragmas (wn));

    while (iter)
    {
      // move any non-pragma stmt to before the region
      // bug 5130: also move inline-pragmas outside
      if ((WN_operator (iter) != OPR_PRAGMA  ||
           (WN_pragma (iter) == WN_PRAGMA_INLINE_BODY_START ||
	    WN_pragma (iter) == WN_PRAGMA_INLINE_BODY_END)) &&
          WN_operator (iter) != OPR_XPRAGMA)
      {
        WN * move  = iter;
        // increment
        iter = WN_next (iter);
	// fail in debug mode, a pragma must be the last node
	Is_True (WN_next (move), ("Pragma node not found in region pragmas"));
	// remove the node from region-pragmas
	if (WN_operator(move) == OPR_REGION)
	{ // bug 14036 */
	  WN * stmt = vho_last_stmt_in_region (move, TRUE /* extract out */);
	  if (stmt && WN_operator(stmt) == OPR_XPRAGMA)
	    WN_INSERT_BlockBefore (WN_region_pragmas (wn), move, stmt);
	}
	move = WN_EXTRACT_FromBlock (WN_region_pragmas (wn), move);


        WN_INSERT_BlockLast (block, move);
      }
      else iter = WN_next (iter);
    }
  }

  // bug 2631:
  // If we have a COMPGOTO stmt inside the region, move it outside the
  // region.
  WN * last = WN_last ( WN_region_body (wn) );
  if ( last && WN_operator(last) == OPR_COMPGOTO && WN_region_is_EH (wn) )
  {
    Is_True (WN_first (WN_region_body (wn)) != last, ("No call inside region"));
    if (WN_prev (last))
    	WN_next (WN_prev (last)) = NULL;
    WN_last (WN_region_body (wn)) = WN_prev (last);

    WN_next (last) = WN_prev (last) = NULL;
    WN_INSERT_BlockLast (block, wn);
    // Now insert the compgoto after the region
    WN_INSERT_BlockLast (block, last);
    wn = NULL;	// don't insert it again
  }
  return wn;
} /* vho_lower_region */
#else
static WN *
vho_lower_region ( WN * wn )
{
  WN_region_body(wn) = vho_lower_block ( WN_region_body(wn) );
  return wn;
} /* vho_lower_region */
#endif


static WN *
vho_lower_do_loop ( WN * wn, WN *block )
{
  WN        * start;
  WN        * end;
  WN        * step;
  WN        * lower_block;
  WN        * do_body;
  BOOL_INFO   bool_info;
  SRCPOS      srcpos;

  srcpos  = VHO_Srcpos;
  do_body = vho_lower_block ( WN_do_body(wn) );

  lower_block = WN_CreateBlock ();
  WN_Set_Linenum ( lower_block, srcpos );

  vho_initialize_bool_info (&bool_info);
  LABEL_Init (New_LABEL (CURRENT_SYMTAB, bool_info.true_label),
	0, LKIND_DEFAULT);
  LABEL_Init (New_LABEL (CURRENT_SYMTAB, bool_info.false_label),
	0, LKIND_DEFAULT);

#if defined( KEY) && !defined(TARG_ST) // bug 6299
  start = WN_start(wn);
  WN_kid0(start) = vho_lower_expr ( WN_kid0(start), lower_block, &bool_info );
#else
  start = vho_lower_expr ( WN_start(wn), lower_block, &bool_info );

  FmtAssert ( WN_first(lower_block) == NULL,
              ( "lowering of do loop start generated statements" ) );
#endif

  end = vho_lower_expr ( WN_end(wn), lower_block, &bool_info );

#if !defined( KEY ) || defined(TARG_ST)
  FmtAssert ( WN_first(lower_block) == NULL,
              ( "lowering of do loop test generated statements" ) );
#else
#ifdef Is_True_On
  // relax assertion, and convert it to debug mode
  Is_True (WN_first (lower_block) == NULL ||
           PU_c_lang (Get_Current_PU())  ||
           PU_cxx_lang (Get_Current_PU()) || 
	   Instrumentation_Enabled,
           ("lowering of do loop test generated statements in Fortran"));
#endif
  // Relax it for DO_LOOP generated for C/C++ OpenMP, when the termination
  // test has function call
  if (WN_first (lower_block))
    WN_INSERT_BlockLast (block, lower_block);
#endif // KEY

#ifdef KEY // bug 14534
  step = WN_step(wn);
  WN_kid0(step) = vho_lower_expr ( WN_kid0(step), lower_block, &bool_info );
#else
  step = vho_lower_expr ( WN_step(wn), lower_block, &bool_info );
#endif

  FmtAssert ( WN_first(lower_block) == NULL,
              ( "lowering of do loop step generated statements" ) );

  Scope_tab [CURRENT_SYMTAB].label_tab->Delete_last (2);
//Fail_FmtAssertion ("TODO_NEW_SYMTAB: VHO_Lower: vho_lower_do_loop");

  WN_start(wn)   = start;
  WN_end(wn)     = end;
  WN_step(wn)    = step;
  WN_do_body(wn) = do_body;

  return wn;
} /* vho_lower_do_loop */


static WN *
vho_lower_do_while ( WN * wn, WN *block )
{
  WN        * test;
  WN        * test_block;
  WN        * while_body;
  BOOL_INFO   bool_info;
  LABEL_IDX   loop_label;

#ifdef KEY
  Misc_Loop_Fusion(wn, block);
#endif
  while_body = vho_lower_block (WN_while_body(wn));

  test_block = WN_CreateBlock ();
  WN_Set_Linenum ( test_block, VHO_Srcpos );

  vho_initialize_bool_info (&bool_info);
  LABEL_Init (New_LABEL (CURRENT_SYMTAB, bool_info.true_label),
	0, LKIND_DEFAULT);
  LABEL_Init (New_LABEL (CURRENT_SYMTAB, bool_info.false_label),
	0, LKIND_DEFAULT);

  test = vho_lower_expr ( WN_while_test(wn), test_block, &bool_info );

  if (    bool_info.used_true_label
       || bool_info.used_false_label ) {

    // Insert a subset of:
    // LABEL <loop_label>
    // <while_body>
    // <test_block>
    // GOTO <loop_label>
    // <false_label>

    if ( bool_info.used_true_label )
      loop_label = bool_info.true_label;

    else
      LABEL_Init (New_LABEL (CURRENT_SYMTAB, loop_label),
	0, LKIND_DEFAULT);

    WN *wn_label = WN_CreateLabel ( (ST_IDX) 0, loop_label, 0, NULL );
    WN_Set_Linenum ( wn_label, VHO_Srcpos );
    WN_INSERT_BlockLast ( block, wn_label );

    WN_INSERT_BlockLast ( block, while_body );

    if ( WN_first(test_block) ) {
      WN_INSERT_BlockLast ( block, test_block );
    }

    WN *wn_back_goto = WN_CreateGoto ( (ST_IDX) NULL, loop_label );
    WN_Set_Linenum ( wn_back_goto, VHO_Srcpos );
    WN_INSERT_BlockLast ( block, wn_back_goto );

    if ( bool_info.used_false_label ) {

      wn_label = WN_CreateLabel ( (ST_IDX) 0, bool_info.false_label, 0, NULL );
      WN_Set_Linenum ( wn_label, VHO_Srcpos );
      WN_INSERT_BlockLast ( block, wn_label );
    }

    // Don't update feedback; let propagate handle updating frequencies

    wn = NULL;
  }

  else {

    if ( WN_first(test_block) )
      WN_INSERT_BlockLast ( while_body, test_block );

    WN_while_test(wn) = test;
    WN_while_body(wn) = while_body;
  }

  return wn;
} /* vho_lower_do_while */

// ============================================================================

static BOOL vho_lower_labels_defined = FALSE;
static BOOL vho_lower_too_many_label_definitions = FALSE;

typedef struct label_map_t {
  LABEL_IDX old_label;
  LABEL_IDX new_label;
  INT32 label_flag;
} LABEL_MAP;

static LABEL_MAP vho_lower_label_map [16];
static INT32 vho_lower_label_map_max = 16;
static INT32 vho_lower_label_map_index = -1;

static void
vho_lower_check_labels ( WN * wn )
{
  WN    * wn1;
  INT32   i;

  switch ( WN_operator(wn) ) {

    case OPR_FUNC_ENTRY:

      vho_lower_check_labels ( WN_func_body(wn) );
      break;

    case OPR_BLOCK:

      for ( wn1 = WN_first(wn); wn1; wn1 = WN_next(wn1) )
        vho_lower_check_labels ( wn1 );
      break;

    case OPR_REGION:

      vho_lower_check_labels ( WN_region_body(wn) );
      break;

    case OPR_REGION_EXIT:

      break;

    case OPR_DO_LOOP:

      vho_lower_check_labels ( WN_index(wn) );
      vho_lower_check_labels ( WN_start(wn) );
      vho_lower_check_labels ( WN_end(wn) );
      vho_lower_check_labels ( WN_step(wn) );
      vho_lower_check_labels ( WN_do_body(wn) );
      break;

    case OPR_DO_WHILE:
    case OPR_WHILE_DO:

      vho_lower_check_labels ( WN_while_test(wn) );
      vho_lower_check_labels ( WN_while_body(wn) );
      break;

    case OPR_IF:

      vho_lower_check_labels ( WN_if_test(wn) );
      vho_lower_check_labels ( WN_then(wn) );
      vho_lower_check_labels ( WN_else(wn) );
      break;

    case OPR_COMPGOTO:
    case OPR_SWITCH:

      for ( i = 0; i < WN_kid_count(wn); i++ )
        vho_lower_check_labels ( WN_kid(wn,i) );
      break;

    case OPR_CASEGOTO:

      break;

    case OPR_XGOTO:

      FmtAssert(FALSE, ("op %s unexpected in vho_lower_check_labels",
      		      OPERATOR_name(WN_operator(wn))));
      break;

    case OPR_GOTO:

      break;

    case OPR_AGOTO:

      FmtAssert(FALSE, ("op %s unexpected in vho_lower_check_labels",
      		      OPERATOR_name(WN_operator(wn))));
      break;

    case OPR_ALTENTRY:

      break;

    case OPR_TRUEBR:

      vho_lower_check_labels ( WN_kid0(wn) );
      break;

    case OPR_RETURN:
#if defined( KEY) && !defined(TARG_ST)
    case OPR_GOTO_OUTER_BLOCK:
#endif

      break;

    case OPR_LABEL:

      vho_lower_labels_defined = TRUE;

      if ( ++vho_lower_label_map_index >= vho_lower_label_map_max )
        vho_lower_too_many_label_definitions = TRUE;

      else {

        vho_lower_label_map [vho_lower_label_map_index].old_label =
          WN_label_number(wn);
        LABEL_Init (New_LABEL (CURRENT_SYMTAB,
		vho_lower_label_map [vho_lower_label_map_index].new_label),
		0, LKIND_DEFAULT);
        vho_lower_label_map [vho_lower_label_map_index].label_flag =
          WN_label_flag(wn);
      }
      break;

    case OPR_EXC_SCOPE_BEGIN:
    case OPR_EXC_SCOPE_END:
    case OPR_FORWARD_BARRIER:
    case OPR_BACKWARD_BARRIER:

      break;

    case OPR_ISTORE:

      vho_lower_check_labels ( WN_kid0(wn) );
      vho_lower_check_labels ( WN_kid1(wn) );
      break;

    case OPR_ISTOREX:

      FmtAssert(FALSE, ("op %s unexpected in vho_lower_check_labels",
      		      OPERATOR_name(WN_operator(wn))));
      break;

    case OPR_MSTORE:

      vho_lower_check_labels ( WN_kid0(wn) );
      vho_lower_check_labels ( WN_kid1(wn) );
      vho_lower_check_labels ( WN_kid2(wn) );
      break;

    case OPR_STID:

      vho_lower_check_labels ( WN_kid0(wn) );
      break;

    case OPR_CALL:
    case OPR_INTRINSIC_CALL:
    case OPR_ICALL:

      for ( i = 0; i < WN_kid_count(wn); i++ )
        vho_lower_check_labels ( WN_actual(wn,i) );
      break;


    case OPR_PICCALL:

      FmtAssert(FALSE, ("op %s unexpected in vho_lower_check_labels",
      		      OPERATOR_name(WN_operator(wn))));
      break;

    case OPR_EVAL:

      vho_lower_check_labels ( WN_kid0(wn) );
      break;

    case OPR_PRAGMA:
    case OPR_XPRAGMA:
    case OPR_PREFETCH:
    case OPR_PREFETCHX:

      break;

    case OPR_IO:

      FmtAssert(FALSE, ("op %s unexpected in vho_lower_check_labels",
      		      OPERATOR_name(WN_operator(wn))));
      break;

    case OPR_COMMENT:

      break;

    case OPR_ILOAD:

      vho_lower_check_labels ( WN_kid0(wn) );
      break;

    case OPR_ILOADX:

      FmtAssert(FALSE, ("op %s unexpected in vho_lower_check_labels",
      		      OPERATOR_name(WN_operator(wn))));
      break;

    case OPR_MLOAD:

      vho_lower_check_labels ( WN_kid0(wn) );
      vho_lower_check_labels ( WN_kid1(wn) );
      break;

    case OPR_ARRAY:

      vho_lower_check_labels ( WN_kid0(wn) );

      for ( i = (WN_kid_count(wn) + 1) >> 1; i < WN_kid_count(wn); i++ )
        vho_lower_check_labels ( WN_kid(wn,i) );
      break;

    case OPR_INTRINSIC_OP:

      for ( i = 0; i < WN_kid_count(wn); i++ )
        vho_lower_check_labels ( WN_kid(wn,i) );
      break;

    case OPR_TAS:
#ifdef TARG_ST
  case OPR_SUBPART:
#endif

      vho_lower_check_labels ( WN_kid0(wn) );
      break;

    case OPR_SELECT:

      vho_lower_check_labels ( WN_kid0(wn) );
      vho_lower_check_labels ( WN_kid1(wn) );
      vho_lower_check_labels ( WN_kid2(wn) );
      break;

    case OPR_CVT:
    case OPR_CVTL:
    case OPR_NEG:
    case OPR_ABS:
    case OPR_SQRT:
    case OPR_REALPART:
    case OPR_IMAGPART:
    case OPR_PAREN:
    case OPR_RND:
    case OPR_TRUNC:
    case OPR_CEIL:
    case OPR_FLOOR:
    case OPR_BNOT:
    case OPR_LNOT:

      vho_lower_check_labels ( WN_kid0(wn) );
      break;

    case OPR_ADD:
    case OPR_SUB:
    case OPR_MPY:
    case OPR_DIV:
    case OPR_MOD:
    case OPR_REM:
    case OPR_DIVREM:
#if defined( KEY) && !defined(TARG_ST)
    case OPR_COMPOSE_BITS:
#endif

      vho_lower_check_labels ( WN_kid0(wn) );
      break;

    case OPR_LOWPART:
    case OPR_HIGHPART:

      vho_lower_check_labels ( WN_kid0(wn) );
      break;

    case OPR_MAX:
    case OPR_MIN:
    case OPR_MINMAX:

      vho_lower_check_labels ( WN_kid0(wn) );
      break;

    case OPR_MINPART:
    case OPR_MAXPART:

      vho_lower_check_labels ( WN_kid0(wn) );
      break;

    case OPR_BAND:
    case OPR_BIOR:
    case OPR_BNOR:
    case OPR_BXOR:
    case OPR_LAND:
    case OPR_LIOR:
    case OPR_CAND:
    case OPR_CIOR:
    case OPR_SHL:
    case OPR_ASHR:
    case OPR_LSHR:
    case OPR_COMPLEX:

      vho_lower_check_labels ( WN_kid0(wn) );
      break;

    case OPR_RECIP:
    case OPR_RSQRT:

      vho_lower_check_labels ( WN_kid0(wn) );
      break;

    case OPR_MADD:
    case OPR_MSUB:
    case OPR_NMADD:
    case OPR_NMSUB:

      vho_lower_check_labels ( WN_kid0(wn) );
      vho_lower_check_labels ( WN_kid1(wn) );
      vho_lower_check_labels ( WN_kid2(wn) );
      break;

    case OPR_IO_ITEM:

      FmtAssert(FALSE, ("op %s unexpected in vho_lower_check_labels",
      		      OPERATOR_name(WN_operator(wn))));
      break;

    case OPR_EQ:
    case OPR_NE:
    case OPR_GT:
    case OPR_GE:
    case OPR_LT:
    case OPR_LE:

      vho_lower_check_labels ( WN_kid0(wn) );
      vho_lower_check_labels ( WN_kid1(wn) );
      break;

    case OPR_LDID:
    case OPR_LDA:
    case OPR_CONST:
    case OPR_INTCONST:
    case OPR_IDNAME:

      break;

    case OPR_FALSEBR:

      vho_lower_check_labels ( WN_kid0(wn) );
      break;

    case OPR_LOOP_INFO:

      FmtAssert(FALSE, ("op %s unexpected in vho_lower_check_labels",
      		      OPERATOR_name(WN_operator(wn))));
      break;

    case OPR_TRAP:
    case OPR_ASSERT:

      FmtAssert(FALSE, ("op %s unexpected in vho_lower_check_labels",
      		      OPERATOR_name(WN_operator(wn))));
      break;

    case OPR_PARM:

      vho_lower_check_labels ( WN_kid0(wn) );
      break;

    case OPR_OPT_CHI:
    case OPR_TRIPLET:
    case OPR_ARRAYEXP:
    case OPR_ARRSECTION:
    case OPR_WHERE:
    case OPR_OPT_RESERVE2:

      FmtAssert(FALSE, ("op %s unexpected in vho_lower_check_labels",
      		      OPERATOR_name(WN_operator(wn))));
      break;

    case OPR_COMMA:
    case OPR_RCOMMA:

      vho_lower_check_labels ( WN_kid0(wn) );
      vho_lower_check_labels ( WN_kid1(wn) );
      break;

    case OPR_HIGHMPY:
    case OPR_XMPY:

      FmtAssert(FALSE, ("op %s unexpected in vho_lower_check_labels",
      		      OPERATOR_name(WN_operator(wn))));
      break;

    case OPR_CSELECT:

      vho_lower_check_labels ( WN_kid0(wn) );
      vho_lower_check_labels ( WN_kid1(wn) );
      vho_lower_check_labels ( WN_kid2(wn) );
      break;
  }
} /* vho_lower_check_labels */


static void
vho_lower_rename_labels_defined ( WN * wn )
{
  WN    * wn1;
  INT32   i;

  switch ( WN_operator(wn) ) {

    case OPR_FUNC_ENTRY:

      vho_lower_rename_labels_defined ( WN_func_body(wn) );
      break;

    case OPR_BLOCK:

      for ( wn1 = WN_first(wn); wn1; wn1 = WN_next(wn1) )
        vho_lower_rename_labels_defined ( wn1 );
      break;

    case OPR_REGION:

      vho_lower_rename_labels_defined ( WN_region_body(wn) );
      break;

    case OPR_REGION_EXIT:

      break;

    case OPR_DO_LOOP:

      vho_lower_rename_labels_defined ( WN_index(wn) );
      vho_lower_rename_labels_defined ( WN_start(wn) );
      vho_lower_rename_labels_defined ( WN_end(wn) );
      vho_lower_rename_labels_defined ( WN_step(wn) );
      vho_lower_rename_labels_defined ( WN_do_body(wn) );
      break;

    case OPR_DO_WHILE:
    case OPR_WHILE_DO:

      vho_lower_rename_labels_defined ( WN_while_test(wn) );
      vho_lower_rename_labels_defined ( WN_while_body(wn) );
      break;

    case OPR_IF:

      vho_lower_rename_labels_defined ( WN_if_test(wn) );
      vho_lower_rename_labels_defined ( WN_then(wn) );
      vho_lower_rename_labels_defined ( WN_else(wn) );
      break;

    case OPR_COMPGOTO:
    case OPR_SWITCH:

      for ( i = 0; i < WN_kid_count(wn); i++ )
        vho_lower_rename_labels_defined ( WN_kid(wn,i) );
      break;

    case OPR_CASEGOTO:

/*    vho_lower_labels_referenced = TRUE; */
      break;

    case OPR_XGOTO:

      FmtAssert(FALSE, ("op %s unexpected in vho_lower_rename_labels_defined",
      		      OPERATOR_name(WN_operator(wn))));
      break;

    case OPR_GOTO:

/*    vho_lower_labels_referenced = TRUE; */
      break;

    case OPR_AGOTO:

      FmtAssert(FALSE, ("op %s unexpected in vho_lower_rename_labels_defined",
      		      OPERATOR_name(WN_operator(wn))));
      break;

    case OPR_ALTENTRY:

      break;

    case OPR_TRUEBR:

      vho_lower_rename_labels_defined ( WN_kid0(wn) );
/*    vho_lower_labels_referenced = TRUE; */
      break;

    case OPR_RETURN:
#if defined( KEY) && !defined(TARG_ST)
    case OPR_GOTO_OUTER_BLOCK:
#endif

      break;

    case OPR_LABEL:

      vho_lower_labels_defined = TRUE;
      break;

    case OPR_EXC_SCOPE_BEGIN:
    case OPR_EXC_SCOPE_END:
    case OPR_FORWARD_BARRIER:
    case OPR_BACKWARD_BARRIER:

      break;

    case OPR_ISTORE:

      vho_lower_rename_labels_defined ( WN_kid0(wn) );
      vho_lower_rename_labels_defined ( WN_kid1(wn) );
      break;

    case OPR_ISTOREX:

      FmtAssert(FALSE, ("op %s unexpected in vho_lower_rename_labels_defined",
      		      OPERATOR_name(WN_operator(wn))));
      break;

    case OPR_MSTORE:

      vho_lower_rename_labels_defined ( WN_kid0(wn) );
      vho_lower_rename_labels_defined ( WN_kid1(wn) );
      vho_lower_rename_labels_defined ( WN_kid2(wn) );
      break;

    case OPR_STID:

      vho_lower_rename_labels_defined ( WN_kid0(wn) );
      break;

    case OPR_CALL:
    case OPR_INTRINSIC_CALL:
    case OPR_ICALL:

      for ( i = 0; i < WN_kid_count(wn); i++ )
        vho_lower_rename_labels_defined ( WN_actual(wn,i) );
      break;


    case OPR_PICCALL:

      FmtAssert(FALSE, ("op %s unexpected in vho_lower_rename_labels_defined",
      		      OPERATOR_name(WN_operator(wn))));
      break;

    case OPR_EVAL:

      vho_lower_rename_labels_defined ( WN_kid0(wn) );
      break;

    case OPR_PRAGMA:
    case OPR_XPRAGMA:
    case OPR_PREFETCH:
    case OPR_PREFETCHX:

      break;

    case OPR_IO:

      FmtAssert(FALSE, ("op %s unexpected in vho_lower_rename_labels_defined",
      		      OPERATOR_name(WN_operator(wn))));
      break;

    case OPR_COMMENT:

      break;

    case OPR_ILOAD:

      vho_lower_rename_labels_defined ( WN_kid0(wn) );
      break;

    case OPR_ILOADX:

      FmtAssert(FALSE, ("op %s unexpected in vho_lower_rename_labels_defined",
      		      OPERATOR_name(WN_operator(wn))));
      break;

    case OPR_MLOAD:

      vho_lower_rename_labels_defined ( WN_kid0(wn) );
      vho_lower_rename_labels_defined ( WN_kid1(wn) );
      break;

    case OPR_ARRAY:

      vho_lower_rename_labels_defined ( WN_kid0(wn) );

      for ( i = (WN_kid_count(wn) + 1) >> 1; i < WN_kid_count(wn); i++ )
        vho_lower_rename_labels_defined ( WN_kid(wn,i) );
      break;

    case OPR_INTRINSIC_OP:

      for ( i = 0; i < WN_kid_count(wn); i++ )
        vho_lower_rename_labels_defined ( WN_kid(wn,i) );
      break;

    case OPR_TAS:
#ifdef TARG_ST
  case OPR_SUBPART:
#endif

      vho_lower_rename_labels_defined ( WN_kid0(wn) );
      break;

    case OPR_SELECT:

      vho_lower_rename_labels_defined ( WN_kid0(wn) );
      vho_lower_rename_labels_defined ( WN_kid1(wn) );
      vho_lower_rename_labels_defined ( WN_kid2(wn) );
      break;

    case OPR_CVT:
    case OPR_CVTL:
    case OPR_NEG:
    case OPR_ABS:
    case OPR_SQRT:
    case OPR_REALPART:
    case OPR_IMAGPART:
    case OPR_PAREN:
    case OPR_RND:
    case OPR_TRUNC:
    case OPR_CEIL:
    case OPR_FLOOR:
    case OPR_BNOT:
    case OPR_LNOT:

      vho_lower_rename_labels_defined ( WN_kid0(wn) );
      break;

    case OPR_ADD:
    case OPR_SUB:
    case OPR_MPY:
    case OPR_DIV:
    case OPR_MOD:
    case OPR_REM:
    case OPR_DIVREM:
#if defined( KEY) && !defined(TARG_ST)
    case OPR_COMPOSE_BITS:
#endif

      vho_lower_rename_labels_defined ( WN_kid0(wn) );
      break;

    case OPR_LOWPART:
    case OPR_HIGHPART:

      vho_lower_rename_labels_defined ( WN_kid0(wn) );
      break;

    case OPR_MAX:
    case OPR_MIN:
    case OPR_MINMAX:

      vho_lower_rename_labels_defined ( WN_kid0(wn) );
      break;

    case OPR_MINPART:
    case OPR_MAXPART:

      vho_lower_rename_labels_defined ( WN_kid0(wn) );
      break;

    case OPR_BAND:
    case OPR_BIOR:
    case OPR_BNOR:
    case OPR_BXOR:
    case OPR_LAND:
    case OPR_LIOR:
    case OPR_CAND:
    case OPR_CIOR:
    case OPR_SHL:
    case OPR_ASHR:
    case OPR_LSHR:
    case OPR_COMPLEX:

      vho_lower_rename_labels_defined ( WN_kid0(wn) );
      break;

    case OPR_RECIP:
    case OPR_RSQRT:

      vho_lower_rename_labels_defined ( WN_kid0(wn) );
      break;

    case OPR_MADD:
    case OPR_MSUB:
    case OPR_NMADD:
    case OPR_NMSUB:

      vho_lower_rename_labels_defined ( WN_kid0(wn) );
      vho_lower_rename_labels_defined ( WN_kid1(wn) );
      vho_lower_rename_labels_defined ( WN_kid2(wn) );
      break;

    case OPR_IO_ITEM:

      FmtAssert(FALSE, ("op %s unexpected in vho_lower_rename_labels_defined",
      		      OPERATOR_name(WN_operator(wn))));
      break;

    case OPR_EQ:
    case OPR_NE:
    case OPR_GT:
    case OPR_GE:
    case OPR_LT:
    case OPR_LE:

      vho_lower_rename_labels_defined ( WN_kid0(wn) );
      vho_lower_rename_labels_defined ( WN_kid1(wn) );
      break;

    case OPR_LDID:
    case OPR_LDA:
    case OPR_CONST:
    case OPR_INTCONST:
    case OPR_IDNAME:

      break;

    case OPR_FALSEBR:

      vho_lower_rename_labels_defined ( WN_kid0(wn) );
/*    vho_lower_labels_referenced = TRUE; */
      break;

    case OPR_LOOP_INFO:

      FmtAssert(FALSE, ("op %s unexpected in vho_lower_rename_labels_defined",
      		      OPERATOR_name(WN_operator(wn))));
      break;

    case OPR_TRAP:
    case OPR_ASSERT:

      FmtAssert(FALSE, ("op %s unexpected in vho_lower_rename_labels_defined",
      		      OPERATOR_name(WN_operator(wn))));
      break;

    case OPR_PARM:

      vho_lower_rename_labels_defined ( WN_kid0(wn) );
      break;

    case OPR_OPT_CHI:
    case OPR_TRIPLET:
    case OPR_ARRAYEXP:
    case OPR_ARRSECTION:
    case OPR_WHERE:
    case OPR_OPT_RESERVE2:

      FmtAssert(FALSE, ("op %s unexpected in vho_lower_rename_labels_defined",
      		      OPERATOR_name(WN_operator(wn))));
      break;

    case OPR_COMMA:
    case OPR_RCOMMA:

      vho_lower_rename_labels_defined ( WN_kid0(wn) );
      vho_lower_rename_labels_defined ( WN_kid1(wn) );
      break;

    case OPR_HIGHMPY:
    case OPR_XMPY:

      FmtAssert(FALSE, ("op %s unexpected in vho_lower_rename_labels_defined",
      		      OPERATOR_name(WN_operator(wn))));
      break;

    case OPR_CSELECT:

      vho_lower_rename_labels_defined ( WN_kid0(wn) );
      vho_lower_rename_labels_defined ( WN_kid1(wn) );
      vho_lower_rename_labels_defined ( WN_kid2(wn) );
      break;
  }
} /* vho_lower_rename_labels_defined */


//LLC LLC (A)
/*-----------------------------------------------------
This routine replaces all the LDIDs from a temp defined 
in stid by the "org" LDID from the original variable.
It also sets "replaced" TRUE if any is replaced
----------------------------------------------------*/
static WN *
Substitute_LDID(WN *wn, WN *stid, WN *org, BOOL &replaced)
{
    if (WN_operator(wn) == OPR_LDID && 
        WN_st(wn) == WN_st(stid) && WN_offset(wn) == WN_offset(stid)) {
          replaced = TRUE;
          return WN_COPY_Tree(org);
    }
    else if(OPCODE_is_leaf(WN_opcode(wn))) 
        return wn;
    else { //Recursively Substitute_LDID the kids
        for (int i=0; i< WN_kid_count(wn); i++)
            WN_kid(wn,i) = Substitute_LDID(WN_kid(wn,i), stid, org, replaced);
        return wn;
    }
}

/*---------------------------------------------------------------------------------
 This routine tries to eliminate the introduced temp in the while loop, say 264, 
 in the example below so that the while loop can be transformed to DO_LOOP later.

 BLOCK
  I4I4LDID 0 <2,2,len> T<4,.predef_I4,4>   --> org_var
 I4STID 264 <1,2,.preg_I4> T<4,.predef_I4,4> # <preg> --> st_tmp
   I4I4LDID 264 <1,2,.preg_I4> T<4,.predef_I4,4> # <preg>
   I4INTCONST constant_number
  I4ADD
 I4STID 0 <2,2,len> T<4,.predef_I4,4>
 END_BLOCK
  I4I4LDID 264 <1,2,.preg_I4> T<4,.predef_I4,4> # <preg>
  I4INTCONST 0 (0x0)
 I4I4GT   -->cmp_wn
I4COMMA // only this kind of pattern will be processed
------------------------------------------------------------------------------------*/

static void
Eliminate_Temp_In_While(WN *test_wn)
{
  WN *stmt, *inc_const;
  WN *preg_ldid;
  BOOL replaced = FALSE;

  if (WN_operator(test_wn)== OPR_COMMA && WN_operator(WN_kid0(test_wn)) == OPR_BLOCK) {
      WN *cmp_wn = WN_kid1(test_wn);   //comparison 
      WN *block_wn = WN_kid0(test_wn); //BLOCK
      WN *st_tmp = WN_first(block_wn); //1st statement in BLOCK
      WN *org_var = WN_kid0(st_tmp);

      if (WN_operator_is(st_tmp, OPR_STID) && ST_class(WN_st(st_tmp)) == CLASS_PREG &&
          OPERATOR_is_compare(WN_operator(cmp_wn)) && WN_operator_is(WN_kid0(cmp_wn), OPR_LDID) &&
          WN_operator_is(WN_kid1(cmp_wn), OPR_INTCONST) ) {

          INT count = 0; //count the number of other statments in BLOCK
          for (stmt = WN_next(st_tmp); stmt; stmt = WN_next(stmt)) 
               count++;

          stmt = WN_next(st_tmp);
          preg_ldid = WN_kid0(WN_kid0(stmt));

          // Find the pattern that "original varialbe" is "post-incremented" by constant
          // by loading the value from the tmp register
          if (count == 1 &&  WN_operator(stmt) == OPR_STID && 
	      WN_operator_is(org_var, OPR_LDID) &&
              WN_st(stmt)== WN_st(org_var) && WN_offset(stmt)== WN_offset(org_var) &&  
              WN_operator_is(WN_kid0(stmt),OPR_ADD) &&
              WN_operator_is(WN_kid1(WN_kid0(stmt)), OPR_INTCONST) &&
              WN_st(preg_ldid) == WN_st(st_tmp) && WN_offset(preg_ldid) == WN_offset(st_tmp)) {

             replaced=FALSE;
             stmt = Substitute_LDID(stmt, st_tmp, org_var, replaced);

             if (replaced) {
                 inc_const = WN_kid1(WN_kid0(stmt));

                 //Deal with the test condition -- need to subtract the post-increment
                 OPCODE mpyopcode = OPCODE_make_op(OPR_SUB, WN_rtype(org_var),MTYPE_V);
                 WN *sub_wn = WN_CreateExp2(mpyopcode, WN_COPY_Tree(org_var), WN_COPY_Tree(inc_const));
                 replaced = FALSE;
                 cmp_wn = Substitute_LDID(cmp_wn, st_tmp, sub_wn, replaced);
                 Is_True(replaced, ("there should be one to be replaced"));
              } //if replaced
          }
     }  
  } //COMMA & BLOCk
} //Eliminate_Temp_In_While
// LLC (A) 

#ifdef KEY
static BOOL Is_Loop_Suitable_For_Misc_Loop_Fusion (WN * wn, WN * block,
                                                   WN **pre_l,
                                                   WN **main_l,
                                                   WN **post_l)
{
  // Check if the inner do loops can be fused.
  WN *body = WN_while_body(wn);
  WN *stmt = WN_first(body);
  WN *preloop = NULL;
  WN *mainloop = NULL;
  WN *postloop = NULL;

  while(stmt) {
    WN * curstmt = stmt;
    if (WN_operator(stmt) == OPR_DO_LOOP &&
        WN_next(stmt)) {
      if (WN_operator(WN_next(stmt)) == OPR_LABEL)
	  stmt = WN_next(stmt);
      if (WN_next(stmt) && WN_operator(WN_next(stmt)) == OPR_STID &&
          WN_desc(WN_next(stmt)) == MTYPE_I4 &&
          WN_next(WN_next(stmt)) && 
          WN_operator(WN_next(WN_next(stmt))) == OPR_DO_LOOP &&
          WN_next(WN_next(WN_next(stmt)))) {
        // potential mainloop
        WN * p_mainloop = WN_next(WN_next(stmt));
        if (WN_operator(WN_next(WN_next(WN_next(stmt)))) == OPR_LABEL)
          stmt = WN_next(stmt);
        if (WN_next(WN_next(WN_next(stmt))) &&
            WN_operator(WN_next(WN_next(WN_next(stmt)))) == OPR_DO_LOOP) {

        if (preloop != NULL)
          return FALSE;
        postloop = WN_next(WN_next(WN_next(stmt)));
	mainloop = p_mainloop;
        stmt = curstmt;
        preloop = stmt;
        // preloop and postloop should have only one stmt.
        if (!WN_do_body(preloop) || !WN_do_body(postloop) ||
	  !WN_first(WN_do_body(preloop)) || 
	  !WN_first(WN_do_body(postloop)) ||
	  (WN_next(WN_first(WN_do_body(preloop))) &&
	   (WN_operator(WN_next(WN_first(WN_do_body(preloop)))) != 
	    OPR_LABEL ||
	    WN_next(WN_next(WN_first(WN_do_body(preloop)))))) ||
	  (WN_next(WN_first(WN_do_body(postloop))) &&
	   (WN_operator(WN_next(WN_first(WN_do_body(postloop)))) != 
	    OPR_LABEL ||
	    WN_next(WN_next(WN_first(WN_do_body(postloop)))))) ||
	  WN_operator(WN_first(WN_do_body(preloop))) != OPR_ISTORE ||
	  WN_operator(WN_first(WN_do_body(postloop))) != OPR_ISTORE ||
	  !WN_start(preloop) || !WN_kid0(WN_start(preloop)) ||
	  !WN_start(mainloop) || !WN_kid0(WN_start(mainloop)) ||
	  !WN_start(postloop) || !WN_kid0(WN_start(postloop)) ||
	  WN_Simp_Compare_Trees(WN_kid0(WN_start(preloop)),
				WN_kid0(WN_start(mainloop))) != 0 ||
	  WN_Simp_Compare_Trees(WN_kid0(WN_start(preloop)),
				WN_kid0(WN_start(postloop))) != 0 ||
	  !WN_end(preloop) || 
	  WN_operator(WN_end(preloop)) != OPR_GT ||
	  !WN_end(postloop) || 
	  WN_operator(WN_end(postloop)) != OPR_GT ||
	  !WN_end(mainloop) || 
	  WN_operator(WN_end(mainloop)) != OPR_GT ||
	  WN_Simp_Compare_Trees(WN_kid0(WN_end(preloop)),
				WN_kid0(WN_end(postloop))) != 0 ||
	  WN_Simp_Compare_Trees(WN_kid0(WN_end(preloop)),
				WN_kid0(WN_end(mainloop))) != 0)
	    preloop = mainloop = postloop = NULL;
        }
      }
    }
    stmt = WN_next(stmt);
  }
  
  *pre_l = preloop;
  *main_l = mainloop;
  *post_l = postloop;
  return preloop != NULL;
}

static void Rename_Subscripts ( WN *to, WN *from, WN *node)
{
  if (WN_operator(node) == OPR_LDID &&
      WN_offset(node) == WN_offset(from))
    WN_offset(node) = WN_offset(to);

  // Recurse
  for (INT kid = 0; kid < WN_kid_count(node); kid ++)
    Rename_Subscripts(to, from, WN_kid(node, kid));
}

static void Misc_Loop_Fusion ( WN * wn, WN * block )
{
  if (!VHO_Enable_Misc_Loop_Fusion)
    return;

  WN *preloop = NULL;
  WN *mainloop = NULL;
  WN *postloop = NULL;
  if (!Is_Loop_Suitable_For_Misc_Loop_Fusion(wn, block,
                                             &preloop, &mainloop, &postloop))
    return;
  
  // Collect the do loops to be fused.
  WN *body = WN_while_body(wn);
#if 0
  WN *stmt = WN_first(body);
  WN *preloop = NULL;
  WN *mainloop = NULL;
  WN *postloop = NULL;
  while(stmt) {
    if (WN_operator(stmt) == OPR_DO_LOOP &&
	WN_next(stmt) && WN_operator(WN_next(stmt)) == OPR_STID &&
	WN_desc(WN_next(stmt)) == MTYPE_I4 &&
	WN_next(WN_next(stmt)) && 
	WN_operator(WN_next(WN_next(stmt))) == OPR_DO_LOOP &&
	WN_next(WN_next(WN_next(stmt))) &&
	WN_operator(WN_next(WN_next(WN_next(stmt)))) == OPR_DO_LOOP) {
      preloop = stmt;
      mainloop = WN_next(WN_next(stmt));
      postloop = WN_next(WN_next(WN_next(stmt)));
      break;
    }
    stmt = WN_next(stmt);
  }
#endif
  FmtAssert(preloop && mainloop && postloop, 
	    ("Handle this case in Misc_Loop_Fusion"));
  
  // Fuse the loops.
  WN *preloopbody = WN_COPY_Tree(WN_do_body(preloop));
  WN *mainloopbody = WN_do_body(mainloop);
  WN *postloopbody = WN_COPY_Tree(WN_do_body(postloop));

  Rename_Subscripts(WN_index(mainloop), WN_index(preloop), 
		    WN_first(preloopbody));
  Rename_Subscripts(WN_index(mainloop), WN_index(postloop), 
		    WN_first(postloopbody));

  // Fuse and move definitions and uses closer together.
  WN *innerloop = WN_first(mainloopbody);
  while (innerloop) {
    if (WN_operator(innerloop) == OPR_DO_LOOP)
      break;
    innerloop = WN_next(innerloop);
  }

  // another safety check.
  if (!innerloop || !WN_next(innerloop) || !WN_next(WN_next(innerloop)))
    return;

  WN_INSERT_BlockAfter ( mainloopbody, innerloop, WN_first(preloopbody));
  WN_INSERT_BlockAfter ( mainloopbody, WN_next(WN_next(innerloop)), 
			 WN_first(postloopbody));

  WN_next(WN_prev(preloop)) = WN_next(preloop);
  WN_prev(WN_next(preloop)) = WN_prev(preloop);
  WN_next(mainloop) = WN_next(postloop);
  WN_prev(WN_next(postloop)) = mainloop;

  WN_DELETE_Tree(preloop);
  WN_DELETE_Tree(postloop);

  return;
}

static WN *vho_lower_while_do ( WN * wn, WN *block );

static BOOL Iload_Inside (WN* wn)
{
  if (WN_operator(wn) == OPR_ILOAD)
    return TRUE;
  else {
    for (INT kid = 0; kid < WN_kid_count(wn); kid ++)
      if (Iload_Inside(WN_kid(wn, kid))) return TRUE;
  }
  return FALSE;
}

static WN* Find_Iload_Iload_Opnd ( WN* wn )
{
  if (WN_operator(wn) == OPR_ILOAD) {
    if (Iload_Inside(WN_kid0(wn)))
      return wn;
    else
      return NULL;
  } else {
    WN* opnd = NULL;
    WN* tree;
    for (INT kid = 0; kid < WN_kid_count(wn); kid ++) {
      if (tree = Find_Iload_Iload_Opnd(WN_kid(wn, kid))) {
	// If more than one iload_iload_opnd then return nothing
	if (opnd && tree) return NULL; 
	else opnd = tree;
      }      
    }
    return opnd;
  }
}

static BOOL 
Is_Loop_Suitable_For_Misc_Loop_Distribute_And_Interchange(WN* wn, WN* block)
{
  WN* last = WN_last(block);
  if (!last) return FALSE;
  if (WN_operator(last) != OPR_STID) {
    if (WN_operator(last) != OPR_LABEL) return FALSE;
    if (!WN_prev(last) ||
	WN_operator(last = WN_prev(last)) != OPR_STID) return FALSE;
  }
  ST* index_outerloop = WN_st(last);
  WN* test_outerloop = WN_while_test(wn);
  if (!OPCODE_is_compare(WN_opcode(test_outerloop))) return FALSE;
  BOOL index_matches = FALSE;
  if (WN_operator(WN_kid0(test_outerloop)) == OPR_LDID &&
      WN_st(WN_kid0(test_outerloop)) == index_outerloop)
    index_matches = TRUE;    
  if (WN_operator(WN_kid1(test_outerloop)) == OPR_LDID &&
      WN_st(WN_kid1(test_outerloop)) == index_outerloop)
    index_matches = TRUE;    
  if (!index_matches)
    return FALSE;

  // The body of the loop should contain an assignment statement then an 
  // if-statement that contains another loop then a label statement and a
  // statement to update the loop variable.
  WN* loop_body = WN_while_body(wn);
  WN* loop_body_start = WN_first(loop_body);
  if (loop_body_start && WN_operator(loop_body_start) == OPR_LABEL)
    loop_body_start = WN_next(loop_body_start);
  if (!loop_body_start ||
      WN_operator(loop_body_start) != OPR_ISTORE) return FALSE;
  if (!WN_next(loop_body_start) ||
      WN_operator(WN_next(loop_body_start)) != OPR_IF) return FALSE;
  WN* if_stmt = WN_next(loop_body_start);
  if (!WN_next(if_stmt)) return FALSE;
  if (WN_operator(WN_next(if_stmt)) == OPR_LABEL) {
    if (!WN_next(WN_next(if_stmt)) ||
	WN_operator(WN_next(WN_next(if_stmt))) != OPR_STID) 
      return FALSE;
  }
  else if (WN_operator(WN_next(if_stmt)) != OPR_STID) 
    return FALSE;
  if (WN_first(WN_else(if_stmt))) return FALSE;
  WN* if_then_body = WN_then(if_stmt);
  if (!WN_first(if_then_body) ||
      WN_operator(WN_first(if_then_body)) != OPR_STID) return FALSE;
  BOOL label_found = FALSE;  
  if (!WN_next(WN_first(if_then_body))) return FALSE;
  if (WN_operator(WN_next(WN_first(if_then_body))) != OPR_WHILE_DO) {
    if (WN_operator(WN_next(WN_first(if_then_body))) != OPR_LABEL)
      return FALSE;
    else
      label_found = TRUE;
  }
 
  // Is it safe and useful to do this distribution and interchange based on the 
  // access patterns? If not, then return FALSE.
  WN *innerloop = WN_next(WN_first(if_then_body));
  if (label_found) innerloop = WN_next(WN_next(WN_first(if_then_body)));
  WN *body = WN_while_body(innerloop);
  WN* stmt = WN_first(body);
  if (stmt && WN_operator(stmt) == OPR_LABEL)
    stmt = WN_next(stmt);
  if (!stmt || WN_operator(stmt) != OPR_ISTORE ||
      !WN_next(stmt)) 
    return FALSE;
  if (WN_operator(WN_next(stmt)) == OPR_LABEL) {
    if (!WN_next(WN_next(stmt)) || 
	WN_operator(WN_next(WN_next(stmt))) != OPR_STID ||
	WN_next(WN_next(WN_next(stmt)))) 
      return FALSE;
  }
  else {
    if (WN_operator(WN_next(stmt)) != OPR_STID ||
        (WN_next(WN_next(stmt)) &&
         WN_operator(WN_next(WN_next(stmt))) != OPR_LABEL))
      return FALSE;
  }
  // Find the operand that is ILOAD(ILOAD(...))
  WN* opnd;
  WN* opnd_base;
  WN* result_base;
  if (!(opnd = Find_Iload_Iload_Opnd(WN_kid0(stmt))))
    return FALSE;
  if (WN_operator(WN_kid1(stmt)) != OPR_ADD ||
      WN_operator(WN_kid1(WN_kid1(stmt))) != OPR_MPY ||
      WN_operator(result_base = WN_kid0(WN_kid1(stmt))) != OPR_LDID)
    return FALSE;
  WN *low_index, *high_index;
  if (WN_operator(low_index = WN_kid0(opnd)) != OPR_ADD ||
      WN_operator(WN_kid0(WN_kid0(opnd))) != OPR_ILOAD ||
      WN_operator(high_index = WN_kid0(WN_kid0(WN_kid0(opnd)))) != OPR_ADD ||
      WN_operator(opnd_base = WN_kid0(WN_kid0(WN_kid0(WN_kid0(opnd))))) != OPR_LDID)
    return FALSE;
  // Extract the loop indices from the access.
  if (WN_operator(WN_kid1(low_index)) != OPR_MPY ||
      WN_operator(WN_kid1(high_index)) != OPR_MPY)
    return FALSE;

  low_index = WN_kid0(WN_kid1(low_index));
  high_index = WN_kid0(WN_kid1(high_index));

  if (WN_operator(low_index) != OPR_LDID &&
      WN_operator(low_index) != OPR_CVT)
    return FALSE;
  if (WN_operator(low_index) == OPR_CVT)
    if (WN_operator(low_index = WN_kid0(low_index)) != OPR_LDID)
      return FALSE;
  if (WN_operator(high_index) != OPR_LDID &&
      WN_operator(high_index) != OPR_CVT)
    return FALSE;
  if (WN_operator(high_index) == OPR_CVT)
    if (WN_operator(high_index = WN_kid0(high_index)) != OPR_LDID)
      return FALSE;
  
  if (WN_st(low_index) != index_outerloop) // not a candidate for interchange
    return FALSE;

  if (WN_st(result_base) == WN_st(opnd_base))
    return FALSE;
  
  return TRUE;
}

// Transform:
//
//   do i
//     stmt-i
//     if (G(i))
//       do j
//         = array[j][i]
//       enddo
//     endif
//   enddo
//
// to:
//
//   do i
//     stmt-i
//   enddo
//   do j
//     do i
//       if (G(i))
//         = array[j][i]
//       endif
//     enddo
//   enddo
// Here array[j][i] is actually declared as **array and dependence information 
// is missing for LNO to do this transformation automatically.
// 
// - Courtesy bug 3581.
static WN* Misc_Loop_Distribute_And_Interchange (WN* wn, WN* block)
{
  if (!VHO_Enable_Misc_Loop_Transformation)
    return NULL;

  if (!Is_Loop_Suitable_For_Misc_Loop_Distribute_And_Interchange(wn, block))
    return NULL;

  WN* last = WN_last(block);
  if (WN_operator(last) == OPR_LABEL)
    last = WN_prev(last);
  WN* start_i_loop = WN_COPY_Tree(last);
  WN* if_stmt = WN_next(WN_first(WN_while_body(wn)));
  WN* end_i_loop;
  if (WN_operator(if_stmt) != OPR_IF) if_stmt = WN_next(if_stmt);
  if (WN_operator(WN_next(if_stmt)) == OPR_LABEL)
    end_i_loop = WN_COPY_Tree(WN_next(WN_next(if_stmt)));
  else end_i_loop = WN_COPY_Tree(WN_next(if_stmt));
  WN* pre_loop = WN_COPY_Tree(wn);
  if (Cur_PU_Feedback) {
    const FB_Info_Loop fb_info = Cur_PU_Feedback->Query_loop(wn);
    Cur_PU_Feedback->Annot_loop( pre_loop, fb_info );
    Cur_PU_Feedback->FB_duplicate( WN_while_test(wn), WN_while_test(pre_loop) );
  }
  WN* while_body = WN_while_body(pre_loop);
  WN* first_stmt = WN_first(while_body);
  if (WN_operator(first_stmt) == OPR_LABEL) first_stmt = WN_next(first_stmt);
  WN_DELETE_Tree(WN_next(WN_next(first_stmt)));
  WN_DELETE_Tree(WN_next(first_stmt));
  WN_next(first_stmt) = end_i_loop;
  pre_loop = vho_lower_while_do(pre_loop, block);
  WN_INSERT_BlockLast ( block, pre_loop );

  first_stmt = WN_first(WN_while_body(wn));
  if (WN_operator(first_stmt) == OPR_LABEL) first_stmt = WN_next(first_stmt);
  WN* start_j_loop = 
    WN_COPY_Tree(WN_first(WN_then(WN_next(first_stmt))));
  WN_INSERT_BlockLast ( block, start_j_loop ); 
  WN* post_loop = 
    WN_next(WN_first(WN_then(WN_next(first_stmt))));
  WN* orig_loop = post_loop;
  if (WN_operator(post_loop) == OPR_LABEL)
    post_loop = WN_COPY_Tree(orig_loop = WN_next(post_loop));
  else
    post_loop = WN_COPY_Tree(post_loop);
  if (Cur_PU_Feedback) {
    const FB_Info_Loop fb_info_outer = Cur_PU_Feedback->Query_loop(wn);
    const FB_Info_Loop fb_info_inner = Cur_PU_Feedback->Query_loop(orig_loop);
    FB_Info_Loop fb_info = fb_info_outer;
    fb_info.freq_iterate._value = 
      fb_info_inner.freq_out._value == 0.0 ? 0.0 :	// bug 10013
        fb_info.freq_out._value * 
        ( fb_info_inner.freq_iterate._value/fb_info_inner.freq_out._value );
    fb_info.freq_back._value = fb_info.freq_iterate._value - 
      fb_info.freq_positive._value;
    Cur_PU_Feedback->Annot_loop( post_loop, fb_info );
  }
  BOOL found_label = WN_operator(WN_first(WN_while_body(post_loop))) == OPR_LABEL;
  WN* end_j_loop = WN_next(WN_first(WN_while_body(post_loop)));
  if (found_label)
  {
    end_j_loop = WN_next(end_j_loop);
    WN_DELETE_Tree(WN_next(WN_first(WN_while_body(post_loop))));
    WN_next(WN_first(WN_while_body(post_loop))) = start_i_loop;
  }
  else
  {
    WN_DELETE_Tree(WN_first(WN_while_body(post_loop)));
    WN_first(WN_while_body(post_loop)) = start_i_loop;
  }
  WN* j_loop_body = WN_COPY_Tree(wn);
  WN* i_loop_body = WN_while_body(j_loop_body);
  if (WN_operator(WN_first(i_loop_body)) == OPR_LABEL)
    WN_DELETE_FromBlock(i_loop_body, WN_first(i_loop_body));

  WN_DELETE_Tree(WN_first(i_loop_body));
  WN_first(i_loop_body) = WN_next(WN_first(i_loop_body));
  if_stmt = WN_first(i_loop_body);
  if (Cur_PU_Feedback) {
    const FB_Info_Loop fb_info_outer = Cur_PU_Feedback->Query_loop(post_loop);
    const FB_Info_Loop fb_info_orig = Cur_PU_Feedback->Query_loop(wn);
    FB_Info_Loop fb_info = fb_info_outer;
    fb_info.freq_zero = fb_info_outer.freq_zero;
    fb_info.freq_positive = fb_info_outer.freq_iterate;
    fb_info.freq_out = fb_info_outer.freq_iterate;
    fb_info.freq_exit = fb_info_outer.freq_iterate;
    fb_info.freq_iterate = fb_info_outer.freq_iterate;
    fb_info.freq_iterate._value *= (fb_info_orig.freq_iterate._value /
				    fb_info_orig.freq_positive._value);
    fb_info.freq_back._value = fb_info.freq_iterate._value - 
      fb_info.freq_positive._value;
    Cur_PU_Feedback->Annot_loop( j_loop_body, fb_info );

    const FB_Info_Branch& info_branch_orig = (WN_operator(WN_first(WN_while_body(wn))) == OPR_LABEL) ?
          Cur_PU_Feedback->Query_branch(WN_next(WN_next(WN_first(WN_while_body(wn))))) :
          Cur_PU_Feedback->Query_branch( WN_next(WN_first(WN_while_body(wn))));

    FB_Info_Branch info_branch = info_branch_orig;
    info_branch.freq_taken._value = 
      fb_info.freq_iterate._value * (info_branch_orig.freq_taken._value /
				     (info_branch_orig.freq_taken._value +
				      info_branch_orig.freq_not_taken._value));
    info_branch.freq_not_taken._value = 
      fb_info.freq_iterate._value - info_branch.freq_taken._value;
    Cur_PU_Feedback->Annot_branch( if_stmt, 
    				   FB_Info_Branch( info_branch.freq_taken,
                                                   info_branch.freq_not_taken,
					           OPR_IF ));
  }
  WN* stmt;
  if (WN_operator(stmt = WN_next(WN_first(WN_then(if_stmt)))) == OPR_WHILE_DO)
  {
    WN * first_stmt = WN_first(WN_while_body(stmt));
    if (WN_operator(first_stmt) == OPR_LABEL)
      WN_first(WN_then(if_stmt)) = WN_next(first_stmt);
    else
      WN_first(WN_then(if_stmt)) = first_stmt;
  }
  else
  {
    WN * first_stmt = WN_first(WN_while_body(WN_next(stmt)));
    if (WN_operator(first_stmt) == OPR_LABEL)
      WN_first(WN_then(if_stmt)) = WN_next(first_stmt);
    else
      WN_first(WN_then(if_stmt)) = first_stmt;
  }
  WN_DELETE_Tree(WN_next(WN_first(WN_then(if_stmt))));
  WN_next(WN_first(WN_then(if_stmt))) = NULL;
  if (found_label)
  {
    WN_next(WN_next(WN_first(WN_while_body(post_loop)))) = j_loop_body;
    WN_next(WN_next(WN_next(WN_first(WN_while_body(post_loop))))) = end_j_loop;
  }
  else
  {
    WN_next(WN_first(WN_while_body(post_loop))) = j_loop_body;
    WN_next(WN_next(WN_first(WN_while_body(post_loop)))) = end_j_loop;
  }
  
  post_loop = vho_lower_while_do(post_loop, block);

  WN_DELETE_Tree(wn);
  return post_loop;
}
#endif

static WN *
vho_lower_while_do ( WN * wn, WN *block )
{
  WN        * while_body;
  WN        * test;
  WN        * test_block;
  WN        * test_original;
  BOOL_INFO   bool_info;
  LABEL_IDX   loop_label;
  LABEL_IDX   exit_label;
  WN        * rcomma_block;
  PREG_NUM    last_preg;

#ifdef KEY
  WN* return_block = Misc_Loop_Distribute_And_Interchange(wn, block);
  if (return_block) return return_block;
  Misc_Loop_Fusion(wn, block);
#endif
  while_body = vho_lower_block (WN_while_body(wn));

  test_block = WN_CreateBlock ();
  WN_Set_Linenum ( test_block, VHO_Srcpos );

  vho_initialize_bool_info (&bool_info);

  LABEL_Init (New_LABEL (CURRENT_SYMTAB, bool_info.true_label),
	0, LKIND_DEFAULT);
  LABEL_Init (New_LABEL (CURRENT_SYMTAB, bool_info.false_label),
	0, LKIND_DEFAULT);

  last_preg = PREG_Table_Size (CURRENT_SYMTAB);

  // Find the loop test and any RCOMMA block
  if ( WN_operator( WN_while_test(wn) ) == OPR_RCOMMA ) {
    test_original = WN_kid0( WN_while_test(wn) );
    rcomma_block  = vho_lower_block ( WN_kid1( WN_while_test(wn) ) );
  } else {
    test_original = WN_while_test(wn);
    rcomma_block = NULL;
  }

  // LLC (B)
  // Call to eliminate unncessary introduced temp register by frontend
  Eliminate_Temp_In_While(test_original);
  //LLC

  // Clone the loop test
#ifndef KEY
  WN *copy = WN_COPY_Tree( test_original );
#else
  WN *copy = WN_COPY_Tree_With_Map( test_original );
#endif

  if ( Cur_PU_Feedback ) {
    // Guess that the loop test will not be cloned
    Cur_PU_Feedback->FB_duplicate( test_original, copy );
  }

  // Lower the loop test
  test = vho_lower_expr ( copy, test_block, &bool_info );

  if (    bool_info.used_true_label
       || bool_info.used_false_label ) {

    // Insert a subset of the following: (test is not cloned)
    // LABEL <loop_label>
    // <test_block>
    // <rcomma_block>
    // GOTO <false_label>
    // LABEL <true_label>
    // <while_body>
    // GOTO <loop_label>
    // LABEL <false_label>
    // <rcomma_block>

    LABEL_Init (New_LABEL (CURRENT_SYMTAB, loop_label),
	0, LKIND_DEFAULT);
    WN *wn_label = WN_CreateLabel ( (ST_IDX) 0, loop_label, 0, NULL );
    WN_Set_Linenum ( wn_label, VHO_Srcpos );
    WN_INSERT_BlockLast ( block, wn_label );

    if ( WN_first(test_block) )
      WN_INSERT_BlockLast ( block, test_block );

    if ( rcomma_block ) {
      WN *rcomma_block_copy = WN_COPY_Tree ( rcomma_block );
      if ( Cur_PU_Feedback )
	Cur_PU_Feedback->FB_clone_loop_test( rcomma_block, rcomma_block_copy,
					     wn );
      WN_INSERT_BlockLast ( block, rcomma_block_copy );
    }

    if ( bool_info.used_true_label ) {

      if ( ! bool_info.used_false_label ) {

        WN *wn_goto = WN_CreateGoto ( (ST_IDX) NULL, bool_info.false_label );
        WN_Set_Linenum ( wn_goto, VHO_Srcpos );
        WN_INSERT_BlockLast ( block, wn_goto );
        bool_info.used_false_label = TRUE;
      }

      WN *wn_label
	= WN_CreateLabel ( (ST_IDX) 0, bool_info.true_label, 0, NULL );
      WN_Set_Linenum ( wn_label, VHO_Srcpos );
      WN_INSERT_BlockLast ( block, wn_label );
    }

    WN_INSERT_BlockLast ( block, while_body ); 

    WN *wn_goto = WN_CreateGoto ( (ST_IDX) NULL, loop_label );
    WN_Set_Linenum ( wn_goto, VHO_Srcpos );
    WN_INSERT_BlockLast ( block, wn_goto );

    if ( bool_info.used_false_label ) {

      WN *wn_label =
	WN_CreateLabel ( (ST_IDX) 0, bool_info.false_label, 0, NULL );
      WN_Set_Linenum ( wn_label, VHO_Srcpos );
      WN_INSERT_BlockLast ( block, wn_label );
    }

    if ( rcomma_block )
      WN_INSERT_BlockLast ( block, rcomma_block );

    // Don't update feedback; leave it to propagation

    wn = NULL;
  }


  else if ( WN_first(test_block) ) {
   
    vho_lower_labels_defined             = FALSE;
    vho_lower_too_many_label_definitions = FALSE;

    vho_lower_check_labels ( test_block );

    vho_lower_too_many_label_definitions = vho_lower_labels_defined;

    if ( VHO_Single_Loop_Test == TRUE
         || vho_lower_too_many_label_definitions ) {

      // Insert a subset of the following: (test is not cloned)
      // LABEL <loop_label>
      // <test_block>
      // <rcomma_block>
      // FALSEBR <exit_label>
      // <while_body>
      // GOTO <loop_label>
      // LABEL <exit_label>
      // <rcomma_block>

      LABEL_Init (New_LABEL (CURRENT_SYMTAB, loop_label),
	0, LKIND_DEFAULT);

      WN *wn_label = WN_CreateLabel ( (ST_IDX) 0, loop_label, 0, NULL );
      WN_Set_Linenum ( wn_label, VHO_Srcpos );
      WN_INSERT_BlockLast ( block, wn_label );

      WN_INSERT_BlockLast ( block, test_block );

      if ( rcomma_block ) {
	WN *rcomma_block_copy = WN_COPY_Tree ( rcomma_block );
	if ( Cur_PU_Feedback )
	  Cur_PU_Feedback->FB_clone_loop_test( rcomma_block,
					       rcomma_block_copy, wn );
        WN_INSERT_BlockLast ( block, rcomma_block_copy );
      }

      LABEL_Init (New_LABEL (CURRENT_SYMTAB, exit_label),
	0, LKIND_DEFAULT);

      WN *wn_top_branch = WN_CreateFalsebr ( exit_label, test );
      WN_Set_Linenum ( wn_top_branch, VHO_Srcpos );
      WN_INSERT_BlockLast ( block, wn_top_branch );

      WN_INSERT_BlockLast ( block, while_body );

      WN *wn_back_goto = WN_CreateGoto ( (ST_IDX) NULL, loop_label );
      WN_Set_Linenum ( wn_back_goto, VHO_Srcpos );
      WN_INSERT_BlockLast ( block, wn_back_goto );

      wn_label = WN_CreateLabel ( (ST_IDX) 0, exit_label, 0, NULL );
      WN_Set_Linenum ( wn_label, VHO_Srcpos );
      WN_INSERT_BlockLast ( block, wn_label );

      if ( rcomma_block )
        WN_INSERT_BlockLast ( block, rcomma_block );

      if ( Cur_PU_Feedback )
	Cur_PU_Feedback->FB_lower_loop_alt ( wn, wn_top_branch );

      wn = NULL;
    }

    else {
      // Case of  bool_info.used_true_label  == FALSE &&
      //          bool_info.used_false_label == FALSE &&
      //          WN_first(test_block) == NULL

      WN *test_duplicate = test;

      if ( Cur_PU_Feedback ) {

	// Oops, test (or at least test_block) is cloned!
	// Back up, clone, and relower test with better feedback data!
	// NOTE: Delete old test_block whirl nodes?
	//   test_top and test have separate (cloned) data
	//   test_duplicate has combined feedback data

	Reset_PREG_Table_Size( CURRENT_SYMTAB, last_preg );

#ifndef KEY
        WN *copy = WN_COPY_Tree( test_original );
#else
        WN *copy = WN_COPY_Tree_With_Map( test_original );
#endif
	Cur_PU_Feedback->FB_clone_loop_test( test_original, copy, wn );

	test_block = WN_CreateBlock ();
	WN_Set_Linenum( test_block, VHO_Srcpos );

	test = vho_lower_expr( copy, test_block, &bool_info );
      }

      WN *test_top = test;
      WN_INSERT_BlockLast ( block, test_block );

      Reset_PREG_Table_Size ( CURRENT_SYMTAB, last_preg );

      test_block = WN_CreateBlock ();
      WN_Set_Linenum ( test_block, VHO_Srcpos );

      test = vho_lower_expr ( test_original, test_block, &bool_info );

      if ( vho_lower_labels_defined )
        vho_lower_rename_labels_defined ( test_block );

      if ( VHO_Use_Do_While ) {

	// Insert a subset of the following: (test is cloned)
	// <test_block>   (already added above)
	// IF
	//  <test>        (test_top)
	// THEN
	//  DO_WHILE
	//   <test>
	//  BODY
	//   <rcomma_block>
	//   <while_body>
	//   <test_block>
	//  END_DO_WHILE
	// END_IF
	// <rcomma_block>

        WN_set_operator ( wn, OPR_DO_WHILE );

        if ( rcomma_block ) {
	  WN *rcomma_block_copy = WN_COPY_Tree ( rcomma_block );
	  if ( Cur_PU_Feedback )
	    Cur_PU_Feedback->FB_clone_loop_test( rcomma_block,
						 rcomma_block_copy, wn );
          WN_INSERT_BlockFirst ( while_body, rcomma_block_copy );
	}

        WN_INSERT_BlockLast ( while_body, test_block );
        WN_while_test(wn) = test;
        WN_while_body(wn) = while_body;

        WN* then_block = WN_CreateBlock ();
        WN* else_block = WN_CreateBlock ();

        WN_INSERT_BlockLast ( then_block, wn );

        WN* wn_top_branch = WN_CreateIf ( test_top, then_block, else_block );
	WN_Set_Linenum ( wn_top_branch, VHO_Srcpos );
        WN_INSERT_BlockLast ( block, wn_top_branch );

        if ( rcomma_block )
          WN_INSERT_BlockLast ( block, rcomma_block );

	if ( Cur_PU_Feedback )
	  Cur_PU_Feedback->FB_lower_while_do_to_do_while ( wn, wn_top_branch );

        wn = NULL;

      } else {  // ! VHO_Use_Do_While

	// Insert a subset of the following: (test_block, not test are cloned)
	// <test_block>   (already added above)
	// WHILE_DO
	//  <test>        (test_duplicate)
	// BODY
	//  <rcomma_block>
	//  <while_body>
	//  <test_block>
	// END_DO_WHILE
	// <rcomma_block>

        WN_INSERT_BlockLast ( while_body, test_block );

        WN_while_test(wn) = test_duplicate;
        WN_while_body(wn) = while_body;

        if ( rcomma_block ) {

	  WN *rcomma_block_copy = WN_COPY_Tree ( rcomma_block );
	  if ( Cur_PU_Feedback )
	    Cur_PU_Feedback->FB_clone_loop_test( rcomma_block,
						 rcomma_block_copy, wn );

          WN_INSERT_BlockFirst ( while_body, rcomma_block_copy );
          WN_INSERT_BlockLast ( block, wn );
          WN_INSERT_BlockLast ( block, rcomma_block );
          wn = NULL;
        }
      }
    }

  } else {  // WN_first( test_block ) == NULL

    // Insert a subset of the following:
    // WHILE_DO
    //  <test>
    // BODY
    //  <rcomma_block>
    //  <while_body>
    // END_DO_WHILE
    // <rcomma_block>

    WN_while_test(wn) = test;
    WN_while_body(wn) = while_body;

    if ( rcomma_block ) {

      WN *rcomma_block_copy = WN_COPY_Tree ( rcomma_block );
      if ( Cur_PU_Feedback )
	Cur_PU_Feedback->FB_clone_loop_test( rcomma_block,
					     rcomma_block_copy, wn );

      WN_INSERT_BlockFirst ( while_body, rcomma_block_copy );
      WN_INSERT_BlockLast ( block, wn );
      WN_INSERT_BlockLast ( block, rcomma_block );
      wn = NULL;
    }
  }

  return wn;
} /* vho_lower_while_do */


static INT32 vho_if_nest_level = 0;

#ifdef KEY
static BOOL 
Address_Expressions_Match ( WN *wn1, WN* wn2 )
{
  if (WN_kid_count(wn1) != WN_kid_count(wn2))
    return FALSE;

  for (INT kid = 0; kid < WN_kid_count(wn1); kid ++) {
    if (WN_operator(WN_kid(wn1, kid)) != WN_operator(WN_kid(wn2, kid)) ||
	WN_desc(WN_kid(wn1, kid)) != WN_desc(WN_kid(wn2, kid)) ||
	WN_rtype(WN_kid(wn1, kid)) != WN_rtype(WN_kid(wn2, kid)))
      return FALSE;
    if (WN_has_sym(WN_kid(wn1, kid))) {
      if (!WN_has_sym(WN_kid(wn2, kid)) ||
	  WN_st(WN_kid(wn2, kid)) != WN_st(WN_kid(wn1, kid)))
	return FALSE;
    }
    if (WN_operator(WN_kid(wn1, kid)) == OPR_INTCONST) {
      if (WN_const_val(WN_kid(wn1, kid)) != WN_const_val(WN_kid(wn2, kid)))
	return FALSE;
    }
    for (INT kid_tmp = 0; kid_tmp < WN_kid_count(WN_kid(wn1, kid)); kid_tmp ++)
      if (!Address_Expressions_Match(WN_kid(wn1, kid), WN_kid(wn2, kid)))
	return FALSE;
  }
  return TRUE;
}

static BOOL
Opcodes_Can_Be_Speculated_Vectorized (WN* wn) 
{
  if (!OPCODE_Can_Be_Speculative(WN_opcode(wn)))
    return FALSE;

  if (WN_operator(wn) == OPR_ILOAD) // note test passed above
    return TRUE;

  // We can If-Convert and vectorize only the FP operations
  if (!MTYPE_is_float(WN_rtype(wn))) 
    return FALSE;

  // Check if the operator can be vectorized. This should be in synch with
  // simd.cxx: is_vectorizable_op (with some exceptions). 
  OPERATOR opr = WN_operator(wn);
  if (!OPCODE_is_load(WN_opcode(wn)) &&
      opr != OPR_NEG && opr != OPR_ADD && opr != OPR_SUB &&
      opr != OPR_MPY && opr != OPR_DIV && opr != OPR_MAX &&
      opr != OPR_MIN && opr != OPR_SQRT && opr != OPR_RSQRT && 
      opr != OPR_RECIP && opr != OPR_CONST && opr != OPR_INTCONST)
    return FALSE;
  
  // Recurse
  for (INT kid = 0; kid < WN_kid_count(wn); kid ++)
    if (!Opcodes_Can_Be_Speculated_Vectorized(WN_kid(wn, kid)))
      return FALSE;

  return TRUE;
}

// The primary goal for this counting algorithm is to reasonably predict
// the overhead in If-Converting the WHIRL tree 'wn'. It assumes that 
// all the operators inside 'wn' are vectorizable (pre-screened by 
// Opcodes_Can_Be_Speculated_And_Vectorized). Count the vector forms.
//
// TODO_1.2 : move this to a target dependent directory table.
//
static INT 
Ifconv_Overhead (WN* wn) {
  OPERATOR opr = WN_operator(wn);
  INT count = 0;
 
  if      (opr == OPR_ILOAD) count = 3;
  else if (opr == OPR_DIV)   count = 40;
  else if (opr == OPR_SQRT)  count = 40;
  else if (opr == OPR_RECIP) count = 4;
  else if (opr == OPR_RSQRT) count = 4;
  else if (opr == OPR_MPY)   count = 5;
  else if (opr == OPR_ADD)   count = 5;
  else if (opr == OPR_SUB)   count = 5;
  else if (opr == OPR_MAX)   count = 3;
  else if (opr == OPR_MIN)   count = 3;
  else                       count = 0; // LDID, CONST, etc.

  // Recurse
  for (INT kid = 0; kid < WN_kid_count(wn); kid ++)
    count += Ifconv_Overhead(WN_kid(wn, kid));

  return count; 
}
#endif

static WN *
vho_lower_if ( WN * wn, WN *block )
{
  WN        * test_block;
  WN        * then_block;
  WN        * else_block;
  BOOL_INFO   bool_info;
  LABEL_IDX   join_label;
  BOOL        emit_join_label;
  WN        * rcomma_block;

#if defined( KEY) && !defined(TARG_ST)
  /* Handle saturation arithmetic SUB operator by converting it 
   * to an intrinsic 
   * if (y >= 0x8000)
   *   x = y - 0x8000;
   * else
   *   x = 0;
   */

  WN* test = WN_if_test(wn);  
  if ( WN_operator(test) == OPR_GT &&
       WN_rtype(test) == MTYPE_I4 &&
       WN_desc(test) == MTYPE_U4 &&
       WN_operator(WN_kid1(test)) == OPR_INTCONST &&
       WN_const_val(WN_kid1(test)) == 0X7fff ) {
    WN* wn_then = WN_then(wn);
    WN* wn_else = WN_else(wn);
    if (WN_first(wn_then) && !WN_next(WN_first(wn_then)) &&
	WN_first(wn_else) && !WN_next(WN_first(wn_else)) &&

	WN_operator(WN_first(wn_then)) == OPR_ISTORE &&
	WN_operator(WN_first(wn_else)) == OPR_ISTORE &&
	WN_desc(WN_first(wn_then)) == MTYPE_U2 &&
	WN_desc(WN_first(wn_else)) == MTYPE_U2 &&

	WN_operator(WN_kid0(WN_first(wn_then))) == OPR_SUB &&
	WN_rtype(WN_kid0(WN_first(wn_then))) == MTYPE_U4 &&
	WN_operator(WN_kid1(WN_kid0(WN_first(wn_then)))) == OPR_INTCONST &&
	WN_const_val(WN_kid1(WN_kid0(WN_first(wn_then)))) == 0x8000 &&

	WN_operator(WN_kid0(WN_first(wn_else))) == OPR_INTCONST &&
	WN_const_val(WN_kid0(WN_first(wn_else))) == 0 &&

	Address_Expressions_Match(WN_kid1(WN_first(wn_then)), 
				  WN_kid1(WN_first(wn_else))) &&
	
	WN_has_sym(WN_kid0(test)) && 
	WN_has_sym(WN_kid0(WN_kid0(WN_first(wn_then)))) &&
	WN_st(WN_kid0(test)) == WN_st(WN_kid0(WN_kid0(WN_first(wn_then))))) {      
      WN* kids[2];
      WN* tmp;
      kids[0] = WN_kid0(test);
      kids[1] = WN_kid1(WN_kid0(WN_first(wn_then)));
      // Create parm nodes for intrinsic op
      kids[0] = WN_CreateParm (MTYPE_U4, kids[0], 
			       Be_Type_Tbl(MTYPE_U4),
			       WN_PARM_BY_VALUE);
      kids[1] = WN_CreateParm (MTYPE_U4, kids[1], 
			       Be_Type_Tbl(MTYPE_U4),
			       WN_PARM_BY_VALUE);
      tmp = WN_Create_Intrinsic(OPCODE_make_op(OPR_INTRINSIC_OP,
					       MTYPE_U4, MTYPE_V),
				INTRN_SUBSU2, 2, kids);
      wn = WN_COPY_Tree(WN_first(wn_then));
      WN_kid0(wn) = tmp;
      return wn;
    }
  }

  // If-Convert:
  //   if <compare>
  //     a[i] = <expr>
  //   endif
  // to:
  //   a[i] = <compare> ? <expr> : a[i]
  // If this select is not vectorized then the scalar form will be
  // converted back to the If-then form to avoid the overhead associated with
  // if-conversion.
  if (VHO_Enable_Simple_If_Conv &&
      WN_first(WN_then(wn)) && !WN_next(WN_first(WN_then(wn))) &&
      !WN_first(WN_else(wn)) &&
      WN_operator(WN_first(WN_then(wn))) == OPR_ISTORE &&
      WN_operator(WN_kid1(WN_first(WN_then(wn)))) == OPR_ARRAY &&
      MTYPE_is_float(WN_desc(WN_first(WN_then(wn)))) &&
      OPCODE_is_compare(WN_opcode(test)) &&
      ((WN_operator(WN_kid0(test)) == OPR_ILOAD && 
	WN_operator(WN_kid0(WN_kid0(test))) == OPR_ARRAY) ||
       (WN_operator(WN_kid1(test)) == OPR_ILOAD && 
	WN_operator(WN_kid0(WN_kid1(test))) == OPR_ARRAY)) &&
#ifndef KEY // bug 14738
      MTYPE_is_float(WN_desc(test)) && 
#endif
      MTYPE_is_integral(WN_rtype(test)) &&
#ifndef KEY // bug 14738
      WN_desc(WN_first(WN_then(wn))) == WN_desc(test) &&
#endif
      Opcodes_Can_Be_Speculated_Vectorized(WN_kid0(WN_first(WN_then(wn)))) &&
      (Ifconv_Overhead(WN_kid0(WN_first(WN_then(wn))))*
       MTYPE_byte_size(WN_desc(test))/16) < VHO_Enable_If_Conv_Limit) {
    WN* istore = WN_first(WN_then(wn));
    WN* t1 = WN_COPY_Tree(WN_kid0(istore));
    WN* t2 = WN_CreateIload(OPR_ILOAD, WN_desc(istore), WN_desc(istore), 
			    WN_store_offset(istore), 
			    TY_pointed(WN_ty(istore)), 
			    WN_ty(istore),
			    WN_COPY_Tree(WN_kid1(istore)),
			    WN_field_id(istore));
    wn = WN_COPY_Tree(istore);
    WN_kid0(wn) = 
      WN_CreateExp3(MTYPE_is_size_double(WN_desc(istore)) ?
		    OPC_F8SELECT : OPC_F4SELECT,
		    test, t1, t2);
    return wn;		       
  }

  //bug 13853: if_conv for one stmt in then that is scalar store
  if (VHO_Enable_Simple_If_Conv &&
      WN_first(WN_then(wn)) && !WN_next(WN_first(WN_then(wn))) &&
      !WN_first(WN_else(wn)) &&
      WN_operator(WN_first(WN_then(wn))) == OPR_STID &&
      WN_operator(WN_kid0(WN_first(WN_then(wn)))) != OPR_CONST && //bug 14163
      MTYPE_is_float(WN_desc(WN_first(WN_then(wn)))) &&
      OPCODE_is_compare(WN_opcode(test)) &&
#ifndef KEY // bug 14738
      MTYPE_is_float(WN_desc(test)) && 
#endif
      MTYPE_is_integral(WN_rtype(test)) &&
#ifndef KEY // bug 14738
      WN_desc(WN_first(WN_then(wn))) == WN_desc(test)                    &&
#endif
      Opcodes_Can_Be_Speculated_Vectorized(WN_kid0(WN_first(WN_then(wn)))) &&
      (Ifconv_Overhead(WN_kid0(WN_first(WN_then(wn))))*
       MTYPE_byte_size(WN_desc(test))/16) < VHO_Enable_If_Conv_Limit) {
      WN* stid = WN_first(WN_then(wn));
      WN* t1 = WN_COPY_Tree(WN_kid0(stid));
      WN* t2 = WN_CreateLdid(OPR_LDID, WN_desc(stid), WN_desc(stid),
                            WN_store_offset(stid),
                            WN_st(stid),
                            WN_ty(stid),
                            WN_field_id(stid));
    wn = WN_COPY_Tree(stid);
    WN_kid0(wn) =
      WN_CreateExp3(MTYPE_is_size_double(WN_desc(stid)) ?
                    OPC_F8SELECT : OPC_F4SELECT,
                    test, t1, t2);
    return wn;
  }

  // If-Convert:
  //   if <compare>
  //     a[i] = <expr1>
  //   else
  //     a[i] = <expr2>
  //   endif
  // to:
  //   a[i] = <compare> ? <expr1> : <expr2>
  // -- bug 2724, 2722
  if (VHO_Enable_Simple_If_Conv &&
      WN_first(WN_then(wn)) && !WN_next(WN_first(WN_then(wn))) &&
      WN_first(WN_else(wn)) && !WN_next(WN_first(WN_else(wn))) &&
      WN_operator(WN_first(WN_then(wn))) == OPR_ISTORE &&
      WN_operator(WN_kid1(WN_first(WN_then(wn)))) == OPR_ARRAY &&
      MTYPE_is_float(WN_desc(WN_first(WN_then(wn)))) &&
      WN_operator(WN_first(WN_else(wn))) == OPR_ISTORE &&
      WN_operator(WN_kid1(WN_first(WN_else(wn)))) == OPR_ARRAY &&
      MTYPE_is_float(WN_desc(WN_first(WN_else(wn)))) &&
      OPCODE_is_compare(WN_opcode(test)) &&
#ifndef KEY // bug 14738
      MTYPE_is_float(WN_desc(test)) && 
#endif
      MTYPE_is_integral(WN_rtype(test)) &&
#ifndef KEY // bug 14738
      WN_desc(WN_first(WN_then(wn))) == WN_desc(test) &&
#endif
      Opcodes_Can_Be_Speculated_Vectorized(WN_kid0(WN_first(WN_then(wn)))) &&
      (Ifconv_Overhead(WN_kid0(WN_first(WN_then(wn))))*
       MTYPE_byte_size(WN_desc(test))/16) < VHO_Enable_If_Conv_Limit &&
#ifndef KEY // bug 14738
      WN_desc(WN_first(WN_else(wn))) == WN_desc(test) &&
#endif
      Opcodes_Can_Be_Speculated_Vectorized(WN_kid0(WN_first(WN_else(wn)))) &&
      (Ifconv_Overhead(WN_kid0(WN_first(WN_else(wn))))*
       MTYPE_byte_size(WN_desc(test))/16) < VHO_Enable_If_Conv_Limit &&
      WN_Simp_Compare_Trees(WN_kid1(WN_first(WN_else(wn))), 
			    WN_kid1(WN_first(WN_then(wn)))) == 0) {
    WN* istore = WN_first(WN_then(wn));
    WN* t1 = WN_COPY_Tree(WN_kid0(istore));
    WN* t2 = WN_COPY_Tree(WN_kid0(WN_first(WN_else(wn))));
    wn = WN_COPY_Tree(istore);
    WN_kid0(wn) = 
      WN_CreateExp3(MTYPE_is_size_double(WN_desc(istore)) ?
		    OPC_F8SELECT : OPC_F4SELECT,
		    test, t1, t2);
    return wn;		       
  }
#endif
  ++vho_if_nest_level;

  then_block = vho_lower_block ( WN_then(wn) );
  else_block = vho_lower_block ( WN_else(wn) );

  test_block = WN_CreateBlock ();
  WN_Set_Linenum ( test_block, VHO_Srcpos );

  vho_initialize_bool_info (&bool_info);

  LABEL_Init (New_LABEL (CURRENT_SYMTAB, bool_info.true_label),
	0, LKIND_DEFAULT);
  LABEL_Init (New_LABEL (CURRENT_SYMTAB, bool_info.false_label),
	0, LKIND_DEFAULT);

  if ( WN_operator(WN_if_test(wn)) == OPR_RCOMMA ) {

    rcomma_block   = vho_lower_block ( WN_kid1(WN_if_test(wn)) );
    WN_if_test(wn) = vho_lower_expr ( WN_kid0(WN_if_test(wn)),
                                      test_block, &bool_info );

    WN *rcomma_block_copy = WN_COPY_Tree ( rcomma_block );
    if (Cur_PU_Feedback)
      Cur_PU_Feedback->FB_clone_loop_test( rcomma_block, rcomma_block_copy,
					   wn );

    WN_INSERT_BlockFirst ( then_block, rcomma_block_copy );
    WN_INSERT_BlockFirst ( else_block, rcomma_block );
  }

  else
    WN_if_test(wn) = vho_lower_expr ( WN_if_test(wn), test_block, &bool_info );

  if ( WN_first(test_block) )
    WN_INSERT_BlockLast ( block, test_block );

  if (    bool_info.used_true_label
       || bool_info.used_false_label ) {

    // A subset of the following lines is produced:
    // GOTO  <false_label>
    // LABEL <true_label>
    // <then_clause>
    // GOTO  <join_label>
    // LABEL <false_label>
    // <else_clause>
    // LABEL <join_label>

    if ( WN_first(else_block) ) {

      LABEL_Init (New_LABEL (CURRENT_SYMTAB, join_label),
	0, LKIND_DEFAULT);
      emit_join_label = TRUE;
    }

    else {

      join_label = bool_info.false_label;
      emit_join_label = ! bool_info.used_false_label;
    }

    WN *wn_branch = NULL;

    if ( bool_info.used_true_label ) {

      if ( ! bool_info.used_false_label ) {

        WN *wn_goto = WN_CreateGoto ( (ST_IDX) NULL, bool_info.false_label );
        WN_Set_Linenum ( wn_goto, VHO_Srcpos );
        WN_INSERT_BlockLast ( block, wn_goto );
      }

      WN *wn_label
	= WN_CreateLabel ( (ST_IDX) 0, bool_info.true_label, 0, NULL );
      WN_Set_Linenum ( wn_label, VHO_Srcpos );
      WN_INSERT_BlockLast ( block, wn_label );
    }
      
    WN_INSERT_BlockLast ( block, then_block );

    if ( WN_first(else_block) ) {

      bool_info.used_false_label = TRUE;
      WN *wn_goto = WN_CreateGoto ( (ST_IDX) NULL, join_label );
      WN_Set_Linenum ( wn_goto, VHO_Srcpos );
      WN_INSERT_BlockLast ( block, wn_goto );
    }

    if ( bool_info.used_false_label ) {

      WN *wn_label
	= WN_CreateLabel ( (ST_IDX) 0, bool_info.false_label, 0, NULL );
      WN_Set_Linenum ( wn_label, VHO_Srcpos );
      WN_INSERT_BlockLast ( block, wn_label );
    }
      
    WN_INSERT_BlockLast ( block, else_block );

    if ( emit_join_label ) {

      WN *wn_label = WN_CreateLabel ( (ST_IDX) 0, join_label, 0, NULL );
      WN_Set_Linenum ( wn_label, VHO_Srcpos );
      WN_INSERT_BlockLast ( block, wn_label );
    }

    if ( Cur_PU_Feedback ) {
      Cur_PU_Feedback->FB_lower_branch( wn, wn_branch );
    }

    wn = NULL;
  }

  else {

    WN_then(wn) = then_block;
    WN_else(wn) = else_block;
  }

  --vho_if_nest_level;

  if ( vho_if_nest_level == 0 ) {

  }

  return wn;
} /* vho_lower_if */

#ifdef KEY // bug 8581
// determine if two WHIRL statements are identical; only handle store statements
// for now
static BOOL
Identical_stmt(WN *stmt1, WN *stmt2)
{
  if (stmt1 == NULL || stmt2 == NULL)
    return FALSE;
  if (WN_opcode(stmt1) != WN_opcode(stmt2))
    return FALSE;
  switch(WN_operator(stmt1)) {
  case OPR_STID:
    if (WN_store_offset(stmt1) != WN_store_offset(stmt2))
      return FALSE;
    if (WN_field_id(stmt1) != WN_field_id(stmt2))
      return FALSE;
    if (WN_desc(stmt1) != WN_desc(stmt2))
      return FALSE;
    if (WN_st_idx(stmt1) != WN_st_idx(stmt2))
      return FALSE;
    return WN_Simp_Compare_Trees(WN_kid0(stmt1), WN_kid0(stmt2)) == 0;
  case OPR_STBITS:
    if (WN_store_offset(stmt1) != WN_store_offset(stmt2))
      return FALSE;
    if (WN_bit_offset(stmt1) != WN_bit_offset(stmt2))
      return FALSE;
    if (WN_bit_size(stmt1) != WN_bit_size(stmt2))
      return FALSE;
    if (WN_st_idx(stmt1) != WN_st_idx(stmt2))
      return FALSE;
    return WN_Simp_Compare_Trees(WN_kid0(stmt1), WN_kid0(stmt2)) == 0;
  case OPR_ISTORE:
    if (WN_store_offset(stmt1) != WN_store_offset(stmt2))
      return FALSE;
    if (WN_field_id(stmt1) != WN_field_id(stmt2))
      return FALSE;
    if (WN_desc(stmt1) != WN_desc(stmt2))
      return FALSE;
    if (WN_Simp_Compare_Trees(WN_kid0(stmt1), WN_kid0(stmt2)) != 0)
      return FALSE;
    return WN_Simp_Compare_Trees(WN_kid1(stmt1), WN_kid1(stmt2)) == 0;
  case OPR_ISTBITS:
    if (WN_store_offset(stmt1) != WN_store_offset(stmt2))
      return FALSE;
    if (WN_bit_offset(stmt1) != WN_bit_offset(stmt2))
      return FALSE;
    if (WN_bit_size(stmt1) != WN_bit_size(stmt2))
      return FALSE;
    if (WN_Simp_Compare_Trees(WN_kid0(stmt1), WN_kid0(stmt2)) != 0)
      return FALSE;
    return WN_Simp_Compare_Trees(WN_kid1(stmt1), WN_kid1(stmt2)) == 0;
  case OPR_MSTORE:
    if (WN_store_offset(stmt1) != WN_store_offset(stmt2))
      return FALSE;
    if (WN_field_id(stmt1) != WN_field_id(stmt2))
      return FALSE;
    if (WN_Simp_Compare_Trees(WN_kid0(stmt1), WN_kid0(stmt2)) != 0)
      return FALSE;
    if (WN_Simp_Compare_Trees(WN_kid1(stmt1), WN_kid1(stmt2)) != 0)
      return FALSE;
    return WN_Simp_Compare_Trees(WN_kid2(stmt1), WN_kid2(stmt2)) == 0;
  case OPR_BLOCK: {
      WN *s1 = WN_first(stmt1);
      WN *s2 = WN_first(stmt2);
      for ( ; ; ) {
	if (s1 == NULL && s2 == NULL)
	  return TRUE;
	if (! Identical_stmt(s1, s2))
	  return FALSE;
	s1 = WN_next(s1);
	s2 = WN_next(s2);
      }
    }
  default: ;
  }
  return FALSE;
}

// If the given IF has a nested IF in the ELSE part with identical THEN stmt:
//
//   IF (A) THEN S1; ELSE { if (B) THEN S1; ELSE S2; }
//
// transform it to:
//
//   IF (A || B) THEN S1; ELSE S2;
//
// S1 can be the empty statement.
//
// Since this is done bottom up, multiple occurrences of the identical THEN 
// stmts can be commonized.  If tranformed, the new IF statement is returned;
// otherwise, the original IF.
static WN *
Merge_identical_then_parts(WN *wn)
{
  WN *then_wn = WN_then(wn);
  WN *else_wn = WN_else(wn);
  if (WN_opcode(then_wn) != OPC_BLOCK)
    return wn;
  if (WN_opcode(else_wn) != OPC_BLOCK)
    return wn;
  if (WN_first(else_wn) == NULL)
    return wn;
  if (WN_first(else_wn) != WN_last(else_wn)) // else part more than 1 stmt
    return wn;
  if (WN_opcode(WN_first(else_wn)) != OPC_IF) // that 1 stmt is not an IF
    return wn;
  WN *elseifwn = WN_first(else_wn);
  WN *then_elseifwn = WN_then(elseifwn);
  if (! Identical_stmt(then_wn, then_elseifwn))
    return wn;
  WN *if_test1 = WN_if_test(wn);
  WN *if_test2 = WN_if_test(elseifwn);
  if (OPT_Lower_Speculate && WN_Expr_Can_Be_Speculative(if_test2, NULL))
    WN_if_test(wn) = WN_LIOR(if_test1, if_test2);
  else WN_if_test(wn) = WN_CIOR(if_test1, if_test2);
  WN_else(wn) = WN_else(elseifwn);
  // update frequencies
  if (Cur_PU_Feedback) {
    const FB_Info_Branch& info_branch1 = Cur_PU_Feedback->Query_branch(wn);
    const FB_Info_Branch& info_branch2 = Cur_PU_Feedback->Query_branch(elseifwn);
    Cur_PU_Feedback->Annot_branch( wn, FB_Info_Branch(
    		      info_branch1.freq_taken+info_branch2.freq_taken, 
		      info_branch2.freq_not_taken, OPR_IF ) );
    if (WN_operator(WN_if_test(wn)) == OPR_CIOR) {
      FB_FREQ freq_left = info_branch1.freq_taken;
      FB_FREQ freq_right = info_branch2.freq_taken;
      FB_FREQ freq_neither = info_branch2.freq_not_taken;
      Cur_PU_Feedback->Annot_circuit(WN_if_test(wn), FB_Info_Circuit(freq_left,
      						freq_right, freq_neither));
    }
  }
  // clean up
  WN_if_test(elseifwn) = NULL;
  WN_else(elseifwn) = NULL;
  WN_DELETE_Tree(else_wn);
  return wn;
}

// If the given IF has a nested IF in the THEN part with identical ELSE stmt:
//
//   IF (A) THEN { if (B) THEN S1; ELSE S2; } ELSE S2;
//
// transform it to:
//
//   IF (A && B) THEN S1; ELSE S2;
//
// S2 can be the empty statement.
//
// Since this is done bottom up, multiple occurrences of the identical ELSE 
// stmts can be commonized.  If tranformed, the new IF statement is returned;
// otherwise, the original IF.
static WN *
Merge_identical_else_parts(WN *wn)
{
  WN *then_wn = WN_then(wn);
  WN *else_wn = WN_else(wn);
  if (WN_opcode(then_wn) != OPC_BLOCK)
    return wn;
  if (WN_opcode(else_wn) != OPC_BLOCK)
    return wn;
  if (WN_first(then_wn) == NULL)
    return wn;
  if (WN_first(then_wn) != WN_last(then_wn)) // then part more than 1 stmt
    return wn;
  if (WN_opcode(WN_first(then_wn)) != OPC_IF) // that 1 stmt is not an IF
    return wn;
  WN *thenifwn = WN_first(then_wn);
  WN *else_thenifwn = WN_else(thenifwn);
  if (! Identical_stmt(else_wn, else_thenifwn))
    return wn;
  WN *if_test1 = WN_if_test(wn);
  WN *if_test2 = WN_if_test(thenifwn);
  if (OPT_Lower_Speculate && WN_Expr_Can_Be_Speculative(if_test2, NULL))
    WN_if_test(wn) = WN_LAND(if_test1, if_test2);
  else WN_if_test(wn) = WN_CAND(if_test1, if_test2);
  WN_then(wn) = WN_then(thenifwn);
  // update frequencies
  if (Cur_PU_Feedback) {
    const FB_Info_Branch& info_branch1 = Cur_PU_Feedback->Query_branch(wn);
    const FB_Info_Branch& info_branch2 = Cur_PU_Feedback->Query_branch(thenifwn);
    Cur_PU_Feedback->Annot_branch( wn, FB_Info_Branch(info_branch2.freq_taken, 
		      info_branch1.freq_not_taken+info_branch2.freq_not_taken, 
		      OPR_IF ) );
    if (WN_operator(WN_if_test(wn)) == OPR_CAND) {
      FB_FREQ freq_left = info_branch1.freq_not_taken;
      FB_FREQ freq_right = info_branch2.freq_taken;
      FB_FREQ freq_neither = info_branch2.freq_not_taken;
      Cur_PU_Feedback->Annot_circuit(WN_if_test(wn), FB_Info_Circuit(freq_left,
      						freq_right, freq_neither));
    }
  }
  // clean up
  WN_if_test(thenifwn) = NULL;
  WN_then(thenifwn) = NULL;
  WN_DELETE_Tree(then_wn);
  return wn;
}
#endif

static WN *
vho_lower_block ( WN * old_block )
{
  WN * block;
  WN * wn;
  WN * next_wn;

  if ( old_block == NULL )
    return old_block;

  block = WN_CreateBlock ();
  WN_Set_Linenum ( block, VHO_Srcpos );

  for ( wn = WN_first(old_block);
        wn;
        wn = next_wn ) {

    next_wn = WN_next(wn);
    wn = vho_lower ( wn, block );

    if ( wn )
      WN_INSERT_BlockLast ( block, wn );
  }

  WN_Delete ( old_block );

  return block;
} /* vho_lower_block */


static WN *
vho_lower_scf ( WN * wn, WN * block )
{
  PREG_NUM last_preg;

  last_preg = PREG_Table_Size (CURRENT_SYMTAB);

  VHO_Srcpos = WN_Get_Linenum(wn);

  switch ( WN_opcode(wn) ) {

    case OPC_DO_WHILE:

      wn = vho_lower_do_while ( wn, block );
      break;

    case OPC_WHILE_DO:

      wn = vho_lower_while_do ( wn, block );
      break;

    case OPC_DO_LOOP:

      wn = vho_lower_do_loop ( wn, block );
      break;

    case OPC_IF:

      wn = vho_lower_if ( wn, block );
#if defined( KEY) && !defined(TARG_ST) // bug 8581
      if (wn != NULL && WN_opcode(wn) == OPC_IF) {
	if (VHO_Merge_Thens)
	  wn = Merge_identical_then_parts(wn);
	if (VHO_Merge_Elses)
	  wn = Merge_identical_else_parts(wn);
      }
#endif
      break;

    case OPC_BLOCK:

      wn = vho_lower_block ( wn );
      break;

    case OPC_REGION:

#if defined( KEY) && !defined(TARG_ST)
      wn = vho_lower_region ( wn, block );
#else
      wn = vho_lower_region ( wn );
#endif
      break;
  }

  if ( VHO_Recycle_Pregs )
    Reset_PREG_Table_Size (CURRENT_SYMTAB, last_preg);

  return wn;
} /* vho_lower_scf */


static WN *
vho_lower_entry ( WN * wn )
{
  PREG_NUM last_preg;

  last_preg = PREG_Table_Size (CURRENT_SYMTAB);

  VHO_Srcpos = WN_Get_Linenum(wn);
#if defined( KEY) && !defined(TARG_ST)
  current_pu_id ++;
#endif

  /* See if we need to lower the pu */
  if (    PU_has_very_high_whirl (Get_Current_PU ()) == FALSE
       && VHO_Force_Lowering == FALSE ) {

    return wn;
  }

  WN_func_body(wn) = vho_lower_block ( WN_func_body(wn) );

  if ( VHO_Recycle_Pregs )
    Reset_PREG_Table_Size ( CURRENT_SYMTAB, last_preg );

  return ( wn );
} /* VHO_lower_entry */


static WN *
vho_lower ( WN * wn, WN * block )
{
  if ( WN_opcode(wn) == OPC_FUNC_ENTRY )
    wn = vho_lower_entry ( wn );

  else
  if ( OPCODE_is_scf(WN_opcode(wn)) )
    wn = vho_lower_scf ( wn, block );

  else
  if ( OPCODE_is_stmt(WN_opcode(wn)) )
    wn = vho_lower_stmt ( wn, block );

  else
  if ( OPCODE_is_expression(WN_opcode(wn)) )
    wn = vho_lower_expr ( wn, block, NULL );

  return wn;
} /* vho_lower */


WN *
VHO_Lower ( WN * wn )
{
  wn = vho_lower ( wn, NULL );

  return wn;
} /* VHO_Lower */


#ifndef FRONT_END

/* ============================================================================
 *
 * WN *VHO_Lower_Driver (PU_Info* pu_info, WN * wn )
 *
 * Main driver for VH->H lowering. Currently, it calls the 
 * F90 lowerer (if necessary) and the more generic routine VHO_Lower.
 *
 * ============================================================================
 */

WN * VHO_Lower_Driver (PU_Info* pu_info, 
		       WN *wn) {

   if (Get_Trace ( TKIND_IR, TP_VHO_LOWER )) {

     if ( Get_Trace ( TP_VHO_LOWER, 1 ) == 0 ) {

       fprintf(TFile,"\n\n========== Dump before VHO Lowering ==========\n");
       fdump_tree(TFile,wn);
     }
   }
  
#ifdef TARG_ST
   extern INT32   Scalar_Struct_Limit;
   if (Scalar_Struct_Limit >= 0) 
     VHO_Struct_Limit = Scalar_Struct_Limit;
#endif

   // Only run F90 lowering if this is not the link phase of an IPA compilation
   if (Ipa_Ident_Number == 0) {
      WB_F90_Lower_Initialize(wn, Prompf_Id_Map);
      wn = F90_Lower(pu_info, wn);
      WB_F90_Lower_Terminate();
   }
   wn = VHO_Lower(wn);

   if (Get_Trace ( TKIND_IR, TP_VHO_LOWER )) {

     if ( Get_Trace ( TP_VHO_LOWER, 2 ) == 0 ) {

       fprintf(TFile,"\n\n========== Dump after VHO Lowering ==========\n");
       fdump_tree(TFile,wn);
     }
   }

   if (Get_Trace(TKIND_SYMTAB, TP_VHO_LOWER)) {

     fprintf(TFile,
	     "\n\n========== Symbol tables after VHO Lowering ==========\n");
     // TODO_NEW_SYMTAB: figure out how to trace the symbol table
     // Here is the old symtab code:
     // Trace_SYMTAB (TFile, Global_Symtab, TRUE);
     // Trace_SYMTAB (TFile, Current_Symtab, TRUE);
   }

   if (Inline_Intrinsics_Early) {
      /* Lower intrinsics and reduce tree height if running at high
       * optimization levels */
      wn = WN_Lower(wn, LOWER_TREEHEIGHT | LOWER_INLINE_INTRINSIC, NULL,
		    "Intrinsic lowering");
   }
#if defined( KEY) && !defined(TARG_ST) // bug 6938
   else wn = WN_Lower(wn, LOWER_FAST_EXP, NULL,
		    "Fast exponents lowering");
#endif

   return (wn);
}
 
#endif /* FRONT_END */

#pragma reset woff 1172