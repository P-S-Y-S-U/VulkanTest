#include "VulkanApplication.h"
#include "vkrenderer/VulkanLayer.hpp"
#include "vkrenderer/VulkanDebugMessenger.h"
#include "utilities/VulkanLogger.h"

#include <vulkan/vulkan.hpp>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <set>

VulkanApplication::VulkanApplication( const std::string& applicationName )
    :m_applicationName{ applicationName }
	,m_window{ 800, 600 }
{
	if (utils::VulkanRendererApiLogger::getSingletonPtr() == nullptr)
	{
		spdlog::sink_ptr consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		std::initializer_list<spdlog::sink_ptr> logSinks{
			consoleSink
		};
		utils::VulkanRendererApiLogger::createInstance(logSinks);
		utils::VulkanRendererApiLogger::getSingletonPtr()->getLogger()->set_level(spdlog::level::debug);
	}
}

VulkanApplication::~VulkanApplication()
{
	shutdown();
}

void VulkanApplication::initialise()
{
	initWindow();
	initVulkan();
}

void VulkanApplication::initWindow()
{
	m_window.init();
}

void VulkanApplication::initVulkan()
{
	createInstance();
	setupDebugMessenger();
	pickPhysicalDevice();
}

void VulkanApplication::shutdown()
{
	using namespace vkrender;

	m_vkInstance.destroySurfaceKHR( m_vkSurface );

	if( ENABLE_VALIDATION_LAYER )
	{
		VulkanDebugMessenger::destroyDebugUtilsMessengerEXT( m_vkInstance, m_vkDebugUtilsMessenger, nullptr );
	}

	m_vkInstance.destroy();
	
	m_window.destroy();
}


void VulkanApplication::createInstance()
{
	using namespace vkrender;

	if (ENABLE_VALIDATION_LAYER && !checkValidationLayerSupport())
	{
		throw std::runtime_error("Validation layers requested, not available");
	}

    vk::ApplicationInfo applicationInfo{};
	applicationInfo.sType = vk::StructureType::eApplicationInfo;
	applicationInfo.pApplicationName = m_applicationName.c_str();
	applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	applicationInfo.pEngineName = "No Engine";
	applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	applicationInfo.apiVersion = VK_API_VERSION_1_3;
	applicationInfo.pNext = nullptr;

	vk::InstanceCreateInfo instanceCreateInfo{};
	instanceCreateInfo.sType = vk::StructureType::eInstanceCreateInfo;
	instanceCreateInfo.pApplicationInfo = &applicationInfo;
	m_extensionContainer = Window::populateAvailableExtensions();
	if( ENABLE_VALIDATION_LAYER )
	{
		m_extensionContainer.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}
	instanceCreateInfo.enabledExtensionCount = static_cast<std::uint32_t>(m_extensionContainer.size());
	instanceCreateInfo.ppEnabledExtensionNames = m_extensionContainer.data();
	vk::DebugUtilsMessengerCreateInfoEXT vkDebugUtilsMessengerCreateInfo{};
	if (ENABLE_VALIDATION_LAYER)
	{
		instanceCreateInfo.enabledLayerCount = static_cast<std::uint32_t>(layer::VALIDATION_LAYER.m_layers.size());
		instanceCreateInfo.ppEnabledLayerNames = layer::VALIDATION_LAYER.m_layers.data();
		populateDebugUtilsMessengerCreateInfo( vkDebugUtilsMessengerCreateInfo );
		instanceCreateInfo.pNext = &vkDebugUtilsMessengerCreateInfo;
	}
	else {
		instanceCreateInfo.enabledLayerCount = 0;
	}

	// creating Vulkan Instance
	if (vk::createInstance(&instanceCreateInfo, nullptr, &m_vkInstance) != vk::Result::eSuccess)
	{
		std::string errorMsg = "FAILED TO CREATE INSTANCE!";
		LOG_ERROR( "FAILED TO CREATE INSTANCE!" );
		throw std::runtime_error( errorMsg );
	}
	LOG_INFO( "Created Vulkan instance successfully" );

}

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
		const std::vector<const char*>& requiredExtensions 
	) -> bool{
		QueueFamilyIndices queueFamilyIndices = findQueueFamilyIndices( physicalDevice, surface );

		const vk::PhysicalDeviceFeatures& vkPhysicalDeviceFeatures = physicalDevice.getFeatures();
		const vk::PhysicalDeviceProperties& vkPhysicalDeviceProperties = physicalDevice.getProperties();

        bool bShader =  vkPhysicalDeviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu && 
                        vkPhysicalDeviceFeatures.geometryShader;
        
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

        return bShader && bGraphicsFamily && bExtensionsSupported && bSwapChainAdequate;
	};

	for( auto& vkTemporaryDevice : devices )
	{
		l_probePhysicalDeviceHandle( vkTemporaryDevice );

		std::vector<const char*> requiredExtensions{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		vk::PhysicalDeviceFeatures vkPhysicalDeviceFeatures;
		vk::PhysicalDeviceProperties vkPhysicalDeviceProperties;

		l_populateDeviceProperties( vkTemporaryDevice, &vkPhysicalDeviceProperties, &vkPhysicalDeviceFeatures );

		if( l_isDeviceSuitable( vkTemporaryDevice, &m_vkSurface, requiredExtensions ) )
		{
			m_vkPhysicalDevice = vkTemporaryDevice;

			LOG_INFO("Selected Suitable Vulkan GPU!");
            l_probePhysicalDeviceHandle( m_vkPhysicalDevice );
			return;
		}
	}

	std::string errorMsg = "FAILED TO SELECT A SUITABLE VULKAN GPU!";
    LOG_ERROR(errorMsg);
    throw std::runtime_error(errorMsg);
}

