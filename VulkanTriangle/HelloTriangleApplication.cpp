#include "HellloTriangleApplication.hpp"

namespace app
{
    HelloTriangleApplication::HelloTriangleApplication()
        :_window{}
        , _instance{ std::make_unique<app::VulkanInstance>("Hello Triangle") }
    {}

    void HelloTriangleApplication::run() {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

    void HelloTriangleApplication::initWindow()
    {
        _window.init();
    }
    void HelloTriangleApplication::initVulkan() 
    {
        createInstance();
        setup_debug_messenger();
    }
    void HelloTriangleApplication::createInstance()
    {
        _instance->createInstance();
    }
    void HelloTriangleApplication::setup_debug_messenger()
    {
        if (!(_instance->enable_validation_layer)) { return; }
        _debugger = std::make_unique<app::debug::VulkanDebugMessenger>();
        _debugger->create_debug_messenger(_instance->get_instance(), nullptr);
    }
    void HelloTriangleApplication::mainLoop()
    {
        _window.loop();
    }
    void HelloTriangleApplication::cleanup()
    {
        if (_instance->enable_validation_layer)
        {
            _debugger->destroy_debug_messenger(_instance->get_instance(), nullptr);
        }
        _instance->destroyInstance();
        _window.destroy();
    }
} // namespace app
