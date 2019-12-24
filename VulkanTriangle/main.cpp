#include <iostream>
#include <stdexcept>
#include <functional>
#include <cstdlib>
#include <vector>

#include "window.hpp"
#include "VulkanInstance.h"

class HelloTriangleApplication {
public:
    HelloTriangleApplication()
        :_window{}
        ,_instance{ "Hello Triangle" }
    {}
    ~HelloTriangleApplication() = default;

    void run() {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    void initWindow()
    {
        _window.init();
    }
    void initVulkan() {
        createInstance();
    }
    void createInstance()
    {
        _instance.createInstance();
    }

    void mainLoop() {
        _window.loop();
    }

    void cleanup() {
        _instance.destroyInstance();
        _window.destroy();
    }

    app::Window             _window;
    app::VulkanInstance     _instance;
};

int main() {
    HelloTriangleApplication app;

    try {
        app.run();
    }
    catch (const std::exception & e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}