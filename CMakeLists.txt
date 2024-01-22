cmake_minimum_required(VERSION 3.16)
project(VulkanTest)
set(CMAKE_CXX_STANDARD 17)

# Find CMAKE package
find_package(glm CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(tinyobjloader CONFIG REQUIRED)
find_package(VulkanLoader CONFIG REQUIRED)

# CMAKE C & CXX FLAGS SETUP

# compile definitions
add_compile_definitions(NOMINMAX)

# PROJECT VARS SETUP
set(PROJECT_BIN     "bin")
set(PROJECT_LIB     "lib")
set(PROJECT_ARCHIVE "lib/static")
set(_EXPORT_PREFIX "Vulkan_Export")
set(PROJECT_INSTALL ${CMAKE_INSTALL_PREFIX})

# PROJECT CMAKE SCRIPT REQUIREMENTS (Dependencies cmake script)#

# CONAN SETUP #

# Additional cmake scripts in sub-directory #
add_subdirectory(src)
add_subdirectory(test)