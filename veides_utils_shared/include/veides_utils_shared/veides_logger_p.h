#if !defined(VEIDES_LOGGER_P_H)
#define VEIDES_LOGGER_P_H

#if defined(__cplusplus)
 extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "veides_utils_shared/veides_logger.h"

void veides_log(VeidesLogLevel level, const char *items, ...);

#define VEIDES_LOG_DEBUG(fmts...) veides_log(VEIDES_LOG_DEBUG, fmts);
#define VEIDES_LOG_INFO(fmts...) veides_log(VEIDES_LOG_INFO, fmts);
#define VEIDES_LOG_WARNING(fmts...) veides_log(VEIDES_LOG_WARNING, fmts);
#define VEIDES_LOG_ERROR(fmts...) veides_log(VEIDES_LOG_ERROR, fmts);

#if defined(__cplusplus)
 }
#endif

#endif /* VEIDES_LOGGER_P_H */
