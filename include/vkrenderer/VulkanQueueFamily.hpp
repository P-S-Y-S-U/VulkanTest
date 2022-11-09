#ifndef VKRENDER_VULKAN_QUEUE_FAMILY_HPP
#define VKRENDER_VULKAN_QUEUE_FAMILY_HPP

#include "utilities/memory.hpp"
#include "exports.hpp"

#include <optional>

namespace vkrender
{
	struct QueueFamilyIndices
	{
		std::optional<std::uint32_t>	m_graphicsFamily;
		std::optional<std::uint32_t>	m_presentFamily;
	};

	class VulkanQueueFamilyHelper 
	{
	public:
		static QueueFamilyIndices findQueueFamilyIndices( const vk::PhysicalDevice& physicalDevice, vk::SurfaceKHR* pVkSurface )
		{
			QueueFamilyIndices queueFamilyIndices;

			std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

			int validQueueIndex = 0;

			for (const auto& prop : queueFamilyProperties)
			{
				if (prop.queueFlags & vk::QueueFlagBits::eGraphics )
				{
					queueFamilyIndices.m_graphicsFamily = validQueueIndex;
				}

				if( pVkSurface )
				{
					vk::Bool32 bPresentationSupport = physicalDevice.getSurfaceSupportKHR( validQueueIndex, *pVkSurface );
					if( bPresentationSupport )
					{
						queueFamilyIndices.m_presentFamily = validQueueIndex;					
					}
				}
				validQueueIndex++;
			}

			return queueFamilyIndices;
		}
	};

} // namespace vkrender

#endif