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
# include "dbxelf.h"
# include "elfos.h"
# include "arm/unknown-elf.h"
# include "arm/elf.h"
# include "arm/aout.h"
# include "arm/arm.h"
# include "defaults.h"
#endif
#ifndef POSIX
# define POSIX
#endif
