cmake_minimum_required(VERSION 3.20)
project(CageRepair)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add cmake files to CMAKE_MODULE_PATH
list(PREPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Libigl
include(libigl)

# Enable the target of igl
igl_include(glfw)
igl_include(imgui)
igl_include(opengl)
#igl_include(restricted triangle)

# Add your project files
#file(GLOB SRC_FILES src/*.cpp src/*/*.cpp)

#file(GLOB LIB_SRC_FILES src/*/*.cpp)
file(GLOB MAIN_SRC_FILES src/main.cpp)


# Add project as a executable
add_executable(${PROJECT_NAME} ${MAIN_SRC_FILES})

# Link igl to your project
target_link_libraries(${PROJECT_NAME} PUBLIC
        igl::glfw
        igl::imgui
        igl::opengl
)