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
 *  Target Specific Miscellany
 *
 *  Here are various miscellaneous functions to provide machine dependent
 *  information.
 *
 *  Exported functions:
 *
 *    Misc:
 *
 *	void CGTARG_Initialize(void)
 *	  Perform one-time initialization.
 *
 *    Target capabilities and peculiarities:
 *
 *	const INT INST_BYTES
 *	  The number of bytes per instruction word.
 *
 *      const INT DEFAULT_LONG_BRANCH_LIMIT
 *	  The default limit for branch displacements (in bytes) after
 *	  which long-branch fixups occur.
 *
 *      const INT DEFAULT_BRP_BRANCH_LIMIT
 *	  The default limit for branch-predict displacements (in bytes).
 *        This currently applies only for IA-64, all other targets have 
 *        this set to INT32_MAX. This is the legal displacement limit
 *        of the branch-predict instructions, beyond which they will be
 *        converted to NOPs by the cgemit phase. 
 *
 *      const INT MIN_BRANCH_DISP
 *	  The displacement (in bytes) for the shortest PC-relative branch.
 *
 *	BOOL CGTARG_Have_Indexed_Mem_Insts(void)
 *	  Returns a boolean that indicates if the target supports
 *	  indexed memory reference instructions.
 *
 *	TOP CGTARG_Inter_RegClass_Copy(ISA_REGISTER_CLASS dst,
 *				       ISA_REGISTER_CLASS src,
 *				       BOOL is_double)
 *	  Returns the copy instruction for moving data from register
 *	  class <src> to <dst> in single or double precision according
 *	  to <is_double>. Returns TOP_UNDEFINED if there is no
 *	  such instruction.
 *
 *	INT CGTARG_Text_Alignment(void)
 *	  Return the best instruction alignment for the processor.
 *
 *      INT32 CGTARG_Branch_Taken_Penalty(void)
 *        Return the number of extra cycles (often 0) consumed by a
 *        branch that is taken (even if correctly predicted).
 *
 *	INT32 CGTARG_Max_OP_Latency(OP op)
 *	  Return the maximum latency this op can have to any
 *	  successor.
 *
 *	INT32 CGTARG_Max_RES_Latency(OP op, INT i)
 *	  Return the maximum latency this result in op can have to any
 *	  successor.
 *
 *	BOOL CGTARG_Use_Brlikely(float branch_taken_probability)
 *	  Given a conditional branch with a <branch_taken_probability>
 *	  return TRUE if it would be beneficial to convert it to a brlikely.
 *	  This handles targets that have a penalty for brlikely 
 *        instructions that are not taken (e.g. T5).
 *
 *	BOOL CGTARG_Can_Change_To_Brlikely(OP *xfer_op, TOP *new_opcode)
 *        Checks to see if <xfer_op> can be converted to its branch-likely 
 *	  form and returns the new_opcode if result TRUE.
 *
 *	BOOL CGTARG_Can_Predicate_Calls()
 *	  Returns true if the target supports a predicated call instruction.
 *
 *	BOOL CGTARG_Can_Predicate_Returns()
 *	  Returns true if the target supports a predicated call instruction.
 *
 *	BOOL CGTARG_Can_Predicate_Branches()
 *	  Returns true if the target supports predicated branch instructions.
 *
 *	BOOL CGTARG_Unconditional_Compare(OP* op, TOP* uncond_ver)
 *	  Returns true if a compare sets predicates unconditionally.  It
 *	  returns the unconditional version of the compare if one exists.
 *
 *      TOP CGTARG_Noop_Top(void)
 *        TOP (opcode) to use for noops on current target.
 *        
 *      TOP CGTARG_Simulated_Top(TOP, ISA_EXEC_UNIT_PROPERTY)
 *        Returns the right opcode for simulated TOP which matches 
 *	  ISA_EXEC_UNIT_PROPERTY.
 *        
 *      TOP CGTARG_Noop(ISA_EXEC_UNIT_PROPERTY)
 *        TOP (opcode) to use for noops on current target, with given unit.
 *        
 *      BOOL CGTARG_Can_daddu_Be_Folded(OP *op1, OP *op2)
 *        Returns TRUE if <op1> and <op2> is a daddu/addiu sequence
 *	  that can be folded, i.e.  <op1> is a addiu op which is feeding 
 *	  into daddu op <op2>.
 *
 *      TOP CGTARG_Copy_Op(UINT8 size, BOOL is_float)
 *        TOP (opcode) to use for copying a register <size> bytes in
 *	  length (FP if <is_float>).
 *
 *	TOP CGTARG_Parallel_Compare(TOP cmp_top, COMPARE_TYPE ctype)
 *	  Return a parallel version of cmp_top if any such exists.
 *        
 *	BOOL CGTARG_Dependence_Required(OP *pred_op, OP *succ_op)
 *	  Check for any target-specific dependences (other than the usual
 *        repertoire) that may be required between <pred_op> and <succ_op>. 
 *        
 *	void CGTARG_Adjust_Latency(OP *pred_op, OP *succ_op, 
 *                                 CG_DEP_KIND kind, UINT8 opnd, INT *latency)
 *	  Makes any target-specific latency adjustments that may be
 *	  required between <pred_op> and <succ_op>.
 *        
 *	void CGTARG_Peak_Rate( PEAK_RATE_CLASS prc, INT ratio[2] )
 *	  What is the peak rate (in ops per cycle) for the given
 *	  class of instruction(s). A rate of 0 means the class of
 *	  instruction isn't supported on the target processor.
 *	  The rate is expressed as a fraction: insts/cycle and stored
 *	  in the <ratio> array (ratio[0]==insts, ratio[1]==cycles).
 *
 *	void CGTARG_Compute_PRC_INFO (BB *bb, PRC_INFO *info)
 *	  For the given basic block <bb> compute some basic information
 *        about the distribution of the different instruction types.
 *	  Return this information in <info>.
 *
 *	void CGTARG_Print_PRC_INFO (FILE *file, PRC_INFO *info, INT ii,
 *				    const char *prefix, const char *suffix)
 *	  Print the PRC_INFO <info> to <file>. The iteration interval to
 *	  be used for printing the statistics is <ii>. Use the string
 *	  <prefix> before printing each line of output. Use the string
 *	  <suffix> after printing each line of output.
 *
 *	INT CGTARG_ARC_Sched_Latency( ARC *arc )
 *	  Wrapper function for ARC_latency to let us fix up some cases 
 *	  where it returns a result that just doesn't make sence.  In
 *	  particular a latency of -1 for the pre-branch latency makes no
 *	  scheduling sense for CPUs which have same-cycle branch shadows.
 *	  Should be 0.
 *
 *      void CGTARG_Handle_Errata_Hazard (OP *op, INT erratnum, 
 *                                        INT ops_to_check)
 *        Handle all the Errata hazards. These are typically workarounds
 *        for bugs in particular versions of various target processors.
 *
 *      void CGTARG_Handle_Bundle_Hazard(OP *op, TI_BUNDLE *bundle, 
 *					 VECTOR *bundle_vector, 
 *					 BOOL can_fill, INT slot_pos, 
 *					 INT max_pos,
 *					 ISA_EXEC_UNIT_PROPERTY prop)
 *	  Handle all bundle hazards in this routine. It inserts any extra
 *        nops required to form  a legal bundle. (1) <op> is the current
 *        OP being processed. (2) <bundle> is the current bundle. (3)
 *        <bundle_vector> is an internal vect to determine relations with
 * 	  any prior filled OPs in the <bundle>. (4) <can_fill> tells that
 *        a <slot_pos> has already been chosen for the <op> in <bundle>.
 *        (5) <max_pos> identifies OPs which occupy more than one slot position.
 *        (6) <prop> is the ISA_EXEC_UNIT_PROPERTY type that is being used.
 *
 *      BOOL CGTARG_Bundle_Slot_Available(TI_BUNDLE *bundle, OP *op, INT slot,
 *					  ISA_EXEC_UNIT_PROPERTY *prop,
 *					  BOOL stop_bit_reqd,
 *                                        const CG_GROUPING *grouping)
 *	  Checks to see if <slot> position is available for <op> using the
 *	  property type <prop> in the <bundle>. The <stop_bit_reqd> flag
 *        checks if stop bit is also required due to dependencies.  The
 *        grouping parameter, if not NULL, is used to choose between property
 *        types where there is a choice.
 *
 * 	BOOL CGTARG_Bundle_Stop_Bit_Available(TI_BUNDLE *bundle, INT slot);
 *        Checks to see if <stop> bit is available at <slot> position in the
 *	  bundle.
 *
 *	BOOL CGTARG_Preg_Register_And_Class(WN_OFFSET preg,
 *					    ISA_REGISTER_CLASS *rclass,
 *					    REGISTER *reg)
 *	  Return the register and class (by reference through 'rclass'
 *	  and 'reg') corresponding to 'preg'. The return value indicates
 *	  if there was a preg -> register/class mapping.
 *
 *
 *	void CGTARG_Compute_Branch_Parameters(INT32 *mispredict,
 *					      INT32 *fixed,
 *					      INT32 *brtaken,
 *					      double factor)
 *	  Various machine dependent values use for non loop if conversion.
 *	  mispredict:
 *		Branch misprediction penalty. Given the condition ready, how 
 *		many cycles are wasted in resolving the new branch and 
 *		starting the pipline?
 *	  fixed:
 *		Number of cycles in executing the branch and it's trailer.
 *	  brtaken:
 *		For some machines, the taken pc requires ALU resources.
 *	  factor
 *		The misprediction rate may not correlate to the branch 
 *		probability.
 *		A factor (0..1.0) will scale the misprediction penalty.
 *
 *	INT32 CGTARG_Special_Min_II(BB* loop_body, BOOL trace)
 *	  Check for target specific (tail stepping, and other?) special
 *	  cases that might force a higher Min II. If a case applies, the
 *	  target specific MII is returned, otherwise 0 is returned.
 *
 *    OP/TOP specific:
 *
 *      BOOL CGTARG_Is_OP_Barrier(OP* op)
 *        Returns TRUE if <op> is a barrier node.
 *        
 *      BOOL CGTARG_Is_OP_Intrinsic(OP *op)
 *        Returns TRUE if <op> is an intrinsic node.
 *
 *      BOOL CGTARG_Is_OP_Speculative(OP *op)
 *        Returns TRUE if <op> is a safe speculative OP.
 *
 *      BOOL CGTARG_Is_OP_Addr_Incr(OP* op)
 *        Returns TRUE if <op> is an address increment op
 *	  (eg. {addiu, daddiu} rd, rd, offset) with the destination and
 *         source1 being the same registers.
 *
 *      void CGTARG_Perform_THR_Code_Generation(OP *op, THR_TYPE type)
 *        Perform THR (and target-specific) code generation tasks per
 *        THR_TYPE and OP. Depending on <type>, <op> can now be 
 *        substituted (or replaced with) new ops.
 *
 *	INT CGTARG_Copy_Operand(OP *op)
 *	  If 'op' performs a copy operation, return the index of
 *	  the source operand; otherwise return -1.
 *
 *	TN *CGTARG_Copy_Operand_TN(OP *op)
 *	  If 'op' performs a copy operation, return the TN of
 *	  the source operand; otherwise return NULL.
 *
 *	INT CGTARG_Copy_Result(OP *op)
 *	  If 'op' is a copy operation, return the index of
 *	  the result; otherwise return -1.
 *
 *	TN *CGTARG_Copy_Result_TN(OP *op)
 *	  If 'op' is a copy operation, return the TN of
 *	  the result; otherwise return NULL.
 *
 *	BOOL CGTARG_Is_Copy(OP *op)
 *	  Return a boolean indicating if 'op' performs a copy operation.
 *
 *	BOOL CGTARG_Is_Preference_Copy(OP *op)
 *	  Return a boolean indicating if 'op' performs a copy operation
 *	  that is a candidate for preferencing.  Does extra consistency
 *	  checks.  This is the preferred method for testing for a copy
 *	  before invoking CGTARG_Copy_Operand() and friends.
 *
 *	BOOL CGTARG_Is_OP_daddu(OP *op)
 *	  Return a boolean indicating if 'op' is a mips daddu inst.
 *
 *	void CGTARG_Load_From_Memory(TN *tn, ST *mem_loc, OPS *ops)
 *	void CGTARG_Store_To_Memory(TN *tn, ST *mem_loc, OPS *ops)
 *	  Load/store any TN from/to memory, even if it takes a sequence
 *	  of instructions to do so. Note that the expander can only
 *	  store TNs in register classes that have load and store
 *	  instructions.
 *
 *	TOP CGTARG_Immed_To_Reg(TOP opr)
 *	  <opr> is an immediate format instruction, return its equivalent
 *	  register form if there is one, return TOP_UNDEFINED if
 *	  there isn't.
 *
 *	void CGTARG_Predicate_OP(BB* bb, OP* op, TN* pred_tn)
 *	    Replace <op> with its predicated form, inserting new ops as needed.
 *
 *      OP *CGTARG_Dup_OP_Predicate (OP* op, TN *new_pred)
 *          Duplicate <op> and predicate the copy with <new_pred>.
 *
 *	BOOL CGTARG_Branches_On_True(OP* br_op, OP* cmp_op)
 *	    Returns TRUE if branch executed on TRUE test of cmp_op.  Note
 *	    that it doesn't really verify that br_op is conditional, so it
 *	    may return odd results if given an unconditional branch.
 *
 *      BOOL CGTARG_Is_Long_Latency(TOP op)
 *	  Return a boolean indicating if <op> is a long latency operation.
 *
 *      BOOL CGTARG_Is_Bad_Shift_Op (OP *op)
 *        Return TRUE if <op> is a shift instruction that has the
 *        ERRATA01 or the ERRATA02 hazard.
 *
 *	TOP CGTARG_Invert(TOP opr)
 *	  Return the inverse of <opr>. TOP_UNDEFINED is returned
 *	  if there is no inverse. Note that the inverse of an
 *	  instruction that takes an immediate does not also necessarily
 *	  take an immediate. 
 *
 *      void CGTARG_Branch_Info ( const OP* op, INT* tfirst, INT* tcount )
 *        Given a branch OP, which operand is the first branch target and how
 *        many branch targets are there?  Returns index of first branch target
 *        by reference in <tfirst> and count of branch targets in <tcount>.
 *
 *	UINT32 CGTARG_Mem_Ref_Bytes(const OP *memop)
 *	  Requires: OP_load(memop) || OP_store(memop)
 *	  Return the number of bytes in memory referenced by <memop>.
 *  
 *      BOOL CGTARG_Can_Be_Speculative( OP* op )
 *        Determine whether the given OP may be scheduled eagerly.  This
 *        involves a target-specific mapping of its exception classification
 *        to an exception level, and determination of whether that level of
 *        speculation is enabled.
 *  
 *        There is a major can of worms lurking under this seemingly innocent
 *        function.  Different targets have vastly different styles of
 *        speculation.  For example, TFP was designed with the expectation
 *        that software would be responsible for speculation and thus fully
 *        supports dismissing floating point exceptions in hardware.  By
 *        contrast, T5 does not support this.  Triton, which does nothing in
 *        hardware to speculate also cannot dismiss may floating point traps.
 *
 *      BOOL CGTARG_Is_OP_Speculative_Load( OP* memop )
 *      BOOL CGTARG_Is_OP_Advanced_Load( OP* memop )
 *      BOOL CGTARG_Is_OP_Check_Load( OP* memop )
 *        These routines check to see if <memop> is a speculative,
 *        advanced or check load. Assumes that <memop> is a load operation.
 *  
 *	VARIANT CGTARG_Analyze_Compare(OP *br,
 *				       TN **tn1, 
 *				       TN **tn2, 
 *				       OP **compare_op)
 *	  Analyze a branch to determine the condition of the branch and
 *	  TNs being compared. Where possible and appropriate if the branch
 *	  is based on the result of an slt instruction, take that in to
 *	  consideration in determining the condition and arguments.
 * 
 *	  The variant of the branch (see variant.h) is returned through 
 *	  the function return value.
 *
 *	  <br> is a pointer to a branch instruction to be analyzed. The
 *	  arguments of the comparison are returned through the out parameters
 *	  <tn1> and <tn2>. The comparison OP is returned through out
 *	  parameter <compare_op>. 
 *
 *	  NOTES: Currently restricted to integer branches; also see
 *	  CGTARG_Analyze_Branch
 *
 *      VARIANT CGTARG_Analyze_Branch(OP *br, TN **tn1, TN **tn2)
 *	  Analyze a branch to determine the condition of the branch and
 *	  the operand TNs.
 * 
 *	  The variant of the branch (see variant.h) is returned through 
 *	  the function return value.
 *
 *	  <br> is a pointer to a branch instruction to be analyzed. The
 *	  arguments of the branch are returned through the out parameters
 *	  <tn1> and <tn2>. 
 *
 *	  NOTES: also see CGTARG_Analyze_Compare
 *
 *    Horrible implicit register definitions and uses:
 *
 *      BOOL CGTARG_OP_Defs_TN( OP* op, TN* tn )
 *      BOOL CGTARG_OP_Refs_TN( OP* op, TN* tn )
 *        There are (only very few) cases where a TN is implictly defined or
 *        referenced by certain OPs.  These functions allow us to find these
 *        cases. (Currently used only for the floating status register.)
 *
 *    Associated base operations:
 *
 *	void CGTARG_Init_Assoc_Base(void)
 *	  A number of tables are maintained for performing base
 *	  association related inqueries. The accessors to the
 *	  tables are declared below. See cg_loop_recur:Find_Assoc_Base_Opr
 *	  and cgprep:assoc_base_opr for additional details.
 *
 *	OPCODE CGTARG_Assoc_Base_Opr(TOP topcode)
 *	  Return the correpsonding associated base whirl opcode for <topcode>.
 *
 *	TOP CGTARG_Assoc_Base_Top(TOP topcode)
 *	  Return the correpsonding associated base topcode for <topcode>.
 *
 *	CGTARG_ASSOC_BASE_FNC CGTARG_Assoc_Base_Fnc(TOP topcode)
 *	  When the associated base operation is not a simple mapping
 *	  based soley on <topcode>, this function returns an
 *	  enumeration of the function that needs to be performed to
 *	  determine the corresponding base operation, if any.
 *	  The following are the possible enumerations:
 *
 *	  ASSOC_BASE_null
 *	    This is a simple mapping, no additional function need be performed.
 *
 *	  ASSOC_BASE_inverse
 *	    Indicates this is an inverse operation.
 *
 *	  ASSOC_BASE_minmax
 *	    <topcode> is a min/max operation comprised of multiple operations.
 *	    The additional operations need to be checked.
 *
 *	  ASSOC_BASE_imul
 *	    <topcode> is an integer multiply. It is necessary to determine
 *	    if the multiply is within a recurrence.
 *
 *  Target specific live range interference:
 *
 *    For TFP we implement two register allocation restrictions.  Violation of
 *    these rules results in stalls.  Essentially this is because the hardware
 *    does not have register renaming and a specific register can contain at
 *    most one particular value in any given cycle.
 *
 *      1. No two operations can target the same register with the
 *         same register write cycle, and
 *
 *      2. No operand may be read in the register write cycle of
 *         another value.  This particularly disallows flds to overwrite the
 *         operand of a previous flop in the same cycle.
 *    
 *    A third restriction exists but is not implemented:
 *
 *      3. V0 may not redifine the register of v1 while v1 is in
 *         flight.  A live range is in flight from its definition point
 *         through the cycle before it is available for bypassing.  This
 *         final rule is not currently implemented.  TODO: implement it if
 *         it becomes a problem.
 *
 *    Theese restrictions are convied through the following functions.  They
 *    are intended to be called for the operation in a set of contigious
 *    blocks before register allocation is performend but after secheduling.
 *    The client supplies live ranges and information about which OP define
 *    and use them when in the schedules.  A client supplied function is
 *    called when two live ranges may not have the same register (interfere).
 *    
 *    It is important to call these functions in a particular order:
 *
 *      1. CGTARG_Interface_Initialize
 *      2. All calls to CGTARG_Result_Live_Range
 *      3. All calls to CGTARG_Operand_Live_Range
 *      4. CGTARG_Interface_Finalize
 *
 *    Exported Function for target specific live range interference:
 *
 *      BOOL CGTARG_Interference_Required(void)
 *        Does the current target require target specific live range
 *        interference at all?
 *        
 *      void CGTARG_Interference_Initialize( INT32 cycle_count,
 *                                           BOOL is_loop,
 *                                           void (*inter_fn)(void*,void*) )
 *        Prepare to make the target specific interferences.  <cycle_count> is
 *        the number of total cycles in the contigious set of blocks under
 *        consideration.  <is_loop> is true if the blocks constitute a loop
 *        body (and thus we need to worry about OPs in the final few cycles
 *        writing in the first few cycles of the loop.)  <inter_fn> ia a
 *        function pointer to be called when for pairs of live ranges that
 *        must not be allocated to the same register.  It is assumed that
 *        interference is a reflexive relation and thus that it is not
 *        necessary to call the function for both x,y and y,x.  (So the client
 *        must make sure that both interference graph edges are drawn.)
 *        
 *      void CGTARG_Result_Live_Range( void* lrange, OP* op, INT32 offset )
 *        Present the <lrange> written by <op>.  <offset> gives a cycle count
 *        to be added to OP_scycle as the start time of <op> relative to the
 *        first cycle of the blocks under consideration.
 *        
 *      void CGTARG_Operand_Live_Range( void * lrange, INT opnd, OP* op,
 *                                      INT32  offset )
 *        Present the <lrange> used by <op> and it's <opnd> operand.  <offset>
 *        gives a cycle count to be added to OP_scycle as the start time of
 *        <op> relative tot he first cycle of the blocks under consideration.
 *        
 *      void CGTARG_Interference_Finalize(void)
 *        All done presenting this set of OPs.
 *
 *      TOP CGTARG_Which_OP_Select (UINT16 bit_size, BOOL is_float,
 *                                  BOOL is_fcc)
 *        Determine which opcode to use to copy TNs based upon whether or
 *        not the value being copied is floating point, its size in bits,
 *        and whether the condition to be used is a floating CC or integer
 *        register.
 *
 *      TOP CGTARG_Equiv_Nonindex_Memory_Op(OP *op)
 *        Return the nonindexed OP code corresponding to a indexed memory
 *        OP or TOP_UNDEFINED if code is not an indexed memory OP.
 *
 *      void CGTARG_Generate_Remainder_Branch(TN *trip_count, TN *label_tn
 *                                            OPS *prolog_ops, OPS *body_ops)
 *        Generate the branch instruction and its initialization into the
 *        body_ops and prolog_ops respectively for the unrolling remainder loop.
 *
 *      void CGTARG_Generate_Branch_Cloop(OP *op, TN *unrolled_trip_count, TN *trip_count, INT32 ntimes,
 *                                        TN *label_tn, OPS *prolog_ops. OPS *body_ops);
 *        If the target architecture supports a counted loop instruction,
 *        generate the initialization and the cloop instruction and append
 *        them to the prolog_ops and body_ops respectively.
 *
 *      BOOL CGTARG_OP_is_counted_loop(OP *op)
 *        Returns TRUE if op is the counted loop branch.
 *        
 *	TOP CGTARG_Invert_Branch(BB* bb)
 *	  Invert sense of the branch terminating bb.  Returns new branch
 *	  op if successful.
 *
 *      void CGTARG_Init_OP_cond_def_kind(OP *op)
 *        Initialize target dependent OP_cond_def_kind.
 *
 *      void CGTARG_Use_Load_Latency(OP *, TN *)
 *        The TN is the load-result of the OP, so use of TN has the
 *        latency of a load
 *
 *  BOOL CGTARG_Is_Simple_Jump(const OP* op); 	 
 *    Check whether given op is a simple jump, i.e. a jump with a known label
 *    (OU_target operand)
 *
 *   BOOL CGTARG_op_may_alias_with_call(const OP* op)
 *      Returns whether op may alias with a call. (used in loop_invar_hoist.cxx)
 * ====================================================================
 * ==================================================================== */

