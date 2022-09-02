#include "vkrenderer/VulkanLogicalDevice.h"
#include "vkrenderer/VulkanLayer.hpp"
#include "vkrenderer/VulkanQueueFamily.h"

#include <iostream>
#include <set>

namespace vkrender
{
	VulkanLogicalDevice::VulkanLogicalDevice( VulkanPhysicalDevice* pPhysicalDevice, VulkanSurface* pSurface )
		:m_pPhysicalDevice{ pPhysicalDevice }
		,m_pSurface{ pSurface }
		,m_queueFamilyIndices{ VulkanQueueFamily::findQueueFamilyIndices( *m_pPhysicalDevice, m_pSurface ) }
	{}

	VulkanLogicalDevice::~VulkanLogicalDevice()
	{
		destroyLogicalDevice();
	}

	void VulkanLogicalDevice::createLogicalDevice()
	{
		std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos = populateDeviceQueueCreateInfo();
		populateDeviceCreateInfo( queueCreateInfos );
		
		m_deviceHandle = m_pPhysicalDevice->m_deviceHandle.createDevice( *m_spDeviceCreateInfo );

		for( const auto& deviceQueueCreateInfo : queueCreateInfos )
		{
			m_queueIndicesMap[ deviceQueueCreateInfo.queueFamilyIndex ] = createDeviceQueue( deviceQueueCreateInfo.queueFamilyIndex, 0 );
		}
	}
	
	void VulkanLogicalDevice::destroyLogicalDevice()
	{
		m_deviceHandle.destroy();
	}

	std::vector<vk::DeviceQueueCreateInfo> VulkanLogicalDevice::populateDeviceQueueCreateInfo()
	{	
		std::set<std::uint32_t> uniqueQueueFamilyIndices{ 
			m_queueFamilyIndices.m_graphicsFamily.value(), 
			m_queueFamilyIndices.m_presentFamily.value() 
		};

		std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;

		for( const auto& queueFamilyIndex : uniqueQueueFamilyIndices )
		{
			vk::DeviceQueueCreateInfo deviceQueueCreateInfo{};
			deviceQueueCreateInfo.sType = vk::StructureType::eDeviceQueueCreateInfo;
			deviceQueueCreateInfo.queueFamilyIndex = queueFamilyIndex;
			deviceQueueCreateInfo.queueCount = 1;

			float queue_priority = 1.0f;
			deviceQueueCreateInfo.pQueuePriorities = &queue_priority;

			queueCreateInfos.push_back( deviceQueueCreateInfo );
		}

		return queueCreateInfos;
	}

	void VulkanLogicalDevice::populateDeviceCreateInfo( const std::vector<vk::DeviceQueueCreateInfo>& queueCreateInfos )
	{
		m_spDeviceCreateInfo = std::make_shared<vk::DeviceCreateInfo>();
		m_spDeviceCreateInfo->sType = vk::StructureType::eDeviceCreateInfo;
		m_spDeviceCreateInfo->enabledExtensionCount = 0;
		m_spDeviceCreateInfo->pQueueCreateInfos = queueCreateInfos.data();
		m_spDeviceCreateInfo->queueCreateInfoCount = queueCreateInfos.size();
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

	vk::Queue VulkanLogicalDevice::createDeviceQueue( const std::uint32_t& queueFamilyIndex, const std::uint32_t& index )
	{
		return m_deviceHandle.getQueue( queueFamilyIndex, index );
	}
} // namespace vkrender