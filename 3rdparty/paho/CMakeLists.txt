project(paho_builer C)
include(ExternalProject)

set(GIT_REPOSITORY https://github.com/eclipse/paho.mqtt.c)
set(GIT_TAG v${PAHO_CLIENT_VERSION})

set(
    CMAKE_ARGS

    -DPAHO_BUILD_SHARED:BOOL=ON
    -DPAHO_BUILD_STATIC:BOOL=OFF
    -DPAHO_WITH_SSL:BOOL=ON
    -DPAHO_ENABLE_TESTING:BOOL=OFF
    -DPAHO_ENABLE_CPACK:BOOL=OFF
    -DOPENSSL_ROOT_DIR:PATH=${OPENSSL_ROOT_DIR}
)

if(NOT ${VEIDES_INSTALL_DEPENDENCIES})
    ExternalProject_Add(
        paho

        GIT_REPOSITORY ${GIT_REPOSITORY}
        GIT_TAG ${GIT_TAG}
        CMAKE_ARGS ${CMAKE_ARGS}
        BUILD_ALWAYS OFF
        PREFIX "${CMAKE_CURRENT_BINARY_DIR}"
        INSTALL_COMMAND ""
    )
else()
    ExternalProject_Add(
        paho

        GIT_REPOSITORY ${GIT_REPOSITORY}
        GIT_TAG ${GIT_TAG}
        CMAKE_ARGS ${CMAKE_ARGS}
        BUILD_ALWAYS OFF
        PREFIX "${CMAKE_CURRENT_BINARY_DIR}"
    )
endif()

ExternalProject_Get_Property(paho source_dir)
set(PAHO_INCLUDE_DIR ${source_dir}/src PARENT_SCOPE)

ExternalProject_Get_Property(paho binary_dir)
set(PAHO_LIBS_DIR ${binary_dir}/src PARENT_SCOPE)