#ifndef CGTARGET_INCLUDED
#define CGTARGET_INCLUDED

#include "defs.h"
#include "cgir.h"
#include "variants.h"
#include "cg_flags.h"
#include "cg_dep_graph.h"
#include "cg_vector.h"
#ifdef TARG_ST 
#include "config_target.h"
#include "cg_loop.h"
#else
#include "config_targ.h"
#endif
#include "cg_thr.h"

#include "targ_proc_properties.h"
#include "targ_abi_properties.h"
#include "targ_isa_lits.h"
#include "targ_isa_registers.h"
#include "targ_isa_enums.h"
#include "targ_isa_pack.h"
#include "targ_isa_bundle.h"
#include "targ_isa_print.h"
#include "targ_grouping.h"

#include "ti_errors.h"
#include "ti_bundle.h"
#include "ti_latency.h"

/* Target-specific information */
#include "targ_cg.h"

/* placeholder for all hardware workarounds */
extern void Hardware_Workarounds (void);

/* --------------------------------------------------------------------
 *   Perform one-time initialization.
 * --------------------------------------------------------------------
 */
extern void CGTARG_Initialize(void);

/* ====================================================================
 *    Branch related interface:
 * ====================================================================
 */

extern UINT32 CGTARG_branch_taken_penalty;
extern BOOL CGTARG_branch_taken_penalty_overridden;

