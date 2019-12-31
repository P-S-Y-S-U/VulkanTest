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
		VulkanPhysicalDevice(const VulkanPhysicalDevice&) = delete;
		~VulkanPhysicalDevice() = default;
		
		VulkanPhysicalDevice& operator=(const VulkanPhysicalDevice&) = delete;
		VulkanPhysicalDevice& operator=(VulkanPhysicalDevice&&) = delete;

		void get_physical_devices();
	private:
		utils::Uptr<VulkanInstance>&						_vulkan_instance;
		VkPhysicalDevice									_device;
		utils::Sptr<VkPhysicalDeviceFeatures>				_device_features;
		utils::Sptr<VkPhysicalDeviceProperties>				_device_properties;
		
		VulkanPhysicalDevice(utils::Uptr<VulkanInstance>&, VkPhysicalDevice);
		auto populate_device_properties(VkPhysicalDevice device)->std::pair<utils::Sptr<VkPhysicalDeviceFeatures>, utils::Sptr<VkPhysicalDeviceProperties>>;


		bool is_device_suitable(VkPhysicalDevice);
		void probe_physical_device(VkPhysicalDevice);

		friend class VulkanQueueFamily;
	};

} // namespace app
#endif