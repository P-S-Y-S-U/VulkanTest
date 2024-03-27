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
	,m_window{ 1920, 1080 }
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

#if 1
	ubo.model = glm::rotate(
		glm::mat4{1.0f},
		duration * glm::radians( 90.0f ),
		glm::vec3{ 0.0, 1.0, 0.0 }
	);

	ubo.view = glm::lookAt(
		glm::vec3{ 0.0f, 0.0f, 3.0f },
		glm::vec3{ 0.0f, 0.0f, 0.0f },
		glm::vec3{ 0.0f, 1.0f, 0.0f }
	);
	ubo.projection = glm::perspective(
		glm::radians( 45.0f ), 
		m_vkSwapchainExtent.width / (float) m_vkSwapchainExtent.height,
		0.1f,
		10.0f
	);
	//ubo.projection[1][1] *= -1.0f;
#else 
	ubo.model = glm::identity<glm::mat4>();
	ubo.view = glm::identity<glm::mat4>();
	ubo.projection = glm::identity<glm::mat4>();
#endif 

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
	clearColorValue.setFloat32( {0.1, 0.1, 0.4, 1.0} );
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