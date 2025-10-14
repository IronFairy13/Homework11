cmake_minimum_required(VERSION 3.16)

project(join_server VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_CONFIGURATION_TYPES)
    set(DEFAULT_BUILD_TYPE "Release")
    if(NOT CMAKE_BUILD_TYPE)
        message(STATUS "No CMAKE_BUILD_TYPE specified, defaulting to ${DEFAULT_BUILD_TYPE}")
    endif()
    set(CMAKE_BUILD_TYPE "${CMAKE_BUILD_TYPE}" CACHE STRING "Choose the type of build." FORCE)
    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE "${DEFAULT_BUILD_TYPE}" CACHE STRING "Choose the type of build." FORCE)
    endif()
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

find_package(Threads REQUIRED)

add_library(join_server_core
    source/command.cpp
    source/tables.cpp
)

target_include_directories(join_server_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(join_server::core ALIAS join_server_core)

add_executable(join_server
    source/main.cpp
    source/server.cpp
)

set_target_properties(join_server PROPERTIES
    OUTPUT_NAME join_server
)

target_link_libraries(join_server
    PRIVATE
        join_server::core
        Threads::Threads
)

install(TARGETS join_server
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

option(JOIN_SERVER_BUILD_TESTS "Build join_server unit tests" ON)

if(JOIN_SERVER_BUILD_TESTS)
    enable_testing()

    # Allow Conan-installed packages (generated in the build tree) to be discoverable.
    list(APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}")

    find_package(GTest CONFIG REQUIRED)

    add_executable(join_server_tests
        tests/tables_tests.cpp
        tests/command_tests.cpp
    )

    target_link_libraries(join_server_tests
        PRIVATE
            join_server::core
            GTest::gtest_main
            Threads::Threads
    )

    include(GoogleTest)
    gtest_discover_tests(join_server_tests)
endif()

include(CPack)
