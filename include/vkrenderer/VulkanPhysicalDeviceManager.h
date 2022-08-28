#ifndef VKRENDER_VULKAN_PHYSICAL_DEVICE_MANAGER_H
#define VKRENDER_VULKAN_PHYSICAL_DEVICE_MANAGER_H

#include <vulkan/vulkan.hpp>
#include <optional>

#include "exports.hpp"
#include "vkrenderer/VulkanInstance.h"
#include "vkrenderer/VulkanPhysicalDevice.h"

namespace vkrender
{
    class VULKAN_EXPORTS VulkanPhysicalDeviceManager
    {
    public:
        using DeviceCapabilitiesPair = std::pair<utils::Sptr<vk::PhysicalDeviceProperties>, utils::Sptr<vk::PhysicalDeviceFeatures>>;

        VulkanPhysicalDeviceManager( VulkanInstance* pInstance );
        ~VulkanPhysicalDeviceManager();

        VulkanPhysicalDevice* CreateSuitableDevice();
        void probePhysicalDevice( const VulkanPhysicalDevice& physicalDevice );
    private:
        VulkanInstance*     m_pInstance;
        std::vector<utils::Uptr<VulkanPhysicalDevice>> m_ActiveDevices;

        std::vector<vk::PhysicalDevice, std::allocator<vk::PhysicalDevice>> getAvailableDevices();

        VulkanPhysicalDevice&& createTemporaryDevice( vk::PhysicalDevice* pDeviceHandle );
        bool isDeviceSuitable( const VulkanPhysicalDevice& physicalDevice );

        void probePhysicalDeviceHandle(const vk::PhysicalDevice& deviceHandle );
        DeviceCapabilitiesPair populateDeviceProperties( vk::PhysicalDevice* pDeviceHandle );
    }; 
} // namespace vkrender

#endif 