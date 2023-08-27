#include "application/VulkanApplication.h"
#include "vkrenderer/VulkanLayer.hpp"
#include "vkrenderer/VulkanDebugMessenger.h"
#include "vkrenderer/VulkanSwapChainFactory.h"
#include "vkrenderer/VulkanUBO.hpp"
#include "graphics/Vertex.hpp"
#include "utilities/VulkanLogger.h"

#include <vulkan/vulkan.hpp>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <set>
#include <fstream>
#include <vector>
#include <unordered_map>

#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

vk::SurfaceFormatKHR chooseSwapSurfaceFormat( const vkrender::SwapChainSupportDetails& swapChainSupportDetails );
vk::PresentModeKHR chooseSwapPresentMode( const vkrender::SwapChainSupportDetails& swapChainSupportDetails );
vk::Extent2D chooseSwapExtent( const vkrender::SwapChainSupportDetails& swapChainSupportDetails, const vkrender::Window& window );
std::uint32_t chooseImageCount( const vkrender::SwapChainSupportDetails& swapChainSupportDetails );


vk::SurfaceFormatKHR chooseSwapSurfaceFormat( const vkrender::SwapChainSupportDetails& swapChainSupportDetails )
{
    for( const auto& availableFormat : swapChainSupportDetails.surfaceFormats )
    {
        if( availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear )
        {
            return availableFormat;
        }
    }
    return swapChainSupportDetails.surfaceFormats[0];
}

vk::PresentModeKHR chooseSwapPresentMode( const vkrender::SwapChainSupportDetails& swapChainSupportDetails )
{
    for( const auto& availablePresentMode : swapChainSupportDetails.presentModes )
    {
        if( availablePresentMode == vk::PresentModeKHR::eMailbox )
        {
            return availablePresentMode;
        }
    }
    return vk::PresentModeKHR::eFifo;
}

vk::Extent2D chooseSwapExtent( const vkrender::SwapChainSupportDetails& swapChainSupportDetials, const vkrender::Window& window )
{
    const vk::SurfaceCapabilitiesKHR& surfaceCapabilities = swapChainSupportDetials.capabilities;

    if( surfaceCapabilities.currentExtent.width != std::numeric_limits<std::uint32_t>::max() )
    {
        return surfaceCapabilities.currentExtent;
    }
    else
    {
        const auto& [width, height] = window.getFrameBufferSize();

        vk::Extent2D actualExtent{
            width, height
        };

        actualExtent.width = std::clamp(actualExtent.width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width );
        actualExtent.height = std::clamp(actualExtent.height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height );

        return actualExtent;
    }
}
 
std::uint32_t chooseImageCount( const vkrender::SwapChainSupportDetails& swapChainSupportDetails )
{
    std::uint32_t imageCount = swapChainSupportDetails.capabilities.minImageCount + 1; // always ask for minImageCount + 1

    if( swapChainSupportDetails.capabilities.maxImageCount > 0 && imageCount > swapChainSupportDetails.capabilities.maxImageCount )
    {
        imageCount = swapChainSupportDetails.capabilities.maxImageCount;
    }

    return imageCount;
}

VulkanApplication::VulkanApplication( const std::string& applicationName )
    :m_applicationName{ applicationName }
	,m_window{ 800, 600 }
	,m_currentFrame{0}
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

void VulkanApplication::initialise( const std::filesystem::path& modelPath, const std::filesystem::path& texturePath )
{
	m_modelFilePath = modelPath; 
	m_textureImageFilePath = texturePath;

	initialise();
}

void VulkanApplication::updateUniformBuffer( const std::uint32_t& currentFrame )
{
	auto timeNow = std::chrono::high_resolution_clock::now();

	auto duration = std::chrono::duration<float, std::chrono::seconds::period>(timeNow - m_simulationStart).count();

	VulkanUniformBufferObject ubo{};

	ubo.model = glm::rotate(
		glm::mat4{1.0f},
		duration * glm::radians( 90.0f ),
		glm::vec3{ 0.0, 0.0, 1.0 }
	);
	ubo.view = glm::lookAt(
		glm::vec3{ 2.0f, 2.0f, 2.0f },
		glm::vec3{ 0.0f, 0.0f, 0.0f },
		glm::vec3{ 0.0f, 0.0f, 1.0f }
	);
	ubo.projection = glm::perspective(
		glm::radians( 45.0f ), 
		m_vkSwapchainExtent.width / (float) m_vkSwapchainExtent.height,
		0.1f,
		10.0f
	);
	ubo.projection[1][1] *= -1.0f;

	std::memcpy( m_uniformBuffersMapped[currentFrame], &ubo, sizeof(ubo) );
	
}

void VulkanApplication::initWindow()
{
	m_window.init();
}

void VulkanApplication::initVulkan()
{
	createInstance();
	setupDebugMessenger();
	createSurface();
	pickPhysicalDevice();
	createLogicalDevice();
	createSwapchain();
	createSwapChainImageViews();
	createRenderPass();
	createDescriptorSetLayout();
	createGraphicsPipeline();
	createCommandPool();
	createConfigCommandBuffer();
	createDepthResources();
	createFrameBuffers();
	createTextureImage();
	createTextureImageView();
	createTextureSampler();
	if( std::filesystem::exists(m_modelFilePath) )
		loadModel();
	createVertexBuffer();
	createIndexBuffer();
	createUniformBuffers();
	createDescriptorPool();
	createDescriptorSets();
	createGraphicsCommandBuffers();
	createSyncObjects();
}

void VulkanApplication::mainLoop()
{
	m_simulationStart = std::chrono::high_resolution_clock::now();

	while( !m_window.quit() )
	{
		m_window.processEvents();
		drawFrame();
	}
	m_vkLogicalDevice.waitIdle();
}

void VulkanApplication::drawFrame()
{
	auto opFenceWait = m_vkLogicalDevice.waitForFences( 1, &m_vkInFlightFences[m_currentFrame], VK_TRUE, std::numeric_limits<std::uint64_t>::max() ); 

	std::uint32_t imageIndex;
	vk::ResultValue<std::uint32_t> opImageAcquistion = this->swapchainNextImageWrapper(
		m_vkLogicalDevice,
		m_vkSwapchain, 
		std::numeric_limits<std::uint64_t>::max(),
		m_vkImageAvailableSemaphores[m_currentFrame],
		nullptr
	);

	if( opImageAcquistion.result == vk::Result::eErrorOutOfDateKHR )
	{
		recreateSwapChain();
		return;
	}
	else if( opImageAcquistion.result != vk::Result::eSuccess && opImageAcquistion.result != vk::Result::eSuboptimalKHR )
	{
		std::string errorMsg = "FAILED TO ACQUIRE SWAPCHAIN IMAGE TO START RENDERING";
		LOG_ERROR(errorMsg);
		throw std::runtime_error( errorMsg );
	}

	m_timeSinceLastUpdateFrame = std::chrono::high_resolution_clock::now();
	updateUniformBuffer(m_currentFrame);
	
	// only reset the fence if we are submitting for work
	auto opFenceReset = m_vkLogicalDevice.resetFences( 1, &m_vkInFlightFences[m_currentFrame] );

	imageIndex = opImageAcquistion.value;

	m_vkGraphicsCommandBuffers[m_currentFrame].reset();
	recordCommandBuffer( m_vkGraphicsCommandBuffers[m_currentFrame], imageIndex );

	vk::SubmitInfo vkCmdSubmitInfo{};
	vkCmdSubmitInfo.sType = vk::StructureType::eSubmitInfo;
	vk::Semaphore waitSemaphores[] = { m_vkImageAvailableSemaphores[m_currentFrame] };
	vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
	vkCmdSubmitInfo.waitSemaphoreCount = 1;
	vkCmdSubmitInfo.pWaitSemaphores = waitSemaphores;
	vkCmdSubmitInfo.pWaitDstStageMask = waitStages;
	vkCmdSubmitInfo.commandBufferCount = 1;
	vkCmdSubmitInfo.pCommandBuffers = &m_vkGraphicsCommandBuffers[m_currentFrame];
	vk::Semaphore signalSemaphores[] = { m_vkRenderFinishedSemaphores[m_currentFrame] };
	vkCmdSubmitInfo.signalSemaphoreCount = 1;
	vkCmdSubmitInfo.pSignalSemaphores = signalSemaphores;

	vk::ArrayProxy<const vk::SubmitInfo> submitInfos{ vkCmdSubmitInfo };
	m_vkGraphicsQueue.submit( submitInfos, m_vkInFlightFences[m_currentFrame] );

	vk::PresentInfoKHR vkPresentInfo{};
	vkPresentInfo.sType = vk::StructureType::ePresentInfoKHR;
	vkPresentInfo.waitSemaphoreCount = 1;
	vkPresentInfo.pWaitSemaphores = signalSemaphores;
	vk::SwapchainKHR swapchains[] = { m_vkSwapchain };
	vkPresentInfo.swapchainCount = 1;
	vkPresentInfo.pSwapchains = swapchains;
	vkPresentInfo.pImageIndices = &imageIndex;
	vkPresentInfo.pResults = nullptr;

	vk::Result opPresentResult = queuePresentWrapper( 
		m_vkPresentationQueue,
		vkPresentInfo
	);
	
	if( opPresentResult == vk::Result::eErrorOutOfDateKHR || opPresentResult == vk::Result::eSuboptimalKHR || m_window.isFrameBufferResized() )
	{
		recreateSwapChain();
	}
	else if( opPresentResult != vk::Result::eSuccess )
	{
		std::string errorMsg = "FAILED TO PRESENT SWAPCHAIN IMAGE";
		LOG_ERROR(errorMsg);
		throw std::runtime_error( errorMsg );
	}

	m_currentFrame = ( m_currentFrame + 1 ) % MAX_FRAMES_IN_FLIGHT;
}

