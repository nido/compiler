/*

  Copyright (C) 2000 Silicon Graphics, Inc.  All Rights Reserved.

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


/* EXP routines for loads and stores */

#include <elf.h>
#include <vector.h>
#include "defs.h"
#include "glob.h"
#include "em_elf.h"
#include "erglob.h"
#include "ercg.h"
#include "tracing.h"
#include "config.h"
#include "config_TARG.h"
#include "config_debug.h"
#include "xstats.h"
#include "topcode.h"
#include "tn.h"
#include "op.h"
#include "targ_isa_lits.h"
#include "cg.h"
#include "cg_flags.h"
#include "cgexp.h"
#include "stblock.h"
#include "sections.h"
#include "data_layout.h"
#include "strtab.h"
#include "symtab.h"

/* ====================================================================
 *   Expand_Lda
 * ====================================================================
 */
void
Expand_Lda (TN *dest, TN *src, OPS *ops)
{
  FmtAssert(FALSE, ("NYI: Expand_Lda"));
}

/* ====================================================================
 *   Gen_Offset_TN
 *
 *   Depending on the object's section and size, make offset TN
 * ====================================================================
 */
static TN*
Gen_Offset_TN (
  ST *sym,
  INT64 ofst,
  TYPE_ID mtype,
  ST *base_sym,
  OPS *ops
)
{
  UINT16     tn_reloc = TN_RELOC_NONE;
  UINT8        reloc_size = MTYPE_byte_size(mtype);
  SECTION_IDX  sec = STB_section_idx (base_sym);
  TN          *ofst_tn;

  FmtAssert(FALSE,("Gen_Offset_TN: not implemented"));

  return ofst_tn;
}

/* ====================================================================
 *   Pick_Load_Imm_Instruction
 * ====================================================================
 */
static TOP
Pick_Load_Imm_Instruction (
  TYPE_ID rtype, 
  TYPE_ID desc
)
{
  TOP top = TOP_UNDEFINED;

  switch (desc) {
  case MTYPE_I1: top = TOP_ldb_i; break;
  case MTYPE_U1: top = TOP_ldbu_i; break;
  case MTYPE_I2: top = TOP_ldh_i; break;
  case MTYPE_U2: top = TOP_ldhu_i; break;
  case MTYPE_I4:
  case MTYPE_U4: 
  case MTYPE_A4: top = TOP_ldw_i; break;
  case MTYPE_V:
    if (rtype != MTYPE_V) {
      // use rtype to pick load (e.g. if lda)
      top = Pick_Load_Imm_Instruction(rtype, rtype);
    }
  }

  if (top == TOP_UNDEFINED) {
    FmtAssert(0,("Gen_Load_Imm_Instruction: mtype"));
  }

  return top;
}

/* ====================================================================
 *   Expand_Load
 * ====================================================================
 */
void
Expand_Load (
  OPCODE opcode,
  TN    *result, 
  TN    *base, 
  TN    *ofst, 
  OPS   *ops
)
{
  TYPE_ID  mtype = OPCODE_desc(opcode);
  TOP      top = TOP_UNDEFINED;
  INT64    val;
  TN      *tmp;

  Is_True (TN_is_constant(ofst), ("Illegal load offset TN"));

  if (TN_has_value(ofst)) {
    val = TN_value(ofst);
    if (ISA_LC_Value_In_Class(val, LC_s32)) {
      top = Pick_Load_Imm_Instruction (OPCODE_rtype(opcode), mtype);
    } else {
      FmtAssert(FALSE,("Expand_Load: offset larger than 32 bits"));
    }
  } 
  else if (TN_is_symbol(ofst)) {    
    //  
    // TN must be a symbol: SP/FP or GP relative. 
    //
    INT64 base_ofst;
    ST *base_sym;

    // get the base_sym: GP/FP/SP and base_ofst from it.
    Base_Symbol_And_Offset_For_Addressing (TN_var(ofst), 
                                  TN_offset(ofst), &base_sym, &base_ofst);

    // Some symbols don't have their base fixed (formals on small
    // stack) but it's OK with the large offsets:
    //
    if (TN_is_gp_reg(base)) {
      FmtAssert(FALSE, ("Expand_Load: gp relative ??"));
    }
    else {
      //
      // SP/FP relative
      //
      top = Pick_Load_Imm_Instruction (OPCODE_rtype(opcode), mtype);
    } 
  }
  else {
    FmtAssert(FALSE,("unexpected constant in Expand_Load"));
  }

  FmtAssert(top != TOP_UNDEFINED,("Expand_Load: TOP_UNDEFINED"));


  Build_OP (top, result, ofst, base, ops);
  return;
}

