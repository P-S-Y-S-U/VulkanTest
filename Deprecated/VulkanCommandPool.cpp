#include "vkrenderer/VulkanCommandPool.h"

namespace vkrender
{
    VulkanCommandPool::VulkanCommandPool(
        VulkanLogicalDevice* pLogicalDevice,
        const std::uint32_t& queueFamilyIndex
    )
        :m_pLogicalDevice{ pLogicalDevice }
        ,m_commandPoolCreateInfo{}
    {
        populateCommandPoolCreateInfo( queueFamilyIndex );
    }

    VulkanCommandPool::~VulkanCommandPool()
    {
        destroyCommandPool();
    }

    void VulkanCommandPool::createCommandPool()
    {
        m_commandPoolHandle = m_pLogicalDevice->m_deviceHandle.createCommandPool( m_commandPoolCreateInfo );
    }

    void VulkanCommandPool::destroyCommandPool()
    {
        m_pLogicalDevice->m_deviceHandle.destroyCommandPool( m_commandPoolHandle );
    }

    void VulkanCommandPool::populateCommandPoolCreateInfo( const std::uint32_t& queuFamilyIndex )
    {
        m_commandPoolCreateInfo.sType = vk::StructureType::eCommandPoolCreateInfo;
        m_commandPoolCreateInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
        m_commandPoolCreateInfo.queueFamilyIndex = queuFamilyIndex;
    }
} // namespace vkrender
