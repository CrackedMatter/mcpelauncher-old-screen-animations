cmake_minimum_required(VERSION 3.14...3.31)

project(mcpelauncheroldscreenanimations)

if(NOT CMAKE_ANDROID_ARCH_ABI STREQUAL "x86_64")
    message(FATAL_ERROR "Unsupported ABI")
endif()

set(CMAKE_CXX_STANDARD 23)

set(CPM_SOURCE_CACHE ${PROJECT_SOURCE_DIR}/cpm-cache CACHE PATH "")
include(cmake/CPM.cmake)

add_library(mcpelauncheroldscreenanimations SHARED src/main.cpp)

target_compile_options(mcpelauncheroldscreenanimations PRIVATE -Wall -Wextra -Wconversion -Wpedantic)

CPMAddPackage("gh:BasedInc/libhat@0.5.0")
target_link_libraries(mcpelauncheroldscreenanimations PRIVATE libhat::libhat)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(mcpelauncheroldscreenanimations PRIVATE -flto)
    target_link_options(mcpelauncheroldscreenanimations PRIVATE
        -flto -Wl,--version-script,${PROJECT_SOURCE_DIR}/map.txt -Wl,--no-undefined-version)
    set_target_properties(mcpelauncheroldscreenanimations PROPERTIES
        LINK_DEPENDS ${PROJECT_SOURCE_DIR}/map.txt)
endif()

install(TARGETS mcpelauncheroldscreenanimations LIBRARY DESTINATION mods)
