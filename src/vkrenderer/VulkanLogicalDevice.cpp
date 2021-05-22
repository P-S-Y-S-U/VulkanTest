#include "vkrenderer/VulkanLogicalDevice.hpp"
#include "vkrenderer/VulkanLayer.hpp"
#include "vkrenderer/VulkanQueueFamily.hpp"
#include <iostream>

namespace app
{
	VulkanLogicalDevice::VulkanLogicalDevice(utils::Uptr<VulkanPhysicalDevice>& physical_device)
		:_physical_device{ physical_device }
	{}

	void VulkanLogicalDevice::create_logical_device()
	{
		populate_device_queue_info();
		populate_device_features();
		populate_create_info();

		if (vkCreateDevice(_physical_device->_device, _info.get(), nullptr, &_device))
		{
			throw std::runtime_error("failed to create logical device!");
		}
		create_queue();
	}
	
	void VulkanLogicalDevice::destroy_logical_device()
	{
		vkDestroyDevice(_device, nullptr);
	}

	void VulkanLogicalDevice::populate_device_features()
	{
		_device_features = std::make_shared<VkPhysicalDeviceFeatures>();
	}

	void VulkanLogicalDevice::populate_device_queue_info()
	{
		_queue_family_indices = VulkanQueueFamily::find_queue_family(_physical_device.get());
		auto& vulkan_physical_device = _physical_device->_device;

		auto queue_create_info = std::make_shared<VkDeviceQueueCreateInfo>();
		queue_create_info->sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_create_info->queueFamilyIndex = _queue_family_indices.graphics_family.value();
		queue_create_info->queueCount = 1;

		float queue_priority = 1.0f;
		queue_create_info->pQueuePriorities = &queue_priority;

		_device_queue_info = queue_create_info;
	}

	void VulkanLogicalDevice::populate_create_info()
	{
		_info = std::make_shared<VkDeviceCreateInfo>();
		_info->sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		_info->enabledExtensionCount = 0;
		_info->pQueueCreateInfos = _device_queue_info.get();
		_info->queueCreateInfoCount = 1;
		_info->pEnabledFeatures = _device_features.get();
		if (enable_validation_layer)
		{
			_info->ppEnabledLayerNames = layer::validation_layer.layers.data();
			_info->enabledLayerCount = static_cast<std::uint32_t>(layer::validation_layer.layers.size());
		}
		else {
			_info->enabledLayerCount = 0;
		}
	}

	void VulkanLogicalDevice::create_queue()
	{
		vkGetDeviceQueue(_device, _queue_family_indices.graphics_family.value(), 0, &_graphics_queue);
	}
} // namespace app