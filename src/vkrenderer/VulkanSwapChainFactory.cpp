#include "vkrenderer/VulkanSwapChainFactory.h"
#include "vkrenderer/VulkanQueueFamily.h"
#include <limits>
#include <algorithm>

namespace vkrender
{
    vk::SurfaceFormatKHR chooseSwapSurfaceFormat( const SwapChainSupportDetails& swapChainSupportDetails );
    vk::PresentModeKHR chooseSwapPresentMode( const SwapChainSupportDetails& swapChainSupportDetails );
    vk::Extent2D chooseSwapExtent( const SwapChainSupportDetails& swapChainSupportDetails, const Window& window );
    std::uint32_t chooseImageCount( const SwapChainSupportDetails& swapChainSupportDetails );

    utils::Sptr<SwapChainPreset> VulkanSwapChainFactory::createSuitableSwapChainPreset( const VulkanPhysicalDevice& physicalDevice, const VulkanSurface& surface, const Window& window )
    {
        const SwapChainSupportDetails& swapChainSupportDetails = physicalDevice.querySwapChainSupport(surface);

        utils::Sptr<SwapChainPreset> pPreset = std::make_shared<SwapChainPreset>();
        pPreset->capabilities = swapChainSupportDetails.capabilities;
        pPreset->surfaceFormat = chooseSwapSurfaceFormat( swapChainSupportDetails );
        pPreset->imageCount = chooseImageCount( swapChainSupportDetails );
        pPreset->imageExtent = chooseSwapExtent( swapChainSupportDetails, window );
        pPreset->presentMode = chooseSwapPresentMode( swapChainSupportDetails );

        QueueFamilyIndices indices = VulkanQueueFamily::findQueueFamilyIndices( physicalDevice, surface );

        if( indices.m_graphicsFamily.value() != indices.m_presentFamily.value() )
        {
            pPreset->sharingMode = vk::SharingMode::eConcurrent;
            pPreset->queueFamilyIndices.push_back( indices.m_graphicsFamily.value() );
            pPreset->queueFamilyIndices.push_back( indices.m_presentFamily.value() );
        }
        else
        {
            pPreset->sharingMode = vk::SharingMode::eExclusive;
        }

        return pPreset;
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
