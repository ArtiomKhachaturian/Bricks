add_executable(bricks_tests
    test_common.h
    listener_tests.cpp
    listeners_tests.cpp
)

target_link_libraries(bricks_tests
    PRIVATE
        bricks
        gtest_main
)

target_compile_features(bricks_tests PRIVATE cxx_std_17)

include(GoogleTest)
gtest_discover_tests(bricks_tests)