/* ====================================================================
 *   Pick_Store_Imm_Instruction
 * ====================================================================
 */
static TOP
Pick_Store_Imm_Instruction (
  TYPE_ID  mtype
)
{
  TOP top = TOP_UNDEFINED;

  switch (mtype) {
  case MTYPE_I1:
  case MTYPE_U1: top = TOP_stb_i; break;
  case MTYPE_I2:
  case MTYPE_U2: top = TOP_sth_i; break;
  case MTYPE_I4:
  case MTYPE_U4:
  case MTYPE_A4: top = TOP_stw_i; break;

  default:
    FmtAssert(FALSE, ("Gen_Store_Imm_Instruction mtype"));
  }

  return top;
}

/* ====================================================================
 *   Expand_Store
 * ====================================================================
 */
void
Expand_Store (
  TYPE_ID mtype,
  TN     *src, 
  TN     *base, 
  TN     *ofst, 
  OPS    *ops
)
{
  TOP   top = TOP_UNDEFINED;
  TN   *tmp;

  Is_True (TN_is_constant(ofst), ("Illegal store offset TN"));

  if (TN_has_value(ofst)) {
    // TN has immediate value:
    INT64 val = TN_value(ofst);
    if (ISA_LC_Value_In_Class(val, LC_s32)) {
      top = Pick_Store_Imm_Instruction (mtype);
    }
    else {
      FmtAssert(FALSE,("Expand_Store: offset larger than 32-bits"));
    }
  }
  else if (TN_is_symbol(ofst)) { 
    //
    // TN must be a symbol: SP/FP or GP relative. 
    //
    INT64 base_ofst;
    ST *base_sym;

    // get the base_sym: GP/FP/SP and base_ofst from it.
    Base_Symbol_And_Offset_For_Addressing (TN_var(ofst), 
                                 TN_offset(ofst), &base_sym, &base_ofst);

    if (TN_is_gp_reg(base)) {
      FmtAssert(FALSE, ("Expand_Store: gp relative ??"));
    }
    else {
      top = Pick_Store_Imm_Instruction (mtype);
    }
  }
  else {
    FmtAssert(FALSE,("unexpected constant in Expand_Store"));
  }

  FmtAssert(top != TOP_UNDEFINED,("Expand_Store: TOP_UNDEFINED"));

  Build_OP (top, ofst, base, src, ops);
  return;
}

/* ====================================================================
 *   INT32 get_variant_alignment(TYPE_ID rtype, VARIANT variant)
 *
 *   Given a variant compute a valid alignment return gcd of m,n;
 *   Used for alignment reasons;
 * ==================================================================== */
static INT32 
get_variant_alignment (
  TYPE_ID rtype, 
  VARIANT variant
)
{
  INT32 r;
  INT32 n= V_alignment(variant);
  INT32 m= MTYPE_alignment(rtype);

  FmtAssert(FALSE,("Not Implemented"));

  while(r = m % n)
  {
    m=  n;
    n=  r;
  }
  return n;
}

/* ====================================================================
 *   Composed_Align_Type
 * ====================================================================
 */
