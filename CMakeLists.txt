cmake_minimum_required(VERSION 3.12)

project(Yuna LANGUAGES CXX)

# set constants untuk configurasi C++
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# flag compiler
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -O2)
endif()


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_SOURCE_DIR}/dist/app")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/dist/app")

# target utama
set(MAIN_SOURCES
    src/main.cc
    src/resources.rc
)
add_executable(Yuna WIN32 ${MAIN_SOURCES})
# Include path
target_include_directories(Yuna PRIVATE
    src
    src/include
)

# Library Windows yang diperlukan
target_link_libraries(Yuna PRIVATE
    comdlg32
    comctl32
)




# Testing session
option(BUILD_TESTS "Build test executable" OFF)

if(BUILD_TESTS)
    # Test unit utama
    add_executable(tests
        test/main_tests.cc
    )
    target_include_directories(tests PRIVATE
        src/include
        test/include
    )

    # Evaluator PSNR
    add_executable(psnr_eval
        test/psnr_evaluator.cc
    )
    target_include_directories(psnr_eval PRIVATE
        src/include
        test/include
    )
endif()