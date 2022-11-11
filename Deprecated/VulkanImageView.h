#ifndef VKRENDER_VULKAN_IMAGE_VIEW_H
#define VKRENDER_VULKAN_IMAGE_VIEW_H

#include <vulkan/vulkan.hpp>

#include "exports.hpp"
#include "vkrenderer/VulkanLogicalDevice.h"

namespace vkrender
{
    class VULKAN_EXPORTS VulkanImageView
    {
    public:
        explicit VulkanImageView( const vk::Image& vkImage, const vk::Format& vkFormat, VulkanLogicalDevice* pLogicalDevice );
        ~VulkanImageView();

        void createImageView();
        void destroyImageView();

    private:
        vk::ImageView m_imageViewHandle;
        VulkanLogicalDevice* m_pLogicalDevice;

        utils::Sptr<vk::ImageViewCreateInfo> m_spImageViewCreateInfo;

        void populateImageViewCreateInfo(const vk::Image& vkImage, const vk::Format& vkFormat);

        friend class VulkanFrameBuffer;
    };
} // namespace vkrender

#endif 