static TYPE_ID 
Composed_Align_Type (
  TYPE_ID mtype, 
  VARIANT variant, 
  INT32 *alignment, 
  INT32 *partials
)
{
  FmtAssert(FALSE,("Not Implemented"));

  *alignment =	get_variant_alignment(mtype, variant);
  *partials =	MTYPE_alignment(mtype) / *alignment;
  return Mtype_AlignmentClass( *alignment, MTYPE_CLASS_UNSIGNED_INTEGER);
}

/* ====================================================================
 *   OPCODE_make_signed_op
 * ====================================================================
 */
static OPCODE 
OPCODE_make_signed_op (
  OPERATOR op, 
  TYPE_ID rtype, 
  TYPE_ID desc, 
  BOOL is_signed
)
{
  if (MTYPE_is_signed(rtype) != is_signed)
	rtype = MTYPE_complement(rtype);
  if (MTYPE_is_signed(desc) != is_signed)
	desc =	MTYPE_complement(desc);

  return OPCODE_make_op(op, rtype, desc);
}

/* ====================================================================
 *   Adjust_Addr_TNs
 *
 *   We have a memory reference operation, with a base and displacement,
 *   where the displacement is literal.  We want to create another memop
 *   with the displacement modified by a small amount.
 *
 *   WARNING:  If an add operation is required, it will be expanded here.
 * ====================================================================
 */
static void
Adjust_Addr_TNs (
  TOP	opcode,		/* The new memory operation */
  TN	**base_tn,	/* The base address -- may be modified */
  TN	**disp_tn,	/* The displacement -- may be modified */
  INT16	disp,		/* A displacement to add */
  OPS *ops)
{
  FmtAssert(FALSE,("Not Implemented"));

  if (Potential_Immediate_TN_Expr (opcode, *disp_tn, disp)) {
    if ( TN_has_value(*disp_tn) ) {
      *disp_tn = Gen_Literal_TN ( TN_value(*disp_tn) + disp, 4 );
    } else {
      *disp_tn = Gen_Symbol_TN ( TN_var(*disp_tn),
				 TN_offset(*disp_tn) + disp, 0);
    }
  } else {
    TN *tmp = Build_TN_Of_Mtype (Pointer_Mtype);
    // because disp may be symbolic reloc on base,
    // want to still add it with base and create new base and disp.
    Expand_Add (tmp, *disp_tn, *base_tn, Pointer_Mtype, ops);
    *base_tn = tmp;
    *disp_tn = Gen_Literal_TN (disp, 4);
  }
}

/* ====================================================================
 *   Expand_Composed_Load
 * ====================================================================
 */
static void
Expand_Composed_Load ( 
  OPCODE op, 
  TN *result, 
  TN *base, 
  TN *disp, 
  VARIANT variant, 
  OPS *ops
)
{
  TYPE_ID rtype= OPCODE_rtype(op);
  TYPE_ID desc = OPCODE_desc(op);

  TOP		top;
  INT32		alignment, nLoads, i;
  OPCODE	new_opcode;
  TYPE_ID	new_desc;
  TN		*tmpV[8];

  FmtAssert(FALSE,("Not Implemented"));

  new_desc = Composed_Align_Type(desc, variant, &alignment, &nLoads);
  new_opcode = OPCODE_make_signed_op(OPR_LDID, rtype, new_desc, FALSE);
  //  top = Pick_Load_Instruction (rtype, new_desc);

  Is_True(nLoads > 1, ("Expand_Composed_Load with nLoads == %d", nLoads));

 /* Generate the component loads, storing the result in a vector
  * of TNs. The vector is filled in such a way that the LSB is in
  * tmpV[0] so that later code can ignore the endianess of the target.
  */
  INT endian_xor = (Target_Byte_Sex == BIG_ENDIAN) ? (nLoads-1) : 0;
  for (i=0; i < nLoads; i++) {
    INT idx = i ^ endian_xor;
    tmpV[idx] = Build_TN_Of_Mtype(rtype);
    Expand_Load ( new_opcode, tmpV[idx], base, disp, ops);
    if (i < nLoads-1) Adjust_Addr_TNs (top, &base, &disp, alignment, ops);
  }

  /* Now combine the components into the desired value. The only
   * complication is that the form of the 'dep' instruction that we
   * need, supports a maximum length of 16 bits. Fortunately that
   * leaves just creating a 64-bit integer from two 32-bit pieces --
   * the mix4.r instruction handles that case.
   */
  INT nLoadBits = alignment * 8;
  if (nLoadBits <= 16) {
    TN *tmp0 = tmpV[0]; 
    for (i = 1; i < (nLoads-1); i++) {
      TN *tmp= Build_TN_Of_Mtype(rtype);
      Build_OP(TOP_noop, tmp, True_TN, tmpV[i], tmp0,
	       Gen_Literal_TN(i*nLoadBits, 4), 
                                      Gen_Literal_TN(nLoadBits, 4), ops);
      tmp0 = tmp;
    }
    Build_OP(TOP_noop, result, True_TN, tmpV[i], tmp0,
	     Gen_Literal_TN(i*nLoadBits, 4), 
                                      Gen_Literal_TN(nLoadBits, 4), ops);
  } else {
    FmtAssert(nLoadBits == 32 && nLoads == 2,
	      ("Expand_Composed_Load: unexpected composition"));
    Build_OP(TOP_noop, result, True_TN, tmpV[1], tmpV[0], ops);
  }
}

