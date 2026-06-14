cmake_minimum_required(VERSION 3.20)
project(CageConversion)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(PREPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(libigl)

file(GLOB MAIN_SRC_FILES src/main.cpp)

add_executable(${PROJECT_NAME} ${MAIN_SRC_FILES})

target_link_libraries(${PROJECT_NAME} PUBLIC igl::core)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/demo $<TARGET_FILE_DIR:${PROJECT_NAME}>/demo)
