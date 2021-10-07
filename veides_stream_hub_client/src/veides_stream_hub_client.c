#include <cJSON/cJSON.h>

#include "veides_stream_hub_client/veides_stream_hub_client.h"
#include "veides_stream_hub_client/veides_base_client.h"
#include "veides_stream_hub_client/veides_internal.h"
#include "veides_utils_shared/veides_logger_p.h"

static void veides_sh_client_trailMessageReceived(VeidesStreamHubClient *client, char *topic, size_t topiclen, void* payload, size_t payloadlen);
static void veides_sh_client_eventMessageReceived(VeidesStreamHubClient *client, char *topic, size_t topiclen, void* payload, size_t payloadlen);

static char* veides_sh_client_buildTopicWithName(const char *clientId, const char *type, const char *name) {
    char *format = "agent/%s/%s/%s";
    int len = strlen(format) + strlen(clientId) + strlen(type) + strlen(name) - 3;

    char data[len];
    snprintf(data, len, format, clientId, type, name);

    return strdup(data);
}

VEIDES_RC VeidesStreamHubClient_create(VeidesStreamHubClient **client, VeidesStreamHubClientProperties *properties)
{
    VEIDES_RC rc = VEIDES_RC_SUCCESS;

    rc = veides_sh_client_create((void **) client, properties);
    if (rc != VEIDES_RC_SUCCESS) {
        VEIDES_LOG_ERROR("Failed to create Stream Hub client handle (rc=%d)", rc);
    }

    return rc;
}

VEIDES_RC VeidesStreamHubClient_destroy(VeidesStreamHubClient *client) {
    VEIDES_RC rc = VEIDES_RC_SUCCESS;

    rc = veides_sh_client_destroy((void *) client);
    if (rc != VEIDES_RC_SUCCESS) {
        VEIDES_LOG_ERROR("Failed to destroy Stream Hub client handle (rc=%d)", rc);
    }

    return rc;
}

VEIDES_RC VeidesStreamHubClient_connect(VeidesStreamHubClient *client) {
    VEIDES_RC rc = VEIDES_RC_SUCCESS;

    rc = veides_sh_client_connect((void *) client);
    if (rc != VEIDES_RC_SUCCESS) {
        VEIDES_LOG_ERROR("Failed to connect (rc=%d)", rc);
    }

    return rc;
}

VEIDES_RC VeidesStreamHubClient_disconnect(VeidesStreamHubClient *client) {
    VEIDES_RC rc = VEIDES_RC_SUCCESS;

    rc = veides_sh_client_disconnect((void *) client);
    if (rc != VEIDES_RC_SUCCESS) {
        VEIDES_LOG_ERROR("Failed to disconnect (rc=%d)", rc);
    }

    return rc;
}

VEIDES_RC VeidesStreamHubClient_isConnected(VeidesStreamHubClient *client) {
    return veides_sh_client_isConnected((void *) client);
}

VEIDES_RC VeidesStreamHubClient_setTrailHandler(VeidesStreamHubClient *client, const char *agent, const char *name, VeidesTrailCallbackHandler callback) {
    VEIDES_RC rc = VEIDES_RC_SUCCESS;

    if (!client) {
        rc = VEIDES_RC_INVALID_HANDLE;
        VEIDES_LOG_WARNING("Invalid Stream Hub client handle provided (rc=%d)", rc);
        return rc;
    }

    if (!callback || !agent || *agent == '\0' || !name || *name == '\0') {
        rc = VEIDES_RC_NULL_PARAM;
        VEIDES_LOG_WARNING("Invalid name or callback provided (rc=%d)", rc);
        return rc;
    }

    rc = veides_sh_client_setTrailHandler((void *) client, agent, name, callback);
    if (rc == VEIDES_RC_SUCCESS) {
        VeidesStreamHubClient *veidesClient = (VeidesStreamHubClient *) client;

        char *topic = veides_sh_client_buildTopicWithName(agent, "trail", name);

        VEIDES_LOG_DEBUG("Subscribe to topic %s", topic);

        rc = veides_sh_client_subscribe((void *) client, topic, 1);
        if (rc != VEIDES_RC_SUCCESS) {
            VEIDES_LOG_ERROR("Failed to subscribe to topic %s (rc=%d)", topic, rc);
        }

        rc = veides_sh_client_setHandler((void *) client, topic, (void *) veides_sh_client_trailMessageReceived);
        if (rc != VEIDES_RC_SUCCESS) {
            VEIDES_LOG_ERROR("Failed to set internal message handler (rc=%d)", rc);
        }
    } else {
        VEIDES_LOG_ERROR("Failed to set trail handler (rc=%d)", rc);
    }

    return rc;
}

