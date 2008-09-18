#include <stdio.h>
#include <stdarg.h>

/*-------------------- CGIR Imports --------------------------------------*/
#include "cgtarget.h"

/*-------------------- LAO Target definitions --------------------*/
extern "C" {
#define this THIS
#define operator OPERATOR
#define restrict
#include "lao_interface.h"
#include "lao_target.h"
#undef operator
#undef this
}

#include "lao_cgir.h"

/*-------------------- Maps for CGIR <-> LIR Conversions ------------------*/

// Map CGIR ISA_SUBSET to LIR Processor.
static O64_Processor IS__Processor[ISA_SUBSET_COUNT_MAX];

// Map CGIR TOP to LIR Operator.
static O64_Operator *TOP__Operator;

// Map LIR Operator to CGIR TOP.
static TOP *Operator__TOP;

// Map CGIR Literal to LIR Immediate.
static O64_Immediate *LC__Immediate;

// Map CGIR ISA_REGISTER_CLASS to LIR RegFile.
// WARNING! ISA_REGISTER_CLASS reaches ISA_REGISTER_CLASS_MAX
static O64_RegFile IRC__RegFile[ISA_REGISTER_CLASS_MAX_LIMIT+1];

// Map LIR RegFile to CGIR ISA_REGISTER_CLASS.
static ISA_REGISTER_CLASS RegFile__IRC[RegFile__];

// Map LIR NativeType to CGIR TYPE_ID
static TYPE_ID NativeType__TYPE_ID[NativeType__];


/*-------------------- CGIR -> LIR Conversion Fonctions ------------------*/

// Convert ISA_SUBSET to LIR Processor.
O64_Processor
CGIR_IS_to_Processor(ISA_SUBSET is) {
  O64_Processor lao_processor;
  lao_processor = IS__Processor[is];
  Is_True(is >= ISA_SUBSET_MIN && is <= ISA_SUBSET_MAX, ("ISA_SUBSET out of range"));
  Is_True(lao_processor != Processor__UNDEF, ("Cannot map ISA_SUBSET to Processor"));
  return lao_processor;
}

// Convert CGIR ISA_LIT_CLASS to LIR Immediate.
O64_Immediate
CGIR_LC_to_Immediate(ISA_LIT_CLASS ilc) {
  O64_Immediate lao_immediate = LC__Immediate[ilc];
  Is_True(ilc >= 0 && ilc <= ISA_LC_MAX, ("ISA_LIT_CLASS out of range"));
  Is_True(lao_immediate != Immediate__UNDEF, ("Cannot map ISA_LIT_CLASS to Immediate"));
  return lao_immediate;
}

// Convert CGIR ISA_REGISTER_CLASS to LIR RegFile.
O64_RegFile
CGIR_IRC_to_RegFile(ISA_REGISTER_CLASS irc) {
  O64_RegFile lao_regFile = IRC__RegFile[irc];
  Is_True(irc >= ISA_REGISTER_CLASS_MIN && irc <= ISA_REGISTER_CLASS_MAX, ("ISA_REGISTER_CLASS out of range"));
  Is_True(lao_regFile != RegFile__UNDEF, ("Cannot map ISA_REGISTER_CLASS to RegFile"));
  return lao_regFile;
}

// Convert CGIR CLASS_REG_PAIR to LIR Register.
O64_Register
CGIR_CRP_to_Register(CLASS_REG_PAIR crp) {
  mREGISTER reg = CLASS_REG_PAIR_reg(crp);
  ISA_REGISTER_CLASS irc = CLASS_REG_PAIR_rclass(crp);
  O64_RegFile regFile = CGIR_IRC_to_RegFile(irc);
  O64_Register lowReg;
  if (irc == ISA_REGISTER_CLASS_branch) 
    lowReg = Register_st200_BR0;
  else 
    lowReg = Register_st200_GR0;
  return (O64_Register)(lowReg + (reg - 1));
}

// Convert CGIR TOP to LIR Operator.
O64_Operator
CGIR_TOP_to_Operator(TOP top) {
  O64_Operator lao_operator = TOP__Operator[top];
  Is_True(top >= 0 && top < TOP_count, ("TOPcode out of range"));
  Is_True(lao_operator != Operator__UNDEF, ("Cannot map TOPcode to Operator"));
  return lao_operator;
}

/*-------------------- LIR -> CGIR Conversion Fonctions ------------------*/

// Convert LIR RegFile to CGIR ISA_REGISTER_CLASS.
ISA_REGISTER_CLASS
RegFile_to_CGIR_IRC(O64_RegFile regFile) {
  Is_True(regFile < RegFile__, ("RegFile out of range"));
  ISA_REGISTER_CLASS irc = RegFile__IRC[regFile];
  Is_True(irc != ISA_REGISTER_CLASS_UNDEFINED,
      ("Cannot map RegFile %d to ISA_REGISTER_CLASS", regFile));
  return irc;
}

// Convert LIR Register to CGIR CLASS_REG_PAIR.
CLASS_REG_PAIR
Register_to_CGIR_CRP(O64_Register registre) {
  CLASS_REG_PAIR crp;
  O64_Register lowReg;
  ISA_REGISTER_CLASS irc;
  if (registre >= Register_st200_BR0 && registre <= Register_st200_BR7) {
    irc = ISA_REGISTER_CLASS_branch;
    lowReg = Register_st200_BR0;
  } else {
    irc = ISA_REGISTER_CLASS_integer;
    lowReg = Register_st200_GR0;
  }
  Set_CLASS_REG_PAIR(crp, irc, (registre - lowReg) + 1);
  return crp;
}

// Convert LIR Operator to TOP.
TOP
Operator_to_CGIR_TOP(O64_Operator lir_operator) {
  Is_True(lir_operator < Operator__, ("Operator out of range"));
  TOP top = Operator__TOP[lir_operator];
  Is_True(top != TOP_UNDEFINED,
      ("Cannot map Operator %d to TOP", lir_operator));
  return top;
}

// Convert LIR NativeType to CGIR TYPE_ID
TYPE_ID
NativeType_to_CGIR_TYPE_ID(O64_NativeType lir_nativeType) {
  Is_True(lir_nativeType < NativeType__, ("NativeType out of range"));
  TYPE_ID type_id = NativeType__TYPE_ID[lir_nativeType];
  Is_True(type_id != MTYPE_UNKNOWN,
      ("Cannot map nativeType %d to TYPE_ID", lir_nativeType));
  return type_id;
}