void VulkanApplication::shutdown()
{
	using namespace vkrender;

	for( auto i = 0u; i < MAX_FRAMES_IN_FLIGHT; i++ )
	{
		m_vkLogicalDevice.destroyFence( m_vkInFlightFences[i] );
		m_vkLogicalDevice.destroySemaphore( m_vkRenderFinishedSemaphores[i] );
		m_vkLogicalDevice.destroySemaphore( m_vkImageAvailableSemaphores[i] );
	}

	m_vkLogicalDevice.destroyCommandPool( m_vkTransferCommandPool );
	m_vkLogicalDevice.destroyCommandPool( m_vkGraphicsCommandPool );

	destroySwapChain();

	m_vkLogicalDevice.destroySampler( m_vkTextureSampler );
	m_vkLogicalDevice.destroyImageView( m_vkTextureImageView );
	m_vkLogicalDevice.destroyImage( m_vkTextureImage );
	m_vkLogicalDevice.freeMemory( m_vkTextureImageMemory );

	m_vkLogicalDevice.destroyBuffer( m_vkIndexBuffer );
	m_vkLogicalDevice.freeMemory( m_vkIndexBufferMemory );

	m_vkLogicalDevice.destroyBuffer( m_vkVertexBuffer );
	m_vkLogicalDevice.freeMemory( m_vkVertexBufferMemory );

	for( std::size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ )
	{
		m_vkLogicalDevice.destroyBuffer( m_vkUniformBuffers[i] );
		m_vkLogicalDevice.freeMemory( m_vkUniformBuffersMemory[i] );
	}

	m_vkLogicalDevice.destroyDescriptorPool( m_vkDescriptorPool );
	m_vkLogicalDevice.destroyDescriptorSetLayout( m_vkDescriptorSetLayout );

	m_vkLogicalDevice.destroyPipeline( m_vkGraphicsPipeline );
	m_vkLogicalDevice.destroyPipelineLayout( m_vkPipelineLayout );
	m_vkLogicalDevice.destroyRenderPass( m_vkRenderPass );

	m_vkLogicalDevice.destroy();
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
	m_instanceExtensionContainer = Window::populateAvailableExtensions();
	if( ENABLE_VALIDATION_LAYER )
	{
		m_instanceExtensionContainer.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}
	instanceCreateInfo.enabledExtensionCount = static_cast<std::uint32_t>(m_instanceExtensionContainer.size());
	instanceCreateInfo.ppEnabledExtensionNames = m_instanceExtensionContainer.data();
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

void VulkanApplication::createSurface()
{
	vk::Win32SurfaceCreateInfoKHR surfaceCreateInfo;
	surfaceCreateInfo.sType = vk::StructureType::eWin32SurfaceCreateInfoKHR;
	surfaceCreateInfo.hwnd = m_window.getHandle();
	surfaceCreateInfo.hinstance = GetModuleHandle( nullptr );

	m_vkSurface = m_vkInstance.createWin32SurfaceKHR( surfaceCreateInfo );
	LOG_INFO( "Vulkan Surface Created" );
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

        return bShader && bGraphicsFamily && bExtensionsSupported && bSwapChainAdequate & bSamplerAnisotropy;
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
			m_deviceExtensionContainer = requiredExtensions;
			m_deviceExtensionContainer.shrink_to_fit();

			LOG_INFO("Selected Suitable Vulkan GPU!");
            l_probePhysicalDeviceHandle( m_vkPhysicalDevice );
			return;
		}
	}

	std::string errorMsg = "FAILED TO SELECT A SUITABLE VULKAN GPU!";
    LOG_ERROR(errorMsg);
    throw std::runtime_error(errorMsg);
}

void VulkanApplication::createLogicalDevice()
{
	using namespace vkrender;
	QueueFamilyIndices queueFamilyIndices = findQueueFamilyIndices( m_vkPhysicalDevice, &m_vkSurface );

	vk::DeviceQueueCreateInfo vkDeviceGraphicsQueueCreateInfo{};	
	std::uint32_t graphicsQueueFamilyIndex = queueFamilyIndices.m_graphicsFamily.value();
	std::size_t graphicsQueueCount = 1;
	std::vector<float> graphicsQueuePriorities( graphicsQueueCount ); // TODO check state
	graphicsQueuePriorities[0] = 1.0f;
	graphicsQueuePriorities.shrink_to_fit();
	populateDeviceQueueCreateInfo( vkDeviceGraphicsQueueCreateInfo, graphicsQueueFamilyIndex, graphicsQueuePriorities );

	vk::DeviceQueueCreateInfo vkDevicePresentationQueueCreateInfo{};
	std::size_t presentationQueueCount = 1;
	std::vector<float> presentationQueuePriorities( presentationQueueCount );
	presentationQueuePriorities[0] = 1.0f;
	presentationQueuePriorities.shrink_to_fit();
	if( queueFamilyIndices.m_presentFamily.has_value() )
	{	
		std::uint32_t presentationQueueFamilyIndex = queueFamilyIndices.m_presentFamily.value();
		populateDeviceQueueCreateInfo( vkDevicePresentationQueueCreateInfo, presentationQueueFamilyIndex, presentationQueuePriorities );
	}

	vk::DeviceQueueCreateInfo vkDeviceTransferQueueCreateInfo{};
	std::size_t transferQueueCount = 1;
	std::vector<float> transferQueuePriorities( transferQueueCount );
	transferQueuePriorities[0] = 1.0f;
	transferQueuePriorities.shrink_to_fit();
	if( queueFamilyIndices.m_exclusiveTransferFamily.has_value() )
	{
		std::uint32_t transferQueueFamilyIndex = queueFamilyIndices.m_exclusiveTransferFamily.value();
		populateDeviceQueueCreateInfo( vkDeviceTransferQueueCreateInfo, transferQueueFamilyIndex, transferQueuePriorities );
	}

	vk::DeviceCreateInfo vkDeviceCreateInfo{};
	std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
	queueCreateInfos.push_back( vkDeviceGraphicsQueueCreateInfo );
	if( queueFamilyIndices.m_presentFamily.has_value() )
		queueCreateInfos.push_back( vkDevicePresentationQueueCreateInfo );
	if( queueFamilyIndices.m_exclusiveTransferFamily.has_value() )
		queueCreateInfos.push_back( vkDeviceTransferQueueCreateInfo );
	queueCreateInfos.shrink_to_fit();
	vk::PhysicalDeviceFeatures physicalDeviceFeatures = m_vkPhysicalDevice.getFeatures(); // TODO check state
	populateDeviceCreateInfo( vkDeviceCreateInfo, queueCreateInfos, &physicalDeviceFeatures );

	m_vkLogicalDevice = m_vkPhysicalDevice.createDevice( vkDeviceCreateInfo );	
	LOG_INFO("Logical Device created");

	m_vkGraphicsQueue = m_vkLogicalDevice.getQueue( graphicsQueueFamilyIndex, 0 );
	LOG_INFO("Graphics Queue created");

	if( queueFamilyIndices.m_presentFamily.has_value() )
	{
		m_vkPresentationQueue = m_vkLogicalDevice.getQueue( queueFamilyIndices.m_presentFamily.value(), 0 );
		LOG_INFO("Presentation Queue created");
	}

	if( queueFamilyIndices.m_exclusiveTransferFamily.has_value() )
	{
		m_vkTransferQueue = m_vkLogicalDevice.getQueue( queueFamilyIndices.m_exclusiveTransferFamily.value(), 0 );	
		LOG_INFO("Transfer Queue created");
	}
}

