#include "vkrenderer/VulkanQueueFamily.hpp"
#include <vector>

namespace app
{
	
	QueueFamilyIndex VulkanQueueFamily::find_queue_family(app::VulkanPhysicalDevice* vulkan_device)
	{
		auto queue_family_index = QueueFamilyIndex{};
		auto& vulkan_physical_device = vulkan_device->_device;
		std::uint32_t queue_family_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(vulkan_physical_device, &queue_family_count, nullptr);
		
		auto queue_families = std::vector<VkQueueFamilyProperties>{};
		queue_families.resize(queue_family_count);
		queue_families.shrink_to_fit();
		vkGetPhysicalDeviceQueueFamilyProperties(vulkan_physical_device, &queue_family_count, queue_families.data());

		int valid_queues_index = 0;

		for (const auto& queue_family : queue_families)
		{
			if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				queue_family_index.graphics_family = valid_queues_index;
			}

			if (queue_family_index.is_valid()) { break; }
			valid_queues_index++;
		}

		return queue_family_index;
	}
} // namespace app