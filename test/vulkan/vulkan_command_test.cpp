#include "vkrenderer/VulkanInstance.h"
#include "vkrenderer/VulkanDebugMessenger.h"
#include "vkrenderer/VulkanPhysicalDeviceManager.h"
#include "vkrenderer/VulkanLogicalDeviceManager.h"
#include "vkrenderer/VulkanObjectCreateInfoFactory.h"
#include "vkrenderer/VulkanSurface.h"
#include "vkrenderer/VulkanSwapChainFactory.h"
#include "vkrenderer/VulkanSwapChain.h"
#include "vkrenderer/VulkanImageView.h"
#include "vkrenderer/VulkanRenderPassFactory.h"
#include "vkrenderer/VulkanGraphicsPipeline.h"
#include "vkrenderer/VulkanFrameBuffer.h"
#include "vkrenderer/VulkanCommandPool.h"

#include "window/window.h"
#include "utilities/VulkanLogger.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <limits>

#include <iostream>

int main(int argc, const char* argv[])
{
    spdlog::sink_ptr consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

    std::initializer_list<spdlog::sink_ptr> logSinks{
        consoleSink
    };

    utils::VulkanValidationLayerLogger::createInstance( logSinks );
    utils::VulkanValidationLayerLogger::getSingletonPtr()->getLogger()->set_level( spdlog::level::debug );

    utils::VulkanRendererApiLogger::createInstance( logSinks );
    utils::VulkanRendererApiLogger::getSingletonPtr()->getLogger()->set_level( spdlog::level::debug );

    vkrender::Window window{};;
    vkrender::VulkanInstance instance{ "VulkanCommandTest" };
    vkrender::VulkanDebugMessenger debugMessenger{};

    auto debugMsgCreateInfo = vkrender::VulkanObjectCreateInfoFactory::populateDebugMessengerCreateInfoExt();
    
    window.init();
    instance.init( debugMsgCreateInfo );
    debugMessenger.init( debugMsgCreateInfo );

    instance.createInstance();
    debugMessenger.createDebugMessenger( &instance, nullptr );

    utils::Uptr<vkrender::VulkanSurface> upSurface = std::move( window.createSurface( &instance ) );

    vkrender::VulkanPhysicalDeviceManager deviceManager{ &instance };
    vkrender::VulkanLogicalDeviceManager logicalDeviceManager{};

    vkrender::VulkanPhysicalDevice* pPhysicalDevice = deviceManager.createSuitableDevice( *upSurface ); // Throws error if manager cant find a suitable device

    vkrender::VulkanLogicalDevice* pLogicalDevice = logicalDeviceManager.createLogicalDevice( pPhysicalDevice, upSurface.get() );

    utils::Sptr<vkrender::SwapChainPreset> spSwapChainPreset = vkrender::VulkanSwapChainFactory::createSuitableSwapChainPreset( *pPhysicalDevice, *upSurface, window );
    vkrender::VulkanSwapChain swapChain{ spSwapChainPreset, pLogicalDevice, upSurface.get() };
    swapChain.createSwapChain();

    std::vector<utils::Uptr<vkrender::VulkanImageView>> swapChainImageViews;

    for( const auto& vkImage : swapChain.getSwapChainImages() )
    {
        utils::Uptr<vkrender::VulkanImageView> imageView = std::make_unique<vkrender::VulkanImageView>(
            vkImage, swapChain.getImageFormat(), pLogicalDevice
        );
        imageView->createImageView();
        swapChainImageViews.emplace_back( std::move(imageView) );
    }

    std::filesystem::path vertexShaderPath = "triangleVert.spv";
    std::filesystem::path fragmentShaderPath = "triangleFrag.spv";

    utils::Sptr<vkrender::VulkanShaderModule> pVertexShaderModule = std::make_unique<vkrender::VulkanShaderModule>( vertexShaderPath, pLogicalDevice );
    utils::Sptr<vkrender::VulkanShaderModule> pFragmentShaderModule = std::make_unique<vkrender::VulkanShaderModule>( fragmentShaderPath, pLogicalDevice );

    pVertexShaderModule->createShaderModule();
    pFragmentShaderModule->createShaderModule();

    utils::Uptr<vkrender::VulkanRenderPass> pGraphicsRenderPass = std::move( vkrender::VulkanRenderPassFactory::setupRenderPassForSwapChain( pLogicalDevice, &swapChain ) );
    pGraphicsRenderPass->createRenderPass();

    vkrender::VulkanPipelineLayout pipelineLayout{ pLogicalDevice };
    pipelineLayout.createPipelineLayout();

    vkrender::VulkanGraphicsPipeline::PipelineShaderStage shaderStages;
    shaderStages.pVertexShader = pVertexShaderModule;
    shaderStages.pFragmentShader = pFragmentShaderModule;

    vkrender::VulkanGraphicsPipeline::PipelinePrimitiveDescriptor primitiveDescriptor;
    primitiveDescriptor.primitiveTopology = vk::PrimitiveTopology::eTriangleList;
    primitiveDescriptor.primitiveRestartEnable = false;

    vk::Viewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    vk::Extent2D swapChainExtent = swapChain.getSwapChainExtent();
    viewport.width = static_cast<float>( swapChainExtent.width );
    viewport.height = static_cast<float>( swapChainExtent.height );
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vk::Rect2D scissor;
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent = swapChainExtent;

    vkrender::VulkanGraphicsPipeline::PipelineViewportStage viewportStage;
    viewportStage.viewports.push_back( viewport );
    viewportStage.scissors.push_back( scissor );

    std::vector<vk::DynamicState> dynamicStates{
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor
    };
    vkrender::VulkanGraphicsPipeline graphicsPipeline{ pLogicalDevice, pGraphicsRenderPass.get(), &pipelineLayout };
    graphicsPipeline.createGraphicsPipeline(
        shaderStages, dynamicStates, primitiveDescriptor, viewportStage
    );

    std::vector<utils::Uptr<vkrender::VulkanFrameBuffer>> framebuffers;
    for( auto& imageView : swapChainImageViews )
    {
        std::vector<vkrender::VulkanImageView*> imageviewPtr{ imageView.get() };
        utils::Uptr<vkrender::VulkanFrameBuffer> pFramebuffer = std::make_unique<vkrender::VulkanFrameBuffer>(pLogicalDevice, pGraphicsRenderPass.get(), imageviewPtr, swapChainExtent);
        pFramebuffer->createFrameBuffer();
        framebuffers.push_back(std::move(pFramebuffer));
    }

    vkrender::QueueFamilyIndices queueFamilyIndices = vkrender::VulkanQueueFamily::findQueueFamilyIndices( *pPhysicalDevice, *upSurface );

    vkrender::VulkanCommandPool graphicsCommandPool{ pLogicalDevice, queueFamilyIndices.m_graphicsFamily.value() };
    graphicsCommandPool.createCommandPool();
    
    vk::CommandBufferAllocateInfo allocationInfo{};
    allocationInfo.sType = vk::StructureType::eCommandBufferAllocateInfo;
    allocationInfo.commandPool = graphicsCommandPool.m_commandPoolHandle;
    allocationInfo.level = vk::CommandBufferLevel::ePrimary;
    allocationInfo.commandBufferCount = 1;

    vk::Device* pDevice = pLogicalDevice->getHandle();

    vk::CommandBuffer commandBuffer = pDevice->allocateCommandBuffers( allocationInfo )[0];    

    auto l_recordRenderingCommand = [&pGraphicsRenderPass, &graphicsPipeline, &swapChainExtent, &framebuffers]( vk::CommandBuffer* pCommandBuffer, const std::uint32_t& imageIndex ) {
        vk::CommandBufferBeginInfo commandBeginInfo{};
        commandBeginInfo.sType = vk::StructureType::eCommandBufferBeginInfo;
        commandBeginInfo.flags = {};
        commandBeginInfo.pInheritanceInfo = nullptr;

        pCommandBuffer->begin( commandBeginInfo );

        vk::RenderPass* pRenderPassHandle = pGraphicsRenderPass->getHandle();
        vk::RenderPassBeginInfo renderpassBeginInfo{};
        renderpassBeginInfo.sType = vk::StructureType::eRenderPassBeginInfo;
        renderpassBeginInfo.renderPass = *pRenderPassHandle;
        renderpassBeginInfo.framebuffer = *framebuffers[imageIndex]->getHandle();
        vk::ClearValue clearColor;
        clearColor.color.setFloat32({ 0.0f, 0.0f, 0.0f, 1.0f });
        renderpassBeginInfo.clearValueCount = 1;
        renderpassBeginInfo.pClearValues = &clearColor;

        pCommandBuffer->beginRenderPass( renderpassBeginInfo, vk::SubpassContents::eInline );
    
        vk::Pipeline* pPipelineHandle = graphicsPipeline.getHandle();

        pCommandBuffer->bindPipeline( vk::PipelineBindPoint::eGraphics, *pPipelineHandle );

        vk::Viewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(swapChainExtent.width);
        viewport.height = static_cast<float>(swapChainExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 0.0f;

        pCommandBuffer->setViewport(0, 1, &viewport);
    
        vk::Rect2D scissor{};
        scissor.offset.x = 0;
        scissor.offset.y = 0; 
        scissor.extent = swapChainExtent;

        pCommandBuffer->setScissor( 0, 1, &scissor );

        pCommandBuffer->draw(3, 1, 0, 0);
        pCommandBuffer->endRenderPass();
    };

    auto l_createSemaphore = []( vk::Device* pDevice ) -> vk::Semaphore{
        vk::SemaphoreCreateInfo semaphoreCreateInfo{};
        semaphoreCreateInfo.sType = vk::StructureType::eSemaphoreCreateInfo;
        
        return pDevice->createSemaphore( semaphoreCreateInfo );
    };

    auto l_destroySemaphore = []( vk::Device* pDevice, vk::Semaphore* pSemaphore ){
        pDevice->destroySemaphore( *pSemaphore );
    };

    auto l_createFence = []( vk::Device* pDevice ) -> vk::Fence{
        vk::FenceCreateInfo fenceCreateInfo{};
        fenceCreateInfo.sType = vk::StructureType::eFenceCreateInfo;
        fenceCreateInfo.flags = vk::FenceCreateFlagBits::eSignaled;

        return pDevice->createFence( fenceCreateInfo );
    };

    auto l_destroyFence = []( vk::Device* pDevice, vk::Fence* pFence ){
        pDevice->destroy(*pFence);
    };

    vk::Semaphore imageAvailableSemaphore = l_createSemaphore( pDevice );
    vk::Semaphore renderFinishedSemaphore = l_createSemaphore( pDevice );
    vk::Fence frameAvailableToSwapFence = l_createFence( pDevice );

    // RENDERING LOOP
    auto l_RenderFrame = 
        [
            &frameAvailableToSwapFence, 
            &imageAvailableSemaphore, 
            &renderFinishedSemaphore,
            &l_recordRenderingCommand        
        ]( vk::Device* pDevice, vk::SwapchainKHR* pSwapChain, vk::CommandBuffer* pCommandBuffer, vk::Queue* pGraphicsQueue ) -> void {

        pDevice->waitForFences( 
            1, 
            &frameAvailableToSwapFence, VK_TRUE, 
            std::numeric_limits<std::uint64_t>::max()
        );

        auto&&[result, imageIndex] = pDevice->acquireNextImageKHR( 
            *pSwapChain,
            std::numeric_limits<std::uint64_t>::max(),
            imageAvailableSemaphore, nullptr
        );
        
        if( result != vk::Result::eSuccess )
        {
            const char* errorMsg = "FAILED TO ACQUIRE NEXT IMAGE FROM SWAPCHAIN";
            utils::VulkanRendererApiLogger::getSingletonPtr()->getLogger()->error( errorMsg );
            vk::throwResultException(result, errorMsg);
        }

        pCommandBuffer->reset();
        l_recordRenderingCommand( pCommandBuffer, imageIndex );
        
        vk::SubmitInfo submitInfo{};
        submitInfo.sType = vk::StructureType::eSubmitInfo;

        vk::Semaphore waitSemaphores[] = {imageAvailableSemaphore};
        vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = pCommandBuffer;
        vk::Semaphore signalSemaphore[] = {renderFinishedSemaphore};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphore;

        pGraphicsQueue->submit( 1, &submitInfo, frameAvailableToSwapFence );
    };
    
    vk::SwapchainKHR* pSwapChainHandle = swapChain.getHandle();

    debugMessenger.destroyDebugMessenger( &instance, nullptr );

    l_destroyFence( pDevice, &frameAvailableToSwapFence );
    l_destroySemaphore( pDevice, &renderFinishedSemaphore );
    l_destroySemaphore( pDevice, &imageAvailableSemaphore );
    
    return 0;
}