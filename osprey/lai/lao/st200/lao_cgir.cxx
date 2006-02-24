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
static LAI_Processor IS__Processor[ISA_SUBSET_MAX+1];

// Map CGIR TOP to LIR Operator.
static LAI_Operator TOP__Operator[TOP_UNDEFINED];

// Map LIR Operator to CGIR TOP.
static TOP Operator__TOP[Operator__];

// Map CGIR ISA_ENUM_CLASS to LIR Modifier.
static LAI_Modifier IEC__Modifier[ISA_EC_MAX];

// Map CGIR Literal to LIR Immediate.
static LAI_Immediate LC__Immediate[ISA_LC_MAX];

// Map CGIR ISA_REGISTER_CLASS to LIR RegFile.
// WARNING! ISA_REGISTER_CLASS reaches ISA_REGISTER_CLASS_MAX
static LAI_RegFile IRC__RegFile[ISA_REGISTER_CLASS_MAX+1];

// Map LIR RegFile to CGIR ISA_REGISTER_CLASS.
static ISA_REGISTER_CLASS RegFile__IRC[RegFile__];

// Map LIR NativeType to CGIR TYPE_ID
static TYPE_ID NativeType__TYPE_ID[NativeType__];


/*-------------------- CGIR -> LIR Conversion Fonctions ------------------*/

// Convert ISA_SUBSET to LIR Processor.
LAI_Processor
CGIR_IS_to_Processor(ISA_SUBSET is) {
  LAI_Processor lao_processor;
  lao_processor = IS__Processor[is];
  Is_True(is >= 0 && is <= ISA_SUBSET_MAX, ("ISA_SUBSET out of range"));
  Is_True(lao_processor != Processor__, ("Cannot map ISA_SUBSET to Processor"));
  return lao_processor;
}

// Convert CGIR ISA_ENUM_CLASS to LIR Modifier.
LAI_Modifier
CGIR_IEC_to_Modifier(ISA_ENUM_CLASS iec) {
  LAI_Modifier lao_modifier = IEC__Modifier[iec];
  Is_True(iec >= 0 && iec < ISA_EC_MAX, ("ISA_ENUM_CLASS out of range"));
  Is_True(lao_modifier != Modifier__, ("Cannot map ISA_ENUM_CLASS to Modifier"));
  return lao_modifier;
}

// Convert CGIR ISA_LIT_CLASS to LIR Immediate.
LAI_Immediate
CGIR_LC_to_Immediate(ISA_LIT_CLASS ilc) {
  LAI_Immediate lao_immediate = LC__Immediate[ilc];
  Is_True(ilc >= 0 && ilc < ISA_LC_MAX, ("ISA_LIT_CLASS out of range"));
  Is_True(lao_immediate != Immediate__, ("Cannot map ISA_LIT_CLASS to Immediate"));
  return lao_immediate;
}

// Convert CGIR ISA_REGISTER_CLASS to LIR RegFile.
LAI_RegFile
CGIR_IRC_to_RegFile(ISA_REGISTER_CLASS irc) {
  LAI_RegFile lao_regFile = IRC__RegFile[irc];
  Is_True(irc >= 0 && irc <= ISA_REGISTER_CLASS_MAX, ("ISA_REGISTER_CLASS out of range"));
  Is_True(lao_regFile != RegFile__, ("Cannot map ISA_REGISTER_CLASS to RegFile"));
  return lao_regFile;
}

// Convert CGIR CLASS_REG_PAIR to LIR Register.
LAI_Register
CGIR_CRP_to_Register(CLASS_REG_PAIR crp) {
  mREGISTER reg = CLASS_REG_PAIR_reg(crp);
  ISA_REGISTER_CLASS irc = CLASS_REG_PAIR_rclass(crp);
  LAI_RegFile regFile = CGIR_IRC_to_RegFile(irc);
  LAI_Register lowReg;
  if (irc == ISA_REGISTER_CLASS_branch) 
    lowReg = Register_st200_BR0;
  else 
    lowReg = Register_st200_GR0;
  return (LAI_Register)(lowReg + (reg - 1));
}

// Convert CGIR TOP to LIR Operator.
LAI_Operator
CGIR_TOP_to_Operator(TOP top) {
  LAI_Operator lao_operator = TOP__Operator[top];
  Is_True(top >= 0 && top < TOP_UNDEFINED, ("TOPcode out of range"));
  Is_True(lao_operator != Operator__, ("Cannot map TOPcode to Operator"));
  return lao_operator;
}

/*-------------------- LIR -> CGIR Conversion Fonctions ------------------*/

// Convert LIR RegFile to CGIR ISA_REGISTER_CLASS.
ISA_REGISTER_CLASS
RegFile_to_CGIR_IRC(LAI_RegFile regFile) {
  Is_True(regFile < RegFile__, ("RegFile out of range"));
  ISA_REGISTER_CLASS irc = RegFile__IRC[regFile];
  Is_True(irc != ISA_REGISTER_CLASS_UNDEFINED,
      ("Cannot map RegFile %d to ISA_REGISTER_CLASS", regFile));
  return irc;
}