void VulkanApplication::createSwapchain()
{
	using namespace vkrender;

	const SwapChainSupportDetails& swapChainSupportDetails = querySwapChainSupport(m_vkPhysicalDevice, m_vkSurface);

    const vk::SurfaceCapabilitiesKHR& capabilities = swapChainSupportDetails.capabilities;
    vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat( swapChainSupportDetails );
    std::uint32_t imageCount = chooseImageCount( swapChainSupportDetails );
    vk::Extent2D imageExtent = chooseSwapExtent( swapChainSupportDetails, m_window );
    vk::PresentModeKHR presentMode = chooseSwapPresentMode( swapChainSupportDetails );

	vkrender::QueueFamilyIndices queueFamilyIndices = findQueueFamilyIndices( m_vkPhysicalDevice, &m_vkSurface );
    vk::SharingMode sharingMode;
	std::vector<std::uint32_t> queueFamilyContainer;
    if( queueFamilyIndices.m_graphicsFamily.value() != queueFamilyIndices.m_presentFamily.value() )
    {
        sharingMode = vk::SharingMode::eConcurrent;
        queueFamilyContainer.push_back( queueFamilyIndices.m_graphicsFamily.value() );
        queueFamilyContainer.push_back( queueFamilyIndices.m_presentFamily.value() );
		LOG_INFO("Different Queue Familiy for Graphics and Presentation using Concurrent mode for swapchain");
    }
    else
    {
        sharingMode = vk::SharingMode::eExclusive;
		LOG_INFO("Different Queue Familiy for Graphics and Presentation using Exclusive mode for swapchain");
    }
	queueFamilyContainer.shrink_to_fit();

	vk::SwapchainCreateInfoKHR vkSwapChainCreateInfo{};
	vkSwapChainCreateInfo.sType = vk::StructureType::eSwapchainCreateInfoKHR;
	vkSwapChainCreateInfo.surface = m_vkSurface;
	vkSwapChainCreateInfo.imageFormat = surfaceFormat.format;
    vkSwapChainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
    vkSwapChainCreateInfo.minImageCount = imageCount;
    vkSwapChainCreateInfo.imageExtent = imageExtent;
    vkSwapChainCreateInfo.imageArrayLayers = 1;
    vkSwapChainCreateInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
    vkSwapChainCreateInfo.imageSharingMode = vk::SharingMode::eExclusive; // TODO subject to break
    vkSwapChainCreateInfo.queueFamilyIndexCount = queueFamilyContainer.size();
    vkSwapChainCreateInfo.pQueueFamilyIndices = queueFamilyContainer.data();
    vkSwapChainCreateInfo.preTransform = capabilities.currentTransform;
    vkSwapChainCreateInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    vkSwapChainCreateInfo.presentMode = presentMode;
    vkSwapChainCreateInfo.clipped = VK_TRUE;
    vkSwapChainCreateInfo.oldSwapchain = nullptr;
	
	m_vkSwapchain = m_vkLogicalDevice.createSwapchainKHR( vkSwapChainCreateInfo );
	m_swapchainImages = m_vkLogicalDevice.getSwapchainImagesKHR( m_vkSwapchain );
	m_vkSwapchainImageFormat = surfaceFormat.format;
	m_vkSwapchainExtent = imageExtent;

	LOG_INFO("Swapchain Created");
}

void VulkanApplication::createSwapChainImageViews()
{
	m_swapchainImageViews.resize( m_swapchainImages.size() );

	for( auto i = 0u; i < m_swapchainImages.size(); i++ )
	{		
		m_swapchainImageViews[i] = createImageView( m_swapchainImages[i], m_vkSwapchainImageFormat, vk::ImageAspectFlagBits::eColor );
	}
	m_swapchainImageViews.shrink_to_fit();

	LOG_INFO("Swapchain ImageViews created");
}

void VulkanApplication::createRenderPass()
{
	vk::AttachmentDescription vkColorAttachment{};
	vkColorAttachment.format = m_vkSwapchainImageFormat;
	vkColorAttachment.samples = vk::SampleCountFlagBits::e1;
	vkColorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	vkColorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
	vkColorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	vkColorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	vkColorAttachment.initialLayout = vk::ImageLayout::eUndefined;
	vkColorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

	vk::AttachmentReference vkColorAttachmentRef{};
	vkColorAttachmentRef.attachment = 0;
	vkColorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

	vk::AttachmentDescription vkDepthAttachment{};
	vkDepthAttachment.format = findDepthFormat();
	vkDepthAttachment.samples = vk::SampleCountFlagBits::e1;
	vkDepthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	vkDepthAttachment.storeOp = vk::AttachmentStoreOp::eDontCare;
	vkDepthAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	vkDepthAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	vkDepthAttachment.initialLayout = vk::ImageLayout::eUndefined;
	vkDepthAttachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

	vk::AttachmentReference vkDepthAttachmentRef{};
	vkDepthAttachmentRef.attachment = 1;
	vkDepthAttachmentRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

	vk::SubpassDescription vkSubPassDesc{};
	vkSubPassDesc.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	vkSubPassDesc.colorAttachmentCount = 1;
	vkSubPassDesc.pColorAttachments = &vkColorAttachmentRef;
	vkSubPassDesc.pDepthStencilAttachment = &vkDepthAttachmentRef;

	vk::SubpassDependency vkSubpassDependency{};
	vkSubpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	vkSubpassDependency.dstSubpass = 0;
	vkSubpassDependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
	vkSubpassDependency.srcAccessMask = vk::AccessFlagBits::eNone;
	vkSubpassDependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
	vkSubpassDependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

	std::array<vk::AttachmentDescription, 2> attachments{ vkColorAttachment, vkDepthAttachment };
	vk::RenderPassCreateInfo vkRenderPassInfo{};
	vkRenderPassInfo.sType = vk::StructureType::eRenderPassCreateInfo;
	vkRenderPassInfo.attachmentCount = static_cast<std::uint32_t>( attachments.size() );
	vkRenderPassInfo.pAttachments = attachments.data();
	vkRenderPassInfo.subpassCount = 1;
	vkRenderPassInfo.pSubpasses = &vkSubPassDesc;
	vkRenderPassInfo.dependencyCount = 1;
	vkRenderPassInfo.pDependencies = &vkSubpassDependency;

	m_vkRenderPass = m_vkLogicalDevice.createRenderPass( vkRenderPassInfo );

	LOG_INFO("RenderPass created");
}

void VulkanApplication::createDescriptorSetLayout()
{
	vk::DescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eVertex;
	uboLayoutBinding.pImmutableSamplers = nullptr;

	vk::DescriptorSetLayoutBinding samplerLayoutBinding{};
	samplerLayoutBinding.binding = 1;
	samplerLayoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
	samplerLayoutBinding.pImmutableSamplers = nullptr;
	samplerLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

	std::array<vk::DescriptorSetLayoutBinding, 2> bindings{ uboLayoutBinding, samplerLayoutBinding };

	vk::DescriptorSetLayoutCreateInfo descLayoutInfo{};
	descLayoutInfo.sType = vk::StructureType::eDescriptorSetLayoutCreateInfo;
	descLayoutInfo.bindingCount = static_cast<std::uint32_t>( bindings.size() );
	descLayoutInfo.pBindings = bindings.data();

	m_vkDescriptorSetLayout = m_vkLogicalDevice.createDescriptorSetLayout( descLayoutInfo );
}

void VulkanApplication::createDescriptorPool()
{
	std::array<vk::DescriptorPoolSize, 2> descPoolSizes;
	descPoolSizes[0].type = vk::DescriptorType::eUniformBuffer;
	descPoolSizes[0].descriptorCount = static_cast<std::uint32_t>( MAX_FRAMES_IN_FLIGHT );
	descPoolSizes[1].type = vk::DescriptorType::eCombinedImageSampler;
	descPoolSizes[1].descriptorCount = static_cast<std::uint32_t>( MAX_FRAMES_IN_FLIGHT );

	vk::DescriptorPoolCreateInfo descCreateInfo{};
	descCreateInfo.sType = vk::StructureType::eDescriptorPoolCreateInfo;
	descCreateInfo.poolSizeCount = static_cast<std::uint32_t>( descPoolSizes.size() );
	descCreateInfo.pPoolSizes = descPoolSizes.data();
	descCreateInfo.maxSets = static_cast<std::uint32_t>( MAX_FRAMES_IN_FLIGHT );

	m_vkDescriptorPool = m_vkLogicalDevice.createDescriptorPool( descCreateInfo );
}

void VulkanApplication::createDescriptorSets()
{
	std::vector<vk::DescriptorSetLayout> descLayouts( MAX_FRAMES_IN_FLIGHT, m_vkDescriptorSetLayout );

	vk::DescriptorSetAllocateInfo descSetAllocInfo{};
	descSetAllocInfo.sType = vk::StructureType::eDescriptorSetAllocateInfo;
	descSetAllocInfo.descriptorPool = m_vkDescriptorPool;
	descSetAllocInfo.descriptorSetCount = static_cast<std::uint32_t>(MAX_FRAMES_IN_FLIGHT);
	descSetAllocInfo.pSetLayouts = descLayouts.data();

	m_vkDescriptorSets = m_vkLogicalDevice.allocateDescriptorSets( descSetAllocInfo );


	for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vk::DescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = m_vkUniformBuffers[i];
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(VulkanUniformBufferObject);

		vk::DescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		imageInfo.imageView = m_vkTextureImageView;
		imageInfo.sampler = m_vkTextureSampler;

		std::array<vk::WriteDescriptorSet, 2> descWrites;
		
		descWrites[0].sType = vk::StructureType::eWriteDescriptorSet;
		descWrites[0].dstSet = m_vkDescriptorSets[i];
		descWrites[0].dstBinding = 0;
		descWrites[0].dstArrayElement = 0;
		descWrites[0].descriptorType = vk::DescriptorType::eUniformBuffer;
		descWrites[0].descriptorCount = 1;
		descWrites[0].pBufferInfo = &bufferInfo;
		descWrites[0].pImageInfo = nullptr;
		descWrites[0].pTexelBufferView = nullptr;

		descWrites[1].sType = vk::StructureType::eWriteDescriptorSet;
		descWrites[1].dstSet = m_vkDescriptorSets[i];
		descWrites[1].dstBinding = 1;
		descWrites[1].dstArrayElement = 0;
		descWrites[1].descriptorType = vk::DescriptorType::eCombinedImageSampler;
		descWrites[1].descriptorCount = 1;
		descWrites[1].pBufferInfo = nullptr;
		descWrites[1].pImageInfo = &imageInfo;
		descWrites[1].pTexelBufferView = nullptr;

		m_vkLogicalDevice.updateDescriptorSets( descWrites, {} );
	}
}

