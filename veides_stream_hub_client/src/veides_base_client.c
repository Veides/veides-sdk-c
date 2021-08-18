#include <MQTTAsync.h>
#include <pthread.h>

#include "veides_stream_hub_client/veides_base_client.h"
#include "veides_stream_hub_client/veides_internal.h"
#include "veides_utils_shared/veides_logger.h"
#include "veides_utils_shared/veides_logger_p.h"
#include "veides_version.h"

static pthread_mutex_t veides_sh_client_mutex_store = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t* veides_sh_client_mutex = &veides_sh_client_mutex_store;
static int veides_mutex_inited = 0;

static void veides_init_mutex(void) {
    if (veides_mutex_inited == 1) {
        return;
    }

    int rc = 0;
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    if ((rc = pthread_mutex_init(veides_sh_client_mutex, &attr)) != 0)
        VEIDES_LOG_ERROR("Failed to initialize veides_sh_client_mutex (rc=%d)", rc);

    veides_mutex_inited = 1;
}

static int veides_sh_client_messageArrived(void *context, char *topicName, int topicLen, MQTTAsync_message *message);
static int veides_properties_validate(VeidesStreamHubClientProperties *properties);

static VEIDES_RC veides_add_handler(VeidesHandlers *handlers, VeidesHandler *handler) {
    VEIDES_RC rc = VEIDES_RC_SUCCESS;

    VeidesHandler **tmp = NULL;
    tmp = realloc(handlers->entries, sizeof(VeidesHandler *));
    if (tmp == NULL) {
        return VEIDES_RC_NOMEM;
    }

    handlers->entries = tmp;

    handlers->entries[handlers->count] = handler;

    handlers->count++;

    return rc;
}

static VEIDES_RC veides_add_trailHandler(VeidesTrailHandlers *handlers, VeidesTrailHandler *handler) {
    VEIDES_RC rc = VEIDES_RC_SUCCESS;

    VeidesTrailHandler **tmp = NULL;
    tmp = realloc(handlers->entries, sizeof(VeidesTrailHandler *));
    if (tmp == NULL) {
        return VEIDES_RC_NOMEM;
    }

    handlers->entries = tmp;
    handlers->entries[handlers->count] = handler;
    handlers->count++;

    return rc;
}

static VeidesHandler* veides_sh_client_getHandler(VeidesHandlers *handlers, char *topic) {
    int i = 0;
    VeidesHandler *handler = NULL;
    int found = 0;
    for (i = 0; i < handlers->count; i++) {
        handler = handlers->entries[i];
        if (topic && handler->topic && veides_sh_client_topic_match(handler->topic, topic) == VEIDES_RC_SUCCESS) {
            found = 1;
            break;
        }
    }
    if (found == 0) {
        handler = NULL;
    }

    return handler;
}

VeidesTrailHandler* veides_sh_client_getTrailHandler(VeidesTrailHandlers *handlers, char *agent, char *name) {
    VeidesTrailHandler *handler = NULL;

    if (!handlers) {
        VEIDES_LOG_ERROR("Invalid handle for trail handlers provided (handlers=%s)", handlers ? "valid" : "NULL");
        return NULL;
    }

    if (!name || *name == '\0') {
        VEIDES_LOG_ERROR("Invalid arguments provided (name=%s)", name ? "Valid" : "NULL");
        return NULL;
    }

    int i = 0;
    int found = 0;
    for (i = 0; i < handlers->count; i++) {
        handler = handlers->entries[i];
        if (handler->name && handler->agent && strcmp(name, handler->name) == 0 && strcmp(agent, handler->agent) == 0) {
            found = 1;
            break;
        }
    }
    if (found == 0) {
        handler = NULL;
    }

    return handler;
}

void onConnect(void *context, MQTTAsync_successData *response) {
    VeidesStreamHubClientInternal *client = (VeidesStreamHubClientInternal *) context;

    pthread_mutex_lock(veides_sh_client_mutex);

    client->connected = 1;

    VEIDES_LOG_INFO("Veides Stream Hub client connected");

    pthread_mutex_unlock(veides_sh_client_mutex);
}

