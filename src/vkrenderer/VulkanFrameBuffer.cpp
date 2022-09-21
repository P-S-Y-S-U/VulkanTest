#include "vkrenderer/VulkanFrameBuffer.h"

namespace vkrender
{
    VulkanFrameBuffer::VulkanFrameBuffer( 
        VulkanLogicalDevice* pLogicalDevice, 
        VulkanRenderPass* pRenderPass, 
        const std::vector<VulkanImageView*>& imageViews,
        const vk::Extent2D& swapchainExtent
    )
        :m_pLogicalDevice{ pLogicalDevice }
        ,m_pGraphicsRenderPass{ pRenderPass }
        ,m_numOfImageViews{ static_cast<std::uint32_t>( imageViews.size() ) }
        ,m_swapChainExtent{ swapchainExtent }
    {
        m_swapChainImageViews = new vk::ImageView[m_numOfImageViews];

        for( auto i = 0; i < m_numOfImageViews; i++ )
        {
            m_swapChainImageViews[i] = imageViews[i]->m_imageViewHandle;
        }  
    }
    
    VulkanFrameBuffer::~VulkanFrameBuffer()
    {
        destroyFrameBuffer();
    }

    void VulkanFrameBuffer::createFrameBuffer()
    {
        populateFrameBufferCreateInfo();
        
        m_frameBufferHandle = m_pLogicalDevice->m_deviceHandle.createFramebuffer( *m_spFrameBufferCreateInfo );
    }

    void VulkanFrameBuffer::destroyFrameBuffer()
    {
        m_pLogicalDevice->m_deviceHandle.destroyFramebuffer( m_frameBufferHandle );
    }

    void VulkanFrameBuffer::populateFrameBufferCreateInfo()
    {
        m_spFrameBufferCreateInfo = std::make_shared<vk::FramebufferCreateInfo>();
        m_spFrameBufferCreateInfo->sType = vk::StructureType::eFramebufferCreateInfo;
        m_spFrameBufferCreateInfo->renderPass = m_pGraphicsRenderPass->m_renderPassHandle;
        m_spFrameBufferCreateInfo->attachmentCount = m_numOfImageViews;
        m_spFrameBufferCreateInfo->pAttachments = m_swapChainImageViews;
        m_spFrameBufferCreateInfo->width = m_swapChainExtent.width;
        m_spFrameBufferCreateInfo->height = m_swapChainExtent.height;
        m_spFrameBufferCreateInfo->layers = 1;
    }
} // namespace vkrender
