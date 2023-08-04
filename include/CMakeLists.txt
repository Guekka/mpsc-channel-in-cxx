add_library(mpsc_channel INTERFACE
        ../tests/tests.cpp)

set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(mpsc_channel INTERFACE
        ${INCLUDE_DIR}/mpsc/mpsc_channel.hpp
)

target_include_directories(mpsc_channel INTERFACE ${INCLUDE_DIR})

find_path(MPSC_ZIB_INCLUDE_DIRS "zib/overflow_mpsc_queue.hpp")
target_include_directories(mpsc_channel INTERFACE ${MPSC_ZIB_INCLUDE_DIRS})

find_package(OpenMP REQUIRED)
target_link_libraries(mpsc_channel INTERFACE OpenMP::OpenMP_CXX)

set_target_properties("${PROJECT_NAME}" PROPERTIES DEBUG_POSTFIX "d")
set_target_properties("${PROJECT_NAME}" PROPERTIES LINKER_LANGUAGE CXX)

target_compile_features("${PROJECT_NAME}" INTERFACE cxx_std_20)

