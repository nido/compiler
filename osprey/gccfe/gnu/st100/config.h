/* since we do not use the regular gcc configure way, we need to
   distinguish here between the different host platforms... */

#ifdef linux
# include "auto-host_linux.h"
#endif
#ifdef sun
# include "auto-host_sun.h"
#endif
#ifdef __CYGWIN__
# include "auto-host_cygwin.h"
#endif
#ifdef __MINGW32__
# include "auto-host_mingw32.h"
#endif


#ifdef IN_GCC
# include "gansidecl.h"
#ifdef linux
# include "i386/xm-linux.h"
#endif
#ifdef sun
#define __BIG_ENDIAN__
# include "i386/xm-sun.h"
#endif
#ifdef __CYGWIN__
# include "i386/xm-i386.h"
# include "i386/xm-cygwin.h"
#endif
#ifdef __MINGW32__
# include "i386/xm-i386.h"
# include "i386/xm-mingw32.h"
#endif
# include "defaults.h"
# include "hwint.h"
#endif

#ifndef HAVE_ATEXIT
#define HAVE_ATEXIT
#endif
#ifndef POSIX
#define POSIX
#endif
#ifndef BSTRING
#define BSTRING
#endif
#ifdef SGI_MONGOOSE
#undef DWARF2_DEBUGGING_INFO
#endif

