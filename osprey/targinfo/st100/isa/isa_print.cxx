//  AUTOMATICALLY GENERATED FROM ARC DATABASE !!! 
// Group TOPS with similar printing format. 
//////////////////////////////////////////////////


#include <stdio.h> 
#include <stddef.h> 
#include <string.h> 
#include <ctype.h> 
#include "topcode.h" 
#include "isa_print_gen.h" 

// Multiple topcodes map to the same assembly name. To disambiguate the 
// topcodes, we append a signature to the basename. To get the assembly 
// name we must strip off the suffix. 

static const char *asmname(TOP topcode) 
{ 
  int c; 
  int i; 
  int j; 
  int k; 
  const char *name = TOP_Name(topcode); 
  char buf[100]; 
  char lower_buf[100]; 

  /* 
   * By convention we have GP32 or GP16 followed by _<name>_ 
   */ 
  for (i = 0; ; i++) { 
    c = name[i]; 
    if (c == '_') break; 
    buf[i] = c; 
  } 

  buf[i] = '\0'; 
  k = 0; 
  // if this is an intrinsic, prepend __ to the name: 
  if (!strcmp (buf, "IFR")) { 
    buf[k++] = '_'; 
    buf[k++] = '_'; 
  } 

  for (j = k; ; j++, i++) { 
    c = name[i+1]; 
    if (c == '\0' || c == '_') break; 
    buf[j] = c; 
  } 

  buf[j] = '\0'; 

  // convert to lower case 
  for (i = 0; i <= j; i++) { 
    lower_buf[i] = tolower(buf[i]); 
  } 

  return strdup(lower_buf); 
} 

