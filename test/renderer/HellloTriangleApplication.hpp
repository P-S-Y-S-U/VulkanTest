#ifndef APP_TRIANGLE_HPP
#define APP_TRIANGLE_HPP

#include "window/window.hpp"
#include "vkrenderer/VulkanInstance.hpp"
#include "vkrenderer/VulkanDebugMessenger.hpp"
#include "vkrenderer/VulkanPhysicalDevice.hpp"
#include "vkrenderer/VulkanLogicalDevice.hpp"
#include "utilities/memory.hpp"

namespace app
{

    class HelloTriangleApplication
    {
    public:
        HelloTriangleApplication();
        ~HelloTriangleApplication() = default;

        void run();

    private:
        void initWindow();
        void initVulkan();
        void createInstance();
        void setup_debug_messenger();
        void pick_physical_device();
        void create_logical_device();
        void mainLoop();
        void cleanup();

        Window                                                  _window;
        utils::Uptr<VulkanInstance>                             _instance;
        utils::Uptr<debug::VulkanDebugMessenger>                _debugger;
        utils::Uptr<VulkanPhysicalDevice>                       _device;
        utils::Uptr<VulkanLogicalDevice>                        _logical_device;
    };

}// namespace app

#endif