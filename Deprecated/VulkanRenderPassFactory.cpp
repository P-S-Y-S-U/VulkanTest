#include "vkrenderer/VulkanRenderPassFactory.h"

namespace vkrender
{
    utils::Uptr<VulkanRenderPass> VulkanRenderPassFactory::setupRenderPassForSwapChain( VulkanLogicalDevice* pLogicalDevice, VulkanSwapChain* pSwapchain )
    {
        utils::Sptr<vk::AttachmentDescription[]> colorAttachment{ new vk::AttachmentDescription[1] };
        colorAttachment[0].format = pSwapchain->getImageFormat();
        colorAttachment[0].samples = vk::SampleCountFlagBits::e1;
        colorAttachment[0].loadOp = vk::AttachmentLoadOp::eClear;
        colorAttachment[0].storeOp = vk::AttachmentStoreOp::eStore;
        colorAttachment[0].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
        colorAttachment[0].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        colorAttachment[0].initialLayout = vk::ImageLayout::eUndefined;
        colorAttachment[0].finalLayout = vk::ImageLayout::ePresentSrcKHR;   

        utils::Sptr<vk::AttachmentReference[]> colorAttachmentRef{ new vk::AttachmentReference[1] };
        colorAttachmentRef[0].attachment = 0;
        colorAttachmentRef[0].layout = vk::ImageLayout::eColorAttachmentOptimal;

        utils::Uptr<vk::SubpassDescription[]> subPasses( new vk::SubpassDescription[1] );
        subPasses[0].pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
        subPasses[0].colorAttachmentCount = 1;
        subPasses[0].pColorAttachments = colorAttachmentRef.get();

        utils::Uptr<vk::SubpassDependency[]> subpassDependencies( new vk::SubpassDependency[1] );
        subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        subpassDependencies[0].dstSubpass = 0;
        subpassDependencies[0].srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        subpassDependencies[0].srcAccessMask = vk::AccessFlagBits::eNone;
        subpassDependencies[0].dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        subpassDependencies[0].dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

        utils::Uptr<VulkanRenderPass> pGraphicsRenderPass = std::make_unique<VulkanRenderPass>( 
            pLogicalDevice, 
            colorAttachment, 1, colorAttachmentRef, 1, 
            std::move(subPasses), 1,
            std::move(subpassDependencies), 1
        );

        return std::move(pGraphicsRenderPass);
    }
} // namespace vkrender
