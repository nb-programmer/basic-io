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
add_subdirectory("lib/utility/")
add_subdirectory("lib/http/" EXCLUDE_FROM_ALL)

# Build the main project executable
add_executable(${CMAKE_PROJECT_NAME}
    "src/main.c"
)

add_dependencies(${CMAKE_PROJECT_NAME}
    basic
    data_structures
    utility
)

target_link_libraries(${CMAKE_PROJECT_NAME}
    PRIVATE
        basic::basic
        data_structures::data_structures
        utility::utility
)

# Server
add_executable(server
    EXCLUDE_FROM_ALL
    "src/http_server_main.c"
)

target_link_libraries(server
    PRIVATE
        basic::basic
        data_structures::data_structures
        utility::utility
        http::http
)

# Build the test executables

add_executable(${CMAKE_PROJECT_NAME}-test_ast
    EXCLUDE_FROM_ALL
    "src/test_ast.c"
)

target_link_libraries(${CMAKE_PROJECT_NAME}-test_ast
    PRIVATE
        basic::basic
        data_structures::data_structures
)
