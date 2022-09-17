#ifndef VKRENDER_VULKAN_RENDER_PASS_H
#define VKRENDER_VULKAN_RENDER_PASS_H

#include <vulkan/vulkan.hpp>

#include "exports.hpp"
#include "utilities/memory.hpp"
#include "vkrenderer/VulkanLogicalDevice.h"

namespace vkrender
{
    class VULKAN_EXPORTS VulkanRenderPass
    {
    public:
        VulkanRenderPass( 
            VulkanLogicalDevice* pLogicalDevice, 
            utils::Sptr<vk::AttachmentDescription[]> attachments,
            std::uint32_t numOfAttachments, 
            utils::Sptr<vk::AttachmentReference[]> attachmentReferences,
            std::uint32_t numOfAttachmentReferences,
            utils::Uptr<vk::SubpassDescription[]>  subpasses,
            std::uint32_t numOfSubpasses
        );
        ~VulkanRenderPass();

        void createRenderPass();
        void destroyRenderPass();
    private:
        vk::RenderPass m_renderPassHandle;
        VulkanLogicalDevice* m_pLogicalDevice;

        utils::Sptr<vk::AttachmentDescription[]> m_attachments;
        std::uint32_t m_numOfAttachments;
        utils::Sptr<vk::AttachmentReference[]>  m_attachmentReferences;
        std::uint32_t m_numOfAttachmentReferences;
        utils::Uptr<vk::SubpassDescription[]> m_subpasses;
        std::uint32_t m_numOfSubpasses;

        utils::Sptr<vk::RenderPassCreateInfo> m_spRenderPassCreateInfo;

        void populateRenderPassCreateInfo();
    }; 
} // namespace vkrender


#endif 