void onConnectFailure(void *context, MQTTAsync_failureData *response) {
    int rc = 0;

    VeidesStreamHubClientInternal *client = (VeidesStreamHubClientInternal *) context;

    if (response) {
        rc = response->code;
    }

    pthread_mutex_lock(veides_sh_client_mutex);

    client->connected = (0 - rc);

    if (response && response->message != NULL) {
        VEIDES_LOG_WARNING("Failed to connect (rc=%d, responseMessage=%s)", response->code, response->message);
    } else {
        VEIDES_LOG_WARNING("Failed to connect");
    }

    pthread_mutex_unlock(veides_sh_client_mutex);
}

void onDisconnect(void *context, MQTTAsync_successData *response) {
    VeidesStreamHubClientInternal *client = (VeidesStreamHubClientInternal *) context;

    pthread_mutex_lock(veides_sh_client_mutex);

    client->connected = 0;

    VEIDES_LOG_INFO("Client disconnected successfuly");

    pthread_mutex_unlock(veides_sh_client_mutex);
}

void onDisconnectFailure(void *context, MQTTAsync_failureData *response) {
    int rc = 0;

    VeidesStreamHubClientInternal *client = (VeidesStreamHubClientInternal *) context;

    if (response) {
        rc = response->code;
    }

    pthread_mutex_lock(veides_sh_client_mutex);

    client->connected = (0 - rc);

    if (response && response->message != NULL) {
        VEIDES_LOG_WARNING("Failed to disconnect (rc=%d, responseMessage=%s)", response->code, response->message);
    } else {
        VEIDES_LOG_WARNING("Failed to disconnect");
    }

    pthread_mutex_unlock(veides_sh_client_mutex);
}

void onSendFailure(void *context, MQTTAsync_failureData *response) {
    if (response && response->message != NULL) {
        VEIDES_LOG_WARNING("Failed to send message (rc=%d, responseMessage=%s)", response->code, response->message);
    } else {
        VEIDES_LOG_WARNING("Failed to send message");
    }
}

void onSubscribe(void *context, MQTTAsync_successData *response) {
    VEIDES_LOG_DEBUG("Subscribed to a topic");
}

void onSubscribeFailure(void *context, MQTTAsync_failureData *response) {
    if (response && response->message != NULL) {
        VEIDES_LOG_WARNING("Failed to subscribe to a topic (rc=%d, responseMessage=%s)", response->code, response->message);
    } else {
        VEIDES_LOG_WARNING("Failed to subscribe to a topic");
    }
}

VEIDES_RC veides_sh_client_create(void **veidesClient, VeidesStreamHubClientProperties *properties) {
    VEIDES_LOG_INFO("Veides Stream Hub Client - Version: %s", VEIDES_SDK_VERSION);
    VEIDES_LOG_INFO("Paho MQTT Client - Version: %s", PAHO_CLIENT_VERSION);

    VEIDES_RC rc = VEIDES_RC_SUCCESS;

    veides_init_mutex();

    if (!veidesClient) {
        rc = VEIDES_RC_INVALID_HANDLE;
        VEIDES_LOG_ERROR("Invalid aget client handle provided (rc=%d)", rc);
        return rc;
    }

    if (*veidesClient != NULL) {
        rc = VEIDES_RC_INVALID_PARAM_VALUE;
        VEIDES_LOG_ERROR("Stream Hub client handle is already created");
        return rc;
    }

    rc = veides_properties_validate(properties);

    if (rc != VEIDES_RC_SUCCESS) {
        return rc;
    }

    char *host = properties->connectionProperties->host;
    int port = properties->connectionProperties->port;

    int len = strlen(host) + 13;
    char url[len];

    snprintf(url, len, "ssl://%s:%d", host, port);

    VeidesStreamHubClientInternal *client = (VeidesStreamHubClientInternal *) malloc(sizeof(VeidesStreamHubClientInternal));
    client->properties = properties;
    client->mqttClient = NULL;
    client->handlers = (VeidesHandlers *) malloc(sizeof(VeidesHandlers));
    client->handlers->count = 0;
    client->handlers->entries = (VeidesHandler **) malloc(sizeof(VeidesHandler*));
    client->trailHandlers = (VeidesTrailHandlers *) malloc(sizeof(VeidesTrailHandlers));
    client->trailHandlers->count = 0;
    client->trailHandlers->entries = (VeidesTrailHandler **) malloc(sizeof(VeidesTrailHandler*));

    int clientIdLen = strlen(properties->authProperties->username) + 11;
    client->clientId = malloc(clientIdLen);
    snprintf(client->clientId, clientIdLen, "%s_%d", properties->authProperties->username, getpid());

    MQTTAsync mqttClient;
    MQTTAsync_createOptions create_opts = MQTTAsync_createOptions_initializer;

    create_opts.MQTTVersion = MQTTVERSION_3_1_1;
    create_opts.sendWhileDisconnected = 1;

    rc = MQTTAsync_createWithOptions(&mqttClient, url, client->clientId, MQTTCLIENT_PERSISTENCE_NONE, NULL, &create_opts);
    if (rc != MQTTASYNC_SUCCESS) {
        VEIDES_LOG_ERROR("MQTTAsync_createWithOptions failed (url=%s)", url);
        veides_sh_client_destroy(client);
        client = NULL;
        return rc;
    }

    veides_utils_sleep(50);

    client->mqttClient = (void *) mqttClient;

    *veidesClient = client;

    return rc;
}

