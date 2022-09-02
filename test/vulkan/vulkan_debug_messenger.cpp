#include "vkrenderer/VulkanInstance.h"
#include "vkrenderer/VulkanDebugMessenger.h"
#include "vkrenderer/VulkanObjectCreateInfoFactory.h"

int main(int argc, const char* argv[])
{
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
