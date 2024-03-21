#include "application/VulkanApplication.h"
#include "vkrenderer/VulkanDebugMessenger.h"
#include "vkrenderer/VulkanLayer.hpp"
#include "vkrenderer/VulkanSwapChainFactory.h"
#include "vkrenderer/VulkanUBO.hpp"
#include "graphics/Vertex.hpp"
#include "utilities/VulkanLogger.h"

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_wayland.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <set>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <algorithm>

#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

#ifndef TINYOBJLOADER_IMPLEMENTATION
	#define TINYOBJLOADER_IMPLEMENTATION
#endif
#include <tiny_obj_loader.h>

#ifndef STB_IMAGE_IMPLEMENTATION
	#define STB_IMAGE_IMPLEMENTATION
#endif
#include <stb/stb_image.h>

VulkanApplication::VulkanApplication( const std::string& applicationName )
    :m_applicationName{ applicationName }
	,m_window{ 800, 600 }
	,m_currentFrame{0}
	,m_bHasExclusiveTransferQueue{ false }
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
	createColorResources();
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

	m_vkGraphicsCommandBuffers[m_currentFrame].reset( {} );
	recordCommandBuffer( m_vkGraphicsCommandBuffers[m_currentFrame], imageIndex );

	vk::SubmitInfo vkCmdSubmitInfo{};
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

	if( m_bHasExclusiveTransferQueue )
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

void VulkanApplication::createSurface()
{
#ifdef _WIN32
	vk::Win32SurfaceCreateInfoKHR surfaceCreateInfo;
	surfaceCreateInfo.sType = vk::StructureType::eWin32SurfaceCreateInfoKHR;
	surfaceCreateInfo.hwnd = m_window.getHandle();
	surfaceCreateInfo.hinstance = GetModuleHandle( nullptr );

	m_vkSurface = m_vkInstance.createWin32SurfaceKHR( surfaceCreateInfo );
#else
	VkWaylandSurfaceCreateInfoKHR surfaceCreateInfo;
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.flags = 0;
	surfaceCreateInfo.surface = m_window.getHandle();
	surfaceCreateInfo.display = m_window.getDisplayHandle();
	surfaceCreateInfo.pNext = nullptr;

 	VkResult surfaceCreateReslt = vkCreateWaylandSurfaceKHR( 
		reinterpret_cast<const VkInstance&>( m_vkInstance ),
		&surfaceCreateInfo,
		nullptr,
		reinterpret_cast<VkSurfaceKHR*>( &m_vkSurface )	
	);

	if( surfaceCreateReslt != VK_SUCCESS )
	{
		LOG_ERROR("Failed to Create Vulkan Wayland Surface");
	}

#endif
	LOG_INFO( "Vulkan Surface Created" );
}