VEIDES_RC VeidesStreamHubClient_setEventHandler(VeidesStreamHubClient *client, const char *agent, const char *name, VeidesEventCallbackHandler callback) {
    VEIDES_RC rc = VEIDES_RC_SUCCESS;

    if (!client) {
        rc = VEIDES_RC_INVALID_HANDLE;
        VEIDES_LOG_WARNING("Invalid Stream Hub client handle provided (rc=%d)", rc);
        return rc;
    }

    if (!callback || !agent || *agent == '\0' || !name || *name == '\0') {
        rc = VEIDES_RC_NULL_PARAM;
        VEIDES_LOG_WARNING("Invalid name or callback provided (rc=%d)", rc);
        return rc;
    }

    rc = veides_sh_client_setEventHandler((void *) client, agent, name, callback);
    if (rc == VEIDES_RC_SUCCESS) {
        VeidesStreamHubClient *veidesClient = (VeidesStreamHubClient *) client;

        char *topic = veides_sh_client_buildTopicWithName(agent, "event", name);

        VEIDES_LOG_DEBUG("Subscribe to topic %s", topic);

        rc = veides_sh_client_subscribe((void *) client, topic, 1);
        if (rc != VEIDES_RC_SUCCESS) {
            VEIDES_LOG_ERROR("Failed to subscribe to topic %s (rc=%d)", topic, rc);
        }

        rc = veides_sh_client_setHandler((void *) client, topic, (void *) veides_sh_client_eventMessageReceived);
        if (rc != VEIDES_RC_SUCCESS) {
            VEIDES_LOG_ERROR("Failed to set internal message handler (rc=%d)", rc);
        }
    } else {
        VEIDES_LOG_ERROR("Failed to set event handler (rc=%d)", rc);
    }

    return rc;
}

static void veides_sh_client_trailMessageReceived(VeidesStreamHubClient *client, char *topic, size_t topiclen, void* payload, size_t payloadlen) {
    char *pl = NULL;

    pl = (char *) malloc(payloadlen+1);

    memset(pl, 0, payloadlen+1);
    strncpy(pl, payload, payloadlen);

    cJSON *json = cJSON_ParseWithLength(pl, payloadlen+1);

    if (json == NULL) {
        goto endTrailReceived;
    }

    VeidesTrail trail = {NULL, NULL, 0, NULL, 0, 0, 0};

    cJSON *timestamp = cJSON_GetObjectItemCaseSensitive(json, "timestamp");

    char *name = strrchr(topic, '/');
    memmove(name, name, strlen(name));

    char *agent = strchr(topic, '/');
    memmove(agent, agent+1, strlen(agent));

    char *p = strchr(agent, '/');
    *p = '\0';

    VEIDES_LOG_DEBUG("Trail received (agent=%s, name=%s, payload=%s)", agent, name, pl);

    trail.name = strdup(name);
    trail.timestamp = strdup(timestamp->valuestring);

    cJSON *value = cJSON_GetObjectItemCaseSensitive(json, "value");

    if (cJSON_IsString(value)) {
        trail.valueType = VEIDES_TRAIL_VALUE_STRING;
        trail.valueString = value->valuestring;
    } else if (cJSON_IsBool(value)) {
        trail.valueType = VEIDES_TRAIL_VALUE_BOOL;
        trail.valueBool = cJSON_IsTrue(value);
    } else if (cJSON_IsNumber(value)) {
        if ((double) value->valueint != value->valuedouble) {
            trail.valueType = VEIDES_TRAIL_VALUE_DOUBLE;
            trail.valueDouble = value->valuedouble;
        } else {
            trail.valueType = VEIDES_TRAIL_VALUE_INT;
            trail.valueInt = value->valueint;
        }
    }

    VeidesStreamHubClientInternal *veidesClient = (VeidesStreamHubClientInternal *) client;

    VeidesTrailHandler *handler = veides_sh_client_getTrailHandler(veidesClient->trailHandlers, agent, name);

    if (handler != NULL) {
        VeidesTrailCallbackHandler callback = (VeidesTrailCallbackHandler) handler->callback;
        if (callback != NULL) {
            (*callback)(client, agent, trail);
        }
    }

endTrailReceived:
    if (pl) {
        free(pl);
    }
}

static void veides_sh_client_eventMessageReceived(VeidesStreamHubClient *client, char *topic, size_t topiclen, void* payload, size_t payloadlen) {
    char *pl = NULL;

    pl = (char *) malloc(payloadlen+1);

    memset(pl, 0, payloadlen+1);
    strncpy(pl, payload, payloadlen);

    cJSON *json = cJSON_ParseWithLength(pl, payloadlen+1);

    if (json == NULL) {
        goto endEventReceived;
    }

    VeidesEvent event = {NULL, NULL, NULL};

    cJSON *timestamp = cJSON_GetObjectItemCaseSensitive(json, "timestamp");
    cJSON *message = cJSON_GetObjectItemCaseSensitive(json, "message");

    char *name = strrchr(topic, '/');
    memmove(name, name, strlen(name));

    char *agent = strchr(topic, '/');
    memmove(agent, agent+1, strlen(agent));

    char *p = strchr(agent, '/');
    *p = '\0';

    VEIDES_LOG_DEBUG("Event received (agent=%s, name=%s, payload=%s)", agent, name, pl);

    event.name = strdup(name);
    event.timestamp = strdup(timestamp->valuestring);
    event.message = strdup(message->valuestring);

    VeidesStreamHubClientInternal *veidesClient = (VeidesStreamHubClientInternal *) client;

    VeidesEventHandler *handler = veides_sh_client_getEventHandler(veidesClient->eventHandlers, agent, name);

    if (handler != NULL) {
        VeidesEventCallbackHandler callback = (VeidesEventCallbackHandler) handler->callback;
        if (callback != NULL) {
            (*callback)(client, agent, event);
        }
    }

endEventReceived:
    if (pl) {
        free(pl);
    }
}
