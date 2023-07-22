#ifndef VULKAN_UBO_HPP
#define VULKAN_UBO_HPP

#include <glm/glm.hpp>

struct VulkanUniformBufferObject
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
};

#endif 