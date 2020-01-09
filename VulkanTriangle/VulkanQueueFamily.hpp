#ifndef APP_VULKAN_QUEUE_FAMILY_HPP
#define APP_VULKAN_QUEUE_FAMILY_HPP

#include <optional>
#include "VulkanPhysicalDevice.hpp"
#include "utilities.hpp"

namespace app
{
	struct QueueFamilyIndex
	{
		std::optional<std::uint32_t>	graphics_family;
		
		constexpr bool is_valid() { return graphics_family.has_value(); }
	};

	class VulkanQueueFamily
	{
	public:
		VulkanQueueFamily() = default;
		~VulkanQueueFamily() = default;

		static QueueFamilyIndex find_queue_family(VulkanPhysicalDevice*);
	};
} // namespace app

#endif