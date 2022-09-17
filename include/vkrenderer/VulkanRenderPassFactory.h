#ifndef VKRENDER_VULKAN_RENDERPASS_FACTORY_H
#define VKRENDER_VULKAN_RENDERPASS_FACTORY_H

#include <vulkan/vulkan.hpp>

#include "exports.hpp"
#include "utilities/memory.hpp"
#include "vkrenderer/VulkanRenderPass.h"
#include "vkrenderer/VulkanSwapChain.h"

namespace vkrender
{
    class VULKAN_EXPORTS VulkanRenderPassFactory 
    {
    public:
        static utils::Uptr<VulkanRenderPass> setupRenderPassForSwapChain( VulkanLogicalDevice* pLogicalDevice, VulkanSwapChain* pSwapchain );
    }; 
} // namespace vkrender

#endif