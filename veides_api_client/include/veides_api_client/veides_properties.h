#if !defined(VEIDES_API_PROPERTIES_H)
#define VEIDES_API_PROPERTIES_H

#if defined(__cplusplus)
 extern "C" {
#endif

#define LIBRARY_API __attribute__ ((visibility ("default")))

#include <stdio.h>

#include "veides_utils_shared/veides_rc.h"

extern char **environ;

typedef struct VeidesApiAuthProperties {
    char *token;
} VeidesApiAuthProperties;

typedef struct VeidesConfigurationProperties {
    char *baseUrl;
} VeidesConfigurationProperties;

typedef struct VeidesApiClientProperties {
    VeidesApiAuthProperties *authProperties;
    VeidesConfigurationProperties *configurationProperties;
    int logLevel;
} VeidesApiClientProperties;

/**
 * @brief Creates VeidesApiClientProperties object that stores auth and configuration properties used by VeidesApiClent.
 * To destroy object use VeidesApiClientProperties_destroy()
 *
 * @param properties
 */
LIBRARY_API VEIDES_RC VeidesApiClientProperties_create(VeidesApiClientProperties **properties);

/**
 * @brief Destroys VeidesApiClientProperties object created by VeidesApiClientProperties_create()
 *
 * @param properties
 */
LIBRARY_API VEIDES_RC VeidesApiClientProperties_destroy(VeidesApiClientProperties *properties);

/**
 * @brief Sets VeidesApiClientProperties properties provided in environment variables
 *
 * @param properties
 */
LIBRARY_API VEIDES_RC VeidesApiClientProperties_setPropertiesFromEnv(VeidesApiClientProperties *properties);

/**
 * @brief Sets VeidesApiClientProperties property with given name
 *
 * @param properties
 * @param name
 * @param value
 */
LIBRARY_API VEIDES_RC VeidesApiClientProperties_setProperty(VeidesApiClientProperties *properties, const char *name, const char *value);

#if defined(__cplusplus)
 }
#endif

#endif /* VEIDES_API_CLIENT_PROPERTIES_H */
