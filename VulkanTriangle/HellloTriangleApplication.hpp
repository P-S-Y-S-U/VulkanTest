#ifndef APP_TRIANGLE_HPP
#define APP_TRIANGLE_HPP

#include "window.hpp"
#include "VulkanInstance.hpp"
#include "VulkanDebugMessenger.hpp"
#include "VulkanPhysicalDevice.hpp"
#include "utilities.hpp"

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
        void mainLoop();
        void cleanup();

        Window                                          _window;
        utils::Uptr<VulkanInstance>                     _instance;
        utils::Uptr<debug::VulkanDebugMessenger>        _debugger;
        app::VulkanPhysicalDevice                       _device;
    };

}// namespace app

#endif