/* ====================================================================
 *   Expand_Misaligned_Load
 * ====================================================================
 */
void
Expand_Misaligned_Load ( 
  OPCODE op, 
  TN *result, 
  TN *base, 
  TN *disp, 
  VARIANT variant, 
  OPS *ops
)
{
  Expand_Composed_Load (op, result, base, disp, variant, ops);
}

/* ====================================================================
 *   Expand_Composed_Store
 * ====================================================================
 */
static void
Expand_Composed_Store (
  TYPE_ID mtype, 
  TN *obj, 
  TN *base, 
  TN *disp, 
  VARIANT variant, 
  OPS *ops
)
{
  TOP		top;
  INT32		alignment, nStores;
  TYPE_ID	new_desc;

  FmtAssert(FALSE,("Not Implemented"));

  new_desc =	Composed_Align_Type(mtype, variant, &alignment, &nStores);
  //  top = Pick_Store_Instruction (new_desc);

  if (Target_Byte_Sex == BIG_ENDIAN)
    Adjust_Addr_TNs (top, &base, &disp, 
                                   MTYPE_alignment(mtype)-alignment, ops);
  Expand_Store (new_desc, obj, base, disp, ops); 

  while(--nStores >0) {
    TN *tmp = Build_TN_Of_Mtype(mtype);
    Expand_Shift(tmp, obj, Gen_Literal_TN(alignment*8, 4), 
                                                mtype, shift_lright, ops);
    obj = tmp;

    if (Target_Byte_Sex == BIG_ENDIAN)
      Adjust_Addr_TNs (top, &base, &disp, -alignment, ops);
    else 
      Adjust_Addr_TNs (top, &base, &disp, alignment, ops);

    Expand_Store (new_desc, obj, base, disp, ops); 
  }
}

/* ====================================================================
 *   Expand_Misaligned_Store
 * ====================================================================
 */
void
Expand_Misaligned_Store (
  TYPE_ID mtype, 
  TN *obj_tn, 
  TN *base_tn, 
  TN *disp_tn, 
  VARIANT variant, 
  OPS *ops
)
{
  Expand_Composed_Store (mtype, obj_tn, base_tn, disp_tn, variant, ops);
}

/* ====================================================================
 *   Exp_Ldst
 * ====================================================================
 */
