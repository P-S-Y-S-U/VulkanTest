#include "application/VulkanApplication.h"
#include "utilities/VulkanLogger.h"

#include <tiny_obj_loader.h>
#include <stb/stb_image.h>

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

	m_imageMiplevels = static_cast<std::uint32_t>( std::floor( std::log2( std::max( texWidth, texHeight ) ) ) ) + 1;

	vk::DeviceSize imageSize = texWidth * texHeight * 4;

	vk::Buffer stagingBuffer;
	vk::DeviceMemory stagingBufferMemory;
	vk::SharingMode stagingBufferSharingMode = m_bHasExclusiveTransferQueue ? vk::SharingMode::eConcurrent : vk::SharingMode::eExclusive;
	createBuffer( 
		imageSize, 
		vk::BufferUsageFlagBits::eTransferSrc,
		stagingBufferSharingMode,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
		stagingBuffer,
		stagingBufferMemory
	);


	void* bufferMappedData = m_vkLogicalDevice.mapMemory( stagingBufferMemory, 0, imageSize );
	std::memcpy(bufferMappedData, pixels, static_cast<std::size_t>(imageSize) );
	m_vkLogicalDevice.unmapMemory(stagingBufferMemory);

	stbi_image_free(pixels);

	createImage( 
		texWidth, texHeight, m_imageMiplevels,
		vk::SampleCountFlagBits::e1,
		vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
		vk::MemoryPropertyFlagBits::eDeviceLocal,
		m_vkTextureImage, m_vkTextureImageMemory
	);

	transitionImageLayout( 
		m_vkTextureImage, vk::Format::eR8G8B8A8Srgb, 
		vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal,
		m_imageMiplevels
	);

	copyBufferToImage( 
		stagingBuffer, m_vkTextureImage, 
		static_cast<std::uint32_t>( texWidth ), 
		static_cast<std::uint32_t>( texHeight )
	);

	generateMipmaps( m_vkTextureImage, vk::Format::eR8G8B8A8Srgb, texWidth, texHeight, m_imageMiplevels);

	m_vkLogicalDevice.destroyBuffer( stagingBuffer, nullptr );
	m_vkLogicalDevice.freeMemory( stagingBufferMemory, nullptr );
}

void VulkanApplication::createTextureImageView()
{
	m_vkTextureImageView = createImageView( 
		m_vkTextureImage, vk::Format::eR8G8B8A8Srgb, 
		vk::ImageAspectFlagBits::eColor,
		m_imageMiplevels 
	);
}

void VulkanApplication::createTextureSampler()
{
	vk::PhysicalDeviceProperties phyDeviceProp = m_vkPhysicalDevice.getProperties(); 

	vk::SamplerCreateInfo samplerCreateInfo{};
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
	samplerCreateInfo.maxLod = static_cast<float>(m_imageMiplevels);

	m_vkTextureSampler = m_vkLogicalDevice.createSampler( samplerCreateInfo );
}

void VulkanApplication::createImage(
    const std::uint32_t& width, const std::uint32_t& height, const std::uint32_t& mipmapLevels,
	const vk::SampleCountFlagBits& numOfSamples,
    const vk::Format& format, const vk::ImageTiling& tiling,
    const vk::ImageUsageFlags& usageFlags, const vk::MemoryPropertyFlags& memPropFlags,
    vk::Image& image, vk::DeviceMemory& imageMemory
)
{
	vk::ImageCreateInfo imageCreateInfo{};
	imageCreateInfo.imageType = vk::ImageType::e2D;
	imageCreateInfo.extent.width = static_cast<std::uint32_t>( width );
	imageCreateInfo.extent.height = static_cast<std::uint32_t>( height );
	imageCreateInfo.extent.depth = 1;
	imageCreateInfo.mipLevels = mipmapLevels;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.format = format;
	imageCreateInfo.tiling = tiling;
	imageCreateInfo.initialLayout = vk::ImageLayout::eUndefined;
	imageCreateInfo.usage = usageFlags;
	imageCreateInfo.sharingMode = vk::SharingMode::eExclusive;
	imageCreateInfo.samples = numOfSamples;
	imageCreateInfo.flags = {};

	image = m_vkLogicalDevice.createImage( imageCreateInfo );

	vk::MemoryRequirements memRequirements = m_vkLogicalDevice.getImageMemoryRequirements( image );
	
	vk::MemoryAllocateInfo allocInfo{};
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType( memRequirements.memoryTypeBits, memPropFlags );

	imageMemory = m_vkLogicalDevice.allocateMemory( allocInfo );

	m_vkLogicalDevice.bindImageMemory( image, imageMemory, 0 );
}

