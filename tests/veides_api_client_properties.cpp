#include "gtest/gtest.h"
#include "veides_api_client/veides_api_client.h"
#include "veides_api_client/veides_internal.h"
#include "veides_utils_shared/veides_utils.h"

TEST(veides_api_client_properties, should_return_error_when_null_handle_provided)
{
    VEIDES_RC rc = VeidesApiClientProperties_create(NULL);

    EXPECT_EQ(rc, VEIDES_RC_INVALID_HANDLE);
}

TEST(veides_api_client_properties, should_return_error_when_empty_value_provided)
{
    VeidesApiClientProperties *properties = NULL;

    VEIDES_RC rc = VeidesApiClientProperties_create(&properties);

    EXPECT_EQ(rc, VEIDES_RC_SUCCESS);

    rc = VeidesApiClientProperties_setProperty(properties, "api.base.url", "");

    EXPECT_EQ(rc, VEIDES_RC_NULL_PARAM);
}

TEST(veides_api_client_properties, should_return_error_when_unknown_name_provided)
{
    VeidesApiClientProperties *properties = NULL;

    VEIDES_RC rc = VeidesApiClientProperties_create(&properties);

    EXPECT_EQ(rc, VEIDES_RC_SUCCESS);

    rc = VeidesApiClientProperties_setProperty(properties, "unknown.property", "value");

    EXPECT_EQ(rc, VEIDES_RC_INVALID_PROPERTY);
}

TEST(veides_api_client_properties, should_return_success_when_valid_value_provided)
{
    VeidesApiClientProperties *properties = NULL;

    VEIDES_RC rc = VeidesApiClientProperties_create(&properties);

    EXPECT_EQ(rc, VEIDES_RC_SUCCESS);

    rc = VeidesApiClientProperties_setProperty(properties, "api.base.url", "https://localhost");

    EXPECT_EQ(rc, VEIDES_RC_SUCCESS);

    EXPECT_EQ(strcmp(properties->configurationProperties->baseUrl, "https://localhost"), 0);
}

TEST(veides_api_client_properties, should_return_success)
{
    VeidesApiClientProperties *properties = NULL;

    VEIDES_RC rc = VeidesApiClientProperties_create(&properties);

    EXPECT_EQ(rc, VEIDES_RC_SUCCESS);
}
