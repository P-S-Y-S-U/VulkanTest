# project files src files list #
set(PROJECT_SRC_FILES       window/window.cpp
                            vkrenderer/VulkanDebugMessenger.cpp
                            utilities/VulkanLogger_VulkanValidationLayerLogger.cpp
                            utilities/VulkanLogger_VulkanRendererApiLogger.cpp
                            application/VulkanApplication.cpp
)

# library & executable config #
add_library(vulkanrenderer ${PROJECT_SRC_FILES})
target_include_directories(vulkanrenderer PUBLIC    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
                                                    $<INSTALL_INTERFACE:include>)
target_link_libraries(vulkanrenderer PUBLIC glm::glm glfw spdlog::spdlog tinyobjloader Vulkan::Loader ) 

# cmake install config #
install(TARGETS vulkanrenderer  EXPORT ${_EXPORT_PREFIX}
                                RUNTIME DESTINATION ${PROJECT_BIN}
                                LIBRARY DESTINATION ${PROJECT_LIB}
                                ARCHIVE DESTINATION ${PROJECT_ARCHIVE}
)

install(DIRECTORY ${PROJECT_SOURCE_DIR}/include DESTINATION ${PROJECT_INSTALL}/include)