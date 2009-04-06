#define TARGET_CPU_DEFAULT (TARGET_CPU_generic)
#include "auto-host.h"
#ifdef IN_GCC
/* Provide three core typedefs used by everything, if we are compiling
   GCC.  These used to be found in rtl.h and tree.h, but this is no
   longer practical.  Providing these here rather that system.h allows
   the typedefs to be used everywhere within GCC. */
struct rtx_def;
typedef struct rtx_def *rtx;
struct rtvec_def;
typedef struct rtvec_def *rtvec;
union tree_node;
typedef union tree_node *tree;
#endif
#define GTY(x)
#ifdef IN_GCC
# include "ansidecl.h"
/* since we do not use the regular gcc configure way, we need to
   distinguish here between the different host platforms... */
#ifdef __MINGW32__
# include "config/i386/xm-mingw32.h"
#endif
# include "dbxelf.h"
# include "elfos.h"
# include "hwint.h"
# include "arm/unknown-elf.h"
# include "arm/elf.h"
# include "arm/aout.h"
# include "arm/arm.h"
# include "defaults.h"
#endif
#ifndef POSIX
# define POSIX
#endif
#ifndef GENERATOR_FILE
# include "insn-constants.h"
# include "insn-flags.h"
#endif