void VulkanApplication::createRenderPass()
{
	vk::AttachmentDescription vkColorAttachment{};
	vkColorAttachment.format = m_vkSwapchainImageFormat;
	vkColorAttachment.samples = m_msaaSampleCount;
	vkColorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	vkColorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
	vkColorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	vkColorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	vkColorAttachment.initialLayout = vk::ImageLayout::eUndefined;
	vkColorAttachment.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;

	vk::AttachmentReference vkColorAttachmentRef{};
	vkColorAttachmentRef.attachment = 0;
	vkColorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

	vk::AttachmentDescription vkDepthAttachment{};
	vkDepthAttachment.format = findDepthFormat();
	vkDepthAttachment.samples = m_msaaSampleCount;
	vkDepthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	vkDepthAttachment.storeOp = vk::AttachmentStoreOp::eDontCare;
	vkDepthAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	vkDepthAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	vkDepthAttachment.initialLayout = vk::ImageLayout::eUndefined;
	vkDepthAttachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

	vk::AttachmentReference vkDepthAttachmentRef{};
	vkDepthAttachmentRef.attachment = 1;
	vkDepthAttachmentRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

	vk::AttachmentDescription vkColorAttachmentResolve{};
	vkColorAttachmentResolve.format = m_vkSwapchainImageFormat;
	vkColorAttachmentResolve.samples = vk::SampleCountFlagBits::e1;
	vkColorAttachmentResolve.loadOp = vk::AttachmentLoadOp::eDontCare;
	vkColorAttachmentResolve.storeOp = vk::AttachmentStoreOp::eStore;
	vkColorAttachmentResolve.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	vkColorAttachmentResolve.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	vkColorAttachmentResolve.initialLayout = vk::ImageLayout::eUndefined;
	vkColorAttachmentResolve.finalLayout = vk::ImageLayout::ePresentSrcKHR;

	vk::AttachmentReference vkColorAttachmentResolveRef{};
	vkColorAttachmentResolveRef.attachment = 2;
	vkColorAttachmentResolveRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

	vk::SubpassDescription vkSubPassDesc{};
	vkSubPassDesc.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	vkSubPassDesc.colorAttachmentCount = 1;
	vkSubPassDesc.pColorAttachments = &vkColorAttachmentRef;
	vkSubPassDesc.pDepthStencilAttachment = &vkDepthAttachmentRef;
	vkSubPassDesc.pResolveAttachments = &vkColorAttachmentResolveRef;

	vk::SubpassDependency vkSubpassDependency{};
	vkSubpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	vkSubpassDependency.dstSubpass = 0;
	vkSubpassDependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
	vkSubpassDependency.srcAccessMask = vk::AccessFlagBits::eNone;
	vkSubpassDependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
	vkSubpassDependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

	std::array<vk::AttachmentDescription, 3> attachments{ vkColorAttachment, vkDepthAttachment, vkColorAttachmentResolve };
	vk::RenderPassCreateInfo vkRenderPassInfo{};
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
	descCreateInfo.poolSizeCount = static_cast<std::uint32_t>( descPoolSizes.size() );
	descCreateInfo.pPoolSizes = descPoolSizes.data();
	descCreateInfo.maxSets = static_cast<std::uint32_t>( MAX_FRAMES_IN_FLIGHT );

	m_vkDescriptorPool = m_vkLogicalDevice.createDescriptorPool( descCreateInfo );
}

