#include "TriangleApplication.h"
#include <iostream>
#include <stdexcept>
#include <functional>

TriangleApplication::TriangleApplication()
    :VulkanApplication::VulkanApplication( "TriangleApplication" )
{}

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