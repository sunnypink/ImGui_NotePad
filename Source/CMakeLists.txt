cmake_minimum_required(VERSION 3.22.1) 
	
set(CMAKE_GENERATOR "Ninja")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_LIBRARY_PATH "/usr/lib/")
set(GLFW_INCLUDE_PATH "/usr/local/include/")

project(ImGuiCMake)
configure_file(config.h.in config.h)

find_package(glfw3 REQUIRED PATHS "/usr/local/lib/cmake/")
find_library(GL_SHARED_LIB NAMES GL REQUIRED)
find_library(GLFW_STATIC_LIB NAMES glfw3 REQUIRED PATHS "/usr/local/lib/" NO_DEFAULT_PATH)

add_subdirectory(ImGui ${CMAKE_BINARY_DIR}/${PROJECT_NAME}/) # add_subdirectory(inFile OutFile)

add_executable(ImGuiApp main.cpp ImGui)

target_link_libraries(ImGuiApp ImGui ${GLFW_STATIC_LIB} ${GL_SHARED_LIB})
target_include_directories(ImGuiApp PRIVATE Headers ${GLFW_INCLUDE_PATH})

