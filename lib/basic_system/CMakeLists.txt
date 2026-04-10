cmake_minimum_required(VERSION 3.20)

# Configure project
project(
    basic_system
    DESCRIPTION "System interface for basic interpreter"
    LANGUAGES C CXX
    VERSION 1.0
)

# Build the main project executable
add_library(${PROJECT_NAME}
    "src/system.c"
)

add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

# Add some preprocessor defines
target_compile_definitions(${PROJECT_NAME} PUBLIC "$<$<CONFIG:DEBUG>:DEBUG>")

target_include_directories(${PROJECT_NAME} PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
)