void VulkanApplication::createGraphicsPipeline()
{
	auto l_populatePipelineShaderStageCreateInfo = []( 
		vk::PipelineShaderStageCreateInfo& shaderStageCreateInfo,
		const vk::ShaderStageFlagBits& shaderStage,
		const vk::ShaderModule& shaderModule,
		const std::string& entryPoint
	)
	{
		shaderStageCreateInfo.sType = vk::StructureType::ePipelineShaderStageCreateInfo;
		shaderStageCreateInfo.stage = shaderStage;
		shaderStageCreateInfo.module = shaderModule;
		shaderStageCreateInfo.pName =  entryPoint.c_str();
		shaderStageCreateInfo.pSpecializationInfo = nullptr;
	};

	std::filesystem::path vertexShaderPath = "triangleVert.spv";
    std::filesystem::path fragmentShaderPath = "triangleFrag.spv";

	std::vector<char> vertexShaderBuffer; 
	std::vector<char> fragmentShaderBuffer;

	populateShaderBufferFromSourceFile( vertexShaderPath, vertexShaderBuffer );
	populateShaderBufferFromSourceFile( fragmentShaderPath, fragmentShaderBuffer );

	vk::ShaderModule vertexShaderModule = createShaderModule( vertexShaderBuffer );
	vk::ShaderModule fragmentShaderModule = createShaderModule( fragmentShaderBuffer );
	
	vk::PipelineShaderStageCreateInfo vkPipelineVertexShaderStageCreateInfo;
	vk::PipelineShaderStageCreateInfo vkPipelineFragmentShaderStageCreateInfo;

	std::string vertexShaderEntryPoint = "main";
	std::string fragmentShaderEntryPoint = "main";

	l_populatePipelineShaderStageCreateInfo( 
		vkPipelineVertexShaderStageCreateInfo, vk::ShaderStageFlagBits::eVertex, vertexShaderModule, vertexShaderEntryPoint
	);
	l_populatePipelineShaderStageCreateInfo(
		vkPipelineFragmentShaderStageCreateInfo, vk::ShaderStageFlagBits::eFragment, fragmentShaderModule, fragmentShaderEntryPoint
	);

	vk::PipelineShaderStageCreateInfo vkShaderStages[] = {
		vkPipelineVertexShaderStageCreateInfo, 
		vkPipelineFragmentShaderStageCreateInfo
	};
	
	vk::PipelineVertexInputStateCreateInfo vkVertexInputInfo{};
	vkVertexInputInfo.sType = vk::StructureType::ePipelineVertexInputStateCreateInfo;
	vk::VertexInputBindingDescription bindingDesc = vertex::getBindingDescription();
	std::array<vk::VertexInputAttributeDescription, 3> attributeDesc = vertex::getAttributeDescriptions();
	vkVertexInputInfo.vertexBindingDescriptionCount = 1;
	vkVertexInputInfo.pVertexBindingDescriptions = &bindingDesc;
	vkVertexInputInfo.vertexAttributeDescriptionCount = attributeDesc.size();
	vkVertexInputInfo.pVertexAttributeDescriptions = attributeDesc.data();

	vk::PipelineInputAssemblyStateCreateInfo vkInputAssemblyInfo{};
	vkInputAssemblyInfo.sType = vk::StructureType::ePipelineInputAssemblyStateCreateInfo;
	vkInputAssemblyInfo.topology = vk::PrimitiveTopology::eTriangleList;
	vkInputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

	vk::PipelineViewportStateCreateInfo vkViewportInfo{};
	vkViewportInfo.sType = vk::StructureType::ePipelineViewportStateCreateInfo;
	vkViewportInfo.viewportCount = 1;
	vkViewportInfo.pViewports = nullptr;
	vkViewportInfo.scissorCount = 1;
	vkViewportInfo.pScissors = nullptr;

	vk::PipelineRasterizationStateCreateInfo vkRasterizerInfo{};
	vkRasterizerInfo.sType = vk::StructureType::ePipelineRasterizationStateCreateInfo;
	vkRasterizerInfo.depthClampEnable = VK_FALSE;
	vkRasterizerInfo.rasterizerDiscardEnable = VK_FALSE;
	vkRasterizerInfo.polygonMode = vk::PolygonMode::eFill;
	vkRasterizerInfo.lineWidth = 1.0f;
	vkRasterizerInfo.cullMode = vk::CullModeFlagBits::eBack;
	vkRasterizerInfo.frontFace = vk::FrontFace::eCounterClockwise;
	vkRasterizerInfo.depthBiasEnable = VK_FALSE;
	vkRasterizerInfo.depthBiasConstantFactor = 0.0f;
	vkRasterizerInfo.depthBiasClamp = 0.0f;
	vkRasterizerInfo.depthBiasSlopeFactor = 0.0f;

	vk::PipelineMultisampleStateCreateInfo vkMultisamplingInfo{};
	vkMultisamplingInfo.sType = vk::StructureType::ePipelineMultisampleStateCreateInfo;
	vkMultisamplingInfo.sampleShadingEnable = VK_FALSE;
	vkMultisamplingInfo.rasterizationSamples = vk::SampleCountFlagBits::e1;
	vkMultisamplingInfo.minSampleShading = 1.0f;
	vkMultisamplingInfo.pSampleMask = nullptr;
	vkMultisamplingInfo.alphaToCoverageEnable = VK_FALSE;
	vkMultisamplingInfo.alphaToOneEnable = VK_FALSE;

	vk::PipelineDepthStencilStateCreateInfo vkDepthStencil{};
	vkDepthStencil.sType = vk::StructureType::ePipelineDepthStencilStateCreateInfo;
	vkDepthStencil.depthTestEnable = VK_TRUE;
	vkDepthStencil.depthWriteEnable = VK_TRUE;
	vkDepthStencil.depthCompareOp = vk::CompareOp::eLess;
	vkDepthStencil.depthBoundsTestEnable = VK_FALSE;
	vkDepthStencil.minDepthBounds = 0.0f;
	vkDepthStencil.maxDepthBounds = 1.0f;
	vkDepthStencil.stencilTestEnable = VK_FALSE;
	vkDepthStencil.front = vk::StencilOp::eKeep;
	vkDepthStencil.back = vk::StencilOp::eKeep;

	vk::PipelineColorBlendAttachmentState vkColorBlendAttachment{};
	vkColorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | 
											vk::ColorComponentFlagBits::eG |
											vk::ColorComponentFlagBits::eB |
											vk::ColorComponentFlagBits::eA;
	vkColorBlendAttachment.blendEnable = VK_FALSE;
	vkColorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eOne;
	vkColorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eZero;
	vkColorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;
	vkColorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
	vkColorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
	vkColorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd;
	vk::PipelineColorBlendStateCreateInfo vkColorBlendInfo{};
	vkColorBlendInfo.sType = vk::StructureType::ePipelineColorBlendStateCreateInfo;
	vkColorBlendInfo.logicOpEnable = VK_FALSE;
	vkColorBlendInfo.logicOp = vk::LogicOp::eCopy;
	vkColorBlendInfo.attachmentCount = 1;
	vkColorBlendInfo.pAttachments = &vkColorBlendAttachment;
	vkColorBlendInfo.blendConstants[0] = 0.0f;
	vkColorBlendInfo.blendConstants[1] = 0.0f;
	vkColorBlendInfo.blendConstants[2] = 0.0f;
	vkColorBlendInfo.blendConstants[3] = 0.0f;

	std::vector<vk::DynamicState> dynamicStates{
		vk::DynamicState::eViewport,
		vk::DynamicState::eScissor
	};
	vk::PipelineDynamicStateCreateInfo vkDynamicStateInfo{};
	vkDynamicStateInfo.sType = vk::StructureType::ePipelineDynamicStateCreateInfo;
	vkDynamicStateInfo.dynamicStateCount = static_cast<std::uint32_t>(dynamicStates.size());
	vkDynamicStateInfo.pDynamicStates = dynamicStates.data();

	vk::PipelineLayoutCreateInfo vkPipelineLayoutInfo{};
	vkPipelineLayoutInfo.sType = vk::StructureType::ePipelineLayoutCreateInfo;
	vkPipelineLayoutInfo.setLayoutCount = 1;
	vkPipelineLayoutInfo.pSetLayouts = &m_vkDescriptorSetLayout;
	vkPipelineLayoutInfo.pushConstantRangeCount = 0;
	vkPipelineLayoutInfo.pPushConstantRanges = nullptr;
	
	m_vkPipelineLayout = m_vkLogicalDevice.createPipelineLayout( vkPipelineLayoutInfo );

	LOG_INFO("Pipeline Layout created");

	vk::GraphicsPipelineCreateInfo vkGraphicsPipelineCreateInfo{};
	vkGraphicsPipelineCreateInfo.sType = vk::StructureType::eGraphicsPipelineCreateInfo;
	vkGraphicsPipelineCreateInfo.stageCount = 2;
	vkGraphicsPipelineCreateInfo.pStages = vkShaderStages;
	vkGraphicsPipelineCreateInfo.pVertexInputState = &vkVertexInputInfo;
	vkGraphicsPipelineCreateInfo.pInputAssemblyState = &vkInputAssemblyInfo;
	vkGraphicsPipelineCreateInfo.pViewportState = &vkViewportInfo;
	vkGraphicsPipelineCreateInfo.pRasterizationState = &vkRasterizerInfo;
	vkGraphicsPipelineCreateInfo.pMultisampleState = &vkMultisamplingInfo;
	vkGraphicsPipelineCreateInfo.pDepthStencilState = &vkDepthStencil;
	vkGraphicsPipelineCreateInfo.pColorBlendState = &vkColorBlendInfo;
	vkGraphicsPipelineCreateInfo.pDynamicState = &vkDynamicStateInfo;
	vkGraphicsPipelineCreateInfo.layout = m_vkPipelineLayout;
	vkGraphicsPipelineCreateInfo.renderPass = m_vkRenderPass;
	vkGraphicsPipelineCreateInfo.subpass = 0;
	vkGraphicsPipelineCreateInfo.basePipelineHandle = nullptr;
	vkGraphicsPipelineCreateInfo.basePipelineIndex = -1;

	vk::ResultValue<vk::Pipeline> operationResult = m_vkLogicalDevice.createGraphicsPipeline( nullptr, vkGraphicsPipelineCreateInfo );
	if( operationResult.result == vk::Result::eSuccess )
	{
		m_vkGraphicsPipeline = operationResult.value;
		LOG_INFO("Graphics Pipeline created");
	}
	else
	{
		std::string errorMsg = "Failed to create Graphics Pipeline";
		LOG_ERROR(errorMsg);
		throw std::runtime_error(errorMsg);
	}

	m_vkLogicalDevice.destroyShaderModule( fragmentShaderModule );
	m_vkLogicalDevice.destroyShaderModule( vertexShaderModule );
}

