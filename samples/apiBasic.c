#include <stdio.h>
#include <signal.h>
#include <memory.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>

#include "veides_api_client/veides_api_client.h"

#define MAX_LOG_BUFSIZE 8192

volatile int finish = 0;

void log_message(void *target, const char *items, ...) {
    va_list args;

    char buffer[MAX_LOG_BUFSIZE];
    memset(buffer, '0', MAX_LOG_BUFSIZE);

    va_start(args, items);
    vsnprintf(buffer, MAX_LOG_BUFSIZE, items, args);
    va_end(args);

    fprintf(target, "%s\n", buffer);
    fflush(target);
}

void sdkLogCallback(int level, char *message) {
    fprintf(stdout, "%s", message);
    fflush(stdout);
}

void sigHandler(int signalNumber) {
    signal(SIGINT, NULL);
    log_message(stdout, "INFO: Received signal: %d", signalNumber);
    finish = 1;
}

void printHelp() {
    printf(
        "Usage: apiBasic [-h] -i <client_id> -u <base_url> -t <token>\n\n"

        "Basic example of interacting with Veides API\n\n"

        "Options:\n"
        "  -u <base_url>     Veides API url\n"
        "  -t <token>        User's token\n"
        "  -i <client_id>    Agent's client id\n\n"

        "  -h, --help        Displays this help.\n"
    );
}

int main(int argc, char *argv[]) {
    char *baseUrl = NULL;
    char *token = NULL;
    char *clientId = NULL;

    for (int optind = 1; optind < argc && argv[optind][0] == '-'; optind++) {
        switch (argv[optind][1]) {
        case 'u': baseUrl = argv[optind + 1]; optind++; break;
        case 't': token = argv[optind + 1]; optind++; break;
        case 'i': clientId = argv[optind + 1]; optind++; break;
        case 'h': printHelp(); exit(0);
        default:
            printHelp();
            exit(0);
        }
    }

    int rc = 0;

    VeidesApiClientProperties *properties = NULL;
    VeidesApiClient *client = NULL;

    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);

    // Set SDK log handler
    rc = VeidesSDK_setLogHandler(&sdkLogCallback);
    if (rc != VEIDES_RC_SUCCESS) {
        log_message(stderr, "WARN: Failed to set SDK log handler (rc=%d)", rc);
    }

    // Set DEBUG level to see received and sent data. Level is INFO by default
    VeidesSDK_setLogLevel(VEIDES_LOG_DEBUG);

    rc = VeidesApiClientProperties_create(&properties);
    if (rc != VEIDES_RC_SUCCESS) {
        log_message(stderr, "ERROR: Failed to initialize properties (rc=%d)", rc);
        exit(1);
    }

    // Set required properties
    VeidesApiClientProperties_setProperty(properties, "api.base.url", baseUrl);
    VeidesApiClientProperties_setProperty(properties, "auth.token", token);

    // Properties may also be provided in environment variables
    // VeidesApiClientProperties_setPropertiesFromEnv(properties);

    rc = VeidesApiClient_create(&client, properties);
    if (rc != VEIDES_RC_SUCCESS) {
        log_message(stderr, "ERROR: Failed to initialize Veides API client (rc=%d)", rc);
        exit(1);
    }

    int responseCode;
    char *responsePayload;
    int responsePayloadLen;

    char *payload = "{\"time\": \"now\"}";

    rc = VeidesApiClient_invokeMethod(client, clientId, "shutdown", payload, 1000, &responseCode, &responsePayload, &responsePayloadLen);
    if (rc != VEIDES_RC_SUCCESS) {
        log_message(stderr, "ERROR: Failed to invoke agent method (rc=%d)", rc);
        exit(1);
    }
    
    log_message(stdout, "Method response (code=%d, payload=%s, payloadLen=%d)", responseCode, responsePayload, responsePayloadLen);

    // Clear Veides API client
    VeidesApiClient_destroy(client);

    // Clear Veides API client properties
    VeidesApiClientProperties_destroy(properties);

    return 0;
}

