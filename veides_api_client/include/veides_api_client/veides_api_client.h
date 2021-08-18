#if !defined(VEIDES_API_CLIENT_H)
#define VEIDES_API_CLIENT_H

#if defined(__cplusplus)
 extern "C" {
#endif

#define LIBRARY_API __attribute__ ((visibility ("default")))

#include "veides_api_client/veides_properties.h"

#include "veides_utils_shared/veides_rc.h"
#include "veides_utils_shared/veides_logger.h"

typedef void *VeidesApiClient;

/**
 * @brief Creates VeidesApiClient object that is able to interact with Veides API. To destroy object use VeidesApiClient_destroy()
 *
 * @param client
 * @param properties
 */
LIBRARY_API VEIDES_RC VeidesApiClient_create(VeidesApiClient **client, VeidesApiClientProperties *properties);

/**
 * @brief Destroys VeidesApiClient object created by VeidesApiClient_create()
 *
 * @param client
 */
LIBRARY_API VEIDES_RC VeidesApiClient_destroy(VeidesApiClient *client);

/**
 * @brief Invokes a method on an agent
 *
 * @param client
 */
LIBRARY_API VEIDES_RC VeidesApiClient_invokeMethod(VeidesApiClient *client, const char *agent, const char *name, const char *payload, int timeout, int *responseCode, char **responsePayload, int *responsePayloadLen);

#if defined(__cplusplus)
 }
#endif

#endif /* VEIDES_API_CLIENT_H */

