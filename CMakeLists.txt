# Created by Mateusz Karbowiak 2024

# Specify the minimum CMake version and project name
cmake_minimum_required(VERSION 3.10)
project(MkStl VERSION 1.0)

# Set the required C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Configure build options for MSVC
if(MSVC)
    # Debug options
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Zi")
    set(CMAKE_BUILD_TYPE Debug)

    # Ensure consistent runtime library usage
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /MT")
endif()


# -----------------------------------------------------------------------------
# Fetch and Configure Dependencies
# -----------------------------------------------------------------------------

include(FetchContent)

# GoogleTest setup
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.11.0 # Use a stable release version
)

# Download and build GoogleTest
FetchContent_MakeAvailable(googletest)

# -----------------------------------------------------------------------------
# Source File Collection
# -----------------------------------------------------------------------------

# Define project source directories
set(CORE_DIR ${PROJECT_SOURCE_DIR}/Core)
set(TESTS_DIR ${PROJECT_SOURCE_DIR}/Tests)

# Collect all source files for Core and Tests
file(GLOB_RECURSE CORE_SOURCES "${CORE_DIR}/*.cpp")
file(GLOB_RECURSE TEST_SOURCES "${TESTS_DIR}/*.cpp")


# -----------------------------------------------------------------------------
# Core Library
# -----------------------------------------------------------------------------

# Add the Core library
add_library(Core STATIC ${CORE_SOURCES})

# Set include directories for the Core library
target_include_directories(Core PUBLIC ${CORE_DIR}/include)


# -----------------------------------------------------------------------------
# Tests Executable
# -----------------------------------------------------------------------------

# Add the Tests executable
add_executable(Tests ${TEST_SOURCES})

# Link the Tests executable with required libraries
target_link_libraries(Tests PRIVATE Core gtest gtest_main)

# Include directories for Tests
target_include_directories(Tests PRIVATE 
    ${CORE_DIR}/include               # Core headers
    ${gtest_SOURCE_DIR}/include       # GoogleTest headers
    ${gtest_BINARY_DIR}/include       # GoogleTest build output (if needed)
)


# -----------------------------------------------------------------------------
# Testing Configuration
# -----------------------------------------------------------------------------

# Enable CTest for running tests with CMake
enable_testing()

# Add a test suite
add_test(NAME MyTests COMMAND Tests)


# -----------------------------------------------------------------------------
# Additional Configuration for MSVC
# -----------------------------------------------------------------------------

if(MSVC)
    # Set the startup project for Visual Studio
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Tests)
endif()

# -----------------------------------------------------------------------------
# Debugging Information
# -----------------------------------------------------------------------------

# Print collected sources for debugging
message(STATUS "Core sources: ${CORE_SOURCES}")
message(STATUS "Test sources: ${TEST_SOURCES}")
