#include <curl/curl.h>
#include <math.h>

#include "veides_api_client/veides_internal.h"
#include "veides_api_client/veides_base_client.h"
#include "veides_utils_shared/veides_logger_p.h"
#include "veides_version.h"

static int veides_properties_validate(VeidesApiClientProperties *properties);

static struct curl_slist *baseHeaders = NULL;

static char *VEIDES_USER_AGENT_HEADER_FORMAT = "User-Agent: Veides-SDK-ApiClient/%s/%s/C";
static char *VEIDES_AUTH_HEADER_FORMAT = "Authorization: Token %s";
static char *VEIDES_CONTENT_TYPE_HEADER = "Content-Type: application/json";
static char *VEIDES_ACCEPT_HEADER = "Accept: application/json";

static size_t write_data(void *buffer, size_t size, size_t nmemb, VeidesApiResponse *response)
{
    int newSize = strlen(response->data) + (size * nmemb);
    char *newBuffer = (char *) malloc(newSize);

    if (newBuffer == NULL) {
        return 0;
    }

    strcpy(newBuffer, response->data);
    strncat(newBuffer, buffer, size * nmemb);

    response->data = newBuffer;

    return size * nmemb;
}

VEIDES_RC veides_api_client_create(void **veidesClient, VeidesApiClientProperties *properties) {
    VEIDES_LOG_INFO("Veides API Client - Version: %s", VEIDES_SDK_VERSION);

    VEIDES_RC rc = VEIDES_RC_SUCCESS;

    if (!veidesClient) {
        rc = VEIDES_RC_INVALID_HANDLE;
        VEIDES_LOG_ERROR("Invalid API client handle provided (rc=%d)", rc);
        return rc;
    }

    if (*veidesClient != NULL) {
        rc = VEIDES_RC_INVALID_PARAM_VALUE;
        VEIDES_LOG_ERROR("API client handle is already created");
        return rc;
    }

    rc = veides_properties_validate(properties);

    if (rc != VEIDES_RC_SUCCESS) {
        return rc;
    }

    char *baseUrl = properties->configurationProperties->baseUrl;

    VeidesApiClientInternal *client = (VeidesApiClientInternal *) malloc(sizeof(VeidesApiClientInternal));
    client->properties = properties;
    client->version = "v1";

    curl_global_init(CURL_GLOBAL_ALL);

    int size = strlen(VEIDES_USER_AGENT_HEADER_FORMAT) + strlen(VEIDES_SDK_VERSION);
    char userAgent[size];
    snprintf(userAgent, size, VEIDES_USER_AGENT_HEADER_FORMAT, client->version, VEIDES_SDK_VERSION);

    baseHeaders = curl_slist_append(baseHeaders, userAgent);

    size = strlen(VEIDES_AUTH_HEADER_FORMAT) + strlen(client->properties->authProperties->token);
    char auth[size];
    snprintf(auth, size, VEIDES_AUTH_HEADER_FORMAT, client->properties->authProperties->token);

    baseHeaders = curl_slist_append(baseHeaders, auth);

    baseHeaders = curl_slist_append(baseHeaders, VEIDES_CONTENT_TYPE_HEADER);
    baseHeaders = curl_slist_append(baseHeaders, VEIDES_ACCEPT_HEADER);

    *veidesClient = client;

    return rc;
}

VEIDES_RC veides_api_client_destroy(void *veidesClient) {
    VEIDES_RC rc = VEIDES_RC_SUCCESS;
    VeidesApiClientInternal *client = (VeidesApiClientInternal *) veidesClient;
    int i = 0;

    if (!client) {
        rc = VEIDES_RC_INVALID_HANDLE;
        VEIDES_LOG_ERROR("Invalid or NULL API client handle provided (rc=%d)", rc);
        return rc;
    }
    client->properties = NULL;
    veides_utils_freePtr((void *) client);
    curl_global_cleanup();
    client = NULL;

    return rc;
}

