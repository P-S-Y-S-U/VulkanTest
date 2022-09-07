#ifndef VKRENDER_VULKAN_SWAP_CHAIN_FACTORY_H
#define VKRENDER_VULKAN_SWAP_CHAIN_FACTORY_H

#include "utilities/memory.hpp"
#include "exports.hpp"
#include "vkrenderer/VulkanSwapChainStructs.hpp"
#include "vkrenderer/VulkanPhysicalDevice.h"
#include "window/window.h"

namespace vkrender
{
    class VULKAN_EXPORTS VulkanSwapChainFactory
    {
    public:
        VulkanSwapChainFactory() = default;
        ~VulkanSwapChainFactory() = default;

        static utils::Sptr<SwapChainPreset> createSuitableSwapChainPreset( const VulkanPhysicalDevice& physicalDevice, const VulkanSurface& surface, const Window& window );
    }; 
} // namespace vkrender


#endif