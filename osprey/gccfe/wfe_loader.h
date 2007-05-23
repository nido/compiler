
#ifndef wfe_loader_INCLUDED
#define wfe_loader_INCLUDED

/* This file is in charge of providing extension sepcific feature */
/* Define the dynamique count: to be place in the loader section*/

#include "machmode.h"
#include "mtypes.h"
#ifdef __cplusplus
extern "C" {
#endif
  /* Loader initialization */
  extern void WFE_Init_Loader(void);
  /* Add the dynamic builtins */
  extern void WFE_Add_Builtins(void);
  /* matching between GCC BUITINS and OPEN64 INTRINSICS */
  extern INTRINSIC WFE_Intrinsic(enum built_in_function built);
  //For a given PREG in an extension, return the associated MTYPE
  extern TYPE_ID EXTENSION_Get_Mtype_For_Preg(PREG_NUM preg);
  extern void WFE_Loader_Initialize_Register (void);

  //Mapping between gcc reg and open64 PREG
  extern int *Map_Reg_To_Preg;
  extern int Map_Reg_To_Preg_Size;

#ifdef __cplusplus
}
#endif

#endif /* wfe_loader_INCLUDED */
