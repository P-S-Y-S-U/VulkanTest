#include "application/VulkanApplication.h"
#include "utilities/VulkanLogger.h"
#include "vkrenderer/VulkanLayer.hpp"

#include <set>

void VulkanApplication::pickPhysicalDevice()
{
	using namespace vkrender;

	std::vector<vk::PhysicalDevice, std::allocator<vk::PhysicalDevice>> devices = m_vkInstance.enumeratePhysicalDevices();

	if( devices.empty() )
    {
        std::string errorMsg = "NO VULKAN DEVICE FOUND!";
        LOG_ERROR(errorMsg);
        throw std::runtime_error(errorMsg);
    }

	struct DeviceCandiate
	{
		vk::PhysicalDeviceType mDeviceType;
		bool mbHasGeometryShader;
	};

	auto l_probePhysicalDeviceHandle = []( const vk::PhysicalDevice& vkPhysicalDevice ) {
		const vk::PhysicalDeviceProperties& deviceProperties = vkPhysicalDevice.getProperties();
        LOG_INFO(
            fmt::format("Device ID : {} Device Name : {} Vendor: {}", deviceProperties.deviceID, deviceProperties.deviceName, deviceProperties.vendorID)
        );
	};

	auto l_populateDeviceProperties = []( 
		vk::PhysicalDevice& vkPhysicalDevice, 
		vk::PhysicalDeviceProperties* pPhysicalDeviceProperties, vk::PhysicalDeviceFeatures* pPhysicalDeviceFeatures
	)
	{
		vkPhysicalDevice.getProperties( pPhysicalDeviceProperties );
		vkPhysicalDevice.getFeatures( pPhysicalDeviceFeatures );
	};

	auto l_isDeviceSuitable = [this](
		const vk::PhysicalDevice& physicalDevice, 
		vk::SurfaceKHR* surface,
		const std::vector<const char*>& requiredExtensions,
		const DeviceCandiate& bestCandidate,
		std::uint32_t& deviceScore
	) -> bool{
		QueueFamilyIndices queueFamilyIndices = findQueueFamilyIndices( physicalDevice, surface );

		const vk::PhysicalDeviceFeatures& vkPhysicalDeviceFeatures = physicalDevice.getFeatures();
		const vk::PhysicalDeviceProperties& vkPhysicalDeviceProperties = physicalDevice.getProperties();

		bool bIntegratedGpu = vkPhysicalDeviceProperties.deviceType == vk::PhysicalDeviceType::eIntegratedGpu;
		bool bDiscreteGpu = vkPhysicalDeviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu;
        bool bShader =  vkPhysicalDeviceFeatures.geometryShader;
        bool bSamplerAnisotropy = static_cast<bool>( vkPhysicalDeviceFeatures.samplerAnisotropy );

        bool bGraphicsFamily = queueFamilyIndices.m_graphicsFamily.has_value();
        
		auto l_checkDeviceExtensionSupport = []( const vk::PhysicalDevice& physicalDevice, const std::vector<const char*>& requiredExtensions ){
			std::vector<vk::ExtensionProperties, std::allocator<vk::ExtensionProperties>> availableExtensions = physicalDevice.enumerateDeviceExtensionProperties();

        	std::set<std::string> requiredExtensionQuery( requiredExtensions.begin(), requiredExtensions.end() );

        	for( const auto& deviceExtensionProp : availableExtensions )
        	{
        	    requiredExtensionQuery.erase( deviceExtensionProp.extensionName  );
        	}

        	return requiredExtensionQuery.empty();
		};

		auto l_checkSwapChainAdequacy = [this]( 
			const vk::PhysicalDevice& physicalDevice, 
			const vk::SurfaceKHR& surface, const bool& bExtensionSupported ) -> bool{
	
			SwapChainSupportDetails swapChainDetails = querySwapChainSupport( physicalDevice, surface );

			if( bExtensionSupported )
        	{
        	    return !swapChainDetails.surfaceFormats.empty() && !swapChainDetails.presentModes.empty();
        	}
        	return false;
		};

        bool bExtensionsSupported =  l_checkDeviceExtensionSupport( physicalDevice, requiredExtensions );
        bool bSwapChainAdequate = l_checkSwapChainAdequacy( physicalDevice, *surface, bExtensionsSupported );

		if(bestCandidate.mDeviceType == vkPhysicalDeviceProperties.deviceType)
			deviceScore += 10;
		if( vkPhysicalDeviceFeatures.geometryShader == bestCandidate.mbHasGeometryShader )
			deviceScore += 10;

        return bShader && ( bIntegratedGpu || bDiscreteGpu ) && bGraphicsFamily && bExtensionsSupported && bSwapChainAdequate & bSamplerAnisotropy;
	};

	DeviceCandiate bestCandidate{
		vk::PhysicalDeviceType::eDiscreteGpu,
		true
	};

	//for( auto& vkTemporaryDevice : devices )
	std::uint32_t bestCandidateIndex = 0u;
	std::uint32_t bestDeviceScore = 0u;
	std::vector<const char*> requiredExtensions{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
	for( std::uint32_t deviceIndex = 0u; deviceIndex < devices.size(); deviceIndex++ )
	{
		vk::PhysicalDevice& vkTemporaryDevice = devices[deviceIndex];

		l_probePhysicalDeviceHandle( vkTemporaryDevice );

		vk::PhysicalDeviceFeatures vkPhysicalDeviceFeatures;
		vk::PhysicalDeviceProperties vkPhysicalDeviceProperties;

		l_populateDeviceProperties( vkTemporaryDevice, &vkPhysicalDeviceProperties, &vkPhysicalDeviceFeatures );
		
		std::uint32_t deviceScore = 0u;
		if( l_isDeviceSuitable( vkTemporaryDevice, &m_vkSurface, requiredExtensions, bestCandidate, deviceScore ) )
		{
			if( deviceScore > bestDeviceScore )
			{
				bestDeviceScore = deviceScore;
				bestCandidateIndex = deviceIndex;
			}
		}
	}

	if( bestDeviceScore > 0u )
	{
		m_vkPhysicalDevice = devices[bestCandidateIndex];
		m_msaaSampleCount = getMaxUsableSampleCount();
		m_deviceExtensionContainer = requiredExtensions;
		m_deviceExtensionContainer.shrink_to_fit();

		LOG_INFO("Selected Suitable Vulkan GPU!");
        l_probePhysicalDeviceHandle( m_vkPhysicalDevice );
		return;
	}

	std::string errorMsg = "FAILED TO SELECT A SUITABLE VULKAN GPU!";
    LOG_ERROR(errorMsg);
    throw std::runtime_error(errorMsg);
}

void VulkanApplication::createLogicalDevice()
{
	using namespace vkrender;
	QueueFamilyIndices queueFamilyIndices = findQueueFamilyIndices( m_vkPhysicalDevice, &m_vkSurface );

	logQueueFamilyIndices( queueFamilyIndices );
	
	std::set<std::uint32_t> uniqueQueueFamilies;

	if( queueFamilyIndices.m_graphicsFamily.has_value() ) uniqueQueueFamilies.emplace( queueFamilyIndices.m_graphicsFamily.value() );
	if( queueFamilyIndices.m_presentFamily.has_value() ) uniqueQueueFamilies.emplace( queueFamilyIndices.m_presentFamily.value() );
	if( queueFamilyIndices.m_exclusiveTransferFamily.has_value() ) 
	{
		uniqueQueueFamilies.emplace( queueFamilyIndices.m_exclusiveTransferFamily.value() );
		m_bHasExclusiveTransferQueue = true;
	}
	
	std::vector<vk::DeviceQueueCreateInfo> deviceQueueCreateInfos{ uniqueQueueFamilies.size() };

	std::vector<float> queuePriorities{1};
	queuePriorities[0] = 1.0f;

	auto createInfoIndex = 0u;
	for( const auto& queueFamilyIndex : uniqueQueueFamilies )
	{
		vk::DeviceQueueCreateInfo queueCreateInfo{};
		std::size_t queueCount = 1;
		populateDeviceQueueCreateInfo( queueCreateInfo, queueFamilyIndex, queueCount, queuePriorities.data() );
		deviceQueueCreateInfos[createInfoIndex] = queueCreateInfo;
		createInfoIndex++;
	}

	vk::DeviceCreateInfo vkDeviceCreateInfo{};
	vk::PhysicalDeviceFeatures physicalDeviceFeatures = m_vkPhysicalDevice.getFeatures(); // TODO check state
	populateDeviceCreateInfo( vkDeviceCreateInfo, deviceQueueCreateInfos, &physicalDeviceFeatures );

	m_vkLogicalDevice = m_vkPhysicalDevice.createDevice( vkDeviceCreateInfo );	
	LOG_INFO("Logical Device created");

	m_vkGraphicsQueue = m_vkLogicalDevice.getQueue( queueFamilyIndices.m_graphicsFamily.value(), 0 );
	LOG_INFO("Graphics Queue Retrieved");

	if( queueFamilyIndices.m_presentFamily.has_value() )
	{
		m_vkPresentationQueue = m_vkLogicalDevice.getQueue( queueFamilyIndices.m_presentFamily.value(), 0 );
		LOG_INFO("Presentation Queue Retrieved");
	} 

	if( queueFamilyIndices.m_exclusiveTransferFamily.has_value() )
	{
		m_vkTransferQueue = m_vkLogicalDevice.getQueue( queueFamilyIndices.m_exclusiveTransferFamily.value(), 0 );	
		LOG_INFO("Transfer Queue Retrieved");
	}
	else
	{
		m_vkTransferQueue = m_vkLogicalDevice.getQueue( queueFamilyIndices.m_graphicsFamily.value(), 0 );
		LOG_INFO("Using Graphics Queue for Transfer Operations");
	}
}

vkrender::QueueFamilyIndices VulkanApplication::findQueueFamilyIndices( const vk::PhysicalDevice& physicalDevice, vk::SurfaceKHR* pVkSurface )
{
	vkrender::QueueFamilyIndices queueFamilyIndices;

	std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

	int validQueueIndex = 0;

	for (const auto& prop : queueFamilyProperties)
	{
		if (prop.queueFlags & vk::QueueFlagBits::eGraphics )
		{
			queueFamilyIndices.m_graphicsFamily = validQueueIndex;
		}

		if( prop.queueFlags & vk::QueueFlagBits::eCompute )
		{
			queueFamilyIndices.m_computeFamily = validQueueIndex;
		}

		if( 
			prop.queueFlags & vk::QueueFlagBits::eTransfer && 
			( !( prop.queueFlags & vk::QueueFlagBits::eGraphics ) && !( prop.queueFlags & vk::QueueFlagBits::eCompute ) )
		)
		{
			queueFamilyIndices.m_exclusiveTransferFamily = validQueueIndex;
		}

		if( pVkSurface )
		{
			vk::Bool32 bPresentationSupport = physicalDevice.getSurfaceSupportKHR( validQueueIndex, *pVkSurface );
			if( bPresentationSupport )
			{
				queueFamilyIndices.m_presentFamily = validQueueIndex;					
			}
		}
		validQueueIndex++;
	}

	return queueFamilyIndices;
}

void VulkanApplication::logQueueFamilyIndices( const vkrender::QueueFamilyIndices& queueFamilyIndices )
{
	LOG_DEBUG("Found Queue Family Indices");
	if( queueFamilyIndices.m_graphicsFamily.has_value() )
	{
		LOG_DEBUG(fmt::format("Has Graphics Queue Index: {}", queueFamilyIndices.m_graphicsFamily.value()));
	}

	if( queueFamilyIndices.m_presentFamily.has_value() )
	{
		LOG_DEBUG(fmt::format("Has Presentation Queue Index: {}", queueFamilyIndices.m_presentFamily.value()));
	}

	if( queueFamilyIndices.m_computeFamily.has_value() )
	{
		LOG_DEBUG(fmt::format("Has Compute Queue Index: {}", queueFamilyIndices.m_computeFamily.value()));
	}

	if( queueFamilyIndices.m_exclusiveTransferFamily.has_value() )
	{
		LOG_DEBUG(fmt::format("Has Exclusive Transfer Queue Index: {}", queueFamilyIndices.m_exclusiveTransferFamily.value()));
	}
}

void VulkanApplication::populateDeviceQueueCreateInfo( 
        vk::DeviceQueueCreateInfo& vkDeviceQueueCreateInfo, 
        const std::uint32_t& queueFamilyIndex, 
        const std::uint32_t& queueCount, 
        const float* queuePriorities
)
{
	vkDeviceQueueCreateInfo.queueFamilyIndex = queueFamilyIndex;
	vkDeviceQueueCreateInfo.queueCount = queueCount;
	vkDeviceQueueCreateInfo.pQueuePriorities = queuePriorities;
}

void VulkanApplication::populateDeviceCreateInfo( 
	vk::DeviceCreateInfo& vkDeviceCreateInfo, 
	const std::vector<vk::DeviceQueueCreateInfo>& queueCreateInfos, 
	const vk::PhysicalDeviceFeatures* pEnabledFeatures 
)
{
	using namespace vkrender;

	vkDeviceCreateInfo.enabledExtensionCount = m_deviceExtensionContainer.size();
	vkDeviceCreateInfo.ppEnabledExtensionNames = m_deviceExtensionContainer.data();
	vkDeviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
	vkDeviceCreateInfo.queueCreateInfoCount = queueCreateInfos.size();
	vkDeviceCreateInfo.pEnabledFeatures = pEnabledFeatures;
	if( ENABLE_VALIDATION_LAYER )
	{
		vkDeviceCreateInfo.enabledLayerCount = static_cast<std::uint32_t>(layer::VALIDATION_LAYER.m_layers.size());
		vkDeviceCreateInfo.ppEnabledLayerNames = layer::VALIDATION_LAYER.m_layers.data();
	}
	else {
		vkDeviceCreateInfo.enabledLayerCount = 0;
	}
}


