#include "gtest/gtest.h"
#include "veides_stream_hub_client/veides_stream_hub_client.h"
#include "veides_stream_hub_client/veides_internal.h"
#include "veides_utils_shared/veides_utils.h"

TEST(veides_stream_hub_client_properties, should_return_error_when_null_handle_provided)
{
    VEIDES_RC rc = VeidesStreamHubClientProperties_create(NULL);

    EXPECT_EQ(rc, VEIDES_RC_INVALID_HANDLE);
}

TEST(veides_stream_hub_client_properties, should_return_error_when_empty_value_provided)
{
    VeidesStreamHubClientProperties *properties = NULL;

    VEIDES_RC rc = VeidesStreamHubClientProperties_create(&properties);

    EXPECT_EQ(rc, VEIDES_RC_SUCCESS);

    rc = VeidesStreamHubClientProperties_setProperty(properties, "client.host", "");

    EXPECT_EQ(rc, VEIDES_RC_NULL_PARAM);
}

TEST(veides_stream_hub_client_properties, should_return_error_when_unknown_name_provided)
{
    VeidesStreamHubClientProperties *properties = NULL;

    VEIDES_RC rc = VeidesStreamHubClientProperties_create(&properties);

    EXPECT_EQ(rc, VEIDES_RC_SUCCESS);

    rc = VeidesStreamHubClientProperties_setProperty(properties, "unknown.property", "value");

    EXPECT_EQ(rc, VEIDES_RC_INVALID_PROPERTY);
}

TEST(veides_stream_hub_client_properties, should_return_success_when_valid_value_provided)
{
    VeidesStreamHubClientProperties *properties = NULL;

    VEIDES_RC rc = VeidesStreamHubClientProperties_create(&properties);

    EXPECT_EQ(rc, VEIDES_RC_SUCCESS);

    rc = VeidesStreamHubClientProperties_setProperty(properties, "client.host", "host");

    EXPECT_EQ(rc, VEIDES_RC_SUCCESS);

    EXPECT_EQ(strcmp(properties->connectionProperties->host, "host"), 0);
}

TEST(veides_stream_hub_client_properties, should_return_success)
{
    VeidesStreamHubClientProperties *properties = NULL;

    VEIDES_RC rc = VeidesStreamHubClientProperties_create(&properties);

    EXPECT_EQ(rc, VEIDES_RC_SUCCESS);
}