void VulkanApplication::createDescriptorSets()
{
	std::vector<vk::DescriptorSetLayout> descLayouts( MAX_FRAMES_IN_FLIGHT, m_vkDescriptorSetLayout );

	vk::DescriptorSetAllocateInfo descSetAllocInfo{};
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
		
		descWrites[0].dstSet = m_vkDescriptorSets[i];
		descWrites[0].dstBinding = 0;
		descWrites[0].dstArrayElement = 0;
		descWrites[0].descriptorType = vk::DescriptorType::eUniformBuffer;
		descWrites[0].descriptorCount = 1;
		descWrites[0].pBufferInfo = &bufferInfo;
		descWrites[0].pImageInfo = nullptr;
		descWrites[0].pTexelBufferView = nullptr;

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
	vk::VertexInputBindingDescription bindingDesc = vertex::getBindingDescription();
	std::array<vk::VertexInputAttributeDescription, 3> attributeDesc = vertex::getAttributeDescriptions();
	vkVertexInputInfo.vertexBindingDescriptionCount = 1;
	vkVertexInputInfo.pVertexBindingDescriptions = &bindingDesc;
	vkVertexInputInfo.vertexAttributeDescriptionCount = attributeDesc.size();
	vkVertexInputInfo.pVertexAttributeDescriptions = attributeDesc.data();

	vk::PipelineInputAssemblyStateCreateInfo vkInputAssemblyInfo{};
	vkInputAssemblyInfo.topology = vk::PrimitiveTopology::eTriangleList;
	vkInputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

	vk::PipelineViewportStateCreateInfo vkViewportInfo{};
	vkViewportInfo.viewportCount = 1;
	vkViewportInfo.pViewports = nullptr;
	vkViewportInfo.scissorCount = 1;
	vkViewportInfo.pScissors = nullptr;

	vk::PipelineRasterizationStateCreateInfo vkRasterizerInfo{};
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
	vkMultisamplingInfo.sampleShadingEnable = VK_FALSE;
	vkMultisamplingInfo.rasterizationSamples = m_msaaSampleCount;
	vkMultisamplingInfo.minSampleShading = 1.0f;
	vkMultisamplingInfo.pSampleMask = nullptr;
	vkMultisamplingInfo.alphaToCoverageEnable = VK_FALSE;
	vkMultisamplingInfo.alphaToOneEnable = VK_FALSE;

	vk::PipelineDepthStencilStateCreateInfo vkDepthStencil{};
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
	vkDynamicStateInfo.dynamicStateCount = static_cast<std::uint32_t>(dynamicStates.size());
	vkDynamicStateInfo.pDynamicStates = dynamicStates.data();

	vk::PipelineLayoutCreateInfo vkPipelineLayoutInfo{};
	vkPipelineLayoutInfo.setLayoutCount = 1;
	vkPipelineLayoutInfo.pSetLayouts = &m_vkDescriptorSetLayout;
	vkPipelineLayoutInfo.pushConstantRangeCount = 0;
	vkPipelineLayoutInfo.pPushConstantRanges = nullptr;
	
	m_vkPipelineLayout = m_vkLogicalDevice.createPipelineLayout( vkPipelineLayoutInfo );

	LOG_INFO("Pipeline Layout created");

	vk::GraphicsPipelineCreateInfo vkGraphicsPipelineCreateInfo{};
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

void VulkanApplication::createSyncObjects()
{
	m_vkImageAvailableSemaphores.resize( MAX_FRAMES_IN_FLIGHT );
	m_vkRenderFinishedSemaphores.resize( MAX_FRAMES_IN_FLIGHT );
	m_vkInFlightFences.resize( MAX_FRAMES_IN_FLIGHT );

	vk::SemaphoreCreateInfo vkSemaphoreInfo{};

	vk::FenceCreateInfo vkFenceInfo{};
	vkFenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;

	for( auto i = 0u; i < MAX_FRAMES_IN_FLIGHT; i++ )
	{
		m_vkImageAvailableSemaphores[i] = m_vkLogicalDevice.createSemaphore( vkSemaphoreInfo );
		m_vkRenderFinishedSemaphores[i] = m_vkLogicalDevice.createSemaphore( vkSemaphoreInfo );

		m_vkInFlightFences[i] = m_vkLogicalDevice.createFence( vkFenceInfo );
	}

	LOG_INFO("Sync Objects For Rendering and Presentation created");
}

void VulkanApplication::recordCommandBuffer( vk::CommandBuffer& vkCommandBuffer, const std::uint32_t& imageIndex )
{
	vk::CommandBufferBeginInfo vkCmdBufBeginInfo{};
	vkCmdBufBeginInfo.flags = {};
	vkCmdBufBeginInfo.pInheritanceInfo = nullptr;

	vkCommandBuffer.begin( vkCmdBufBeginInfo );

	vk::RenderPassBeginInfo vkRenderPassBeginInfo{};
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

#if 1 // REMOVE
	vk::Buffer vertexBuffers[] = { m_vkVertexBuffer };
	vk::DeviceSize offsets[] = { 0 };
#else 
	vk::ArrayProxy<const vk::Buffer> vertexBuffers{ m_vkVertexBuffer };
	vk::ArrayProxy<const vk::DeviceSize> offsets{ 0 };
#endif 

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
	allocInfo.level = vk::CommandBufferLevel::ePrimary;
	allocInfo.commandPool = commandPoolToAllocFrom;
	allocInfo.commandBufferCount = 1;

	vk::CommandBuffer commandBuffer = m_vkLogicalDevice.allocateCommandBuffers(allocInfo)[0];

	vk::CommandBufferBeginInfo beginInfo{};
	beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

	commandBuffer.begin( beginInfo );

	return commandBuffer;
}

void VulkanApplication::endSingleTimeCommands( const vk::CommandPool& commandPoolAllocFrom, vk::CommandBuffer vkCommandBuffer, vk::Queue queueToSubmitOn )
{
	vkCommandBuffer.end();

	vk::SubmitInfo submitInfo{};
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &vkCommandBuffer;

	vk::Result opResult = queueToSubmitOn.submit( 1, &submitInfo, nullptr );
	queueToSubmitOn.waitIdle();

	m_vkLogicalDevice.freeCommandBuffers( commandPoolAllocFrom, 1, &vkCommandBuffer );
}

void VulkanApplication::transitionImageLayout( 
    const vk::Image& image, const vk::Format& format, 
    const vk::ImageLayout& oldLayout, const vk::ImageLayout& newLayout,
	const std::uint32_t& mipmapLevels
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
	imgBarrier.oldLayout = oldLayout;
	imgBarrier.newLayout = newLayout;
	imgBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imgBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imgBarrier.image = image;
	imgBarrier.subresourceRange.aspectMask = aspectMask;
	imgBarrier.subresourceRange.baseMipLevel = 0;
	imgBarrier.subresourceRange.levelCount = mipmapLevels;
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
	vkShaderModuleCreateInfo.codeSize = shaderSourceBuffer.size();
	vkShaderModuleCreateInfo.pCode = reinterpret_cast<const std::uint32_t*>( shaderSourceBuffer.data() );

	vk::ShaderModule vkShaderModule = m_vkLogicalDevice.createShaderModule( vkShaderModuleCreateInfo );

	return vkShaderModule;
}

void VulkanApplication::createBuffer(
    const vk::DeviceSize& bufferSizeInBytes,
    const vk::BufferUsageFlags& bufferUsage,
	const vk::SharingMode& bufferSharingMode,
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
	bufferInfo.size = bufferSizeInBytes;
	bufferInfo.usage = bufferUsage;
	bufferInfo.sharingMode = bufferSharingMode;
	std::vector<uint32_t> queueFamilyToShare;
	queueFamilyToShare.emplace_back( queueFamilyIndices.m_graphicsFamily.value() );
	if( queueFamilyIndices.m_exclusiveTransferFamily.has_value() ) queueFamilyToShare.emplace_back( queueFamilyIndices.m_exclusiveTransferFamily.value() );
	bufferInfo.pQueueFamilyIndices = queueFamilyToShare.data();
	bufferInfo.queueFamilyIndexCount = queueFamilyToShare.size();

	buffer = m_vkLogicalDevice.createBuffer(
		bufferInfo
	);

	vk::MemoryRequirements memRequirements = m_vkLogicalDevice.getBufferMemoryRequirements( buffer );

	vk::MemoryAllocateInfo allocInfo{};
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType( memRequirements.memoryTypeBits, memProps );

	bufferMemory = m_vkLogicalDevice.allocateMemory( allocInfo );

	m_vkLogicalDevice.bindBufferMemory( buffer, bufferMemory, 0 );
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

vkrender::SwapChainSupportDetails VulkanApplication::querySwapChainSupport( const vk::PhysicalDevice& vkPhysicalDevice, const vk::SurfaceKHR& vkSurface )
{
    vkrender::SwapChainSupportDetails swapChainDetails;

    swapChainDetails.capabilities =	vkPhysicalDevice.getSurfaceCapabilitiesKHR( vkSurface );
    swapChainDetails.surfaceFormats = vkPhysicalDevice.getSurfaceFormatsKHR( vkSurface );
    swapChainDetails.presentModes = vkPhysicalDevice.getSurfacePresentModesKHR( vkSurface );

    return swapChainDetails;
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
#if 0 // REMOVE THIS DEF 
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
#else 
	return logicalDevice.acquireNextImageKHR( swapchain, timeout, imageAcquireSemaphore, imageAcquireFence );
#endif 
}

vk::Result VulkanApplication::queuePresentWrapper(
    const vk::Queue& presentationQueue,
    const vk::PresentInfoKHR& presentInfo
)
{
#if 0 // REMOVE THIS DEF
	vk::DispatchLoaderStatic& dispatcher = vk::getDispatchLoaderStatic();
	return static_cast<vk::Result>( dispatcher.vkQueuePresentKHR(
		presentationQueue, reinterpret_cast<const VkPresentInfoKHR*>(&presentInfo)
	) );
#else 
	return presentationQueue.presentKHR( presentInfo );
#endif 
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

vk::SampleCountFlagBits VulkanApplication::getMaxUsableSampleCount()
{
	vk::PhysicalDeviceProperties physicalDeviceProps = m_vkPhysicalDevice.getProperties();

	vk::SampleCountFlags counts = physicalDeviceProps.limits.framebufferColorSampleCounts & physicalDeviceProps.limits.framebufferDepthSampleCounts;

	if( counts & vk::SampleCountFlagBits::e64 ) { return vk::SampleCountFlagBits::e64; }
	if( counts & vk::SampleCountFlagBits::e32 ) { return vk::SampleCountFlagBits::e32; }
	if( counts & vk::SampleCountFlagBits::e16 ) { return vk::SampleCountFlagBits::e16; }
	if( counts & vk::SampleCountFlagBits::e8 ) { return vk::SampleCountFlagBits::e8; }
	if( counts & vk::SampleCountFlagBits::e4 ) { return vk::SampleCountFlagBits::e4; }
	if( counts & vk::SampleCountFlagBits::e2 ) { return vk::SampleCountFlagBits::e2; }

	return vk::SampleCountFlagBits::e1;
}