static void
Exp_Ldst (
  OPCODE opcode,
  TN *tn,
  ST *sym,
  INT64 ofst,
  BOOL indirect_call,
  BOOL is_store,
  BOOL is_load,
  OPS *ops,
  VARIANT variant
)
{
  ST *base_sym;
  INT64 base_ofst;
  TN *base_tn;
  TN *ofst_tn;
  BOOL is_lda = (!is_load && !is_store);
  OPS newops;
  OP *op;
  OPS_Init(&newops);

  if (Trace_Exp) {
    fprintf(TFile, "exp_ldst %s: ", OPCODE_name(opcode));
    if (tn) Print_TN(tn,FALSE);
    if (is_store) fprintf(TFile, " -> ");
    else fprintf(TFile, " <- ");
    if (ST_class(sym) == CLASS_CONST)
      fprintf ( TFile, "<constant: %s>\n", Targ_Print(NULL,STC_val(sym)));
    else
      fprintf(TFile, "%lld (%s)\n", ofst, ST_name(sym));
  }

  Allocate_Object(sym);         /* make sure sym is allocated */

  Base_Symbol_And_Offset_For_Addressing (sym, ofst, &base_sym, &base_ofst);

  if (Trace_Exp) {
    fprintf(TFile, "exp_ldst: ");
    fprintf(TFile, "base %s, ", ST_name(base_sym));
    fprintf(TFile, "ofst %lld \n", base_ofst);
  }

  if (ST_on_stack(sym)) {
    // formals on small stack are not assigned to sp/fp yet
    // cause base is not finished, but they will be assigned to SP.
    // So only use FP if already based on FP.
    base_tn = (base_sym == FP_Sym) ? FP_TN : SP_TN;

    if (sym == base_sym) {
      // can have direct reference to SP or FP,
      // e.g. if actual stored to stack.
      ofst_tn = Gen_Literal_TN (base_ofst, Pointer_Size);
    }
    else {
      /* Because we'd like to see symbol name in .s file, 
       * still reference the symbol rather than the sp/fp base.  
       * Do put in the offset from the symbol.  
       * We put the symbol in the TN and then
       * let cgemit replace symbol with the final offset.
       * We generate a SW reg, <sym>, <SP> rather than SW reg,<sym>
       * because cgemit and others expect a separate tn for the
       * offset and base. 
       */
      ofst_tn = Gen_Symbol_TN (sym, ofst, TN_RELOC_NONE);
    }
  }
  else if (Gen_GP_Relative &&
           (ST_class(base_sym) == CLASS_BLOCK || 
                                     ST_class(base_sym)==CLASS_VAR) &&
	   ST_gprel(base_sym)) {
    FmtAssert(FALSE,("Exp_Ldst: GP-relative"));
    // gp-relative reference
    PU_References_GP = TRUE;
    base_tn = GP_TN;
    // Use rtype for size of LDA
    ofst_tn = Gen_Offset_TN (sym, ofst, 
	            is_lda ? OPCODE_rtype(opcode) : OPCODE_desc(opcode), 
		    base_sym, 
		    &newops);

    if (Constant_GP) {
      // GP-relative where GP never changes
      // nada
      ;
    }
    else if (Guaranteed_Small_GOT) {
      // Generate GOT
      // nada
      ;
    }
  }
  else {
    // Not gp-relative reference

    FmtAssert(!ST_gprel(base_sym),
                 ("Exp_Ldst: %s is set gp-relarive", ST_name(base_sym)));

    // address TNs
    TN *tmp1 = Build_TN_Of_Mtype (Pointer_Mtype);

    if (ST_class(sym) == CLASS_CONST) {
      char *cname = Get_TCON_name (ST_tcon(sym));

      if (Trace_Exp) {
	fprintf(TFile,"exp_ldst: constant ST name %s\n", cname);
      }

      Build_OP (TOP_mov_i, tn, 
                         Gen_Symbol_TN (sym, 0, TN_RELOC_NONE), &newops);

      // want to stop at address (either that or add with 0)
      is_lda = FALSE;	// so nothing done
    }
    else {

      if (is_lda && base_ofst == 0) {
	// want to stop at address (either that or add with 0)
	tmp1 = tn;
	is_lda = FALSE;	// so nothing done
      }

      // because it is not GP-relative, just make the address
      Build_OP (TOP_mov_i, tmp1, 
                         Gen_Symbol_TN (sym, 0, TN_RELOC_NONE), &newops);

      // load is of address, not of result type
      base_tn = tmp1;

      // because it is not GP-relative, just use the address
      // load is of address, not of result type
      //      base_tn = Gen_Symbol_TN (sym, 0, TN_RELOC_NONE);

      // add offset to address
      ofst_tn = Gen_Literal_TN(ofst, Pointer_Size);
    }
  }

  if (is_store) {
    if (variant == V_NONE) {
      Expand_Store (OPCODE_desc(opcode), tn, base_tn, ofst_tn, &newops);
    }
    else {
      Expand_Misaligned_Store (OPCODE_desc(opcode), tn, 
			            base_tn, ofst_tn, variant, &newops);
    }
  }
  else if (is_load) {
    if (variant == V_NONE)
      Expand_Load (opcode, tn, base_tn, ofst_tn, &newops);
    else 
      Expand_Misaligned_Load (opcode, tn, 
			            base_tn, ofst_tn, variant, &newops);
  }
  else if (is_lda) {
    Expand_Add (tn, base_tn, ofst_tn, OPCODE_rtype(opcode), &newops);
  }

  FOR_ALL_OPS_OPs (&newops, op) {
    if (is_load && ST_is_constant(sym) && OP_load(op)) {
      // If we expanded a load of a constant, 
      // nothing else can alias with the loads 
      // we have generated.
      Set_OP_no_alias(op);
    }
    if (Trace_Exp) {
      fprintf(TFile, "exp_ldst into "); Print_OP (op);
    }
  }

  /* Add the new OPs to the end of the list passed in */
  OPS_Append_Ops(ops, &newops);

  return;
}

