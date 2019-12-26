#ifndef APP_TRIANGLE
#define APP_TRIANGLE

#include "window.hpp"
#include "VulkanInstance.hpp"
#include "VulkanDebugMessenger.hpp"
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
        void mainLoop();
        void cleanup();

        Window                                          _window;
        utils::Uptr<VulkanInstance>                     _instance;
        utils::Uptr<debug::VulkanDebugMessenger>        _debugger;
    };

}// namespace app

#endif