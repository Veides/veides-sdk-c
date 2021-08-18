#if !defined(VEIDES_STREAM_HUB_BASE_CLIENT_H)
#define VEIDES_STREAM_HUB_BASE_CLIENT_H

#if defined(__cplusplus)
 extern "C" {
#endif

typedef void *VeidesStreamHubClient;

typedef enum {
    VEIDES_TRAIL_VALUE_STRING = 0,
    VEIDES_TRAIL_VALUE_BOOL = 1,
    VEIDES_TRAIL_VALUE_DOUBLE = 2,
    VEIDES_TRAIL_VALUE_INT = 3,
} VEIDES_TRAIL_VALUE_TYPE;

typedef struct {
    char *name;
    char *timestamp;
    int valueType;

    char *valueString;
    double valueDouble;
    int valueInt;
    int valueBool;
} VeidesTrail;

typedef void (*VeidesTrailCallbackHandler)(VeidesStreamHubClient *client, char *agent, VeidesTrail trail);

#if defined(__cplusplus)
 }
#endif

#endif /* VEIDES_STREAM_HUB_BASE_CLIENT_H */