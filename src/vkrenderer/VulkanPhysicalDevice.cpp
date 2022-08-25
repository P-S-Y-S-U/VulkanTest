#include "vkrenderer/VulkanPhysicalDevice.hpp"
#include "vkrenderer/VulkanQueueFamily.hpp"
#include <vulkan/vulkan.h>
#include <iostream>

namespace app
{
	VulkanPhysicalDevice::VulkanPhysicalDevice(utils::Uptr<VulkanInstance>& vulkan_instance)
		:_vulkan_instance{vulkan_instance}
	{}

	bool VulkanPhysicalDevice::is_device_suitable(vk::PhysicalDevice device)
	{
		auto [device_features, device_properties] = populate_device_properties(device);
		auto vk_device = get_temp_device(device);
		auto queue_family_indices = app::VulkanQueueFamily::find_queue_family(vk_device.get());

		auto is_suitable = [&, device_properties = device_properties, device_features = device_features]() -> bool {
			bool shader = device_properties->deviceType == vk::PhysicalDeviceType::eDiscreteGpu && device_features->geometryShader;
			queue_family_indices = app::VulkanQueueFamily::find_queue_family(vk_device.get());
			bool queue_family = queue_family_indices.graphics_family.has_value();

			return shader && queue_family;
		}();
		if ( is_suitable )
		{
			_device_features = device_features;
			_device_properties = device_properties;
		}
		return  is_suitable;
	}

	void VulkanPhysicalDevice::get_physical_devices()
	{
		auto& instance = _vulkan_instance->_instance;

		std::vector<vk::PhysicalDevice, std::allocator<vk::PhysicalDevice> > devices = instance.enumeratePhysicalDevices();

		if (devices.empty())
		{
			throw std::runtime_error("no suitable vulkan device found!");
		}

		for (const auto& device : devices)
		{
			probe_physical_device(device);
			if ( is_device_suitable(device) )
			{
				_device = device;
				return;
			}
		}

		throw std::runtime_error("failed to select compatible vulkan GPU!");
	}

	auto VulkanPhysicalDevice::get_temp_device(const vk::PhysicalDevice& device) -> utils::Uptr<VulkanPhysicalDevice>
	{
		auto temp_device = std::make_unique<VulkanPhysicalDevice>(_vulkan_instance);
		temp_device->_device = device;
		auto [device_features, device_properties] = populate_device_properties(temp_device->_device);
		temp_device->_device_features = device_features;
		temp_device->_device_properties = device_properties;

		return std::move(temp_device);
	}

	void VulkanPhysicalDevice::probe_physical_device(vk::PhysicalDevice device)
	{
		auto device_properties = std::make_shared<vk::PhysicalDeviceProperties>();
		device.getProperties( device_properties.get() );
		std::cout << "Device ID : " << device_properties->deviceID << " Device Name : " << device_properties->deviceName << " Vendor : " << device_properties->vendorID << std::endl;
	}

	auto VulkanPhysicalDevice::populate_device_properties(vk::PhysicalDevice device)->std::pair<utils::Sptr<vk::PhysicalDeviceFeatures>, utils::Sptr<vk::PhysicalDeviceProperties>>
	{
		auto device_properties = std::make_shared<vk::PhysicalDeviceProperties>();
		auto device_features = std::make_shared<vk::PhysicalDeviceFeatures>();
		device.getProperties( device_properties.get() );
		device.getFeatures( device_features.get() );

		return std::make_pair(device_features, device_properties);
	}
} // namespace app