
#ifndef H_AR_H
#define H_AR_H

#ifdef CROSS_COMPILE

#define ARMAG	"!<arch>\n"
#define SARMAG	8

struct ar_hdr {
    char    ar_name[16];
    char    ar_date[12];
    char    ar_uid[6];
    char    ar_gid[6];
    char    ar_mode[8];
    char    ar_size[10];
    char    ar_fmag[2];
};

#define ARFMAG	"`\n"

#else /* if !CROSS_COMPILE */
/* linux-linux build. */
#include <ar.h>

#endif /* !CROSS_COMPILE */

#endif /* H_AR_H */
