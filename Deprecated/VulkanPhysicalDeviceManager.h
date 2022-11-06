#ifndef VKRENDER_VULKAN_PHYSICAL_DEVICE_MANAGER_H
#define VKRENDER_VULKAN_PHYSICAL_DEVICE_MANAGER_H

#include <vulkan/vulkan.hpp>
#include <optional>

#include "exports.hpp"
#include "vkrenderer/VulkanInstance.h"
#include "vkrenderer/VulkanPhysicalDevice.h"
#include "vkrenderer/VulkanSurface.h"

namespace vkrender
{
    class VULKAN_EXPORTS VulkanPhysicalDeviceManager
    {
    public:
        using DeviceCapabilitiesPair = std::pair<utils::Sptr<vk::PhysicalDeviceProperties>, utils::Sptr<vk::PhysicalDeviceFeatures>>;

        VulkanPhysicalDeviceManager( VulkanInstance* pInstance );
        VulkanPhysicalDeviceManager(const VulkanPhysicalDeviceManager&) = delete;
        VulkanPhysicalDeviceManager& operator=(const VulkanPhysicalDeviceManager&) = delete;
        ~VulkanPhysicalDeviceManager();

        VulkanPhysicalDevice* createSuitableDevice( const VulkanSurface& surface );
        void probePhysicalDevice( const VulkanPhysicalDevice& physicalDevice );
    private:
        VulkanInstance*     m_pInstance;
        std::vector<utils::Uptr<VulkanPhysicalDevice>> m_ActiveDevices;

        std::vector<vk::PhysicalDevice, std::allocator<vk::PhysicalDevice>> getAvailableDevices();

        VulkanPhysicalDevice createTemporaryDevice( vk::PhysicalDevice& deviceHandle, const std::vector<const char*>& deviceExtensions );
        bool isDeviceSuitable( const VulkanPhysicalDevice& physicalDevice, const VulkanSurface& surface, const std::vector<const char*>& requiredExtensions );
        bool checkDeviceExtensionSupport( const VulkanPhysicalDevice& physicalDevice, const std::vector<const char*>& requiredExtensions  );
        bool checkSwapChainAdequacy( const VulkanPhysicalDevice& physicalDevice, const VulkanSurface& surface, const bool& bExtensionSupported );

        void probePhysicalDeviceHandle(const vk::PhysicalDevice& deviceHandle );
        DeviceCapabilitiesPair populateDeviceProperties( vk::PhysicalDevice& deviceHandle );
    }; 
} // namespace vkrender

#endif 