cmake_minimum_required(VERSION 3.20)

# Configure project
project(
    BasicIO
    DESCRIPTION "Basic interpreter"
    LANGUAGES C CXX
    VERSION 1.0
)

add_subdirectory("lib/basic/")
add_subdirectory("lib/basic_system_interface/")
add_subdirectory("lib/data_structures/")

# Build the main project executable
add_executable(server
    "src/http_server_main.c"

    "src/http.c"
    "src/http.h"
    "src/tcpserver.c"
    "src/tcpserver.h"
    "src/utils.c"
    "src/utils.h"
    "src/platform/platform.c"
    "src/platform/platform.h"
)

# Add some preprocessor defines
target_compile_definitions(server
    PUBLIC "$<$<CONFIG:DEBUG>:DEBUG>"
)

add_dependencies(server
    basic
    data_structures
)

target_link_libraries(server
    PRIVATE
        basic::basic
        data_structures::data_structures
)

# Build the main project executable
add_executable(${CMAKE_PROJECT_NAME}-test_ast
    "src/test_ast.c"
    "src/utils.c"
    "src/utils.h"
)

target_link_libraries(${CMAKE_PROJECT_NAME}-test_ast
    PRIVATE
        basic::basic
        data_structures::data_structures
)
