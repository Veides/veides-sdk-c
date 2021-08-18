#include <stdio.h>
#include <signal.h>
#include <memory.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>

#include "veides_stream_hub_client/veides_stream_hub_client.h"

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
        "Usage: streamHubBasic [-h] -u <username> -t <token> -i <client_id> -H <host>\n\n"

        "Basic example of interacting with Veides Stream Hub\n\n"

        "Options:\n"
        "  -u <username>     User's name\n"
        "  -t <token>        User's token\n"
        "  -i <client_id>    Agent's client id\n"
        "  -H <host>         Hostname used to connect to Veides Stream Hub\n\n"

        "  -h, --help        Displays this help.\n"
    );
}

void onUptimeTrail(VeidesStreamHubClient *client, char *agent, VeidesTrail trail) {
    log_message(stdout, "INFO: Trail received: (name=%s, value=%s, timestamp=%s)", trail.name, trail.valueString, trail.timestamp);
}

int main(int argc, char *argv[]) {
    char *host = NULL;
    char *username = NULL;
    char *token = NULL;
    char *clientId = NULL;

    for (int optind = 1; optind < argc && argv[optind][0] == '-'; optind++) {
        switch (argv[optind][1]) {
        case 'u': username = argv[optind + 1]; optind++; break;
        case 't': token = argv[optind + 1]; optind++; break;
        case 'i': clientId = argv[optind + 1]; optind++; break;
        case 'H': host = argv[optind + 1]; optind++; break;
        case 'h': printHelp(); exit(0);
        default:
            printHelp();
            exit(0);
        }
    }

    int rc = 0;

    VeidesStreamHubClientProperties *properties = NULL;
    VeidesStreamHubClient *client = NULL;

    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);

    // Set SDK log handler
    rc = VeidesSDK_setLogHandler(&sdkLogCallback);
    if (rc != VEIDES_RC_SUCCESS) {
        log_message(stderr, "WARN: Failed to set SDK log handler (rc=%d)", rc);
    }

    // Set DEBUG level to see received and sent data. Level is INFO by default
    VeidesSDK_setLogLevel(VEIDES_LOG_DEBUG);

    rc = VeidesStreamHubClientProperties_create(&properties);
    if (rc != VEIDES_RC_SUCCESS) {
        log_message(stderr, "ERROR: Failed to initialize properties (rc=%d)", rc);
        exit(1);
    }

    // Set required properties
    VeidesStreamHubClientProperties_setProperty(properties, "client.host", host);
    VeidesStreamHubClientProperties_setProperty(properties, "auth.user.name", username);
    VeidesStreamHubClientProperties_setProperty(properties, "auth.token", token);

    // Properties may also be provided in environment variables
    // VeidesStreamHubClientProperties_setPropertiesFromEnv(properties);

    rc = VeidesStreamHubClient_create(&client, properties);
    if (rc != VEIDES_RC_SUCCESS) {
        log_message(stderr, "ERROR: Failed to initialize Veides Stream Hub client (rc=%d)", rc);
        exit(1);
    }

    rc = VeidesStreamHubClient_connect(client);
    if (rc != VEIDES_RC_SUCCESS) {
        log_message(stderr, "ERROR: Failed to connect to Veides Stream Hub (rc=%d)", rc);
        exit(1);
    }

    VeidesStreamHubClient_setTrailHandler(client, clientId, "uptime", onUptimeTrail);

    while (!finish) {
        sleep(1);
    }

    rc = VeidesStreamHubClient_disconnect(client);
    if (rc != VEIDES_RC_SUCCESS) {
        log_message(stderr, "ERROR: Failed to disconnect from Veides Stream Hub (rc=%d)", rc);
        exit(1);
    }

    // Clear Veides Stream Hub client
    VeidesStreamHubClient_destroy(client);

    // Clear Veides Stream Hub client properties
    VeidesStreamHubClientProperties_destroy(properties);

    return 0;
}

