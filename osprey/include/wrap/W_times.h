
#ifndef W_TIMES_H
#define W_TIMES_H

#include <host/config.h>

#ifndef USE_H_TIMES_H
#include <time.h>
#include <sys/times.h>
#else
#include "host/H_times.h"
#endif

#endif /* W_TIMES_H */
