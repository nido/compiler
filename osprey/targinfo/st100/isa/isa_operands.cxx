//  AUTOMATICALLY GENERATED FROM ARC DATABASE !!! 
// Group TOPS with similar operands/results format. 
///////////////////////////////////////////////////////// 
// Within each category, the instructions are arranged roughly in order 
// of increasing numbers of operands. 
///////////////////////////////////// 


#include <stddef.h> 
#include "topcode.h" 
#include "isa_operands_gen.h" 

main() 
{ 

  OPERAND_VALUE_TYPE int40, ptr32; 
  OPERAND_VALUE_TYPE lr, lr0, lr1, lr2; 
  OPERAND_VALUE_TYPE p3, p11, p13, p15; 
  OPERAND_VALUE_TYPE pr, g0; 
  OPERAND_VALUE_TYPE ctrl, ctrll, ctrlh, cr8, cr9, cr29; 
  OPERAND_VALUE_TYPE int40l, int40h, r3; 
  OPERAND_VALUE_TYPE ptr32l, ptr32h; 
  OPERAND_VALUE_TYPE md; 
  OPERAND_VALUE_TYPE u4, u5, u6, u7, u8, u9, u10, u11, u12, u15, u16, u20, u32; 
  OPERAND_VALUE_TYPE s7, s9, s11, s16, s21, s25, s32, s40; 
  OPERAND_VALUE_TYPE absadr, pcrel; 

  /* ------------------------------------------------------
   *   Following types must be specified: 
   *     1. base operand use for TOP_load, TOP_store; 
   *     2. offset operand use for TOP_load, TOP_store; 
   *     3. storeval operand use for TOP_store; 
   * 
   *   Following built-in use types may be specified: 
   * 
   *   Here you can specify any additional operand uses. 
   * ------------------------------------------------------
   */
  OPERAND_USE_TYPE 
	  base,		// a base address (memory insts) 
	  offset,	// an offset added to a base (imm) 
	  storeval,	// value to be stored 
	  target,	// the target of a branch 
	  postincr,	// a post increment applied to a base address 
	  predicate,	// is OP predicate 
	  uniq_res,	//  
	  implicit,	//  
	  opnd1,	// first/left operand of an alu operator 
        opnd2;	// second/right operand of an alu operator 

  ISA_Operands_Begin("st100"); 
  /* Create the register operand types: */ 
  pr = ISA_Reg_Opnd_Type_Create("pr", 
                ISA_REGISTER_CLASS_guard, 
		  ISA_REGISTER_SUBCLASS_UNDEFINED, 
		  1, UNSIGNED, INVALID); 
  lr = ISA_Reg_Opnd_Type_Create("lr", 
                ISA_REGISTER_CLASS_loop, 
		  ISA_REGISTER_SUBCLASS_UNDEFINED, 
		  1, UNSIGNED, INVALID); 
  lr0 = ISA_Reg_Opnd_Type_Create("lr0", 
                ISA_REGISTER_CLASS_loop, 
		  ISA_REGISTER_SUBCLASS_lr0, 
		  1, UNSIGNED, INVALID); 
  lr1 = ISA_Reg_Opnd_Type_Create("lr1", 
                ISA_REGISTER_CLASS_loop, 
		  ISA_REGISTER_SUBCLASS_lr1, 
		  1, UNSIGNED, INVALID); 
  lr2 = ISA_Reg_Opnd_Type_Create("lr2", 
                ISA_REGISTER_CLASS_loop, 
		  ISA_REGISTER_SUBCLASS_lr2, 
		  1, UNSIGNED, INVALID); 
  g0 = ISA_Reg_Opnd_Type_Create("g0", 
                ISA_REGISTER_CLASS_guard, 
		  ISA_REGISTER_SUBCLASS_g0, 
		  1, UNSIGNED, INVALID); 
  int40 = ISA_Reg_Opnd_Type_Create("int40", 
                ISA_REGISTER_CLASS_du, 
		  ISA_REGISTER_SUBCLASS_UNDEFINED, 
		  40, SIGNED, INVALID); 
  r3 = ISA_Reg_Opnd_Type_Create("r3", 
                ISA_REGISTER_CLASS_du, 
		  ISA_REGISTER_SUBCLASS_r3, 
		  40, SIGNED, INVALID); 
  ptr32 = ISA_Reg_Opnd_Type_Create("ptr32", 
                ISA_REGISTER_CLASS_au, 
		  ISA_REGISTER_SUBCLASS_UNDEFINED, 
		  32, SIGNED, INVALID); 
  p3 = ISA_Reg_Opnd_Type_Create("p3", 
                ISA_REGISTER_CLASS_au, 
		  ISA_REGISTER_SUBCLASS_p3, 
		  32, UNSIGNED, INVALID); 
  p11 = ISA_Reg_Opnd_Type_Create("p11", 
                ISA_REGISTER_CLASS_au, 
		  ISA_REGISTER_SUBCLASS_p11, 
		  32, UNSIGNED, INVALID); 
  p13 = ISA_Reg_Opnd_Type_Create("p13", 
                ISA_REGISTER_CLASS_au, 
		  ISA_REGISTER_SUBCLASS_p13, 
		  32, UNSIGNED, INVALID); 
  p15 = ISA_Reg_Opnd_Type_Create("p15", 
                ISA_REGISTER_CLASS_au, 
		  ISA_REGISTER_SUBCLASS_p15, 
		  32, UNSIGNED, INVALID); 
  cr8 = ISA_Reg_Opnd_Type_Create("cr8", 
                ISA_REGISTER_CLASS_control, 
		  ISA_REGISTER_SUBCLASS_cr8, 
		  32, UNSIGNED, INVALID); 
  cr9 = ISA_Reg_Opnd_Type_Create("cr9", 
                ISA_REGISTER_CLASS_control, 
		  ISA_REGISTER_SUBCLASS_cr9, 
		  32, UNSIGNED, INVALID); 
  cr29 = ISA_Reg_Opnd_Type_Create("cr29", 
                ISA_REGISTER_CLASS_control, 
		  ISA_REGISTER_SUBCLASS_cr29, 
		  32, UNSIGNED, INVALID); 
  ctrl = ISA_Reg_Opnd_Type_Create("ctrl", 
                ISA_REGISTER_CLASS_control, 
		  ISA_REGISTER_SUBCLASS_UNDEFINED, 
		  32, UNSIGNED, INVALID); 
  int40l = ISA_Reg_Opnd_Type_Create("int40_l", 
                ISA_REGISTER_CLASS_du, 
		  ISA_REGISTER_SUBCLASS_drl, 
		  40, SIGNED, INVALID); 
  int40h = ISA_Reg_Opnd_Type_Create("int40_h", 
                ISA_REGISTER_CLASS_du, 
		  ISA_REGISTER_SUBCLASS_drh, 
		  40, SIGNED, INVALID); 
  ptr32l = ISA_Reg_Opnd_Type_Create("ptr32_l", 
                ISA_REGISTER_CLASS_au, 
		  ISA_REGISTER_SUBCLASS_arl, 
		  32, SIGNED, INVALID); 
  ptr32h = ISA_Reg_Opnd_Type_Create("ptr32_h", 
                ISA_REGISTER_CLASS_au, 
		  ISA_REGISTER_SUBCLASS_arh, 
		  32, SIGNED, INVALID); 
  ctrll = ISA_Reg_Opnd_Type_Create("ctrl_l", 
                ISA_REGISTER_CLASS_control, 
		  ISA_REGISTER_SUBCLASS_crl, 
		  32, UNSIGNED, INVALID); 
  ctrlh = ISA_Reg_Opnd_Type_Create("ctrl_h", 
                ISA_REGISTER_CLASS_control, 
		  ISA_REGISTER_SUBCLASS_crh, 
		  32, UNSIGNED, INVALID); 

  /* Create the enum operand types: */ 

  md = ISA_Enum_Opnd_Type_Create("md", 8, UNSIGNED, EC_amod); 

  /* Create the literal operand types: */ 

  u4   = ISA_Lit_Opnd_Type_Create("u4",   4, UNSIGNED, LC_u4); 
  u5   = ISA_Lit_Opnd_Type_Create("u5",   5, UNSIGNED, LC_u5); 
  u6   = ISA_Lit_Opnd_Type_Create("u6",   6, UNSIGNED, LC_u6); 
  u7   = ISA_Lit_Opnd_Type_Create("u7",   7, UNSIGNED, LC_u7); 
  u8   = ISA_Lit_Opnd_Type_Create("u8",   8, UNSIGNED, LC_u8); 
  u9   = ISA_Lit_Opnd_Type_Create("u9",   9, UNSIGNED, LC_u9); 
  u10   = ISA_Lit_Opnd_Type_Create("u10",   10, UNSIGNED, LC_u10); 
  u11   = ISA_Lit_Opnd_Type_Create("u11",   11, UNSIGNED, LC_u11); 
  u12   = ISA_Lit_Opnd_Type_Create("u12",   12, UNSIGNED, LC_u12); 
  u15   = ISA_Lit_Opnd_Type_Create("u15",   15, UNSIGNED, LC_u15); 
  u16   = ISA_Lit_Opnd_Type_Create("u16",   16, UNSIGNED, LC_u16); 
  u20   = ISA_Lit_Opnd_Type_Create("u20",   20, UNSIGNED, LC_u20); 
  u32   = ISA_Lit_Opnd_Type_Create("u32",   32, UNSIGNED, LC_u32); 
  s7   = ISA_Lit_Opnd_Type_Create("s7",   7, SIGNED, LC_s7); 
  s9   = ISA_Lit_Opnd_Type_Create("s9",   9, SIGNED, LC_s9); 
  s11   = ISA_Lit_Opnd_Type_Create("s11",   11, SIGNED, LC_s11); 
  s16   = ISA_Lit_Opnd_Type_Create("s16",   16, SIGNED, LC_s16); 
  s21   = ISA_Lit_Opnd_Type_Create("s21",   21, SIGNED, LC_s21); 
  s25   = ISA_Lit_Opnd_Type_Create("s25",   25, SIGNED, LC_s25); 
  s32   = ISA_Lit_Opnd_Type_Create("s32",   32, SIGNED, LC_s32); 
  s40   = ISA_Lit_Opnd_Type_Create("s40",   40, SIGNED, LC_s40); 
  pcrel   = ISA_Lit_Opnd_Type_Create("pcrel",   32, PCREL, LC_s32); 
  absadr   = ISA_Lit_Opnd_Type_Create("absadr",   32, UNSIGNED, LC_u32); 

  /* Create the operand uses: */ 

  base       = Create_Operand_Use("base"); 
  offset     = Create_Operand_Use("offset"); 
  storeval   = Create_Operand_Use("storeval"); 
  target     = Create_Operand_Use("target"); 
  postincr   = Create_Operand_Use("postincr"); 
  predicate  = Create_Operand_Use("predicate"); 
  uniq_res   = Create_Operand_Use("uniq_res"); 
  implicit   = Create_Operand_Use("implicit"); 
  opnd1      = Create_Operand_Use("opnd1"); 
  opnd2      = Create_Operand_Use("opnd2"); 

  /* ====================================== */ 
  Instruction_Group("O_0", 
		 TOP_GP32_LCG_GT_BR_AR_BM_U5, 
		 TOP_GP32_LCG_GT_BR_AR_BP_U5, 
		 TOP_GP32_LCG_GT_BR_AR_MQ_U5, 
		 TOP_GP32_LCG_GT_BR_AR_QM_U5, 
		 TOP_GP32_LCG_GT_BR_AR_QP_U5, 
		 TOP_UNDEFINED); 

  Result (0, pr); 
  Result (1, ptr32); 
  Same_Res (2); 
  Operand (0, pr, predicate); 
  Operand (1, ptr32, base); 
  Operand (2, u5, offset); 

  /* ====================================== */ 
  Instruction_Group("O_1", 
		 TOP_GP32_LCG_GT_BR_AR_BM_AR, 
		 TOP_GP32_LCG_GT_BR_AR_BP_AR, 
		 TOP_GP32_LCG_GT_BR_AR_QM_AR, 
		 TOP_GP32_LCG_GT_BR_AR_QP_AR, 
		 TOP_UNDEFINED); 

  Result (0, pr); 
  Result (1, ptr32); 
  Same_Res (2); 
  Operand (0, pr, predicate); 
  Operand (1, ptr32, base); 
  Operand (2, ptr32, offset); 

  /* ====================================== */ 
  Instruction_Group("O_2", 
		 TOP_GP32_SCW_GT_MD_AR_BM_AR_CRL, 
		 TOP_GP32_SCW_GT_MD_AR_BP_AR_CRL, 
		 TOP_GP32_SCW_GT_MD_AR_QM_AR_CRL, 
		 TOP_GP32_SCW_GT_MD_AR_QP_AR_CRL, 
		 TOP_UNDEFINED); 

  Result (0, ptr32); 
  Same_Res (2); 
  Operand (0, pr, predicate); 
  Operand (1, md); 
  Operand (2, ptr32, base); 
  Operand (3, ptr32, offset); 
  Operand (4, ctrll, storeval); 

  /* ====================================== */ 
  Instruction_Group("O_3", 
		 TOP_GP32_ADDCP_GT_DR_DR_DR, 
		 TOP_GP32_ADDCW_GT_DR_DR_DR, 
		 TOP_GP32_ADDP_GT_DR_DR_DR, 
		 TOP_GP32_ADDUP_GT_DR_DR_DR, 
		 TOP_GP32_ADDU_GT_DR_DR_DR, 
		 TOP_GP32_ADD_GT_DR_DR_DR, 
		 TOP_GP32_ANDNP_GT_DR_DR_DR, 
		 TOP_GP32_ANDN_GT_DR_DR_DR, 
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
		 TOP_GP32_NANDP_GT_DR_DR_DR, 
		 TOP_GP32_NAND_GT_DR_DR_DR, 
		 TOP_GP32_NORP_GT_DR_DR_DR, 
		 TOP_GP32_NOR_GT_DR_DR_DR, 
		 TOP_GP32_ORNP_GT_DR_DR_DR, 
		 TOP_GP32_ORN_GT_DR_DR_DR, 
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
		 TOP_GP32_SUBCP_GT_DR_DR_DR, 
		 TOP_GP32_SUBCW_GT_DR_DR_DR, 
		 TOP_GP32_SUBP_GT_DR_DR_DR, 
		 TOP_GP32_SUBUP_GT_DR_DR_DR, 
		 TOP_GP32_SUBU_GT_DR_DR_DR, 
		 TOP_GP32_SUB_GT_DR_DR_DR, 
		 TOP_GP32_XNORP_GT_DR_DR_DR, 
		 TOP_GP32_XNOR_GT_DR_DR_DR, 
		 TOP_GP32_XORP_GT_DR_DR_DR, 
		 TOP_GP32_XOR_GT_DR_DR_DR, 
		 TOP_UNDEFINED); 

  Result (0, int40); 
  Operand (0, pr, predicate); 
  Operand (1, int40, opnd1); 
  Operand (2, int40, opnd2); 

  /* ====================================== */ 
  Instruction_Group("O_4", 
		 TOP_GP32_MAKEC_GT_CRL_U16, 
		 TOP_UNDEFINED); 

  Result (0, ctrll); 
  Operand (0, pr, predicate); 
  Operand (1, u16, opnd1); 

  /* ====================================== */ 
  Instruction_Group("O_5", 
		 TOP_GP32_COPYA_GT_AR_DR, 
		 TOP_GP32_COPYSA_GT_AR_DR, 
		 TOP_UNDEFINED); 

  Result (0, ptr32); 
  Operand (0, pr, predicate); 
  Operand (1, int40, opnd1); 

  /* ====================================== */ 
  Instruction_Group("O_6", 
		 TOP_GP32_CLRSCL_GT, 
		 TOP_GP32_CLRSNR_GT, 
		 TOP_GP32_CLRSVE_GT, 
		 TOP_GP32_CLRSVH_GT, 
		 TOP_GP32_CLRSVL_GT, 
		 TOP_GP32_CLRSVP_GT, 
		 TOP_GP32_CLRSVW_GT, 
		 TOP_UNDEFINED); 

  Result (0, cr8); 
  Operand (0, pr, predicate); 
  Operand (1, cr8, opnd1); 

  /* ====================================== */ 
  Instruction_Group("O_intrncall", 
		 TOP_intrncall, 
		 TOP_UNDEFINED); 


  /* ====================================== */ 
  Instruction_Group("O_7", 
		 TOP_GP32_SWNMI, 
		 TOP_UNDEFINED); 

  Operand (0, cr29); 
  Operand (1, p15, opnd1); 

  /* ====================================== */ 
  Instruction_Group("O_copy_br", 
		 TOP_copy_br, 
		 TOP_UNDEFINED); 

  Result (0, ptr32); 
  Operand (0, pr, predicate); 
  Operand (1, ptr32); 

  /* ====================================== */ 
  Instruction_Group("O_8", 
		 TOP_GP32_SCW_GT_MD_AR_M_AR_CRL, 
		 TOP_GP32_SCW_GT_MD_AR_P_AR_CRL, 
		 TOP_UNDEFINED); 

  Operand (0, pr, predicate); 
  Operand (1, md); 
  Operand (2, ptr32, base); 
  Operand (3, ptr32, offset); 
  Operand (4, ctrll, storeval); 

  /* ====================================== */ 
  Instruction_Group("O_9", 
		 TOP_GP32_SCW_GT_MD_AR_BM_U5_CRL, 
		 TOP_GP32_SCW_GT_MD_AR_BP_U5_CRL, 
		 TOP_GP32_SCW_GT_MD_AR_QM_U5_CRL, 
		 TOP_GP32_SCW_GT_MD_AR_QP_U5_CRL, 
		 TOP_UNDEFINED); 

  Result (0, ptr32); 
  Same_Res (2); 
  Operand (0, pr, predicate); 
  Operand (1, md); 
  Operand (2, ptr32, base); 
  Operand (3, u5, offset); 
  Operand (4, ctrll, storeval); 

  /* ====================================== */ 
  Instruction_Group("O_10", 
		 TOP_GP32_GETP15U_GT_AR, 
		 TOP_UNDEFINED); 

  Result (0, ptr32); 
  Operand (0, pr, predicate); 
  Operand (1, p15, opnd1); 

  /* ====================================== */ 
  Instruction_Group("O_11", 
		 TOP_GP32_SETILE2_S16, 
		 TOP_GP32_SETLE2_S16, 
		 TOP_GP32_SETLS2_S16, 
		 TOP_GP32_SETULS2_S16, 
		 TOP_UNDEFINED); 

  Result (0, lr2); 
  Operand (0, s16, target); 

  /* ====================================== */ 
  Instruction_Group("O_12", 
		 TOP_GP32_LCW_GT_CRH_AR_M_AR, 
		 TOP_GP32_LCW_GT_CRH_AR_P_AR, 
		 TOP_UNDEFINED); 

  Result (0, ctrlh); 
  Same_Res (1); 
  Operand (0, pr, predicate); 
  Operand (1, ctrlh); 
  Operand (2, ptr32, base); 
  Operand (3, ptr32, offset); 

  /* ====================================== */ 
  Instruction_Group("O_13", 
		 TOP_GP32_LFR_GT_P13_P_U15, 
		 TOP_UNDEFINED); 

  Result (0, cr8); 
  Operand (0, pr, predicate); 
  Operand (1, p13, base); 
  Operand (2, u15, offset); 

  /* ====================================== */ 
  Instruction_Group("O_14", 
		 TOP_GP32_LDEW_GT_MD_DR_AR_BM_AR, 
		 TOP_GP32_LDEW_GT_MD_DR_AR_BP_AR, 
		 TOP_GP32_LDEW_GT_MD_DR_AR_QM_AR, 
		 TOP_GP32_LDEW_GT_MD_DR_AR_QP_AR, 
		 TOP_GP32_LDHH_GT_MD_DR_AR_BM_AR, 
		 TOP_GP32_LDHH_GT_MD_DR_AR_BP_AR, 
		 TOP_GP32_LDHH_GT_MD_DR_AR_QM_AR, 
		 TOP_GP32_LDHH_GT_MD_DR_AR_QP_AR, 
		 TOP_GP32_LDLH_GT_MD_DR_AR_BM_AR, 
		 TOP_GP32_LDLH_GT_MD_DR_AR_BP_AR, 
		 TOP_GP32_LDLH_GT_MD_DR_AR_QM_AR, 
		 TOP_GP32_LDLH_GT_MD_DR_AR_QP_AR, 
		 TOP_UNDEFINED); 

  Result (0, int40); 
  Same_Res (2); 
  Result (1, ptr32); 
  Same_Res (3); 
  Operand (0, pr, predicate); 
  Operand (1, md); 
  Operand (2, int40); 
  Operand (3, ptr32, base); 
  Operand (4, ptr32, offset); 

  /* ====================================== */ 
  Instruction_Group("O_15", 
		 TOP_GP32_MORE_GT_DR_U16, 
		 TOP_UNDEFINED); 

  Result (0, int40); 
  Same_Res (1); 
  Operand (0, pr, predicate); 
  Operand (1, int40, opnd1); 
  Operand (2, u16, opnd2); 

  /* ====================================== */ 
  Instruction_Group("O_16", 
		 TOP_GP32_XSHLW_GT_DR_DR_DR_U5, 
		 TOP_GP32_XSHRW_GT_DR_DR_DR_U5, 
		 TOP_UNDEFINED); 

  Result (0, int40); 
  Operand (0, pr, predicate); 
  Operand (1, int40, opnd1); 
  Operand (2, int40, opnd2); 
  Operand (3, u5); 

  /* ====================================== */ 
  Instruction_Group("O_spadjust", 
		 TOP_spadjust, 
		 TOP_UNDEFINED); 

  Result (0, ptr32); 
  Operand (0, pr, predicate); 
  Operand (1, ptr32); 
  Operand (2, s32); 

  /* ====================================== */ 
  Instruction_Group("O_17", 
		 TOP_GP32_LDEW_GT_MD_DR_AR_BM_U5, 
		 TOP_GP32_LDEW_GT_MD_DR_AR_BP_U5, 
		 TOP_GP32_LDEW_GT_MD_DR_AR_QM_U5, 
		 TOP_GP32_LDEW_GT_MD_DR_AR_QP_U5, 
		 TOP_GP32_LDHH_GT_MD_DR_AR_BM_U5, 
		 TOP_GP32_LDHH_GT_MD_DR_AR_BP_U5, 
		 TOP_GP32_LDHH_GT_MD_DR_AR_QM_U5, 
		 TOP_GP32_LDHH_GT_MD_DR_AR_QP_U5, 
		 TOP_GP32_LDLH_GT_MD_DR_AR_BM_U5, 
		 TOP_GP32_LDLH_GT_MD_DR_AR_BP_U5, 
		 TOP_GP32_LDLH_GT_MD_DR_AR_QM_U5, 
		 TOP_GP32_LDLH_GT_MD_DR_AR_QP_U5, 
		 TOP_UNDEFINED); 

  Result (0, int40); 
  Same_Res (2); 
  Result (1, ptr32); 
  Same_Res (3); 
  Operand (0, pr, predicate); 
  Operand (1, md); 
  Operand (2, int40); 
  Operand (3, ptr32, base); 
  Operand (4, u5, offset); 

  /* ====================================== */ 
  Instruction_Group("O_18", 
		 TOP_GP32_ADDBA_GT_AR_AR_U9, 
		 TOP_GP32_SUBBA_GT_AR_AR_U9, 
		 TOP_GP32_SUBHA_GT_AR_AR_U9, 
		 TOP_GP32_SUBWA_GT_AR_AR_U9, 
		 TOP_UNDEFINED); 

  Result (0, ptr32); 
  Operand (0, pr, predicate); 
  Operand (1, ptr32, opnd1); 
  Operand (2, u9, opnd2); 

  /* ====================================== */ 
  Instruction_Group("O_asm", 
		 TOP_asm, 
		 TOP_phi, 
		 TOP_psi, 
		 TOP_UNDEFINED); 


  /* ====================================== */ 
  Instruction_Group("O_19", 
		 TOP_GP32_SDBP_GT_MD_AR_BM_AR_DR, 
		 TOP_GP32_SDBP_GT_MD_AR_BP_AR_DR, 
		 TOP_GP32_SDBP_GT_MD_AR_QM_AR_DR, 
		 TOP_GP32_SDBP_GT_MD_AR_QP_AR_DR, 
		 TOP_GP32_SDBSW_GT_MD_AR_BM_AR_DR, 
		 TOP_GP32_SDBSW_GT_MD_AR_BP_AR_DR, 
		 TOP_GP32_SDBSW_GT_MD_AR_QM_AR_DR, 
		 TOP_GP32_SDBSW_GT_MD_AR_QP_AR_DR, 
		 TOP_GP32_SDB_GT_MD_AR_BM_AR_DR, 
		 TOP_GP32_SDB_GT_MD_AR_BP_AR_DR, 
		 TOP_GP32_SDB_GT_MD_AR_QM_AR_DR, 
		 TOP_GP32_SDB_GT_MD_AR_QP_AR_DR, 
		 TOP_GP32_SDEW_GT_MD_AR_BM_AR_DR, 
		 TOP_GP32_SDEW_GT_MD_AR_BP_AR_DR, 
		 TOP_GP32_SDEW_GT_MD_AR_QM_AR_DR, 
		 TOP_GP32_SDEW_GT_MD_AR_QP_AR_DR, 
		 TOP_GP32_SDF_GT_MD_AR_BM_AR_DR, 
		 TOP_GP32_SDF_GT_MD_AR_BP_AR_DR, 
		 TOP_GP32_SDF_GT_MD_AR_QM_AR_DR, 
		 TOP_GP32_SDF_GT_MD_AR_QP_AR_DR, 
		 TOP_GP32_SDHSW_GT_MD_AR_BM_AR_DR, 
		 TOP_GP32_SDHSW_GT_MD_AR_BP_AR_DR, 
		 TOP_GP32_SDHSW_GT_MD_AR_QM_AR_DR, 
		 TOP_GP32_SDHSW_GT_MD_AR_QP_AR_DR, 
		 TOP_GP32_SDH_GT_MD_AR_BM_AR_DR, 
		 TOP_GP32_SDH_GT_MD_AR_BP_AR_DR, 
		 TOP_GP32_SDH_GT_MD_AR_QM_AR_DR, 
		 TOP_GP32_SDH_GT_MD_AR_QP_AR_DR, 
		 TOP_GP32_SDP_GT_MD_AR_BM_AR_DR, 
		 TOP_GP32_SDP_GT_MD_AR_BP_AR_DR, 
		 TOP_GP32_SDP_GT_MD_AR_QM_AR_DR, 
		 TOP_GP32_SDP_GT_MD_AR_QP_AR_DR, 
		 TOP_GP32_SDW_GT_MD_AR_BM_AR_DR, 
		 TOP_GP32_SDW_GT_MD_AR_BP_AR_DR, 
		 TOP_GP32_SDW_GT_MD_AR_QM_AR_DR, 
		 TOP_GP32_SDW_GT_MD_AR_QP_AR_DR, 
		 TOP_UNDEFINED); 

  Result (0, ptr32); 
  Same_Res (2); 
  Operand (0, pr, predicate); 
  Operand (1, md); 
  Operand (2, ptr32, base); 
  Operand (3, ptr32, offset); 
  Operand (4, int40, storeval); 

  /* ====================================== */ 
  Instruction_Group("O_20", 
		 TOP_GP32_LDEW_GT_DR_AR_BM_U5, 
		 TOP_GP32_LDEW_GT_DR_AR_BP_U5, 
		 TOP_GP32_LDEW_GT_DR_AR_MQ_U5, 
		 TOP_GP32_LDEW_GT_DR_AR_QM_U5, 
		 TOP_GP32_LDEW_GT_DR_AR_QP_U5, 
		 TOP_GP32_LDHH_GT_DR_AR_BM_U5, 
		 TOP_GP32_LDHH_GT_DR_AR_BP_U5, 
		 TOP_GP32_LDHH_GT_DR_AR_MQ_U5, 
		 TOP_GP32_LDHH_GT_DR_AR_QM_U5, 
		 TOP_GP32_LDHH_GT_DR_AR_QP_U5, 
		 TOP_GP32_LDLH_GT_DR_AR_BM_U5, 
		 TOP_GP32_LDLH_GT_DR_AR_BP_U5, 
		 TOP_GP32_LDLH_GT_DR_AR_MQ_U5, 
		 TOP_GP32_LDLH_GT_DR_AR_QM_U5, 
		 TOP_GP32_LDLH_GT_DR_AR_QP_U5, 
		 TOP_UNDEFINED); 

  Result (0, int40); 
  Same_Res (1); 
  Result (1, ptr32); 
  Same_Res (2); 
  Operand (0, pr, predicate); 
  Operand (1, int40); 
  Operand (2, ptr32, base); 
  Operand (3, u5, offset); 

  /* ====================================== */ 
  Instruction_Group("O_21", 
		 TOP_GP32_LCW_GT_CRH_AR_BM_AR, 
		 TOP_GP32_LCW_GT_CRH_AR_BP_AR, 
		 TOP_GP32_LCW_GT_CRH_AR_QM_AR, 
		 TOP_GP32_LCW_GT_CRH_AR_QP_AR, 
		 TOP_UNDEFINED); 

  Result (0, ctrlh); 
  Same_Res (1); 
  Result (1, ptr32); 
  Same_Res (2); 
  Operand (0, pr, predicate); 
  Operand (1, ctrlh); 
  Operand (2, ptr32, base); 
  Operand (3, ptr32, offset); 

  /* ====================================== */ 
  Instruction_Group("O_22", 
		 TOP_GP32_LCG_GT_MD_BR_AR_BM_AR, 
		 TOP_GP32_LCG_GT_MD_BR_AR_BP_AR, 
		 TOP_GP32_LCG_GT_MD_BR_AR_QM_AR, 
		 TOP_GP32_LCG_GT_MD_BR_AR_QP_AR, 
		 TOP_UNDEFINED); 

  Result (0, pr); 
  Result (1, ptr32); 
  Same_Res (3); 
  Operand (0, pr, predicate); 
  Operand (1, md); 
  Operand (2, ptr32, base); 
  Operand (3, ptr32, offset); 

  /* ====================================== */ 
  Instruction_Group("O_23", 
		 TOP_GP32_LDBP_GT_DR_AR_M_AR, 
		 TOP_GP32_LDBP_GT_DR_AR_P_AR, 
		 TOP_GP32_LDBSW_GT_DR_AR_M_AR, 
		 TOP_GP32_LDBSW_GT_DR_AR_P_AR, 
		 TOP_GP32_LDB_GT_DR_AR_M_AR, 
		 TOP_GP32_LDB_GT_DR_AR_P_AR, 
		 TOP_GP32_LDF_GT_DR_AR_M_AR, 
		 TOP_GP32_LDF_GT_DR_AR_P_AR, 
		 TOP_GP32_LDHSW_GT_DR_AR_M_AR, 
		 TOP_GP32_LDHSW_GT_DR_AR_P_AR, 
		 TOP_GP32_LDH_GT_DR_AR_M_AR, 
		 TOP_GP32_LDH_GT_DR_AR_P_AR, 
		 TOP_GP32_LDP_GT_DR_AR_M_AR, 
		 TOP_GP32_LDP_GT_DR_AR_P_AR, 
		 TOP_GP32_LDSETUB_GT_DR_AR_M_AR, 
		 TOP_GP32_LDSETUB_GT_DR_AR_P_AR, 
		 TOP_GP32_LDUBP_GT_DR_AR_M_AR, 
		 TOP_GP32_LDUBP_GT_DR_AR_P_AR, 
		 TOP_GP32_LDUB_GT_DR_AR_M_AR, 
		 TOP_GP32_LDUB_GT_DR_AR_P_AR, 
		 TOP_GP32_LDUH_GT_DR_AR_M_AR, 
		 TOP_GP32_LDUH_GT_DR_AR_P_AR, 
		 TOP_GP32_LDUW_GT_DR_AR_M_AR, 
		 TOP_GP32_LDUW_GT_DR_AR_P_AR, 
		 TOP_GP32_LDW_GT_DR_AR_M_AR, 
		 TOP_GP32_LDW_GT_DR_AR_P_AR, 
		 TOP_UNDEFINED); 

  Result (0, int40); 
  Operand (0, pr, predicate); 
  Operand (1, ptr32, base); 
  Operand (2, ptr32, offset); 

  /* ====================================== */ 
  Instruction_Group("O_24", 
		 TOP_GP32_LDH_GT_DR_AR_BM_U5, 
		 TOP_GP32_LDH_GT_DR_AR_BP_U5, 
		 TOP_GP32_LDH_GT_DR_AR_MQ_U5, 
		 TOP_GP32_LDH_GT_DR_AR_QP_U5, 
		 TOP_UNDEFINED); 

  Result (0, int40); 
  Result (1, ptr32); 
  Same_Res (2); 
  Operand (0, pr, predicate); 
  Operand (1, ptr32, base); 
  Operand (2, u6, offset); 

  /* ====================================== */ 
  Instruction_Group("O_25", 
		 TOP_GP32_FA_GT_BR_AR, 
		 TOP_GP32_TA_GT_BR_AR, 
		 TOP_UNDEFINED); 

  Result (0, pr); 
  Operand (0, pr, predicate); 
  Operand (1, ptr32, opnd1); 

  /* ====================================== */ 
  Instruction_Group("O_26", 
		 TOP_GP32_ADDBA_GT_MD_AR_AR_U5, 
		 TOP_GP32_ADDHA_GT_MD_AR_AR_U5, 
		 TOP_GP32_ADDWA_GT_MD_AR_AR_U5, 
		 TOP_GP32_SUBBA_GT_MD_AR_AR_U5, 
		 TOP_GP32_SUBHA_GT_MD_AR_AR_U5, 
		 TOP_GP32_SUBWA_GT_MD_AR_AR_U5, 
		 TOP_UNDEFINED); 

  Result (0, ptr32); 
  Operand (0, pr, predicate); 
  Operand (1, md, opnd1); 
  Operand (2, ptr32, opnd2); 
  Operand (3, u5); 

  /* ====================================== */ 
  Instruction_Group("O_27", 
		 TOP_GP32_LDBP_GT_MD_DR_AR_BM_AR, 
		 TOP_GP32_LDBP_GT_MD_DR_AR_BP_AR, 
		 TOP_GP32_LDBP_GT_MD_DR_AR_QM_AR, 
		 TOP_GP32_LDBP_GT_MD_DR_AR_QP_AR, 
		 TOP_GP32_LDBSW_GT_MD_DR_AR_BM_AR, 
		 TOP_GP32_LDBSW_GT_MD_DR_AR_BP_AR, 
		 TOP_GP32_LDBSW_GT_MD_DR_AR_QM_AR, 
		 TOP_GP32_LDBSW_GT_MD_DR_AR_QP_AR, 
		 TOP_GP32_LDB_GT_MD_DR_AR_BM_AR, 
		 TOP_GP32_LDB_GT_MD_DR_AR_BP_AR, 
		 TOP_GP32_LDB_GT_MD_DR_AR_QM_AR, 
		 TOP_GP32_LDB_GT_MD_DR_AR_QP_AR, 
		 TOP_GP32_LDF_GT_MD_DR_AR_BM_AR, 
		 TOP_GP32_LDF_GT_MD_DR_AR_BP_AR, 
		 TOP_GP32_LDF_GT_MD_DR_AR_QM_AR, 
		 TOP_GP32_LDF_GT_MD_DR_AR_QP_AR, 
		 TOP_GP32_LDHSW_GT_MD_DR_AR_BM_AR, 
		 TOP_GP32_LDHSW_GT_MD_DR_AR_BP_AR, 
		 TOP_GP32_LDHSW_GT_MD_DR_AR_QM_AR, 
		 TOP_GP32_LDHSW_GT_MD_DR_AR_QP_AR, 
		 TOP_GP32_LDH_GT_MD_DR_AR_BM_AR, 
		 TOP_GP32_LDH_GT_MD_DR_AR_BP_AR, 
		 TOP_GP32_LDH_GT_MD_DR_AR_QM_AR, 
		 TOP_GP32_LDH_GT_MD_DR_AR_QP_AR, 
		 TOP_GP32_LDP_GT_MD_DR_AR_BM_AR, 
		 TOP_GP32_LDP_GT_MD_DR_AR_BP_AR, 
		 TOP_GP32_LDP_GT_MD_DR_AR_QM_AR, 
		 TOP_GP32_LDP_GT_MD_DR_AR_QP_AR, 
		 TOP_GP32_LDSETUB_GT_MD_DR_AR_BM_AR, 
		 TOP_GP32_LDSETUB_GT_MD_DR_AR_BP_AR, 
		 TOP_GP32_LDSETUB_GT_MD_DR_AR_QM_AR, 
		 TOP_GP32_LDSETUB_GT_MD_DR_AR_QP_AR, 
		 TOP_GP32_LDUBP_GT_MD_DR_AR_BM_AR, 
		 TOP_GP32_LDUBP_GT_MD_DR_AR_BP_AR, 
		 TOP_GP32_LDUBP_GT_MD_DR_AR_QM_AR, 
		 TOP_GP32_LDUBP_GT_MD_DR_AR_QP_AR, 
		 TOP_GP32_LDUB_GT_MD_DR_AR_BM_AR, 
		 TOP_GP32_LDUB_GT_MD_DR_AR_BP_AR, 
		 TOP_GP32_LDUB_GT_MD_DR_AR_QM_AR, 
		 TOP_GP32_LDUB_GT_MD_DR_AR_QP_AR, 
		 TOP_GP32_LDUH_GT_MD_DR_AR_BM_AR, 
		 TOP_GP32_LDUH_GT_MD_DR_AR_BP_AR, 
		 TOP_GP32_LDUH_GT_MD_DR_AR_QM_AR, 
		 TOP_GP32_LDUH_GT_MD_DR_AR_QP_AR, 
		 TOP_GP32_LDUW_GT_MD_DR_AR_BM_AR, 
		 TOP_GP32_LDUW_GT_MD_DR_AR_BP_AR, 
		 TOP_GP32_LDUW_GT_MD_DR_AR_QM_AR, 
		 TOP_GP32_LDUW_GT_MD_DR_AR_QP_AR, 
		 TOP_GP32_LDW_GT_MD_DR_AR_BM_AR, 
		 TOP_GP32_LDW_GT_MD_DR_AR_BP_AR, 
		 TOP_GP32_LDW_GT_MD_DR_AR_QM_AR, 
		 TOP_GP32_LDW_GT_MD_DR_AR_QP_AR, 
		 TOP_UNDEFINED); 

  Result (0, int40); 
  Result (1, ptr32); 
  Same_Res (3); 
  Operand (0, pr, predicate); 
  Operand (1, md); 
  Operand (2, ptr32, base); 
  Operand (3, ptr32, offset); 

  /* ====================================== */ 
  Instruction_Group("O_28", 
		 TOP_GP32_LCW_GT_CRH_AR_BM_U5, 
		 TOP_GP32_LCW_GT_CRH_AR_BP_U5, 
		 TOP_GP32_LCW_GT_CRH_AR_MQ_U5, 
		 TOP_GP32_LCW_GT_CRH_AR_QM_U5, 
		 TOP_GP32_LCW_GT_CRH_AR_QP_U5, 
		 TOP_UNDEFINED); 

  Result (0, ctrlh); 
  Same_Res (1); 
  Result (1, ptr32); 
  Same_Res (2); 
  Operand (0, pr, predicate); 
  Operand (1, ctrlh); 
  Operand (2, ptr32, base); 
  Operand (3, u5, offset); 

  /* ====================================== */ 
  Instruction_Group("O_29", 
		 TOP_GP32_BITRA_GT_AR_AR, 
		 TOP_GP32_MOVEA_GT_AR_AR, 
		 TOP_GP32_SHRA1_GT_AR_AR, 
		 TOP_GP32_SHRA2_GT_AR_AR, 
		 TOP_UNDEFINED); 

  Result (0, ptr32); 
  Operand (0, pr, predicate); 
  Operand (1, ptr32, opnd1); 

  /* ====================================== */ 
  Instruction_Group("O_30", 
		 TOP_GP32_SDBP_GT_MD_AR_M_AR_DR, 
		 TOP_GP32_SDBP_GT_MD_AR_P_AR_DR, 
		 TOP_GP32_SDBSW_GT_MD_AR_M_AR_DR, 
		 TOP_GP32_SDBSW_GT_MD_AR_P_AR_DR, 
		 TOP_GP32_SDB_GT_MD_AR_M_AR_DR, 
		 TOP_GP32_SDB_GT_MD_AR_P_AR_DR, 
		 TOP_GP32_SDEW_GT_MD_AR_M_AR_DR, 
		 TOP_GP32_SDEW_GT_MD_AR_P_AR_DR, 
		 TOP_GP32_SDF_GT_MD_AR_M_AR_DR, 
		 TOP_GP32_SDF_GT_MD_AR_P_AR_DR, 
		 TOP_GP32_SDHSW_GT_MD_AR_M_AR_DR, 
		 TOP_GP32_SDHSW_GT_MD_AR_P_AR_DR, 
		 TOP_GP32_SDH_GT_MD_AR_M_AR_DR, 
		 TOP_GP32_SDH_GT_MD_AR_P_AR_DR, 
		 TOP_GP32_SDP_GT_MD_AR_M_AR_DR, 
		 TOP_GP32_SDP_GT_MD_AR_P_AR_DR, 
		 TOP_GP32_SDW_GT_MD_AR_M_AR_DR, 
		 TOP_GP32_SDW_GT_MD_AR_P_AR_DR, 
		 TOP_UNDEFINED); 

  Operand (0, pr, predicate); 
  Operand (1, md); 
  Operand (2, ptr32, base); 
  Operand (3, ptr32, offset); 
  Operand (4, int40, storeval); 

  /* ====================================== */ 
  Instruction_Group("O_31", 
		 TOP_GP32_SDBP_GT_MD_AR_BM_U5_DR, 
		 TOP_GP32_SDBP_GT_MD_AR_BP_U5_DR, 
		 TOP_GP32_SDBP_GT_MD_AR_QM_U5_DR, 
		 TOP_GP32_SDBP_GT_MD_AR_QP_U5_DR, 
		 TOP_GP32_SDBSW_GT_MD_AR_BM_U5_DR, 
		 TOP_GP32_SDBSW_GT_MD_AR_BP_U5_DR, 
		 TOP_GP32_SDBSW_GT_MD_AR_QM_U5_DR, 
		 TOP_GP32_SDBSW_GT_MD_AR_QP_U5_DR, 
		 TOP_GP32_SDB_GT_MD_AR_BM_U5_DR, 
		 TOP_GP32_SDB_GT_MD_AR_BP_U5_DR, 
		 TOP_GP32_SDB_GT_MD_AR_QM_U5_DR, 
		 TOP_GP32_SDB_GT_MD_AR_QP_U5_DR, 
		 TOP_GP32_SDEW_GT_MD_AR_BM_U5_DR, 
		 TOP_GP32_SDEW_GT_MD_AR_BP_U5_DR, 
		 TOP_GP32_SDEW_GT_MD_AR_QM_U5_DR, 
		 TOP_GP32_SDEW_GT_MD_AR_QP_U5_DR, 
		 TOP_GP32_SDF_GT_MD_AR_BM_U5_DR, 
		 TOP_GP32_SDF_GT_MD_AR_BP_U5_DR, 
		 TOP_GP32_SDF_GT_MD_AR_QM_U5_DR, 
		 TOP_GP32_SDF_GT_MD_AR_QP_U5_DR, 
		 TOP_GP32_SDHSW_GT_MD_AR_BM_U5_DR, 
		 TOP_GP32_SDHSW_GT_MD_AR_BP_U5_DR, 
		 TOP_GP32_SDHSW_GT_MD_AR_QM_U5_DR, 
		 TOP_GP32_SDHSW_GT_MD_AR_QP_U5_DR, 
		 TOP_GP32_SDH_GT_MD_AR_BM_U5_DR, 
		 TOP_GP32_SDH_GT_MD_AR_BP_U5_DR, 
		 TOP_GP32_SDH_GT_MD_AR_QM_U5_DR, 
		 TOP_GP32_SDH_GT_MD_AR_QP_U5_DR, 
		 TOP_GP32_SDP_GT_MD_AR_BM_U5_DR, 
		 TOP_GP32_SDP_GT_MD_AR_BP_U5_DR, 
		 TOP_GP32_SDP_GT_MD_AR_QM_U5_DR, 
		 TOP_GP32_SDP_GT_MD_AR_QP_U5_DR, 
		 TOP_GP32_SDW_GT_MD_AR_BM_U5_DR, 
		 TOP_GP32_SDW_GT_MD_AR_BP_U5_DR, 
		 TOP_GP32_SDW_GT_MD_AR_QM_U5_DR, 
		 TOP_GP32_SDW_GT_MD_AR_QP_U5_DR, 
		 TOP_UNDEFINED); 

  Result (0, ptr32); 
  Same_Res (2); 
  Operand (0, pr, predicate); 
  Operand (1, md); 
  Operand (2, ptr32, base); 
  Operand (3, u5, offset); 
  Operand (4, int40, storeval); 

  /* ====================================== */ 
  Instruction_Group("O_32", 
		 TOP_GP32_LCG_GT_BR_AR_M_U9, 
		 TOP_GP32_LCG_GT_BR_AR_P_U9, 
		 TOP_UNDEFINED); 

  Result (0, pr); 
  Operand (0, pr, predicate); 
  Operand (1, ptr32, base); 
  Operand (2, u9, offset); 

  /* ====================================== */ 
  Instruction_Group("O_33", 
		 TOP_GP32_LAH_GT_MD_AR_AR_M_AR, 
		 TOP_GP32_LAH_GT_MD_AR_AR_P_AR, 
		 TOP_GP32_LAW_GT_MD_AR_AR_M_AR, 
		 TOP_GP32_LAW_GT_MD_AR_AR_P_AR, 
		 TOP_UNDEFINED); 

  Result (0, ptr32); 
  Operand (0, pr, predicate); 
  Operand (1, md); 
  Operand (2, ptr32, base); 
  Operand (3, ptr32, offset); 

  /* ====================================== */ 
  Instruction_Group("O_34", 
		 TOP_GP32_LCW_GT_MD_CRH_AR_BM_U5, 
		 TOP_GP32_LCW_GT_MD_CRH_AR_BP_U5, 
		 TOP_GP32_LCW_GT_MD_CRH_AR_QM_U5, 
		 TOP_GP32_LCW_GT_MD_CRH_AR_QP_U5, 
		 TOP_UNDEFINED); 

  Result (0, ctrlh); 
  Same_Res (2); 
  Result (1, ptr32); 
  Same_Res (3); 
  Operand (0, pr, predicate); 
  Operand (1, md); 
  Operand (2, ctrlh); 
  Operand (3, ptr32, base); 
  Operand (4, u5, offset); 

  /* ====================================== */ 
  Instruction_Group("O_35", 
		 TOP_GP32_LCG_GT_MD_BR_AR_M_AR, 
		 TOP_GP32_LCG_GT_MD_BR_AR_P_AR, 
		 TOP_UNDEFINED); 

  Result (0, pr); 
  Operand (0, pr, predicate); 
  Operand (1, md); 
  Operand (2, ptr32, base); 
  Operand (3, ptr32, offset); 

  /* ====================================== */ 
  Instruction_Group("O_36", 
		 TOP_GP32_SWI_U12, 
		 TOP_UNDEFINED); 

  Operand (0, u12); 
  Operand (1, cr29, opnd1); 
  Operand (2, p15, opnd2); 

  /* ====================================== */ 
  Instruction_Group("O_37", 
		 TOP_GP32_SCW_GT_AR_BM_U5_CRH, 
		 TOP_GP32_SCW_GT_AR_BP_U5_CRH, 
		 TOP_GP32_SCW_GT_AR_MQ_U5_CRH, 
		 TOP_GP32_SCW_GT_AR_QM_U5_CRH, 
		 TOP_GP32_SCW_GT_AR_QP_U5_CRH, 
		 TOP_UNDEFINED); 

  Result (0, ptr32); 
  Same_Res (1); 
  Operand (0, pr, predicate); 
  Operand (1, ptr32, base); 
  Operand (2, u5, offset); 
  Operand (3, ctrlh, storeval); 

  /* ====================================== */ 
  Instruction_Group("O_38", 
		 TOP_GP32_LFR_GT_MD_AR_M_AR, 
		 TOP_GP32_LFR_GT_MD_AR_P_AR, 
		 TOP_UNDEFINED); 

  Result (0, cr8); 
  Operand (0, pr, predicate); 
  Operand (1, md); 
  Operand (2, ptr32, base); 
  Operand (3, ptr32, offset); 

  /* ====================================== */ 
  Instruction_Group("O_39", 
		 TOP_GP32_BFPSR0_GT_U8_U8, 
		 TOP_GP32_BFPSR1_GT_U8_U8, 
		 TOP_GP32_BFPSR2_GT_U8_U8, 
		 TOP_UNDEFINED); 

  Operand (0, pr, predicate); 
  Operand (1, u8, opnd1); 
  Operand (2, u8, opnd2); 

  /* ====================================== */ 
  Instruction_Group("O_40", 
		 TOP_GP32_LAH_GT_AR_AR_M_U9, 
		 TOP_GP32_LAH_GT_AR_AR_P_U9, 
		 TOP_GP32_LAW_GT_AR_AR_M_U9, 
		 TOP_GP32_LAW_GT_AR_AR_P_U9, 
		 TOP_UNDEFINED); 

  Result (0, ptr32); 
  Operand (0, pr, predicate); 
  Operand (1, ptr32, base); 
  Operand (2, u9, offset); 

  /* ====================================== */ 
  Instruction_Group("O_41", 
		 TOP_GP32_SFR_GT_MD_AR_M_U5, 
		 TOP_GP32_SFR_GT_MD_AR_P_U5, 
		 TOP_UNDEFINED); 

  Operand (0, pr, predicate); 
  Operand (1, md); 
  Operand (2, ptr32, base); 
  Operand (3, u5, offset); 
  Operand (3, u5, storeval); 
  Operand (4, cr8); 

  /* ====================================== */ 
  Instruction_Group("O_42", 
		 TOP_GP32_SGR_GT_MD_AR_M_U5, 
		 TOP_GP32_SGR_GT_MD_AR_P_U5, 
		 TOP_UNDEFINED); 

  Operand (0, pr, predicate); 
  Operand (1, md); 
  Operand (2, ptr32, base); 
  Operand (3, u5, offset); 
  Operand (3, u5, storeval); 
  Operand (4, cr9); 

  /* ====================================== */ 
  Instruction_Group("O_43", 
		 TOP_GP32_LDBP_GT_MD_DR_AR_M_U5, 
		 TOP_GP32_LDBP_GT_MD_DR_AR_P_U5, 
		 TOP_GP32_LDBSW_GT_MD_DR_AR_M_U5, 
		 TOP_GP32_LDBSW_GT_MD_DR_AR_P_U5, 
		 TOP_GP32_LDB_GT_MD_DR_AR_M_U5, 
		 TOP_GP32_LDB_GT_MD_DR_AR_P_U5, 
		 TOP_GP32_LDF_GT_MD_DR_AR_M_U5, 
		 TOP_GP32_LDF_GT_MD_DR_AR_P_U5, 
		 TOP_GP32_LDHSW_GT_MD_DR_AR_M_U5, 
		 TOP_GP32_LDHSW_GT_MD_DR_AR_P_U5, 
		 TOP_GP32_LDH_GT_MD_DR_AR_M_U5, 
		 TOP_GP32_LDH_GT_MD_DR_AR_P_U5, 
		 TOP_GP32_LDP_GT_MD_DR_AR_M_U5, 
		 TOP_GP32_LDP_GT_MD_DR_AR_P_U5, 
		 TOP_GP32_LDSETUB_GT_MD_DR_AR_M_U5, 
		 TOP_GP32_LDSETUB_GT_MD_DR_AR_P_U5, 
		 TOP_GP32_LDUBP_GT_MD_DR_AR_M_U5, 
		 TOP_GP32_LDUBP_GT_MD_DR_AR_P_U5, 
		 TOP_GP32_LDUB_GT_MD_DR_AR_M_U5, 
		 TOP_GP32_LDUB_GT_MD_DR_AR_P_U5, 
		 TOP_GP32_LDUH_GT_MD_DR_AR_M_U5, 
		 TOP_GP32_LDUH_GT_MD_DR_AR_P_U5, 
		 TOP_GP32_LDUW_GT_MD_DR_AR_M_U5, 
		 TOP_GP32_LDUW_GT_MD_DR_AR_P_U5, 
		 TOP_UNDEFINED); 

  Result (0, int40); 
  Operand (0, pr, predicate); 
  Operand (1, md); 
  Operand (2, ptr32, base); 
  Operand (3, u5, offset); 

  /* ====================================== */ 
  Instruction_Group("O_44", 
		 TOP_GP32_SETILE0_S16, 
		 TOP_GP32_SETLE0_S16, 
		 TOP_GP32_SETLS0_S16, 
		 TOP_GP32_SETULS0_S16, 
		 TOP_UNDEFINED); 

  Result (0, lr0); 
  Operand (0, s16, target); 

  /* ====================================== */ 
  Instruction_Group("O_45", 
		 TOP_GP32_SCW_GT_MD_AR_M_U5_CRL, 
		 TOP_GP32_SCW_GT_MD_AR_P_U5_CRL, 
		 TOP_UNDEFINED); 

  Operand (0, pr, predicate); 
  Operand (1, md); 
  Operand (2, ptr32, base); 
  Operand (3, u5, offset); 
  Operand (4, ctrll, storeval); 

  /* ====================================== */ 
  Instruction_Group("O_46", 
		 TOP_GP32_PUSH_RSET, 
		 TOP_UNDEFINED); 

  Result (0, p15); 
  Operand (0, u20); 
  Operand (1, p15, opnd1); 

  /* ====================================== */ 
  Instruction_Group("O_47", 
		 TOP_GP32_BRANCH_GF, 
		 TOP_GP32_JUMP_GF, 
		 TOP_UNDEFINED); 

  Operand (0, pr, predicate); 
  Operand (1, p3); 

  /* ====================================== */ 
  Instruction_Group("O_48", 
		 TOP_GP32_CALLPR_GT_U16, 
		 TOP_UNDEFINED); 

  Result (0, g0); 
  Result (1, p11); 
  Operand (0, pr, predicate); 
  Operand (1, u16, target); 
  Operand (1, u16, opnd1); 
  Operand (2, p3, opnd2); 

  /* ====================================== */ 
  Instruction_Group("O_49", 
		 TOP_GP32_LGR_GT_MD_AR_M_AR, 
		 TOP_GP32_LGR_GT_MD_AR_P_AR, 
		 TOP_UNDEFINED); 

  Result (0, cr9); 
  Operand (0, pr, predicate); 
  Operand (1, md); 
  Operand (2, ptr32, base); 
  Operand (3, ptr32, offset); 

  /* ====================================== */ 
  Instruction_Group("O_50", 
		 TOP_GP32_BARRIER, 
		 TOP_GP32_GP32MD, 
		 TOP_GP32_LOOPDIS, 
		 TOP_GP32_LOOPENA, 
		 TOP_GP32_NOP, 
		 TOP_GP32_RTE, 
		 TOP_GP32_SLIWMD, 
		 TOP_UNDEFINED); 


  /* ====================================== */ 
  Instruction_Group("O_51", 
		 TOP_GP32_LCW_GT_MD_CRH_AR_BM_AR, 
		 TOP_GP32_LCW_GT_MD_CRH_AR_BP_AR, 
		 TOP_GP32_LCW_GT_MD_CRH_AR_QM_AR, 
		 TOP_GP32_LCW_GT_MD_CRH_AR_QP_AR, 
		 TOP_UNDEFINED); 

  Result (0, ctrlh); 
  Same_Res (2); 
  Result (1, ptr32); 
  Same_Res (3); 
  Operand (0, pr, predicate); 
  Operand (1, md); 
  Operand (2, ctrlh); 
  Operand (3, ptr32, base); 
  Operand (4, ptr32, offset); 

  /* ====================================== */ 
  Instruction_Group("O_52", 
		 TOP_GP32_ADDWA_GT_AR_AR_U9, 
		 TOP_UNDEFINED); 

  Result (0, ptr32); 
  Operand (0, pr, predicate); 
  Operand (1, ptr32, opnd1); 
  Operand (2, u11, opnd2); 

  /* ====================================== */ 
  Instruction_Group("O_53", 
		 TOP_GP32_LFR_GT_AR_BM_U5, 
		 TOP_GP32_LFR_GT_AR_BP_U5, 
		 TOP_GP32_LFR_GT_AR_MQ_U5, 
		 TOP_GP32_LFR_GT_AR_QM_U5, 
		 TOP_GP32_LFR_GT_AR_QP_U5, 
		 TOP_UNDEFINED); 

  Result (0, ptr32); 
  Same_Res (1); 
  Result (1, cr8); 
  Operand (0, pr, predicate); 
  Operand (1, ptr32, base); 
  Operand (2, u5, offset); 

  /* ====================================== */ 
  Instruction_Group("O_54", 
		 TOP_GP32_LDBP_GT_MD_DR_AR_BM_U5, 
		 TOP_GP32_LDBP_GT_MD_DR_AR_BP_U5, 
		 TOP_GP32_LDBP_GT_MD_DR_AR_QM_U5, 
		 TOP_GP32_LDBP_GT_MD_DR_AR_QP_U5, 
		 TOP_GP32_LDBSW_GT_MD_DR_AR_BM_U5, 
		 TOP_GP32_LDBSW_GT_MD_DR_AR_BP_U5, 
		 TOP_GP32_LDBSW_GT_MD_DR_AR_QM_U5, 
		 TOP_GP32_LDBSW_GT_MD_DR_AR_QP_U5, 
		 TOP_GP32_LDB_GT_MD_DR_AR_BM_U5, 
		 TOP_GP32_LDB_GT_MD_DR_AR_BP_U5, 
		 TOP_GP32_LDB_GT_MD_DR_AR_QM_U5, 
		 TOP_GP32_LDB_GT_MD_DR_AR_QP_U5, 
		 TOP_GP32_LDF_GT_MD_DR_AR_BM_U5, 
		 TOP_GP32_LDF_GT_MD_DR_AR_BP_U5, 
		 TOP_GP32_LDF_GT_MD_DR_AR_QM_U5, 
		 TOP_GP32_LDF_GT_MD_DR_AR_QP_U5, 
		 TOP_GP32_LDHSW_GT_MD_DR_AR_BM_U5, 
		 TOP_GP32_LDHSW_GT_MD_DR_AR_BP_U5, 
		 TOP_GP32_LDHSW_GT_MD_DR_AR_QM_U5, 
		 TOP_GP32_LDHSW_GT_MD_DR_AR_QP_U5, 
		 TOP_GP32_LDH_GT_MD_DR_AR_BM_U5, 
		 TOP_GP32_LDH_GT_MD_DR_AR_BP_U5, 
		 TOP_GP32_LDH_GT_MD_DR_AR_QM_U5, 
		 TOP_GP32_LDH_GT_MD_DR_AR_QP_U5, 
		 TOP_GP32_LDP_GT_MD_DR_AR_BM_U5, 
		 TOP_GP32_LDP_GT_MD_DR_AR_BP_U5, 
		 TOP_GP32_LDP_GT_MD_DR_AR_QM_U5, 
		 TOP_GP32_LDP_GT_MD_DR_AR_QP_U5, 
		 TOP_GP32_LDSETUB_GT_MD_DR_AR_BM_U5, 
		 TOP_GP32_LDSETUB_GT_MD_DR_AR_BP_U5, 
		 TOP_GP32_LDSETUB_GT_MD_DR_AR_QM_U5, 
		 TOP_GP32_LDSETUB_GT_MD_DR_AR_QP_U5, 
		 TOP_GP32_LDUBP_GT_MD_DR_AR_BM_U5, 
		 TOP_GP32_LDUBP_GT_MD_DR_AR_BP_U5, 
		 TOP_GP32_LDUBP_GT_MD_DR_AR_QM_U5, 
		 TOP_GP32_LDUBP_GT_MD_DR_AR_QP_U5, 
		 TOP_GP32_LDUB_GT_MD_DR_AR_BM_U5, 
		 TOP_GP32_LDUB_GT_MD_DR_AR_BP_U5, 
		 TOP_GP32_LDUB_GT_MD_DR_AR_QM_U5, 
		 TOP_GP32_LDUB_GT_MD_DR_AR_QP_U5, 
		 TOP_GP32_LDUH_GT_MD_DR_AR_BM_U5, 
		 TOP_GP32_LDUH_GT_MD_DR_AR_BP_U5, 
		 TOP_GP32_LDUH_GT_MD_DR_AR_QM_U5, 
		 TOP_GP32_LDUH_GT_MD_DR_AR_QP_U5, 
		 TOP_GP32_LDUW_GT_MD_DR_AR_BM_U5, 
		 TOP_GP32_LDUW_GT_MD_DR_AR_BP_U5, 
		 TOP_GP32_LDUW_GT_MD_DR_AR_QM_U5, 
		 TOP_GP32_LDUW_GT_MD_DR_AR_QP_U5, 
		 TOP_GP32_LDW_GT_MD_DR_AR_BM_U5, 
		 TOP_GP32_LDW_GT_MD_DR_AR_BP_U5, 
		 TOP_UNDEFINED); 

  Result (0, int40); 
  Result (1, ptr32); 
  Same_Res (3); 
  Operand (0, pr, predicate); 
  Operand (1, md); 
  Operand (2, ptr32, base); 
  Operand (3, u5, offset); 

  /* ====================================== */ 
  Instruction_Group("O_55", 
		 TOP_GP32_EQE_GT_BR_DR_U8, 
		 TOP_GP32_EQP_GT_BR_DR_U8, 
		 TOP_GP32_EQUE_GT_BR_DR_U8, 
		 TOP_GP32_EQUP_GT_BR_DR_U8, 
		 TOP_GP32_EQUW_GT_BR_DR_U8, 
		 TOP_GP32_EQW_GT_BR_DR_U8, 
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
		 TOP_GP32_NEE_GT_BR_DR_U8, 
		 TOP_GP32_NEP_GT_BR_DR_U8, 
		 TOP_GP32_NEUE_GT_BR_DR_U8, 
		 TOP_GP32_NEUP_GT_BR_DR_U8, 
		 TOP_GP32_NEUW_GT_BR_DR_U8, 
		 TOP_GP32_NEW_GT_BR_DR_U8, 
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
		 TOP_UNDEFINED); 

  Result (0, pr); 
  Operand (0, pr, predicate); 
  Operand (1, int40, opnd1); 
  Operand (2, u8, opnd2); 

  /* ====================================== */ 
  Instruction_Group("O_56", 
		 TOP_GP32_LDW_GT_MD_DR_AR_M_U5, 
		 TOP_GP32_LDW_GT_MD_DR_AR_P_U5, 
		 TOP_UNDEFINED); 

  Result (0, int40); 
  Operand (0, pr, predicate); 
  Operand (1, md); 
  Operand (2, ptr32, base); 
  Operand (3, u7, offset); 

  /* ====================================== */ 
  Instruction_Group("O_57", 
		 TOP_GP32_SAH_GT_AR_M_AR_AR, 
		 TOP_GP32_SAH_GT_AR_P_AR_AR, 
		 TOP_GP32_SAW_GT_AR_M_AR_AR, 
		 TOP_GP32_SAW_GT_AR_P_AR_AR, 
		 TOP_UNDEFINED); 

  Operand (0, pr, predicate); 
  Operand (1, ptr32, base); 
  Operand (2, ptr32, offset); 
  Operand (3, ptr32, storeval); 

  /* ====================================== */ 
  Instruction_Group("O_58", 
		 TOP_GP32_LFR_GT_MD_AR_BM_U5, 
		 TOP_GP32_LFR_GT_MD_AR_BP_U5, 
		 TOP_GP32_LFR_GT_MD_AR_QM_U5, 
		 TOP_GP32_LFR_GT_MD_AR_QP_U5, 
		 TOP_UNDEFINED); 

  Result (0, ptr32); 
  Same_Res (2); 
  Result (1, cr8); 
  Operand (0, pr, predicate); 
  Operand (1, md); 
  Operand (2, ptr32, base); 
  Operand (3, u5, offset); 

  /* ====================================== */ 
  Instruction_Group("O_59", 
		 TOP_GP32_MOVEHH_GT_DR_DR, 
		 TOP_GP32_MOVEHL_GT_DR_DR, 
		 TOP_GP32_MOVELH_GT_DR_DR, 
		 TOP_GP32_MOVELL_GT_DR_DR, 
		 TOP_UNDEFINED); 

  Result (0, int40); 
  Same_Res (1); 
  Operand (0, pr, predicate); 
  Operand (1, int40, opnd1); 
  Operand (2, int40, opnd2); 

  /* ====================================== */ 
  Instruction_Group("O_60", 
		 TOP_GP32_LCW_GT_MD_CRH_AR_M_U5, 
		 TOP_GP32_LCW_GT_MD_CRH_AR_P_U5, 
		 TOP_UNDEFINED); 

  Result (0, ctrlh); 
  Same_Res (2); 
  Operand (0, pr, predicate); 
  Operand (1, md); 
  Operand (2, ctrlh); 
  Operand (3, ptr32, base); 
  Operand (4, u5, offset); 

  /* ====================================== */ 
  Instruction_Group("O_61", 
		 TOP_GP32_LFR_GT_AR_M_U9, 
		 TOP_GP32_LFR_GT_AR_P_U9, 
		 TOP_UNDEFINED); 

  Result (0, cr8); 
  Operand (0, pr, predicate); 
  Operand (1, ptr32, base); 
  Operand (2, u9, offset); 

  /* ====================================== */ 
  Instruction_Group("O_62", 
		 TOP_GP32_SAH_GT_AR_BM_AR_AR, 
		 TOP_GP32_SAH_GT_AR_BP_AR_AR, 
		 TOP_GP32_SAH_GT_AR_QM_AR_AR, 
		 TOP_GP32_SAH_GT_AR_QP_AR_AR, 
		 TOP_GP32_SAW_GT_AR_BM_AR_AR, 
		 TOP_GP32_SAW_GT_AR_BP_AR_AR, 
		 TOP_GP32_SAW_GT_AR_QM_AR_AR, 
		 TOP_GP32_SAW_GT_AR_QP_AR_AR, 
		 TOP_UNDEFINED); 

  Result (0, ptr32); 
  Same_Res (1); 
  Operand (0, pr, predicate); 
  Operand (1, ptr32, base); 
  Operand (2, ptr32, offset); 
  Operand (3, ptr32, storeval); 

  /* ====================================== */ 
  Instruction_Group("O_63", 
		 TOP_GP32_SAH_GT_P13_P_U15_AR, 
		 TOP_GP32_SAW_GT_P13_P_U15_AR, 
		 TOP_UNDEFINED); 

  Operand (0, pr, predicate); 
  Operand (1, p13, base); 
  Operand (2, u15, offset); 
  Operand (3, ptr32, storeval); 

  /* ====================================== */ 
  Instruction_Group("O_64", 
		 TOP_GP32_FBPOS_GT_BR_DR_U5, 
		 TOP_GP32_TBPOS_GT_BR_DR_U5, 
		 TOP_UNDEFINED); 

  Result (0, pr); 
  Operand (0, pr, predicate); 
  Operand (1, int40, opnd1); 
  Operand (2, u5, opnd2); 

  /* ====================================== */ 
  Instruction_Group("O_65", 
		 TOP_GP32_SDBP_GT_MD_AR_M_U5_DR, 
		 TOP_GP32_SDBP_GT_MD_AR_P_U5_DR, 
		 TOP_GP32_SDBSW_GT_MD_AR_M_U5_DR, 
		 TOP_GP32_SDBSW_GT_MD_AR_P_U5_DR, 
		 TOP_GP32_SDB_GT_MD_AR_M_U5_DR, 
		 TOP_GP32_SDB_GT_MD_AR_P_U5_DR, 
		 TOP_GP32_SDEW_GT_MD_AR_M_U5_DR, 
		 TOP_GP32_SDEW_GT_MD_AR_P_U5_DR, 
		 TOP_GP32_SDF_GT_MD_AR_M_U5_DR, 
		 TOP_GP32_SDF_GT_MD_AR_P_U5_DR, 
		 TOP_GP32_SDHSW_GT_MD_AR_M_U5_DR, 
		 TOP_GP32_SDHSW_GT_MD_AR_P_U5_DR, 
		 TOP_GP32_SDH_GT_MD_AR_M_U5_DR, 
		 TOP_GP32_SDH_GT_MD_AR_P_U5_DR, 
		 TOP_GP32_SDP_GT_MD_AR_M_U5_DR, 
		 TOP_GP32_SDP_GT_MD_AR_P_U5_DR, 
		 TOP_GP32_SDW_GT_MD_AR_M_U5_DR, 
		 TOP_GP32_SDW_GT_MD_AR_P_U5_DR, 
		 TOP_UNDEFINED); 

  Operand (0, pr, predicate); 
  Operand (1, md); 
  Operand (2, ptr32, base); 
  Operand (3, u5, offset); 
  Operand (4, int40, storeval); 

  /* ====================================== */ 
  Instruction_Group("O_66", 
		 TOP_GP32_POPRTE_RSET, 
		 TOP_GP32_POPRTS_RSET, 
		 TOP_GP32_POP_RSET, 
		 TOP_UNDEFINED); 

  Result (0, u20); 
  Result (1, p15); 
  Operand (0, p15); 

  /* ====================================== */ 
  Instruction_Group("O_67", 
		 TOP_GP32_BRANCH, 
		 TOP_GP32_JUMP, 
		 TOP_UNDEFINED); 

  Operand (0, p3); 

  /* ====================================== */ 
  Instruction_Group("O_68", 
		 TOP_GP32_SHLUM_GT_DR_U5, 
		 TOP_GP32_SHRUWM_GT_DR_U5, 
		 TOP_UNDEFINED); 

  Result (0, int40); 
  Operand (0, pr, predicate); 
  Operand (1, u5, opnd1); 

  /* ====================================== */ 
  Instruction_Group("O_69", 
		 TOP_GP32_MAKEA_GT_AR_S16, 
		 TOP_GP32_MAKEBA_GT_AR_S16, 
		 TOP_GP32_MAKEHA_GT_AR_S16, 
		 TOP_GP32_MAKEWA_GT_AR_S16, 
		 TOP_UNDEFINED); 

  Result (0, ptr32); 
  Operand (0, pr, predicate); 
  Operand (1, s16, opnd1); 

  /* ====================================== */ 
  Instruction_Group("O_70", 
		 TOP_GP32_LDBP_GT_DR_P13_P_U15, 
		 TOP_GP32_LDBSW_GT_DR_P13_P_U15, 
		 TOP_GP32_LDB_GT_DR_P13_P_U15, 
		 TOP_GP32_LDF_GT_DR_P13_P_U15, 
		 TOP_GP32_LDHSW_GT_DR_P13_P_U15, 
		 TOP_GP32_LDH_GT_DR_P13_P_U15, 
		 TOP_GP32_LDP_GT_DR_P13_P_U15, 
		 TOP_GP32_LDSETUB_GT_DR_P13_P_U15, 
		 TOP_GP32_LDUBP_GT_DR_P13_P_U15, 
		 TOP_GP32_LDUB_GT_DR_P13_P_U15, 
		 TOP_GP32_LDUH_GT_DR_P13_P_U15, 
		 TOP_GP32_LDUW_GT_DR_P13_P_U15, 
		 TOP_GP32_LDW_GT_DR_P13_P_U15, 
		 TOP_UNDEFINED); 

  Result (0, int40); 
  Operand (0, pr, predicate); 
  Operand (1, p13, base); 
  Operand (2, u15, offset); 

  /* ====================================== */ 
  Instruction_Group("O_71", 
		 TOP_GP32_LDW_GT_MD_DR_AR_QM_U5, 
		 TOP_GP32_LDW_GT_MD_DR_AR_QP_U5, 
		 TOP_UNDEFINED); 

  Result (0, int40); 
  Result (1, ptr32); 
  Same_Res (3); 
  Operand (0, pr, predicate); 
  Operand (1, md); 
  Operand (2, ptr32, base); 
  Operand (3, u7, offset); 

  /* ====================================== */ 
  Instruction_Group("O_72", 
		 TOP_GP32_CALL_GT_S21, 
		 TOP_UNDEFINED); 

  Result (0, g0); 
  Result (1, p11); 
  Operand (0, pr, predicate); 
  Operand (1, s21, target); 

  /* ====================================== */ 
  Instruction_Group("O_73", 
		 TOP_GP32_SFR_GT_P13_P_U15, 
		 TOP_UNDEFINED); 

  Operand (0, pr, predicate); 
  Operand (1, p13, base); 
  Operand (2, u15, offset); 
  Operand (2, u15, storeval); 
  Operand (3, cr8); 

  /* ====================================== */ 
  Instruction_Group("O_74", 
		 TOP_GP32_SGR_GT_P13_P_U15, 
		 TOP_UNDEFINED); 

  Operand (0, pr, predicate); 
  Operand (1, p13, base); 
  Operand (2, u15, offset); 
  Operand (2, u15, storeval); 
  Operand (3, cr9); 

  /* ====================================== */ 
  Instruction_Group("O_GP32_CALL_S25", 
		 TOP_GP32_CALL_S25, 
		 TOP_UNDEFINED); 

  Operand (0, s25); 

  /* ====================================== */ 
  Instruction_Group("O_75", 
		 TOP_GP32_LDEW_GT_MD_DR_AR_M_AR, 
		 TOP_GP32_LDEW_GT_MD_DR_AR_P_AR, 
		 TOP_GP32_LDHH_GT_MD_DR_AR_M_AR, 
		 TOP_GP32_LDHH_GT_MD_DR_AR_P_AR, 
		 TOP_GP32_LDLH_GT_MD_DR_AR_M_AR, 
		 TOP_GP32_LDLH_GT_MD_DR_AR_P_AR, 
		 TOP_UNDEFINED); 

  Result (0, int40); 
  Same_Res (2); 
  Operand (0, pr, predicate); 
  Operand (1, md); 
  Operand (2, int40); 
  Operand (3, ptr32, base); 
  Operand (4, ptr32, offset); 

  /* ====================================== */ 
  Instruction_Group("O_76", 
		 TOP_GP32_LDEW_GT_DR_AR_M_AR, 
		 TOP_GP32_LDEW_GT_DR_AR_P_AR, 
		 TOP_GP32_LDHH_GT_DR_AR_M_AR, 
		 TOP_GP32_LDHH_GT_DR_AR_P_AR, 
		 TOP_GP32_LDLH_GT_DR_AR_M_AR, 
		 TOP_GP32_LDLH_GT_DR_AR_P_AR, 
		 TOP_UNDEFINED); 

  Result (0, int40); 
  Same_Res (1); 
  Operand (0, pr, predicate); 
  Operand (1, int40); 
  Operand (2, ptr32, base); 
  Operand (3, ptr32, offset); 

  /* ====================================== */ 
  Instruction_Group("O_noop", 
		 TOP_noop, 
		 TOP_UNDEFINED); 

  Operand (0, pr, predicate); 

  /* ====================================== */ 
  Instruction_Group("O_77", 
		 TOP_GP32_SCW_GT_MD_AR_BM_AR_CRH, 
		 TOP_GP32_SCW_GT_MD_AR_BP_AR_CRH, 
		 TOP_GP32_SCW_GT_MD_AR_QM_AR_CRH, 
		 TOP_GP32_SCW_GT_MD_AR_QP_AR_CRH, 
		 TOP_UNDEFINED); 

  Result (0, ptr32); 
  Same_Res (2); 
  Operand (0, pr, predicate); 
  Operand (1, md); 
  Operand (2, ptr32, base); 
  Operand (3, ptr32, offset); 
  Operand (4, ctrlh, storeval); 

  /* ====================================== */ 
  Instruction_Group("O_78", 
		 TOP_GP32_MAKEC_GT_CRL_P3_U16, 
		 TOP_UNDEFINED); 

  Result (0, ctrll); 
  Operand (0, pr, predicate); 
  Operand (1, p3, implicit); 
  Operand (1, p3, opnd1); 
  Operand (2, u16, opnd2); 

  /* ====================================== */ 
  Instruction_Group("O_79", 
		 TOP_GP32_CLRFR_GT, 
		 TOP_UNDEFINED); 

  Result (0, cr8); 
  Operand (0, pr, predicate); 

  /* ====================================== */ 
  Instruction_Group("O_80", 
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
		 TOP_GP32_GEE_GT_BR_DR_DR, 
		 TOP_GP32_GEP_GT_BR_DR_DR, 
		 TOP_GP32_GEUE_GT_BR_DR_DR, 
		 TOP_GP32_GEUP_GT_BR_DR_DR, 
		 TOP_GP32_GEUW_GT_BR_DR_DR, 
		 TOP_GP32_GEW_GT_BR_DR_DR, 
		 TOP_GP32_GTE_GT_BR_DR_DR, 
		 TOP_GP32_GTP_GT_BR_DR_DR, 
		 TOP_GP32_GTUE_GT_BR_DR_DR, 
		 TOP_GP32_GTUP_GT_BR_DR_DR, 
		 TOP_GP32_GTUW_GT_BR_DR_DR, 
		 TOP_GP32_GTW_GT_BR_DR_DR, 
		 TOP_GP32_LEE_GT_BR_DR_DR, 
		 TOP_GP32_LEP_GT_BR_DR_DR, 
		 TOP_GP32_LEUE_GT_BR_DR_DR, 
		 TOP_GP32_LEUP_GT_BR_DR_DR, 
		 TOP_GP32_LEUW_GT_BR_DR_DR, 
		 TOP_GP32_LEW_GT_BR_DR_DR, 
		 TOP_GP32_LTE_GT_BR_DR_DR, 
		 TOP_GP32_LTP_GT_BR_DR_DR, 
		 TOP_GP32_LTUE_GT_BR_DR_DR, 
		 TOP_GP32_LTUP_GT_BR_DR_DR, 
		 TOP_GP32_LTUW_GT_BR_DR_DR, 
		 TOP_GP32_LTW_GT_BR_DR_DR, 
		 TOP_GP32_NEE_GT_BR_DR_DR, 
		 TOP_GP32_NEP_GT_BR_DR_DR, 
		 TOP_GP32_NEUE_GT_BR_DR_DR, 
		 TOP_GP32_NEUP_GT_BR_DR_DR, 
		 TOP_GP32_NEUW_GT_BR_DR_DR, 
		 TOP_GP32_NEW_GT_BR_DR_DR, 
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
		 TOP_UNDEFINED); 

  Result (0, pr); 
  Operand (0, pr, predicate); 
  Operand (1, int40, opnd1); 
  Operand (2, int40, opnd2); 

  /* ====================================== */ 
  Instruction_Group("O_81", 
		 TOP_GP32_TRAP_GT_U4, 
		 TOP_UNDEFINED); 

  Operand (0, pr, predicate); 
  Operand (1, u4, opnd1); 
  Operand (2, p15, opnd2); 

  /* ====================================== */ 
  Instruction_Group("O_82", 
		 TOP_GP32_SAH_GT_AR_M_U9_AR, 
		 TOP_GP32_SAH_GT_AR_P_U9_AR, 
		 TOP_GP32_SAW_GT_AR_M_U9_AR, 
		 TOP_GP32_SAW_GT_AR_P_U9_AR, 
		 TOP_UNDEFINED); 

  Operand (0, pr, predicate); 
  Operand (1, ptr32, base); 
  Operand (2, u9, offset); 
  Operand (3, ptr32, storeval); 

  /* ====================================== */ 
  Instruction_Group("O_GP32_RTS_GT", 
		 TOP_GP32_RTS_GT, 
		 TOP_UNDEFINED); 

  Operand (0, pr, predicate); 
  Operand (1, p11, implicit); 
  Operand (1, p11, target); 

  /* ====================================== */ 
  Instruction_Group("O_83", 
		 TOP_GP32_SFR_GT_AR_M_AR, 
		 TOP_GP32_SFR_GT_AR_P_AR, 
		 TOP_UNDEFINED); 

  Operand (0, pr, predicate); 
  Operand (1, ptr32, base); 
  Operand (2, ptr32, offset); 
  Operand (2, ptr32, storeval); 
  Operand (3, cr8); 

  /* ====================================== */ 
  Instruction_Group("O_84", 
		 TOP_GP32_SGR_GT_AR_M_AR, 
		 TOP_GP32_SGR_GT_AR_P_AR, 
		 TOP_UNDEFINED); 

  Operand (0, pr, predicate); 
  Operand (1, ptr32, base); 
  Operand (2, ptr32, offset); 
  Operand (2, ptr32, storeval); 
  Operand (3, cr9); 

  /* ====================================== */ 
  Instruction_Group("O_85", 
		 TOP_GP32_MOREA_GT_AR_U16, 
		 TOP_UNDEFINED); 

  Result (0, ptr32); 
  Same_Res (1); 
  Operand (0, pr, predicate); 
  Operand (1, ptr32, opnd1); 
  Operand (2, u16, opnd2); 

  /* ====================================== */ 
  Instruction_Group("O_86", 
		 TOP_GP32_GOTOPR_GF_U16, 
		 TOP_UNDEFINED); 

  Operand (0, pr, predicate); 
  Operand (1, u16, target); 
  Operand (2, p3); 

  /* ====================================== */ 
  Instruction_Group("O_87", 
		 TOP_GP32_MAKEB_GT_DR_S32, 
		 TOP_UNDEFINED); 

  Result (0, int40); 
  Operand (0, pr, predicate); 
  Operand (1, s32, opnd1); 

  /* ====================================== */ 
  Instruction_Group("O_88", 
		 TOP_GP32_SCW_GT_MD_AR_M_AR_CRH, 
		 TOP_GP32_SCW_GT_MD_AR_P_AR_CRH, 
		 TOP_UNDEFINED); 

  Operand (0, pr, predicate); 
  Operand (1, md); 
  Operand (2, ptr32, base); 
  Operand (3, ptr32, offset); 
  Operand (4, ctrlh, storeval); 

  /* ====================================== */ 
  Instruction_Group("O_89", 
		 TOP_GP32_SCW_GT_MD_AR_BM_U5_CRH, 
		 TOP_GP32_SCW_GT_MD_AR_BP_U5_CRH, 
		 TOP_GP32_SCW_GT_MD_AR_QM_U5_CRH, 
		 TOP_GP32_SCW_GT_MD_AR_QP_U5_CRH, 
		 TOP_UNDEFINED); 

  Result (0, ptr32); 
  Same_Res (2); 
  Operand (0, pr, predicate); 
  Operand (1, md); 
  Operand (2, ptr32, base); 
  Operand (3, u5, offset); 
  Operand (4, ctrlh, storeval); 

  /* ====================================== */ 
  Instruction_Group("O_90", 
		 TOP_GP32_LDW_GT_DR_AR_M_U9, 
		 TOP_GP32_LDW_GT_DR_AR_P_U9, 
		 TOP_UNDEFINED); 

  Result (0, int40); 
  Operand (0, pr, predicate); 
  Operand (1, ptr32, base); 
  Operand (2, u11, offset); 

  /* ====================================== */ 
  Instruction_Group("O_91", 
		 TOP_GP32_ANDG_GT_BR_BR_BR, 
		 TOP_GP32_ANDNG_GT_BR_BR_BR, 
		 TOP_GP32_ANDNPG_GT_BR_BR_BR, 
		 TOP_GP32_ANDPG_GT_BR_BR_BR, 
		 TOP_GP32_NANDG_GT_BR_BR_BR, 
		 TOP_GP32_NANDPG_GT_BR_BR_BR, 
		 TOP_GP32_NORG_GT_BR_BR_BR, 
		 TOP_GP32_NORPG_GT_BR_BR_BR, 
		 TOP_GP32_ORG_GT_BR_BR_BR, 
		 TOP_GP32_ORNG_GT_BR_BR_BR, 
		 TOP_GP32_ORNPG_GT_BR_BR_BR, 
		 TOP_GP32_ORPG_GT_BR_BR_BR, 
		 TOP_GP32_XNORG_GT_BR_BR_BR, 
		 TOP_GP32_XNORPG_GT_BR_BR_BR, 
		 TOP_GP32_XORG_GT_BR_BR_BR, 
		 TOP_GP32_XORPG_GT_BR_BR_BR, 
		 TOP_UNDEFINED); 

  Result (0, pr); 
  Operand (0, pr, predicate); 
  Operand (1, pr, opnd1); 
  Operand (2, pr, opnd2); 

  /* ====================================== */ 
  Instruction_Group("O_92", 
		 TOP_GP32_LAH_GT_AR_AR_M_AR, 
		 TOP_GP32_LAH_GT_AR_AR_P_AR, 
		 TOP_GP32_LAW_GT_AR_AR_M_AR, 
		 TOP_GP32_LAW_GT_AR_AR_P_AR, 
		 TOP_UNDEFINED); 

  Result (0, ptr32); 
  Operand (0, pr, predicate); 
  Operand (1, ptr32, base); 
  Operand (2, ptr32, offset); 

  /* ====================================== */ 
  Instruction_Group("O_fixup", 
		 TOP_ffixup, 
		 TOP_UNDEFINED); 

  Result (0, ptr32); 

  /* ====================================== */ 
  Instruction_Group("O_93", 
		 TOP_GP32_LDBP_GT_DR_AR_M_U9, 
		 TOP_GP32_LDBP_GT_DR_AR_P_U9, 
		 TOP_GP32_LDBSW_GT_DR_AR_M_U9, 
		 TOP_GP32_LDBSW_GT_DR_AR_P_U9, 
		 TOP_GP32_LDB_GT_DR_AR_M_U9, 
		 TOP_GP32_LDB_GT_DR_AR_P_U9, 
		 TOP_GP32_LDF_GT_DR_AR_M_U9, 
		 TOP_GP32_LDF_GT_DR_AR_P_U9, 
		 TOP_GP32_LDHSW_GT_DR_AR_M_U9, 
		 TOP_GP32_LDHSW_GT_DR_AR_P_U9, 
		 TOP_GP32_LDP_GT_DR_AR_M_U9, 
		 TOP_GP32_LDP_GT_DR_AR_P_U9, 
		 TOP_GP32_LDSETUB_GT_DR_AR_M_U9, 
		 TOP_GP32_LDSETUB_GT_DR_AR_P_U9, 
		 TOP_GP32_LDUBP_GT_DR_AR_M_U9, 
		 TOP_GP32_LDUBP_GT_DR_AR_P_U9, 
		 TOP_GP32_LDUB_GT_DR_AR_M_U9, 
		 TOP_GP32_LDUB_GT_DR_AR_P_U9, 
		 TOP_GP32_LDUH_GT_DR_AR_M_U9, 
		 TOP_GP32_LDUH_GT_DR_AR_P_U9, 
		 TOP_GP32_LDUW_GT_DR_AR_M_U9, 
		 TOP_GP32_LDUW_GT_DR_AR_P_U9, 
		 TOP_UNDEFINED); 

  Result (0, int40); 
  Operand (0, pr, predicate); 
  Operand (1, ptr32, base); 
  Operand (2, u9, offset); 

  /* ====================================== */ 
  Instruction_Group("O_barrier", 
		 TOP_bwd_bar, 
		 TOP_fwd_bar, 
		 TOP_UNDEFINED); 


  /* ====================================== */ 
  Instruction_Group("O_94", 
		 TOP_GP32_SFR_GT_AR_BM_U5, 
		 TOP_GP32_SFR_GT_AR_BP_U5, 
		 TOP_GP32_SFR_GT_AR_MQ_U5, 
		 TOP_GP32_SFR_GT_AR_QM_U5, 
		 TOP_GP32_SFR_GT_AR_QP_U5, 
		 TOP_UNDEFINED); 

  Result (0, ptr32); 
  Same_Res (1); 
  Operand (0, pr, predicate); 
  Operand (1, ptr32, base); 
  Operand (2, u5, offset); 
  Operand (2, u5, storeval); 
  Operand (3, cr8); 

  /* ====================================== */ 
  Instruction_Group("O_95", 
		 TOP_GP32_LGR_GT_AR_BM_AR, 
		 TOP_GP32_LGR_GT_AR_BP_AR, 
		 TOP_GP32_LGR_GT_AR_QM_AR, 
		 TOP_GP32_LGR_GT_AR_QP_AR, 
		 TOP_UNDEFINED); 

  Result (0, ptr32); 
  Same_Res (1); 
  Result (1, cr9); 
  Operand (0, pr, predicate); 
  Operand (1, ptr32, base); 
  Operand (2, ptr32, offset); 

  /* ====================================== */ 
  Instruction_Group("O_96", 
		 TOP_GP32_LAH_GT_AR_AR_BM_U5, 
		 TOP_GP32_LAH_GT_AR_AR_BP_U5, 
		 TOP_GP32_LAH_GT_AR_AR_MQ_U5, 
		 TOP_GP32_LAH_GT_AR_AR_QM_U5, 
		 TOP_GP32_LAH_GT_AR_AR_QP_U5, 
		 TOP_GP32_LAW_GT_AR_AR_BM_U5, 
		 TOP_GP32_LAW_GT_AR_AR_BP_U5, 
		 TOP_GP32_LAW_GT_AR_AR_MQ_U5, 
		 TOP_GP32_LAW_GT_AR_AR_QM_U5, 
		 TOP_GP32_LAW_GT_AR_AR_QP_U5, 
		 TOP_UNDEFINED); 

  Result (0, ptr32); 
  Result (1, ptr32); 
  Same_Res (2); 
  Operand (0, pr, predicate); 
  Operand (1, ptr32, base); 
  Operand (2, u5, offset); 

  /* ====================================== */ 
  Instruction_Group("O_97", 
		 TOP_GP32_SGR_GT_AR_BM_U5, 
		 TOP_GP32_SGR_GT_AR_BP_U5, 
		 TOP_GP32_SGR_GT_AR_MQ_U5, 
		 TOP_GP32_SGR_GT_AR_QM_U5, 
		 TOP_GP32_SGR_GT_AR_QP_U5, 
		 TOP_UNDEFINED); 

  Result (0, ptr32); 
  Same_Res (1); 
  Operand (0, pr, predicate); 
  Operand (1, ptr32, base); 
  Operand (2, u5, offset); 
  Operand (2, u5, storeval); 
  Operand (3, cr9); 

  /* ====================================== */ 
  Instruction_Group("O_98", 
		 TOP_GP32_LDEW_GT_DR_AR_M_U9, 
		 TOP_GP32_LDEW_GT_DR_AR_P_U9, 
		 TOP_GP32_LDHH_GT_DR_AR_M_U9, 
		 TOP_GP32_LDHH_GT_DR_AR_P_U9, 
		 TOP_GP32_LDLH_GT_DR_AR_M_U9, 
		 TOP_GP32_LDLH_GT_DR_AR_P_U9, 
		 TOP_UNDEFINED); 

  Result (0, int40); 
  Same_Res (1); 
  Operand (0, pr, predicate); 
  Operand (1, int40); 
  Operand (2, ptr32, base); 
  Operand (3, u9, offset); 

  /* ====================================== */ 
  Instruction_Group("O_99", 
		 TOP_GP32_FBCLR_GT_BR_DR_DR_U5, 
		 TOP_GP32_FBNOT_GT_BR_DR_DR_U5, 
		 TOP_GP32_FBSET_GT_BR_DR_DR_U5, 
		 TOP_GP32_TBCLR_GT_BR_DR_DR_U5, 
		 TOP_GP32_TBNOT_GT_BR_DR_DR_U5, 
		 TOP_GP32_TBSET_GT_BR_DR_DR_U5, 
		 TOP_UNDEFINED); 

  Result (0, pr); 
  Result (1, int40); 
  Operand (0, pr, predicate); 
  Operand (1, int40, opnd1); 
  Operand (2, u5, opnd2); 

  /* ====================================== */ 
  Instruction_Group("O_100", 
		 TOP_GP32_BOOLP_GT_DR_BR, 
		 TOP_GP32_BOOL_GT_DR_BR, 
		 TOP_UNDEFINED); 

  Result (0, int40); 
  Operand (0, pr, predicate); 
  Operand (1, pr, opnd1); 

  /* ====================================== */ 
  Instruction_Group("O_101", 
		 TOP_GP32_LCG_GT_BR_AR_M_AR, 
		 TOP_GP32_LCG_GT_BR_AR_P_AR, 
		 TOP_UNDEFINED); 

  Result (0, pr); 
  Operand (0, pr, predicate); 
  Operand (1, ptr32, base); 
  Operand (2, ptr32, offset); 

  /* ====================================== */ 
  Instruction_Group("O_label", 
		 TOP_label, 
		 TOP_UNDEFINED); 

  Operand (0, pcrel); 

  /* ====================================== */ 
  Instruction_Group("O_102", 
		 TOP_GP32_COPYC_GT_CRL_DR, 
		 TOP_UNDEFINED); 

  Result (0, ctrll); 
  Operand (0, pr, predicate); 
  Operand (1, int40, opnd1); 

  /* ====================================== */ 
  Instruction_Group("O_103", 
		 TOP_GP32_LAH_GT_MD_AR_AR_M_U5, 
		 TOP_GP32_LAH_GT_MD_AR_AR_P_U5, 
		 TOP_GP32_LAW_GT_MD_AR_AR_M_U5, 
		 TOP_GP32_LAW_GT_MD_AR_AR_P_U5, 
		 TOP_UNDEFINED); 

  Result (0, ptr32); 
  Operand (0, pr, predicate); 
  Operand (1, md); 
  Operand (2, ptr32, base); 
  Operand (3, u5, offset); 

  /* ====================================== */ 
  Instruction_Group("O_104", 
		 TOP_GP32_LGR_GT_AR_M_AR, 
		 TOP_GP32_LGR_GT_AR_P_AR, 
		 TOP_UNDEFINED); 

  Result (0, cr9); 
  Operand (0, pr, predicate); 
  Operand (1, ptr32, base); 
  Operand (2, ptr32, offset); 

  /* ====================================== */ 
  Instruction_Group("O_105", 
		 TOP_GP32_CLRG_GT_BR, 
		 TOP_GP32_CLRPG_GT_BR, 
		 TOP_GP32_SETG_GT_BR, 
		 TOP_GP32_SETPG_GT_BR, 
		 TOP_UNDEFINED); 

  Result (0, pr); 
  Operand (0, pr, predicate); 

  /* ====================================== */ 
  Instruction_Group("O_106", 
		 TOP_GP32_ADDCP_GT_DR_DR_U8, 
		 TOP_GP32_ADDCW_GT_DR_DR_U8, 
		 TOP_GP32_ADDP_GT_DR_DR_U8, 
		 TOP_GP32_ADDUP_GT_DR_DR_U8, 
		 TOP_GP32_ADDU_GT_DR_DR_U8, 
		 TOP_GP32_ADD_GT_DR_DR_U8, 
		 TOP_GP32_ANDNP_GT_DR_DR_U8, 
		 TOP_GP32_ANDN_GT_DR_DR_U8, 
		 TOP_GP32_ANDP_GT_DR_DR_U8, 
		 TOP_GP32_AND_GT_DR_DR_U8, 
		 TOP_GP32_ORNP_GT_DR_DR_U8, 
		 TOP_GP32_ORN_GT_DR_DR_U8, 
		 TOP_GP32_ORP_GT_DR_DR_U8, 
		 TOP_GP32_OR_GT_DR_DR_U8, 
		 TOP_GP32_SUBCP_GT_DR_DR_U8, 
		 TOP_GP32_SUBCW_GT_DR_DR_U8, 
		 TOP_GP32_SUBP_GT_DR_DR_U8, 
		 TOP_GP32_SUBUP_GT_DR_DR_U8, 
		 TOP_GP32_SUBU_GT_DR_DR_U8, 
		 TOP_GP32_SUB_GT_DR_DR_U8, 
		 TOP_GP32_XNORP_GT_DR_DR_U8, 
		 TOP_GP32_XNOR_GT_DR_DR_U8, 
		 TOP_GP32_XORP_GT_DR_DR_U8, 
		 TOP_GP32_XOR_GT_DR_DR_U8, 
		 TOP_UNDEFINED); 

  Result (0, int40); 
  Operand (0, pr, predicate); 
  Operand (1, int40, opnd1); 
  Operand (2, u8, opnd2); 

  /* ====================================== */ 
  Instruction_Group("O_107", 
		 TOP_GP32_LDBP_GT_DR_AR_BM_AR, 
		 TOP_GP32_LDBP_GT_DR_AR_BP_AR, 
		 TOP_GP32_LDBP_GT_DR_AR_QM_AR, 
		 TOP_GP32_LDBP_GT_DR_AR_QP_AR, 
		 TOP_GP32_LDBSW_GT_DR_AR_BM_AR, 
		 TOP_GP32_LDBSW_GT_DR_AR_BP_AR, 
		 TOP_GP32_LDBSW_GT_DR_AR_QM_AR, 
		 TOP_GP32_LDBSW_GT_DR_AR_QP_AR, 
		 TOP_GP32_LDB_GT_DR_AR_BM_AR, 
		 TOP_GP32_LDB_GT_DR_AR_BP_AR, 
		 TOP_GP32_LDB_GT_DR_AR_QM_AR, 
		 TOP_GP32_LDB_GT_DR_AR_QP_AR, 
		 TOP_GP32_LDF_GT_DR_AR_BM_AR, 
		 TOP_GP32_LDF_GT_DR_AR_BP_AR, 
		 TOP_GP32_LDF_GT_DR_AR_QM_AR, 
		 TOP_GP32_LDF_GT_DR_AR_QP_AR, 
		 TOP_GP32_LDHSW_GT_DR_AR_BM_AR, 
		 TOP_GP32_LDHSW_GT_DR_AR_BP_AR, 
		 TOP_GP32_LDHSW_GT_DR_AR_QM_AR, 
		 TOP_GP32_LDHSW_GT_DR_AR_QP_AR, 
		 TOP_GP32_LDH_GT_DR_AR_BM_AR, 
		 TOP_GP32_LDH_GT_DR_AR_BP_AR, 
		 TOP_GP32_LDH_GT_DR_AR_QM_AR, 
		 TOP_GP32_LDH_GT_DR_AR_QP_AR, 
		 TOP_GP32_LDP_GT_DR_AR_BM_AR, 
		 TOP_GP32_LDP_GT_DR_AR_BP_AR, 
		 TOP_GP32_LDP_GT_DR_AR_QM_AR, 
		 TOP_GP32_LDP_GT_DR_AR_QP_AR, 
		 TOP_GP32_LDSETUB_GT_DR_AR_BM_AR, 
		 TOP_GP32_LDSETUB_GT_DR_AR_BP_AR, 
		 TOP_GP32_LDSETUB_GT_DR_AR_QM_AR, 
		 TOP_GP32_LDSETUB_GT_DR_AR_QP_AR, 
		 TOP_GP32_LDUBP_GT_DR_AR_BM_AR, 
		 TOP_GP32_LDUBP_GT_DR_AR_BP_AR, 
		 TOP_GP32_LDUBP_GT_DR_AR_QM_AR, 
		 TOP_GP32_LDUBP_GT_DR_AR_QP_AR, 
		 TOP_GP32_LDUB_GT_DR_AR_BM_AR, 
		 TOP_GP32_LDUB_GT_DR_AR_BP_AR, 
		 TOP_GP32_LDUB_GT_DR_AR_QM_AR, 
		 TOP_GP32_LDUB_GT_DR_AR_QP_AR, 
		 TOP_GP32_LDUH_GT_DR_AR_BM_AR, 
		 TOP_GP32_LDUH_GT_DR_AR_BP_AR, 
		 TOP_GP32_LDUH_GT_DR_AR_QM_AR, 
		 TOP_GP32_LDUH_GT_DR_AR_QP_AR, 
		 TOP_GP32_LDUW_GT_DR_AR_BM_AR, 
		 TOP_GP32_LDUW_GT_DR_AR_BP_AR, 
		 TOP_GP32_LDUW_GT_DR_AR_QM_AR, 
		 TOP_GP32_LDUW_GT_DR_AR_QP_AR, 
		 TOP_GP32_LDW_GT_DR_AR_BM_AR, 
		 TOP_GP32_LDW_GT_DR_AR_BP_AR, 
		 TOP_GP32_LDW_GT_DR_AR_QM_AR, 
		 TOP_GP32_LDW_GT_DR_AR_QP_AR, 
		 TOP_UNDEFINED); 

  Result (0, int40); 
  Result (1, ptr32); 
  Same_Res (2); 
  Operand (0, pr, predicate); 
  Operand (1, ptr32, base); 
  Operand (2, ptr32, offset); 

  /* ====================================== */ 
  Instruction_Group("O_108", 
		 TOP_GP32_LAH_GT_MD_AR_AR_BM_AR, 
		 TOP_GP32_LAH_GT_MD_AR_AR_BP_AR, 
		 TOP_GP32_LAH_GT_MD_AR_AR_QM_AR, 
		 TOP_GP32_LAH_GT_MD_AR_AR_QP_AR, 
		 TOP_GP32_LAW_GT_MD_AR_AR_BM_AR, 
		 TOP_GP32_LAW_GT_MD_AR_AR_BP_AR, 
		 TOP_GP32_LAW_GT_MD_AR_AR_QM_AR, 
		 TOP_GP32_LAW_GT_MD_AR_AR_QP_AR, 
		 TOP_UNDEFINED); 

  Result (0, ptr32); 
  Result (1, ptr32); 
  Same_Res (3); 
  Operand (0, pr, predicate); 
  Operand (1, md); 
  Operand (2, ptr32, base); 
  Operand (3, ptr32, offset); 

  /* ====================================== */ 
  Instruction_Group("O_109", 
		 TOP_GP32_LDEW_GT_DR_P13_P_U15, 
		 TOP_GP32_LDHH_GT_DR_P13_P_U15, 
		 TOP_GP32_LDLH_GT_DR_P13_P_U15, 
		 TOP_UNDEFINED); 

  Result (0, int40); 
  Same_Res (1); 
  Operand (0, pr, predicate); 
  Operand (1, int40); 
  Operand (2, p13, base); 
  Operand (3, u15, offset); 

  /* ====================================== */ 
  Instruction_Group("O_110", 
		 TOP_GP32_SCW_GT_AR_M_AR_CRL, 
		 TOP_GP32_SCW_GT_AR_P_AR_CRL, 
		 TOP_UNDEFINED); 

  Operand (0, pr, predicate); 
  Operand (1, ptr32, base); 
  Operand (2, ptr32, offset); 
  Operand (3, ctrll, storeval); 

  /* ====================================== */ 
  Instruction_Group("O_111", 
		 TOP_GP32_LCW_GT_CRL_P13_P_U15, 
		 TOP_UNDEFINED); 

  Result (0, ctrll); 
  Operand (0, pr, predicate); 
  Operand (1, p13, base); 
  Operand (2, u15, offset); 

  /* ====================================== */ 
  Instruction_Group("O_112", 
		 TOP_GP32_LCG_GT_MD_BR_AR_M_U5, 
		 TOP_GP32_LCG_GT_MD_BR_AR_P_U5, 
		 TOP_UNDEFINED); 

  Result (0, pr); 
  Operand (0, pr, predicate); 
  Operand (1, md); 
  Operand (2, ptr32, base); 
  Operand (3, u5, offset); 

  /* ====================================== */ 
  Instruction_Group("O_113", 
		 TOP_GP32_SFR_GT_MD_AR_M_AR, 
		 TOP_GP32_SFR_GT_MD_AR_P_AR, 
		 TOP_UNDEFINED); 

  Operand (0, pr, predicate); 
  Operand (1, md); 
  Operand (2, ptr32, base); 
  Operand (3, ptr32, offset); 
  Operand (3, ptr32, storeval); 
  Operand (4, cr8); 

  /* ====================================== */ 
  Instruction_Group("O_114", 
		 TOP_GP32_SCW_GT_MD_AR_M_U5_CRH, 
		 TOP_GP32_SCW_GT_MD_AR_P_U5_CRH, 
		 TOP_UNDEFINED); 

  Operand (0, pr, predicate); 
  Operand (1, md); 
  Operand (2, ptr32, base); 
  Operand (3, u5, offset); 
  Operand (4, ctrlh, storeval); 

  /* ====================================== */ 
  Instruction_Group("O_115", 
		 TOP_GP32_SGR_GT_MD_AR_M_AR, 
		 TOP_GP32_SGR_GT_MD_AR_P_AR, 
		 TOP_UNDEFINED); 

  Operand (0, pr, predicate); 
  Operand (1, md); 
  Operand (2, ptr32, base); 
  Operand (3, ptr32, offset); 
  Operand (3, ptr32, storeval); 
  Operand (4, cr9); 

  /* ====================================== */ 
  Instruction_Group("O_116", 
		 TOP_GP32_BCLR_GT_DR_DR_U5, 
		 TOP_GP32_BNOT_GT_DR_DR_U5, 
		 TOP_GP32_BSET_GT_DR_DR_U5, 
		 TOP_GP32_SHLCW_GT_DR_DR_U5, 
		 TOP_GP32_SHLU_GT_DR_DR_U5, 
		 TOP_GP32_SHL_GT_DR_DR_U5, 
		 TOP_GP32_SHRUW_GT_DR_DR_U5, 
		 TOP_GP32_SHRU_GT_DR_DR_U5, 
		 TOP_GP32_SHRW_GT_DR_DR_U5, 
		 TOP_GP32_SHR_GT_DR_DR_U5, 
		 TOP_UNDEFINED); 

  Result (0, int40); 
  Operand (0, pr, predicate); 
  Operand (1, int40, opnd1); 
  Operand (2, u5, opnd2); 

  /* ====================================== */ 
  Instruction_Group("O_117", 
		 TOP_GP32_SCW_GT_AR_BM_AR_CRL, 
		 TOP_GP32_SCW_GT_AR_BP_AR_CRL, 
		 TOP_GP32_SCW_GT_AR_QM_AR_CRL, 
		 TOP_GP32_SCW_GT_AR_QP_AR_CRL, 
		 TOP_UNDEFINED); 

  Result (0, ptr32); 
  Same_Res (1); 
  Operand (0, pr, predicate); 
  Operand (1, ptr32, base); 
  Operand (2, ptr32, offset); 
  Operand (3, ctrll, storeval); 

  /* ====================================== */ 
  Instruction_Group("O_118", 
		 TOP_GP32_SCW_GT_P13_P_U15_CRL, 
		 TOP_UNDEFINED); 

  Operand (0, pr, predicate); 
  Operand (1, p13, base); 
  Operand (2, u15, offset); 
  Operand (3, ctrll, storeval); 

  /* ====================================== */ 
  Instruction_Group("O_119", 
		 TOP_GP32_MOVEG_GT_BR_BR, 
		 TOP_GP32_NOTG_GT_BR_BR, 
		 TOP_GP32_NOTPG_GT_BR_BR, 
		 TOP_UNDEFINED); 

  Result (0, pr); 
  Operand (0, pr, predicate); 
  Operand (1, pr, opnd1); 

  /* ====================================== */ 
  Instruction_Group("O_120", 
		 TOP_GP32_FMOVEA_GT_BR_AR_AR, 
		 TOP_GP32_TMOVEA_GT_BR_AR_AR, 
		 TOP_UNDEFINED); 

  Result (0, pr); 
  Result (1, ptr32); 
  Operand (0, pr, predicate); 
  Operand (1, ptr32, opnd1); 

  /* ====================================== */ 
  Instruction_Group("O_121", 
		 TOP_GP32_SAH_GT_AR_BM_U5_AR, 
		 TOP_GP32_SAH_GT_AR_BP_U5_AR, 
		 TOP_GP32_SAH_GT_AR_MQ_U5_AR, 
		 TOP_GP32_SAH_GT_AR_QM_U5_AR, 
		 TOP_GP32_SAH_GT_AR_QP_U5_AR, 
		 TOP_GP32_SAW_GT_AR_BM_U5_AR, 
		 TOP_GP32_SAW_GT_AR_BP_U5_AR, 
		 TOP_GP32_SAW_GT_AR_MQ_U5_AR, 
		 TOP_GP32_SAW_GT_AR_QM_U5_AR, 
		 TOP_GP32_SAW_GT_AR_QP_U5_AR, 
		 TOP_UNDEFINED); 

  Result (0, ptr32); 
  Same_Res (1); 
  Operand (0, pr, predicate); 
  Operand (1, ptr32, base); 
  Operand (2, u5, offset); 
  Operand (3, ptr32, storeval); 

  /* ====================================== */ 
  Instruction_Group("O_122", 
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

  Result (0, pr); 
  Result (1, int40); 
  Operand (0, pr, predicate); 
  Operand (1, int40, opnd1); 
  Operand (2, int40, opnd2); 

  /* ====================================== */ 
  Instruction_Group("O_123", 
		 TOP_GP32_LGR_GT_MD_AR_M_U5, 
		 TOP_GP32_LGR_GT_MD_AR_P_U5, 
		 TOP_UNDEFINED); 

  Result (0, cr9); 
  Operand (0, pr, predicate); 
  Operand (1, md); 
  Operand (2, ptr32, base); 
  Operand (3, u5, offset); 

  /* ====================================== */ 
  Instruction_Group("O_pregtn", 
		 TOP_begin_pregtn, 
		 TOP_end_pregtn, 
		 TOP_UNDEFINED); 

  Operand (0, int40); 
  Operand (1, s16); 

  /* ====================================== */ 
  Instruction_Group("O_124", 
		 TOP_GP32_ADDBA_GT_AR_AR_AR, 
		 TOP_GP32_ADDHA_GT_AR_AR_AR, 
		 TOP_GP32_ADDWA_GT_AR_AR_AR, 
		 TOP_GP32_SUBBA_GT_AR_AR_AR, 
		 TOP_GP32_SUBHA_GT_AR_AR_AR, 
		 TOP_GP32_SUBWA_GT_AR_AR_AR, 
		 TOP_UNDEFINED); 

  Result (0, ptr32); 
  Operand (0, pr, predicate); 
  Operand (1, ptr32, opnd1); 
  Operand (2, ptr32, opnd2); 

  /* ====================================== */ 
  Instruction_Group("O_125", 
		 TOP_GP32_ADDHA_GT_AR_AR_U9, 
		 TOP_UNDEFINED); 

  Result (0, ptr32); 
  Operand (0, pr, predicate); 
  Operand (1, ptr32, opnd1); 
  Operand (2, u10, opnd2); 

  /* ====================================== */ 
  Instruction_Group("O_126", 
		 TOP_GP32_GOTO_GF_S21, 
		 TOP_UNDEFINED); 

  Operand (0, pr, predicate); 
  Operand (1, s21, target); 

  /* ====================================== */ 
  Instruction_Group("O_127", 
		 TOP_GP32_LDBP_GT_DR_AR_BM_U5, 
		 TOP_GP32_LDBP_GT_DR_AR_BP_U5, 
		 TOP_GP32_LDBP_GT_DR_AR_MQ_U5, 
		 TOP_GP32_LDBP_GT_DR_AR_QM_U5, 
		 TOP_GP32_LDBP_GT_DR_AR_QP_U5, 
		 TOP_GP32_LDBSW_GT_DR_AR_BM_U5, 
		 TOP_GP32_LDBSW_GT_DR_AR_BP_U5, 
		 TOP_GP32_LDBSW_GT_DR_AR_MQ_U5, 
		 TOP_GP32_LDBSW_GT_DR_AR_QM_U5, 
		 TOP_GP32_LDBSW_GT_DR_AR_QP_U5, 
		 TOP_GP32_LDB_GT_DR_AR_BM_U5, 
		 TOP_GP32_LDB_GT_DR_AR_BP_U5, 
		 TOP_GP32_LDB_GT_DR_AR_MQ_U5, 
		 TOP_GP32_LDB_GT_DR_AR_QM_U5, 
		 TOP_GP32_LDB_GT_DR_AR_QP_U5, 
		 TOP_GP32_LDF_GT_DR_AR_BM_U5, 
		 TOP_GP32_LDF_GT_DR_AR_BP_U5, 
		 TOP_GP32_LDF_GT_DR_AR_MQ_U5, 
		 TOP_GP32_LDF_GT_DR_AR_QM_U5, 
		 TOP_GP32_LDF_GT_DR_AR_QP_U5, 
		 TOP_GP32_LDHSW_GT_DR_AR_BM_U5, 
		 TOP_GP32_LDHSW_GT_DR_AR_BP_U5, 
		 TOP_GP32_LDHSW_GT_DR_AR_MQ_U5, 
		 TOP_GP32_LDHSW_GT_DR_AR_QM_U5, 
		 TOP_GP32_LDHSW_GT_DR_AR_QP_U5, 
		 TOP_GP32_LDH_GT_DR_AR_QM_U5, 
		 TOP_GP32_LDP_GT_DR_AR_BM_U5, 
		 TOP_GP32_LDP_GT_DR_AR_BP_U5, 
		 TOP_GP32_LDP_GT_DR_AR_MQ_U5, 
		 TOP_GP32_LDP_GT_DR_AR_QM_U5, 
		 TOP_GP32_LDP_GT_DR_AR_QP_U5, 
		 TOP_GP32_LDSETUB_GT_DR_AR_BM_U5, 
		 TOP_GP32_LDSETUB_GT_DR_AR_BP_U5, 
		 TOP_GP32_LDSETUB_GT_DR_AR_MQ_U5, 
		 TOP_GP32_LDSETUB_GT_DR_AR_QM_U5, 
		 TOP_GP32_LDSETUB_GT_DR_AR_QP_U5, 
		 TOP_GP32_LDUBP_GT_DR_AR_BM_U5, 
		 TOP_GP32_LDUBP_GT_DR_AR_BP_U5, 
		 TOP_GP32_LDUBP_GT_DR_AR_MQ_U5, 
		 TOP_GP32_LDUBP_GT_DR_AR_QM_U5, 
		 TOP_GP32_LDUBP_GT_DR_AR_QP_U5, 
		 TOP_GP32_LDUB_GT_DR_AR_BM_U5, 
		 TOP_GP32_LDUB_GT_DR_AR_BP_U5, 
		 TOP_GP32_LDUB_GT_DR_AR_MQ_U5, 
		 TOP_GP32_LDUB_GT_DR_AR_QM_U5, 
		 TOP_GP32_LDUB_GT_DR_AR_QP_U5, 
		 TOP_GP32_LDUH_GT_DR_AR_BM_U5, 
		 TOP_GP32_LDUH_GT_DR_AR_BP_U5, 
		 TOP_GP32_LDUH_GT_DR_AR_MQ_U5, 
		 TOP_GP32_LDUH_GT_DR_AR_QM_U5, 
		 TOP_GP32_LDUH_GT_DR_AR_QP_U5, 
		 TOP_GP32_LDUW_GT_DR_AR_BM_U5, 
		 TOP_GP32_LDUW_GT_DR_AR_BP_U5, 
		 TOP_GP32_LDUW_GT_DR_AR_MQ_U5, 
		 TOP_GP32_LDUW_GT_DR_AR_QM_U5, 
		 TOP_GP32_LDUW_GT_DR_AR_QP_U5, 
		 TOP_UNDEFINED); 

  Result (0, int40); 
  Result (1, ptr32); 
  Same_Res (2); 
  Operand (0, pr, predicate); 
  Operand (1, ptr32, base); 
  Operand (2, u5, offset); 

  /* ====================================== */ 
  Instruction_Group("O_128", 
		 TOP_GP32_LAH_GT_MD_AR_AR_BM_U5, 
		 TOP_GP32_LAH_GT_MD_AR_AR_BP_U5, 
		 TOP_GP32_LAH_GT_MD_AR_AR_QM_U5, 
		 TOP_GP32_LAH_GT_MD_AR_AR_QP_U5, 
		 TOP_GP32_LAW_GT_MD_AR_AR_BM_U5, 
		 TOP_GP32_LAW_GT_MD_AR_AR_BP_U5, 
		 TOP_GP32_LAW_GT_MD_AR_AR_QM_U5, 
		 TOP_GP32_LAW_GT_MD_AR_AR_QP_U5, 
		 TOP_UNDEFINED); 

  Result (0, ptr32); 
  Result (1, ptr32); 
  Same_Res (3); 
  Operand (0, pr, predicate); 
  Operand (1, md); 
  Operand (2, ptr32, base); 
  Operand (3, u5, offset); 

  /* ====================================== */ 
  Instruction_Group("O_129", 
		 TOP_GP32_LCG_GT_BR_P13_P_U15, 
		 TOP_UNDEFINED); 

  Result (0, pr); 
  Operand (0, pr, predicate); 
  Operand (1, p13, base); 
  Operand (2, u15, offset); 

  /* ====================================== */ 
  Instruction_Group("O_130", 
		 TOP_GP32_LCW_GT_MD_CRL_AR_BM_U5, 
		 TOP_GP32_LCW_GT_MD_CRL_AR_BP_U5, 
		 TOP_GP32_LCW_GT_MD_CRL_AR_QM_U5, 
		 TOP_GP32_LCW_GT_MD_CRL_AR_QP_U5, 
		 TOP_UNDEFINED); 

  Result (0, ctrll); 
  Result (1, ptr32); 
  Same_Res (3); 
  Operand (0, pr, predicate); 
  Operand (1, md); 
  Operand (2, ptr32, base); 
  Operand (3, u5, offset); 

  /* ====================================== */ 
  Instruction_Group("O_131", 
		 TOP_GP32_NOTG_GT_BR, 
		 TOP_UNDEFINED); 

  Result (0, pr); 
  Same_Res (1); 
  Operand (0, pr, predicate); 
  Operand (1, pr, opnd1); 

  /* ====================================== */ 
  Instruction_Group("O_132", 
		 TOP_GP32_MAKEK_GT_DR_S40, 
		 TOP_UNDEFINED); 

  Result (0, int40); 
  Operand (0, pr, predicate); 
  Operand (1, s40, opnd1); 

  /* ====================================== */ 
  Instruction_Group("O_133", 
		 TOP_GP32_FEANDN_GT_BR_U8_DR, 
		 TOP_GP32_FEORN_GT_BR_U8_DR, 
		 TOP_GP32_FPANDN_GT_BR_U8_DR, 
		 TOP_GP32_FPORN_GT_BR_U8_DR, 
		 TOP_GP32_FWANDN_GT_BR_U8_DR, 
		 TOP_GP32_FWORN_GT_BR_U8_DR, 
		 TOP_GP32_TEANDN_GT_BR_U8_DR, 
		 TOP_GP32_TEORN_GT_BR_U8_DR, 
		 TOP_GP32_TPANDN_GT_BR_U8_DR, 
		 TOP_GP32_TPORN_GT_BR_U8_DR, 
		 TOP_GP32_TWANDN_GT_BR_U8_DR, 
		 TOP_GP32_TWORN_GT_BR_U8_DR, 
		 TOP_UNDEFINED); 

  Result (0, pr); 
  Operand (0, pr, predicate); 
  Operand (1, u8, opnd1); 
  Operand (2, int40, opnd2); 

  /* ====================================== */ 
  Instruction_Group("O_fixup", 
		 TOP_dfixup, 
		 TOP_ifixup, 
		 TOP_UNDEFINED); 

  Result (0, int40); 

  /* ====================================== */ 
  Instruction_Group("O_134", 
		 TOP_GP32_LCW_GT_CRL_AR_M_AR, 
		 TOP_GP32_LCW_GT_CRL_AR_P_AR, 
		 TOP_UNDEFINED); 

  Result (0, ctrll); 
  Operand (0, pr, predicate); 
  Operand (1, ptr32, base); 
  Operand (2, ptr32, offset); 

  /* ====================================== */ 
  Instruction_Group("O_135", 
		 TOP_GP32_SETILE1_S16, 
		 TOP_GP32_SETLE1_S16, 
		 TOP_GP32_SETLS1_S16, 
		 TOP_GP32_SETULS1_S16, 
		 TOP_UNDEFINED); 

  Result (0, lr1); 
  Operand (0, s16, target); 

  /* ====================================== */ 
  Instruction_Group("O_136", 
		 TOP_GP32_SCW_GT_AR_M_U9_CRL, 
		 TOP_GP32_SCW_GT_AR_P_U9_CRL, 
		 TOP_UNDEFINED); 

  Operand (0, pr, predicate); 
  Operand (1, ptr32, base); 
  Operand (2, u9, offset); 
  Operand (3, ctrll, storeval); 

  /* ====================================== */ 
  Instruction_Group("O_137", 
		 TOP_GP32_SFR_GT_AR_BM_AR, 
		 TOP_GP32_SFR_GT_AR_BP_AR, 
		 TOP_GP32_SFR_GT_AR_QM_AR, 
		 TOP_GP32_SFR_GT_AR_QP_AR, 
		 TOP_UNDEFINED); 

  Result (0, ptr32); 
  Same_Res (1); 
  Operand (0, pr, predicate); 
  Operand (1, ptr32, base); 
  Operand (2, ptr32, offset); 
  Operand (2, ptr32, storeval); 
  Operand (3, cr8); 

  /* ====================================== */ 
  Instruction_Group("O_138", 
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
		 TOP_GP32_XSHLW_GT_DR_DR_DR_DR, 
		 TOP_GP32_XSHRW_GT_DR_DR_DR_DR, 
		 TOP_UNDEFINED); 

  Result (0, int40); 
  Operand (0, pr, predicate); 
  Operand (1, int40, opnd1); 
  Operand (2, int40, opnd2); 
  Operand (3, int40); 

  /* ====================================== */ 
  Instruction_Group("O_139", 
		 TOP_GP32_SGR_GT_AR_BM_AR, 
		 TOP_GP32_SGR_GT_AR_BP_AR, 
		 TOP_GP32_SGR_GT_AR_QM_AR, 
		 TOP_GP32_SGR_GT_AR_QP_AR, 
		 TOP_UNDEFINED); 

  Result (0, ptr32); 
  Same_Res (1); 
  Operand (0, pr, predicate); 
  Operand (1, ptr32, base); 
  Operand (2, ptr32, offset); 
  Operand (2, ptr32, storeval); 
  Operand (3, cr9); 

  /* ====================================== */ 
  Instruction_Group("O_140", 
		 TOP_GP32_SDBP_GT_AR_M_AR_DR, 
		 TOP_GP32_SDBP_GT_AR_P_AR_DR, 
		 TOP_GP32_SDBSW_GT_AR_M_AR_DR, 
		 TOP_GP32_SDBSW_GT_AR_P_AR_DR, 
		 TOP_GP32_SDB_GT_AR_M_AR_DR, 
		 TOP_GP32_SDB_GT_AR_P_AR_DR, 
		 TOP_GP32_SDEW_GT_AR_M_AR_DR, 
		 TOP_GP32_SDEW_GT_AR_P_AR_DR, 
		 TOP_GP32_SDF_GT_AR_M_AR_DR, 
		 TOP_GP32_SDF_GT_AR_P_AR_DR, 
		 TOP_GP32_SDHSW_GT_AR_M_AR_DR, 
		 TOP_GP32_SDHSW_GT_AR_P_AR_DR, 
		 TOP_GP32_SDH_GT_AR_M_AR_DR, 
		 TOP_GP32_SDH_GT_AR_P_AR_DR, 
		 TOP_GP32_SDP_GT_AR_M_AR_DR, 
		 TOP_GP32_SDP_GT_AR_P_AR_DR, 
		 TOP_GP32_SDW_GT_AR_M_AR_DR, 
		 TOP_GP32_SDW_GT_AR_P_AR_DR, 
		 TOP_UNDEFINED); 

  Operand (0, pr, predicate); 
  Operand (1, ptr32, base); 
  Operand (2, ptr32, offset); 
  Operand (3, int40, storeval); 

  /* ====================================== */ 
  Instruction_Group("O_141", 
		 TOP_GP32_EQA_GT_BR_AR_AR, 
		 TOP_GP32_GEA_GT_BR_AR_AR, 
		 TOP_GP32_GTA_GT_BR_AR_AR, 
		 TOP_GP32_LEA_GT_BR_AR_AR, 
		 TOP_GP32_LTA_GT_BR_AR_AR, 
		 TOP_GP32_NEA_GT_BR_AR_AR, 
		 TOP_UNDEFINED); 

  Result (0, pr); 
  Operand (0, pr, predicate); 
  Operand (1, ptr32, opnd1); 
  Operand (2, ptr32, opnd2); 

  /* ====================================== */ 
  Instruction_Group("O_142", 
		 TOP_GP32_FBPOSP_GT_BR_DR_U4, 
		 TOP_GP32_TBPOSP_GT_BR_DR_U4, 
		 TOP_UNDEFINED); 

  Result (0, pr); 
  Operand (0, pr, predicate); 
  Operand (1, int40, opnd1); 
  Operand (2, u4, opnd2); 

  /* ====================================== */ 
  Instruction_Group("O_143", 
		 TOP_GP32_LGR_GT_AR_BM_U5, 
		 TOP_GP32_LGR_GT_AR_BP_U5, 
		 TOP_GP32_LGR_GT_AR_MQ_U5, 
		 TOP_GP32_LGR_GT_AR_QM_U5, 
		 TOP_GP32_LGR_GT_AR_QP_U5, 
		 TOP_UNDEFINED); 

  Result (0, ptr32); 
  Same_Res (1); 
  Result (1, cr9); 
  Operand (0, pr, predicate); 
  Operand (1, ptr32, base); 
  Operand (2, u5, offset); 

  /* ====================================== */ 
  Instruction_Group("O_144", 
		 TOP_GP32_LDW_GT_DR_AR_BM_U5, 
		 TOP_GP32_LDW_GT_DR_AR_BP_U5, 
		 TOP_GP32_LDW_GT_DR_AR_MQ_U5, 
		 TOP_GP32_LDW_GT_DR_AR_QM_U5, 
		 TOP_GP32_LDW_GT_DR_AR_QP_U5, 
		 TOP_UNDEFINED); 

  Result (0, int40); 
  Result (1, ptr32); 
  Same_Res (2); 
  Operand (0, pr, predicate); 
  Operand (1, ptr32, base); 
  Operand (2, u7, offset); 

  /* ====================================== */ 
  Instruction_Group("O_145", 
		 TOP_GP32_SDBP_GT_AR_BM_AR_DR, 
		 TOP_GP32_SDBP_GT_AR_BP_AR_DR, 
		 TOP_GP32_SDBP_GT_AR_QM_AR_DR, 
		 TOP_GP32_SDBP_GT_AR_QP_AR_DR, 
		 TOP_GP32_SDBSW_GT_AR_BM_AR_DR, 
		 TOP_GP32_SDBSW_GT_AR_BP_AR_DR, 
		 TOP_GP32_SDBSW_GT_AR_QM_AR_DR, 
		 TOP_GP32_SDBSW_GT_AR_QP_AR_DR, 
		 TOP_GP32_SDB_GT_AR_BM_AR_DR, 
		 TOP_GP32_SDB_GT_AR_BP_AR_DR, 
		 TOP_GP32_SDB_GT_AR_QM_AR_DR, 
		 TOP_GP32_SDB_GT_AR_QP_AR_DR, 
		 TOP_GP32_SDEW_GT_AR_BM_AR_DR, 
		 TOP_GP32_SDEW_GT_AR_BP_AR_DR, 
		 TOP_GP32_SDEW_GT_AR_QM_AR_DR, 
		 TOP_GP32_SDEW_GT_AR_QP_AR_DR, 
		 TOP_GP32_SDF_GT_AR_BM_AR_DR, 
		 TOP_GP32_SDF_GT_AR_BP_AR_DR, 
		 TOP_GP32_SDF_GT_AR_QM_AR_DR, 
		 TOP_GP32_SDF_GT_AR_QP_AR_DR, 
		 TOP_GP32_SDHSW_GT_AR_BM_AR_DR, 
		 TOP_GP32_SDHSW_GT_AR_BP_AR_DR, 
		 TOP_GP32_SDHSW_GT_AR_QM_AR_DR, 
		 TOP_GP32_SDHSW_GT_AR_QP_AR_DR, 
		 TOP_GP32_SDH_GT_AR_BM_AR_DR, 
		 TOP_GP32_SDH_GT_AR_BP_AR_DR, 
		 TOP_GP32_SDH_GT_AR_QM_AR_DR, 
		 TOP_GP32_SDH_GT_AR_QP_AR_DR, 
		 TOP_GP32_SDP_GT_AR_BM_AR_DR, 
		 TOP_GP32_SDP_GT_AR_BP_AR_DR, 
		 TOP_GP32_SDP_GT_AR_QM_AR_DR, 
		 TOP_GP32_SDP_GT_AR_QP_AR_DR, 
		 TOP_GP32_SDW_GT_AR_BM_AR_DR, 
		 TOP_GP32_SDW_GT_AR_BP_AR_DR, 
		 TOP_GP32_SDW_GT_AR_QM_AR_DR, 
		 TOP_GP32_SDW_GT_AR_QP_AR_DR, 
		 TOP_UNDEFINED); 

  Result (0, ptr32); 
  Same_Res (1); 
  Operand (0, pr, predicate); 
  Operand (1, ptr32, base); 
  Operand (2, ptr32, offset); 
  Operand (3, int40, storeval); 

  /* ====================================== */ 
  Instruction_Group("O_146", 
		 TOP_GP32_SDBP_GT_P13_P_U15_DR, 
		 TOP_GP32_SDBSW_GT_P13_P_U15_DR, 
		 TOP_GP32_SDB_GT_P13_P_U15_DR, 
		 TOP_GP32_SDEW_GT_P13_P_U15_DR, 
		 TOP_GP32_SDF_GT_P13_P_U15_DR, 
		 TOP_GP32_SDHSW_GT_P13_P_U15_DR, 
		 TOP_GP32_SDH_GT_P13_P_U15_DR, 
		 TOP_GP32_SDP_GT_P13_P_U15_DR, 
		 TOP_GP32_SDW_GT_P13_P_U15_DR, 
		 TOP_UNDEFINED); 

  Operand (0, pr, predicate); 
  Operand (1, p13, base); 
  Operand (2, u15, offset); 
  Operand (3, int40, storeval); 

  /* ====================================== */ 
  Instruction_Group("O_147", 
		 TOP_GP32_LGR_GT_AR_M_U9, 
		 TOP_GP32_LGR_GT_AR_P_U9, 
		 TOP_UNDEFINED); 

  Result (0, cr9); 
  Operand (0, pr, predicate); 
  Operand (1, ptr32, base); 
  Operand (2, u9, offset); 

  /* ====================================== */ 
  Instruction_Group("O_148", 
		 TOP_GP32_MAKEC_GT_CRL_P3, 
		 TOP_UNDEFINED); 

  Result (0, ctrll); 
  Operand (0, pr, predicate); 
  Operand (1, p3, implicit); 
  Operand (1, p3, opnd1); 

  /* ====================================== */ 
  Instruction_Group("O_149", 
		 TOP_GP32_SFR_GT_AR_M_U9, 
		 TOP_GP32_SFR_GT_AR_P_U9, 
		 TOP_UNDEFINED); 

  Operand (0, pr, predicate); 
  Operand (1, ptr32, base); 
  Operand (2, u9, offset); 
  Operand (2, u9, storeval); 
  Operand (3, cr8); 

  /* ====================================== */ 
  Instruction_Group("O_150", 
		 TOP_GP32_SGR_GT_AR_M_U9, 
		 TOP_GP32_SGR_GT_AR_P_U9, 
		 TOP_UNDEFINED); 

  Operand (0, pr, predicate); 
  Operand (1, ptr32, base); 
  Operand (2, u9, offset); 
  Operand (2, u9, storeval); 
  Operand (3, cr9); 

  /* ====================================== */ 
  Instruction_Group("O_151", 
		 TOP_GP32_LCW_GT_MD_CRL_AR_M_AR, 
		 TOP_GP32_LCW_GT_MD_CRL_AR_P_AR, 
		 TOP_UNDEFINED); 

  Result (0, ctrll); 
  Operand (0, pr, predicate); 
  Operand (1, md); 
  Operand (2, ptr32, base); 
  Operand (3, ptr32, offset); 

  /* ====================================== */ 
  Instruction_Group("O_152", 
		 TOP_GP32_LCW_GT_MD_CRL_AR_M_U5, 
		 TOP_GP32_LCW_GT_MD_CRL_AR_P_U5, 
		 TOP_UNDEFINED); 

  Result (0, ctrll); 
  Operand (0, pr, predicate); 
  Operand (1, md); 
  Operand (2, ptr32, base); 
  Operand (3, u5, offset); 

  /* ====================================== */ 
  Instruction_Group("O_153", 
		 TOP_GP32_ADDBA_GT_MD_AR_AR_AR, 
		 TOP_GP32_ADDHA_GT_MD_AR_AR_AR, 
		 TOP_GP32_ADDWA_GT_MD_AR_AR_AR, 
		 TOP_GP32_SUBBA_GT_MD_AR_AR_AR, 
		 TOP_GP32_SUBHA_GT_MD_AR_AR_AR, 
		 TOP_GP32_SUBWA_GT_MD_AR_AR_AR, 
		 TOP_UNDEFINED); 

  Result (0, ptr32); 
  Operand (0, pr, predicate); 
  Operand (1, md, opnd1); 
  Operand (2, ptr32, opnd2); 
  Operand (3, ptr32); 

  /* ====================================== */ 
  Instruction_Group("O_154", 
		 TOP_GP32_LDEW_GT_DR_AR_BM_AR, 
		 TOP_GP32_LDEW_GT_DR_AR_BP_AR, 
		 TOP_GP32_LDEW_GT_DR_AR_QM_AR, 
		 TOP_GP32_LDEW_GT_DR_AR_QP_AR, 
		 TOP_GP32_LDHH_GT_DR_AR_BM_AR, 
		 TOP_GP32_LDHH_GT_DR_AR_BP_AR, 
		 TOP_GP32_LDHH_GT_DR_AR_QM_AR, 
		 TOP_GP32_LDHH_GT_DR_AR_QP_AR, 
		 TOP_GP32_LDLH_GT_DR_AR_BM_AR, 
		 TOP_GP32_LDLH_GT_DR_AR_BP_AR, 
		 TOP_GP32_LDLH_GT_DR_AR_QM_AR, 
		 TOP_GP32_LDLH_GT_DR_AR_QP_AR, 
		 TOP_UNDEFINED); 

  Result (0, int40); 
  Same_Res (1); 
  Result (1, ptr32); 
  Same_Res (2); 
  Operand (0, pr, predicate); 
  Operand (1, int40); 
  Operand (2, ptr32, base); 
  Operand (3, ptr32, offset); 

  /* ====================================== */ 
  Instruction_Group("O_155", 
		 TOP_GP32_SDBP_GT_AR_M_U9_DR, 
		 TOP_GP32_SDBP_GT_AR_P_U9_DR, 
		 TOP_GP32_SDBSW_GT_AR_M_U9_DR, 
		 TOP_GP32_SDBSW_GT_AR_P_U9_DR, 
		 TOP_GP32_SDB_GT_AR_M_U9_DR, 
		 TOP_GP32_SDB_GT_AR_P_U9_DR, 
		 TOP_GP32_SDEW_GT_AR_M_U9_DR, 
		 TOP_GP32_SDEW_GT_AR_P_U9_DR, 
		 TOP_GP32_SDF_GT_AR_M_U9_DR, 
		 TOP_GP32_SDF_GT_AR_P_U9_DR, 
		 TOP_GP32_SDHSW_GT_AR_M_U9_DR, 
		 TOP_GP32_SDHSW_GT_AR_P_U9_DR, 
		 TOP_GP32_SDH_GT_AR_M_U9_DR, 
		 TOP_GP32_SDH_GT_AR_P_U9_DR, 
		 TOP_GP32_SDP_GT_AR_M_U9_DR, 
		 TOP_GP32_SDP_GT_AR_P_U9_DR, 
		 TOP_GP32_SDW_GT_AR_M_U9_DR, 
		 TOP_GP32_SDW_GT_AR_P_U9_DR, 
		 TOP_UNDEFINED); 

  Operand (0, pr, predicate); 
  Operand (1, ptr32, base); 
  Operand (2, u9, offset); 
  Operand (3, int40, storeval); 

  /* ====================================== */ 
  Instruction_Group("O_156", 
		 TOP_GP32_SAH_GT_MD_AR_BM_AR_AR, 
		 TOP_GP32_SAH_GT_MD_AR_BP_AR_AR, 
		 TOP_GP32_SAH_GT_MD_AR_QM_AR_AR, 
		 TOP_GP32_SAH_GT_MD_AR_QP_AR_AR, 
		 TOP_GP32_SAW_GT_MD_AR_BM_AR_AR, 
		 TOP_GP32_SAW_GT_MD_AR_BP_AR_AR, 
		 TOP_GP32_SAW_GT_MD_AR_QM_AR_AR, 
		 TOP_GP32_SAW_GT_MD_AR_QP_AR_AR, 
		 TOP_UNDEFINED); 

  Result (0, ptr32); 
  Same_Res (2); 
  Operand (0, pr, predicate); 
  Operand (1, md); 
  Operand (2, ptr32, base); 
  Operand (3, ptr32, offset); 
  Operand (4, ptr32, storeval); 

  /* ====================================== */ 
  Instruction_Group("O_157", 
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

  Result (0, pr); 
  Result (1, cr8); 
  Operand (0, pr, predicate); 
  Operand (1, cr8, opnd1); 

  /* ====================================== */ 
  Instruction_Group("O_158", 
		 TOP_GP32_LCG_GT_MD_BR_AR_BM_U5, 
		 TOP_GP32_LCG_GT_MD_BR_AR_BP_U5, 
		 TOP_GP32_LCG_GT_MD_BR_AR_QM_U5, 
		 TOP_GP32_LCG_GT_MD_BR_AR_QP_U5, 
		 TOP_UNDEFINED); 

  Result (0, pr); 
  Result (1, ptr32); 
  Same_Res (3); 
  Operand (0, pr, predicate); 
  Operand (1, md); 
  Operand (2, ptr32, base); 
  Operand (3, u5, offset); 

  /* ====================================== */ 
  Instruction_Group("O_159", 
		 TOP_GP32_LFR_GT_AR_BM_AR, 
		 TOP_GP32_LFR_GT_AR_BP_AR, 
		 TOP_GP32_LFR_GT_AR_QM_AR, 
		 TOP_GP32_LFR_GT_AR_QP_AR, 
		 TOP_UNDEFINED); 

  Result (0, ptr32); 
  Same_Res (1); 
  Result (1, cr8); 
  Operand (0, pr, predicate); 
  Operand (1, ptr32, base); 
  Operand (2, ptr32, offset); 

  /* ====================================== */ 
  Instruction_Group("O_160", 
		 TOP_GP32_SAH_GT_MD_AR_M_AR_AR, 
		 TOP_GP32_SAH_GT_MD_AR_P_AR_AR, 
		 TOP_GP32_SAW_GT_MD_AR_M_AR_AR, 
		 TOP_GP32_SAW_GT_MD_AR_P_AR_AR, 
		 TOP_UNDEFINED); 

  Operand (0, pr, predicate); 
  Operand (1, md); 
  Operand (2, ptr32, base); 
  Operand (3, ptr32, offset); 
  Operand (4, ptr32, storeval); 

  /* ====================================== */ 
  Instruction_Group("O_161", 
		 TOP_GP32_SAH_GT_MD_AR_BM_U5_AR, 
		 TOP_GP32_SAH_GT_MD_AR_BP_U5_AR, 
		 TOP_GP32_SAH_GT_MD_AR_QM_U5_AR, 
		 TOP_GP32_SAH_GT_MD_AR_QP_U5_AR, 
		 TOP_GP32_SAW_GT_MD_AR_BM_U5_AR, 
		 TOP_GP32_SAW_GT_MD_AR_BP_U5_AR, 
		 TOP_GP32_SAW_GT_MD_AR_QM_U5_AR, 
		 TOP_GP32_SAW_GT_MD_AR_QP_U5_AR, 
		 TOP_UNDEFINED); 

  Result (0, ptr32); 
  Same_Res (2); 
  Operand (0, pr, predicate); 
  Operand (1, md); 
  Operand (2, ptr32, base); 
  Operand (3, u5, offset); 
  Operand (4, ptr32, storeval); 

  /* ====================================== */ 
  Instruction_Group("O_162", 
		 TOP_GP32_ADDBA_GT_AR_P13_U15, 
		 TOP_GP32_ADDHA_GT_AR_P13_U15, 
		 TOP_GP32_ADDWA_GT_AR_P13_U15, 
		 TOP_UNDEFINED); 

  Result (0, ptr32); 
  Operand (0, pr, predicate); 
  Operand (1, p13, opnd1); 
  Operand (2, u15, opnd2); 

  /* ====================================== */ 
  Instruction_Group("O_163", 
		 TOP_GP32_FBCLRP_GT_BR_DR_DR_U4, 
		 TOP_GP32_FBNOTP_GT_BR_DR_DR_U4, 
		 TOP_GP32_FBSETP_GT_BR_DR_DR_U4, 
		 TOP_GP32_TBCLRP_GT_BR_DR_DR_U4, 
		 TOP_GP32_TBNOTP_GT_BR_DR_DR_U4, 
		 TOP_GP32_TBSETP_GT_BR_DR_DR_U4, 
		 TOP_UNDEFINED); 

  Result (0, pr); 
  Result (1, int40); 
  Operand (0, pr, predicate); 
  Operand (1, int40, opnd1); 
  Operand (2, u4, opnd2); 

  /* ====================================== */ 
  Instruction_Group("O_164", 
		 TOP_GP32_LCW_GT_CRH_P13_P_U15, 
		 TOP_UNDEFINED); 

  Result (0, ctrlh); 
  Same_Res (1); 
  Operand (0, pr, predicate); 
  Operand (1, ctrlh); 
  Operand (2, p13, base); 
  Operand (3, u15, offset); 

  /* ====================================== */ 
  Instruction_Group("O_165", 
		 TOP_GP32_CLAMPW_GT_DR_DR, 
		 TOP_GP32_EXTB_GT_DR_DR, 
		 TOP_GP32_EXTH_GT_DR_DR, 
		 TOP_GP32_EXTUB_GT_DR_DR, 
		 TOP_GP32_EXTUH_GT_DR_DR, 
		 TOP_GP32_EXTUW_GT_DR_DR, 
		 TOP_GP32_EXTW_GT_DR_DR, 
		 TOP_GP32_LOCW_GT_DR_DR, 
		 TOP_GP32_LZCW_GT_DR_DR, 
		 TOP_GP32_MOVEP_GT_DR_DR, 
		 TOP_GP32_MOVE_GT_DR_DR, 
		 TOP_GP32_NEGCP_GT_DR_DR, 
		 TOP_GP32_NEGCW_GT_DR_DR, 
		 TOP_GP32_NEGP_GT_DR_DR, 
		 TOP_GP32_NEGUP_GT_DR_DR, 
		 TOP_GP32_NEGU_GT_DR_DR, 
		 TOP_GP32_NEG_GT_DR_DR, 
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
		 TOP_GP32_SHRU32_GT_DR_DR, 
		 TOP_GP32_SHRUWM_GT_DR_DR, 
		 TOP_UNDEFINED); 

  Result (0, int40); 
  Operand (0, pr, predicate); 
  Operand (1, int40, opnd1); 

  /* ====================================== */ 
  Instruction_Group("O_166", 
		 TOP_GP32_SCW_GT_AR_BM_U5_CRL, 
		 TOP_GP32_SCW_GT_AR_BP_U5_CRL, 
		 TOP_GP32_SCW_GT_AR_MQ_U5_CRL, 
		 TOP_GP32_SCW_GT_AR_QM_U5_CRL, 
		 TOP_GP32_SCW_GT_AR_QP_U5_CRL, 
		 TOP_UNDEFINED); 

  Result (0, ptr32); 
  Same_Res (1); 
  Operand (0, pr, predicate); 
  Operand (1, ptr32, base); 
  Operand (2, u5, offset); 
  Operand (3, ctrll, storeval); 

  /* ====================================== */ 
  Instruction_Group("O_167", 
		 TOP_GP32_LCW_GT_CRL_AR_M_U9, 
		 TOP_GP32_LCW_GT_CRL_AR_P_U9, 
		 TOP_UNDEFINED); 

  Result (0, ctrll); 
  Operand (0, pr, predicate); 
  Operand (1, ptr32, base); 
  Operand (2, u9, offset); 

  /* ====================================== */ 
  Instruction_Group("O_168", 
		 TOP_GP32_LFR_GT_AR_M_AR, 
		 TOP_GP32_LFR_GT_AR_P_AR, 
		 TOP_UNDEFINED); 

  Result (0, cr8); 
  Operand (0, pr, predicate); 
  Operand (1, ptr32, base); 
  Operand (2, ptr32, offset); 

  /* ====================================== */ 
  Instruction_Group("O_169", 
		 TOP_GP32_LCW_GT_CRH_AR_M_U9, 
		 TOP_GP32_LCW_GT_CRH_AR_P_U9, 
		 TOP_UNDEFINED); 

  Result (0, ctrlh); 
  Same_Res (1); 
  Operand (0, pr, predicate); 
  Operand (1, ctrlh); 
  Operand (2, ptr32, base); 
  Operand (3, u9, offset); 

  /* ====================================== */ 
  Instruction_Group("O_170", 
		 TOP_GP32_LCW_GT_MD_CRL_AR_BM_AR, 
		 TOP_GP32_LCW_GT_MD_CRL_AR_BP_AR, 
		 TOP_GP32_LCW_GT_MD_CRL_AR_QM_AR, 
		 TOP_GP32_LCW_GT_MD_CRL_AR_QP_AR, 
		 TOP_UNDEFINED); 

  Result (0, ctrll); 
  Result (1, ptr32); 
  Same_Res (3); 
  Operand (0, pr, predicate); 
  Operand (1, md); 
  Operand (2, ptr32, base); 
  Operand (3, ptr32, offset); 

  /* ====================================== */ 
  Instruction_Group("O_171", 
		 TOP_GP32_SFR_GT_MD_AR_BM_AR, 
		 TOP_GP32_SFR_GT_MD_AR_BP_AR, 
		 TOP_GP32_SFR_GT_MD_AR_QM_AR, 
		 TOP_GP32_SFR_GT_MD_AR_QP_AR, 
		 TOP_UNDEFINED); 

  Result (0, ptr32); 
  Same_Res (2); 
  Operand (0, pr, predicate); 
  Operand (1, md); 
  Operand (2, ptr32, base); 
  Operand (3, ptr32, offset); 
  Operand (3, ptr32, storeval); 
  Operand (4, cr8); 

  /* ====================================== */ 
  Instruction_Group("O_172", 
		 TOP_GP32_LDEW_GT_MD_DR_AR_M_U5, 
		 TOP_GP32_LDEW_GT_MD_DR_AR_P_U5, 
		 TOP_GP32_LDHH_GT_MD_DR_AR_M_U5, 
		 TOP_GP32_LDHH_GT_MD_DR_AR_P_U5, 
		 TOP_GP32_LDLH_GT_MD_DR_AR_M_U5, 
		 TOP_GP32_LDLH_GT_MD_DR_AR_P_U5, 
		 TOP_UNDEFINED); 

  Result (0, int40); 
  Same_Res (2); 
  Operand (0, pr, predicate); 
  Operand (1, md); 
  Operand (2, int40); 
  Operand (3, ptr32, base); 
  Operand (4, u5, offset); 

  /* ====================================== */ 
  Instruction_Group("O_173", 
		 TOP_GP32_SGR_GT_MD_AR_BM_AR, 
		 TOP_GP32_SGR_GT_MD_AR_BP_AR, 
		 TOP_GP32_SGR_GT_MD_AR_QM_AR, 
		 TOP_GP32_SGR_GT_MD_AR_QP_AR, 
		 TOP_UNDEFINED); 

  Result (0, ptr32); 
  Same_Res (2); 
  Operand (0, pr, predicate); 
  Operand (1, md); 
  Operand (2, ptr32, base); 
  Operand (3, ptr32, offset); 
  Operand (3, ptr32, storeval); 
  Operand (4, cr9); 

  /* ====================================== */ 
  Instruction_Group("O_174", 
		 TOP_GP32_MAKEPR_GT_S21, 
		 TOP_UNDEFINED); 

  Result (0, p3); 
  Operand (0, pr, predicate); 
  Operand (1, s21, opnd1); 

  /* ====================================== */ 
  Instruction_Group("O_175", 
		 TOP_GP32_LCW_GT_MD_CRH_AR_M_AR, 
		 TOP_GP32_LCW_GT_MD_CRH_AR_P_AR, 
		 TOP_UNDEFINED); 

  Result (0, ctrlh); 
  Same_Res (2); 
  Operand (0, pr, predicate); 
  Operand (1, md); 
  Operand (2, ctrlh); 
  Operand (3, ptr32, base); 
  Operand (4, ptr32, offset); 

  /* ====================================== */ 
  Instruction_Group("O_176", 
		 TOP_GP32_LCW_GT_CRL_AR_BM_U5, 
		 TOP_GP32_LCW_GT_CRL_AR_BP_U5, 
		 TOP_GP32_LCW_GT_CRL_AR_MQ_U5, 
		 TOP_GP32_LCW_GT_CRL_AR_QM_U5, 
		 TOP_GP32_LCW_GT_CRL_AR_QP_U5, 
		 TOP_UNDEFINED); 

  Result (0, ctrll); 
  Result (1, ptr32); 
  Same_Res (2); 
  Operand (0, pr, predicate); 
  Operand (1, ptr32, base); 
  Operand (2, u5, offset); 

  /* ====================================== */ 
  Instruction_Group("O_177", 
		 TOP_GP32_LDBP_GT_MD_DR_AR_M_AR, 
		 TOP_GP32_LDBP_GT_MD_DR_AR_P_AR, 
		 TOP_GP32_LDBSW_GT_MD_DR_AR_M_AR, 
		 TOP_GP32_LDBSW_GT_MD_DR_AR_P_AR, 
		 TOP_GP32_LDB_GT_MD_DR_AR_M_AR, 
		 TOP_GP32_LDB_GT_MD_DR_AR_P_AR, 
		 TOP_GP32_LDF_GT_MD_DR_AR_M_AR, 
		 TOP_GP32_LDF_GT_MD_DR_AR_P_AR, 
		 TOP_GP32_LDHSW_GT_MD_DR_AR_M_AR, 
		 TOP_GP32_LDHSW_GT_MD_DR_AR_P_AR, 
		 TOP_GP32_LDH_GT_MD_DR_AR_M_AR, 
		 TOP_GP32_LDH_GT_MD_DR_AR_P_AR, 
		 TOP_GP32_LDP_GT_MD_DR_AR_M_AR, 
		 TOP_GP32_LDP_GT_MD_DR_AR_P_AR, 
		 TOP_GP32_LDSETUB_GT_MD_DR_AR_M_AR, 
		 TOP_GP32_LDSETUB_GT_MD_DR_AR_P_AR, 
		 TOP_GP32_LDUBP_GT_MD_DR_AR_M_AR, 
		 TOP_GP32_LDUBP_GT_MD_DR_AR_P_AR, 
		 TOP_GP32_LDUB_GT_MD_DR_AR_M_AR, 
		 TOP_GP32_LDUB_GT_MD_DR_AR_P_AR, 
		 TOP_GP32_LDUH_GT_MD_DR_AR_M_AR, 
		 TOP_GP32_LDUH_GT_MD_DR_AR_P_AR, 
		 TOP_GP32_LDUW_GT_MD_DR_AR_M_AR, 
		 TOP_GP32_LDUW_GT_MD_DR_AR_P_AR, 
		 TOP_GP32_LDW_GT_MD_DR_AR_M_AR, 
		 TOP_GP32_LDW_GT_MD_DR_AR_P_AR, 
		 TOP_UNDEFINED); 

  Result (0, int40); 
  Operand (0, pr, predicate); 
  Operand (1, md); 
  Operand (2, ptr32, base); 
  Operand (3, ptr32, offset); 

  /* ====================================== */ 
  Instruction_Group("O_178", 
		 TOP_GP32_LAH_GT_AR_AR_BM_AR, 
		 TOP_GP32_LAH_GT_AR_AR_BP_AR, 
		 TOP_GP32_LAH_GT_AR_AR_QM_AR, 
		 TOP_GP32_LAH_GT_AR_AR_QP_AR, 
		 TOP_GP32_LAW_GT_AR_AR_BM_AR, 
		 TOP_GP32_LAW_GT_AR_AR_BP_AR, 
		 TOP_GP32_LAW_GT_AR_AR_QM_AR, 
		 TOP_GP32_LAW_GT_AR_AR_QP_AR, 
		 TOP_UNDEFINED); 

  Result (0, ptr32); 
  Result (1, ptr32); 
  Same_Res (2); 
  Operand (0, pr, predicate); 
  Operand (1, ptr32, base); 
  Operand (2, ptr32, offset); 

  /* ====================================== */ 
  Instruction_Group("O_179", 
		 TOP_GP32_GOTO_S25, 
		 TOP_UNDEFINED); 

  Operand (0, s25, target); 

  /* ====================================== */ 
  Instruction_Group("O_180", 
		 TOP_GP32_LCW_GT_CRL_AR_BM_AR, 
		 TOP_GP32_LCW_GT_CRL_AR_BP_AR, 
		 TOP_GP32_LCW_GT_CRL_AR_QM_AR, 
		 TOP_GP32_LCW_GT_CRL_AR_QP_AR, 
		 TOP_UNDEFINED); 

  Result (0, ctrll); 
  Result (1, ptr32); 
  Same_Res (2); 
  Operand (0, pr, predicate); 
  Operand (1, ptr32, base); 
  Operand (2, ptr32, offset); 

  /* ====================================== */ 
  Instruction_Group("O_181", 
		 TOP_GP32_ANDNP_GT_DR_U8_DR, 
		 TOP_GP32_ANDN_GT_DR_U8_DR, 
		 TOP_GP32_ORNP_GT_DR_U8_DR, 
		 TOP_GP32_ORN_GT_DR_U8_DR, 
		 TOP_UNDEFINED); 

  Result (0, int40); 
  Operand (0, pr, predicate); 
  Operand (1, u8, opnd1); 
  Operand (2, int40, opnd2); 

  /* ====================================== */ 
  Instruction_Group("O_182", 
		 TOP_GP32_LDH_GT_DR_AR_M_U9, 
		 TOP_GP32_LDH_GT_DR_AR_P_U9, 
		 TOP_UNDEFINED); 

  Result (0, int40); 
  Operand (0, pr, predicate); 
  Operand (1, ptr32, base); 
  Operand (2, u10, offset); 

  /* ====================================== */ 
  Instruction_Group("O_183", 
		 TOP_GP32_SCW_GT_AR_M_AR_CRH, 
		 TOP_GP32_SCW_GT_AR_P_AR_CRH, 
		 TOP_UNDEFINED); 

  Operand (0, pr, predicate); 
  Operand (1, ptr32, base); 
  Operand (2, ptr32, offset); 
  Operand (3, ctrlh, storeval); 

  /* ====================================== */ 
  Instruction_Group("O_184", 
		 TOP_GP32_LFR_GT_MD_AR_BM_AR, 
		 TOP_GP32_LFR_GT_MD_AR_BP_AR, 
		 TOP_GP32_LFR_GT_MD_AR_QM_AR, 
		 TOP_GP32_LFR_GT_MD_AR_QP_AR, 
		 TOP_UNDEFINED); 

  Result (0, ptr32); 
  Same_Res (2); 
  Result (1, cr8); 
  Operand (0, pr, predicate); 
  Operand (1, md); 
  Operand (2, ptr32, base); 
  Operand (3, ptr32, offset); 

  /* ====================================== */ 
  Instruction_Group("O_185", 
		 TOP_GP32_SCW_GT_AR_BM_AR_CRH, 
		 TOP_GP32_SCW_GT_AR_BP_AR_CRH, 
		 TOP_GP32_SCW_GT_AR_QM_AR_CRH, 
		 TOP_GP32_SCW_GT_AR_QP_AR_CRH, 
		 TOP_UNDEFINED); 

  Result (0, ptr32); 
  Same_Res (1); 
  Operand (0, pr, predicate); 
  Operand (1, ptr32, base); 
  Operand (2, ptr32, offset); 
  Operand (3, ctrlh, storeval); 

  /* ====================================== */ 
  Instruction_Group("O_186", 
		 TOP_GP32_BCLRP_GT_DR_DR_U4, 
		 TOP_GP32_BNOTP_GT_DR_DR_U4, 
		 TOP_GP32_BSETP_GT_DR_DR_U4, 
		 TOP_UNDEFINED); 

  Result (0, int40); 
  Operand (0, pr, predicate); 
  Operand (1, int40, opnd1); 
  Operand (2, u4, opnd2); 

  /* ====================================== */ 
  Instruction_Group("O_187", 
		 TOP_GP32_SCW_GT_P13_P_U15_CRH, 
		 TOP_UNDEFINED); 

  Operand (0, pr, predicate); 
  Operand (1, p13, base); 
  Operand (2, u15, offset); 
  Operand (3, ctrlh, storeval); 

  /* ====================================== */ 
  Instruction_Group("O_188", 
		 TOP_GP32_SFR_GT_MD_AR_BM_U5, 
		 TOP_GP32_SFR_GT_MD_AR_BP_U5, 
		 TOP_GP32_SFR_GT_MD_AR_QM_U5, 
		 TOP_GP32_SFR_GT_MD_AR_QP_U5, 
		 TOP_UNDEFINED); 

  Result (0, ptr32); 
  Same_Res (2); 
  Operand (0, pr, predicate); 
  Operand (1, md); 
  Operand (2, ptr32, base); 
  Operand (3, u5, offset); 
  Operand (3, u5, storeval); 
  Operand (4, cr8); 

  /* ====================================== */ 
  Instruction_Group("O_189", 
		 TOP_GP32_LAH_GT_AR_P13_P_U15, 
		 TOP_GP32_LAW_GT_AR_P13_P_U15, 
		 TOP_UNDEFINED); 

  Result (0, ptr32); 
  Operand (0, pr, predicate); 
  Operand (1, p13, base); 
  Operand (2, u15, offset); 

  /* ====================================== */ 
  Instruction_Group("O_190", 
		 TOP_GP32_SGR_GT_MD_AR_BM_U5, 
		 TOP_GP32_SGR_GT_MD_AR_BP_U5, 
		 TOP_GP32_SGR_GT_MD_AR_QM_U5, 
		 TOP_GP32_SGR_GT_MD_AR_QP_U5, 
		 TOP_UNDEFINED); 

  Result (0, ptr32); 
  Same_Res (2); 
  Operand (0, pr, predicate); 
  Operand (1, md); 
  Operand (2, ptr32, base); 
  Operand (3, u5, offset); 
  Operand (3, u5, storeval); 
  Operand (4, cr9); 

  /* ====================================== */ 
  Instruction_Group("O_191", 
		 TOP_GP32_BKP_GF, 
		 TOP_GP32_BKP_GT, 
		 TOP_UNDEFINED); 

  Operand (0, pr, predicate); 

  /* ====================================== */ 
  Instruction_Group("O_192", 
		 TOP_GP32_LFR_GT_MD_AR_M_U5, 
		 TOP_GP32_LFR_GT_MD_AR_P_U5, 
		 TOP_UNDEFINED); 

  Result (0, cr8); 
  Operand (0, pr, predicate); 
  Operand (1, md); 
  Operand (2, ptr32, base); 
  Operand (3, u5, offset); 

  /* ====================================== */ 
  Instruction_Group("O_193", 
		 TOP_GP32_POPRTE_U20, 
		 TOP_GP32_POPRTS_U20, 
		 TOP_GP32_POP_U20, 
		 TOP_GP32_PUSH_U20, 
		 TOP_UNDEFINED); 

  Operand (0, u20); 

  /* ====================================== */ 
  Instruction_Group("O_194", 
		 TOP_GP32_GOTOPR_U16, 
		 TOP_UNDEFINED); 

  Operand (0, u16, target); 
  Operand (1, p3); 

  /* ====================================== */ 
  Instruction_Group("O_195", 
		 TOP_GP32_LGR_GT_MD_AR_BM_U5, 
		 TOP_GP32_LGR_GT_MD_AR_BP_U5, 
		 TOP_GP32_LGR_GT_MD_AR_QM_U5, 
		 TOP_GP32_LGR_GT_MD_AR_QP_U5, 
		 TOP_UNDEFINED); 

  Result (0, ptr32); 
  Same_Res (2); 
  Result (1, cr9); 
  Operand (0, pr, predicate); 
  Operand (1, md); 
  Operand (2, ptr32, base); 
  Operand (3, u5, offset); 

  /* ====================================== */ 
  Instruction_Group("O_196", 
		 TOP_GP32_SAH_GT_MD_AR_M_U5_AR, 
		 TOP_GP32_SAH_GT_MD_AR_P_U5_AR, 
		 TOP_GP32_SAW_GT_MD_AR_M_U5_AR, 
		 TOP_GP32_SAW_GT_MD_AR_P_U5_AR, 
		 TOP_UNDEFINED); 

  Operand (0, pr, predicate); 
  Operand (1, md); 
  Operand (2, ptr32, base); 
  Operand (3, u5, offset); 
  Operand (4, ptr32, storeval); 

  /* ====================================== */ 
  Instruction_Group("O_197", 
		 TOP_GP32_MAKEF_GT_DR_S16, 
		 TOP_GP32_MAKEP_GT_DR_S16, 
		 TOP_GP32_MAKE_GT_DR_S16, 
		 TOP_UNDEFINED); 

  Result (0, int40); 
  Operand (0, pr, predicate); 
  Operand (1, s16, opnd1); 

  /* ====================================== */ 
  Instruction_Group("O_198", 
		 TOP_GP32_COPYD_GT_DR_AR, 
		 TOP_UNDEFINED); 

  Result (0, int40); 
  Operand (0, pr, predicate); 
  Operand (1, ptr32, opnd1); 

  /* ====================================== */ 
  Instruction_Group("O_199", 
		 TOP_GP32_LGR_GT_MD_AR_BM_AR, 
		 TOP_GP32_LGR_GT_MD_AR_BP_AR, 
		 TOP_GP32_LGR_GT_MD_AR_QM_AR, 
		 TOP_GP32_LGR_GT_MD_AR_QP_AR, 
		 TOP_UNDEFINED); 

  Result (0, ptr32); 
  Same_Res (2); 
  Result (1, cr9); 
  Operand (0, pr, predicate); 
  Operand (1, md); 
  Operand (2, ptr32, base); 
  Operand (3, ptr32, offset); 

  /* ====================================== */ 
  Instruction_Group("O_200", 
		 TOP_GP32_SDBP_GT_AR_BM_U5_DR, 
		 TOP_GP32_SDBP_GT_AR_BP_U5_DR, 
		 TOP_GP32_SDBP_GT_AR_MQ_U5_DR, 
		 TOP_GP32_SDBP_GT_AR_QM_U5_DR, 
		 TOP_GP32_SDBP_GT_AR_QP_U5_DR, 
		 TOP_GP32_SDBSW_GT_AR_BM_U5_DR, 
		 TOP_GP32_SDBSW_GT_AR_BP_U5_DR, 
		 TOP_GP32_SDBSW_GT_AR_MQ_U5_DR, 
		 TOP_GP32_SDBSW_GT_AR_QM_U5_DR, 
		 TOP_GP32_SDBSW_GT_AR_QP_U5_DR, 
		 TOP_GP32_SDB_GT_AR_BM_U5_DR, 
		 TOP_GP32_SDB_GT_AR_BP_U5_DR, 
		 TOP_GP32_SDB_GT_AR_MQ_U5_DR, 
		 TOP_GP32_SDB_GT_AR_QM_U5_DR, 
		 TOP_GP32_SDB_GT_AR_QP_U5_DR, 
		 TOP_GP32_SDEW_GT_AR_BM_U5_DR, 
		 TOP_GP32_SDEW_GT_AR_BP_U5_DR, 
		 TOP_GP32_SDEW_GT_AR_MQ_U5_DR, 
		 TOP_GP32_SDEW_GT_AR_QM_U5_DR, 
		 TOP_GP32_SDEW_GT_AR_QP_U5_DR, 
		 TOP_GP32_SDF_GT_AR_BM_U5_DR, 
		 TOP_GP32_SDF_GT_AR_BP_U5_DR, 
		 TOP_GP32_SDF_GT_AR_MQ_U5_DR, 
		 TOP_GP32_SDF_GT_AR_QM_U5_DR, 
		 TOP_GP32_SDF_GT_AR_QP_U5_DR, 
		 TOP_GP32_SDHSW_GT_AR_BM_U5_DR, 
		 TOP_GP32_SDHSW_GT_AR_BP_U5_DR, 
		 TOP_GP32_SDHSW_GT_AR_MQ_U5_DR, 
		 TOP_GP32_SDHSW_GT_AR_QM_U5_DR, 
		 TOP_GP32_SDHSW_GT_AR_QP_U5_DR, 
		 TOP_GP32_SDH_GT_AR_BM_U5_DR, 
		 TOP_GP32_SDH_GT_AR_BP_U5_DR, 
		 TOP_GP32_SDH_GT_AR_MQ_U5_DR, 
		 TOP_GP32_SDH_GT_AR_QM_U5_DR, 
		 TOP_GP32_SDH_GT_AR_QP_U5_DR, 
		 TOP_GP32_SDP_GT_AR_BM_U5_DR, 
		 TOP_GP32_SDP_GT_AR_BP_U5_DR, 
		 TOP_GP32_SDP_GT_AR_MQ_U5_DR, 
		 TOP_GP32_SDP_GT_AR_QM_U5_DR, 
		 TOP_GP32_SDP_GT_AR_QP_U5_DR, 
		 TOP_GP32_SDW_GT_AR_BM_U5_DR, 
		 TOP_GP32_SDW_GT_AR_BP_U5_DR, 
		 TOP_GP32_SDW_GT_AR_MQ_U5_DR, 
		 TOP_GP32_SDW_GT_AR_QM_U5_DR, 
		 TOP_GP32_SDW_GT_AR_QP_U5_DR, 
		 TOP_UNDEFINED); 

  Result (0, ptr32); 
  Same_Res (1); 
  Operand (0, pr, predicate); 
  Operand (1, ptr32, base); 
  Operand (2, u5, offset); 
  Operand (3, int40, storeval); 

  /* ====================================== */ 
  Instruction_Group("O_201", 
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
		 TOP_GP32_NEEINS_GT_BR_DR_DR_DR, 
		 TOP_GP32_NEPINS_GT_BR_DR_DR_DR, 
		 TOP_GP32_NEUEINS_GT_BR_DR_DR_DR, 
		 TOP_GP32_NEUPINS_GT_BR_DR_DR_DR, 
		 TOP_GP32_NEUWINS_GT_BR_DR_DR_DR, 
		 TOP_GP32_NEWINS_GT_BR_DR_DR_DR, 
		 TOP_UNDEFINED); 

  Result (0, pr); 
  Result (1, int40); 
  Same_Res (2); 
  Operand (0, pr, predicate); 
  Operand (1, int40, opnd1); 
  Operand (2, int40, opnd2); 
  Operand (3, int40); 

  /* ====================================== */ 
  Instruction_Group("O_GP32_LINK_GT", 
		 TOP_GP32_LINK_GT, 
		 TOP_UNDEFINED); 

  Operand (0, pr, predicate); 
  Operand (1, p3, implicit); 
  Operand (1, p3, target); 

  /* ====================================== */ 
  Instruction_Group("O_202", 
		 TOP_GP32_SETP15U_GT_AR, 
		 TOP_UNDEFINED); 

  Result (0, p15); 
  Operand (0, pr, predicate); 
  Operand (1, ptr32, opnd1); 

  /* ====================================== */ 
  Instruction_Group("O_203", 
		 TOP_GP32_LGR_GT_P13_P_U15, 
		 TOP_UNDEFINED); 

  Result (0, cr9); 
  Operand (0, pr, predicate); 
  Operand (1, p13, base); 
  Operand (2, u15, offset); 

  /* ====================================== */ 
  Instruction_Group("O_204", 
		 TOP_GP32_SCW_GT_AR_M_U9_CRH, 
		 TOP_GP32_SCW_GT_AR_P_U9_CRH, 
		 TOP_UNDEFINED); 

  Operand (0, pr, predicate); 
  Operand (1, ptr32, base); 
  Operand (2, u9, offset); 
  Operand (3, ctrlh, storeval); 



  ISA_Operands_End(); 
  return 0; 
} 
