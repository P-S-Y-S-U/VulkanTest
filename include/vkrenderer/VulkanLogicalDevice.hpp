#ifndef APP_VULKAN_LOGICAL_DEVICE_HPP
#define APP_VULKAN_LOGICAL_DEVICE_HPP

#include <vulkan/vulkan.hpp>
#include "VulkanPhysicalDevice.hpp"
#include "VulkanQueueFamily.hpp"
#include "exports.hpp"

namespace app
{
	class VULKAN_EXPORTS VulkanLogicalDevice
	{
	public:
		explicit VulkanLogicalDevice(utils::Uptr<VulkanPhysicalDevice>&);
		~VulkanLogicalDevice() = default;

		void create_logical_device();
		void destroy_logical_device();
#if NDEBUG
		const bool ENABLE_VALIDATION_LAYER = false;
#else
		const bool ENABLE_VALIDATION_LAYER = true;
#endif // NDEBUG
	private:
		vk::Device								_device;
		utils::Uptr<VulkanPhysicalDevice>&		_physical_device;
		utils::Sptr<vk::PhysicalDeviceFeatures>	_device_features;
		utils::Sptr<vk::DeviceQueueCreateInfo>	_device_queue_info;
		utils::Sptr<vk::DeviceCreateInfo>			_info;
		QueueFamilyIndices						_queue_family_indices;
		vk::Queue									_graphics_queue;
		
		void populate_device_queue_info();
		void populate_create_info();
		void populate_device_features();
		
		void create_queue();
	};
} // namespace app

#endif // !APP_VULKAN_LOGICAL_DEVICE