void VulkanApplication::createFrameBuffers()
{
	m_swapchainFrameBuffers.resize( m_swapchainImageViews.size() );

	for( size_t i = 0; i < m_swapchainFrameBuffers.size(); i++ )
	{
		std::array<vk::ImageView, 2> attachments = {
			m_swapchainImageViews[i],
			m_vkDepthImageView
		};

		vk::FramebufferCreateInfo vkFrameBufferInfo{};
		vkFrameBufferInfo.sType = vk::StructureType::eFramebufferCreateInfo;
		vkFrameBufferInfo.renderPass = m_vkRenderPass;
		vkFrameBufferInfo.attachmentCount = static_cast<std::uint32_t>( attachments.size() );
		vkFrameBufferInfo.pAttachments = attachments.data();
		vkFrameBufferInfo.width = m_vkSwapchainExtent.width;
		vkFrameBufferInfo.height = m_vkSwapchainExtent.height;
		vkFrameBufferInfo.layers = 1;

		vk::Framebuffer vkFrameBuffer = m_vkLogicalDevice.createFramebuffer( vkFrameBufferInfo );

		m_swapchainFrameBuffers[i] = vkFrameBuffer;
	}
	LOG_INFO( fmt::format("{} Framebuffer created", m_swapchainFrameBuffers.size() ) );
}

void VulkanApplication::createCommandPool()
{
	using namespace vkrender;

	QueueFamilyIndices queueFamilyIndices = findQueueFamilyIndices( m_vkPhysicalDevice, &m_vkSurface );

	vk::CommandPoolCreateInfo vkGraphicsCommandPoolInfo{};
	vkGraphicsCommandPoolInfo.sType = vk::StructureType::eCommandPoolCreateInfo;
	vkGraphicsCommandPoolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
	vkGraphicsCommandPoolInfo.queueFamilyIndex = queueFamilyIndices.m_graphicsFamily.value();

	vk::CommandPoolCreateInfo vkTransferCommandPoolInfo{};
	vkTransferCommandPoolInfo.sType = vk::StructureType::eCommandPoolCreateInfo;
	vkTransferCommandPoolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
	vkTransferCommandPoolInfo.queueFamilyIndex = queueFamilyIndices.m_exclusiveTransferFamily.value();

	m_vkGraphicsCommandPool = m_vkLogicalDevice.createCommandPool( vkGraphicsCommandPoolInfo );
	LOG_INFO("Graphics Command Pool created");

	m_vkTransferCommandPool = m_vkLogicalDevice.createCommandPool( vkTransferCommandPoolInfo );
	LOG_INFO("Transfer Command Pool created");
}

void VulkanApplication::createConfigCommandBuffer()
{
	vk::CommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = vk::StructureType::eCommandBufferAllocateInfo;
	allocInfo.commandPool = m_vkGraphicsCommandPool;
	allocInfo.level = vk::CommandBufferLevel::ePrimary;
	allocInfo.commandBufferCount = 1;

	m_vkConfigCommandBuffer = m_vkLogicalDevice.allocateCommandBuffers(allocInfo)[0];

	LOG_INFO("Config Command Buffer created");
}

void VulkanApplication::createDepthResources()
{
	vk::Format depthFormat = findDepthFormat();

	createImage(
		m_vkSwapchainExtent.width, m_vkSwapchainExtent.height,
		depthFormat, vk::ImageTiling::eOptimal, 
		vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal,
		m_vkDepthImage, m_vkDepthImageMemory
	);
	m_vkDepthImageView = createImageView( m_vkDepthImage, depthFormat, vk::ImageAspectFlagBits::eDepth );

	transitionImageLayout( m_vkDepthImage, depthFormat, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal );

	LOG_INFO("Depth Resources Created");
}

void VulkanApplication::createTextureImage()
{
	int texWidth, texHeight, texChannels;
	
	std::filesystem::path def_texPath{"textures/texture.jpg"};

	if( !std::filesystem::exists(m_textureImageFilePath) )
		m_textureImageFilePath = def_texPath;

	stbi_uc* pixels = stbi_load(
		m_textureImageFilePath.string().data(),
		&texWidth, &texHeight, &texChannels,
		STBI_rgb_alpha
	);

	if(!pixels)
	{
		std::string errorMsg = fmt::format("Failed to load {} image",m_textureImageFilePath.string());
		LOG_ERROR(errorMsg);
		throw  std::runtime_error(errorMsg);
	}

	vk::DeviceSize imageSize = texWidth * texHeight * 4;

	vk::Buffer stagingBuffer;
	vk::DeviceMemory stagingBufferMemory;

	createBuffer( 
		imageSize, 
		vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
		stagingBuffer,
		stagingBufferMemory
	);


	void* bufferMappedData = m_vkLogicalDevice.mapMemory( stagingBufferMemory, 0, imageSize );
	std::memcpy(bufferMappedData, pixels, static_cast<std::size_t>(imageSize) );
	m_vkLogicalDevice.unmapMemory(stagingBufferMemory);

	stbi_image_free(pixels);

	createImage( 
		texWidth, texHeight,
		vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
		vk::MemoryPropertyFlagBits::eDeviceLocal,
		m_vkTextureImage, m_vkTextureImageMemory
	);

	transitionImageLayout( 
		m_vkTextureImage, vk::Format::eR8G8B8A8Srgb, 
		vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal 
	);

	copyBufferToImage( 
		stagingBuffer, m_vkTextureImage, 
		static_cast<std::uint32_t>( texWidth ), 
		static_cast<std::uint32_t>( texHeight )
	);

	transitionImageLayout(
		m_vkTextureImage, vk::Format::eR8G8B8A8Srgb,
		vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal
	);

	m_vkLogicalDevice.destroyBuffer( stagingBuffer, nullptr );
	m_vkLogicalDevice.freeMemory( stagingBufferMemory, nullptr );
}

void VulkanApplication::createTextureImageView()
{
	m_vkTextureImageView = createImageView( m_vkTextureImage, vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor );
}

void VulkanApplication::createTextureSampler()
{
	vk::PhysicalDeviceProperties phyDeviceProp = m_vkPhysicalDevice.getProperties(); 

	vk::SamplerCreateInfo samplerCreateInfo{};
	samplerCreateInfo.sType = vk::StructureType::eSamplerCreateInfo;
	samplerCreateInfo.magFilter = vk::Filter::eLinear;
	samplerCreateInfo.minFilter = vk::Filter::eLinear;
	samplerCreateInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
	samplerCreateInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
	samplerCreateInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
	samplerCreateInfo.anisotropyEnable = VK_TRUE;
	samplerCreateInfo.maxAnisotropy = phyDeviceProp.limits.maxSamplerAnisotropy;
	samplerCreateInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
	samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
	samplerCreateInfo.compareEnable = VK_FALSE;
	samplerCreateInfo.compareOp = vk::CompareOp::eAlways;
	samplerCreateInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
	samplerCreateInfo.mipLodBias = 0.0f;
	samplerCreateInfo.minLod = 0.0f;
	samplerCreateInfo.maxLod = 0.0f;

	m_vkTextureSampler = m_vkLogicalDevice.createSampler( samplerCreateInfo );
}

void VulkanApplication::createGraphicsCommandBuffers()
{
	m_vkGraphicsCommandBuffers.resize( MAX_FRAMES_IN_FLIGHT );

	vk::CommandBufferAllocateInfo vkCmdBufAllocateInfo{};
	vkCmdBufAllocateInfo.sType = vk::StructureType::eCommandBufferAllocateInfo;
	vkCmdBufAllocateInfo.commandPool = m_vkGraphicsCommandPool;
	vkCmdBufAllocateInfo.level = vk::CommandBufferLevel::ePrimary;
	vkCmdBufAllocateInfo.commandBufferCount = m_vkGraphicsCommandBuffers.size();
	
	m_vkGraphicsCommandBuffers = m_vkLogicalDevice.allocateCommandBuffers( vkCmdBufAllocateInfo );

	LOG_INFO("Graphics Command Buffer created");
}

