
#ifdef __cplusplus
extern "C" {
#endif
//Export functions needed by both the de and the be
#include "targ_isa_registers.h"
extern PREG_NUM CGTARG_Regclass_Preg_Min(  ISA_REGISTER_CLASS rclass);
extern PREG_NUM CGTARG_Regclass_Preg_Max(  ISA_REGISTER_CLASS rclass);
#ifdef __cplusplus
}
#endif
