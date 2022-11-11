#ifndef VKRENDER_VULKAN_COMMAND_POOL_H
#define VKRENDER_VULKAN_COMMAND_POOL_H

#include <vulkan/vulkan.hpp>

#include "exports.hpp"
#include "vkrenderer/VulkanLogicalDevice.h"
#include "utilities/memory.hpp"

namespace vkrender
{
    class VULKAN_EXPORTS VulkanCommandPool
    {
    public:
        VulkanCommandPool( VulkanLogicalDevice* pLogicalDevice, const std::uint32_t& queueFamilyIndex );
        ~VulkanCommandPool();

        void createCommandPool();
        void destroyCommandPool();

        vk::CommandPool m_commandPoolHandle;
    private:
        VulkanLogicalDevice* m_pLogicalDevice;
        vk::CommandPoolCreateInfo m_commandPoolCreateInfo;

        void populateCommandPoolCreateInfo( const std::uint32_t& queueFamilyIndex );
    };
} // namespace vkrender


#endif