// If a BB ends in an unconditional branch, turn it into a 
// conditional branch with TRUE predicate.
#ifdef TARG_ST
extern void Make_Branch_Conditional(BB *bb, TN *pred_tn, BOOL cond);
#else
extern void Make_Branch_Conditional(BB *bb);
#endif

// Given a branch OP, which operand is the first branch target and how
// many branch targets are there?
extern void CGTARG_Branch_Info ( const OP* op, INT* tfirst, INT* tcount );

// Analyze a branch to determine the condition of the branch and
// the operand TNs.
extern VARIANT CGTARG_Analyze_Branch(OP *br,
				     TN **tn1, 
				     TN **tn2);
// Analyze a branch to determine the condition of the branch and
// TNs being compared.
extern VARIANT CGTARG_Analyze_Compare(OP *br,
				      TN **tn1,
				      TN **tn2,
				      OP **compare_op);

// Various machine dependent values use for non loop if conversion.
extern void CGTARG_Compute_Branch_Parameters(INT32 *mispredict,
						    INT32 *fixed,
						    INT32 *brtaken,
						    double *factor);

extern void CGTARG_Generate_Remainder_Branch(TN *trip_count, TN *label_tn,
					     OPS *prolog_ops, OPS *body_ops);

#ifdef TARG_ST
extern BOOL CGTARG_Generate_Branch_Cloop(LOOP_DESCR* cl,
                                         OP *op, TN *trip_count,
					 TN *label_tn, 
                                         BB *prolog, BB *tail);
