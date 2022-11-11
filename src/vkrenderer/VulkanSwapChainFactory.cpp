#include "vkrenderer/VulkanSwapChainFactory.h"
#include "vkrenderer/VulkanQueueFamily.hpp"
#include <limits>
#include <algorithm>

namespace vkrender
{
    

    vk::SwapchainCreateInfoKHR VulkanSwapChainFactory::createSuitableSwapChainPreset( 
        const vk::PhysicalDevice& physicalDevice, 
        const vk::SurfaceKHR& surface, 
        const QueueFamilyIndices& queueFamilyIndices,
        const Window& window 
    )
    {
        const SwapChainSupportDetails& swapChainSupportDetails = VulkanSwapChainFactory::querySwapChainSupport(physicalDevice, surface);

        const vk::SurfaceCapabilitiesKHR& capabilities = swapChainSupportDetails.capabilities;
        vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat( swapChainSupportDetails );
        std::uint32_t imageCount = chooseImageCount( swapChainSupportDetails );
        vk::Extent2D imageExtent = chooseSwapExtent( swapChainSupportDetails, window );
        vk::PresentModeKHR presentMode = chooseSwapPresentMode( swapChainSupportDetails );

        vk::SharingMode sharingMode;
        if( queueFamilyIndices.m_graphicsFamily.value() != queueFamilyIndices.m_presentFamily.value() )
        {
            sharingMode = vk::SharingMode::eConcurrent;
            swapChainPreset.queueFamilyIndices.push_back( queueFamilyIndices.m_graphicsFamily.value() );
            swapChainPreset.queueFamilyIndices.push_back( queueFamilyIndices.m_presentFamily.value() );
        }
        else
        {
            swapChainPreset.sharingMode = vk::SharingMode::eExclusive;
        }

        vk::SwapchainCreateInfoKHR swapChainPreset{};
        swapChainPreset.sType = vk::StructureType::eSwapchainCreateInfoKHR;
        swapChainPreset. = swapChainSupportDetails.capabilities;
        swapChainPreset.surfaceFormat = 
        swapChainPreset.imageCount = 
        swapChainPreset.imageExtent = 
        swapChainPreset.presentMode = 


        

        return swapChainPreset;
    }

    

} // namespace vkrender
