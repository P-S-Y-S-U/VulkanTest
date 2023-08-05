#include "TriangleApplication.h"
#include <iostream>
#include <stdexcept>
#include <functional>

TriangleApplication::TriangleApplication()
    :VulkanApplication::VulkanApplication( "TriangleApplication" )
{
    m_inputVertexData = {
        {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
        {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
        {{-0.5f ,0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
    };

    m_inputIndexData = {
        0, 1, 2,
        2, 3, 0
    };
}

TriangleApplication::~TriangleApplication()
{}

void TriangleApplication::run() 
{
    initialise();
    mainLoop();
}

int main() {
    auto application = TriangleApplication{};

    try {
        application.run();
    }
    catch (const std::exception & e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}