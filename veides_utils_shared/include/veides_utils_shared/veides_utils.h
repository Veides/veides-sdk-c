#if !defined(VEIDES_UTILS_SHARED_H)
#define VEIDES_UTILS_SHARED_H

#if defined(__cplusplus)
 extern "C" {
#endif

#include <unistd.h>
#include <stdlib.h>

#include "veides_utils_shared/veides_rc.h"

void veides_utils_freePtr(void *p);
void veides_utils_sleep(long milsecs);

#if defined(__cplusplus)
 }
#endif

#endif /* VEIDES_UTILS_H */