#else
extern void CGTARG_Generate_Branch_Cloop(OP *op, TN *unrolled_trip_count, 
                                         TN *trip_count,
					 INT32 ntimes, TN *label_tn, 
                                         OPS *prolog_ops, OPS *body_ops);
#endif

/* ====================================================================
 *    Properties:
 * ====================================================================
 */

inline INT
CGTARG_Text_Alignment (void) {
  return DEFAULT_TEXT_ALIGNMENT;
}

/* --------------------------------------------------------------------
 *    Return the inverse of <opr>. TOP_UNDEFINED is returned
 *    if there is no inverse. Note that the inverse of an
 *    instruction that takes an immediate does not also necessarily
 *    take an immediate. 
 * --------------------------------------------------------------------
 */
extern TOP CGTARG_Invert(TOP opr);

#ifdef TARG_ST
// [CL] handle inverses that involve using ops with a different number
// of arguments
extern OP* CGTARG_Invert_OP(OP* op);
#endif


/* ====================================================================
 *    Target Load/Store Properties:
 * ====================================================================
 */

#ifdef TARG_ST
extern void CGTARG_Load_From_Memory(TN *tn, ST *mem_loc, OPS *ops, VARIANT variant=V_NONE);
extern void CGTARG_Store_To_Memory(TN *tn, ST *mem_loc, OPS *ops, VARIANT variant=V_NONE);
#else
extern void CGTARG_Load_From_Memory(TN *tn, ST *mem_loc, OPS *ops);
extern void CGTARG_Store_To_Memory(TN *tn, ST *mem_loc, OPS *ops);
#endif

