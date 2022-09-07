#include "vkrenderer/VulkanInstance.h"
#include "vkrenderer/VulkanDebugMessenger.h"
#include "vkrenderer/VulkanPhysicalDeviceManager.h"
#include "vkrenderer/VulkanLogicalDeviceManager.h"
#include "vkrenderer/VulkanObjectCreateInfoFactory.h"
#include "vkrenderer/VulkanSurface.h"
#include "vkrenderer/VulkanSwapChainFactory.h"
#include "vkrenderer/VulkanSwapChain.h"
#include "vkrenderer/VulkanImageView.h"
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
    vkrender::VulkanInstance instance{ "ImageViewTest" };
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

    debugMessenger.destroyDebugMessenger( &instance, nullptr );
    
    return 0;
}