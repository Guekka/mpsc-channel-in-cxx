add_library(mpsc_channel STATIC)

set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(mpsc_channel PRIVATE
        ${INCLUDE_DIR}/mpsc/mpsc_channel.hpp
)

find_path(MPSC_ZIB_INCLUDE_DIRS "zib/overflow_mpsc_queue.hpp")
target_include_directories(mpsc_channel PRIVATE ${MPSC_ZIB_INCLUDE_DIRS})

find_package(OpenMP REQUIRED)
target_link_libraries(mpsc_channel PRIVATE OpenMP::OpenMP_CXX)

set_target_properties("${PROJECT_NAME}" PROPERTIES DEBUG_POSTFIX "d")
set_target_properties("${PROJECT_NAME}" PROPERTIES LINKER_LANGUAGE CXX)

target_compile_features("${PROJECT_NAME}" PUBLIC cxx_std_20)

target_include_directories(
        "${PROJECT_NAME}" SYSTEM
        PUBLIC "$<BUILD_INTERFACE:${INCLUDE_DIR}>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
        PRIVATE "$<BUILD_INTERFACE:${INCLUDE_DIR}>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

install(TARGETS "${PROJECT_NAME}" EXPORT "${PROJECT_NAME}-targets")

install(
        EXPORT "${PROJECT_NAME}-targets"
        NAMESPACE "mpsc::"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")

configure_file("${ROOT_DIR}/cmake/config.cmake.in"
        "${PROJECT_NAME}Config.cmake" @ONLY)

write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
        COMPATIBILITY SameMajorVersion)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")

install(DIRECTORY "${INCLUDE_DIR}/mpsc"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
