#include "vkrenderer/VulkanQueueFamily.h"
#include <vector>

namespace vkrender
{
	QueueFamilyIndices VulkanQueueFamily::findQueueFamilyIndices( const VulkanPhysicalDevice& physicalDevice )
	{
		QueueFamilyIndices queueFamilyIndices;
		vk::PhysicalDevice* pDeviceHandle = physicalDevice.m_pDeviceHandle;

		std::vector<vk::QueueFamilyProperties> queueFamilyProperties = pDeviceHandle->getQueueFamilyProperties();
		
		int validQueueIndex = 0;

		for (const auto& prop : queueFamilyProperties)
		{
			if (prop.queueFlags & vk::QueueFlagBits::eGraphics )
			{
				queueFamilyIndices.m_graphicsFamily = validQueueIndex;
			}

			validQueueIndex++;
		}

		return queueFamilyIndices;
	}
} // namespace vkrender