#include "HellloTriangleApplication.hpp"
#include <iostream>
#include <stdexcept>
#include <functional>

namespace app
{
    HelloTriangleApplication::HelloTriangleApplication()
        :_window{}
        ,_instance{ std::make_unique<app::VulkanInstance>("Hello Triangle") }
        ,_device{ std::make_unique<VulkanPhysicalDevice>(_instance) }
        ,_logical_device{ std::make_unique<VulkanLogicalDevice>(_device) }
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
        pick_physical_device();
        create_logical_device();
    }
    void HelloTriangleApplication::createInstance()
    {
        _instance->createInstance();
    }
    void HelloTriangleApplication::setup_debug_messenger()
    {
        if (!(_instance->ENABLE_VALIDATION_LAYER)) { return; }
        _debugger = std::make_unique<app::debug::VulkanDebugMessenger>();
        _debugger->createDebugMessenger(_instance, nullptr);
    }
    void HelloTriangleApplication::pick_physical_device()
    {
        _device->get_physical_devices();
    }
    void HelloTriangleApplication::create_logical_device()
    {
        _logical_device->create_logical_device();
    }
    void HelloTriangleApplication::mainLoop()
    {
        _window.processEvents();
    }
    void HelloTriangleApplication::cleanup()
    {
        _logical_device->destroy_logical_device();
        if (_instance->ENABLE_VALIDATION_LAYER)
        {
            _debugger->destroyDebugMessenger(_instance, nullptr);
        }
        _instance->destroyInstance();
        _window.destroy();
    }
} // namespace app


int main() {
    auto application = app::HelloTriangleApplication{};

    try {
        application.run();
    }
    catch (const std::exception & e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}