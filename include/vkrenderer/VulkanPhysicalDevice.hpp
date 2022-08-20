#ifndef APP_VULKAN_PHYSICAL_DEVICE_HPP
#define APP_VULKAN_PHYSICAL_DEVICE_HPP

#include <vulkan/vulkan.h>
#include "VulkanInstance.hpp"
#include "exports.hpp"

namespace app
{
	class VULKAN_EXPORTS VulkanPhysicalDevice
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
		
		auto get_temp_device(const VkPhysicalDevice&)->utils::Uptr<app::VulkanPhysicalDevice>;
		auto populate_device_properties(VkPhysicalDevice device)->std::pair<utils::Sptr<VkPhysicalDeviceFeatures>, utils::Sptr<VkPhysicalDeviceProperties>>;

		bool is_device_suitable(VkPhysicalDevice);
		void probe_physical_device(VkPhysicalDevice);

		friend class VulkanLogicalDevice;
		friend class VulkanQueueFamily;
	};

} // namespace app
#endif