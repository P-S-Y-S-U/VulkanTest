#include "vkrenderer/VulkanSwapChainFactory.h"
#include "vkrenderer/VulkanQueueFamily.h"
#include <limits>
#include <algorithm>

namespace vkrender
{
    utils::Sptr<vk::SwapchainCreateInfoKHR> VulkanSwapChainFactory::createSuitableSwapChainPreset( const VulkanPhysicalDevice& physicalDevice, const VulkanSurface& surface, const Window& window )
    {
        const SwapChainSupportDetails& swapChainSupportDetails = physicalDevice.querySwapChainSupport(surface);

        utils::Sptr<vk::SwapchainCreateInfoKHR> pSwapChainCreateInfo = std::make_shared<vk::SwapchainCreateInfoKHR>();
        pSwapChainCreateInfo->sType = vk::StructureType::eSwapchainCreateInfoKHR;
        pSwapChainCreateInfo->surface = *surface.m_upSurfaceHandle;

        vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat( swapChainSupportDetails );
        pSwapChainCreateInfo->imageFormat = surfaceFormat.format;
        pSwapChainCreateInfo->imageColorSpace = surfaceFormat.colorSpace;
        pSwapChainCreateInfo->minImageCount = chooseImageCount( swapChainSupportDetails );
        pSwapChainCreateInfo->imageExtent = chooseSwapExtent( swapChainSupportDetails, window );
        pSwapChainCreateInfo->imageArrayLayers = 1;
        pSwapChainCreateInfo->imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

        QueueFamilyIndices indices = VulkanQueueFamily::findQueueFamilyIndices( physicalDevice, surface );
        std::uint32_t queueFamilyIndices[] = {
            indices.m_graphicsFamily.value(),
            indices.m_presentFamily.value()
        };

        if( indices.m_graphicsFamily != indices.m_presentFamily )
        {
            pSwapChainCreateInfo->imageSharingMode = vk::SharingMode::eConcurrent;
            pSwapChainCreateInfo->queueFamilyIndexCount = 2;
            pSwapChainCreateInfo->pQueueFamilyIndices = queueFamilyIndices;
        } 
        else
        {
            pSwapChainCreateInfo->imageSharingMode = vk::SharingMode::eExclusive;
            pSwapChainCreateInfo->queueFamilyIndexCount = 0;
            pSwapChainCreateInfo->pQueueFamilyIndices = nullptr;
        }

        pSwapChainCreateInfo->preTransform = swapChainSupportDetails.capabilities.currentTransform;
        pSwapChainCreateInfo->compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
        pSwapChainCreateInfo->presentMode = chooseSwapPresentMode( swapChainSupportDetails );
        pSwapChainCreateInfo->clipped = VK_TRUE;
        pSwapChainCreateInfo->oldSwapchain = nullptr;

        return pSwapChainCreateInfo;
    }

    vk::SurfaceFormatKHR VulkanSwapChainFactory::chooseSwapSurfaceFormat( const SwapChainSupportDetails& swapChainSupportDetails )
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

    vk::PresentModeKHR VulkanSwapChainFactory::chooseSwapPresentMode( const SwapChainSupportDetails& swapChainSupportDetails )
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

    vk::Extent2D VulkanSwapChainFactory::chooseSwapExtent( const SwapChainSupportDetails& swapChainSupportDetials, const Window& window )
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
    
    std::uint32_t VulkanSwapChainFactory::chooseImageCount( const SwapChainSupportDetails& swapChainSupportDetails )
    {
        std::uint32_t imageCount = swapChainSupportDetails.capabilities.minImageCount + 1; // always ask for minImageCount + 1

        if( swapChainSupportDetails.capabilities.maxImageCount > 0 && imageCount > swapChainSupportDetails.capabilities.maxImageCount )
        {
            imageCount = swapChainSupportDetails.capabilities.maxImageCount;
        }

        return imageCount;
    }

} // namespace vkrender
