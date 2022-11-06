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

    while( m_window.quit() )
    {
        m_window.processEvents();
    }
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