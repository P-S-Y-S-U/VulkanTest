#include "vkrenderer/VulkanSwapChainFactory.h"
#include "vkrenderer/VulkanQueueFamily.hpp"
#include <limits>
#include <algorithm>

namespace vkrender
{
    vk::SurfaceFormatKHR chooseSwapSurfaceFormat( const SwapChainSupportDetails& swapChainSupportDetails );
    vk::PresentModeKHR chooseSwapPresentMode( const SwapChainSupportDetails& swapChainSupportDetails );
    vk::Extent2D chooseSwapExtent( const SwapChainSupportDetails& swapChainSupportDetails, const Window& window );
    std::uint32_t chooseImageCount( const SwapChainSupportDetails& swapChainSupportDetails );

    SwapChainSupportDetails VulkanSwapChainFactory::querySwapChainSupport( const vk::PhysicalDevice& vkPhysicalDevice, const vk::SurfaceKHR& vkSurface )
    {
    	SwapChainSupportDetails swapChainDetails;

    	swapChainDetails.capabilities =	vkPhysicalDevice.getSurfaceCapabilitiesKHR( vkSurface );
    	swapChainDetails.surfaceFormats = vkPhysicalDevice.getSurfaceFormatsKHR( vkSurface );
    	swapChainDetails.presentModes = vkPhysicalDevice.getSurfacePresentModesKHR( vkSurface );

    	return swapChainDetails;
    }

    SwapChainPreset VulkanSwapChainFactory::createSuitableSwapChainPreset( 
        const vk::PhysicalDevice& physicalDevice, 
        const vk::SurfaceKHR& surface, 
        const QueueFamilyIndices& queueFamilyIndices,
        const Window& window 
    )
    {
        const SwapChainSupportDetails& swapChainSupportDetails = VulkanSwapChainFactory::querySwapChainSupport(physicalDevice, surface);

        SwapChainPreset swapChainPreset{};
        swapChainPreset.capabilities = swapChainSupportDetails.capabilities;
        swapChainPreset.surfaceFormat = chooseSwapSurfaceFormat( swapChainSupportDetails );
        swapChainPreset.imageCount = chooseImageCount( swapChainSupportDetails );
        swapChainPreset.imageExtent = chooseSwapExtent( swapChainSupportDetails, window );
        swapChainPreset.presentMode = chooseSwapPresentMode( swapChainSupportDetails );


        if( queueFamilyIndices.m_graphicsFamily.value() != queueFamilyIndices.m_presentFamily.value() )
        {
            swapChainPreset.sharingMode = vk::SharingMode::eConcurrent;
            swapChainPreset.queueFamilyIndices.push_back( queueFamilyIndices.m_graphicsFamily.value() );
            swapChainPreset.queueFamilyIndices.push_back( queueFamilyIndices.m_presentFamily.value() );
        }
        else
        {
            swapChainPreset.sharingMode = vk::SharingMode::eExclusive;
        }

        return swapChainPreset;
    }

    vk::SurfaceFormatKHR chooseSwapSurfaceFormat( const SwapChainSupportDetails& swapChainSupportDetails )
    {
        for( const auto& availableFormat : swapChainSupportDetails.surfaceFormats )
        {
            if( availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear )
            {
                return availableFormat;
            }
        }
        return swapChainSupportDetails.surfaceFormats[0];
    }

    vk::PresentModeKHR chooseSwapPresentMode( const SwapChainSupportDetails& swapChainSupportDetails )
    {
        for( const auto& availablePresentMode : swapChainSupportDetails.presentModes )
        {
            if( availablePresentMode == vk::PresentModeKHR::eMailbox )
            {
                return availablePresentMode;
            }
        }
        return vk::PresentModeKHR::eFifo;
    }

    vk::Extent2D chooseSwapExtent( const SwapChainSupportDetails& swapChainSupportDetials, const Window& window )
    {
        const vk::SurfaceCapabilitiesKHR& surfaceCapabilities = swapChainSupportDetials.capabilities;

        if( surfaceCapabilities.currentExtent.width != std::numeric_limits<std::uint32_t>::max() )
        {
            return surfaceCapabilities.currentExtent;
        }
        else
        {
            const auto& [width, height] = window.getFrameBufferSize();

            vk::Extent2D actualExtent{
                width, height
            };

            actualExtent.width = std::clamp(actualExtent.width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width );
            actualExtent.height = std::clamp(actualExtent.height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height );

            return actualExtent;
        }
    }
    
    std::uint32_t chooseImageCount( const SwapChainSupportDetails& swapChainSupportDetails )
    {
        std::uint32_t imageCount = swapChainSupportDetails.capabilities.minImageCount + 1; // always ask for minImageCount + 1

        if( swapChainSupportDetails.capabilities.maxImageCount > 0 && imageCount > swapChainSupportDetails.capabilities.maxImageCount )
        {
            imageCount = swapChainSupportDetails.capabilities.maxImageCount;
        }

        return imageCount;
    }

} // namespace vkrender
