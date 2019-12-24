#include "VulkanInstance.h"
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
		auto application_info = std::make_unique<VkApplicationInfo>();
		application_info->sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		application_info->pApplicationName = app_name.c_str();
		application_info->applicationVersion = VK_MAKE_VERSION(1, 1, 0);
		application_info->pEngineName = "No Engine";
		application_info->engineVersion = VK_MAKE_VERSION(1, 1, 0);
		application_info->apiVersion = VK_API_VERSION_1_1;
		//application_info->pNext = nullptr;
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
		
		setup_debug_messenger();
	}

	void VulkanInstance::destroyInstance()
	{
		if (enable_validation_layer)
		{
			DestroyDebugUtilsMessengerEXT(nullptr);
			_debugmessenger_info.reset();
		}
		vkDestroyInstance(_instance, nullptr);
		_info.reset();
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL VulkanInstance::debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

		return VK_FALSE;
	}

	void VulkanInstance::init(utils::Uptr<VkApplicationInfo> app_info)
	{
		_info = std::make_unique<VkInstanceCreateInfo>();
		_info->sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		_info->pApplicationInfo = app_info.get();
		
		_extensions = get_extensions();
		_info->enabledExtensionCount = static_cast<std::uint32_t>(_extensions.size());
		_info->ppEnabledExtensionNames = _extensions.data();
		
		if (enable_validation_layer)
		{
			_info->enabledLayerCount = static_cast<std::uint32_t>(validation_layers.size());
			_info->ppEnabledLayerNames = validation_layers.data();

			_debugCreate_info = std::move(populate_debug_messenger_info());
			_info->pNext = (VkDebugUtilsMessengerCreateInfoEXT*)_debugCreate_info.get();
		}
		else {
			_info->enabledLayerCount = 0;
		}

		validate_glfw_extensions(_extensions);
	}

	void VulkanInstance::setup_debug_messenger()
	{
		if (!enable_validation_layer) { return; }

		_debugmessenger_info = std::move(populate_debug_messenger_info());
		_debugmessenger_info->pUserData = nullptr;
		if (CreateDebugUtilsMessengerEXT( nullptr ) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to setup debug messenger!");
		}
	}

	utils::Uptr<VkDebugUtilsMessengerCreateInfoEXT> VulkanInstance::populate_debug_messenger_info()
	{
		auto debugmessenger_info = std::make_unique<VkDebugUtilsMessengerCreateInfoEXT>();
		debugmessenger_info->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugmessenger_info->messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debugmessenger_info->messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		debugmessenger_info->pfnUserCallback = debugCallback;
	
		return std::move(debugmessenger_info);
	}

	VkResult VulkanInstance::CreateDebugUtilsMessengerEXT(const VkAllocationCallbacks* pAllocator )
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(_instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr) {
			return func(_instance, _debugmessenger_info.get(), pAllocator, &_debugmessenger);
		}
		else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void VulkanInstance::DestroyDebugUtilsMessengerEXT(const VkAllocationCallbacks* pAllocator) 
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(_instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) {
			func(_instance, _debugmessenger, pAllocator);
		}
	}

	void VulkanInstance::validate_glfw_extensions(VulkanInstance::ExtensionContainer& extensions)
	{
		// Checking for Vulkan Instance extensions
		std::uint32_t vulkan_extensions_count = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &vulkan_extensions_count, nullptr);
		auto vulkan_extension_properties = std::vector<VkExtensionProperties>{};
		vulkan_extension_properties.resize(vulkan_extensions_count);
		vulkan_extension_properties.shrink_to_fit();
		vkEnumerateInstanceExtensionProperties(nullptr, &vulkan_extensions_count, vulkan_extension_properties.data());

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
		vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

		std::vector<VkLayerProperties> available_layers;
		available_layers.resize(layer_count);
		available_layers.shrink_to_fit();
		
		// Enumerating Insatnce layer properties
		vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

		// validating Layer support
		for (const auto& layer_name : validation_layers)
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
}