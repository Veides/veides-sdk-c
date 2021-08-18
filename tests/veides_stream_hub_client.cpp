#include "gtest/gtest.h"
#include "veides_stream_hub_client/veides_stream_hub_client.h"
#include "veides_stream_hub_client/veides_internal.h"

TEST(veides_stream_hub_client, should_return_error_when_null_handle_provided)
{
    VeidesStreamHubClientProperties *properties = NULL;

    VEIDES_RC rc = VeidesStreamHubClient_create(NULL, properties);

    EXPECT_EQ(rc, VEIDES_RC_INVALID_HANDLE);
}

TEST(veides_stream_hub_client, should_return_error_when_null_properties_provided)
{
    VeidesStreamHubClientProperties *properties = NULL;
    VeidesStreamHubClient *client = NULL;

    VEIDES_RC rc = VeidesStreamHubClient_create(&client, NULL);

    EXPECT_EQ(rc, VEIDES_RC_NULL_PARAM);

    rc = VeidesStreamHubClientProperties_create(&properties);

    EXPECT_EQ(rc, VEIDES_RC_SUCCESS);

    rc = VeidesStreamHubClient_create(&client, properties);

    EXPECT_EQ(rc, VEIDES_RC_NULL_PARAM);
}

TEST(veides_stream_hub_client, should_return_error_when_client_properties_not_provided)
{
    VeidesStreamHubClientProperties *properties = NULL;
    VeidesStreamHubClient *client = NULL;

    VEIDES_RC rc = VeidesStreamHubClientProperties_create(&properties);

    EXPECT_EQ(rc, VEIDES_RC_SUCCESS);

    VeidesStreamHubClientProperties_setProperty(properties, "client.host", "host");

    rc = VeidesStreamHubClient_create(&client, properties);

    EXPECT_EQ(rc, VEIDES_RC_NULL_PARAM);
}

TEST(veides_stream_hub_client, should_return_error_when_invalid_client_host_provided)
{
    VeidesStreamHubClientProperties *properties = NULL;
    VeidesStreamHubClient *client = NULL;

    VEIDES_RC rc = VeidesStreamHubClientProperties_create(&properties);

    EXPECT_EQ(rc, VEIDES_RC_SUCCESS);

    VeidesStreamHubClientProperties_setProperty(properties, "auth.user.name", "name");
    VeidesStreamHubClientProperties_setProperty(properties, "auth.user.token", "token");

    rc = VeidesStreamHubClient_create(&client, properties);

    EXPECT_EQ(rc, VEIDES_RC_NULL_PARAM);

    VeidesStreamHubClientProperties_setProperty(properties, "client.host", "");

    rc = VeidesStreamHubClient_create(&client, properties);

    EXPECT_EQ(rc, VEIDES_RC_NULL_PARAM);
}

TEST(veides_stream_hub_client, should_return_error_when_invalid_user_name_provided)
{
    VeidesStreamHubClientProperties *properties = NULL;
    VeidesStreamHubClient *client = NULL;

    VEIDES_RC rc = VeidesStreamHubClientProperties_create(&properties);

    EXPECT_EQ(rc, VEIDES_RC_SUCCESS);

    VeidesStreamHubClientProperties_setProperty(properties, "client.host", "some_host");
    VeidesStreamHubClientProperties_setProperty(properties, "auth.token", "token");

    rc = VeidesStreamHubClient_create(&client, properties);

    EXPECT_EQ(rc, VEIDES_RC_NULL_PARAM);

    VeidesStreamHubClientProperties_setProperty(properties, "auth.user.name", "");

    rc = VeidesStreamHubClient_create(&client, properties);

    EXPECT_EQ(rc, VEIDES_RC_NULL_PARAM);
}

