
add_executable(TriangleApplication ${VULKAN_APPLICATION_BASE_SRCS} TriangleApplication.cpp)
target_compile_definitions(TriangleApplication PUBLIC ${PROJECT_COMPILER_DEFINITIONS})
target_link_libraries(TriangleApplication PUBLIC $<BUILD_INTERFACE:vulkanrenderer>)

add_executable(ModelApplication ${VULKAN_APPLICATION_BASE_SRCS} ModelApplication.cpp)
target_compile_definitions(ModelApplication PUBLIC ${PROJECT_COMPILER_DEFINITIONS})
target_link_libraries(ModelApplication PUBLIC $<BUILD_INTERFACE:vulkanrenderer>)
