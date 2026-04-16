cmake_minimum_required(VERSION 3.20)

# Configure project
project(
    utility
    DESCRIPTION "Utility functions and headers"
    LANGUAGES C CXX
    VERSION 1.0
)

# Build the main project executable
add_library(${PROJECT_NAME}
    "src/utils.c"
    "src/platform.c"
    "src/logging.c"
)

add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
)
