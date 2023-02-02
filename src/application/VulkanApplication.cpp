#include "application/VulkanApplication.h"
#include "vkrenderer/VulkanLayer.hpp"
#include "vkrenderer/VulkanDebugMessenger.h"
#include "vkrenderer/VulkanSwapChainFactory.h"
#include "utilities/VulkanLogger.h"

#include <vulkan/vulkan.hpp>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <set>
#include <fstream>

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
	createImageViews();
	createRenderPass();
	createGraphicsPipeline();
	createFrameBuffers();
	createCommandPool();
	createCommandBuffers();
	createSyncObjects();
}

void VulkanApplication::mainLoop()
{
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
	vk::ResultValue<std::uint32_t> opImageAcquistion = m_vkLogicalDevice.acquireNextImageKHR( 
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

	vk::Result opPresentResult = m_vkPresentationQueue.presentKHR( vkPresentInfo );
	
	if( opPresentResult == vk::Result::eErrorOutOfDateKHR || opPresentResult == vk::Result::eSuboptimalKHR )
	{
		recreateSwapChain();
	}
	else if( opPresentResult != vk::Result::eSuccess )
	{
		std::string errorMsg = "FAILED TO PRESENT SWAPCHAIN IMAGE";
		LOG_ERROR(errorMsg);
		throw std::runtime_error( errorMsg );
	}

	m_currentFrame = m_currentFrame % MAX_FRAMES_IN_FLIGHT;
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

	m_vkLogicalDevice.destroyCommandPool( m_vkGraphicsCommandPool );

	destroySwapChain();

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

	vk::DeviceCreateInfo vkDeviceCreateInfo{};
	std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
	queueCreateInfos.push_back( vkDeviceGraphicsQueueCreateInfo );
	if( queueFamilyIndices.m_presentFamily.has_value() )
		queueCreateInfos.push_back( vkDevicePresentationQueueCreateInfo );
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

void VulkanApplication::createImageViews()
{
	m_swapchainImageViews.resize( m_swapchainImages.size() );

	for( auto i = 0u; i < m_swapchainImages.size(); i++ )
	{
		vk::ImageViewCreateInfo vkImageViewCreateInfo{};
		vkImageViewCreateInfo.sType = vk::StructureType::eImageViewCreateInfo;
		vkImageViewCreateInfo.image = m_swapchainImages[i];
		vkImageViewCreateInfo.viewType = vk::ImageViewType::e2D;
        vkImageViewCreateInfo.format = m_vkSwapchainImageFormat;
        vk::ComponentMapping componentMap;
        componentMap.r = vk::ComponentSwizzle::eIdentity;
        componentMap.g = vk::ComponentSwizzle::eIdentity;
        componentMap.b = vk::ComponentSwizzle::eIdentity;
        componentMap.a = vk::ComponentSwizzle::eIdentity;
        vkImageViewCreateInfo.components = componentMap;
        vk::ImageSubresourceRange subResourceRange;
        subResourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        subResourceRange.baseMipLevel = 0;
        subResourceRange.levelCount = 1;
        subResourceRange.baseArrayLayer = 0;
        subResourceRange.layerCount = 1;
        vkImageViewCreateInfo.subresourceRange = subResourceRange;

		vk::ImageView vkImageView = m_vkLogicalDevice.createImageView( vkImageViewCreateInfo );
		m_swapchainImageViews[i] = vkImageView;
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

	vk::SubpassDescription vkSubPassDesc{};
	vkSubPassDesc.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	vkSubPassDesc.colorAttachmentCount = 1;
	vkSubPassDesc.pColorAttachments = &vkColorAttachmentRef;

	vk::SubpassDependency vkSubpassDependency{};
	vkSubpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	vkSubpassDependency.dstSubpass = 0;
	vkSubpassDependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	vkSubpassDependency.srcAccessMask = vk::AccessFlagBits::eNone;
	vkSubpassDependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	vkSubpassDependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

	vk::RenderPassCreateInfo vkRenderPassInfo{};
	vkRenderPassInfo.sType = vk::StructureType::eRenderPassCreateInfo;
	vkRenderPassInfo.attachmentCount = 1;
	vkRenderPassInfo.pAttachments = &vkColorAttachment;
	vkRenderPassInfo.subpassCount = 1;
	vkRenderPassInfo.pSubpasses = &vkSubPassDesc;
	vkRenderPassInfo.dependencyCount = 1;
	vkRenderPassInfo.pDependencies = &vkSubpassDependency;

	m_vkRenderPass = m_vkLogicalDevice.createRenderPass( vkRenderPassInfo );

	LOG_INFO("RenderPass created");
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
	vkVertexInputInfo.vertexBindingDescriptionCount = 0;
	vkVertexInputInfo.pVertexBindingDescriptions = nullptr;
	vkVertexInputInfo.vertexAttributeDescriptionCount = 0;
	vkVertexInputInfo.pVertexAttributeDescriptions = nullptr;

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
	vkRasterizerInfo.frontFace = vk::FrontFace::eClockwise;
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
	vkPipelineLayoutInfo.setLayoutCount = 0;
	vkPipelineLayoutInfo.pSetLayouts = nullptr;
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
	vkGraphicsPipelineCreateInfo.pDepthStencilState = nullptr;
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
		vk::ImageView attachments[] = {
			m_swapchainImageViews[i]
		};

		vk::FramebufferCreateInfo vkFrameBufferInfo{};
		vkFrameBufferInfo.sType = vk::StructureType::eFramebufferCreateInfo;
		vkFrameBufferInfo.renderPass = m_vkRenderPass;
		vkFrameBufferInfo.attachmentCount = 1;
		vkFrameBufferInfo.pAttachments = attachments;
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

	vk::CommandPoolCreateInfo vkCommandPoolInfo{};
	vkCommandPoolInfo.sType = vk::StructureType::eCommandPoolCreateInfo;
	vkCommandPoolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
	vkCommandPoolInfo.queueFamilyIndex = queueFamilyIndices.m_graphicsFamily.value();

	m_vkGraphicsCommandPool = m_vkLogicalDevice.createCommandPool( vkCommandPoolInfo );
	LOG_INFO("Graphics Command Pool created");
}

void VulkanApplication::createCommandBuffers()
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
	m_vkLogicalDevice.waitIdle();

	destroySwapChain();
	
	createSwapchain();
	createImageViews();
	createFrameBuffers();	
}

void VulkanApplication::destroySwapChain()
{
	for( auto& vkFramebuffer : m_swapchainFrameBuffers )
	{
		m_vkLogicalDevice.destroyFramebuffer( vkFramebuffer );
	}

	for( auto& vkImageView : m_swapchainImageViews )
	{
		m_vkLogicalDevice.destroyImageView( vkImageView );
	}

	m_swapchainImageViews.clear();

	m_vkLogicalDevice.destroySwapchainKHR( m_vkSwapchain );
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
	vk::ClearValue clearValue;
	vk::ClearColorValue clearColorValue;
	clearColorValue.setFloat32( {0.0f, 0.0f, 0.0f, 1.0f} );
	clearValue.setColor( clearColorValue );
	vkRenderPassBeginInfo.clearValueCount = 1;
	vkRenderPassBeginInfo.pClearValues = &clearValue;

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

	vkCommandBuffer.draw( 3, 1, 0, 0 );
	vkCommandBuffer.endRenderPass();
	vkCommandBuffer.end();
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