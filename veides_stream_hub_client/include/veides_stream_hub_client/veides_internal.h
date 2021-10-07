#if !defined(VEIDES_STREAM_HUB_INTERNAL_H)
#define VEIDES_STREAM_HUB_INTERNAL_H

#if defined(__cplusplus)
 extern "C" {
#endif

#include <ctype.h>
#include <MQTTProperties.h>

#include "veides_stream_hub_client/veides_properties.h"
#include "veides_stream_hub_client/veides_base_client.h"

#include "veides_utils_shared/veides_rc.h"
#include "veides_utils_shared/veides_logger.h"
#include "veides_utils_shared/veides_utils.h"

typedef struct {
    int type;
    char *topic;
    void *callback;
} VeidesHandler;

typedef struct {
    VeidesHandler **entries;
    int count;
} VeidesHandlers;

typedef struct {
    char *name;
    char *agent;
    void *callback;
} VeidesTrailHandler;

typedef struct {
    VeidesTrailHandler **entries;
    int count;
} VeidesTrailHandlers;

typedef struct {
    char *name;
    char *agent;
    void *callback;
} VeidesEventHandler;

typedef struct {
    VeidesEventHandler **entries;
    int count;
} VeidesEventHandlers;

typedef struct VeidesStreamHubClientInternal {
    VeidesStreamHubClientProperties *properties;
    char                            *clientId;
    void                            *mqttClient;
    VeidesHandlers                  *handlers;
    VeidesTrailHandlers             *trailHandlers;
    VeidesEventHandlers             *eventHandlers;
    int                             connected;
} VeidesStreamHubClientInternal;

typedef void (*VeidesCallbackHandler)(void *client, char *topic, size_t topiclen, void *payload, size_t payloadlen);

VEIDES_RC veides_sh_client_create(void **client, VeidesStreamHubClientProperties *properties);
VEIDES_RC veides_sh_client_destroy(void *client);
VEIDES_RC veides_sh_client_connect(void *client);
VEIDES_RC veides_sh_client_isConnected(void *client);
VEIDES_RC veides_sh_client_disconnect(void *client);
VEIDES_RC veides_sh_client_retry_connection(void *client);
VEIDES_RC veides_sh_client_setHandler(void *client, char * topic, VeidesCallbackHandler handler);
VEIDES_RC veides_sh_client_setTrailHandler(void *client, const char *agent, const char *name, VeidesTrailCallbackHandler callback);
VeidesTrailHandler* veides_sh_client_getTrailHandler(VeidesTrailHandlers *handlers, char *agent, char *name);
VEIDES_RC veides_sh_client_setEventHandler(void *client, const char *agent, const char *name, VeidesEventCallbackHandler callback);
VeidesEventHandler* veides_sh_client_getEventHandler(VeidesEventHandlers *handlers, char *agent, char *name);
VEIDES_RC veides_sh_client_subscribe(void *client, char *topic, int qos);
VEIDES_RC veides_sh_client_publish(void *client, char *topic, char *payload, int qos);
VEIDES_RC veides_sh_client_topic_match(const char *sub, const char *topic);


#if defined(__cplusplus)
 }
#endif

#endif /* VEIDES_STREAM_HUB_INTERNAL_H */
