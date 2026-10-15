cmake_minimum_required(VERSION 3.16)

project(Yuna VERSION 1.0.0 LANGUAGES CXX)

# ========================================================
# VERSION
# ========================================================
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

# ========================================================
# APP ICON
# ========================================================
set(YUNA_ICON
    "${CMAKE_SOURCE_DIR}/public/assets/app.ico")
if(NOT EXISTS "${YUNA_ICON}")
    message(FATAL_ERROR "Icon tidak ditemukan: ${YUNA_ICON}")
endif()
message(STATUS "Using icon: ${YUNA_ICON}")

# ========================================================
# BUILD SETTINGS
# ========================================================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Static CRT (tidak butuh VC++ Redistributable)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# Default build type untuk single-config generator
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

# ========================================================
# HELPER WARNINGS
# ========================================================
function(yuna_set_warnings tgt)
    if(MSVC)
        target_compile_options(${tgt} PRIVATE /W4 /permissive- /wd4996)
    else()
        target_compile_options(${tgt} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endfunction()

# ========================================================
# TARGET UTAMA
# ========================================================
add_executable(Yuna WIN32
    src/main.cc
    src/resources.rc
    src/app.manifest
)
target_compile_definitions(Yuna PRIVATE UNICODE _UNICODE)
target_include_directories(Yuna PRIVATE src src/include)
target_link_libraries(Yuna PRIVATE comdlg32 comctl32 uxtheme dwmapi gdi32 ole32)
yuna_set_warnings(Yuna)

# ========================================================
# TEST TOOLS (opsional)
# ========================================================
option(BUILD_TESTS "Build test executables" OFF)

if(BUILD_TESTS)
    add_executable(tests test/main_tests.cc)
    target_include_directories(tests PRIVATE src src/include test/include)
    yuna_set_warnings(tests)

    add_executable(psnr_eval test/psnr_evaluator.cc)
    target_include_directories(psnr_eval PRIVATE src src/include test/include)
    yuna_set_warnings(psnr_eval)
endif()

# ========================================================
# INSTALL RULES  (WAJIB sebelum include(CPack))
# ========================================================
install(TARGETS Yuna
    RUNTIME DESTINATION bin
)

foreach(extra README.md LICENSE)
    if(EXISTS "${CMAKE_SOURCE_DIR}/${extra}")
        install(FILES "${CMAKE_SOURCE_DIR}/${extra}" DESTINATION .)
    endif()
endforeach()


# ========================================================
# CPACK - WiX MSI + ZIP
# ========================================================
set(CPACK_PACKAGE_NAME           "Yuna")
set(CPACK_PACKAGE_VENDOR         "RATH")
set(CPACK_PACKAGE_CONTACT        "you@example.com")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
    "Yuna - ${PROJECT_VERSION_FULL}")
set(CPACK_PACKAGE_DESCRIPTION
    "Yuna ${PROJECT_VERSION_FULL}. Aplikasi C++20 untuk Windows.")

# WiX ProductVersion WAJIB numerik
set(CPACK_PACKAGE_VERSION_MAJOR  "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR  "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH  "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_VERSION        "${PROJECT_VERSION}")

set(CPACK_PACKAGE_INSTALL_DIRECTORY "Yuna")
set(CPACK_PACKAGE_FILE_NAME "Yuna-${PROJECT_VERSION_FULL}-win64")

# Output ke <project-root>/dist/
set(CPACK_PACKAGE_DIRECTORY "${CMAKE_SOURCE_DIR}/dist")
file(MAKE_DIRECTORY "${CPACK_PACKAGE_DIRECTORY}")

if(EXISTS "${CMAKE_SOURCE_DIR}/LICENSE")
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
endif()
if(EXISTS "${CMAKE_SOURCE_DIR}/README.md")
    set(CPACK_RESOURCE_FILE_README  "${CMAKE_SOURCE_DIR}/README.md")
endif()

# ---- Generator ----
set(CPACK_GENERATOR "WIX;ZIP")

# ---- WiX options ----
# Generate GUID baru: [guid]::NewGuid().ToString().ToUpper()
set(CPACK_WIX_UPGRADE_GUID "B7D2F1A8-4C9E-4B3D-9A1F-2E8C5F6D7A90")

set(CPACK_PACKAGE_EXECUTABLES "Yuna" "Yuna")
set(CPACK_WIX_PROGRAM_MENU_FOLDER "Yuna")
set(CPACK_CREATE_DESKTOP_LINKS "Yuna")
set(CPACK_WIX_INSTALL_SCOPE "perMachine")
set(CPACK_WIX_PRODUCT_ICON "${YUNA_ICON}")

# WiX root dari environment WIX
if(NOT CPACK_WIX_ROOT AND DEFINED ENV{WIX})
    file(TO_CMAKE_PATH "$ENV{WIX}" CPACK_WIX_ROOT)
    string(REGEX REPLACE "/$" "" CPACK_WIX_ROOT "${CPACK_WIX_ROOT}")
endif()

if(CPACK_WIX_ROOT)
    message(STATUS "WiX root: ${CPACK_WIX_ROOT}")
else()
    message(WARNING "WIX env tidak di-set. Set manual: -DCPACK_WIX_ROOT=...")
endif()

# ---- ZIP ----
set(CPACK_ARCHIVE_COMPONENT_INSTALL OFF)

# ========================================================
include(CPack)