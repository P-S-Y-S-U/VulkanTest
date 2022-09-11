#ifndef VKRENDER_VULKAN_PIPELINE_LAYOUT_H
#define VKRENDER_VULKAN_PIPELINE_LAYOUT_H

#include <vulkan/vulkan.hpp>

#include "exports.hpp"
#include "VulkanLogicalDevice.h"

namespace vkrender
{
    class VULKAN_EXPORTS VulkanPipelineLayout
    {
    public:
        explicit VulkanPipelineLayout( VulkanLogicalDevice* pLogicalDevice );
        ~VulkanPipelineLayout();

        void createPipelineLayout();
        void destroyPipelineLayout();

    private:
        vk::PipelineLayout  m_pipelineLayoutHandle;

        VulkanLogicalDevice* m_pLogicalDevice;
        utils::Sptr<vk::PipelineLayoutCreateInfo> m_spPipelineLayoutCreateInfo;

        void populatePipelineLayoutCreateInfo();
    };
} // namespace vkrender

#endif