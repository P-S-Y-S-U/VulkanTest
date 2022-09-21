#ifndef VKRENDER_VULKAN_FRAMEBUFFER_H
#define VKRENDER_VULKAN_FRAMEBUFFER_H

#include <vulkan/vulkan.hpp>

#include "exports.hpp"
#include "vkrenderer/VulkanLogicalDevice.h"
#include "vkrenderer/VulkanImageView.h"
#include "vkrenderer/VulkanRenderPass.h"
#include <vector>

namespace vkrender
{
    class VULKAN_EXPORTS VulkanFrameBuffer
    {
    public:
        VulkanFrameBuffer( 
            VulkanLogicalDevice* pLogicalDevice, 
            VulkanRenderPass* pRenderPass, 
            const std::vector<VulkanImageView*>& imageViews,
            const vk::Extent2D& swapChainExtent );
        ~VulkanFrameBuffer();

        void createFrameBuffer();
        void destroyFrameBuffer();
    private:
        vk::Framebuffer m_frameBufferHandle;
        VulkanLogicalDevice* m_pLogicalDevice;
        VulkanRenderPass* m_pGraphicsRenderPass;

        std::uint32_t m_numOfImageViews;
        vk::ImageView* m_swapChainImageViews;

        vk::Extent2D m_swapChainExtent;

        utils::Sptr<vk::FramebufferCreateInfo> m_spFrameBufferCreateInfo;
        void populateFrameBufferCreateInfo();
    }; 
} // namespace vkrender

#endif