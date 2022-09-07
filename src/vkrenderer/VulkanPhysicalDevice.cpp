#include "vkrenderer/VulkanPhysicalDevice.h"
#include "vkrenderer/VulkanQueueFamily.h"
#include <vulkan/vulkan.h>
#include <iostream>

namespace vkrender
{
	VulkanPhysicalDevice::VulkanPhysicalDevice( 
		VulkanInstance* pVulkanInstance, 
		const vk::PhysicalDevice& pPhysicalDeviceHandle, 
		const std::vector<const char*>& enabledExtensions 
	)
		:m_pVulkanInstance{ pVulkanInstance }
		,m_deviceHandle{ pPhysicalDeviceHandle }
		,m_enabledExtensions{ enabledExtensions }
	{}
	
	SwapChainSupportDetails	VulkanPhysicalDevice::querySwapChainSupport( const VulkanSurface& surface ) const
	{
		SwapChainSupportDetails swapChainDetails;

		const vk::SurfaceKHR& surfaceHandle = *surface.m_upSurfaceHandle;

		swapChainDetails.capabilities =	m_deviceHandle.getSurfaceCapabilitiesKHR( surfaceHandle );
		swapChainDetails.surfaceFormats = m_deviceHandle.getSurfaceFormatsKHR( surfaceHandle );
		swapChainDetails.presentModes = m_deviceHandle.getSurfacePresentModesKHR( surfaceHandle );

		return swapChainDetails;
	}

} // namespace vkrender