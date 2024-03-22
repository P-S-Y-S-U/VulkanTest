#include "application/VulkanApplication.h"
#include "utilities/VulkanLogger.h"
#include "vkrenderer/VulkanUBO.hpp"

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