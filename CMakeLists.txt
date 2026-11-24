cmake_minimum_required(VERSION 3.24)

project(
    mimage
    VERSION 0.1.0
    DESCRIPTION "ncnn image container, codecs, resize and drawing"
    LANGUAGES CXX
)

include(GNUInstallDirs)
include(CTest)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()

option(mimage_build_tests "Build mimage tests" ${PROJECT_IS_TOP_LEVEL})
option(MIMAGE_SANITIZE "Enable ASan+UBSan" OFF)

if(NOT TARGET mmemory::mmemory)
    set(MIMAGE_MMEMORY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../mmemory" CACHE PATH "mmemory source directory")
    if(NOT EXISTS "${MIMAGE_MMEMORY_DIR}/CMakeLists.txt")
        message(FATAL_ERROR "mimage requires the mmemory source tree; set MIMAGE_MMEMORY_DIR")
    endif()
    set(mmemory_build_tests OFF CACHE BOOL "" FORCE)
    add_subdirectory("${MIMAGE_MMEMORY_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/mmemory")
endif()

set(MIMAGE_WARNINGS_COMMON
    -Wall
    -Wextra
    -Wpedantic
    -Wconversion
    -Wsign-conversion
    -Wshadow
    -Wold-style-cast
    -Wcast-qual
    -Wdouble-promotion
    -Wformat=2
    -Wundef
    -Wpointer-arith
    -Wwrite-strings
    -Wnull-dereference
    -Wzero-as-null-pointer-constant
)
set(MIMAGE_WARNINGS_GCC
    -Wmissing-declarations
    -Wredundant-decls
    -Wduplicated-cond
    -Wduplicated-branches
    -Wlogical-op
    -Wuseless-cast
)
set(MIMAGE_WARNINGS
    ${MIMAGE_WARNINGS_COMMON}
    $<$<CXX_COMPILER_ID:GNU>:${MIMAGE_WARNINGS_GCC}>
)

# The codec/resize/drawing bodies are behaviour-preserving ports of ncnn's
# BSD-3-Clause implementation. Keep them clean under the upstream warning
# level instead of rewriting legacy pointer arithmetic and font tables.
set(MIMAGE_WARNINGS_PORTED
    -Wall
    -Wextra
    -Wpedantic
    -Wundef
    -Wpointer-arith
    -Wwrite-strings
    -Wnull-dereference
)

add_library(mimage_sanitizer INTERFACE)
if(MIMAGE_SANITIZE)
    target_compile_options(mimage_sanitizer INTERFACE
        -fsanitize=address,undefined -fno-omit-frame-pointer -D_GLIBCXX_ASSERTIONS)
    target_link_options(mimage_sanitizer INTERFACE
        -fsanitize=address,undefined -fno-omit-frame-pointer)
endif()

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/config.hpp.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/mimage/config.hpp
    @ONLY)

add_library(mimage_objs OBJECT
    src/image.cpp
    src/resize.cpp
    src/drawing.cpp
    src/log.cpp)
target_include_directories(mimage_objs
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party
        $<TARGET_PROPERTY:mmemory::mmemory,INTERFACE_INCLUDE_DIRECTORIES>)
target_compile_options(mimage_objs PRIVATE ${MIMAGE_WARNINGS_PORTED})
target_link_libraries(mimage_objs PRIVATE mimage_sanitizer)

add_library(mimage STATIC)
# Self-contained archive: Image + codecs + resize/drawing + mmemory + mplatform.
target_link_libraries(mimage PUBLIC
    $<BUILD_INTERFACE:mimage_objs>
    $<BUILD_INTERFACE:mmemory_objs>
    $<BUILD_INTERFACE:mplatform_objs>)
add_library(mimage::mimage ALIAS mimage)

target_include_directories(mimage
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_link_libraries(mimage INTERFACE $<BUILD_INTERFACE:mimage_sanitizer>)

if(PROJECT_IS_TOP_LEVEL)
    include(CMakePackageConfigHelpers)

    install(TARGETS mimage EXPORT mimageTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/include/mimage/config.hpp
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/mimage)
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../mmemory/include/mmemory
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/mmemory/include/mmemory/config.hpp
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/mmemory)
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../mplatform/include/mplatform
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/mmemory/mplatform/include/mplatform/config.hpp
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/mplatform)
    install(EXPORT mimageTargets
        NAMESPACE mimage::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/mimage)

    configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/mimageConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/mimageConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/mimage)
    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/mimageConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion)
    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/mimageConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/mimageConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/mimage)
endif()

if(mimage_build_tests AND BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()
