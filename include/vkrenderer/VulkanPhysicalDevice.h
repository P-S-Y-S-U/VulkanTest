#ifndef VKRENDER_VULKAN_PHYSICAL_DEVICE_H
#define VKRENDER_VULKAN_PHYSICAL_DEVICE_H

#include <vulkan/vulkan.hpp>
#include "VulkanInstance.h"
#include "exports.hpp"

namespace vkrender
{
	struct SwapChainSupportDetails
	{
		vk::SurfaceCapabilitiesKHR			capabilities;
		std::vector<vk::SurfaceFormatKHR>	surfaceFormats;
		std::vector<vk::PresentModeKHR>		presentModes;
	};

	class VULKAN_EXPORTS VulkanPhysicalDevice
	{
	public:
		VulkanPhysicalDevice(const VulkanPhysicalDevice&) = default;
		VulkanPhysicalDevice(VulkanPhysicalDevice&&) noexcept = default;
		VulkanPhysicalDevice& operator=(const VulkanPhysicalDevice&) = default;
		VulkanPhysicalDevice& operator=(VulkanPhysicalDevice&&) noexcept = default;
		~VulkanPhysicalDevice() = default;

		SwapChainSupportDetails querySwapChainSupport( const VulkanSurface& surface );
	private:
		explicit VulkanPhysicalDevice(
			VulkanInstance* pVulkanInstance, 
			const vk::PhysicalDevice& pPhysicalDeviceHandle,
			const std::vector<const char*>& enabledExtensions = {}
		);

		VulkanInstance*		m_pVulkanInstance;
		vk::PhysicalDevice	m_deviceHandle;

		utils::Sptr<vk::PhysicalDeviceFeatures>		m_spDeviceFeatures;
		utils::Sptr<vk::PhysicalDeviceProperties>	m_spDeviceProperties;

		const std::vector<const char*>	m_enabledExtensions;
		

		friend class VulkanLogicalDevice;
		friend class VulkanQueueFamily;
		friend class VulkanPhysicalDeviceManager;
	};
} // namespace vkrender

#endif