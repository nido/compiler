
/* This file holds definitions specific to the ST200 ELF ABI. */

#ifndef _TARG_ELF_H
#define _TARG_ELF_H

/* (pp) core */
#define _ELF_LX_CORE_BIT	(8)                      /* 1st bit position in byte */
#define ELF_LX_CORE_MASK	(0x3<<_ELF_LX_CORE_BIT)           /* mask */
#define ELF_LX_CORE_ST210	(0x0<<_ELF_LX_CORE_BIT)
#define ELF_LX_CORE_ST220	(0x1<<_ELF_LX_CORE_BIT)
#define ELF_LX_CORE_ST230	(0x2<<_ELF_LX_CORE_BIT)
#define ELF_LX_CORE_UNDEF	(0x3<<_ELF_LX_CORE_BIT)
#define _ELF_LX_CHECK_CORE(m) ((m&ELF_LX_CORE_MASK)==m)

/* (pp) cut */
#define _ELF_LX_CUT_BIT	(16)                             /* 1st bit position in byte */
#define ELF_LX_CUT_MASK		(0x7<<_ELF_LX_CUT_BIT)           /* mask */
#define ELF_LX_CUT_0		(0x0<<_ELF_LX_CUT_BIT)
#define ELF_LX_CUT_1		(0x1<<_ELF_LX_CUT_BIT)
#define ELF_LX_CUT_2		(0x2<<_ELF_LX_CUT_BIT)
#define ELF_LX_CUT_3		(0x3<<_ELF_LX_CUT_BIT)
#define ELF_LX_CUT_4		(0x4<<_ELF_LX_CUT_BIT)
#define ELF_LX_CUT_5		(0x5<<_ELF_LX_CUT_BIT)
#define ELF_LX_CUT_UNDEF	(0x6<<_ELF_LX_CUT_BIT)
#define _ELF_LX_CHECK_CUT(m) ((m&ELF_LX_CUT_MASK)==m)

/* (pp) abi */
#define _ELF_LX_ABI_BIT	(0)                             /* 1st bit position in byte */
#define ELF_LX_ABI_MASK		(0x7<<_ELF_LX_ABI_BIT)           /* mask */
#define ELF_LX_ABI_NO		(0x0<<_ELF_LX_ABI_BIT)
#define ELF_LX_ABI_MULTI	(0x1<<_ELF_LX_ABI_BIT)
#define ELF_LX_ABI_EMBED	(0x2<<_ELF_LX_ABI_BIT)
#define ELF_LX_ABI_PIC		(0x3<<_ELF_LX_ABI_BIT)
#define ELF_LX_ABI_GCC		(0x4<<_ELF_LX_ABI_BIT)
#define ELF_LX_ABI_UNDEF	(0x5<<_ELF_LX_ABI_BIT)
#define ELF_LX_ABI_RELOC_EMBED  (0x6<<_ELF_LX_ABI_BIT)
#define _ELF_LX_CHECK_ABI(m) ((m&ELF_LX_ABI_MASK)==m)

/* [CL] code generation model */
#define _ELX_LX_CODEGEN_BIT (24)
#define ELF_LX_CODEGEN_MASK      (0x7<<_ELF_LX_CODEGEN_BIT)
#define ELF_LX_CODEGEN_ABSOLUTE  (0x0<<_ELF_LX_CODEGEN_BIT)
#define ELF_LX_CODEGEN_PIC       (0x1<<_ELF_LX_CODEGEN_BIT)
#define ELF_LX_CODEGEN_CPIC      (0x2<<_ELF_LX_CODEGEN_BIT)
#define _ELF_LX_CHECK_CODEGEN(m) ((m&ELF_LX_CODEGEN_MASK)==m)

#define EM_LX 0x64

#endif