VEIDES_RC veides_sh_client_destroy(void *veidesClient) {
    VEIDES_RC rc = VEIDES_RC_SUCCESS;
    VeidesStreamHubClientInternal *client = (VeidesStreamHubClientInternal *) veidesClient;
    VeidesHandlers *handlers = NULL;
    int i = 0;

    if (!client || (client && client->mqttClient == NULL)) {
        rc = VEIDES_RC_INVALID_HANDLE;
        VEIDES_LOG_ERROR("Invalid or NULL Stream Hub client handle provided (rc=%d)", rc);
        return rc;
    }
    veides_utils_freePtr((void *) client->clientId);
    handlers = client->handlers;

    for (i=0; i<handlers->count; i++)
    {
        VeidesHandler *sub = handlers->entries[i];
        veides_utils_freePtr((void *) sub->topic);
    }

    veides_utils_freePtr((void *) handlers);

    client->properties = NULL;
    veides_utils_freePtr((void *) client);
    client = NULL;

    return rc;
}

VEIDES_RC veides_sh_client_connect(void *veidesClient) {
    VEIDES_RC rc = VEIDES_RC_SUCCESS;
    VeidesStreamHubClientInternal *client = (VeidesStreamHubClientInternal *) veidesClient;

    if (!client || (client && client->mqttClient == NULL)) {
        rc = VEIDES_RC_INVALID_HANDLE;
        VEIDES_LOG_ERROR("Invalid Stream Hub client handle provided (rc=%d)", rc);
        return rc;
    }

    VeidesStreamHubClientProperties *properties = (VeidesStreamHubClientProperties *) client->properties;

    rc = veides_properties_validate(properties);

    if (rc != VEIDES_RC_SUCCESS) {
        return rc;
    }

    MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
    MQTTAsync_SSLOptions ssl_opts = MQTTAsync_SSLOptions_initializer;

    conn_opts.keepAliveInterval = 60;
    conn_opts.onSuccess = onConnect;
    conn_opts.onFailure = onConnectFailure;
    conn_opts.MQTTVersion = MQTTVERSION_3_1_1;
    conn_opts.context = client;
    conn_opts.automaticReconnect = 1;
    conn_opts.cleansession = 1;

    ssl_opts.enableServerCertAuth = 1;
    ssl_opts.verify = 1;
    conn_opts.ssl = &ssl_opts;

    conn_opts.username = properties->authProperties->username;
    conn_opts.password = properties->authProperties->token;

    MQTTAsync_setCallbacks((MQTTAsync *) client->mqttClient, (void *) client, NULL, veides_sh_client_messageArrived, NULL);

    VEIDES_LOG_DEBUG(
        "Connecting to %s:%d with clientId %s",
        client->properties->connectionProperties->host,
        client->properties->connectionProperties->port,
        client->clientId
    );

    if ((rc = MQTTAsync_connect((MQTTAsync *) client->mqttClient, &conn_opts)) == MQTTASYNC_SUCCESS) {
        int iteration = 1;
        int isConnected = 0;
        while (isConnected == 0) {
            veides_utils_sleep(1000);

            pthread_mutex_lock(veides_sh_client_mutex);
            isConnected = client->connected;
            pthread_mutex_unlock(veides_sh_client_mutex);

            if (isConnected == 1) {
                break;
            }

            if (isConnected < 0) {
                rc = (0 - isConnected);
                pthread_mutex_lock(veides_sh_client_mutex);
                client->connected = 0;
                pthread_mutex_unlock(veides_sh_client_mutex);
                break;
            }

            VEIDES_LOG_INFO("Waiting for client to connect (iteration=%d)", iteration);

            if (iteration >= 30)  {
                rc = VEIDES_RC_TIMEOUT;
                break;
            }

            iteration++;
        }

    }

    return rc;
}

