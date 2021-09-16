#if !defined(VEIDES_STREAM_HUB_CLIENT_H)
#define VEIDES_STREAM_HUB_CLIENT_H

#if defined(__cplusplus)
 extern "C" {
#endif

#define LIBRARY_API __attribute__ ((visibility ("default")))

#include <MQTTProperties.h>

#include "veides_utils_shared/veides_rc.h"
#include "veides_utils_shared/veides_logger.h"

#include "veides_stream_hub_client/veides_base_client.h"
#include "veides_stream_hub_client/veides_properties.h"

/**
 * @brief Creates VeidesStreamHubClient object that is able to interact with Veides Stream Hub. To destroy object use VeidesStreamHubClient_destroy()
 *
 * @param client
 * @param properties
 */
LIBRARY_API VEIDES_RC VeidesStreamHubClient_create(VeidesStreamHubClient **client, VeidesStreamHubClientProperties *properties);

/**
 * @brief Destroys VeidesStreamHubClient object created by VeidesStreamHubClient_create()
 *
 * @param client
 */
LIBRARY_API VEIDES_RC VeidesStreamHubClient_destroy(VeidesStreamHubClient *client);

/**
 * @brief Connect to Veides Stream Hub
 *
 * @param client
 */
LIBRARY_API VEIDES_RC VeidesStreamHubClient_connect(VeidesStreamHubClient *client);

/**
 * @brief Disconnect from Veides Stream Hub
 *
 * @param client
 */
LIBRARY_API VEIDES_RC VeidesStreamHubClient_disconnect(VeidesStreamHubClient *client);

/**
 * @brief Check Veides Stream Hub connection status
 *
 * @param client
 */
LIBRARY_API VEIDES_RC VeidesStreamHubClient_isConnected(VeidesStreamHubClient *client);

/**
 * @brief Set a callback for the trail sent by particular agent
 *
 * @param client
 * @param agent
 * @param name
 * @param callback
 */
LIBRARY_API VEIDES_RC VeidesStreamHubClient_setTrailHandler(VeidesStreamHubClient *client, const char *agent, const char *name, VeidesTrailCallbackHandler callback);

#if defined(__cplusplus)
 }
#endif

#endif /* VEIDES_STREAM_HUB_CLIENT_H */

