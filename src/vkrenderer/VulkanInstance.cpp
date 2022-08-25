#include "vkrenderer/VulkanInstance.hpp"
#include "vkrenderer/VulkanDebugMessenger.hpp"
#include "vkrenderer/VulkanLayer.hpp"
#include <cstdlib>
#include <vector>
#include <iostream>

namespace app
{
	VulkanInstance::VulkanInstance(const std::string& app_name)
	{
		if (enable_validation_layer && !check_validation_layer_support())
		{
			throw std::runtime_error("Validation layers requested, not available");
		}
		if (enable_validation_layer)
		{
			_debug_create_info = debug::populate_debug_messenger_info();
		}
		setup_application_info(app_name);
		init();
	}
	
	void VulkanInstance::createInstance()
	{
		// creating Vulkan Instance
		if (vk::createInstance(_info.get(), nullptr, &_instance) != vk::Result::eSuccess)
		{
			throw std::runtime_error("failed to create instance!");
		}
		std::cout << "Created Vulkan instance successfully" << std::endl;
	}

	void VulkanInstance::destroyInstance()
	{
		vkDestroyInstance(_instance, nullptr);
		_info.reset();
	}

	void VulkanInstance::setup_application_info(const std::string& app_name)
	{
		_app_info = std::make_unique<vk::ApplicationInfo>();
		_app_info->sType = vk::StructureType::eApplicationInfo;
		_app_info->pApplicationName = app_name.c_str();
		_app_info->applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		_app_info->pEngineName = "No Engine";
		_app_info->engineVersion = VK_MAKE_VERSION(1, 0, 0);
		_app_info->apiVersion = VK_API_VERSION_1_3;
		_app_info->pNext = nullptr;
	}

	void VulkanInstance::init()
	{
		_info = std::make_unique<vk::InstanceCreateInfo>();
		_info->sType = vk::StructureType::eInstanceCreateInfo;
		_info->pApplicationInfo = _app_info.get();
		
		_extensions = get_extensions();
		_info->enabledExtensionCount = static_cast<std::uint32_t>(_extensions.size());
		_info->ppEnabledExtensionNames = _extensions.data();
		
		if (enable_validation_layer)
		{
			_info->enabledLayerCount = static_cast<std::uint32_t>(layer::validation_layer.layers.size());
			_info->ppEnabledLayerNames = layer::validation_layer.layers.data();
			_info->pNext = _debug_create_info.get();
		}
		else {
			_info->enabledLayerCount = 0;
		}

		validate_glfw_extensions(_extensions);
	}

	void VulkanInstance::validate_glfw_extensions(VulkanInstance::ExtensionContainer& extensions)
	{
		// Checking for Vulkan Instance extensions
		std::uint32_t vulkan_extensions_count = 0;
		vk::enumerateInstanceExtensionProperties(nullptr, &vulkan_extensions_count, nullptr);
		auto vulkan_extension_properties = std::vector<vk::ExtensionProperties>{};
		vulkan_extension_properties.resize(vulkan_extensions_count);
		vulkan_extension_properties.shrink_to_fit();
		vk::enumerateInstanceExtensionProperties(nullptr, &vulkan_extensions_count, vulkan_extension_properties.data());

		for (auto extension_name : extensions)
		{
			bool found = false;
			for (const auto& vulkan_extension : vulkan_extension_properties)
			{
				if (std::strcmp(extension_name, vulkan_extension.extensionName) == 0) {
					std::cout << extension_name << " Extension found" << std::endl;
					found = true;
				}
			}
			if (found == false)
			{
				throw std::runtime_error("extension not found");
			}
		}
	}

	bool VulkanInstance::check_validation_layer_support()
	{
		// getting Instance layer properties count
		std::uint32_t layer_count;
		vk::enumerateInstanceLayerProperties(&layer_count, nullptr);

		std::vector<vk::LayerProperties> available_layers;
		available_layers.resize(layer_count);
		available_layers.shrink_to_fit();
		
		// Enumerating Insatnce layer properties
		vk::enumerateInstanceLayerProperties(&layer_count, available_layers.data());

		// validating Layer support
		for (const auto& layer_name : layer::validation_layer.layers)
		{
			bool layer_found = false;

			for (const auto& layer_properties : available_layers)
			{
				if (strcmp(layer_name, layer_properties.layerName) == 0)
				{
					layer_found = true;
				}
			}
			if (!layer_found) { return false;  }
		}

		return true;
	}

	auto VulkanInstance::get_extensions() -> VulkanInstance::ExtensionContainer
	{
		std::uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;

		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		ExtensionContainer extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
		
		if (enable_validation_layer)
		{
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}
		
		extensions.shrink_to_fit();
		
		return extensions;
	}
} // namespace appp