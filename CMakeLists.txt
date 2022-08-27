cmake_minimum_required(VERSION 3.5)

project(mavlink_microservices_example LANGUAGES C CXX)
 
set(CMAKE_PREFIX_PATH "F:/Qt5.14.2/5.14.2/mingw73_64")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5Core)
find_package(Qt5Network)
add_subdirectory(src)

add_executable(${PROJECT_NAME}  
example/mavlink_microservices_example.cpp
)

target_link_libraries(${PROJECT_NAME}
    mavlink_microservices
    Qt5::Core
    Qt5::Network
)

target_include_directories(${PROJECT_NAME} 
    PUBLIC
    "${PROJECT_BINARY_DIR}"
)