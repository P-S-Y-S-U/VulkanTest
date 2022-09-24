#include "vkrenderer/VulkanRenderPass.h"

namespace vkrender
{
    VulkanRenderPass::VulkanRenderPass(
        VulkanLogicalDevice* pLogicalDevice, 
        utils::Sptr<vk::AttachmentDescription[]> attachments,
        std::uint32_t numOfAttachments, 
        utils::Sptr<vk::AttachmentReference[]> attachmentReferences,
        std::uint32_t numOfAttachmentReferences,
        utils::Uptr<vk::SubpassDescription[]>  subpasses,
        std::uint32_t numOfSubpasses,
        utils::Uptr<vk::SubpassDependency[]> subpassDependencies,
        std::uint32_t numOfSubpassDependencies
    )
        :m_pLogicalDevice{ pLogicalDevice }
        ,m_attachments{ attachments }
        ,m_numOfAttachments{ numOfAttachments }
        ,m_attachmentReferences{ attachmentReferences }
        ,m_numOfAttachmentReferences{ numOfAttachmentReferences }
        ,m_subpasses{ std::move(subpasses ) }
        ,m_numOfSubpasses{ numOfSubpasses }
        ,m_subpassDependencies{ std::move(subpassDependencies) }
        ,m_numOfDependencies{ numOfSubpassDependencies }
    {}

    VulkanRenderPass::~VulkanRenderPass()
    {
        destroyRenderPass();
    }

    void VulkanRenderPass::createRenderPass()
    {
        populateRenderPassCreateInfo();
        m_renderPassHandle = m_pLogicalDevice->m_deviceHandle.createRenderPass( *m_spRenderPassCreateInfo );
    }

    void VulkanRenderPass::destroyRenderPass()
    {
        m_pLogicalDevice->m_deviceHandle.destroyRenderPass( m_renderPassHandle );
    }

    void VulkanRenderPass::populateRenderPassCreateInfo()
    {
        m_spRenderPassCreateInfo = std::make_shared<vk::RenderPassCreateInfo>();
        m_spRenderPassCreateInfo->sType = vk::StructureType::eRenderPassCreateInfo;
        m_spRenderPassCreateInfo->attachmentCount = m_numOfAttachments;
        m_spRenderPassCreateInfo->pAttachments = m_attachments.get();
        m_spRenderPassCreateInfo->subpassCount = m_numOfSubpasses;
        m_spRenderPassCreateInfo->pSubpasses = m_subpasses.get();
        m_spRenderPassCreateInfo->dependencyCount = m_numOfDependencies;
        m_spRenderPassCreateInfo->pDependencies = m_subpassDependencies.get();
    }
} // namespace vkrender
