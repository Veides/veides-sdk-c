cmake_minimum_required(VERSION 3.5.0)

project(veides_sdk C CXX)

set(VERSION_MAJOR 0)
set(VERSION_MINOR 3)
set(VERSION_PATCH 0)

set(VEIDES_SDK_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

option(VEIDES_WITH_STREAM_HUB_CLIENT "Include building of Stream Hub client" ON)
option(VEIDES_WITH_API_CLIENT "Include building of API client" ON)
option(VEIDES_BUILD_DEPENDENCIES "Either build dependent libraries or use system libraries" OFF)
option(VEIDES_INSTALL_DEPENDENCIES "Whether dependencies should be installed in the system or not" OFF)
option(VEIDES_BUILD_TESTS "Set to ON to build the tests" OFF)
option(VEIDES_BUILD_SAMPLES "To build samples or not to build" ON)

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    message(STATUS "Setting OPENSSL_ROOT_DIR to /usr/local/opt/openssl")
    set(OPENSSL_ROOT_DIR /usr/local/opt/openssl)
    link_directories(${OPENSSL_ROOT_DIR}/lib)
endif()

find_package(OpenSSL REQUIRED)

if(NOT ${VEIDES_BUILD_DEPENDENCIES})
    find_library(PAHO_SYSTEM_LIBRARY paho-mqtt3as REQUIRED)
    set(PAHO_CLIENT_VERSION "system")
endif()

message(STATUS "Veides SDK ${VEIDES_SDK_VERSION}")

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty
    ${PROJECT_BINARY_DIR}
)

set(
    shared_sources

    veides_utils_shared/src/veides_logger.c
    veides_utils_shared/src/veides_utils.c
)

set(
    shared_headers

    veides_utils_shared/include/veides_utils_shared/veides_logger.h
    veides_utils_shared/include/veides_utils_shared/veides_utils.h
    veides_utils_shared/include/veides_utils_shared/veides_rc.h
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/veides_utils_shared/include
)

install(
    FILES

    ${shared_headers}

    DESTINATION include/veides_utils_shared
)

set(
    3rdparty

    3rdparty/cJSON/cJSON.c
    3rdparty/cJSON/cJSON.h

)

if (${VEIDES_WITH_STREAM_HUB_CLIENT})
    message(STATUS "Veides Stream Hub client library included")

    find_package(Threads REQUIRED)

    include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/veides_stream_hub_client/include
    )

    set(
        sh_sources

        veides_stream_hub_client/src/veides_stream_hub_client.c
        veides_stream_hub_client/src/veides_base_client.c
        veides_stream_hub_client/src/veides_properties.c
    )

    set(
        sh_headers

        veides_stream_hub_client/include/veides_stream_hub_client/veides_stream_hub_client.h
        veides_stream_hub_client/include/veides_stream_hub_client/veides_base_client.h
        veides_stream_hub_client/include/veides_stream_hub_client/veides_properties.h
    )

    set(
        sh_headers_internal

        veides_stream_hub_client/include/veides_stream_hub_client/veides_internal.h
    )

    add_library(veides_stream_hub_client SHARED ${sh_sources} ${sh_headers} ${sh_headers_internal} ${3rdparty} ${shared_sources} ${shared_headers})

    set_target_properties(
        veides_stream_hub_client

        PROPERTIES
        VERSION ${VEIDES_SDK_VERSION}
        SOVERSION ${VERSION_MAJOR}
    )

    if(NOT ${VEIDES_BUILD_DEPENDENCIES})
        message(STATUS "Using system Paho MQTT Library")

        target_link_libraries(
            veides_stream_hub_client

            ${PAHO_SYSTEM_LIBRARY}
        )
    else()
        message(STATUS "Adding Paho MQTT Library to build pipeline")

        set(PAHO_CLIENT_VERSION 1.3.7)

        add_subdirectory(3rdparty/paho)

        include_directories(
            ${PAHO_INCLUDE_DIR}
        )

        add_dependencies(veides_stream_hub_client paho)

        target_link_libraries(
            veides_stream_hub_client

            ${PAHO_LIBS_DIR}/libpaho-mqtt3as${CMAKE_SHARED_LIBRARY_SUFFIX}
        )
    endif()

    target_link_libraries(veides_stream_hub_client ${CMAKE_THREAD_LIBS_INIT})

    install(
        TARGETS veides_stream_hub_client

        EXPORT VeidesStreamHubClientConfig

        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
    )

    install(
        EXPORT VeidesStreamHubClientConfig

        DESTINATION lib/cmake
    )

    install(
        FILES

        ${sh_headers}

        DESTINATION include/veides_stream_hub_client
    )
