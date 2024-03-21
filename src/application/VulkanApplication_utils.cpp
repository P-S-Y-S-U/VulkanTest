#include "application/VulkanApplication.h"
#include "utilities/VulkanLogger.h"
#include "vkrenderer/VulkanLayer.hpp"

#include <fstream>

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