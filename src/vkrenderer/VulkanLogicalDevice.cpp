#include "vkrenderer/VulkanLogicalDevice.h"
#include "vkrenderer/VulkanLayer.hpp"
#include "vkrenderer/VulkanQueueFamily.h"
#include <iostream>

namespace vkrender
{
	VulkanLogicalDevice::VulkanLogicalDevice( VulkanPhysicalDevice* pPhysicalDevice )
		:m_pPhysicalDevice{ pPhysicalDevice }
		,m_queueFamilyIndices{ VulkanQueueFamily::findQueueFamilyIndices( *m_pPhysicalDevice ) }
	{}

	VulkanLogicalDevice::~VulkanLogicalDevice()
	{
		destroyLogicaDevice();
	}

	void VulkanLogicalDevice::createLogicalDevice()
	{
		populateDeviceQueueCreateInfo();
		populateDeviceCreateInfo();
		
		m_deviceHandle = m_pPhysicalDevice->m_deviceHandle.createDevice( *m_spDeviceCreateInfo );
		createDeviceQueue();
	}
	
	void VulkanLogicalDevice::destroyLogicaDevice()
	{
		m_deviceHandle.destroy();
	}

	void VulkanLogicalDevice::populateDeviceQueueCreateInfo()
	{
		vk::PhysicalDevice& vulkan_physical_device = m_pPhysicalDevice->m_deviceHandle;

		m_spDeviceQueueCreateInfo = std::make_shared<vk::DeviceQueueCreateInfo>();
		m_spDeviceQueueCreateInfo->sType = vk::StructureType::eDeviceQueueCreateInfo;
		m_spDeviceQueueCreateInfo->queueFamilyIndex = m_queueFamilyIndices.m_graphicsFamily.value();
		m_spDeviceQueueCreateInfo->queueCount = 1;

		float queue_priority = 1.0f;
		m_spDeviceQueueCreateInfo->pQueuePriorities = &queue_priority;
	}

	void VulkanLogicalDevice::populateDeviceCreateInfo()
	{
		m_spDeviceCreateInfo = std::make_shared<vk::DeviceCreateInfo>();
		m_spDeviceCreateInfo->sType = vk::StructureType::eDeviceCreateInfo;
		m_spDeviceCreateInfo->enabledExtensionCount = 0;
		m_spDeviceCreateInfo->pQueueCreateInfos = m_spDeviceQueueCreateInfo.get();
		m_spDeviceCreateInfo->queueCreateInfoCount = 1;
		m_spDeviceCreateInfo->pEnabledFeatures = m_pPhysicalDevice->m_spDeviceFeatures.get();
		if( VulkanInstance::ENABLE_VALIDATION_LAYER )
		{
			m_spDeviceCreateInfo->ppEnabledLayerNames = layer::VALIDATION_LAYER.m_layers.data();
			m_spDeviceCreateInfo->enabledLayerCount = static_cast<std::uint32_t>(layer::VALIDATION_LAYER.m_layers.size());
		}
		else {
			m_spDeviceCreateInfo->enabledLayerCount = 0;
		}
	}

	void VulkanLogicalDevice::createDeviceQueue()
	{
		m_graphicsQueue = m_deviceHandle.getQueue( m_queueFamilyIndices.m_graphicsFamily.value(), 0 );
	}
} // namespace vkrender