// Initialization. Once per process execution.
static int CGIR_LAO_initialized;
void
CGIR_LAO_Init(void) {
  if (CGIR_LAO_initialized++ == 0) {
    // initialize the IS__Processor array
    IS__Processor[ISA_SUBSET_st220] = Processor_st220_cpu;
    IS__Processor[ISA_SUBSET_st231] = Processor_st231_cpu;
    IS__Processor[ISA_SUBSET_st240] = Processor_st240_cpu;
    // initialize the TOP__Operator array
    TOP__Operator = TYPE_MEM_POOL_ALLOC_N(O64_Operator, Malloc_Mem_Pool, (TOP_count + 1));
    for (int i = 0; i < TOP_count; i++) TOP__Operator[i] = Operator__UNDEF;
    Is_True(TOP_UNDEFINED >= 0 && TOP_UNDEFINED <= TOP_count, ("TOP_UNDEFINED out of bounds"));
    TOP__Operator[TOP_UNDEFINED] = Operator__UNDEF;
    TOP__Operator[TOP_abss_ph_r_r] = Operator_st200_abss_ph_1general_2general;
    TOP__Operator[TOP_absubu_pb_r_r_r] = Operator_st200_absubu_pb_1general_2general_3general;
    TOP__Operator[TOP_addcg_b_r_r_b_r] = Operator_st200_addcg_1general_2branch_3general_4general_5branch;
    TOP__Operator[TOP_addf_n_r_r_r] = Operator_st200_addf_n_1nolink_2general_3general;
    TOP__Operator[TOP_addpc_i_r] = Operator_st200_addpc_1general_2isrc2;
    TOP__Operator[TOP_addpc_ii_r] = Operator_st200_addpc_1general_2xsrc2;
    TOP__Operator[TOP_addso_r_r_r] = Operator_st200_addso_1general_2general_3general;
    TOP__Operator[TOP_adds_r_r_r] = Operator_st200_adds_1general_2general_3general;
    TOP__Operator[TOP_adds_ph_r_r_r] = Operator_st200_adds_ph_1general_2general_3general;
    TOP__Operator[TOP_add_r_r_r] = Operator_st200_add_1general_2general_3general;
    TOP__Operator[TOP_add_i_r_r] = Operator_st200_add_1general_2general_3isrc2;
    TOP__Operator[TOP_add_ii_r_r] = Operator_st200_add_1general_2general_3xsrc2;
    TOP__Operator[TOP_add_ph_r_r_r] = Operator_st200_add_ph_1general_2general_3general;
    TOP__Operator[TOP_spadjust] = Operator_st200_ADJUST;
    TOP__Operator[TOP_andc_r_r_r] = Operator_st200_andc_1general_2general_3general;
    TOP__Operator[TOP_andc_i_r_r] = Operator_st200_andc_1general_2general_3isrc2;
    TOP__Operator[TOP_andc_ii_r_r] = Operator_st200_andc_1general_2general_3xsrc2;
    TOP__Operator[TOP_andl_r_r_b] = Operator_st200_andl_1branch_2general_3general;
    TOP__Operator[TOP_andl_i_r_b] = Operator_st200_andl_1branch_2general_3isrc2;
    TOP__Operator[TOP_andl_ii_r_b] = Operator_st200_andl_1branch_2general_3xsrc2;
    TOP__Operator[TOP_andl_r_r_r] = Operator_st200_andl_1general_2general_3general;
    TOP__Operator[TOP_andl_i_r_r] = Operator_st200_andl_1general_2general_3isrc2;
    TOP__Operator[TOP_andl_ii_r_r] = Operator_st200_andl_1general_2general_3xsrc2;
    TOP__Operator[TOP_andl_b_b_b] = Operator_st200_andl_bdest_1branch_2branch_3branch;
    TOP__Operator[TOP_and_r_r_r] = Operator_st200_and_1general_2general_3general;
    TOP__Operator[TOP_and_i_r_r] = Operator_st200_and_1general_2general_3isrc2;
    TOP__Operator[TOP_and_ii_r_r] = Operator_st200_and_1general_2general_3xsrc2;
    TOP__Operator[TOP_intrncall] = Operator_APPLY;
    TOP__Operator[TOP_asm_0_r_r_r] = Operator_st200_asm_0_1general_2general_3general;
    TOP__Operator[TOP_asm_10_r_r_r] = Operator_st200_asm_10_1general_2general_3general;
    TOP__Operator[TOP_asm_11_r_r_r] = Operator_st200_asm_11_1general_2general_3general;
    TOP__Operator[TOP_asm_12_r_r_r] = Operator_st200_asm_12_1general_2general_3general;
    TOP__Operator[TOP_asm_13_r_r_r] = Operator_st200_asm_13_1general_2general_3general;
    TOP__Operator[TOP_asm_14_r_r_r] = Operator_st200_asm_14_1general_2general_3general;
    TOP__Operator[TOP_asm_15_r_r_r] = Operator_st200_asm_15_1general_2general_3general;
    TOP__Operator[TOP_asm_16_i_r_r] = Operator_st200_asm_16_1general_2general_3isrc2;
    TOP__Operator[TOP_asm_16_ii_r_r] = Operator_st200_asm_16_1general_2general_3xsrc2;
    TOP__Operator[TOP_asm_17_i_r_r] = Operator_st200_asm_17_1general_2general_3isrc2;
    TOP__Operator[TOP_asm_17_ii_r_r] = Operator_st200_asm_17_1general_2general_3xsrc2;
    TOP__Operator[TOP_asm_18_i_r_r] = Operator_st200_asm_18_1general_2general_3isrc2;
    TOP__Operator[TOP_asm_18_ii_r_r] = Operator_st200_asm_18_1general_2general_3xsrc2;
    TOP__Operator[TOP_asm_19_i_r_r] = Operator_st200_asm_19_1general_2general_3isrc2;
    TOP__Operator[TOP_asm_19_ii_r_r] = Operator_st200_asm_19_1general_2general_3xsrc2;
    TOP__Operator[TOP_asm_1_r_r_r] = Operator_st200_asm_1_1general_2general_3general;
    TOP__Operator[TOP_asm_20_i_r_r] = Operator_st200_asm_20_1general_2general_3isrc2;
    TOP__Operator[TOP_asm_20_ii_r_r] = Operator_st200_asm_20_1general_2general_3xsrc2;
    TOP__Operator[TOP_asm_21_i_r_r] = Operator_st200_asm_21_1general_2general_3isrc2;
    TOP__Operator[TOP_asm_21_ii_r_r] = Operator_st200_asm_21_1general_2general_3xsrc2;
    TOP__Operator[TOP_asm_22_i_r_r] = Operator_st200_asm_22_1general_2general_3isrc2;
    TOP__Operator[TOP_asm_22_ii_r_r] = Operator_st200_asm_22_1general_2general_3xsrc2;
    TOP__Operator[TOP_asm_23_i_r_r] = Operator_st200_asm_23_1general_2general_3isrc2;
    TOP__Operator[TOP_asm_23_ii_r_r] = Operator_st200_asm_23_1general_2general_3xsrc2;
    TOP__Operator[TOP_asm_24_i_r_r] = Operator_st200_asm_24_1general_2general_3isrc2;
    TOP__Operator[TOP_asm_24_ii_r_r] = Operator_st200_asm_24_1general_2general_3xsrc2;
    TOP__Operator[TOP_asm_25_i_r_r] = Operator_st200_asm_25_1general_2general_3isrc2;
    TOP__Operator[TOP_asm_25_ii_r_r] = Operator_st200_asm_25_1general_2general_3xsrc2;
    TOP__Operator[TOP_asm_26_i_r_r] = Operator_st200_asm_26_1general_2general_3isrc2;
    TOP__Operator[TOP_asm_26_ii_r_r] = Operator_st200_asm_26_1general_2general_3xsrc2;
    TOP__Operator[TOP_asm_27_i_r_r] = Operator_st200_asm_27_1general_2general_3isrc2;
    TOP__Operator[TOP_asm_27_ii_r_r] = Operator_st200_asm_27_1general_2general_3xsrc2;
    TOP__Operator[TOP_asm_28_i_r_r] = Operator_st200_asm_28_1general_2general_3isrc2;
    TOP__Operator[TOP_asm_28_ii_r_r] = Operator_st200_asm_28_1general_2general_3xsrc2;
    TOP__Operator[TOP_asm_29_i_r_r] = Operator_st200_asm_29_1general_2general_3isrc2;
    TOP__Operator[TOP_asm_29_ii_r_r] = Operator_st200_asm_29_1general_2general_3xsrc2;
    TOP__Operator[TOP_asm_2_r_r_r] = Operator_st200_asm_2_1general_2general_3general;
    TOP__Operator[TOP_asm_30_i_r_r] = Operator_st200_asm_30_1general_2general_3isrc2;
    TOP__Operator[TOP_asm_30_ii_r_r] = Operator_st200_asm_30_1general_2general_3xsrc2;
    TOP__Operator[TOP_asm_31_i_r_r] = Operator_st200_asm_31_1general_2general_3isrc2;
    TOP__Operator[TOP_asm_31_ii_r_r] = Operator_st200_asm_31_1general_2general_3xsrc2;
    TOP__Operator[TOP_asm_3_r_r_r] = Operator_st200_asm_3_1general_2general_3general;
    TOP__Operator[TOP_asm_4_r_r_r] = Operator_st200_asm_4_1general_2general_3general;
    TOP__Operator[TOP_asm_5_r_r_r] = Operator_st200_asm_5_1general_2general_3general;
    TOP__Operator[TOP_asm_6_r_r_r] = Operator_st200_asm_6_1general_2general_3general;
    TOP__Operator[TOP_asm_7_r_r_r] = Operator_st200_asm_7_1general_2general_3general;
    TOP__Operator[TOP_asm_8_r_r_r] = Operator_st200_asm_8_1general_2general_3general;
    TOP__Operator[TOP_asm_9_r_r_r] = Operator_st200_asm_9_1general_2general_3general;
    TOP__Operator[TOP_avg4u_pb_r_r_b_r] = Operator_st200_avg4u_pb_1nolink_2branch_3general_4general;
    TOP__Operator[TOP_avgu_pb_r_r_b_r] = Operator_st200_avgu_pb_1general_2branch_3general_4general;
    TOP__Operator[TOP_begin_pregtn] = Operator_BEGIN_PREGTN;
    TOP__Operator[TOP_break] = Operator_st200_break;
    TOP__Operator[TOP_brf_i_b] = Operator_st200_brf_1branch_2btarg;
    TOP__Operator[TOP_br_i_b] = Operator_st200_br_1branch_2btarg;
    TOP__Operator[TOP_bswap_r_r] = Operator_st200_bswap_1general_2general;
    TOP__Operator[TOP_bwd_bar] = Operator_BWDBAR;
    TOP__Operator[TOP_call_i] = Operator_st200_call_1btarg;
    TOP__Operator[TOP_clz_r_r] = Operator_st200_clz_1general_2general;
    TOP__Operator[TOP_cmpeqf_n_r_r_b] = Operator_st200_cmpeqf_n_1branch_2general_3general;
    TOP__Operator[TOP_cmpeqf_n_r_r_r] = Operator_st200_cmpeqf_n_1general_2general_3general;
    TOP__Operator[TOP_cmpeq_r_r_b] = Operator_st200_cmpeq_1branch_2general_3general;
    TOP__Operator[TOP_cmpeq_i_r_b] = Operator_st200_cmpeq_1branch_2general_3isrc2;
    TOP__Operator[TOP_cmpeq_ii_r_b] = Operator_st200_cmpeq_1branch_2general_3xsrc2;
    TOP__Operator[TOP_cmpeq_r_r_r] = Operator_st200_cmpeq_1general_2general_3general;
    TOP__Operator[TOP_cmpeq_i_r_r] = Operator_st200_cmpeq_1general_2general_3isrc2;
    TOP__Operator[TOP_cmpeq_ii_r_r] = Operator_st200_cmpeq_1general_2general_3xsrc2;
    TOP__Operator[TOP_cmpeq_pb_r_r_r] = Operator_st200_cmpeq_pb_1general_2general_3general;
    TOP__Operator[TOP_cmpeq_pb_r_r_b] = Operator_st200_cmpeq_pb_bdest_1branch_2general_3general;
    TOP__Operator[TOP_cmpeq_ph_r_r_r] = Operator_st200_cmpeq_ph_1general_2general_3general;
    TOP__Operator[TOP_cmpeq_ph_r_r_b] = Operator_st200_cmpeq_ph_bdest_1branch_2general_3general;
    TOP__Operator[TOP_cmpgef_n_r_r_b] = Operator_st200_cmpgef_n_1branch_2general_3general;
    TOP__Operator[TOP_cmpgef_n_r_r_r] = Operator_st200_cmpgef_n_1general_2general_3general;
    TOP__Operator[TOP_cmpgeu_r_r_b] = Operator_st200_cmpgeu_1branch_2general_3general;
    TOP__Operator[TOP_cmpgeu_i_r_b] = Operator_st200_cmpgeu_1branch_2general_3isrc2;
    TOP__Operator[TOP_cmpgeu_ii_r_b] = Operator_st200_cmpgeu_1branch_2general_3xsrc2;
    TOP__Operator[TOP_cmpgeu_r_r_r] = Operator_st200_cmpgeu_1general_2general_3general;
    TOP__Operator[TOP_cmpgeu_i_r_r] = Operator_st200_cmpgeu_1general_2general_3isrc2;
    TOP__Operator[TOP_cmpgeu_ii_r_r] = Operator_st200_cmpgeu_1general_2general_3xsrc2;
    TOP__Operator[TOP_cmpge_r_r_b] = Operator_st200_cmpge_1branch_2general_3general;
    TOP__Operator[TOP_cmpge_i_r_b] = Operator_st200_cmpge_1branch_2general_3isrc2;
    TOP__Operator[TOP_cmpge_ii_r_b] = Operator_st200_cmpge_1branch_2general_3xsrc2;
    TOP__Operator[TOP_cmpge_r_r_r] = Operator_st200_cmpge_1general_2general_3general;
    TOP__Operator[TOP_cmpge_i_r_r] = Operator_st200_cmpge_1general_2general_3isrc2;
    TOP__Operator[TOP_cmpge_ii_r_r] = Operator_st200_cmpge_1general_2general_3xsrc2;
    TOP__Operator[TOP_cmpgtf_n_r_r_b] = Operator_st200_cmpgtf_n_1branch_2general_3general;
    TOP__Operator[TOP_cmpgtf_n_r_r_r] = Operator_st200_cmpgtf_n_1general_2general_3general;
    TOP__Operator[TOP_cmpgtu_r_r_b] = Operator_st200_cmpgtu_1branch_2general_3general;
    TOP__Operator[TOP_cmpgtu_i_r_b] = Operator_st200_cmpgtu_1branch_2general_3isrc2;
    TOP__Operator[TOP_cmpgtu_ii_r_b] = Operator_st200_cmpgtu_1branch_2general_3xsrc2;
    TOP__Operator[TOP_cmpgtu_r_r_r] = Operator_st200_cmpgtu_1general_2general_3general;
    TOP__Operator[TOP_cmpgtu_i_r_r] = Operator_st200_cmpgtu_1general_2general_3isrc2;
    TOP__Operator[TOP_cmpgtu_ii_r_r] = Operator_st200_cmpgtu_1general_2general_3xsrc2;
    TOP__Operator[TOP_cmpgtu_pb_r_r_r] = Operator_st200_cmpgtu_pb_1general_2general_3general;
    TOP__Operator[TOP_cmpgtu_pb_r_r_b] = Operator_st200_cmpgtu_pb_bdest_1branch_2general_3general;
    TOP__Operator[TOP_cmpgt_r_r_b] = Operator_st200_cmpgt_1branch_2general_3general;
    TOP__Operator[TOP_cmpgt_i_r_b] = Operator_st200_cmpgt_1branch_2general_3isrc2;
    TOP__Operator[TOP_cmpgt_ii_r_b] = Operator_st200_cmpgt_1branch_2general_3xsrc2;
    TOP__Operator[TOP_cmpgt_r_r_r] = Operator_st200_cmpgt_1general_2general_3general;
    TOP__Operator[TOP_cmpgt_i_r_r] = Operator_st200_cmpgt_1general_2general_3isrc2;
    TOP__Operator[TOP_cmpgt_ii_r_r] = Operator_st200_cmpgt_1general_2general_3xsrc2;
    TOP__Operator[TOP_cmpgt_ph_r_r_r] = Operator_st200_cmpgt_ph_1general_2general_3general;
    TOP__Operator[TOP_cmpgt_ph_r_r_b] = Operator_st200_cmpgt_ph_bdest_1branch_2general_3general;
    TOP__Operator[TOP_cmplef_n_r_r_b] = Operator_st200_cmplef_n_1branch_2general_3general;
    TOP__Operator[TOP_cmplef_n_r_r_r] = Operator_st200_cmplef_n_1general_2general_3general;
    TOP__Operator[TOP_cmpleu_r_r_b] = Operator_st200_cmpleu_1branch_2general_3general;
    TOP__Operator[TOP_cmpleu_i_r_b] = Operator_st200_cmpleu_1branch_2general_3isrc2;
    TOP__Operator[TOP_cmpleu_ii_r_b] = Operator_st200_cmpleu_1branch_2general_3xsrc2;
    TOP__Operator[TOP_cmpleu_r_r_r] = Operator_st200_cmpleu_1general_2general_3general;
    TOP__Operator[TOP_cmpleu_i_r_r] = Operator_st200_cmpleu_1general_2general_3isrc2;
    TOP__Operator[TOP_cmpleu_ii_r_r] = Operator_st200_cmpleu_1general_2general_3xsrc2;
    TOP__Operator[TOP_cmple_r_r_b] = Operator_st200_cmple_1branch_2general_3general;
    TOP__Operator[TOP_cmple_i_r_b] = Operator_st200_cmple_1branch_2general_3isrc2;
    TOP__Operator[TOP_cmple_ii_r_b] = Operator_st200_cmple_1branch_2general_3xsrc2;
    TOP__Operator[TOP_cmple_r_r_r] = Operator_st200_cmple_1general_2general_3general;
    TOP__Operator[TOP_cmple_i_r_r] = Operator_st200_cmple_1general_2general_3isrc2;
    TOP__Operator[TOP_cmple_ii_r_r] = Operator_st200_cmple_1general_2general_3xsrc2;
    TOP__Operator[TOP_cmpltf_n_r_r_b] = Operator_st200_cmpltf_n_1branch_2general_3general;
    TOP__Operator[TOP_cmpltf_n_r_r_r] = Operator_st200_cmpltf_n_1general_2general_3general;
    TOP__Operator[TOP_cmpltu_r_r_b] = Operator_st200_cmpltu_1branch_2general_3general;
    TOP__Operator[TOP_cmpltu_i_r_b] = Operator_st200_cmpltu_1branch_2general_3isrc2;
    TOP__Operator[TOP_cmpltu_ii_r_b] = Operator_st200_cmpltu_1branch_2general_3xsrc2;
    TOP__Operator[TOP_cmpltu_r_r_r] = Operator_st200_cmpltu_1general_2general_3general;
    TOP__Operator[TOP_cmpltu_i_r_r] = Operator_st200_cmpltu_1general_2general_3isrc2;
    TOP__Operator[TOP_cmpltu_ii_r_r] = Operator_st200_cmpltu_1general_2general_3xsrc2;
    TOP__Operator[TOP_cmplt_r_r_b] = Operator_st200_cmplt_1branch_2general_3general;
    TOP__Operator[TOP_cmplt_i_r_b] = Operator_st200_cmplt_1branch_2general_3isrc2;
    TOP__Operator[TOP_cmplt_ii_r_b] = Operator_st200_cmplt_1branch_2general_3xsrc2;
    TOP__Operator[TOP_cmplt_r_r_r] = Operator_st200_cmplt_1general_2general_3general;
    TOP__Operator[TOP_cmplt_i_r_r] = Operator_st200_cmplt_1general_2general_3isrc2;
    TOP__Operator[TOP_cmplt_ii_r_r] = Operator_st200_cmplt_1general_2general_3xsrc2;
    TOP__Operator[TOP_cmpne_r_r_b] = Operator_st200_cmpne_1branch_2general_3general;
    TOP__Operator[TOP_cmpne_i_r_b] = Operator_st200_cmpne_1branch_2general_3isrc2;
    TOP__Operator[TOP_cmpne_ii_r_b] = Operator_st200_cmpne_1branch_2general_3xsrc2;
    TOP__Operator[TOP_cmpne_r_r_r] = Operator_st200_cmpne_1general_2general_3general;
    TOP__Operator[TOP_cmpne_i_r_r] = Operator_st200_cmpne_1general_2general_3isrc2;
    TOP__Operator[TOP_cmpne_ii_r_r] = Operator_st200_cmpne_1general_2general_3xsrc2;
    TOP__Operator[TOP_composep] = Operator_st200_COMPOSEP;
    TOP__Operator[TOP_convbi_b_r] = Operator_st200_convbi_1general_2branch;
    TOP__Operator[TOP_convfi_n_r_r] = Operator_st200_convfi_n_1nolink_2general;
    TOP__Operator[TOP_convib_r_b] = Operator_st200_convib_1branch_2general;
    TOP__Operator[TOP_convif_n_r_r] = Operator_st200_convif_n_1nolink_2general;
    TOP__Operator[TOP_COPY] = Operator_COPY;
    TOP__Operator[TOP_dbgsbrk] = Operator_st200_dbgsbrk;
    TOP__Operator[TOP_dib] = Operator_st200_dib;
    TOP__Operator[TOP_divs_b_r_r_b_r] = Operator_st200_divs_1general_2branch_3general_4general_5branch;
    TOP__Operator[TOP_divu_r_r_r] = Operator_st200_divu_1nolink_2general_3general;
    TOP__Operator[TOP_div_r_r_r] = Operator_st200_div_1nolink_2general_3general;
    TOP__Operator[TOP_end_pregtn] = Operator_END_PREGTN;
    TOP__Operator[TOP_ext1_pb_r_r_r] = Operator_st200_ext1_pb_1general_2general_3general;
    TOP__Operator[TOP_ext2_pb_r_r_r] = Operator_st200_ext2_pb_1general_2general_3general;
    TOP__Operator[TOP_ext3_pb_r_r_r] = Operator_st200_ext3_pb_1general_2general_3general;
    TOP__Operator[TOP_extl_pb_r_r_b_r] = Operator_st200_extl_pb_1general_2branch_3general_4general;
    TOP__Operator[TOP_extractlu_i_r_r] = Operator_st200_extractlu_1general_2general_3isrc2;
    TOP__Operator[TOP_extractlu_ii_r_r] = Operator_st200_extractlu_1general_2general_3xsrc2;
    TOP__Operator[TOP_extractl_i_r_r] = Operator_st200_extractl_1general_2general_3isrc2;
    TOP__Operator[TOP_extractl_ii_r_r] = Operator_st200_extractl_1general_2general_3xsrc2;
    TOP__Operator[TOP_extractp] = Operator_st200_EXTRACTP;
    TOP__Operator[TOP_extractu_i_r_r] = Operator_st200_extractu_1general_2general_3isrc2;
    TOP__Operator[TOP_extractu_ii_r_r] = Operator_st200_extractu_1general_2general_3xsrc2;
    TOP__Operator[TOP_extract_i_r_r] = Operator_st200_extract_1general_2general_3isrc2;
    TOP__Operator[TOP_extract_ii_r_r] = Operator_st200_extract_1general_2general_3xsrc2;
    TOP__Operator[TOP_extr_pb_r_r_b_r] = Operator_st200_extr_pb_1general_2branch_3general_4general;
    TOP__Operator[TOP_flushadd_r_i] = Operator_st200_flushadd_1isrc2_2general;
    TOP__Operator[TOP_flushadd_r_ii] = Operator_st200_flushadd_1xsrc2_2general;
    TOP__Operator[TOP_flushadd_l1_r_i] = Operator_st200_flushadd_l1_1isrc2_2general;
    TOP__Operator[TOP_flushadd_l1_r_ii] = Operator_st200_flushadd_l1_1xsrc2_2general;
    TOP__Operator[TOP_fwd_bar] = Operator_FWDBAR;
    TOP__Operator[TOP_getpc] = Operator_st200_GETPC;
    TOP__Operator[TOP_asm] = Operator_st200_GNUASM;
    TOP__Operator[TOP_goto_i] = Operator_st200_goto_1btarg;
    TOP__Operator[TOP_icall] = Operator_st200_icall;
    TOP__Operator[TOP_idle] = Operator_st200_idle;
    TOP__Operator[TOP_ifixup] = Operator_IFIXUP;
    TOP__Operator[TOP_igoto] = Operator_st200_igoto;
    TOP__Operator[TOP_invadd_r_i] = Operator_st200_invadd_1isrc2_2general;
    TOP__Operator[TOP_invadd_r_ii] = Operator_st200_invadd_1xsrc2_2general;
    TOP__Operator[TOP_invadd_l1_r_i] = Operator_st200_invadd_l1_1isrc2_2general;
    TOP__Operator[TOP_invadd_l1_r_ii] = Operator_st200_invadd_l1_1xsrc2_2general;
    TOP__Operator[TOP_KILL] = Operator_KILL;
    TOP__Operator[TOP_label] = Operator_LABEL;
    TOP__Operator[TOP_ldbc_r_i_b_r] = Operator_st200_ldbc_1nolink_2predicate_3isrc2_4general;
    TOP__Operator[TOP_ldbc_r_ii_b_r] = Operator_st200_ldbc_1nolink_2predicate_3xsrc2_4general;
    TOP__Operator[TOP_ldbuc_r_i_b_r] = Operator_st200_ldbuc_1nolink_2predicate_3isrc2_4general;
    TOP__Operator[TOP_ldbuc_r_ii_b_r] = Operator_st200_ldbuc_1nolink_2predicate_3xsrc2_4general;
    TOP__Operator[TOP_ldbu_r_i_r] = Operator_st200_ldbu_1nolink_2isrc2_3general;
    TOP__Operator[TOP_ldbu_r_ii_r] = Operator_st200_ldbu_1nolink_2xsrc2_3general;
    TOP__Operator[TOP_ldbu_d_r_i_r] = Operator_st200_ldbu_d_1nolink_2isrc2_3general;
    TOP__Operator[TOP_ldbu_d_r_ii_r] = Operator_st200_ldbu_d_1nolink_2xsrc2_3general;
    TOP__Operator[TOP_ldb_r_i_r] = Operator_st200_ldb_1nolink_2isrc2_3general;
    TOP__Operator[TOP_ldb_r_ii_r] = Operator_st200_ldb_1nolink_2xsrc2_3general;
    TOP__Operator[TOP_ldb_d_r_i_r] = Operator_st200_ldb_d_1nolink_2isrc2_3general;
    TOP__Operator[TOP_ldb_d_r_ii_r] = Operator_st200_ldb_d_1nolink_2xsrc2_3general;
    TOP__Operator[TOP_ldhc_r_i_b_r] = Operator_st200_ldhc_1nolink_2predicate_3isrc2_4general;
    TOP__Operator[TOP_ldhc_r_ii_b_r] = Operator_st200_ldhc_1nolink_2predicate_3xsrc2_4general;
    TOP__Operator[TOP_ldhuc_r_i_b_r] = Operator_st200_ldhuc_1nolink_2predicate_3isrc2_4general;
    TOP__Operator[TOP_ldhuc_r_ii_b_r] = Operator_st200_ldhuc_1nolink_2predicate_3xsrc2_4general;
    TOP__Operator[TOP_ldhu_r_i_r] = Operator_st200_ldhu_1nolink_2isrc2_3general;
    TOP__Operator[TOP_ldhu_r_ii_r] = Operator_st200_ldhu_1nolink_2xsrc2_3general;
    TOP__Operator[TOP_ldhu_d_r_i_r] = Operator_st200_ldhu_d_1nolink_2isrc2_3general;
    TOP__Operator[TOP_ldhu_d_r_ii_r] = Operator_st200_ldhu_d_1nolink_2xsrc2_3general;
    TOP__Operator[TOP_ldh_r_i_r] = Operator_st200_ldh_1nolink_2isrc2_3general;
    TOP__Operator[TOP_ldh_r_ii_r] = Operator_st200_ldh_1nolink_2xsrc2_3general;
    TOP__Operator[TOP_ldh_d_r_i_r] = Operator_st200_ldh_d_1nolink_2isrc2_3general;
    TOP__Operator[TOP_ldh_d_r_ii_r] = Operator_st200_ldh_d_1nolink_2xsrc2_3general;
    TOP__Operator[TOP_ldlc_r_i_b_p] = Operator_st200_ldlc_1nzpaired_2predicate_3isrc2_4general;
    TOP__Operator[TOP_ldlc_r_ii_b_p] = Operator_st200_ldlc_1nzpaired_2predicate_3xsrc2_4general;
    TOP__Operator[TOP_ldl_r_i_p] = Operator_st200_ldl_1nzpaired_2isrc2_3general;
    TOP__Operator[TOP_ldl_r_ii_p] = Operator_st200_ldl_1nzpaired_2xsrc2_3general;
    TOP__Operator[TOP_ldwc_r_i_b_r] = Operator_st200_ldwc_1general_2predicate_3isrc2_4general;
    TOP__Operator[TOP_ldwc_r_ii_b_r] = Operator_st200_ldwc_1general_2predicate_3xsrc2_4general;
    TOP__Operator[TOP_ldwl_r_r] = Operator_st200_ldwl_1general_2general;
    TOP__Operator[TOP_ldw_r_i_r] = Operator_st200_ldw_1general_2isrc2_3general;
    TOP__Operator[TOP_ldw_r_ii_r] = Operator_st200_ldw_1general_2xsrc2_3general;
    TOP__Operator[TOP_ldw_d_r_i_r] = Operator_st200_ldw_d_1general_2isrc2_3general;
    TOP__Operator[TOP_ldw_d_r_ii_r] = Operator_st200_ldw_d_1general_2xsrc2_3general;
    TOP__Operator[TOP_maxu_r_r_r] = Operator_st200_maxu_1general_2general_3general;
    TOP__Operator[TOP_maxu_i_r_r] = Operator_st200_maxu_1general_2general_3isrc2;
    TOP__Operator[TOP_maxu_ii_r_r] = Operator_st200_maxu_1general_2general_3xsrc2;
    TOP__Operator[TOP_max_r_r_r] = Operator_st200_max_1general_2general_3general;
    TOP__Operator[TOP_max_i_r_r] = Operator_st200_max_1general_2general_3isrc2;
    TOP__Operator[TOP_max_ii_r_r] = Operator_st200_max_1general_2general_3xsrc2;
    TOP__Operator[TOP_max_ph_r_r_r] = Operator_st200_max_ph_1general_2general_3general;
    TOP__Operator[TOP_mfb_b_r] = Operator_st200_mfb_1general_2branch;
    TOP__Operator[TOP_minu_r_r_r] = Operator_st200_minu_1general_2general_3general;
    TOP__Operator[TOP_minu_i_r_r] = Operator_st200_minu_1general_2general_3isrc2;
    TOP__Operator[TOP_minu_ii_r_r] = Operator_st200_minu_1general_2general_3xsrc2;
    TOP__Operator[TOP_min_r_r_r] = Operator_st200_min_1general_2general_3general;
    TOP__Operator[TOP_min_i_r_r] = Operator_st200_min_1general_2general_3isrc2;
    TOP__Operator[TOP_min_ii_r_r] = Operator_st200_min_1general_2general_3xsrc2;
    TOP__Operator[TOP_min_ph_r_r_r] = Operator_st200_min_ph_1general_2general_3general;
    TOP__Operator[TOP_movc] = Operator_st200_MOVC;
    TOP__Operator[TOP_movcf] = Operator_st200_MOVCF;
    TOP__Operator[TOP_movp] = Operator_st200_MOVP;
    TOP__Operator[TOP_mov_r_r] = Operator_st200_mov_1general_2general;
    TOP__Operator[TOP_mov_i_r] = Operator_st200_mov_1general_2isrc2;
    TOP__Operator[TOP_mov_ii_r] = Operator_st200_mov_1general_2xsrc2;
    TOP__Operator[TOP_mov_r_b] = Operator_st200_mov_bdest_1branch_2general;
    TOP__Operator[TOP_st240_mov_r_b] = Operator_st240_mov_bdest_1branch_2general;
    TOP__Operator[TOP_mov_b_r] = Operator_st200_mov_bsrc_1general_2branch;
    TOP__Operator[TOP_st240_mov_b_r] = Operator_st240_mov_bsrc_1general_2branch;
    TOP__Operator[TOP_mov_b_b] = Operator_st200_mov_bsrc_bdest_1branch_2branch;
    TOP__Operator[TOP_mtb_r_b] = Operator_st200_mtb_1branch_2general;
    TOP__Operator[TOP_mul32_r_r_r] = Operator_st200_mul32_1nolink_2general_3general;
    TOP__Operator[TOP_mul32_i_r_r] = Operator_st200_mul32_1nolink_2general_3isrc2;
    TOP__Operator[TOP_mul32_ii_r_r] = Operator_st200_mul32_1nolink_2general_3xsrc2;
    TOP__Operator[TOP_mul64hu_r_r_r] = Operator_st200_mul64hu_1nolink_2general_3general;
    TOP__Operator[TOP_mul64hu_i_r_r] = Operator_st200_mul64hu_1nolink_2general_3isrc2;
    TOP__Operator[TOP_mul64hu_ii_r_r] = Operator_st200_mul64hu_1nolink_2general_3xsrc2;
    TOP__Operator[TOP_mul64h_r_r_r] = Operator_st200_mul64h_1nolink_2general_3general;
    TOP__Operator[TOP_mul64h_i_r_r] = Operator_st200_mul64h_1nolink_2general_3isrc2;
    TOP__Operator[TOP_mul64h_ii_r_r] = Operator_st200_mul64h_1nolink_2general_3xsrc2;
    TOP__Operator[TOP_muladdus_pb_r_r_r] = Operator_st200_muladdus_pb_1nolink_2general_3general;
    TOP__Operator[TOP_muladd_ph_r_r_r] = Operator_st200_muladd_ph_1nolink_2general_3general;
    TOP__Operator[TOP_mulfracadds_ph_r_r_r] = Operator_st200_mulfracadds_ph_1nolink_2general_3general;
    TOP__Operator[TOP_mulfracrm_ph_r_r_r] = Operator_st200_mulfracrm_ph_1nolink_2general_3general;
    TOP__Operator[TOP_mulfracrne_ph_r_r_r] = Operator_st200_mulfracrne_ph_1nolink_2general_3general;
    TOP__Operator[TOP_mulfrac_r_r_r] = Operator_st200_mulfrac_1nolink_2general_3general;
    TOP__Operator[TOP_mulfrac_i_r_r] = Operator_st200_mulfrac_1nolink_2general_3isrc2;
    TOP__Operator[TOP_mulfrac_ii_r_r] = Operator_st200_mulfrac_1nolink_2general_3xsrc2;
    TOP__Operator[TOP_mulf_n_r_r_r] = Operator_st200_mulf_n_1nolink_2general_3general;
    TOP__Operator[TOP_mulhhs_r_r_r] = Operator_st200_mulhhs_1nolink_2general_3general;
    TOP__Operator[TOP_mulhhs_i_r_r] = Operator_st200_mulhhs_1nolink_2general_3isrc2;
    TOP__Operator[TOP_mulhhs_ii_r_r] = Operator_st200_mulhhs_1nolink_2general_3xsrc2;
    TOP__Operator[TOP_mulhhu_r_r_r] = Operator_st200_mulhhu_1nolink_2general_3general;
    TOP__Operator[TOP_mulhhu_i_r_r] = Operator_st200_mulhhu_1nolink_2general_3isrc2;
    TOP__Operator[TOP_mulhhu_ii_r_r] = Operator_st200_mulhhu_1nolink_2general_3xsrc2;
    TOP__Operator[TOP_mulhh_r_r_r] = Operator_st200_mulhh_1nolink_2general_3general;
    TOP__Operator[TOP_mulhh_i_r_r] = Operator_st200_mulhh_1nolink_2general_3isrc2;
    TOP__Operator[TOP_mulhh_ii_r_r] = Operator_st200_mulhh_1nolink_2general_3xsrc2;
    TOP__Operator[TOP_mulhs_r_r_r] = Operator_st200_mulhs_1nolink_2general_3general;
    TOP__Operator[TOP_mulhs_i_r_r] = Operator_st200_mulhs_1nolink_2general_3isrc2;
    TOP__Operator[TOP_mulhs_ii_r_r] = Operator_st200_mulhs_1nolink_2general_3xsrc2;
    TOP__Operator[TOP_mulhu_r_r_r] = Operator_st200_mulhu_1nolink_2general_3general;
    TOP__Operator[TOP_mulhu_i_r_r] = Operator_st200_mulhu_1nolink_2general_3isrc2;
    TOP__Operator[TOP_mulhu_ii_r_r] = Operator_st200_mulhu_1nolink_2general_3xsrc2;
    TOP__Operator[TOP_mulh_r_r_r] = Operator_st200_mulh_1nolink_2general_3general;
    TOP__Operator[TOP_mulh_i_r_r] = Operator_st200_mulh_1nolink_2general_3isrc2;
    TOP__Operator[TOP_mulh_ii_r_r] = Operator_st200_mulh_1nolink_2general_3xsrc2;
    TOP__Operator[TOP_mullhus_r_r_r] = Operator_st200_mullhus_1nolink_2general_3general;
    TOP__Operator[TOP_mullhus_i_r_r] = Operator_st200_mullhus_1nolink_2general_3isrc2;
    TOP__Operator[TOP_mullhus_ii_r_r] = Operator_st200_mullhus_1nolink_2general_3xsrc2;
    TOP__Operator[TOP_mullhu_r_r_r] = Operator_st200_mullhu_1nolink_2general_3general;
    TOP__Operator[TOP_mullhu_i_r_r] = Operator_st200_mullhu_1nolink_2general_3isrc2;
    TOP__Operator[TOP_mullhu_ii_r_r] = Operator_st200_mullhu_1nolink_2general_3xsrc2;
    TOP__Operator[TOP_mullh_r_r_r] = Operator_st200_mullh_1nolink_2general_3general;
    TOP__Operator[TOP_mullh_i_r_r] = Operator_st200_mullh_1nolink_2general_3isrc2;
    TOP__Operator[TOP_mullh_ii_r_r] = Operator_st200_mullh_1nolink_2general_3xsrc2;
    TOP__Operator[TOP_mulllu_r_r_r] = Operator_st200_mulllu_1nolink_2general_3general;
    TOP__Operator[TOP_mulllu_i_r_r] = Operator_st200_mulllu_1nolink_2general_3isrc2;
    TOP__Operator[TOP_mulllu_ii_r_r] = Operator_st200_mulllu_1nolink_2general_3xsrc2;
    TOP__Operator[TOP_mulll_r_r_r] = Operator_st200_mulll_1nolink_2general_3general;
    TOP__Operator[TOP_mulll_i_r_r] = Operator_st200_mulll_1nolink_2general_3isrc2;
    TOP__Operator[TOP_mulll_ii_r_r] = Operator_st200_mulll_1nolink_2general_3xsrc2;
    TOP__Operator[TOP_mullu_r_r_r] = Operator_st200_mullu_1nolink_2general_3general;
    TOP__Operator[TOP_mullu_i_r_r] = Operator_st200_mullu_1nolink_2general_3isrc2;
    TOP__Operator[TOP_mullu_ii_r_r] = Operator_st200_mullu_1nolink_2general_3xsrc2;
    TOP__Operator[TOP_mull_r_r_r] = Operator_st200_mull_1nolink_2general_3general;
    TOP__Operator[TOP_mull_i_r_r] = Operator_st200_mull_1nolink_2general_3isrc2;
    TOP__Operator[TOP_mull_ii_r_r] = Operator_st200_mull_1nolink_2general_3xsrc2;
    TOP__Operator[TOP_st240_mull_ii_r_r] = Operator_st240_mull_1nolink_2general_3xsrc2;
    TOP__Operator[TOP_multi_ldlc_r_i_b_r] = Operator_st200_MULTI_ldlc_1nzpaired_2predicate_3isrc2_4general;
    TOP__Operator[TOP_multi_ldlc_r_ii_b_r] = Operator_st200_MULTI_ldlc_1nzpaired_2predicate_3xsrc2_4general;
    TOP__Operator[TOP_multi_ldl_r_i_r] = Operator_st200_MULTI_ldl_1nzpaired_2isrc2_3general;
    TOP__Operator[TOP_multi_ldl_r_ii_r] = Operator_st200_MULTI_ldl_1nzpaired_2xsrc2_3general;
    TOP__Operator[TOP_multi_stlc_r_b_r_i] = Operator_st200_MULTI_stlc_1isrc2_2general_3predicate_4paired;
    TOP__Operator[TOP_multi_stlc_r_b_r_ii] = Operator_st200_MULTI_stlc_1xsrc2_2general_3predicate_4paired;
    TOP__Operator[TOP_multi_stl_r_r_i] = Operator_st200_MULTI_stl_1isrc2_2general_3paired;
    TOP__Operator[TOP_multi_stl_r_r_ii] = Operator_st200_MULTI_stl_1xsrc2_2general_3paired;
    TOP__Operator[TOP_mul_ph_r_r_r] = Operator_st200_mul_ph_1nolink_2general_3general;
    TOP__Operator[TOP_nandl_r_r_b] = Operator_st200_nandl_1branch_2general_3general;
    TOP__Operator[TOP_nandl_i_r_b] = Operator_st200_nandl_1branch_2general_3isrc2;
    TOP__Operator[TOP_nandl_ii_r_b] = Operator_st200_nandl_1branch_2general_3xsrc2;
    TOP__Operator[TOP_nandl_r_r_r] = Operator_st200_nandl_1general_2general_3general;
    TOP__Operator[TOP_nandl_i_r_r] = Operator_st200_nandl_1general_2general_3isrc2;
    TOP__Operator[TOP_nandl_ii_r_r] = Operator_st200_nandl_1general_2general_3xsrc2;
    TOP__Operator[TOP_nandl_b_b_b] = Operator_st200_nandl_bdest_1branch_2branch_3branch;
    TOP__Operator[TOP_nop] = Operator_st200_nop;
    TOP__Operator[TOP_norl_r_r_b] = Operator_st200_norl_1branch_2general_3general;
    TOP__Operator[TOP_norl_i_r_b] = Operator_st200_norl_1branch_2general_3isrc2;
    TOP__Operator[TOP_norl_ii_r_b] = Operator_st200_norl_1branch_2general_3xsrc2;
    TOP__Operator[TOP_norl_r_r_r] = Operator_st200_norl_1general_2general_3general;
    TOP__Operator[TOP_norl_i_r_r] = Operator_st200_norl_1general_2general_3isrc2;
    TOP__Operator[TOP_norl_ii_r_r] = Operator_st200_norl_1general_2general_3xsrc2;
    TOP__Operator[TOP_norl_b_b_b] = Operator_st200_norl_bdest_1branch_2branch_3branch;
    TOP__Operator[TOP_orc_r_r_r] = Operator_st200_orc_1general_2general_3general;
    TOP__Operator[TOP_orc_i_r_r] = Operator_st200_orc_1general_2general_3isrc2;
    TOP__Operator[TOP_orc_ii_r_r] = Operator_st200_orc_1general_2general_3xsrc2;
    TOP__Operator[TOP_orl_r_r_b] = Operator_st200_orl_1branch_2general_3general;
    TOP__Operator[TOP_orl_i_r_b] = Operator_st200_orl_1branch_2general_3isrc2;
    TOP__Operator[TOP_orl_ii_r_b] = Operator_st200_orl_1branch_2general_3xsrc2;
    TOP__Operator[TOP_orl_r_r_r] = Operator_st200_orl_1general_2general_3general;
    TOP__Operator[TOP_orl_i_r_r] = Operator_st200_orl_1general_2general_3isrc2;
    TOP__Operator[TOP_orl_ii_r_r] = Operator_st200_orl_1general_2general_3xsrc2;
    TOP__Operator[TOP_orl_b_b_b] = Operator_st200_orl_bdest_1branch_2branch_3branch;
    TOP__Operator[TOP_or_r_r_r] = Operator_st200_or_1general_2general_3general;
    TOP__Operator[TOP_or_i_r_r] = Operator_st200_or_1general_2general_3isrc2;
    TOP__Operator[TOP_or_ii_r_r] = Operator_st200_or_1general_2general_3xsrc2;
    TOP__Operator[TOP_packrnp_phh_r_r_r] = Operator_st200_packrnp_phh_1general_2general_3general;
    TOP__Operator[TOP_packsu_pb_r_r_r] = Operator_st200_packsu_pb_1general_2general_3general;
    TOP__Operator[TOP_packs_ph_r_r_r] = Operator_st200_packs_ph_1general_2general_3general;
    TOP__Operator[TOP_pack_pb_r_r_r] = Operator_st200_pack_pb_1general_2general_3general;
    TOP__Operator[TOP_pack_ph_r_r_r] = Operator_st200_pack_ph_1general_2general_3general;
    TOP__Operator[TOP_perm_pb_r_r_r] = Operator_st200_perm_pb_1general_2general_3general;
    TOP__Operator[TOP_perm_pb_i_r_r] = Operator_st200_perm_pb_1general_2general_3isrc2;
    TOP__Operator[TOP_perm_pb_ii_r_r] = Operator_st200_perm_pb_1general_2general_3xsrc2;
    TOP__Operator[TOP_pftc_r_i_b] = Operator_st200_pftc_1predicate_2isrc2_3general;
    TOP__Operator[TOP_pftc_r_ii_b] = Operator_st200_pftc_1predicate_2xsrc2_3general;
    TOP__Operator[TOP_pft_r_i] = Operator_st200_pft_1isrc2_2general;
    TOP__Operator[TOP_pft_r_ii] = Operator_st200_pft_1xsrc2_2general;
    TOP__Operator[TOP_phi] = Operator_PHI;
    TOP__Operator[TOP_prgadd_r_i] = Operator_st200_prgadd_1isrc2_2general;
    TOP__Operator[TOP_prgadd_r_ii] = Operator_st200_prgadd_1xsrc2_2general;
    TOP__Operator[TOP_prgadd_l1_r_i] = Operator_st200_prgadd_l1_1isrc2_2general;
    TOP__Operator[TOP_prgadd_l1_r_ii] = Operator_st200_prgadd_l1_1xsrc2_2general;
    TOP__Operator[TOP_prgins] = Operator_st200_prgins;
    TOP__Operator[TOP_prginsadd_r_i] = Operator_st200_prginsadd_1isrc2_2general;
    TOP__Operator[TOP_prginsadd_r_ii] = Operator_st200_prginsadd_1xsrc2_2general;
    TOP__Operator[TOP_prginsadd_l1_r_i] = Operator_st200_prginsadd_l1_1isrc2_2general;
    TOP__Operator[TOP_prginsadd_l1_r_ii] = Operator_st200_prginsadd_l1_1xsrc2_2general;
    TOP__Operator[TOP_prginspg_r_i] = Operator_st200_prginspg_1isrc2_2general;
    TOP__Operator[TOP_prginspg_r_ii] = Operator_st200_prginspg_1xsrc2_2general;
    TOP__Operator[TOP_prginsset_r_i] = Operator_st200_prginsset_1isrc2_2general;
    TOP__Operator[TOP_prginsset_r_ii] = Operator_st200_prginsset_1xsrc2_2general;
    TOP__Operator[TOP_prginsset_l1_r_i] = Operator_st200_prginsset_l1_1isrc2_2general;
    TOP__Operator[TOP_prginsset_l1_r_ii] = Operator_st200_prginsset_l1_1xsrc2_2general;
    TOP__Operator[TOP_prgset_r_i] = Operator_st200_prgset_1isrc2_2general;
    TOP__Operator[TOP_prgset_r_ii] = Operator_st200_prgset_1xsrc2_2general;
    TOP__Operator[TOP_prgset_l1_r_i] = Operator_st200_prgset_l1_1isrc2_2general;
    TOP__Operator[TOP_prgset_l1_r_ii] = Operator_st200_prgset_l1_1xsrc2_2general;
    TOP__Operator[TOP_psi] = Operator_PSI;
    TOP__Operator[TOP_pswclr_r] = Operator_st200_pswclr_1general;
    TOP__Operator[TOP_pswmask_i_r_r] = Operator_st200_pswmask_1nolink_2general_3isrc2;
    TOP__Operator[TOP_pswmask_ii_r_r] = Operator_st200_pswmask_1nolink_2general_3xsrc2;
    TOP__Operator[TOP_pswset_r] = Operator_st200_pswset_1general;
    TOP__Operator[TOP_pushregs] = Operator_st200_PUSHREGS;
    TOP__Operator[TOP_remu_r_r_r] = Operator_st200_remu_1nolink_2general_3general;
    TOP__Operator[TOP_rem_r_r_r] = Operator_st200_rem_1nolink_2general_3general;
    TOP__Operator[TOP_retention] = Operator_st200_retention;
    TOP__Operator[TOP_return] = Operator_st200_return;
    TOP__Operator[TOP_rfi] = Operator_st200_rfi;
    TOP__Operator[TOP_rotl_r_r_r] = Operator_st200_rotl_1general_2general_3general;
    TOP__Operator[TOP_rotl_i_r_r] = Operator_st200_rotl_1general_2general_3isrc2;
    TOP__Operator[TOP_rotl_ii_r_r] = Operator_st200_rotl_1general_2general_3xsrc2;
    TOP__Operator[TOP_sadu_pb_r_r_r] = Operator_st200_sadu_pb_1nolink_2general_3general;
    TOP__Operator[TOP_satso_r_r] = Operator_st200_satso_1general_2general;
    TOP__Operator[TOP_sats_r_r] = Operator_st200_sats_1general_2general;
    TOP__Operator[TOP_sbrk_i] = Operator_st200_sbrk;
    TOP__Operator[TOP_st240_sbrk_i] = Operator_st240_sbrk;
    TOP__Operator[TOP_sh1addso_r_r_r] = Operator_st200_sh1addso_1general_2general_3general;
    TOP__Operator[TOP_sh1adds_r_r_r] = Operator_st200_sh1adds_1general_2general_3general;
    TOP__Operator[TOP_sh1add_r_r_r] = Operator_st200_sh1add_1general_2general_3general;
    TOP__Operator[TOP_sh1add_i_r_r] = Operator_st200_sh1add_1general_2general_3isrc2;
    TOP__Operator[TOP_sh1add_ii_r_r] = Operator_st200_sh1add_1general_2general_3xsrc2;
    TOP__Operator[TOP_sh1subso_r_r_r] = Operator_st200_sh1subso_1general_2general_3general;
    TOP__Operator[TOP_sh1subs_r_r_r] = Operator_st200_sh1subs_1general_2general_3general;
    TOP__Operator[TOP_sh2add_r_r_r] = Operator_st200_sh2add_1general_2general_3general;
    TOP__Operator[TOP_sh2add_i_r_r] = Operator_st200_sh2add_1general_2general_3isrc2;
    TOP__Operator[TOP_sh2add_ii_r_r] = Operator_st200_sh2add_1general_2general_3xsrc2;
    TOP__Operator[TOP_sh3add_r_r_r] = Operator_st200_sh3add_1general_2general_3general;
    TOP__Operator[TOP_sh3add_i_r_r] = Operator_st200_sh3add_1general_2general_3isrc2;
    TOP__Operator[TOP_sh3add_ii_r_r] = Operator_st200_sh3add_1general_2general_3xsrc2;
    TOP__Operator[TOP_sh4add_r_r_r] = Operator_st200_sh4add_1general_2general_3general;
    TOP__Operator[TOP_sh4add_i_r_r] = Operator_st200_sh4add_1general_2general_3isrc2;
    TOP__Operator[TOP_sh4add_ii_r_r] = Operator_st200_sh4add_1general_2general_3xsrc2;
    TOP__Operator[TOP_shlso_r_r_r] = Operator_st200_shlso_1nolink_2general_3general;
    TOP__Operator[TOP_shlso_i_r_r] = Operator_st200_shlso_1nolink_2general_3isrc2;
    TOP__Operator[TOP_shlso_ii_r_r] = Operator_st200_shlso_1nolink_2general_3xsrc2;
    TOP__Operator[TOP_shls_r_r_r] = Operator_st200_shls_1nolink_2general_3general;
    TOP__Operator[TOP_shls_i_r_r] = Operator_st200_shls_1nolink_2general_3isrc2;
    TOP__Operator[TOP_shls_ii_r_r] = Operator_st200_shls_1nolink_2general_3xsrc2;
    TOP__Operator[TOP_shls_ph_r_r_r] = Operator_st200_shls_ph_1nolink_2general_3general;
    TOP__Operator[TOP_shls_ph_i_r_r] = Operator_st200_shls_ph_1nolink_2general_3isrc2;
    TOP__Operator[TOP_shls_ph_ii_r_r] = Operator_st200_shls_ph_1nolink_2general_3xsrc2;
    TOP__Operator[TOP_shl_r_r_r] = Operator_st200_shl_1general_2general_3general;
    TOP__Operator[TOP_shl_i_r_r] = Operator_st200_shl_1general_2general_3isrc2;
    TOP__Operator[TOP_shl_ii_r_r] = Operator_st200_shl_1general_2general_3xsrc2;
    TOP__Operator[TOP_shl_ph_r_r_r] = Operator_st200_shl_ph_1general_2general_3general;
    TOP__Operator[TOP_shl_ph_i_r_r] = Operator_st200_shl_ph_1general_2general_3isrc2;
    TOP__Operator[TOP_shl_ph_ii_r_r] = Operator_st200_shl_ph_1general_2general_3xsrc2;
    TOP__Operator[TOP_shrrne_ph_r_r_r] = Operator_st200_shrrne_ph_1nolink_2general_3general;
    TOP__Operator[TOP_shrrne_ph_i_r_r] = Operator_st200_shrrne_ph_1nolink_2general_3isrc2;
    TOP__Operator[TOP_shrrne_ph_ii_r_r] = Operator_st200_shrrne_ph_1nolink_2general_3xsrc2;
    TOP__Operator[TOP_shrrnp_i_r_r] = Operator_st200_shrrnp_1nolink_2general_3isrc2;
    TOP__Operator[TOP_shrrnp_ii_r_r] = Operator_st200_shrrnp_1nolink_2general_3xsrc2;
    TOP__Operator[TOP_shrrnp_ph_r_r_r] = Operator_st200_shrrnp_ph_1nolink_2general_3general;
    TOP__Operator[TOP_shrrnp_ph_i_r_r] = Operator_st200_shrrnp_ph_1nolink_2general_3isrc2;
    TOP__Operator[TOP_shrrnp_ph_ii_r_r] = Operator_st200_shrrnp_ph_1nolink_2general_3xsrc2;
    TOP__Operator[TOP_shru_r_r_r] = Operator_st200_shru_1general_2general_3general;
    TOP__Operator[TOP_shru_i_r_r] = Operator_st200_shru_1general_2general_3isrc2;
    TOP__Operator[TOP_shru_ii_r_r] = Operator_st200_shru_1general_2general_3xsrc2;
    TOP__Operator[TOP_shr_r_r_r] = Operator_st200_shr_1general_2general_3general;
    TOP__Operator[TOP_shr_i_r_r] = Operator_st200_shr_1general_2general_3isrc2;
    TOP__Operator[TOP_shr_ii_r_r] = Operator_st200_shr_1general_2general_3xsrc2;
    TOP__Operator[TOP_shr_ph_r_r_r] = Operator_st200_shr_ph_1general_2general_3general;
    TOP__Operator[TOP_shr_ph_i_r_r] = Operator_st200_shr_ph_1general_2general_3isrc2;
    TOP__Operator[TOP_shr_ph_ii_r_r] = Operator_st200_shr_ph_1general_2general_3xsrc2;
    TOP__Operator[TOP_shuffeve_pb_r_r_r] = Operator_st200_shuffeve_pb_1general_2general_3general;
    TOP__Operator[TOP_shuffodd_pb_r_r_r] = Operator_st200_shuffodd_pb_1general_2general_3general;
    TOP__Operator[TOP_shuff_pbh_r_r_r] = Operator_st200_shuff_pbh_1general_2general_3general;
    TOP__Operator[TOP_shuff_pbl_r_r_r] = Operator_st200_shuff_pbl_1general_2general_3general;
    TOP__Operator[TOP_shuff_phh_r_r_r] = Operator_st200_shuff_phh_1general_2general_3general;
    TOP__Operator[TOP_shuff_phl_r_r_r] = Operator_st200_shuff_phl_1general_2general_3general;
    TOP__Operator[TOP_SIGMA] = Operator_SIGMA;
    TOP__Operator[TOP_noop] = Operator_SKIP;
    TOP__Operator[TOP_slctf_r_r_b_r] = Operator_st200_slctf_1general_2branch_3general_4general;
    TOP__Operator[TOP_slctf_i_r_b_r] = Operator_st200_slctf_1general_2branch_3general_4isrc2;
    TOP__Operator[TOP_slctf_ii_r_b_r] = Operator_st200_slctf_1general_2branch_3general_4xsrc2;
    TOP__Operator[TOP_slctf_pb_r_r_b_r] = Operator_st200_slctf_pb_1general_2branch_3general_4general;
    TOP__Operator[TOP_slctf_pb_i_r_b_r] = Operator_st200_slctf_pb_1general_2branch_3general_4isrc2;
    TOP__Operator[TOP_slctf_pb_ii_r_b_r] = Operator_st200_slctf_pb_1general_2branch_3general_4xsrc2;
    TOP__Operator[TOP_slct_r_r_b_r] = Operator_st200_slct_1general_2branch_3general_4general;
    TOP__Operator[TOP_slct_i_r_b_r] = Operator_st200_slct_1general_2branch_3general_4isrc2;
    TOP__Operator[TOP_slct_ii_r_b_r] = Operator_st200_slct_1general_2branch_3general_4xsrc2;
    TOP__Operator[TOP_slct_pb_r_r_b_r] = Operator_st200_slct_pb_1general_2branch_3general_4general;
    TOP__Operator[TOP_slct_pb_i_r_b_r] = Operator_st200_slct_pb_1general_2branch_3general_4isrc2;
    TOP__Operator[TOP_slct_pb_ii_r_b_r] = Operator_st200_slct_pb_1general_2branch_3general_4xsrc2;
    TOP__Operator[TOP_st240_slct_r_r_b_r] = Operator_st200_slct_rsrc_1general_2branch_3general_4general;
    TOP__Operator[TOP_stbc_r_b_r_i] = Operator_st200_stbc_1isrc2_2general_3predicate_4general;
    TOP__Operator[TOP_stbc_r_b_r_ii] = Operator_st200_stbc_1xsrc2_2general_3predicate_4general;
    TOP__Operator[TOP_stb_r_r_i] = Operator_st200_stb_1isrc2_2general_3general;
    TOP__Operator[TOP_stb_r_r_ii] = Operator_st200_stb_1xsrc2_2general_3general;
    TOP__Operator[TOP_sthc_r_b_r_i] = Operator_st200_sthc_1isrc2_2general_3predicate_4general;
    TOP__Operator[TOP_sthc_r_b_r_ii] = Operator_st200_sthc_1xsrc2_2general_3predicate_4general;
    TOP__Operator[TOP_sth_r_r_i] = Operator_st200_sth_1isrc2_2general_3general;
    TOP__Operator[TOP_sth_r_r_ii] = Operator_st200_sth_1xsrc2_2general_3general;
    TOP__Operator[TOP_stlc_p_b_r_i] = Operator_st200_stlc_1isrc2_2general_3predicate_4paired;
    TOP__Operator[TOP_stlc_p_b_r_ii] = Operator_st200_stlc_1xsrc2_2general_3predicate_4paired;
    TOP__Operator[TOP_stl_p_r_i] = Operator_st200_stl_1isrc2_2general_3paired;
    TOP__Operator[TOP_stl_p_r_ii] = Operator_st200_stl_1xsrc2_2general_3paired;
    TOP__Operator[TOP_stwc_r_b_r_i] = Operator_st200_stwc_1isrc2_2general_3predicate_4general;
    TOP__Operator[TOP_stwc_r_b_r_ii] = Operator_st200_stwc_1xsrc2_2general_3predicate_4general;
    TOP__Operator[TOP_stwl_r_r_b] = Operator_st200_stwl_1branch_2general_3general;
    TOP__Operator[TOP_stw_r_r_i] = Operator_st200_stw_1isrc2_2general_3general;
    TOP__Operator[TOP_stw_r_r_ii] = Operator_st200_stw_1xsrc2_2general_3general;
    TOP__Operator[TOP_subf_n_r_r_r] = Operator_st200_subf_n_1nolink_2general_3general;
    TOP__Operator[TOP_subso_r_r_r] = Operator_st200_subso_1general_2general_3general;
    TOP__Operator[TOP_subs_r_r_r] = Operator_st200_subs_1general_2general_3general;
    TOP__Operator[TOP_subs_ph_r_r_r] = Operator_st200_subs_ph_1general_2general_3general;
    TOP__Operator[TOP_sub_r_r_r] = Operator_st200_sub_1general_2general_3general;
    TOP__Operator[TOP_sub_r_i_r] = Operator_st200_sub_1general_2isrc2_3general;
    TOP__Operator[TOP_sub_r_ii_r] = Operator_st200_sub_1general_2xsrc2_3general;
    TOP__Operator[TOP_sub_ph_r_r_r] = Operator_st200_sub_ph_1general_2general_3general;
    TOP__Operator[TOP_sxtb_r_r] = Operator_st200_sxtb_1general_2general;
    TOP__Operator[TOP_sxth_r_r] = Operator_st200_sxth_1general_2general;
    TOP__Operator[TOP_sxt_r_r_r] = Operator_st200_sxt_1general_2general_3general;
    TOP__Operator[TOP_sxt_i_r_r] = Operator_st200_sxt_1general_2general_3isrc2;
    TOP__Operator[TOP_sxt_ii_r_r] = Operator_st200_sxt_1general_2general_3xsrc2;
    TOP__Operator[TOP_sync] = Operator_st200_sync;
    TOP__Operator[TOP_syncins] = Operator_st200_syncins;
    TOP__Operator[TOP_syscall_i] = Operator_st200_syscall;
    TOP__Operator[TOP_st240_syscall_i] = Operator_st240_syscall;
    TOP__Operator[TOP_unpacku_pbh_r_r] = Operator_st200_unpacku_pbh_1general_2general;
    TOP__Operator[TOP_unpacku_pbl_r_r] = Operator_st200_unpacku_pbl_1general_2general;
    TOP__Operator[TOP_waitl] = Operator_st200_waitl;
    TOP__Operator[TOP_wmb] = Operator_st200_wmb;
    TOP__Operator[TOP_xor_r_r_r] = Operator_st200_xor_1general_2general_3general;
    TOP__Operator[TOP_xor_i_r_r] = Operator_st200_xor_1general_2general_3isrc2;
    TOP__Operator[TOP_xor_ii_r_r] = Operator_st200_xor_1general_2general_3xsrc2;
    TOP__Operator[TOP_zxtb_r_r] = Operator_st200_zxtb_1general_2general;
    TOP__Operator[TOP_zxth_r_r] = Operator_st200_zxth_1general_2general;
    TOP__Operator[TOP_zxt_r_r_r] = Operator_st200_zxt_1general_2general_3general;
    TOP__Operator[TOP_zxt_i_r_r] = Operator_st200_zxt_1general_2general_3isrc2;
    TOP__Operator[TOP_zxt_ii_r_r] = Operator_st200_zxt_1general_2general_3xsrc2;
    // initialize Operator__TOP;
    Operator__TOP = TYPE_MEM_POOL_ALLOC_N(TOP, Malloc_Mem_Pool, (Operator__));
    for (int i = 0; i < Operator__; i++) Operator__TOP[i] = TOP_UNDEFINED;
    Operator__TOP[Operator_GOTO] = TOP_goto_i;	// FIXME in LAO_PRO!
    //
    for (int i = 0; i < TOP_count; i++) {
      if (TOP__Operator[i] < 0 || TOP__Operator[i] >= Operator__);
      else Operator__TOP[TOP__Operator[i]] = (TOP)i;
    }
    // initialize LC__Immediate
    LC__Immediate =  TYPE_MEM_POOL_ALLOC_N(O64_Immediate, Malloc_Mem_Pool, ISA_LC_MAX+1);
    for (int i = 0; i <= ISA_LC_MAX; i++) LC__Immediate[i] = Immediate__UNDEF;
    Is_True(ISA_LC_UNDEFINED == 0, ("ISA_LC_UNDEFINED != 0. Unedxpected."));
    LC__Immediate[ISA_LC_UNDEFINED] = Immediate__UNDEF;
    LC__Immediate[LC_btarg] = Immediate_st200_btarg;
    LC__Immediate[LC_isrc2] = Immediate_st200_isrc2;
    LC__Immediate[LC_imm] = Immediate_st200_imm;
    LC__Immediate[LC_sbrknum] = Immediate_st200_sbrknum;
    LC__Immediate[LC_xsrc2] = Immediate_st200_xsrc2;
    LC__Immediate[LC_brknum] = Immediate_st200_brknum;
    // initialize IRC__RegFile
    for (int i = 0; i <= ISA_REGISTER_CLASS_MAX; i++) IRC__RegFile[i] = RegFile__UNDEF;
    IRC__RegFile[ISA_REGISTER_CLASS_integer] = RegFile_st200_GR;
    IRC__RegFile[ISA_REGISTER_CLASS_branch] = RegFile_st200_BR;
    // initialize RegFile__IRC
    for (int i = 0; i < RegFile__; i++) RegFile__IRC[i] = ISA_REGISTER_CLASS_UNDEFINED;
    for (int i = 0; i <= ISA_REGISTER_CLASS_MAX; i++) {
      if (IRC__RegFile[i] < 0 || IRC__RegFile[i] >= RegFile__);
      else RegFile__IRC[IRC__RegFile[i]] = (ISA_REGISTER_CLASS)i;
    }
    // initialize the NativeType__TYPE_ID array
    for (int i = 0; i < NativeType__; i++) {
      NativeType__TYPE_ID[i] = MTYPE_UNKNOWN;
    }
    NativeType__TYPE_ID[NativeType_st200_Int8] = MTYPE_I1;
    NativeType__TYPE_ID[NativeType_st200_UInt8] = MTYPE_U1;
    NativeType__TYPE_ID[NativeType_st200_Int16] = MTYPE_I2;
    NativeType__TYPE_ID[NativeType_st200_UInt16] = MTYPE_U2;
    NativeType__TYPE_ID[NativeType_st200_Int32] = MTYPE_I4;
    NativeType__TYPE_ID[NativeType_st200_UInt32] = MTYPE_U4;
    NativeType__TYPE_ID[NativeType_st200_Int64] = MTYPE_I8;
    NativeType__TYPE_ID[NativeType_st200_UInt64] = MTYPE_U8;
    NativeType__TYPE_ID[NativeType_st200_Float32] = MTYPE_F4;
    NativeType__TYPE_ID[NativeType_st200_Float64] = MTYPE_F8;
  }
}

void
CGIR_LAO_Fini(void) {
}
