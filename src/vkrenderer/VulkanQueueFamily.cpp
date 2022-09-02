#include "vkrenderer/VulkanQueueFamily.h"
#include <vector>

namespace vkrender
{
	QueueFamilyIndices VulkanQueueFamily::findQueueFamilyIndices( const VulkanPhysicalDevice& physicalDevice, VulkanSurface* pSurface )
	{
		QueueFamilyIndices queueFamilyIndices;
		const vk::PhysicalDevice& deviceHandle = physicalDevice.m_deviceHandle;

		std::vector<vk::QueueFamilyProperties> queueFamilyProperties = deviceHandle.getQueueFamilyProperties();
		
		int validQueueIndex = 0;

		for (const auto& prop : queueFamilyProperties)
		{
			if (prop.queueFlags & vk::QueueFlagBits::eGraphics )
			{
				queueFamilyIndices.m_graphicsFamily = validQueueIndex;
			}

			if( pSurface )
			{
				vk::Bool32 bPresentationSupport = physicalDevice.m_deviceHandle.getSurfaceSupportKHR( validQueueIndex, *pSurface->m_upSurfaceHandle.get() );
				if( bPresentationSupport )
				{
					queueFamilyIndices.m_presentFamily = validQueueIndex;					
				}
			}
			validQueueIndex++;
		}

		return queueFamilyIndices;
	}
} // namespace vkrender