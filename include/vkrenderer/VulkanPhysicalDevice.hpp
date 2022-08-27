#ifndef APP_VULKAN_PHYSICAL_DEVICE_HPP
#define APP_VULKAN_PHYSICAL_DEVICE_HPP

#include <vulkan/vulkan.hpp>
#include "VulkanInstance.h"
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
		vk::PhysicalDevice									_device;
		utils::Sptr<vk::PhysicalDeviceFeatures>				_device_features;
		utils::Sptr<vk::PhysicalDeviceProperties>				_device_properties;
		
		auto get_temp_device(const vk::PhysicalDevice&)->utils::Uptr<app::VulkanPhysicalDevice>;
		auto populate_device_properties(vk::PhysicalDevice device)->std::pair<utils::Sptr<vk::PhysicalDeviceFeatures>, utils::Sptr<vk::PhysicalDeviceProperties>>;

		bool is_device_suitable(vk::PhysicalDevice);
		void probe_physical_device(vk::PhysicalDevice);

		friend class VulkanLogicalDevice;
		friend class VulkanQueueFamily;
	};

} // namespace app
#endif