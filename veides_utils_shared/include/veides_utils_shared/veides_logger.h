#if !defined(VEIDES_LOGGER_H)
#define VEIDES_LOGGER_H

#if defined(__cplusplus)
 extern "C" {
#endif

#define LIBRARY_API __attribute__ ((visibility ("default")))

#include "veides_utils_shared/veides_rc.h"

typedef enum VeidesLogLevel {
    VEIDES_LOG_DEBUG = 0,
    VEIDES_LOG_INFO = 1,
    VEIDES_LOG_WARNING = 2,
    VEIDES_LOG_ERROR = 3
} VeidesLogLevel;

typedef void VeidesLogHandler(int logLevel, char *message);

LIBRARY_API void VeidesSDK_setLogLevel(VeidesLogLevel level);
LIBRARY_API VEIDES_RC VeidesSDK_setLogHandler(VeidesLogHandler *handler);

#if defined(__cplusplus)
 }
#endif

#endif /* VEIDES_LOGGER_H */
