# SPDX-FileCopyrightText: 2026 Thomas Ascher <thomas.ascher@gmx.at>
#
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.15)
project(test_package LANGUAGES CXX)

find_package(olfarve REQUIRED CONFIG)

add_executable(${PROJECT_NAME} test_package.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE olfarve::olfarve)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
