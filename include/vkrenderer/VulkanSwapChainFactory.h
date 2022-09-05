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

        utils::Sptr<vk::SwapchainCreateInfoKHR> createSuitableSwapChainPreset( const VulkanPhysicalDevice& physicalDevice, const VulkanSurface& surface, const Window& window );
    
    private:
        vk::SurfaceFormatKHR chooseSwapSurfaceFormat( const SwapChainSupportDetails& swapChainSupportDetails );
        vk::PresentModeKHR chooseSwapPresentMode( const SwapChainSupportDetails& swapChainSupportDetails );
        vk::Extent2D chooseSwapExtent( const SwapChainSupportDetails& swapChainSupportDetails, const Window& window );
        std::uint32_t chooseImageCount( const SwapChainSupportDetails& swapChainSupportDetails );
    }; 
} // namespace vkrender


#endif