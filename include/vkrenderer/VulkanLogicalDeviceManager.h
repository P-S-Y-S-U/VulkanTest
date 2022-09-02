#ifndef VKRENDER_VULKAN_LOGICAL_DEVICE_MANAGER_H
#define VKRENDER_VULKAN_LOGICAL_DEVICE_MANAGER_H

#include "vkrenderer/VulkanPhysicalDevice.h"
#include "vkrenderer/VulkanLogicalDevice.h"
#include "exports.hpp"

#include <map>

namespace vkrender
{
    class VULKAN_EXPORTS VulkanLogicalDeviceManager
    {
    public:
        using LogicalDevicePtr = utils::Uptr<VulkanLogicalDevice>;
        using LogicalDeviceContainer = std::vector<LogicalDevicePtr>;

        VulkanLogicalDeviceManager();
        VulkanLogicalDeviceManager(const VulkanLogicalDeviceManager&) = delete;
        VulkanLogicalDeviceManager(VulkanLogicalDeviceManager&&) noexcept = default;
        VulkanLogicalDeviceManager& operator=(const VulkanLogicalDeviceManager&) = delete;
        VulkanLogicalDeviceManager& operator=(VulkanLogicalDeviceManager&&) noexcept = default;
        ~VulkanLogicalDeviceManager();
        
        void createLogicalDevice( VulkanPhysicalDevice* pPhysicalDevice, VulkanSurface* pSurface = nullptr );

        void destroyAllLogicalDevices();
    private:
        std::map<VulkanPhysicalDevice*, LogicalDeviceContainer> m_logicalDeviceMap;
    };
} // namespace vkrender

#endif 