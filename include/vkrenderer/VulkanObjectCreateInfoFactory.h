#ifndef VKRENDER_VULKAN_OBJECT_CREATE_INFO_FACTORY_H
#define VKRENDER_VULKAN_OBJECT_CREATE_INFO_FACTORY_H

#include <vulkan/vulkan.hpp>

#include "utilities/memory.hpp"
#include "exports.hpp"

namespace vkrender
{

class VULKAN_EXPORTS VulkanObjectCreateInfoFactory
{
public:
    static vk::DebugUtilsMessengerCreateInfoEXT populateDebugMessengerCreateInfoExt();
};

} // namespace vkrender


#endif