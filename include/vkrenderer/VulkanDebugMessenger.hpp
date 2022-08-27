#ifndef VKRENDER_VULKAN_DEBUGGER_EXT_HPP
#define VKRENDER_VULKAN_DEBUGGER_EXT_HPP

#include <vulkan/vulkan.hpp>
#include "utilities/memory.hpp"
#include "vkrenderer/VulkanInstance.h"
#include "exports.hpp"

namespace vkrender
{
	class VULKAN_EXPORTS VulkanDebugMessenger
	{
	public:
		VulkanDebugMessenger();
		VulkanDebugMessenger(const VulkanDebugMessenger&) = delete;
		VulkanDebugMessenger(VulkanDebugMessenger&&) = delete;
		~VulkanDebugMessenger() = default;

		VulkanDebugMessenger& operator=(const VulkanDebugMessenger&) = delete;
		VulkanDebugMessenger& operator=(VulkanDebugMessenger&&) = delete;

		void init(const utils::Sptr<vk::DebugUtilsMessengerCreateInfoEXT>& pDebugMessengerCreateInfo);
		void create_debug_messenger(utils::Uptr<VulkanInstance>&, const vk::AllocationCallbacks*);
		void destroy_debug_messenger(utils::Uptr<VulkanInstance>&, const vk::AllocationCallbacks*);

		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData);
	private:
		vk::DebugUtilsMessengerEXT	_debug_messenger;
		utils::Sptr<vk::DebugUtilsMessengerCreateInfoEXT>				_debug_messenger_info;

		VkResult create_debug_utils_messenger_EXT(utils::Uptr<VulkanInstance>&, const vk::AllocationCallbacks*);
		void destroy_debug_utils_messenger_EXT(utils::Uptr<VulkanInstance>&, const vk::AllocationCallbacks*);
	};

} // namespace app::debug

#endif