/* ====================================================================
 *   Exp_Lda
 * ====================================================================
 */
void Exp_Lda ( 
  TYPE_ID mtype, 
  TN *tgt_tn, 
  ST *sym, 
  INT64 ofst, 
  OPERATOR call_opr,
  OPS *ops
)
{
  OPCODE opcode = OPCODE_make_op(OPR_LDA, mtype, MTYPE_V);
  Exp_Ldst (opcode, tgt_tn, sym, ofst, (call_opr == OPR_ICALL),
	                                     FALSE, FALSE, ops, V_NONE);
  return;
}

/* ====================================================================
 *   Exp_Load2
 * ====================================================================
 */
void
Exp_Load (
  TYPE_ID rtype, 
  TYPE_ID desc, 
  TN *tgt_tn, 
  ST *sym, 
  INT64 ofst, 
  OPS *ops, 
  VARIANT variant
)
{
  OPCODE opcode = OPCODE_make_op (OPR_LDID, rtype, desc);
  Exp_Ldst (opcode, tgt_tn, sym, ofst, FALSE, FALSE, TRUE, ops, variant);
  return;
}

/* ====================================================================
 *   Exp_Store
 * ====================================================================
 */
void
Exp_Store (
  TYPE_ID mtype, 
  TN *src_tn, 
  ST *sym, 
  INT64 ofst, 
  OPS *ops, 
  VARIANT variant
)
{
  OPCODE opcode = OPCODE_make_op(OPR_STID, MTYPE_V, mtype);
  Exp_Ldst (opcode, src_tn, sym, ofst, FALSE, TRUE, FALSE, ops, variant);
}

/* ====================================================================
 *   Exp_Prefetch
 * ====================================================================
 */
void Exp_Prefetch (
  TOP opc, 
  TN* src1, 
  TN* src2, 
  VARIANT variant, 
  OPS* ops
)
{
  FmtAssert(FALSE,("Not Implemented"));

  return;
}

/* ======================================================================
 *   Exp_Extract_Bits
 * ======================================================================*/
