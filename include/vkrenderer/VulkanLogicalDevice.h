#ifndef VKRENDER_VULKAN_LOGICAL_DEVICE_HPP
#define VKRENDER_VULKAN_LOGICAL_DEVICE_HPP

#include <vulkan/vulkan.hpp>
#include "vkrenderer/VulkanPhysicalDevice.h"
#include "vkrenderer/VulkanQueueFamily.h"
#include "exports.hpp"

namespace vkrender
{
	class VULKAN_EXPORTS VulkanLogicalDevice
	{
	public:
		explicit VulkanLogicalDevice( VulkanPhysicalDevice* pPhysicalDevice );
		~VulkanLogicalDevice();

		void createLogicalDevice();
		void destroyLogicaDevice();
	private:
		vk::Device								m_deviceHandle;

		VulkanPhysicalDevice*				m_pPhysicalDevice;

		utils::Sptr<vk::DeviceQueueCreateInfo>	m_spDeviceQueueCreateInfo;
		utils::Sptr<vk::DeviceCreateInfo>		m_spDeviceCreateInfo;

		QueueFamilyIndices						m_queueFamilyIndices;
		vk::Queue									m_graphicsQueue;
		
		void populateDeviceQueueCreateInfo();
		void populateDeviceCreateInfo();
	
		void createDeviceQueue();
	};
} // namespace vkrender

#endif
