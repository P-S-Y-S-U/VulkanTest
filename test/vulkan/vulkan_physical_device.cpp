#include "vkrenderer/VulkanInstance.h"
#include "vkrenderer/VulkanDebugMessenger.h"
#include "vkrenderer/VulkanPhysicalDeviceManager.h"
#include "vkrenderer/VulkanObjectCreateInfoFactory.h"

#include <iostream>

int main(int argc, const char* argv[])
{
    vkrender::VulkanInstance instance{ "PhysicalDeviceTest" };
    vkrender::VulkanDebugMessenger debugMessenger{};

    auto debugMsgCreateInfo = vkrender::VulkanObjectCreateInfoFactory::populateDebugMessengerCreateInfoExt();
    
    instance.init( debugMsgCreateInfo );
    debugMessenger.init( debugMsgCreateInfo );

    instance.createInstance();
    debugMessenger.createDebugMessenger( &instance, nullptr );

    vkrender::VulkanPhysicalDeviceManager deviceManager{ &instance };

    vkrender::VulkanPhysicalDevice* pPhysicalDevice = deviceManager.createSuitableDevice(); // Throws error if manager cant find a suitable device

    std::cout << "Vulkan GPU selected!" << "\n";
    deviceManager.probePhysicalDevice( *pPhysicalDevice );

    debugMessenger.destroyDebugMessenger( &instance, nullptr );
    
    return 0;
}