void
Exp_Extract_Bits (
  TYPE_ID rtype, 
  TYPE_ID desc, 
  UINT bit_offset, 
  UINT bit_size,
  TN *tgt_tn, 
  TN *src_tn, 
  OPS *ops
)
{
  TOP extr_op;
  TN *tmp;

  // for LX as LITTLE_ENDIAN:
  if (MTYPE_is_class_integer(rtype)) {
    // The sequence is shift left 32-bit_size; 
    //                 shift right (bit_offset+bit_size):
    tmp = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer);
    Build_OP(TOP_shl_i, tmp, src_tn, 
      Gen_Literal_TN(MTYPE_bit_size(desc)-bit_offset-bit_size, 4), ops);
    extr_op = MTYPE_signed(rtype) ? TOP_shr_i : TOP_shru_i;
    Build_OP(extr_op, tgt_tn, tmp, 
                  Gen_Literal_TN(MTYPE_bit_size(desc)-bit_size,4), ops);
  }
  else {
    FmtAssert(FALSE, 
           ("Exp_Extract_Bits: unhandled rtype %s", Mtype_Name(rtype)));
  }

  return;
}

/* ======================================================================
 *   Exp_Deposit_Bits 
 *
 *   deposit src2_tn into a field of src1_tn returning the result in 
 *   tgt_tn.
 * ======================================================================
 */
void
Exp_Deposit_Bits (
  TYPE_ID rtype, 
  TYPE_ID desc, 
  UINT bit_offset, 
  UINT bit_size,
  TN *tgt_tn, 
  TN *src1_tn, 
  TN *src2_tn, 
  OPS *ops
)
{
  FmtAssert(FALSE,("not implemented"));
  FmtAssert(bit_size != 0, ("size of bit field cannot be 0"));

  UINT targ_bit_offset = bit_offset;
  if (Target_Byte_Sex == BIG_ENDIAN) {
    targ_bit_offset = MTYPE_bit_size(desc) - bit_offset - bit_size;
  }

  //
  // generate the following sequence (if there is a faster one,
  // we'll find it later)
  //
  // extract bits returns sign-extended TN
  //    tmp1 = extract_bits src1, 0 .. targ_bit_offset
  //    tmp2 = extract_bits src2, targ_bit_offset .. targ_bit_offset+bit_size
  //    tmp3 = extract_bits src1, targ_bit_offset+bit_size .. 32
  //    tmp4 = shl tmp3, 32-bit_size
  //    tmp5 = 

  FmtAssert(rtype == MTYPE_U4,
	  ("Exp_Deposit_Bits: mtype cannot be %s", MTYPE_name(rtype)));

  if (Target_Byte_Sex == BIG_ENDIAN) {
    targ_bit_offset = MTYPE_bit_size(desc) - bit_offset - bit_size;
  }

  if (bit_size <= 16) {
    /*
    Build_OP(TOP_noop, tgt_tn, True_TN, src2_tn, src1_tn,
	     Gen_Literal_TN(targ_bit_offset, 4), Gen_Literal_TN(bit_size, 4), ops);
    */
    return;
  }

  // bit_size > 16 requires 3 instructions

  return;
}

/* ======================================================================
 *   Expand_Lda_Label
 * ======================================================================
 */
void 
Expand_Lda_Label (
  TN *dest, 
  TN *lab, 
  OPS *ops
)
{
  TN *tmp1 = Build_TN_Of_Mtype (Pointer_Mtype);

  FmtAssert(FALSE,("Not Implemented"));

  Set_TN_is_reloc_gprel16(lab);
  // first get address of LT entry
  Expand_Add (tmp1, lab, GP_TN, Pointer_Mtype, ops);
  // then get address of var
  // load is of address, not of result type
  Expand_Load (OPCODE_make_op(OPR_LDID, Pointer_Mtype, Pointer_Mtype),
		                dest, tmp1, Gen_Literal_TN (0, 4), ops);

  return;
}