void VulkanApplication::loadModel()
{
	tinyobj::attrib_t attributes;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string errorMsg;

	if( !tinyobj::LoadObj(
			&attributes,
			&shapes,
			&materials,
			&errorMsg,
			m_modelFilePath.string().c_str()
		)
	)
	{
		LOG_ERROR(errorMsg);
		throw std::runtime_error(errorMsg);
	}

	std::unordered_map<vertex, std::uint32_t> unique_vertices;

	for( const auto& shape : shapes )
	{
		for( const auto& index : shape.mesh.indices )
		{
			vertex vertexData{};
			
			vertexData.pos = { 
				attributes.vertices[ 3 * index.vertex_index + 0 ],
				attributes.vertices[ 3 * index.vertex_index + 1 ],
				attributes.vertices[ 3 * index.vertex_index + 2 ]
			};
			
			vertexData.texCoord = { 
				attributes.texcoords[ 2 * index.texcoord_index + 0 ],
				1.0f - attributes.texcoords[ 2 * index.texcoord_index + 1 ]
			};

			vertexData.color = { 1.0, 1.0, 1.0 };
			
			if( unique_vertices.count(vertexData) == 0 )
			{
				unique_vertices[vertexData] = static_cast<std::uint32_t>( m_inputVertexData.size() );
				m_inputVertexData.push_back( vertexData );
			}
			m_inputIndexData.push_back( unique_vertices[vertexData] );
		}
	}
}

void VulkanApplication::createVertexBuffer()
{

	std::size_t bufferSizeInBytes = sizeof(vertex) * m_inputVertexData.size();

	vk::Buffer stagingBuffer;
	vk::DeviceMemory stagingBufferMemory;

	createBuffer( 
		static_cast<vk::DeviceSize>( bufferSizeInBytes ),
		vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
		stagingBuffer,
		stagingBufferMemory
	);

	void* mappedMemory = m_vkLogicalDevice.mapMemory(
		stagingBufferMemory,
		0, bufferSizeInBytes
	);
	std::memcpy( 
		mappedMemory, 
		m_inputVertexData.data(),
		bufferSizeInBytes
	);
	m_vkLogicalDevice.unmapMemory( stagingBufferMemory );

	createBuffer(
		bufferSizeInBytes,
		vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
		vk::MemoryPropertyFlagBits::eDeviceLocal,
		m_vkVertexBuffer,
		m_vkVertexBufferMemory
	);

	copyBuffer( stagingBuffer, m_vkVertexBuffer, bufferSizeInBytes );

	m_vkLogicalDevice.destroyBuffer( stagingBuffer );
	m_vkLogicalDevice.freeMemory( stagingBufferMemory );
}

void VulkanApplication::createIndexBuffer()
{
	std::size_t bufferSizeInBytes = sizeof(IndexData::value_type) * m_inputIndexData.size();

	vk::Buffer stagingBuffer;
	vk::DeviceMemory stagingBufferMemory;

	createBuffer(
		bufferSizeInBytes,
		vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
		stagingBuffer,
		stagingBufferMemory
	);

	void* mappedMemory = m_vkLogicalDevice.mapMemory(
		stagingBufferMemory,
		0, 
		static_cast<vk::DeviceSize>( bufferSizeInBytes )
	);
	std::memcpy( mappedMemory, m_inputIndexData.data(), bufferSizeInBytes );
	m_vkLogicalDevice.unmapMemory( stagingBufferMemory );

	createBuffer(
		bufferSizeInBytes,
		vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
		vk::MemoryPropertyFlagBits::eDeviceLocal,
		m_vkIndexBuffer,
		m_vkIndexBufferMemory
	);

	copyBuffer( stagingBuffer, m_vkIndexBuffer, bufferSizeInBytes );

	m_vkLogicalDevice.destroyBuffer( stagingBuffer );
	m_vkLogicalDevice.freeMemory( stagingBufferMemory );
}

void VulkanApplication::createUniformBuffers()
{
	vk::DeviceSize bufferSize = sizeof(VulkanUniformBufferObject);

	m_vkUniformBuffers.resize( MAX_FRAMES_IN_FLIGHT );
	m_vkUniformBuffersMemory.resize( MAX_FRAMES_IN_FLIGHT );
	m_uniformBuffersMapped.resize( MAX_FRAMES_IN_FLIGHT );

	for( std::size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ )
	{
		createBuffer(
			bufferSize, vk::BufferUsageFlagBits::eUniformBuffer,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
			m_vkUniformBuffers[i], m_vkUniformBuffersMemory[i]
		);

		m_uniformBuffersMapped[i] = m_vkLogicalDevice.mapMemory( m_vkUniformBuffersMemory[i], 0, bufferSize );
	}
}

void VulkanApplication::createSyncObjects()
{
	m_vkImageAvailableSemaphores.resize( MAX_FRAMES_IN_FLIGHT );
	m_vkRenderFinishedSemaphores.resize( MAX_FRAMES_IN_FLIGHT );
	m_vkInFlightFences.resize( MAX_FRAMES_IN_FLIGHT );

	vk::SemaphoreCreateInfo vkSemaphoreInfo{};
	vkSemaphoreInfo.sType = vk::StructureType::eSemaphoreCreateInfo;

	vk::FenceCreateInfo vkFenceInfo{};
	vkFenceInfo.sType = vk::StructureType::eFenceCreateInfo;
	vkFenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;

	for( auto i = 0u; i < MAX_FRAMES_IN_FLIGHT; i++ )
	{
		m_vkImageAvailableSemaphores[i] = m_vkLogicalDevice.createSemaphore( vkSemaphoreInfo );
		m_vkRenderFinishedSemaphores[i] = m_vkLogicalDevice.createSemaphore( vkSemaphoreInfo );

		m_vkInFlightFences[i] = m_vkLogicalDevice.createFence( vkFenceInfo );
	}

	LOG_INFO("Sync Objects For Rendering and Presentation created");
}

void VulkanApplication::recreateSwapChain()
{
	m_window.getFrameBufferSize();
	m_vkLogicalDevice.waitIdle();

	destroySwapChain();
	
	createSwapchain();
	createSwapChainImageViews();
	createDepthResources();
	createFrameBuffers();	
}

void VulkanApplication::destroySwapChain()
{
	m_vkLogicalDevice.destroyImageView( m_vkDepthImageView );
	m_vkLogicalDevice.destroyImage( m_vkDepthImage );
	m_vkLogicalDevice.freeMemory( m_vkDepthImageMemory );

	for( auto& vkFramebuffer : m_swapchainFrameBuffers )
	{
		m_vkLogicalDevice.destroyFramebuffer( vkFramebuffer );
	}

	for( auto& vkImageView : m_swapchainImageViews )
	{
		m_vkLogicalDevice.destroyImageView( vkImageView );
	}
	m_swapchainFrameBuffers.clear();
	m_swapchainImageViews.clear();

	m_vkLogicalDevice.destroySwapchainKHR( m_vkSwapchain );
}

void VulkanApplication::setupConfigCommandBuffer()
{
	m_vkConfigCommandBuffer.reset();

	vk::CommandBufferBeginInfo beginInfo{};
	beginInfo.sType = vk::StructureType::eCommandBufferBeginInfo;
	beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
	
	m_vkConfigCommandBuffer.begin( beginInfo );
}

void VulkanApplication::flushConfigCommandBuffer()
{
	m_vkConfigCommandBuffer.end();

	vk::SubmitInfo submitInfo{};
	submitInfo.sType = vk::StructureType::eSubmitInfo;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_vkConfigCommandBuffer;

	vk::Result opResult = m_vkGraphicsQueue.submit( 1, &submitInfo, {} );
	m_vkGraphicsQueue.waitIdle();
}


