add_library(mavlink_microservices)

file(GLOB LIB_MSER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/Microservices/*.c)
file(GLOB LIB_HAL_FILES ${CMAKE_CURRENT_SOURCE_DIR}/route/*.c)

target_sources(mavlink_microservices
    PRIVATE
    ${LIB_MSER_FILES}
    ${LIB_HAL_FILES}
)

target_include_directories(mavlink_microservices
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Microservices
    ${CMAKE_CURRENT_SOURCE_DIR}/mavlink
    ${CMAKE_CURRENT_SOURCE_DIR}/route
)