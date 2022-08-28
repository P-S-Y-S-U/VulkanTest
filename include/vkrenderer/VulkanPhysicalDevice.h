#ifndef VKRENDER_VULKAN_PHYSICAL_DEVICE_H
#define VKRENDER_VULKAN_PHYSICAL_DEVICE_H

#include <vulkan/vulkan.hpp>
#include "VulkanInstance.h"
#include "exports.hpp"

namespace vkrender
{
	class VULKAN_EXPORTS VulkanPhysicalDevice
	{
	public:
		VulkanPhysicalDevice(const VulkanPhysicalDevice&) = delete;
		VulkanPhysicalDevice(VulkanPhysicalDevice&&) noexcept = default;
		VulkanPhysicalDevice& operator=(const VulkanPhysicalDevice&) = delete;
		VulkanPhysicalDevice& operator=(VulkanPhysicalDevice&&) noexcept = default;
		~VulkanPhysicalDevice() = default;

	private:
		VulkanPhysicalDevice(
			VulkanInstance* pVulkanInstance, 
			vk::PhysicalDevice* pPhysicalDeviceHandle 
		);

		VulkanInstance*		m_pVulkanInstance;
		vk::PhysicalDevice*	m_pDeviceHandle;
		utils::Sptr<vk::PhysicalDeviceFeatures>		m_spDeviceFeatures;
		utils::Sptr<vk::PhysicalDeviceProperties>	m_spDeviceProperties;
		

		friend class VulkanLogicalDevice;
		friend class VulkanQueueFamily;
		friend class VulkanPhysicalDeviceManager;
	};

} // namespace app
#endif