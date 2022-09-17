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
#include "window/window.h"
#include "utilities/VulkanLogger.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

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
    vkrender::VulkanInstance instance{ "GraphicsPipelineTest" };
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

    vkrender::VulkanGraphicsPipeline graphicsPipeline{ pLogicalDevice, pGraphicsRenderPass.get(), &pipelineLayout };
    graphicsPipeline.createGraphicsPipeline(
        shaderStages, {}, primitiveDescriptor, viewportStage
    );

    debugMessenger.destroyDebugMessenger( &instance, nullptr );
    
    return 0;
}