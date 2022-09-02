#include "vkrenderer/VulkanLogicalDeviceManager.h"

namespace vkrender
{
    VulkanLogicalDeviceManager::VulkanLogicalDeviceManager()
    {}

    VulkanLogicalDeviceManager::~VulkanLogicalDeviceManager()
    {
        destroyAllLogicalDevices();
    }

    void VulkanLogicalDeviceManager::createLogicalDevice( VulkanPhysicalDevice* pPhysicalDevice )
    {
        LogicalDevicePtr upLogicalDevice = std::make_unique<VulkanLogicalDevice>( pPhysicalDevice );

        upLogicalDevice->createLogicalDevice();

        m_logicalDeviceMap[pPhysicalDevice].push_back( std::move( upLogicalDevice ) );
    }

    void VulkanLogicalDeviceManager::destroyAllLogicalDevices()
    {
        m_logicalDeviceMap.clear();
    }
} // namespace vkrender
