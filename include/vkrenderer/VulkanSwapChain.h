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
        explicit VulkanSwapChain( const utils::Sptr<SwapChainPreset>& pPreset, VulkanLogicalDevice* pLogicalDevice, VulkanSurface* pSurface );
        ~VulkanSwapChain();

        void createSwapChain();
        void destroySwapChain();

        vk::Extent2D getSwapChainExtent() const { return m_spSwapChainPreset->imageExtent; }
        const std::vector<vk::Image>& getSwapChainImages() const { return m_swapChainImages; }
        vk::Format getImageFormat() const { return m_spSwapChainPreset->surfaceFormat.format; }
    private:
        vk::SwapchainKHR    m_swapchainHandle;
        VulkanLogicalDevice* m_pLogicalDevice;
        VulkanSurface* m_pSurface;

        utils::Sptr<SwapChainPreset> m_spSwapChainPreset;
        utils::Sptr<vk::SwapchainCreateInfoKHR> m_spSwapChainCreateInfo;
        
        std::vector<vk::Image> m_swapChainImages;

        void populateSwapChainCreateInfo();
    };
} // namespace vkrender

#endif 