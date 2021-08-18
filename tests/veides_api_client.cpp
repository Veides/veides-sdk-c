#include "gtest/gtest.h"
#include "veides_api_client/veides_api_client.h"
#include "veides_api_client/veides_internal.h"

TEST(veides_api_client, should_return_error_when_null_handle_provided)
{
    VeidesApiClientProperties *properties = NULL;

    VEIDES_RC rc = VeidesApiClient_create(NULL, properties);

    EXPECT_EQ(rc, VEIDES_RC_INVALID_HANDLE);
}

TEST(veides_api_client, should_return_error_when_null_properties_provided)
{
    VeidesApiClientProperties *properties = NULL;
    VeidesApiClient *client = NULL;

    VEIDES_RC rc = VeidesApiClient_create(&client, NULL);

    EXPECT_EQ(rc, VEIDES_RC_NULL_PARAM);

    rc = VeidesApiClientProperties_create(&properties);

    EXPECT_EQ(rc, VEIDES_RC_SUCCESS);

    rc = VeidesApiClient_create(&client, properties);

    EXPECT_EQ(rc, VEIDES_RC_NULL_PARAM);
}

TEST(veides_api_client, should_return_error_when_client_properties_not_provided)
{
    VeidesApiClientProperties *properties = NULL;
    VeidesApiClient *client = NULL;

    VEIDES_RC rc = VeidesApiClientProperties_create(&properties);

    EXPECT_EQ(rc, VEIDES_RC_SUCCESS);

    VeidesApiClientProperties_setProperty(properties, "api.base.url", "https://localhost");

    rc = VeidesApiClient_create(&client, properties);

    EXPECT_EQ(rc, VEIDES_RC_NULL_PARAM);
}

TEST(veides_api_client, should_return_error_when_invalid_base_url_provided)
{
    VeidesApiClientProperties *properties = NULL;
    VeidesApiClient *client = NULL;

    VEIDES_RC rc = VeidesApiClientProperties_create(&properties);

    EXPECT_EQ(rc, VEIDES_RC_SUCCESS);

    VeidesApiClientProperties_setProperty(properties, "auth.token", "token");

    rc = VeidesApiClient_create(&client, properties);

    EXPECT_EQ(rc, VEIDES_RC_NULL_PARAM);

    VeidesApiClientProperties_setProperty(properties, "api.base.url", "");

    rc = VeidesApiClient_create(&client, properties);

    EXPECT_EQ(rc, VEIDES_RC_NULL_PARAM);
}

TEST(veides_api_client, should_return_error_when_invalid_auth_token_provided)
{
    VeidesApiClientProperties *properties = NULL;
    VeidesApiClient *client = NULL;

    VEIDES_RC rc = VeidesApiClientProperties_create(&properties);

    EXPECT_EQ(rc, VEIDES_RC_SUCCESS);

    VeidesApiClientProperties_setProperty(properties, "api.base.url", "https://localhost");

    rc = VeidesApiClient_create(&client, properties);

    EXPECT_EQ(rc, VEIDES_RC_NULL_PARAM);

    VeidesApiClientProperties_setProperty(properties, "auth.token", "");

    rc = VeidesApiClient_create(&client, properties);

    EXPECT_EQ(rc, VEIDES_RC_NULL_PARAM);
}

TEST(veides_api_client, should_return_success_when_valid_properties_provided)
{
    VeidesApiClientProperties *properties = NULL;
    VeidesApiClient *client = NULL;

    VEIDES_RC rc = VeidesApiClientProperties_create(&properties);

    EXPECT_EQ(rc, VEIDES_RC_SUCCESS);

    VeidesApiClientProperties_setProperty(properties, "api.base.url", "https://localhost");
    VeidesApiClientProperties_setProperty(properties, "auth.token", "token");

    rc = VeidesApiClient_create(&client, properties);

    EXPECT_EQ(rc, VEIDES_RC_SUCCESS);
}
