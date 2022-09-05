#include "vkrenderer/VulkanInstance.h"
#include "vkrenderer/VulkanDebugMessenger.h"
#include "vkrenderer/VulkanObjectCreateInfoFactory.h"
#include "utilities/VulkanLogger.h"

#include <spdlog/sinks/stdout_color_sinks.h>

int main(int argc, const char* argv[])
{
    spdlog::sink_ptr consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

    utils::VulkanRendererApiLogger::createInstance( { consoleSink } );
    utils::VulkanRendererApiLogger::getSingletonPtr()->getLogger()->set_level( spdlog::level::debug );
    
    vkrender::VulkanDebugMessenger debugMessenger{};
    vkrender::VulkanInstance instance{ "DebugMessengerTest" };

    auto debugMsgCreateInfo = vkrender::VulkanObjectCreateInfoFactory::populateDebugMessengerCreateInfoExt();

    instance.init( debugMsgCreateInfo );
    debugMessenger.init( debugMsgCreateInfo );

    instance.createInstance();

    debugMessenger.createDebugMessenger( &instance, nullptr );

    // debugMessenger should be destroyed before deleteing VulkanInstance
    // Should throw error message by Validation Layer if DebugMessengerExt is setup properly

    return 0;
}
