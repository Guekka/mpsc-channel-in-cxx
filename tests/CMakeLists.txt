add_executable(tests tests.cpp)

target_compile_definitions(
        tests PRIVATE
        _GLIBCXX_PARALLEL) # required for parallel STL algorithms on GCC

find_package(Catch2 CONFIG REQUIRED)
include(Catch)
catch_discover_tests(tests)

target_link_libraries(tests PRIVATE ${PROJECT_NAME} Catch2::Catch2WithMain)
