#include "VulkanDebugMessenger.hpp"
#include <iostream>

namespace app::debug
{
	VulkanDebugMessenger::VulkanDebugMessenger()
	{
		_debug_messenger_info = populate_debug_messenger_info();
	}

	void VulkanDebugMessenger::create_debug_messenger(VkInstance* vulkan_instance, const VkAllocationCallbacks* pAllocator)
	{
		if (create_debug_utils_messenger_EXT(vulkan_instance, pAllocator) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to setup debug messenger!");
		}
	}

	void VulkanDebugMessenger::destroy_debug_messenger(VkInstance* vulkan_instance, const VkAllocationCallbacks* pAllocator)
	{
		destroy_debug_utils_messenger_EXT(vulkan_instance, pAllocator);
	}

	DebugMsgInfoPtr populate_debug_messenger_info()
	{
		auto debugmessenger_info = std::make_shared<VkDebugUtilsMessengerCreateInfoEXT>();
		debugmessenger_info->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugmessenger_info->messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debugmessenger_info->messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		debugmessenger_info->pfnUserCallback = VulkanDebugMessenger::debugCallback;

		return debugmessenger_info;
	}

	VkResult VulkanDebugMessenger::create_debug_utils_messenger_EXT(VkInstance* vulkan_instance, const VkAllocationCallbacks* pAllocator)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(*vulkan_instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr) {
			return func(*vulkan_instance, _debug_messenger_info.get(), pAllocator, &_debug_messenger);
		}
		else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void VulkanDebugMessenger::destroy_debug_utils_messenger_EXT(VkInstance* vulkan_instance, const VkAllocationCallbacks* pAllocator)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(*vulkan_instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			func(*vulkan_instance, _debug_messenger, pAllocator);
		}
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugMessenger::debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

		return VK_FALSE;
	}

} // namespace app