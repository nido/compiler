#ifndef lao_cgir_INCLUDED
#define lao_cgir_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/*-------------------- Init/Fini --------------------------------*/
/* Need to be called only once per process execution. */
extern void CGIR_LAO_Init(void);
extern void CGIR_LAO_Fini(void);

/*-------------------- CGIR -> LIR Conversions ------------------*/
extern Operator CGIR_TOP_to_Operator(TOP top);
extern Operator CGIR_TOP_to_Operator(TOP top);
extern Register CGIR_CRP_to_Register(CLASS_REG_PAIR crp);
extern RegFile CGIR_IRC_to_RegFile(ISA_REGISTER_CLASS irc);
extern Immediate CGIR_LC_to_Immediate(ISA_LIT_CLASS ilc);
extern Processor CGIR_IS_to_Processor(ISA_SUBSET is);

/*-------------------- LIR -> CGIR Conversions ------------------*/
extern ISA_REGISTER_CLASS RegFile_to_CGIR_IRC(RegFile regClass);
extern CLASS_REG_PAIR Register_to_CGIR_CRP(Register registre);
extern TOP Operator_to_CGIR_TOP(Operator lir_operator);
extern TYPE_ID NativeType_to_CGIR_TYPE_ID(NativeType lir_nativeType);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* !lao_cgir_INCLUDED */
