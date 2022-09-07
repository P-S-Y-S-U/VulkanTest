#include "vkrenderer/VulkanSwapChain.h"
#include "utilities/VulkanLogger.h"

namespace vkrender
{
    VulkanSwapChain::VulkanSwapChain( 
        const utils::Sptr<SwapChainPreset>& pPreset,
        VulkanLogicalDevice* pLogicalDevice, 
        VulkanSurface* pSurface
    )
        :m_spSwapChainPreset{ pPreset }
        ,m_pLogicalDevice{ pLogicalDevice }
        ,m_pSurface{ pSurface }
    {}

    VulkanSwapChain::~VulkanSwapChain()
    {
        destroySwapChain();
    }

    void VulkanSwapChain::createSwapChain()
    {
        populateSwapChainCreateInfo();
        m_swapchainHandle = m_pLogicalDevice->m_deviceHandle.createSwapchainKHR( *m_spSwapChainCreateInfo );
    }
    
    void VulkanSwapChain::destroySwapChain()
    {
        m_pLogicalDevice->m_deviceHandle.destroySwapchainKHR( m_swapchainHandle );
    }

    void VulkanSwapChain::populateSwapChainCreateInfo()
    {
        m_spSwapChainCreateInfo = std::make_shared<vk::SwapchainCreateInfoKHR>();
        m_spSwapChainCreateInfo->sType = vk::StructureType::eSwapchainCreateInfoKHR;
        m_spSwapChainCreateInfo->surface = *m_pSurface->m_upSurfaceHandle;

        const vk::SurfaceFormatKHR& surfaceFormat = m_spSwapChainPreset->surfaceFormat;

        m_spSwapChainCreateInfo->imageFormat = surfaceFormat.format;
        m_spSwapChainCreateInfo->imageColorSpace = surfaceFormat.colorSpace;
        m_spSwapChainCreateInfo->minImageCount = m_spSwapChainPreset->imageCount;
        m_spSwapChainCreateInfo->imageExtent = m_spSwapChainPreset->imageExtent;
        m_spSwapChainCreateInfo->imageArrayLayers = 1;
        m_spSwapChainCreateInfo->imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
        m_spSwapChainCreateInfo->imageSharingMode = vk::SharingMode::eExclusive;
        m_spSwapChainCreateInfo->queueFamilyIndexCount = m_spSwapChainPreset->queueFamilyIndices.size();
        m_spSwapChainCreateInfo->pQueueFamilyIndices = m_spSwapChainPreset->queueFamilyIndices.data();
        m_spSwapChainCreateInfo->preTransform = m_spSwapChainPreset->capabilities.currentTransform;
        m_spSwapChainCreateInfo->compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
        m_spSwapChainCreateInfo->presentMode = m_spSwapChainPreset->presentMode;
        m_spSwapChainCreateInfo->clipped = VK_TRUE;
        m_spSwapChainCreateInfo->oldSwapchain = nullptr;
    }

} // namespace vkrender
