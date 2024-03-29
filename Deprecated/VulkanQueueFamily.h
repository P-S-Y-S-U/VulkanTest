#ifndef VKRENDER_VULKAN_QUEUE_FAMILY_H
#define VKRENDER_VULKAN_QUEUE_FAMILY_H

#include "vkrenderer/VulkanPhysicalDevice.h"
#include "vkrenderer/VulkanSurface.h"
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

	class VULKAN_EXPORTS VulkanQueueFamily
	{
	public:
		VulkanQueueFamily() = default;
		~VulkanQueueFamily() = default;

		static QueueFamilyIndices findQueueFamilyIndices( const VulkanPhysicalDevice& physicalDevice, const VulkanSurface& surface );
	};
} // namespace vkrender

#endif