/*
 * Get speculative load opcode given 'op'
 */
extern TOP CGTARG_Speculative_Load (OP *op);
extern TOP CGTARG_Predicated_Store (OP *op);
extern TOP CGTARG_Predicated_Load (OP *op);

extern OP *CGTARG_Dup_OP_Predicate (OP* op, TN *new_pred);

/* ====================================================================
 *    PROC:
 * ====================================================================
 */
inline BOOL PROC_Is_Out_Of_Order(void) 
{
  return PROC_is_out_of_order();
}

inline BOOL PROC_Has_Branch_Delay_Slot(void)
{
  return PROC_has_branch_delay_slot();
}

/* ====================================================================
 *    OP: TODO -- move over to op.h, rename into OP_...
 * ====================================================================
 */
extern BOOL CGTARG_Can_Load_Immediate_In_Single_Instruction (INT64 immed);
extern BOOL CGTARG_Can_Fit_Immediate_In_Add_Instruction (INT64 immed);

#ifdef TARG_ST
// moved
#else
extern BOOL CGTARG_Is_OP_Speculative_Load(OP* memop);
extern BOOL CGTARG_Is_OP_Advanced_Load(OP* memop);
extern BOOL CGTARG_Is_OP_Check_Load(OP* memop);
extern BOOL CGTARG_Is_OP_Speculative(OP *op);
extern BOOL CGTARG_Can_Be_Speculative(OP* op);
#endif

