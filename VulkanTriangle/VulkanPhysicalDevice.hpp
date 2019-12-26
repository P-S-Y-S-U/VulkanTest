#ifndef APP_VULKAN_PHYSICAL_DEVICE_HPP
#define APP_VULKAN_PHYSICAL_DEVICE_HPP

#include <vulkan/vulkan.h>
#include "VulkanInstance.hpp"

namespace app
{
	class VulkanPhysicalDevice
	{
	public:
		VulkanPhysicalDevice(utils::Uptr<VulkanInstance>&);
		~VulkanPhysicalDevice() = default;

		void get_physical_devices();
	private:
		utils::Uptr<VulkanInstance>&						_vulkan_instance;
		VkPhysicalDevice									_device;
		utils::Sptr<VkPhysicalDeviceFeatures>				_device_features;
		utils::Sptr<VkPhysicalDeviceProperties>				_device_properties;
	
		bool is_device_suitable(VkPhysicalDevice);
		void probe_physical_device(VkPhysicalDevice);
	};

} // namespace app
#endif