VEIDES_RC veides_sh_client_isConnected(void *veidesClient) {
    VEIDES_RC rc = VEIDES_RC_SUCCESS;
    VeidesStreamHubClientInternal *client = (VeidesStreamHubClientInternal *) veidesClient;

    if (!client || (client && client->mqttClient == NULL)) {
        rc = VEIDES_RC_INVALID_HANDLE;
        VEIDES_LOG_ERROR("Invalid Stream Hub client handle provided (rc=%d)", rc);
        return rc;
    }

    if (MQTTAsync_isConnected(client->mqttClient) == 0) {
        rc = VEIDES_RC_NOT_CONNECTED;
    }

    return rc;
}

VEIDES_RC veides_sh_client_disconnect(void *veidesClient) {
    VEIDES_RC rc = 0;
    VeidesStreamHubClientInternal *client = (VeidesStreamHubClientInternal *) veidesClient;

    if (!client || (client && client->mqttClient == NULL)) {
        rc = VEIDES_RC_INVALID_HANDLE;
        VEIDES_LOG_ERROR("Invalid Stream Hub client handle provided (rc=%d)", rc);
        return rc;
    }

    MQTTAsync mqttClient = (MQTTAsync *) client->mqttClient;
    MQTTAsync_disconnectOptions disc_opts = MQTTAsync_disconnectOptions_initializer;

    disc_opts.onSuccess = onDisconnect;
    disc_opts.onFailure = onDisconnectFailure;
    disc_opts.context = client;

    int iteration = 1;
    int isConnected = client->connected;

    if (isConnected == 1) {
        VEIDES_LOG_INFO("Disconnect client.");
        int mqttRC = 0;
        mqttRC = MQTTAsync_disconnect(mqttClient, &disc_opts);
        if (mqttRC == MQTTASYNC_DISCONNECTED) {
            rc = VEIDES_RC_SUCCESS;
        } else {
            rc = mqttRC;
            return rc;
        }

        pthread_mutex_lock(veides_sh_client_mutex);
        isConnected = client->connected;
        pthread_mutex_unlock(veides_sh_client_mutex);

        while (isConnected == 1) {
            veides_utils_sleep(1000);

            pthread_mutex_lock(veides_sh_client_mutex);
            isConnected = client->connected;
            pthread_mutex_unlock(veides_sh_client_mutex);

            if (isConnected == 0) {
                break;
            }

            if (isConnected < 0) {
                rc = (0 - isConnected);
                pthread_mutex_lock(veides_sh_client_mutex);
                client->connected = 0;
                pthread_mutex_unlock(veides_sh_client_mutex);
                break;
            }

            VEIDES_LOG_INFO("Wait for client to disconnect (iteration=%d)", iteration);

            if (iteration >= 30)  {
                rc = VEIDES_RC_TIMEOUT;
                break;
            }

            iteration++;
        }
    }

    return rc;
}

static int reconnect_delay(int try) {
    if (try <= 10) {
        return 3000;
    }

    if (try <= 20) {
        return 15000;
    }

    return 60000;
}

VEIDES_RC veides_sh_client_retry_connection(void *veidesClient) {
    VEIDES_RC rc = VEIDES_RC_SUCCESS;
    int retry = 1;

    while((rc = veides_sh_client_connect(veidesClient)) != MQTTASYNC_SUCCESS)
    {
        int delay = reconnect_delay(retry++);
        VEIDES_LOG_DEBUG("Connect retry (attempt=%d, delay=%d)", retry, delay);
        veides_utils_sleep(delay);
    }

    return rc;
}

