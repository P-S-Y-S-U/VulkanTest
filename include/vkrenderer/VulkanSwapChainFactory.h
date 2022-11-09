#ifndef VKRENDER_VULKAN_SWAP_CHAIN_FACTORY_H
#define VKRENDER_VULKAN_SWAP_CHAIN_FACTORY_H

#include "utilities/memory.hpp"
#include "exports.hpp"
#include "vkrenderer/VulkanSwapChainStructs.hpp"
#include "vkrenderer/VulkanQueueFamily.hpp"
#include "window/window.h"

namespace vkrender
{
    class VULKAN_EXPORTS VulkanSwapChainFactory
    {
    public:
        VulkanSwapChainFactory() = default;
        ~VulkanSwapChainFactory() = default;

        static SwapChainSupportDetails querySwapChainSupport( const vk::PhysicalDevice& vkPhysicalDevice, const vk::SurfaceKHR& vkSurface );
        static SwapChainPreset createSuitableSwapChainPreset( 
            const vk::PhysicalDevice& physicalDevice, 
            const vk::SurfaceKHR& surface, 
            const QueueFamilyIndices& queueFamilyIndices,
            const Window& window 
        );
    }; 
} // namespace vkrender


#endif