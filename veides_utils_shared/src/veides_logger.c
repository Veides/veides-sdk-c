#include "veides_utils_shared/veides_logger.h"
#include "veides_utils_shared/veides_logger_p.h"
#include "veides_utils_shared/veides_rc.h"

#define MAX_LOG_BUFSIZE 8192
// Default SDK log level
static int logLevel = VEIDES_LOG_INFO;
VeidesLogHandler *logHandler = NULL;
char *logFormat = "%s [%s] %s\n";
int maxBufSize = 8500;
char message[8500];

static char* veides_log_levelToString(int level) {
    switch(level) {
        case VEIDES_LOG_ERROR: return "ERROR";
        case VEIDES_LOG_WARNING: return "WARN";
        case VEIDES_LOG_INFO: return "INFO";
        case VEIDES_LOG_DEBUG: return "DEBUG";
    }

    return "UNKNOWN";
}

void veides_log(VeidesLogLevel level, const char *items, ...)
{
    if ((int) level < logLevel) {
        return;
    }

    va_list args;

    char buffer[MAX_LOG_BUFSIZE];
    memset(buffer, '0', MAX_LOG_BUFSIZE);

    va_start(args, items);
    vsnprintf(buffer, MAX_LOG_BUFSIZE, items, args);
    va_end(args);

    if (logHandler != NULL) {
        memset(message, '0', maxBufSize);
        snprintf(message, maxBufSize, logFormat, __TIMESTAMP__, veides_log_levelToString(level), buffer);
        (*logHandler)(level, message);
    } else {
        fprintf(stdout, logFormat, __TIMESTAMP__, veides_log_levelToString(level), buffer);
        fflush(stdout);
    }
}

void VeidesSDK_setLogLevel(VeidesLogLevel level) {
    VEIDES_LOG_INFO("SDK log level is set to %s", veides_log_levelToString(level));
    logLevel = level;
}

VEIDES_RC VeidesSDK_setLogHandler(VeidesLogHandler *handler)
{
    VEIDES_RC rc = VEIDES_RC_SUCCESS;

    if (!handler) {
        VEIDES_LOG_WARNING("Invalid SDK log handler provided");
        return VEIDES_RC_INVALID_HANDLE;
    }

    logHandler = handler;
    VEIDES_LOG_INFO("SDK log handler is set");

    return rc;
}
