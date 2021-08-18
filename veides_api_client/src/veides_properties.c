#include "veides_api_client/veides_properties.h"
#include "veides_api_client/veides_internal.h"
#include "veides_utils_shared/veides_logger_p.h"

VEIDES_RC VeidesApiClientProperties_create(VeidesApiClientProperties **properties) {
    VEIDES_RC rc = VEIDES_RC_SUCCESS;

    if (!properties) {
        rc = VEIDES_RC_INVALID_HANDLE;
        VEIDES_LOG_ERROR("Invalid properties handle provided (rc=%d)", rc);
        return rc;
    }
    if (*properties != NULL) {
        rc = VEIDES_RC_INVALID_HANDLE;
        VEIDES_LOG_ERROR("Properties object is already initialized (rc=%d)", rc);
        return rc;
    }

    *properties = (VeidesApiClientProperties *) calloc(1, sizeof(VeidesApiClientProperties));
    if (*properties == NULL) {
        rc = VEIDES_RC_NOMEM;
        return rc;
    }

    VeidesApiAuthProperties *authProperties = (VeidesApiAuthProperties *) calloc(1, sizeof(VeidesApiAuthProperties));
    if (authProperties == NULL) {
        rc = VEIDES_RC_NOMEM;
        return rc;
    }

    VeidesConfigurationProperties *configurationProperties = (VeidesConfigurationProperties *) calloc(1, sizeof(VeidesConfigurationProperties));
    if (configurationProperties == NULL) {
        rc = VEIDES_RC_NOMEM;
        return rc;
    }

    (*properties)->authProperties = authProperties;
    (*properties)->configurationProperties = configurationProperties;

    return rc;
}


VEIDES_RC VeidesApiClientProperties_destroy(VeidesApiClientProperties *properties)
{
    VEIDES_RC rc = VEIDES_RC_SUCCESS;

    if (!properties) {
        rc = VEIDES_RC_INVALID_HANDLE;
        VEIDES_LOG_ERROR("Invalid properties handle provided (rc=%d)", rc);
    }

    if (properties->authProperties != NULL) {
        VeidesApiAuthProperties *authProperties = properties->authProperties;
        veides_utils_freePtr((void *) authProperties->token);
    }
    if (properties->configurationProperties != NULL) {
        VeidesConfigurationProperties *configurationProperties = properties->configurationProperties;
        veides_utils_freePtr((void *) configurationProperties->baseUrl);
    }

    veides_utils_freePtr(properties);

    return rc;
}


VEIDES_RC VeidesApiClientProperties_setProperty(VeidesApiClientProperties *properties, const char *name, const char *value) {
    VEIDES_RC rc = VEIDES_RC_SUCCESS;

    if (!properties) {
        VEIDES_LOG_ERROR("Invalid properties handle provided (rc=%d)", rc);
        return VEIDES_RC_INVALID_HANDLE;
    }

    if (!name || *name == '\0') {
        rc = VEIDES_RC_NULL_PARAM;
        goto setPropertyEnd;
    }

    if (!value || *value == '\0') {
        rc = VEIDES_RC_NULL_PARAM;
        goto setPropertyEnd;
    }

    if (strcmp(name, "auth.token") == 0) {
        if (properties->authProperties->token)
            veides_utils_freePtr((void *) properties->authProperties->token);
        properties->authProperties->token = strdup(value);

        rc = VEIDES_RC_SUCCESS;
    } else if (strcmp(name, "api.base.url") == 0) {
        if (properties->configurationProperties->baseUrl)
            veides_utils_freePtr((void *) properties->configurationProperties->baseUrl);
        properties->configurationProperties->baseUrl = strdup(value);

        rc = VEIDES_RC_SUCCESS;
    } else {
        rc = VEIDES_RC_INVALID_PROPERTY;
    }

setPropertyEnd:
    if (rc == VEIDES_RC_INVALID_PROPERTY) {
        VEIDES_LOG_INFO("Property %s could not be recognized", name);
    } else if (rc != VEIDES_RC_SUCCESS) {
        VEIDES_LOG_ERROR("Invalid value for property %s provided (rc=%d)", name, rc);
    } else {
        VEIDES_LOG_DEBUG("Property %s set to %s", name, value);
    }

    return rc;
}

VEIDES_RC VeidesApiClientProperties_setPropertiesFromEnv(VeidesApiClientProperties *properties)
{
    VEIDES_RC rc = VEIDES_RC_SUCCESS;
    char *env = *environ;
    int index = 1;
    int invalidProperties = 0;
    int prefixlen = 7;

    if (!properties) {
        rc = VEIDES_RC_INVALID_HANDLE;
        VEIDES_LOG_ERROR("Invalid properties handle provided (rc=%d)", rc);
        return rc;
    }

    VEIDES_LOG_INFO("Setting properties from environment variables");

    for (; env; index++)
    {
        char *envval = strdup(env);
        char *prop = strtok(envval, "=");
        char *value = strtok(NULL, "=");

        if (prop && *prop != '\0' && !strncasecmp(prop, "VEIDES_", prefixlen) && value && *value != '\0')
        {
            char *name = NULL;
            char *p = NULL;

            p = prop;
            for (; *p; ++p) {
                if (*p == '_') {
                    *p = '.';
                } else {
                    *p = tolower(*p);
                }
            }

            name = prop + prefixlen;
            VEIDES_RC setRc = VeidesApiClientProperties_setProperty(properties, name, value);

            if (setRc == VEIDES_RC_INVALID_PROPERTY) {
                VEIDES_LOG_INFO("Environment variable %s could not be recognized", prop);
            } else if (setRc != VEIDES_RC_SUCCESS) {
                invalidProperties++;
                VEIDES_LOG_ERROR("Invalid value for environment variable %s provided (rc=%d)", prop, setRc);
            }
        }

        veides_utils_freePtr((void *) envval);

        env = *(environ + index);
    }

    if (invalidProperties > 0) {
        VEIDES_LOG_WARNING("Found %d INVALID Veides properties", invalidProperties);
    }

    return rc;
}