void VulkanApplication::recordCommandBuffer( vk::CommandBuffer& vkCommandBuffer, const std::uint32_t& imageIndex )
{
	vk::CommandBufferBeginInfo vkCmdBufBeginInfo{};
	vkCmdBufBeginInfo.sType = vk::StructureType::eCommandBufferBeginInfo;
	vkCmdBufBeginInfo.flags = {};
	vkCmdBufBeginInfo.pInheritanceInfo = nullptr;

	vkCommandBuffer.begin( vkCmdBufBeginInfo );

	vk::RenderPassBeginInfo vkRenderPassBeginInfo{};
	vkRenderPassBeginInfo.sType = vk::StructureType::eRenderPassBeginInfo;
	vkRenderPassBeginInfo.renderPass = m_vkRenderPass;
	vkRenderPassBeginInfo.framebuffer = m_swapchainFrameBuffers[ imageIndex ];
	vkRenderPassBeginInfo.renderArea.offset = vk::Offset2D{ 0, 0 };
	vkRenderPassBeginInfo.renderArea.extent = m_vkSwapchainExtent;
	std::array<vk::ClearValue, 2> clearValues{};
	vk::ClearColorValue clearColorValue;
	clearColorValue.setFloat32( {0.0f, 0.0f, 0.0f, 1.0f} );
	clearValues[0].setColor( clearColorValue );
	vk::ClearDepthStencilValue clearDepthStencilValue{ 1.0f, 0 };
	clearValues[1].setDepthStencil(clearDepthStencilValue);
	vkRenderPassBeginInfo.clearValueCount = static_cast<std::uint32_t>( clearValues.size() );
	vkRenderPassBeginInfo.pClearValues = clearValues.data();

	vkCommandBuffer.beginRenderPass( vkRenderPassBeginInfo, vk::SubpassContents::eInline );
	vkCommandBuffer.bindPipeline( vk::PipelineBindPoint::eGraphics, m_vkGraphicsPipeline );
	
	vk::Viewport vkViewport{};
	vkViewport.x = 0.0f;
	vkViewport.y = 0.0f;
	vkViewport.width = static_cast<float>(m_vkSwapchainExtent.width);
	vkViewport.height = static_cast<float>(m_vkSwapchainExtent.height);
	vkViewport.minDepth = 0.0f;
	vkViewport.maxDepth = 1.0f;
	vkCommandBuffer.setViewport(0, 1, &vkViewport);

	vk::Rect2D vkScissor{};
	vkScissor.offset = vk::Offset2D{ 0, 0 };
	vkScissor.extent = m_vkSwapchainExtent;
	vkCommandBuffer.setScissor(0, 1, &vkScissor);

	vk::Buffer vertexBuffers[] = { m_vkVertexBuffer };
	vk::DeviceSize offsets[] = { 0 };

	vkCommandBuffer.bindVertexBuffers( 0, vertexBuffers, offsets );
	vkCommandBuffer.bindIndexBuffer( m_vkIndexBuffer, 0, vk::IndexType::eUint32 );
	vkCommandBuffer.bindDescriptorSets( 
		vk::PipelineBindPoint::eGraphics, m_vkPipelineLayout, 
		0, 1, &m_vkDescriptorSets[m_currentFrame],
		0, nullptr
	);
	vkCommandBuffer.drawIndexed(
		static_cast<std::uint32_t>( m_inputIndexData.size() ),
		1,
		0,
		0,
		0
	);

	vkCommandBuffer.endRenderPass();
	vkCommandBuffer.end();
}

vk::CommandBuffer VulkanApplication::beginSingleTimeCommands( const vk::CommandPool& commandPoolToAllocFrom )
{
	vk::CommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = vk::StructureType::eCommandBufferAllocateInfo;
	allocInfo.level = vk::CommandBufferLevel::ePrimary;
	allocInfo.commandPool = commandPoolToAllocFrom;
	allocInfo.commandBufferCount = 1;

	vk::CommandBuffer commandBuffer = m_vkLogicalDevice.allocateCommandBuffers(allocInfo)[0];

	vk::CommandBufferBeginInfo beginInfo{};
	beginInfo.sType = vk::StructureType::eCommandBufferBeginInfo;
	beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

	commandBuffer.begin( beginInfo );

	return commandBuffer;
}

void VulkanApplication::endSingleTimeCommands( const vk::CommandPool& commandPoolAllocFrom, vk::CommandBuffer vkCommandBuffer, vk::Queue queueToSubmitOn )
{
	vkCommandBuffer.end();

	vk::SubmitInfo submitInfo{};
	submitInfo.sType = vk::StructureType::eSubmitInfo;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &vkCommandBuffer;

	vk::Result opResult = queueToSubmitOn.submit( 1, &submitInfo, nullptr );
	queueToSubmitOn.waitIdle();

	m_vkLogicalDevice.freeCommandBuffers( commandPoolAllocFrom, 1, &vkCommandBuffer );
}

void VulkanApplication::transitionImageLayout( 
    const vk::Image& image, const vk::Format& format, 
    const vk::ImageLayout& oldLayout, const vk::ImageLayout& newLayout
)
{
	setupConfigCommandBuffer();

	vk::AccessFlags srcAccessMask;
	vk::AccessFlags dstAccessMask;

	vk::PipelineStageFlags srcStage;
	vk::PipelineStageFlags dstStage;
	
	vk::ImageAspectFlags aspectMask;

	if( newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal )
	{
		aspectMask = vk::ImageAspectFlagBits::eDepth;

		if( hasStencilComponent(format) )
			aspectMask |= vk::ImageAspectFlagBits::eStencil;
	}
	else
	{
		aspectMask = vk::ImageAspectFlagBits::eColor;
	}

	if( oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal )
	{
		srcAccessMask = {};
		dstAccessMask = vk::AccessFlagBits::eTransferWrite;

		srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
		dstStage = vk::PipelineStageFlagBits::eTransfer;
	}
	else if( oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal )
	{
		srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		dstAccessMask = vk::AccessFlagBits::eShaderRead;

		srcStage = vk::PipelineStageFlagBits::eTransfer;
		dstStage = vk::PipelineStageFlagBits::eFragmentShader;
	}
	else if( oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal )
	{
		srcAccessMask = {};
		dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

		srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
		dstStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
	}
	else 
	{
		std::string errorMsg = "unsupported image layout transition!";
		LOG_ERROR(errorMsg);
		throw  std::invalid_argument(errorMsg);
	}

	vk::ImageMemoryBarrier imgBarrier{};
	imgBarrier.sType = vk::StructureType::eImageMemoryBarrier;
	imgBarrier.oldLayout = oldLayout;
	imgBarrier.newLayout = newLayout;
	imgBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imgBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imgBarrier.image = image;
	imgBarrier.subresourceRange.aspectMask = aspectMask;
	imgBarrier.subresourceRange.baseMipLevel = 0;
	imgBarrier.subresourceRange.levelCount = 1;
	imgBarrier.subresourceRange.baseArrayLayer = 0;
	imgBarrier.subresourceRange.layerCount = 1;
	imgBarrier.srcAccessMask = srcAccessMask;
	imgBarrier.dstAccessMask = dstAccessMask;

	m_vkConfigCommandBuffer.pipelineBarrier( 
		srcStage, dstStage,
		{},
		0, nullptr,
		0, nullptr,
		1, &imgBarrier
	);

	flushConfigCommandBuffer();
}

vk::ShaderModule VulkanApplication::createShaderModule(const std::vector<char>& shaderSourceBuffer)
{
	vk::ShaderModuleCreateInfo vkShaderModuleCreateInfo{};
	vkShaderModuleCreateInfo.sType = vk::StructureType::eShaderModuleCreateInfo;
	vkShaderModuleCreateInfo.codeSize = shaderSourceBuffer.size();
	vkShaderModuleCreateInfo.pCode = reinterpret_cast<const std::uint32_t*>( shaderSourceBuffer.data() );

	vk::ShaderModule vkShaderModule = m_vkLogicalDevice.createShaderModule( vkShaderModuleCreateInfo );

	return vkShaderModule;
}

void VulkanApplication::createBuffer(
    const vk::DeviceSize& bufferSizeInBytes,
    const vk::BufferUsageFlags& bufferUsage,
    const vk::MemoryPropertyFlags& memProps,
    vk::Buffer& buffer,
    vk::DeviceMemory& bufferMemory
)
{
	vkrender::QueueFamilyIndices queueFamilyIndices = findQueueFamilyIndices( 
		m_vkPhysicalDevice,
		&m_vkSurface
	);

	vk::BufferCreateInfo bufferInfo{};
	bufferInfo.sType = vk::StructureType::eBufferCreateInfo;
	bufferInfo.size = bufferSizeInBytes;
	bufferInfo.usage = bufferUsage;
	bufferInfo.sharingMode = vk::SharingMode::eConcurrent;
	std::uint32_t queueFamilyToShare[] = { 
		queueFamilyIndices.m_graphicsFamily.value(),
		queueFamilyIndices.m_exclusiveTransferFamily.value()
	};
	bufferInfo.pQueueFamilyIndices = queueFamilyToShare;
	bufferInfo.queueFamilyIndexCount = 2;

	buffer = m_vkLogicalDevice.createBuffer(
		bufferInfo
	);

	vk::MemoryRequirements memRequirements = m_vkLogicalDevice.getBufferMemoryRequirements( buffer );

	vk::MemoryAllocateInfo allocInfo{};
	allocInfo.sType = vk::StructureType::eMemoryAllocateInfo;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType( memRequirements.memoryTypeBits, memProps );

	bufferMemory = m_vkLogicalDevice.allocateMemory( allocInfo );

	m_vkLogicalDevice.bindBufferMemory( buffer, bufferMemory, 0 );
}

void VulkanApplication::createImage(
    const std::uint32_t& width, const std::uint32_t& height,
    const vk::Format& format, const vk::ImageTiling& tiling,
    const vk::ImageUsageFlags& usageFlags, const vk::MemoryPropertyFlags& memPropFlags,
    vk::Image& image, vk::DeviceMemory& imageMemory
)
{
	vk::ImageCreateInfo imageCreateInfo{};
	imageCreateInfo.sType = vk::StructureType::eImageCreateInfo;
	imageCreateInfo.imageType = vk::ImageType::e2D;
	imageCreateInfo.extent.width = static_cast<std::uint32_t>( width );
	imageCreateInfo.extent.height = static_cast<std::uint32_t>( height );
	imageCreateInfo.extent.depth = 1;
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.format = format;
	imageCreateInfo.tiling = tiling;
	imageCreateInfo.initialLayout = vk::ImageLayout::eUndefined;
	imageCreateInfo.usage = usageFlags;
	imageCreateInfo.sharingMode = vk::SharingMode::eExclusive;
	imageCreateInfo.samples = vk::SampleCountFlagBits::e1;
	imageCreateInfo.flags = {};

	image = m_vkLogicalDevice.createImage( imageCreateInfo );

	vk::MemoryRequirements memRequirements = m_vkLogicalDevice.getImageMemoryRequirements( image );
	
	vk::MemoryAllocateInfo allocInfo{};
	allocInfo.sType = vk::StructureType::eMemoryAllocateInfo;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType( memRequirements.memoryTypeBits, memPropFlags );

	imageMemory = m_vkLogicalDevice.allocateMemory( allocInfo );

	m_vkLogicalDevice.bindImageMemory( image, imageMemory, 0 );
}