extern BOOL CGTARG_Is_OP_Inter_RegClass_Copy(OP *op);

extern BOOL CGTARG_OP_is_counted_loop(OP *op);
extern BOOL CGTARG_Can_Change_To_Brlikely(OP *xfer_op, TOP *new_opcode);

// Get the target register number and class associated with the
// preg, if there is one that is.
extern BOOL CGTARG_Preg_Register_And_Class(WN_OFFSET preg,
					   ISA_REGISTER_CLASS *rclass,
					   REGISTER *reg);

/* ====================================================================
 *    Pick Rate Class (PRC):
 * ====================================================================
 */
typedef enum {
  PRC_INST,
  PRC_MEMREF,
  PRC_FLOP,
  PRC_FLOP_S,
  PRC_MADD,
  PRC_MADD_S,
  PRC_FADD,
  PRC_FADD_S,
  PRC_FMUL,
  PRC_FMUL_S,
  PRC_IOP,
  PRC_LAST
} PEAK_RATE_CLASS;

typedef struct {
  mINT16 refs[PRC_LAST];
} PRC_INFO;

// What is the peak rate (in ops per cycle) for the given
// class of instruction(s).
extern void CGTARG_Peak_Rate(PEAK_RATE_CLASS prc, PRC_INFO *info, INT ratio[2]);

