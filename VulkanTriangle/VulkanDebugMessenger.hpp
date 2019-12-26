#ifndef APP_VULKAN_DEBUGGER_EXT
#define APP_VULKAN_DEBUGGER_EXT

#include <vulkan/vulkan.h>
#include "utilities.hpp"

namespace app::debug
{
	
	class VulkanDebugMessenger
	{
	public:
		using DebugMsgInfoPtr = utils::Sptr<VkDebugUtilsMessengerCreateInfoEXT>;
		VulkanDebugMessenger();
		VulkanDebugMessenger(const VulkanDebugMessenger&) = delete;
		VulkanDebugMessenger(VulkanDebugMessenger&&) = delete;
		~VulkanDebugMessenger() = default;

		VulkanDebugMessenger& operator=(const VulkanDebugMessenger&) = delete;
		VulkanDebugMessenger& operator=(VulkanDebugMessenger&&) = delete;

		void create_debug_messenger(VkInstance*, const VkAllocationCallbacks*);
		void destroy_debug_messenger(VkInstance*, const VkAllocationCallbacks*);

		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData);
		DebugMsgInfoPtr populate_debug_messenger_info();
	private:
		VkDebugUtilsMessengerEXT	_debug_messenger;
		DebugMsgInfoPtr				_debug_messenger_info;

		VkResult create_debug_utils_messenger_EXT(VkInstance*, const VkAllocationCallbacks*);
		void destroy_debug_utils_messenger_EXT(VkInstance*, const VkAllocationCallbacks*);
	};

} // namespace app::debug

#endif

