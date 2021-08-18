project(gtest_builder C CXX)
include(ExternalProject)

set(GTEST_FORCE_SHARED_CRT ON)
set(GTEST_DISABLE_PTHREADS OFF)

ExternalProject_Add(
    googletest

    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.10.0
    CMAKE_ARGS
        -DCMAKE_CXX_FLAGS=${MSVC_COMPILER_DEFS}
        -Dgtest_force_shared_crt=${GTEST_FORCE_SHARED_CRT}
        -Dgtest_disable_pthreads=${GTEST_DISABLE_PTHREADS}
    BUILD_ALWAYS OFF
    PREFIX "${CMAKE_CURRENT_BINARY_DIR}"
    INSTALL_COMMAND ""
)

ExternalProject_Get_Property(googletest source_dir)
set(GTEST_INCLUDE_DIR ${source_dir}/googletest/include ${source_dir}/googlemock/include PARENT_SCOPE)

ExternalProject_Get_Property(googletest binary_dir)
set(GTEST_LIBS_DIR ${binary_dir}/lib PARENT_SCOPE)
