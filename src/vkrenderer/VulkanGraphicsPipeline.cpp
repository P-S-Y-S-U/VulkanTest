#include "vkrenderer/VulkanGraphicsPipeline.h"
#include <algorithm>

namespace vkrender
{
    VulkanGraphicsPipeline::VulkanGraphicsPipeline( VulkanLogicalDevice* pLogicalDevice, VulkanRenderPass* pRenderPass, VulkanPipelineLayout* pPipelineLayout )
        :m_pLogicalDevice{ pLogicalDevice }
        ,m_pRenderPass{ pRenderPass }
        ,m_pPipelineLayout{ pPipelineLayout }
    {}

    VulkanGraphicsPipeline::~VulkanGraphicsPipeline()
    {
        destroyGraphicsPipeline();
    }

    void VulkanGraphicsPipeline::createGraphicsPipeline(
        const PipelineShaderStage& shaderStage,
        const std::vector<vk::DynamicState>& dynamicStates,
        const PipelinePrimitiveDescriptor& primitiveDescriptor,
        const PipelineViewportStage& viewportStage
    )
    {
        populatePipelineStatesInfo( shaderStage, dynamicStates, primitiveDescriptor, viewportStage );
        populateGraphicsPipelineCreateInfo();
        if( auto vkResultType = m_pLogicalDevice->m_deviceHandle.createGraphicsPipeline( nullptr, *m_spGraphicsPipelineCreateInfo ); vkResultType.result == vk::Result::eSuccess )
        {
            m_pipelineHandle = vkResultType.value;
        } 
        else
        {
            throw std::runtime_error("FAILED TO CREATE GRAPHICS PIPELINE!");
        }
    }

    void VulkanGraphicsPipeline::destroyGraphicsPipeline()
    {
        m_pLogicalDevice->m_deviceHandle.destroyPipeline(m_pipelineHandle);
    }

    void VulkanGraphicsPipeline::populatePipelineStatesInfo( 
        const PipelineShaderStage& shaderStage,
        const std::vector<vk::DynamicState>& dynamicStates,
        const PipelinePrimitiveDescriptor& primitiveDescriptor,
        const PipelineViewportStage& viewportStage
    )
    {
        m_pShaderStages = std::move( populatePipelineShaderStageInfo( shaderStage ) );
        m_dynamicStatesData = dynamicStates;

        m_dynamicState.sType = vk::StructureType::ePipelineDynamicStateCreateInfo;
        m_dynamicState.dynamicStateCount = m_dynamicStatesData.size();
        m_dynamicState.pDynamicStates = m_dynamicStatesData.data();

        m_vertexInputState.sType = vk::StructureType::ePipelineVertexInputStateCreateInfo;
        m_vertexInputState.vertexBindingDescriptionCount = 0;
        m_vertexInputState.pVertexBindingDescriptions = nullptr;
        m_vertexInputState.vertexAttributeDescriptionCount = 0;
        m_vertexInputState.pVertexAttributeDescriptions = nullptr;

        m_inputAssemblyState.sType = vk::StructureType::ePipelineInputAssemblyStateCreateInfo;
        m_inputAssemblyState.topology = primitiveDescriptor.primitiveTopology;
        m_inputAssemblyState.primitiveRestartEnable = static_cast<vk::Bool32>( m_inputAssemblyState.primitiveRestartEnable );

        m_viewportState.sType = vk::StructureType::ePipelineViewportStateCreateInfo;
        m_viewportState.viewportCount = viewportStage.viewports.size();
        m_viewportState.scissorCount = viewportStage.scissors.size();

        auto dynamicViewportStates = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };

        if( std::search( dynamicStates.begin(), dynamicStates.end(), dynamicViewportStates.begin(), dynamicViewportStates.end() ) == dynamicStates.end() )
        {
            // has no dynamic states for viewport
            m_viewports = viewportStage.viewports;
            m_scissors = viewportStage.scissors;
            m_viewportState.pViewports = m_viewports.data();
            m_viewportState.pScissors = m_scissors.data();
        }

        m_rasterizerState.sType = vk::StructureType::ePipelineRasterizationStateCreateInfo;
        m_rasterizerState.depthClampEnable = VK_FALSE;
        m_rasterizerState.rasterizerDiscardEnable = VK_FALSE;
        m_rasterizerState.polygonMode = vk::PolygonMode::eFill;
        m_rasterizerState.lineWidth = 1.0f;
        m_rasterizerState.cullMode = vk::CullModeFlagBits::eBack;
        m_rasterizerState.frontFace = vk::FrontFace::eClockwise;
        m_rasterizerState.depthBiasEnable = VK_FALSE;
        m_rasterizerState.depthBiasConstantFactor = 0.0f;
        m_rasterizerState.depthBiasClamp = 0.0f;
        m_rasterizerState.depthBiasSlopeFactor = 0.0f;

