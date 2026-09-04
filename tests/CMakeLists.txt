# SPDX-FileCopyrightText: 2026 Thomas Ascher <thomas.ascher@gmx.at>
#
# SPDX-License-Identifier: MIT

# FIND_PACKAGE_ARGS makes FetchContent try find_package(Catch2 3) before
# downloading.
include(FetchContent)
FetchContent_Declare(Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.11.0
    GIT_SHALLOW TRUE
    SYSTEM
    EXCLUDE_FROM_ALL
    FIND_PACKAGE_ARGS 3
)
FetchContent_MakeAvailable(Catch2)

# catch_discover_tests() lives beside the sources when downloaded, inside the
# package when found installed.
if(catch2_SOURCE_DIR)
    list(APPEND CMAKE_MODULE_PATH "${catch2_SOURCE_DIR}/extras")
endif()
include(Catch)

add_executable(olfarve_tests
    test_color.cpp
    test_package.cpp
)

# The tests reach into the internal headers.
target_include_directories(olfarve_tests PRIVATE "${PROJECT_SOURCE_DIR}/src")
target_link_libraries(olfarve_tests PRIVATE olfarve::olfarve Catch2::Catch2WithMain)
set_target_properties(olfarve_tests PROPERTIES CXX_EXTENSIONS OFF)

target_compile_options(olfarve_tests PRIVATE ${OLFARVE_WARNINGS})

# Windows has no RPATH, so the DLL is copied beside the test program. This
# must come before catch_discover_tests() below, which runs the test program:
# post build commands execute in the order they are added.
if(WIN32)
    add_custom_command(TARGET olfarve_tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy -t
            "$<TARGET_FILE_DIR:olfarve_tests>" "$<TARGET_RUNTIME_DLLS:olfarve_tests>"
        COMMAND_EXPAND_LISTS
    )
endif()

catch_discover_tests(olfarve_tests)
