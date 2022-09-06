#include "vkrenderer/VulkanSwapChain.h"
#include "utilities/VulkanLogger.h"

namespace vkrender
{
    VulkanSwapChain::VulkanSwapChain( 
        VulkanLogicalDevice* pLogicalDevice, 
        const utils::Sptr<vk::SwapchainCreateInfoKHR>& pSwapChainCreateInfo 
    )
        :m_pLogicalDevice{ pLogicalDevice }
        ,m_spSwapChainCreateInfo{ pSwapChainCreateInfo }
    {}

    VulkanSwapChain::~VulkanSwapChain()
    {
        destroySwapChain();
    }

    void VulkanSwapChain::createSwapChain()
    {
        m_swapchainHandle = m_pLogicalDevice->m_deviceHandle.createSwapchainKHR( *m_spSwapChainCreateInfo );
    }
    
    void VulkanSwapChain::destroySwapChain()
    {
        m_pLogicalDevice->m_deviceHandle.destroySwapchainKHR( m_swapchainHandle );
    }

} // namespace vkrender
