#include "vkrenderer/VulkanDebugMessenger.hpp"
#include <iostream>

namespace vkrender
{
	VulkanDebugMessenger::VulkanDebugMessenger()
	{}
	
	void VulkanDebugMessenger::init( const utils::Sptr<vk::DebugUtilsMessengerCreateInfoEXT>& pDebugMessengerCreateInfo )
	{
		_debug_messenger_info = pDebugMessengerCreateInfo;
	}

	void VulkanDebugMessenger::create_debug_messenger(utils::Uptr<VulkanInstance>& vulkan_instance, const vk::AllocationCallbacks* pAllocator)
	{
		if (create_debug_utils_messenger_EXT(vulkan_instance, pAllocator) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to setup debug messenger!");
		}
	}

	void VulkanDebugMessenger::destroy_debug_messenger(utils::Uptr<VulkanInstance>& vulkan_instance, const vk::AllocationCallbacks* pAllocator)
	{
		destroy_debug_utils_messenger_EXT(vulkan_instance, pAllocator);
	}

	VkResult VulkanDebugMessenger::create_debug_utils_messenger_EXT(utils::Uptr<VulkanInstance>& vulkan_instance, const vk::AllocationCallbacks* pAllocator)
	{
		auto& instance = vulkan_instance->m_instance;
		auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(instance.getProcAddr("vkCreateDebugUtilsMessengerEXT"));
		if (func != nullptr) {
			return func( 
				instance,
				reinterpret_cast<VkDebugUtilsMessengerCreateInfoEXT*>( _debug_messenger_info.get() ), 
				reinterpret_cast<const VkAllocationCallbacks*>( pAllocator ),
				reinterpret_cast<VkDebugUtilsMessengerEXT*>( &_debug_messenger )
			);
		}
		else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void VulkanDebugMessenger::destroy_debug_utils_messenger_EXT(utils::Uptr<VulkanInstance>& vulkan_instance, const vk::AllocationCallbacks* pAllocator)
	{
		auto& instance = vulkan_instance->m_instance;
		auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>( instance.getProcAddr("vkDestroyDebugUtilsMessengerEXT") );
		if (func != nullptr)
		{
			func(
				instance, 
				_debug_messenger, 
				reinterpret_cast<const VkAllocationCallbacks*>(pAllocator)
			);
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