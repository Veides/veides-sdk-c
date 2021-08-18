if(${VEIDES_WITH_STREAM_HUB_CLIENT})
    add_executable(streamHubBasic streamHubBasic.c)

    target_link_libraries(streamHubBasic veides_stream_hub_client)
endif()

if(${VEIDES_WITH_API_CLIENT})
    add_executable(apiBasic apiBasic.c)

    target_link_libraries(apiBasic veides_api_client)
endif()
