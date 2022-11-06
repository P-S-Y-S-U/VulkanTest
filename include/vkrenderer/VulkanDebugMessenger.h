#ifndef VKRENDER_VULKAN_DEBUGGER_EXT_H
#define VKRENDER_VULKAN_DEBUGGER_EXT_H

#include <vulkan/vulkan.hpp>
#include "utilities/memory.hpp"
#include "exports.hpp"

namespace vkrender
{
	class VULKAN_EXPORTS VulkanDebugMessenger
	{
	public:
		VulkanDebugMessenger();
		VulkanDebugMessenger(const VulkanDebugMessenger&) = delete;
		VulkanDebugMessenger(VulkanDebugMessenger&&) = delete;
		~VulkanDebugMessenger();

		VulkanDebugMessenger& operator=(const VulkanDebugMessenger&) = delete;
		VulkanDebugMessenger& operator=(VulkanDebugMessenger&&) = delete;

		static void initLogger();
		static VkResult createDebugUtilsMessengerEXT( 
			vk::Instance& vkInstance, 
			vk::DebugUtilsMessengerCreateInfoEXT& vkDebugMessengerCreateInfo, 
			const vk::AllocationCallbacks* pVkAllocatorCB, 
			vk::DebugUtilsMessengerEXT& vkDebugMessenger 
		);
		static void destroyDebugUtilsMessengerEXT( 
			vk::Instance& vkInstance,
			vk::DebugUtilsMessengerEXT& vkDebugMessenger,
			const vk::AllocationCallbacks* pAllocatorCB 
		); 

		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData
		);
	};

} // namespace vkrender

#endif

