#ifndef VKRENDER_VULKAN_SWAP_CHAIN_H
#define VKRENDER_VULKAN_SWAP_CHAIN_H

#include <vulkan/vulkan.hpp>

#include "exports.hpp"
#include "utilities/memory.hpp"
#include "vkrenderer/VulkanLogicalDevice.h"

namespace vkrender
{
    class VULKAN_EXPORTS VulkanSwapChain
    {
    public:
        explicit VulkanSwapChain( VulkanLogicalDevice* pLogicalDevice, const utils::Sptr<vk::SwapchainCreateInfoKHR>& pSwapChainCreateInfo );
        ~VulkanSwapChain();

        void createSwapChain();
        void destroySwapChain();

    private:
        vk::SwapchainKHR    m_swapchainHandle;
        VulkanLogicalDevice* m_pLogicalDevice;

        utils::Sptr<vk::SwapchainCreateInfoKHR> m_spSwapChainCreateInfo;
    };
} // namespace vkrender

#endif 