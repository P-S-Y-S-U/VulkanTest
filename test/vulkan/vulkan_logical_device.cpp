#include "vkrenderer/VulkanInstance.h"
#include "vkrenderer/VulkanDebugMessenger.h"
#include "vkrenderer/VulkanPhysicalDeviceManager.h"
#include "vkrenderer/VulkanLogicalDeviceManager.h"
#include "vkrenderer/VulkanObjectCreateInfoFactory.h"
#include "vkrenderer/VulkanSurface.h"
#include "window/window.h"

#include <iostream>

int main(int argc, const char* argv[])
{
    vkrender::Window window{};
    vkrender::VulkanInstance instance{ "LogicalDeviceTest" };
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

    std::cout << "Vulkan GPU selected!" << "\n";
    deviceManager.probePhysicalDevice( *pPhysicalDevice );

    vkrender::VulkanLogicalDevice* pGraphicsLogicalDevice = logicalDeviceManager.createLogicalDevice( pPhysicalDevice );
    vkrender::VulkanLogicalDevice* pPresentationLogicalDevice = logicalDeviceManager.createLogicalDevice( pPhysicalDevice, upSurface.get() );

    debugMessenger.destroyDebugMessenger( &instance, nullptr );
    
    return 0;
}