TEST(veides_stream_hub_client, should_return_error_when_invalid_user_token_provided)
{
    VeidesStreamHubClientProperties *properties = NULL;
    VeidesStreamHubClient *client = NULL;

    VEIDES_RC rc = VeidesStreamHubClientProperties_create(&properties);

    EXPECT_EQ(rc, VEIDES_RC_SUCCESS);

    VeidesStreamHubClientProperties_setProperty(properties, "client.host", "some_host");
    VeidesStreamHubClientProperties_setProperty(properties, "auth.user.name", "name");

    rc = VeidesStreamHubClient_create(&client, properties);

    EXPECT_EQ(rc, VEIDES_RC_NULL_PARAM);

    VeidesStreamHubClientProperties_setProperty(properties, "auth.token", "");

    rc = VeidesStreamHubClient_create(&client, properties);

    EXPECT_EQ(rc, VEIDES_RC_NULL_PARAM);
}

TEST(veides_stream_hub_client, should_return_success_when_valid_properties_provided)
{
    VeidesStreamHubClientProperties *properties = NULL;
    VeidesStreamHubClient *client = NULL;

    VEIDES_RC rc = VeidesStreamHubClientProperties_create(&properties);

    EXPECT_EQ(rc, VEIDES_RC_SUCCESS);

    VeidesStreamHubClientProperties_setProperty(properties, "client.host", "some_host");
    VeidesStreamHubClientProperties_setProperty(properties, "auth.user.name", "name");
    VeidesStreamHubClientProperties_setProperty(properties, "auth.token", "token");

    rc = VeidesStreamHubClient_create(&client, properties);

    EXPECT_EQ(rc, VEIDES_RC_SUCCESS);
}

TEST(veides_stream_hub_client, should_return_success_when_valid_input_provided)
{
    VEIDES_RC rc = veides_sh_client_topic_match("test/#", "test/");

    EXPECT_EQ(rc, VEIDES_RC_SUCCESS);

    rc = veides_sh_client_topic_match("test/+", "test/foo");

    EXPECT_EQ(rc, VEIDES_RC_SUCCESS);

    rc = veides_sh_client_topic_match("#", "test/foo/bar");

    EXPECT_EQ(rc, VEIDES_RC_SUCCESS);

    rc = veides_sh_client_topic_match("test/+/+/foo", "test/bar/baz/foo");

    EXPECT_EQ(rc, VEIDES_RC_SUCCESS);

    rc = veides_sh_client_topic_match("test/#", "test/bar/baz/foo");

    EXPECT_EQ(rc, VEIDES_RC_SUCCESS);

    rc = veides_sh_client_topic_match("test/+/#", "test/bar/baz/foo");

    EXPECT_EQ(rc, VEIDES_RC_SUCCESS);

    rc = veides_sh_client_topic_match("#", "$SYS/foo");

    EXPECT_EQ(rc, VEIDES_RC_SUCCESS);
}

TEST(veides_stream_hub_client, should_return_error_when_valid_input_provided_but_does_not_match)
{
    VEIDES_RC rc = veides_sh_client_topic_match("test/+", "test/foo/bar");

    EXPECT_EQ(rc, VEIDES_RC_FAILURE);

    rc = veides_sh_client_topic_match("test/#bar", "test/foo/bar");

    EXPECT_EQ(rc, VEIDES_RC_FAILURE);

    rc = veides_sh_client_topic_match("test/#", "tes/foo/bar");

    EXPECT_EQ(rc, VEIDES_RC_FAILURE);
}

TEST(veides_stream_hub_client, should_return_error_when_empty_input_provided)
{
    VEIDES_RC rc = veides_sh_client_topic_match(NULL, NULL);

    EXPECT_EQ(rc, VEIDES_RC_FAILURE);

    rc = veides_sh_client_topic_match(NULL, "");

    EXPECT_EQ(rc, VEIDES_RC_FAILURE);

    rc = veides_sh_client_topic_match("", NULL);

    EXPECT_EQ(rc, VEIDES_RC_FAILURE);

    rc = veides_sh_client_topic_match(NULL, "test");

    EXPECT_EQ(rc, VEIDES_RC_FAILURE);

    rc = veides_sh_client_topic_match("test", NULL);

    EXPECT_EQ(rc, VEIDES_RC_FAILURE);

    rc = veides_sh_client_topic_match("test", "");

    EXPECT_EQ(rc, VEIDES_RC_FAILURE);

    rc = veides_sh_client_topic_match("", "test");

    EXPECT_EQ(rc, VEIDES_RC_FAILURE);
}