        m_multisampleState.sType = vk::StructureType::ePipelineMultisampleStateCreateInfo;
        m_multisampleState.sampleShadingEnable = VK_FALSE;
        m_multisampleState.rasterizationSamples = vk::SampleCountFlagBits::e1;
        m_multisampleState.minSampleShading = 1.0f;
        m_multisampleState.pSampleMask = nullptr;
        m_multisampleState.alphaToCoverageEnable = VK_FALSE;
        m_multisampleState.alphaToOneEnable = VK_FALSE;

        m_colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eA;
        m_colorBlendAttachment.blendEnable = VK_FALSE;
        m_colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eOne;
        m_colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eZero;
        m_colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;
        m_colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
        m_colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
        m_colorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd;

        m_colorBlendState.sType = vk::StructureType::ePipelineColorBlendStateCreateInfo;
        m_colorBlendState.logicOpEnable = VK_FALSE;
        m_colorBlendState.logicOp = vk::LogicOp::eCopy;
        m_colorBlendState.attachmentCount = 1;
        m_colorBlendState.pAttachments = &m_colorBlendAttachment;
        m_colorBlendState.blendConstants[0] = 0.0f;
        m_colorBlendState.blendConstants[1] = 0.0f;
        m_colorBlendState.blendConstants[2] = 0.0f;
        m_colorBlendState.blendConstants[3] = 0.0f;
    }

    void VulkanGraphicsPipeline::populateGraphicsPipelineCreateInfo()
    {
        m_spGraphicsPipelineCreateInfo = std::make_shared<vk::GraphicsPipelineCreateInfo>();
        m_spGraphicsPipelineCreateInfo->stageCount = m_shaderStageCount;
        m_spGraphicsPipelineCreateInfo->pStages = m_pShaderStages.get();
        m_spGraphicsPipelineCreateInfo->pVertexInputState = &m_vertexInputState;
        m_spGraphicsPipelineCreateInfo->pInputAssemblyState = &m_inputAssemblyState;
        m_spGraphicsPipelineCreateInfo->pViewportState = &m_viewportState;
        m_spGraphicsPipelineCreateInfo->pRasterizationState = &m_rasterizerState;
        m_spGraphicsPipelineCreateInfo->pMultisampleState = &m_multisampleState;
        m_spGraphicsPipelineCreateInfo->pDepthStencilState = nullptr;
        m_spGraphicsPipelineCreateInfo->pColorBlendState = &m_colorBlendState;
        m_spGraphicsPipelineCreateInfo->pDynamicState = &m_dynamicState;
        m_spGraphicsPipelineCreateInfo->layout = m_pPipelineLayout->m_pipelineLayoutHandle;
        m_spGraphicsPipelineCreateInfo->renderPass = m_pRenderPass->m_renderPassHandle;
        m_spGraphicsPipelineCreateInfo->subpass = 0;
        m_spGraphicsPipelineCreateInfo->basePipelineHandle = nullptr;
        m_spGraphicsPipelineCreateInfo->basePipelineIndex = -1;
    }

    utils::Uptr<vk::PipelineShaderStageCreateInfo[]> VulkanGraphicsPipeline::populatePipelineShaderStageInfo( const PipelineShaderStage& shaderStage )
    {
        m_shaderStageCount = 0;

        std::optional<vk::PipelineShaderStageCreateInfo> vertexStage;
        std::optional<vk::PipelineShaderStageCreateInfo> fragmentStage;

        if( shaderStage.pVertexShader.get() ) 
        {
            vertexStage = vk::PipelineShaderStageCreateInfo{};
            vertexStage->sType = vk::StructureType::ePipelineShaderStageCreateInfo;
            vertexStage->stage = vk::ShaderStageFlagBits::eVertex;
            vertexStage->module = shaderStage.pVertexShader->m_shaderModuleHandle;
            vertexStage->pName = "main";
            m_shaderStageCount++;
        }
        if( shaderStage.pFragmentShader.get() ) 
        {
            fragmentStage = vk::PipelineShaderStageCreateInfo{};
            fragmentStage->sType = vk::StructureType::ePipelineShaderStageCreateInfo;
            fragmentStage->stage = vk::ShaderStageFlagBits::eFragment;
            fragmentStage->module = shaderStage.pFragmentShader->m_shaderModuleHandle;
            fragmentStage->pName = "main";
            m_shaderStageCount++;
        }

        utils::Uptr<vk::PipelineShaderStageCreateInfo[]> upShaderStages( new vk::PipelineShaderStageCreateInfo[m_shaderStageCount] );

        if( vertexStage.has_value() )
        {
            upShaderStages[0] = vertexStage.value();
        }

        if( fragmentStage.has_value() )
        {
            upShaderStages[1] = fragmentStage.value();
        }

        return std::move(upShaderStages);
    }
} // namespace vkrender
