#include "vkrenderer/VulkanDebugMessenger.h"
#include <iostream>

namespace vkrender
{
	VulkanDebugMessenger::VulkanDebugMessenger()
	{}
	
	void VulkanDebugMessenger::init( const utils::Sptr<vk::DebugUtilsMessengerCreateInfoEXT>& pDebugMessengerCreateInfo )
	{
		m_spDebugMessengerCreateInfo = pDebugMessengerCreateInfo;
	}

	void VulkanDebugMessenger::createDebugMessenger( VulkanInstance* pVulkanInstance, const vk::AllocationCallbacks* pAllocatorCB )
	{
		if (createDebugUtilsMessengerEXT(pVulkanInstance, pAllocatorCB) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to setup debug messenger!");
		}
	}

	void VulkanDebugMessenger::destroyDebugMessenger( VulkanInstance* pVulkanInstance, const vk::AllocationCallbacks* pAllocatorCB )
	{
		destroyDebugUtilsMessengerEXT(pVulkanInstance, pAllocatorCB);
	}

	VkResult VulkanDebugMessenger::createDebugUtilsMessengerEXT( VulkanInstance* pVulkanInstance, const vk::AllocationCallbacks* pAllocatorCB )
	{
		vk::Instance& vkInstance = pVulkanInstance->m_instance;
		PFN_vkCreateDebugUtilsMessengerEXT func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>( vkInstance.getProcAddr("vkCreateDebugUtilsMessengerEXT"));
		if (func != nullptr) {
			return func( 
				vkInstance,
				reinterpret_cast<VkDebugUtilsMessengerCreateInfoEXT*>( m_spDebugMessengerCreateInfo.get() ), 
				reinterpret_cast<const VkAllocationCallbacks*>( pAllocatorCB ),
				reinterpret_cast<VkDebugUtilsMessengerEXT*>( &m_debugMessenger )
			);
		}
		else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void VulkanDebugMessenger::destroyDebugUtilsMessengerEXT( VulkanInstance* pVulkanInstance, const vk::AllocationCallbacks* pAllocatorCB )
	{
		vk::Instance& vkInstance = pVulkanInstance->m_instance;
		PFN_vkDestroyDebugUtilsMessengerEXT func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>( vkInstance.getProcAddr("vkDestroyDebugUtilsMessengerEXT") );
		if (func != nullptr)
		{
			func(
				vkInstance, 
				m_debugMessenger, 
				reinterpret_cast<const VkAllocationCallbacks*>(pAllocatorCB)
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