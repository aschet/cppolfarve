# SPDX-FileCopyrightText: 2026 Thomas Ascher <thomas.ascher@gmx.at>
#
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.28...4.2)

project(olfarve
    VERSION 1.0.0
    DESCRIPTION "sRGB color rendering of SRM/EBC beer color values"
    HOMEPAGE_URL "https://github.com/aschet/cppolfarve"
    LANGUAGES CXX
)

include(CMakePackageConfigHelpers)
include(GenerateExportHeader)
include(GNUInstallDirs)

if(WIN32 AND NOT DEFINED CMAKE_DEBUG_POSTFIX)
    set(CMAKE_DEBUG_POSTFIX d)
endif()

set(OLFARVE_WARNINGS
    $<$<CXX_COMPILER_ID:MSVC>:/W4;/permissive-;/utf-8>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall;-Wextra;-Wpedantic;-Wshadow;-Wconversion>
)

add_library(olfarve)
add_library(olfarve::olfarve ALIAS olfarve)

set(OLFARVE_GENERATED_INCLUDE_DIR "${PROJECT_BINARY_DIR}/generated/include")

generate_export_header(olfarve
    BASE_NAME olfarve
    EXPORT_FILE_NAME "${OLFARVE_GENERATED_INCLUDE_DIR}/olfarve/olfarve_export.hpp"
)
configure_file(include/olfarve/version.hpp.in
    "${OLFARVE_GENERATED_INCLUDE_DIR}/olfarve/version.hpp" @ONLY
)

target_sources(olfarve
    PRIVATE
        src/cie.hpp
        src/color.cpp
        src/model.hpp
    PUBLIC
        FILE_SET HEADERS
            BASE_DIRS
                "${PROJECT_SOURCE_DIR}/include"
                "${OLFARVE_GENERATED_INCLUDE_DIR}"
            FILES
                include/olfarve/color.hpp
                include/olfarve/olfarve.hpp
                "${OLFARVE_GENERATED_INCLUDE_DIR}/olfarve/olfarve_export.hpp"
                "${OLFARVE_GENERATED_INCLUDE_DIR}/olfarve/version.hpp"
)

target_compile_features(olfarve PUBLIC cxx_std_17)

set_target_properties(olfarve PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    EXPORT_NAME olfarve
    CXX_EXTENSIONS OFF
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
    POSITION_INDEPENDENT_CODE ON
)

target_compile_options(olfarve PRIVATE ${OLFARVE_WARNINGS})

if(PROJECT_IS_TOP_LEVEL)
    include(CTest)
    if(BUILD_TESTING)
        add_subdirectory(tests)
    endif()
endif()

set(olfarve_cmakedir "${CMAKE_INSTALL_LIBDIR}/cmake/olfarve")

install(TARGETS olfarve
    EXPORT olfarve-targets
    FILE_SET HEADERS
)

install(EXPORT olfarve-targets
    FILE olfarve-targets.cmake
    NAMESPACE olfarve::
    DESTINATION "${olfarve_cmakedir}"
)

# The API is header driven and source compatible within a major version.
write_basic_package_version_file("${PROJECT_BINARY_DIR}/olfarve-config-version.cmake"
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(cmake/olfarve-config.cmake.in
    "${PROJECT_BINARY_DIR}/olfarve-config.cmake"
    INSTALL_DESTINATION "${olfarve_cmakedir}"
)

install(FILES
    "${PROJECT_BINARY_DIR}/olfarve-config.cmake"
    "${PROJECT_BINARY_DIR}/olfarve-config-version.cmake"
    DESTINATION "${olfarve_cmakedir}"
)

install(FILES LICENSE DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/licenses/olfarve")
