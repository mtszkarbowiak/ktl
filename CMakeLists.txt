# Created by Mateusz Karbowiak 2024

# Specify the minimum CMake version and project name
cmake_minimum_required(VERSION 3.10)
project(MkStl VERSION 1.0)

# Set the required C++ standard
set(CMAKE_CXX_STANDARD ${CMAKE_CXX_STANDARD})
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Assertions level configuration
set(ASSERTIONS_LEVEL "2" CACHE STRING "Set the level of assertions (0, 1, 2, 3)")
add_compile_definitions(ASSERTIONS_LEVEL=${ASSERTIONS_LEVEL})
message(STATUS "ASSERTIONS_LEVEL is set to: ${ASSERTIONS_LEVEL}")

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
target_include_directories(Core PUBLIC 
    ${PROJECT_SOURCE_DIR}/Include
)


# -----------------------------------------------------------------------------
# Tests Executable
# -----------------------------------------------------------------------------

# Add the Tests executable
add_executable(Tests ${TEST_SOURCES})

# Link the Tests executable with required libraries
target_link_libraries(Tests PRIVATE Core gtest gtest_main)

# Include directories for Tests
target_include_directories(Tests PRIVATE 
    ${gtest_SOURCE_DIR}/include       # GoogleTest headers
    ${gtest_BINARY_DIR}/include       # GoogleTest build output (if needed)
)

# Add the Core include directory for the Tests target
target_include_directories(Tests PRIVATE
    ${PROJECT_SOURCE_DIR}/Include
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

    # Define the source and destination paths of Natvis files
    set(NATVIS_SOURCE_DIR "${CMAKE_SOURCE_DIR}/Debug/Natvis")
    set(NATVIS_DEST_DIR "${CMAKE_BINARY_DIR}")

    # Copy Natvis files only if they change
    file(GLOB NATVIS_FILES "${NATVIS_SOURCE_DIR}/*.natvis")
    foreach(NATVIS_FILE ${NATVIS_FILES})
        get_filename_component(NATVIS_NAME ${NATVIS_FILE} NAME)
        add_custom_command(
            TARGET Tests POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${NATVIS_FILE} ${NATVIS_DEST_DIR}/${NATVIS_NAME}
            COMMENT "Copying updated ${NATVIS_NAME} for debugging visualization"
        )
    endforeach()


endif()

# -----------------------------------------------------------------------------
# Debugging Information
# -----------------------------------------------------------------------------

# Print collected sources for debugging
message(STATUS "Core sources: ${CORE_SOURCES}")
message(STATUS "Test sources: ${TEST_SOURCES}")
