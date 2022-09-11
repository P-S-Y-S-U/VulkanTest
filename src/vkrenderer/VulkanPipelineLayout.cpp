#include "vkrenderer/VulkanPipelineLayout.h"

namespace vkrender
{
    VulkanPipelineLayout::VulkanPipelineLayout( VulkanLogicalDevice* pLogicalDevice )
        :m_pLogicalDevice{ pLogicalDevice }
    {}    
    
    VulkanPipelineLayout::~VulkanPipelineLayout()
    {
        destroyPipelineLayout();
    }

    void VulkanPipelineLayout::createPipelineLayout()
    {
        populatePipelineLayoutCreateInfo();
        m_pipelineLayoutHandle = m_pLogicalDevice->m_deviceHandle.createPipelineLayout( *m_spPipelineLayoutCreateInfo );
    }

    void VulkanPipelineLayout::destroyPipelineLayout()
    {
        m_pLogicalDevice->m_deviceHandle.destroyPipelineLayout( m_pipelineLayoutHandle );
    }

    void VulkanPipelineLayout::populatePipelineLayoutCreateInfo()
    {
        m_spPipelineLayoutCreateInfo = std::make_shared<vk::PipelineLayoutCreateInfo>();
        m_spPipelineLayoutCreateInfo->sType = vk::StructureType::ePipelineLayoutCreateInfo;
        m_spPipelineLayoutCreateInfo->setLayoutCount = 0;
        m_spPipelineLayoutCreateInfo->pSetLayouts = nullptr;
        m_spPipelineLayoutCreateInfo->pushConstantRangeCount = 0;
        m_spPipelineLayoutCreateInfo->pPushConstantRanges = nullptr;
    }
    
} // namespace vkrender
