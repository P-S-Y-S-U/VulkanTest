#ifndef VKRENDER_VULKAN_DEBUGGER_EXT_H
#define VKRENDER_VULKAN_DEBUGGER_EXT_H

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
		~VulkanDebugMessenger();

		VulkanDebugMessenger& operator=(const VulkanDebugMessenger&) = delete;
		VulkanDebugMessenger& operator=(VulkanDebugMessenger&&) = delete;

		void init(const utils::Sptr<vk::DebugUtilsMessengerCreateInfoEXT>& pDebugMessengerCreateInfo);
		void createDebugMessenger( VulkanInstance* pVulkanInstance, const vk::AllocationCallbacks* pAllocatorCB );
		void destroyDebugMessenger( VulkanInstance* pVulkanInstance, const vk::AllocationCallbacks* pAllocatorCB );

		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData
		);
	private:
		vk::DebugUtilsMessengerEXT	m_debugMessenger;
		utils::Sptr<vk::DebugUtilsMessengerCreateInfoEXT>	m_spDebugMessengerCreateInfo;

		VkResult createDebugUtilsMessengerEXT( VulkanInstance* pVulkanInstance, const vk::AllocationCallbacks* pAllocatorCB );
		void destroyDebugUtilsMessengerEXT( VulkanInstance* pVulkanInstance, const vk::AllocationCallbacks* pAllocatorCB );
	};

} // namespace vkrender

#endif

