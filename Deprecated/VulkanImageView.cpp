#include "vkrenderer/VulkanImageView.h"

namespace vkrender
{
    VulkanImageView::VulkanImageView( const vk::Image& vkImage, const vk::Format& vkFormat, VulkanLogicalDevice* pLogicalDevice )
        :m_pLogicalDevice{ pLogicalDevice }
    {
        populateImageViewCreateInfo(vkImage, vkFormat);
    }
    
    VulkanImageView::~VulkanImageView()
    {
        destroyImageView();
    }

    void VulkanImageView::createImageView()
    {
        m_imageViewHandle = m_pLogicalDevice->m_deviceHandle.createImageView( *m_spImageViewCreateInfo.get() );
    }

    void VulkanImageView::destroyImageView()
    {
        m_pLogicalDevice->m_deviceHandle.destroyImageView( m_imageViewHandle );
    }

    void VulkanImageView::populateImageViewCreateInfo( const vk::Image& vkImage, const vk::Format& vkFormat )
    {
        m_spImageViewCreateInfo = std::make_shared<vk::ImageViewCreateInfo>();
        m_spImageViewCreateInfo->sType = vk::StructureType::eImageViewCreateInfo;
        m_spImageViewCreateInfo->image = vkImage;
        m_spImageViewCreateInfo->viewType = vk::ImageViewType::e2D;
        m_spImageViewCreateInfo->format = vkFormat;

        vk::ComponentMapping componentMap;
        componentMap.r = vk::ComponentSwizzle::eIdentity;
        componentMap.g = vk::ComponentSwizzle::eIdentity;
        componentMap.b = vk::ComponentSwizzle::eIdentity;
        componentMap.a = vk::ComponentSwizzle::eIdentity;

        m_spImageViewCreateInfo->components = componentMap;
        
        vk::ImageSubresourceRange subResourceRange;
        subResourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        subResourceRange.baseMipLevel = 0;
        subResourceRange.levelCount = 1;
        subResourceRange.baseArrayLayer = 0;
        subResourceRange.layerCount = 1;

        m_spImageViewCreateInfo->subresourceRange = subResourceRange;
    }

} // namespace vkrender
