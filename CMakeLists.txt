cmake_minimum_required(VERSION 3.14) # floor needed for FetchContent_MakeAvailable
project(Bricks CXX)

add_library(bricks INTERFACE)
target_include_directories(bricks INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(bricks INTERFACE cxx_std_17)

# Default tests ON only when Bricks is the top-level project being configured
# directly, not when it's pulled in as a subdirectory/submodule by a consumer.
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    option(BRICKS_BUILD_TESTS "Build Bricks unit tests" ON)
else()
    option(BRICKS_BUILD_TESTS "Build Bricks unit tests" OFF)
endif()

if(BRICKS_BUILD_TESTS)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.15.2
    )
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    if(WIN32)
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    endif()
    FetchContent_MakeAvailable(googletest)

    enable_testing()
    add_subdirectory(tests)
endif()
