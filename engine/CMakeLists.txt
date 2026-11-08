cmake_minimum_required(VERSION 3.28)

project(limenamer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

option(LN_BUILD_TESTS "Build tests" ON)
option(LN_SANITIZE "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)

if(LN_SANITIZE)
    add_compile_options(-fsanitize=address,undefined -fno-sanitize-recover=all
        -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()

add_library(ln_build_options INTERFACE)
if(MSVC)
    target_compile_options(ln_build_options INTERFACE /W4 /WX /permissive- /utf-8)
else()
    target_compile_options(ln_build_options INTERFACE
        -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Werror)
endif()

add_library(limenamer_core OBJECT
    src/session.cpp
    src/utf.cpp)
target_include_directories(limenamer_core PUBLIC include src)
target_link_libraries(limenamer_core PRIVATE ln_build_options)

if(WIN32)
    add_library(limenamer SHARED src/c_api.cpp)
    target_compile_definitions(limenamer PRIVATE LN_BUILDING)
else()
    add_library(limenamer STATIC src/c_api.cpp)
endif()
target_include_directories(limenamer PUBLIC include)
target_link_libraries(limenamer PRIVATE limenamer_core ln_build_options)

if(LN_BUILD_TESTS)
    include(FetchContent)
    FetchContent_Declare(googletest
        URL https://github.com/google/googletest/releases/download/v1.18.0/googletest-1.18.0.tar.gz
        URL_HASH SHA256=6e3191c1455468b3fc35a417fb565c1c5071aee1b7e7f85e30cf48a98d37d8b5
        SYSTEM
        EXCLUDE_FROM_ALL)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()
    include(GoogleTest)

    add_executable(limenamer_tests
        tests/session_test.cpp
        tests/utf_test.cpp)
    target_link_libraries(limenamer_tests PRIVATE limenamer_core ln_build_options
        GTest::gtest_main)
    gtest_discover_tests(limenamer_tests DISCOVERY_MODE PRE_TEST)

    add_executable(limenamer_api_tests
        tests/c_api_test.cpp)
    target_include_directories(limenamer_api_tests PRIVATE src)
    target_link_libraries(limenamer_api_tests PRIVATE limenamer ln_build_options
        GTest::gtest_main)
    gtest_discover_tests(limenamer_api_tests DISCOVERY_MODE PRE_TEST)
endif()
