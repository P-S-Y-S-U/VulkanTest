cmake_minimum_required(VERSION 3.16)
project(VulkanTest)
set(CMAKE_CXX_STANDARD 17)

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

# Find CMAKE package
find_package(glm CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(tinyobjloader CONFIG REQUIRED)
find_package(Vulkan 1.3.275 REQUIRED)
if(LINUX)
    find_package(Wayland REQUIRED)
endif(LINUX)

set(VULKAN_SHADER_COMPILER ${Vulkan_GLSLC_EXECUTABLE})

if( "${VULKAN_SHADER_COMPILER}" STREQUAL "" )
if(LINUX)
find_file(VULKAN_SHADER_COMPILER         glslc
                            HINTS   /usr/
                            HINTS   ENV VULKAN_SDK
                            REQUIRED
)
else()
find(VULKAN_SHADER_COMPILER         glslc
                            HINTS   ENV PATH
                            HINTS   ENV VULKAN_SDK
                            REQUIRED
)
endif(LINUX)
endif()

message( "Found Vulkan Shader Compiler ${VULKAN_SHADER_COMPILER}" )

# CMAKE VAR SETUP
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# compile definitions
set(PROJECT_COMPILER_DEFINITIONS NOMINMAX)
if(WIN32)
list(APPEND PROJECT_COMPILER_DEFINITIONS _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING)
endif()

# PROJECT VARS SETUP
set(PROJECT_BIN     "bin")
set(PROJECT_LIB     "lib")
set(PROJECT_ARCHIVE "lib/static")
set(_EXPORT_PREFIX "Vulkan_Export")
set(PROJECT_INSTALL ${CMAKE_INSTALL_PREFIX})

set(PROJECT_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")

foreach( PREFIX_ITEM IN LISTS CMAKE_PREFIX_PATH )
    list(APPEND DEPENDENCIES_INCLUDE_DIR "${PREFIX_ITEM}/include" )
endforeach( PREFIX_ITEM IN LISTS CMAKE_PREFIX_PATH )
if(LINUX)
    list(APPEND DEPENDENCIES_INCLUDE_DIR ${Wayland_Client_INCLUDE_DIR})
endif(LINUX)

# PROJECT CMAKE SCRIPT REQUIREMENTS (Dependencies cmake script)#

# CONAN SETUP #

# Additional cmake scripts in sub-directory #
add_subdirectory(src)
add_subdirectory(media)
add_subdirectory(test)