endif()

if (${VEIDES_WITH_API_CLIENT})
    message(STATUS "Veides API client library included")

    find_package(CURL REQUIRED)

    include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/veides_api_client/include
    )

    set(
        api_sources

        veides_api_client/src/veides_api_client.c
        veides_api_client/src/veides_base_client.c
        veides_api_client/src/veides_properties.c
    )

    set(
        api_headers

        veides_api_client/include/veides_api_client/veides_api_client.h
        veides_api_client/include/veides_api_client/veides_base_client.h
        veides_api_client/include/veides_api_client/veides_properties.h
    )

    set(
        api_headers_internal

        veides_api_client/include/veides_api_client/veides_internal.h
    )

    add_library(veides_api_client SHARED ${api_sources} ${api_headers} ${api_headers_internal} ${shared_sources} ${shared_headers})

    set_target_properties(
        veides_api_client

        PROPERTIES
        VERSION ${VEIDES_SDK_VERSION}
        SOVERSION ${VERSION_MAJOR}
    )

    target_link_libraries(
        veides_api_client

        ${CURL_LIBRARIES}
    )

    install(
        TARGETS veides_api_client

        EXPORT VeidesApiClientConfig

        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
    )

    install(
        EXPORT VeidesApiClientConfig

        DESTINATION lib/cmake
    )

    install(
        FILES

        ${api_headers}

        DESTINATION include/veides_api_client
    )
endif()

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/veides_version.h.in
    ${CMAKE_BINARY_DIR}/veides_version.h
    @ONLY
)

if(${VEIDES_BUILD_SAMPLES})
    add_subdirectory(samples)
endif()

if(${VEIDES_BUILD_TESTS})
    enable_testing()

    add_subdirectory(3rdparty/gtest)

    set(
        test_sources

        tests/veides_stream_hub_client.cpp
        tests/veides_stream_hub_client_properties.cpp

        tests/veides_api_client.cpp
        tests/veides_api_client_properties.cpp
    )

    include_directories(
        ${GTEST_INCLUDE_DIR}
        tests
    )

    add_executable(veidesSdkTests ${test_sources} ${sh_sources} ${api_sources} ${3rdparty} ${shared_sources} ${shared_headers})
    add_dependencies(veidesSdkTests googletest)
    add_test(veidesSdkTests veidesSdkTests)

    target_link_libraries(
        veidesSdkTests

        ${GTEST_LIBS_DIR}/libgtest${CMAKE_STATIC_LIBRARY_SUFFIX}
        ${GTEST_LIBS_DIR}/libgtest_main${CMAKE_STATIC_LIBRARY_SUFFIX}
    )

    if(NOT ${VEIDES_BUILD_DEPENDENCIES})
        target_link_libraries(
            veidesSdkTests

            ${PAHO_SYSTEM_LIBRARY}
        )
    else()
        target_link_libraries(
            veidesSdkTests

            ${PAHO_LIBS_DIR}/libpaho-mqtt3as${CMAKE_SHARED_LIBRARY_SUFFIX}
        )

        add_dependencies(veidesSdkTests paho)
    endif()

    target_link_libraries(veidesSdkTests ${CMAKE_THREAD_LIBS_INIT} ${CURL_LIBRARIES})
endif()