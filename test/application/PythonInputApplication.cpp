#include "PythonInputApplication.h"
#include <exception>
#include <iostream>

PythonInputApplicaiton::PythonInputApplicaiton()
    :VulkanApplication::VulkanApplication{
        "PythonInputApp"
    }
{

}

PythonInputApplicaiton::~PythonInputApplicaiton()
{

}

void PythonInputApplicaiton::run()
{
    initialise();
    mainLoop();
}

void PythonInputApplicaiton::ParseMeshInputData()
{
    m_inputVertexData = {
        { {-0.5f, -0.5f}, { 1.0f, 0.0f, 0.0f } },
        { { 0.5f, -0.5f }, { 0.0f, 1.0, 0.0f } },
        { { 0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f } },
        { { -0.5f, 0.5f }, { 1.0f, 1.0f, 1.0f } }
    };

    m_inputIndexData = {
        0, 1, 2,
        2, 3, 0
    };
}

int main(int argc, const char* argv[])
{
    auto application = PythonInputApplicaiton{};
    
    application.ParseMeshInputData();
    
    try
    {
        application.run();
    }
    catch( const std::exception& e )
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}