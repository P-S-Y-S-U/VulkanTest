#include "application/VulkanApplication.h"
#include "utilities/VulkanLogger.h"

#include "vkrenderer/VulkanUBO.hpp"

void VulkanApplication::createCommandPool()
{
	using namespace vkrender;

	QueueFamilyIndices queueFamilyIndices = findQueueFamilyIndices( m_vkPhysicalDevice, &m_vkSurface );

	vk::CommandPoolCreateInfo vkGraphicsCommandPoolInfo{};
	vkGraphicsCommandPoolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
	vkGraphicsCommandPoolInfo.queueFamilyIndex = queueFamilyIndices.m_graphicsFamily.value();

	if( m_bHasExclusiveTransferQueue )
	{
		vk::CommandPoolCreateInfo vkTransferCommandPoolInfo{};
		vkTransferCommandPoolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
		vkTransferCommandPoolInfo.queueFamilyIndex = queueFamilyIndices.m_exclusiveTransferFamily.has_value() ? queueFamilyIndices.m_exclusiveTransferFamily.value() : queueFamilyIndices.m_graphicsFamily.value();

		m_vkGraphicsCommandPool = m_vkLogicalDevice.createCommandPool( vkGraphicsCommandPoolInfo );
		LOG_INFO("Graphics Command Pool created");

		m_vkTransferCommandPool = m_vkLogicalDevice.createCommandPool( vkTransferCommandPoolInfo );
		LOG_INFO("Transfer Command Pool created");
	} 
	else
	{
		m_vkGraphicsCommandPool = m_vkLogicalDevice.createCommandPool( vkGraphicsCommandPoolInfo );
		LOG_INFO("Graphics Command Pool created");

		m_vkTransferCommandPool = m_vkGraphicsCommandPool;
		LOG_INFO("Using Graphics Command Pool for Transfer Operations");
	}
}

void VulkanApplication::createConfigCommandBuffer()
{
	vk::CommandBufferAllocateInfo allocInfo{};
	allocInfo.commandPool = m_vkGraphicsCommandPool;
	allocInfo.level = vk::CommandBufferLevel::ePrimary;
	allocInfo.commandBufferCount = 1;

	m_vkConfigCommandBuffer = m_vkLogicalDevice.allocateCommandBuffers(allocInfo)[0];

	LOG_INFO("Config Command Buffer created");
}

void VulkanApplication::createGraphicsCommandBuffers()
{
	m_vkGraphicsCommandBuffers.resize( MAX_FRAMES_IN_FLIGHT );

	vk::CommandBufferAllocateInfo vkCmdBufAllocateInfo{};
	vkCmdBufAllocateInfo.commandPool = m_vkGraphicsCommandPool;
	vkCmdBufAllocateInfo.level = vk::CommandBufferLevel::ePrimary;
	vkCmdBufAllocateInfo.commandBufferCount = m_vkGraphicsCommandBuffers.size();
	
	m_vkGraphicsCommandBuffers = m_vkLogicalDevice.allocateCommandBuffers( vkCmdBufAllocateInfo );

	LOG_INFO("Graphics Command Buffer created");
}

void VulkanApplication::createVertexBuffer()
{

	std::size_t bufferSizeInBytes = sizeof(vertex) * m_inputVertexData.size();

	vk::Buffer stagingBuffer;
	vk::DeviceMemory stagingBufferMemory;
	vk::SharingMode bufferSharingMode = m_bHasExclusiveTransferQueue ? vk::SharingMode::eConcurrent : vk::SharingMode::eExclusive;
	createBuffer( 
		static_cast<vk::DeviceSize>( bufferSizeInBytes ),
		vk::BufferUsageFlagBits::eTransferSrc,
		bufferSharingMode,
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
		bufferSharingMode,
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
	vk::SharingMode bufferSharingMode = m_bHasExclusiveTransferQueue ? vk::SharingMode::eConcurrent : vk::SharingMode::eExclusive;

	createBuffer(
		bufferSizeInBytes,
		vk::BufferUsageFlagBits::eTransferSrc,
		bufferSharingMode,
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
		bufferSharingMode,
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

	vk::SharingMode bufferSharingMode = m_bHasExclusiveTransferQueue ? vk::SharingMode::eConcurrent : vk::SharingMode::eExclusive;
	
	for( std::size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ )
	{
		createBuffer(
			bufferSize, vk::BufferUsageFlagBits::eUniformBuffer,
			bufferSharingMode,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
			m_vkUniformBuffers[i], m_vkUniformBuffersMemory[i]
		);

		m_uniformBuffersMapped[i] = m_vkLogicalDevice.mapMemory( m_vkUniformBuffersMemory[i], 0, bufferSize );
	}
}

void VulkanApplication::setupConfigCommandBuffer()
{
	m_vkConfigCommandBuffer.reset( {} );

	vk::CommandBufferBeginInfo beginInfo{};
	beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
	
	m_vkConfigCommandBuffer.begin( beginInfo );
}

void VulkanApplication::flushConfigCommandBuffer()
{
	m_vkConfigCommandBuffer.end();

	vk::SubmitInfo submitInfo{};
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_vkConfigCommandBuffer;

	vk::Result opResult = m_vkGraphicsQueue.submit( 1, &submitInfo, {} );
	m_vkGraphicsQueue.waitIdle();
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