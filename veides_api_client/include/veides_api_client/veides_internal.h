#if !defined(VEIDES_API_INTERNAL_H)
#define VEIDES_API_INTERNAL_H

#if defined(__cplusplus)
 extern "C" {
#endif

#include <ctype.h>

#include "veides_api_client/veides_properties.h"
#include "veides_api_client/veides_base_client.h"

#include "veides_utils_shared/veides_rc.h"
#include "veides_utils_shared/veides_logger.h"
#include "veides_utils_shared/veides_utils.h"

typedef struct VeidesApiClientInternal {
    VeidesApiClientProperties *properties;
    void                      *curl;
    char                      *version;
} VeidesApiClientInternal;

typedef struct VeidesApiResponse {
    char *data;
    long code;
} VeidesApiResponse;

VEIDES_RC veides_api_client_create(void **veidesClient, VeidesApiClientProperties *properties);
VEIDES_RC veides_api_client_destroy(void *veidesClient);
VEIDES_RC veides_api_client_build_query(char **query, VeidesApiQueryParams *params);
VEIDES_RC veides_api_client_sendRequest(void *veidesClient, const char *uri, const char *method, const char *body, VeidesApiQueryParams *params, int *responseCode, char **responsePayload, int *responsePayloadLen);

#if defined(__cplusplus)
 }
#endif

#endif /* VEIDES_API_INTERNAL_H */