vk::ImageView VulkanApplication::createImageView( const vk::Image& image, const vk::Format& format, const vk::ImageAspectFlags& aspect )
{
	vk::ImageViewCreateInfo vkImageViewCreateInfo{};
	vkImageViewCreateInfo.sType = vk::StructureType::eImageViewCreateInfo;
	vkImageViewCreateInfo.image = image;
	vkImageViewCreateInfo.viewType = vk::ImageViewType::e2D;
    vkImageViewCreateInfo.format = format;
    vk::ComponentMapping componentMap;
    componentMap.r = vk::ComponentSwizzle::eIdentity;
    componentMap.g = vk::ComponentSwizzle::eIdentity;
    componentMap.b = vk::ComponentSwizzle::eIdentity;
    componentMap.a = vk::ComponentSwizzle::eIdentity;
    vkImageViewCreateInfo.components = componentMap;
    vk::ImageSubresourceRange subResourceRange;
    subResourceRange.aspectMask = aspect;
    subResourceRange.baseMipLevel = 0;
    subResourceRange.levelCount = 1;
    subResourceRange.baseArrayLayer = 0;
    subResourceRange.layerCount = 1;
    vkImageViewCreateInfo.subresourceRange = subResourceRange;

	return m_vkLogicalDevice.createImageView( vkImageViewCreateInfo );
}

void VulkanApplication::populateShaderBufferFromSourceFile( const std::filesystem::path& filePath, std::vector<char>& shaderSourceBuffer )
{
	if( filePath.extension() != std::filesystem::path{ ".spv" } )
    {
        std::string errorMsg = "ILLEGAL SHADER FILE FORMAT!";
        LOG_ERROR(errorMsg);
        throw std::runtime_error(errorMsg);
    }

    std::ifstream fstream( filePath.string(), std::ios::ate | std::ios::binary );

    std::size_t fileSize = static_cast<std::size_t>( fstream.tellg() );
    shaderSourceBuffer.resize( fileSize );

    fstream.seekg(0);
    fstream.read( shaderSourceBuffer.data(), fileSize );

    fstream.close();
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

void VulkanApplication::populateDebugUtilsMessengerCreateInfo( vk::DebugUtilsMessengerCreateInfoEXT& vkDebugUtilsMessengerCreateInfo )
{
	using namespace vkrender;

	vkDebugUtilsMessengerCreateInfo.sType = vk::StructureType::eDebugUtilsMessengerCreateInfoEXT;
	vkDebugUtilsMessengerCreateInfo.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
	vkDebugUtilsMessengerCreateInfo.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
	vkDebugUtilsMessengerCreateInfo.pfnUserCallback = VulkanDebugMessenger::debugCallback;
}

void VulkanApplication::populateDeviceQueueCreateInfo( vk::DeviceQueueCreateInfo& vkDeviceQueueCreateInfo, const std::uint32_t& queueFamilyIndex, const std::vector<float>& queuePriorities )
{
	vkDeviceQueueCreateInfo.sType = vk::StructureType::eDeviceQueueCreateInfo;
	vkDeviceQueueCreateInfo.queueFamilyIndex = queueFamilyIndex;
	vkDeviceQueueCreateInfo.queueCount = queuePriorities.size();
	vkDeviceQueueCreateInfo.pQueuePriorities = queuePriorities.data();
}

void VulkanApplication::populateDeviceCreateInfo( 
	vk::DeviceCreateInfo& vkDeviceCreateInfo, 
	const std::vector<vk::DeviceQueueCreateInfo>& queueCreateInfos, 
	const vk::PhysicalDeviceFeatures* pEnabledFeatures 
)
{
	using namespace vkrender;

	vkDeviceCreateInfo.sType = vk::StructureType::eDeviceCreateInfo;
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

vkrender::SwapChainSupportDetails VulkanApplication::querySwapChainSupport( const vk::PhysicalDevice& vkPhysicalDevice, const vk::SurfaceKHR& vkSurface )
{
    vkrender::SwapChainSupportDetails swapChainDetails;

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

vk::ResultValue<std::uint32_t> VulkanApplication::swapchainNextImageWrapper(
	const vk::Device& logicalDevice,
    const vk::SwapchainKHR& swapchain,
    std::uint64_t timeout,
    vk::Semaphore imageAcquireSemaphore,
    vk::Fence imageAcquireFence
)
{
	std::uint32_t imageIndex;

	vk::DispatchLoaderStatic& dispatcher = vk::getDispatchLoaderStatic();
	vk::Result result = static_cast<vk::Result>( dispatcher.vkAcquireNextImageKHR(
		static_cast<VkDevice>( logicalDevice ),
		static_cast<VkSwapchainKHR>( swapchain ),
		timeout, 
		static_cast<VkSemaphore>( imageAcquireSemaphore ),
		static_cast<VkFence>( imageAcquireFence ),
		&imageIndex
	) );

	return vk::ResultValue<std::uint32_t>( result, imageIndex );
}

vk::Result VulkanApplication::queuePresentWrapper(
    const vk::Queue& presentationQueue,
    const vk::PresentInfoKHR& presentInfo
)
{
	vk::DispatchLoaderStatic& dispatcher = vk::getDispatchLoaderStatic();
	return static_cast<vk::Result>( dispatcher.vkQueuePresentKHR(
		presentationQueue, reinterpret_cast<const VkPresentInfoKHR*>(&presentInfo)
	) );
}

vk::Format VulkanApplication::findSupportedImgFormat( const std::initializer_list<vk::Format>& candidates, const vk::ImageTiling& tiling, const vk::FormatFeatureFlags& features )
{
	for( const vk::Format& format : candidates )
	{
		vk::FormatProperties prop = m_vkPhysicalDevice.getFormatProperties( format );

		if( tiling == vk::ImageTiling::eLinear && (prop.linearTilingFeatures & features) == features )
		{
			return format;
		}
		else if ( tiling == vk::ImageTiling::eOptimal && (prop.optimalTilingFeatures & features) == features )
		{
			return format;
		}
	}

	std::string errorMsg{ "failed to find supported format"};
	LOG_ERROR(errorMsg);
	throw std::runtime_error(errorMsg);
}

vk::Format VulkanApplication::findDepthFormat()
{
	return findSupportedImgFormat(
		{ vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint },
		vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment
	);
}

bool VulkanApplication::hasStencilComponent( const vk::Format& format ) const
{
	return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
}

std::uint32_t VulkanApplication::findMemoryType( const std::uint32_t& typeFilter, const vk::MemoryPropertyFlags& propertyFlags )
{
	vk::PhysicalDeviceMemoryProperties memoryProps = m_vkPhysicalDevice.getMemoryProperties();

	for( auto i = 0u; i < memoryProps.memoryTypeCount; i++ )
	{
		if( 
			( typeFilter & ( 1u << i ) ) &&
			( memoryProps.memoryTypes[i].propertyFlags & propertyFlags ) == propertyFlags 
		)
		{
			return i;
		}
	}

	// TODO throw exception
}

void VulkanApplication::copyBuffer( const vk::Buffer& srcBuffer, const vk::Buffer& dstBuffer, const vk::DeviceSize& sizeInBytes )
{
	vk::CommandBuffer transferCmdBuf = beginSingleTimeCommands( m_vkTransferCommandPool );

	vk::BufferCopy copyRegion{};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = sizeInBytes;

	vk::ArrayProxy<const vk::BufferCopy> copyRegionArray{
		copyRegion
	};

	transferCmdBuf.copyBuffer( srcBuffer, dstBuffer, copyRegionArray );

	endSingleTimeCommands( m_vkTransferCommandPool, transferCmdBuf, m_vkTransferQueue );
}

void VulkanApplication::copyBufferToImage( const vk::Buffer& srcBuffer, const vk::Image& dstImage, const std::uint32_t& width, const std::uint32_t& height )
{
	vk::CommandBuffer cmdBuf = beginSingleTimeCommands( m_vkTransferCommandPool );

	vk::BufferImageCopy copyRegion{};
	copyRegion.bufferOffset = 0;
	copyRegion.bufferRowLength = 0;
	copyRegion.bufferImageHeight = 0;

	copyRegion.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
	copyRegion.imageSubresource.mipLevel = 0;
	copyRegion.imageSubresource.baseArrayLayer = 0;
	copyRegion.imageSubresource.layerCount = 1;

	copyRegion.imageOffset = vk::Offset3D{ 0, 0, 0 };
	copyRegion.imageExtent = vk::Extent3D{ width, height, 1 };

	cmdBuf.copyBufferToImage( srcBuffer, dstImage, vk::ImageLayout::eTransferDstOptimal, 1, &copyRegion );

	endSingleTimeCommands(m_vkTransferCommandPool, cmdBuf, m_vkTransferQueue );
}