VEIDES_RC veides_sh_client_publish(void *veidesClient, char *topic, char *payload, int qos) {
    VEIDES_RC rc = VEIDES_RC_SUCCESS;
    VeidesStreamHubClientInternal *client = (VeidesStreamHubClientInternal *) veidesClient;

    if (!client || (client && client->mqttClient == NULL)) {
        rc = VEIDES_RC_INVALID_HANDLE;
        VEIDES_LOG_ERROR("Invalid Stream Hub client handle provided (rc=%d)", rc);
        return rc;
    }

    if (client->connected == 0) {
        rc = VEIDES_RC_NOT_CONNECTED;
        VEIDES_LOG_ERROR("Stream Hub client is not connected");
        return rc;
    }

    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    MQTTAsync mqttClient = (MQTTAsync *) client->mqttClient;

    opts.onFailure = onSendFailure;
    opts.context = client;

    int payloadlen = 0;
    if (payload && *payload != '\0') {
        payloadlen = strlen(payload);
    }

    VEIDES_LOG_DEBUG("Sending message on topic %s with payload %s", topic, payload);

    rc = MQTTAsync_send(mqttClient, topic, payloadlen, payload, qos, 0, &opts);
    if (rc != MQTTASYNC_SUCCESS && rc != VEIDES_RC_INVALID_HANDLE) {
        VEIDES_LOG_ERROR("MQTTAsync_send returned error (rc=%d)", rc);
        VEIDES_LOG_WARNING("Connection is lost, retry connection and republish message.");
        veides_sh_client_retry_connection(mqttClient);
        rc = MQTTAsync_send(mqttClient, topic, payloadlen, payload, qos, 0, &opts);
    }

    return rc;
}

VEIDES_RC veides_sh_client_subscribe(void *veidesClient, char *topic, int qos) {
    VEIDES_RC rc = VEIDES_RC_SUCCESS;
    VeidesStreamHubClientInternal *client = (VeidesStreamHubClientInternal *) veidesClient;

    if (!client || (client && client->mqttClient == NULL)) {
        rc = VEIDES_RC_INVALID_HANDLE;
        VEIDES_LOG_ERROR("Invalid Stream Hub client handle provided (rc=%d)", rc);
        return rc;
    }

    for (int tries = 0; tries < 5; tries++) {
        if (client->connected == 0) {
            VEIDES_LOG_WARNING("Client is not connected yet. Wait for client to connect and subscribe.");
            veides_utils_sleep(2000);
        }
    }

    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    MQTTAsync mqttClient = (MQTTAsync *) client->mqttClient;

    opts.onSuccess = onSubscribe;
    opts.onFailure = onSubscribeFailure;
    opts.context = client;

    rc = MQTTAsync_subscribe(mqttClient, topic, qos, &opts);

    if (rc != MQTTASYNC_SUCCESS) {
        VEIDES_LOG_ERROR("Unable to subscribe to %s", topic);
    }

    return rc;
}

VEIDES_RC veides_sh_client_setHandler(void *veidesClient, char *topic, VeidesCallbackHandler callback) {
    VEIDES_RC rc = VEIDES_RC_SUCCESS;
    VeidesStreamHubClientInternal *client = (VeidesStreamHubClientInternal *) veidesClient;

    if (!client) {
        rc = VEIDES_RC_INVALID_HANDLE;
        VEIDES_LOG_ERROR("Invalid Stream Hub client handle provided (rc=%d)", rc);
        return rc;
    }

    if (!topic || *topic == '\0') {
        rc = VEIDES_RC_NULL_PARAM;
        VEIDES_LOG_ERROR("Invalid topic provided (rc=%d)", rc);
        return rc;
    }

    int found = 0;
    int i=0;
    for (i = 0; i < client->handlers->count; i++) {
        VeidesHandler *handler = client->handlers->entries[i];
        if (topic && handler->topic && veides_sh_client_topic_match(handler->topic, topic) == VEIDES_RC_SUCCESS) {
            found = 1;
            break;
        }
    }
    if (found == 0) {
        VeidesHandler *handler = (VeidesHandler *) malloc(sizeof(VeidesHandler));
        handler->topic = strdup(topic);
        handler->callback = callback;
        rc = veides_add_handler(client->handlers, handler);
        if (rc == VEIDES_RC_SUCCESS) {
            VEIDES_LOG_DEBUG("Set message handler for topic %s", topic);
        } else {
            VEIDES_LOG_WARNING("Failed to set handler for topic %s (rc=%d)", topic, rc);
        }
    } else {
        VeidesHandler *handler = client->handlers->entries[i];
        handler->callback = callback;
        VEIDES_LOG_DEBUG("Updated handler for topic %s", topic);
    }

    return rc;
}

