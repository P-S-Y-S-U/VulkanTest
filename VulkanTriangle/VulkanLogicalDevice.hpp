#ifndef APP_VULKAN_LOGICAL_DEVICE_HPP
#define APP_VULKAN_LOGICAL_DEVICE_HPP

#include "vulkan/vulkan.h"
#include "VulkanPhysicalDevice.hpp"
#include "VulkanQueueFamily.hpp"

namespace app
{
	class VulkanLogicalDevice
	{
	public:
		explicit VulkanLogicalDevice(utils::Uptr<VulkanPhysicalDevice>&);
		~VulkanLogicalDevice() = default;

		void create_logical_device();
		void destroy_logical_device();
#if NDEBUG
		const bool enable_validation_layer = false;
#else
		const bool enable_validation_layer = true;
#endif // NDEBUG
	private:
		VkDevice								_device;
		utils::Uptr<VulkanPhysicalDevice>&		_physical_device;
		utils::Sptr<VkPhysicalDeviceFeatures>	_device_features;
		utils::Sptr<VkDeviceQueueCreateInfo>	_device_queue_info;
		utils::Sptr<VkDeviceCreateInfo>			_info;
		QueueFamilyIndex						_queue_family_indices;
		VkQueue									_graphics_queue;
		
		void populate_device_queue_info();
		void populate_create_info();
		void populate_device_features();
		
		void create_queue();
	};
} // namespace app

#endif // !APP_VULKAN_LOGICAL_DEVICE
