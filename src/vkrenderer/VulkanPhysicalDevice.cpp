#include "vkrenderer/VulkanPhysicalDevice.h"
#include "vkrenderer/VulkanQueueFamily.h"
#include <vulkan/vulkan.h>
#include <iostream>

namespace vkrender
{
	VulkanPhysicalDevice::VulkanPhysicalDevice( VulkanInstance* pVulkanInstance, vk::PhysicalDevice* pPhysicalDeviceHandle )
		:m_pVulkanInstance{ pVulkanInstance }
		,m_pDeviceHandle{ pPhysicalDeviceHandle }
	{}

} // namespace app