VEIDES_RC veides_sh_client_setTrailHandler(void *veidesClient, const char *agent, const char *name, VeidesTrailCallbackHandler callback) {
    VEIDES_RC rc = VEIDES_RC_SUCCESS;
    VeidesStreamHubClientInternal *client = (VeidesStreamHubClientInternal *) veidesClient;

    if (!client) {
        rc = VEIDES_RC_INVALID_HANDLE;
        VEIDES_LOG_ERROR("Invalid Stream Hub client handle provided (rc=%d)", rc);
        return rc;
    }

    int found = 0;
    int i = 0;
    for (i = 0; i < client->trailHandlers->count; i++) {
        VeidesTrailHandler *handler = client->trailHandlers->entries[i];
        if (handler->name && strcmp(name, handler->name) == 0) {
            found = 1;
            break;
        }
    }
    if (found == 0) {
        VeidesTrailHandler *handler = (VeidesTrailHandler *) malloc(sizeof(VeidesTrailHandler));
        handler->name = strdup(name);
        handler->agent = strdup(agent);
        handler->callback = callback;

        rc = veides_add_trailHandler(client->trailHandlers, handler);
        if (rc == VEIDES_RC_SUCCESS) {
            VEIDES_LOG_INFO("Added handler for trail %s", name);
        } else {
            VEIDES_LOG_WARNING("Failed to set handler for trail %s (rc=%d)", name, rc);
        }
    } else {
        VeidesTrailHandler *handler = client->trailHandlers->entries[i];
        handler->callback = callback;
        VEIDES_LOG_INFO("Updated handler for trail %s", name);
    }

    return rc;
}

VEIDES_RC veides_sh_client_topic_match(const char *sub, const char *topic)
{
    size_t spos;

    if (!sub || !topic || sub[0] == 0 || topic[0] == 0) {
        return VEIDES_RC_FAILURE;
    }

    if ((sub[0] == '$' && topic[0] != '$') || (topic[0] == '$' && sub[0] != '$')) {
        return VEIDES_RC_SUCCESS;
    }

    spos = 0;

    while (sub[0] != 0) {
        if (topic[0] == '+' || topic[0] == '#') {
            return VEIDES_RC_FAILURE;
        }
        if (sub[0] != topic[0] || topic[0] == 0) { /* Check for wildcard matches */
            if (sub[0] == '+') {
                /* Check for bad "+foo" or "a/+foo" subscription */
                if (spos > 0 && sub[-1] != '/') {
                    return VEIDES_RC_FAILURE;
                }
                /* Check for bad "foo+" or "foo+/a" subscription */
                if (sub[1] != 0 && sub[1] != '/') {
                    return VEIDES_RC_FAILURE;
                }
                spos++;
                sub++;
                while(topic[0] != 0 && topic[0] != '/') {
                    if (topic[0] == '+' || topic[0] == '#') {
                        return VEIDES_RC_FAILURE;
                    }
                    topic++;
                }
                if (topic[0] == 0 && sub[0] == 0) {
                    return VEIDES_RC_SUCCESS;
                }
            } else if (sub[0] == '#') {
                /* Check for bad "foo#" subscription */
                if (spos > 0 && sub[-1] != '/') {
                    return VEIDES_RC_FAILURE;
                }
                /* Check for # not the final character of the sub, e.g. "#foo" */
                if (sub[1] != 0) {
                    return VEIDES_RC_FAILURE;
                }

                while(topic[0] != 0) {
                    if (topic[0] == '+' || topic[0] == '#') {
                        return VEIDES_RC_FAILURE;
                    }
                    topic++;
                }
                return VEIDES_RC_SUCCESS;
            } else {
                /* Check for e.g. foo/bar matching foo/+/# */
                if (topic[0] == 0
                        && spos > 0
                        && sub[-1] == '+'
                        && sub[0] == '/'
                        && sub[1] == '#')
                {
                    return VEIDES_RC_SUCCESS;
                }

                /* There is no match at this point, but is the sub invalid? */
                while(sub[0] != 0) {
                    if (sub[0] == '#' && sub[1] != 0) {
                        return VEIDES_RC_FAILURE;
                    }
                    spos++;
                    sub++;
                }

                /* Valid input, but no match */
                return VEIDES_RC_FAILURE;
            }
        } else {
            /* sub[spos] == topic[tpos] */
            if (topic[1] == 0) {
                /* Check for e.g. foo matching foo/# */
                if (sub[1] == '/'
                        && sub[2] == '#'
                        && sub[3] == 0) {
                    return VEIDES_RC_SUCCESS;
                }
            }
            spos++;
            sub++;
            topic++;
            if (sub[0] == 0 && topic[0] == 0) {
                return VEIDES_RC_SUCCESS;
            } else if (topic[0] == 0 && sub[0] == '+' && sub[1] == 0) {
                if (spos > 0 && sub[-1] != '/') {
                    return VEIDES_RC_FAILURE;
                }
                spos++;
                sub++;
                return VEIDES_RC_SUCCESS;
            }
        }
    }
    if ((topic[0] != 0 || sub[0] != 0)) {
        return VEIDES_RC_FAILURE;
    }
    while (topic[0] != 0) {
        if (topic[0] == '+' || topic[0] == '#') {
            return VEIDES_RC_FAILURE;
        }
        topic++;
    }

    return VEIDES_RC_SUCCESS;
}

