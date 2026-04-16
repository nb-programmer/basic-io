cmake_minimum_required(VERSION 3.20)

# Configure project
project(
    basic
    DESCRIPTION "Basic lexer, parser and interpreter"
    LANGUAGES C CXX
    VERSION 1.0
)

# Build the main project executable
add_library(${PROJECT_NAME}
    "src/ast.c"
    "src/basic_lexer.c"
    "src/basic_parser.c"
    "src/basic_program.c"
    "src/basic_runner.c"
    "src/basic_runtime_builtin_functions.c"
)

add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

add_dependencies(${PROJECT_NAME} basic_system_interface data_structures utility)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        basic_system_interface::basic_system_interface
        data_structures::data_structures
        utility::utility
)

target_include_directories(${PROJECT_NAME} PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
)
