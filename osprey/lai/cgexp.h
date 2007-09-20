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


/* ====================================================================
 * ====================================================================
 *
 * Module: cgexp.h
 *
 * Description:
 * ------------
 * This module exports a functional interface for the code expansion 
 * phase of the code generator. It provides a set of routines to 
 * expand high level operations into a sequence of machine level 
 * instructions.
 *
 * It has several distinct clients:
 *
 * 1. The translation of WHIRL into machine instructions (whirl2ops.c)
 * 2. Spill/restore generation during register allocation (cg_spill.c)
 * 3. Generation of new instructions (SWP, entry-exit, CGPREP).
 *
 * Overall Structure:
 * ------------------
 * All the code expansion routines have a common OPS argument.
 * The new OPs will be added to this OPS list.
 *
 * All the code expansion routines have the prefix "Exp_". 
 * There are internal routines with the prefix Expand_,
 * but those should not be used outside of cgexp.
 * ======================================================================
 */


#ifndef cgexp_INCLUDED
#define cgexp_INCLUDED

#include "defs.h"
#include "symtab.h"
#include "variants.h"
#include "cgtarget.h"

extern BOOL Trace_Exp;

/* ----------------------------------------------------------------------
 *   Intitialize things for the cgexp phase at the start of each PU.
 * ----------------------------------------------------------------------
 */
extern void Init_CG_Expand (void);	/* setup trace flags */

/* ----------------------------------------------------------------------
 *   Exp_OP0 / Exp_OP1 / Exp_OP2 / Exp_OP3
 *
 *   Expand 'op' and append it to <ops>.
 *   All these routines create an OP by putting together the opcode 'opr',
 *   the result 'r' and the operands. The resulting OP is then expanded
 *   into machine OPs which are appended to the OPS.
 *
 *   NOTE: <ops> MUST NOT BE IN A BB.
 * ----------------------------------------------------------------------
 */
extern void Exp_OP (OPCODE opcode, TN *result, TN *op1, TN *op2, TN *op3, VARIANT variant, OPS *ops);
#define Exp_OP0(c,r,ops)        	Exp_OP(c,r,NULL,NULL,NULL,0,ops)
#define Exp_OP1(c,r,o1,ops)     	Exp_OP(c,r,o1,NULL,NULL,0,ops)
#define Exp_OP1v(c,r,o1,v,ops)     	Exp_OP(c,r,o1,NULL,NULL,v,ops)
#define Exp_OP2(c,r,o1,o2,ops)  	Exp_OP(c,r,o1,o2,NULL,0,ops)
#define Exp_OP2v(c,r,o1,o2,v,ops)	Exp_OP(c,r,o1,o2,NULL,v,ops)
#define Exp_OP3(c,r,o1,o2,o3,ops)	Exp_OP(c,r,o1,o2,o3,0,ops)
#define Exp_OP3v(c,r,o1,o2,o3,v,ops)	Exp_OP(c,r,o1,o2,o3,v,ops)


/* Generate code to load the address of 'sym'+'ofst' into 'tgt_tn'. 
 * The 'call_opr' parameter indicates whether the LDA is for a call.
 */
extern void Exp_Lda (
  TYPE_ID mtype, TN *tgt_tn, ST *sym, INT64 ofst, OPERATOR call_opr, OPS *ops);

/* Generate code to load memory location 'sym'+'ofst' into 'tgt_tn'. */
extern void Exp_Load (
  TYPE_ID rtype, TYPE_ID desc, TN *tgt_tn, ST *sym, INT64 ofst, OPS *ops, VARIANT variant);

/* Generate code to store 'src_tn' into the memory location 'sym'+'ofst'. */
extern void Exp_Store (
  TYPE_ID mtype, TN *src_tn, ST *sym, INT64 ofst, OPS *ops, VARIANT variant);

/* Generate a copy from 'src_tn' to 'tgt_tn'. */
extern void Exp_COPY (TN *tgt_tn, TN *src_tn, OPS *ops); 