// Convert LIR Register to CGIR CLASS_REG_PAIR.
CLASS_REG_PAIR
Register_to_CGIR_CRP(LAI_Register registre) {
  CLASS_REG_PAIR crp;
  LAI_Register lowReg;
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
Operator_to_CGIR_TOP(LAI_Operator lir_operator) {
  Is_True(lir_operator < Operator__, ("Operator out of range"));
  TOP top = Operator__TOP[lir_operator];
  Is_True(top != TOP_UNDEFINED,
      ("Cannot map Operator %d to TOP", lir_operator));
  return top;
}

// Convert LIR NativeType to CGIR TYPE_ID
TYPE_ID
NativeType_to_CGIR_TYPE_ID(LAI_NativeType lir_nativeType) {
  Is_True(lir_nativeType < NativeType__, ("NativeType out of range"));
  TYPE_ID type_id = NativeType__TYPE_ID[lir_nativeType];
  Is_True(type_id != MTYPE_UNKNOWN,
      ("Cannot map nativeType %d to TYPE_ID", lir_nativeType));
  return type_id;
}

// Initialization. Once per process execution.
static int CGIR_LAI_initialized;
void
CGIR_LAI_Init(void) {
  if (CGIR_LAI_initialized++ == 0) {
    // initialize the IS__Processor array
    IS__Processor[0] = Processor__;
    IS__Processor[ISA_SUBSET_MAX] = Processor__;
    IS__Processor[ISA_SUBSET_st220] = Processor_st220_cpu;
    IS__Processor[ISA_SUBSET_st231] = Processor_st231_cpu;
    IS__Processor[ISA_SUBSET_st235] = Processor_st235_cpu;
    // initialize the TOP__Operator array
    for (int i = 0; i < TOP_UNDEFINED; i++) TOP__Operator[i] = Operator__;
    TOP__Operator[TOP_addcg] = Operator_st200_addcg_general_branch_general_general_branch;
    TOP__Operator[TOP_addf_n] = Operator_st200_addf_n_nolink_general_general;
    TOP__Operator[TOP_addpc_i] = Operator_st200_addpc_general_isrc2;
    TOP__Operator[TOP_addpc_ii] = Operator_st200_addpc_general_xsrc2;
    TOP__Operator[TOP_add_r] = Operator_st200_add_general_general_general;
    TOP__Operator[TOP_add_i] = Operator_st200_add_general_general_isrc2;
    TOP__Operator[TOP_add_ii] = Operator_st200_add_general_general_xsrc2;
    TOP__Operator[TOP_spadjust] = Operator_st200_ADJUST;
    TOP__Operator[TOP_andc_r] = Operator_st200_andc_general_general_general;
    TOP__Operator[TOP_andc_i] = Operator_st200_andc_general_general_isrc2;
    TOP__Operator[TOP_andc_ii] = Operator_st200_andc_general_general_xsrc2;
    TOP__Operator[TOP_andl_r_b] = Operator_st200_andl_branch_general_general;
    TOP__Operator[TOP_andl_i_b] = Operator_st200_andl_branch_general_isrc2;
    TOP__Operator[TOP_andl_ii_b] = Operator_st200_andl_branch_general_xsrc2;
    TOP__Operator[TOP_andl_r_r] = Operator_st200_andl_general_general_general;
    TOP__Operator[TOP_andl_i_r] = Operator_st200_andl_general_general_isrc2;
    TOP__Operator[TOP_andl_ii_r] = Operator_st200_andl_general_general_xsrc2;
    TOP__Operator[TOP_and_r] = Operator_st200_and_general_general_general;
    TOP__Operator[TOP_and_i] = Operator_st200_and_general_general_isrc2;
    TOP__Operator[TOP_and_ii] = Operator_st200_and_general_general_xsrc2;
    TOP__Operator[TOP_asm_0] = Operator_st200_asm_0_general_general_general;
    TOP__Operator[TOP_asm_10] = Operator_st200_asm_10_general_general_general;
    TOP__Operator[TOP_asm_11] = Operator_st200_asm_11_general_general_general;
    TOP__Operator[TOP_asm_12] = Operator_st200_asm_12_general_general_general;
    TOP__Operator[TOP_asm_13] = Operator_st200_asm_13_general_general_general;
    TOP__Operator[TOP_asm_14] = Operator_st200_asm_14_general_general_general;
    TOP__Operator[TOP_asm_15] = Operator_st200_asm_15_general_general_general;
    TOP__Operator[TOP_asm_16_i] = Operator_st200_asm_16_general_general_isrc2;
    TOP__Operator[TOP_asm_16_ii] = Operator_st200_asm_16_general_general_xsrc2;
    TOP__Operator[TOP_asm_17_i] = Operator_st200_asm_17_general_general_isrc2;
    TOP__Operator[TOP_asm_17_ii] = Operator_st200_asm_17_general_general_xsrc2;
    TOP__Operator[TOP_asm_18_i] = Operator_st200_asm_18_general_general_isrc2;
    TOP__Operator[TOP_asm_18_ii] = Operator_st200_asm_18_general_general_xsrc2;
    TOP__Operator[TOP_asm_19_i] = Operator_st200_asm_19_general_general_isrc2;
    TOP__Operator[TOP_asm_19_ii] = Operator_st200_asm_19_general_general_xsrc2;
    TOP__Operator[TOP_asm_1] = Operator_st200_asm_1_general_general_general;
    TOP__Operator[TOP_asm_20_i] = Operator_st200_asm_20_general_general_isrc2;
    TOP__Operator[TOP_asm_20_ii] = Operator_st200_asm_20_general_general_xsrc2;
    TOP__Operator[TOP_asm_21_i] = Operator_st200_asm_21_general_general_isrc2;
    TOP__Operator[TOP_asm_21_ii] = Operator_st200_asm_21_general_general_xsrc2;
    TOP__Operator[TOP_asm_22_i] = Operator_st200_asm_22_general_general_isrc2;
    TOP__Operator[TOP_asm_22_ii] = Operator_st200_asm_22_general_general_xsrc2;
    TOP__Operator[TOP_asm_23_i] = Operator_st200_asm_23_general_general_isrc2;
    TOP__Operator[TOP_asm_23_ii] = Operator_st200_asm_23_general_general_xsrc2;
    TOP__Operator[TOP_asm_24_i] = Operator_st200_asm_24_general_general_isrc2;
    TOP__Operator[TOP_asm_24_ii] = Operator_st200_asm_24_general_general_xsrc2;
    TOP__Operator[TOP_asm_25_i] = Operator_st200_asm_25_general_general_isrc2;
    TOP__Operator[TOP_asm_25_ii] = Operator_st200_asm_25_general_general_xsrc2;
    TOP__Operator[TOP_asm_26_i] = Operator_st200_asm_26_general_general_isrc2;
    TOP__Operator[TOP_asm_26_ii] = Operator_st200_asm_26_general_general_xsrc2;
    TOP__Operator[TOP_asm_27_i] = Operator_st200_asm_27_general_general_isrc2;
    TOP__Operator[TOP_asm_27_ii] = Operator_st200_asm_27_general_general_xsrc2;
    TOP__Operator[TOP_asm_28_i] = Operator_st200_asm_28_general_general_isrc2;
    TOP__Operator[TOP_asm_28_ii] = Operator_st200_asm_28_general_general_xsrc2;
    TOP__Operator[TOP_asm_29_i] = Operator_st200_asm_29_general_general_isrc2;
    TOP__Operator[TOP_asm_29_ii] = Operator_st200_asm_29_general_general_xsrc2;
    TOP__Operator[TOP_asm_2] = Operator_st200_asm_2_general_general_general;
    TOP__Operator[TOP_asm_30_i] = Operator_st200_asm_30_general_general_isrc2;
    TOP__Operator[TOP_asm_30_ii] = Operator_st200_asm_30_general_general_xsrc2;
    TOP__Operator[TOP_asm_31_i] = Operator_st200_asm_31_general_general_isrc2;
    TOP__Operator[TOP_asm_31_ii] = Operator_st200_asm_31_general_general_xsrc2;
    TOP__Operator[TOP_asm_3] = Operator_st200_asm_3_general_general_general;
    TOP__Operator[TOP_asm_4] = Operator_st200_asm_4_general_general_general;
    TOP__Operator[TOP_asm_5] = Operator_st200_asm_5_general_general_general;
    TOP__Operator[TOP_asm_6] = Operator_st200_asm_6_general_general_general;
    TOP__Operator[TOP_asm_7] = Operator_st200_asm_7_general_general_general;
    TOP__Operator[TOP_asm_8] = Operator_st200_asm_8_general_general_general;
    TOP__Operator[TOP_asm_9] = Operator_st200_asm_9_general_general_general;
    TOP__Operator[TOP_begin_pregtn] = Operator_BEGIN_PREGTN;
    TOP__Operator[TOP_break] = Operator_st200_break;
    TOP__Operator[TOP_brf] = Operator_st200_brf_branch_btarg;
    TOP__Operator[TOP_br] = Operator_st200_br_branch_btarg;
    TOP__Operator[TOP_bswap] = Operator_st200_bswap_general_general;
    TOP__Operator[TOP_bwd_bar] = Operator_BWDBAR;
    TOP__Operator[TOP_CALL] = Operator_CALL;
    TOP__Operator[TOP_call] = Operator_st200_call_link_btarg;
    TOP__Operator[TOP_clz] = Operator_st200_clz_general_general;
    TOP__Operator[TOP_cmpeqf_n_b] = Operator_st200_cmpeqf_n_branch_general_general;
    TOP__Operator[TOP_cmpeqf_n_r] = Operator_st200_cmpeqf_n_general_general_general;
    TOP__Operator[TOP_cmpeq_r_b] = Operator_st200_cmpeq_branch_general_general;
    TOP__Operator[TOP_cmpeq_i_b] = Operator_st200_cmpeq_branch_general_isrc2;
    TOP__Operator[TOP_cmpeq_ii_b] = Operator_st200_cmpeq_branch_general_xsrc2;
    TOP__Operator[TOP_cmpeq_r_r] = Operator_st200_cmpeq_general_general_general;
    TOP__Operator[TOP_cmpeq_i_r] = Operator_st200_cmpeq_general_general_isrc2;
    TOP__Operator[TOP_cmpeq_ii_r] = Operator_st200_cmpeq_general_general_xsrc2;
    TOP__Operator[TOP_cmpgef_n_b] = Operator_st200_cmpgef_n_branch_general_general;
    TOP__Operator[TOP_cmpgef_n_r] = Operator_st200_cmpgef_n_general_general_general;
    TOP__Operator[TOP_cmpgeu_r_b] = Operator_st200_cmpgeu_branch_general_general;
    TOP__Operator[TOP_cmpgeu_i_b] = Operator_st200_cmpgeu_branch_general_isrc2;
    TOP__Operator[TOP_cmpgeu_ii_b] = Operator_st200_cmpgeu_branch_general_xsrc2;
    TOP__Operator[TOP_cmpgeu_r_r] = Operator_st200_cmpgeu_general_general_general;
    TOP__Operator[TOP_cmpgeu_i_r] = Operator_st200_cmpgeu_general_general_isrc2;
    TOP__Operator[TOP_cmpgeu_ii_r] = Operator_st200_cmpgeu_general_general_xsrc2;
    TOP__Operator[TOP_cmpge_r_b] = Operator_st200_cmpge_branch_general_general;
    TOP__Operator[TOP_cmpge_i_b] = Operator_st200_cmpge_branch_general_isrc2;
    TOP__Operator[TOP_cmpge_ii_b] = Operator_st200_cmpge_branch_general_xsrc2;
    TOP__Operator[TOP_cmpge_r_r] = Operator_st200_cmpge_general_general_general;
    TOP__Operator[TOP_cmpge_i_r] = Operator_st200_cmpge_general_general_isrc2;
    TOP__Operator[TOP_cmpge_ii_r] = Operator_st200_cmpge_general_general_xsrc2;
    TOP__Operator[TOP_cmpgtf_n_b] = Operator_st200_cmpgtf_n_branch_general_general;
    TOP__Operator[TOP_cmpgtf_n_r] = Operator_st200_cmpgtf_n_general_general_general;
    TOP__Operator[TOP_cmpgtu_r_b] = Operator_st200_cmpgtu_branch_general_general;
    TOP__Operator[TOP_cmpgtu_i_b] = Operator_st200_cmpgtu_branch_general_isrc2;
    TOP__Operator[TOP_cmpgtu_ii_b] = Operator_st200_cmpgtu_branch_general_xsrc2;
    TOP__Operator[TOP_cmpgtu_r_r] = Operator_st200_cmpgtu_general_general_general;
    TOP__Operator[TOP_cmpgtu_i_r] = Operator_st200_cmpgtu_general_general_isrc2;
    TOP__Operator[TOP_cmpgtu_ii_r] = Operator_st200_cmpgtu_general_general_xsrc2;
    TOP__Operator[TOP_cmpgt_r_b] = Operator_st200_cmpgt_branch_general_general;
    TOP__Operator[TOP_cmpgt_i_b] = Operator_st200_cmpgt_branch_general_isrc2;
    TOP__Operator[TOP_cmpgt_ii_b] = Operator_st200_cmpgt_branch_general_xsrc2;
    TOP__Operator[TOP_cmpgt_r_r] = Operator_st200_cmpgt_general_general_general;
    TOP__Operator[TOP_cmpgt_i_r] = Operator_st200_cmpgt_general_general_isrc2;
    TOP__Operator[TOP_cmpgt_ii_r] = Operator_st200_cmpgt_general_general_xsrc2;
    TOP__Operator[TOP_cmplef_n_b] = Operator_st200_cmplef_n_branch_general_general;
    TOP__Operator[TOP_cmplef_n_r] = Operator_st200_cmplef_n_general_general_general;
    TOP__Operator[TOP_cmpleu_r_b] = Operator_st200_cmpleu_branch_general_general;
    TOP__Operator[TOP_cmpleu_i_b] = Operator_st200_cmpleu_branch_general_isrc2;
    TOP__Operator[TOP_cmpleu_ii_b] = Operator_st200_cmpleu_branch_general_xsrc2;
    TOP__Operator[TOP_cmpleu_r_r] = Operator_st200_cmpleu_general_general_general;
    TOP__Operator[TOP_cmpleu_i_r] = Operator_st200_cmpleu_general_general_isrc2;
    TOP__Operator[TOP_cmpleu_ii_r] = Operator_st200_cmpleu_general_general_xsrc2;
    TOP__Operator[TOP_cmple_r_b] = Operator_st200_cmple_branch_general_general;
    TOP__Operator[TOP_cmple_i_b] = Operator_st200_cmple_branch_general_isrc2;
    TOP__Operator[TOP_cmple_ii_b] = Operator_st200_cmple_branch_general_xsrc2;
    TOP__Operator[TOP_cmple_r_r] = Operator_st200_cmple_general_general_general;
    TOP__Operator[TOP_cmple_i_r] = Operator_st200_cmple_general_general_isrc2;
    TOP__Operator[TOP_cmple_ii_r] = Operator_st200_cmple_general_general_xsrc2;
    TOP__Operator[TOP_cmpltf_n_b] = Operator_st200_cmpltf_n_branch_general_general;
    TOP__Operator[TOP_cmpltf_n_r] = Operator_st200_cmpltf_n_general_general_general;
    TOP__Operator[TOP_cmpltu_r_b] = Operator_st200_cmpltu_branch_general_general;
    TOP__Operator[TOP_cmpltu_i_b] = Operator_st200_cmpltu_branch_general_isrc2;
    TOP__Operator[TOP_cmpltu_ii_b] = Operator_st200_cmpltu_branch_general_xsrc2;
    TOP__Operator[TOP_cmpltu_r_r] = Operator_st200_cmpltu_general_general_general;
    TOP__Operator[TOP_cmpltu_i_r] = Operator_st200_cmpltu_general_general_isrc2;
    TOP__Operator[TOP_cmpltu_ii_r] = Operator_st200_cmpltu_general_general_xsrc2;
    TOP__Operator[TOP_cmplt_r_b] = Operator_st200_cmplt_branch_general_general;
    TOP__Operator[TOP_cmplt_i_b] = Operator_st200_cmplt_branch_general_isrc2;
    TOP__Operator[TOP_cmplt_ii_b] = Operator_st200_cmplt_branch_general_xsrc2;
    TOP__Operator[TOP_cmplt_r_r] = Operator_st200_cmplt_general_general_general;
    TOP__Operator[TOP_cmplt_i_r] = Operator_st200_cmplt_general_general_isrc2;
    TOP__Operator[TOP_cmplt_ii_r] = Operator_st200_cmplt_general_general_xsrc2;
    TOP__Operator[TOP_cmpne_r_b] = Operator_st200_cmpne_branch_general_general;
    TOP__Operator[TOP_cmpne_i_b] = Operator_st200_cmpne_branch_general_isrc2;
    TOP__Operator[TOP_cmpne_ii_b] = Operator_st200_cmpne_branch_general_xsrc2;
    TOP__Operator[TOP_cmpne_r_r] = Operator_st200_cmpne_general_general_general;
    TOP__Operator[TOP_cmpne_i_r] = Operator_st200_cmpne_general_general_isrc2;
    TOP__Operator[TOP_cmpne_ii_r] = Operator_st200_cmpne_general_general_xsrc2;
    TOP__Operator[TOP_composep] = Operator_st200_COMPOSEP;
    TOP__Operator[TOP_convfi_n] = Operator_st200_convfi_n_nolink_general;
    TOP__Operator[TOP_convif_n] = Operator_st200_convif_n_nolink_general;
    TOP__Operator[TOP_COPY] = Operator_COPY;
    TOP__Operator[TOP_divs] = Operator_st200_divs_general_branch_general_general_branch;
    TOP__Operator[TOP_divu] = Operator_st200_divu_nolink_general_general;
    TOP__Operator[TOP_div] = Operator_st200_div_nolink_general_general;
    TOP__Operator[TOP_end_pregtn] = Operator_END_PREGTN;
    TOP__Operator[TOP_extractp] = Operator_st200_EXTRACTP;
    TOP__Operator[TOP_FALL] = Operator_FALL;
    TOP__Operator[TOP_fwd_bar] = Operator_FWDBAR;
    TOP__Operator[TOP_getpc] = Operator_st200_GETPC;
    TOP__Operator[TOP_asm] = Operator_st200_GNUASM;
    TOP__Operator[TOP_GOTO] = Operator_GOTO;
    TOP__Operator[TOP_goto] = Operator_st200_goto_btarg;
    TOP__Operator[TOP_icall] = Operator_st200_icall_link_link;
    TOP__Operator[TOP_idle] = Operator_st200_idle;
    TOP__Operator[TOP_ifixup] = Operator_IFIXUP;
    TOP__Operator[TOP_igoto] = Operator_st200_igoto_link;
    TOP__Operator[TOP_intrncall] = Operator_INTRINSIC;
    TOP__Operator[TOP_JUMP] = Operator_JUMP;
    TOP__Operator[TOP_KILL] = Operator_KILL;
    TOP__Operator[TOP_label] = Operator_LABEL;
    TOP__Operator[TOP_ldbc_i] = Operator_st200_ldbc_nolink_predicate_isrc2_general;
    TOP__Operator[TOP_ldbc_ii] = Operator_st200_ldbc_nolink_predicate_xsrc2_general;
    TOP__Operator[TOP_ldbuc_i] = Operator_st200_ldbuc_nolink_predicate_isrc2_general;
    TOP__Operator[TOP_ldbuc_ii] = Operator_st200_ldbuc_nolink_predicate_xsrc2_general;
    TOP__Operator[TOP_ldbu_d_i] = Operator_st200_ldbu_d_nolink_isrc2_general;
    TOP__Operator[TOP_ldbu_d_ii] = Operator_st200_ldbu_d_nolink_xsrc2_general;
    TOP__Operator[TOP_ldbu_i] = Operator_st200_ldbu_nolink_isrc2_general;
    TOP__Operator[TOP_ldbu_ii] = Operator_st200_ldbu_nolink_xsrc2_general;
    TOP__Operator[TOP_ldb_d_i] = Operator_st200_ldb_d_nolink_isrc2_general;
    TOP__Operator[TOP_ldb_d_ii] = Operator_st200_ldb_d_nolink_xsrc2_general;
    TOP__Operator[TOP_ldb_i] = Operator_st200_ldb_nolink_isrc2_general;
    TOP__Operator[TOP_ldb_ii] = Operator_st200_ldb_nolink_xsrc2_general;
    TOP__Operator[TOP_ldhc_i] = Operator_st200_ldhc_nolink_predicate_isrc2_general;
    TOP__Operator[TOP_ldhc_ii] = Operator_st200_ldhc_nolink_predicate_xsrc2_general;
    TOP__Operator[TOP_ldhuc_i] = Operator_st200_ldhuc_nolink_predicate_isrc2_general;
    TOP__Operator[TOP_ldhuc_ii] = Operator_st200_ldhuc_nolink_predicate_xsrc2_general;
    TOP__Operator[TOP_ldhu_d_i] = Operator_st200_ldhu_d_nolink_isrc2_general;
    TOP__Operator[TOP_ldhu_d_ii] = Operator_st200_ldhu_d_nolink_xsrc2_general;
    TOP__Operator[TOP_ldhu_i] = Operator_st200_ldhu_nolink_isrc2_general;
    TOP__Operator[TOP_ldhu_ii] = Operator_st200_ldhu_nolink_xsrc2_general;
    TOP__Operator[TOP_ldh_d_i] = Operator_st200_ldh_d_nolink_isrc2_general;
    TOP__Operator[TOP_ldh_d_ii] = Operator_st200_ldh_d_nolink_xsrc2_general;
    TOP__Operator[TOP_ldh_i] = Operator_st200_ldh_nolink_isrc2_general;
    TOP__Operator[TOP_ldh_ii] = Operator_st200_ldh_nolink_xsrc2_general;
    TOP__Operator[TOP_ldlc_i] = Operator_st200_ldlc_nzpaired_predicate_isrc2_general;
    TOP__Operator[TOP_ldlc_ii] = Operator_st200_ldlc_nzpaired_predicate_xsrc2_general;
    TOP__Operator[TOP_ldl_i] = Operator_st200_ldl_nzpaired_isrc2_general;
    TOP__Operator[TOP_ldl_ii] = Operator_st200_ldl_nzpaired_xsrc2_general;
    TOP__Operator[TOP_ldwc_i] = Operator_st200_ldwc_general_predicate_isrc2_general;
    TOP__Operator[TOP_ldwc_ii] = Operator_st200_ldwc_general_predicate_xsrc2_general;
    TOP__Operator[TOP_ldwl] = Operator_st200_ldwl_general_general;
    TOP__Operator[TOP_ldw_d_i] = Operator_st200_ldw_d_general_isrc2_general;
    TOP__Operator[TOP_ldw_d_ii] = Operator_st200_ldw_d_general_xsrc2_general;
    TOP__Operator[TOP_ldw_i] = Operator_st200_ldw_general_isrc2_general;
    TOP__Operator[TOP_ldw_ii] = Operator_st200_ldw_general_xsrc2_general;
    TOP__Operator[TOP_LINK] = Operator_LINK;
    TOP__Operator[TOP_LOOP] = Operator_LOOP;
    TOP__Operator[TOP_maxu_r] = Operator_st200_maxu_general_general_general;
    TOP__Operator[TOP_maxu_i] = Operator_st200_maxu_general_general_isrc2;
    TOP__Operator[TOP_maxu_ii] = Operator_st200_maxu_general_general_xsrc2;
    TOP__Operator[TOP_max_r] = Operator_st200_max_general_general_general;
    TOP__Operator[TOP_max_i] = Operator_st200_max_general_general_isrc2;
    TOP__Operator[TOP_max_ii] = Operator_st200_max_general_general_xsrc2;
    TOP__Operator[TOP_mfb] = Operator_st200_mfb_general_branch;
    TOP__Operator[TOP_minu_r] = Operator_st200_minu_general_general_general;
    TOP__Operator[TOP_minu_i] = Operator_st200_minu_general_general_isrc2;
    TOP__Operator[TOP_minu_ii] = Operator_st200_minu_general_general_xsrc2;
    TOP__Operator[TOP_min_r] = Operator_st200_min_general_general_general;
    TOP__Operator[TOP_min_i] = Operator_st200_min_general_general_isrc2;
    TOP__Operator[TOP_min_ii] = Operator_st200_min_general_general_xsrc2;
    TOP__Operator[TOP_movc] = Operator_MOVC;
    TOP__Operator[TOP_movcf] = Operator_MOVCF;
    TOP__Operator[TOP_movp] = Operator_st200_MOVP;
    TOP__Operator[TOP_mov_r] = Operator_st200_mov_general_general;
    TOP__Operator[TOP_mov_i] = Operator_st200_mov_general_isrc2;
    TOP__Operator[TOP_mov_ii] = Operator_st200_mov_general_xsrc2;
    TOP__Operator[TOP_mtb] = Operator_st200_mtb_branch_general;
    TOP__Operator[TOP_mul32_r] = Operator_st200_mul32_nolink_general_general;
    TOP__Operator[TOP_mul32_i] = Operator_st200_mul32_nolink_general_isrc2;
    TOP__Operator[TOP_mul32_ii] = Operator_st200_mul32_nolink_general_xsrc2;
    TOP__Operator[TOP_mul64hu_r] = Operator_st200_mul64hu_nolink_general_general;
    TOP__Operator[TOP_mul64hu_i] = Operator_st200_mul64hu_nolink_general_isrc2;
    TOP__Operator[TOP_mul64hu_ii] = Operator_st200_mul64hu_nolink_general_xsrc2;
    TOP__Operator[TOP_mul64h_r] = Operator_st200_mul64h_nolink_general_general;
    TOP__Operator[TOP_mul64h_i] = Operator_st200_mul64h_nolink_general_isrc2;
    TOP__Operator[TOP_mul64h_ii] = Operator_st200_mul64h_nolink_general_xsrc2;
    TOP__Operator[TOP_mulfrac_r] = Operator_st200_mulfrac_nolink_general_general;
    TOP__Operator[TOP_mulfrac_i] = Operator_st200_mulfrac_nolink_general_isrc2;
    TOP__Operator[TOP_mulfrac_ii] = Operator_st200_mulfrac_nolink_general_xsrc2;
    TOP__Operator[TOP_mulf_n] = Operator_st200_mulf_n_nolink_general_general;
    TOP__Operator[TOP_mulhhs_r] = Operator_st200_mulhhs_nolink_general_general;
    TOP__Operator[TOP_mulhhs_i] = Operator_st200_mulhhs_nolink_general_isrc2;
    TOP__Operator[TOP_mulhhs_ii] = Operator_st200_mulhhs_nolink_general_xsrc2;
    TOP__Operator[TOP_mulhhu_r] = Operator_st200_mulhhu_nolink_general_general;
    TOP__Operator[TOP_mulhhu_i] = Operator_st200_mulhhu_nolink_general_isrc2;
    TOP__Operator[TOP_mulhhu_ii] = Operator_st200_mulhhu_nolink_general_xsrc2;
    TOP__Operator[TOP_mulhh_r] = Operator_st200_mulhh_nolink_general_general;
    TOP__Operator[TOP_mulhh_i] = Operator_st200_mulhh_nolink_general_isrc2;
    TOP__Operator[TOP_mulhh_ii] = Operator_st200_mulhh_nolink_general_xsrc2;
    TOP__Operator[TOP_mulhs_r] = Operator_st200_mulhs_nolink_general_general;
    TOP__Operator[TOP_mulhs_i] = Operator_st200_mulhs_nolink_general_isrc2;
    TOP__Operator[TOP_mulhs_ii] = Operator_st200_mulhs_nolink_general_xsrc2;
    TOP__Operator[TOP_mulhu_r] = Operator_st200_mulhu_nolink_general_general;
    TOP__Operator[TOP_mulhu_i] = Operator_st200_mulhu_nolink_general_isrc2;
    TOP__Operator[TOP_mulhu_ii] = Operator_st200_mulhu_nolink_general_xsrc2;
    TOP__Operator[TOP_mulh_r] = Operator_st200_mulh_nolink_general_general;
    TOP__Operator[TOP_mulh_i] = Operator_st200_mulh_nolink_general_isrc2;
    TOP__Operator[TOP_mulh_ii] = Operator_st200_mulh_nolink_general_xsrc2;
    TOP__Operator[TOP_mullhus_r] = Operator_st200_mullhus_nolink_general_general;
    TOP__Operator[TOP_mullhus_i] = Operator_st200_mullhus_nolink_general_isrc2;
    TOP__Operator[TOP_mullhus_ii] = Operator_st200_mullhus_nolink_general_xsrc2;
    TOP__Operator[TOP_mullhu_r] = Operator_st200_mullhu_nolink_general_general;
    TOP__Operator[TOP_mullhu_i] = Operator_st200_mullhu_nolink_general_isrc2;
    TOP__Operator[TOP_mullhu_ii] = Operator_st200_mullhu_nolink_general_xsrc2;
    TOP__Operator[TOP_mullh_r] = Operator_st200_mullh_nolink_general_general;
    TOP__Operator[TOP_mullh_i] = Operator_st200_mullh_nolink_general_isrc2;
    TOP__Operator[TOP_mullh_ii] = Operator_st200_mullh_nolink_general_xsrc2;
    TOP__Operator[TOP_mulllu_r] = Operator_st200_mulllu_nolink_general_general;
    TOP__Operator[TOP_mulllu_i] = Operator_st200_mulllu_nolink_general_isrc2;
    TOP__Operator[TOP_mulllu_ii] = Operator_st200_mulllu_nolink_general_xsrc2;
    TOP__Operator[TOP_mulll_r] = Operator_st200_mulll_nolink_general_general;
    TOP__Operator[TOP_mulll_i] = Operator_st200_mulll_nolink_general_isrc2;
    TOP__Operator[TOP_mulll_ii] = Operator_st200_mulll_nolink_general_xsrc2;
    TOP__Operator[TOP_mullu_r] = Operator_st200_mullu_nolink_general_general;
    TOP__Operator[TOP_mullu_i] = Operator_st200_mullu_nolink_general_isrc2;
    TOP__Operator[TOP_mullu_ii] = Operator_st200_mullu_nolink_general_xsrc2;
    TOP__Operator[TOP_mull_r] = Operator_st200_mull_nolink_general_general;
    TOP__Operator[TOP_mull_i] = Operator_st200_mull_nolink_general_isrc2;
    TOP__Operator[TOP_mull_ii] = Operator_st200_mull_nolink_general_xsrc2;
    TOP__Operator[TOP_multi_ldlc_i] = Operator_st200_MULTI_ldlc_nzpaired_predicate_isrc2_general;
    TOP__Operator[TOP_multi_ldlc_ii] = Operator_st200_MULTI_ldlc_nzpaired_predicate_xsrc2_general;
    TOP__Operator[TOP_multi_ldl_i] = Operator_st200_MULTI_ldl_nzpaired_isrc2_general;
    TOP__Operator[TOP_multi_ldl_ii] = Operator_st200_MULTI_ldl_nzpaired_xsrc2_general;
    TOP__Operator[TOP_multi_stlc_i] = Operator_st200_MULTI_stlc_isrc2_general_predicate_paired;
    TOP__Operator[TOP_multi_stlc_ii] = Operator_st200_MULTI_stlc_xsrc2_general_predicate_paired;
    TOP__Operator[TOP_multi_stl_i] = Operator_st200_MULTI_stl_isrc2_general_paired;
    TOP__Operator[TOP_multi_stl_ii] = Operator_st200_MULTI_stl_xsrc2_general_paired;
    TOP__Operator[TOP_nandl_r_b] = Operator_st200_nandl_branch_general_general;
    TOP__Operator[TOP_nandl_i_b] = Operator_st200_nandl_branch_general_isrc2;
    TOP__Operator[TOP_nandl_ii_b] = Operator_st200_nandl_branch_general_xsrc2;
    TOP__Operator[TOP_nandl_r_r] = Operator_st200_nandl_general_general_general;
    TOP__Operator[TOP_nandl_i_r] = Operator_st200_nandl_general_general_isrc2;
    TOP__Operator[TOP_nandl_ii_r] = Operator_st200_nandl_general_general_xsrc2;
    TOP__Operator[TOP_noop] = Operator_NOOP;
    TOP__Operator[TOP_nop] = Operator_st200_nop;
    TOP__Operator[TOP_norl_r_b] = Operator_st200_norl_branch_general_general;
    TOP__Operator[TOP_norl_i_b] = Operator_st200_norl_branch_general_isrc2;
    TOP__Operator[TOP_norl_ii_b] = Operator_st200_norl_branch_general_xsrc2;
    TOP__Operator[TOP_norl_r_r] = Operator_st200_norl_general_general_general;
    TOP__Operator[TOP_norl_i_r] = Operator_st200_norl_general_general_isrc2;
    TOP__Operator[TOP_norl_ii_r] = Operator_st200_norl_general_general_xsrc2;
    TOP__Operator[TOP_orc_r] = Operator_st200_orc_general_general_general;
    TOP__Operator[TOP_orc_i] = Operator_st200_orc_general_general_isrc2;
    TOP__Operator[TOP_orc_ii] = Operator_st200_orc_general_general_xsrc2;
    TOP__Operator[TOP_orl_r_b] = Operator_st200_orl_branch_general_general;
    TOP__Operator[TOP_orl_i_b] = Operator_st200_orl_branch_general_isrc2;
    TOP__Operator[TOP_orl_ii_b] = Operator_st200_orl_branch_general_xsrc2;
    TOP__Operator[TOP_orl_r_r] = Operator_st200_orl_general_general_general;
    TOP__Operator[TOP_orl_i_r] = Operator_st200_orl_general_general_isrc2;
    TOP__Operator[TOP_orl_ii_r] = Operator_st200_orl_general_general_xsrc2;
    TOP__Operator[TOP_or_r] = Operator_st200_or_general_general_general;
    TOP__Operator[TOP_or_i] = Operator_st200_or_general_general_isrc2;
    TOP__Operator[TOP_or_ii] = Operator_st200_or_general_general_xsrc2;
    TOP__Operator[TOP_pftc_i] = Operator_st200_pftc_predicate_isrc2_general;
    TOP__Operator[TOP_pftc_ii] = Operator_st200_pftc_predicate_xsrc2_general;
    TOP__Operator[TOP_pft_i] = Operator_st200_pft_isrc2_general;
    TOP__Operator[TOP_pft_ii] = Operator_st200_pft_xsrc2_general;
    TOP__Operator[TOP_phi] = Operator_PHI;
    TOP__Operator[TOP_prgadd_i] = Operator_st200_prgadd_isrc2_general;
    TOP__Operator[TOP_prgadd_ii] = Operator_st200_prgadd_xsrc2_general;
    TOP__Operator[TOP_prgins] = Operator_st200_prgins;
    TOP__Operator[TOP_prginsadd_i] = Operator_st200_prginsadd_isrc2_general;
    TOP__Operator[TOP_prginsadd_ii] = Operator_st200_prginsadd_xsrc2_general;
    TOP__Operator[TOP_prginspg_i] = Operator_st200_prginspg_isrc2_general;
    TOP__Operator[TOP_prginspg_ii] = Operator_st200_prginspg_xsrc2_general;
    TOP__Operator[TOP_prgset_i] = Operator_st200_prgset_isrc2_general;
    TOP__Operator[TOP_prgset_ii] = Operator_st200_prgset_xsrc2_general;
    TOP__Operator[TOP_psi] = Operator_PSI;
    TOP__Operator[TOP_pswclr] = Operator_st200_pswclr_general;
    TOP__Operator[TOP_pswset] = Operator_st200_pswset_general;
    TOP__Operator[TOP_pushregs] = Operator_st200_PUSHREGS;
    TOP__Operator[TOP_remu] = Operator_st200_remu_nolink_general_general;
    TOP__Operator[TOP_rem] = Operator_st200_rem_nolink_general_general;
    TOP__Operator[TOP_RETURN] = Operator_RETURN;
    TOP__Operator[TOP_return] = Operator_st200_return_link;
    TOP__Operator[TOP_rfi] = Operator_st200_rfi;
    TOP__Operator[TOP_sbrk] = Operator_st200_sbrk;
    TOP__Operator[TOP_st235_sbrk] = Operator_st235_sbrk;
    TOP__Operator[TOP_sh1add_r] = Operator_st200_sh1add_general_general_general;
    TOP__Operator[TOP_sh1add_i] = Operator_st200_sh1add_general_general_isrc2;
    TOP__Operator[TOP_sh1add_ii] = Operator_st200_sh1add_general_general_xsrc2;
    TOP__Operator[TOP_sh2add_r] = Operator_st200_sh2add_general_general_general;
    TOP__Operator[TOP_sh2add_i] = Operator_st200_sh2add_general_general_isrc2;
    TOP__Operator[TOP_sh2add_ii] = Operator_st200_sh2add_general_general_xsrc2;
    TOP__Operator[TOP_sh3add_r] = Operator_st200_sh3add_general_general_general;
    TOP__Operator[TOP_sh3add_i] = Operator_st200_sh3add_general_general_isrc2;
    TOP__Operator[TOP_sh3add_ii] = Operator_st200_sh3add_general_general_xsrc2;
    TOP__Operator[TOP_sh4add_r] = Operator_st200_sh4add_general_general_general;
    TOP__Operator[TOP_sh4add_i] = Operator_st200_sh4add_general_general_isrc2;
    TOP__Operator[TOP_sh4add_ii] = Operator_st200_sh4add_general_general_xsrc2;
    TOP__Operator[TOP_shl_r] = Operator_st200_shl_general_general_general;
    TOP__Operator[TOP_shl_i] = Operator_st200_shl_general_general_isrc2;
    TOP__Operator[TOP_shl_ii] = Operator_st200_shl_general_general_xsrc2;
    TOP__Operator[TOP_shru_r] = Operator_st200_shru_general_general_general;
    TOP__Operator[TOP_shru_i] = Operator_st200_shru_general_general_isrc2;
    TOP__Operator[TOP_shru_ii] = Operator_st200_shru_general_general_xsrc2;
    TOP__Operator[TOP_shr_r] = Operator_st200_shr_general_general_general;
    TOP__Operator[TOP_shr_i] = Operator_st200_shr_general_general_isrc2;
    TOP__Operator[TOP_shr_ii] = Operator_st200_shr_general_general_xsrc2;
    TOP__Operator[TOP_SIGMA] = Operator_SIGMA;
    TOP__Operator[TOP_slctf_r] = Operator_st200_slctf_general_branch_general_general;
    TOP__Operator[TOP_slctf_i] = Operator_st200_slctf_general_branch_general_isrc2;
    TOP__Operator[TOP_slctf_ii] = Operator_st200_slctf_general_branch_general_xsrc2;
    TOP__Operator[TOP_slct_r] = Operator_st200_slct_general_branch_general_general;
    TOP__Operator[TOP_slct_i] = Operator_st200_slct_general_branch_general_isrc2;
    TOP__Operator[TOP_slct_ii] = Operator_st200_slct_general_branch_general_xsrc2;
    TOP__Operator[TOP_stbc_i] = Operator_st200_stbc_isrc2_general_predicate_general;
    TOP__Operator[TOP_stbc_ii] = Operator_st200_stbc_xsrc2_general_predicate_general;
    TOP__Operator[TOP_stb_i] = Operator_st200_stb_isrc2_general_general;
    TOP__Operator[TOP_stb_ii] = Operator_st200_stb_xsrc2_general_general;
    TOP__Operator[TOP_sthc_i] = Operator_st200_sthc_isrc2_general_predicate_general;
    TOP__Operator[TOP_sthc_ii] = Operator_st200_sthc_xsrc2_general_predicate_general;
    TOP__Operator[TOP_sth_i] = Operator_st200_sth_isrc2_general_general;
    TOP__Operator[TOP_sth_ii] = Operator_st200_sth_xsrc2_general_general;
    TOP__Operator[TOP_stlc_i] = Operator_st200_stlc_isrc2_general_predicate_paired;
    TOP__Operator[TOP_stlc_ii] = Operator_st200_stlc_xsrc2_general_predicate_paired;
    TOP__Operator[TOP_stl_i] = Operator_st200_stl_isrc2_general_paired;
    TOP__Operator[TOP_stl_ii] = Operator_st200_stl_xsrc2_general_paired;
    TOP__Operator[TOP_stwc_i] = Operator_st200_stwc_isrc2_general_predicate_general;
    TOP__Operator[TOP_stwc_ii] = Operator_st200_stwc_xsrc2_general_predicate_general;
    TOP__Operator[TOP_stwl] = Operator_st200_stwl_branch_general_general;
    TOP__Operator[TOP_stw_i] = Operator_st200_stw_isrc2_general_general;
    TOP__Operator[TOP_stw_ii] = Operator_st200_stw_xsrc2_general_general;
    TOP__Operator[TOP_subf_n] = Operator_st200_subf_n_nolink_general_general;
    TOP__Operator[TOP_sub_r] = Operator_st200_sub_general_general_general;
    TOP__Operator[TOP_sub_i] = Operator_st200_sub_general_isrc2_general;
    TOP__Operator[TOP_sub_ii] = Operator_st200_sub_general_xsrc2_general;
    TOP__Operator[TOP_sxtb] = Operator_st200_sxtb_general_general;
    TOP__Operator[TOP_sxth] = Operator_st200_sxth_general_general;
    TOP__Operator[TOP_sync] = Operator_st200_sync;
    TOP__Operator[TOP_syncins] = Operator_st200_syncins;
    TOP__Operator[TOP_syscall] = Operator_st200_syscall;
    TOP__Operator[TOP_st235_syscall] = Operator_st235_syscall;
    TOP__Operator[TOP_wmb] = Operator_st200_wmb;
    TOP__Operator[TOP_xor_r] = Operator_st200_xor_general_general_general;
    TOP__Operator[TOP_xor_i] = Operator_st200_xor_general_general_isrc2;
    TOP__Operator[TOP_xor_ii] = Operator_st200_xor_general_general_xsrc2;
    TOP__Operator[TOP_zxtb] = Operator_st200_zxtb_general_general;
    TOP__Operator[TOP_zxth] = Operator_st200_zxth_general_general;
    // initialize Operator__TOP;
    for (int i = 0; i < Operator__; i++) Operator__TOP[i] = TOP_UNDEFINED;
    Operator__TOP[Operator_GOTO] = TOP_goto;	// FIXME in LAO_PRO!
    //
    for (int i = 0; i < TOP_UNDEFINED; i++) {
      if (TOP__Operator[i] < 0 || TOP__Operator[i] >= Operator__);
      else Operator__TOP[TOP__Operator[i]] = (TOP)i;
    }
    // initialize IEC__Modifier
    for (int i = 0; i < ISA_EC_MAX; i++) IEC__Modifier[i] = Modifier__;
    // initialize LC__Immediate
    for (int i = 0; i < ISA_LC_MAX; i++) LC__Immediate[i] = Immediate__;
    LC__Immediate[0] = Immediate_st200_xsrc2; // HACK ALERT
    LC__Immediate[LC_btarg] = Immediate_st200_btarg;
    LC__Immediate[LC_isrc2] = Immediate_st200_isrc2;
    LC__Immediate[LC_imm] = Immediate_st200_imm;
    LC__Immediate[LC_sbrknum] = Immediate_st200_sbrknum;
    LC__Immediate[LC_xsrc2] = Immediate_st200_xsrc2;
    LC__Immediate[LC_issrc2] = Immediate_st200_issrc2;
    LC__Immediate[LC_brknum] = Immediate_st200_brknum;
    // initialize IRC__RegFile
    for (int i = 0; i <= ISA_REGISTER_CLASS_MAX; i++) IRC__RegFile[i] = RegFile__;
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
CGIR_LAI_Fini(void) {
}
