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

/*
 * [CG] temporary flag 
 * (until all the compiler is fixed concerning misaligned accesses)
 * If set this flag tells the expander to decompose a misaligned access.
 * If not set the compiler forces a misaligned access (generate an
 * access as if it were not misaligned).
 * At the end this should be a target dependent and/or user level flag.
 * However currently the compiler itself generate misaligned access, and
 * we don't know how to differenciate this with user generated misaligned
 * accesses. Thus we are obliged to force it.
 */
#define GEN_MISALIGNED_ACCESS


/* Import from exp_targ.cxx */
extern TN *Expand_Or_Inline_Immediate(TN *src, TYPE_ID mtype, OPS *ops);

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
  case MTYPE_F4: 
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

#if 0
  // [CG]: never generate and add, even if an extended immediate is needed
  if (TN_has_value(ofst)) {
    ofst = Expand_Or_Inline_Immediate(ofst, MTYPE_I4, ops);
  }

  if (TN_is_register(ofst)) {
    if (ofst != Zero_TN) {
      TN *tmp = Build_TN_Of_Mtype (Pointer_Mtype);
      Expand_Add (tmp, base, ofst, Pointer_Mtype, ops);
      base = tmp;
    }
    ofst = Gen_Literal_TN(0, 4);
  }
#endif
  if (TN_has_value(ofst)) {
    top = Pick_Load_Imm_Instruction (OPCODE_rtype(opcode), mtype);
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
    //
    // SP/FP/GP relative
    //
    top = Pick_Load_Imm_Instruction (OPCODE_rtype(opcode), mtype);
  }
  else {
    FmtAssert(FALSE,("unexpected constant in Expand_Load"));
  }

  FmtAssert(top != TOP_UNDEFINED,("Expand_Load: TOP_UNDEFINED"));

  if (TN_has_value(ofst)) 
    top = TOP_opnd_immediate_variant(top, 0, TN_value(ofst));

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
  case MTYPE_F4:
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

#if 0
  // [CG]: never generate and add, even if an extended immediate is needed
  if (TN_has_value(ofst)) {
    ofst = Expand_Or_Inline_Immediate(ofst, MTYPE_I4, ops);
  }

  if (TN_is_register(ofst)) {
    if (ofst != Zero_TN) {
      TN *tmp = Build_TN_Of_Mtype (Pointer_Mtype);
      Expand_Add (tmp, base, ofst, Pointer_Mtype, ops);
      base = tmp;
    }
    ofst = Gen_Literal_TN(0, 4);
  }
