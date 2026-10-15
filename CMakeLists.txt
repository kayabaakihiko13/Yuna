cmake_minimum_required(VERSION 3.16)

project(Yuna VERSION 1.0.0 LANGUAGES CXX)

set(PROJECT_VERSION_SUFFIX "alpha")
if(NOT DEFINED YUNA_BUILD_NUMBER)
    set(YUNA_BUILD_NUMBER "0")
endif()
set(PROJECT_VERSION_FULL
    "${PROJECT_VERSION}-${PROJECT_VERSION_SUFFIX}.${YUNA_BUILD_NUMBER}")
message(STATUS "Building ${PROJECT_NAME} v${PROJECT_VERSION_FULL}")

if(NOT WIN32)
    message(FATAL_ERROR "Yuna saat ini hanya untuk Windows")
endif()

# ---- Standar C++ ----
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---- CRT static ----
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# ---- Default build type (single-config generator) ----
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

# ---- Helper warnings ----
function(yuna_set_warnings tgt)
    if(MSVC)
        target_compile_options(${tgt} PRIVATE /W4 /permissive- /wd4996)
    else()
        target_compile_options(${tgt} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endfunction()

# ---- Target utama ----
add_executable(Yuna WIN32
    src/main.cc
    src/resources.rc
    src/app.manifest
)
target_compile_definitions(Yuna PRIVATE UNICODE _UNICODE)
target_include_directories(Yuna PRIVATE src src/include)
target_link_libraries(Yuna PRIVATE comdlg32 comctl32 uxtheme)
yuna_set_warnings(Yuna)

# (tidak ada RUNTIME_OUTPUT_DIRECTORY — pakai default CMake)

# ---- Test tools (manual) ----
option(BUILD_TESTS "Build test executables" OFF)

if(BUILD_TESTS)
    add_executable(tests test/main_tests.cc)
    target_include_directories(tests PRIVATE src src/include test/include)
    yuna_set_warnings(tests)

    add_executable(psnr_eval test/psnr_evaluator.cc)
    target_include_directories(psnr_eval PRIVATE src src/include test/include)
    yuna_set_warnings(psnr_eval)
endif()