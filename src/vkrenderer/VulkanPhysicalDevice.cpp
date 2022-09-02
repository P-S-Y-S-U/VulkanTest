#include "vkrenderer/VulkanPhysicalDevice.h"
#include "vkrenderer/VulkanQueueFamily.h"
#include <vulkan/vulkan.h>
#include <iostream>

namespace vkrender
{
	VulkanPhysicalDevice::VulkanPhysicalDevice( VulkanInstance* pVulkanInstance, const vk::PhysicalDevice& pPhysicalDeviceHandle )
		:m_pVulkanInstance{ pVulkanInstance }
		,m_deviceHandle{ pPhysicalDeviceHandle }
	{}

} // namespace app