cmake_minimum_required(VERSION 3.0.0)

project(mavlink_microservices_example)
 
add_subdirectory(src)

add_executable(${PROJECT_NAME}  example/mavlink_microservices_example.c)

target_link_libraries(${PROJECT_NAME}
    mavlink_microservices
)

target_include_directories(${PROJECT_NAME} 
    PUBLIC
    "${PROJECT_BINARY_DIR}"
)