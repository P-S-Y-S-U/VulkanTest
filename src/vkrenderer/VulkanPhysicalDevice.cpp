#include "vkrenderer/VulkanPhysicalDevice.h"
#include "vkrenderer/VulkanQueueFamily.h"
#include <vulkan/vulkan.h>
#include <iostream>

namespace vkrender
{
	VulkanPhysicalDevice::VulkanPhysicalDevice( 
		VulkanInstance* pVulkanInstance, 
		const vk::PhysicalDevice& pPhysicalDeviceHandle, 
		const std::vector<const char*>& enabledExtensions 
	)
		:m_pVulkanInstance{ pVulkanInstance }
		,m_deviceHandle{ pPhysicalDeviceHandle }
		,m_enabledExtensions{ enabledExtensions }
	{}

} // namespace app