main() 
{ 
  ISA_Print_Begin("st100"); 

  Set_AsmName_Func(asmname); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_0; 
  print_0 = ISA_Print_Type_Create("print_0", "%s 	%s %s , @( %s !- %s )"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 
  Operand(3); 

  Instruction_Print_Group(print_0, 
		 TOP_GP32_LCW_GT_CRH_AR_BM_U5, 
		 TOP_GP32_LDEW_GT_DR_AR_BM_U5, 
		 TOP_GP32_LDHH_GT_DR_AR_BM_U5, 
		 TOP_GP32_LDLH_GT_DR_AR_BM_U5, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_1; 
  print_1 = ISA_Print_Type_Create("print_1", "%s 	%s %s %s , @( %s ?- %s )"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Result(0); 
  Operand(2); 
  Operand(3); 

  Instruction_Print_Group(print_1, 
		 TOP_GP32_LAH_GT_MD_AR_AR_QM_U5, 
		 TOP_GP32_LAW_GT_MD_AR_AR_QM_U5, 
		 TOP_GP32_LCG_GT_MD_BR_AR_QM_U5, 
		 TOP_GP32_LCW_GT_MD_CRL_AR_QM_U5, 
		 TOP_GP32_LDBP_GT_MD_DR_AR_QM_U5, 
		 TOP_GP32_LDBSW_GT_MD_DR_AR_QM_U5, 
		 TOP_GP32_LDB_GT_MD_DR_AR_QM_U5, 
		 TOP_GP32_LDF_GT_MD_DR_AR_QM_U5, 
		 TOP_GP32_LDHSW_GT_MD_DR_AR_QM_U5, 
		 TOP_GP32_LDH_GT_MD_DR_AR_QM_U5, 
		 TOP_GP32_LDP_GT_MD_DR_AR_QM_U5, 
		 TOP_GP32_LDSETUB_GT_MD_DR_AR_QM_U5, 
		 TOP_GP32_LDUBP_GT_MD_DR_AR_QM_U5, 
		 TOP_GP32_LDUB_GT_MD_DR_AR_QM_U5, 
		 TOP_GP32_LDUH_GT_MD_DR_AR_QM_U5, 
		 TOP_GP32_LDUW_GT_MD_DR_AR_QM_U5, 
		 TOP_GP32_LDW_GT_MD_DR_AR_QM_U5, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_2; 
  print_2 = ISA_Print_Type_Create("print_2", "%s 	%s %s , @( %s + %s )"); 
  Operand(0); 
  Name(); 
  Result(0); 
  Operand(1); 
  Operand(2); 

  Instruction_Print_Group(print_2, 
		 TOP_GP16_LAW_G0T_ARL_ARL_P_U4, 
		 TOP_GP16_LAW_G0T_ARL_P13_P_U8, 
		 TOP_GP16_LAW_G0T_ARL_P15_P_U6, 
		 TOP_GP16_LDB_G0T_DRL_ARL_P_U4, 
		 TOP_GP16_LDB_G0T_DRL_P13_P_U8, 
		 TOP_GP16_LDB_G0T_DRL_P15_P_U6, 
		 TOP_GP16_LDH_G0T_DRL_ARL_P_U4, 
		 TOP_GP16_LDH_G0T_DRL_P13_P_U8, 
		 TOP_GP16_LDH_G0T_DRL_P15_P_U6, 
		 TOP_GP16_LDUB_G0T_DRL_ARL_P_U4, 
		 TOP_GP16_LDUB_G0T_DRL_P13_P_U8, 
		 TOP_GP16_LDUB_G0T_DRL_P15_P_U6, 
		 TOP_GP16_LDUH_G0T_DRL_ARL_P_U4, 
		 TOP_GP16_LDUH_G0T_DRL_P13_P_U8, 
		 TOP_GP16_LDUH_G0T_DRL_P15_P_U6, 
		 TOP_GP16_LDUW_G0T_DRL_ARL_P_U4, 
		 TOP_GP16_LDUW_G0T_DRL_P13_P_U8, 
		 TOP_GP16_LDUW_G0T_DRL_P15_P_U6, 
		 TOP_GP32_LAH_GT_AR_AR_P_U9, 
		 TOP_GP32_LAH_GT_AR_P13_P_U15, 
		 TOP_GP32_LAW_GT_AR_AR_P_U9, 
		 TOP_GP32_LAW_GT_AR_P13_P_U15, 
		 TOP_GP32_LCG_GT_BR_AR_P_U9, 
		 TOP_GP32_LCG_GT_BR_P13_P_U15, 
		 TOP_GP32_LCW_GT_CRL_AR_P_U9, 
		 TOP_GP32_LCW_GT_CRL_P13_P_U15, 
		 TOP_GP32_LDBP_GT_DR_AR_P_U9, 
		 TOP_GP32_LDBP_GT_DR_P13_P_U15, 
		 TOP_GP32_LDBSW_GT_DR_AR_P_U9, 
		 TOP_GP32_LDBSW_GT_DR_P13_P_U15, 
		 TOP_GP32_LDB_GT_DR_AR_P_U9, 
		 TOP_GP32_LDB_GT_DR_P13_P_U15, 
		 TOP_GP32_LDF_GT_DR_AR_P_U9, 
		 TOP_GP32_LDF_GT_DR_P13_P_U15, 
		 TOP_GP32_LDHSW_GT_DR_AR_P_U9, 
		 TOP_GP32_LDHSW_GT_DR_P13_P_U15, 
		 TOP_GP32_LDH_GT_DR_AR_P_U9, 
		 TOP_GP32_LDH_GT_DR_P13_P_U15, 
		 TOP_GP32_LDP_GT_DR_AR_P_U9, 
		 TOP_GP32_LDP_GT_DR_P13_P_U15, 
		 TOP_GP32_LDSETUB_GT_DR_AR_P_U9, 
		 TOP_GP32_LDSETUB_GT_DR_P13_P_U15, 
		 TOP_GP32_LDUBP_GT_DR_AR_P_U9, 
		 TOP_GP32_LDUBP_GT_DR_P13_P_U15, 
		 TOP_GP32_LDUB_GT_DR_AR_P_U9, 
		 TOP_GP32_LDUB_GT_DR_P13_P_U15, 
		 TOP_GP32_LDUH_GT_DR_AR_P_U9, 
		 TOP_GP32_LDUH_GT_DR_P13_P_U15, 
		 TOP_GP32_LDUW_GT_DR_AR_P_U9, 
		 TOP_GP32_LDUW_GT_DR_P13_P_U15, 
		 TOP_GP32_LDW_GT_DR_AR_P_U9, 
		 TOP_GP32_LDW_GT_DR_P13_P_U15, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_3; 
  print_3 = ISA_Print_Type_Create("print_3", "%s 	%s %s , @( %s -? %s )"); 
  Operand(0); 
  Name(); 
  Result(0); 
  Operand(1); 
  Operand(2); 

  Instruction_Print_Group(print_3, 
		 TOP_GP32_LAH_GT_AR_AR_MQ_U5, 
		 TOP_GP32_LAW_GT_AR_AR_MQ_U5, 
		 TOP_GP32_LCG_GT_BR_AR_MQ_U5, 
		 TOP_GP32_LCW_GT_CRL_AR_MQ_U5, 
		 TOP_GP32_LDBP_GT_DR_AR_MQ_U5, 
		 TOP_GP32_LDBSW_GT_DR_AR_MQ_U5, 
		 TOP_GP32_LDB_GT_DR_AR_MQ_U5, 
		 TOP_GP32_LDF_GT_DR_AR_MQ_U5, 
		 TOP_GP32_LDHSW_GT_DR_AR_MQ_U5, 
		 TOP_GP32_LDH_GT_DR_AR_MQ_U5, 
		 TOP_GP32_LDP_GT_DR_AR_MQ_U5, 
		 TOP_GP32_LDSETUB_GT_DR_AR_MQ_U5, 
		 TOP_GP32_LDUBP_GT_DR_AR_MQ_U5, 
		 TOP_GP32_LDUB_GT_DR_AR_MQ_U5, 
		 TOP_GP32_LDUH_GT_DR_AR_MQ_U5, 
		 TOP_GP32_LDUW_GT_DR_AR_MQ_U5, 
		 TOP_GP32_LDW_GT_DR_AR_MQ_U5, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_4; 
  print_4 = ISA_Print_Type_Create("print_4", "%s 	%s @( %s + ) , %s"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 

  Instruction_Print_Group(print_4, 
		 TOP_GP16_SAW_G0T_P15_QP_ARL, 
		 TOP_GP16_SDB_G0T_P15_QP_DRL, 
		 TOP_GP16_SDH_G0T_P15_QP_DRL, 
		 TOP_GP16_SDW_G0T_P15_QP_DRL, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_5; 
  print_5 = ISA_Print_Type_Create("print_5", "%s 	%s %s , @( %s - %s )"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 
  Operand(3); 

  Instruction_Print_Group(print_5, 
		 TOP_GP32_LCW_GT_CRH_AR_M_AR, 
		 TOP_GP32_LDEW_GT_DR_AR_M_AR, 
		 TOP_GP32_LDHH_GT_DR_AR_M_AR, 
		 TOP_GP32_LDLH_GT_DR_AR_M_AR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_6; 
  print_6 = ISA_Print_Type_Create("print_6", "%s 	%s %s %s , @( %s + %s )"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Result(0); 
  Operand(2); 
  Operand(3); 

  Instruction_Print_Group(print_6, 
		 TOP_GP32_LAH_GT_MD_AR_AR_P_AR, 
		 TOP_GP32_LAW_GT_MD_AR_AR_P_AR, 
		 TOP_GP32_LCG_GT_MD_BR_AR_P_AR, 
		 TOP_GP32_LCW_GT_MD_CRL_AR_P_AR, 
		 TOP_GP32_LDBP_GT_MD_DR_AR_P_AR, 
		 TOP_GP32_LDBSW_GT_MD_DR_AR_P_AR, 
		 TOP_GP32_LDB_GT_MD_DR_AR_P_AR, 
		 TOP_GP32_LDF_GT_MD_DR_AR_P_AR, 
		 TOP_GP32_LDHSW_GT_MD_DR_AR_P_AR, 
		 TOP_GP32_LDH_GT_MD_DR_AR_P_AR, 
		 TOP_GP32_LDP_GT_MD_DR_AR_P_AR, 
		 TOP_GP32_LDSETUB_GT_MD_DR_AR_P_AR, 
		 TOP_GP32_LDUBP_GT_MD_DR_AR_P_AR, 
		 TOP_GP32_LDUB_GT_MD_DR_AR_P_AR, 
		 TOP_GP32_LDUH_GT_MD_DR_AR_P_AR, 
		 TOP_GP32_LDUW_GT_MD_DR_AR_P_AR, 
		 TOP_GP32_LDW_GT_MD_DR_AR_P_AR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_7; 
  print_7 = ISA_Print_Type_Create("print_7", "%s 	%s %s %s , @( %s - %s )"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Result(0); 
  Operand(2); 
  Operand(3); 

  Instruction_Print_Group(print_7, 
		 TOP_GP32_LAH_GT_MD_AR_AR_M_AR, 
		 TOP_GP32_LAW_GT_MD_AR_AR_M_AR, 
		 TOP_GP32_LCG_GT_MD_BR_AR_M_AR, 
		 TOP_GP32_LCW_GT_MD_CRL_AR_M_AR, 
		 TOP_GP32_LDBP_GT_MD_DR_AR_M_AR, 
		 TOP_GP32_LDBSW_GT_MD_DR_AR_M_AR, 
		 TOP_GP32_LDB_GT_MD_DR_AR_M_AR, 
		 TOP_GP32_LDF_GT_MD_DR_AR_M_AR, 
		 TOP_GP32_LDHSW_GT_MD_DR_AR_M_AR, 
		 TOP_GP32_LDH_GT_MD_DR_AR_M_AR, 
		 TOP_GP32_LDP_GT_MD_DR_AR_M_AR, 
		 TOP_GP32_LDSETUB_GT_MD_DR_AR_M_AR, 
		 TOP_GP32_LDUBP_GT_MD_DR_AR_M_AR, 
		 TOP_GP32_LDUB_GT_MD_DR_AR_M_AR, 
		 TOP_GP32_LDUH_GT_MD_DR_AR_M_AR, 
		 TOP_GP32_LDUW_GT_MD_DR_AR_M_AR, 
		 TOP_GP32_LDW_GT_MD_DR_AR_M_AR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_8; 
  print_8 = ISA_Print_Type_Create("print_8", "%s 	%s %s , %s , %s , %s"); 
  Operand(0); 
  Name(); 
  Result(0); 
  Result(1); 
  Operand(1); 
  Operand(2); 

  Instruction_Print_Group(print_8, 
		 TOP_GP32_EQESUB_GT_BR_DR_DR_DR, 
		 TOP_GP32_EQPSUBC_GT_BR_DR_DR_DR, 
		 TOP_GP32_EQPSUB_GT_BR_DR_DR_DR, 
		 TOP_GP32_EQUESUB_GT_BR_DR_DR_DR, 
		 TOP_GP32_EQUPSUB_GT_BR_DR_DR_DR, 
		 TOP_GP32_EQUWSUB_GT_BR_DR_DR_DR, 
		 TOP_GP32_EQWSUBC_GT_BR_DR_DR_DR, 
		 TOP_GP32_EQWSUB_GT_BR_DR_DR_DR, 
		 TOP_GP32_FBCLRP_GT_BR_DR_DR_DR, 
		 TOP_GP32_FBCLR_GT_BR_DR_DR_DR, 
		 TOP_GP32_FBNOTP_GT_BR_DR_DR_DR, 
		 TOP_GP32_FBNOT_GT_BR_DR_DR_DR, 
		 TOP_GP32_FBSETP_GT_BR_DR_DR_DR, 
		 TOP_GP32_FBSET_GT_BR_DR_DR_DR, 
		 TOP_GP32_FEMANDN_GT_BR_DR_DR_DR, 
		 TOP_GP32_FEMAND_GT_BR_DR_DR_DR, 
		 TOP_GP32_FEMNAND_GT_BR_DR_DR_DR, 
		 TOP_GP32_FEMNOR_GT_BR_DR_DR_DR, 
		 TOP_GP32_FEMORN_GT_BR_DR_DR_DR, 
		 TOP_GP32_FEMOR_GT_BR_DR_DR_DR, 
		 TOP_GP32_FEMXNOR_GT_BR_DR_DR_DR, 
		 TOP_GP32_FEMXOR_GT_BR_DR_DR_DR, 
		 TOP_GP32_FPMANDN_GT_BR_DR_DR_DR, 
		 TOP_GP32_FPMAND_GT_BR_DR_DR_DR, 
		 TOP_GP32_FPMNAND_GT_BR_DR_DR_DR, 
		 TOP_GP32_FPMNOR_GT_BR_DR_DR_DR, 
		 TOP_GP32_FPMORN_GT_BR_DR_DR_DR, 
		 TOP_GP32_FPMOR_GT_BR_DR_DR_DR, 
		 TOP_GP32_FPMXNOR_GT_BR_DR_DR_DR, 
		 TOP_GP32_FPMXOR_GT_BR_DR_DR_DR, 
		 TOP_GP32_FWMANDN_GT_BR_DR_DR_DR, 
		 TOP_GP32_FWMAND_GT_BR_DR_DR_DR, 
		 TOP_GP32_FWMNAND_GT_BR_DR_DR_DR, 
		 TOP_GP32_FWMNOR_GT_BR_DR_DR_DR, 
		 TOP_GP32_FWMORN_GT_BR_DR_DR_DR, 
		 TOP_GP32_FWMOR_GT_BR_DR_DR_DR, 
		 TOP_GP32_FWMXNOR_GT_BR_DR_DR_DR, 
		 TOP_GP32_FWMXOR_GT_BR_DR_DR_DR, 
		 TOP_GP32_GEESUB_GT_BR_DR_DR_DR, 
		 TOP_GP32_GEPSUBC_GT_BR_DR_DR_DR, 
		 TOP_GP32_GEPSUB_GT_BR_DR_DR_DR, 
		 TOP_GP32_GEUESUB_GT_BR_DR_DR_DR, 
		 TOP_GP32_GEUPSUB_GT_BR_DR_DR_DR, 
		 TOP_GP32_GEUWSUB_GT_BR_DR_DR_DR, 
		 TOP_GP32_GEWSUBC_GT_BR_DR_DR_DR, 
		 TOP_GP32_GEWSUB_GT_BR_DR_DR_DR, 
		 TOP_GP32_GTESUB_GT_BR_DR_DR_DR, 
		 TOP_GP32_GTPSUBC_GT_BR_DR_DR_DR, 
		 TOP_GP32_GTPSUB_GT_BR_DR_DR_DR, 
		 TOP_GP32_GTUESUB_GT_BR_DR_DR_DR, 
		 TOP_GP32_GTUPSUB_GT_BR_DR_DR_DR, 
		 TOP_GP32_GTUWSUB_GT_BR_DR_DR_DR, 
		 TOP_GP32_GTWSUBC_GT_BR_DR_DR_DR, 
		 TOP_GP32_GTWSUB_GT_BR_DR_DR_DR, 
		 TOP_GP32_LEESUB_GT_BR_DR_DR_DR, 
		 TOP_GP32_LEPSUBC_GT_BR_DR_DR_DR, 
		 TOP_GP32_LEPSUB_GT_BR_DR_DR_DR, 
		 TOP_GP32_LEUESUB_GT_BR_DR_DR_DR, 
		 TOP_GP32_LEUPSUB_GT_BR_DR_DR_DR, 
		 TOP_GP32_LEUWSUB_GT_BR_DR_DR_DR, 
		 TOP_GP32_LEWSUBC_GT_BR_DR_DR_DR, 
		 TOP_GP32_LEWSUB_GT_BR_DR_DR_DR, 
		 TOP_GP32_LTESUB_GT_BR_DR_DR_DR, 
		 TOP_GP32_LTPSUBC_GT_BR_DR_DR_DR, 
		 TOP_GP32_LTPSUB_GT_BR_DR_DR_DR, 
		 TOP_GP32_LTUESUB_GT_BR_DR_DR_DR, 
		 TOP_GP32_LTUPSUB_GT_BR_DR_DR_DR, 
		 TOP_GP32_LTUWSUB_GT_BR_DR_DR_DR, 
		 TOP_GP32_LTWSUBC_GT_BR_DR_DR_DR, 
		 TOP_GP32_LTWSUB_GT_BR_DR_DR_DR, 
		 TOP_GP32_NEESUB_GT_BR_DR_DR_DR, 
		 TOP_GP32_NEPSUBC_GT_BR_DR_DR_DR, 
		 TOP_GP32_NEPSUB_GT_BR_DR_DR_DR, 
		 TOP_GP32_NEUESUB_GT_BR_DR_DR_DR, 
		 TOP_GP32_NEUPSUB_GT_BR_DR_DR_DR, 
		 TOP_GP32_NEUWSUB_GT_BR_DR_DR_DR, 
		 TOP_GP32_NEWSUBC_GT_BR_DR_DR_DR, 
		 TOP_GP32_NEWSUB_GT_BR_DR_DR_DR, 
		 TOP_GP32_TBCLRP_GT_BR_DR_DR_DR, 
		 TOP_GP32_TBCLR_GT_BR_DR_DR_DR, 
		 TOP_GP32_TBNOTP_GT_BR_DR_DR_DR, 
		 TOP_GP32_TBNOT_GT_BR_DR_DR_DR, 
		 TOP_GP32_TBSETP_GT_BR_DR_DR_DR, 
		 TOP_GP32_TBSET_GT_BR_DR_DR_DR, 
		 TOP_GP32_TEMANDN_GT_BR_DR_DR_DR, 
		 TOP_GP32_TEMAND_GT_BR_DR_DR_DR, 
		 TOP_GP32_TEMNAND_GT_BR_DR_DR_DR, 
		 TOP_GP32_TEMNOR_GT_BR_DR_DR_DR, 
		 TOP_GP32_TEMORN_GT_BR_DR_DR_DR, 
		 TOP_GP32_TEMOR_GT_BR_DR_DR_DR, 
		 TOP_GP32_TEMXNOR_GT_BR_DR_DR_DR, 
		 TOP_GP32_TEMXOR_GT_BR_DR_DR_DR, 
		 TOP_GP32_TPMANDN_GT_BR_DR_DR_DR, 
		 TOP_GP32_TPMAND_GT_BR_DR_DR_DR, 
		 TOP_GP32_TPMNAND_GT_BR_DR_DR_DR, 
		 TOP_GP32_TPMNOR_GT_BR_DR_DR_DR, 
		 TOP_GP32_TPMORN_GT_BR_DR_DR_DR, 
		 TOP_GP32_TPMOR_GT_BR_DR_DR_DR, 
		 TOP_GP32_TPMXNOR_GT_BR_DR_DR_DR, 
		 TOP_GP32_TPMXOR_GT_BR_DR_DR_DR, 
		 TOP_GP32_TWMANDN_GT_BR_DR_DR_DR, 
		 TOP_GP32_TWMAND_GT_BR_DR_DR_DR, 
		 TOP_GP32_TWMNAND_GT_BR_DR_DR_DR, 
		 TOP_GP32_TWMNOR_GT_BR_DR_DR_DR, 
		 TOP_GP32_TWMORN_GT_BR_DR_DR_DR, 
		 TOP_GP32_TWMOR_GT_BR_DR_DR_DR, 
		 TOP_GP32_TWMXNOR_GT_BR_DR_DR_DR, 
		 TOP_GP32_TWMXOR_GT_BR_DR_DR_DR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_9; 
  print_9 = ISA_Print_Type_Create("print_9", "%s 	%s %s @( %s !+ %s )"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 
  Operand(3); 

  Instruction_Print_Group(print_9, 
		 TOP_GP32_LFR_GT_MD_AR_BP_U5, 
		 TOP_GP32_LGR_GT_MD_AR_BP_U5, 
		 TOP_GP32_SFR_GT_MD_AR_BP_U5, 
		 TOP_GP32_SGR_GT_MD_AR_BP_U5, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_10; 
  print_10 = ISA_Print_Type_Create("print_10", "%s 	%s %s"); 
  Operand(0); 
  Name(); 
  Operand(1); 

  Instruction_Print_Group(print_10, 
		 TOP_GP16_GOTO_G0F_S9, 
		 TOP_GP32_GOTOPR_GF_U16, 
		 TOP_GP32_GOTO_GF_S21, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_11; 
  print_11 = ISA_Print_Type_Create("print_11", "%s 	%s %s @( %s + %s )"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 
  Operand(3); 

  Instruction_Print_Group(print_11, 
		 TOP_GP32_LFR_GT_MD_AR_P_AR, 
		 TOP_GP32_LGR_GT_MD_AR_P_AR, 
		 TOP_GP32_SFR_GT_MD_AR_P_AR, 
		 TOP_GP32_SGR_GT_MD_AR_P_AR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_12; 
  print_12 = ISA_Print_Type_Create("print_12", "%s 	%s @( %s + %s )"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 

  Instruction_Print_Group(print_12, 
		 TOP_GP32_LFR_GT_AR_P_AR, 
		 TOP_GP32_LGR_GT_AR_P_AR, 
		 TOP_GP32_SFR_GT_AR_P_AR, 
		 TOP_GP32_SGR_GT_AR_P_AR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_13; 
  print_13 = ISA_Print_Type_Create("print_13", "%s 	%s @( %s !+ %s )"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 

  Instruction_Print_Group(print_13, 
		 TOP_GP32_LFR_GT_AR_BP_AR, 
		 TOP_GP32_LGR_GT_AR_BP_AR, 
		 TOP_GP32_SFR_GT_AR_BP_AR, 
		 TOP_GP32_SGR_GT_AR_BP_AR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_14; 
  print_14 = ISA_Print_Type_Create("print_14", "%s 	%s @( %s - %s )"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 

  Instruction_Print_Group(print_14, 
		 TOP_GP32_LFR_GT_AR_M_AR, 
		 TOP_GP32_LGR_GT_AR_M_AR, 
		 TOP_GP32_SFR_GT_AR_M_AR, 
		 TOP_GP32_SGR_GT_AR_M_AR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_15; 
  print_15 = ISA_Print_Type_Create("print_15", "%s 	%s @( %s !- %s )"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 

  Instruction_Print_Group(print_15, 
		 TOP_GP32_LFR_GT_AR_BM_AR, 
		 TOP_GP32_LGR_GT_AR_BM_AR, 
		 TOP_GP32_SFR_GT_AR_BM_AR, 
		 TOP_GP32_SGR_GT_AR_BM_AR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_16; 
  print_16 = ISA_Print_Type_Create("print_16", "%s 	%s %s %s , @( %s ?+ %s )"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 
  Operand(3); 
  Operand(4); 

  Instruction_Print_Group(print_16, 
		 TOP_GP32_LCW_GT_MD_CRH_AR_QP_U5, 
		 TOP_GP32_LDEW_GT_MD_DR_AR_QP_U5, 
		 TOP_GP32_LDHH_GT_MD_DR_AR_QP_U5, 
		 TOP_GP32_LDLH_GT_MD_DR_AR_QP_U5, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_17; 
  print_17 = ISA_Print_Type_Create("print_17", "%s 	%s %s , %s, %s"); 
  Operand(0); 
  Name(); 
  Result(0); 
  Operand(1); 
  Operand(2); 

  Instruction_Print_Group(print_17, 
		 TOP_IFR_DIVE_GT_DR_DR_DR, 
		 TOP_IFR_DIVUE_GT_DR_DR_DR, 
		 TOP_IFR_DIVUW_GT_DR_DR_DR, 
		 TOP_IFR_DIVW_GT_DR_DR_DR, 
		 TOP_IFR_MAXE_GT_DR_DR_DR, 
		 TOP_IFR_MAXH_GT_DR_DR_DR, 
		 TOP_IFR_MAXUE_GT_DR_DR_DR, 
		 TOP_IFR_MAXUH_GT_DR_DR_DR, 
		 TOP_IFR_MAXUW_GT_DR_DR_DR, 
		 TOP_IFR_MAXW_GT_DR_DR_DR, 
		 TOP_IFR_MINE_GT_DR_DR_DR, 
		 TOP_IFR_MINH_GT_DR_DR_DR, 
		 TOP_IFR_MINUE_GT_DR_DR_DR, 
		 TOP_IFR_MINUH_GT_DR_DR_DR, 
		 TOP_IFR_MINUW_GT_DR_DR_DR, 
		 TOP_IFR_MINW_GT_DR_DR_DR, 
		 TOP_IFR_MULH_GT_DR_DR_DR, 
		 TOP_IFR_MULUH_GT_DR_DR_DR, 
		 TOP_IFR_MULUW_GT_DR_DR_DR, 
		 TOP_IFR_MULW_GT_DR_DR_DR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_18; 
  print_18 = ISA_Print_Type_Create("print_18", "%s 	%s %s @( %s ?- %s ) , %s"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 
  Operand(3); 
  Operand(4); 

  Instruction_Print_Group(print_18, 
		 TOP_GP32_SAH_GT_MD_AR_QM_U5_AR, 
		 TOP_GP32_SAW_GT_MD_AR_QM_U5_AR, 
		 TOP_GP32_SCW_GT_MD_AR_QM_U5_CRH, 
		 TOP_GP32_SCW_GT_MD_AR_QM_U5_CRL, 
		 TOP_GP32_SDBP_GT_MD_AR_QM_U5_DR, 
		 TOP_GP32_SDBSW_GT_MD_AR_QM_U5_DR, 
		 TOP_GP32_SDB_GT_MD_AR_QM_U5_DR, 
		 TOP_GP32_SDEW_GT_MD_AR_QM_U5_DR, 
		 TOP_GP32_SDF_GT_MD_AR_QM_U5_DR, 
		 TOP_GP32_SDHSW_GT_MD_AR_QM_U5_DR, 
		 TOP_GP32_SDH_GT_MD_AR_QM_U5_DR, 
		 TOP_GP32_SDP_GT_MD_AR_QM_U5_DR, 
		 TOP_GP32_SDW_GT_MD_AR_QM_U5_DR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_19; 
  print_19 = ISA_Print_Type_Create("print_19", "%s 	%s %s %s , @( %s ?- %s )"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 
  Operand(3); 
  Operand(4); 

  Instruction_Print_Group(print_19, 
		 TOP_GP32_LCW_GT_MD_CRH_AR_QM_U5, 
		 TOP_GP32_LDEW_GT_MD_DR_AR_QM_U5, 
		 TOP_GP32_LDHH_GT_MD_DR_AR_QM_U5, 
		 TOP_GP32_LDLH_GT_MD_DR_AR_QM_U5, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_20; 
  print_20 = ISA_Print_Type_Create("print_20", "%s 	%s %s , @( %s + %s )"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 
  Operand(3); 

  Instruction_Print_Group(print_20, 
		 TOP_GP32_LCW_GT_CRH_AR_P_U9, 
		 TOP_GP32_LCW_GT_CRH_P13_P_U15, 
		 TOP_GP32_LDEW_GT_DR_AR_P_U9, 
		 TOP_GP32_LDEW_GT_DR_P13_P_U15, 
		 TOP_GP32_LDHH_GT_DR_AR_P_U9, 
		 TOP_GP32_LDHH_GT_DR_P13_P_U15, 
		 TOP_GP32_LDLH_GT_DR_AR_P_U9, 
		 TOP_GP32_LDLH_GT_DR_P13_P_U15, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_21; 
  print_21 = ISA_Print_Type_Create("print_21", "%s 	%s @( %s -? %s ) , %s"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 
  Operand(3); 

  Instruction_Print_Group(print_21, 
		 TOP_GP32_SAH_GT_AR_MQ_U5_AR, 
		 TOP_GP32_SAW_GT_AR_MQ_U5_AR, 
		 TOP_GP32_SCW_GT_AR_MQ_U5_CRH, 
		 TOP_GP32_SCW_GT_AR_MQ_U5_CRL, 
		 TOP_GP32_SDBP_GT_AR_MQ_U5_DR, 
		 TOP_GP32_SDBSW_GT_AR_MQ_U5_DR, 
		 TOP_GP32_SDB_GT_AR_MQ_U5_DR, 
		 TOP_GP32_SDEW_GT_AR_MQ_U5_DR, 
		 TOP_GP32_SDF_GT_AR_MQ_U5_DR, 
		 TOP_GP32_SDHSW_GT_AR_MQ_U5_DR, 
		 TOP_GP32_SDH_GT_AR_MQ_U5_DR, 
		 TOP_GP32_SDP_GT_AR_MQ_U5_DR, 
		 TOP_GP32_SDW_GT_AR_MQ_U5_DR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_22; 
  print_22 = ISA_Print_Type_Create("print_22", "%s 	%s %s , @( %s -? %s )"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 
  Operand(3); 

  Instruction_Print_Group(print_22, 
		 TOP_GP32_LCW_GT_CRH_AR_MQ_U5, 
		 TOP_GP32_LDEW_GT_DR_AR_MQ_U5, 
		 TOP_GP32_LDHH_GT_DR_AR_MQ_U5, 
		 TOP_GP32_LDLH_GT_DR_AR_MQ_U5, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_23; 
  print_23 = ISA_Print_Type_Create("print_23", "%s 	%s %s %s , @( %s !+ %s )"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Result(0); 
  Operand(2); 
  Operand(3); 

  Instruction_Print_Group(print_23, 
		 TOP_GP32_LAH_GT_MD_AR_AR_BP_U5, 
		 TOP_GP32_LAW_GT_MD_AR_AR_BP_U5, 
		 TOP_GP32_LCG_GT_MD_BR_AR_BP_U5, 
		 TOP_GP32_LCW_GT_MD_CRL_AR_BP_U5, 
		 TOP_GP32_LDBP_GT_MD_DR_AR_BP_U5, 
		 TOP_GP32_LDBSW_GT_MD_DR_AR_BP_U5, 
		 TOP_GP32_LDB_GT_MD_DR_AR_BP_U5, 
		 TOP_GP32_LDF_GT_MD_DR_AR_BP_U5, 
		 TOP_GP32_LDHSW_GT_MD_DR_AR_BP_U5, 
		 TOP_GP32_LDH_GT_MD_DR_AR_BP_U5, 
		 TOP_GP32_LDP_GT_MD_DR_AR_BP_U5, 
		 TOP_GP32_LDSETUB_GT_MD_DR_AR_BP_U5, 
		 TOP_GP32_LDUBP_GT_MD_DR_AR_BP_U5, 
		 TOP_GP32_LDUB_GT_MD_DR_AR_BP_U5, 
		 TOP_GP32_LDUH_GT_MD_DR_AR_BP_U5, 
		 TOP_GP32_LDUW_GT_MD_DR_AR_BP_U5, 
		 TOP_GP32_LDW_GT_MD_DR_AR_BP_U5, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_24; 
  print_24 = ISA_Print_Type_Create("print_24", "%s 	%s %s %s , @( %s !- %s )"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Result(0); 
  Operand(2); 
  Operand(3); 

  Instruction_Print_Group(print_24, 
		 TOP_GP32_LAH_GT_MD_AR_AR_BM_U5, 
		 TOP_GP32_LAW_GT_MD_AR_AR_BM_U5, 
		 TOP_GP32_LCG_GT_MD_BR_AR_BM_U5, 
		 TOP_GP32_LCW_GT_MD_CRL_AR_BM_U5, 
		 TOP_GP32_LDBP_GT_MD_DR_AR_BM_U5, 
		 TOP_GP32_LDBSW_GT_MD_DR_AR_BM_U5, 
		 TOP_GP32_LDB_GT_MD_DR_AR_BM_U5, 
		 TOP_GP32_LDF_GT_MD_DR_AR_BM_U5, 
		 TOP_GP32_LDHSW_GT_MD_DR_AR_BM_U5, 
		 TOP_GP32_LDH_GT_MD_DR_AR_BM_U5, 
		 TOP_GP32_LDP_GT_MD_DR_AR_BM_U5, 
		 TOP_GP32_LDSETUB_GT_MD_DR_AR_BM_U5, 
		 TOP_GP32_LDUBP_GT_MD_DR_AR_BM_U5, 
		 TOP_GP32_LDUB_GT_MD_DR_AR_BM_U5, 
		 TOP_GP32_LDUH_GT_MD_DR_AR_BM_U5, 
		 TOP_GP32_LDUW_GT_MD_DR_AR_BM_U5, 
		 TOP_GP32_LDW_GT_MD_DR_AR_BM_U5, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_25; 
  print_25 = ISA_Print_Type_Create("print_25", "%s 	%s %s @( %s ?+ %s ) , %s"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 
  Operand(3); 
  Operand(4); 

  Instruction_Print_Group(print_25, 
		 TOP_GP32_SAH_GT_MD_AR_QP_AR_AR, 
		 TOP_GP32_SAW_GT_MD_AR_QP_AR_AR, 
		 TOP_GP32_SCW_GT_MD_AR_QP_AR_CRH, 
		 TOP_GP32_SCW_GT_MD_AR_QP_AR_CRL, 
		 TOP_GP32_SDBP_GT_MD_AR_QP_AR_DR, 
		 TOP_GP32_SDBSW_GT_MD_AR_QP_AR_DR, 
		 TOP_GP32_SDB_GT_MD_AR_QP_AR_DR, 
		 TOP_GP32_SDEW_GT_MD_AR_QP_AR_DR, 
		 TOP_GP32_SDF_GT_MD_AR_QP_AR_DR, 
		 TOP_GP32_SDHSW_GT_MD_AR_QP_AR_DR, 
		 TOP_GP32_SDH_GT_MD_AR_QP_AR_DR, 
		 TOP_GP32_SDP_GT_MD_AR_QP_AR_DR, 
		 TOP_GP32_SDW_GT_MD_AR_QP_AR_DR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_26; 
  print_26 = ISA_Print_Type_Create("print_26", "%s 	%s %s %s , @( %s + %s )"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 
  Operand(3); 
  Operand(4); 

  Instruction_Print_Group(print_26, 
		 TOP_GP32_LCW_GT_MD_CRH_AR_P_AR, 
		 TOP_GP32_LDEW_GT_MD_DR_AR_P_AR, 
		 TOP_GP32_LDHH_GT_MD_DR_AR_P_AR, 
		 TOP_GP32_LDLH_GT_MD_DR_AR_P_AR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_27; 
  print_27 = ISA_Print_Type_Create("print_27", "%s 	%s %s %s , @( %s - %s )"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 
  Operand(3); 
  Operand(4); 

  Instruction_Print_Group(print_27, 
		 TOP_GP32_LCW_GT_MD_CRH_AR_M_AR, 
		 TOP_GP32_LDEW_GT_MD_DR_AR_M_AR, 
		 TOP_GP32_LDHH_GT_MD_DR_AR_M_AR, 
		 TOP_GP32_LDLH_GT_MD_DR_AR_M_AR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_28; 
  print_28 = ISA_Print_Type_Create("print_28", "%s 	%s %s , @( %s ?+ %s )"); 
  Operand(0); 
  Name(); 
  Result(0); 
  Operand(1); 
  Operand(2); 

  Instruction_Print_Group(print_28, 
		 TOP_GP32_LAH_GT_AR_AR_QP_AR, 
		 TOP_GP32_LAW_GT_AR_AR_QP_AR, 
		 TOP_GP32_LCG_GT_BR_AR_QP_AR, 
		 TOP_GP32_LCW_GT_CRL_AR_QP_AR, 
		 TOP_GP32_LDBP_GT_DR_AR_QP_AR, 
		 TOP_GP32_LDBSW_GT_DR_AR_QP_AR, 
		 TOP_GP32_LDB_GT_DR_AR_QP_AR, 
		 TOP_GP32_LDF_GT_DR_AR_QP_AR, 
		 TOP_GP32_LDHSW_GT_DR_AR_QP_AR, 
		 TOP_GP32_LDH_GT_DR_AR_QP_AR, 
		 TOP_GP32_LDP_GT_DR_AR_QP_AR, 
		 TOP_GP32_LDSETUB_GT_DR_AR_QP_AR, 
		 TOP_GP32_LDUBP_GT_DR_AR_QP_AR, 
		 TOP_GP32_LDUB_GT_DR_AR_QP_AR, 
		 TOP_GP32_LDUH_GT_DR_AR_QP_AR, 
		 TOP_GP32_LDUW_GT_DR_AR_QP_AR, 
		 TOP_GP32_LDW_GT_DR_AR_QP_AR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_29; 
  print_29 = ISA_Print_Type_Create("print_29", "%s 	%s %s , %s , %s , %s"); 
  Operand(0); 
  Name(); 
  Result(0); 
  Operand(1); 
  Operand(2); 
  Operand(3); 

  Instruction_Print_Group(print_29, 
		 TOP_GP32_EQEINS_GT_BR_DR_DR_DR, 
		 TOP_GP32_EQPINS_GT_BR_DR_DR_DR, 
		 TOP_GP32_EQUEINS_GT_BR_DR_DR_DR, 
		 TOP_GP32_EQUPINS_GT_BR_DR_DR_DR, 
		 TOP_GP32_EQUWINS_GT_BR_DR_DR_DR, 
		 TOP_GP32_EQWINS_GT_BR_DR_DR_DR, 
		 TOP_GP32_GEEINS_GT_BR_DR_DR_DR, 
		 TOP_GP32_GEPINS_GT_BR_DR_DR_DR, 
		 TOP_GP32_GEUEINS_GT_BR_DR_DR_DR, 
		 TOP_GP32_GEUPINS_GT_BR_DR_DR_DR, 
		 TOP_GP32_GEUWINS_GT_BR_DR_DR_DR, 
		 TOP_GP32_GEWINS_GT_BR_DR_DR_DR, 
		 TOP_GP32_GTEINS_GT_BR_DR_DR_DR, 
		 TOP_GP32_GTPINS_GT_BR_DR_DR_DR, 
		 TOP_GP32_GTUEINS_GT_BR_DR_DR_DR, 
		 TOP_GP32_GTUPINS_GT_BR_DR_DR_DR, 
		 TOP_GP32_GTUWINS_GT_BR_DR_DR_DR, 
		 TOP_GP32_GTWINS_GT_BR_DR_DR_DR, 
		 TOP_GP32_LEEINS_GT_BR_DR_DR_DR, 
		 TOP_GP32_LEPINS_GT_BR_DR_DR_DR, 
		 TOP_GP32_LEUEINS_GT_BR_DR_DR_DR, 
		 TOP_GP32_LEUPINS_GT_BR_DR_DR_DR, 
		 TOP_GP32_LEUWINS_GT_BR_DR_DR_DR, 
		 TOP_GP32_LEWINS_GT_BR_DR_DR_DR, 
		 TOP_GP32_LTEINS_GT_BR_DR_DR_DR, 
		 TOP_GP32_LTPINS_GT_BR_DR_DR_DR, 
		 TOP_GP32_LTUEINS_GT_BR_DR_DR_DR, 
		 TOP_GP32_LTUPINS_GT_BR_DR_DR_DR, 
		 TOP_GP32_LTUWINS_GT_BR_DR_DR_DR, 
		 TOP_GP32_LTWINS_GT_BR_DR_DR_DR, 
		 TOP_GP32_MAFCHH_GT_DR_DR_DR_DR, 
		 TOP_GP32_MAFCHL_GT_DR_DR_DR_DR, 
		 TOP_GP32_MAFCLH_GT_DR_DR_DR_DR, 
		 TOP_GP32_MAFCLL_GT_DR_DR_DR_DR, 
		 TOP_GP32_MAFHH_GT_DR_DR_DR_DR, 
		 TOP_GP32_MAFHL_GT_DR_DR_DR_DR, 
		 TOP_GP32_MAFLH_GT_DR_DR_DR_DR, 
		 TOP_GP32_MAFLL_GT_DR_DR_DR_DR, 
		 TOP_GP32_MAFRCHH_GT_DR_DR_DR_DR, 
		 TOP_GP32_MAFRCHL_GT_DR_DR_DR_DR, 
		 TOP_GP32_MAFRCLH_GT_DR_DR_DR_DR, 
		 TOP_GP32_MAFRCLL_GT_DR_DR_DR_DR, 
		 TOP_GP32_MAFRHH_GT_DR_DR_DR_DR, 
		 TOP_GP32_MAFRHL_GT_DR_DR_DR_DR, 
		 TOP_GP32_MAFRLH_GT_DR_DR_DR_DR, 
		 TOP_GP32_MAFRLL_GT_DR_DR_DR_DR, 
		 TOP_GP32_MASK_GT_DR_DR_DR_DR, 
		 TOP_GP32_MASSHH_GT_DR_DR_DR_DR, 
		 TOP_GP32_MASSHL_GT_DR_DR_DR_DR, 
		 TOP_GP32_MASSLH_GT_DR_DR_DR_DR, 
		 TOP_GP32_MASSLL_GT_DR_DR_DR_DR, 
		 TOP_GP32_MASUHH_GT_DR_DR_DR_DR, 
		 TOP_GP32_MASUHL_GT_DR_DR_DR_DR, 
		 TOP_GP32_MASULH_GT_DR_DR_DR_DR, 
		 TOP_GP32_MASULL_GT_DR_DR_DR_DR, 
		 TOP_GP32_MAUSHH_GT_DR_DR_DR_DR, 
		 TOP_GP32_MAUSHL_GT_DR_DR_DR_DR, 
		 TOP_GP32_MAUSLH_GT_DR_DR_DR_DR, 
		 TOP_GP32_MAUSLL_GT_DR_DR_DR_DR, 
		 TOP_GP32_MAUUHH_GT_DR_DR_DR_DR, 
		 TOP_GP32_MAUUHL_GT_DR_DR_DR_DR, 
		 TOP_GP32_MAUULH_GT_DR_DR_DR_DR, 
		 TOP_GP32_MAUULL_GT_DR_DR_DR_DR, 
		 TOP_GP32_MSFCHH_GT_DR_DR_DR_DR, 
		 TOP_GP32_MSFCHL_GT_DR_DR_DR_DR, 
		 TOP_GP32_MSFCLH_GT_DR_DR_DR_DR, 
		 TOP_GP32_MSFCLL_GT_DR_DR_DR_DR, 
		 TOP_GP32_MSFHH_GT_DR_DR_DR_DR, 
		 TOP_GP32_MSFHL_GT_DR_DR_DR_DR, 
		 TOP_GP32_MSFLH_GT_DR_DR_DR_DR, 
		 TOP_GP32_MSFLL_GT_DR_DR_DR_DR, 
		 TOP_GP32_MSFRCHH_GT_DR_DR_DR_DR, 
		 TOP_GP32_MSFRCHL_GT_DR_DR_DR_DR, 
		 TOP_GP32_MSFRCLH_GT_DR_DR_DR_DR, 
		 TOP_GP32_MSFRCLL_GT_DR_DR_DR_DR, 
		 TOP_GP32_MSFRHH_GT_DR_DR_DR_DR, 
		 TOP_GP32_MSFRHL_GT_DR_DR_DR_DR, 
		 TOP_GP32_MSFRLH_GT_DR_DR_DR_DR, 
		 TOP_GP32_MSFRLL_GT_DR_DR_DR_DR, 
		 TOP_GP32_MSSSHH_GT_DR_DR_DR_DR, 
		 TOP_GP32_MSSSHL_GT_DR_DR_DR_DR, 
		 TOP_GP32_MSSSLH_GT_DR_DR_DR_DR, 
		 TOP_GP32_MSSSLL_GT_DR_DR_DR_DR, 
		 TOP_GP32_MSSUHH_GT_DR_DR_DR_DR, 
		 TOP_GP32_MSSUHL_GT_DR_DR_DR_DR, 
		 TOP_GP32_MSSULH_GT_DR_DR_DR_DR, 
		 TOP_GP32_MSSULL_GT_DR_DR_DR_DR, 
		 TOP_GP32_MSUSHH_GT_DR_DR_DR_DR, 
		 TOP_GP32_MSUSHL_GT_DR_DR_DR_DR, 
		 TOP_GP32_MSUSLH_GT_DR_DR_DR_DR, 
		 TOP_GP32_MSUSLL_GT_DR_DR_DR_DR, 
		 TOP_GP32_MSUUHH_GT_DR_DR_DR_DR, 
		 TOP_GP32_MSUUHL_GT_DR_DR_DR_DR, 
		 TOP_GP32_MSUULH_GT_DR_DR_DR_DR, 
		 TOP_GP32_MSUULL_GT_DR_DR_DR_DR, 
		 TOP_GP32_NEEINS_GT_BR_DR_DR_DR, 
		 TOP_GP32_NEPINS_GT_BR_DR_DR_DR, 
		 TOP_GP32_NEUEINS_GT_BR_DR_DR_DR, 
		 TOP_GP32_NEUPINS_GT_BR_DR_DR_DR, 
		 TOP_GP32_NEUWINS_GT_BR_DR_DR_DR, 
		 TOP_GP32_NEWINS_GT_BR_DR_DR_DR, 
		 TOP_GP32_XSHLW_GT_DR_DR_DR_DR, 
		 TOP_GP32_XSHRW_GT_DR_DR_DR_DR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_30; 
  print_30 = ISA_Print_Type_Create("print_30", "%s 	%s %s @( %s - %s )"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 
  Operand(3); 

  Instruction_Print_Group(print_30, 
		 TOP_GP32_LFR_GT_MD_AR_M_U5, 
		 TOP_GP32_LGR_GT_MD_AR_M_U5, 
		 TOP_GP32_SFR_GT_MD_AR_M_U5, 
		 TOP_GP32_SGR_GT_MD_AR_M_U5, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_31; 
  print_31 = ISA_Print_Type_Create("print_31", "%s 	%s %s @( %s !+ %s ) , %s"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 
  Operand(3); 
  Operand(4); 

  Instruction_Print_Group(print_31, 
		 TOP_GP32_SAH_GT_MD_AR_BP_U5_AR, 
		 TOP_GP32_SAW_GT_MD_AR_BP_U5_AR, 
		 TOP_GP32_SCW_GT_MD_AR_BP_U5_CRH, 
		 TOP_GP32_SCW_GT_MD_AR_BP_U5_CRL, 
		 TOP_GP32_SDBP_GT_MD_AR_BP_U5_DR, 
		 TOP_GP32_SDBSW_GT_MD_AR_BP_U5_DR, 
		 TOP_GP32_SDB_GT_MD_AR_BP_U5_DR, 
		 TOP_GP32_SDEW_GT_MD_AR_BP_U5_DR, 
		 TOP_GP32_SDF_GT_MD_AR_BP_U5_DR, 
		 TOP_GP32_SDHSW_GT_MD_AR_BP_U5_DR, 
		 TOP_GP32_SDH_GT_MD_AR_BP_U5_DR, 
		 TOP_GP32_SDP_GT_MD_AR_BP_U5_DR, 
		 TOP_GP32_SDW_GT_MD_AR_BP_U5_DR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_32; 
  print_32 = ISA_Print_Type_Create("print_32", "%s 	%s %s %s , @( %s !+ %s )"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 
  Operand(3); 
  Operand(4); 

  Instruction_Print_Group(print_32, 
		 TOP_GP32_LCW_GT_MD_CRH_AR_BP_U5, 
		 TOP_GP32_LDEW_GT_MD_DR_AR_BP_U5, 
		 TOP_GP32_LDHH_GT_MD_DR_AR_BP_U5, 
		 TOP_GP32_LDLH_GT_MD_DR_AR_BP_U5, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_33; 
  print_33 = ISA_Print_Type_Create("print_33", "%s 	%s %s @( %s - %s ) , %s"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 
  Operand(3); 
  Operand(4); 

  Instruction_Print_Group(print_33, 
		 TOP_GP32_SAH_GT_MD_AR_M_U5_AR, 
		 TOP_GP32_SAW_GT_MD_AR_M_U5_AR, 
		 TOP_GP32_SCW_GT_MD_AR_M_U5_CRH, 
		 TOP_GP32_SCW_GT_MD_AR_M_U5_CRL, 
		 TOP_GP32_SDBP_GT_MD_AR_M_U5_DR, 
		 TOP_GP32_SDBSW_GT_MD_AR_M_U5_DR, 
		 TOP_GP32_SDB_GT_MD_AR_M_U5_DR, 
		 TOP_GP32_SDEW_GT_MD_AR_M_U5_DR, 
		 TOP_GP32_SDF_GT_MD_AR_M_U5_DR, 
		 TOP_GP32_SDHSW_GT_MD_AR_M_U5_DR, 
		 TOP_GP32_SDH_GT_MD_AR_M_U5_DR, 
		 TOP_GP32_SDP_GT_MD_AR_M_U5_DR, 
		 TOP_GP32_SDW_GT_MD_AR_M_U5_DR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_34; 
  print_34 = ISA_Print_Type_Create("print_34", "%s 	%s %s %s , @( %s !- %s )"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 
  Operand(3); 
  Operand(4); 

  Instruction_Print_Group(print_34, 
		 TOP_GP32_LCW_GT_MD_CRH_AR_BM_U5, 
		 TOP_GP32_LDEW_GT_MD_DR_AR_BM_U5, 
		 TOP_GP32_LDHH_GT_MD_DR_AR_BM_U5, 
		 TOP_GP32_LDLH_GT_MD_DR_AR_BM_U5, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_35; 
  print_35 = ISA_Print_Type_Create("print_35", "%s 	%s @( %s !- %s ) , %s"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 
  Operand(3); 

  Instruction_Print_Group(print_35, 
		 TOP_GP32_SAH_GT_AR_BM_U5_AR, 
		 TOP_GP32_SAW_GT_AR_BM_U5_AR, 
		 TOP_GP32_SCW_GT_AR_BM_U5_CRH, 
		 TOP_GP32_SCW_GT_AR_BM_U5_CRL, 
		 TOP_GP32_SDBP_GT_AR_BM_U5_DR, 
		 TOP_GP32_SDBSW_GT_AR_BM_U5_DR, 
		 TOP_GP32_SDB_GT_AR_BM_U5_DR, 
		 TOP_GP32_SDEW_GT_AR_BM_U5_DR, 
		 TOP_GP32_SDF_GT_AR_BM_U5_DR, 
		 TOP_GP32_SDHSW_GT_AR_BM_U5_DR, 
		 TOP_GP32_SDH_GT_AR_BM_U5_DR, 
		 TOP_GP32_SDP_GT_AR_BM_U5_DR, 
		 TOP_GP32_SDW_GT_AR_BM_U5_DR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_36; 
  print_36 = ISA_Print_Type_Create("print_36", "%s 	%s %s , @( %s ?- %s )"); 
  Operand(0); 
  Name(); 
  Result(0); 
  Operand(1); 
  Operand(2); 

  Instruction_Print_Group(print_36, 
		 TOP_GP32_LAH_GT_AR_AR_QM_U5, 
		 TOP_GP32_LAW_GT_AR_AR_QM_U5, 
		 TOP_GP32_LCG_GT_BR_AR_QM_U5, 
		 TOP_GP32_LCW_GT_CRL_AR_QM_U5, 
		 TOP_GP32_LDBP_GT_DR_AR_QM_U5, 
		 TOP_GP32_LDBSW_GT_DR_AR_QM_U5, 
		 TOP_GP32_LDB_GT_DR_AR_QM_U5, 
		 TOP_GP32_LDF_GT_DR_AR_QM_U5, 
		 TOP_GP32_LDHSW_GT_DR_AR_QM_U5, 
		 TOP_GP32_LDH_GT_DR_AR_QM_U5, 
		 TOP_GP32_LDP_GT_DR_AR_QM_U5, 
		 TOP_GP32_LDSETUB_GT_DR_AR_QM_U5, 
		 TOP_GP32_LDUBP_GT_DR_AR_QM_U5, 
		 TOP_GP32_LDUB_GT_DR_AR_QM_U5, 
		 TOP_GP32_LDUH_GT_DR_AR_QM_U5, 
		 TOP_GP32_LDUW_GT_DR_AR_QM_U5, 
		 TOP_GP32_LDW_GT_DR_AR_QM_U5, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_37; 
  print_37 = ISA_Print_Type_Create("print_37", "%s 	%s %s @( %s + %s ) , %s"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 
  Operand(3); 
  Operand(4); 

  Instruction_Print_Group(print_37, 
		 TOP_GP32_SAH_GT_MD_AR_P_AR_AR, 
		 TOP_GP32_SAW_GT_MD_AR_P_AR_AR, 
		 TOP_GP32_SCW_GT_MD_AR_P_AR_CRH, 
		 TOP_GP32_SCW_GT_MD_AR_P_AR_CRL, 
		 TOP_GP32_SDBP_GT_MD_AR_P_AR_DR, 
		 TOP_GP32_SDBSW_GT_MD_AR_P_AR_DR, 
		 TOP_GP32_SDB_GT_MD_AR_P_AR_DR, 
		 TOP_GP32_SDEW_GT_MD_AR_P_AR_DR, 
		 TOP_GP32_SDF_GT_MD_AR_P_AR_DR, 
		 TOP_GP32_SDHSW_GT_MD_AR_P_AR_DR, 
		 TOP_GP32_SDH_GT_MD_AR_P_AR_DR, 
		 TOP_GP32_SDP_GT_MD_AR_P_AR_DR, 
		 TOP_GP32_SDW_GT_MD_AR_P_AR_DR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_38; 
  print_38 = ISA_Print_Type_Create("print_38", "%s 	%s %s , @( %s ?+ %s )"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 
  Operand(3); 

  Instruction_Print_Group(print_38, 
		 TOP_GP32_LCW_GT_CRH_AR_QP_AR, 
		 TOP_GP32_LDEW_GT_DR_AR_QP_AR, 
		 TOP_GP32_LDHH_GT_DR_AR_QP_AR, 
		 TOP_GP32_LDLH_GT_DR_AR_QP_AR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_39; 
  print_39 = ISA_Print_Type_Create("print_39", "%s 	%s @( %s !- %s ) , %s"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 
  Operand(3); 

  Instruction_Print_Group(print_39, 
		 TOP_GP32_SAH_GT_AR_BM_AR_AR, 
		 TOP_GP32_SAW_GT_AR_BM_AR_AR, 
		 TOP_GP32_SCW_GT_AR_BM_AR_CRH, 
		 TOP_GP32_SCW_GT_AR_BM_AR_CRL, 
		 TOP_GP32_SDBP_GT_AR_BM_AR_DR, 
		 TOP_GP32_SDBSW_GT_AR_BM_AR_DR, 
		 TOP_GP32_SDB_GT_AR_BM_AR_DR, 
		 TOP_GP32_SDEW_GT_AR_BM_AR_DR, 
		 TOP_GP32_SDF_GT_AR_BM_AR_DR, 
		 TOP_GP32_SDHSW_GT_AR_BM_AR_DR, 
		 TOP_GP32_SDH_GT_AR_BM_AR_DR, 
		 TOP_GP32_SDP_GT_AR_BM_AR_DR, 
		 TOP_GP32_SDW_GT_AR_BM_AR_DR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_40; 
  print_40 = ISA_Print_Type_Create("print_40", "%s 	%s %s , %s"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 

  Instruction_Print_Group(print_40, 
		 TOP_GP16_AND_G0T_DRL_DRL, 
		 TOP_GP16_BCLR_G0T_DRL_DRL, 
		 TOP_GP16_BNOT_G0T_DRL_DRL, 
		 TOP_GP16_BSET_G0T_DRL_DRL, 
		 TOP_GP16_DECBA_G0T_ARL_ARL, 
		 TOP_GP16_DECWA_G0T_P15_ARL, 
		 TOP_GP16_INCBA_G0T_ARL_ARL, 
		 TOP_GP16_INCWA_G0T_P15_ARL, 
		 TOP_GP16_MPSSLL_G0T_DRL_DRL, 
		 TOP_GP16_MPSUHL_G0T_DRL_DRL, 
		 TOP_GP16_MPUUHL_G0T_DRL_DRL, 
		 TOP_GP16_MPUULL_G0T_DRL_DRL, 
		 TOP_GP16_OR_G0T_DRL_DRL, 
		 TOP_GP16_ROTLW_G0T_DRL_DRL, 
		 TOP_GP16_ROTRW_G0T_DRL_DRL, 
		 TOP_GP16_SHLU_G0T_DRL_DRL, 
		 TOP_GP16_SHRUW_G0T_DRL_DRL, 
		 TOP_GP16_SHRW_G0T_DRL_DRL, 
		 TOP_GP16_XOR_G0T_DRL_DRL, 
		 TOP_GP32_MOVEHH_GT_DR_DR, 
		 TOP_GP32_MOVEHL_GT_DR_DR, 
		 TOP_GP32_MOVELH_GT_DR_DR, 
		 TOP_GP32_MOVELL_GT_DR_DR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_41; 
  print_41 = ISA_Print_Type_Create("print_41", "%s 	%s %s , @( %s !- %s )"); 
  Operand(0); 
  Name(); 
  Result(0); 
  Operand(1); 
  Operand(2); 

  Instruction_Print_Group(print_41, 
		 TOP_GP32_LAH_GT_AR_AR_BM_AR, 
		 TOP_GP32_LAW_GT_AR_AR_BM_AR, 
		 TOP_GP32_LCG_GT_BR_AR_BM_AR, 
		 TOP_GP32_LCW_GT_CRL_AR_BM_AR, 
		 TOP_GP32_LDBP_GT_DR_AR_BM_AR, 
		 TOP_GP32_LDBSW_GT_DR_AR_BM_AR, 
		 TOP_GP32_LDB_GT_DR_AR_BM_AR, 
		 TOP_GP32_LDF_GT_DR_AR_BM_AR, 
		 TOP_GP32_LDHSW_GT_DR_AR_BM_AR, 
		 TOP_GP32_LDH_GT_DR_AR_BM_AR, 
		 TOP_GP32_LDP_GT_DR_AR_BM_AR, 
		 TOP_GP32_LDSETUB_GT_DR_AR_BM_AR, 
		 TOP_GP32_LDUBP_GT_DR_AR_BM_AR, 
		 TOP_GP32_LDUB_GT_DR_AR_BM_AR, 
		 TOP_GP32_LDUH_GT_DR_AR_BM_AR, 
		 TOP_GP32_LDUW_GT_DR_AR_BM_AR, 
		 TOP_GP32_LDW_GT_DR_AR_BM_AR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_42; 
  print_42 = ISA_Print_Type_Create("print_42", "%s 	%s %s , %s , %s"); 
  Operand(0); 
  Name(); 
  Result(0); 
  Operand(1); 
  Operand(2); 

  Instruction_Print_Group(print_42, 
		 TOP_GP16_ADDBA_G0T_ARL_ARL_U4, 
		 TOP_GP16_ADDBA_G0T_ARL_P15_U7, 
		 TOP_GP16_SUBBA_G0T_ARL_ARL_U4, 
		 TOP_GP32_ADDBA_GT_AR_AR_U9, 
		 TOP_GP32_ADDBA_GT_AR_P13_U15, 
		 TOP_GP32_ADDCP_GT_DR_DR_U8, 
		 TOP_GP32_ADDCW_GT_DR_DR_U8, 
		 TOP_GP32_ADDHA_GT_AR_AR_U9, 
		 TOP_GP32_ADDHA_GT_AR_P13_U15, 
		 TOP_GP32_ADDP_GT_DR_DR_U8, 
		 TOP_GP32_ADDUP_GT_DR_DR_U8, 
		 TOP_GP32_ADDU_GT_DR_DR_U8, 
		 TOP_GP32_ADDWA_GT_AR_AR_U9, 
		 TOP_GP32_ADDWA_GT_AR_P13_U15, 
		 TOP_GP32_ADD_GT_DR_DR_U8, 
		 TOP_GP32_ANDNP_GT_DR_DR_U8, 
		 TOP_GP32_ANDN_GT_DR_DR_U8, 
		 TOP_GP32_ANDP_GT_DR_DR_U8, 
		 TOP_GP32_AND_GT_DR_DR_U8, 
		 TOP_GP32_BCLRP_GT_DR_DR_U4, 
		 TOP_GP32_BCLR_GT_DR_DR_U5, 
		 TOP_GP32_BNOTP_GT_DR_DR_U4, 
		 TOP_GP32_BNOT_GT_DR_DR_U5, 
		 TOP_GP32_BSETP_GT_DR_DR_U4, 
		 TOP_GP32_BSET_GT_DR_DR_U5, 
		 TOP_GP32_EQE_GT_BR_DR_U8, 
		 TOP_GP32_EQP_GT_BR_DR_U8, 
		 TOP_GP32_EQUE_GT_BR_DR_U8, 
		 TOP_GP32_EQUP_GT_BR_DR_U8, 
		 TOP_GP32_EQUW_GT_BR_DR_U8, 
		 TOP_GP32_EQW_GT_BR_DR_U8, 
		 TOP_GP32_FBPOSP_GT_BR_DR_U4, 
		 TOP_GP32_FBPOS_GT_BR_DR_U5, 
		 TOP_GP32_FEANDN_GT_BR_DR_U8, 
		 TOP_GP32_FEAND_GT_BR_DR_U8, 
		 TOP_GP32_FEORN_GT_BR_DR_U8, 
		 TOP_GP32_FEOR_GT_BR_DR_U8, 
		 TOP_GP32_FEXNOR_GT_BR_DR_U8, 
		 TOP_GP32_FEXOR_GT_BR_DR_U8, 
		 TOP_GP32_FPANDN_GT_BR_DR_U8, 
		 TOP_GP32_FPAND_GT_BR_DR_U8, 
		 TOP_GP32_FPORN_GT_BR_DR_U8, 
		 TOP_GP32_FPOR_GT_BR_DR_U8, 
		 TOP_GP32_FPXNOR_GT_BR_DR_U8, 
		 TOP_GP32_FPXOR_GT_BR_DR_U8, 
		 TOP_GP32_FWANDN_GT_BR_DR_U8, 
		 TOP_GP32_FWAND_GT_BR_DR_U8, 
		 TOP_GP32_FWORN_GT_BR_DR_U8, 
		 TOP_GP32_FWOR_GT_BR_DR_U8, 
		 TOP_GP32_FWXNOR_GT_BR_DR_U8, 
		 TOP_GP32_FWXOR_GT_BR_DR_U8, 
		 TOP_GP32_GEE_GT_BR_DR_U8, 
		 TOP_GP32_GEP_GT_BR_DR_U8, 
		 TOP_GP32_GEUE_GT_BR_DR_U8, 
		 TOP_GP32_GEUP_GT_BR_DR_U8, 
		 TOP_GP32_GEUW_GT_BR_DR_U8, 
		 TOP_GP32_GEW_GT_BR_DR_U8, 
		 TOP_GP32_GTE_GT_BR_DR_U8, 
		 TOP_GP32_GTP_GT_BR_DR_U8, 
		 TOP_GP32_GTUE_GT_BR_DR_U8, 
		 TOP_GP32_GTUP_GT_BR_DR_U8, 
		 TOP_GP32_GTUW_GT_BR_DR_U8, 
		 TOP_GP32_GTW_GT_BR_DR_U8, 
		 TOP_GP32_LEE_GT_BR_DR_U8, 
		 TOP_GP32_LEP_GT_BR_DR_U8, 
		 TOP_GP32_LEUE_GT_BR_DR_U8, 
		 TOP_GP32_LEUP_GT_BR_DR_U8, 
		 TOP_GP32_LEUW_GT_BR_DR_U8, 
		 TOP_GP32_LEW_GT_BR_DR_U8, 
		 TOP_GP32_LTE_GT_BR_DR_U8, 
		 TOP_GP32_LTP_GT_BR_DR_U8, 
		 TOP_GP32_LTUE_GT_BR_DR_U8, 
		 TOP_GP32_LTUP_GT_BR_DR_U8, 
		 TOP_GP32_LTUW_GT_BR_DR_U8, 
		 TOP_GP32_LTW_GT_BR_DR_U8, 
		 TOP_GP32_MAKEC_GT_CRL_P3_U16, 
		 TOP_GP32_NEE_GT_BR_DR_U8, 
		 TOP_GP32_NEP_GT_BR_DR_U8, 
		 TOP_GP32_NEUE_GT_BR_DR_U8, 
		 TOP_GP32_NEUP_GT_BR_DR_U8, 
		 TOP_GP32_NEUW_GT_BR_DR_U8, 
		 TOP_GP32_NEW_GT_BR_DR_U8, 
		 TOP_GP32_ORNP_GT_DR_DR_U8, 
		 TOP_GP32_ORN_GT_DR_DR_U8, 
		 TOP_GP32_ORP_GT_DR_DR_U8, 
		 TOP_GP32_OR_GT_DR_DR_U8, 
		 TOP_GP32_SHLCW_GT_DR_DR_U5, 
		 TOP_GP32_SHLU_GT_DR_DR_U5, 
		 TOP_GP32_SHL_GT_DR_DR_U5, 
		 TOP_GP32_SHRUW_GT_DR_DR_U5, 
		 TOP_GP32_SHRU_GT_DR_DR_U5, 
		 TOP_GP32_SHRW_GT_DR_DR_U5, 
		 TOP_GP32_SHR_GT_DR_DR_U5, 
		 TOP_GP32_SUBBA_GT_AR_AR_U9, 
		 TOP_GP32_SUBCP_GT_DR_DR_U8, 
		 TOP_GP32_SUBCW_GT_DR_DR_U8, 
		 TOP_GP32_SUBHA_GT_AR_AR_U9, 
		 TOP_GP32_SUBP_GT_DR_DR_U8, 
		 TOP_GP32_SUBUP_GT_DR_DR_U8, 
		 TOP_GP32_SUBU_GT_DR_DR_U8, 
		 TOP_GP32_SUBWA_GT_AR_AR_U9, 
		 TOP_GP32_SUB_GT_DR_DR_U8, 
		 TOP_GP32_TBPOSP_GT_BR_DR_U4, 
		 TOP_GP32_TBPOS_GT_BR_DR_U5, 
		 TOP_GP32_TEANDN_GT_BR_DR_U8, 
		 TOP_GP32_TEAND_GT_BR_DR_U8, 
		 TOP_GP32_TEORN_GT_BR_DR_U8, 
		 TOP_GP32_TEOR_GT_BR_DR_U8, 
		 TOP_GP32_TEXNOR_GT_BR_DR_U8, 
		 TOP_GP32_TEXOR_GT_BR_DR_U8, 
		 TOP_GP32_TPANDN_GT_BR_DR_U8, 
		 TOP_GP32_TPAND_GT_BR_DR_U8, 
		 TOP_GP32_TPORN_GT_BR_DR_U8, 
		 TOP_GP32_TPOR_GT_BR_DR_U8, 
		 TOP_GP32_TPXNOR_GT_BR_DR_U8, 
		 TOP_GP32_TPXOR_GT_BR_DR_U8, 
		 TOP_GP32_TWANDN_GT_BR_DR_U8, 
		 TOP_GP32_TWAND_GT_BR_DR_U8, 
		 TOP_GP32_TWORN_GT_BR_DR_U8, 
		 TOP_GP32_TWOR_GT_BR_DR_U8, 
		 TOP_GP32_TWXNOR_GT_BR_DR_U8, 
		 TOP_GP32_TWXOR_GT_BR_DR_U8, 
		 TOP_GP32_XNORP_GT_DR_DR_U8, 
		 TOP_GP32_XNOR_GT_DR_DR_U8, 
		 TOP_GP32_XORP_GT_DR_DR_U8, 
		 TOP_GP32_XOR_GT_DR_DR_U8, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_43; 
  print_43 = ISA_Print_Type_Create("print_43", "%s 	%s %s @( %s ?- %s )"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 
  Operand(3); 

  Instruction_Print_Group(print_43, 
		 TOP_GP32_LFR_GT_MD_AR_QM_AR, 
		 TOP_GP32_LGR_GT_MD_AR_QM_AR, 
		 TOP_GP32_SFR_GT_MD_AR_QM_AR, 
		 TOP_GP32_SGR_GT_MD_AR_QM_AR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_44; 
  print_44 = ISA_Print_Type_Create("print_44", "	%s %s"); 
  Name(); 
  Operand(0); 

  Instruction_Print_Group(print_44, 
		 TOP_GP32_PUSH_RSET, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_45; 
  print_45 = ISA_Print_Type_Create("print_45", "	%s %s"); 
  Name(); 
  Result(0); 

  Instruction_Print_Group(print_45, 
		 TOP_GP16_SETG_G0, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_46; 
  print_46 = ISA_Print_Type_Create("print_46", "%s 	%s @( %s ?- %s ) , %s"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 
  Operand(3); 

  Instruction_Print_Group(print_46, 
		 TOP_GP32_SAH_GT_AR_QM_U5_AR, 
		 TOP_GP32_SAW_GT_AR_QM_U5_AR, 
		 TOP_GP32_SCW_GT_AR_QM_U5_CRH, 
		 TOP_GP32_SCW_GT_AR_QM_U5_CRL, 
		 TOP_GP32_SDBP_GT_AR_QM_U5_DR, 
		 TOP_GP32_SDBSW_GT_AR_QM_U5_DR, 
		 TOP_GP32_SDB_GT_AR_QM_U5_DR, 
		 TOP_GP32_SDEW_GT_AR_QM_U5_DR, 
		 TOP_GP32_SDF_GT_AR_QM_U5_DR, 
		 TOP_GP32_SDHSW_GT_AR_QM_U5_DR, 
		 TOP_GP32_SDH_GT_AR_QM_U5_DR, 
		 TOP_GP32_SDP_GT_AR_QM_U5_DR, 
		 TOP_GP32_SDW_GT_AR_QM_U5_DR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_47; 
  print_47 = ISA_Print_Type_Create("print_47", "%s 	%s %s , @( %s ?- %s )"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 
  Operand(3); 

  Instruction_Print_Group(print_47, 
		 TOP_GP32_LCW_GT_CRH_AR_QM_U5, 
		 TOP_GP32_LDEW_GT_DR_AR_QM_U5, 
		 TOP_GP32_LDHH_GT_DR_AR_QM_U5, 
		 TOP_GP32_LDLH_GT_DR_AR_QM_U5, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_48; 
  print_48 = ISA_Print_Type_Create("print_48", "%s 	%s @( %s - %s ) , %s"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 
  Operand(3); 

  Instruction_Print_Group(print_48, 
		 TOP_GP32_SAH_GT_AR_M_U9_AR, 
		 TOP_GP32_SAW_GT_AR_M_U9_AR, 
		 TOP_GP32_SCW_GT_AR_M_U9_CRH, 
		 TOP_GP32_SCW_GT_AR_M_U9_CRL, 
		 TOP_GP32_SDBP_GT_AR_M_U9_DR, 
		 TOP_GP32_SDBSW_GT_AR_M_U9_DR, 
		 TOP_GP32_SDB_GT_AR_M_U9_DR, 
		 TOP_GP32_SDEW_GT_AR_M_U9_DR, 
		 TOP_GP32_SDF_GT_AR_M_U9_DR, 
		 TOP_GP32_SDHSW_GT_AR_M_U9_DR, 
		 TOP_GP32_SDH_GT_AR_M_U9_DR, 
		 TOP_GP32_SDP_GT_AR_M_U9_DR, 
		 TOP_GP32_SDW_GT_AR_M_U9_DR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_49; 
  print_49 = ISA_Print_Type_Create("print_49", "%s 	%s %s %s , @( %s + %s )"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Result(0); 
  Operand(2); 
  Operand(3); 

  Instruction_Print_Group(print_49, 
		 TOP_GP32_LAH_GT_MD_AR_AR_P_U5, 
		 TOP_GP32_LAW_GT_MD_AR_AR_P_U5, 
		 TOP_GP32_LCG_GT_MD_BR_AR_P_U5, 
		 TOP_GP32_LCW_GT_MD_CRL_AR_P_U5, 
		 TOP_GP32_LDBP_GT_MD_DR_AR_P_U5, 
		 TOP_GP32_LDBSW_GT_MD_DR_AR_P_U5, 
		 TOP_GP32_LDB_GT_MD_DR_AR_P_U5, 
		 TOP_GP32_LDF_GT_MD_DR_AR_P_U5, 
		 TOP_GP32_LDHSW_GT_MD_DR_AR_P_U5, 
		 TOP_GP32_LDH_GT_MD_DR_AR_P_U5, 
		 TOP_GP32_LDP_GT_MD_DR_AR_P_U5, 
		 TOP_GP32_LDSETUB_GT_MD_DR_AR_P_U5, 
		 TOP_GP32_LDUBP_GT_MD_DR_AR_P_U5, 
		 TOP_GP32_LDUB_GT_MD_DR_AR_P_U5, 
		 TOP_GP32_LDUH_GT_MD_DR_AR_P_U5, 
		 TOP_GP32_LDUW_GT_MD_DR_AR_P_U5, 
		 TOP_GP32_LDW_GT_MD_DR_AR_P_U5, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_50; 
  print_50 = ISA_Print_Type_Create("print_50", "%s 	%s %s , %s"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 

  Instruction_Print_Group(print_50, 
		 TOP_GP16_AND_G0T_DRL_U5, 
		 TOP_GP16_DECU_G0T_DRL_U5, 
		 TOP_GP16_DECWA_G0T_P15_U8, 
		 TOP_GP16_INCU_G0T_DRL_U5, 
		 TOP_GP16_INCWA_G0T_P15_U8, 
		 TOP_GP16_SHLU_G0T_DRL_U5, 
		 TOP_GP16_SHRUW_G0T_DRL_U5, 
		 TOP_GP16_SHRW_G0T_DRL_U5, 
		 TOP_GP32_MOREA_GT_AR_U16, 
		 TOP_GP32_MORE_GT_DR_U16, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_51; 
  print_51 = ISA_Print_Type_Create("print_51", "%s 	%s %s , @( %s !+ %s )"); 
  Operand(0); 
  Name(); 
  Result(0); 
  Operand(1); 
  Operand(2); 

  Instruction_Print_Group(print_51, 
		 TOP_GP32_LAH_GT_AR_AR_BP_U5, 
		 TOP_GP32_LAW_GT_AR_AR_BP_U5, 
		 TOP_GP32_LCG_GT_BR_AR_BP_U5, 
		 TOP_GP32_LCW_GT_CRL_AR_BP_U5, 
		 TOP_GP32_LDBP_GT_DR_AR_BP_U5, 
		 TOP_GP32_LDBSW_GT_DR_AR_BP_U5, 
		 TOP_GP32_LDB_GT_DR_AR_BP_U5, 
		 TOP_GP32_LDF_GT_DR_AR_BP_U5, 
		 TOP_GP32_LDHSW_GT_DR_AR_BP_U5, 
		 TOP_GP32_LDH_GT_DR_AR_BP_U5, 
		 TOP_GP32_LDP_GT_DR_AR_BP_U5, 
		 TOP_GP32_LDSETUB_GT_DR_AR_BP_U5, 
		 TOP_GP32_LDUBP_GT_DR_AR_BP_U5, 
		 TOP_GP32_LDUB_GT_DR_AR_BP_U5, 
		 TOP_GP32_LDUH_GT_DR_AR_BP_U5, 
		 TOP_GP32_LDUW_GT_DR_AR_BP_U5, 
		 TOP_GP32_LDW_GT_DR_AR_BP_U5, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_52; 
  print_52 = ISA_Print_Type_Create("print_52", "%s 	%s %s %s , @( %s - %s )"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Result(0); 
  Operand(2); 
  Operand(3); 

  Instruction_Print_Group(print_52, 
		 TOP_GP32_LAH_GT_MD_AR_AR_M_U5, 
		 TOP_GP32_LAW_GT_MD_AR_AR_M_U5, 
		 TOP_GP32_LCG_GT_MD_BR_AR_M_U5, 
		 TOP_GP32_LCW_GT_MD_CRL_AR_M_U5, 
		 TOP_GP32_LDBP_GT_MD_DR_AR_M_U5, 
		 TOP_GP32_LDBSW_GT_MD_DR_AR_M_U5, 
		 TOP_GP32_LDB_GT_MD_DR_AR_M_U5, 
		 TOP_GP32_LDF_GT_MD_DR_AR_M_U5, 
		 TOP_GP32_LDHSW_GT_MD_DR_AR_M_U5, 
		 TOP_GP32_LDH_GT_MD_DR_AR_M_U5, 
		 TOP_GP32_LDP_GT_MD_DR_AR_M_U5, 
		 TOP_GP32_LDSETUB_GT_MD_DR_AR_M_U5, 
		 TOP_GP32_LDUBP_GT_MD_DR_AR_M_U5, 
		 TOP_GP32_LDUB_GT_MD_DR_AR_M_U5, 
		 TOP_GP32_LDUH_GT_MD_DR_AR_M_U5, 
		 TOP_GP32_LDUW_GT_MD_DR_AR_M_U5, 
		 TOP_GP32_LDW_GT_MD_DR_AR_M_U5, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_53; 
  print_53 = ISA_Print_Type_Create("print_53", "%s 	%s %s , %s"); 
  Operand(0); 
  Name(); 
  Result(0); 
  Operand(1); 

  Instruction_Print_Group(print_53, 
		 TOP_GP16_MAKEBA_G0T_AR_U32, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_54; 
  print_54 = ISA_Print_Type_Create("print_54", "%s 	%s %s , @( %s + )"); 
  Operand(0); 
  Name(); 
  Result(0); 
  Operand(1); 

  Instruction_Print_Group(print_54, 
		 TOP_GP16_LAW_G0T_ARL_P15_QP, 
		 TOP_GP16_LDB_G0T_DRL_P15_QP, 
		 TOP_GP16_LDH_G0T_DRL_P15_QP, 
		 TOP_GP16_LDUB_G0T_DRL_P15_QP, 
		 TOP_GP16_LDUH_G0T_DRL_P15_QP, 
		 TOP_GP16_LDUW_G0T_DRL_P15_QP, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_55; 
  print_55 = ISA_Print_Type_Create("print_55", "%s 	%s %s @( %s ?- %s ) , %s"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 
  Operand(3); 
  Operand(4); 

  Instruction_Print_Group(print_55, 
		 TOP_GP32_SAH_GT_MD_AR_QM_AR_AR, 
		 TOP_GP32_SAW_GT_MD_AR_QM_AR_AR, 
		 TOP_GP32_SCW_GT_MD_AR_QM_AR_CRH, 
		 TOP_GP32_SCW_GT_MD_AR_QM_AR_CRL, 
		 TOP_GP32_SDBP_GT_MD_AR_QM_AR_DR, 
		 TOP_GP32_SDBSW_GT_MD_AR_QM_AR_DR, 
		 TOP_GP32_SDB_GT_MD_AR_QM_AR_DR, 
		 TOP_GP32_SDEW_GT_MD_AR_QM_AR_DR, 
		 TOP_GP32_SDF_GT_MD_AR_QM_AR_DR, 
		 TOP_GP32_SDHSW_GT_MD_AR_QM_AR_DR, 
		 TOP_GP32_SDH_GT_MD_AR_QM_AR_DR, 
		 TOP_GP32_SDP_GT_MD_AR_QM_AR_DR, 
		 TOP_GP32_SDW_GT_MD_AR_QM_AR_DR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_56; 
  print_56 = ISA_Print_Type_Create("print_56", "%s 	%s @( %s ?- %s ) , %s"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 
  Operand(3); 

  Instruction_Print_Group(print_56, 
		 TOP_GP32_SAH_GT_AR_QM_AR_AR, 
		 TOP_GP32_SAW_GT_AR_QM_AR_AR, 
		 TOP_GP32_SCW_GT_AR_QM_AR_CRH, 
		 TOP_GP32_SCW_GT_AR_QM_AR_CRL, 
		 TOP_GP32_SDBP_GT_AR_QM_AR_DR, 
		 TOP_GP32_SDBSW_GT_AR_QM_AR_DR, 
		 TOP_GP32_SDB_GT_AR_QM_AR_DR, 
		 TOP_GP32_SDEW_GT_AR_QM_AR_DR, 
		 TOP_GP32_SDF_GT_AR_QM_AR_DR, 
		 TOP_GP32_SDHSW_GT_AR_QM_AR_DR, 
		 TOP_GP32_SDH_GT_AR_QM_AR_DR, 
		 TOP_GP32_SDP_GT_AR_QM_AR_DR, 
		 TOP_GP32_SDW_GT_AR_QM_AR_DR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_57; 
  print_57 = ISA_Print_Type_Create("print_57", "%s 	%s @( %s - %s ) , %s"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 
  Operand(3); 

  Instruction_Print_Group(print_57, 
		 TOP_GP32_SAH_GT_AR_M_AR_AR, 
		 TOP_GP32_SAW_GT_AR_M_AR_AR, 
		 TOP_GP32_SCW_GT_AR_M_AR_CRH, 
		 TOP_GP32_SCW_GT_AR_M_AR_CRL, 
		 TOP_GP32_SDBP_GT_AR_M_AR_DR, 
		 TOP_GP32_SDBSW_GT_AR_M_AR_DR, 
		 TOP_GP32_SDB_GT_AR_M_AR_DR, 
		 TOP_GP32_SDEW_GT_AR_M_AR_DR, 
		 TOP_GP32_SDF_GT_AR_M_AR_DR, 
		 TOP_GP32_SDHSW_GT_AR_M_AR_DR, 
		 TOP_GP32_SDH_GT_AR_M_AR_DR, 
		 TOP_GP32_SDP_GT_AR_M_AR_DR, 
		 TOP_GP32_SDW_GT_AR_M_AR_DR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_58; 
  print_58 = ISA_Print_Type_Create("print_58", "%s 	%s %s , @( %s + %s )"); 
  Operand(0); 
  Name(); 
  Result(0); 
  Operand(1); 
  Operand(2); 

  Instruction_Print_Group(print_58, 
		 TOP_GP16_LAW_G0T_ARL_ARH_P_P3, 
		 TOP_GP16_LAW_G0T_ARL_ARL_P_P3, 
		 TOP_GP16_LDB_G0T_DRL_ARH_P_P3, 
		 TOP_GP16_LDB_G0T_DRL_ARL_P_P3, 
		 TOP_GP16_LDH_G0T_DRL_ARH_P_P3, 
		 TOP_GP16_LDH_G0T_DRL_ARL_P_P3, 
		 TOP_GP16_LDUB_G0T_DRL_ARH_P_P3, 
		 TOP_GP16_LDUB_G0T_DRL_ARL_P_P3, 
		 TOP_GP16_LDUH_G0T_DRL_ARH_P_P3, 
		 TOP_GP16_LDUH_G0T_DRL_ARL_P_P3, 
		 TOP_GP16_LDUW_G0T_DRL_ARH_P_P3, 
		 TOP_GP16_LDUW_G0T_DRL_ARL_P_P3, 
		 TOP_GP32_LAH_GT_AR_AR_P_AR, 
		 TOP_GP32_LAW_GT_AR_AR_P_AR, 
		 TOP_GP32_LCG_GT_BR_AR_P_AR, 
		 TOP_GP32_LCW_GT_CRL_AR_P_AR, 
		 TOP_GP32_LDBP_GT_DR_AR_P_AR, 
		 TOP_GP32_LDBSW_GT_DR_AR_P_AR, 
		 TOP_GP32_LDB_GT_DR_AR_P_AR, 
		 TOP_GP32_LDF_GT_DR_AR_P_AR, 
		 TOP_GP32_LDHSW_GT_DR_AR_P_AR, 
		 TOP_GP32_LDH_GT_DR_AR_P_AR, 
		 TOP_GP32_LDP_GT_DR_AR_P_AR, 
		 TOP_GP32_LDSETUB_GT_DR_AR_P_AR, 
		 TOP_GP32_LDUBP_GT_DR_AR_P_AR, 
		 TOP_GP32_LDUB_GT_DR_AR_P_AR, 
		 TOP_GP32_LDUH_GT_DR_AR_P_AR, 
		 TOP_GP32_LDUW_GT_DR_AR_P_AR, 
		 TOP_GP32_LDW_GT_DR_AR_P_AR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_59; 
  print_59 = ISA_Print_Type_Create("print_59", "%s 	%s %s , @( %s !- %s )"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 
  Operand(3); 

  Instruction_Print_Group(print_59, 
		 TOP_GP32_LCW_GT_CRH_AR_BM_AR, 
		 TOP_GP32_LDEW_GT_DR_AR_BM_AR, 
		 TOP_GP32_LDHH_GT_DR_AR_BM_AR, 
		 TOP_GP32_LDLH_GT_DR_AR_BM_AR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_60; 
  print_60 = ISA_Print_Type_Create("print_60", "%s 	%s %s @( %s ?+ %s )"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 
  Operand(3); 

  Instruction_Print_Group(print_60, 
		 TOP_GP32_LFR_GT_MD_AR_QP_U5, 
		 TOP_GP32_LGR_GT_MD_AR_QP_U5, 
		 TOP_GP32_SFR_GT_MD_AR_QP_U5, 
		 TOP_GP32_SGR_GT_MD_AR_QP_U5, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_61; 
  print_61 = ISA_Print_Type_Create("print_61", "%s 	%s %s , %s"); 
  Operand(0); 
  Name(); 
  Result(0); 
  Operand(1); 

  Instruction_Print_Group(print_61, 
		 TOP_GP16_COPYA_G0T_ARL_DRL, 
		 TOP_GP16_COPYD_G0T_DRL_ARL, 
		 TOP_GP16_EXTB_G0T_DRL_DRL, 
		 TOP_GP16_EXTH_G0T_DRL_DRL, 
		 TOP_GP16_EXTUB_G0T_DRL_DRL, 
		 TOP_GP16_EXTUH_G0T_DRL_DRL, 
		 TOP_GP16_EXTUW_G0T_DRL_DRL, 
		 TOP_GP16_EXTW_G0T_DRL_DRL, 
		 TOP_GP16_MOVEA_G0T_ARH_ARL, 
		 TOP_GP16_MOVEA_G0T_ARL_ARH, 
		 TOP_GP16_MOVEA_G0T_ARL_ARL, 
		 TOP_GP16_MOVE_G0T_DRH_DRL, 
		 TOP_GP16_MOVE_G0T_DRL_DRH, 
		 TOP_GP16_MOVE_G0T_DRL_DRL, 
		 TOP_GP16_NEGU_G0T_DRL_DRL, 
		 TOP_GP16_NOT_G0T_DRL_DRL, 
		 TOP_GP32_BITRA_GT_AR_AR, 
		 TOP_GP32_BOOLP_GT_DR_BR, 
		 TOP_GP32_BOOL_GT_DR_BR, 
		 TOP_GP32_CLAMPW_GT_DR_DR, 
		 TOP_GP32_COPYA_GT_AR_DR, 
		 TOP_GP32_COPYC_GT_CRL_DR, 
		 TOP_GP32_COPYD_GT_DR_AR, 
		 TOP_GP32_COPYSA_GT_AR_DR, 
		 TOP_GP32_EXTB_GT_DR_DR, 
		 TOP_GP32_EXTH_GT_DR_DR, 
		 TOP_GP32_EXTUB_GT_DR_DR, 
		 TOP_GP32_EXTUH_GT_DR_DR, 
		 TOP_GP32_EXTUW_GT_DR_DR, 
		 TOP_GP32_EXTW_GT_DR_DR, 
		 TOP_GP32_FA_GT_BR_AR, 
		 TOP_GP32_LOCW_GT_DR_DR, 
		 TOP_GP32_LZCW_GT_DR_DR, 
		 TOP_GP32_MAKEC_GT_CRL_P3, 
		 TOP_GP32_MOVEA_GT_AR_AR, 
		 TOP_GP32_MOVEG_GT_BR_BR, 
		 TOP_GP32_MOVEP_GT_DR_DR, 
		 TOP_GP32_MOVE_GT_DR_DR, 
		 TOP_GP32_NEGCP_GT_DR_DR, 
		 TOP_GP32_NEGCW_GT_DR_DR, 
		 TOP_GP32_NEGP_GT_DR_DR, 
		 TOP_GP32_NEGUP_GT_DR_DR, 
		 TOP_GP32_NEGU_GT_DR_DR, 
		 TOP_GP32_NEG_GT_DR_DR, 
		 TOP_GP32_NOTG_GT_BR_BR, 
		 TOP_GP32_NOTPG_GT_BR_BR, 
		 TOP_GP32_NOT_GT_DR_DR, 
		 TOP_GP32_PRIORE_GT_DR_DR, 
		 TOP_GP32_PRIORW_GT_DR_DR, 
		 TOP_GP32_RND2CCW_GT_DR_DR, 
		 TOP_GP32_RND2C_GT_DR_DR, 
		 TOP_GP32_RNDCVCW_GT_DR_DR, 
		 TOP_GP32_RNDCV_GT_DR_DR, 
		 TOP_GP32_SHLU32_GT_DR_DR, 
		 TOP_GP32_SHLUM_GT_DR_DR, 
		 TOP_GP32_SHR32_GT_DR_DR, 
		 TOP_GP32_SHRA1_GT_AR_AR, 
		 TOP_GP32_SHRA2_GT_AR_AR, 
		 TOP_GP32_SHRU32_GT_DR_DR, 
		 TOP_GP32_SHRUWM_GT_DR_DR, 
		 TOP_GP32_TA_GT_BR_AR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_62; 
  print_62 = ISA_Print_Type_Create("print_62", "%s 	%s %s @( %s !+ %s )"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 
  Operand(3); 

  Instruction_Print_Group(print_62, 
		 TOP_GP32_LFR_GT_MD_AR_BP_AR, 
		 TOP_GP32_LGR_GT_MD_AR_BP_AR, 
		 TOP_GP32_SFR_GT_MD_AR_BP_AR, 
		 TOP_GP32_SGR_GT_MD_AR_BP_AR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_63; 
  print_63 = ISA_Print_Type_Create("print_63", "%s 	%s @( %s + %s + %s ) , %s"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 
  Operand(3); 
  Name(); 

  Instruction_Print_Group(print_63, 
		 TOP_GP16_SAW_G0T_P13_PR_U5_ARL, 
		 TOP_GP16_SDB_G0T_P13_PR_U5_DRL, 
		 TOP_GP16_SDH_G0T_P13_PR_U5_DRL, 
		 TOP_GP16_SDW_G0T_P13_PR_U5_DRL, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_64; 
  print_64 = ISA_Print_Type_Create("print_64", "%s 	%s %s , %s , %s"); 
  Operand(0); 
  Name(); 
  Result(0); 
  Operand(1); 
  Operand(2); 

  Instruction_Print_Group(print_64, 
		 TOP_GP16_ADDBA_G0T_ARL_ARH_P3, 
		 TOP_GP16_ADDBA_G0T_ARL_ARL_P3, 
		 TOP_GP16_ADDHA_G0T_ARL_ARH_P3, 
		 TOP_GP16_ADDHA_G0T_ARL_ARL_P3, 
		 TOP_GP16_ADDU_G0T_DRL_DRL_DRL, 
		 TOP_GP16_ADDWA_G0T_ARL_ARH_P3, 
		 TOP_GP16_ADDWA_G0T_ARL_ARL_P3, 
		 TOP_GP16_SUBU_G0T_DRL_DRL_DRL, 
		 TOP_GP32_ADDBA_GT_AR_AR_AR, 
		 TOP_GP32_ADDCP_GT_DR_DR_DR, 
		 TOP_GP32_ADDCW_GT_DR_DR_DR, 
		 TOP_GP32_ADDHA_GT_AR_AR_AR, 
		 TOP_GP32_ADDP_GT_DR_DR_DR, 
		 TOP_GP32_ADDUP_GT_DR_DR_DR, 
		 TOP_GP32_ADDU_GT_DR_DR_DR, 
		 TOP_GP32_ADDWA_GT_AR_AR_AR, 
		 TOP_GP32_ADD_GT_DR_DR_DR, 
		 TOP_GP32_ANDG_GT_BR_BR_BR, 
		 TOP_GP32_ANDNG_GT_BR_BR_BR, 
		 TOP_GP32_ANDNPG_GT_BR_BR_BR, 
		 TOP_GP32_ANDNP_GT_DR_DR_DR, 
		 TOP_GP32_ANDN_GT_DR_DR_DR, 
		 TOP_GP32_ANDPG_GT_BR_BR_BR, 
		 TOP_GP32_ANDP_GT_DR_DR_DR, 
		 TOP_GP32_AND_GT_DR_DR_DR, 
		 TOP_GP32_BCLRP_GT_DR_DR_DR, 
		 TOP_GP32_BCLR_GT_DR_DR_DR, 
		 TOP_GP32_BNOTP_GT_DR_DR_DR, 
		 TOP_GP32_BNOT_GT_DR_DR_DR, 
		 TOP_GP32_BSETP_GT_DR_DR_DR, 
		 TOP_GP32_BSET_GT_DR_DR_DR, 
		 TOP_GP32_EDGESP_GT_DR_DR_DR, 
		 TOP_GP32_EDGES_GT_DR_DR_DR, 
		 TOP_GP32_EQA_GT_BR_AR_AR, 
		 TOP_GP32_EQE_GT_BR_DR_DR, 
		 TOP_GP32_EQP_GT_BR_DR_DR, 
		 TOP_GP32_EQUE_GT_BR_DR_DR, 
		 TOP_GP32_EQUP_GT_BR_DR_DR, 
		 TOP_GP32_EQUW_GT_BR_DR_DR, 
		 TOP_GP32_EQW_GT_BR_DR_DR, 
		 TOP_GP32_FBPOSP_GT_BR_DR_DR, 
		 TOP_GP32_FBPOS_GT_BR_DR_DR, 
		 TOP_GP32_FEANDN_GT_BR_DR_DR, 
		 TOP_GP32_FEAND_GT_BR_DR_DR, 
		 TOP_GP32_FENAND_GT_BR_DR_DR, 
		 TOP_GP32_FENOR_GT_BR_DR_DR, 
		 TOP_GP32_FEORN_GT_BR_DR_DR, 
		 TOP_GP32_FEOR_GT_BR_DR_DR, 
		 TOP_GP32_FEXNOR_GT_BR_DR_DR, 
		 TOP_GP32_FEXOR_GT_BR_DR_DR, 
		 TOP_GP32_FPANDN_GT_BR_DR_DR, 
		 TOP_GP32_FPAND_GT_BR_DR_DR, 
		 TOP_GP32_FPNAND_GT_BR_DR_DR, 
		 TOP_GP32_FPNOR_GT_BR_DR_DR, 
		 TOP_GP32_FPORN_GT_BR_DR_DR, 
		 TOP_GP32_FPOR_GT_BR_DR_DR, 
		 TOP_GP32_FPXNOR_GT_BR_DR_DR, 
		 TOP_GP32_FPXOR_GT_BR_DR_DR, 
		 TOP_GP32_FWANDN_GT_BR_DR_DR, 
		 TOP_GP32_FWAND_GT_BR_DR_DR, 
		 TOP_GP32_FWNAND_GT_BR_DR_DR, 
		 TOP_GP32_FWNOR_GT_BR_DR_DR, 
		 TOP_GP32_FWORN_GT_BR_DR_DR, 
		 TOP_GP32_FWOR_GT_BR_DR_DR, 
		 TOP_GP32_FWXNOR_GT_BR_DR_DR, 
		 TOP_GP32_FWXOR_GT_BR_DR_DR, 
		 TOP_GP32_GEA_GT_BR_AR_AR, 
		 TOP_GP32_GEE_GT_BR_DR_DR, 
		 TOP_GP32_GEP_GT_BR_DR_DR, 
		 TOP_GP32_GEUE_GT_BR_DR_DR, 
		 TOP_GP32_GEUP_GT_BR_DR_DR, 
		 TOP_GP32_GEUW_GT_BR_DR_DR, 
		 TOP_GP32_GEW_GT_BR_DR_DR, 
		 TOP_GP32_GTA_GT_BR_AR_AR, 
		 TOP_GP32_GTE_GT_BR_DR_DR, 
		 TOP_GP32_GTP_GT_BR_DR_DR, 
		 TOP_GP32_GTUE_GT_BR_DR_DR, 
		 TOP_GP32_GTUP_GT_BR_DR_DR, 
		 TOP_GP32_GTUW_GT_BR_DR_DR, 
		 TOP_GP32_GTW_GT_BR_DR_DR, 
		 TOP_GP32_LEA_GT_BR_AR_AR, 
		 TOP_GP32_LEE_GT_BR_DR_DR, 
		 TOP_GP32_LEP_GT_BR_DR_DR, 
		 TOP_GP32_LEUE_GT_BR_DR_DR, 
		 TOP_GP32_LEUP_GT_BR_DR_DR, 
		 TOP_GP32_LEUW_GT_BR_DR_DR, 
		 TOP_GP32_LEW_GT_BR_DR_DR, 
		 TOP_GP32_LTA_GT_BR_AR_AR, 
		 TOP_GP32_LTE_GT_BR_DR_DR, 
		 TOP_GP32_LTP_GT_BR_DR_DR, 
		 TOP_GP32_LTUE_GT_BR_DR_DR, 
		 TOP_GP32_LTUP_GT_BR_DR_DR, 
		 TOP_GP32_LTUW_GT_BR_DR_DR, 
		 TOP_GP32_LTW_GT_BR_DR_DR, 
		 TOP_GP32_MPFCHH_GT_DR_DR_DR, 
		 TOP_GP32_MPFCHL_GT_DR_DR_DR, 
		 TOP_GP32_MPFCLH_GT_DR_DR_DR, 
		 TOP_GP32_MPFCLL_GT_DR_DR_DR, 
		 TOP_GP32_MPFHH_GT_DR_DR_DR, 
		 TOP_GP32_MPFHL_GT_DR_DR_DR, 
		 TOP_GP32_MPFLH_GT_DR_DR_DR, 
		 TOP_GP32_MPFLL_GT_DR_DR_DR, 
		 TOP_GP32_MPFRCHH_GT_DR_DR_DR, 
		 TOP_GP32_MPFRCHL_GT_DR_DR_DR, 
		 TOP_GP32_MPFRCLH_GT_DR_DR_DR, 
		 TOP_GP32_MPFRCLL_GT_DR_DR_DR, 
		 TOP_GP32_MPFRHH_GT_DR_DR_DR, 
		 TOP_GP32_MPFRHL_GT_DR_DR_DR, 
		 TOP_GP32_MPFRLH_GT_DR_DR_DR, 
		 TOP_GP32_MPFRLL_GT_DR_DR_DR, 
		 TOP_GP32_MPSSHH_GT_DR_DR_DR, 
		 TOP_GP32_MPSSHL_GT_DR_DR_DR, 
		 TOP_GP32_MPSSLH_GT_DR_DR_DR, 
		 TOP_GP32_MPSSLL_GT_DR_DR_DR, 
		 TOP_GP32_MPSUHH_GT_DR_DR_DR, 
		 TOP_GP32_MPSUHL_GT_DR_DR_DR, 
		 TOP_GP32_MPSULH_GT_DR_DR_DR, 
		 TOP_GP32_MPSULL_GT_DR_DR_DR, 
		 TOP_GP32_MPUSHH_GT_DR_DR_DR, 
		 TOP_GP32_MPUSHL_GT_DR_DR_DR, 
		 TOP_GP32_MPUSLH_GT_DR_DR_DR, 
		 TOP_GP32_MPUSLL_GT_DR_DR_DR, 
		 TOP_GP32_MPUUHH_GT_DR_DR_DR, 
		 TOP_GP32_MPUUHL_GT_DR_DR_DR, 
		 TOP_GP32_MPUULH_GT_DR_DR_DR, 
		 TOP_GP32_MPUULL_GT_DR_DR_DR, 
		 TOP_GP32_NANDG_GT_BR_BR_BR, 
		 TOP_GP32_NANDPG_GT_BR_BR_BR, 
		 TOP_GP32_NANDP_GT_DR_DR_DR, 
		 TOP_GP32_NAND_GT_DR_DR_DR, 
		 TOP_GP32_NEA_GT_BR_AR_AR, 
		 TOP_GP32_NEE_GT_BR_DR_DR, 
		 TOP_GP32_NEP_GT_BR_DR_DR, 
		 TOP_GP32_NEUE_GT_BR_DR_DR, 
		 TOP_GP32_NEUP_GT_BR_DR_DR, 
		 TOP_GP32_NEUW_GT_BR_DR_DR, 
		 TOP_GP32_NEW_GT_BR_DR_DR, 
		 TOP_GP32_NORG_GT_BR_BR_BR, 
		 TOP_GP32_NORPG_GT_BR_BR_BR, 
		 TOP_GP32_NORP_GT_DR_DR_DR, 
		 TOP_GP32_NOR_GT_DR_DR_DR, 
		 TOP_GP32_ORG_GT_BR_BR_BR, 
		 TOP_GP32_ORNG_GT_BR_BR_BR, 
		 TOP_GP32_ORNPG_GT_BR_BR_BR, 
		 TOP_GP32_ORNP_GT_DR_DR_DR, 
		 TOP_GP32_ORN_GT_DR_DR_DR, 
		 TOP_GP32_ORPG_GT_BR_BR_BR, 
		 TOP_GP32_ORP_GT_DR_DR_DR, 
		 TOP_GP32_OR_GT_DR_DR_DR, 
		 TOP_GP32_SHLCW_GT_DR_DR_DR, 
		 TOP_GP32_SHLUN_GT_DR_DR_DR, 
		 TOP_GP32_SHLU_GT_DR_DR_DR, 
		 TOP_GP32_SHL_GT_DR_DR_DR, 
		 TOP_GP32_SHRUWN_GT_DR_DR_DR, 
		 TOP_GP32_SHRUW_GT_DR_DR_DR, 
		 TOP_GP32_SHRU_GT_DR_DR_DR, 
		 TOP_GP32_SHRWN_GT_DR_DR_DR, 
		 TOP_GP32_SHRW_GT_DR_DR_DR, 
		 TOP_GP32_SHR_GT_DR_DR_DR, 
		 TOP_GP32_SUBBA_GT_AR_AR_AR, 
		 TOP_GP32_SUBCP_GT_DR_DR_DR, 
		 TOP_GP32_SUBCW_GT_DR_DR_DR, 
		 TOP_GP32_SUBHA_GT_AR_AR_AR, 
		 TOP_GP32_SUBP_GT_DR_DR_DR, 
		 TOP_GP32_SUBUP_GT_DR_DR_DR, 
		 TOP_GP32_SUBU_GT_DR_DR_DR, 
		 TOP_GP32_SUBWA_GT_AR_AR_AR, 
		 TOP_GP32_SUB_GT_DR_DR_DR, 
		 TOP_GP32_TBPOSP_GT_BR_DR_DR, 
		 TOP_GP32_TBPOS_GT_BR_DR_DR, 
		 TOP_GP32_TEANDN_GT_BR_DR_DR, 
		 TOP_GP32_TEAND_GT_BR_DR_DR, 
		 TOP_GP32_TENAND_GT_BR_DR_DR, 
		 TOP_GP32_TENOR_GT_BR_DR_DR, 
		 TOP_GP32_TEORN_GT_BR_DR_DR, 
		 TOP_GP32_TEOR_GT_BR_DR_DR, 
		 TOP_GP32_TEXNOR_GT_BR_DR_DR, 
		 TOP_GP32_TEXOR_GT_BR_DR_DR, 
		 TOP_GP32_TPANDN_GT_BR_DR_DR, 
		 TOP_GP32_TPAND_GT_BR_DR_DR, 
		 TOP_GP32_TPNAND_GT_BR_DR_DR, 
		 TOP_GP32_TPNOR_GT_BR_DR_DR, 
		 TOP_GP32_TPORN_GT_BR_DR_DR, 
		 TOP_GP32_TPOR_GT_BR_DR_DR, 
		 TOP_GP32_TPXNOR_GT_BR_DR_DR, 
		 TOP_GP32_TPXOR_GT_BR_DR_DR, 
		 TOP_GP32_TWANDN_GT_BR_DR_DR, 
		 TOP_GP32_TWAND_GT_BR_DR_DR, 
		 TOP_GP32_TWNAND_GT_BR_DR_DR, 
		 TOP_GP32_TWNOR_GT_BR_DR_DR, 
		 TOP_GP32_TWORN_GT_BR_DR_DR, 
		 TOP_GP32_TWOR_GT_BR_DR_DR, 
		 TOP_GP32_TWXNOR_GT_BR_DR_DR, 
		 TOP_GP32_TWXOR_GT_BR_DR_DR, 
		 TOP_GP32_XNORG_GT_BR_BR_BR, 
		 TOP_GP32_XNORPG_GT_BR_BR_BR, 
		 TOP_GP32_XNORP_GT_DR_DR_DR, 
		 TOP_GP32_XNOR_GT_DR_DR_DR, 
		 TOP_GP32_XORG_GT_BR_BR_BR, 
		 TOP_GP32_XORPG_GT_BR_BR_BR, 
		 TOP_GP32_XORP_GT_DR_DR_DR, 
		 TOP_GP32_XOR_GT_DR_DR_DR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_65; 
  print_65 = ISA_Print_Type_Create("print_65", "%s 	%s %s %s , @( %s + %s )"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 
  Operand(3); 
  Operand(4); 

  Instruction_Print_Group(print_65, 
		 TOP_GP32_LCW_GT_MD_CRH_AR_P_U5, 
		 TOP_GP32_LDEW_GT_MD_DR_AR_P_U5, 
		 TOP_GP32_LDHH_GT_MD_DR_AR_P_U5, 
		 TOP_GP32_LDLH_GT_MD_DR_AR_P_U5, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_66; 
  print_66 = ISA_Print_Type_Create("print_66", "%s 	%s @( %s !+ %s ) , %s"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 
  Operand(3); 

  Instruction_Print_Group(print_66, 
		 TOP_GP32_SAH_GT_AR_BP_U5_AR, 
		 TOP_GP32_SAW_GT_AR_BP_U5_AR, 
		 TOP_GP32_SCW_GT_AR_BP_U5_CRH, 
		 TOP_GP32_SCW_GT_AR_BP_U5_CRL, 
		 TOP_GP32_SDBP_GT_AR_BP_U5_DR, 
		 TOP_GP32_SDBSW_GT_AR_BP_U5_DR, 
		 TOP_GP32_SDB_GT_AR_BP_U5_DR, 
		 TOP_GP32_SDEW_GT_AR_BP_U5_DR, 
		 TOP_GP32_SDF_GT_AR_BP_U5_DR, 
		 TOP_GP32_SDHSW_GT_AR_BP_U5_DR, 
		 TOP_GP32_SDH_GT_AR_BP_U5_DR, 
		 TOP_GP32_SDP_GT_AR_BP_U5_DR, 
		 TOP_GP32_SDW_GT_AR_BP_U5_DR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_67; 
  print_67 = ISA_Print_Type_Create("print_67", "%s 	%s %s %s , @( %s - %s )"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 
  Operand(3); 
  Operand(4); 

  Instruction_Print_Group(print_67, 
		 TOP_GP32_LCW_GT_MD_CRH_AR_M_U5, 
		 TOP_GP32_LDEW_GT_MD_DR_AR_M_U5, 
		 TOP_GP32_LDHH_GT_MD_DR_AR_M_U5, 
		 TOP_GP32_LDLH_GT_MD_DR_AR_M_U5, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_68; 
  print_68 = ISA_Print_Type_Create("print_68", "%s 	%s %s , @( %s !+ %s )"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 
  Operand(3); 

  Instruction_Print_Group(print_68, 
		 TOP_GP32_LCW_GT_CRH_AR_BP_U5, 
		 TOP_GP32_LDEW_GT_DR_AR_BP_U5, 
		 TOP_GP32_LDHH_GT_DR_AR_BP_U5, 
		 TOP_GP32_LDLH_GT_DR_AR_BP_U5, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_69; 
  print_69 = ISA_Print_Type_Create("print_69", "%s 	%s %s @( %s !- %s ) , %s"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 
  Operand(3); 
  Operand(4); 

  Instruction_Print_Group(print_69, 
		 TOP_GP32_SAH_GT_MD_AR_BM_U5_AR, 
		 TOP_GP32_SAW_GT_MD_AR_BM_U5_AR, 
		 TOP_GP32_SCW_GT_MD_AR_BM_U5_CRH, 
		 TOP_GP32_SCW_GT_MD_AR_BM_U5_CRL, 
		 TOP_GP32_SDBP_GT_MD_AR_BM_U5_DR, 
		 TOP_GP32_SDBSW_GT_MD_AR_BM_U5_DR, 
		 TOP_GP32_SDB_GT_MD_AR_BM_U5_DR, 
		 TOP_GP32_SDEW_GT_MD_AR_BM_U5_DR, 
		 TOP_GP32_SDF_GT_MD_AR_BM_U5_DR, 
		 TOP_GP32_SDHSW_GT_MD_AR_BM_U5_DR, 
		 TOP_GP32_SDH_GT_MD_AR_BM_U5_DR, 
		 TOP_GP32_SDP_GT_MD_AR_BM_U5_DR, 
		 TOP_GP32_SDW_GT_MD_AR_BM_U5_DR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_70; 
  print_70 = ISA_Print_Type_Create("print_70", "%s 	%s %s , @( %s - %s )"); 
  Operand(0); 
  Name(); 
  Result(0); 
  Operand(1); 
  Operand(2); 

  Instruction_Print_Group(print_70, 
		 TOP_GP32_LAH_GT_AR_AR_M_U9, 
		 TOP_GP32_LAW_GT_AR_AR_M_U9, 
		 TOP_GP32_LCG_GT_BR_AR_M_U9, 
		 TOP_GP32_LCW_GT_CRL_AR_M_U9, 
		 TOP_GP32_LDBP_GT_DR_AR_M_U9, 
		 TOP_GP32_LDBSW_GT_DR_AR_M_U9, 
		 TOP_GP32_LDB_GT_DR_AR_M_U9, 
		 TOP_GP32_LDF_GT_DR_AR_M_U9, 
		 TOP_GP32_LDHSW_GT_DR_AR_M_U9, 
		 TOP_GP32_LDH_GT_DR_AR_M_U9, 
		 TOP_GP32_LDP_GT_DR_AR_M_U9, 
		 TOP_GP32_LDSETUB_GT_DR_AR_M_U9, 
		 TOP_GP32_LDUBP_GT_DR_AR_M_U9, 
		 TOP_GP32_LDUB_GT_DR_AR_M_U9, 
		 TOP_GP32_LDUH_GT_DR_AR_M_U9, 
		 TOP_GP32_LDUW_GT_DR_AR_M_U9, 
		 TOP_GP32_LDW_GT_DR_AR_M_U9, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_71; 
  print_71 = ISA_Print_Type_Create("print_71", "%s 	%s %s , %s"); 
  Operand(0); 
  Name(); 
  Result(0); 
  Operand(1); 

  Instruction_Print_Group(print_71, 
		 TOP_GP16_MAKEB_G0T_DR_S32, 
		 TOP_GP16_MAKEK_G0T_DR_S40, 
		 TOP_GP16_MAKE_G0T_DRL_S7, 
		 TOP_GP32_MAKEA_GT_AR_S16, 
		 TOP_GP32_MAKEBA_GT_AR_U32, 
		 TOP_GP32_MAKEB_GT_DR_S32, 
		 TOP_GP32_MAKEC_GT_CRL_U16, 
		 TOP_GP32_MAKEF_GT_DR_S16, 
		 TOP_GP32_MAKEK_GT_DR_S40, 
		 TOP_GP32_MAKEP_GT_DR_S16, 
		 TOP_GP32_MAKE_GT_DR_S16, 
		 TOP_GP32_SHLUM_GT_DR_U5, 
		 TOP_GP32_SHRUWM_GT_DR_U5, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_72; 
  print_72 = ISA_Print_Type_Create("print_72", "%s 	%s @( - %s ) , %s"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 

  Instruction_Print_Group(print_72, 
		 TOP_GP16_SAW_G0T_MQ_P15_ARL, 
		 TOP_GP16_SDB_G0T_MQ_P15_DRL, 
		 TOP_GP16_SDH_G0T_MQ_P15_DRL, 
		 TOP_GP16_SDW_G0T_MQ_P15_DRL, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_73; 
  print_73 = ISA_Print_Type_Create("print_73", "%s 	%s %s , @( %s + %s )"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 
  Operand(3); 

  Instruction_Print_Group(print_73, 
		 TOP_GP32_LCW_GT_CRH_AR_P_AR, 
		 TOP_GP32_LDEW_GT_DR_AR_P_AR, 
		 TOP_GP32_LDHH_GT_DR_AR_P_AR, 
		 TOP_GP32_LDLH_GT_DR_AR_P_AR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_74; 
  print_74 = ISA_Print_Type_Create("print_74", "%s 	%s %s @( %s !+ %s ) , %s"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 
  Operand(3); 
  Operand(4); 

  Instruction_Print_Group(print_74, 
		 TOP_GP32_SAH_GT_MD_AR_BP_AR_AR, 
		 TOP_GP32_SAW_GT_MD_AR_BP_AR_AR, 
		 TOP_GP32_SCW_GT_MD_AR_BP_AR_CRH, 
		 TOP_GP32_SCW_GT_MD_AR_BP_AR_CRL, 
		 TOP_GP32_SDBP_GT_MD_AR_BP_AR_DR, 
		 TOP_GP32_SDBSW_GT_MD_AR_BP_AR_DR, 
		 TOP_GP32_SDB_GT_MD_AR_BP_AR_DR, 
		 TOP_GP32_SDEW_GT_MD_AR_BP_AR_DR, 
		 TOP_GP32_SDF_GT_MD_AR_BP_AR_DR, 
		 TOP_GP32_SDHSW_GT_MD_AR_BP_AR_DR, 
		 TOP_GP32_SDH_GT_MD_AR_BP_AR_DR, 
		 TOP_GP32_SDP_GT_MD_AR_BP_AR_DR, 
		 TOP_GP32_SDW_GT_MD_AR_BP_AR_DR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_75; 
  print_75 = ISA_Print_Type_Create("print_75", "%s 	%s @( %s !+ %s ) , %s"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 
  Operand(3); 

  Instruction_Print_Group(print_75, 
		 TOP_GP32_SAH_GT_AR_BP_AR_AR, 
		 TOP_GP32_SAW_GT_AR_BP_AR_AR, 
		 TOP_GP32_SCW_GT_AR_BP_AR_CRH, 
		 TOP_GP32_SCW_GT_AR_BP_AR_CRL, 
		 TOP_GP32_SDBP_GT_AR_BP_AR_DR, 
		 TOP_GP32_SDBSW_GT_AR_BP_AR_DR, 
		 TOP_GP32_SDB_GT_AR_BP_AR_DR, 
		 TOP_GP32_SDEW_GT_AR_BP_AR_DR, 
		 TOP_GP32_SDF_GT_AR_BP_AR_DR, 
		 TOP_GP32_SDHSW_GT_AR_BP_AR_DR, 
		 TOP_GP32_SDH_GT_AR_BP_AR_DR, 
		 TOP_GP32_SDP_GT_AR_BP_AR_DR, 
		 TOP_GP32_SDW_GT_AR_BP_AR_DR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_76; 
  print_76 = ISA_Print_Type_Create("print_76", "%s 	%s %s @( %s - %s ) , %s"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 
  Operand(3); 
  Operand(4); 

  Instruction_Print_Group(print_76, 
		 TOP_GP32_SAH_GT_MD_AR_M_AR_AR, 
		 TOP_GP32_SAW_GT_MD_AR_M_AR_AR, 
		 TOP_GP32_SCW_GT_MD_AR_M_AR_CRH, 
		 TOP_GP32_SCW_GT_MD_AR_M_AR_CRL, 
		 TOP_GP32_SDBP_GT_MD_AR_M_AR_DR, 
		 TOP_GP32_SDBSW_GT_MD_AR_M_AR_DR, 
		 TOP_GP32_SDB_GT_MD_AR_M_AR_DR, 
		 TOP_GP32_SDEW_GT_MD_AR_M_AR_DR, 
		 TOP_GP32_SDF_GT_MD_AR_M_AR_DR, 
		 TOP_GP32_SDHSW_GT_MD_AR_M_AR_DR, 
		 TOP_GP32_SDH_GT_MD_AR_M_AR_DR, 
		 TOP_GP32_SDP_GT_MD_AR_M_AR_DR, 
		 TOP_GP32_SDW_GT_MD_AR_M_AR_DR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_77; 
  print_77 = ISA_Print_Type_Create("print_77", "	%s %s"); 
  Name(); 
  Operand(0); 

  Instruction_Print_Group(print_77, 
		 TOP_GP16_GOTO_S11, 
		 TOP_GP16_MAKEPR_S11, 
		 TOP_GP16_MOREPR_U10, 
		 TOP_GP16_POPRTS_U5, 
		 TOP_GP16_POP_U5, 
		 TOP_GP16_PUSH_U5, 
		 TOP_GP16_TRAP_U4, 
		 TOP_GP32_CALL_S25, 
		 TOP_GP32_GOTOPR_U16, 
		 TOP_GP32_GOTO_S25, 
		 TOP_GP32_POPRTE_U20, 
		 TOP_GP32_POPRTS_U20, 
		 TOP_GP32_POP_U20, 
		 TOP_GP32_PUSH_U20, 
		 TOP_GP32_SETILE0_S16, 
		 TOP_GP32_SETILE1_S16, 
		 TOP_GP32_SETILE2_S16, 
		 TOP_GP32_SETLE0_S16, 
		 TOP_GP32_SETLE1_S16, 
		 TOP_GP32_SETLE2_S16, 
		 TOP_GP32_SETLS0_S16, 
		 TOP_GP32_SETLS1_S16, 
		 TOP_GP32_SETLS2_S16, 
		 TOP_GP32_SETULS0_S16, 
		 TOP_GP32_SETULS1_S16, 
		 TOP_GP32_SETULS2_S16, 
		 TOP_GP32_SWI_U12, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_78; 
  print_78 = ISA_Print_Type_Create("print_78", "%s 	%s @( %s -? %s )"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 

  Instruction_Print_Group(print_78, 
		 TOP_GP32_LFR_GT_AR_MQ_U5, 
		 TOP_GP32_LGR_GT_AR_MQ_U5, 
		 TOP_GP32_SFR_GT_AR_MQ_U5, 
		 TOP_GP32_SGR_GT_AR_MQ_U5, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_79; 
  print_79 = ISA_Print_Type_Create("print_79", "%s 	%s %s @( %s !- %s )"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 
  Operand(3); 

  Instruction_Print_Group(print_79, 
		 TOP_GP32_LFR_GT_MD_AR_BM_U5, 
		 TOP_GP32_LGR_GT_MD_AR_BM_U5, 
		 TOP_GP32_SFR_GT_MD_AR_BM_U5, 
		 TOP_GP32_SGR_GT_MD_AR_BM_U5, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_80; 
  print_80 = ISA_Print_Type_Create("print_80", "%s 	%s %s %s , %s , %s"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Result(0); 
  Operand(2); 
  Operand(3); 

  Instruction_Print_Group(print_80, 
		 TOP_GP32_ADDBA_GT_MD_AR_AR_U5, 
		 TOP_GP32_ADDHA_GT_MD_AR_AR_U5, 
		 TOP_GP32_ADDWA_GT_MD_AR_AR_U5, 
		 TOP_GP32_SUBBA_GT_MD_AR_AR_U5, 
		 TOP_GP32_SUBHA_GT_MD_AR_AR_U5, 
		 TOP_GP32_SUBWA_GT_MD_AR_AR_U5, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_81; 
  print_81 = ISA_Print_Type_Create("print_81", "	%s %s , %s , %s"); 
  Name(); 
  Result(0); 
  Operand(0); 
  Operand(1); 

  Instruction_Print_Group(print_81, 
		 TOP_GP16_EQA_G0_ARL_ARL, 
		 TOP_GP16_EQW_G0_DRL_DRL, 
		 TOP_GP16_FBPOS_G0_DRL_DRL, 
		 TOP_GP16_FWAND_G0_DRL_DRL, 
		 TOP_GP16_FWOR_G0_DRL_DRL, 
		 TOP_GP16_GEA_G0_ARL_ARL, 
		 TOP_GP16_GEUW_G0_DRL_DRL, 
		 TOP_GP16_GEW_G0_DRL_DRL, 
		 TOP_GP16_GTA_G0_ARL_ARL, 
		 TOP_GP16_GTUW_G0_DRL_DRL, 
		 TOP_GP16_GTW_G0_DRL_DRL, 
		 TOP_GP16_NEA_G0_ARL_ARL, 
		 TOP_GP16_NEW_G0_DRL_DRL, 
		 TOP_GP16_TBPOS_G0_DRL_DRL, 
		 TOP_GP16_TWAND_G0_DRL_DRL, 
		 TOP_GP16_TWOR_G0_DRL_DRL, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_82; 
  print_82 = ISA_Print_Type_Create("print_82", "%s 	%s %s @( %s - %s )"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 
  Operand(3); 

  Instruction_Print_Group(print_82, 
		 TOP_GP32_LFR_GT_MD_AR_M_AR, 
		 TOP_GP32_LGR_GT_MD_AR_M_AR, 
		 TOP_GP32_SFR_GT_MD_AR_M_AR, 
		 TOP_GP32_SGR_GT_MD_AR_M_AR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_83; 
  print_83 = ISA_Print_Type_Create("print_83", "%s 	%s %s"); 
  Operand(0); 
  Name(); 
  Operand(1); 

  Instruction_Print_Group(print_83, 
		 TOP_GP16_SHLU32_G0T_DRL, 
		 TOP_GP16_SHR16_G0T_DRL, 
		 TOP_GP16_SHR32_G0T_DRL, 
		 TOP_GP32_NOTG_GT_BR, 
		 TOP_GP32_SETP15U_GT_AR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_84; 
  print_84 = ISA_Print_Type_Create("print_84", "%s 	%s @( %s ?+ %s ) , %s"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 
  Operand(3); 

  Instruction_Print_Group(print_84, 
		 TOP_GP32_SAH_GT_AR_QP_U5_AR, 
		 TOP_GP32_SAW_GT_AR_QP_U5_AR, 
		 TOP_GP32_SCW_GT_AR_QP_U5_CRH, 
		 TOP_GP32_SCW_GT_AR_QP_U5_CRL, 
		 TOP_GP32_SDBP_GT_AR_QP_U5_DR, 
		 TOP_GP32_SDBSW_GT_AR_QP_U5_DR, 
		 TOP_GP32_SDB_GT_AR_QP_U5_DR, 
		 TOP_GP32_SDEW_GT_AR_QP_U5_DR, 
		 TOP_GP32_SDF_GT_AR_QP_U5_DR, 
		 TOP_GP32_SDHSW_GT_AR_QP_U5_DR, 
		 TOP_GP32_SDH_GT_AR_QP_U5_DR, 
		 TOP_GP32_SDP_GT_AR_QP_U5_DR, 
		 TOP_GP32_SDW_GT_AR_QP_U5_DR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_85; 
  print_85 = ISA_Print_Type_Create("print_85", "%s 	%s @( %s + %s ) , %s"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 
  Operand(3); 

  Instruction_Print_Group(print_85, 
		 TOP_GP16_SAW_G0T_ARL_P_U4_ARL, 
		 TOP_GP16_SAW_G0T_P13_P_U8_ARL, 
		 TOP_GP16_SAW_G0T_P15_P_U6_ARL, 
		 TOP_GP16_SDB_G0T_ARL_P_U4_DRL, 
		 TOP_GP16_SDB_G0T_P13_P_U8_DRL, 
		 TOP_GP16_SDB_G0T_P15_P_U6_DRL, 
		 TOP_GP16_SDH_G0T_ARL_P_U4_DRL, 
		 TOP_GP16_SDH_G0T_P13_P_U8_DRL, 
		 TOP_GP16_SDH_G0T_P15_P_U6_DRL, 
		 TOP_GP16_SDW_G0T_ARL_P_U4_DRL, 
		 TOP_GP16_SDW_G0T_P13_P_U8_DRL, 
		 TOP_GP16_SDW_G0T_P15_P_U6_DRL, 
		 TOP_GP32_SAH_GT_AR_P_U9_AR, 
		 TOP_GP32_SAH_GT_P13_P_U15_AR, 
		 TOP_GP32_SAW_GT_AR_P_U9_AR, 
		 TOP_GP32_SAW_GT_P13_P_U15_AR, 
		 TOP_GP32_SCW_GT_AR_P_U9_CRH, 
		 TOP_GP32_SCW_GT_AR_P_U9_CRL, 
		 TOP_GP32_SCW_GT_P13_P_U15_CRH, 
		 TOP_GP32_SCW_GT_P13_P_U15_CRL, 
		 TOP_GP32_SDBP_GT_AR_P_U9_DR, 
		 TOP_GP32_SDBP_GT_P13_P_U15_DR, 
		 TOP_GP32_SDBSW_GT_AR_P_U9_DR, 
		 TOP_GP32_SDBSW_GT_P13_P_U15_DR, 
		 TOP_GP32_SDB_GT_AR_P_U9_DR, 
		 TOP_GP32_SDB_GT_P13_P_U15_DR, 
		 TOP_GP32_SDEW_GT_AR_P_U9_DR, 
		 TOP_GP32_SDEW_GT_P13_P_U15_DR, 
		 TOP_GP32_SDF_GT_AR_P_U9_DR, 
		 TOP_GP32_SDF_GT_P13_P_U15_DR, 
		 TOP_GP32_SDHSW_GT_AR_P_U9_DR, 
		 TOP_GP32_SDHSW_GT_P13_P_U15_DR, 
		 TOP_GP32_SDH_GT_AR_P_U9_DR, 
		 TOP_GP32_SDH_GT_P13_P_U15_DR, 
		 TOP_GP32_SDP_GT_AR_P_U9_DR, 
		 TOP_GP32_SDP_GT_P13_P_U15_DR, 
		 TOP_GP32_SDW_GT_AR_P_U9_DR, 
		 TOP_GP32_SDW_GT_P13_P_U15_DR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_86; 
  print_86 = ISA_Print_Type_Create("print_86", "%s 	%s %s , %s , %s"); 
  Operand(0); 
  Name(); 
  Result(0); 
  Result(1); 
  Operand(1); 

  Instruction_Print_Group(print_86, 
		 TOP_GP32_FMOVEA_GT_BR_AR_AR, 
		 TOP_GP32_TMOVEA_GT_BR_AR_AR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_87; 
  print_87 = ISA_Print_Type_Create("print_87", "%s 	%s %s , @( %s - %s )"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 
  Operand(3); 

  Instruction_Print_Group(print_87, 
		 TOP_GP32_LCW_GT_CRH_AR_M_U9, 
		 TOP_GP32_LDEW_GT_DR_AR_M_U9, 
		 TOP_GP32_LDHH_GT_DR_AR_M_U9, 
		 TOP_GP32_LDLH_GT_DR_AR_M_U9, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_88; 
  print_88 = ISA_Print_Type_Create("print_88", "%s 	%s %s , %s , %s , %s"); 
  Operand(0); 
  Name(); 
  Result(0); 
  Result(1); 
  Operand(1); 
  Operand(2); 

  Instruction_Print_Group(print_88, 
		 TOP_GP32_FBCLRP_GT_BR_DR_DR_U4, 
		 TOP_GP32_FBCLR_GT_BR_DR_DR_U5, 
		 TOP_GP32_FBNOTP_GT_BR_DR_DR_U4, 
		 TOP_GP32_FBNOT_GT_BR_DR_DR_U5, 
		 TOP_GP32_FBSETP_GT_BR_DR_DR_U4, 
		 TOP_GP32_FBSET_GT_BR_DR_DR_U5, 
		 TOP_GP32_TBCLRP_GT_BR_DR_DR_U4, 
		 TOP_GP32_TBCLR_GT_BR_DR_DR_U5, 
		 TOP_GP32_TBNOTP_GT_BR_DR_DR_U4, 
		 TOP_GP32_TBNOT_GT_BR_DR_DR_U5, 
		 TOP_GP32_TBSETP_GT_BR_DR_DR_U4, 
		 TOP_GP32_TBSET_GT_BR_DR_DR_U5, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_89; 
  print_89 = ISA_Print_Type_Create("print_89", "%s 	%s @( %s ?+ %s ) , %s"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 
  Operand(3); 

  Instruction_Print_Group(print_89, 
		 TOP_GP32_SAH_GT_AR_QP_AR_AR, 
		 TOP_GP32_SAW_GT_AR_QP_AR_AR, 
		 TOP_GP32_SCW_GT_AR_QP_AR_CRH, 
		 TOP_GP32_SCW_GT_AR_QP_AR_CRL, 
		 TOP_GP32_SDBP_GT_AR_QP_AR_DR, 
		 TOP_GP32_SDBSW_GT_AR_QP_AR_DR, 
		 TOP_GP32_SDB_GT_AR_QP_AR_DR, 
		 TOP_GP32_SDEW_GT_AR_QP_AR_DR, 
		 TOP_GP32_SDF_GT_AR_QP_AR_DR, 
		 TOP_GP32_SDHSW_GT_AR_QP_AR_DR, 
		 TOP_GP32_SDH_GT_AR_QP_AR_DR, 
		 TOP_GP32_SDP_GT_AR_QP_AR_DR, 
		 TOP_GP32_SDW_GT_AR_QP_AR_DR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_90; 
  print_90 = ISA_Print_Type_Create("print_90", "%s 	%s @( %s + %s ) , %s"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 
  Operand(3); 

  Instruction_Print_Group(print_90, 
		 TOP_GP16_SAW_G0T_ARH_P_P3_ARL, 
		 TOP_GP16_SAW_G0T_ARL_P_P3_ARL, 
		 TOP_GP16_SDB_G0T_ARH_P_P3_DRL, 
		 TOP_GP16_SDB_G0T_ARL_P_P3_DRL, 
		 TOP_GP16_SDH_G0T_ARH_P_P3_DRL, 
		 TOP_GP16_SDH_G0T_ARL_P_P3_DRL, 
		 TOP_GP16_SDW_G0T_ARH_P_P3_DRL, 
		 TOP_GP16_SDW_G0T_ARL_P_P3_DRL, 
		 TOP_GP32_SAH_GT_AR_P_AR_AR, 
		 TOP_GP32_SAW_GT_AR_P_AR_AR, 
		 TOP_GP32_SCW_GT_AR_P_AR_CRH, 
		 TOP_GP32_SCW_GT_AR_P_AR_CRL, 
		 TOP_GP32_SDBP_GT_AR_P_AR_DR, 
		 TOP_GP32_SDBSW_GT_AR_P_AR_DR, 
		 TOP_GP32_SDB_GT_AR_P_AR_DR, 
		 TOP_GP32_SDEW_GT_AR_P_AR_DR, 
		 TOP_GP32_SDF_GT_AR_P_AR_DR, 
		 TOP_GP32_SDHSW_GT_AR_P_AR_DR, 
		 TOP_GP32_SDH_GT_AR_P_AR_DR, 
		 TOP_GP32_SDP_GT_AR_P_AR_DR, 
		 TOP_GP32_SDW_GT_AR_P_AR_DR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_91; 
  print_91 = ISA_Print_Type_Create("print_91", "%s 	%s %s , @( - %s )"); 
  Operand(0); 
  Name(); 
  Result(0); 
  Operand(1); 

  Instruction_Print_Group(print_91, 
		 TOP_GP16_LAW_G0T_ARL_MQ_P15, 
		 TOP_GP16_LDB_G0T_DRL_MQ_P15, 
		 TOP_GP16_LDH_G0T_DRL_MQ_P15, 
		 TOP_GP16_LDUB_G0T_DRL_MQ_P15, 
		 TOP_GP16_LDUH_G0T_DRL_MQ_P15, 
		 TOP_GP16_LDUW_G0T_DRL_MQ_P15, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_92; 
  print_92 = ISA_Print_Type_Create("print_92", "%s 	%s %s %s , @( %s ?+ %s )"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Result(0); 
  Operand(2); 
  Operand(3); 

  Instruction_Print_Group(print_92, 
		 TOP_GP32_LAH_GT_MD_AR_AR_QP_AR, 
		 TOP_GP32_LAW_GT_MD_AR_AR_QP_AR, 
		 TOP_GP32_LCG_GT_MD_BR_AR_QP_AR, 
		 TOP_GP32_LCW_GT_MD_CRL_AR_QP_AR, 
		 TOP_GP32_LDBP_GT_MD_DR_AR_QP_AR, 
		 TOP_GP32_LDBSW_GT_MD_DR_AR_QP_AR, 
		 TOP_GP32_LDB_GT_MD_DR_AR_QP_AR, 
		 TOP_GP32_LDF_GT_MD_DR_AR_QP_AR, 
		 TOP_GP32_LDHSW_GT_MD_DR_AR_QP_AR, 
		 TOP_GP32_LDH_GT_MD_DR_AR_QP_AR, 
		 TOP_GP32_LDP_GT_MD_DR_AR_QP_AR, 
		 TOP_GP32_LDSETUB_GT_MD_DR_AR_QP_AR, 
		 TOP_GP32_LDUBP_GT_MD_DR_AR_QP_AR, 
		 TOP_GP32_LDUB_GT_MD_DR_AR_QP_AR, 
		 TOP_GP32_LDUH_GT_MD_DR_AR_QP_AR, 
		 TOP_GP32_LDUW_GT_MD_DR_AR_QP_AR, 
		 TOP_GP32_LDW_GT_MD_DR_AR_QP_AR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_93; 
  print_93 = ISA_Print_Type_Create("print_93", "	%s %s , %s"); 
  Name(); 
  Result(0); 
  Operand(0); 

  Instruction_Print_Group(print_93, 
		 TOP_GP16_FA_G0_ARL, 
		 TOP_GP16_TA_G0_ARL, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_94; 
  print_94 = ISA_Print_Type_Create("print_94", "%s 	%s %s %s , @( %s ?- %s )"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Result(0); 
  Operand(2); 
  Operand(3); 

  Instruction_Print_Group(print_94, 
		 TOP_GP32_LAH_GT_MD_AR_AR_QM_AR, 
		 TOP_GP32_LAW_GT_MD_AR_AR_QM_AR, 
		 TOP_GP32_LCG_GT_MD_BR_AR_QM_AR, 
		 TOP_GP32_LCW_GT_MD_CRL_AR_QM_AR, 
		 TOP_GP32_LDBP_GT_MD_DR_AR_QM_AR, 
		 TOP_GP32_LDBSW_GT_MD_DR_AR_QM_AR, 
		 TOP_GP32_LDB_GT_MD_DR_AR_QM_AR, 
		 TOP_GP32_LDF_GT_MD_DR_AR_QM_AR, 
		 TOP_GP32_LDHSW_GT_MD_DR_AR_QM_AR, 
		 TOP_GP32_LDH_GT_MD_DR_AR_QM_AR, 
		 TOP_GP32_LDP_GT_MD_DR_AR_QM_AR, 
		 TOP_GP32_LDSETUB_GT_MD_DR_AR_QM_AR, 
		 TOP_GP32_LDUBP_GT_MD_DR_AR_QM_AR, 
		 TOP_GP32_LDUB_GT_MD_DR_AR_QM_AR, 
		 TOP_GP32_LDUH_GT_MD_DR_AR_QM_AR, 
		 TOP_GP32_LDUW_GT_MD_DR_AR_QM_AR, 
		 TOP_GP32_LDW_GT_MD_DR_AR_QM_AR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_95; 
  print_95 = ISA_Print_Type_Create("print_95", "%s 	%s %s , @( %s ?- %s )"); 
  Operand(0); 
  Name(); 
  Result(0); 
  Operand(1); 
  Operand(2); 

  Instruction_Print_Group(print_95, 
		 TOP_GP32_LAH_GT_AR_AR_QM_AR, 
		 TOP_GP32_LAW_GT_AR_AR_QM_AR, 
		 TOP_GP32_LCG_GT_BR_AR_QM_AR, 
		 TOP_GP32_LCW_GT_CRL_AR_QM_AR, 
		 TOP_GP32_LDBP_GT_DR_AR_QM_AR, 
		 TOP_GP32_LDBSW_GT_DR_AR_QM_AR, 
		 TOP_GP32_LDB_GT_DR_AR_QM_AR, 
		 TOP_GP32_LDF_GT_DR_AR_QM_AR, 
		 TOP_GP32_LDHSW_GT_DR_AR_QM_AR, 
		 TOP_GP32_LDH_GT_DR_AR_QM_AR, 
		 TOP_GP32_LDP_GT_DR_AR_QM_AR, 
		 TOP_GP32_LDSETUB_GT_DR_AR_QM_AR, 
		 TOP_GP32_LDUBP_GT_DR_AR_QM_AR, 
		 TOP_GP32_LDUB_GT_DR_AR_QM_AR, 
		 TOP_GP32_LDUH_GT_DR_AR_QM_AR, 
		 TOP_GP32_LDUW_GT_DR_AR_QM_AR, 
		 TOP_GP32_LDW_GT_DR_AR_QM_AR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_96; 
  print_96 = ISA_Print_Type_Create("print_96", "%s 	%s %s @( %s + %s )"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 
  Operand(3); 

  Instruction_Print_Group(print_96, 
		 TOP_GP32_LFR_GT_MD_AR_P_U5, 
		 TOP_GP32_LGR_GT_MD_AR_P_U5, 
		 TOP_GP32_SFR_GT_MD_AR_P_U5, 
		 TOP_GP32_SGR_GT_MD_AR_P_U5, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_97; 
  print_97 = ISA_Print_Type_Create("print_97", "	%s %s , %s , %s"); 
  Name(); 
  Result(0); 
  Operand(0); 
  Operand(1); 

  Instruction_Print_Group(print_97, 
		 TOP_GP16_EQW_G0_DRL_U5, 
		 TOP_GP16_GEUW_G0_DRL_U5, 
		 TOP_GP16_GEW_G0_DRL_U5, 
		 TOP_GP16_GTUW_G0_DRL_U5, 
		 TOP_GP16_GTW_G0_DRL_U5, 
		 TOP_GP16_LEUW_G0_DRL_U5, 
		 TOP_GP16_LEW_G0_DRL_U5, 
		 TOP_GP16_LTUW_G0_DRL_U5, 
		 TOP_GP16_LTW_G0_DRL_U5, 
		 TOP_GP16_NEW_G0_DRL_U5, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_98; 
  print_98 = ISA_Print_Type_Create("print_98", "%s 	%s %s , %s , %s"); 
  Operand(0); 
  Name(); 
  Result(0); 
  Operand(1); 
  Operand(2); 

  Instruction_Print_Group(print_98, 
		 TOP_GP32_ANDNP_GT_DR_U8_DR, 
		 TOP_GP32_ANDN_GT_DR_U8_DR, 
		 TOP_GP32_FEANDN_GT_BR_U8_DR, 
		 TOP_GP32_FEORN_GT_BR_U8_DR, 
		 TOP_GP32_FPANDN_GT_BR_U8_DR, 
		 TOP_GP32_FPORN_GT_BR_U8_DR, 
		 TOP_GP32_FWANDN_GT_BR_U8_DR, 
		 TOP_GP32_FWORN_GT_BR_U8_DR, 
		 TOP_GP32_ORNP_GT_DR_U8_DR, 
		 TOP_GP32_ORN_GT_DR_U8_DR, 
		 TOP_GP32_TEANDN_GT_BR_U8_DR, 
		 TOP_GP32_TEORN_GT_BR_U8_DR, 
		 TOP_GP32_TPANDN_GT_BR_U8_DR, 
		 TOP_GP32_TPORN_GT_BR_U8_DR, 
		 TOP_GP32_TWANDN_GT_BR_U8_DR, 
		 TOP_GP32_TWORN_GT_BR_U8_DR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_99; 
  print_99 = ISA_Print_Type_Create("print_99", "%s 	%s"); 
  Operand(0); 
  Name(); 

  Instruction_Print_Group(print_99, 
		 TOP_GP16_BKP_G0F, 
		 TOP_GP16_BRANCH_G0F, 
		 TOP_GP32_BKP_GF, 
		 TOP_GP32_BRANCH_GF, 
		 TOP_GP32_JUMP_GF, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_100; 
  print_100 = ISA_Print_Type_Create("print_100", "	%s %s"); 
  Name(); 
  Result(0); 

  Instruction_Print_Group(print_100, 
		 TOP_GP16_POPRTS_RSET, 
		 TOP_GP16_POP_RSET, 
		 TOP_GP16_PUSH_RSET, 
		 TOP_GP32_POPRTE_RSET, 
		 TOP_GP32_POPRTS_RSET, 
		 TOP_GP32_POP_RSET, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_101; 
  print_101 = ISA_Print_Type_Create("print_101", "%s 	%s %s , @( %s ?+ %s )"); 
  Operand(0); 
  Name(); 
  Result(0); 
  Operand(1); 
  Operand(2); 

  Instruction_Print_Group(print_101, 
		 TOP_GP32_LAH_GT_AR_AR_QP_U5, 
		 TOP_GP32_LAW_GT_AR_AR_QP_U5, 
		 TOP_GP32_LCG_GT_BR_AR_QP_U5, 
		 TOP_GP32_LCW_GT_CRL_AR_QP_U5, 
		 TOP_GP32_LDBP_GT_DR_AR_QP_U5, 
		 TOP_GP32_LDBSW_GT_DR_AR_QP_U5, 
		 TOP_GP32_LDB_GT_DR_AR_QP_U5, 
		 TOP_GP32_LDF_GT_DR_AR_QP_U5, 
		 TOP_GP32_LDHSW_GT_DR_AR_QP_U5, 
		 TOP_GP32_LDH_GT_DR_AR_QP_U5, 
		 TOP_GP32_LDP_GT_DR_AR_QP_U5, 
		 TOP_GP32_LDSETUB_GT_DR_AR_QP_U5, 
		 TOP_GP32_LDUBP_GT_DR_AR_QP_U5, 
		 TOP_GP32_LDUB_GT_DR_AR_QP_U5, 
		 TOP_GP32_LDUH_GT_DR_AR_QP_U5, 
		 TOP_GP32_LDUW_GT_DR_AR_QP_U5, 
		 TOP_GP32_LDW_GT_DR_AR_QP_U5, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_102; 
  print_102 = ISA_Print_Type_Create("print_102", "%s 	%s %s , %s , %s , %s"); 
  Operand(0); 
  Name(); 
  Result(0); 
  Operand(1); 
  Operand(2); 
  Operand(3); 

  Instruction_Print_Group(print_102, 
		 TOP_GP32_XSHLW_GT_DR_DR_DR_U5, 
		 TOP_GP32_XSHRW_GT_DR_DR_DR_U5, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_103; 
  print_103 = ISA_Print_Type_Create("print_103", "%s 	%s %s %s , @( %s ?+ %s )"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 
  Operand(3); 
  Operand(4); 

  Instruction_Print_Group(print_103, 
		 TOP_GP32_LCW_GT_MD_CRH_AR_QP_AR, 
		 TOP_GP32_LDEW_GT_MD_DR_AR_QP_AR, 
		 TOP_GP32_LDHH_GT_MD_DR_AR_QP_AR, 
		 TOP_GP32_LDLH_GT_MD_DR_AR_QP_AR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_104; 
  print_104 = ISA_Print_Type_Create("print_104", "%s 	%s %s %s , @( %s ?- %s )"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 
  Operand(3); 
  Operand(4); 

  Instruction_Print_Group(print_104, 
		 TOP_GP32_LCW_GT_MD_CRH_AR_QM_AR, 
		 TOP_GP32_LDEW_GT_MD_DR_AR_QM_AR, 
		 TOP_GP32_LDHH_GT_MD_DR_AR_QM_AR, 
		 TOP_GP32_LDLH_GT_MD_DR_AR_QM_AR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_105; 
  print_105 = ISA_Print_Type_Create("print_105", "%s 	%s %s , @( %s ?- %s )"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 
  Operand(3); 

  Instruction_Print_Group(print_105, 
		 TOP_GP32_LCW_GT_CRH_AR_QM_AR, 
		 TOP_GP32_LDEW_GT_DR_AR_QM_AR, 
		 TOP_GP32_LDHH_GT_DR_AR_QM_AR, 
		 TOP_GP32_LDLH_GT_DR_AR_QM_AR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_106; 
  print_106 = ISA_Print_Type_Create("print_106", "%s 	%s %s @( %s !- %s ) , %s"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 
  Operand(3); 
  Operand(4); 

  Instruction_Print_Group(print_106, 
		 TOP_GP32_SAH_GT_MD_AR_BM_AR_AR, 
		 TOP_GP32_SAW_GT_MD_AR_BM_AR_AR, 
		 TOP_GP32_SCW_GT_MD_AR_BM_AR_CRH, 
		 TOP_GP32_SCW_GT_MD_AR_BM_AR_CRL, 
		 TOP_GP32_SDBP_GT_MD_AR_BM_AR_DR, 
		 TOP_GP32_SDBSW_GT_MD_AR_BM_AR_DR, 
		 TOP_GP32_SDB_GT_MD_AR_BM_AR_DR, 
		 TOP_GP32_SDEW_GT_MD_AR_BM_AR_DR, 
		 TOP_GP32_SDF_GT_MD_AR_BM_AR_DR, 
		 TOP_GP32_SDHSW_GT_MD_AR_BM_AR_DR, 
		 TOP_GP32_SDH_GT_MD_AR_BM_AR_DR, 
		 TOP_GP32_SDP_GT_MD_AR_BM_AR_DR, 
		 TOP_GP32_SDW_GT_MD_AR_BM_AR_DR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_107; 
  print_107 = ISA_Print_Type_Create("print_107", "%s 	%s %s %s , @( %s !+ %s )"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Result(0); 
  Operand(2); 
  Operand(3); 

  Instruction_Print_Group(print_107, 
		 TOP_GP32_LAH_GT_MD_AR_AR_BP_AR, 
		 TOP_GP32_LAW_GT_MD_AR_AR_BP_AR, 
		 TOP_GP32_LCG_GT_MD_BR_AR_BP_AR, 
		 TOP_GP32_LCW_GT_MD_CRL_AR_BP_AR, 
		 TOP_GP32_LDBP_GT_MD_DR_AR_BP_AR, 
		 TOP_GP32_LDBSW_GT_MD_DR_AR_BP_AR, 
		 TOP_GP32_LDB_GT_MD_DR_AR_BP_AR, 
		 TOP_GP32_LDF_GT_MD_DR_AR_BP_AR, 
		 TOP_GP32_LDHSW_GT_MD_DR_AR_BP_AR, 
		 TOP_GP32_LDH_GT_MD_DR_AR_BP_AR, 
		 TOP_GP32_LDP_GT_MD_DR_AR_BP_AR, 
		 TOP_GP32_LDSETUB_GT_MD_DR_AR_BP_AR, 
		 TOP_GP32_LDUBP_GT_MD_DR_AR_BP_AR, 
		 TOP_GP32_LDUB_GT_MD_DR_AR_BP_AR, 
		 TOP_GP32_LDUH_GT_MD_DR_AR_BP_AR, 
		 TOP_GP32_LDUW_GT_MD_DR_AR_BP_AR, 
		 TOP_GP32_LDW_GT_MD_DR_AR_BP_AR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_108; 
  print_108 = ISA_Print_Type_Create("print_108", "%s 	%s %s , @( %s !+ %s )"); 
  Operand(0); 
  Name(); 
  Result(0); 
  Operand(1); 
  Operand(2); 

  Instruction_Print_Group(print_108, 
		 TOP_GP32_LAH_GT_AR_AR_BP_AR, 
		 TOP_GP32_LAW_GT_AR_AR_BP_AR, 
		 TOP_GP32_LCG_GT_BR_AR_BP_AR, 
		 TOP_GP32_LCW_GT_CRL_AR_BP_AR, 
		 TOP_GP32_LDBP_GT_DR_AR_BP_AR, 
		 TOP_GP32_LDBSW_GT_DR_AR_BP_AR, 
		 TOP_GP32_LDB_GT_DR_AR_BP_AR, 
		 TOP_GP32_LDF_GT_DR_AR_BP_AR, 
		 TOP_GP32_LDHSW_GT_DR_AR_BP_AR, 
		 TOP_GP32_LDH_GT_DR_AR_BP_AR, 
		 TOP_GP32_LDP_GT_DR_AR_BP_AR, 
		 TOP_GP32_LDSETUB_GT_DR_AR_BP_AR, 
		 TOP_GP32_LDUBP_GT_DR_AR_BP_AR, 
		 TOP_GP32_LDUB_GT_DR_AR_BP_AR, 
		 TOP_GP32_LDUH_GT_DR_AR_BP_AR, 
		 TOP_GP32_LDUW_GT_DR_AR_BP_AR, 
		 TOP_GP32_LDW_GT_DR_AR_BP_AR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_109; 
  print_109 = ISA_Print_Type_Create("print_109", "%s 	%s %s %s , @( %s !- %s )"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Result(0); 
  Operand(2); 
  Operand(3); 

  Instruction_Print_Group(print_109, 
		 TOP_GP32_LAH_GT_MD_AR_AR_BM_AR, 
		 TOP_GP32_LAW_GT_MD_AR_AR_BM_AR, 
		 TOP_GP32_LCG_GT_MD_BR_AR_BM_AR, 
		 TOP_GP32_LCW_GT_MD_CRL_AR_BM_AR, 
		 TOP_GP32_LDBP_GT_MD_DR_AR_BM_AR, 
		 TOP_GP32_LDBSW_GT_MD_DR_AR_BM_AR, 
		 TOP_GP32_LDB_GT_MD_DR_AR_BM_AR, 
		 TOP_GP32_LDF_GT_MD_DR_AR_BM_AR, 
		 TOP_GP32_LDHSW_GT_MD_DR_AR_BM_AR, 
		 TOP_GP32_LDH_GT_MD_DR_AR_BM_AR, 
		 TOP_GP32_LDP_GT_MD_DR_AR_BM_AR, 
		 TOP_GP32_LDSETUB_GT_MD_DR_AR_BM_AR, 
		 TOP_GP32_LDUBP_GT_MD_DR_AR_BM_AR, 
		 TOP_GP32_LDUB_GT_MD_DR_AR_BM_AR, 
		 TOP_GP32_LDUH_GT_MD_DR_AR_BM_AR, 
		 TOP_GP32_LDUW_GT_MD_DR_AR_BM_AR, 
		 TOP_GP32_LDW_GT_MD_DR_AR_BM_AR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_110; 
  print_110 = ISA_Print_Type_Create("print_110", "%s 	%s @( %s ?+ %s )"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 

  Instruction_Print_Group(print_110, 
		 TOP_GP32_LFR_GT_AR_QP_U5, 
		 TOP_GP32_LGR_GT_AR_QP_U5, 
		 TOP_GP32_SFR_GT_AR_QP_U5, 
		 TOP_GP32_SGR_GT_AR_QP_U5, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_111; 
  print_111 = ISA_Print_Type_Create("print_111", "%s 	%s @( %s ?- %s )"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 

  Instruction_Print_Group(print_111, 
		 TOP_GP32_LFR_GT_AR_QM_U5, 
		 TOP_GP32_LGR_GT_AR_QM_U5, 
		 TOP_GP32_SFR_GT_AR_QM_U5, 
		 TOP_GP32_SGR_GT_AR_QM_U5, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_112; 
  print_112 = ISA_Print_Type_Create("print_112", "	%s"); 
  Name(); 

  Instruction_Print_Group(print_112, 
		 TOP_GP16_BARRIER, 
		 TOP_GP16_BKP, 
		 TOP_GP16_BRANCH, 
		 TOP_GP16_GP32MD, 
		 TOP_GP16_GP32NXT, 
		 TOP_GP16_JUMP, 
		 TOP_GP16_NOP, 
		 TOP_GP16_RTS, 
		 TOP_GP32_BARRIER, 
		 TOP_GP32_BRANCH, 
		 TOP_GP32_GP16MD, 
		 TOP_GP32_GP32MD, 
		 TOP_GP32_JUMP, 
		 TOP_GP32_LOOPDIS, 
		 TOP_GP32_LOOPENA, 
		 TOP_GP32_NOP, 
		 TOP_GP32_RTE, 
		 TOP_GP32_SLIWMD, 
		 TOP_GP32_SWNMI, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_113; 
  print_113 = ISA_Print_Type_Create("print_113", "%s 	%s %s @( %s ?+ %s )"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 
  Operand(3); 

  Instruction_Print_Group(print_113, 
		 TOP_GP32_LFR_GT_MD_AR_QP_AR, 
		 TOP_GP32_LGR_GT_MD_AR_QP_AR, 
		 TOP_GP32_SFR_GT_MD_AR_QP_AR, 
		 TOP_GP32_SGR_GT_MD_AR_QP_AR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_114; 
  print_114 = ISA_Print_Type_Create("print_114", "%s 	%s"); 
  Operand(0); 
  Name(); 

  Instruction_Print_Group(print_114, 
		 TOP_GP16_BKPNXT_G0T, 
		 TOP_GP16_BKP_G0T, 
		 TOP_GP16_LINK_G0T, 
		 TOP_GP32_BKP_GT, 
		 TOP_GP32_CLRFR_GT, 
		 TOP_GP32_CLRSCL_GT, 
		 TOP_GP32_CLRSNR_GT, 
		 TOP_GP32_CLRSVE_GT, 
		 TOP_GP32_CLRSVH_GT, 
		 TOP_GP32_CLRSVL_GT, 
		 TOP_GP32_CLRSVP_GT, 
		 TOP_GP32_CLRSVW_GT, 
		 TOP_GP32_LINK_GT, 
		 TOP_GP32_RTS_GT, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_115; 
  print_115 = ISA_Print_Type_Create("print_115", "%s 	%s %s , %s"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 

  Instruction_Print_Group(print_115, 
		 TOP_GP32_BFPSR0_GT_U8_U8, 
		 TOP_GP32_BFPSR1_GT_U8_U8, 
		 TOP_GP32_BFPSR2_GT_U8_U8, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_116; 
  print_116 = ISA_Print_Type_Create("print_116", "%s 	%s %s @( %s ?+ %s ) , %s"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 
  Operand(3); 
  Operand(4); 

  Instruction_Print_Group(print_116, 
		 TOP_GP32_SAH_GT_MD_AR_QP_U5_AR, 
		 TOP_GP32_SAW_GT_MD_AR_QP_U5_AR, 
		 TOP_GP32_SCW_GT_MD_AR_QP_U5_CRH, 
		 TOP_GP32_SCW_GT_MD_AR_QP_U5_CRL, 
		 TOP_GP32_SDBP_GT_MD_AR_QP_U5_DR, 
		 TOP_GP32_SDBSW_GT_MD_AR_QP_U5_DR, 
		 TOP_GP32_SDB_GT_MD_AR_QP_U5_DR, 
		 TOP_GP32_SDEW_GT_MD_AR_QP_U5_DR, 
		 TOP_GP32_SDF_GT_MD_AR_QP_U5_DR, 
		 TOP_GP32_SDHSW_GT_MD_AR_QP_U5_DR, 
		 TOP_GP32_SDH_GT_MD_AR_QP_U5_DR, 
		 TOP_GP32_SDP_GT_MD_AR_QP_U5_DR, 
		 TOP_GP32_SDW_GT_MD_AR_QP_U5_DR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_117; 
  print_117 = ISA_Print_Type_Create("print_117", "%s 	%s %s , @( %s ?+ %s )"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 
  Operand(3); 

  Instruction_Print_Group(print_117, 
		 TOP_GP32_LCW_GT_CRH_AR_QP_U5, 
		 TOP_GP32_LDEW_GT_DR_AR_QP_U5, 
		 TOP_GP32_LDHH_GT_DR_AR_QP_U5, 
		 TOP_GP32_LDLH_GT_DR_AR_QP_U5, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_118; 
  print_118 = ISA_Print_Type_Create("print_118", "%s 	%s %s , @( %s + %s + %s )"); 
  Operand(0); 
  Name(); 
  Result(0); 
  Operand(1); 
  Operand(2); 
  Name(); 

  Instruction_Print_Group(print_118, 
		 TOP_GP16_LAW_G0T_ARL_P13_PR_U5, 
		 TOP_GP16_LDB_G0T_DRL_P13_PR_U5, 
		 TOP_GP16_LDH_G0T_DRL_P13_PR_U5, 
		 TOP_GP16_LDUB_G0T_DRL_P13_PR_U5, 
		 TOP_GP16_LDUH_G0T_DRL_P13_PR_U5, 
		 TOP_GP16_LDUW_G0T_DRL_P13_PR_U5, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_119; 
  print_119 = ISA_Print_Type_Create("print_119", "%s 	%s %s , @( %s !- %s )"); 
  Operand(0); 
  Name(); 
  Result(0); 
  Operand(1); 
  Operand(2); 

  Instruction_Print_Group(print_119, 
		 TOP_GP32_LAH_GT_AR_AR_BM_U5, 
		 TOP_GP32_LAW_GT_AR_AR_BM_U5, 
		 TOP_GP32_LCG_GT_BR_AR_BM_U5, 
		 TOP_GP32_LCW_GT_CRL_AR_BM_U5, 
		 TOP_GP32_LDBP_GT_DR_AR_BM_U5, 
		 TOP_GP32_LDBSW_GT_DR_AR_BM_U5, 
		 TOP_GP32_LDB_GT_DR_AR_BM_U5, 
		 TOP_GP32_LDF_GT_DR_AR_BM_U5, 
		 TOP_GP32_LDHSW_GT_DR_AR_BM_U5, 
		 TOP_GP32_LDH_GT_DR_AR_BM_U5, 
		 TOP_GP32_LDP_GT_DR_AR_BM_U5, 
		 TOP_GP32_LDSETUB_GT_DR_AR_BM_U5, 
		 TOP_GP32_LDUBP_GT_DR_AR_BM_U5, 
		 TOP_GP32_LDUB_GT_DR_AR_BM_U5, 
		 TOP_GP32_LDUH_GT_DR_AR_BM_U5, 
		 TOP_GP32_LDUW_GT_DR_AR_BM_U5, 
		 TOP_GP32_LDW_GT_DR_AR_BM_U5, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_120; 
  print_120 = ISA_Print_Type_Create("print_120", "%s 	%s %s %s , @( %s !+ %s )"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 
  Operand(3); 
  Operand(4); 

  Instruction_Print_Group(print_120, 
		 TOP_GP32_LCW_GT_MD_CRH_AR_BP_AR, 
		 TOP_GP32_LDEW_GT_MD_DR_AR_BP_AR, 
		 TOP_GP32_LDHH_GT_MD_DR_AR_BP_AR, 
		 TOP_GP32_LDLH_GT_MD_DR_AR_BP_AR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_121; 
  print_121 = ISA_Print_Type_Create("print_121", "%s 	%s %s , @( %s !+ %s )"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 
  Operand(3); 

  Instruction_Print_Group(print_121, 
		 TOP_GP32_LCW_GT_CRH_AR_BP_AR, 
		 TOP_GP32_LDEW_GT_DR_AR_BP_AR, 
		 TOP_GP32_LDHH_GT_DR_AR_BP_AR, 
		 TOP_GP32_LDLH_GT_DR_AR_BP_AR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_122; 
  print_122 = ISA_Print_Type_Create("print_122", "%s 	%s %s %s , @( %s !- %s )"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 
  Operand(3); 
  Operand(4); 

  Instruction_Print_Group(print_122, 
		 TOP_GP32_LCW_GT_MD_CRH_AR_BM_AR, 
		 TOP_GP32_LDEW_GT_MD_DR_AR_BM_AR, 
		 TOP_GP32_LDHH_GT_MD_DR_AR_BM_AR, 
		 TOP_GP32_LDLH_GT_MD_DR_AR_BM_AR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_123; 
  print_123 = ISA_Print_Type_Create("print_123", "%s 	%s %s , @( %s - %s )"); 
  Operand(0); 
  Name(); 
  Result(0); 
  Operand(1); 
  Operand(2); 

  Instruction_Print_Group(print_123, 
		 TOP_GP32_LAH_GT_AR_AR_M_AR, 
		 TOP_GP32_LAW_GT_AR_AR_M_AR, 
		 TOP_GP32_LCG_GT_BR_AR_M_AR, 
		 TOP_GP32_LCW_GT_CRL_AR_M_AR, 
		 TOP_GP32_LDBP_GT_DR_AR_M_AR, 
		 TOP_GP32_LDBSW_GT_DR_AR_M_AR, 
		 TOP_GP32_LDB_GT_DR_AR_M_AR, 
		 TOP_GP32_LDF_GT_DR_AR_M_AR, 
		 TOP_GP32_LDHSW_GT_DR_AR_M_AR, 
		 TOP_GP32_LDH_GT_DR_AR_M_AR, 
		 TOP_GP32_LDP_GT_DR_AR_M_AR, 
		 TOP_GP32_LDSETUB_GT_DR_AR_M_AR, 
		 TOP_GP32_LDUBP_GT_DR_AR_M_AR, 
		 TOP_GP32_LDUB_GT_DR_AR_M_AR, 
		 TOP_GP32_LDUH_GT_DR_AR_M_AR, 
		 TOP_GP32_LDUW_GT_DR_AR_M_AR, 
		 TOP_GP32_LDW_GT_DR_AR_M_AR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_124; 
  print_124 = ISA_Print_Type_Create("print_124", "%s 	%s @( %s + %s )"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 

  Instruction_Print_Group(print_124, 
		 TOP_GP32_LFR_GT_AR_P_U9, 
		 TOP_GP32_LFR_GT_P13_P_U15, 
		 TOP_GP32_LGR_GT_AR_P_U9, 
		 TOP_GP32_LGR_GT_P13_P_U15, 
		 TOP_GP32_SFR_GT_AR_P_U9, 
		 TOP_GP32_SFR_GT_P13_P_U15, 
		 TOP_GP32_SGR_GT_AR_P_U9, 
		 TOP_GP32_SGR_GT_P13_P_U15, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_125; 
  print_125 = ISA_Print_Type_Create("print_125", "%s 	%s %s @( %s ?- %s )"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 
  Operand(3); 

  Instruction_Print_Group(print_125, 
		 TOP_GP32_LFR_GT_MD_AR_QM_U5, 
		 TOP_GP32_LGR_GT_MD_AR_QM_U5, 
		 TOP_GP32_SFR_GT_MD_AR_QM_U5, 
		 TOP_GP32_SGR_GT_MD_AR_QM_U5, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_126; 
  print_126 = ISA_Print_Type_Create("print_126", "%s 	%s @( %s !+ %s )"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 

  Instruction_Print_Group(print_126, 
		 TOP_GP32_LFR_GT_AR_BP_U5, 
		 TOP_GP32_LGR_GT_AR_BP_U5, 
		 TOP_GP32_SFR_GT_AR_BP_U5, 
		 TOP_GP32_SGR_GT_AR_BP_U5, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_127; 
  print_127 = ISA_Print_Type_Create("print_127", "%s 	%s @( %s - %s )"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 

  Instruction_Print_Group(print_127, 
		 TOP_GP32_LFR_GT_AR_M_U9, 
		 TOP_GP32_LGR_GT_AR_M_U9, 
		 TOP_GP32_SFR_GT_AR_M_U9, 
		 TOP_GP32_SGR_GT_AR_M_U9, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_128; 
  print_128 = ISA_Print_Type_Create("print_128", "%s 	%s %s"); 
  Operand(0); 
  Name(); 
  Operand(1); 

  Instruction_Print_Group(print_128, 
		 TOP_GP16_CALLPR_G0T_U9, 
		 TOP_GP16_CALL_G0T_S9, 
		 TOP_GP16_MAKESR_G0T_S11, 
		 TOP_GP16_MORESR_G0T_U10, 
		 TOP_GP32_CALLPR_GT_U16, 
		 TOP_GP32_CALL_GT_S21, 
		 TOP_GP32_MAKEPR_GT_S21, 
		 TOP_GP32_TRAP_GT_U4, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_129; 
  print_129 = ISA_Print_Type_Create("print_129", "%s 	%s @( %s !- %s )"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 

  Instruction_Print_Group(print_129, 
		 TOP_GP32_LFR_GT_AR_BM_U5, 
		 TOP_GP32_LGR_GT_AR_BM_U5, 
		 TOP_GP32_SFR_GT_AR_BM_U5, 
		 TOP_GP32_SGR_GT_AR_BM_U5, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_130; 
  print_130 = ISA_Print_Type_Create("print_130", "%s 	%s @( %s ?+ %s )"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 

  Instruction_Print_Group(print_130, 
		 TOP_GP32_LFR_GT_AR_QP_AR, 
		 TOP_GP32_LGR_GT_AR_QP_AR, 
		 TOP_GP32_SFR_GT_AR_QP_AR, 
		 TOP_GP32_SGR_GT_AR_QP_AR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_131; 
  print_131 = ISA_Print_Type_Create("print_131", "	%s %s"); 
  Name(); 
  Operand(0); 

  Instruction_Print_Group(print_131, 
		 TOP_GP16_NOTG_G0, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_132; 
  print_132 = ISA_Print_Type_Create("print_132", "%s 	%s @( %s ?- %s )"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 

  Instruction_Print_Group(print_132, 
		 TOP_GP32_LFR_GT_AR_QM_AR, 
		 TOP_GP32_LGR_GT_AR_QM_AR, 
		 TOP_GP32_SFR_GT_AR_QM_AR, 
		 TOP_GP32_SGR_GT_AR_QM_AR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_133; 
  print_133 = ISA_Print_Type_Create("print_133", "%s 	%s %s @( %s !- %s )"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 
  Operand(3); 

  Instruction_Print_Group(print_133, 
		 TOP_GP32_LFR_GT_MD_AR_BM_AR, 
		 TOP_GP32_LGR_GT_MD_AR_BM_AR, 
		 TOP_GP32_SFR_GT_MD_AR_BM_AR, 
		 TOP_GP32_SGR_GT_MD_AR_BM_AR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_134; 
  print_134 = ISA_Print_Type_Create("print_134", "%s 	%s %s %s , %s , %s"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Result(0); 
  Operand(2); 
  Operand(3); 

  Instruction_Print_Group(print_134, 
		 TOP_GP32_ADDBA_GT_MD_AR_AR_AR, 
		 TOP_GP32_ADDHA_GT_MD_AR_AR_AR, 
		 TOP_GP32_ADDWA_GT_MD_AR_AR_AR, 
		 TOP_GP32_SUBBA_GT_MD_AR_AR_AR, 
		 TOP_GP32_SUBHA_GT_MD_AR_AR_AR, 
		 TOP_GP32_SUBWA_GT_MD_AR_AR_AR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_135; 
  print_135 = ISA_Print_Type_Create("print_135", "%s 	%s %s @( %s + %s ) , %s"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Operand(2); 
  Operand(3); 
  Operand(4); 

  Instruction_Print_Group(print_135, 
		 TOP_GP32_SAH_GT_MD_AR_P_U5_AR, 
		 TOP_GP32_SAW_GT_MD_AR_P_U5_AR, 
		 TOP_GP32_SCW_GT_MD_AR_P_U5_CRH, 
		 TOP_GP32_SCW_GT_MD_AR_P_U5_CRL, 
		 TOP_GP32_SDBP_GT_MD_AR_P_U5_DR, 
		 TOP_GP32_SDBSW_GT_MD_AR_P_U5_DR, 
		 TOP_GP32_SDB_GT_MD_AR_P_U5_DR, 
		 TOP_GP32_SDEW_GT_MD_AR_P_U5_DR, 
		 TOP_GP32_SDF_GT_MD_AR_P_U5_DR, 
		 TOP_GP32_SDHSW_GT_MD_AR_P_U5_DR, 
		 TOP_GP32_SDH_GT_MD_AR_P_U5_DR, 
		 TOP_GP32_SDP_GT_MD_AR_P_U5_DR, 
		 TOP_GP32_SDW_GT_MD_AR_P_U5_DR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_136; 
  print_136 = ISA_Print_Type_Create("print_136", "%s 	%s %s"); 
  Operand(0); 
  Name(); 
  Result(0); 

  Instruction_Print_Group(print_136, 
		 TOP_GP32_CLRG_GT_BR, 
		 TOP_GP32_CLRPG_GT_BR, 
		 TOP_GP32_FCLFSCL_GT_BR, 
		 TOP_GP32_FCLFSNR_GT_BR, 
		 TOP_GP32_FCLFSVE_GT_BR, 
		 TOP_GP32_FCLFSVH_GT_BR, 
		 TOP_GP32_FCLFSVL_GT_BR, 
		 TOP_GP32_FCLFSVP_GT_BR, 
		 TOP_GP32_FCLFSVW_GT_BR, 
		 TOP_GP32_FCLRSCL_GT_BR, 
		 TOP_GP32_FCLRSNR_GT_BR, 
		 TOP_GP32_FCLRSVE_GT_BR, 
		 TOP_GP32_FCLRSVH_GT_BR, 
		 TOP_GP32_FCLRSVL_GT_BR, 
		 TOP_GP32_FCLRSVP_GT_BR, 
		 TOP_GP32_FCLRSVW_GT_BR, 
		 TOP_GP32_FSCL_GT_BR, 
		 TOP_GP32_FSNR_GT_BR, 
		 TOP_GP32_FSVE_GT_BR, 
		 TOP_GP32_FSVH_GT_BR, 
		 TOP_GP32_FSVL_GT_BR, 
		 TOP_GP32_FSVP_GT_BR, 
		 TOP_GP32_FSVW_GT_BR, 
		 TOP_GP32_GETP15U_GT_AR, 
		 TOP_GP32_SETG_GT_BR, 
		 TOP_GP32_SETPG_GT_BR, 
		 TOP_GP32_TCLFSCL_GT_BR, 
		 TOP_GP32_TCLFSNR_GT_BR, 
		 TOP_GP32_TCLFSVE_GT_BR, 
		 TOP_GP32_TCLFSVH_GT_BR, 
		 TOP_GP32_TCLFSVL_GT_BR, 
		 TOP_GP32_TCLFSVP_GT_BR, 
		 TOP_GP32_TCLFSVW_GT_BR, 
		 TOP_GP32_TCLRSCL_GT_BR, 
		 TOP_GP32_TCLRSNR_GT_BR, 
		 TOP_GP32_TCLRSVE_GT_BR, 
		 TOP_GP32_TCLRSVH_GT_BR, 
		 TOP_GP32_TCLRSVL_GT_BR, 
		 TOP_GP32_TCLRSVP_GT_BR, 
		 TOP_GP32_TCLRSVW_GT_BR, 
		 TOP_GP32_TSCL_GT_BR, 
		 TOP_GP32_TSNR_GT_BR, 
		 TOP_GP32_TSVE_GT_BR, 
		 TOP_GP32_TSVH_GT_BR, 
		 TOP_GP32_TSVL_GT_BR, 
		 TOP_GP32_TSVP_GT_BR, 
		 TOP_GP32_TSVW_GT_BR, 
		 TOP_UNDEFINED); 

  /* ================================= */ 
  ISA_PRINT_TYPE print_137; 
  print_137 = ISA_Print_Type_Create("print_137", "%s 	%s %s %s , @( %s ?+ %s )"); 
  Operand(0); 
  Name(); 
  Operand(1); 
  Result(0); 
  Operand(2); 
  Operand(3); 

  Instruction_Print_Group(print_137, 
		 TOP_GP32_LAH_GT_MD_AR_AR_QP_U5, 
		 TOP_GP32_LAW_GT_MD_AR_AR_QP_U5, 
		 TOP_GP32_LCG_GT_MD_BR_AR_QP_U5, 
		 TOP_GP32_LCW_GT_MD_CRL_AR_QP_U5, 
		 TOP_GP32_LDBP_GT_MD_DR_AR_QP_U5, 
		 TOP_GP32_LDBSW_GT_MD_DR_AR_QP_U5, 
		 TOP_GP32_LDB_GT_MD_DR_AR_QP_U5, 
		 TOP_GP32_LDF_GT_MD_DR_AR_QP_U5, 
		 TOP_GP32_LDHSW_GT_MD_DR_AR_QP_U5, 
		 TOP_GP32_LDH_GT_MD_DR_AR_QP_U5, 
		 TOP_GP32_LDP_GT_MD_DR_AR_QP_U5, 
		 TOP_GP32_LDSETUB_GT_MD_DR_AR_QP_U5, 
		 TOP_GP32_LDUBP_GT_MD_DR_AR_QP_U5, 
		 TOP_GP32_LDUB_GT_MD_DR_AR_QP_U5, 
		 TOP_GP32_LDUH_GT_MD_DR_AR_QP_U5, 
		 TOP_GP32_LDUW_GT_MD_DR_AR_QP_U5, 
		 TOP_GP32_LDW_GT_MD_DR_AR_QP_U5, 
		 TOP_UNDEFINED); 



  ISA_Print_End(); 
  return 0; 
} 