vkrender::QueueFamilyIndices VulkanApplication::findQueueFamilyIndices( const vk::PhysicalDevice& physicalDevice, vk::SurfaceKHR* pVkSurface )
{
	using namespace vkrender;

	QueueFamilyIndices queueFamilyIndices;

	std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();
		
	int validQueueIndex = 0;

	for (const auto& prop : queueFamilyProperties)
	{
		if (prop.queueFlags & vk::QueueFlagBits::eGraphics )
		{
			queueFamilyIndices.m_graphicsFamily = validQueueIndex;
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

void VulkanApplication::populateDebugUtilsMessengerCreateInfo( vk::DebugUtilsMessengerCreateInfoEXT& vkDebugUtilsMessengerCreateInfo )
{
	using namespace vkrender;

	vkDebugUtilsMessengerCreateInfo.sType = vk::StructureType::eDebugUtilsMessengerCreateInfoEXT;
	vkDebugUtilsMessengerCreateInfo.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
	vkDebugUtilsMessengerCreateInfo.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
	vkDebugUtilsMessengerCreateInfo.pfnUserCallback = VulkanDebugMessenger::debugCallback;
}

vkrender::SwapChainSupportDetails VulkanApplication::querySwapChainSupport( const vk::PhysicalDevice& vkPhysicalDevice, const vk::SurfaceKHR& vkSurface ) const
{
	using namespace vkrender;

	SwapChainSupportDetails swapChainDetails;

	swapChainDetails.capabilities =	vkPhysicalDevice.getSurfaceCapabilitiesKHR( vkSurface );
	swapChainDetails.surfaceFormats = vkPhysicalDevice.getSurfaceFormatsKHR( vkSurface );
	swapChainDetails.presentModes = vkPhysicalDevice.getSurfacePresentModesKHR( vkSurface );

	return swapChainDetails;
}

void VulkanApplication::setupDebugMessenger()
{
	using namespace vkrender;
	vk::DebugUtilsMessengerCreateInfoEXT vkDebugUtilsMessengerCreateInfo{};
	populateDebugUtilsMessengerCreateInfo( vkDebugUtilsMessengerCreateInfo );
	if ( VulkanDebugMessenger::createDebugUtilsMessengerEXT(m_vkInstance, vkDebugUtilsMessengerCreateInfo, nullptr, m_vkDebugUtilsMessenger) != VK_SUCCESS )
	{
		throw std::runtime_error("failed to setup debug messenger!");
	}
}

bool VulkanApplication::checkValidationLayerSupport()
{
	using namespace vkrender;

	// Enumerating Insatnce layer properties
	std::vector<vk::LayerProperties> availableLayers = vk::enumerateInstanceLayerProperties();

	// validating Layer support
	for (const auto& layerName : layer::VALIDATION_LAYER.m_layers)
	{
		bool bLayerFound = false;

		for (const auto& layerProperties : availableLayers)
		{
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{
				bLayerFound = true;
			}
		}
		if (!bLayerFound) { return false;  }
	}

	return true;
}