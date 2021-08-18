#include "veides_version.h"
#include "veides_utils_shared/veides_utils.h"

void veides_utils_freePtr(void *ptr) {
    if (ptr != NULL) {
        free(ptr);
        ptr = NULL;
    }
}

void veides_utils_sleep(long milsecs) {
    usleep(milsecs * 1000);
}