VEIDES_RC veides_api_client_build_query(char **query, VeidesApiQueryParams *params) {
    VEIDES_RC rc = VEIDES_RC_SUCCESS;

    if (params == NULL) {
        return rc;
    }

    *query[0] = '?';

    if (params->timeout) {
        int timeoutLen = (int) log10(params->timeout) + 1;

        char *timeoutFormat = "timeout=%d";
        int s = strlen(timeoutFormat) - 2 + timeoutLen + 1;
        char buf[s];
        snprintf(buf, s, timeoutFormat, params->timeout);

        strncat(*query, buf, strlen(buf));
    }

    return rc;
}

VEIDES_RC veides_api_client_sendRequest(void *veidesClient, const char *uri, const char *method, const char *body, VeidesApiQueryParams *params, int *responseCode, char **responsePayload, int *responsePayloadLen) {
    VEIDES_RC rc = VEIDES_RC_SUCCESS;

    VeidesApiClientInternal *client = (VeidesApiClientInternal *) veidesClient;

    if (!client) {
        rc = VEIDES_RC_INVALID_HANDLE;
        VEIDES_LOG_ERROR("Invalid API client handle provided (rc=%d)", rc);
        return rc;
    }

    if (responseCode == NULL || responsePayload == NULL || responsePayloadLen == NULL) {
        rc = VEIDES_RC_NULL_PARAM;
        VEIDES_LOG_WARNING("Invalid responseCode, responsePayload or responsePayloadLen provided (rc=%d)", rc);
        return rc;
    }

    CURL *curl;

    curl = curl_easy_init();

    if (!curl) {
        return VEIDES_RC_FAILURE;
    }

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 1L);

    char *baseUrl = client->properties->configurationProperties->baseUrl;

    char *format = "%s/%s%s%s";
    char *queryParams = (char*) calloc(8, sizeof(char*));

    veides_api_client_build_query(&queryParams, params);

    int len = strlen(format) - 8 + strlen(baseUrl) + strlen(client->version) + strlen(uri) + strlen(queryParams) + 1;
    char url[len];

    snprintf(url, len, format, baseUrl, client->version, uri, queryParams);

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method);

    VeidesApiResponse *response = (VeidesApiResponse *) malloc(sizeof(VeidesApiResponse));
    response->data = "";

    curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);

    if (body && *body != '\0') {
        VEIDES_LOG_DEBUG("Setting request body: %s", body);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);
    }

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, baseHeaders);

    CURLcode res = curl_easy_perform(curl);

    if(res != CURLE_OK) {
        VEIDES_LOG_ERROR("Error while performing request by CURL (rc=%d)", res);
        rc = VEIDES_RC_FAILURE;
        goto sendRequestEnd;
    }

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response->code);

    curl_easy_cleanup(curl);

    VEIDES_LOG_DEBUG("Got response (body=%s, code=%d)", response->data, response->code);

    switch (response->code) {
        case 403:
            rc = VEIDES_RC_FORBIDDEN;
            break;

        case 404:
            rc = VEIDES_RC_NOT_FOUND;
            break;

        case 504:
            rc = VEIDES_RC_METHOD_INVOKE_TIMEOUT;
            break;
    
        default:
            *responseCode = response->code;
            *responsePayload = response->data;
            *responsePayloadLen = strlen(response->data);
            break;
    }

sendRequestEnd:
    return rc;
}

static int veides_properties_validate(VeidesApiClientProperties *properties) {
    VEIDES_RC rc = VEIDES_RC_SUCCESS;

    if (!properties) {
        rc = VEIDES_RC_NULL_PARAM;
        VEIDES_LOG_ERROR("Invalid properties handle (rc=%d)", rc);
        return rc;
    }

    if (properties->authProperties == NULL || properties->configurationProperties == NULL) {
        rc = VEIDES_RC_NULL_PARAM;
        VEIDES_LOG_ERROR("Invalid auth or configuration properties provided (rc=%d)", rc);
        return rc;
    }

    char *baseUrl = properties->configurationProperties->baseUrl;

    if (baseUrl == NULL) {
        rc = VEIDES_RC_NULL_PARAM;
        VEIDES_LOG_ERROR("Invalid configuration properties provided (rc=%d)", rc);
        return rc;
    }

    char *token = properties->authProperties->token;

    if (token == NULL || *token == '\0') {
        rc = VEIDES_RC_NULL_PARAM;
        VEIDES_LOG_ERROR("Invalid auth properties provided (rc=%d)", rc);
        return rc;
    }

    return rc;
}