static int veides_sh_client_messageArrived(void *context, char *topicName, int topicLen, MQTTAsync_message *message) {
    VEIDES_RC rc = VEIDES_RC_SUCCESS;
    VeidesStreamHubClientInternal *client = (VeidesStreamHubClientInternal *) context;

    if (topicLen > 0) {
        VEIDES_LOG_DEBUG("Message Received (topic=%s, topicLen=%d)", topicName ? topicName : "", topicLen);
    }

    if (!client) {
        rc = VEIDES_RC_INVALID_HANDLE;
        VEIDES_LOG_ERROR("Invalid Stream Hub client handle provided (rc=%d)", rc);
        return rc;
    }

    if (client->handlers->count == 0) {
        rc = VEIDES_RC_HANDLER_NOT_FOUND;
        VEIDES_LOG_WARNING("Not a single callback is registered");
        return rc;
    }

    VeidesHandler *sub = veides_sh_client_getHandler(client->handlers, topicName);
    if (sub == NULL) {
        rc = VEIDES_RC_HANDLER_NOT_FOUND;
        VEIDES_LOG_ERROR("Callback not found for topic %s", topicName ? topicName : "");
        return rc;
    }

    VeidesCallbackHandler cb = (VeidesCallbackHandler) sub->callback;

    if (cb != 0) {
        void *payload = message->payload;
        size_t payloadlen = message->payloadlen;
        char *pl = (char *) payload;

        pl[payloadlen] = '\0';
        VEIDES_LOG_DEBUG("Invoke callback to process message (topic=%s)", topicName);
        (*cb)(context, topicName, topicLen, payload, payloadlen);
    } else {
        VEIDES_LOG_DEBUG("No registered callback function is found to process the arrived message.");
    }

    if (rc == VEIDES_RC_SUCCESS) {
        return MQTTASYNC_TRUE;
    }

    return rc;
}

static int veides_properties_validate(VeidesStreamHubClientProperties *properties) {
    VEIDES_RC rc = VEIDES_RC_SUCCESS;

    if (!properties) {
        rc = VEIDES_RC_NULL_PARAM;
        VEIDES_LOG_ERROR("Invalid properties handle (rc=%d)", rc);
        return rc;
    }

    if (properties->authProperties == NULL || properties->connectionProperties == NULL) {
        rc = VEIDES_RC_NULL_PARAM;
        VEIDES_LOG_ERROR("Invalid auth or connection properties provided (rc=%d)", rc);
        return rc;
    }

    char *clientHost = NULL;
    clientHost = properties->connectionProperties->host;

    int port = properties->connectionProperties->port;

    if (clientHost == NULL || *clientHost == '\0' || !port) {
        rc = VEIDES_RC_NULL_PARAM;
        VEIDES_LOG_ERROR("Invalid connection properties provided (rc=%d)", rc);
        return rc;
    }

    char *username = NULL;
    char *token = NULL;

    username = properties->authProperties->username;
    token = properties->authProperties->token;

    if (username == NULL || *username == '\0'
        || token == NULL || *token == '\0'
    ) {
        rc = VEIDES_RC_NULL_PARAM;
        VEIDES_LOG_ERROR("Invalid auth properties provided (rc=%d)", rc);
        return rc;
    }

    return rc;
}