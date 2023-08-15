#include "ModelApplication.h"
#include <exception>
#include <iostream>

ModelApplication::ModelApplication( const std::filesystem::path& modelFilePath, const std::filesystem::path& imageFilePath )
    :VulkanApplication::VulkanApplication{"ModelApplication"}
    ,m_modelFilePath{ modelFilePath }
    ,m_imageFilePath{ imageFilePath }
{}

ModelApplication::~ModelApplication()
{}

void ModelApplication::run()
{
    initialise( m_modelFilePath, m_imageFilePath );
    mainLoop();
}

int main()
{
    auto app = ModelApplication{ 
        "models/viking_room.obj",
        "textures/viking_room.png"
    };

    try
    {
        app.run();
    }
    catch( const std::exception& e )
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}