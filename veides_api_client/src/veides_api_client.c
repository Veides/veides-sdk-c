#include "veides_api_client/veides_api_client.h"
#include "veides_api_client/veides_base_client.h"
#include "veides_api_client/veides_internal.h"
#include "veides_utils_shared/veides_logger_p.h"

VEIDES_RC VeidesApiClient_create(VeidesApiClient **client, VeidesApiClientProperties *properties)
{
    VEIDES_RC rc = VEIDES_RC_SUCCESS;

    rc = veides_api_client_create((void **) client, properties);
    if (rc != VEIDES_RC_SUCCESS) {
        VEIDES_LOG_ERROR("Failed to create API client handle (rc=%d)", rc);
    }

    return rc;
}

VEIDES_RC VeidesApiClient_destroy(VeidesApiClient *client)
{
    VEIDES_RC rc = VEIDES_RC_SUCCESS;

    rc = veides_api_client_destroy((void *) client);
    if (rc != VEIDES_RC_SUCCESS) {
        VEIDES_LOG_ERROR("Failed to destroy handle (rc=%d)", rc);
    }

    return rc;
}

VEIDES_RC VeidesApiClient_invokeMethod(VeidesApiClient *client, const char *agent, const char *name, const char *payload, int timeout, int *responseCode, char **responsePayload, int *responsePayloadLen)
{
    VEIDES_RC rc = VEIDES_RC_SUCCESS;

    if (!name || *name == '\0') {
        rc = VEIDES_RC_NULL_PARAM;
        VEIDES_LOG_WARNING("Invalid name provided (rc=%d)", rc);
        return rc;
    }

    if (!payload || *payload == '\0') {
        rc = VEIDES_RC_NULL_PARAM;
        VEIDES_LOG_WARNING("Invalid payload provided (rc=%d)", rc);
        return rc;
    }

    if (!timeout) {
        rc = VEIDES_RC_NULL_PARAM;
        VEIDES_LOG_WARNING("Invalid timeout provided (rc=%d)", rc);
        return rc;
    }

    if (timeout < 1000 || timeout > 30000) {
        rc = VEIDES_RC_INVALID_PARAM_VALUE;
        VEIDES_LOG_WARNING("Invalid timeout value provided (rc=%d)", rc);
        return rc;
    }

    char *format = "/agents/%s/methods/%s";

    int size = strlen(format) - 4 + strlen(agent) + strlen(name) + 1;
    char url[size];
    snprintf(url, size, format, agent, name);

    VeidesApiQueryParams *params = (VeidesApiQueryParams *) malloc(sizeof(VeidesApiQueryParams));
    params->timeout = timeout;

    rc = veides_api_client_sendRequest((void **) client, url, "POST", payload, params, responseCode, responsePayload, responsePayloadLen);
    if (rc != VEIDES_RC_SUCCESS) {
        VEIDES_LOG_ERROR("Invoke method on an agent failed (rc=%d, agent=%s)", rc, agent);
    }

    return rc;
}