// Print statistics for the PRC_INFO to a 'file'.
extern void CGTARG_Print_PRC_INFO (FILE *file, PRC_INFO *info, INT ii, 
				   const char *prefix, const char *suffix);

// Compute some basic information about the given 'bb'.
extern void CGTARG_Compute_PRC_INFO (BB *bb, PRC_INFO *info);

/* ====================================================================
 *                 Predication target interface
 * ====================================================================
 */

typedef enum {
  COMPARE_TYPE_unc,
  COMPARE_TYPE_or,
  COMPARE_TYPE_orcm,
  COMPARE_TYPE_and,
  COMPARE_TYPE_andcm,
  COMPARE_TYPE_or_andcm,
  COMPARE_TYPE_and_orcm,
  COMPARE_TYPE_normal
} COMPARE_TYPE;

inline BOOL CGTARG_Can_Predicate_Calls() { 
  return PROC_has_predicate_calls(); 
}
inline BOOL CGTARG_Can_Predicate_Returns() { 
  return PROC_has_predicate_returns(); 
}
inline BOOL CGTARG_Can_Predicate_Branches() { 
  return PROC_has_predicate_branches(); 
}
inline BOOL CGTARG_Can_Predicate() { 
  return PROC_is_ia64_predication(); 
}
inline BOOL CGTARG_Can_Select() { 
  return PROC_is_select(); 
}

extern TOP CGTARG_Parallel_Compare(OP* cmp_op, COMPARE_TYPE ctype);
extern BOOL CGTARG_Unconditional_Compare(OP* op, TOP* uncond_ver);
extern BOOL CGTARG_Branches_On_True(OP* br_op, OP* cmp_op);

/* ---------------------------------------------------------------------
 *    Given a compare opcode, return the unconditional variant form. 
 *    Return the opcode if there is no such form.
 * ---------------------------------------------------------------------
 */
extern TOP CGTARG_Get_unc_Variant(TOP top);

/* ====================================================================
 *   Target specific scheduling and dependence graph:
 * ====================================================================
 */

inline INT32 CGTARG_Branch_Taken_Penalty(void)
{
  return CGTARG_branch_taken_penalty_overridden ?
    CGTARG_branch_taken_penalty : 1;
}

INT32 CGTARG_Max_OP_Latency(OP *op);
#ifdef TARG_ST
INT32 CGTARG_Max_RES_Latency(OP *op, INT i);
#endif

#ifdef TARG_ST
extern BOOL CGTARG_Dependence_Required(OP *pred_op, OP *succ_op, INT16 *latency);
#else
extern BOOL CGTARG_Dependence_Required(OP *pred_op, OP *succ_op);
#endif
extern void CGTARG_Adjust_Latency(OP *pred_op, OP *succ_op, CG_DEP_KIND kind, UINT8 opnd, INT16 *latency);

#ifdef TARG_ST
// Arthur: this function is only used in cg_loop_mii.cxx, and
//         it's functionality is kind of redundant, so I just
//         get rid of it in the target description interface.
#else
extern INT  CGTARG_ARC_Sched_Latency(ARC *arc);
#endif

//  Returns TRUE if the dependence between the latency
//  between pred_op and succ_op is the load_latency
//
inline BOOL CGTARG_Use_Load_Latency(OP *pred_op, TN *tn) 
{
#if defined(TARG_IA64) || defined (TARG_ST)
  return OP_load(pred_op) && OP_result(pred_op,0) == tn;
#else
  return TRUE;
#endif
}

// Returns TRUE if OP is a suitable candidate for HBF.
extern BOOL CGTARG_Check_OP_For_HB_Suitability(OP *op);

// Handle all the Errata hazards. These are typically workarounds
// for bugs in particular versions of various target processors.
extern void CGTARG_Handle_Errata_Hazard (OP *op, INT erratnum, 
					 INT ops_to_check);

#ifdef TARG_ST200 // [CL]
extern void CGTARG_Finish_Bundle(OP                     *op, 
				 TI_BUNDLE              *bundle);
extern void CGTARG_Make_Bundles_Postpass(BB *bb);
#endif

#ifdef TARG_ST
// Perform target-specific instruction size adjustments.
extern void CGTARG_Resize_Instructions ();
// Perform pseudo MAKE expansion.
extern void CGTARG_Pseudo_Make_Expand ();
// Perform target-specific immediates adjustment.
extern void CGTARG_Fixup_Immediates ();
extern void Perform_HwLoop_Checking();
#endif

