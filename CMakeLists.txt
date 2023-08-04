cmake_minimum_required(VERSION 3.19)

project(
        mpsc_channel
        VERSION 0.1.0
        LANGUAGES CXX)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(include)

if("${BUILD_TESTING}")
    include(CTest)
    add_subdirectory(tests)
endif()