vk::ImageView VulkanApplication::createImageView( 
	const vk::Image& image,
	const vk::Format& format,
	const vk::ImageAspectFlags& aspect,
	const std::uint32_t& mipmapLevels
)
{
	vk::ImageViewCreateInfo vkImageViewCreateInfo{};
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
    subResourceRange.levelCount = mipmapLevels;
    subResourceRange.baseArrayLayer = 0;
    subResourceRange.layerCount = 1;
    vkImageViewCreateInfo.subresourceRange = subResourceRange;

	return m_vkLogicalDevice.createImageView( vkImageViewCreateInfo );
}

void VulkanApplication::generateMipmaps( 
    const vk::Image& image,
	const vk::Format& imgFormat,
    const std::int32_t& texWidth, const std::int32_t& texHeight,
    const std::uint32_t& mipLevels
)
{
	vk::FormatProperties formatProps = m_vkPhysicalDevice.getFormatProperties(
		imgFormat
	);

	if( !( formatProps.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear ) )
	{
		std::string errormsg = "texture image format does not support linear blitting";
		LOG_ERROR(errormsg);
		throw std::runtime_error(errormsg);
	}

	std::int32_t mipImgWidth = texWidth;
	std::int32_t mipImgHeight = texHeight;

	vk::CommandBuffer cmdBuf = beginSingleTimeCommands( m_vkGraphicsCommandPool );

	vk::ImageMemoryBarrier imgBarrier{};
	imgBarrier.image = image;
	imgBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imgBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imgBarrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	imgBarrier.subresourceRange.baseArrayLayer = 0;
	imgBarrier.subresourceRange.layerCount = 1;
	imgBarrier.subresourceRange.levelCount = 1;

	for( int32_t i = 1; i < mipLevels; i++ )
	{
		// Transition the baseMipLevel to Transfer source first
		imgBarrier.subresourceRange.baseMipLevel = i - 1;
		imgBarrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
		imgBarrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
		imgBarrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		imgBarrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

		cmdBuf.pipelineBarrier(
			vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, {},
			0, nullptr, 
			0, nullptr,
			1, &imgBarrier
		);

		// Blit Image
		vk::ImageBlit imgBlit{};
		imgBlit.srcOffsets[0] = vk::Offset3D{ 0, 0, 0 };
		imgBlit.srcOffsets[1] = vk::Offset3D{ mipImgWidth, mipImgHeight, 1 };
		imgBlit.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
		imgBlit.srcSubresource.mipLevel = i - 1;
		imgBlit.srcSubresource.baseArrayLayer = 0;
		imgBlit.srcSubresource.layerCount = 1;
		imgBlit.dstOffsets[0] = vk::Offset3D{ 0, 0, 0 };
		imgBlit.dstOffsets[1] = vk::Offset3D{ 
			mipImgWidth > 1 ?  mipImgWidth / 2 : 1,
			mipImgHeight > 1 ? mipImgHeight / 2 : 1,
			1
		};
		imgBlit.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
		imgBlit.dstSubresource.mipLevel = i;
		imgBlit.dstSubresource.baseArrayLayer = 0;
		imgBlit.dstSubresource.layerCount = 1;

		vk::ArrayProxy<vk::ImageBlit> imgBlitArray{ imgBlit };
		
		cmdBuf.blitImage(
			image, vk::ImageLayout::eTransferSrcOptimal,
			image, vk::ImageLayout::eTransferDstOptimal,
			imgBlitArray,
			vk::Filter::eLinear
		);
		
		// Transition the baseMipLevel to Shader
		imgBarrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
		imgBarrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		imgBarrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
		imgBarrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;		

		cmdBuf.pipelineBarrier(
			vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {},
			0, nullptr, 
			0, nullptr,
			1, &imgBarrier
		);

		if( mipImgWidth > 1 ) mipImgWidth /= 2;
		if( mipImgHeight > 1 ) mipImgHeight /= 2;
	}

	// Transition the finalMiplevel to Shader
	imgBarrier.subresourceRange.baseMipLevel = mipLevels - 1;
	imgBarrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
	imgBarrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	imgBarrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
	imgBarrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

	cmdBuf.pipelineBarrier(
		vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {},
		0, nullptr, 
		0, nullptr,
		1, &imgBarrier
	);

	endSingleTimeCommands( m_vkGraphicsCommandPool, cmdBuf, m_vkGraphicsQueue );
}