#ifdef TARG_ST
extern void Exp_Enable_Allocate_Object(BOOL onoff);
#endif

/* Given a simulated <op>, expand it into the sequence of instructions
 * that must be generated. The <pc_value> is the PC location of the 
 * <op>. It is used to generate offsets for branches in the sequence 
 * of instructions generated.
 */
#ifdef TARG_ST
extern void Exp_Simulated_Op (const OP *op, OPS *ops, INT pc_value, ST **sym);
#else
extern void Exp_Simulated_Op (const OP *op, OPS *ops, INT pc_value);
#endif

/* For the given simulated <op>, return the number of 
 * instructions/instruction-words that will be generated after expansion.
 */
extern INT Simulated_Op_Real_Ops (const OP *op);
extern INT Simulated_Op_Real_Inst_Words (const OP *op);

/* Initial expansion of intrinsic call (may not be complete lowering).
 * return result TN (if set).
 * if creates a loop, then returns loop ops and label for new bb.
 */
#ifdef TARG_ST
/* JV: Use same kind of interface for Exp_Intrinsic_Op and Exp_Intrinsic_Call */
extern void Exp_Intrinsic_Call (INTRINSIC id, INT num_results, INT num_opnds, TN **result, TN **opnd,
				OPS *ops, LABEL_IDX *label, OPS *loop_ops, SRCPOS srcpos);
#else
extern TN * Exp_Intrinsic_Call (
  INTRINSIC id, TN *op0, TN *op1, TN *op2, OPS *ops, 
  LABEL_IDX *label, OPS *loop_ops);
#endif

/* expand intrinsic op */
#ifdef TARG_ST
/* Arthur: this is because I may have to generate them for Lai */
extern void Exp_Intrinsic_Op (INTRINSIC id, INT num_results, INT num_opnds, TN **result, TN **opnd, OPS *ops, SRCPOS scrpos);
#else
extern void Exp_Intrinsic_Op (INTRINSIC id, TN *result, TN *op0, TN *op1, TN *op2, OPS *ops);
#endif

/* Expand TN(const) into a sequence of ops (used in prolog)
 */
extern void Exp_Immediate (TN *dest, TN *src, BOOL is_signed, OPS *);

/* create add/sub/mpy op of given mtype */
#define Exp_ADD(mtype,dest,src1,src2,ops)	\
	Exp_OP2 (OPCODE_make_op(OPR_ADD,mtype,MTYPE_V), dest,src1,src2,ops)
#define Exp_SUB(mtype,dest,src1,src2,ops)	\
	Exp_OP2 (OPCODE_make_op(OPR_SUB,mtype,MTYPE_V), dest,src1,src2,ops)
#define Exp_MPY(mtype,dest,src1,src2,ops)	\
	Exp_OP2 (OPCODE_make_op(OPR_MPY,mtype,MTYPE_V), dest,src1,src2,ops)

/* check whether to eval the condition separate from the select */
extern BOOL Check_Select_Expansion (OPCODE compare);

/* create select and condition for select */
extern void Exp_Select_And_Condition (
        OPCODE select, TN *result, TN *true_tn, TN *false_tn,
        OPCODE compare, TN *cmp_kid1, TN *cmp_kid2, VARIANT variant, OPS *ops);

#if 0
/* Arthur: moved to cg.h */
/* ----------------------------------------------------------------------
 * By default, when we expand multi-instruction sequences, we use a new
 * temporary TN for each intermediate result.  This simplifies things for
 * CGPREP.  But in some case like spill code and and -O0 code, it is more
 * efficient to reuse the same TN for intermediate results.  We reuse the
 * TNs when this flag, Reuse_Temp_TNs, is TRUE.
 * ----------------------------------------------------------------------
 */
extern BOOL Reuse_Temp_TNs;
#define Get_Temp_TN(tn)	(Reuse_Temp_TNs ? tn : Build_TN_Like(tn))
#endif

/* Expand a local (to a BB) jump, choosing between a PIC (longer) sequence
 * and an absolute (shorter) sequence depending on the compilation mode.
 * A jump implies the ability to jump long distances.
 */