#endif

  if (TN_has_value(ofst)) {
    top = Pick_Store_Imm_Instruction (mtype);
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

    top = Pick_Store_Imm_Instruction (mtype);
  }
  else {
    FmtAssert(FALSE,("unexpected constant in Expand_Store"));
  }

  FmtAssert(top != TOP_UNDEFINED,("Expand_Store: TOP_UNDEFINED"));

  if (TN_has_value(ofst)) 
    top = TOP_opnd_immediate_variant(top, 0, TN_value(ofst));

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
  INT32 new_align = get_variant_alignment(mtype, variant);

  *alignment =	new_align;
  *partials =	MTYPE_alignment(mtype) / new_align;
 //  return Mtype_AlignmentClass(new_align, (MTYPE_type_class(mtype));
  return Mtype_AlignmentClass(new_align, MTYPE_CLASS_UNSIGNED_INTEGER );
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
  if (Potential_Immediate_TN_Expr (opcode, *disp_tn, disp)) {
    if ( TN_has_value(*disp_tn) ) {
      *disp_tn = Gen_Literal_TN ( TN_value(*disp_tn) + disp, 4 );
    } else {
      *disp_tn = Gen_Symbol_TN ( TN_var(*disp_tn),
				 TN_offset(*disp_tn) + disp, TN_relocs(*disp_tn));
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

/* ============================================================
 * [CG]
 * This function add a constant to the  address TNs (base,disp).
 * The disp must be literal.
 * Returns modified (base,disp).
 * ============================================================
 */
static void
Add_Disp_To_Addr_TNs (
  TN	**base_tn,	/* The base address -- may be modified */
  TN	**disp_tn,	/* The displacement -- may be modified */
  INT16	disp)		/* A displacement to add */
{
  if (TN_has_value(*disp_tn)) {
    *disp_tn = Gen_Literal_TN(TN_value(*disp_tn) + disp, 4);
  } else if (TN_is_symbol(*disp_tn)) {
    *disp_tn = Gen_Symbol_TN(TN_var(*disp_tn), TN_offset(*disp_tn) + disp, TN_relocs(*disp_tn));
  } else {
    FmtAssert(0, ("Unexpected non literal displacement"));
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

  INT32		alignment, nLoads, i;
  OPCODE	new_opcode, unsigned_opcode;
  TYPE_ID	new_desc;
  TN		*tmpV[8];

  new_desc = Composed_Align_Type(desc, variant, &alignment, &nLoads);

  // (cbr) loading unaligned floats ? can't have F4U1 load
  if (!MTYPE_is_class_integer(rtype)) {
    rtype = MTYPE_U4;
  }

  // unsigned opcode for all but the most significant bits
  unsigned_opcode = OPCODE_make_signed_op(OPR_LDID, rtype, new_desc, FALSE);
  // opcode of the sign of desc for the most significant bits
  new_opcode = OPCODE_make_signed_op(OPR_LDID, rtype, new_desc, MTYPE_is_signed(desc));
  

  Is_True(nLoads > 1, ("Expand_Composed_Load with nLoads == %d", nLoads));

 /* Generate the component loads, storing the result in a vector
  * of TNs. The vector is filled in such a way that the LSB is in
  * tmpV[0] so that later code can ignore the endianess of the target.
  */
  INT endian_xor = (Target_Byte_Sex == BIG_ENDIAN) ? (nLoads-1) : 0;
  for (i=0; i < nLoads; i++) {
    INT idx = i ^ endian_xor;
    tmpV[idx] = Build_TN_Of_Mtype(rtype);
    if (idx < nLoads-1) {
      Expand_Load (unsigned_opcode, tmpV[idx], base, disp, ops);
    } else {
      Expand_Load (new_opcode, tmpV[idx], base, disp, ops);
    }
    Add_Disp_To_Addr_TNs (&base, &disp, alignment);
  }

  /* 
   * Now combine the components into the desired value.
   */
  INT nLoadBits = alignment * 8;
  TN *tmp0= Build_TN_Of_Mtype(rtype);
  Build_OP(TOP_shl_i, tmp0, tmpV[nLoads-1], Gen_Literal_TN((nLoads-1)*nLoadBits, 4), ops);
  for (i=nLoads-2; i >= 0; i--) {
    TN *tmp1= Build_TN_Of_Mtype(rtype);
    TN *tmp2= i == 0 ? result: Build_TN_Of_Mtype(rtype);
    TN *part = tmpV[i];
    Build_OP(TOP_shl_i, tmp1, part, Gen_Literal_TN(i*nLoadBits, 4), ops);
    Build_OP(TOP_or_r, tmp2, tmp0, tmp1, ops);
    tmp0 = tmp2;
  }
}


/* ====================================================================
 *   Expand_Forced_Misaligned_Load
 *
 *   Generate a load at the given addr + offset as if the load was
 *   correctly aligned. This should end into a runtime error for
 *   machines with unaligned access not supported.
 *   It is an alternative to Expand_Composed_Load.
 *   Called from Expand_Misaligned_Load only.
 * ====================================================================
 */
static void
Expand_Forced_Misaligned_Load (
  OPCODE op, 
  TN *result, 
  TN *base, 
  TN *disp, 
  OPS *ops
  )
{
  TN *new_base = Build_TN_Of_Mtype (Pointer_Mtype);
  TN *new_offset = Gen_Literal_TN(0, 4);
  Expand_Add (new_base, base, disp, Pointer_Mtype, ops);
  Expand_Load (op, result, new_base, new_offset, ops);
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
  DevWarn("misaligned load encountered: TODO : differenciate between compiler an user generated");

#ifdef GEN_MISALIGNED_ACCESS
  Expand_Composed_Load (op, result, base, disp, variant, ops);
#else
  Expand_Forced_Misaligned_Load(op, result, base, disp, ops);
#endif
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
  INT32		alignment, nStores;
  TYPE_ID	new_desc;

  new_desc =	Composed_Align_Type(mtype, variant, &alignment, &nStores);

  if (Target_Byte_Sex == BIG_ENDIAN)
    Add_Disp_To_Addr_TNs (&base, &disp, MTYPE_alignment(mtype)-alignment);

  Expand_Store (new_desc, obj, base, disp, ops); 
  
  while(--nStores >0) {
    TN *tmp = Build_TN_Of_Mtype(mtype);
    Expand_Shift(tmp, obj, Gen_Literal_TN(alignment*8, 4), 
                                                mtype, shift_lright, ops);
    obj = tmp;
    if (Target_Byte_Sex == BIG_ENDIAN)
      Add_Disp_To_Addr_TNs (&base, &disp, -alignment);
    else 
      Add_Disp_To_Addr_TNs (&base, &disp, alignment);
    Expand_Store (new_desc, obj, base, disp, ops); 
  }
}

/* ====================================================================
 *   Expand_Forced_Misaligned_Store
 *
 *   Generate a store at the given addr + offset as if the store was
 *   correctly aligned. This should end into a runtime error for
 *   machines with unaligned access not supported.
 *   It is an alternative to Expand_Composed_Store.
 *   Called from Expand_Misaligned_Store only.
 * ====================================================================
 */
static void
Expand_Forced_Misaligned_Store (
  TYPE_ID mtype, 
  TN *obj_tn, 
  TN *base_tn, 
  TN *disp_tn, 
  OPS *ops
  )
{
  TN *new_base = Build_TN_Of_Mtype (Pointer_Mtype);
  TN *new_offset = Gen_Literal_TN(0, 4);
  Expand_Add (new_base, base_tn, disp_tn, Pointer_Mtype, ops);
  Expand_Store (mtype, obj_tn, new_base, new_offset, ops);
}

static BOOL is_own_func_p (
  ST *st
  )
{
  if (ST_class (st) == CLASS_FUNC) {
    if (! ST_is_preemptible (st))
      return TRUE;
    else if (!Gen_PIC_Shared) {
      return (ST_sclass (st) != SCLASS_EXTERN)
	&& ! ST_is_weak_symbol (st);
    }
  }
  return FALSE;
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
  DevWarn("misaligned store encountered: TODO : differenciate between compiler an user generated");

#ifdef GEN_MISALIGNED_ACCESS
  Expand_Composed_Store (mtype, obj_tn, base_tn, disp_tn, variant, ops);
#else
  Expand_Forced_Misaligned_Store (mtype, obj_tn, base_tn, disp_tn, ops);
#endif
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
           (((ST_class(base_sym) == CLASS_BLOCK || 
	      ST_class(base_sym) == CLASS_VAR) &&
	     ST_gprel(base_sym))
	    || (is_own_func_p(base_sym) && ! Is_Caller_Save_GP))) {
    // gp-relative reference
    PU_References_GP = TRUE;
    base_tn = GP_TN;
    // Use rtype for size of LDA
    ofst_tn = Gen_Symbol_TN (sym, ofst, TN_RELOC_GOT_DISP);

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
  else if (Gen_PIC_Shared || Gen_PIC_Call_Shared) {
    TN *tmp;
    INT32 reloc;

    // Load address of sym from GOT
    if (ST_class(sym) == CLASS_FUNC && Is_Caller_Save_GP)
      reloc = TN_RELOC_GOTOFF_FPTR;
    else
      reloc = TN_RELOC_GOTOFF;

    if (is_lda && ofst == 0) {
      tmp = tn;       // put value from GOT directly into tn
      is_lda = FALSE; // suppress next add
    }
    else
      tmp = Build_TN_Of_Mtype (Pointer_Mtype);
    PU_References_GP = TRUE;
    Expand_Load (OPCODE_make_signed_op(OPR_LDID,
				       Pointer_Mtype, Pointer_Mtype, FALSE),
		 tmp, GP_TN, Gen_Symbol_TN (sym, 0, reloc), &newops);
    // GOT address should not alias
    Set_OP_no_alias (OPS_last (&newops));
    base_tn = tmp;
    // add offset to address
    ofst_tn = Gen_Literal_TN (ofst, Pointer_Size);
  }
  else {
    // Not gp-relative reference

    FmtAssert(!ST_gprel(base_sym),
                 ("Exp_Ldst: %s is set gp-relative", ST_name(base_sym)));
    FmtAssert(!Gen_PIC_Shared && ! Gen_PIC_Call_Shared,
	      ("Exp_Ldst: Absolute reference forbidden by ABI"));
#if 0
    // address TNs
    TN *tmp1 = Build_TN_Of_Mtype (Pointer_Mtype);
#endif
    // address TNs
    TN *tmp1;

    if (is_lda && base_ofst == 0) {
      // want to stop at address (either that or add with 0)
      tmp1 = tn;
      is_lda = FALSE;	// so nothing done
    }
    else {
      // address TNs
      tmp1 = Build_TN_Of_Mtype (Pointer_Mtype);
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

  if (is_store) {
    if (V_alignment(variant) == V_NONE) {
      Expand_Store (OPCODE_desc(opcode), tn, base_tn, ofst_tn, &newops);
    }
    else {
      Expand_Misaligned_Store (OPCODE_desc(opcode), tn, 
			            base_tn, ofst_tn, variant, &newops);
    }
  }
  else if (is_load) {
    if (V_alignment(variant) == V_NONE)
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
 *   Exp_Load
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
  FmtAssert(opc == TOP_UNDEFINED,
            ("Prefetch opcode should be selected in Exp_Prefetch"));
  
  opc = TOP_pft_i;
  if (!TN_has_value(src2)) {
    TN* tmp = Build_TN_Like(src1);
    Expand_Add (tmp, src1, src2, Pointer_Mtype, ops);
    src1 = tmp;
    src2 = Gen_Literal_TN (0, 4);
  }
  opc = TOP_opnd_immediate_variant(opc, 0, TN_value(src2));
  Build_OP(opc, src2, src1, ops);
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
  UINT bit_ofst;
  UINT left_shift;
  UINT right_shift;

  FmtAssert(MTYPE_is_class_integer(rtype) && MTYPE_bit_size(rtype) == 32,
	    ("can't handle"));

  if (Target_Byte_Sex == LITTLE_ENDIAN) {
      // for LX as LITTLE_ENDIAN:
      bit_ofst = bit_offset;
  } else {
      // for LX as BIG_ENDIAN:
      bit_ofst = MTYPE_bit_size(desc) - bit_size - bit_offset;
  }

  // shift left -> clear the bits bit_offset+size -> 31
  left_shift = 32 - bit_ofst - bit_size;
  if (left_shift) {
      tmp = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer);
      Build_OP(TOP_shl_i, tmp, src_tn, 
               Gen_Literal_TN(left_shift, 4), ops);
  } else {
      tmp = src_tn;
  }

  // move the bits to be extracted to position 0
  right_shift = 32 - bit_size;
  extr_op = MTYPE_signed(desc) ? TOP_shr_i : TOP_shru_i;
  Build_OP(extr_op, tgt_tn, tmp, 
           Gen_Literal_TN(right_shift,4), ops);

  return;
}

/* ======================================================================
 *   Exp_Deposit_Bits 
 *
 *   deposit src2_tn into a field of src1_tn returning the result in 
 *   tgt_tn.
 *   Note that this code is endian dependent.
 *   
 * ======================================================================
 */
void
Exp_Deposit_Bits (
  TYPE_ID rtype, 
  TYPE_ID desc, 
  UINT bit_offset, 
  UINT bit_size,
  TN *tgt_tn, 
  TN *src1, 
  TN *src2, 
  OPS *ops
)
{
    UINT bit_ofst;
    UINT right_shift_val;

  FmtAssert(bit_size != 0, ("size of bit field cannot be 0"));

  // Registers are always 32 bits in size
  FmtAssert(MTYPE_is_class_integer(rtype) && MTYPE_bit_size(rtype) == 32,
	  ("Exp_Deposit_Bits: mtype cannot be %s", MTYPE_name(rtype)));

  if (Target_Byte_Sex == LITTLE_ENDIAN) {
      // for LX as LITTLE_ENDIAN:
      bit_ofst = bit_offset;
  } else  {
      // for LX as BIG_ENDIAN
      bit_ofst = MTYPE_bit_size(desc) - bit_size - bit_offset;
  }

  //
  // generate the following sequence (if there is a faster one,
  // we'll find it later)
  //
  //    mask = 0..01110..0       (in the bit pos required)
  //    val  = shl src2, 32-bit_size (clear bits)
  //    val  = shru val, 32-(bit_pos+bit_size) (place value in position)
  //    tmp1 = mask and src1     (only bits in mask pos are valid)
  //    tmp1 = tmp1 xor src1     (bits in mask pos are zero)
  //    tgt_tn = tmp1 or val

  TN *mask = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer);
  
  // set mask to all 1    111....111
  Build_OP(TOP_mov_i, mask, Gen_Literal_TN(-1, 4), ops);
  // only keep bit_size bits to 1
  Build_OP(TOP_shru_i, mask, mask, 
           Gen_Literal_TN(32-bit_size,4), ops);
  
  // shift left to position the bit_size bits to the required position
  Build_OP(TOP_shl_i, mask, mask, Gen_Literal_TN(bit_ofst,4), ops);
  
  // clear bits
  TN *val = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer);
  Build_OP(TOP_shl_i, val, src2, Gen_Literal_TN(32-bit_size,4), ops);

  // place value in position
  right_shift_val = 32-bit_size-bit_ofst;
  Build_OP(TOP_shru_i, val, val,
           Gen_Literal_TN(right_shift_val,4), ops);

  TN *tmp1 = Build_RCLASS_TN (ISA_REGISTER_CLASS_integer);
  Build_OP(TOP_and_r, tmp1, mask, src1, ops);
  Build_OP(TOP_xor_r, tmp1, tmp1, src1, ops);
      
  Build_OP(TOP_or_r, tgt_tn, tmp1, val, ops);
  
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
#if 0
  Print_TN (dest, TRUE );
  Print_TN (lab, TRUE );
#endif

  if (Gen_GP_Relative) {
    // [clarkes]: For gp-relative model, address of a label
    // can be obtained by adding offset to PC, or by loading
    // from initialized data.
    // Adding offset to PC is painful on ST200, as it requires
    // a call instruction, and overwrites R63, so instead load
    // from initialized data.
    Set_TN_is_reloc_got_disp (lab);
    Expand_Load (OPCODE_make_signed_op(OPR_LDID, Pointer_Mtype,
				       Pointer_Mtype, FALSE),
		 dest, GP_TN, lab, ops);
    Set_OP_no_alias (OPS_last (ops));
  }
  else {
    // [CG]: On the ST200 base model, address of a label
    // is the absolute address
    Build_OP (TOP_mov_i, dest, lab, ops);
  }

#if 0
  // CG: commented out
  FmtAssert(FALSE,("Not Implemented"));

  Set_TN_is_reloc_gprel16(lab);
  // first get address of LT entry
  Expand_Add (tmp1, lab, GP_TN, Pointer_Mtype, ops);
  // then get address of var
  // load is of address, not of result type
  Expand_Load (OPCODE_make_op(OPR_LDID, Pointer_Mtype, Pointer_Mtype),
		                dest, tmp1, Gen_Literal_TN (0, 4), ops);
#endif
  return;
}





