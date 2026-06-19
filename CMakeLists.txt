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

set(MAIN_SRC src/main.cc)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_SOURCE_DIR}/dist/app")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/dist/app")

# target utama
add_executable(main ${MAIN_SRC})

# base folder include
target_include_directories(main PRIVATE src/include)