extern void Exp_Local_Jump(BB *bb, INT64 offset, OPS *ops);


/*
 * determine if a given symbol is a stack relative reference that will
 * require multiple instructions to load or store.
 */
extern BOOL Exp_Is_Large_Stack_Sym(ST* sym, INT64 ofst);

/* generate prefetch inst */
extern void Exp_Prefetch (TOP opc, TN *src1, TN *src2, VARIANT variant, OPS *ops);

/* generate instructions to extract a bit field */
extern void Exp_Extract_Bits (TYPE_ID rtype, TYPE_ID desc, UINT bit_offset,
			      UINT bit_size, TN *tgt_tn, TN *src_tn, OPS *ops);

/* generate instructions to deposit value in a bit field */
extern void Exp_Deposit_Bits (TYPE_ID rtype, TYPE_ID desc, 
			      UINT bit_offset, UINT bit_size, 
			      TN *tgt_tn, TN *src1_tn, TN *src2_tn, OPS *ops);

/* generate instructions to rotate-left a bit field */
extern void Exp_LRotate (TYPE_ID rtype,
			 TN *tgt_tn, TN *src1_tn, TN *src2_tn, OPS *ops);

/* generate instructions to rotate-right a bit field */
extern void Exp_RRotate (TYPE_ID rtype,
			 TN *tgt_tn, TN *src1_tn, TN *src2_tn, OPS *ops);

/* expand return instruction */
extern void Exp_Return (TN *return_address, OPS *ops);
extern void Exp_Return_Interrupt (TN *return_address, OPS *ops);

/* expand call instruction */
extern void Exp_Call (OPERATOR opr, TN *return_address, TN *target, OPS *ops);

/* expand indirect branch */
extern void Exp_Indirect_Branch (TN *targ_reg, OPS *ops);

/* expand no-op */
extern void Exp_Noop (OPS *ops);

/* build spadjust op */
extern void Exp_Spadjust (TN *dest, TN *size, VARIANT variant, OPS *ops);

/* expand GP initialization */
extern void Exp_GP_Init (TN *dest, ST *st, OPS *ops);

/* 
 *  Generate a generic 2-result predicate operation.
 *  COMPARE_type_or sets result1 and result2 true if qual_pred is true
 *  COMPARE_type_and sets result1 and result2 false if qual_pred is true
 */
extern void Exp_Generic_Pred_Calc(TN* result1, TN *result2, COMPARE_TYPE ctype,
				  TN *qual_pred, OPS *ops);

/*
 * Used by if conversion. See expand.cxx for details.
 */
extern void Exp_True_False_Preds_For_Block(BB *bb,
					   TN* &true_tn, TN * &false_tn);

/* Predicate manipulation routines.
 *
 * Most if not all of these routines take two result TNs. The second
 * result is a complement of the first. If one result is not needed,
 * it can be specified as NULL, in which case the utility will allocate
 * a temp if necessary, or a dedicated TN for a sink register (if
 * the architecture has one). 
 */
extern void Exp_Pred_Set(TN *dest, TN *cdest, INT val, OPS *ops);
extern void Exp_Pred_Copy(TN *dest, TN *cdest, TN *src, OPS *ops);
extern void Exp_Pred_Complement(TN *dest, TN *cdest, TN *src, OPS *ops);
extern void Exp_Pred_Compare(TN *dest, TN *cdest, TN *src1, TN *src2, 
			     VARIANT variant, OPS *ops);


#ifdef TARG_ST
extern void Exp_Var_Extract (INT count, TN **dests, TN *src, OPS *ops);
extern void Exp_Var_Compose (INT count, TN *dst, TN **srcs, OPS *ops);
#endif

#ifndef TARG_ST
// Arthur: moved to be/com/betarget.h

/* check if target can handle immediate operand;
 * True if target can, false if should use target-independent logic.
 */
extern BOOL Target_Has_Immediate_Operand (WN *parent, WN *expr);
#endif

#include "exp_targ.h"

#endif /* cgexp_INCLUDED */
