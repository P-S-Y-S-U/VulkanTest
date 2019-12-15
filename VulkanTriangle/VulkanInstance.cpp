#include "VulkanInstance.h"
#include <cstdlib>
#include <vector>
#include <iostream>

namespace app
{
	VulkanInstance::VulkanInstance(const std::string& app_name)
	{
		auto application_info = std::make_unique<VkApplicationInfo>();
		application_info->sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		application_info->pApplicationName = app_name.c_str();
		application_info->applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		application_info->pEngineName = "No Engine";
		application_info->engineVersion = VK_MAKE_VERSION(1, 0, 0);
		application_info->apiVersion = VK_API_VERSION_1_0;
		application_info->pNext = nullptr;
		init(std::move(application_info));
	}

	void VulkanInstance::createInstance()
	{
		// creating Vulkan Instance
		if (vkCreateInstance(_info.get(), nullptr, &_instance) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create instance!");
		}
		std::cout << "Created Vulkan instance successfully" << std::endl;
	}

	void VulkanInstance::destroyInstance()
	{
		vkDestroyInstance(_instance, nullptr);
	}

	void VulkanInstance::init(utils::Uptr<VkApplicationInfo> app_info)
	{
		_info = std::make_unique<VkInstanceCreateInfo>();
		_info->sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		_info->pApplicationInfo = app_info.get();

		std::uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;

		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		_info->enabledExtensionCount = glfwExtensionCount;
		_info->ppEnabledExtensionNames = glfwExtensions;
		_info->enabledLayerCount = 0;

		validate_glfw_extensions(glfwExtensions, glfwExtensionCount);
	}

	void VulkanInstance::validate_glfw_extensions(const char** glfw_extensions, const std::uint32_t& extension_count)
	{
		// Checking for Vulkan Instance extensions
		std::uint32_t vulkan_extensions_count = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &vulkan_extensions_count, nullptr);
		auto vulkan_extension_properties = std::vector<VkExtensionProperties>{};
		vulkan_extension_properties.resize(vulkan_extensions_count);
		vkEnumerateInstanceExtensionProperties(nullptr, &vulkan_extensions_count, vulkan_extension_properties.data());

		for (std::uint32_t i = 0; i < extension_count; i++)
		{
			bool found = false;
			for (const auto& vulkan_extension : vulkan_extension_properties)
			{
				if (std::strcmp(glfw_extensions[i], vulkan_extension.extensionName) == 0) {
					std::cout << glfw_extensions[i] << " Extension found" << std::endl;
					found = true;
				}
			}
			if (found == false)
			{
				throw std::runtime_error("extension not found");
			}
		}
	}
}