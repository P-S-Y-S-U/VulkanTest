#include "vkrenderer/VulkanQueueFamily.hpp"
#include <vector>

namespace app
{
	
	QueueFamilyIndex VulkanQueueFamily::find_queue_family(app::VulkanPhysicalDevice* vulkan_device)
	{
		auto queue_family_index = QueueFamilyIndex{};
		auto& vulkan_physical_device = vulkan_device->_device;

		std::vector<vk::QueueFamilyProperties> queue_families = vulkan_device->_device.getQueueFamilyProperties();
		
		int valid_queues_index = 0;

		for (const auto& queue_family : queue_families)
		{
			if (queue_family.queueFlags & vk::QueueFlagBits::eGraphics ) // TODO check this works in debug mode
			{
				queue_family_index.graphics_family = valid_queues_index;
			}

			if (queue_family_index.is_valid()) { break; }
			valid_queues_index++;
		}

		return queue_family_index;
	}
} // namespace app