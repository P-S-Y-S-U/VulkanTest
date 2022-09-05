#include "vkrenderer/VulkanInstance.h"
#include "vkrenderer/VulkanDebugMessenger.h"
#include "window/window.h"
#include "vkrenderer/VulkanObjectCreateInfoFactory.h"
#include "utilities/VulkanLogger.h"
#include <spdlog/sinks/stdout_color_sinks.h>

#include <iostream>

int main(int argc, const char* argv[])
{
    spdlog::sink_ptr consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

    utils::VulkanRendererApiLogger::createInstance( { consoleSink } );
    utils::VulkanRendererApiLogger::getSingletonPtr()->getLogger()->set_level( spdlog::level::debug );

    vkrender::Window window{};
    vkrender::VulkanInstance instance{ "LogicalDeviceTest" };
    vkrender::VulkanDebugMessenger debugMessenger{};

    auto debugMsgCreateInfo = vkrender::VulkanObjectCreateInfoFactory::populateDebugMessengerCreateInfoExt();
    
    window.init();
    instance.init( debugMsgCreateInfo );
    debugMessenger.init( debugMsgCreateInfo );

    instance.createInstance();
    debugMessenger.createDebugMessenger( &instance, nullptr );

    utils::Uptr<vkrender::VulkanSurface> pSurface = std::move( window.createSurface( &instance ) );
    
    pSurface.reset();
    debugMessenger.destroyDebugMessenger(&instance, nullptr);

    return 0;
}