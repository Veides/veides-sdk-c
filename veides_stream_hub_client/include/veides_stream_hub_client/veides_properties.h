#if !defined(VEIDES_STREAM_HUB_PROPERTIES_H)
#define VEIDES_STREAM_HUB_PROPERTIES_H

#if defined(__cplusplus)
 extern "C" {
#endif

#define LIBRARY_API __attribute__ ((visibility ("default")))

#include <stdio.h>

#include "veides_utils_shared/veides_rc.h"
#include "veides_utils_shared/veides_logger.h"

extern char **environ;

typedef struct VeidesStreamHubAuthProperties {
    char *username;
    char *token;
} VeidesStreamHubAuthProperties;

typedef struct VeidesConnectionProperties {
    char *host;
    int port;
} VeidesConnectionProperties;

typedef struct VeidesStreamHubClientProperties {
    VeidesStreamHubAuthProperties *authProperties;
    VeidesConnectionProperties *connectionProperties;
    int logLevel;
    int mqttLogLevel;
} VeidesStreamHubClientProperties;

/**
 * @brief Creates VeidesStreamHubClientProperties object that stores auth and connection properties used by VeidesStreamHubClient.
 * To destroy object use VeidesStreamHubClientProperties_destroy()
 *
 * @param properties
 */
LIBRARY_API VEIDES_RC VeidesStreamHubClientProperties_create(VeidesStreamHubClientProperties **properties);

/**
 * @brief Destroys VeidesStreamHubClientProperties object created by VeidesStreamHubClientProperties_create()
 *
 * @param properties
 */
LIBRARY_API VEIDES_RC VeidesStreamHubClientProperties_destroy(VeidesStreamHubClientProperties *properties);

/**
 * @brief Sets VeidesStreamHubClientProperties properties provided in environment variables
 *
 * @param properties
 */
LIBRARY_API VEIDES_RC VeidesStreamHubClientProperties_setPropertiesFromEnv(VeidesStreamHubClientProperties *properties);

/**
 * @brief Sets VeidesStreamHubClientProperties property with given name
 *
 * @param properties
 * @param name
 * @param value
 */
LIBRARY_API VEIDES_RC VeidesStreamHubClientProperties_setProperty(VeidesStreamHubClientProperties *properties, const char *name, const char *value);

#if defined(__cplusplus)
 }
#endif

#endif /* VEIDES_STREAM_HUB_PROPERTIES_H */