// Handle all bundle hazards.
extern void CGTARG_Handle_Bundle_Hazard(OP                     *op, 
					TI_BUNDLE              *bundle, 
					VECTOR                 *bundle_vector, 
					BOOL                   can_fill, 
					INT                    slot_pos, 
					INT                    max_pos,
					BOOL                   stop_bit_reqd,
					ISA_EXEC_UNIT_PROPERTY prop);

// Check for any extra hazards not handled in above, particular 
// hardware implementation specific ?. Shouldn't this be coordinated
// with PROC_version and ISA_subset stuff ??
#ifdef TARG_IA64
extern void Insert_Stop_Bits(BB *bb);
#else
extern void CGTARG_Insert_Stop_Bits(BB *bb);
#endif

// Checks to see if <slot> position is available for <op> using the
// property type <prop> in the <bundle>.
extern BOOL CGTARG_Bundle_Slot_Available(TI_BUNDLE              *bundle, 
					 OP                     *op, 
					 INT                    slot,
					 ISA_EXEC_UNIT_PROPERTY *prop,
					 BOOL                   stop_bit_reqd,
                                         const CG_GROUPING      *grouping);

#ifdef TARG_ST
extern void  CGTARG_Get_Info_For_Common_Base_Opt( INT *min_offset_alignment, INT *min_offset, INT *max_offset);
extern BOOL CGTARG_offset_is_extended(TN *offset, INT64 *val);
extern BOOL CGTARG_need_extended_Opcode(OP *op, TOP *etop);
extern BOOL CGTARG_is_expensive_load_imm(OP* op);
extern INT CGTARG_expensive_load_imm_immediate_index(OP* op);
extern INT CGTARG_expensive_load_imm_base_index(OP* op);
extern BOOL CGTARG_sequence_is_cheaper_than_load_imm(OPS* ops, OP* op);
extern TOP CGTARG_TOP_To_Multi(TOP top);
extern TOP CGTARG_TOP_From_Multi(TOP top);
extern BOOL CGTARG_should_factorize(OP* first, OP* last);
#endif

// Checks to see if <stop> bit is available at <slot> position in the
// bundle.
extern BOOL CGTARG_Bundle_Stop_Bit_Available(TI_BUNDLE *bundle, INT slot);

#ifdef TARG_ST
// Arthur: this function is only used in cg_loop_mii.cxx
//         It's functionality is not entirely clear to me, is it
//         necessary ?
#else
extern INT32 CGTARG_Special_Min_II(BB* loop_body, BOOL trace);
#endif

#ifdef TARG_ST200
// FdF: Code size improvement on the st220 target. Enable a sequence
// of two NOP bundles to be replaced by a GOTO .+4 instruction
extern BOOL CG_NOPs_to_GOTO;
#endif

#ifdef TARG_ST
extern UINT32 CGTARG_max_issue_width;
extern BOOL CGTARG_max_issue_width_overriden;
inline INT32 CGTARG_Max_Issue_Width(void)
{
  return CGTARG_max_issue_width;
}
#endif

/* ====================================================================
 *   Target specific tree height reduction:
 * ====================================================================
 */
#ifdef TARG_IA64
extern void CGTARG_Perform_THR_Code_Generation(OP *load_op, OP *check_load,
					       THR_TYPE type);
#else
extern void CGTARG_Perform_THR_Code_Generation(OP *load_op, THR_TYPE type);
#endif

/* ====================================================================
 *   Target specific interference for GRA:
 * ====================================================================
 */

// Prepare to make the target specific interferences.
extern void CGTARG_Interference_Initialize( INT32 cycle_count,
                                            BOOL is_loop,
                                            void (*inter_fn)(void*,void*) );
// All done presenting this set of OPs.
extern void CGTARG_Interference_Finalize(void);

// Does the current target require target specific live range
// interference at all?
extern BOOL CGTARG_Interference_Required(void);

// Present the <lrange> written by <op>.
extern void CGTARG_Result_Live_Range( void* lrange, OP* op, INT32 offset );

// Present the <lrange> used by <op> and it's <opnd> operand.
extern void CGTARG_Operand_Live_Range( void * lrange, INT opnd, OP* op,
                                       INT32  offset );

#ifdef TARG_ST
/* ====================================================================
 *   Target specific spill information:
 * ====================================================================
 */
// Count of valid entries is specified by CGTARG_NUM_SPILL_TYPES,
// that is no more constant (reconfigurability)
extern TY_IDX CGTARG_Spill_Type[MTYPE_MAX_LIMIT+1];
extern CLASS_INDEX CGTARG_Spill_Mtype[MTYPE_MAX_LIMIT+1];


// Should we save reg with regmask mechanism?
extern BOOL EETARG_Save_With_Regmask (ISA_REGISTER_CLASS cl, REGISTER reg);
#endif

#ifdef TARG_ST 	 
extern BOOL CGTARG_Is_Simple_Jump(const OP* op);
extern BOOL CGTARG_op_may_alias_with_call(const OP* op);
extern BOOL CGTARG_registerclass_may_be_copied(ISA_REGISTER_CLASS cl);
extern BOOL CGTARG_Code_Motion_To_LoopHead_Is_Legal(const OP* op, BB* loophead);
#endif

#endif /* CGTARGET_INCLUDED */
