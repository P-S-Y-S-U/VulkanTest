#ifndef VKRENDER_VULKAN_GRAPHICS_PIPELINE_H
#define VKRENDER_VULKAN_GRAPHICS_PIPELINE_H

#include <vulkan/vulkan.hpp>

#include "exports.hpp"
#include "vkrenderer/VulkanLogicalDevice.h"
#include "vkrenderer/VulkanRenderPass.h"
#include "vkrenderer/VulkanShaderModule.h"
#include "vkrenderer/VulkanPipelineLayout.h"

namespace vkrender
{
    class VULKAN_EXPORTS VulkanGraphicsPipeline
    {
    public: 
        struct PipelineShaderStage
        {
            utils::Sptr<VulkanShaderModule> pVertexShader;
            utils::Sptr<VulkanShaderModule> pFragmentShader;
        };
        struct PipelinePrimitiveDescriptor
        {
            vk::PrimitiveTopology primitiveTopology;
            bool primitiveRestartEnable;
        };
        struct PipelineViewportStage
        {
            std::vector<vk::Viewport> viewports;
            std::vector<vk::Rect2D> scissors;
        };

        VulkanGraphicsPipeline( VulkanLogicalDevice* pLogicalDevice, VulkanRenderPass* pRenderPass, VulkanPipelineLayout* pPipelineLayout );
        ~VulkanGraphicsPipeline();

        void createGraphicsPipeline(
            const PipelineShaderStage& shaderStage,
            const std::vector<vk::DynamicState>& dynamicStates,
            const PipelinePrimitiveDescriptor& primitiveDescriptor,
            const PipelineViewportStage& viewportStage
        );

        void destroyGraphicsPipeline();
    private:
        vk::Pipeline m_pipelineHandle;
        VulkanLogicalDevice* m_pLogicalDevice;
        VulkanRenderPass* m_pRenderPass;
        
        VulkanPipelineLayout* m_pPipelineLayout;

        utils::Uptr<vk::PipelineShaderStageCreateInfo[]> m_pShaderStages;
        std::uint16_t m_shaderStageCount;
        vk::PipelineVertexInputStateCreateInfo  m_vertexInputState;
        vk::PipelineInputAssemblyStateCreateInfo m_inputAssemblyState;
        vk::PipelineViewportStateCreateInfo     m_viewportState;
        std::vector<vk::Viewport>   m_viewports;
        std::vector<vk::Rect2D>     m_scissors;
        vk::PipelineRasterizationStateCreateInfo    m_rasterizerState;
        vk::PipelineMultisampleStateCreateInfo  m_multisampleState;
        vk::PipelineColorBlendAttachmentState   m_colorBlendAttachment;
        vk::PipelineColorBlendStateCreateInfo   m_colorBlendState;
        vk::PipelineDynamicStateCreateInfo  m_dynamicState;
        std::vector<vk::DynamicState> m_dynamicStatesData;

        utils::Sptr<vk::GraphicsPipelineCreateInfo> m_spGraphicsPipelineCreateInfo;

        void populatePipelineStatesInfo(
            const PipelineShaderStage& shaderStage,
            const std::vector<vk::DynamicState>& dynamicStates,
            const PipelinePrimitiveDescriptor& primitiveDescriptor,
            const PipelineViewportStage& viewportStage
        );
        void populateGraphicsPipelineCreateInfo();

        utils::Uptr<vk::PipelineShaderStageCreateInfo[]> populatePipelineShaderStageInfo( const PipelineShaderStage& shaderStage );
    };
} // namespace vkrender


#endif 