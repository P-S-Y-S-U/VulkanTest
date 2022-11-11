#include "vkrenderer/VulkanLogicalDeviceManager.h"

namespace vkrender
{
    VulkanLogicalDeviceManager::VulkanLogicalDeviceManager()
    {}

    VulkanLogicalDeviceManager::~VulkanLogicalDeviceManager()
    {
        destroyAllLogicalDevices();
    }

    VulkanLogicalDevice* VulkanLogicalDeviceManager::createLogicalDevice( VulkanPhysicalDevice* pPhysicalDevice, VulkanSurface* pSurface )
    {
        LogicalDevicePtr upLogicalDevice = std::make_unique<VulkanLogicalDevice>( pPhysicalDevice, pSurface );

        upLogicalDevice->createLogicalDevice();

        VulkanLogicalDevice* pLogicalDevice = upLogicalDevice.get();
        m_logicalDeviceMap[pPhysicalDevice].push_back( std::move( upLogicalDevice ) );

        return pLogicalDevice;
    }

    void VulkanLogicalDeviceManager::destroyAllLogicalDevices()
    {
        m_logicalDeviceMap.clear();
    }
} // namespace vkrender
